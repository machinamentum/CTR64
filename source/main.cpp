
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

struct opcode
{
    u8 select0;
    u8 select1;
    u8 rs;
    u8 rt;
    u8 rd;
    u8 imm5;
    u16 imm16;
    u32 imm26; // 25 bit params too
    u32 comment;
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

            u16 hi;
            u16 lo;
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
    printf("Select 0: 0x%02X\n", Op->select0);
    printf("Select 1: 0x%02X\n", Op->select1);
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
    Cpu->registers[OpCode->rd] = Cpu->registers[OpCode->rs] + Cpu->registers[OpCode->rt];
    printf("\x1b[0;0H%s", __func__);
}

static void
AddIU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->registers[OpCode->rt] = Cpu->registers[OpCode->rs] + (s16)OpCode->imm16;
    printf("\x1b[0;0H%s", __func__);
}

static void
SubU(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->registers[OpCode->rd] = Cpu->registers[OpCode->rs] - Cpu->registers[OpCode->rt];
    printf("\x1b[0;0H%s", __func__);
}

//Store
static void
SW(MIPS_R3000 *Cpu, opcode *OpCode)
{
    WriteMemWord(Cpu, Cpu->registers[OpCode->rs] + OpCode->imm16, Cpu->registers[OpCode->rt]);
    printf("\x1b[0;0H%s", __func__);
}

static void
SH(MIPS_R3000 *Cpu, opcode *OpCode)
{
    WriteMemHalfWord(Cpu, Cpu->registers[OpCode->rs] + OpCode->imm16, Cpu->registers[OpCode->rt]);
}

//Load
static void
LUI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->registers[OpCode->rt] = OpCode->imm16 << 16;
    printf("\x1b[0;0H%s", __func__);
}


// Jump/Call
static void
J(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->pc = (Cpu->pc & 0xF0000000) + (OpCode->imm26 * 4);
    printf("\x1b[0;0H%s", __func__);
}

static void
JAL(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->ra = Cpu->pc + 8;
    Cpu->pc = (Cpu->pc & 0xF0000000) + (OpCode->imm26 * 4);
    printf("\x1b[0;0H%s", __func__);
}

static void
JR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->pc = Cpu->registers[OpCode->rs];
    printf("\x1b[0;0H%s", __func__);
}

static void
JALR(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->registers[OpCode->rd] = Cpu->pc + 8;
    Cpu->pc = Cpu->registers[OpCode->rs];
    printf("\x1b[0;0H%s", __func__);
}

static void
BranchZero(MIPS_R3000 *Cpu, opcode *OpCode)
{
    //bltz, bgez, bltzal, bgezal
    u8 type = OpCode->rt;
    u32 Address = Cpu->pc + 4 + (s16)OpCode->imm16 * 4;
    u32 Check = Cpu->registers[OpCode->rs];

    if (type & 0b00001)
    {
        //bgez
        if (Check >= 0)
        {
            if (type & 0b10000)
            {
                Cpu->ra = Cpu->pc + 8;
            }
            Cpu->pc = Address;
        }
    }
    else
    {
        //bltz
        if (Check < 0)
        {
            if (type & 0b10000)
            {
                Cpu->ra = Cpu->pc + 8;
            }
            Cpu->pc = Address;
        }
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BEQ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (Cpu->registers[OpCode->rs] == Cpu->registers[OpCode->rt])
    {
        Cpu->pc = Cpu->pc + 4 + (s16)OpCode->imm16 * 4;
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BNE(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (Cpu->registers[OpCode->rs] != Cpu->registers[OpCode->rt])
    {
        Cpu->pc = Cpu->pc + 4 + (s16)OpCode->imm16 * 4;
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BLEZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (Cpu->registers[OpCode->rs] <= 0)
    {
        Cpu->pc = Cpu->pc + 4 + (s16)OpCode->imm16 * 4;
    }
    printf("\x1b[0;0H%s", __func__);
}

static void
BGTZ(MIPS_R3000 *Cpu, opcode *OpCode)
{
    if (Cpu->registers[OpCode->rs] > 0)
    {
        Cpu->pc = Cpu->pc + 4 + (s16)OpCode->imm16 * 4;
    }
    printf("\x1b[0;0H%s", __func__);
}

//Logical
static void
AndI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->registers[OpCode->rt] = Cpu->registers[OpCode->rs] & OpCode->imm16;
    printf("\x1b[0;0H%s", __func__);
}

static void
OrI(MIPS_R3000 *Cpu, opcode *OpCode)
{
    Cpu->registers[OpCode->rt] = Cpu->registers[OpCode->rs] | OpCode->imm16;
    printf("\x1b[0;0H%s", __func__);
}

static void
DecodeOpcode(opcode *OpCode, u32 Data)
{
    OpCode->select0 = (Data & PRIMARY_OP_MASK) >> 26;
    OpCode->select1 = (Data & SECONDARY_OP_MASK) >> 0;
    OpCode->rs = (Data & REG_RS_MASK) >> 21;
    OpCode->rt = (Data & REG_RT_MASK) >> 16;
    OpCode->rd = (Data & REG_RD_MASK) >> 11;

    OpCode->comment = (Data & COMMENT20_MASK) >> 6;
    OpCode->imm5 = (Data & IMM5_MASK) >> 6;
    OpCode->imm16 = (Data & IMM16_MASK) >> 0;
    OpCode->imm26 = (Data & IMM26_MASK) >> 0;
}

static void
InstructionFetch(MIPS_R3000 *Cpu, opcode *OpCode)
{
    u32 OC = MemReadWord(Cpu, Cpu->pc);
    DecodeOpcode(OpCode, OC);
}

static void
ExecuteSecondary(MIPS_R3000 *Cpu, opcode *Op)
{
    SecondaryJumpTable[Op->select1](Cpu, Op);
}

static void
ExecuteOpCode(MIPS_R3000 *Cpu, opcode *OpCode)
{
    PrimaryJumpTable[OpCode->select0](Cpu, OpCode);
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

    while (aptMainLoop())
    {
        hidScanInput();

        if (keysDown() & KEY_START)
            break;

        opcode OpCode;
        InstructionFetch(&Cpu, &OpCode);
        ExecuteOpCode(&Cpu, &OpCode);
        Cpu.pc += 4;

        gfxFlushBuffers();
        gfxSwapBuffersGpu();
        gspWaitForVBlank();
    }
    
    // Exit services
    gfxExit();
    hidExit();
    return 0;
}



