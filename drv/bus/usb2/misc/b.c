# 1 "a.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "a.c"


struct sysinit {
        unsigned int subsystem;
        unsigned int order;
        void (*func) __P((void *));
        void *udata;
        si_elem_t type;
};

typedef int (*modeventhand_t)(void* mod, int what,
                              void *arg);


struct driver_module_data {
        int (*dmd_chainevh)(struct module *, int, void *);
        void *dmd_chainarg;
        const char *dmd_busname;
        driver_t* dmd_driver;
        devclass_t *dmd_devclass;
};




typedef struct moduledata {
        char *name;
        modeventhand_t evhand;
        void *priv;
        void *_file;
} moduledata_t;
# 48 "a.c"
int driver_module_handler(void*mod, int what, void *arg);
# 65 "a.c"
static struct driver_module_data uhci_pci_driver_mod = { mytest, hello, "pci", (void*) &uhci_driver, &ohci_devclass }; moduledata_t uhci_pci_mod __attribute__( ( section( ".driver_module" ) ) ) = { "pci" "/" "uhci", driver_module_handler, &uhci_pci_driver_mod };;

int main(int argc, char *argv[])
{
        printf("Hello, world\n");

        return 0;
}
