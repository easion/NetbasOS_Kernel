/*
**     (R)Jicama OS
**     Dll Manage
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/grub.h>
#include <jicama/process.h>
#include <jicama/coff.h>
#include <jicama/module.h>
#include <string.h>
static sem_t thread_sock_sem;


typedef struct module_cb_t
{
	LIST_ENTRY(module_cb_t) lists;
	char dll_name[256];
	int dll_num;
	struct _export_table_entry *dll_addr;
}module_cb_t;

static volatile  LIST_HEAD(, module_cb_t) mod_head;


void reset_dll_table()
{
	LIST_INIT(&mod_head);
}

inline module_cb_t* find_dll_table(char *dll_namex)
{
	module_cb_t *pmod;

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH(pmod,&mod_head,lists){
		if (strcmp(pmod->dll_name,dll_namex)==0){
			UNLOCK_SCHED(thread_sock_sem);	
			return pmod;
		}
	}

	UNLOCK_SCHED(thread_sock_sem);	

	return NULL;
}


 int  
 remove_dll_table(char *dll_namex)
{
	int found=0;
	module_cb_t *pmod=find_dll_table(dll_namex);

	
	if (!pmod)
	{
		return -1;
	}

	LIST_REMOVE(pmod,lists);
	kfree(pmod);
	
	return 0;
}

 int write_dll(char * buf, int size)
 {
	int len,i=0;
	module_cb_t *pmod;

	sprintf(buf, "NO\tDLLNAME\tSYMNUM\tADDRESS\n");
	len=strlen(buf);

	LOCK_SCHED(thread_sock_sem);	


	LIST_FOREACH(pmod,&mod_head,lists)
	{
		if (len>=size)break;	
		if(pmod->dll_name==NULL)continue;

		sprintf((char *)(buf+len), "%d\t%s\t%d\t0x%-8x\n",
		i++, pmod->dll_name, pmod->dll_num, pmod->dll_addr);

		len=strlen(buf);
	}

	UNLOCK_SCHED(thread_sock_sem);	
		return len;
 }

__local unsigned  
find_sym_in_dll(struct _export_table_entry *sym, int num, char *str)
{
	register int i;
	struct _export_table_entry *ete;

	for (i=0; i<num; i++)
	{
		ete=sym+i;
		if (strcmp(ete->export_name, str)==0){
			return ete->export_addr;
		}
	}
		//panic("err_code%d no f %s", num, str);
	return 0;
}

__local int 
dll_install_check(char *name, 
		int num, struct _export_table_entry *sym_pos)
{
	int i, j;
	struct _export_table_entry *sym;
	module_cb_t *pmod;

	LOCK_SCHED(thread_sock_sem);

	LIST_FOREACH(pmod,&mod_head,lists){/*check if exist here*/

		//if (pmod->dll_name==NULL)continue;				
		if (strcmp(pmod->dll_name,name)==0)return -1;	/*dll name exist*/

		for (j=0; j<num; j++)
		{
			sym=sym_pos+j;/*check if function name exist*/
			if (find_sym_in_dll(pmod->dll_addr, pmod->dll_num, sym->export_name)!=0){
				UNLOCK_SCHED(thread_sock_sem);
				kprintf("dll_install_check(): Function Name (%s) Exist!\n", sym->export_name);
				return -1;			
			}
		}

	}/*end for*/

	UNLOCK_SCHED(thread_sock_sem);

	return OK;
}

 int  install_dll_table(char *dll_namex, u32_t handle,
		int symbol_num, struct _export_table_entry *symbol_array)
{
	int i;
	module_cb_t *pmod;


	if (symbol_num==0)
	{
		kprintf(KFAILEDMSG"install_dll_table():  symbol_num null!\n");
		return -1;
	}
	
	if (dll_install_check(dll_namex, symbol_num, symbol_array)!=OK)
	{
		return -1;
	}

	pmod = kcalloc(sizeof(module_cb_t));
	if (!pmod)	{
		kprintf(KWARNMSG"install_dll_table(): no memory!\n");
		return -1;
	}

	strncpy(pmod->dll_name,dll_namex,256);
	pmod->dll_num=symbol_num;
	pmod->dll_addr=symbol_array;

	LOCK_SCHED(thread_sock_sem);	
	LIST_INSERT_HEAD(&mod_head,pmod,lists);
	UNLOCK_SCHED(thread_sock_sem);	
	return pmod;
}

__local unsigned  find_sym(char *s)
{
	unsigned entry;
	register int i;
	module_cb_t *pmod;

	LOCK_SCHED(thread_sock_sem);	

	LIST_FOREACH(pmod,&mod_head,lists)
	{
		//kprintf("find in %s\n", pmod->dll_name);
		entry=find_sym_in_dll(pmod->dll_addr, pmod->dll_num, s);
		if (entry==0)
		{
			//panic("find_sym(): err_code: not found %s in %s(%d)", s, pmod->dll_name, pmod->dll_num);
		}
		else{
			UNLOCK_SCHED(thread_sock_sem);	
			return entry;
		}
	}

	UNLOCK_SCHED(thread_sock_sem);	
	return 0;
}


int lookup_kernel_symbol(char *sym_name, unsigned *adr, unsigned uscore)
{
	unsigned  kern_addr;

	if(uscore)
	{
		if(sym_name[0] != '_')
			return -1;
		sym_name++;
	}

	kern_addr=find_sym(sym_name);

	*adr = kern_addr;
	if(kern_addr!=0)return 0;

	syslog(4, KWARNMSG"undefined external symbol '%s'\n", sym_name);
	return -1;
}


/*called from user space*/
int load_dll_file(char *file, char *s_argv[])
{
	int text_len,i;
	u8_t *exec_module;
	struct filp*fp;
	proc_t *rp = current_proc();
	char *default_argv[32] = {NULL,};

 	fp = server_open(file, "r");
	if(fp==NIL_FLP)return ENOENT;

	for (i=0; i<31; i++)
	{
		if (!(s_argv[i+1])){
			break;
		}
		default_argv[i] = (void*)proc_vir2phys(rp, s_argv[i+1]);
	}

	default_argv[i] = NULL;

	text_len = flength(fp);
	exec_module =(u8_t *)kcalloc(text_len);
	fread(exec_module, 1, text_len, fp);
	fclose(fp);
	return load_dll_memory(file, exec_module,text_len,default_argv);
}


int load_dll_memory(char*file, u8_t *exec_module, int text_len, char *default_argv[])
{
	int err_code;
	kern_module_t *  mod;
	exec_t ef=ELF;

	mod = find_mod_slot(file);

	if (mod)
	{
		return -1;
	}

	if (is_gzip_format_file(exec_module))
	{
		int ret;
		char *out;
		int out_len;
		ret = gzip_file(exec_module, text_len, &out, &out_len);
		mm_free(exec_module, text_len);
		exec_module = out;
		text_len = out_len;
	}

	ef=exec_get_format(exec_module,NULL);

	mod = get_mod_slot();

	if (!mod)
	{
		kprintf("get_mod_slot error\n");
		return -1;
	}

	switch (ef)
	{
	case ELF:		
		/* try loading as ELF */
		err_code = load_elf_relocatable((u8_t *)exec_module, mod);
		//kprintf("load_elf_relocatable %d!", err_code);
		break;
#ifdef __IA32__
	case COFF:
		/* try loading as djgpp coff */
		//kprintf("%s: coff dll type.\n", file);
		err_code=load_djcoff_relocatable((u8_t *)exec_module, mod);
		break;
	case DJMZ:	
		kprintf(KFAILEDMSG"wait DJMZ!");
		err_code=-1;
	break;
	case PE:		
		/* try loading as vc++ PE coff */
		err_code=load_pecoff_relocatable((u8_t *)exec_module, mod);
		break;
#endif
	default:
		kprintf(KFAILEDMSG"%s: unknow dll type.\n", file);
		err_code=-1;
		break;
	}

	mod->module_name = (char*)mm_malloc(strlen(file)+1);
	strcpy(mod->module_name,file);
	mod->module_address = exec_module;
	mod->module_size = text_len;
	mod->module_status = USER_MOD_FLAGS | MOD_RUNNING_FLAGS;

	if(err_code==(OK)){	
		//kprintf("load dll %s ok\n", file);
		module_insert(mod, default_argv);
	}else{
		kprintf(KFAILEDMSG"load dll %s error\n", file);
		mod->opaddr.dll_destroy=0;
		module_remove(mod);
	}	
	
	return err_code;
}

int remove_dll_file(char *modname)
{
	int ret;
	kern_module_t *  mod;

	mod = find_mod_slot(modname);
	if(!mod){
		syslog(LOG_DEBUG,"%s not exist\n",modname);
		return -1;
	}

	ret = module_remove(mod);
	return ret;
}
