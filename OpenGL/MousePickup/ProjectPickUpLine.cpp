// 启动后，显示50个随机分布的四边形线框（不同深度和颜色）。
// 鼠标左键点击线框或其内部区域，控制台输出Selected polygon: <ID>，选中线框变为黄色。
// 深度测试确保远近关系正确。
// -----------------------------------------------
// 要实现鼠标点击闭合线框（例如一个闭合的多边形）内部时也能选中该线框，我们需要修改之前的程序逻辑。
// 之前的代码使用颜色拾取技术来检测鼠标点击是否落在三角形上，但对于闭合线框（例如使用GL_LINE_LOOP绘制的多边形），
// 直接拾取线条可能不够精确，因为线条的像素宽度有限，点击线框内部的区域不会触发拾取。为了解决这个问题，我们可以使用以下方法：

// 将闭合线框视为填充的多边形进行拾取：

// 在拾取模式下，将闭合线框渲染为填充的多边形（使用GL_TRIANGLE_FAN或GL_POLYGON），这样点击线框内部的区域也会拾取到对应的颜色。
// 在正常渲染模式下，仍然绘制为线框（GL_LINE_LOOP）。

// 颜色拾取技术：

// 继续使用颜色拾取，分配唯一的颜色ID给每个闭合线框。
// 鼠标点击时，读取像素颜色，确定选中的线框。

// 修改数据结构：

// 将三角形改为多边形（顶点数可变），支持闭合线框。
// 每个多边形存储顶点列表、显示颜色和拾取颜色。

// 透视投影和深度：

// 保留透视投影和不同深度，确保拾取考虑Z缓冲。

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <random>

// 窗口尺寸
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// 多边形结构体（闭合线框）
struct Polygon
{
    std::vector<glm::vec3> vertices; // 顶点列表
    glm::vec3 color;                 // 显示颜色
    glm::vec3 pickColor;             // 拾取颜色
};

// 全局变量
std::vector<Polygon> polygons;
unsigned int VBO, VAO, shaderProgram, pickShaderProgram;
glm::mat4 projection, view;
bool pickMode = false;
int selectedPolygon = -1;

// 检查OpenGL错误
void checkGLError(const char* operation)
{
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
    {
        std::cerr << "OpenGL error after " << operation << ": " << error << std::endl;
    }
}

// 鼠标点击回调
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        pickMode = true;
    }
}

// 着色器源码
const char* vertexShaderSource = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 460 core
out vec4 FragColor;
uniform vec3 color;
void main() {
    FragColor = vec4(color, 1.0);
}
)";

const char* pickFragmentShaderSource = R"(
#version 460 core
out vec4 FragColor;
uniform vec3 pickColor;
void main() {
    FragColor = vec4(pickColor, 1.0);
}
)";

// 编译着色器
unsigned int compileShader(const char* source, GLenum type)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error (" << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment") << "): " << infoLog << std::endl;
        return 0;
    }
    return shader;
}

// 创建着色器程序
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    unsigned int vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (!vertexShader || !fragmentShader)
    {
        return 0;
    }
    unsigned int program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Program linking error: " << infoLog << std::endl;
        return 0;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

// 初始化多边形（闭合线框）
void initPolygons()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);
    std::uniform_real_distribution<float> colorDist(0.0f, 1.0f);
    std::uniform_real_distribution<float> zDist(-10.0f, 0.0f);
    std::uniform_real_distribution<float> sizeDist(0.5f, 1.5f); // 控制线框大小

    polygons.clear();
    for (int i = 0; i < 50; ++i)
    {
        Polygon poly;
        // 生成一个四边形（可改为其他顶点数）
        float centerX = posDist(gen);
        float centerY = posDist(gen);
        float z = zDist(gen);
        float size = sizeDist(gen);
        int numVertices = 4; // 四边形
        for (int j = 0; j < numVertices; ++j)
        {
            float angle = 2.0f * glm::pi<float>() * j / numVertices;
            float x = centerX + size * cos(angle);
            float y = centerY + size * sin(angle);
            poly.vertices.push_back(glm::vec3(x, y, z));
        }
        poly.color = glm::vec3(colorDist(gen), colorDist(gen), colorDist(gen));
        // 拾取颜色：将索引i编码为RGB
        poly.pickColor = glm::vec3(
            ((i & 0xFF0000) >> 16) / 255.0f,
            ((i & 0x00FF00) >> 8) / 255.0f,
            (i & 0x0000FF) / 255.0f
        );
        polygons.push_back(poly);
    }
}

// 初始化缓冲区
bool initBuffers()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    if (VAO == 0 || VBO == 0)
    {
        std::cerr << "Failed to generate VAO or VBO" << std::endl;
        return false;
    }
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    checkGLError("initBuffers");
    return true;
}

// 渲染场景
void renderScene(bool picking)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    unsigned int program = picking ? pickShaderProgram : shaderProgram;
    glUseProgram(program);
    checkGLError("glUseProgram");

    glUniformMatrix4fv(glGetUniformLocation(program, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    checkGLError("glBindVertexArray");

    for (size_t i = 0; i < polygons.size(); ++i)
    {
        const Polygon& poly = polygons[i];
        glm::mat4 model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));

        if (picking)
        {
            glUniform3fv(glGetUniformLocation(program, "pickColor"), 1, glm::value_ptr(poly.pickColor));
            // 拾取模式：渲染为填充多边形
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, poly.vertices.size() * sizeof(glm::vec3), poly.vertices.data(), GL_STATIC_DRAW);
            checkGLError("glBufferData (pick)");
            glDrawArrays(GL_TRIANGLE_FAN, 0, poly.vertices.size());
            checkGLError("glDrawArrays (pick)");
        }
        else
        {
            glm::vec3 displayColor = (i == selectedPolygon) ? glm::vec3(1.0f, 1.0f, 0.0f) : poly.color;
            glUniform3fv(glGetUniformLocation(program, "color"), 1, glm::value_ptr(displayColor));
            // 正常模式：渲染为线框
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, poly.vertices.size() * sizeof(glm::vec3), poly.vertices.data(), GL_STATIC_DRAW);
            checkGLError("glBufferData (render)");
            glDrawArrays(GL_LINE_LOOP, 0, poly.vertices.size());
            checkGLError("glDrawArrays (render)");
        }
    }
    // 恢复默认填充模式
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

int main()
{
    // 初始化GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Line Loop Picking", nullptr, nullptr);
    if (!window)
    {
        const char* description;
        glfwGetError(&description);
        std::cerr << "Failed to create GLFW window: " << description << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwDestroyWindow(window);
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

    // 设置视口和深度测试
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    checkGLError("glEnable(GL_DEPTH_TEST)");

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 创建着色器程序
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    pickShaderProgram = createShaderProgram(vertexShaderSource, pickFragmentShaderSource);
    if (shaderProgram == 0 || pickShaderProgram == 0)
    {
        std::cerr << "Failed to create shader programs" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 初始化多边形和缓冲区
    initPolygons();
    if (!initBuffers())
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    // 设置投影和视图矩阵
    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        if (pickMode)
        {
            // 拾取模式
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            renderScene(true);
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            ypos = SCR_HEIGHT - ypos; // 翻转y轴

            unsigned char pixel[3];
            glReadPixels((int)xpos, (int)ypos, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, pixel);
            checkGLError("glReadPixels");

            // 解码拾取颜色
            int pickedID = (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];
            if (pickedID < polygons.size())
            {
                selectedPolygon = pickedID;
                std::cout << "Selected polygon: " << selectedPolygon << std::endl;
            }
            else
            {
                selectedPolygon = -1;
            }
            pickMode = false;
        }

        // 正常渲染
        renderScene(false);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(pickShaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}