

#include <net/net.h>
#include <jicama/msgport.h>

unsigned long tcp_ticks,fast_tcp_ticks;
unsigned long iss;
unsigned short tcp_next_port;

struct timer tcpslow_timer;
struct timer tcpfast_timer;
struct task tcp_slow_task;
struct task tcp_fast_task;
static sem_t tcp_time_sem_lock;
static sem_t tcp_sem;

unsigned char tcp_backoff[13] = {1, 2, 3, 4, 5, 6, 7, 7, 7, 7, 7, 7, 7};
const u8_t tcp_persist_backoff[7] = { 3, 6, 12, 24, 48, 96, 120 };

// TCP PCB lists

struct tcp_pcb_listen *tcp_listen_pcbs; // TCP PCBs in LISTEN state
struct tcp_pcb *tcp_active_pcbs;        // TCP PCBs that are in a state in which they accept or send data
struct tcp_pcb *tcp_tw_pcbs;            // TCP PCBs in TIME-WAIT

//#define MIN(x,y) ((x) < (y) ? (x): (y))

//
// tcpstat_proc
//

static int tcpstat_proc(void *arg, int len,struct proc_entry *pf)
{
  static char *statename[] = {"CLOSED", "LISTEN", "SYN_SENT", "SYN_RCVD", "ESTABLISHED", "FIN_WAIT_1", "FIN_WAIT_2", "CLOSE_WAIT", "CLOSING", "LAST_ACK", "TIME_WAIT"};
  struct tcp_pcb *pcb;

  pprintf(pf,"tcp_ticks = %d, fast_tcp_ticks=%d\n",tcp_ticks,fast_tcp_ticks);

  pprintf(pf, "local port  remote port local ip        remote ip       state\n");
  pprintf(pf, "----------- ----------- --------------- --------------- -----------\n");

  for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    pprintf(pf, "%8d    %8d    %-15s %-15s %s\n", 
		pcb->local_port, pcb->remote_port, inetntoa(pcb->local_ip.addr), 
		inetntoa(pcb->remote_ip.addr), statename[pcb->state]);
  }

  for (pcb = (struct tcp_pcb *) tcp_listen_pcbs; pcb != NULL; pcb = pcb->next)
  {
    pprintf(pf, "%8d    %8d    %-15s %-15s %s\n", 
		pcb->local_port, pcb->remote_port, 
		inetntoa(pcb->local_ip.addr), inetntoa(pcb->remote_ip.addr), statename[pcb->state]);
  }    

  for (pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    pprintf(pf, "%8d    %8d    %-15s %-15s %s\n", 
		pcb->local_port, pcb->remote_port, 
		inetntoa(pcb->local_ip.addr), inetntoa(pcb->remote_ip.addr), statename[pcb->state]);
  }    

  return 0;
}

//
// tcp_new_port
//
// A nastly hack featuring 'goto' statements that allocates a
// new TCP local port.
//

static unsigned short tcp_new_port()
{
  struct tcp_pcb *pcb;

again:
  if (++tcp_next_port > 0x7FFF) tcp_next_port = 4096;
  
  for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
  {
    if (pcb->local_port == tcp_next_port)
		goto again;
  }

  for (pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next)
  {
    if (pcb->local_port == tcp_next_port)
		goto again;
  }

  for (pcb = (struct tcp_pcb *)tcp_listen_pcbs; pcb != NULL; pcb = pcb->next)
  {
    if (pcb->local_port == tcp_next_port)
		goto again;
  }

  return tcp_next_port;
}

//
// tcp_close
//
// Closes the connection held by the PCB.
//

err_t tcp_close(struct tcp_pcb *pcb)
{
  err_t err;

  switch (pcb->state)
  {
    case LISTEN:
      err = 0;
	  kprintf("tcp_close called\n");
      tcp_pcb_remove((struct tcp_pcb **) &tcp_listen_pcbs, pcb);
      kfree(pcb);
      pcb = NULL;
      break;

    case SYN_SENT:
      err = 0;
      tcp_pcb_remove(&tcp_active_pcbs, pcb);
      kfree(pcb);
      pcb = NULL;
      break;

    case SYN_RCVD:
      err = tcp_send_ctrl(pcb, TCP_FIN);
      if (err == 0) pcb->state = FIN_WAIT_1;
      break;

    case ESTABLISHED:
      err = tcp_send_ctrl(pcb, TCP_FIN);
      if (err == 0) pcb->state = FIN_WAIT_1;
      break;

    case CLOSE_WAIT:
      err = tcp_send_ctrl(pcb, TCP_FIN);
      if (err == 0) pcb->state = LAST_ACK;
      break;

    default:
      // Has already been closed, do nothing
      err = 0;
      pcb = NULL;
      break;
  }

  if (pcb != NULL && err == 0) {
    /* To ensure all data has been sent when tcp_close returns, we have
       to make sure tcp_output doesn't fail.
       Since we don't really have to ensure all data has been sent when tcp_close
       returns (unsent data is sent from tcp timer functions, also), we don't care
       for the return value of tcp_output for now. */
    /* @todo: When implementing SO_LINGER, this must be changed somehow:
       If SOF_LINGER is set, the data should be sent when tcp_close returns. */
    tcp_output(pcb);
  }
  return err;
}

//
// tcp_abort
//
// Aborts a connection by sending a RST to the remote host and deletes
// the local protocol control block. 
//

void tcp_abort(struct tcp_pcb *pcb)
{
  u32_t seqno, ackno;
  u16_t remote_port, local_port;
  struct ip_addr remote_ip, local_ip;
  void (*errf)(void *arg, err_t err);
  void *errf_arg;

  // Figure out on which TCP PCB list we are, and remove us. If we
  // are in an active state, call the receive function associated with
  // the PCB with a NULL argument, and send an RST to the remote end.

  if (pcb->state == TIME_WAIT)
  {
    tcp_pcb_remove(&tcp_tw_pcbs, pcb);
    kfree(pcb);
  }
  else 
  {
    seqno = pcb->snd_nxt;
    ackno = pcb->rcv_nxt;
    ip_addr_set(&local_ip, &pcb->local_ip);
    ip_addr_set(&remote_ip, &pcb->remote_ip);
    local_port = pcb->local_port;
    remote_port = pcb->remote_port;
    errf = pcb->errf;
    errf_arg = pcb->callback_arg;

    tcp_pcb_remove(&tcp_active_pcbs, pcb);
    if (pcb->unacked != NULL) {
      tcp_segs_free(pcb->unacked);
    }
    if (pcb->unsent != NULL) {
      tcp_segs_free(pcb->unsent);
    }
    if (pcb->ooseq != NULL) {
      tcp_segs_free(pcb->ooseq);
    }

    if (errf != NULL) errf(errf_arg, EFAULT);
    kfree(pcb);
    //kprintf("tcp_abort: sending RST\n");
    tcp_rst(seqno, ackno, &local_ip, &remote_ip, local_port, remote_port);
  }
}

//
// tcp_bind
//
// Binds the connection to a local portnumber and IP address. If the
// IP address is not given (i.e., ipaddr == NULL), the IP address of
// the outgoing network interface is used instead.
//

err_t tcp_bind(struct tcp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port)
{
  struct tcp_pcb *cpcb;

  // Assign new port if port number is zero
  if (port == 0) port = tcp_new_port();

  // Check if the address already is in use
  for (cpcb = (struct tcp_pcb *) tcp_listen_pcbs; cpcb != NULL; cpcb = cpcb->next) 
  {
    if (cpcb->local_port == port)
    {
      if (ip_addr_isany(&cpcb->local_ip) ||
	  ip_addr_isany(ipaddr) ||
	  ip_addr_cmp(&cpcb->local_ip, ipaddr)) 
      {
		kprintf("%s() address already is in use on listen\n",__FUNCTION__);
		return EADDRINUSE;
      }
    }
  }

  for (cpcb = tcp_active_pcbs; cpcb != NULL; cpcb = cpcb->next) 
  {
    if (cpcb->local_port == port)
    {
      if (ip_addr_isany(&cpcb->local_ip) ||
	  ip_addr_isany(ipaddr) ||
	  ip_addr_cmp(&cpcb->local_ip, ipaddr))
      {
		kprintf("%s() address already is in use on active\n",__FUNCTION__);
		return EADDRINUSE;
      }
    }
  }

  if (!ip_addr_isany(ipaddr)) 
  {
    if (!ip_ownaddr(ipaddr)) 
		return EADDRNOTAVAIL;
    pcb->local_ip = *ipaddr;
  }

  pcb->local_port = port;
  
  //kprintf("tcp_bind: bind to port %d\n", port);
  return 0;
}

//
// tcp_listen
//
// Set the state of the connection to be LISTEN, which means that it
// is able to accept incoming connections. The protocol control block
// is reallocated in order to consume less memory. Setting the
// connection to LISTEN is an irreversible process.
//

struct tcp_pcb *tcp_listen(struct tcp_pcb *pcb)
{
  if (pcb->state == LISTEN) 
	  return pcb;
  pcb->state = LISTEN;
  TCP_REG((struct tcp_pcb **) &tcp_listen_pcbs, pcb);

  return pcb;
}

//
// tcp_recved
//
// This function should be called by the application when it has
// processed the data. The purpose is to advertise a larger window
// when the data has been processed.
//

void tcp_recved(struct tcp_pcb *pcb, int len)
{
	struct netif *netif;
	int mtu = TCP_MSS;

    netif = ip_route(&pcb->remote_ip);
    if(netif != NULL){
		mtu = netif->mtu_size;
	}

	LOCK_SCHED(tcp_sem);

  pcb->rcv_wnd += len;
  if (pcb->rcv_wnd > TCP_WND) pcb->rcv_wnd = TCP_WND;
  
  //if (!(pcb->flags & TF_ACK_DELAY) && !(pcb->flags & TF_ACK_NOW)) tcp_ack(pcb);
  if (!(pcb->flags & TF_IN_RECV)) pcb->flags |= TF_ACK_DELAY;
  UNLOCK_SCHED(tcp_sem);

  //kprintf("tcp_recved: received %d bytes, wnd %u (%u).\n", len, pcb->rcv_wnd, TCP_WND - pcb->rcv_wnd);
}

//
// tcp_connect()
//
// Connects to another host. The function given as the "connected"
// argument will be called when the connection has been established.
//
//

err_t tcp_connect(struct tcp_pcb *pcb, struct ip_addr *ipaddr, unsigned short port,
	          err_t (*connected)(void *arg, struct tcp_pcb *tpcb, err_t err))
{
  u32_t optdata;
  err_t ret;
  u32_t iss;
  struct netif *netif;
  int mtu=TCP_MSS;

  //kprintf("tcp_connect to port %d\n", port);

  if (ip_addr_isany(ipaddr) || port == 0) return EADDRNOTAVAIL;

  pcb->remote_ip = *ipaddr;
  pcb->remote_port = port;

  netif = ip_route(ipaddr);

  if (netif)
  {
	  mtu = netif->mtu_size;
	  //kprintf("%s : mtu=%d\n",__FUNCTION__,mtu);
  }


  if (pcb->local_port == 0) pcb->local_port = tcp_new_port();
  iss = tcp_next_iss();
  pcb->rcv_nxt = 0;
  pcb->snd_nxt = iss;
  pcb->lastack = iss - 1;
  pcb->snd_lbb = iss - 1;
  pcb->rcv_wnd = TCP_WND;
  pcb->snd_wnd = TCP_WND;
  pcb->mss = mtu;
  pcb->cwnd = 1;
  pcb->ssthresh = pcb->mss * 10;
  pcb->state = SYN_SENT;
  pcb->connected = connected;
  TCP_REG(&tcp_active_pcbs, pcb);
  
  // Build an MSS option
  optdata = HTONL(((unsigned long) 2 << 24) | 
		  ((unsigned long) 4 << 16) | 
		  (((unsigned long) pcb->mss / 256) << 8) |
		  (pcb->mss & 255));

  ret = tcp_enqueue(pcb, NULL, 0, TCP_SYN, (unsigned char *) &optdata, 4);
  if (ret == 0) tcp_output(pcb);

  return ret;
} 

void
tcp_keepalive(struct tcp_pcb *pcb);

void
tcp_keepalive(struct tcp_pcb *pcb)
{
   struct pbuf *p;
   struct tcp_hdr *tcphdr;

   /*LWIP_DEBUGF(TCP_DEBUG, ("tcp_keepalive: sending KEEPALIVE probe to %"U16_F".%"U16_F".%"U16_F".%"U16_F"\n",
                           ip4_addr1(&pcb->remote_ip), ip4_addr2(&pcb->remote_ip),
                           ip4_addr3(&pcb->remote_ip), ip4_addr4(&pcb->remote_ip)));

   LWIP_DEBUGF(TCP_DEBUG, ("tcp_keepalive: tcp_ticks %"U32_F"   pcb->tmr %"U32_F"  pcb->keep_cnt %"U16_F"\n", tcp_ticks, pcb->tmr, pcb->keep_cnt));
   */

   p = pbuf_alloc(PBUF_IP, TCP_HLEN, PBUF_RAM);

   if(p == NULL) {
      //LWIP_DEBUGF(TCP_DEBUG, ("tcp_keepalive: could not allocate memory for pbuf\n"));
      return;
   }

   tcphdr = p->payload;
   tcphdr->src = htons(pcb->local_port);
   tcphdr->dest = htons(pcb->remote_port);
   tcphdr->seqno = htonl(pcb->snd_nxt - 1);
   tcphdr->ackno = htonl(pcb->rcv_nxt);
   tcphdr->wnd = htons(pcb->rcv_wnd);
   tcphdr->urgp = 0;
   TCPH_HDRLEN_SET(tcphdr, 5);
   
   tcphdr->chksum = 0;
#if CHECKSUM_GEN_TCP
   tcphdr->chksum = inet_chksum_pseudo(p, &pcb->local_ip, &pcb->remote_ip, IP_PROTO_TCP, p->tot_len);
#endif
  TCP_STATS_INC(tcp.xmit);

   /* Send output to IP */
  ip_output(p, &pcb->local_ip, &pcb->remote_ip, TCP_TTL,  IP_PROTO_TCP);

  pbuf_free(p);

  //LWIP_DEBUGF(TCP_RST_DEBUG, ("tcp_keepalive: seqno %"U32_F" ackno %"U32_F".\n", pcb->snd_nxt - 1, pcb->rcv_nxt));
}




//
// tcp_slowtmr
//
// Called every 500 ms and implements the retransmission timer and the timer that
// removes PCBs that have been in TIME-WAIT for enough time. It also increments
// various timers such as the inactivity timer in each PCB.
//

void tcp_slowtmr(void *arg)
{
  struct tcp_pcb *pcb, *pcb2, *prev;
  unsigned long eff_wnd;
  int pcb_remove;      // flag if a PCB should be removed

  lock_semaphore(tcp_time_sem_lock);

  tcp_ticks++;
  //kprintf("tcp_slowtmr: ticks %d\n", tcp_ticks);

  // Steps through all of the active PCBs.
  prev = NULL;
  pcb = tcp_active_pcbs;
  while (pcb != NULL) 
  {
    pcb_remove = 0;

    if (pcb->state == SYN_SENT && pcb->nrtx == TCP_SYNMAXRTX)
      pcb_remove++;
    else if (pcb->nrtx == TCP_MAXRTX)
      pcb_remove++;
    else 
    {
      pcb->rtime++;
      if (pcb->unacked != NULL && pcb->rtime >= pcb->rto) 
      {
        //kprintf("tcp_slowtmr: rtime %ld pcb->rto %d\n", tcp_ticks - pcb->rtime, pcb->rto);

	// Double retransmission time-out unless we are trying to
        // connect to somebody (i.e., we are in SYN_SENT)
	if (pcb->state != SYN_SENT) 
	{
	  pcb->rto = ((pcb->sa >> 3) + pcb->sv) << tcp_backoff[pcb->nrtx];
	}


	// Retransmit unacknowled segments
	tcp_rexmit(pcb);

	// Reduce congestion window and ssthresh
        eff_wnd = MIN(pcb->cwnd, pcb->snd_wnd);
        pcb->ssthresh = eff_wnd >> 1;
        if (pcb->ssthresh < (unsigned long) pcb->mss) pcb->ssthresh = pcb->mss * 2;
        pcb->cwnd = pcb->mss;

       // kprintf("tcp_rexmit_seg: cwnd %u ssthresh %u\n", pcb->cwnd, pcb->ssthresh);
      }
    }
	  
    // Check if this PCB has stayed too long in FIN-WAIT-2
    if (pcb->state == FIN_WAIT_2) 
    {
      if ((unsigned long) (tcp_ticks - pcb->tmr) > TCP_FIN_WAIT_TIMEOUT / TCP_SLOW_INTERVAL) pcb_remove++;
    }

	 /* Check if KEEPALIVE should be sent */
   if((pcb->so_options & SOF_KEEPALIVE) && ((pcb->state == ESTABLISHED) || (pcb->state == CLOSE_WAIT))) {
      if((u32_t)(tcp_ticks - pcb->tmr) > (pcb->keepalive + TCP_MAXIDLE) / TCP_SLOW_INTERVAL)  {
         /*LWIP_DEBUGF(TCP_DEBUG, ("tcp_slowtmr: KEEPALIVE timeout. Aborting connection to %"U16_F".%"U16_F".%"U16_F".%"U16_F".\n",
                                 ip4_addr1(&pcb->remote_ip), ip4_addr2(&pcb->remote_ip),
                                 ip4_addr3(&pcb->remote_ip), ip4_addr4(&pcb->remote_ip)));
								 */

         tcp_abort(pcb);
      }
      else if((u32_t)(tcp_ticks - pcb->tmr) > (pcb->keepalive + pcb->keep_cnt * TCP_KEEPINTVL) / TCP_SLOW_INTERVAL) {
         tcp_keepalive(pcb);
         pcb->keep_cnt++;
      }
   }


    // If this PCB has queued out of sequence data, but has been
    // inactive for too long, will drop the data (it will eventually
    // be retransmitted).

    if (pcb->ooseq != NULL && (unsigned long) (tcp_ticks - pcb->tmr) >= (unsigned long) (pcb->rto * TCP_OOSEQ_TIMEOUT)) 
    {
      tcp_segs_free(pcb->ooseq);
      pcb->ooseq = NULL;
    }

    // Check if this PCB has stayed too long in SYN-RCVD
    if (pcb->state == SYN_RCVD) 
    {
      if ((unsigned long)(tcp_ticks - pcb->tmr) > TCP_SYN_RCVD_TIMEOUT / TCP_SLOW_INTERVAL) pcb_remove++;
    }

    // If the PCB should be removed, do it
    if (pcb_remove) 
    {
      tcp_pcb_purge(pcb);      
      
      // Remove PCB from tcp_active_pcbs list
      if (prev != NULL) 
        prev->next = pcb->next;
      else 
        tcp_active_pcbs = pcb->next;

      if (pcb->errf != NULL) 
      {
	pcb->errf(pcb->callback_arg, EFAULT);
      }

      pcb2 = pcb->next;
      kfree(pcb);
      pcb = pcb2;
    } 
    else 
    {
      // We check if we should poll the connection
      pcb->polltmr++;
      if (pcb->polltmr >= pcb->pollinterval && pcb->poll != NULL) 
      {
	pcb->polltmr = 0;
	pcb->poll(pcb->callback_arg, pcb);
	tcp_output(pcb);
      }
      
      prev = pcb;
      pcb = pcb->next;
    }
  }
  
  // Steps through all of the TIME-WAIT PCBs.
  prev = NULL;    
  pcb = tcp_tw_pcbs;
  while (pcb != NULL) 
  {
    pcb_remove = 0;

    // Check if this PCB has stayed long enough in TIME-WAIT
    if ((unsigned long)(tcp_ticks - pcb->tmr) > 2 * TCP_MSL_MSEC / TCP_SLOW_INTERVAL) 
		pcb_remove++;
    
    // If the PCB should be removed, do it
    if (pcb_remove) 
    {
      tcp_pcb_purge(pcb);

      // Remove PCB from tcp_tw_pcbs list
      if (prev != NULL)
        prev->next = pcb->next;
      else
        tcp_tw_pcbs = pcb->next;

      pcb2 = pcb->next;
      kfree(pcb);
      pcb = pcb2;
    } 
    else
    {
      prev = pcb;
      pcb = pcb->next;
    }
  }

  unlock_semaphore(tcp_time_sem_lock);
}

//
//
// tcp_slow_handler
//

void tcp_slow_handler(void *arg)
{
  queue_task( &tcp_slow_task, tcp_slowtmr, NULL);
  mod_timer(&tcpslow_timer,  TCP_SLOW_INTERVAL );
}

//
//
// tcp_fasttmr
//
// Is called every 100 ms and sends delayed ACKs
//

void tcp_fasttmr(void *arg)
{
  struct tcp_pcb *pcb;

  lock_semaphore(tcp_time_sem_lock);
  fast_tcp_ticks++;

  // Send delayed ACKs
  for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
  {
    if (pcb->flags & TF_ACK_DELAY) 
    {
     // kprintf("tcp_fasttmr: delayed ACK\n");
      pcb->flags |= TF_ACK_NOW;
      tcp_output(pcb);
      pcb->flags &= ~(TF_ACK_DELAY | TF_ACK_NOW);
    }
  }
  unlock_semaphore(tcp_time_sem_lock);
}

//
//
// tcp_fast_handler
//

void tcp_fast_handler(void *arg)
{
  queue_task( &tcp_fast_task, tcp_fasttmr, NULL);
  mod_timer(&tcpfast_timer,  TCP_FAST_INTERVAL );
}

static u8_t tcp_timer;
/**
 * Called periodically to dispatch TCP timers.
 *
 */
void tcp_tmr(void)
{
  /* Call tcp_fasttmr() every 250 ms */
  tcp_fasttmr(NULL);

  if (++tcp_timer & 1) {
    /* Call tcp_tmr() every 500 ms, i.e., every other timer
       tcp_tmr() is called. */
    tcp_slowtmr(NULL);
  }
}

//
// tcp_segs_free
//
// Deallocates a list of TCP segments (tcp_seg structures).
//
//

int tcp_segs_free(struct tcp_seg *seg)
{
  int count = 0;
  struct tcp_seg *next;

  while (seg != NULL) 
  {
    next = seg->next;
    count += tcp_seg_free(seg);
    seg = next;
  }

  return count;
}

//
// tcp_seg_free
//
// Frees a TCP segment.
//

int tcp_seg_free(struct tcp_seg *seg)
{
  int count = 0;
  
  if (seg != NULL) 
  {
    if (seg->p == NULL)
      kfree(seg);
    else 
    {
      count = pbuf_free(seg->p);
      kfree(seg);
    }
  }

  return count;
}

//
// tcp_seg_copy
//
// Returns a copy of the given TCP segment.
//

struct tcp_seg *tcp_seg_copy(struct tcp_seg *seg)
{
  struct tcp_seg *cseg;

  cseg = (struct tcp_seg *) kmalloc(sizeof(struct tcp_seg),0);
  if (cseg == NULL) return NULL;

  memcpy(cseg, seg, sizeof(struct tcp_seg));
  pbuf_ref(cseg->p);
  return cseg;
}

//
// tcp_new
//
// Creates a new TCP protocol control block but doesn't place it on
// any of the TCP PCB lists.
//

struct tcp_pcb *tcp_new()
{
  struct tcp_pcb *pcb;
  unsigned long iss;
  
  pcb = (struct tcp_pcb *) kmalloc(sizeof(struct tcp_pcb),0);
  if (pcb == NULL) return NULL;

  memset(pcb, 0, sizeof(struct tcp_pcb));
  pcb->snd_buf = TCP_SND_BUF;
  pcb->snd_queuelen = 0;
  pcb->rcv_wnd = TCP_WND;
  pcb->mss = TCP_MSS;
  pcb->rto = 3000 / TCP_SLOW_INTERVAL;
  pcb->sa = 0;
  pcb->sv = 3000 / TCP_SLOW_INTERVAL;
  pcb->rtime = 0;
  pcb->cwnd = 1;
  iss = tcp_next_iss();
  pcb->snd_wl2 = iss;
  pcb->snd_nxt = iss;
  pcb->snd_max = iss;
  pcb->lastack = iss;
  pcb->snd_lbb = iss;   
  pcb->tmr = tcp_ticks;

  pcb->polltmr = 0;
  /* Init KEEPALIVE timer */
  pcb->keepalive = TCP_KEEPDEFAULT;
  pcb->keep_cnt = 0;
  pcb->so_options=0;

  return pcb;
}

//
// tcp_init
//
// Initializes the TCP layer
//
static struct proc_entry e_tcpstat_proc = {
	name: "tcpstat",
	write_func: tcpstat_proc,
	read_func: NULL,
};
#define TCP_BASE_PORT 4096
void tcp_init()
{
  /* Clear globals. */
  tcp_listen_pcbs = NULL;
  tcp_active_pcbs = NULL;
  tcp_tw_pcbs = NULL;
  //tcp_tmp_pcb = NULL;
  
  /* initialize timer */
  tcp_ticks = 0;
  tcp_timer = 0;
  fast_tcp_ticks = 0;
  tcp_time_sem_lock = create_semaphore("tcptimer",0,1);
  //tcp_timer = 0;

  // Initialize timer
  iss = get_unixtime(0) + 6510;
  tcp_next_port = (unsigned short) (TCP_BASE_PORT + (get_unixtime(0) % 1024));
  tcp_ticks = 0;
  /*init_task(&tcp_slow_task);
  init_task(&tcp_fast_task);
  init_timer(&tcpslow_timer, tcp_slow_handler, NULL);
  init_timer(&tcpfast_timer, tcp_fast_handler, NULL);
  mod_timer(&tcpslow_timer,  TCP_SLOW_INTERVAL );
  mod_timer(&tcpfast_timer, TCP_FAST_INTERVAL );*/



  register_proc_entry(&e_tcpstat_proc);
}

//
// tcp_shutdown
//
// Shutdown TCP layer by resetting all active connections
//

void tcp_shutdown()
{
  struct tcp_pcb *pcb;

  // Send RST for all active connections  
  for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next)
  {
    tcp_rst(pcb->snd_nxt, pcb->rcv_nxt, 
            &pcb->local_ip, &pcb->remote_ip, 
	    pcb->local_port, pcb->remote_port);
  }
}

//
// tcp_arg
//
// Used to specify the argument that should be passed callback
// functions.
//

void tcp_arg(struct tcp_pcb *pcb, void *arg)
{  
  pcb->callback_arg = arg;
}

//
// tcp_recv
//
// Used to specify the function that should be called when a TCP
// connection receives data.
//

void tcp_recv(struct tcp_pcb *pcb, err_t (*recv)(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err))
{
  pcb->recv = recv;
}

//
// tcp_sent
//
// Used to specify the function that should be called when TCP data
// has been successfully delivered to the remote host.
//

void tcp_sent(struct tcp_pcb *pcb, err_t (*sent)(void *arg, struct tcp_pcb *tpcb, unsigned short len))
{
  pcb->sent = sent;
}

//
// tcp_err():
//
// Used to specify the function that should be called when a fatal error
// has occured on the connection.
//

void tcp_err(struct tcp_pcb *pcb, void (*errf)(void *arg, err_t err))
{
  pcb->errf = errf;
}

//
// tcp_poll
//
// Used to specify the function that should be called periodically
// from TCP. The interval is specified in terms of the TCP coarse
// timer interval, which is called twice a second.
//

void tcp_poll(struct tcp_pcb *pcb, err_t (*poll)(void *arg, struct tcp_pcb *tpcb), int interval)
{
  pcb->poll = poll;
  pcb->pollinterval = interval;
}

//
// tcp_accept
//
// Used for specifying the function that should be called when a
// LISTENing connection has been connected to another host.
//

void tcp_accept(struct tcp_pcb *pcb, err_t (*accept)(void *arg, struct tcp_pcb *newpcb, err_t err))
{
  pcb->accept = accept;
}

//
// tcp_pcb_purge
//
// Purges a TCP PCB. Removes any buffered data and frees the buffer memory.
//

void tcp_pcb_purge(struct tcp_pcb *pcb)
{
  if (pcb->state != CLOSED && pcb->state != TIME_WAIT && pcb->state != LISTEN) 
  {
    tcp_segs_free(pcb->unsent);
    tcp_segs_free(pcb->ooseq);
    tcp_segs_free(pcb->unacked);
    pcb->unacked = pcb->unsent = pcb->ooseq = NULL;
  }
}

//
// tcp_pcb_remove
//
// Purges the PCB and removes it from a PCB list. Any delayed ACKs are sent first.
//

void tcp_pcb_remove(struct tcp_pcb **pcblist, struct tcp_pcb *pcb)
{
  TCP_RMV(pcblist, pcb);

  tcp_pcb_purge(pcb);
  
  // If there is an outstanding delayed ACKs, send it
  if (pcb->state != TIME_WAIT && pcb->state != LISTEN && pcb->flags & TF_ACK_DELAY) 
  {
    pcb->flags |= TF_ACK_NOW;
    tcp_output(pcb);
  }  

  pcb->state = CLOSED;
}

//
// tcp_next_iss
//
// Calculates a new initial sequence number for new connections.
//

unsigned long tcp_next_iss()
{
  iss += tcp_ticks;
  return iss;
}

void tcp_debug_print(struct tcp_hdr *tcphdr)
{
  kprintf("+-------------------------------+\n");
  kprintf("|      %4d     |      %4d     | (src port, dest port)\n", ntohs(tcphdr->src), ntohs(tcphdr->dest));
  kprintf("+-------------------------------+\n");
  kprintf("|          %10lu           | (seq no)\n", ntohl(tcphdr->seqno));
  kprintf("+-------------------------------+\n");
  kprintf("|          %10lu           | (ack no)\n", ntohl(tcphdr->ackno));
  kprintf("+-------------------------------+\n");
  kprintf("| %2d |    |%d%d%d%d%d|    %5d      | (offset, flags (",
	  ntohs(tcphdr->_offset_flags) >> 4 & 1,
	  ntohs(tcphdr->_offset_flags) >> 4 & 1,
	  ntohs(tcphdr->_offset_flags) >> 3 & 1,
	  ntohs(tcphdr->_offset_flags) >> 2 & 1,
	  ntohs(tcphdr->_offset_flags) >> 1 & 1,
	  ntohs(tcphdr->_offset_flags) & 1,
	  ntohs(tcphdr->wnd));
  tcp_debug_print_flags(ntohs(tcphdr->_offset_flags));
  kprintf("), win)\n");
  kprintf("+-------------------------------+\n");
  kprintf("|    0x%04x     |     %5d     | (chksum, urgp)\n", ntohs(tcphdr->chksum), ntohs(tcphdr->urgp));
  kprintf("+-------------------------------+\n");
}

void tcp_debug_print_state(enum tcp_state s)
{
  kprintf("State: ");
  switch (s) 
  {
    case CLOSED:      kprintf("CLOSED\n"); break;
    case LISTEN:      kprintf("LISTEN\n"); break;
    case SYN_SENT:    kprintf("SYN_SENT\n"); break;
    case SYN_RCVD:    kprintf("SYN_RCVD\n"); break;
    case ESTABLISHED: kprintf("ESTABLISHED\n"); break;
    case FIN_WAIT_1:  kprintf("FIN_WAIT_1\n"); break;
    case FIN_WAIT_2:  kprintf("FIN_WAIT_2\n"); break;
    case CLOSE_WAIT:  kprintf("CLOSE_WAIT\n"); break;
    case CLOSING:     kprintf("CLOSING\n"); break;
    case LAST_ACK:    kprintf("LAST_ACK\n"); break;
    case TIME_WAIT:   kprintf("TIME_WAIT\n"); break;
  }
}

void tcp_debug_print_flags(int flags)
{
  if (flags & TCP_FIN) kprintf("FIN ");
  if (flags & TCP_SYN) kprintf("SYN ");
  if (flags & TCP_RST) kprintf("RST ");
  if (flags & TCP_PSH) kprintf("PSH ");
  if (flags & TCP_ACK) kprintf("ACK ");
  if (flags & TCP_URG) kprintf("URG ");
}

void tcp_debug_print_pcbs()
{
  struct tcp_pcb *pcb;

  kprintf("Active PCB states:\n");
  for (pcb = tcp_active_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    kprintf("Local port %d, remote port %d snd_nxt %lu rcv_nxt %lu ",
	    pcb->local_port, pcb->remote_port,
	    pcb->snd_nxt, pcb->rcv_nxt);
    tcp_debug_print_state(pcb->state);
  }

  kprintf("Listen PCB states:\n");
  for (pcb = (struct tcp_pcb *) tcp_listen_pcbs; pcb != NULL; pcb = pcb->next)
  {
    kprintf("Local port %d, remote port %d snd_nxt %lu rcv_nxt %lu ",
            pcb->local_port, pcb->remote_port,
            pcb->snd_nxt, pcb->rcv_nxt);
    tcp_debug_print_state(pcb->state);
  }    

  kprintf("TIME-WAIT PCB states:\n");
  for (pcb = tcp_tw_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    kprintf("Local port %d, remote port %d snd_nxt %lu rcv_nxt %lu ",
            pcb->local_port, pcb->remote_port,
            pcb->snd_nxt, pcb->rcv_nxt);
    tcp_debug_print_state(pcb->state);
  }    
}
