#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <iostream>
#include <string>

namespace py = pybind11;

int main() {
    py::scoped_interpreter guard{}; // 初始化 Python 解释器

    try {
        // 加载 Python 模块
        py::module dpi_extractor = py::module_::import("PyDPI");

        // 调用 Python 函数
        // std::string image_path = "/home/x/Pictures/2021_02_17_10_19_53_1.jpg";
        std::string image_path = "/home/x/Pictures/BingWallpaper/20241021-HeianJingu2024_JA-JP4866409141_UHD.jpg";
        // std::string image_path = "/home/x/Pictures/asian_stars.jpg";
        py::object dpi = dpi_extractor.attr("get_image_dpi")(image_path);

        // 检查是否成功获取 DPI 信息
        if (!dpi.is_none()) {
            // 将 Python 元组转换为 C++ std::pair
            std::pair<int, int> dpi_values = dpi.cast<std::pair<int, int>>();
            std::cout << "图片的DPI为：" << dpi_values.first << " x " << dpi_values.second << std::endl;
        } else {
            std::cout << "未找到DPI信息" << std::endl;
        }
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error: " << e.what() << std::endl;
    }

    return 0;
}