#include <net/net.h>

#include <jicama/module.h>
#ifndef isprint
#define in_range(c, lo, up)  ((u8_t)c >= lo && (u8_t)c <= up)
#define isprint(c)           in_range(c, 0x20, 0x7f)
#define isdigit(c)           in_range(c, '0', '9')
#define isxdigit(c)          (isdigit(c) || in_range(c, 'a', 'f') || in_range(c, 'A', 'F'))
#define islower(c)           in_range(c, 'a', 'z')
#define isspace(c)           (c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v')
#endif    
#include <net/nf_hook.h> 

int inet_aton(const char *cp, struct in_addr *addr);
int socketpair(int domain, int type, int protocol, struct socket *retval[2]);

 err_t ip_forward(struct pbuf *p, struct ip_hdr *iphdr, struct netif *inp);
   
/**
 * Ascii internet address interpretation routine.
 * The value returned is in network order.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @return ip address in network order
 */
u32_t inet_addr(const char *cp)
{
  struct in_addr val;

  if (inet_aton(cp, &val)) {
    return (val.s_addr);
  }
  return (INADDR_NONE);
}

/**
 * Check whether "cp" is a valid ascii representation
 * of an Internet address and convert to a binary address.
 * Returns 1 if the address is valid, 0 if not.
 * This replaces inet_addr, the return value from which
 * cannot distinguish between failure and a local broadcast address.
 *
 * @param cp IP address in ascii represenation (e.g. "127.0.0.1")
 * @param addr pointer to which to save the ip address in network order
 * @return 1 if cp could be converted to addr, 0 on failure
 */
int inet_aton(const char *cp, struct in_addr *addr)
{
  u32_t val;
  int base, n, c;
  u32_t parts[4];
  u32_t *pp = parts;

  c = *cp;
  for (;;) {
    /*
     * Collect number up to ``.''.
     * Values are specified as for C:
     * 0x=hex, 0=octal, 1-9=decimal.
     */
    if (!isdigit(c))
      return (0);
    val = 0;
    base = 10;
    if (c == '0') {
      c = *++cp;
      if (c == 'x' || c == 'X') {
        base = 16;
        c = *++cp;
      } else
        base = 8;
    }
    for (;;) {
      if (isdigit(c)) {
        val = (val * base) + (int)(c - '0');
        c = *++cp;
      } else if (base == 16 && isxdigit(c)) {
        val = (val << 4) | (int)(c + 10 - (islower(c) ? 'a' : 'A'));
        c = *++cp;
      } else
        break;
    }
    if (c == '.') {
      /*
       * Internet format:
       *  a.b.c.d
       *  a.b.c   (with c treated as 16 bits)
       *  a.b (with b treated as 24 bits)
       */
      if (pp >= parts + 3)
        return (0);
      *pp++ = val;
      c = *++cp;
    } else
      break;
  }
  /*
   * Check for trailing characters.
   */
  if (c != '\0' && (!isprint(c) || !isspace(c)))
    return (0);
  /*
   * Concoct the address according to
   * the number of parts specified.
   */
  n = pp - parts + 1;
  switch (n) {

  case 0:
    return (0);       /* initial nondigit */

  case 1:             /* a -- 32 bits */
    break;

  case 2:             /* a.b -- 8.24 bits */
    if (val > 0xffffffUL)
      return (0);
    val |= parts[0] << 24;
    break;

  case 3:             /* a.b.c -- 8.8.16 bits */
    if (val > 0xffff)
      return (0);
    val |= (parts[0] << 24) | (parts[1] << 16);
    break;

  case 4:             /* a.b.c.d -- 8.8.8.8 bits */
    if (val > 0xff)
      return (0);
    val |= (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8);
    break;
  }
  if (addr)
    addr->s_addr = htonl(val);
  return (1);
}

/**
 * Convert numeric IP address into decimal dotted ASCII representation.
 * returns ptr to static buffer; not reentrant!
 *
 * @param addr ip address in network order to convert
 * @return pointer to a global static (!) buffer that holds the ASCII
 *         represenation of addr
 */
char *
inet_ntoa(struct in_addr addr)
{
  static char str[16];
  u32_t s_addr = addr.s_addr;
  char inv[3];
  char *rp;
  u8_t *ap;
  u8_t rem;
  u8_t n;
  u8_t i;

  rp = str;
  ap = (u8_t *)&s_addr;
  for(n = 0; n < 4; n++) {
    i = 0;
    do {
      rem = *ap % (u8_t)10;
      *ap /= (u8_t)10;
      inv[i++] = '0' + rem;
    } while(*ap);
    while(i--)
      *rp++ = inv[i];
    *rp++ = '.';
    ap++;
  }
  *--rp = 0;
  return str;
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
//#define API_SELECT	18		/* 		*/
//#define API_IOCTL	19		/* 		*/


#define A2_NOFD(t0, t1) (t0)(argp[0]), (t1)(argp[1])
#define A3_NOFD(t0, t1, t2) A2_NOFD(t0, t1), (t2)argp[2]

#define A2(t0, t1) (t0)(fd), (t1)(argp[1])
#define A3(t0, t1, t2) A2(t0, t1), (t2)(argp[2])
//static const unsigned char argc[18]={0,3,3,3,2,3,3,3,4,4,4,6,6,2,5,5,3,3};

#define A3_BIND_TYPE(t0, t1, t2) \
(t0)(fd), (t1)current_proc_vir2phys(argp[1]), (t2)argp[2]

#define A3_ACCEPT_TYPE(t0, t1, t2) \
(t0)(fd), (t1)current_proc_vir2phys(argp[1]), (t2)current_proc_vir2phys(argp[2])

#define A3_IOCTL_TYPE(t0, t1, t2) \
(t0)(fd), (t1)cmd, (t2)(argp)

#define A4_PAIR_TYPE(t0, t1, t2, t3) A3(t0, t1, t2),\
(t3)current_proc_vir2phys(argp[3])

#define A4_SEND_TYPE(t0, t1, t2, t3) \
(t0)(fd), (t1)current_proc_vir2phys(argp[1]),\
(t2)(argp[2]),(t3)(argp[3])


#define A5_SOPT_TYPE(t0, t1, t2, t3, t4) A3(t0, t1, t2),\
(t3)current_proc_vir2phys(argp[3]),(t4)(argp[4])

#define A5_GOPT_TYPE(t0, t1, t2, t3, t4) A3(t0, t1, t2),\
(t3)current_proc_vir2phys(argp[3]),(t4)current_proc_vir2phys(argp[4])



#define A6_SENDTO_TYPE(t0, t1, t2,t3,t4,t5) \
(t0)(fd), (t1)current_proc_vir2phys(argp[1]), (t2)(argp[2]),\
(t3)(argp[3]),(t4)current_proc_vir2phys(argp[4]),(t5)(argp[5])


#define A6_RECVFROM_TYPE(t0, t1, t2,t3,t4,t5) \
(t0)(fd), (t1)current_proc_vir2phys(argp[1]), (t2)(argp[2]),\
(t3)(argp[3]),(t4)current_proc_vir2phys(argp[4]),(t5)current_proc_vir2phys(argp[5])


int socket_call_hook(int cmd, unsigned long  * argp)
{
	int ret;
	u32_t fd_type=0;
	struct socket * fd=NULL;
	//unsigned long  * argp = (void*)current_proc_vir2phys((unsigned long int)arg);

	trace("socket_call_hook() cmd=%d \n",cmd);

	switch (cmd)
	{
	case API_SOCKET:{
		void *ret;
		int err = new_socket(A3_NOFD(int, int, int), &ret);
		if (err<0)
		{
			return err;
		}
		ret = fs_create_file_filp(ret,1);
		//kprintf("socket_call_hook fs_create_file_filp = %d,tid=%d\n",ret,current_thread()->tid);
		return ret;
	}
	case API_BIND:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return bind(A3_BIND_TYPE(int, struct sockaddr*, int));
	case API_CONNECT:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return connect(A3_BIND_TYPE(int, struct sockaddr*, int));
	case API_LISTEN:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return listen(A2(int, int));
	case API_ACCEPT:{
		void *ret;

		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	kprintf("accept bad fd %d\n",argp[0]);return EBADF;}
		//kprintf("accept by fd %d\n",argp[0]);
		int err= accept(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*),&ret);
		if (err<0)
		{
			return err;
		}
		return fs_create_file_filp(ret,1);
	}
	case API_GETSOCKNAME:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return getsockname(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*)); 
	case API_GETPEERNAME:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return getpeername(A3_ACCEPT_TYPE(int, struct sockaddr*, socklen_t*)); 
	case API_SOCKETPAIR:
		//fd = fs_get_prvi_data(argp[0],&fd_type);
		//if (!fd){	return EBADF;}
		return socketpair(A4_PAIR_TYPE(int, int, int, int*));
	case API_SEND:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return send(A4_SEND_TYPE(int, void*, int, int));
	case API_SENDTO:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		//printf("send to called\n");
		return sendto(A6_SENDTO_TYPE(int, void*, int, int, 
			   struct sockaddr*, int));
	case API_RECV:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return recv(A4_SEND_TYPE(int, void*, int, int));
	case API_RECVFROM:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return recvfrom(A6_RECVFROM_TYPE(int, void*, int, int, 
			   struct sockaddr*, socklen_t*));
	case API_SHUTDOWN:
		remove_socket_filp(argp[0]);
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return shutdown(A2(int, int));
	case API_SETSOCKOPT:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return setsockopt(A5_SOPT_TYPE(int, int, int, void*, int));
	case API_GETSOCKOPT:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return getsockopt(A5_GOPT_TYPE(int, int, int, void*, socklen_t*));
	case API_SENDMSG:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return sendmsg(A3(int, struct msghdr *, int));
	case API_RECVMSG:
		fd = fs_get_prvi_data(argp[0],&fd_type);
		if (!fd){	return EBADF;}
		return recvmsg(A3(int, struct msghdr *, int));
	default: 
		ret = -1;
		break;
	}

	return ret;
}

static struct _export_table_entry sym_table []=
{
#if 0
	EXPORT_PC_SYMBOL(inet_ntoa),
	EXPORT_PC_SYMBOL(inet_aton),
#endif
	//socket操作
	EXPORT_PC_SYMBOL(closesocket),
	/*bsd socket style*/
	EXPORT_PC_SYMBOL(accept),
	EXPORT_PC_SYMBOL(bind),
	EXPORT_PC_SYMBOL(bind),
	EXPORT_PC_SYMBOL(shutdown),
	EXPORT_PC_SYMBOL(connect),
	EXPORT_PC_SYMBOL(getsockname),
	EXPORT_PC_SYMBOL(getpeername),
	EXPORT_PC_SYMBOL(setsockopt),
	EXPORT_PC_SYMBOL(getsockopt),
	EXPORT_PC_SYMBOL(listen),
	EXPORT_PC_SYMBOL(recv),
	EXPORT_PC_SYMBOL(recvfrom),
	EXPORT_PC_SYMBOL(send),
	EXPORT_PC_SYMBOL(sendto),
	EXPORT_PC_SYMBOL(new_socket),

	//tcp/ip输入输出
	EXPORT_PC_SYMBOL(ether_interface_register),
	EXPORT_PC_SYMBOL(ether_interface_unregister),
	EXPORT_PC_SYMBOL(ether_input),
	EXPORT_PC_SYMBOL(ether_output),
	EXPORT_PC_SYMBOL(ip_input),
	EXPORT_PC_SYMBOL(ip_forward),
	EXPORT_PC_SYMBOL(ip_output),
	EXPORT_PC_SYMBOL(ip_route),
	EXPORT_PC_SYMBOL(netif_add),
	EXPORT_PC_SYMBOL(netif_delete),
	
	//pbuf操作
	EXPORT_PC_SYMBOL(pbuf_free),
	EXPORT_PC_SYMBOL(pbuf_alloc),
	EXPORT_PC_SYMBOL(pbuf_header),
	EXPORT_PC_SYMBOL(pbuf_ref),
	EXPORT_PC_SYMBOL(pbuf_realloc),
	EXPORT_PC_SYMBOL(pbuf_clen),
	EXPORT_PC_SYMBOL(pbuf_dechain),
	EXPORT_PC_SYMBOL(pbuf_chain),
	EXPORT_PC_SYMBOL(pbuf_dup),
	EXPORT_PC_SYMBOL(pbuf_linearize),
	EXPORT_PC_SYMBOL(pbuf_cow),

	EXPORT_PC_SYMBOL(nf_unregister_hook),
	EXPORT_PC_SYMBOL(nf_register_hook),

	EXPORT_PC_SYMBOL(nf_register_sockopt),
	EXPORT_PC_SYMBOL(nf_unregister_sockopt),

#if 0
	EXPORT_PC_SYMBOL(pbuf_ref_chain),
	EXPORT_PC_SYMBOL(pbuf_cat),
	EXPORT_PC_SYMBOL(pbuf_queue),
	EXPORT_PC_SYMBOL(pbuf_dequeue),
#endif
};


static int export_symtab()
{
	int num =sizeof(sym_table)/sizeof(struct _export_table_entry);
	//proc_entry_init();
	return install_dll_table("tcpip.dll", 1, num, sym_table);
}

void remove_symtab()
{
	remove_dll_table("tcpip.dll");
}

static struct peb __peb;
struct peb *peb=NULL;
void init_net(void)
{
	peb=&__peb;

	setup_unix_socketcall(socket_call_hook);
	export_symtab();

	nf_init();
	nf_sockopt_init();

	stats_init();
	netif_init();
	ether_init();
	pbuf_init();
	arp_init();
	ip_init();
	udp_init();
	raw_init();
	uds_init();
	dhcp_init();
	tcp_init();
	socket_init();
	loopif_init();
#if 0
	dns_init();
#endif
}


