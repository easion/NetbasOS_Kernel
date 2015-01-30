#include <string.h>
#include <errno.h>

#include "lwip/opt.h"
#include "lwip/api.h"
#include "lwip/arch.h"
#include "lwip/sys.h"

#include "lwip/sockets.h"


u32_t cur_addr_insystem(size_t pos);

static inline u32_t access_addr(size_t addr)
{
	if (addr == 0){
		return 0;
	}
	return cur_addr_insystem(addr);
}


#define API_SOCKET	1		/* sock(2)		*/
#define API_BIND		2		/* bind(2)			*/
#define API_CONNECT	3		/* connect(2)		*/
#define API_LISTEN	4		/* listen(2)		*/
#define API_ACCEPT	5		/* accept(2)		*/
#define API_GETSOCKNAME	6		/* getsockname(2)		*/
#define API_GETPEERNAME	7		/* getpeername(2)		*/
#define API_SOCKETPAIR	8		/* sockpair(2)		*/
#define API_SEND		9		/* send(2)			*/
#define API_RECV		10		/* recv(2)			*/
#define API_SENDTO	11		/* sendto(2)		*/
#define API_RECVFROM	12		/* recvfrom(2)		*/
#define API_SHUTDOWN	13		/* shutdown(2)		*/
#define API_SETSOCKOPT	14		/* setsockopt(2)		*/
#define API_GETSOCKOPT	15		/* getsockopt(2)		*/
#define API_SENDMSG	16		/* sendmsg(2)		*/
#define API_RECVMSG	17		/* recvmsg(2)		*/
#define API_SELECT	18		/* 		*/
#define API_IOCTL	19		/* 		*/


#define A2_NOFD(t0, t1) (t0)(argp[0]), (t1)(argp[1])
#define A3_NOFD(t0, t1, t2) A2_NOFD(t0, t1), (t2)argp[2]

#define A2(t0, t1) (t0)(argp[0]), (t1)(argp[1])
#define A3(t0, t1, t2) A2(t0, t1), (t2)(argp[2])
//static const unsigned char argc[18]={0,3,3,3,2,3,3,3,4,4,4,6,6,2,5,5,3,3};

#define A3_BIND_TYPE(t0, t1, t2) \
(t0)(argp[0]), (t1)access_addr(argp[1]), (t2)argp[2]

#define A3_ACCEPT_TYPE(t0, t1, t2) \
(t0)argp[0], (t1)access_addr(argp[1]), (t2)access_addr(argp[2])

#define A3_IOCTL_TYPE(t0, t1, t2) \
(t0)argp[0], (t1)cmd, (t2)(argp)

#define A4_PAIR_TYPE(t0, t1, t2, t3) A3(t0, t1, t2),\
(t3)access_addr(argp[3])

#define A4_SEND_TYPE(t0, t1, t2, t3) \
(t0)argp[0], (t1)access_addr(argp[1]),\
(t2)(argp[2]),(t3)(argp[3])


#define A5_SOPT_TYPE(t0, t1, t2, t3, t4) A3(t0, t1, t2),\
(t3)access_addr(argp[3]),(t4)(argp[4])

#define A5_GOPT_TYPE(t0, t1, t2, t3, t4) A3(t0, t1, t2),\
(t3)access_addr(argp[3]),(t4)access_addr(argp[4])

#define A5_SELECT_TYPE(t0, t1, t2, t3, t4) \
(t0)argp[0], \
(t1)access_addr(argp[1]),\
(t2)access_addr(argp[2]),\
(t3)access_addr(argp[3]),\
(t4)access_addr(argp[4])


#define A6_SENDTO_TYPE(t0, t1, t2,t3,t4,t5) \
(t0)argp[0], (t1)access_addr(argp[1]), (t2)(argp[2]),\
(t3)(argp[3]),(t4)access_addr(argp[4]),(t5)(argp[5])


#define A6_RECVFROM_TYPE(t0, t1, t2,t3,t4,t5) \
(t0)argp[0], (t1)access_addr(argp[1]), (t2)(argp[2]),\
(t3)(argp[3]),(t4)access_addr(argp[4]),(t5)access_addr(argp[5])

#define REALFD(N) ((N)& 0x000000ff)


int lwip_socket_call_hook(int cmd, unsigned long  * argp)
{
	int ret;
	//unsigned long  * argp = (void*)access_addr((unsigned long int)arg);

	kprintf("socket_call_hook() %d \n",cmd);

	switch (cmd)
	{
	case API_SOCKET:
		return lwip_socket(A3_NOFD(int, int, int));
	case API_BIND:
		return lwip_bind(A3_BIND_TYPE(int, struct sockaddr*, int));
	case API_CONNECT:
		return lwip_connect(A3_BIND_TYPE(int, struct sockaddr*, int));
	case API_LISTEN:
		return lwip_listen(A2(int, int));
	case API_ACCEPT:
		return lwip_accept(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*));
	case API_GETSOCKNAME:
		return lwip_getsockname(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*)); 
	case API_GETPEERNAME:
		return lwip_getpeername(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*)); 
	case API_SOCKETPAIR:
		return -1;//lwip_socketpair(A4_PAIR_TYPE(int, int, int, int*));
	case API_SEND:
		return lwip_send(A4_SEND_TYPE(int, void*, int, int));
	case API_SENDTO:
		//printf("send to called\n");
		return lwip_sendto(A6_SENDTO_TYPE(int, void*, int, int, 
			   struct sockaddr*, int));
	case API_RECV:
		return lwip_recv(A4_SEND_TYPE(int, void*, int, int));
	case API_RECVFROM:
		return lwip_recvfrom(A6_RECVFROM_TYPE(int, void*, int, int, 
			   struct sockaddr*, socklen_t*));
	case API_SHUTDOWN:
		return lwip_shutdown(A2(int, int));
	case API_SETSOCKOPT:
		return lwip_setsockopt(A5_SOPT_TYPE(int, int, int, void*, int));
	case API_GETSOCKOPT:
		return lwip_getsockopt(A5_GOPT_TYPE(int, int, int, void*, socklen_t*));
	case API_SENDMSG:
		return 0;//lwip_sendmsg(A3(int, msghdr_t*, int));
	case API_RECVMSG:
		return 0;//lwip_recvmsg(A3(int, msghdr_t*, int));
	default: 
		ret = -1;
		break;
	}

	return ret;
}


int lwip_drv_ioctl(int  file, int cmd, void* arg, int fromkernel)
{
	int ret;
	unsigned long  * argp = (void*)access_addr((unsigned long int)arg);
	int fd = REALFD(file);

	kprintf("lwip_drv_ioctl() %d %d\n",cmd, fd);


	switch (cmd)
	{
	case API_SELECT:{
		return lwip_select(
			A5_SELECT_TYPE(int,fd_set*,fd_set*,fd_set*, struct timeval *));
		}
	/*case API_IOCTL:{
		return lwip_ioctl(A3_IOCTL_TYPE(int,int,void*));
		}*/
	case API_SOCKET:
		return lwip_socket(A3_NOFD(int, int, int));
	case API_BIND:
		return lwip_bind(A3_BIND_TYPE(int, struct sockaddr*, int));
	case API_CONNECT:
		return lwip_connect(A3_BIND_TYPE(int, struct sockaddr*, int));
	case API_LISTEN:
		return lwip_listen(A2(int, int));
	case API_ACCEPT:
		return lwip_accept(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*));
	case API_GETSOCKNAME:
		return lwip_getsockname(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*)); 
	case API_GETPEERNAME:
		return lwip_getpeername(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*)); 
	case API_SOCKETPAIR:
		return -1;//lwip_socketpair(A4_PAIR_TYPE(int, int, int, int*));
	case API_SEND:
		return lwip_send(A4_SEND_TYPE(int, void*, int, int));
	case API_SENDTO:
		//printf("send to called\n");
		return lwip_sendto(A6_SENDTO_TYPE(int, void*, int, int, 
			   struct sockaddr*, int));
	case API_RECV:
		return lwip_recv(A4_SEND_TYPE(int, void*, int, int));
	case API_RECVFROM:
		return lwip_recvfrom(A6_RECVFROM_TYPE(int, void*, int, int, 
			   struct sockaddr*, socklen_t*));
	case API_SHUTDOWN:
		return lwip_shutdown(A2(int, int));
	case API_SETSOCKOPT:
		return lwip_setsockopt(A5_SOPT_TYPE(int, int, int, void*, int));
	case API_GETSOCKOPT:
		return lwip_getsockopt(A5_GOPT_TYPE(int, int, int, void*, socklen_t*));
	case API_SENDMSG:
		return 0;//lwip_sendmsg(A3(int, msghdr_t*, int));
	case API_RECVMSG:
		return 0;//lwip_recvmsg(A3(int, msghdr_t*, int));
	default: 
		kprintf("lwip_ioctl() with cmd %d\n", cmd);
		ret = lwip_ioctl(A3_IOCTL_TYPE(int,int,void*));
		break;
	}

	return ret;
}

 int lwip_drv_write(int minor, int  pos, const void * buf,int count)
{
	kprintf("lwip_drv_write(%d) called\n",minor);
	return 	lwip_write ((minor), buf, count);
}

 int lwip_drv_read(int minor, int  pos, void * buf,int count)
 {
	 kprintf("lwip_drv_read(%d) called\n",minor);
	return 	lwip_read ((minor), buf, count);
}

/*int lwip_drv_select(int file, int cmd, void *arg)
{
	unsigned long  * argp = (void*)access_addr((unsigned long int)arg);
	int fd = REALFD(file);

	return lwip_select(
			A5_SELECT_TYPE(int,fd_set*,fd_set*,fd_set*, struct timeval *));
}*/

int lwip_drv_close(int  file)
{	
	kprintf("lwip_drv_close(%d) called\n",file);
	lwip_close((file));
	return 0;
}




