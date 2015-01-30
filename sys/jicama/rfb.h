
// ----------------------------------------------------------------------------------------------------------------------------
//Jicama Operating System
// Copyright (C) 2003  DengPingPing      All rights reserved.  
//------------------------------------------------------------------------------------------------------------------------------
#ifndef _RFB_H_
#define _RFB_H_

/*
//CLIENT TO SERVER MESSAGES:
*/
//SetPixelFormat
#define RFB_SPF_EVENT 0
//SetEncodings
#define RFB_SET_ENCODING 2
//FramebufferUpdateRequest
#define RFB_UPDATE_EVENT 3
//KeyEvent
#define RFB_KEY_EVENT 4
//PointerEvent
#define RFB_MOUSE_EVENT 5
//ClientCutText
#define RFB_CCT_EVENT 6

/*
**SERVER TO CLIENT MESSAGES
*/
//FramebufferUpdate
#define SRFB_UPDATE_EVENT 0
//SetColourMapEntries
#define SRFB_SCM_EVENT 1
//Ring a bell on the client if it has one.
#define SRFB_BELL_EVENT 2
//ServerCutText
#define SRFB_SCT_EVENT 3

/*
**Encodings
*/
//Raw
#define SRFB_RAW_ENCODE 0
//CopyRect
#define SRFB_COPYRECT_ENCODE 1
//RRE
#define SRFB_RRE_ENCODE 2
//Hextile
#define SRFB_HEX_ENCODE 4
//ZRLE
#define SRFB_ZRLE_ENCODE 5
//Cursor
#define SRFB_CURSOR_ENCODE -239
//desktop size
#define SRFB_DESKTOP_SZ_ENCODE -223


struct RFB_init_event
{
   u16_t width;
   u16_t height;
   u8_t  bitsPerPixel;
   u8_t  depth;
   u8_t  bigEndian;
   u8_t  trueColor;
   u16_t maxRed;
   u16_t maxGreen;
   u16_t maxBlue;
   u8_t  shrRed;
   u8_t  shrGreen;
   u8_t  shrBlue;
   u8_t  padding[3];
   u32_t nameLength;
   u8_t  name[256];
};

struct RFB_key_event
{
  u8_t  msgType;
  u8_t  down;
  u16_t padding;
  u32_t key;
};

struct RFB_mouse_event
{
  u8_t  msgType;
  u8_t  buttons;
  u16_t x;
  u16_t y;
};

struct RFB_update_event
{
  u8_t  msgType;
  u8_t  incremental;
  u16_t x;
  u16_t y;
  u16_t width;
  u16_t height;
};

struct RFB_resh_event
{
	u16_t x;
	u16_t y;
	u16_t width;
	u16_t height;
	u32_t enc;
};

int sendKeyEvent( int key, int down);
int sendPointerEvent( int x, int y, int buttons);

int recv_rfb_event(void *buf, int len);
int send_rfb_event(const void *buf, int len);

typedef struct kvideo_t
{
	int width;
	int height;
	int depth;
	int bpl;
	unsigned long lfb_base;
	int mode_index;
	int             red_length   ;
	int             green_length ;
	int             blue_length  ;
	int             red_offset  ;
	int             green_offset;
	int             blue_offset ;
}kvideo_t;

static inline unsigned long bswap(unsigned long x)
{
   __asm__ __volatile__("bswap %0" : "=r"(x) : "0"(x));
   return x;
}

static inline unsigned short bswap16(unsigned short x)
{
   __asm__ __volatile__("xchgb %b0,%h0" : "=q"(x) : "0"(x));
   return x;
}

#define htonl bswap
#define ntohl bswap
#define htons bswap16
#define ntohs bswap16
#define HTONS(_x_) ( (((_x_) >> 8) & 0xFF) | (((_x_) & 0xFF) << 8) )
#define HTONL(_x_) ( (((_x_) >> 24) & 0xFF) | (((_x_) >> 8) & 0xFF00) \
			| (((_x_) & 0xFF00) << 8) | (((_x_) & 0xFF) << 24) )


#endif

