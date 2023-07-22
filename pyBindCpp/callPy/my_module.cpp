#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/embed.h>

namespace py = pybind11;

int main() {
    std::cout<<"　Welcome Here is CPP "<<std::endl;
    
    // 初始化 Python 解释器
    py::scoped_interpreter guard{};

    try {
        // 导入 Python 模块
        py::module module = py::module::import("python_script");

        // 调用 Python 函数 greet
        py::object result = module.attr("greet")("John");
        std::string greeting = result.cast<std::string>();
        std::cout << greeting << std::endl;  // 输出: Hello, John!

        // 创建 Python 类 MyClass 的实例
        py::object MyClass = module.attr("MyClass");
        py::object instance = MyClass(42);

        // 调用 Python 类 MyClass 的方法 get_value
        py::object value = instance.attr("get_value")();
        int intValue = value.cast<int>();
        std::cout << "Value: " << intValue << std::endl;  // 输出: Value: 42
    } catch (const py::error_already_set& e) {
        std::cerr << "Python error:\n" << e.what() << std::endl;
    }

    std::cout<<"　End "<<std::endl;
    
    return 0;
}
