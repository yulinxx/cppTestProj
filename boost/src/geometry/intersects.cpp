#include <iostream>

#include <boost/assign.hpp>
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/polygon.hpp>

namespace bg = boost::geometry;

typedef bg::model::d2::point_xy<double> DPoint;
typedef bg::model::segment<DPoint> DSegment;
typedef bg::model::linestring<DPoint> DLineString;
typedef bg::model::box<DPoint> DBox;

//这里的ring就是我们通常说的多边形闭合区域(内部不存在缕空)，模板参数为true，表示顺时针存储点，为false，表示逆时针存储点，
// 由于MM_TEXT坐标系与传统上的坐标系的Y轴方向是相反的，所以最后为false，将TopLeft、TopRight、BottomRight、BottomLeft、TopLeft以此存储到ring中，以便能正确计算
typedef bg::model::ring<DPoint, false> DRing;

//polygon模板参数false，也是由上面相同的原因得出来的
typedef bg::model::polygon<DPoint, false> DPolygon;

int main()
{
    DPoint pt0(100, 100);
    DPoint pt1(200, 200);
    DSegment sg0(pt0, pt1);

    double dDistance = 0;

    //1、点到点的距离
    dDistance = bg::distance(pt0, pt1);
    std::cout << "Distance:" << dDistance << std::endl;

    //2、点到线段的距离，如果点到直线的垂足不在线段上，所计算的距离为(点到直线的距离、线段到垂足延长的距离之和）
    dDistance = bg::distance(DPoint(200, 100), sg0);
    std::cout << "Distance:" << dDistance << std::endl;
    dDistance = bg::distance(DPoint(100, 0), sg0);
    std::cout << "Distance:" << dDistance << std::endl;

    //3、判断线段是否相交
    DSegment sg1(DPoint(0, 100), DPoint(100, 0));
    DSegment sg2(DPoint(100, 200), DPoint(200, 100));
    bool bIntersect = false;
    bIntersect = bg::intersects(sg0, sg1);
    std::cout << "Intersect:" << bIntersect << std::endl;

    bIntersect = bg::intersects(sg0, sg2);
    std::cout << "Intersect:" << bIntersect << std::endl;

    //4、求线段与线段的交点
    std::list<DPoint> lstPoints;
    bg::intersection(sg0, sg1, lstPoints);
    lstPoints.clear();
    bg::intersection(sg0, sg2, lstPoints);

    DBox rc2(DPoint(0, 0), DPoint(0, 0));

    //5、判断box是否相交
    DBox rc(DPoint(0, 0), DPoint(200, 200));
    DBox rc0(DPoint(250, 250), DPoint(450, 450));
    DBox rc1(DPoint(100, 100), DPoint(300, 300));

    bIntersect = bg::intersects(rc, rc0);
    bIntersect = bg::intersects(rc, rc1);
    //bg::intersection(rc, rc0, container);//error

    //6、判断box是否与LineString相交
    DLineString line0;

    line0.push_back(DPoint(10, 250));
    line0.push_back(DPoint(100, 100));
    line0.push_back(DPoint(120, -10));
    line0.push_back(DPoint(210, 200));
    bIntersect = bg::intersects(rc, line0);
    bIntersect = bg::intersects(rc0, line0);

    //7、求box与linestring的交点
    std::list<DLineString> lstLines;
    bg::intersection(rc, line0, lstLines);

    //8、点是否在box内
    DBox rc7(DPoint(0, 0), DPoint(100, 100));
    bool bInside = false;
    bInside = bg::within(DPoint(50, 50), rc7);
    bInside = bg::within(DPoint(0, 0), rc7);

    //9、判断LineString与LineString是否相交
    DLineString line1, line2, line3;

    line1.push_back(DPoint(50, 50));
    line1.push_back(DPoint(150, 50));
    line1.push_back(DPoint(50, 200));
    line1.push_back(DPoint(150, 200));
    line2.push_back(DPoint(100, 0));
    line2.push_back(DPoint(70, 100));
    line2.push_back(DPoint(150, 210));
    line3.push_back(DPoint(200, 0));
    line3.push_back(DPoint(200, 200));

    bIntersect = bg::intersects(line1, line2);
    bIntersect = bg::intersects(line1, line3);

    //10、求LineString与LineString的交点
    lstPoints.clear();
    bg::intersection(line1, line2, lstPoints);
    lstPoints.clear();
    bg::intersection(line1, line3, lstPoints);

    //11、判断ring与ring是否相交
    DPoint arDPoint0[6] = { DPoint(0, 0), DPoint(100, 0), DPoint(200, 100), DPoint(100, 200), DPoint(0, 200), DPoint(0, 0) };
    DPoint arDPoint1[6] = { DPoint(100, 100), DPoint(200, 0), DPoint(300, 0), DPoint(300, 200), DPoint(200, 200), DPoint(100, 100) };
    DRing r0(arDPoint0, arDPoint0 + 6);
    DRing r1(arDPoint1, arDPoint1 + 6);
    bIntersect = bg::intersects(r0, r1);

    //12、求ring与ring的交点
    lstPoints.clear();
    bg::intersection(r0, r1, lstPoints);

    DPolygon poly1;
    DPolygon poly2;
    DPolygon poly3;

    auto lstOf = boost::assign::list_of(DPoint(0, 0))(DPoint(200, 0))(DPoint(200, 200))(DPoint(0, 200))(DPoint(0, 0));
    poly1.outer().assign(lstOf.begin(), lstOf.end());
    lstOf = boost::assign::list_of(DPoint(50, 50))(DPoint(150, 50))(DPoint(150, 150))(DPoint(50, 150))(DPoint(50, 50));
    poly1.inners().push_back(lstOf);
    lstOf = boost::assign::list_of(DPoint(100, 0))(DPoint(120, 0))(DPoint(120, 200))(DPoint(100, 200))(DPoint(100, 0));
    poly2.outer().assign(lstOf.begin(), lstOf.end());
    lstOf = boost::assign::list_of(DPoint(100, 60))(DPoint(120, 60))(DPoint(120, 140))(DPoint(100, 140))(DPoint(100, 60));
    poly3.outer().assign(lstOf.begin(), lstOf.end());

    //13、判断polygon与polygon是否相交
    bIntersect = bg::intersects(poly1, poly2);
    bIntersect = bg::intersects(poly1, poly3);

    //14、求polygon与polygon相交的区域
    std::list<DPolygon> lstPolygon;

    bg::intersection(poly1, poly2, lstPolygon);
    lstPolygon.clear();
    bg::intersection(poly1, poly3, lstPolygon);

    //15、判断点是否在polygon内
    bInside = bg::within(DPoint(100, 100), poly1);
    bInside = bg::within(DPoint(25, 25), poly1);

    return 0;
}