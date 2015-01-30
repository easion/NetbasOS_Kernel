
#include <jicama/system.h>
#include <jicama/fs.h>
#include <string.h>
#include <errno.h>

#define SFIFO_SIZEMASK(x)	((x)->size - 1)
//已经使用的字节数
#define fifo_used(x)	(((x)->writepos - (x)->readpos) & SFIFO_SIZEMASK(x))
//剩余
#define fifo_space(x)	((x)->size - 1 - fifo_used(x))
#define DBG(x)	/*(x)*/
__local void fifo_close(file_t *f);
__local void fifo_flush(file_t *f);
int fifo_write(file_t *f, const void *buf, int len);
int fifo_read(file_t *f, void *buf, int len);
static void fifo_open(file_t *f);

/*
**
*/
__public file_t* fifo_init(const char *name, int size)
{
	file_t *f=kfs_slot();

	if (!f)
	{
		return NIL_FLP;
	}

	f->size = 1;
	for(; f->size <= size; f->size <<= 1)
		;

	/* Get buffer */
	if( 0 == (f->data = kcalloc(f->size)) )
		return -ENOMEM;
		
	strncpy(f->name, name, FPATH_LEN);

	f->attr = FIFO_FILE;
	f->open_kfs = &fifo_open;
	f->close_kfs = &fifo_close;
	f->read_kfs = &fifo_read;
	f->write_kfs = &fifo_write;
	f->flush_kfs = &fifo_flush;

	return f;
}

/*
**
*/
__public void fifo_close(file_t *f)
{
	if(f->data)
		mm_free(f->data, f->size);
}

/*
**
*/
__local void fifo_open(file_t *f)
{
	 f->readpos = 0;
	 f->writepos = 0;
}

/*
**
*/
__local void fifo_flush(file_t *f)
{
	f->readpos = 0;
	f->writepos = 0;
}

/*
**
*/
__public int fifo_write(file_t *f, const void *_buf, int len)
{
	int total;
	int i;
	const char *buf = (const char *)_buf;

	if(!f->data)
		return -ENODEV;	/* No data! */

	/* total = len = min(space, len) */
	total = fifo_space(f);

	if(len > total)
		len = total;
	else
		total = len;

	i = f->writepos;
	if(i + len > f->size)
	{
		//超过实际大小
		memcpy(f->data + i, buf, f->size - i);
		//到达顶部
		buf += f->size - i;
		len -= f->size - i;
		//缓冲全部满了
		i = 0;
	}
	//在缓冲区大小之内
	memcpy(f->data + i, buf, len);
	f->writepos = i + len;

	return total;
}


/*
**
*/
__public int fifo_read(file_t *f, void *_buf, int len)
{
	int total;
	int i;
	char *buf = (char *)_buf;

	if(!f->data)
		return -ENODEV;	/* No data! */

	/* total = len = min(used, len) */
	total = fifo_used(f);
	DBG(kprintf("fifo_used() = %d\n",total));
	if(len > total)
		len = total;
	else
		total = len;

	i = f->readpos;
	//要读的内容大于实际大小
	if(i + len > f->size)
	{
		//拷贝剩余的字节
		memcpy(buf, f->data + i, f->size - i);
		buf += f->size - i;
		len -= f->size - i;
		i = 0;
	}
	memcpy(buf, f->data + i, len);
	f->readpos = i + len;

	return total;
}


