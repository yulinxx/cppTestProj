#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cwchar>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H 

#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 800

const char* vs = R"(
#version 400
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
// 主函数,每个顶点都会执行此函数
void main()
{
   // 通过投影矩阵变换顶点位置,得到裁剪空间坐标
   gl_Position = projection * vec4(aPos, 0.0, 1.0);
}
)";

const char* fs = R"(
#version 400
out vec4 FragColor;
// 主函数,每个片段都会执行此函数
void main()
{
   // 设置片段颜色为白色,RGBA 各通道值均为 1.0
   FragColor = vec4(1.0, 1.0, 1.0, 1.0); // 白色线条
}
)";

/**
 * @brief 字体数据结构,用于存储字体的OpenGL资源和对齐数据
 */
struct FontData
{
    GLuint vao;         // 顶点数组对象ID,用于OpenGL绑定顶点数据
    GLuint vbo;        // 顶点缓冲对象ID,用于存储顶点数据到GPU
    std::vector<float> vertices;      // 所有字符的顶点坐标(x,y)序列
    std::vector<int> contourCounts;   // 每个轮廓的顶点数,用于绘制时区分不同字符
} font;

/**
 * @brief FT_Outline_Decompose回调函数的上下文结构体
 *        用于在轮廓分解过程中传递和保存状态信息
 */
struct DecomposeContext
{
    std::vector<float>* globalVerts;
    std::vector<int>* contourCounts;
    std::vector<float> curContour;       // 当前正在处理的轮廓的顶点数据
    float scale;                         // 缩放因子,FreeType坐标到像素坐标的转换比例(1/64)
    float xOffset;                       // 字符的X轴偏移量(像素坐标)
    float yOffset;                       // 字符的Y轴偏移量(像素坐标)
    FT_Vector last;                       // 上一个处理过的轮廓点坐标
    bool italic;
    float italicSkew;
};

/**
 * @brief FT_Outline_Decompose的移动到回调函数
 *        当FreeType开始一个新的轮廓时调用此函数
 * @param to 目标位置的坐标
 * @param user 用户数据指针,指向DecomposeContext结构体
 * @return 返回0表示成功
 */
int moveTo(const FT_Vector* to, void* user)
{
    DecomposeContext* ctx = (DecomposeContext*)user;
    // 如果当前轮廓不为空,说明前一个轮廓已处理完毕
    // 将前一个轮廓的顶点数添加到计数向量
    if (!ctx->curContour.empty())
    {
        // 每个顶点包含x和y两个坐标,所以顶点数要除以2
        ctx->contourCounts->push_back(ctx->curContour.size() / 2);
        // 将当前轮廓的所有顶点追加到全局顶点向量
        ctx->globalVerts->insert(ctx->globalVerts->end(),
            ctx->curContour.begin(), ctx->curContour.end());
        // 清空当前轮廓,为下一个轮廓做准备
        ctx->curContour.clear();
    }
    // 更新last为当前目标点,作为下一个线段的起点
    ctx->last = *to;
    return 0;
}

/**
 * @brief FT_Outline_Decompose的直线段回调函数
 *        当FreeType遇到直线段时调用此函数
 * @param to 直线段终点的坐标
 * @param user 用户数据指针,指向DecomposeContext结构体
 * @return 返回0表示成功
 */
int lineTo(const FT_Vector* to, void* user)
{
    DecomposeContext* ctx = (DecomposeContext*)user;

    float x1 = ctx->xOffset + ctx->last.x * ctx->scale;
    float y1 = ctx->yOffset + ctx->last.y * ctx->scale;
    if (ctx->italic)
    {
        x1 += y1 * ctx->italicSkew;
    }

    float x2 = ctx->xOffset + to->x * ctx->scale;
    float y2 = ctx->yOffset + to->y * ctx->scale;
    if (ctx->italic)
    {
        x2 += y2 * ctx->italicSkew;
    }

    ctx->curContour.push_back(x1);
    ctx->curContour.push_back(y1);
    ctx->curContour.push_back(x2);
    ctx->curContour.push_back(y2);
    // 更新last为当前点,作为下一段的起点
    ctx->last = *to;
    return 0;
}

/**
 * @brief FT_Outline_Decompose的二次贝塞尔曲线回调函数
 *        当FreeType遇到二次曲线(抛物线)时调用此函数
 * @param control 二次贝塞尔曲线的控制点坐标
 * @param to 曲线终点的坐标
 * @param user 用户数据指针,指向DecomposeContext结构体
 * @return 返回0表示成功
 */
int conicTo(const FT_Vector* control, const FT_Vector* to, void* user)
{
    DecomposeContext* ctx = (DecomposeContext*)user;
    // 曲线控制点
    FT_Vector p0 = ctx->last, p1 = *control, p2 = *to;
    // 将二次贝塞尔曲线细分为多个直线段来近似表示
    int segs = 16;  // 细分数,值越大曲线越平滑
    for (int i = 0; i <= segs; ++i)
    {
        float t = (float)i / segs, u = 1.0f - t;
        // 二次贝塞尔曲线公式: B(t) = (1-t)²P0 + 2(1-t)tP1 + t²P2

        float x = u * u * p0.x + 2 * u * t * p1.x + t * t * p2.x;
        float y = u * u * p0.y + 2 * u * t * p1.y + t * t * p2.y;

        float finalX = ctx->xOffset + x * ctx->scale;
        float finalY = ctx->yOffset + y * ctx->scale;
        if (ctx->italic)
        {
            finalX += finalY * ctx->italicSkew;
        }

        ctx->curContour.push_back(finalX);
        ctx->curContour.push_back(finalY);
    }
    ctx->last = *to;
    return 0;
}

/**
 * @brief FT_Outline_Decompose的三次贝塞尔曲线回调函数
 *        当FreeType遇到三次曲线时调用此函数
 *        此处简化为直线段近似,以保证兼容性
 * @param c1 三次贝塞尔曲线的第一个控制点
 * @param c2 三次贝塞尔曲线的第二个控制点
 * @param to 曲线终点的坐标
 * @param user 用户数据指针,指向DecomposeContext结构体
 * @return 返回0表示成功
 */
int cubicTo(const FT_Vector* c1, const FT_Vector* c2, const FT_Vector* to, void* user)
{
    // 简单用直线近似三次曲线(可扩展为更精细的曲线细分算法)
    DecomposeContext* ctx = (DecomposeContext*)user;

    float x1 = ctx->xOffset + ctx->last.x * ctx->scale;
    float y1 = ctx->yOffset + ctx->last.y * ctx->scale;
    if (ctx->italic)
    {
        x1 += y1 * ctx->italicSkew;
    }

    float x2 = ctx->xOffset + to->x * ctx->scale;
    float y2 = ctx->yOffset + to->y * ctx->scale;
    if (ctx->italic)
    {
        x2 += y2 * ctx->italicSkew;
    }

    ctx->curContour.push_back(x1);
    ctx->curContour.push_back(y1);
    ctx->curContour.push_back(x2);
    ctx->curContour.push_back(y2);
    ctx->last = *to;
    return 0;
}

/**
 * @brief 添加矩形轮廓(用于加粗或下划线)
 * @param x1 左上角x
 * @param y1 左上角y
 * @param x2 右下角x
 * @param y2 右下角y
 * @param vertices 目标顶点向量
 * @param contourCounts 轮廓计数向量
 */
void addRectangle(float x1, float y1, float x2, float y2,
    std::vector<float>& vertices, std::vector<int>& contourCounts)
{
    std::vector<float> rect;
    rect.push_back(x1); rect.push_back(y1);
    rect.push_back(x2); rect.push_back(y1);
    rect.push_back(x2); rect.push_back(y2);
    rect.push_back(x1); rect.push_back(y2);
    rect.push_back(x1); rect.push_back(y1);

    contourCounts.push_back(rect.size() / 2);
    vertices.insert(vertices.end(), rect.begin(), rect.end());
}

/**
 * @brief 添加加粗轮廓(通过扩展字符轮廓)
 * @param originalVerts 原始顶点
 * @param scale 缩放因子
 * @param offset 偏移量
 * @param boldWidth 加粗宽度
 * @param vertices 目标顶点向量
 * @param contourCounts 轮廓计数向量
 */
void addBoldOutline(const std::vector<float>& originalVerts, float scale,
    float offset, float boldWidth,
    std::vector<float>& vertices, std::vector<int>& contourCounts)
{
    if (originalVerts.empty()) return;

    std::vector<float> boldVerts;
    int numPoints = originalVerts.size() / 2;

    for (int i = 0; i < numPoints; i++)
    {
        float x = originalVerts[i * 2];
        float y = originalVerts[i * 2 + 1];

        boldVerts.push_back(x + boldWidth);
        boldVerts.push_back(y + boldWidth);
    }

    for (int i = 0; i < numPoints; i++)
    {
        float x = originalVerts[i * 2];
        float y = originalVerts[i * 2 + 1];

        boldVerts.push_back(x - boldWidth);
        boldVerts.push_back(y - boldWidth);
    }

    for (int i = 0; i < numPoints; i++)
    {
        float x = originalVerts[i * 2];
        float y = originalVerts[i * 2 + 1];

        boldVerts.push_back(x + boldWidth);
        boldVerts.push_back(y - boldWidth);
    }

    for (int i = 0; i < numPoints; i++)
    {
        float x = originalVerts[i * 2];
        float y = originalVerts[i * 2 + 1];

        boldVerts.push_back(x - boldWidth);
        boldVerts.push_back(y + boldWidth);
    }

    contourCounts.push_back(boldVerts.size() / 2);
    vertices.insert(vertices.end(), boldVerts.begin(), boldVerts.end());
}

void initFont(const char* fontPath, const char32_t* text, float fontSize,
    float x, float y, bool bold, bool underline, float lineWidth, bool italic, float italicSkew)
{
    // 初始化FreeType库
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "FT init failed\n"; std::exit(1);
    }

    // 从文件加载字体Face
    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face))
    {
        std::cerr << "Load font failed: " << fontPath << "\n"; std::exit(1);
    }

    // 设置字体大小,width设为0表示使用与height相同的宽高比
    FT_Set_Pixel_Sizes(face, 0, (FT_UInt)fontSize);

    // FreeType使用1/64像素作为基本单位,需要转换到像素坐标
    float scale = 1.0f / 64.0f;
    // 字符起始位置(像素坐标)
    float xpos = x, ypos = y;

    // 配置轮廓分解回调函数
    FT_Outline_Funcs funcs;
    funcs.move_to = moveTo;      // 新轮廓开始时调用
    funcs.line_to = lineTo;     // 直线段处理
    funcs.conic_to = conicTo;   // 二次曲线处理
    funcs.cubic_to = cubicTo;   // 三次曲线处理
    funcs.shift = 0;            // 坐标位移(无位移)
    funcs.delta = 0;            // 坐标增量(无增量)

    // 遍历每个字符
    while (*text)
    {
        // 获取当前字符的UTF-32编码,并移动到下一个字符
        FT_ULong char_code = *text++;
        std::cout << "Processing char_code: 0x" << std::hex << char_code << std::dec << std::endl;

        // 根据字符编码查找对应的字形索引
        FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);
        // 如果找不到对应字形,跳过此字符
        if (glyph_index == 0)
        {
            std::cerr << "Glyph not found for 0x" << std::hex << char_code << std::dec << std::endl;
            // 移动到下一个字符的位置
            xpos += face->glyph->advance.x * scale;
            continue;
        }

        // 加载字形(不使用位图,只获取轮廓)
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_BITMAP))
        {
            std::cerr << "Load glyph failed\n";
            xpos += face->glyph->advance.x * scale;
            continue;
        }

        // 获取字形的轮廓数据
        FT_Outline& outline = face->glyph->outline;
        // 如果轮廓没有点,说明是空字符,直接跳过
        if (outline.n_points <= 0)
        {
            xpos += face->glyph->advance.x * scale;
            continue;
        }

        // 初始化分解上下文
        DecomposeContext ctx;
        ctx.globalVerts = &font.vertices;      // 全局顶点存储
        ctx.contourCounts = &font.contourCounts; // 轮廓计数存储
        ctx.scale = scale;                      // 缩放因子
        ctx.xOffset = xpos;                    // 当前字符的X偏移
        ctx.yOffset = ypos;                    // 当前字符的Y偏移
        ctx.last = FT_Vector();                 // 初始化起始点
        ctx.italic = italic;
        ctx.italicSkew = italicSkew;

        // 执行轮廓分解,将曲线转换为直线段顶点
        FT_Outline_Decompose(&outline, &funcs, &ctx);

        // 处理最后一个轮廓(如果还有未保存的数据)
        if (!ctx.curContour.empty())
        {
            font.contourCounts.push_back(ctx.curContour.size() / 2);
            font.vertices.insert(font.vertices.end(),
                ctx.curContour.begin(), ctx.curContour.end());
        }

        // 更新下一个字符的X位置(字形前进量)
        xpos += face->glyph->advance.x * scale;
    }

    if (bold && lineWidth > 0)
    {
        float xposBold = x;
        const char32_t* textStart = text;
        while (*textStart)
        {
            textStart++;
        }

        const char32_t* textPtr = text;
        while (*textPtr)
        {
            FT_ULong char_code = *textPtr++;
            FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);
            if (glyph_index == 0)
            {
                xposBold += face->glyph->advance.x * scale;
                continue;
            }

            if (FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_BITMAP))
            {
                xposBold += face->glyph->advance.x * scale;
                continue;
            }

            FT_Outline& outline = face->glyph->outline;
            if (outline.n_points <= 0)
            {
                xposBold += face->glyph->advance.x * scale;
                continue;
            }

            float charWidth = face->glyph->metrics.width * scale;
            float charHeight = face->glyph->metrics.height * scale;
            float charTop = ypos - face->glyph->metrics.horiBearingY * scale;
            float charBottom = charTop + charHeight;
            float charLeft = xposBold + face->glyph->metrics.horiBearingX * scale;

            addRectangle(charLeft, charBottom, charLeft + lineWidth, charBottom + lineWidth * 3,
                font.vertices, font.contourCounts);
            addRectangle(charLeft + charWidth - lineWidth, charBottom,
                charLeft + charWidth, charBottom + lineWidth * 3,
                font.vertices, font.contourCounts);

            xposBold += face->glyph->advance.x * scale;
        }
    }

    if (underline)
    {
        float underlineY = ypos + fontSize * 0.1f;
        float underlineHeight = fontSize * 0.05f;
        addRectangle(x, underlineY, xpos, underlineY + underlineHeight,
            font.vertices, font.contourCounts);
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    glGenVertexArrays(1, &font.vao);
    glGenBuffers(1, &font.vbo);
    glBindVertexArray(font.vao);
    glBindBuffer(GL_ARRAY_BUFFER, font.vbo);
    glBufferData(GL_ARRAY_BUFFER, font.vertices.size() * sizeof(float),
        font.vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

/**
 * @brief 渲染文字函数
 * @param program 着色器程序对象
 */
void renderText(GLuint program)
{
    // 激活着色器程序
    glUseProgram(program);

    // 创建正交投影矩阵,将窗口坐标映射到裁剪空间(-1到1)
    // 变换公式: clipX = (windowX / width) * 2 - 1
    float proj[16] = {
        2.0f / WINDOW_WIDTH, 0.0f, 0.0f, 0.0f,
        0.0f, 2.0f / WINDOW_HEIGHT, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 1.0f
    };
    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, proj);
    glBindVertexArray(font.vao);

    // 遍历每个轮廓,分别绘制
    // 使用GL_LINE_LOOP绘制闭合的轮廓线
    int offset = 0;  // 顶点偏移量
    for (int count : font.contourCounts)
    {
        glDrawArrays(GL_LINE_LOOP, offset, count);
        offset += count;
    }
}

int main()
{
    if (!glfwInit())
        return -1;

    // 设置OpenGL上下文版本(主版本4.0,次版本0)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Outline Text - Font Styles", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // 设置当前OpenGL上下文
    glfwMakeContextCurrent(window);
    // 初始化GLAD(OpenGL函数指针加载器)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) return -1;

    // 创建和编译顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);

    // 创建和编译片段着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);

    // 创建着色器程序并链接着色器
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // 删除已编译的着色器对象(不再需要)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 初始化字体,解析文字生成顶点数据
    // (const char* fontPath, text, fontSize, float x, float y, bool bold, bool underline, lineWidth, bool italic, float italicSkew)
    initFont("C:/Windows/Fonts/simhei.ttf", U"中文ABC+abc### 常规 Normal", 48.0f, 50.0f, 150.0f, false, false, 0.0f, false, 0.0f);
    initFont("C:/Windows/Fonts/simhei.ttf", U"中文ABC+abc### 斜体 Italic", 48.0f, 50.0f, 220.0f, false, false, 0.0f, true, 0.2f);
    initFont("C:/Windows/Fonts/simhei.ttf", U"中文ABC+abc### 下划线 Underline", 48.0f, 50.0f, 290.0f, false, true, 0.0f, false, 0.0f);
    initFont("C:/Windows/Fonts/simhei.ttf", U"中文ABC+abc### 加粗 Bold", 48.0f, 50.0f, 360.0f, true, false, 3.0f, false, 0.0f);
    initFont("C:/Windows/Fonts/simhei.ttf", U"中文ABC+abc### 加粗下划线 Bold Underline", 48.0f, 50.0f, 430.0f, true, true, 3.0f, false, 0.0f);
    initFont("C:/Windows/Fonts/simhei.ttf", U"中文ABC+abc### 斜粗体 Bold Italic", 48.0f, 50.0f, 500.0f, true, false, 3.0f, true, 0.2f);


    while (!glfwWindowShouldClose(window))
    {
        // 清除颜色缓冲区
        glClear(GL_COLOR_BUFFER_BIT);
        // 渲染文字
        renderText(shaderProgram);
        // 交换前后缓冲区(双缓冲)
        glfwSwapBuffers(window);
        // 处理窗口事件(按键、鼠标等)
        glfwPollEvents();
    }

    // 清理OpenGL资源
    glDeleteVertexArrays(1, &font.vao);
    glDeleteBuffers(1, &font.vbo);
    // 终止GLFW
    glfwTerminate();
    return 0;
}
