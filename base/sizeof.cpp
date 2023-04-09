#include <iostream>
#include <map>
#include <list>
#include <string>

int main()
{
    std::map<int, std::string> mapTest;
    mapTest.insert({1, "abc"});
    std::cout<<"Size of map"<<sizeof(mapTest)<<" -- "<<mapTest.size;
    mapTest.insert({2, "def"});
    std::cout<<"Size of map"<<sizeof(mapTest)<<" -- "<<mapTest.size;
    mapTest.insert({3, "fffffffffffffff"});
    std::cout<<"Size of map"<<sizeof(mapTest)<<" -- "<<mapTest.size;

}