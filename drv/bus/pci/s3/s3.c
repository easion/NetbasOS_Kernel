/* $Id: s3.c,v 1.2 2003/06/05 21:59:53 pavlovskii Exp $ */

/*
 * Mostly hacked from S3 Trio64 Linux framebuffer driver written by 
 *  Hannu Mallat <hmallat@cs.hut.fi>.
 */

#include <kernel/kernel.h>
#include <kernel/arch.h>
#include <kernel/driver.h>
#include <kernel/memory.h>
#include <kernel/vmm.h>

#include <wchar.h>

#include "video.h"
#include "s3.h"
#include "bpp8.h"

void swap_int(int *a, int *b);

//#define TRIO_ACCEL
#define PCI_VENDOR_ID_S3        0x5333
#define PCI_DEVICE_ID_S3_TRIO   0x8811

#define vga_inb(reg)        in(reg)
#define vga_outb(reg, dat)  out(reg, (dat) & 0xff)
#define trio_outw(idx, val) out16(idx, (val) & 0xffff)
#define trio_inw(idx)       in16(idx)

static void *trio_base;
static void *trio_base_global;
static unsigned long board_size;
static long trio_memclk = 45000000; /* default (?) */

/*void udelay(unsigned us)
{
    volatile int a;
    a = in(0x80);
}*/

void gra_outb(unsigned short idx, unsigned char val) {
  vga_outb(GCT_ADDRESS,   idx); 
  vga_outb(GCT_ADDRESS_W, val);
}

void seq_outb(unsigned short idx, unsigned char val) {
  vga_outb(SEQ_ADDRESS,   idx); 
  vga_outb(SEQ_ADDRESS_W, val);
}

void crt_outb(unsigned short idx, unsigned char val) {
  vga_outb(CRT_ADDRESS,   idx); 
  vga_outb(CRT_ADDRESS_W, val);
}

void att_outb(unsigned short idx, unsigned char val) {
  unsigned char tmp;
  tmp = vga_inb(ACT_ADDRESS_RESET);
  vga_outb(ACT_ADDRESS_W, idx);
  vga_outb(ACT_ADDRESS_W, val);
} 

/*unsigned char att_inb(unsigned short idx) {
  vga_outb(ACT_ADDRESS_W, idx);
  udelay(100);
  return vga_inb(ACT_ADDRESS_R);
}*/

unsigned char seq_inb(unsigned short idx) {
  vga_outb(SEQ_ADDRESS, idx);
  return vga_inb(SEQ_ADDRESS_R);
}

unsigned char crt_inb(unsigned short idx) {
  vga_outb(CRT_ADDRESS, idx);
  return vga_inb(CRT_ADDRESS_R);
}

unsigned char gra_inb(unsigned short idx) {
  vga_outb(GCT_ADDRESS, idx);
  return vga_inb(GCT_ADDRESS_R);
}

#define s3WriteMulti(index, data)   trio_outw(S3_MULT_MISC, ((index) << 12) | ((data) & 0xfff))

typedef struct
{
    videomode_t mode;
    /* Timing: All values in pixclocks, except pixclock (of course) */
    uint32_t pixclock;          /* pixel clock in ps (pico seconds) */
    uint32_t left_margin;       /* time from sync to picture    */
    uint32_t right_margin;      /* time from picture to sync    */
    uint32_t upper_margin;      /* time from sync to picture    */
    uint32_t lower_margin;
    uint32_t hsync_len;         /* length of horizontal sync    */
    uint32_t vsync_len;         /* length of vertical sync      */
} s3mode_t;

#define S3_FB_NAME L"fb_s3"

static s3mode_t s3_modes[] =
{
    /* 640x480x8 works fine */
    {
		{ 0,  640, 480, 8, 0, VIDEO_MODE_GRAPHICS, BPP8_ACCEL, S3_FB_NAME, }, 
		39722,  40, 24, 32, 11,  96, 2
	},

    /* xxx - 800x600 gives a black screen */
    /*{ { 1,  800, 600, 8, 0, VIDEO_MODE_GRAPHICS, S3_FB_NAME, }, 27778,  64, 24, 22,  1,  72, 2 },*/

    /* xxx - 1024x768 has a migraine-inducingly slow refresh rate */
    /*{ { 2, 1024, 768, 8, 0, VIDEO_MODE_GRAPHICS, S3_FB_NAME, }, 16667, 224, 72, 60, 12, 168, 4 },*/

    /* xxx -- 640x480x16 flickers when video RAM is written to */
    /*{ { 3,  640, 480, 16, 0, VIDEO_MODE_GRAPHICS, S3_FB_NAME, }, 39722,  40, 24, 32, 11,  96, 2 },*/
};


static int s3EnumModes(void *cookie, unsigned index, videomode_t *mode)
{
    if (index < _countof(s3_modes))
	{
        s3_modes[index].mode.bytesPerLine = 
            (s3_modes[index].mode.width * s3_modes[index].mode.bitsPerPixel) / 8;
        *mode = s3_modes[index].mode;
        return index == _countof(s3_modes) - 1 ? VID_ENUM_STOP : VID_ENUM_CONTINUE;
    }
	else
		return VID_ENUM_ERROR;
}


static void Trio_WaitQueue(void) {
  uint16_t status;
  do { status = trio_inw(S3_GP_STAT); }  while(!(status & S3_FIFO_EMPTY));
}


static void Trio_WaitBlit (void) {
  uint16_t status;
  do { status = trio_inw(S3_GP_STAT); } while (status & S3_HDW_BUSY);
}


static void Trio_WaitIdle (void) {
  Trio_WaitQueue();
  Trio_WaitBlit();
}


#include "cur_hand.c"


void s3MoveCursor(void *cookie, point_t pt)
{
    pt.x -= hotspot_x;
    pt.y -= hotspot_y;
    
    if (pt.x < 0)
    {
        crt_outb(CRT_ID_HWGC_DSTART_X, -pt.x);
        pt.x = 0;
    }
    else
        crt_outb(CRT_ID_HWGC_DSTART_X, 0);
    
    if (pt.y < 0)
    {
        crt_outb(CRT_ID_HWGC_DSTART_Y, -pt.y);
        pt.y = 0;
    }
    else
        crt_outb(CRT_ID_HWGC_DSTART_Y, 0);

    crt_outb(CRT_ID_HWGC_ORIGIN_X_LO, (char)(pt.x & 0x00ff));
    crt_outb(CRT_ID_HWGC_ORIGIN_X_HI, (char)((pt.x & 0x0700) >> 8));
    crt_outb(CRT_ID_HWGC_ORIGIN_Y_LO, (char)(pt.y & 0x00ff));
    crt_outb(CRT_ID_HWGC_ORIGIN_Y_HI, (char)((pt.y & 0x0700) >> 8));
}


inline void trio_video_disable(int toggle) {
  int r;
  
  toggle &= 0x1;
  toggle = toggle << 5;
  
  r = (int) seq_inb(SEQ_ID_CLOCKING_MODE);
  r &= 0xdf;/* Set bit 5 to 0 */
  
  seq_outb(SEQ_ID_CLOCKING_MODE, r | toggle);
}


/*
 * Computes M, N, and R values from
 * given input frequency. It uses a table of
 * precomputed values, to keep CPU time low.
 *
 * The return value consist of:
 * lower byte:  Bits 4-0: N Divider Value
 *        Bits 5-6: R Value          for e.g. SR10 or SR12
 * higher byte: Bits 0-6: M divider value  for e.g. SR11 or SR13
 */
static unsigned short trio_compute_clock(unsigned long freq) {
  /*
   * Calculate the video clocks;
   *
   * Fout = Fref * (M + 2) / ((N + 2) * 2^R)
   * 
   *      where 0 <= R <= 3, 1 <= N <= 31, 1 <= M <= 127, and
   *      135 MHz <= ((M + 2)*Fref) / (N + 2) <= 270 MHz
   *      
   */
    
  double Fref = 14.31818;
  double Fout = ((double)freq) / 1.0e6;
  uint32_t N, M, R;
  uint32_t mnr;

do_clocks:

  for(R = 0; R < 4; R++)
    if(((double)(1 << R))*Fout >  135.0 &&
       ((double)(1 << R))*Fout <= 270.0)
      break;

  if(R == 4) {
    wprintf(L"s3: Unsupported clock freq %ld, using 25MHz\n", 
           (long)(Fout*1.0e6));
    Fout = 25.000;
    goto do_clocks;
  }

  for(N = 1; N < 32; N++) {
    double Ftry;
    M = (int)(Fout*((double)(N + 2))*((double)(1 << R))/Fref - 2.0);
    Ftry = ((double)(M + 2))*Fref/(((double)(N + 2))*((double)(1 << R)));
    if(0.995*Fout < Ftry && 1.005*Fout > Ftry)
      break;
  }
  
  if(N == 32) {
    wprintf(L"s3: Unsupported clock freq %ld, using 25MHz\n", 
           (long)(Fout*1.0e6));
    Fout = 25.000;
    goto do_clocks;
  }
  
  mnr = ((M & 0x7f) << 8) | ((R & 0x03) << 5) | (N & 0x1f);
  return mnr;
}

#ifdef TRIO_ACCEL
static void s3FillRect16(video_t *vid, const clip_t *clips, int x1, int y1, int x2, int y2, 
                         colour_t clr)
{
    unsigned i;
    rect_t *rect;
    uint16_t pix;

    pix = s3ColourToPixel16(clr);
    for (i = 0, rect = clips->rects; i < clips->num_rects; i++, rect++)
    {
        Trio_WaitIdle();

        s3WriteMulti(S3_MULT_SCISSORS_T, rect->top);
        s3WriteMulti(S3_MULT_SCISSORS_L, rect->left);
        s3WriteMulti(S3_MULT_SCISSORS_B, rect->bottom - 1);
        s3WriteMulti(S3_MULT_SCISSORS_R, rect->right - 1);

        trio_outw(S3_PIXEL_CNTL, 0xa000);
        trio_outw(S3_FRGD_COLOR, pix);
        trio_outw(S3_FRGD_MIX,   0x0027);
        trio_outw(S3_WRT_MASK,   0xffff);

        Trio_WaitIdle();
        trio_outw(S3_CUR_X, x1 & 0x0fff);
        trio_outw(S3_CUR_Y, y1 & 0x0fff);
        trio_outw(S3_MIN_AXIS_PCNT, (y2 - y1 - 1) & 0x0fff);
        trio_outw(S3_MAJ_AXIS_PCNT, (x2 - x1 - 1) & 0x0fff);
        trio_outw(S3_CMD, S3_FILLEDRECT);
    }
}
#endif


static void trio_load_video_mode (s3mode_t *mode)
{
    int fx, fy;
    unsigned short mnr;
    unsigned short HT, HDE, HBS, HBE, HSS, HSE, VDE, VBS, VBE, VSS, VSE, VT;
    unsigned short FIFO;
    int cr50, sr15, sr18, clock_mode, test;
    int m, n;
    int tfillm, temptym;
    int hmul;

    /* ---------------- */
    int xres, hfront, hsync, hback;
    int yres, vfront, vsync, vback;
    int bpp;
    long freq;
    /* ---------------- */

    fx = fy = 8;/* force 8x8 font */

/* GRF - Disable interrupts */

    switch (mode->mode.bitsPerPixel) {
    case 15:
    case 16:
        hmul = 2;
        break;                                
    default:
        hmul = 1;
        break;
    }
                
    trio_video_disable(1);

    bpp    = mode->mode.bitsPerPixel;
    xres   = mode->mode.width;
    hfront = mode->right_margin;
    hsync  = mode->hsync_len;
    hback  = mode->left_margin;
    
    yres   = mode->mode.height;
    vfront = mode->lower_margin;
    vsync  = mode->vsync_len;
    vback  = mode->upper_margin;
    
    HBS = HDE = hmul*(xres)/8 - 1;
    HSS =       hmul*(xres + hfront)/8 - 1;
    HSE =       hmul*(xres + hfront + hsync)/8 - 1;
    HBE = HT  = hmul*(xres + hfront + hsync + hback)/8 - 1;
    HBE = HSE; /* weirdness appeared if HBE = HT */

    FIFO = (HBS + (HBE - 5))/2;

    VBS = VDE =   yres - 1;
    VSS =         yres + vfront - 1;
    VSE =         yres + vfront + vsync - 1;
    VBE = VT    = yres + vfront + vsync + vback - 2;

    crt_outb(0x11, crt_inb(0x11) & 0x7f); /* unlock crt 0-7 */

    /* xxx -- enable hardware cursor */
    /*crt_outb(CRT_ID_HWGC_MODE, crt_inb(CRT_ID_HWGC_MODE) | 1);

    switch (mode->mode.bitsPerPixel)
    {
    default:
    case 8:
        crt_outb(CRT_ID_HWGC_FG_STACK, 0);
        crt_outb(CRT_ID_HWGC_BG_STACK, 15);
        break;

    case 16:
        crt_outb(CRT_ID_HWGC_FG_STACK, s3ColourToPixel16(0x000000));
        crt_outb(CRT_ID_HWGC_BG_STACK, s3ColourToPixel16(0xffffff));
        break;
    }*/
    
    crt_outb(CRT_ID_EXT_DAC_CNTL, 0x00);

    /* sequential addressing, chain-4 */
    seq_outb(SEQ_ID_MEMORY_MODE, 0x0e);

    gra_outb(GCT_ID_READ_MAP_SELECT, 0x00);
    seq_outb(SEQ_ID_MAP_MASK,        0xff);
    seq_outb(SEQ_ID_CHAR_MAP_SELECT, 0x00);

    /* trio_compute_clock accepts arguments in Hz */
    /* pixclock is in ps ... convert to Hz */
    freq = (1000000000 / mode->pixclock) * 1000;
    
    mnr = trio_compute_clock (freq);
    seq_outb(SEQ_ID_DCLK_HI, ((mnr & 0xFF00) >> 8));
    seq_outb(SEQ_ID_DCLK_LO, (mnr & 0xFF));

    crt_outb(CRT_ID_MEMORY_CONF,        0x08); 
    crt_outb(CRT_ID_BACKWAD_COMP_1, 0x00); /* - */
    crt_outb(CRT_ID_REGISTER_LOCK,    0x00);
    crt_outb(CRT_ID_EXT_MODE,             0x00);

    /* Load display parameters into board */
    crt_outb(CRT_ID_EXT_HOR_OVF,
             (((HT - 4) & 0x100) ? 0x01 : 0x00)    |
             ((HDE & 0x100) ? 0x02 : 0x00) |
             ((HBS & 0x100) ? 0x04 : 0x00) |
             /* ((HBE & 0x40) ? 0x08 : 0x00)    | */
             ((HSS & 0x100) ? 0x10 : 0x00) |
             /* ((HSE & 0x20) ? 0x20 : 0x00)    | */
             ((FIFO & 0x100) ? 0x40 : 0x00)
             );

    crt_outb(CRT_ID_EXT_VER_OVF,
             0x40 |
             ((VT & 0x400) ? 0x01 : 0x00) |
             ((VDE & 0x400) ? 0x02 : 0x00) |
             ((VBS & 0x400) ? 0x04 : 0x00) |
             ((VSS & 0x400) ? 0x10 : 0x00)
             );
    
    crt_outb(CRT_ID_HOR_TOTAL, HT - 4);
    crt_outb(CRT_ID_DISPLAY_FIFO, FIFO);
    crt_outb(CRT_ID_HOR_DISP_ENA_END, HDE);
    crt_outb(CRT_ID_START_HOR_BLANK, HBS);
    crt_outb(CRT_ID_END_HOR_BLANK, (HBE & 0x1F));
    crt_outb(CRT_ID_START_HOR_RETR, HSS);
    crt_outb(CRT_ID_END_HOR_RETR,
             (HSE & 0x1F) |
             ((HBE & 0x20) ? 0x80 : 0x00)
             );
    crt_outb(CRT_ID_VER_TOTAL, VT);
    crt_outb(CRT_ID_OVERFLOW,
             0x10 |
             ((VT & 0x100) ? 0x01 : 0x00)    |
             ((VDE & 0x100) ? 0x02 : 0x00) |
             ((VSS & 0x100) ? 0x04 : 0x00) |
             ((VBS & 0x100) ? 0x08 : 0x00) |
             ((VT & 0x200) ? 0x20 : 0x00)    |
             ((VDE & 0x200) ? 0x40 : 0x00) |
             ((VSS & 0x200) ? 0x80 : 0x00)
             );
    crt_outb(CRT_ID_MAX_SCAN_LINE,
             0x40 |
             ((VBS & 0x200) ? 0x20 : 0x00)
             );

    crt_outb(CRT_ID_MODE_CONTROL, 0xe3); /* . */

    crt_outb(CRT_ID_UNDERLINE_LOC, 0x00);

    /* start address */
    crt_outb(CRT_ID_EXT_SYS_CNTL_3,    0x00);
    crt_outb(CRT_ID_START_ADDR_HIGH, 0x00);
    crt_outb(CRT_ID_START_ADDR_LOW,    0x00);

    crt_outb(CRT_ID_START_VER_RETR, VSS);
    crt_outb(CRT_ID_END_VER_RETR, (VSE & 0x0F));
    crt_outb(CRT_ID_VER_DISP_ENA_END, VDE);
    crt_outb(CRT_ID_START_VER_BLANK, VBS);
    crt_outb(CRT_ID_END_VER_BLANK, VBE);
    crt_outb(CRT_ID_LINE_COMPARE, 0xFF);
    crt_outb(CRT_ID_LACE_RETR_START, HT / 2);
    crt_outb(CRT_ID_LACE_CONTROL, 0x00);
    gra_outb(GCT_ID_GRAPHICS_MODE, 0x40);
    gra_outb(GCT_ID_MISC, 0x01);
    seq_outb(SEQ_ID_MEMORY_MODE, 0x02);

    vga_outb(VDAC_MASK, 0xFF);

    /* Blank border */
    test = crt_inb(CRT_ID_BACKWAD_COMP_2);
    crt_outb(CRT_ID_BACKWAD_COMP_2, test | 0x20); /* - */

    sr15 = seq_inb(SEQ_ID_CLKSYN_CNTL_2);
    sr15 &= 0xEF;
    sr18 = seq_inb(SEQ_ID_RAMDAC_CNTL);
    sr18 &= 0x7F;
    clock_mode = 0x00;
    cr50 = 0x00;

    test = crt_inb(CRT_ID_EXT_MISC_CNTL_2);
    test &= 0xD;

    switch (mode->mode.bitsPerPixel) {
    case 8:
        if (freq > 80000000) {
            clock_mode = 0x10 | 0x02;
            sr15 |= 0x10;
            sr18 |= 0x80;
        }
        HDE = mode->mode.width/8;
        cr50 |= 0x00;
        break;
    case 15:
        clock_mode = 0x30;
        HDE = mode->mode.width / 4;
        cr50 |= 0x10;
        break;                        
    case 16:
        clock_mode = 0x50;
        HDE = mode->mode.width / 4;
        cr50 |= 0x10;
        break;
    }

    crt_outb(CRT_ID_EXT_MISC_CNTL_2, clock_mode | test);
    seq_outb(SEQ_ID_CLKSYN_CNTL_2, sr15);
    seq_outb(SEQ_ID_RAMDAC_CNTL, sr18);
    crt_outb(CRT_ID_SCREEN_OFFSET, HDE);

    crt_outb(CRT_ID_MISC_1, 0xb5); 

    test = (HDE >> 4) & 0x30;
    crt_outb(CRT_ID_EXT_SYS_CNTL_2, test);

    /* Set up graphics engine */
    switch (mode->mode.width) {
    case 1024:
        cr50 |= 0x00;
        break;

    case 640:
        cr50 |= 0x40;
        break;

    case 800:
        cr50 |= 0x80;
        break;

    case 1280:
        cr50 |= 0xC0;
        break;

    case 1152:
        cr50 |= 0x01;
        break;

    case 1600:
        cr50 |= 0x81;
        break;

    default:/* XXX */
        break;
    }

    crt_outb(CRT_ID_EXT_SYS_CNTL_1, cr50);

    att_outb(ACT_ID_ATTR_MODE_CNTL, 0x41);
    att_outb(ACT_ID_COLOR_PLANE_ENA, 0x0f);

    tfillm = (96 * (trio_memclk / 1000)) / 240000;

    switch (mode->mode.bitsPerPixel) {
    case 15:
    case 16:
        temptym = (48 * (trio_memclk / 1000)) / (freq / 1000);
        break;
    default:
        temptym = (96 * (trio_memclk / 1000)) / (freq / 1000);
        break;
    }

    m = (temptym - tfillm - 9) / 2;
    if (m < 0)
        m = 0;
    m = (m & 0x1F) << 3;
    if (m < 0x18)
        m = 0x18;
    n = 0xFF;

    crt_outb(CRT_ID_EXT_MEM_CNTL_2, m);
    crt_outb(CRT_ID_EXT_MEM_CNTL_3, n);

    att_outb(0x33, 0);

    /* Turn gfx on again */
    trio_video_disable(0);

#ifdef TRIO_ACCEL
    Trio_WaitIdle();
    trio_outw(S3_FRGD_MIX, 0x0007);
    trio_outw(S3_WRT_MASK, 0x0027);

    Trio_WaitIdle();
    trio_outw(S3_READ_REG_DATA, 0x1000); /* set clip rect */
    trio_outw(S3_READ_REG_DATA, 0x2000);
    trio_outw(S3_READ_REG_DATA, 0x3fff);
    trio_outw(S3_READ_REG_DATA, 0x4fff);

    Trio_WaitIdle();
    trio_outw(S3_RD_MASK, 0xffff);    /* set masks */
    trio_outw(S3_WRT_MASK, 0xffff);

#endif /* TRIO_ACCEL */
}


void _swab(char *src, char *dest, int nbytes)
{
    char b1, b2;

    while (nbytes > 1)
    {
        b1 = *src++;
        b2 = *src++;
        *dest++ = b2;
        *dest++ = b1;
        nbytes -= 2;
    }
}


void s3InitHardware(void)
{
    int i;
    unsigned char test;
    unsigned int clockpar;
    //volatile uint16_t *CursorBase;
    //unsigned cursor_off;
    //uint32_t a, b;

    /* make sure 0x46e8 accesses are responded to */
    crt_outb(CRT_ID_EXT_MISC_CNTL, crt_inb(CRT_ID_EXT_MISC_CNTL) & 0xfb);

    vga_outb(S3_VIDEO_SUBS_ENABLE, 0x10);
    vga_outb(S3_OPTION_SELECT,         0x01);
    vga_outb(S3_VIDEO_SUBS_ENABLE, 0x08);

    out16(S3_SUBSYS_CNTL, 0x8000); in(S3_SUBSYS_CNTL);/* reset accelerator */
    out16(S3_SUBSYS_CNTL, 0x4000); in(S3_SUBSYS_CNTL);/* enable accelerator */

#ifdef TRIO_MMIO
    out16(S3_ADVFUNC_CNTL, 0x0031);
    crt_outb(CRT_ID_EXT_MEM_CNTL_1, 0x18);
#else /* TRIO_MMIO */
    out16(S3_ADVFUNC_CNTL, 0x0011);
    crt_outb(CRT_ID_EXT_MEM_CNTL_1, 0x00);
#endif /* TRIO_MMIO */

    Trio_WaitIdle();
    out16(S3_MULT_MISC, 0xe000);
    out16(S3_MULT_MISC_2, 0xd000);

    if(board_size == 4096*1024) {
        crt_outb(CRT_ID_LAW_CNTL, 0x13);
    } else {
        crt_outb(CRT_ID_LAW_CNTL, 0x12);
    }

    seq_outb(SEQ_ID_CLOCKING_MODE,     0x01);
    seq_outb(SEQ_ID_MAP_MASK,                0x0f);
    seq_outb(SEQ_ID_CHAR_MAP_SELECT, 0x00);
    seq_outb(SEQ_ID_MEMORY_MODE,         0x0e);

    seq_outb(SEQ_ID_EXT_SEQ_REG9,        0x00);
    if((crt_inb(0x36) & 0x0c) == 0x0c && /* fast page mode */
         (crt_inb(0x36) & 0xe0) == 0x00) { /* 4Mb buffer */
        seq_outb(SEQ_ID_BUS_REQ_CNTL, seq_inb(SEQ_ID_BUS_REQ_CNTL) | 0x40);
    }

    /* Clear immediate clock load bit */
    test = seq_inb(SEQ_ID_CLKSYN_CNTL_2);
    test = test & 0xDF;
    /* If > 55MHz, enable 2 cycle memory write */
    if (trio_memclk >= 55000000) {
        test |= 0x80;
    }
    seq_outb(SEQ_ID_CLKSYN_CNTL_2, test);

    /* Set MCLK value */
    clockpar = trio_compute_clock(trio_memclk);
    test = (clockpar & 0xFF00) >> 8;
    seq_outb(SEQ_ID_MCLK_HI, test);
    test = clockpar & 0xFF;
    seq_outb(SEQ_ID_MCLK_LO, test);

    /* Chip rev specific: Not in my Trio manual!!! */
    if(crt_inb(CRT_ID_REVISION) == 0x10)
        seq_outb(SEQ_ID_MORE_MAGIC, test);

    /* Set DCLK value */
    seq_outb(SEQ_ID_DCLK_HI, 0x13);
    seq_outb(SEQ_ID_DCLK_LO, 0x41);

    /* Load DCLK (and MCLK?) immediately */
    test = seq_inb(SEQ_ID_CLKSYN_CNTL_2);
    test = test | 0x22;
    seq_outb(SEQ_ID_CLKSYN_CNTL_2, test);

    /* Enable loading of DCLK */
    test = vga_inb(GREG_MISC_OUTPUT_R);
    test = test | 0x0C;
    vga_outb(GREG_MISC_OUTPUT_W, test);

    /* Turn off immediate xCLK load */
    seq_outb(SEQ_ID_CLKSYN_CNTL_2, 0x2);

    gra_outb(GCT_ID_SET_RESET, 0x0);
    gra_outb(GCT_ID_ENABLE_SET_RESET, 0x0);
    gra_outb(GCT_ID_COLOR_COMPARE, 0x0);
    gra_outb(GCT_ID_DATA_ROTATE, 0x0);
    gra_outb(GCT_ID_READ_MAP_SELECT, 0x0);
    gra_outb(GCT_ID_GRAPHICS_MODE, 0x40);
    gra_outb(GCT_ID_MISC, 0x01);
    gra_outb(GCT_ID_COLOR_XCARE, 0x0F);
    gra_outb(GCT_ID_BITMASK, 0xFF);

    /* Colors for text mode */
    for (i = 0; i < 0xf; i++)
        att_outb(i, i);

    att_outb(ACT_ID_ATTR_MODE_CNTL, 0x41);
    att_outb(ACT_ID_OVERSCAN_COLOR, 0x01);
    att_outb(ACT_ID_COLOR_PLANE_ENA, 0x0F);
    att_outb(ACT_ID_HOR_PEL_PANNING, 0x0);
    att_outb(ACT_ID_COLOR_SELECT, 0x0);
    vga_outb(VDAC_MASK, 0xFF);

    crt_outb(CRT_ID_BACKWAD_COMP_3, 0x10);/* FIFO enabled */
    crt_outb(CRT_ID_HWGC_MODE, 0x00);     /* GFx hardware cursor off */

#if 0
    cursor_off = board_size - 0x400;
    CursorBase = (uint16_t *)((char *)(trio_base) + cursor_off);
    crt_outb(CRT_ID_HWGC_START_AD_HI, ((cursor_off / 1024) & 0xf00) >> 8);
    crt_outb(CRT_ID_HWGC_START_AD_LO,  (cursor_off / 1024) & 0x0ff);
    wprintf(L"s3: TrioMem = %p, board_size = %x, CursorBase = %p\n",
        TrioMem, board_size, CursorBase);

    att_outb(0x33, 0);
    trio_video_disable(0);    

    /* Initialize hardware cursor */
    /*for (i = 0; i < 8; i++)
    {
        *(CursorBase  +(i*4)) = 0xffffff00;
        *(CursorBase+1+(i*4)) = 0xffff0000;
        *(CursorBase+2+(i*4)) = 0xffff0000;
        *(CursorBase+3+(i*4)) = 0xffff0000;
    }

    for (i = 8; i < 64; i++)
    {
        *(CursorBase  +(i*4)) = 0xffff0000;
        *(CursorBase+1+(i*4)) = 0xffff0000;
        *(CursorBase+2+(i*4)) = 0xffff0000;
        *(CursorBase+3+(i*4)) = 0xffff0000;
    }*/

    for (i = 0; i < 256; i++)
    {
        CursorBase[i * 2 + 0] = 0xffff;
        CursorBase[i * 2 + 1] = 0x0000;
    }

    for (i = 0; i < _countof(cur_hand); i += j)
    {
        a = b = 0;
        for (j = 0; j < 32; j++)
        {
            a <<= 1;
            b <<= 1;

            /*
             *  The bits are interpreted as:
             *  A    B    MS-Windows:         X-11:
             *  0    0    Background          Screen data
             *  0    1    Foreground          Screen data
             *  1    0    Screen data         Background
             *  1    1    Inverted screen     Foreground
             */
            switch (cur_hand[i + j])
            {
            case B:
                a |= 0;
                b |= 0;
                break;

            case F:
                a |= 0;
                b |= 1;
                break;

            case _:
                a |= 1;
                b |= 0;
                break;

            case I:
                a |= 1;
                b |= 1;
                break;
            }
        }

        /* xxx - Why are these bytes swapped? Some S3 weirdness... */
        _swab((char*) &a, (char*) &a, sizeof(a));
        _swab((char*) &b, (char*) &b, sizeof(b));

        CursorBase[i / 4 + 0] = a >> 16;
        CursorBase[i / 4 + 1] = b >> 16;
        CursorBase[i / 4 + 2] = a;
        CursorBase[i / 4 + 3] = b;
    }
#endif
}

#undef F
#undef B
#undef I
#undef _

static bool s3EnterMode(void *cookie, videomode_t *mode, 
						const surface_vtbl_t **surf_vtbl, void **surf_cookie)
{
    bool found;
    unsigned i;
    
    found = false;
    for (i = 0; i < _countof(s3_modes); i++)
        if (s3_modes[i].mode.cookie == mode->cookie)
        {
            found = true;
            break;
        }

    if (!found)
        return false;

    s3InitHardware();
    trio_load_video_mode(s3_modes + i);

    if (mode->bitsPerPixel == 8)
    {
        VidStoreVgaPalette(cookie, Bpp8GetPalette(), 0, 256);
		return Bpp8CreateSurface(mode, trio_base_global, surf_vtbl, surf_cookie);
    }
	else
		return false;
}


static void s3LeaveMode(void *cookie)
{
}


static const video_vtbl_t s3_vtbl =
{
	s3EnumModes,
	s3EnterMode,
	s3LeaveMode,
	VidStoreVgaPalette,
	s3MoveCursor,
};


static bool s3AddDevice(dev_config_t *cfg, const video_vtbl_t **vtbl, void **cookie)
{
    dev_resource_t *res;
    pci_businfo_t *pci;
    addr_t board_addr;
    unsigned long board_size;
    unsigned char tmp, board_type;

    if (cfg == NULL)
    {
        wprintf(L"s3: no PCI configuration present\n");
        return false;
    }

    if (cfg->bus_type != DEV_BUS_PCI)
    {
        wprintf(L"s3: not a PCI card\n");
        return false;
    }

    pci = cfg->businfo;
    if (pci->vendor_id != PCI_VENDOR_ID_S3)
    {
        wprintf(L"s3: vendor (%x) is not S3\n", pci->vendor_id);
        return false;
    }

    if (pci->device_id != PCI_DEVICE_ID_S3_TRIO)
    {
        wprintf(L"s3: device (%x) is not S3 Trio\n", pci->device_id);
        return false;
    }

    /* goto color emulation, enable CPU access */
    vga_outb(GREG_MISC_OUTPUT_W, vga_inb(GREG_MISC_OUTPUT_R) | 0x03);

    /* unlock registers */
    crt_outb(CRT_ID_REGISTER_LOCK_1, 0x48);
    crt_outb(CRT_ID_REGISTER_LOCK_2, 0xa5);
    crt_outb(CRT_ID_SYSTEM_CONFIG, crt_inb(CRT_ID_SYSTEM_CONFIG) | 0x01);
    crt_outb(CRT_ID_REGISTER_LOCK, crt_inb(CRT_ID_REGISTER_LOCK) & ~0x30);
    seq_outb(SEQ_ID_UNLOCK_EXT, 0x06);
    crt_outb(CRT_ID_BACKWAD_COMP_2, 
           (crt_inb(CRT_ID_BACKWAD_COMP_2) & ~(0x2 | 0x10 |  0x40)) | 0x20);

    /* get Trio identification, 0x10 = trio32, 0x11 = trio64 */
	board_type = crt_inb(CRT_ID_DEVICE_LOW);
    switch (board_type)
	{
	case 0x10:
		wprintf(L"s3: got S3 Trio32\n");
		break;

	case 0x11:
		wprintf(L"s3: got S3 Trio64\n");
		break;

	default:
		wprintf(L"s3: got unknown S3 card %x\n", board_type);
		return false;
	}

    tmp = crt_inb(CRT_ID_CONFIG_1);
    board_size = 
        (tmp & 0xe0) == 0x00 ? 4096*1024 :
        (tmp & 0xe0) == 0x80 ? 2048*1024 :
        (tmp & 0xe0) == 0xc0 ? 1024*1024 : 512*1024;

    res = DevCfgFindMemory(cfg, 0);
    if (res)
        board_addr = res->u.memory.base;
    else
    {
        wprintf(L"s3: no memory resource found\n");
        /* xxx - default for my card */
        board_addr = 0xFE000000;
    }

    trio_base = VmmMap(PAGE_ALIGN_UP(board_size) / PAGE_SIZE,
        NULL, (void*) board_addr, NULL, VM_AREA_MAP, 
        VM_MEM_USER | VM_MEM_READ | VM_MEM_WRITE);
	trio_base_global = sbrk_virtual(PAGE_ALIGN_UP(board_size));
	MemMapRange(trio_base_global, 
		board_addr, 
		(uint8_t*) trio_base_global + PAGE_ALIGN_UP(board_size),
		PRIV_RD | PRIV_WR | PRIV_KERN | PRIV_PRES);

    wprintf(L"s3: using %ldK of video memory at %x (= %p)\n", 
        board_size>>10, board_addr, trio_base_global);

    VmmShare(trio_base, S3_FB_NAME);

	*vtbl = &s3_vtbl;
	*cookie = NULL;
    return true;
}


bool DllMainCRTStartup(VIDEO_ADD_DEVICE *add_device)
{
	*add_device = s3AddDevice;
	return true;
}
