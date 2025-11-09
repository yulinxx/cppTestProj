// boost::geometry简介,真香_南城小金刚的博客-CSDN博客_boost geometry
// https://blog.csdn.net/iLuoPu/article/details/124034790

// 求地理距离

#include <iostream>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>

namespace bg = boost::geometry;

// 自定义点,继承
// 当我们想在点结构中加入其他信息,比如一个字段id,该怎么办？你可能会使用继承,
// 但这样你就不能用相关算法了,因为想要使用相关算法,类必须要满足对应的concept.
// 那如何是好？你应该将自定义类,用BOOST_GEOMETRY_REGISTER*宏,
// 注册到boost的native geometry type中.啥？来个例子你就明白了.
struct MyPoint // 自定义的点
{
    MyPoint(double x, double y, int id) : m_x(x), m_y(y), m_id(id)
    {
    }
    double m_x;
    double m_y;
    int m_id; // 标识id
};

// 将MyPoint注册到bg的native类型中
BOOST_GEOMETRY_REGISTER_POINT_2D(MyPoint, double, bg::cs::cartesian, m_x, m_y)

int main(int argc, char* argv[])
{
    // cs / coordinate system  坐标系
    // geographic  美[ˌdʒiəˈgræfɪk] 地理

    { // 求地理距离
        using geo_point = bg::model::point<
            double, 2, bg::cs::geographic<bg::degree>>; // 地理坐标系,单位为度

        geo_point gp_beijing(116.23128, 40.22077);  // 北京经纬度坐标
        geo_point gp_shanghai(121.48941, 31.40527); // 上海经纬度坐标
        auto dis = bg::distance(gp_beijing, gp_shanghai);
        std::cout << "北京到上海的距离是 " << dis / 1000 << " km" << std::endl;

        // 北京到上海的距离是 1086.91 km
    }

    std::cout << "------------------------------" << std::endl;

    { // 点到线段的距离
        using geo_point = bg::model::point<
            double, 2, bg::cs::cartesian>; // 笛卡尔坐标系

        using geo_segment = bg::model::segment<geo_point>; // 线段

        geo_segment seg = { {0, 1}, {4, 1} };
        geo_point gp_in(2, 0);  // 垂足在线段上
        geo_point gp_out(5, 0); // 垂足不在线段上

        auto dis_in = bg::distance(gp_in, seg);
        auto dis_out = bg::distance(gp_out, seg);

        // DSV stands for Delimiter Separated Values
        std::cout << "distance from " << bg::dsv(gp_in) << " to " << bg::dsv(seg) << " is " << dis_in << std::endl;
        std::cout << "distance from " << bg::dsv(gp_out) << " to " << bg::dsv(seg) << " is " << dis_out << std::endl;

        // distance from (2, 0) to ((0, 1), (4, 1)) is 1
        // distance from (5, 0) to ((0, 1), (4, 1)) is 1.41421
    }

    std::cout << "------------------------------" << std::endl;

    { // 点是否在图形内
        using geo_point = bg::model::point<
            double, 2, bg::cs::cartesian>;           // 笛卡尔坐标系
        using geo_ring = bg::model::ring<geo_point>; // 线段

        geo_ring ring = { {0, 0}, {0, 2}, {2, 2}, {2, 0}, {0, 0} }; // 矩形
        geo_point gp_in(1, 1);                                    // 在图形内
        geo_point gp_out(3, 0);                                   // 不在图形内
        auto is_in = bg::within(gp_in, ring);                     // 是否在图形内部
        auto is_out = bg::within(gp_out, ring);                   // 是否在图形内部
        std::cout << bg::dsv(gp_in) << " locates inside" << bg::dsv(ring) << " " << is_in << std::endl;
        std::cout << bg::dsv(gp_out) << " locates inside" << bg::dsv(ring) << " " << is_out << std::endl;

        // (1, 1) locates inside((0, 0), (0, 2), (2, 2), (2, 0), (0, 0)) 1
        // (3, 0) locates inside((0, 0), (0, 2), (2, 2), (2, 0), (0, 0)) 0
    }

    { // 点乘操作
        using geo_point = bg::model::point<
            double, 2, bg::cs::cartesian>; // 笛卡尔坐标系

        geo_point gp1(1, 0), gp2(1, 1), gp3(0, 1);
        auto dp12 = bg::dot_product(gp1, gp2);
        auto dp13 = bg::dot_product(gp1, gp3);
        std::cout << bg::dsv(gp1) << "dot product" << bg::dsv(gp2) << "=" << dp12 << std::endl;
        std::cout << bg::dsv(gp1) << "dot product" << bg::dsv(gp3) << "=" << dp13 << std::endl;

        // (1, 0)dot product(1, 1)=1
        // (1, 0)dot product(0, 1)=0
    }

    { // 求外接矩形AABB
        using geo_point = bg::model::point<
            double, 2, bg::cs::cartesian>;                 // 笛卡尔坐标系
        using geo_line = bg::model::linestring<geo_point>; // 线
        using geo_box = bg::model::box<geo_point>;         // box

        geo_line line = { {1, 1}, {2, 0}, {3, 2} };
        geo_box box;
        // envelope  信封 包络 包络线
        bg::envelope(line, box);
        std::cout << bg::dsv(line) << "'s AABB is " << bg::dsv(box) << std::endl;

        // ((1, 1), (2, 0), (3, 2))'s AABB is ((1, 0), (3, 2))
    }

    {
        MyPoint gp1(0, 0, 1), gp2(1, 0, 2);
        auto dis = bg::distance(gp1, gp2);
        std::cout << bg::dsv(gp1) << " distant from " << bg::dsv(gp2) << " is " << dis << std::endl;
    }
    return 0;
}