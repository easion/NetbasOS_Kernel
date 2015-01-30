

#include <net/net.h>

static struct raw_pcb *raw_pcbs = NULL;

//
// rawstat_proc
//

static int rawstat_proc(void *arg, int len,struct proc_entry *pf)
{
  struct raw_pcb *pcb;

  pprintf(pf, "protocol    local ip        remote ip\n");
  pprintf(pf, "----------- --------------- ---------------\n");

  for (pcb = raw_pcbs; pcb != NULL; pcb = pcb->next) 
  {
    pprintf(pf, "%8d    %-15s %-15s\n", 
		pcb->protocol, inetntoa(pcb->local_ip.addr), inetntoa(pcb->remote_ip.addr));
  }

  return 0;
}

//
// raw_init
//

static struct proc_entry e_rawstat_proc = {
	name: "rawstat",
	write_func: rawstat_proc,
	read_func: NULL,
};

void raw_init()
{
  raw_pcbs = NULL;
  register_proc_entry(&e_rawstat_proc);
}

//
// Determine if in incoming IP packet is covered by a RAW PCB
// and if so, pass it to a user-provided receive callback function.
//
// Given an incoming IP datagram (as a chain of pbufs) this function
// finds a corresponding RAW PCB and calls the corresponding receive
// callback function.
//

err_t raw_input(struct pbuf *p, struct netif *inp)
{
  struct raw_pcb *pcb;
  struct ip_hdr *iphdr;
  int proto;
  int rc;
  int eaten = 0;

  iphdr = p->payload;
  proto = IPH_PROTO(iphdr);

  // Loop through all raw pcbs until the packet is eaten by one
  // This allows multiple pcbs to match against the packet by design
  pcb = raw_pcbs;
  while (!eaten && pcb != NULL) 
  {
    if (pcb->protocol == proto) 
    {
      // Receive callback function available?
      if (pcb->recv != NULL) 
      {
        // The receive callback function did not eat the packet?
	rc = pcb->recv(pcb->recv_arg, pcb, p, &iphdr->src);
	if (rc < 0) return rc;
        if (rc > 0)
        {
          // Receive function ate the packet
          stats.raw.recv++;
          p = NULL;
          eaten = 1;
        }
      }
    }
    pcb = pcb->next;
  }

  return eaten;
}

//
// Bind a RAW PCB.
//

err_t raw_bind(struct raw_pcb *pcb, struct ip_addr *ipaddr)
{
  ip_addr_set(&pcb->local_ip, ipaddr);
  return 0;
}

//
// Connect an RAW PCB.  This will associate the RAW PCB with the remote address.
//

err_t raw_connect(struct raw_pcb *pcb, struct ip_addr *ipaddr)
{
  ip_addr_set(&pcb->remote_ip, ipaddr);
  return 0;
}

//
// Set the callback function for received packets that match the
// raw PCB's protocol and binding. 
// 
// The callback function MUST either
// - eat the packet by calling pbuf_free() and returning non-zero. The
//   packet will not be passed to other raw PCBs or other protocol layers.
// - not free the packet, and return zero. The packet will be matched
//   against further PCBs and/or forwarded to another protocol layers.
//

err_t raw_recv(struct raw_pcb *pcb, int (*recv)(void *arg, struct raw_pcb *upcb, struct pbuf *p, struct ip_addr *addr), void *recv_arg)
{
  // Remember recv() callback and user data
  pcb->recv = recv;
  pcb->recv_arg = recv_arg;
  return 0;
}

//
// Send the raw IP packet to the given address. Note that actually you cannot
// modify the IP headers (this is inconsistent with the receive callback where
// you actually get the IP headers), you can only specify the IP payload here.
//

err_t raw_sendto(struct raw_pcb *pcb, struct pbuf *p, struct ip_addr *ipaddr)
{
  struct netif *netif;
  struct ip_addr *src_ip;

  if ((netif = ip_route(ipaddr)) == NULL) 
  {
    stats.raw.rterr++;
    return EHOSTUNREACH;
  }

  if (ip_addr_isany(&pcb->local_ip))
  {
    // Use outgoing network interface IP address as source address
    src_ip = &netif->ipaddr;
  } 
  else 
  {
    // use RAW PCB local IP address as source address
    src_ip = &pcb->local_ip;
  }

  stats.raw.xmit++;
  return ip_output_if(p, src_ip, ipaddr, pcb->ttl, pcb->protocol, netif);
}

//
// Send the raw IP packet to the address given by raw_connect()
//

err_t raw_send(struct raw_pcb *pcb, struct pbuf *p)
{
  return raw_sendto(pcb, p, &pcb->remote_ip);
}

//
// Remove an RAW PCB.
//

void raw_remove(struct raw_pcb *pcb)
{
  struct raw_pcb *pcb2;
  
  if (raw_pcbs == pcb) 
  {
    raw_pcbs = raw_pcbs->next;
  } 
  else 
  {
    for (pcb2 = raw_pcbs; pcb2 != NULL; pcb2 = pcb2->next) 
    {
      if (pcb2->next != NULL && pcb2->next == pcb) 
      {
        pcb2->next = pcb->next;
      }
    }
  }
    
  kfree(pcb);
}

//
// Create a RAW PCB.
//

struct raw_pcb *raw_new(unsigned short proto)
{
  struct raw_pcb *pcb;

  pcb = (struct raw_pcb *) kmalloc(sizeof(struct raw_pcb),0);
  if (!pcb) return NULL;

  // Initialize PCB to all zeroes
  memset(pcb, 0, sizeof(struct raw_pcb));
  pcb->protocol = proto;
  pcb->ttl = RAW_TTL;
  pcb->next = raw_pcbs;
  raw_pcbs = pcb;

  return pcb;
}
