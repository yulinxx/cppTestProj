// 把一个三角形沿 X、Y 各排 1000 个，生成 1 000 000 个实例的方阵。
// g++ main.cpp -lglfw -ldl -lGL -std=c++17

// 核心思想 
// OpenGL实例化就是用一份几何数据+多份参数，高效绘制多个相似物体
// 一个VBO存基础图形（如一个立方体的顶点）
// 另一个VBO存实例数据（如每个立方体的位置、颜色、缩放等）
// 顶点着色器自动组合，让每个实例有不同变化

// GPU绘制第0个实例时，gl_InstanceID=0
// 从实例VBO中取出第0个数据（如位置）
// 应用到这个实例的全部N个顶点上
// 移动到第1个实例，gl_InstanceID=1，重复...

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <iostream>

// ---------- 着色器 ----------
const char* vs = R"(#version 330 core
layout (location = 0) in vec2 aPos;   // 基础图形顶点
layout (location = 1) in mat4 instanceMatrix;   // 占用 4 个 attribute 槽 每个实例的位置矩阵
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
    GLFWwindow* w = glfwCreateWindow(1280, 720, "Instanced 1M", nullptr, nullptr);
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

    // VBO1: 基础图形（只需设置一次）
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

    // VBO2: 实例数据（每个实例一个矩阵）
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

        // 绘制 N 个实例，每个实例用相同的3个顶点
        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, NX * NY);

        glfwSwapBuffers(w); 
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &instanceVBO);
    glDeleteProgram(program);

    glfwTerminate();
}