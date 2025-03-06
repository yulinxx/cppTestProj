// 图元重启的优化代码
// 以下是针对 CAD 场景优化的代码，增加了动态缓冲支持：

// cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <string>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

constexpr float X = 4.0f;

const char* vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 in_pos;
layout(location = 1) in float in_len;

uniform mat4 cameraTrans;
uniform float dashScale;
uniform float timeOffset = 0.0;

out float dashParam;

void main() {
    gl_Position = cameraTrans * vec4(in_pos, 0.0, 1.0);
    float dashLength = in_len * dashScale + timeOffset;
    dashParam = dashLength;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in float dashParam;
uniform vec4 color;
uniform int dashType = 0;
out vec4 fragColor;

void main() {
    bool draw = false;
    float pattern;
    float cycle;

    switch(dashType) {
        case 0: // 默认等长虚线
            pattern = mod(dashParam, 1.0);
            draw = (pattern < 0.5);
            break;
        default:
            draw = true;
            break;
    }

    if (!draw) discard;
    fragColor = color;
}
)";

GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X)
{
    return glm::vec2(
        minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX),
        minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY)
    );
}

void generateRandomMixedLines(
    std::vector<float>& vertices,
    std::vector<unsigned int>& flatIndices,
    int numLines,
    int numSegments,
    int bezierSegments,
    float minX, float maxX, float minY, float maxY
)
{
    vertices.clear();
    flatIndices.clear();

    for (int line = 0; line < numLines; ++line)
    {
        glm::vec2 startPoint = randomPoint(minX, maxX, minY, maxY);
        glm::vec2 currentPoint = startPoint;
        glm::vec2 prevPoint = startPoint;
        float dAccLen = 0.0;

        unsigned int vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
        vertices.push_back(startPoint.x);
        vertices.push_back(startPoint.y);
        vertices.push_back(dAccLen);
        flatIndices.push_back(vertexIndex);

        for (int i = 0; i < numSegments; ++i)
        {
            bool bLine = rand() % 2 == 0;

            if (bLine)
            {
                glm::vec2 point = randomPoint(minX, maxX, minY, maxY);
                vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                vertices.push_back(point.x);
                vertices.push_back(point.y);

                float segmentLength = glm::distance(prevPoint, point);
                dAccLen += segmentLength;
                vertices.push_back(dAccLen);

                flatIndices.push_back(vertexIndex);

                prevPoint = point;
                currentPoint = point;
            }
            else
            {
                glm::vec2 controlPoint1 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 controlPoint2 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 nextPoint = randomPoint(minX, maxX, minY, maxY);

                for (int j = 1; j <= bezierSegments; ++j)
                {
                    float t = float(j) / float(bezierSegments);
                    float u = 1.0f - t;

                    glm::vec2 point = u * u * u * currentPoint +
                        3.0f * u * u * t * controlPoint1 +
                        3.0f * u * t * t * controlPoint2 +
                        t * t * t * nextPoint;

                    vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                    vertices.push_back(point.x);
                    vertices.push_back(point.y);

                    float segmentLength = glm::distance(prevPoint, point);
                    dAccLen += segmentLength;
                    vertices.push_back(dAccLen);

                    flatIndices.push_back(vertexIndex);

                    prevPoint = point;
                }
                currentPoint = nextPoint;
            }
        }
        if (line < numLines - 1)
        {
            flatIndices.push_back(0xFFFFFFFF); // 图元重启标记
        }
    }
}

float zoomFactor = 1.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoomFactor += float(yoffset) * 0.1f;
    zoomFactor = std::max(zoomFactor, 0.1f);
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    srand(static_cast<unsigned int>(time(NULL)));

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1400, 1400, "CAD Lines", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetScrollCallback(window, scroll_callback);

    GLuint shaderProgram = loadShader(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), 8.0f);

    std::vector<float> shapeVertices;
    std::vector<unsigned int> flatIndices;

    {
        const int NUM_LINES = 10000; // 模拟 CAD 中的数万条线
        const int NUM_SEGMENTS = 3;
        const int BEZIER_RES = 10; // 减少细分以模拟更多线条
        generateRandomMixedLines(shapeVertices, flatIndices, NUM_LINES, NUM_SEGMENTS, BEZIER_RES, -X, X, -X, X);
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_DYNAMIC_DRAW); // 使用动态缓冲

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(unsigned int), flatIndices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);

    glClearColor(1.0, 1.0, 1.0, 1.0);

    while (!glfwWindowShouldClose(window))
    {
        glm::mat4 cameraTrans = glm::ortho(-X * zoomFactor, X * zoomFactor, -X * zoomFactor, X * zoomFactor);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

        float dashScale = 12.0f / zoomFactor;
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        float time = duration.count();
        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawElements(GL_LINE_STRIP, flatIndices.size(), GL_UNSIGNED_INT, 0);

        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        static int i = 0;
        i++;
        if (i % 4000 == 0 && false)
        {
            int width, height;
            glfwGetWindowSize(window, &width, &height);
            std::vector<unsigned char> pixels(width * height * 4);
            glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            stbi_flip_vertically_on_write(1);
            std::string filename = "frame_" + std::to_string(i) + ".png";
            stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDisable(GL_PRIMITIVE_RESTART);
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

// 使用动态缓冲 有哪些好处

// 在 OpenGL 中使用动态缓冲区（通常通过 GL_DYNAMIC_DRAW 或 GL_STREAM_DRAW 指定）相比静态缓冲区（GL_STATIC_DRAW）有特定的优势，尤其在需要频繁更新数据的场景下，比如你的 CAD 软件中涉及数万条线的实时编辑和渲染。以下是使用动态缓冲区的主要好处，以及在你的具体场景中的意义：

// 使用动态缓冲区的好处
// 1. 支持高效的数据更新
// 好处：动态缓冲区允许你频繁修改缓冲区内容（如顶点数据或索引数据），而无需重新分配整个缓冲区。使用 glBufferSubData 或 glMapBuffer 可以直接更新缓冲区的特定部分。
// CAD 场景意义：
// 用户在 CAD 软件中编辑线条（添加、删除、移动顶点）时，只需更新受影响的顶点或索引，而不是重新上传所有数据。
// 例如，修改一条线的某个顶点，只需更新 shapeVertices 中对应的 3 个 float（x, y, dAccLen），而不是重新生成整个缓冲区。
// 2. 减少内存分配开销
// 好处：动态缓冲区在创建时分配一次内存，后续更新不会触发新的内存分配或释放操作。相比之下，频繁使用 glBufferData（静态缓冲区常用）会导致内存重新分配和数据拷贝，开销较大。
// CAD 场景意义：
// 在数万条线的场景中（例如 100,000 个顶点 ≈ 1.2MB 数据），每次编辑都重新分配缓冲区会显著降低性能。
// 动态缓冲区避免了这种开销，保持内存使用的稳定性。
// 3. 优化 GPU 性能
// 好处：动态缓冲区提示驱动程序数据会频繁更改，GPU 可以优化内存管理和缓存策略（例如避免长期缓存数据）。
// CAD 场景意义：
// CAD 软件的实时交互（缩放、平移、编辑）需要快速响应。动态缓冲区确保 GPU 能够高效处理更新后的数据，避免因缓存过期或重新加载导致的延迟。
// 4. 灵活性更高
// 好处：动态缓冲区支持部分更新，可以只修改缓冲区中的一小段数据，而静态缓冲区通常假设数据不变，更新时需要替换全部内容。
// CAD 场景意义：
// 例如，你可能只更新某条线的索引（flatIndices 中的一段），而其他线的索引保持不变。使用 glBufferSubData 可以精确更新这部分数据，减少传输量。
// 5. 适合流式数据
// 好处：如果使用 GL_STREAM_DRAW（动态缓冲的一种变体），它特别适合数据只使用一次或几次后就更新的场景（如流式生成的数据）。
// CAD 场景意义：
// 如果 CAD 软件支持实时生成线条（例如用户拖动画线），GL_STREAM_DRAW 能进一步优化性能。不过你的场景更倾向于多次渲染，GL_DYNAMIC_DRAW 更合适。

// 与静态缓冲区的对比

// 特性      	静态缓冲区 (GL_STATIC_DRAW)	    动态缓冲区 (GL_DYNAMIC_DRAW)
// 数据更新频率	    很少或从不更新	                频繁更新
// 更新方式	    通常替换整个缓冲区	                支持部分更新
// 内存分配	    一次性，可能优化为 GPU 内存	        保留灵活性，避免重复分配
// 性能开销	    初始化快，更新慢	                初始化稍慢，更新快
// 典型用途	    固定几何体（如模型）	            动态几何体（如 CAD 线条）

// 进一步优化的建议
// 分块管理：
// 将数万条线分成多个缓冲区（例如每 1000 条线一个 VBO/EBO），只更新受影响的块，减少单次传输的数据量。
// 双缓冲：
// 使用两个动态缓冲区，一个用于渲染，一个用于更新，交替切换以避免渲染和更新冲突。
// 映射缓冲区：
// 使用 glMapBuffer 直接修改缓冲区内容，避免额外的拷贝：
// cpp
// 換行
// 複製
// glBindBuffer(GL_ARRAY_BUFFER, VBO);
// float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
// data[125 * 3] = newPoint.x;
// data[125 * 3 + 1] = newPoint.y;
// data[125 * 3 + 2] = newLen;
// glUnmapBuffer(GL_ARRAY_BUFFER);
// 批量更新：
// 累积多次编辑，然后一次性更新缓冲区，减少 glBufferSubData 调用次数。

// 总结
// 在你的 CAD 软件中，使用动态缓冲区的好处包括：

// 高效更新：支持实时编辑线条，适合交互式应用。
// 性能优化：减少内存分配和数据传输开销，适应数万条线的规模。
// 灵活性：为未来的动态功能（如动画、用户输入）奠定基础。
// 相比静态缓冲区，动态缓冲区是 CAD 场景的理想选择。如果你需要实现具体的动态更新功能（例如编辑某条线），我可以进一步提供代码示例！有什么具体需求吗？