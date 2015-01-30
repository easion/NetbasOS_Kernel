

#include <drv/drv.h>
#include <drv/fs.h>
#include "../fat/fat.h"
#include <drv/buffer.h>

#include "tb.h"

/* UTF-16 management functions */
int  u16_tto32(const u16_t *s, u32_t *Ch);
int  u32_tto16(u32_t Ch, u16_t *s);

/* UTF-8 management functions */
int  u8_tto32(const u8_t *s, u32_t *Ch);
int  u32_tto8(u32_t Ch, u8_t *s);
int u8_t_stricmp (const u8_t *s1, const u8_t *s2);
int u8_t_strupr  (u8_t *Dest, const u8_t *Source);

/* Conversion functions between UTF-8 and UTF-16 */
int  u8_tto16(const u8_t *u8_t, u16_t *u16_t);
int  u16_tto8(const u16_t *u16_t, u8_t *u8_t);

/* Case conversion functions */
u32_t unicode_toupper(u32_t Ch);

#if 0

int  u8_tto32(const u8_t *s, u32_t *Ch)
{
  /* "if 0" means that it's assumed that the UTF-8 character is multi-byte */
  #if 0
  if (!(*s & 0x80))
  {
    *Ch = *s;
    return 1;
  }
  #endif
  if ((*s & 0xE0) == 0xC0)
  {
    *Ch = (*s++ & 0x8F) << 6;
    if ((*s & 0xC0) != 0x80) return FD32_Eu8_t;
    *Ch += *s++ & 0x3F;
    return 2;
  }
  if ((*s & 0xF0) == 0xE0)
  {
    *Ch = (*s++ & 0x0F) << 12;
    if ((*s & 0xC0) != 0x80) return FD32_Eu8_t;
    *Ch += (*s++ & 0x3F) << 6;
    if ((*s & 0xC0) != 0x80) return FD32_Eu8_t;
    *Ch += *s++ & 0x3F;
    return 3;
  }
  if ((*s & 0xF8) == 0xF0)
  {
    *Ch = (*s++ & 0x07) << 18;
    if ((*s & 0xC0) != 0x80) return FD32_Eu8_t;
    *Ch = (*s++ & 0x3F) << 12;
    if ((*s & 0xC0) != 0x80) return FD32_Eu8_t;
    *Ch += (*s++ & 0x3F) << 6;
    if ((*s & 0xC0) != 0x80) return FD32_Eu8_t;
    *Ch += *s++ & 0x3F;
    return 4;
  }
  return FD32_Eu8_t;
}


/* Converts a Unicode scalar value (same as UTF-32) to UTF-8.         */
/* On success, returns the number of BYTEs taken by the character.    */
/* On failure, returns FD32_Eu32_t (invalid scalar value).            */
/* See  u8_tto32 comments for conversion details.                 */
/* NOTE: For optimization reasons, it is assumed that this function   */
/* is not called when the UTF-8 character is not multi-byte. In this  */
/* case the caller should process the single-byte character directly. */
int  u32_tto8(u32_t Ch, u8_t *s)
{
  /* "if 0" means that it's assumed that the UTF-8 character is multi-byte */
  #if 0
  if (Ch < 0x000080)
  {
    *s = (u8_t) Ch;
    return 1;
  }
  #endif
  if (Ch < 0x000800)
  {
    *s++ = (u8_t) (0xC0 + ((Ch & 0x0007C0) >> 6));
    *s   = (u8_t) (0x80 +  (Ch & 0x00003F));
    return 2;
  }
  if (Ch < 0x010000)
  {
    *s++ = (u8_t) (0xE0 + ((Ch & 0x00F000) >> 12));
    *s++ = (u8_t) (0x80 + ((Ch & 0x000FC0) >> 6));
    *s   = (u8_t) (0x80 +  (Ch & 0x00003F));
    return 3;
  }
  if (Ch < 0x200000)
  {
    *s++ = (u8_t) (0xF0 + ((Ch & 0xFC0000) >> 18));
    *s++ = (u8_t) (0x80 + ((Ch & 0x03F000) >> 12));
    *s++ = (u8_t) (0x80 + ((Ch & 0x000FC0) >> 6));
    *s   = (u8_t) (0x80 +  (Ch & 0x00003F));
    return 4;
  }
  return FD32_Eu32_t;
}


/* Compares two Unicode UTF-8 strings, disregarding case.                */
/* Returns 0 if the strings match, a positive value if they don't match, */
/* or a FD32_Eu8_t if an invalid UTF-8 sequence is found.                */
int utf8_stricmp(const u8_t *s1, const u8_t *s2)
{
  u32_t Ch1, Ch2;
  int   Res;

  for (;;)
  {
    if (!(*s2 & 0x80))
    {
      if (toupper(*s1) != toupper(*s2)) return 1;
      if (*s1 == 0) return 0;
      s1++;
      s2++;
    }
    else
    {
      if ((Res =  u8_tto32(s1, &Ch1)) < 0) return FD32_Eu8_t;
      s1 += Res;
      if ((Res =  u8_tto32(s2, &Ch2)) < 0) return FD32_Eu8_t;
      s2 += Res;
      if (unicode_toupper(Ch1) != unicode_toupper(Ch2)) return 1;
    }
  }
}


/* Converts a UTF-8 string to upper case.                          */
/* Returns 0 if case has not been changed (already in upper case), */
/* a positive number if case has been changed or FD32_Eu8_t if an  */
/* invalid UTF-8 sequence is found.                                */
int utf8_strupr(u8_t *Dest, const u8_t *Source)
{
  u32_t Ch, upCh;
  int   CaseChanged = 0; /* Case not changed by default */
  int   Res;

  for (;;)
  {
    /* Try to do it faster on single-byte characters */
    if (!(*Source & 0x80))
    {
      if ((*Source >= 'a') && (*Source <= 'z'))
      {
        CaseChanged = 1;
        *Dest++ = *Source++ + 'A' - 'a';
      }
      else if ((*Dest++ = *Source++) == 0x00) return CaseChanged;
    }
    else /* Process extended characters */
    {
      if ((Res =  utf8to32(Source, &Ch)) < 0) return FD32_Eu8_t;
      Source += Res;
      upCh = unicode_toupper(Ch);
      if (upCh != Ch) CaseChanged = 1;
      Dest +=  u32_tto8(upCh, Dest);
    }
  }
}

/* Converts a UTF-16 character to Unicode scalar value (same as UTF-32).  */
/* On success, returns the number of WORDs taken by the character.        */
/* On failure, returns FD32_Eu16_t.                                       */
/*                                                                        */
/* The conversion is done according to the following rules:               */
/*                                                                        */
/*           Scalar                              UTF-16                   */
/* 00000000 zzzzyyyy yyxxxxxx <-> zzzzyyyy yyxxxxxx                       */
/* 000uuuuu zzzzyyyy yyxxxxxx <-> 110110ww wwzzzzyy  110111yy yyxxxxxx    */
/* where wwww = uuuuu - 1.                                                */
int  u16_tto32(const u16_t *s, u32_t *Ch)
{
  if ((*s & 0xFC00) != 0xD800)
  {
    *Ch = *s;
    return 1;
  }
  *Ch = ((*s++ & 0x03FF) << 10) + 0x010000;
  if ((*s & 0xFC00) != 0xDC00) return FD32_Eu16_t;
  *Ch += *s & 0x03FF;
  return 2;
}


/* Converts a Unicode scalar value (same as UTF-32) to UTF-16.     */
/* On success, returns the number of WORDs taken by the character. */
/* On failure, returns FD32_Eu32_t (invalid scalar value).         */
/* See  u16_tto32 comments for conversion details.             */
int  u32_tto16(u32_t Ch, u16_t *s)
{
  if (Ch < 0x010000)
  {
    *s = (u16_t) Ch;
    return 1;
  }
  if (Ch < 0x200000)
  {
    *s++ = (u16_t) (0xD800 + (((Ch >> 16) - 1) << 6) + ((Ch & 0x00FC00) >> 2));
    *s   = (u16_t) (0xDC00 + (Ch & 0x0003FF));
    return 2;
  }
  return FD32_Eu32_t;
}


/* Converts a UTF-8 string into UTF-16.         */
/* Returns 0 on success, FD32_Eu8_t on failure. */
int  utf8to16(const u8_t *u8_t, u16_t *u16_t)
{
  u32_t Ch;
  int   Res;

  for (;;)
  {
    /* If the UTF-8 character is not multi-byte, process it directly. */
    if (!(*u8_t & 0x80))
    {
      if ((*(u16_t++) = (u16_t) *(u8_t++)) == 0x0000) return 0;
    }
    else
    {
      if ((Res =  utf8to32(u8_t, &Ch)) < 0) return FD32_Eu8_t;
      u8_t  += Res;
      u16_t +=  u32_tto16(Ch, u16_t);
    }
  }
}


/* Converts a UTF-16 string into UTF-8.          */
/* Returns 0 on success, FD32_Eu16_t on failure. */
int  u16_tto8(const u16_t *u16_t, u8_t *u8_t)
{
  u32_t Ch;
  int   Res;

  for (;;)
  {
    /* If the UTF-16 character fits in a single-byte UTF-8 character, */
    /* process it directly.                                           */
    if (*u16_t < 0x0080)
    {
      if ((*(u8_t++) = (u8_t) *(u16_t++)) == 0x00) return 0;
    }
    else
    {
      if ((Res =  u16_tto32(u16_t, &Ch)) < 0) return FD32_Eu16_t;
      u16_t += Res;
      u8_t  +=  u32_tto8(Ch, u8_t);
    }
  }
}


/* Returns the upper case form of a Unicode scalar character */
u32_t unicode_toupper(u32_t Ch)
{
  /* from LATIN SMALL LETTER A to LATIN SMALL LETTER Z */
  if ((Ch >= 0x0061) && (Ch <= 0x007A)) return Ch - 0x0020;

  /* from CYRILLIC SMALL LETTER A to CYRILLIC SMALL LETTER YA */
  if ((Ch >= 0x0430) && (Ch <= 0x044F)) return Ch - 0x0020;

  /* from FULLWIDTH LATIN SMALL LETTER A to FULLWIDTH LATIN SMALL LETTER Z */
  if ((Ch >= 0xFF41) && (Ch <= 0xFF5A)) return Ch - 0x0020;

  /* from DESERET SMALL LETTER LONG I to DESERET SMALL LETTER ENG */
  if ((Ch >= 0x10428) && (Ch <= 0x1044D)) return Ch - 0x0028;

  /* Binary search in the UCD_toupper table */
  if ((Ch >= 0x00B5) && (Ch <= 0x24E9))
  {
    unsigned Semi, Lo = 0, Hi = (sizeof(UCD_toupper) >> 2) - 1;

    for (;;)
    {
      Semi = (Lo + Hi) >> 1;
      if (UCD_toupper[Semi].Scalar == Ch) return UCD_toupper[Semi].ToUpper;
      if (Hi - Lo == 0) break;
      if (UCD_toupper[Semi].Scalar < Ch) Lo = Semi - 1;
                                    else Hi = Semi + 1;
    }
  }

  /* If we arrive here, we don't know how to put Ch in upper case */
  return Ch;
}
#endif

