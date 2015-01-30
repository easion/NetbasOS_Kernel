

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/grub.h>
#include <jicama/coff.h>
#include <jicama/fs.h>
#include <jicama/module.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
file_t* dll_info_init();

u32_t parse_args(const char *cmdline, char *str, int *argc, char *argv[]);
int module_exist(const char *str, const char *label, int count,char **ret);
void sym_init();
int ramdisk_set_params(void *addr, int size, int blk_size);
int init_kernel_command(void);

__asmlink void rawfs_init();
__asmlink char *init_cmd;

__local int last_dll;
module_t MOD_shell;
module_t MOD_dll[MAX_DLL_FILE];
#define DEBUG_NAMESIZE 16

struct kernel_param_list
{
	char *k_param_name;
	char k_param_type;
	void *k_param_value;
};
static char debug_dev[DEBUG_NAMESIZE];
static char input_dev[DEBUG_NAMESIZE];
__local console_t debug_console;

struct kernel_param_list kernel_param_list[32]={
	{"/network=", 'd', 1},
	{"/blocksize=", 'd', 512},
	{"/kmem=", 'd', 20},
	{"/baudrate=", 'd', 115200},
	{"/debug=", 'd', 0},
	{"/inputport=", 's', input_dev},
	{"/debugport=", 's', debug_dev},
	{NULL, 0, NULL},
};


int load_kernel_dlls(char *arg)
{
	int i, ac, err=-1, n=0;
	kern_module_t *  mod;
	char *pname;
	char *argv[16];

	ASSERT(arg!=NULL);

	for (i=0; i<last_dll; i++)
	{
		mod=get_mod_slot();
		if(!mod)return ;

		pname=(char *)MOD_dll[i].string;

		err = module_exist(pname, (char*)arg,0,NULL);

		if (err == false)
			continue;
		else
			n++;

		mod->module_name = pname;
		mod->module_address = MOD_dll[i].mod_start;
		mod->module_size = MOD_dll[i].mod_end - MOD_dll[i].mod_start;
		mod->module_status = MOD_RUNNING_FLAGS;
		bzero((u8_t *)argv, sizeof(argv));

		(char *)parse_args((char *)pname,0,&ac, argv);
		

		if (MOD_dll[i].mod_start<0x100000)
			panic("MOD_dll[i].mod_start=%x", MOD_dll[i].mod_start);

		switch (MOD_dll[i].reserved)
		{
		case COFF:
			/* try loading as djgpp coff */
		//kprintf("COFFDLL ");
			err=load_djcoff_relocatable((u8_t *)MOD_dll[i].mod_start, mod);
			break;

		case ELF:		
			/* try loading as ELF */
			//kprintf("Try loading as ELF dll: %s\n", pname);
			err = load_elf_relocatable((u8_t *)MOD_dll[i].mod_start, mod);	
			break;

		case DJMZ:	panic("DJMZ: wait");	break;

		case PE:		
			/* try loading as vc++ PE coff */
			kprintf("try loading win32 dll\n");
			err=load_pecoff_relocatable((u8_t *)MOD_dll[i].mod_start, mod);
			break;
			default:
				err=-1;
				panic("Unknow DLL Type!");
				break;
		}

		if(err==(OK)){	
			kprintf( "Load DLL %s "KOKMSG" begin.\n", argv[0]);
			module_insert(mod, argv);
			//module_version(mod);
		}
		else{
			trace( "load dll "KFAILEDMSG"\n");

			mod->opaddr.dll_destroy=0;
			module_remove(mod);

			panic("load_kernel_dlls(): Load %s failed!"
			"(Address %x type=%d,error=%d)\n", 
				(char *)MOD_dll[i].string, 
				MOD_dll[i].mod_start,
				MOD_dll[i].reserved,err );		
		}

	}

	return n;
}


#define DRIVER_MODULE 6
#define INIT_MODULE 4

void chinese_area(u32_t *addr, size_t *sz)
{
	ASSERT(addr != NULL);
	ASSERT(sz != NULL);
	*addr=boot_parameters.font_start;
	*sz=boot_parameters.font_size;
}

void save_module(int type, module_t *pmodule, int arg)
{
	ASSERT(pmodule != NULL);

	unsigned char *ptr;
	switch (type)
	{	
	

	case INIT_MODULE:
		memcpy(&MOD_shell, pmodule, sizeof(module_t));
		MOD_shell.reserved=arg;
	break;

	case DRIVER_MODULE:
		memcpy(&MOD_dll[last_dll], pmodule, sizeof(module_t));
		MOD_dll[last_dll].reserved = arg;
		last_dll++;
	break;

	default:
		break;	
	}
}


struct kernel_param_list* get_kparam_value(const char *tag_name)
{
	int i;
	for (i=0; kernel_param_list[i].k_param_name; i++)
	{
		if (stricmp(kernel_param_list[i].k_param_name, tag_name)==0)
		{
			return &kernel_param_list[i];
		}
	}
	return NULL;
}

static int init_kparam_value(const char *module_string)
{
	int i,len,retval;
	int err;
	char *string;
	char *ret;
	char match_name[1024];

	for (i=0; kernel_param_list[i].k_param_name; i++)
	{
		strncpy(match_name,kernel_param_list[i].k_param_name,1024);
		//strcat(match_name,"=");

		len = strlen(match_name);
		retval = module_exist(module_string, match_name,len,&ret);

		if(retval){ 

			string = kernel_param_list[i].k_param_value;

			switch (kernel_param_list[i].k_param_type)
			{
			case 's':
				{
			  strncpy(string,ret+len,DEBUG_NAMESIZE);
			  for (i=0; i<DEBUG_NAMESIZE; i++)
				  if (string[i]==' ')
					  string[i] = 0;
				break;
				}

			//case 'b':
			//	break;

			case 'd':
				{
				kernel_param_list[i].k_param_value = atoi(ret+len);
				break;
				}

			//case 'c':
			//	break;

			default:
				break;
			}
		}			
	}
	return 0;
}



int check_debug(void)
{
	return (int)get_kparam_value("/debug=")->k_param_value;
}

int check_kmem(void)
{
	int kmemsize=(int)get_kparam_value("/kmem=")->k_param_value;

	if (kmemsize>1024)
	{
		kmemsize = 1024;
	}
	else if (kmemsize<0)
	{
		kmemsize=24;
	}

	kprintf("kmemsize %d M\n", kmemsize);
	return kmemsize*0x100000;
}


int check_network()
{
	return (int)get_kparam_value("/network=")->k_param_value;
}

static void debug_init()
{
	memset(debug_dev,0,DEBUG_NAMESIZE);
	memset(input_dev,0,DEBUG_NAMESIZE);
}

int get_debug_device(char *devname, int len, int *bau)
{
	if (debug_dev[0] == 0)
		return -1;

	if(devname)
		strncpy(devname, debug_dev,len);
	if(bau)
		*bau = (int)get_kparam_value("/baudrate=")->k_param_value;
	return 0;
}

static int debug_putchar (console_t *con, unsigned char ch, bool vt)
{
	uart_putchar(ch);
	return 0;
}

__local struct tty_ops debug_ops = {
	name:		"Debug Port",
	putchar:	debug_putchar,
	readbuf:	kb_read,
	char_erase:	NULL,
	char_attrib:NULL,
};
static dev_prvi_t devprvi;

int load_krnl_option(void)
{
	int dev;
	char  module_string[1024]; //=get_kernel_command();
	char *ret;
	int len,i;

	debug_init();
	init_kernel_command();

	strncpy(module_string, get_kernel_command(),1024);

	if (!module_string){
		kprintf("get_kernel_command = %s\n","Null");
		return -1;
	}

	init_kparam_value(module_string);

	if (!check_debug() || !debug_dev[0])
		return 0;
	//debug

	//kprintf("kernel entry debug mode, with cmdline = %s\n",module_string);

	dev = dev_open("/dev/ttyS",2,&devprvi);
	if (dev<0){
		//return 0;
	}

	debug_console.attrib = 0;
	debug_console.base = 0;
	debug_console.vga_ops =  &debug_ops; 
	regster_console_device(&debug_console);
	select_console(&debug_console,true);
	//panic("load_krnl_option switch");
	return 1;
}

int grub_load_modules(grub_info_t* info)
{
	int i;
	void *exec_module;
	module_t *mod = (module_t *)0;
	char *module_string;

	ASSERT(info != NULL);

	rawfs_init();
	last_dll=0;

   for (i=0; i<MAX_DLL_FILE; i++)
 		MOD_dll[i].mod_start=0;

	invalidate_all_mod_slot();

   if (!CHECK_GRUB(info->flags,3))
	   return -1;

	dll_info_init();
	filp_init();
	

   for (i = 0; i < info->mods_count; i++)
    {
	  mod = (module_t*)( info->mods_addr + i * sizeof(module_t) );
	  (void)modfs_add((char *)mod->string, (char *)mod->mod_start, mod->mod_end-mod->mod_start);

	  module_string = (char *)mod->string;

      trace("module name: %s area: [%x - %x]\n", 
		  module_string ,
		mod->mod_start,
		  mod->mod_end );

	  exec_module = (void* ) mod->mod_start;	

	if(module_exist(module_string, "/module",0,NULL) || 
		module_exist(module_string, "/device",0,NULL)){ 
		
		/*djcpp coff format*/
		exec_t ef = exec_get_format(exec_module,NULL);
		if(ef==NO_EXEC)
			panic("Module [%s] format error!", module_string);

		save_module(DRIVER_MODULE, mod,ef);
		continue;
	  }

	if(module_exist(module_string, "/init",0,NULL)){ 
		/*djcpp coff format*/

		exec_t ef=exec_get_format(exec_module,NULL);

		if(ef==NO_EXEC)
			panic("%s not a EXEC format module!\n", module_string);
			init_cmd=module_string;
			save_module(INIT_MODULE, mod,ef);
			trace("init program is %s\n", mod->string);
			continue;
	  }	

	if(module_exist(module_string, "/ramdisk",0,NULL)){ 
		/*ramdisk*/
		ramdisk_set_params(mod->mod_start,mod->mod_end-mod->mod_start, (int)get_kparam_value("/blocksize=")->k_param_value);
	  }	

	trace("unknow module %s found\n", module_string);
	}

	return 0;
}


unsigned long grub_modules_end_address(grub_info_t* info)
{
	int i;
	module_t *mod = (module_t *)0;
	unsigned long module_end = 0;

	ASSERT(info != NULL);	

	for (i = 0; i < info->mods_count; i++) {
		mod = (module_t*)( info->mods_addr + i * sizeof(module_t) );
		module_end = MAX(mod->mod_end,module_end);
	}

	return module_end;
}


