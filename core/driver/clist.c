
#include <jicama/system.h> 
#include <jicama/process.h> 
#include <jicama/console.h>
#include <jicama/devices.h>
#include <jicama/spin.h>
#include <string.h>
#include <conio.h> 
#include <clist.h> 

/*****************************************************************************
returns 0 and sets 'data' if byte available,
else returns -1
*****************************************************************************/
int deq(key_queue_t *q, unsigned char *data)
{
/* if out_ptr reaches in_ptr, the queue is empty */
	if(q->out_ptr == q->in_ptr)
		return -1;

	*data = q->data[q->out_ptr++];

	if(q->out_ptr >= q->size)
		q->out_ptr = 0;

	return 0;
}

/*****************************************************************************
returns 0 if byte stored in queue,
else returns -1 if queue is full
*****************************************************************************/
int inq(key_queue_t *q, unsigned char data)
{
	unsigned i;

	i = q->in_ptr + 1;

	if(i >= q->size)
		i = 0;

	/* if in_ptr reaches out_ptr,
		the queue is full */
	if(i == q->out_ptr)
		return -1;

	q->data[q->in_ptr] = data;
	q->in_ptr = i;

	return 0;
}


/*be zero clist*/

int init_clist(struct cblock *c)
{
	c->start_pos =0;
	c->end_pos =0;
	c->cur_pos =0;
	c->flags =0;

	bzero(c->data, NCHARS_CLIST);
	return 0;
}

int clist_size(struct cblock *c)
{
	int sz = c->end_pos;	
	return sz;
}

/*conv key to event*/

int key_clist(int key, int flags)
{
	int event=CLIST_KEY; /*default value*/
	
	switch (key)
	{
		case '\b':
			event = CLIST_DEL;
			break;
		
		default:
			event =	CLIST_KEY;
			break;									
	}
	
	return event;		
}

#define TTY_JUST_GET_ONE 0x1000

/*input*/
int clist_input(struct cblock *c, int key, int flags)
{
	/*prase key*/
	int event = key_clist(key, flags);
	int movechars=0;
	int count = flags & 0x000000ff;
	int nkey=1;
	
	/*fix it*/
	c->start_pos=MAX(0, c->start_pos);
	c->cur_pos=MAX(0, c->cur_pos);
	c->end_pos=MAX(0,c->end_pos);

	c->start_pos=MIN(NCHARS_CLIST, c->start_pos);
	c->cur_pos=MIN(NCHARS_CLIST, c->cur_pos);
	c->end_pos=MIN(NCHARS_CLIST,c->end_pos);
			
	switch (event){
		case CLIST_DEL:
			//copy buffer
			movechars = -1;

			if (flags & TTY_JUST_GET_ONE)
			{
				c->data[c->cur_pos] = key;
				c->cur_pos++;
				c->end_pos++;
				//kprintf("\b");
			}else{
			c->cur_pos--;
			c->end_pos--;
			}
			break;
		
			
		case CLIST_KEY:
			if(c->cur_pos < c->end_pos){
				int pos = c->end_pos;
				//copy it
				while(pos > c->cur_pos){
					c->data[pos+1] = c->data[pos];
					pos--;
				}
				
			}else if(c->cur_pos > c->end_pos){
				c->cur_pos = c->end_pos;
			}
			nkey = vt100_conv (key, &c->data[c->cur_pos]);

			c->end_pos+=nkey;
			c->cur_pos+=nkey;
			movechars=1;
			break;
			
		default:
			break;
	}
	/*set lock_x2 value*/
	return movechars;
}


/*
**get clist data
*/
int clist_done(struct cblock *c, unsigned char *buf , int event)
{
	int len = c->end_pos - c->start_pos;
	
	memcpy(buf, c->data, len);
	return len;
}
