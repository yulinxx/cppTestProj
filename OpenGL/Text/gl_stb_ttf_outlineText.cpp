#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_truetype.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

// 窗口尺寸
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// 顶点着色器
const char* vs = "#version 400\n"
"layout (location = 0) in vec2 aPos;\n"
"uniform mat4 projection;\n"
"void main()\n"
"{\n"
"   gl_Position = projection * vec4(aPos, 0.0, 1.0);\n"
"}\0";

// 片段着色器
const char* fs = "#version 400\n"
"out vec4 FragColor;\n"
"void main()\n"
"{\n"
"   FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色线条\n"
"}\0";

// 字体数据结构
typedef struct
{
    GLuint vao, vbo;
    std::vector<float> vertices; // 存储所有顶点
    std::vector<int> glyphVertexCounts; // 每个字符的顶点数量
} FontData;

FontData font;

// 贝塞尔曲线细分(二阶贝塞尔)
void addBezierCurve(float x0, float y0, float cx, float cy, float x1, float y1, float scale, float xOffset, float yOffset, std::vector<float>& vertices, int segments = 20)
{
    for (int i = 0; i <= segments; i++)
    {
        float t = i / (float)segments;
        float u = 1.0f - t;
        float x = u * u * x0 + 2 * u * t * cx + t * t * x1;
        float y = u * u * y0 + 2 * u * t * cy + t * t * y1;
        vertices.push_back(xOffset + x * scale);
        vertices.push_back(yOffset + y * scale);
    }
}

// 初始化字体并生成轮廓顶点
void initFont(const char* fontPath, const char* text, float fontSize, float x, float y)
{
    FILE* fp = fopen(fontPath, "rb");
    if (!fp)
    {
        printf("无法打开字体文件!\n");
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    unsigned char* ttf_buffer = (unsigned char*)malloc(size);
    fread(ttf_buffer, 1, size, fp);
    fclose(fp);

    stbtt_fontinfo fontInfo;
    if (!stbtt_InitFont(&fontInfo, ttf_buffer, 0))
    {
        printf("字体初始化失败!\n");
        free(ttf_buffer);
        exit(1);
    }

    float scale = stbtt_ScaleForPixelHeight(&fontInfo, fontSize);
    float xpos = x;

    while (*text)
    {
        if (*text >= 32 && *text < 128)
        {
            int glyphIndex = stbtt_FindGlyphIndex(&fontInfo, *text);
            stbtt_vertex* vertices;
            int numVerts = stbtt_GetGlyphShape(&fontInfo, glyphIndex, &vertices);

            float x0 = xpos;
            float y0 = y;
            std::vector<float> glyphVertices;

            // 遍历轮廓顶点并连接
            for (int i = 0; i < numVerts; i++)
            {
                switch (vertices[i].type)
                {
                case STBTT_vmove:
                    if (!glyphVertices.empty())
                    {
                        glyphVertices.push_back(glyphVertices[0]); // 闭合轮廓
                        glyphVertices.push_back(glyphVertices[1]);
                    }
                    glyphVertices.push_back(x0 + vertices[i].x * scale);
                    glyphVertices.push_back(y0 + vertices[i].y * scale);
                    break;
                case STBTT_vline:
                    glyphVertices.push_back(x0 + vertices[i].x * scale);
                    glyphVertices.push_back(y0 + vertices[i].y * scale);
                    break;
                case STBTT_vcurve:
                    addBezierCurve(
                        glyphVertices[glyphVertices.size() - 2],
                        glyphVertices[glyphVertices.size() - 1],
                        x0 + vertices[i].cx * scale, y0 + vertices[i].cy * scale,
                        x0 + vertices[i].x * scale, y0 + vertices[i].y * scale,
                        1.0f, 0.0f, 0.0f, glyphVertices
                    );
                    break;
                }
            }
            if (!glyphVertices.empty())
            {
                glyphVertices.push_back(glyphVertices[0]); // 闭合最后一个轮廓
                glyphVertices.push_back(glyphVertices[1]);
                font.vertices.insert(font.vertices.end(), glyphVertices.begin(), glyphVertices.end());
                font.glyphVertexCounts.push_back(glyphVertices.size() / 2); // 记录顶点数量
            }

            // 更新 x 位置
            int advanceWidth, leftSideBearing;
            stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &advanceWidth, &leftSideBearing);
            xpos += (advanceWidth + leftSideBearing) * scale;

            stbtt_FreeShape(&fontInfo, vertices);
        }
        ++text;
    }

    free(ttf_buffer);

    // 初始化 OpenGL 缓冲区
    glGenVertexArrays(1, &font.vao);
    glGenBuffers(1, &font.vbo);

    glBindVertexArray(font.vao);
    glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
    glBufferData(GL_ARRAY_BUFFER, font.vertices.size() * sizeof(float), font.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

// 渲染线条文字
void renderText(GLuint program)
{
    glUseProgram(program);

    // 设置正交投影矩阵
    float proj[16] = {
        2.0f / WINDOW_WIDTH, 0.0f, 0.0f, 0.0f,
        0.0f, -2.0f / WINDOW_HEIGHT, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, proj);

    glBindVertexArray(font.vao);

    // 按字符逐个绘制,避免字符间连接
    int vertexOffset = 0;
    for (int count : font.glyphVertexCounts)
    {
        glDrawArrays(GL_LINE_STRIP, vertexOffset, count);
        vertexOffset += count;
    }
}

int main()
{
    if (!glfwInit())
    {
        printf("GLFW 初始化失败!\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Outline Text Rendering", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("GLAD 初始化失败!\n");
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

    // 初始化字体并生成轮廓顶点
    initFont("C:/Windows/Fonts/arial.ttf", "Hello World!", 64.0f, 100.0f, 300.0f);

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        renderText(shaderProgram);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &font.vao);
    glDeleteBuffers(1, &font.vbo);
    glfwTerminate();

    return 0;
}