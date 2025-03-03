// 在 OpenGL 中，使用**图元重启（Primitive Restart）**是一种优化绘制多条折线（或多条线段、条带等）的技术。它允许你在单个索引缓冲区中通过插入一个特殊的索引值（通常是最大值，如 0xFFFFFFFF）来分隔不同的图元（如多条 GL_LINE_STRIP），从而避免多次调用绘制函数（如 glMultiDrawElements）或手动管理多个偏移和计数数组。

// 结合你的代码和需求（绘制多条连续的虚线折线，使用 GL_LINE_STRIP），我将对比图元重启与当前 glMultiDrawElements 的方式，并提供基于图元重启的实现代码，最后分析哪种方式更适合你的场景。

// 图元重启 vs 当前方式 (glMultiDrawElements)
// 当前方式：glMultiDrawElements
// 实现：使用 glMultiDrawElements 绘制多条独立的 GL_LINE_STRIP，通过 indicesOffsets 和 counts 指定每条折线的起始偏移和顶点数。
// 优点：
// 直观且易于理解，代码逻辑清晰。
// 不需要额外的硬件支持（图元重启需要 OpenGL 3.1+ 和驱动支持）。
// 索引数据无需特殊处理，直接按折线分段存储。
// 缺点：
// 需要额外的 CPU 端数组（indicesOffsets 和 counts），增加了内存开销和维护成本。
// 每次绘制需要传递多个参数，可能降低性能（尤其是折线数量很大时）。
// 不能充分利用 GPU 的批量处理能力。
// 图元重启方式
// 实现：在索引缓冲区中插入一个特殊值（如 0xFFFFFFFF）来分隔每条折线，然后使用单次 glDrawElements 调用绘制所有折线。
// 优点：
// 只需一次绘制调用（glDrawElements），减少 CPU 到 GPU 的通信开销。
// 索引缓冲区更紧凑，管理更简单，无需额外的偏移和计数数组。
// 在 GPU 端更高效，尤其适合大量折线的场景。
// 缺点：
// 需要启用图元重启功能（glEnable(GL_PRIMITIVE_RESTART)），并确保硬件支持。
// 索引数据中需要插入特殊值，稍微增加了一点生成复杂性。
// 如果调试不当，可能导致难以察觉的绘制错误（比如忘记启用图元重启）。


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <string>

constexpr float X = 4.0f;

const char* vertexShaderSource = R"(
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

GLuint loadShader(const char* vertexShaderSource, const char* fragmentShaderSource) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X) {
    return glm::vec2(
        minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX),
        minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY)
    );
}

void generateRandomMixedLines(
    std::vector<float>& vertices,
    std::vector<unsigned int>& flatIndices, // 直接生成展平的索引数组
    int numLines,
    int numSegments,
    int bezierSegments,
    float minX, float maxX, float minY, float maxY
) {
    vertices.clear();
    flatIndices.clear();

    for (int line = 0; line < numLines; ++line) {
        glm::vec2 startPoint = randomPoint(minX, maxX, minY, maxY);
        glm::vec2 currentPoint = startPoint;
        glm::vec2 prevPoint = startPoint;
        float dAccLen = 0.0;

        unsigned int vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
        vertices.push_back(startPoint.x);
        vertices.push_back(startPoint.y);
        vertices.push_back(dAccLen);
        flatIndices.push_back(vertexIndex);

        for (int i = 0; i < numSegments; ++i) {
            bool bLine = rand() % 2 == 0;

            if (bLine) {
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
            } else {
                glm::vec2 controlPoint1 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 controlPoint2 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 nextPoint = randomPoint(minX, maxX, minY, maxY);

                for (int j = 1; j <= bezierSegments; ++j) {
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
        // 在每条折线结束后插入图元重启标记
        if (line < numLines - 1) {
            flatIndices.push_back(0xFFFFFFFF); // 图元重启标记
        }
    }
}

float zoomFactor = 1.0f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoomFactor += float(yoffset) * 0.1f;
    zoomFactor = std::max(zoomFactor, 0.1f);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    srand(static_cast<unsigned int>(time(NULL)));

    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    GLFWwindow* window = glfwCreateWindow(1400, 1400, "OpenGL Dash Lines", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
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
        const int NUM_LINES = 6;
        const int NUM_SEGMENTS = 3;
        const int BEZIER_RES = 30;
        generateRandomMixedLines(shapeVertices, flatIndices, NUM_LINES, NUM_SEGMENTS, BEZIER_RES, -X, X, -X, X);
    }

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(unsigned int), flatIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 启用图元重启
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF); // 设置重启标记为 0xFFFFFFFF

    glClearColor(1.0, 1.0, 1.0, 1.0);

    while (!glfwWindowShouldClose(window)) {
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
        while ((err = glGetError()) != GL_NO_ERROR) {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDisable(GL_PRIMITIVE_RESTART); // 清理时禁用图元重启
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}



// 修改点说明
// 索引生成：
// generateRandomMixedLines 直接生成一个展平的 flatIndices 数组。
// 在每条折线结束后插入 0xFFFFFFFF 作为图元重启标记（仅在非最后一条折线后添加）。
// 启用图元重启：
// 添加 glEnable(GL_PRIMITIVE_RESTART) 和 glPrimitiveRestartIndex(0xFFFFFFFF)，告诉 OpenGL 在遇到 0xFFFFFFFF 时重启 GL_LINE_STRIP。
// 绘制调用：
// 使用单次 glDrawElements(GL_LINE_STRIP, flatIndices.size(), GL_UNSIGNED_INT, 0) 绘制所有折线，取代 glMultiDrawElements。
// 哪种方式更好？
// 你的场景分析
// 折线数量：当前代码生成 6 条折线，每条折线由 3 个段组成（混合直线和贝塞尔曲线）。
// 顶点数量：每条折线的顶点数约为 1 + numSegments * (bezierSegments 或 1)，即大约 91 个顶点（1 + 3 * 30）。总共约 546 个顶点，索引数稍多（包括重启标记）。
// 性能需求：你的代码有实时缩放和动画（timeOffset），需要较高的渲染效率。
// 比较与建议
// 性能：
// 图元重启：一次绘制调用，适合大量折线（数百或更多）。在你的场景（6 条折线）中，性能提升不明显，但随着折线数量增加优势会更显著。
// glMultiDrawElements：多次绘制调用（6 次），开销稍高，但在小规模场景下影响不大。
// 代码复杂性：
// 图元重启：索引生成略复杂（需插入重启标记），但绘制逻辑更简单。
// glMultiDrawElements：需要管理偏移和计数数组，代码更繁琐。
// 兼容性：
// 图元重启：需要 OpenGL 3.1+，你的代码已使用 3.3，应该没问题。
// glMultiDrawElements：更广泛支持，但你的场景已满足要求。
// 调试与维护：
// 图元重启：如果索引错误，可能导致整张图绘制混乱，调试稍难。
// glMultiDrawElements：每条折线独立，问题更容易定位。
// 推荐
// 当前场景（6 条折线）：两种方式性能差异不大，建议保留 glMultiDrawElements，因为它更直观，调试方便，且你的折线数量较少。
// 未来扩展（大量折线）：如果计划增加折线数量（例如数百条），切换到图元重启会更好，因为它减少了绘制调用的开销，GPU 效率更高。
// 如何选择
// 如果你优先考虑简单性和调试，继续使用 glMultiDrawElements（前一个版本）。
// 如果你追求性能和扩展性，采用上面的图元重启版本。



// 图元重启 vs glMultiDrawElements 详细对比
// 图元重启（Primitive Restart）
// 实现：所有线的索引存储在一个连续的缓冲区中，用特殊值（如 0xFFFFFFFF）分隔每条 GL_LINE_STRIP。
// 性能：
// 绘制调用：只需一次 glDrawElements，将整个索引缓冲区提交给 GPU。
// CPU 开销：极低，仅需单次调用，适合大规模数据。
// GPU 效率：高，GPU 可以一次性处理所有线条，充分利用硬件加速。
// 内存：
// 索引缓冲区大小约为顶点数 + 线条数（每个线条末尾加一个重启标记）。
// 例如：10,000 条线，每条 10 个顶点，总索引数 ≈ 110,000（100,000 个顶点索引 + 10,000 个重启标记）。
// 动态更新：
// 更新单条线需要修改整个索引缓冲区的一部分，复杂度较高（需要重新计算偏移并上传）。
// 优点：
// 极高的渲染效率，适合数万条线。
// 减少 CPU 到 GPU 的通信开销。
// 缺点：
// 数据管理稍复杂，尤其是动态更新时。
// 如果忘记启用图元重启或索引出错，可能导致绘制错误难以调试。
// glMultiDrawElements
// 实现：为每条线维护独立的索引范围，通过 counts 和 offsets 数组指定，多次绘制。
// 性能：
// 绘制调用：单次 glMultiDrawElements 调用，但需要传递数万个 counts 和 offsets 条目。
// CPU 开销：中等，需要准备并上传较大的偏移和计数数组（例如 10,000 条线需要 10,000 个 GLsizei 和 10,000 个偏移指针）。
// GPU 效率：稍逊于图元重启，因为 GPU 仍需处理多个独立绘制命令，尽管比多次单独 glDrawElements 好。
// 内存：
// 索引缓冲区仅存储顶点索引（无重启标记），约为 100,000 个索引。
// 额外需要 counts 和 offsets 数组，各占用 10,000 个元素，总计约 80KB（假设 GLsizei 和指针各 4 字节）。
// 动态更新：
// 更新单条线更容易，只需修改对应线的索引数据和 counts/offsets 数组中的一项。
// 优点：
// 数据管理更模块化，动态更新更方便。
// 每条线独立，调试和错误隔离更容易。
// 缺点：
// CPU 开销随线条数量线性增长，数万条线时可能成为瓶颈。
// 额外内存开销（偏移和计数数组）。
// 性能测试（理论估计）
// 图元重启：
// 索引缓冲区：~440KB（110,000 个 unsigned int，4 字节/个）。
// 绘制调用：1 次。
// CPU 到 GPU 数据传输：仅索引缓冲区 (~440KB)。
// glMultiDrawElements：
// 索引缓冲区：~400KB（100,000 个 unsigned int）。
// 偏移和计数数组：~80KB（10,000 个 GLsizei + 10,000 个指针）。
// 绘制调用：1 次，但参数复杂度更高。
// CPU 到 GPU 数据传输：~480KB（索引 + 偏移 + 计数）。
// 结论：图元重启在数据传输和 CPU 开销上略占优势，尤其在极大规模（数十万条线）时更明显。
// CAD 软件的推荐选择：图元重启
// 鉴于你的需求（数万条线，CAD 软件的高性能要求），图元重启 是更合适的选择。原因如下：

// 性能优先：
// 单次 glDrawElements 调用比 glMultiDrawElements 的参数传递更高效，尤其当线条数量达到数万时。
// GPU 可以一次性处理所有线条，避免额外的命令解析开销。
// 内存效率：
// 虽然索引缓冲区稍大（多了重启标记），但避免了 counts 和 offsets 数组的额外开销，总内存使用接近。
// 扩展性：
// 图元重启的性能不会因线条数量增加而显著下降，适合 CAD 软件的复杂场景。
// 如何优化图元重启以适应 CAD
// 动态更新：
// 使用动态缓冲区（GL_DYNAMIC_DRAW）存储顶点和索引，支持实时编辑。
// 对于单条线的修改，可以局部更新缓冲区（使用 glBufferSubData）。
// 批处理：
// 将线条分组（例如每 1000 条线一个缓冲区），结合图元重启，减少单次更新的数据量。
// 调试支持：
// 在开发阶段记录每条线的索引范围，便于定位问题