#### 在项目中要用到计算几何的东西，计算三维空间中面片与六面体的相交判断， 通过各种搜索发现boost库中的Geometry模块还不错，可以比较容易地实现。这里记录一下这个库的基本情况。
https://blog.csdn.net/yanfeng1022/article/details/100000741

### 1、常见几何对象
```cpp
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point_xy.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/multi_point.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/geometry/geometries/polygon.hpp>
#include <boost/geometry/geometries/multi_polygon.hpp>
#include <boost/geometry/geometries/linestring.hpp>
#include <boost/geometry/geometries/multi_linestring.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/ring.hpp>
#include <boost/geometry/geometries/variant.hpp>
``` 

Boost.Geometry的model有point_xy,  point, multi_point, ，segment，linestring，multi_linestring, box，ring，polygon，multi_polygon, variant.

model::point
model::d2::point_xy
model::linestring
model::polygon
model::multi_point
model::multi_linestring

model::multi_polygon
model::box，
model::ring
model::segment
model::referring_segment
 
### 2、常见算法

提供的算法有：面积、长度、周长、质心、凸壳、交集（剪裁）、内（多边形中的点）、距离、包络线（边界框）、简化、变换等。

area
assign
append
buffer

centroid
clear
convert
convex_hull

correct
covered_by
crosses
densify

difference
discrete_frechet_distance
discrete_hausdorff_distance

disjoint
distance
envelope

equals
expand
for_each
intersection
intersects

is_empty
is_simple
is_valid
length
make

num_geometries
num_interior_rings
num_points
num_segments

overlaps
perimeter
relate
relation
reverse

simplify
sym_difference
touches
transform

union_
unique
within

常见的有以下几种：

计算面积
Boost::Geometry::area(obj1)

计算距离
Boost::Geometry::distance(obj1, obj2)

判断是否相交
Boost::Geometry::intersects(obj1, obj2)

计算交点
Boost::Geometry::intersection(obj1, obj2, result)

判断是否在box内
Boost::Geometry::within(obj1, obj2)

### 3、boost.Geometry可以与VS的GraphicalDebugging插件可以配合使用，查看几何图形。

在调试过程中插入断点，通过在GraphicalDebugging界面输入对象的名字就可以查看几何对象的形态。在视图中的其他窗口可以找到刚刚安装的插件的窗口，Geometry Watch，Graphical Watch，Plot Watch。

 

### 4、几何图形的输入输出

DSV (Delimiter-Separated Values)

WKT (Well-Known Text)

SVG (Scalable Vector Graphics)

前两种是文本格式数据流，第三种是图形化输入和输出。

 

### 5、几何图形的算术运算

add_point
add_value

assign_point
assign_value

cross_product
cross_product

divide_point
divide_value

dot_product

multiply_point
multiply_value

subtract_point
subtract_value


### 6、官方示例
https://www.boost.org/doc/libs/1_69_0/libs/geometry/doc/html/geometry/spatial_indexes/rtree_examples/index_of_polygons_stored_in_vector.html
 