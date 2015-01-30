/*
** Copyright 2003, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/


 
#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/ia.h>
#include <drv/pci.h>
#include <drv/errno.h>
#include <linux/module.h>
#include <linux/list.h>

#include <i386/isa_io.h>
#include <bus/pci.h>

#include <usb.h>
#include <usb_hc.h>
#include "usb_priv.h"

#define debug_level_flow 10
#define debug_level_error 10
#define debug_level_info 10

#define DEBUG_MSG_PREFIX "USB - "


usb_bus *usb;

static int
hc_init_callback(void *hooks, void *hc_cookie)
{
	usb_hc *hc;

	SHOW_FLOW(1, "cookie %p", hc_cookie);

	hc = (usb_hc *)KMALLOC(sizeof(usb_hc));
	if(!hc)
		return -1;

	memset(hc, 0, sizeof(*hc));

	hc->hooks = hooks;
	hc->hc_cookie = hc_cookie;

	// add it to the list of host controllers
	hc->next = usb->hc_list;
	usb->hc_list = hc;

	// create an enumerator thread
	hc->enumerator_thread = new_kernel_thread("usb bus enumerator", &usb_enumerator_thread, hc);
	//set_thread_priority(hc->enumerator_thread, THREAD_MIN_RT_PRIORITY);
	//thread_resume_thread(hc->enumerator_thread);

	return 0;
}


static int
usb_module_init(void)
{
	usb = KMALLOC(sizeof(usb_bus));
	if(!usb)
		return -1;

	load_hc_modules();

	return 0;
}

static int
usb_module_uninit(void)
{
	return 0;
}

static struct usb_module_hooks usb_hooks = {
	NULL,
};
#if 0

static int
load_hc_modules()
{
	char module_name[255];
	size_t bufsize;
	modules_cookie cookie;
	struct usb_hc_module_hooks *hooks;
	int err;
	int count = 0;

	// scan through the host controller module dir and load all of them
	cookie = module_open_list(USB_HC_MODULE_NAME_PREFIX);
	bufsize = sizeof(module_name);
	while(read_next_module_name(cookie, module_name, &bufsize) >= 0) {
		bufsize = sizeof(module_name); // reset this for the next iteration

		err = module_get(module_name, 0, (void **)&hooks);
		if(err < 0)
			continue;

		err = hooks->init_hc(&hc_init_callback, hooks);
		if(err < 0) {
			module_put(module_name); // it failed, put it away
		}

		count++;
	}
	close_module_list(cookie);

	return count;
}


static module_header usb_module_header = {
	USB_BUS_MODULE_NAME,
	MODULE_CURR_VERSION,
	MODULE_KEEP_LOADED,
	&usb_hooks,
	&usb_module_init,
	&usb_module_uninit
};

module_header *modules[]  = {
	&usb_module_header,
	NULL
};

#endif
