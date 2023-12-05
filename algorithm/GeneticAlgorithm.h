#ifndef _GeneticAlgorithm_H_
#define _GeneticAlgorithm_H_

//在遗传算法中，变异是指对个体进行随机操作以产生新的个体，从而提高种群的多样性。
//变异操作可以根据需要进行多种设计，常用的变异操作包括：
//
//位变异：在个体的染色体中随机选择一个位置，然后将该位置的值进行随机替换。
//交换变异：在个体的染色体中随机选择两个位置，然后将这两个位置的值进行交换。
//插入变异：在个体的染色体中随机选择一个位置，然后将一个随机值插入到该位置之后。
//删除变异：在个体的染色体中随机选择一个位置，然后删除该位置之后的值。
//在此代码中，变异操作采用位变异，即在个体的染色体中随机选择一个位置，然后将该位置的值进行随机替换。

#define CITY_150NUM 150				// TSP_城市个数
#define GROUP_30NUM 30				// 群体规模
#define SON_32NUM 32				// 产生儿子的个数	SON_32NUM = GROUP_30NUM + 2

const double MAX_INT = 9999999.0;
const double P_COPULATION = 0.8;	// 杂交概率
const double P_INHERIATANCE = 0.01;	// 变异概率,即每 100 个个体中有一个个体会进行变异。
const int ITERATION_NUM = 1500;		// 遗传次数(迭代次数)

////////////////////////////////////////////////////////
//定义了图的结构体 Graph，包括顶点数、边数、顶点向量和邻接矩阵。

typedef struct
{
	int vexNum;				// 顶点数，表示图中城市的数量。
	int arcNum;				// 边数，这里对于TSP问题，边的数量是确定的，即每个城市与其他所有城市都有一条边。
	int vexs[CITY_150NUM];					// 顶点向量，存储城市名称
	double arcs[CITY_150NUM][CITY_150NUM];	// 邻接矩阵
}Graph;

typedef struct
{
	double pathLen;				// 路径长度
	int pathArray[CITY_150NUM];	// 路径数组,路径城市序列
	double dPReproduction;		// 繁殖概率
}TSPSolution;

////////////////////////////////////////////////////////
//全局变量:

TSPSolution g_tspGroups[GROUP_30NUM];		// 存储群体
TSPSolution g_tspSonSolution[SON_32NUM];	// 存储杂交后的个体

int g_nSonSolutionLen = 0;					// 遗传产生的孩子的个数

//用于记录交叉过程中的索引
int g_nCrossI;
int g_nCrossJ;

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

// 返回冲突的数组
int* GetConflict(int nFatherConflictArray[], int nMotherConflictArray[], int nCrossLength, int& nLengthConflict);

// 解决冲突
TSPSolution HandleConflict(Graph* pGraph, TSPSolution conflictSolution, int* nDetectionConflictArray, int* nModelConflictArray, int nLengthConflict);

//计算每个个体的繁殖概率，用于后续选择操作。
void CalcProbablity(Graph* pGraph, double dTotalLength);	// 计算概率

//检查路径是否合法，即是否存在重复的城市。
bool CheckPath(Graph* pGraph, TSPSolution& curSolution);

//void Display(Graph* pGraph);


#endif	// _GeneticAlgorithm_H_
