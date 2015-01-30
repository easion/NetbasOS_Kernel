/* 
**  Jicama OS  
* * Copyright (C) 2003  DengPingPing     
**  All rights reserved.   
*/

#ifndef MODULE_H
#define MODULE_H

#include <sys/queue.h>

#define NR_OK_OPS (2)
#define NR_DLL_OPS (sizeof(struct module_ops_addr )/sizeof(unsigned))
#define EXPORT_PC_SYMBOL(sym_name) {#sym_name, (unsigned)sym_name }
#define EXPORT_DATA_SYMBOL(sym_name) {#sym_name, (unsigned)&sym_name }
#define NR_MODULE_ARRAY (0x80)

#define MOD_RUNNING_FLAGS 0X04
#define USER_MOD_FLAGS 0X10


struct _export_table_entry
{
	char *export_name;
	unsigned  export_addr;
	//int used;
};


typedef int (*dll_main_t)(char *argp[],void*);
typedef int (*dll_static_init_t)(int type, int value);
typedef	 int (*dll_destroy_t)(void);
typedef void* (*dll_find_secton_t)(void*,const char*,int *);
typedef void (*void_fn_void_t)(void);

struct module_ops_addr
{
	void* dll_start;
	void* dll_main;
	void* dll_destroy;
	void* dll_find_secton;
	void* dll_static_init; //for cpp
};




typedef struct module
{
	u8_t flags;
	//kern_module_t *next;

	char *module_name;
	unsigned long module_status;
	unsigned long module_size;
	unsigned char *module_address;
	unsigned char *text_address;
	unsigned char *data_address;

	struct module_ops_addr opaddr;

	void *bss;
	void *bss_size;
	LIST_ENTRY(dll_array) lists;
}kern_module_t;



int module_insert(kern_module_t *  m, char **argv);
int module_remove(kern_module_t *  m);
int   remove_dll_table(char *dll_namex);
int  install_dll_table(char *dll_namex, u32_t handle, int symbol_num, struct _export_table_entry *symbol_array);
void* module_find_secton(kern_module_t *  mod,const char *name,int *size);

kern_module_t *find_mod_slot(const char *modname);
kern_module_t *get_mod_slot(void);
void free_mod_slot(kern_module_t *mod);
extern void invalidate_all_mod_slot(void);
extern int lookup_kernel_symbol(char *sym_name, unsigned *adr, unsigned uscore);
extern int load_djcoff_relocatable(unsigned char *image,  kern_module_t *m);
extern int load_elf_relocatable(unsigned char *image,  kern_module_t *m);
extern int load_pecoff_relocatable(unsigned char *image, kern_module_t *mod);
extern int get_main_elf_sym(void *image, kern_module_t *mod);
extern char *get_module_sym_name(int index);
void relocatable_setup(struct module_ops_addr *ops, unsigned start_point);
int load_dll_memory(char*file, u8_t *exec_module, int text_len, char *default_argv[]);



#endif
