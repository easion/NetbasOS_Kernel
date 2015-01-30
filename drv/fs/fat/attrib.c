/*
**     (R)Jicama OS
**     Microsoft FAT file Create
**     Copyright (C) 2003 DengPingPing
*/

#include <drv/drv.h>
#include <drv/fs.h>
#include "fat.h"


int msdos_check_name(char *filename)
      {
         int i;

         for (i=0;filename[i]&&i<12;i++)
         {
            if (filename[i]=='!' ||
               filename[i]=='*' ||
               filename[i]=='?' ||
               filename[i]=='\\' ||
               filename[i]=='/' ||
               filename[i]=='|' ||
               filename[i]=='>' ||
            filename[i]=='<' )
            return -1;
         };

         return 0;
      };

void msdos_get_attr(struct msdos_dir *entry, char * attrchar)
{
  strcpy(attrchar, "-r----");
  if (entry->attribute&MSDOS_DIR)
    attrchar[0]='d';
  if (!(entry->attribute&MSDOS_READONLY))
    attrchar[2]='w';
  if (entry->attribute&MSDOS_ARCH)
    attrchar[3]='a';
  if (entry->attribute&MSDOS_VOLUME)
    attrchar[4]='s';
  if (entry->attribute&MSDOS_HIDDEN)
    attrchar[5]='h';
  attrchar[6]='\0';

}





