/*
运行后：左键拖动相机，滚轮缩放，观察三种线段行为对比（白色随缩放变长短，红色固定不动，绿色移动但长度固定）。

“固定屏幕长度线段”（比如调试线、UI线、血条、瞄准线等）
脱离场景的正交矩阵（ViewProj），改用 UI 矩阵（或者叫屏幕矩阵、NDC矩阵）来绘制

不能用场景的正交矩阵（ViewProj），它会跟随 zoom 缩放  
必须使用 UI/屏幕矩阵（ortho(0,width,height,0)）或直接在着色器里转 NDC  
最简单可靠的做法：顶点直接传像素坐标，着色器里除以屏幕分辨率转NDC  
这样画出来的线段长度永远是固定像素，完全不受相机缩放影响

这就是所有游戏引擎（Unity、Unreal、Godot）的 Debug.DrawLine、Gizmos、UI线条的实现原理。

线段跟随相机平移，但不跟随缩放
比如：角色头顶血条、名字（跟随平移，但长度不随相机 zoom 变长变短）
用“固定像素投影 + 只平移的view”这种专用的 fixedSizeMatrix

*/





#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <cmath>
#include <cstring>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ------------------- 手写 4x4 矩阵和常用函数 -------------------
typedef struct
{
    float m[16];
} mat4;

static float mat4_get(mat4 a, int i, int j)
{
    return a.m[i + j * 4];
}

mat4 mat4_identity()
{
    mat4 r = { 0 };
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

mat4 mat4_mul(mat4 a, mat4 b)
{
    mat4 r = { 0 };
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                r.m[i + j * 4] += mat4_get(a, i, k) * mat4_get(b, k, j);
    return r;
}

mat4 mat4_translate(float x, float y, float z)
{
    mat4 r = mat4_identity();
    r.m[12] = x; r.m[13] = y; r.m[14] = z;
    return r;
}

// 正交投影：left, right, bottom, top, near, far
mat4 mat4_ortho(float l, float r, float b, float t, float n, float f)
{
    mat4 m = { 0 };
    m.m[0] = 2.0f / (r - l);
    m.m[5] = 2.0f / (t - b);
    m.m[10] = -2.0f / (f - n);
    m.m[12] = -(r + l) / (r - l);
    m.m[13] = -(t + b) / (t - b);
    m.m[14] = -(f + n) / (f - n);
    m.m[15] = 1.0f;
    return m;
}

// ------------------- 全局变量 -------------------
int winW = 1400, winH = 1200;
float camX = 0.0f, camY = 0.0f;
float zoom = 1.0f;               // 1.0 = 原始大小，>1 放大，<1 缩小
double lastMx = 0, lastMy = 0;
bool dragging = false;

// 三个专门的矩阵
mat4 scenePV;      // 普通场景矩阵（随缩放）
mat4 uiPV;         // 纯 UI 矩阵（固定像素）
mat4 fixedPV;      // 关键：随平移但不随缩放

void update_matrices()
{
    // 1. 普通场景矩阵（受 zoom 影响）
    float halfW = winW * 0.5f / zoom;
    float halfH = winH * 0.5f / zoom;
    mat4 proj = mat4_ortho(-halfW, halfW, -halfH, halfH, -1000.0f, 1000.0f);
    mat4 view = mat4_translate(-camX, -camY, 0.0f);
    scenePV = mat4_mul(proj, view);

    // 2. 纯 UI 矩阵（永远 1:1 像素，不随平移缩放）
    uiPV = mat4_ortho(0.0f, (float)winW, (float)winH, 0.0f, -1000.0f, 1000.0f);

    // 3. 关键矩阵：随相机平移，但投影固定为 1:1 像素（长度不随 zoom）
    mat4 fixedProj = mat4_ortho(0.0f, (float)winW, (float)winH, 0.0f, -1000.0f, 1000.0f);
    mat4 fixedView = mat4_translate(-camX, camY, 0.0f);
    fixedPV = mat4_mul(fixedProj, fixedView);
}

// ------------------- Shader 源码 -------------------
const char* vertexSrc = R"(
#version 460 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aColor;

uniform mat4 u_PV;  // Projection * View

out vec4 vColor;

void main() {
    gl_Position = u_PV * vec4(aPos, 0.0, 1.0);
    vColor = aColor;
}
)";

const char* fragSrc = R"(
#version 460 core
in vec4 vColor;
out vec4 FragColor;

void main() {
    FragColor = vColor;
}
)";

// ------------------- 编译 shader -------------------
GLuint compile_shader(const char* src, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetShaderInfoLog(shader, 512, NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
    }
    return shader;
}

// ------------------- 鼠标回调 -------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            dragging = true;
            glfwGetCursorPos(window, &lastMx, &lastMy);
        }
        else if (action == GLFW_RELEASE)
        {
            dragging = false;
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (dragging)
    {
        double dx = xpos - lastMx;
        double dy = ypos - lastMy;
        camX -= dx / zoom;  // 反向移动相机
        camY += dy / zoom;  // y轴方向根据坐标系调整
        lastMx = xpos;
        lastMy = ypos;
        update_matrices();
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoom += yoffset * 0.1f;
    if (zoom < 0.1f) zoom = 0.1f;
    update_matrices();
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    winW = width;
    winH = height;
    glViewport(0, 0, width, height);
    update_matrices();
}

// ------------------- 绘制一条线段 -------------------
void draw_line(GLuint vao, GLuint vbo, mat4 pv, 
    float x1, float y1, float x2, float y2, 
    float r, float g, float b, float a)
{
    float vertices[] =
    {
        x1, y1, r, g, b, a,
        x2, y2, r, g, b, a
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    glUniformMatrix4fv(glGetUniformLocation(vao, "u_PV"), 1, GL_FALSE, pv.m);  // 注意：program 绑定后

    glDrawArrays(GL_LINES, 0, 2);
}

// ------------------- 主函数 -------------------
int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(winW, winH, "OpenGL Line Demo", NULL, NULL);
    if (!window)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGL())
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return -1;
    }

    glViewport(0, 0, winW, winH);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 编译 shader program
    GLuint vs = compile_shader(vertexSrc, GL_VERTEX_SHADER);
    GLuint fs = compile_shader(fragSrc, GL_FRAGMENT_SHADER);
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char log[512];
        glGetProgramInfoLog(program, 512, NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);

    // 设置 VAO VBO
    GLuint vao, vbo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), NULL, GL_DYNAMIC_DRAW);  // 够两条点

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 初始化矩阵
    update_matrices();

    glUseProgram(program);
    glBindVertexArray(vao);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制三种线段（每种 200 单位长，但行为不同）

        // 1. 白色：普通场景线段，随平移+缩放（世界坐标）
        draw_line(program, vbo, scenePV, 0.0f, 10.0f, 200.0f, 10.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        // 2. 红色：纯 UI 线段，固定屏幕像素，不随任何变化（屏幕坐标）
        draw_line(program, vbo, uiPV, 50.0f, 50.0f, 250.0f, 50.0f, 1.0f, 0.0f, 0.0f, 1.0f);

        // 3. 绿色：随平移但不随缩放（世界坐标，但用 fixedPV）
        draw_line(program, vbo, fixedPV, 0.0f, 30.0f, 200.0f, 30.0f, 0.0f, 1.0f, 0.0f, 1.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwTerminate();
    return 0;
}