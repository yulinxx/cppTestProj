// g++ text_batch.cpp -std=c++17 -lglfw -lGLEW -lGL -o text_batch
#define STB_TRUETYPE_IMPLEMENTATION

#include "stb_truetype.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

// ---------- 全局资源 ----------
GLuint program, vao, vbo, ebo, atlasTex;
struct Glyph { float u0,v0,u1,v1, x0,y0,x1,y1, adv; };
Glyph glyphs[65536];
int winW = 1000, winH = 1000;

// ---------- 工具：读文件 ----------
std::vector<unsigned char> readFile(const char* fn){
    FILE* f = fopen(fn,"rb");
    if(!f){ printf("cannot open %s\n",fn); exit(1); }
    fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
    std::vector<unsigned char> buf(sz);
    fread(buf.data(),1,sz,f); fclose(f);
    return buf;
}

// ---------- 初始化字体 ----------
void initFont(){
    auto ttf = readFile("C:/Windows/Fonts/simhei.ttf");   // 随便一个 TTF
    // auto ttf = readFile("/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf");
    const int ATLAS_W = 2048, ATLAS_H = 2048, FONT_HEIGHT = 48;
    std::vector<unsigned char> atlas(ATLAS_W*ATLAS_H);
    std::vector<stbtt_bakedchar> baked(65536);
    stbtt_BakeFontBitmap(ttf.data(),0, (float)FONT_HEIGHT,
                         atlas.data(),ATLAS_W,ATLAS_H,  0,65536, baked.data());

    glGenTextures(1,&atlasTex);
    glBindTexture(GL_TEXTURE_2D,atlasTex);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RED,ATLAS_W,ATLAS_H,0,GL_RED,GL_UNSIGNED_BYTE,atlas.data());
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    for(int i=0;i<65536;i++){
        auto&b=baked[i];
        glyphs[i].u0 = b.x0/(float)ATLAS_W;  glyphs[i].v0 = b.y0/(float)ATLAS_H;
        glyphs[i].u1 = b.x1/(float)ATLAS_W;  glyphs[i].v1 = b.y1/(float)ATLAS_H;
        glyphs[i].x0 = b.xoff;               glyphs[i].y0 = b.yoff;
        glyphs[i].x1 = b.xoff+b.x1-b.x0;     glyphs[i].y1 = b.yoff+b.y1-b.y0;
        glyphs[i].adv= b.xadvance;
    }
}

// ---------- 拼文字 ----------
struct V{ float x,y,u,v; unsigned char r,g,b,a; };
std::vector<V> verts;
std::vector<unsigned int> idxs;
void addText(float x,float y,const char* s,unsigned col){
    while(*s){
        int c = *(unsigned char*)s++;
        auto&g=glyphs[c];
        float w = g.x1-g.x0, h = g.y1-g.y0;
        unsigned base = verts.size();
        // 分解32位颜色值为RGBA分量 (假设格式为0xAARRGGBB)
        unsigned char alpha = (col >> 24) & 0xFF;
        unsigned char red = (col >> 16) & 0xFF;
        unsigned char green = (col >> 8) & 0xFF;
        unsigned char blue = col & 0xFF;
        verts.push_back(V{x, y, g.u0, g.v0, red, green, blue, alpha});
        verts.push_back(V{x+w, y, g.u1, g.v0, red, green, blue, alpha});
        verts.push_back(V{x+w, y+h, g.u1, g.v1, red, green, blue, alpha});
        verts.push_back(V{x, y+h, g.u0, g.v1, red, green, blue, alpha});
        idxs.push_back(base+0); idxs.push_back(base+1); idxs.push_back(base+2);
        idxs.push_back(base+0); idxs.push_back(base+2); idxs.push_back(base+3);
        x += g.adv;
    }
}

// ---------- 着色器 ----------
const char* vsrc = R"(
#version 330 core
layout(location=0) in vec2 pos;
layout(location=1) in vec2 uv;
layout(location=2) in vec4 col;
out vec2 vUV;
out vec4 vCol;
uniform mat4 P;
void main(){
    gl_Position = P * vec4(pos,0,1);
    vUV=uv; vCol=col;
})";
const char* fsrc = R"(
#version 330 core
in vec2 vUV;
in vec4 vCol;
out vec4 frag;
uniform sampler2D tex;
void main(){
    float a = texture(tex,vUV).r;
    frag = vec4(vCol.rgb, vCol.a*a);
})";
GLuint compile(const char* src,GLenum type){
    GLuint s = glCreateShader(type);
    glShaderSource(s,1,&src,NULL); glCompileShader(s);
    GLint ok; glGetShaderiv(s,GL_COMPILE_STATUS,&ok);
    if(!ok){ char buf[1024]; glGetShaderInfoLog(s,sizeof(buf),NULL,buf); puts(buf); exit(1); }
    return s;
}
void initGL(){
    glewInit();
    program = glCreateProgram();
    glAttachShader(program,compile(vsrc,GL_VERTEX_SHADER));
    glAttachShader(program,compile(fsrc,GL_FRAGMENT_SHADER));
    glLinkProgram(program);
    glUseProgram(program);

    glGenVertexArrays(1,&vao);
    glGenBuffers(1,&vbo);
    glGenBuffers(1,&ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER,vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,false,sizeof(V),(void*)0);
    // uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1,2,GL_FLOAT,false,sizeof(V),(void*)8);
    // col
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2,4,GL_UNSIGNED_BYTE,true,sizeof(V),(void*)16);
}

// ---------- 每帧 ----------
void draw(){
    // 生成随机文字（只做一次）
    static bool once=false;
    if(!once){
        once=true;
        srand(time(NULL));
        const char* pool = "Hello世界OpenGL文字批量渲染1234567890";
        char line[256];
        for(int y=900;y>50;y-=60){
            int len = rand()%50+20;
            for(int i=0;i<len;i++) line[i]=pool[rand()%(strlen(pool))];
            line[len]=0;
            addText(50,(float)y,line,0xffffffff);
        }
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,verts.size()*sizeof(V),verts.data(),GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,idxs.size()*sizeof(unsigned int),idxs.data(),GL_STATIC_DRAW);
    }

    glClearColor(0.1f,0.1f,0.1f,1);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(program);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,atlasTex);
    glUniform1i(glGetUniformLocation(program,"tex"),0);
    // 正交投影 像素坐标
    float P[16]={2.0f/winW,0,0,0,  0,-2.0f/winH,0,0,  0,0,-1,0,  -1,1,0,1};
    glUniformMatrix4fv(glGetUniformLocation(program,"P"),1,GL_FALSE,P);
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES,idxs.size(),GL_UNSIGNED_INT,0);
}

// ---------- 主 ----------
int main(){
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* win = glfwCreateWindow(winW,winH,"OpenGL Text Batch",NULL,NULL);
    glfwMakeContextCurrent(win);
    initGL();
    initFont();
    while(!glfwWindowShouldClose(win)){
        glfwGetFramebufferSize(win,&winW,&winH);
        glViewport(0,0,winW,winH);
        draw();
        glfwSwapBuffers(win);
        glfwPollEvents();
    }
    return 0;
}