#include <iostream>
#include <map>
#include <list>
#include <string>
#include <vector>

int main()
{
    std::map<int, std::string> mapTest;
    mapTest.insert({ 1, "abc" });
    std::cout << "Size of map:" << sizeof(mapTest) << " -- " << mapTest.size() << std::endl;
    mapTest.insert({ 2, "def" });
    std::cout << "Size of map:" << sizeof(mapTest) << " -- " << mapTest.size() << std::endl;
    mapTest.insert({ 3, "fffffffffffffff" });
    std::cout << "Size of map:" << sizeof(mapTest) << " -- " << mapTest.size() << std::endl;

    std::vector<int> vInt;
    std::vector<float> vFloat;
    std::vector<double> vDouble;

    std::cout << sizeof(vInt) << " -- " << sizeof(vFloat) << " -- " << sizeof(vDouble) << std::endl;

    vInt.push_back(0);
    vInt.push_back(0);
    vInt.push_back(10);
    vInt.push_back(0);
    vInt.push_back(10);
    vInt.push_back(10);
    vInt.push_back(0);
    vInt.push_back(10);

    return 0;
}