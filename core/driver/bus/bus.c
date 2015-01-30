#include <jicama/system.h>
#include <jicama/fs.h>
#include <jicama/process.h>
#include <jicama/console.h>
#include <jicama/devices.h>
#include <string.h>
#include <assert.h>



#define NR_BUSMAN 32
static struct bus_man *bus_man[NR_BUSMAN];
int dev_cmp(char *devname, char *devcmp)
{
	return strcmp(devname, devcmp);
}

static int bus_get_empty(const char *devname)
{
	int i;
	int freeslot = -1;

	for (i=0; i<NR_BUSMAN; i++)
	{
		if(bus_man[i] == NULL){
			if(freeslot==-1)
				freeslot=i;
			continue;
		}
		if (dev_cmp(bus_man[i]->bus_name, devname)==0)
			return EEXIST;/*redo*/
		//if (bus_man[i]->bus_no== devno)
		//	return EEXIST;/*redo*/
	}

	return freeslot;
}

int	busman_register(const struct bus_man *bus)
{
	int	freeslot = bus_get_empty(bus->bus_name);

	if (freeslot<0)
	{
		return freeslot;
	}

	bus_man[freeslot] = bus;
	return 0;
}

struct bus_man *busman_get(const char *busname)
{
	int i;
	for (i=0;i< NR_BUSMAN; i++)
	{
		if(bus_man[i] == NULL)continue;
		trace("find %s bus\n", bus_man[i]->bus_name);
		if (dev_cmp(bus_man[i]->bus_name, busname) == 0)
		{
			return bus_man[i];
		}
	}
	return NULL;
}

int busman_init()
{
	int i;
	for (i=0;i< NR_BUSMAN; i++)
	{
		bus_man[i] = NULL;
	}
	return 0;
}

int	busman_unregister(const struct bus_man *bus)
{
	int i;

	if (!bus){
		return -1;
	}
	for (i=0;i< NR_BUSMAN; i++){
		if(bus_man[i] == bus)break;
	}
	bus_man[i] = NULL;
	return 0;
}


