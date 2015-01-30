#include <jicama/system.h>
#include <jicama/fs.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <string.h>

#include <sys/queue.h>
#include <sys/compat_bsd.h>
#include <sys/endian.h>

TAILQ_HEAD(,device)     bus_data_devices;
devop_t get_bsd_driver_ops(struct driver* driver,const char* ops);
devclass_t devclass_find_internal(const char *classname, const char *parentname,
                       int create);
static int
device_probe_child(device_t dev, device_t child);

#define PDEBUG(a)       //{kprintf("%s:%d: ", __func__, __LINE__), kprintf a; kprintf("\n");}
#define DEVICENAME(d)   ((d)? device_get_name(d): "no device")
#define DRIVERNAME(d)   ((d)? d->name : "no driver")
#define DEVCLANAME(d)   ((d)? d->name : "no devclass")

#define MINBUCKET       4
#define MINALLOCSIZE    (1 << MINBUCKET)
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))

#define DEVICE_PROBE(n) dev_method((n),"device_probe")
#define DEVICE_ATTACH(n) dev_method((n),"device_attach")
#define DEVICE_RESUME(n) dev_method((n),"device_resume")
#define DEVICE_SHUTDOWN(n) dev_method((n),"device_shutdown")
#define DEVICE_DETACH(n) dev_method((n),"device_detach")
#define BUS_PROBE_NOMATCH(n,m) //dev_method((n),"device_nomatch") //fixme
#define DEVICE_IDENTIFY(n,m) do_dev_method((n),(m),"device_identify")
#define DEVICE_SUSPEND(n) dev_method((n),"device_suspend")
typedef struct driverlink *driverlink_t;
struct driverlink {
        driver_t*    driver;
        TAILQ_ENTRY(driverlink) link;   /* list of drivers in devclass */
};
static	int bootverbose=0;


struct devclass {
        TAILQ_ENTRY(devclass) link;
        devclass_t      parent;         /* parent in devclass hierarchy */
        driver_list_t   drivers;     /* bus devclasses store drivers for bus */
        char            *name;
        device_t        *devices;       /* array of devices indexed by unit */
        int             maxunit;        /* size of devices array */

        //struct sysctl_ctx_list sysctl_ctx;
        //struct sysctl_oid *sysctl_tree;
};
static devclass_list_t devclasses = TAILQ_HEAD_INITIALIZER(devclasses);



static inline int do_dev_method(driver_t *driver,struct device *dev,char *name)
{
	int err;
	devop_t ops;	

	ops = get_bsd_driver_ops(driver,name);
	if (!ops)
	{
		return -1;
	}
	err = (*ops)(dev);	
	return err;
}


static inline int dev_method(struct device *dev,char *name)
{	
	return do_dev_method(dev->driver,dev,name);
}

/*static int dev_probe(struct driver*driver,struct device *dev )
{
	int err;
	devop_t ops;	

	ops = get_bsd_driver_ops(driver,"device_probe");
	if (!ops)
	{
		return -2;
	}
	err = ops(dev);

	if (err<0)
	{
		return err;
	}

	ops = get_bsd_driver_ops(driver,"device_attach");
	if (!ops)
	{
		return -3;
	}
	err = ops(dev);
	return err;
}
*/




devop_t get_bsd_driver_ops(struct driver* driver,const char* ops)
{
	device_method_t *methods= driver->methods;

	for (methods; methods->desc; methods++)
	{
		if (strcmp(methods->desc,ops)==0)
		{
			return methods->func;
		}
	}
	
	return 0;	
}


static int
devclass_alloc_unit(devclass_t dc, int *unitp)
{
        int unit = *unitp;

        PDEBUG(("unit %d in devclass %s", unit, DEVCLANAME(dc)));

        /* If we were given a wired unit number, check for existing device */
        /* XXX imp XXX */
        if (unit != -1) {
                if (unit >= 0 && unit < dc->maxunit &&
                    dc->devices[unit] != NULL) {
                        if (bootverbose)
                                kprintf("%s: %s%d already exists; skipping it\n",
                                    dc->name, dc->name, *unitp);
                        return (EEXIST);
                }
        } else {
                /* Unwired device, find the next available slot for it */
                unit = 0;
                while (unit < dc->maxunit && dc->devices[unit] != NULL)
                        unit++;
        }

        /*
         * We've selected a unit beyond the length of the table, so let's
         * extend the table to make room for all units up to and including
         * this one.
         */
        if (unit >= dc->maxunit) {
                device_t *newlist;
                int newsize;

                newsize = roundup((unit + 1), MINALLOCSIZE / sizeof(device_t));
                newlist = kcalloc(sizeof(device_t) * newsize);
                if (!newlist)
                        return (ENOMEM);
                memcpy(dc->devices, newlist, sizeof(device_t) * dc->maxunit);
                bzero(newlist + dc->maxunit,
                    sizeof(device_t) * (newsize - dc->maxunit));
                if (dc->devices)
                        kfree(dc->devices);
                dc->devices = newlist;
                dc->maxunit = newsize;
        }
        PDEBUG(("now: unit %d in devclass %s", unit, DEVCLANAME(dc)));

        *unitp = unit;
        return (0);
}


/**
 * @internal
 * @brief Add a device to a devclass
 *
 * A unit number is allocated for the device (using the device's
 * preferred unit number if any) and the device is registered in the
 * devclass. This allows the device to be looked up by its unit
 * number, e.g. by decoding a dev_t minor number.
 *
 * @param dc            the devclass to add to
 * @param dev           the device to add
 *
 * @retval 0            success
 * @retval EEXIST       the requested unit number is already allocated
 * @retval ENOMEM       memory allocation failure
 */
static int
devclass_add_device(devclass_t dc, device_t dev)
{
        int buflen, error;

        PDEBUG(("%s in devclass %s", DEVICENAME(dev), DEVCLANAME(dc)));

        buflen = snprintf(NULL, 0, "%s%d$", dc->name, dev->unit);
        if (buflen < 0)
                return (ENOMEM);
        dev->nameunit = kcalloc(buflen);
        if (!dev->nameunit)
                return (ENOMEM);

        if ((error = devclass_alloc_unit(dc, &dev->unit)) != 0) {
                kfree(dev->nameunit);
                dev->nameunit = NULL;
                return (error);
        }
        dc->devices[dev->unit] = dev;
        dev->devclass = dc;
        snprintf(dev->nameunit, buflen, "%s%d", dc->name, dev->unit);

        return (0);
}

static __inline void BUS_DRIVER_ADDED(device_t _dev, driver_t *_driver)
{
	//TRACE_HERE;
        //kobjop_t _m;
        //KOBJOPLOOKUP(((kobj_t)_dev)->ops,bus_driver_added);
        //((bus_driver_added_t *) _m)(_dev, _driver);
}


int
devclass_add_driver(devclass_t dc,kobj_class_t drv)
{
	driver_t *driver=drv;
        driverlink_t dl;
        int i;

        PDEBUG(("%s", DRIVERNAME(driver)));

        dl = kcalloc(sizeof *dl);
        if (!dl)
                return (ENOMEM);

        /*
         * Compile the driver's methods. Also increase the reference count
         * so that the class doesn't get freed when the last instance
         * goes. This means we can safely use static methods and avoids a
         * double-free in devclass_delete_driver.
         */
       // kobj_class_compile((kobj_class_t) driver);

        /*
         * Make sure the devclass which the driver is implementing exists.
         */
        devclass_find_internal(driver->name, 0, TRUE);

        dl->driver = driver;
        TAILQ_INSERT_TAIL(&dc->drivers, dl, link);
        //driver->refs++;

        /*
         * Call BUS_DRIVER_ADDED for any existing busses in this class.
         */
        for (i = 0; i < dc->maxunit; i++)
                if (dc->devices[i]){
			#if 1
				kprintf("BUS_DRIVER_ADDED %p %d\n",dc->devices[i],i);
			#else
                        BUS_DRIVER_ADDED(dc->devices[i], driver);
			#endif
		}

        //bus_data_generation_update();
        return (0);
}


/**
 * @internal
 * @brief Delete a device from a devclass
 *
 * The device is removed from the devclass's device list and its unit
 * number is freed.

 * @param dc            the devclass to delete from
 * @param dev           the device to delete
 *
 * @retval 0            success
 */
static int
devclass_delete_device(devclass_t dc, device_t dev)
{
        if (!dc || !dev)
                return (0);

        PDEBUG(("%s in devclass %s", DEVICENAME(dev), DEVCLANAME(dc)));

        if (dev->devclass != dc || dc->devices[dev->unit] != dev)
                panic("devclass_delete_device: inconsistent device class");
        dc->devices[dev->unit] = NULL;
        if (dev->flags & DF_WILDCARD)
                dev->unit = -1;
        dev->devclass = NULL;
        kfree(dev->nameunit);
        dev->nameunit = NULL;

        return (0);
}


/**
 * @brief Delete a device driver from a device class
 *
 * Delete a device driver from a devclass. This is normally called
 * automatically by DRIVER_MODULE().
 *
 * If the driver is currently attached to any devices,
 * devclass_delete_driver() will first attempt to detach from each
 * device. If one of the detach calls fails, the driver will not be
 * deleted.
 *
 * @param dc            the devclass to edit
 * @param driver        the driver to unregister
 */
int
devclass_delete_driver(devclass_t busclass, kobj_class_t drv)
{
	driver_t *driver = drv;
        devclass_t dc = devclass_find(driver->name);
        driverlink_t dl;
        device_t dev;
        int i;
        int error;

        PDEBUG(("%s from devclass %s", driver->name, DEVCLANAME(busclass)));

        if (!dc)
                return (0);

        /*
         * Find the link structure in the bus' list of drivers.
         */
        TAILQ_FOREACH(dl, &busclass->drivers, link) {
                if (dl->driver == driver)
                        break;
        }

        if (!dl) {
                PDEBUG(("%s not found in %s list", driver->name,
                    busclass->name));
                return (ENOENT);
        }

        /*
         * Disassociate from any devices.  We iterate through all the
         * devices in the devclass of the driver and detach any which are
         * using the driver and which have a parent in the devclass which
         * we are deleting from.
         *
         * Note that since a driver can be in multiple devclasses, we
         * should not detach devices which are not children of devices in
         * the affected devclass.
         */
        for (i = 0; i < dc->maxunit; i++) {
                if (dc->devices[i]) {
                        dev = dc->devices[i];
                        if (dev->driver == driver && dev->parent &&
                            dev->parent->devclass == busclass) {
                                if ((error = device_detach(dev)) != 0)
                                        return (error);
                                device_set_driver(dev, NULL);
                        }
                }
        }

        TAILQ_REMOVE(&busclass->drivers, dl, link);
        kfree(dl);

        //driver->refs--;
        //if (driver->refs == 0)
         //       kobj_class_free((kobj_class_t) driver);

        //bus_data_generation_update();
        return (0);
}

/**
 * @internal
 */
static driverlink_t
devclass_find_driver_internal(devclass_t dc, const char *classname)
{
        driverlink_t dl;

        PDEBUG(("devclass_find_driver_internal %s in devclass %s", classname, DEVCLANAME(dc)));

        TAILQ_FOREACH(dl, &dc->drivers, link) {
			PDEBUG((" found:%s",dl->driver->name));
                if (!strcmp(dl->driver->name, classname))
                        return (dl);
        }

        PDEBUG(("not found"));
        return (NULL);
}

/**
 * @brief Search a devclass for a driver
 *
 * This function searches the devclass's list of drivers and returns
 * the first driver whose name is @p classname or @c NULL if there is
 * no driver of that name.
 *
 * @param dc            the devclass to search
 * @param classname     the driver name to search for
 */
void*
devclass_find_driver(devclass_t dc, const char *classname)
{
        driverlink_t dl;

        dl = devclass_find_driver_internal(dc, classname);
        if (dl)
                return (dl->driver);
        return (NULL);
}

/**
 * @brief Return the name of the devclass
 */
const char *
devclass_get_name(devclass_t dc)
{
        return (dc->name);
}



/**
 * @brief Find a device given a unit number
 *
 * @param dc            the devclass to search
 * @param unit          the unit number to search for
 *
 * @returns             the device with the given unit number or @c
 *                      NULL if there is no such device
 */
device_t
devclass_get_device(devclass_t dc, int unit)
{
        if (dc == NULL || unit < 0 || unit >= dc->maxunit)
                return (NULL);
        return (dc->devices[unit]);
}

/**
 * @brief Find the softc field of a device given a unit number
 *
 * @param dc            the devclass to search
 * @param unit          the unit number to search for
 *
 * @returns             the softc field of the device with the given
 *                      unit number or @c NULL if there is no such
 *                      device
 */
void *
devclass_get_softc(devclass_t dc, int unit)
{
        device_t dev;

        dev = devclass_get_device(dc, unit);
        if (!dev)
                return (NULL);

        return (device_get_softc(dev));
}

/**
 * @brief Get a list of devices in the devclass
 *
 * An array containing a list of all the devices in the given devclass
 * is allocated and returned in @p *devlistp. The number of devices
 * in the array is returned in @p *devcountp. The caller should free
 * the array using @c free(p, M_TEMP).
 *
 * @param dc            the devclass to examine
 * @param devlistp      points at location for array pointer return
 *                      value
 * @param devcountp     points at location for array size return value
 *
 * @retval 0            success
 * @retval ENOMEM       the array allocation failed
 */
int
devclass_get_devices(devclass_t dc, device_t **devlistp, int *devcountp)
{
        int count, i;
        device_t *list;

        count = devclass_get_count(dc);
        list = kcalloc(count * sizeof(device_t));
        if (!list)
                return (ENOMEM);

        count = 0;
        for (i = 0; i < dc->maxunit; i++) {
                if (dc->devices[i]) {
                        list[count] = dc->devices[i];
                        count++;
                }
        }

        *devlistp = list;
        *devcountp = count;

        return (0);
}

/**
 * @brief Get the number of devices in a devclass
 *
 * @param dc            the devclass to examine
 */
int
devclass_get_count(devclass_t dc)
{
        int count, i;

        count = 0;
        for (i = 0; i < dc->maxunit; i++)
                if (dc->devices[i])
                        count++;
        return (count);
}

/**
 * @brief Get the maximum unit number used in a devclass
 *
 * @param dc            the devclass to examine
 */
int
devclass_get_maxunit(devclass_t dc)
{
        return (dc->maxunit);
}

/**
 * @brief Find a free unit number in a devclass
 *
 * This function searches for the first unused unit number greater
 * that or equal to @p unit.
 *
 * @param dc            the devclass to examine
 * @param unit          the first unit number to check
 */
int
devclass_find_free_unit(devclass_t dc, int unit)
{
        if (dc == NULL)
                return (unit);
        while (unit < dc->maxunit && dc->devices[unit] != NULL)
                unit++;
        return (unit);
}

/**
 * @brief Set the parent of a devclass
 *
 * The parent class is normally initialised automatically by
 * DRIVER_MODULE().
 *
 * @param dc            the devclass to edit
 * @param pdc           the new parent devclass
 */
void
devclass_set_parent(devclass_t dc, devclass_t pdc)
{
        dc->parent = pdc;
}

/**
 * @brief Get the parent of a devclass
 *
 * @param dc            the devclass to examine
 */
devclass_t
devclass_get_parent(devclass_t dc)
{
        return (dc->parent);
}



devclass_t
devclass_find_internal(const char *classname, const char *parentname,
                       int create)
{
        devclass_t dc;

        if (!classname){
			PDEBUG(("looking for null"));
                return (NULL);
		}
        PDEBUG(("looking for %s", classname));
        TAILQ_FOREACH(dc, &devclasses, link) {
                if (!strcmp(dc->name, classname))
                        break;
        }

        if (create && !dc) {
                PDEBUG(("creating %s", classname));
                dc = kcalloc(sizeof(struct devclass) + strlen(classname) + 1);
                if (!dc)
                        return (NULL);
                dc->parent = NULL;
                dc->name = (char*) (dc + 1);
                strcpy(dc->name, classname);
                TAILQ_INIT(&dc->drivers);
                TAILQ_INSERT_TAIL(&devclasses, dc, link);

                //bus_data_generation_update();
        }
        if (parentname && dc && !dc->parent) {
                dc->parent = devclass_find_internal(parentname, 0, FALSE);
        }
        return (dc);
}
/**
 * @brief Create a device class
 *
 * If a device class with the name @p classname exists, return it,
 * otherwise create and return a new device class.
 *
 * @param classname     the devclass name to find or create
 */
devclass_t
devclass_create(const char *classname)
{
        return (devclass_find_internal(classname, 0, TRUE));
}

devclass_t
devclass_find(const char *classname)
{
        return (devclass_find_internal(classname, 0, FALSE));
}

device_t
make_device(device_t parent, const char *name, int unit)
{
        device_t dev;
        devclass_t dc;

        PDEBUG(("%s at %s as unit %d", name, DEVICENAME(parent), unit));

        if (name) {
                dc = devclass_find_internal(name, 0, TRUE);
                if (!dc) {
                        kprintf("make_device: can't find device class %s\n",
                            name);
                        return (NULL);
                }
        } else {
                dc = NULL;
        }

        dev = kcalloc(sizeof(struct device));
        if (!dev)
                return (NULL);

        dev->parent = parent;
        TAILQ_INIT(&dev->children);
        //kobj_init((kobj_t) dev, &null_class);
        dev->driver = NULL;
        dev->devclass = NULL;
        dev->unit = unit;
        dev->nameunit = NULL;
        dev->desc = NULL;
        dev->busy = 0;
        dev->devflags = 0;
        dev->flags = DF_ENABLED;
        dev->order = 0;
        if (unit == -1)
                dev->flags |= DF_WILDCARD;
        if (name) {
                dev->flags |= DF_FIXEDCLASS;
               if (devclass_add_device(dc, dev)) {
                       /*  kobj_delete((kobj_t) dev, M_BUS);*/
                        return (NULL);
                }
        }
        dev->ivars = NULL;
        dev->softc = NULL;

        dev->state = DS_NOTPRESENT;

        TAILQ_INSERT_TAIL(&bus_data_devices, dev, devlink);
        //bus_data_generation_update();

        return (dev);
}

device_t
device_add_child_ordered(device_t dev, int order, const char *name, int unit)
{
        device_t child;
        device_t place;

        PDEBUG(("%s at %s with order %d as unit %d",
            name, DEVICENAME(dev), order, unit));

        child = make_device(dev, name, unit);
        if (child == NULL)
                return (child);
        child->order = order;

        TAILQ_FOREACH(place, &dev->children, link) {
                if (place->order > order)
                        break;
        }

        if (place) {
                /*
                 * The device 'place' is the first device whose order is
                 * greater than the new child.
                 */
                TAILQ_INSERT_BEFORE(place, child, link);
        } else {
                /*
                 * The new child's order is greater or equal to the order of
                 * any existing device. Add the child to the tail of the list.
                 */
                TAILQ_INSERT_TAIL(&dev->children, child, link);
        }

        //bus_data_generation_update();
        return (child);
}


/**
 * @brief Set the devclass of a device
 * @see devclass_add_device().
 */
int
device_set_devclass(device_t dev, const char *classname)
{
        devclass_t dc;
        int error;

        if (!classname) {
                if (dev->devclass)
                        devclass_delete_device(dev->devclass, dev);
                return (0);
        }

        if (dev->devclass) {
                kprintf("device_set_devclass: device class already set\n");
                return (EINVAL);
        }

        dc = devclass_find_internal(classname, 0, TRUE);
        if (!dc)
                return (ENOMEM);

        error = devclass_add_device(dc, dev);

        //bus_data_generation_update();
        return (error);
}

int
device_probe_and_attach(device_t dev)
{
        int error;
		int bootverbose=1;


        if (dev->state >= DS_ALIVE)
                return (0);

        if (!(dev->flags & DF_ENABLED)) {
                if (bootverbose) {
                        //device_print_prettyname(dev);
                        kprintf("not probed (disabled)\n");
                }
                return (0);
        }
        if ((error = device_probe_child(dev->parent, dev)) != 0) {
                if (!(dev->flags & DF_DONENOMATCH)) {
                        BUS_PROBE_NOMATCH(dev->parent, dev);
                        //devnomatch(dev);
                        dev->flags |= DF_DONENOMATCH;
                }
                return (error);
        }
        error = device_attach(dev);

        return (error);
}

/**
 * @brief Set the unit number of a device
 *
 * This function can be used to override the unit number used for a
 * device (e.g. to wire a device to a pre-configured unit number).
 */
int
device_set_unit(device_t dev, int unit)
{
        devclass_t dc;
        int err;

        dc = device_get_devclass(dev);
        if (unit < dc->maxunit && dc->devices[unit])
                return (EBUSY);
        err = devclass_delete_device(dc, dev);
        if (err)
                return (err);
        dev->unit = unit;
        err = devclass_add_device(dc, dev);
        if (err)
                return (err);

        //bus_data_generation_update();
        return (0);
}
int
device_attach(device_t dev)
{
        int error;

       // device_sysctl_init(dev);
        //if (!device_is_quiet(dev))
                //device_print_child(dev->parent, dev);
        if ((error = DEVICE_ATTACH(dev)) != 0) {
                kprintf("device_attach: %s%d attach returned %d\n",
                    dev->driver->name, dev->unit, error);
                /* Unset the class; set in device_probe_child */
                if (dev->devclass == 0)
                        device_set_devclass(dev, 0);
                device_set_driver(dev, NULL);
               // device_sysctl_fini(dev);
                dev->state = DS_NOTPRESENT;
                return (error);
        }
        dev->state = DS_ATTACHED;
        //devadded(dev);
        return (0);
}

int
device_shutdown(device_t dev)
{
        if (dev->state < DS_ATTACHED)
                return (0);
        return (DEVICE_SHUTDOWN(dev));
}

void
device_disable(device_t dev)
{
        dev->flags &= ~DF_ENABLED;
}

/**
 * @brief Return non-zero if the device was successfully probed
 */
int
device_is_alive(device_t dev)
{
        return (dev->state >= DS_ALIVE);
}

void
device_enable(device_t dev)
{
        dev->flags |= DF_ENABLED;
}
/**
 * @brief Return the device's state
 */
device_state_t
device_get_state(device_t dev)
{
        return (dev->state);
}
/**
 * @brief Set the device's flags
 */
void
device_set_flags(device_t dev, u_int32_t flags)
{
        dev->devflags = flags;
}

/**
 * @brief Clear the DF_QUIET flag for the device
 */
void
device_verbose(device_t dev)
{
        dev->flags &= ~DF_QUIET;
}

/**
 * @brief Helper function for implementing BUS_PRINT_CHILD().
 *
 * This function prints the first part of the ascii representation of
 * @p child, including its name, unit and description (if any - see
 * device_set_desc()).
 *
 * @returns the number of characters printed
 */
int
bus_print_child_header(device_t dev, device_t child)
{
        int     retval = 0;

        if (device_get_desc(child)) {
                 device_printf(child, "<%s>", device_get_desc(child));
        } else {
                kprintf("%s", device_get_nameunit(child));
        }

        return (retval);
}

/**
 * @brief Helper function for implementing BUS_PRINT_CHILD().
 *
 * This function prints the last part of the ascii representation of
 * @p child, which consists of the string @c " on " followed by the
 * name and unit of the @p dev.
 *
 * @returns the number of characters printed
 */
int
bus_print_child_footer(device_t dev, device_t child)
{
        return (kprintf(" on %s\n", device_get_nameunit(dev)));
}

/**
 * @brief Helper function for implementing BUS_PRINT_CHILD().
 *
 * This function simply calls bus_print_child_header() followed by
 * bus_print_child_footer().
 *
 * @returns the number of characters printed
 */
int
bus_generic_print_child(device_t dev, device_t child)
{
        int     retval = 0;

        retval += bus_print_child_header(dev, child);
        retval += bus_print_child_footer(dev, child);

        return (retval);
}

/**
 * @brief Print the name of the device followed by a colon and a space
 *
 * @returns the number of characters printed
 */
int
device_print_prettyname(device_t dev)
{
        const char *name = device_get_name(dev);

        if (name == 0)
                return (kprintf("unknown: "));
        return (kprintf("%s%d: ", name, device_get_unit(dev)));
}

/**
 * @brief Increment the busy counter for the device
 */
void
device_busy(device_t dev)
{
        if (dev->state < DS_ATTACHED)
                panic("device_busy: called for unattached device");
        if (dev->busy == 0 && dev->parent)
                device_busy(dev->parent);
        dev->busy++;
        dev->state = DS_BUSY;
}

void
device_unbusy(device_t dev)
{
        if (dev->state != DS_BUSY)
                panic("device_unbusy: called for non-busy device");
        dev->busy--;
        if (dev->busy == 0) {
                if (dev->parent)
                        device_unbusy(dev->parent);
                dev->state = DS_ATTACHED;
        }
}


void device_quiet(device_t dev)
{
        dev->flags |= DF_QUIET;
	}

int
device_is_quiet(device_t dev)
{
        return ((dev->flags & DF_QUIET) != 0);
}

const char *
device_get_name(device_t dev)
{
        if (dev != NULL && dev->devclass)
                return (devclass_get_name(dev->devclass));
        return (NULL);
}


/**
 * @brief Return non-zero if the DF_ENABLED flag is set on the device
 */
int
device_is_enabled(device_t dev)
{
        return ((dev->flags & DF_ENABLED) != 0);
}



/**
 * @brief Detach a driver from a device
 *
 * This function is a wrapper around the DEVICE_DETACH() driver
 * method. If the call to DEVICE_DETACH() succeeds, it calls
 * BUS_CHILD_DETACHED() for the parent of @p dev, queues a
 * notification event for user-based device management services and
 * cleans up the device's sysctl tree.
 *
 * @param dev           the device to un-initialise
 *
 * @retval 0            success
 * @retval ENXIO        no driver was found
 * @retval ENOMEM       memory allocation failure
 * @retval non-zero     some other unix error code
 */
int
device_detach(device_t dev)
{
        int error;

        PDEBUG(("%s", DEVICENAME(dev)));
        if (dev->state == DS_BUSY)
                return (EBUSY);
        if (dev->state != DS_ATTACHED)
                return (0);

        if ((error = DEVICE_DETACH(dev)) != 0)
                return (error);
        //devremoved(dev);
        device_printf(dev, "detached\n");
        if (dev->parent){
                //BUS_CHILD_DETACHED(dev->parent, dev);
		}

        if (!(dev->flags & DF_FIXEDCLASS))
                devclass_delete_device(dev->devclass, dev);

        dev->state = DS_NOTPRESENT;
        device_set_driver(dev, NULL);
        device_set_desc(dev, NULL);
       // device_sysctl_fini(dev);

        return (0);
}

/**
 * @brief Find a device given a unit number
 *
 * This is similar to devclass_get_devices() but only searches for
 * devices which have @p dev as a parent.
 *
 * @param dev           the parent device to search
 * @param unit          the unit number to search for.  If the unit is -1,
 *                      return the first child of @p dev which has name
 *                      @p classname (that is, the one with the lowest unit.)
 *
 * @returns             the device with the given unit number or @c
 *                      NULL if there is no such device
 */
device_t
device_find_child(device_t dev, const char *classname, int unit)
{
        devclass_t dc;
        device_t child;

        dc = devclass_find(classname);
        if (!dc)
                return (NULL);

        if (unit != -1) {
                child = devclass_get_device(dc, unit);
                if (child && child->parent == dev)
                        return (child);
        } else {
                for (unit = 0; unit <= devclass_get_maxunit(dc); unit++) {
                        child = devclass_get_device(dc, unit);
                        if (child && child->parent == dev)
                                return (child);
                }
        }
        return (NULL);
}

/**
 * @internal
 */
static driverlink_t
first_matching_driver(devclass_t dc, device_t dev)
{
        if (dev->devclass)
                return (devclass_find_driver_internal(dc, dev->devclass->name));
        return (TAILQ_FIRST(&dc->drivers));
}

/**
 * @internal
 */
static driverlink_t
next_matching_driver(devclass_t dc, device_t dev, driverlink_t last)
{
        if (dev->devclass) {
                driverlink_t dl;
                for (dl = TAILQ_NEXT(last, link); dl; dl = TAILQ_NEXT(dl, link))
                        if (!strcmp(dev->devclass->name, dl->driver->name))
                                return (dl);
                return (NULL);
        }
        return (TAILQ_NEXT(last, link));
}

/**
 * @internal
 */
static int
device_probe_child(device_t dev, device_t child)
{
        devclass_t dc;
        driverlink_t best = 0;
        driverlink_t dl;
        int result, pri = 0;
        int hasclass = (child->devclass != 0);

        dc = dev->devclass;
        if (!dc)
                panic("device_probe_child: parent device has no devclass");

        if (child->state == DS_ALIVE)
                return (0);

        for (; dc; dc = dc->parent) {
                for (dl = first_matching_driver(dc, child);
                     dl;
                     dl = next_matching_driver(dc, child, dl)) {
                        PDEBUG(("Trying %s", DRIVERNAME(dl->driver)));
                        device_set_driver(child, dl->driver);
                        if (!hasclass)
                                device_set_devclass(child, dl->driver->name);

                        /* Fetch any flags for the device before probing. */
                        //resource_int_value(dl->driver->name, child->unit,
                        //    "flags", &child->devflags);

                        result = DEVICE_PROBE(child);

                        /* Reset flags and devclass before the next probe. */
                        child->devflags = 0;
                        if (!hasclass){
                                device_set_devclass(child, 0);
						}

                        /*
                         * If the driver returns SUCCESS, there can be
                         * no higher match for this device.
                         */
                        if (result == 0) {
                                best = dl;
                                pri = 0;
                                break;
                        }

                        /*
                         * The driver returned an error so it
                         * certainly doesn't match.
                         */
                        if (result > 0) {
                                device_set_driver(child, 0);
                                continue;
                        }

                        /*
                         * A priority lower than SUCCESS, remember the
                         * best matching driver. Initialise the value
                         * of pri for the first match.
                         */
                        if (best == 0 || result > pri) {
                                best = dl;
                                pri = result;
                                continue;
                        }
                }
                /*
                 * If we have an unambiguous match in this devclass,
                 * don't look in the parent.
                 */
                if (best && pri == 0)
                        break;
        }

        /*
         * If we found a driver, change state and initialise the devclass.
         */
        if (best) {

                /* Set the winning driver, devclass, and flags. */
                if (!child->devclass)
                        device_set_devclass(child, best->driver->name);
                device_set_driver(child, best->driver);
                //resource_int_value(best->driver->name, child->unit,
                 //   "flags", &child->devflags);

                if (pri < 0) {
                        /*
                         * A bit bogus. Call the probe method again to make
                         * sure that we have the right description.
                         */
                        DEVICE_PROBE(child);
                }
                child->state = DS_ALIVE;

                //bus_data_generation_update();
                return (0);
        }

        return (ENXIO);
}

int
device_get_children(device_t dev, device_t **devlistp, int *devcountp)
{
        int count;
        device_t child;
        device_t *list;

        count = 0;
        TAILQ_FOREACH(child, &dev->children, link) {
                count++;
        }

        list = kcalloc(count * sizeof(device_t));
        if (!list)
                return (ENOMEM);

        count = 0;
        TAILQ_FOREACH(child, &dev->children, link) {
                list[count] = child;
                count++;
        }

        *devlistp = list;
        *devcountp = count;

        return (0);
}

/**
 * @brief Return the current driver for the device or @c NULL if there
 * is no driver currently attached
 */
driver_t *
device_get_driver(device_t dev)
{
        return (dev->driver);
}

static void
device_set_desc_internal(device_t dev, const char* desc, int copy)
{
        if (dev->desc && (dev->flags & DF_DESCMALLOCED)) {
                kfree(dev->desc);
                dev->flags &= ~DF_DESCMALLOCED;
                dev->desc = NULL;
        }

        if (copy && desc) {
                dev->desc = kcalloc(strlen(desc) + 1);
                if (dev->desc) {
                        strcpy(dev->desc, desc);
                        dev->flags |= DF_DESCMALLOCED;
                }
        } else {
                /* Avoid a -Wcast-qual warning */
                dev->desc = (char *) desc;
        }

        //bus_data_generation_update();
}

/**
 * @brief Set the device's description
 *
 * The value of @c desc should be a string constant that will not
 * change (at least until the description is changed in a subsequent
 * call to device_set_desc() or device_set_desc_copy()).
 */
void
device_set_desc(device_t dev, const char* desc)
{
        device_set_desc_internal(dev, desc, FALSE);
}

/**
 * @brief Set the device's description
 *
 * The string pointed to by @c desc is copied. Use this function if
 * the device description is generated, (e.g. with sprintf()).
 */
void
device_set_desc_copy(device_t dev, const char* desc)
{
        device_set_desc_internal(dev, desc, TRUE);
}

void* device_get_ivars(device_t dev)
{
	return dev->ivars;
}

void device_set_ivars(device_t dev, void *ivars)
{
	dev->ivars = ivars;
}


void* device_get_softc(device_t dev)
{
	return dev->softc;
}

void device_set_softc(device_t dev, void *ivars)
{
	dev->softc = ivars;
}


/**
 * @brief Return the current devclass for the device or @c NULL if
 * there is none.
 */
devclass_t
device_get_devclass(device_t dev)
{
        return (dev->devclass);
}
device_t
device_add_child(device_t dev, const char *name, int unit)
{
        return (device_add_child_ordered(dev, 0, name, unit));
}

device_t
device_get_parent(device_t dev)
{
        return (dev->parent);
}



int
device_delete_child(device_t dev, device_t child)
{
        int error;
        device_t grandchild;

        PDEBUG(("%s from %s", DEVICENAME(child), DEVICENAME(dev)));

        /* remove children first */
        while ( (grandchild = TAILQ_FIRST(&child->children)) ) {
                error = device_delete_child(child, grandchild);
                if (error)
                        return (error);
        }

        if ((error = device_detach(child)) != 0)
                return (error);
        if (child->devclass)
                devclass_delete_device(child->devclass, child);
        TAILQ_REMOVE(&dev->children, child, link);
        TAILQ_REMOVE(&bus_data_devices, child, devlink);
        //kobj_delete((kobj_t) child, M_BUS);

        //bus_data_generation_update();
        return (0);
}
int
device_is_attached(device_t dev)
{
        return (dev->state >= DS_ATTACHED);
}

int
device_set_driver(device_t dev, driver_t *driver)
{
        if (dev->state >= DS_ATTACHED)
                return (EBUSY);

        if (dev->driver == driver)
                return (0);

        if (dev->softc && !(dev->flags & DF_EXTERNALSOFTC)) {
                //kfree(dev->softc); //fixme
                //dev->softc = NULL;
        }
        //kobj_delete((kobj_t) dev, 0);
        dev->driver = driver;
        if (driver) {
                //kobj_init((kobj_t) dev, (kobj_class_t) driver);
                if (!(dev->flags & DF_EXTERNALSOFTC) && driver->size > 0) {
						if(!dev->softc)
							dev->softc = kcalloc(driver->size);

                        if (!dev->softc) {
                                //kobj_delete((kobj_t) dev, 0);
                                //kobj_init((kobj_t) dev, &null_class);
                                //dev->driver = NULL;
                                return (ENOMEM);
                        }
                }
        } else {
               // kobj_init((kobj_t) dev, &null_class);
        }

        //bus_data_generation_update();
        return (0);
}


/**
 * @brief Return a string containing the device's devclass name
 * followed by an ascii representation of the device's unit number
 * (e.g. @c "foo2").
 */
const char *
device_get_nameunit(device_t dev)
{
        return (dev->nameunit);
}

/**
 * @brief Return the device's unit number.
 */
int
device_get_unit(device_t dev)
{
        return (dev->unit);
}

/**
 * @brief Return the device's description string
 */
const char *
device_get_desc(device_t dev)
{
        return (dev->desc);
}




/**
 * @brief Return the device's flags
 */
u_int32_t
device_get_flags(device_t dev)
{
        return (dev->devflags);
}



int
bus_teardown_intr(device_t dev, void *r, void *cookie)
{
        if (dev->parent == 0)
                return (EINVAL);
        //return (BUS_TEARDOWN_INTR(dev->parent, dev, r, cookie));
}
/**
 * @brief Helper function for implementing DEVICE_PROBE()
 *
 * This function can be used to help implement the DEVICE_PROBE() for
 * a bus (i.e. a device which has other devices attached to it). It
 * calls the DEVICE_IDENTIFY() method of each driver in the device's
 * devclass.
 */
int
bus_generic_probe(device_t dev)
{
        devclass_t dc = dev->devclass;
        driverlink_t dl;

        TAILQ_FOREACH(dl, &dc->drivers, link) {
                DEVICE_IDENTIFY(dl->driver, dev);
        }

        return (0);
}

/**
 * @brief Helper function for implementing DEVICE_ATTACH()
 *
 * This function can be used to help implement the DEVICE_ATTACH() for
 * a bus. It calls device_probe_and_attach() for each of the device's
 * children.
 */
int
bus_generic_attach(device_t dev)
{
        device_t child;

        TAILQ_FOREACH(child, &dev->children, link) {
              device_probe_and_attach(child);
        }

        return (0);
}

/**
 * @brief Helper function for implementing DEVICE_DETACH()
 *
 * This function can be used to help implement the DEVICE_DETACH() for
 * a bus. It calls device_detach() for each of the device's
 * children.
 */
int
bus_generic_detach(device_t dev)
{
        device_t child;
        int error;

        if (dev->state != DS_ATTACHED)
                return (EBUSY);

        TAILQ_FOREACH(child, &dev->children, link) {
                if ((error = device_detach(child)) != 0)
                        return (error);
        }

        return (0);
}

/**
 * @brief Helper function for implementing DEVICE_SHUTDOWN()
 *
 * This function can be used to help implement the DEVICE_SHUTDOWN()
 * for a bus. It calls device_shutdown() for each of the device's
 * children.
 */
int
bus_generic_shutdown(device_t dev)
{
        device_t child;

        TAILQ_FOREACH(child, &dev->children, link) {
                device_shutdown(child);
        }

        return (0);
}

/**
 * @brief Helper function for implementing DEVICE_SUSPEND()
 *
 * This function can be used to help implement the DEVICE_SUSPEND()
 * for a bus. It calls DEVICE_SUSPEND() for each of the device's
 * children. If any call to DEVICE_SUSPEND() fails, the suspend
 * operation is aborted and any devices which were suspended are
 * resumed immediately by calling their DEVICE_RESUME() methods.
 */
int
bus_generic_suspend(device_t dev)
{
        int             error;
        device_t        child, child2;

        TAILQ_FOREACH(child, &dev->children, link) {
                error = DEVICE_SUSPEND(child);
                if (error) {
                        for (child2 = TAILQ_FIRST(&dev->children);
                             child2 && child2 != child;
                             child2 = TAILQ_NEXT(child2, link))
                                DEVICE_RESUME(child2);
                        return (error);
                }
        }
        return (0);
}

/**
 * @brief Helper function for implementing DEVICE_RESUME()
 *
 * This function can be used to help implement the DEVICE_RESUME() for
 * a bus. It calls DEVICE_RESUME() on each of the device's children.
 */
int
bus_generic_resume(device_t dev)
{
        device_t        child;

        TAILQ_FOREACH(child, &dev->children, link) {
                DEVICE_RESUME(child);
                /* if resume fails, there's nothing we can usefully do... */
        }
        return (0);
}


/*bus_space_barrier(device_t dev)
{
	//¹ØÖÐ¶Ï
	TRACE_HERE;
	return 0;
}*/



int
driver_module_handler(void*mod, int what, void *arg)
{
        int error;
        struct driver_module_data *dmd;
        devclass_t bus_devclass;
        driver_t* driver;

		//kprintf("driver_module_handler xxx\n");

        dmd = (struct driver_module_data *)arg;
        bus_devclass = devclass_find_internal(dmd->dmd_busname, 0, TRUE);
        error = 0;

        switch (what) {
        case MOD_LOAD:
                if (dmd->dmd_chainevh)
                        error = dmd->dmd_chainevh(mod,what,dmd->dmd_chainarg);

                driver = dmd->dmd_driver;
                PDEBUG(("Loading module: driver %s on bus %s",
                    DRIVERNAME(driver), dmd->dmd_busname));
                error = devclass_add_driver(bus_devclass, driver);
                if (error)
                        break;

                /*
                 * If the driver has any base classes, make the
                 * devclass inherit from the devclass of the driver's
                 * first base class. This will allow the system to
                 * search for drivers in both devclasses for children
                 * of a device using this driver.
                 */
                /*if (driver->baseclasses) {
                        const char *parentname;
                        parentname = driver->baseclasses[0]->name;
                        *dmd->dmd_devclass =
                                devclass_find_internal(driver->name,
                                    parentname, TRUE);
                } else*/ {
                        *dmd->dmd_devclass =
                                devclass_find_internal(driver->name, 0, TRUE);
                }
                break;

        case MOD_UNLOAD:
                PDEBUG(("Unloading module: driver %s from bus %s",
                    DRIVERNAME(dmd->dmd_driver),
                    dmd->dmd_busname));
                error = devclass_delete_driver(bus_devclass,
                    dmd->dmd_driver);

                if (!error && dmd->dmd_chainevh)
                        error = dmd->dmd_chainevh(mod,what,dmd->dmd_chainarg);
                break;
        default:
                error = EOPNOTSUPP;
                break;
        }

        return (error);
}

int register_module_handler(void *mod,void *arg)
{
	moduledata_t *mod_data; 
	modeventhand_t evhand;

     mod_data = (moduledata_t *)arg;
	 evhand=mod_data->evhand; //priv;

	 if (!(evhand))
	 {
		 return -1;
	 }
		

	 return (evhand)(mod, MOD_LOAD,mod_data->priv);
}

int unregister_module_handler(void *mod,void *arg)
{
	moduledata_t *mod_data; 
	modeventhand_t evhand;

     mod_data = (moduledata_t *)arg;
	 evhand=mod_data->evhand; //priv;

	 if (!(evhand))
	 {
		 return -1;
	 }
		

	 return (evhand)(mod, MOD_UNLOAD,mod_data->priv);

}



static int
root_print_child(device_t dev, device_t child)
{
        int     retval = 0;

        //retval += bus_print_child_header(dev, child);
        //retval += kprintf("\n");

        return (retval);
}

static int
root_setup_intr(device_t dev, device_t child, driver_intr_t *intr, void *arg,
    void **cookiep)
{
        /*
         * If an interrupt mapping gets to here something bad has happened.
         */
        panic("root_setup_intr");
}

/*
 * If we get here, assume that the device is permanant and really is
 * present in the system.  Removable bus drivers are expected to intercept
 * this call long before it gets here.  We return -1 so that drivers that
 * really care can check vs -1 or some ERRNO returned higher in the food
 * chain.
 */
static int
root_child_present(device_t dev, device_t child)
{
        return (-1);
}

static device_method_t root_methods[] = {
        /* Device interface */
        DEVMETHOD(device_shutdown,     bus_generic_shutdown),
        DEVMETHOD(device_suspend,      bus_generic_suspend),
        DEVMETHOD(device_resume,       bus_generic_resume),

        /* Bus interface */
        DEVMETHOD(bus_print_child,     root_print_child),
        //DEVMETHOD(bus_read_ivar,       bus_generic_read_ivar),
        //DEVMETHOD(bus_write_ivar,      bus_generic_write_ivar),
        DEVMETHOD(bus_setup_intr,      root_setup_intr),
        DEVMETHOD(bus_child_present,   root_child_present),

        { 0, 0 }
};

static driver_t root_driver = {
        name:"root",
        methods:root_methods,
        1,                      /* no softc */
};

device_t        root_bus;
devclass_t      root_devclass;


device_t device_get_top()
{
	return root_bus;
}

static int
root_bus_module_handler(void* mod, int what, void* arg)
{
        switch (what) {
        case MOD_LOAD:
			//kprintf("root_bus_module_handler:load root bus\n");
                TAILQ_INIT(&bus_data_devices);
                //kobj_class_compile((kobj_class_t) &root_driver);
                root_bus = make_device(NULL, "root", 0);
                root_bus->desc = "System root bus";
                //kobj_init((kobj_t) root_bus, (kobj_class_t) &root_driver);
                root_bus->driver = &root_driver;
                root_bus->state = DS_ATTACHED;
                root_devclass = devclass_find_internal("root", 0, FALSE);
                //devinit();
                return (0);

        case MOD_UNLOAD:
                device_shutdown(root_bus);
                return (0);
        default:
                return (EOPNOTSUPP);
        }

        return (0);
}

static moduledata_t root_bus_mod = {
        "rootbus",
        root_bus_module_handler,
        0
};

void init_compat_bsd(void)
{
	register_module_handler(NULL,&root_bus_mod);
	return ;
}


