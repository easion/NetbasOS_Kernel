

#include <net/net.h>

struct stats_all stats;

static void protstat(struct proc_entry *pf, char *prot, struct stats_proto *stat)
{
  pprintf(pf, "%-4s%7d%6d%7d%6d%6d%6d%6d%6d%6d%6d%6d%6d\n",
          prot, stat->xmit, stat->rexmit, stat->recv, stat->fw, stat->drop,
	  stat->chkerr, stat->lenerr, stat->memerr, stat->rterr, stat->proterr,
	  stat->opterr, stat->err);
}

static int netstat_proc(void *arg, int len,struct proc_entry *pf)
{
  pprintf(pf, "     -------------- packets -------- ----------------- errors ----------------\n");
  pprintf(pf, "       xmit rexmt   recv forwd  drop cksum   len   mem route proto   opt  misc\n");
  pprintf(pf, "---- ------ ----- ------ ----- ----- ----- ----- ----- ----- ----- ----- -----\n");

  protstat(pf, "link", &stats.link);
  protstat(pf, "ip", &stats.ip);
  protstat(pf, "icmp", &stats.icmp);
  protstat(pf, "udp", &stats.udp);
  protstat(pf, "tcp", &stats.tcp);
  protstat(pf, "raw", &stats.raw);

  return 0;
}

static int pbufs_proc(void *arg, int len,struct proc_entry *pf)
{
  pprintf(pf, "Pool Available .. : %6d\n", stats.pbuf.avail);
  pprintf(pf, "Pool Used ....... : %6d\n", stats.pbuf.used);
  pprintf(pf, "Pool Max Used ... : %6d\n", stats.pbuf.max);
  pprintf(pf, "Errors .......... : %6d\n", stats.pbuf.err);
  pprintf(pf, "Reclaimed ....... : %6d\n", stats.pbuf.reclaimed);
  pprintf(pf, "R/W Allocated ... : %6d\n", stats.pbuf.rwbufs);

  return 0;
}

static struct proc_entry e_netstat_proc = {
	name: "netstat",
	write_func: netstat_proc,
	read_func: NULL,
};

static struct proc_entry e_pbufs_proc = {
	name: "pbufs",
	write_func: pbufs_proc,
	read_func: NULL,
};

void stats_init()
{
  memset(&stats, 0, sizeof(stats));
  register_proc_entry(&e_netstat_proc);
  register_proc_entry(&e_pbufs_proc);  
}
