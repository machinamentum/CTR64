
#include "gpu.h"

static void
GteExecuteOperation(Coprocessor *Cp, u32 FunctionCode)
{

}

GPU::
GPU()
{
    ExecuteOperation = GteExecuteOperation;
}

void
GpuGp0(void *Object, u32 Value)
{
    GPU *Gpu = (GPU *)Object;
    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;
}

void
GpuGp1(void *Object, u32 Value)
{
    GPU *Gpu = (GPU *)Object;
    u32 Param = Value & 0x00FFFFFF;
    u32 Command = Value >> 24;

    if (Command == GP1_COMMAND_DISP_EN)
    {
        
    }
}
