# OpenGL 绘制函数

本文档汇总了OpenGL中的绘制函数，按功能类别组织，便于查阅和理解不同绘制方式的特点和使用场景。

## 1. 基础绘制

### glDrawArrays
- **OpenGL版本**: 1.1+
- **函数原型**: `void glDrawArrays(GLenum mode, GLint first, GLsizei count)`
- **核心参数说明**: `mode`: 图元类型；`first`: 起始顶点索引；`count`: 顶点数量
- **使用场景与特点**: 直接按顺序从顶点缓冲区读取数据，无索引复用，适合简单几何体

### glDrawElements
- **OpenGL版本**: 1.1+
- **函数原型**: `void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices)`
- **核心参数说明**: `indices`: 索引缓冲区指针/偏移；`type`: 索引类型(GL_UNSIGNED_BYTE/SHORT/INT)
- **使用场景与特点**: 通过索引复用顶点数据，减少内存占用，是复杂模型的标准绘制方式

## 2. 带偏移基础绘制

### glDrawElementsBaseVertex
- **OpenGL版本**: 3.2+
- **函数原型**: `void glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)`
- **核心参数说明**: `basevertex`: 顶点缓冲区偏移量，索引值会加上此偏移
- **使用场景与特点**: 将多个子网格合并到单一VBO时，无需重写索引值，只需调整basevertex

### glDrawArraysBaseInstance
- **OpenGL版本**: 4.2+
- **函数原型**: `void glDrawArraysBaseInstance(GLenum mode, GLint first, GLsizei count, GLuint baseInstance)`
- **核心参数说明**: `baseInstance`: 实例数据起始偏移
- **使用场景与特点**: 基础绘制时支持实例数据偏移，适合与多实例渲染配合使用

### glDrawRangeElements
- **OpenGL版本**: 1.2+
- **函数原型**: `void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)`
- **核心参数说明**: `start`/`end`: 索引最小/最大值，用于驱动优化
- **使用场景与特点**: 相比glDrawElements提供索引范围提示，可能被驱动用于预取优化，性能潜力有限

### glDrawRangeElementsBaseVertex
- **OpenGL版本**: 3.2+
- **函数原型**: `void glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)`
- **核心参数说明**: 结合范围提示与basevertex偏移
- **使用场景与特点**: 同DrawRangeElements，但支持顶点偏移，适合合并VBO场景

## 3. 多组绘制(Direct)

### glMultiDrawArrays
- **OpenGL版本**: 1.4+
- **函数原型**: `void glMultiDrawArrays(GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)`
- **核心参数说明**: `first[]`/`count[]`: 每组绘制的起始和数量数组；`primcount`: 组数
- **使用场景与特点**: 单函数调用绘制多个不相连的图元组，减少API调用开销，但参数仍在CPU端

### glMultiDrawElements
- **OpenGL版本**: 1.4+
- **函数原型**: `void glMultiDrawElements(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei primcount)`
- **核心参数说明**: `indices[]`: 每组索引指针数组
- **使用场景与特点**: 多组索引绘制，适合批量渲染多个小物体，但indices指针数组在客户端内存

### glMultiDrawElementsBaseVertex
- **OpenGL版本**: 3.2+
- **函数原型**: `void glMultiDrawElementsBaseVertex(GLenum mode, const GLsizei *count, GLenum type, const void *const *indices, GLsizei primcount, const GLint *baseVertex)`
- **核心参数说明**: `baseVertex[]`: 每组顶点偏移数组
- **使用场景与特点**: 多组绘制结合baseVertex，适合批量渲染共享VBO的多个物体

## 4. 间接绘制

### glDrawArraysIndirect
- **OpenGL版本**: 4.0+
- **函数原型**: `void glDrawArraysIndirect(GLenum mode, const void *indirect)`
- **核心参数说明**: `indirect`: 指向`DrawArraysIndirectCommand`结构体的指针/偏移
- **使用场景与特点**: 参数从GPU缓冲区读取，结构体：`{count, instanceCount, first, baseInstance}`，支持GPU驱动渲染

### glDrawElementsIndirect
- **OpenGL版本**: 4.0+
- **函数原型**: `void glDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect)`
- **核心参数说明**: `indirect`: 指向`DrawElementsInirectCommand`结构体的指针/偏移
- **使用场景与特点**: 索引版间接绘制，结构体：`{count, instanceCount, firstIndex, baseVertex, baseInstance}`

### glMultiDrawArraysIndirect
- **OpenGL版本**: 4.3+
- **函数原型**: `void glMultiDrawArraysIndirect(GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)`
- **核心参数说明**: `drawcount`: 命令数量；`stride`: 结构体步长(0=紧密排列)
- **使用场景与特点**: 单调用执行多个glDrawArraysIndirect命令，极大降低CPU开销，适合GPU剔除

### glMultiDrawElementsIndirect
- **OpenGL版本**: 4.3+
- **函数原型**: `void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)`
- **核心参数说明**: 同上
- **使用场景与特点**: 单调用执行多个glDrawElementsIndirect命令，批量渲染首选方案

## 5. 多实例渲染

### glDrawArraysInstanced
- **OpenGL版本**: 3.1+ / OpenGL ES 3.0+
- **函数原型**: `void glDrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei primCount)`
- **核心参数说明**: `primCount`: 实例数量
- **使用场景与特点**: 绘制同一几何体的多个实例，通过`gl_InstanceID`在着色器中区分

### glDrawElementsInstanced
- **OpenGL版本**: 3.1+ / OpenGL ES 3.0+
- **函数原型**: `void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primCount)`
- **核心参数说明**: 同上，索引版本
- **使用场景与特点**: 索引版多实例渲染，适合渲染大量相同网格

### glDrawArraysInstancedBaseVertex
- **OpenGL版本**: 4.3+
- **函数原型**: `void glDrawArraysInstancedBaseVertex(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount, GLint baseVertex)`
- **核心参数说明**: `baseVertex`: 顶点缓冲区偏移量；`instanceCount`: 实例数量
- **使用场景与特点**: 多实例渲染时支持顶点偏移，适合合并多个网格的情况

### glDrawElementsInstancedBaseVertex
- **OpenGL版本**: 3.2+
- **函数原型**: `void glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instanceCount, GLint baseVertex)`
- **核心参数说明**: 结合实例化与baseVertex偏移
- **使用场景与特点**: 多实例渲染时支持顶点偏移

### glDrawArraysInstancedBaseInstance
- **OpenGL版本**: 4.2+
- **函数原型**: `void glDrawArraysInstancedBaseInstance(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount, GLuint baseInstance)`
- **核心参数说明**: `baseInstance`: 实例数据起始偏移
- **使用场景与特点**: 多实例渲染时支持实例数据偏移，配合`gl_InstanceID + baseInstance`访问uniform数组

### glDrawElementsInstancedBaseInstance
- **OpenGL版本**: 4.2+
- **函数原型**: `void glDrawElementsInstancedBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instanceCount, GLuint baseInstance)`
- **核心参数说明**: 同上，索引版本
- **使用场景与特点**: 索引版多实例+baseInstance

### glDrawElementsInstancedBaseVertexBaseInstance
- **OpenGL版本**: 4.2+
- **函数原型**: `void glDrawElementsInstancedBaseVertexBaseInstance(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instanceCount, GLint baseVertex, GLuint baseInstance)`
- **核心参数说明**: 同时支持顶点偏移和实例偏移
- **使用场景与特点**: 功能最全面的实例化绘制函数

## 6. 图元重启

### glPrimitiveRestartIndex
- **OpenGL版本**: 3.1+ / OpenGL ES 3.0+
- **函数原型**: `void glPrimitiveRestartIndex(GLuint index)`
- **核心参数说明**: `index`: 作为重启标志的特殊索引值
- **使用场景与特点**: 在索引绘制中，遇到此索引值时自动结束当前图元并开始新图元，需配合`glEnable(GL_PRIMITIVE_RESTART)`使用

### 图元重启使用示例
```cpp
// 启用图元重启
GLuint restartIndex = 0xFFFFFFFF; // 对于GL_UNSIGNED_INT类型索引
GLuint restartIndex_16 = 0xFFFF;   // 对于GL_UNSIGNED_SHORT类型索引

// 设置重启索引
// 在绘制之前
// glEnable(GL_PRIMITIVE_RESTART); // 使用glPrimitiveRestartIndex的索引值
// 或
// glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX); // 使用固定索引值0xFFFFFFFF

glPrimitiveRestartIndex(restartIndex);
glEnable(GL_PRIMITIVE_RESTART);

// 绘制（顶点数据和索引中包含重启索引）
glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);

// 绘制完成后禁用
// glDisable(GL_PRIMITIVE_RESTART);
```

### 适用图元类型
- **有效图元类型**: GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_LINE_STRIP, GL_LINE_LOOP, GL_TRIANGLE_STRIP_ADJACENCY, GL_LINE_STRIP_ADJACENCY
- **无效图元类型**: GL_TRIANGLES, GL_LINES, GL_POINTS等独立图元（无需重启机制）

## 7. 其他绘制函数

### glDrawTransformFeedback
- **OpenGL版本**: 3.0+
- **函数原型**: `void glDrawTransformFeedback(GLenum mode, GLuint id)`
- **核心参数说明**: `id`: 变换反馈对象ID
- **使用场景与特点**: 直接使用变换反馈缓冲区中的顶点数据进行绘制，无需手动绑定VBO

### glDrawTransformFeedbackStream
- **OpenGL版本**: 4.0+
- **函数原型**: `void glDrawTransformFeedbackStream(GLenum mode, GLuint id, GLuint stream)`
- **核心参数说明**: `stream`: 变换反馈流索引
- **使用场景与特点**: 绘制变换反馈的特定流数据，支持多流变换反馈

### glDrawTransformFeedbackInstanced
- **OpenGL版本**: 4.0+
- **函数原型**: `void glDrawTransformFeedbackInstanced(GLenum mode, GLuint id, GLsizei primCount)`
- **核心参数说明**: `primCount`: 实例数量
- **使用场景与特点**: 结合变换反馈和实例化渲染，适合GPU粒子系统

### glDrawTransformFeedbackStreamInstanced
- **OpenGL版本**: 4.0+
- **函数原型**: `void glDrawTransformFeedbackStreamInstanced(GLenum mode, GLuint id, GLuint stream, GLsizei primCount)`
- **核心参数说明**: 结合特定流和实例化
- **使用场景与特点**: 更灵活的多流变换反馈实例化绘制

## 8. OpenGL ES兼容性说明

### 基础绘制兼容性
- **glDrawArrays**: OpenGL ES 1.0+
- **glDrawElements**: OpenGL ES 1.0+
- **glDrawElementsBaseVertex**: OpenGL ES 3.2+

### 多实例渲染兼容性
- **glDrawArraysInstanced**: OpenGL ES 3.0+
- **glDrawElementsInstanced**: OpenGL ES 3.0+
- **glDrawArraysInstancedBaseInstance**: OpenGL ES 3.2+
- **glDrawElementsInstancedBaseInstance**: OpenGL ES 3.2+

### 间接绘制兼容性
- **glDrawArraysIndirect**: OpenGL ES 3.1+
- **glDrawElementsIndirect**: OpenGL ES 3.1+
- **glMultiDrawArraysIndirect**: OpenGL ES 3.1+
- **glMultiDrawElementsIndirect**: OpenGL ES 3.1+


# OpenGL 数据传输函数

本文档汇总了OpenGL中用于将数据从CPU传输到GPU的函数，按功能类别组织，便于查阅和理解不同数据传输方式的特点和使用场景。

## 1. 缓冲区数据传输函数

### 基本缓冲区数据传输

#### glBufferData
- **OpenGL版本**: 1.0+ / OpenGL ES 1.0+
- **函数原型**: `void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)`
- **核心参数说明**:
  - `target`: 缓冲区绑定目标(GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER等)
  - `size`: 数据大小(字节)
  - `data`: 指向CPU数据的指针(NULL表示只分配空间)
  - `usage`: 使用模式(GL_STATIC_DRAW, GL_DYNAMIC_DRAW, GL_STREAM_DRAW等)
- **使用场景与特点**: 初始化或完全重写缓冲区数据，会分配新内存

#### glBufferSubData
- **OpenGL版本**: 1.1+ / OpenGL ES 2.0+
- **函数原型**: `void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const void *data)`
- **核心参数说明**:
  - `offset`: 缓冲区偏移量(字节)
  - `size`: 更新数据大小(字节)
  - `data`: 指向CPU数据的指针
- **使用场景与特点**: 更新缓冲区部分数据，不会重新分配内存，要求缓冲区已存在且大小足够

#### glNamedBufferData
- **OpenGL版本**: 4.5+ / OpenGL ES 3.2+
- **函数原型**: `void glNamedBufferData(GLuint buffer, GLsizeiptr size, const void *data, GLenum usage)`
- **核心参数说明**: 与glBufferData类似，但直接通过buffer ID操作，无需绑定
- **使用场景与特点**: 核心配置文件推荐的缓冲区创建方式，更安全且灵活

#### glNamedBufferSubData
- **OpenGL版本**: 4.5+ / OpenGL ES 3.2+
- **函数原型**: `void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data)`
- **核心参数说明**: 与glBufferSubData类似，但直接通过buffer ID操作
- **使用场景与特点**: 核心配置文件推荐的缓冲区更新方式

### 缓冲区映射函数

#### glMapBuffer
- **OpenGL版本**: 1.4+ / OpenGL ES 3.0+
- **函数原型**: `void *glMapBuffer(GLenum target, GLenum access)`
- **核心参数说明**:
  - `access`: 映射访问模式(GL_READ_ONLY, GL_WRITE_ONLY, GL_READ_WRITE)
- **使用场景与特点**: 将GPU缓冲区映射到CPU地址空间，允许直接读写，使用后需调用glUnmapBuffer

#### glMapBufferRange
- **OpenGL版本**: 3.0+ / OpenGL ES 3.0+
- **函数原型**: `void *glMapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)`
- **核心参数说明**:
  - `offset`: 映射起始偏移(字节)
  - `length`: 映射长度(字节)
  - `access`: 访问标志位组合(GL_MAP_READ_BIT, GL_MAP_WRITE_BIT等)
- **使用场景与特点**: 更灵活的缓冲区映射，支持范围映射和多种访问控制

#### glNamedBufferMap
- **OpenGL版本**: 4.5+
- **函数原型**: `void *glNamedBufferMap(GLuint buffer, GLenum access)`
- **核心参数说明**: 与glMapBuffer类似，但直接通过buffer ID操作
- **使用场景与特点**: 核心配置文件推荐的缓冲区映射方式

#### glNamedBufferMapRange
- **OpenGL版本**: 4.5+
- **函数原型**: `void *glNamedBufferMapRange(GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)`
- **核心参数说明**: 与glMapBufferRange类似，但直接通过buffer ID操作
- **使用场景与特点**: 核心配置文件推荐的范围映射方式

#### glUnmapBuffer
- **OpenGL版本**: 1.4+ / OpenGL ES 3.0+
- **函数原型**: `GLboolean glUnmapBuffer(GLenum target)`
- **核心参数说明**: - 
- **使用场景与特点**: 解除缓冲区映射，返回映射是否成功

#### glUnmapNamedBuffer
- **OpenGL版本**: 4.5+
- **函数原型**: `GLboolean glUnmapNamedBuffer(GLuint buffer)`
- **核心参数说明**: - 
- **使用场景与特点**: 解除具名缓冲区映射

### 缓冲区复制与填充

#### glCopyBufferSubData
- **OpenGL版本**: 3.1+ / OpenGL ES 3.1+
- **函数原型**: `void glCopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)`
- **核心参数说明**:
  - `readTarget`/`writeTarget`: 源和目标缓冲区绑定点
  - `readOffset`/`writeOffset`: 源和目标偏移量
  - `size`: 复制数据大小
- **使用场景与特点**: GPU内部缓冲区数据复制，无需CPU参与，高效

#### glCopyNamedBufferSubData
- **OpenGL版本**: 4.5+
- **函数原型**: `void glCopyNamedBufferSubData(GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)`
- **核心参数说明**: 与glCopyBufferSubData类似，但使用具名缓冲区
- **使用场景与特点**: 核心配置文件推荐的缓冲区复制方式

#### glClearBufferData
- **OpenGL版本**: 4.4+ / OpenGL ES 3.2+
- **函数原型**: `void glClearBufferData(GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data)`
- **核心参数说明**:
  - `internalformat`: 缓冲区内部格式
  - `format`/`type`: 源数据格式和类型
  - `data`: 填充值数据
- **使用场景与特点**: 使用CPU提供的值填充整个缓冲区

#### glClearBufferSubData
- **OpenGL版本**: 4.4+ / OpenGL ES 3.2+
- **函数原型**: `void glClearBufferSubData(GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data)`
- **核心参数说明**:
  - `offset`/`size`: 填充范围
- **使用场景与特点**: 使用CPU提供的值填充缓冲区指定区域

## 2. 纹理数据传输函数

### 基本纹理数据传输

#### glTexImage1D
- **OpenGL版本**: 1.0+ / OpenGL ES 1.0+
- **函数原型**: `void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *data)`
- **核心参数说明**:
  - `target`: 纹理目标(GL_TEXTURE_1D等)
  - `level`: 纹理级别
  - `internalformat`: 内部格式
  - `width`: 纹理宽度
  - `format`/`type`: 源数据格式和类型
  - `data`: 指向纹理数据的指针
- **使用场景与特点**: 创建或替换1D纹理数据

#### glTexImage2D
- **OpenGL版本**: 1.0+ / OpenGL ES 1.0+
- **函数原型**: `void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexImage1D类似，增加height参数
- **使用场景与特点**: 创建或替换2D纹理数据

#### glTexImage3D
- **OpenGL版本**: 1.2+ / OpenGL ES 3.0+
- **函数原型**: `void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexImage2D类似，增加depth参数
- **使用场景与特点**: 创建或替换3D纹理数据

#### glTexSubImage1D
- **OpenGL版本**: 1.1+ / OpenGL ES 1.0+
- **函数原型**: `void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *data)`
- **核心参数说明**:
  - `xoffset`: 水平偏移量
- **使用场景与特点**: 更新1D纹理的部分区域

#### glTexSubImage2D
- **OpenGL版本**: 1.1+ / OpenGL ES 1.0+
- **函数原型**: `void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data)`
- **核心参数说明**:
  - `xoffset`/`yoffset`: 水平和垂直偏移量
- **使用场景与特点**: 更新2D纹理的部分区域

#### glTexSubImage3D
- **OpenGL版本**: 1.2+ / OpenGL ES 3.0+
- **函数原型**: `void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data)`
- **核心参数说明**:
  - `xoffset`/`yoffset`/`zoffset`: 三个维度的偏移量
- **使用场景与特点**: 更新3D纹理的部分区域

### 纹理对象操作

#### glTextureImage1D
- **OpenGL版本**: 4.5+
- **函数原型**: `void glTextureImage1D(GLuint texture, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexImage1D类似，但直接通过texture ID操作
- **使用场景与特点**: 核心配置文件推荐的纹理创建方式

#### glTextureImage2D
- **OpenGL版本**: 4.5+
- **函数原型**: `void glTextureImage2D(GLuint texture, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexImage2D类似，但直接通过texture ID操作
- **使用场景与特点**: 核心配置文件推荐的2D纹理创建方式

#### glTextureImage3D
- **OpenGL版本**: 4.5+
- **函数原型**: `void glTextureImage3D(GLuint texture, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexImage3D类似，但直接通过texture ID操作
- **使用场景与特点**: 核心配置文件推荐的3D纹理创建方式

#### glTextureSubImage1D
- **OpenGL版本**: 4.5+
- **函数原型**: `void glTextureSubImage1D(GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexSubImage1D类似，但直接通过texture ID操作
- **使用场景与特点**: 更新具名1D纹理的部分区域

#### glTextureSubImage2D
- **OpenGL版本**: 4.5+
- **函数原型**: `void glTextureSubImage2D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexSubImage2D类似，但直接通过texture ID操作
- **使用场景与特点**: 更新具名2D纹理的部分区域

#### glTextureSubImage3D
- **OpenGL版本**: 4.5+
- **函数原型**: `void glTextureSubImage3D(GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: 与glTexSubImage3D类似，但直接通过texture ID操作
- **使用场景与特点**: 更新具名3D纹理的部分区域

### 压缩纹理数据

#### glCompressedTexImage1D
- **OpenGL版本**: 1.3+ / OpenGL ES 3.0+
- **函数原型**: `void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data)`
- **核心参数说明**:
  - `internalformat`: 压缩格式(GL_COMPRESSED_RGBA_S3TC_DXT1_EXT等)
  - `imageSize`: 压缩数据大小
- **使用场景与特点**: 创建压缩1D纹理数据

#### glCompressedTexImage2D
- **OpenGL版本**: 1.3+ / OpenGL ES 3.0+
- **函数原型**: `void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)`
- **核心参数说明**: 与glCompressedTexImage1D类似，增加height参数
- **使用场景与特点**: 创建压缩2D纹理数据

#### glCompressedTexImage3D
- **OpenGL版本**: 1.3+ / OpenGL ES 3.0+
- **函数原型**: `void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)`
- **核心参数说明**: 与glCompressedTexImage2D类似，增加depth参数
- **使用场景与特点**: 创建压缩3D纹理数据

#### glCompressedTexSubImage1D
- **OpenGL版本**: 1.3+ / OpenGL ES 3.0+
- **函数原型**: `void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)`
- **核心参数说明**: - 
- **使用场景与特点**: 更新压缩1D纹理的部分区域

#### glCompressedTexSubImage2D
- **OpenGL版本**: 1.3+ / OpenGL ES 3.0+
- **函数原型**: `void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)`
- **核心参数说明**: - 
- **使用场景与特点**: 更新压缩2D纹理的部分区域

### 像素数据传输

#### glReadPixels
- **OpenGL版本**: 1.0+ / OpenGL ES 1.0+
- **函数原型**: `void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data)`
- **核心参数说明**:
  - `x`/`y`: 读取区域起始坐标
  - `width`/`height`: 读取区域大小
  - `format`/`type`: 目标数据格式和类型
  - `data`: 接收像素数据的缓冲区
- **使用场景与特点**: 从帧缓冲区读取像素数据到CPU内存

#### glReadnPixels
- **OpenGL版本**: 4.5+
- **函数原型**: `void glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)`
- **核心参数说明**: 与glReadPixels类似，增加bufSize参数
- **使用场景与特点**: 提供额外的缓冲区大小安全检查

#### glReadPixelsFormat
- **OpenGL版本**: 4.3+
- **函数原型**: `void glReadPixelsFormat(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLuint bufSize, GLuint *length, void *pixels)`
- **核心参数说明**: - 
- **使用场景与特点**: 支持格式转换的像素读取

## 3. 着色器与程序数据传输

### Uniform变量设置

#### glUniform*系列函数
- **OpenGL版本**: 2.0+ / OpenGL ES 2.0+
- **函数原型示例**:
  - `void glUniform1f(GLint location, GLfloat v0)`
  - `void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3)`
  - `void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)`
- **核心参数说明**:
  - `location`: uniform变量位置
  - `count`: 数组元素数量
  - `transpose`: 矩阵是否转置
  - `value`/各分量: 要设置的值
- **使用场景与特点**: 设置着色器uniform变量的单值或数组

#### glProgramUniform*系列函数
- **OpenGL版本**: 4.1+ / OpenGL ES 3.0+
- **函数原型示例**:
  - `void glProgramUniform1f(GLuint program, GLint location, GLfloat v0)`
  - `void glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)`
- **核心参数说明**: 与glUniform*类似，但直接指定program参数
- **使用场景与特点**: 无需激活程序即可设置uniform，更灵活

### Uniform缓冲区

#### glBindBufferRange
- **OpenGL版本**: 3.1+ / OpenGL ES 3.0+
- **函数原型**: `void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)`
- **核心参数说明**:
  - `target`: GL_UNIFORM_BUFFER
  - `index`: uniform块索引
  - `buffer`: 缓冲区ID
  - `offset`: 缓冲区偏移
  - `size`: 缓冲区大小
- **使用场景与特点**: 将缓冲区的特定范围绑定到uniform块

#### glBindBufferBase
- **OpenGL版本**: 3.1+ / OpenGL ES 3.0+
- **函数原型**: `void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)`
- **核心参数说明**: 与glBindBufferRange类似，但使用整个缓冲区
- **使用场景与特点**: 将整个缓冲区绑定到uniform块

#### glUniformBlockBinding
- **OpenGL版本**: 3.1+ / OpenGL ES 3.0+
- **函数原型**: `void glUniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)`
- **核心参数说明**:
  - `program`: 着色器程序ID
  - `uniformBlockIndex`: uniform块索引
  - `uniformBlockBinding`: uniform块绑定点
- **使用场景与特点**: 关联uniform块与绑定点

### Shader Storage缓冲区

#### glBindBufferBase (用于SSBO)
- **OpenGL版本**: 4.3+ / OpenGL ES 3.1+
- **函数原型**: `void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)`
- **核心参数说明**: target为GL_SHADER_STORAGE_BUFFER
- **使用场景与特点**: 将缓冲区绑定到着色器存储块

#### glBindBufferRange (用于SSBO)
- **OpenGL版本**: 4.3+ / OpenGL ES 3.1+
- **函数原型**: `void glBindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)`
- **核心参数说明**: target为GL_SHADER_STORAGE_BUFFER
- **使用场景与特点**: 将缓冲区特定范围绑定到着色器存储块

#### glShaderStorageBlockBinding
- **OpenGL版本**: 4.3+ / OpenGL ES 3.1+
- **函数原型**: `void glShaderStorageBlockBinding(GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)`
- **核心参数说明**:
  - `program`: 着色器程序ID
  - `storageBlockIndex`: 存储块索引
  - `storageBlockBinding`: 存储块绑定点
- **使用场景与特点**: 关联着色器存储块与绑定点

## 4. 变换反馈数据传输

#### glTransformFeedbackBufferBase
- **OpenGL版本**: 4.0+
- **函数原型**: `void glTransformFeedbackBufferBase(GLuint id, GLuint index, GLuint buffer)`
- **核心参数说明**:
  - `id`: 变换反馈对象ID
  - `index`: 变换反馈绑定点索引
  - `buffer`: 缓冲区ID
- **使用场景与特点**: 将缓冲区关联到变换反馈对象

#### glTransformFeedbackBufferRange
- **OpenGL版本**: 4.0+
- **函数原型**: `void glTransformFeedbackBufferRange(GLuint id, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)`
- **核心参数说明**:
  - `offset`/`size`: 缓冲区范围
- **使用场景与特点**: 将缓冲区特定范围关联到变换反馈对象

#### glBindTransformFeedback
- **OpenGL版本**: 3.0+ / OpenGL ES 3.0+
- **函数原型**: `void glBindTransformFeedback(GLenum target, GLuint id)`
- **核心参数说明**: 
  - `target`: GL_TRANSFORM_FEEDBACK
  - `id`: 变换反馈对象ID
- **使用场景与特点**: 绑定变换反馈对象以进行操作

## 5. 原子计数器与间接命令

#### glBindBufferBase (用于原子计数器)
- **OpenGL版本**: 4.2+
- **函数原型**: `void glBindBufferBase(GLenum target, GLuint index, GLuint buffer)`
- **核心参数说明**: target为GL_ATOMIC_COUNTER_BUFFER
- **使用场景与特点**: 将缓冲区绑定到原子计数器

#### glNamedBufferSubData (用于间接命令)
- **OpenGL版本**: 4.5+
- **函数原型**: `void glNamedBufferSubData(GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data)`
- **核心参数说明**: - 
- **使用场景与特点**: 更新间接绘制命令数据

## 6. 像素缓冲区对象(PBO)

#### glBindBuffer (用于PBO)
- **OpenGL版本**: 2.1+ / OpenGL ES 3.0+
- **函数原型**: `void glBindBuffer(GLenum target, GLuint buffer)`
- **核心参数说明**: target为GL_PIXEL_PACK_BUFFER或GL_PIXEL_UNPACK_BUFFER
- **使用场景与特点**: 绑定像素缓冲区，用于加速像素传输

#### glBufferData (用于PBO)
- **OpenGL版本**: 2.1+ / OpenGL ES 3.0+
- **函数原型**: `void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage)`
- **核心参数说明**: target为GL_PIXEL_PACK_BUFFER或GL_PIXEL_UNPACK_BUFFER
- **使用场景与特点**: 创建像素传输缓冲区

#### glReadPixels (配合PBO)
- **OpenGL版本**: 1.0+ / OpenGL ES 1.0+
- **函数原型**: `void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *data)`
- **核心参数说明**: data为缓冲区偏移而非CPU指针
- **使用场景与特点**: 将像素直接读取到GPU缓冲区

#### glTexSubImage2D (配合PBO)
- **OpenGL版本**: 1.1+ / OpenGL ES 1.0+
- **函数原型**: `void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data)`
- **核心参数说明**: data为缓冲区偏移而非CPU指针
- **使用场景与特点**: 从GPU缓冲区直接更新纹理

## 7. 数据传输优化策略

### 缓冲区使用模式建议

- **GL_STATIC_DRAW**: 数据上传一次，多次绘制(如静态几何体)
- **GL_DYNAMIC_DRAW**: 数据上传多次，多次绘制(如动态更新的场景)
- **GL_STREAM_DRAW**: 数据上传一次，绘制几次(如粒子效果)
- **GL_STATIC_READ**: 数据上传一次，多次读取(如计算结果)
- **GL_DYNAMIC_READ**: 数据上传多次，多次读取
- **GL_STREAM_READ**: 数据上传一次，读取几次
- **GL_STATIC_COPY**: 数据只在GPU内部使用
- **GL_DYNAMIC_COPY**: 数据在GPU内部多次更新
- **GL_STREAM_COPY**: 数据在GPU内部短暂使用

### 映射标志位建议

- **GL_MAP_WRITE_BIT**: 只写映射，最快
- **GL_MAP_PERSISTENT_BIT**: 持久映射，允许GPU和CPU同时访问
- **GL_MAP_COHERENT_BIT**: 连贯映射，确保CPU写操作对GPU可见
- **GL_MAP_INVALIDATE_BUFFER_BIT**: 使现有缓冲区内容无效，优化写入
- **GL_MAP_UNSYNCHRONIZED_BIT**: 不自动同步，需要手动同步

### 批量数据传输

- 合并多个小传输为一个大传输
- 使用多个缓冲区进行交替更新，避免等待
- 对频繁更新的数据使用多个缓冲区(双缓冲或环形缓冲)
- 考虑使用统一内存架构(UMA)的优势

## 8. OpenGL ES兼容性说明

### 缓冲区数据传输兼容性

- **glBufferData**: OpenGL ES 1.0+
- **glBufferSubData**: OpenGL ES 2.0+
- **glMapBufferRange**: OpenGL ES 3.0+
- **glCopyBufferSubData**: OpenGL ES 3.1+
- **glClearBufferData/glClearBufferSubData**: OpenGL ES 3.2+

### 纹理数据传输兼容性

- **glTexImage2D**: OpenGL ES 1.0+
- **glTexSubImage2D**: OpenGL ES 1.0+
- **glTexImage3D**: OpenGL ES 3.0+
- **glCompressedTexImage2D**: OpenGL ES 3.0+
- **glReadPixels**: OpenGL ES 1.0+

### 着色器数据传输兼容性

- **glUniform***: OpenGL ES 2.0+
- **glProgramUniform***: OpenGL ES 3.0+
- **glUniformBlockBinding**: OpenGL ES 3.0+
- **glBindBufferBase/BindBufferRange**: OpenGL ES 3.0+
- **Shader Storage Buffer**: OpenGL ES 3.1+

### 变换反馈兼容性

- **glBindTransformFeedback**: OpenGL ES 3.0+
- **glTransformFeedbackBufferBase**: 不支持
- **glTransformFeedbackBufferRange**: 不支持
- **使用替代方案**: `glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, index, buffer)`



