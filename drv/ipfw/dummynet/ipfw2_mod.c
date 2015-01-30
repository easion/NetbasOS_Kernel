

#include <sys/cdefs.h>
#include <sys/mbuf.h>			/* sizeof struct mbuf */
#include <drv/nf_hook.h>

///#include <netinet/in.h>			/* in_addr */
#include <netinet/ip_fw.h>		/* ip_fw_ctl_t, ip_fw_chk_t */
#include <netinet/ip_dummynet.h>	/* ip_dn_ctl_t, ip_dn_io_t */
#include <net/pfil.h>	/* PFIL_IN, PFIL_OUT */

#define REG_QH_ARG(fn)	fn, NULL	/* argument for nf_[un]register_queue_handler */
#define UNREG_QH_ARG(fn) //fn	/* argument for nf_[un]register_queue_handler */
#define SET_MOD_OWNER


#if 1

#define AF_INET          2
#define PF_INET          AF_INET

void nf_unregister_sockopt(struct nf_sockopt_ops *reg);
int nf_register_sockopt(struct nf_sockopt_ops *reg);


struct nf_sockopt_ops
{
	TAILQ_ENTRY(nf_sockopt_ops) next;

	int pf;

	/* Non-inclusive ranges: use 0/0/NULL to never get called. */
	int set_optmin;
	int set_optmax;
	int (*set)(struct socket *sk, int optval, void *user, unsigned int len);

	int get_optmin;
	int get_optmax;
	int (*get)(struct socket *sk, int optval, void *user, int *len);

	/* Number of users inside set() or get(). */
	unsigned int use;
       // struct task_struct *cleanup_task;
};
#endif


static struct nf_sockopt_ops ipfw_sockopts;


/*
 * Here we allocate some global variables used in the firewall.
 */
ip_dn_ctl_t    *ip_dn_ctl_ptr;
ip_fw_ctl_t    *ip_fw_ctl_ptr;

ip_dn_io_t     *ip_dn_io_ptr;
ip_fw_chk_t    *ip_fw_chk_ptr;

void		(*bridge_dn_p)(struct mbuf *, struct ifnet *);

/*---
 * Glue code to implement the registration of children with the parent.
 *
 * Each child should call my_mod_register() when linking, so that
 * module_init() and module_exit() can call init_children() and
 * fini_children() to provide the necessary initialization.
 */
#include <sys/module.h>
struct mod_args {
        struct moduledata *mod;
        const char *name;
        int order;
};

static unsigned int mod_idx;
static struct mod_args mods[10];	/* hard limit to 10 modules */

/*
 * my_mod_register should be called automatically as the init
 * functions in the submodules. Unfortunately this compiler/linker
 * trick is not supported yet so we call it manually.
 */
int
my_mod_register(struct moduledata *mod, const char *name, int order)
{
	struct mod_args m = { mod, name, order };

	printf("%s %s called\n", __FUNCTION__, name);
	if (mod_idx < sizeof(mods) / sizeof(mods[0]))
		mods[mod_idx++] = m;
	return 0;
}

static void
init_children(void)
{
	unsigned int i;

        /* Call the functions registered at init time. */
	printf("%s mod_idx value %d\n", __FUNCTION__, mod_idx);
        for (i = 0; i < mod_idx; i++) {
                printf("+++ start module %d %s %s at %p order 0x%x\n",
                        i, mods[i].name, mods[i].mod->name,
                        mods[i].mod, mods[i].order);
                mods[i].mod->evhand(NULL, MOD_LOAD, mods[i].mod->priv);
        }
}

static void
fini_children(void)
{
	int i;

        /* Call the functions registered at init time. */
        for (i = mod_idx - 1; i >= 0; i--) {
                printf("+++ end module %d %s %s at %p order 0x%x\n",
                        i, mods[i].name, mods[i].mod->name,
                        mods[i].mod, mods[i].order);
                mods[i].mod->evhand(NULL, MOD_UNLOAD, mods[i].mod->priv);
        }
}

static int
nf_register_hooks(struct nf_hook_ops *ops, int n)
{
	int i, ret = 0;
	for (i = 0; i < n; i++) {
		ret = nf_register_hook(ops + i);
		if (ret < 0)
			break;
	}
	return ret;
}

static void
nf_unregister_hooks(struct nf_hook_ops *ops, int n)
{
	int i;
	for (i = 0; i < n; i++) {
		nf_unregister_hook(ops + i);
	}
}


/*
 * The main netfilter hook.
 * To make life simple, we queue everything and then do all the
 * decision in the queue handler.
 *
 * XXX note that in 2.4 and up to 2.6.22 the skbuf is passed as pbuf**
 * so we have an #ifdef to set the proper argument type.
 */
static unsigned int
call_ipfw_in(unsigned int  hooknum,
	struct pbuf   **skb,
	const struct netif   *in,
	const struct netif   *out,
	int  (*okfn)(struct pbuf *));

static unsigned int
call_ipfw_out(unsigned int  hooknum,
	struct pbuf   **skb,
	const struct netif   *in,
	const struct netif   *out,
	int  (*okfn)(struct pbuf *));
static struct nf_hook_ops ipfw_ops[]  = {
        {
                .hook           = call_ipfw_in,
                .pf             = AF_INET,
                .hooknum        = NF_IP_PRE_ROUTING,
                .priority       = 0,
               // SET_MOD_OWNER
        },
        {
                .hook           = call_ipfw_out,
                .pf             = AF_INET,
                .hooknum        = NF_IP_POST_ROUTING,
                .priority       = 0,
		//SET_MOD_OWNER
        },
};

//static int ipfw2_queue_handler(struct pbuf *skb, struct nf_info *info, void *data)
static unsigned int
call_ipfw_in(unsigned int  hooknum,
	struct pbuf   **skb,
	const struct netif   *in,
	const struct netif   *out,
	int  (*okfn)(struct pbuf *))

{
	//DEFINE_SKB	/* no semicolon here, goes in the macro */
	int ret = 0;	/* return value */
	struct mbuf *m;
	struct pbuf   *p=*skb;

	//printf("%s called\n", __FUNCTION__);
	//return NF_ACCEPT;

	m = malloc(sizeof(*m), 0, 0);
	if (m == NULL) {
		//printf("malloc fail, len %d reinject now\n", skb->len);
		//REINJECT(info, NF_ACCEPT);
		return 0;
	}

	m->m_skb = p;
	m->m_len = p->len;		/* len in this skbuf */
	m->m_pkthdr.len = p->len;	/* total packet len */
	m->m_pkthdr.rcvif = in;
	//m->queue_entry = info;
	m->m_data = p->payload;

	//m->m_data = skb_network_header(skb);
		ret = ipfw_check_in(NULL, &m, in, PFIL_IN, NULL);

	/* XXX add the interface */
	/*if (info->hook == NF_IP_PRE_ROUTING) {
	} else {
		ret = ipfw_check_out(NULL, &m, info->outdev, PFIL_OUT, NULL);
	}*/
		printf("%s: %p %d\n",__FUNCTION__, m, ret);

	if (m != NULL) {	/* Accept. reinject and free the mbuf */
		//REINJECT(info, NF_STOP);
		m_freem(m);
	} else if (ret == 0) {
		/* dummynet has kept the packet, will reinject later. */
	} else {
		/*
		 * Packet dropped by ipfw or dummynet, reinject as NF_DROP
		 * mbuf already released by ipfw itself
		 */
	        //REINJECT(info, NF_DROP);
			return NF_DROP;
	}
	return NF_ACCEPT;
}

static unsigned int
call_ipfw_out(unsigned int  hooknum,
	struct pbuf   **skb,
	const struct netif   *in,
	const struct netif   *out,
	int  (*okfn)(struct pbuf *))

{
	//DEFINE_SKB	/* no semicolon here, goes in the macro */
	int ret = 0;	/* return value */
	struct mbuf *m;
	struct pbuf   *p=*skb;

	//printf("%s called\n", __FUNCTION__);
	//return NF_ACCEPT;


	m = malloc(sizeof(*m), 0, 0);
	if (m == NULL) {
		printf("malloc fail, len %d reinject now\n", p->len);
		//REINJECT(info, NF_ACCEPT);
		return NF_ACCEPT;
	}

	m->m_skb = skb;
	m->m_len = p->len;		/* len in this skbuf */
	m->m_pkthdr.len = p->len;	/* total packet len */
	m->m_pkthdr.rcvif = out;
	//m->queue_entry = info;
	m->m_data = p->payload;

	//m->m_data = skb_network_header(skb);

	/* XXX add the interface */
	/*if (info->hook == NF_IP_PRE_ROUTING) {
		ret = ipfw_check_in(NULL, &m, info->indev, PFIL_IN, NULL);
	} else {
	}*/
		ret = ipfw_check_out(NULL, &m, out, PFIL_OUT, NULL);

		printf("%s: %p %d\n",__FUNCTION__, m, ret);

	if (m != NULL) {	/* Accept. reinject and free the mbuf */
		//REINJECT(info, NF_STOP);
		m_freem(m);
	} else if (ret == 0) {
		/* dummynet has kept the packet, will reinject later. */
	} else {
		/*
		 * Packet dropped by ipfw or dummynet, reinject as NF_DROP
		 * mbuf already released by ipfw itself
		 */
	        //REINJECT(info, NF_DROP);
			return NF_DROP;
	}
	return NF_ACCEPT;
}


int
ip_output(struct mbuf *m, struct mbuf  *opt,
	struct route  *ro, int  flags,
    struct ip_moptions  *imo, struct inpcb  *inp)
{
	netisr_dispatch(0, m);
        return 0;
}

void
netisr_dispatch(int num, struct mbuf *m)
{
	//struct nf_queue_entry *info = m->queue_entry;
	//struct pbuf *skb = m->m_skb;	/* always used */
	printf("%s called\n", __FUNCTION__);

	m_freem(m);

	//KASSERT((info != NULL), ("%s info null!\n", __FUNCTION__));
//#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,22)	// XXX above 2.6.x ?
	//__net_timestamp(skb);	/* update timestamp */
//#endif

	/* XXX to obey one-pass, possibly call the queue handler here */
	//REINJECT(info, ((num == -1)?NF_DROP:NF_STOP));	/* accept but no more firewall */
}
/* descriptors for the children */
extern moduledata_t *moddesc_ipfw;
extern moduledata_t *moddesc_ipfw_nat;
extern moduledata_t *moddesc_dummynet;

/*
 * Module glue - init and exit function.
 */
static int __init
dll_main(char **args)
{
	int ret = 0;

	printf("%s called\n", __FUNCTION__);

	my_mod_register(moddesc_ipfw, "ipfw",  1);
	my_mod_register(moddesc_dummynet, "dummynet",  2);
	my_mod_register(moddesc_ipfw_nat, "ipnat",  2);
	init_children();

        ret = nf_register_hooks(ipfw_ops, ARRAY_SIZE(ipfw_ops));
        if (ret < 0)return -1;

	/* sockopt register, in order to talk with user space */
	ret = nf_register_sockopt(&ipfw_sockopts);
        if (ret < 0) {
		printf("error %d in nf_register_sockopt\n", ret);
		goto clean_modules;
	}


	printf("%s loaded\n", __FUNCTION__);
	return 0;

unregister_sockopt:
	nf_unregister_sockopt(&ipfw_sockopts);

clean_modules:
	fini_children();
	printf("%s error\n", __FUNCTION__);

	return ret;
}

/* module shutdown */
static void __exit
dll_destroy(void)
{
    nf_unregister_hooks(ipfw_ops, ARRAY_SIZE(ipfw_ops));
	nf_unregister_sockopt(&ipfw_sockopts);


	fini_children();

	printf("%s unloaded\n", __FUNCTION__);
}


static int
ipfw_ctl_h(struct sockopt *s, int cmd, int dir, int len, void __user *user)
{
	struct thread t;
	int ret = EINVAL;

	memset(s, 0, sizeof(s));
	s->sopt_name = cmd;
	s->sopt_dir = dir;
	s->sopt_valsize = len;
	s->sopt_val = user;

	/* sopt_td is not used but it is referenced */
	memset(&t, 0, sizeof(t));
	s->sopt_td = &t;
	
	//printf("%s called with cmd %d len %d\n", __FUNCTION__, cmd, len);

	if (cmd < IP_DUMMYNET_CONFIGURE && ip_fw_ctl_ptr)
		ret = ip_fw_ctl_ptr(s);
	else if (cmd >= IP_DUMMYNET_CONFIGURE && ip_dn_ctl_ptr)
		ret = ip_dn_ctl_ptr(s);

	return -ret;	/* errors are < 0 on linux */
}

/*
 * setsockopt hook has no return value other than the error code.
 */
static int
do_ipfw_set_ctl(struct sock  *sk, int cmd,
	void __user *user, unsigned int len)
{
	struct sockopt s;	/* pass arguments */
	//printf("%s called\n", __FUNCTION__);

	return ipfw_ctl_h(&s, cmd, SOPT_SET, len, user);
}

/*
 * getsockopt can can return a block of data in response.
 */
static int
do_ipfw_get_ctl(struct sock  *sk,
	int cmd, void __user *user, int *len)
{
	struct sockopt s;	/* pass arguments */
	int ret = ipfw_ctl_h(&s, cmd, SOPT_GET, *len, user);
	//printf("%s called\n", __FUNCTION__);

	*len = s.sopt_valsize;	/* return lenght back to the caller */
	return ret;
}


/*
 * declare our [get|set]sockopt hooks
 */
static struct nf_sockopt_ops ipfw_sockopts = {
	.pf		= PF_INET,
	.set_optmin	= _IPFW_SOCKOPT_BASE,
	.set_optmax	= _IPFW_SOCKOPT_END,
	.set		= do_ipfw_set_ctl,
	.get_optmin	= _IPFW_SOCKOPT_BASE,
	.get_optmax	= _IPFW_SOCKOPT_END,
	.get		= do_ipfw_get_ctl,


};


