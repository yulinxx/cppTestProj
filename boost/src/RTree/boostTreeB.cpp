///////////////////////////////////////////////////////////////////////////////////////////////
// 一个多边形构建R树的例子
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/foreach.hpp>

#include <cmath>
#include <vector>
#include <iostream>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

typedef bg::model::point<double, 2, bg::cs::cartesian> DPoint;
typedef bg::model::box<DPoint> DBox;
typedef bg::model::polygon<DPoint, false, false> DPolygon; // ccw, open polygon
typedef std::pair<DBox, unsigned> DValue;

int main()
{
	std::vector<DPolygon> polygons;

	//构建多边形
	for (unsigned i = 0; i < 10; ++i)
	{
		//创建多边形
		DPolygon polg;
		for (float a = 0; a < 6.28316f; a += 1.04720f)
		{
			float x = i + int(10 * ::cos(a)) * 0.1f;
			float y = i + int(10 * ::sin(a)) * 0.1f;
			polg.outer().push_back(DPoint(x, y));
		}

		//插入
		polygons.push_back(polg);
	}

	//打印多边形值
	std::cout << "--- generated polygons:" << std::endl;
	BOOST_FOREACH (DPolygon const &p, polygons)
		std::cout << bg::wkt<DPolygon>(p) << std::endl;

	std::cout<<"----------------------"<<std::endl;

	//创建R树
	bgi::rtree<DValue, bgi::rstar<16, 4>> rtree; //最大最小

	//计算多边形包围矩形并插入R树
	for (unsigned i = 0; i < polygons.size(); ++i)
	{
		//计算多边形包围矩形
		DBox b = bg::return_envelope<DBox>(polygons[i]);
		//插入R树
		rtree.insert(std::make_pair(b, i));
	}

	//按矩形范围查找
	DBox query_box(DPoint(0, 0), DPoint(5, 5));
	std::vector<DValue> result_s;
	rtree.query(bgi::intersects(query_box), std::back_inserter(result_s));

	// 5个最近点
	std::vector<DValue> result_n;
	rtree.query(bgi::nearest(DPoint(0, 0), 5), std::back_inserter(result_n));

	// note: in Boost.Geometry the WKT representation of a box is polygon
	// note: the values store the bounding boxes of polygons
	// the polygons aren't used for querying but are printed

	// display results
	std::cout << "--- spatial query box:" << std::endl;
	std::cout << bg::wkt<DBox>(query_box) << std::endl;

	std::cout<<"----------------------"<<std::endl;

	std::cout << "--- spatial query result:" << std::endl;
	BOOST_FOREACH (DValue const &v, result_s)
		std::cout << bg::wkt<DPolygon>(polygons[v.second]) << std::endl;

	std::cout<<"----------------------"<<std::endl;
	std::cout << "--- knn query point:" << std::endl;
	std::cout << bg::wkt<DPoint>(DPoint(0, 0)) << std::endl;

	std::cout<<"----------------------"<<std::endl;
	std::cout << "knn query result:" << std::endl;
	BOOST_FOREACH (DValue const &v, result_n)
		std::cout << bg::wkt<DPolygon>(polygons[v.second]) << std::endl;

	std::cout<<std::endl;
	return 0;
}

/*
--- generated polygons:
POLYGON((1 0,0.4 0.8,-0.5 0.8,-1 0,-0.4 -0.8,0.5 -0.8,1 0))
POLYGON((2 1,1.4 1.8,0.5 1.8,0 1,0.6 0.2,1.5 0.2,2 1))
POLYGON((3 2,2.4 2.8,1.5 2.8,1 2,1.6 1.2,2.5 1.2,3 2))
POLYGON((4 3,3.4 3.8,2.5 3.8,2 3,2.6 2.2,3.5 2.2,4 3))
POLYGON((5 4,4.4 4.8,3.5 4.8,3 4,3.6 3.2,4.5 3.2,5 4))
POLYGON((6 5,5.4 5.8,4.5 5.8,4 5,4.6 4.2,5.5 4.2,6 5))
POLYGON((7 6,6.4 6.8,5.5 6.8,5 6,5.6 5.2,6.5 5.2,7 6))
POLYGON((8 7,7.4 7.8,6.5 7.8,6 7,6.6 6.2,7.5 6.2,8 7))
POLYGON((9 8,8.4 8.8,7.5 8.8,7 8,7.6 7.2,8.5 7.2,9 8))
POLYGON((10 9,9.4 9.8,8.5 9.8,8 9,8.6 8.2,9.5 8.2,10 9))
----------------------
--- spatial query box:
POLYGON((0 0,0 5,5 5,5 0,0 0))
----------------------
--- spatial query result:
POLYGON((1 0,0.4 0.8,-0.5 0.8,-1 0,-0.4 -0.8,0.5 -0.8,1 0))
POLYGON((2 1,1.4 1.8,0.5 1.8,0 1,0.6 0.2,1.5 0.2,2 1))
POLYGON((3 2,2.4 2.8,1.5 2.8,1 2,1.6 1.2,2.5 1.2,3 2))
POLYGON((4 3,3.4 3.8,2.5 3.8,2 3,2.6 2.2,3.5 2.2,4 3))
POLYGON((5 4,4.4 4.8,3.5 4.8,3 4,3.6 3.2,4.5 3.2,5 4))
POLYGON((6 5,5.4 5.8,4.5 5.8,4 5,4.6 4.2,5.5 4.2,6 5))
----------------------
--- knn query point:
POINT(0 0)
----------------------
knn query result:
POLYGON((5 4,4.4 4.8,3.5 4.8,3 4,3.6 3.2,4.5 3.2,5 4))
POLYGON((4 3,3.4 3.8,2.5 3.8,2 3,2.6 2.2,3.5 2.2,4 3))
POLYGON((3 2,2.4 2.8,1.5 2.8,1 2,1.6 1.2,2.5 1.2,3 2))
POLYGON((1 0,0.4 0.8,-0.5 0.8,-1 0,-0.4 -0.8,0.5 -0.8,1 0))
POLYGON((2 1,1.4 1.8,0.5 1.8,0 1,0.6 0.2,1.5 0.2,2 1))
*/