/*-
 * Copyright (c) 1998 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Lennart Augustsson (lennart@augustsson.net) at
 * Carlstedt Research & Technology.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by the NetBSD
 *        Foundation, Inc. and its contributors.
 * 4. Neither the name of The NetBSD Foundation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Modifications for SUN TYPE 6 USB Keyboard by
 *  Jörg Peter Schley (jps@scxnet.de)
 */

/*
 * HID spec: http://www.usb.org/developers/devclass_docs/HID1_11.pdf
 */

#include "opt_kbd.h"
#include "opt_ukbd.h"

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/systm.h>
#include <sys/kernel.h>
#include <sys/malloc.h>
#include <sys/ioccom.h>
#include <sys/filio.h>
#include <sys/tty.h>
#include <sys/kbio.h>

#include <dev/kbd/kbdreg.h>
#include <dev/kbd/kbdtables.h>

#include <dev/usb2/usb_port.h>
#include <dev/usb2/usb.h>
#include <dev/usb2/usb_subr.h>
#include <dev/usb2/usb_hid.h>
#include <dev/usb2/usb_quirks.h>

/* the initial key map, accent map and fkey strings */
#ifdef UKBD_DFLT_KEYMAP
#define KBD_DFLT_KEYMAP
#include "ukbdmap.h"
#endif

__FBSDID("$FreeBSD: src/sys/dev/usb2/ukbd.c,v 1.53 2006/02/28 03:34:06 emax Exp $");

#ifdef USB_DEBUG
#define DPRINTF(n,fmt,...)						\
  do { if (ukbd_debug > (n)) {						\
      printf("%s: " fmt, __FUNCTION__,## __VA_ARGS__); } } while (0)

static int ukbd_debug = 0;
SYSCTL_NODE(_hw_usb, OID_AUTO, ukbd, CTLFLAG_RW, 0, "USB ukbd");
SYSCTL_INT(_hw_usb_ukbd, OID_AUTO, debug, CTLFLAG_RW,
	   &ukbd_debug, 0, "ukbd debug level");
#else
#define DPRINTF(...)
#endif

#define UPROTO_BOOT_KEYBOARD 1

#define UKBD_EMULATE_ATSCANCODE	       1
#define UKBD_DRIVER_NAME          "ukbd"
#define UKBD_NMOD                     8 /* units */
#define UKBD_NKEYCODE                 6 /* units */
#define UKBD_N_TRANSFER               3 /* units */
#define UKBD_IN_BUF_SIZE  (2*(UKBD_NMOD + (2*UKBD_NKEYCODE))) /* bytes */
#define UKBD_IN_BUF_FULL  (UKBD_IN_BUF_SIZE / 2) /* bytes */
#define UKBD_NFKEY        (sizeof(fkey_tab)/sizeof(fkey_tab[0])) /* units */

struct ukbd_data {
	u_int8_t	modifiers;
#define MOD_CONTROL_L	0x01
#define MOD_CONTROL_R	0x10
#define MOD_SHIFT_L	0x02
#define MOD_SHIFT_R	0x20
#define MOD_ALT_L	0x04
#define MOD_ALT_R	0x40
#define MOD_WIN_L	0x08
#define MOD_WIN_R	0x80
	u_int8_t	reserved;
	u_int8_t	keycode[UKBD_NKEYCODE];
} __attribute__((__packed__));

struct ukbd_softc {
	keyboard_t         sc_kbd;
	keymap_t           sc_keymap;
	accentmap_t        sc_accmap;
	fkeytab_t          sc_fkeymap[UKBD_NFKEY];
	struct __callout   sc_callout;
	struct ukbd_data   sc_ndata;
	struct ukbd_data   sc_odata;
	struct usbd_memory_wait sc_mem_wait;

	struct usbd_device * sc_udev;
	struct usbd_interface * sc_iface;
	struct usbd_xfer * sc_xfer[UKBD_N_TRANSFER];

	u_int32_t          sc_ntime[UKBD_NKEYCODE];
	u_int32_t          sc_otime[UKBD_NKEYCODE];
	u_int32_t          sc_input[UKBD_IN_BUF_SIZE]; /* input buffer */
	u_int32_t          sc_time_ms;
	u_int32_t          sc_composed_char; /* composed char code, if non-zero */
#ifdef UKBD_EMULATE_ATSCANCODE
	u_int32_t          sc_buffered_char[2];
#endif
	u_int32_t	   sc_flags; /* flags */
#define UKBD_FLAG_COMPOSE    0x0001
#define UKBD_FLAG_POLLING    0x0002
#define UKBD_FLAG_SET_LEDS   0x0004
#define UKBD_FLAG_PIPE_ERROR 0x0008
#define UKBD_FLAG_ATTACHED   0x0010
#define UKBD_FLAG_GONE       0x0020

	int32_t            sc_mode; /* input mode (K_XLATE,K_RAW,K_CODE) */
	int32_t		   sc_state;  /* shift/lock key state */
	int32_t		   sc_accents; /* accent key index (> 0) */

	u_int16_t          sc_inputs;
	u_int16_t          sc_inputhead;
	u_int16_t          sc_inputtail;

	u_int8_t           sc_leds;
	u_int8_t	   sc_iface_index;
};

#define KEY_ERROR	  0x01

#define KEY_PRESS	  0
#define KEY_RELEASE	  0x400
#define KEY_INDEX(c)	  ((c) & 0xFF)

#define SCAN_PRESS	  0
#define SCAN_RELEASE	  0x80
#define SCAN_PREFIX_E0	  0x100
#define SCAN_PREFIX_E1	  0x200
#define SCAN_PREFIX_CTL	  0x400
#define SCAN_PREFIX_SHIFT 0x800
#define SCAN_PREFIX	(SCAN_PREFIX_E0  | SCAN_PREFIX_E1 | \
			 SCAN_PREFIX_CTL | SCAN_PREFIX_SHIFT)
#define SCAN_CHAR(c)	((c) & 0x7f)

static const struct {
	u_int32_t mask, key;
} ukbd_mods[UKBD_NMOD] = {
	{ MOD_CONTROL_L, 0xe0 },
	{ MOD_CONTROL_R, 0xe4 },
	{ MOD_SHIFT_L,   0xe1 },
	{ MOD_SHIFT_R,   0xe5 },
	{ MOD_ALT_L,     0xe2 },
	{ MOD_ALT_R,     0xe6 },
	{ MOD_WIN_L,     0xe3 },
	{ MOD_WIN_R,	 0xe7 },
};

#define NN 0			/* no translation */
/*
 * Translate USB keycodes to AT keyboard scancodes.
 */
/*
 * FIXME: Mac USB keyboard generates:
 * 0x53: keypad NumLock/Clear
 * 0x66: Power
 * 0x67: keypad =
 * 0x68: F13
 * 0x69: F14
 * 0x6a: F15
 */
static const u_int8_t ukbd_trtab[256] = {
	   0,   0,   0,   0,  30,  48,  46,  32, /* 00 - 07 */
	  18,  33,  34,  35,  23,  36,  37,  38, /* 08 - 0F */
	  50,  49,  24,  25,  16,  19,  31,  20, /* 10 - 17 */
	  22,  47,  17,  45,  21,  44,   2,   3, /* 18 - 1F */
	   4,   5,   6,   7,   8,   9,  10,  11, /* 20 - 27 */
	  28,   1,  14,  15,  57,  12,  13,  26, /* 28 - 2F */
	  27,  43,  43,  39,  40,  41,  51,  52, /* 30 - 37 */
	  53,  58,  59,  60,  61,  62,  63,  64, /* 38 - 3F */
	  65,  66,  67,  68,  87,  88,  92,  70, /* 40 - 47 */
	 104, 102,  94,  96, 103,  99, 101,  98, /* 48 - 4F */
	  97, 100,  95,  69,  91,  55,  74,  78, /* 50 - 57 */
	  89,  79,  80,  81,  75,  76,  77,  71, /* 58 - 5F */
          72,  73,  82,  83,  86, 107, 122,  NN, /* 60 - 67 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* 68 - 6F */
          NN,  NN,  NN,  NN, 115, 108, 111, 113, /* 70 - 77 */
         109, 110, 112, 118, 114, 116, 117, 119, /* 78 - 7F */
         121, 120,  NN,  NN,  NN,  NN,  NN, 115, /* 80 - 87 */
         112, 125, 121, 123,  NN,  NN,  NN,  NN, /* 88 - 8F */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* 90 - 97 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* 98 - 9F */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* A0 - A7 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* A8 - AF */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* B0 - B7 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* B8 - BF */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* C0 - C7 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* C8 - CF */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* D0 - D7 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* D8 - DF */
          29,  42,  56, 105,  90,  54,  93, 106, /* E0 - E7 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* E8 - EF */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* F0 - F7 */
          NN,  NN,  NN,  NN,  NN,  NN,  NN,  NN, /* F8 - FF */
};

/* prototypes */
static void		ukbd_timeout(void *arg);
static void		ukbd_set_leds(struct ukbd_softc *sc, u_int8_t leds);
static int		ukbd_set_typematic(keyboard_t *kbd, int code);
#ifdef UKBD_EMULATE_ATSCANCODE
static int		ukbd_key2scan(struct ukbd_softc *sc, int keycode, 
				      int shift, int up);
#endif
static u_int32_t	ukbd_read_char(keyboard_t *kbd, int wait);
static void		ukbd_clear_state(keyboard_t *kbd);
static int		ukbd_ioctl(keyboard_t *kbd, u_long cmd, caddr_t arg);
static int		ukbd_enable(keyboard_t *kbd);
static int		ukbd_disable(keyboard_t *kbd);
static void		ukbd_interrupt(struct ukbd_softc *sc);

static device_probe_t ukbd_probe;
static device_attach_t ukbd_attach;
static device_detach_t ukbd_detach;
static device_resume_t ukbd_resume;

static void
ukbd_put_key(struct ukbd_softc *sc, u_int32_t key)
{
	mtx_assert(&Giant, MA_OWNED);

	DPRINTF(0, "0x%02x (%d) %s\n", key, key,
		(key & KEY_RELEASE) ? "released" : "pressed");

	if (sc->sc_inputs < UKBD_IN_BUF_SIZE) {
	    sc->sc_input[sc->sc_inputtail] = key;
	    ++(sc->sc_inputs);
	    ++(sc->sc_inputtail);
	    if (sc->sc_inputtail >= UKBD_IN_BUF_SIZE) {
	        sc->sc_inputtail = 0;
	    }
	} else {
	  DPRINTF(0, "input buffer is full\n");
	}
	return;
}

static int32_t
ukbd_get_key(struct ukbd_softc *sc)
{
	int32_t c;

	mtx_assert(&Giant, MA_OWNED);

	if (sc->sc_inputs == 0) {
	    /* start transfer, if not already started */
	    usbd_transfer_start(sc->sc_xfer[0]);
	}

	if (sc->sc_flags & UKBD_FLAG_POLLING) {
	    DPRINTF(1, "polling\n");

	    while (sc->sc_inputs == 0) {

	        usbd_do_poll(sc->sc_udev);

		DELAY(1000); /* delay 1 ms */

		sc->sc_time_ms ++;

		/* support repetition of keys: */

		ukbd_interrupt(sc);
	    }
	}

	if (sc->sc_inputs == 0) {
	    c = -1;
	} else {
	    c = sc->sc_input[sc->sc_inputhead];
	    --(sc->sc_inputs);
	    ++(sc->sc_inputhead);
	    if (sc->sc_inputhead >= UKBD_IN_BUF_SIZE) {
	        sc->sc_inputhead = 0;
	    }
	}
	return c;
}

static void
ukbd_interrupt(struct ukbd_softc *sc)
{
	u_int32_t n_mod;
	u_int32_t o_mod;
	u_int32_t now = sc->sc_time_ms;
	u_int32_t dtime;
	u_int32_t c;
	u_int8_t key;
	u_int8_t i;
	u_int8_t j;

	if (sc->sc_ndata.keycode[0] == KEY_ERROR) {
	    goto done;
	}

	n_mod = sc->sc_ndata.modifiers;
	o_mod = sc->sc_odata.modifiers;
	if (n_mod != o_mod) {
	    for (i = 0; i < UKBD_NMOD; i++) {
	        if ((n_mod & ukbd_mods[i].mask) !=
		    (o_mod & ukbd_mods[i].mask)) {
		    ukbd_put_key(sc, ukbd_mods[i].key |
				 ((n_mod & ukbd_mods[i].mask) ? 
				  KEY_PRESS : KEY_RELEASE));
		}
	    }
	}

	/* Check for released keys. */
	for (i = 0; i < UKBD_NKEYCODE; i++) {
	    key = sc->sc_odata.keycode[i];
	    if (key == 0) {
	        continue;
	    }
	    for (j = 0; j < UKBD_NKEYCODE; j++) {
	        if (sc->sc_ndata.keycode[j] == 0) {
		    continue;
		}
		if (key == sc->sc_ndata.keycode[j]) {
		    goto rfound;
		}
	    }
	    ukbd_put_key(sc, key | KEY_RELEASE);
	rfound:;
	}

	/* Check for pressed keys. */
	for (i = 0; i < UKBD_NKEYCODE; i++) {
	    key = sc->sc_ndata.keycode[i];
	    if (key == 0) {
	        continue;
	    }
	    sc->sc_ntime[i] = now + sc->sc_kbd.kb_delay1;
	    for (j = 0; j < UKBD_NKEYCODE; j++) {
	        if (sc->sc_odata.keycode[j] == 0) {
		    continue;
		}
		if (key == sc->sc_odata.keycode[j]) {

		    /* key is still pressed */

		    sc->sc_ntime[i] = sc->sc_otime[j];
		    dtime = (sc->sc_otime[j] - now);

		    if (!(dtime & 0x80000000)) {
		        /* time has not elapsed */
		        goto pfound;
		    }
		    sc->sc_ntime[i] = now + sc->sc_kbd.kb_delay2;
		    break;
		}
	    }
	    ukbd_put_key(sc, key | KEY_PRESS);
	pfound:;
	}

	sc->sc_odata = sc->sc_ndata;

	bcopy(sc->sc_ntime, sc->sc_otime, sizeof(sc->sc_otime));

	if (sc->sc_inputs == 0) {
	    goto done;
	}

	if (sc->sc_flags & UKBD_FLAG_POLLING) {
	    goto done;
	}

	if (KBD_IS_ACTIVE(&(sc->sc_kbd)) && 
	    KBD_IS_BUSY(&(sc->sc_kbd))) {
	    /* let the callback function process the input */
	    (sc->sc_kbd.kb_callback.kc_func)(&(sc->sc_kbd), KBDIO_KEYINPUT,
					     sc->sc_kbd.kb_callback.kc_arg);
	} else {
	    /* read and discard the input, no one is waiting for it */
	    do {
	        c = ukbd_read_char(&(sc->sc_kbd), 0);
	    } while (c != NOKEY);
	}
 done:
	return;
}

static void
ukbd_timeout(void *arg)
{
	struct ukbd_softc *sc = arg;

	mtx_assert(&Giant, MA_OWNED);

	if (!(sc->sc_flags & UKBD_FLAG_POLLING)) {
	    sc->sc_time_ms += 25; /* milliseconds */
	}

	ukbd_interrupt(sc);

	__callout_reset(&(sc->sc_callout), hz / 40, &ukbd_timeout, sc);

	mtx_unlock(&Giant);

	return;
}

static void
ukbd_clear_stall_callback(struct usbd_xfer *xfer1)
{
	usb_device_request_t *req = xfer1->buffer;
	struct ukbd_softc *sc = xfer1->priv_sc;
	struct usbd_xfer *xfer0 = sc->sc_xfer[0];

	USBD_CHECK_STATUS(xfer1);

 tr_setup:

        /* setup a CLEAR STALL packet */

        req->bmRequestType = UT_WRITE_ENDPOINT;
        req->bRequest = UR_CLEAR_FEATURE;
        USETW(req->wValue, UF_ENDPOINT_HALT);
        req->wIndex[0] = xfer0->pipe->edesc->bEndpointAddress;
        req->wIndex[1] = 0;
        USETW(req->wLength, 0);

        usbd_start_hardware(xfer1);
        return;

 tr_error:
	DPRINTF(0, "error=%s\n", usbd_errstr(xfer1->error));

 tr_transferred:

	sc->sc_flags &= ~UKBD_FLAG_PIPE_ERROR;

	if (xfer1->error != USBD_CANCELLED) {

	    xfer0->pipe->clearstall = 0;
	    xfer0->pipe->toggle_next = 0;

	    usbd_transfer_start(xfer0);
	}
	return;
}

static void
ukbd_intr_callback(struct usbd_xfer *xfer)
{
	struct ukbd_softc *sc = xfer->priv_sc;
	u_int8_t *buf = xfer->buffer;
	u_int16_t len = xfer->actlen;
	u_int8_t i;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
	DPRINTF(0, "actlen=%d bytes\n", len);

	if (len > sizeof(sc->sc_ndata)) {
	    len = sizeof(sc->sc_ndata);
	}

	if (len) {
	    bzero(&(sc->sc_ndata), sizeof(sc->sc_ndata));
	    bcopy(buf, &(sc->sc_ndata), len);
#ifdef USB_DEBUG
	    if (sc->sc_ndata.modifiers) {
	        DPRINTF(0, "mod: 0x%04x\n", sc->sc_ndata.modifiers);
	    }
	    for (i = 0; i < UKBD_NKEYCODE; i++) {
	        if (sc->sc_ndata.keycode[i]) {
		    DPRINTF(0, "[%d] = %d\n", i, sc->sc_ndata.keycode[i]);
		}
	    }
#endif /* USB_DEBUG */
	    ukbd_interrupt(sc);
	}

 tr_setup:
	if (!(sc->sc_flags & UKBD_FLAG_PIPE_ERROR)) {
	    if (sc->sc_inputs < UKBD_IN_BUF_FULL) {
	        usbd_start_hardware(xfer);
	    } else {
	        DPRINTF(0, "input queue is full!\n");
	    }
	}
	return;

 tr_error:
	DPRINTF(0, "error=%s\n", usbd_errstr(xfer->error));

	if (xfer->error != USBD_CANCELLED) {

	    /* start clear stall */
	    sc->sc_flags |= UKBD_FLAG_PIPE_ERROR;

	    usbd_transfer_start(sc->sc_xfer[1]);
	}
	return;
}

static void
ukbd_set_leds_callback(struct usbd_xfer *xfer)
{
	usb_device_request_t *req = xfer->buffer;
	struct ukbd_softc *sc = xfer->priv_sc;

	USBD_CHECK_STATUS(xfer);

 tr_transferred:
 tr_setup:
	if (sc->sc_flags &   UKBD_FLAG_SET_LEDS) {
	    sc->sc_flags &= ~UKBD_FLAG_SET_LEDS;

	    req->bmRequestType = UT_WRITE_CLASS_INTERFACE;
	    req->bRequest = UR_SET_REPORT;
	    USETW2(req->wValue, UHID_OUTPUT_REPORT, 0);
	    USETW(req->wIndex, sc->sc_iface->idesc->bInterfaceNumber);
	    USETW(req->wLength, 1);
	    req->bData[0] = sc->sc_leds;

	    usbd_start_hardware(xfer);
	}
	return;

 tr_error:
	DPRINTF(0, "error=%s\n", usbd_errstr(xfer->error));
	return;
}

static const struct usbd_config ukbd_config[UKBD_N_TRANSFER] = {

    [0] = {
      .type      = UE_INTERRUPT,
      .endpoint  = -1, /* any */
      .direction = UE_DIR_IN,
      .flags     = USBD_SHORT_XFER_OK,
      .bufsize   = 0, /* use wMaxPacketSize */
      .callback  = &ukbd_intr_callback,
    },

    [1] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t),
      .callback  = &ukbd_clear_stall_callback,
      .timeout   = 1000, /* 1 second */
    },

    [2] = {
      .type      = UE_CONTROL,
      .endpoint  = 0x00, /* Control pipe */
      .direction = -1,
      .bufsize   = sizeof(usb_device_request_t) + 1,
      .callback  = &ukbd_set_leds_callback,
      .timeout   = 1000, /* 1 second */
    },
};

static int
ukbd_probe(device_t dev)
{
	keyboard_switch_t *sw = kbd_get_switch(UKBD_DRIVER_NAME);
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	usb_interface_descriptor_t *id;

	DPRINTF(10, "\n");

	if (sw == NULL) {
	    return UMATCH_NONE;
	}

	if (uaa->iface == NULL) {
	    /* attach to ifaces only */
	    return UMATCH_NONE;
	}

	/* check that the keyboard speaks the boot protocol: */
	id = usbd_get_interface_descriptor(uaa->iface);
	if (id
	    && (id->bInterfaceClass == UICLASS_HID)
	    && (id->bInterfaceSubClass == UISUBCLASS_BOOT)
	    && (id->bInterfaceProtocol == UPROTO_BOOT_KEYBOARD)) {
	    return UMATCH_IFACECLASS_IFACESUBCLASS_IFACEPROTO;
	}
	return UMATCH_NONE;
}

static int
ukbd_attach(device_t dev)
{
	struct ukbd_softc *sc = device_get_softc(dev);
	struct usb_attach_arg *uaa = device_get_ivars(dev);
	int32_t unit = device_get_unit(dev);
	keyboard_t *kbd = &(sc->sc_kbd);
	usbd_status err;
	u_int16_t n;

	if (sc == NULL) {
	    return ENOMEM;
	}

	mtx_assert(&Giant, MA_OWNED);

	kbd_init_struct(kbd, UKBD_DRIVER_NAME, KB_OTHER, unit, 0, 0, 0);

	kbd->kb_data = (void *)sc;

	usbd_set_desc(dev, uaa->device);

	sc->sc_udev = uaa->device;
	sc->sc_iface = uaa->iface;
	sc->sc_iface_index = uaa->iface_index;
	sc->sc_mode = K_XLATE;
	sc->sc_iface = uaa->iface;

	__callout_init_mtx(&(sc->sc_callout), &Giant,
			   CALLOUT_RETURNUNLOCKED);

	err = usbd_transfer_setup(uaa->device, uaa->iface_index, sc->sc_xfer, 
				  ukbd_config, UKBD_N_TRANSFER, sc, 
				  &Giant, &(sc->sc_mem_wait));
	if (err) {
	    DPRINTF(0, "error=%s\n", usbd_errstr(err)) ;
	    goto detach;
	}

	/* setup default keyboard maps */

	sc->sc_keymap = key_map;
	sc->sc_accmap = accent_map;
	for (n = 0; n < UKBD_NFKEY; n++) {
	    sc->sc_fkeymap[n] = fkey_tab[n];
	}

	kbd_set_maps(kbd, &(sc->sc_keymap), &(sc->sc_accmap), 
		     sc->sc_fkeymap, UKBD_NFKEY);

	KBD_FOUND_DEVICE(kbd);

	ukbd_clear_state(kbd);

	/*
	 * FIXME: set the initial value for lock keys in "sc_state"
	 * according to the BIOS data?
	 */
	KBD_PROBE_DONE(kbd);

	if ((usbd_get_quirks(uaa->device)->uq_flags & UQ_NO_SET_PROTO) == 0) {

	    err = usbreq_set_protocol(sc->sc_udev, sc->sc_iface_index, 0);

	    DPRINTF(5, "protocol set\n");

	    if (err) {
	        device_printf(dev, "set protocol failed\n");
		goto detach;
	    }
	}

	/* ignore if SETIDLE fails, hence it is not crucial */
	err = usbreq_set_idle(sc->sc_udev, sc->sc_iface_index, 0, 0);

	ukbd_ioctl(kbd, KDSETLED, (caddr_t)&(sc->sc_state));

	KBD_INIT_DONE(kbd);

	if (kbd_register(kbd) < 0) {
	    goto detach;
	}

	KBD_CONFIG_DONE(kbd);

	ukbd_enable(kbd);

#ifdef KBD_INSTALL_CDEV
	if (kbd_attach(kbd)) {
	    goto detach;
	}
#endif
	sc->sc_flags |= UKBD_FLAG_ATTACHED;

	if (bootverbose) {
	    genkbd_diag(kbd, bootverbose);
	}

	/* start the keyboard */

	usbd_transfer_start(sc->sc_xfer[0]);

	/* start the timer */

	mtx_lock(&Giant);

	ukbd_timeout(sc);

	return 0; /* success */

 detach:
	ukbd_detach(dev);
	return ENXIO; /* error */
}

int
ukbd_detach(device_t dev)
{
	struct ukbd_softc *sc = device_get_softc(dev);
	int error;

	mtx_assert(&Giant, MA_OWNED);

	DPRINTF(0, "\n");

	if (sc->sc_flags & UKBD_FLAG_POLLING) {
	    panic("cannot detach polled keyboard!\n");
	}

	sc->sc_flags |= UKBD_FLAG_GONE;

	__callout_stop(&(sc->sc_callout));

	ukbd_disable(&(sc->sc_kbd));

#ifdef KBD_INSTALL_CDEV
	if (sc->sc_flags & UKBD_FLAG_ATTACHED) {
	    error = kbd_detach(&(sc->sc_kbd));
	    if (error) {
	        /* usb attach cannot return an error */
	        device_printf(dev, "WARNING: kbd_detach() "
			      "returned non-zero! (ignored)\n");
	    }
	}
#endif
	if (KBD_IS_CONFIGURED(&(sc->sc_kbd))) {
	    error = kbd_unregister(&(sc->sc_kbd));
	    if (error) {
	        /* usb attach cannot return an error */
	        device_printf(dev, "WARNING: kbd_unregister() "
			      "returned non-zero! (ignored)\n");
	    }
	}

	sc->sc_kbd.kb_flags = 0;

	usbd_transfer_unsetup(sc->sc_xfer, UKBD_N_TRANSFER);

	usbd_transfer_drain(&(sc->sc_mem_wait), &Giant);

	DPRINTF(0, "%s: disconnected\n", 
		device_get_nameunit(dev));

	return 0;
}

static int
ukbd_resume(device_t dev)
{
	struct ukbd_softc *sc = device_get_softc(dev);

	mtx_assert(&Giant, MA_OWNED);

	ukbd_clear_state(&(sc->sc_kbd));

	return 0;
}

/* early keyboard probe, not supported */
static int
ukbd_configure(int flags)
{
	return 0;
}

/* detect a keyboard, not used */
static int
ukbd__probe(int unit, void *arg, int flags)
{
	mtx_assert(&Giant, MA_OWNED);
	return ENXIO;
}

/* reset and initialize the device, not used */
static int
ukbd_init(int unit, keyboard_t **kbdp, void *arg, int flags)
{
	mtx_assert(&Giant, MA_OWNED);
	return ENXIO;
}

/* test the interface to the device, not used */
static int
ukbd_test_if(keyboard_t *kbd)
{
	mtx_assert(&Giant, MA_OWNED);
	return 0;
}

/* finish using this keyboard, not used */
static int
ukbd_term(keyboard_t *kbd)
{
	mtx_assert(&Giant, MA_OWNED);
	return ENXIO;
}

/* keyboard interrupt routine, not used */
static int
ukbd_intr(keyboard_t *kbd, void *arg)
{
	mtx_assert(&Giant, MA_OWNED);
	return 0;
}

/* lock the access to the keyboard, not used */
static int
ukbd_lock(keyboard_t *kbd, int lock)
{
	mtx_assert(&Giant, MA_OWNED);
	return 1;
}

/*
 * Enable the access to the device; until this function is called,
 * the client cannot read from the keyboard.
 */
static int
ukbd_enable(keyboard_t *kbd)
{
	mtx_assert(&Giant, MA_OWNED);
	KBD_ACTIVATE(kbd);
	return 0;
}

/* disallow the access to the device */
static int
ukbd_disable(keyboard_t *kbd)
{
	mtx_assert(&Giant, MA_OWNED);
	KBD_DEACTIVATE(kbd);
	return 0;
}

/* check if data is waiting */
static int
ukbd_check(keyboard_t *kbd)
{
	struct ukbd_softc *sc = kbd->kb_data;

	mtx_assert(&Giant, MA_OWNED);

	if (!KBD_IS_ACTIVE(kbd)) {
	    return 0;
	}
#ifdef UKBD_EMULATE_ATSCANCODE
	if (sc->sc_buffered_char[0]) {
	    return 1;
	}
#endif
	if (sc->sc_inputs > 0) {
	    return 1;
	}
	return 0;
}

/* check if char is waiting */
static int
ukbd_check_char(keyboard_t *kbd)
{
	struct ukbd_softc *sc = kbd->kb_data;

	mtx_assert(&Giant, MA_OWNED);

	if (!KBD_IS_ACTIVE(kbd)) {
	    return 0;
	}
	if ((sc->sc_composed_char > 0) &&
	    (!(sc->sc_flags & UKBD_FLAG_COMPOSE))) {
	    return 1;
	}
	return ukbd_check(kbd);
}


/* read one byte from the keyboard if it's allowed */
static int
ukbd_read(keyboard_t *kbd, int wait)
{
	struct ukbd_softc *sc = kbd->kb_data;
	int32_t usbcode;
#ifdef UKBD_EMULATE_ATSCANCODE
	u_int32_t keycode;
	u_int32_t scancode;
#endif

	mtx_assert(&Giant, MA_OWNED);

#ifdef UKBD_EMULATE_ATSCANCODE
	if (sc->sc_buffered_char[0]) {
	    scancode = sc->sc_buffered_char[0];
	    if (scancode & SCAN_PREFIX) {
	        sc->sc_buffered_char[0] &= ~SCAN_PREFIX;
		return ((scancode & SCAN_PREFIX_E0) ? 0xe0 : 0xe1);
	    }
	    sc->sc_buffered_char[0] = sc->sc_buffered_char[1];
	    sc->sc_buffered_char[1] = 0;
	    return scancode;
	}
#endif /* UKBD_EMULATE_ATSCANCODE */

	/* XXX */
	usbcode = ukbd_get_key(sc);
	if (!KBD_IS_ACTIVE(kbd) || (usbcode == -1)) {
	    return -1;
	}

	++(kbd->kb_count);

#ifdef UKBD_EMULATE_ATSCANCODE
	keycode = ukbd_trtab[KEY_INDEX(usbcode)];
	if (keycode == NN) {
	    return -1;
	}
	return ukbd_key2scan(sc, keycode, sc->sc_ndata.modifiers, 
			     (usbcode & KEY_RELEASE));
#else /* !UKBD_EMULATE_ATSCANCODE */
	return usbcode;
#endif /* UKBD_EMULATE_ATSCANCODE */
}

/* read char from the keyboard */
static u_int32_t
ukbd_read_char(keyboard_t *kbd, int wait)
{
	struct ukbd_softc *sc = kbd->kb_data;
	u_int32_t action;
	u_int32_t keycode;
	int32_t usbcode;
#ifdef UKBD_EMULATE_ATSCANCODE
	u_int32_t scancode;
#endif

	mtx_assert(&Giant, MA_OWNED);

 next_code:

	/* do we have a composed char to return ? */

	if ((sc->sc_composed_char > 0) &&
	    (!(sc->sc_flags & UKBD_FLAG_COMPOSE))) {

	    action = sc->sc_composed_char;
	    sc->sc_composed_char = 0;

	    if (action > 0xFF) {
	        goto errkey;
	    }
	    goto done;
	}

#ifdef UKBD_EMULATE_ATSCANCODE

	/* do we have a pending raw scan code? */

	if (sc->sc_mode == K_RAW) {
	    scancode = sc->sc_buffered_char[0];
	    if (scancode) {
		  if (scancode & SCAN_PREFIX) {
		      sc->sc_buffered_char[0] = (scancode & ~SCAN_PREFIX);
		      return ((scancode & SCAN_PREFIX_E0) ? 0xe0 : 0xe1);
		  }
		  sc->sc_buffered_char[0] = sc->sc_buffered_char[1];
		  sc->sc_buffered_char[1] = 0;
		  return scancode;
	    }
	}
#endif /* UKBD_EMULATE_ATSCANCODE */

	/* see if there is something in the keyboard port */
	/* XXX */
	usbcode = ukbd_get_key(sc);
	if (usbcode == -1) {
	    return NOKEY;
	}
	++kbd->kb_count;

#ifdef UKBD_EMULATE_ATSCANCODE
	/* USB key index -> key code -> AT scan code */
	keycode = ukbd_trtab[KEY_INDEX(usbcode)];
	if (keycode == NN) {
	    return NOKEY;
	}

	/* return an AT scan code for the K_RAW mode */
	if (sc->sc_mode == K_RAW) {
	    return ukbd_key2scan(sc, keycode, sc->sc_ndata.modifiers,
				 (usbcode & KEY_RELEASE));
	}
#else /* !UKBD_EMULATE_ATSCANCODE */

	/* return the byte as is for the K_RAW mode */
	if (sc->sc_mode == K_RAW) {
	    return usbcode;
	}

	/* USB key index -> key code */
	keycode = ukbd_trtab[KEY_INDEX(usbcode)];
	if (keycode == NN) {
	    return NOKEY;
	}

#endif /* UKBD_EMULATE_ATSCANCODE */

	switch (keycode) {
	case 0x38:	/* left alt (compose key) */
		if (usbcode & KEY_RELEASE) {
		    if (sc->sc_flags &   UKBD_FLAG_COMPOSE) {
		        sc->sc_flags &= ~UKBD_FLAG_COMPOSE;

			if (sc->sc_composed_char > 0xFF) {
			    sc->sc_composed_char = 0;
			}
		    }
		} else {
		    if (!(sc->sc_flags & UKBD_FLAG_COMPOSE)) {
		        sc->sc_flags |=  UKBD_FLAG_COMPOSE;
			sc->sc_composed_char = 0;
		    }
		}
		break;
	/* XXX: I don't like these... */
	case 0x5c:	/* print screen */
		if (sc->sc_flags & ALTS) {
		    keycode = 0x54; /* sysrq */
		}
		break;
	case 0x68:	/* pause/break */
		if (sc->sc_flags & CTLS) {
		    keycode = 0x6c; /* break */
		}
		break;
	}

	/* return the key code in the K_CODE mode */
	if (usbcode & KEY_RELEASE) {
	    keycode |= SCAN_RELEASE;
	}

	if (sc->sc_mode == K_CODE) {
	    return keycode;
	}

	/* compose a character code */
	if (sc->sc_flags & UKBD_FLAG_COMPOSE) {
		switch (keycode) {
		/* key pressed, process it */
		case 0x47: case 0x48: case 0x49:	/* keypad 7,8,9 */
			sc->sc_composed_char *= 10;
			sc->sc_composed_char += keycode - 0x40;
			goto check_composed;

		case 0x4B: case 0x4C: case 0x4D:	/* keypad 4,5,6 */
			sc->sc_composed_char *= 10;
			sc->sc_composed_char += keycode - 0x47;
			goto check_composed;

		case 0x4F: case 0x50: case 0x51:	/* keypad 1,2,3 */
			sc->sc_composed_char *= 10;
			sc->sc_composed_char += keycode - 0x4E;
			goto check_composed;

		case 0x52:				/* keypad 0 */
			sc->sc_composed_char *= 10;
			goto check_composed;

		/* key released, no interest here */
		case SCAN_RELEASE | 0x47:
		case SCAN_RELEASE | 0x48:
		case SCAN_RELEASE | 0x49:		/* keypad 7,8,9 */
		case SCAN_RELEASE | 0x4B:
		case SCAN_RELEASE | 0x4C:
		case SCAN_RELEASE | 0x4D:		/* keypad 4,5,6 */
		case SCAN_RELEASE | 0x4F:
		case SCAN_RELEASE | 0x50:
		case SCAN_RELEASE | 0x51:		/* keypad 1,2,3 */
		case SCAN_RELEASE | 0x52:		/* keypad 0 */
			goto next_code;

		case 0x38:				/* left alt key */
			break;

		default:
			if (sc->sc_composed_char > 0) {
			    sc->sc_flags &= ~UKBD_FLAG_COMPOSE;
			    sc->sc_composed_char = 0;
			    goto errkey;
			}
			break;
		}
	}

	/* keycode to key action */
	action = genkbd_keyaction(kbd, SCAN_CHAR(keycode),
				  (keycode & SCAN_RELEASE), 
				  &sc->sc_state, &sc->sc_accents);
	if (action == NOKEY) {
	    goto next_code;
	}
 done:
	return action;

 check_composed:
	if (sc->sc_composed_char <= 0xFF) {
	    goto next_code;
	}

 errkey:
	return ERRKEY;
}

/* some useful control functions */
static int
ukbd_ioctl(keyboard_t *kbd, u_long cmd, caddr_t arg)
{
	/* translate LED_XXX bits into the device specific bits */
	static const u_int8_t ledmap[8] = {
	    0, 2, 1, 3, 4, 6, 5, 7,
	};
	struct ukbd_softc *sc = kbd->kb_data;
	int i;

	mtx_assert(&Giant, MA_OWNED);

	switch (cmd) {
	case KDGKBMODE:		/* get keyboard mode */
		*(int *)arg = sc->sc_mode;
		break;
	case KDSKBMODE:		/* set keyboard mode */
		switch (*(int *)arg) {
		case K_XLATE:
		    if (sc->sc_mode != K_XLATE) {
		        /* make lock key state and LED state match */
		        sc->sc_state &= ~LOCK_MASK;
			sc->sc_state |= KBD_LED_VAL(kbd);
		    }
			/* FALLTHROUGH */
		case K_RAW:
		case K_CODE:
		    if (sc->sc_mode != *(int *)arg) {
		        ukbd_clear_state(kbd);
			sc->sc_mode = *(int *)arg;
		    }
		    break;
		default:
		    return EINVAL;
		}
		break;

	case KDGETLED:		/* get keyboard LED */
		*(int *)arg = KBD_LED_VAL(kbd);
		break;
	case KDSETLED:		/* set keyboard LED */
		/* NOTE: lock key state in "sc_state" won't be changed */
		if (*(int *)arg & ~LOCK_MASK) {
		    return EINVAL;
		}
		i = *(int *)arg;
		/* replace CAPS LED with ALTGR LED for ALTGR keyboards */
		if (kbd->kb_keymap->n_keys > ALTGR_OFFSET) {
		    if (i & ALKED)
		        i |= CLKED;
		    else
		        i &= ~CLKED;
		}
		if (KBD_HAS_DEVICE(kbd)) {
		    ukbd_set_leds(sc, ledmap[i & LED_MASK]);
		}
		KBD_LED_VAL(kbd) = *(int *)arg;
		break;

	case KDGKBSTATE:	/* get lock key state */
		*(int *)arg = sc->sc_state & LOCK_MASK;
		break;
	case KDSKBSTATE:	/* set lock key state */
		if (*(int *)arg & ~LOCK_MASK) {
		    return EINVAL;
		}
		sc->sc_state &= ~LOCK_MASK;
		sc->sc_state |= *(int *)arg;

		/* set LEDs and quit */
		return ukbd_ioctl(kbd, KDSETLED, arg);

	case KDSETREPEAT:	/* set keyboard repeat rate (new interface) */
		if (!KBD_HAS_DEVICE(kbd)) {
		    return 0;
		}
		if (((int *)arg)[1] < 0) {
		    return EINVAL;
		}
		if (((int *)arg)[0] < 0) {
		    return EINVAL;
		}
		if (((int *)arg)[0] < 200)	/* fastest possible value */
		    kbd->kb_delay1 = 200;
		else
		    kbd->kb_delay1 = ((int *)arg)[0];
		kbd->kb_delay2 = ((int *)arg)[1];
		return 0;

	case KDSETRAD:		/* set keyboard repeat rate (old interface) */
		return ukbd_set_typematic(kbd, *(int *)arg);

	case PIO_KEYMAP:	/* set keyboard translation table */
	case PIO_KEYMAPENT:	/* set keyboard translation table entry */
	case PIO_DEADKEYMAP:	/* set accent key translation table */
		sc->sc_accents = 0;
		/* FALLTHROUGH */
	default:
		return genkbd_commonioctl(kbd, cmd, arg);

#ifdef USB_DEBUG
	case USB_SETDEBUG:
		ukbd_debug = *(int *)arg;
		break;
#endif
	}

	return 0;
}

/* clear the internal state of the keyboard */
static void
ukbd_clear_state(keyboard_t *kbd)
{
	struct ukbd_softc *sc = kbd->kb_data;

	mtx_assert(&Giant, MA_OWNED);

	sc->sc_flags &= ~(UKBD_FLAG_COMPOSE|UKBD_FLAG_POLLING);
	sc->sc_state &= LOCK_MASK;	/* preserve locking key state */
	sc->sc_accents = 0;
	sc->sc_composed_char = 0;
#ifdef UKBD_EMULATE_ATSCANCODE
	sc->sc_buffered_char[0] = 0;
	sc->sc_buffered_char[1] = 0;
#endif
	bzero(&sc->sc_ndata, sizeof(sc->sc_ndata));
	bzero(&sc->sc_odata, sizeof(sc->sc_odata));
	bzero(&sc->sc_ntime, sizeof(sc->sc_ntime));
	bzero(&sc->sc_otime, sizeof(sc->sc_otime));
	return;
}

/* save the internal state, not used */
static int
ukbd_get_state(keyboard_t *kbd, void *buf, size_t len)
{
	mtx_assert(&Giant, MA_OWNED);
	return (len == 0) ? 1 : -1;
}

/* set the internal state, not used */
static int
ukbd_set_state(keyboard_t *kbd, void *buf, size_t len)
{
	mtx_assert(&Giant, MA_OWNED);
	return EINVAL;
}

static int
ukbd_poll(keyboard_t *kbd, int on)
{
	struct ukbd_softc *sc = kbd->kb_data;

	mtx_assert(&Giant, MA_OWNED);

	if (on) {
	    sc->sc_flags |= UKBD_FLAG_POLLING;
	} else {
	    sc->sc_flags &= ~UKBD_FLAG_POLLING;
	}
	return 0;
}

/* local functions */

static void
ukbd_set_leds(struct ukbd_softc *sc, u_int8_t leds)
{
	DPRINTF(0, "leds=0x%02x\n", leds);

	sc->sc_leds = leds;
	sc->sc_flags |= UKBD_FLAG_SET_LEDS;

	/* start transfer, if not already started */

	usbd_transfer_start(sc->sc_xfer[2]);

	return;
}

static int
ukbd_set_typematic(keyboard_t *kbd, int code)
{
	static const int delays[] = { 250, 500, 750, 1000 };
	static const int rates[] = {  34,  38,  42,  46,  50,  55,  59,  63,
				      68,  76,  84,  92, 100, 110, 118, 126,
				      136, 152, 168, 184, 200, 220, 236, 252,
				      272, 304, 336, 368, 400, 440, 472, 504 };
	if (code & ~0x7f) {
	    return EINVAL;
	}
	kbd->kb_delay1 = delays[(code >> 5) & 3];
	kbd->kb_delay2 = rates[code & 0x1f];
	return 0;
}

#ifdef UKBD_EMULATE_ATSCANCODE
static int
ukbd_key2scan(struct ukbd_softc *sc, int code, int shift, int up)
{
	static const int scan[] = {
		0x1c, 0x1d, 0x35,
		0x37 | SCAN_PREFIX_SHIFT, /* PrintScreen */
		0x38, 0x47, 0x48, 0x49, 0x4b, 0x4d, 0x4f,
		0x50, 0x51, 0x52, 0x53,
		0x46, 	/* XXX Pause/Break */
		0x5b, 0x5c, 0x5d,
		/* SUN TYPE 6 USB KEYBOARD */
		0x68, 0x5e, 0x5f, 0x60,	0x61, 0x62, 0x63,
		0x64, 0x65, 0x66, 0x67, 0x25, 0x1f, 0x1e,
		0x20, 
	};

	if ((code >= 89) && (code < (89 + (sizeof(scan)/sizeof(scan[0]))))) {
	    code = scan[code - 89] | SCAN_PREFIX_E0;
	}

	/* Pause/Break */
	if ((code == 104) && (!(shift & (MOD_CONTROL_L | MOD_CONTROL_R)))) {
	    code = (0x45 | SCAN_PREFIX_E1 | SCAN_PREFIX_CTL);
	}

	if (shift & (MOD_SHIFT_L | MOD_SHIFT_R)) {
	    code &= ~SCAN_PREFIX_SHIFT;
	}

	code |= (up ? SCAN_RELEASE : SCAN_PRESS);

	if (code & SCAN_PREFIX) {
	    if (code & SCAN_PREFIX_CTL) {
	        /* Ctrl */
	        sc->sc_buffered_char[0] = (0x1d | (code & SCAN_RELEASE)); 
		sc->sc_buffered_char[1] = (code & ~SCAN_PREFIX);
	    } else if (code & SCAN_PREFIX_SHIFT) {
	        /* Shift */
	        sc->sc_buffered_char[0] = (0x2a | (code & SCAN_RELEASE));
		sc->sc_buffered_char[1] = (code & ~SCAN_PREFIX_SHIFT);
	    } else {
	        sc->sc_buffered_char[0] = (code & ~SCAN_PREFIX);
		sc->sc_buffered_char[1] = 0;
	    }
	    return ((code & SCAN_PREFIX_E0) ? 0xe0 : 0xe1);
	}
	return code;

}
#endif /* UKBD_EMULATE_ATSCANCODE */

keyboard_switch_t ukbdsw = {
  .probe       = &ukbd__probe,
  .init        = &ukbd_init,
  .term        = &ukbd_term,
  .intr        = &ukbd_intr,
  .test_if     = &ukbd_test_if,
  .enable      = &ukbd_enable,
  .disable     = &ukbd_disable,
  .read        = &ukbd_read,
  .check       = &ukbd_check,
  .read_char   = &ukbd_read_char,
  .check_char  = &ukbd_check_char,
  .ioctl       = &ukbd_ioctl,
  .lock        = &ukbd_lock,
  .clear_state = &ukbd_clear_state,
  .get_state   = &ukbd_get_state,
  .set_state   = &ukbd_set_state,
  .get_fkeystr = &genkbd_get_fkeystr,
  .poll        = &ukbd_poll,
  .diag        = &genkbd_diag,
};

KEYBOARD_DRIVER(ukbd, ukbdsw, ukbd_configure);

static int
ukbd_driver_load(module_t mod, int what, void *arg)
{
	switch (what) {
	case MOD_LOAD:
	    kbd_add_driver(&ukbd_kbd_driver);
	    break;
	case MOD_UNLOAD:
	    kbd_delete_driver(&ukbd_kbd_driver);
	    break;
	}
	return usbd_driver_load(mod, what, 0);
}

static devclass_t ukbd_devclass;

static device_method_t ukbd_methods[] = {
  DEVMETHOD(device_probe, ukbd_probe),
  DEVMETHOD(device_attach, ukbd_attach),
  DEVMETHOD(device_detach, ukbd_detach),
  DEVMETHOD(device_resume, ukbd_resume),
  { 0, 0 }
};

static driver_t ukbd_driver = {
  .name    = "ukbd",
  .methods = ukbd_methods,
  .size    = sizeof(struct ukbd_softc),
};

DRIVER_MODULE(ukbd, uhub, ukbd_driver, ukbd_devclass, ukbd_driver_load, 0);
MODULE_DEPEND(ukbd, usb, 1, 1, 1);
