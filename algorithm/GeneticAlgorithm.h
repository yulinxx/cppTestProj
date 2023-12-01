#ifndef _GeneticAlgorithm_H_
#define _GeneticAlgorithm_H_

#define CITY_NUM 150				// TSP_城市个数
#define GROUP_NUM 30				// 群体规模
#define SON_NUM 32					// 产生儿子的个数	SON_NUM = GROUP_NUM + 2

const double P_INHERIATANCE = 0.01;	// 变异概率
const double P_COPULATION = 0.8;	// 杂交概率
const int ITERATION_NUM = 1500;		// 遗传次数(迭代次数)
const double MAX_INT = 9999999.0;

////////////////////////////////////////////////////////
//定义了图的结构体 Graph，包括顶点数、边数、顶点向量和邻接矩阵。

typedef struct
{
	int vexNum;				// 顶点数，表示图中城市的数量。
	int arcNum;				// 边数，这里对于TSP问题，边的数量是确定的，即每个城市与其他所有城市都有一条边。
	int vexs[CITY_NUM];					// 顶点向量，存储城市名称
	double arcs[CITY_NUM][CITY_NUM];	// 邻接矩阵
}Graph;

typedef struct
{
	double pathLen;				// 路径长度
	int pathArray[CITY_NUM];	// 路径数组
	double dPReproduction;		// 繁殖概率
}TSPSolution;

////////////////////////////////////////////////////////
//全局变量:

TSPSolution gTspGroups[GROUP_NUM];		// 存储群体
TSPSolution gSonSolution[SON_NUM];		// 存储杂交后的个体

int gSonSolitonLen = 0;				// 遗传产生的孩子的个数

////////////////////////////////////////////////////////

//从文件中读取城市信息，包括顶点数、顶点信息、邻接矩阵等，存储在 Graph 结构体对象中。
void CreateGraph(Graph* pGraph);

//对每个个体随机生成路径，然后检查路径的合法性，并计算路径长度。
//计算群体的概率，并评估每个个体。
void InitialGroup(Graph* pGraph);

//计算路径长度，根据给定的图和路径信息。
double CalculateLength(Graph* pGraph, TSPSolution newSolution);

//遗传算法的演化过程，包括选择、交叉、变异和更新群体。
//循环进行一定次数的迭代，每次迭代都执行选择、交叉、变异和更新操作。
//选择使用轮盘赌选择规则。
//交叉采用中间杂交方式，处理了交叉后的冲突。
//变异随机交换两个城市的位置。
//更新群体时，根据新生成的子代替换群体中路径长度较长的个体。
void TspEvolution(Graph* pGraph);	// 模拟生物进化 - 解决TSP问题

//选择操作，使用轮盘赌选择规则，返回选中的个体索引。
int EvoSelect(Graph* pGraph);		// 选择函数

//交叉操作，使用中间杂交方式，在两个个体的交叉点之间互换基因段，处理交叉后的冲突。
void EvoCross(Graph* pGraph, TSPSolution TSP_Father, TSPSolution TSP_Mother);	// 杂交函数

//变异操作，随机交换两个城市的位置。
void EvoVariation(Graph* pGraph, int Index_Variation);	// 变异函数

//更新群体操作，根据新生成的子代替换群体中路径长度较长的个体。
void EvoUpdateGroup(Graph* pGraph);

//evolution 演变
//评价函数，选择路径最短的个体作为最优解。
void TspEvaluate(Graph* pGraph);		// TSP - 评价函数

int* GetConflict(int nConflictFather[], int nConflictMother[], int nLengthCross, int& nLengthConflict);	// 返回冲突的数组
TSPSolution HandleConflict(Graph* pGraph, TSPSolution conflictSolution, int* nDetectionConflict, int* nModelConflict, int nLengthConflict);	// 解决冲突

//计算每个个体的繁殖概率，用于后续选择操作。
void CalcProbablity(Graph* pGraph, double dTotalLength);	// 计算概率

//检查路径是否合法，即是否存在重复的城市。
bool CheckPath(Graph* pGraph, TSPSolution curSolution);

//void Display(Graph* pGraph);


#endif	// _GeneticAlgorithm_H_
