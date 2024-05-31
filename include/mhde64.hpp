#pragma once
#include "pstdint.hpp"

enum Flags_e
{
    F_MODRM = 0x00000001,
    F_SIB = 0x00000002,
    F_IMM8 = 0x00000004,
    F_IMM16 = 0x00000008,
    F_IMM32 = 0x00000010,
    F_IMM64 = 0x00000020,
    F_DISP8 = 0x00000040,
    F_DISP16 = 0x00000080,
    F_DISP32 = 0x00000100,
    F_RELATIVE = 0x00000200,
    F_ERROR = 0x00001000,
    F_ERROR_OPCODE = 0x00002000,
    F_ERROR_LENGTH = 0x00004000,
    F_ERROR_LOCK = 0x00008000,
    F_ERROR_OPERAND = 0x00010000,
    F_PREFIX_REPNZ = 0x01000000,
    F_PREFIX_REPX = 0x02000000,
    F_PREFIX_REP = 0x03000000,
    F_PREFIX_66 = 0x04000000,
    F_PREFIX_67 = 0x08000000,
    F_PREFIX_LOCK = 0x10000000,
    F_PREFIX_SEG = 0x20000000,
    F_PREFIX_REX = 0x40000000,
    F_PREFIX_ANY = 0x7f000000
};

enum Prefixes_e
{
    PREFIX_SEGMENT_CS = 0x2e,
    PREFIX_SEGMENT_SS = 0x36,
    PREFIX_SEGMENT_DS = 0x3e,
    PREFIX_SEGMENT_ES = 0x26,
    PREFIX_SEGMENT_FS = 0x64,
    PREFIX_SEGMENT_GS = 0x65,
    PREFIX_LOCK = 0xf0,
    PREFIX_REPNZ = 0xf2,
    PREFIX_REPX = 0xf3,
    PREFIX_OPERAND_SIZE = 0x66,
    PREFIX_ADDRESS_SIZE = 0x67
};

#pragma pack(push, 1)

typedef struct
{
    UINT8 len;
    UINT8 p_rep;
    UINT8 p_lock;
    UINT8 p_seg;
    UINT8 p_66;
    UINT8 p_67;
    UINT8 rex;
    UINT8 rex_w;
    UINT8 rex_r;
    UINT8 rex_x;
    UINT8 rex_b;
    UINT8 opcode;
    UINT8 opcode2;
    UINT8 modrm;
    UINT8 modrm_mod;
    UINT8 modrm_reg;
    UINT8 modrm_rm;
    UINT8 sib;
    UINT8 sib_scale;
    UINT8 sib_index;
    UINT8 sib_base;

    union
    {
        UINT8 imm8;
        UINT16 imm16;
        UINT32 imm32;
        UINT64 imm64;
    } imm;

    union
    {
        UINT8 disp8;
        UINT16 disp16;
        UINT32 disp32;
    } disp;

    UINT32 flags;
} mhde64s;

#pragma pack(pop)

unsigned int mhde64_disasm(const void *pCode, mhde64s *pHs);
