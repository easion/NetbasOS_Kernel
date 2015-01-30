/*
 * Copyright (C) 2009 Luigi Rizzo, Universita` di Pisa
 *
 * BSD copyright.
 *
 * A simple compatibility interface to map mbufs onto sk_buff
 */

#ifndef _SYS_MBUF_H_
#define	_SYS_MBUF_H_

#include <sys/malloc.h>		/* we use free() */
/* hopefully queue.h is already included by someone else */
//#include <sys/queue.h>
#ifdef _KERNEL

/* bzero not present on linux, but this should go in glue.h */
#define bzero(s, n) memset(s, 0, n)

/*
 * We implement a very simplified UMA allocator where the backend
 * is simply malloc, and uma_zone only stores the length of the components.
 */
typedef int uma_zone_t;		/* the zone size */

#define uma_zcreate(name, len, _3, _4, _5, _6, _7, _8)	(len)


#define uma_zfree(zone, item)	free(item, M_IPFW)
#define uma_zalloc(zone, flags) malloc(zone, M_IPFW, flags)
#define uma_zdestroy(zone)	do {} while (0)

/*-
 * Macros for type conversion:
 * mtod(m, t)	-- Convert mbuf pointer to data pointer of correct type.
 */
#define	mtod(m, t)	((t)((m)->m_data))

#endif /* _KERNEL */

/*
 * Packet tag structure (see below for details).
 */
struct m_tag {
	SLIST_ENTRY(m_tag)	m_tag_link;	/* List of packet tags */
	u_int16_t		m_tag_id;	/* Tag ID */
	u_int16_t		m_tag_len;	/* Length of data */
	u_int32_t		m_tag_cookie;	/* ABI/Module ID */
	void			(*m_tag_free)(struct m_tag *);
};

#if defined(__NETBAS__) || defined( _WIN32 )

/*
 * Auxiliary structure to store values from the sk_buf.
 * Note that we should not alter the sk_buff, and if we do
 * so make sure to keep the values in sync between the mbuf
 * and the sk_buff (especially m_len and m_pkthdr.len).
 */

struct mbuf {
	struct mbuf *m_next;
	struct mbuf *m_nextpkt;
	void *m_data;
	int m_len;	/* length in this mbuf */
	int m_flags;

	//struct nf_info *queue_entry;

	void *queue_entry;

	struct pbuf *m_skb;
	struct {
		struct netif *rcvif;
		int len;	/* total packet len */
		int     csum_flags;             /* flags regarding checksum */
        int     csum_data;              /* data field used by csum routines */

		SLIST_HEAD (packet_tags, m_tag) tags;
	} m_pkthdr;
};

#define M_SKIP_FIREWALL	0x01		/* skip firewall processing */
#define M_BCAST         0x02 /* send/received as link-level broadcast */
#define M_MCAST         0x04 /* send/received as link-level multicast */

#define M_DONTWAIT      M_NOWAIT	/* should not be here... */


/*
 * m_dup() is used in the TEE case, currently unsupported so we
 * just return.
 */
static __inline struct mbuf	*m_dup(struct mbuf __unused *m, int __unused n)
{
	return NULL;
};

#define	MTAG_ABI_COMPAT		0		/* compatibility ABI */
static __inline struct m_tag *
m_tag_find(struct mbuf __unused *m, int __unused type, struct m_tag __unused *start)
{
	return NULL;
};


static __inline void
m_tag_prepend(struct mbuf *m, struct m_tag *t)
{
	SLIST_INSERT_HEAD(&m->m_pkthdr.tags, t, m_tag_link);
}

/*
 * Create an mtag of the given type
 */
static __inline struct m_tag *
m_tag_get(int type, int length, int wait)
{
	int l = length + sizeof(struct m_tag);
	struct m_tag *m = malloc(l, 0, M_NOWAIT);
	if (m) {
		memset(m, 0, l);
		m->m_tag_id = type;
		m->m_tag_len = length;
	}
	return m;
};

static __inline struct m_tag *
m_tag_first(struct mbuf *m)
{
	return SLIST_FIRST(&m->m_pkthdr.tags);
};

static __inline void
m_tag_delete(struct mbuf *m, struct m_tag *t)
{
};

static __inline struct m_tag *
m_tag_locate(struct mbuf *m, u_int32_t n, int x, struct m_tag *t)
{
	return NULL;
};

static __inline void
m_freem(struct mbuf *m)
{

	struct m_tag *t;

	/* free the m_tag chain */
	while ( (t = SLIST_FIRST(&m->m_pkthdr.tags) ) ) {
		SLIST_REMOVE_HEAD(&m->m_pkthdr.tags, m_tag_link);
		free(t, 0);
	}

	/* free the mbuf */
	free(m, M_IPFW);
};

/* we cannot pullup */
#define m_pullup(__m, __i)	(m)

#define M_GETFIB(_m)	0

#endif /* !__NETBAS__ */

/*
 * Persistent tags stay with an mbuf until the mbuf is reclaimed.  Otherwise
 * tags are expected to ``vanish'' when they pass through a network
 * interface.  For most interfaces this happens normally as the tags are
 * reclaimed when the mbuf is free'd.  However in some special cases
 * reclaiming must be done manually.  An example is packets that pass through
 * the loopback interface.  Also, one must be careful to do this when
 * ``turning around'' packets (e.g., icmp_reflect).
 *
 * To mark a tag persistent bit-or this flag in when defining the tag id.
 * The tag will then be treated as described above.
 */
#define	MTAG_PERSISTENT				0x800

#define	PACKET_TAG_NONE				0  /* Nadda */

/* Packet tags for use with PACKET_ABI_COMPAT. */
#define	PACKET_TAG_IPSEC_IN_DONE		1  /* IPsec applied, in */
#define	PACKET_TAG_IPSEC_OUT_DONE		2  /* IPsec applied, out */
#define	PACKET_TAG_IPSEC_IN_CRYPTO_DONE		3  /* NIC IPsec crypto done */
#define	PACKET_TAG_IPSEC_OUT_CRYPTO_NEEDED	4  /* NIC IPsec crypto req'ed */
#define	PACKET_TAG_IPSEC_IN_COULD_DO_CRYPTO	5  /* NIC notifies IPsec */
#define	PACKET_TAG_IPSEC_PENDING_TDB		6  /* Reminder to do IPsec */
#define	PACKET_TAG_BRIDGE			7  /* Bridge processing done */
#define	PACKET_TAG_GIF				8  /* GIF processing done */
#define	PACKET_TAG_GRE				9  /* GRE processing done */
#define	PACKET_TAG_IN_PACKET_CHECKSUM		10 /* NIC checksumming done */
#define	PACKET_TAG_ENCAP			11 /* Encap.  processing */
#define	PACKET_TAG_IPSEC_SOCKET			12 /* IPSEC socket ref */
#define	PACKET_TAG_IPSEC_HISTORY		13 /* IPSEC history */
#define	PACKET_TAG_IPV6_INPUT			14 /* IPV6 input processing */
#define	PACKET_TAG_DUMMYNET			15 /* dummynet info */
#define	PACKET_TAG_DIVERT			17 /* divert info */
#define	PACKET_TAG_IPFORWARD			18 /* ipforward info */
#define	PACKET_TAG_MACLABEL	(19 | MTAG_PERSISTENT) /* MAC label */
#define	PACKET_TAG_PF				21 /* PF + ALTQ information */
#define	PACKET_TAG_RTSOCKFAM			25 /* rtsock sa family */
#define	PACKET_TAG_IPOPTIONS			27 /* Saved IP options */
#define	PACKET_TAG_CARP                         28 /* CARP info */

#endif /* !_SYS_MBUF_H_ */
