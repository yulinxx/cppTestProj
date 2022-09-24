// https://blog.csdn.net/luoti784600/article/details/12646405

// TcpDataSplit.cpp : 定义控制台应用程序的入口点。
//
#define  _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NETPACK_SIZE	10000
#define MAX_DATA_SIZE			4086

/* 数据包头类型 */
struct NetDataHeader_t
{
	int nDataType;											//数据包类型，标识对应的对象类型
	int nDataSize;											//数据包中szData真实数据的长度
};

/*  数据包类型 */
struct NetDataBase_t
{
	NetDataHeader_t  dataHeader;			//数据包头
	char	 szData[MAX_DATA_SIZE];				//真实数据
};

/**
	其实NetDataBase_t是基础类型，由此我们可以延伸出很多子类型，
	所以我们要清楚，每个类型的长度是不一样的，不都是sizeof(NetDataBase_t)，
	就是各个类型对象大小不一样，比如：
	在派生结构体中，NetDataPeople_t和NetDataSchool_t是两个各异的结构体，
	但他们都有相关的Header部分指明结构体类型和长度。
*/
struct NetDataPeople_t
{
	NetDataHeader_t dataHeader;
	int		nAge;
	char    chName[10];
};

struct NetDataSchool_t
{
	NetDataHeader_t dataHeader;
	char    chShoolName[20];
	char    chShoolAddress[30];
};

/**
	处理整理好的对象。
*/
bool HandleNetPack(NetDataHeader_t* pDataHeader);


bool TcpDataSplit(const char* chRecNetData, int nRecSize)
{
	/**
		对于chRecNetData, nRemainSize,为了简单，本例子只
		作为静态变量使用，因此只限于一个socket的数据接收，
		假如要同时处理多个socket数据，请放在对应容器里保存
	*/
	static char chLastSaveData[MAX_NETPACK_SIZE];
	static int nRemainSize = 0;
	static bool bFirst = true;

	if (bFirst)
	{
		memset(chLastSaveData, 0, sizeof(chLastSaveData));
		bFirst = false;
	}

	/* 本次接收到的数据拼接到上次数据 */
	memcpy((char*)(chLastSaveData + nRemainSize), chRecNetData, nRecSize);
	nRemainSize = nRecSize + nRemainSize;

	/* 强制转换成NetDataPack指针 */
	NetDataHeader_t* pDataHead = (NetDataHeader_t*)chLastSaveData;

	/**
	   核心算法
	*/
	while (nRemainSize > sizeof(NetDataHeader_t) &&
		nRemainSize >= pDataHead->nDataSize + sizeof(NetDataHeader_t))
	{
		HandleNetPack(pDataHead);
		int  nRecObjectSize = sizeof(NetDataHeader_t) + pDataHead->nDataSize;		//本次收到对象的大小
		nRemainSize -= nRecObjectSize;
		pDataHead = (NetDataHeader_t*)((char*)pDataHead + nRecObjectSize);		//移动下一个对象头
	}

	/* 余下数据未能组成一个对象，先保存起来 */
	if (chLastSaveData != (char*)pDataHead)
	{
		memmove(chLastSaveData, (char*)pDataHead, nRemainSize);
		memset((char*)(chLastSaveData + nRemainSize), 0, sizeof(chLastSaveData) - nRemainSize);
	}

	return true;
}


/**
	处理整理好的对象。
*/
bool HandleNetPack(NetDataHeader_t* pDataHeader)
{
	//处理数据包
	if (pDataHeader->nDataType == 1)
	{
		NetDataPeople_t* pPeople = (NetDataPeople_t*)pDataHeader;
		printf("收到People对象，Age:%d, Name:%s\n", pPeople->nAge, pPeople->chName);
	}
	else if (pDataHeader->nDataType == 2)
	{
		NetDataSchool_t* pSchool = (NetDataSchool_t*)pDataHeader;
		printf("收到School对象，SchoolName:%s, SchoolAddress:%s\n", pSchool->chShoolName, pSchool->chShoolAddress);
	}

	return true;
}

int main()
{
	/* 本例子以两个对象作为接收到的数据 */
	NetDataPeople_t  people;
	people.dataHeader.nDataSize = sizeof(people) - sizeof(NetDataHeader_t);
	people.dataHeader.nDataType = 1;
	people.nAge = 20;
	sprintf(people.chName, "Jim");		//real data

	NetDataSchool_t  school;
	school.dataHeader.nDataSize = sizeof(school) - sizeof(NetDataHeader_t);
	school.dataHeader.nDataType = 2;
	sprintf(school.chShoolName, "清华大学");		//real data
	sprintf(school.chShoolAddress, "北京市北京路");		//real data

	/* 将两个对象数据合并到一个地址里面以便重现粘包 */
	char chSendData[sizeof(people) + sizeof(school)];
	memcpy(chSendData, (char*)&people, sizeof(people));
	memcpy(chSendData + sizeof(people), (char*)&school, sizeof(school));

	//这里进行收数据操作，这里省略。。。

	/**
		特意设置粘包：
		1.第一次只发送3个字节，还不足以构建包头
		2.第二次发送10个字节，总共13个，但第一个对象大小是8+14=18；因此第一个对象people还没收满
		3.第三次发送剩下的全部，第一个对象剩下的部分与第二个对象粘在一起，验证处理
	*/
	TcpDataSplit((char*)chSendData, 3);
	TcpDataSplit((char*)chSendData + 3, 10);
	TcpDataSplit((char*)chSendData + 13, sizeof(chSendData) - 13);

	getchar();
	return 0;
}