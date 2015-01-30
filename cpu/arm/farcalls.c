
#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/module.h>
#include <jicama/devices.h>
#include <jicama/console.h>

__local struct _export_table_entry arch_sym_table []=
{
	//EXPORT_PC_SYMBOL(arch_thread_create)
};

void arch_sym_setup()
{
	int arch_nr =sizeof(arch_sym_table)/sizeof(struct _export_table_entry);
	install_dll_table("ARM.krnl", 1,arch_nr, arch_sym_table);
}


