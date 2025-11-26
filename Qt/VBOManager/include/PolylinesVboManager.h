#pragma once
#include <vector>
#include <unordered_map>
#include <shared_mutex>
#include <atomic>
#include <thread>
#include <map>
#include "RenderCommon.h"
#include <QOpenGLFunctions_3_3_Core>
namespace GLRhi
{
    // 每个图元在块中的真实信息
    struct PrimitiveInfo
    {
        long long id{ -1 };
        GLsizei nIndexCount{ 0 }; // = vertexCount (LINE_STRIP)
        GLint nBaseVertex{ 0 }; // 配合 glMultiDrawElementsBaseVertex
        bool bValid{ true };
    };
    struct ColorVBOBlock
    {
        unsigned int vao{ 0 };
        unsigned int vbo{ 0 };
        unsigned int ebo{ 0 };
        Color color;
        // 容量 & 已使用
        size_t nVertexCapacity{ 0 }; // 顶点数（不是字节）
        size_t nIndexCapacity{ 0 };
        size_t nVertexCount{ 0 }; // 已写入顶点总数
        size_t nIndexCount{ 0 }; // 已写入索引总数（=vertexCount）
        // 每个图元的绘制信息（顺序严格对应）
        std::vector<GLsizei> vDrawCounts; // 每条线的索引数
        std::vector<GLint> vBaseVertices; // 每条线的 baseVertex
        std::vector<PrimitiveInfo> vPrimitives; // 完整的图元信息
        // ID → 在 primitives 中的下标（快速查找）
        std::unordered_map<long long, size_t> idToIndexMap;
        bool bDirty{ false }; // VBO/EBO 数据已改
    };
    ////////////////////////////////////////////////////////////////
    class PolylinesVboManager final
    {
    public:
        PolylinesVboManager();
        ~PolylinesVboManager();
    public:
        bool addPolylines(std::vector<PolylineData>& vPolylineDatas);
        bool addPolyline(long long id, const std::vector<float>& vertices, const Color& color);
        bool removePolyline(long long id);
        bool updatePolyline(long long id, const std::vector<float>& vertices);
        bool setPolylineVisible(long long id, bool visible);
        void clearAllPrimitives();
        void renderVisiblePrimitives();
        void defragment();
        void startBackgroundDefrag();
        void stopBackgroundDefrag();
    private:
        ColorVBOBlock* createNewColorBlock(const Color& color);
        ColorVBOBlock* findOrCreateColorBlock(const Color& color);
        void isBlockEnable(ColorVBOBlock* block, size_t needVert, size_t needIdx);
        void uploadAllData(ColorVBOBlock* block); // 采用 Orphaning 方式全量上传
        void rebuildDrawCommands(ColorVBOBlock* block); // 只重建 vDrawCounts / vBaseVertices
        void bindBlock(ColorVBOBlock* block) const;
        void unbindBlock() const;
    private:
        QOpenGLFunctions_3_3_Core* m_gl{ nullptr };
        mutable std::shared_mutex m_mutex; // 可多线程读（render）
        // 颜色 → 多个 block（同一个颜色可能有多个 block，防止单个太大）
        std::map<Color, std::vector<ColorVBOBlock*>> m_colorBlocks;
        // 全局 ID → {color, block*, primitiveIndexInBlock}
        struct Loc
        {
            Color color;
            ColorVBOBlock* block{ nullptr };
            size_t primIdx{ 0 };
        };
        std::unordered_map<long long, Loc> m_locationMap;
        // 顶点缓存（用于 update / defrag）
        std::unordered_map<long long, std::vector<float>> m_vertexCache;
        std::thread m_defragThread;
        std::atomic<bool> m_stopDefrag{ false };
        static constexpr size_t INIT_CAPACITY = 256 * 1024; // 顶点数
        static constexpr size_t GROW_STEP = 512 * 1024;
        static constexpr size_t MAX_VERT_PER_BLOCK = 1'000'000;
    };
}

