
#ifndef _MONA_IDEDRVIER_
#define _MONA_IDEDRIVER_

#define IRQ_PRIMARY   14
#define IRQ_SECONDARY 15
enum
{
	PRIMARY   = 0,
	SECONDARY = 1,
	MASTER    = 0,
	SLAVE     = 1,
	DEVICE_UNKNOWN,
	DEVICE_NONE,
	DEVICE_ATA,
	DEVICE_ATAPI
};

enum
{
	SELECTION_ERROR = -1,
	DATA_READY_CHECK_ERROR=-2,
	BUSY_TIMEOUT_ERROR=-3,
	STATUS_ERROR=-4
};


    enum
    {
        ATA_DTR        = 0,
        ATA_ERR        = 1,
        ATA_FTR        = 1,
        ATA_SCR        = 2,
        ATA_IRR        = 2,
        ATA_SNR        = 3,
        ATA_CLR        = 4,
        ATA_BLR        = 4,
        ATA_CHR        = 5,
        ATA_BHR        = 5,
        ATA_DHR        = 6,
        ATA_STR        = 7,
        ATA_CMR        = 7,
        ATA_ASR        = 8,
        ATA_DCR        = 8,
        ATA_TIMEOUT    = 100000, // for Virtual PC changed from 100000 -> 10000
        BIT_BSY        = 0x80,
        BIT_DRDY       = 0x40,
        BIT_DRQ        = 8,
        BIT_ABRT       = 4,
        BIT_CHK        = 1,
        BIT_ERR        = 1,
        BIT_IO         = 2,
        BIT_CD         = 1,
        RETRY_MAX      = 2,
        DEV_HEAD_OBS   = 0xa0,
        LBA_FLG        = 0x40,
        REQUEST_CAPACITY_BUFFER_SIZE = 12,
        REQUEST_SENSE_BUFFER_SIZE = 18,

        ATAPI_SECTOR_SIZE = 2048,
        ATA_SECTOR_SIZE   = 512
    };



typedef struct IDEDevice
{
	int type;
	int typeDetail;
	int deviceNo;
	unsigned short sectorSize;
	char name[64];
}IDEDevice;

typedef struct IDEController
{
	//IDEController() : selectedDevice(NULL) {}
	unsigned char irq;
	int registers[10];
	IDEDevice devices[2];
	IDEDevice* selectedDevice;
 

}IDEController;

    typedef struct
    {
        unsigned char feature;
        unsigned char sectorCount;
        unsigned char sectorNumber;
        unsigned char cylinderLow;
        unsigned char cylinderHigh;
        unsigned char deviceNo;
        unsigned char command;
        bool drdyCheck;
    } ATACommand;

    typedef struct ATAPICommand
    {
        unsigned char feature;
        unsigned char deviceNo;
        unsigned char packet[12];
        unsigned short limit;
        void* buffer;
    }ATAPICommand;

//
// ATAPI commands
//

#define ATAPI_CMD_REQUESTSENSE  0x03
#define ATAPI_CMD_READCAPICITY  0x25
#define ATAPI_CMD_READ10        0x28

//
#define IOCTL_GETBLKSIZE        1
#define IOCTL_GETDEVSIZE        2
#define IOCTL_GETGEOMETRY       3
#define IOCTL_REVALIDATE        4

bool atapi_selectDevice(IDEController**,int controller, int deviceNo);
bool _atapi_selectDevice(IDEController* controller, int deviceNo);
unsigned char atapi_deviceValue(int deviceNo) ;
void atapi_initialize(IDEController* controller);
void atapi_setDeviceTypeSecond(IDEController* controller, int deviceNo);
unsigned char atapi_deviceValue(int deviceNo) ;
void atapi_protocolInterrupt(IDEController* controller);
bool atapi_commandRequestSense(IDEController* controller);
void atapi_setDeviceTypeFirst(IDEController* controller, int deviceNo);
bool atapi_commandRead10(IDEController* controller, unsigned long lba, void* buffer, int size);
int atapi_getLastError() ;

static inline unsigned long ntohl(unsigned long n)
{
  return ((n & 0xFF) << 24) | ((n & 0xFF00) << 8) | ((n & 0xFF0000) >> 8) | ((n & 0xFF000000) >> 24);
}
#endif
