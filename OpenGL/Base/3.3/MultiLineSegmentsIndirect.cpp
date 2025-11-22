/*
glMultiDrawElementsIndirect 是 OpenGL 4.3 引入的高级绘制函数，
用于通过单个 API 调用执行多个索引化绘制命令，参数存储在 GPU 缓冲区中，极大降低 CPU 开销。

编写一个使用 glMultiDrawElementsIndirect 渲染多条线段的 OpenGL 示例程序，
支持每条线段顶点数量不固定的情况。这种情况下，我们会通过 多个绘制命令 分别指定每条线段的顶点数和起始索引。

glMultiDrawElementsIndirect 是 OpenGL 中用于高效绘制多个图元批次的高级函数，
特别适用于大规模渲染场景（如粒子系统、批处理模型、大规模轨迹绘制等）。
它通过一个命令缓冲区一次性提交多个绘制调用，减少 CPU 与 GPU 之间的交互开销。

void glMultiDrawElementsIndirect(
    GLenum mode,                // 图元类型：GL_TRIANGLES, GL_LINES, GL_LINE_STRIP 等
    GLenum type,                // 索引类型：GL_UNSIGNED_INT, GL_UNSIGNED_SHORT 等
    const void *indirect,       // 指向命令数组的指针（通常是 buffer offset）
    GLsizei drawcount,          // 绘制命令的数量
    GLsizei stride              // 每个命令结构体的字节大小（通常 sizeof(DrawElementsIndirectCommand)）
);

参数通过 DrawElementsIndirectCommand 结构体数组传递：

typedef struct {
    uint32_t count;         // 顶点索引数量
    uint32_t instanceCount; // 实例数量（0表示不绘制）
    uint32_t firstIndex;    // 索引缓冲区的起始偏移（单位为索引个数）
    int32_t  baseVertex;    // 顶点缓冲区偏移量
    uint32_t baseInstance;  // 实例数据偏移量
} DrawElementsIndirectCommand;

重要说明：
结构体大小为 20 字节
当 stride = 0 时，结构体必须紧密排列
firstIndex 需乘以索引类型大小（如 GL_UNSIGNED_INT 为 4）得到实际字节偏移

✅ 示例目标：
•
使用 GL_LINE_STRIP 模式绘制多条独立线段
•
每条线段有不同数量的顶点（即不同数量的线段对）
•
利用 glMultiDrawElementsIndirect 批量提交绘制命令
•
命令数据存储在 GPU 缓冲中（真正的间接绘制）
*/

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

struct DrawElementsIndirectCommand
{
    GLuint count;              // 该折线使用的顶点数量（至少 2 才能画出线）
    GLuint primCount;          // 实例数（这里为 1）
    GLuint firstIndex;         // 索引缓冲中的起始位置
    GLint baseVertex;          // base vertex offset（不用可为 0）
    GLuint reservedMustBeZero; // 必须为 0
};

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(1400, 1400, "GL_LINE_STRIP with glMultiDrawElementsIndirect", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 定义多条折线 每条顶点数不固定
    std::vector<std::vector<float>> polylines =
    {
        {-0.8f, -0.5f, 0.0f, // 折线1  4个点
         -0.5f, -0.2f, 0.0f,
         -0.3f, -0.6f, 0.0f,
         -0.1f, -0.3f, 0.0f},

        {0.1f, 0.1f, 0.0f, // 折线2  3个点
         0.3f, 0.4f, 0.0f,
         0.5f, 0.2f, 0.0f},

        {0.6f, -0.7f, 0.0f, // 折线3  5个点
         0.7f, -0.4f, 0.0f,
         0.75f, -0.6f, 0.0f,
         0.8f, -0.3f, 0.0f,
         0.9f, -0.5f, 0.0f} };

    std::vector<float> vVertices;
    std::vector<GLuint> vIndices;
    std::vector<DrawElementsIndirectCommand> vCmds;

    GLuint indexOffset = 0;
    for (const auto& line : polylines)
    {
        if (line.size() == 0 || line.size() % 3 != 0)
            continue;

        GLuint vertexCount = static_cast<GLuint>(line.size()) / 3;
        if (vertexCount < 2)
            continue;

        vVertices.insert(vVertices.end(), line.begin(), line.end());

        // 当前这条折线在索引缓冲中的起始位置
        GLuint firstIndex = static_cast<GLuint>(vIndices.size());
        for (GLuint i = 0; i < vertexCount; ++i)
        {
            vIndices.push_back(indexOffset + i);
        }
        indexOffset += vertexCount;

        // 创建间接绘制命令
        vCmds.push_back({
            vertexCount, // count: 用多少个顶点画这条折线（N 个点 → N-1 条线段）
            1,           // primCount: 单实例
            firstIndex,  // firstIndex: 起始索引位置
            0,           // baseVertex
            0            // reserved
            });
    }

    // 创建并绑定 VAO/VBO/EBO
    GLuint vao, vbo, ebo, cmdBuffer;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vVertices.size() * sizeof(float), vVertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vIndices.size() * sizeof(GLuint), vIndices.data(), GL_STATIC_DRAW);

    // 顶点属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // 创建间接命令缓冲
    glGenBuffers(1, &cmdBuffer);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, cmdBuffer);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, vCmds.size() * sizeof(DrawElementsIndirectCommand),
        vCmds.data(), GL_STATIC_DRAW);

    // 简单着色器程序
    const char* vsSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const char* fsSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(0.0, 1.0, 1.0, 1.0); // 青色线条
        }
    )";

    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, nullptr);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, nullptr);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glUseProgram(program);

    // 启用多采样抗锯齿（让线条更平滑）
    glEnable(GL_MULTISAMPLE);

    // 设置线宽（部分系统可能忽略 >1.0）
    glLineWidth(8.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 执行间接绘制
        glBindVertexArray(vao);
        glUseProgram(program);
        glMultiDrawElementsIndirect(GL_LINE_STRIP, GL_UNSIGNED_INT, nullptr,
            static_cast<GLsizei>(vCmds.size()), 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glDeleteShader(vs);
    glDeleteShader(fs);
    glDeleteBuffers(1, &cmdBuffer);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}