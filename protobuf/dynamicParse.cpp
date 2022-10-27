#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>

#include "sylar.pb.h"

#include <functional>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

// filename proto文件名
// classname proto文件里面的class， 格式package.Classname
// cb, 创建好proto文件里面的classname类之后执行的回调，可以在该回调里面操作数据
int DynamicParseFromPBFile(const std::string &filename, const std::string &classname, std::function<void(::google::protobuf::Message *msg)> cb);

// proto_string proto文件的内容
// classname proto文件里面的class， 格式package.Classname
// cb, 创建好proto文件里面的classname类之后执行的回调，可以在该回调里面操作数据
int DynamicParseFromPBString(const std::string &proto_string, const std::string &classname, std::function<void(::google::protobuf::Message *msg)> cb);

int main(int argc, char **argv)
{
    //创建一个测试类，并进行初始化
    sylarPkg::Test test;
    test.set_name("test_name");
    test.set_age(100);
    test.add_phones("138xxxxxx");
    test.add_phones("139xxxxxx");
    double d = 9.8765432198765432;  // 测试精度问题
    test.set_dd(d);
    float f = 1.1234567f;
    test.set_ff(f);

    for (int i = 0; i < 3; ++i)
    {
        auto *a = test.add_aa();
        a->set_name("a_name_" + std::to_string(i));
        a->set_age(100 + i);
    }

    std::string pb_str;
    //把测试类，序列化到string中，以供后续解析
    test.SerializeToString(&pb_str);

    //打印测试类的数据信息
    std::cout << test.DebugString() << std::endl;
    std::cout << "===============================" << std::endl;

    //从文件中sylar.proto中的sylar.XX类，解析上面测试类的序列化二进制
    //并输出序列化后的sylar.XX的信息
    DynamicParseFromPBFile("sylar.proto", "sylar.XX", [pb_str](::google::protobuf::Message *msg)
                           {
        if(msg->ParseFromString(pb_str)) {
            std::cout << msg->DebugString() << std::endl;
        } });

    std::cout << "===============================" << std::endl;

    //从文件中sylar.proto中的sylar.Test，解析上面测试类的序列化二进制
    //并输出序列化后的sylar.Test的信息
    DynamicParseFromPBFile("sylar.proto", "sylar.Test", [pb_str](::google::protobuf::Message *msg)
                           {
        if(msg->ParseFromString(pb_str)) {
            std::cout << msg->DebugString() << std::endl;
        } });

    //字符串pb文件内容
    std::string pbstr = "package xx;\n"
                        "message BB { \n"
                        "    optional string name = 1; \n"
                        "    optional int32 age = 2; \n"
                        "} \n"
                        "message TT { \n"
                        "    optional string name = 1; \n"
                        "    optional int32 age = 2; \n"
                        "    repeated string phones = 3; \n"
                        "    repeated BB aa = 4; \n"
                        "}";

    std::cout << "===============================" << std::endl;

    // pbstr的proto信息中的xx.TT，解析上面测试类的序列化二进制
    //并输出序列化后的xx.TT的信息
    DynamicParseFromPBString(pbstr, "xx.TT", [pb_str](::google::protobuf::Message *msg)
                             {
        if(msg->ParseFromString(pb_str)) {
            std::cout << msg->DebugString() << std::endl;
        } });

    return 0;
}

int DynamicParseFromPBFile(const std::string &filename, const std::string &classname, std::function<void(::google::protobuf::Message *msg)> cb)
{
    // TODO 检查文件名是否合法
    auto pos = filename.find_last_of('/');
    std::string path;
    std::string file;
    if (pos == std::string::npos)
    {
        file = filename;
    }
    else
    {
        path = filename.substr(0, pos);
        file = filename.substr(pos + 1);
    }

    ::google::protobuf::compiler::DiskSourceTree sourceTree;
    sourceTree.MapPath("", path);
    ::google::protobuf::compiler::Importer importer(&sourceTree, NULL);
    importer.Import(file);
    const ::google::protobuf::Descriptor *descriptor = importer.pool()->FindMessageTypeByName(classname);
    if (!descriptor)
    {
        return 1;
    }
    ::google::protobuf::DynamicMessageFactory factory;
    const ::google::protobuf::Message *message = factory.GetPrototype(descriptor);
    if (!message)
    {
        return 2;
    }
    ::google::protobuf::Message *msg = message->New();
    if (!msg)
    {
        return 3;
    }
    cb(msg);
    delete msg;
    return 0;
}

int DynamicParseFromPBString(const std::string &proto_string, const std::string &classname, std::function<void(::google::protobuf::Message *msg)> cb)
{
    std::stringstream ss;
    ss << "/tmp/dps_" << rand() << "_" << rand() << ".proto";
    std::ofstream ofs(ss.str());
    ofs << proto_string;
    ofs.close();
    return DynamicParseFromPBFile(ss.str(), classname, cb);
}
