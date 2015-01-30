#include <stdio.h>

struct sysinit {
	unsigned int	subsystem;		/* subsystem identifier*/
	unsigned int	order;			/* init order within subsystem*/
	void		(*func) __P((void *));	/* init function*/
	void		*udata;			/* multiplexer/argument */
	si_elem_t	type;			/* sysinit_elem_type*/
};

typedef	int (*modeventhand_t)(void* mod, int /*modeventtype_t*/ what,
			      void *arg);


struct driver_module_data {
        int             (*dmd_chainevh)(struct module *, int, void *);
        void            *dmd_chainarg;
        const char      *dmd_busname;
        driver_t*    dmd_driver;
        devclass_t      *dmd_devclass;
};




typedef struct moduledata {
        char *name;
        modeventhand_t evhand;
        void *priv;
        void *_file;
} moduledata_t;



#define MAKE_SET(set, sym)						\
	static void const * const __set_##set##_sym_##sym = &sym;	\
	__asm(".section .set." #set ",\"aw\"");				\
	__asm(".long " #sym);						\
	__asm(".previous")
#endif
#define TEXT_SET(set, sym) MAKE_SET(set, sym)
#define DATA_SET(set, sym) MAKE_SET(set, sym)
#define BSS_SET(set, sym)  MAKE_SET(set, sym)
#define ABS_SET(set, sym)  MAKE_SET(set, sym)



int driver_module_handler(void*mod, int what, void *arg);

#define DRIVER_MODULE(name, busname, driver, devclass, evh, arg)        \
                                                                        \
static struct driver_module_data name##_##busname##_driver_mod = {      \
        evh, arg,                                                       \
        #busname,                                                       \
        (void*) &driver,                                         \
        &devclass                                                       \
};                                                                      \
                                                                        \
moduledata_t name##_##busname##_mod __attribute__( ( section( ".driver_module" ) ) ) = {                          \
        #busname "/" #name,                                             \
        driver_module_handler,                                          \
        &name##_##busname##_driver_mod                                  \
};      

DRIVER_MODULE(uhci, pci, uhci_driver, ohci_devclass, mytest, hello);

int main(int argc, char *argv[])
{
	printf("Hello, world\n");
	
	return 0;
}
