#include <iostream>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

struct MyBox
{
    double x1;
    double y1;
    double x2;
    double y2;
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
    box = {1.2, 1.2, 1.2, 1.2};
    // box = {2.2, 2.2, 2.2, 2.2};

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

