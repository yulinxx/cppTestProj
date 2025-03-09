#include <iostream>
#include <Eigen/Dense>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <ctime> 


const float MIN_SCALE = 0.1f;
const float MAX_SCALE = 10.0f;


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

float dScale = 1.0f; 
Eigen::Vector2f offset(0.0f, 0.0f); 
bool bIsDragging = false; 
Eigen::Vector2f lastMousePos; 


Eigen::Matrix4f orthoMatrix = Eigen::Matrix4f::Identity();





void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    
    float aspect = static_cast<float>(width) / height;
    orthoMatrix = Eigen::Matrix4f::Identity();
    orthoMatrix(0, 0) = dScale / aspect;
    orthoMatrix(1, 1) = dScale;
    orthoMatrix(0, 3) = -offset.x();
    orthoMatrix(1, 3) = -offset.y();
}


void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_MIDDLE)
    {
        if (action == GLFW_PRESS)
        {
            bIsDragging = true;
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastMousePos = Eigen::Vector2f(xpos, ypos);
        }
        else if (action == GLFW_RELEASE)
        {
            bIsDragging = false;
        }
    }
}

GLuint VAO1, VBO1;
GLuint VAO2, VBO2;
std::vector<Eigen::Vector2f> vCurvePts1;
std::vector<Eigen::Vector2f> vCurvePts2;



Eigen::Matrix4f createTranslationMatrix(const Eigen::Vector2f& translation)
{
    Eigen::Matrix4f translationMatrix = Eigen::Matrix4f::Identity();
    translationMatrix(0, 3) = translation.x();
    translationMatrix(1, 3) = translation.y();
    return translationMatrix;
}

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


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

    if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        switch (key)
        {
        case GLFW_KEY_UP:
        {
            Eigen::Vector2f move(0.0f, 0.1f);
            Eigen::Matrix4f transMat = createTranslationMatrix(move);
            for (auto& point : vCurvePts1)
            {
                // 将二维点转换为四维齐次坐标
                Eigen::Vector4f homogeneousPoint(point.x(), point.y(), 0.0f, 1.0f);
                // 应用变换矩阵
                homogeneousPoint = transMat * homogeneousPoint;
                // 将结果转换回二维点
                point = Eigen::Vector2f(homogeneousPoint.x(), homogeneousPoint.y());
            }

            glBindVertexArray(VAO1);
            glBindBuffer(GL_ARRAY_BUFFER, VBO1);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vCurvePts1.size() * sizeof(Eigen::Vector2f), vCurvePts1.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
            break;
        case GLFW_KEY_DOWN:
        {
            // 创建负方向的平移矩阵
            Eigen::Vector2f move(0.0f, -0.1f);
            Eigen::Matrix4f transMat = createTranslationMatrix(move);

            for (auto& point : vCurvePts1)
            {
                Eigen::Vector4f homogeneousPoint(point.x(), point.y(), 0.0f, 1.0f);
                homogeneousPoint = transMat * homogeneousPoint;
                point = Eigen::Vector2f(homogeneousPoint.x(), homogeneousPoint.y());
            }

            glBindVertexArray(VAO1);
            glBindBuffer(GL_ARRAY_BUFFER, VBO1);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vCurvePts1.size() * sizeof(Eigen::Vector2f), vCurvePts1.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
            break;
        case GLFW_KEY_LEFT:
        {
            Eigen::Vector2f move(0.1f, 0.0f);
            Eigen::Matrix4f transMat = createTranslationMatrix(move);

            for (auto& point : vCurvePts2)
            {
                Eigen::Vector4f homogeneousPoint(point.x(), point.y(), 0.0f, 1.0f);
                homogeneousPoint = transMat * homogeneousPoint;
                point = Eigen::Vector2f(homogeneousPoint.x(), homogeneousPoint.y());
            }

            glBindVertexArray(VAO2); // 修正：绑定正确的 VAO
            glBindBuffer(GL_ARRAY_BUFFER, VBO2);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vCurvePts2.size() * sizeof(Eigen::Vector2f), vCurvePts2.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
            break;
        case GLFW_KEY_RIGHT:
        {
            Eigen::Vector2f move(-0.1f, 0.0f);
            Eigen::Matrix4f transMat = createTranslationMatrix(move);

            for (auto& point : vCurvePts2)
            {
                Eigen::Vector4f homogeneousPoint(point.x(), point.y(), 0.0f, 1.0f);
                homogeneousPoint = transMat * homogeneousPoint;
                point = Eigen::Vector2f(homogeneousPoint.x(), homogeneousPoint.y());
            }

            glBindVertexArray(VAO2);
            glBindBuffer(GL_ARRAY_BUFFER, VBO2);
            glBufferSubData(GL_ARRAY_BUFFER, 0, vCurvePts2.size() * sizeof(Eigen::Vector2f), vCurvePts2.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
            break;
        }
    }
}


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


GLuint linkShaderProgram(GLuint vertexShader, GLuint fragmentShader)
{
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
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
        void main()
        {
            gl_Position = orthoMatrix * vec4(aPos, 0.0, 1.0);
        }
    )";

    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        void main()
        {
            FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        }
    )";

    
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    
    GLuint shaderProgram = linkShaderProgram(vertexShader, fragmentShader);

    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    
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