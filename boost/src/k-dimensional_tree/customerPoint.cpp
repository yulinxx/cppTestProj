#include <iostream>
#include <vector>
#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

/////////////////////////////////////////////
// 自定义的点类
template<typename T>
class XPoint{
private:
public:
    T x;
    T y;

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
};


/////////////////////////////////////////////
// 将自定义的点,转成Boost可识别的点数据结构
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


    std::cout<<"------------intersects\n";
    auto qPt = XPoint(10.0, 10.0);
    auto it = ptTree.qbegin(bgi::intersects(qPt));
    for (; it != ptTree.qend(); ++it)
    {
        std::cout<<(*it).second<<" x:"<<(*it).first.getX()<<" y:"<<(*it).first.getY()<<std::endl;

        std::cout<<"\n";
    }

    std::cout<<"------------nearest\n";
    auto nPt = XPoint(12.0, 10.0);
    // std::vector<decltype(qPt)> vecRes;

    // ---查找5个离点最近的索引
    std::vector< XPtPair > vecPair;
	ptTree.query(bgi::nearest(nPt, 2), std::back_inserter(vecPair));
    for(auto& v : vecPair)
    {
        std::cout<<v.second<<" x:"<<v.first.getX()<<" y:"<<v.first.getY()<<std::endl;
    }

    return 0;
}