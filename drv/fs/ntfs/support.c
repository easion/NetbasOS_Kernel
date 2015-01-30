/**
 * inode.c - NTFS kernel inode handling. Part of the Linux-NTFS project.
 *
 * Copyright (c) 2001-2005 Anton Altaparmakov
 *
 * This program/include file is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program/include file is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (in the main directory of the Linux-NTFS
 * distribution in the file COPYING); if not, write to the Free Software
 * Foundation,Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/
#include <ntfs.h>

int ntfs_read_mft_record( ntfs_volume * vol, int mftno, char *buf )
{
	int error;
	ntfs_io io;

	if( mftno == FILE_Mft )
	{
	//如果是主文件表
		ntfs_memcpy( buf, vol->mft, vol->mft_record_size );
		return 0;
	}
	if( !vol->mft_ino )
	{
	//主文件表还没有装载进来
		ntfs_error( "NTFS: mft_ino is NULL. Something is terribly wrong here!\n" );
		return -ENODATA;
	}
	io.fn_put = ntfs_put;
	io.fn_get = 0;
	io.param = buf;
	io.size = vol->mft_record_size;
	//取得其属性
	error = ntfs_read_attr( vol->mft_ino, vol->at_data, NULL, ( int64 )mftno << vol->mft_record_size_bits, &io );
	if( error || ( io.size != vol->mft_record_size ) )
	{
		//kprintf("ntfs_read_attr error%d-%d\n",error, io.size != vol->mft_record_size);
		return error ? error : -ENODATA;
	}
	if( !ntfs_check_mft_record( vol, buf ) )
	{
	//不通过mft检查，其magic类型必须是FILE
		/* FIXME: This is incomplete behaviour. We might be able to
		 * recover at this stage. ntfs_check_mft_record() is too
		 * conservative at aborting it's operations. It is OK for
		 * now as we just can't handle some on disk structures
		 * this way. (AIA) */
		kprintf( "NTFS: Invalid MFT record for 0x%x\n", mftno );
		return -EIO;
	}
	return 0;
}

int ntfs_getput_clusters( ntfs_volume * vol, int cluster, size_t start_offs, ntfs_io * buf )
{
	int length = buf->size;
	int error = 0;
	size_t to_copy;
	char *bh;

	to_copy = vol->cluster_size - start_offs;
	while( length )
	{
		if( !( bh = get_cblock( vol->fd, cluster, vol->cluster_size ) ) )
		{
			error = -EIO;
			goto error_ret;
		}
		if( to_copy > length )
			to_copy = length;

		if( buf->do_read )
		{
			buf->fn_put( buf, bh + start_offs, to_copy );
		}
		else
		{
			buf->fn_get( bh + start_offs, buf, to_copy );
			mark_dirty( vol->fd, cluster, 1 );
		}
		free_cblock( vol->fd, cluster );
		length -= to_copy;
		start_offs = 0;
		to_copy = vol->cluster_size;
		cluster++;
	}

error_ret:
	return error;
}

bigtime_t ntfs_now( void )
{
	return ntfs_unixutc2ntutc( startup_ticks() );
}
int ntfs_dupuni2map(ntfs_volume *vol, ntfs_u16 *in, int in_len, char **out,
		int *out_len)
{
	#define NLS_MAX_CHARSET_SIZE 255
	int i, o, chl, chi;
	char *result, *buf, charbuf[NLS_MAX_CHARSET_SIZE];
	void *nls = vol->nls_map;

	result = ntfs_malloc(in_len + 1);
	if (!result)
		return -ENOMEM;
	*out_len = in_len;
	for (i = o = 0; i < in_len; i++) {
		/* FIXME: Byte order? */
		wchar_t uni = in[i];
		chl = -1;//nls->uni2char(uni, charbuf,NLS_MAX_CHARSET_SIZE);
		if ((chl) > 0) {
			/* Adjust result buffer. */
			if (chl > 1) {
				buf = ntfs_malloc(*out_len + chl);
				if (!buf) {
					i = -ENOMEM;
					goto err_ret;
				}
				memcpy(buf, result, o);
				ntfs_free(result);
				result = buf;
				*out_len += (chl - 1);
			}
			for (chi = 0; chi < chl; chi++)
				result[o++] = charbuf[chi];
		} else {
			/* Invalid character. */
			printk( "NTFS: Unicode name contains a "
					"character that cannot be converted "
					"to chosen character set. Remount "
					"with utf8 encoding and this should "
					"work.\n");
			i = -EILSEQ;
			goto err_ret;
		}
	}
	result[*out_len] = '\0';
	*out = result;
	return 0;
err_ret:
	ntfs_free(result);
	*out_len = 0;
	*out = NULL;
	return i;
}

int ntfs_dupmap2uni(ntfs_volume *vol, char* in, int in_len, ntfs_u16 **out,
		int *out_len)
{
	int i, o;
	ntfs_u16 *result;
	void *nls = vol->nls_map;

	*out = result = ntfs_malloc(2 * in_len);
	if (!result) {
		*out_len = 0;
		return -ENOMEM;
	}
	*out_len = in_len;
	for (i = o = 0; i < in_len; i++, o++) {
		wchar_t uni;
		int charlen;

		charlen = -1;//nls->char2uni(&in[i], in_len - i, &uni);
		if (charlen < 0) {
			i = charlen;
			goto err_ret;
		}
		*out_len -= charlen - 1;
		i += charlen - 1;
		/* FIXME: Byte order? */
		result[o] = uni;
		if (!result[o]) {
			i = -EILSEQ;
			goto err_ret;
		}
	}
	return 0;
err_ret:
	printk( "NTFS: Name contains a character that cannot be "
			"converted to Unicode.\n");
	ntfs_free(result);
	*out_len = 0;
	*out = NULL;
	return i;
}

