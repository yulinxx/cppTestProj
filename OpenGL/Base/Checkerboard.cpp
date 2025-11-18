#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <iostream>

// ----------------------------------------------------
// 着色器
// ----------------------------------------------------
const char* vertSrc = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
void main(){
    gl_Position = vec4(aPos, 0.0, 1.0);
})";

const char* fragSrc = R"(
#version 330 core
out vec4 fragColor;
uniform float uCellSize = 40;   // 每个格子多少像素

uniform vec3 uColorLight = vec3(1.0, 1.0, 1.0);
uniform vec3 uColorDark  = vec3(0.8, 0.8, 0.8);

void main(){
    ivec2 board = ivec2(gl_FragCoord.xy / uCellSize);
    bool black = (board.x + board.y) % 2 == 0;
    fragColor =vec4(black ? uColorDark : uColorLight, 1.0);
})";

// ----------------------------------------------------
// 工具：编译 + 链接
// ----------------------------------------------------
GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[512]; glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::cerr << "Shader compile error:\n" << log << std::endl;
    }
    return s;
}
GLuint createProgram()
{
    GLuint v = compileShader(GL_VERTEX_SHADER, vertSrc);
    GLuint f = compileShader(GL_FRAGMENT_SHADER, fragSrc);
    GLuint p = glCreateProgram();
    glAttachShader(p, v); glAttachShader(p, f);
    glLinkProgram(p);
    glDeleteShader(v); glDeleteShader(f);
    return p;
}

// ----------------------------------------------------
// 全局
// ----------------------------------------------------
float g_cellSize = 40.0f;   // 格子大小（像素）
GLuint g_prog, g_vao;

// ----------------------------------------------------
// 渲染
// ----------------------------------------------------
void render()
{
    int W, H;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &W, &H);
    glViewport(0, 0, W, H);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_prog);
    glUniform1f(glGetUniformLocation(g_prog, "uCellSize"), g_cellSize);

    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, 3);   // 全屏三角形
}

// ----------------------------------------------------
// 窗口大小回调
// ----------------------------------------------------
void framebuffer_size_callback(GLFWwindow*, int W, int H)
{
    glViewport(0, 0, W, H);
}

// ----------------------------------------------------
// main
// ----------------------------------------------------
int main()
{
    if (!glfwInit())
    {
        std::cerr << "GLFW init failed\n"; return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* win = glfwCreateWindow(800, 600, "Fixed-size Chessboard", nullptr, nullptr);
    if (!win)
    {
        std::cerr << "Create window failed\n"; glfwTerminate(); return -1;
    }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "GLAD load failed\n"; return -1;
    }
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    // 1. 着色器
    g_prog = createProgram();

    // 2. 全屏三角形 VAO
    float tri[] = { -1,-1,  3,-1,  -1,3 };
    GLuint vbo;
    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    // 3. 主循环
    while (!glfwWindowShouldClose(win))
    {
        render();
        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    // 4. 清理
    glDeleteVertexArrays(1, &g_vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(g_prog);
    glfwTerminate();
    return 0;
}