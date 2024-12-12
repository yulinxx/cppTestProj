#include <pybind11/pybind11.h>
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <iostream>
#include <vector>

namespace py = pybind11;

// ... 其他代码 ...

int main() {
    py::scoped_interpreter guard{}; // 初始化 Python 解释器

    try {
        // 导入 Python 模块
        py::module transform_ops = py::module::import("nums");

        // C++ 中创建一个 3x3 矩阵
        std::vector<std::vector<double>> matrix = {
            {1.0, 2.0},
            {3.0, 4.0}
        };

        // 将 C++ 矩阵转换为 Python numpy 数组
        py::array_t<double> py_matrix = to_numpy_array(matrix);

        // 设置平移向量、缩放因子和旋转角度
        std::vector<double> translation_vector = {1.0, 2.0}; // 平移向量
        double scale_factor = 2.0; // 缩放因子
        double angle_degrees = 45.0; // 旋转角度

        // 手动将 translation_vector 转换为 Python list
        py::list py_translation_vector = to_python_list(translation_vector);

        // 1. 应用平移、缩放和旋转变换
        py::object transformed = transform_ops.attr("apply_transformations")(
            py_matrix, py_translation_vector, scale_factor, angle_degrees
        );

        // 输出变换后的矩阵
        py::print(transformed); // 直接打印Python对象

    } catch (const py::error_already_set& e) {
        std::cerr << "Python error: " << e.what() << std::endl;
    }

    return 0;
}