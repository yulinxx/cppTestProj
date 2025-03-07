#include <glad/glad.h>  // 包含 GLAD 库，用于加载 OpenGL 函数指针
#include <GLFW/glfw3.h> // 包含 GLFW 库，用于创建窗口和处理输入

#include <glm/glm.hpp>  // 包含 GLM 库，用于处理向量和矩阵运算
#include <glm/gtc/matrix_transform.hpp> // 包含 GLM 的矩阵变换功能
#include <glm/gtc/type_ptr.hpp> // 包含 GLM 的类型转换功能

#include <stdio.h>      // 标准输入输出库
#include <stdlib.h>     // 标准库，包含一些常用函数，如 exit
#include <string.h>     // 字符串处理库

#define STB_TRUETYPE_IMPLEMENTATION // 定义宏，用于实现 STB 字体库
#include "stb_truetype.h"          // 包含 STB 字体库

////////////////////////////////////////////////////////

#include <glad/glad.h>  // 再次包含 GLAD 库
#include <GLFW/glfw3.h> // 再次包含 GLFW 库
#include <glm/glm.hpp>  // 再次包含 GLM 库
#include <glm/gtc/matrix_transform.hpp> // 再次包含 GLM 的矩阵变换功能
#include <glm/gtc/type_ptr.hpp> // 再次包含 GLM 的类型转换功能
#include <iostream>     // 标准输入输出流库
#include <vector>       // 动态数组容器库
#include <fstream>      // 文件输入输出流库

// 窗口大小
const int WIDTH = 800;
const int HEIGHT = 600;

// OpenGL 相关变量
GLuint shaderProgram, VAO, VBO;

/**
 * @brief 读取字体文件并返回文件内容的字节向量
 * 
 * 该函数打开指定的字体文件，以二进制模式读取其内容，并将内容存储在一个字节向量中。
 * 如果文件打开失败，函数将输出错误信息并终止程序。
 * 
 * @param filename 字体文件的路径
 * @return std::vector<unsigned char> 包含字体文件内容的字节向量
 */
std::vector<unsigned char> readFontFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // 以二进制模式打开文件，并定位到文件末尾
    if (!file)
    {
        std::cerr << "无法打开字体文件: " << filename << std::endl; // 输出错误信息
        exit(EXIT_FAILURE); // 终止程序
    }
    size_t size = file.tellg(); // 获取文件大小
    std::vector<unsigned char> buffer(size); // 创建一个大小为文件大小的字节向量
    file.seekg(0); // 将文件指针移到文件开头
    file.read(reinterpret_cast<char*>(buffer.data()), size); // 读取文件内容到字节向量
    return buffer; // 返回包含文件内容的字节向量
}

// 顶点着色器代码
const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos; // 输入顶点位置
uniform mat4 projection; // 投影矩阵
void main() {
    gl_Position = projection * vec4(aPos, 0.0, 1.0); // 将顶点位置乘以投影矩阵得到裁剪空间坐标
}
)";

// 片段着色器代码
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor; // 输出片段颜色
void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色轮廓
}
)";

/**
 * @brief 编译着色器
 * 
 * 该函数根据指定的着色器类型（顶点着色器或片段着色器）和着色器源代码编译着色器。
 * 如果编译失败，函数将输出错误信息。
 * 
 * @param type 着色器类型（GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER）
 * @param source 着色器源代码
 * @return GLuint 编译后的着色器对象 ID
 */
GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type); // 创建着色器对象
    glShaderSource(shader, 1, &source, NULL); // 设置着色器源代码
    glCompileShader(shader); // 编译着色器
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success); // 检查编译状态
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log); // 获取编译错误信息
        std::cerr << "Shader Compilation Failed\n" << log << std::endl; // 输出错误信息
    }
    return shader; // 返回编译后的着色器对象 ID
}

/**
 * @brief 初始化 OpenGL 环境
 * 
 * 该函数编译顶点着色器和片段着色器，并将它们链接到一个着色器程序中。
 * 最后删除不再需要的着色器对象。
 */
void initOpenGL()
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource); // 编译顶点着色器
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource); // 编译片段着色器

    shaderProgram = glCreateProgram(); // 创建着色器程序
    glAttachShader(shaderProgram, vertexShader); // 将顶点着色器附加到着色器程序
    glAttachShader(shaderProgram, fragmentShader); // 将片段着色器附加到着色器程序
    glLinkProgram(shaderProgram); // 链接着色器程序

    glDeleteShader(vertexShader); // 删除顶点着色器
    glDeleteShader(fragmentShader); // 删除片段着色器
}

/**
 * @brief 将二次贝塞尔曲线采样为直线段
 * 
 * 该函数将二次贝塞尔曲线采样为指定数量的直线段，并返回这些直线段的顶点坐标。
 * 
 * @param x0 起始点的 x 坐标
 * @param y0 起始点的 y 坐标
 * @param x1 控制点的 x 坐标
 * @param y1 控制点的 y 坐标
 * @param x2 结束点的 x 坐标
 * @param y2 结束点的 y 坐标
 * @param segments 采样的直线段数量
 * @return std::vector<float> 包含采样后直线段顶点坐标的向量
 */
std::vector<float> tessellateQuadBezier(float x0, float y0, float x1, float y1, float x2, float y2, int segments)
{
    std::vector<float> vertices; // 存储顶点坐标的向量
    for (int i = 0; i <= segments; ++i)
    {
        float t = static_cast<float>(i) / segments; // 计算参数 t
        float mt = 1.0f - t; // 计算 1 - t
        float x = mt * mt * x0 + 2.0f * mt * t * x1 + t * t * x2; // 计算 x 坐标
        float y = mt * mt * y0 + 2.0f * mt * t * y1 + t * t * y2; // 计算 y 坐标
        vertices.push_back(x); // 将 x 坐标添加到向量中
        vertices.push_back(y); // 将 y 坐标添加到向量中
    }
    return vertices; // 返回包含顶点坐标的向量
}

/**
 * @brief 获取字符轮廓并生成顶点（支持多个轮廓段）
 * 
 * 该函数根据指定的字体信息、字符编码点、缩放比例和偏移量，获取字符的轮廓并生成顶点坐标。
 * 支持多个轮廓段，每个轮廓段由一组顶点坐标表示。
 * 
 * @param font 字体信息结构体指针
 * @param codepoint 字符编码点
 * @param scale 缩放比例
 * @param xOffset 字符的水平偏移量
 * @return std::vector<std::vector<float>> 包含字符轮廓顶点坐标的二维向量
 */
std::vector<std::vector<float>> getGlyphOutlines(stbtt_fontinfo* font, int codepoint, float scale, float& xOffset)
{
    std::vector<std::vector<float>> outlines; // 存储字符轮廓的二维向量
    stbtt_vertex* v;
    int numVertices = stbtt_GetCodepointShape(font, codepoint, &v); // 获取字符的顶点信息

    std::vector<float> currentOutline; // 存储当前轮廓段的顶点坐标
    for (int i = 0; i < numVertices; ++i)
    {
        switch (v[i].type)
        {
        case STBTT_vmove: // 新的轮廓段开始
            if (!currentOutline.empty())
            {
                outlines.push_back(currentOutline); // 将当前轮廓段添加到轮廓列表中
                currentOutline.clear(); // 清空当前轮廓段
            }
            currentOutline.push_back(v[i].x * scale); // 添加起始点的 x 坐标
            currentOutline.push_back(v[i].y * scale); // 添加起始点的 y 坐标
            break;
        case STBTT_vline: // 直线
            currentOutline.push_back(v[i].x * scale); // 添加直线终点的 x 坐标
            currentOutline.push_back(v[i].y * scale); // 添加直线终点的 y 坐标
            break;
        case STBTT_vcurve: // 二次贝塞尔曲线
            float x0 = currentOutline[currentOutline.size() - 2]; // 获取曲线起始点的 x 坐标
            float y0 = currentOutline[currentOutline.size() - 1]; // 获取曲线起始点的 y 坐标
            float x1 = v[i].cx * scale; // 获取控制点的 x 坐标
            float y1 = v[i].cy * scale; // 获取控制点的 y 坐标
            float x2 = v[i].x * scale; // 获取曲线终点的 x 坐标
            float y2 = v[i].y * scale; // 获取曲线终点的 y 坐标
            auto curve = tessellateQuadBezier(x0, y0, x1, y1, x2, y2, 10); // 将曲线采样为直线段
            currentOutline.insert(currentOutline.end(), curve.begin() + 2, curve.end()); // 跳过起点，将采样后的顶点添加到当前轮廓段
            break;
        }
    }
    if (!currentOutline.empty())
    {
        outlines.push_back(currentOutline); // 将最后一个轮廓段添加到轮廓列表中
    }
    stbtt_FreeShape(font, v); // 释放顶点信息

    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(font, codepoint, &advanceWidth, &leftSideBearing); // 获取字符的水平度量信息
    xOffset = advanceWidth * scale; // 计算字符的水平偏移量

    return outlines; // 返回包含字符轮廓顶点坐标的二维向量
}

/**
 * @brief 渲染文字轮廓（支持多个轮廓段）
 * 
 * 该函数根据指定的字体信息、文本内容、起始位置和缩放比例，渲染文字的轮廓。
 * 支持多个轮廓段，每个轮廓段由一组顶点坐标表示。
 * 
 * @param font 字体信息结构体指针
 * @param text 要渲染的文本内容
 * @param x 起始位置的 x 坐标
 * @param y 起始位置的 y 坐标
 * @param scale 缩放比例
 */
void renderTextOutline(stbtt_fontinfo* font, const std::string& text, float x, float y, float scale)
{
    glUseProgram(shaderProgram); // 使用着色器程序
    glBindVertexArray(VAO); // 绑定顶点数组对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // 绑定顶点缓冲对象

    float cursorX = x; // 初始化光标位置
    for (char c : text)
    {
        float xOffset;
        std::vector<std::vector<float>> outlines = getGlyphOutlines(font, c, scale, xOffset); // 获取字符的轮廓顶点坐标

        for (const auto& outline : outlines)
        {
            if (!outline.empty())
            {
                std::vector<float> translatedOutline; // 存储平移后的顶点坐标
                for (size_t i = 0; i < outline.size(); i += 2)
                {
                    translatedOutline.push_back(outline[i] + cursorX); // 平移 x 坐标
                    translatedOutline.push_back(outline[i + 1] + y); // 平移 y 坐标
                }
                glBufferData(GL_ARRAY_BUFFER, translatedOutline.size() * sizeof(float), translatedOutline.data(), GL_STATIC_DRAW); // 将平移后的顶点坐标上传到缓冲对象
                glDrawArrays(GL_LINE_LOOP, 0, translatedOutline.size() / 2); // 绘制闭合轮廓
            }
        }
        cursorX += xOffset; // 更新光标位置
    }
}

// 主函数
int main()
{
    if (!glfwInit()) return -1; // 初始化 GLFW，如果失败则返回 -1
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Text Outline", NULL, NULL); // 创建窗口
    if (!window) return -1; // 如果窗口创建失败则返回 -1
    glfwMakeContextCurrent(window); // 将窗口的上下文设置为当前上下文
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress); // 加载 OpenGL 函数指针

    // 初始化 OpenGL
    initOpenGL();

    // 创建 VAO 和 VBO
    glGenVertexArrays(1, &VAO); // 生成顶点数组对象
    glGenBuffers(1, &VBO); // 生成顶点缓冲对象
    glBindVertexArray(VAO); // 绑定顶点数组对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO); // 绑定顶点缓冲对象
    glEnableVertexAttribArray(0); // 启用顶点属性数组
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0); // 设置顶点属性指针

    // 加载字体
    auto fontBuffer = readFontFile("C:/Windows/Fonts/arial.ttf"); // 读取字体文件
    stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuffer.data(), stbtt_GetFontOffsetForIndex(fontBuffer.data(), 0)))
    {
        std::cerr << "Failed to initialize font!" << std::endl; // 输出字体初始化失败信息
        return -1; // 返回 -1
    }

    // 设置投影矩阵
    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT); // 创建正交投影矩阵
    glUseProgram(shaderProgram); // 使用着色器程序
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection)); // 设置投影矩阵

    // 设置线宽（可选）
    glLineWidth(2.0f);

    // 主循环
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 设置清屏颜色为黑色
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT); // 清除颜色缓冲区
        renderTextOutline(&font, "Hello, OpenGL!", 100.0f, 500.0f, 0.08f); // 渲染文本轮廓
        renderTextOutline(&font, "ABCDEFGHIJKLMN!", 100.0f, 300.0f, 0.08f);
        renderTextOutline(&font, "OPQRSTUVWXYZ!", 100.0f, 200.0f, 0.08f);
        renderTextOutline(&font, "~!@#$%^&*()_+abcde", 100.0f, 100.0f, 0.06f);
        glfwSwapBuffers(window); // 交换前后缓冲区
        glfwPollEvents(); // 处理事件
    }

    glfwTerminate(); // 终止 GLFW
    return 0; // 返回 0
}