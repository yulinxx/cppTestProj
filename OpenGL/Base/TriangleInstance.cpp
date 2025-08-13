// 把一个三角形沿 X、Y 各排 1000 个，生成 1 000 000 个实例的方阵。
// g++ main.cpp -lglfw -ldl -lGL -std=c++17
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

// ---------- 着色器 ----------
const char* vs = R"(#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in mat4 instanceMatrix;   // 占用 4 个 attribute 槽
void main() {
    gl_Position = instanceMatrix * vec4(aPos, 0.0, 1.0);
})";

const char* fs = R"(#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.0, 0.6, 1.0, 1.0);
})";

GLuint compile(const char* src, GLenum type)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);

    if (!ok)
    {
        char log[512];
        glGetShaderInfoLog(s, 512, nullptr, log);
        std::cerr << log;
    }
    return s;
}

GLuint prog()
{
    GLuint v = compile(vs, GL_VERTEX_SHADER);
    GLuint f = compile(fs, GL_FRAGMENT_SHADER);
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    GLint ok;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);

    if (!ok)
    {
        char log[512];
        glGetProgramInfoLog(p, 512, nullptr, log);
        std::cerr << log;
    }

    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// ---------- GLFW ----------
void resize(GLFWwindow*, int w, int h)
{
    glViewport(0, 0, w, h);
}
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* w = glfwCreateWindow(800, 600, "Instanced 1M", nullptr, nullptr);
    glfwMakeContextCurrent(w);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glfwSetFramebufferSizeCallback(w, resize);

    GLuint program = prog();

    // ---------- 单个三角形 ----------
    float tri[] = { -0.005f,-0.005f,  0.005f,-0.005f,  0.0f, 0.005f };
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    // ---------- 生成 1M 个实例矩阵 ----------
    const int NX = 1000, NY = 1000;
    std::vector<glm::mat4> matrices;
    matrices.reserve(NX * NY);

    for (int y = 0; y < NY; ++y)
    {
        for (int x = 0; x < NX; ++x)
        {
            glm::mat4 m(1.0f);
            m = glm::translate(m, glm::vec3(
                (x - NX / 2) * 0.011f,
                (y - NY / 2) * 0.011f,
                0.0f));

            matrices.push_back(m);
        }
    }

    GLuint instanceVBO;
    glGenBuffers(1, &instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, matrices.size() * sizeof(glm::mat4), matrices.data(), GL_STATIC_DRAW);

    // 设置实例化属性：mat4 占 4 个 vec4
    for (int i = 0; i < 4; ++i)
    {
        glEnableVertexAttribArray(1 + i);
        glVertexAttribPointer(1 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
            (void*)(i * sizeof(glm::vec4)));

        glVertexAttribDivisor(1 + i, 1);   // 关键：按实例更新
    }

    // ---------- 渲染循环 ----------
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    while (!glfwWindowShouldClose(w))
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(program);
        glBindVertexArray(VAO);
        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, NX * NY);

        glfwSwapBuffers(w); glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteProgram(program);

    glfwTerminate();
}