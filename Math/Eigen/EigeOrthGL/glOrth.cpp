
/**
 * @file glOrth.cpp
 * @brief 此代码实现了一个使用 OpenGL 和 Eigen 库绘制贝塞尔曲线的程序，支持正交投影变换，包括缩放、平移等操作。
 * 
 * 该程序使用 GLFW 库创建窗口，GLEW 库初始化 OpenGL 扩展，Eigen 库进行矩阵和向量运算。
 * 程序会随机生成两组贝塞尔曲线的控制点，然后计算并绘制这两条曲线。
 * 用户可以通过鼠标和键盘操作对曲线进行缩放、平移和移动。
 * 
 - 鼠标中键 ：按下鼠标中键重置缩放因子和偏移量。
- 鼠标拖动 ：按住鼠标左键拖动进行平移操作。
- 鼠标滚动 ：滚动鼠标滚轮进行缩放操作。
- 键盘按键 ：按下 1 或 A 切换到移动第一个图元，按下 2 或 B 切换到移动第二个图元。使用方向键对当前图元进行平移操作。
- 键盘按键 L 、 K 、 R 、 T 来切换正交投影的原点位置（左下角、左上角、右上角、右下角）
 * @author [lyl]
 * @date [2025.03.09]
 * @version [v1.0]
 */

#include <iostream>
#include <Eigen/Dense>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <ctime> 

// 缩放比例的最小值和最大值
const float MIN_SCALE = 0.1f;
const float MAX_SCALE = 10.0f;

/**
 * @brief 计算贝塞尔曲线上的点
 * 
 * 根据给定的四个控制点和参数 t，计算贝塞尔曲线上对应位置的点。
 * 
 * @param p0 第一个控制点
 * @param p1 第二个控制点
 * @param p2 第三个控制点
 * @param p3 第四个控制点
 * @param t 参数 t，范围在 [0, 1] 之间
 * @return Eigen::Vector2f 贝塞尔曲线上对应位置的点
 */
Eigen::Vector2f bezier(const Eigen::Vector2f& p0, const Eigen::Vector2f& p1,
    const Eigen::Vector2f& p2, const Eigen::Vector2f& p3, float t)
{
    float u = 1 - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;

    Eigen::Vector2f p = uuu * p0;
    p += 3 * uu * t * p1;
    p += 3 * u * tt * p2;
    p += ttt * p3;

    return p;
}

// 缩放因子
float dScale = 1.0f; 
// 偏移量
Eigen::Vector2f offset(0.0f, 0.0f); 
// 是否正在拖动
bool bIsDragging = false; 
// 上次鼠标位置
Eigen::Vector2f lastMousePos; 

// 正交投影矩阵
Eigen::Matrix4f orthoMatrix = Eigen::Matrix4f::Identity();

// 定义原点位置的枚举类型
enum OriginPosition {
    BOTTOM_LEFT,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_RIGHT
};

// 当前原点位置
OriginPosition currentOrigin = BOTTOM_LEFT;

/**
 * @brief 窗口大小改变时的回调函数
 * 
 * 当窗口大小改变时，更新正交投影矩阵以适应新的窗口比例，并根据当前原点位置调整矩阵。
 * 
 * @param window GLFW 窗口指针
 * @param width 新的窗口宽度
 * @param height 新的窗口高度
 */
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    float aspect = static_cast<float>(width) / height;
    orthoMatrix = Eigen::Matrix4f::Identity();
    orthoMatrix(0, 0) = dScale / aspect;
    orthoMatrix(1, 1) = dScale;
    orthoMatrix(0, 3) = -offset.x();
    orthoMatrix(1, 3) = -offset.y();

    // 根据当前原点位置调整矩阵
    switch (currentOrigin) {
        case TOP_LEFT:
            orthoMatrix(1, 1) = -dScale;
            orthoMatrix(1, 3) = -offset.y() + 1.0f;
            break;
        case TOP_RIGHT:
            orthoMatrix(0, 0) = -dScale / aspect;
            orthoMatrix(1, 1) = -dScale;
            orthoMatrix(0, 3) = -offset.x() + 1.0f;
            orthoMatrix(1, 3) = -offset.y() + 1.0f;
            break;
        case BOTTOM_RIGHT:
            orthoMatrix(0, 0) = -dScale / aspect;
            orthoMatrix(0, 3) = -offset.x() + 1.0f;
            break;
        case BOTTOM_LEFT:
        default:
            break;
    }
}

/**
 * @brief 鼠标按钮事件的回调函数
 * 
 * 处理鼠标按钮事件，当按下鼠标中键时，重置缩放因子、偏移量和正交投影矩阵。
 * 
 * @param window GLFW 窗口指针
 * @param button 鼠标按钮
 * @param action 按钮动作（按下、释放等）
 * @param mods 修饰键状态
 */
void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) // 中键还原正交矩阵
    {
        if (action == GLFW_PRESS)
        {
            // 重置缩放因子
            dScale = 1.0f; 
            // 重置偏移量
            offset = Eigen::Vector2f(0.0f, 0.0f); 
            // 重置正交矩阵
            orthoMatrix = Eigen::Matrix4f::Identity(); 

            int w, h;
            glfwGetFramebufferSize(window, &w, &h);
            float aspect = static_cast<float>(w) / h;
            orthoMatrix(0, 0) = dScale / aspect;
            orthoMatrix(1, 1) = dScale;
            orthoMatrix(0, 3) = -offset.x();
            orthoMatrix(1, 3) = -offset.y();

            bIsDragging = false; // 确保不进入拖动状态
        }
        else if (action == GLFW_RELEASE)
        {
            bIsDragging = false;
        }
    }
}

// 第一个曲线的顶点数组对象和顶点缓冲对象
GLuint VAO1, VBO1;
// 第二个曲线的顶点数组对象和顶点缓冲对象
GLuint VAO2, VBO2;
// 第一个曲线的点集
std::vector<Eigen::Vector2f> vCurvePts1;
// 第二个曲线的点集
std::vector<Eigen::Vector2f> vCurvePts2;

/**
 * @brief 创建平移矩阵
 * 
 * 根据给定的平移向量创建一个 4x4 的平移矩阵。
 * 
 * @param translation 平移向量
 * @return Eigen::Matrix4f 平移矩阵
 */
Eigen::Matrix4f createTranslationMatrix(const Eigen::Vector2f& translation)
{
    Eigen::Matrix4f translationMatrix = Eigen::Matrix4f::Identity();
    translationMatrix(0, 3) = translation.x();
    translationMatrix(1, 3) = translation.y();
    return translationMatrix;
}

/**
 * @brief 鼠标移动事件的回调函数
 * 
 * 处理鼠标移动事件，当鼠标拖动时，更新偏移量和正交投影矩阵。
 * 
 * @param window GLFW 窗口指针
 * @param xpos 鼠标的 x 坐标
 * @param ypos 鼠标的 y 坐标
 */
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (bIsDragging)
    {
        Eigen::Vector2f currentMousePos(xpos, ypos);
        Eigen::Vector2f delta = currentMousePos - lastMousePos;
        offset += delta * 0.001f;

        Eigen::Matrix4f translationMatrix = createTranslationMatrix(offset);
        orthoMatrix = translationMatrix * orthoMatrix;

        lastMousePos = currentMousePos;
    }
}

/**
 * @brief 鼠标滚动事件的回调函数
 * 
 * 处理鼠标滚动事件，根据滚动方向更新缩放因子和正交投影矩阵。
 * 
 * @param window GLFW 窗口指针
 * @param xoffset 鼠标滚动的 x 偏移量
 * @param yoffset 鼠标滚动的 y 偏移量
 */
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    float newScale = dScale * (1.0f + yoffset * 0.1f);
    dScale = std::max(MIN_SCALE, std::min(MAX_SCALE, newScale));

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = static_cast<float>(width) / height;

    orthoMatrix(0, 0) = dScale / aspect;
    orthoMatrix(1, 1) = dScale;
}

/**
 * @brief 平移点集
 * 
 * 根据给定的平移向量平移点集，并更新顶点缓冲对象中的数据。
 * 
 * @param points 点集
 * @param move 平移向量
 * @param VAO 顶点数组对象
 * @param VBO 顶点缓冲对象
 */
void translatePoints(std::vector<Eigen::Vector2f>& points, const Eigen::Vector2f& move, GLuint VAO, GLuint VBO)
{
    Eigen::Matrix4f transMat = createTranslationMatrix(move);
    for (auto& point : points)
    {
        Eigen::Vector4f homogeneousPoint(point.x(), point.y(), 0.0f, 1.0f);
        homogeneousPoint = transMat * homogeneousPoint;
        point = Eigen::Vector2f(homogeneousPoint.x(), homogeneousPoint.y());
    }

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, points.size() * sizeof(Eigen::Vector2f), points.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// 当前要移动的图元索引，0 表示第一个图元，1 表示第二个图元
int currentMovingPrimitive = 1; 

/**
 * @brief 键盘事件的回调函数
 * 
 * 处理键盘事件，根据按下的键切换当前要移动的图元，并对其进行平移操作，同时处理原点位置切换。
 * 
 * @param window GLFW 窗口指针
 * @param key 按下的键
 * @param scancode 扫描码
 * @param action 按键动作（按下、释放等）
 * @param mods 修饰键状态
 */
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_1:
        case GLFW_KEY_A:
            // 按下 1 切换到移动第一个图元
            currentMovingPrimitive = 0; 
            break;
        case GLFW_KEY_2:
        case GLFW_KEY_B:
            currentMovingPrimitive = 1; 
            break;
        case GLFW_KEY_UP:
            if (currentMovingPrimitive == 0) {
                translatePoints(vCurvePts1, Eigen::Vector2f(0.0f, 0.1f), VAO1, VBO1);
            } else {
                translatePoints(vCurvePts2, Eigen::Vector2f(0.0f, 0.1f), VAO2, VBO2);
            }
            break;
        case GLFW_KEY_DOWN:
            if (currentMovingPrimitive == 0) {
                translatePoints(vCurvePts1, Eigen::Vector2f(0.0f, -0.1f), VAO1, VBO1);
            } else {
                translatePoints(vCurvePts2, Eigen::Vector2f(0.0f, -0.1f), VAO2, VBO2);
            }
            break;
        case GLFW_KEY_LEFT:
            if (currentMovingPrimitive == 0) {
                translatePoints(vCurvePts1, Eigen::Vector2f(-0.1f, 0.0f), VAO1, VBO1);
            } else {
                translatePoints(vCurvePts2, Eigen::Vector2f(-0.1f, 0.0f), VAO2, VBO2);
            }
            break;
        case GLFW_KEY_RIGHT:
            if (currentMovingPrimitive == 0) {
                translatePoints(vCurvePts1, Eigen::Vector2f(0.1f, 0.0f), VAO1, VBO1);
            } else {
                translatePoints(vCurvePts2, Eigen::Vector2f(0.1f, 0.0f), VAO2, VBO2);
            }
            break;
        case GLFW_KEY_L:
            currentOrigin = BOTTOM_LEFT;
            framebuffer_size_callback(window, 800, 600); // 更新正交投影矩阵
            break;
        case GLFW_KEY_K:
            currentOrigin = TOP_LEFT;
            framebuffer_size_callback(window, 800, 600); // 更新正交投影矩阵
            break;
        case GLFW_KEY_R:
            currentOrigin = TOP_RIGHT;
            framebuffer_size_callback(window, 800, 600); // 更新正交投影矩阵
            break;
        case GLFW_KEY_T:
            currentOrigin = BOTTOM_RIGHT;
            framebuffer_size_callback(window, 800, 600); // 更新正交投影矩阵
            break;
        }
    }
}

/**
 * @brief 编译着色器
 * 
 * 编译给定类型和源代码的着色器，并检查编译是否成功。
 * 
 * @param shaderType 着色器类型（如 GL_VERTEX_SHADER、GL_FRAGMENT_SHADER）
 * @param shaderSource 着色器源代码
 * @return GLuint 编译后的着色器对象
 */
GLuint compileShader(GLenum shaderType, const char* shaderSource)
{
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error: " << infoLog << std::endl;
    }
    return shader;
}

/**
 * @brief 链接着色器程序
 * 
 * 链接顶点着色器和片段着色器，创建一个完整的着色器程序，并检查链接是否成功。
 * 
 * @param vs 顶点着色器对象
 * @param fragmentShader 片段着色器对象
 * @return GLuint 链接后的着色器程序对象
 */
GLuint linkShaderProgram(GLuint vs, GLuint fragmentShader)
{
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vs);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error: " << infoLog << std::endl;
    }
    return shaderProgram;
}

/**
 * @brief 主函数
 * 
 * 程序的入口点，初始化 GLFW 和 GLEW，创建窗口，生成贝塞尔曲线的控制点和点集，
 * 编译和链接着色器程序，设置回调函数，进入主循环进行渲染。
 * 
 * @return int 程序退出状态码
 */
int main()
{
    srand(static_cast<unsigned int>(std::time(nullptr)));

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(800, 600, "Bezier Curve with Orthogonal Projection", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    std::vector<Eigen::Vector2f> controlPoints1;
    std::vector<Eigen::Vector2f> controlPoints2;
    for (int i = 0; i < 4; ++i)
    {
        float x = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        float y = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        controlPoints1.emplace_back(x, y);

        x = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        y = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        controlPoints2.emplace_back(x, y);
    }

    vCurvePts1.clear();
    vCurvePts2.clear();
    const int numSegments = 100;
    for (int i = 0; i <= numSegments; ++i)
    {
        float t = (float)i / numSegments;
        vCurvePts1.push_back(bezier(controlPoints1[0], controlPoints1[1], controlPoints1[2], controlPoints1[3], t));
        vCurvePts2.push_back(bezier(controlPoints2[0], controlPoints2[1], controlPoints2[2], controlPoints2[3], t));
    }

    glGenVertexArrays(1, &VAO1);
    glGenBuffers(1, &VBO1);
    glGenVertexArrays(1, &VAO2);
    glGenBuffers(1, &VBO2);

    glBindVertexArray(VAO1);
    glBindBuffer(GL_ARRAY_BUFFER, VBO1);
    glBufferData(GL_ARRAY_BUFFER, vCurvePts1.size() * sizeof(Eigen::Vector2f), vCurvePts1.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glBindVertexArray(VAO2);
    glBindBuffer(GL_ARRAY_BUFFER, VBO2);
    glBufferData(GL_ARRAY_BUFFER, vCurvePts2.size() * sizeof(Eigen::Vector2f), vCurvePts2.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Eigen::Vector2f), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glfwSetMouseButtonCallback(window, mouse_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        uniform mat4 orthoMatrix;
        out vec2 pos; // 传递坐标给片段着色器
    
        void main()
        {
            gl_Position = orthoMatrix * vec4(aPos, 0.0, 1.0);
            pos = aPos; // 传递坐标
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 pos;
        out vec4 FragColor;
    
        void main()
        {
            // 将 x y 坐标放到 r g 颜色中,用于观察
            //FragColor = vec4(pos.x, pos.y, 0.0f, 1.0f);
            FragColor = vec4(pos.x, 0.0f, 0.0f, 1.0f);
        }
    )";

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint shaderProgram = linkShaderProgram(vs, fs);

    glDeleteShader(vs);
    glDeleteShader(fs);

    GLint orthoMatrixLoc = glGetUniformLocation(shaderProgram, "orthoMatrix");

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glUniformMatrix4fv(orthoMatrixLoc, 1, GL_FALSE, orthoMatrix.data());

        glBindVertexArray(VAO1);
        glDrawArrays(GL_LINE_STRIP, 0, vCurvePts1.size());
        glBindVertexArray(0);

        glBindVertexArray(VAO2);
        glDrawArrays(GL_LINE_STRIP, 0, vCurvePts2.size());
        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO1);
    glDeleteBuffers(1, &VBO1);
    glDeleteVertexArrays(1, &VAO2);
    glDeleteBuffers(1, &VBO2);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}