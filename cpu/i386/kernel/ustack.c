#include <jicama/system.h>
#include <jicama/paging.h>
#include <jicama/process.h>
#include <jicama/elf.h>
#include <signal.h>
#include <string.h>
#include <ansi.h>
#include <assert.h>

#define ENV_CHAR_FLAGS '\n'
#define ENV_ITEMS 32

int read_script_info(const char *scriptfile,int mode, char *execfile);

__local char *default_env[ENV_ITEMS]={"HOME=/user/", "SHELL=/bin/sh", 
	"PATH=/bin:/system/bin:/user/bin:/usr/bin",	"TMP=/user/tmp", "HOSTNAME=NetbasPC", "OS=NetbasOS",0};

//环境变量的匹配算法 
int env_match(const char *env1,const char *env2)
{
	char *str1 = env1;
	char *str2 = env2;
	char *tmp = env2;
	int len=0,c=0;

	while (*str1==' '&&*str1)str1++;
	if(!*str1)return -1;
	tmp = str1;
	while(tmp[len] && tmp[len]!='='){
		tmp[len++];
	}

	while (*str2==' '&&*str2)str2++;
	if(!*str2)return -1;

	while (c<len)
	{
		if(str1[c] != str2[c])
			return -1;
		if (str2[c]=='=')
			break;
		c++;
	}

	if (c<len || str1[c]!='=')
		return -1;

	return 0;
}

//设置,更新系统参数
int set_userspace_environmental(const char *envptr)
{
	int i;
	for (i=0; i<ENV_ITEMS; i++)
	{
		//找到空闲的项
		if (!default_env[i])
			break;
		//找到匹配的项
		if(env_match(default_env[i], envptr)==0)
			break;
	}

	if (i>=ENV_ITEMS-1)
		return -1;

	default_env[i] = envptr;
	default_env[ENV_ITEMS-1] = NULL;
	return 0;
}

char *kern_strdup(const char *str)
{
	int size = strlen(str)+1;
	char *res = kmalloc(size,0);

	if (!res)
		return NULL;
	strcpy(res,str);
	return res;
}

//装载内核参数为默认的环境变量 

int userspace_environmental_init(void)
{
	char  str[1024];// = get_kernel_command();
	char *begin_of_str=str;
	int len1,len2 ;
	char *end_of_str;
	char buf[1024];
	int i;

	strncpy(str,get_kernel_command(),1024 );

	//kprintf("kernel command = %s\n",str);

	len2 = strlen(str);
	end_of_str=str+len2;

	if(!str[0])return 0;

	if(!len2)
		return FALSE;

	while((unsigned long)(begin_of_str)<=(unsigned long)end_of_str){
		char *res;

		while(*begin_of_str==' ' || *begin_of_str=='/')
			begin_of_str++;

		i=0;
		memset(buf,0,sizeof(buf));

		while(begin_of_str[i]!=0 && begin_of_str[i]!=' '){
			buf[i] = begin_of_str[i];
			i++;
		}

		if(i){
			res = kern_strdup(buf);
			begin_of_str += i;
			set_userspace_environmental(res);
		}else{
			break;
		}
	}

	return 0;
}

u32_t put_netbas_style_argv(thread_t *pthread, int arg_count, char *argv[],  char *environ_arg[])
{
	int i;
	int env_count, len;
	u32_t kesp,user_esp, v, env;
	char **__environ;
	u32_t env_array[MAX_EXEC_ARGS];
	u32_t argment_array[MAX_EXEC_ARGS];
	u32_t proc_space_base = proc_phy_addr(THREAD_SPACE(pthread));

	//the kesp in system space
	kesp = proc_vir2phys(THREAD_SPACE(pthread),USER_STACK_ADDR_END);
	
	//the kesp in user space
	user_esp = USER_STACK_ADDR_END;

	trace("put to %x at proc%d\n", kesp, THREAD_SPACE(pthread)->p_asid);

	if(environ_arg==NULL)
		{__environ = default_env;}
	else
		{__environ = environ_arg;}


	env_count=0;
	while(__environ[env_count]){env_count++;};
	bzero((u8_t*)env_array,  sizeof(u32_t)*MAX_EXEC_ARGS);

	arg_count=MIN(arg_count, MAX_EXEC_ARGS-1);
	env_count=MIN(env_count, MAX_EXEC_ARGS-1);

	for (i=0;i<env_count;i++) /*put environ*/
	{
		len=strlen(__environ[i])+1;
		user_esp -= len;
		kesp -= len;		
		env_array[i] = user_esp;
		//trace("[environ] %s\n", __environ[i]);
		strcpy((char *)kesp, __environ[i]);
	}

	memset(argment_array, 0, sizeof(argment_array));

	for (i=0;i<arg_count;i++)
	{
		len=strlen(argv[i])+1;
		user_esp-=len;
		argment_array[i]=user_esp;
		kesp -= len;
		//trace("[argv] %s\n", argv[i]);
		strcpy((char *)kesp, argv[i]);
	}

	/*stack list:
	**  +xx  env[x]
	**  +xx  argv[x]
	**  +16  argv[1]
	**  +12  argv[0]
	**  +8  **env
	**  +4  argv
	**  +0  argc
	*/
	kesp-=((env_count+1)*4); /*env space*/
	env=kesp-proc_space_base;

	kesp-=((arg_count+4)*4); /*argc and argv space*/

	pthread->tss.esp=kesp-proc_space_base;
	v=pthread->tss.esp+12;
	
	poke32(kesp, arg_count); /*argc*/
	
	poke32(kesp+4, (v)); /*argv*/
	poke32(kesp+8, (env)); /*env*/
	
	for (i=3; i<=(arg_count+3); i++){
	 poke32((kesp+i*4), argment_array[i-3]); /*argv*/
	}

	kesp+=((arg_count+4)*4); /*write env space*/
	
	for (i=0; i<=env_count; i++){
	 poke32((kesp+i*4), env_array[i]); /*env*/
	}

	kesp=pthread->tss.esp;
	return kesp;
}

/*parse args from cmd_line*/
u32_t parse_args(const char *cmd_line, char *space, int *argc, char *argv[])
{
	int i=0;
	long page;
	char *s;
	
	//check if it null
	if(!cmd_line)return  0;

	if(space==(char *)0){
		page=get_page();	
		if (!page)	panic("parse_args(): NO Memory (%s)\n", cmd_line);
		s=(char *)page;	
	}else{
		s=space;
		page=0;
	}

	strncpy(s, cmd_line,PAGE_SIZE);
 
  	do {
		while (*s == ' ' || *s == '\t')*s++ = '\0';
		if (*s != '\0'){
			argv[i++] = s;
			//trace("%s-%d\n", argv[i-1],i-1);
		}
		while (*s != ' ' && *s != '\t' && *s != '\0')s++;
	} while (*s != '\0');

	*argc=i;
	return page;
}


/*parse args from cmd_line*/
u32_t parse_args_new(const char *cmd_line,  int *argc, char *argv[])
{
	int count,i=0;
	char *s;
	
	//check if it null
	if(!cmd_line)return  0;
	s=(char *)cmd_line;		

	//strncpy(s, cmd_line,PAGE_SIZE);
 
  	do {
		count = *(u16_t*)s;
		*(u16_t*)s = 0;
		s+=2;
		argv[i++] = s;
		//kprintf("env = %s\n", argv[i-1]);
		s+=count;
		//while (*s == ' ' || *s == '\t')*s++ = '\0';
		//if (*s != '\0'){
		//	argv[i++] = s;
		//}
		//while (*s != ' ' && *s != '\t' && *s != '\0')s++;
	} while (*s);

	*argc=i;
	return 1;
}

__local u32_t parse_environs(const char *cmd_line,int *argc, char *_envs[])
{
	int i;
	long page;
	char *s;
	
	//check cmd_line
	if(!cmd_line)return 0;
		
	page=get_page();	
	/*can not get page */
	if (!page){panic("parse_args(): NO Memory (%s)\n", cmd_line);return 0;}
	s=(char *)page;	

	i=0;
	strncpy(s, cmd_line,PAGE_SIZE);
 
  	do {
		while (*s == ENV_CHAR_FLAGS || *s == '\t')*s++ = '\0';
		if (*s != '\0'){
			_envs[i++] = s;
			trace("%s-%d\n", _envs[i-1],i-1);
		}
		while (*s != ENV_CHAR_FLAGS && *s != '\t' && *s != '\0')s++;
	} while (*s != '\0');

	*argc=i;
	return page;
}

__public u32_t push_default_argv(thread_t *pthread, char *argline, char *envline)
{
   //u32_t tp;
   u32_t tp2;
   u32_t user_esp;
   char *p[MAX_EXEC_ARGS];
   char *p2[MAX_EXEC_ARGS];
   int  argcount=0;
   int tmp;
   proc_t *rp = THREAD_SPACE(pthread);
	
	/*check args*/
	if(argline==(char *)0)	{  return 0;  }

	bzero((u8_t*)p, sizeof(char *)*MAX_EXEC_ARGS);
	bzero((u8_t*)p2, sizeof(char *)*MAX_EXEC_ARGS);

	trace("argline%s\n", argline);
	parse_args_new(argline, &argcount, p);

	if(envline!=NULL)	{
		trace("envline%s\n", envline);
		tp2=parse_environs(envline, &tmp, p2);
		/*make , put to user space*/
		user_esp= put_netbas_style_argv(pthread, argcount, p,p2);
		free_page(tp2);
	}else{
		/*make , put to user space*/
		user_esp= put_netbas_style_argv(pthread, argcount, p,(char **)0);
	}

	//free_page(tp);
	return user_esp;
}
/*
    When we enter this piece of code, the program stack looks like this:
        argc            argument counter (integer)
        argv[0]         program name (pointer)
        argv[1...N]     program args (pointers)
        argv[argc-1]    end of args (integer)
	NULL
        env[0...N]      environment variables (pointers)
        NULL
*/
/* Symbolic values for the entries in the auxiliary table
   put on the initial stack */
#define ATNULL   0		/* end of vector */
#define ATIGNORE 1		/* entry should be ignored */
#define ATEXECFD 2		/* file descriptor of program */
#define ATPHDR   3		/* program headers for program */
#define ATPHENT  4		/* size of program header entry */
#define ATPHNUM  5		/* number of program headers */
#define ATPAGESZ 6		/* system page size */
#define ATBASE   7		/* base address of interpreter */
#define ATFLAGS  8		/* flags */
#define ATENTRY  9		/* entry point of program */
#define ATNOTELF 10		/* program is not ELF */
#define ATUID    11		/* real uid */
#define ATEUID   12		/* effective uid */
#define ATGID    13		/* real gid */
#define ATEGID   14		/* effective gid */

u32_t put_linux_style_argv(thread_t *pthread, int arg_count, char *argv[],  char *environ_arg[])
{
	int i;
	int env_count, len;
	u32_t kesp, env;
	char **__environ;
	u32_t env_array[MAX_EXEC_ARGS];
	u32_t argment_array[MAX_EXEC_ARGS];
	u32_t proc_space_base = proc_phy_addr(THREAD_SPACE(pthread));

	ASSERT(pthread);
	ASSERT(THREAD_SPACE(pthread));

	//the kesp in system space
	kesp = proc_vir2phys(THREAD_SPACE(pthread),USER_STACK_ADDR_END)-1;

	

	kesp -= (ATEGID+2)*8;

	u32_t * entry = (u32_t*)kesp;  

#define AUXVEC_PUT(id, value) do{*entry++ = id, *entry++ = value;}while(0)
	AUXVEC_PUT(ATPHDR, 0);
	AUXVEC_PUT(ATPHENT, sizeof(elfphdr_t));
	AUXVEC_PUT(ATPHNUM, 0);
	AUXVEC_PUT(ATPAGESZ, PAGESIZE);
	AUXVEC_PUT(ATBASE, 0);
	AUXVEC_PUT(ATFLAGS, 0);
	AUXVEC_PUT(ATEGID, 0);
	AUXVEC_PUT(ATUID, THREAD_SPACE(pthread)->uid);
	AUXVEC_PUT(ATEUID, 0);//THREAD_SPACE(pthread)->euid);
	AUXVEC_PUT(ATGID, 0);// THREAD_SPACE(pthread)->gid);
	AUXVEC_PUT(ATEGID, 0);// THREAD_SPACE(pthread)->egid);
	AUXVEC_PUT(ATNULL, 0);
#undef AUXVEC_PUT

	if(environ_arg==NULL){
		__environ = default_env;
	}
	else{
		__environ = environ_arg;
	}

	env_count=0;
	while(__environ[env_count]){
		env_count++;
	};

	bzero((u8_t*)env_array,  sizeof(u32_t)*MAX_EXEC_ARGS);
	bzero((u8_t*)argment_array, sizeof(u32_t)*MAX_EXEC_ARGS);

	arg_count=MIN(arg_count, MAX_EXEC_ARGS-1);
	env_count=MIN(env_count, MAX_EXEC_ARGS-1);


	for (i=0;i<env_count;i++) /*put environ*/
	{
		//trace("envtag  %s at %x ", __environ[i],  proc_space_base);
		len=strlen(__environ[i])+1;
		kesp -= len;		
		//kprintf("tagxx1  %x - %x \n",kesp, env_array[i]);
		strcpy((char *)kesp, __environ[i]);
		env_array[i] = kesp - proc_space_base;
	}


	for (i=0;i<arg_count && argv;i++)	{
		len=strlen(argv[i])+1;
		kesp -= len;
		argment_array[i]=kesp - proc_space_base;
		strcpy((char *)kesp, argv[i]);
		//trace("argv[%d] %s\n",i,argv[i] );
	}


	/*stack list:
	**  +xx  env[x]
	**  +xx  argv[x]
	**  +8  argv[1]
	**  +4  argv[0]
	**  +0  argc
	*/
	kesp -= ((env_count+1)*4); /*env space*/
	for (i=0; i<env_count; i++){
		 poke32((kesp+i*4), env_array[i]); /*env*/
		 if(!env_array[i]){
			 break;
		 }
		trace("env @ 0x%x %s \n", env_array[i], (char*)proc_space_base+env_array[i]);
	}

	poke32((kesp+i*4), 0); /*env*/

	kesp-=((arg_count+1)*4); /*argv space*/	

	for (i=0; i<(arg_count); i++){
		 poke32((kesp+i*4), argment_array[i]); /*argv*/
		 trace ("argv @ 0x%x at %s\n", argment_array[i], (char*)proc_space_base+argment_array[i]);
	}

	poke32((kesp+i*4), 0); /*argv,null*/

	kesp-=(4);
	poke32(kesp, arg_count); /*argc*/

	return (kesp-proc_space_base);
}


__public u32_t push_linux_argv(thread_t *pthread, char *argline, char *envline)
{
   //u32_t tp;
   u32_t tp2;
   u32_t user_esp;
   char *p[MAX_EXEC_ARGS];
   char *p2[MAX_EXEC_ARGS];
   int  argcount;
   int tmp;
   proc_t *rp = THREAD_SPACE(pthread);
	
	/*check args*/
	if(argline==(char *)0)	{  return 0;  }

	bzero((u8_t*)p, sizeof(char *)*MAX_EXEC_ARGS);
	bzero((u8_t*)p2, sizeof(char *)*MAX_EXEC_ARGS);

	parse_args_new(argline, &argcount, p);

	if(envline!=NULL)	{
		//kprintf("envline%s\n", envline);
		tp2=parse_environs(envline, &tmp, p2);
		/*make , put to user space*/
		user_esp= put_linux_style_argv(pthread, argcount, p,p2);
		free_page(tp2);
	}else{
		/*make , put to user space*/
		user_esp= put_linux_style_argv(pthread, argcount, p,(char **)0);
	}


	//free_page(tp);
	return user_esp;
}


/*count how many args*/
__local int count (const char **argv)
{
  int i = 0;
  char **tmp= (char **)argv;

    while (tmp[i]!=NULL){
      i++;
	}

  return i;
}

/*save  args for user space*/
int save_arg_space(proc_t *rp, char *file_name, char **argv, char **env, int start_arg)
{
	int count,start_env=0; //开始的环境参数
	char **tmp;
	u8_t *argptr;

	rp->proc_user_params = get_page();
	//flush pages buffer
	rp->proc_user_env = NULL;
	

	if(argv){
		if (!rp->proc_user_params)
		{
			return -1;
		}
	}
	else{
		kprintf("save_arg_space: empty argv\n");
	}

	if(env){
		rp->proc_user_env=get_page();
		if (!rp->proc_user_env)
		{
			return -1;
		}
	}
	else{
		//kprintf("save_arg_space: empty env\n");
	}

	argptr = rp->proc_user_params;

	//first args is user file
	if(argptr){
		count = strlen(file_name);
		*(u16_t*)argptr = count;
		argptr+=2;
		strncpy((char *)argptr, file_name,PAGE_SIZE);
		argptr+=(count);
	}

	if ((tmp = argv)!=NULL){
		while  (tmp[start_arg]!=NULL){
		//取得系统地址
		tmp[start_arg]=(char *)(proc_vir2phys(rp,tmp[start_arg]));
		count = strlen(tmp[start_arg]);
		*(u16_t*)argptr = count;
		argptr+=2;
		strcpy((char *)argptr, tmp[start_arg]);
		argptr+=(count);
		//构造一个字符串来包含所有参数内容
		//strcat((char *)rp->proc_user_params, " ");
		//strcat((char *)rp->proc_user_params, tmp[start_arg]);
		start_arg++;
		}
	}

	if ((tmp = env)!=NULL){
		while  (tmp[start_env]!=NULL)
		{
			tmp[start_env]=(char *)(proc_vir2phys(rp,tmp[start_env]));
			trace("%s-", tmp[start_env]);
			
			 if(start_env==0){
				strcpy((char *)rp->proc_user_env, tmp[start_env]);
			 }
			 else{
				strcat((char *)rp->proc_user_env, "\n");  //ENV_CHAR_FLAGS" "
				strcat((char *)rp->proc_user_env, tmp[start_env]);
			 }		
			start_env++;
		}
	}

	return 0;
}

/*after put,
we must free the args pages
*/
void destory_arg_space(proc_t *rp)
{
	if (!rp)
	{
		return;
	}

	if (rp->proc_user_params)
	{
		free_page(rp->proc_user_params);
		rp->proc_user_params=NULL;
	}	

	if (rp->proc_user_env)
	{
		free_page(rp->proc_user_env);
		rp->proc_user_env=NULL;
	}
}
