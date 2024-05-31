#include "../include/wrapper.hpp"

#include <iostream>
#include <stdexcept>

void TestDisassemble()
{
    unsigned char pCode[] = { 0x90, 0x90, 0x90 }; // NOP

    CMHDE mhde;
    const unsigned int nResult = mhde.Disassemble(pCode);

    if (nResult != 1)
        throw std::runtime_error("Test failed: expected 1 byte processed.");
}

int main()
{
    try 
    {
        TestDisassemble();
        printf("Tests passed\n");
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << '\n';
        return 1;
    }

    return 0;
}
