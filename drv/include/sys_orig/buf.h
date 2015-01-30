/*
 * Copyright (c) 1982, 1986, 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)buf.h	8.9 (Berkeley) 3/30/95
 * $Id: buf.h,v 1.61.2.1 1999/03/03 19:33:14 julian Exp $
 */

#ifndef _SYS_BUF_H_
#define	_SYS_BUF_H_

#include <sys/queue.h>

struct buf;
struct mount;
struct vnode;

/*
 * To avoid including <ufs/ffs/softdep.h> 
 */   
LIST_HEAD(workhead, worklist);
/*
 * These are currently used only by the soft dependency code, hence
 * are stored once in a global variable. If other subsystems wanted
 * to use these hooks, a pointer to a set of bio_ops could be added
 * to each buffer.
 */
extern struct bio_ops {
	void	(*io_start) __P((struct buf *));
	void	(*io_complete) __P((struct buf *));
	void	(*io_deallocate) __P((struct buf *));
	int	(*io_fsync) __P((struct vnode *));
	int	(*io_sync) __P((struct mount *));
} bioops;

struct iodone_chain {
	long	ic_prev_flags;
	void	(*ic_prev_iodone) __P((struct buf *));
	void	*ic_prev_iodone_chain;
	struct {
		long	ia_long;
		void	*ia_ptr;
	}	ic_args[5];
};

/*
 * The buffer header describes an I/O operation in the kernel.
 */
struct buf {
	LIST_ENTRY(buf) b_hash;		/* Hash chain. */
	TAILQ_ENTRY(buf) b_vnbufs;	/* Buffer's associated vnode. */
	TAILQ_ENTRY(buf) b_freelist;	/* Free list position if not active. */
	TAILQ_ENTRY(buf) b_act;		/* Device driver queue when active. *new* */
	struct  proc *b_proc;		/* Associated proc; NULL if kernel. */
	long	b_flags;		/* B_* flags. */
	unsigned short b_qindex;	/* buffer queue index */
	unsigned char b_usecount;	/* buffer use count */
	unsigned char b_xflags;		/* extra flags */
	int	b_error;		/* Errno value. */
	long	b_bufsize;		/* Allocated buffer size. */
	long	b_bcount;		/* Valid bytes in buffer. */
	long	b_resid;		/* Remaining I/O. */
	dev_t	b_dev;			/* Device associated with buffer. */
	caddr_t	b_data;			/* Memory, superblocks, indirect etc. */
	caddr_t	b_kvabase;		/* base kva for buffer */
	int	b_kvasize;		/* size of kva for buffer */
	daddr_t	b_lblkno;		/* Logical block number. */
	daddr_t	b_blkno;		/* Underlying physical block number. */
	off_t	b_offset;		/* Offset into file */
					/* Function to call upon completion. */
	void	(*b_iodone) __P((struct buf *));
					/* For nested b_iodone's. */
	struct	iodone_chain *b_iodone_chain;
	struct	vnode *b_vp;		/* Device vnode. */
	int	b_dirtyoff;		/* Offset in buffer of dirty region. */
	int	b_dirtyend;		/* Offset of end of dirty region. */
	struct	ucred *b_rcred;		/* Read credentials reference. */
	struct	ucred *b_wcred;		/* Write credentials reference. */
	int	b_validoff;		/* Offset in buffer of valid region. */
	int	b_validend;		/* Offset of end of valid region. */
	daddr_t	b_pblkno;               /* physical block number */
	void	*b_saveaddr;		/* Original b_addr for physio. */
	caddr_t	b_savekva;              /* saved kva for transfer while bouncing */
	void	*b_driver1;		/* for private use by the driver */
	void	*b_driver2;		/* for private use by the driver */
	void	*b_spc;
	union	cluster_info {
		TAILQ_HEAD(cluster_list_head, buf) cluster_head;
		TAILQ_ENTRY(buf) cluster_entry;
	} b_cluster;
	struct	vm_page *b_pages[btoc(MAXPHYS)];
	int		b_npages;
	struct	workhead b_dep;		/* List of filesystem dependencies. */
};

/*
 * These flags are kept in b_flags.
 */
#define	B_AGE		0x00000001	/* Move to age queue when I/O done. */
#define	B_NEEDCOMMIT	0x00000002	/* Append-write in progress. */
#define	B_ASYNC		0x00000004	/* Start I/O, do not wait. */
#define	B_BAD		0x00000008	/* Bad block revectoring in progress. */
#define	B_BUSY		0x00000010	/* I/O in progress. */
#define	B_CACHE		0x00000020	/* Bread found us in the cache. */
#define	B_CALL		0x00000040	/* Call b_iodone from biodone. */
#define	B_DELWRI	0x00000080	/* Delay I/O until buffer reused. */
#define	B_FREEBUF	0x00000100	/* Instruct driver: free blocks */
#define	B_DONE		0x00000200	/* I/O completed. */
#define	B_EINTR		0x00000400	/* I/O was interrupted */
#define	B_ERROR		0x00000800	/* I/O error occurred. */
#define	B_SCANNED	0x00001000	/* VOP_FSYNC funcs mark written bufs */
#define	B_INVAL		0x00002000	/* Does not contain valid info. */
#define	B_LOCKED	0x00004000	/* Locked in core (not reusable). */
#define	B_NOCACHE	0x00008000	/* Do not cache block after use. */
#define	B_MALLOC	0x00010000	/* malloced b_data */
#define	B_CLUSTEROK	0x00020000	/* Pagein op, so swap() can count it. */
#define	B_PHYS		0x00040000	/* I/O to user memory. */
#define	B_RAW		0x00080000	/* Set by physio for raw transfers. */
#define	B_READ		0x00100000	/* Read buffer. */
#define	B_DIRTY		0x00200000	/* Needs writing later. */
#define	B_RELBUF	0x00400000	/* Release VMIO buffer. */
#define	B_WANTED	0x00800000	/* Process wants this buffer. */
#define	B_WRITE		0x00000000	/* Write buffer (pseudo flag). */
#define	B_WRITEINPROG	0x01000000	/* Write in progress. */
#define	B_XXX		0x02000000	/* Debugging flag. */
#define	B_PAGING	0x04000000	/* volatile paging I/O -- bypass VMIO */
#define	B_ORDERED	0x08000000	/* Must guarantee I/O ordering */
#define B_RAM		0x10000000	/* Read ahead mark (flag) */
#define B_VMIO		0x20000000	/* VMIO flag */
#define B_CLUSTER	0x40000000	/* pagein op, so swap() can count it */
#define B_AVAIL1	0x80000000	/* Available flag */

#define PRINT_BUF_FLAGS "\20\40avail1\37cluster\36vmio\35ram\34ordered" \
	"\33paging\32xxx\31writeinprog\30wanted\27relbuf\26dirty" \
	"\25read\24raw\23phys\22clusterok\21malloc\20nocache" \
	"\17locked\16inval\15avail2\14error\13eintr\12done\11freebuf" \
	"\10delwri\7call\6cache\5busy\4bad\3async\2needcommit\1age"

/*
 * These flags are kept in b_xflags.
 */
#define	B_VNDIRTY	0x01		/* On vnode dirty list */
#define	B_VNCLEAN	0x02		/* On vnode clean list */

#define	NOOFFSET	(-1LL)		/* No buffer offset calculated yet */

struct buf_queue_head {
	TAILQ_HEAD(buf_queue, buf) queue;
	daddr_t	last_pblkno;
	struct	buf *insert_point;
	struct	buf *switch_point;
};

/*
 * This structure describes a clustered I/O.  It is stored in the b_saveaddr
 * field of the buffer on which I/O is done.  At I/O completion, cluster
 * callback uses the structure to parcel I/O's to individual buffers, and
 * then free's this structure.
 */
struct cluster_save {
	long	bs_bcount;		/* Saved b_bcount. */
	long	bs_bufsize;		/* Saved b_bufsize. */
	void	*bs_saveaddr;		/* Saved b_addr. */
	int	bs_nchildren;		/* Number of associated buffers. */
	struct buf **bs_children;	/* List of associated buffers. */
};

static __inline void bufq_init __P((struct buf_queue_head *head));

static __inline void bufq_insert_tail __P((struct buf_queue_head *head,
					   struct buf *bp));

static __inline void bufq_remove __P((struct buf_queue_head *head,
				      struct buf *bp));

static __inline struct buf *bufq_first __P((struct buf_queue_head *head));

static __inline void
bufq_init(struct buf_queue_head *head)
{
	TAILQ_INIT(&head->queue);
	head->last_pblkno = 0;
	head->insert_point = NULL;
	head->switch_point = NULL;
}

static __inline void
bufq_insert_tail(struct buf_queue_head *head, struct buf *bp)
{
	if ((bp->b_flags & B_ORDERED) != 0) {
		head->insert_point = bp;
		head->switch_point = NULL;
	}
	TAILQ_INSERT_TAIL(&head->queue, bp, b_act);
}

static __inline void
bufq_remove(struct buf_queue_head *head, struct buf *bp)
{
	if (bp == head->switch_point)
		head->switch_point = TAILQ_NEXT(bp, b_act);
	if (bp == head->insert_point) {
		head->insert_point = TAILQ_PREV(bp, buf_queue, b_act);
		if (head->insert_point == NULL)
			head->last_pblkno = 0;
	} else if (bp == TAILQ_FIRST(&head->queue))
		head->last_pblkno = bp->b_pblkno;
	TAILQ_REMOVE(&head->queue, bp, b_act);
	if (TAILQ_FIRST(&head->queue) == head->switch_point)
		head->switch_point = NULL;
}

static __inline struct buf *
bufq_first(struct buf_queue_head *head)
{
	return (TAILQ_FIRST(&head->queue));
}


/*
 * number of buffer hash entries
 */
#define BUFHSZ 512

/*
 * buffer hash table calculation, originally by David Greenman
 */
#define BUFHASH(vnp, bn)        \
	(&bufhashtbl[(((uintptr_t)(vnp) >> 7)+(int)(bn)) % BUFHSZ])

/*
 * Definitions for the buffer free lists.
 */
#define BUFFER_QUEUES	6	/* number of free buffer queues */

#define QUEUE_NONE	0	/* on no queue */
#define QUEUE_LOCKED	1	/* locked buffers */
#define QUEUE_LRU	2	/* useful buffers */
#define QUEUE_VMIO	3	/* VMIO buffers */
#define QUEUE_AGE	4	/* not-useful buffers */
#define QUEUE_EMPTY	5	/* empty buffer headers*/

/*
 * Zero out the buffer's data area.
 */
#define	clrbuf(bp) {							\
	bzero((bp)->b_data, (u_int)(bp)->b_bcount);			\
	(bp)->b_resid = 0;						\
}

/* Flags to low-level allocation routines. */
#define B_CLRBUF	0x01	/* Request allocated buffer be cleared. */
#define B_SYNC		0x02	/* Do all allocations synchronously. */

#ifdef KERNEL
extern int	nbuf;			/* The number of buffer headers */
extern struct	buf *buf;		/* The buffer headers. */
extern char	*buffers;		/* The buffer contents. */
extern int	bufpages;		/* Number of memory pages in the buffer pool. */
extern struct	buf *swbuf;		/* Swap I/O buffer headers. */
extern int	nswbuf;			/* Number of swap I/O buffer headers. */
extern int	needsbuffer, numdirtybuffers;
extern TAILQ_HEAD(swqueue, buf) bswlist;
extern TAILQ_HEAD(bqueues, buf) bufqueues[BUFFER_QUEUES];

struct uio;

void	bufinit __P((void));
void	bremfree __P((struct buf *));
int	bread __P((struct vnode *, daddr_t, int,
	    struct ucred *, struct buf **));
int	breadn __P((struct vnode *, daddr_t, int, daddr_t *, int *, int,
	    struct ucred *, struct buf **));
int	bwrite __P((struct buf *));
void	bdwrite __P((struct buf *));
void	bawrite __P((struct buf *));
void	bdirty __P((struct buf *));
int	bowrite __P((struct buf *));
void	brelse __P((struct buf *));
void	bqrelse __P((struct buf *));
int	vfs_bio_awrite __P((struct buf *));
struct buf *     getpbuf __P((void));
struct buf *incore __P((struct vnode *, daddr_t));
struct buf *gbincore __P((struct vnode *, daddr_t));
int	inmem __P((struct vnode *, daddr_t));
struct buf *getblk __P((struct vnode *, daddr_t, int, int, int));
struct buf *geteblk __P((int));
int	allocbuf __P((struct buf *, int));
int	biowait __P((struct buf *));
void	biodone __P((struct buf *));

void	cluster_callback __P((struct buf *));
int	cluster_read __P((struct vnode *, u_quad_t, daddr_t, long,
	    struct ucred *, long, int, struct buf **));
int	cluster_wbuild __P((struct vnode *, long, daddr_t, int));
void	cluster_write __P((struct buf *, u_quad_t));
int	physio __P((void (*)(struct buf *), struct buf *, dev_t, 
	    int, u_int (*)(struct buf *), struct uio *));
u_int	minphys __P((struct buf *));
void	vfs_bio_clrbuf __P((struct buf *));
void	vfs_busy_pages __P((struct buf *, int clear_modify));
void	vfs_unbusy_pages __P((struct buf *));
void	vwakeup __P((struct buf *));
void	vmapbuf __P((struct buf *));
void	vunmapbuf __P((struct buf *));
void	relpbuf __P((struct buf *));
void	brelvp __P((struct buf *));
void	bgetvp __P((struct vnode *, struct buf *));
void	pbgetvp __P((struct vnode *, struct buf *));
void	pbrelvp __P((struct buf *));
void	reassignbuf __P((struct buf *, struct vnode *));
struct	buf *trypbuf __P((void));
void	vfs_bio_need_satisfy __P((void));
#endif /* KERNEL */

#endif /* !_SYS_BUF_H_ */
