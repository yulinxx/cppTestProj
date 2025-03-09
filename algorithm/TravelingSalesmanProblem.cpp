// 模拟退火(SA, Simulated Annealing)算法解决旅行商TSP问题_模拟退火算法tsp_lamusique的博客-CSDN博客
// https://blog.csdn.net/lamusique/article/details/123721794

// 使用模拟退火(SA)算法求解旅行商(TSP)问题

// 稍作修改, 以第一点保持不变
// 算法大至思路:
// 初始化从 0 到 末尾的点, 作为一个初始数组,
// 然后随机交换数据的两个点,并计算总长
// 不停的交换,并得到最短的路径即可
// 以下为cpp代码
// 注释部分为python代码, 用于展示点以及点与点之间的连线

// 什么是旅行商问题(TSP)?
// TSP问题(Traveling Salesman Problem,旅行商问题),由威廉哈密顿爵士和英国数学家克克曼T.P.Kirkman于19世纪初提出。问题描述如下:
// 有若干个城市,任何两个城市之间的距离都是确定的,现要求一旅行商从某城市出发必须经过每一个城市且只在一个城市逗留一次,最后回到出发的城市,问如何事先确定一条最短的线路已保证其旅行的费用最少？

// 模拟退火其实也是一种贪心算法,只不过与爬山法不同的是,模拟退火算法在搜索过程引入了随机因素。模拟退火算法以一定的概率来接受一个比当前解要差的解,因此有可能会跳出这个局部的最优解,达到全局的最优解。

// 有这么一个古老问题,被称为旅行商问题 ( TSP , Traveling Salesman Problem ) ,是数学领域中著名的问题,问题内容是这样的:有这么一个旅行商人,他要拜访n个城市,但是每次走的路径是有限制的,即每个城市只能拜访一次,并且最后要回到原来出发的城市。如何选择路径使得总路程为最小值。

// 之所以著名是因为这个问题至今还没有找到一个有效的算法。如果想精确的求解只能只能通过穷举所有的路径组合但是随着城市数量的增加其复杂度会很高。

// 不过还好,我们有模拟退火算法,当然它不能精确的求解这个问题,不过他能近似的求解这个问题。具体做法如下:

// 1. 设定初始温度T0,并且随机选择一条遍历路径P(i)作为初始路径,算出其长度L(P(i));
// 2. 随机产生一条新的遍历路径P(i+1),算出其长度L(P(i + 1));
// 3. 若L(P(i + 1)) < L(P(i)),则接收P(i + 1)为新路径,否则以模拟退火的概率接收P(i + 1),然后降温
// 4. 重复步骤1和2直至温度达到最低值Tmin。

// 产生新的遍历路径的方法有很多,下面列举其中3种:

// 1. 随机选择2个节点,交换路径中的这2个节点的顺序。
// 2. 随机选择2个节点,将路径中这2个节点间的节点顺序逆转。
// 3. 随机选择3个节点m,n,k,然后将节点m与n间的节点移位到节点k后面。

// 模拟退火算法的特点
// (1) 模拟搜索算法是单体搜索算法,算法流程简单明了,通用性强,鲁棒性强,多应用于组合优化问题和非线性优化问题等。
// (2) 模拟退火算法通常运行时间略长,因为其算法的特性,初始温度设置一般较高,降温系数较小,才能更有效地收敛到最优解,再加上内部有蒙特卡洛循环,使得算法运行时间较遗传和蚁群算法来讲较长。
// (3) 模拟退火算法是被证明的可以完全收敛到最优解的算法,但收敛速度较缓慢,因为Metropolis准则的原因,有一部分解是接受的较差解,所以并不是全程都在收敛中。

///*
// * 使用模拟退火算法(SA)求解TSP问题(以中国TSP问题为例)
// * 参考自《Matlab 智能算法30个案例分析》
// */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#define TS 50000.0  // 初始温度
#define TE (1e-8)
#define q  0.98     // 退火系数
#define L 5000      // 每个温度时的迭代次数,即链长
#define N 32        // 城市数量
int cityList[N];   // 用于存放一个解

// 中国31个城市坐标
double city_pos[N][2] = {
    {900, 500},
    {1304, 2312}, {3639, 1315}, {4177, 2244}, {3712, 1399},
    {3488, 1535}, {3326, 1556}, {3238, 1229}, {4196, 1004},
    {4312, 790},  {4386, 570},  {3007, 1970}, {2562, 1756},
    {2788, 1491}, {2381, 1676}, {1332, 695},
    {3715, 1678}, {3918, 2179}, {4061, 2370},
    {3780, 2212}, {3676, 2578}, {4029, 2838},
    {4263, 2931}, {3429, 1908}, {3507, 2367},
    {3394, 2643}, {3439, 3201}, {2935, 3240},
    {3140, 3550}, {2545, 2357}, {2778, 2826},
    {2370, 2975} };

// 函数声明
double distance(double*, double*); // 计算两个城市距离
double pathLen(int*);             // 计算路径长度
void init();                       // 初始化函数
void createNew();                 // 产生新解

// 距离函数
double distance(double* city1, double* city2)
{
    double x1 = *city1;
    double y1 = *(city1 + 1);
    double x2 = *(city2);
    double y2 = *(city2 + 1);
    double dis = sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
    return dis;
}

// 计算路径长度
double pathLen(int* arr)
{
    double path = 0; // 初始化路径长度
    for (int i = 0; i < N - 1; i++)
    {
        int index1 = *(arr + i);
        int index2 = *(arr + i + 1);
        double dis = distance(city_pos[index1 - 1], city_pos[index2 - 1]);
        path += dis;
    }

    return path; // 返回总的路径长度
}

// 初始化函数
void init()
{
    for (int i = 0; i < N; i++)
        cityList[i] = i + 1; // 初始化一个解
}

// 产生一个新解
// 此处采用随机交换两个位置的方式产生新的解
void createNew()
{
    double r1, r2;
    int pos1, pos2;

    // Ensure pos1 is not equal to 1
    do
    {
        r1 = ((double)rand()) / (RAND_MAX + 1.0);
        pos1 = (int)(N * r1);
    }
    while (pos1 == 0);

    // Ensure pos2 is not equal to 1 or equal to pos1
    do
    {
        r2 = ((double)rand()) / (RAND_MAX + 1.0);
        pos2 = (int)(N * r2);
    }
    while (pos2 == 0 || pos2 == pos1);

    int temp = cityList[pos1];
    cityList[pos1] = cityList[pos2];
    cityList[pos2] = temp; // 交换两个点

    double x;
    if (pos1 == 0 || pos2 == 0)
        x = pos1 + pos2;
}

// 主函数
int main(void)
{
    srand((unsigned)time(NULL));    //初始化随机数种子

    time_t dStartTime = clock();   // 程序运行开始计时
    int nCount = 0;         // 记录降温次数
    double dStartT = TS;    //初始温度

    init();                 //初始化一个解

    int cityListCopy[N];    // 用于保存原始解
    double f1, f2;          // f1为初始解目标函数值,f2为新解目标函数值
    double df;              // df为二者差值
    double dRandom;         // 0-1之间的随机数,用来决定是否接受新解

    while (dStartT > TE)    // 当温度低于结束温度时,退火结束
    {
        for (int i = 0; i < L; i++)
        {
            // 复制数组
            memcpy(cityListCopy, cityList, N * sizeof(int));

            createNew();    // 产生新解

            f1 = pathLen(cityListCopy);
            f2 = pathLen(cityList);
            df = f2 - f1;

            // 以下是Metropolis准则
            if (df >= 0)
            {
                dRandom = ((double)rand()) / (RAND_MAX);
                if (exp(-df / dStartT) <= dRandom) // 保留原来的解
                {
                    memcpy(cityList, cityListCopy, N * sizeof(int));
                }
            }
        }

        dStartT *= q; // 降温
        nCount++;
    }

    time_t dFinishTime = clock(); // 退火过程结束
    double duration = ((double)(dFinishTime - dStartTime)) / CLOCKS_PER_SEC; // 计算时间

    printf("模拟退火算法,初始温度TS=%.2f,降温系数q=%.2f,每个温度迭代%d次,共降温%d次,得到的TSP最优路径为:\n", TS, q, L, nCount);
    for (int i = 0; i < N - 1; i++) // 输出最优路径
        printf("%d,", cityList[i]);

    printf("%d\n", cityList[N - 1]);

    double len = pathLen(cityList); // 最优路径长度
    printf("最优路径长度为:%lf\n", len);
    printf("程序运行耗时:%lf秒.\n", duration);
    return 0;
}

////////////////////////
// python代码

/*

import matplotlib.pyplot as plt

# 原始坐标数据, 和上述C++部分一致
xy = [900, 500,
      1304, 2312, 3639, 1315, 4177, 2244, 3712, 1399,
      3488, 1535, 3326, 1556, 3238, 1229, 4196, 1004,
      4312, 790, 4386, 570, 3007, 1970, 2562, 1756,
      2788, 1491, 2381, 1676, 1332, 695,
      3715, 1678, 3918, 2179, 4061, 2370,
      3780, 2212, 3676, 2578, 4029, 2838,
      4263, 2931, 3429, 1908, 3507, 2367,
      3394, 2643, 3439, 3201, 2935, 3240,
      3140, 3550, 2545, 2357, 2778, 2826,
      2370, 2975]

# 指定要连接的点的索引,将上述C++代码运行的结果替换,即索引点的连线顺序
connectPts = [

#1,16,2,15,13,14,4,19,23,22,21,18,20,25,26,27,29,28,31,32,30,12,24,7,8,6,17,5,3,9,10,11
#1,16,2,30,32,28,29,27,22,23,4,17,9,10,11,3,8,14,15,13,12,31,26,25,21,19,18,20,24,7,6,5
1,16,15,13,14,8,11,10,9,4,23,19,18,20,17,5,3,6,7,12,24,25,26,21,22,27,29,28,31,32,30,2

]

# 将坐标数据组织成 x 和 y 列表
xCoords = xy[::2]
yCoords = xy[1::2]

# 绘制散点图
plt.scatter(xCoords, yCoords, marker='o', color='blue')

# 根据指定的点连接线
xConnecting = [xCoords[i-1] for i in connectPts]
yConnecting = [yCoords[i-1] for i in connectPts]

# 绘制连接线
#plt.plot(xConnecting + [xConnecting[0]], yConnecting + [yConnecting[0]], linestyle='-', color='red')
plt.plot(xConnecting, yConnecting, linestyle='-', color='red')

# 设置图表标题和坐标轴标签
plt.title('Scatter Plot with Connecting Lines')
plt.xlabel('X Coordinate')
plt.ylabel('Y Coordinate')

# 显示图表
plt.show()

*/