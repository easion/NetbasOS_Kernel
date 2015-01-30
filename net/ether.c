

#include <net/net.h>

void tcp_tmr(void);

static const struct eth_addr ethbroadcast = {{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}};
//static struct queue *ether_queue;
static thread_t* eth_thread=NULL ;

struct ether_msg
{
  struct pbuf *p;
  struct netif *netif;
};


static struct netdev_link *netdev_list=NULL;


int ether_interface_register(struct netdev_link *netdev)
{
	//kprintf("ether_interface_register:%s\n", netdev->dev_name);
	if (netdev_list==NULL)
	{
		netdev_list = netdev;
	}
	else{
		netdev_list->next_link = netdev;
	}

	netdev->next_link = NULL;
	netdev->flags = IF_STOP;
	return 0;
}

int ether_interface_unregister(struct netdev_link *remove_dev)
{
	struct netdev_link *netdev = netdev_list;

	if (netdev_list == remove_dev)
	{
		netdev_list = NULL;
		return 1;
	}

	while (netdev)
	{
		if (netdev->next_link == remove_dev)
		{
			netdev->next_link = remove_dev->next_link;
			remove_dev->flags = IF_STOP;
			return 1;
		}
		netdev = netdev->next_link;
	}

	return 0;
}



int register_ether_netifs(void)
{
  dev_t devno;
  dev_prvi_t *devfp;
  int rc;
  struct ip_addr noaddr;
  struct netif *netif;
  char dev_name[32];
  int count=0;
  struct netdev_link *net_list=netdev_list;

  noaddr.addr = IP_ADDR_ANY;

  while (net_list!=NULL)
  {
	  if (net_list->flags == IF_RUNNING)
	  {
		  net_list = net_list->next_link;
		  continue;
	  }

	  devfp = kmalloc(sizeof(dev_prvi_t),0);
	  sprintf(dev_name,"/dev/%s", net_list->dev_name);
	  devno = dev_open(dev_name,2,devfp);


	  if (devno<0)
	  {
		   kprintf("dev_name=%s open failed\n",dev_name);
		  return 0;
	  }
    /*dev = device(devno);
    if (!dev) continue;
    if (!dev->driver) continue;
    if (dev->driver->type != DEV_TYPE_PACKET) continue;*/

	 // kprintf("dev_name=%s open ok\n",dev_name);
    netif = netif_add(net_list->dev_name, &noaddr, &noaddr, &noaddr);
    if (!netif) return ENOMEM;
	 // kprintf("netif_add %s\n",dev_name);

    netif->output = ether_output;
    netif->state = (void *) devfp;
	 // kprintf("netif_add %s ok\n",dev_name);

    rc = dev_attach(devfp, netif,&netif->hwaddr, ether_input);
	 // kprintf("dev_attach netif->hwaddr ok\n");
    if (rc < 0)
		kprintf(KERN_ERR "ether: unable to attach to device %s (error %d)\n",
		net_list->dev_name, rc);
	  //kprintf("netif_add %s dev_attach\n",dev_name);

	net_list->flags = IF_RUNNING;
	net_list = net_list->next_link;
	count++;
  }

  if (count==0)
  {
	  //kprintf("no ether interface found\n");
  }


  return count;
}


//
// ether2str
//
// This function converts an ethernet address to string format.
//

char *ether2str(struct eth_addr *hwaddr, char *s)
{
  sprintf(s, "%02x:%02x:%02x:%02x:%02x:%02x",  
          hwaddr->addr[0], hwaddr->addr[1], hwaddr->addr[2], 
	  hwaddr->addr[3], hwaddr->addr[4], hwaddr->addr[5]);
  return s;
}

//
// ether_crc
//

#define ETHERNET_POLYNOMIAL 0x04c11db7U

unsigned long ether_crc(int length, unsigned char *data)
{
  int crc = -1;

  while (--length >= 0) 
  {
    unsigned char current_octet = *data++;
    int bit;
    for (bit = 0; bit < 8; bit++, current_octet >>= 1)
    {
      crc = (crc << 1) ^ ((crc < 0) ^ (current_octet & 1) ? ETHERNET_POLYNOMIAL : 0);
    }
  }

  return crc;
}

//
// ether_output
//
// This function is called by the TCP/IP stack when an IP packet
// should be sent.
//

err_t ether_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr)
{
  struct pbuf *q;
  struct eth_hdr *ethhdr;
  struct eth_addr *dest, mcastaddr;
  struct ip_addr *queryaddr;
  err_t err;
  int i;
  int loopback = 0;

  //kprintf("ether: xmit %d bytes, %d bufs\n", p->tot_len, pbuf_clen(p));

  if ((netif->flags & NETIF_UP) == 0) return ENETDOWN;

  if (pbuf_header(p, ETHER_HLEN))
  {
    kprintf(KERN_ERR "ether_output: not enough room for Ethernet header in pbuf\n");
    stats.link.err++;
    return ENOSPC;
  }

  // Construct Ethernet header. Start with looking up deciding which
  // MAC address to use as a destination address. Broadcasts and
  // multicasts are special, all other addresses are looked up in the
  // ARP table.

  queryaddr = ipaddr;
  if (ip_addr_isany(ipaddr) || ip_addr_isbroadcast(ipaddr, &netif->netmask)) 
    dest = (struct eth_addr *) &ethbroadcast;
  else if (ip_addr_ismulticast(ipaddr)) 
  {
    // Hash IP multicast address to MAC address.
    mcastaddr.addr[0] = 0x01;
    mcastaddr.addr[1] = 0x0;
    mcastaddr.addr[2] = 0x5e;
    mcastaddr.addr[3] = ip4_addr2(ipaddr) & 0x7f;
    mcastaddr.addr[4] = ip4_addr3(ipaddr);
    mcastaddr.addr[5] = ip4_addr4(ipaddr);
    dest = &mcastaddr;
  }
  else if (ip_addr_cmp(ipaddr, &netif->ipaddr))
  {
    dest = &netif->hwaddr;
    loopback = 1;
  }
  else 
  {
    if (ip_addr_maskcmp(ipaddr, &netif->ipaddr, &netif->netmask))
    {
      // Use destination IP address if the destination is on the same subnet as we are.
      queryaddr = ipaddr;
    }
    else
    {
      // Otherwise we use the default router as the address to send the Ethernet frame to.
      queryaddr = &netif->gw;
    }

    dest = arp_lookup(queryaddr);
  }

  // If the arp_lookup() didn't find an address, we send out an ARP query for the IP address.
  if (dest == NULL) 
  {
    q = arp_query(netif, &netif->hwaddr, queryaddr);
    if (q != NULL) 
    {
      err = dev_transmit((dev_prvi_t*) netif->state, q);
      if (err < 0)
      {
        kprintf(KERN_ERR "ether: error %d sending arp packet\n", err);
        pbuf_free(q);
        stats.link.drop++;
	return err;
      }
    }

    // Queue packet for transmission, when the ARP reply returns
    err = arp_queue(netif, p, queryaddr);
    if (err < 0)
    {
      kprintf(KERN_ERR "ether: error %d queueing packet\n", err);
      stats.link.drop++;
      stats.link.memerr++;
      return err;
    }

    return 0;
  }

  ethhdr = p->payload;

  for (i = 0; i < 6; i++)
  {
    ethhdr->dest.addr[i] = dest->addr[i];
    ethhdr->src.addr[i] = netif->hwaddr.addr[i];
  }
  ethhdr->type = htons(ETHTYPE_IP);
  
  if (loopback)
  {
    struct pbuf *q;


    q = pbuf_dup(PBUF_RAW, p);
    if (!q) return ENOMEM;

    err = ether_input(netif, q);
	//err = loopif_input(netif, q);
    if (err < 0)
    {
      pbuf_free(q);
      return err;
    }
  }
  else
  {
    err = dev_transmit((dev_prvi_t*) netif->state, p);
    if (err < 0)
    {
      kprintf(KERN_ERR "ether: error %d sending packet\n", err);
      return err;
    }
  }

  return 0;
}

//
// ether_input
//
// This function should be called when a packet is received
// from the interface. 
//

static err_t ether_wakeup()
{
	int rc;
  struct ether_msg msg;

  msg.p = NULL;
  msg.netif = NULL;

  rc = post_thread_message(eth_thread, &msg, sizeof(msg));
  if (rc!=sizeof(msg))
  {
	kprintf("ether: post_thread_message (%d)\n",rc);
	return 1;
  }

  return 0;
}

err_t ether_input(struct netif *netif, struct pbuf *p)
{
	int rc;
  struct ether_msg msg;

  if ((netif->flags & NETIF_UP) == 0)
	  return ENETDOWN;

   if(!eth_thread)
	   return  -EINVAL;

  if (p->len < ETHER_HLEN)
  {
    kprintf("ether: Packet dropped due to too short packet %d %s\n",
		p->len, netif->name);
    stats.link.lenerr++;
    stats.link.drop++;
    return EINVAL;
  }

  //msg = (struct ether_msg *) kmalloc(sizeof(struct ether_msg),0);
  //if (!msg) return ENOMEM;

  msg.p = p;
  msg.netif = netif;

  rc = post_thread_message(eth_thread, &msg, sizeof(msg));
  if (rc!=sizeof(msg))
  {
	kprintf("ether: post_thread_message (%d)\n",rc);
	return 1;
  }

  return 0;
}

int dispatch_packet(struct netif *netif, struct pbuf *p)
{
	struct eth_hdr *ethhdr;

    if (p != NULL) 
    {
      ethhdr = p->payload;

      /*if (!eth_addr_isbroadcast(&ethhdr->dest)) 
	  kprintf("ether: recv src=%la dst=%la type=%04X len=%d\n", 
		  &ethhdr->src, &ethhdr->dest, htons(ethhdr->type), p->len);*/
      
      switch (htons(ethhdr->type))
      {
	case ETHTYPE_IP:
	  arp_ip_input(netif, p);
	  pbuf_header(p, -ETHER_HLEN);
	  if (netif->input(p, netif) < 0) pbuf_free(p);
	  break;

	case ETHTYPE_ARP:
	  p = arp_arp_input(netif, &netif->hwaddr, p);
	  if (p != NULL) 
	  {
		  if ((netif->flags & NETIF_NO_ARP)&&netif->linkoutput)
		  {
			  netif->linkoutput(netif,p);
		  }
		  else
	    if (dev_transmit((dev_prvi_t*) netif->state, p) < 0) pbuf_free(p);
	  }
	  break;

	default:
		//kprintf("ether_dispatcher() packet error\n");
	  pbuf_free(p);
	  break;
      }
    }

	return 0;
}

err_t loopif_input(struct netif *netif, struct pbuf *p)
{
	int rc;

  if ((netif->flags & NETIF_UP) == 0)
	  return ENETDOWN;

  

  if (p->len < ETHER_HLEN)
  {
    kprintf("ether: Packet dropped due to too short packet %d %s\n",
		p->len, netif->name);
    stats.link.lenerr++;
    stats.link.drop++;
    return EINVAL;
  }

  dispatch_packet(netif,p);

  

  return 0;
}



struct sys_timeo {
  struct sys_timeo *next;
  u32_t time;
  sys_timeout_handler h;
  void *arg;
};

struct sys_timeouts {
  struct sys_timeo *next;
};

struct sys_timeouts timeouts;
#define SYS_ARCH_TIMEOUT 0xffffffff

struct sys_timeouts *
sys_arch_timeouts(void)
{
	return &timeouts;
}


int
ether_msg_fetch(struct ether_msg *msg, int size)
{
  int time;
  struct sys_timeouts *timeouts;
  //struct sys_timeouts *nexttimer;
  struct sys_timeo *tmptimeout;
  sys_timeout_handler h;
  void *arg;
  int rc=-1;
  unsigned old_ticks=0;
	unsigned flags;

	memset(msg,0,size);

 again:
  timeouts = sys_arch_timeouts();

  if (!timeouts || !timeouts->next) {
	  old_ticks = startup_ticks();
	  
    rc = get_thread_message(eth_thread,msg,size,INFINITE);
	if (rc<=0)
		{
		kprintf("%s got line %d,%d\n",__FUNCTION__,__LINE__,rc);
		//应该有数据获得
		return -1;
			//time = SYS_ARCH_TIMEOUT;
		}
		else{
			time = startup_ticks() - old_ticks;
			if (time<=0)
			{
				time = 0;
			}
		}
  } else {
	  //nexttimer = timeouts->next;
    if (timeouts->next->time > 0) {
		old_ticks = startup_ticks();
      //time = sys_arch_mbox_fetch(mbox, msg, timeouts->next->time);
	  rc = get_thread_message(eth_thread,msg,size,timeouts->next->time);
	  if (rc<=0)
		{
			time = SYS_ARCH_TIMEOUT;
		}
		else{
			//取得了数据，除掉时差
			time = startup_ticks() - old_ticks;
			if (time<=0)
			{
				time = 0;
			}
		}

    } else {
      time = SYS_ARCH_TIMEOUT;
    }

	timeouts = sys_arch_timeouts();

    if ((time == SYS_ARCH_TIMEOUT || !msg->netif || !msg->p) && (timeouts) && timeouts->next) {
		save_eflags(&flags);

      /* If time == SYS_ARCH_TIMEOUT, a timeout occured before a message
   could be fetched. We should now call the timeout handler and
   deallocate the memory allocated for the timeout. */
      tmptimeout = timeouts->next;
      timeouts->next = tmptimeout->next;
      h = tmptimeout->h;
      arg = tmptimeout->arg;
	  restore_eflags(flags);
      kfree( tmptimeout);
      if (h != NULL) {
        //LWIP_DEBUGF(SYS_DEBUG, ("smf calling h=%p(%p)\n", (void *)h, (void *)arg));
      	h(arg);
      }

      /* We try again to fetch a message from the mbox. */
      goto again;
    } else {

		save_eflags(&flags);

		ASSERT(timeouts->next);

      /* If time != SYS_ARCH_TIMEOUT, a message was received before the timeout
   occured. The time variable is set to the number of
   milliseconds we waited for the message. */
      if (time <= timeouts->next->time) {
  timeouts->next->time -= time;
      } else {
  timeouts->next->time = 0;
      }

	  restore_eflags(flags);
    }

  }

  return rc;
}


void sys_timeout(u32_t msecs, sys_timeout_handler h, void *arg)
{
  struct sys_timeouts *timeouts;
  struct sys_timeo *timeout, *t;
  unsigned flags;

  timeout = kmalloc(sizeof(struct sys_timeo), 0);
  if (timeout == NULL) {
    return;
  }
  timeout->next = NULL;
  timeout->h = h;
  timeout->arg = arg;
  timeout->time = msecs;

	save_eflags(&flags);
  timeouts = sys_arch_timeouts();

  //LWIP_DEBUGF(SYS_DEBUG, ("sys_timeout: %p msecs=%"U32_F" h=%p arg=%p\n",
  //  (void *)timeout, msecs, (void *)h, (void *)arg));

  if (timeouts->next == NULL) {
    timeouts->next = timeout;
	restore_eflags(flags);
    return;
  }

  if (timeouts->next->time > msecs) {
    timeouts->next->time -= msecs;
    timeout->next = timeouts->next;
    timeouts->next = timeout;
  } else {
    for(t = timeouts->next; t != NULL; t = t->next) {
      timeout->time -= t->time;
      if (t->next == NULL || t->next->time > timeout->time) {
        if (t->next != NULL) {
          t->next->time -= timeout->time;
        }
        timeout->next = t->next;
        t->next = timeout;
        break;
      }
    }
  }
  restore_eflags(flags);

}

/* Go through timeout list (for this task only) and remove the first matching entry,
   even though the timeout has not triggered yet.
*/

void sys_untimeout(sys_timeout_handler h, void *arg)
{
    struct sys_timeouts *timeouts;
    struct sys_timeo *prev_t, *t;
	unsigned flags;

	save_eflags(&flags);

    timeouts = sys_arch_timeouts();

    if (timeouts->next == NULL){
		restore_eflags(flags);
        return;
	}


    for (t = timeouts->next, prev_t = NULL; t != NULL; prev_t = t, t = t->next)
    {
        if ((t->h == h) && (t->arg == arg))
        {
            /* We have a match */
            /* Unlink from previous in list */
            if (prev_t == NULL)
                timeouts->next = t->next;
            else
                prev_t->next = t->next;
            /* If not the last one, add time of this one back to next */
            if (t->next != NULL)
                t->next->time += t->time;
            kfree( t);
			restore_eflags(flags);
            return;
        }
    }

	restore_eflags(flags);
    return;
}

static volatile int tcpip_tcp_timer_active = 0;
static volatile int tcpip_fast_timer_active=0;

static void
tcpip_tcp_timer(void *arg)
{
  (void)arg;

  /* call TCP timer handler */
  tcp_tmr();
  /* timer still needed? */
  if (tcp_active_pcbs || tcp_tw_pcbs) {
    /* restart timer */
    sys_timeout(TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
  } else {
    /* disable timer */
    tcpip_tcp_timer_active = 0;
  }
}

void tcp_fasttmr(void *arg);


void tcpip_fast_timer(void *arg)
{
  if (tcp_active_pcbs || tcp_tw_pcbs) 
	tcp_fasttmr(arg);

	 tcpip_fast_timer_active = 0;
#if 0
	//sys_untimeout(tcpip_tcp_timer, NULL);
	//sys_timeout(TCP_FAST_TMR_INTERVAL, tcpip_tcp_timer, NULL);	
#endif
    /* restart timer */
   
}

void tcp_fasttimer_needed(void)
{
  /* timer is off but needed again? */
  //if(1)
  if (!tcpip_fast_timer_active && (tcp_active_pcbs || tcp_tw_pcbs)) 
  
  {
    /* enable and start timer */
    tcpip_fast_timer_active = 1;
	sys_timeout(0, tcpip_fast_timer, NULL);	
	ether_wakeup();
    //sys_timeout(TCP_TMR_INTERVAL, tcpip_fast_timer, NULL);
  }
  
}

void tcp_timer_needed(void)
{
  /* timer is off but needed again? */
  if (!tcpip_tcp_timer_active && (tcp_active_pcbs || tcp_tw_pcbs)) {
    /* enable and start timer */
    tcpip_tcp_timer_active = 1;
    sys_timeout(TCP_TMR_INTERVAL, tcpip_tcp_timer, NULL);
  }
}

//
// ether_dispatcher
//
// This task dispatches received packets from the network interfaces 
// to the TCP/IP stack. 
//

static void ether_dispatcher(void *arg)
{
  struct ether_msg msg;
  struct pbuf *p;
  struct netif *netif;
  
  int rc;

  eth_thread = current_thread();

  timeouts.next = NULL;

  while (1)
  {
	  rc = ether_msg_fetch(&msg,sizeof(msg));

		  //250毫秒运行一次
		//  tcp_tmr();	
	  //rc = get_thread_message(eth_thread,&msg,sizeof(msg),150);
	  if (rc<=0)
	  {


		  //kprintf("error retrieving message from ethernet packet queue\n");
		  continue;
	  }
	  
	  //kprintf("retrieving message from ethernet\n");

    p = msg.p;
    netif = msg.netif;

	if (!p || !netif)
	{
		continue;
	}

	dispatch_packet(netif,p);



    //yield();
  }
}

//
// ether_init
//
// Initializes the ethernet packet dispatcher
//

void ether_init()
{
  //struct thread *ethertask;

  netdev_list = NULL;

  new_kernel_thread("ethernet", ether_dispatcher,NULL );
  //ethertask = create_kernel_thread(ether_dispatcher, NULL, 
	//  /*PRIORITY_ABOVE_NORMAL*/ PRIORITY_NORMAL, "ethertask");
}
