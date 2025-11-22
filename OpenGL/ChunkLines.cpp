
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

#include <memory>
#include <random>

// 线类型枚举
enum class LineType
{
    STRAIGHT,
    POLYLINE,
    BEZIER
};

// 线基类
class Line
{
public:
    LineType m_lineType;
    std::vector<glm::vec2> m_vecPts;

    glm::vec3 m_color;
    bool m_bDashed;
    float m_dDashSize;
    float m_dGapSize;
    size_t m_nVboOffset; // 在Chunk的VBO中的偏移（以顶点为单位）
    size_t m_nPtsCount; // 顶点数量
    size_t m_nChunkIndex; // 所属Chunk索引

    Line(LineType t, glm::vec3 c = glm::vec3(1.0f), bool dashed = false,
        float dash = 5.0f, float gap = 5.0f)
        :m_lineType(t), m_color(c), m_bDashed(dashed), m_dDashSize(dash), m_dGapSize(gap),
        m_nVboOffset(0), m_nPtsCount(0), m_nChunkIndex(0)
    {
    }

    virtual ~Line() = default;
    
    virtual void addVertex(const glm::vec2& vertex)
    {
        m_vecPts.push_back(vertex);
        m_nPtsCount = getRenderVertices().size();
    }

    virtual std::vector<glm::vec2> getRenderVertices() const
    {
        return m_vecPts;
    }
};

// 直线
class StraightLine : public Line
{
public:
    StraightLine() : Line(LineType::STRAIGHT)
    {
    }
};

// 折线
class Polyline : public Line
{
public:
    Polyline() : Line(LineType::POLYLINE)
    {
    }
};

// 贝塞尔曲线
class BezierCurve : public Line
{
public:
    BezierCurve() : Line(LineType::BEZIER)
    {
    }

    std::vector<glm::vec2> getRenderVertices() const override
    {
        std::vector<glm::vec2> points;
        if (m_vecPts.size() < 3) // 检查是否至少有3个顶点
            return points;

        const int segments = 20;
        for (int i = 0; i <= segments; ++i)
        {
            float t = static_cast<float>(i) / segments;
            glm::vec2 p = (1 - t) * (1 - t) * m_vecPts[0] + 2 * (1 - t) * t * m_vecPts[1] + t * t * m_vecPts[2];
            points.push_back(p);
        }
        return points;
    }
};

// Chunk类，管理一组线的VBO和VAO
struct Chunk
{
    GLuint m_vao, m_vbo;
    std::vector<std::unique_ptr<Line>> m_vecLines;
    size_t m_nTotalPts; // Chunk中的总顶点数

    static const size_t MAX_LINES = 1000; // 每Chunk最大线数
    static const size_t MAX_VERTICES = 100000; // 每Chunk最大顶点数

    Chunk() : m_nTotalPts(0)
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vbo);

        glBindVertexArray(m_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);

        glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(glm::vec2),
            nullptr, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
        glEnableVertexAttribArray(0);
    }

    ~Chunk()
    {
        glDeleteVertexArrays(1, &m_vao);
        glDeleteBuffers(1, &m_vbo);
    }

    bool isFull() const
    {
        return m_vecLines.size() >= MAX_LINES;
    }

    void updateVBO(size_t lineIndex)
    {
        if (lineIndex >= m_vecLines.size())
            return;

        auto& line = m_vecLines[lineIndex];
        auto m_vecPts = line->getRenderVertices();
        line->m_nPtsCount = m_vecPts.size();

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferSubData(GL_ARRAY_BUFFER, line->m_nVboOffset * sizeof(glm::vec2),
            m_vecPts.size() * sizeof(glm::vec2), m_vecPts.data());
    }

    void recalculateOffsets()
    {
        size_t offset = 0;

        for (auto& line : m_vecLines)
        {
            line->m_nVboOffset = offset;
            line->m_nPtsCount = line->getRenderVertices().size();
            offset += line->m_nPtsCount;
        }

        m_nTotalPts = offset;
    }

    void updateFullVBO()
    {
        std::vector<glm::vec2> vertexData;
        for (const auto& line : m_vecLines)
        {
            auto m_vecPts = line->getRenderVertices();
            vertexData.insert(vertexData.end(), m_vecPts.begin(), m_vecPts.end());
        }

        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(glm::vec2),
            vertexData.data(), GL_DYNAMIC_DRAW);

        m_nTotalPts = vertexData.size();
    }
};

// CAD管理类
class CAD
{
private:
    std::vector<std::unique_ptr<Chunk>> m_vecChunks;
    GLuint m_shaderProgram;

    // 初始化着色器（保持不变）
    GLuint initShaders()
    {
        const char* vs = R"(
            #version 330 core
            layout (location = 0) in vec2 aPos;
            uniform mat4 projection;
            out vec2 fragCoord;
            void main() {
                gl_Position = projection * vec4(aPos, 0.0, 1.0);
                fragCoord = aPos;
            }
        )";
        const char* fs = R"(
            #version 330 core
            out vec4 FragColor;
            uniform vec3 lineColor;
            uniform bool isDashed;
            uniform float dashSize;
            uniform float gapSize;
            uniform vec2 resolution;
            in vec2 fragCoord;
            void main() {
                if (isDashed) {
                    float totalSize = dashSize + gapSize;
                    float pattern = mod(fragCoord.x + fragCoord.y, totalSize);
                    if (pattern > dashSize) {
                        discard;
                    }
                }
                FragColor = vec4(lineColor, 1.0);
            }
        )";

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vs, nullptr);
        glCompileShader(vertexShader);

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fs, nullptr);
        glCompileShader(fragmentShader);

        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return program;
    }

public:
    CAD()
    {
        m_shaderProgram = initShaders();
        m_vecChunks.push_back(std::make_unique<Chunk>());
    }

    ~CAD()
    {
        glDeleteProgram(m_shaderProgram);
    }

    // 新增：获取chunks的const引用
    const std::vector<std::unique_ptr<Chunk>>& getChunks() const
    {
        return m_vecChunks;
    }

    void addLine(std::unique_ptr<Line> line)
    {
        // Get the last chunk
        if (m_vecChunks.empty() || m_vecChunks.back()->isFull())
        {
            m_vecChunks.push_back(std::make_unique<Chunk>());
        }

        // Work with the last chunk
        auto& targetChunk = m_vecChunks.back();
        line->m_nChunkIndex = m_vecChunks.size() - 1;
        targetChunk->m_vecLines.push_back(std::move(line));
        targetChunk->recalculateOffsets();
        targetChunk->updateVBO(targetChunk->m_vecLines.size() - 1);
    }

    // void addLine(std::unique_ptr<Line> line) {
    //     auto& targetChunk = m_vecChunks.back();
    //     if (targetChunk->isFull()) {
    //         m_vecChunks.push_back(std::make_unique<Chunk>());
    //         targetChunk = m_vecChunks.back();
    //     }
    //     line->m_nChunkIndex = m_vecChunks.size() - 1;
    //     targetChunk->m_vecLines.push_back(std::move(line));
    //     targetChunk->recalculateOffsets();
    //     targetChunk->updateVBO(targetChunk->m_vecLines.size() - 1);
    // }

    void removeLine(size_t m_nChunkIndex, size_t lineIndex)
    {
        if (m_nChunkIndex >= m_vecChunks.size() ||
            lineIndex >= m_vecChunks[m_nChunkIndex]->m_vecLines.size())
            return;

        m_vecChunks[m_nChunkIndex]->m_vecLines.erase(
            m_vecChunks[m_nChunkIndex]->m_vecLines.begin() + lineIndex);

        m_vecChunks[m_nChunkIndex]->recalculateOffsets();
        m_vecChunks[m_nChunkIndex]->updateFullVBO();
        if (m_vecChunks[m_nChunkIndex]->m_vecLines.empty() && m_vecChunks.size() > 1)

        {
            m_vecChunks.erase(m_vecChunks.begin() + m_nChunkIndex);
        }
    }

    void moveLine(size_t m_nChunkIndex, size_t lineIndex, const glm::vec2& delta)
    {
        if (m_nChunkIndex >= m_vecChunks.size() ||
            lineIndex >= m_vecChunks[m_nChunkIndex]->m_vecLines.size())
            return;

        auto& m_vecPts = m_vecChunks[m_nChunkIndex]->m_vecLines[lineIndex]->m_vecPts;
        for (auto& v : m_vecPts)
        {
            v += delta;
        }
        m_vecChunks[m_nChunkIndex]->updateVBO(lineIndex);
    }

    void render(const glm::mat4& projection)
    {
        glUseProgram(m_shaderProgram);
        glUniformMatrix4fv(glGetUniformLocation(m_shaderProgram, "projection"),
            1, GL_FALSE, &projection[0][0]);

        glUniform2f(glGetUniformLocation(m_shaderProgram, "resolution"),
            800.0f, 600.0f);

        for (const auto& chunk : m_vecChunks)
        {
            glBindVertexArray(chunk->m_vao);

            for (const auto& line : chunk->m_vecLines)
            {
                if (line->m_nPtsCount == 0)
                    continue;

                glUniform3fv(glGetUniformLocation(m_shaderProgram, "lineColor"),
                    1, &line->m_color[0]);

                glUniform1i(glGetUniformLocation(m_shaderProgram, "isDashed"),
                    line->m_bDashed);

                glUniform1f(glGetUniformLocation(m_shaderProgram, "dashSize"),
                    line->m_dDashSize);

                glUniform1f(glGetUniformLocation(m_shaderProgram, "gapSize"),
                    line->m_dGapSize);

                GLenum mode = line->m_lineType == LineType::STRAIGHT ? GL_LINES : GL_LINE_STRIP;

                glDrawArrays(mode, line->m_nVboOffset, line->m_nPtsCount);
            }
        }
    }
};

// 主程序
int main()
{
    // 初始化GLFW和GLAD
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "CAD Chunked VBO", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
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

    glViewport(0, 0, 1200, 1000);
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f, -1.0f, 1.0f);

    // 初始化CAD
    CAD cad;

    //// 添加示例线
    //auto straight = std::make_unique<StraightLine>();
    //straight->addVertex({ 100, 100 });
    //straight->addVertex({ 200, 200 });
    //straight->m_color = { 1.0f, 0.0f, 0.0f };
    //straight->m_bDashed = true;
    //cad.addLine(std::move(straight));

    //auto polyline = std::make_unique<Polyline>();
    //polyline->addVertex({ 300, 100 });
    //polyline->addVertex({ 350, 150 });
    //polyline->addVertex({ 400, 100 });
    //polyline->m_color = { 0.0f, 1.0f, 0.0f };
    //cad.addLine(std::move(polyline));

    //auto bezier = std::make_unique<BezierCurve>();
    //bezier->addVertex({ 500, 100 });
    //bezier->addVertex({ 550, 200 });
    //bezier->addVertex({ 600, 100 });
    //bezier->m_color = { 0.0f, 0.0f, 1.0f };
    //bezier->m_bDashed = true;
    //cad.addLine(std::move(bezier));


    // 随机数生成器
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> pos_dist_x(0.0f, 400.0f); // x坐标范围
    std::uniform_real_distribution<float> pos_dist_y(0.0f, 400.0f); // y坐标范围
    std::uniform_real_distribution<float> color_dist(0.0f, 1.0f);   // 颜色分量
    std::uniform_int_distribution<int> type_dist(0, 2);             // 线类型 (0: Straight, 1: Polyline, 2: Bezier)
    std::uniform_int_distribution<int> vertex_dist(2, 6);           // 折线顶点数 (2-6)
    std::uniform_int_distribution<int> dash_dist(0, 1);             // 是否虚线
    std::uniform_real_distribution<float> dash_size_dist(3.0f, 10.0f); // 虚线大小
    std::uniform_real_distribution<float> gap_size_dist(3.0f, 10.0f);  // 虚线间隙

    // 生成 N 条随机线条
    const int NumLines = 10;
    for (int i = 0; i < NumLines; ++i)
    {
        // 随机选择线类型
        int line_type = type_dist(gen);
        std::unique_ptr<Line> line;

        switch (line_type)
        {
        case 0: // StraightLine
            line = std::make_unique<StraightLine>();
            break;
        case 1: // Polyline
            line = std::make_unique<Polyline>();
            break;
        case 2: // BezierCurve
            line = std::make_unique<BezierCurve>();
            break;
        }

        // 随机顶点数（直线固定2个，折线2-6个，贝塞尔固定3个）
        int num_vertices = (line_type == 0) ? 2 : (line_type == 2) ? 3 : vertex_dist(gen);

        // 添加随机顶点
        for (int j = 0; j < num_vertices; ++j)
        {
            line->addVertex({ pos_dist_x(gen), pos_dist_y(gen) });
        }

        // 随机颜色
        line->m_color = { color_dist(gen), color_dist(gen), color_dist(gen) };

        // 随机虚线属性
        line->m_bDashed = dash_dist(gen) == 1;
        if (line->m_bDashed)
        {
            line->m_dDashSize = dash_size_dist(gen);
            line->m_dGapSize = gap_size_dist(gen);
        }

        // 添加到 CAD
        cad.addLine(std::move(line));
    }

    // 按键状态记录
    bool key1Pressed = false; // 记录按键 1 的上一帧状态
    bool key2Pressed = false; // 记录按键 2 的上一帧状态
    bool key3Pressed = false; // 记录按键 3 的上一帧状态
    bool key4Pressed = false; // 记录按键 4 的上一帧状态

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        cad.render(projection);

        glfwSwapBuffers(window);
        glfwPollEvents();

        // 按键 1：添加新直线
        bool key1Current = glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS;
        if (key1Current && !key1Pressed) // 检测按键 1 从未按下到按下的边缘
        {
            auto newLine = std::make_unique<StraightLine>();
            newLine->addVertex({ pos_dist_x(gen), pos_dist_y(gen) });
            newLine->addVertex({ pos_dist_x(gen), pos_dist_y(gen) });

            newLine->m_color = { 1.0f, 0.5f, 0.0f };
            cad.addLine(std::move(newLine));
        }
        key1Pressed = key1Current; // 更新按键 1 状态

        // 按键 2：删除最后一条线
        bool key2Current = glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS;
        if (key2Current && !key2Pressed && !cad.getChunks().empty()) // 检测按键 2 的边缘
        {
            auto& chunk = cad.getChunks()[0];
            if (!chunk->m_vecLines.empty())
            {
                cad.removeLine(0, chunk->m_vecLines.size() - 1);
            }
        }
        key2Pressed = key2Current; // 更新按键 2 状态

        // 按键 3：移动第一条线
        bool key3Current = glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS;
        if (key3Current && !key3Pressed && !cad.getChunks().empty()) // 检测按键 3 的边缘
        {
            if (!cad.getChunks()[0]->m_vecLines.empty())
            {
                cad.moveLine(0, 0, { 1.5f, 0.0f });
            }
        }
        key3Pressed = key3Current; // 更新按键 3 状态

        // 按键 4：切换第一条线的虚线状态
        bool key4Current = glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS;
        if (key4Current && !key4Pressed && !cad.getChunks().empty()) // 检测按键 4 的边缘
        {
            if (!cad.getChunks()[0]->m_vecLines.empty())
            {
                cad.getChunks()[0]->m_vecLines[0]->m_bDashed = !cad.getChunks()[0]->m_vecLines[0]->m_bDashed;
                cad.getChunks()[0]->updateVBO(0);
            }
        }
        key4Pressed = key4Current; // 更新按键 4 状态
    }

    glfwTerminate();
    return 0;
}