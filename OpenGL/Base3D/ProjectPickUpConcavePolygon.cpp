#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

// ============================================================
// 项目: 凹多边形鼠标拾取系统
// 功能: 使用耳切法三角化凹多边形，结合颜色编码实现高效拾取
// 主要技术点: 耳切法三角剖分、点在三角形内检测、颜色编码拾取
// ============================================================

// ===================== 窗口常量 =====================
const unsigned int SCR_WIDTH = 1400;   // 窗口宽度
const unsigned int SCR_HEIGHT = 1000;  // 窗口高度

// ===================== 多边形结构 =====================
struct Polygon
{
    std::vector<glm::vec3> vertices;      // 原始多边形顶点（世界坐标）
    std::vector<unsigned int> triIndices; // 耳切法得到的三角形索引
    glm::vec3 color;                      // 正常显示颜色
    glm::vec3 pickColor;                  // 拾取编码颜色（ID → RGB）
};

// ===================== 全局资源 =====================
std::vector<Polygon> polygons;   // 所有多边形对象
unsigned int VBO, VAO, EBO;      // OpenGL缓冲区对象
unsigned int shaderProgram, pickShaderProgram; // 着色器程序
glm::mat4 projection, view;      // 投影和视图矩阵
bool pickMode = false;           // 一帧内触发拾取标志
int selectedPolygon = -1;        // 当前高亮的多边形索引 (-1表示无选中)

// ===================== 调试工具 =====================
void checkGLError(const char* operation)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
        std::cerr << "⚠️ GL error after " << operation << " : " << err << std::endl;
}

// ===================== 着色器源码 =====================
// 顶点着色器: 处理坐标变换
const char* vertexShaderSource = R"(#version 460 core
layout (location = 0) in vec3 aPos;    // 顶点位置输入
uniform mat4 model;     // 模型矩阵
uniform mat4 view;      // 视图矩阵
uniform mat4 projection;// 投影矩阵
void main(){
    // 变换顺序: 局部坐标 → 世界坐标 → 观察坐标 → 裁剪坐标
    gl_Position = projection * view * model * vec4(aPos, 1.0);
})";

// 标准片段着色器: 用于正常显示多边形
const char* fragmentShaderSource = R"(#version 460 core
out vec4 FragColor;     // 输出颜色
uniform vec3 color;     // 输入颜色
void main(){
    FragColor = vec4(color, 1.0); // 输出固定颜色
})";

// 拾取片段着色器: 用于编码ID为颜色
const char* pickFragmentShaderSource = R"(#version 460 core
out vec4 FragColor;     // 输出颜色
uniform vec3 pickColor; // 拾取颜色（编码ID）
void main(){
    FragColor = vec4(pickColor, 1.0); // 输出拾取颜色
})";

// ===================== 着色器编译辅助 =====================
// 编译单个着色器
unsigned int compileShader(const char* source, GLenum type)
{
    unsigned int shader = glCreateShader(type);     // 创建着色器对象
    glShaderSource(shader, 1, &source, nullptr);    // 设置着色器源码
    glCompileShader(shader);                        // 编译着色器

    // 检查编译错误
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "❌ Shader compile failed:\n" << infoLog << std::endl;
        return 0;
    }
    return shader;
}

// 创建着色器程序
unsigned int createShaderProgram(const char* vertexSource, const char* fragmentSource)
{
    // 编译顶点和片段着色器
    unsigned int vs = compileShader(vertexSource, GL_VERTEX_SHADER);
    unsigned int fs = compileShader(fragmentSource, GL_FRAGMENT_SHADER);
    if (!vs || !fs) return 0; // 任一着色器编译失败则返回

    // 创建并链接程序
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);

    // 检查链接错误
    int success;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(prog, 512, nullptr, infoLog);
        std::cerr << "❌ Program link failed:\n" << infoLog << std::endl;
        return 0;
    }

    // 链接成功后可删除着色器对象
    glDeleteShader(vs);
    glDeleteShader(fs);
    return prog;
}

// ===================== 点在三角形内检测算法 =====================
// 使用叉号法检测二维点是否在三角形内部
// 算法原理: 点与三角形各边组成的三个叉积符号相同则在内部
bool isPointInTriangle(const glm::vec2& p, const glm::vec2& a, const glm::vec2& b, const glm::vec2& c)
{
    // 计算叉积的辅助lambda函数
    // 叉积>0表示逆时针转向，<0表示顺时针转向，=0表示共线
    auto sign = [](const glm::vec2& p1, const glm::vec2& p2, const glm::vec2& p3) {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
        };

    // 计算点p与三角形三条边的叉积
    float d1 = sign(p, a, b);
    float d2 = sign(p, b, c);
    float d3 = sign(p, c, a);

    // 检查是否包含正负叉积
    bool has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
    bool has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

    // 如果叉积同号（都正或都负），点在三角形内
    // 如果有零值，表示点在边上
    return !(has_neg && has_pos);
}

// ===================== 耳切法核心算法 =====================
// 判断顶点是否为"耳朵"（可安全移除的凸顶点）
bool isEar(const std::vector<glm::vec3>& vertices,
    const std::vector<unsigned int>& indices,
    size_t i, size_t n)
{
    // 获取当前顶点、前一个顶点和后一个顶点的索引（环形结构）
    size_t prev = (i == 0) ? n - 1 : i - 1;
    size_t next = (i == n - 1) ? 0 : i + 1;

    // 提取二维坐标（忽略Z轴）
    glm::vec2 a(vertices[indices[prev]].x, vertices[indices[prev]].y);
    glm::vec2 b(vertices[indices[i]].x, vertices[indices[i]].y);
    glm::vec2 c(vertices[indices[next]].x, vertices[indices[next]].y);

    // 1. 凸性检测
    // 通过计算边ab和边bc的叉积来判断顶点b是否为凸点
    // 假设多边形顶点按逆时针顺序排列，凸点的叉积应为正
    glm::vec2 ab = b - a;
    glm::vec2 bc = c - b;
    float cross = ab.x * bc.y - ab.y * bc.x;
    if (cross <= 0) return false;  // 凹点不能作为耳朵

    // 2. 验证耳朵三角形内部是否包含其他顶点
    // 遍历所有非相邻顶点，检查是否在耳朵三角形内部
    for (size_t j = 0; j < n; ++j)
    {
        if (j == prev || j == i || j == next) continue;  // 跳过相邻顶点
        glm::vec2 p(vertices[indices[j]].x, vertices[indices[j]].y);
        if (isPointInTriangle(p, a, b, c)) return false; // 包含其他顶点，不是耳朵
    }

    // 通过所有测试，是一个有效的耳朵
    return true;
}

// ===================== 耳切法多边形三角化 =====================
// 算法原理:
// 1. 每次找到一个"耳朵"(可安全移除的凸顶点)
// 2. 输出该耳朵形成的三角形
// 3. 移除该顶点，缩小多边形规模
// 4. 重复直到剩余3个顶点，形成最后一个三角形
void triangulatePolygon(const std::vector<glm::vec3>& vertices,
    std::vector<unsigned int>& triIndices)
{
    triIndices.clear();  // 清空输出索引数组
    const size_t n = vertices.size();
    if (n < 3) return;  // 至少需要3个顶点才能形成多边形

    // 创建顶点索引数组（避免直接修改原始顶点数据）
    std::vector<unsigned int> indices(n);
    for (size_t i = 0; i < n; ++i) indices[i] = static_cast<unsigned int>(i);

    // 逐步减少多边形顶点数量，直到剩余3个
    size_t remaining = n;
    while (remaining > 3)
    {
        bool foundEar = false;
        // 遍历当前多边形的所有顶点，寻找耳朵
        for (size_t i = 0; i < remaining; ++i)
        {
            if (isEar(vertices, indices, i, remaining))
            {
                // 获取耳朵的三个顶点索引
                size_t prev = (i == 0) ? remaining - 1 : i - 1;
                size_t next = (i == remaining - 1) ? 0 : i + 1;

                // 输出一个三角形（按逆时针顺序）
                triIndices.push_back(indices[prev]);
                triIndices.push_back(indices[i]);
                triIndices.push_back(indices[next]);

                // 从索引数组中删除当前耳朵顶点
                indices.erase(indices.begin() + i);
                --remaining;  // 顶点数量减1
                foundEar = true;
                break;  // 每次只处理一个耳朵
            }
        }
        if (!foundEar) break;   // 防御机制：避免陷入无限循环
    }

    // 处理最后剩余的三个顶点，形成最终的三角形
    if (remaining == 3)
    {
        triIndices.push_back(indices[0]);
        triIndices.push_back(indices[1]);
        triIndices.push_back(indices[2]);
    }
}

// ===================== 生成随机凹多边形 =====================
void initPolygons()
{
    // 随机数生成器初始化
    std::random_device rd;
    std::mt19937 gen(rd()); // Mersenne Twister 19937 随机数引擎

    // 各种随机分布
    std::uniform_real_distribution<float> posDist(-5.0f, 5.0f);      // 位置分布
    std::uniform_real_distribution<float> zDist(-10.0f, 0.0f);       // Z轴深度分布
    std::uniform_real_distribution<float> colorDist(0.2f, 1.0f);     // 颜色分布
    std::uniform_real_distribution<float> sizeDist(0.5f, 1.5f);      // 大小分布
    std::uniform_int_distribution<int> vertexDist(4, 8);             // 顶点数分布(4-8边形)

    polygons.clear();  // 清空多边形列表

    // 生成50个随机凹多边形
    for (int i = 0; i < 50; ++i)
    {
        Polygon poly;
        // 随机生成多边形中心位置和大小
        float cx = posDist(gen);       // 中心X坐标
        float cy = posDist(gen);       // 中心Y坐标
        float z = zDist(gen);         // Z坐标
        float size = sizeDist(gen);    // 多边形大小
        int n = vertexDist(gen);       // 顶点数量

        // 生成凹多边形顶点：使用交替半径方法制造凹形状
        for (int j = 0; j < n; ++j)
        {
            float angle = 2.0f * glm::pi<float>() * j / n;  // 当前顶点角度
            // 通过交替半径制造凹凸效果：奇数顶点半径小，偶数顶点半径大
            float radius = size * (0.5f + 0.5f * (j % 2));
            // 计算顶点坐标并添加到多边形
            poly.vertices.emplace_back(cx + radius * cos(angle),
                cy + radius * sin(angle), z);
        }

        // 设置多边形颜色
        poly.color = glm::vec3(colorDist(gen), colorDist(gen), colorDist(gen));

        // 生成拾取颜色：将多边形ID编码为24位RGB颜色
        // 这种编码方式最多支持256*256*256=16,777,216个不同对象
        poly.pickColor = glm::vec3(((i & 0xFF0000) >> 16) / 255.0f, // R通道存储高8位
            ((i & 0x00FF00) >> 8) / 255.0f, // G通道存储中8位
            (i & 0x0000FF) / 255.0f); // B通道存储低8位

        // 对多边形进行三角化
        triangulatePolygon(poly.vertices, poly.triIndices);
        polygons.push_back(poly);  // 添加到多边形列表
    }
}

// ===================== 初始化OpenGL缓冲区 =====================
bool initBuffers()
{
    // 生成顶点数组对象、顶点缓冲对象和元素缓冲对象
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    // 检查创建是否成功
    if (!VAO || !VBO || !EBO) return false;

    // 配置VAO和VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 设置顶点属性指针：位置属性，3个浮点数，不标准化，步长为3个float，偏移量为0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0); // 启用位置属性

    return true;
}

// ===================== 场景渲染 =====================
// 参数说明：
// - picking = true: 拾取模式，绘制三角形并使用ID颜色编码
// - picking = false: 正常模式，绘制多边形线框并支持高亮显示
void renderScene(bool picking)
{
    // 清空颜色和深度缓冲区
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 选择适当的着色器程序
    unsigned int prog = picking ? pickShaderProgram : shaderProgram;
    glUseProgram(prog);

    // 设置变换矩阵
    glUniformMatrix4fv(glGetUniformLocation(prog, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(prog, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    // 由于顶点已在世界坐标系中，model矩阵为单位矩阵
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(prog, "model"), 1, GL_FALSE, glm::value_ptr(model));

    // 绑定顶点数组对象
    glBindVertexArray(VAO);

    // 遍历所有多边形
    for (size_t i = 0; i < polygons.size(); ++i)
    {
        const Polygon& poly = polygons[i];

        // 上传当前多边形顶点数据到VBO
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER,
            poly.vertices.size() * sizeof(glm::vec3),
            poly.vertices.data(),
            GL_DYNAMIC_DRAW);  // 使用动态绘制，因为每个多边形都不同

        if (picking)  // 拾取模式：绘制三角形并使用ID颜色
        {
            // 设置拾取颜色（编码多边形ID）
            glUniform3fv(glGetUniformLocation(prog, "pickColor"), 1, glm::value_ptr(poly.pickColor));

            // 上传三角形索引到EBO
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                poly.triIndices.size() * sizeof(unsigned int),
                poly.triIndices.data(),
                GL_DYNAMIC_DRAW);

            // 使用索引绘制三角形
            glDrawElements(GL_TRIANGLES,
                static_cast<GLsizei>(poly.triIndices.size()),
                GL_UNSIGNED_INT,
                nullptr);
        }
        else  // 正常模式：绘制多边形线框
        {
            // 确定绘制颜色：选中的多边形使用黄色高亮，其他使用原始颜色
            glm::vec3 drawColor = (i == selectedPolygon)
                ? glm::vec3(1.0f, 1.0f, 0.0f)  // 黄色高亮
                : poly.color;                 // 原始颜色
            glUniform3fv(glGetUniformLocation(prog, "color"), 1, glm::value_ptr(drawColor));

            // 使用GL_LINE_LOOP绘制多边形线框
            glDrawArrays(GL_LINE_LOOP, 0, static_cast<GLsizei>(poly.vertices.size()));
        }
    }
}

// ===================== 鼠标按钮回调 =====================
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    // 当左键按下时，触发拾取模式
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        pickMode = true;  // 下一帧将进入拾取分支
}

// ===================== 主函数 =====================
int main()
{
    // 1. 初始化GLFW和创建窗口
    if (!glfwInit()) return -1;  // 初始化GLFW

    // 设置OpenGL版本和配置
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT,
        "Concave Polygon Picking (Commented)", nullptr, nullptr);

    if (!window)
    {
        glfwTerminate(); return -1;
    }  // 检查窗口创建

    glfwMakeContextCurrent(window);  // 设置当前上下文
    glfwSetMouseButtonCallback(window, mouse_button_callback);  // 注册鼠标回调

    // 加载OpenGL函数指针
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) 
        return -1;
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    // 设置视口和启用深度测试
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // 2. 初始化着色器和多边形数据
    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    pickShaderProgram = createShaderProgram(vertexShaderSource, pickFragmentShaderSource);
    initPolygons();  // 生成随机凹多边形
    initBuffers();   // 创建OpenGL缓冲区

    // 设置投影矩阵和视图矩阵
    projection = glm::perspective(glm::radians(45.0f),   // 45度视场角
        float(SCR_WIDTH) / SCR_HEIGHT,  // 宽高比
        0.1f, 100.0f);  // 近远裁剪面

    // 相机位置在Z轴10的位置，看向原点，上方向为Y轴
    view = glm::lookAt(glm::vec3(0, 0, 10),  // 相机位置
        glm::vec3(0, 0, 0),   // 目标位置
        glm::vec3(0, 1, 0));  // 上方向

    // 3. 主渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // 3.1 处理拾取请求
        if (pickMode)
        {
            // 在拾取模式下渲染场景：绘制三角形并使用ID颜色
            renderScene(true);

            // 获取鼠标位置
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            // 转换坐标系：GLFW的原点在左上角，OpenGL的原点在左下角
            ypos = SCR_HEIGHT - ypos;

            // 读取鼠标位置的像素颜色
            unsigned char pixel[3];
            glReadPixels((int)xpos, (int)ypos, 1, 1,
                GL_RGB, GL_UNSIGNED_BYTE, pixel);

            // 解码颜色为多边形ID：将RGB通道组合为一个整数
            int pickedID = (pixel[0] << 16) | (pixel[1] << 8) | pixel[2];

            // 更新选中状态
            if (pickedID >= 0 && pickedID < (int)polygons.size())
            {
                selectedPolygon = pickedID;
                std::cout << "Selected ID: " << selectedPolygon << std::endl;
            }
            else
            {
                selectedPolygon = -1;  // 没有选中任何多边形
            }

            pickMode = false;  // 清除拾取标志
        }

        // 3.2 正常渲染场景
        renderScene(false);

        // 交换前后缓冲区，处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 4. 清理资源
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(pickShaderProgram);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}