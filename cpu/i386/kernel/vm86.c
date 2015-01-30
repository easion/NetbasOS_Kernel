#include <jicama/system.h>
#include <jicama/process.h>
#include <arch/x86/io.h>
#include <string.h>
#include <arch/x86/keyboard.h>
#include <arch/x86/traps.h>
#include <signal.h>

#define DEBUGOUT //kprintf

#include "vm86.h"

__asmlink struct tss_struct *vm86_get_tss(void);
__asmlink regs_t *vmback_regs;
__local void *vm86_task_buffer;
__local regs_t vm_regs_data;


 void vm86_context_load(u16_t c);

__local unsigned long to_linear(unsigned seg, unsigned off)
{
	return (seg & 0xFFFF) * 16L + off;
}

unsigned vm86_peekb(unsigned seg, unsigned off)
{
	return *(u8_t *)(to_linear(seg, off) );
}

unsigned vm86_peekw(unsigned seg, unsigned off)
{
	return *(u16_t *)(to_linear(seg, off) );
}

void vm86_pokew(unsigned seg, unsigned off, unsigned val)
{
	*(u16_t *)(to_linear(seg, off) ) = val;
}

__local unsigned long vm86_peekl(unsigned seg, unsigned off)
{
	return *(u32_t *)(to_linear(seg, off) );
}

__local void v86_pokel(unsigned seg, unsigned off, unsigned long val)
{
	*(u32_t *)(to_linear(seg, off) ) = val;
}

void v86_push16(regs_t *regs, unsigned value)
{
	regs->user_esp = (regs->user_esp - 2) & 0xFFFF;
	vm86_pokew(regs->user_ss, regs->user_esp, value);
}

__local unsigned v86_pop16(regs_t *regs)
{
	unsigned rv;

	rv = vm86_peekw(regs->user_ss, regs->user_esp);
	regs->user_esp = (regs->user_esp + 2) & 0xFFFF;
	return rv;
}

__local void v86_push32(regs_t *regs, unsigned long value)
{
	regs->user_esp = (regs->user_esp - 4) & 0xFFFF;
	v86_pokel(regs->user_ss, regs->user_esp, value);
}

__local unsigned long v86_pop32(regs_t *regs)
{
	unsigned long rv;

	rv = vm86_peekl(regs->user_ss, regs->user_esp);
	regs->user_esp = (regs->user_esp + 4) & 0xFFFF;
	return rv;
}

__local unsigned v86_fetch8(regs_t *regs)
{
	unsigned byte;

	byte = vm86_peekb(regs->cs, regs->eip);
	regs->eip = (regs->eip + 1) & 0xFFFF;
	return byte;
}

__local void v86_int(regs_t *regs, unsigned int_num)
{
	/* push return IP, CS, and FLAGS onto V86 mode stack */
	v86_push16(regs, regs->eflags);
	v86_push16(regs, regs->cs);
	v86_push16(regs, regs->eip);

	/* disable interrupts */
	regs->eflags &= ~0x200;

	/* load new CS and IP from IVT */
	int_num *= 4;
	regs->eip = (regs->eip & ~0xFFFF) | vm86_peekw(0, int_num + 0);
	regs->cs = vm86_peekw(0, int_num + 2);
}

void v86_enable_port(unsigned port);


__noreturn void return32bit()
{
	struct tss_struct *tss;	

  	if ((vm_regs_data.eflags & EFLAGS_VM) == EFLAGS_VM)
	{
		u16_t c = get_TR();
		tss = vm86_get_tss();


		if (c != VM86_TASK_TSS_SEL) {
			panic("return32bit() bad TSS Sector\n");
		}
		vmback_regs =&vm_regs_data;
		//kprintf("return32bit() by 0x%x.. eax=0x%x,%x\n", vm_regs_data.eflags,vm_regs_data.eax_org,vm_regs_data.eax);
		vm86_context_load(tss->backlink);
	}
	panic("return32bit(): foo, xxx?\n");
}

int v86_monitor(regs_t *regs)
{
	unsigned init_eip, prefix, i;

	init_eip = regs->eip;

	/* consume prefix bytes */
	prefix = 0;
	while(1)
	{
		i = v86_fetch8(regs);
		//kprintf("i=%x ..\n", i);
		switch(i)
		{
		case 0x26:	prefix |= PFX_ES;	break;
		case 0x2E:	prefix |= PFX_CS;	break;
		case 0x36:	prefix |= PFX_SS;	break;
		case 0x3E:	prefix |= PFX_DS;	break;
		case 0x64:	prefix |= PFX_FS;	break;
		case 0x65:	prefix |= PFX_GS;	break;
		case 0x66:prefix |= PFX_OP32;	break;
		case 0x67:	prefix |= PFX_ADR32;	break;
		case 0xF0:	prefix |= PFX_LOCK;break;
		case 0xF2:	prefix |= PFX_REPNE;	break;
		case 0xF3:	prefix |= PFX_REP;	break;
		default:goto END;
		}
	}

END:
	switch(i)
	{
	case 0x9C:/* PUSHF */
		if(prefix & PFX_OP32)
			v86_push32(regs, regs->eflags);
		else
			v86_push16(regs, regs->eflags);
		DEBUGOUT("[PUSHF]",i);
		return 0;
	case 0x9D:/* POPF */
		if(prefix & PFX_OP32)
		{
			if(regs->user_esp > 0xFFFC)
				return 1;
			regs->eflags = v86_pop32(regs);
		}
		else
		{
			if(regs->user_esp > 0xFFFE)
				return 1;
/* tarnation!		regs->eflags = v86_pop16(regs); */
			regs->eflags = (regs->eflags & 0xFFFF0000L) |v86_pop16(regs);
		}
		return 0;

	case 0xCD:/* INT nn */
		i = v86_fetch8(regs); /* get interrupt number */
		DEBUGOUT("[int %X - eip=0x%x,flags=%x]\n",i,init_eip,regs->eflags);
		v86_int(regs, i);
		return 0;
	case 0xCF:/* IRET */
		if(prefix & PFX_OP32)
		{
			if(regs->user_esp > 0xFFF4)
				return 1;
			regs->eip = v86_pop32(regs);
			regs->cs = v86_pop32(regs);
			regs->eflags = v86_pop32(regs);
		}
		else
		{
			if(regs->user_esp > 0xFFFA)
				return 1;
			regs->eip = v86_pop16(regs);
			regs->cs = v86_pop16(regs);
			regs->eflags = (regs->eflags & 0xFFFF0000L) |v86_pop16(regs);
		}
		DEBUGOUT("[IRET]");
		return 0;

	case 0xE4:/* IN AL,imm8 */
	case 0xE6:/* OUT imm8,AL */
		i = v86_fetch8(regs);
		v86_enable_port(i);
		DEBUGOUT("[IN-OUT]");
/* restore original EIP -- we will re-try the instruction */
		regs->eip = init_eip;
		return 0;
	case 0xE5:/* IN [E]AX,imm8 */
	case 0xE7:/* OUT imm8,[E]AX */
		i = v86_fetch8(regs);
		v86_enable_port(i);
		v86_enable_port(i + 1);
		if(prefix & PFX_OP32)
		{
			v86_enable_port(i + 2);
			v86_enable_port(i + 3);
		}
		regs->eip = init_eip;
		return 0;
	case 0x6C:/* INSB */
	case 0x6E:/* OUTSB */
	case 0xEC:/* IN AL,DX */
	case 0xEE:/* OUT DX,AL */
		i = regs->edx & 0xFFFF;
		v86_enable_port(i);
		regs->eip = init_eip;
		DEBUGOUT("[INSB-OUT]");
		return 0;

	case 0x6D:/* INSW, INSD */
	case 0x6F:/* OUTSW, OUTSD */
	case 0xED:/* IN [E]AX,DX */
	case 0xEF:/* OUT DX,[E]AX */
		i = regs->edx & 0xFFFF;
		v86_enable_port(i);
		v86_enable_port(i + 1);
		if(prefix & PFX_OP32)
		{
			v86_enable_port(i + 2);
			v86_enable_port(i + 3);
		}
		regs->eip = init_eip;
		DEBUGOUT("[INSW-OUT]");
		return 0;
	case 0xFA:/* CLI */
		regs->eflags &= ~EFLAGS_IF;
		DEBUGOUT("[CLI]");
		return 0;
	case 0xFB:/* STI */
		regs->eflags |= EFLAGS_IF;
		DEBUGOUT("[STI]");
		return 0;
	}
	DEBUGOUT("Error in V86 mode at CS:IP=%x:%x\n",regs->cs, init_eip);/* anything else */

	return32bit();
	return 0;
}

void save_regs_stat(const regs_t *r)
{
	if(r)
	memcpy(&vm_regs_data, r, sizeof(regs_t));
}

void reset_regs_stat()
{
	vmback_regs=(regs_t *)0;
	memset(&vm_regs_data, 0, sizeof(regs_t));
}


 void * low_alloc(u16_t len);


void vm86_low_buffer_alloc(void)
{
    vm86_task_buffer = low_alloc(VM86_USEABLE_BUFSZ); 
}

void read_real_memory(char *buf, int sz)
{
	int s=(sz==VM86_USEABLE_BUFSZ?
		VM86_USEABLE_BUFSZ:(sz%VM86_USEABLE_BUFSZ));

	memcpy(buf, (void *)vm86_task_buffer, s);
}

void fill_real_memory(char *buf, int sz)
{
	reg_t flags;
	int s=(sz==VM86_USEABLE_BUFSZ?
		VM86_USEABLE_BUFSZ:(sz%VM86_USEABLE_BUFSZ));
	save_eflags(&flags);
	memcpy(vm86_task_buffer, (void *)buf, s);
	restore_eflags(flags); 
}

void real_memory_addr(u32_t *addr, int *sz)
{
	*addr=(u32_t)vm86_task_buffer;
	*sz=VM86_USEABLE_BUFSZ;
}
