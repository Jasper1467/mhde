#include "../include/mhde_wrapper.hpp"

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
#if defined(_M_IX86) || defined(__i386__)
    printf("Testing 32 bit!\n");
#elif defined(_M_X64) || defined(__x86_64__)
    printf("Testing 64 bit!\n");
#else 
    #error "Unsupported architecture"
#endif // defined(_M_IX86) || defined(__i386__)

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
