#pragma once

// only included in case there's a C++11 compiler out there that doesn't support `#pragma once`
#ifndef DKM_KMEANS_H
#define DKM_KMEANS_H

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <tuple>
#include <type_traits>
#include <vector>

/*
DKM - A k-means implementation that is generic across variable data dimensions.
DKM - 一个在不同数据维度上通用的k-means算法实现。
*/
namespace dkm
{

    /*
    These functions are all private implementation details and shouldn't be referenced outside of this
    file.
    这些函数都是私有实现细节，不应该在这个文件之外引用。
    */
    namespace details
    {

        /*
        Calculate the square of the distance between two points.
        计算两点之间距离的平方。
        */
        template <typename T, size_t N>
        T distance_squared(const std::array<T, N> &point_a, const std::array<T, N> &point_b)
        {
            // 初始化距离平方为类型T的默认值
            T d_squared = T();
            // 遍历每个维度
            for (typename std::array<T, N>::size_type i = 0; i < N; ++i)
            {
                // 计算当前维度上两点的差值
                auto delta = point_a[i] - point_b[i];
                // 累加差值的平方
                d_squared += delta * delta;
            }
            return d_squared;
        }

        template <typename T, size_t N>
        T distance(const std::array<T, N> &point_a, const std::array<T, N> &point_b)
        {
            // 调用distance_squared函数计算距离平方，然后取平方根得到距离
            return std::sqrt(distance_squared(point_a, point_b));
        }

        /*
        Calculate the smallest distance between each of the data points and any of the input means.
        计算每个数据点与任何一个输入均值之间的最小距离。
        */
        template <typename T, size_t N>
        std::vector<T> closest_distance(
            const std::vector<std::array<T, N>> &means, const std::vector<std::array<T, N>> &data)
        {
            // 用于存储每个数据点的最小距离
            std::vector<T> distances;
            // 预先分配足够的空间，避免多次重新分配内存
            distances.reserve(data.size());
            // 遍历每个数据点
            for (auto &d : data)
            {
                // 初始化最小距离为第一个均值与当前数据点的距离平方
                T closest = distance_squared(d, means[0]);
                // 遍历每个均值
                for (auto &m : means)
                {
                    // 计算当前均值与当前数据点的距离平方
                    T distance = distance_squared(d, m);
                    // 如果当前距离更小，则更新最小距离
                    if (distance < closest)
                        closest = distance;
                }
                // 将最小距离添加到结果向量中
                distances.push_back(closest);
            }
            return distances;
        }

        /*
        This is an alternate initialization method based on the [kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B)
        initialization algorithm.
        这是一种基于[kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B)初始化算法的替代初始化方法。
        */
        template <typename T, size_t N>
        std::vector<std::array<T, N>> random_plusplus(const std::vector<std::array<T, N>> &data, uint32_t k, uint64_t seed)
        {
            // 确保k大于0
            assert(k > 0);
            // 确保数据点数量大于0
            assert(data.size() > 0);
            // 定义输入数据的大小类型
            using input_size_t = typename std::array<T, N>::size_type;
            // 用于存储均值
            std::vector<std::array<T, N>> means;
            // 使用线性同余生成器作为随机数引擎，参数根据https://en.wikipedia.org/wiki/Linear_congruential_generator#Parameters_in_common_use选择
            std::linear_congruential_engine<uint64_t, 6364136223846793005, 1442695040888963407, UINT64_MAX> rand_engine(seed);

            // Select first mean at random from the set
            // 从数据集中随机选择第一个均值
            {
                // 定义一个均匀分布的随机数生成器
                std::uniform_int_distribution<input_size_t> uniform_generator(0, data.size() - 1);
                // 随机选择一个数据点作为第一个均值
                means.push_back(data[uniform_generator(rand_engine)]);
            }

            // 循环选择剩余的k-1个均值
            for (uint32_t count = 1; count < k; ++count)
            {
                // 计算每个数据点到最近均值的距离
                auto distances = details::closest_distance(means, data);
                // 定义一个离散分布的随机数生成器，根据距离加权选择数据点
#if !defined(_MSC_VER) || _MSC_VER >= 1900
                std::discrete_distribution<input_size_t> generator(distances.begin(), distances.end());
#else // MSVC++ older than 14.0
                input_size_t i = 0;
                std::discrete_distribution<input_size_t> generator(distances.size(), 0.0, 0.0, [&distances, &i](double)
                                                                   { return distances[i++]; });
#endif
                // 根据离散分布选择一个数据点作为新的均值
                means.push_back(data[generator(rand_engine)]);
            }
            return means;
        }

        /*
        Calculate the index of the mean a particular data point is closest to (euclidean distance)
        计算某个特定数据点最接近的均值的索引（欧几里得距离）。
        */
        template <typename T, size_t N>
        uint32_t closest_mean(const std::array<T, N> &point, const std::vector<std::array<T, N>> &means)
        {
            // 确保均值向量不为空
            assert(!means.empty());
            // 初始化最小距离为第一个均值与当前数据点的距离平方
            T smallest_distance = distance_squared(point, means[0]);
            // 初始化最接近的均值的索引为0
            typename std::array<T, N>::size_type index = 0;
            T distance;
            // 遍历每个均值
            for (size_t i = 1; i < means.size(); ++i)
            {
                // 计算当前均值与当前数据点的距离平方
                distance = distance_squared(point, means[i]);
                // 如果当前距离更小，则更新最小距离和最接近的均值的索引
                if (distance < smallest_distance)
                {
                    smallest_distance = distance;
                    index = i;
                }
            }
            return index;
        }

        /*
        Calculate the index of the mean each data point is closest to (euclidean distance).
        计算每个数据点最接近的均值的索引（欧几里得距离）。
        */
        template <typename T, size_t N>
        std::vector<uint32_t> calculate_clusters(
            const std::vector<std::array<T, N>> &data, const std::vector<std::array<T, N>> &means)
        {
            // 用于存储每个数据点所属的聚类索引
            std::vector<uint32_t> clusters;
            // 遍历每个数据点
            for (auto &point : data)
            {
                // 计算当前数据点最接近的均值的索引，并添加到结果向量中
                clusters.push_back(closest_mean(point, means));
            }
            return clusters;
        }

        /*
        Calculate means based on data points and their cluster assignments.
        根据数据点及其聚类分配计算均值。
        */
        template <typename T, size_t N>
        std::vector<std::array<T, N>> calculate_means(const std::vector<std::array<T, N>> &data,
                                                      const std::vector<uint32_t> &clusters,
                                                      const std::vector<std::array<T, N>> &old_means,
                                                      uint32_t k)
        {
            // 用于存储新的均值
            std::vector<std::array<T, N>> means(k);
            // 用于存储每个聚类中的数据点数量
            std::vector<T> count(k, T());
            // 遍历每个数据点
            for (size_t i = 0; i < std::min(clusters.size(), data.size()); ++i)
            {
                // 获取当前数据点所属的聚类的均值
                auto &mean = means[clusters[i]];
                // 增加该聚类中的数据点数量
                count[clusters[i]] += 1;
                // 遍历每个维度
                for (size_t j = 0; j < std::min(data[i].size(), mean.size()); ++j)
                {
                    // 累加该维度上的数据点值
                    mean[j] += data[i][j];
                }
            }
            // 遍历每个聚类
            for (size_t i = 0; i < k; ++i)
            {
                // 如果该聚类中没有数据点，则使用旧的均值
                if (count[i] == 0)
                {
                    means[i] = old_means[i];
                }
                else
                {
                    // 遍历每个维度
                    for (size_t j = 0; j < means[i].size(); ++j)
                    {
                        // 计算该维度上的均值
                        means[i][j] /= count[i];
                    }
                }
            }
            return means;
        }

        template <typename T, size_t N>
        std::vector<T> deltas(
            const std::vector<std::array<T, N>> &old_means, const std::vector<std::array<T, N>> &means)
        {
            // 用于存储每个均值的变化量
            std::vector<T> distances;
            // 预先分配足够的空间，避免多次重新分配内存
            distances.reserve(means.size());
            // 确保旧均值和新均值的数量相同
            assert(old_means.size() == means.size());
            // 遍历每个均值
            for (size_t i = 0; i < means.size(); ++i)
            {
                // 计算旧均值和新均值之间的距离，并添加到结果向量中
                distances.push_back(distance(means[i], old_means[i]));
            }
            return distances;
        }

        template <typename T>
        bool deltas_below_limit(const std::vector<T> &deltas, T min_delta)
        {
            // 遍历每个变化量
            for (T d : deltas)
            {
                // 如果有任何变化量大于最小变化量，则返回false
                if (d > min_delta)
                {
                    return false;
                }
            }
            // 所有变化量都小于等于最小变化量，返回true
            return true;
        }

    } // namespace details

    /*
    clustering_parameters is the configuration used for running the kmeans_lloyd algorithm.

    It requires a k value for initialization, and can subsequently be configured with your choice
    of optional parameters, including:
    * Maximum iteration count; the algorithm will terminate if it reaches this iteration count
      before converging on a solution. The results returned are the means and cluster assignments
      calculated in the last iteration before termination.
    * Minimum delta; the algorithm will terminate if the change in position of all means is
      smaller than the specified distance.
    * Random seed; if present, this will be used in place of `std::random_device` for kmeans++
      initialization. This can be used to ensure reproducible/deterministic behavior.
    clustering_parameters 是用于运行kmeans_lloyd算法的配置。

    它需要一个k值进行初始化，随后可以配置可选参数，包括：
    * 最大迭代次数；如果算法在收敛到解决方案之前达到此迭代次数，则将终止。返回的结果是终止前最后一次迭代中计算的均值和聚类分配。
    * 最小变化量；如果所有均值的位置变化小于指定距离，则算法将终止。
    * 随机种子；如果存在，将用于替代`std::random_device`进行kmeans++初始化。这可用于确保可重复/确定性的行为。
    */
    template <typename T>
    class clustering_parameters
    {
    public:
        /**
         * @brief 构造函数，初始化聚类参数
         * 
         * @param k 聚类的数量，必须大于0
         */
        explicit clustering_parameters(uint32_t k) : _k(k),
                                                     _has_max_iter(false), _max_iter(),
                                                     _has_min_delta(false), _min_delta(),
                                                     _has_rand_seed(false), _rand_seed()
        {
        }

        /**
         * @brief 设置最大迭代次数
         * 
         * @param max_iter 最大迭代次数
         */
        void set_max_iteration(uint64_t max_iter)
        {
            _max_iter = max_iter;
            _has_max_iter = true;
        }

        /**
         * @brief 设置最小变化量
         * 
         * @param min_delta 最小变化量
         */
        void set_min_delta(T min_delta)
        {
            _min_delta = min_delta;
            _has_min_delta = true;
        }

        /**
         * @brief 设置随机种子
         * 
         * @param rand_seed 随机种子
         */
        void set_random_seed(uint64_t rand_seed)
        {
            _rand_seed = rand_seed;
            _has_rand_seed = true;
        }

        /**
         * @brief 检查是否设置了最大迭代次数
         * 
         * @return true 如果设置了最大迭代次数
         * @return false 如果未设置最大迭代次数
         */
        bool has_max_iteration() const { return _has_max_iter; }

        /**
         * @brief 检查是否设置了最小变化量
         * 
         * @return true 如果设置了最小变化量
         * @return false 如果未设置最小变化量
         */
        bool has_min_delta() const { return _has_min_delta; }

        /**
         * @brief 检查是否设置了随机种子
         * 
         * @return true 如果设置了随机种子
         * @return false 如果未设置随机种子
         */
        bool has_random_seed() const { return _has_rand_seed; }

        /**
         * @brief 获取聚类的数量
         * 
         * @return uint32_t 聚类的数量
         */
        uint32_t get_k() const { return _k; };

        /**
         * @brief 获取最大迭代次数
         * 
         * @return uint64_t 最大迭代次数
         */
        uint64_t get_max_iteration() const { return _max_iter; }

        /**
         * @brief 获取最小变化量
         * 
         * @return T 最小变化量
         */
        T get_min_delta() const { return _min_delta; }

        /**
         * @brief 获取随机种子
         * 
         * @return uint64_t 随机种子
         */
        uint64_t get_random_seed() const { return _rand_seed; }

    private:
        // 聚类的数量
        uint32_t _k;
        // 是否设置了最大迭代次数
        bool _has_max_iter;
        // 最大迭代次数
        uint64_t _max_iter;
        // 是否设置了最小变化量
        bool _has_min_delta;
        // 最小变化量
        T _min_delta;
        // 是否设置了随机种子
        bool _has_rand_seed;
        // 随机种子
        uint64_t _rand_seed;
    };

    /*
    Implementation of k-means generic across the data type and the dimension of each data item. Expects
    the data to be a vector of fixed-size arrays. Generic parameters are the type of the base data (T)
    and the dimensionality of each data point (N). All points must have the same dimensionality.

    e.g. points of the form (X, Y, Z) would be N = 3.

    Takes a `clustering_parameters` struct for algorithm configuration. See the comments for the
    `clustering_parameters` struct for more information about the configuration values and how they
    affect the algorithm.

    Returns a std::tuple containing:
      0: A vector holding the means for each cluster from 0 to k-1.
      1: A vector containing the cluster number (0 to k-1) for each corresponding element of the input
         data vector.

    Implementation details:
    This implementation of k-means uses [Lloyd's Algorithm](https://en.wikipedia.org/wiki/Lloyd%27s_algorithm)
    with the [kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B)
    used for initializing the means.

    实现了一个通用的k-means算法，适用于不同数据类型和数据维度。要求数据是一个固定大小数组的向量。
    模板参数包括基础数据类型 (T) 和每个数据点的维度 (N)。所有数据点必须具有相同的维度。

    例如，形式为 (X, Y, Z) 的点，N = 3。

    接受一个 `clustering_parameters` 结构体来配置算法。有关配置值及其对算法的影响，请参阅 `clustering_parameters` 结构体的注释。

    返回一个 std::tuple，包含：
      0: 一个向量，包含从 0 到 k-1 每个聚类的均值。
      1: 一个向量，包含输入数据向量中每个对应元素的聚类编号 (0 到 k-1)。

    实现细节:
    这个 k-means 实现使用了 [Lloyd's 算法](https://en.wikipedia.org/wiki/Lloyd%27s_algorithm)，
    并使用 [kmeans++](https://en.wikipedia.org/wiki/K-means%2B%2B) 算法来初始化均值。
    */
    template <typename T, size_t N>
    std::tuple<std::vector<std::array<T, N>>, std::vector<uint32_t>> kmeans_lloyd(
        const std::vector<std::array<T, N>> &data, const clustering_parameters<T> &parameters)
    {
        // 静态断言，确保模板参数T是有符号的算术类型（如float, double, int）
        static_assert(std::is_arithmetic<T>::value && std::is_signed<T>::value,
                      "kmeans_lloyd requires the template parameter T to be a signed arithmetic type (e.g. float, double, int)");
        // 确保k大于0
        assert(parameters.get_k() > 0);
        // 确保数据点数量至少为k
        assert(data.size() >= parameters.get_k());
        // 创建一个随机设备
        std::random_device rand_device;
        // 获取随机种子，如果设置了随机种子，则使用设置的值，否则使用随机设备生成的值
        uint64_t seed = parameters.has_random_seed() ? parameters.get_random_seed() : rand_device();
        // 使用kmeans++算法初始化均值
        std::vector<std::array<T, N>> means = details::random_plusplus(data, parameters.get_k(), seed);

        // 用于存储旧的均值
        std::vector<std::array<T, N>> old_means;
        // 用于存储更旧的均值
        std::vector<std::array<T, N>> old_old_means;
        // 用于存储每个数据点所属的聚类索引
        std::vector<uint32_t> clusters;
        // 迭代计数器
        uint64_t count = 0;
        // 计算新的均值，直到收敛或达到最大迭代次数
        do
        {
            // 计算每个数据点所属的聚类索引
            clusters = details::calculate_clusters(data, means);
            // 更新更旧的均值
            old_old_means = old_means;
            // 更新旧的均值
            old_means = means;
            // 计算新的均值
            means = details::calculate_means(data, clusters, old_means, parameters.get_k());
            // 增加迭代计数器
            ++count;
        } while (means != old_means && means != old_old_means && !(parameters.has_max_iteration() && count == parameters.get_max_iteration()) && !(parameters.has_min_delta() && details::deltas_below_limit(details::deltas(old_means, means), parameters.get_min_delta())));

        // 返回包含均值和聚类索引的元组
        return std::tuple<std::vector<std::array<T, N>>, std::vector<uint32_t>>(means, clusters);
    }

    /*
    This overload exists to support legacy code which uses this signature of the kmeans_lloyd function.
    Any code still using this signature should move to the version of this function that uses a
    `clustering_parameters` struct for configuration.
    此重载函数用于支持使用旧签名的kmeans_lloyd函数的遗留代码。
    任何仍在使用此签名的代码都应迁移到使用`clustering_parameters`结构体进行配置的版本。
    */
    template <typename T, size_t N>
    std::tuple<std::vector<std::array<T, N>>, std::vector<uint32_t>> kmeans_lloyd(
        const std::vector<std::array<T, N>> &data, uint32_t k,
        uint64_t max_iter = 0, T min_delta = -1.0)
    {
        // 创建一个聚类参数对象
        clustering_parameters<T> parameters(k);
        // 如果设置了最大迭代次数，则更新聚类参数
        if (max_iter != 0)
        {
            parameters.set_max_iteration(max_iter);
        }
        // 如果设置了最小变化量，则更新聚类参数
        if (min_delta != 0)
        {
            // 检查最小变化量是否不为0，如果不为0则设置最小变化量
            parameters.set_min_delta(min_delta);
        }
        // 调用使用聚类参数对象的kmeans_lloyd函数
        return kmeans_lloyd(data, parameters);
    }

} // namespace dkm

#endif /* DKM_KMEANS_H */
