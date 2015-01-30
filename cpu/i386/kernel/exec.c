/*
 **     (R)Jicama OS
**     user program support
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <signal.h>
#include <string.h>
#include <ansi.h>
#include <assert.h>
#include <jicama/grub.h>
#include <jicama/elf.h>
#include <jicama/coff.h>


int load_pe_exec(struct filp*  f,  thread_t *_rp, unsigned  *_entry);
__local int check_real_program(const char *scriptfile,int mode, char *execfile);
int save_arg_space(proc_t *rp, char *file_name, char **argv, char **env, int start_arg);
u32_t push_linux_argv(thread_t *pthread, char *line, char *line2);

int do_execve(char *filename,  char ** argp, char ** envp)
{
	int  ret,nr;
	int arg1 = 1;
	char execname[512];
	thread_t *pthread = current_thread();
	proc_t *rp = THREAD_SPACE(pthread);
	char *realpath=filename;

	 nr=proc_number(rp);

	 if (!IS_USER_TASK(nr)){
		 trace("process number error !\n");
		 return EAGAIN;
	 }

	 memset(execname, 0, sizeof(execname));
	
	/*make sure file exist, and test if is a script file*/
	 ret = check_real_program(filename, 1, execname);

	 trace("check real program ok");

	 if (ret<0)
	 {
		//kprintf("%s not exist here\n", filename);
		 return ret;
	 }
	 else if (ret>0)
	 {
		 trace("'exec %s\n", execname);
		 realpath = execname;
		 //filename = execname;
		 arg1 = 0;
	 }
	 else{
		 //kprintf("%s  exist here\n", filename);
		 /*if not a script file*/
	 }

	trace("begin save args ...\n");

	/*setup args in user space*/
	ret = save_arg_space(rp, filename, argp, envp,arg1);	
	if (ret<0)
	{
		kprintf("save_arg_space error\n");
		return ENOMEM;
	}

	 /*main step: load program to memory*/
	 ret= exec_file(pthread, realpath, filename,(char *)rp->proc_user_params, (char *)rp->proc_user_env);


	 if(ret!=OK){
		 if(ret == -1){
			sendsig(pthread, SIGSTOP);
			kprintf("exec send to task %s!\n",pthread->name);
		 }else{
			 kprintf("do_execve() exec failed!\n");
		 }
		 return ret;
	 }
	 
	sendfsinfo(proc_number(rp), nr,FS_EXEC);
	return (OK);
}



/*script file*/
exec_t script_check(char *ptr, char *fname)
{
	int i=0;

	trace("script_check %s\n", ptr);

	if (strncmp(ptr, "#!",2)!=0)
	{
		return NO_EXEC;
	}

	while (ptr[i+2]&&!isspace(ptr[i+2]))
	{
		fname[i] = ptr[i+2];
		i++;
	}
	return  SCRIPT;
}

//#define LOAD_SO_USER_MODE 1

//检查是否为一个脚本，或者是否是一个动态连接库
int check_real_program(const char *scriptfile,int mode, char *execfile)
{
	struct filp*fp;
	u8_t buff[512];

	if (mode)
	{
		fp = server_open(scriptfile, "r");
	}else{
		fp = modfs_open(scriptfile, "r");
	}

	if(fp==NIL_FLP){
		trace("file %s not found", scriptfile);
		return ENOENT;
	}

	memset(buff,0,512);

	fread( buff, 512, 1, fp);
	if (script_check(buff, execfile) == SCRIPT)
	{
		/*ok, we find a script file*/
		return 1;
	}
	if (elf_check_so(fp) == ELF_USER_SO)
	{
#ifdef LOAD_SO_USER_MODE
		strcpy(execfile, "/bin/rldserver");
		return 1;
#endif
	}
	fseek(fp, 0, 0);
	fclose(fp);
	return 0;
}

inline void dump_code(u8_t *code)
{
	int i;
	kprintf("address 0x%x error code:[", (u32_t)code);
	for (i=0; i<20; i++)
	{
		kprintf("0x%x ", code[i]);
	}
	kprintf("]\n");
}

int user_space_write_test(proc_t *rp)
{
	ASSERT(rp);
	u8_t old;
	u8_t *addr_ptr;
	unsigned long offset = 0x04;	

	addr_ptr = (u8_t *)proc_vir2phys(rp,offset);
	old = addr_ptr[0];
	addr_ptr[0]=0x5a;

	if (addr_ptr[0]!=0x5a && old != 0x5a)
	{
		kprintf("addr_ptr: %x - %x -%x at 0x%x\n",
			addr_ptr[0], addr_ptr[1],addr_ptr[2], addr_ptr);
		return -1;
	}

	addr_ptr[0] = old;
	return 0;
}

void dump_user_space_code(proc_t *rp, const long offset)
{
	ASSERT(rp);
	u8_t *addr_ptr;

	addr_ptr = (u8_t *)proc_vir2phys(rp,offset);
	dump_code((u8_t*)addr_ptr);
	return;
}

int exec_file(thread_t *ethread, char*realpath,  char *filepath, char *arg_str, char *env_str)
{
	int r=-1;
	u32_t uesp;
	unsigned e,  nr;
	exec_t format;
	struct filp*fp;
	u8_t buff[1024];

	if (!filepath)
	{
		filepath = realpath ;
	}

	if (user_space_write_test(ethread))
	{
		panic("user_space_write_test(): failed!\n");
		return -1;
	}

	nr=proc_number(ethread->plink);

	/*try open if it loaded into mem*/
	fp = modfs_open(realpath, "r");

	if (fp == NIL_FLP)	{
		/*not in mem, try open it on disk*/
		fp = server_open(realpath, "r");	
	}
	else{
		trace("exec_file(): %s load on memory\n", realpath);
	}

	if(fp==NIL_FLP){
		/*file not exist*/
		//kprintf("exec_file(): file %s not found", realpath);
		return ENOENT;
	}

	memset(buff,0,512);

	fread( buff, 512, 1, fp);
	fseek(fp, 0, 0);

	if (djmz_check(fp)==0){
		format = exec_get_format((void *)buff,fp);
	}
	else{
		format = DJMZ;
	}
	pname_strcpy((char *)ethread->name, filepath);

	if (nr != INIT_PROC)	{
	trace("the format is %d,unmap proc ...\n", format);
		unmap_proc(ethread->plink, false);
		//map_proc(ethread->plink);
	}


	switch (format)
	{
		case COFF:
		{
			trace("coff execute Format!\n");
			r=read_coff(fp, ethread,&e,0);
			break;
		}
		case ELF:
		{
			r = read_elf_static(fp, ethread,&e);
			trace("elf execute Format r=%d!\n",r);
			break;
		}

		case ELF_USER_SO:{
			trace("elf dy Format!\n");
			user_heap_init(THREAD_SPACE(ethread),USER_MMAP_ADDR, USER_MMAP_SIZE);
			//user_heap_init(THREAD_SPACE(ethread),0x10000000, 0x10000000);
			r = read_elf_dynamic(fp, ethread,&e);
			break;
		}

		case PE:
		{
			kprintf("PE execute Format!\n");
			r = load_pe_exec(fp, ethread,&e);
			break;
		}
		case DJMZ:
		{
			r = read_djmz(fp, ethread,&e);
			break;
		}
		default:
		{
			kprintf("No execute Format %d,%s!\n", format, realpath);
			r = -1;
			break;
		}
	}

#if 0
	if (ethread->plink->linux_type!=1){
		uesp = push_default_argv(ethread, arg_str, env_str);
	}
	else
#endif		
	{
		/*linux style*/
		uesp = push_linux_argv(ethread, arg_str, env_str);
	}

	 if (uesp == 0) {
		kprintf("push_default_argv error\n");
		 return -1;
	}

	ethread->u_stack = uesp - 16384;
	ethread->plink->p_brk_code = ethread->plink->p_bss_code;

	trace("execute entry at 0x%x! uesp 0x%x - 0x%x\n",e,uesp,ethread->u_stack);

	fclose(fp);
	//snprintf(ethread->name,OS_NAME_LENGTH,"%s_0",ethread->name);
		
	if(r==OK){
		ethread->tss.eip = e;	
		ethread->tss.esp = uesp;
		ethread->plink->start_stack = uesp&0xfffff000;	
	}
	else{
		kprintf("failed: %s", realpath);
		return r;
	}

	//panic("done\n");
	return OK;
}



int handler_execve_ptrace(thread_t *pthread, regs_t *reg)
{
	//proc_t *rp = THREAD_SPACE(pthread);

	if ((pthread->trace_flags & PF_PTRACED))
	{
		//进入调试
		pthread->trace_regs = reg;
		pthread->exit_code = SIGTRAP;
		kprintf("handler_execve_ptrace() called\n");
		thread_wait(pthread, INFINITE);
	}
	return 0;
}


