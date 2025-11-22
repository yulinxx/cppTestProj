//GLFW + glad + OpenGL 3.3，
//
//使用 顶点池 + 索引池 + first - fit 空闲块管理
//
//使用 glMapBufferRange 来高效更新 VBO / EBO（避免频繁 glBufferSubData）
//
//动态 Defragment（整理 VBO）：在检测到碎片化达到阈值时，将所有活跃 polyline 紧凑写入 VBO / EBO，释放连续大块空间
//
//每帧随机更新部分 polyline，周期性添加 / 删除 polyline，窗口标题显示 FPS & polyline 数量
//
//注意：为便于实现 defragment，示例在 CPU 端为每个 Polyline 保留了顶点数据（std::vector<float> verts）。
//这在 CAD 引擎中很常见（以便进行编辑、选择、序列化等），同时也使得重排内存 / 整理时可以直接从 CPU 拷贝到 GPU。

// main.cpp
// Compile with: g++ main.cpp -lglfw -ldl -lGL -pthread (example on Linux)
// Make sure glad and GLFW are available and glad loader included properly.

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <thread>
#include <chrono>

// ----------------------------- (配置常量) -----------------------------
// 缓冲区池大小配置
const size_t MAX_VERTICES = 2000000;  // 最大顶点数
const size_t MAX_INDICES = 4000000;   // 最大索引数

// 初始多段线配置
const size_t MAX_INITIAL_POLYLINES = 1000000;  // 最大尝试创建的初始多段线数量
const int MIN_VERTICES_PER_POLYLINE = 4;       // 每条多段线最小顶点数
const int MAX_VERTICES_PER_POLYLINE = 100;     // 每条多段线最大顶点数

// 每帧更新配置
const int MIN_UPDATES_PER_FRAME = 50;     // 每帧最少更新的多段线数量
const int MAX_UPDATES_PER_FRAME = 700;    // 每帧最多更新的多段线数量

// 周期性操作配置
const double OPERATION_INTERVAL = 1.0;    // 添加/删除操作的时间间隔（秒）
const int UPDATE_VERTEX_PROBABILITY = 4;  // 更新顶点的概率（1/4）

// 内存碎片整理配置
const size_t FRAG_THRESHOLD = 20;         // 空闲块数量阈值，超过时触发整理
const double DEFRAG_INTERVAL = 5.0;       // 两次整理之间的最小时间间隔（秒）

// 帧率更新配置
const double FPS_UPDATE_INTERVAL = 1;     // FPS更新间隔（秒）

// ----------------------------- (数据结构) -----------------------------
// 表示一条多段线（由多个线段连接的顶点序列）
struct Polyline
{
    size_t vboOffset = 0;           // 该多段线顶点数据在VBO中的起始偏移量（以顶点数量为单位）
    size_t vertexCount = 0;         // 顶点数量
    size_t indexOffset = 0;         // 该多段线索引数据在EBO中的起始偏移量（以索引数量为单位）
    size_t indexCount = 0;          // 索引数量，通常为 (vertexCount - 1) * 2，因为每段线段需要2个索引
    std::vector<float> verts;       // CPU端保存的顶点数据副本，格式为x,y,r,g,b坐标和颜色，大小为 vertexCount * 5
    float color[3];                 // 线条颜色 (r, g, b)
};

// 表示VBO中的空闲内存块
struct FreeBlock
{
    size_t offset;   // 空闲块起始偏移量（以顶点数量为单位）
    size_t length;   // 空闲块长度（以顶点数量为单位）
};

// ----------------------------- (内存分配器: 首次适应算法 + 合并) -----------------------------
// 从空闲块列表中分配一个大小为nPts的内存块
// 参数:
//   vFreeBlock: 空闲块列表
//   nPts: 需要的顶点数量
//   nOutOffset: 输出参数，返回分配到的起始偏移量
// 返回值: 成功返回true，失败返回false
bool allocateFreeBlock(std::vector<FreeBlock>& vFreeBlock, size_t nPts, size_t& nOutOffset)
{
    for (auto it = vFreeBlock.begin(); it != vFreeBlock.end(); ++it)
    {
        if (it->length >= nPts)  // 找到第一个足够大的空闲块
        {
            nOutOffset = it->offset;  // 记录分配位置

            if (it->length == nPts)  // 如果恰好完全使用
            {
                vFreeBlock.erase(it);  // 从空闲列表中移除该块
            }
            else  // 如果分配后还有剩余空间
            {
                it->offset += nPts;    // 空闲块起始位置向后移动
                it->length -= nPts;    // 空闲块长度减少
            }
            return true;
        }
    }
    return false;  // 没有足够大的空闲块，分配失败
}

// 释放一个内存块回空闲列表
// 参数:
//   vFreeBlock: 空闲块列表
//   offset: 要释放的块起始偏移量
//   length: 要释放的块长度
void freeBlock(std::vector<FreeBlock>& vFreeBlock, size_t offset, size_t length)
{
    vFreeBlock.push_back({ offset, length });  // 将释放的块添加到空闲列表

    // 按偏移量排序，便于合并相邻块
    std::sort(vFreeBlock.begin(), vFreeBlock.end(),
        [](const FreeBlock& a, const FreeBlock& b) {
            return a.offset < b.offset;  // 按起始位置从小到大排序
        }
    );

    // 合并相邻的空闲块 (例如: 块A结束位置正好是块B起始位置)
    for (size_t i = 0; i + 1 < vFreeBlock.size(); )
    {
        if ((vFreeBlock[i].offset + vFreeBlock[i].length) == (vFreeBlock[i + 1].offset))
        {
            vFreeBlock[i].length += vFreeBlock[i + 1].length;  // 合并两个块
            vFreeBlock.erase(vFreeBlock.begin() + i + 1);       // 删除第二个块
        }
        else
        {
            i++;
        }
    }
}

// ----------------------------- (着色器代码) -----------------------------
// 顶点着色器: 传递顶点位置和颜色
static const char* vs_src = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec3 aColor;
out vec3 ourColor;
void main() {
    gl_Position = vec4(aPos, 0.0, 1.0);
    ourColor = aColor;
}
)";

// 片段着色器: 使用传入的颜色
static const char* fs_src = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

// 编译着色器
GLuint compileShader(GLenum t, const char* src)
{
    GLuint s = glCreateShader(t);  // 创建着色器对象
    glShaderSource(s, 1, &src, nullptr);  // 设置着色器源代码
    glCompileShader(s);  // 编译着色器

    int nRes;
    glGetShaderiv(s, GL_COMPILE_STATUS, &nRes);  // 检查编译状态

    if (!nRes)  // 如果编译失败
    {
        char buf[512];
        glGetShaderInfoLog(s, 512, nullptr, buf);  // 获取错误信息
        std::cerr << "Shader compilation error: " << buf << "\n";
    }
    return s;
}

// 链接着色器程序
GLuint buildProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vs_src);    // 编译顶点着色器
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fs_src);  // 编译片段着色器
    GLuint p = glCreateProgram();  // 创建着色器程序

    glAttachShader(p, vs);  // 附加顶点着色器
    glAttachShader(p, fs);  // 附加片段着色器
    glLinkProgram(p);       // 链接程序
    glDeleteShader(vs);     // 删除着色器对象（已链接到程序中）
    glDeleteShader(fs);

    int nRes;
    glGetProgramiv(p, GL_LINK_STATUS, &nRes);  // 检查链接状态
    if (!nRes)  // 如果链接失败
    {
        char buf[512];
        glGetProgramInfoLog(p, 512, nullptr, buf);  // 获取错误信息
        std::cerr << "Program linking error: " << buf << "\n";
    }

    return p;
}

// -----------------------------  生成随机多段线顶点 -----------------------------
// 生成包含pts个顶点的随机多段线顶点数据
// 返回值: 顶点数据向量，格式为x,y,r,g,b坐标和颜色
std::vector<float> randomPolylineVerts(int pts, const float* color = nullptr)
{
    std::vector<float> v(pts * 5);  // x, y, r, g, b

    // 如果没有提供颜色，使用默认白色
    float lineColor[3] = { 1.0f, 1.0f, 1.0f };
    if (color)
    {
        lineColor[0] = color[0];
        lineColor[1] = color[1];
        lineColor[2] = color[2];
    }

    for (int i = 0; i < pts; i++)
    {
        // 生成 -1.0 到 1.0 之间的随机坐标
        v[i * 5 + 0] = ((rand() % 2000) / 1000.0f) - 1.0f;  // x坐标
        v[i * 5 + 1] = ((rand() % 2000) / 1000.0f) - 1.0f;  // y坐标
        v[i * 5 + 2] = lineColor[0];  // r
        v[i * 5 + 3] = lineColor[1];  // g
        v[i * 5 + 4] = lineColor[2];  // b
    }
    return v;
}

// 生成随机颜色
void generateRandomColor(float* color)
{
    // 生成较鲜艳的颜色，避免太暗
    color[0] = 0.2f + (rand() % 801) / 1000.0f;  // r: 0.2-1.0
    color[1] = 0.2f + (rand() % 801) / 1000.0f;  // g: 0.2-1.0
    color[2] = 0.2f + (rand() % 801) / 1000.0f;  // b: 0.2-1.0
}

// ----------------------------- 内存整理: 紧凑化VBO和EBO -----------------------------
// 将所有活跃的多段线数据紧凑地排列在缓冲区开头，消除内存碎片
// 实现步骤：
//  1. 计算每个多段线的新位置（紧凑排列）
//  2. 使用 glMapBufferRange 映射整个 VBO 和 EBO
//  3. 将 CPU 端数据按新位置复制到 GPU 缓冲区
//  4. 更新每个多段线的元数据（vboOffset, indexOffset）
//  5. 重建空闲列表（只有一个大块空闲区域在末尾）
void defragmentBuffers(GLuint VBO, GLuint EBO,
    std::vector<Polyline>& m_vPlDatas,
    std::vector<FreeBlock>& vFreeBlock,
    size_t MaxVertices, size_t MaxIndices,
    size_t& eboUsedCount)
{
    // 计算新的紧凑布局
    size_t nNextV = 0;  // 下一个可用的VBO起始位置
    size_t nNextI = 0;  // 下一个可用的EBO起始位置

    // 先计算所有多段线的新位置，不修改原数据
    std::vector<std::pair<size_t, size_t>> vPairOffsets;  // 存储<新VBO偏移, 新EBO偏移>
    for (auto& p : m_vPlDatas)
    {
        vPairOffsets.push_back({ nNextV, nNextI });  // 记录新位置
        nNextV += p.vertexCount;  // 更新VBO指针
        nNextI += p.indexCount;   // 更新EBO指针
    }

    // 验证空间是否足够
    if (nNextV > MaxVertices || nNextI > MaxIndices)
    {
        std::cerr << "[Defragmentation] Error: Insufficient buffer space!\n";
        return;
    }

    // 映射VBO到CPU内存以便写入
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void* vPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, MaxVertices * sizeof(float) * 5,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);  // INVALIDATE: 丢弃原有内容

    if (!vPtr)
    {
        std::cerr << "VBO mapping failed!\n";
        return;
    }

    // 映射EBO到CPU内存以便写入
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    void* iPtr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, MaxIndices * sizeof(unsigned int),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    if (!iPtr)
    {
        std::cerr << "EBO mapping failed!\n";
        glUnmapBuffer(GL_ARRAY_BUFFER);  // 先取消VBO映射
        return;
    }

    char* vwrite = (char*)vPtr;           // 转换为字节指针，便于偏移计算
    unsigned int* iWrite = (unsigned int*)iPtr;  // 转换为索引指针

    // 复制数据并更新索引
    for (size_t i = 0; i < m_vPlDatas.size(); ++i)
    {
        auto& p = m_vPlDatas[i];
        auto [newVboOffset, newIndexOffset] = vPairOffsets[i];  // 获取新位置

        // 更新多段线的元数据
        p.vboOffset = newVboOffset;
        p.indexOffset = newIndexOffset;

        // 复制顶点数据（包含颜色）
        size_t vbytes = p.verts.size() * sizeof(float);
        memcpy(vwrite + newVboOffset * sizeof(float) * 5, p.verts.data(), vbytes);

        // 生成线段索引（每个线段需要2个索引）
        // 索引值是相对于整个VBO的全局索引
        for (size_t s = 0; s + 1 < p.vertexCount; ++s)
        {
            size_t idxPos = newIndexOffset + s * 2;
            iWrite[idxPos + 0] = (unsigned int)(newVboOffset + s);      // 线段起始点
            iWrite[idxPos + 1] = (unsigned int)(newVboOffset + s + 1);  // 线段结束点
        }
    }

    // 解除映射，将数据提交到GPU
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    // 重建空闲列表：只有末尾一个大块空闲区域
    vFreeBlock.clear();
    if (nNextV < MaxVertices)
    {
        vFreeBlock.push_back({ nNextV, MaxVertices - nNextV });
    }

    eboUsedCount = nNextI;  // 更新已使用的索引数量

    std::cout << "[Defragmentation] Completed. Polyline count=" << m_vPlDatas.size()
        << " Used vertices=" << nNextV << " Used indices=" << nNextI
        << " Free blocks=" << vFreeBlock.size() << "\n";
}

// ----------------------------- (主函数) -----------------------------
int main()
{
    srand((unsigned)time(nullptr));  // 初始化随机种子

    if (!glfwInit())  // 初始化GLFW
        return -1;

    // 设置OpenGL版本为3.3核心模式
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Polyline Defrag + MapBufferRange", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);  // 设置当前上下文

    // 禁用垂直同步，以便观察实际帧率
    // glfwSwapInterval(1): 启用V-Sync，帧率锁定显示器刷新率(通常60FPS)
    // glfwSwapInterval(0): 禁用V-Sync，帧率无限制
    //glfwSwapInterval(0);

    // 加载OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init glad\n";
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

    // 编译链接着色器程序
    GLuint program = buildProgram();
    glUseProgram(program);

    // 缓冲区池大小配置
    const size_t MaxVertices = 200000;  // 最大顶点数(x,y坐标对)
    const size_t MaxIndices = 400000;   // 最大索引数

    // 创建并初始化VAO/VBO/EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);  // 生成顶点数组对象
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);  // 生成顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 预分配整个VBO空间，使用动态绘制模式(DYNAMIC_DRAW表示数据会频繁更新)
    // 每个顶点包含5个float: x, y, r, g, b
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(float) * 5, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &EBO);  // 生成索引缓冲对象
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    // 设置顶点属性指针: 每个顶点5个float(x,y,r,g,b)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);  // 位置属性
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));  // 颜色属性

    // 初始化空闲块列表：开始时整个VBO都是空闲的
    std::vector<FreeBlock> vFreeBlock;
    vFreeBlock.push_back({ 0, MaxVertices });

    std::vector<Polyline> vPolylines;  // 存储所有多段线
    size_t nEboUsedCount = 0;          // EBO中已使用的索引数量

    // 创建初始多段线
    for (int i = 0; i < MAX_INITIAL_POLYLINES; i++)
    {
        int pts = MIN_VERTICES_PER_POLYLINE + rand() %
            (MAX_VERTICES_PER_POLYLINE - MIN_VERTICES_PER_POLYLINE + 1);  // 每个多段线顶点数

        // 生成随机颜色
        float color[3];
        generateRandomColor(color);
        auto verts = randomPolylineVerts(pts, color);

        size_t vOffset;
        if (!allocateFreeBlock(vFreeBlock, pts, vOffset))  // 从空闲块分配空间
            break;

        Polyline pl{};  // 创建多段线对象
        pl.vertexCount = pts;
        pl.vboOffset = vOffset;
        pl.indexOffset = nEboUsedCount;
        pl.indexCount = (pts > 1) ? (pts - 1) * 2 : 0;  // 线段数量 * 2个索引
        pl.verts = verts;  // 保存CPU端副本
        pl.color[0] = color[0];
        pl.color[1] = color[1];
        pl.color[2] = color[2];

        // 使用glMapBufferRange上传顶点数据到VBO的指定区域
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,
            pl.vboOffset * sizeof(float) * 5,  // 偏移量(字节)
            pl.verts.size() * sizeof(float),     // 大小(字节)
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);  // 写入并失效该范围

        if (ptr)  // 如果映射成功
        {
            memcpy(ptr, pl.verts.data(), pl.verts.size() * sizeof(float));  // 复制数据
            glUnmapBuffer(GL_ARRAY_BUFFER);  // 解除映射
        }
        else  // 映射失败，使用回退方案
        {
            glBufferSubData(GL_ARRAY_BUFFER,
                pl.vboOffset * sizeof(float) * 5,
                pl.verts.size() * sizeof(float),
                pl.verts.data());
        }

        // 构建线段索引数据
        std::vector<unsigned int> idx(pl.indexCount);
        for (int j = 0; j < pts - 1; j++)
        {
            idx[j * 2 + 0] = (unsigned int)(pl.vboOffset + j);      // 线段起点
            idx[j * 2 + 1] = (unsigned int)(pl.vboOffset + j + 1);  // 线段终点
        }

        // 上传索引数据到EBO的指定区域
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        void* iPtr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,
            pl.indexOffset * sizeof(unsigned int),
            idx.size() * sizeof(unsigned int),
            GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);

        if (iPtr)
        {
            memcpy(iPtr, idx.data(), idx.size() * sizeof(unsigned int));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        }
        else
        {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                pl.indexOffset * sizeof(unsigned int),
                idx.size() * sizeof(unsigned int),
                idx.data());
        }

        nEboUsedCount += pl.indexCount;  // 更新已使用索引数
        vPolylines.push_back(std::move(pl));  // 添加到列表
    }

    // FPS计算相关变量
    double fpsTimer = glfwGetTime();
    int frameCount = 0;

    // 内存碎片检测阈值
    const size_t FRAG_THRESHOLD = 20;       // 如果空闲块数量超过20个，触发整理
    const double DEFRAG_INTERVAL = 5.0;     // 最少间隔5秒整理一次
    double lastDefrag = glfwGetTime();      // 上次整理时间

    // ----------------------------- (主渲染循环) -----------------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();  // 处理窗口事件

        // ---- 随机更新部分多段线的顶点数据 ----
        int nUpdates = 50 + rand() % 20;  // 每帧更新5-24条多段线
        for (int u = 0; u < nUpdates; ++u)
        {
            if (vPolylines.empty())
                break;

            int nId = rand() % vPolylines.size();  // 随机选择一条多段线
            Polyline& pl = vPolylines[nId];

            // 随机扰动部分顶点（约25%概率），保持颜色不变
            for (int k = 0; k < (int)pl.vertexCount; ++k)
            {
                if (rand() % 4 == 0)
                {
                    // 只更新位置，保持颜色不变
                    pl.verts[k * 5 + 0] = ((rand() % 2000) / 1000.0f) - 1.0f;  // x坐标
                    pl.verts[k * 5 + 1] = ((rand() % 2000) / 1000.0f) - 1.0f;  // y坐标
                    // 颜色保持不变：pl.verts[k*5+2], pl.verts[k*5+3], pl.verts[k*5+4]
                }
            }

            // 将更新后的顶点数据上传到VBO
            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,
                pl.vboOffset * sizeof(float) * 5,
                pl.verts.size() * sizeof(float),
                GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);

            if (ptr)
            {
                memcpy(ptr, pl.verts.data(), pl.verts.size() * sizeof(float));
                glUnmapBuffer(GL_ARRAY_BUFFER);
            }
            else
            {
                glBufferSubData(GL_ARRAY_BUFFER,
                    pl.vboOffset * sizeof(float) * 5,
                    pl.verts.size() * sizeof(float),
                    pl.verts.data());
            }
        }

        // ---- 周期性添加/删除多段线 ----
        static double dOpTimer = glfwGetTime();
        if (glfwGetTime() - dOpTimer > 1.0)  // 每秒执行一次
        {
            dOpTimer = glfwGetTime();
            if (rand() % 2 == 0)  // 50%概率添加
            {
                // 添加新多段线
                int pts = 4 + rand() % 12;

                // 生成随机颜色
                float color[3];
                generateRandomColor(color);
                auto verts = randomPolylineVerts(pts, color);

                size_t nOffset;

                // 检查空间是否足够
                if (allocateFreeBlock(vFreeBlock, pts, nOffset) &&
                    nEboUsedCount + (pts - 1) * 2 < MaxIndices)
                {
                    Polyline pl;
                    pl.vertexCount = pts;
                    pl.vboOffset = nOffset;
                    pl.indexOffset = nEboUsedCount;
                    pl.indexCount = (pts > 1) ? (pts - 1) * 2 : 0;
                    pl.verts = verts;
                    pl.color[0] = color[0];
                    pl.color[1] = color[1];
                    pl.color[2] = color[2];

                    // 上传顶点数据
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);

                    void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,
                        pl.vboOffset * sizeof(float) * 5,
                        pl.verts.size() * sizeof(float),
                        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);

                    if (ptr)
                    {
                        memcpy(ptr, pl.verts.data(), pl.verts.size() * sizeof(float));
                        glUnmapBuffer(GL_ARRAY_BUFFER);
                    }
                    else
                    {
                        glBufferSubData(GL_ARRAY_BUFFER,
                            pl.vboOffset * sizeof(float) * 5,
                            pl.verts.size() * sizeof(float),
                            pl.verts.data());
                    }

                    // 构建并上传索引数据
                    std::vector<unsigned int> idx(pl.indexCount);
                    for (int j = 0; j < pts - 1; j++)
                    {
                        idx[j * 2 + 0] = (unsigned int)(pl.vboOffset + j);
                        idx[j * 2 + 1] = (unsigned int)(pl.vboOffset + j + 1);
                    }

                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

                    void* iPtr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,
                        pl.indexOffset * sizeof(unsigned int),
                        idx.size() * sizeof(unsigned int),
                        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);

                    if (iPtr)
                    {
                        memcpy(iPtr, idx.data(), idx.size() * sizeof(unsigned int));
                        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
                    }
                    else
                    {
                        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                            pl.indexOffset * sizeof(unsigned int),
                            idx.size() * sizeof(unsigned int),
                            idx.data());
                    }

                    nEboUsedCount += pl.indexCount;
                    vPolylines.push_back(std::move(pl));
                }
            }
            else  // 50%概率删除
            {
                // 删除一条随机的多段线
                if (!vPolylines.empty())
                {
                    int id = rand() % vPolylines.size();
                    Polyline pl = vPolylines[id];

                    // 释放VBO空间（EBO空间暂时不处理，等待defrag整理）
                    freeBlock(vFreeBlock, pl.vboOffset, pl.vertexCount);

                    // 从列表中移除
                    vPolylines.erase(vPolylines.begin() + id);
                    // eboUsedCount 不立即更新，defrag时会重新计算
                }
            }
        }

        // ---- 检测内存碎片并触发整理 ----
        bool needDefrag = false;

        // 条件1: 空闲块数量超过阈值
        if (vFreeBlock.size() > FRAG_THRESHOLD && glfwGetTime() - lastDefrag > DEFRAG_INTERVAL)
        {
            needDefrag = true;
        }

        if (needDefrag)
        {
            // 执行内存整理
            defragmentBuffers(VBO, EBO, vPolylines, vFreeBlock, MaxVertices, MaxIndices, nEboUsedCount);
            lastDefrag = glfwGetTime();  // 更新上次整理时间
        }

        // ---- 渲染 ----
        glClearColor(0, 0, 0, 1);       // 设置清屏颜色为黑色
        glClear(GL_COLOR_BUFFER_BIT);   // 清除颜色缓冲区

        glBindVertexArray(VAO);         // 绑定VAO

        // 检查eboUsedCount有效性后绘制
        if (nEboUsedCount > 0 && nEboUsedCount <= MaxIndices)
        {
            //glDrawElements(GL_LINES, (GLsizei)eboUsedCount, GL_UNSIGNED_INT, 0);  // 绘制所有线段

            size_t effectiveEboCount = 0;
            for (const auto& p : vPolylines)
            {
                effectiveEboCount += p.indexCount;
            }

            if (effectiveEboCount > 0 && effectiveEboCount <= MaxIndices)
            {
                glDrawElements(GL_LINES, (GLsizei)effectiveEboCount, GL_UNSIGNED_INT, 0);
            }
            else if (effectiveEboCount > MaxIndices)
            {
                std::cerr << "Error: Valid index count(" << effectiveEboCount
                    << ") exceeds maximum(" << MaxIndices << ")!\n";
            }
        }

        // ---- 计算并显示FPS ----
        frameCount++;  // 帧计数器加1
        double now = glfwGetTime();  // 当前时间

        if (now - fpsTimer >= 0.5)  // 每0.5秒更新一次标题
        {
            double fps = frameCount / (now - fpsTimer);  // 计算FPS
            fpsTimer = now;  // 重置计时器
            frameCount = 0;  // 重置计数器

            char title[256];
            sprintf(title, "Polylines: %zu  |  FPS: %.1f  |  FreeBlocks: %zu",
                vPolylines.size(), fps, vFreeBlock.size());
            glfwSetWindowTitle(window, title);  // 设置窗口标题
        }

        glfwSwapBuffers(window);  // 交换前后缓冲区
    }

    // 清理资源
    glfwTerminate();
    return 0;
}