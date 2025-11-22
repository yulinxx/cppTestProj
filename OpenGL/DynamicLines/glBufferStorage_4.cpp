/*

glBufferStorage 的基本信息
- 引入版本 ：OpenGL 4.4
- 主要特点 ：提供了一种更高效的方式来分配和初始化缓冲区对象的存储
- 核心优势 ：
  - 允许应用程序指定缓冲区数据的使用模式（通过 flags 参数）
  - 支持持久映射（persistent mapping）功能
  - 可以设置不可变存储（immutable storage），让驱动进行更好的优化
  - 支持客户端存储（client storage）选项

// 创建一个不可变、动态更新的缓冲区
GLuint buffer;
glGenBuffers(1, &buffer);
glBindBuffer(GL_ARRAY_BUFFER, buffer);
glBufferStorage(GL_ARRAY_BUFFER, 
                bufferSize,     // 缓冲区大小（字节）
                initialData,    // 初始数据（可为NULL）
                GL_DYNAMIC_STORAGE_BIT); // 使用标志


----------------------
glBufferStorage 的使用流程主要包括以下步骤：

## 1. 生成缓冲区对象

GLuint buffer;
glGenBuffers(1, &buffer);
glBindBuffer(GL_ARRAY_BUFFER, buffer);

## 2. 使用 glBufferStorage 创建不可变存储

// 基本语法
void glBufferStorage(GLenum target, GLsizeiptr size, const void* data, 
GLbitfield flags);

// 示例：创建顶点缓冲区，支持动态更新但不能调整大小
const GLsizeiptr bufferSize = vertexCount * sizeof(Vertex);
glBufferStorage(GL_ARRAY_BUFFER, bufferSize, initialVertexData, 
                GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | 
                GL_MAP_PERSISTENT_BIT);

## 3. 配置缓冲区标志 (flags 参数)
常用标志包括：

- GL_DYNAMIC_STORAGE_BIT ：允许通过 glBufferSubData 或 glMapBufferRange 更新内容
- GL_MAP_READ_BIT ：允许映射缓冲区进行读取
- GL_MAP_WRITE_BIT ：允许映射缓冲区进行写入
- GL_MAP_PERSISTENT_BIT ：允许持久映射（可以在GPU访问时同时访问）
- GL_MAP_COHERENT_BIT ：确保CPU和GPU访问的一致性，无需显式同步
- GL_CLIENT_STORAGE_BIT ：提示驱动应将数据存储在客户端可访问的内存

## 4. 绑定缓冲区到目标并设置顶点属性（如适用）
glBindBuffer(GL_ARRAY_BUFFER, buffer);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
glEnableVertexAttribArray(0);

## 5. 更新缓冲区数据（三种常见方式）
### 方式一：使用 glBufferSubData 更新部分数据

glBindBuffer(GL_ARRAY_BUFFER, buffer);
glBufferSubData(GL_ARRAY_BUFFER, offset, updateSize, updateData);

### 方式二：使用 glMapBufferRange 映射并更新
// 非持久映射
void* mappedData = glMapBufferRange(GL_ARRAY_BUFFER, offset, mapSize, 
                                  GL_MAP_WRITE_BIT | 
                                  GL_MAP_INVALIDATE_RANGE_BIT);
memcpy(mappedData, newData, mapSize);
glUnmapBuffer(GL_ARRAY_BUFFER);

### 方式三：使用持久映射（高性能）
// 初始化时设置 GL_MAP_PERSISTENT_BIT 和 GL_MAP_COHERENT_BIT
// 然后映射一次，后续重复使用
void* persistentMappedData = glMapBufferRange(GL_ARRAY_BUFFER, 0, 
bufferSize, 
                                          GL_MAP_WRITE_BIT | 
                                          GL_MAP_PERSISTENT_BIT | 
                                          GL_MAP_COHERENT_BIT);

// 后续更新，无需重新映射
memcpy(persistentMappedData, newData, dataSize);
// 可以插入内存屏障确保可见性
glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

## 6. 使用完缓冲区后释放
glDeleteBuffers(1, &buffer);

## 注意事项
1. 缓冲区大小一旦设定不可更改，需要重新创建缓冲区才能调整大小
2. 选择合适的 flags 对于性能至关重要，应根据实际使用场景选择
3. 持久映射配合相干性标志可以获得最佳性能，但需要OpenGL 4.4或ARB_buffer_storage扩展支持
4. 在使用映射时，注意管理映射范围和同步操作，避免GPU和CPU访问冲突


glBufferStorage的不可变特性说明：
- 1. 大小固定 ：
  - 一旦通过glBufferStorage创建了缓冲区存储，其大小就被固定下来，无法通过常规方法（如glBufferData）重新调整大小。
- 2. 存储属性固定 ：
  - 除了大小外，其他存储属性（如使用方式、标志等）也在创建时确定并不可更改。
- 3. 性能优势 ：
  - 这种不可变性允许GPU和驱动进行更积极的优化，例如将缓冲区放在更合适的内存区域或采用更高效的访问模式。
- 4. 如果需要调整大小 ：
  - 必须销毁当前缓冲区并创建一个新的缓冲区，然后复制数据（如果需要的话）。

  使用场景 ：glBufferStorage特别适合那些大小相对固定的数据，例如静态几何体数据、纹理数据或需要频繁更新但大小不变的数据（通过映射或子数据更新）。
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <random>
#include <cstring>
#include <map>
#include <algorithm>
#include <chrono>

// ====================== 1. 配置常量 ======================
constexpr size_t MAX_VERTICES = 3'000'000; // VBO/CPU顶点池大小
constexpr size_t BUFFER_COUNT = 2;         // 双缓冲

// 顶点格式: X, Y, R, G, B (5 floats per vertex)
constexpr size_t VERTEX_STRIDE = 5;

// ====================== 2. 数据结构 ======================

// Handle: 稳定的外部引用
struct Handle
{
    uint32_t index;
    uint32_t nGeneration;
    bool operator<(const Handle& other) const
    {
        return index < other.index;
    }
};

// MDI Command: 间接绘制参数
struct DrawCommand
{
    GLuint count = 0;         // 顶点数量
    GLuint instanceCount = 1;
    GLuint first = 0;         // 顶点在 VBO 中的起始偏移 (以顶点为单位)
    GLuint baseInstance = 0;
};

// Polyline 数据: CPU 端备份及 GPU 偏移
struct PolylineData
{
    size_t nPtOffset = 0;        // 当前缓冲区中的顶点偏移 (VBO offset in vertices)
    size_t nPtSz = 0;
    float color[3] = { 1.0f, 1.0f, 1.0f };
    std::vector<float> verts; // CPU 备份 (用于碎片整理和更新)
};

// ====================== 3. First-Fit 内存分配器 (O(log N) 合并) ======================
using FreeBlockMap = std::map<size_t, size_t>; // Key: Offset, Value: Size

bool allocate(FreeBlockMap& fb, size_t nPts, size_t& nOutOffset)
{
    if (nPts == 0) return false;
    for (auto it = fb.begin(); it != fb.end(); ++it)
    {
        if (it->second >= nPts)
        {
            nOutOffset = it->first;
            if (it->second == nPts)
            {
                fb.erase(it);
            }
            else
            {
                size_t newOffset = it->first + nPts;
                size_t newSize = it->second - nPts;
                it = fb.erase(it);
                fb.emplace(newOffset, newSize);
            }
            return true;
        }
    }
    return false;
}

void deallocate(FreeBlockMap& fb, size_t offset, size_t size)
{
    if (size == 0) return;
    auto next = fb.lower_bound(offset);
    bool mergedPrev = false;

    if (next != fb.begin())
    {
        auto prev = std::prev(next);
        if (prev->first + prev->second == offset)
        {
            prev->second += size;
            offset = prev->first;
            size = prev->second;
            mergedPrev = true;
        }
    }

    if (next != fb.end() && offset + size == next->first)
    {
        size_t nextSize = next->second;
        fb.erase(next);
        if (mergedPrev)
        {
            auto prev = fb.find(offset);
            if (prev != fb.end()) prev->second += nextSize;
        }
        else
        {
            fb.emplace(offset, size + nextSize);
        }
    }
    else if (!mergedPrev)
    {
        fb.emplace(offset, size);
    }
}

// ====================== 4. SlotMap ======================
struct SlotMapEntry
{
    uint32_t nNextFree = ~0u;
    uint32_t nGeneration = 0;
    bool bAlive = false;
};

class PolylineSlotMap
{
private:
    std::vector<SlotMapEntry> m_vSlotMapEntry;
    std::vector<uint32_t> m_vDataIndices;
    std::vector<uint32_t> m_vSlotIndices;
    uint32_t m_nFreeHead = ~0u;

public:
    std::vector<PolylineData> m_vPlDatas;

    Handle create(size_t count, const float* color = nullptr)
    {
        uint32_t slot;
        if (m_nFreeHead != ~0u)
        {
            slot = m_nFreeHead;
            m_nFreeHead = m_vSlotMapEntry[slot].nNextFree;
        }
        else
        {
            slot = static_cast<uint32_t>(m_vSlotMapEntry.size());
            m_vSlotMapEntry.emplace_back();
            m_vDataIndices.push_back(0);
            m_vSlotIndices.push_back(slot);
            m_vPlDatas.emplace_back();
        }

        uint32_t dataIdx = static_cast<uint32_t>(m_vPlDatas.size() - 1);
        auto& entry = m_vSlotMapEntry[slot];
        entry.bAlive = true;
        entry.nGeneration++;

        m_vDataIndices[slot] = dataIdx;
        m_vSlotIndices[dataIdx] = slot;

        float c[3] = { 1.0f, 1.0f, 1.0f };
        if (color) std::memcpy(c, color, sizeof(c));
        auto& pl = m_vPlDatas[dataIdx];
        pl.color[0] = c[0];
        pl.color[1] = c[1];
        pl.color[2] = c[2];

        return { slot, entry.nGeneration };
    }

    void destroy(Handle h)
    {
        if (!isValid(h)) return;
        uint32_t slot = h.index;
        m_vSlotMapEntry[slot].nNextFree = m_nFreeHead;
        m_nFreeHead = slot;
        m_vSlotMapEntry[slot].bAlive = false;
    }

    void remove_at_data_index(size_t dataIdx)
    {
        if (dataIdx >= m_vPlDatas.size()) return;

        uint32_t slotToDelete = m_vSlotIndices[dataIdx];
        destroy({ slotToDelete, m_vSlotMapEntry[slotToDelete].nGeneration });

        size_t lastDataIdx = m_vPlDatas.size() - 1;
        if (dataIdx != lastDataIdx)
        {
            uint32_t slotToUpdate = m_vSlotIndices[lastDataIdx];
            m_vPlDatas[dataIdx] = std::move(m_vPlDatas.back());
            m_vDataIndices[slotToUpdate] = static_cast<uint32_t>(dataIdx);
            m_vSlotIndices[dataIdx] = slotToUpdate;
        }

        m_vPlDatas.pop_back();
        m_vSlotIndices.pop_back();
    }

    bool isValid(Handle h) const
    {
        return h.index < m_vSlotMapEntry.size() && m_vSlotMapEntry[h.index].bAlive && m_vSlotMapEntry[h.index].nGeneration == h.nGeneration;
    }

    PolylineData& operator[](Handle h)
    {
        return m_vPlDatas[m_vDataIndices[h.index]];
    }
    const PolylineData& operator[](Handle h) const
    {
        return m_vPlDatas[m_vDataIndices[h.index]];
    }

    size_t size() const
    {
        return m_vPlDatas.size();
    }
};

// ====================== 5. OpenGL 资源 ======================
GLuint VBOs[BUFFER_COUNT] = {};
GLuint VAOs[BUFFER_COUNT] = {};
GLsync fences[BUFFER_COUNT] = { nullptr, nullptr }; // 每个缓冲区一个fence
float* mappedPtrs[BUFFER_COUNT] = {};

// 双缓冲核心：分离写入和绘制缓冲区
int nWriteBuffer = 0;  // CPU当前写入的缓冲区
int nDrawBuffer = 1;   // GPU当前绘制的缓冲区（与写入缓冲相反）

GLuint indirectBuffer = 0;
FreeBlockMap arrFreeBlockMap[BUFFER_COUNT];
std::vector<DrawCommand> commands;
PolylineSlotMap objPlineSlotMap;

// ====================== 6. OpenGL 初始化 ======================
void initBuffers()
{
    // glBufferStorage使用流程 - 步骤1: 生成缓冲区对象
    // 1.1 生成顶点数组对象(VAO)
    glGenVertexArrays(BUFFER_COUNT, VAOs);
    // 1.2 生成顶点缓冲对象(VBO)
    glGenBuffers(BUFFER_COUNT, VBOs);
    // 生成间接绘制缓冲区
    glGenBuffers(1, &indirectBuffer);

    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        // glBufferStorage使用流程 - 步骤4: 绑定顶点数组对象(VAO)，准备配置顶点属性
        // 这一步将当前VAO设为活动状态，后续的顶点属性配置都将应用到这个VAO
        glBindVertexArray(VAOs[i]);
        // glBufferStorage使用流程 - 步骤4(续): 绑定顶点缓冲区对象(VBO)到目标
        // 这一步将VBO与当前活动的VAO关联起来
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

        // glBufferStorage使用流程 - 步骤2: 创建不可变缓冲区存储
        // 1. target: GL_ARRAY_BUFFER - 目标是顶点数组缓冲区
        // 2. size: MAX_VERTICES * VERTEX_STRIDE * sizeof(float) - 缓冲区总大小
        // 3. data: nullptr - 不提供初始数据
        // 4. flags: 组合了多个标志以支持高性能更新
        glBufferStorage(GL_ARRAY_BUFFER, MAX_VERTICES * VERTEX_STRIDE * sizeof(float), nullptr,
            // 标志组合说明（glBufferStorage使用流程 - 步骤3: 配置缓冲区标志）
            // GL_DYNAMIC_STORAGE_BIT: 允许通过glBufferSubData或映射更新内容
            // GL_MAP_WRITE_BIT: 允许映射缓冲区进行写入操作
            // GL_MAP_PERSISTENT_BIT: 允许持久映射（GPU访问时CPU可同时访问）
            // GL_MAP_COHERENT_BIT: 确保CPU和GPU访问的一致性，无需显式同步
            GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::cerr << "Error: glBufferStorage failed with error: 0x" << std::hex << err << std::dec << "\n";
            exit(EXIT_FAILURE);
        }

        // glBufferStorage使用流程 - 步骤5(方式三): 建立持久映射（高性能）
        // 这里采用方式三：持久映射，初始化时映射一次，后续重复使用
        // 注意：映射标志必须与glBufferStorage中设置的标志兼容
        mappedPtrs[i] = static_cast<float*>(glMapBufferRange(GL_ARRAY_BUFFER, 0,
            MAX_VERTICES * VERTEX_STRIDE * sizeof(float),
            GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT));

        if (!mappedPtrs[i])
        {
            std::cerr << "Error: Failed to map VBO persistently! Error code: 0x" << std::hex << glGetError() << std::dec << "\n";
            exit(EXIT_FAILURE);
        }

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERTEX_STRIDE * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, VERTEX_STRIDE * sizeof(float), (void*)(2 * sizeof(float)));

        arrFreeBlockMap[i] = { {0, MAX_VERTICES} };
    }

    glBindVertexArray(VAOs[nWriteBuffer]);
}

void cleanup()
{
    // 等待GPU完成
    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        if (fences[i])
        {
            glClientWaitSync(fences[i], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(fences[i]);
            fences[i] = nullptr;
        }
    }

    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        if (mappedPtrs[i])
        {
            glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            mappedPtrs[i] = nullptr;
        }
        if (VBOs[i]) glDeleteBuffers(1, &VBOs[i]);
        if (VAOs[i]) glDeleteVertexArrays(1, &VAOs[i]);
    }
    if (indirectBuffer) glDeleteBuffers(1, &indirectBuffer);
}

// ====================== 7. 碎片整理 ======================
void defragment()
{
    // 碎片整理始终在写入缓冲区上进行
    int targetBuffer = nWriteBuffer;

    // 等待GPU完成对目标缓冲区的绘制
    if (fences[targetBuffer])
    {
        glClientWaitSync(fences[targetBuffer], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(fences[targetBuffer]);
        fences[targetBuffer] = nullptr;
    }

    size_t cur = 0;
    commands.clear();

    // 紧凑写入目标缓冲区
    for (auto& pl : objPlineSlotMap.m_vPlDatas)
    {
        pl.nPtOffset = cur;
        std::memcpy(mappedPtrs[targetBuffer] + cur * VERTEX_STRIDE, pl.verts.data(), pl.verts.size() * sizeof(float));
        commands.push_back({ static_cast<GLuint>(pl.nPtSz), 1, static_cast<GLuint>(cur), 0 });
        cur += pl.nPtSz;
    }

    // 重建目标缓冲区的空闲列表
    arrFreeBlockMap[targetBuffer].clear();
    if (cur < MAX_VERTICES)
    {
        arrFreeBlockMap[targetBuffer].emplace(cur, MAX_VERTICES - cur);
    }

    // 上传MDI命令
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawCommand), commands.data(), GL_DYNAMIC_DRAW);

    // 为目标缓冲区插入fence标记绘制开始
    fences[targetBuffer] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    std::cout << "[Defragment] Completed. Buffer=" << targetBuffer
        << " | Polylines=" << objPlineSlotMap.size()
        << " | Used Vertices=" << cur
        << " | Free Blocks=" << arrFreeBlockMap[targetBuffer].size() << "\n";
}

// ====================== 8. 辅助函数 ======================
static std::mt19937& getRNG()
{
    static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    return rng;
}

// 生成随机多段线顶点数据
std::vector<float> randomPolyline(size_t cnt, const float* c)
{
    std::vector<float> v(cnt * VERTEX_STRIDE);
    auto& rng = getRNG();
    std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);

    for (size_t j = 0; j < cnt; ++j)
    {
        v[j * VERTEX_STRIDE] = posDist(rng);
        v[j * VERTEX_STRIDE + 1] = posDist(rng);
        v[j * VERTEX_STRIDE + 2] = c[0];
        v[j * VERTEX_STRIDE + 3] = c[1];
        v[j * VERTEX_STRIDE + 4] = c[2];
    }
    return v;
}

// 编译着色器
GLuint buildProgram()
{
    const char* vsSource = R"(
        #version 440 core
        layout(location=0) in vec2 p;
        layout(location=1) in vec3 c;
        out vec3 v;
        void main() {
            gl_Position = vec4(p, 0.0, 1.0);
            v = c;
        }
    )";
    const char* fsSource = R"(
        #version 440 core
        in vec3 v;
        out vec4 o;
        void main() {
            o = vec4(v, 1.0);
        }
    )";

    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vsSource, nullptr);
    glCompileShader(v);

    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fsSource, nullptr);
    glCompileShader(f);

    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);

    glDeleteShader(v);
    glDeleteShader(f);
    return p;
}

// ====================== 9. 主函数 ======================
int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(1280, 720, "Dynamic Polylines - MDI + SlotMap + DoubleBuffer", nullptr, nullptr);
    if (!win)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(win);
    //glfwSwapInterval(0); // 禁用V-Sync以获得更高FPS

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        glfwTerminate();
        return -1;
    }

    // 输出OpenGL信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    initBuffers();
    GLuint prog = buildProgram();
    glUseProgram(prog);

    // 初始填充：只在写入缓冲区(0)中填充数据
    const int initialPolylines = 50000;
    auto& rng = getRNG();
    std::uniform_real_distribution<float> colDist(0.3f, 1.0f);

    for (int i = 0; i < initialPolylines; ++i)
    {
        size_t cnt = 8 + (rng() % 80);
        size_t off = 0;

        if (!allocate(arrFreeBlockMap[nWriteBuffer], cnt, off)) break;

        float c[3] = { colDist(rng), colDist(rng), colDist(rng) };
        auto v = randomPolyline(cnt, c);

        std::memcpy(mappedPtrs[nWriteBuffer] + off * VERTEX_STRIDE, v.data(), v.size() * sizeof(float));

        auto h = objPlineSlotMap.create(cnt, c);
        objPlineSlotMap[h].nPtOffset = off;
        objPlineSlotMap[h].nPtSz = cnt;
        objPlineSlotMap[h].verts = std::move(v);

        commands.push_back({ static_cast<GLuint>(cnt), 1, static_cast<GLuint>(off), 0 });
    }

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawCommand), commands.data(), GL_DYNAMIC_DRAW);

    double lastOp = glfwGetTime(), lastDefrag = glfwGetTime(), fpsTime = glfwGetTime();
    int frames = 0;

    // ====================== 10. 修复后的主循环 ======================
    while (!glfwWindowShouldClose(win))
    {
        glfwPollEvents();

        // 1. 等待GPU完成对*写入缓冲区*的绘制（确保2帧前的绘制已完成）
        if (fences[nWriteBuffer])
        {
            glClientWaitSync(fences[nWriteBuffer], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(fences[nWriteBuffer]);
            fences[nWriteBuffer] = nullptr;
        }

        // 2. 更新随机多段线（写入到nWriteBuffer）- 增加更新比例以提高可见性
        std::uniform_real_distribution<float> posDist(-1.0f, 1.0f);
        // 每帧更新更多的多段线，从200增加到500
        for (int i = 0; i < 500 && !objPlineSlotMap.m_vPlDatas.empty(); ++i)
        {
            auto& pl = objPlineSlotMap.m_vPlDatas[rng() % objPlineSlotMap.m_vPlDatas.size()];
            // 提高顶点更新比例，从1/3增加到2/3
            for (size_t j = 0; j < pl.nPtSz; ++j)
            {
                if (rng() % 3 != 0) // 现在2/3的顶点会更新
                {
                    pl.verts[j * VERTEX_STRIDE] = posDist(rng);
                    pl.verts[j * VERTEX_STRIDE + 1] = posDist(rng);
                }
            }
            // 确保数据正确写入到写入缓冲区
            std::memcpy(mappedPtrs[nWriteBuffer] + pl.nPtOffset * VERTEX_STRIDE, pl.verts.data(),
                pl.verts.size() * sizeof(float));

            // 添加内存屏障以确保写入对GPU可见
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }

        // 3. 周期性增/删操作（始终在写入缓冲区上操作）
        if (glfwGetTime() - lastOp > 1.0)
        {
            lastOp = glfwGetTime();
            std::uniform_real_distribution<float> colDist(0.3f, 1.0f);

            if (rng() % 2 == 0 && objPlineSlotMap.size() < 200000)
            {
                size_t cnt = 10 + (rng() % 70);
                size_t off = 0;

                if (allocate(arrFreeBlockMap[nWriteBuffer], cnt, off))
                {
                    float c[3] = { colDist(rng), colDist(rng), colDist(rng) };
                    auto v = randomPolyline(cnt, c);

                    std::memcpy(mappedPtrs[nWriteBuffer] + off * VERTEX_STRIDE, v.data(), v.size() * sizeof(float));

                    auto h = objPlineSlotMap.create(cnt, c);
                    objPlineSlotMap[h].nPtOffset = off;
                    objPlineSlotMap[h].nPtSz = cnt;
                    objPlineSlotMap[h].verts = std::move(v);

                    commands.push_back({ static_cast<GLuint>(cnt), 1, static_cast<GLuint>(off), 0 });
                }
            }
            else if (!objPlineSlotMap.m_vPlDatas.empty())
            {
                size_t idx = rng() % objPlineSlotMap.m_vPlDatas.size();
                auto& pl = objPlineSlotMap.m_vPlDatas[idx];

                deallocate(arrFreeBlockMap[nWriteBuffer], pl.nPtOffset, pl.nPtSz);

                commands[idx] = commands.back();
                commands.pop_back();

                objPlineSlotMap.remove_at_data_index(idx);
            }
        }

        // 4. 碎片整理（在写入缓冲区上操作）
        if (arrFreeBlockMap[nWriteBuffer].size() > 40 && glfwGetTime() - lastDefrag > 5.0)
        {
            defragment();
            lastDefrag = glfwGetTime();
        }

        // 5. 上传最新的commands到GPU（确保在绘制前）
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
        glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawCommand), commands.data(), GL_DYNAMIC_DRAW);

        // 6. 从*绘制缓冲区*渲染
        glClear(GL_COLOR_BUFFER_BIT);
        if (!commands.empty())
        {
            glBindVertexArray(VAOs[nDrawBuffer]); // 绑定绘制缓冲区的VAO
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
            glMultiDrawArraysIndirect(GL_LINE_STRIP, (void*)0, static_cast<GLsizei>(commands.size()), 0);
        }

        // 7. 为*绘制缓冲区*创建Fence（保护GPU正在读取的缓冲）
        fences[nDrawBuffer] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        // 8. 交换缓冲（写入变绘制，绘制变写入）
        std::swap(nWriteBuffer, nDrawBuffer);

        // 9. 为下一帧的CPU写入操作绑定VAO
        glBindVertexArray(VAOs[nWriteBuffer]);

        // FPS & 标题更新
        ++frames;
        if (glfwGetTime() - fpsTime >= 0.5)
        {
            double fps = frames / (glfwGetTime() - fpsTime);
            char title[256];
            snprintf(title, sizeof(title), "Polylines: %zu | FPS: %.0f | FreeBlocks: %zu | WriteBuf: %d | DrawBuf: %d",
                objPlineSlotMap.size(), fps, arrFreeBlockMap[nWriteBuffer].size(), nWriteBuffer, nDrawBuffer);
            glfwSetWindowTitle(win, title);
            fpsTime = glfwGetTime();
            frames = 0;
        }

        glfwSwapBuffers(win);
    }

    cleanup();
    glfwTerminate();
    return 0;
}

// glBufferStorage使用流程 - 步骤6: 释放缓冲区资源
// 在程序结束时清理所有OpenGL资源是良好的编程实践
void cleanup()
{
    // 1. 取消所有缓冲区的映射
    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        if (mappedPtrs[i])
        {
            glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
            glUnmapBuffer(GL_ARRAY_BUFFER);
            mappedPtrs[i] = nullptr;
        }
        
        // 2. 删除同步对象（fences）
        if (fences[i])
        {
            glDeleteSync(fences[i]);
            fences[i] = nullptr;
        }
    }
    
    // 3. 删除顶点数组对象(VAOs)
    glDeleteVertexArrays(BUFFER_COUNT, VAOs);
    
    // 4. 删除顶点缓冲对象(VBOs)
    glDeleteBuffers(BUFFER_COUNT, VBOs);
    
    // 5. 删除间接绘制缓冲区
    glDeleteBuffers(1, &indirectBuffer);
    
    // 注意：glBufferStorage创建的不可变缓冲区在删除前应先取消映射
    // 这样可以确保所有GPU操作完成并释放相关资源