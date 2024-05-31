#if defined(_M_X64) || defined(__x86_64__)

#include "../include/mhde64.hpp"
#include "../include/table64.hpp"

#include <cstdint>
#include <cstring>

std::uint8_t x = 0;
std::uint8_t c = 0;
std::uint8_t cflags = 0;
std::uint8_t opcode = 0;
std::uint8_t pref = 0;

std::uint8_t* p = nullptr;
std::uint8_t* ht = nullptr;

std::uint8_t nMod = 0;
std::uint8_t nReg = 0;
std::uint8_t nRm = 0;
std::uint8_t nDispSize = 0;

std::uint8_t op64 = 0;

mhde64s* hs = nullptr;

#define CHECK_REX(c) ((c & 0xf0) == 0x40)

void ResetGlobals()
{
    x = 0;
    c = 0;
    cflags = 0;
    opcode = 0;
    pref = 0;

    nMod = 0;
    nReg = 0;
    nRm = 0;
    nDispSize = 0;

    op64 = 0;
}

void error_opcode()
{
    // Set error flags in the instruction structure
    hs->flags |= F_ERROR | F_ERROR_OPCODE;

    cflags = 0;

    // Check if the opcode is a special case
    if ((opcode & -3) == 0x24)
        cflags++;
}

void error_operand()
{
    // Add (operand) error flags
    hs->flags |= F_ERROR | F_ERROR_OPERAND;
}

void error_opcode()
{
    // Add (opcode) error flags
    hs->flags |= F_ERROR | F_ERROR_OPCODE;
}

void error_lock()
{
    // Add (lock) error flags
    hs->flags |= F_ERROR | F_ERROR_LOCK;
}

void ProcessPrefixes()
{
    // Iterate over the prefixes
    for (int x = 16; x; x--)
    {
        // Switch based on the current prefix
        switch (c = *p++)
        {
        case PREFIX_REPX:
            hs->p_rep = c;
            pref |= F_PREFIX_REPX;
            break;
        case PREFIX_REPNZ:
            hs->p_rep = c;
            pref |= F_PREFIX_REPNZ;
            break;
        case PREFIX_LOCK:
            hs->p_lock = c;
            pref |= F_PREFIX_LOCK;
            break;
        // Handle segment prefixes
        case PREFIX_SEGMENT_CS:
        case PREFIX_SEGMENT_SS:
        case PREFIX_SEGMENT_DS:
        case PREFIX_SEGMENT_ES:
        case PREFIX_SEGMENT_FS:
        case PREFIX_SEGMENT_GS:
            hs->p_seg = c;
            pref |= F_PREFIX_SEG;
            break;
        case PREFIX_OPERAND_SIZE:
            hs->p_66 = c;
            pref |= F_PREFIX_66;
            break;
        case PREFIX_ADDRESS_SIZE:
            hs->p_67 = c;
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
    if (CHECK_REX(c))
    {
        // Set the REX flag in the instruction structure
        hs->flags |= F_PREFIX_REX;

        // Check for the presence of REX.W and adjust the opcode accordingly
        if ((hs->rex_w = (c & 0xf) >> 3) && (*p & 0xf8) == 0xb8)
            op64++;

        // Extract REX.R, REX.X, and REX.B flags
        hs->rex_r = (c & 7) >> 2;
        hs->rex_x = (c & 3) >> 1;
        hs->rex_b = c & 1;

        // Check for the presence of another REX prefix
        if (CHECK_REX(c = *p++))
        {
            // Set opcode and handle error
            opcode = c;
            error_opcode();
        }
    }

    // Check for 0x0F opcode
    if ((hs->opcode = c) == 0x0f)
    {
        // Set the secondary opcode and adjust pointer
        hs->opcode2 = c = *p++;
        ht += DELTA_OPCODES;
    }
    // Check for opcodes 0xA0 to 0xA3
    else if (c >= 0xa0 && c <= 0xa3)
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
    if (pref & PRE_LOCK)
    {
        // Check if the mod field is 3
        if (nMod == 3)
        {
            // If mod field is 3, set error flags
            error_lock();
        }
        else
        {
            std::uint8_t *table_end, op = opcode;

            // Determine the appropriate lookup table based on opcode and prefix state
            if (hs->opcode2)
            {
                ht = mhde64_table + DELTA_OP2_LOCK_OK;
                table_end = ht + DELTA_OP_ONLY_MEM - DELTA_OP2_LOCK_OK;
            }
            else
            {
                ht = mhde64_table + DELTA_OP_LOCK_OK;
                table_end = ht + DELTA_OP2_LOCK_OK - DELTA_OP_LOCK_OK;
                op &= -2;
            }

            // Iterate through the lookup table to find a matching opcode
            for (; ht != table_end; ht++)
            {
                // Check if the opcode matches
                if (*ht++ == op)
                {
                    // If the condition is met, break out of the loop
                    if (!((*ht << nReg) & 0x80))
                        goto no_lock_error;
                    else
                        break;
                }
            }

            // If no match is found, set error flags
            error_lock();
        no_lock_error:;
        }
    }
}

std::uint32_t disasm_done(const void* pCode)
{
    constexpr std::uint8_t MAX_DISASM_LENGTH = 15;

    // Calculate the length of the instruction
    hs->len = static_cast<std::uint8_t>(p - reinterpret_cast<const std::uint8_t*>(pCode));

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

std::uint32_t mhde64_disasm(const void* pCode, mhde64s* _hs)
{
    hs = _hs;

    p = (std::uint8_t*)pCode;
    ht = mhde64_table;

    memset(hs, 0, sizeof(mhde64s));

    ResetGlobals();
    ProcessPrefixes();

    opcode = c;

    // Each entry in mhde64_table corresponds to a group of four opcodes. So dividing the opcode by 4 and then taking
    // the remainder with % 4 helps determine the specific entry in mhde64_table that corresponds to the opcode.
    constexpr int OPCODE_TABLE_GROUP_SIZE = 4;

    cflags = ht[ht[opcode / OPCODE_TABLE_GROUP_SIZE] + (opcode % OPCODE_TABLE_GROUP_SIZE)];
    if (cflags == C_ERROR)
        error_opcode();

    x = 0;
    if (cflags & C_GROUP)
    {
        const std::uint16_t t = *reinterpret_cast<std::uint16_t*>(ht + (cflags & 0x7f));
        cflags = static_cast<std::uint8_t>(t);
        x = static_cast<std::uint8_t>(t >> 8);
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
        hs->modrm = c = *p++;

        // Extract the "mod" field from the ModR/M byte
        hs->modrm_mod = nMod = c >> 6;

        // Extract the operand field from the ModR/M byte
        hs->modrm_rm = nRm = c & 7;

        // Extract the register field from the ModR/M byte
        hs->modrm_reg = nReg = (c & 0x3f) >> 3;

        if (x && ((x << nReg) & 0x80))
            error_opcode();

        if (!hs->opcode2 && opcode >= 0xd9 && opcode <= 0xdf)
        {
            std::uint8_t t = opcode - 0xd9;
            if (nMod == 3)
            {
                ht = mhde64_table + DELTA_FPU_MODRM + t * 8;
                t = ht[nReg] << nRm;
            }
            else
            {
                ht = mhde64_table + DELTA_FPU_REG;
                t = ht[t] << nReg;
            }
            if (t & 0x80)
                error_opcode();
        }

        if (hs->opcode2)
        {
            switch (opcode)
            {
            case 0x20:
            case 0x22:
                nMod = 3;
                if (nReg > 4 || nReg == 1)
                    error_operand();
                else
                    goto no_error_operand;
            case 0x21:
            case 0x23:
                nMod = 3;
                if (nReg == 4 || nReg == 5)
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
                if (nReg > 5)
                    error_operand();
                else
                    goto no_error_operand;
            case 0x8e:
                if (nReg == 1 || nReg > 5)
                    error_operand();
                else
                    goto no_error_operand;
            }
        }

        if (nMod == 3)
        {
            std::uint8_t* table_end;
            if (hs->opcode2)
            {
                ht = mhde64_table + DELTA_OP2_ONLY_MEM;
                table_end = ht + sizeof(mhde64_table) - DELTA_OP2_ONLY_MEM;
            }
            else
            {
                ht = mhde64_table + DELTA_OP_ONLY_MEM;
                table_end = ht + DELTA_OP2_ONLY_MEM - DELTA_OP_ONLY_MEM;
            }
            for (; ht != table_end; ht += 2)
                if (*ht++ == opcode)
                {
                    if ((*ht++ & pref) && !((*ht << nReg) & 0x80))
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

        c = *p++;
        if (nReg <= 1)
        {
            if (opcode == 0xf6)
                cflags |= C_IMM8;
            else if (opcode == 0xf7)
                cflags |= C_IMM_P66;
        }

        switch (nMod)
        {
        case 0:
            if (pref & PRE_67)
            {
                if (nRm == 6)
                    nDispSize = 2;
            }
            else if (nRm == 5)
                nDispSize = 4;
            break;
        case 1:
            nDispSize = 1;
            break;
        case 2:
            nDispSize = 2;
            if (!(pref & PRE_67))
                nDispSize <<= 1;
            break;
        }

        if (nMod != 3 && nRm == 4)
        {
            hs->flags |= F_SIB;
            p++;
            hs->sib = c;
            hs->sib_scale = c >> 6;
            hs->sib_index = (c & 0x3f) >> 3;
            if ((hs->sib_base = c & 7) == 5 && !(nMod & 1))
                nDispSize = 4;
        }

        p--;
        switch (nDispSize)
        {
        case 1:
            hs->flags |= F_DISP8;
            hs->disp.disp8 = *p;
            break;
        case 2:
            hs->flags |= F_DISP16;
            hs->disp.disp16 = *reinterpret_cast<std::uint16_t*>(p);
            break;
        case 4:
            hs->flags |= F_DISP32;
            hs->disp.disp32 = *reinterpret_cast<std::uint32_t*>(p);
            break;
        default:;
        }
        p += nDispSize;
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
                hs->imm.imm16 = *reinterpret_cast<std::uint16_t*>(p);
                p += 2;
                return disasm_done(pCode);
            }
            goto rel32_ok;
        }
        if (op64)
        {
            hs->flags |= F_IMM64;
            hs->imm.imm64 = *reinterpret_cast<std::uint64_t*>(p);
            p += 8;
        }
        else if (!(pref & PRE_66))
        {
            hs->flags |= F_IMM32;
            hs->imm.imm32 = *reinterpret_cast<std::uint32_t*>(p);
            p += 4;
        }
        else
            goto imm16_ok;
    }

    if (cflags & C_IMM16)
    {
    imm16_ok:
        hs->flags |= F_IMM16;
        hs->imm.imm16 = *reinterpret_cast<std::uint16_t*>(p);
        p += 2;
    }
    if (cflags & C_IMM8)
    {
        hs->flags |= F_IMM8;
        hs->imm.imm8 = *p++;
    }

    if (cflags & C_REL32)
    {
    rel32_ok:
        hs->flags |= F_IMM32 | F_RELATIVE;
        hs->imm.imm32 = *reinterpret_cast<std::uint32_t*>(p);
        p += 4;
    }
    else if (cflags & C_REL8)
    {
        hs->flags |= F_IMM8 | F_RELATIVE;
        hs->imm.imm8 = *p++;
    }

    return disasm_done(pCode);
}

#endif // defined(_M_X64) || defined(__x86_64__)
