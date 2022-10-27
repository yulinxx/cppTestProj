#### Boost.Geometry中常用数据结构的定义

```cpp
namespace bg=boost::geometry;

typedef bg::model::point_xy<double> Point2d;
typedef bg::model::multi_point<Point2d> Point2dArray;
typedef bg::model::box<Point2d> Box2d;
typedef bg::model::segment<Point2d> Segment2d;
typedef bg::model::linestring<Point2d> Linestring2d;
typedef bg::model::multi_linestring<Linestring2d> Linestring2dArray;
typedef bg::model::ring<Point2d,false,false> Rind2d;
typedef bg::model::polygon<Point2d,false,false> Polygon2d;
typedef bg::moddel::multi_polygon<Polygon2d> Polygon2dArray;
```

Boost.Geometry的model有  
point_xy,  point, multi_point, segment，  
linestring，multi_linestring, box，ring，polygon，multi_polygon, variant.  
- model::point 
- model::d2::point_xy
- model::linestring
- model::polygon
  
- model::multi_point
- model::multi_linestring
- model::multi_polygon
  
- model::box
- model::ring
- model::segment
- model::referring_segment
