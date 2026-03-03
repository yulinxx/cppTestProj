安装:
x@x:~/Install/vcpkg$ ./vcpkg  install cgal:x64-linux

使用:
find_package(CGAL CONFIG REQUIRED)
target_link_libraries(main PRIVATE CGAL::CGAL)

说明:
https://doc.cgal.org/latest/Manual/index.html
CGAL 是一个开源的计算几何 C++ 库.它由 CGAL 项目组开发，该项目组由来自世界各地的数学家、计算机科学家和工程师组成.CGAL 的目标是提供一个功能齐全的计算几何库，可用于研究、教育和应用.
CGAL 提供广泛的功能，包括：

* 几何对象的表示和操作
* 几何算法
* 几何数据结构
* 几何可视化

CGAL 被广泛用于研究、教育和应用.它被使用在许多不同的领域，包括：

* 计算机图形学
* 计算机辅助设计
* 计算机视觉
* 机器学习
* 数学
* 物理学

CGAL 是一个强大的工具，可用于进行复杂的计算几何计算.它是研究、教育和应用的宝贵资源.


我来为您介绍 CGAL（Computational Geometry Algorithms Library，计算几何算法库）。

## CGAL 简介

**CGAL** 是当今最权威、最全面的**计算几何开源 C++ 库**，由 CGAL 开源项目社区维护，最初由欧洲几所大学联合开发。它提供了大量稳健、高效的几何算法和数据结构。

---

## 核心特性

| 特性 | 说明 |
|------|------|
| **稳健性** | 支持精确算术（exact arithmetic），避免浮点精度问题 |
| **丰富性** | 涵盖 2D/3D/高维几何的几乎所有基础算法 |
| **高性能** | 经过高度优化，支持大规模数据处理 |
| **模块化** | 按功能分为多个包（packages），按需使用 |
| **标准兼容** | 基于 C++14/17，与 STL/Boost 良好集成 |

---

## 主要模块

### 1. **几何内核（Kernel）**
- 基础几何对象：点、向量、线段、射线、直线、圆、多边形、多面体等
- 支持多种表示：笛卡尔坐标、齐次坐标
- 提供精确数类型（如 `CGAL::Exact_predicates_inexact_constructions_kernel`）

### 2. **凸包与极值（Convex Hull）**
- 2D/3D 凸包计算（Graham 扫描、QuickHull、增量算法等）
- 极值点、直径计算

### 3. **三角剖分（Triangulations）**
- **2D 三角剖分**：Delaunay、约束 Delaunay、正则三角剖分
- **3D 三角剖分**：3D Delaunay、正则三角剖分（用于网格生成）
- **曲面网格**：基于曲面的三角剖分

### 4. **多边形与多面体（Polygons & Polyhedra）**
- 布尔运算（并、交、差）
- 偏移、缓冲、骨架提取
- 多边形分割、三角化

### 5. **网格生成（Mesh Generation）**
- 2D 约束三角剖分网格
- 3D 表面网格生成（基于 Delaunay 细化）
- 3D 体网格生成（四面体网格）

### 6. **几何处理（Geometry Processing）**
- 曲面重建（泊松重建、 advancing front）
- 网格简化、细分、变形、参数化
- 形状分析、分割

### 7. **搜索结构（Spatial Searching）**
- KD-Tree、范围树、区间树
- 最近邻搜索、范围查询

### 8. **排列与分割（Arrangements & Partitioning）**
- 2D 排列（线段、圆弧、代数曲线的排列）
- 多边形分割为凸片或单调片

### 9. **Voronoi 图**
- 2D/3D Voronoi 图
- 加权 Voronoi、Apollonius 图

### 10. **直线与凸包（Polyline Simplification & Skeleton）**
- 多边形直骨架（Straight Skeleton）
- 中轴变换

---

## 典型应用场景

- **计算机图形学**：网格处理、曲面重建、碰撞检测
- **CAD/CAM**：布尔运算、偏移曲线、数控加工路径规划
- **GIS**：地形分析、空间索引、地图叠加分析
- **机器人**：运动规划、可见性计算、构型空间
- **有限元分析（FEM）**：高质量网格生成
- **分子生物学**：分子表面建模、蛋白质结构分析

---

## 代码示例

```cpp
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef CGAL::Delaunay_triangulation_2<K> Delaunay;
typedef K::Point_2 Point;

int main() {
    std::vector<Point> points = {
        Point(0, 0), Point(1, 0), Point(0, 1), Point(1, 1), Point(0.5, 0.5)
    };
    
    Delaunay dt;
    dt.insert(points.begin(), points.end());
    
    // 遍历所有面
    for (auto face = dt.finite_faces_begin(); face != dt.finite_faces_end(); ++face) {
        // 处理每个三角形面
    }
    
    return 0;
}
```

---

## 许可证与获取

- **开源协议**：双重许可（GPL/LGPL + 商业许可）
  - 开源项目可使用 LGPL/GPL 免费使用
  - 商业应用可购买商业许可
- **官网**：[www.cgal.org](https://www.cgal.org)
- **GitHub**：github.com/CGAL/cgal
- **文档**：详尽的官方文档与教程

---

## 学习建议

1. **先掌握几何内核概念**：理解 Kernel、Traits 的设计模式
2. **从 2D 算法入手**：Delaunay 三角剖分、凸包是最佳入门点
3. **注意数类型选择**：根据精度需求选择 `Exact_predicates_*` 或 `Exact_constructions_*`
4. **结合示例学习**：CGAL 自带大量示例代码（`examples/` 目录）

CGAL 是计算几何领域的**事实标准**。


### CGAL 和 Boost.Geometry 对比：虽然都处理几何计算，但设计理念和适用场景有显著差异。

---

## 核心定位对比

| 维度 | **CGAL** | **Boost.Geometry** |
|------|----------|-------------------|
| **设计目标** | 计算几何**算法库** | 计算几何**泛型库** |
| **核心优势** | 算法丰富度、数值稳健性 | 标准兼容性、轻量级、易集成 |
| **架构哲学** | 提供完整解决方案 | 提供可扩展的泛型框架 |
| **成熟度** | 1995年开始，专注几何20+年 | 2009年并入Boost，相对年轻 |

---
。
## 详细差异分析

### 1. **算法覆盖度**

| 功能 | CGAL | Boost.Geometry |
|------|------|----------------|
| **基础几何** | ✅ 完整（2D/3D/高维） | ✅ 完整（主要2D，部分3D） |
| **三角剖分** | ✅ Delaunay、约束、3D、曲面 | ❌ 无内置 |
| **网格生成** | ✅ 2D/3D 表面与体网格 | ❌ 无 |
| **凸包** | ✅ 2D/3D/高维，多种算法 | ✅ 2D Graham扫描 |
| **布尔运算** | ✅ 多边形/多面体布尔运算 | ✅ 多边形裁剪（基于Clipper） |
| **Voronoi图** | ✅ 2D/3D | ❌ 无 |
| **曲面重建** | ✅ 泊松、Advancing Front | ❌ 无 |
| **空间搜索** | ✅ KD-Tree、范围树等 | ✅ R-Tree（主要） |
| **排列计算** | ✅ 2D曲线排列 | ❌ 无 |
| **直骨架/中轴** | ✅ 完整支持 | ❌ 无 |

**结论**：CGAL 在**复杂算法**（三角剖分、网格生成、曲面处理）上遥遥领先；Boost.Geometry 专注**基础2D几何操作**。

---

### 2. **数值稳健性**

| 特性 | CGAL | Boost.Geometry |
|------|------|----------------|
| **精确算术** | ✅ 原生支持（GMP/MPFR集成） | ⚠️ 依赖用户实现或第三方 |
| **鲁棒性内核** | ✅ 多种Kernel策略（EPIC、EPEC等） | ❌ 主要使用浮点，需自行处理 |
| **退化情况处理** | ✅ 系统化解决 | ⚠️ 部分算法有限支持 |

**CGAL 的核心优势**：通过**区间算术**和**精确数类型**从根本上解决浮点误差问题，这是计算几何的硬需求。

---

### 3. **架构与设计**

#### CGAL 的架构特点
```cpp
// CGAL：基于Kernel的策略模式
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point;  // 几何类型与Kernel绑定

// 算法与数据分离，通过Traits配置
CGAL::Delaunay_triangulation_2<K> dt;
```
- **重型设计**：算法通常包含复杂内部状态
- **策略模式**：通过模板参数配置算法行为
- **独立包结构**：各模块可独立使用，但依赖关系复杂

#### Boost.Geometry 的架构特点
```cpp
// Boost.Geometry：基于Concept的泛型编程
#include <boost/geometry.hpp>
namespace bg = boost::geometry;

struct MyPoint { double x, y; };  // 自定义类型

// 通过traits特化适配自定义类型
namespace boost::geometry::traits {
    template<> struct tag<MyPoint> { typedef point_tag type; };
    template<> struct coordinate_type<MyPoint> { typedef double type; };
    // ... 其他特化
}

// 算法对任何满足Concept的类型工作
bg::convex_hull(my_points, result);
```
- **轻量级**：头文件为主，编译期多态
- **非侵入式**：通过 traits 系统适配现有类型
- **STL风格**：与标准库无缝集成

---

### 4. **性能与开销**

| 指标 | CGAL | Boost.Geometry |
|------|------|----------------|
| **编译时间** | 较慢（模板实例化复杂） | 较快 |
| **运行时开销** | 精确模式有显著开销 | 通常浮点运算，开销低 |
| **内存占用** | 较大（复杂数据结构） | 较小 |
| **二进制大小** | 较大 | 较小 |

---

### 5. **依赖与集成**

| 方面 | CGAL | Boost.Geometry |
|------|------|----------------|
| **依赖项** | GMP, MPFR, Boost（部分模块）, Eigen（可选） | 仅依赖Boost其他模块 |
| **头文件库** | 否（部分需编译） | ✅ 是 |
| **C++标准** | C++14/17 | C++14 起 |
| **Boost集成** | 部分使用Boost | ✅ 原生Boost模块 |
| **学习曲线** | 陡峭 | 平缓 |

---

## 相似之处

1. **泛型编程**：都大量使用 C++ 模板
2. **概念检查**：都基于几何 Concept 设计（虽然实现方式不同）
3. **基础算法重叠**：凸包、多边形操作、空间搜索等
4. **可扩展性**：都支持自定义数类型和几何类型
5. **开源协议**：都使用宽松的开源许可（LGPL/GPL vs Boost License）

---

## 选型建议

### 选择 **CGAL** 当：
- 需要 **Delaunay 三角剖分** 或 **网格生成**
- 处理 **3D 复杂几何**（曲面重建、体网格）
- 对**数值稳健性**有严格要求（CAD/GIS/科学计算）
- 需要 **Voronoi 图**、**排列计算** 等高级算法
- 项目规模较大，可接受学习成本

### 选择 **Boost.Geometry** 当：
- 需要**轻量级**几何工具，快速集成
- 主要处理 **2D 几何**（GIS 应用、游戏2D碰撞）
- 已有 **Boost 依赖**，希望保持一致性
- 需要**头文件-only**库，简化部署
- 对**编译时间**敏感

---

## 协同使用策略

实际项目中两者可以**互补**：

```cpp
// 示例：Boost.Geometry处理简单2D，CGAL处理复杂三角剖分
#include <boost/geometry.hpp>
#include <CGAL/Delaunay_triangulation_2.h>

// 1. 用Boost.Geometry进行快速边界框计算
bg::envelope(polygon, box);

// 2. 将数据导入CGAL进行精确三角剖分
std::vector<Point> cgal_points;
// ... 转换逻辑
CGAL::Delaunay_triangulation_2<K> dt;
dt.insert(cgal_points.begin(), cgal_points.end());
```

---

## 总结

| 场景 | 推荐 |
|------|------|
| 学术研究/算法原型 | CGAL |
| 工业级CAD/CAM/CAE | CGAL |
| 游戏引擎2D物理 | Boost.Geometry |
| Web GIS/轻量级应用 | Boost.Geometry |
| 3D打印/网格处理 | CGAL |
| 快速原型开发 | Boost.Geometry → 必要时迁移到CGAL |

需要我深入对比某个具体算法（如凸包或空间搜索）的实现差异吗？