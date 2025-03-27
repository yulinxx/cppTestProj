
#include <iostream>

// **********************************************************************************
// **********************************************************************************

// Vec模板类，包含2或3个值x, y, z的坐标
 template <typename T, size_t N>
class Vec {
public:
    Vec() {
        for (size_t i = 0; i < N; ++i) {
            data[i] = T();
        }
    }

    Vec(T x, T y) {
        static_assert(N == 2, "Vec dimension must be 2 for this constructor");
        data[0] = x;
        data[1] = y;
    }

    Vec(T x, T y, T z) {
        static_assert(N == 3, "Vec dimension must be 3 for this constructor");
        data[0] = x;
        data[1] = y;
        data[2] = z;
    }

    T& operator[](size_t index) {
        return data[index];
    }

    const T& operator[](size_t index) const {
        return data[index];
    }

private:
    T data[N];
};

// 定义不同类型和维度的 Vec 别名，方便后续使用
using Vec2i = Vec<int, 2>;
using Vec2f = Vec<float, 2>;

using Vec2 = Vec<double, 2>;
using Vec2d = Vec<double, 2>;

using Vec3i = Vec<int, 3>;
using Vec3f = Vec<float, 3>;
using Vec3d = Vec<double, 3>;


// **********************************************************************************
// **********************************************************************************



#include "vec.hpp"

// Rect矩形类，使用Vec模板类表示矩形的两个对角点
template <typename T>
class Rect {
public:
    Rect(const Vec<T, 2>& min, const Vec<T, 2>& max) : min_point(min), max_point(max) {}

    // 非 const 版本
    Vec<T, 2>& min() { return min_point; }
    Vec<T, 2>& max() { return max_point; }

    // const 版本
    const Vec<T, 2>& min() const { return min_point; }
    const Vec<T, 2>& max() const { return max_point; }

private:
    Vec<T, 2> min_point;
    Vec<T, 2> max_point;
};

// 定义不同类型的 Rect 别名，方便后续使用
using Rect2i = Rect<int>;
using Rect2f = Rect<float>;
// using Rect = Rect<double>;
using Rect2 = Rect<double>;
using Rect2d = Rect<double>;

// Rect3 类，用于表示三维空间中的矩形
template <typename T>
class Rect3 {
public:
    // 构造函数，接受两个三维向量作为矩形的最小和最大点
    Rect3(const Vec<T, 3>& min, const Vec<T, 3>& max) : min_point(min), max_point(max) {}

    // 非 const 版本的访问器，用于修改最小和最大点
    Vec<T, 3>& min() { return min_point; }
    Vec<T, 3>& max() { return max_point; }

    // const 版本的访问器，用于只读访问最小和最大点
    const Vec<T, 3>& min() const { return min_point; }
    const Vec<T, 3>& max() const { return max_point; }

private:
    // 矩形的最小和最大点
    Vec<T, 3> min_point;
    Vec<T, 3> max_point;
};

// 定义不同类型的 Rect3 别名，方便后续使用
using Rect3i = Rect3<int>;
using Rect3f = Rect3<float>;
// using Rect3 = Rect3<double>;
using Rect3d = Rect3<double>;


// **********************************************************************************

// **********************************************************************************



#include <iostream>
// 引入 Boost.Geometry 库，用于处理几何计算
#include <boost/geometry.hpp>
// 引入 Boost.Geometry 的 RTree 索引库，用于高效的空间索引
#include <boost/geometry/index/rtree.hpp>
// 引入自定义的向量类头文件
#include "vec.hpp"
// 引入自定义的矩形类头文件
#include "rect.hpp"

// 定义 Boost.Geometry 特性的命名空间别名，方便后续使用
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

/**
 * @brief 为自定义的 Vec 类定义 Boost.Geometry 特性，使其能被 Boost.Geometry 识别为点类型
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::tag<Vec<T, N>> {
    // 指定 Vec 类为点类型
    using type = bg::point_tag;
};

/**
 * @brief 定义 Vec 类的坐标类型，用于 Boost.Geometry 识别
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::coordinate_type<Vec<T, N>> {
    // 指定 Vec 类的坐标类型为 T
    using type = T;
};

/**
 * @brief 定义 Vec 类的坐标系统，用于 Boost.Geometry 识别
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::coordinate_system<Vec<T, N>> {
    // 指定 Vec 类使用笛卡尔坐标系
    using type = bg::cs::cartesian;
};

/**
 * @brief 定义 Vec 类的维度，用于 Boost.Geometry 识别
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::dimension<Vec<T, N>> : boost::mpl::int_<N> {};

/**
 * @brief 定义 Vec 类第一个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::access<Vec<T, N>, 0> {
    /**
     * @brief 获取 Vec 类对象第一个维度的坐标值
     * @param p Vec 类对象的常量引用
     * @return 第一个维度的坐标值
     */
    static T get(const Vec<T, N>& p) { return p[0]; }
    /**
     * @brief 设置 Vec 类对象第一个维度的坐标值
     * @param p Vec 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Vec<T, N>& p, T value) { p[0] = value; }
};

/**
 * @brief 定义 Vec 类第二个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::access<Vec<T, N>, 1> {
    /**
     * @brief 获取 Vec 类对象第二个维度的坐标值
     * @param p Vec 类对象的常量引用
     * @return 第二个维度的坐标值
     */
    static T get(const Vec<T, N>& p) { return p[1]; }
    /**
     * @brief 设置 Vec 类对象第二个维度的坐标值
     * @param p Vec 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Vec<T, N>& p, T value) { p[1] = value; }
};

/**
 * @brief 定义三维 Vec 类第三个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 向量元素的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::access<Vec<T, 3>, 2> {
    /**
     * @brief 获取三维 Vec 类对象第三个维度的坐标值
     * @param p 三维 Vec 类对象的常量引用
     * @return 第三个维度的坐标值
     */
    static T get(const Vec<T, 3>& p) { return p[2]; }
    /**
     * @brief 设置三维 Vec 类对象第三个维度的坐标值
     * @param p 三维 Vec 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Vec<T, 3>& p, T value) { p[2] = value; }
};

/**
 * @brief 为自定义的 Rect 类定义 Boost.Geometry 特性，使其能被 Boost.Geometry 识别为矩形类型
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::tag<Rect<T>> {
    // 指定 Rect 类为矩形类型
    using type = bg::box_tag;
};

/**
 * @brief 定义 Rect 类的点类型，用于 Boost.Geometry 识别
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::point_type<Rect<T>> {
    // 指定 Rect 类使用二维向量作为点类型
    using type = Vec<T, 2>;
};

/**
 * @brief 定义 Rect 类最小角第一个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::min_corner, 0> {
    /**
     * @brief 获取 Rect 类对象最小角第一个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最小角第一个维度的坐标值
     */
    static T get(const Rect<T>& b) { return b.min()[0]; }
    /**
     * @brief 设置 Rect 类对象最小角第一个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value) { b.min()[0] = value; }
};

/**
 * @brief 定义 Rect 类最小角第二个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::min_corner, 1> {
    /**
     * @brief 获取 Rect 类对象最小角第二个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最小角第二个维度的坐标值
     */
    static T get(const Rect<T>& b) { return b.min()[1]; }
    /**
     * @brief 设置 Rect 类对象最小角第二个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value) { b.min()[1] = value; }
};

/**
 * @brief 定义 Rect 类最大角第一个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::max_corner, 0> {
    /**
     * @brief 获取 Rect 类对象最大角第一个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最大角第一个维度的坐标值
     */
    static T get(const Rect<T>& b) { return b.max()[0]; }
    /**
     * @brief 设置 Rect 类对象最大角第一个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value) { b.max()[0] = value; }
};

/**
 * @brief 定义 Rect 类最大角第二个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::max_corner, 1> {
    /**
     * @brief 获取 Rect 类对象最大角第二个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最大角第二个维度的坐标值
     */
    static T get(const Rect<T>& b) { return b.max()[1]; }
    /**
     * @brief 设置 Rect 类对象最大角第二个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value) { b.max()[1] = value; }
};

// 定义不同类型和维度的 RTree 别名，方便后续使用
using RTree2i = bgi::rtree<std::pair<Rect2i, int>, bgi::quadratic<16>>;
using RTree2f = bgi::rtree<std::pair<Rect2f, int>, bgi::quadratic<16>>;
using RTree2d = bgi::rtree<std::pair<Rect2d, int>, bgi::quadratic<16>>;

// 对于三维 RTree，需要定义三维的矩形和点
// 这里省略了三维矩形的定义，假设已经定义了 Rect3 类
using RTree3i = bgi::rtree<std::pair<Rect3i, int>, bgi::quadratic<16>>;
using RTree3f = bgi::rtree<std::pair<Rect3f, int>, bgi::quadratic<16>>;
using RTree3d = bgi::rtree<std::pair<Rect3d, int>, bgi::quadratic<16>>;

// 为 Rect3 类定义 Boost.Geometry 特性
template <typename T>
struct boost::geometry::traits::tag<Rect3<T>> {
    using type = bg::box_tag;
};

template <typename T>
struct boost::geometry::traits::point_type<Rect3<T>> {
    using type = Vec<T, 3>;
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::min_corner, 0> {
    static T get(const Rect3<T>& b) { return b.min()[0]; }
    static void set(Rect3<T>& b, T value) { b.min()[0] = value; }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::min_corner, 1> {
    static T get(const Rect3<T>& b) { return b.min()[1]; }
    static void set(Rect3<T>& b, T value) { b.min()[1] = value; }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::min_corner, 2> {
    static T get(const Rect3<T>& b) { return b.min()[2]; }
    static void set(Rect3<T>& b, T value) { b.min()[2] = value; }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::max_corner, 0> {
    static T get(const Rect3<T>& b) { return b.max()[0]; }
    static void set(Rect3<T>& b, T value) { b.max()[0] = value; }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::max_corner, 1> {
    static T get(const Rect3<T>& b) { return b.max()[1]; }
    static void set(Rect3<T>& b, T value) { b.max()[1] = value; }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::max_corner, 2> {
    static T get(const Rect3<T>& b) { return b.max()[2]; }
    static void set(Rect3<T>& b, T value) { b.max()[2] = value; }
};

/**
 * @brief 主函数，程序入口
 * @return 程序退出状态码
 */
int main() {
    // 创建二维整数类型的 RTree 对象
    RTree2i rtree2i;
    // 创建一个二维整数类型的矩形对象，由两个二维整数向量定义其最小角和最大角
    Rect2i rect2i(Vec2i(0, 0), Vec2i(10, 10));
    // 将矩形对象和一个整数标识（这里是 1）作为一对插入到 RTree 中
    rtree2i.insert(std::make_pair(rect2i, 1));

    // 创建二维浮点数类型的 RTree 对象
    RTree2f rtree2f;
    // 创建一个二维浮点数类型的矩形对象，由两个二维浮点数向量定义其最小角和最大角
    Rect2f rect2f(Vec2f(0.0f, 0.0f), Vec2f(10.0f, 10.0f));
    // 将矩形对象和一个整数标识（这里是 1）作为一对插入到 RTree 中
    rtree2f.insert(std::make_pair(rect2f, 1));

    // 创建二维双精度浮点数类型的 RTree 对象
    RTree2d rtree2d;
    // 创建一个二维双精度浮点数类型的矩形对象，由两个二维双精度浮点数向量定义其最小角和最大角
    Rect2d rect2d(Vec2d(0.0, 0.0), Vec2d(10.0, 10.0));
    // 将矩形对象和一个整数标识（这里是 1）作为一对插入到 RTree 中
    rtree2d.insert(std::make_pair(rect2d, 1));

    // 可以添加三维 RTree 的创建和插入代码

    // 创建三维整数类型的 RTree 对象
    RTree3i rtree3i;
    // 创建一个三维整数类型的矩形对象，由两个三维整数向量定义其最小角和最大角
    Rect3i rect3i(Vec3i(0, 0, 0), Vec3i(10, 10, 10));
    // 将矩形对象和一个整数标识（这里是 1）作为一对插入到 RTree 中
    rtree3i.insert(std::make_pair(rect3i, 1));

    // 程序正常退出，返回 0
    return 0;
}


// **********************************************************************************
