邻接矩阵的定义：
```cpp
typedef struct
{
    int vertexNum;  // 顶点数
    int edgeNum;    // 边数
    vector<string> vertex;  // 顶点向量，存储城市名称
    vector<vector<int>> graphMatrix;  // 邻接矩阵，存储城市间的距离
} Graph;
``````
`vertexNum`: 顶点数，表示图中城市的数量。
`edgeNum`: 边数，这里对于TSP问题，边的数量是确定的，即每个城市与其他所有城市都有一条边。
`vertex`: 顶点向量，存储城市的名称。每个城市用一个字符串表示。
`graphMatrix`: 邻接矩阵，是一个二维向量，存储城市之间的距离信息。

#### 如何理解邻接矩阵的元素：
假设有 n 个城市，那么 graphMatrix 的第 i 行第 j 列的元素 graphMatrix[i][j] 表示城市 i 到城市 j 之间的距离。这个距离可以是实际距离，时间，成本等，具体问题中的定义可能不同。

在TSP中，邻接矩阵是对称的，因为从城市A到城市B的距离应该与从城市B到城市A的距离相同。

示例：
假设有3个城市 A、B、C，它们之间的距离如下：

```less
A到A: 0
A到B: 2
A到C: 3

B到A: 2
B到B: 0
B到C: 4

C到A: 3
C到B: 4
C到C: 0
```

对应的邻接矩阵为：

```css
Copy code
   | A  B  C
---|---------
A  | 0  2  3
B  | 2  0  4
C  | 3  4  0
``````
这个邻接矩阵表示了每两个城市之间的距离。在TSP中，算法的目标是找到一条路径，经过每个城市一次，使得路径的总距离最短。