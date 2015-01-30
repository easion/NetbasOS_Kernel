/*
** Copyright 2001-2002, Travis Geiselbrecht. All rights reserved.
** Distributed under the terms of the NewOS License.
*/

#include <kernel/kernel.h>
#include <kernel/debug.h>
#include <kernel/lock.h>
#include <kernel/smp.h>
#include <kernel/int.h>
#include <kernel/heap.h>
#include <kernel/thread.h>
#include <kernel/module.h>
#include <kernel/vm.h>
#include <newos/errors.h>

#include <string.h>

#include <kernel/bus/pci/pci.h>

//#include "pcihdr.h"

#define TRACE_PCI 1
#if TRACE_PCI
#	define TRACE(x) dprintf x
#else
#	define TRACE(x) ;
#endif


struct pci_device {
	struct pci_device *next;
	int type;
	pci_info *info;
};

struct pci_bus {
	struct pci_bus *next;
	pci_info *info;
};

enum {
	PCI_DEVICE = 0,
	PCI_HOST_BUS,
	PCI_BRIDGE,
	PCI_CARDBUS
};

static struct pci_device *pci_devices = NULL;
static struct pci_bus    *pci_busses  = NULL;

static spinlock_t pci_config_lock = 0;  /* lock for config space access */
static int        pci_mode = 1;         /* The pci config mechanism we're using.
                                         * NB defaults to 1 as this is more common, but
                                         * checked at runtime
                                         */
static int        bus_max_devices = 32; /* max devices that any bus can support
                                         * Yes, if we're using pci_mode == 2 then
                                         * this is only 16, instead of the 32 we
                                         * have with pci_mode == 1
                                         */
static int        pci_max_bus = 0;      /* maximum bus we've found/configured */
static region_id  pci_region;           /* pci_bios region we map */
static void *     pci_bios_ptr = NULL;  /* virtual address of memory we map */

static void pci_scan_bus(uint8 bus);
static void pci_bridge(uint8 bus, uint8 dev, uint8 func);
static void fill_basic_pci_structure(pci_info *pcii);
static uint32 read_pci_config(uchar, uchar, uchar, uchar, uchar);
static void write_pci_config (uchar, uchar, uchar, uchar, uchar, uint32);

/* XXX - move these to a header file */
#define PCI_VENDOR_INTEL                 0x8086

#define PCI_PRODUCT_INTEL_82371AB_ISA    0x7110 /* PIIX4 ISA */
#define PCI_PRODUCT_INTEL_82371AB_IDE    0x7111 /* PIIX4 IDE */
#define PCI_PRODUCT_INTEL_82371AB_USB    0x7112 /* PIIX4 USB */
#define PCI_PRODUCT_INTEL_82371AB_PMC    0x7113 /* PIIX4 Power Management */

#define PCI_PRODUCT_INTEL_82443BX        0x7190
#define PCI_PRODUCT_INTEL_82443BX_AGP    0x7191
#define PCI_PRODUCT_INTEL_82443BX_NOAGP  0x7192
#define PCI_PRODUCT_INTEL_82845_AGP      0x1a31

/* Config space locking!
 * We need to make sure we only have one access at a time into the config space,
 * so we'll use a spinlock and also disbale interrupts so if we're smp we
 * won't have problems.
 */

#define PCI_LOCK_CONFIG() \
{ \
	int_disable_interrupts(); \
	acquire_spinlock(&pci_config_lock); \
}

#define PCI_UNLOCK_CONFIG() \
{ \
	release_spinlock(&pci_config_lock); \
	int_restore_interrupts(); \
}


/* decode_class
 * DEBUG DEBUG DEBUG
 * Provide a string that describes the class and sub-class.
 * This could/should (?) be expanded to use the api as well.
 */

static char *
decode_class(uint8 base, uint8 sub_class)
{
	switch(base) {
		case 0:
			switch (sub_class) {
				case 0:
					return "legacy (non VGA)";
				case 1:
					return "legacy VGA";
			}
		case 0x01:
			switch (sub_class) {
				case 0:
					return "mass storage: scsi";
				case 1:
					return "mass storage: ide";
				case 2:
					return "mass storage: floppy";
				case 3:
					return "mass storage: ipi";
				case 4:
					return "mass storage: raid";
				case 5:
					return "mass storage: ata";
				case 6:
					return "mass storage: sata";
				case 0x80:
					return "mass storage: other";
			}
		case 0x02:
			switch (sub_class) {
				case 0:
					return "network: ethernet";
				case 1:
					return "network: token ring";
				case 2:
					return "network: fddi";
				case 3:
					return "network: atm";
				case 4:
					return "network: isdn";
				case 0x80:
					return "network: other";
			}
		case 0x03:
			switch (sub_class) {
				case 0:
					return "display: vga";
				case 1:
					return "display: xga";
				case 2:
					return "display: 3d";
				case 0x80:
					return "display: other";
			}
		case 0x04:
			switch (sub_class) {
				case 0:
					return "multimedia device: video";
				case 1:
					return "multimedia device: audio";
				case 2:
					return "multimedia device: telephony";
				case 0x80:
					return "multimedia device: other";
			}
		case 0x05:
			switch (sub_class) {
				case 0:
					return "memory: ram";
				case 1:
					return "memory: flash";
				case 0x80:
					return "memory: other";
			}
		case 0x06:
			switch (sub_class) {
				case 0:
					return "bridge: host bridge";
				case 1:
					return "bridge: isa";
				case 2:
					return "bridge: eisa";
				case 3:
					return "bridge: microchannel";
				case 4:
					return "bridge: PCI";
				case 5:
					return "bridge: PC Card";
				case 6:
					return "bridge: nubus";
				case 7:
					return "bridge: CardBus";
				case 8:
					return "bridge: raceway";
				case 9:
					return "bridge: stpci";
				case 10:
					return "bridge: infiniband";
				case 0x80:
					return "bridge: other";
			}
		case 0x07: return "simple comms controller";
		case 0x08: return "base system peripheral";
		case 0x09: return "input device";
		case 0x0a: return "docking station";
		case 0x0b: return "processor";
		case 0x0c:
			switch (sub_class) {
				case 0:
					return "bus: IEEE1394 FireWire";
				case 1:
					return "bus: ACCESS.bus";
				case 2:
					return "bus: SSA";
				case 3:
					return "bus: USB";
				case 4:
					return "bus: fibre channel";
			}
		case 0x0d: return "wireless";
		case 0x0e: return "intelligent i/o ??";
		case 0x0f: return "satellite";
		case 0x10: return "encryption";
		case 0x11: return "signal processing";
		default: return "unknown";
	}
}


static void
show_pci_details(struct pci_info *p)
{
#if TRACE_PCI
	uint16 ss_vend, ss_dev;
#endif
	uint8 irq = 0;

	if (p->header_type == PCI_header_type_generic)
		irq = p->u.h0.interrupt_line;
	else
		irq = p->u.h1.interrupt_line;

	dprintf("0x%04x:0x%04x : ", p->vendor_id, p->device_id);

	if (irq > 0)
		dprintf("irq %d : ", irq);

	dprintf("location %d:%d:%d : ", p->bus, p->device, p->function);
	dprintf("class %02x:%02x:%02x", p->class_base, p->class_sub,
	        p->class_api);
	dprintf(" => %s\n", decode_class(p->class_base, p->class_sub));

#if TRACE_PCI
	dprintf("\trevision    : %02x\n", p->revision);
	dprintf("\tstatus      : %04x\n",
	        read_pci_config(p->bus, p->device, p->function, PCI_status, 2));
	dprintf("\tcommand     : %04x\n",
	        read_pci_config(p->bus, p->device, p->function, PCI_command, 2));
	dprintf("\tbase address: %08x\n",
	        read_pci_config(p->bus, p->device, p->function, PCI_base_registers, 4));

	dprintf("\tline_size   : %02x\n", p->line_size);
	dprintf("\theader_type : %02x\n", p->header_type);

	if (p->header_type == PCI_header_type_generic) {
		dprintf("Header Type 0 (Generic)\n");
		dprintf("\tcardbus_cis         : %08lx\n", p->u.h0.cardbus_cis);
		dprintf("\trom_base_pci        : %08lx\n", p->u.h0.rom_base_pci);
		dprintf("\tinterrupt_line      : %02x\n", p->u.h0.interrupt_line);
		dprintf("\tinterrupt_pin       : %02x\n", p->u.h0.interrupt_pin);
		ss_vend = p->u.h0.subsystem_vendor_id;
		ss_dev = p->u.h0.subsystem_id;
	} else if (p->header_type == PCI_header_type_PCI_to_PCI_bridge) {
		dprintf("Header Type 1 (PCI-PCI bridge)\n");
		dprintf("\trom_base_pci        : %08lx\n", p->u.h1.rom_base_pci);
		dprintf("\tinterrupt_line      : %02x\n", p->u.h1.interrupt_line);
		dprintf("\tinterrupt_pin       : %02x\n", p->u.h1.interrupt_pin);
		dprintf("\t2ndry status        : %04x\n", p->u.h1.secondary_status);
		dprintf("\tprimary_bus         : %d\n", p->u.h1.primary_bus);
		dprintf("\tsecondary_bus       : %d\n", p->u.h1.secondary_bus);
		dprintf("\tsubordinate_bus     : %d\n", p->u.h1.subordinate_bus);

		ss_vend = p->u.h1.subsystem_vendor_id;
		ss_dev = p->u.h1.subsystem_id;
	} else {
		ss_vend = 0;
		ss_dev = 0;
	}
	dprintf("\tsubsystem_id        : %04x\n", ss_dev);
	dprintf("\tsubsystem_vendor_id : %04x\n", ss_vend);
#endif
}

/* PCI has 2 Configuration Mechanisms. We need to decide which one the
 * PCI Host Bridge is speaking and then speak to it correctly. This is decided
 * in set_pci_mechanism() where the pci_mode value is set to the appropriate
 * value and the bus_max_devices value is set correctly.
 *
 * Mechanism 1
 * ===========
 * Mechanism 1 is the more common one found on modern computers, so presently
 * that's the one that has been tested and added.
 *
 * This has 2 ranges, one for addressing and for data. Basically we write in the details of
 * the configuration information we want (bus, device and function) and then either
 * read or write from the data port.
 *
 * Apparently most modern hardware no longer has Configuration Type #2.
 *
 * XXX - add code for Mechanism Two
 *
 *
 */

#define CONFIG_REQ_PORT    0xCF8
#define CONFIG_DATA_PORT   0xCFC

#define CONFIG_ADDR_1(bus, device, func, reg) \
	(0x80000000 | (bus << 16) | (device << 11) | (func << 8) | (reg & ~3))

#define CONFIG_ADDR_2(dev, reg) \
	(uint16)(0xC00 | (dev << 8) | reg)


static uint32
read_pci_config(uchar bus, uchar device, uchar function, uchar reg, uchar size)
{
	uint32 val = 0;

	PCI_LOCK_CONFIG();

	if (pci_mode == 1) {
		/* write request details */
		out32(CONFIG_ADDR_1(bus, device, function, reg), CONFIG_REQ_PORT);
		/* Now read data back from the data port...
		 * offset for 1 byte can be 1,2 or 3
		 * offset for 2 bytes can be 1 or 2
		 */
		switch (size) {
			case 1:
				val = in8 (CONFIG_DATA_PORT + (reg & 3));
				break;
			case 2:
				if ((reg & 3) != 3)
					val = in16(CONFIG_DATA_PORT + (reg & 3));
				else {
					val = ERR_INVALID_ARGS;
					dprintf("ERROR: read_pci_config: can't read 2 bytes at reg %d (offset 3)!\n",reg);
				}
				break;
			case 4:
				if ((reg & 3) == 0)
					val = in32(CONFIG_DATA_PORT);
				else {
					val = ERR_INVALID_ARGS;
					dprintf("ERROR: read_pci_config: can't read 4 bytes at reg %d (offset != 0)!\n",reg);
				}
				break;
			default:
				dprintf("ERROR: mech #1: read_pci_config: called for %d bytes!!\n", size);
		}
	} else if (pci_mode == 2) {
		if (!(device & 0x10)) {
			out8((uint8)(0xF0 | (function << 1)), 0xCF8);
			out8(bus, 0xCFA);

			switch (size) {
				case 1:
					val = in8 (CONFIG_ADDR_2(device, reg));
					break;
				case 2:
					val = in16(CONFIG_ADDR_2(device, reg));
					break;
				case 4:
					val = in32(CONFIG_ADDR_2(device, reg));
					break;
				default:
					dprintf("ERROR: mech #2: read_pci_config: called for %d bytes!!\n", size);
			}
			out8(0, 0xCF8);
		} else
			val = ERR_INVALID_ARGS;
	} else
		dprintf("PCI: Config Mechanism %d isn't known!\n", pci_mode);

	PCI_UNLOCK_CONFIG();
	return val;
}


static void
write_pci_config(uchar bus, uchar device, uchar function, uchar reg, uchar size, uint32 value)
{
	PCI_LOCK_CONFIG();

	if (pci_mode == 1) {
		/* write request details */
		out32(CONFIG_ADDR_1(bus, device, function, reg), CONFIG_REQ_PORT);
		/* Now read data back from the data port...
		 * offset for 1 byte can be 1,2 or 3
		 * offset for 2 bytes can be 1 or 2
		 */
		switch (size) {
			case 1:
				out8 (value, CONFIG_DATA_PORT + (reg & 3));
				break;
			case 2:
				if ((reg & 3) != 3)
					out16(value, CONFIG_DATA_PORT + (reg & 3));
				else
					dprintf("ERROR: write_pci_config: can't write 2 bytes at reg %d (offset 3)!\n",reg);
				break;
			case 4:
				if ((reg & 3) == 0)
					out32(value, CONFIG_DATA_PORT);
				else
					dprintf("ERROR: write_pci_config: can't write 4 bytes at reg %d (offset != 0)!\n",reg);
				break;
			default:
				dprintf("ERROR: write_pci_config: called for %d bytes!!\n", size);
		}
	} else if (pci_mode == 2) {
		if (!(device & 0x10)) {
			out8((uint8)(0xF0 | (function << 1)), 0xCF8);
			out8(bus, 0xCFA);

			switch (size) {
				case 1:
					out8 (value, CONFIG_ADDR_2(device, reg));
					break;
				case 2:
					out16(value, CONFIG_ADDR_2(device, reg));
					break;
				case 4:
					out32(value, CONFIG_ADDR_2(device, reg));
					break;
				default:
					dprintf("ERROR: write_pci_config: called for %d bytes!!\n", size);
			}
			out8(0, 0xCF8);
		}
	} else
		dprintf("PCI: Config Mechanism %d isn't known!\n", pci_mode);

	PCI_UNLOCK_CONFIG();

	return;
}


/* pci_get_capability
 * Try to get the offset and value of the specified capability from the
 * devices capability list.
 * returns 0 if unable, 1 if succesful
 */

static int
pci_get_capability(uint8 bus, uint8 dev, uint8 func, uint8 cap, uint8 *offs)
{
	uint16 status;
	uint8 cap_data;
	uint8 hdr_type;
	uint8 ofs;
	int maxcount;

	status = read_pci_config(bus, dev, func, PCI_status, 2);
	if (!(status & PCI_status_capabilities))
		return 0;

	hdr_type = read_pci_config(bus, dev, func, PCI_header_type, 1);
	switch (hdr_type & 0x7f) { /* mask off multi function device indicator bit */
		case PCI_header_type_generic:
			ofs = PCI_capabilities_ptr;
			break;
		case PCI_header_type_PCI_to_PCI_bridge:
			dprintf("ERROR: pci_get_capability: PCItoPCI bridge header type has no capabilities pointer???\n");
			return 0;
		case PCI_header_type_cardbus:
			ofs = PCI_capabilities_ptr_2;
			break;
		default:
			dprintf("ERROR: pci_get_capability: unknown PCI header type\n");
			return 0;
	}

	/* the 192 bytes vendor defined configuration space can
	 * hold as maximum 48 times a 32 bit aligned capability,
	 * we use this as abort condition to avoid lock up by
	 * searching in a circular loop on bad hardware
	 */
	maxcount = 48;
	ofs = read_pci_config(bus, dev, func, ofs, 1);
	while (maxcount-- != 0 && ofs != 0) {

		/* mask off low two bits, demanded by PCI standard */
		ofs &= ~3;

		/* PCI specification 2.2, section 6.8.1.1 and following
		 * describe capability ID and next capability position as
		 * two 8 bit values, the "capability ID" is at the 32 bit
		 * aligned position, after it the "next pointer" follows.
		 */

		/* read the 8 bit capability id is at the 32bit aligned ofs position */
		cap_data = read_pci_config(bus, dev, func, ofs, 1);
		if (cap_data == cap) {
			if (offs)
				*offs = ofs;
			return 1;
		}
		/* at ofs + 1, we can read the next capability position */
		ofs = read_pci_config(bus, dev, func, ofs + 1, 1);
	}
	return 0;
}


/* pci_set_power_state
 * This attempts to set the device into one of the 4 power management
 * modes, where PCI_pm_state_d0 is "Full Power" and _d3 is the lowest.
 * The delay's at the end come from code I've seen but they really need
 * to be checked against the spec, which can be found at
 * PCI PM 1.1, section 5.6.1 table 18
 *
 * There is a note on Linux to say that whilst we can jump straight to
 * D0 we can't move to D3 unless we're already at D1, though the code they
 * use to check this seems suspect. We may want to check against the spec.
 *
 * Returns
 * ERR_IO_ERROR if the device doesn't support the power management state requested
 *
 */

static int
pci_set_power_state(uint8 bus, uint8 dev, uint8 func, int state)
{
	uint8 pm_reg, cur_state;
	uint16 cur_status;

	if (pci_get_capability(bus, dev, func, PCI_cap_id_pm, &pm_reg) == 0)
		return ERR_IO_ERROR;

	if (state > PCI_pm_state_d3)
		state = PCI_pm_state_d3;

	cur_status = read_pci_config(bus, dev, func, pm_reg + PCI_pm_status, 1);
	cur_state = cur_status & PCI_pm_mask;

	if (cur_state == state)
		return 0;

	if (state == PCI_pm_state_d1 || state == PCI_pm_state_d2) {
		uint16 pmc;
		pmc = read_pci_config(bus, dev, func, pm_reg + PCI_pm_ctrl, 2);
		if (state == PCI_pm_state_d1 && !(pmc & PCI_pm_d1supp))
			return ERR_IO_ERROR;
		if (state == PCI_pm_state_d2 && !(pmc & PCI_pm_d2supp))
			return ERR_IO_ERROR;
	}

	if (cur_state != PCI_pm_state_d3) {
		cur_status &= ~PCI_pm_mask;
		cur_status |= state;
	}

	write_pci_config(bus, dev, func, pm_reg + PCI_pm_status, 2, cur_status);

	if (state == PCI_pm_state_d3 || cur_state == PCI_pm_state_d3) {
		thread_snooze(10 * 1000);
	} else if (state == PCI_pm_state_d2 || cur_state == PCI_pm_state_d2)
		thread_snooze(200);

	return 0;
}

/* pci_set_bus_master
 * This sets the bus master bit in the command configuration register
 * if it isn't already set.
 */

static void
pci_set_bus_master(uint8 bus, uint8 dev, uint8 func)
{
        uint16 command;

	command = read_pci_config(bus, dev, func, PCI_command, 2);

	// if the bus master bit isn't already set, set it and write back.
        if ( (command & PCI_command_master) != PCI_command_master)
		write_pci_config(bus, dev, func, PCI_command, 2, 
				 command | PCI_command_master);
}

/* This used to be fixup_host_bridges, but some PCI-PCI bridges need
 * to be adjusted as well so I'll make it more general.
 *
 * Partially borrowed from NetBSD.
 */

static void
fixup_bridge(uint8 bus, uint8 dev, uint8 func)
{
	uint16 vendor, device;

	vendor = read_pci_config(bus, dev, func, PCI_vendor_id, 2);
	device = read_pci_config(bus, dev, func, PCI_device_id, 2);

	switch (vendor) {
		case PCI_VENDOR_INTEL:
			switch (device) {
				case PCI_PRODUCT_INTEL_82443BX_AGP:
				case PCI_PRODUCT_INTEL_82443BX_NOAGP: {
					/* BIOS bug workaround
					 * While the datasheet indicates that the only valid setting
					 * for "Idle/Pipeline DRAM Leadoff Timing (IPLDT)" is 01,
					 * some BIOS's do not set these correctly, so we check and
					 * correct if required.
					 */
					uint16 bcreg = read_pci_config(bus, dev, func, 0x76, 2);
					if ((bcreg & 0x0300) != 0x0100) {
						dprintf("Intel 82443BX Host Bridge: Fixing IPDLT setting\n");
						bcreg &= ~0x0300;
						bcreg |= 0x100;
						write_pci_config(bus, dev, func, 0x76, 2, bcreg);
					}
					break;
				}
				case PCI_PRODUCT_INTEL_82845_AGP: {
					/* Foward accesses to VGA memory onto the AGP card
					 *
					 * This is very experimental! Added to see if this will
					 * fix Marcus's problem with this device.
					 */
					uint8 ctrl = read_pci_config(bus, dev, func, 0x3e, 1);

					if ((ctrl & 0x04) != 0x04) {
						dprintf("Enabling VGA_EN1 for Intel 82845 AGP Bridge\n");
						ctrl |= 0x04;
						write_pci_config(bus, dev, func, 0x3e, 1, ctrl);
					}
				}
			}
	}
}


/* Given a vendor/device pairing, do we need to scan through
 * the entire set of funtions? normally the return will be 0,
 * implying we don't need to, but for some it will be 1 which
 * means scan all functions.
 * This function may seem overkill but it prevents scanning
 * functions we don't need to and shoudl reduce the possibility of
 * duplicates being detected.
 */

static int
pci_quirk_multifunction(uint16 vendor, uint16 device)
{
	switch (vendor) {
		case PCI_VENDOR_INTEL:
			switch (device) {
				case PCI_PRODUCT_INTEL_82371AB_ISA:
				case PCI_PRODUCT_INTEL_82371AB_IDE:
				case PCI_PRODUCT_INTEL_82371AB_USB:
				case PCI_PRODUCT_INTEL_82371AB_PMC:
					return 1;
			}
	}
	return 0;
}


/* set_pci_mechanism()
 * Try to determine which configuration mechanism the PCI Host Bridge
 * wants to deal with.
 * XXX - we really should add code to detect and use a PCI BIOS if one
 *       exists, and this code then becomes the fallback. For now we'll
 *       just use this.
 */

static int
set_pci_mechanism(void)
{
	uint32 ckval = 0x80000000;
	/* Start by looking for the older and more limited mechanism 2
	 * as the test will probably work for mechanism 1 as well.
	 *
	 * This code copied/adapted from OpenBSD
	 */
#define PCI_MODE2_ENABLE  0x0cf8
#define PCI_MODE2_FORWARD 0x0cfa
	out8(0, PCI_MODE2_ENABLE);
	out8(0, PCI_MODE2_FORWARD);
	if (in8(PCI_MODE2_ENABLE) == 0 &&
	    in8(PCI_MODE2_FORWARD) == 0) {
		dprintf("PCI_Mechanism 2 test passed\n");
		bus_max_devices = 16;
		pci_mode = 2;
		return 0;
	}

	/* If we get here, the first test (for mechanism 2) failed, so there
	 * is a good chance this one will pass. Basically enable then disable and
	 * make sure we have the same values.
	 */
#define PCI_MODE1_ADDRESS 0x0cf8
	out32(ckval, PCI_MODE1_ADDRESS);
	if (in32(PCI_MODE1_ADDRESS) == ckval) {
		out32(0, PCI_MODE1_ADDRESS);
		if (in32(PCI_MODE1_ADDRESS) == 0) {
			dprintf("PCI_Mechanism 1 test passed\n");
			bus_max_devices = 32;
			pci_mode = 1;
			return 0;
		}
	}

	dprintf("PCI: Failed to find a valid PCI Configuration Mechanism!\n"
	        "PCI: disabled\n");
	pci_mode = 0;

	return -1;
}


/* check_pci()
 * Basically PCI bus #0 "should" contain a PCI Host Bridge.
 * This can be identified by the 8 bit pci class base of 0x06.
 *
 * XXX - this is pretty simplistic and needs improvement. In particular
 *       some Intel & Compaq bridges don't have the correct class base set.
 *       Need to review these. For the time being if this sanity check
 *       fails it won't be a hanging offense, but it will generate a
 *       message asking for the info to be sent to the kernel list so we
 *       can refine this code. :) Assuming anyone ever looks at the
 *       debug output!
 *
 * returns  0 if PCI seems to be OK
 *         -1 if PCI fails the test
 */

static int
check_pci(void)
{
	int dev = 0;

	/* Scan through the first 16 devices on bus 0 looking for
	 * a PCI Host Bridge
	 */
	for (dev = 0; dev < bus_max_devices; dev++) {
		uint8 val = read_pci_config(0, dev, 0, PCI_class_base, 1);
		if (val == 0x06)
			return 0;
	}
	/* Bit wordy, but it needs to be :( */
	dprintf("*** PCI Warning! ***\n"
	        "The PCI sanity check appears to have failed on your system.\n"
	        "This is probably due to the test being used, so please email\n"
	        "\topen-beos-kernel-devel@lists.sourceforge.net\n"
	        "Your assistance will help improve this test :)\n"
	        "***\n"
	        "PCI will attempt to continue normally.\n");
	return -1;
}

/* Intel specific
 *
 * See http://www.microsoft.com/HWDEV/busbios/PCIIRQ.htm
 *
 * Intel boards have a PCI IRQ Routing table that contains details of
 * how things get routed, information we need :(
 */

struct linkmap {
	uint8  pin;
	uint16 possible_irq;
} _PACKED;

struct pir_slot {
	uint8  bus;
	uint8  devfunc;
	struct linkmap linkmap[4];
	uint8  slot;
	uint8  reserved;
} _PACKED;

#define PIR_HEADER "$PIR"

struct pir_header {
	char   signature[4];     /* always '$PIR' */
	uint16 version;
	uint16 tbl_sz;
	uint8  router_bus;
	uint8  router_devfunc;
	uint16 exclusive_irq;
	uint32 compat_vend;
	uint32 miniport;
	uint8  rsvd[11];
	uint8  cksum;
} _PACKED;

struct pir_table {
	struct pir_header hdr;
	struct pir_slot slot[1];
} _PACKED;

#define PIR_DEVICE(devfunc)     (((devfunc) >> 3) & 0x1f)
#define PIR_FUNCTION(devfunc)   ((devfunc) & 0x07)

/* find_pir_table
 * No real magic, just scan through the memory until we find it,
 * or not!
 *
 * When we check the size of the table, we need to satisfy that
 *  size >= size of a pir_table with no pir_slots
 *  size <= size of a pir_table with 32 pir_slots
 * If we find a table, and it's a suitable size we'll check that the
 * checksum is OK. This is done by simply adding up all the bytes and
 * checking that the final value == 0. If it is we return the pointer to the table.
 *
 * Returns NULL if we fail to find a suitable table.
 */

static struct pir_table *
find_pir_table(void)
{
	uint32 mem_addr = (uint32)pci_bios_ptr;
	int range = 0x10000;

	for (; range > 0; range -= 16, mem_addr += 16) {
		if (memcmp((void*)mem_addr, PIR_HEADER, 4) == 0) {
			uint16 size = ((struct pir_header*)mem_addr)->tbl_sz;
			if ((size >= (sizeof(struct pir_header))) &&
			    (size <= (sizeof(struct pir_header) + sizeof(struct pir_slot) * 32))) {
				uint16 i;
				uint8 cksum = 0;

				for (i = 0; i < size; i++)
					cksum += ((uint8*)mem_addr)[i];
				if (cksum == 0)
					return (struct pir_table *)mem_addr;
			}
		}
	}
	return NULL;
}

#if TRACE_PCI
/* print_pir_table
 * Print out the table to debug output
 */

static void
print_pir_table(struct pir_table *tbl)
{
	int i, j;
	int entries = (tbl->hdr.tbl_sz - sizeof(struct pir_header)) / sizeof(struct pir_slot);

	for (i = 0; i < entries; i++) {
		dprintf("PIR slot %d: bus %d, device %d\n",
		        tbl->slot[i].slot, tbl->slot[i].bus,
		        PIR_DEVICE(tbl->slot[i].devfunc));
		for (j = 0; j < 4; j++)
			dprintf("\tINT%c: pin %d possible_irq's %04x\n",
			        'A'+j, tbl->slot[i].linkmap[j].pin,
			        tbl->slot[i].linkmap[j].possible_irq);
	}
	dprintf("*** end of table\n");
}
#endif

/* Scanning for Devices
 * ====================
 *
 * http://www.tldp.org/LDP/tlk/dd/pci.html
 *
 * Although it refers to Linux it gives a good overview of the basic
 * methodology they use for scanning PCI. We've adopted a similar
 * approach here, using the "depthwise" algorithm.
 *
 * We start by scanning a bus, which treats every device it finds the
 * same and passes them onto pci_probe_device().
 * pci_probe_device() sends pci_bridges to pci_bridge() to be setup
 * and deals with type 0 (generic) devices itself.
 *
 * NB presently we don't really cope with type 2 (Cardbus) devices
 *    but will need to eventually.
 */


/**	The values passed in specify the "device" on the bus being searched
 *	that has been identified as a PCI-PCI bridge. We now setup that bridge
 *	and scan the bus it defines.
 *	The bus is initially taken off-line, scanned and then put back on-line
 *
 *	NB We increment the pci_max_bus value before we use mybus in the following
 *	  code and pci_max_bus is incremented before we recurse to preserve the
 *	  correct relationships with nubering.
 *
 *	We initally set the subordinate_bus to 0xff and then adjust it to the max
 *	once we've scanned the bus on the other side of the bridge. See the URL
 *	above for information on why this is done.
 */

static void
pci_bridge(uint8 bus, uint8 dev, uint8 func)
{
	uint16 command;
	uint8 mybus;
	struct pci_device *pcid;
	struct pci_bus *pcib;
	pci_info *pcii;

	TRACE(("pci_bridge()\n"));

	command = read_pci_config(bus, dev, func, PCI_command, 2);
	command &= ~ 0x03;
	write_pci_config(bus, dev, func, PCI_command, 2, command);

	pci_max_bus += 1;
	mybus = pci_max_bus;

	write_pci_config(bus, dev, func, PCI_primary_bus, 1, bus);
	write_pci_config(bus, dev, func, PCI_secondary_bus, 1, mybus);
	write_pci_config(bus, dev, func, PCI_subordinate_bus, 1, 0xff);

	dprintf("PCI-PCI bridge at %d:%d:%d configured as bus %d\n", bus, dev, func, mybus);
	pci_scan_bus(mybus);

	write_pci_config(bus, dev, func, PCI_subordinate_bus, 1, pci_max_bus);

	pcii = (pci_info *)kmalloc(sizeof(pci_info));
	if (!pcii)
		goto pci_bridge_skip_infolist;
	pcid = (struct pci_device *)kmalloc(sizeof(struct pci_device));
	if (!pcid) {
		kfree(pcii);
		goto pci_bridge_skip_infolist;
	}
	pcib = (struct pci_bus *)kmalloc(sizeof(struct pci_bus));
	if (!pcib) {
		kfree(pcii);
		kfree(pcid);
		goto pci_bridge_skip_infolist;
	}

	pcid->info = pcii;
	pcid->type = PCI_BRIDGE;
	pcid->next = NULL;

	pcib->info = pcii;
	pcib->next = NULL;

	pcii->bus = bus;
	pcii->device = dev;
	pcii->function = func;

	fill_basic_pci_structure(pcii);

	pcii->u.h1.rom_base_pci =        read_pci_config(bus, dev, func, PCI_bridge_rom_base, 4);
	pcii->u.h1.primary_bus =         read_pci_config(bus, dev, func, PCI_primary_bus, 1);
	pcii->u.h1.secondary_bus =       read_pci_config(bus, dev, func, PCI_secondary_bus, 1);
	pcii->u.h1.secondary_latency =   read_pci_config(bus, dev, func, PCI_secondary_latency, 1);
	pcii->u.h1.secondary_status =    read_pci_config(bus, dev, func, PCI_secondary_status, 2);
	pcii->u.h1.subordinate_bus =     read_pci_config(bus, dev, func, PCI_subordinate_bus, 1);
	pcii->u.h1.memory_base =         read_pci_config(bus, dev, func, PCI_memory_base, 2);
	pcii->u.h1.memory_limit =        read_pci_config(bus, dev, func, PCI_memory_limit, 2);
	pcii->u.h1.prefetchable_memory_base =  read_pci_config(bus, dev, func, PCI_prefetchable_memory_base, 2);
	pcii->u.h1.prefetchable_memory_limit = read_pci_config(bus, dev, func, PCI_prefetchable_memory_limit, 2);
	pcii->u.h1.bridge_control =      read_pci_config(bus, dev, func, PCI_bridge_control, 1);
	pcii->u.h1.subsystem_vendor_id = read_pci_config(bus, dev, func, PCI_sub_vendor_id_1, 2);
	pcii->u.h1.subsystem_id        = read_pci_config(bus, dev, func, PCI_sub_device_id_1, 2);
	pcii->u.h1.interrupt_line =      read_pci_config(bus, dev, func, PCI_interrupt_line, 1);
	pcii->u.h1.interrupt_pin =       read_pci_config(bus, dev, func, PCI_interrupt_pin, 1) & PCI_pin_mask;

	{
		struct pci_device *pd = pci_devices;
		while (pd && pd->next)
			pd = pd->next;

		if (pd)
			pd->next = pcid;
		else
			pci_devices = pcid;
	}
	{
		struct pci_bus *pb = pci_busses;
		while (pb && pb->next)
			pb = pb->next;

		if (pb)
			pb->next = pcib;
		else
			pci_busses = pcib;
	}

	show_pci_details(pcii);

pci_bridge_skip_infolist:
	command |= 0x03;
	write_pci_config(bus, dev, func, PCI_command, 2, command);
	return;
}


/** This is a very, very brief 2 liner for a device... */

static void
debug_show_device(pci_info *pcii)
{
	uint16 status = read_pci_config(pcii->bus, pcii->device, pcii->function, PCI_status, 2);

	dprintf("device @ %d:%d:%d > %s [%04x:%04x]\n", pcii->bus, pcii->device,
	        pcii->function,
	        decode_class(pcii->class_base, pcii->class_sub),
	        pcii->vendor_id, pcii->device_id);

	if ((status & PCI_status_capabilities)) {
		dprintf("Capabilities: ");
		if (pci_get_capability(pcii->bus, pcii->device, pcii->function, PCI_cap_id_agp, NULL))
			dprintf("AGP ");
		if (pci_get_capability(pcii->bus, pcii->device, pcii->function, PCI_cap_id_pm, NULL))
			dprintf("Pwr Mgmt ");
		if (pci_get_capability(pcii->bus, pcii->device, pcii->function, PCI_cap_id_vpd, NULL))
			dprintf("VPD ");
		if (pci_get_capability(pcii->bus, pcii->device, pcii->function, PCI_cap_id_slotid, NULL))
			dprintf("slotID ");
		if (pci_get_capability(pcii->bus, pcii->device, pcii->function, PCI_cap_id_msi, NULL))
			dprintf("MSI ");
		if (pci_get_capability(pcii->bus, pcii->device, pcii->function, PCI_cap_id_hotplug, NULL))
			dprintf("hotplug ");
		dprintf("\n");
	} else
		dprintf("\t(No capabilities exist for this device)\n");
}


static void
pci_device_probe(uint8 bus, uint8 dev, uint8 func)
{
	uint8 base_class, sub_class;
	uint8 type;
	pci_info *pcii;
	struct pci_device *pcid;

	TRACE(("pci_device_probe()\n"));

	if (func > 0) {
		uint16 vend = read_pci_config(bus, dev, func, PCI_vendor_id, 2);
		if (vend == 0xffff)
			return;
	}

	type = read_pci_config(bus, dev, func, PCI_header_type, 1);
	type &= PCI_header_type_mask;
	base_class = read_pci_config(bus, dev, func, PCI_class_base, 1);
	sub_class  = read_pci_config(bus, dev, func, PCI_class_sub, 1);

	if (base_class == PCI_bridge) {
		fixup_bridge(bus, dev, func);
		if (sub_class == PCI_pci) {
			pci_bridge(bus, dev, func);
			return;
		}
	}

	/* If we get here then it's not a bridge, so we add it... */
	pcii = (pci_info *)kmalloc(sizeof(pci_info));
	if (!pcii)
		return;
	pcid = (struct pci_device *)kmalloc(sizeof(struct pci_device));
	if (!pcid) {
		kfree(pcii);
		return;
	}

	pcid->info = pcii;
	pcid->type = PCI_DEVICE;
	pcid->next = NULL;

	pcii->bus = bus;
	pcii->device = dev;
	pcii->function = func;

	fill_basic_pci_structure(pcii);

	if (pcii->class_base == PCI_bridge && pcii->class_sub == PCI_host)
		pcid->type = PCI_HOST_BUS;

	pcii->u.h0.cardbus_cis =         read_pci_config(bus, dev, func, PCI_cardbus_cis, 4);
	pcii->u.h0.subsystem_id =        read_pci_config(bus, dev, func, PCI_subsystem_id, 2);
	pcii->u.h0.subsystem_vendor_id = read_pci_config(bus, dev, func, PCI_subsystem_vendor_id, 2);
	pcii->u.h0.rom_base_pci =        read_pci_config(bus, dev, func, PCI_rom_base, 4);
	pcii->u.h0.interrupt_line =      read_pci_config(bus, dev, func, PCI_interrupt_line, 1);
	pcii->u.h0.interrupt_pin =       read_pci_config(bus, dev, func, PCI_interrupt_pin, 1) & PCI_pin_mask;
	pcii->u.h0.max_latency =         read_pci_config(bus, dev, func, PCI_max_latency, 1);
	pcii->u.h0.min_grant =           read_pci_config(bus, dev, func, PCI_min_grant, 1);

	/* now add to list (at end) */
	{
		struct pci_device *pd = pci_devices;
		while (pd && pd->next)
			pd = pd->next;

		if (pd)
			pd->next = pcid;
		else
			pci_devices = pcid;
	}
	pci_set_power_state(bus, dev, func, PCI_pm_state_d0);

	// if the device appears to want to be a bus master, we'll set
	// it as one.
	// XXX This may not be the right place to do this, but as nowhere else
	//     seems to attempt to do configuration of the device beyond powering
	//     up, and that's done here, it seems like as good a place as any.
	if (pcii->u.h0.max_latency != 0 && pcii->u.h0.min_grant != 0)
		pci_set_bus_master(bus, dev, func);

	debug_show_device(pcii);
}


/**	Scan a bus for PCI devices. For each device that's a possible
 *	we get the vendor_id. If it's 0xffff then it's not a valid
 *	device so we move on.
 *	Valid devices then have their header_type checked to detrmine how
 *	many functions we need to check. However, some devices that support
 *	multiple functions have a header_type of 0 (generic) so we also check
 *	for these using pci_quirk_multifunction().
 *	If it's a multifunction device we scan all 8 possible functions,
 *	otherwise we just probe the first one.
 */

static void
pci_scan_bus(uint8 bus)
{
	uint8 dev = 0, func = 0;
	uint16 vend = 0;

	TRACE(("pci_scan_bus()\n"));

	for (dev = 0; dev < 32; dev++) {
		vend = read_pci_config(bus, dev, 0, PCI_vendor_id, 2);
		if (vend != 0xffff) {
			uint16 device = read_pci_config(bus, dev, func, PCI_device_id, 2);
			uint8 type = read_pci_config(bus, dev, func, PCI_header_type, 1);
			uint8 nfunc = 8;

			type &= PCI_header_type_mask;

			if ((type & PCI_multifunction) == 0
				&& !pci_quirk_multifunction(vend, device))
				nfunc = 1;

			for (func = 0; func < nfunc; func++)
				pci_device_probe(bus, dev, func);
		}
	}
}

// unused
#if 0
static void
scan_pci(void)
{
	int bus, dev, func;

	TRACE(("scan_pci()\n"));

	/* We can have up to 255 busses */
	for (bus = 0; bus < 255; bus++) {
		/* Each bus can have up to 'bus_max_devices' devices on it */
		for (dev = 0; dev <= bus_max_devices; dev++) {
			/* Each device can have up to 8 functions */
			uint16 sv_vendor_id = 0, sv_device_id = 0;

			for (func = 0; func < 8; func++) {
				pci_info *pcii = NULL;
				uint16 vendor_id = read_pci_config(bus, dev, func, 0, 2);
				uint16 device_id;
				uint8 offset;
				/* If we get 0xffff then there is no device here. As there can't
				 * be any gaps in function allocation this tells us that we
				 * can move onto the next device/bus
				 */
				if (vendor_id == 0xffff)
					break;

				device_id = read_pci_config(bus, dev, func, PCI_device_id, 2);

				/* Is this a new device? As we're scanning the functions here, many devices
				 * will supply identical information for all 8 accesses! Try to catch this and
				 * simply move past duplicates. We need to continue scanning in case we miss
				 * a different device at the end.
				 * XXX - is this correct????
				 */
				if (vendor_id == sv_vendor_id && sv_device_id == device_id) {
					/* It's a duplicate */
					continue;
				}
				sv_vendor_id = vendor_id;
				sv_device_id = device_id;

				/* At present we will add a device to our list if we get here,
				 * but we may want to review if we need to add 8 version of the
				 * same device if only the functions differ?
				 */
				if ((pcii = (pci_info *)kmalloc(sizeof(pci_info))) == NULL) {
					dprintf("Failed to get memory for a pic_info structure in scan_pci\n");
					return;
				}

				if (pci_get_capability(bus, dev, func, PCI_cap_id_agp, &offset))
					dprintf("Device @ %d:%d:%d has AGP capability\n", bus, dev, func);
				pci_set_power_state(bus, dev, func, PCI_pm_state_d0);

				/* basic header */
				pcii->bus = bus;
				pcii->device = dev;
				pcii->function = func;
				pcii->vendor_id = vendor_id;
				pcii->device_id = device_id;

				pcii->revision =    read_pci_config(bus, dev, func, PCI_revision, 1);
				pcii->class_api =   read_pci_config(bus, dev, func, PCI_class_api, 1);
				pcii->class_sub =   read_pci_config(bus, dev, func, PCI_class_sub, 1);
				pcii->class_base =  read_pci_config(bus, dev, func, PCI_class_base, 1);

				pcii->header_type = read_pci_config(bus, dev, func, PCI_header_type, 1);
				pcii->header_type &= PCI_header_type_mask;
			}
		}
	}
}
#endif

static void
fill_basic_pci_structure(pci_info *pcii)
{
	uint8 bus = pcii->bus, dev = pcii->device, func = pcii->function;
	uint8 int_line = 0, int_pin = 0;
	int i;

	pcii->vendor_id =   read_pci_config(bus, dev, func, PCI_vendor_id, 2);
	pcii->device_id =   read_pci_config(bus, dev, func, PCI_device_id, 2);

	pcii->revision =    read_pci_config(bus, dev, func, PCI_revision, 1);
	pcii->class_api =   read_pci_config(bus, dev, func, PCI_class_api, 1);
	pcii->class_sub =   read_pci_config(bus, dev, func, PCI_class_sub, 1);
	pcii->class_base =  read_pci_config(bus, dev, func, PCI_class_base, 1);

	pcii->header_type = read_pci_config(bus, dev, func, PCI_header_type, 1);
	pcii->header_type &= PCI_header_type_mask;

	pcii->latency =     read_pci_config(bus, dev, func, PCI_latency, 1);
	pcii->bist =        read_pci_config(bus, dev, func, PCI_bist, 1);
	pcii->line_size =   read_pci_config(bus, dev, func, PCI_line_size, 1);

	if(pcii->header_type == 0) {
		for(i = 0; i < 6; i++) {
			uint32 temp, temp2;

			temp = read_pci_config(bus, dev, func, PCI_base_registers + i*4, 4);
			if(temp) {
				write_pci_config(bus, dev, func, PCI_base_registers + i*4, 4, 0xffffffff);
				temp2 = read_pci_config(bus, dev, func, PCI_base_registers + i*4, 4) & 0xfffffff0;
				write_pci_config(bus, dev, func, PCI_base_registers + i*4, 4, temp);
				temp2 = 1 + ~temp2;
				if(temp & 1) {
					pcii->u.h0.base_registers[i] = temp & 0xfff0;
					pcii->u.h0.base_register_sizes[i] = temp2 & 0xffff;
				} else {
					pcii->u.h0.base_registers[i] = temp & 0xfffffff0;
					pcii->u.h0.base_register_sizes[i] = temp2;
				}
			} else {
				pcii->u.h0.base_registers[i] = 0;
				pcii->u.h0.base_register_sizes[i] = 0;
			}

//			dprintf("basereg %d:%d:%d 0x%x\n", bus, dev, func, pcii->u.h0.base_registers[i]);
//			dprintf("size    %d:%d:%d 0x%x\n", bus, dev, func, pcii->u.h0.base_register_sizes[i]);
		}
	}

	int_line =          read_pci_config(bus, dev, func, PCI_interrupt_line, 1);
	int_pin =           read_pci_config(bus, dev, func, PCI_interrupt_pin, 1);
	int_pin &= PCI_pin_mask;
}


static long
get_nth_pci_info(long index, pci_info *copyto)
{
	struct pci_device *pd = pci_devices;

	while (index-- > 0 && pd)
		pd = pd->next;

	if (index > 0 || !pd || !pd->info)
		return ERR_NOT_FOUND;

	TRACE(("pci_nth_pci_info: returning %d:%d:%d\n", pd->info->bus, pd->info->device, pd->info->function));

	memcpy(copyto, pd->info, sizeof(pci_info));

	return 0;
}

/* I/O routines */

static uint8
pci_read_io_8(int mapped_io_addr)
{
	return in8(mapped_io_addr);
}


static void
pci_write_io_8(int mapped_io_addr, uint8 value)
{
	out8(value, mapped_io_addr);
}


static uint16
pci_read_io_16(int mapped_io_addr)
{
	return in16(mapped_io_addr);
}


static void
pci_write_io_16(int mapped_io_addr, uint16 value)
{
	out16(value, mapped_io_addr);
}


static uint32
pci_read_io_32(int mapped_io_addr )
{
	return in32(mapped_io_addr);
}


static void
pci_write_io_32(int mapped_io_addr, uint32 value)
{
	out32(value, mapped_io_addr);
}


/* pci_module_init
 * This is where we start all the PCI work, or not as the case may be.
 * We start by trying to detect and set the correct configuration mechanism
 * to use. We don't handle PCI BIOS's as that method seems to be falling out
 * of fashion and is Intel specific.
 *
 * Next we try to check if we have working PCI on this machine. check_pci()
 * isn't the smartest routine as it essentially just looks for a Host-PCI
 * bridge, but it will show us if we can continue.
 * NB Until this has been tested with a wider audience we don't consider
 *    success/failure here but spew a large debug message and try to continue.
 * XXX - once we're happy with check_pci() make failing a criminal offense!
 *
 * Next we check to see if we can find a PIR table.
 * NB This is Intel specific and currently included here as a test.
 *
 * Finally we scan for devices starting at bus 0.
 */

static int
pci_module_init(void)
{
	struct pir_table *pirt = NULL;

	TRACE(("pci_module_init()\n"));

	if (set_pci_mechanism() == -1) {
		return -1;
	}

	check_pci();

	pci_region = vm_map_physical_memory(vm_get_kernel_aspace_id(),
	                                    "pci_bios", &pci_bios_ptr,
	                                    REGION_ADDR_ANY_ADDRESS,
	                                    0x10000,
	                                    LOCK_RO | LOCK_KERNEL,
	                                    (addr_t)0xf0000);

	pirt = find_pir_table();
	if (pirt) {
		dprintf("PCI IRQ Routing table found\n");
#if TRACE_PCI
		print_pir_table(pirt);
#endif
	}

	pci_scan_bus(0);

	return 0;
}


/**	Finish the module by releasing the memory we grabbed for
 *	the pci_bios :)
 */

static int
pci_module_uninit(void)
{
	vm_delete_region(vm_get_kernel_aspace_id(), pci_region);
	return 0;
}

static struct pci_module_hooks pci_mod_hooks = {
	&pci_read_io_8,
	&pci_write_io_8,
	&pci_read_io_16,
	&pci_write_io_16,
	&pci_read_io_32,
	&pci_write_io_32,
	&get_nth_pci_info,
	&read_pci_config,
	&write_pci_config,
	NULL,	//	&ram_address
};

static module_header pci_module_header = {
	PCI_BUS_MODULE_NAME,
	MODULE_CURR_VERSION,
	MODULE_KEEP_LOADED,
	&pci_mod_hooks,
	&pci_module_init,
	&pci_module_uninit
};

module_header *modules[]  = {
	&pci_module_header,
	NULL
};

