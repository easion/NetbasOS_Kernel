#ifndef _IP_DIVERT_H
#define _IP_DIVERT_H

struct mbuf;
typedef void ip_divert_packet_t(struct mbuf *, int);

extern  ip_divert_packet_t *ip_divert_ptr;

struct divert_tag {
        u_int32_t       info;           /* port & flags */
        u_int16_t       cookie;         /* ipfw rule number */
};

#endif /* !_IP_DIVERT_H */
