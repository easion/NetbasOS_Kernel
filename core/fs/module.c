
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/fs.h>
#include <jicama/module.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>

int module_exist(const char *str, const char *label, int count,char **ret);

int module_insert(kern_module_t *mod, char **argv)
{
	int ret;
	moduledata_t *mod_data_ptr;
	int size=0;

	ASSERT(mod != NULL);

	if(!mod->opaddr.dll_main){
		ERROR:
		kprintf("module_insert: dll_main() not found\n");
		free_mod_slot(mod);
		return -1;
	}

	if(mod->opaddr.dll_static_init){
		/*cpp init*/
		((dll_static_init_t)(mod->opaddr.dll_static_init))(1, 0xffff);
	}

	mod_data_ptr = module_find_secton(mod,KDRV_SECTON, &size);

	if (mod_data_ptr)
	{
		//kprintf("%s found\n",KDRV_SECTON);
		if (size%sizeof(moduledata_t))
		{
			kprintf("module moduledata_t error %d\n",size);
		}
		else{
			int i;
			for (i=0; i<(size/sizeof(moduledata_t)); i++)
			{
				register_module_handler(mod,&mod_data_ptr[i]);
			}
		}
	}
	

	ret=((dll_main_t)(mod->opaddr.dll_main))(argv,mod);
	if(ret!=OK){
		kprintf("module_insert: dll_main() init failed\n");
		free_mod_slot(mod);
	}
	return (OK);
}

int module_remove(kern_module_t *  mod)
{
	int ret;
	moduledata_t *mod_data_ptr;
	int size=0;

	ASSERT(mod != NULL);

	if (!(mod->module_status & MOD_RUNNING_FLAGS))
	{
		return -1;
	}

	mod_data_ptr = module_find_secton(mod,KDRV_SECTON, &size);

	if (mod_data_ptr)
	{
		//kprintf("%s found\n",KDRV_SECTON);
		if (size%sizeof(moduledata_t))
		{
			kprintf("module moduledata_t error %d\n",size);
		}
		else{
			int i;
			for (i=0; i<(size/sizeof(moduledata_t)); i++)
			{
				unregister_module_handler(mod,&mod_data_ptr[i]);
			}
		}
	}
	
	if(mod->opaddr.dll_static_init){
		/*cpp init*/
		((dll_static_init_t)(mod->opaddr.dll_static_init))(0, 0xffff);
	}

	if(mod->opaddr.dll_destroy)	
		ret=((dll_destroy_t)(mod->opaddr.dll_destroy))();
	else 
		ret=OK;



	if (mod->module_status & USER_MOD_FLAGS){
		mm_free(mod->module_address,mod->module_size);
		mm_free(mod->module_name,strlen(mod->module_name)+1);
	}

	if (mod->bss)
	{
		mm_free(mod->bss,mod->bss_size);
	}

	//if(mod->argp)	free_page((long)mod->argp);
	free_mod_slot(mod);
	if(ret!=OK)return EFAULT;
	//panic('module_remove called!');
	return (OK);
}

/*get module version*/
void* module_find_secton(kern_module_t *  mod,const char *name, int *size)
{
	ASSERT(mod != NULL);
	if(!mod->opaddr.dll_find_secton){
		kprintf("dll_find_secton empty");
		//free_mod_slot(mod);
		return NULL;
	}
	if(mod->opaddr.dll_find_secton!=0)
		return ((dll_find_secton_t)(mod->opaddr.dll_find_secton))(mod,name,size);
}


__local void module_name_trim(char *str)
{
	while (*str && *str!=' ')
	{
		str++;
	}
	*str = '\0';
	return ;
}

int module_exist(const char *str, const char *label, int count, char **ret)
{
	int len1=strlen(label);
	int len2=strlen(str);
	char *end_of_str=str+len2;
	char *begin_of_str=str;

	ASSERT(str != NULL);
	ASSERT(label != NULL);

	if(!len1 || !len2)
		return FALSE;

	if (count>0)
		len1 = MIN(count,len1);

	while((unsigned long)(begin_of_str+len1)<=(unsigned long)end_of_str){
		if(!strnicmp(begin_of_str, label,len1)){
			if(ret)*ret = begin_of_str;
			return TRUE;
		}
		begin_of_str++;
	}

	return FALSE;
}



#define NR_MODULE 32

__local kern_module_t *module[NR_MODULE];



kern_module_t *get_mod_slot()
{
	register int i;

	for (i=0; i<NR_MODULE; i++)
	{
		if(module[i]==(kern_module_t  *)0){
			module[i]=(kern_module_t  *)kcalloc(sizeof(kern_module_t));
			if(module[i])module[i]->flags=0;
			memset(module[i],0,sizeof(kern_module_t));
			return module[i];
		}
	}
	return (kern_module_t *)0;
}

kern_module_t *find_mod_slot(const char *modname)
{
	register int i;

	ASSERT(modname != NULL);

	for (i=0; i<NR_MODULE; i++)
	{
		if(module[i]==(kern_module_t * )0){
			continue;
			}
		//if(strcmp(modname, (char *)MOD_dll[i].string)==0)return module[i];
		if(strcmp(modname, module[i]->module_name)==0)
			return module[i];
	}
	return (kern_module_t * )0;
}

void free_mod_slot(kern_module_t *mod)
{
	int i;

	ASSERT(mod != NULL);

	if(!mod)return;
	for (i=0; i<NR_MODULE; i++)
	{
		if(module[i]==mod){
			mm_free(mod, sizeof(kern_module_t));
			module[i]=(kern_module_t *)0;
		}
	}
}

void invalidate_all_mod_slot()
{
	register int i;

	for (i=0; i<NR_MODULE; i++)
	{
		module[i]=(kern_module_t  *)0;
	}
}

void relocatable_setup(struct module_ops_addr *ops, unsigned base_address)
{
	ops->dll_start=base_address;
	ops->dll_main+=base_address;
	ops->dll_destroy+=base_address;
	
	//¿ÉÑ¡
	if(ops->dll_static_init)ops->dll_static_init+=base_address;
}

__public int moduledump(char *buf, int size)
{
	int len = 0;
	int i;

	len += sprintf(buf+len,"%-30s\t%s\t\t%s\t\t%s\n", "Module Name","Address","Size/bss","Flags");

	for (i=0; i<NR_MODULE; i++)
	{
		if(module[i]==(kern_module_t  *)0){
			continue;
		}
		if(!module[i]->module_size){
			continue;
		}
		module_name_trim(module[i]->module_name);
		len+=sprintf(buf+len,"%-22s 0x%x/0x%x/0x%x \t%d/%d\t\t%d\n", 
			module[i]->module_name, module[i]->module_address,
			module[i]->text_address,module[i]->data_address,
			module[i]->module_size,module[i]->bss_size,
			module[i]->module_status);
	}
	
	return len;
}

