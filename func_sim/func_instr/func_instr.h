//multi-include protection
#ifndef FUNC_INSTR_H
#define FUNC_INSTR_H

#include <iostream>
#include <ostream>
#include <cassert>

#include <types.h>

enum Registers
{
    ZERO, // constant 0
    AT,   // assebmler temporary
    V0, V1,
    A0, A1, A2, A3,
    T0, T1, T2, T3, T4, T5, T6, T7,
    S0, S1, S2, S3, S4, S5, S6, S7,
    T8, T9,
    K0, K1,
    GP,
    SP,
    S8,
    RA
};

 enum Instructions
{
    ADD, ADDU, SUB, SUBU, ADDI, ADDIU,
    MULT, MULTU, DIV, DIVU,
    MFHI, MTHI, MFLO, MTLO,
    SLL, SRL, SRA, SLLV, SRLV, SRAV,
    LUI,
    SLT, SLTU, SLTI, SLTIU,
    AND, OR, XOR, NOR, ANDI, ORI, XORI,
    BEQ, BNE, BLEZ, BGTZ,
    J, JAL, JR, JALR,
    LB, LH, LW, LBU, LHU,
    SB, SH, SW,
    SYSCALL, BREAK, TRAP
};

enum Format
{
    FORMAT_R,
    FORMAT_I,
    FORMAT_J,
    INCORRECT
};

struct ISAEntry
{
    const char* name;

    uint8 opcode;
    uint8 func;

    Format format;
    Instructions type;
};

class FuncInstr
{
////////////////////////////////////////////////////////////////////
    union
    {
        struct
        {
            unsigned imm:16;
            unsigned t:5;
            unsigned s:5;
            unsigned opcode:6;
        } asI;

        struct
        {
            unsigned funct:6;
            unsigned shamt:5;
            unsigned d:5;
            unsigned t:5;
            unsigned s:5;
            unsigned opcode:6;
        } asR;

        struct
        {
            unsigned addr:26;
            unsigned opcode:6;
        } asJ;

        uint32 raw;
    } bytes;
/////////////////////////////////////////////////////////////////////////////
    Instructions instr_type;
/////////////////////////////////////////////////////////////////////////////
    Format format;
/////////////////////////////////////////////////////////////////////////////
    Registers dst;
    Registers src;
    Registers trg;
    uint32 shamt; // for R-type
    uint32 constant; // for I type
    uint32 address;  // for J type
/////////////////////////////////////////////////////////////////////////////
    public:

        FuncInstr( uint32 bytes); // constructor

        void initFormat( uint32 bytes); // identifies type of instruction(bytes array)

        void parseR( uint32 bytes); // parses R-type instructions
        void parseI( uint32 bytes); // parses I-type instructions
        void parseJ( uint32 bytes); // parses J-type instructions

        std::string Dump( std::string indent = " ") const;
};
#endif //FUNC_INSTR_H
