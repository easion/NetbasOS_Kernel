#include <arch/x86/io.h>
#include <arch/x86/regs.h>
#include <jicama/devices.h>
#include <jicama/system.h>
#include <jicama/proc_entry.h>
#include <string.h>
#include "cpu.h"
void reset_fpu(void);
void init_fpu(void);
static int cpuid_proc(void *arg, int len,struct proc_entry *pf);
//
// unix_init
//
static struct proc_entry e_cpuid_proc = {
	name: "cpuinfo",
	write_func: cpuid_proc,
	read_func: NULL,
};


__local void cpuid(int op, int *eax, int *ebx, int *ecx, int *edx)
{
	__asm("cpuid"
	: "=a" (*eax),
	  "=b" (*ebx),
	  "=c" (*ecx),
	  "=d" (*edx)
	: "a" (op));
}

__local void cx_w (char index, char value)
{
	__asm ("pushf\n\t cli");		/* save flags */
	outb (0x22, index);	/* tell CPU which config. register */
	outb (0x23, value);	/* write to CPU config. register */
	__asm ("popf");			/* restore flags */
}

__local char cx_r (char index)
{
	char value;

	__asm ("pushf\n\t cli");		/* save flags */
	outb (0x22, index);	/* tell CPU which config. register */
	value = inb (0x23);	/* read CPU config, register */
	__asm ("popf");			/* restore flags */
	return value;
}

 int cpu_model_name(cpuinfo_t *c)
{
	int *v;
	char *p, *q;
	int __eax, __ebx, __ecx, __edx;

	cpuid(0x80000000, &__eax, &__ebx, &__ecx, &__edx);

	if (__eax < 0x80000004){		
	trace ("cpu_model_name: failed (eax- %x)\n", __eax);
	return -1;
	}

	v = (int *) c->x86_model_id;
	cpuid(0x80000002, &v[0], &v[1], &v[2], &v[3]);
	cpuid(0x80000003, &v[4], &v[5], &v[6], &v[7]);
	cpuid(0x80000004, &v[8], &v[9], &v[10], &v[11]);
	c->x86_model_id[48] = 0;

	/* Intel chips right-justify this string for some dumb reason;
	undo that brain damage */
	p = q = &c->x86_model_id[0];
	while ( *p == ' ' ) p++;
	if ( p != q ) {
	 while ( *p ) *q++ = *p++;
	 while ( q <= &c->x86_model_id[48] ) *q++ = '\0';
	}

	trace("CPU NAME (%s)\n", c->x86_model_id);
	return 0;
}


char *cpu_get_mode(int vendor_max, cpuinfo_t* __cpu)
{
	int i;

	for (i=0; i<vendor_max; i++)
	if (cpu_models[i].m_family == __cpu->family){
	strcpy(__cpu->vendor_family, cpu_models[i].m_name[__cpu->model]);
	return cpu_models[i].m_name[__cpu->model];
	}
	return NIL_PTR;
}

int cpu_cache_size(cpuinfo_t* __cpu)
{
	int __eax, __ebx, __ecx, __edx;
	cpuid(0x80000000, &__eax, &__ebx, &__ecx, &__edx);

	if(__eax > 0x80000005){
	cpuid(0x80000000, &__eax, &__ebx, &__ecx, &__edx);
	trace("CPU: L1 I Cache: %dK (%d bytes/line), D cache %dK (%d bytes/line)\n",
	__edx>>24, __edx&0xff, __ecx>>24, __ecx&0xFF);
	__cpu->x86_cache_size=(__ecx>>24)+(__edx>>24);
	}

	return __cpu->x86_cache_size;
}

void cpu_vendor_name(cpuinfo_t* __cpu)
{
	cpuid(0x00000000, &__cpu->cpuid_levels,
		  (int *)&__cpu->x86_vendor_id[0],
		  (int *)&__cpu->x86_vendor_id[8],
		  (int *)&__cpu->x86_vendor_id[4]);
}


int cpu_family(cpuinfo_t* __cpu)
{	
	int __eax, __ebx, __ecx, __edx;
	__eax = __ebx = __ecx = __edx = 0;
	cpuid(1, &__eax, &__ebx, &__ecx, &__edx);

	__cpu->serial[0] = __eax;
	__cpu->brand_id = (__ebx & 0xff);
	__cpu->flags = __edx;
	trace ("cpu flag (%d)\n", __cpu->flags);

	__cpu->family_ex = (__eax >> 20) & 0xff;
	__cpu->model_ex = (__eax >> 16) & 0x0f;

//	  __cpu->family_ex = (__eax & 0xff00000) >> 20;
//	  __cpu->model_ex = (__eax & 0xf0000) >> 16;
//	  __cpu->type = (__eax & 0x3000) >> 12;

	__cpu->type = (__eax >> 12) & 0x03;

	__cpu->family = (__eax >> 8) & 0x0f;


	__cpu->model = (__eax >> 4) & 0x0f;

	__cpu->setpping = (__eax & 0xf);
	__cpu->fpu = (__edx & 1 ? TRUE : FALSE);

	if(__cpu->fpu == TRUE){	
		trace("Initialising the fpu\n");
	}

	trace ("cpu type (%d)\n", __cpu->type);
	trace ("cpu family (%d)\n", __cpu->family);
	trace ("cpu model (%d)", __cpu->model);

	if (__cpu->family >= 6)  {
	__eax = __ebx = __ecx = __edx = 0;
	cpuid(2, &__eax, &__ebx, &__ecx, &__edx);

	__cpu->eax_2[0] = __eax;
	__cpu->eax_2[1] = __ebx;
	__cpu->eax_2[2] = __ecx;
	__cpu->eax_2[3] = __edx;

	if (__cpu->setpping >= 7){
	__eax = __ebx = __ecx = __edx = 0;
	cpuid(0, &__eax, &__ebx, &__ecx, &__edx);

	__cpu->serial[1] = __edx;
	__cpu->serial[2] = __ecx;
	}
	}
	return 0;
}

void cpuinfo_init(void)
{
	int i;
	cpuinfo_t cpuinfo;
	int __eax, __ebx, __ecx, __edx;

	memset(&cpuinfo,0,sizeof(cpuinfo));

	reset_fpu();
	init_fpu();
	/*
	**Intel-defined flags: level 0x00000001 
	**AMD-defined flags: level 0x80000001 
	** Transmeta-defined flags: level 0x80860001
	*/

	cpuid(0x80000000, &__eax, &__ebx, &__ecx, &__edx);

	trace("cpu:0x%x-", __eax);

	if (__eax > 0){
	__eax = __ebx = __ecx = __edx = 0;
	cpuid(0x80000001, &__eax, &__ebx, &__ecx, &__edx);
	cpuinfo.flags_ex = __edx;
	//panic("amd: 0x%x\n", __edx);
	}

	cpu_vendor_name(&cpuinfo);

	if (cpuinfo.cpuid_levels <= 0){
	  trace("Maximum supported CPUID level : %d, (-1=no CPUID)\n", cpuinfo.cpuid_levels);
	  return;
	}

	cpu_model_name(&cpuinfo);
	cpu_cache_size(&cpuinfo);
	cpu_family(&cpuinfo);
	cpu_get_mode(MAX_INTEL_MODEL, &cpuinfo);

	/*print vendor and model name*/
	trace("CPU Type: %s  %s\n", cpuinfo.x86_vendor_id, cpuinfo.vendor_family);

  register_proc_entry(&e_cpuid_proc);
}

static int cpuid_proc(void *arg, int len,struct proc_entry *pf)
{
	int i;
	cpuinfo_t cpuinfo;
	int __eax, __ebx, __ecx, __edx;

	memset(&cpuinfo,0,sizeof(cpuinfo));

	/*
	**Intel-defined flags: level 0x00000001 
	**AMD-defined flags: level 0x80000001 
	** Transmeta-defined flags: level 0x80860001
	*/

	cpuid(0x80000000, &__eax, &__ebx, &__ecx, &__edx);

	trace("cpu:0x%x-", __eax);

	if (__eax > 0){
	__eax = __ebx = __ecx = __edx = 0;
	cpuid(0x80000001, &__eax, &__ebx, &__ecx, &__edx);
	cpuinfo.flags_ex = __edx;
	//panic("amd: 0x%x\n", __edx);
	}

	cpu_vendor_name(&cpuinfo);

	if (cpuinfo.cpuid_levels <= 0){
	  trace("Maximum supported CPUID level : %d, (-1=no CPUID)\n", cpuinfo.cpuid_levels);
	  return;
	}

	cpu_model_name(&cpuinfo);
	cpu_cache_size(&cpuinfo);
	cpu_family(&cpuinfo);
	cpu_get_mode(MAX_INTEL_MODEL, &cpuinfo);

  pprintf(pf, "CPU infomation                       \n");
  pprintf(pf, "----------- ----------- --------------- ---------------\n");

  pprintf(pf,"CPU Type: %s  %s\n", cpuinfo.x86_vendor_id, cpuinfo.vendor_family);
  pprintf(pf,"CPU NAME (%s)\n", cpuinfo.x86_model_id);

  pprintf(pf,"fpu support: %s\n",cpuinfo.fpu == TRUE?"Enable":"Disable");
  pprintf(pf,"cpu type (%d)\n", cpuinfo.type);
  pprintf(pf,"cpu family (%d)\n", cpuinfo.family);
  pprintf(pf,"cpu model (%d)\n", cpuinfo.model);
  pprintf(pf,"cpu cache size (%d)\n", cpuinfo.x86_cache_size);
  return 0;
}

