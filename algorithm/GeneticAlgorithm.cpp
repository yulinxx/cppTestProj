#include <algorithm>
#include <ctime>
#include <random>
#include <fstream>
#include <iomanip>	// 本文用于输出对齐
#include <iostream>
#include <stdlib.h>

#include <fstream>

#include "GA.h"


// int _iter = 0;

int main()
{
	time_t tmBegin = clock();
	Graph* pGraph = new Graph();	// 创建一个图对象

	int temp = sizeof(pGraph);

	CreateGraph(pGraph);			// 从文件中读取图数据并初始化图

	int temp2 = sizeof(pGraph);
	int temp3 = sizeof(*pGraph);

	srand(unsigned(time(0)));

	InitialGroup(pGraph);			// 使用遗传算法初始化初始解集

	// 运行 旅行推销员问题（TSP）的遗传算法进化过程
	TspEvolution(pGraph);			// 遗传算法

	delete(pGraph);
	time_t tmEnd = clock();
	double RunningTime = double(tmEnd - tmBegin) / CLOCKS_PER_SEC;
	std::cout << std::endl << "【 程序运行时间 RunningTime = " << RunningTime << " 】" << std::endl;

	system("pause");
	return 0;
}

//创建图的函数
//这个函数从文件中读取图的信息，包括顶点数、顶点信息、邻接矩阵等，并将其存储在 Graph 结构体对象中。
void CreateGraph(Graph* pGraph)
{
	std::ifstream fReadIn;
	fReadIn.open("C:/Users/april/source/repos/Win32Test/GA/city_150.txt");
	if (!fReadIn.is_open())
	{
		std::cout << "从文件中读取图数据失败." << std::endl;
		return;
	}

	fReadIn >> pGraph->vexNum;	// 存储 150
	// fReadIn >> pGraph->arcNum;
	pGraph->arcNum = 0;

	// 读取顶点信息和邻接矩阵
	for (int i = 0; i < pGraph->vexNum; i++)
	{
		fReadIn >> pGraph->vexs[i];	// 存储 1 - 150
	}
	pGraph->vexs[pGraph->vexNum] = '\0';	// char的结束符.

	// 计算边数
	for (int i = 0; i < pGraph->vexNum; i++)
	{
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			fReadIn >> pGraph->arcs[i][j];	// 保存距离

			// calculate the arcNum
			if (pGraph->arcs[i][j] > 0)
			{
				pGraph->arcNum++;
			}
		}
	}

	// 输出图的信息
	std::cout << "无向图创建完毕，相关信息如下：" << std::endl;
	std::cout << "【顶点数】 pGraph->vexNum = " << pGraph->vexNum << std::endl;
	std::cout << "【边数】 pGraph->arcNum = " << pGraph->arcNum << std::endl;
	std::cout << "【顶点向量】 vexs[max_vexNum] = ";

	for (int i = 0; i < pGraph->vexNum; i++)
	{
		std::cout << pGraph->vexs[i] << " ";
	}

	/*
	std::cout << std::endl << "【邻接矩阵】 arcs[max_vexNum][max_vexNum] 如下：" << std::endl;
	for (int i = 0; i < pGraph->vexNum; i++)
	{
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			std::cout << pGraph->arcs[i][j] << " ";
		}
		std::cout << std::endl;
	}
	*/
}

// 初始化遗传算法的个体群体
// 函数随机生成群体中每个个体的初始路径，然后检查路径的合法性，并计算路径长度。
// 接着，计算群体的概率，并评估每个个体。

void InitialGroup(Graph* pGraph)
{
	//std::cout << "----------------------【遗传算法参数】-----------------------" << std::endl;
	//std::cout << "【城市个数】 CITY_150NUM =" << CITY_150NUM << std::endl;
	//std::cout << "【群体规模】 GROUP_30NUM = " << GROUP_30NUM << std::endl;
	//std::cout << "【子代规模】 SON_32NUM = " << SON_32NUM << std::endl;
	//std::cout << "【变异概率】 P_INHERIATANCE = " << P_INHERIATANCE << std::endl;
	//std::cout << "【杂交概率】 P_COPULATION = " << P_COPULATION << std::endl;
	//std::cout << "【迭代次数】 ITERATION_NUM = " << ITERATION_NUM << std::endl;

	// 对每个个体随机生成路径
	double dTotalLength = 0.0;
	for (int i = 0; i < GROUP_30NUM; i++)
	{
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			g_tspGroups[i].pathArray[j] = pGraph->vexs[j];
		}

		// 随机打乱路径
		std::random_shuffle(g_tspGroups[i].pathArray + 1, g_tspGroups[i].pathArray + pGraph->vexNum);

		//检查路径是否合法，即是否存在重复的城市。
		if (CheckPath(pGraph, g_tspGroups[i]))
		{
			// 计算路径长度
			g_tspGroups[i].pathLen = CalculateLength(pGraph, g_tspGroups[i]);
			dTotalLength += g_tspGroups[i].pathLen;
		}
		else
		{
			std::cout << "【error！城市路径产生重复城市！】" << std::endl;
			g_tspGroups[i].pathLen = MAX_INT;
			g_tspGroups[i].dPReproduction = 0;
		}
	}

	// 计算概率并评估个体
	CalcProbablity(pGraph, dTotalLength);

	//遗传算法的演化过程函数
	TspEvaluate(pGraph);
}

// 处理对象：每次新产生的群体, 计算每个个体的概率
// 问题：解决TSP问题, 路径越短概率应该越高
// 方案：对于当前总群, 将所有个体路径取倒数, 然后乘以该总群的总路径得到大于1的值, 然后进行归一化, 取得概率
// 归一化：累加当前所有大于1的个体的伪概率, 得到nTempTotalP, 每个概率再分别除以 nTempTotalP 进行归一化
void CalcProbablity(Graph* pGraph, double dTotalLength)
{
	double dTempTotalP = 0.0;

	for (int i = 0; i < GROUP_30NUM; i++)
	{
		g_tspGroups[i].dPReproduction = (1.0 / g_tspGroups[i].pathLen) * dTotalLength;
		dTempTotalP += g_tspGroups[i].dPReproduction;
	}

	for (int i = 0; i < GROUP_30NUM; i++)
	{
		g_tspGroups[i].dPReproduction = g_tspGroups[i].dPReproduction / dTempTotalP;
	}
}

// 遗传算法的演化过程,遗传算法的演化过程的核心。
// 在每一次迭代中，首先通过选择操作选择父代个体，然后进行交叉操作生成子代个体，接着进行变异操作。
// 最后，评估每个个体的适应度，更新最优解，并进入下一轮迭代。
void TspEvolution(Graph* pGraph)
{
	int nIter = 0;
	while (nIter < ITERATION_NUM)	// 1500
	{
		// std::cout<<"***********************【第次"<<(nIter + 1)<<"迭代】*************************"<<std::endl;

		// 1. 选择
		int nFatherIndex = EvoSelect(pGraph);
		int nMotherIndex = EvoSelect(pGraph);

		while (nMotherIndex == nFatherIndex)
		{
			// 防止tspFather和tspMother都是同一个个体 -> 自交( 父母为同一个个体时, 母亲重新选择, 直到父母为不同的个体为止 )
			// std::cout<<"Warning!【nFatherIndex = nMotherIndex】"<<std::endl;
			nMotherIndex = EvoSelect(pGraph);
		}

		// g_tspGroups[]为当前总群,从总群中选择两个当作杂交个体
		TSPSolution tspFather = g_tspGroups[nFatherIndex];
		TSPSolution tspMother = g_tspGroups[nMotherIndex];

		// 2. 交叉Copulation, 存储在 g_tspSonSolution[] 数组 - 通过M次杂交, 产生2M个新个体, 2M >= GROUP_30NUM
		g_nSonSolutionLen = 0;		// 遗传产生的个体个数, 置零重新累加
		int M = GROUP_30NUM - GROUP_30NUM / 2;
		while (M)
		{
			double dIsCopulation = ((rand() % 100 + 0.0) / 100.0);	// 杂交概率
			if (dIsCopulation < P_COPULATION)
			{
				// 杂交, 将结果存储于遗传个体总群,全局变量g_tspSonSolution[]
				EvoCross(pGraph, tspFather, tspMother);
				M--;
			}
			//else// std::cout<<"[ 这两个染色体不进行杂交 ]dIsCopulation = "<<dIsCopulation<<std::endl;
		}

		// 3. 变异Variation：针对 g_tspSonSolution[]
		double dTotalLength = 0.0;	// 更新新个体的概率

		for (int nVariation = 0; nVariation < g_nSonSolutionLen; nVariation++)
		{
			double dRateVariation = (rand() % 100) / 100.0;
			if (dRateVariation < P_INHERIATANCE)				// 产生的随机数小于变异概率 则该个体进行变异
				EvoVariation(pGraph, nVariation);

			// 经过变异处理后 重新计算路径值
			if (!CheckPath(pGraph, g_tspSonSolution[nVariation]))
				std::cout << "【Error! 路径有重复!】" << std::endl;

			// 产生新个体, 计算新路径和新概率
			g_tspSonSolution[nVariation].pathLen = CalculateLength(pGraph, g_tspSonSolution[nVariation]);
			dTotalLength += g_tspSonSolution[nVariation].pathLen;
		}

		CalcProbablity(pGraph, dTotalLength);

		/*
		std::cout<<"【遗传产生的子代个体如下...】"<<std::endl;
		for (int i = 0; i < g_nSonSolutionLen; i++)
		{
			for (int j = 0;j < pGraph->vexNum;j++)
			{
				std::cout<<g_tspSonSolution[i].pathArray[j]<<" -> ";
			}
			std::cout<<g_tspSonSolution[i].pathArray[0]<<"    pathLen = "<<g_tspSonSolution[i].pathLen<<"    dPReproduction = "<<g_tspSonSolution[i].dPReproduction<<std::endl;
		}
		*/

		// 4. 更新群体
		// 参与对象：父代 + 遗传的子代
		EvoUpdateGroup(pGraph);

		nIter++;
		// _iter = nIter + 1;
	}
}

// 选择
/*
	输入：当前总群
	输出：按照一个评价, 随机从当前总群筛选出杂交对象, 本程序每次返回一个个体
	选择方案：比例选择规则, [轮盘赌选择]
	机制：反映在对父代种群中每一个体所赋予的允许繁殖概率及其从2M个中间个体中如何选择子代种群的机制上！
*/
/*
	[轮盘赌选择] -  轮盘赌选择是从染色体群体中选择一些成员的方法，被选中的机率和它们的适应性分数成比例，染色体的适应性分数愈高，被选中的概率也愈多.
	1. 随机产生一个概率 dSelectP
	2. [概率分布函数]声明变量 dDistributionP = 0, 对于每个个体, 依次累加个体的概率到dDistributionP上, 判断当前随机概率dSelectP是否小于dDistributionP, 若是则中该染色体, 结束循环

*/
int EvoSelect(Graph* pGraph)
{
	// 生成一个随机概率值
	double dSelectP = ((rand() % 100 + 0.0) / 100.0);
	// std::cout<<"dSelectP = "<<dSelectP<<std::endl;

	// 按照概率选择个体
	double dDistributionP = 0.0;
	for (int i = 0; i < GROUP_30NUM; i++)
	{
		dDistributionP += g_tspGroups[i].dPReproduction;
		if (dSelectP < dDistributionP)
		{
			return i;
		}
	}

	// 如果遍历完数组还没有选择个体，可能是概率计算有误
	std::cout << "【ERROR!】EvoSelect() 轮盘赌选择有误..." << std::endl;
	return 0;
}

// 交叉  使用中间杂交方式，在两个个体的交叉点之间互换基因段，处理交叉后的冲突。
/*
	输入：[TSP_Father , TSP_Mother]两个个体作为父母, 进行杂交
	输出：通过杂交产生新个体(遗传算法产生2个新个体, 演化算法产生1个新个体)
	杂交方案：[父子混合选择][自然选择 - 父母不参与竞争]
	-- [演化策略]所使用的杂交算子是从两个个体生成一个个体的操作
	-- [遗传算法]生成两个新个体。常见的“中间杂交”（intermediate crossover）及“随机杂交”（random crossover）等!
*/
/*
	TSP_杂交具体方法：
	1. 随机选取两个交叉点i和j,记为 nFatherCrossArray 和 nMotherCrossArray
	2. 将两交叉点中间的基因段互换
	3. 分别对tspFather和tspMother的路径进行冲突处理：
		-- 以tspFather为例, 保持nFatherCrossArray基因段不变, 基因段以外的部分与nFatherCrossArray基因段冲突的城市,
			用nFatherCrossArray和nMotherCrossArray对应的位置去互换, 直到没有冲突.
		-- 冲突城市的确定: nFatherCrossArray 和 nMotherCrossArray去补集,存放于数组 nConflictArray[] 中.
*/
void EvoCross(Graph* pGraph, TSPSolution TSP_Father, TSPSolution TSP_Mother)
{
	// 杂交过程：随机产生杂交的位置, 保证 g_nCrossI < g_nCrossJ【全局变量】
	g_nCrossI = rand() % (CITY_150NUM - 1) + 1;	// 不能取到起始城市
	g_nCrossJ = rand() % (CITY_150NUM - 1) + 1;	//

	if (g_nCrossI > g_nCrossJ)	// 使 I < J
	{
		int temp = g_nCrossI;
		g_nCrossI = g_nCrossJ;
		g_nCrossJ = temp;
	}

	if (g_nCrossJ == CITY_150NUM || g_nCrossI == 0)
		std::cout << "[ 杂交过程的随机数产生有问题... ]" << std::endl;

	// 杂交基因段 记录从父亲和母亲中被选中的基因段
	int nFatherCrossArray[CITY_150NUM]{};	// 父亲遗传基因段
	int nMotherCrossArray[CITY_150NUM]{};	// 母亲遗传基因段
	int nCrossLength = 0;					// 杂交的个数

	for (int i = g_nCrossI; i <= g_nCrossJ; i++)	// 提取基因段
	{
		nFatherCrossArray[nCrossLength] = TSP_Father.pathArray[i];
		nMotherCrossArray[nCrossLength] = TSP_Mother.pathArray[i];
		nCrossLength++;
	}

	// 开始杂交 - 处理 TSP_Father：找到nFatherCrossArray[]中会产生冲突的城市
	int nLengthConflict = 0;	// 冲突的个数

	// 获取可能产生冲突的城市位置,即 在A 中,但不在 B 中的元素,赋给 nConflictArray
	int* nFatherConflictArray = GetConflict(nFatherCrossArray, nMotherCrossArray, nCrossLength, nLengthConflict);
	int* nMotherConflictArray = GetConflict(nMotherCrossArray, nFatherCrossArray, nCrossLength, nLengthConflict);

	// 交换 tspFather 与 tspMother  I -> J 的基因段
	int nCityTemp = 0;
	for (int i = g_nCrossI; i <= g_nCrossJ; i++)
	{
		nCityTemp = TSP_Father.pathArray[i];
		TSP_Father.pathArray[i] = TSP_Mother.pathArray[i];
		TSP_Mother.pathArray[i] = nCityTemp;
	}

	// 开始杂交 - 处理 TSP_Mother, 其中nLengthConflict会在函数GetConflict()中改变并保存
	TSPSolution descendantA = HandleConflict(pGraph, TSP_Father, nFatherConflictArray, nMotherConflictArray, nLengthConflict);	// 解决 TSP_Father 的冲突
	TSPSolution descendantB = HandleConflict(pGraph, TSP_Mother, nMotherConflictArray, nFatherConflictArray, nLengthConflict);	// 解决 TSP_Mother 的冲突

	//存储子代个体：
	g_tspSonSolution[g_nSonSolutionLen++] = descendantA;
	g_tspSonSolution[g_nSonSolutionLen++] = descendantB;

	delete(nFatherConflictArray);
	delete(nMotherConflictArray);
}

//HandleConflict 函数用于解决基因交叉时可能发生的冲突。
//在遗传算法中，基因交叉可能导致基因中包含重复的城市，而这是不允许的。该函数通过检测冲突并进行合适的调整，确保每个城市在基因中只出现一次。
TSPSolution HandleConflict(Graph* pGraph, TSPSolution conflictSolution, int* nDetectionConflictArray, int* nModelConflictArray, int nLengthConflict)
{
	/*
	std::cout<<"[ HandleConflict ]"<<std::endl<<"nDetectionConflictArray = ";
	for (int i = 0;i < nLengthConflict; i++)
	{
		std::cout<<nDetectionConflictArray[i]<<" ";
	}
	std::cout<<std::endl<<"nModelConflictArray = ";
	for (int i = 0;i < nLengthConflict; i++)
	{
		std::cout<<nModelConflictArray[i]<<" ";
	}
	std::cout<<std::endl;

	std::cout<<"【存在冲突】基因组为：";
	for (int i = 0;i < pGraph->vexNum;i++)
	{
		std::cout<<conflictSolution.pathArray[i]<<" -> ";
	}
	std::cout<<conflictSolution.pathArray[0]<<std::endl;
	*/

	for (int i = 0; i <= nLengthConflict; i++)
	{
		bool bFindCityFlag = false;
		int nIndex = 0;

		// [0, g_nCrossI) 在第一段基因中寻找冲突
		for (nIndex = 0; nIndex < g_nCrossI; nIndex++)
		{
			if (nModelConflictArray[i] == conflictSolution.pathArray[nIndex])
			{
				bFindCityFlag = true;
				break;
			}
		}

		// 第一段没找到, 找剩余的部分【除了交换的基因段外】
		if (!bFindCityFlag)
		{
			// [g_nCrossI + 1, pGraph->vexNum) 寻找冲突
			for (nIndex = g_nCrossJ + 1; nIndex < pGraph->vexNum; nIndex++)
			{
				if (nModelConflictArray[i] == conflictSolution.pathArray[nIndex])
					break;
			}
		}

		// 9 8 [1 4 0 3 2] 3 2 0 --> conflictSolution
		// 8 7 [4 5 6 7 1] 9 6 5
		// [0 3 2] --> nDetectionConflictArray
		// [4 5 6] --> nModelConflictArray
		// 解决冲突, nIndex 为当前i冲突的位置, 用nModelConflictArray去替换.
		// std::cout<<"nIndex = "<<nIndex<<std::endl;
		conflictSolution.pathArray[nIndex] = nDetectionConflictArray[i];
	}

	/*
	std::cout<<std::endl<<"【解决冲突】基因组为：";
	for (int i = 0;i < pGraph->vexNum;i++)
	{
		std::cout<<conflictSolution.pathArray[i]<<" -> ";
	}
	std::cout<<conflictSolution.pathArray[0]<<std::endl;

	// 重新计算新路径的长度
	// CalculateLength(pGraph, conflictSolution);
	*/
	if (!CheckPath(pGraph, conflictSolution))
	{
		std::cout << "【error - 冲突未解决......】" << std::endl;
	}

	// std::cout<<"  pathLen = "<<conflictSolution.pathLen<<"    dPReproduction = "<<conflictSolution.dPReproduction<<std::endl;

	return conflictSolution;
}

// 用于找出在进行基因交叉时可能导致冲突的城市位置
// 找到两个数组中不相同的元素，用于解决遗传算法中基因交叉产生的冲突
// 同时存在于 nFatherCrossArray 和 nMotherCrossArray 为不冲突的城市, 反之是冲突的城市.
// nDetectionCross[]:表示当前搜索的个体, 即找冲突的对象
// nModelCross[]:
// int nConflictArray[CITY_150NUM];
int* GetConflict(int nDetectionCross[], int nModelCross[], int nCrossLength, int& nLengthConflict)
{
	int* nConflictArray = new int[CITY_150NUM];	// 注意内存释放
	nLengthConflict = 0;	// 初始化冲突元素的数量为0

	for (int i = 0; i < nCrossLength; i++)
	{
		bool bConflictFlag = true;	// 判断是否属于冲突
		for (int j = 0; j < nCrossLength; j++)
		{
			if (nDetectionCross[i] == nModelCross[j])	// 检查两个城市是否相同,相同则不加
			{
				bConflictFlag = false;	// 该城市不属于冲突
				j = nCrossLength;		// 结束第二层循环
			}
		}

		if (bConflictFlag)
		{
			nConflictArray[nLengthConflict] = nDetectionCross[i];	// 将不同的城市添加到冲突数组中
			nLengthConflict++;	// 冲突元素计数数量加1
		}
	}

	/*
	std::cout<<"nConflictArray[] = ";
	for (int i = 0; i < nLengthConflict; i++)
	{
		std::cout<<nConflictArray[i]<<"  ";
	}
	std::cout<<std::endl;
	*/
	return nConflictArray;	// 返回存储冲突元素的数组
}

// 变异 Variation
/*
	输入：杂交得到的所有个体（大于总群规模）
	输出：通过变异策略, 以一定的变异概率（确定变异个数）随机选择个体进行变异
	变异策略：随机交换染色体的片段, TSP - 随机交换两个城市的位置

	变异过程：随机产生一个变异的位置
	随机产生两个随机数表示两个城市的位置, 并进行位置交换
	变异是为了增加种群的多样性，防止陷入局部最优解。
*/
void EvoVariation(Graph* pGraph, int nVariation)
{
	int nCityI = (rand() % (CITY_150NUM - 1)) + 1;	// [1, CITY_150NUM - 1]起始城市不变异
	int nCityJ = (rand() % (CITY_150NUM - 1)) + 1;	//

	while (nCityI == nCityJ)
		nCityJ = (rand() % (CITY_150NUM - 1)) + 1;

	/*
	std::cout<<"-----------------"<<std::endl;
	std::cout<<"变异位置如下:"<<std::endl;
	std::cout<<"nCityI = "<<nCityI<<std::endl;
	std::cout<<"nCityJ = "<<nCityJ<<std::endl;
	*/

	// 交换城市位置 - 变异
	int nTempCity = g_tspSonSolution[nVariation].pathArray[nCityI];
	g_tspSonSolution[nVariation].pathArray[nCityI] = g_tspSonSolution[nVariation].pathArray[nCityJ];
	g_tspSonSolution[nVariation].pathArray[nCityJ] = nTempCity;
}

// 根据新生成的子代替换群体中路径长度较长的个体。
// 父代 - g_tspGroups[]
// 子代 - g_tspSonSolution[]
void EvoUpdateGroup(Graph* pGraph)
{
	TSPSolution tempSolution;
	// 先对子代 - g_tspSonSolution[] 依据路径长度进行排序 - 降序[按路径从大到小]
	for (int i = 0; i < g_nSonSolutionLen; i++)
	{
		for (int j = g_nSonSolutionLen - 1; j > i; j--)
		{
			if (g_tspSonSolution[i].pathLen > g_tspSonSolution[j].pathLen)
			{
				tempSolution = g_tspSonSolution[i];
				g_tspSonSolution[i] = g_tspSonSolution[j];
				g_tspSonSolution[j] = tempSolution;
			}
		}
	}

	/*
	std::cout<<"【冒泡排序后...】"<<std::endl;
	for (int i = 0; i < g_nSonSolutionLen; i++)
	{
		std::cout<<"pathLen = "<<g_tspSonSolution[i].pathLen<<std::endl;
	}
	 */

	 // 更新
	for (int i = 0; i < g_nSonSolutionLen; i++)	// 子代 - 按路径从大到小排序
	{
		for (int j = 0; j < GROUP_30NUM; j++)	// 父代
		{
			if (g_tspSonSolution[i].pathLen < g_tspGroups[j].pathLen)
			{
				g_tspGroups[j] = g_tspSonSolution[i];	// 种群更新
				break;
			}
		}
	}

	TspEvaluate(pGraph);
}

double CalculateLength(Graph* pGraph, TSPSolution newSolution)
{
	double dLength = 0;

	for (int i = 0; i < pGraph->vexNum - 1; i++)
	{
		int startCity = newSolution.pathArray[i] - 1;	// 路径下标是从 1 开始存储
		int endCity = newSolution.pathArray[i + 1] - 1;
		if (pGraph->arcs[startCity][endCity] == -1)
		{
			return MAX_INT;
		}
		else
		{
			dLength += pGraph->arcs[startCity][endCity];
		}
	}

	// 判断该路径是否能回到起始城市
	auto a = newSolution.pathArray[pGraph->vexNum - 1];
	auto b = newSolution.pathArray[0] - 1;
	if (pGraph->arcs[a][b] == -1)
	{
		return MAX_INT;
	}
	else
	{
		dLength += pGraph->arcs[a - 1][b];
		// std::cout<<"dLength = "<<dLength<<std::endl;
		return dLength;
	}
}

bool CheckPath(Graph* pGraph, TSPSolution& curSolution)
{
	for (int i = 0; i < pGraph->vexNum; i++)
	{
		for (int j = i + 1; j < pGraph->vexNum; j++)
		{
			if (curSolution.pathArray[i] == curSolution.pathArray[j])
			{
				return false;
			}
		}
	}
	return true;
}

// TSP - 评价函数 遗传算法的演化过程函数: 包含选择、交叉、变异、更新群体等操作的循环过程
// 输入：当前总群 g_tspGroups[] - 包括 每个个体的路径和所需的长度
// 输出：当前总群中, 最优的个体：bestSolution
// 评价方法：路径最短的为最优
void TspEvaluate(Graph* pGraph)
{
	TSPSolution& bestSolution = g_tspGroups[0];
	for (int i = 1; i < GROUP_30NUM; i++)
	{
		if (bestSolution.pathLen > g_tspGroups[i].pathLen)
		{
			bestSolution = g_tspGroups[i];
		}
	}

	// std::cout<<"---------------- 【当前总群】 ----------------"<<std::endl;
	//for (int i = 0; i < GROUP_30NUM; i++)
	//{
	//	std::cout << "\n【第 " << i + 1 << " 号染色体】：";
	//	for (int j = 0; j < pGraph->vexNum; j++)
	//	{
	//		std::cout << g_tspGroups[i].pathArray[j] << " -> ";
	//	}
	//	std::cout << g_tspGroups[i].pathArray[0] << "    ";
	//	std::cout << "length = " << g_tspGroups[i].pathLen << "\t\tdPReproduction = "
	//		<< g_tspGroups[i].dPReproduction << std::endl;
	//}

	// 打开文件流
	std::ofstream outputFile("D:/result.txt");

	static int i = 0;
	if (outputFile.is_open())
	{
		std::cout << "\n" << i++ << " =================================\n"
			<< ":当前最优个体 bestSolution = \n";
		for (int i = 0; i < pGraph->vexNum; i++)
		{
			std::cout << bestSolution.pathArray[i] << " -> ";

			outputFile << bestSolution.pathArray[i];

			// 如果不是最后一个元素，则添加逗号
			if (i < pGraph->vexNum - 1)
			{
				outputFile << ",";
			}
		}
	}

	std::cout << bestSolution.pathArray[0] << "\nlength = " << bestSolution.pathLen << std::endl;
}

/*
void Display(Graph pGraph){
	std::cout<<"---------------- 【遗传算法 - 当前总群】 ----------------"<<std::endl;
	for (int i = 0;i < GROUP_30NUM;i++)
	{
		std::cout<<"【第 "<<i + 1<<" 号染色体】：";
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			std::cout<<g_tspGroups[i].pathArray[j]<<" -> ";
		}
		std::cout<<g_tspGroups[i].pathArray[0]<<"    ";
		std::cout<<"length = "<<g_tspGroups[i].pathLen<<"\t\tdPReproduction = "
		<<g_tspGroups[i].dPReproduction<<std::endl;
	}
}
*/