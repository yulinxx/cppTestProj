#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

const char* vertex_shader_src =
"#version 400\n"
"layout (location = 0) in vec2 position;\n"
"layout (location = 1) in vec2 texcoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    TexCoord = texcoord;\n"
"}\n";

const char* fragment_shader_src =
"#version 400\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D tex;\n"
"uniform vec3 color;\n"
"void main() {\n"
"    float alpha = texture(tex, TexCoord).r;\n"
"    FragColor = vec4(color, alpha);\n"
"}\n";

struct Character {
    GLuint textureID;
    int width;
    int height;
    int bearingX;
    int bearingY;
    int advance;
};

std::map<unsigned int, Character> characters;

void check_shader_error(GLuint shader)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }
}

unsigned int utf8_to_uint32(const char* str, int* len)
{
    unsigned char c = (unsigned char)str[0];
    if (c < 0x80)
    {
        *len = 1;
        return c;
    }
    else if ((c & 0xE0) == 0xC0)
    {
        *len = 2;
        return ((c & 0x1F) << 6) | (str[1] & 0x3F);
    }
    else if ((c & 0xF0) == 0xE0)
    {
        *len = 3;
        return ((c & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
    }
    else if ((c & 0xF8) == 0xF0)
    {
        *len = 4;
        return ((c & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
    }
    *len = 1;
    return c;
}

void loadCharacter(FT_Face face, unsigned int charCode)
{
    if (characters.find(charCode) != characters.end())
        return;

    FT_UInt glyph_index = FT_Get_Char_Index(face, charCode);
    if (glyph_index == 0)
    {
        glyph_index = FT_Get_Char_Index(face, 0xFFFD);
        if (glyph_index == 0) return;
    }

    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER))
    {
        fprintf(stderr, "Failed to load glyph: 0x%X\n", charCode);
        return;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RED,
        face->glyph->bitmap.width,
        face->glyph->bitmap.rows,
        0,
        GL_RED,
        GL_UNSIGNED_BYTE,
        face->glyph->bitmap.buffer
    );

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    Character ch = {
        texture,
        (int)face->glyph->bitmap.width,
        (int)face->glyph->bitmap.rows,
        (int)face->glyph->bitmap_left,
        (int)face->glyph->bitmap_top,
        (int)(face->glyph->advance.x >> 6)
    };
    characters[charCode] = ch;
}

void initFont(const char* fontPath, int fontSize, const char* text)
{
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        fprintf(stderr, "Failed to init FreeType\n");
        return;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face))
    {
        fprintf(stderr, "Failed to load font: %s\n", fontPath);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, fontSize);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    const char* p = text;
    while (*p)
    {
        int charLen = 0;
        unsigned int charCode = utf8_to_uint32(p, &charLen);
        loadCharacter(face, charCode);
        p += charLen;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

int main()
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Chinese Text Demo", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    const char* text = "你好,世界! Hello World!";
    initFont("C:/Windows/Fonts/simhei.ttf", 48, text);

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertexShader);
    check_shader_error(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragment_shader_src, NULL);
    glCompileShader(fragmentShader);
    check_shader_error(fragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glUniform3f(glGetUniformLocation(program, "color"), 1.0f, 1.0f, 1.0f);

        glActiveTexture(GL_TEXTURE0);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);

        glBindVertexArray(VAO);

        float x = 50.0f;
        float y = 300.0f;

        const char* p = text;
        while (*p)
        {
            int charLen = 0;
            unsigned int charCode = utf8_to_uint32(p, &charLen);

            if (characters.find(charCode) == characters.end())
            {
                p += charLen;
                continue;
            }

            Character& ch = characters[charCode];

            float xpos = x + ch.bearingX;
            float ypos = y - (ch.height - ch.bearingY);
            float w = (float)ch.width;
            float h = (float)ch.height;

            float vertices[6][4] = {
                {xpos,     ypos + h, 0.0f, 0.0f},
                {xpos,     ypos,     0.0f, 1.0f},
                {xpos + w, ypos,     1.0f, 1.0f},
                {xpos,     ypos + h, 0.0f, 0.0f},
                {xpos + w, ypos,     1.0f, 1.0f},
                {xpos + w, ypos + h, 1.0f, 0.0f}
            };

            for (int j = 0; j < 6; j++)
            {
                vertices[j][0] = (vertices[j][0] / WINDOW_WIDTH) * 2.0f - 1.0f;
                vertices[j][1] = (vertices[j][1] / WINDOW_HEIGHT) * 2.0f - 1.0f;
            }

            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
            glDrawArrays(GL_TRIANGLES, 0, 6);

            x += ch.advance;
            p += charLen;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (auto& pair : characters)
    {
        glDeleteTextures(1, &pair.second.textureID);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}
