#ifndef _USB_FREEBSD_COMPAT_H_
#define	_USB_FREEBSD_COMPAT_H_



#define TRACE_HERE do{\
	kprintf("File %s:Got %s() line%d\n",__FILE__,__FUNCTION__,__LINE__);\
}\
while (0)

#define device_printf(N,format, argc...) 	do{\
kprintf(format, ##argc)	;\
}\
while (0)\

typedef void *usb_malloc_type;
typedef void *caddr_t;
typedef u8_t u_int8_t;
typedef u16_t u_int16_t;
typedef u32_t u_int32_t;
typedef	unsigned char	u_char;
typedef	unsigned short	u_short;
typedef	unsigned int	u_int;
typedef	unsigned long	u_long;
typedef	unsigned short	ushort;		/* Sys V compatibility */


typedef unsigned long long u_int64_t;
#define __packed __attribute__((packed)) 
#define __aligned(x)	__attribute__((__aligned__(x)))

/*
 * Forward declarations
 */
typedef struct device		*device_t;
typedef struct driver		driver_t;
typedef struct device_method	device_method_t;
typedef struct devclass		*devclass_t;
typedef struct device_ops	*device_ops_t;
typedef struct device_op_desc	*device_op_desc_t;
typedef void* bus_space_tag_t;
typedef void* bus_space_handle_t;
typedef void* bus_size_t;
typedef void driver_intr_t(void*);

enum{
	BUS_SPACE_IO,
	BUS_SPACE_MEM,
};

/*
 * We define this in terms of bits because some devices may belong
 * to multiple classes (and therefore need to be included in
 * multiple interrupt masks, which is what this really serves to
 * indicate.  Buses which do interrupt remapping will want to
 * change their type to reflect what sort of devices are underneath.
 */
typedef enum driver_type {
    DRIVER_TYPE_TTY = 1,
    DRIVER_TYPE_BIO = 2,
    DRIVER_TYPE_NET = 4,
    DRIVER_TYPE_CAM = 8,
    DRIVER_TYPE_MISC = 16,
    DRIVER_TYPE_FAST = 128
} driver_type_t;

enum{
	MOD_LOAD,
	MOD_UNLOAD
};


typedef int (*devop_t)(device_t);

struct device_op_desc {
    unsigned int        offset; /* offset in driver ops */
    //struct method*      method; /* internal method implementation */
    devop_t             deflt;  /* default implementation */
    const char*         name;   /* unique name (for registration) */
};

/*
 * Forward declarations
 */
typedef TAILQ_HEAD(devclass_list, devclass) devclass_list_t;
typedef TAILQ_HEAD(driver_list, driverlink) driver_list_t;
typedef TAILQ_HEAD(device_list, device) device_list_t;

typedef enum device_state {
        DS_NOTPRESENT,                  /* not probed or probe failed */
        DS_ALIVE,                       /* probe succeeded */
        DS_ATTACHED,                    /* attach method called */
        DS_BUSY                         /* device is open */
} device_state_t;


struct device {
 /*
         * Device hierarchy.
         */
        TAILQ_ENTRY(device)     link;   /**< list of devices in parent */
        TAILQ_ENTRY(device)     devlink; /**< global device list membership */
        device_t        parent;         /**< parent of this device  */
        device_list_t   children;       /**< list of child devices */

        /*
         * Details of this device.
         */
        driver_t        *driver;        /**< current driver */
        devclass_t      devclass;       /**< current device class */
        int             unit;           /**< current unit number */
        char*           nameunit;       /**< name+unit e.g. foodev0 */
        char*           desc;           /**< driver specific description */
        int             busy;           /**< count of calls to device_busy() */
        int  state;          /**< current device state  */
        u_int32_t       devflags;       /**< api level flags for device_get_flags() */
        u16_t         flags;          /**< internal device flags  */
#define DF_ENABLED      1               /* device should be probed/attached */
#define DF_FIXEDCLASS   2               /* devclass specified at create time */
#define DF_WILDCARD     4               /* unit was originally wildcard */
#define DF_DESCMALLOCED 8               /* description was malloced */
#define DF_QUIET        16              /* don't print verbose attach message */
#define DF_DONENOMATCH  32              /* don't execute DEVICE_NOMATCH again */
#define DF_EXTERNALSOFTC 64             /* softc not allocated by us */
        u8_t  order;                  /**< order from device_add_child_ordered() */
        u8_t  pad;
        void    *ivars;                 /**< instance variables  */
        void    *softc;                 /**< current driver's variables  */

	char name[64];
	
};

typedef	int (*modeventhand_t)(void* mod, int /*modeventtype_t*/ what,
			      void *arg);


struct driver_module_data {
        int             (*dmd_chainevh)(void *, int, void *);
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

#define KDRV_SECTON ".kdrv"

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
moduledata_t name##_##busname##_mod __attribute__( ( section( KDRV_SECTON ) ) ) = {                          \
        #busname "/" #name,                                             \
        driver_module_handler,                                          \
        &name##_##busname##_driver_mod                                  \
};                                                                      \

struct device_method {
 #define DEVMETHOD(NAME, FUNC) { #NAME, (devop_t) FUNC }
    char* desc;
    devop_t		func;
};

struct driver {
    const char		*name;		/* driver name */
    device_method_t	*methods;	/* method table */
	int size;
    driver_type_t	type;
    size_t		softc;		/* size of device softc struct */
    void		*priv;		/* driver private data */
    LIST_ENTRY(driver) link;		/* list of devices on bus */
    device_ops_t	ops;		/* compiled method table */
	int refs;
	void *prvi;
};

#define MODULE_VERSION(a,b)
#define MODULE_DEPEND(a,b,c,d,e)
typedef void* kobj_class_t;



/*
 * Access functions for device.
 */
device_t        device_add_child(device_t dev, const char *name, int unit);
device_t        device_add_child_ordered(device_t dev, int order,
                                         const char *name, int unit);
void    device_busy(device_t dev);
int     device_delete_child(device_t dev, device_t child);
int     device_attach(device_t dev);
int     device_detach(device_t dev);
void    device_disable(device_t dev);
void    device_enable(device_t dev);
device_t        device_find_child(device_t dev, const char *classname,
                                  int unit);
const char      *device_get_desc(device_t dev);
devclass_t      device_get_devclass(device_t dev);
driver_t        *device_get_driver(device_t dev);
u_int32_t       device_get_flags(device_t dev);
device_t        device_get_parent(device_t dev);
int     device_get_children(device_t dev, device_t **listp, int *countp);
void    *device_get_ivars(device_t dev);
void    device_set_ivars(device_t dev, void *ivars);
const   char *device_get_name(device_t dev);
const   char *device_get_nameunit(device_t dev);
void    *device_get_softc(device_t dev);
device_state_t  device_get_state(device_t dev);
int     device_get_unit(device_t dev);
int     device_is_alive(device_t dev);  /* did probe succeed? */
int     device_is_attached(device_t dev);       /* did attach succeed? */
int     device_is_enabled(device_t dev);
int     device_is_quiet(device_t dev);
int     device_print_prettyname(device_t dev);
int     device_probe_and_attach(device_t dev);
void    device_quiet(device_t dev);
void    device_set_desc(device_t dev, const char* desc);
void    device_set_desc_copy(device_t dev, const char* desc);
int     device_set_devclass(device_t dev, const char *classname);
int     device_set_driver(device_t dev, driver_t *driver);
void    device_set_flags(device_t dev, u_int32_t flags);
void    device_set_softc(device_t dev, void *softc);
int     device_set_unit(device_t dev, int unit);        /* XXX DONT USE XXX */
int     device_shutdown(device_t dev);
void    device_unbusy(device_t dev);
void    device_verbose(device_t dev);
device_t device_get_top();

/*
 * Access functions for devclass.
 */
int     devclass_add_driver(devclass_t dc, kobj_class_t driver);
int     devclass_delete_driver(devclass_t dc, kobj_class_t driver);
devclass_t      devclass_create(const char *classname);
devclass_t      devclass_find(const char *classname);
kobj_class_t    devclass_find_driver(devclass_t dc, const char *classname);
const char      *devclass_get_name(devclass_t dc);
device_t        devclass_get_device(devclass_t dc, int unit);
void    *devclass_get_softc(devclass_t dc, int unit);
int     devclass_get_devices(devclass_t dc, device_t **listp, int *countp);
int     devclass_get_count(devclass_t dc);
int     devclass_get_maxunit(devclass_t dc);
int     devclass_find_free_unit(devclass_t dc, int unit);
void    devclass_set_parent(devclass_t dc, devclass_t pdc);
devclass_t      devclass_get_parent(devclass_t dc);


int	bus_generic_detach(device_t dev);
int	bus_generic_print_child(device_t dev, device_t child);
int	bus_generic_resume(device_t dev);

int	bus_generic_shutdown(device_t dev);
int	bus_generic_suspend(device_t dev);
int bus_generic_attach(device_t dev);
int bus_teardown_intr(device_t dev, void *r, void *cookie);
#endif //
