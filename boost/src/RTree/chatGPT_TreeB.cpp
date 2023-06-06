// 抱歉，作为AI语言模型，我无法编写代码。但是，我可以为您提供一些关于如何使用boost库实现rtree的指导。

// 首先，您需要了解boost库中的rtree模块。该模块提供了一个基于矩形查找的空间索引数据结构。
// 它支持多种数据类型和查询方式，并且可以自定义矩形数据结构。

// 下面是一个简单的示例代码，演示如何使用boost库实现rtree：

#include <iostream>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// 自定义矩形数据结构
template<typename T>
struct MyRect {
    T x1, y1, x2, y2;
};


typedef bg::model::point<int, 2, bg::cs::cartesian> point_t;
// typedef bg::model::box<point_t> box_t;


// 定义矩形的bounding box
namespace boost
{
    namespace geometry
    {
        namespace traits
        {
            // template <>
            // struct tag<MyRect>
            // {
            //     typedef box_tag type;
            // };
            // template <>
            // // struct box<MyRect>
            // struct box<MyRect>
            // {
            //     static inline MyRect apply(const MyRect &r)
            //     {
            //         return r;
            //     }
            // };

            template <typename T>
            struct tag<MyRect<T>>
            {
                typedef box_tag type;
            };
            template <typename T>
            struct point_type<MyRect<T>>
            {
                typedef point_t type;
            };
            template <typename T>
            struct indexed_access<MyRect<T>, min_corner, 0>
            {
                static inline double get(const MyRect<T> &b) { return b.x1; }
            };
            template <typename T>
            struct indexed_access<MyRect<T>, min_corner, 1>
            {
                static inline double get(const MyRect<T> &b) { return b.y1; }
            };
            template <typename T>
            struct indexed_access<MyRect<T>, max_corner, 0>
            {
                static inline double get(const MyRect<T> &b) { return b.x2; }
            };
            template <typename T>
            struct indexed_access<MyRect<T>, max_corner, 1>
            {
                static inline double get(const MyRect<T> &b) { return b.y2; }
            };
        }
    }
}

int main() {
    // 创建一个rtree对象
    bgi::rtree<MyRect<double>, bgi::quadratic<16>> rtree;

    // 添加一些矩形到rtree中
    rtree.insert(MyRect<double>{0, 0, 10, 10});
    rtree.insert(MyRect<double>{20, 20, 30, 30});

    // 查询包含某个点的所有矩形
    std::vector<MyRect<double>> result;
    bg::model::point<int, 2, bg::cs::cartesian> point(5, 5);
    rtree.query(bgi::intersects(point), std::back_inserter(result));

    // 输出查询结果
    for (const auto& rect : result) {
        std::cout << "MyRect: (" << rect.x1 << ", " << rect.y1 << "), (" << rect.x2 << ", " << rect.y2 << ")" << std::endl;
    }

    result.clear();
    std::cout<<"\n ---- \n";

    // MyRect qBox{9, 9, 25, 25};
    MyRect<double> qBox{21, 21, 25, 25};
    rtree.query(bgi::intersects(qBox), std::back_inserter(result));

    // 输出查询结果
    for (const auto& rect : result) {
        std::cout << "MyRect: (" << rect.x1 << ", " << rect.y1 << "), (" << rect.x2 << ", " << rect.y2 << ")" << std::endl;
    }

    return 0;
}

// 在上面的示例代码中，首先定义了一个自定义的矩形数据结构，并为其定义了bounding box。
// 然后，创建了一个rtree对象，并向其中添加了一些矩形。
// 最后，使用rtree的query函数查询包含某个点的所有矩形，并输出查询结果。

// 需要注意的是，上面的示例代码只是一个简单的演示，实际使用中可能需要根据具体需求进行修改和扩展。
