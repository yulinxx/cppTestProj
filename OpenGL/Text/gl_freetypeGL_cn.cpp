#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

#include <freetype-gl/freetype-gl.h>
#include <freetype-gl/vertex-buffer.h>
#include <freetype-gl/texture-atlas.h>
#include <freetype-gl/texture-font.h>

// 窗口尺寸
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// 着色器源码
const char* vertex_shader_src =
"#version 400\n"
"in vec2 position;\n"
"in vec2 texcoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    TexCoord = texcoord;\n"
"}\n";

const char* fragment_shader_src =
"#version 400\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D tex;\n"
"uniform vec3 color;\n"
"void main() {\n"
"    float alpha = texture(tex, TexCoord).r;\n"
"    FragColor = vec4(color, alpha);\n"
"}\n";

// 检查着色器编译错误
void check_shader_error(GLuint shader)
{
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation error: %s\n", infoLog);
    }
}

// 将像素坐标转换为 OpenGL 标准化设备坐标 (NDC)
void pixel_to_ndc(float* x, float* y, int width, int height)
{
    *x = (*x / (float)width) * 2.0f - 1.0f;
    *y = 1.0f - (*y / (float)height) * 2.0f;
}

// 获取 UTF-8 字符的字节长度
int utf8_char_length(unsigned char c)
{
    if (c < 0x80) return 1;        // 单字节 (ASCII)
    if ((c & 0xE0) == 0xC0) return 2; // 双字节
    if ((c & 0xF0) == 0xE0) return 3; // 三字节
    if ((c & 0xF8) == 0xF0) return 4; // 四字节
    return 1; // 默认按单字节处理
}

int main()
{
    // 初始化 GLFW
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // 创建窗口
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Draw Chinese with FreeType-GL", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化 GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    // 初始化 freetype-gl
    texture_atlas_t* atlas = texture_atlas_new(512, 512, 1); // 创建纹理图集
    texture_font_t* font = texture_font_new_from_file(atlas, 48, "C:/Windows/Fonts/arial.ttf"); // 替换为你的字体路径
    if (!font)
    {
        fprintf(stderr, "Failed to load font\n");
        return -1;
    }

    // 要渲染的中文文本(UTF-8 编码)
    //const char *text = "你好,世界！";
    const char* text = "Hello,World";
    vertex_buffer_t* buffer = vertex_buffer_new("position:2f,texcoord:2f");

    // 手动布局和填充顶点缓冲区
    float x = 10.0f, y = WINDOW_HEIGHT - 50.0f; // 起始位置(像素坐标)
    for (int i = 0; text[i]; )
    {
        int char_len = utf8_char_length((unsigned char)text[i]);
        char utf8_char[5] = { 0 }; // 最多 4 字节 + 结束符
        for (int j = 0; j < char_len && text[i + j]; j++)
        {
            utf8_char[j] = text[i + j];
        }

        texture_glyph_t* glyph = texture_font_get_glyph(font, utf8_char);
        if (!glyph)
        {
            i += char_len;
            continue;
        }

        float x0 = x + glyph->offset_x;
        float y0 = y - glyph->offset_y;
        float x1 = x0 + glyph->width;
        float y1 = y0 + glyph->height;

        float s0 = glyph->s0;
        float t0 = glyph->t0;
        float s1 = glyph->s1;
        float t1 = glyph->t1;

        // 转换为 NDC 坐标
        float vertices[4][4] = {
            {x0, y0, s0, t0}, // 左下
            {x1, y0, s1, t0}, // 右下
            {x1, y1, s1, t1}, // 右上
            {x0, y1, s0, t1}  // 左上
        };

        for (int j = 0; j < 4; j++)
        {
            pixel_to_ndc(&vertices[j][0], &vertices[j][1], WINDOW_WIDTH, WINDOW_HEIGHT);
        }

        // 添加顶点和索引
        GLuint indices[6] = { 0, 1, 2, 0, 2, 3 };
        vertex_buffer_push_back(buffer, vertices, 4, indices, 6);

        x += glyph->advance_x; // 更新 x 位置
        y += glyph->advance_y; // 更新 y 位置(通常为 0)
        i += char_len; // 移动到下一个字符
    }

    // 创建并编译着色器
    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_src, NULL);
    glCompileShader(vertex_shader);
    check_shader_error(vertex_shader);

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_src, NULL);
    glCompileShader(fragment_shader);
    check_shader_error(fragment_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    // 获取 uniform 位置
    GLint tex_loc = glGetUniformLocation(program, "tex");
    GLint color_loc = glGetUniformLocation(program, "color");

    // 启用混合
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 主循环
    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(program);
        glUniform1i(tex_loc, 0);
        glUniform3f(color_loc, 0.0f, 0.0f, 0.0f); // 设置字体颜色为黑色

        // 绑定纹理
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlas->id);

        // 渲染缓冲区
        vertex_buffer_render(buffer, GL_TRIANGLES);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理资源
    vertex_buffer_delete(buffer);
    texture_font_delete(font);
    texture_atlas_delete(atlas);
    glDeleteProgram(program);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}