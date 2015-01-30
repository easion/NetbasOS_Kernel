
#include "ipf-linux.h"
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
# include <linux/devfs_fs_kernel.h>
#endif

#ifdef CONFIG_PROC_FS
# include <linux/proc_fs.h>
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,10)
#  define USE_PROC_FS		1
# endif
#endif

extern wait_queue_head_t	iplh_linux[IPL_LOGSIZE];

#ifdef MODULE
MODULE_SUPPORTED_DEVICE("ipf");
MODULE_AUTHOR("Darren Reed");
MODULE_DESCRIPTION("IP-Filter Firewall");
MODULE_LICENSE("(C)Copyright 2003-2004 Darren Reed");

# if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,10)
module_param(fr_flags, int, 0);
module_param(fr_control_forwarding, int, 0);
module_param(fr_update_ipid, int, 0);
module_param(fr_chksrc, int, 0);
module_param(fr_pass, int, 0);
module_param(ipstate_logging, int, 0);
module_param(nat_logging, int, 0);
module_param(ipl_suppress, int, 0);
module_param(ipl_logall, int, 0);
# else
MODULE_PARM(fr_flags, "i");
MODULE_PARM(fr_control_forwarding, "i");
MODULE_PARM(fr_update_ipid, "i");
MODULE_PARM(fr_chksrc, "i");
MODULE_PARM(fr_pass, "i");
MODULE_PARM(ipstate_logging, "i");
MODULE_PARM(nat_logging, "i");
MODULE_PARM(ipl_suppress, "i");
MODULE_PARM(ipl_logall, "i");
# endif
#endif

static int ipf_open(struct inode *, struct file *);
static ssize_t ipf_write(struct file *, const char *, size_t, loff_t *);
static ssize_t ipf_read(struct file *, char *, size_t, loff_t *);
static u_int ipf_poll(struct file *fp, struct poll_table_struct *wait);
extern int ipf_ioctl(struct inode *, struct file *, u_int, u_long);


#ifdef	CONFIG_DEVFS_FS
static	char	*ipf_devfiles[] = { IPL_NAME, IPNAT_NAME, IPSTATE_NAME,
				    IPAUTH_NAME, IPSYNC_NAME, IPSCAN_NAME,
				    IPLOOKUP_NAME, NULL };
#endif

static struct file_operations ipf_fops = {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	.owner = THIS_MODULE,
#endif
	open:	ipf_open,
	read:	ipf_read,
	write:	ipf_write,
	ioctl:	ipf_ioctl,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	poll:	ipf_poll,
#endif
};


#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
static	devfs_handle_t	dh[IPL_LOGSIZE];
#endif
static	int		ipfmajor = 0;



int uiomove(address, nbytes, rwflag, uiop)
caddr_t address;
size_t nbytes;
int rwflag;
uio_t *uiop;
{
	int err = 0;

	if (rwflag == UIO_READ) {
		err = copy_to_user(uiop->uio_buf, address, nbytes);
		if (err == 0) {
			uiop->uio_resid -= nbytes;
			uiop->uio_buf += nbytes;
		}
	} else if (rwflag == UIO_WRITE) {
		err = copy_from_user(uiop->uio_buf, address, nbytes);
		if (err == 0) {
			uiop->uio_resid -= nbytes;
			uiop->uio_buf += nbytes;
		}
	}
	if (err)
		return -EFAULT;
	return 0;
}


static int ipf_open(struct inode *in, struct file *fp)
{
	int unit, err;

	unit = MINOR(in->i_rdev);

	if (unit < 0 || unit > IPL_LOGMAX) {
		err = -ENXIO;
	} else {
		switch (unit)
		{
		case IPL_LOGIPF :
		case IPL_LOGNAT :
		case IPL_LOGSTATE :
		case IPL_LOGAUTH :
#ifdef IPFILTER_LOOKUP
		case IPL_LOGLOOKUP :
#endif
#ifdef IPFILTER_SYNC  
		case IPL_LOGSYNC :
#endif
#ifdef IPFILTER_SCAN
		case IPL_LOGSCAN :
#endif
			err = 0;
			break;
		default :  
			err = -ENXIO;
			break;
		}
	}
	return err;
}


static ssize_t ipf_write(struct file *fp, const char *buf, size_t count,
			loff_t *posp)
{
#ifdef IPFILTER_SYNC
	struct inode *i;
	int unit, err;
	uio_t uio;

	i = fp->f_dentry->d_inode;
	unit = MINOR(i->i_rdev);

	if (unit != IPL_LOGSYNC)
		return -ENXIO;

	uio.uio_rw = UIO_WRITE;
        uio.uio_iov = NULL;
        uio.uio_file = fp;
        uio.uio_buf = (char *)buf;
        uio.uio_iovcnt = 0;
        uio.uio_offset = *posp;
        uio.uio_resid = count;

	err = ipfsync_write(&uio);
	if (err > 0)
		err = -err;
	return err;
#else
	return -ENXIO;
#endif
}


static ssize_t ipf_read(struct file *fp, char *buf, size_t count, loff_t *posp)
{
	struct inode *i;
	int unit, err;
	uio_t uio;

	i = fp->f_dentry->d_inode;
	unit = MINOR(i->i_rdev);

	uio.uio_rw = UIO_READ;
        uio.uio_iov = NULL;
        uio.uio_file = fp;
        uio.uio_buf = (char *)buf;
        uio.uio_iovcnt = 0;
        uio.uio_offset = *posp;
        uio.uio_resid = count;

	switch (unit)
	{
#ifdef IPFILTER_SYNC
	case IPL_LOGSYNC :
		err = ipfsync_read(&uio);
		break;
#endif
	default :
		err = ipflog_read(unit, &uio);
		if (err == 0)
			return count - uio.uio_resid;
		break;
	}

	if (err > 0)
		err = -err;
	return err;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
static u_int ipf_poll(struct file *fp, poll_table *wait)
{
	struct inode *i;
	u_int revents;
	int unit;

	revents = 0;
	i = fp->f_dentry->d_inode;
	unit = MINOR(i->i_rdev);
	if (unit < 0 || unit > IPL_LOGMAX)
		return 0;

	poll_wait(fp, &iplh_linux[unit], wait);

	switch (unit)
	{
	case IPL_LOGIPF :
	case IPL_LOGNAT :
	case IPL_LOGSTATE :
# ifdef IPFILTER_LOG
		if (ipflog_canread(unit))
			revents = (POLLIN | POLLRDNORM);
# endif
		break;
	case IPL_LOGAUTH :
		if (fr_auth_waiting())
			revents = (POLLIN | POLLRDNORM);
		break;
	case IPL_LOGSYNC :
# ifdef IPFILTER_SYNC
		if (ipfsync_canread())
			revents = (POLLIN | POLLRDNORM);
# endif
		break;
	case IPL_LOGSCAN :
	case IPL_LOGLOOKUP :
	default :
		break;
	}

	return revents;
}
#endif


#if defined(USE_PROC_FS)
static int ipf_proc_info(char *buffer, char **start, off_t offset, int len)
{
	snprintf(buffer, len, "ipfmajor %d", ipfmajor);
	buffer[len - 1] = '\0';
	return strlen(buffer);
}
#endif

static int ipfilter_init(void)
{
#ifdef	CONFIG_DEVFS_FS
	char *s;
#endif
	int i;

	ipfmajor = register_chrdev(0, "ipf", &ipf_fops);
	if (ipfmajor < 0) {
		printf("unable to get major for ipf devs (%d)\n", ipfmajor);
		return -EINVAL;
	}

#ifdef	CONFIG_DEVFS_FS
	for (i = 0; ipf_devfiles[i] != NULL; i++) {
		s = strrchr(ipf_devfiles[i], '/');
		if (s != NULL) {
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			dh[i] = devfs_register(NULL, s + 1, DEVFS_FL_DEFAULT,
					       ipfmajor, i, 0600|S_IFCHR,
					       &ipf_fops, NULL);
# else
			devfs_mk_cdev(MKDEV(ipfmajor, i),0600|S_IFCHR,s+1);
# endif
		}
	}
#endif

	RWLOCK_INIT(&ipf_global, "ipf global rwlock");
	RWLOCK_INIT(&ipf_mutex, "ipf global mutex rwlock");
	RWLOCK_INIT(&ipf_frcache, "ipf cache mutex rwlock");

	i = ipfattach();

	if (i == 0) {
		char *defpass;
#if defined(USE_PROC_FS)
		struct proc_dir_entry *ipfproc;

		ipfproc = proc_net_create("ipfilter", 0, ipf_proc_info);
# ifndef __GENKSYMS__
		if (ipfproc != NULL)
			ipfproc->owner = THIS_MODULE;
# endif
#else
		printf("IPFilter: device major number: %d\n", ipfmajor);
#endif /* USE_PROC_FS */
		if (FR_ISPASS(fr_pass))
			defpass = "pass";
		else if (FR_ISBLOCK(fr_pass))
			defpass = "block";
		else
			defpass = "no-match -> block";

		printk(KERN_INFO "%s initialized.  Default = %s all, "
		       "Logging = %s%s\n",
			ipfilter_version, defpass,
# ifdef IPFILTER_LOG
			"enabled",
# else
			"disabled",
# endif
# ifdef IPFILTER_COMPILED
			" (COMPILED)"
# else
			""
# endif
			);

		fr_running = 1;
	}

	if (i != 0) {
		RW_DESTROY(&ipf_global);
		RW_DESTROY(&ipf_mutex);
		RW_DESTROY(&ipf_frcache);
	}

	return i;
}


static int ipfilter_fini(void)
{
	int result;
#ifdef	CONFIG_DEVFS_FS
	char *s;
	int i;
#endif

	if (fr_refcnt)
		return -EBUSY;

	if (fr_running >= 0) {
		result = ipfdetach();
		if (result != 0) {
			if (result > 0)
				result = -result;
			return result;
		}
	}

	RW_DESTROY(&ipf_global);
	RW_DESTROY(&ipf_mutex);
	RW_DESTROY(&ipf_frcache);

	fr_running = -2;
#if defined(USE_PROC_FS)
	proc_net_remove("ipfilter");
#endif


#ifdef	CONFIG_DEVFS_FS
	for (i = 0; ipf_devfiles[i] != NULL; i++) {
		s = strrchr(ipf_devfiles[i], '/');
		if (s != NULL)
# if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
			devfs_unregister_chrdev(ipfmajor, s + 1);
# else
			devfs_remove(s+1);
# endif
	}
#endif

	if (ipfmajor >= 0)
		unregister_chrdev(ipfmajor, "ipf");
	printk(KERN_INFO "%s unloaded\n", ipfilter_version);

	return 0;
}


static int __init ipf_init(void)
{
	int result;

	result = ipfilter_init();
	return result;
}


static void __exit ipf_fini(void)
{
	(void) ipfilter_fini();
}

module_init(ipf_init)
module_exit(ipf_fini)
