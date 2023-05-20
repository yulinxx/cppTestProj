//  ChatGPT  V3.5
//  2023-05-20
// 要在Boost中的RTree中使用自定义的Box和Point数据类，你需要为它们定义相应的Traits。Traits为Boost.Geometry提供了与你的数据类交互的方式。

// 首先，我们定义自定义的Box和Point数据类：

// cpp
// Copy code
// class MyBox {
// public:
//     // 定义你的Box类的成员变量和方法
//     // ...
// };

// class MyPoint {
// public:
//     // 定义你的Point类的成员变量和方法
//     // ...
// };
// 接下来，我们为这两个数据类定义Traits。对于Point类型，我们需要定义一个PointTraits类，告诉Boost.Geometry如何访问和操作Point对象。
// 对于Box类型，我们需要定义一个BoxTraits类，告诉Boost.Geometry如何访问和操作Box对象。

// cpp
// Copy code
// #include <boost/geometry.hpp>

// namespace bg = boost::geometry;

// // PointTraits 定义
// namespace boost { namespace geometry { namespace traits {

// template<>
// struct tag<MyPoint> {
//     typedef point_tag type;
// };

// template<>
// struct coordinate_type<MyPoint> {
//     typedef double type;
// };

// template<>
// struct coordinate_system<MyPoint> {
//     typedef bg::cs::cartesian type;
// };

// template<>
// struct dimension<MyPoint> : boost::mpl::int_<2> {};

// template<std::size_t Dimension>
// struct access<MyPoint, Dimension> {
//     static inline double get(const MyPoint& point) {
//         // 返回自定义Point对象的坐标值
//         // 例如，如果Point类有成员变量x和y，则返回point.x或point.y
//     }

//     static inline void set(MyPoint& point, double value) {
//         // 设置自定义Point对象的坐标值
//         // 例如，如果Point类有成员变量x和y，则设置point.x或point.y为value
//     }
// };

// }}} // namespace boost::geometry::traits


// // BoxTraits 定义
// namespace boost { namespace geometry { namespace traits {

// template<>
// struct tag<MyBox> {
//     typedef box_tag type;
// };

// template<>
// struct point_type<MyBox> {
//     typedef MyPoint type;
// };

// template<std::size_t Index, typename Box>
// struct indexed_access<Box, min_corner, Index> {
//     static inline typename bg::coordinate_type<Box>::type get(const Box& box) {
//         // 返回自定义Box对象的最小角点的坐标值
//         // 例如，如果Box类有成员变量minPoint，则返回box.minPoint的坐标值
//     }
// };

// template<std::size_t Index, typename Box>
// struct indexed_access<Box, max_corner, Index> {
//     static inline typename bg::coordinate_type<Box>::type get(const Box& box) {
//         // 返回自定义Box对象的最大角点的坐标值
//         // 例如，如果Box类有成员变量maxPoint，则返回box.maxPoint的坐标值
//     }
// };

// }}} // namespace boost::geometry::traits
// 在上述代码中，我们在boost::geometry::traits命名空间下定义了PointTraits和BoxTraits。
// 对于PointTraits，我们定义了tag、coordinate_type、coordinate_system、dimension和access等特征。
// 对于BoxTraits，我们定义了tag、point_type和indexed_access特征。

// 注意，在access特征中，我们提供了get和set函数，用于访问和设置自定义Point对象的坐标值。你需要根据你的自定义Point类的实现，在这些函数中正确地访问和操作坐标值。

// 完成了Traits的定义后，你就可以在Boost的RTree中使用自定义的Box和Point数据类了。

// cpp
// Copy code
// #include <boost/geometry/index/rtree.hpp>

// namespace bgi = boost::geometry::index;

// int main() {
//     typedef bgi::rtree<MyBox, bgi::quadratic<16>> RTree;

//     RTree rtree;

//     // 创建自定义的Box和Point对象
//     MyBox box1(/* 设置Box的参数 */);
//     MyBox box2(/* 设置Box的参数 */);

//     // 插入自定义的Box对象到RTree中
//     rtree.insert(box1);
//     rtree.insert(box2);

//     // 查询操作等...
    
//     return 0;
// }
// 在上述示例中，我们创建了一个RTree对象（RTree类型为bgi::rtree，使用quadratic<16>作为分割策略）。
// 然后，我们创建自定义的Box对象（box1和box2）并将它们插入到RTree中，使用rtree.insert函数进行插入操作。

// 请确保根据你的自定义数据类的实现，适当地实现Traits中的函数，以便能够正确地访问和操作自定义的Box和Point对象。



// 要为自定义的XPoint和XRect类编写Boost.Geometry的traits，你需要实现以下几个特性：

// 对于XPoint类：

// tag特性指定为point_tag，表示这是一个点类型。
// coordinate_type特性指定为点的坐标类型T。
// coordinate_system特性指定为bg::cs::cartesian，表示坐标系为笛卡尔坐标系。
// dimension特性指定为boost::mpl::int_<2>，表示点的维度为2。
// access特性用于访问点的坐标。你需要为XPoint类的X坐标和Y坐标分别实现access特性。
// 对于XRect类：

// tag特性指定为box_tag，表示这是一个框类型。
// point_type特性指定为XPoint<T>，表示框的角点类型为XPoint<T>。
// indexed_access特性用于访问框的最小角点和最大角点的坐标。你需要为XRect类的最小角点和最大角点分别实现indexed_access特性。


#include <iostream>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

/////////////////////////////////////////////
// 自定义的点类
template<typename T>
class XPoint
{


public:
    XPoint(T _x, T _y) : x(_x), y(_y) {

    }

    T getX() const {
        return x;
    }

    void setX(T _x){
        x = _x;
    }

    T getY() const {
        return y;
    }

    void setY(T _y){
        y = _y;
    }
private:
    T x;
    T y;

};

/////////////////////////////////////////////
// 自定义的矩形类
template<typename T>
class XRect
{
public:
XRect(XPoint<T> const & _ptLB, XPoint<T> const & _ptRT):
    ptLB(_ptLB), ptRT(_ptRT)
    {

    }

    XPoint<T> const& min_corner() const {
        return ptLB;
    }

    XPoint<T> const& max_corner() const {
        return ptRT;
    }
private:
    XPoint<T> ptLB;
    XPoint<T> ptRT;
};

/////////////////////////////////////////////
// Traits for XPoint
namespace boost { namespace geometry { namespace traits
{

template <typename T> struct tag<XPoint<T>> { using type = point_tag; };
template <typename T> struct dimension<XPoint<T>> : boost::mpl::int_<2> {};
template <typename T> struct coordinate_type<XPoint<T>> { using type = T; };
template <typename T> struct coordinate_system<XPoint<T>> { using type = bg::cs::cartesian; };

template <typename T>
struct access<XPoint<T>, 0> {
    using CoordinateType = typename coordinate_type<XPoint<T>>::type;

    static inline CoordinateType get(XPoint<T> const& p) { return p.getX(); }
    static inline void set(XPoint<T>& p, CoordinateType const& value) { p.setX(value); }
};

template <typename T>
struct access<XPoint<T>, 1> {
    using CoordinateType = typename coordinate_type<XPoint<T>>::type;

    static inline CoordinateType get(XPoint<T> const& p) { return p.getY(); }
    static inline void set(XPoint<T>& p, CoordinateType const& value) { p.setY(value); }
};

}}} // namespace boost::geometry::traits


/////////////////////////////////////////////
// Traits for XRect
namespace boost { namespace geometry { namespace traits
{

template <typename T> struct tag<XRect<T>> { using type = box_tag ; };
template <typename T> struct point_type<XRect<T>> { using type = XPoint<T>; };

template <typename T, std::size_t Index>
struct indexed_access<XRect<T>, min_corner, Index>
{
    static inline typename coordinate_type<XPoint<T>>::type get(XRect<T> const& box)
    {
        return access<XPoint<T>, Index>::get(box.min_corner());
    }
};

template <typename T, std::size_t Index>
struct indexed_access<XRect<T>, max_corner, Index>
{
    static inline typename coordinate_type<XPoint<T>>::type get(XRect<T> const& box)
    {
        return access<XPoint<T>, Index>::get(box.max_corner());
    }
};

}}} // namespace boost::geometry::traits


/////////////////////////////////////////////
int main()
{
    typedef std::pair<XPoint<double>, int> XPtPair;
    typedef bgi::rtree<std::pair<XPoint<double>, int>, bgi::quadratic<12>> PtRTree;

    PtRTree  ptTree;
    ptTree.insert({XPoint(0.0, 0.0), 0});
    ptTree.insert({XPoint(1.0, 1.0), 1});
    ptTree.insert({XPoint(10.0, 10.0), 2});
    
    ptTree.insert({XPoint(12.0, 0.0), 3});
    ptTree.insert({XPoint(0.0, 12.0), 4});

    std::cout<<"------ XPoint ------"<<std::endl;
    std::cout<<"--- intersects\n";
    auto qPt = XPoint(10.0, 10.0);
    auto it = ptTree.qbegin(bgi::intersects(qPt));
    for (; it != ptTree.qend(); ++it)
    {
        std::cout<<(*it).second<<" x:"<<(*it).first.getX()<<" y:"<<(*it).first.getY()<<std::endl;

        std::cout<<"\n";
    }

    std::cout<<"--- nearest\n";
    auto nPt = XPoint(12.0, 10.0);
    // std::vector<decltype(qPt)> vecRes;

    // ---查找5个离点最近的索引
    std::vector< XPtPair > vecPair;
	ptTree.query(bgi::nearest(nPt, 2), std::back_inserter(vecPair));
    for(auto& v : vecPair)
    {
        std::cout<<v.second<<" x:"<<v.first.getX()<<" y:"<<v.first.getY()<<std::endl;
    }

    std::cout<<std::endl<<"------ XRect------"<<std::endl;

    //////////////////////////////////////////
    typedef bgi::rtree<XRect<double>, bgi::quadratic<16>> RTree;
    typedef std::pair<XRect<double>, int> XRectPair;
    typedef bgi::rtree<XRectPair, bgi::quadratic<12>> XRectTree;

    RTree rectTree;
    XRectTree xrectTree;

    XPoint<double> ptA{0.0, 0.0};
    XPoint<double> ptB{10.0, 10.0};
    XRect<double> rectA(ptA, ptB);

    // rectTree.insert(rectA);
    auto pA = std::make_pair(rectA, 1);
    xrectTree.insert(pA);

    xrectTree.insert({XRect<double>(XPoint<double>(8, 8),XPoint<double>(15, 15)), 2});

    auto ptQuery = XPoint<double>(3.0, 3.0);
    auto rectQuery = XRect<double>(XPoint<double>(11.0, 11.0), XPoint<double>(12.0, 12.0));

    auto iterA = xrectTree.qbegin(bgi::intersects(ptQuery));
    for(; iterA != xrectTree.qend(); iterA++)
    {
        auto f = (*iterA).first;
        auto s = (*iterA).second;
        std::cout<<s<<" ptMin:"<<f.min_corner().getX()<<" y:"<<f.min_corner().getX()<<std::endl;
        std::cout<<s<<" ptMax:"<<f.max_corner().getX()<<" y:"<<f.max_corner().getX()<<std::endl;
        std::cout<<"------"<<std::endl;
    }

    auto iterB = xrectTree.qbegin(bgi::intersects(rectQuery));
    for(; iterB != xrectTree.qend(); iterB++)
    {
        auto f = (*iterB).first;
        auto s = (*iterB).second;
        std::cout<<s<<" ptMin:"<<f.min_corner().getX()<<" y:"<<f.min_corner().getX()<<std::endl;
        std::cout<<s<<" ptMax:"<<f.max_corner().getX()<<" y:"<<f.max_corner().getX()<<std::endl;
        std::cout<<"------"<<std::endl;
    }
    return 0;
}