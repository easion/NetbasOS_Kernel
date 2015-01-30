/*
**     (R)Jicama OS
**     Microsoft File Directory Table
**     Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"
#include <drv/errno.h>
#include <drv/ansi.h>
#include <assert.h>
#include <drv/log.h>

void fat_expand_name(struct msdos_dir *dirent, unsigned char* buf)
{
	//Copyright (c) 1996 Bart Sekura
   unsigned char* p;    /////copy file name to buffer
   unsigned char ext[4];

   memcpy(buf, dirent->file_name, 8);  //copy file name to buffer
   memcpy(ext, dirent->ext_name, 3); //strncpy old

   buf[8] = 0;
   ext[3] = 0;

   p = buf;
   while((*p != 0) && (*p != ' '))
      p++;  //delete space

   if((*ext != 0) && (*ext != ' '))
	{
      unsigned char* e = ext;
      *p++ = '.';  //add dot
      while((*e != 0) && (*e != ' '))
         *p++ = *e++;
   }

   *p = 0;  //last char will be zero.
}

void set_ffattribute(struct msdos_dir *item, int attribute)
 {
    item->attribute = attribute;
 }

void set_fftime(struct fdate* fdate, struct ftime* ftime)
 {
	struct tm chgtm;

    do_gettime(&chgtm);

	if(ftime){
	ftime->hour = chgtm.hour;
	ftime->min = chgtm.min;
	ftime->sec = chgtm.sec/2;
	}

	if(fdate){
	 fdate->year = chgtm.year+2000-1980;
     fdate->month = chgtm.month;
	 fdate->day_of_month = chgtm.day;
	}
 }



int node_to_dosdir(	inode_t *__temp,	struct msdos_dir *__md)
{
	__md->first_cluster = __temp->i_number;
	__md->file_size = __temp->i_size;
	//__md->attribute = __temp->i_flag;
    return OK;
}


void filetostr(struct msdos_dir *dir,char *str)
{
   int i=0,i2=0;

   assert(dir != NULL);

   for (i=0;i<8;i++)
   {
      if (dir->file_name[i]==' ') break;
      str[i2]=tolower(dir->file_name[i]);i2++;
   };
   
   if (dir->ext_name[0]==' '&&
	   dir->ext_name[1]==' '&&
	   dir->ext_name[2]==' ')
      {str[i2]=0;return;};

   if (!MSDOS_ISDIR(dir->attribute))
      str[i2]='.';i2++;
   for (i=0;i<3;i++)
   {
      if (dir->ext_name[i]==' ') break;
      str[i2]=tolower(dir->ext_name[i]);i2++;
   };
   str[i2]=0;
   str[13]=0;
};


void strtofile(const char *str,char *s)
{
   int makeextension=0;
   int i,i2=0,i3;
   
   for (i=0;i<8&&str[i];i++)
   {
      if (str[i]=='.')
		  {makeextension=1;i2=i+1;break;};
      s[i]=toupper(str[i]);
   };
   
   for (;i<8;i++) s[i]=' ';
   
   i=0;
   
   if  (!makeextension)
   for (i3=0;str[i3];i3++)
      if (str[i3]=='.') 
	   {makeextension=1;i2=i3+1;break;};
   
   if (makeextension)
   for (i=0; i<3 && str[i2] ; i++)
      s[8+i]=toupper(str[i2++]);
   
   for (;i<3;i++) s[8+i]=' ';
;};


