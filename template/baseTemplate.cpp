#include <string>
// https://blog.csdn.net/c1sdn19/article/details/123669618
template <typename NumType>
class Obj
{

};


template <typename NumType, int Dimension>
class Obj3
{

};

// template <typename NumType>
// class Obj<NumType, 9>
// {

// };


int main()
{

    Obj<int> objA;
    Obj3<int, 2> objBError;
    // Obj<std::string, 2> objC();
    // Obj<int, 4> objD;

    int nA = 43, nB = 84;
    // test.Set(nA, nB);

    // Obj<int, 2> testB(test);

    int m = 00;

    return 0;
}
