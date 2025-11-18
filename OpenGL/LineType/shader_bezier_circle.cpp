// 用途：
// 此 C++ 文件利用 OpenGL、GLAD 和 GLFW 库实现了在 GPU 上绘制不同图形的功能。
// 主要支持绘制三阶贝塞尔曲线、圆和椭圆，通过将绘制逻辑放在 GLSL 着色器中，
// 由 CPU 向 GPU 传入相应的控制点、中心点、半径或长短轴等数据进行图形的计算和绘制。

// 使用说明：
// 1. 编译运行此程序前，请确保已经正确安装并配置了 OpenGL、GLAD、GLFW 和 GLM 库。
// 2. 程序运行后会弹出一个窗口，初始显示为三阶贝塞尔曲线。
// 3. 在程序运行过程中，你可以通过键盘输入以下按键来切换不同的绘制图形：
//    - 按下数字键 1：切换到绘制三阶贝塞尔曲线。
//    - 按下数字键 2：切换到绘制圆。
//    - 按下数字键 3：切换到绘制椭圆。
// 4. 关闭程序时，直接关闭窗口即可，程序会自动清理相关资源。

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <glm/glm.hpp> // 引入 GLM 库

// 生成着色器程序
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, nullptr);
    glCompileShader(vertexShader);

    // 检查顶点着色器编译错误
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex shader compilation error: " << infoLog << std::endl;
    }

    // 创建片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, nullptr);
    glCompileShader(fragmentShader);

    // 检查片段着色器编译错误
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment shader compilation error: " << infoLog << std::endl;
    }

    // 创建着色器程序
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 检查着色器程序链接错误
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
    }

    // 删除着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "GLFW initialization failed" << std::endl;
        return -1;
    }

    // 配置 GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(800, 600, "Bezier Curve", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
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

    // 修改顶点着色器代码
    const char* vertexShaderSource = R"(
        #version 330 core
        uniform int numPoints; // 新增：用于指定采样点数
        uniform vec2 controlPoints[4];
        uniform int shapeType; // 新增：用于指定图形类型
        uniform vec2 center;   // 新增：用于圆和椭圆的中心点
        uniform float radius;  // 新增：用于圆的半径
        uniform vec2 axes;     // 新增：用于椭圆的长短轴

        void main()
        {
            float t = float(gl_VertexID) / float(numPoints - 1); // 在顶点着色器中计算 t 值
            vec2 position;
            if (shapeType == 0) { // 贝塞尔曲线
                float u = 1 - t;
                float tt = t * t;
                float uu = u * u;
                float uuu = uu * u;
                float ttt = tt * t;

                position = uuu * controlPoints[0] +
                           3 * uu * t * controlPoints[1] +
                           3 * u * tt * controlPoints[2] +
                           ttt * controlPoints[3];
            } else if (shapeType == 1) { // 圆
                float angle = 2 * 3.1415926 * t;
                position = center + vec2(radius * cos(angle), radius * sin(angle));
            } else if (shapeType == 2) { // 椭圆
                float angle = 2 * 3.1415926 * t;
                position = center + vec2(axes.x * cos(angle), axes.y * sin(angle));
            }

            gl_Position = vec4(position, 0.0, 1.0);
        }
    )";

    // 片段着色器代码
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        void main()
        {
            FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
        }
    )";

    // 创建着色器程序
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // 定义三阶贝塞尔曲线的四个控制点
    std::vector<float> controlPoints = {
        -0.8f, -0.8f,
        -0.2f, 0.8f,
        0.2f, -0.8f,
        0.8f, 0.8f };

    // 移除生成参数 t 的值的代码
    const int numPoints = 100;
    // std::vector<float> tValues(numPoints);
    // for (int i = 0; i < numPoints; ++i)
    // {
    //     tValues[i] = static_cast<float>(i) / (numPoints - 1);
    // }

    // 创建 VAO 和 VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // 绑定 VAO 和 VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 这里不需要传递 tValues 数据
    // glBufferData(GL_ARRAY_BUFFER, tValues.size() * sizeof(float), tValues.data(), GL_STATIC_DRAW);

    // 设置顶点属性
    // 移除顶点属性设置
    // glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), (void *)0);
    // glEnableVertexAttribArray(0);

    // 解绑 VAO 和 VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    int shapeType = 0; // 默认为贝塞尔曲线

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 处理输入
        glfwPollEvents();

        // 新增：处理键盘输入
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            shapeType = 0; // 贝塞尔曲线
        }
        else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            shapeType = 1; // 圆
        }
        else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        {
            shapeType = 2; // 椭圆
        }

        // 清除颜色缓冲
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        // 设置采样点数的 uniform 变量
        const int numPoints = 100;
        GLint numPointsLoc = glGetUniformLocation(shaderProgram, "numPoints");
        if (numPointsLoc == -1)
        {
            std::cerr << "Failed to get uniform location for numPoints" << std::endl;
        }
        else
        {
            glUniform1i(numPointsLoc, numPoints);
        }

        // 设置图形类型的 uniform 变量
        GLint shapeTypeLoc = glGetUniformLocation(shaderProgram, "shapeType");
        if (shapeTypeLoc == -1)
        {
            std::cerr << "Failed to get uniform location for shapeType" << std::endl;
        }
        else
        {
            glUniform1i(shapeTypeLoc, shapeType);
        }

        if (shapeType == 0)
        {
            // 设置控制点的 uniform 变量
            GLint controlPointsLoc = glGetUniformLocation(shaderProgram, "controlPoints");
            if (controlPointsLoc == -1)
            {
                std::cerr << "Failed to get uniform location for controlPoints" << std::endl;
            }
            else
            {
                glUniform2fv(controlPointsLoc, 4, controlPoints.data());
            }
        }
        else if (shapeType == 1)
        {
            // 设置圆的中心点和半径
            glm::vec2 center = glm::vec2(0.0f, 0.0f); // 使用 GLM 的 vec2 类型
            float radius = 0.5f;
            GLint centerLoc = glGetUniformLocation(shaderProgram, "center");
            GLint radiusLoc = glGetUniformLocation(shaderProgram, "radius");
            if (centerLoc == -1)
            {
                std::cerr << "Failed to get uniform location for center" << std::endl;
            }
            else
            {
                glUniform2fv(centerLoc, 1, &center[0]);
            }
            if (radiusLoc == -1)
            {
                std::cerr << "Failed to get uniform location for radius" << std::endl;
            }
            else
            {
                glUniform1f(radiusLoc, radius);
            }
        }
        else if (shapeType == 2)
        {
            // 设置椭圆的中心点和长短轴
            glm::vec2 center = glm::vec2(0.0f, 0.0f); // 使用 GLM 的 vec2 类型
            glm::vec2 axes = glm::vec2(0.5f, 0.3f); // 使用 GLM 的 vec2 类型
            GLint centerLoc = glGetUniformLocation(shaderProgram, "center");
            GLint axesLoc = glGetUniformLocation(shaderProgram, "axes");
            if (centerLoc == -1)
            {
                std::cerr << "Failed to get uniform location for center" << std::endl;
            }
            else
            {
                glUniform2fv(centerLoc, 1, &center[0]);
            }
            if (axesLoc == -1)
            {
                std::cerr << "Failed to get uniform location for axes" << std::endl;
            }
            else
            {
                glUniform2fv(axesLoc, 1, &axes[0]);
            }
        }

        // 绑定 VAO
        glBindVertexArray(VAO);

        // 绘制曲线
        glDrawArrays(GL_LINE_STRIP, 0, numPoints);

        // 解绑 VAO
        glBindVertexArray(0);

        // 交换前后缓冲
        glfwSwapBuffers(window);
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    // 终止 GLFW
    glfwTerminate();
    return 0;
}