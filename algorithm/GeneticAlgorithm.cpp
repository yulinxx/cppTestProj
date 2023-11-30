#include <algorithm>
#include <ctime>
#include <fstream>
#include <iomanip>	// 本文用于输出对齐
#include <iostream>
#include <stdlib.h>

#include <fstream>

#include "GeneticAlgorithm.h"

using namespace std;

//用于记录交叉过程中的索引
int indexCrossI;
int indexCrossJ;

// int _iter = 0;

int main()
{
	time_t T_begin = clock();
	Graph* pGraph = new Graph();	// 创建一个图对象

	int temp = sizeof(pGraph);

	CreateGraph(pGraph);			// 从文件中读取图数据并初始化图

	int temp2 = sizeof(pGraph);
	int temp3 = sizeof(*pGraph);

	srand(unsigned(time(0)));
	InitialGroup(pGraph);			// 使用遗传算法初始化初始解集

	// 运行 旅行推销员问题（TSP）的遗传算法进化过程
	TspEvolution(pGraph);	// 遗传算法

	time_t T_end = clock();
	double RunningTime = double(T_end - T_begin) / CLOCKS_PER_SEC;
	cout << endl << "【 程序运行时间 RunningTime = " << RunningTime << " 】" << endl;

	system("pause");
	return 0;
}

//创建图的函数
//这个函数从文件中读取图的信息，包括顶点数、顶点信息、邻接矩阵等，并将其存储在 Graph 结构体对象中。
void CreateGraph(Graph* pGraph)
{
	ifstream read_in;
	read_in.open("GeneticAlgorithmCityData150.txt");
	if (!read_in.is_open())
	{
		cout << "从文件中读取图数据失败." << endl;
		return;
	}

	read_in >> pGraph->vexNum;
	// read_in >> pGraph->arcNum;
	pGraph->arcNum = 0;

	// 读取顶点信息和邻接矩阵
	for (int i = 0; i < pGraph->vexNum; i++)
	{
		read_in >> pGraph->vexs[i];
	}
	pGraph->vexs[pGraph->vexNum] = '\0';	// char的结束符.

	// 计算边数
	for (int i = 0; i < pGraph->vexNum; i++)
	{
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			read_in >> pGraph->arcs[i][j];

			// calculate the arcNum
			if (pGraph->arcs[i][j] > 0)
			{
				pGraph->arcNum++;
			}
		}
	}

	// 输出图的信息
	cout << "无向图创建完毕，相关信息如下：" << endl;
	cout << "【顶点数】 pGraph->vexNum = " << pGraph->vexNum << endl;
	cout << "【边数】 pGraph->arcNum = " << pGraph->arcNum << endl;
	cout << "【顶点向量】 vexs[max_vexNum] = ";

	for (int i = 0; i < pGraph->vexNum; i++)
	{
		cout << pGraph->vexs[i] << " ";
	}

	cout << endl << "【邻接矩阵】 arcs[max_vexNum][max_vexNum] 如下：" << endl;
/*
	for (int i = 0; i < pGraph->vexNum;i++)
	{
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			cout << pGraph->arcs[i][j]<<" ";
		}
		cout<<endl;
	}
*/
}

// 初始化遗传算法的个体群体
// 函数随机生成群体中每个个体的初始路径，然后检查路径的合法性，并计算路径长度。
// 接着，计算群体的概率，并评估每个个体。

void InitialGroup(Graph* pGraph)
{
	//cout << "----------------------【遗传算法参数】-----------------------" << endl;
	//cout << "【城市个数】 CITY_NUM =" << CITY_NUM << endl;
	//cout << "【群体规模】 GROUP_NUM = " << GROUP_NUM << endl;
	//cout << "【子代规模】 SON_NUM = " << SON_NUM << endl;
	//cout << "【变异概率】 P_INHERIATANCE = " << P_INHERIATANCE << endl;
	//cout << "【杂交概率】 P_COPULATION = " << P_COPULATION << endl;
	//cout << "【迭代次数】 ITERATION_NUM = " << ITERATION_NUM << endl;

	// 对每个个体随机生成路径
	double dTotalLength = 0.0;
	for (int i = 0; i < GROUP_NUM; i++)
	{
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			gTspGroups[i].pathArray[j] = pGraph->vexs[j];
		}

		// 随机打乱路径
		random_shuffle(gTspGroups[i].pathArray + 1, gTspGroups[i].pathArray + pGraph->vexNum);

		// 检查路径是否合法，计算路径长度
		if (CheckPath(pGraph, gTspGroups[i]))
		{
			gTspGroups[i].pathLen = CalculateLength(pGraph, gTspGroups[i]);
			dTotalLength += gTspGroups[i].pathLen;
		}
		else
		{
			cout << "【error！城市路径产生重复城市！】" << endl;
			gTspGroups[i].pathLen = MAX_INT;
			gTspGroups[i].P_Reproduction = 0;
		}
	}

	// 计算概率并评估个体
	CalcProbablity(pGraph, dTotalLength);
	TspEvaluate(pGraph);
}

// 处理对象：每次新产生的群体, 计算每个个体的概率
// 问题：解决TSP问题, 路径越短概率应该越高
// 方案：对于当前总群, 将所有个体路径取倒数, 然后乘以该总群的总路径得到大于1的值, 然后进行归一化, 取得概率
// 归一化：累加当前所有大于1的个体的伪概率, 得到TempTotal_P, 每个概率再分别除以 TempTotal_P 进行归一化
void CalcProbablity(Graph* pGraph, double dTotalLength)
{
	double TempTotal_P = 0.0;

	for (int i = 0; i < GROUP_NUM; i++)
	{
		gTspGroups[i].P_Reproduction = (1.0 / gTspGroups[i].pathLen) * dTotalLength;
		TempTotal_P += gTspGroups[i].P_Reproduction;
	}

	for (int i = 0; i < GROUP_NUM; i++)
	{
		gTspGroups[i].P_Reproduction = gTspGroups[i].P_Reproduction / TempTotal_P;
	}
}

// 遗传算法的演化过程
// 这段代码是遗传算法的演化过程的核心。
// 在每一次迭代中，首先通过选择操作选择父代个体，然后进行交叉操作生成子代个体，接着进行变异操作。
// 最后，评估每个个体的适应度，更新最优解，并进入下一轮迭代。
void TspEvolution(Graph* pGraph)
{
	/* */
	int iter = 0;
	while (iter < ITERATION_NUM)
	{
		// cout<<"***********************【第次"<<(iter + 1)<<"迭代】*************************"<<endl;

		// 1. 选择
		int Father_index = EvoSelect(pGraph);
		int Mother_index = EvoSelect(pGraph);

		while (Mother_index == Father_index)
		{
			// 防止Father和Mother都是同一个个体 -> 自交( 父母为同一个个体时, 母亲重新选择, 直到父母为不同的个体为止 )
			// cout<<"Warning!【Father_index = Mother_index】"<<endl;
			Mother_index = EvoSelect(pGraph);
		}

		// gTspGroups[]为当前总群
		TSP_solution Father = gTspGroups[Father_index];
		TSP_solution Mother = gTspGroups[Mother_index];

		// 2. 交叉, 存储在全局变脸 gSonSolution[] 数组 - 通过M次杂交, 产生2M个新个体, 2M >= GROUP_NUM
		int M = GROUP_NUM - GROUP_NUM / 2;
		gSonSolitonLen = 0;	// 遗传产生的个体个数, 置零重新累加
		while (M)
		{
			double Is_COPULATION = ((rand() % 100 + 0.0) / 100);
			if (Is_COPULATION > P_COPULATION)
			{
				// cout<<"[ 这两个染色体不进行杂交 ]Is_COPULATION = "<<Is_COPULATION<<endl;
			}
			else
			{
				// 杂交, 将结果存储于遗传个体总群,全局变量gSonSolution[]
				EvoCross(pGraph, Father, Mother);
				M--;
			}
		}

		// 3. 变异：针对 gSonSolution[]
		double dTotalLength = 0.0;	// 更新新个体的概率

		for (int indexVariation = 0; indexVariation < gSonSolitonLen; indexVariation++)
		{
			double RateVariation = (rand() % 100) / 100.0;
			// 产生的随机数小于变异概率 则该个体进行变异
			if (RateVariation < P_INHERIATANCE)
			{
				EvoVariation(pGraph, indexVariation);
			}

			// 经过变异处理后 重新计算路径值
			if (!CheckPath(pGraph, gSonSolution[indexVariation]))
			{
				cout << "【Error! 路径有重复!】" << endl;
			}

			// 产生新个体, 计算新路径和新概率
			gSonSolution[indexVariation].pathLen = CalculateLength(pGraph, gSonSolution[indexVariation]);
			dTotalLength += gSonSolution[indexVariation].pathLen;
		}

		CalcProbablity(pGraph, dTotalLength);

		/*
		cout<<"【遗传产生的子代个体如下...】"<<endl;
		for (int i = 0; i < gSonSolitonLen; i++)
		{
			for (int j = 0;j < pGraph->vexNum;j++)
			{
				cout<<gSonSolution[i].pathArray[j]<<" -> ";
			}
			cout<<gSonSolution[i].pathArray[0]<<"    pathLen = "<<gSonSolution[i].pathLen<<"    P_Reproduction = "<<gSonSolution[i].P_Reproduction<<endl;
		}
		*/

		// 4. 更新群体
		// 参与对象：父代 + 遗传的子代
		EvoUpdateGroup(pGraph);

		iter++;
		// _iter = iter + 1;
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
	1. 随机产生一个概率 selection_P
	2. [概率分布函数]声明变量 distribution_P = 0, 对于每个个体, 依次累加个体的概率到distribution_P上, 判断当前随机概率selection_P是否小于distribution_P, 若是则中该染色体, 结束循环

*/
int EvoSelect(Graph* pGraph)
{
	double selection_P = ((rand() % 100 + 0.0) / 100);
	// cout<<"selection_P = "<<selection_P<<endl;

	double distribution_P = 0.0;
	for (int i = 0; i < GROUP_NUM; i++)
	{
		distribution_P += gTspGroups[i].P_Reproduction;
		if (selection_P < distribution_P)
		{
			return i;
		}
	}
	cout << "【ERROR!】EvoSelect() 轮盘赌选择有误..." << endl;
	return 0;
}

// 交叉
/*
	输入：[TSP_Father , TSP_Mother]两个个体作为父母, 进行杂交
	输出：通过杂交产生新个体(遗传算法产生2个新个体, 演化算法产生1个新个体)
	杂交方案：[父子混合选择][自然选择 - 父母不参与竞争]
	-- [演化策略]所使用的杂交算子是从两个个体生成一个个体的操作
	-- [遗传算法]生成两个新个体。常见的“中间杂交”（intermediate crossover）及“随机杂交”（random crossover）等!
*/
/*
	TSP_杂交具体方法：
	1. 随机选取两个交叉点i和j,记为 Father_Cross 和 Mother_Cross
	2. 将两交叉点中间的基因段互换
	3. 分别对Father和Mother的路径进行冲突处理：
		-- 以Father为例, 保持Father_Cross基因段不变, 基因段以外的部分与Father_Cross基因段冲突的城市, 用Father_Cross和Mother_Cross对应的位置去互换, 直到没有冲突.
		-- 冲突城市的确定: Father_Cross 和 Mother_Cross去补集,存放于数组 Conflict[] 中.
*/
void EvoCross(Graph* pGraph, TSP_solution TSP_Father, TSP_solution TSP_Mother)
{
	// 杂交过程：随机产生杂交的位置, 保证 indexCrossI < indexCrossJ【全局变量】
	indexCrossI = rand() % (CITY_NUM - 1) + 1;	// 不能取到起始城市
	indexCrossJ = rand() % (CITY_NUM - 1) + 1;	//

	if (indexCrossI > indexCrossJ)
	{
		int temp = indexCrossI;
		indexCrossI = indexCrossJ;
		indexCrossJ = temp;
	}

	if (indexCrossJ == CITY_NUM || indexCrossI == 0)
	{
		cout << "[ 杂交过程的随机数产生有问题... ]" << endl;
	}

	// 杂交基因段
	int Father_Cross[CITY_NUM];	// 父亲遗传基因段
	int Mother_Cross[CITY_NUM];	// 母亲遗传基因段
	int Length_Cross = 0;		// 杂交的个数
	for (int i = indexCrossI; i <= indexCrossJ; i++)
	{
		Father_Cross[Length_Cross] = TSP_Father.pathArray[i];
		Mother_Cross[Length_Cross] = TSP_Mother.pathArray[i];
		Length_Cross++;
	}

	// 开始杂交 - 处理 TSP_Father：找到Father_Cross[]中会产生冲突的城市
	int* Conflict_Father;		// 存储冲突的位置
	int* Conflict_Mother;
	int Length_Conflict = 0;	// 冲突的个数
	Conflict_Father = GetConflict(Father_Cross, Mother_Cross, Length_Cross, Length_Conflict);
	Conflict_Mother = GetConflict(Mother_Cross, Father_Cross, Length_Cross, Length_Conflict);

	// Father and Mother 交换基因段
	int city_temp;
	for (int i = indexCrossI; i <= indexCrossJ; i++)
	{
		city_temp = TSP_Father.pathArray[i];
		TSP_Father.pathArray[i] = TSP_Mother.pathArray[i];
		TSP_Mother.pathArray[i] = city_temp;
	}

	// 开始杂交 - 处理 TSP_Mother, 其中Length_Conflict会在函数GetConflict()中改变并保存
	TSP_solution Descendant_ONE = HandleConflict(pGraph, TSP_Father, Conflict_Father, Conflict_Mother, Length_Conflict);	// 解决 TSP_Father 的冲突
	TSP_solution Descendant_TWO = HandleConflict(pGraph, TSP_Mother, Conflict_Mother, Conflict_Father, Length_Conflict);	// 解决 TSP_Mother 的冲突

	gSonSolution[gSonSolitonLen++] = Descendant_ONE;
	gSonSolution[gSonSolitonLen++] = Descendant_TWO;
}

TSP_solution HandleConflict(Graph* pGraph, TSP_solution ConflictSolution, int* Detection_Conflict, int* Model_Conflict, int Length_Conflict)
{
	/*
	cout<<"[ HandleConflict ]"<<endl<<"Detection_Conflict = ";
	for (int i = 0;i < Length_Conflict; i++)
	{
		cout<<Detection_Conflict[i]<<" ";
	}
	cout<<endl<<"Model_Conflict = ";
	for (int i = 0;i < Length_Conflict; i++)
	{
		cout<<Model_Conflict[i]<<" ";
	}
	cout<<endl;

	cout<<"【存在冲突】基因组为：";
	for (int i = 0;i < pGraph->vexNum;i++)
	{
		cout<<ConflictSolution.pathArray[i]<<" -> ";
	}
	cout<<ConflictSolution.pathArray[0]<<endl;
	*/
	for (int i = 0; i <= Length_Conflict; i++)
	{
		bool flag_FindCity = false;
		int index = 0;
		// [0, indexCrossI) 寻找冲突
		for (index = 0; index < indexCrossI; index++)
		{
			if (Model_Conflict[i] == ConflictSolution.pathArray[index])
			{
				flag_FindCity = true;
				break;
			}
		}

		// 第一段没找到, 找剩余的部分【除了交换的基因段外】
		if (!flag_FindCity)
		{
			// [indexCrossI + 1, pGraph->vexNum) 寻找冲突
			for (index = indexCrossJ + 1; index < pGraph->vexNum; index++)
			{
				if (Model_Conflict[i] == ConflictSolution.pathArray[index])
				{
					break;
				}
			}
		}

		// 9 8 [1 4 0 3 2] 3 2 0 --> ConflictSolution
		// 8 7 [4 5 6 7 1] 9 6 5
		// [0 3 2] --> Detection_Conflict
		// [4 5 6] --> Model_Conflict
		// 解决冲突, index 为当前i冲突的位置, 用Model_Conflict去替换.
		// cout<<"index = "<<index<<endl;
		ConflictSolution.pathArray[index] = Detection_Conflict[i];
	}

	/*
	cout<<endl<<"【解决冲突】基因组为：";
	for (int i = 0;i < pGraph->vexNum;i++)
	{
		cout<<ConflictSolution.pathArray[i]<<" -> ";
	}
	cout<<ConflictSolution.pathArray[0]<<endl;

	// 重新计算新路径的长度
	// CalculateLength(pGraph, ConflictSolution);
	*/
	if (!CheckPath(pGraph, ConflictSolution))
	{
		cout << "【error - 冲突未解决......】" << endl;
	}
	// cout<<"  pathLen = "<<ConflictSolution.pathLen<<"    P_Reproduction = "<<ConflictSolution.P_Reproduction<<endl;

	return ConflictSolution;
}

int* GetConflict(int Detection_Cross[], int Model_Cross[], int Length_Cross, int& Length_Conflict)
{
	// 同时存在于 Father_Cross 和 Mother_Cross 为不冲突的城市, 反之是冲突的城市.
	// Detection_Cross[]:表示当前搜索的个体, 即找冲突的对象
	// Model_Cross[]:
	// int Conflict[CITY_NUM];
	int* Conflict = new int[CITY_NUM];
	Length_Conflict = 0;
	for (int i = 0; i < Length_Cross; i++)
	{
		bool flag_Conflict = true;	// 判断是否属于冲突
		for (int j = 0; j < Length_Cross; j++)
		{
			if (Detection_Cross[i] == Model_Cross[j])
			{
				// 结束第二层循环
				j = Length_Cross;
				flag_Conflict = false;	// 该城市不属于冲突
			}
		}
		if (flag_Conflict)
		{
			Conflict[Length_Conflict] = Detection_Cross[i];
			Length_Conflict++;
		}
	}

	/*
	cout<<"Conflict[] = ";
	for (int i = 0; i < Length_Conflict; i++)
	{
		cout<<Conflict[i]<<"  ";
	}
	cout<<endl;
	*/
	return Conflict;
}

// 变异
/*
	输入：杂交得到的所有个体（大于总群规模）
	输出：通过变异策略, 以一定的变异概率（确定变异个数）随机选择个体进行变异
	变异策略：随机交换染色体的片段, TSP - 随机交换两个城市的位置
*/
void EvoVariation(Graph* pGraph, int Index_Variation)
{
	// 随机产生两个随机数表示两个城市的位置, 并进行位置交换
	int City_i = (rand() % (CITY_NUM - 1)) + 1;	// [1, CITY_NUM - 1]起始城市不变异
	int City_j = (rand() % (CITY_NUM - 1)) + 1;	//

	while (City_i == City_j)
	{
		City_j = (rand() % (CITY_NUM - 1)) + 1;
	}

	/*
	cout<<"-----------------"<<endl;
	cout<<"变异位置如下:"<<endl;
	cout<<"City_i = "<<City_i<<endl;
	cout<<"City_j = "<<City_j<<endl;
	*/

	// 交换城市位置 - 变异
	int temp_City = gSonSolution[Index_Variation].pathArray[City_i];
	gSonSolution[Index_Variation].pathArray[City_i] = gSonSolution[Index_Variation].pathArray[City_j];
	gSonSolution[Index_Variation].pathArray[City_j] = temp_City;
}

// 父代 - gTspGroups[]
// 子代 - gSonSolution[]
void EvoUpdateGroup(Graph* pGraph)
{
	TSP_solution tempSolution;
	// 先对子代 - gSonSolution[] 依据路径长度进行排序 - 降序[按路径从大到小]
	for (int i = 0; i < gSonSolitonLen; i++)
	{
		for (int j = gSonSolitonLen - 1; j > i; j--)
		{
			if (gSonSolution[i].pathLen > gSonSolution[j].pathLen)
			{
				tempSolution = gSonSolution[i];
				gSonSolution[i] = gSonSolution[j];
				gSonSolution[j] = tempSolution;
			}
		}
	}

	/*
	cout<<"【冒泡排序后...】"<<endl;
	for (int i = 0; i < gSonSolitonLen; i++)
	{
		cout<<"pathLen = "<<gSonSolution[i].pathLen<<endl;
	}
	 */

	 // 更新
	for (int i = 0; i < gSonSolitonLen; i++)	// 子代 - 按路径从大到小排序
	{
		for (int j = 0; j < GROUP_NUM; j++)	// 父代
		{
			if (gSonSolution[i].pathLen < gTspGroups[j].pathLen)
			{
				gTspGroups[j] = gSonSolution[i];	// 种群更新
				break;
			}
		}
	}

	TspEvaluate(pGraph);
}

double CalculateLength(Graph* pGraph, TSP_solution newSolution)
{
	double _length = 0;

	for (int i = 0; i < pGraph->vexNum - 1; i++)
	{
		int _startCity = newSolution.pathArray[i] - 1;	// 路径下标是从 1 开始存储
		int _endCity = newSolution.pathArray[i + 1] - 1;
		if (pGraph->arcs[_startCity][_endCity] == -1)
		{
			return MAX_INT;
		}
		else
		{
			_length += pGraph->arcs[_startCity][_endCity];
		}
	}

	// 判断该路径是否能回到起始城市
	if (pGraph->arcs[newSolution.pathArray[pGraph->vexNum - 1]][newSolution.pathArray[0] - 1] == -1)
	{
		return MAX_INT;
	}
	else
	{
		_length += pGraph->arcs[newSolution.pathArray[pGraph->vexNum - 1] - 1][newSolution.pathArray[0] - 1];
		// cout<<"_length = "<<_length<<endl;
		return _length;
	}
}

bool CheckPath(Graph* pGraph, TSP_solution CurrentSolution)
{
	for (int i = 0; i < pGraph->vexNum; i++)
	{
		for (int j = i + 1; j < pGraph->vexNum; j++)
		{
			if (CurrentSolution.pathArray[i] == CurrentSolution.pathArray[j])
			{
				return false;
			}
		}
	}
	return true;
}

/*
	// TSP - 评价函数
	// 输入：当前总群 gTspGroups[] - 包括 每个个体的路径和所需的长度
	// 输出：当前总群中, 最优的个体：bestSolution
	// 评价方法：路径最短的为最优
*/
void TspEvaluate(Graph* pGraph)
{
	TSP_solution bsetSolution;
	bsetSolution = gTspGroups[0];
	for (int i = 1; i < GROUP_NUM; i++)
	{
		if (bsetSolution.pathLen > gTspGroups[i].pathLen)
		{
			bsetSolution = gTspGroups[i];
		}
	}

	// cout<<"----------------------- 【当前总群】 --------------------"<<endl;
	/*
	for (int i = 0;i < GROUP_NUM; i++)
	{
		cout<<"【第 "<<i + 1<<" 号染色体】：";
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			cout<<gTspGroups[i].pathArray[j]<<" -> ";
		}
		cout<<gTspGroups[i].pathArray[0]<<"    ";
		cout<<"length = "<<gTspGroups[i].pathLen<<"    P_Reproduction = "<<gTspGroups[i].P_Reproduction<<endl;
	}
	*/
	// 打开文件流
	ofstream outputFile("D:/result.txt");

	static int i = 0;
	if (outputFile.is_open())
	{
		cout <<"\n"<< i++ << " :当前最优个体 bsetSolution =\n";
		for (int i = 0; i < pGraph->vexNum; i++)
		{
			cout << bsetSolution.pathArray[i] << " -> ";

			outputFile << bsetSolution.pathArray[i];

			// 如果不是最后一个元素，则添加逗号
			if (i < pGraph->vexNum - 1)
			{
				outputFile << ",";
			}
		}
	}

	cout << bsetSolution.pathArray[0] << "\nlength = " << bsetSolution.pathLen << endl;
}

/*
void Display(Graph pGraph){
	cout<<"----------------------- 【遗传算法 - 当前总群】 --------------------"<<endl;
	for (int i = 0;i < GROUP_NUM;i++)
	{
		cout<<"【第 "<<i + 1<<" 号染色体】：";
		for (int j = 0; j < pGraph->vexNum; j++)
		{
			cout<<gTspGroups[i].pathArray[j]<<" -> ";
		}
		cout<<gTspGroups[i].pathArray[0]<<"    ";
		cout<<"length = "<<gTspGroups[i].pathLen<<"    P_Reproduction = "<<gTspGroups[i].P_Reproduction<<endl;
	}
}
*/