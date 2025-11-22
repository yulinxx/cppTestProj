/*
## 概述
这是一个基于OpenGL的高性能动态多段线渲染程序，能够高效地管理、渲染和更新大量的多段线数据。
程序采用了多种优化技术，包括双缓冲机制、内存碎片整理和高效的对象引用管理系统。

## 核心功能
1. 大规模多段线渲染 ：能够渲染多达50,000条或更多的多段线，每条多段线包含4-12个顶点
2. 动态数据更新 ：支持在运行时随机更新部分多段线顶点位置，实现动态效果
3. 对象生命周期管理 ：通过槽位映射(SlotMap)实现高效的多段线对象创建和删除
4. 内存碎片整理 ：自动检测并整理内存碎片，保持渲染性能
5. 性能监控 ：实时显示FPS和多段线数量等性能指标

## 系统架构
### 1. 数据结构
- Handle ：句柄结构，使用索引+代数组合防止悬垂引用
- PolylineData ：存储单条多段线的顶点数据、颜色和在缓冲区中的位置信息
- FreeBlockMap ：使用有序映射管理空闲内存块

### 2. 内存管理
- 首次适应算法 ：高效查找合适的内存块
- 空闲块合并 ：释放内存时自动与相邻块合并，减少碎片
- 双缓冲设计 ：使用两个缓冲区交替进行渲染和数据更新，提高效率

### 3. 对象管理系统
- SlotMap ：结合数组和链表优势，提供O(1)时间复杂度的对象创建和删除
- 代数跟踪 ：通过递增代数检测无效句柄
- Swap-and-Pop ：删除元素时避免大量数据移动

### 4. OpenGL资源管理
- 双缓冲区 ：两个VBO和VAO交替使用
- 同步机制 ：使用GLsync fence确保GPU和CPU正确同步
- 顶点数据格式 ：每个顶点包含位置(X,Y)和颜色(RGB)信息

## 主要工作流程
### 初始化阶段
1. 初始化GLFW和OpenGL上下文
2. 创建并配置双缓冲VBO和VAO
3. 构建并链接着色器程序
4. 初始化多段线数据和内存管理系统
5. 创建初始的随机多段线集合

### 主渲染循环
1. 事件处理 ：调用glfwPollEvents处理用户输入
2. 数据更新 ：
   - 每200ms随机更新最多1000条多段线的部分顶点
   - 每5秒随机添加或删除多段线
   - 当空闲块过多时执行碎片整理
3. 渲染 ：清除缓冲区并绘制所有多段线
4. 性能监控 ：每秒更新FPS显示和窗口标题
5. 缓冲区交换 ：调用glfwSwapBuffers显示渲染结果

### 碎片整理过程
1. 将所有活跃多段线数据紧凑地复制到后台缓冲区
2. 重建后台缓冲区的空闲块列表
3. 等待GPU完成当前帧渲染
4. 创建新的同步栅栏
5. 切换到整理后的缓冲区

## 技术亮点
1. 高效内存管理 ：使用有序映射实现的首次适应算法，支持快速内存分配和合并
2. 稳定的对象引用 ：Handle系统确保在对象删除和重用后不会出现悬垂引用
3. 双缓冲优化 ：分离渲染和数据更新过程，避免渲染卡顿
4. GPU同步 ：使用fence机制确保CPU和GPU操作的正确同步
5. 低内存开销 ：对象池和数据压缩策略减少内存占用和GC压力

## 代码组织
代码按功能模块清晰组织，包括配置常量、数据结构定义、内存分配器、SlotMap实现、
OpenGL状态管理、初始化函数、辅助函数和主函数等部分，整体结构清晰，注释完善。
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <random>
#include <cstring>
#include <map>
#include <algorithm>

// ====================== 1. 配置常量 ======================
constexpr size_t MAX_VERTICES = 500'000;    // 顶点缓冲区最大容量（顶点数量）
constexpr size_t INIT_PL_NUMS = 50'000;     // 初始化线段数量
constexpr size_t BUFFER_COUNT = 2;          // 双缓冲数量
constexpr size_t VERTEX_STRIDE = 5;         // 每个顶点的浮点数数量（X, Y, R, G, B）

// ====================== 2. 数据结构 ======================

/**
 * @brief 句柄结构，用于引用多段线对象
 *
 * 使用索引和代数的组合来防止悬垂引用问题。
 * 当对象被删除后重新创建时，代数会增加，使旧句柄失效。
 */
struct Handle
{
    uint32_t index;      // 在SlotMap中的索引位置
    uint32_t generation; // 代数，用于检测句柄是否过期

    // 比较运算符，用于在map中排序
    bool operator<(const Handle& other) const
    {
        return index < other.index;
    }
};

/**
 * @brief 多段线数据结构，存储单个多段线的所有信息
 */
struct PolylineData
{
    size_t nPtOffset = 0;           // 在顶点缓冲区中的偏移量（以顶点为单位）
    size_t nPtSz = 0;            // 顶点数量
    std::vector<float> nPts;    // 顶点数据CPU备份 (X,Y,R,G,B交替存储)
};

// ====================== 3. 内存分配器 ======================

// 使用有序映射存储空闲内存块，键为起始偏移，值为块大小
using FreeBlockMap = std::map<size_t, size_t>;

/**
 * @brief 从空闲块列表中分配指定大小的内存块
 * @param mapFB 空闲块映射表引用
 * @param nNeed 需要的顶点数量
 * @param nOutOffset 输出参数，分配到的起始偏移
 * @return 分配成功返回true，失败返回false
 *
 * 使用首次适应算法(first-fit)查找合适的空闲块。
 * 如果找到合适块，将其从空闲列表中移除或分割剩余部分。
 */
bool allocate(FreeBlockMap& mapFB, size_t nNeed, size_t& nOutOffset)
{
    // 遍历所有空闲块，找到第一个大小足够的块
    for (auto it = mapFB.begin(); it != mapFB.end(); ++it)
    {
        if (it->second >= nNeed)
        {
            nOutOffset = it->first; // 记录分配到的偏移

            // 检查是否完全使用该空闲块
            if (it->second == nNeed)
            {
                // 完全使用，直接移除该空闲块
                mapFB.erase(it);
            }
            else
            {
                // 部分使用，分割剩余空间
                size_t newOffset = it->first + nNeed;
                size_t newSize = it->second - nNeed;
                mapFB.erase(it);

                // 添加剩余部分回空闲列表
                mapFB.emplace(newOffset, newSize);
            }
            return true; // 分配成功
        }
    }
    return false; // 没有找到合适的空闲块
}

/**
 * @brief 释放内存块回空闲列表
 * @param fb 空闲块映射表引用
 * @param offset 要释放的块起始偏移
 * @param size 要释放的块大小
 *
 * 释放时会尝试与相邻的空闲块合并，减少内存碎片。
 * 合并操作时间复杂度为O(log N)。
 */
void deallocate(FreeBlockMap& fb, size_t offset, size_t size)
{
    if (size == 0)
        return; // 忽略大小为0的释放请求

    // 查找第一个起始位置大于等于offset的块（用于后继检查）
    auto nextIter = fb.lower_bound(offset);
    bool mergedPrev = false; // 标记是否与前驱块合并

    // 检查前驱块（前一个空闲块）
    if (nextIter != fb.begin())
    {
        auto prev = std::prev(nextIter);
        // 检查前驱块是否与当前释放块相邻（前驱结束位置 == 当前开始位置）
        if (prev->first + prev->second == offset)
        {
            // 与前驱相邻，合并到前驱块
            prev->second += size;
            offset = prev->first; // 更新当前合并块的起始位置
            size = prev->second;  // 更新当前合并块的大小
            mergedPrev = true;    // 标记已与前驱合并
        }
    }

    // 检查后继块（下一个空闲块）
    if (nextIter != fb.end() && offset + size == nextIter->first)
    {
        // 与后继相邻，合并后继块
        size_t nextSize = nextIter->second;
        fb.erase(nextIter); // 移除后继块

        if (mergedPrev)
        {
            // 如果已经与前驱合并，则修改前驱块的大小
            auto prev = fb.find(offset);
            if (prev != fb.end())
            {
                prev->second += nextSize;
            }
        }
        else
        {
            // 没有与前驱合并，插入新的合并块
            fb.emplace(offset, size + nextSize);
        }
    }
    else if (!mergedPrev)
    {
        // 既不与前驱也不与后继相邻，直接插入新空闲块
        fb.emplace(offset, size);
    }
}

// ====================== 4. SlotMap ======================

/**
 * @brief SlotMap条目，管理对象的生命周期和代数
 */
struct SlotMapEntry
{
    uint32_t nextFree = ~0u; // 下一个空闲槽的索引，~0u表示无下一个
    uint32_t generation = 0; // 代数，每次重用槽时递增
    bool alive = false;      // 标记该槽是否被占用
};

/**
 * @brief 多段线SlotMap类，提供稳定的对象引用和高效的内存管理
 *
 * SlotMap结合了数组的缓存友好性和链表的动态性。
 * 支持O(1)的创建和删除操作，同时提供稳定的句柄引用。
 */
class PolylineSlotMap
{
private:
    std::vector<SlotMapEntry> m_vSlotMapEntry;    // 槽位数组，管理对象生命周期
    std::vector<uint32_t> m_vDataIndices;         // 槽位到数据索引的映射 slots[slot] -> data index
    std::vector<uint32_t> m_vSlotIndices;         // 数据索引到槽位的映射 data[index] -> slot index
    uint32_t m_nFreeHead = ~0u;                   // 空闲链表头指针 无符号整数的最大值

public:
    std::vector<PolylineData> m_vPlDatas;         // 实际的多段线数据存储

    /**
     * @brief 创建新的多段线对象
     * @param vertexCount 顶点数量
     * @return 新创建对象的句柄
     */
    Handle create(size_t vertexCount)
    {
        uint32_t nSlot;

        // 检查是否有可重用的空闲槽
        if (m_nFreeHead != ~0u)
        {
            // 从空闲链表中取出槽位
            nSlot = m_nFreeHead;
            m_nFreeHead = m_vSlotMapEntry[nSlot].nextFree;
        }
        else
        {
            // 没有空闲槽，扩展槽位数组
            nSlot = (uint32_t)m_vSlotMapEntry.size();

            m_vSlotMapEntry.emplace_back();
            m_vDataIndices.push_back(0);
            m_vSlotIndices.push_back(nSlot);
            m_vPlDatas.emplace_back();
        }

        // 获取新数据在polylines向量中的索引
        uint32_t nDataIdx = (uint32_t)m_vPlDatas.size() - 1;

        // 初始化槽位状态
        m_vSlotMapEntry[nSlot].alive = true;
        m_vSlotMapEntry[nSlot].generation++;

        m_vDataIndices[nSlot] = nDataIdx;

        m_vSlotIndices[nDataIdx] = nSlot;

        return { nSlot, m_vSlotMapEntry[nSlot].generation };
    }

    /**
     * @brief 销毁指定句柄对应的对象
     * @param h 要销毁的对象的句柄
     */
    void destroy(Handle h)
    {
        if (!isValid(h))
            return;

        uint32_t nSlot = h.index;
        // 将槽位加入空闲链表
        m_vSlotMapEntry[nSlot].nextFree = m_nFreeHead;
        m_nFreeHead = nSlot;
        m_vSlotMapEntry[nSlot].alive = false;
        // 注意：此时polylines中的数据尚未被移除，需要后续调用remove_at_data_index
    }

    /**
     * @brief 从数据索引安全删除多段线
     * @param dataIdx 要删除的数据在polylines向量中的索引
     *
     * 使用swap-and-pop技术避免删除时的数据移动，保持O(1)时间复杂度。
     */
    void remove_at_data_index(size_t nDataIdx)
    {
        if (nDataIdx >= m_vPlDatas.size())
            return;

        // 1. 销毁对应的槽位
        uint32_t nSlotToDelete = m_vSlotIndices[nDataIdx];
        destroy({ nSlotToDelete, m_vSlotMapEntry[nSlotToDelete].generation });

        // 2. 执行swap-and-pop：将末尾元素移动到被删除位置
        size_t nLastDataIdx = m_vPlDatas.size() - 1;
        if (nDataIdx != nLastDataIdx)
        {
            // 找到被移动元素（末尾元素）对应的槽位
            uint32_t nSlotToUpdate = m_vSlotIndices[nLastDataIdx];

            // 移动数据：将末尾元素移动到被删除位置
            m_vPlDatas[nDataIdx] = std::move(m_vPlDatas.back());

            // 更新映射关系
            m_vDataIndices[nSlotToUpdate] = (uint32_t)nDataIdx;
            m_vSlotIndices[nDataIdx] = nSlotToUpdate;
        }

        // 3. 移除末尾元素
        m_vPlDatas.pop_back();
        m_vSlotIndices.pop_back();
    }

    /**
     * @brief 检查句柄是否有效
     * @param h 要检查的句柄
     * @return 句柄有效返回true，否则返回false
     */
    bool isValid(Handle h) const
    {
        return h.index < m_vSlotMapEntry.size() &&
            m_vSlotMapEntry[h.index].alive &&
            m_vSlotMapEntry[h.index].generation == h.generation;
    }

    // 通过句柄访问多段线数据
    PolylineData& operator[](Handle h)
    {
        return m_vPlDatas[m_vDataIndices[h.index]];
    }

    const PolylineData& operator[](Handle h) const
    {
        return m_vPlDatas[m_vDataIndices[h.index]];
    }

    /**
     * @brief 获取当前活跃的多段线数量
     * @return 多段线数量
     */
    size_t size() const
    {
        return m_vPlDatas.size();
    }
};

// ====================== 5. OpenGL 全局状态 ======================
GLuint VBOs[BUFFER_COUNT]{};    // 顶点缓冲区对象数组
GLuint VAOs[BUFFER_COUNT]{};    // 顶点数组对象数组
int nCurBuffer = 0;             // 当前活跃的缓冲区索引
GLsync fence = nullptr;         // GPU同步栅栏，用于双缓冲同步

FreeBlockMap freeBlocksMap[BUFFER_COUNT]; // 每个缓冲区的空闲块管理
PolylineSlotMap objPlineSlotMap;               // 多段线数据管理

// ====================== 6. 初始化函数 ======================

/**
 * @brief 检查OpenGL错误
 * @param context 错误发生的上下文描述
 */
void checkGLError(const char* context)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL Error in " << context << ": " << error << std::endl;
    }
}

/**
 * @brief 初始化OpenGL缓冲区
 * @return 初始化成功返回true，失败返回false
 *
 * 创建双缓冲的VBO和VAO，设置顶点属性，初始化空闲块管理。
 */
bool initBuffers()
{
    // 生成顶点数组和缓冲区对象
    glGenVertexArrays(BUFFER_COUNT, VAOs);
    glGenBuffers(BUFFER_COUNT, VBOs);

    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        // 绑定当前VAO和VBO
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

        // 分配顶点缓冲区内存（动态绘制，数据会频繁更新）
        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * VERTEX_STRIDE * sizeof(float),
            nullptr, GL_DYNAMIC_DRAW);

        // 设置顶点属性指针
        // 属性0：位置 (X, Y)
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERTEX_STRIDE * sizeof(float), (void*)0);

        // 属性1：颜色 (R, G, B)
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_STRIDE * sizeof(float),
            (void*)(2 * sizeof(float)));

        // 初始化空闲块：整个缓冲区开始时空闲
        freeBlocksMap[i] = { {0, MAX_VERTICES} };
    }

    // 解绑VAO，避免意外修改
    glBindVertexArray(0);
    checkGLError("initBuffers");
    return true;
}

/**
 * @brief 更新顶点数据到当前缓冲区
 * @param offset 在缓冲区中的起始偏移（顶点数）
 * @param data 顶点数据指针
 * @param vertexCount 顶点数量
 *
 * 使用glBufferSubData更新部分缓冲区内容，避免全缓冲区更新。
 */
void updateVertexData(size_t offset, const float* data, size_t vertexCount)
{
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[nCurBuffer]);

    glBufferSubData(GL_ARRAY_BUFFER,
        offset * VERTEX_STRIDE * sizeof(float),     // 起始字节偏移
        vertexCount * VERTEX_STRIDE * sizeof(float),// 数据字节大小
        data);                                      // 数据指针
}

/**
 * @brief 批量更新所有多段线数据到当前缓冲区
 *
 * 将所有活跃多段线的数据紧凑地更新到GPU缓冲区。
 * 用于初始化和碎片整理后的数据上传。
 */
void batchUpdateVertexData()
{
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[nCurBuffer]);

    size_t nCurOffset = 0;
    for (const auto& pl : objPlineSlotMap.m_vPlDatas)
    {
        glBufferSubData(GL_ARRAY_BUFFER,
            nCurOffset * VERTEX_STRIDE * sizeof(float),
            pl.nPts.size() * sizeof(float),
            pl.nPts.data());

        nCurOffset += pl.nPtSz;
    }
}

/**
 * @brief 执行碎片整理，优化内存布局
 *
 * 将所有活跃多段线数据紧凑地重新排列到后台缓冲区，
 * 消除内存碎片，然后切换到整理后的缓冲区。
 * 使用双缓冲技术避免渲染卡顿。
 */
void defragment()
{
    int nBack = 1 - nCurBuffer; // 后台缓冲区索引

    // 紧凑复制所有数据到后台缓冲区
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[nBack]);

    size_t nCurOffset = 0;
    for (auto& pl : objPlineSlotMap.m_vPlDatas)
    {
        // 更新多段线在缓冲区中的新偏移
        pl.nPtOffset = nCurOffset;

        // 上传顶点数据到后台缓冲区
        glBufferSubData(GL_ARRAY_BUFFER,
            nCurOffset * VERTEX_STRIDE * sizeof(float),
            pl.nPts.size() * sizeof(float),
            pl.nPts.data());

        nCurOffset += pl.nPtSz;
    }

    // 重建后台缓冲区的空闲列表
    freeBlocksMap[nBack].clear();
    if (nCurOffset < MAX_VERTICES)
    {
        // 添加剩余空间到空闲列表
        freeBlocksMap[nBack].emplace(nCurOffset, MAX_VERTICES - nCurOffset);
    }

    // 等待GPU完成当前帧的所有渲染命令
    if (fence)
    {
        // 等待最多1秒钟（1,000,000,000纳秒）
        glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1000000000);
        glDeleteSync(fence);
    }

    // 创建新的同步栅栏，标记当前GPU状态
    fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // 切换到整理后的缓冲区
    nCurBuffer = nBack;

    std::cout << "[Defragment] Completed. Polylines=" << objPlineSlotMap.size()
        << ", Used Vertices=" << nCurOffset << std::endl;
}

// ====================== 7. 辅助函数 ======================

// 随机数生成器
std::mt19937 rng((unsigned)time(nullptr));
// 位置随机分布 (-1.0 到 1.0，NDC坐标系)
std::uniform_real_distribution<float> pos(-1.0f, 1.0f);
// 颜色随机分布 (0.3 到 1.0，避免太暗的颜色)
std::uniform_real_distribution<float> col(0.3f, 1.0f);

/**
 * @brief 生成随机多段线顶点数据
 * @param nSz 顶点数量
 * @return 包含随机顶点数据的vector
 */
std::vector<float> randomPolyline(size_t nSz/*, const float* c*/)
{
    std::vector<float> v(nSz * VERTEX_STRIDE);

    float c[3] = { col(rng), col(rng), col(rng) };

    for (size_t j = 0; j < nSz; ++j)
    {
        // 位置数据 (X, Y)
        v[j * VERTEX_STRIDE] = pos(rng);
        v[j * VERTEX_STRIDE + 1] = pos(rng);

        // 颜色数据 (R, G, B)
        v[j * VERTEX_STRIDE + 2] = c[0];
        v[j * VERTEX_STRIDE + 3] = c[1];
        v[j * VERTEX_STRIDE + 4] = c[2];
    }
    return v;
}

/**
 * @brief 构建OpenGL着色器程序
 * @return 链接成功的着色器程序ID
 *
 * 创建并编译顶点着色器和片段着色器，然后链接成完整的着色器程序。
 * 包含完整的错误检查和日志输出。
 */
GLuint buildProgram()
{
    // 顶点着色器源码
    // 输入：位置(vec2)和颜色(vec3)，输出颜色到片段着色器
    const char* vs = R"(#version 330 core
layout(location=0) in vec2 p;
layout(location=1) in vec3 c;
out vec3 v_color;
void main() {
    gl_Position = vec4(p, 0, 1);
    v_color = c;
})";

    // 片段着色器源码
    // 输入：顶点着色器传递的颜色，输出最终片段颜色
    const char* fs = R"(#version 330 core
in vec3 v_color;
out vec4 o_color;
void main() {
    o_color = vec4(v_color, 1);
})";

    // 创建着色器对象
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);

    // 设置着色器源码
    glShaderSource(v, 1, &vs, nullptr);
    glShaderSource(f, 1, &fs, nullptr);

    // 编译着色器
    glCompileShader(v);
    glCompileShader(f);

    // 检查编译错误
    GLint success;
    char infoLog[512];

    glGetShaderiv(v, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(v, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation failed: " << infoLog << std::endl;
    }

    glGetShaderiv(f, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(f, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation failed: " << infoLog << std::endl;
    }

    // 创建并链接程序
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);

    // 检查链接错误
    glGetProgramiv(p, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(p, 512, nullptr, infoLog);
        std::cerr << "Program linking failed: " << infoLog << std::endl;
    }

    // 链接完成后可以删除着色器对象（它们已链接到程序中）
    glDeleteShader(v);
    glDeleteShader(f);

    return p;
}

/**
 * @brief 渲染所有多段线
 *
 * 绑定当前VAO，按顺序绘制所有多段线。
 * 每个多段线使用LINE_STRIP图元连接其顶点。
 */
void render()
{
    glBindVertexArray(VAOs[nCurBuffer]);

    size_t currentOffset = 0;
    for (const auto& pl : objPlineSlotMap.m_vPlDatas)
    {
        // 绘制单个多段线
        glDrawArrays(GL_LINE_STRIP,   // 图元类型：线段带
            (GLint)currentOffset,     // 起始顶点索引
            (GLsizei)pl.nPtSz);       // 顶点数量
        currentOffset += pl.nPtSz;
    }
}

// ====================== 8. 主函数 ======================

/**
 * @brief 程序入口点
 * @return 程序退出代码
 *
 * 初始化GLFW和OpenGL上下文，设置渲染循环，处理用户输入和更新逻辑。
 */
int main()
{
    // 初始化GLFW库
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    // 设置OpenGL上下文版本为3.3 Core Profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* win = glfwCreateWindow(2100, 1600,
        "Dynamic Polylines - Simplified Version", nullptr, nullptr);
    if (!win)
    {
        std::cerr << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 设置当前上下文
    glfwMakeContextCurrent(win);

    // 初始化GLAD加载OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD!" << std::endl;
        glfwTerminate();
        return -1;
    }

    //输出OpenGL信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    // 初始化缓冲区
    if (!initBuffers())
    {
        std::cerr << "Failed to initialize buffers!" << std::endl;
        glfwTerminate();
        return -1;
    }

    // 构建着色器程序并设置为当前使用
    GLuint prog = buildProgram();
    glUseProgram(prog);

    // 初始填充多段线数据
    std::cout << std::endl;
    std::cout << "Initializing with sample polylines..." << std::endl;

    for (int i = 0; i < INIT_PL_NUMS; ++i)  // 创建 N个 多段线
    {
        size_t nPts = 4 + rng() % 8; // 每个多段线 N个 顶点
        size_t nOff = 0;

        // 尝试在顶点缓冲区中分配空间
        if (!allocate(freeBlocksMap[nCurBuffer], nPts, nOff))
            break; // 空间不足时停止创建

        // 生成随机颜色和顶点数据
        std::vector<float> verts = randomPolyline(nPts);

        // 创建多段线对象并设置数据
        Handle h = objPlineSlotMap.create(nPts);

        //m_vPlDatas[m_vDataIndices[h.index]];

        PolylineData& plData = objPlineSlotMap[h];
        plData.nPtOffset = nOff;  // 设置在缓冲区中的偏移量
        plData.nPtSz = nPts;   // 设置顶点数量
        plData.nPts = std::move(verts); // 移动顶点数据（避免拷贝）
    }

    // 初始上传所有数据到GPU
    batchUpdateVertexData();

    std::cout << "Initialized " << objPlineSlotMap.size() << " polylines" << std::endl;

    // 时间跟踪变量
    double dLastOpTm = glfwGetTime();     // 上次增删操作时间
    double dLastDefragTm = glfwGetTime(); // 上次碎片整理时间
    double dLastUpdateTm = glfwGetTime(); // 上次顶点更新时间
    double dFpsTm = glfwGetTime();    // 上次FPS更新时间

    int nFrames = 0;                    // 帧计数器
    bool bNeedsUpdate = false;          // 标记是否需要更新（当前未使用）

    // 设置OpenGL状态
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // 深灰色背景
    glLineWidth(1.0f);                     // 设置线宽

    // ====================== 主渲染循环 ======================
    while (!glfwWindowShouldClose(win))
    {
        // 处理窗口事件（输入等）
        glfwPollEvents();

        double dCurTm = glfwGetTime();

        // 随机更新一些多段线的顶点位置
        if (dCurTm - dLastUpdateTm > 0.2f) // 每50ms更新一次（提高频率）
        {
            dLastUpdateTm = dCurTm;

            if (!objPlineSlotMap.m_vPlDatas.empty())
            {
                // 更新更多多段线（最多20个或总数的1/5）
                int nUpdates = std::min(1000, (int)(objPlineSlotMap.m_vPlDatas.size() / 5));
                for (int i = 0; i < nUpdates; ++i)
                {
                    // 随机选择一个多段线
                    size_t nIdx = rng() % objPlineSlotMap.m_vPlDatas.size();
                    auto& pl = objPlineSlotMap.m_vPlDatas[nIdx];

                    // 随机更新部分顶点位置
                    for (size_t j = 0; j < pl.nPtSz; j += 2) // 每隔一个顶点检查
                    {
                        if (rng() % 5 == 0) // 20%的概率更新该顶点
                        {
                            pl.nPts[j * VERTEX_STRIDE] = pos(rng);
                            pl.nPts[j * VERTEX_STRIDE + 1] = pos(rng);
                        }
                    }

                    // 将更新后的数据上传到GPU
                    updateVertexData(pl.nPtOffset, pl.nPts.data(), pl.nPtSz);
                }

                // 添加内存屏障确保GPU能看到所有更新
                // glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
            }
        }

        // 周期性增删操作
        if (dCurTm - dLastOpTm > 5.0) // 每5秒执行一次
        {
            dLastOpTm = dCurTm;

            if (rng() % 2 == 0 && objPlineSlotMap.size() < 5000000) // 50%概率添加，且总数不超过1000
            {
                size_t nPtSz = 4 + rng() % 6; // 新多段线的顶点数
                size_t nOff = 0;

                if (allocate(freeBlocksMap[nCurBuffer], nPtSz, nOff))
                {
                    // 创建新多段线
                    auto verts = randomPolyline(nPtSz);

                    auto h = objPlineSlotMap.create(nPtSz);

                    PolylineData& plData = objPlineSlotMap[h];

                    plData.nPtOffset = nOff;
                    plData.nPtSz = nPtSz;
                    plData.nPts = std::move(verts);

                    // 上传新数据到GPU
                    updateVertexData(nOff, verts.data(), nPtSz);
                }
            }
            else if (!objPlineSlotMap.m_vPlDatas.empty()) // 50%概率删除
            {
                // 随机选择一个多段线删除
                size_t nIdx = rng() % objPlineSlotMap.m_vPlDatas.size();
                auto& pl = objPlineSlotMap.m_vPlDatas[nIdx];

                // 释放顶点缓冲区空间
                deallocate(freeBlocksMap[nCurBuffer], pl.nPtOffset, pl.nPtSz);

                // 从SlotMap中移除多段线
                objPlineSlotMap.remove_at_data_index(nIdx);

                // 重新上传所有数据（简化处理，实际可优化为部分更新）
                batchUpdateVertexData();
            }
        }

        // 碎片整理：当空闲块过多且距离上次整理足够久时执行
        if (freeBlocksMap[nCurBuffer].size() > 5 && dCurTm - dLastDefragTm > 20.0)
        {
            defragment();
            dLastDefragTm = dCurTm;
        }

        // ====================== 渲染阶段 ======================
        glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区
        render();                     // 绘制所有多段线

        // 更新FPS显示
        ++nFrames;
        if (dCurTm - dFpsTm >= 1.0) // 每秒更新一次窗口标题
        {
            double fps = nFrames / (dCurTm - dFpsTm);
            char title[256];
            sprintf_s(title, "Polylines: %zu | FPS: %.0f | FreeBlocks: %zu",
                objPlineSlotMap.size(), fps, freeBlocksMap[nCurBuffer].size());

            glfwSetWindowTitle(win, title);
            dFpsTm = dCurTm;
            nFrames = 0;
        }

        // 交换前后缓冲区
        glfwSwapBuffers(win);
    }

    // ====================== 清理资源 ======================
    if (fence)
    {
        glDeleteSync(fence);
    }
    glDeleteBuffers(BUFFER_COUNT, VBOs);
    glDeleteVertexArrays(BUFFER_COUNT, VAOs);

    glfwDestroyWindow(win);
    glfwTerminate();

    return 0;
}