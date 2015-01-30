/* 
**  Jicama OS  
* * Copyright (C) 2003  DengPingPing     
**  All rights reserved.   
 */

  #ifndef DEVICES_H
  #define DEVICES_H
#include <jicama/iomux.h>
#include <sys/queue.h>

  #define NULL_DEVICE 0
  #define BLK_DEVICE 1
  #define CHAR_DEVICE 2
  #define OTHER_DEVICE 3

#define NR_BLK_REQUEST	32
#define KERNEL_VERSION(X,Y) ((X)<<8 | (Y))
#define CURRENT_KERNEL_VERSION KERNEL_VERSION(0,18)


struct bus_man
{
	int bus_no;
	int bus_name;
	int bus_count;
	void(* bus_get_hooks)( int ver );
	time_t bus_time;
};

int	busman_register(const struct bus_man *bus);
struct bus_man *busman_get(const char *busname);
int	busman_unregister(const struct bus_man *bus);

#undef  FD_SETSIZE
#define FD_SETSIZE    256
#define FD_SET(n, p)  (((fd_set*)p)->fd_bits[(n)/8] |=  (1 << ((n) & 7)))
#define FD_CLR(n, p)  (((fd_set*)p)->fd_bits[(n)/8] &= ~(1 << ((n) & 7)))
#define FD_ISSET(n,p) (((fd_set*)p)->fd_bits[(n)/8] &   (1 << ((n) & 7)))
#define FD_ZERO(p)    memset((void*)(p),0,sizeof(*(p)))

typedef struct fd_set {
	  unsigned char fd_bits [(FD_SETSIZE+7)/8];
} fd_set;



typedef struct dev_prvi
{
  struct ioobject iob;
	int fd;
	dev_t devno;
	void *data;
	struct dev_prvi *next;
	short type;
	short subtype;
	long params[6];
}dev_prvi_t;

typedef struct device_struct
{
	char *d_name;
	char *d_author;
	char *d_version;
	dev_t d_index;
	long type;
	unsigned short d_kver; /*kernel version */
	unsigned short access;
	//struct device_struct* next; /*  */
	//void* d_data; /*instance data */
	TAILQ_ENTRY(device_struct) next_dev;
	void *netif;

	int (*open)(const char *f, int mode,dev_prvi_t*);
	int (*close)(dev_prvi_t*);
	int (*read)(dev_prvi_t*, off_t  , void *, int );
	int (*write)(dev_prvi_t*, off_t  ,  const void *, int );
	int (*ioctl)(dev_prvi_t*, int fd, void* args,int , int);
	

	int (*attach)(dev_prvi_t* devfp, void* *hwaddr);
	int (*detach)(dev_prvi_t* devfp);
	int (*transmit)(dev_prvi_t* devfp, void *p);
	int (*set_rx_mode)(dev_prvi_t* devfp);
	int (*receive)(void *netif, void *p);
	void* d_data; /*instance data */

	void *res0;
	void *res1;

} driver_ops_t;


#define MAJOR2DEVNNO(n) ((n)<<8)
#define	RAM_DEVNO MAJOR2DEVNNO(0x01)
#define CONSOLE_DEVNO MAJOR2DEVNNO(0x04)
/*enum{
	FLOPPY_DEVNO=MAJOR2DEVNNO(0x02),
	HD_DEVNO=MAJOR2DEVNNO(0x03),
	CON_DEVNO=MAJOR2DEVNNO(0x04),
	SER_DEVNO=MAJOR2DEVNNO(0x05),
	LP_DEVNO=MAJOR2DEVNNO(0x06),
	SISC_DEVNO=MAJOR2DEVNNO(0x07),
	SB_DEVNO=MAJOR2DEVNNO(0x08),
	NULL_DEVNO=MAJOR2DEVNNO(0x09),
	MOUSE_DEVNO=MAJOR2DEVNNO(0x0a),
	PIPE_DEVNO=MAJOR2DEVNNO(0x0b),
	USB_DEVNO=MAJOR2DEVNNO(0x0c),
	SYSTEM_DEVNO=MAJOR2DEVNNO(0x0d),
	MISC_DEVNO=MAJOR2DEVNNO(0x0E),
	RFB_DEVNO=MAJOR2DEVNNO(0x0f),
	KFS_DEVNO=MAJOR2DEVNNO(0x10),
	KFS1_DEVNO=MAJOR2DEVNNO(0x11),
	KFS2_DEVNO=MAJOR2DEVNNO(0x12),
	KFS3_DEVNO=MAJOR2DEVNNO(0x13),
	KFS4_DEVNO=MAJOR2DEVNNO(0x14),
	KFS5_DEVNO=MAJOR2DEVNNO(0x15),
	RAMDISK_DEVNO=MAJOR2DEVNNO(0x16),
};
*/
#define KMD_VERSION "0.20"
#define ALLOC_MAJOR_NUMBER (dev_t)-1


int free_irq_handler(int irq,void*);
void* put_irq_handler(int irq, void* handler, void*,const char*);
unsigned char * malloc(int size);
int isadma_startdma (int channelnr, void *addr, size_t len, int mode);
void isadma_stopdma (int channelnr);

__asmlink void enable_dma(unsigned int dmanr);
__asmlink void disable_dma(unsigned int dmanr);
__asmlink void isadma_stopdma (int channelnr);
__asmlink int isadma_startdma (int channelnr, void *addr, size_t len, int mode);
__asmlink  void set_dma_addr(unsigned int dmanr, unsigned int a);
__asmlink void set_dma_page(unsigned int dmanr, char pagenr);
__asmlink  void set_dma_mode(unsigned int dmanr, char mode);
__asmlink  void set_dma_count(unsigned int dmanr, unsigned int count);
__asmlink  void clear_dma_ff(unsigned int dmanr);

void device_init(void);
int kernel_driver_register(driver_ops_t *new_d);
int kernel_driver_unregister(driver_ops_t *old);
driver_ops_t *dev_driver_header(void);
int dev_ioctl(dev_prvi_t*,int,void *,int, int);
int dev_read(dev_prvi_t*,off_t pos, void *buf, unsigned len);
int dev_open(const char *path, unsigned access,dev_prvi_t*);
int dev_write(dev_prvi_t*, off_t pos, void *buf, unsigned len);
int dev_close(dev_prvi_t*);

int kb_read(u8_t* buffer, int max_len);
void mdelay(int sec);
int dev_receive(dev_prvi_t* devfp, void *p);
int dev_transmit(dev_prvi_t* devfp, void *p);
int dev_detach(dev_prvi_t* devfp);
int dev_attach(dev_prvi_t* devfp, void *netif,void*hwaddr, 
	int (*receive)(void *netif, void *p));

/*鼠标事件*/
enum { 
	E_MOUSE_TYPE,
	E_KEY_DOWN,
	E_KEY_UP,
};

struct Event {
	int what;
	int dev;
	int key;
	uint16_t modifiers;
	uint16_t value;
	int x, y;
};

typedef  int (input_func) (void*);

//提交输入事件
int submit_input_enent(struct Event*);
//设置事件管理者
int set_input_manager(input_func *inputfuc);
int reset_input_manager(void);

void clock_handler(void* arg, int irq,interrupt_regs_t reg);

#endif
