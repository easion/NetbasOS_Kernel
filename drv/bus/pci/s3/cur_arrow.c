/*
 *  The bits are interpreted as:
 *  A    B    MS-Windows:         X-11:
 *  0    0    Background          Screen data
 *  0    1    Foreground          Screen data
 *  1    0    Screen data         Background
 *  1    1    Inverted screen     Foreground
 */

static const uint16_t cursor_a[4 * 64] =
{
    0x0000, 0x0000, 0x0000, 0x0000, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
    0x7fff, 0xffff, 0xffff, 0xffff, 
};

static const uint16_t cursor_b[4 * 64] =
{
    0xffff, 0xffff, 0xffff, 0xffff, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
    0x8000, 0x0000, 0x0000, 0x0000, 
};