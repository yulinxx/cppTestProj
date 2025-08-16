// 包含必要的头文件：OpenGL加载器、窗口管理、数学库、FreeType字体库等
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
#include <set> // 新增：用于收集独特字符
#include <locale> // 新增：用于wchar转换
#include <codecvt> // 新增：用于UTF-8到UTF-32转换

// 定义Character结构体，用于存储每个字符的位图信息
// - Size: 字符位图的宽度和高度
// - Bearing: 从基线到字符左上角的偏移量
// - Advance: 到下一个字符的水平前进量（以像素为单位）
// - Offset: 字符在纹理图集中的起始位置（x, y）
struct Character
{
    glm::ivec2 Size;    // glyph 大小
    glm::ivec2 Bearing; // 从基线到 glyph 左上角的偏移
    GLuint Advance;     // 到下一个 glyph 的水平偏移
    glm::ivec2 Offset;  // 在图集中的 x0, y0
};

// 全局变量：
// - Characters: 映射Unicode code point到其对应Character结构体的std::map（使用unsigned long作为键）
std::map<unsigned long, Character> Characters;
GLuint atlasTexture = 0;
int atlasWidth = 0, atlasHeight = 0;

// 顶点着色器源代码：
// - 输入：vec4 vertex (前两个分量为位置，后两个为纹理坐标)
// - 输出：纹理坐标TexCoords
// - 使用投影矩阵转换位置
const char* vertexShaderSource = R"(
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

// 片段着色器源代码：
// - 输入：纹理坐标TexCoords
// - 输出：最终颜色color
// - 从纹理采样红色通道作为alpha，乘以指定的文本颜色
const char* fragmentShaderSource = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    color = vec4(textColor, 1.0) * sampled;
}
)";

// 编译并链接顶点和片段着色器，返回着色器程序ID
GLuint CompileShader()
{
    // 创建并编译顶点着色器
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexShaderSource, NULL);
    glCompileShader(vertex);

    // 创建并编译片段着色器
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragment);

    // 创建程序对象，附加着色器并链接
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    // 删除不再需要的着色器对象
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return program;
}

// CollectUniqueCodePoints
// 从UTF-8字符串中收集独特的Unicode code points
// 使用std::codecvt将UTF-8转换为UTF-32
std::set<unsigned long> CollectUniqueCodePoints(const std::string& text)
{
    std::set<unsigned long> codePoints;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string u32text = converter.from_bytes(text);
    for (char32_t c : u32text)
    {
        codePoints.insert(static_cast<unsigned long>(c));
    }
    return codePoints;
}

// 函数：LoadFont
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
    std::set<unsigned long> charsToLoad;
    for (unsigned long c = 0; c < 128; ++c)
    {
        charsToLoad.insert(c);
    }
    std::set<unsigned long> unicodeChars = CollectUniqueCodePoints(text);
    charsToLoad.insert(unicodeChars.begin(), unicodeChars.end());

    // 计算纹理图集初始大小（基于字符数和字体大小）
    int numGlyphs = static_cast<int>(charsToLoad.size());
    int max_dim = (1 + fontSize) * static_cast<int>(std::ceil(std::sqrt(numGlyphs)));
    atlasWidth = 1;
    while (atlasWidth < max_dim) atlasWidth <<= 1;
    atlasHeight = atlasWidth;

    // 分配图集像素缓冲区，并初始化为0
    unsigned char* pixels = new unsigned char[atlasWidth * atlasHeight];
    std::memset(pixels, 0, atlasWidth * atlasHeight);

    // 打包位置：pen_x, pen_y 为当前放置位置；row_height 为当前行最大高度
    int pen_x = 0, pen_y = 0;
    int row_height = 0;

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
        if (pen_x + bmp->width + 1 >= atlasWidth)
        {
            pen_x = 0;
            pen_y += row_height + 1; // 移动到下一行，使用当前行最大高度 + 间距
            row_height = 0;
            // 如果高度不足，动态扩展图集高度
            if (pen_y + fontSize + 1 >= atlasHeight)
            {
                atlasHeight *= 2;
                unsigned char* new_pixels = new unsigned char[atlasWidth * atlasHeight];
                std::memset(new_pixels, 0, atlasWidth * atlasHeight);
                std::memcpy(new_pixels, pixels, atlasWidth * (atlasHeight / 2));
                delete[] pixels;
                pixels = new_pixels;
            }
        }

        // 更新当前行高度
        row_height = std::max(row_height, static_cast<int>(bmp->rows));

        // 将位图数据拷贝到图集缓冲区
        for (int row = 0; row < bmp->rows; ++row)
        {
            for (int col = 0; col < bmp->width; ++col)
            {
                int x = pen_x + col;
                int y = pen_y + row;
                pixels[y * atlasWidth + x] = bmp->buffer[row * bmp->pitch + col];
            }
        }

        // 存储字符信息
        Character character = {
            glm::ivec2(bmp->width, bmp->rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<GLuint>(face->glyph->advance.x >> 6),
            glm::ivec2(pen_x, pen_y)
        };
        Characters[c] = character;

        // 移动到下一个放置位置（添加间距）
        pen_x += bmp->width + 1;
    }

    // 生成OpenGL纹理对象，并上传像素数据（GL_RED通道）
    glGenTextures(1, &atlasTexture);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);
    // 设置纹理参数：边缘clamp，线性过滤
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    // 释放缓冲区和FreeType资源
    delete[] pixels;
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

// 定义TextBatch结构体，用于存储静态文本的批处理数据
// - VAO, VBO: 顶点数组和缓冲对象
// - vertexCount: 顶点总数（用于绘制）
struct TextBatch
{
    GLuint VAO, VBO;
    GLsizei vertexCount; // 顶点数
};

// 函数：CreateStaticTextBatch
// 为给定的静态文本生成批处理VBO
// - text: 要渲染的UTF-8字符串（支持中文）
// - x, y: 起始位置
// - scale: 缩放因子
// 从图集索引每个字符的UV，生成四边形顶点数据（位置 + UV）
TextBatch CreateStaticTextBatch(const std::string& text, GLfloat x, GLfloat y, GLfloat scale)
{
    TextBatch batch;
    // 生成VAO和VBO
    glGenVertexArrays(1, &batch.VAO);
    glGenBuffers(1, &batch.VBO);
    glBindVertexArray(batch.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, batch.VBO);

    // 收集顶点数据的vector
    std::vector<GLfloat> vertices;

    // 将UTF-8转换为UTF-32以处理每个code point
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    std::u32string u32text = converter.from_bytes(text);

    // 遍历每个Unicode字符
    for (char32_t ch : u32text)
    {
        unsigned long c = static_cast<unsigned long>(ch);
        // 查找字符信息
        auto it = Characters.find(c);
        if (it == Characters.end())
        {
            std::cerr << "WARNING: Character U+" << std::hex << c << " not found in atlas!" << std::endl;
            continue;
        }
        Character charInfo = it->second;

        // 计算字符在屏幕上的位置（考虑bearing偏移）
        GLfloat xpos = x + charInfo.Bearing.x * scale;
        GLfloat ypos = y - (charInfo.Size.y - charInfo.Bearing.y) * scale;

        // 计算字符四边形的宽度和高度
        GLfloat w = charInfo.Size.x * scale;
        GLfloat h = charInfo.Size.y * scale;

        // 计算纹理UV坐标（u0,v0 为左上，u1,v1 为右下）
        GLfloat u0 = static_cast<GLfloat>(charInfo.Offset.x) / atlasWidth;
        GLfloat v0 = static_cast<GLfloat>(charInfo.Offset.y) / atlasHeight;
        GLfloat u1 = static_cast<GLfloat>(charInfo.Offset.x + charInfo.Size.x) / atlasWidth;
        GLfloat v1 = static_cast<GLfloat>(charInfo.Offset.y + charInfo.Size.y) / atlasHeight;

        // 定义6个顶点（两个三角形组成四边形）
        GLfloat quadVertices[6][4] = {
            { xpos,     ypos + h,   u0, v0 },
            { xpos,     ypos,       u0, v1 },
            { xpos + w, ypos,       u1, v1 },

            { xpos,     ypos + h,   u0, v0 },
            { xpos + w, ypos,       u1, v1 },
            { xpos + w, ypos + h,   u1, v0 }
        };
        // 添加到vertices vector
        for (int i = 0; i < 6; ++i)
        {
            vertices.push_back(quadVertices[i][0]);
            vertices.push_back(quadVertices[i][1]);
            vertices.push_back(quadVertices[i][2]);
            vertices.push_back(quadVertices[i][3]);
        }

        // 更新下一个字符的起始x位置
        x += charInfo.Advance * scale;
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

// 函数：RenderStaticText
// 渲染静态文本批次
// - batch: TextBatch对象
// - shader: 着色器程序ID
// - projection: 投影矩阵
// - color: 文本颜色
// 绑定纹理和VAO，通过一次绘制调用渲染所有字符
void RenderStaticText(const TextBatch& batch, GLuint shader, glm::mat4 projection, glm::vec3 color)
{
    // 使用着色器程序
    glUseProgram(shader);
    // 设置统一变量：文本颜色和投影矩阵
    glUniform3f(glGetUniformLocation(shader, "textColor"), color.x, color.y, color.z);
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    // 激活纹理单元0，并绑定图集纹理
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, atlasTexture);
    // 绑定VAO
    glBindVertexArray(batch.VAO);

    // 绘制所有顶点（三角形模式）
    glDrawArrays(GL_TRIANGLES, 0, batch.vertexCount);

    // 解绑VAO和纹理
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// 主函数：程序入口
// 初始化OpenGL窗口，加载字体，创建文本批次，并进入渲染循环
int main()
{
    glfwInit();
    // 设置OpenGL上下文版本（3.3核心配置文件）
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Text Atlas Rendering with Chinese", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    // 启用混合模式（用于透明文本）
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 编译着色器
    GLuint shader = CompileShader();

    // 要渲染的中文文本（UTF-8编码）
    std::string chineseText = "你好，世界！\nイベント体験~한국말韓國말-Hello World~";

    // 加载字体并生成纹理图集（使用支持中文的字体，如simsun.ttc或arialuni.ttf）
    LoadFont("C:/Windows/Fonts/STCAIYUN.TTF", 68, chineseText);

    // 设置正交投影矩阵（2D渲染）
    glm::mat4 projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);

    // 创建静态文本批次（渲染中文文本）
    TextBatch helloBatch = CreateStaticTextBatch(chineseText, 25.0f, 500.0f, 0.5f);

    // 进入主循环
    while (!glfwWindowShouldClose(window))
    {
        // 清屏
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 渲染文本
        RenderStaticText(helloBatch, shader, projection, glm::vec3(1.0f, 1.0f, 1.0f));

        // 交换缓冲区并处理事件
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    glDeleteTextures(1, &atlasTexture);
    glfwTerminate();
    return 0;
}