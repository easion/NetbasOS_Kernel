

#ifndef __fscntl_h__
#define __fscntl_h__
#ifdef __cplusplus
extern "C" {
#endif

#define EOF (-1)                /* End of file indicator */
/* open/fcntl - O_SYNC is only implemented on blocks devices and on files
   located on an ext2 file system */
#define O_ACCMODE	   0003
#define O_RDONLY	     00
#define O_WRONLY	     01
#define O_RDWR		     02

#define O_CREAT		   0100	/* not fcntl */
#define O_EXCL		   0200	/* not fcntl */
#define O_NOCTTY	   0400	/* not fcntl */
#define O_TRUNC		  01000	/* not fcntl */
#define O_APPEND	  02000
#define O_NONBLOCK	  04000
#define O_NDELAY	O_NONBLOCK
#define O_SYNC		 010000
#define O_FSYNC		 O_SYNC
#define O_ASYNC		 020000

/*============files.h constants=========*/
#define FILENAME_MAX	260

/*ftell, fseek constants*/
#define SEEK_SET	0
#define SEEK_CUR	1
#define SEEK_END	2


#define	_IOFBF		0x000
#define	_IOREAD		0x001
#define	_IOWRITE	0x002
#define	_IONBF		0x004
#define	_IOMYBUF	0x008
#define	_IOEOF		0x010
#define	_IOERR		0x020
#define	_IOLBF		0x040
#define	_IOREADING	0x080
#define	_IOWRITING	0x100
#define	_IOAPPEND	0x200

#define I_TYPE          0170000
#define I_DIRECTORY	0040000
#define I_REGULAR       0100000
#define I_BLOCK_SPECIAL 0060000
#define I_CHAR_SPECIAL  0020000
#define I_NAMED_PIPE	0010000
#define I_SET_UID_BIT   0004000
#define I_SET_GID_BIT   0002000

#define	S_IFMT	 0170000		/* type of file mask */
#define	S_IFIFO	 0010000		/* named pipe (fifo) */
#define	S_IFCHR	 0020000		/* character special */
#define	S_IFDIR	 0040000		/* directory */
#define	S_IFBLK	 0060000		/* block special */
#define	S_IFREG	 0100000		/* regular */
#define	S_IFLNK	 0120000		/* symbolic link */
#define	S_IFSOCK 0140000		/* socket */
#define	S_IFWHT  0160000		/* whiteout */
#define	S_ISVTX	 0001000		/* save swapped text even after use */


#define	S_ISDIR(m)	(((m) & 0170000) == 0040000)	/* directory */
#define	S_ISCHR(m)	(((m) & 0170000) == 0020000)	/* char special */
#define	S_ISBLK(m)	(((m) & 0170000) == 0060000)	/* block special */
#define	S_ISREG(m)	(((m) & 0170000) == 0100000)	/* regular file */
#define	S_ISFIFO(m)	(((m) & 0170000) == 0010000)	/* fifo or socket */

#define	S_ISLNK(m)	(((m) & 0170000) == 0120000)	/* symbolic link */
#define	S_ISSOCK(m)	(((m) & 0170000) == 0140000)	/* socket */
#define	S_ISWHT(m)	(((m) & 0170000) == 0160000)	/* whiteout */

inline mode_t typemode(int t, mode_t m);

#define S_IRWXU 00700
#define S_IRUSR 00400
#define S_IWUSR 00200
#define S_IXUSR 00100

#define S_IRWXG 00070
#define S_IRGRP 00040
#define S_IWGRP 00020
#define S_IXGRP 00010

#define S_IRWXO 00007
#define S_IROTH 00004
#define S_IWOTH 00002
#define S_IXOTH 00001

#ifdef __cplusplus
}
#endif
#endif


