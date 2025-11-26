#include "PolylinesVboManager.h"

namespace GLRhi
{
    PolylinesVboManager::PolylinesVboManager()
    {
        m_gl = QOpenGLContext::currentContext()
            ? QOpenGLContext::currentContext()->versionFunctions<QOpenGLFunctions_3_3_Core>()
            : nullptr;
    }

    PolylinesVboManager::~PolylinesVboManager()
    {
        stopBackgroundDefrag();
        {
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            for (auto& [col, vBlock] : m_colorBlocks)
            {

                for (auto b : vBlock)
                {
                    if (m_gl)
                    {
                        m_gl->glDeleteVertexArrays(1, &b->vao);
                        m_gl->glDeleteBuffers(1, &b->vbo);
                        m_gl->glDeleteBuffers(1, &b->ebo);
                    }
                    delete b;
                }
            }
            m_colorBlocks.clear();
            m_locationMap.clear();
            m_vertexCache.clear();
        }
    }

    bool PolylinesVboManager::addPolyline(long long id,
        const std::vector<float>& vVertices,
        const Color& color)
    {
        if ((vVertices.size() < 6) || (vVertices.size() % 3 != 0))
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);

        if (m_locationMap.count(id))
            return false;

        size_t vertCount = vVertices.size() / 3;

        auto* block = findOrCreateColorBlock(color);
        if (!block)
            return false;

        // 确保容量（会自动扩容）
        isBlockEnable(block, block->nVertexCount + vertCount, block->nIndexCount + vertCount);

        // 记录图元信息
        PrimitiveInfo prim;
        prim.id = id;
        prim.nIndexCount = static_cast<GLsizei>(vertCount);
        prim.nBaseVertex = static_cast<GLint>(block->nVertexCount);
        prim.bValid = true;

        size_t primIdx = block->vPrimitives.size();
        block->vPrimitives.push_back(prim);
        block->idToIndexMap[id] = primIdx;

        // 更新计数
        block->nVertexCount += vertCount;
        block->nIndexCount += vertCount;
        block->bDirty = true;

        // 绘制命令（后面 rebuild 时会重新生成，这里先占位）
        block->vDrawCounts.push_back(prim.nIndexCount);
        block->vBaseVertices.push_back(prim.nBaseVertex);

        // 缓存顶点 + 全局位置
        m_vertexCache[id] = vVertices;
        m_locationMap[id] = { color.toUInt32(), color, block, primIdx };

        // 直接全量上传（最安全）
        uploadAllData(block);
        return true;
    }

    bool PolylinesVboManager::addPolylines(std::vector<PolylineData>& vDatas)
    {
        //std::unique_lock<std::shared_mutex> lock(m_mutex);
        bool bRes = true;
        for (auto& plData : vDatas)
        {
            size_t nBaseIndex = 0;
            for (size_t i = 0; i < plData.vId.size(); ++i)
            {
                size_t nCount = plData.vCount[i];
                if (nCount < 2)
                {
                    nBaseIndex += nCount;
                    continue;
                }

                std::vector<float> vVerts;
                vVerts.reserve(nCount * 3);
                const float* src = plData.vVerts.data() + nBaseIndex * 3;
                vVerts.insert(vVerts.end(), src, src + nCount * 3);

                Color c(plData.brush.getColor());
                if (!addPolyline(plData.vId[i], vVerts, c))
                    bRes = false;

                nBaseIndex += nCount;
            }
        }

        return bRes;
    }

    bool PolylinesVboManager::removePolyline(long long id)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_locationMap.find(id);
        if (it == m_locationMap.end())
            return false;

        const auto& loc = it->second;
        auto* block = loc.block;
        size_t idx = loc.primIdx;

        block->vPrimitives[idx].bValid = false;
        block->bDirty = true; // 需要重建绘制命令

        m_locationMap.erase(it);
        m_vertexCache.erase(id);
        block->idToIndexMap.erase(id);

        return true;
    }

    bool PolylinesVboManager::updatePolyline(long long id,
        const std::vector<float>& vVertices)
    {
        if (vVertices.size() < 6 || vVertices.size() % 3 != 0)
            return false;

        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_locationMap.find(id);
        if (it == m_locationMap.end())
            return false;

        const auto& loc = it->second;
        auto* block = loc.block;
        size_t primIdx = loc.primIdx;
        auto& prim = block->vPrimitives[primIdx];

        size_t nNewVertCount = vVertices.size() / 3;

        // 如果大小变化太大，直接删了再加（最安全）
        if (nNewVertCount > prim.nIndexCount * 2 || nNewVertCount < prim.nIndexCount / 2)
        {
            lock.unlock();
            removePolyline(id);
            lock.lock();
            return addPolyline(id, vVertices, loc.color);
        }

        // 否则直接覆盖旧数据（基址不变，长度可能微变）
        prim.nIndexCount = static_cast<GLsizei>(nNewVertCount);
        prim.bValid = true;

        // 更新缓存
        m_vertexCache[id] = vVertices;

        block->bDirty = true;
        uploadAllData(block); // 全量重新上传，最稳妥
        return true;
    }

    bool PolylinesVboManager::setPolylineVisible(long long id, bool visible)
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        auto it = m_locationMap.find(id);
        if (it == m_locationMap.end())
            return false;

        const auto& loc = it->second;
        loc.block->vPrimitives[loc.primIdx].bValid = visible;
        loc.block->bDirty = true;
        return true;
    }

    void PolylinesVboManager::clearAllPrimitives()
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        for (auto& [c, vec] : m_colorBlocks)
        {
            for (auto b : vec)
            {
                if (m_gl)
                {
                    m_gl->glDeleteVertexArrays(1, &b->vao);
                    m_gl->glDeleteBuffers(1, &b->vbo);
                    m_gl->glDeleteBuffers(1, &b->ebo);
                }
                delete b;
            }
        }

        m_colorBlocks.clear();
        m_locationMap.clear();
        m_vertexCache.clear();
    }

    void PolylinesVboManager::renderVisiblePrimitives()
    {
        if (!m_gl)
            return;

        std::shared_lock<std::shared_mutex> lock(m_mutex);

        GLint prog = 0;
        m_gl->glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        GLint uColorLoc = (prog > 0) ? m_gl->glGetUniformLocation(prog, "uColor") : -1;

        for (const auto& [colorKey, vBlocks] : m_colorBlocks)
        {
            if (vBlocks.empty())
                continue;
            // 使用第一个block的颜色（同一键下所有block颜色相同）
            const Color& color = vBlocks[0]->color;
            if (uColorLoc != -1)
                m_gl->glUniform4f(uColorLoc, color.r(), color.g(), color.b(), color.a());

            for (auto* block : vBlocks)
            {
                if (block->vDrawCounts.empty())
                    continue;

                if (block->bDirty)
                    rebuildDrawCommands(block);

                if (block->vDrawCounts.empty())
                    continue;

                bindBlock(block);

                for (size_t i = 0; i < block->vDrawCounts.size(); ++i)
                {
                    m_gl->glDrawElementsBaseVertex(
                        GL_LINE_STRIP,
                        block->vDrawCounts[i],
                        GL_UNSIGNED_INT,
                        nullptr,                    // 索引永远从 0 开始（相对索引）
                        block->vBaseVertices[i]     // 关键：顶点基数
                    );
                }

                unbindBlock();
            }
        }
    }

    //void PolylinesVboManager::renderVisiblePrimitives()
    //{
    //    if (!m_gl)
    //        return;

    //    std::shared_lock<std::shared_mutex> lock(m_mutex); // 支持多线程渲染

    //    GLint prog = 0;
    //    m_gl->glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
    //    GLint uColorLoc = (prog > 0) ? m_gl->glGetUniformLocation(prog, "uColor") : -1;

    //    for (const auto& [colorKey, vBlocks] : m_colorBlocks)
    //    {
    //        if (vBlocks.empty())
    //            continue;
    //        // 使用第一个block的颜色（同一键下所有block颜色相同）
    //        const Color& color = vBlocks[0]->color;
    //        if (uColorLoc != -1)
    //            m_gl->glUniform4f(uColorLoc, color.r(), color.g(), color.b(), color.a());

    //        for (auto* block : vBlocks)
    //        {
    //            if (block->vDrawCounts.empty())
    //                continue;

    //            // 延迟重建有效的绘制命令
    //            if (block->bDirty)
    //                rebuildDrawCommands(block);

    //            if (block->vDrawCounts.empty())
    //                continue;

    //            bindBlock(block);

    //            m_gl->glMultiDrawElementsBaseVertex(
    //                GL_LINE_STRIP,
    //                block->vDrawCounts.data(),
    //                GL_UNSIGNED_INT,
    //                nullptr, // 偏移统一为 0（索引永远从 0 开始）
    //                static_cast<GLsizei>(block->vDrawCounts.size()),
    //                block->vBaseVertices.data());

    //            unbindBlock();
    //        }
    //    }
    //}

    // ====================== 私有实现 ======================

    ColorVBOBlock* PolylinesVboManager::findOrCreateColorBlock(const Color& color)
    {
        uint32_t colorKey = color.toUInt32();
        auto& vBlock = m_colorBlocks[colorKey];
        // 找一个还有足够空间的 block
        for (auto* b : vBlock)
        {
            if (b->nVertexCount + 1000 < MAX_VERT_PER_BLOCK) // 预留一点
                return b;
        }

        return createNewColorBlock(color);
    }

    ColorVBOBlock* PolylinesVboManager::createNewColorBlock(const Color& color)
    {
        uint32_t colorKey = color.toUInt32();
        auto* block = new ColorVBOBlock();
        block->color = color;

        m_gl->glGenVertexArrays(1, &block->vao);
        m_gl->glGenBuffers(1, &block->vbo);
        m_gl->glGenBuffers(1, &block->ebo);

        block->nVertexCapacity = INIT_CAPACITY;
        block->nIndexCapacity = INIT_CAPACITY;

        m_gl->glBindVertexArray(block->vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(block->nVertexCapacity * 3 * sizeof(float)),
            nullptr, GL_DYNAMIC_DRAW);

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(block->nIndexCapacity * sizeof(unsigned int)),
            nullptr, GL_DYNAMIC_DRAW);

        // 位置属性
        m_gl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        m_gl->glEnableVertexAttribArray(0);

        m_gl->glBindVertexArray(0);

        m_colorBlocks[colorKey].push_back(block);
        return block;
    }

    void PolylinesVboManager::isBlockEnable(ColorVBOBlock* block,
        size_t needVert, size_t needIdx)
    {
        if (needVert > block->nVertexCapacity || needIdx > block->nIndexCapacity)
        {
            size_t nNewVertex = block->nVertexCapacity;
            size_t nNewIndex = block->nIndexCapacity;

            while (nNewVertex < needVert)
                nNewVertex += GROW_STEP;

            while (nNewIndex < needIdx)
                nNewIndex += GROW_STEP;

            block->nVertexCapacity = nNewVertex;
            block->nIndexCapacity = nNewIndex;

            // Orphaning + 重新分配（最快且无同步问题）
            m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
            m_gl->glBufferData(GL_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(nNewVertex * 3 * sizeof(float)), nullptr, GL_DYNAMIC_DRAW);

            m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
            m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                static_cast<GLsizeiptr>(nNewIndex * sizeof(unsigned int)), nullptr, GL_DYNAMIC_DRAW);
        }
    }

    void PolylinesVboManager::uploadAllData(ColorVBOBlock* block)
    {
        if (!block->bDirty)
            return;

        // 重新生成紧凑的顶点和索引数据
        std::vector<float> vAllVerts;
        std::vector<unsigned int> vAllIndices;

        size_t nCurrentBase = 0;
        for (auto& prim : block->vPrimitives)
        {
            if (!prim.bValid)
            {
                prim.nBaseVertex = -1; // 标记为无效
                continue;
            }

            const auto& verts = m_vertexCache[prim.id];
            vAllVerts.insert(vAllVerts.end(), verts.begin(), verts.end());

            // 生成相对索引 0,1,2,...
            for (size_t i = 0; i < verts.size() / 3; ++i)
                vAllIndices.push_back(static_cast<unsigned int>(nCurrentBase + i));

            prim.nBaseVertex = static_cast<GLint>(nCurrentBase);
            nCurrentBase += verts.size() / 3;
        }

        // Orphaning 上传
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);

        m_gl->glBufferData(GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(block->nVertexCapacity * 3 * sizeof(float)),
            nullptr, GL_DYNAMIC_DRAW);

        if (!vAllVerts.empty())
        {
            m_gl->glBufferSubData(GL_ARRAY_BUFFER, 0,
                static_cast<GLsizeiptr>(vAllVerts.size() * sizeof(float)), vAllVerts.data());
        }

        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);

        m_gl->glBufferData(GL_ELEMENT_ARRAY_BUFFER,
            static_cast<GLsizeiptr>(block->nIndexCapacity * sizeof(unsigned int)),
            nullptr, GL_DYNAMIC_DRAW);

        if (!vAllIndices.empty())
        {
            m_gl->glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
                static_cast<GLsizeiptr>(vAllIndices.size() * sizeof(unsigned int)), vAllIndices.data());
        }

        block->nVertexCount = vAllVerts.size() / 3;
        block->nIndexCount = vAllIndices.size();
        block->bDirty = false;

        // 重新生成绘制命令（只保留有效的）
        rebuildDrawCommands(block);
    }

    void PolylinesVboManager::rebuildDrawCommands(ColorVBOBlock* block)
    {
        block->vDrawCounts.clear();
        block->vBaseVertices.clear();

        for (const auto& prim : block->vPrimitives)
        {
            if (prim.bValid && prim.nBaseVertex >= 0)
            {
                block->vDrawCounts.push_back(prim.nIndexCount);
                block->vBaseVertices.push_back(prim.nBaseVertex);
            }
        }
    }

    void PolylinesVboManager::bindBlock(ColorVBOBlock* block) const
    {
        m_gl->glBindVertexArray(block->vao);
        m_gl->glBindBuffer(GL_ARRAY_BUFFER, block->vbo);
        m_gl->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, block->ebo);
    }

    void PolylinesVboManager::unbindBlock() const
    {
        m_gl->glBindVertexArray(0);
    }

    // ====================== 碎片整理 ======================

    void PolylinesVboManager::defragment()
    {
        std::unique_lock<std::shared_mutex> lock(m_mutex);

        // 简单的策略：只要有任何无效图元就整体重新打包一次
        bool bDefrag = false;
        for (const auto& [colorKey, vBlock] : m_colorBlocks)
        {
            for (auto* b : vBlock)
            {
                for (const auto& p : b->vPrimitives)
                {
                    if (!p.bValid)
                    {
                        bDefrag = true;
                        break;
                    }
                }
            }
        }

        if (!bDefrag)
            return;

        // 直接调用 uploadAllData 即可完成真正意义上的紧凑
        for (auto& [colorKey, vBlock] : m_colorBlocks)
        {
            for (auto* b : vBlock)
                uploadAllData(b);
        }
    }

    void PolylinesVboManager::startBackgroundDefrag()
    {
        if (m_defragThread.joinable())
            return;

        m_stopDefrag = false;
        m_defragThread = std::thread([this] {
            while (!m_stopDefrag)
            {
                std::this_thread::sleep_for(std::chrono::seconds(8));
                defragment();
            } });
    }

    void PolylinesVboManager::stopBackgroundDefrag()
    {
        if (m_defragThread.joinable())
        {
            m_stopDefrag = true;
            m_defragThread.join();
        }
    }
}