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
    size_t offset = 0;        // 当前缓冲区中的顶点偏移 (VBO offset in vertices)
    size_t count = 0;
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
    std::vector<SlotMapEntry> vSlotMapEntry;
    std::vector<uint32_t> vDataIndices;
    std::vector<uint32_t> vSlotIndices;
    uint32_t nFreeHead = ~0u;

public:
    std::vector<PolylineData> vPlDatas;

    Handle create(size_t count, const float* color = nullptr)
    {
        uint32_t slot;
        if (nFreeHead != ~0u)
        {
            slot = nFreeHead;
            nFreeHead = vSlotMapEntry[slot].nNextFree;
        }
        else
        {
            slot = static_cast<uint32_t>(vSlotMapEntry.size());
            vSlotMapEntry.emplace_back();
            vDataIndices.push_back(0);
            vSlotIndices.push_back(slot);
            vPlDatas.emplace_back();
        }

        uint32_t dataIdx = static_cast<uint32_t>(vPlDatas.size() - 1);
        auto& entry = vSlotMapEntry[slot];
        entry.bAlive = true;
        entry.nGeneration++;

        vDataIndices[slot] = dataIdx;
        vSlotIndices[dataIdx] = slot;

        float c[3] = { 1.0f, 1.0f, 1.0f };
        if (color) std::memcpy(c, color, sizeof(c));
        auto& pl = vPlDatas[dataIdx];
        pl.color[0] = c[0];
        pl.color[1] = c[1];
        pl.color[2] = c[2];

        return { slot, entry.nGeneration };
    }

    void destroy(Handle h)
    {
        if (!isValid(h)) return;
        uint32_t slot = h.index;
        vSlotMapEntry[slot].nNextFree = nFreeHead;
        nFreeHead = slot;
        vSlotMapEntry[slot].bAlive = false;
    }

    void remove_at_data_index(size_t dataIdx)
    {
        if (dataIdx >= vPlDatas.size()) return;

        uint32_t slotToDelete = vSlotIndices[dataIdx];
        destroy({ slotToDelete, vSlotMapEntry[slotToDelete].nGeneration });

        size_t lastDataIdx = vPlDatas.size() - 1;
        if (dataIdx != lastDataIdx)
        {
            uint32_t slotToUpdate = vSlotIndices[lastDataIdx];
            vPlDatas[dataIdx] = std::move(vPlDatas.back());
            vDataIndices[slotToUpdate] = static_cast<uint32_t>(dataIdx);
            vSlotIndices[dataIdx] = slotToUpdate;
        }

        vPlDatas.pop_back();
        vSlotIndices.pop_back();
    }

    bool isValid(Handle h) const
    {
        return h.index < vSlotMapEntry.size() && vSlotMapEntry[h.index].bAlive && vSlotMapEntry[h.index].nGeneration == h.nGeneration;
    }

    PolylineData& operator[](Handle h)
    {
        return vPlDatas[vDataIndices[h.index]];
    }
    const PolylineData& operator[](Handle h) const
    {
        return vPlDatas[vDataIndices[h.index]];
    }

    size_t size() const
    {
        return vPlDatas.size();
    }
};

// ====================== 5. OpenGL 资源 ======================
GLuint VBOs[BUFFER_COUNT] = {};
GLuint VAOs[BUFFER_COUNT] = {};
GLsync fences[BUFFER_COUNT] = { nullptr, nullptr }; // 每个缓冲区一个fence
float* mappedPtrs[BUFFER_COUNT] = {};
int nCurBuffer = 0; // CPU当前写入的缓冲区
GLuint indirectBuffer = 0;
FreeBlockMap arrFreeBlockMap[BUFFER_COUNT];
std::vector<DrawCommand> commands;
PolylineSlotMap plSlotMap;

// ====================== 6. OpenGL 初始化 ======================
void initBuffers()
{
    glGenVertexArrays(BUFFER_COUNT, VAOs);
    glGenBuffers(BUFFER_COUNT, VBOs);
    glGenBuffers(1, &indirectBuffer);

    for (int i = 0; i < BUFFER_COUNT; ++i)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);

        // 关键: 使用 glBufferStorage 创建不可变存储
        glBufferStorage(GL_ARRAY_BUFFER, MAX_VERTICES * VERTEX_STRIDE * sizeof(float), nullptr,
            GL_DYNAMIC_STORAGE_BIT | GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::cerr << "Error: glBufferStorage failed with error: 0x" << std::hex << err << std::dec << "\n";
            exit(EXIT_FAILURE);
        }

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

    glBindVertexArray(VAOs[nCurBuffer]);
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

// ====================== 7. 碎片整理 (修复同步) ======================
void defragment()
{
    int back = 1 - nCurBuffer;

    // 等待GPU完成对后台缓冲区的绘制
    if (fences[back])
    {
        glClientWaitSync(fences[back], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        glDeleteSync(fences[back]);
        fences[back] = nullptr;
    }

    size_t cur = 0;
    commands.clear();

    // 紧凑写入后台缓冲区
    for (auto& pl : plSlotMap.vPlDatas)
    {
        pl.offset = cur;
        std::memcpy(mappedPtrs[back] + cur * VERTEX_STRIDE, pl.verts.data(), pl.verts.size() * sizeof(float));
        commands.push_back({ static_cast<GLuint>(pl.count), 1, static_cast<GLuint>(cur), 0 });
        cur += pl.count;
    }

    // 重建后台空闲列表
    arrFreeBlockMap[back].clear();
    if (cur < MAX_VERTICES)
    {
        arrFreeBlockMap[back].emplace(cur, MAX_VERTICES - cur);
    }

    // 上传MDI命令
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawCommand), commands.data(), GL_DYNAMIC_DRAW);

    // 在后台缓冲区插入fence标记绘制开始
    fences[back] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    // 切换到后台缓冲区作为新的绘制目标
    nCurBuffer = back;
    glBindVertexArray(VAOs[nCurBuffer]);

    std::cout << "[Defragment] Completed. Polylines=" << plSlotMap.size()
        << ", Used Vertices=" << cur << ", Free Blocks=" << arrFreeBlockMap[nCurBuffer].size() << "\n";
}

// ====================== 8. 辅助函数 ======================
static std::mt19937& getRNG()
{
    static std::mt19937 rng(static_cast<unsigned>(std::chrono::system_clock::now().time_since_epoch().count()));
    return rng;
}

// 生成随机多段线顶点数据
std::vector<float> randomPolylineVerts(size_t cnt, const float* c)
{
    std::vector<float> v(cnt * VERTEX_STRIDE);
    auto& rng = getRNG();
    std::uniform_real_distribution<float> posDist(-1.0f, 1.0f); // 定义位置分布

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
        #version 330 core
        layout(location=0) in vec2 p;
        layout(location=1) in vec3 c;
        out vec3 v;
        void main() {
            gl_Position = vec4(p, 0.0, 1.0);
            v = c;
        }
    )";
    const char* fsSource = R"(
        #version 330 core
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
    //glfwSwapInterval(0); // 禁用V-Sync

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
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

    initBuffers();
    GLuint prog = buildProgram();
    glUseProgram(prog);

    // 初始填充
    const int initialPolylines = 50000;
    auto& rng = getRNG();
    std::uniform_real_distribution<float> colDist(0.3f, 1.0f);

    for (int i = 0; i < initialPolylines; ++i)
    {
        size_t cnt = 8 + (rng() % 80);
        size_t off = 0;

        if (!allocate(arrFreeBlockMap[nCurBuffer], cnt, off)) break;

        float c[3] = { colDist(rng), colDist(rng), colDist(rng) };
        auto v = randomPolylineVerts(cnt, c);

        std::memcpy(mappedPtrs[nCurBuffer] + off * VERTEX_STRIDE, v.data(), v.size() * sizeof(float));

        auto h = plSlotMap.create(cnt, c);
        plSlotMap[h].offset = off;
        plSlotMap[h].count = cnt;
        plSlotMap[h].verts = std::move(v);

        commands.push_back({ static_cast<GLuint>(cnt), 1, static_cast<GLuint>(off), 0 });
    }

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawCommand), commands.data(), GL_DYNAMIC_DRAW);

    double lastOp = glfwGetTime(), lastDefrag = glfwGetTime(), fpsTime = glfwGetTime();
    int frames = 0;

    // ====================== 10. 修复黑屏的主循环 ======================
    while (!glfwWindowShouldClose(win))
    {
        glfwPollEvents();

        // 等待GPU完成对当前缓冲区的绘制
        if (fences[nCurBuffer])
        {
            glClientWaitSync(fences[nCurBuffer], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
            glDeleteSync(fences[nCurBuffer]);
            fences[nCurBuffer] = nullptr;
        }

        // 定义随机数分布
        auto& rng = getRNG();
        std::uniform_real_distribution<float> posDist(-1.0f, 1.0f); // 修复: 定义位置分布

        // 更新随机多段线
        for (int i = 0; i < 200 && !plSlotMap.vPlDatas.empty(); ++i)
        {
            auto& pl = plSlotMap.vPlDatas[rng() % plSlotMap.vPlDatas.size()];
            for (size_t j = 0; j < pl.count; ++j)
            {
                if (rng() % 3 == 0)
                {
                    pl.verts[j * VERTEX_STRIDE] = posDist(rng);
                    pl.verts[j * VERTEX_STRIDE + 1] = posDist(rng);
                }
            }
            std::memcpy(mappedPtrs[nCurBuffer] + pl.offset * VERTEX_STRIDE, pl.verts.data(),
                pl.verts.size() * sizeof(float));
        }

        // 周期性增/删操作
        if (glfwGetTime() - lastOp > 1.0)
        {
            lastOp = glfwGetTime();
            std::uniform_real_distribution<float> colDist(0.3f, 1.0f); // 定义颜色分布

            if (rng() % 2 == 0 && plSlotMap.size() < 200000)
            {
                size_t cnt = 10 + (rng() % 70);
                size_t off = 0;

                if (allocate(arrFreeBlockMap[nCurBuffer], cnt, off))
                {
                    float c[3] = { colDist(rng), colDist(rng), colDist(rng) };
                    auto v = randomPolylineVerts(cnt, c);

                    std::memcpy(mappedPtrs[nCurBuffer] + off * VERTEX_STRIDE, v.data(), v.size() * sizeof(float));

                    auto h = plSlotMap.create(cnt, c);
                    plSlotMap[h].offset = off;
                    plSlotMap[h].count = cnt;
                    plSlotMap[h].verts = std::move(v);

                    commands.push_back({ static_cast<GLuint>(cnt), 1, static_cast<GLuint>(off), 0 });
                }
            }
            else if (!plSlotMap.vPlDatas.empty())
            {
                size_t idx = rng() % plSlotMap.vPlDatas.size();
                auto& pl = plSlotMap.vPlDatas[idx];

                deallocate(arrFreeBlockMap[nCurBuffer], pl.offset, pl.count);

                commands[idx] = commands.back();
                commands.pop_back();

                plSlotMap.remove_at_data_index(idx);

                glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
                glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(DrawCommand), commands.data(),
                    GL_DYNAMIC_DRAW);
            }
        }

        // 碎片整理
        if (arrFreeBlockMap[nCurBuffer].size() > 40 && glfwGetTime() - lastDefrag > 5.0)
        {
            defragment();
            lastDefrag = glfwGetTime();
        }

        // 渲染
        glClear(GL_COLOR_BUFFER_BIT);
        if (!commands.empty())
        {
            glBindVertexArray(VAOs[nCurBuffer]);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirectBuffer);
            glMultiDrawArraysIndirect(GL_LINE_STRIP, (void*)0, static_cast<GLsizei>(commands.size()), 0);
        }

        // 标记GPU绘制开始
        fences[nCurBuffer] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

        // 切换到另一个缓冲区供CPU下一帧写入
        nCurBuffer = 1 - nCurBuffer;
        glBindVertexArray(VAOs[nCurBuffer]);

        // FPS & 标题更新
        ++frames;
        if (glfwGetTime() - fpsTime >= 0.5)
        {
            double fps = frames / (glfwGetTime() - fpsTime);
            char title[256];
            snprintf(title, sizeof(title), "Polylines: %zu | FPS: %.0f | FreeBlocks: %zu | Buffer: %d",
                plSlotMap.size(), fps, arrFreeBlockMap[nCurBuffer].size(), nCurBuffer);
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