 

#include <net/net.h>
#include <jicama/msgport.h>

struct netif *netif_list = NULL;
struct netif *netif_default = NULL;

static int netif_proc(void *arg, int len,struct proc_entry *pf)
{
  struct netif *netif;
  
  for (netif = netif_list; netif != NULL; netif = netif->next) 
  {
    pprintf(pf, "%s: addr %s mask %s gw %s\n", 
		netif->name,
		inetntoa(netif->ipaddr.addr),
		inetntoa(netif->netmask.addr),
		inetntoa(netif->gw.addr));
  }

  return 0;
}

static struct proc_entry e_netif_proc = {
	name: "netif",
	write_func: netif_proc,
	read_func: NULL,
};


void netif_init()
{
	netif_list = netif_default = NULL;
	register_proc_entry(&e_netif_proc);
}

void netif_delete(struct netif *netif)
{
  struct netif *prev;
	if (netif == netif_list)
	{
		netif_list = netif->next;
	}
	else{
		//fixme
		prev = netif_list;
		while (prev && prev->next!=netif)
		{
			prev = prev->next;
		}

		if (prev)
		{
			prev->next = netif->next;
		}
	}

	kfree(netif);
}

struct netif *netif_add(char *name, struct ip_addr *ipaddr, struct ip_addr *netmask, struct ip_addr *gw)
{
  struct netif *netif;
  
  netif = kmalloc(sizeof(struct netif),0);
  if (!netif) return NULL;
  memset(netif, 0, sizeof(struct netif));

  strcpy(netif->name, name);
  netif->input = ip_input;
  netif->output = NULL;
  netif->flags = NETIF_ALLMULTI;
  netif->mtu_size = TCP_MSS;

  ip_addr_set(&netif->ipaddr, ipaddr);
  ip_addr_set(&netif->netmask, netmask);
  ip_addr_set(&netif->gw, gw);
  netif->broadcast.addr = (ipaddr->addr & netmask->addr) | ~netmask->addr;

  netif->mclist = NULL;
  netif->mccount = 0;

  netif->next = netif_list;
  netif_list = netif;

  return netif;
}

struct netif *netif_find(char *name)
{
  struct netif *netif;
  
  if (!name) return NULL;

  for (netif = netif_list; netif != NULL; netif = netif->next) 
  {
    if (strcmp(name, netif->name) == 0) return netif;
  }

  return NULL;
}

void netif_set_ipaddr(struct netif *netif, struct ip_addr *ipaddr)
{
  ip_addr_set(&netif->ipaddr, ipaddr);
}

void netif_set_gw(struct netif *netif, struct ip_addr *gw)
{
  ip_addr_set(&netif->gw, gw);
}

void netif_set_netmask(struct netif *netif, struct ip_addr *netmask)
{
  ip_addr_set(&netif->netmask, netmask);
}

void netif_set_default(struct netif *netif)
{
  netif_default = netif;
}

int netif_ioctl_list(void *data, size_t size)
{
  int numifs;
  struct netif *netif;
  struct ifcfg *ifcfg;
  struct sockaddr_in *sin;

  if (!data) return EFAULT;

  // Find number of network interfaces
  numifs = 0;
  netif = netif_list;
  while (netif)
  {
    numifs++;
    netif = netif->next;
  }

  // Fill interface info into buffer
 // if (size >= (size_t) (numifs * sizeof(struct ifcfg)))
  {
    netif = netif_list;
    ifcfg = (struct ifcfg *) data;
    while (netif)
    {
      memset(ifcfg, 0, sizeof(struct ifcfg));

      strcpy(ifcfg->name, netif->name);

	 // kprintf("ifcfg->name=%s\n", ifcfg->name);

      sin = (struct sockaddr_in *) &ifcfg->addr;
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = netif->ipaddr.addr;

      sin = (struct sockaddr_in *) &ifcfg->gw;
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = netif->gw.addr;

      sin = (struct sockaddr_in *) &ifcfg->netmask;
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = netif->netmask.addr;

      sin = (struct sockaddr_in *) &ifcfg->broadcast;
      sin->sin_family = AF_INET;
      sin->sin_addr.s_addr = netif->broadcast.addr;

      memcpy(ifcfg->hwaddr, &netif->hwaddr, sizeof(struct eth_addr));

      if (netif->flags & NETIF_UP) ifcfg->flags |= IFCFG_UP;
      if (netif->flags & NETIF_DHCP) ifcfg->flags |= IFCFG_DHCP;
      if (netif->flags & NETIF_LOOPBACK) ifcfg->flags |= IFCFG_LOOPBACK;
      if (netif == netif_default) ifcfg->flags |= IFCFG_DEFAULT;

      netif = netif->next;
      ifcfg++;
    }
  }

  return numifs * sizeof(struct ifcfg);
}

int netif_ioctl_cfg(void *data, size_t size)
{
  struct netif *netif;
  struct ifcfg *ifcfg;
  struct dhcp_state *state;
  int count;

  if (!data) return EFAULT;
  ifcfg = (struct ifcfg *) data;

  if (size != sizeof(struct ifcfg)) {
 // kprintf("netif_ioctl_cfg find %s %d\n",ifcfg->name,size);
	 // return EINVAL;
  }

  count = register_ether_netifs();

  if (!count)
  {
	 // return 0;
  }

	//kprintf("netif_ioctl_cfg %x, %x\n",ifcfg->flags & IFCFG_DHCP, netif->flags & NETIF_UP);

  netif = netif_find(ifcfg->name);
  if (!netif){
	  kprintf("netif_ioctl_cfg interface %s not found\n",ifcfg->name);
	  return ENXIO;
  }


  // Check for interface down
  if ((ifcfg->flags & IFCFG_UP) == 0 && (netif->flags & NETIF_UP) == 1)
  {
	  //kprintf("go ifdown\n");
    // Release DHCP lease
    dhcp_stop(netif);
    netif->flags &= ~NETIF_UP;
	//return 0;
  }

  // Update network interface configuration
  if (ifcfg->flags & IFCFG_DHCP){
    netif->flags |= NETIF_DHCP;
  //kprintf("netif_ioctl_cfg use dhcp %s\n",ifcfg->name);
  }
  else{
	netif->flags &= ~NETIF_DHCP;

	netif->ipaddr.addr = ((struct sockaddr_in *) &ifcfg->addr)->sin_addr.s_addr;
	netif->netmask.addr = ((struct sockaddr_in *) &ifcfg->netmask)->sin_addr.s_addr;
	netif->gw.addr = ((struct sockaddr_in *) &ifcfg->gw)->sin_addr.s_addr;
	netif->broadcast.addr = ((struct sockaddr_in *) &ifcfg->broadcast)->sin_addr.s_addr;
  }

  if (netif->broadcast.addr == IP_ADDR_ANY)
  {
    netif->broadcast.addr = (netif->ipaddr.addr & netif->netmask.addr) | ~(netif->netmask.addr);
  }

  if (ifcfg->flags & IFCFG_DEFAULT)
    netif_default = netif;
  else if (netif == netif_default)
    netif_default = NULL;

  // Copy hwaddr into ifcfg as info
  memcpy(ifcfg->hwaddr, &netif->hwaddr, sizeof(struct eth_addr));

  // Check for interface up
  if ((ifcfg->flags & IFCFG_UP) == 1 && (netif->flags & NETIF_UP) == 0)
  {
    netif->flags |= NETIF_UP;

    if (netif->flags & NETIF_DHCP)
    {


      // Obtain network parameters using DHCP
      state = dhcp_start(netif);

      if (state) 
      {
		int error = lock_semaphore_timeout(state->binding_complete, 30000) ;
		if (error<0)
		{
			kprintf("%s() timeout\n",__FUNCTION__);
			return ETIMEDOUT;
		}

		//kprintf("binding_complete dhcp succ\n");
		((struct sockaddr_in *) &ifcfg->addr)->sin_addr.s_addr = netif->ipaddr.addr;
		((struct sockaddr_in *) &ifcfg->netmask)->sin_addr.s_addr = netif->netmask.addr;
		((struct sockaddr_in *) &ifcfg->gw)->sin_addr.s_addr = netif->gw.addr;
		((struct sockaddr_in *) &ifcfg->broadcast)->sin_addr.s_addr = netif->broadcast.addr;
      }
    }
  }
  else{
	  //kprintf("go static ...\n");
  }

  destroy_semaphore(state->binding_complete);

  //kprintf("netif_ioctl_cfg dhcp for %s done\n",ifcfg->name);
  return 0;
}
