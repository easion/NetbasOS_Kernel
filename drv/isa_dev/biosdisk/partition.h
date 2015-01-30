#ifndef _PARTITION_H
#define _PARTITION_H

#define ACTIVE_FLAG	0x80	////��������ֶ�ֵ 
#define NR_PARTITIONS	4	////����������� 
#define	PART_TABLE_OFF	0x1BE	////��������������ƫ�� 
#define NO_PART		0x00	////δʹ���� 
#define OLD_MINIX_PART	0x80	////created before 1.4b, obsolete 

////��������������Description of entry in partition table.  
struct partition {
  unsigned char indicator;	////����ָʾ�� 
  unsigned char start_head;	////�׸�����ͷֵ	 
  unsigned char start_sec;	////����sector value + cyl bits for first sector 
  unsigned char start_cyl;	//�׸�������ջ track value for first sector	 
  unsigned char type;		////ϵͳָʾ��system indicator		 
  unsigned char last_head;	////head value for������� last sector	 
  unsigned char last_sec;	////����ֵsector value + cyl bits for last sector 
  unsigned char last_cyl;	////track value for last sector	 
  unsigned long lowsec;		//�߼��׸����� 		 
  unsigned long size;		////����������С 
};

struct disk_partition{ //���̷�����
        char name;     ////5������hda1 .. hda10...
		unsigned char flag;
        unsigned char* type;
	    unsigned char start_sect; /////��ʼ����
	     long nr_sects;    ///////��������
	   unsigned long lowsec;
	 //  struct partition* p;
};

#endif ////_PARTITION_H 
