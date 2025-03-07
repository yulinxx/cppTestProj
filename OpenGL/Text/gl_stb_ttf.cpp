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
#include <iostream>
#include <vector>
#include <fstream>

const int WIDTH = 800;
const int HEIGHT = 600;

GLuint shaderProgram;
GLuint VAO, VBO, texture;

stbtt_bakedchar charData[96];
GLuint fontTexture;

std::vector<unsigned char> readFontFile(const std::string& filename)
{
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file)
    {
        exit(EXIT_FAILURE);
    }
    size_t size = file.tellg();
    std::vector<unsigned char> buffer(size);
    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    return buffer;
}

void loadFontTexture(const std::string& fontPath)
{
    auto fontBuffer = readFontFile(fontPath);

    unsigned char bitmap[512 * 512] = { 0 };
    stbtt_BakeFontBitmap(fontBuffer.data(), 0, 48.0, bitmap, 512, 512, 32, 96, charData);

    glGenTextures(1, &fontTexture);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 512, 512, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec4 vertex;
out vec2 TexCoords;
uniform mat4 projection;

void main() {
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 FragColor;
uniform sampler2D text;
void main() {
    float alpha = texture(text, TexCoords).r;
    FragColor = vec4(1.0, 1.0, 1.0, alpha);
}
)";

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
    }
    return shader;
}

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

    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void renderText(const std::string& text, float x, float y, float scale, GLuint texture)
{
    glUseProgram(shaderProgram);
    glBindTexture(GL_TEXTURE_2D, texture);

    std::vector<float> vertices;
    float startX = x;
    for (char c : text)
    {
        if (c < 32 || c > 126) continue;

        stbtt_bakedchar* ch = &charData[c - 32];
        float x0 = startX + ch->xoff * scale;
        float y0 = y - ch->yoff * scale;
        float x1 = x0 + (ch->x1 - ch->x0) * scale;
        float y1 = y0 - (ch->y1 - ch->y0) * scale;

        float s0 = ch->x0 / 512.0f, t0 = ch->y0 / 512.0f;
        float s1 = ch->x1 / 512.0f, t1 = ch->y1 / 512.0f;

        vertices.insert(vertices.end(), {
            x0, y0, s0, t0,
            x1, y0, s1, t0,
            x1, y1, s1, t1,

            x0, y0, s0, t0,
            x1, y1, s1, t1,
            x0, y1, s0, t1
            });

        startX += ch->xadvance * scale;
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 4);
}

int main()
{
    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Font Rendering", NULL, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    initOpenGL();

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    // TTF
    loadFontTexture("C:/Windows/Fonts/arial.ttf");

    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        renderText("你好 Hello World!", 100.0f, 300.0f, 1.0f, fontTexture);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}