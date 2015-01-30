#ifndef IDE_HARDDISK_H
#define IDE_HARDDISK_H

////////////////////////////////AT-硬盘控制寄存器. //////////////////////////
#define HD_DATA		0x1f0	/* _CTL配置控制寄存器 when writing */
#define HD_ERROR	0x1f1	//////////////参考错误位
#define HD_NSECTOR	0x1f2	///////////读写的扇区数
#define HD_SECTOR	0x1f3	///////////////////////开始扇区
#define HD_LCYL		0x1f4	////////////////// 开始柱面
#define HD_HCYL		0x1f5	/////////////开始柱面的高字节
#define HD_CURRENT	0x1f6	/* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS	0x1f7	///////////////// 状态位 , 也是命令寄存器
#define HD_PRECOMP HD_ERROR	/* same io address, read=error, write=precomp */
#define HD_COMMAND HD_STATUS	/* same io address, read=status, write=cmd */

#define HD_CMD		0x3f6

///////硬盘状态////////////////////////////////////
#define ERR_STAT	0x01
#define INDEX_STAT	0x02
#define ECC_STAT	0x04	/* Corrected error */
#define DRQ_STAT	0x08
#define SEEK_STAT	0x10
#define WRERR_STAT	0x20
#define READY_STAT	0x40
#define BUSY_STAT	0x80         ///////////忙碌

//////////////////硬盘命令值///////////////////////////////
#define WIN_RESTORE		0x10
#define WIN_READ		0x20
#define WIN_WRITE		0x30
#define WIN_VERIFY		0x40
#define WIN_FORMAT		0x50
#define WIN_INIT		0x60
#define WIN_SEEK 		0x70
#define WIN_DIAGNOSE		0x90
#define WIN_SPECIFY		0x91

////////////////////////错误位///////////////////////////////////////////////
#define MARK_ERR	0x01	/* Bad address mark ? */
#define TRK0_ERR	0x02	/* couldn't find track 0 */
#define ABRT_ERR	0x04	/* ? */
#define ID_ERR		0x10	/* ? */
#define ECC_ERR		0x40	/* ? */
#define	BBD_ERR		0x80	/* ? */
#define NR_HD ((sizeof (hd_info))/(sizeof (struct harddisk_struct)))

/////////////////////////////数据结构//////////////////////////////////////////
 struct harddisk_struct
{
	int head;     ////////头
	int sect;        //////扇区
	int cyl;          /////////柱面
};

 struct hd_request {
	int hd;		/* -1 if no request */
	int nsector;     /////总扇区数
	int sector;     //////扇区数
	int head;      ////磁头
	int cyl;          ////////柱面
	int cmd;   ////////写入的命令
	int errors;     ///返回的错误   
	unsigned long buffer;
	struct hd_request *next;  /////////下一个读写请求
} request[32];

#define insw(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))

#define outsw(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))

#endif
