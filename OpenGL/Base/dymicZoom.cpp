/*
 * @Author: lyl yulinxx@gmail.com
 * @Date: 2025-11-05 22:38:08
 * @LastEditors: lyl yulinxx@gmail.com
 * @LastEditTime: 2025-11-05 22:50:53
 * @FilePath: \CppTestProj\OpenGL\Base\dymicZoom.cpp
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
 // main.cpp
#define _CRT_SECURE_NO_WARNINGS
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>

#include <vector>
#include <thread>
#include <atomic>
#include <omp.h>

// ======== 上面 10 行神奇代码 ========
BOOST_GEOMETRY_REGISTER_POINT_2D(glm::vec2, float, cs::cartesian, x, y)
namespace boost::geometry::traits
{
    template<> struct tag<glm::vec2>
    {
        using type = point_tag;
    };
}
BOOST_GEOMETRY_REGISTER_BOX(Box, glm::vec2, min_corner(), max_corner())
// =====================================

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

using Box = bg::model::box<glm::vec2>;
using Value = std::pair<Box, uint32_t>;
using RTree = bgi::rtree<Value, bgi::quadratic<16>>;

std::vector<glm::vec2> g_points;
RTree                   g_rtree;
std::atomic<bool>       g_rebuild{ true };

int main()
{
    // 造 10 万个随机点
    for (int i = 0; i < 100000; ++i)
    {
        glm::vec2 p{ (rand() % 2000 - 1000) / 10.f, (rand() % 2000 - 1000) / 10.f };
        g_points.push_back(p);
    }

    // 建 RTree
    std::vector<Value> boxes;
    boxes.reserve(g_points.size());
    for (uint32_t i = 0; i < g_points.size(); ++i)
    {
        Box b(g_points[i], g_points[i]);   // 点当作 0 大小的 box
        boxes.emplace_back(b, i);
    }
    g_rtree = RTree(boxes.begin(), boxes.end());

    // GLFW + GLAD
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* win = glfwCreateWindow(800, 600, "Boost+GLM OK!", nullptr, nullptr);
    glfwMakeContextCurrent(win);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    while (!glfwWindowShouldClose(win))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // 随便查个范围
        Box query({ -50,-50 }, { 50,50 });
        std::vector<Value> result;
        g_rtree.query(bgi::intersects(query), std::back_inserter(result));
        printf("Visible: %zu\n", result.size());

        glfwSwapBuffers(win);
        glfwPollEvents();
    }
}