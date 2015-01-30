
// ------------------------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//---------------------------------------------------------------------------------------------

#include <jicama/system.h>
#include <jicama/process.h>
#include <string.h>
static sem_t thread_sock_sem;

struct entry {
        LIST_ENTRY(entry) lists;
		int(*func)();
        char *arg;
		int interrupts;
		char name[64];
};

struct irq_act
{
	LIST_HEAD(listhead_irq, entry) head;
};

static volatile struct irq_act irq_act[NR_IRQS];


/*irq*/

int spurious_irq(int nr)
{
  if (nr < 0 || nr >= NR_IRQS){
    kprintf("Spurious irq number:%d\n", nr);
	panic("Invalid call to spurious_irq()\n");
  }
  return 1;// Reenable interrupt 
}

int irq_table_init(void)
{
	int i;
	for (i = 0; i < NR_IRQS; i++) {
		//irq_table[i] = (void*) spurious_irq;
		//irq_arg_table[i] = NULL;
		LIST_INIT(&irq_act[i].head);
	}
	return 0;
}

void intr_handler(int irq)
{
	int err;
	struct entry *item ;

	if (irq < 0 || irq >= NR_IRQS)
		panic("irq:%u  invalid call to put_irq_handler", irq);

	if (LIST_EMPTY(&irq_act[irq].head))
	{
		//Ã»ÓÐ±»×¢²á
		//kprintf("Spurious irq number:%d\n", irq);
		//panic("Invalid call to spurious_irq()\n");
		return;
	}


	LIST_FOREACH (item,&irq_act[irq].head, lists)
	{
		err = (*item->func)(item->arg,irq);
		if (!err)
		{
			item->interrupts++;
			break;
		}
	}


}

void* put_irq_handler(int irq,void* handler, void *arg,const char *irq_name)
{   
	unsigned cur_flag;

	struct entry *entry ;


	if (irq < 0 || irq >= NR_IRQS)
		panic("irq:%u  invalid call to put_irq_handler", irq);

	//if (irq_table[irq] == handler)
	//	return 0;	// extra initialization 

	//if (irq_table[irq] != (void*)spurious_irq)
	//	panic("Attempt to register:\n second irq handler for irq %d", irq);

	entry = kcalloc(sizeof(struct entry));

	if (!entry)
	{
		return NULL;
	}



	save_eflags(&cur_flag);


	//kprintf("dis_irq %d\n", irq);
	dis_irq(irq);
	

	entry->func = handler;
	entry->arg = arg;
	snprintf(entry->name,64,"%s",irq_name);


	LIST_INSERT_HEAD(&irq_act[irq].head,entry, lists);

	//puts("en_irq\n");
	en_irq(irq);
	//puts("en_irq done\n");
	restore_eflags(cur_flag);
	return entry;
}

/*
**
*/
__public int irq_proc_dump(char *buf, int cunt)
{
	int len = 0;
	int i;
	struct entry *item;
	

	len += sprintf(buf+len,"%-15s\t%s\t%s\t\t%s\t\t%s\n", 
		"DEV","IRQ","INTERRUPTS","PARAM", "OTHER");

	LOCK_SCHED(thread_sock_sem);	

	for (i=0; i<NR_IRQS; i++)
	{
		LIST_FOREACH (item,&irq_act[i].head, lists){
		/*if(irq_table[i]==(void *)&spurious_irq)
			continue;*/		
		len+=sprintf(buf+len,"%-15s\t%d\t%d\t%x\t%d\n", 
			item->name,
			i,
			item->interrupts,
			item->arg ,
			0
			);
		}
	}

	UNLOCK_SCHED(thread_sock_sem);	
	
	return len;
}


u32_t get_irq_handler(int irq)
{
  if (irq < 0 || irq >= NR_IRQS)
	panic("irq:%u  invalid call to put_irq_handler", irq);

 //if (irq_table[irq] == (void*)spurious_irq)
	return 0;	// extra initialization 

   //return  irq_table[irq];
}

int free_irq_handler(int irq, void *entry)
{
	struct entry *pentry=entry;
  if (irq < 0 || irq >= NR_IRQS)
	panic("irq:%u  invalid call to put_irq_handler", irq);

   dis_irq(irq);
   LIST_REMOVE(pentry, lists);
   
   return 0;

}

//////////////////////////////

int sys_irq_nest;

void sys_irq_enter()
{
	KATOMIC_INC(sys_irq_nest, int);
}

void sys_irq_leave()
{
	KATOMIC_DEC(sys_irq_nest, int);
}
