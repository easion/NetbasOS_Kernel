#include <jicama/system.h>
#include <assert.h>
#include <string.h>


bool is_gzip_format_file(u8_t *magic)
{
    if (magic[0] != 037 ||
	((magic[1] != 0213) && (magic[1] != 0236))) {
		trace("no gzip format %x - %x\n", magic[0],magic[1]);
	    return false;
    }
	return true;
}

size_t gzip_orig_length(u8_t *magic, int len)
{
    size_t *size = (size_t *)&magic[len-4];
    /*orig_len = (ulg) get_byte();
    orig_len |= (ulg) get_byte() << 8;
    orig_len |= (ulg) get_byte() << 16;
    orig_len |= (ulg) get_byte() << 24;*/
	//kprintf("gzip file size is %d\n",*size);
	return (*size);
}


#define malloc kcalloc
#define free(p) mm_free((p), sizeof(sizeof(struct huft)))
#define memzero bzero


/*
 * gzip declarations
 */

#define OF(args)  args
#define STATIC static


typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define WSIZE 0x8000		/* Window size must be at least 32k, */
				/* and a power of two */

static uch *inbuf;	     /* input buffer */
static uch window[WSIZE];    /* Sliding window buffer */

static unsigned insize = 0;  /* valid bytes in inbuf */
static unsigned inptr = 0;   /* index of next byte to be processed in inbuf */
static unsigned outcnt = 0;  /* bytes in output buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ASCII text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

void gzip_ran_init(void)
{
	insize = 0;
	inptr = 0;
	outcnt = 0;
}

/* Diagnostic functions */

#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)

static int  fill_inbuf(void);
static void flush_window(void);
static void error(char *m);

static void gzip_mark(void **ptr)
{
}
static void gzip_release(void **ptr)
{
}
  
static long bytes_out;
static uch *output_data;
static const uint8_t *input_data;
static int input_len;

#include "inflate.c"

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
static int fill_inbuf(void)
{
	if (insize != 0) {
		error("ran out of input data\n");
	}

	inbuf = (uint8_t *)input_data;
	insize = input_len;
	inptr = 1;
	return inbuf[0];
}

static void flush_window(void)
{
    memcpy(output_data, window, outcnt);
    output_data += outcnt;
    bytes_out += outcnt;
    outcnt = 0;
}

static void error(char *x)
{
    kprintf("%s", x);
}

int do_gunzip(uint8_t *dest, const uint8_t *src, int src_len)
{
    input_data = src;
    input_len = src_len;
    output_data = dest;
    bytes_out = 0;
    gunzip();
    return bytes_out;
}

int gzip_file(const char *inbuf,const int inlen, char **outbuf, int *len)
{
	char *dest;
	int bytes;
	size_t length = gzip_orig_length(inbuf,inlen);

	ASSERT(inbuf);
	ASSERT(outbuf);

	if (length == 0)
	{
		return -1;
	}

	gzip_ran_init();

	//kprintf("gzip file len %d\n", length);
	dest = (void *)kcalloc(length);
	bytes = do_gunzip(dest, inbuf, inlen);
	if (bytes<=0)
	{
		return -2;
	}
	
	*outbuf = dest;
	*len = length;
	return 0;
}

void gzip_test(void )
{
	static uint8_t agzipfile[]={0x1f, 0x8b, 0x8, 0x8, 0x7c, 0x3a, 0x9c, 0x43, 0x0, 0xb, 0x6b, 0x2e, 0x74, 0x78, 0x74, 0x0, 0x2b, 0xc9, 0xc8, 0x2c, 0x56, 0x0, 0xa2, 0x44, 0x85, 0xf4, 0xaa, 0xcc, 0x2, 0x85, 0x92, 0xd4, 0x8a, 0x12, 0x85, 0xb4, 0xcc, 0x9c, 0x54, 0x45, 0x5e, 0xae, 0x8c, 0xd4, 0x9c, 0x9c, 0x7c, 0x85, 0xf2, 0xfc, 0xa2, 0x9c, 0x14, 0x20, 0x8f, 0x97, 0xab, 0x84, 0x78, 0xa5, 0x0, 0x29, 0xc9, 0x79, 0x66, 0x56, 0x0, 0x0, 0x0};
	int len;
	char buf[512];

	//return ;

	memset(buf, 0,512);
	len = do_gunzip(buf, agzipfile, sizeof(agzipfile));

	kprintf("buf:%s", buf);
	kprintf("length %d\n", len);

	if (len==0)
	{
		kprintf("bad magic: %o %o\n",agzipfile[0], agzipfile[1]);
	}

	//panic("");

}



