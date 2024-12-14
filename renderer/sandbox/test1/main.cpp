
#include <iostream>
#include "huan/my_dll_test.h"

int main(int argc, char* argv[])
{
    std::cout << "Hello, from test1" << std::endl;
    huan_renderer::test_dll_func();

    return 0;
}
