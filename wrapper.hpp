#pragma oce

#if defined(_M_IX86) || defined(__i386__)
    #include "mhde32.hpp"
    typedef mhde32s MHDE_STATE;
    #define MHDE_DISASM(code, state) mhde32_disasm(code, state)
#elif defined(_M_X64) || defined(__x86_64__)
    #include "mhde64.hpp"
    typedef mhde64s MHDE_STATE;
    #define MHDE_DISASM(code, state) mhde64_disasm(code, state)
#else 
    #error "Unsupported architecture"
#endif // defined(_M_IX86) || defined(__i386__)

class CMHDE
{
public:
    CMHDE()
    {
        m_pState = nullptr;
    }

    ~CMHDE()
    {
        m_pState = nullptr;
    }

    unsigned int Disassemble(const void* pCode)
    {
        return MHDE_DISASM(pCode, m_pState);
    }

private:
    MHDE_STATE* m_pState;
};
