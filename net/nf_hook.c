#include <net/net.h>

#include <net/nf_hook.h> 

static TAILQ_HEAD(,nf_hook_ops) head;

void nf_init(void)
{
	TAILQ_INIT(&head);
}

int nf_register_hook(struct nf_hook_ops *reg)
{
	int ret=0;
	unsigned flags;

	if (!reg)
	{
		return -1;
	}

	save_eflags(&flags);

	TAILQ_INSERT_TAIL(&head,reg,next);
	restore_eflags(flags);
	return ret;
}

void nf_unregister_hook(struct nf_hook_ops *reg)
{
	int ret=0;
	unsigned flags;

	if (!reg)
	{
		return ;
	}

	save_eflags(&flags);

	TAILQ_REMOVE(&head,reg,next);
	restore_eflags(flags);

	return ret;
}

int nf_hook_pre_route(struct netif *netif, struct pbuf *p)
{
	struct nf_hook_ops *reg;
	int err;

	TAILQ_FOREACH(reg,&head,next){

		if (reg->pf == PF_INET && reg->hooknum == NF_IP_PRE_ROUTING)
		{
			err = reg->hook(NF_IP_PRE_ROUTING,&p,netif,NULL,NULL);
			if (err != NF_ACCEPT)
			{
				return NF_DROP;
			}
		}
	}

	return NF_ACCEPT;
}

int nf_hook_post_route(struct netif *netif, struct pbuf *p)
{
	struct nf_hook_ops *reg;
	int err;

	TAILQ_FOREACH(reg,&head,next){

		if (reg->pf == PF_INET && reg->hooknum == NF_IP_POST_ROUTING)
		{
			err = reg->hook(NF_IP_PRE_ROUTING,&p,NULL,netif,NULL);
			if (err != NF_ACCEPT)
			{
				return NF_DROP;
			}
		}
	}

	return NF_ACCEPT;
}


