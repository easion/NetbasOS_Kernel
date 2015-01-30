/*
**     (R)Jicama OS
**     Multi File system Main Function
**     Copyright (C) 2003 DengPingPing
*/
#include <jicama/system.h>
#include <jicama/fs.h>
#include <string.h>
#include <ansi.h>
#include <assert.h>

extern char *init_cmd;

/*zero filp struct pool*/
struct filp raw_file[RAW_FILE_NR];

/*
**
*/
__public int filp_init(void)
{
	int i;

	for (i=0; i<RAW_FILE_NR; i++)
	{
		bzero(&raw_file[i], sizeof(struct filp));
	}

	return 0;
}


static int raw_write(file_t *f, const void *buf, int len);
static int raw_read(file_t *f, void *buf, int len);
static int raw_flush(struct filp *fp);
static int raw_open(struct filp *fp);

/*
**
*/
__public struct filp* kfs_do_open(char *p, char *mode)
{
	int i,j;
	//char *arg[16];
	//char _file[128];	
	
	if(!p)return NIL_FLP;
	
	 for (i=0; i<RAW_FILE_NR; i++){
		 if(!raw_file[i].name[0])continue;		
		 if(!stricmp((char *)raw_file[i].name, p)){;
		trace("file %s find here\n", p);
		
		if(raw_file[i].open_kfs)
			raw_file[i].open_kfs(&raw_file[i]);
			
		return &raw_file[i];
		}
	 }
	
	trace("file %s not find here\n", p);
	return NIL_FLP;
}

/*
**
*/
__public struct filp* kfs_slot()
{
	int i;
	 for (i=0; i<RAW_FILE_NR; i++)
		 if(!raw_file[i].name[0])break;

	 if(RAW_FILE_NR==i)return NIL_FLP;

	 memset(&raw_file[i],0,sizeof(file_t));
	 raw_file[i].fd = (int)(&raw_file[i]-raw_file)+MODFS_STARTFD;
	 return &raw_file[i];
}

__public struct filp* modfs_alloc()
{
	file_t *f=kmalloc(sizeof(file_t),0);
	memset(f,0,sizeof(file_t));
	return f;
}
/*
**
*/
__public struct filp* modfs_add(const char *filename, char *data, int sz)
{
	int i=0;
	file_t *f=kfs_slot();

	 if(!f){
		 kprintf("modfs_add error0\n");
		 return NIL_FLP;
	 }

	 //f->name = filename;
	 memset( f->name,0, FPATH_LEN);
	 while (filename[i]&&!isspace(filename[i]))
	 {
		 i++;
	 }

	 if (i==0 || i > FPATH_LEN)
	 {
		 kprintf("modfs_add error1\n");
		 return NULL;
	 }

	 strncpy( f->name, filename, i);

	 f->data = data;
	 f->size = sz;
	 f->readpos = 0;
	 f->writepos = 0;
	f->attr = DLL_FILE;

	f->open_kfs = &raw_open;
	f->close_kfs = &fclose;
	f->read_kfs = &raw_read;
	f->write_kfs = &raw_write;
	f->flush_kfs = &raw_flush;
	f->seek_kfs = &fseek;
	 //kprintf("mod%d - %d\n", i, f->fd);
	 return f;
}

/*
**
*/
__public void dump_all_module_file()
{
	int i;
	 for (i=0; i<RAW_FILE_NR; i++){
		 if(!raw_file[i].name)continue;
		 kprintf("%s\n", raw_file[i].name);
	 }
	 kprintf("INIT=%s\n", init_cmd);
}

/*
**
*/
__public struct filp* modfs_open(const char *p, char *mode)
{
	return kfs_do_open(p, mode);
}

/*
**
*/
__local int raw_open(struct filp *fp)
{
	ASSERT(fp != NULL);
	fp->readpos = 0;
	fp->writepos = 0;
	return 0;
}

/*
**
*/
__local int raw_flush(struct filp *fp)
{
	ASSERT(fp != NULL);
	fp->readpos = 0;
	fp->writepos = 0;
	return 0;
}

/*
**
*/
__public struct filp* server_open(const char *name, char *mode)
{
	int fd;
	struct filp*fp;

	fd = vfs_open(name, O_RDONLY,0);
	trace("server open fd=%d\n", fd);

	if (fd<0)	{return NIL_FLP;	}

	fp = modfs_alloc(name);

	if (fp)
		fp->fd=fd;
	else {
		vfs_close(fd);
		return NIL_FLP;
	}

	return fp;
}

/*
**
*/
__public void rawfs_init()
{
	int i;
	for (i=0; i<RAW_FILE_NR; i++)
	{
		memset(&raw_file, 0, sizeof(struct filp));
	}
}


/*
**
*/
__public int fseek(struct filp *fp, off_t offset, int where)
{
	int fsz=fp->size;
	int readpos=fp->readpos;
	if(!fp)return EINVAL;

	ASSERT(fp != NULL);

	if (!ismodfsfd(fp))
	{
		return lseek(fp->fd, offset, where);
	}

	switch (where)
	{
	case SEEK_SET:
		    if (offset < 0)  return EINVAL;
			 readpos = offset;
			break;
	case SEEK_CUR:
		    if (readpos + offset < 0)return EINVAL;
			readpos += offset;
			break;	
	case SEEK_END:
		 if ((readpos = fsz + offset) < 0)	return EINVAL;
	       readpos = fsz+ offset;
			break;
		default:
		break;
	}

	if(readpos<0)return -1;
	fp->readpos=readpos;
	return readpos;
}

/*
**
*/
__public u32_t modfs_chbase(struct filp *fp, char *_base)
{
	int szpos=(int)_base- (int)fp->data;

	ASSERT(fp != NULL);

	 fp->data = _base;
	 fp->size -= szpos;
	 fp->readpos = 0;
	 return (u32_t)_base;
}



/*
**
*/
__public void raw_free(struct filp* p)
{
	ASSERT(p != NULL);

	if(p)
		bzero((u8_t *)p, sizeof(struct filp));
}

/*
**
*/
__public void fclose(struct filp* p)
{
	ASSERT(p != NULL);

	if (!ismodfsfd(p))
	{
		//panic("fclose %d error!", p->fd);
		vfs_close(p->fd);
		kfree(p);
		return;
	}
	 p->readpos = 0;
}


/*
**
*/
__public int modfs_getc(struct filp *p)
{
	u8_t c;
	int readpos=p->readpos;

	ASSERT(p != NULL);

		if(!p || p->readpos >= (int)p->size)
			return EOF;

		c=(char)p->data[readpos];
		p->readpos++;
		return c;
}

/*
**
*/
__public int flength(struct filp *p)
{
	long cur, result;
	int stream;

	ASSERT(p != NULL);

	if (ismodfsfd(p)){
		return p->size;
	}
	stream=p->fd;
	cur = lseek(stream, (off_t)0, SEEK_CUR);
	result = lseek(stream, (off_t)0, SEEK_END);
	lseek(stream, (off_t)cur, SEEK_SET);
	return result;
}


/*
**
*/
__public int modfs_putc(int c, struct filp *p)
{
	int writepos=p->writepos;

	ASSERT(p != NULL);

	if(!p || p->writepos >= (int)p->size)
		return EOF;

	p->data[writepos]=c;
	p->writepos++;
	return c;
}

/*
**
*/
__public char *modfs_fgets(char *s, register int n, register struct filp *stream)
{
	register int ch=0;
	register char *ptr;

	ASSERT(stream != NULL);

	if (s == NULL)
	{
		return NULL;
	}

	ptr = s;
	while (--n > 0 && (ch = modfs_getc(stream)) != EOF) {
		*ptr++ = ch;
		if ( ch == '\n')
			break;
	}
	if (ch == EOF) {
		if (stream->readpos >= (int)stream->size) {
			if (ptr == s) return (char *)0;
		} 
		else return (char *)0;
	}
	*ptr = '\0';
	return s;
}


/*
**
*/
size_t fread(void *ptr, size_t size, size_t nmemb, register struct filp *stream)
{
	register char *cp = (char *)ptr;
	register size_t c;
	size_t ndone = 0;
	register size_t s;

	ASSERT(stream != NULL);

	if (!ismodfsfd(stream))
	{
		c=size*nmemb;
		ndone= vfs_read(stream->fd,  ptr, c,0);
		if(c!=ndone)trace("warn: read no buffer size(%i)!\n", ndone);
		return c;
	}

	if (size)
		while ( ndone < nmemb ) {
			s = size;
			do {
				if ((c = modfs_getc(stream)) != EOF){
					ndone++;
					*cp++ = c;
				}
				else
					return ndone;
			} while (--s);
		}

	return ndone;
}

/*
**
*/
__public size_t fwrite(const void *ptr, size_t size, size_t nmemb,
	    register struct filp *stream)
{
	register const unsigned char *cp = (const unsigned char *)ptr;
	register size_t s;
	size_t ndone = 0;

	ASSERT(stream != NULL);

	if (!ismodfsfd(stream))
	{
		return vfs_write(stream->fd,  (void*)ptr,size*nmemb,0);
	}

	if (size)
		while ( ndone < nmemb ) {
			s = size;
			do {
				if (modfs_putc((int)*cp, stream)
					== EOF)
					return ndone;
				cp++;
			} 
			while (--s);
			ndone++;
		}
	return ndone;
}


/*
**
*/
__local int raw_write(file_t *f, const void *buf, int len)
{
	return fwrite(buf, len,1,f);
}

/*
**
*/
__local int raw_read(file_t *f, void *buf, int len)
{
	return fread(buf, len,1,f);
}


