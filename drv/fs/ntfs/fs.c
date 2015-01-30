/**
 * ntfs.c - NTFS kernel inode handling. Part of the Linux-NTFS project.
 *
 * Copyright (c) 2006 Jarek Pelczar <jpelczar@gmail.com>
 * Copyright (c) 2001-2005 Anton Altaparmakov
 *
 */

/* Builder sets CFLAGS with -O2, but someone compiling manually may not have CFLAGS set */
#ifndef __OPTIMIZE__
# error You must compile this driver with "-O".
#endif

#include "ntfs.h"
#include <drv/fs.h>
#include <assert.h>
enum{
	NLS_ASCII
};

static void ntfs_putuser( ntfs_io * dest, void *src, size_t len );
static ntfs_volume *get_ntfs_volume();

enum{
	FS_IS_PERSISTENT,
		FS_IS_BLOCKBASED ,
		FS_CAN_MOUNT ,
		FS_HAS_MIME ,
		FS_HAS_ATTR
};

static ntfs_volume *jicama_nt_vol;
static  fs_dev_ops_t _ntfs;

static int ntfs_read_stat(  ntfs_inode *ino, inode_t *st );
static int is_boot_sector_ntfs( uint8 *b );

#define ITEM_SIZE	2040

static void ntfs_putuser( ntfs_io * dest, void *src, size_t len )
{
	memcpy( dest->param, src, len );
	dest->param += len;
}
struct ntfs_getuser_update_vm_s {
	const char *user;
	struct inode *ino;
	loff_t off;
};

static void ntfs_getuser_update_vm(void *dest, ntfs_io *src, ntfs_size_t len)
{
	struct ntfs_getuser_update_vm_s *p = src->param;
	
	memcpy(dest, p->user, len);
	p->user += len;
	p->off += len;
}

/* loff_t is 64 bit signed, so is cool. */
int ntfs_read( void *pVolume, void *pNode, off_t nPos, void *pBuf, size_t nLen )
{
	ntfs_volume *vol = ( ntfs_volume * ) pVolume;
	ntfs_inode *ino = ( ntfs_inode * ) pNode;
	ntfs_io io;
	ntfs_attribute *attr;

	if( !ino )
		return -EINVAL;

	attr = ntfs_find_attr( ino, vol->at_data, NULL );
	/* Inode has no unnamed data attribute. */
	if (!attr) {
		ntfs_error( "ntfs_read: $DATA not found !\n" );
		return -EINVAL;
	}

	if (attr->flags & ATTR_IS_ENCRYPTED)
		return -EACCES;
	/* Read the data. */
	io.fn_put = ntfs_putuser;
	io.fn_get = 0;
	io.param = pBuf;
	io.size = nLen;
	int error = ntfs_read_attr( ino, vol->at_data, NULL, nPos, &io );

	if( error && !io.size )
	{
		return error;
	}

	return io.size;
}

#define CONFIG_NTFS_RW
//ssize_t ntfs_write(struct file *filp, const char *buf, size_t count,loff_t *pos)
ssize_t ntfs_write( void *pVolume, void *pNode, off_t nPos, void *pBuf, size_t nLen )
{
	ntfs_volume *vol = ( ntfs_volume * ) pVolume;
	ntfs_inode *ntfs_ino = ( ntfs_inode * ) pNode;
	int err;
	//struct inode *vfs_ino = filp->f_dentry->d_inode;
	//ntfs_inode *ntfs_ino = NTFS_LINO2NINO(vfs_ino);
	ntfs_attribute *data;
	ntfs_io io;
	struct ntfs_getuser_update_vm_s param;

	//if (!ntfs_ino)
	//	return -EINVAL;
	//ntfs_debug(DEBUG_LINUX, "%s(): Entering for inode 0x%lx, *pos 0x%Lx, "
	//		"count 0x%x.\n", __FUNCTION__, ntfs_ino->i_number,
	//		*pos, count);
	/* Allows to lock fs ro at any time. */
	//if (vfs_ino->i_sb->s_flags & MS_RDONLY)
	//	return -EROFS;
	data = ntfs_find_attr(ntfs_ino, vol->at_data, NULL);
	if (!data)
		return -EINVAL;
	/* Evaluating O_APPEND is the file system's job... */
	//if (filp->f_mode & O_APPEND)
	//	*pos = vfs_ino->i_size;
	if (!data->resident && nPos + nLen > data->allocated) {
		err = ntfs_extend_attr(ntfs_ino, data, nPos + nLen);
		if (err < 0)
			return err;
	}
	param.user = pBuf;
	//param.ino = vfs_ino;
	param.off = nPos;
	io.fn_put = 0;
	io.fn_get = ntfs_getuser_update_vm;
	io.param = &param;
	io.size = nLen;
	io.do_read = 0;
	err = ntfs_readwrite_attr(ntfs_ino, data, nPos, &io);
	ntfs_debug("%s(): Returning %i\n", __FUNCTION__, -err);
	if (!err) {
		nPos += io.size;
		//if (nPos > vfs_ino->i_size)
		//	vfs_ino->i_size = nPos;
		//mark_inode_dirty(vfs_ino);
		return io.size;
	}
	return err;
}
//#endif

static int 
my_filldir(vfs_dirent_t *dirent,const char *name, 
	int namelen, vfs_dirent_t *dirent_from )
{
	if( dirent->l_long_name[0])
		return -EINVAL;

	if(dirent_from)
		memcpy(dirent, dirent_from,sizeof(vfs_dirent_t));
	else
		memset(dirent, 0,sizeof(vfs_dirent_t));
	
	strncpy( dirent->l_long_name, name, namelen );
	return 0;
}

static int ntfs_printcb( uint8 *entry, void *param )
{
	unsigned long inum = NTFS_GETU64(entry) & 0xffffffffffff;
	struct ntfs_filldir *nf = ( struct ntfs_filldir * )param;
	u32_t flags = NTFS_GETU32(entry + 0x48);
	char show_sys_files = 0;
	int length = NTFS_GETU8( entry +0x50 );
	u8 name_type = NTFS_GETU8(entry + 0x51);
	int error;	
	vfs_dirent_t dirent;
	
	dirent.l_ctime =  ntfs_ntutc2unixutc(NTFS_GETU64(entry +0x18));
	dirent.l_atime =  ntfs_ntutc2unixutc(NTFS_GETU64(entry +0x20));
	dirent.l_mtime =  ntfs_ntutc2unixutc(NTFS_GETU64(entry +0x30));
	dirent.l_size_high = 0 ;//NTFS_GETU64(entry +0x38);
	dirent.l_size_low = NTFS_GETU64(entry +0x40);


	switch (nf->type) {
		case ngt_dos:
		/* Don't display long names. */
		if (!(name_type & 2))
			return 0;
		break;

		case ngt_nt:
		/* Don't display short-only names. */
		if ((name_type & 3) == 2)
			return 0;
			break;

		case ngt_posix:
			break;

		case ngt_full:
		show_sys_files = 1;
			break;
	default:
		BUG();
	}

	nf->name = 0;

	if( ntfs_encodeuni(NULL, ( uint16 * )( entry +0x52 ), length, &nf->name, &nf->namelen ) )
	{
		ntfs_debug( "Skipping unrepresentable file\n");
		if( nf->name )
			ntfs_free( nf->name );
		return 0;
	}
	/* Do not return ".", as this is faked. */
	if( length == 1 && *nf->name == '.' )
		return 0;

	nf->name[nf->namelen] = 0;
#if 0
	if (flags & 0x10000000) /* FILE_ATTR_DUP_FILE_NAME_INDEX_PRESENT */
		file_type = DT_DIR;
	else
		file_type = DT_REG;
#endif

	ntfs_debug("readdir got %s,len %d, filldir=%x\n",nf->name,nf->namelen,error);
	/*
	 * Userspace side of filldir expects an off_t rather than an loff_t.
	 * And it also doesn't like the most significant bit being set as it
	 * then considers the value to be negative. Thus this implementation
	 * limits the number of index records to 32766, which should be plenty.
	 */

	error = nf->filldir( nf->dirent, nf->name,	nf->namelen,&dirent);
	//printf("name %s offset=%d ino%d\n",nf->name, (*(unsigned short*)(entry + 8)), inum);
	if( error )
		nf->ret_code = error;
err_ret:
	ntfs_free( nf->name );
err_noname:
	nf->namelen = 0;
	nf->name = NULL;
	return error;
}


int ntfs_node_makesure(ntfs_volume *vol, inode_t *inode)
{
	int err;
	ntfs_inode *current_node;

	assert(sizeof(ntfs_inode)<4096);

	if (inode->i_private_data)
	{
		ntfs_debug("ntfs_node_makesure() inode @ 0x%x i_private_data @ %x\n",
			(u32_t)inode, (u32_t)inode->i_private_data);
		return 0;
	}

	err = ntfs_read_inode(vol, inode->i_number, &current_node);

	if (err)
	{
		//printk("ntfs_node_makesure():  ntfs_read_inode error\n");
		return -1;
	}

	if (!current_node->i_mode & S_IFDIR)
	{
		ntfs_free(current_node);
		ntfs_debug("ntfs_node_makesure(): current node no inode \n");
		return -1;
	}

	inode->i_private_data = (void*)kmalloc(4096,0);
	ntfs_debug("ntfs_node_makesure() inode->i_private_data @ %x\n", inode->i_private_data);
	if (!inode->i_private_data )
	{
		ntfs_free(current_node);
		return -1;
	}
	memcpy((void*)inode->i_private_data, (void*)current_node,sizeof(ntfs_inode) );
	ntfs_free(current_node);
	return 0;
}

/*
 * readdir returns '.', then '..', then the directory entries in sequence.
 * As the root directory contains an entry for itself, '.' is not emulated for
 * the root directory.
 */
int ntfs_readdir2(file_t* filp, vfs_dirent_t *dirent)
{
	inode_t *s_ino=NULL;
	inode_t *dir = filp->f_inode;
	int err=-1;
	struct ntfs_filldir cb;
	ntfs_volume *vol = get_ntfs_volume();
	ntfs_inode *current_node;
	ntfs_inode *mydir_node;
	ino_t c_ino;
	int error=1, file_notfound=1;
		
	assert(dir !=NULL);

	if (ntfs_node_makesure(vol, dir))
	{
		return err;
	}

	memset(dirent, 0, sizeof(vfs_dirent_t));
	current_node = dir->i_private_data;

	cb.ret_code = 0;
	cb.pl = filp->f_pos & 0xffff;
	cb.ph = (filp->f_pos >> 16) & 0x7fff;
	if (cb.ph>=0x7fff)
	{
		return -1;
	}
	filp->f_pos = (loff_t)(cb.ph << 16) | cb.pl;
	ntfs_debug( "%s(): Entering for inode %lu, f_pos 0x%x, "
			"i_mode 0x%x,\n", __FUNCTION__,
			dir->i_number, filp->f_pos, (unsigned int)dir->i_mode		);

	if (!cb.ph) {
		/* Start of directory. Emulate "." and "..". */
		if (!cb.pl) {
			ntfs_debug( "%s(): Calling my_filldir for . "
				    "with len 1, f_pos 0x%Lx, inode %lu, "
				    "DT_DIR.\n", __FUNCTION__, filp->f_pos,
				    dir->i_number);
			cb.ret_code = my_filldir(dirent, ".", 1,NULL);
			if (cb.ret_code)
				goto done;
			cb.pl++;
			filp->f_pos = (loff_t)(cb.ph << 16) | cb.pl;
		}

		if (cb.pl == (u32)1) {
			ntfs_debug( "%s(): Calling my_filldir for .. "
				    "with len 2, f_pos 0x%Lx, inode %lu, "
				    "DT_DIR.\n", __FUNCTION__, filp->f_pos,
				    filp->f_inode->i_father);
			cb.ret_code = my_filldir(dirent, "..", 2, NULL);
			if (cb.ret_code){
				ntfs_debug("cb.ret_code ofdir\n");
				goto done;
			}
			cb.pl++;
			filp->f_pos = (loff_t)(cb.ph << 16) | cb.pl;
		}
	} else if (cb.ph >= 0x7fff){
		ntfs_debug("end ofdir\n");
		/* End of directory. */
		goto done;
	}

	cb.dir = dir;
	cb.filldir = my_filldir;
	cb.dirent = dirent;
	cb.type =  vol->ngt;
	do {
		ntfs_debug( "%s(): Looking for next file using "
				"ntfs_getdir_unsorted(), f_pos 0x%Lx.\n",
				__FUNCTION__, (loff_t)(cb.ph << 16) | cb.pl);
		err = ntfs_getdir_unsorted(current_node, &cb.ph, &cb.pl,
				ntfs_printcb, &cb);
	} 
	while (!err && !cb.ret_code && cb.ph < 0x7fff);

	filp->f_pos = (loff_t)(cb.ph << 16) | cb.pl;
	ntfs_debug( "%s(): After ntfs_getdir_unsorted()"
			" calls, f_pos 0x%Lx.\n", __FUNCTION__, filp->f_pos);

	if (!err) {
#ifndef DEBUG
		if (!cb.ret_code)
			ntfs_debug( "%s(): EOD, f_pos 0x%x, "
					"returning %d.\n", __FUNCTION__,err,
					filp->f_pos);
		else 
			ntfs_debug( "%s(): my_filldir returned %i, "
					"returning %d, f_pos 0x%x.\n",
					__FUNCTION__,err, cb.ret_code, filp->f_pos);
#endif

done:
	dirent->l_name_len = 58; // fixme
		return 0;
	}
	ntfs_debug( "%s(): Returning %i, f_pos 0x%x.\n",
			__FUNCTION__, err, filp->f_pos);
	err:
	return -1;
}

int ntfs_create_vol(mount_t *mp, ntfs_volume ** ppVol )
{
	ntfs_volume *vol;
	int fd, i, error;

	fd = mp->m_dev;	

	vol = ( ntfs_volume * ) kmalloc( sizeof( ntfs_volume ), MEMF_KERNEL | MEMF_CLEAR | MEMF_OKTOFAILHACK );
	if( !vol )
	{
		return -ENOMEM;
	}

	memset(vol,0,sizeof( ntfs_volume ));
	strcpy( vol->volumeLabel, "Unknown" );
	//vol->id = id;
	vol->fd = mp->m_dev;
	vol->umask = 0222;
	vol->ngt = ngt_nt; //类型
	vol->nls_map = NLS_ASCII;
	vol->mft_zone_multiplier = -1;

	int blocksize = GetDeviceBlockSize( vol->fd );

	if( blocksize < 512 )
		blocksize = 512;

	char *bootblock = ( char * )kmalloc( blocksize, MEMF_KERNEL | MEMF_OKTOFAILHACK );

	if( !bootblock )
	{
		kfree( vol );
		return -ENOMEM;
	}

	mp->m_super = bootblock;
	//取得超级块
	int readsize = dev_read( vol->fd, 0, ( void * )bootblock, blocksize );
	if( readsize < 0)// != blocksize )
	{
		ntfs_error( "ntfs: Unable to read bootblock @ 0x%x\n", vol->fd );
		kfree( bootblock );
		kfree( vol );
		return -EIO;
	}

	//检查超级块的合法性
	if( !is_boot_sector_ntfs( ( uint8 * )bootblock ) )
	{
		kfree( bootblock );
		kfree( vol );
		ntfs_error("is_boot_sector_ntfs not!\n");
		return -EINVAL;
	}

	unsigned short *sn = (unsigned short *)(( uint8 * )bootblock+0x48);

	printf("volume %x-%x\n", sn[0], sn[1]);

	//根据启动扇区数据初始化系统数据
	if( ntfs_init_volume( vol, bootblock ) )
	{
		kfree( bootblock );
		kfree( vol );
		return -EINVAL;
	}

	kfree( bootblock );

	ntfs_debug( "$Mft at cluster 0x%x mft_clusters_per_record %x\n", 
		vol->mft_lcn, vol->mft_clusters_per_record );
	ntfs_debug( "Done to init volume\n" );

	i = vol->cluster_size;
	if( i < vol->mft_record_size )
		i = vol->mft_record_size;

	if( !( vol->mft = ( uint8 * )kmalloc( i, MEMF_KERNEL | MEMF_OKTOFAILHACK ) ) )
	{
		ntfs_error( "Unable to allocate %d bytes for MFT\n", i );
		kfree( vol );
		return -ENOMEM;
	}

	int to_read = vol->mft_clusters_per_record;

	if( to_read < 1 ){
		printk("vol->mft_clusters_per_record=0x%x \n", vol->mft_clusters_per_record );
		to_read = 1;
	}

	u32_t mft_blk = (vol->mft_lcn * vol->cluster_size)/HD_SECTOR_SIZE;

	readsize = dev_read( vol->fd, (off_t)mft_blk,
		( void * )vol->mft, to_read * vol->cluster_size ) ;

	if(readsize  != ( to_read * vol->cluster_size))
	{
		ntfs_error( "Unable to read MFT(%d-%d-0x%x)\n",  vol->cluster_size,vol->mft_lcn, mft_blk );
		error = -EIO;
		goto err1;
	}

	ntfs_debug("read buffer is %s\n", ( char * )vol->mft);
	//检查mft
	if( !ntfs_check_mft_record( vol, ( char * )vol->mft ) )
	{
		ntfs_error( "Invalid $Mft record 0\n" );
		error = -EINVAL;
		goto err1;
	}

	*ppVol = vol;
	return 0;

err1:
	kfree( vol->mft );
	kfree( vol );
	return error;
}


int ntfs_probe(const mount_t *mp )
{
	ntfs_volume *vol;
	ntfs_inode *current_node;

	ntfs_debug("ntfs probe\n");

	if( ntfs_create_vol( mp, &vol ) != 0 )
	{
		ntfs_error( "Error loading boot sector\n" );
		return -EINVAL;
	}
	return 0;
}





int ntfs_write_inode( void *_vol, void *_node )
{
	ntfs_volume *vol = ( ntfs_volume * ) _vol;
	ntfs_inode *ino = ( ntfs_inode * ) _node;
	ntfs_inode *new_ino;

	switch ( ino->i_ino )
	{
		case FILE_Mft:
			if( vol->mft_ino && !( vol->ino_flags & 1 ) )
			{
				new_ino = ( ntfs_inode * ) ntfs_malloc( sizeof( ntfs_inode ) );
				memcpy( new_ino, ino, sizeof( ntfs_inode ) );
				vol->mft_ino = new_ino;
				vol->ino_flags |= 1;
				return 0;
			}

		case FILE_MftMirr:
			if( vol->mftmirr && !( vol->ino_flags & 2 ) )
			{
				new_ino = ( ntfs_inode * ) ntfs_malloc( sizeof( ntfs_inode ) );
				memcpy( new_ino, ino, sizeof( ntfs_inode ) );
				vol->mftmirr = new_ino;
				vol->ino_flags |= 2;
				return 0;
			}

		case FILE_BitMap:
			if( vol->bitmap && !( vol->ino_flags & 4 ) )
			{
				new_ino = ( ntfs_inode * ) ntfs_malloc( sizeof( ntfs_inode ) );
				memcpy( new_ino, ino, sizeof( ntfs_inode ) );
				vol->bitmap = new_ino;
				vol->ino_flags |= 4;
				return 0;
			}
	}

	//释放这个扇区
	ntfs_clear_inode( ino );
	kfree( ino );
	return 0;
}

int ntfs_walk( void *_vol, void *_parent, const char *name, int name_len, ino_t *ino )
{
	/*ntfs_volume *vol = ( ntfs_volume * ) _vol;*/
	ntfs_inode *parent = ( ntfs_inode * ) _parent;

	char *item = 0;
	ntfs_iterate_s walk;
	int err;

	walk.name = NULL;
	walk.namelen = 0;

	err = ntfs_decodeuni( _vol, (char *)name, name_len, &walk.name, &walk.namelen );
	if( err )
		return err;

	item = ntfs_malloc( ITEM_SIZE );
	if( !item )
	{
		kfree( walk.name );
		return -ENOMEM;
	}

	walk.type = BY_NAME;
	walk.dir = parent;
	walk.result = item;
	if( ntfs_getdir_byname( &walk ) )
	{
		err = 0;
		*ino = NTFS_GETU32( item );
	}
	else
	{
		err = -ENOENT;
	}
	kfree( item );
	kfree( walk.name );
	return err;
}

#if 0
static int ntfs_create(struct inode* dir, struct dentry *d, int mode)
{
	struct inode *r = 0;
	ntfs_inode *ino = 0;
	ntfs_volume *vol;
	int error = 0;
	ntfs_attribute *si;

	r = new_inode(dir->i_sb);
	if (!r) {
		error = -ENOMEM;
		goto fail;
	}
	ntfs_debug(DEBUG_OTHER, "ntfs_create %s\n", d->d_name.name);
	vol = NTFS_INO2VOL(dir);
	ino = NTFS_LINO2NINO(r);
	//分配文件和数据体
	error = ntfs_alloc_file(NTFS_LINO2NINO(dir), ino, (char*)d->d_name.name,
				d->d_name.len);
	if (error) {
		ntfs_error("ntfs_alloc_file FAILED: error = %i", error);
		goto fail;
	}
	/* Not doing this one was causing a huge amount of corruption! Now the
	 * bugger bytes the dust! (-8 (AIA) */
	r->i_ino = ino->i_number;

	//更新磁盘节点
	error = ntfs_update_inode(ino);
	if (error)
		goto fail;
	//更新磁盘节点
	error = ntfs_update_inode(NTFS_LINO2NINO(dir));
	if (error)
		goto fail;
	r->i_uid = vol->uid;
	r->i_gid = vol->gid;
	/* FIXME: dirty? dev? */
	/* Get the file modification times from the standard information. */
	si = ntfs_find_attr(ino, vol->at_standard_information, NULL);
	//找到文件的标准信息
	if (si) {
		char *attr = si->d.data;
		r->i_atime = ntfs_ntutc2unixutc(NTFS_GETU64(attr + 0x18));
		r->i_ctime = ntfs_ntutc2unixutc(NTFS_GETU64(attr));
		r->i_mtime = ntfs_ntutc2unixutc(NTFS_GETU64(attr + 8));
	}
	/* It's not a directory */
	//r->i_op = &ntfs_inode_operations;
	//r->i_fop = &ntfs_file_operations;
	r->i_mode = S_IFREG | S_IRUGO;
#ifdef CONFIG_NTFS_RW
	r->i_mode |= S_IWUGO;
#endif
	r->i_mode &= ~vol->umask;
	insert_inode_hash(r);
	d_instantiate(d, r);
	return 0;
 fail:
	if (r)
		iput(r);
	return error;
}
#endif

int _linux_ntfs_mkdir(inode_t* dnode, char* dname)//struct inode *dir, struct dentry* d, int mode)
{
	int error;
	inode_t *r = 0;
	ntfs_volume *vol=get_ntfs_volume();
	ntfs_inode *ino;
	ntfs_inode _ino;
	ntfs_attribute *si;
	int d_name_len=strlen(dname);

	kprintf ( "mkdir %s in %x\n", dname, dnode->i_number);
	error = -ENAMETOOLONG;
	if (d_name_len > /* FIXME: */ 255)
		goto out;

	error = -EIO;
	//r = new_inode(dir->i_sb);
	//if (!r)
	//	goto out;

	//ino = NTFS_LINO2NINO(r);
	if (ntfs_node_makesure(vol, dnode))
	{
		kprintf("%s() Line%d Error\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ntfs_inode *dir_ino = dnode->i_private_data;

	error = ntfs_mkdir(dir_ino, dname, d_name_len,&_ino);
	if (error)
		goto out;
	/* Not doing this one was causing a huge amount of corruption! Now the
	 * bugger bytes the dust! (-8 (AIA) */

	r = iget(dnode->i_dev, _ino.i_ino);
	//r->i_number  = ino->i_ino ;
	//r->i_uid = vol->uid;
	//r->i_gid = vol->gid;
	if (ntfs_node_makesure(vol, r))
	{
		kprintf("%s() Line%d Error\n", __FUNCTION__, __LINE__);
		return -1;
	}

	ino = r->i_private_data;

	si = ntfs_find_attr(ino, vol->at_standard_information, NULL);
	if (si) {
		char *attr = si->d.data;
		r->i_atime = ntfs_ntutc2unixutc(NTFS_GETU64(attr + 0x18));
		r->i_ctime = ntfs_ntutc2unixutc(NTFS_GETU64(attr));
		r->i_mtime = ntfs_ntutc2unixutc(NTFS_GETU64(attr + 8));
	}
	/* It's a directory. */
	//r->i_op = &ntfs_dir_inode_operations;
	//r->i_fop = &ntfs_dir_operations;
	//r->i_mode = S_IFDIR | S_IRUGO | S_IXUGO;
#ifdef CONFIG_NTFS_RW
	//r->i_mode |= S_IWUGO;
#endif
	r->i_mode &= ~vol->umask;	
	
	//insert_inode_hash(r);
	//d_instantiate(d, r);
	error = 0;
 out:
 	kprintf ("mkdir returns %d\n", error);
	return error;
}


int ntfs_read_inode( void *_vol, ino_t inid, void **_node )
{
	ntfs_volume *vol = ( ntfs_volume * ) _vol;
	ntfs_inode *ino, *tmpnode;
	ntfs_attribute *data, *si;

	ino = ( ntfs_inode * ) ntfs_calloc( sizeof( ntfs_inode ) );
	if( !ino ){
		kprintf("ntfs_read_inode() no memory!\n");
		return -ENOMEM;
	}

	ntfs_debug( "reading inode %d at %p\n",inid,ino);

	ino->i_ino = inid;

	switch ( inid )
	{
		case FILE_Mft:
			if( !vol->mft_ino || ( ( vol->ino_flags & 1 ) == 0 ) )
				goto sys_file_error;
			ntfs_debug( "Opening $MFT ...\n" );
			memcpy( ino, vol->mft_ino, sizeof( ntfs_inode ) );
			tmpnode = vol->mft_ino;
			vol->mft_ino = ino;
			vol->ino_flags &= ~1;
			kfree( tmpnode );
			break;
		case FILE_MftMirr:
			if( !vol->mftmirr || ( ( vol->ino_flags & 2 ) == 0 ) )
				goto sys_file_error;
			ntfs_debug( "Opening $MFTMirr ...\n" );
			memcpy( ino, vol->mftmirr, sizeof( ntfs_inode ) );
			tmpnode = vol->mftmirr;
			vol->mftmirr = ino;
			vol->ino_flags &= ~2;
			kfree( ino );
			break;
		case FILE_BitMap:
			if( !vol->bitmap || ( ( vol->ino_flags & 4 ) == 0 ) )
				goto sys_file_error;
			ntfs_debug( "Opening $Bitmap\n" );
			memcpy( ino, vol->bitmap, sizeof( ntfs_inode ) );
			tmpnode = vol->bitmap;
			vol->bitmap = ino;
			vol->ino_flags &= ~4;
			kfree( tmpnode );
			break;
		case FILE_LogFile...FILE_AttrDef:
		case FILE_Boot...FILE_UpCase:
			ntfs_debug( "Opening system file %d\n", ( int64 )inid );
		default:
			if( ntfs_init_inode( ino, vol, ino->i_ino ) )
			{
				ntfs_error( "ntfs: error loading inode %d\n", ( int64 )ino->i_ino );
				//kprintf("%s() ntfs_init_inode error\n", __FUNCTION__);
				return -ENOMEM;
			}
			break;
	}

	ino->i_uid = vol->uid;
	ino->i_gid = vol->gid;

	data = ntfs_find_attr( ino, vol->at_data, NULL );
	if( !data )
		ino->i_size = 0;
	else{
		ino->i_size = data->size;
		//kprintf("inode %d size=%d\n", ino->i_ino, ino->i_size);
	}

	si = ntfs_find_attr( ino, vol->at_standard_information, NULL );
	if( si )
	{
		char *attr = si->d.data;

		ino->atime = ntfs_ntutc2unixutc( NTFS_GETU64( attr + 0x18 ) );
		ino->ctime = ntfs_ntutc2unixutc( NTFS_GETU64( attr ) );
		ino->mtime = ntfs_ntutc2unixutc( NTFS_GETU64( attr + 8 ) );
	}

	if( ntfs_find_attr( ino, vol->at_index_root, "$I30" ) )
	{
		ntfs_debug( "ntfs_read_inode: this is directory\n");
		ntfs_attribute *at = ntfs_find_attr( ino, vol->at_index_root, "$I30" );

		ino->i_size = at ? at->size : 0;
		ino->i_mode = S_IFDIR | 0666;
	}
	else
	{
		ntfs_debug( "ntfs_read_inode: this is regular file\n" );
		ino->i_mode = S_IFREG | 0666;
	}
	ino->i_mode &= ~vol->umask;

	ntfs_debug( "ino %d mode: %o size %d\n",(int)ino->i_ino,ino->i_mode,ino->i_size);
	ntfs_debug( "atime %d mtime %d ctime %d\n",ino->atime,ino->mtime,ino->ctime);

	*_node = ( void * )ino;
	return 0;

sys_file_error:
	ntfs_error( "Critical error. Tried to call ntfs_read_inode() before we have completed read_super() or VFS error.\n" );
	return -EINVAL;
}

inode_t* ntfs_opendir(inode_t * inode, char *name)
{
	int errorval;
	inode_t* child_vnode = NULL;
	ntfs_inode *fathernode;
	ntfs_inode *childnode;
	mount_t *sp;
	int nlen = strlen(name);
	ino_t c_ino;

	 ntfs_debug("ntfs open dir call @ %d\n", inode->i_number);

	assert(inode !=NULL);
	if (ntfs_node_makesure(get_ntfs_volume(),inode ))
	{
		return -1;
	}
	
	fathernode = inode->i_private_data;
	 ntfs_debug("ntfs_node_makesure  call ok @node %d\n", fathernode->i_ino);

	errorval = ntfs_walk(get_ntfs_volume(), fathernode, 
		name,nlen, &c_ino );

	 if (errorval)
	 {
		 ntfs_debug("%s() : not found \n", name);
		 goto err;
	 }
	 ntfs_debug("ntfs_walk  call ok\n");

	 child_vnode = iget(inode->i_dev, c_ino);
	 ntfs_debug("found %s @ %d\n", name, c_ino);

	if (ntfs_node_makesure(get_ntfs_volume(),child_vnode ))
	{
		return -1;
	}
	childnode = child_vnode->i_private_data;
	ntfs_read_stat(childnode, child_vnode);
	ntfs_debug("found2 %s @ %d\n", name, c_ino);

	//sp=get_superblock(inode->i_dev);
	child_vnode->i_father = inode->i_number;
	return child_vnode;
err:
	return NULL;
}

int ntfs_mount(mount_t *mp, void *_data )
{
	ntfs_volume *vol;
	int error;
	inode_t *tmp_root_inode;
	ntfs_inode *current_node;

	if( ntfs_create_vol( mp, &vol ) != 0 )
	{
		ntfs_error( "Error loading boot sector\n" );
		return -EINVAL;
	}

	mp->m_magic = NTFS_MAGIC;

	_ntfs.fs_blksz = vol->cluster_size;

	error = setup_device_cache( vol->fd,  vol->nr_clusters );
	if( error )
		goto err1;

	if( ( error = ntfs_load_special_files( vol ) ) )
	{
		ntfs_error( "Error loading special files\n" );
		goto err2;
	}

	jicama_nt_vol = ( void * )vol;
	tmp_root_inode = iget(mp->m_dev->devno, FILE_root);
	if (ntfs_node_makesure(vol,tmp_root_inode ))
	{
		return -1;
	}

	current_node = tmp_root_inode->i_private_data;

	ntfs_read_stat(current_node,tmp_root_inode);

	if (!tmp_root_inode)
	{
		goto err2;
	}
	mp->m_root_ino=FILE_root;
	mp->m_root=tmp_root_inode;
	mp->m_count ++;
	mp->m_blk_size = 512;
	return 0;

err2:
	shutdown_device_cache( vol->fd );
err1:
	kfree( vol->mft );
	kfree( vol );
	return error;
}

int ntfs_unmount(mount_t *m, void *pVolume )
{
	ntfs_volume *vol = ( ntfs_volume * ) pVolume;

	//delete_semaphore( vol->lock );

	ntfs_release_volume( vol );
	shutdown_device_cache( vol->fd );
	kfree( vol );

	return 0;
}

/* Called by the kernel when asking for stats. */
static int ntfs_statfs(mount_t *mnt, statfs_t *sf)
{
	inode_t *mft;
	ntfs_volume *vol = ( ntfs_volume * ) get_ntfs_volume();
	__s64 size;
	int error;

	ntfs_debug(DEBUG_OTHER, "ntfs_statfs\n");
	sf->f_type = 'NTFS'; //0x5346544e;// NTFS_SUPER_MAGIC;
	sf->f_bsize = vol->cluster_size;
	error = ntfs_get_volumesize(vol, &size);
	if (error)
		return error;
	sf->f_blocks = size;	/* Volumesize is in clusters. */
	size = (__s64)ntfs_get_free_cluster_count(vol->bitmap);
	/* Just say zero if the call failed. */
	if (size < 0LL)
		size = 0;
	sf->f_bfree = sf->f_bavail = size;
	ntfs_debug( "ntfs_statfs: calling mft = iget(sb, "
			"FILE_Mft)\n");
	mft = iget(mnt->m_dev->devno, FILE_Mft);
	ntfs_debug( "ntfs_statfs: iget(sb, FILE_Mft) returned "
			"0x%x\n", mft);
	if (!mft)
		return -EIO;
	sf->f_files = mft->i_size >> vol->mft_record_size_bits;
	kprintf( "ntfs_statfs: calling iput(mft)\n");
	iput(mft);
	/* Should be read from volume. */
	sf->f_namelen = 255;
	return 0;
}

static int is_boot_sector_ntfs( uint8 *b )
{
	uint32 i;

	/* Check magic is "NTFS    " */
	if( b[3] != 0x4e )
		goto not_ntfs;
	if( b[4] != 0x54 )
		goto not_ntfs;
	if( b[5] != 0x46 )
		goto not_ntfs;
	if( b[6] != 0x53 )
		goto not_ntfs;
	for( i = 7; i < 0xb; ++i )
		if( b[i] != 0x20 )
			goto not_ntfs;
	/* Check bytes per sector value is between 512 and 4096. */
	if( b[0xb] != 0 )
		goto not_ntfs;
	if( b[0xc] > 0x10 )
		goto not_ntfs;
	/* Check sectors per cluster value is valid. */
	switch ( b[0xd] )
	{
		case 1:
		case 2:
		case 4:
		case 8:
		case 16:
		case 32:
		case 64:
		case 128:
			break;
		default:
			goto not_ntfs;
	}
	/* Check reserved sectors value and four other fields are zero. */
	for( i = 0xe; i < 0x15; ++i )
		if( b[i] != 0 ){
			ntfs_epoint();
			goto not_ntfs;
	}
	if( b[0x16] != 0 ){
		ntfs_epoint();
		goto not_ntfs;
	}
	if( b[0x17] != 0 ){
		ntfs_epoint();
		goto not_ntfs;
	}
	for( i = 0x20; i < 0x24; ++i )
		if( b[i] != 0 ){
			ntfs_epoint();
			goto not_ntfs;
	}
	/* Check clusters per file record segment value is valid. */
	if( b[0x40] < 0xe1 || b[0x40] > 0xf7 )
	{
		switch ( b[0x40] )
		{
			case 1:
			case 2:
			case 4:
			case 8:
			case 16:
			case 32:
			case 64:
				break;
			default:
				ntfs_epoint();
				goto not_ntfs;
		}
	}
	/* Check clusters per index block value is valid. */
	if( b[0x44] < 0xe1 || b[0x44] > 0xf7 )
	{
		switch ( b[0x44] )
		{
			case 1:
			case 2:
			case 4:
			case 8:
			case 16:
			case 32:
			case 64:
				break;
			default:
				ntfs_epoint();
				goto not_ntfs;
		}
	}
	return 1;

not_ntfs:
	return 0;
}


static int ntfs_read_stat(  ntfs_inode *ino, inode_t *st )
{
	//assert( st->i_number == ino->i_ino);
	assert( st != NULL);
	assert( ino != NULL);
	//st->i_dev = vol->id;
	//st->st_ino = ino->i_ino;
	//st->st_nlink = 1;
	st->i_uid = ino->i_uid;
	//st->st_gid = ino->i_gid;
	//st->st_blksize = vol->cluster_size;
	st->i_mode = ino->i_mode;
	st->i_size = ino->i_size;
	st->i_ctime = ino->ctime / 1000000;
	st->i_seek = 0; 
	st->i_mtime = ino->mtime / 1000000;
	st->i_atime = ino->atime / 1000000;

	//kprintf("inode %d st->i_size = %d\n", ino->i_ino, st->i_size);


	return -EOK;
}



int ntfs_fread(file_t * filp, char * buf, int count)
{
	int errorval;
	inode_t *inode;
	off_t pos = filp->f_pos;
	ntfs_inode *current_node;

	assert(inode !=NULL);
	assert(filp!=NULL);

	//ntfs_debug("ntfs_fread(): ino %d\n", inode->i_number);

	if (ntfs_node_makesure(get_ntfs_volume(), inode))
	{
		return -1;
	}

	current_node = inode->i_private_data;

	errorval = ntfs_read(get_ntfs_volume(), current_node,pos, buf,count);

	if (errorval<0)
	{
		return errorval;
	}

	filp->f_pos += errorval;
	return errorval;
}

int ntfs_fwrite(file_t * filp, char * buf, int count)
{
	int errorval;
	inode_t *inode=filp->f_inode;
	off_t pos = filp->f_pos;
	ntfs_inode *current_node;

	assert(inode !=NULL);
	if (ntfs_node_makesure(get_ntfs_volume(), inode))
	{
		return -1;
	}

	current_node = inode->i_private_data;

	errorval = ntfs_write(get_ntfs_volume(), current_node,pos, buf,count);

	if (errorval<0)
	{
		return errorval;
	}

	filp->f_pos += errorval;
	return 0;
}

static ntfs_volume *get_ntfs_volume()
{
	return jicama_nt_vol;
}

const char *fsname = "ntfs";

static int GetDeviceBlockSize( int fd )
{
#if 0
	struct stat st;
	device_geometry dg;

	kerndbg( KERN_DEBUG_LOW, "GetDeviceBlockSize( int fd = %d )\n", fd );
	if( ioctl( fd, IOCTL_GET_DEVICE_GEOMETRY, &dg ) != -EOK )
	{
		if( fstat( fd, &st ) < 0 || S_ISDIR( st.st_mode ) )
			return 0;
		return HD_SECTOR_SIZE;	/* just assume it's a plain old file or something */
	}

	return dg.bytes_per_sector;
#else
	return HD_SECTOR_SIZE;
#endif
}

