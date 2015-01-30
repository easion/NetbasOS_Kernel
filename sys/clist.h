#ifndef _CLIST_H_
#define _CLIST_H_

enum{
	CLIST_NULL=1,
	CLIST_DEL,
	/*CLIST_LEFT,
	CLIST_RIGHT,
	CLIST_DOWN,
	CLIST_UP,*/
	CLIST_KEY
};

#define NCHARS_CLIST 255

struct cblock
{
	int start_pos;
	int end_pos;
	int cur_pos;
	long flags;
	unsigned char data[NCHARS_CLIST];
};

int init_clist(struct cblock *c);
int key_clist(int key, int flags);
int clist_input(struct cblock *c, int key, int fl);
int clist_done(struct cblock *c, unsigned char *buf , int event);


#endif


