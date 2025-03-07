#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

int main() {
    // 初始化 FreeType 库
    FT_Library library;
    if (FT_Init_FreeType(&library)) {
        std::cerr << "Freetype 初始化失败!" << std::endl;
        return 1;
    }

    // 加载字体
    const char* font_path = "C:/Windows/Fonts/arial.ttf";  // 替换为你本地的字体路径
    FT_Face face;
    if (FT_New_Face(library, font_path, 0, &face)) {
        std::cerr << "加载字体失败!" << std::endl;
        return 1;
    }

    // 设置字体大小
    FT_Set_Pixel_Sizes(face, 0, 48); // 48px 字号

    // 加载一个字符的轮廓
    FT_UInt glyph_index = FT_Get_Char_Index(face, 'A');
    if (glyph_index == 0) {
        std::cerr << "字符未找到!" << std::endl;
        return 1;
    }

    if (FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT)) {
        std::cerr << "加载字符轮廓失败!" << std::endl;
        return 1;
    }

    std::cout << "成功加载并渲染字形! Successful! " << std::endl;

    // 清理资源
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return 0;
}
