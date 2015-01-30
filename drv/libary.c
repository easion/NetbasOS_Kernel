
#include <sys/types.h>
#include <sys/param.h>
#include <stdio.h>
#include <string.h>
#include <int86.h>
#include <servers.h>

#define MSG_CALL 0x40

static inline int msgcall(int func, int task, msg_t *ptr)
{
  int return_value;
    __asm__ volatile ("int $0x40" \
        : "=a" (return_value) \
        : "0" ((long)(0x00)),"b" ((long)(func)),"c" ((long)(task)),"d" ((long)(ptr))); \
    return return_value;
}

int send(int task, msg_t *ptr)
{
	return msgcall(SEND, task , ptr);
}

int receive(int task, msg_t *ptr)
{
	return msgcall(RECEIVE, task , ptr);
}

int reply(int task, msg_t *ptr)
{
	return msgcall(REPLY, task , ptr);
}


int get_boot_arg(struct bparam_s *p)
{
	int ret=-1;
	msg_t m;

	memset(&m, 0, sizeof(msg_t));

	m.func=0;
	m.message_len=sizeof(struct bparam_s);
	m.msg_addr= p;
	ret=send(DEV_SERVER, &m);
	if(ret<0)printk("send boot message failed!\n");
	return ret;
}

static int dev_ops(int op, int dev, off_t pos, char *buf, int c, int arg )
{
	int ret=-1;
	msg_t m;

	struct args
	{
		int op_code;
		int dev;
		off_t pos;
		char * buf;
		int count;
		int arg;
	};
	struct args a;
	a.op_code=op;
	a.dev=dev;
	a.pos=pos;
	a.buf=buf;
	a.count=c;
	a.arg=arg;

	memset(&m, 0, sizeof(msg_t));

	m.func=0;
	m.message_len=sizeof(struct args);
	m.msg_addr= (u32_t)&a;
	ret=send(DEV_SERVER, &m);
	return ret;
}

int blk_read_lib(int dev, off_t pos, char * buf, int count)
{
	return dev_ops(1, dev,  pos, buf, count, 0 );
	}

