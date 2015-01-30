/*
 *  Tun/Tap Driver for Netbas OS Kernel
 *  Copyright (C) 2009  Easion(easion@gmail.com)
 
Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met: 
1. Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in the 
   documentation and/or other materials provided with the distribution. 
3. Neither the name of Axon Digital Design nor the names of its contributors 
   may be used to endorse or promote products derived from this software 
   without specific prior written permission. 

THIS SOFTWARE IS PROVIDED BY AXON DIGITAL DESIGN AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE 
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY 
OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE. 
*/
#include <drv/defs.h>
#include <drv/drv.h>
#include <drv/sym.h>
#include <drv/pci.h>
#include <drv/ia.h>
#include <drv/errno.h>
#include <drv/spin.h>
#include <drv/log.h>
#include <drv/pbuf.h>
#include <assert.h>

#include <net/ipaddr.h>
#include <net/stats.h>
#include <net/opt.h>
#include <net/pbuf.h>
#include <net/ether.h>
#include <net/inet.h>
#include <net/netif.h>
#include "tunif.h"
#include <drv/fs.h>

#define DBG //printf 
#define LOCKSCHED // 
#define UNLOCKSCHED // 

struct tunif_packet
{
	struct pbuf	*p;
	int is_link;
	TAILQ_ENTRY(tunif_packet) next;
};


struct tunif_pcb {
	char 			if_name[8];
	wait_queue_head_t 	read_que;

	struct netif *tunif_netif;
	struct ip_addr tunif_ip;
	struct ip_addr tunif_gw;
	struct ip_addr tunif_mask;
	
	long tunif_mode,tunif_flags;
	int tunif_lock;
	TAILQ_HEAD(,tunif_packet) tunif_packet;

	LIST_ENTRY(tunif_pcb) next;
	dev_prvi_t* devprvi;
};


static LIST_HEAD(,tunif_pcb) tun_list;

extern err_t ether_output(struct netif *netif, struct pbuf *p, struct ip_addr *ipaddr);

struct tunif_pcb *find_tunif(struct netif *netif)
{
	struct tunif_pcb *pcb;

	LOCKSCHED;

	LIST_FOREACH (pcb,&tun_list,  next)
	{
		if (pcb->tunif_netif == netif)
		{
			UNLOCKSCHED;
			return pcb;
		}
	}

	UNLOCKSCHED;
	return NULL;
}

struct tunif_pcb *find_tunif_byname(char *name)
{
	struct tunif_pcb *pcb;

	LOCKSCHED;

	LIST_FOREACH (pcb,&tun_list,  next)
	{
		if (strcmp(pcb->if_name, name)==0)
		{
			UNLOCKSCHED;
			return pcb;
		}

	}

	UNLOCKSCHED;
	return NULL;
}

int tunif_add_packet(struct tunif_pcb *pcb,struct pbuf *p, int isarp)
{
	struct tunif_packet *tmp;
	
	tmp=kmalloc(sizeof(struct tunif_packet),0);

	if (!tmp)
	{
		return -1;
	}

	tmp->p = p;
	tmp->is_link = isarp;

	lock_semaphore(pcb->tunif_lock);
	//packets limited?
	//if(packets<TUN_TXQ_SIZE)
	TAILQ_INSERT_TAIL(&pcb->tunif_packet, tmp,next);
	unlock_semaphore(pcb->tunif_lock);
	DBG("tunif_add_packet add %d bytes\n", p->len);
	return 0;
}

int tunif_queue_empty(struct tunif_pcb *pcb)
{
	int e;

	lock_semaphore(pcb->tunif_lock);
	e = TAILQ_EMPTY(&pcb->tunif_packet);
	unlock_semaphore(pcb->tunif_lock);

	return e;

}

struct pbuf * tunif_get_packet(struct tunif_pcb *pcb, int *is_arp)
{
	struct tunif_packet *tmp;
	struct pbuf *p;

	lock_semaphore(pcb->tunif_lock);
	if (TAILQ_EMPTY(&pcb->tunif_packet))
	{
		unlock_semaphore(pcb->tunif_lock);
		return NULL;
	}


	tmp= TAILQ_FIRST(&pcb->tunif_packet);
	TAILQ_REMOVE(&pcb->tunif_packet,tmp,next);

	unlock_semaphore(pcb->tunif_lock);
	p = tmp->p;
	*is_arp = tmp->is_link;

	kfree(tmp);

	return p;
}

//arp
err_t tunif_linkoutput (struct netif *inp, struct pbuf *p)
{
	//从mac获取数据
	int err;
	struct tunif_pcb *pcb;

	//DBG("tunif_linkoutput %p...\n",p);

	pcb = find_tunif(inp);

	if (!pcb)
		return -1;

	tunif_add_packet(pcb,p,1);

	err=set_io_event(&pcb->devprvi->iob, IOEVT_READ);
	if (err==0)
	{
		thread_wakeup(&pcb->read_que);
	}

	//add to queue
	//DBG("packet add %d bytes succ\n", p->len);
	return 0;
}


err_t tunif_output(struct netif *inp, struct pbuf *p, struct ip_addr *ipaddr)
{
	//从mac获取数据
	int err;
	struct tunif_pcb *pcb;

	//DBG("tunif_output %p...\n",p);

	pcb = find_tunif(inp);

	if (!pcb)
		return -1;

	//DBG("packet add %d bytes\n", p->len);
	tunif_add_packet(pcb,p,0);

	err=set_io_event(&pcb->devprvi->iob, IOEVT_READ);
	if (err==0)
	{
		//thread_wakeup(&client->wait_queue);
		thread_wakeup(&pcb->read_que);
	}

	//add to queue
	//DBG("packet add %d bytes succ\n", p->len);
	return 0;
}



struct tunif_pcb *get_tun(char *name)
{
	struct tunif_pcb *pcb = kmalloc(sizeof(struct tunif_pcb),0);

	if (!pcb)
	{
		return NULL;
	}

	memset(pcb,0,sizeof(struct tunif_pcb));
	thread_waitq_init(&pcb->read_que);
	strcpy(pcb->if_name,name);
	LIST_INSERT_HEAD(&tun_list,pcb,next);
	return pcb;
}

int tunif_delete(struct tunif_pcb *pcb)
{
	LIST_REMOVE(pcb,next);
	destroy_semaphore(pcb->tunif_lock);
	kfree(pcb);
	return 0;
}


int tunif_create(struct tunif_pcb *pcb)
{
  struct netif *tunif_netif;
  unsigned char mac[6]={0x00, 0xFF, 0xD4, 0xA3, 0x87, 0x0B};


  tunif_netif = netif_add(pcb->if_name, &pcb->tunif_ip, &pcb->tunif_mask, &pcb->tunif_gw);
  if (!tunif_netif) return -1;

  /*need Generate random Ethernet address?  */
  mac[5] = startup_ticks();
  memcpy(tunif_netif->hwaddr.addr,mac,ETHER_ADDR_LEN);

  tunif_netif->linkoutput = tunif_linkoutput;
  tunif_netif->output = tunif_output;
  tunif_netif->flags |=  (NETIF_UP|NETIF_NO_ARP) ;
  //tunif_netif->input = tunif_output;


	//	*(u16 *)dev->dev_addr = htons(0x00FF);
	//	get_random_buffer(dev->dev_addr + sizeof(u16), 4);


  pcb->tunif_netif = tunif_netif;
  pcb->tunif_lock = create_semaphore( "tun interface lock", SEMAPHONE_CLEAR_FLAGS, 1 );
  TAILQ_INIT(&pcb->tunif_packet);

  //DBG("tunif_create tunif_output %p...\n",tunif_output);
  return 0;
}


int tunif_read(dev_prvi_t* devfp, off_t  offset, char * buf,int count)
{
	struct tunif_pcb *pcb = NULL;
	int len;
	struct pbuf *p;
	struct eth_hdr *eth_hdr;
	int taplen=0;
	unsigned char mac[6]={0,0,0,0,0,0};
	int is_arp=0;
	file_t* file;
  
	file = get_dev_filp(devfp);
	if (!file)
	{
	  return -1;
	}

	pcb = devfp->data;
	if (!pcb)
		return -1;

	if (pcb->tunif_mode == IFF_TAP)
	{
		taplen = sizeof(struct eth_hdr);
	}

	do
	{
		p = tunif_get_packet(pcb,&is_arp);

		if (!p  )
		{
			if (file->f_mode&O_NONBLOCK)
			{
				return -EINTR;
			}
			else
				thread_sleep_on(&pcb->read_que);
		}
		else{
			break;
		}

	}
	while (1);

	if (p->len!=p->tot_len)
	{
	DBG("packet got %d,p->tot_len %d bytes\n", p->len,p->tot_len);
	}


	if (!is_arp)
	{
		eth_hdr = (struct eth_hdr *)buf;
		eth_hdr->type = htons(ETHTYPE_IP);

		memcpy(eth_hdr->dest.addr, mac,ETHER_ADDR_LEN);
		memcpy(eth_hdr->src.addr, pcb->tunif_netif->hwaddr.addr,ETHER_ADDR_LEN);
		
		len =  p->len+taplen;
		memcpy(buf+taplen,p->payload,p->len);
	}
	else{
		len =  p->len;
		memcpy(buf,p->payload,p->len);
	}

	DBG("tunif_read %d succ\n", len);

	if (tunif_queue_empty(pcb))
	{
		clear_io_event(&devfp->iob, IOEVT_READ);
	}


	pbuf_free(p);
	return len;

}
err_t ip_output(struct pbuf *p, struct ip_addr *src, struct ip_addr *dest, int ttl, int proto);

int tunif_write(dev_prvi_t* devfp, off_t  offset, char * buf,int count)
{
	struct tunif_pcb *pcb = NULL;
	struct pbuf *p;
	struct ip_hdr *iphdr;
	int err,len,taplen=0;

	pcb = devfp->data;

	if (!pcb)
		return -1;

	len = count-taplen;

	p = pbuf_alloc(PBUF_RAW, ETHER_FRAME_LEN, PBUF_RW);
	if (!p) return ENOMEM;

	memcpy(p->payload,buf, len);
	pbuf_realloc(p, count);
	iphdr =p->payload;
	
	err=ether_input(pcb->tunif_netif,p);
	//err=ip_forward(p, iphdr, pcb->tunif_netif);
	if (err)
	{
	  pbuf_free(p);
	}
	DBG("tunif_write %d succ\n",count);
	return count;
}


static int tunif_drv_open(const char *f, int mode,dev_prvi_t* devfp)
{
	struct tunif_pcb *pcb = NULL;


	pcb =get_tun("tun");
	if (!pcb)
		return -1;

	devfp->data = pcb;
	pcb->devprvi = devfp;
	return 0;
}

static int tunif_drv_close(dev_prvi_t* devfp)
{  
	struct tunif_pcb *pcb = NULL;

	pcb = devfp->data;

	if (!pcb)
		return -1;

	return 0;
}


int tunif_drv_ioctl(dev_prvi_t* devfp, int cmd, void* args,int size, int bfrom)
{
	struct tunif_pcb *pcb = NULL;
	int err=0;
	int block;
	char *ifname="TUN";

	args = current_proc_vir2phys(args);

	pcb = devfp->data;
	if (!pcb)
		return -1;

	switch (cmd)
	{
	case TUN_IOC_CREATE_IF:

		ifname = args;
		DBG("ifname = %s\n",ifname);

		strcpy(pcb->if_name,ifname);
		err=tunif_create(pcb);

		if (err)
		{
			DBG("tunif_create error on ifname = %s\n",ifname);
			tunif_delete(pcb);
			return err;
		}

		break;

	case TUN_IOC_DELETE_IF:
		netif_delete(pcb->tunif_netif);
		tunif_delete(pcb);
		break;

	case TUN_IOC_IF_MODE:
		block = *(int *)args;

		if (block!=IFF_TUN && block!=IFF_TAP)
		{
			return -1;
		}
		pcb->tunif_mode=block;
		break;

	case FIONBIO:
		block = *(int *)args;

		DBG("block = %d\n",block);
		pcb->tunif_flags = block;
		break;

	case TUN_IOC_IF_ADDR:

		if (!pcb)
		{
			DBG("TUN_IOC_IF_ADDR: create first\n");
			return -1;
		}

	  pcb->tunif_ip.addr = 0;
	  pcb->tunif_mask.addr = 0;
	  pcb->tunif_gw.addr = 0;

	  DBG("addr = %s\n",args);

	 struct ip_addr *ip=args;
	  pcb->tunif_ip = ip[0];
	  pcb->tunif_mask  = ip[1];
	  pcb->tunif_gw  = ip[2];
	  break;

	 default:
		 err=-1;
		 break;	
	}

	return err;
}



static const driver_ops_t tunif_drv_fops =
{
	d_name:		"tun",
	d_author:	"easion",
	d_version:	"2009/10/4",
	d_kver:	CURRENT_KERNEL_VERSION,
	d_index:	ALLOC_MAJOR_NUMBER,
	open:		tunif_drv_open,
	close:		tunif_drv_close,
	read:		tunif_read,
	write:		tunif_write,
	ioctl:		tunif_drv_ioctl,		
};



int dll_main()
{
	int retval;

	retval=kernel_driver_register(&tunif_drv_fops);		
	if(retval < 0)
    {
      printk("Could not register tunif device\n");
      return 0;
    }  

	LIST_INIT(&tun_list);
	return 0;
}


int dll_destroy()
{
	return 0;
}




