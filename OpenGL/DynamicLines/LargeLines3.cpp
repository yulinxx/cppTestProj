#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>

//-------------------------------------------------------------
// 多线段（Polyline）结构
//-------------------------------------------------------------
struct Polyline
{
    size_t nVboOffset;     // 在 VBO 中的起始顶点 index
    size_t vertexCount;   // 顶点数量
    size_t indexOffset;   // 在 EBO 中的起始 index
    size_t indexCount;    // 需要绘制的 index 数
};

//-------------------------------------------------------------
// 空闲块（Free block）
//-------------------------------------------------------------
struct FreeBlock
{
    size_t offset;
    size_t length;
};

//-------------------------------------------------------------
// 简单 first-fit 空闲块分配器
//-------------------------------------------------------------
bool allocateFreeBlock(std::vector<FreeBlock>& vFreeBlock, size_t need, size_t& nOutOffset)
{
    for (auto& b : vFreeBlock)
    {
        if (b.length >= need)
        {
            nOutOffset = b.offset;
            b.offset += need;
            b.length -= need;
            return true;
        }
    }
    return false; // 无可用块
}

void freeBlock(std::vector<FreeBlock>& vFreeBlock, size_t offset, size_t length)
{
    vFreeBlock.push_back({ offset, length });

    // 尝试合并相邻空闲块（可选）
    std::sort(vFreeBlock.begin(), vFreeBlock.end(),
        [](auto& a, auto& b) { return a.offset < b.offset; });

    for (size_t i = 0; i + 1 < vFreeBlock.size(); )
    {
        if (vFreeBlock[i].offset + vFreeBlock[i].length == vFreeBlock[i + 1].offset)
        {
            vFreeBlock[i].length += vFreeBlock[i + 1].length;
            vFreeBlock.erase(vFreeBlock.begin() + i + 1);
        }
        else
        {
            i++;
        }
    }
}

//-------------------------------------------------------------
// shader
//-------------------------------------------------------------
static const char* vs_src = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    ourColor = aColor;
}
)";

static const char* fs_src = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;
void main()
{
    FragColor = vec4(ourColor, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    return s;
}

GLuint buildProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fs_src);
    GLuint prog = glCreateProgram();

    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return prog;
}

// 生成随机颜色的函数
float randomColorComponent() {
    // 生成0.2到1.0之间的值，确保颜色不会太暗
    return 0.2f + ((rand() % 801) / 1000.0f);
}

//-------------------------------------------------------------
// 随机 Polyline 生成（包含颜色）
//-------------------------------------------------------------
std::vector<float> randomPolylineVertices(int pointCount)
{
    std::vector<float> data(pointCount * 5); // x,y,r,g,b 每个顶点5个float
    
    // 为整个多段线生成一种随机颜色
    float r = randomColorComponent();
    float g = randomColorComponent();
    float b = randomColorComponent();
    
    for (int i = 0; i < pointCount; i++)
    {
        data[i * 5 + 0] = ((rand() % 2000) / 1000.0f) - 1.0f;
        data[i * 5 + 1] = ((rand() % 2000) / 1000.0f) - 1.0f;
        data[i * 5 + 2] = r;
        data[i * 5 + 3] = g;
        data[i * 5 + 4] = b;
    }
    return data;
}

//-------------------------------------------------------------
// 主程序
//-------------------------------------------------------------
int main()
{
    srand((unsigned)time(nullptr));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Polyline CAD + FPS", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    GLuint program = buildProgram();
    glUseProgram(program);

    //-------------------------------------------------------------------
    // 创建一个“大顶点池”和“大索引池”
    //-------------------------------------------------------------------
    const size_t MaxVertices = 500000; // 50 万顶点
    const size_t MaxIndices = 800000; // 索引
    std::vector<FreeBlock> vFreeBlocksVbo;
    vFreeBlocksVbo.push_back({ 0, MaxVertices });

    size_t eboUsedCount = 0;

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 顶点池
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, MaxVertices * sizeof(float) * 5, nullptr, GL_DYNAMIC_DRAW);

    // 索引池
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndices * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 2));

    //-------------------------------------------------------------------
    // Polyline 存储列表
    //-------------------------------------------------------------------
    std::vector<Polyline> polylines;

    //-------------------------------------------------------------------
    // 创建几十条随机 polyline
    //-------------------------------------------------------------------
    for (int i = 0; i < 300; i++)
    {
        int nCount = 5 + rand() % 15; // 多线段每条长度随机
        std::vector<float> vVerts = randomPolylineVertices(nCount);

        size_t nVboOffset = 0;
        if (!allocateFreeBlock(vFreeBlocksVbo, nCount, nVboOffset))
            continue;

        glBufferSubData(GL_ARRAY_BUFFER, nVboOffset * sizeof(float) * 5,
            vVerts.size() * sizeof(float), vVerts.data());

        Polyline pl{};
        pl.nVboOffset = nVboOffset;
        pl.vertexCount = nCount;
        pl.indexOffset = eboUsedCount;
        pl.indexCount = (nCount - 1) * 2;

        std::vector<unsigned int> vIdx(pl.indexCount);
        for (int j = 0; j < nCount - 1; j++)
        {
            vIdx[j * 2 + 0] = nVboOffset + j;
            vIdx[j * 2 + 1] = nVboOffset + j + 1;
        }

        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
            pl.indexOffset * sizeof(unsigned int),
            vIdx.size() * sizeof(unsigned int),
            vIdx.data());

        eboUsedCount += pl.indexCount;
        polylines.push_back(pl);
    }

    //-------------------------------------------------------------------
    // FPS 控制变量
    //-------------------------------------------------------------------
    double fpsTimer = glfwGetTime();
    int nFrameCount = 0;

    //-------------------------------------------------------------------
    // 主循环
    //-------------------------------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        //----------------------------------------------------------
        // 每帧随机更新几条 polyline（顶点直接写入 VBO）
        //----------------------------------------------------------
        for (int i = 0; i < 5; i++)
        {
            if (polylines.empty()) break;
            int id = rand() % polylines.size();

            auto& pl = polylines[id];
            auto vVerts = randomPolylineVertices(pl.vertexCount);

            glBufferSubData(GL_ARRAY_BUFFER,
                pl.nVboOffset * sizeof(float) * 5,
                vVerts.size() * sizeof(float),
                vVerts.data());
        }

        //----------------------------------------------------------
        // 每 1 秒随机添加 / 删除 polyline
        //----------------------------------------------------------
        static double opTimer = glfwGetTime();
        if (glfwGetTime() - opTimer > 1.0)
        {
            opTimer = glfwGetTime();

            int action = rand() % 2;

            if (action == 0)
            {
                // 添加
                int nCount = 5 + rand() % 15;
                auto vVerts = randomPolylineVertices(nCount);

                size_t nVboOffset = 0;
                if (allocateFreeBlock(vFreeBlocksVbo, nCount, nVboOffset))
                {
                    Polyline pl{};
                    pl.nVboOffset = nVboOffset;
                    pl.vertexCount = nCount;
                    pl.indexOffset = eboUsedCount;
                    pl.indexCount = (nCount - 1) * 2;

                    glBufferSubData(GL_ARRAY_BUFFER,
                        nVboOffset * sizeof(float) * 5,
                        vVerts.size() * sizeof(float), vVerts.data());

                    std::vector<unsigned int> vIdx(pl.indexCount);
                    for (int j = 0; j < nCount - 1; j++)
                    {
                        vIdx[j * 2 + 0] = nVboOffset + j;
                        vIdx[j * 2 + 1] = nVboOffset + j + 1;
                    }

                    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                        pl.indexOffset * sizeof(unsigned int),
                        vIdx.size() * sizeof(unsigned int), vIdx.data());

                    eboUsedCount += pl.indexCount;
                    polylines.push_back(pl);
                }
            }
            else
            {
                // 删除
                if (!polylines.empty())
                {
                    int id = rand() % polylines.size();
                    const auto& pl = polylines[id];

                    freeBlock(vFreeBlocksVbo, pl.nVboOffset, pl.vertexCount);
                    polylines.erase(polylines.begin() + id);
                }
            }
        }

        //----------------------------------------------------------
        // 绘制
        //----------------------------------------------------------
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_LINES, eboUsedCount, GL_UNSIGNED_INT, 0);

        //----------------------------------------------------------
        // FPS 计算
        //----------------------------------------------------------
        nFrameCount++;
        double curr = glfwGetTime();

        if (curr - fpsTimer >= 0.5)
        {
            double fps = nFrameCount / (curr - fpsTimer);
            fpsTimer = curr;
            nFrameCount = 0;

            char title[256];
            sprintf(title, "Polyline CAD | FPS: %.2f | Polylines: %zu",
                fps, polylines.size());
            glfwSetWindowTitle(window, title);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
