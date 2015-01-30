
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama OS  
// Copyright (C) 2002-2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
#include "pci32.h"
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>

struct pci_vendor
{
    int id;
    const char *name;
}pci_vendors[] =
{
    {PCI_VENDOR_COMPAQ, "Compaq"},
    {PCI_VENDOR_NCR, "NCR"},
    {PCI_VENDOR_ATI, "ATI"},
    {PCI_VENDOR_VLSI, "VLSI"},
    {PCI_VENDOR_TSENG, "Tseng"},
    {PCI_VENDOR_WEITEK, "Weitek"},
    {PCI_VENDOR_DEC, "DEC"},
    {PCI_VENDOR_CIRRUS, "Cirrus Logic"},
    {PCI_VENDOR_IBM, "IBM"},
    {PCI_VENDOR_AMD, "AMD"},
    {PCI_VENDOR_TRIDENT, "Trident"},
    {PCI_VENDOR_MATROX, "Matrox"},
    {PCI_VENDOR_NEC, "NEC"},
    {PCI_VENDOR_HP, "HP"},
    {PCI_VENDOR_BUSLOGIC, "Buslogic"},
    {PCI_VENDOR_TI, "Texas Instruments"},
    {PCI_VENDOR_MOTOROLA, "Motorola"},
    {PCI_VENDOR_NUMBER9, "Number 9"},
    {PCI_VENDOR_APPLE, "Apple"},
    {PCI_VENDOR_CYRIX, "Cyrix"},
    {PCI_VENDOR_SUN, "Sun"},
    {PCI_VENDOR_3COM, "3Com"},
    {PCI_VENDOR_ACER, "Acer Labs"},
    {PCI_VENDOR_MITSUBISHI, "Mitsubishi"},
    {PCI_VENDOR_NVIDIA, "Nvidia"},
    {PCI_VENDOR_FORE, "Fore"},
    {PCI_VENDOR_PHILLIPS, "Phillips"},
    {PCI_VENDOR_RENDITION, "Rendition"},
    {PCI_VENDOR_TOSHIBA, "Toshiba"},
    {PCI_VENDOR_ENSONIQ, "Ensoniq"},
    {PCI_VENDOR_ROCKWELL, "Rockwell"},
    {PCI_VENDOR_NETGEAR, "Netgear"},
    {PCI_VENDOR_VMWARE, "VMware"},
    {PCI_VENDOR_S3, "S3"},
    {PCI_VENDOR_INTEL, "Intel"},
    {PCI_VENDOR_ADAPTEC, "Adaptec"},
    {PCI_VENDOR_ADAPTEC2, "Adaptec"},
    {PCI_VENDOR_VIA, "VIA"},
    {PCI_VENDOR_SIS, "SiS"},
    {PCI_VENDOR_02MICRO, "02 MICRO"},
    {PCI_VENDOR_REALTEK, "Realtek"},
    {PCI_VENDOR_UMC, "UMC"},
    {PCI_VENDOR_BROOKTREE, "Brooktree"},
    {PCI_VENDOR_3DFX, "3DFX"},
    {PCI_VENDOR_FIBERLINE, "FiberLine"},
    {PCI_VENDOR_CHIPSANDTECHNOLOGIE, "Chips & Tech."},
};

struct pci_class
{
    int id;
    char *name;
} pci_classes[] =
{
    {0x00, "pre ver 2.0"},
    {0x01, "mass storage controller"},
    {0x02, "network interface"},
    {0x03, "display controller"},
    {0x04, "multimedia controller"},
    {0x05, "memory controller"},
    {0x06, "bridge controller"},
    {0x07, "communications controller"},
    {0x08, "base system peripheral"},
    {0x09, "input controller"},
    {0x0a, "docking station"},
    {0x0b, "processor"},
    {0x0c, "serial bus controller"},
    {0x0d, "wireless interface"},
    {0x0e, "intelligent I/O controller"},
    {0x0f, "satellite interface"},
    {0x10, "encryption/decryption"},
    {0x11, "digital signal processor"},
};


struct pci_storage_subclass
{
    int id;
    char *name;
}pci_storage_subclasses[] =
{
    {0, "SCSI controller"},
    {1, "IDE controller"},
    {2, "floppy controller"},
    {4, "RAID controller"},
};


struct pci_network_subclass
{
    int id;
    char *name;
}pci_network_subclasses[] =
{
    {0, "Ethernet interface"},
    {1, "Token Ring interface"},
    {2, "FDDI interface"},
    {3, "ATM interface"},
    {4, "ISDN interface"},
};


struct pci_display_subclass
{
    int id;
    char *name;
}pci_display_subclasses[] =
{
    {0, "VGA controller"},
    {1, "XGA controller"},
    {2, "3D controller"},
};

struct pci_multimedia_subclass
{
    int id;
    char *name;
} pci_multimedia_subclasses[] =
{
    {0, "video controller"},
    {1, "audio controller"},
    {2, "telephony controller"},
};


struct pci_memory_subclass
{
    int id;
    char *name;
}pci_memory_subclasses[] =
{
    {0, "RAM controller"},
    {1, "Flash controller"},
};


struct pci_bridge_subclass
{
    int id;
    char *name;
}pci_bridge_subclasses[] =
{
    {0x00, "host"},
    {0x01, "ISA"},
    {0x02, "EISA"},
    {0x03, "Microchannel"},
    {0x04, "PCI"},
    {0x05, "PCMCIA"},
    {0x06, "Nubus"},
    {0x07, "Cardbus"},
    {0x08, "Raceway"},
};


struct pci_serial_subclass
{
    int id;
    char *name;
}pci_serial_subclasses[] =
{
    {0, "IEEE 1394 controller"},
    {3, "USB controller"},
    {4, "Fibre Channel controller"},
};

/*pci dev class*/


const struct all_class
{
    u8_t base_class;
    u8_t sub_class;
    u8_t interface_id;
    const u8_t* dev_name;
} my_pci_classes[] =
{
    { 0x00, 0x00, 0x00, "Undefined" },
    { 0x00, 0x01, 0x00, "VGA" },
    /*disk*/
    { 0x01, 0x00, 0x00, "SCSI" },
    { 0x01, 0x01, 0x00, "IDE" },
    { 0x01, 0x02, 0x00, "Floppy" },
    { 0x01, 0x03, 0x00, "IPI" },
    { 0x01, 0x04, 0x00, "RAID" },
    { 0x01, 0x80, 0x00, "Other" },

    /*net*/
    { 0x02, 0x00, 0x00, "Ethernet" },
    { 0x02, 0x01, 0x00, "Token Ring" },
    { 0x02, 0x02, 0x00, "FDDI" },
    { 0x02, 0x03, 0x00, "ATM" },
    { 0x02, 0x04, 0x00, "ISDN" },
    { 0x02, 0x80, 0x00, "Other" },
     
	/*video*/
    { 0x03, 0x00, 0x00, "VGA" },
    { 0x03, 0x00, 0x01, "VGA+8514" },
    { 0x03, 0x01, 0x00, "XGA" },
    { 0x03, 0x02, 0x00, "3D" },
    { 0x03, 0x80, 0x00, "Other" },

    { 0x04, 0x00, 0x00, "Video" },
    { 0x04, 0x01, 0x00, "Audio" },
    { 0x04, 0x02, 0x00, "Telephony" },
    { 0x04, 0x80, 0x00, "Other" },

    /*memory*/
    { 0x05, 0x00, 0x00, "RAM" },
    { 0x05, 0x01, 0x00, "Flash" },
    { 0x05, 0x80, 0x00, "Other" },

    /*pci bus*/
    { 0x06, 0x00, 0x00, "PCI to HOST" },
    { 0x06, 0x01, 0x00, "PCI to ISA" },
    { 0x06, 0x02, 0x00, "PCI to EISA" },
    { 0x06, 0x03, 0x00, "PCI to MCA" },
    { 0x06, 0x04, 0x00, "PCI to PCI" },
    { 0x06, 0x04, 0x01, "PCI to PCI (Subtractive Decode)" },
    { 0x06, 0x05, 0x00, "PCI to PCMCIA" },
    { 0x06, 0x06, 0x00, "PCI to NuBUS" },
    { 0x06, 0x07, 0x00, "PCI to Cardbus" },
    { 0x06, 0x08, 0x00, "PCI to RACEway" },
    { 0x06, 0x09, 0x00, "PCI to PCI" },
    { 0x06, 0x0A, 0x00, "PCI to InfiBand" },
    { 0x06, 0x80, 0x00, "PCI to Other" },

   /*serial*/
    { 0x07, 0x00, 0x00, "Serial" },
    { 0x07, 0x00, 0x01, "Serial - 16450" },
    { 0x07, 0x00, 0x02, "Serial - 16550" },
    { 0x07, 0x00, 0x03, "Serial - 16650" },
    { 0x07, 0x00, 0x04, "Serial - 16750" },
    { 0x07, 0x00, 0x05, "Serial - 16850" },
    { 0x07, 0x00, 0x06, "Serial - 16950" },
    { 0x07, 0x01, 0x00, "Parallel" },
    { 0x07, 0x01, 0x01, "Parallel - BiDir" },
    { 0x07, 0x01, 0x02, "Parallel - ECP" },
    { 0x07, 0x01, 0x03, "Parallel - IEEE1284" },
    { 0x07, 0x01, 0xFE, "Parallel - IEEE1284 Target" },
    { 0x07, 0x02, 0x00, "Multiport Serial" },
    { 0x07, 0x03, 0x00, "Hayes Compatible Modem" },
    { 0x07, 0x03, 0x01, "Hayes Compatible Modem, 16450" },
    { 0x07, 0x03, 0x02, "Hayes Compatible Modem, 16550" },
    { 0x07, 0x03, 0x03, "Hayes Compatible Modem, 16650" },
    { 0x07, 0x03, 0x04, "Hayes Compatible Modem, 16750" },
    { 0x07, 0x80, 0x00, "Other" },

    /*bus type*/
    { 0x08, 0x00, 0x00, "PIC" },
    { 0x08, 0x00, 0x01, "ISA PIC" },
    { 0x08, 0x00, 0x02, "EISA PIC" },
    { 0x08, 0x00, 0x10, "I/O APIC" },
    { 0x08, 0x00, 0x20, "I/O(x) APIC" },
    { 0x08, 0x01, 0x00, "DMA" },
    { 0x08, 0x01, 0x01, "ISA DMA" },
    { 0x08, 0x01, 0x02, "EISA DMA" },
    { 0x08, 0x02, 0x00, "Timer" },
    { 0x08, 0x02, 0x01, "ISA Timer" },
    { 0x08, 0x02, 0x02, "EISA Timer" },
    { 0x08, 0x03, 0x00, "RTC" },
    { 0x08, 0x03, 0x00, "ISA RTC" },
    { 0x08, 0x03, 0x00, "Hot-Plug" },
    { 0x08, 0x80, 0x00, "Other" },

   /*keyboard*/
    { 0x09, 0x00, 0x00, "Keyboard" },
    { 0x09, 0x01, 0x00, "Pen" },
    { 0x09, 0x02, 0x00, "Mouse" },
    { 0x09, 0x03, 0x00, "Scanner" },
    { 0x09, 0x04, 0x00, "Game Port" },
    { 0x09, 0x80, 0x00, "Other" },

    { 0x0a, 0x00, 0x00, "Generic" },
    { 0x0a, 0x80, 0x00, "Other" },

    /*cpu type*/
    { 0x0b, 0x00, 0x00, "386" },
    { 0x0b, 0x01, 0x00, "486" },
    { 0x0b, 0x02, 0x00, "Pentium" },
    { 0x0b, 0x03, 0x00, "PentiumPro" },
    { 0x0b, 0x10, 0x00, "DEC Alpha" },
    { 0x0b, 0x20, 0x00, "PowerPC" },
    { 0x0b, 0x30, 0x00, "MIPS" },
    { 0x0b, 0x40, 0x00, "Coprocessor" },
    { 0x0b, 0x80, 0x00, "Other" },

    /*ieee 1394*/
    { 0x0c, 0x00, 0x00, "FireWire" },
    { 0x0c, 0x00, 0x10, "OHCI FireWire" },
    { 0x0c, 0x01, 0x00, "Access.bus" },
    { 0x0c, 0x02, 0x00, "SSA" },
    { 0x0c, 0x03, 0x00, "USB (UHCI)" },
    { 0x0c, 0x03, 0x10, "USB (OHCI)" },
    { 0x0c, 0x03, 0x80, "USB" },
    { 0x0c, 0x03, 0xFE, "USB Device" },
    { 0x0c, 0x04, 0x00, "Fiber" },
    { 0x0c, 0x05, 0x00, "SMBus Controller" },
    { 0x0c, 0x06, 0x00, "InfiniBand" },
    { 0x0c, 0x80, 0x00, "Other" },

    { 0x0d, 0x00, 0x00, "iRDA" },
    { 0x0d, 0x01, 0x00, "Consumer IR" },
    { 0x0d, 0x10, 0x00, "RF" },
    { 0x0d, 0x80, 0x00, "Other" },

    { 0x0e, 0x00, 0x00, "I2O" },
    { 0x0e, 0x80, 0x00, "Other" },

    { 0x0f, 0x01, 0x00, "TV" },
    { 0x0f, 0x02, 0x00, "Audio" },
    { 0x0f, 0x03, 0x00, "Voice" },
    { 0x0f, 0x04, 0x00, "Data" },
    { 0x0f, 0x80, 0x00, "Other" },

    /*cards*/
    { 0x10, 0x00, 0x00, "Network" },
    { 0x10, 0x10, 0x00, "Entertainment" },
    { 0x10, 0x80, 0x00, "Other" },

    { 0x11, 0x00, 0x00, "DPIO Modules" },
    { 0x11, 0x01, 0x00, "Performance Counters" },
    { 0x11, 0x10, 0x00, "Comm Sync, Time+Frequency Measurement" },
    { 0x11, 0x80, 0x00, "Other" },
};
 static int pci_class_nr = (sizeof(my_pci_classes)/sizeof(struct all_class));

 static int vendors = (sizeof(pci_vendors)/sizeof(struct pci_vendor));
 static int classes = (sizeof(pci_classes)/sizeof(struct pci_class));
 static int storagesubclasses = (sizeof(pci_storage_subclasses)/sizeof(struct pci_storage_subclass));
 static int networksubclasses = (sizeof(pci_network_subclasses)/sizeof(struct pci_network_subclass));
 static int displaysubclasses = (sizeof(pci_display_subclasses)/sizeof(struct pci_display_subclass));
 static int multimediasubclasses = (sizeof(pci_multimedia_subclasses)/sizeof(struct pci_multimedia_subclass));
 static int memorysubclasses = (sizeof(pci_memory_subclasses)/sizeof(struct pci_memory_subclass));
 static int bridgesubclasses = (sizeof(pci_bridge_subclasses)/sizeof(struct pci_bridge_subclass));
 static int serialsubclasses = (sizeof(pci_serial_subclasses)/sizeof(struct pci_serial_subclass));

extern struct pci_dev* sys_dev[32];
int pci_dev_nr=0;

const char *pci_show_class(unsigned b, unsigned sub, unsigned id)
{
	int i;
	for (i=0; i<pci_class_nr; i++)
	{
		if(my_pci_classes[i].base_class==b 
			&& my_pci_classes[i].sub_class==b 
			&&  my_pci_classes[i].interface_id == id)
			return my_pci_classes[i].dev_name;
	}
	return NULL;
}

char* pci_showvendor( unsigned short vendorid )  //显示厂商信息
{
    int index;
    
    for ( index = 0; index < vendors; index++ )
	   if (pci_vendors[ index ].id == vendorid )	{
	    return pci_vendors[ index ].name;
    }

	return "Unknow";
}

char* pci_showclass( unsigned short _class, unsigned short _sub )  //显示子类信息
{
    int i, j;  //用一个SWITCH代替它，这样更加结构化了

    for ( i = 0; i < classes; i++ )
    {
	if ( pci_classes[ i ].id == _class )
	{

	    if(_class== 0x01)  //存储类型
	    {
		    for ( j = 0; j < storagesubclasses; j++ )
			    if ( pci_storage_subclasses[ j ].id == _sub ){
			    return pci_storage_subclasses[ j ].name ;
			    break;
			    }

		    if ( j == storagesubclasses )
			printk( " %s", pci_classes[ i ].name );

		    break;
	    }

	    else if(_class== 0x02)  //网络类
	    {
		    for ( j = 0; j < networksubclasses; j++ )
			  if ( pci_network_subclasses[ j ].id == _sub ){
			    return pci_network_subclasses[ j ].name ;
			    break;
			  }

		    if ( j == networksubclasses )
			printk( " %s", pci_classes[ i ].name );

		    break;
	     }

	     else if(_class== 0x03)  //显示类
	     {
		    for ( j = 0; j < displaysubclasses; j++ )
			if ( pci_display_subclasses[ j ].id == _sub )
			{
			    return pci_display_subclasses[ j ].name ;
			    break;
			}

		    if ( j == displaysubclasses )
			printk( " %s", pci_classes[ i ].name );

		    break;
	      }

	      else if(_class ==0x04)  //多媒体子类
	      {
		    for ( j = 0; j < multimediasubclasses; j++ )
			if ( pci_multimedia_subclasses[ j ].id == _sub )
			{
			    return pci_multimedia_subclasses[ j ].name ;
			    break;
			}

		    if ( j == multimediasubclasses )
			printk( " %s", pci_classes[ i ].name );

		    break;
		}
		
		else if(_class==0x05)  //内存子类
		{
		    for ( j = 0; j < memorysubclasses; j++ )
			if (pci_memory_subclasses[ j ].id == _sub )
			{
			    return pci_memory_subclasses[ j ].name ;
			    break;
			}

		    if ( j == memorysubclasses )
			printk(" %s", pci_classes[ i ].name);

		    break;
		 }

		 else if(_class==0x06)  //PCI桥
		 {
		    for ( j = 0; j < bridgesubclasses; j++ )
			if ( pci_bridge_subclasses[ j ].id == _sub )
			{
			    return pci_bridge_subclasses[ j ].name ;
			    break;
			}

		    printk( " %s", pci_classes[ i ].name );
		    break;
		}
		
		else	if(_class==0x0c)  //串口子类
		{
		    for ( j = 0; j < serialsubclasses; j++ )
			if ( pci_serial_subclasses[ j ].id == _sub ){
			    return pci_serial_subclasses[ j ].name ;
			    break;
		    }

		    if ( j == serialsubclasses )
			printk( " %s", pci_classes[ i ].name );

		    break;
		}

		else
		   printk( " %s", pci_classes[ i ].name );
	        break;
	}

	if ( i == classes )
	    printk( " _class code %X", _class );

    }
	return "Unknow";
}
