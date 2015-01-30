

#ifndef sym_dll_H
#define sym_dll_H
#include <sys/queue.h>
#include <drv/ia.h>

#ifdef __cplusplus
extern "C" {
#endif


#define KERNEL_VERSION(X,Y) ((X)<<8 | (Y))
#define CURRENT_KERNEL_VERSION KERNEL_VERSION(0,18)



struct bus_man
{
	int bus_no;
	int bus_name;
	int bus_count;
	void *(* bus_hooks)(void *arg);
	time_t bus_time;
};

int	busman_register(const struct bus_man *bus);
struct bus_man *busman_get(const char *busname);
int	busman_unregister(const struct bus_man *bus);


#define IOEVT_READ     0x0001
#define IOEVT_WRITE    0x0002
#define IOEVT_ERROR    0x0004
#define IOEVT_ACCEPT   0x0008
#define IOEVT_CONNECT  0x0010
#define IOEVT_CLOSE    0x0020

struct ioobject
{ 
  void *iomux;

  struct ioobject *next;
  struct ioobject *prev;

  unsigned short events_signaled;
  u32_t type;
  void *data;

};
void init_ioobject(struct ioobject *iob,char type[4]);
void clear_io_event(struct ioobject *iob, int events);
int set_io_event(struct ioobject *iob, int events);
void detach_ioobject(struct ioobject *iob);


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



int dev_ioctl(dev_prvi_t*,int,void *,int, int);
int dev_read(dev_prvi_t*,off_t pos, void *buf, unsigned len);
int dev_open(const char *path, unsigned access,dev_prvi_t*);
int dev_write(dev_prvi_t*, off_t pos, void *buf, unsigned len);
int dev_close(dev_prvi_t*);
int dev_receive(dev_prvi_t* devfp, void *p);
int dev_transmit(dev_prvi_t* devfp, void *p);
int dev_detach(dev_prvi_t* devfp);
int dev_attach(dev_prvi_t* devfp, void *netif,void*hwaddr, 
	int (*receive)(void *netif, void *p));

#define MAJOR2DEVNNO(n) ((n)<<8)

enum{
	RAM_DEVNO=MAJOR2DEVNNO(0x01),
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
};


struct _export_table_entry
{
	char *export_name;
	unsigned  export_addr;
	//int used;
};

#define EXPORT_PC_SYMBOL(sym_name) {#sym_name, (unsigned)sym_name }
#define EXPORT_DATA_SYMBOL(sym_name) {#sym_name, (unsigned)&sym_name }

int  install_dll_table(char *dll_namex, u32_t handle,
		int symbol_num, struct _export_table_entry *symbol_array);
int  remove_dll_table(char *dll_namex);

void sendsig(void *rp, int signo);
int sigdelset(void *rp, int signo);
int sigrecv(void *rp, char*);
void *find_thread(const char* tname);


u32_t proc_vis_addr(int nr, size_t pos);
void enable(void);
void disable(void);
int kernel_driver_register( driver_ops_t *new_d);
int kernel_driver_unregister( driver_ops_t *old);
void delay(int sec);

#define SEMAPHONE_CLEAR_FLAGS 0

void* create_semaphore(const char *name, int flags,int init_val);
int lock_semaphore(void* semaphore);
int lock_semaphore_timeout(void* semaphore,time_t timeout);
int unlock_semaphore(void* semaphore);
int unlock_semaphore_ex(void* semaphore,int cnt);
int destroy_semaphore(void* semaphore);
int trylock_semaphore(void* semaphore);
/*
int sem_down(void *s, unsigned timeout);
int sem_up(void *s);
void *sem_new( int cnt);
void sem_free(void *s);
*/
unsigned long startup_ticks();

//unsigned int malloc(unsigned int clicks);
int hex2num(char *str);
int printk( char * fmt, ...);
int kprintf( char * fmt, ...);
int puts(char *str); /*kernel function*/
int panic( char * fmt, ...);

int hex2num(char *str);
void flush_all(dev_t dev);
unsigned long get_page();
void * low_alloc(u16_t len);
int low_free(void* p,u32_t len);

u32_t get_irq_handler(int irq);
int free_irq_handler(int irq,void*);
void* put_irq_handler(int irq,unsigned long handler, void *arg,const char *name);

int register_server_task(int proc_nr, char *name, unsigned entry);
void read_real_memory(char *buf, int sz);
void fill_real_memory(char *buf, int sz);
void real_memory_addr(u32_t *addr, int *sz);
int change_server_entry(int proc_nr, unsigned entry);
int register_new_task(int *proc_nr, char *name, unsigned entry);
extern int blk_read(int dev, off_t pos, char * buf, int count);
extern int blk_write(int dev, off_t pos, char * buf, int count);
size_t snprintf (char *str, size_t len, const char *fmt, ...);
int sprintf(char *buf, const char *fmt, ...);
int unzip(void *src, unsigned long srclen, void *dst, unsigned long dstlen, char *heap, int heapsize);
char * strtok(char *s, const char *delim);
//unsigned long strlen (const char *s);
char *strcat(char *src_char, const char *add_char);
 int stricmp(const char *_src1, const char *_src2);
int hex2num(char *str);
void flush_all(dev_t dev);
 unsigned long get_page();
//int sscanf(char *buf,char *fmt,...);
// void * memcpy (void * dst, const void * src,unsigned int count);
//void * memset (void *dst,int val,unsigned int count);
unsigned long cur_addr_insystem(size_t pos);
//设置事件管理者
int map_physical_memory_nocache(void* vaddr, int size,const char *name);
void* map_high_memory(void* vaddr, int size,const char *name);

void *find_proc_bypid(pid_t pid);
void* kmalloc(unsigned int size, long flag);
void* kcalloc(size_t size);
void kfree(void *);
 int ksize(void *p);
unsigned long get_unix_time(void);
#define HZ 1000

void *memmove(void *dst, const void *src, size_t count);

typedef struct	/* circular queue */
{
	unsigned char *data;
	unsigned size, in_ptr, out_ptr;
} key_queue_t;

int deq(key_queue_t *q, unsigned char *data);
int inq(key_queue_t *q, unsigned char data);

//key_queue_t data_queue;

typedef struct
{
	void *head, *tail;
} wait_queue_t;

struct timeval 
{
  long tv_sec;		        // Seconds
  long tv_usec;		        // Microseconds
};


void waitq_init(wait_queue_t *queue);
bool waitq_is_empty(wait_queue_t *queue);
void wake_up(wait_queue_t *queue);
int sleep_on(wait_queue_t *queue, unsigned *timeout);
int strcmp(const char *s1, const char *s2);


void unlock_schedule();
void lock_schedule(int *);
void set_schedule(int );
void schedule(void);
int create_msgport(const char *ptname, void* t);
int connect_msgport(const char *pname);
int    msgport_send(int port,  void* va, u32_t count, int timeout);
int msgport_pend(int port, void* kbuf, u32_t count, unsigned long timeout);
int msgport_destroy(int port);
int msgport_wait(int port,   unsigned timeout);

#define KMD_VERSION "0.20"
#define MAJOR2DEVNNO(n) ((n)<<8)

enum
{
    IOCTL_GET_DEVICE_GEOMETRY = 1,
    IOCTL_REREAD_PTABLE=2,
    IOCTL_GET_DEVICE_PATH = 3,
	IOCTL_GET_DEVICE_HANDLE = 4,
	
	IOCTL_GET_APPSERVER_DRIVER = 5,
	IOCTL_GET_USERSPACE_DRIVER = 6,
	
    IOCTL_USER = 100000000
};

/*鼠标事件*/
enum { 
	E_MOUSE_TYPE,
	E_KEY_DOWN,
	E_KEY_UP,
	E_QUALIFIERS_CHANGED,
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
int reset_input_manager();

int post_thread_message(void *pthread,  void* va, u32_t count);
int get_thread_message(void *pthread, void* kbuf,	u32_t count, unsigned long timeout);

//
 int get_thread_msgport(void *pthread);

 char* get_kernel_command(void);

  pid_t current_thread_id(char **name);
  int set_thread_name(pid_t id, char *name);
   int set_thread_priority(pid_t id, int prio);


 pid_t new_kernel_thread(char *name, void* fn, void*arg);
 bool remove_kernel_thread(pid_t tid);


#ifdef __cplusplus
}
#endif


#endif


