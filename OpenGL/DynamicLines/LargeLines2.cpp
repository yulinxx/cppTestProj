//随机创建几千条线段
//
//所有线段顶点存在一个大 VBO 中
//使用 元素索引（EBO / Index Buffer） 来绘制
//
//每帧随机挑选一部分线段进行绘制
//隔一段时间删除 / 添加新的线段并更新 VBO

//存在一个大 VBO 中，顶点排列为：
//(x1, y1), (x2, y2)
//
//✔ 使用 EBO 作为索引绘制
//用 glDrawElements(GL_LINES)
//
//✔ 每帧随机更新部分线段
//使用 glBufferSubData 更新 VBO 中某一小段数据
//
//✔ 隔几秒执行“删除 / 添加”线段
//删除：用最后一个覆盖要删除的项
//添加：往 VBO 末尾追加
//
//✔ 所有逻辑极其简单
//便于理解“动态更新 + 使用索引绘制”的基本示例。

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>

struct Line
{
    float x1, y1;
    float x2, y2;
};

static const char* vs_src = R"(#version 330 core
layout (location = 0) in vec2 aPos;
void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
)";

static const char* fs_src = R"(#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1,1,1,1); }
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

int main()
{
    srand((unsigned)time(nullptr));

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Dynamic Lines + FPS", nullptr, nullptr);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);   // 关闭vsync，以看到真实FPS

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

    //------------------------
    // 初始化随机几千条线
    //------------------------
    std::vector<Line> lines;
    const int initialLines = 3000;
    lines.reserve(initialLines);

    for (int i = 0; i < initialLines; i++)
    {
        Line line{};
        line.x1 = ((rand() % 2000) / 1000.0f) - 1.0f;
        line.y1 = ((rand() % 2000) / 1000.0f) - 1.0f;
        line.x2 = ((rand() % 2000) / 1000.0f) - 1.0f;
        line.y2 = ((rand() % 2000) / 1000.0f) - 1.0f;
        lines.push_back(line);
    }

    //----------------------------------------------------------
    // VBO + EBO: 预分配大量空间，用 glBufferSubData 动态更新
    //----------------------------------------------------------
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    const int maxLines = 10000;
    const int maxVertices = maxLines * 2;
    const int maxIndices = maxLines * 2;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(float) * 2, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, maxIndices * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    //----------------
    // FPS 相关变量
    //----------------
    double dFpsTimer = glfwGetTime();
    int nFrameCount = 0;

    //----------------
    // 主循环
    //----------------
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        //------------------------------------------------------
        // 每帧随机更新部分线段
        //------------------------------------------------------
        int nUpdateLines = rand() % 50;  // 每帧随机更新 0~50 条
        for (int i = 0; i < nUpdateLines; i++)
        {
            if (lines.empty())
                break;

            int nIdx = rand() % lines.size();
            lines[nIdx].x1 = ((rand() % 2000) / 1000.0f) - 1.0f;
            lines[nIdx].y1 = ((rand() % 2000) / 1000.0f) - 1.0f;
            lines[nIdx].x2 = ((rand() % 2000) / 1000.0f) - 1.0f;
            lines[nIdx].y2 = ((rand() % 2000) / 1000.0f) - 1.0f;
        }

        //------------------------------------------------------
        // 隔一段时间：增加/删除线段
        //------------------------------------------------------
        static double opTimer = glfwGetTime();
        if (glfwGetTime() - opTimer > 1.0)
        {
            opTimer = glfwGetTime();

            int action = rand() % 2;

            if (action == 0 && lines.size() < maxLines)
            {
                Line line{};
                line.x1 = ((rand() % 2000) / 1000.0f) - 1.0f;
                line.y1 = ((rand() % 2000) / 1000.0f) - 1.0f;
                line.x2 = ((rand() % 2000) / 1000.0f) - 1.0f;
                line.y2 = ((rand() % 2000) / 1000.0f) - 1.0f;
                lines.push_back(line);
            }
            else if (!lines.empty())
            {
                int nIdx = rand() % lines.size();
                lines.erase(lines.begin() + nIdx);
            }
        }

        //------------------------------------------------------
        // 上传当前所有线的数据（顶点 + 索引）
        //------------------------------------------------------
        std::vector<float> vVertexData(lines.size() * 4);
        std::vector<unsigned int> vIndexData(lines.size() * 2);

        for (size_t i = 0; i < lines.size(); i++)
        {
            vVertexData[i * 4 + 0] = lines[i].x1;
            vVertexData[i * 4 + 1] = lines[i].y1;
            vVertexData[i * 4 + 2] = lines[i].x2;
            vVertexData[i * 4 + 3] = lines[i].y2;

            vIndexData[i * 2 + 0] = i * 2 + 0;
            vIndexData[i * 2 + 1] = i * 2 + 1;
        }

        glBufferSubData(GL_ARRAY_BUFFER, 0, vVertexData.size() * sizeof(float), vVertexData.data());
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, vIndexData.size() * sizeof(unsigned int), vIndexData.data());

        //------------------------------------------------------
        // 渲染
        //------------------------------------------------------
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_LINES, (int)vIndexData.size(), GL_UNSIGNED_INT, 0);

        //------------------------------------------------------
        // FPS 计算
        //------------------------------------------------------
        nFrameCount++;
        double currentTime = glfwGetTime();

        // 每 0.5 秒更新一次 FPS
        if (currentTime - dFpsTimer >= 0.5)
        {
            double fps = nFrameCount / (currentTime - dFpsTimer);
            dFpsTimer = currentTime;
            nFrameCount = 0;

            char title[128];
            sprintf(title, "Dynamic Lines + FPS | FPS: %.2f | Lines: %zu", fps, lines.size());
            glfwSetWindowTitle(window, title);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
