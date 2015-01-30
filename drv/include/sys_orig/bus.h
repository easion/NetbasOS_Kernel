/*-
 * Copyright (c) 1997,1998 Doug Rabson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id: bus.h,v 1.8 1999/01/16 17:44:08 dfr Exp $
 */

#ifndef _SYS_BUS_H_
#define _SYS_BUS_H_

#ifdef KERNEL

/*
 * Forward declarations
 */
typedef struct device		*device_t;
typedef struct driver		driver_t;
typedef struct device_method	device_method_t;
typedef struct devclass		*devclass_t;
typedef struct device_ops	*device_ops_t;
typedef struct device_op_desc	*device_op_desc_t;

typedef void driver_intr_t(void*);

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

typedef int (*devop_t)(void);

struct device_method {
    device_op_desc_t	desc;
    devop_t		func;
};

struct driver {
    const char		*name;		/* driver name */
    device_method_t	*methods;	/* method table */
    driver_type_t	type;
    size_t		softc;		/* size of device softc struct */
    void		*priv;		/* driver private data */
    TAILQ_ENTRY(driver) link;		/* list of devices on bus */
    device_ops_t	ops;		/* compiled method table */
};

typedef enum device_state device_state_t;
enum device_state {
    DS_NOTPRESENT,			/* not probed or probe failed */
    DS_ALIVE,				/* probe succeeded */
    DS_ATTACHED,			/* attach method called */
    DS_BUSY				/* device is open */
};

/*
 * The root bus, to which all top-level busses are attached.
 */
extern device_t root_bus;
extern devclass_t root_devclass;
void	root_bus_configure(void);

/*
 * Useful functions for implementing busses.
 */
struct	resource;

int	bus_generic_activate_resource(device_t dev, device_t child, int type,
				      int rid, struct resource *r);
struct resource *bus_generic_alloc_resource(device_t bus, device_t child,
					    int type, int *rid,
					    u_long start, u_long end,
					    u_long count, u_int flags);
int	bus_generic_attach(device_t dev);
int	bus_generic_deactivate_resource(device_t dev, device_t child, int type,
					int rid, struct resource *r);
int	bus_generic_detach(device_t dev);
void	bus_generic_print_child(device_t dev, device_t child);
int	bus_generic_read_ivar(device_t dev, device_t child, int which,
			      uintptr_t *result);
int	bus_generic_release_resource(device_t bus, device_t child,
				     int type, int rid, struct resource *r);
int	bus_generic_resume(device_t dev);
int	bus_generic_setup_intr(device_t dev, device_t child,
			       struct resource *irq,
			       driver_intr_t *intr, void *arg, void **cookiep);
int	bus_generic_shutdown(device_t dev);
int	bus_generic_suspend(device_t dev);
int	bus_generic_teardown_intr(device_t dev, device_t child,
				  struct resource *irq, void *cookie);
int	bus_generic_write_ivar(device_t dev, device_t child, int which,
			       uintptr_t value);

/*
 * Wrapper functions for the BUS_*_RESOURCE methods to make client code
 * a little simpler.
 */
struct	resource *bus_alloc_resource(device_t dev, int type, int *rid,
				     u_long start, u_long end, u_long count,
				     u_int flags);
int	bus_activate_resource(device_t dev, int type, int rid, 
			      struct resource *r);
int	bus_deactivate_resource(device_t dev, int type, int rid,
				struct resource *r);
int	bus_release_resource(device_t dev, int type, int rid, 
			     struct resource *r);

/*
 * Access functions for device.
 */
device_t	device_add_child(device_t dev, const char *name, int unit,
				 void *ivp);
device_t	device_add_child_after(device_t dev, device_t place,
				       const char *name, int unit, void *ivp);
void	device_busy(device_t dev);
int	device_delete_child(device_t dev, device_t child);
int	device_detach(device_t dev);
void	device_disable(device_t dev);
void	device_enable(device_t dev);
device_t	device_find_child(device_t dev, const char *classname,
				  int unit);
const char 	*device_get_desc(device_t dev);
devclass_t	device_get_devclass(device_t dev);
driver_t	*device_get_driver(device_t dev);
device_t	device_get_parent(device_t dev);
int	device_get_children(device_t dev, device_t **listp, int *countp);
void	*device_get_ivars(device_t dev);
const	char *device_get_name(device_t dev);
void	*device_get_softc(device_t dev);
device_state_t	device_get_state(device_t dev);
int	device_get_unit(device_t dev);
int	device_is_enabled(device_t dev);
int	device_is_alive(device_t dev); /* did probe succeed? */
void	device_print_prettyname(device_t dev);
void	device_printf(device_t dev, const char *, ...) __printflike(2, 3);
int	device_probe_and_attach(device_t dev);
void	device_set_desc(device_t dev, const char* desc);
int	device_set_devclass(device_t dev, const char *classname);
int	device_set_driver(device_t dev, driver_t *driver);
int	device_shutdown(device_t dev);
void	device_unbusy(device_t dev);

/*
 * Access functions for devclass.
 */
int	devclass_add_driver(devclass_t dc, driver_t *driver);
int	devclass_delete_driver(devclass_t dc, driver_t *driver);
devclass_t	devclass_find(const char *classname);
driver_t	*devclass_find_driver(devclass_t dc, const char *classname);
const char 	*devclass_get_name(devclass_t dc);
device_t	devclass_get_device(devclass_t dc, int unit);
void	*devclass_get_softc(devclass_t dc, int unit);
int	devclass_get_devices(devclass_t dc, device_t **listp, int *countp);
int	devclass_get_maxunit(devclass_t dc);

/*
 * Access functions for device resources.
 */
int	resource_int_value(const char *name, int unit, char *resname,
			   int *result);
int	resource_long_value(const char *name, int unit, char *resname,
			    long *result);
int	resource_string_value(const char *name, int unit, char *resname,
			      char **result);
int	resource_query_string(int i, char *resname, char *value);
char	*resource_query_name(int i);
int	resource_query_unit(int i);

/*
 * Shorthand for constructing method tables.
 */
#define DEVMETHOD(NAME, FUNC) { &NAME##_desc, (devop_t) FUNC }

/*
 * Some common device interfaces.
 */
#include "device_if.h"
#include "bus_if.h"

struct	module;

int	driver_module_handler(struct module *, int, void *);

/*
 * Module support for automatically adding drivers to busses.
 */
struct driver_module_data {
	int		(*dmd_chainevh)(struct module *, int, void *);
	void		*dmd_chainarg;
	const char	*dmd_busname;
	driver_t	**dmd_drivers;
	int		dmd_ndrivers;
	devclass_t	*dmd_devclass;
};

#define DRIVER_MODULE(name, busname, driver, devclass, evh, arg)	\
									\
static driver_t *name##_##busname##_driver_list[] = { &driver };	\
static struct driver_module_data name##_##busname##_driver_mod = {	\
	evh, arg,							\
	#busname,							\
	name##_##busname##_driver_list,					\
	(sizeof name##_##busname##_driver_list) /			\
		(sizeof name##_##busname##_driver_list[0]),		\
	&devclass							\
};									\
									\
static moduledata_t name##_##busname##_mod = {				\
	#busname "/" #name,						\
	driver_module_handler,						\
	&name##_##busname##_driver_mod					\
};									\
DECLARE_MODULE(name##_##busname, name##_##busname##_mod,		\
	       SI_SUB_DRIVERS, SI_ORDER_MIDDLE)

#define MULTI_DRIVER_MODULE(name, busname, drivers, devclass, evh, arg) \
									\
static driver_t name##_##busname##_driver_list[] = drivers;		\
static struct driver_module_data name##_##busname##_driver_mod = {	\
	evh, arg,							\
	#busname,							\
	name##_##busname##_driver_list,					\
	(sizeof name##_##busname##_driver_list) /			\
		(sizeof name##_##busname##_driver_list[0]),		\
	&devclass							\
};									\
									\
static moduledata_t name##_##busname##_mod = {				\
	#busname "/" #name,						\
	driver_module_handler,						\
	&name##_##busname##_driver_mod					\
};									\
DECLARE_MODULE(name##_##busname, name##_##busname##_mod,		\
	       SI_SUB_DRIVERS, SI_ORDER_MIDDLE)

#define CDEV_DRIVER_MODULE(name, busname, driver, devclass,		\
			   major, devsw, evh, arg)			\
									\
static struct cdevsw_module_data name##_##busname##_cdevsw_mod = {	\
    evh, arg, makedev(major, 0), &devsw					\
};									\
									\
DRIVER_MODULE(name, busname, driver, devclass,				\
	      cdevsw_module_handler, &name##_##busname##_cdevsw_mod)

#define BDEV_DRIVER_MODULE(name, busname, driver, devclass,		\
			   bmajor, cmajor, devsw, evh, arg)		\
									\
static struct bdevsw_module_data name##_##busname##_bdevsw_mod = {	\
    evh, arg, makedev(bmajor, 0), makedev(cmajor, 0), &devsw		\
};									\
									\
DRIVER_MODULE(name, busname, driver, devclass,				\
	      bdevsw_module_handler, &name##_##busname##_bdevsw_mod)

#endif /* KERNEL */

#endif /* !_SYS_BUS_H_ */
