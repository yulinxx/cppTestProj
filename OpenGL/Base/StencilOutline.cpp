// 使用模板缓冲 (Stencil Buffer) 来实现物体轮廓描边的 OpenGL 3.3 示例程序模板。
// 模板缓冲常用于：物体描边、平面反射、阴影体等。

// 这个示例的核心思路是：

// 第一次绘制（写入模板）： 绘制待描边的物体，但只写入模板缓冲（设置模板值为 1），不写入颜色和深度。

// 第二次绘制（放大轮廓）： 关闭模板写入，启用模板测试。将模型放大一点，使用纯色绘制。只有模板值为 1 的像素（即第一次绘制的物体所在的区域）的相邻区域会被绘制成轮廓色。

// 第三次绘制（正常物体）： 关闭模板测试，正常绘制物体本身。

#include <iostream>
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// --- 配置 ---
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// --- 着色器代码 ---

// 顶点着色器 (通用着色器和轮廓着色器共享)
const char* vertexShaderSource = R"glsl(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)glsl";

// 正常绘制的片段着色器
const char* fragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

void main()
{
    // 蓝色
    FragColor = vec4(0.2f, 0.4f, 0.8f, 1.0f);
}
)glsl";

// 轮廓描边的片段着色器 (纯色，例如红色)
const char* outlineFragmentShaderSource = R"glsl(
#version 330 core
out vec4 FragColor;

void main()
{
    // 描边色：亮黄色/金色
    FragColor = vec4(1.0f, 0.8f, 0.0f, 1.0f);
}
)glsl";

// --- 实用函数：编译和链接着色器 ---

unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::COMPILATION_FAILED of type "
            << (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT") << "\n" << infoLog << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}

unsigned int createShaderProgram(const char* vsSource, const char* fsSource)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vsSource);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fsSource);

    if (vs == 0 || fs == 0) return 0;

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    char infoLog[512];
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        glDeleteProgram(program);
        return 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

// --- GLFW 回调函数和输入处理 ---

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// --- 核心程序逻辑 ---
int main()
{
    // 1. 初始化 GLFW 和窗口配置
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // **关键：请求模板缓冲位**
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Stencil Outline", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 2. 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    // 3. 配置全局 OpenGL 状态
    glEnable(GL_DEPTH_TEST);
    // **关键：启用模板测试**
    glEnable(GL_STENCIL_TEST);
    // 设置模板测试通过时的默认操作：保持当前值不变
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    // 4. 编译着色器程序
    unsigned int objectShader = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    unsigned int outlineShader = createShaderProgram(vertexShaderSource, outlineFragmentShaderSource);
    if (objectShader == 0 || outlineShader == 0)
    {
        glfwTerminate();
        return -1;
    }

    // 5. 准备几何体数据 (立方体)
    float vertices[] = {
        // positions
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f
    };

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // 顶点位置属性 (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 6. 准备矩阵
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -3.0f));

    // 获取 Uniform 位置
    glUseProgram(objectShader);
    glUniformMatrix4fv(glGetUniformLocation(objectShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(objectShader, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glUseProgram(outlineShader);
    glUniformMatrix4fv(glGetUniformLocation(outlineShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(outlineShader, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // 7. 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        // --- 每一帧的开始 ---
        // 确保模板缓冲是可写的 (重要：否则 glClearStencil 无效)
        glStencilMask(0xFF);
        // 清除颜色、深度和模板缓冲。模板缓冲清零。
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // 计算模型矩阵 (立方体随时间旋转)
        float currentFrame = (float)glfwGetTime();
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, currentFrame * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 originalModel = model; // 记录原始模型矩阵

        // --- 描边步骤 1: 写入模板缓冲 ---

        // 1.1. 设置模板写入和测试函数
        // glStencilFunc(GL_ALWAYS, 1, 0xFF) : 永远通过测试，将 Ref (参考值) 1 传入模板缓冲
        glStencilFunc(GL_ALWAYS, 1, 0xFF);
        // glStencilMask(0xFF) : 启用模板缓冲写入
        glStencilMask(0xFF);
        // glDepthMask(GL_TRUE) : 启用深度写入 (让物体参与深度测试，确保只有可见部分被写入模板)
        glDepthMask(GL_TRUE);
        // 1.2. 禁用颜色写入 (只写入模板和深度)
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // 1.3. 绘制物体
        glUseProgram(objectShader);
        glUniformMatrix4fv(glGetUniformLocation(objectShader, "model"), 1, GL_FALSE, glm::value_ptr(originalModel));
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- 描边步骤 2: 绘制轮廓 ---

        // 2.1. 设置模板读取和测试函数
        // glStencilFunc(GL_NOTEQUAL, 1, 0xFF) : 只有当模板值不等于 1 (即在物体外部) 时通过测试
        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
        // glStencilMask(0x00) : 禁用模板缓冲写入 (只进行测试和读取)
        glStencilMask(0x00);
        // 2.2. 启用颜色写入
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        // 2.3. 禁用深度测试，以确保轮廓在物体和其他物体前面（如果需要“穿墙”效果）
        glDisable(GL_DEPTH_TEST);

        // 2.4. 放大模型并绘制轮廓
        float scaleFactor = 1.05f; // 放大 5%
        model = glm::scale(originalModel, glm::vec3(scaleFactor, scaleFactor, scaleFactor));

        glUseProgram(outlineShader);
        glUniformMatrix4fv(glGetUniformLocation(outlineShader, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- 描边步骤 3: 正常绘制物体本身 ---

        // 3.1. 恢复深度测试和模板状态
        glStencilMask(0xFF);                       // 恢复模板写入掩码
        glEnable(GL_DEPTH_TEST);                   // 恢复深度测试
        glDepthMask(GL_TRUE);                      // 恢复深度写入

        // 3.2. 正常绘制物体 (使用原始模型矩阵)
        glUseProgram(objectShader);
        glUniformMatrix4fv(glGetUniformLocation(objectShader, "model"), 1, GL_FALSE, glm::value_ptr(originalModel));
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // --- 渲染循环结束 ---

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 8. 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(objectShader);
    glDeleteProgram(outlineShader);

    glfwTerminate();
    return 0;
}