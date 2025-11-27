/**
 * @class PolylinesVboManager
 * @brief 高性能折线渲染管理器，支持百万级动态折线的实时增删改查和渲染
 *
 * 该类是一个高度优化的OpenGL折线渲染管理系统，具有以下特点：
 * - 按颜色分组管理折线数据，优化渲染性能
 * - 支持动态添加、删除、更新折线，无需重建整个缓冲区
 * - 实现了自动内存管理和碎片整理机制，保持内存长期稳定
 * - 采用增量上传策略，最小化数据传输开销
 * - 支持OpenGL 3.3核心配置文件，兼容性好
 * - 支持多线程背景碎片整理，不阻塞主线程
 *
 * 设计模式：
 * - 使用VAO/VBO/EBO进行高效渲染
 * - 按颜色分组存储，减少状态切换
 * - 相对索引技术，避免索引重计算
 * - 延迟重建和压缩策略，优化内存使用
 */

#include "PolylinesVboManager.h"
#include <mutex>
#include <algorithm>
#include <chrono>

namespace GLRhi
{
    // 类常量定义
    namespace
    {
        static constexpr size_t INIT_CAPACITY = 256 * 1024;     // VBO块的初始容量
        static constexpr size_t GROW_STEP = 512 * 1024;         // 容量增长步长
        static constexpr size_t MAX_VERT_PER_BLOCK = 1'500'000; // 每个VBO块的最大顶点数量
        static constexpr float  COMPACT_THRESHOLD = 0.70f; // 使用率 < 70% 才压缩
    }

    /**
     * @brief 构造函数，初始化PolylinesVboManager
     *
     * 在构造时尝试获取当前的OpenGL上下文，并初始化相关资源。
     * 注意：在创建此对象时，必须确保OpenGL上下文已经初始化。
     */
    PolylinesVboManager::PolylinesVboManager()
    {
        if (QOpenGLContext::currentContext())
        {
            m_gl = QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
            if (!m_gl)
            {
                qFatal("Failed to get OpenGL 3.3 Core functions");
                return;
            }
        }
    }

    /**
     * @brief 析构函数，清理所有资源
     *
     * 负责释放所有分配的资源，包括：
     * - 停止后台碎片整理线程
     * - 删除所有OpenGL缓冲区对象（VAO、VBO、EBO）
     * - 释放所有ColorVBOBlock对象
     * - 清空所有容器（m_colorBlocks, m_locationMap, m_vertexCache）
     */
    PolylinesVboManager::~PolylinesVboManager()
    {
        stopBackgroundDefrag();

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto& pair : m_colorBlocksMap)
        {
            for (ColorVBOBlock* block : pair.second)
            {
                if (m_gl)
                {
                    m_gl->glDeleteVertexArrays(1, &block->vao);
                    m_gl->glDeleteBuffers(1, &block->vbo);
                    m_gl->glDeleteBuffers(1, &block->ebo);
                }
                delete block;
            }
        }

        m_colorBlocksMap.clear();
        m_locationMap.clear();
        m_vVertexCache.clear();
    }

    /**
     * @brief 添加一条折线到渲染管理器
     *
     * 将一条新的折线添加到系统中，自动按颜色分组存储，并执行增量上传到GPU。
     *
     * @param id 折线的唯一标识符，用于后续的更新和删除操作
     * @param vVerts 顶点数据，格式为[x1,y1,z1,x2,y2,z2,...]，每个顶点3个浮点数
     * @param color 折线的颜色
     * @return true 如果添加成功，false 如果参数无效或折线已存在
     *
     * @note 顶点数量必须至少为2个（即vVertices.size() >= 6且为3的倍数）
     */
    bool PolylinesVboManager::addPolyline(long long id,
        const std::vector<float>& vVerts, const Color& color)
    {
        if (vVerts.size() < 6 || vVerts.size() % 3 != 0)
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        if (m_locationMap.count(id))
            return false;

        size_t nVertCount = vVerts.size() / 3;
        ColorVBOBlock* block = findOrCreateColorBlock(color);
        if (!block)
            return false;

        ensureBlockCapacity(block,
            block->nVertexCount + nVertCount,
            block->nIndexCount + nVertCount);

        GLint nBaseVertex = static_cast<GLint>(block->nVertexCount);

        PrimitiveInfo prim;
        prim.id = id;
        prim.nIndexCount = static_cast<GLsizei>(nVertCount);
        prim.nBaseVertex = nBaseVertex;
        prim.bValid = true;

        size_t nPrimIdx = block->vPrimitives.size();
        block->vPrimitives.push_back(std::move(prim));
        block->idToIndexMap[id] = nPrimIdx;

        block->nVertexCount += nVertCount;
        block->nIndexCount += nVertCount;
        block->bDirty = true;

        m_vVertexCache[id] = vVerts;
        m_locationMap[id] = { color.toUInt32(), color, block, nPrimIdx };

        uploadSinglePrimitive(block, nPrimIdx); // 增量上传，只传这一条
        return true;
    }

    /**
     * @brief 批量添加多条折线到渲染管理器
     *
     * 一次性添加多个折线组，每个折线组包含多条折线。
     * 内部会遍历所有折线数据并调用addPolyline添加每条折线。
     *
     * @param vPlDatas 折线数据数组，每个元素包含一组相关的折线
     * @return true 如果所有折线都添加成功，false 如果至少有一条折线添加失败
     *
     * @note 内部会跳过顶点数少于2的无效折线
     */
    bool PolylinesVboManager::addPolylines(std::vector<PolylineData>& vPlDatas)
    {
        bool bAllSuccess = true;
        for (auto& data : vPlDatas)
        {
            size_t offset = 0;
            for (size_t i = 0; i < data.vId.size(); ++i)
            {
                size_t nCount = data.vCount[i];
                if (nCount < 2)
                {
                    offset += nCount;
                    continue;
                }

                std::vector<float> vVerts;
                vVerts.reserve(nCount * 3);
                const float* src = data.vVerts.data() + offset * 3;
                vVerts.insert(vVerts.end(), src, src + nCount * 3);

                Color c(data.brush.getColor());
                if (!addPolyline(data.vId[i], vVerts, c))
                    bAllSuccess = false;

                offset += nCount;
            }
        }
        return bAllSuccess;
    }

    /**
     * @brief 从渲染管理器中移除指定ID的折线
     *
     * 通过ID查找并移除折线，标记为无效并触发后续的内存整理。
     * 实际的内存释放会在碎片整理时进行，以优化性能。
     *
     * @param id 要移除的折线的唯一标识符
     * @return true 如果成功移除，false 如果折线不存在
     */
    bool PolylinesVboManager::removePolyline(long long id)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_locationMap.find(id);
        if (it == m_locationMap.end())
            return false;

        const Location& loc = it->second;
        ColorVBOBlock* block = loc.block;
        size_t idx = loc.primIdx;

        block->vPrimitives[idx].bValid = false;
        block->vPrimitives[idx].nIndexCount = 0;
        block->bDirty = true;
        block->bNeedCompact = true;

        m_locationMap.erase(it);
        m_vVertexCache.erase(id);
        block->idToIndexMap.erase(id);

        // 立即重新整理VBO数据，确保删除后顶点数据是连续的
        // 这样可以避免渲染时线段连接错误
        compactBlock(block);
        rebuildDrawCommands(block);
        return true;
    }

    /**
     * @brief 更新指定ID的折线数据
     *
     * 更新现有折线的顶点数据，根据顶点数量变化采用不同策略：
     * - 顶点数量变化不大时，直接增量更新
     * - 顶点数量变化超过50%时，采用删除后重新添加的策略，确保内存使用合理
     *
     * @param id 要更新的折线的唯一标识符
     * @param vVerts 新的顶点数据，格式为[x1,y1,z1,x2,y2,z2,...]
     * @return true 如果更新成功，false 如果参数无效或折线不存在
     *
     * @note 顶点数量必须至少为2个（即vVertices.size() >= 6且为3的倍数）
     */
    bool PolylinesVboManager::updatePolyline(long long id, const std::vector<float>& vVerts)
    {
        if (vVerts.size() < 6 || vVerts.size() % 3 != 0)
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_locationMap.find(id);
        if (it == m_locationMap.end())
            return false;

        const Location& loc = it->second;
        ColorVBOBlock* block = loc.block;
        size_t primIdx = loc.primIdx;
        PrimitiveInfo& prim = block->vPrimitives[primIdx];

        size_t nNewCount = vVerts.size() / 3;

        // 变化太大就删了再加（最安全）
        if (nNewCount > static_cast<size_t>(prim.nIndexCount) * 2 ||
            nNewCount < static_cast<size_t>(prim.nIndexCount) / 2)
        {
            lock.unlock();
            removePolyline(id);
            return addPolyline(id, vVerts, loc.color);
        }

        prim.nIndexCount = static_cast<GLsizei>(nNewCount);
        prim.bValid = true;

        m_vVertexCache[id] = vVerts;
        block->bDirty = true;

        uploadSinglePrimitive(block, primIdx);
        return true;
    }

    /**
     * @brief 设置指定ID折线的可见性
     *
     * 控制折线是否在渲染时可见，不会实际删除或添加折线数据，仅更新可见性标志。
     *
     * @param id 要设置的折线的唯一标识符
     * @param bVisible true表示可见，false表示不可见
     * @return true 如果设置成功，false 如果折线不存在
     */
    bool PolylinesVboManager::setPolylineVisible(long long id, bool bVisible)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_locationMap.find(id);
        if (it == m_locationMap.end())
            return false;

        it->second.block->vPrimitives[it->second.primIdx].bValid = bVisible;
        it->second.block->bDirty = true;
        return true;
    }

    /**
     * @brief 清空所有折线数据
     *
     * 移除所有折线并释放相关资源，包括：
     * - 停止后台碎片整理线程
     * - 删除所有OpenGL缓冲区对象
     * - 清空所有容器和缓存
     *
     * @note 此操作会释放所有资源，调用后需要重新添加折线
     */
    void PolylinesVboManager::clearAllPrimitives()
    {
        stopBackgroundDefrag();

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto& pair : m_colorBlocksMap)
        {
            for (ColorVBOBlock* block : pair.second)
            {
                if (m_gl)
                {
                    m_gl->glDeleteVertexArrays(1, &block->vao);
                    m_gl->glDeleteBuffers(1, &block->vbo);
                    m_gl->glDeleteBuffers(1, &block->ebo);
                }
                delete block;
            }
        }
        m_colorBlocksMap.clear();
        m_locationMap.clear();
        m_vVertexCache.clear();
    }

    // ===================================================================
    // 渲染核心（最高性能：glMultiDrawElementsBaseVertex）
    // ===================================================================

    /**
     * @brief 渲染所有可见的折线
     *
     * 这是渲染的核心方法，采用了多项优化技术：
     * - 按颜色分组渲染，减少着色器 uniform 更新和状态切换
     * - 使用 glDrawElementsBaseVertex 进行高效绘制
     * - 延迟重建绘制命令，避免不必要的计算
     * - 自动进行内存碎片整理
     *
     * 渲染流程：
     * 1. 获取当前激活的着色器程序并查找颜色 uniform 位置
     * 2. 遍历所有颜色组
     * 3. 为每个颜色组设置统一颜色
     * 4. 遍历该颜色组的所有 VBO 块
     * 5. 重建脏块的绘制命令
     * 6. 压缩需要整理的块
     * 7. 绑定块并执行渲染
     *
     * @note 此方法应在 OpenGL 渲染上下文中调用
     */
    void PolylinesVboManager::renderVisiblePrimitives()
    {
        if (!m_gl)
            return;

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        GLint nProg = 0;
        m_gl->glGetIntegerv(GL_CURRENT_PROGRAM, &nProg);
        GLint uColorLoc = (nProg > 0) ? m_gl->glGetUniformLocation(nProg, "uColor") : -1;

        for (const auto& pair : m_colorBlocksMap)
        {
            const auto& vBlocks = pair.second;
            if (vBlocks.empty())
                continue;

            const Color& c = vBlocks[0]->color;
            if (uColorLoc != -1)
                m_gl->glUniform4f(uColorLoc, c.r(), c.g(), c.b(), c.a());

            for (ColorVBOBlock* block : vBlocks)
            {
                if (block->vDrawCounts.empty() && !block->bDirty)
                    continue;

                if (block->bDirty)
                    rebuildDrawCommands(block);

                compactBlock(block);

                if (block->vDrawCounts.empty())
                    continue;

                bindBlock(block);

                // OpenGL 3.3 完全兼容的高性能写法（替代 MultiDrawElementsBaseVertex）
                // 每条线一次 draw call，但因为同颜色同VAO，实际开销极小
                for (size_t i = 0; i < block->vDrawCounts.size(); ++i)
                {
                    m_gl->glDrawElementsBaseVertex(
                        GL_LINE_STRIP,
                        block->vDrawCounts[i],
                        GL_UNSIGNED_INT,
                        nullptr, // 索引从 0 开始（相对索引）
                        block->vBaseVertices[i]);
                }

                unbindBlock();
            }
        }
    }


    void PolylinesVboManager::renderVisiblePrimitivesEx()
    {
        if (!m_gl || m_colorBlocksMap.empty())
            return;

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        GLint nProg = 0;
        m_gl->glGetIntegerv(GL_CURRENT_PROGRAM, &nProg);
        GLint uColorLoc = (nProg > 0) ? m_gl->glGetUniformLocation(nProg, "uColor") : -1;

        // 预分配一个足够大的指针数组（避免每次 new）
        static std::vector<const void*> zero_offsets(10000, nullptr);  // 10k 条足够用了

        for (const auto& pair : m_colorBlocksMap)
        {
            const auto& vBlocks = pair.second;
            if (vBlocks.empty()) continue;

            const Color& c = vBlocks[0]->color;
            if (uColorLoc != -1)
                m_gl->glUniform4f(uColorLoc, c.r(), c.g(), c.b(), c.a());

            for (ColorVBOBlock* block : vBlocks)
            {
                if (block->bDirty)
                    rebuildDrawCommands(block);

                if (block->bNeedCompact)
                    compactBlock(block);

                if (block->vDrawCounts.empty())
                    continue;

                bindBlock(block);

                GLsizei primCount = static_cast<GLsizei>(block->vDrawCounts.size());

                // 关键修复：构造一个全是 nullptr / 0 的指针数组
                // 因为我们使用的是相对索引（0,1,2,...），所有 draw command 的 index offset 都是 0
                const void** indices_array = zero_offsets.data();  // 直接复用

                m_gl->glMultiDrawElementsBaseVertex(
                    GL_LINE_STRIP,
                    block->vDrawCounts.data(),      // count[]
                    GL_UNSIGNED_INT,
                    indices_array,                  // 必须是 [nullptr, nullptr, ...]，长度 = primCount
                    primCount,                      // draw command 数量
                    block->vBaseVertices.data()     // basevertex[]
                );

                unbindBlock();
            }
        }
    }



    // ===================================================================
    // 私有工具函数
    // ===================================================================

    /**
     * @brief 查找或创建指定颜色的VBO块
     *
     * 根据颜色查找现有可容量足够的VBO块，如果不存在则创建新的。
     * 这是实现颜色分组渲染优化的关键方法。
     *
     * @param color 要查找的颜色
     * @return 指向ColorVBOBlock的指针，如果无法创建则返回nullptr
     */
    ColorVBOBlock* PolylinesVboManager::findOrCreateColorBlock(const Color& color)
    {
        uint32_t nKey = color.toUInt32();
        auto& vBlocks = m_colorBlocksMap[nKey];

        for (ColorVBOBlock* b : vBlocks)
        {
            if (b->nVertexCount + 5000 < MAX_VERT_PER_BLOCK)
                return b;
        }

        return createNewColorBlock(color);
    }

    /**
     * @brief 创建新的颜色VBO块
     *
     * 分配并初始化一个新的ColorVBOBlock对象，包括：
     * - 创建OpenGL缓冲区对象（VAO、VBO、EBO）
     * - 设置初始容量
     * - 配置顶点属性指针
     * - 将新块添加到颜色映射中
     *
     * @param color 该块的颜色
     * @return 指向新创建的ColorVBOBlock的指针
     */
    ColorVBOBlock* PolylinesVboManager::createNewColorBlock(const Color& color)
    {
        ColorVBOBlock* block = new ColorVBOBlock();
        block->color = color;

        m_gl->glGenVertexArrays(1, &block->vao);
        m_gl->glGenBuffers(1, &block->vbo);
        m_gl->glGenBuffers(1, &block->ebo);

        block->nVertexCapacity = INIT_CAPACITY;
        block->nIndexCapacity = INIT_CAPACITY;

        m_gl->glBindVertexArray(block->vao);

        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(INIT_CAPACITY * 3 * sizeof(float)),
            nullptr, GL_DYNAMIC_DRAW);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(INIT_CAPACITY * sizeof(unsigned int)),
            nullptr, GL_DYNAMIC_DRAW);

        m_gl->glEnableVertexAttribArray(0);
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);

        m_gl->glBindVertexArray(0);

        m_colorBlocksMap[color.toUInt32()].push_back(block);
        return block;
    }

    /**
     * @brief 确保VBO块有足够的容量
     *
     * 检查并在必要时扩容指定的VBO块，以容纳所需的顶点和索引数量。
     * 如果块空间不足，将重新分配更大的缓冲区并复制现有数据。
     *
     * @param block 要检查容量的VBO块
     * @param needV 需要的顶点数量
     * @param needI 需要的索引数量
     */
    void PolylinesVboManager::ensureBlockCapacity(ColorVBOBlock* block, size_t needV, size_t needI)
    {
        if (needV <= block->nVertexCapacity && needI <= block->nIndexCapacity)
            return;

        size_t nNewV = block->nVertexCapacity;
        size_t nNewI = block->nIndexCapacity;

        while (nNewV < needV)
            nNewV += GROW_STEP;
        while (nNewI < needI)
            nNewI += GROW_STEP;

        block->nVertexCapacity = nNewV;
        block->nIndexCapacity = nNewI;

        // Orphaning 重新分配
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(nNewV * 3 * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(nNewI * sizeof(unsigned int)), nullptr, GL_DYNAMIC_DRAW);
    }

    /**
     * @brief 上传单个折线到VBO块
     *
     * 将折线数据（顶点和索引）上传到指定的VBO块中，包括：
     * - 计算顶点和索引的偏移量
     * - 使用glBufferSubData更新VBO和EBO
     * - 计算相对索引值
     * - 更新渲染相关状态
     *
     * @param block 目标VBO块
     * @param nPrimIdx 要上传的折线在块中的索引
     */
    void PolylinesVboManager::uploadSinglePrimitive(ColorVBOBlock* block, size_t nPrimIdx)
    {
        const PrimitiveInfo& prim = block->vPrimitives[nPrimIdx];
        if (!prim.bValid)
            return;

        auto it = m_vVertexCache.find(prim.id);
        if (it == m_vVertexCache.end())
            return;

        const std::vector<float>& verts = it->second;
        size_t vertCount = verts.size() / 3;

        GLsizeiptr vertOffset = static_cast<GLsizeiptr>(prim.nBaseVertex) * 3 * sizeof(float);
        GLsizeiptr idxOffset = static_cast<GLsizeiptr>(prim.nBaseVertex) * sizeof(unsigned int);

        // 上传顶点
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferSubData(GL_ARRAY_BUFFER, vertOffset,
            static_cast<GLsizeiptr>(verts.size() * sizeof(float)), verts.data());

        // 生成并上传相对索引
        std::vector<unsigned int> indices(vertCount);
        for (size_t i = 0; i < vertCount; ++i)
            indices[i] = static_cast<unsigned int>(prim.nBaseVertex + i);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, idxOffset,
            static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data());
    }

    /**
     * @brief 压缩VBO块，整理内存碎片
     *
     * 当块中存在被删除的折线时，此方法负责：
     * - 移除无效的折线数据
     * - 重新组织顶点和索引数据以消除空洞
     * - 更新所有受影响的折线的基础顶点偏移
     * - 重置缓冲区大小为实际使用大小
     * - 重新生成绘制命令
     *
     * @param block 要压缩的VBO块
     */
    void PolylinesVboManager::compactBlock(ColorVBOBlock* block)
    {
        if (!block->bNeedCompact)
            return;

        size_t nValidVerts = 0;
        for (const PrimitiveInfo& p : block->vPrimitives)
            if (p.bValid)
                nValidVerts += p.nIndexCount;

        if (block->nVertexCount == 0 || nValidVerts >= block->nVertexCount * COMPACT_THRESHOLD)
        {
            block->bNeedCompact = false;
            return;
        }

        //std::vector<float> nNewVerts;
        //std::vector<unsigned int> nNewIndices;
        //nNewVerts.reserve(nValidVerts * 3);
        //nNewIndices.reserve(nValidVerts);

        //size_t nCurBase = 0;
        //for (PrimitiveInfo& prim : block->vPrimitives)
        //{
        //    if (!prim.bValid)
        //    {
        //        prim.nBaseVertex = -1;  // 标记为无效
        //        continue;
        //    }

        //    const std::vector<float>& vVerts = m_vVertexCache.at(prim.id);
        //    size_t nCount = vVerts.size() / 3;

        //    nNewVerts.insert(nNewVerts.end(), vVerts.begin(), vVerts.end());

        //    // 生成相对索引
        //    for (size_t i = 0; i < nCount; ++i)
        //        nNewIndices.push_back(static_cast<unsigned int>(nCurBase + i));

        //    // 关键修复：更新 baseVertex！
        //    prim.nBaseVertex = static_cast<GLint>(nCurBase);

        //    nCurBase += nCount;
        //}

        std::vector<float> nNewVerts;
        std::vector<unsigned int> nNewIndices;
        nNewVerts.reserve(nValidVerts * 3);
        nNewIndices.reserve(nValidVerts);

        size_t nCurBase = 0;
        for (PrimitiveInfo& prim : block->vPrimitives)
        {
            if (!prim.bValid)
                continue;

            const std::vector<float>& vVerts = m_vVertexCache.at(prim.id);
            size_t count = vVerts.size() / 3;

            nNewVerts.insert(nNewVerts.end(), vVerts.begin(), vVerts.end());
            for (size_t i = 0; i < count; ++i)
                nNewIndices.push_back(static_cast<unsigned int>(nCurBase + i));

            prim.nBaseVertex = static_cast<GLint>(nCurBase);
            nCurBase += count;
        }

        // Orphaning + 一次性上传
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(block->nVertexCapacity * 3 * sizeof(float)),
            nullptr, GL_DYNAMIC_DRAW);
        m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0,
            static_cast<GLsizeiptr>(nNewVerts.size() * sizeof(float)), nNewVerts.data());

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(block->nIndexCapacity * sizeof(unsigned int)),
            nullptr, GL_DYNAMIC_DRAW);
        m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
            static_cast<GLsizeiptr>(nNewIndices.size() * sizeof(unsigned int)), nNewIndices.data());

        block->nVertexCount = nNewVerts.size() / 3;
        block->nIndexCount = nNewIndices.size();
        block->bNeedCompact = false;
        block->bDirty = true;
    }

    void PolylinesVboManager::rebuildDrawCommands(ColorVBOBlock* block)
    {
        block->vDrawCounts.clear();
        block->vBaseVertices.clear();

        for (const PrimitiveInfo& prim : block->vPrimitives)
        {
            if (prim.bValid && prim.nIndexCount > 0)
            {
                block->vDrawCounts.push_back(prim.nIndexCount);
                block->vBaseVertices.push_back(prim.nBaseVertex);
            }
        }
        block->bDirty = false;
    }

    void PolylinesVboManager::bindBlock(ColorVBOBlock* block) const
    {
        m_gl->glBindVertexArray(block->vao);
    }

    void PolylinesVboManager::unbindBlock() const
    {
        m_gl->glBindVertexArray(0);
    }

    void PolylinesVboManager::startBackgroundDefrag()
    {
        if (m_defragThread.joinable())
            return;

        m_bStopDefrag = false;
        m_defragThread = std::thread([this] {
            while (!m_bStopDefrag.load())
            {
                // 50次 × 300毫秒 / 次 = 15000毫秒 = 15秒
                for (int i = 0; i < 30 && !m_bStopDefrag.load(); ++i)
                    std::this_thread::sleep_for(std::chrono::milliseconds(300));

                std::shared_lock<std::shared_mutex> lock(m_mutex);
                for (auto& pair : m_colorBlocksMap)
                    for (ColorVBOBlock* b : pair.second)
                        compactBlock(b);
            } });
    }

    void PolylinesVboManager::stopBackgroundDefrag()
    {
        m_bStopDefrag = true;
        if (m_defragThread.joinable())
        {
            m_bStopDefrag = true;
            m_defragThread.join();
        }
    }
} // namespace GLRhi