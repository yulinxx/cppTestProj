#if 0
#include <pybind11/embed.h> // everything needed for embedding
namespace py = pybind11;

int main()
{
  // 在执行Python代码之前，都会先创建py::scoped_interpreter guard{};
  // 在使用任何 Python API (包括 pybind11中的所有函数和类)之前，必须对解释器进行初始化。 
  // scoped_interpreter RAII 类负责解释器的生命周期。
  // scoped_interpreter生命周期结束后，解释器关闭并清理内存，在此之后就不能调用 Python 函数了。

  py::scoped_interpreter guard{}; // start the interpreter and keep it alive
  py::print("Hello, World!");     // use the Python API
}

#endif

#if 0
#include <pybind11/embed.h>
namespace py = pybind11;

int main() 
{
py::scoped_interpreter guard{};

py::exec(R"(
kwargs = dict(name="World", number=42)
message = "Hello, {name}! The answer is {number}".format(**kwargs)
print(message)
)");
}

#endif

#if 0
#include <pybind11/embed.h>
namespace py = pybind11;
using namespace py::literals;

int main() {
py::scoped_interpreter guard{};

auto kwargs = py::dict("name"_a="World", "number"_a=42);
auto message = "Hello, {name}! The answer is {number}"_s.format(**kwargs);
py::print(message);
}

#endif

#if 0
#include <pybind11/embed.h>
#include <iostream>

namespace py = pybind11;
using namespace py::literals;

int main()
{
  py::scoped_interpreter guard{};

  auto locals = py::dict("name"_a = "World", "number"_a = 42);
  py::exec(R"(
message = "Hello, {name}! The answer is {number}".format(**locals())
)",
           py::globals(), locals);

  auto message = locals["message"].cast<std::string>();
  std::cout << message;
}

#endif

#if 1
#include <pybind11/pybind11.h>
namespace py = pybind11;

int add(int i, int j)
{
    return i + j;
}

PYBIND11_MODULE(example, m)
{
    // optional module docstring
    m.doc() = "pybind11 example plugin";
    // expose add function, and add keyword arguments and default arguments
    m.def("add", &add, "A function which adds two numbers", py::arg("i")=1, py::arg("j")=2);

    // exporting variables
    m.attr("the_answer") = 42;
    py::object world = py::cast("World");
    m.attr("what") = world;
}

#endif