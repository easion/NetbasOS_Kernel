#ifndef JICAMABUF_H
#define JICAMABUF_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_HASH 63
#define NR_BUFS  256

#define UNUSED 0
#define USED 1
#define NULL_BUF ((buffer_t *) 0)	
#define NO_DEV  	0

typedef struct buf_struct
{
	int b_dev;
	long b_block;        
	int b_count;
	short int b_dirt;     /* buf state */
	unsigned int b_size;
	//unsigned char b_data[BLOCK_SIZE];  /* point to the disk data in memory */
	unsigned char *b_data;  /* point to the disk data in memory */
	//struct buf_struct *b_next;
	//struct buf_struct *b_prev;
	//struct buf_struct *b_hash;
	TAILQ_ENTRY(buf_struct) next;
}buffer_t;


 //buffer_t *buf_hash[MAX_HASH];	/* the buffer hash table哈西表 */
extern  buffer_t *buf_head;	/* points to最少 least recently used free block */
extern  buffer_t *buf_tail;	/* points to 最多most recently used free block */
extern void buf_release(buffer_t* inst);


buffer_t* buf_getblk();
buffer_t* buf_find(int dev, unsigned int block);
bool buf_try_remove(buffer_t* bp);
buffer_t* bread_with_size(dev_prvi_t* dev, unsigned long block, int sz);
buffer_t* bread(dev_prvi_t* dev, unsigned long block);
void buf_release(buffer_t* bp);
void sync_blks();
void buffer_init();
void mark_buf_dirty(buffer_t *bp);


#ifdef __cplusplus
}
#endif

#endif

