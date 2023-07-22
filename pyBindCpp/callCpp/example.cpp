#include <pybind11/pybind11.h>
namespace py = pybind11;

int add(int i, int j)
{
    return i + j;
}

std::string say(std::string str)
{
    std::string strA = "CPP Hello, word";
    return strA + str;
}

// https://www.cnblogs.com/okmai77xue/p/17487285.html
// pybind11绑定类

struct Pet
{
    Pet(const std::string &name) : name(name) {}
    void setName(const std::string &name_) { name = name_; }
    const std::string &getName() const { return name; }

    std::string name;
};


class MyClass {
public:
    MyClass(int value) : data(value) {}

    int getData() const {
        return data;
    }

    void setData(int value) {
        data = value;
    }

private:
    int data;
};



class ComplexNumber {
public:
    ComplexNumber(double real, double imaginary)
        : realPart(real), imaginaryPart(imaginary) {}

    double getRealPart() const {
        return realPart;
    }

    double getImaginaryPart() const {
        return imaginaryPart;
    }

    double getMagnitude() const {
        return std::sqrt(realPart * realPart + imaginaryPart * imaginaryPart);
    }

    void setRealPart(double real) {
        realPart = real;
    }

    void setImaginaryPart(double imaginary) {
        imaginaryPart = imaginary;
    }

private:
    double realPart;
    double imaginaryPart;
};


// 当一个类的成员变量是另一个类的对象时
class Point {
public:
    Point(double x, double y)
        : xCoord(x), yCoord(y) {}

    double getX() const {
        return xCoord;
    }

    double getY() const {
        return yCoord;
    }

private:
    double xCoord;
    double yCoord;
};

class Circle {
public:
    Circle(double radius, const Point& center)
        : circleRadius(radius), circleCenter(center) {}

    double getRadius() const {
        return circleRadius;
    }

    Point getCenter() const {
        return circleCenter;
    }

private:
    double circleRadius;
    Point circleCenter;
};

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
// PYBIND11_MODULE会创建一个函数，它在Python中使用import语句时被调用。
// 宏的第一个参数是模块名（example），不使用引号包住；
// 第二个参数是类型为py::module_的变量名（m），它是创建绑定的主要接口。

// PYBIND11_MODULE 是一个宏，用于将 C++ 代码中的函数、类和变量绑定到 Python 中。
// 即 将 C++ 模块导出到 Python 解释器中。
PYBIND11_MODULE(example, m)
{
    m.doc() = "pybind11 example plugin";

    // module_::def()方法，则会生成add函数的Python绑定代码。
    // def: 这是一个 PyBind11 的方法，用于定义函数、类或变量的导出规则。使用 def 来定义导出的函数。
    // "add" python中的函数名称,使用此名称来访问C++的函数
    // &add  C++中的函数地址,通过此地址传递给PyBind11
    // "A fu... bers"  字符串,用于 为导入的函数提供文档描述.
    m.def("add", &add, "A function which adds two numbers");
    m.def("say", &say, "A fucntion which return str");

    // `class_`会创建C++ class或 struct的绑定。`init()`方法使用类构造函数的参数类型作为模板参数，
    
    // py::class_ 是 PyBind11 库中的一个宏，用于将 C++ 类绑定到 Python 中，使其在 Python 中可用。
    // 它是 PyBind11 的核心功能之一，用于实现 C++ 和 Python 之间的无缝交互。


    // 并包装相应的构造函数；静态成员函数需要使用`class_::def_static`来绑定
    // py::class_<Pet>(m, "Pet"): 这行代码创建了一个 PyBind11 包装类，它将 C++ 类 Pet 导出为名为 "Pet" 的 Python 类。
    // .def(py::init<const std::string &>()):
    // 这行代码定义了 Python 类的构造函数。接受一个 const std::string& 类型的参数，并且在 Python 中可以使用相同的参数来创建 Pet 对象。

    // .def("setName", &Pet::setName): 这行代码定义了一个名为 "setName" 的方法，并将其与 C++ 类 Pet 中的 setName 方法相关联。
    // 这意味着在 Python 中创建的 Pet 对象可以调用 setName 方法，并且在 C++ 中实际执行对应的函数。

    // .def("__repr__", ...): 这行代码定义了 Python 类的 __repr__ 方法。
    // __repr__ 方法是一个特殊的方法，它用于返回对象的字符串表示形式。
    // 在这里，我们使用 lambda 函数来自定义 __repr__ 方法的行为，将返回一个字符串，其中包含 Pet 对象的名称。
    // 这使得在 Python 中打印 Pet 对象时，会显示这个自定义的字符串表示形式。

    // py::class_<Pet>(m, "Pet")
    //     .def(py::init<const std::string &>())
    //     .def("setName", &Pet::setName)
    //     .def("getName", &Pet::getName);

    // 使用`print(p)`打印对象信息时，上面的例子会得到一些基本无用的信息，可以绑定一个工具函数到`__repr__`方法，
    // 来返回可读性好的摘要信息。在不改变Pet类的基础上，使用一个匿名函数来完成。
    // pybind11支持无状态和有状态的lambda闭包，即lambda表达式的`[]`是否带捕获参数。
    py::class_<Pet>(m, "Pet")
        .def(py::init<const std::string &>())
        .def("setName", &Pet::setName)
        .def("getName", &Pet::getName)
        .def("__repr__",
             [](const Pet &a)
             {
                 return "<example.Pet named '" + a.name + "'>";
             });
             
    py::class_<MyClass>(m, "MyClass")
        .def(py::init<int>())    // 导出构造函数
        .def("getData", &MyClass::getData)  // 导出成员函数 getData
        .def("setData", &MyClass::setData); // 导出成员函数 setData
        
    py::class_<ComplexNumber>(m, "ComplexNumber")
        .def(py::init<double, double>())  // 导出构造函数
        .def("getRealPart", &ComplexNumber::getRealPart)  // 导出 getRealPart 成员函数
        .def("getImaginaryPart", &ComplexNumber::getImaginaryPart)  // 导出 getImaginaryPart 成员函数
        .def("getMagnitude", &ComplexNumber::getMagnitude)  // 导出 getMagnitude 成员函数
        .def("setRealPart", &ComplexNumber::setRealPart)  // 导出 setRealPart 成员函数
        .def("setImaginaryPart", &ComplexNumber::setImaginaryPart);  // 导出 setImaginaryPart 成员函数
        

    py::class_<Point>(m, "Point")
        .def(py::init<double, double>())  // 导出构造函数
        .def("getX", &Point::getX)  // 导出 getX 成员函数
        .def("getY", &Point::getY);  // 导出 getY 成员函数

    py::class_<Circle>(m, "Circle")
        .def(py::init<double, const Point&>())  // 导出构造函数
        .def("getRadius", &Circle::getRadius)  // 导出 getRadius 成员函数
        .def("getCenter", &Circle::getCenter);  // 导出 getCenter 成员函数
}