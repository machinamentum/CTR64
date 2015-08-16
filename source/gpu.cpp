
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
