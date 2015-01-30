

#ifndef __IF_TUN_H__
#define __IF_TUN_H__

/* Uncomment to enable debugging */
/* #define TUN_DEBUG 1 */



/* Number of devices */
#define TUN_MAX_DEV	255

/* TX queue size */
#define TUN_TXQ_SIZE	10

/* Max frame size */
#define TUN_MAX_FRAME	4096

/* TUN device flags */
#define TUN_FASYNC	0x0010
#define TUN_NOCHECKSUM	0x0020
#define TUN_NO_PI	0x0040

#define TUN_IFF_SET	0x1000



/* TUNSETIFF ifr flags */
#define IFF_TUN		0x0001
#define IFF_TAP		0x0002
#define IFF_NO_PI	0x1000



#define TUN_IOC_CREATE_IF (('T'<< 8) | 300) 
#define TUN_IOC_DELETE_IF  (('T'<< 8) | 301) 
#define FIONBIO    (('T'<< 8) | 302) 
#define TUN_IOC_IF_ADDR (('T'<< 8) | 303) 
#define TUN_IOC_IF_MODE (('T'<< 8) | 304) 


#endif /* __IF_TUN_H__ */
