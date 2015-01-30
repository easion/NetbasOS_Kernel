

#ifndef __XCONSOLE_H__
#define __XCONSOLE_H__


typedef struct {
	char in[1024];
	char out[2048];
    u32_t in_cons;
	u32_t in_prod;
    u32_t out_cons;
	u32_t out_prod;
} xencons_t;

extern xencons_t console_page;

extern void xen_console_init(void);

#endif

/** @}
 */
