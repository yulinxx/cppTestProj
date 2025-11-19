
// 全局：当前帧的视图-投影矩阵（正交示例）
glm::mat4 gVP;   // 你得自己算，或用 glm::ortho / glm::perspective

// 多段线的轴对齐包围盒（AABB）
struct AABB {
    glm::vec2 min, max;
    AABB(const std::vector<float>& verts) { // verts = [x0,y0,x1,y1,...]
        min = max = glm::vec2(verts[0], verts[1]);
        for (size_t i = 1; i < verts.size() / 2; ++i) {
            glm::vec2 v(verts[i*2], verts[i*2+1]);
            min = glm::min(min, v);
            max = glm::max(max, v);
        }
    }
};

// 判断 AABB 是否完全在裁剪立方体之外
bool CullAABB(const AABB& box, const glm::mat4& VP)
{
    // 8 个顶点齐次坐标
    glm::vec4 v[8] = {
        VP * glm::vec4(box.min.x, box.min.y, 0, 1),
        VP * glm::vec4(box.max.x, box.min.y, 0, 1),
        VP * glm::vec4(box.min.x, box.max.y, 0, 1),
        VP * glm::vec4(box.max.x, box.max.y, 0, 1),
        // z=0 的 4 个点已足够，线条在 z=0 平面
    };
    // 裁剪空间规则： -w <= x,y,z <= w
    bool allLeft = true, allRight = true, allBottom = true, allTop = true;
    for (int i = 0; i < 4; ++i) {
        float x = v[i].x, y = v[i].y, w = v[i].w;
        if (x >= -w) allLeft  = false;
        if (x <=  w) allRight = false;
        if (y >= -w) allBottom= false;
        if (y <=  w) allTop   = false;
    }
    // 只要有一侧全部在外，就剔除
    return allLeft || allRight || allBottom || allTop;
}

// 改造后的 defrag：只写可见线
void defragmentBuffers_WithCull(GLuint VBO, GLuint EBO,
                                std::vector<Polyline>& polylines,
                                std::vector<FreeBlock>& vFreeBlock,
                                size_t MaxVertices, size_t MaxIndices,
                                size_t& eboUsedCount)
{
    // 1. 先收集可见线
    std::vector<Polyline*> visible;
    for (auto& p : polylines) {
        AABB box(p.verts);
        if (!CullAABB(box, gVP)) visible.push_back(&p);
    }

    // 2. 按可见列表做紧凑布局（其余代码与你原来一样，只是遍历 visible）
    size_t nNextV = 0, nNextI = 0;
    std::vector<std::pair<size_t,size_t>> newOffs;
    for (auto* p : visible) {
        newOffs.push_back({nNextV, nNextI});
        nNextV += p->vertexCount;
        nNextI += p->indexCount;
    }
    if (nNextV > MaxVertices || nNextI > MaxIndices) {
        std::cerr << "[Cull-Defrag] 空间不足\n";
        return;
    }

    // 3. map & 写数据（只写 visible）
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void* vPtr = glMapBufferRange(GL_ARRAY_BUFFER, 0, MaxVertices*2*sizeof(float),
                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    void* iPtr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, MaxIndices*sizeof(unsigned int),
                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    // ... 后续复制代码与你原来相同，只是循环 visible ...
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    // 4. 更新元数据 & 重建空闲块（同旧代码）
    for (size_t i = 0; i < visible.size(); ++i) {
        visible[i]->vboOffset = newOffs[i].first;
        visible[i]->indexOffset= newOffs[i].second;
    }
    vFreeBlock.clear();
    if (nNextV < MaxVertices) vFreeBlock.push_back({nNextV, MaxVertices - nNextV});
    eboUsedCount = nNextI;

    std::cout << "[Cull-Defrag] 可见 " << visible.size() << "/" << polylines.size()
              << " 条线，写入顶点 " << nNextV << "\n";
}