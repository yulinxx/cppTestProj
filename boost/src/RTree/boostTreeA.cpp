// Boost库-功能介绍-Geometry-R树-空间索引_插件开发的博客-CSDN博客
// https://blog.csdn.net/m0_67316550/article/details/124464921

// R树是一种多级平衡树,它是B树在多维空间上的扩展。在R树中存放的数据并不是原始数据，而是这些数据的最小边界矩形（MBR   minimum bounding rectangle），
// 空间对象的MBR被包含于R树的叶结点中。
// 在R树空间索引中，设计一些虚拟的矩形目标，将一些空间位置相近的目标，包含在这个矩形内，这些虚拟的矩形作为空间索引，它含有所包含的空间对象的指针。

// 虚拟矩形还可以进一步细分，即可以再套虚拟矩形形成多级空间索引。
//   R+树，在R树的构造中，要求虚拟矩形一般尽可能少地重叠，并且一个空间对通常仅被一个虚拟矩形所包含。
// 但空间对象千姿百态，它们的最小矩形范围经常重叠。

// R+ 改进R树的空间索引，为了平衡，它允许虚拟矩形相互重叠，并允许一个空间目标被多个虚拟矩形所包含。
// 在Boost.Geometry中有R树的实现，它依赖Boost.Container, Boost.Core, Boost.Move, Boost.MPL, Boost.Range, Boost.Tuple.这些库。
// R树的元素都是box（矩形）和整数索引值。R树的实现在Geometry中被很好封装，如果使用它，最主要的需要掌握它的查询技巧。

#include <boost/geometry.hpp>
#include <boost/geometry/core/point_type.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/assign.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>
#include <vector>
#include <iostream>

// https://blog.csdn.net/ljp341621/article/details/119684904
// Boost.Geometry中已支持的可索引几何要素有:
// 点(Point)
// 外包(Box)
// 线段(Segment)

// 单纯Point、Segment、Box构建RTree
/*
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point_xy<double> Point2d;
typedef bg::model::box<Point2d> Box2d;
typedef bg::model::segment<Point2d> Segment2d;

bgi::rtree<Point2d, boost::geometry::index::quadratic<16>> RTree_P_2;
bgi::rtree<Box2d, boost::geometry::index::quadratic<16>> RTree_B_2;
bgi::rtree<Segment2d, boost::geometry::index::quadratic<16>> RTree_S_2;
*/

// 由Point、Segment、Box结合自定义数据构建RTree
/*
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;


typedef bg::model::point_xy<double> Point2d;
typedef bg::model::box<Point2d> Box2d;
typedef bg::model::segment<Point2d> Segment2d;

typedef std::pair<Point2d, size_t> PointValuePair;//点+自定义数据的索引
typedef std::pair<Box2d, size_t> BoxValuePair;//外包+自定义数据的索引
typedef std::pair<Segment2d, size_t> SegmentValuePair;//线段+自定义数据的索引

bgi::rtree<PointValuePair, boost::geometry::index::quadratic<16>> RTree_P_2;
bgi::rtree<BoxValuePair, boost::geometry::index::quadratic<16>> RTree_B_2;
bgi::rtree<SegmentValuePair, boost::geometry::index::quadratic<16>> RTree_S_2;

*/

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

// 这里定义点为2维的笛卡尔点 bg::cs::cartesian
typedef bg::model::d2::point_xy<double, boost::geometry::cs::cartesian> DPoint; //双精度的点
typedef bg::model::box<DPoint> DBox;											//矩形
typedef std::pair<DBox, unsigned> BoxValue;

int main()
{
	// 创建R树 linear quadratic rstar三种算法
	// quadratic	英[kwɒˈdrætɪk] 美[kwɑːˈdrætɪk]	平方的; 二次方的;

	bgi::rtree<BoxValue, bgi::quadratic<16>> rtree; //采用quadratic algorithm，节点中元素个数最多16个

	// 填充元素
	for (unsigned i = 0; i < 10; ++i)
	{
		DBox b(DPoint(i + 0.0f, i + 0.0f), DPoint(i + 0.5f, i + 0.5f));

		rtree.insert(std::make_pair(b, i)); // r树插入外包围矩形 i为索引
	}

	// ---查询与矩形相交的矩形索引
	// https://www.boost.org/doc/libs/1_58_0/libs/geometry/doc/html/geometry/spatial_indexes/queries.html
	DBox query_box(DPoint(0, 0), DPoint(5, 5));
	std::vector<BoxValue> result_s;
	rtree.query(bgi::intersects(query_box), std::back_inserter(result_s));

	// 显示值
	std::cout << std::endl
			  << "spatial query box:" << std::endl;
	std::cout << bg::wkt<DBox>(query_box) << std::endl;
	std::cout << "spatial query result:" << std::endl;

	BOOST_FOREACH (BoxValue const &v, result_s)
		std::cout << bg::wkt<DBox>(v.first) << " - " << v.second << std::endl;

	// ---查找5个离点最近的索引
	std::vector<BoxValue> result_n;
	rtree.query(bgi::nearest(DPoint(0, 0), 5), std::back_inserter(result_n));

	std::cout << std::endl
			  << "knn query point:" << std::endl;
	std::cout << bg::wkt<DPoint>(DPoint(0, 0)) << std::endl;
	std::cout << "knn query result:" << std::endl;
	BOOST_FOREACH (BoxValue const &v, result_n)
		std::cout << bg::wkt<DBox>(v.first) << " - " << v.second << std::endl;

	return 0;
}




// #include <boost/geometry.hpp>
// #include <boost/geometry/geometry.hpp>
// #include <string>

// using namespace boost;
// typedef bg::model::point<double, 2, bg::cs::cartesian> point;
// typedef bg::model::box<point> box;
// typedef std::pair<box, std::pair<int, int> > edgesValue;

// struct Item;
// using ItemRelation = Item*;

// struct Item {
//     double x, y;
//     int id;
//     std::vector<ItemRelation> _rel;
//     std::vector<ItemRelation> getRelations() const { return {}; }
// };

// int main() {
//     Item item1  { 1, 2, 1, {} },
//          item2  { 1, 4, 2, {} },
//          item3  { 1, 6, 2, {} },
//          item4  { 2, 1, 4, {} },
//          item5  { 2, 3, 5, {} },
//          item6  { 2, 5, 5, {} };

//     std::vector<Item*> items { &item1, &item2, &item3, &item4, &item5, &item6, };

//     item3._rel = { &item1, &item2 };
//     item6._rel = { &item4, &item5 };

//     bgi::rtree<edgesValue, bgi::rstar<16> > edgesTree;

//     for (size_t i = 0; i < items.size(); i++) {
//         Item *item = items.at(i);
//         std::vector<ItemRelation> relations = item->getRelations();

//         point ps1(item->x, item->y);
//         Item *relatedItem;
//         for (size_t j = 0; j < relations.size(); j++) {
//             relatedItem = relations.at(j);
//             point ps2(relatedItem->x, relatedItem->y);
//             box bounds(ps1, ps2);
//             edgesTree.insert(std::make_pair(bounds, std::make_pair(item->id, relatedItem->id)));
//         }
//     }
// }




#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

#include <cmath>
#include <vector>
#include <iostream>
#include <boost/foreach.hpp>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

template <typename Container>
class my_indexable
{
    typedef typename Container::size_type size_t;
    typedef typename Container::const_reference cref;
    Container const &container;

public:
    typedef cref result_type;
    explicit my_indexable(Container const &c) : container(c) {}
    result_type operator()(size_t i) const { return container[i]; }
};

int mainx()
{
    typedef bg::model::point<float, 2, bg::cs::cartesian> point;
    typedef bg::model::box<point> box;
    typedef std::vector<box>::size_type value;
    typedef bgi::rstar<16, 4> parameters;
    typedef my_indexable<std::vector<box>> indexable_getter;

    // boxes
    std::vector<box> boxes;

    // create some boxes
    for (unsigned i = 0; i < 10; ++i)
    {
        // add a box
        boxes.push_back(box(point(i + 0.0f, i + 0.0f), point(i + 0.5f, i + 0.5f)));
    }

    // display boxes
    std::cout << "generated boxes:" << std::endl;
    BOOST_FOREACH (box const &b, boxes)
        std::cout << bg::wkt<box>(b) << std::endl;

    // create the rtree
    parameters params;
    indexable_getter ind(boxes);
    bgi::rtree<value, parameters, indexable_getter> rtree(params, ind);

    // fill the spatial index
    for (size_t i = 0; i < boxes.size(); ++i)
        rtree.insert(i);

    // find values intersecting some area defined by a box
    box query_box(point(0, 0), point(5, 5));
    std::vector<value> result_s;
    rtree.query(bgi::intersects(query_box), std::back_inserter(result_s));

    // find 5 nearest values to a point
    std::vector<value> result_n;
    rtree.query(bgi::nearest(point(0, 0), 5), std::back_inserter(result_n));

    // note: in Boost.Geometry the WKT representation of a box is polygon

    // display results
    std::cout << "spatial query box:" << std::endl;
    std::cout << bg::wkt<box>(query_box) << std::endl;
    std::cout << "spatial query result:" << std::endl;
    BOOST_FOREACH (value i, result_s)
        std::cout << bg::wkt<box>(boxes[i]) << std::endl;

    std::cout << "knn query point:" << std::endl;
    std::cout << bg::wkt<point>(point(0, 0)) << std::endl;
    std::cout << "knn query result:" << std::endl;
    BOOST_FOREACH (value i, result_n)
        std::cout << bg::wkt<box>(boxes[i]) << std::endl;

    return 0;
}