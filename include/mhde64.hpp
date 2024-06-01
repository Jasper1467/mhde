#pragma once

#include <cstdint>

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

// #pragma pack(push, 1)

typedef struct
{
    // Length of the instruction in bytes
    std::uint8_t len;

    // Prefixes
    std::uint8_t p_rep;  // Repeat prefix
    std::uint8_t p_lock; // Lock prefix
    std::uint8_t p_seg;  // Segment override prefix
    std::uint8_t p_66;   // Operand size override prefix
    std::uint8_t p_67;   // Address size override prefix
    std::uint8_t rex;    // REX prefix
    std::uint8_t rex_w;  // REX.W bit
    std::uint8_t rex_r;  // REX.R bit
    std::uint8_t rex_x;  // REX.X bit
    std::uint8_t rex_b;  // REX.B bit

    // Opcodes
    std::uint8_t opcode;  // Primary opcode
    std::uint8_t opcode2; // Secondary opcode

    // ModR/M byte
    std::uint8_t modrm;     // ModR/M byte
    std::uint8_t modrm_mod; // Mod field
    std::uint8_t modrm_reg; // Reg field
    std::uint8_t modrm_rm;  // R/M field

    // SIB byte
    std::uint8_t sib;       // SIB byte
    std::uint8_t sib_scale; // SIB scale factor
    std::uint8_t sib_index; // SIB index
    std::uint8_t sib_base;  // SIB base

    // Immediate data
    union
    {
        std::uint8_t imm8;   // 8-bit immediate
        std::uint16_t imm16; // 16-bit immediate
        std::uint32_t imm32; // 32-bit immediate
        std::uint64_t imm64; // 64-bit immediate
    } imm;

    // Displacement data
    union
    {
        std::uint8_t disp8;   // 8-bit displacement
        std::uint16_t disp16; // 16-bit displacement
        std::uint32_t disp32; // 32-bit displacement
    } disp;

    // Flags indicating various properties of the instruction
    std::uint32_t flags;
} mhde64s;

// #pragma pack(pop)

std::uint32_t mhde64_disasm(const std::uint8_t* pCode, mhde64s* _hs);
