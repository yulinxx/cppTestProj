#include <iostream>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

struct MyBox
{
    double x1;
    double y1;
    double x2;
    double y2;

    bool operator==(const MyBox& other) const {
        return (x1 == other.x1 && y1 == other.y1 &&
                x2 == other.x2 && y2 == other.y2);
    }
};

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> point_t;
typedef bg::model::box<point_t> box_t;

namespace boost
{
    namespace geometry
    {
        namespace traits
        {
            template <>
            struct tag<MyBox>
            {
                typedef box_tag type;
            };
            template <>
            struct point_type<MyBox>
            {
                typedef point_t type;
            };
            template <>
            struct indexed_access<MyBox, min_corner, 0>
            {
                static inline double get(const MyBox &b) { return b.x1; }
            };
            template <>
            struct indexed_access<MyBox, min_corner, 1>
            {
                static inline double get(const MyBox &b) { return b.y1; }
            };
            template <>
            struct indexed_access<MyBox, max_corner, 0>
            {
                static inline double get(const MyBox &b) { return b.x2; }
            };
            template <>
            struct indexed_access<MyBox, max_corner, 1>
            {
                static inline double get(const MyBox &b) { return b.y2; }
            };
        }
    }
} // namespace boost::geometry::traits

typedef boost::geometry::index::rtree<MyBox, boost::geometry::index::quadratic<16>> rtree_t;

int main()
{

    rtree_t rtree;

    MyBox box = {0.0, 0.0, 1.0, 1.0};
    rtree.insert(box);

    box = {0.5, 0.5, 1.5, 1.5};
    rtree.insert(box);


    // box = {0.3, 0.3, 0.4, 0.4};
    // box = {0.7, 0.7, 0.8, 0.8};
    // box = {1.2, 1.2, 1.2, 1.2};
    box = {2.2, 2.2, 2.2, 2.2};

    std::vector<MyBox> result;              // 存储查询结果的向量

    rtree.query(bgi::intersects(box), std::back_inserter(result)); // 执行相交查询
    for(auto& e : result)
    {
        std::cout<<"x1:"<<e.x1<<" y1:"<<e.y1
        <<"x2:"<<e.x2<<" y2:"<<e.y2<<std::endl;
    }

    std::cout<<std::endl<<" --------------- "<<std::endl;

    auto it = rtree.qbegin(bgi::intersects(box));
    for (auto it = rtree.qbegin(bgi::intersects(box));
         it != rtree.qend(); ++it)
    {
        std::cout<<"x1:"<<(*it).x1<<" y1:"<<(*it).y1
        <<"x2:"<<(*it).x2<<" y2:"<<(*it).y2<<std::endl;
    }
    return 0;
}

















// // 要将自定义结构体添加到Boost的Rtree中，需要先定义一个适配器类，
// // 该类将自定义结构体转换为Boost Rtree所需的类型。以下是一个示例适配器类：

// #include <boost/geometry.hpp>
// #include <boost/geometry/index/rtree.hpp>

// namespace bg = boost::geometry;
// namespace bgi = boost::geometry::index;

// struct MyBox {
//     double x1;
//     double y1;
//     double x2;
//     double y2;
// };

// // 定义适配器类
// struct MyBoxAdapter {
//     typedef MyBox boxx;
//     typedef bg::model::box<bg::model::point<double, 2, bg::cs::cartesian>> box_type;

//     box_type operator()(const boxx& b) const {
//         // return box_type(bg::make_point(b.x1, b.y1), bg::make_point(b.x2, b.y2));
//         return box_type(b.x1, b.y1, b.x2, b.y2);
//     }
// };

// int main()
// {

//     // 然后，可以使用以下代码创建一个Rtree并将自定义结构体添加到其中：
//     bgi::rtree<MyBox, bgi::quadratic<16>> rtree; // 创建Rtree
//     MyBox box = {0.0, 0.0, 1.0, 1.0};            // 创建自定义结构体实例
//     rtree.insert(box);                           // 将自定义结构体添加到Rtree中

//     // 要进行相交查询，可以使用以下代码：
//     MyBox query_box = {0.5, 0.5, 1.5, 1.5}; // 创建查询框
//     std::vector<MyBox> result;              // 存储查询结果的向量

//     rtree.query(bgi::intersects(query_box), std::back_inserter(result)); // 执行相交查询

//     // 输出查询结果
//     for (const auto &r : result)
//     {
//         std::cout << "(" << r.x1 << ", " << r.y1 << ") - (" << r.x2 << ", " << r.y2 << ")" << std::endl;
//     }
// }