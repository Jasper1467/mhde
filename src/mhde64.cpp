#if defined(_M_X64) || defined(__x86_64__)

#include "../include/mhde64.hpp"
#include "../include/table64.hpp"

#include <cstdint>
#include <cstring>

std::uint8_t g_nGroupFlag = 0;
std::uint8_t g_nCurrentOpcodeByte = 0;
std::uint8_t cflags = 0;
std::uint8_t opcode = 0;
std::uint8_t pref = 0;

std::uint8_t* g_pOpcodeIter = nullptr;
std::uint8_t* ht = nullptr;

// ModR/M byte layout
//
//  7   6   5   4   3   2   1   0
//+---+---+---+---+---+---+---+---+
//|  Mod  | Reg/Opcode |   R/M   |
//+---+---+---+---+---+---+---+---+

std::uint8_t g_nMod = 0;
std::uint8_t g_nReg = 0;
std::uint8_t g_nRm = 0;
std::uint8_t g_nDispSize = 0;

std::uint8_t op64 = 0;

mhde64s* hs = nullptr;

#define CHECK_REX(opcode) (((opcode) & 0xf0) == 0x40)
#define CHECK_ESP_REGISTER(opcode) (((opcode) & -3) == 0x24)

#define IMM_BASED_INCREMENT_AMOUNT(n) ((n) >> 3) // <=> (n/8)

// Checks if the most significant bit (MSB) of a variable is set to 1
#define INSPECT_MSB(n) ((n) & 0x80)

void ResetGlobals()
{
    g_nGroupFlag = 0;
    g_nCurrentOpcodeByte = 0;
    cflags = 0;
    opcode = 0;
    pref = 0;

    g_nMod = 0;
    g_nReg = 0;
    g_nRm = 0;
    g_nDispSize = 0;

    op64 = 0;
}

void error_operand()
{
    // Add (opcode) error flags
    hs->flags |= F_ERROR | F_ERROR_OPERAND;
}

void error_opcode()
{
    // Add (operand) error flags
    // Set error flags in the instruction structure
    hs->flags |= F_ERROR | F_ERROR_OPCODE;
}

void error_opcode_increment()
{
    error_operand();
    cflags = 0;

    // Check if the opcode is a special case
    if (CHECK_ESP_REGISTER(opcode))
        cflags++;
}

void error_lock()
{
    // Add (lock) error flags
    hs->flags |= F_ERROR | F_ERROR_LOCK;
}

void ProcessPrefixes()
{
    // Iterate over the prefixes
    for (int i = 16; i; i--)
    {
        // Switch based on the current prefix
        switch (g_nCurrentOpcodeByte = *g_pOpcodeIter++)
        {
        case PREFIX_REPX:
            hs->p_rep = g_nCurrentOpcodeByte;
            pref |= F_PREFIX_REPX;
            break;
        case PREFIX_REPNZ:
            hs->p_rep = g_nCurrentOpcodeByte;
            pref |= F_PREFIX_REPNZ;
            break;
        case PREFIX_LOCK:
            hs->p_lock = g_nCurrentOpcodeByte;
            pref |= F_PREFIX_LOCK;
            break;
        // Handle segment prefixes
        case PREFIX_SEGMENT_CS:
        case PREFIX_SEGMENT_SS:
        case PREFIX_SEGMENT_DS:
        case PREFIX_SEGMENT_ES:
        case PREFIX_SEGMENT_FS:
        case PREFIX_SEGMENT_GS:
            hs->p_seg = g_nCurrentOpcodeByte;
            pref |= F_PREFIX_SEG;
            break;
        case PREFIX_OPERAND_SIZE:
            hs->p_66 = g_nCurrentOpcodeByte;
            pref |= F_PREFIX_66;
            break;
        case PREFIX_ADDRESS_SIZE:
            hs->p_67 = g_nCurrentOpcodeByte;
            pref |= F_PREFIX_67;
            break;
        default:
            // Exit the loop if an unrecognized prefix is encountered
            goto pref_done;
        }
    }

pref_done:

    // Set the flags field in the instruction structure
    hs->flags = static_cast<std::uint32_t>(pref) << 23;

    // If no prefix is set, set it to PRE_NONE
    if (!pref)
        pref |= PRE_NONE;

    // Check for REX prefix
    if (CHECK_REX(g_nCurrentOpcodeByte))
    {
        // Set the REX flag in the instruction structure
        hs->flags |= F_PREFIX_REX;

        // Check for the presence of REX.W and adjust the opcode accordingly
        if ((hs->rex_w = (g_nCurrentOpcodeByte & 0xf) >> 3) && (*g_pOpcodeIter & 0xf8) == 0xb8)
            op64++;

        // Extract REX.R, REX.X, and REX.B flags
        hs->rex_r = (g_nCurrentOpcodeByte & 7) >> 2;
        hs->rex_x = (g_nCurrentOpcodeByte & 3) >> 1;
        hs->rex_b = g_nCurrentOpcodeByte & 1;

        // Check for the presence of another REX prefix
        if (CHECK_REX(g_nCurrentOpcodeByte = *g_pOpcodeIter++))
        {
            // Set opcode and handle error
            opcode = g_nCurrentOpcodeByte;
            error_opcode();
        }
    }

    // Check for 0x0F opcode
    if ((hs->opcode = g_nCurrentOpcodeByte) == 0x0f)
    {
        // Set the secondary opcode and adjust pointer
        hs->opcode2 = g_nCurrentOpcodeByte = *g_pOpcodeIter++;
        ht += DELTA_OPCODES;
    }
    // Check for opcodes 0xA0 to 0xA3
    else if (g_nCurrentOpcodeByte >= 0xa0 && g_nCurrentOpcodeByte <= 0xa3)
    {
        // Increment op64 count
        op64++;

        // Check for 0x66 prefix
        if (pref & PRE_66)
            pref |= PRE_66;
        else
            pref &= ~PRE_66;
    }
}

void HandlePrefixLock()
{
    // Check if the instruction has a lock prefix
    if (!(pref & PRE_LOCK))
        return;

    // Check if the mod field is 3
    if (g_nMod == 3)
    {
        // If mod field is 3, set error flags
        error_lock();
        return;
    }

    // Determine the appropriate lookup table based on opcode and prefix state
    const std::uint8_t* pTableStart;
    const std::uint8_t* pTableEnd;
    std::uint8_t op = opcode;

    if (hs->opcode2)
    {
        pTableStart = mhde64_table + DELTA_OP2_LOCK_OK;
        pTableEnd = pTableStart + DELTA_OP_ONLY_MEM - DELTA_OP2_LOCK_OK;
    }
    else
    {
        pTableStart = mhde64_table + DELTA_OP_LOCK_OK;
        pTableEnd = pTableStart + DELTA_OP2_LOCK_OK - DELTA_OP_LOCK_OK;
        op &= ~0x01; // Clear the least significant bit
    }

    // Iterate through the lookup table to find a matching opcode
    for (const std::uint8_t* ht = pTableStart; ht != pTableEnd; ht++)
    {
        // Check if the opcode matches
        if (*ht == op)
        {
            // If the condition is met, check for lock error
            if (!(INSPECT_MSB(*(ht + 1) << g_nReg)))
                return; // No lock error, exit the function
            break;
        }
    }

    // If no match is found, or lock error detected, set error flags
    error_lock();
}

std::uint32_t disasm_done(const std::uint8_t* pCode)
{
    constexpr std::uint8_t MAX_DISASM_LENGTH = 15;

    // Calculate the length of the instruction
    hs->len = static_cast<std::uint8_t>(g_pOpcodeIter - reinterpret_cast<const std::uint8_t*>(pCode));

    // Check if the length exceeds the maximum allowable length
    if (hs->len > MAX_DISASM_LENGTH)
    {
        // If so, set error flags and truncate the length
        hs->flags |= F_ERROR | F_ERROR_LENGTH;
        hs->len = MAX_DISASM_LENGTH;
    }

    // Return the length of the instruction
    return static_cast<std::uint32_t>(hs->len);
}

std::uint32_t mhde64_disasm(const std::uint8_t* pCode, mhde64s* _hs)
{
    hs = _hs;

    g_pOpcodeIter = (std::uint8_t*)pCode;
    ht = mhde64_table;

    memset(hs, 0, sizeof(mhde64s));

    ResetGlobals();
    ProcessPrefixes();

    opcode = g_nCurrentOpcodeByte;

    // Each entry in mhde64_table corresponds to a group of four opcodes. So dividing the opcode by 4 and then taking
    // the remainder with % 4 helps determine the specific entry in mhde64_table that corresponds to the opcode.
    constexpr int OPCODE_TABLE_GROUP_SIZE = 4; // Can increase without fucking up?

    cflags = ht[ht[opcode / OPCODE_TABLE_GROUP_SIZE] + (opcode % OPCODE_TABLE_GROUP_SIZE)];
    if (cflags == C_ERROR)
        error_opcode_increment();

    g_nGroupFlag = 0;
    if (cflags & C_GROUP)
    {
        const std::uint16_t nGroupExtra = *reinterpret_cast<std::uint16_t*>(ht + (cflags & 0x7f));
        cflags = static_cast<std::uint8_t>(nGroupExtra);
        g_nGroupFlag = static_cast<std::uint8_t>(nGroupExtra >> 8);
    }

    if (hs->opcode2)
    {
        ht = mhde64_table + DELTA_PREFIXES;
        if (ht[ht[opcode / OPCODE_TABLE_GROUP_SIZE] + (opcode % OPCODE_TABLE_GROUP_SIZE)] & pref)
            error_opcode();
    }

    if (cflags & C_MODRM)
    {
        hs->flags |= F_MODRM;
        hs->modrm = g_nCurrentOpcodeByte = *g_pOpcodeIter++;

        // Extract the "mod" field from the ModR/M byte
        hs->modrm_mod = g_nMod = g_nCurrentOpcodeByte >> 6;

        // Extract the operand field from the ModR/M byte
        hs->modrm_rm = g_nRm = g_nCurrentOpcodeByte & 7;

        // Extract the register field from the ModR/M byte
        hs->modrm_reg = g_nReg = (g_nCurrentOpcodeByte & 0x3f) >> 3;

        if (g_nGroupFlag && (INSPECT_MSB(g_nGroupFlag << g_nReg)))
            error_opcode();

        if (!hs->opcode2 && opcode >= 0xd9 && opcode <= 0xdf)
        {
            std::uint8_t nOpcodeDelta = opcode - 0xd9;
            if (g_nMod == 3)
            {
                ht = mhde64_table + DELTA_FPU_MODRM + nOpcodeDelta * 8;
                nOpcodeDelta = ht[g_nReg] << g_nRm;
            }
            else
            {
                ht = mhde64_table + DELTA_FPU_REG;
                nOpcodeDelta = ht[nOpcodeDelta] << g_nReg;
            }

            if (INSPECT_MSB(nOpcodeDelta))
                error_opcode();
        }

        if (hs->opcode2)
        {
            switch (opcode)
            {
            case 0x20:
            case 0x22:
                g_nMod = 3;
                if (g_nReg > 4 || g_nReg == 1)
                    error_operand();
                else
                    goto no_error_operand;
            case 0x21:
            case 0x23:
                g_nMod = 3;
                if (g_nReg == 4 || g_nReg == 5)
                    error_operand();
                else
                    goto no_error_operand;
            }
        }
        else
        {
            switch (opcode)
            {
            case 0x8c:
                if (g_nReg > 5)
                    error_operand();
                else
                    goto no_error_operand;
            case 0x8e:
                if (g_nReg == 1 || g_nReg > 5)
                    error_operand();
                else
                    goto no_error_operand;
            }
        }

        if (g_nMod == 3)
        {
            std::uint8_t* pTableEnd;
            if (hs->opcode2)
            {
                ht = mhde64_table + DELTA_OP2_ONLY_MEM;
                pTableEnd = ht + sizeof(mhde64_table) - DELTA_OP2_ONLY_MEM;
            }
            else
            {
                ht = mhde64_table + DELTA_OP_ONLY_MEM;
                pTableEnd = ht + DELTA_OP2_ONLY_MEM - DELTA_OP_ONLY_MEM;
            }
            for (; ht != pTableEnd; ht += 2)
                if (*ht++ == opcode)
                {
                    if ((*ht++ & pref) && !(INSPECT_MSB(*ht << g_nReg)))
                        error_operand();
                    else
                        break;
                }
            goto no_error_operand;
        }
        else if (hs->opcode2)
        {
            switch (opcode)
            {
            case 0x50:
            case 0xd7:
            case 0xf7:
                if (pref & (PRE_NONE | PRE_66))
                    error_operand();
                break;
            case 0xd6:
                if (pref & (PRE_F2 | PRE_F3))
                    error_operand();
                break;
            case 0xc5:
                error_operand();
            default:;
            }
            goto no_error_operand;
        }
        else
            goto no_error_operand;

    no_error_operand:

        g_nCurrentOpcodeByte = *g_pOpcodeIter++;
        if (g_nReg <= 1)
        {
            if (opcode == 0xf6)
                cflags |= C_IMM8;
            else if (opcode == 0xf7)
                cflags |= C_IMM_P66;
        }

        switch (g_nMod)
        {
        case 0:
            if (pref & PRE_67)
            {
                if (g_nRm == 6)
                    g_nDispSize = 2;
            }
            else if (g_nRm == 5)
                g_nDispSize = 4;
            break;
        case 1:
            g_nDispSize = 1;
            break;
        case 2:
            g_nDispSize = 2;
            if (!(pref & PRE_67))
                g_nDispSize <<= 1;
            break;
        }

        if (g_nMod != 3 && g_nRm == 4)
        {
            hs->flags |= F_SIB;
            g_pOpcodeIter++;
            hs->sib = g_nCurrentOpcodeByte;
            hs->sib_scale = g_nCurrentOpcodeByte >> 6;
            hs->sib_index = (g_nCurrentOpcodeByte & 0x3f) >> 3;
            if ((hs->sib_base = g_nCurrentOpcodeByte & 7) == 5 && !(g_nMod & 1))
                g_nDispSize = 4;
        }

        g_pOpcodeIter--;
        switch (g_nDispSize)
        {
        case IMM_BASED_INCREMENT_AMOUNT(8):
            hs->flags |= F_DISP8;
            hs->disp.disp8 = *g_pOpcodeIter;
            break;
        case IMM_BASED_INCREMENT_AMOUNT(16):
            hs->flags |= F_DISP16;
            hs->disp.disp16 = *reinterpret_cast<std::uint16_t*>(g_pOpcodeIter);
            break;
        case IMM_BASED_INCREMENT_AMOUNT(4):
            hs->flags |= F_DISP32;
            hs->disp.disp32 = *reinterpret_cast<std::uint32_t*>(g_pOpcodeIter);
            break;
        default:;
        }
        g_pOpcodeIter += g_nDispSize;
    }
    else if (pref & PRE_LOCK)
        error_lock();

    if (cflags & C_IMM_P66)
    {
        if (cflags & C_REL32)
        {
            if (pref & PRE_66)
            {
                hs->flags |= F_IMM16 | F_RELATIVE;
                hs->imm.imm16 = *reinterpret_cast<std::uint16_t*>(g_pOpcodeIter);
                g_pOpcodeIter += IMM_BASED_INCREMENT_AMOUNT(16);
                return disasm_done(pCode);
            }
            goto rel32_ok;
        }
        if (op64)
        {
            hs->flags |= F_IMM64;
            hs->imm.imm64 = *reinterpret_cast<std::uint64_t*>(g_pOpcodeIter);
            g_pOpcodeIter += IMM_BASED_INCREMENT_AMOUNT(64);
        }
        else if (!(pref & PRE_66))
        {
            hs->flags |= F_IMM32;
            hs->imm.imm32 = *reinterpret_cast<std::uint32_t*>(g_pOpcodeIter);
            g_pOpcodeIter += IMM_BASED_INCREMENT_AMOUNT(32);
        }
        else
            goto imm16_ok;
    }

    if (cflags & C_IMM16)
    {
    imm16_ok:
        hs->flags |= F_IMM16;
        hs->imm.imm16 = *reinterpret_cast<std::uint16_t*>(g_pOpcodeIter);
        g_pOpcodeIter += IMM_BASED_INCREMENT_AMOUNT(16);
    }
    if (cflags & C_IMM8)
    {
        hs->flags |= F_IMM8;
        hs->imm.imm8 = *g_pOpcodeIter++;
    }

    if (cflags & C_REL32)
    {
    rel32_ok:
        hs->flags |= F_IMM32 | F_RELATIVE;
        hs->imm.imm32 = *reinterpret_cast<std::uint32_t*>(g_pOpcodeIter);
        g_pOpcodeIter += IMM_BASED_INCREMENT_AMOUNT(32);
    }
    else if (cflags & C_REL8)
    {
        hs->flags |= F_IMM8 | F_RELATIVE;
        hs->imm.imm8 = *g_pOpcodeIter++;
    }

    return disasm_done(pCode);
}

#endif // defined(_M_X64) || defined(__x86_64__)
