// 深度缓冲只能回答“谁在前面”，而模板缓冲能回答“谁有资格被画”。
// 深度测试 = 安检门（看你是不是够近）。
// 模板测试 = 门禁卡（看你是不是有权限）。
// 新来的片元 → 模板测试(读缓冲) → 通过？→ 深度测试 → 通过？→ 写颜色
//                                  ↓                           ↓
//                             操作缓冲值                操作缓冲值
// 把模板缓冲想成“施工围挡上的喷漆编号”：
// 深度测试 = “近的挡远的”（物理遮挡）。
// 模板测试 = “只准在编号为 1 的区域作业”（权限控制）。


// 渲染流程：
// 第一遍：绘制红色三角形到模板缓冲（glStencilOp(GL_REPLACE) 将三角形区域的模板值设为1）
// 第二遍：绘制绿色三角形时，模板测试设置为 GL_EQUAL，只有模板值为1的像素才会被绘制
// 关键设置：
// glStencilFunc(GL_ALWAYS, 1, 0xFF)：第一遍总是通过测试
// glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE)：通过测试时替换模板值
// glStencilFunc(GL_EQUAL, 1, 0xFF)：第二遍只有模板值等于1才通过
// glColorMask(false...)：第一遍只写模板，不写颜色
// 效果： 绿色三角形被“裁剪”在红色三角形的区域内，只会在红色三角形的轮廓内显示。

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 顶点着色器
const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";

// 片段着色器 - 红色模板区域
const char* fragmentShaderRed = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
}
)";

// 片段着色器 - 绿色被遮罩对象
const char* fragmentShaderGreen = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(0.0, 1.0, 0.0, 0.8); // 绿色，带透明度
}
)";

// 三角形顶点数据
float triangleVertices[] = {
    // 红色模板三角形（大）
     0.5f,  0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
     0.0f, -0.5f, 0.0f,

    // 绿色三角形（小）
     0.3f,  0.3f, 0.0f,
    -0.3f,  0.3f, 0.0f,
     0.3f, -0.3f, 0.0f,
};

unsigned int indices[] = {
    0, 1, 2,  // 红色三角形
    3, 4, 5   // 绿色三角形
};

// 着色器编译函数
unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) 
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader compilation error: " << infoLog << std::endl;
    }
    return shader;
}

// 着色器程序链接函数
unsigned int createShaderProgram(const char* fragmentSource) {
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << "Program linking error: " << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

int main() {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cout << "GLFW initialization failed" << std::endl;
        return -1;
    }
    
    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Stencil Buffer Demo", nullptr, nullptr);
    if (!window) {
        std::cout << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    
    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "GLAD initialization failed" << std::endl;
        return -1;
    }
    
    // 创建着色器程序
    unsigned int redProgram = createShaderProgram(fragmentShaderRed);
    unsigned int greenProgram = createShaderProgram(fragmentShaderGreen);
    
    // 设置顶点数据和缓冲
    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    // 主循环
    while (!glfwWindowShouldClose(window)) 
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        
        // 1. 绘制模板区域（红色大三角形）
        // 第一遍：绘制红色三角形到模板缓冲（glStencilOp(GL_REPLACE) 将三角形区域的模板值设为1）
        glUseProgram(redProgram);
        glBindVertexArray(VAO);
        
        // 设置模板测试参数
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 1, 0xFF);  // 总是通过，参考值设为1
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // 测试通过时将模板值写入缓冲区
        glStencilMask(0xFF); // 允许写入模板缓冲
        
        // 禁止写入颜色缓冲，只写模板
        // 各通道全是False,则只画模板形状，不画颜色
        // void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        
        // 绘制红色三角形（作为模板）
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
        
        // 2. 绘制被遮罩的对象（绿色小三角形）
        // 第二遍：绘制绿色三角形时，模板测试设置为 GL_EQUAL，只有模板值为1的像素才会被绘制
        glUseProgram(greenProgram);
        
        // 重新启用颜色写入,全 GL_TRUE：正常情况下所有通道都可写。
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        
        // 设置模板测试：只绘制模板值为1的区域
        glStencilFunc(GL_EQUAL, 1, 0xFF); // 只有模板值等于1时才通过
        glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP); // 不改变模板缓冲
        glStencilMask(0x00); // 禁止写入模板缓冲
        
        // 启用混合，使绿色三角形半透明
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // 绘制绿色三角形（只在红色区域内显示）
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(unsigned int)));
        
        // 禁用模板测试和混合
        glDisable(GL_STENCIL_TEST);
        glDisable(GL_BLEND);
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    // 清理
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(redProgram);
    glDeleteProgram(greenProgram);
    
    glfwTerminate();
    return 0;
}