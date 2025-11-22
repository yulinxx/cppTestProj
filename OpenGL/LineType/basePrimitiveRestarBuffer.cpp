// 图元重启的优化代码

// 以下是针对 CAD 场景优化的代码,增加了动态缓冲支持:

// cpp

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <chrono>
#include <string>

// 引入图像写入库
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

// 定义常量 X,用于相机投影范围
constexpr float X = 4.0f;

// 顶点着色器源代码
const char* vs = R"(
#version 330 core
// 输入顶点位置
layout(location = 0) in vec2 in_pos;
// 输入线段长度
layout(location = 1) in float in_len;

// 相机变换矩阵
uniform mat4 cameraTrans;
// 虚线缩放因子
uniform float dashScale;
// 时间偏移量,用于动画效果
uniform float timeOffset = 0.0;

// 输出虚线参数
out float dashParam;

void main() {
    // 计算顶点在裁剪空间的位置
    gl_Position = cameraTrans * vec4(in_pos, 0.0, 1.0);
    // 计算虚线长度
    float dashLength = in_len * dashScale + timeOffset;
    // 将虚线长度传递给片段着色器
    dashParam = dashLength;
}
)";

// 片段着色器源代码
const char* fs = R"(
#version 330 core
// 从顶点着色器接收的虚线参数
in float dashParam;
// 颜色
uniform vec4 color;
// 虚线类型
uniform int dashType = 0;
// 输出片段颜色
out vec4 fragColor;

void main() {
    // 是否绘制片段
    bool draw = false;
    // 虚线模式
    float pattern;
    // 虚线周期
    float cycle;

    // 根据虚线类型选择绘制模式
    switch(dashType) {
        case 0: // 默认等长虚线
            // 计算虚线模式
            pattern = mod(dashParam, 1.0);
            // 判断是否绘制片段
            draw = (pattern < 0.5);
            break;
        default:
            // 其他情况绘制片段
            draw = true;
            break;
    }

    // 如果不绘制,则丢弃片段
    if (!draw) discard;
    // 设置片段颜色
    fragColor = color;
}
)";

/**
 * @brief 加载并编译顶点着色器和片段着色器,链接成着色器程序
 * @param vs 顶点着色器源代码
 * @param fs 片段着色器源代码
 * @return 着色器程序的 ID
 */
GLuint loadShader(const char* vs, const char* fs)
{
    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    // 设置顶点着色器源代码
    glShaderSource(vertexShader, 1, &vs, nullptr);
    // 编译顶点着色器
    glCompileShader(vertexShader);

    // 检查编译状态
    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        // 获取编译错误信息
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cerr << "Vertex Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 创建片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    // 设置片段着色器源代码
    glShaderSource(fragmentShader, 1, &fs, nullptr);
    // 编译片段着色器
    glCompileShader(fragmentShader);

    // 检查编译状态
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        // 获取编译错误信息
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cerr << "Fragment Shader Compilation Failed:\n" << infoLog << std::endl;
    }

    // 创建着色器程序
    GLuint shaderProgram = glCreateProgram();
    // 附加顶点着色器到程序
    glAttachShader(shaderProgram, vertexShader);
    // 附加片段着色器到程序
    glAttachShader(shaderProgram, fragmentShader);
    // 链接着色器程序
    glLinkProgram(shaderProgram);

    // 检查链接状态
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        // 获取链接错误信息
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader Program Linking Failed:\n" << infoLog << std::endl;
    }

    // 删除已编译的着色器
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

/**
 * @brief 生成一个随机点
 * @param minX 最小 X 坐标
 * @param maxX 最大 X 坐标
 * @param minY 最小 Y 坐标
 * @param maxY 最大 Y 坐标
 * @return 随机点的坐标
 */
glm::vec2 randomPoint(float minX = -X, float maxX = X, float minY = -X, float maxY = X)
{
    return glm::vec2(
        // 生成随机 X 坐标
        minX + (rand() / static_cast<float>(RAND_MAX)) * (maxX - minX),
        // 生成随机 Y 坐标
        minY + (rand() / static_cast<float>(RAND_MAX)) * (maxY - minY)
    );
}

/**
 * @brief 生成随机混合线条(直线和贝塞尔曲线)
 * @param vertices 存储顶点数据的向量
 * @param flatIndices 存储索引数据的向量
 * @param numLines 线条数量
 * @param numSegments 每条线的段数
 * @param bezierSegments 贝塞尔曲线的细分段数
 * @param minX 最小 X 坐标
 * @param maxX 最大 X 坐标
 * @param minY 最小 Y 坐标
 * @param maxY 最大 Y 坐标
 */
void generateRandomMixedLines(
    std::vector<float>& vertices,
    std::vector<unsigned int>& flatIndices,
    int numLines,
    int numSegments,
    int bezierSegments,
    float minX, float maxX, float minY, float maxY
)
{
    // 清空顶点和索引数据
    vertices.clear();
    flatIndices.clear();

    // 生成每条线
    for (int line = 0; line < numLines; ++line)
    {
        // 随机生成起始点
        glm::vec2 startPoint = randomPoint(minX, maxX, minY, maxY);
        glm::vec2 currentPoint = startPoint;
        glm::vec2 prevPoint = startPoint;
        // 累积长度
        float dAccLen = 0.0;

        // 获取顶点索引
        unsigned int vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
        // 添加起始点坐标
        vertices.push_back(startPoint.x);
        vertices.push_back(startPoint.y);
        // 添加累积长度
        vertices.push_back(dAccLen);
        // 添加索引
        flatIndices.push_back(vertexIndex);

        // 生成每条线的段
        for (int i = 0; i < numSegments; ++i)
        {
            // 随机选择直线或贝塞尔曲线
            bool bLine = rand() % 2 == 0;

            if (bLine)
            {
                // 随机生成下一个点
                glm::vec2 point = randomPoint(minX, maxX, minY, maxY);
                // 获取顶点索引
                vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                // 添加点坐标
                vertices.push_back(point.x);
                vertices.push_back(point.y);

                // 计算线段长度
                float segmentLength = glm::distance(prevPoint, point);
                // 累积长度
                dAccLen += segmentLength;
                // 添加累积长度
                vertices.push_back(dAccLen);

                // 添加索引
                flatIndices.push_back(vertexIndex);

                // 更新上一个点和当前点
                prevPoint = point;
                currentPoint = point;
            }
            else
            {
                // 随机生成贝塞尔曲线的控制点和终点
                glm::vec2 controlPoint1 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 controlPoint2 = randomPoint(minX, maxX, minY, maxY);
                glm::vec2 nextPoint = randomPoint(minX, maxX, minY, maxY);

                // 细分贝塞尔曲线
                for (int j = 1; j <= bezierSegments; ++j)
                {
                    // 计算参数 t
                    float t = float(j) / float(bezierSegments);
                    float u = 1.0f - t;

                    // 计算贝塞尔曲线上的点
                    glm::vec2 point = u * u * u * currentPoint +
                        3.0f * u * u * t * controlPoint1 +
                        3.0f * u * t * t * controlPoint2 +
                        t * t * t * nextPoint;

                    // 获取顶点索引
                    vertexIndex = static_cast<unsigned int>(vertices.size() / 3);
                    // 添加点坐标
                    vertices.push_back(point.x);
                    vertices.push_back(point.y);

                    // 计算线段长度
                    float segmentLength = glm::distance(prevPoint, point);
                    // 累积长度
                    dAccLen += segmentLength;
                    // 添加累积长度
                    vertices.push_back(dAccLen);

                    // 添加索引
                    flatIndices.push_back(vertexIndex);

                    // 更新上一个点
                    prevPoint = point;
                }
                // 更新当前点
                currentPoint = nextPoint;
            }
        }
        // 如果不是最后一条线,添加图元重启标记
        if (line < numLines - 1)
        {
            flatIndices.push_back(0xFFFFFFFF); // 图元重启标记
        }
    }
}

// 缩放因子
float zoomFactor = 1.0f;

/**
 * @brief 鼠标滚轮回调函数,用于缩放
 * @param window GLFW 窗口指针
 * @param xoffset 鼠标滚轮 X 偏移量
 * @param yoffset 鼠标滚轮 Y 偏移量
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    // 更新缩放因子
    zoomFactor += float(yoffset) * 0.1f;
    // 限制缩放因子范围
    zoomFactor = std::max(zoomFactor, 0.1f);
}

/**
 * @brief 主函数,程序入口
 * @return 程序退出状态码
 */
int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置随机数种子
    srand(static_cast<unsigned int>(time(NULL)));

    // 设置 OpenGL 上下文版本和配置
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

    // 创建 GLFW 窗口
    GLFWwindow* window = glfwCreateWindow(1400, 1400, "CAD Lines", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        // 终止 GLFW
        glfwTerminate();
        return -1;
    }
    // 设置当前上下文
    glfwMakeContextCurrent(window);

    // 初始化 GLAD
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
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

    // 设置鼠标滚轮回调函数
    glfwSetScrollCallback(window, scroll_callback);

    // 加载着色器程序
    GLuint shaderProgram = loadShader(vs, fs);
    // 使用着色器程序
    glUseProgram(shaderProgram);

    // 创建正交投影矩阵
    glm::mat4 cameraTrans = glm::ortho(-X, X, -X, X);
    // 设置相机变换矩阵
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

    // 设置颜色
    glUniform4f(glGetUniformLocation(shaderProgram, "color"), 0.0f, 0.0f, 1.0f, 1.0f);
    // 设置虚线缩放因子
    glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), 8.0f);

    // 存储顶点数据的向量
    std::vector<float> shapeVertices;
    // 存储索引数据的向量
    std::vector<unsigned int> flatIndices;

    {
        // 模拟 CAD 中的数万条线
        const int NUM_LINES = 10000;
        // 每条线的段数
        const int NUM_SEGMENTS = 3;
        // 减少细分以模拟更多线条
        const int BEZIER_RES = 10;
        // 生成随机混合线条
        generateRandomMixedLines(shapeVertices, flatIndices, NUM_LINES, NUM_SEGMENTS, BEZIER_RES, -X, X, -X, X);
    }

    // 顶点数组对象
    GLuint VAO, VBO, EBO;
    // 生成顶点数组对象
    glGenVertexArrays(1, &VAO);
    // 生成顶点缓冲对象
    glGenBuffers(1, &VBO);
    // 生成索引缓冲对象
    glGenBuffers(1, &EBO);

    // 绑定顶点数组对象
    glBindVertexArray(VAO);

    // 绑定顶点缓冲对象
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 使用动态缓冲分配顶点数据
    glBufferData(GL_ARRAY_BUFFER, shapeVertices.size() * sizeof(float), shapeVertices.data(), GL_DYNAMIC_DRAW);

    // 绑定索引缓冲对象
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // 使用动态缓冲分配索引数据
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, flatIndices.size() * sizeof(unsigned int), flatIndices.data(), GL_DYNAMIC_DRAW);

    // 设置顶点属性指针
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 启用顶点属性
    glEnableVertexAttribArray(0);
    // 设置顶点属性指针
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(2 * sizeof(float)));
    // 启用顶点属性
    glEnableVertexAttribArray(1);

    // 启用图元重启
    glEnable(GL_PRIMITIVE_RESTART);
    // 设置图元重启索引
    glPrimitiveRestartIndex(0xFFFFFFFF);

    // 设置清屏颜色
    glClearColor(1.0, 1.0, 1.0, 1.0);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        // 更新相机变换矩阵
        glm::mat4 cameraTrans = glm::ortho(-X * zoomFactor, X * zoomFactor, -X * zoomFactor, X * zoomFactor);
        // 设置相机变换矩阵
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "cameraTrans"), 1, GL_FALSE, &cameraTrans[0][0]);

        // 计算虚线缩放因子
        float dashScale = 12.0f / zoomFactor;
        // 设置虚线缩放因子
        glUniform1f(glGetUniformLocation(shaderProgram, "dashScale"), dashScale);

        // 获取当前时间
        auto now = std::chrono::high_resolution_clock::now();
        // 计算时间差
        auto duration = std::chrono::duration<float>(now.time_since_epoch());
        // 获取时间值
        float time = duration.count();
        // 设置时间偏移量
        glUniform1f(glGetUniformLocation(shaderProgram, "timeOffset"), time * 0.8f);

        // 清屏
        glClear(GL_COLOR_BUFFER_BIT);

        // 绑定顶点数组对象
        glBindVertexArray(VAO);
        // 绘制元素
        glDrawElements(GL_LINE_STRIP, flatIndices.size(), GL_UNSIGNED_INT, 0);

        // 检查 OpenGL 错误
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            std::cerr << "OpenGL Error: " << err << std::endl;
        }

        // 计数器
        static int i = 0;
        i++;
        // 保存帧为 PNG 图像(未启用)
        if (i % 4000 == 0 && false)
        {
            int width, height;
            // 获取窗口大小
            glfwGetWindowSize(window, &width, &height);
            // 存储像素数据的向量
            std::vector<unsigned char> pixels(width * height * 4);
            // 读取像素数据
            glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
            // 垂直翻转图像
            stbi_flip_vertically_on_write(1);
            // 生成文件名
            std::string filename = "frame_" + std::to_string(i) + ".png";
            // 保存图像为 PNG
            stbi_write_png(filename.c_str(), width, height, 4, pixels.data(), width * 4);
        }

        // 交换前后缓冲区
        glfwSwapBuffers(window);
        // 处理事件
        glfwPollEvents();
    }

    // 禁用图元重启
    glDisable(GL_PRIMITIVE_RESTART);
    // 删除着色器程序
    glDeleteProgram(shaderProgram);
    // 删除顶点数组对象
    glDeleteVertexArrays(1, &VAO);
    // 删除顶点缓冲对象
    glDeleteBuffers(1, &VBO);
    // 删除索引缓冲对象
    glDeleteBuffers(1, &EBO);
    // 销毁窗口
    glfwDestroyWindow(window);
    // 终止 GLFW
    glfwTerminate();

    return 0;
}

// 使用动态缓冲 有哪些好处

// 在 OpenGL 中使用动态缓冲区(通常通过 GL_DYNAMIC_DRAW 或 GL_STREAM_DRAW 指定)相比静态缓冲区(GL_STATIC_DRAW)有特定的优势,尤其在需要频繁更新数据的场景下,比如你的 CAD 软件中涉及数万条线的实时编辑和渲染.以下是使用动态缓冲区的主要好处,以及在你的具体场景中的意义:

// 使用动态缓冲区的好处
// 1. 支持高效的数据更新
// 好处:动态缓冲区允许你频繁修改缓冲区内容(如顶点数据或索引数据),而无需重新分配整个缓冲区.使用 glBufferSubData 或 glMapBuffer 可以直接更新缓冲区的特定部分.
// CAD 场景意义:
// 用户在 CAD 软件中编辑线条(添加、删除、移动顶点)时,只需更新受影响的顶点或索引,而不是重新上传所有数据.
// 例如,修改一条线的某个顶点,只需更新 shapeVertices 中对应的 3 个 float(x, y, dAccLen),而不是重新生成整个缓冲区.
// 2. 减少内存分配开销
// 好处:动态缓冲区在创建时分配一次内存,后续更新不会触发新的内存分配或释放操作.相比之下,频繁使用 glBufferData(静态缓冲区常用)会导致内存重新分配和数据拷贝,开销较大.
// CAD 场景意义:
// 在数万条线的场景中(例如 100,000 个顶点 ≈ 1.2MB 数据),每次编辑都重新分配缓冲区会显著降低性能.
// 动态缓冲区避免了这种开销,保持内存使用的稳定性.
// 3. 优化 GPU 性能
// 好处:动态缓冲区提示驱动程序数据会频繁更改,GPU 可以优化内存管理和缓存策略(例如避免长期缓存数据).
// CAD 场景意义:
// CAD 软件的实时交互(缩放、平移、编辑)需要快速响应.动态缓冲区确保 GPU 能够高效处理更新后的数据,避免因缓存过期或重新加载导致的延迟.
// 4. 灵活性更高
// 好处:动态缓冲区支持部分更新,可以只修改缓冲区中的一小段数据,而静态缓冲区通常假设数据不变,更新时需要替换全部内容.
// CAD 场景意义:
// 例如,你可能只更新某条线的索引(flatIndices 中的一段),而其他线的索引保持不变.使用 glBufferSubData 可以精确更新这部分数据,减少传输量.
// 5. 适合流式数据
// 好处:如果使用 GL_STREAM_DRAW(动态缓冲的一种变体),它特别适合数据只使用一次或几次后就更新的场景(如流式生成的数据).
// CAD 场景意义:
// 如果 CAD 软件支持实时生成线条(例如用户拖动画线),GL_STREAM_DRAW 能进一步优化性能.不过你的场景更倾向于多次渲染,GL_DYNAMIC_DRAW 更合适.

// 与静态缓冲区的对比

// 特性      	静态缓冲区 (GL_STATIC_DRAW)	    动态缓冲区 (GL_DYNAMIC_DRAW)
// 数据更新频率	    很少或从不更新	                频繁更新
// 更新方式	    通常替换整个缓冲区	                支持部分更新
// 内存分配	    一次性,可能优化为 GPU 内存	        保留灵活性,避免重复分配
// 性能开销	    初始化快,更新慢	                初始化稍慢,更新快
// 典型用途	    固定几何体(如模型)	            动态几何体(如 CAD 线条)

// 进一步优化的建议
// 分块管理:
// 将数万条线分成多个缓冲区(例如每 1000 条线一个 VBO/EBO),只更新受影响的块,减少单次传输的数据量.
// 双缓冲:
// 使用两个动态缓冲区,一个用于渲染,一个用于更新,交替切换以避免渲染和更新冲突.
// 映射缓冲区:
// 使用 glMapBuffer 直接修改缓冲区内容,避免额外的拷贝:
// cpp
// 換行
// 複製
// glBindBuffer(GL_ARRAY_BUFFER, VBO);
// float* data = (float*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
// data[125 * 3] = newPoint.x;
// data[125 * 3 + 1] = newPoint.y;
// data[125 * 3 + 2] = newLen;
// glUnmapBuffer(GL_ARRAY_BUFFER);
// 批量更新:
// 累积多次编辑,然后一次性更新缓冲区,减少 glBufferSubData 调用次数.

// 总结
// 在你的 CAD 软件中,使用动态缓冲区的好处包括:

// 高效更新:支持实时编辑线条,适合交互式应用.
// 性能优化:减少内存分配和数据传输开销,适应数万条线的规模.
// 灵活性:为未来的动态功能(如动画、用户输入)奠定基础.
// 相比静态缓冲区,动态缓冲区是 CAD 场景的理想选择.如果你需要实现具体的动态更新功能(例如编辑某条线),我可以进一步提供代码示例！有什么具体需求吗？