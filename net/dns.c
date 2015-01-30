#include <net/net.h>
#include <net/dns.h>
#include <string.h>
#include <assert.h>
#ifndef LWIP_UNUSED_ARG
#define LWIP_UNUSED_ARG(x) (void)x
#endif /* LWIP_UNUSED_ARG */ 

#define DNS_DBG kprintf
//extern const struct ip_addr ip_addr_any;
//extern const struct ip_addr ip_addr_broadcast;

/** IP_ADDR_ can be used as a fixed IP address
 *  for the wildcard and the broadcast address
 */
#define IP_ADDR_ANY         ((struct ip_addr *)&ip_addr_any)
//#define IP_ADDR_BROADCAST   ((struct ip_addr *)&ip_addr_broadcast)


/** DNS server IP address */
#ifndef DNS_SERVER_ADDRESS
#define DNS_SERVER_ADDRESS        inet_addr("208.67.222.222") /* resolver1.opendns.com */
#endif

/** DNS server port address */
#ifndef DNS_SERVER_PORT
#define DNS_SERVER_PORT           53
#endif

/** DNS maximum number of retries when asking for a name, before "timeout". */
#ifndef DNS_MAX_RETRIES
#define DNS_MAX_RETRIES           4
#endif

/** DNS resource record max. TTL (one week as default) */
#ifndef DNS_MAX_TTL
#define DNS_MAX_TTL               604800
#endif

/* DNS protocol flags */
#define DNS_FLAG1_RESPONSE        0x80
#define DNS_FLAG1_OPCODE_STATUS   0x10
#define DNS_FLAG1_OPCODE_INVERSE  0x08
#define DNS_FLAG1_OPCODE_STANDARD 0x00
#define DNS_FLAG1_AUTHORATIVE     0x04
#define DNS_FLAG1_TRUNC           0x02
#define DNS_FLAG1_RD              0x01
#define DNS_FLAG2_RA              0x80
#define DNS_FLAG2_ERR_MASK        0x0f
#define DNS_FLAG2_ERR_NONE        0x00
#define DNS_FLAG2_ERR_NAME        0x03

/* DNS protocol states */
#define DNS_STATE_UNUSED          0
#define DNS_STATE_NEW             1
#define DNS_STATE_ASKING          2
#define DNS_STATE_DONE            3


/** DNS message header */
struct dns_hdr {
  u16_t id;
  u8_t flags1;
  u8_t flags2;
  u16_t numquestions;
  u16_t numanswers;
  u16_t numauthrr;
  u16_t numextrarr;
} __attr_packet;


/** DNS query message structure */
struct dns_query {
  /* DNS query record starts with either a domain name or a pointer
     to a name already present somewhere in the packet. */
  u16_t type;
  u16_t class;
} __attr_packet;



/** DNS answer message structure */
struct dns_answer {
  /* DNS answer record starts with either a domain name or a pointer
     to a name already present somewhere in the packet. */
  u16_t type;
  u16_t class;
  u32_t ttl;
  u16_t len;
} __attr_packet;


/** DNS table entry */
struct dns_table_entry {
  u8_t  state;
  u8_t  numdns;
  u8_t  tmr;
  u8_t  retries;
  u8_t  seqno;
  u8_t  err;
  u32_t ttl;
  char name[DNS_MAX_NAME_LENGTH];
  struct ip_addr ipaddr;
  /* pointer to callback on DNS query done */
  dns_found_callback found;
  void *arg;
};


/* forward declarations */
static void dns_recv(void *s, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port);
static void dns_check_entries(void);

/*-----------------------------------------------------------------------------
 * Globales
 *----------------------------------------------------------------------------*/

/* DNS variables */
static struct udp_pcb        *dns_pcb;
static u8_t                   dns_seqno;
static struct dns_table_entry dns_table[DNS_TABLE_SIZE];
static struct ip_addr         dns_servers[DNS_MAX_SERVERS];

static u8_t                   dns_payload[DNS_MSG_SIZE];

static void dns_timer(void *arg);
/** DNS timer period */
#define DNS_TMR_INTERVAL          1000

//struct timer dnstimer;


/**
 * Initialize the resolver: set up the UDP pcb and configure the default server
 * (DNS_SERVER_ADDRESS).
 */
void
dns_init()
{
  struct ip_addr dnsserver;
  
  /* initialize default DNS server address */
  dnsserver.addr = DNS_SERVER_ADDRESS;

  DNS_DBG ("dns_init: initializing\n");

  /* if dns client not yet initialized... */
  if (dns_pcb == NULL) {
    dns_pcb = udp_new();

    if (dns_pcb != NULL) {
      /* initialize DNS table not needed (initialized to zero since it is a
       * global variable) */
      //LWIP_ASSERT("For implicit initialization to work, DNS_STATE_UNUSED needs to be 0",
        ASSERT(DNS_STATE_UNUSED == 0);

      /* initialize DNS client */
      udp_bind(dns_pcb, IP_ADDR_ANY, 0);
      udp_recv(dns_pcb, dns_recv, NULL);

      /* initialize default DNS primary server */
      dns_setserver(0, &dnsserver);
    }
  }
}

/**
 * Initialize one of the DNS servers.
 *
 * @param numdns the index of the DNS server to set must be < DNS_MAX_SERVERS
 * @param dnsserver IP address of the DNS server to set
 */
void
dns_setserver(u8_t numdns, struct ip_addr *dnsserver)
{
  if ((numdns < DNS_MAX_SERVERS) && (dns_pcb != NULL) &&
      (dnsserver != NULL) && (dnsserver->addr !=0 )) {
    dns_servers[numdns] = (*dnsserver);
  }
}

/**
 * Obtain one of the currently configured DNS server.
 *
 * @param numdns the index of the DNS server
 * @return IP address of the indexed DNS server or "ip_addr_any" if the DNS
 *         server has not been configured.
 */
struct ip_addr
dns_getserver(u8_t numdns)
{
  if (numdns < DNS_MAX_SERVERS) {
    return dns_servers[numdns];
  } else {
    return *IP_ADDR_ANY;
  }
}

/**
 * The DNS resolver client timer - handle retries and timeouts and should
 * be called every DNS_TMR_INTERVAL milliseconds (every second by default).
 */
void
dns_tmr(void)
{
  if (dns_pcb != NULL) {
    DNS_DBG ("dns_tmr: dns_check_entries\n");
    dns_check_entries();
  }
}


static void dns_timer(void *arg)
{
  LWIP_UNUSED_ARG(arg);
  //LWIP_DEBUGF(TCPIP_DEBUG, ("tcpip: dns_tmr()\n"));
  dns_tmr();
  sys_timeout(DNS_TMR_INTERVAL, dns_timer, NULL);
}


/**
 * Look up a hostname in the array of known hostnames.
 *
 * @note This function only looks in the internal array of known
 * hostnames, it does not send out a query for the hostname if none
 * was found. The function dns_enqueue() can be used to send a query
 * for a hostname.
 *
 * @param name the hostname to look up
 * @return the hostname's IP address, as u32_t (instead of struct ip_addr to
 *         better check for failure: != 0) or 0 if the hostname was not found
 *         in the cached dns_table.
 */
static u32_t
dns_lookup(const char *name)
{
  u8_t i;

  /* Walk through name list, return entry if found. If not, return NULL. */
  for (i = 0; i < DNS_TABLE_SIZE; ++i) {
    if ((dns_table[i].state == DNS_STATE_DONE) &&
        (strcmp(name, dns_table[i].name) == 0)) {
      DNS_DBG ("dns_lookup: \"%s\": found = ", name);
      ip_addr_debug_print( &(dns_table[i].ipaddr));
      DNS_DBG ("\n");
      return dns_table[i].ipaddr.addr;
    }
  }

  return 0;
}

#if DNS_DOES_NAME_CHECK
/**
 * Compare the "dotted" name "query" with the encoded name "response"
 * to make sure an answer from the DNS server matches the current dns_table
 * entry (otherwise, answers might arrive late for hostname not on the list
 * any more).
 *
 * @param query hostname (not encoded) from the dns_table
 * @param response encoded hostname in the DNS response
 * @return 0: names equal; 1: names differ
 */
static u8_t
dns_compare_name(unsigned char *query, unsigned char *response)
{
  unsigned char n;

  do {
    n = *response++;
    /** @see RFC 1035 - 4.1.4. Message compression */
    if ((n & 0xc0) == 0xc0) {
      /* Compressed name */
      break;
    } else {
      /* Not compressed name */
      while (n > 0) {
        if ((*query) != (*response)) {
          return 1;
        }
        ++response;
        ++query;
        --n;
      };
      ++query;
    }
  } while (*response != 0);

  return 0;
}
#endif /* DNS_DOES_NAME_CHECK */

/**
 * Walk through a compact encoded DNS name and return the end of the name.
 *
 * @param query encoded DNS name in the DNS server response
 * @return end of the name
 */
static unsigned char *
dns_parse_name(unsigned char *query)
{
  unsigned char n;

  do {
    n = *query++;
    /** @see RFC 1035 - 4.1.4. Message compression */
    if ((n & 0xc0) == 0xc0) {
      /* Compressed name */
      break;
    } else {
      /* Not compressed name */
      while (n > 0) {
        ++query;
        --n;
      };
    }
  } while (*query != 0);

  return query + 1;
}

/**
 * Send a DNS query packet.
 *
 * @param numdns index of the DNS server in the dns_servers table
 * @param name hostname to query
 * @param id index of the hostname in dns_table, used as transaction ID in the
 *        DNS query packet
 * @return 0 if packet is sent; an err_t indicating the problem otherwise
 */
static err_t
dns_send(u8_t numdns, const char* name, u8_t id)
{
  err_t err;
  struct dns_hdr *hdr;
  struct dns_query qry;
  struct pbuf *p;
  char *query, *nptr;
  const char *pHostname;
  u8_t n;

  DNS_DBG ("dns_send: dns_servers[%d] \"%s\": request\n",
              (u16_t)(numdns), name);
  ASSERT( numdns < DNS_MAX_SERVERS);
  ASSERT( dns_servers[numdns].addr != 0);

  /* if here, we have either a new query or a retry on a previous query to process */
  p = pbuf_alloc(PBUF_TRANSPORT, sizeof(struct dns_hdr) + DNS_MAX_NAME_LENGTH +
                 sizeof(struct dns_query), PBUF_RW);
  if (p != NULL) {
    ASSERT(p->next == NULL);
    /* fill dns header */
    hdr = (struct dns_hdr*)p->payload;
    memset(hdr, 0, sizeof(struct dns_hdr));
    hdr->id = htons(id);
    hdr->flags1 = DNS_FLAG1_RD;
    hdr->numquestions = htons(1);
    query = (char*)hdr + sizeof(struct dns_hdr);
    pHostname = name;
    --pHostname;

    /* convert hostname into suitable query format. */
    do {
      ++pHostname;
      nptr = query;
      ++query;
      for(n = 0; *pHostname != '.' && *pHostname != 0; ++pHostname) {
        *query = *pHostname;
        ++query;
        ++n;
      }
      *nptr = n;
    } while(*pHostname != 0);
    *query++='\0';

    /* fill dns query */
    qry.type  = htons(DNS_RRTYPE_A);
    qry.class = htons(DNS_RRCLASS_IN);
    memcpy( query, &qry, sizeof(struct dns_query));

    /* resize pbuf to the exact dns query */
    pbuf_realloc(p, (query + sizeof(struct dns_query)) - ((char*)(p->payload)));

    /* connect to the server for faster receiving */
    udp_connect(dns_pcb, &dns_servers[numdns], DNS_SERVER_PORT);
    /* send dns packet */
    err = udp_sendto(dns_pcb, p, &dns_servers[numdns], DNS_SERVER_PORT);

    /* free pbuf */
    pbuf_free(p);
  } else {
    err = ENOMEM;
  }

  return err;
}

/**
 * dns_check_entry() - see if pEntry has not yet been queried and, if so, sends out a query.
 * Check an entry in the dns_table:
 * - send out query for new entries
 * - retry old pending entries on timeout (also with different servers)
 * - remove completed entries from the table if their TTL has expired
 *
 * @param i index of the dns_table entry to check
 */
static void
dns_check_entry(u8_t i)
{
  struct dns_table_entry *pEntry = &dns_table[i];

  ASSERT( i < DNS_TABLE_SIZE);

  switch(pEntry->state) {

    case DNS_STATE_NEW: {
      /* initialize new entry */
      pEntry->state   = DNS_STATE_ASKING;
      pEntry->numdns  = 0;
      pEntry->tmr     = 1;
      pEntry->retries = 0;
      
      /* send DNS packet for this entry */
      dns_send(pEntry->numdns, pEntry->name, i);
      break;
    }

    case DNS_STATE_ASKING: {
      if (--pEntry->tmr == 0) {
        if (++pEntry->retries == DNS_MAX_RETRIES) {
          if ((pEntry->numdns+1<DNS_MAX_SERVERS) && (dns_servers[pEntry->numdns+1].addr!=0)) {
            /* change of server */
            pEntry->numdns++;
            pEntry->tmr     = 1;
            pEntry->retries = 0;
            break;
          } else {
            DNS_DBG ("dns_check_entry: \"%s\": timeout\n", pEntry->name);
            /* call specified callback function if provided */
            if (pEntry->found)
              (*pEntry->found)(pEntry->name, NULL, pEntry->arg);
            /* flush this entry */
            pEntry->state   = DNS_STATE_UNUSED;
            pEntry->found   = NULL;
            break;
          }
        }

        /* wait longer for the next retry */
        pEntry->tmr = pEntry->retries;

        /* send DNS packet for this entry */
        dns_send(pEntry->numdns, pEntry->name, i);
      }
      break;
    }

    case DNS_STATE_DONE: {
      /* if the time to live is nul */
      if (--pEntry->ttl == 0) {
        DNS_DBG ("dns_check_entry: \"%s\": flush\n", pEntry->name);
        /* flush this entry */
        pEntry->state = DNS_STATE_UNUSED;
        pEntry->found = NULL;
      }
      break;
    }
    case DNS_STATE_UNUSED:
      /* nothing to do */
      break;
    default:
      ASSERT(0);
      break;
  }
}

/**
 * Call dns_check_entry for each entry in dns_table - check all entries.
 */
static void
dns_check_entries(void)
{
  u8_t i;

  for (i = 0; i < DNS_TABLE_SIZE; ++i) {
    dns_check_entry(i);
  }
}

/**
 * Receive input function for DNS response packets arriving for the dns UDP pcb.
 *
 * @params see udp.h
 */
static void
dns_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
  u8_t i;
  char *pHostname;
  struct dns_hdr *hdr;
  struct dns_answer ans;
  struct dns_table_entry *pEntry;
  u8_t nquestions, nanswers;

  LWIP_UNUSED_ARG(arg);
  LWIP_UNUSED_ARG(pcb);
  LWIP_UNUSED_ARG(addr);
  LWIP_UNUSED_ARG(port);

  /* is the dns message too big ? */
  if (p->tot_len > DNS_MSG_SIZE) {
    DNS_DBG ("dns_recv: pbuf too big\n");
    /* free pbuf and return */
    goto memerr1;
  }

  /* is the dns message big enough ? */
  if (p->tot_len < (sizeof(struct dns_hdr) + sizeof(struct dns_query) + sizeof(struct dns_answer))) {
    DNS_DBG ("dns_recv: pbuf too small\n");
    /* free pbuf and return */
    goto memerr1;
  }

  /* copy dns payload inside static buffer for processing */ 
  if (pbuf_copy_partial(p, dns_payload, p->tot_len, 0) == p->tot_len) {
    /* The ID in the DNS header should be our entry into the name table. */
    hdr = (struct dns_hdr*)dns_payload;
    i = htons(hdr->id);
    if (i < DNS_TABLE_SIZE) {
      pEntry = &dns_table[i];
      if(pEntry->state == DNS_STATE_ASKING) {
        /* This entry is now completed. */
        pEntry->state = DNS_STATE_DONE;
        pEntry->err   = hdr->flags2 & DNS_FLAG2_ERR_MASK;

        /* We only care about the question(s) and the answers. The authrr
           and the extrarr are simply discarded. */
        nquestions = htons(hdr->numquestions);
        nanswers   = htons(hdr->numanswers);

        /* Check for error. If so, call callback to inform. */
        if (((hdr->flags1 & DNS_FLAG1_RESPONSE) == 0) || (pEntry->err != 0) || (nquestions != 1)) {
          DNS_DBG ("dns_recv: \"%s\": error in flags\n", pEntry->name);
          /* call callback to indicate error, clean up memory and return */
          goto responseerr;
        }

#if DNS_DOES_NAME_CHECK
        /* Check if the name in the "question" part match with the name in the entry. */
        if (dns_compare_name((unsigned char *)(pEntry->name), (unsigned char *)dns_payload + sizeof(struct dns_hdr)) != 0) {
          DNS_DBG ("dns_recv: \"%s\": response not match to query\n", pEntry->name);
          /* call callback to indicate error, clean up memory and return */
          goto responseerr;
        }
#endif /* DNS_DOES_NAME_CHECK */

        /* Skip the name in the "question" part */
        pHostname = (char *) dns_parse_name((unsigned char *)dns_payload + sizeof(struct dns_hdr)) + sizeof(struct dns_query);

        while(nanswers > 0) {
          /* skip answer resource record's host name */
          pHostname = (char *) dns_parse_name((unsigned char *)pHostname);

          /* Check for IP address type and Internet class. Others are discarded. */
          memcpy(&ans, pHostname, sizeof(struct dns_answer));
          if((ntohs(ans.type) == DNS_RRTYPE_A) && (ntohs(ans.class) == DNS_RRCLASS_IN) && (ntohs(ans.len) == sizeof(struct ip_addr)) ) {
            /* read the answer resource record's TTL, and maximize it if needed */
            pEntry->ttl = ntohl(ans.ttl);
            if (pEntry->ttl > DNS_MAX_TTL) {
              pEntry->ttl = DNS_MAX_TTL;
            }
            /* read the IP address after answer resource record's header */
            memcpy( &(pEntry->ipaddr), (pHostname+sizeof(struct dns_answer)), sizeof(struct ip_addr));
            DNS_DBG ("dns_recv: \"%s\": response = ", pEntry->name);
            ip_addr_debug_print( &(pEntry->ipaddr));
            DNS_DBG ("\n");
            /* call specified callback function if provided */
            if (pEntry->found) {
              (*pEntry->found)(pEntry->name, &pEntry->ipaddr, pEntry->arg);
            }
            /* deallocate memory and return */
            goto memerr2;
          } else {
            pHostname = pHostname + sizeof(struct dns_answer) + htons(ans.len);
          }
          --nanswers;
        }
        DNS_DBG ("dns_recv: \"%s\": error in response\n", pEntry->name);
        /* call callback to indicate error, clean up memory and return */
        goto responseerr;
      }
    }
  }

  /* deallocate memory and return */
  goto memerr2;

responseerr:
  /* ERROR: call specified callback function with NULL as name to indicate an error */
  if (pEntry->found) {
    (*pEntry->found)(pEntry->name, NULL, pEntry->arg);
  }
  /* flush this entry */
  pEntry->state = DNS_STATE_UNUSED;
  pEntry->found = NULL;

memerr2:

memerr1:
  /* free pbuf */
  pbuf_free(p);
  return;
}

/**
 * Queues a new hostname to resolve and sends out a DNS query for that hostname
 *
 * @param name the hostname that is to be queried
 * @param found a callback founction to be called on success, failure or timeout
 * @param callback_arg argument to pass to the callback function
 * @return @return a err_t return code.
 */
static err_t
dns_enqueue(const char *name, dns_found_callback found, void *callback_arg)
{
  u8_t i;
  u8_t lseq, lseqi;
  struct dns_table_entry *pEntry = NULL;

  /* search an unused entry, or the oldest one */
  lseq = lseqi = 0;
  for (i = 0; i < DNS_TABLE_SIZE; ++i) {
    pEntry = &dns_table[i];
    /* is it an unused entry ? */
    if (pEntry->state == DNS_STATE_UNUSED)
      break;

    /* check if this is the oldest completed entry */
    if (pEntry->state == DNS_STATE_DONE) {
      if ((dns_seqno - pEntry->seqno) > lseq) {
        lseq = dns_seqno - pEntry->seqno;
        lseqi = i;
      }
    }
  }

  /* if we don't have found an unused entry, use the oldest completed one */
  if (i == DNS_TABLE_SIZE) {
    if ((lseqi >= DNS_TABLE_SIZE) || (dns_table[lseqi].state != DNS_STATE_DONE)) {
      /* no entry can't be used now, table is full */
      DNS_DBG ("dns_enqueue: \"%s\": DNS entries table is full\n", name);
      return ENOMEM;
    } else {
      /* use the oldest completed one */
      i = lseqi;
      pEntry = &dns_table[i];
    }
  }

  /* use this entry */
  DNS_DBG ("dns_enqueue: \"%s\": use DNS entry %d\n", name, (u16_t)(i));

  /* fill the entry */
  pEntry->state = DNS_STATE_NEW;
  pEntry->seqno = dns_seqno++;
  pEntry->found = found;
  pEntry->arg   = callback_arg;
  strcpy(pEntry->name, name);

  /* force to send query without waiting timer */
  dns_check_entry(i);

  /* dns query is enqueued */
  return 0;
}

/**
 * Resolve a hostname (string) into an IP address.
 * NON-BLOCKING callback version for use with raw API!!!
 *
 * Returns immediately with one of err_t return codes:
 * - 0 if hostname is a valid IP address string or the host
 *   name is already in the local names table.
 * - 0 enqueue a request to be sent to the DNS server
 *   for resolution if no errors are present.
 *
 * @param hostname the hostname that is to be queried
 * @param addr pointer to a struct ip_addr where to store the address if it is already
 *             cached in the dns_table (only valid if 0 is returned!)
 * @param found a callback function to be called on success, failure or timeout (only if
 *              0 is returned!)
 * @param callback_arg argument to pass to the callback function
 * @return a err_t return code.
 */
err_t
dns_gethostbyname(const char *hostname, struct ip_addr *addr, dns_found_callback found,
                  void *callback_arg)
{
  /* not initialized or no valid server yet, or invalid addr pointer
   * or invalid hostname or invalid hostname length */
  if ((dns_pcb == NULL) || (addr == NULL) ||
      (!hostname) || (!hostname[0]) ||
      (strlen(hostname) >= DNS_MAX_NAME_LENGTH)) {
    return EINVAL;
  }

  if (strcmp(hostname,"localhost")==0) {
    addr->addr = INADDR_LOOPBACK;
    return 0;
  }

  /* host name already in octet notation? set ip addr and return 0
   * already have this address cached? */
  if (((addr->addr = inet_addr(hostname)) != INADDR_NONE) ||
      ((addr->addr = dns_lookup(hostname)) != 0)) {
    return 0;
  }

  /* queue query with specified callback */
  return dns_enqueue(hostname, found, callback_arg);
}

#if 0
struct dns_api_msg {
  /** Hostname to query or dotted IP address string */
  const char *name;
  /** Rhe resolved address is stored here */
  struct ip_addr *addr;
  /** This semaphore is posted when the name is resolved, the application thread
      should wait on it. */
 // sys_sem_t sem;
  /** Errors are given back here */
  err_t *err;
};
static void
do_dns_found(const char *name, struct ip_addr *ipaddr, void *arg)
{
  struct dns_api_msg *msg = (struct dns_api_msg*)arg;

 // LWIP_ASSERT("DNS response for wrong host name", strcmp(msg->name, name) == 0);

  if (ipaddr == NULL) {
    /* timeout or memory error */
    *msg->err = EINVAL;
  } else {
    /* address was resolved */
    *msg->err = 0;
    *msg->addr = *ipaddr;
  }
  /* wake up the application task waiting in netconn_gethostbyname */
  //sys_sem_signal(msg->sem);
}

void
do_gethostbyname(void *arg)
{
	err_t err;
  //struct dns_api_msg *msg = (struct dns_api_msg*)arg;

  //*msg->err = dns_gethostbyname(msg->name, msg->addr, do_dns_found, msg);
  if (err != 0) {
    /* on error or immediate success, wake up the application
     * task waiting in netconn_gethostbyname */
    //sys_sem_signal(msg->sem);
  }
}
#endif
