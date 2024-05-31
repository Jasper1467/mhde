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
    hs->flags |= F_ERROR | F_ERROR_OPCODE;
    cflags = 0;
    if ((opcode & -3) == 0x24)
        cflags++;
}

void ProcessPrefixes()
{
    for (int x = 16; x; x--)
    {
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
            goto pref_done;
        }
    }

pref_done:

    hs->flags = static_cast<std::uint32_t>(pref) << 23;

    if (!pref)
        pref |= PRE_NONE;

    if ((c & 0xf0) == 0x40)
    {
        hs->flags |= F_PREFIX_REX;
        if ((hs->rex_w = (c & 0xf) >> 3) && (*p & 0xf8) == 0xb8)
            op64++;
        hs->rex_r = (c & 7) >> 2;
        hs->rex_x = (c & 3) >> 1;
        hs->rex_b = c & 1;
        if (((c = *p++) & 0xf0) == 0x40)
        {
            opcode = c;
            error_opcode();
        }
    }

    if ((hs->opcode = c) == 0x0f)
    {
        hs->opcode2 = c = *p++;
        ht += DELTA_OPCODES;
    }
    else if (c >= 0xa0 && c <= 0xa3)
    {
        op64++;
        if (pref & PRE_66)
            pref |= PRE_66;
        else
            pref &= ~PRE_66;
    }
}

void HandlePrefixLock()
{
    if (pref & PRE_LOCK)
    {
        if (nMod == 3)
        {
            hs->flags |= F_ERROR | F_ERROR_LOCK;
        }
        else
        {
            std::uint8_t *table_end, op = opcode;
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
            for (; ht != table_end; ht++)
                if (*ht++ == op)
                {
                    if (!((*ht << nReg) & 0x80))
                        goto no_lock_error;
                    else
                        break;
                }
            hs->flags |= F_ERROR | F_ERROR_LOCK;
        no_lock_error:;
        }
    }
}

unsigned int disasm_done(const void* pCode)
{
    if ((hs->len = static_cast<std::uint8_t>(p - (std::uint8_t*)pCode)) > 15)
    {
        hs->flags |= F_ERROR | F_ERROR_LENGTH;
        hs->len = 15;
    }

    return (unsigned int)hs->len;
}

void error_operand()
{
    hs->flags |= F_ERROR | F_ERROR_OPERAND;
}

unsigned int mhde64_disasm(const void* pCode, mhde64s* _hs)
{
    hs = _hs;

    p = (std::uint8_t*)pCode;
    ht = mhde64_table;

    memset(hs, 0, sizeof(mhde64s));

    ResetGlobals();
    ProcessPrefixes();

    opcode = c;
    cflags = ht[ht[opcode / 4] + (opcode % 4)];

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
        if (ht[ht[opcode / 4] + (opcode % 4)] & pref)
            hs->flags |= F_ERROR | F_ERROR_OPCODE;
    }

    if (cflags & C_MODRM)
    {
        hs->flags |= F_MODRM;
        hs->modrm = c = *p++;
        hs->modrm_mod = nMod = c >> 6;
        hs->modrm_rm = nRm = c & 7;
        hs->modrm_reg = nReg = (c & 0x3f) >> 3;

        if (x && ((x << nReg) & 0x80))
            hs->flags |= F_ERROR | F_ERROR_OPCODE;

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
                hs->flags |= F_ERROR | F_ERROR_OPCODE;
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
        hs->flags |= F_ERROR | F_ERROR_LOCK;

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
