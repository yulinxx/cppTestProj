// 包含必要的头文件：OpenGL加载器、窗口管理、数学库、FreeType字体库等
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
#include <set>
#include <locale>
#include <codecvt>

// 包含 stb_rect_pack 用于优化纹理图集打包
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

// 定义Character结构体，用于存储每个字符的位图信息
struct Character
{
    glm::ivec2 sz;    // glyph 大小
    glm::ivec2 bearing; // 从基线到 glyph 左上角的偏移
    GLuint advance;     // 到下一个 glyph 的水平偏移
    glm::ivec2 offset;  // 在图集中的 x0, y0
};

// 全局变量
std::map<unsigned long, Character> mapCharacters;
GLuint nAtlasTexture = 0;
int nAtlasWidth = 0, nAtlasHeight = 0;

// 顶点着色器源代码
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

// 片段着色器源代码
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

// 编译并链接着色器
GLuint CompileShader()
{
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs, NULL);
    glCompileShader(fragment);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

// 收集独特的Unicode code points
std::set<unsigned long> CollectUniqueCodePoints(const std::string& text)
{
    std::set<unsigned long> codePoints;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string u32text = converter.from_bytes(text);
    for (char32_t c : u32text)
        codePoints.insert(static_cast<unsigned long>(c));
    return codePoints;
}

// 加载字体并生成纹理图集，使用 stb_rect_pack 优化打包
void LoadFont(const std::string& fontPath, int fontSize, const std::string& text)
{
    // 初始化FreeType库
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // 加载字体文件
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    // 设置字体像素大小
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 收集需要加载的字符：ASCII 0-127 + 文本中的独特Unicode
    std::set<unsigned long> charsToLoad;
    for (unsigned long c = 0; c < 128; ++c) // ASCII字符
        charsToLoad.insert(c);

    std::set<unsigned long> unicodeChars = CollectUniqueCodePoints(text);
    charsToLoad.insert(unicodeChars.begin(), unicodeChars.end());

    // 计算纹理图集初始大小
    int nNumGlyphs = static_cast<int>(charsToLoad.size());
    int nMaxDim = (1 + fontSize) * static_cast<int>(std::ceil(std::sqrt(nNumGlyphs)));

    nAtlasWidth = 1;
    while (nAtlasWidth < nMaxDim)
        nAtlasWidth <<= 1;

    nAtlasHeight = nAtlasWidth;

    // 分配图集像素缓冲区
    unsigned char* chPixels = new unsigned char[nAtlasWidth * nAtlasHeight];
    std::memset(chPixels, 0, nAtlasWidth * nAtlasHeight);

    // 初始化 stb_rect_pack
    std::vector<stbrp_rect> rects;
    for (unsigned long c : charsToLoad)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "WARNING::FREETYTPE: Failed to load Glyph U+" << std::hex << c << std::endl;
            continue;
        }
        FT_Bitmap* bmp = &face->glyph->bitmap;
        stbrp_rect rect = { 0 };
        rect.id = static_cast<int>(c);
        rect.w = bmp->width + 1; // 添加间距
        rect.h = bmp->rows + 1;
        rects.push_back(rect);
    }

    // 使用 stb_rect_pack 进行打包
    stbrp_context context;
    std::vector<stbrp_node> nodes(nAtlasWidth);
    stbrp_init_target(&context, nAtlasWidth, nAtlasHeight, nodes.data(), nodes.size());
    if (!stbrp_pack_rects(&context, rects.data(), rects.size()))
    {
        // 如果打包失败，增加图集大小并重试
        nAtlasHeight *= 2;
        delete[] chPixels;
        chPixels = new unsigned char[nAtlasWidth * nAtlasHeight];
        std::memset(chPixels, 0, nAtlasWidth * nAtlasHeight);
        nodes.resize(nAtlasWidth);
        stbrp_init_target(&context, nAtlasWidth, nAtlasHeight, nodes.data(), nodes.size());
        if (!stbrp_pack_rects(&context, rects.data(), rects.size()))
        {
            std::cerr << "ERROR: Failed to pack glyphs into atlas!" << std::endl;
            return;
        }
    }

    // 将位图数据拷贝到图集
    for (const auto& rect : rects)
    {
        unsigned long c = static_cast<unsigned long>(rect.id);
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            continue;

        FT_Bitmap* bmp = &face->glyph->bitmap;

        for (int nRow = 0; nRow < bmp->rows; ++nRow)
        {
            for (int nCol = 0; nCol < bmp->width; ++nCol)
            {
                int x = rect.x + nCol;
                int y = rect.y + nRow;
                chPixels[y * nAtlasWidth + x] = bmp->buffer[nRow * bmp->pitch + nCol];
            }
        }

        // 存储字符信息
        Character character = {
            glm::ivec2(bmp->width, bmp->rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x >> 6),
            glm::ivec2(rect.x, rect.y)
        };

        mapCharacters[c] = character;
    }

    // 生成OpenGL纹理
    glGenTextures(1, &nAtlasTexture);
    glBindTexture(GL_TEXTURE_2D, nAtlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nAtlasWidth, nAtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, chPixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, 0);

    // 释放资源
    delete[] chPixels;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

// TextBatch 结构体
struct TextBatch
{
    GLuint VAO, VBO;
    GLsizei vertexCount;
};

// 生成静态文本批次
TextBatch CreateStaticTextBatch(const std::string& text, GLfloat x, GLfloat y, GLfloat scale)
{
    TextBatch batch;
    glGenVertexArrays(1, &batch.VAO);
    glGenBuffers(1, &batch.VBO);
    glBindVertexArray(batch.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch.VBO);

    std::vector<GLfloat> vertices;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string u32text = converter.from_bytes(text);

    for (char32_t ch : u32text)
    {
        unsigned long c = static_cast<unsigned long>(ch);
        auto it = mapCharacters.find(c);
        if (it == mapCharacters.end())
        {
            std::cerr << "WARNING: Character U+" << std::hex << c << " not found in atlas!" << std::endl;
            continue;
        }
        Character charInfo = it->second;

        GLfloat xpos = x + charInfo.bearing.x * scale;
        GLfloat ypos = y - (charInfo.sz.y - charInfo.bearing.y) * scale;
        GLfloat w = charInfo.sz.x * scale;
        GLfloat h = charInfo.sz.y * scale;

        GLfloat u0 = static_cast<GLfloat>(charInfo.offset.x) / nAtlasWidth;
        GLfloat v0 = static_cast<GLfloat>(charInfo.offset.y) / nAtlasHeight;
        GLfloat u1 = static_cast<GLfloat>(charInfo.offset.x + charInfo.sz.x) / nAtlasWidth;
        GLfloat v1 = static_cast<GLfloat>(charInfo.offset.y + charInfo.sz.y) / nAtlasHeight;

        GLfloat quadVertices[6][4] = {
            { xpos,     ypos + h,   u0, v0 },
            { xpos,     ypos,       u0, v1 },
            { xpos + w, ypos,       u1, v1 },
            { xpos,     ypos + h,   u0, v0 },
            { xpos + w, ypos,       u1, v1 },
            { xpos + w, ypos + h,   u1, v0 }
        };

        for (int i = 0; i < 6; ++i)
        {
            vertices.push_back(quadVertices[i][0]);
            vertices.push_back(quadVertices[i][1]);
            vertices.push_back(quadVertices[i][2]);
            vertices.push_back(quadVertices[i][3]);
        }

        x += charInfo.advance * scale;
    }

    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    batch.vertexCount = static_cast<GLsizei>(vertices.size() / 4);
    return batch;
}

// 渲染静态文本
void RenderStaticText(const TextBatch& batch, GLuint shader, glm::mat4 projection, glm::vec3 color)
{
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, nAtlasTexture);
    glBindVertexArray(batch.VAO);

    glDrawArrays(GL_TRIANGLES, 0, batch.vertexCount);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// 主函数
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL Text Atlas Rendering with Chinese", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // Output OpenGL information
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===========================" << std::endl;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLuint shader = CompileShader();

    // 要渲染的多语言文本（UTF-8编码）
    //std::string text = "你好，世界！。！イベント体験~한국말韓國말-Hello World~";
    std::string text = "从左到右分别是简体中文、繁体中文、日语和韩语的“述”的字形";

    // 加载字体（需支持中文、日文、韩文的字体）
    LoadFont("C:/Windows/Fonts/STCAIYUN.TTF", 68, text);

    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    TextBatch helloBatch = CreateStaticTextBatch(text, 25.0f, 500.0f, 0.5f);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        RenderStaticText(helloBatch, shader, projection, glm::vec3(1.0f, 1.0f, 1.0f));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &nAtlasTexture);
    glfwTerminate();
    return 0;
}