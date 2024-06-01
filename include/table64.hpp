#pragma once

enum ClassFlags_e
{
    // No class flags
    C_NONE = 0x00,

    // ModR/M byte present
    C_MODRM = 0x01,

    // 8-bit immediate value present
    C_IMM8 = 0x02,

    // 16-bit immediate value present
    C_IMM16 = 0x04,

    // Immediate value prefixed with 66h
    C_IMM_P66 = 0x10,

    // 8-bit relative offset present
    C_REL8 = 0x20,

    // 32-bit relative offset present
    C_REL32 = 0x40,

    // Group flag set
    C_GROUP = 0x80,

    // Error flag set
    C_ERROR = 0xff
};

enum PrefixFlags_e
{
    PRE_ANY = 0x00,
    PRE_NONE = 0x01,
    PRE_F2 = 0x02,
    PRE_F3 = 0x04,
    PRE_66 = 0x08,
    PRE_67 = 0x10,
    PRE_LOCK = 0x20,
    PRE_SEG = 0x40,
    PRE_ALL = 0xff
};

enum DeltaValues_e
{
    // Offset for the secondary opcode table
    DELTA_OPCODES = 0x4a,

    // Offset for the FPU register table
    DELTA_FPU_REG = 0xfd,

    // Offset for the FPU register table when using MODRM addressing
    DELTA_FPU_MODRM = 0x104,

    // Offset for the prefixes table
    DELTA_PREFIXES = 0x13c,

    // Offset for the opcode table when lock prefix is allowed
    DELTA_OP_LOCK_OK = 0x1ae,

    // Offset for the secondary opcode table when lock prefix is allowed
    DELTA_OP2_LOCK_OK = 0x1c6,

    // Offset for the opcode table when only memory operands are allowed
    DELTA_OP_ONLY_MEM = 0x1d8,

    // Offset for the secondary opcode table when only memory operands are allowed
    DELTA_OP2_ONLY_MEM = 0x1e7
};

// Table for decoding x64 instructions
// This table is used to determine the behavior of various x64 opcodes
// Each entry corresponds to a specific opcode or instruction pattern
inline unsigned char mhde64_table[] = {
    0xa5, 0xaa, 0xa5, 0xb8, 0xa5, 0xaa, 0xa5, 0xaa, 0xa5, 0xb8, 0xa5, 0xb8, 0xa5, 0xb8, 0xa5, 0xb8, 0xc0, 0xc0, 0xc0,
    0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xac, 0xc0, 0xcc, 0xc0, 0xa1, 0xa1, 0xa1, 0xa1, 0xb1, 0xa5, 0xa5, 0xa6, 0xc0, 0xc0,
    0xd7, 0xda, 0xe0, 0xc0, 0xe4, 0xc0, 0xea, 0xea, 0xe0, 0xe0, 0x98, 0xc8, 0xee, 0xf1, 0xa5, 0xd3, 0xa5, 0xa5, 0xa1,
    0xea, 0x9e, 0xc0, 0xc0, 0xc2, 0xc0, 0xe6, 0x03, 0x7f, 0x11, 0x7f, 0x01, 0x7f, 0x01, 0x3f, 0x01, 0x01, 0xab, 0x8b,
    0x90, 0x64, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b, 0x92, 0x5b, 0x5b, 0x76, 0x90, 0x92, 0x92, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b,
    0x5b, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b, 0x6a, 0x73, 0x90, 0x5b, 0x52, 0x52, 0x52, 0x52, 0x5b, 0x5b, 0x5b, 0x5b,
    0x77, 0x7c, 0x77, 0x85, 0x5b, 0x5b, 0x70, 0x5b, 0x7a, 0xaf, 0x76, 0x76, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b, 0x5b,
    0x5b, 0x5b, 0x5b, 0x5b, 0x86, 0x01, 0x03, 0x01, 0x04, 0x03, 0xd5, 0x03, 0xd5, 0x03, 0xcc, 0x01, 0xbc, 0x03, 0xf0,
    0x03, 0x03, 0x04, 0x00, 0x50, 0x50, 0x50, 0x50, 0xff, 0x20, 0x20, 0x20, 0x20, 0x01, 0x01, 0x01, 0x01, 0xc4, 0x02,
    0x10, 0xff, 0xff, 0xff, 0x01, 0x00, 0x03, 0x11, 0xff, 0x03, 0xc4, 0xc6, 0xc8, 0x02, 0x10, 0x00, 0xff, 0xcc, 0x01,
    0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x01, 0xff, 0xff, 0xc0, 0xc2, 0x10, 0x11, 0x02, 0x03, 0x01,
    0x01, 0x01, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x10, 0x10, 0x10, 0x10,
    0x02, 0x10, 0x00, 0x00, 0xc6, 0xc8, 0x02, 0x02, 0x02, 0x02, 0x06, 0x00, 0x04, 0x00, 0x02, 0xff, 0x00, 0xc0, 0xc2,
    0x01, 0x01, 0x03, 0x03, 0x03, 0xca, 0x40, 0x00, 0x0a, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x33, 0x01,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xbf, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0xff, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0xbf,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x40, 0x40, 0x40, 0x40, 0x41, 0x49, 0x40,
    0x40, 0x40, 0x40, 0x4c, 0x42, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x4f, 0x44, 0x53, 0x40, 0x40, 0x40,
    0x44, 0x57, 0x43, 0x5c, 0x40, 0x60, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
    0x40, 0x64, 0x66, 0x6e, 0x6b, 0x40, 0x40, 0x6a, 0x46, 0x40, 0x40, 0x44, 0x46, 0x40, 0x40, 0x5b, 0x44, 0x40, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x06, 0x06, 0x06, 0x06, 0x01, 0x06, 0x06, 0x02, 0x06, 0x06, 0x00, 0x06, 0x00, 0x0a, 0x0a,
    0x00, 0x00, 0x00, 0x02, 0x07, 0x07, 0x06, 0x02, 0x0d, 0x06, 0x06, 0x06, 0x0e, 0x05, 0x05, 0x02, 0x02, 0x00, 0x00,
    0x04, 0x04, 0x04, 0x04, 0x05, 0x06, 0x06, 0x06, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x08, 0x00, 0x10, 0x00, 0x18,
    0x00, 0x20, 0x00, 0x28, 0x00, 0x30, 0x00, 0x80, 0x01, 0x82, 0x01, 0x86, 0x00, 0xf6, 0xcf, 0xfe, 0x3f, 0xab, 0x00,
    0xb0, 0x00, 0xb1, 0x00, 0xb3, 0x00, 0xba, 0xf8, 0xbb, 0x00, 0xc0, 0x00, 0xc1, 0x00, 0xc7, 0xbf, 0x62, 0xff, 0x00,
    0x8d, 0xff, 0x00, 0xc4, 0xff, 0x00, 0xc5, 0xff, 0x00, 0xff, 0xff, 0xeb, 0x01, 0xff, 0x0e, 0x12, 0x08, 0x00, 0x13,
    0x09, 0x00, 0x16, 0x08, 0x00, 0x17, 0x09, 0x00, 0x2b, 0x09, 0x00, 0xae, 0xff, 0x07, 0xb2, 0xff, 0x00, 0xb4, 0xff,
    0x00, 0xb5, 0xff, 0x00, 0xc3, 0x01, 0x00, 0xc7, 0xff, 0xbf, 0xe7, 0x08, 0x00, 0xf0, 0x02, 0x00
};
