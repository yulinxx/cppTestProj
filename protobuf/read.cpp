#include "message.pb.h"
#include "baseInfo.pb.h"
#include "all.pb.h"

#include <fstream>
#include <iostream>
#include <vector>

using namespace std;
using namespace MsgPkg;
using namespace basePkg;

#include "baseinfoObj.h"
#include "messageObj.h"

void traceMsg(const Info& msg)
{
    cout << "\n----Info\n";
    cout << msg.ver() << endl;
    cout << msg.author() << endl;
    cout << msg.create_time() << endl;
    cout << msg.encrypt() << endl;
    cout << msg.mask() << endl;
}

void traceMsg(const MessageTest& msg)
{
    cout << "\n----Message\n";
    cout << msg.datetime() << endl;
    cout << msg.hostname() << endl;
    cout << msg.ip() << endl;
    cout << msg.info() << endl;
}

void testAllItem()
{
    fstream input("allMsg.db", ios::in | ios::binary);

    allPkg::allInfo al;

    if (!al.ParseFromIstream(&input))
        return;

    input.close();

    int nFlag = al.num_flag();
    auto strFlag = al.str_flag();

    int nBaseInfoSize = al.info_obj_size();
    std::vector<baseinfoObj> vecBaseInfo;
    for (int i = 0; i < nBaseInfoSize; i++)
    {
        baseinfoObj baseObj;
        auto obj = al.info_obj(i);
        baseObj.m_strVer = obj.ver();
        baseObj.m_strAuthor = obj.author();
        baseObj.m_strCreateTime = obj.create_time();
        baseObj.m_strEncrypt = obj.encrypt();
        baseObj.m_strMask = obj.mask();
        vecBaseInfo.emplace_back(baseObj);
    }

    int nMsgSize = al.msg_obj_size();
    std::vector<messageObj> vecMsgInfo;
    for (int i = 0; i < nMsgSize; i++)
    {
        messageObj msgObj;
        const MsgPkg::MessageTest  obj = al.msg_obj(i);
        msgObj.m_nDataTime = obj.datetime();
        msgObj.m_strHostName = obj.hostname();
        msgObj.m_strIP = obj.ip();
        msgObj.m_strInfo = obj.info();
        vecMsgInfo.emplace_back(msgObj);
    }
    std::cout << std::endl;
}

int main(void)
{
    fstream input("allMsg.db", ios::in | ios::binary);

    testAllItem();

    if (0)
    {
        int nSize = 3;

        Info info;

        int nSizeArr[] = { 106, 109, 124 };
        for (int i = 0; i < nSize; i++)
        {
            int n = info.ByteSizeLong();
            std::cout << input.tellg() << std::endl;

            char chData[1024];
            input.read(chData, nSizeArr[i]);

            fstream fsA("a.db", ios::out | ios::binary);
            fsA.write(chData, nSizeArr[i]);
            fsA.flush();
            fsA.close();

            fstream fsAX("a.db", ios::in | ios::binary);
            if (!info.ParseFromIstream(&fsAX))
            {
                cerr << "read data from file error." << endl;
                return -1;
            }
            fsAX.close();

            std::cout << input.tellg() << std::endl;
            n = info.ByteSizeLong();

            traceMsg(info);

            if (input.eof())
                break;
        }
    }

    std::cout << " ---------------------- " << std::endl;

    if (0)
    {
        fstream inputx("message.db", ios::in | ios::binary);
        char chData[1024];
        input.seekp(106 + 109 + 124);
        input.read(chData, 102);

        fstream fsB("b.db", ios::out | ios::binary);
        fsB.write(chData, 102);
        fsB.flush();
        fsB.close();

        fstream fsBX("b.db", ios::in | ios::binary);
        MessageTest msg;
        if (!msg.ParseFromIstream(&fsBX))
        {
            cerr << "read data from file error." << endl;
            return -1;
        }
        fsBX.close();

        traceMsg(msg);
    }

    cout << "\n----Read Successful\n";
    return 0;
}