// boost是基于C++11的被广泛使用的开源库;
// 多索引容器实现了可以通过多个索引去查找数据,不像std::map 一样 只能通过单一索引key查找对应value值.
// 多索引容器可以通过多个索引key查找对应value值.多对多的方式,有了更多的应用场景.
// 其实现了STL的一些基本操作,如迭代器等.如下示例作为对多索引容器的进一步了解.
// https://blog.csdn.net/weixin_44328568/article/details/129424977

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/shared_ptr.hpp>

using namespace std; /* 项目里不要这样 */

/* 定义学生信息,同理可以使用结构定义 */
class student
{
public:
    student(int id, string name, int score, string remark) :id(id), name(name), score(score), remark(remark)
    {}

    void print() const
    {
        cout << "\tid:" << id << "\tname:" << name << "\tscore:" << score << "\tremark:" << remark << endl;
    }
    int id;
    string name;
    int score;
    string remark;
};

/* 如果要把student某个属性字段设置为搜索引擎,则需要定义用于排序的空结构体对象 */
struct _id
{
};
struct _name
{
};
struct _score
{
};

// 定义一个multi_index_container(多索引容器)
using student_table =
boost::multi_index::multi_index_container<
    student,
    boost::multi_index::indexed_by<
    boost::multi_index::ordered_unique<boost::multi_index::tag<_id>, BOOST_MULTI_INDEX_MEMBER(student, int, id)>,   // ID为唯一索引,类似主键
    boost::multi_index::ordered_non_unique<boost::multi_index::tag<_name>, BOOST_MULTI_INDEX_MEMBER(student, string, name)>, // 非唯一索引
    boost::multi_index::ordered_non_unique<boost::multi_index::tag<_score>, BOOST_MULTI_INDEX_MEMBER(student, int, score)>
    >
>;

void insertData(student_table& allStu)
{
    allStu.insert(student(1, "lili", 85, "hello"));
    allStu.insert(student(2, "liming", 90, "hello"));
    allStu.insert(student(3, "xiaoming", 65, "hello"));
    allStu.insert(student(4, "ergou", 80, "hello"));
    allStu.insert(student(5, "dagou", 60, "hello"));
}

/* compile:  g++ -std=c++11 -I./include multi_index.cpp */
int main()
{
    student_table allStu;

    insertData(allStu); // 插入数据

    cout << endl << "-- sort by student id:" << endl << endl;
    for (auto& iter : allStu.get<_id>())
    { // get return of student_table::index<_id>::type&
        iter.print();
    }

    cout << endl << "-- sort by student name:" << endl << endl;
    for (auto& iter : allStu.get<_name>())
    { // get return of student_table::index<_name>::type&
        iter.print();
    }

    cout << endl << "-- sort by student score:" << endl << endl;
    for (auto& iter_score : allStu.get<_score>())
    { // student_table::index<_score>::type&
        iter_score.print();
    }

    // find and modify
    auto& stu_of_name = allStu.get<_name>(); // student_table::index<_name>::type
    auto iter_ergou = stu_of_name.find("ergou"); // student_table::index<_name>::type::iterator

    if (iter_ergou != stu_of_name.end())
    {
        student ergou = *iter_ergou;
        ergou.score = 33;
        ergou.remark = "modified by program";
        bool isSuc = stu_of_name.replace(iter_ergou, ergou);
    }

    cout << endl << "-- sort by student id after replace ergou:" << endl << endl;
    for (auto& iter : allStu.get<0>())
    { // get return of student_table::index<_id>::type&
        iter.print();
    }

    cout << endl;

    return 0;
}

/*
编译及结果:

-- sort by student id:

        id:1    name:lili       score:85        remark:hello
        id:2    name:liming     score:90        remark:hello
        id:3    name:xiaoming   score:65        remark:hello
        id:4    name:ergou      score:80        remark:hello
        id:5    name:dagou      score:60        remark:hello

-- sort by student name:

        id:5    name:dagou      score:60        remark:hello
        id:4    name:ergou      score:80        remark:hello
        id:1    name:lili       score:85        remark:hello
        id:2    name:liming     score:90        remark:hello
        id:3    name:xiaoming   score:65        remark:hello

-- sort by student score:

        id:5    name:dagou      score:60        remark:hello
        id:3    name:xiaoming   score:65        remark:hello
        id:4    name:ergou      score:80        remark:hello
        id:1    name:lili       score:85        remark:hello
        id:2    name:liming     score:90        remark:hello

-- sort by student id after replace ergou:

        id:1    name:lili       score:85        remark:hello
        id:2    name:liming     score:90        remark:hello
        id:3    name:xiaoming   score:65        remark:hello
        id:4    name:ergou      score:33        remark:modified by program
        id:5    name:dagou      score:60        remark:hello

-bash-4.2#

*/