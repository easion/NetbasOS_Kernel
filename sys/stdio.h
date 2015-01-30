#ifndef STDIO_H
#define STDIO_H



#define NULL_FCB  (FILE *) 0

struct file_minix {
	unsigned short f_mode;
	unsigned short f_flags;
	unsigned short f_count;
	struct inode * f_inode;
	off_t f_pos;
};


struct file2 {
	unsigned short f_mode;
	unsigned short f_flags;
	unsigned short f_count;
	struct inode * f_inode;
	off_t f_pos;
};

struct file_mem
{
	   unsigned char     *buf;
	   unsigned int     sz;
};



typedef struct filp
{
       unsigned short        f_mode; ///读还是写
       unsigned short        f_flags; ///读还是写
       unsigned short        f_count; ///
	   unsigned long     current_pos;
	   struct inode         *f_inode;
	   //unsigned short        f_sys_nr;
	const struct _ops *ops;
}FILE;

typedef struct _ops
{
	int (*close)(FILE *file);
	int (*write)(FILE *file, unsigned char *buf, unsigned len);
	int (*read)(FILE *file, unsigned char *buf, unsigned want);
	int (*select)(FILE *file, unsigned access, unsigned *timeout);
} ops_t;

 unsigned char fgetc(FILE* fp);


#endif

