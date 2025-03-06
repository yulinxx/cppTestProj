// // writer.cpp
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

int main(void)
{
    fstream output("allMsg.db", ios::out | ios::trunc | ios::binary);
    // fstream output("message.db", ios::out|ios::trunc);

    std::vector<baseinfoObj> vecBaseInfo;

    {
        baseinfoObj infoObj;
        infoObj.m_strVer = "1.0";
        infoObj.m_strAuthor = "a";
        infoObj.m_strCreateTime = "2022.01.01";
        infoObj.m_strEncrypt = "encryptStr";
        infoObj.m_strMask = "maskStr";
        vecBaseInfo.emplace_back(infoObj);
    }

    {
        baseinfoObj infoObj;
        infoObj.m_strVer = "1.1";
        infoObj.m_strAuthor = "b";
        infoObj.m_strCreateTime = "2022.01.02";
        infoObj.m_strEncrypt = "encryptStr2";
        infoObj.m_strMask = "maskStr2";
        vecBaseInfo.emplace_back(infoObj);
    }

    {
        baseinfoObj infoObj;
        infoObj.m_strVer = "1.2";
        infoObj.m_strAuthor = "c";
        infoObj.m_strCreateTime = "2022.01.03";
        infoObj.m_strEncrypt = "encryptStr3";
        infoObj.m_strMask = "maskStr3";
        vecBaseInfo.emplace_back(infoObj);
    }

    allPkg::allInfo al;
    al.set_num_flag(999);
    al.set_str_flag("ssssstring Flag test");
    for (const auto& item : vecBaseInfo)
    {
        basePkg::Info* info = al.add_info_obj();
        info->set_ver(item.m_strVer);
        info->set_author(item.m_strAuthor);
        info->set_create_time(item.m_strCreateTime);
        info->set_encrypt(item.m_strEncrypt);
        info->set_mask(item.m_strMask);
    }

    std::vector<messageObj> vecMsgInfo;

    {
        messageObj msgObj;
        msgObj.m_nDataTime = 1;
        msgObj.m_strHostName = "abcA";
        msgObj.m_strIP = "192.168.1.1";
        msgObj.m_strInfo = "baseInfo.prot";
        vecMsgInfo.emplace_back(msgObj);
    }

    {
        messageObj msgObj;
        msgObj.m_nDataTime = 2;
        msgObj.m_strHostName = "abcB";
        msgObj.m_strIP = "192.168.1.2";
        msgObj.m_strInfo = "encryptStr2";
        vecMsgInfo.emplace_back(msgObj);
    }

    {
        messageObj msgObj;
        msgObj.m_nDataTime = 3;
        msgObj.m_strHostName = "abcxxxxcC";
        msgObj.m_strIP = "192.168.1.3";
        msgObj.m_strInfo = "repeated MsgPkg.MessageTes";
        vecMsgInfo.emplace_back(msgObj);
    }

    {
        messageObj msgObj;
        msgObj.m_nDataTime = 4;
        msgObj.m_strHostName = "abcD";
        msgObj.m_strIP = "192.168.1.4";
        msgObj.m_strInfo = "message MessageTest{";
        vecMsgInfo.emplace_back(msgObj);
    }

    MsgPkg::MessageTest msg;
    for (const auto& item : vecMsgInfo)
    {
        MsgPkg::MessageTest* msg = al.add_msg_obj();
        msg->set_datetime(item.m_nDataTime);
        msg->set_hostname(item.m_strHostName);
        msg->set_ip(item.m_strIP);
        msg->set_info(item.m_strInfo);
    }

    // fstream output2("message.db", ios::out|ios::trunc|ios::ate|ios::binary);
    // fstream output2("message.db", ios::out | ios::app | ios::binary);
    if (!al.SerializeToOstream(&output))
    {
        cerr << "save data error." << endl;
        return -1;
    }

    if (0)
    {
        basePkg::Info info;

        int n = info.ByteSizeLong(); // 0

        info.set_ver("~1ver: 1");
        info.set_author("~1author: abc");

        int nA = info.ByteSizeLong(); // 25

        info.set_create_time("~1createTime: 2020.11.11");
        info.set_encrypt("~1Encrypt1");
        info.set_mask("~1mask: maskInfo1");

        if (!info.SerializeToOstream(&output))
        {
            cerr << "save data error." << endl;
            return -1;
        }

        int nB = info.ByteSizeLong(); // 106
        std::cout << info.DebugString() << std::endl;
        std::cout << "Size:" << nB << std::endl << std::endl;
    }

    if (0)
    {
        basePkg::Info info2;
        int nC = info2.ByteSizeLong();
        info2.set_ver("~2ver: 2");
        info2.set_author("~2author: 2bc");
        info2.set_create_time("~2createTime: 22020.22.22");
        info2.set_encrypt("~2Encrypt");
        info2.set_mask("~2mask: maskInfo test");

        // info2.MergeFrom(info);

        if (!info2.SerializeToOstream(&output))
        {
            cerr << "save data error." << endl;
            return -1;
        }

        int nD = info2.ByteSizeLong(); // 109
        std::cout << info2.DebugString() << std::endl;
        std::cout << "Size:" << nD << std::endl << std::endl;
    }

    if (0)
    {
        basePkg::Info info3;
        int nE = info3.ByteSizeLong();
        info3.set_ver("~3ver: 3");
        info3.set_author("~3author: 3bccd");
        info3.set_create_time("~3createTime: 2033.33.33");
        info3.set_encrypt("~3Encrypt333333");
        info3.set_mask("~3mask: maskInfo test33333END");

        // info2.MergeFrom(info);

        if (!info3.SerializeToOstream(&output))
        {
            cerr << "save data error." << endl;
            return -1;
        }

        int nF = info3.ByteSizeLong(); // 124
        std::cout << info3.DebugString() << std::endl;
        std::cout << "Size:" << nF << std::endl << std::endl;
    }

    // output.close();
    // return 0;

    if (0)
    {
        MsgPkg::MessageTest msg;
        msg.set_datetime(99999);
        msg.set_hostname("#--msg--hostName -dataNode-1000");
        msg.set_ip("#IP: 192.168.0.128");
        msg.set_info("#msgInfo: Everything is normal, I'm healthy");

        // fstream output2("message.db", ios::out|ios::trunc|ios::ate|ios::binary);
        // fstream output2("message.db", ios::out | ios::app | ios::binary);
        if (!msg.SerializeToOstream(&output))
        {
            cerr << "save data error." << endl;
            return -1;
        }

        int nG = msg.ByteSizeLong(); // 102
        std::cout << msg.DebugString() << std::endl;
        std::cout << "Size:" << nG << std::endl;
        // output2.close();
    }

    std::cout << "Write successfull" << std::endl;
    return 0;
}