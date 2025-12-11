// glad_opengl46_three_kinds_of_lines.cpp
// g++ main.cpp glad.c -lglfw -ldl -o demo
// 运行：./demo
// 按键：ESC 退出；鼠标滚轮可改相机缩放（仅影响第 3 条线）

/*
在OpenGL里画一条长度固定的线段，让它不受正交投影缩放影响，
线段顶点在 CPU 里就用像素坐标（或 NDC）算好，直接送进管线，不要再乘任何“世界 / 视图 / 缩放”矩阵。
如果你用 glm 等库，只要把 MVP 里的 V 和 P 都设为单位矩阵，或者干脆只传一个 UI 矩阵
（通常就是一个把像素坐标映射到 NDC 的正交矩阵，不带任何缩放 / 平移的“相机”概念），就能保证线段长度永远是多少像素就是多少像素。

如果窗口大小变了怎么办？
只改 glm::ortho(0, w, 0, h, ...) 里的 w, h 即可，线段长度仍然固定。
我想让线段跟随相机平移，但不跟随缩放，可以吗？
可以，把 view 矩阵的 平移部分 保留，但把 缩放部分 扔掉（或者把模型矩阵的缩放设回 1）。


UI 矩阵 → 固定像素大小、跟随窗口左下角（不随相机动）
UI 矩阵 → 固定像素大小、跟随相机平移（不随相机缩放）
方向键移动相机
普通投影矩阵 → 随相机缩放 + 平移（世界坐标线）
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <cmath>
#include <cstring>

// -------------------- 手写 4x4 矩阵小工具 --------------------
struct Mat4
{
    float m[16];   // 列主序，OpenGL 风格
    Mat4()
    {
        identity();
    }

    void identity()
    {
        for (int i = 0; i < 16; ++i)
            m[i] = 0.0f;

        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    const float* ptr() const
    {
        return m;
    }
};

// 矩阵乘法：ret = a * b
Mat4 operator*(const Mat4& a, const Mat4& b)
{
    Mat4 ret;
    for (int i = 0; i < 4; ++i) // 列
    {
        for (int j = 0; j < 4; ++j) // 行
        {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k)
                sum += a.m[k * 4 + j] * b.m[i * 4 + k];

            ret.m[i * 4 + j] = sum;
        }
    }
    return ret;
}

// 手写正交投影：left,right,bottom,top,near,far
Mat4 ortho(float l, float r, float b, float t, float n, float f)
{
    Mat4 mat;
    mat.m[0] = 2.0f / (r - l);
    mat.m[5] = 2.0f / (t - b);
    mat.m[10] = -2.0f / (f - n);
    mat.m[12] = -(r + l) / (r - l);
    mat.m[13] = -(t + b) / (t - b);
    mat.m[14] = -(f + n) / (f - n);
    // m[15] 已在构造函数中设为 1
    return mat;
}

// 手写平移矩阵
Mat4 translate(float x, float y)
{
    Mat4 mat;
    mat.m[12] = x;
    mat.m[13] = y;
    return mat;
}

// 手写缩放矩阵
Mat4 scale(float sx, float sy)
{
    Mat4 mat;
    mat.m[0] = sx;
    mat.m[5] = sy;
    return mat;
}

// -------------------- 全局变量 --------------------
int g_width = 800, g_height = 600;
float g_camX = 0.0f, g_camY = 0.0f;   // 相机中心（世界坐标）
float g_zoom = 1.0f;                  // 相机缩放因子（只影响第3条线）

GLuint buildShader(const char* vsSrc, const char* fsSrc)
{
    auto compile = [](GLenum type, const char* src) -> GLuint {
        GLuint sh = glCreateShader(type);
        glShaderSource(sh, 1, &src, nullptr);
        glCompileShader(sh);
        GLint ok;
        glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);

        if (!ok)
        {
            char log[512];
            glGetShaderInfoLog(sh, 512, nullptr, log);
            printf("Shader compile error:\n%s\n", log);
            exit(1);
        }
        return sh;
        };

    GLuint vs = compile(GL_VERTEX_SHADER, vsSrc);
    GLuint fs = compile(GL_FRAGMENT_SHADER, fsSrc);

    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    GLint ok;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);

    if (!ok)
    {
        char log[512];
        glGetProgramInfoLog(prog, 512, nullptr, log);
        printf("Program link error:\n%s\n", log);
        exit(1);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// 键盘回调：方向键移动相机
void keyCB(GLFWwindow* w, int key, int, int action, int)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(w, GLFW_TRUE);

    float speed = 20.0f;
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_LEFT:  g_camX -= speed; break;
        case GLFW_KEY_RIGHT: g_camX += speed; break;
        case GLFW_KEY_UP:    g_camY += speed; break;
        case GLFW_KEY_DOWN:  g_camY -= speed; break;
        }
    }
}

// 滚轮回调：只改缩放因子，不动 UI 矩阵
void scrollCB(GLFWwindow*, double, double y)
{
    float s = y > 0 ? 1.1f : 0.9f;
    g_zoom *= s;

    if (g_zoom < 0.1f) g_zoom = 0.1f;
    if (g_zoom > 10.0f) g_zoom = 10.0f;
}

int main()
{
    if (!glfwInit())
    {
        puts("GLFW init error"); 
        return 0;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(g_width, g_height, "UI And Ortho Matrix",nullptr, nullptr);

    if (!win)
    {
        puts("Window create error"); 
        return 0;
    }

    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD 初始化失败！" << std::endl;
        return false;
    }

    printf("OpenGL %s\n", glGetString(GL_VERSION));

    glfwSetKeyCallback(win, keyCB);
    glfwSetScrollCallback(win, scrollCB);

    // 极简着色器：只乘一个 mat4
    const char* vs = R"GLSL(
#version 460 core
layout(location = 0) in vec2 aPos;
uniform mat4 uMVP;
void main(){
    gl_Position = uMVP * vec4(aPos, 0.0, 1.0);
}
)GLSL";

    const char* fs = R"GLSL(
#version 460 core
uniform vec3 uColor;
out vec4 FragColor;
void main(){
    FragColor = vec4(uColor, 1.0);
}
)GLSL";

    GLuint prog = buildShader(vs, fs);

    const float P = 100.0f;  // 固定像素长度

    // 红色线：固定窗口左下角 y=400 处（明显可见）
    float line1[4] = { 50, 400, 50 + P, 400 };

    // 绿色线：固定像素长度，但 y=10 抬高避免被底边裁剪
    // 初始在 (0,10) 到 (100,10)，会跟随相机移动
    float line2[4] = { 0, 10, P, 10 };

    // 蓝色线：世界坐标，放大到200单位长度，放在 (0, -50) 处
    // 这样缩放时能明显看到变化
    float line3[4] = { -100.0f, -50.0f, 100.0f, -50.0f };
    // ======================================================================

    GLuint vbo[3], vao[3];
    glGenBuffers(3, vbo);
    glGenVertexArrays(3, vao);
    for (int i = 0; i < 3; ++i)
    {
        glBindVertexArray(vao[i]);
        glBindBuffer(GL_ARRAY_BUFFER, vbo[i]);
        glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(float),
            i == 0 ? line1 : (i == 1 ? line2 : line3), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glUseProgram(prog);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // 深色背景更易观察

    // 获取 uniform 位置（只需一次）
    GLint locMVP = glGetUniformLocation(prog, "uMVP");
    GLint locColor = glGetUniformLocation(prog, "uColor");

    while (!glfwWindowShouldClose(win))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        int w, h;
        glfwGetFramebufferSize(win, &w, &h);
        glViewport(0, 0, w, h);
        g_width = w; g_height = h;

        // 每帧打印状态
        printf("Camera: (%.1f, %.1f)  Zoom: %.2f  Window: %dx%d\r",
            g_camX, g_camY, g_zoom, w, h);

        fflush(stdout);

        // -------- 1. 红色线：固定窗口左下角，不随相机动 --------
        // MVP = 纯UI投影矩阵（单位矩阵 * 正交投影）
        Mat4 ui = ortho(0, w, 0, h, -1, 1);
        glUniformMatrix4fv(locMVP, 1, GL_FALSE, ui.ptr());
        glUniform3f(locColor, 1, 0, 0);  // 红色
        glBindVertexArray(vao[0]);
        glDrawArrays(GL_LINES, 0, 2);

        // -------- 2. 绿色线：固定像素大小，跟随相机平移 --------
        // MVP = UI投影 * 视图平移（无缩放）
        Mat4 ui2 = ortho(0, w, 0, h, -1, 1);
        Mat4 view = translate(-g_camX, -g_camY);  // 只平移相机
        ui2 = ui2 * view;
        glUniformMatrix4fv(locMVP, 1, GL_FALSE, ui2.ptr());
        glUniform3f(locColor, 0, 1, 0);  // 绿色
        glBindVertexArray(vao[1]);
        glDrawArrays(GL_LINES, 0, 2);

        // -------- 3. 蓝色线：普通世界坐标，会被缩放 --------
        // MVP = 世界投影(带缩放) * 视图平移
        float halfW = w / 2.0f * g_zoom;
        float halfH = h / 2.0f * g_zoom;

        Mat4 world = ortho(-halfW, halfW, -halfH, halfH, -1, 1);
        Mat4 worldView = translate(-g_camX, -g_camY);
        world = world * worldView;

        glUniformMatrix4fv(locMVP, 1, GL_FALSE, world.ptr());
        glUniform3f(locColor, 0, 0, 1);  // 蓝色
        glBindVertexArray(vao[2]);
        glDrawArrays(GL_LINES, 0, 2);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    // 清理
    glDeleteProgram(prog);
    glDeleteVertexArrays(3, vao);
    glDeleteBuffers(3, vbo);
    glfwTerminate();
    printf("\n");
    return 0;
}