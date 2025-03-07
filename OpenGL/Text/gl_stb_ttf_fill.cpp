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

// 文字填充
//
//以下是问题的可能原因和解决方法：
//

//GL_TRIANGLE_FAN 的局限性：
//当前的 renderFilledText 使用 GL_TRIANGLE_FAN 从轮廓的第一个点开始填充，但它假设轮廓是一个简单的闭合多边形，且没有内部孔洞。
// 对于有孔洞的字符（如 "O" 或 "e"），GL_TRIANGLE_FAN 会填充整个区域，包括孔洞内部，导致文字变得不可识别。
//轮廓数据未正确区分外部和内部轮廓：
//stb_truetype 的轮廓数据可能包含多个轮廓段（外部轮廓和内部孔洞），但当前代码没有区分这些轮廓，导致所有轮廓都被当成单个区域填充。
//顶点顺序或方向问题：
//如果轮廓的顶点顺序（顺时针或逆时针）不一致，可能会导致填充区域错误。OpenGL 根据顶点顺序（通过环绕规则）判断内部和外部区域。
//解决方法
//为了正确填充文字轮廓，同时处理外部轮廓和内部孔洞，我们需要：
//
//区分外部轮廓和内部孔洞：
//检查每个轮廓的顶点顺序（顺时针或逆时针），根据 OpenGL 的非零环绕规则（Non - Zero Winding Rule）或奇偶规则（Even - Odd Rule）确定填充区域。
//外部轮廓通常是逆时针，内部孔洞是顺时针（或相反，取决于字体数据）。
//使用三角形分解：
//将每个轮廓转换为三角形网格（三角剖分），然后用 GL_TRIANGLES 填充。可以使用简单的三角形生成方法（如耳分法）或引入专门的几何库（如 poly2tri）。
//改进填充逻辑：
//仅填充外部轮廓，跳过或反向填充内部孔洞。

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

// 片段着色器（填充颜色）
const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色填充
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

// 计算多边形的面积（用于判断顺时针/逆时针）
float calculateArea(const std::vector<float>& vertices)
{
    if (vertices.size() < 6) return 0.0f; // 至少需要 3 个点
    float area = 0.0f;
    for (size_t i = 0; i < vertices.size() - 2; i += 2)
    {
        float x1 = vertices[i];
        float y1 = vertices[i + 1];
        float x2 = vertices[(i + 2) % vertices.size()];
        float y2 = vertices[(i + 3) % vertices.size()];
        area += (x1 * y2 - x2 * y1);
    }
    return area / 2.0f;
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
// 渲染实心文字（修正轮廓方向，处理外部和内部）
void renderFilledText(stbtt_fontinfo* font, const std::string& text, float x, float y, float scale)
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
            if (outline.size() >= 6) // 至少需要 3 个点
            {
                std::vector<float> filledVertices = outline;
                filledVertices.push_back(outline[0]); // 闭合轮廓
                filledVertices.push_back(outline[1]);

                // 平移顶点
                for (size_t i = 0; i < filledVertices.size(); i += 2)
                {
                    filledVertices[i] += cursorX;
                    filledVertices[i + 1] += y;
                }

                // 判断轮廓方向
                float area = calculateArea(outline);
                bool isClockwise = (area < 0); // 面积负数表示顺时针

                // 假设外部轮廓为逆时针（面积正），内部孔洞为顺时针（面积负）
                if (isClockwise) // 逆时针轮廓（外部轮廓）填充
                {
                    // 这里应该使用三角形分解生成填充顶点
                    // 临时使用 GL_TRIANGLE_FAN 调试（需替换为三角剖分）
                    glBufferData(GL_ARRAY_BUFFER, filledVertices.size() * sizeof(float), filledVertices.data(), GL_STATIC_DRAW);
                    glDrawArrays(GL_TRIANGLE_FAN, 0, filledVertices.size() / 2);
                }
                // 顺时针轮廓（内部孔洞）不填充
            }
        }
        cursorX += xOffset; // 更新光标位置
    }
}

// 主函数
int main()
{
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Filled Text", NULL, NULL);
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
    auto fontBuffer = readFontFile("C:/Windows/Fonts/arial.ttf");
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

    // 主循环
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 黑色背景
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        renderFilledText(&font, "Hello, OpenGL!", 100.0f, 300.0f, 0.1f); // 调整 scale 和位置
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}