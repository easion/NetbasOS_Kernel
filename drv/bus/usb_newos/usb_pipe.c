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

#define DEBUG_MSG_PREFIX "USB_PIPE - "


int create_usb_pipe(usb_hc *hc, usb_interface *iface, usb_pipe **_p,
	usb_endpoint_descriptor *desc, int address, bool lowspeed)
{
	usb_pipe *p;
	int err;

	p = KMALLOC(sizeof(usb_pipe));
	if(!p)
		return -1;

	p->lowspeed = lowspeed;
	p->hc = hc;

	// create the endpoint
	err = hc->hooks->create_endpoint(hc->hc_cookie, &p->endpoint, desc, address, lowspeed);
	if(err < 0)
		goto err;

	*_p = p;

	return 0;

err:
	kfree(p);
	return err;
}

int send_usb_request(usb_device *device, uint8_t request_type, uint8_t request, uint16_t value,
	uint16_t index, uint16_t len, void *data)
{
	usb_hc_transfer *transfer;
	usb_request *req;
	int err;

	// allocate a hc transfer descriptor
	transfer = KMALLOC(sizeof(usb_hc_transfer));
	if(!transfer)
		return -1;

	// allocate a usb request
	req = KMALLOC(sizeof(usb_request));
	if(!req) {
		kfree(transfer);
		return -1;
	}

	req->type = request_type;
	req->request = request;
	req->value = value;
	req->index = index;
	req->len = len;

	transfer->setup_buf = req;
	transfer->setup_len = 8;

	transfer->data_buf = data;
	transfer->data_len = len;

	// create a notification sem
	transfer->completion_sem = sem_create(0, "usb notify");
	transfer->callback = NULL;
	transfer->cookie = NULL;

	// do the request
	err = device->control_pipe->hc->hooks->enqueue_transfer(device->control_pipe->hc->hc_cookie,
		device->control_pipe->endpoint, transfer);
	if(err < 0)
		goto out;

	// wait for the transfer to complete
	sem_acquire(transfer->completion_sem, 1);

	err = transfer->status;

out:
	sem_delete(transfer->completion_sem);
	kfree(req);
	kfree(transfer);

	return err;
}

int queue_interrupt_transfer(usb_pipe *pipe, void *data, size_t len, void (*callback)(usb_hc_transfer *, void *), void *cookie)
{
	usb_hc_transfer *transfer;
	int err;

	SHOW_FLOW(5, "pipe %p, data %p, len %d, callback %p, cookie %p",
		pipe, data, len, callback, cookie);

	// allocate a hc transfer descriptor
	transfer = KMALLOC(sizeof(usb_hc_transfer));
	if(!transfer)
		return -1;

	transfer->setup_buf = NULL;
	transfer->setup_len = 0;
	transfer->data_buf = data;
	transfer->data_len = len;
	transfer->status = 0;
	transfer->callback = callback;
	transfer->cookie = cookie;
	transfer->completion_sem = -1;

	err = pipe->hc->hooks->enqueue_transfer(pipe->hc->hc_cookie,
		pipe->endpoint, transfer);
	if(err < 0)
		kfree(transfer);

	return err;
}

