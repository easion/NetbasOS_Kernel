
#include <jicama/process.h>

#include "./s3c4510/s3c4510.h"


/**
 * The software interrupt instruction (SWI) is used for entering 
 * Supervisor mode, usually to request a particular supervisor 
 * function.
 */
void arm_trap_swi(struct arm_register *regs)
{
	//arm下的系统调用
    kprintf("software interrupt\n");
    //while (1);   
}


