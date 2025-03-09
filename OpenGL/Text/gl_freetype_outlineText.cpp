#include <glad/glad.h>  // 添加这一行以包含GLAD库
// 包含 GLFW 库,用于创建窗口和处理输入等操作
#include <GLFW/glfw3.h>
// 包含 stb_truetype 库,用于处理 TrueType 字体
#include <stb_truetype.h>
// 包含输入输出流库,用于输入输出操作
#include <iostream>
// 包含向量容器库,用于存储动态数组
#include <vector>
// 包含文件流库,用于文件操作
#include <fstream>
// 包含字符串流库,用于字符串操作
#include <sstream>
// 包含宽字符输入输出流库
#include <cwchar>

// 引入 freetype 库
#include <ft2build.h>
#include FT_FREETYPE_H

// 定义窗口的宽度
#define WINDOW_WIDTH 800
// 定义窗口的高度
#define WINDOW_HEIGHT 600

/**
 * 顶点着色器代码,使用原始字符串字面量表示
 * 接收顶点位置,通过投影矩阵将其转换到裁剪空间
 */
const char* vertexShaderSource = R"(
#version 400
// 顶点属性,位置坐标,在顶点缓冲区中的位置为 0
layout (location = 0) in vec2 aPos;
// 投影矩阵,用于将顶点从局部坐标转换到裁剪空间
uniform mat4 projection;
// 主函数,每个顶点都会执行此函数
void main()
{
   // 通过投影矩阵变换顶点位置,得到裁剪空间坐标
   gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

/**
 * 片段着色器代码,使用原始字符串字面量表示
 * 为每个片段输出白色颜色
 */
const char* fragmentShaderSource = R"(
#version 400
// 输出变量,最终的片段颜色
out vec4 FragColor;
// 主函数,每个片段都会执行此函数
void main()
{
   // 设置片段颜色为白色,RGBA 各通道值均为 1.0
   FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色线条
}
)";

// 定义字体数据结构
typedef struct
{
    // 顶点数组对象 ID
    GLuint vao;
    // 顶点缓冲对象 ID
    GLuint vbo;
    // 存储所有顶点的向量
    std::vector<float> vertices;
    // 存储每个字符的顶点数量的向量
    std::vector<int> glyphVertexCounts;
} FontData;

// 全局字体数据对象
FontData font;

/**
 * 二阶贝塞尔曲线细分函数
 * @param x0 起点的 x 坐标
 * @param y0 起点的 y 坐标
 * @param cx 控制点的 x 坐标
 * @param cy 控制点的 y 坐标
 * @param x1 终点的 x 坐标
 * @param y1 终点的 y 坐标
 * @param scale 缩放因子
 * @param xOffset x 偏移量
 * @param yOffset y 偏移量
 * @param vertices 存储顶点的向量
 * @param segments 细分的段数,默认为 20
 */
void addBezierCurve(float x0, float y0, float cx, float cy, float x1, float y1, float scale, float xOffset, float yOffset,
    std::vector<float>& vertices, int segments = 20)
{
    // 遍历细分的段数
    for (int i = 0; i <= segments; i++)
    {
        // 计算当前细分点的参数 t
        float t = i / (float)segments;
        // 计算 1 - t
        float u = 1.0f - t;
        // 根据贝塞尔曲线公式计算当前细分点的 x 坐标
        float x = u * u * x0 + 2 * u * t * cx + t * t * x1;
        // 根据贝塞尔曲线公式计算当前细分点的 y 坐标
        float y = u * u * y0 + 2 * u * t * cy + t * t * y1;
        // 将细分点的 x 坐标添加到顶点向量中,并考虑偏移和缩放
        vertices.push_back(xOffset + x * scale);
        // 将细分点的 y 坐标添加到顶点向量中,并考虑偏移和缩放
        vertices.push_back(yOffset + y * scale);
    }
}

// 使用 freetype 初始化字体
void initFont(const char* fontPath, const wchar_t* text, float fontSize, float x, float y)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "无法初始化 FreeType 库!" << std::endl;
        std::exit(1);
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face))
    {
        std::cerr << "无法加载字体文件!" << std::endl;
        std::exit(1);
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);

    float xpos = x;
    while (*text)
    {
        FT_UInt glyph_index = FT_Get_Char_Index(face, *text);
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT))
        {
            std::cerr << "无法加载字形!" << std::endl;
            continue;
        }

        if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
        {
            std::cerr << "无法渲染字形!" << std::endl;
            continue;
        }

        FT_Outline& outline = face->glyph->outline;
        std::vector<float> glyphVertices;
        float x0 = xpos;
        float y0 = y;

        for (int i = 0; i < outline.n_contours; ++i)
        {
            int start = (i == 0) ? 0 : outline.contours[i - 1] + 1;
            int end = outline.contours[i];

            for (int j = start; j < end; ++j)
            {
                FT_Vector& p = outline.points[j];
                glyphVertices.push_back(x0 + p.x * 0.01f); // 简单缩放
                glyphVertices.push_back(y0 + p.y * 0.01f);
            }

            if (!glyphVertices.empty())
            {
                glyphVertices.push_back(glyphVertices[0]);
                glyphVertices.push_back(glyphVertices[1]);
            }
        }

        if (!glyphVertices.empty())
        {
            font.vertices.insert(font.vertices.end(), glyphVertices.begin(), glyphVertices.end());
            font.glyphVertexCounts.push_back(glyphVertices.size() / 2);
        }

        xpos += face->glyph->advance.x >> 6;
        ++text;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &font.vao);
    glGenBuffers(1, &font.vbo);

    glBindVertexArray(font.vao);
    glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
    glBufferData(GL_ARRAY_BUFFER, font.vertices.size() * sizeof(float), font.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

/**
 * 渲染线条文字
 * @param program 着色器程序 ID
 */
void renderText(GLuint program)
{
    // 使用指定的着色器程序
    glUseProgram(program);

    // 设置正交投影矩阵
    // 位置和 OpenGL 一致
    float proj[16] = {
        2.0f / WINDOW_WIDTH, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / WINDOW_HEIGHT, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };

    // 设置投影矩阵的 uniform 变量
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, proj);

    // 绑定顶点数组对象
    glBindVertexArray(font.vao);

    // 按字符逐个绘制,避免字符间连接
    int vertexOffset = 0;
    for (int count : font.glyphVertexCounts)
    {
        // 绘制当前字符的顶点
        glDrawArrays(GL_LINE_STRIP, vertexOffset, count);
        // 更新顶点偏移量
        vertexOffset += count;
    }
}

/**
 * 主函数,程序入口
 * @return 程序退出状态码
 */
int main()
{
    // 初始化 GLFW 库
    if (!glfwInit())
    {
        // 输出错误信息
        printf("GLFW 初始化失败!\n");
        // 返回错误状态码
        return -1;
    }

    // 设置 GLFW 上下文版本为 4.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // 创建 GLFW 窗口
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Outline Text Rendering", NULL, NULL);
    // 检查窗口是否成功创建
    if (!window)
    {
        // 终止 GLFW 库
        glfwTerminate();
        // 返回错误状态码
        return -1;
    }

    // 设置当前上下文为创建的窗口
    glfwMakeContextCurrent(window);

    // 初始化 GLAD 库
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        // 输出错误信息
        printf("GLAD 初始化失败!\n");
        // 返回错误状态码
        return -1;
    }

    // 创建顶点着色器对象
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 设置顶点着色器源代码
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 检查顶点着色器编译错误
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // 获取编译错误信息
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        // 输出编译错误信息
        printf("顶点着色器编译失败:\n%s\n", infoLog);
    }

    // 创建片段着色器对象
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 设置片段着色器源代码
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 检查片段着色器编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // 获取编译错误信息
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        // 输出编译错误信息
        printf("片段着色器编译失败:\n%s\n", infoLog);
    }

    // 创建着色器程序对象
    GLuint shaderProgram = glCreateProgram();
    // 附着顶点着色器到着色器程序
    glAttachShader(shaderProgram, vertexShader);
    // 附着片段着色器到着色器程序
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 检查着色器程序链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        // 获取链接错误信息
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        // 输出链接错误信息
        printf("着色器程序链接失败:\n%s\n", infoLog);
    }

    // 删除顶点着色器对象
    glDeleteShader(vertexShader);
    // 删除片段着色器对象
    glDeleteShader(fragmentShader);

    // 初始化字体并生成轮廓顶点
    initFont("C:/Windows/Fonts/simhei.ttf", L"你好OpenGL", 64.0f, 100.0f, 300.0f);

    // 主循环,直到窗口关闭
    while (!glfwWindowShouldClose(window))
    {
        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);

        // 渲染文字
        renderText(shaderProgram);

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    // 删除顶点数组对象
    glDeleteVertexArrays(1, &font.vao);
    // 删除顶点缓冲对象
    glDeleteBuffers(1, &font.vbo);
    // 终止 GLFW 库
    glfwTerminate();

    // 返回成功状态码
    return 0;
}