#include <glad/glad.h> 
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>         

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h" 


////////////////////////////////////////////////////////

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <fstream>

// 窗口大小
const int WIDTH = 800;
const int HEIGHT = 600;

// OpenGL 相关变量
GLuint shaderProgram, VAO, VBO;

// 读取字体文件
std::vector<unsigned char> readFontFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file)
    {
        std::cerr << "无法打开字体文件: " << filename << std::endl;
        exit(EXIT_FAILURE);
    }
    size_t size = file.tellg();
    std::vector<unsigned char> buffer(size);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

// 顶点着色器
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

// 片段着色器
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色轮廓
}
)";

// 编译着色器
GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cerr << "Shader Compilation Failed\n" << log << std::endl;
    }
    return shader;
}

// 初始化 OpenGL
void initOpenGL()
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// 将二次贝塞尔曲线采样为直线段
std::vector<float> tessellateQuadBezier(float x0, float y0, float x1, float y1, float x2, float y2, int segments)
{
    std::vector<float> vertices;
    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) / segments;
        float mt = 1.0f - t;
        float x = mt * mt * x0 + 2.0f * mt * t * x1 + t * t * x2;
        float y = mt * mt * y0 + 2.0f * mt * t * y1 + t * t * y2;
        vertices.push_back(x);
        vertices.push_back(y);
    }
    return vertices;
}

// 获取字符轮廓并生成顶点（支持多个轮廓段）
std::vector<std::vector<float>> getGlyphOutlines(stbtt_fontinfo* font, int codepoint, float scale, float& xOffset)
{
    std::vector<std::vector<float>> outlines;
    stbtt_vertex* v;
    int numVertices = stbtt_GetCodepointShape(font, codepoint, &v);

    std::vector<float> currentOutline;
    for (int i = 0; i < numVertices; ++i)
    {
        switch (v[i].type)
        {
        case STBTT_vmove: // 新的轮廓段开始
            if (!currentOutline.empty())
            {
                outlines.push_back(currentOutline);
                currentOutline.clear();
            }
            currentOutline.push_back(v[i].x * scale);
            currentOutline.push_back(v[i].y * scale);
            break;
        case STBTT_vline: // 直线
            currentOutline.push_back(v[i].x * scale);
            currentOutline.push_back(v[i].y * scale);
            break;
        case STBTT_vcurve: // 二次贝塞尔曲线
            float x0 = currentOutline[currentOutline.size() - 2];
            float y0 = currentOutline[currentOutline.size() - 1];
            float x1 = v[i].cx * scale;
            float y1 = v[i].cy * scale;
            float x2 = v[i].x * scale;
            float y2 = v[i].y * scale;
            auto curve = tessellateQuadBezier(x0, y0, x1, y1, x2, y2, 10);
            currentOutline.insert(currentOutline.end(), curve.begin() + 2, curve.end()); // 跳过起点
            break;
        }
    }
    if (!currentOutline.empty())
    {
        outlines.push_back(currentOutline);
    }
    stbtt_FreeShape(font, v);

    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(font, codepoint, &advanceWidth, &leftSideBearing);
    xOffset = advanceWidth * scale;

    return outlines;
}

// 渲染文字轮廓（支持多个轮廓段）
void renderTextOutline(stbtt_fontinfo* font, const std::string& text, float x, float y, float scale)
{
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    float cursorX = x;
    for (char c : text)
    {
        float xOffset;
        std::vector<std::vector<float>> outlines = getGlyphOutlines(font, c, scale, xOffset);

        for (const auto& outline : outlines)
        {
            if (!outline.empty())
            {
                std::vector<float> translatedOutline;
                for (size_t i = 0; i < outline.size(); i += 2)
                {
                    translatedOutline.push_back(outline[i] + cursorX);
                    translatedOutline.push_back(outline[i + 1] + y);
                }
                glBufferData(GL_ARRAY_BUFFER, translatedOutline.size() * sizeof(float), translatedOutline.data(), GL_STATIC_DRAW);
                glDrawArrays(GL_LINE_LOOP, 0, translatedOutline.size() / 2); // 闭合每个轮廓
            }
        }
        cursorX += xOffset; // 更新光标位置
    }
}

// 主函数
int main()
{
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Text Outline", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 初始化 OpenGL
    initOpenGL();

    // 创建 VAO 和 VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 加载字体
    auto fontBuffer = readFontFile("STHUPO.TTF");
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer.data(), stbtt_GetFontOffsetForIndex(fontBuffer.data(), 0)))
    {
        std::cerr << "Failed to initialize font!" << std::endl;
        return -1;
    }

    // 设置投影矩阵
    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // 设置线宽（可选）
    glLineWidth(2.0f);

    // 主循环
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 黑色背景
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        renderTextOutline(&font, "Hello, OpenGL!", 100.0f, 300.0f, 0.1f); // 调整 scale 和位置
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
