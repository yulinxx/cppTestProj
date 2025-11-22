// 引入 GLAD 库,用于管理 OpenGL 函数指针
#include <glad/glad.h>
// 引入 GLFW 库,用于创建窗口和处理输入事件
#include <GLFW/glfw3.h>

// 引入 GLM 库,用于进行向量和矩阵运算
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 引入标准输入输出库
#include <iostream>
// 引入标准向量容器
#include <vector>
// 引入标准数组容器
#include <array>
// 引入文件流库,用于文件操作
#include <fstream>
// 引入数学库,用于数学计算
#include <cmath>

// 实现 STB 字体库
#define STB_TRUETYPE_IMPLEMENTATION
// 引入 STB 字体库头文件
#include "stb_truetype.h"

// 引入 earcut 库,用于多边形三角剖分
#include "earcut.hpp"

//==========================================================

// 定义窗口的宽度
static const int WIDTH = 800;
// 定义窗口的高度
static const int HEIGHT = 600;

// 定义着色器程序 ID
GLuint shaderProgram,
// 定义顶点数组对象 ID
VAO,
// 定义顶点缓冲对象 ID
VBO;

/**
 * @brief 读取字体文件并将其内容存储在向量中.
 *
 * 该函数以二进制模式打开指定的字体文件,并将文件内容读取到一个无符号字符向量中.
 * 如果文件打开失败,将输出错误信息并终止程序.
 *
 * @param filename 字体文件的路径和文件名.
 * @return std::vector<unsigned char> 包含字体文件内容的向量.
 */
std::vector<unsigned char> readFontFile(const std::string& filename)
{
    // 以二进制模式和文件末尾模式打开文件
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    // 检查文件是否成功打开
    if (!file)
    {
        // 输出错误信息
        std::cerr << "Failed to open font file: " << filename << std::endl;
        // 终止程序
        exit(EXIT_FAILURE);
    }
    // 获取文件大小
    size_t size = file.tellg();
    // 创建一个无符号字符向量,用于存储文件内容
    std::vector<unsigned char> buffer(size);
    // 将文件指针移动到文件开头
    file.seekg(0);
    // 读取文件内容到向量中
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    // 返回包含文件内容的向量
    return buffer;
}

// 顶点着色器源代码
const char* vs = R"(
#version 330 core
// 定义顶点属性,位置为 0 的二维向量
layout(location = 0) in vec2 aPos;
// 定义投影矩阵统一变量
uniform mat4 projection;
void main()
{
    // 计算顶点在裁剪空间中的位置
    gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

// 片段着色器源代码
const char* fs = R"(
#version 330 core
// 定义片段着色器的输出颜色
out vec4 FragColor;
void main()
{
    // 设置片段颜色为白色
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
)";

/**
 * @brief 编译指定类型的着色器.
 *
 * 该函数创建一个指定类型的着色器对象,并将源代码加载到该对象中进行编译.
 * 目前省略了错误检查.
 *
 * @param type 着色器类型,如 GL_VERTEX_SHADER 或 GL_FRAGMENT_SHADER.
 * @param src 着色器源代码.
 * @return GLuint 编译后的着色器对象 ID.
 */
GLuint compileShader(GLenum type, const char* src)
{
    // 创建一个指定类型的着色器对象
    GLuint shader = glCreateShader(type);
    // 将源代码加载到着色器对象中
    glShaderSource(shader, 1, &src, nullptr);
    // 编译着色器
    glCompileShader(shader);
    // 省略错误检查
    // 返回编译后的着色器对象 ID
    return shader;
}

/**
 * @brief 初始化 OpenGL 着色器程序.
 *
 * 该函数编译顶点着色器和片段着色器,并将它们链接到一个着色器程序中.
 * 编译和链接完成后,删除临时的着色器对象.
 */
void initOpenGL()
{
    // 编译顶点着色器
    GLuint vShader = compileShader(GL_VERTEX_SHADER, vs);
    // 编译片段着色器
    GLuint fShader = compileShader(GL_FRAGMENT_SHADER, fs);
    // 创建一个着色器程序对象
    shaderProgram = glCreateProgram();
    // 将顶点着色器附加到着色器程序中
    glAttachShader(shaderProgram, vShader);
    // 将片段着色器附加到着色器程序中
    glAttachShader(shaderProgram, fShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);
    // 删除顶点着色器对象
    glDeleteShader(vShader);
    // 删除片段着色器对象
    glDeleteShader(fShader);
}

/**
 * @brief 计算多边形的面积.
 *
 * 该函数根据多边形的顶点坐标计算其面积.面积的正负表示多边形的顶点顺序:
 * 顺时针顺序为负,逆时针顺序为正.
 *
 * @param poly 包含多边形顶点坐标的向量,格式为 [x0, y0, x1, y1, ...].
 * @return float 多边形的面积.
 */
float computeArea(const std::vector<float>& poly)
{
    // 初始化面积为 0
    float area = 0.0f;
    // 计算多边形的顶点数量
    size_t n = poly.size() / 2;
    // 遍历多边形的每个顶点
    for (size_t i = 0; i < n; ++i)
    {
        // 计算下一个顶点的索引
        size_t j = (i + 1) % n;
        // 累加面积
        area += poly[2 * i] * poly[2 * j + 1] - poly[2 * j] * poly[2 * i + 1];
    }
    // 返回面积的一半
    return 0.5f * area;
}

/**
 * @brief 翻转多边形的顶点顺序.
 *
 * 该函数将多边形的顶点顺序从顺时针翻转到逆时针,或从逆时针翻转到顺时针.
 * 顶点坐标存储在向量中,格式为 [x0, y0, x1, y1, ...].
 *
 * @param poly 包含多边形顶点坐标的向量.
 */
void reversePolygon(std::vector<float>& poly)
{
    // poly 存储为 x0,y0, x1,y1, ...
    // 翻转顺序时需要对半交换
    // 计算多边形的顶点数量
    size_t n = poly.size() / 2;
    // 遍历多边形的前半部分顶点
    for (size_t i = 0; i < n / 2; ++i)
    {
        // 计算对称顶点的索引
        size_t j = n - 1 - i;
        // 交换当前顶点和对称顶点的 x 坐标
        std::swap(poly[2 * i], poly[2 * j]);
        // 交换当前顶点和对称顶点的 y 坐标
        std::swap(poly[2 * i + 1], poly[2 * j + 1]);
    }
}

/**
 * @brief 细分二次贝塞尔曲线.
 *
 * 该函数将二次贝塞尔曲线细分为多个线段,并返回细分后的顶点坐标.
 *
 * @param x0 起始点的 x 坐标.
 * @param y0 起始点的 y 坐标.
 * @param x1 控制点的 x 坐标.
 * @param y1 控制点的 y 坐标.
 * @param x2 结束点的 x 坐标.
 * @param y2 结束点的 y 坐标.
 * @param segments 细分的线段数量.
 * @return std::vector<float> 细分后的顶点坐标向量,格式为 [x0, y0, x1, y1, ...].
 */
std::vector<float> tessellateQuadBezier(float x0, float y0, float x1, float y1, float x2, float y2, int segments)
{
    // 创建一个向量,用于存储细分后的顶点坐标
    std::vector<float> result;
    // 遍历细分的线段数量
    for (int i = 0; i <= segments; ++i)
    {
        // 计算当前细分点的参数 t
        float t = (float)i / segments;
        // 计算 1 - t
        float mt = 1.0f - t;
        // 计算当前细分点的 x 坐标
        float x = mt * mt * x0 + 2.0f * mt * t * x1 + t * t * x2;
        // 计算当前细分点的 y 坐标
        float y = mt * mt * y0 + 2.0f * mt * t * y1 + t * t * y2;
        // 将当前细分点的 x 坐标添加到结果向量中
        result.push_back(x);
        // 将当前细分点的 y 坐标添加到结果向量中
        result.push_back(y);
    }
    // 返回细分后的顶点坐标向量
    return result;
}

/**
 * @brief 获取字符的轮廓.
 *
 * 该函数使用 STB 字体库获取指定字符的轮廓,并将轮廓存储在一个二维向量中.
 * 同时,计算字符的水平偏移量.
 *
 * @param font 字体信息结构体指针.
 * @param codepoint 字符的 Unicode 码点.
 * @param scale 字体缩放比例.
 * @param xOffset 字符的水平偏移量.
 * @return std::vector<std::vector<float>> 包含字符轮廓的二维向量,每个子向量表示一个轮廓.
 */
std::vector<std::vector<float>> getGlyphOutlines(stbtt_fontinfo* font, int codepoint, float scale, float& xOffset)
{
    // 创建一个二维向量,用于存储字符的轮廓
    std::vector<std::vector<float>> outlines;
    // 定义一个指向 STB 顶点结构体的指针
    stbtt_vertex* v;
    // 获取指定字符的顶点数量
    int numVerts = stbtt_GetCodepointShape(font, codepoint, &v);
    // 创建一个向量,用于存储当前轮廓的顶点坐标
    std::vector<float> current;

    // 遍历字符的每个顶点
    for (int i = 0; i < numVerts; ++i)
    {
        // 根据顶点类型进行不同的处理
        switch (v[i].type)
        {
            // 处理移动到新点的操作
        case STBTT_vmove:
            // 如果当前轮廓不为空
            if (!current.empty())
            {
                // 将当前轮廓添加到轮廓向量中
                outlines.push_back(current);
                // 清空当前轮廓
                current.clear();
            }
            // 将新点的 x 坐标添加到当前轮廓中
            current.push_back(v[i].x * scale);
            // 将新点的 y 坐标添加到当前轮廓中
            current.push_back(v[i].y * scale);
            break;
            // 处理直线段的操作
        case STBTT_vline:
            // 将直线段终点的 x 坐标添加到当前轮廓中
            current.push_back(v[i].x * scale);
            // 将直线段终点的 y 坐标添加到当前轮廓中
            current.push_back(v[i].y * scale);
            break;
            // 处理二次贝塞尔曲线的操作
        case STBTT_vcurve:
        {
            // 获取当前轮廓的最后一个点的 x 坐标
            float x0 = current[current.size() - 2];
            // 获取当前轮廓的最后一个点的 y 坐标
            float y0 = current[current.size() - 1];
            // 获取控制点的 x 坐标
            float x1 = v[i].cx * scale;
            // 获取控制点的 y 坐标
            float y1 = v[i].cy * scale;
            // 获取曲线终点的 x 坐标
            float x2 = v[i].x * scale;
            // 获取曲线终点的 y 坐标
            float y2 = v[i].y * scale;
            // 细分二次贝塞尔曲线
            auto curve = tessellateQuadBezier(x0, y0, x1, y1, x2, y2, 10);
            // curve 第一个点与 current 最后一个点重叠,故跳过 curve 的前 2 个 float
            // 将细分后的曲线顶点添加到当前轮廓中
            current.insert(current.end(), curve.begin() + 2, curve.end());
            break;
        }
        }
    }
    // 如果当前轮廓不为空
    if (!current.empty())
        // 将当前轮廓添加到轮廓向量中
        outlines.push_back(current);
    // 释放 STB 顶点结构体的内存
    stbtt_FreeShape(font, v);

    // 定义字符的前进宽度和左侧边距
    int advanceWidth, leftSideBearing;
    // 获取指定字符的水平度量信息
    stbtt_GetCodepointHMetrics(font, codepoint, &advanceWidth, &leftSideBearing);
    // 计算字符的水平偏移量
    xOffset = advanceWidth * scale;
    // 返回包含字符轮廓的二维向量
    return outlines;
}

/**
 * @brief 对单个轮廓进行三角剖分.
 *
 * 该函数使用 earcut 库对单个轮廓进行三角剖分,并将三角剖分后的顶点坐标存储在输出向量中.
 *
 * @param outline 包含单个轮廓顶点坐标的向量,格式为 [x0, y0, x1, y1, ...].
 * @param outVertices 存储三角剖分后顶点坐标的输出向量.
 */
void triangulateSingleOutline(const std::vector<float>& outline, std::vector<float>& outVertices)
{
    // earcut 的输入必须是 2D 点的多重多边形: std::vector<std::vector<std::array<T,2>>>
    // 这里我们只做“单个外轮廓”,不考虑孔洞 => 只有一层 vector
    // 创建一个二维数组向量,用于存储轮廓的顶点
    std::vector<std::array<float, 2>> polygon;
    // 预分配内存,提高性能
    polygon.reserve(outline.size() / 2);
    // 遍历轮廓的顶点坐标
    for (size_t i = 0; i < outline.size(); i += 2)
    {
        // 将当前顶点的 x 和 y 坐标存储在一个二维数组中
        polygon.push_back({ outline[i], outline[i + 1] });
    }

    // 创建一个二维数组向量的向量,用于存储多重多边形
    std::vector<std::vector<std::array<float, 2>>> polygonSet;
    // 将单个轮廓添加到多重多边形向量中
    polygonSet.push_back(polygon);

    // 使用 earcut 库进行三角剖分,返回索引序列
    auto indices = mapbox::earcut<unsigned int>(polygonSet);

    // 根据索引序列构造最终顶点
    // 遍历索引序列
    for (auto idx : indices)
    {
        // 将索引对应的顶点的 x 坐标添加到输出向量中
        outVertices.push_back(polygon[idx][0]);
        // 将索引对应的顶点的 y 坐标添加到输出向量中
        outVertices.push_back(polygon[idx][1]);
    }
}

/**
 * @brief 绘制单个字符.
 *
 * 该函数获取指定字符的轮廓,对轮廓进行三角剖分,并将三角剖分后的顶点数据上传到 OpenGL 缓冲区进行绘制.
 * 同时,更新光标位置.
 *
 * @param font 字体信息结构体指针.
 * @param codepoint 字符的 Unicode 码点.
 * @param cursorX 光标当前的 x 坐标.
 * @param baseY 字符的基线 y 坐标.
 * @param scale 字体缩放比例.
 */
void renderGlyph(stbtt_fontinfo* font, int codepoint, float& cursorX, float baseY, float scale)
{
    // 定义字符的水平偏移量
    float xOffset;
    // 获取指定字符的轮廓
    auto outlines = getGlyphOutlines(font, codepoint, scale, xOffset);

    // 将每个 outline 强制翻转为 CCW
    // 然后分别进行 earcut
    // 遍历字符的每个轮廓
    for (auto& outline : outlines)
    {
        // 如果轮廓的顶点数量少于 6 个(即少于 3 个点)
        if (outline.size() < 6) continue; // 至少要3个点
        // 计算轮廓的面积
        float area = computeArea(outline);
        // 如果面积 < 0 => 翻转它
        // 如果轮廓的顶点顺序为顺时针
        if (area < 0)
        {
            // 翻转轮廓的顶点顺序
            reversePolygon(outline);
        }
        // 对 outline 做三角剖分
        // 先把它平移到 (cursorX, baseY)
        // 遍历轮廓的顶点坐标
        for (size_t i = 0; i < outline.size(); i += 2)
        {
            // 将顶点的 x 坐标平移到光标位置
            outline[i] += cursorX;
            // 将顶点的 y 坐标平移到基线位置
            outline[i + 1] += baseY;
        }

        // earcut
        // 创建一个向量,用于存储三角剖分后的顶点坐标
        std::vector<float> triVerts;
        // 对轮廓进行三角剖分
        triangulateSingleOutline(outline, triVerts);

        // 上传数据并绘制
        // 将三角剖分后的顶点数据上传到 OpenGL 缓冲区
        glBufferData(GL_ARRAY_BUFFER, triVerts.size() * sizeof(float), triVerts.data(), GL_STATIC_DRAW);
        // 绘制三角形
        glDrawArrays(GL_TRIANGLES, 0, triVerts.size() / 2);
    }

    // 移动光标
    // 更新光标位置
    cursorX += xOffset;
}

/**
 * @brief 渲染填充文本.
 *
 * 该函数遍历字符串中的每个字符,调用 renderGlyph 函数绘制每个字符,并更新光标位置.
 *
 * @param font 字体信息结构体指针.
 * @param text 要渲染的字符串.
 * @param x 文本的起始 x 坐标.
 * @param y 文本的起始 y 坐标.
 * @param scale 字体缩放比例.
 */
void renderFilledText(stbtt_fontinfo* font, const std::string& text, float x, float y, float scale)
{
    // 初始化光标位置
    float cursorX = x;
    // 遍历字符串中的每个字符
    for (char c : text)
    {
        // 绘制当前字符
        renderGlyph(font, static_cast<int>(c), cursorX, y, scale);
    }
}

/**
 * @brief 主函数,程序的入口点.
 *
 * 该函数初始化 GLFW 和 GLAD,创建窗口,初始化 OpenGL 着色器程序,加载字体,设置投影矩阵,
 * 并在主循环中渲染填充文本.最后,清理资源并退出程序.
 *
 * @return int 程序的退出状态码,0 表示正常退出.
 */
int main()
{
    // 初始化 GLFW 库
    if (!glfwInit()) return -1;
    // 创建一个 GLFW 窗口
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Earcut Text", nullptr, nullptr);
    // 检查窗口是否创建成功
    if (!window) return -1;
    // 将当前上下文设置为该窗口
    glfwMakeContextCurrent(window);
    // 初始化 GLAD 库
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        // 输出错误信息
        std::cerr << "Failed to init GLAD" << std::endl;
        // 返回错误状态码
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

    // 初始化 OpenGL 着色器程序
    initOpenGL();

    // 生成顶点数组对象
    glGenVertexArrays(1, &VAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &VBO);

    // 绑定顶点数组对象
    glBindVertexArray(VAO);
    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // 启用顶点属性
    glEnableVertexAttribArray(0);
    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // 加载字体
    // 读取字体文件
    auto fontBuf = readFontFile("C:/Windows/Fonts/arial.ttf");
    // 定义字体信息结构体
    stbtt_fontinfo font;
    // 初始化字体信息
    if (!stbtt_InitFont(&font, fontBuf.data(), stbtt_GetFontOffsetForIndex(fontBuf.data(), 0)))
    {
        // 输出错误信息
        std::cerr << "Failed to init font" << std::endl;
        // 返回错误状态码
        return -1;
    }

    // 设置正交投影
    // 创建正交投影矩阵
    glm::mat4 proj = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);
    // 使用着色器程序
    glUseProgram(shaderProgram);
    // 获取投影矩阵的统一变量位置
    GLint loc = glGetUniformLocation(shaderProgram, "projection");
    // 设置投影矩阵的统一变量值
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(proj));

    // 设置清屏颜色
    glClearColor(0, 0, 0, 1);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 清屏
        glClear(GL_COLOR_BUFFER_BIT);

        // 试着渲染字符串
        // 渲染第一行文本
        renderFilledText(&font, "Hello, OpenGL!", 10.0f, 500.0f, 0.08f);
        // 渲染第二行文本
        renderFilledText(&font, "ABCDEFGHIJKLMN", 10.0f, 350.0f, 0.08f);
        // 渲染第三行文本
        renderFilledText(&font, "OPQRTSUVWXYZ", 10.0f, 150.0f, 0.08f);
        // 渲染第四行文本
        renderFilledText(&font, "~+-*/!@#$%^&*(){}", 10.0f, 50.0f, 0.08f);

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    // 终止 GLFW 库
    glfwTerminate();
    // 返回正常退出状态码
    return 0;
}