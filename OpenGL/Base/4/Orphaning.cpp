/*
OpenGL 4.4+ 持久化映射（Persistent Mapping）优化版本
以下是使用 glBufferStorage + GL_MAP_PERSISTENT_BIT 技术的完整示例，
这是现代 OpenGL 中性能最高的动态缓冲区更新方案，
相比 Orphaning 可进一步减少驱动开销。

| 特性            | **Orphaning**             | **持久化映射**                          |
| ------------- | -------------------------- | ------------------------------------- |
| **内存分配**      | 每帧可能分配新存储         | 一次性分配，永不改变                      |
| **CPU-GPU同步** | 驱动异步释放旧存储          | 显式 `glFenceSync` + `glClientWaitSync` |
| **拷贝次数**      | 1次 (`memcpy`→驱动→GPU) | 0次 (`memcpy`直接到映射内存)               |
| **CPU开销**     | 中（驱动管理孤儿内存）      | **极低**（纯用户态操作）                    |
| **实现复杂度**     | 简单                   | 复杂（需正确同步）                          |
| **适用版本**      | OpenGL 2.1+            | **OpenGL 4.4+**                          |
| **峰值内存**      | 瞬时翻倍                 | 固定不变                                  |


*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <string>
#include <cmath>
#include <cstring>

// ==================== 着色器代码 ====================
const char* vertexShaderSource = R"(
#version 440 core  // 需要OpenGL 4.4+
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 vertexColor;
void main() {
    gl_Position = vec4(aPos, 1.0);
    vertexColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 440 core
in vec3 vertexColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vertexColor, 1.0);
}
)";

// ==================== 同步辅助结构 ====================
struct SyncObject {
    GLuint fence = 0;  // GLsync 对象
    
    void create() {
        if (fence) glDeleteSync(fence);
        // 创建同步点：插入到OpenGL命令流中
        fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    }
    
    // 等待GPU完成（非阻塞检查）
    void wait() {
        if (!fence) return;
        
        // 等待1纳秒（非阻塞轮询）
        while (true) {
            GLenum result = glClientWaitSync(fence, GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED) {
                break;
            }
            // 避免CPU空转，让出时间片
            if (result == GL_WAIT_TIMEOUT) {
                std::this_thread::yield();
            }
        }
        glDeleteSync(fence);
        fence = 0;
    }
    
    ~SyncObject() {
        if (fence) glDeleteSync(fence);
    }
};

// ==================== 持久化缓冲区管理器 ====================
struct PersistentBuffer {
    GLuint buffer = 0;
    void* mappedPtr = nullptr;      // 永久映射的CPU指针
    GLsizeiptr bufferSize = 0;
    
    SyncObject sync;                // GPU完成信号
    
    // 一次性分配 + 永久映射
    bool allocate(GLsizeiptr size) {
        bufferSize = size;
        
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        
        // ======== 关键：使用 glBufferStorage 分配不可变存储 ========
        // GL_MAP_PERSISTENT_BIT: 允许永久映射
        // GL_MAP_COHERENT_BIT: 自动同步（无需手动刷）
        // GL_MAP_WRITE_BIT: 允许写入
        glBufferStorage(GL_ARRAY_BUFFER, size, nullptr, 
                        GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
        
        // 永久映射，永不解除映射！
        mappedPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, size, 
                                     GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT);
        
        if (!mappedPtr) {
            std::cerr << "持久化映射失败！" << std::endl;
            return false;
        }
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        return true;
    }
    
    // 更新数据（直接写入映射内存，零拷贝）
    void updateData(const void* data, GLsizeiptr size) {
        // 等待GPU完成上一帧的读取
        sync.wait();
        
        // 直接内存拷贝（无驱动层开销） 优势：绕过驱动层，速度接近内存带宽极限。
        memcpy(mappedPtr, data, size);
        
        // 注意：由于使用了 GL_MAP_COHERENT_BIT，无需 glFlushMappedBufferRange
    }
    
    // 渲染前调用：标记缓冲区将被GPU使用
    void beforeRender() {
        sync.create();  // 在命令流中插入同步点
    }
    
    ~PersistentBuffer() {
        if (buffer && mappedPtr) {
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glUnmapBuffer(GL_ARRAY_BUFFER);  // 程序结束时解除映射
        }
        if (buffer) glDeleteBuffers(1, &buffer);
    }
};

// ==================== 演示主结构 ====================
struct OrphaningDemo {
    GLFWwindow* window = nullptr;
    GLuint shaderProgram = 0;
    GLuint VAO = 0;
    
    PersistentBuffer persistentVBO;  // 持久化VBO
    
    std::vector<float> vertices;     // CPU端数据缓存
    const size_t vertexCount = 50000;
    bool useOrphaning = false;       // 可切换对比
    float frameTime = 0.0f;
    size_t framesRendered = 0;
    
    double avgTimeOrphaning = 0.0;
    double avgTimePersistent = 0.0;
};

// ==================== 初始化函数（与之前类似）=====================
bool initGLFW(OrphaningDemo& demo) {
    if (!glfwInit()) {
        std::cerr << "GLFW 初始化失败！" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    demo.window = glfwCreateWindow(800, 600, "持久化映射 vs Orphaning", nullptr, nullptr);
    if (!demo.window) {
        std::cerr << "GLFW 窗口创建失败！" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(demo.window);
    return true;
}

bool initGLAD() {
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD 初始化失败！" << std::endl;
        return false;
    }
    return true;
}

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "着色器编译失败：" << infoLog << std::endl;
    }
    return shader;
}

GLuint createShaderProgram() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "着色器链接失败：" << infoLog << std::endl;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return program;
}

void generateDynamicVertices(OrphaningDemo& demo) {
    demo.vertices.clear();
    demo.vertices.reserve(demo.vertexCount * 6);
    
    float time = glfwGetTime();
    for (size_t i = 0; i < demo.vertexCount; ++i) {
        float angle = static_cast<float>(i) / demo.vertexCount * 2.0f * M_PI * 10.0f;
        float radius = 0.5f + 0.3f * sin(time + angle * 0.1f);
        
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        float z = 0.0f;
        
        float r = 0.5f + 0.5f * sin(time + angle * 0.2f);
        float g = 0.5f + 0.5f * sin(time + angle * 0.3f + 2.0f);
        float b = 0.5f + 0.5f * sin(time + angle * 0.4f + 4.0f);
        
        demo.vertices.push_back(x);
        demo.vertices.push_back(y);
        demo.vertices.push_back(z);
        demo.vertices.push_back(r);
        demo.vertices.push_back(g);
        demo.vertices.push_back(b);
    }
}

// ==================== Orphaning 更新（用于对比）=====================
void updateBufferWithOrphaning(OrphaningDemo& demo) {
    auto start = std::chrono::high_resolution_clock::now();
    
    glBindBuffer(GL_ARRAY_BUFFER, demo.persistentVBO.buffer);
    
    // 传统 Orphaning（分配新存储）
    glBufferData(GL_ARRAY_BUFFER, 
                 demo.vertices.size() * sizeof(float), 
                 nullptr, // NULL 指针
                 GL_DYNAMIC_DRAW);
    
    // 写入数据
    glBufferSubData(GL_ARRAY_BUFFER, 
                    0, 
                    demo.vertices.size() * sizeof(float), 
                    demo.vertices.data());
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    auto end = std::chrono::high_resolution_clock::now();
    demo.avgTimeOrphaning += std::chrono::duration<double, std::milli>(end - start).count();
}

// ==================== 持久化映射更新 ====================
void updateBufferPersistent(OrphaningDemo& demo) {
    auto start = std::chrono::high_resolution_clock::now();
    
    // ======== 核心：直接写入永久映射内存 ========
    // 1. 等待GPU完成（0.1ms级别）
    demo.persistentVBO.sync.wait();
    
    // 2. 零拷贝写入（memcpy速度，无驱动层开销）
    memcpy(demo.persistentVBO.mappedPtr, 
           demo.vertices.data(), 
           demo.vertices.size() * sizeof(float));
    
    // 3. 标记GPU开始使用（插入同步点）
    demo.persistentVBO.beforeRender();
    
    auto end = std::chrono::high_resolution_clock::now();
    demo.avgTimePersistent += std::chrono::duration<double, std::milli>(end - start).count();
}

// ==================== 初始化缓冲区 ====================
void initBuffers(OrphaningDemo& demo) {
    generateDynamicVertices(demo);
    
    // 创建 VAO
    glGenVertexArrays(1, &demo.VAO);
    glBindVertexArray(demo.VAO);
    
    // 分配持久化VBO
    GLsizeiptr bufferSize = demo.vertices.size() * sizeof(float);
    demo.persistentVBO.allocate(bufferSize);
    
    // 初始数据写入
    demo.persistentVBO.updateData(demo.vertices.data(), bufferSize);
    
    // 设置顶点属性
    glBindBuffer(GL_ARRAY_BUFFER, demo.persistentVBO.buffer);
    
    // 位置属性
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // 颜色属性
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    glBindVertexArray(0);
}

// ==================== 渲染循环 ====================
void render(OrphaningDemo& demo) {
    auto frameStart = std::chrono::high_resolution_clock::now();
    
    generateDynamicVertices(demo);
    
    // 根据模式选择更新方式
    if (demo.useOrphaning) {
        updateBufferWithOrphaning(demo);
    } else {
        updateBufferPersistent(demo);
    }
    
    // 渲染
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(demo.shaderProgram);
    
    glBindVertexArray(demo.VAO);
    glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(demo.vertexCount));
    glBindVertexArray(0);
    
    glfwSwapBuffers(demo.window);
    glfwPollEvents();
    
    auto frameEnd = std::chrono::high_resolution_clock::now();
    demo.frameTime = std::chrono::duration<float, std::milli>(frameEnd - frameStart).count();
    demo.framesRendered++;
}

// ==================== UI 和信息显示 ====================
void showInfo(const OrphaningDemo& demo) {
    std::cout << "\r\033[2K";
    
    if (demo.useOrphaning) {
        std::cout << "[模式: Orphaning] ";
        std::cout << "帧时间: " << demo.frameTime << "ms | ";
        std::cout << "上传: " << demo.avgTimeOrphaning / demo.framesRendered << "ms";
    } else {
        std::cout << "[模式: 持久化映射] ";
        std::cout << "帧时间: " << demo.frameTime << "ms | ";
        std::cout << "上传: " << demo.avgTimePersistent / demo.framesRendered << "ms";
    }
    
    std::cout << " | FPS: " << (1000.0f / demo.frameTime) << " | ";
    std::cout << "顶点: " << demo.vertexCount << " | ";
    std::cout << "按空格键切换";
    std::flush(std::cout);
}

// ==================== 主函数 ====================
int main() {
    OrphaningDemo demo;
    
    // 初始化
    if (!initGLFW(demo)) return -1;
    if (!initGLAD()) return -1;
    
    // 检查OpenGL版本
    if (GLVersion.major < 4 || (GLVersion.major == 4 && GLVersion.minor < 4)) {
        std::cerr << "错误：需要OpenGL 4.4+（当前版本: " << GLVersion.major << "." << GLVersion.minor << "）" << std::endl;
        std::cerr << "请更新显卡驱动或改用Orphaning版本" << std::endl;
        return -1;
    }
    
    demo.shaderProgram = createShaderProgram();
    initBuffers(demo);
    
    glfwSwapInterval(0);  // 禁用VSync测性能
    
    glEnable(GL_PROGRAM_POINT_SIZE);
    glPointSize(2.0f);
    
    // 主循环
    while (!glfwWindowShouldClose(demo.window)) {
        // 处理输入
        if (glfwGetKey(demo.window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            static bool keyPressed = false;
            if (!keyPressed) {
                demo.useOrphaning = !demo.useOrphaning;
                demo.framesRendered = 0;
                demo.avgTimeOrphaning = 0.0;
                demo.avgTimePersistent = 0.0;
                keyPressed = true;
            }
        } else {
            static bool firstRelease = true;
            if (firstRelease) {
                glfwSetInputMode(demo.window, GLFW_STICKY_KEYS, GLFW_FALSE);
                firstRelease = false;
            }
        }
        
        render(demo);
        
        if (demo.framesRendered > 0) {
            showInfo(demo);
        }
    }
    
    std::cout << std::endl;
    
    // 清理（RAII自动处理）
    glDeleteVertexArrays(1, &demo.VAO);
    glDeleteProgram(demo.shaderProgram);
    
    glfwTerminate();
    return 0;
}