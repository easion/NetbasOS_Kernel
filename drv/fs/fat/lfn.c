
#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"
#include "../unicode/unicode.h"


#define FAT_LFNPMAX 260 /* Max length for a long file name path  */
#define FAT_LFNMAX  255 /* Max length for a long file name       */
#define FAT_SFNPMAX 64  /* Max length for a short file name path */
#define FAT_SFNMAX  14  /* Max length for a short file name      */
#define FATLFN


/* Calculate the 8-bit checksum for the long file name from its */
/* corresponding short name.                                    */
/* Called by split_lfn and find (find.c).                       */
u8_t lfn_checksum(fat_dir_entry *dirbuf)
{
  int __s = 0, i;
  char *short_name=(char *)dirbuf;

  for (i = 0; i < 11; i++)
  {
    __s = (((__s & 1) << 7) | ((__s & 0xFE) >> 1)) + short_name[i];
  }
  return __s;
}



int fat_fetch_lfn(fat_dir_entry *dir_buf, int buf_pos, u16_t *fname)
{
  lfn_entry_t *lfnslot  = (lfn_entry_t *) &dir_buf[buf_pos];
  u8_t       checksum_value = lfn_checksum(&dir_buf[buf_pos]);
  int        order    = 0;
  int        name_pos  = 0;
  int        k;

  do
  {
	  --lfnslot;

    if (lfnslot < (lfn_entry_t *) dir_buf)
		lfnslot += LFN_FETCH_SLOTS;

    if (lfnslot->l_attribute != MSDOS_LONGNAME) return 0;
    if (++order != (lfnslot->Order & 0x1F)) return 0;
    if (checksum_value != lfnslot->checksum_value) return 0;

    /* Ok, the LFN slot is valid, attach it to the long name */
    for (k = 0; k < 5; k++) fname[name_pos++] = lfnslot->Name0_4[k];
    for (k = 0; k < 6; k++) fname[name_pos++] = lfnslot->Name5_10[k];
    for (k = 0; k < 2; k++) fname[name_pos++] = lfnslot->Name11_12[k];
  }
  while (!(lfnslot->Order & LFN_END_FLAGS));

  if (fname[name_pos - 1] != 0x0000) fname[name_pos] = 0x0000;

  return order;
}

void fat_expand_name(struct msdos_dir *dirent, unsigned char* buf);

static int fat_do_readdir( int dir_fd, fatfs_find_t *findbuf)
{
  fat_dir_entry  dir_buf[LFN_FETCH_SLOTS];
  int        buf_pos = 0; /*start first entry*/
  int        iread;
  u16_t       long_utf16_name[FAT_LFNMAX];

 // if (!(dir_fd->DirEntry.l_attribute & MSDOS_DIR)) return -1;

  iread = file_read(dir_fd, &dir_buf[0], 32);
  if (iread < 0) return iread;

  while (iread > 0)
  {
    if (dir_buf[buf_pos].file_name[0] == 0x00) return -1;

    if (dir_buf[buf_pos].file_name[0] != 0xe5)
    {
      if (dir_buf[buf_pos].attribute != LFN_FETCH_SLOTS)/*skip all long filename entry */
      {
        findbuf->SfnEntry    = dir_buf[buf_pos];
       //findbuf->EntryOffset = dir_fd->TargetPos - iread;
        fat_expand_name(&dir_buf[buf_pos], findbuf->l_long_name);

        findbuf->LfnEntries = fat_fetch_lfn(dir_buf, buf_pos, long_utf16_name);

        if (findbuf->LfnEntries){
          fd32_utf16to8(long_utf16_name, findbuf->l_long_name);
           printk("Found: %-14s\n", findbuf->l_long_name);
		}
      else{
          strcpy(findbuf->l_long_name, findbuf->l_long_name); 
	  }

        return 0; /*ok backup*/
      }

    }

    if (++buf_pos == LFN_FETCH_SLOTS) 
		buf_pos = 0;

    iread = file_read(dir_fd, &dir_buf[buf_pos], 32);
    if (iread < 0) return iread;

  } /*end  while */

  return -1;
}


/* Searches an open directory for files matching the file specification and   */
/* the search flags.                                                          */
/* On success, returns 0 and fills the passed find data structure (optional). */
/* Returns a negative error code on failure.                                  */
int fat_find( int dir_fd, char *file_spec, u32_t flags, fatfs_find_t *findbuf)
{
  int      retval;
  fatfs_find_t find_res;
  u8_t     allow_attrs = (u8_t) flags;
  u8_t     req_attrs  = (u8_t) (flags >> 8);

  while ((retval = fat_do_readdir(dir_fd, &find_res)) == 0)
    if (((allow_attrs | find_res.SfnEntry.attribute) == allow_attrs)
     && ((req_attrs & find_res.SfnEntry.attribute) == req_attrs))
    {
      if (utf8_stricmp(find_res.l_long_name, file_spec) == 0)
      {
        if (findbuf) memcpy(findbuf, &find_res, sizeof(fatfs_find_t));
        return 0;
      }

      if (utf8_stricmp(find_res.l_long_name, file_spec) == 0)
      {
        if (findbuf) memcpy(findbuf, &find_res, sizeof(fatfs_find_t));
        return 0;
      }
    }
  return retval;
}

