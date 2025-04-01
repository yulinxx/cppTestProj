
#include <iostream>
// 引入 Boost.Geometry 库，用于处理几何计算
#include <boost/geometry.hpp>
// 引入 Boost.Geometry 的 RTree 索引库，用于高效的空间索引
#include <boost/geometry/index/rtree.hpp>

// 定义 Boost.Geometry 特性的命名空间别名，方便后续使用
namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

/**
 * @brief 为自定义的 Vec 类定义 Boost.Geometry 特性，使其能被 Boost.Geometry 识别为点类型
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::tag<Vec<T, N>>
{
    // 指定 Vec 类为点类型
    using type = bg::point_tag;
};

/**
 * @brief 定义 Vec 类的坐标类型，用于 Boost.Geometry 识别
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::coordinate_type<Vec<T, N>>
{
    // 指定 Vec 类的坐标类型为 T
    using type = T;
};

/**
 * @brief 定义 Vec 类的坐标系统，用于 Boost.Geometry 识别
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::coordinate_system<Vec<T, N>>
{
    // 指定 Vec 类使用笛卡尔坐标系
    using type = bg::cs::cartesian;
};

/**
 * @brief 定义 Vec 类的维度，用于 Boost.Geometry 识别
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::dimension<Vec<T, N>> : boost::mpl::int_<N>
{
};

/**
 * @brief 定义 Vec 类第一个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::access<Vec<T, N>, 0>
{
    /**
     * @brief 获取 Vec 类对象第一个维度的坐标值
     * @param p Vec 类对象的常量引用
     * @return 第一个维度的坐标值
     */
    static T get(const Vec<T, N>& p)
    {
        return p[0];
    }
    /**
     * @brief 设置 Vec 类对象第一个维度的坐标值
     * @param p Vec 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Vec<T, N>& p, T value)
    {
        p[0] = value;
    }
};

/**
 * @brief 定义 Vec 类第二个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 向量元素的类型，如 int、float、double 等
 * @tparam N 向量的维度
 */
template <typename T, std::size_t N>
struct boost::geometry::traits::access<Vec<T, N>, 1>
{
    /**
     * @brief 获取 Vec 类对象第二个维度的坐标值
     * @param p Vec 类对象的常量引用
     * @return 第二个维度的坐标值
     */
    static T get(const Vec<T, N>& p)
    {
        return p[1];
    }
    /**
     * @brief 设置 Vec 类对象第二个维度的坐标值
     * @param p Vec 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Vec<T, N>& p, T value)
    {
        p[1] = value;
    }
};

/**
 * @brief 定义三维 Vec 类第三个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 向量元素的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::access<Vec<T, 3>, 2>
{
    /**
     * @brief 获取三维 Vec 类对象第三个维度的坐标值
     * @param p 三维 Vec 类对象的常量引用
     * @return 第三个维度的坐标值
     */
    static T get(const Vec<T, 3>& p)
    {
        return p[2];
    }
    /**
     * @brief 设置三维 Vec 类对象第三个维度的坐标值
     * @param p 三维 Vec 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Vec<T, 3>& p, T value)
    {
        p[2] = value;
    }
};

/**
 * @brief 为自定义的 Rect 类定义 Boost.Geometry 特性，使其能被 Boost.Geometry 识别为矩形类型
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::tag<Rect<T>>
{
    // 指定 Rect 类为矩形类型
    using type = bg::box_tag;
};

/**
 * @brief 定义 Rect 类的点类型，用于 Boost.Geometry 识别
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::point_type<Rect<T>>
{
    // 指定 Rect 类使用二维向量作为点类型
    using type = Vec<T, 2>;
};

/**
 * @brief 定义 Rect 类最小角第一个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::min_corner, 0>
{
    /**
     * @brief 获取 Rect 类对象最小角第一个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最小角第一个维度的坐标值
     */
    static T get(const Rect<T>& b)
    {
        return b.min()[0];
    }
    /**
     * @brief 设置 Rect 类对象最小角第一个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value)
    {
        b.min()[0] = value;
    }
};

/**
 * @brief 定义 Rect 类最小角第二个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::min_corner, 1>
{
    /**
     * @brief 获取 Rect 类对象最小角第二个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最小角第二个维度的坐标值
     */
    static T get(const Rect<T>& b)
    {
        return b.min()[1];
    }
    /**
     * @brief 设置 Rect 类对象最小角第二个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value)
    {
        b.min()[1] = value;
    }
};

/**
 * @brief 定义 Rect 类最大角第一个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::max_corner, 0>
{
    /**
     * @brief 获取 Rect 类对象最大角第一个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最大角第一个维度的坐标值
     */
    static T get(const Rect<T>& b)
    {
        return b.max()[0];
    }
    /**
     * @brief 设置 Rect 类对象最大角第一个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value)
    {
        b.max()[0] = value;
    }
};

/**
 * @brief 定义 Rect 类最大角第二个维度的访问器，用于 Boost.Geometry 获取和设置坐标值
 * @tparam T 矩形坐标的类型，如 int、float、double 等
 */
template <typename T>
struct boost::geometry::traits::indexed_access<Rect<T>, bg::max_corner, 1>
{
    /**
     * @brief 获取 Rect 类对象最大角第二个维度的坐标值
     * @param b Rect 类对象的常量引用
     * @return 最大角第二个维度的坐标值
     */
    static T get(const Rect<T>& b)
    {
        return b.max()[1];
    }
    /**
     * @brief 设置 Rect 类对象最大角第二个维度的坐标值
     * @param b Rect 类对象的引用
     * @param value 要设置的坐标值
     */
    static void set(Rect<T>& b, T value)
    {
        b.max()[1] = value;
    }
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
struct boost::geometry::traits::tag<Rect3<T>>
{
    using type = bg::box_tag;
};

template <typename T>
struct boost::geometry::traits::point_type<Rect3<T>>
{
    using type = Vec<T, 3>;
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::min_corner, 0>
{
    static T get(const Rect3<T>& b)
    {
        return b.min()[0];
    }
    static void set(Rect3<T>& b, T value)
    {
        b.min()[0] = value;
    }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::min_corner, 1>
{
    static T get(const Rect3<T>& b)
    {
        return b.min()[1];
    }
    static void set(Rect3<T>& b, T value)
    {
        b.min()[1] = value;
    }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::min_corner, 2>
{
    static T get(const Rect3<T>& b)
    {
        return b.min()[2];
    }
    static void set(Rect3<T>& b, T value)
    {
        b.min()[2] = value;
    }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::max_corner, 0>
{
    static T get(const Rect3<T>& b)
    {
        return b.max()[0];
    }
    static void set(Rect3<T>& b, T value)
    {
        b.max()[0] = value;
    }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::max_corner, 1>
{
    static T get(const Rect3<T>& b)
    {
        return b.max()[1];
    }
    static void set(Rect3<T>& b, T value)
    {
        b.max()[1] = value;
    }
};

template <typename T>
struct boost::geometry::traits::indexed_access<Rect3<T>, bg::max_corner, 2>
{
    static T get(const Rect3<T>& b)
    {
        return b.max()[2];
    }
    static void set(Rect3<T>& b, T value)
    {
        b.max()[2] = value;
    }
};


void testBoostRTree()
{
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
}


// 引入 LibSpatialIndex 库的头文件，用于空间索引操作
#include <spatialindex/SpatialIndex.h>

/**
 * @brief 回调函数类，用于处理查询结果
 * 
 * 该类继承自 SpatialIndex::IVisitor，实现了三个虚函数，用于处理节点和数据的访问。
 */
class MyVisitor : public SpatialIndex::IVisitor
{
public:
    /**
     * @brief 访问节点的回调函数
     * 
     * 当遍历到 RTree 中的节点时，会调用此函数。在本实现中，此函数为空，不做任何处理。
     * @param n 要访问的节点的常量引用
     */
    void visitNode(const SpatialIndex::INode& n) {}

    /**
     * @brief 访问单个数据项的回调函数
     * 
     * 当查询到匹配的数据项时，会调用此函数。在本实现中，会输出找到的数据项的 ID。
     * @param d 要访问的数据项的常量引用
     */
    void visitData(const SpatialIndex::IData& d)
    {
        std::cout << "Found data with ID: " << d.getIdentifier() << std::endl;
    }

    /**
     * @brief 访问多个数据项的回调函数
     * 
     * 当查询到多个匹配的数据项时，会调用此函数。在本实现中，此函数为空，不做任何处理。
     * @param v 包含多个数据项指针的向量
     */
    void visitData(std::vector<const SpatialIndex::IData*>& v) {}
};

/**
 * @brief 测试 LibSpatialIndex 库的函数
 *  find_package(libspatialindex CONFIG REQUIRED)
 *  target_link_libraries(${PROJECT_NAME} PRIVATE spatialindex spatialindex_c)
 * 该函数演示了如何使用 LibSpatialIndex 库创建二维和三维的 RTree 索引，插入数据并进行查询操作。
 */
void testLibSpatialIndex()
{
    // 定义存储管理器的基础名称，用于磁盘存储
    std::string baseName = "spatialindex";
    // 创建一个新的磁盘存储管理器，块大小为 4096 字节
    SpatialIndex::IStorageManager* storage = SpatialIndex::StorageManager::createNewDiskStorageManager(baseName, 4096);
    // 定义一个索引标识符，用于标识创建的 RTree 索引
    SpatialIndex::id_type indexIdentifier;

    // 创建二维 RTree 索引
    // 参数说明：
    // *storage: 存储管理器的引用
    // 0.7: 填充因子，表示节点的最小填充比例
    // 100: 节点的最大容量
    // 100: 节点的最小容量
    // 2: 索引的维度，这里是二维
    // SpatialIndex::RTree::RV_RSTAR: RTree 的分裂策略，使用 R* 算法
    // indexIdentifier: 索引标识符
    SpatialIndex::ISpatialIndex* rtree2d = SpatialIndex::RTree::createNewRTree(*storage, 0.7, 100, 100, 2, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);

    // 定义二维区域的最小坐标
    double low2d[2] = {0.0, 0.0};
    // 定义二维区域的最大坐标
    double high2d[2] = {10.0, 10.0};
    // 创建一个二维区域对象
    SpatialIndex::Region r2d(low2d, high2d, 2);
    // 向二维 RTree 中插入数据
    // 参数说明：
    // 0: 数据的大小，这里为 0 表示无额外数据
    // 0: 数据的指针，这里为 0 表示无额外数据
    // r2d: 要插入的区域对象
    // 1: 数据的标识符
    rtree2d->insertData(0, 0, r2d, 1);

    // 创建一个 MyVisitor 对象，用于处理查询结果
    MyVisitor visitor2d;
    // 在二维 RTree 中进行查询，查找与 r2d 相交的区域
    rtree2d->intersectsWithQuery(r2d, visitor2d);

    // 创建三维 RTree 索引，参数与二维 RTree 类似，只是维度改为 3
    SpatialIndex::ISpatialIndex* rtree3d = SpatialIndex::RTree::createNewRTree(*storage, 0.7, 100, 100, 3, SpatialIndex::RTree::RV_RSTAR, indexIdentifier);

    // 定义三维区域的最小坐标
    double low3d[3] = {0.0, 0.0, 0.0};
    // 定义三维区域的最大坐标
    double high3d[3] = {10.0, 10.0, 10.0};
    // 创建一个三维区域对象
    SpatialIndex::Region r3d(low3d, high3d, 3);
    // 向三维 RTree 中插入数据
    rtree3d->insertData(0, 0, r3d, 1);

    // 创建一个 MyVisitor 对象，用于处理查询结果
    MyVisitor visitor3d;
    // 在三维 RTree 中进行查询，查找与 r3d 相交的区域
    rtree3d->intersectsWithQuery(r3d, visitor3d);

    // 释放二维 RTree 索引的内存
    delete rtree2d;
    // 释放三维 RTree 索引的内存
    delete rtree3d;
    // 释放存储管理器的内存
    delete storage;
}

/**
 * @brief 主函数，程序入口
 * @return 程序退出状态码
 */
int main()
{
    testBoostRTree();
    testLibSpatialIndex();

    return 0;
}