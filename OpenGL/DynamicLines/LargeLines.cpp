// main.cpp   (保存为 main.cpp，C++17)
// g++ main.cpp -lglfw -ldl -lGL -lstdc++fs -O3 -std=c++17 && ./a.out

#define _GNU_SOURCE
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <vector>
#include <iostream>
#include <random>
#include <chrono>
#include <cstring>

using namespace std;
using namespace glm;

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// ------------------- 基础类型 -------------------
using Point = bg::model::point<float, 2, bg::cs::cartesian>;
using Box = bg::model::box<Point>;
using Value = pair<Box, size_t>;                // RTree 存储 (bbox, index)

struct Primitive;
vector<Primitive*> g_primitives;
bgi::rtree<Value, bgi::quadratic<16>> g_rtree;

// ------------------- 图元定义 -------------------
enum class PrimType
{
    Line, Circle, Polyline
};

struct Primitive
{
    Box     aabb;
    PrimType type;
    vec2    p1, p2;           // Line: start/end, Circle: center, Polyline: unused
    float   radius = 0.f;
    vec4    color = vec4(0.8f, 0.8f, 0.8f, 1.f);
    float   thickness = 2.f;
    vector<vec2> points;      // 只 Polyline 用

    virtual ~Primitive() = default;
    virtual void rasterize(vector<float>& buffer) const = 0;
};

struct Line : Primitive
{
    Line(vec2 a, vec2 b, vec4 c = vec4(0.9f), float t = 2.f)
    {
        type = PrimType::Line;
        p1 = a; p2 = b; color = c; thickness = t;
        bg::envelope(bg::model::segment<Point>(Point(a.x, a.y), Point(b.x, b.y)), aabb);
    }
    void rasterize(vector<float>& buf) const override
    {
        // pos(2) + color(4) + thickness(1) + dash(1)
        buf.insert(buf.end(), { p1.x, p1.y, color.r, color.g, color.b, color.a, thickness, 0.f });
        buf.insert(buf.end(), { p2.x, p2.y, color.r, color.g, color.b, color.a, thickness, 0.f });
    }
};

struct Circle : Primitive
{
    Circle(vec2 c, float r, vec4 col = vec4(0.2f, 0.7f, 1.f, 1.f), float t = 2.f)
    {
        type = PrimType::Circle;
        p1 = c; radius = r; color = col; thickness = t;
        Point cp(c.x, c.y);
        bg::envelope(bg::model::ring<Point>({ cp,cp,cp,cp,cp }), aabb);
        bg::expand(aabb, bg::model::box<Point>(Point(c.x - r, c.y - r), Point(c.x + r, c.y + r)));
    }
    void rasterize(vector<float>& buf) const override
    {
        const int segments = 64;
        float angleStep = 6.283185f / segments;
        for (int i = 0; i <= segments; ++i)
        {
            float a = i * angleStep;
            vec2 p = p1 + radius * vec2(cosf(a), sinf(a));
            buf.insert(buf.end(), { p.x, p.y, color.r,color.g,color.b,color.a, thickness, 0.f });
        }
    }
};

struct PolylinePrim : Primitive
{
    PolylinePrim(const vector<vec2>& pts, vec4 c = vec4(1, 0.6f, 0, 1), float t = 3.f)
    {
        type = PrimType::Polyline;
        points = pts; color = c; thickness = t;
        if (pts.size() < 2) return;
        Box b(Point(pts[0].x, pts[0].y), Point(pts[0].x, pts[0].y));
        for (auto& p : pts) bg::expand(b, Point(p.x, p.y));
        aabb = b;
    }
    void rasterize(vector<float>& buf) const override
    {
        for (size_t i = 0; i + 1 < points.size(); ++i)
        {
            auto a = points[i], b = points[i + 1];
            buf.insert(buf.end(), { a.x,a.y, color.r,color.g,color.b,color.a, thickness, 0.f });
            buf.insert(buf.end(), { b.x,b.y, color.r,color.g,color.b,color.a, thickness, 0.f });
        }
    }
};

// ------------------- 渲染器 -------------------
struct Batch
{
    GLuint VAO = 0, VBO = 0;
    size_t capacity = 4'000'000;   // 4M floats ≈ 32MB
    vector<float> cpuBuffer;
};

class Renderer
{
public:
    Batch lineBatch;      // 所有线、圆、多段线统一用 GL_LINES + Geometry Shader
    GLuint prog = 0;

    Renderer()
    {
        createShader(); createBuffers();
    }
    ~Renderer()
    {
        glDeleteVertexArrays(1, &lineBatch.VAO); glDeleteBuffers(1, &lineBatch.VBO); glDeleteProgram(prog);
    }

    void createShader()
    {
        const char* vs = R"(
#version 330 core
layout(location=0) in vec2 aPos;
layout(location=1) in vec4 aColor;
layout(location=2) in float aThickness;
layout(location=3) in float aDash;
uniform mat4 uMVP;
out vec4 vColor;
out float vThickness;
out float vDash;
void main(){
    gl_Position = uMVP * vec4(aPos,0.0,1.0);
    vColor = aColor;
    vThickness = aThickness;
    vDash = aDash;
}
)";

        const char* gs = R"(
#version 330 core
layout(lines) in;
layout(triangle_strip, max_vertices=16) out;   // 最多 16 顶点（带圆帽+虚线）
in vec4 vColor[]; in float vThickness[]; in float vDash[];
out vec4 fColor; out vec2 fUV;
uniform vec2 uViewportSize;
#define PI 3.14159265359

void main(){
    vec2 pos0 = gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;
    vec2 pos1 = gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;
    vec2 dir = normalize(pos1 - pos0);
    vec2 ndir = vec2(-dir.y, dir.x);
    float thick = vThickness[0] * 0.5;
    vec4 col = vColor[0];

    // NDC → pixel
    vec2 p0 = pos0 * uViewportSize;
    vec2 p1 = pos1 * uViewportSize;
    vec2 pixelDir = normalize(p1 - p0);
    vec2 pixelNdir = vec2(-pixelDir.y, pixelDir.x);
    float pixelThick = thick * (uViewportSize.x * 0.5);

    // 圆帽 + 主体
    vec2 A = p0 - pixelDir * pixelThick;
    vec2 B = p0 + pixelDir * pixelThick;
    vec2 C = p1 + pixelDir * pixelThick;
    vec2 D = p1 - pixelDir * pixelThick;

    // 主体四边形
    gl_Position = vec4((A - pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(0,0); EmitVertex();
    gl_Position = vec4((A + pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(0,1); EmitVertex();
    gl_Position = vec4((B - pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(1,0); EmitVertex();
    gl_Position = vec4((B + pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(1,1); EmitVertex();
    gl_Position = vec4((C - pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(1,0); EmitVertex();
    gl_Position = vec4((C + pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(1,1); EmitVertex();
    gl_Position = vec4((D - pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(0,0); EmitVertex();
    gl_Position = vec4((D + pixelNdir*pixelThick)/uViewportSize, 0.0, 1.0); fColor=col; fUV=vec2(0,1); EmitVertex();
    EndPrimitive();

    // 起点圆帽（半圆）
    for(int i=0; i<=12; ++i){
        float ang = PI * i / 12.0;
        vec2 off = vec2(cos(ang), sin(ang)) * pixelThick;
        gl_Position = vec4((p0 + off)/uViewportSize, 0.0, 1.0); fColor=col; EmitVertex();
    }
    EndPrimitive();

    // 终点圆帽
    for(int i=0; i<=12; ++i){
        float ang = PI + PI * i / 12.0;
        vec2 off = vec2(cos(ang), sin(ang)) * pixelThick;
        gl_Position = vec4((p1 + off)/uViewportSize, 0.0, 1.0); fColor=col; EmitVertex();
    }
    EndPrimitive();
}
)";

        const char* fs = R"(
#version 330 core
in vec4 fColor;
out vec4 FragColor;
void main(){ FragColor = fColor; }
)";

        auto compile = [](const char* src, GLenum type)->GLuint {
            GLuint s = glCreateShader(type);
            glShaderSource(s, 1, &src, nullptr);
            glCompileShader(s);
            GLint ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
            if (!ok)
            {
                char log[2048]; glGetShaderInfoLog(s, 2048, nullptr, log); fprintf(stderr, "%s\n", log);
            }
            return s;
            };
        GLuint v = compile(vs, GL_VERTEX_SHADER), g = compile(gs, GL_GEOMETRY_SHADER), f = compile(fs, GL_FRAGMENT_SHADER);
        prog = glCreateProgram();
        glAttachShader(prog, v); glAttachShader(prog, g); glAttachShader(prog, f);
        glLinkProgram(prog);
        glDeleteShader(v); glDeleteShader(g); glDeleteShader(f);
    }

    void createBuffers()
    {
        glGenVertexArrays(1, &lineBatch.VAO);
        glGenBuffers(1, &lineBatch.VBO);
        glBindVertexArray(lineBatch.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, lineBatch.VBO);
        glBufferData(GL_ARRAY_BUFFER, lineBatch.capacity * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

        glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1); glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(2); glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3); glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(7 * sizeof(float)));
    }

    void render(const mat4& mvp, ivec2 viewportSize, const Box& viewBox)
    {
        auto t0 = chrono::high_resolution_clock::now();

        vector<Value> result;
        g_rtree.query(bgi::intersects(viewBox), back_inserter(result));

        lineBatch.cpuBuffer.clear();
        lineBatch.cpuBuffer.reserve(result.size() * 200); // 预估

        for (auto& v : result)
        {
            size_t idx = v.second;
            g_primitives[idx]->rasterize(lineBatch.cpuBuffer);
        }

        // Orphan + upload
        glBindBuffer(GL_ARRAY_BUFFER, lineBatch.VBO);
        glBufferData(GL_ARRAY_BUFFER, lineBatch.capacity * sizeof(float), nullptr, GL_DYNAMIC_DRAW); // orphan
        glBufferSubData(GL_ARRAY_BUFFER, 0, lineBatch.cpuBuffer.size() * sizeof(float), lineBatch.cpuBuffer.data());

        glViewport(0, 0, viewportSize.x, viewportSize.y);
        glClearColor(0.07f, 0.07f, 0.1f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(prog);
        glUniformMatrix4fv(glGetUniformLocation(prog, "uMVP"), 1, GL_FALSE, value_ptr(mvp));
        glUniform2f(glGetUniformLocation(prog, "uViewportSize"), (float)viewportSize.x, (float)viewportSize.y);

        glBindVertexArray(lineBatch.VAO);
        if (!lineBatch.cpuBuffer.empty())
            glDrawArrays(GL_LINES, 0, (GLsizei)lineBatch.cpuBuffer.size() / 8);

        auto t1 = chrono::high_resolution_clock::now();
        static int frame = 0;
        if (++frame % 60 == 0)
            printf("Visible: %6zu  Vertices: %7zu  Time: %.2f ms\n",
                result.size(), lineBatch.cpuBuffer.size() / 8,
                chrono::duration<double, milli>(t1 - t0).count());
    }
};

// ------------------- 主程序 -------------------
int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(1400, 1000, "2D CAD - 1M primitives + RTree + GeometryShader", nullptr, nullptr);
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    Renderer renderer;

    // ---------- 生成 100 万随机图元 ----------
    mt19937 rng(42);
    uniform_real_distribution<float> dist(-5000, 5000);
    uniform_real_distribution<float> col(0.3f, 1.f);

    auto add = [&](auto* p) {
        g_primitives.push_back(p);
        g_rtree.insert(make_pair(p->aabb, g_primitives.size() - 1));
        };

    //constexpr int N = 10000;
    constexpr int N = 10;
    for (int i = 0; i < 4 * N; ++i) add(new Line({ dist(rng),dist(rng) }, { dist(rng),dist(rng) },
        vec4(col(rng), col(rng), col(rng), 1), 1.5f + 3.f * col(rng)));

    for (int i = 0; i < 3 * N; ++i) add(new Circle({ dist(rng),dist(rng) }, 5 + 80 * col(rng),
        vec4(col(rng), col(rng), col(rng), 1), 2.f));

    for (int i = 0; i < 3 * N; ++i)
    {
        int n = 3 + rng() % 20;
        vector<vec2> pts; pts.reserve(n);
        vec2 p{ dist(rng),dist(rng) };
        for (int j = 0; j < n; ++j)
        {
            pts.push_back(p); p += vec2(dist(rng) * 0.1f, dist(rng) * 0.1f);
        }
        add(new PolylinePrim(pts, vec4(col(rng), col(rng), 0.4f, 1), 2.5f));
    }
    cout << "Generated " << g_primitives.size() << " primitives, RTree built.\n";

    // ---------- 相机 ----------
    vec2 camPos(0, 0);
    float camZoom = 0.05f;   // 初始缩小看全图
    double lastMx = 0, lastMy = 0;
    bool mouseDown = false;

    while (!glfwWindowShouldClose(win))
    {
        int w, h; glfwGetFramebufferSize(win, &w, &h);

        // 鼠标平移缩放
        double mx, my; glfwGetCursorPos(win, &mx, &my);
        if (glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
        {
            if (!mouseDown)
            {
                lastMx = mx; lastMy = my; mouseDown = true;
            }
            camPos.x -= (mx - lastMx) * camZoom * 2;
            camPos.y += (my - lastMy) * camZoom * 2;  // Y翻转
            lastMx = mx; lastMy = my;
        }
        else mouseDown = false;

        //float wheel = glfwGetMouseWheelOffset(win); // 需要自己写个小封装，这里用 glfw 3.3+ 的 poll
        //if (wheel != 0) camZoom *= (wheel > 0) ? 0.8f : 1.25f;

         // ---------- 修复滚轮输入（GLFW 3.3+ 必备）----------
        static float g_scrollY = 0.0f;
        glfwSetScrollCallback(win, [](GLFWwindow*, double, double yoffset) {
            g_scrollY += static_cast<float>(yoffset);
            });

        float wheel = g_scrollY;
        g_scrollY = 0.0f;
        if (wheel != 0.0f)
        {
            camZoom *= (wheel > 0) ? 0.82f : 1.22f;
            camZoom = std::max(1e-7f, camZoom);
        }


        // 投影矩阵：正交 + 视口坐标系
        mat4 proj = ortho(-w * camZoom / 2 + camPos.x, w * camZoom / 2 + camPos.x,
            -h * camZoom / 2 + camPos.y, h * camZoom / 2 + camPos.y,
            -1.f, 1.f);

        // 计算当前视口包围盒（用于RTree查询）
        vec2 tl = camPos + vec2(-w * camZoom / 2, -h * camZoom / 2);
        vec2 br = camPos + vec2(w * camZoom / 2, h * camZoom / 2);
        Box viewBox(Point(tl.x, tl.y), Point(br.x, br.y));

        renderer.render(proj, ivec2(w, h), viewBox);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    // 清理（省略）
    return 0;
}