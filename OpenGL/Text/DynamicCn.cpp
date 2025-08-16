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
#include <stdexcept>

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
    GLint success;
    glGetShaderiv(v, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(v, 512, nullptr, infoLog);
        std::cerr << "VERTEX SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }
    GLuint f = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f, 1, &fs, nullptr);
    glCompileShader(f);
    glGetShaderiv(f, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(f, 512, nullptr, infoLog);
        std::cerr << "FRAGMENT SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }
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
int gPenX = 0, gPenY = 0, gRowH = 0;

FT_Library gFt;
FT_Face gFace;
int gFontH;

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

// 初始化FreeType和纹理图集
void initTextSystem(const std::string& fontPath, int fontH)
{
    gFontH = fontH;
    if (FT_Init_FreeType(&gFt))
    {
        throw std::runtime_error("Could not init FreeType Library");
    }
    if (FT_New_Face(gFt, fontPath.c_str(), 0, &gFace))
    {
        throw std::runtime_error("Failed to load font");
    }
    FT_Set_Pixel_Sizes(gFace, 0, gFontH);

    // 创建初始的空纹理图集
    gW = 512; gH = 512;
    glGenTextures(1, &gAtlas);
    glBindTexture(GL_TEXTURE_2D, gAtlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gW, gH, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

// 动态载入单个字符并打包到纹理图集
void loadAndPackGlyph(unsigned int cp)
{
    if (gGlyphs.count(cp) > 0) return; // 如果字符已经存在，则直接返回

    if (FT_Load_Char(gFace, cp, FT_LOAD_RENDER))
    {
        return; // 加载失败则跳过
    }
    FT_Bitmap& bmp = gFace->glyph->bitmap;

    // 检查当前行是否还有空间，或者是否需要扩大纹理
    if (gPenX + bmp.width + 1 >= gW)
    {
        gPenX = 0;
        gPenY += gRowH + 1;
        gRowH = 0;
        if (gPenY + gFontH >= gH)
        {
            // 纹理图集空间不足，扩大一倍
            gW *= 2; gH *= 2;
            // 重新分配和复制纹理数据
            std::vector<unsigned char> newAtlas(gW * gH, 0);
            glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, newAtlas.data());
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, gW, gH, 0, GL_RED, GL_UNSIGNED_BYTE, newAtlas.data());
        }
    }
    gRowH = std::max(gRowH, static_cast<int>(bmp.rows));

    // 将字形数据拷贝到纹理图集
    glBindTexture(GL_TEXTURE_2D, gAtlas);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexSubImage2D(GL_TEXTURE_2D, 0, gPenX, gPenY, bmp.width, bmp.rows, GL_RED, GL_UNSIGNED_BYTE, bmp.buffer);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    // 存储字符信息
    gGlyphs[cp] = { { bmp.width, bmp.rows },
                    { gFace->glyph->bitmap_left, gFace->glyph->bitmap_top },
                    static_cast<GLuint>(gFace->glyph->advance.x >> 6),
                    { gPenX, gPenY } };
    gPenX += bmp.width + 1;
}

// ---------- 静态批处理（现在是动态生成） ----------
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
        // 在这里动态载入每个字符，确保它存在于图集中
        loadAndPackGlyph(cp);

        // 如果加载失败，跳过该字符
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
            xpos,    ypos + h, u0, v0,
            xpos,    ypos,    u0, v1,
            xpos + w, ypos,    u1, v1,
            xpos,    ypos + h, u0, v0,
            xpos + w, ypos,    u1, v1,
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
    GLFWwindow* win = glfwCreateWindow(900, 200, "FreeType Dynamic Batch + 中文", nullptr, nullptr);
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // -------------------- 主要改动 --------------------
    // 1. 初始化文字系统：现在只初始化FreeType和创建一个空的纹理图集
    try
    {
        // 使用一个支持大部分中文字符的字体文件
        initTextSystem("C:/Windows/Fonts/simsun.ttc", 48);
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        glfwTerminate();
        return -1;
    }

    GLuint prog = makeProgram();

    // 2. 创建批处理：现在makeBatch会自动加载它需要的字符
    Batch batch = makeBatch("这个示例现在可以动态加载所有你想渲染的中文例如你好世界", 1.0f);

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
    FT_Done_Face(gFace);
    FT_Done_FreeType(gFt);
    glfwTerminate();
}
