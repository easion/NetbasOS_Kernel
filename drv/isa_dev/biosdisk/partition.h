#ifndef _PARTITION_H
#define _PARTITION_H

#define ACTIVE_FLAG	0x80	////活动的引导字段值 
#define NR_PARTITIONS	4	////分区表入口数 
#define	PART_TABLE_OFF	0x1BE	////启动扇区分区表偏移 
#define NO_PART		0x00	////未使用项 
#define OLD_MINIX_PART	0x80	////created before 1.4b, obsolete 

////分区表入口描叙符Description of entry in partition table.  
struct partition {
  unsigned char indicator;	////启动指示器 
  unsigned char start_head;	////首个扇区头值	 
  unsigned char start_sec;	////扇区sector value + cyl bits for first sector 
  unsigned char start_cyl;	//首个扇区堆栈 track value for first sector	 
  unsigned char type;		////系统指示器system indicator		 
  unsigned char last_head;	////head value for最后扇区 last sector	 
  unsigned char last_sec;	////扇区值sector value + cyl bits for last sector 
  unsigned char last_cyl;	////track value for last sector	 
  unsigned long lowsec;		//逻辑首个扇区 		 
  unsigned long size;		////分区扇区大小 
};

struct disk_partition{ //磁盘分区表
        char name;     ////5个文字hda1 .. hda10...
		unsigned char flag;
        unsigned char* type;
	    unsigned char start_sect; /////开始扇区
	     long nr_sects;    ///////扇区总数
	   unsigned long lowsec;
	 //  struct partition* p;
};

#endif ////_PARTITION_H 
