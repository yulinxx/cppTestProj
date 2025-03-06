// 在你的 CAD 软件中，当前将曲线离散成短线段（例如通过贝塞尔曲线细分）后传给 GPU 的做法确实会显著增加数据量，尤其是在处理数万条线时。
// 这种方法将计算负担放在了 CPU 端，而 GPU 的强大并行计算能力未被充分利用。
// 直接将直线和曲线的控制点传给 GPU，让 GPU 负责绘制曲线是一个非常可行的优化方向。这种方法可以减少数据传输量，提高渲染效率，同时保留高质量的曲线绘制。

// 以下我会分析这种方法的优势、可行性，并提供实现思路和代码示例。

// 当前方法的局限性
// 数据量大：每条曲线被离散成多个短线段（例如贝塞尔曲线细分 10 次生成 11 个顶点），导致顶点和索引数量激增。例如：
// 10,000 条线，每条平均 3 段（1 直线 + 2 曲线），每条曲线离散为 10 段，总顶点数 ≈ 10,000 × (1 + 2 × 10) ≈ 210,000。
// 数据大小 ≈ 210,000 × 3 × 4 字节 ≈ 2.5MB。
// CPU 开销：离散化过程需要在 CPU 端计算所有顶点，增加了预处理时间。
// 带宽压力：大量顶点数据需要从 CPU 传输到 GPU，占用宝贵的内存带宽。
// 优化思路：将控制点传给 GPU
// 目标：将直线的两个端点和曲线的控制点（例如贝塞尔曲线的 4 个控制点）直接传给 GPU，由 GPU 的着色器（通常是Tessellation Shader 或 Geometry Shader）动态生成曲线上的点。
// 优势：
// 减少数据量：
// 直线：2 个顶点。
// 三次贝塞尔曲线：4 个控制点。
// 示例：10,000 条线，每条 3 段（1 直线 + 2 曲线），总顶点数 ≈ 10,000 × (2 + 4 × 2) = 100,000，数据大小 ≈ 1.2MB，比离散化少一半以上。
// 降低 CPU 负担：
// CPU 只需提供控制点，曲线计算移到 GPU，充分利用 GPU 的并行性。
// 动态质量：
// GPU 可以根据缩放级别（LOD，Level of Detail）动态调整曲线的细分程度，避免远距离时生成过多不必要的顶点。
// 带宽优化：
// 传输数据量减少，减轻 CPU 到 GPU 的通信压力。
// 可行性与实现方式
// OpenGL 提供了几种方法让 GPU 绘制曲线：

// 1. 使用 Tessellation Shader（推荐）
// 概述：Tessellation Shader（镶嵌着色器）是 OpenGL 4.0+ 引入的功能，专门用于动态细分几何体，非常适合生成曲线。
// 流程：
// 将直线的两个端点和曲线的控制点作为输入（GL_PATCHES 图元）。
// Tessellation Control Shader (TCS) 指定细分级别。
// Tessellation Evaluation Shader (TES) 根据控制点计算曲线上的点（例如三次贝塞尔曲线公式）。
// 要求：需要 OpenGL 4.0+，你的硬件和驱动需支持。
// 适用性：CAD 软件需要高质量曲线和高性能渲染，Tessellation 是最佳选择。
// 2. 使用 Geometry Shader
// 概述：Geometry Shader 可以从输入的控制点生成新的顶点，形成曲线上的线段。
// 流程：
// 输入控制点（例如 GL_POINTS 或 GL_LINES）。
// Geometry Shader 动态生成曲线上的点，输出 GL_LINE_STRIP。
// 要求：OpenGL 3.2+，你的当前代码基于 3.3，已满足。
// 适用性：适合中等复杂度的曲线，但性能不如 Tessellation，尤其在大量曲线时。
// 3. 使用 Compute Shader
// 概述：Compute Shader 可以预计算曲线点，存储到缓冲区，再用常规 Vertex Shader 绘制。
// 流程：
// 输入控制点到 Compute Shader。
// Compute Shader 计算离散点，写入缓冲区。
// 使用 GL_LINE_STRIP 绘制。
// 要求：OpenGL 4.3+。
// 适用性：灵活性高，但需要额外的缓冲区管理，不如 Tessellation 直接。
// 推荐方案：Tessellation Shader
// 理由：
// Tessellation Shader 专为动态几何生成设计，性能和质量最佳。
// 你的 CAD 软件需要处理数万条线，Tessellation 支持动态细分，能根据缩放级别优化渲染。
// OpenGL 4.0+ 在现代硬件上广泛支持（你的 NVIDIA GPU 应没问题）。
// 数据结构：
// 每条直线：2 个顶点（x, y）。
// 每条贝塞尔曲线：4 个控制点（x, y）。
// 使用 GL_PATCHES 图元，区分直线和曲线。

// 实现要点
// 数据结构：
// LineSegment 区分直线（2 个点）和曲线（4 个控制点）。
// 顶点只存储控制点坐标 (x, y)，dashParam 在 TES 中计算。
// Tessellation Shader：
// TCS 根据输入顶点数（2 或 4）判断是直线还是曲线，设置细分级别。
// TES 根据贝塞尔公式生成曲线点，或直接插值直线点。
// 动态细分：
// tessLevel 根据缩放级别调整，避免远距离时生成过多顶点。
// 图元重启：
// 保留 0xFFFFFFFF 分隔不同段。
// 优化效果
// 数据量：从 210,000 个顶点减少到 100,000 个控制点，节省约 50% 数据。
// 性能：GPU 动态生成曲线点，减少 CPU 计算和传输开销。
// 质量：曲线平滑度随缩放自适应，适合 CAD 的精确需求。
// 注意事项
// 硬件要求：需要 OpenGL 4.0+，请确认你的开发环境和目标硬件支持。
// 调试：Tessellation Shader 可能需要调试，确保曲线生成正确。
// 扩展：可添加更多曲线类型（如二次贝塞尔、样条曲线），只需调整 TES。

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <string>

constexpr float X = 4.0f; // 初始世界坐标范围（-X 到 X）

// Vertex Shader: 传递控制点位置和累计长度
const char* vertexShaderSource = R"(
#version 400 core
layout(location = 0) in vec2 in_pos;   // 输入控制点坐标
layout(location = 1) in float in_len;  // 输入累计长度

out vec2 v_pos;    // 传递给 Tessellation 的位置
out float v_len;   // 传递给 Tessellation 的长度

void main() {
    v_pos = in_pos;
    v_len = in_len;
}
)";

// Tessellation Control Shader: 设置细分级别
const char* tessControlShaderSource = R"(
#version 400 core
layout(vertices = 4) out;

uniform float tessLevel = 10.0;  // 细分级别，动态调整

in vec2 v_pos[];
in float v_len[];

out vec2 tc_pos[];
out float tc_len[];

void main() {
    tc_pos[gl_InvocationID] = v_pos[gl_InvocationID];
    tc_len[gl_InvocationID] = v_len[gl_InvocationID];

    if (gl_InvocationID == 0) {
        if (gl_in.length() == 2) {  // 直线：无需细分
            gl_TessLevelOuter[0] = 1.0;
            gl_TessLevelOuter[1] = 1.0;
        } else {  // 曲线：根据 tessLevel 细分
            gl_TessLevelOuter[0] = tessLevel;
            gl_TessLevelOuter[1] = tessLevel;
        }
    }
}
)";

// Tessellation Evaluation Shader: 计算曲线上的点和长度
const char* tessEvaluationShaderSource = R"(
#version 400 core
layout(isolines, equal_spacing) in;

uniform mat4 cameraTrans;       // 相机变换矩阵
uniform float dashScale = 8.0;  // 虚线缩放因子
uniform float timeOffset = 0.0; // 时间偏移，用于虚线动画

in vec2 tc_pos[];
in float tc_len[];

out float dashParam;  // 传递给 Fragment Shader 的虚线参数

void main() {
    float u = gl_TessCoord.x;

    vec4 pos;
    float len;

    if (gl_in.length() == 2) {  // 直线插值
        pos = mix(vec4(tc_pos[0], 0.0, 1.0), vec4(tc_pos[1], 0.0, 1.0), u);
        len = mix(tc_len[0], tc_len[1], u);
    } else {  // 三次贝塞尔曲线插值
        float u2 = u * u;
        float u3 = u2 * u;
        float oneMinusU = 1.0 - u;
        float oneMinusU2 = oneMinusU * oneMinusU;
        float oneMinusU3 = oneMinusU2 * oneMinusU;

        pos = oneMinusU3 * vec4(tc_pos[0], 0.0, 1.0) +
              3.0 * oneMinusU2 * u * vec4(tc_pos[1], 0.0, 1.0) +
              3.0 * oneMinusU * u2 * vec4(tc_pos[2], 0.0, 1.0) +
              u3 * vec4(tc_pos[3], 0.0, 1.0);

        len = mix(tc_len[0], tc_len[3], u);  // 近似长度
    }

    dashParam = len * dashScale + timeOffset;  // 计算虚线参数
    gl_Position = cameraTrans * pos;           // 应用相机变换
}
)";

// Fragment Shader: 实现虚线样式
const char* fragmentShaderSource = R"(
#version 400 core
in float dashParam;
uniform vec4 color;
uniform int dashType = 10;
out vec4 fragColor;

void main() {
    bool draw = false;
    float pattern;
    float cycle;

    switch(dashType) {
        case 0: pattern = mod(dashParam, 1.0); draw = (pattern < 0.5); break;  // 等长虚线
        case 1: pattern = mod(dashParam, 1.0); draw = (pattern < 0.75); break; // 长虚线
        case 2: pattern = mod(dashParam, 1.0); draw = (pattern < 0.25); break; // 短虚线
        case 3: pattern = mod(dashParam, 1.0); draw = (pattern < 0.1); break;  // 点线
        case 4: cycle = mod(dashParam, 2.0);
         draw = (cycle < 0.1) || (cycle >= 1.0 && cycle < 1.5); break;  // 点划线
        case 5: cycle = mod(dashParam, 2.5);
            draw = (cycle < 0.1) || (cycle >= 1.0 && cycle < 1.1) || (cycle >= 2.0 && cycle < 2.5); break;  // 双点划线
        case 6: cycle = floor(dashParam);
            pattern = mod(dashParam, 1.0);
            draw = (int(cycle) % 2 == 0) ? (pattern < 0.7) : (pattern < 0.3); break;  // 长短交替
        case 7: pattern = mod(dashParam, 0.5);
        draw = (pattern < 0.25); break; // 高频虚线
        case 8: pattern = mod(dashParam, 2.0);
            draw = (pattern < 1.0); break; // 低频虚线
        case 9: cycle = mod(dashParam, 3.0);
            draw = (cycle < 0.3) || (cycle >= 1.0 && cycle < 1.3) || (cycle >= 2.0 && cycle < 2.3); break;  // 三短一长
        default: draw = true; break;  // 实线
    }

    if (!draw)
        discard;  // 不绘制虚线空白部分

    fragColor = color;
}
)";

// 编译并链接着色器程序
GLuint loadShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLuint tessControlShader = glCreateShader(GL_TESS_CONTROL_SHADER);
    glShaderSource(tessControlShader, 1, &tessControlShaderSource, nullptr);
    glCompileShader(tessControlShader);

    GLuint tessEvaluationShader = glCreateShader(GL_TESS_EVALUATION_SHADER);
    glShaderSource(tessEvaluationShader, 1, &tessEvaluationShaderSource, nullptr);
    glCompileShader(tessEvaluationShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, tessControlShader);
    glAttachShader(shaderProgram, tessEvaluationShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(tessControlShader);
    glDeleteShader(tessEvaluationShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// 生成随机点
glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X)
{
    return glm::vec2(
        minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX),
        minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY));
}

// 线段结构体
struct LineSegment
{
    std::vector<glm::vec2> controlPoints; // 控制点（直线2个，曲线4个）
    std::vector<float> lengths;           // 累计长度
    bool isCurve;                         // 是否为曲线
    size_t vertexOffset;                  // VBO 中的顶点偏移
    size_t indexOffset;                   // EBO 中的索引偏移
};

// 生成随机线条（直线和曲线混合）
void generateRandomMixedLines(std::vector<LineSegment>& lines, int numLines, int numSegments)
{
    lines.resize(numLines * numSegments);
    int lineIdx = 0;

    for (int line = 0; line < numLines; ++line)
    {
        glm::vec2 currentPoint = randomPoint();
        float dAccLen = 0.0f;
        for (int i = 0; i < numSegments; ++i)
        {
            LineSegment& segment = lines[lineIdx++];
            segment.isCurve = rand() % 2 == 0;

            if (segment.isCurve)
            {
                segment.controlPoints.push_back(currentPoint);
                segment.lengths.push_back(dAccLen);
                segment.controlPoints.push_back(randomPoint());
                segment.lengths.push_back(dAccLen);
                segment.controlPoints.push_back(randomPoint());
                segment.lengths.push_back(dAccLen);
                glm::vec2 nextPoint = randomPoint();
                float segmentLength = glm::distance(currentPoint, nextPoint);
                dAccLen += segmentLength;
                segment.controlPoints.push_back(nextPoint);
                segment.lengths.push_back(dAccLen);
                currentPoint = nextPoint;
            }
            else
            {
                segment.controlPoints.push_back(currentPoint);
                segment.lengths.push_back(dAccLen);
                glm::vec2 nextPoint = randomPoint();
                float segmentLength = glm::distance(currentPoint, nextPoint);
                dAccLen += segmentLength;
                segment.controlPoints.push_back(nextPoint);
                segment.lengths.push_back(dAccLen);
                currentPoint = nextPoint;
            }
        }
    }
}

// 更新顶点和索引缓冲区
void updateBuffers(GLuint VBO, GLuint EBO, std::vector<LineSegment>& lines,
    std::vector<float>& vertices, std::vector<unsigned int>& indices)
{
    vertices.clear();
    indices.clear();
    size_t vertexOffset = 0;
    size_t indexOffset = 0;

    for (auto& segment : lines)
    {
        segment.vertexOffset = vertexOffset;
        segment.indexOffset = indexOffset;

        for (size_t i = 0; i < segment.controlPoints.size(); ++i)
        {
            vertices.push_back(segment.controlPoints[i].x);
            vertices.push_back(segment.controlPoints[i].y);
            vertices.push_back(segment.lengths[i]);
        }

        if (segment.isCurve)
        {
            indices.push_back(static_cast<unsigned int>(vertexOffset));
            indices.push_back(static_cast<unsigned int>(vertexOffset + 1));
            indices.push_back(static_cast<unsigned int>(vertexOffset + 2));
            indices.push_back(static_cast<unsigned int>(vertexOffset + 3));
            vertexOffset += 4;
            indexOffset += 4;
        }
        else
        {
            indices.push_back(static_cast<unsigned int>(vertexOffset));
            indices.push_back(static_cast<unsigned int>(vertexOffset + 1));
            vertexOffset += 2;
            indexOffset += 2;
        }

        indices.push_back(0xFFFFFFFF); // 图元重启标记
        indexOffset++;
    }

    indices.pop_back();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);
}

// 旋转线段
void rotateLine(std::vector<LineSegment>& lines, int lineIdx, float angle, GLuint VBO)
{
    auto& segment = lines[lineIdx];
    glm::mat2 rotation = glm::mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    std::vector<float> updatedVertices;

    for (size_t i = 0; i < segment.controlPoints.size(); ++i)
    {
        segment.controlPoints[i] = rotation * segment.controlPoints[i];
        updatedVertices.push_back(segment.controlPoints[i].x);
        updatedVertices.push_back(segment.controlPoints[i].y);
        updatedVertices.push_back(segment.lengths[i]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, segment.vertexOffset * 3 * sizeof(float),
        updatedVertices.size() * sizeof(float), updatedVertices.data());
}

// 缩放线段
void scaleLine(std::vector<LineSegment>& lines, int lineIdx, float scale, GLuint VBO)
{
    auto& segment = lines[lineIdx];
    std::vector<float> updatedVertices;

    for (size_t i = 0; i < segment.controlPoints.size(); ++i)
    {
        segment.controlPoints[i] *= scale;
        updatedVertices.push_back(segment.controlPoints[i].x);
        updatedVertices.push_back(segment.controlPoints[i].y);
        updatedVertices.push_back(segment.lengths[i]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, segment.vertexOffset * 3 * sizeof(float),
        updatedVertices.size() * sizeof(float), updatedVertices.data());
}

// 删除线段
void deleteLine(std::vector<LineSegment>& lines, int lineIdx, GLuint VBO, GLuint EBO)
{
    auto& deletedSegment = lines[lineIdx];
    size_t vertexCount = deletedSegment.isCurve ? 4 : 2;
    size_t indexCount = vertexCount + 1;

    lines.erase(lines.begin() + lineIdx);

    std::vector<float> updatedVertices;
    std::vector<unsigned int> updatedIndices;
    size_t vertexOffset = deletedSegment.vertexOffset;
    size_t indexOffset = deletedSegment.indexOffset;

    for (size_t i = lineIdx; i < lines.size(); ++i)
    {
        lines[i].vertexOffset -= vertexCount;
        lines[i].indexOffset -= indexCount;

        for (size_t j = 0; j < lines[i].controlPoints.size(); ++j)
        {
            updatedVertices.push_back(lines[i].controlPoints[j].x);
            updatedVertices.push_back(lines[i].controlPoints[j].y);
            updatedVertices.push_back(lines[i].lengths[j]);
        }

        if (lines[i].isCurve)
        {
            updatedIndices.push_back(static_cast<unsigned int>(lines[i].vertexOffset));
            updatedIndices.push_back(static_cast<unsigned int>(lines[i].vertexOffset + 1));
            updatedIndices.push_back(static_cast<unsigned int>(lines[i].vertexOffset + 2));
            updatedIndices.push_back(static_cast<unsigned int>(lines[i].vertexOffset + 3));
        }
        else
        {
            updatedIndices.push_back(static_cast<unsigned int>(lines[i].vertexOffset));
            updatedIndices.push_back(static_cast<unsigned int>(lines[i].vertexOffset + 1));
        }

        updatedIndices.push_back(0xFFFFFFFF);
    }
    updatedIndices.pop_back();

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, vertexOffset * 3 * sizeof(float),
        updatedVertices.size() * sizeof(float), updatedVertices.data());

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indexOffset * sizeof(unsigned int),
        updatedIndices.size() * sizeof(unsigned int), updatedIndices.data());
}

// 全局变量
float zoomFactor = 1.0f;         // 缩放因子
glm::vec2 cameraPos(0.0f, 0.0f); // 相机位置
bool middleMousePressed = false; // 中键按下状态
glm::vec2 lastMousePos;          // 上一次鼠标位置
float aspectRatio = 1.0f;        // 窗口宽高比

// 滚轮回调：调整缩放
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    zoomFactor += float(yoffset) * 0.1f;
    zoomFactor = std::max(zoomFactor, 0.1f);
}

// 鼠标按键回调：检测中键
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            middleMousePressed = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastMousePos = glm::vec2(xpos, ypos);
        }
        else if (action == GLFW_RELEASE)
        {
            middleMousePressed = false;
        }
    }
}

// 鼠标移动回调：拖动画面
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (middleMousePressed)
    {
        glm::vec2 currentMousePos(xpos, ypos);
        glm::vec2 delta = currentMousePos - lastMousePos;

        int width, height;
        glfwGetWindowSize(window, &width, &height);
        float moveX = -delta.x * (2.0f * X * zoomFactor * aspectRatio) / width;
        float moveY = delta.y * (2.0f * X * zoomFactor) / height;

        cameraPos += glm::vec2(moveX, moveY);
        lastMousePos = currentMousePos;
    }
}

// 窗口大小变化回调：更新视口和宽高比
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);                                      // 更新视口
    aspectRatio = static_cast<float>(width) / static_cast<float>(height); // 更新宽高比
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    srand(static_cast<unsigned int>(time(NULL)));

    // 设置 OpenGL 4.0 核心配置文件
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(1400, 1400, "CAD Lines with Tessellation", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 设置回调函数
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // 初始化视口和宽高比
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    aspectRatio = static_cast<float>(width) / static_cast<float>(height);

    // 创建着色器程序
    GLuint shaderProgram = loadShader();
    glUseProgram(shaderProgram);

    // 生成线条数据
    std::vector<LineSegment> allLines;
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    {
        const int NUM_LINES = 10;
        const int NUM_SEGMENTS = 3;
        generateRandomMixedLines(allLines, NUM_LINES, NUM_SEGMENTS);
        updateBuffers(0, 0, allLines, vertices, indices);
    }

    // 设置 VAO、VBO、EBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);

    // 配置顶点属性
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // 启用图元重启
    glEnable(GL_PRIMITIVE_RESTART);
    glPrimitiveRestartIndex(0xFFFFFFFF);
    glPatchParameteri(GL_PATCH_VERTICES, 4);

    // 设置背景色
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // 示例修改线段
    rotateLine(allLines, 5, glm::radians(45.0f), VBO);
    scaleLine(allLines, 10, 1.5f, VBO);
    deleteLine(allLines, 15, VBO, EBO);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 更新相机变换矩阵，考虑宽高比
        glm::mat4 cameraTrans = glm::ortho(
            -X * zoomFactor * aspectRatio + cameraPos.x, // 左边界
            X * zoomFactor * aspectRatio + cameraPos.x,  // 右边界
            -X * zoomFactor + cameraPos.y,               // 下边界
            X * zoomFactor + cameraPos.y                 // 上边界
        );
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

        // 动态调整细分级别和虚线缩放
        float tessLevel = 35.0f / zoomFactor;
        glUniform1f(glGetUniformLocation(shaderProgram, "tessLevel"), tessLevel);

        glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        float time = duration.count();
        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        float dashScale = 12.0f / zoomFactor;
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);

        // 渲染
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(VAO);
        glDrawElements(GL_PATCHES, indices.size(), GL_UNSIGNED_INT, 0);

        // 检查 OpenGL 错误
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteProgram(shaderProgram);
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}