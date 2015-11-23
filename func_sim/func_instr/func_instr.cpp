#include <func_instr.h>
//#include <iostream>
//#include <ostream>
//#include <cassert>
//#include <sstream>

// Genereic C
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <fcntl.h>
#include <gelf.h>
#include <cstdlib>
#include <cerrno>
#include <cassert>

// Generic C++
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

static const ISAEntry isaTable[] =
    {
        // name   opcode    func      format              type
        { "add",     0x0,     0x20, FORMAT_R, ADD /*...*/  },//0
        { "addu",    0x0,     0x21, FORMAT_R, ADDU /*...*/ },//1
        { "sub",     0x0,     0x22, FORMAT_R, SUB /*...*/  },//2
        { "subu",    0x0,     0x23, FORMAT_R, SUBU /*...*/ },//3
        { "addi",    0x8,     0x0,  FORMAT_I, ADDI /*...*/ },//4
        { "addiu",   0x9,     0x0,  FORMAT_I, ADDIU /*...*/},//5
        { "mult",    0x0,     0x18, FORMAT_R, MULT /*...*/ },//6
        { "multu",   0x0,     0x19, FORMAT_R, MULTU /*...*/},//7
        { "div",     0x0,     0x1A, FORMAT_R, DIV /*...*/  },//8
        { "divu",    0x0,     0x1B, FORMAT_R, DIVU /*...*/ },//9
        { "mfhi",    0x0,     0x10, FORMAT_R, MFHI /*...*/ },//10
        { "mthi",    0x0,     0x11, FORMAT_R, MTHI /*...*/ },//11
        { "mflo",    0x0,     0x12, FORMAT_R, MFLO /*...*/ },//12
        { "mtlo",    0x0,     0x13, FORMAT_R, MTLO /*...*/ },//13
        { "sll",     0x0,     0x0,  FORMAT_R, SLL /*...*/  },//14
        { "srl",     0x0,     0x2,  FORMAT_R, SRL /*...*/  },//15
        { "sra",     0x0,     0x3,  FORMAT_R, SRA /*...*/  },//16
        { "sllv",    0x0,     0x4,  FORMAT_R, SLLV /*...*/ },//17
        { "srlv",    0x0,     0x6,  FORMAT_R, SRLV /*...*/ },//18
        { "srav",    0x0,     0x7,  FORMAT_R, SRAV /*...*/ },//19
        { "lui",     0xF,     0x0,  FORMAT_I, LUI /*...*/  },//20
        { "slt",     0x0,     0x2A, FORMAT_R, SLT /*...*/  },//21
        { "sltu",    0x0,     0x2B, FORMAT_R, SLTU /*...*/ },//22
        { "slti",    0xA,     0x0,  FORMAT_I, SLTI /*...*/ },//23
        { "sltiu",   0xB,     0x0,  FORMAT_I, SLTIU/*...*/ },//24
        { "and",     0x0,     0x24, FORMAT_R, AND /*...*/  },//25
        { "or",      0x0,     0x25, FORMAT_R, OR /*...*/   },//26
        { "xor",     0x0,     0x26, FORMAT_R, XOR /*...*/  },//27
        { "nor",     0x0,     0x27, FORMAT_R, NOR /*...*/  },//28
        { "andi",    0xC,     0x0,  FORMAT_I, ANDI /*...*/ },//29
        { "ori",     0xD,     0x0,  FORMAT_I, ORI /*...*/  },//30
        { "xori",    0xE,     0x0,  FORMAT_I, XORI /*...*/ },//31
        { "beq",     0x4,     0x0,  FORMAT_I, BEQ /*...*/  },//32
        { "bne",     0x5,     0x0,  FORMAT_I, BNE /*...*/  },//33
        { "blez",    0x6,     0x0,  FORMAT_I, BLEZ /*...*/ },//34
        { "bgtz",    0x7,     0x0,  FORMAT_I, BGTZ /*...*/ },//35
        { "j",       0x2,     0x0,  FORMAT_J, J /*...*/    },//36
        { "jal",     0x3,     0x0,  FORMAT_J, JAL /*...*/  },//37
        { "jr",      0x0,     0x8,  FORMAT_R, JR /*...*/   },//38
        { "jalr",    0x0,     0x9,  FORMAT_R, JALR /*...*/ },//39
        { "lb",      0x20,    0x0,  FORMAT_I, LB /*...*/   },//40
        { "lh",      0x21,    0x0,  FORMAT_I, LH /*...*/   },//41
        { "lw",      0x23,    0x0,  FORMAT_I, LW /*...*/   },//42
        { "lbu",     0x24,    0x0,  FORMAT_I, LBU /*...*/  },//43
        { "lhu",     0x25,    0x0,  FORMAT_I, LHU /*...*/  },//44
        { "sb",      0x28,    0x0,  FORMAT_I, SB /*...*/   },//45
        { "sh",      0x29,    0x0,  FORMAT_I, SH /*...*/   },//46
        { "sw",      0x2B,    0x0,  FORMAT_I, SW /*...*/   },//47
        { "syscall", 0x0,     0xC,  FORMAT_R, SYSCALL /*...*/ },//48
        { "break",   0x0,     0xD,  FORMAT_R, BREAK /*...*/   },//49
        { "trap",    0x1A,    0x0,  FORMAT_J, TRAP /*...*/    },//50
    };

ostream& operator<<( ostream& out, Registers reg)
{
    switch (reg)
    {
        case ZERO: out << "$zero"; break;
        case AT  : out << "$at"; break;
        case V0  : out << "$v0"; break;
        case V1  : out << "$v1"; break;
        case A0  : out << "$a0"; break;
        case A1  : out << "$a1"; break;
        case A2  : out << "$a2"; break;
        case A3  : out << "$a3"; break;
        case T0  : out << "$t0"; break;
        case T1  : out << "$t1"; break;
        case T2  : out << "$t2"; break;
        case T3  : out << "$t3"; break;
        case T4  : out << "$t4"; break;
        case T5  : out << "$t5"; break;
        case T6  : out << "$t6"; break;
        case T7  : out << "$t7"; break;
        case S0  : out << "$s0"; break;
        case S1  : out << "$s1"; break;
        case S2  : out << "$s2"; break;
        case S3  : out << "$s3"; break;
        case S4  : out << "$s4"; break;
        case S5  : out << "$s5"; break;
        case S6  : out << "$s6"; break;
        case S7  : out << "$s7"; break;
        case T8  : out << "$t8"; break;
        case T9  : out << "$t9"; break;
        case K0  : out << "$k0"; break;
        case K1  : out << "$k1"; break;
        case GP  : out << "$gp"; break;
        case SP  : out << "$sp"; break;
        case S8  : out << "$s8"; break;
        case RA  : out << "$ra"; break;
    }
}

FuncInstr::FuncInstr( uint32 bytes)
{
    this->initFormat( bytes);

    switch ( this->format)
    {
        case FORMAT_R:

            this->parseR( bytes);
            break;

        case FORMAT_I:

            this->parseI( bytes);
            break;

        case FORMAT_J:

            this->parseJ( bytes);
            break;

        default:
            cerr << "ERROR_WRONG_FORMAT";
            exit(EXIT_FAILURE);
    }
};

void FuncInstr::initFormat( uint32 bytes)
{
    this->bytes.raw = bytes;

    if ( this->bytes.asR.opcode == 0)
    {
        this->format = FORMAT_R;
        return;
    }

    if ( ( this->bytes.asR.opcode == 0x2) || // != j
         ( this->bytes.asR.opcode == 0x3) || // != jal
         ( this->bytes.asR.opcode == 0x1A))  // != trap
    {
        this->format = FORMAT_J;
        return;
    }
    this->format = FORMAT_I;
}

void FuncInstr::parseR( uint32 bytes)
{
    this->bytes.raw = bytes;
    this->shamt = this->bytes.asR.shamt;
    this->dst = ( Registers) this->bytes.asR.d;
    this->src = ( Registers) this->bytes.asR.s;
    this->trg = ( Registers) this->bytes.asR.t;

    for ( int i = 0; i < 51; ++i)
    {
        if (this->bytes.asR.funct == isaTable[i].func && isaTable[i].format == FORMAT_R)
        {
            this->instr_type = isaTable[i].type;
            return;
        }
    }
    cerr << "ERROR.NO_SUCH_OPCODE";
    exit(EXIT_FAILURE);
}

void FuncInstr::parseI( uint32 bytes)
{
    this->bytes.raw = bytes;
    this->src = ( Registers) this->bytes.asI.s;
    this->trg = ( Registers) this->bytes.asI.t;
    this->constant = ( uint32) this->bytes.asI.imm;

    for ( int i = 0; i < 51; ++i)
    {
        if (this->bytes.asI.opcode == isaTable[i].opcode && isaTable[i].format == FORMAT_I)
        {
            this->instr_type = isaTable[i].type;
            return;
        }
    }
    cerr << "ERROR.NO_SUCH_OPCODE";
    exit(EXIT_FAILURE);
}

void FuncInstr::parseJ( uint32 bytes)
{
    this->bytes.raw = bytes;
    this->address = ( uint32) this->bytes.asJ.addr;

    int i = 0;

    for ( i = 0; i < 51; ++i)
    {
        if (this->bytes.asJ.opcode == isaTable[i].opcode && isaTable[i].format == FORMAT_J)
        {
            this->instr_type = isaTable[i].type;
            return;
        }
    }
    perror("ERROR.NO_SUCH_OPCODE");
    exit(EXIT_FAILURE);
}

string FuncInstr::Dump( string indent) const
{
    ostringstream oss;

    switch ( this->instr_type)
    {
        case ADD:  // name $d, $s, $t
        case ADDU:
        case SUB:
        case SUBU:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->dst << ", " << this->src << ", " << this->trg;

            break;

        case ADDI: // name $t, $s, C
        case ADDIU:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->trg << ", " << this->src << ", 0x" << hex << this->constant;

            break;

        case MULT: // name $s, $t
        case MULTU:
        case DIV:
        case DIVU:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->src << ", " << this->trg;

            break;

        case MFHI: // name $d
        case MFLO:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->dst;

            break;

        case MTHI: // name $s
        case MTLO:
        case JR:
        case JALR:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->src;


            break;

        case SLL: // name $d, $t, S
        case SRL:
        case SRA:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->dst << ", " << this->trg << ", 0x" << this->shamt;

            break;

        case SLLV: // name $d, $t, $s
        case SRLV:
        case SRAV:
        case SLT:
        case SLTU:
        case AND:
        case OR:
        case XOR:
        case NOR:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->dst << ", " << this->trg << ", " << this->src;

            break;

        case LUI:  // name $t, C

            oss << indent << isaTable[this->instr_type].name << " "
                << this->trg << ", 0x" << this->constant;

            break;

        case SLTI: // name $s, $t, C
        case SLTIU:
        case ANDI:
        case ORI:
        case XORI:
        case BEQ:
        case BNE:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->src << ", " << this->trg << ", 0x" << hex << this->constant;

            break;

        case BLEZ: // name $s, C
        case BGTZ:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->src << ", 0x" << hex << this->constant;

            break;

        case J:   // name A
        case JAL:
        case TRAP:

            oss << indent << isaTable[this->instr_type].name << " 0x"
                << hex << this->address;

            break;

        case SYSCALL: // name
        case BREAK:

            oss << indent << isaTable[this->instr_type].name;

            break;

        case LB: // name $t, C($s)
        case LH:
        case LW:
        case LBU:
        case LHU:
        case SB:
        case SH:
        case SW:

            oss << indent << isaTable[this->instr_type].name << " "
                << this->trg << ", 0x" << hex << this->constant
                << "(" << this->src << ")";

            break;

        default:
            exit(EXIT_FAILURE);
    }

    return oss.str();
}

ostream& operator<<( ostream& out, const FuncInstr& instr)
{
    out << instr.Dump("");
}
