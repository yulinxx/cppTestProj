#include <iostream>

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/geometries/register/box.hpp>

template<typename T>
struct PointTest
{
    // PointTest(double xx, double yy, int i = 0) : x(xx), y(yy), id(i)
    PointTest(T xx, T yy) : x(xx), y(yy)
    {
    }

    T x = 0.0;
    T y = 0.0;
};

template<typename T>
struct RectTest
{
    RectTest(PointTest<T> p1, PointTest<T> p2, PointTest<T> p3, PointTest<T> p4) :
        lb(p1), rb(p2), rt(p3), lt(p4)
    {
    }

    PointTest<T> lbPt(){
        return lb;
    }

    PointTest<T> rbPt(){
        return rb;
    }

    PointTest<T> rtPt(){
        return rt;
    }
    PointTest<T> ltPt(){
        return lt;
    }

    
    PointTest<T> lb;
    PointTest<T> rb;
    PointTest<T> rt;
    PointTest<T> lt;
};

// 将MyPoint注册到bg的native类型中
// 点
BOOST_GEOMETRY_REGISTER_POINT_2D(PointTest<double>, double, cs::cartesian, x, y)

// 包络框
// BOOST_GEOMETRY_REGISTER_BOX(RectTest, PointTest, PointTest, PointTest)
// BOOST_GEOMETRY_REGISTER_BOX(RectTest, PointTest, lbPt(), rtPt())
// BOOST_GEOMETRY_REGISTER_BOX_2D_4VALUES(RectTest, PointTest, lbPt(), rbPt(), rtPt(), ltPt())

namespace bgi = boost::geometry::index;
using PtPair = std::pair<PointTest<double>, int>;
using PtRTree = bgi::rtree<PtPair, bgi::quadratic<16>>;

using RectPair = std::pair<PointTest<double>, int>;
using RectRTree = bgi::rtree<RectPair, bgi::quadratic<16>>;

int main()
{
    std::cout << "RTree Test" << std::endl;

    PtRTree ptTree;
    ptTree.insert(std::make_pair(PointTest<double>(0, 0), 100));
    ptTree.insert({PointTest<double>(10, 0), 101}); // C++11 赋值方式
    ptTree.insert({PointTest<double>(10, 10), 102});
    ptTree.insert({PointTest<double>(0, 10), 103});
    std::cout << "Point Tree have:" << ptTree.size() << std::endl;

    auto iter = ptTree.qbegin(bgi::contains(PointTest<double>(10, 0)));
    // auto iter = tree.qbegin(bgi::intersects(PointTest(10, 0)));
    for (; iter != ptTree.qend(); iter++)
    {
        auto a = *iter;
        std::cout << "--------Find:\n"
                  << a.first.x << " " << a.first.y
                  << " " << a.second << std::endl;
    }

    RectRTree rectTree;
    // rectTree.insert(std::make_pair(
    //     RectTest(
    //              PointTest(100, 100),
    //              PointTest(200, 100),
    //              PointTest(200, 200),
    //              PointTest(100, 200)),
    //     102));


    // rectTree.insert({
    //     RectTest(
    //              PointTest(100, 100),
    //              PointTest(200, 100),
    //              PointTest(200, 200),
    //              PointTest(100, 200)
    //              ), 0
    // });
    // std::cout << "Rect Tree have:" << rectTree.size() << std::endl;

    

    return 0;
}

//////////////////////////////////////////////////////////////

// #include <iostream>
// #include <boost/geometry.hpp>
// #include <boost/geometry/geometries/register/point.hpp>
// struct MyPoint //自定义的点
// {
// 	MyPoint(double x, double y, int id) :m_x(x), m_y(y), m_id(id) {}
// 	double m_x;
// 	double m_y;
// 	int m_id; //标识id
// };
// //将MyPoint注册到bg的native类型中
// BOOST_GEOMETRY_REGISTER_POINT_2D(MyPoint, double, cs::cartesian, m_x, m_y)
// namespace bg = boost::geometry;
// int main(int argc, char *argv[])
// {
// 	using geo_point = MyPoint;
// 	geo_point gp1(0,0,1), gp2(1,0,2);
// 	auto dis = bg::distance(gp1,gp2);
// 	std::cout << bg::dsv(gp1) << " distant from " << bg::dsv(gp2) << " is " << dis <<std::endl;
// 	return 0;
// }
// 原文链接：https://blog.csdn.net/iLuoPu/article/details/124034790

//////////////////////////////////////////////////////////////
// 将自定义类,用BOOST_GEOMETRY_REGISTER*宏，注册到boost的native geometry type中
// #include <boost/geometry.hpp>
// #include <boost/geometry/index/rtree.hpp>
// #include <boost/geometry/geometries/box.hpp>
// #include <boost/geometry/geometries/register/point.hpp>

// namespace bg = boost::geometry;
// namespace bgi = boost::geometry::index;

// #include <glm/vec3.hpp>
// // BOOST_GEOMETRY_REGISTER_POINT_3D(glm::vec3, float, bg::cs::cartesian, x, y, z)

// #include <iostream>
// int main() {

//     using IndexedPoint = std::pair<glm::vec3, uint32_t>;
//     // using RTree = boost::geometry::index::rtree<IndexedPoint, bgi::rstar<8>>;
//     using RTree = boost::geometry::index::rtree<IndexedPoint, bgi::quadratic<16>>;

//     RTree rtree;
//     rtree.insert({glm::vec3(1,1,1), 1});
//     rtree.insert({glm::vec3(2,2,2), 2});
//     rtree.insert({glm::vec3(3,3,3), 3});
//     rtree.insert({glm::vec3(4,4,4), 4});

//     auto it = rtree.qbegin(bgi::nearest(glm::vec3(2.9, 2.9, 2.9), 99));

//     auto p = it->first;
//     std::cout << "Nearest: # " << it->second << " (" << p.x << ", " << p.y << " " << p.z << ")\n";
// }