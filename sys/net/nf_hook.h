
#include <sys/queue.h>
//struct netif *netif, struct pbuf *p
typedef unsigned int nf_hookfn(unsigned int hooknum,
                               struct pbuf **skb,
                               const struct netif *in,
                               const struct netif *out,
                               int (*okfn)(struct pbuf *));


enum{
	NF_IP_PRE_ROUTING,
	NF_IP_POST_ROUTING
};

enum{NF_ACCEPT,NF_DROP,NF_STOLEN,NF_IP_LOCAL_OUT};

struct   nf_hook_ops   
  {   
  //struct   list_head   list;  
    
  /*   User   fills   in   from   here   down.   */   
  nf_hookfn   *hook;   
  int   pf;   
  int   hooknum;   
  /*   Hooks   are   ordered   in   ascending   priority.   */   
  int   priority;   
  TAILQ_ENTRY(nf_hook_ops) next;
  };   

int nf_register_hook(struct nf_hook_ops *reg);
void nf_unregister_hook(struct nf_hook_ops *reg);

int nf_hook_post_route(struct netif *netif, struct pbuf *p);
int nf_hook_pre_route(struct netif *netif, struct pbuf *p);
