// 加载和显示图片，进行纹理贴图

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // 需要 stb_image.h 来加载图片

// 顶点着色器
const char* vs = R"(
    #version 330 core
    layout(location = 0) in vec2 aPos;
    layout(location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    uniform mat4 projection;
    uniform mat4 model;
    void main() {
        gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
        TexCoord = aTexCoord;
    }
)";

// 片段着色器
const char* fs = R"(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texture1;
    void main() {
        FragColor = texture(texture1, TexCoord);
    }
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL Texture", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化 GLEW
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

    // 编译着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 定义矩形顶点数据，包含纹理坐标
    float fixedRect[] = {
        // 位置          // 纹理坐标
        -0.1f, -0.1f,  0.0f, 0.0f, // 左下
         0.1f, -0.1f,  1.0f, 0.0f, // 右下
         0.1f,  0.1f,  1.0f, 1.0f, // 右上
        -0.1f,  0.1f,  0.0f, 1.0f  // 左上
    };

    float scaledRect[] = {
        -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.0f, 1.0f
    };

    GLuint VAOs[2], VBOs[2];
    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    // 固定大小矩形的 VAO 和 VBO
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fixedRect), fixedRect, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 缩放矩形的 VAO 和 VBO
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(scaledRect), scaledRect, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    // 加载纹理
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    // 加载纹理图片，使用 stb_image 库的 stbi_load 函数
    // 第一个参数为图片文件的路径，这里需要根据实际情况修改
    // 第二、三个参数分别用于存储图片的宽度和高度
    // 第四个参数用于存储图片的通道数，如 RGB 为 3，RGBA 为 4
    // 最后一个参数为要求的通道数，设为 0 表示使用图片原始的通道数
    unsigned char* data = stbi_load("D:/xx/Pictures/34.png", &width, &height, &nrChannels, 0);
    // 检查图片是否成功加载
    if (data)
    {
        // 根据图片的通道数，选择合适的纹理格式进行加载
        if (nrChannels == 3)
        {
            // 如果是 RGB 格式（3 通道），使用 GL_RGB 格式加载纹理
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else if (nrChannels == 4)
        {
            // 如果是 RGBA 格式（4 通道），使用 GL_RGBA 格式加载纹理
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            // 如果通道数不是 3 或 4，输出错误信息，提示不支持该通道数
            std::cerr << "Unsupported number of channels: " << nrChannels << std::endl;
        }
        // 生成纹理的多级渐远纹理（Mipmap），提高纹理在不同距离下的显示效果
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        // 如果图片加载失败，输出错误信息
        std::cerr << "Failed to load texture" << std::endl;
    }
    // 释放图片数据，避免内存泄漏
    stbi_image_free(data);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindTexture(GL_TEXTURE_2D, texture);

        // 正交投影矩阵
        float left = -width / 200.0f;
        float right = width / 200.0f;
        float bottom = -height / 200.0f;
        float top = height / 200.0f;
        float ortho[16] = {
            2.0f / (right - left), 0, 0, -(right + left) / (right - left),
            0, 2.0f / (top - bottom), 0, -(top + bottom) / (top - bottom),
            0, 0, -1, 0,
            0, 0, 0, 1
        };

        GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, ortho);

        // 固定大小的矩形
        float fixedModel[16] = {
            200.0f / width, 0, 0, 0,
            0, 200.0f / height, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, fixedModel);
        glBindVertexArray(VAOs[0]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        // 缩放的矩形
        float scaledModel[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, scaledModel);
        glBindVertexArray(VAOs[1]);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteTextures(1, &texture);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}