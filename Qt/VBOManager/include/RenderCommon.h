#ifndef RENDER_COMMON_H
#define RENDER_COMMON_H

#include <functional>
#include <vector>
#include "Brush.h"

namespace GLRhi
{
    // 多段线
    struct  PolylineData
    {
        std::vector<long long> vId;                 // ID
        std::vector<float> vVerts;                  // x, y, z
        std::vector<size_t> vCount;                 // 线段顶点数
        Brush brush{ 0.0f, 0.0f, 0.0f, 1.0f, 1 };   // 渲染信息

        bool bDirty{ true };       // 是否需要更新
        bool bValid{ true };       // 是否有效
    };

    // 实例化线段数据结构
    struct InstanceLineData
    {
        float pos1[3];    // 线段第一个端点位置
        float pos2[3];    // 线段第二个端点位置
        float color[4];   // 线段颜色 (RGBA)
        float width;      // 线段宽度
        float depth;      // 深度值
    };

    // 三角形
    struct  TriangleData
    {
        long long id;                       // ID
        std::vector<float> vVerts;           // x, y, len
        std::vector<unsigned int> indices;  // 索引
        Brush brush;
    };

    // 实例化三角形数据结构
    struct InstanceTriangleData
    {
        float pos1[3];    // 三角形第一个顶点位置
        float pos2[3];    // 三角形第二个顶点位置
        float pos3[3];    // 三角形第三个顶点位置
        float color[4];   // 三角形颜色 (RGBA)
        float depth;      // 深度值
    };

    // 纹理
    struct  TextureData
    {
        long long id;                       // ID
        std::vector<float> vVerts;           //  x, y, u, v
        std::vector<unsigned int> indices;
        unsigned int tex;
        Brush brush;
    };

    // 实例纹理
    struct  InstanceTexData
    {
        long long id;        // ID
        float x, y;          // 位置
        float width, height; // 尺寸
        int   textureLayer;  // 纹理数组层索引
        float alpha;         // 透明度
    };

}

#endif // RENDER_COMMON_H