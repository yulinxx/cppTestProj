#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cmath>
#include <set>
#include <locale>
#include <codecvt>

// 定义Character结构体，用于存储每个字符的位图信息
// - sz: 字符位图的宽度和高度
// - bearing: 从基线到字符左上角的偏移量
// - advance: 到下一个字符的水平前进量（以像素为单位）
// - offset: 字符在纹理图集中的起始位置（x, y）
struct Character
{
    glm::ivec2 sz;    // glyph 大小
    glm::ivec2 bearing; // 从基线到 glyph 左上角的偏移
    GLuint advance;     // 到下一个 glyph 的水平偏移
    glm::ivec2 offset;  // 在图集中的 x0, y0
};

// 全局变量：
// - Characters: 映射Unicode code point到其对应Character结构体的std::map（使用unsigned long作为键）
std::map<unsigned long, Character> mapCharacters;
GLuint nAtlasTexture = 0;
int nAtlasWidth = 0, nAtlasHeight = 0;
glm::vec3 currentTextColor = glm::vec3(1.0f, 1.0f, 0.0f); // 默认色

// 顶点着色器源代码：
// - 输入：vec4 vertex (前两个分量为位置，后两个为纹理坐标)
// - 输出：纹理坐标TexCoords
// - 使用投影矩阵转换位置
const char* vs = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;
uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

// 片段着色器
const char* fs = R"(
#version 330 core
in vec2 TexCoords;  // 纹理坐标（UV），用于指定在纹理图集（text）中采样的位置。
out vec4 color;
uniform sampler2D text; // text 绑定的字形纹理
uniform vec3 textColor;

void main()
{
    float a = texture(text, TexCoords).r; // 使用texture函数在text纹理上根据TexCoords采样。取出单通道纹理的红色分量
    vec4 sampled = vec4(1.0, 1.0, 1.0, a); // 创建一个vec4变量sampled，表示一个白色基底颜色（RGB全为1.0），Alpha通道来自纹理的灰度值a。
    color = vec4(textColor, 1.0) * sampled; // 构造最终输出颜色color。

    //color.r = textColor.r * sampled.r = textColor.r * 1.0 = textColor.r
    //color.g = textColor.g * sampled.g = textColor.g * 1.0 = textColor.g
    //color.b = textColor.b * sampled.b = textColor.b * 1.0 = textColor.b
    //color.a = 1.0 * sampled.a = a
}
)";

// 编译并链接顶点和片段着色器，返回着色器程序ID
GLuint CompileShader()
{
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vs, NULL);
    glCompileShader(vertex);

    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fs, NULL);
    glCompileShader(fragment);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}

// 收集唯一Unicode码点
// 从UTF-8字符串中收集独特的Unicode code points
// 使用std::codecvt将UTF-8转换为UTF-32
// 将输入的std::string（通常包含UTF - 8编码的文本）转换为其包含的唯一Unicode码点（code points）
// 并将这些码点存储在一个std::set<unsigned long>集合中

std::set<unsigned long> CollectUniqueCodePoints(const std::string& text)
{
    std::set<unsigned long> codePoints;
    //std::wstring_convert是C++标准库中的工具，用于在不同字符编码之间转换。
    //std::codecvt_utf8<char32_t>，表示将UTF-8编码的字符串转换为UTF-32编码。
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string u32text = converter.from_bytes(text);
    for (char32_t c : u32text)
        codePoints.insert(static_cast<unsigned long>(c));

    return codePoints;
}

// 加载字体
// 加载指定字体文件，生成包含ASCII字符 + 指定文本中独特Unicode字符（支持中文）的纹理图集
// - fontPath: 字体文件路径（如TTF文件，支持中文字体如simsun.ttc）
// - fontSize: 字体大小（像素）
// - text: 要渲染的文本，用于收集需要加载的Unicode字符
// 使用FreeType栅格化字符位图，并线性打包到纹理图集中
void LoadFont(const std::string& fontPath, int fontSize, const std::string& text)
{
    // 初始化FreeType库
    FT_Library ft;
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // 加载字体文件（确保字体支持中文，如Arial Unicode或中文字体）
    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        std::cerr << "ERROR::FREETYPE: Failed to load font" << std::endl;
        return;
    }

    // 设置字体像素大小
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    // 禁用字节对齐限制，以便正确上传单通道纹理
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // 收集需要加载的字符：ASCII 0-127 + 文本中的独特Unicode
    std::set<unsigned long> charsToLoad; // 存储需要渲染的字符的Unicode码点

    for (unsigned long c = 0; c < 128; ++c)
        charsToLoad.insert(c);

    std::set<unsigned long> unicodeChars = CollectUniqueCodePoints(text);
    charsToLoad.insert(unicodeChars.begin(), unicodeChars.end());

    // 计算纹理图集初始大小（基于字符数和字体大小）,将多个字符的字形（glyphs）打包到一个纹理中

    int nNumGlyphs = static_cast<int>(charsToLoad.size()); // 加载的字符（字形）数量

    // 向上取整计算最大纹理尺寸，比如16个字符，可以排列在一个 4×4 的网格中，并为每个字形增加 1 像素的间距，避免混合，
    // 将网格边长（以字形为单位）乘以每个字形的像素大小，得到图集的最大尺寸（以像素为单位）。
    // 如果 nNumGlyphs = 16，fontSize = 32：sqrt(16) = 4，ceil(4) = 4（4×4 网格）。
    // 则 (1 + 32) * 4 = 33 * 4 = 132。  nMaxDim = 132 像素。

    int nMaxDim = (1 + fontSize) * static_cast<int>(std::ceil(std::sqrt(nNumGlyphs)));

    nAtlasWidth = 1; // 将纹理图集的宽度初始化为 1 像素

    // <<= 1：位左移操作，相当于将 nAtlasWidth 乘以 2。例如，1 变成 2，2 变成 4，4 变成 8，依此类推，直到它大于或等于 nMaxDim
    // 理尺寸通常需要是 2 的幂,便于纹理坐标计算和内存对齐,更高效且兼容性更好,缺点是会浪费空间
    while (nAtlasWidth < nMaxDim) // 通过位运算将 nAtlasWidth 调整为最接近 nMaxDim 的 2 的幂（power of two）
        nAtlasWidth <<= 1;

    nAtlasHeight = nAtlasWidth;

    // 分配图集像素缓冲区，并初始化为0，表示纹理图集的像素缓冲区
    unsigned char* chPixels = new unsigned char[nAtlasWidth * nAtlasHeight];
    std::memset(chPixels, 0, nAtlasWidth * nAtlasHeight);

    // 打包位置：nPenX, nPenY 为当前放置位置；nCurRowHeight 为当前行最大高度
    int nPenX = 0;
    int nPenY = 0;
    int nCurRowHeight = 0;

    // 循环处理每个需要加载的字符
    for (unsigned long c : charsToLoad)
    {
        // 加载字符的glyph位图（使用FT_Load_Char支持Unicode）
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cerr << "WARNING::FREETYTPE: Failed to load Glyph U+" << std::hex << c << std::endl;
            continue;
        }

        FT_Bitmap* bmp = &face->glyph->bitmap;

        // 如果当前行放不下，换行
        if (nPenX + bmp->width + 1 >= nAtlasWidth)
        {
            nPenX = 0;
            nPenY += nCurRowHeight + 1; // 移动到下一行，使用当前行最大高度 + 间距
            nCurRowHeight = 0;

            // 如果高度不足，动态扩展图集高度
            if (nPenY + fontSize + 1 >= nAtlasHeight)
            {
                nAtlasHeight *= 2;
                unsigned char* cNewPixels = new unsigned char[nAtlasWidth * nAtlasHeight];
                if (!cNewPixels)
                {
                    std::cerr << "ERROR: Failed to allocate memory for atlas" << std::endl;
                    return;
                }
                std::memset(cNewPixels, 0, nAtlasWidth * nAtlasHeight);
                std::memcpy(cNewPixels, chPixels, nAtlasWidth * (nAtlasHeight / 2));
                delete[] chPixels;
                chPixels = cNewPixels;
            }
        }

        // 更新当前行高度
        nCurRowHeight = std::max(nCurRowHeight, static_cast<int>(bmp->rows));

        // 将位图数据拷贝到图集缓冲区
        for (int nRow = 0; nRow < bmp->rows; ++nRow)
        {
            for (int nCol = 0; nCol < bmp->width; ++nCol)
            {
                int x = nPenX + nCol;
                int y = nPenY + nRow;
                chPixels[y * nAtlasWidth + x] = bmp->buffer[nRow * bmp->pitch + nCol];
            }
        }

        // 存储字符信息
        Character character = {
            glm::ivec2(bmp->width, bmp->rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x >> 6),
            glm::ivec2(nPenX, nPenY)
        };

        mapCharacters[c] = character;

        // 移动到下一个放置位置（添加间距）
        nPenX += bmp->width + 1;
    }

    // 生成OpenGL纹理对象，并上传像素数据（GL_RED通道）
    glGenTextures(1, &nAtlasTexture);
    glBindTexture(GL_TEXTURE_2D, nAtlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, nAtlasWidth, nAtlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, chPixels);

    // 设置纹理参数：边缘clamp，线性过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 释放缓冲区和FreeType资源
    delete[] chPixels;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

// TextBatch结构体
// - VAO, VBO: 顶点数组和缓冲对象
// - vertexCount: 顶点总数（用于绘制）
struct TextBatch
{
    GLuint VAO, VBO;
    GLsizei vertexCount;
};

// 创建静态文本批次
// 为给定的静态文本生成批处理VBO
// - text: 要渲染的UTF-8字符串（支持中文）
// - x, y: 起始位置
// - scale: 缩放因子
// 从图集索引每个字符的UV，生成四边形顶点数据（位置 + UV）
TextBatch CreateStaticTextBatch(const std::string& text, GLfloat x, GLfloat y, GLfloat scale)
{
    TextBatch batch;
    glGenVertexArrays(1, &batch.VAO);
    glGenBuffers(1, &batch.VBO);
    glBindVertexArray(batch.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch.VBO);

    std::vector<GLfloat> vertices;

    // 将UTF-8转换为UTF-32以处理每个code point
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string u32text = converter.from_bytes(text);

    // 遍历每个Unicode字符
    for (char32_t ch : u32text)
    {
        unsigned long c = static_cast<unsigned long>(ch);

        // 查找字符信息
        auto it = mapCharacters.find(c);
        if (it == mapCharacters.end())
        {
            std::cerr << "WARNING: Character U+" << std::hex << c << " not found in atlas!" << std::endl;
            continue;
        }

        Character charInfo = it->second;

        // 计算字符在屏幕上的位置（考虑bearing偏移）
        GLfloat xpos = x + charInfo.bearing.x * scale;
        GLfloat ypos = y - (charInfo.sz.y - charInfo.bearing.y) * scale;

        // 计算字符四边形的宽度和高度
        GLfloat w = charInfo.sz.x * scale;
        GLfloat h = charInfo.sz.y * scale;

        // 计算纹理UV坐标（u0,v0 为左上，u1,v1 为右下）
        GLfloat u0 = static_cast<GLfloat>(charInfo.offset.x) / nAtlasWidth;
        GLfloat v0 = static_cast<GLfloat>(charInfo.offset.y) / nAtlasHeight;
        GLfloat u1 = static_cast<GLfloat>(charInfo.offset.x + charInfo.sz.x) / nAtlasWidth;
        GLfloat v1 = static_cast<GLfloat>(charInfo.offset.y + charInfo.sz.y) / nAtlasHeight;

        // 定义6个顶点（两个三角形组成四边形）
        GLfloat quadVertices[6][4] = {
            { xpos,     ypos + h,   u0, v0 },
            { xpos,     ypos,       u0, v1 },
            { xpos + w, ypos,       u1, v1 },
            { xpos,     ypos + h,   u0, v0 },
            { xpos + w, ypos,       u1, v1 },
            { xpos + w, ypos + h,   u1, v0 }
        };

        // 添加到 vertices vector
        for (int i = 0; i < 6; ++i)
        {
            vertices.push_back(quadVertices[i][0]);
            vertices.push_back(quadVertices[i][1]);
            vertices.push_back(quadVertices[i][2]);
            vertices.push_back(quadVertices[i][3]);
        }

        // 更新下一个字符的起始x位置
        x += charInfo.advance * scale;
    }

    // 上传顶点数据到VBO（静态绘制）
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    // 配置顶点属性（位置0: 4 floats，位置+UV）
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // 计算顶点总数
    batch.vertexCount = static_cast<GLsizei>(vertices.size() / 4);

    return batch;
}

// 渲染静态文本批次
// - batch: TextBatch对象
// - shader: 着色器程序ID
// - projection: 投影矩阵
// - color: 文本颜色
// 绑定纹理和VAO，通过一次绘制调用渲染所有字符
void RenderStaticText(const TextBatch& batch, GLuint shader, glm::mat4 projection, glm::vec3 color)
{
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, nAtlasTexture);
    glBindVertexArray(batch.VAO);
    glDrawArrays(GL_TRIANGLES, 0, batch.vertexCount);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// 键盘回调函数：处理颜色切换
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action == GLFW_PRESS)
    {
        switch (key)
        {
        case GLFW_KEY_R: // 红色
            currentTextColor = glm::vec3(1.0f, 0.0f, 0.0f);
            break;
        case GLFW_KEY_G: // 绿色
            currentTextColor = glm::vec3(0.0f, 1.0f, 0.0f);
            break;
        case GLFW_KEY_B: // 蓝色
            currentTextColor = glm::vec3(0.0f, 0.0f, 1.0f);
            break;
        case GLFW_KEY_W: // 白色
            currentTextColor = glm::vec3(1.0f, 1.0f, 1.0f);
            break;
        }
    }
}

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "OpenGL Text Atlas Rendering with Chinese", NULL, NULL);
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

    // 启用混合模式（用于透明文本）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 设置键盘回调
    glfwSetKeyCallback(window, key_callback);

    GLuint shader = CompileShader();
    std::string chineseText = "你好，世界！。！イベント体験~한국말韓國말-Hello World~";
    LoadFont("C:/Windows/Fonts/STCAIYUN.TTF", 68, chineseText);

    // 设置正交投影矩阵（2D渲染）
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
    TextBatch helloBatch = CreateStaticTextBatch(chineseText, 25.0f, 500.0f, 0.5f);

    // 进入主循环
    while (!glfwWindowShouldClose(window))
    {
        // 清屏
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用当前颜色渲染文本
        RenderStaticText(helloBatch, shader, projection, currentTextColor);

        // 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteTextures(1, &nAtlasTexture);
    glDeleteVertexArrays(1, &helloBatch.VAO);
    glDeleteBuffers(1, &helloBatch.VBO);
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}