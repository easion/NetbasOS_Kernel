

#include <net/net.h>

err_t icmp_input(struct pbuf *p, struct netif *inp)
{
  unsigned char type;
  unsigned char code;
  struct icmp_echo_hdr *iecho;
  struct icmp_dur_hdr *idur;
  struct ip_hdr *iphdr;
  struct ip_addr tmpaddr;
  int hlen;
  
  stats.icmp.recv++;

  iphdr = p->payload;
  hlen = IPH_HL(iphdr) * 4;
  
  if (pbuf_header(p, -hlen) < 0 || p->tot_len < sizeof(unsigned short) * 2)
  {
    kprintf("icmp_input: short ICMP (%u bytes) received\n", p->tot_len);
    stats.icmp.lenerr++;
    return EPROTO;
  }

  type = *((unsigned char *) p->payload);

  //kprintf("icmp: recv type %d\n", type);

  switch (type) 
  {
    case ICMP_ECHO:
      if (ip_addr_isbroadcast(&iphdr->dest, &inp->netmask) 
		|| ip_addr_ismulticast(&iphdr->dest))
      {
	stats.icmp.err++;
	return EPROTO;
      }
    
      if (!ip_ownaddr(&iphdr->dest))
      {
	stats.icmp.err++;
	return EPROTO;
      }

      //kprintf("icmp_input: ping src %a dest %a\n", &iphdr->src, &iphdr->dest);

      if (p->tot_len < sizeof(struct icmp_echo_hdr)) 
      {
	kprintf("icmp_input: bad ICMP echo received\n");
	stats.icmp.lenerr++;
	return EPROTO;
      }

      iecho = p->payload;
      if (inet_chksum_pbuf(p) != 0) 
      {
	kprintf("icmp_input: checksum failed for received ICMP echo\n");
	stats.icmp.chkerr++;
	return EFAULT;
      }

      tmpaddr.addr = iphdr->src.addr;
      iphdr->src.addr = iphdr->dest.addr;
      iphdr->dest.addr = tmpaddr.addr;
      ICMPH_TYPE_SET(iecho, ICMP_ER);
      
      // Adjust the checksum
      if (iecho->chksum >= htons(0xFFFF - (ICMP_ECHO << 8))) 
	iecho->chksum += htons(ICMP_ECHO << 8) + 1;
      else
	iecho->chksum += htons(ICMP_ECHO << 8);

      stats.icmp.xmit++;
      
      pbuf_header(p, hlen);
      return ip_output_if(p, &iphdr->src, IP_HDRINCL, IPH_TTL(iphdr), IP_PROTO_ICMP, inp);

    case ICMP_DUR:
      if (p->tot_len < ICMP_HLEN)
      {
	kprintf("icmp_input: ICMP message too short\n");
	stats.icmp.lenerr++;
	return EPROTO;
      }

      idur = (struct icmp_dur_hdr *) p->payload;
      code = ICMPH_CODE(idur);
      pbuf_header(p, -ICMP_HLEN);
      return ip_input_dur(code, p);

    default:
      kprintf("icmp_input: ICMP type %d not supported\n", type);
      stats.icmp.proterr++;
      stats.icmp.drop++;
      return EPROTO;
  }

  pbuf_free(p);
  return 0;
}

void icmp_dest_unreach(struct pbuf *p, int t)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_dur_hdr *idur;
  
  // ICMP header + IP header + 8 bytes of data
  q = pbuf_alloc(PBUF_IP, 8 + IP_HLEN + 8, PBUF_RW);
  if (!q) return;

  iphdr = p->payload;
  
  idur = q->payload;
  ICMPH_TYPE_SET(idur, ICMP_DUR);
  ICMPH_CODE_SET(idur, t);

  memcpy((char *) q->payload + 8, p->payload, IP_HLEN + 8);
  
  // Calculate checksum
  idur->chksum = 0;
  idur->chksum = inet_chksum(idur, q->len);
  stats.icmp.xmit++;

  if (ip_output(q, NULL, &iphdr->src, ICMP_TTL, IP_PROTO_ICMP) < 0) pbuf_free(q);
}

void icmp_time_exceeded(struct pbuf *p, int t)
{
  struct pbuf *q;
  struct ip_hdr *iphdr;
  struct icmp_te_hdr *tehdr;

  q = pbuf_alloc(PBUF_IP, 8 + IP_HLEN + 8, PBUF_RW);
  if (!q) return;

  iphdr = p->payload;
  tehdr = q->payload;
  ICMPH_TYPE_SET(tehdr, ICMP_TE);
  ICMPH_CODE_SET(tehdr, t);

  // Copy fields from original packet
  memcpy((char *) q->payload + 8, (char *) p->payload, IP_HLEN + 8);
  
  // Calculate checksum
  tehdr->chksum = 0;
  tehdr->chksum = inet_chksum(tehdr, q->len);
  stats.icmp.xmit++;

  if (ip_output(q, NULL, &iphdr->src, ICMP_TTL, IP_PROTO_ICMP) < 0) pbuf_free(q);
}
