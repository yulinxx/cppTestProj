// 帧缓冲就是 OpenGL 的"画布"——默认画布是屏幕，但你可以创建自己的"离屏画布"，在上面画完后再贴回屏幕或当作纹理使用。

// 1. 组成部分
// 一个完整的帧缓冲（FBO）由多个附件组成：
// 表格
// 复制
// 附件类型	作用	                存储方式
// 颜色附件	存储 RGBA 颜色	       纹理（Texture）或 渲染缓冲（Renderbuffer）
// 深度附件	存储深度值（Z-buffer）	纹理或渲染缓冲
// 模板附件	存储模板值（Stencil）	渲染缓冲（也可组合到深度纹理）

// 默认帧缓冲 = 窗口系统创建的、直接显示到屏幕的帧缓冲。
// 自定义帧缓冲 = 你手动创建的、离屏渲染用的 FBO。

// 为什么需要它？
// 场景 1：后处理特效
// 你想给整个场景加模糊/辉光/色调调整。
// → 先把场景画到 FBO，再把 FBO 当纹理画到屏幕，中间加 Shader 处理。
// 场景 2：阴影贴图
// 想生成阴影？得先把光源视角的深度画到一张纹理上。
// → 创建深度附件为纹理的 FBO，渲染一次，得到阴影贴图。
// 场景 3：镜子/反射
// 镜面需要反射场景。
// → 在 FBO 里画一次反射视角，再把结果贴到镜面模型上。
// 场景 4：UI 组合
// 多个 UI 元素混合，不想污染主场景的深度。
// → 在 FBO 里画 UI，最后整体贴回屏幕。
//  帧缓冲示例：
// 先渲染一个旋转的彩色立方体场景到 FBO，然后对 FBO 纹理进行灰度后处理，最后显示到屏幕。
// 可以按 空格键 切换后处理效果。
// 帧缓冲 = 离屏渲染容器，"先画别处，再贴回来"，是实现后处理、阴影、反射等。

// | 特性      | 纹理（Texture）             |  渲染缓冲（Renderbuffer）             |
// | ------- | ---------------------------- | ------------------------------------ |
// | **可读性** | 可当纹理采样（`sampler2D`） | 只能作为附件，**不能**在 Shader 中读取  |
// | **性能**  | 稍慢（需支持采样）           | 更快（专用内存布局）                   |
// | **用途**  | 需后续使用（后处理、阴影贴图）| 仅需存储（深度/模板）                  |

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

// 顶点着色器 - 场景（3D立方体）
const char* sceneVertexShader = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

// 片段着色器 - 场景
const char* sceneFragmentShader = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

// 顶点着色器 - 后处理全屏四边形
const char* screenVertexShader = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(aPos, 0.0, 1.0);
    texCoord = aTexCoord;
}
)";

// 片段着色器 - 灰度后处理
const char* screenFragmentShader = R"(
#version 330 core
in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform bool enableGrayscale;

void main()
{
    vec3 color = texture(screenTexture, texCoord).rgb;

    if (enableGrayscale)
    {
        // 灰度转换（加权平均）
        float gray = dot(color, vec3(0.299, 0.587, 0.114));
        FragColor = vec4(gray, gray, gray, 1.0);
    }
    else
    {
        FragColor = vec4(color, 1.0);
    }
}
)";

// 全局变量
GLuint sceneShader, screenShader;
GLuint cubeVAO, quadVAO;
GLuint fbo, textureColorbuffer, rbo;
bool enableGrayscale = true;

// 编译着色器
GLuint compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cout << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

// 创建着色器程序
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << "Program linking failed: " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// 初始化场景数据
void initScene()
{
    // 创建立方体几何体
    float cubeVertices[] = {
        // 位置             // 颜色
        -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
        // ... 省略其余顶点，实际应该有36个
    };

    GLuint VBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// 初始化全屏四边形
void initQuad()
{
    float quadVertices[] = {
        // 位置   // 纹理坐标
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    GLuint VBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

// 初始化帧缓冲
void initFramebuffer(int width, int height)
{
    // 创建FBO
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // 创建颜色附件（纹理）
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    // 创建深度和模板附件（渲染缓冲对象）
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    // 检查完整性
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// 渲染场景到FBO:渲染 3D 立方体到帧缓冲纹理
void renderSceneToFBO()
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(sceneShader);

    // 设置矩阵
    float time = glfwGetTime();
    QMatrix4x4 model, view, projection;
    model.rotate(time * 50, QVector3D(0.5f, 1.0f, 0.0f));
    view.translate(QVector3D(0.0f, 0.0f, -3.0f));
    projection.perspective(45.0f, 800.0f / 600.0f, 0.1f, 100.0f);

    GLint modelLoc = glGetUniformLocation(sceneShader, "model");
    GLint viewLoc = glGetUniformLocation(sceneShader, "view");
    GLint projLoc = glGetUniformLocation(sceneShader, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data());
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data());
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.data());

    // 绘制立方体
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

// 渲染四边形到屏幕 将纹理渲染到全屏四边形并应用后处理
void renderQuadToScreen()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(screenShader);
    glBindVertexArray(quadVAO);

    // 设置纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    GLint screenTextureLoc = glGetUniformLocation(screenShader, "screenTexture");
    glUniform1i(screenTextureLoc, 0);

    // 设置灰度开关
    GLint grayscaleLoc = glGetUniformLocation(screenShader, "enableGrayscale");
    glUniform1i(grayscaleLoc, enableGrayscale);

    // 绘制全屏四边形
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

// 初始化
void init()
{
    sceneShader = createShaderProgram(sceneVertexShader, sceneFragmentShader);
    screenShader = createShaderProgram(screenVertexShader, screenFragmentShader);

    initScene();
    initQuad();
    initFramebuffer(800, 600);
}

// 主渲染函数
void render()
{
    renderSceneToFBO();
    renderQuadToScreen();
}

// 键盘回调
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        enableGrayscale = !enableGrayscale;
    }
}

// 窗口大小调整回调
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    // 重新创建FBO
    initFramebuffer(width, height);
}

int main()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        std::cout << "GLFW initialization failed" << std::endl;
        return -1;
    }

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Framebuffer Demo - Press SPACE to toggle grayscale", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Window creation failed" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD initialization failed" << std::endl;
        return -1;
    }

    init();

    glEnable(GL_DEPTH_TEST);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &textureColorbuffer);
    glDeleteRenderbuffers(1, &rbo);
    glDeleteProgram(sceneShader);
    glDeleteProgram(screenShader);

    glfwTerminate();
    return 0;
}