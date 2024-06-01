#pragma once

#include <cstdint>

enum Flags_e
{
    F_MODRM = 0x00000001,
    F_SIB = 0x00000002,
    F_IMM8 = 0x00000004,
    F_IMM16 = 0x00000008,
    F_IMM32 = 0x00000010,
    F_DISP8 = 0x00000020,
    F_DISP16 = 0x00000040,
    F_DISP32 = 0x00000080,
    F_RELATIVE = 0x00000100,
    F_2IMM16 = 0x00000800,
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
    F_PREFIX_ANY = 0x3f000000
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

// #pragma pack(push, 1)

typedef struct
{
    // Length of the instruction
    std::uint8_t len;

    // Prefix bytes (REP and LOCK prefixes)
    std::uint8_t p_rep;
    std::uint8_t p_lock;

    // Segment override prefix
    std::uint8_t p_seg;

    // Operand size override prefix
    std::uint8_t p_66;

    // Address size override prefix
    std::uint8_t p_67;

    // Primary and secondary opcodes
    std::uint8_t opcode;
    std::uint8_t opcode2;

    // ModR/M byte components
    std::uint8_t modrm;
    std::uint8_t modrm_mod;
    std::uint8_t modrm_reg;
    std::uint8_t modrm_rm;

    // SIB byte components
    std::uint8_t sib;
    std::uint8_t sib_scale;
    std::uint8_t sib_index;
    std::uint8_t sib_base;

    // Immediate value (8, 16, or 32 bits)
    union
    {
        std::uint8_t imm8;
        std::uint16_t imm16;
        std::uint32_t imm32;
    } imm;

    // Displacement value (8, 16, or 32 bits)
    union
    {
        std::uint8_t disp8;
        std::uint16_t disp16;
        std::uint32_t disp32;
    } disp;

    // Instruction flags
    std::uint32_t flags;
} mhde32s;

// #pragma pack(pop)

std::uint32_t mhde32_disasm(const void* pCode, mhde32s* pHs);
