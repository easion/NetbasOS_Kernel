

#ifndef pci_H
#define pci_H

#ifdef __cplusplus
extern "C" {
#endif


/* Ports for access to PCI config space */
#define PCI_CONFIG_ADDR			0xcf8
#define PCI_CONFIG_DATA			0xcfc

/* PCI config space register offsets */
#define PCI_CONFIG_VENDOR		0
#define PCI_CONFIG_CMD_STAT		1
#define PCI_CONFIG_CLASS_REV		2
#define PCI_CONFIG_HDR_TYPE		3
#define PCI_CONFIG_BASE_ADDR_0		4
#define PCI_CONFIG_BASE_ADDR_1		5
#define PCI_CONFIG_BASE_ADDR_2		6
#define PCI_CONFIG_BASE_ADDR_3		7
#define PCI_CONFIG_BASE_ADDR_4		8
#define PCI_CONFIG_BASE_ADDR_5		9
#define PCI_CONFIG_CIS			10
#define PCI_CONFIG_SUBSYSTEM		11
#define PCI_CONFIG_ROM			12
#define PCI_CONFIG_CAPABILITIES		13
#define PCI_CONFIG_INTR			15

typedef struct pci_dev
{
	u8_t flags;
	//! By writing to this field the system controls the device.
	u16_t command;
	//! This field gives the status of the device with the
	//! meaning of the bits of this field set by the standard.
	u16_t status;
	//! Base addresses (for both I/O and memory-based devices).
	//! Every PCI device has up to 6 base addresses (6 for
	//! normal devices, 2 for PCI to PCI bridges and only 1 for
	//! cardbuses).
	u32_t base[6];
	//! Size of the I/O space. For memory-based devices it is the
	//! size of the memory-mapped buffer; for I/O based devices it
	//! is the maximum offset of the ports used.
	u32_t size[6];
	//! Type of the I/O operation (memory based or I/O based).
	u8_t type[6];
	//! The ROM base address.
	u32_t rom_base;
	//! The ROM memory space.
	u32_t rom_size;

	//! Power management state (from D0 to D3).
	u8_t current_state;

	unsigned bus, dev_fn;
	unsigned char hdr_type;
	int func;
    unsigned short vendorid;
    unsigned short deviceid;
    //unsigned long iobase;
    int irq;

	unsigned long base_class,	sub_class, interface;
	unsigned char revision_id;
} pci_state_t;


//! This is a normal PCI device.
#define PCI_HEADER_TYPE_NORMAL		0
//! This is a bridge PCI device.
#define PCI_HEADER_TYPE_BRIDGE		1
//! This is a card-bus PCI device.
#define PCI_HEADER_TYPE_CARDBUS		2

int (*pci_read_config_byte)(pci_state_t *state,
		unsigned reg, unsigned char *value);
int (*pci_read_config_word)(pci_state_t *state,
		unsigned reg, unsigned short *value);
int (*pci_read_config_dword)(pci_state_t *state,
		unsigned reg, unsigned long *value);

int (*pci_write_config_byte)(pci_state_t *state,
		unsigned reg, unsigned char value);
int (*pci_write_config_word)(pci_state_t *state,
		unsigned reg, unsigned short value);
int (*pci_write_config_dword)(pci_state_t *state,
		unsigned reg, unsigned long value);

unsigned long pci_read_ulong( int bus, int device, int fn, unsigned long regnum );
struct pci_dev* pci_lookup(unsigned int start, unsigned short vendorid, unsigned short deviceid );

struct pci_dev *pci_dev_lookup(unsigned long basec, unsigned long subc);
void pci_dump(struct pci_dev *cfg);

/* PCI vendor IDs */
#define PCI_VENDOR_COMPAQ		0x0e11
#define PCI_VENDOR_NCR			0x1000
#define PCI_VENDOR_ATI			0x1002
#define PCI_VENDOR_VLSI			0x1004
#define PCI_VENDOR_TSENG		0x100c
#define PCI_VENDOR_WEITEK		0x100e
#define PCI_VENDOR_DEC			0x1011
#define PCI_VENDOR_CIRRUS		0x1013
#define PCI_VENDOR_IBM			0x1014
#define PCI_VENDOR_AMD			0x1022
#define PCI_VENDOR_TRIDENT		0x1023
#define PCI_VENDOR_MATROX		0x102b
#define PCI_VENDOR_NEC			0x1033
#define PCI_VENDOR_HP			0x103c
#define PCI_VENDOR_BUSLOGIC		0x104b
#define PCI_VENDOR_TI			0x104c
#define PCI_VENDOR_MOTOROLA		0x1057
#define PCI_VENDOR_NUMBER9		0x105d
#define PCI_VENDOR_APPLE		0x106b
#define PCI_VENDOR_CYRIX		0x1078
#define PCI_VENDOR_SUN			0x108e
#define PCI_VENDOR_3COM			0x10b7
#define PCI_VENDOR_ACER			0x10b9
#define PCI_VENDOR_MITSUBISHI		0x10ba
#define PCI_VENDOR_NVIDIA		0x10de
#define PCI_VENDOR_FORE			0x1127
#define PCI_VENDOR_PHILLIPS		0x1131
#define PCI_VENDOR_RENDITION		0x1163
#define PCI_VENDOR_TOSHIBA		0x1179
#define PCI_VENDOR_ENSONIQ		0x1274
#define PCI_VENDOR_ROCKWELL		0x127a
#define PCI_VENDOR_NETGEAR		0x1385
#define PCI_VENDOR_VMWARE		0x15ad
#define PCI_VENDOR_S3			0x5333
#define PCI_VENDOR_INTEL		0x8086
#define PCI_VENDOR_ADAPTEC		0x9004
#define PCI_VENDOR_ADAPTEC2		0x9005
#define PCI_VENDOR_VIA			0x1106
#define PCI_VENDOR_SIS			0x1039
#define PCI_VENDOR_02MICRO		0x1217
#define PCI_VENDOR_REALTEK		0x10ec
#define PCI_VENDOR_UMC			0x1060
#define PCI_VENDOR_BROOKTREE		0x109e
#define PCI_VENDOR_3DFX			0x121a
#define PCI_VENDOR_FIBERLINE		0x1282
#define PCI_VENDOR_CHIPSANDTECHNOLOGIE	0x102c
// delievierd in %%al
#define PCI_BIOS_PRESENT 	0x01
#define FIND_PCI_DEVICE		0x02
#define FIND_PCI_CLASS_CODE	0x03
#define GENERATE_SPECIAL_CYCLE	0x06
#define READ_CONFIG_BYTE	0x08
#define READ_CONFIG_WORD	0x09
#define READ_CONFIG_DWORD	0x0A
#define WRITE_CONFIG_BYTE	0x0B
#define WRITE_CONFIG_WORD	0x0C
#define WRITE_CONFIG_DWORD	0x0D

//delieverd in %%ah
#define PCI_FUNCTION_ID		0xB1
#define SUCCESFULL		0x00
#define FUNC_NOT_SUPPORTED	0x81
#define BAD_VENDOR_ID		0x83
#define DEVICE_NOT_FOUND	0x86
#define BAD_REGISTER_NUMBER	0x87

void pci_read_irq(pci_state_t *cfg);
int pci_find_capability(pci_state_t *cfg, int cap);
int pci_enable_device(pci_state_t *cfg);
bool pci_find_cfg(pci_state_t *cfg, bool enable);
int pci_set_power_state(pci_state_t *cfg, int state);
void pci_set_master(pci_state_t *cfg);
bool pci_probe(int bus, int dev, int func, pci_state_t *cfg);
void pci_read_bases(pci_state_t *cfg, int tot_bases, int rom);


typedef struct
{
	int (*get_pci_info) ( pci_state_t* psInfo, int nIndex );

	int (*read_pci_config8)( pci_state_t *state,	unsigned reg, unsigned char *value );
	int (*read_pci_config16)( pci_state_t *state,	unsigned reg, unsigned short *value );
	int (*read_pci_config32)( pci_state_t *state,	unsigned reg, unsigned long *value );
	int (*write_pci_config8)( pci_state_t *state,	unsigned reg, unsigned char value );
	int (*write_pci_config16)( pci_state_t *state,	unsigned reg, unsigned short value );
	int (*write_pci_config32)( pci_state_t *state,	unsigned reg, unsigned long value );

	void (* pci_read_irq) (pci_state_t *cfg);
	int (* pci_find_capability) (pci_state_t *cfg, int cap);
	int (* pci_enable_device) (pci_state_t *cfg);
	bool (* pci_find_cfg) (pci_state_t *cfg, bool enable);
	int (* pci_set_power_state) (pci_state_t *cfg, int state);
	void (* pci_set_master) (pci_state_t *cfg);
	bool (* pci_probe) (int bus, int dev, int func, pci_state_t *cfg);
	void (* pci_read_bases) (pci_state_t *cfg, int tot_bases, int rom);

	void (*pci_dump)(struct pci_dev *cfg);
	struct  pci_dev *(*pci_dev_lookup)(unsigned long basec, unsigned long subc);
	struct  pci_dev* (*pci_lookup)(unsigned int start, unsigned short vendorid, unsigned short deviceid );
	void * (*pci_root_dev)();

} pci_pos_t;

typedef struct pci_softc
{
	pci_pos_t* bus;
	pci_state_t *dev;
}pci_softc_t;



#endif


