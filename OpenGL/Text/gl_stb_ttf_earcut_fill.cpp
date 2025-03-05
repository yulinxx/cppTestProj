#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <vector>
#include <array>
#include <fstream>
#include <cmath>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// 引入 earcut
#include "earcut.hpp"

//==========================================================


static const int WIDTH = 800;
static const int HEIGHT = 600;

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

// 顶点、片段着色器
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

GLuint compileShader(GLenum type, const char* src)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    // 省略错误检查
    return shader;
}

void initOpenGL()
{
    GLuint vShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vShader);
    glAttachShader(shaderProgram, fShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

// 计算多边形面积 (顺时针<0, 逆时针>0)
float computeArea(const std::vector<float>& poly)
{
    float area = 0.0f;
    size_t n = poly.size() / 2;
    for (size_t i = 0; i < n; ++i)
    {
        size_t j = (i + 1) % n;
        area += poly[2 * i] * poly[2 * j + 1] - poly[2 * j] * poly[2 * i + 1];
    }
    return 0.5f * area;
}

// 翻转多边形顶点顺序
void reversePolygon(std::vector<float>& poly)
{
    // poly 存储为 x0,y0, x1,y1, ...
    // 翻转顺序时需要对半交换
    size_t n = poly.size() / 2;
    for (size_t i = 0; i < n / 2; ++i)
    {
        size_t j = n - 1 - i;
        // 交换 (2*i, 2*i+1) 和 (2*j, 2*j+1)
        std::swap(poly[2 * i], poly[2 * j]);
        std::swap(poly[2 * i + 1], poly[2 * j + 1]);
    }
}

// 细分二次贝塞尔曲线
std::vector<float> tessellateQuadBezier(float x0, float y0, float x1, float y1, float x2, float y2, int segments)
{
    std::vector<float> result;
    for (int i = 0; i <= segments; ++i)
    {
        float t = (float)i / segments;
        float mt = 1.0f - t;
        float x = mt * mt * x0 + 2.0f * mt * t * x1 + t * t * x2;
        float y = mt * mt * y0 + 2.0f * mt * t * y1 + t * t * y2;
        result.push_back(x);
        result.push_back(y);
    }
    return result;
}

// 获取字符轮廓 (STB)
std::vector<std::vector<float>> getGlyphOutlines(stbtt_fontinfo* font, int codepoint, float scale, float& xOffset)
{
    std::vector<std::vector<float>> outlines;
    stbtt_vertex* v;
    int numVerts = stbtt_GetCodepointShape(font, codepoint, &v);
    std::vector<float> current;

    for (int i = 0; i < numVerts; ++i)
    {
        switch (v[i].type)
        {
        case STBTT_vmove:
            if (!current.empty())
            {
                outlines.push_back(current);
                current.clear();
            }
            current.push_back(v[i].x * scale);
            current.push_back(v[i].y * scale);
            break;
        case STBTT_vline:
            current.push_back(v[i].x * scale);
            current.push_back(v[i].y * scale);
            break;
        case STBTT_vcurve:
        {
            float x0 = current[current.size() - 2];
            float y0 = current[current.size() - 1];
            float x1 = v[i].cx * scale;
            float y1 = v[i].cy * scale;
            float x2 = v[i].x * scale;
            float y2 = v[i].y * scale;
            auto curve = tessellateQuadBezier(x0, y0, x1, y1, x2, y2, 10);
            // curve 第一个点与 current 最后一个点重叠，故跳过 curve 的前 2 个 float
            current.insert(current.end(), curve.begin() + 2, curve.end());
            break;
        }
        }
    }
    if (!current.empty()) outlines.push_back(current);
    stbtt_FreeShape(font, v);

    int advanceWidth, leftSideBearing;
    stbtt_GetCodepointHMetrics(font, codepoint, &advanceWidth, &leftSideBearing);
    xOffset = advanceWidth * scale;
    return outlines;
}

// 使用 earcut 对“单个轮廓”做三角剖分
void triangulateSingleOutline(const std::vector<float>& outline, std::vector<float>& outVertices)
{
    // earcut 的输入必须是 2D 点的多重多边形: std::vector<std::vector<std::array<T,2>>>
    // 这里我们只做“单个外轮廓”，不考虑孔洞 => 只有一层 vector
    std::vector<std::array<float, 2>> polygon;
    polygon.reserve(outline.size() / 2);
    for (size_t i = 0; i < outline.size(); i += 2)
    {
        polygon.push_back({ outline[i], outline[i + 1] });
    }

    std::vector<std::vector<std::array<float, 2>>> polygonSet;
    polygonSet.push_back(polygon);

    // earcut 返回索引序列
    auto indices = mapbox::earcut<unsigned int>(polygonSet);

    // 根据索引序列构造最终顶点
    for (auto idx : indices)
    {
        outVertices.push_back(polygon[idx][0]);
        outVertices.push_back(polygon[idx][1]);
    }
}

// 绘制单个字符
void renderGlyph(stbtt_fontinfo* font, int codepoint, float& cursorX, float baseY, float scale)
{
    float xOffset;
    auto outlines = getGlyphOutlines(font, codepoint, scale, xOffset);

    // 将每个 outline 强制翻转为 CCW
    // 然后分别进行 earcut
    for (auto& outline : outlines)
    {
        if (outline.size() < 6) continue; // 至少要3个点
        float area = computeArea(outline);
        // 如果面积 < 0 => 翻转它
        if (area < 0)
        {
            reversePolygon(outline);
        }
        // 对 outline 做三角剖分
        // 先把它平移到 (cursorX, baseY)
        for (size_t i = 0; i < outline.size(); i += 2)
        {
            outline[i] += cursorX;
            outline[i + 1] += baseY;
        }

        // earcut
        std::vector<float> triVerts;
        triangulateSingleOutline(outline, triVerts);

        // 上传数据并绘制
        glBufferData(GL_ARRAY_BUFFER, triVerts.size() * sizeof(float), triVerts.data(), GL_STATIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, triVerts.size() / 2);
    }

    // 移动光标
    cursorX += xOffset;
}

// 渲染字符串
void renderFilledText(stbtt_fontinfo* font, const std::string& text, float x, float y, float scale)
{
    float cursorX = x;
    for (char c : text)
    {
        renderGlyph(font, static_cast<int>(c), cursorX, y, scale);
    }
}

int main()
{
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Earcut Text", nullptr, nullptr);
        if (!window) return -1;
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to init GLAD" << std::endl;
            return -1;
    }

    initOpenGL();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 加载字体
    auto fontBuf = readFontFile("STHUPO.TTF");
        stbtt_fontinfo font;
    if (!stbtt_InitFont(&font, fontBuf.data(), stbtt_GetFontOffsetForIndex(fontBuf.data(), 0)))
    {
        std::cerr << "Failed to init font" << std::endl;
            return -1;
    }

    // 设置正交投影
    glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);
    glUseProgram(shaderProgram);
    GLint loc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));

    glClearColor(0, 0, 0, 1);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // 试着渲染字符串
        renderFilledText(&font, "Hello, OpenGL!", 10.0f, 500.0f, 0.08f);
        renderFilledText(&font, "ABCDEFGHIJKLMN", 10.0f, 350.0f, 0.08f);
        renderFilledText(&font, "OPQRTSUVWXYZ", 10.0f, 150.0f, 0.08f);
        renderFilledText(&font, "~+-*/!@#$%^&*(){}", 10.0f, 50.0f, 0.08f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

