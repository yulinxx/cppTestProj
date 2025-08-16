#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cmath>

// 定义Character结构体，用于存储每个字符的位图信息
// - sz: 字符位图的宽度和高度
// - bearing: 从基线到字符左上角的偏移量
// - advance: 到下一个字符的水平前进量（以像素为单位）
// - offset: 字符在纹理图集中的起始位置（x, y）
struct Character
{
    glm::ivec2 sz;    // glyph 大小
    glm::ivec2 bearing; // 从基线到 glyph 左上角的偏移
    GLuint advance;     // 到下一个 glyph 的水平偏移
    glm::ivec2 offset;  // 在图集中的 x0, y0
};

// 全局变量：
// - mapCharacter: 映射字符到其对应Character结构体的std::map
// - nAtlasTexture: 纹理图集的OpenGL纹理ID
// - nAtlasWidth, nAtlasHeight: 纹理图集的宽度和高度
std::map<GLchar, Character> mapCharacter;
GLuint nAtlasTexture = 0;
int nAtlasWidth = 0;
int nAtlasHeight = 0;

// 顶点着色器源代码：
// - 输入：vec4 vertex (前两个分量为位置，后两个为纹理坐标)
// - 输出：纹理坐标TexCoords
// - 使用投影矩阵转换位置
const char* vs = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

// 片段着色器源代码：
// - 输入：纹理坐标TexCoords
// - 输出：最终颜色color
// - 从纹理采样红色通道作为alpha，乘以指定的文本颜色
const char* fs = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

// 编译并链接顶点和片段着色器，返回着色器程序ID
// 注意：这里省略了错误检查代码，在实际生产中应添加
GLuint CompileShader()
{
    // 创建并编译顶点着色器
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs, NULL);
    glCompileShader(vertex);

    // 创建并编译片段着色器
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs, NULL);
    glCompileShader(fragment);

    // 创建程序对象，附加着色器并链接
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    // 删除不再需要的着色器对象
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

// 加载指定字体文件，生成包含所有ASCII字符（0-127，包括A-Z, a-z, 0-9和标点符号）的纹理图集
// - fontPath: 字体文件路径（如TTF文件）
// - fontSize: 字体大小（像素）
// 使用FreeType栅格化字符位图，并线性打包到纹理图集中
void LoadFont(const std::string& fontPath, int fontSize)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))  // 初始化FreeType库
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) // 加载字体文件
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize); // 设置字体像素大小

    // 禁用字节对齐限制，以便正确上传单通道纹理
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 计算纹理图集初始大小（基于字符数和字体大小的平方根估计）
    const int NUM_GLYPHS = 128;
    int nMaxDim = (1 + fontSize) * static_cast<int>(std::ceil(std::sqrt(NUM_GLYPHS)));

    nAtlasWidth = 1;
    while (nAtlasWidth < nMaxDim)
        nAtlasWidth <<= 1;

    nAtlasHeight = nAtlasWidth;

    // 分配图集像素缓冲区，并初始化为0
    unsigned char* chPixels = new unsigned char[nAtlasWidth * nAtlasHeight];
    std::memset(chPixels, 0, nAtlasWidth * nAtlasHeight);

    // 打包位置：pen_x, pen_y 为当前放置位置；row_height 为当前行最大高度
    int nPenX = 0;
    int nPenY = 0;
    int nRowHeight = 0;

    // 循环处理每个ASCII字符（0-127）
    for (unsigned char c = 0; c < NUM_GLYPHS; ++c)
    {
        // 加载字符的glyph位图
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "WARNING::FREETYTPE: Failed to load Glyph '" << c << "'" << std::endl;
            continue;
        }

        FT_Bitmap* bmp = &face->glyph->bitmap;

        // 如果当前行放不下，换行
        if ((nPenX + bmp->width + 1) >= nAtlasWidth)
        {
            nPenX = 0;
            nPenY += nRowHeight + 1; // 移动到下一行，使用当前行最大高度 + 间距
            nRowHeight = 0;

            // 如果高度不足，动态扩展图集高度
            if (nPenY + fontSize + 1 >= nAtlasHeight)
            {
                std::cerr << "ERROR: Atlas too small! Increase size." << std::endl;

                // 可动态增大 nAtlasHeight 并 realloc pixels
                nAtlasHeight *= 2;

                unsigned char* new_pixels = new unsigned char[nAtlasWidth * nAtlasHeight];
                std::memset(new_pixels, 0, nAtlasWidth * nAtlasHeight);
                std::memcpy(new_pixels, chPixels, nAtlasWidth * (nAtlasHeight / 2));

                delete[] chPixels;
                chPixels = new_pixels;
            }
        }

        // 更新当前行高度
        nRowHeight = std::max(nRowHeight, static_cast<int>(bmp->rows));

        // 将位图数据拷贝到图集缓冲区
        for (int row = 0; row < bmp->rows; ++row)
        {
            for (int col = 0; col < bmp->width; ++col)
            {
                int x = nPenX + col;
                int y = nPenY + row;
                chPixels[y * nAtlasWidth + x] = bmp->buffer[row * bmp->pitch + col];
            }
        }

        // 存储字符信息
        Character character = {
            glm::ivec2(bmp->width, bmp->rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x >> 6),
            glm::ivec2(nPenX, nPenY)
        };

        mapCharacter[c] = character;

        // 移动到下一个放置位置（添加间距）
        nPenX += bmp->width + 1;
    }

    // 生成OpenGL纹理对象，并上传像素数据（GL_RED通道）
    glGenTextures(1, &nAtlasTexture);
    glBindTexture(GL_TEXTURE_2D, nAtlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nAtlasWidth, nAtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, chPixels);

    // 设置纹理参数：边缘clamp，线性过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 释放缓冲区和FreeType资源
    delete[] chPixels;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

// 定义TextBatch结构体，用于存储静态文本的批处理数据
// - VAO, VBO: 顶点数组和缓冲对象
// - vertexCount: 顶点总数（用于绘制）
struct TextBatch
{
    GLuint VAO, VBO;
    GLsizei vertexCount; // 顶点数
};

// 函数：CreateStaticTextBatch
// 为给定的静态文本生成批处理VBO
// - text: 要渲染的字符串
// - x, y: 起始位置
// - scale: 缩放因子
// 从图集索引每个字符的UV，生成四边形顶点数据（位置 + UV）
TextBatch CreateStaticTextBatch(const std::string& text, GLfloat x, GLfloat y, GLfloat scale)
{
    TextBatch batch;
    // 生成VAO和VBO
    glGenVertexArrays(1, &batch.VAO);
    glGenBuffers(1, &batch.VBO);
    glBindVertexArray(batch.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch.VBO);

    // 收集顶点数据的vector
    std::vector<GLfloat> vertices;

    // 遍历字符串中的每个字符
    for (auto c : text)
    {
        // 查找字符信息
        auto it = mapCharacter.find(c);
        if (it == mapCharacter.end())
        {
            std::cerr << "WARNING: Character '" << c << "' not found in atlas!" << std::endl;
            continue;
        }

        Character ch = it->second;

        // 计算字符在屏幕上的位置（考虑bearing偏移）
        GLfloat xpos = x + ch.bearing.x * scale;
        GLfloat ypos = y - (ch.sz.y - ch.bearing.y) * scale;

        // 计算字符四边形的宽度和高度
        GLfloat w = ch.sz.x * scale;
        GLfloat h = ch.sz.y * scale;

        // 计算纹理UV坐标（u0,v0 为左上，u1,v1 为右下）
        GLfloat u0 = static_cast<GLfloat>(ch.offset.x) / nAtlasWidth;
        GLfloat v0 = static_cast<GLfloat>(ch.offset.y) / nAtlasHeight;
        GLfloat u1 = static_cast<GLfloat>(ch.offset.x + ch.sz.x) / nAtlasWidth;
        GLfloat v1 = static_cast<GLfloat>(ch.offset.y + ch.sz.y) / nAtlasHeight;

        // 定义6个顶点（两个三角形组成四边形）
        GLfloat quadVertices[6][4] = {
            { xpos,     ypos + h,   u0, v0 },
            { xpos,     ypos,       u0, v1 },
            { xpos + w, ypos,       u1, v1 },

            { xpos,     ypos + h,   u0, v0 },
            { xpos + w, ypos,       u1, v1 },
            { xpos + w, ypos + h,   u1, v0 }
        };

        // 添加到vertices vector
        for (int i = 0; i < 6; ++i)
        {
            vertices.push_back(quadVertices[i][0]);
            vertices.push_back(quadVertices[i][1]);
            vertices.push_back(quadVertices[i][2]);
            vertices.push_back(quadVertices[i][3]);
        }

        // 更新下一个字符的起始x位置
        x += ch.advance * scale;
    }

    // 上传顶点数据到VBO（静态绘制）
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // 配置顶点属性（位置0: 4 floats，位置+UV）
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 计算顶点总数
    batch.vertexCount = static_cast<GLsizei>(vertices.size() / 4);

    return batch;
}

// 渲染静态文本批次
// - batch: TextBatch对象
// - shader: 着色器程序ID
// - projection: 投影矩阵
// - color: 文本颜色
// 绑定纹理和VAO，通过一次绘制调用渲染所有字符
void RenderStaticText(const TextBatch& batch, GLuint shader, glm::mat4 projection, glm::vec3 color)
{
    // 使用着色器程序
    glUseProgram(shader);
    // 设置统一变量：文本颜色和投影矩阵
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // 激活纹理单元0，并绑定图集纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, nAtlasTexture);
    // 绑定VAO
    glBindVertexArray(batch.VAO);

    // 绘制所有顶点（三角形模式）
    glDrawArrays(GL_TRIANGLES, 0, batch.vertexCount);

    // 解绑VAO和纹理
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// 主函数：程序入口
// 初始化OpenGL窗口，加载字体，创建文本批次，并进入渲染循环
int main()
{
    // 初始化GLFW
    glfwInit();
    // 设置OpenGL上下文版本（3.3核心配置文件）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1200, 1000, "OpenGL Text Atlas Rendering", NULL, NULL);
    glfwMakeContextCurrent(window);

    // 加载OpenGL函数指针
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 启用混合模式（用于透明文本）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 编译着色器
    GLuint shader = CompileShader();

    // 加载字体并生成纹理图集
    //LoadFont("C:/Windows/Fonts/arial.ttf", 48); // 字体路径
    LoadFont("C:/Windows/Fonts/STCAIYUN.TTF", 48); // 字体路径

    // 设置正交投影矩阵（2D渲染）
    glm::mat4 projection = glm::ortho(0.0f, 1200.0f, 0.0f, 1000.0f);

    // 创建静态文本批次
    TextBatch helloBatch = CreateStaticTextBatch("Hello**+World", 25.0f, 300.0f, 2.0f);

    // 进入主循环
    while (!glfwWindowShouldClose(window))
    {
        // 清屏
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 渲染文本
        RenderStaticText(helloBatch, shader, projection, glm::vec3(1.0f, 1.0f, 1.0f));

        // 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteTextures(1, &nAtlasTexture);
    glfwTerminate();
    return 0;
}