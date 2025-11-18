// g++ main.cpp -std=c++17 -lfreetype -lglfw -lGL -ldl
// MSVC：vcpkg 已装 freetype glfw3 glad glm，直接 F5
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <vector>
#include <string>
#include <unordered_map>
#include <iostream>

// ---------- 着色器 ----------
const char* vs = R"(#version 330 core
layout(location = 0) in vec4 vertex; // x,y,u,v
uniform mat4 projection;
out vec2 TexCoords;
void main(){
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
})";

const char* fs = R"(#version 330 core
in vec2 TexCoords;
uniform sampler2D text;
uniform vec3 textColor;
out vec4 color;
void main(){
    float a = texture(text, TexCoords).r;
    color = vec4(textColor, 1.0) * vec4(1.0, 1.0, 1.0, a);
})";

GLuint makeProgram()
{
    GLuint v = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v, 1, &vs, nullptr);
    glCompileShader(v);
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fs, nullptr);
    glCompileShader(f);
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// ---------- 字符信息 ----------
struct CharInfo
{
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
    glm::ivec2 offset;
};
std::unordered_map<unsigned int, CharInfo> gGlyphs;
GLuint gAtlas;
int gW = 0, gH = 0;

// ---------- UTF-8 → Unicode ----------
std::vector<unsigned int> utf8ToCodepoints(const std::string& str)
{
    std::vector<unsigned int> out;
    for (size_t i = 0; i < str.size();)
    {
        unsigned char c = str[i];
        unsigned int cp = 0;
        if (c < 0x80)
        {
            cp = c; ++i;
        }
        else if (c < 0xE0)
        {
            cp = ((c & 0x1F) << 6) | (str[i + 1] & 0x3F); i += 2;
        }
        else if (c < 0xF0)
        {
            cp = ((c & 0x0F) << 12) | ((str[i + 1] & 0x3F) << 6) | (str[i + 2] & 0x3F); i += 3;
        }
        else
        {
            cp = ((c & 0x07) << 18) | ((str[i + 1] & 0x3F) << 12) | ((str[i + 2] & 0x3F) << 6) | (str[i + 3] & 0x3F); i += 4;
        }
        out.push_back(cp);
    }
    return out;
}

// ---------- 生成图集 ----------
void buildAtlas(const std::string& fontPath, int fontH)
{
    FT_Library ft; FT_Init_FreeType(&ft);
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "No font\n"; exit(1);
    }
    FT_Set_Pixel_Sizes(face, 0, fontH);

    // 字符集：ASCII + 常用 3500 汉字
    std::vector<unsigned int> cps;
    for (unsigned int c = 32; c < 127; ++c)
        cps.push_back(c);

    //0x4E00 是 CJK Unified Ideographs(中日韓統一表意文字) 字元區塊的起點。
    //for (unsigned int c = 0x4E00; c < 0x4E00 + 3500; ++c)
    for (unsigned int c = 0x4E00; c <= 0x9FFF; ++c) // 從 0x4E00 到 0x9FFF 總共有 20,992 個字元。
        cps.push_back(c);

    int dim = 1; while (dim * dim < (int)cps.size() * fontH * fontH) dim <<= 1;
    gW = gH = dim;

    std::vector<unsigned char> atlas(gW * gH, 0);
    int penX = 0, penY = 0, rowH = 0;

    for (unsigned int cp : cps)
    {
        if (FT_Load_Char(face, cp, FT_LOAD_RENDER)) continue;
        FT_Bitmap& bmp = face->glyph->bitmap;
        if (penX + bmp.width + 1 >= gW)
        {
            penX = 0; penY += rowH + 1; rowH = 0;
            if (penY + fontH >= gH)
            {
                dim <<= 1; gW = gH = dim; atlas.resize(gW * gH);
            }
        }
        rowH = std::max(rowH, static_cast<int>(bmp.rows));

        for (int r = 0; r < bmp.rows; ++r)
            for (int c = 0; c < bmp.width; ++c)
                atlas[(penY + r) * gW + penX + c] = bmp.buffer[r * bmp.pitch + c];

        gGlyphs[cp] = { { bmp.width, bmp.rows },
                        { face->glyph->bitmap_left, face->glyph->bitmap_top },
                        static_cast<GLuint>(face->glyph->advance.x >> 6),
                        { penX, penY } };
        penX += bmp.width + 1;
    }

    glGenTextures(1, &gAtlas);
    glBindTexture(GL_TEXTURE_2D, gAtlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gW, gH, 0, GL_RED, GL_UNSIGNED_BYTE, atlas.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

// ---------- 静态批处理 ----------
struct Batch
{
    GLuint vao, vbo;
    GLsizei count;
};

Batch makeBatch(const std::string& text, float scale)
{
    auto cps = utf8ToCodepoints(text);
    std::vector<float> verts;   // x,y,u,v
    float x = 25.0f, y = 120.0f;

    for (unsigned int cp : cps)
    {
        if (gGlyphs.count(cp) == 0) continue;
        const auto& ch = gGlyphs[cp];
        float w = ch.size.x * scale;
        float h = ch.size.y * scale;
        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float u0 = ch.offset.x / float(gW);
        float v0 = ch.offset.y / float(gH);
        float u1 = (ch.offset.x + ch.size.x) / float(gW);
        float v1 = (ch.offset.y + ch.size.y) / float(gH);

        float quad[] = {
            xpos,     ypos + h, u0, v0,
            xpos,     ypos,     u0, v1,
            xpos + w, ypos,     u1, v1,
            xpos,     ypos + h, u0, v0,
            xpos + w, ypos,     u1, v1,
            xpos + w, ypos + h, u1, v0
        };
        verts.insert(verts.end(), std::begin(quad), std::end(quad));
        x += ch.advance * scale;
    }

    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    return { vao, vbo, static_cast<GLsizei>(verts.size() / 4) };
}

// ---------- GLFW ----------
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(900, 200, "FreeType Static Batch + 中文", nullptr, nullptr);
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //buildAtlas("C:/Windows/Fonts/STCAIYUN.TTF", 48);
    buildAtlas("C:/Windows/Fonts/STCAIYUN.TTF", 48);
    GLuint prog = makeProgram();
    Batch batch = makeBatch("Hello 静态批处理 示例 123456", 1.0f);

    glm::mat4 proj = glm::ortho(0.0f, 900.0f, 0.0f, 200.0f);

    while (!glfwWindowShouldClose(win))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, &proj[0][0]);
        glUniform3f(glGetUniformLocation(prog, "textColor"), 1.0f, 1.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gAtlas);
        glBindVertexArray(batch.vao);
        glDrawArrays(GL_TRIANGLES, 0, batch.count);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &batch.vbo);
    glDeleteVertexArrays(1, &batch.vao);
    glDeleteTextures(1, &gAtlas);
    glfwTerminate();
}