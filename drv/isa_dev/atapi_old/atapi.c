/*!
    \org file  IDEDriver.cpp
    \new file atapi.c
    \brief IDEDriver

    Copyright (c) 2004 HigePon
    WITHOUT ANY WARRANTY

    \author  HigePon
	\modify by easion $Date 2006-4-2
    \version $Revision: 1.2 $
    \date   create:2004/11/14 update:$Date: 2005/07/17 13:27:24 $
*/
#include <drv/drv.h>
#include <stdarg.h>
#include <drv/fs.h>
#include <drv/ia.h>
#include <drv/unistd.h>

void current_sleep(int);

//#define _init_SLEEP(N) 
#define _SLEEP(N) mdelay((N)/3)

//有问题的代码
#define TASK_SLEEP(n) do{\
	thread_wait(current_thread(), (n));\
}\
while (0);

unsigned char inp8(unsigned long port) {

    unsigned char ret;
    asm volatile ("inb %%dx, %%al": "=a"(ret): "d"(port));
    return ret;
}

void outp8(unsigned long port, unsigned char value) {
   asm volatile ("outb %%al, %%dx": :"d" (port), "a" (value));
}

unsigned short inp16(unsigned long port) {

    unsigned short ret;
    asm volatile ("inw %%dx, %%ax": "=a"(ret): "d"(port));
    return ret;
}

void outp16(unsigned long port, unsigned short value) {
   asm volatile ("outw %%ax, %%dx": :"d" (port), "a" (value));
}

unsigned long inp32(unsigned long port) {

    unsigned long ret;
    asm volatile ("inl %%dx, %%eax": "=a"(ret): "d"(port));
    return ret;
}

void outp32(unsigned long port, unsigned long value) {
   asm volatile ("outl %%eax, %%dx": :"d" (port), "a" (value));
}



//	mdelay((n)*10);\
#define kprintf kprintf
#include "IDEDriver.h"
struct atapiinfo
{
	int _atapi_lastError;
	struct IDEController* whichController;
	volatile void* atapiBuffer;
	volatile int atapiReadDone;
	volatile unsigned long atapiTransferSize;
	volatile unsigned long atapiTotalReadSize;
	unsigned char requestSenseBuffer[REQUEST_SENSE_BUFFER_SIZE];
};

struct atapiinfo atapiinfo[2];
struct atapiinfo *current_atapiinfo = &atapiinfo[0];

//typedef enum{false=0, true=1}bool;


bool atapi_IDEDriver(IDEController *atapi_controllers[], int irq_primary, int irq_secondary)
{
   atapi_controllers[PRIMARY]->registers[ATA_DTR] = 0x1f0;
   atapi_controllers[PRIMARY]->registers[ATA_ERR] = 0x1f1;
   atapi_controllers[PRIMARY]->registers[ATA_SCR] = 0x1f2;
   atapi_controllers[PRIMARY]->registers[ATA_SNR] = 0x1f3;
   atapi_controllers[PRIMARY]->registers[ATA_CLR] = 0x1f4;
   atapi_controllers[PRIMARY]->registers[ATA_CHR] = 0x1f5;
   atapi_controllers[PRIMARY]->registers[ATA_DHR] = 0x1f6;
   atapi_controllers[PRIMARY]->registers[ATA_STR] = 0x1f7;
   atapi_controllers[PRIMARY]->registers[ATA_ASR] = 0x3f6;
   atapi_controllers[PRIMARY]->irq = irq_primary;

   atapi_controllers[SECONDARY]->registers[ATA_DTR] = 0x170;
   atapi_controllers[SECONDARY]->registers[ATA_ERR] = 0x171;
   atapi_controllers[SECONDARY]->registers[ATA_SCR] = 0x172;
   atapi_controllers[SECONDARY]->registers[ATA_SNR] = 0x173;
   atapi_controllers[SECONDARY]->registers[ATA_CLR] = 0x174;
   atapi_controllers[SECONDARY]->registers[ATA_CHR] = 0x175;
   atapi_controllers[SECONDARY]->registers[ATA_DHR] = 0x176;
   atapi_controllers[SECONDARY]->registers[ATA_STR] = 0x177;
   atapi_controllers[SECONDARY]->registers[ATA_ASR] = 0x376;
   atapi_controllers[SECONDARY]->irq = irq_secondary;

    /* initialize controllers */
    atapi_initialize(atapi_controllers[PRIMARY]);
    atapi_initialize(atapi_controllers[SECONDARY]);
   current_atapiinfo->whichController = NULL;
   current_atapiinfo->atapiBuffer     = NULL;
    current_atapiinfo->atapiReadDone   = true;
return true;
}


bool atapi_selectDevice(IDEController *controllers[], int controller, int deviceNo)
{
    if (controller != PRIMARY && controller != SECONDARY) 
	{
		printk("atapi_selectDevice controller error 1: %d\n",controller);
		return false;
	}
    if (deviceNo != MASTER && deviceNo != SLAVE) {
		printk("atapi_selectDevice deviceNo error 2: %d\n",deviceNo);
		return false;
	}

    IDEDevice* device = &controllers[controller]->devices[deviceNo];

    if (device->type == DEVICE_NONE || device->type == DEVICE_UNKNOWN){
		printk("atapi_selectDevice type error 3: %d\n",device->type);
		return false;
	}

    controllers[controller]->selectedDevice = device;
    current_atapiinfo->whichController = controllers[controller];
    return true;
}

bool atapi_findDevice(IDEController *controllers[], int type, int detail,
	int* controller, int* deviceNo,IDEController **ret)
{
	int i,j;
    for ( i = 0; i < 2; i++)
    {
        for ( j = 0; j < 2; j++)
        {
            IDEDevice* device = &controllers[i]->devices[j];

            if (type != device->type || detail != device->typeDetail) continue;

            *controller = i;
            *deviceNo = j;
			*ret = controllers[i];
            return true;
        }
    }

    return false;
}
//#define DEBUG_READ_TRACE
  
int atapi_read(IDEController *controllers, unsigned long lba, void* buffer, int size)
{
	int i,j;
 #ifdef DEBUG_READ_TRACE
    void* buffer2 = buffer;
    kprintf("read lba=%d size=%d start\n", lba, size);
#endif


    if (current_atapiinfo->whichController == NULL){
	//kprintf("atapi_read failed: whichController == NULL\n");
	return -1;
	}


    if (current_atapiinfo->whichController->selectedDevice->type == DEVICE_ATAPI)
    {
        int count = (size + 0xf800 - 1) / 0xf800;

        for ( i = 0; i < count; i++)
        {
            int readSize;
            bool readResult;
            if (i == count - 1)
            {
                readSize = size - 0xf800 * i;
            }
            else
            {
                readSize = 0xf800;
            }

            for ( j = 0; j < 20; j++)
            {
                readResult = atapi_commandRead10( current_atapiinfo->whichController,
					lba + 0xf800 * i / 2048, buffer, readSize);
                if (readResult) break;
            }

            buffer = (void*)((unsigned char*)buffer + readSize);

            if (!readResult)
            {
                return atapi_getLastError();
            }
        }

		//kprintf("read good\n");
        return 0;
    }
    else if (current_atapiinfo->whichController->selectedDevice->type == DEVICE_ATA)
    {
	kprintf("atapi_read failed: selectedDevice->type == DEVICE_ATA\n");

        return 4;
    }
    else
    {
	kprintf("atapi_read failed: unknow\n");
        return 5;
    }
}


int atapi_getLastError() 
{
    return current_atapiinfo->_atapi_lastError;
}

void atapi_getLastErrorDetail(IDEController *controllers, unsigned char* buffer)
{
    atapi_commandRequestSense(current_atapiinfo->whichController);
    memcpy(buffer, current_atapiinfo->requestSenseBuffer, REQUEST_SENSE_BUFFER_SIZE);
}

void atapi_outp8(IDEController* controller, int reg, unsigned char value)
{
    outp8(controller->registers[reg], value);
}

unsigned char atapi_inp8(IDEController* controller, int reg)
{
    return inp8(controller->registers[reg]);
}


unsigned short _atapi_inp16(IDEController* controller, int reg)
{
    return inp16(controller->registers[reg]);
}

void _atapi_outp16(IDEController* controller, int reg, unsigned short value)
{
    outp16(controller->registers[reg], value);
}

void atapi_outp16(IDEController* controller, unsigned short* data, int length)
{
	int i;
    for( i=0; i < length; i++)
    {
        _atapi_outp16(controller, ATA_DTR, *data);
        data++;
    }
}

void atapi_inp16(IDEController* controller, unsigned short* data, int size)
{
	int i;

    int length = size / 2;

    if (data == NULL)
    {
        for ( i = 0; i < length; i++)
        {
            _atapi_inp16(controller, ATA_DTR);
        }
    }
    else
    {
        for ( i = 0; i < length; i++)
        {
            *data = _atapi_inp16(controller, ATA_DTR);
            data++;
        }
    }
}

/*----------------------------------------------------------------------
    IDEDRIVER : flag utilities
----------------------------------------------------------------------*/
bool atapi_waitBusyAndDataRequestBothClear(IDEController* controller)
{
    unsigned long i;
    for (i = 0; i < ATA_TIMEOUT; i++)
    {
        unsigned char status = atapi_inp8(controller, ATA_ASR);
        if ((status & BIT_BSY) == 0 && (status & BIT_DRQ) == 0) break;
    }
    return (i != ATA_TIMEOUT);
}

bool atapi_waitBusyClear(IDEController* controller)
{
    unsigned long i;
    for (i = 0; i < ATA_TIMEOUT; i++)
    {
        unsigned char status = atapi_inp8(controller, ATA_ASR);
        if ((status & BIT_BSY) == 0) break;
    }
    return (i != ATA_TIMEOUT);
}

bool atapi_waitDrdySet(IDEController* controller)
{
    unsigned long i;

    for (i = 0; i < ATA_TIMEOUT; i++)
    {
        unsigned char status = atapi_inp8(controller, ATA_ASR);
        if (status & BIT_DRDY) break;
    }

    return (i != ATA_TIMEOUT);
}

/*----------------------------------------------------------------------
    IDEDRIVER : protocol
----------------------------------------------------------------------*/
bool atapi_protocolPacket(IDEController* controller, ATAPICommand* command)
{
    atapi_outp8(controller, ATA_DCR, 0x8);  /* use interrupt */

    current_atapiinfo->atapiBuffer        = command->buffer;
    current_atapiinfo->atapiReadDone      = false;
    current_atapiinfo->atapiTotalReadSize = command->limit;

#if 1
    if (!_atapi_selectDevice(controller, command->deviceNo))
    {
        current_atapiinfo->_atapi_lastError = SELECTION_ERROR;
        return false;
    }
#endif
    /* packet command */
    atapi_outp8(controller, ATA_FTR, command->feature);
    atapi_outp8(controller, ATA_SCR, 0);
    atapi_outp8(controller, ATA_BLR, (unsigned char)(command->limit & 0xff));
    atapi_outp8(controller, ATA_BHR, (unsigned char)(command->limit >> 8));
    atapi_outp8(controller, ATA_CMR, 0xa0);
    //_SLEEP(1);

    unsigned long i;
    for (i = 0; i < ATA_TIMEOUT; i++)
    {
        unsigned char status = atapi_inp8(controller, ATA_ASR);

        if ((status & BIT_BSY) != 0) continue;
        if ((status & BIT_CHK) != 0)
        {
            current_atapiinfo->atapiBuffer = NULL;
            atapi_inp8(controller, ATA_ERR); /* must? */
            current_atapiinfo->_atapi_lastError = STATUS_ERROR;
            return false;
        }

        unsigned char reason = atapi_inp8(controller, ATA_IRR);
        if (((status & BIT_DRQ) != 0) && ((reason & BIT_IO) == 0) && ((reason & BIT_CD) != 0)) break;
    }

    if (i == ATA_TIMEOUT)
    {
        current_atapiinfo->atapiBuffer = NULL;
        current_atapiinfo->_atapi_lastError = BUSY_TIMEOUT_ERROR;
        return false;
    }

    atapi_outp16(controller, (unsigned short*)command->packet, 6);
    for (i = 0; i < ATA_TIMEOUT; i++)
    {
        atapi_protocolInterrupt(controller);

        unsigned char status = atapi_inp8(controller, ATA_ASR);

        if ((status & BIT_BSY) != 0) continue;

        if ((status & BIT_CHK) != 0)
        {
            current_atapiinfo->atapiBuffer = NULL;
            current_atapiinfo->_atapi_lastError = STATUS_ERROR;
            return false;
        }
        if (current_atapiinfo->atapiReadDone) break;
    }

    atapi_inp8(controller, ATA_STR);

    if (i == ATA_TIMEOUT)
    {
        current_atapiinfo->atapiBuffer = NULL;
        current_atapiinfo->_atapi_lastError = BUSY_TIMEOUT_ERROR;
        return false;
    }

    return true;
}

bool atapi_protocolAtaNoneData(IDEController* controller, ATACommand* command)
{
	IDEController* controllers=controller;
    /* select device */
    if (!_atapi_selectDevice(controller, command->deviceNo))
    {
        current_atapiinfo->_atapi_lastError = SELECTION_ERROR;
        return false;
    }

    atapi_outp8(controllers, ATA_DCR, 0x2); /* no interrupt */
    atapi_outp8(controllers, ATA_FTR, command->feature);
    atapi_outp8(controllers, ATA_SCR, command->sectorCount);
    atapi_outp8(controllers, ATA_SNR, command->sectorNumber);
    atapi_outp8(controllers, ATA_CLR, command->cylinderLow);
    atapi_outp8(controllers, ATA_CHR, command->cylinderHigh);

    /* data ready check */
    if (!atapi_waitDrdySet(controller))
    {
       current_atapiinfo->_atapi_lastError = DATA_READY_CHECK_ERROR;
        return false;
    }

    atapi_outp8(controllers, ATA_CMR, command->command);
    //_SLEEP(1);

    /* wait busy clear */
    if (!atapi_waitBusyClear(controller))
    {
        current_atapiinfo->_atapi_lastError = BUSY_TIMEOUT_ERROR;
        return false;
    }

    atapi_inp8(controllers, ATA_ASR); /* read once */

    /* check error */
    unsigned char status = atapi_inp8(controllers, ATA_STR);
    if (status & BIT_ERR)
    {
        atapi_inp8(controllers, ATA_ERR); /* must read ? */
        current_atapiinfo->_atapi_lastError = STATUS_ERROR;
        return false;
    }

    return true;
}

bool atapi_protocolPioDataIn(IDEController* controller, ATACommand* command, unsigned short count, void* buf)
{
	int i;
    unsigned short* p = (unsigned short*)buf;

    if (!_atapi_selectDevice(controller, command->deviceNo))
    {
        current_atapiinfo->_atapi_lastError = SELECTION_ERROR;
        return false;
    }

    atapi_outp8(controller, ATA_DCR, 0x02);                  /* not use interrupt */
    atapi_outp8(controller, ATA_FTR, command->feature);      /* feature           */
    atapi_outp8(controller, ATA_SCR, command->sectorCount);  /* sector count      */
    atapi_outp8(controller, ATA_SNR, command->sectorNumber); /* sector number     */
    atapi_outp8(controller, ATA_CLR, command->cylinderLow);  /* cylinderLow       */
    atapi_outp8(controller, ATA_CHR, command->cylinderHigh); /* cylinderHigh      */

    /* drdy check */
    if (command->drdyCheck && !atapi_waitDrdySet(controller))
    {
        current_atapiinfo->_atapi_lastError = DATA_READY_CHECK_ERROR;
        return false;
    }

    atapi_outp8(controller, ATA_CMR, command->command);
    

    /* read atlternate status once */
    atapi_inp8(controller, ATA_ASR);

    /* read */
    for ( i = 0; i < count; i++, p+=256)
    {
        if (!atapi_waitBusyClear(controller))
        {
            current_atapiinfo->_atapi_lastError = BUSY_TIMEOUT_ERROR;
            return false;
        }

        unsigned char status = atapi_inp8(controller, ATA_STR);

        /* command error */
        if ((status & BIT_ERR) != 0)
        {
            current_atapiinfo->_atapi_lastError = STATUS_ERROR;
            return false;
        }

        /* data not ready */
        if ((status & BIT_DRQ) == 0)
        {
            current_atapiinfo->_atapi_lastError = DATA_READY_CHECK_ERROR;
            return false;
        }

        /* data read */
        atapi_inp16(controller, p, 512);
    }

    atapi_inp8(controller, ATA_ASR);
    unsigned char status = atapi_inp8(controller, ATA_STR);

    if (status & BIT_ERR)
    {
        atapi_inp8(controller, ATA_ERR); /* must ? */
        current_atapiinfo->_atapi_lastError = STATUS_ERROR;
        return false;
    }
    return true;
}

void atapi_protocolInterrupt(IDEController* controller)
{
    for (;;)
    {
       /* if (!MONAPI_WAIT_INTERRUPT(1000, whichController->irq))
        {
            // time out ! 
            return;
        }*/

        unsigned char status = atapi_inp8(current_atapiinfo->whichController, ATA_STR);
        unsigned char reason = atapi_inp8(current_atapiinfo->whichController, ATA_IRR);


        /* read */
        if (((reason & BIT_IO) != 0) && ((reason & BIT_CD) == 0) && ((status & BIT_DRQ) != 0))
        {
            unsigned short transferSize = (atapi_inp8(current_atapiinfo->whichController, ATA_BHR) << 8) | atapi_inp8(current_atapiinfo->whichController, ATA_BLR);
            current_atapiinfo->atapiTransferSize += transferSize;

            if (current_atapiinfo->atapiTransferSize > current_atapiinfo->atapiTotalReadSize)
            {
                atapi_inp16(current_atapiinfo->whichController, NULL, transferSize);
            }
            else
            {
                atapi_inp16(current_atapiinfo->whichController, (unsigned short*)current_atapiinfo->atapiBuffer, transferSize);
                current_atapiinfo->atapiBuffer = (void*)((unsigned char*)current_atapiinfo->atapiBuffer + transferSize);
            }
        }

        /* read / write done */
        if (((reason & BIT_IO)!=0) && ((reason & BIT_CD) != 0) && ((status & BIT_DRQ) == 0))
        {
            current_atapiinfo->atapiReadDone = true;
            return;
        }
    }

}

/*----------------------------------------------------------------------
    IDEDRIVER : execute command using protocol function
----------------------------------------------------------------------*/
bool atapi_commandIdleImmediate(IDEController* controller, int deviceNo)
{
    ATACommand command;
    memset(&command, 0, sizeof(command));

    command.deviceNo  = deviceNo;
    command.command   = controller->selectedDevice->type == DEVICE_ATA ? 0xe3 : 0xe1;
    command.drdyCheck = true;

    return atapi_protocolAtaNoneData(controller, &command);
}

bool atapi_commandRequestSense(IDEController* controller)
{
    ATAPICommand command;
    memset(&command, 0, sizeof(command));

    command.feature   = 0;
    command.deviceNo  = controller->selectedDevice->deviceNo;
    command.packet[0] = 0x03;
    command.packet[4] = REQUEST_SENSE_BUFFER_SIZE;
    command.limit     = REQUEST_SENSE_BUFFER_SIZE;
    command.buffer    = current_atapiinfo->requestSenseBuffer;
    current_atapiinfo->atapiTransferSize = 0;

    memset(current_atapiinfo->requestSenseBuffer, 0, REQUEST_SENSE_BUFFER_SIZE);
    return atapi_protocolPacket(controller, &command);
}

int atapi_ReadCapacity(IDEController* controller)
{
	unsigned long blks,blksize;
	unsigned long rc;

    ATAPICommand command;
    memset(&command, 0, sizeof(command));
  
    command.feature   = 0;
    command.deviceNo  = controller->selectedDevice->deviceNo;
    command.packet[0] = ATAPI_CMD_READCAPICITY;

	command.limit     = REQUEST_CAPACITY_BUFFER_SIZE;
	// command.buffer    = buf;
	command.buffer    = current_atapiinfo->requestSenseBuffer;
	current_atapiinfo->atapiTransferSize = 0;
	
    memset(current_atapiinfo->requestSenseBuffer, 0, REQUEST_SENSE_BUFFER_SIZE);

    rc= atapi_protocolPacket(controller, &command); 

	blks = ntohl(current_atapiinfo->requestSenseBuffer[0]);
	blksize = ntohl(current_atapiinfo->requestSenseBuffer[1]);
	if (blksize != 2048)
		  kprintf("%s: unexpected block size (%d)\n", "atapi_ReadCapacity", blksize);
  return blks;
}

bool atapi_commandRead10(IDEController* controller, unsigned long lba, void* buffer, int size)
{
    ATAPICommand command;
    memset(&command, 0, sizeof(command));

    /* sector count */
    int count = (size + ATAPI_SECTOR_SIZE - 1) / ATAPI_SECTOR_SIZE;

    command.feature   = 0;
    command.deviceNo  = controller->selectedDevice->deviceNo;
    command.packet[0] = 0x28;
    command.packet[2] = (lba >>  24) & 0xff;
    command.packet[3] = (lba >>  16) & 0xff;
    command.packet[4] = (lba >>   8) & 0xff;
    command.packet[5] = (lba       ) & 0xff;
    command.packet[7] = (count >> 8) & 0xff;
    command.packet[8] = (count     ) & 0xff;
    command.limit     = ATAPI_SECTOR_SIZE * count;
    command.buffer    = buffer;
    current_atapiinfo->atapiTransferSize = 0;

    return atapi_protocolPacket(controller, &command);
}

bool atapi_commandIdentify(IDEController* controller, int deviceNo, unsigned short* buffer)
{
	int i;
	unsigned short* p;
	bool commandOK;


    ATACommand command;
    memset(&command, 0, sizeof(command));

    IDEDevice* device = &controller->devices[deviceNo];

    command.deviceNo = deviceNo;
    if (device->type == DEVICE_ATA)
    {
        command.drdyCheck = true;
        command.command   = 0xec;
    }
    else
    {
        command.drdyCheck = false;
        command.command   = 0xa1;
    }

    commandOK = atapi_protocolPioDataIn(controller, &command, 1, buffer);

    if (!commandOK) return false;

     p = buffer;
    for( i = 0; i < 256; i++)
    {
        unsigned short value = *p;
        *p = ((value >> 8) & 0xff) | ((value << 8) & 0xff00);
        p++;
    }

    return true;
}

/*----------------------------------------------------------------------
    IDEDRIVER :initialize functions
----------------------------------------------------------------------*/
void atapi_initialize(IDEController* controller)
{
    /* software reset */
    atapi_outp8(controller, ATA_DCR, 0x06);
    TASK_SLEEP(15);

    /* no interrupt */
    atapi_outp8(controller, ATA_DCR, 0x02);
    TASK_SLEEP(15);

    atapi_setDeviceTypeFirst(controller, MASTER);
    atapi_setDeviceTypeSecond(controller, MASTER);

    atapi_setDeviceTypeFirst(controller, SLAVE);
   atapi_setDeviceTypeSecond(controller, SLAVE);
}
#ifndef dpp

/*
    call only after software reset
*/
void atapi_setDeviceTypeFirst(IDEController* controller, int deviceNo)
{
    unsigned long l;
    unsigned char c;
    unsigned char c1 = 0xff; /* unknown signature */
    unsigned char c2 = 0xff; /* unknown signature */
   bool timeout;

    IDEDevice* device = &controller->devices[deviceNo];
    device->deviceNo = deviceNo;

    for (l = 0; l < RETRY_MAX; l++)
    {
        /* select device */
        atapi_outp8(controller, ATA_DHR, atapi_deviceValue(deviceNo));
//        sleep(10);

        c = atapi_inp8(controller, ATA_STR);
        if (c == 0xff) break;

        timeout = !atapi_waitBusyClear(controller);
        if (timeout) break;

        /* bad device */
        unsigned char error = atapi_inp8(controller, ATA_ERR);
        if (deviceNo == MASTER && (error & 0x7f) != 1)
        {
            device->type = DEVICE_UNKNOWN;
            return;
        }
        else if (deviceNo == SLAVE && error != 1)
        {
            device->type = DEVICE_UNKNOWN;
            return;
        }

        c = atapi_inp8(controller, ATA_DHR);
        if ((c & (deviceNo << 4)) == (deviceNo << 4))
        {
            c1 = atapi_inp8(controller, ATA_CLR);
            c2 = atapi_inp8(controller, ATA_CHR);
            break;
        }
    }

    switch(c1 | (c2 << 8))
    {
    case 0xEB14:
        device->type = DEVICE_ATAPI;
        break;
    case 0:
        device->type = DEVICE_ATA;
        break;
    default:
        device->type = DEVICE_NONE;
        break;
    }
}

void atapi_setDeviceTypeSecond(IDEController* controller, int deviceNo)
{
    unsigned char l;
    unsigned short buffer[256]; /* identify buffer */
    IDEDevice* device = &(controller->devices[deviceNo]);

    if (!atapi_waitBusyClear(controller))
    {
        device->type = DEVICE_NONE;
        return;
    }


    for (l = 0; l < RETRY_MAX; l++)
    {
		bool secondResult ;
		 int secondError ;
        bool firstResult = atapi_commandIdentify(controller, deviceNo, buffer);
        int firstError   = atapi_getLastError();

        TASK_SLEEP(15);
        secondResult = atapi_commandIdentify(controller, deviceNo, buffer);
        secondError   = atapi_getLastError();
        if (firstResult && secondResult)
        {
            break;
        }
        else if (!firstResult && !secondResult)
        {
            if (firstError != secondError) continue;
            if (firstError == SELECTION_ERROR || firstError == BUSY_TIMEOUT_ERROR || firstError == DATA_READY_CHECK_ERROR)
            {
                device->type = DEVICE_NONE;
                break;
            }
        }
    }

    if (l == RETRY_MAX)
    {
        device->type = DEVICE_UNKNOWN;
    }

    /* information */
    switch(device->type)
    {
    case DEVICE_ATA:
		{
		printf("DEVICE_ATA found\n");
        strncpy(device->name ,(const char*)((unsigned char*)buffer + 54), 40);
        device->typeDetail = -1;
        device->sectorSize = ATA_SECTOR_SIZE;
		}
        break;

    case DEVICE_ATAPI:
        strncpy(device->name ,(const char*)((unsigned char*)buffer + 54), 40);
        device->typeDetail = buffer[0] & 0x1f;
        device->sectorSize = ATAPI_SECTOR_SIZE;
        break;

    case DEVICE_NONE:
        //device->name       = "none";
		strncpy(device->name ,"none", 40);
        device->typeDetail = -1;
        break;

    case DEVICE_UNKNOWN:

        //device->name       = "unknown";
		strncpy(device->name ,"unknown", 40);
        device->typeDetail = -1;
        break;
    }

	//kprintf("CDROM TYPE %s\n", device->name);
}

bool _atapi_selectDevice(IDEController* controller, int deviceNo)
{
    if (current_atapiinfo->whichController != NULL)
    {
        IDEDevice* device = current_atapiinfo->whichController->selectedDevice;
        if (current_atapiinfo->whichController == controller
			&& device->deviceNo == deviceNo)
        {
			unsigned char value = atapi_deviceValue( deviceNo);
			//printf("atapi_waitBusyAndDataRequestBothClear %d \n",
			//	controller->registers[ATA_DHR]);
            atapi_outp8(controller, ATA_DHR, value);
           //_SLEEP(1);
            return true;
        }
    }


    if (!atapi_waitBusyAndDataRequestBothClear(controller))
		return false;

    /* select device */
    atapi_outp8(controller, ATA_DHR, atapi_deviceValue(deviceNo));
    

    if (!atapi_waitBusyAndDataRequestBothClear(controller)) 
		return false;

    current_atapiinfo->whichController = controller;
    current_atapiinfo->whichController->selectedDevice = &controller->devices[deviceNo];
    current_atapiinfo->whichController->selectedDevice->deviceNo = deviceNo;

    return true;
}

unsigned char atapi_deviceValue(int deviceNo) 
{
    return (unsigned char)(DEV_HEAD_OBS | (deviceNo << 4));
}
#endif

int atapi_ioctl(int dev, int cmd, void *args)
{
  int rc;

  switch (cmd)
  {
    case IOCTL_GETDEVSIZE:
           rc = atapi_ReadCapacity(current_atapiinfo->whichController );
      if (rc < 0) return rc;

    case IOCTL_GETBLKSIZE:
      return 2048;

    case IOCTL_REVALIDATE:
      rc = atapi_commandRequestSense(current_atapiinfo->whichController );
      if (rc < 0) return rc;

      rc = atapi_ReadCapacity(current_atapiinfo->whichController );
      if (rc < 0) return rc;
      
      return 0;
  }

  return -1;
}


