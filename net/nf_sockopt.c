#include <net/net.h>

#include <net/nf_hook.h> 



static TAILQ_HEAD(,nf_sockopt_ops) head;

void nf_sockopt_init(void)
{
	TAILQ_INIT(&head);
}

int nf_register_sockopt(struct nf_sockopt_ops *reg)
{
	int ret=0;
	unsigned flags;


	if (!reg)
	{
		return EINVAL;
	}

	save_eflags(&flags);

	TAILQ_INSERT_TAIL(&head,reg,next);
	restore_eflags(flags);
	return ret;
}

void nf_unregister_sockopt(struct nf_sockopt_ops *reg)
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


int nf_setsockopt(struct socket *s, int level, int optname, const char *optval, int optlen, int *ret)
{
	struct nf_sockopt_ops *reg;


	TAILQ_FOREACH(reg,&head,next){

	//kprintf("%s: %d %d\n", __FUNCTION__, level, optname);
		if (reg->pf == PF_INET && optname > reg->set_optmin  && optname < reg->set_optmax)
		{
			*ret = reg->set(s, optname,optval, optlen);			
			return 1;			
		}
	}

	return 0;
}


int nf_getsockopt(struct socket *s, int level, int optname, char *optval, int *optlen, int *ret)
{
	struct nf_sockopt_ops *reg;


	TAILQ_FOREACH(reg,&head,next){
	//kprintf("%s: %d %d\n", __FUNCTION__, level, optname);

		if (reg->pf == PF_INET && optname > reg->get_optmin  && optname < reg->get_optmax)
		{
			*ret = reg->get(s, optname,optval, optlen);			
			return 1;			
		}
	}

	return 0;
}


