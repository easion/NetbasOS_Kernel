
#ifndef _mouse_h_
#define _mouse_h_

#ifdef __cplusplus
extern "C" {
#endif


#define MOUSE_BUTTON_LEFT 0x01
#define MOUSE_BUTTON_MIDDLE 0x02
#define MOUSE_BUTTON_RIGHT 0x04

typedef struct _mouse__ {
	volatile int dx;
	volatile int dy;
	volatile unsigned int b;
	unsigned char changed;
	 char show;//mouse 光标显示/不显示
    
} mouse_t;

unsigned char ps2_read();
 void ps2_write(unsigned char data);
void ps2_cmd(unsigned char command);


extern mouse_t mouse;
#endif


