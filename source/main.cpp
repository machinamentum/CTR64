
#include <3ds.h>
#include <cstdio>

#define PRIMARY_OP_MASK \
    (0b111111 << 26)

#define SECONDARY_OP_MASK \
    (0b111111 << 0)

#define COMMENT20_MASK \
    (0xFFFFF << 6)

#define IMM5_MASK \
    (0b11111 << 6)

#define IMM16_MASK \
    (0xFFFF << 0)

#define IMM26_MASK \
    (0x3FFFFFF << 0)

#define REG_RT_MASK \
    (0b11111 << 16)

#define REG_RS_MASK \
    (0b11111 << 21)

#define REG_RD_MASK \
    (0b11111 << 11)

#define KUSEG (0x00000000)
#define KSEG0 (0x80000000)
#define KSEG1 (0xA0000000)
#define KSEG2 (0xFFFE0000)
#define KSEGOFF (0x1F000000)

#define RESET_VECTOR (0xBFC00000)
#define UTLBM_VECTOR (0x80000000)
#define C0BRK_VECTOR (0x80000040)
#define GNRAL_VECTOR (0x80000080)

#define C0_PRID  (0x00000002)

#define C0_BVA    8
#define C0_STATUS 12
#define C0_CAUSE  13
#define C0_EPC    14

#define C0_CAUSE_INT     0
#define C0_CAUSE_ADDRL   4
#define C0_CAUSE_ADDRS   5
#define C0_CAUSE_IBUS    6
#define C0_CAUSE_DBUS    7
#define C0_CAUSE_SYSCALL 8
#define C0_CAUSE_BKPT    9
#define C0_CAUSE_RI     10
#define C0_CAUSE_OVF    11

#define C0_CAUSE_MASK  (0b111111 << 10)

#define MIPS_MODE_KERNEL 1
#define MIPS_MODE_USER   0

#define REG_INDEX_RA 31
#define REG_INDEX_PC 32
#define REG_INDEX_HL 33


#define MEM_ACCESS_NONE   0
#define MEM_ACCESS_BRANCH 1
#define MEM_ACCESS_WRITE  2
#define MEM_ACCESS_READ   4

#define MEM_ACCESS_BYTE   1
#define MEM_ACCESS_HALF   2
#define MEM_ACCESS_WORD   4

struct opcode
{
    u32 CurrentAddress = 0;
    u8 Select0;
    u8 Select1;
    u8 MemAccessType = MEM_ACCESS_NONE;
    u8 MemAccessMode = MEM_ACCESS_WORD;

    u32 LeftValue;
    u32 RightValue;
    u32 Immediate;
    u32 Result;

    u8 DestinationRegister = 0;
    u8 RADestinationRegister = 0; // Used for return address writing
};

struct C0Processor
{
    union
    {
        u32 registers[15];
        struct
        {
            u32 pad[7];
            u32 bva;
            u32 status;
            u32 cause;
            u32 epc;
            const u32 prid = C0_PRID;
        };
    };
};

struct MIPS_R3000
{
    union
    {
        u32 registers[34];
        struct
        {
            u32 zero;
            u32 at;

            u32 v0;
            u32 v1;

            u32 a0;
            u32 a1;
            u32 a2;
            u32 a3;
            u32 t0;
            u32 t1;
            u32 t2;
            u32 t3;
            u32 t4;
            u32 t5;
            u32 t6;
            u32 t7;
            u32 s0;
            u32 s1;
            u32 s2;
            u32 s3;
            u32 s4;
            u32 s5;
            u32 s6;
            u32 s7;
            u32 t8;
            u32 t9;
            u32 k0;
            u32 k1;
            u32 gp;
            u32 sp;
            u32 fp;
            u32 ra;
            u32 pc = RESET_VECTOR;
            u16 hi, lo;
        };
    };

    void *Memory = linearAlloc(0x1000000);
    int KMode;
    C0Processor CP0;

};

static void
DumpState(MIPS_R3000 *Cpu, opcode *Op)
{
    printf("\x1b[0;0H");
    for (int i = 0; i < 32; ++i)
    {
        printf("R%02d: 0x%08lX  ", i, Cpu->registers[i]);
        if (i % 2 == 1) printf("\n");
    }
    printf("RA : 0x%08lX\n", Cpu->ra);
    printf("PC : 0x%08lX\n", Cpu->pc);
    printf("Select 0: 0x%02X\n", Op->Select0);
    printf("Select 1: 0x%02X\n", Op->Select1);
}

//Exceptions
static void
C0GenerateException(MIPS_R3000 *Cpu, u8 Cause, u32 EPC)
{
    Cpu->CP0.cause = (Cause << 10) & C0_CAUSE_MASK;
    Cpu->CP0.epc = EPC;
    Cpu->KMode = MIPS_MODE_KERNEL;
    Cpu->pc = GNRAL_VECTOR;
}

static void
ReservedInstructionException(MIPS_R3000 *Cpu, opcode *Op)
{
    // TODO exceptions & coprocessor0
    DumpState(Cpu, Op);
}


typedef void (*jt_func)(MIPS_R3000 *, opcode *);

jt_func PrimaryJumpTable[0x40];
jt_func SecondaryJumpTable[0x40];

inline u32
MemReadWord(MIPS_R3000 *Cpu, u32 Address)
{
    u32 Base = Address & 0x00FFFFFF;
    return *((u32 *)((u8 *)Cpu->Memory + Base));
}

inline void
WriteMemByte(MIPS_R3000 *Cpu, u32 Address, u8 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u8 *)Cpu->Memory + Base) = value;
}

inline void
WriteMemWord(MIPS_R3000 *Cpu, u32 Address, u32 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u32 *)((u8 *)Cpu->Memory + Base)) = value;
}

inline void
WriteMemHalfWord(MIPS_R3000 *Cpu, u32 Address, u16 value)
{
    u32 Base = Address & 0x00FFFFFF;
    *((u16 *)((u8 *)Cpu->Memory + Base)) = value;
}

//Arithmetic
static void
AddU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->RightValue;
    printf("\x1b[0;0H%s", __func__);
}

static void
AddIU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    printf("\x1b[0;0H%s", __func__);
}

static void
SubU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue - OpCode->RightValue;
    printf("\x1b[0;0H%s", __func__);
}

//Store
static void
SW(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_WORD;
    printf("\x1b[0;0H%s", __func__);
}

static void
SH(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue + OpCode->Immediate;
    OpCode->MemAccessMode = MEM_ACCESS_HALF;
    printf("\x1b[0;0H%s", __func__);
}

//Load
static void
LUI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->Immediate << 16;
    printf("\x1b[0;0H%s", __func__);
}


// Jump/Call
static void
J(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = (Cpu->pc & 0xF0000000) + (OpCode->Immediate * 4);
    printf("\x1b[0;0H%s", __func__);
}

static void
JAL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->RADestinationRegister = REG_INDEX_RA;
    OpCode->Result = (Cpu->pc & 0xF0000000) + (OpCode->Immediate * 4);
    printf("\x1b[0;0H%s", __func__);
}

static void
JR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue;
    printf("\x1b[0;0H%s", __func__);
}

static void
JALR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue;
    printf("\x1b[0;0H%s", __func__);
}

static void
BranchZero(MIPS_R3000 *Cpu, opcode *OpCode)
{
    //bltz, bgez, bltzal, bgezal
    u8 type = OpCode->RightValue;
    OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
    u32 Check = OpCode->LeftValue;

    if (type & 0b00001)
    {
        //bgez
        if (Check >= 0)
        {
            if (type & 0b10000)
            {
                OpCode->RADestinationRegister = REG_INDEX_RA;
            }
            OpCode->DestinationRegister = REG_INDEX_PC;
        }
    }
    else
    {
        //bltz
        if (Check < 0)
        {
            if (type & 0b10000)
            {
                OpCode->RADestinationRegister = REG_INDEX_RA;
            }
            OpCode->DestinationRegister = REG_INDEX_PC;
        }
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BEQ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue == OpCode->RightValue)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BNE(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue != OpCode->RightValue)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BLEZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue <= 0)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BGTZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->LeftValue > 0)
    {
        OpCode->Result = OpCode->CurrentAddress + 4 + OpCode->Immediate * 4;
        OpCode->DestinationRegister = REG_INDEX_PC;
    }
    printf("\x1b[0;0H%s", __func__);
}

//Logical
static void
AndI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue & OpCode->Immediate;
    printf("\x1b[0;0H%s", __func__);
}

static void
OrI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    OpCode->Result = OpCode->LeftValue | OpCode->Immediate;
    printf("\x1b[0;0H%s", __func__);
}

static void
DecodeOpcode(MIPS_R3000 *Cpu, opcode *OpCode, u32 Data, u32 IAddress)
{
    OpCode->CurrentAddress = IAddress;
    OpCode->Select0 = (Data & PRIMARY_OP_MASK) >> 26;
    OpCode->Select1 = (Data & SECONDARY_OP_MASK) >> 0;
    u8 rs = (Data & REG_RS_MASK) >> 21;
    u8 rt = (Data & REG_RT_MASK) >> 16;
    u8 rd = (Data & REG_RD_MASK) >> 11;


//    OpCode->comment = (Data & COMMENT20_MASK) >> 6;
//    OpCode->imm5 = (Data & IMM5_MASK) >> 6;
//    OpCode->imm16 = (Data & IMM16_MASK) >> 0;
//    OpCode->imm26 = (Data & IMM26_MASK) >> 0;

    if (OpCode->Select0 == 0)
    {
        //shift-imm
        if ((OpCode->Select1 & 0b111100) == 0)
        {
            OpCode->Immediate = (Data & IMM5_MASK) >> 6;
            OpCode->LeftValue = Cpu->registers[rt];
            OpCode->DestinationRegister = rd;
        }
        //shift-reg
        else if ((OpCode->Select1 & 0b111000) == 0)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->RightValue = Cpu->registers[rt];
            OpCode->DestinationRegister = rd;
        }
        //jr
        else if (OpCode->Select1 == 0b001000)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->DestinationRegister = REG_INDEX_PC;
            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        }
        //jalr
        else if (OpCode->Select1 == 0b001001)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->DestinationRegister = REG_INDEX_PC;
            OpCode->RADestinationRegister = rd;
            OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        }
        //sys/brk
        else if ((OpCode->Select1 & 0b111110) == 0b001100)
        {
            OpCode->Immediate = (Data & COMMENT20_MASK) >> 6;
        }
        //mfhi/mflo
        else if ((OpCode->Select1 & 0b111101) == 0b010000)
        {
            OpCode->DestinationRegister = rd;
        }
        //mthi/mtlo
        else if ((OpCode->Select1 & 0b111101) == 0b010001)
        {
            OpCode->DestinationRegister = REG_INDEX_HL;
            OpCode->LeftValue = Cpu->registers[rs];
        }
        //mul/div
        else if ((OpCode->Select1 & 0b111100) == 0b011000)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->RightValue = Cpu->registers[rt];
            OpCode->DestinationRegister = REG_INDEX_HL;
        }
        //alu-reg
        else if (OpCode->Select1 & 0b100000)
        {
            OpCode->LeftValue = Cpu->registers[rs];
            OpCode->RightValue = Cpu->registers[rt];
            OpCode->DestinationRegister = rd;
        }

    }
    //bltz, bgez, bltzal bgezal
    else if (OpCode->Select0 == 0b000001)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->RightValue = rt; //rt is used as a function selector here
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        //destination registers set within function
    }
    //j/jal
    else if ((OpCode->Select0 & 0b111110) == 0b000010)
    {
        OpCode->Immediate = (Data & IMM26_MASK) >> 0;
        OpCode->DestinationRegister = REG_INDEX_PC;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        //RADestinationRegister set within function
    }
    //beq/bne
    else if ((OpCode->Select0 & 0b111110) == 0b0000100)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->RightValue = Cpu->registers[rt];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        //destination registers set within function
    }
    //blez/bgtz
    else if ((OpCode->Select0 & 0b111110) == 0b000110)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_BRANCH;
        //destination registers set within function
    }
    //alu-imm
    else if ((OpCode->Select0 & 0b111000) == 0b001000)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->DestinationRegister = rt;
    }
    //lui-imm
    else if (OpCode->Select0 == 0b001111)
    {
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->DestinationRegister = rt;
    }
    //load
    else if ((OpCode->Select0 & 0b111000) == 0b100000)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->DestinationRegister = rt;
        OpCode->MemAccessType = MEM_ACCESS_READ;
    }
    //store
    else if ((OpCode->Select0 & 0b111000) == 0b101000)
    {
        OpCode->LeftValue = Cpu->registers[rs];
        OpCode->RightValue = Cpu->registers[rt];
        OpCode->Immediate = (Data & IMM16_MASK) >> 0;
        OpCode->MemAccessType = MEM_ACCESS_WRITE;
    }
    // TODO C0P instruction decoding
}

static void
InstructionFetch(MIPS_R3000 *Cpu, u32 *Code)
{
    *Code = MemReadWord(Cpu, Cpu->pc += 4);
}

static void
ExecuteSecondary(MIPS_R3000 *Cpu, opcode *Op)
{
    SecondaryJumpTable[Op->Select1](Cpu, Op);
}

static void
ExecuteOpCode(MIPS_R3000 *Cpu, opcode *OpCode)
{
    PrimaryJumpTable[OpCode->Select0](Cpu, OpCode);
}

static void
MemoryAccess(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->MemAccessType & MEM_ACCESS_BRANCH)
    {
        Cpu->pc = OpCode->Result;
        OpCode->DestinationRegister = 0;
    }
    if (OpCode->MemAccessType & MEM_ACCESS_WRITE)
    {
        if (OpCode->MemAccessMode & MEM_ACCESS_BYTE)
        {
            WriteMemByte(Cpu, OpCode->Result, OpCode->RightValue);
        }

        if (OpCode->MemAccessMode & MEM_ACCESS_HALF)
        {
            WriteMemHalfWord(Cpu, OpCode->Result, OpCode->RightValue);
        }

        if (OpCode->MemAccessMode & MEM_ACCESS_BYTE)
        {
            WriteMemWord(Cpu, OpCode->Result, OpCode->RightValue);
        }
        OpCode->DestinationRegister = 0;
    }
    if (OpCode->MemAccessType & MEM_ACCESS_READ)
    {
        // TODO memory reads
    }
}

static void
WriteBack(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (OpCode->DestinationRegister) // Never overwrite Zero
    {
        Cpu->registers[OpCode->DestinationRegister] = OpCode->Result;
    }
    if (OpCode->RADestinationRegister)
    {
        Cpu->registers[OpCode->RADestinationRegister] = OpCode->CurrentAddress + 8;
    }
}

void
InitJumpTables()
{
    for (int i = 0; i < 0x3F; ++i)
    {
        PrimaryJumpTable[i] = ReservedInstructionException;
        SecondaryJumpTable[i] = ReservedInstructionException;
    }

    PrimaryJumpTable[0x00] = ExecuteSecondary;
    PrimaryJumpTable[0x01] = BranchZero;
    PrimaryJumpTable[0x02] = J;
    PrimaryJumpTable[0x03] = JAL;
    PrimaryJumpTable[0x04] = BEQ;
    PrimaryJumpTable[0x05] = BNE;
    PrimaryJumpTable[0x06] = BLEZ;
    PrimaryJumpTable[0x07] = BGTZ;
//    PrimaryJumpTable[0x08] = AddI;
    PrimaryJumpTable[0x09] = AddIU;
//    PrimaryJumpTable[0x0A] = SLTI;
//    PrimaryJumpTable[0x0B] = SLTIU;
    PrimaryJumpTable[0x0C] = AndI;
    PrimaryJumpTable[0x0D] = OrI;
    PrimaryJumpTable[0x0F] = LUI;
//    PrimaryJumpTable[0x10] = COP0;
//    PrimaryJumpTable[0x11] = COP1;
//    PrimaryJumpTable[0x12] = COP2;
//    PrimaryJumpTable[0x13] = COP3;
//    PrimaryJumpTable[0x20] = LB;
//    PrimaryJumpTable[0x21] = LH;
//    PrimaryJumpTable[0x22] = LWL;
//    PrimaryJumpTable[0x23] = LW;
//    PrimaryJumpTable[0x24] = LBU;
//    PrimaryJumpTable[0x25] = LHU;
//    PrimaryJumpTable[0x26] = LWR;
//    PrimaryJumpTable[0x28] = SB;
    PrimaryJumpTable[0x29] = SH;
//    PrimaryJumpTable[0x2A] = SWL;
    PrimaryJumpTable[0x2B] = SW;
//    PrimaryJumpTable[0x2E] = SWR;
//    PrimaryJumpTable[0x30] = LWC0;
//    PrimaryJumpTable[0x31] = LWC1;
//    PrimaryJumpTable[0x32] = LWC2;
//    PrimaryJumpTable[0x33] = LWC3;
//    PrimaryJumpTable[0x38] = SWC0;
//    PrimaryJumpTable[0x39] = SWC1;
//    PrimaryJumpTable[0x3A] = SWC2;
//    PrimaryJumpTable[0x3B] = SWC3;

//    SecondaryJumpTable[0x00] = SLL;
//    SecondaryJumpTable[0x02] = SRL;
//    SecondaryJumpTable[0x03] = SRA;
//    SecondaryJumpTable[0x04] = SLLV;
//    SecondaryJumpTable[0x06] = SRLV;
//    SecondaryJumpTable[0x07] = SRAV;
    SecondaryJumpTable[0x08] = JR;
    SecondaryJumpTable[0x09] = JALR;
//    SecondaryJumpTable[0x0C] = SysCall;
//    SecondaryJumpTable[0x0D] = Break;
//    SecondaryJumpTable[0x10] = MFHI;
//    SecondaryJumpTable[0x11] = MTHI;
//    SecondaryJumpTable[0x12] = MFLO;
//    SecondaryJumpTable[0x13] = MTLO;
//    SecondaryJumpTable[0x18] = Mutl;
//    SecondaryJumpTable[0x19] = MultU;
//    SecondaryJumpTable[0x1A] = Div;
//    SecondaryJumpTable[0x1B] = DivU;
//    SecondaryJumpTable[0x20] = Add;
    SecondaryJumpTable[0x21] = AddU;
//    SecondaryJumpTable[0x22] = Sub;
    SecondaryJumpTable[0x23] = SubU;
//    SecondaryJumpTable[0x24] = And;
//    SecondaryJumpTable[0x25] = Or;
//    SecondaryJumpTable[0x26] = XOr;
//    SecondaryJumpTable[0x27] = NOr;
//    SecondaryJumpTable[0x2A] = SLT;
//    SecondaryJumpTable[0x2B] = SLTU;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define STAGE_IF 1
#define STAGE_DC 2
#define STAGE_EO 4
#define STAGE_MA 8
#define STAGE_WB 16

int main(int argc, char **argv)
{

    if (argc) chdir(argv[0]);

    gfxInitDefault();
    hidInit(NULL);
    consoleInit(GFX_BOTTOM, NULL);
    InitJumpTables();

    MIPS_R3000 Cpu;

    FILE *f = fopen("psx_bios.bin", "rb");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    u8 *BiosBuffer = (u8 *)linearAlloc(fsize + 1);
    fread(BiosBuffer, fsize, 1, f);
    fclose(f);


    for (int i = 0; i < fsize; ++i)
    {
        WriteMemByte(&Cpu, RESET_VECTOR + i, BiosBuffer[i]);
    }

    opcode OpCodes[5];
    int Stages[5];

    for (int i = 4; i >= 0; --i)
    {
        Stages[i] = -i;
    }

    u32 MachineCode = 0;

    while (aptMainLoop())
    {
        hidScanInput();

        if (keysDown() & KEY_START)
            break;

        for (int i  = 0; i < 5; ++i)
        {
            if (Stages[i] < 1)
            {
                ++Stages[i];
            }

            if (Stages[i] == STAGE_IF)
            {
                InstructionFetch(&Cpu, &MachineCode);
                Stages[i] = STAGE_DC;
                continue;
            }
            if (Stages[i] == STAGE_DC)
            {
                DecodeOpcode(&Cpu, &OpCodes[i], MachineCode, Cpu.pc - 4);
                Stages[i] = STAGE_EO;
                continue;
            }
            if (Stages[i] == STAGE_EO)
            {
                ExecuteOpCode(&Cpu, &OpCodes[i]);
                Stages[i] = STAGE_MA;
                continue;
            }
            if (Stages[i] == STAGE_MA)
            {
                MemoryAccess(&Cpu, &OpCodes[i]);
                Stages[i] = STAGE_WB;
                continue;
            }
            if (Stages[i] == STAGE_WB)
            {
                WriteBack(&Cpu, &OpCodes[i]);
                Stages[i] = STAGE_IF;
                continue;
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffersGpu();
        gspWaitForVBlank();
    }
    
    // Exit services
    gfxExit();
    hidExit();
    return 0;
}



