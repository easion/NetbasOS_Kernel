
// ----------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//-------------------------------------------------------------------------------


#include "pci32.h"
#include <drv/cache.h>
#include <drv/errno.h>


void pci_read_irq(pci_state_t *cfg)
{
	unsigned char irq;

	pci_read_config_byte(cfg, PCI_INTERRUPT_PIN, &irq);
	if (irq)pci_read_config_byte(cfg, PCI_INTERRUPT_LINE, &irq);

	cfg->irq = irq;
}

u32_t pci_size(u32_t base, unsigned long mask)
{
	// Find the significant bits					//
	u32_t size = mask & base;
	// Get the lowest of them to find the decode size		//
	size = size & ~(size-1);
	// extent = size - 1						//
	return(size-1);
}

int pci_find_capability(pci_state_t *cfg, int cap)
{
	u16_t status;
	u8_t pos, id;
	int ttl = 48;

	 pci_read_config_word(cfg, PCI_STATUS, &status);
	if ( !(status & PCI_STATUS_CAP_LIST) )
		return(0);

	switch (cfg->hdr_type & 0x7F)
	{
		case PCI_HEADER_TYPE_NORMAL:
		case PCI_HEADER_TYPE_BRIDGE:
		pci_read_config_byte(cfg, PCI_CAPABILITY_LIST, &pos);
		break;

		case PCI_HEADER_TYPE_CARDBUS:
		 pci_read_config_byte(cfg, PCI_CB_CAPABILITY_LIST, &pos);
		break;

		default:
		return(0);
		break;
	}

	while (ttl-- && pos>=0x40)
	{
		pos &= ~3;
		pci_read_config_byte(cfg, pos+PCI_CAP_LIST_ID, &id);
		if (id == 0xff)
			break;
		if (id == cap)
			return(pos);
		pci_read_config_byte(cfg, pos+PCI_CAP_LIST_NEXT, &pos);
	}
	return(0);
}

static int pcibios_enable_device_io(pci_state_t *cfg)
{
	u16_t cmd, old_cmd;
	int i;

	//printk("\nLow level enabling PCI device %d:%d... ", cfg->bus, cfg->dev_fn);

	 pci_read_config_word(cfg, PCI_COMMAND, &cmd);
         old_cmd = cmd;
	for (i=0; i<sizeof(cfg->type); i++)
		if (cfg->type[i] == PCI_IO_RESOURCE_IO)
			// Command IO based				//
			cmd |= PCI_COMMAND_IO;

	if ( !(cmd & PCI_COMMAND_IO) )
	{
		// Device is not IO-based				//
		//printk("Device is not IO-based!!!\n");
		return(-EINVAL);
	}

	if ( (cfg->hdr_type & 0x7F) == PCI_HEADER_TYPE_BRIDGE )
	{
		// Any PCI-to-PCI bridge must be enabled by setting	//
		// both I/O space and memory space access bits in the	//
		// command register.					//
		cmd |= PCI_COMMAND_MEMORY;
	}

	// Always enable bus master!!!					//
	cmd |= PCI_COMMAND_MASTER;

	if ( cmd!=old_cmd )
	{
		// Set the cache line and default latency (32)			//
		pci_write_config_word(cfg,
				PCI_CACHE_LINE_SIZE, (32 << 8) | (L1_CACHE_BYTES / sizeof(u32_t)));
		// Enable the appropriate bits in the PCI command register	//
		pci_write_config_word(cfg, PCI_COMMAND, cmd);
		//printk("Enable IO OK!");
	}
	else{
		//printk("Already enabled.");
	}
	return(0);
}

void pci_read_bases(pci_state_t *cfg, int tot_bases, int rom)
{
	u32_t l, sz, reg;
	int i;

	// Clear all previous bases and sizes informations		//
	memset(cfg->base, 0, sizeof(cfg->base));
	memset(cfg->size, 0, sizeof(cfg->size));
	memset(cfg->type, 0, sizeof(cfg->type));

	// Read informations about bases and sizes			//
	for(i=0; i<tot_bases; i++)
	{
		// Read bases and size					//
		reg = PCI_BASE_ADDRESS_0 + (i << 2);
		pci_read_config_dword(cfg, reg, &l);
		pci_write_config_dword(cfg, reg, ~0);
		 pci_read_config_dword(cfg, reg, &sz);
		pci_write_config_dword(cfg, reg, l);

		// Check if informations are valid			//
		if (!sz || sz == 0xFFFFFFFF)
			continue;
		if (l == 0xFFFFFFFF)
			l = 0;

		// Store informations					//
		if ( (l & PCI_BASE_ADDRESS_SPACE) == PCI_BASE_ADDRESS_SPACE_MEMORY )
		{
			cfg->base[i] = l & PCI_BASE_ADDRESS_MEM_MASK;
			cfg->size[i] = pci_size(sz, PCI_BASE_ADDRESS_MEM_MASK);
			cfg->type[i] = PCI_IO_RESOURCE_MEM;
		}
		else
		{
			cfg->base[i] = l & PCI_BASE_ADDRESS_IO_MASK;
			cfg->size[i] = pci_size(sz, PCI_BASE_ADDRESS_IO_MASK);
			cfg->type[i] = PCI_IO_RESOURCE_IO;
		}
	}

	// --- ROM ---							//
	if (rom)
	{
		// Initialize values					//
		cfg->rom_base = 0;
		cfg->rom_size = 0;

		pci_read_config_dword(cfg, rom, &l);
		pci_write_config_dword(cfg, rom, ~PCI_ROM_ADDRESS_ENABLE);
		 pci_read_config_dword(cfg, rom, &sz);
		pci_write_config_dword(cfg, rom, l);
		if (l == 0xFFFFFFFF)
			l = 0;
		if (sz && sz != 0xFFFFFFFF)
		{
			cfg->rom_base = l & PCI_ROM_ADDRESS_MASK;
			sz = pci_size(sz, PCI_ROM_ADDRESS_MASK);
			cfg->rom_size = cfg->rom_size + (unsigned long)sz;
		}
	}

/*	for (i=0; i<tot_bases; i++)
	{
		if(cfg->base[i])
		printk("iobase[%d]:%x size:%x - ", i, cfg->base[i], cfg->size[i]);
	}
	printk("[rombase-%x, size-%u,  ", cfg->rom_base, cfg->rom_size );
	*/
}


bool pci_probe(int bus, int dev, int func, pci_state_t *cfg)
{
	u32_t *temp = (u32_t *) cfg;
	int i;

       pci_state_t cfg1;
       memset(&cfg1, 0, sizeof(pci_state_t));
       cfg1.bus = bus;
       cfg1.dev_fn = dev;
       cfg1.func = func;

	for(i=0; i<4; i++)
		pci_read_config_dword(&cfg1, (i << 2), (unsigned long *)&(temp[i]));

	if(cfg->vendorid == 0xFFFF) return FALSE;

	// Setup the bus, device and function number			//
	cfg->bus = bus;
	cfg->dev_fn = dev;

	// Set the power state to unknown				//
	cfg->current_state = 4;

	// Identify the type of the device				//
	switch(cfg->hdr_type & 0x7F)
	{
		case PCI_HEADER_TYPE_NORMAL:
		// --- NORMAL DEVICE ---				//
		// Read the IRQ line					//
		pci_read_irq(cfg);
		// Read the base memory and I/O addresses		//
		pci_read_bases(cfg, 6, PCI_ROM_ADDRESS);
		// Read subsysem vendor and subsystem device id		//
		  pci_read_config_word(&cfg1, PCI_SUBSYSTEM_VENDOR_ID, &cfg->vendorid);
		 pci_read_config_word(&cfg1, PCI_SUBSYSTEM_ID, &cfg->deviceid);
		break;

		case PCI_HEADER_TYPE_BRIDGE:
		// --- PCI <-> PCI BRIDGE ---				//
		pci_read_bases(cfg, 2, PCI_ROM_ADDRESS_1);
		break;

		case PCI_HEADER_TYPE_CARDBUS:
		// --- PCI CARDBUS ---					//
		// Read the IRQ line					//
		pci_read_irq(cfg);
		// Read the base memory and I/O addresses		//
		pci_read_bases(cfg, 1, 0);
		// Read subsysem vendor and subsystem device id		//
		 pci_read_config_word(&cfg1, PCI_CB_SUBSYSTEM_VENDOR_ID, &cfg->vendorid);
		 pci_read_config_word(&cfg1, PCI_CB_SUBSYSTEM_ID, &cfg->deviceid);
		break;

		default:
		// --- UNKNOW HEADER TYPE ---				//
		break;
	}

	return(TRUE);
}

//! SiS 5597 and 5598 require latency timer set to at most 32 to avoid
//! lockups; otherwise default value is 255.
unsigned int pcibios_max_latency=255;

//! \brief Enable bus-mastering (aka 32-bit DMA) for a PCI device.
//! \param cfg The PCI device structure.
void pci_set_master(pci_state_t *cfg)
{
	u16_t cmd;
	u8_t lat;

	pci_read_config_word(cfg, PCI_COMMAND, &cmd);
	if ( !(cmd & PCI_COMMAND_MASTER) )
	{
		printk("PCI: Enabling bus mastering for device in slot %d:%d\n", cfg->bus, cfg->dev_fn);
		cmd |= PCI_COMMAND_MASTER;
		pci_write_config_word(cfg, PCI_COMMAND, cmd);
	}
	// Check the latency time, because certain BIOSes forget to set	//
	// it properly...						//
	 pci_read_config_byte(cfg, PCI_LATENCY_TIMER, &lat);
	if ( lat < 16 )
		lat = (64 <= pcibios_max_latency) ? 64 : pcibios_max_latency;
	else if ( lat > pcibios_max_latency )
		lat = pcibios_max_latency;
	else
		return;
	printk("PCI: Setting latency timer of device %d:%d to %u\n", cfg->bus, cfg->dev_fn, lat);
	pci_write_config_byte(cfg, PCI_LATENCY_TIMER, lat);
}

int pci_set_power_state(pci_state_t *cfg, int state)
{
	int pm,i;
	u16_t pmcsr;
	u16_t pmc;

	// Bound the state to a valid range				//
	if (state > 3) state = 3;

	// Validate current state.					//
	// Can enter D0 from any state, but we can't go deeper if we're //
	// in a low power state.					//
	if (state > 0 && cfg->current_state > state)
		return(-EINVAL);
	else if (cfg->current_state == state)
		// we're already there 					//
		return(0);

	// find PCI PM capability in list 				//
	pm = pci_find_capability(cfg, PCI_CAP_ID_PM);

	// Abort if the device doesn't support PM capabilities		//
	if (!pm) return(-EIO);

	// Check if this device supports the desired state		//
	if (state == 1 || state == 2)
	{
		pci_read_config_word(cfg, pm+PCI_PM_PMC, &pmc);
		if ( (state == 1 && !(pmc & PCI_PM_CAP_D1)) )
			return(-EIO);
		else if ( (state == 2 && !(pmc & PCI_PM_CAP_D2)) )
			return(-EIO);
	}

	// If we're in D3, force entire word to 0.			//
	// This doesn't affect PME_Status, disables PME_En, and		//
	// sets PowerState to 0.					//
	if ( cfg->current_state>=3 )
		pmcsr = 0;
	else
	{
		pci_read_config_word(cfg, pm+PCI_PM_CTRL, &pmcsr);
		pmcsr &= ~PCI_PM_CTRL_STATE_MASK;
		pmcsr |= state;
	}

	// Enter specified state //
	pci_write_config_word(cfg, pm+PCI_PM_CTRL, pmcsr);

	// Mandatory power management transition delays			//
	// see PCI PM 1.1 5.6.1 table 18				//
	if( (state == 3) || (cfg->current_state == 3) )
	{
		// Set task state to interruptible			//
		// LINUX do it so:					//
		// 	set_current_state(TASK_UNINTERRUPTIBLE);	//
		// 	schedule_timeout(HZ/100);			//
		for(i=0;i<0xffff;i++);
	}
	else if( (state == 2) || (cfg->current_state == 2) )
		// udelay(200);
		mdelay(200);
	cfg->current_state = state;

	return(0);
}

int pci_enable_device(pci_state_t *cfg)
{
	int err, pm;

	//printk("\nPowering on PCI device %d:%d:%d... ", cfg->bus, cfg->dev_fn, cfg->func);
	pm = pci_set_power_state(cfg, 0);

	switch( pm )
	{
		case 0:
		//printk("pm OK!");
		break;

		case (-EIO):
		//printk("Doesn't support PM Capabilities!\n");
		break;

		case (-EINVAL):
		printk("Already in this power state.\n");
		break;
	}

	if ((err = pcibios_enable_device_io(cfg)) < 0)
		return(err);
	return(0);
}

bool pci_find_cfg(pci_state_t *cfg, bool enable)
{
	u16_t bus, dev, func;
	pci_state_t pcfg;

	for (bus=0; bus<4; bus++)
	for (dev=0; dev<32; dev++)
	for (func=0; func<8; func++)
	{
		if ( pci_probe(bus, dev, func, &pcfg) )
		{
			if (cfg->vendorid == pcfg.vendorid &&
				cfg->deviceid == pcfg.deviceid )
			{
				// Device found				//
				memcpy(cfg, &pcfg, sizeof(pci_state_t));
				// Enable the device if required	//
				if (enable)
					pci_enable_device(&pcfg);
				// Free the temporary structure		//
			
				return(TRUE);
			}
		}
	}
	// Device not found						//

	return(FALSE);
}

