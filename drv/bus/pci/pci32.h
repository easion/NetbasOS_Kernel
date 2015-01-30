
// ----------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/errno.h>
#if 0
#include <i386/isa_io.h>

#define intel_outb(X,Y) outb(Y,X)
#define intel_outw(X,Y) outw(Y,X)
#define intel_outl(X,Y) outl(Y,X)
#else
#define intel_outb  outb
#define intel_outw outw
#define intel_outl outl
#endif

//! Support capability list.
#define PCI_STATUS_CAP_LIST		0x10

//! PCI command register (offset).
#define PCI_COMMAND			0x04
//! PCI status register (offset).
#define PCI_STATUS			0x06
//! PCI interrupt line register (offset).
#define PCI_INTERRUPT_LINE		0x3C
//! PCI interrupt pin register (offset).
#define PCI_INTERRUPT_PIN		0x3D
//! PCI subsystem vendor id register (offset).
#define PCI_SUBSYSTEM_VENDOR_ID		0x2C
//! PCI subsystem id register (offset).
#define PCI_SUBSYSTEM_ID		0x2E
//! PCI latency timer register (offset).
#define PCI_LATENCY_TIMER		0x0D
//! PCI of first capability list entry (offset).
#define PCI_CAPABILITY_LIST		0x34
//! PCI of first capability list cardbus (offset).
#define PCI_CB_CAPABILITY_LIST		0x14
//! The PCI cache line size register (offset).
#define PCI_CACHE_LINE_SIZE		0x0C
//! The PCI subsystem vendor id register for cardbus (offset).
#define PCI_CB_SUBSYSTEM_VENDOR_ID	0x40
//! The PCI subsystem id register for cardbuses (offset).
#define PCI_CB_SUBSYSTEM_ID		0x42
//! The PCI ROM address register for normal devices (offset).
#define PCI_ROM_ADDRESS			0x30
//! The PCI ROM address register for PCI to PCI bridges (offset).
#define PCI_ROM_ADDRESS_1		0x38

// --- Power Management registers ------------------------------------- //

//! Power Management capabilities register.
#define PCI_PM_PMC              	2
//! Power Management control and status register
#define PCI_PM_CTRL			4

// --- Supported features --------------------------------------------- //

//! D1 power state support.
#define PCI_PM_CAP_D1			0x0200
//! D2 power state support.
#define PCI_PM_CAP_D2			0x0400

// --- Actions -------------------------------------------------------- //

//! Enable ROM address.
#define PCI_ROM_ADDRESS_ENABLE		0x01
//! The device is I/O-based (a.k.a. can perform I/O operations
//! using the ports).
#define PCI_COMMAND_IO			0x01
//! The device is memory-based (a.k.a. can perform I/O operations
//! by a memory-mapped buffer)
#define PCI_COMMAND_MEMORY		0x02
//! Enable bus master (a.k.a. 32-bit DMA).
#define PCI_COMMAND_MASTER		0x04

// --- Resources ------------------------------------------------------ //

//! First base address register. Every PCI device has up to 6 base
//! addresses (6 for normal devices, 2 for PCI to PCI bridges and
//! only 1 for cardbuses).
#define PCI_BASE_ADDRESS_0		0x10
//! The base address is I/O-based (a.k.a. it is a port value).
#define PCI_BASE_ADDRESS_SPACE		0x01
//! The base address is memory-based (a.k.a. it is a memory
//! address value).
#define PCI_BASE_ADDRESS_SPACE_MEMORY	0x00
//! The device is memory-based (a.k.a. can perform I/O operations
//! by a memory-mapped buffer).
#define PCI_IO_RESOURCE_MEM		0x00
//! The device is I/O-based (a.k.a. can perform I/O operations
//! using the ports).
#define PCI_IO_RESOURCE_IO		0x01

// --- Capability lists ----------------------------------------------- //

//! Capability ID.
#define PCI_CAP_LIST_ID			0
//! Power Management.
#define PCI_CAP_ID_PM			0x01
//! Accelerated Graphics Port.
#define PCI_CAP_ID_AGP			0x02
//! Vital Product Data.
#define PCI_CAP_ID_VPD			0x03
//! Slot Identification.
#define PCI_CAP_ID_SLOTID		0x04
//! Message Signalled Interrupts.
#define PCI_CAP_ID_MSI			0x05
//! CompactPCI HotSwap.
#define PCI_CAP_ID_CHSWP		0x06
//! Next capability in the list.
#define PCI_CAP_LIST_NEXT		1
//! Capability defined flags (16-bits).
#define PCI_CAP_FLAGS			2
//! Size of PCI capability.
#define PCI_CAP_SIZEOF			4

// --- Masks ---------------------------------------------------------- //

//! PCI ROM address mask.
#define PCI_ROM_ADDRESS_MASK		(~0x7FFUL)
//! PCI base address mask (for memory-based devices).
#define PCI_BASE_ADDRESS_MEM_MASK	(~0x0FUL )
//! PCI base address mask (for I/O-based devices).
#define PCI_BASE_ADDRESS_IO_MASK	(~0x03UL )
//! Current power state (D0 to D3).
#define PCI_PM_CTRL_STATE_MASK		( 0x0003 )

