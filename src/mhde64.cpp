#if defined(_M_X64) || defined(__x86_64__)

#include "../include/mhde64.hpp"
#include "../include/table64.hpp"

#include <cstring>

unsigned int mhde64_disasm(const void *pCode, mhde64s *pHs)
{
    UINT8 x;
    UINT8 c;
    UINT8 cflags;
    UINT8 opcode;
    UINT8 pref = 0;

    UINT8 *p = (UINT8 *)pCode;
    UINT8 *ht = mhde64_table;

    UINT8 nMod;
    UINT8 nReg;
    UINT8 nRm;
    UINT8 nDispSize = 0;

    UINT8 op64 = 0;

    memset(pHs, 0, sizeof(mhde64s));

    for (x = 16; x; x--)
        switch (c = *p++)
        {
        case 0xf3:
            pHs->p_rep = c;
            pref |= PRE_F3;
            break;
        case 0xf2:
            pHs->p_rep = c;
            pref |= PRE_F2;
            break;
        case 0xf0:
            pHs->p_lock = c;
            pref |= PRE_LOCK;
            break;
        case 0x26:
        case 0x2e:
        case 0x36:
        case 0x3e:
        case 0x64:
        case 0x65:
            pHs->p_seg = c;
            pref |= PRE_SEG;
            break;
        case 0x66:
            pHs->p_66 = c;
            pref |= PRE_66;
            break;
        case 0x67:
            pHs->p_67 = c;
            pref |= PRE_67;
            break;
        default:
            goto pref_done;
        }
pref_done:

    pHs->flags = static_cast<UINT32>(pref) << 23;

    if (!pref)
        pref |= PRE_NONE;

    if ((c & 0xf0) == 0x40)
    {
        pHs->flags |= F_PREFIX_REX;
        if ((pHs->rex_w = (c & 0xf) >> 3) && (*p & 0xf8) == 0xb8)
            op64++;
        pHs->rex_r = (c & 7) >> 2;
        pHs->rex_x = (c & 3) >> 1;
        pHs->rex_b = c & 1;
        if (((c = *p++) & 0xf0) == 0x40)
        {
            opcode = c;
            goto error_opcode;
        }
    }

    if ((pHs->opcode = c) == 0x0f)
    {
        pHs->opcode2 = c = *p++;
        ht += DELTA_OPCODES;
    }
    else if (c >= 0xa0 && c <= 0xa3)
    {
        op64++;
        if (pref & PRE_67)
            pref |= PRE_66;
        else
            pref &= ~PRE_66;
    }

    opcode = c;
    cflags = ht[ht[opcode / 4] + (opcode % 4)];

    if (cflags == C_ERROR)
    {
    error_opcode:
        pHs->flags |= F_ERROR | F_ERROR_OPCODE;
        cflags = 0;
        if ((opcode & -3) == 0x24)
            cflags++;
    }

    x = 0;
    if (cflags & C_GROUP)
    {
        const UINT16 t = *reinterpret_cast<UINT16 *>(ht + (cflags & 0x7f));
        cflags = static_cast<UINT8>(t);
        x = static_cast<UINT8>(t >> 8);
    }

    if (pHs->opcode2)
    {
        ht = mhde64_table + DELTA_PREFIXES;
        if (ht[ht[opcode / 4] + (opcode % 4)] & pref)
            pHs->flags |= F_ERROR | F_ERROR_OPCODE;
    }

    if (cflags & C_MODRM)
    {
        pHs->flags |= F_MODRM;
        pHs->modrm = c = *p++;
        pHs->modrm_mod = nMod = c >> 6;
        pHs->modrm_rm = nRm = c & 7;
        pHs->modrm_reg = nReg = (c & 0x3f) >> 3;

        if (x && ((x << nReg) & 0x80))
            pHs->flags |= F_ERROR | F_ERROR_OPCODE;

        if (!pHs->opcode2 && opcode >= 0xd9 && opcode <= 0xdf)
        {
            UINT8 t = opcode - 0xd9;
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
                pHs->flags |= F_ERROR | F_ERROR_OPCODE;
        }

        if (pref & PRE_LOCK)
        {
            if (nMod == 3)
            {
                pHs->flags |= F_ERROR | F_ERROR_LOCK;
            }
            else
            {
                UINT8 *table_end, op = opcode;
                if (pHs->opcode2)
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
                pHs->flags |= F_ERROR | F_ERROR_LOCK;
            no_lock_error:;
            }
        }

        if (pHs->opcode2)
        {
            switch (opcode)
            {
            case 0x20:
            case 0x22:
                nMod = 3;
                if (nReg > 4 || nReg == 1)
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x21:
            case 0x23:
                nMod = 3;
                if (nReg == 4 || nReg == 5)
                    goto error_operand;
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
                    goto error_operand;
                else
                    goto no_error_operand;
            case 0x8e:
                if (nReg == 1 || nReg > 5)
                    goto error_operand;
                else
                    goto no_error_operand;
            }
        }

        if (nMod == 3)
        {
            UINT8 *table_end;
            if (pHs->opcode2)
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
                        goto error_operand;
                    else
                        break;
                }
            goto no_error_operand;
        }
        else if (pHs->opcode2)
        {
            switch (opcode)
            {
            case 0x50:
            case 0xd7:
            case 0xf7:
                if (pref & (PRE_NONE | PRE_66))
                    goto error_operand;
                break;
            case 0xd6:
                if (pref & (PRE_F2 | PRE_F3))
                    goto error_operand;
                break;
            case 0xc5:
                goto error_operand;
            default:;
            }
            goto no_error_operand;
        }
        else
            goto no_error_operand;

    error_operand:
        pHs->flags |= F_ERROR | F_ERROR_OPERAND;
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
            pHs->flags |= F_SIB;
            p++;
            pHs->sib = c;
            pHs->sib_scale = c >> 6;
            pHs->sib_index = (c & 0x3f) >> 3;
            if ((pHs->sib_base = c & 7) == 5 && !(nMod & 1))
                nDispSize = 4;
        }

        p--;
        switch (nDispSize)
        {
        case 1:
            pHs->flags |= F_DISP8;
            pHs->disp.disp8 = *p;
            break;
        case 2:
            pHs->flags |= F_DISP16;
            pHs->disp.disp16 = *reinterpret_cast<UINT16 *>(p);
            break;
        case 4:
            pHs->flags |= F_DISP32;
            pHs->disp.disp32 = *reinterpret_cast<UINT32 *>(p);
            break;
        default:;
        }
        p += nDispSize;
    }
    else if (pref & PRE_LOCK)
        pHs->flags |= F_ERROR | F_ERROR_LOCK;

    if (cflags & C_IMM_P66)
    {
        if (cflags & C_REL32)
        {
            if (pref & PRE_66)
            {
                pHs->flags |= F_IMM16 | F_RELATIVE;
                pHs->imm.imm16 = *reinterpret_cast<UINT16 *>(p);
                p += 2;
                goto disasm_done;
            }
            goto rel32_ok;
        }
        if (op64)
        {
            pHs->flags |= F_IMM64;
            pHs->imm.imm64 = *reinterpret_cast<UINT64 *>(p);
            p += 8;
        }
        else if (!(pref & PRE_66))
        {
            pHs->flags |= F_IMM32;
            pHs->imm.imm32 = *reinterpret_cast<UINT32 *>(p);
            p += 4;
        }
        else
            goto imm16_ok;
    }

    if (cflags & C_IMM16)
    {
    imm16_ok:
        pHs->flags |= F_IMM16;
        pHs->imm.imm16 = *reinterpret_cast<UINT16 *>(p);
        p += 2;
    }
    if (cflags & C_IMM8)
    {
        pHs->flags |= F_IMM8;
        pHs->imm.imm8 = *p++;
    }

    if (cflags & C_REL32)
    {
    rel32_ok:
        pHs->flags |= F_IMM32 | F_RELATIVE;
        pHs->imm.imm32 = *reinterpret_cast<UINT32 *>(p);
        p += 4;
    }
    else if (cflags & C_REL8)
    {
        pHs->flags |= F_IMM8 | F_RELATIVE;
        pHs->imm.imm8 = *p++;
    }

disasm_done:

    if ((pHs->len = static_cast<UINT8>(p - (UINT8 *)pCode)) > 15)
    {
        pHs->flags |= F_ERROR | F_ERROR_LENGTH;
        pHs->len = 15;
    }

    return (unsigned int)pHs->len;
}

#endif // defined(_M_X64) || defined(__x86_64__)
