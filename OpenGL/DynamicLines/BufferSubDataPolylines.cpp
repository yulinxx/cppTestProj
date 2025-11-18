// main.cpp
// Compile with: g++ main.cpp -lglfw -ldl -lGL -pthread (example on Linux)
// Make sure glad and GLFW are available and glad loader included properly.

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstring>

// -----------------------------
// Data structures
// -----------------------------
struct Polyline {
    size_t vboOffset = 0;     // vertex start index inside big VBO (vertex count units)
    size_t vertexCount = 0;   // number of vertices
    size_t indexOffset = 0;   // start index inside big EBO (index count units)
    size_t indexCount = 0;    // usually (vertexCount - 1) * 2 for lines
    std::vector<float> verts; // cpu-side copy: x,y pairs: size == vertexCount*2
};

struct FreeBlock {
    size_t offset;
    size_t length; // in vertex units
};

// -----------------------------
// Simple first-fit allocator + merge
// -----------------------------
bool allocateFreeBlock(std::vector<FreeBlock>& freeList, size_t need, size_t& outOffset) {
    for (auto it = freeList.begin(); it != freeList.end(); ++it) {
        if (it->length >= need) {
            outOffset = it->offset;
            if (it->length == need) {
                freeList.erase(it);
            } else {
                it->offset += need;
                it->length -= need;
            }
            return true;
        }
    }
    return false;
}

void freeBlock(std::vector<FreeBlock>& freeList, size_t offset, size_t length) {
    freeList.push_back({offset, length});
    // merge adjacent
    std::sort(freeList.begin(), freeList.end(), [](const FreeBlock&a,const FreeBlock&b){ return a.offset < b.offset; });
    for (size_t i = 0; i + 1 < freeList.size(); ) {
        if (freeList[i].offset + freeList[i].length == freeList[i+1].offset) {
            freeList[i].length += freeList[i+1].length;
            freeList.erase(freeList.begin() + i + 1);
        } else i++;
    }
}

// -----------------------------
// Shader
// -----------------------------
static const char* vs_src = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
void main() { gl_Position = vec4(aPos, 0.0, 1.0); }
)";
static const char* fs_src = R"(
#version 330 core
out vec4 FragColor;
void main() { FragColor = vec4(1.0,1.0,1.0,1.0); }
)";

GLuint compileShader(GLenum t, const char* src) {
    GLuint s = glCreateShader(t);
    glShaderSource(s,1,&src,nullptr);
    glCompileShader(s);
    int ok; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) { char buf[512]; glGetShaderInfoLog(s,512,nullptr,buf); std::cerr<<"Shader err: "<<buf<<"\n"; }
    return s;
}
GLuint buildProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, vs_src);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fs_src);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs); glAttachShader(p, fs); glLinkProgram(p);
    glDeleteShader(vs); glDeleteShader(fs);
    int ok; glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) { char buf[512]; glGetProgramInfoLog(p,512,nullptr,buf); std::cerr<<"Link err: "<<buf<<"\n"; }
    return p;
}

// -----------------------------
// Helpers: random polyline vertices
// -----------------------------
std::vector<float> randomPolylineVerts(int pts) {
    std::vector<float> v(pts*2);
    for (int i=0;i<pts;i++) {
        v[i*2+0] = ((rand()%2000)/1000.0f) - 1.0f;
        v[i*2+1] = ((rand()%2000)/1000.0f) - 1.0f;
    }
    return v;
}

// -----------------------------
// Defragmentation: compact all active polylines into start of buffer
// - We will map entire VBO and EBO with GL_MAP_WRITE_BIT|GL_MAP_INVALIDATE_BUFFER_BIT
// - Copy CPU-side verts/indices sequentially to mapped memory
// - Update each polyline's vboOffset/indexOffset accordingly
// - Rebuild free list (single free block at the end)
// -----------------------------
void defragmentBuffers(GLuint VBO, GLuint EBO,
                       std::vector<Polyline>& polylines,
                       std::vector<FreeBlock>& freeList,
                       size_t MaxVertices, size_t MaxIndices,
                       size_t &eboUsedCount)
{
    // compute new offsets
    size_t nextV = 0;
    size_t nextI = 0;
    for (auto &p : polylines) {
        p.vboOffset = nextV;
        p.indexOffset = nextI;
        nextV += p.vertexCount;
        nextI += p.indexCount;
    }

    // Map VBO entire range for write (invalidate)
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    void* vptr = glMapBufferRange(GL_ARRAY_BUFFER, 0, MaxVertices * sizeof(float)*2,
                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (!vptr) { std::cerr << "VBO map failed\n"; return; }

    // Map EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    void* iptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, MaxIndices * sizeof(unsigned int),
                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    if (!iptr) { std::cerr << "EBO map failed\n"; glUnmapBuffer(GL_ARRAY_BUFFER); return; }

    // Copy data sequentially
    char* vwrite = (char*)vptr;
    unsigned int* iwrite = (unsigned int*)iptr;

    for (auto &p : polylines) {
        size_t vbytes = p.verts.size() * sizeof(float);
        memcpy(vwrite + p.vboOffset * sizeof(float) * 2, p.verts.data(), vbytes);

        // build indices for this polyline
        // each segment is two indices (u, u+1)
        for (size_t s = 0; s + 1 < p.vertexCount; ++s) {
            size_t idxPos = p.indexOffset + s*2;
            iwrite[idxPos + 0] = (unsigned int)(p.vboOffset + s);
            iwrite[idxPos + 1] = (unsigned int)(p.vboOffset + s + 1);
        }
    }

    // unmap
    glUnmapBuffer(GL_ARRAY_BUFFER);
    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

    // rebuild freeList: single free block at nextV..MaxVertices
    freeList.clear();
    if (nextV < MaxVertices) freeList.push_back({ nextV, MaxVertices - nextV });

    eboUsedCount = nextI;
    // done
}

// -----------------------------
// Main
// -----------------------------
int main() {
    srand((unsigned)time(nullptr));

    if (!glfwInit()) return -1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280,720,"Polyline Defrag + MapBufferRange", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // disable vsync to see actual FPS

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr<<"Failed to init glad\n"; return -1;
    }
    
    // 输出 OpenGL 信息
    {
        std::cout << "=== OpenGL Information ===" << std::endl;
        std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
        std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
        std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
        std::cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
        std::cout << "===================" << std::endl;
    }

    GLuint program = buildProgram();
    glUseProgram(program);

    // pool sizes
    const size_t MaxVertices = 200000; // vertex count (pairs x,y)
    const size_t MaxIndices  = 400000; // indices count

    // create VBO/EBO/VAO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // allocate entire buffer (float x,y) -> 2 floats per vertex
    glBufferData(GL_ARRAY_BUFFER, MaxVertices * sizeof(float) * 2, nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1,&EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MaxIndices * sizeof(unsigned int), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),(void*)0);

    // allocator free list (initially whole VBO free)
    std::vector<FreeBlock> freeList;
    freeList.push_back({0, MaxVertices});

    std::vector<Polyline> polylines;
    size_t eboUsedCount = 0;

    // create some initial polylines (CPU data + upload)
    for (int i=0;i<500;i++) {
        int pts = 4 + rand()%12;
        auto verts = randomPolylineVerts(pts);

        size_t vOffset;
        if (!allocateFreeBlock(freeList, pts, vOffset)) break;

        Polyline P;
        P.vertexCount = pts;
        P.vboOffset = vOffset;
        P.indexOffset = eboUsedCount;
        P.indexCount = (pts>1) ? (pts-1)*2 : 0;
        P.verts = verts;

        // upload vertex bytes using glMapBufferRange for the small range
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,
                                     P.vboOffset * sizeof(float) * 2,
                                     P.verts.size() * sizeof(float),
                                     GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (ptr) {
            memcpy(ptr, P.verts.data(), P.verts.size()*sizeof(float));
            glUnmapBuffer(GL_ARRAY_BUFFER);
        } else {
            // fallback
            glBufferSubData(GL_ARRAY_BUFFER, P.vboOffset * sizeof(float)*2, P.verts.size()*sizeof(float), P.verts.data());
        }

        // build indices (on CPU) and upload
        std::vector<unsigned int> idx(P.indexCount);
        for (int j=0;j<pts-1;j++){
            idx[j*2+0] = (unsigned int)(P.vboOffset + j);
            idx[j*2+1] = (unsigned int)(P.vboOffset + j + 1);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        void* iptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,
                                      P.indexOffset * sizeof(unsigned int),
                                      idx.size() * sizeof(unsigned int),
                                      GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
        if (iptr) {
            memcpy(iptr, idx.data(), idx.size()*sizeof(unsigned int));
            glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
        } else {
            glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, P.indexOffset*sizeof(unsigned int), idx.size()*sizeof(unsigned int), idx.data());
        }

        eboUsedCount += P.indexCount;
        polylines.push_back(std::move(P));
    }

    // FPS
    double fpsTimer = glfwGetTime(); int frameCount = 0;

    // fragmentation detection: trigger defrag if freeList.size() too big or free small pieces sum large
    const size_t FRAG_THRESHOLD = 20; // if more than 20 free blocks -> compact
    const double DEFRAG_INTERVAL = 5.0; // at most once per 5s
    double lastDefrag = glfwGetTime();

    // main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // ---- randomly update some polylines (CPU + GPU write via glMapBufferRange) ----
        int updates = 5 + rand()%20;
        for (int u=0; u<updates; ++u) {
            if (polylines.empty()) break;
            int id = rand() % polylines.size();
            Polyline &P = polylines[id];
            // change a few vertices
            for (int k=0;k< (int)P.vertexCount; ++k) {
                if (rand()%4==0) { // ~25% of points jitter
                    P.verts[k*2+0] = ((rand()%2000)/1000.0f) - 1.0f;
                    P.verts[k*2+1] = ((rand()%2000)/1000.0f) - 1.0f;
                }
            }
            // upload just this polyline's vertex region
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,
                                         P.vboOffset * sizeof(float) * 2,
                                         P.verts.size() * sizeof(float),
                                         GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
            if (ptr) {
                memcpy(ptr, P.verts.data(), P.verts.size()*sizeof(float));
                glUnmapBuffer(GL_ARRAY_BUFFER);
            } else {
                glBufferSubData(GL_ARRAY_BUFFER, P.vboOffset * sizeof(float)*2, P.verts.size()*sizeof(float), P.verts.data());
            }
        }

        // ---- periodically add/remove polylines ----
        static double opTimer = glfwGetTime();
        if (glfwGetTime() - opTimer > 1.0) {
            opTimer = glfwGetTime();
            if (rand()%2==0) {
                // add
                int pts = 4 + rand()%12;
                auto verts = randomPolylineVerts(pts);
                size_t vOffset;
                if (allocateFreeBlock(freeList, pts, vOffset) && eboUsedCount + (pts-1)*2 < MaxIndices) {
                    Polyline P;
                    P.vertexCount = pts;
                    P.vboOffset = vOffset;
                    P.indexOffset = eboUsedCount;
                    P.indexCount = (pts>1) ? (pts-1)*2 : 0;
                    P.verts = verts;

                    // upload vertex region
                    glBindBuffer(GL_ARRAY_BUFFER, VBO);
                    void* ptr = glMapBufferRange(GL_ARRAY_BUFFER,
                                                 P.vboOffset * sizeof(float) * 2,
                                                 P.verts.size() * sizeof(float),
                                                 GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
                    if (ptr) { memcpy(ptr, P.verts.data(), P.verts.size()*sizeof(float)); glUnmapBuffer(GL_ARRAY_BUFFER); }
                    else glBufferSubData(GL_ARRAY_BUFFER, P.vboOffset * sizeof(float)*2, P.verts.size()*sizeof(float), P.verts.data());

                    // indices
                    std::vector<unsigned int> idx(P.indexCount);
                    for (int j=0;j<pts-1;j++) { idx[j*2+0] = (unsigned int)(P.vboOffset + j); idx[j*2+1] = (unsigned int)(P.vboOffset + j + 1); }
                    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
                    void* iptr = glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER,
                                                  P.indexOffset * sizeof(unsigned int),
                                                  idx.size() * sizeof(unsigned int),
                                                  GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
                    if (iptr) { memcpy(iptr, idx.data(), idx.size()*sizeof(unsigned int)); glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER); }
                    else glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, P.indexOffset*sizeof(unsigned int), idx.size()*sizeof(unsigned int), idx.data());

                    eboUsedCount += P.indexCount;
                    polylines.push_back(std::move(P));
                }
            } else {
                // remove
                if (!polylines.empty()) {
                    int id = rand() % polylines.size();
                    Polyline P = polylines[id];
                    freeBlock(freeList, P.vboOffset, P.vertexCount);
                    // For simplicity rebuild eboUsedCount only on defrag; here we mark indices as unused by reducing eboUsedCount only if last
                    // We'll leave "holes" in EBO; defragment will compact them.
                    polylines.erase(polylines.begin() + id);
                }
            }
        }

        // ---- check fragmentation and maybe defrag ----
        bool needDefrag = false;
        if (freeList.size() > FRAG_THRESHOLD && glfwGetTime() - lastDefrag > DEFRAG_INTERVAL) {
            needDefrag = true;
        }
        // also check if total free space is small but fragmented into many pieces
        if (needDefrag) {
            // recompact
            defragmentBuffers(VBO, EBO, polylines, freeList, MaxVertices, MaxIndices, eboUsedCount);
            lastDefrag = glfwGetTime();
            std::cout << "[Defrag] done. polylines=" << polylines.size() << " eboUsed=" << eboUsedCount << " freeBlocks=" << freeList.size() << "\n";
        }

        // ---- render ----
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        glDrawElements(GL_LINES, (GLsizei)eboUsedCount, GL_UNSIGNED_INT, 0);

        // ---- FPS ---
        frameCount++;
        double now = glfwGetTime();
        if (now - fpsTimer >= 0.5) {
            double fps = frameCount / (now - fpsTimer);
            fpsTimer = now; frameCount = 0;
            char title[256];
            sprintf(title, "Polylines: %zu  FPS: %.1f  FreeBlocks: %zu", polylines.size(), fps, freeList.size());
            glfwSetWindowTitle(window, title);
        }

        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}
