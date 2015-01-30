/*
**     (R)Jicama OS
**     Main Function
**     Copyright (C) 2003 DengPingPing
*/

#include <jicama/system.h>
#include <jicama/process.h>
#include <jicama/linux.h>
#include <jicama/module.h>
#include <jicama/grub.h>
#include <jicama/coff.h>
#include <jicama/timer.h>
#include <jicama/msgport.h>
#include <jicama/proc_entry.h>
#include <jicama/devices.h>
#include <jicama/console.h>
#include <jicama/iomux.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

int current_taskno()
{
	thread_t *thread;
	
	thread = current_thread();
	//nr=proc_number(thread->plink);
	return proc_number(thread->plink);
}

__local struct _export_table_entry msvcrt_sym_table []=
{
	/*file system*/
	EXPORT_PC_SYMBOL(modfs_open),
	EXPORT_PC_SYMBOL(fseek),
	EXPORT_PC_SYMBOL(modfs_add),
	EXPORT_PC_SYMBOL(fread),
	EXPORT_PC_SYMBOL(fclose),
	EXPORT_PC_SYMBOL(unregister_fs),
	EXPORT_PC_SYMBOL(select_fs),
	EXPORT_PC_SYMBOL(register_fs),

		/*other*/
	EXPORT_PC_SYMBOL(do_gettime),
	EXPORT_PC_SYMBOL(get_unix_time),
	EXPORT_PC_SYMBOL(to_unix_time),
	EXPORT_PC_SYMBOL(get_unix_sec),
	EXPORT_PC_SYMBOL(startup_ticks),

	//
	EXPORT_PC_SYMBOL(init_ioobject),
	EXPORT_PC_SYMBOL(detach_ioobject),
	EXPORT_PC_SYMBOL(set_io_event),
	EXPORT_PC_SYMBOL(clear_io_event),

	/*string libary*/
	EXPORT_PC_SYMBOL(sprintf),
	EXPORT_PC_SYMBOL(snprintf),
	EXPORT_PC_SYMBOL(strcmp),
	EXPORT_PC_SYMBOL(strcpy),
	EXPORT_PC_SYMBOL(memmove),
	EXPORT_PC_SYMBOL(strncpy),
	EXPORT_PC_SYMBOL(strtok),
	EXPORT_PC_SYMBOL(memset),
	EXPORT_PC_SYMBOL(memcpy),
	EXPORT_PC_SYMBOL(strlen),
	EXPORT_PC_SYMBOL(strcat),
	EXPORT_PC_SYMBOL(strncmp),
	EXPORT_PC_SYMBOL(stricmp),
	EXPORT_PC_SYMBOL(memcmp),
	EXPORT_PC_SYMBOL(strnicmp),
	EXPORT_PC_SYMBOL(bzero),
	EXPORT_PC_SYMBOL(strchr),

	EXPORT_PC_SYMBOL(rand),
	EXPORT_PC_SYMBOL(srand),
};

#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>

__local struct _export_table_entry kernel_sym_table []=
{
	/*kernel task*/

#if 1
	EXPORT_PC_SYMBOL(driver_module_handler),
	EXPORT_PC_SYMBOL(bus_teardown_intr),

	EXPORT_PC_SYMBOL(device_get_top),
	EXPORT_PC_SYMBOL(device_add_child),
	EXPORT_PC_SYMBOL(device_add_child_ordered),
	EXPORT_PC_SYMBOL(device_busy),
	EXPORT_PC_SYMBOL(device_delete_child),
	EXPORT_PC_SYMBOL(device_attach),
	EXPORT_PC_SYMBOL(device_detach),
	EXPORT_PC_SYMBOL(device_disable),
	EXPORT_PC_SYMBOL(device_enable),
	EXPORT_PC_SYMBOL(device_find_child),
	EXPORT_PC_SYMBOL(device_get_desc),
	EXPORT_PC_SYMBOL(device_get_devclass),
	EXPORT_PC_SYMBOL(device_get_driver),
	EXPORT_PC_SYMBOL(device_get_flags),
	EXPORT_PC_SYMBOL(device_get_parent),
	EXPORT_PC_SYMBOL(device_get_children),
	EXPORT_PC_SYMBOL(device_get_ivars),
	EXPORT_PC_SYMBOL(device_set_ivars),
	EXPORT_PC_SYMBOL(device_get_name),
	EXPORT_PC_SYMBOL(device_get_nameunit),
	EXPORT_PC_SYMBOL(device_get_softc),

	EXPORT_PC_SYMBOL(device_get_state),
	EXPORT_PC_SYMBOL(device_get_unit),
	EXPORT_PC_SYMBOL(device_is_alive),
	EXPORT_PC_SYMBOL(device_is_attached),
	EXPORT_PC_SYMBOL(device_is_enabled),
	EXPORT_PC_SYMBOL(device_is_quiet),
	EXPORT_PC_SYMBOL(device_print_prettyname),
	EXPORT_PC_SYMBOL(device_probe_and_attach),
	EXPORT_PC_SYMBOL(device_quiet),
	EXPORT_PC_SYMBOL(device_set_desc),
	EXPORT_PC_SYMBOL(device_set_desc_copy),
	EXPORT_PC_SYMBOL(device_set_devclass),
	EXPORT_PC_SYMBOL(device_set_driver),
	EXPORT_PC_SYMBOL(device_set_flags),
	EXPORT_PC_SYMBOL(device_set_softc),
	EXPORT_PC_SYMBOL(device_set_unit),
	EXPORT_PC_SYMBOL(device_shutdown),
	EXPORT_PC_SYMBOL(device_unbusy),
	EXPORT_PC_SYMBOL(device_verbose),
	EXPORT_PC_SYMBOL(devclass_add_driver),
	EXPORT_PC_SYMBOL(devclass_delete_driver),
	EXPORT_PC_SYMBOL(devclass_create),
	EXPORT_PC_SYMBOL(devclass_find),
	EXPORT_PC_SYMBOL(devclass_find_driver),
	EXPORT_PC_SYMBOL(devclass_get_name),
	EXPORT_PC_SYMBOL(devclass_get_device),
	EXPORT_PC_SYMBOL(devclass_get_softc),
	EXPORT_PC_SYMBOL(devclass_get_devices),
	EXPORT_PC_SYMBOL(devclass_get_count),
	EXPORT_PC_SYMBOL(devclass_get_maxunit),
	EXPORT_PC_SYMBOL(devclass_find_free_unit),
	EXPORT_PC_SYMBOL(devclass_set_parent),
	EXPORT_PC_SYMBOL(devclass_get_parent),

	EXPORT_PC_SYMBOL(bus_generic_detach),
	EXPORT_PC_SYMBOL(bus_generic_print_child),
	EXPORT_PC_SYMBOL(bus_generic_resume),
	EXPORT_PC_SYMBOL(bus_generic_shutdown),
	EXPORT_PC_SYMBOL(bus_generic_suspend),
	EXPORT_PC_SYMBOL(bus_generic_attach),
	EXPORT_PC_SYMBOL(bus_teardown_intr),
	/*EXPORT_PC_SYMBOL(),
	EXPORT_PC_SYMBOL(),*/
#endif

	/*module*/

	EXPORT_PC_SYMBOL(module_find_secton),
	EXPORT_PC_SYMBOL(install_dll_table),
	EXPORT_PC_SYMBOL(remove_dll_table),	
	EXPORT_PC_SYMBOL(new_kernel_thread),	
	EXPORT_PC_SYMBOL(remove_kernel_thread),	

	EXPORT_PC_SYMBOL(register_proc_entry),	
	EXPORT_PC_SYMBOL(unregister_proc_entry),	
	EXPORT_PC_SYMBOL(pprintf),	
	EXPORT_PC_SYMBOL(read_proc),	
	EXPORT_PC_SYMBOL(write_proc),	
	EXPORT_PC_SYMBOL(ls_proc),	
	EXPORT_PC_SYMBOL(proc_get_header),	


	//EXPORT_PC_SYMBOL(inportb),
	//EXPORT_PC_SYMBOL(outportb),
	EXPORT_PC_SYMBOL(disable),
	EXPORT_PC_SYMBOL(enable),


		/*devices driver*/
	EXPORT_PC_SYMBOL(sigmaskset),
	EXPORT_PC_SYMBOL(sigmaskdel),
	EXPORT_PC_SYMBOL(sigmasked),
	EXPORT_PC_SYMBOL(current_thread),
	//EXPORT_PC_SYMBOL(thread_unready),
	//{"thread_unready", thread_wait},
	{"thread_exit", kernel_thread_exit},/*will delete*/
	EXPORT_PC_SYMBOL(kernel_thread_exit),
	EXPORT_PC_SYMBOL(thread_wait),
	EXPORT_PC_SYMBOL(thread_ready),
	EXPORT_PC_SYMBOL(current_thread_id),
	EXPORT_PC_SYMBOL(set_thread_name),
	EXPORT_PC_SYMBOL(set_thread_priority),
	EXPORT_PC_SYMBOL(schedule),
	EXPORT_PC_SYMBOL(current_taskno),
	EXPORT_PC_SYMBOL(get_kernel_command),
	EXPORT_PC_SYMBOL(panic),
	EXPORT_PC_SYMBOL(kprintf), 
	{"printk", kprintf},
	EXPORT_PC_SYMBOL(puts),
	EXPORT_PC_SYMBOL(vsprintf),
	EXPORT_PC_SYMBOL(syslog), 

	EXPORT_PC_SYMBOL(kernel_driver_register),
	EXPORT_PC_SYMBOL(kernel_driver_unregister),
	EXPORT_PC_SYMBOL(dev_driver_header),
	EXPORT_PC_SYMBOL(dev_read),
	EXPORT_PC_SYMBOL(dev_write),
	EXPORT_PC_SYMBOL(dev_open),
	EXPORT_PC_SYMBOL(dev_ioctl),
	EXPORT_PC_SYMBOL(dev_close),
	EXPORT_PC_SYMBOL(dev_receive),
	EXPORT_PC_SYMBOL(dev_transmit),
	EXPORT_PC_SYMBOL(dev_attach),
	EXPORT_PC_SYMBOL(dev_detach),
	EXPORT_PC_SYMBOL(key_cook),

	EXPORT_PC_SYMBOL(put_irq_handler),
	EXPORT_PC_SYMBOL(dis_irq),
	EXPORT_PC_SYMBOL(en_irq),
	EXPORT_PC_SYMBOL(free_irq_handler),
	EXPORT_PC_SYMBOL(select_console),
	EXPORT_PC_SYMBOL(regster_console_device),
	EXPORT_PC_SYMBOL(unregster_console_device),

	EXPORT_PC_SYMBOL(busman_register),
	EXPORT_PC_SYMBOL(busman_get),
	EXPORT_PC_SYMBOL(busman_unregister),

	EXPORT_PC_SYMBOL(init_timer),
	EXPORT_PC_SYMBOL(restart_timer),
	EXPORT_PC_SYMBOL(mod_timer),
	EXPORT_PC_SYMBOL(install_timer),
	EXPORT_PC_SYMBOL(remove_timer),

		/*mm*/

	//EXPORT_PC_SYMBOL(mm_malloc),
	//EXPORT_PC_SYMBOL(mm_free),
	EXPORT_PC_SYMBOL(kmalloc),
	EXPORT_PC_SYMBOL(kcalloc),
	EXPORT_PC_SYMBOL(kfree),
	EXPORT_PC_SYMBOL(ksize),
	EXPORT_PC_SYMBOL(get_page),
	EXPORT_PC_SYMBOL(free_page),
	EXPORT_PC_SYMBOL(mem_writeable),
	EXPORT_PC_SYMBOL(map_high_memory),
	EXPORT_PC_SYMBOL(map_physical_memory_nocache),
	EXPORT_PC_SYMBOL(page_vtophys),
	EXPORT_PC_SYMBOL(virt2phys),

		
	
	//input
	EXPORT_PC_SYMBOL(submit_input_enent),
	EXPORT_PC_SYMBOL(set_input_manager),
	EXPORT_PC_SYMBOL(reset_input_manager),
	EXPORT_PC_SYMBOL(post_thread_message),
	EXPORT_PC_SYMBOL(get_thread_message),
	EXPORT_PC_SYMBOL(get_thread_msgport),
	EXPORT_PC_SYMBOL(find_thread_byid),
	EXPORT_PC_SYMBOL(current_proc_vir2phys),
	EXPORT_PC_SYMBOL(sendsig),
	EXPORT_PC_SYMBOL(setup_unix_socketcall),
	EXPORT_PC_SYMBOL(sigrecv),
	{"findsig", sigrecv},/*will delete*/
	{"cur_addr_insystem", current_proc_vir2phys},/*will delete*/
	EXPORT_PC_SYMBOL(sigdelset),
	EXPORT_PC_SYMBOL(find_thread),
	
	EXPORT_PC_SYMBOL(thread_waitq_init),
	EXPORT_PC_SYMBOL(thread_waitq_empty),
	EXPORT_PC_SYMBOL(thread_wakeup),
	EXPORT_PC_SYMBOL(thread_sleep_on),
	//EXPORT_PC_SYMBOL(sleep),
	//{"proc_sleep", sleep},/*will delete*/

	//信号量操作
	EXPORT_PC_SYMBOL(create_semaphore),
	EXPORT_PC_SYMBOL(trylock_semaphore),
	EXPORT_PC_SYMBOL(lock_semaphore),
	EXPORT_PC_SYMBOL(lock_semaphore_timeout),
	EXPORT_PC_SYMBOL(unlock_semaphore),
	EXPORT_PC_SYMBOL(unlock_semaphore_ex),
	EXPORT_PC_SYMBOL(destroy_semaphore),

	//socket
	//EXPORT_PC_SYMBOL(socketcall_handler),

		//消息端口操作
	EXPORT_PC_SYMBOL(create_msgport),
	EXPORT_PC_SYMBOL(connect_msgport),
	EXPORT_PC_SYMBOL(msgport_send),
	EXPORT_PC_SYMBOL(msgport_pend),
	EXPORT_PC_SYMBOL(msgport_set_limit),
	//EXPORT_PC_SYMBOL(free_msg_buf),
	EXPORT_PC_SYMBOL(msgport_destroy),
	EXPORT_PC_SYMBOL(msgport_wait),

		//in queue
	EXPORT_PC_SYMBOL(deq),
	EXPORT_PC_SYMBOL(inq),
	
		/*kernel data*/
	EXPORT_DATA_SYMBOL(boot_parameters),
};

__asmlink void reset_dll_table();
__asmlink  int queue_pkt_sym_setup();
__asmlink struct _export_table_entry arch_sym_table[] ;

void sym_init()
{
	int ksym_nr, msvcrt_nr;

	reset_dll_table();

	ksym_nr =sizeof(kernel_sym_table)/sizeof(struct _export_table_entry);
	msvcrt_nr =sizeof(msvcrt_sym_table)/sizeof(struct _export_table_entry);

	install_dll_table("kernel.krnl", 1,ksym_nr, kernel_sym_table);
	install_dll_table("msvcrt.krnl", 1,msvcrt_nr, msvcrt_sym_table);
	arch_sym_setup();
	queue_pkt_sym_setup();
}


__public char *get_module_sym_name(int index)
{
	switch (index)
	{
		case 0: return "_dll_main";
		case 1: return "_dll_destroy";
		//case 3: return "__static_initialization_and_destruction_0";
		default:
		break;
	}
	return "bad sym";
}

