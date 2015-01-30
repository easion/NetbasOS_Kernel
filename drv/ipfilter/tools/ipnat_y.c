#ifndef lint
static const char ipnat_yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20050813

#define YYEMPTY (-1)
#define ipnat_yyclearin    (ipnat_yychar = YYEMPTY)
#define ipnat_yyerrok      (ipnat_yyerrflag = 0)
#define YYRECOVERING (ipnat_yyerrflag != 0)

extern int ipnat_yyparse(void);

static int ipnat_yygrowstack(void);
#define YYPREFIX "ipnat_yy"
#line 7 "../tools/ipnat_y.y"
#ifdef  __FreeBSD__
# ifndef __FreeBSD_cc_version
#  include <osreldate.h>
# else
#  if __FreeBSD_cc_version < 430000
#   include <osreldate.h>
#  endif
# endif
#endif
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#if !defined(__SVR4) && !defined(__GNUC__)
#include <strings.h>
#endif
#include <sys/types.h>
#include <sys/param.h>
#include <sys/file.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <sys/time.h>
#include <syslog.h>
#include <net/if.h>
#if __FreeBSD_version >= 300000
# include <net/if_var.h>
#endif
#include <netdb.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "ipf.h"
#include "netinet/ipl.h"
#include "ipnat_l.h"

#define	YYDEBUG	1

extern	void	ipnat_yyerror __P((char *));
extern	int	ipnat_yyparse __P((void));
extern	int	ipnat_yylex __P((void));
extern	int	ipnat_yydebug;
extern	FILE	*ipnat_yyin;
extern	int	ipnat_yylineNum;

static	ipnat_t		*nattop = NULL;
static	ipnat_t		*nat = NULL;
static	int		natfd = -1;
static	ioctlfunc_t	natioctlfunc = NULL;
static	addfunc_t	nataddfunc = NULL;
static	int		suggest_port = 0;

static	void	newnatrule __P((void));
static	void	setnatproto __P((int));

#line 66 "../tools/ipnat_y.y"
typedef union	{
	char	*str;
	u_32_t	num;
	struct	in_addr	ipa;
	frentry_t	fr;
	frtuc_t	*frt;
	u_short	port;
	struct	{
		u_short	p1;
		u_short	p2;
		int	pc;
	} pc;
	struct	{
		struct	in_addr	a;
		struct	in_addr	m;
	} ipp;
	union	i6addr	ip6;
} YYSTYPE;
#line 100 "ipnat_y.c"
#define YY_NUMBER 257
#define YY_HEX 258
#define YY_STR 259
#define YY_COMMENT 260
#define YY_CMP_EQ 261
#define YY_CMP_NE 262
#define YY_CMP_LE 263
#define YY_CMP_GE 264
#define YY_CMP_LT 265
#define YY_CMP_GT 266
#define YY_RANGE_OUT 267
#define YY_RANGE_IN 268
#define YY_IPV6 269
#define IPNY_MAPBLOCK 270
#define IPNY_RDR 271
#define IPNY_PORT 272
#define IPNY_PORTS 273
#define IPNY_AUTO 274
#define IPNY_RANGE 275
#define IPNY_MAP 276
#define IPNY_BIMAP 277
#define IPNY_FROM 278
#define IPNY_TO 279
#define IPNY_MASK 280
#define IPNY_PORTMAP 281
#define IPNY_ANY 282
#define IPNY_ROUNDROBIN 283
#define IPNY_FRAG 284
#define IPNY_AGE 285
#define IPNY_ICMPIDMAP 286
#define IPNY_PROXY 287
#define IPNY_TCP 288
#define IPNY_UDP 289
#define IPNY_TCPUDP 290
#define IPNY_STICKY 291
#define IPNY_MSSCLAMP 292
#define IPNY_TAG 293
#define IPNY_TLATE 294
#define IPNY_SEQUENTIAL 295
#define YYERRCODE 256
short ipnat_yylhs[] = {                                        -1,
    0,    0,    0,    0,   12,   12,   13,   16,   14,   15,
   15,   15,   18,   18,   17,   17,   17,   17,   19,   20,
   20,   20,   20,   23,   23,   23,   33,   33,   33,   33,
   10,   10,   31,   31,   31,   36,    1,    1,   30,   30,
   30,   30,   32,   32,   28,   28,   28,   21,   21,   29,
   27,   26,   26,   26,   35,   35,   35,   37,   22,   22,
   40,   41,   25,   25,   25,   43,   43,   38,   38,   44,
   39,   39,   45,    8,    8,    8,    8,    8,    8,    9,
    9,   11,   11,   24,   34,   50,   50,   46,   46,   47,
   47,   48,   48,   48,   51,   51,   49,   49,   42,   42,
   42,   42,   42,   52,   52,    5,    5,    5,    5,    2,
    6,    6,    6,    3,    3,    3,    3,    3,    3,    3,
    4,    4,    4,    7,
};
short ipnat_yylen[] = {                                         2,
    1,    1,    2,    2,    2,    1,    4,    1,    0,    2,
    2,    2,    0,    1,    7,    7,    7,    7,    7,    9,
    8,    7,    7,    0,    6,    6,    0,    1,    1,    3,
    1,    4,    1,    3,    3,    1,    1,    1,    0,    2,
    4,    4,    2,    3,    0,    2,    2,    1,    1,    1,
    1,    4,    5,    5,    4,    5,    5,    1,    1,    3,
    1,    1,    6,    4,    5,    0,    1,    1,    3,    1,
    1,    3,    1,    1,    1,    3,    3,    3,    3,    1,
    3,    2,    3,    6,    7,    0,    2,    0,    1,    0,
    1,    0,    2,    4,    0,    1,    0,    2,    0,    1,
    1,    1,    3,    2,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    7,
};
short ipnat_yydefred[] = {                                      0,
    0,    6,    0,    1,    2,    0,    8,    0,    3,    4,
   51,   50,   48,   49,    5,    0,    0,    0,    0,    0,
    0,    0,   14,   10,   11,   12,   61,    0,    0,    0,
    0,    7,    0,  111,   58,   74,    0,  113,    0,   75,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   70,    0,    0,   62,   60,    0,    0,
   36,    0,    0,    0,    0,    0,    0,    0,    0,  110,
   79,   78,    0,   77,   76,    0,   31,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   37,   38,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   73,   52,    0,    0,  115,  116,  118,
  120,  117,  119,  114,    0,    0,   69,    0,    0,    0,
    0,    0,  106,  109,    0,  108,   29,   28,    0,    0,
    0,    0,    0,    0,    0,    0,   55,    0,    0,    0,
  101,  102,    0,    0,    0,   89,   15,    0,   16,   17,
   18,   54,    0,   53,  121,  122,  123,    0,   82,   46,
   47,   19,   56,   34,   35,    0,   22,    0,    0,   41,
   42,    0,   23,    0,   43,   57,    0,   32,    0,    0,
    0,    0,    0,    0,   91,    0,   72,   83,   30,    0,
    0,   21,   44,  124,  103,   67,   64,    0,    0,    0,
    0,    0,    0,    0,   20,    0,   65,    0,    0,    0,
    0,    0,   96,    0,   63,  107,   26,   25,    0,   98,
    0,    0,    0,   94,   87,   84,    0,  105,    0,  104,
   85,
};
short ipnat_yydgoto[] = {                                       3,
  115,   71,  116,  158,  128,   37,   38,  104,   40,   78,
  117,    4,    5,    6,   15,    8,   16,   24,   17,   18,
   19,   28,   99,  147,  100,   41,   20,  119,   21,   63,
   86,  133,  129,  167,   47,  135,   42,   55,  105,   29,
   58,  143,  197,   56,  106,  148,  186,  203,  212,  222,
  214,  229,
};
short ipnat_yysindex[] = {                                   -143,
   -9,    0, -143,    0,    0, -129,    0, -210,    0,    0,
    0,    0,    0,    0,    0,   69,   69,   69, -116, -116,
 -116,   91,    0,    0,    0,    0,    0, -196,  125, -186,
  -33,    0,  124,    0,    0,    0,  -45,    0, -123,    0,
 -122, -186,  -86, -119, -102, -259, -115,  -29,  -79, -137,
  -98, -228, -228,    0,  -27,  -92,    0,    0, -186, -186,
    0, -148, -112, -124, -148, -186,  -96,  135,  124,    0,
    0,    0,  124,    0,    0,  -73,    0, -192, -192,  -25,
  -94,  -43,  -87,  -91,   -4, -231, -148,    0,    0,  -26,
 -221,  -89, -186,  -65,  142, -150,  -66,  -92,  -88,  -88,
  -88,  -88, -186,    0,    0,  -92, -186,    0,    0,    0,
    0,    0,    0,    0,  -55, -124,    0, -232,  -88, -186,
  -63, -148,    0,    0,  149,    0,    0,    0,  -88,  -92,
 -124, -124, -231,  -88,  -54, -186,    0,  151,  -73,  153,
    0,    0, -169,  -59, -106,    0,    0,  -85,    0,    0,
    0,    0,  -43,    0,    0,    0,    0, -124,    0,    0,
    0,    0,    0,    0,    0,  -83,    0,  -85, -231,    0,
    0,  -88,    0, -124,    0,    0,  -56,    0,  -82,  -93,
  146,  150,  -50,  -49,    0,  -60,    0,    0,    0,  -60,
  -88,    0,    0,    0,    0,    0,    0, -124,  -40,  180,
  186,  -21,  -53,  -51,    0,  -93,    0, -181, -181,  190,
  -16,  -46,    0,  -53,    0,    0,    0,    0,  -15,    0,
  -13, -231,  -44,    0,    0,    0, -172,    0,  -46,    0,
    0,
};
short ipnat_yyrindex[] = {                                   -113,
    0,    0, -113,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  154,  154,  154,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  -28,    0,
    0,    0,    1,    0,    0,    0,   39,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,  -24,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   77,    0,    0,    0,    0,  152,  152,    0,
    0,    0,  189,    0,  115,  410,    0,    0,    0,  -38,
  410,    0,    0,    0,    0, -145,    0,    0,  276,  276,
  276,  276,    0,    0,    0,  -32,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  276,    0,
    0,    0,    0,    0,  338,    0,    0,    0,  447,    0,
    0,    0,  410,  447,    0,    0,    0,    0,    0, -125,
    0,    0,    0,    0,    0,    0,    0,  379,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,  511,  410,    0,
    0,  447,    0,    0,    0,    0,    0,    0,    0,  238,
    0,    0,    0,    0,    0,  484,    0,    0,    0,  492,
  447,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  540,  567,    0,  238,    0,    0,    0,  314,
    0,  616,    0,  575,    0,    0,    0,    0,    0,    0,
    0,  156,  317,    0,    0,    0,    0,    0,  118,    0,
    0,
};
short ipnat_yygindex[] = {                                      0,
  -52,  193,    0,    0,  -47,  -41,  -35,   71,    0,  198,
  106,  260,  261,    0,    0,    0,    0,  148,    0,    0,
    0,  147,  -68,   25,  187,    0,    0,    0,    0,    0,
  -12,  137,  -77, -117,    0,  -36,   -1,   37,  -70,    0,
    0,    0,   59,    0,    0, -107,  100,   79,   56,   46,
    0,    0,
};
#define YYTABLESIZE 906
short ipnat_yytable[] = {                                      45,
  112,   51,  157,   66,   59,   81,  174,  103,   68,   64,
  101,   90,   61,  134,   72,   75,  173,  114,  131,   82,
   85,  168,  137,   85,  160,  123,  168,  124,   33,   48,
   34,  132,  152,  112,   62,  123,  154,  124,   80,  122,
   95,  161,  121,   60,  112,   85,   76,  112,   22,  163,
   61,    7,   91,   36,  192,  172,  125,  126,  127,  112,
   33,  145,   34,  159,  168,  176,  125,  126,  127,  153,
   33,   80,   34,  205,  130,  123,   81,  124,  170,  171,
  165,   35,  175,  168,   67,   36,  230,   88,   96,   89,
  181,  191,  184,   97,   98,   36,   84,   80,   39,   61,
   44,   46,   92,  178,  180,  188,  216,  126,   33,   81,
   34,   99,   54,   99,   33,    1,    2,   86,   54,   69,
   70,  193,   77,   77,  149,  150,  151,   23,   99,   83,
   54,  100,   88,  100,   89,   81,   54,  140,  141,  142,
   11,   12,   27,  162,  226,  206,   13,   14,  100,   32,
   88,   24,  183,   13,  228,   27,    9,    9,   73,   70,
  217,  218,    9,    9,   25,   26,   30,   31,   43,   49,
   52,   53,   57,   33,   59,   35,   86,   68,   65,   61,
   94,   87,   93,   69,  107,  118,  139,  120,   45,  136,
  145,  138,  144,  164,  146,  166,  177,  182,  185,  179,
  194,  196,   88,  198,   89,  189,  195,  199,  200,  201,
   24,  155,  156,   88,   27,   89,  207,  108,  109,  110,
  111,  112,  113,   33,  202,   34,  208,   33,   59,   34,
   59,   33,  209,   34,   50,  210,  219,   66,  211,  213,
  220,  224,  227,   74,   35,  225,  221,   45,   36,   59,
   79,   80,   36,   59,   68,   40,   36,  112,  187,  112,
  112,   71,    9,   10,  215,  102,  169,  190,  204,  223,
  112,  112,  112,  112,  231,   88,  112,  112,    0,  112,
  112,  112,    0,  112,  112,  112,  112,  112,  112,  112,
  112,  112,  112,  112,  112,   80,   66,   80,   80,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   80,   80,
   80,   80,    0,   93,   80,   80,   24,   80,    0,   80,
    0,   80,   80,   80,   80,   80,   80,   80,   80,    0,
   80,   80,   80,   81,   88,   81,   81,  107,    0,    0,
    0,    0,    0,    0,    0,    0,   81,   81,   81,   81,
    0,    0,   81,   81,    0,   81,    0,   81,    0,   81,
   81,   81,   81,   81,   81,   81,   81,    0,   81,   81,
   81,   33,   93,   33,   33,   24,   86,   86,   90,    0,
    0,    0,    0,    0,   33,   33,   33,   86,   86,    0,
   33,   33,    0,   86,   86,    0,  107,   33,   33,   33,
    0,   33,   33,   33,   33,   33,   33,   33,   24,   27,
   24,   24,   13,   13,    0,   27,    0,    0,    0,    0,
    0,   24,   24,   13,   13,   27,   27,   24,   24,   13,
   13,   27,   27,    0,   24,   24,   24,   90,    0,   24,
   24,   24,    0,   24,   24,   45,   88,   45,   45,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   45,   45,
    0,    0,    0,    0,   45,   45,    0,    0,   27,    0,
    0,   45,   45,   45,    0,    0,   45,   45,   45,    0,
   45,   45,    0,   92,    0,    0,    0,    0,    0,    0,
    0,   92,    0,    0,   66,    0,   66,   66,    0,    0,
    0,    0,    0,    0,    0,   88,    0,   66,   66,    0,
   90,    0,    0,   66,   66,    0,    0,    0,    0,    0,
   66,   66,   66,    0,    0,   66,   66,   66,    0,   66,
   66,    0,   88,    0,   88,   88,    0,    0,    0,   97,
    0,    0,   92,    0,    0,   88,   88,    0,    0,    0,
   92,   88,   88,    0,    0,    0,    0,    0,    0,   88,
   88,    0,    0,   88,   88,   88,   95,   88,   88,   90,
   93,    0,   93,   93,   97,   24,   24,    0,    0,    0,
    0,    0,    0,   93,   93,    0,   24,   24,    0,   93,
   93,    0,   24,   24,    0,    0,  107,  107,   97,    0,
   93,   93,   93,   93,   93,   93,   93,  107,  107,   24,
    0,    0,    0,  107,  107,   86,    0,    0,    0,    0,
  107,  107,  107,    0,  107,   95,    0,    0,  107,  107,
  107,    0,    0,   97,    0,   90,    0,   90,   90,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   90,   90,
    0,    0,    0,    0,   90,   90,    0,    0,    0,    0,
    0,    0,    0,   90,    0,    0,   90,   90,   90,   27,
   90,   90,    0,    0,   86,    0,    0,    0,    0,   27,
   27,    0,    0,    0,    0,   27,   27,    0,    0,    0,
    0,    0,   27,   27,   27,    0,   27,    0,    0,    0,
   27,   27,   27,    0,    0,   88,   88,    0,    0,    0,
    0,    0,    0,    0,    0,    0,   88,   88,    0,    0,
    0,    0,   88,   88,    0,    0,    0,    0,    0,    0,
   88,   88,    0,   88,    0,    0,    0,   88,   88,   88,
   92,    0,   92,   92,    0,    0,    0,    0,    0,    0,
   92,   92,    0,   92,   92,    0,    0,    0,    0,   92,
   92,   92,   92,    0,    0,    0,    0,   92,   92,   90,
   90,   92,   92,   92,    0,   92,   92,    0,   92,    0,
   90,   90,   92,   92,   92,    0,   90,   90,    0,    0,
    0,    0,    0,    0,    0,   90,   97,   90,   97,   97,
    0,   90,   90,   90,    0,    0,    0,    0,    0,   97,
   97,    0,    0,    0,    0,   97,   97,    0,    0,    0,
    0,    0,    0,    0,    0,   95,   95,   97,   97,   97,
    0,    0,   97,   97,   97,    0,   95,   95,    0,    0,
    0,    0,   95,   95,   97,   97,    0,    0,    0,    0,
   97,   97,    0,   95,    0,    0,    0,    0,   95,   95,
    0,   97,    0,    0,    0,    0,    0,   97,    0,    0,
    0,    0,   86,    0,   86,   86,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   86,   86,    0,    0,    0,
    0,   86,   86,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   86,   86,   86,
};
short ipnat_yycheck[] = {                                      33,
    0,   47,   58,   33,   33,   33,   61,   33,   33,   46,
   79,   64,  272,   91,   50,   51,  134,   61,   45,   56,
   62,  129,   93,   65,  257,  257,  134,  259,  257,   31,
  259,   58,  103,   33,  294,  257,  107,  259,    0,   44,
   76,  274,   47,   45,   44,   87,  275,   47,  259,  120,
  272,   61,   65,  282,  172,  133,  288,  289,  290,   59,
  257,   98,  259,  116,  172,  136,  288,  289,  290,  106,
  257,   33,  259,  191,   87,  257,    0,  259,  131,  132,
  122,  278,  135,  191,   48,  282,  259,  257,  281,  259,
  143,  169,  145,  286,  287,  282,   60,   59,   28,  272,
   30,   31,   66,  139,  274,  158,  288,  289,  257,   33,
  259,  257,   42,  259,    0,  259,  260,    0,   48,  257,
  258,  174,   52,   53,  100,  101,  102,   59,  274,   59,
   60,  257,  257,  259,  259,   59,   66,  288,  289,  290,
  270,  271,  259,  119,  222,  198,  276,  277,  274,   59,
  257,    0,  259,    0,  223,    0,  270,  271,  257,  258,
  208,  209,  276,  277,   17,   18,   20,   21,   44,   46,
  294,  294,  259,   59,  294,  278,   59,  257,  294,  272,
   46,  294,  279,  257,  279,  273,   45,  279,    0,  279,
  227,  257,  259,  257,  283,   47,   46,  257,  284,   47,
  257,  295,  257,   58,  259,  289,  289,   58,  259,  259,
   59,  267,  268,  257,   59,  259,  257,  261,  262,  263,
  264,  265,  266,  257,  285,  259,   47,  257,  257,  259,
  259,  257,   47,  259,  280,  257,   47,    0,  292,  291,
  257,  257,  287,   51,  278,  259,  293,   59,  282,  278,
   53,  279,  282,  282,  279,  294,  282,  257,  153,  259,
  260,  294,    3,    3,  206,   79,  130,  168,  190,  214,
  270,  271,  272,  273,  229,    0,  276,  277,   -1,  279,
  280,  281,   -1,  283,  284,  285,  286,  287,  288,  289,
  290,  291,  292,  293,  294,  257,   59,  259,  260,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  270,  271,
  272,  273,   -1,    0,  276,  277,    0,  279,   -1,  281,
   -1,  283,  284,  285,  286,  287,  288,  289,  290,   -1,
  292,  293,  294,  257,   59,  259,  260,    0,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  270,  271,  272,  273,
   -1,   -1,  276,  277,   -1,  279,   -1,  281,   -1,  283,
  284,  285,  286,  287,  288,  289,  290,   -1,  292,  293,
  294,  257,   59,  259,  260,   59,  259,  260,    0,   -1,
   -1,   -1,   -1,   -1,  270,  271,  272,  270,  271,   -1,
  276,  277,   -1,  276,  277,   -1,   59,  283,  284,  285,
   -1,  287,  288,  289,  290,  291,  292,  293,  257,    0,
  259,  260,  259,  260,   -1,  260,   -1,   -1,   -1,   -1,
   -1,  270,  271,  270,  271,  270,  271,  276,  277,  276,
  277,  276,  277,   -1,  283,  284,  285,   59,   -1,  288,
  289,  290,   -1,  292,  293,  257,    0,  259,  260,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  270,  271,
   -1,   -1,   -1,   -1,  276,  277,   -1,   -1,   59,   -1,
   -1,  283,  284,  285,   -1,   -1,  288,  289,  290,   -1,
  292,  293,   -1,    0,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,    0,   -1,   -1,  257,   -1,  259,  260,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   59,   -1,  270,  271,   -1,
    0,   -1,   -1,  276,  277,   -1,   -1,   -1,   -1,   -1,
  283,  284,  285,   -1,   -1,  288,  289,  290,   -1,  292,
  293,   -1,  257,   -1,  259,  260,   -1,   -1,   -1,    0,
   -1,   -1,   59,   -1,   -1,  270,  271,   -1,   -1,   -1,
   59,  276,  277,   -1,   -1,   -1,   -1,   -1,   -1,  284,
  285,   -1,   -1,  288,  289,  290,    0,  292,  293,   59,
  257,   -1,  259,  260,    0,  259,  260,   -1,   -1,   -1,
   -1,   -1,   -1,  270,  271,   -1,  270,  271,   -1,  276,
  277,   -1,  276,  277,   -1,   -1,  259,  260,   59,   -1,
  287,  288,  289,  290,  291,  292,  293,  270,  271,  293,
   -1,   -1,   -1,  276,  277,    0,   -1,   -1,   -1,   -1,
  283,  284,  285,   -1,  287,   59,   -1,   -1,  291,  292,
  293,   -1,   -1,   59,   -1,  257,   -1,  259,  260,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  270,  271,
   -1,   -1,   -1,   -1,  276,  277,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  285,   -1,   -1,  288,  289,  290,  260,
  292,  293,   -1,   -1,   59,   -1,   -1,   -1,   -1,  270,
  271,   -1,   -1,   -1,   -1,  276,  277,   -1,   -1,   -1,
   -1,   -1,  283,  284,  285,   -1,  287,   -1,   -1,   -1,
  291,  292,  293,   -1,   -1,  259,  260,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,  270,  271,   -1,   -1,
   -1,   -1,  276,  277,   -1,   -1,   -1,   -1,   -1,   -1,
  284,  285,   -1,  287,   -1,   -1,   -1,  291,  292,  293,
  257,   -1,  259,  260,   -1,   -1,   -1,   -1,   -1,   -1,
  259,  260,   -1,  270,  271,   -1,   -1,   -1,   -1,  276,
  277,  270,  271,   -1,   -1,   -1,   -1,  276,  277,  259,
  260,  288,  289,  290,   -1,  292,  293,   -1,  287,   -1,
  270,  271,  291,  292,  293,   -1,  276,  277,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  285,  257,  287,  259,  260,
   -1,  291,  292,  293,   -1,   -1,   -1,   -1,   -1,  270,
  271,   -1,   -1,   -1,   -1,  276,  277,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  259,  260,  288,  289,  290,
   -1,   -1,  293,  259,  260,   -1,  270,  271,   -1,   -1,
   -1,   -1,  276,  277,  270,  271,   -1,   -1,   -1,   -1,
  276,  277,   -1,  287,   -1,   -1,   -1,   -1,  292,  293,
   -1,  287,   -1,   -1,   -1,   -1,   -1,  293,   -1,   -1,
   -1,   -1,  257,   -1,  259,  260,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,  270,  271,   -1,   -1,   -1,
   -1,  276,  277,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  288,  289,  290,
};
#define YYFINAL 3
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 295
#if YYDEBUG
char *ipnat_yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,0,0,0,0,0,0,"','","'-'","'.'","'/'",0,0,0,0,0,0,0,0,0,0,"':'",
"';'",0,"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"YY_NUMBER","YY_HEX","YY_STR","YY_COMMENT","YY_CMP_EQ","YY_CMP_NE",
"YY_CMP_LE","YY_CMP_GE","YY_CMP_LT","YY_CMP_GT","YY_RANGE_OUT","YY_RANGE_IN",
"YY_IPV6","IPNY_MAPBLOCK","IPNY_RDR","IPNY_PORT","IPNY_PORTS","IPNY_AUTO",
"IPNY_RANGE","IPNY_MAP","IPNY_BIMAP","IPNY_FROM","IPNY_TO","IPNY_MASK",
"IPNY_PORTMAP","IPNY_ANY","IPNY_ROUNDROBIN","IPNY_FRAG","IPNY_AGE",
"IPNY_ICMPIDMAP","IPNY_PROXY","IPNY_TCP","IPNY_UDP","IPNY_TCPUDP","IPNY_STICKY",
"IPNY_MSSCLAMP","IPNY_TAG","IPNY_TLATE","IPNY_SEQUENTIAL",
};
char *ipnat_yyrule[] = {
"$accept : file",
"file : line",
"file : assign",
"file : file line",
"file : file assign",
"line : xx rule",
"line : YY_COMMENT",
"assign : YY_STR assigning YY_STR ';'",
"assigning : '='",
"xx :",
"rule : map eol",
"rule : mapblock eol",
"rule : redir eol",
"eol :",
"eol : ';'",
"map : mapit ifnames addr IPNY_TLATE rhaddr proxy mapoptions",
"map : mapit ifnames addr IPNY_TLATE rhaddr mapport mapoptions",
"map : mapit ifnames mapfrom IPNY_TLATE rhaddr proxy mapoptions",
"map : mapit ifnames mapfrom IPNY_TLATE rhaddr mapport mapoptions",
"mapblock : mapblockit ifnames addr IPNY_TLATE addr ports mapoptions",
"redir : rdrit ifnames addr dport IPNY_TLATE dip nport setproto rdroptions",
"redir : rdrit ifnames rdrfrom IPNY_TLATE dip nport setproto rdroptions",
"redir : rdrit ifnames addr IPNY_TLATE dip setproto rdroptions",
"redir : rdrit ifnames rdrfrom IPNY_TLATE dip setproto rdroptions",
"proxy :",
"proxy : IPNY_PROXY port portspec YY_STR '/' proto",
"proxy : IPNY_PROXY port YY_STR YY_STR '/' proto",
"setproto :",
"setproto : proto",
"setproto : IPNY_TCPUDP",
"setproto : IPNY_TCP '/' IPNY_UDP",
"rhaddr : addr",
"rhaddr : IPNY_RANGE ipv4 '-' ipv4",
"dip : hostname",
"dip : hostname '/' YY_NUMBER",
"dip : hostname ',' hostname",
"port : IPNY_PORT",
"portspec : YY_NUMBER",
"portspec : YY_STR",
"dport :",
"dport : port portspec",
"dport : port portspec '-' portspec",
"dport : port portspec ':' portspec",
"nport : port portspec",
"nport : port '=' portspec",
"ports :",
"ports : IPNY_PORTS YY_NUMBER",
"ports : IPNY_PORTS IPNY_AUTO",
"mapit : IPNY_MAP",
"mapit : IPNY_BIMAP",
"rdrit : IPNY_RDR",
"mapblockit : IPNY_MAPBLOCK",
"mapfrom : from sobject IPNY_TO dobject",
"mapfrom : from sobject '!' IPNY_TO dobject",
"mapfrom : from sobject IPNY_TO '!' dobject",
"rdrfrom : from sobject IPNY_TO dobject",
"rdrfrom : '!' from sobject IPNY_TO dobject",
"rdrfrom : from '!' sobject IPNY_TO dobject",
"from : IPNY_FROM",
"ifnames : ifname",
"ifnames : ifname ',' otherifname",
"ifname : YY_STR",
"otherifname : YY_STR",
"mapport : IPNY_PORTMAP tcpudp portspec ':' portspec randport",
"mapport : IPNY_PORTMAP tcpudp IPNY_AUTO randport",
"mapport : IPNY_ICMPIDMAP YY_STR YY_NUMBER ':' YY_NUMBER",
"randport :",
"randport : IPNY_SEQUENTIAL",
"sobject : saddr",
"sobject : saddr port portstuff",
"saddr : addr",
"dobject : daddr",
"dobject : daddr port portstuff",
"daddr : addr",
"addr : IPNY_ANY",
"addr : nummask",
"addr : hostname '/' ipv4",
"addr : hostname '/' hexnumber",
"addr : hostname IPNY_MASK ipv4",
"addr : hostname IPNY_MASK hexnumber",
"nummask : hostname",
"nummask : hostname '/' YY_NUMBER",
"portstuff : compare portspec",
"portstuff : portspec range portspec",
"mapoptions : rr frag age mssclamp nattag setproto",
"rdroptions : rr frag age sticky mssclamp rdrproxy nattag",
"nattag :",
"nattag : IPNY_TAG YY_STR",
"rr :",
"rr : IPNY_ROUNDROBIN",
"frag :",
"frag : IPNY_FRAG",
"age :",
"age : IPNY_AGE YY_NUMBER",
"age : IPNY_AGE YY_NUMBER '/' YY_NUMBER",
"sticky :",
"sticky : IPNY_STICKY",
"mssclamp :",
"mssclamp : IPNY_MSSCLAMP YY_NUMBER",
"tcpudp :",
"tcpudp : IPNY_TCP",
"tcpudp : IPNY_UDP",
"tcpudp : IPNY_TCPUDP",
"tcpudp : IPNY_TCP '/' IPNY_UDP",
"rdrproxy : IPNY_PROXY YY_STR",
"rdrproxy : proxy",
"proto : YY_NUMBER",
"proto : IPNY_TCP",
"proto : IPNY_UDP",
"proto : YY_STR",
"hexnumber : YY_HEX",
"hostname : YY_STR",
"hostname : YY_NUMBER",
"hostname : ipv4",
"compare : '='",
"compare : YY_CMP_EQ",
"compare : YY_CMP_NE",
"compare : YY_CMP_LT",
"compare : YY_CMP_LE",
"compare : YY_CMP_GT",
"compare : YY_CMP_GE",
"range : YY_RANGE_OUT",
"range : YY_RANGE_IN",
"range : ':'",
"ipv4 : YY_NUMBER '.' YY_NUMBER '.' YY_NUMBER '.' YY_NUMBER",
};
#endif
#if YYDEBUG
#include <stdio.h>
#endif

/* define the initial stack-sizes */
#ifdef YYSTACKSIZE
#undef YYMAXDEPTH
#define YYMAXDEPTH  YYSTACKSIZE
#else
#ifdef YYMAXDEPTH
#define YYSTACKSIZE YYMAXDEPTH
#else
#define YYSTACKSIZE 500
#define YYMAXDEPTH  500
#endif
#endif

#define YYINITSTACKSIZE 500

int      ipnat_yydebug;
int      ipnat_yynerrs;
int      ipnat_yyerrflag;
int      ipnat_yychar;
short   *ipnat_yyssp;
YYSTYPE *ipnat_yyvsp;
YYSTYPE  ipnat_yyval;
YYSTYPE  ipnat_yylval;

/* variables for the parser stack */
static short   *ipnat_yyss;
static short   *ipnat_yysslim;
static YYSTYPE *ipnat_yyvs;
static int      ipnat_yystacksize;
#line 631 "../tools/ipnat_y.y"


static	wordtab_t	ipnat_yywords[] = {
	{ "age",	IPNY_AGE },
	{ "any",	IPNY_ANY },
	{ "auto",	IPNY_AUTO },
	{ "bimap",	IPNY_BIMAP },
	{ "frag",	IPNY_FRAG },
	{ "from",	IPNY_FROM },
	{ "icmpidmap",	IPNY_ICMPIDMAP },
	{ "mask",	IPNY_MASK },
	{ "map",	IPNY_MAP },
	{ "map-block",	IPNY_MAPBLOCK },
	{ "mssclamp",	IPNY_MSSCLAMP },
	{ "netmask",	IPNY_MASK },
	{ "port",	IPNY_PORT },
	{ "portmap",	IPNY_PORTMAP },
	{ "ports",	IPNY_PORTS },
	{ "proxy",	IPNY_PROXY },
	{ "range",	IPNY_RANGE },
	{ "rdr",	IPNY_RDR },
	{ "round-robin",IPNY_ROUNDROBIN },
	{ "sequential",	IPNY_SEQUENTIAL },
	{ "sticky",	IPNY_STICKY },
	{ "tag",	IPNY_TAG },
	{ "tcp",	IPNY_TCP },
	{ "tcpudp",	IPNY_TCPUDP },
	{ "to",		IPNY_TO },
	{ "udp",	IPNY_UDP },
	{ "-",		'-' },
	{ "->",		IPNY_TLATE },
	{ "eq",		YY_CMP_EQ },
	{ "ne",		YY_CMP_NE },
	{ "lt",		YY_CMP_LT },
	{ "gt",		YY_CMP_GT },
	{ "le",		YY_CMP_LE },
	{ "ge",		YY_CMP_GE },
	{ NULL,		0 }
};


int ipnat_parsefile(fd, addfunc, ioctlfunc, filename)
int fd;
addfunc_t addfunc;
ioctlfunc_t ioctlfunc;
char *filename;
{
	FILE *fp = NULL;
	char *s;

	(void) ipnat_yysettab(ipnat_yywords);

	s = getenv("YYDEBUG");
	if (s)
		ipnat_yydebug = atoi(s);
	else
		ipnat_yydebug = 0;

	if (strcmp(filename, "-")) {
		fp = fopen(filename, "r");
		if (!fp) {
			fprintf(stderr, "fopen(%s) failed: %s\n", filename,
				STRERROR(errno));
			return -1;
		}
	} else
		fp = stdin;

	while (ipnat_parsesome(fd, addfunc, ioctlfunc, fp) == 1)
		;
	if (fp != NULL)
		fclose(fp);
	return 0;
}


int ipnat_parsesome(fd, addfunc, ioctlfunc, fp)
int fd;
addfunc_t addfunc;
ioctlfunc_t ioctlfunc;
FILE *fp;
{
	char *s;
	int i;

	ipnat_yylineNum = 1;

	natfd = fd;
	nataddfunc = addfunc;
	natioctlfunc = ioctlfunc;

	if (feof(fp))
		return 0;
	i = fgetc(fp);
	if (i == EOF)
		return 0;
	if (ungetc(i, fp) == EOF)
		return 0;
	if (feof(fp))
		return 0;
	s = getenv("YYDEBUG");
	if (s)
		ipnat_yydebug = atoi(s);
	else
		ipnat_yydebug = 0;

	ipnat_yyin = fp;
	ipnat_yyparse();
	return 1;
}


static void newnatrule()
{
	ipnat_t *n;

	n = calloc(1, sizeof(*n));
	if (n == NULL)
		return;

	if (nat == NULL)
		nattop = nat = n;
	else {
		nat->in_next = n;
		nat = n;
	}

	suggest_port = 0;
}


static void setnatproto(p)
int p;
{
	nat->in_p = p;

	switch (p)
	{
	case IPPROTO_TCP :
		nat->in_flags |= IPN_TCP;
		nat->in_flags &= ~IPN_UDP;
		break;
	case IPPROTO_UDP :
		nat->in_flags |= IPN_UDP;
		nat->in_flags &= ~IPN_TCP;
		break;
	case IPPROTO_ICMP :
		nat->in_flags &= ~IPN_TCPUDP;
		if (!(nat->in_flags & IPN_ICMPQUERY)) {
			nat->in_dcmp = 0;
			nat->in_scmp = 0;
			nat->in_pmin = 0;
			nat->in_pmax = 0;
			nat->in_pnext = 0;
		}
		break;
	default :
		if ((nat->in_redir & NAT_MAPBLK) == 0) {
			nat->in_flags &= ~IPN_TCPUDP;
			nat->in_dcmp = 0;
			nat->in_scmp = 0;
			nat->in_pmin = 0;
			nat->in_pmax = 0;
			nat->in_pnext = 0;
		}
		break;
	}

	if ((nat->in_flags & (IPN_TCPUDP|IPN_FIXEDDPORT)) == IPN_FIXEDDPORT)
		nat->in_flags &= ~IPN_FIXEDDPORT;
}


void ipnat_addrule(fd, ioctlfunc, ptr)
int fd;
ioctlfunc_t ioctlfunc;
void *ptr;
{
	ioctlcmd_t add, del;
	ipfobj_t obj;
	ipnat_t *ipn;

	ipn = ptr;
	bzero((char *)&obj, sizeof(obj));
	obj.ipfo_rev = IPFILTER_VERSION;
	obj.ipfo_size = sizeof(ipnat_t);
	obj.ipfo_type = IPFOBJ_IPNAT;
	obj.ipfo_ptr = ptr;
	add = 0;
	del = 0;

	if ((opts & OPT_DONOTHING) != 0)
		fd = -1;

	if (opts & OPT_ZERORULEST) {
		add = SIOCZRLST;
	} else if (opts & OPT_INACTIVE) {
		add = SIOCADNAT;
		del = SIOCRMNAT;
	} else {
		add = SIOCADNAT;
		del = SIOCRMNAT;
	}

	if ((opts & OPT_VERBOSE) != 0)
		printnat(ipn, opts);

	if (opts & OPT_DEBUG)
		binprint(ipn, sizeof(*ipn));

	if ((opts & OPT_ZERORULEST) != 0) {
		if ((*ioctlfunc)(fd, add, (void *)&obj) == -1) {
			if ((opts & OPT_DONOTHING) == 0) {
				fprintf(stderr, "%d:", ipnat_yylineNum);
				perror("ioctl(SIOCZRLST)");
			}
		} else {
#ifdef	USE_QUAD_T
/*
			printf("hits %qd bytes %qd ",
				(long long)fr->fr_hits,
				(long long)fr->fr_bytes);
*/
#else
/*
			printf("hits %ld bytes %ld ",
				fr->fr_hits, fr->fr_bytes);
*/
#endif
			printnat(ipn, opts);
		}
	} else if ((opts & OPT_REMOVE) != 0) {
		if ((*ioctlfunc)(fd, del, (void *)&obj) == -1) {
			if ((opts & OPT_DONOTHING) == 0) {
				fprintf(stderr, "%d:", ipnat_yylineNum);
				perror("ioctl(delete nat rule)");
			}
		}
	} else {
		if ((*ioctlfunc)(fd, add, (void *)&obj) == -1) {
			if ((opts & OPT_DONOTHING) == 0) {
				fprintf(stderr, "%d:", ipnat_yylineNum);
				perror("ioctl(add/insert nat rule)");
			}
		}
	}
}
#line 883 "ipnat_y.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int ipnat_yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = ipnat_yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = ipnat_yyssp - ipnat_yyss;
    newss = (ipnat_yyss != 0)
          ? (short *)realloc(ipnat_yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    ipnat_yyss  = newss;
    ipnat_yyssp = newss + i;
    newvs = (ipnat_yyvs != 0)
          ? (YYSTYPE *)realloc(ipnat_yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    ipnat_yyvs = newvs;
    ipnat_yyvsp = newvs + i;
    ipnat_yystacksize = newsize;
    ipnat_yysslim = ipnat_yyss + newsize - 1;
    return 0;
}

#define YYABORT goto ipnat_yyabort
#define YYREJECT goto ipnat_yyabort
#define YYACCEPT goto ipnat_yyaccept
#define YYERROR goto ipnat_yyerrlab
int
ipnat_yyparse(void)
{
    register int ipnat_yym, ipnat_yyn, ipnat_yystate;
#if YYDEBUG
    register const char *ipnat_yys;

    if ((ipnat_yys = getenv("YYDEBUG")) != 0)
    {
        ipnat_yyn = *ipnat_yys;
        if (ipnat_yyn >= '0' && ipnat_yyn <= '9')
            ipnat_yydebug = ipnat_yyn - '0';
    }
#endif

    ipnat_yynerrs = 0;
    ipnat_yyerrflag = 0;
    ipnat_yychar = YYEMPTY;

    if (ipnat_yyss == NULL && ipnat_yygrowstack()) goto ipnat_yyoverflow;
    ipnat_yyssp = ipnat_yyss;
    ipnat_yyvsp = ipnat_yyvs;
    *ipnat_yyssp = ipnat_yystate = 0;

ipnat_yyloop:
    if ((ipnat_yyn = ipnat_yydefred[ipnat_yystate]) != 0) goto ipnat_yyreduce;
    if (ipnat_yychar < 0)
    {
        if ((ipnat_yychar = ipnat_yylex()) < 0) ipnat_yychar = 0;
#if YYDEBUG
        if (ipnat_yydebug)
        {
            ipnat_yys = 0;
            if (ipnat_yychar <= YYMAXTOKEN) ipnat_yys = ipnat_yyname[ipnat_yychar];
            if (!ipnat_yys) ipnat_yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, ipnat_yystate, ipnat_yychar, ipnat_yys);
        }
#endif
    }
    if ((ipnat_yyn = ipnat_yysindex[ipnat_yystate]) && (ipnat_yyn += ipnat_yychar) >= 0 &&
            ipnat_yyn <= YYTABLESIZE && ipnat_yycheck[ipnat_yyn] == ipnat_yychar)
    {
#if YYDEBUG
        if (ipnat_yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, ipnat_yystate, ipnat_yytable[ipnat_yyn]);
#endif
        if (ipnat_yyssp >= ipnat_yysslim && ipnat_yygrowstack())
        {
            goto ipnat_yyoverflow;
        }
        *++ipnat_yyssp = ipnat_yystate = ipnat_yytable[ipnat_yyn];
        *++ipnat_yyvsp = ipnat_yylval;
        ipnat_yychar = YYEMPTY;
        if (ipnat_yyerrflag > 0)  --ipnat_yyerrflag;
        goto ipnat_yyloop;
    }
    if ((ipnat_yyn = ipnat_yyrindex[ipnat_yystate]) && (ipnat_yyn += ipnat_yychar) >= 0 &&
            ipnat_yyn <= YYTABLESIZE && ipnat_yycheck[ipnat_yyn] == ipnat_yychar)
    {
        ipnat_yyn = ipnat_yytable[ipnat_yyn];
        goto ipnat_yyreduce;
    }
    if (ipnat_yyerrflag) goto ipnat_yyinrecovery;

    ipnat_yyerror("syntax error");

#ifdef lint
    goto ipnat_yyerrlab;
#endif

ipnat_yyerrlab:
    ++ipnat_yynerrs;

ipnat_yyinrecovery:
    if (ipnat_yyerrflag < 3)
    {
        ipnat_yyerrflag = 3;
        for (;;)
        {
            if ((ipnat_yyn = ipnat_yysindex[*ipnat_yyssp]) && (ipnat_yyn += YYERRCODE) >= 0 &&
                    ipnat_yyn <= YYTABLESIZE && ipnat_yycheck[ipnat_yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (ipnat_yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *ipnat_yyssp, ipnat_yytable[ipnat_yyn]);
#endif
                if (ipnat_yyssp >= ipnat_yysslim && ipnat_yygrowstack())
                {
                    goto ipnat_yyoverflow;
                }
                *++ipnat_yyssp = ipnat_yystate = ipnat_yytable[ipnat_yyn];
                *++ipnat_yyvsp = ipnat_yylval;
                goto ipnat_yyloop;
            }
            else
            {
#if YYDEBUG
                if (ipnat_yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *ipnat_yyssp);
#endif
                if (ipnat_yyssp <= ipnat_yyss) goto ipnat_yyabort;
                --ipnat_yyssp;
                --ipnat_yyvsp;
            }
        }
    }
    else
    {
        if (ipnat_yychar == 0) goto ipnat_yyabort;
#if YYDEBUG
        if (ipnat_yydebug)
        {
            ipnat_yys = 0;
            if (ipnat_yychar <= YYMAXTOKEN) ipnat_yys = ipnat_yyname[ipnat_yychar];
            if (!ipnat_yys) ipnat_yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, ipnat_yystate, ipnat_yychar, ipnat_yys);
        }
#endif
        ipnat_yychar = YYEMPTY;
        goto ipnat_yyloop;
    }

ipnat_yyreduce:
#if YYDEBUG
    if (ipnat_yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, ipnat_yystate, ipnat_yyn, ipnat_yyrule[ipnat_yyn]);
#endif
    ipnat_yym = ipnat_yylen[ipnat_yyn];
    ipnat_yyval = ipnat_yyvsp[1-ipnat_yym];
    switch (ipnat_yyn)
    {
case 5:
#line 109 "../tools/ipnat_y.y"
{ while ((nat = nattop) != NULL) {
				nattop = nat->in_next;
				(*nataddfunc)(natfd, natioctlfunc, nat);
				free(nat);
			  }
			  resetlexer();
			}
break;
case 7:
#line 119 "../tools/ipnat_y.y"
{ set_variable(ipnat_yyvsp[-3].str, ipnat_yyvsp[-1].str);
					  resetlexer();
					  free(ipnat_yyvsp[-3].str);
					  free(ipnat_yyvsp[-1].str);
					  ipnat_yyvarnext = 0;
					}
break;
case 8:
#line 128 "../tools/ipnat_y.y"
{ ipnat_yyvarnext = 1; }
break;
case 9:
#line 131 "../tools/ipnat_y.y"
{ newnatrule(); }
break;
case 15:
#line 143 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  nat->in_inip = ipnat_yyvsp[-4].ipp.a.s_addr;
				  nat->in_inmsk = ipnat_yyvsp[-4].ipp.m.s_addr;
				  nat->in_outip = ipnat_yyvsp[-2].ipp.a.s_addr;
				  nat->in_outmsk = ipnat_yyvsp[-2].ipp.m.s_addr;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				  if ((nat->in_flags & IPN_TCPUDP) == 0)
					setnatproto(nat->in_p);
				  if (((nat->in_redir & NAT_MAPBLK) != 0) ||
				      ((nat->in_flags & IPN_AUTOPORTMAP) != 0))
					nat_setgroupmap(nat);
				}
break;
case 16:
#line 159 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  nat->in_inip = ipnat_yyvsp[-4].ipp.a.s_addr;
				  nat->in_inmsk = ipnat_yyvsp[-4].ipp.m.s_addr;
				  nat->in_outip = ipnat_yyvsp[-2].ipp.a.s_addr;
				  nat->in_outmsk = ipnat_yyvsp[-2].ipp.m.s_addr;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				  if (((nat->in_redir & NAT_MAPBLK) != 0) ||
				      ((nat->in_flags & IPN_AUTOPORTMAP) != 0))
					nat_setgroupmap(nat);
				}
break;
case 17:
#line 173 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  nat->in_outip = ipnat_yyvsp[-2].ipp.a.s_addr;
				  nat->in_outmsk = ipnat_yyvsp[-2].ipp.m.s_addr;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				  if ((suggest_port == 1) &&
				      (nat->in_flags & IPN_TCPUDP) == 0)
					nat->in_flags |= IPN_TCPUDP;
				  if ((nat->in_flags & IPN_TCPUDP) == 0)
					setnatproto(nat->in_p);
				  if (((nat->in_redir & NAT_MAPBLK) != 0) ||
				      ((nat->in_flags & IPN_AUTOPORTMAP) != 0))
					nat_setgroupmap(nat);
				}
break;
case 18:
#line 190 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  nat->in_outip = ipnat_yyvsp[-2].ipp.a.s_addr;
				  nat->in_outmsk = ipnat_yyvsp[-2].ipp.m.s_addr;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				  if ((suggest_port == 1) &&
				      (nat->in_flags & IPN_TCPUDP) == 0)
					nat->in_flags |= IPN_TCPUDP;
				  if (((nat->in_redir & NAT_MAPBLK) != 0) ||
				      ((nat->in_flags & IPN_AUTOPORTMAP) != 0))
					nat_setgroupmap(nat);
				}
break;
case 19:
#line 208 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  nat->in_inip = ipnat_yyvsp[-4].ipp.a.s_addr;
				  nat->in_inmsk = ipnat_yyvsp[-4].ipp.m.s_addr;
				  nat->in_outip = ipnat_yyvsp[-2].ipp.a.s_addr;
				  nat->in_outmsk = ipnat_yyvsp[-2].ipp.m.s_addr;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				  if ((nat->in_flags & IPN_TCPUDP) == 0)
					setnatproto(nat->in_p);
				  if (((nat->in_redir & NAT_MAPBLK) != 0) ||
				      ((nat->in_flags & IPN_AUTOPORTMAP) != 0))
					nat_setgroupmap(nat);
				}
break;
case 20:
#line 226 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  nat->in_outip = ipnat_yyvsp[-6].ipp.a.s_addr;
				  nat->in_outmsk = ipnat_yyvsp[-6].ipp.m.s_addr;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				  if ((nat->in_p == 0) &&
				      ((nat->in_flags & IPN_TCPUDP) == 0) &&
				      (nat->in_pmin != 0 ||
				       nat->in_pmax != 0 ||
				       nat->in_pnext != 0))
					setnatproto(IPPROTO_TCP);
				}
break;
case 21:
#line 241 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  if ((nat->in_p == 0) &&
				      ((nat->in_flags & IPN_TCPUDP) == 0) &&
				      (nat->in_pmin != 0 ||
				       nat->in_pmax != 0 ||
				       nat->in_pnext != 0))
					setnatproto(IPPROTO_TCP);
				  if ((suggest_port == 1) &&
				      (nat->in_flags & IPN_TCPUDP) == 0)
					nat->in_flags |= IPN_TCPUDP;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				}
break;
case 22:
#line 257 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  nat->in_outip = ipnat_yyvsp[-4].ipp.a.s_addr;
				  nat->in_outmsk = ipnat_yyvsp[-4].ipp.m.s_addr;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				}
break;
case 23:
#line 266 "../tools/ipnat_y.y"
{ nat->in_v = 4;
				  if ((suggest_port == 1) &&
				      (nat->in_flags & IPN_TCPUDP) == 0)
					nat->in_flags |= IPN_TCPUDP;
				  if (nat->in_ifnames[1][0] == '\0')
					strncpy(nat->in_ifnames[1],
						nat->in_ifnames[0],
						sizeof(nat->in_ifnames[0]));
				}
break;
case 25:
#line 278 "../tools/ipnat_y.y"
{ strncpy(nat->in_plabel, ipnat_yyvsp[-2].str, sizeof(nat->in_plabel));
			  if (nat->in_dcmp == 0) {
				nat->in_dport = htons(ipnat_yyvsp[-3].port);
			  } else if (ipnat_yyvsp[-3].port != nat->in_dport) {
				ipnat_yyerror("proxy port numbers not consistant");
			  }
			  setnatproto(ipnat_yyvsp[0].num);
			  free(ipnat_yyvsp[-2].str);
			}
break;
case 26:
#line 288 "../tools/ipnat_y.y"
{ int pnum;
			  strncpy(nat->in_plabel, ipnat_yyvsp[-2].str, sizeof(nat->in_plabel));
			  pnum = getportproto(ipnat_yyvsp[-3].str, ipnat_yyvsp[0].num);
			  if (pnum == -1)
				ipnat_yyerror("invalid port number");
			  nat->in_dport = pnum;
			  setnatproto(ipnat_yyvsp[0].num);
			  free(ipnat_yyvsp[-3].str);
			  free(ipnat_yyvsp[-2].str);
			}
break;
case 28:
#line 301 "../tools/ipnat_y.y"
{ if (nat->in_p != 0 ||
					      nat->in_flags & IPN_TCPUDP)
						ipnat_yyerror("protocol set twice");
					  setnatproto(ipnat_yyvsp[0].num);
					}
break;
case 29:
#line 306 "../tools/ipnat_y.y"
{ if (nat->in_p != 0 ||
					      nat->in_flags & IPN_TCPUDP)
						ipnat_yyerror("protocol set twice");
					  nat->in_flags |= IPN_TCPUDP;
					  nat->in_p = 0;
					}
break;
case 30:
#line 312 "../tools/ipnat_y.y"
{ if (nat->in_p != 0 ||
					      nat->in_flags & IPN_TCPUDP)
						ipnat_yyerror("protocol set twice");
					  nat->in_flags |= IPN_TCPUDP;
					  nat->in_p = 0;
					}
break;
case 31:
#line 320 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[0].ipp.a; ipnat_yyval.ipp.m = ipnat_yyvsp[0].ipp.m; }
break;
case 32:
#line 322 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[-2].ipa; ipnat_yyval.ipp.m = ipnat_yyvsp[0].ipa;
					  nat->in_flags |= IPN_IPRANGE; }
break;
case 33:
#line 327 "../tools/ipnat_y.y"
{ nat->in_inip = ipnat_yyvsp[0].ipa.s_addr;
					  nat->in_inmsk = 0xffffffff; }
break;
case 34:
#line 329 "../tools/ipnat_y.y"
{ if (ipnat_yyvsp[0].num != 0 || ipnat_yyvsp[-2].ipa.s_addr != 0)
						ipnat_yyerror("Only 0/0 supported");
					  nat->in_inip = 0;
					  nat->in_inmsk = 0;
					}
break;
case 35:
#line 334 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_SPLIT;
					  nat->in_inip = ipnat_yyvsp[-2].ipa.s_addr;
					  nat->in_inmsk = ipnat_yyvsp[0].ipa.s_addr; }
break;
case 36:
#line 339 "../tools/ipnat_y.y"
{ suggest_port = 1; }
break;
case 37:
#line 343 "../tools/ipnat_y.y"
{ if (ipnat_yyvsp[0].num > 65535)	/* Unsigned */
						ipnat_yyerror("invalid port number");
					  else
						ipnat_yyval.port = ipnat_yyvsp[0].num;
					}
break;
case 38:
#line 348 "../tools/ipnat_y.y"
{ if (getport(NULL, ipnat_yyvsp[0].str, &(ipnat_yyval.port)) == -1)
						ipnat_yyerror("invalid port number");
					  ipnat_yyval.port = ntohs(ipnat_yyval.port);
					}
break;
case 40:
#line 354 "../tools/ipnat_y.y"
{ nat->in_pmin = htons(ipnat_yyvsp[0].port);
					  nat->in_pmax = htons(ipnat_yyvsp[0].port); }
break;
case 41:
#line 356 "../tools/ipnat_y.y"
{ nat->in_pmin = htons(ipnat_yyvsp[-2].port);
					  nat->in_pmax = htons(ipnat_yyvsp[0].port); }
break;
case 42:
#line 358 "../tools/ipnat_y.y"
{ nat->in_pmin = htons(ipnat_yyvsp[-2].port);
					  nat->in_pmax = htons(ipnat_yyvsp[0].port); }
break;
case 43:
#line 362 "../tools/ipnat_y.y"
{ nat->in_pnext = htons(ipnat_yyvsp[0].port); }
break;
case 44:
#line 363 "../tools/ipnat_y.y"
{ nat->in_pnext = htons(ipnat_yyvsp[0].port);
					  nat->in_flags |= IPN_FIXEDDPORT;
					}
break;
case 46:
#line 368 "../tools/ipnat_y.y"
{ nat->in_pmin = ipnat_yyvsp[0].num; }
break;
case 47:
#line 369 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_AUTOPORTMAP; }
break;
case 48:
#line 372 "../tools/ipnat_y.y"
{ nat->in_redir = NAT_MAP; }
break;
case 49:
#line 373 "../tools/ipnat_y.y"
{ nat->in_redir = NAT_BIMAP; }
break;
case 50:
#line 376 "../tools/ipnat_y.y"
{ nat->in_redir = NAT_REDIRECT; }
break;
case 51:
#line 380 "../tools/ipnat_y.y"
{ nat->in_redir = NAT_MAPBLK; }
break;
case 53:
#line 386 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_NOTDST; }
break;
case 54:
#line 388 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_NOTDST; }
break;
case 56:
#line 394 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_NOTSRC; }
break;
case 57:
#line 396 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_NOTSRC; }
break;
case 58:
#line 399 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_FILTER; }
break;
case 61:
#line 407 "../tools/ipnat_y.y"
{ strncpy(nat->in_ifnames[0], ipnat_yyvsp[0].str,
					  sizeof(nat->in_ifnames[0]));
				  nat->in_ifnames[0][LIFNAMSIZ - 1] = '\0';
				  free(ipnat_yyvsp[0].str);
				}
break;
case 62:
#line 415 "../tools/ipnat_y.y"
{ strncpy(nat->in_ifnames[1], ipnat_yyvsp[0].str,
					  sizeof(nat->in_ifnames[1]));
				  nat->in_ifnames[1][LIFNAMSIZ - 1] = '\0';
				  free(ipnat_yyvsp[0].str);
				}
break;
case 63:
#line 424 "../tools/ipnat_y.y"
{ nat->in_pmin = htons(ipnat_yyvsp[-3].port);
			  nat->in_pmax = htons(ipnat_yyvsp[-1].port);
			}
break;
case 64:
#line 428 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_AUTOPORTMAP;
			  nat->in_pmin = htons(1024);
			  nat->in_pmax = htons(65535);
			}
break;
case 65:
#line 433 "../tools/ipnat_y.y"
{ if (strcmp(ipnat_yyvsp[-3].str, "icmp") != 0) {
				ipnat_yyerror("icmpidmap not followed by icmp");
			  }
			  free(ipnat_yyvsp[-3].str);
			  if (ipnat_yyvsp[-2].num < 0 || ipnat_yyvsp[-2].num > 65535)
				ipnat_yyerror("invalid ICMP Id number");
			  if (ipnat_yyvsp[0].num < 0 || ipnat_yyvsp[0].num > 65535)
				ipnat_yyerror("invalid ICMP Id number");
			  nat->in_flags = IPN_ICMPQUERY;
			  nat->in_pmin = htons(ipnat_yyvsp[-2].num);
			  nat->in_pmax = htons(ipnat_yyvsp[0].num);
			}
break;
case 67:
#line 448 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_SEQUENTIAL; }
break;
case 69:
#line 453 "../tools/ipnat_y.y"
{ nat->in_sport = ipnat_yyvsp[0].pc.p1;
					  nat->in_stop = ipnat_yyvsp[0].pc.p2;
					  nat->in_scmp = ipnat_yyvsp[0].pc.pc; }
break;
case 70:
#line 458 "../tools/ipnat_y.y"
{ if (nat->in_redir == NAT_REDIRECT) {
						nat->in_srcip = ipnat_yyvsp[0].ipp.a.s_addr;
						nat->in_srcmsk = ipnat_yyvsp[0].ipp.m.s_addr;
					  } else {
						nat->in_inip = ipnat_yyvsp[0].ipp.a.s_addr;
						nat->in_inmsk = ipnat_yyvsp[0].ipp.m.s_addr;
					  }
					}
break;
case 72:
#line 470 "../tools/ipnat_y.y"
{ nat->in_dport = ipnat_yyvsp[0].pc.p1;
					  nat->in_dtop = ipnat_yyvsp[0].pc.p2;
					  nat->in_dcmp = ipnat_yyvsp[0].pc.pc;
					  if (nat->in_redir == NAT_REDIRECT)
						nat->in_pmin = htons(ipnat_yyvsp[0].pc.p1);
					}
break;
case 73:
#line 478 "../tools/ipnat_y.y"
{ if (nat->in_redir == NAT_REDIRECT) {
						nat->in_outip = ipnat_yyvsp[0].ipp.a.s_addr;
						nat->in_outmsk = ipnat_yyvsp[0].ipp.m.s_addr;
					  } else {
						nat->in_srcip = ipnat_yyvsp[0].ipp.a.s_addr;
						nat->in_srcmsk = ipnat_yyvsp[0].ipp.m.s_addr;
					  }
					}
break;
case 74:
#line 488 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a.s_addr = 0; ipnat_yyval.ipp.m.s_addr = 0; }
break;
case 75:
#line 489 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[0].ipp.a; ipnat_yyval.ipp.m = ipnat_yyvsp[0].ipp.m;
					  ipnat_yyval.ipp.a.s_addr &= ipnat_yyval.ipp.m.s_addr; }
break;
case 76:
#line 491 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[-2].ipa; ipnat_yyval.ipp.m = ipnat_yyvsp[0].ipa;
					  ipnat_yyval.ipp.a.s_addr &= ipnat_yyval.ipp.m.s_addr; }
break;
case 77:
#line 493 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[-2].ipa; ipnat_yyval.ipp.m.s_addr = htonl(ipnat_yyvsp[0].num);
					  ipnat_yyval.ipp.a.s_addr &= ipnat_yyval.ipp.m.s_addr; }
break;
case 78:
#line 495 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[-2].ipa; ipnat_yyval.ipp.m = ipnat_yyvsp[0].ipa;
					  ipnat_yyval.ipp.a.s_addr &= ipnat_yyval.ipp.m.s_addr; }
break;
case 79:
#line 497 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[-2].ipa; ipnat_yyval.ipp.m.s_addr = htonl(ipnat_yyvsp[0].num);
					  ipnat_yyval.ipp.a.s_addr &= ipnat_yyval.ipp.m.s_addr; }
break;
case 80:
#line 502 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[0].ipa;
					  ipnat_yyval.ipp.m.s_addr = 0xffffffff; }
break;
case 81:
#line 504 "../tools/ipnat_y.y"
{ ipnat_yyval.ipp.a = ipnat_yyvsp[-2].ipa;
					  ntomask(4, ipnat_yyvsp[0].num, &ipnat_yyval.ipp.m.s_addr); }
break;
case 82:
#line 509 "../tools/ipnat_y.y"
{ ipnat_yyval.pc.pc = ipnat_yyvsp[-1].num; ipnat_yyval.pc.p1 = ipnat_yyvsp[0].port; }
break;
case 83:
#line 510 "../tools/ipnat_y.y"
{ ipnat_yyval.pc.pc = ipnat_yyvsp[-1].num; ipnat_yyval.pc.p1 = ipnat_yyvsp[-2].port; ipnat_yyval.pc.p2 = ipnat_yyvsp[0].port; }
break;
case 87:
#line 521 "../tools/ipnat_y.y"
{ strncpy(nat->in_tag.ipt_tag, ipnat_yyvsp[0].str,
						  sizeof(nat->in_tag.ipt_tag));
					}
break;
case 89:
#line 525 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_ROUNDR; }
break;
case 91:
#line 528 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_FRAG; }
break;
case 93:
#line 531 "../tools/ipnat_y.y"
{ nat->in_age[0] = ipnat_yyvsp[0].num;
						  nat->in_age[1] = ipnat_yyvsp[0].num; }
break;
case 94:
#line 533 "../tools/ipnat_y.y"
{ nat->in_age[0] = ipnat_yyvsp[-2].num;
						  nat->in_age[1] = ipnat_yyvsp[0].num; }
break;
case 96:
#line 537 "../tools/ipnat_y.y"
{ if (!(nat->in_flags & IPN_ROUNDR) &&
					      !(nat->in_flags & IPN_SPLIT)) {
						fprintf(stderr,
		"'sticky' for use with round-robin/IP splitting only\n");
					  } else
						nat->in_flags |= IPN_STICKY;
					}
break;
case 98:
#line 547 "../tools/ipnat_y.y"
{ nat->in_mssclamp = ipnat_yyvsp[0].num; }
break;
case 100:
#line 550 "../tools/ipnat_y.y"
{ setnatproto(IPPROTO_TCP); }
break;
case 101:
#line 551 "../tools/ipnat_y.y"
{ setnatproto(IPPROTO_UDP); }
break;
case 102:
#line 552 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_TCPUDP;
					  nat->in_p = 0;
					}
break;
case 103:
#line 555 "../tools/ipnat_y.y"
{ nat->in_flags |= IPN_TCPUDP;
					  nat->in_p = 0;
					}
break;
case 104:
#line 562 "../tools/ipnat_y.y"
{ strncpy(nat->in_plabel, ipnat_yyvsp[0].str,
						  sizeof(nat->in_plabel));
					  nat->in_dport = nat->in_pnext;
					  nat->in_dport = htons(nat->in_dport);
					  free(ipnat_yyvsp[0].str);
					}
break;
case 105:
#line 568 "../tools/ipnat_y.y"
{ if (nat->in_plabel[0] != '\0') {
						  nat->in_pmin = nat->in_dport;
						  nat->in_pmax = nat->in_pmin;
						  nat->in_pnext = nat->in_pmin;
					  }
					}
break;
case 106:
#line 576 "../tools/ipnat_y.y"
{ ipnat_yyval.num = ipnat_yyvsp[0].num;
					  if (ipnat_yyval.num != IPPROTO_TCP &&
					      ipnat_yyval.num != IPPROTO_UDP)
						suggest_port = 0;
					}
break;
case 107:
#line 581 "../tools/ipnat_y.y"
{ ipnat_yyval.num = IPPROTO_TCP; }
break;
case 108:
#line 582 "../tools/ipnat_y.y"
{ ipnat_yyval.num = IPPROTO_UDP; }
break;
case 109:
#line 583 "../tools/ipnat_y.y"
{ ipnat_yyval.num = getproto(ipnat_yyvsp[0].str); free(ipnat_yyvsp[0].str);
					  if (ipnat_yyval.num != IPPROTO_TCP &&
					      ipnat_yyval.num != IPPROTO_UDP)
						suggest_port = 0;
					}
break;
case 110:
#line 591 "../tools/ipnat_y.y"
{ ipnat_yyval.num = ipnat_yyvsp[0].num; }
break;
case 111:
#line 595 "../tools/ipnat_y.y"
{ if (gethost(ipnat_yyvsp[0].str, &ipnat_yyval.ipa.s_addr) == -1)
						fprintf(stderr,
							"Unknown host '%s'\n",
							ipnat_yyvsp[0].str);
					  free(ipnat_yyvsp[0].str);
					}
break;
case 112:
#line 601 "../tools/ipnat_y.y"
{ ipnat_yyval.ipa.s_addr = htonl(ipnat_yyvsp[0].num); }
break;
case 113:
#line 602 "../tools/ipnat_y.y"
{ ipnat_yyval.ipa.s_addr = ipnat_yyvsp[0].ipa.s_addr; }
break;
case 114:
#line 606 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_EQUAL; }
break;
case 115:
#line 607 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_EQUAL; }
break;
case 116:
#line 608 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_NEQUAL; }
break;
case 117:
#line 609 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_LESST; }
break;
case 118:
#line 610 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_LESSTE; }
break;
case 119:
#line 611 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_GREATERT; }
break;
case 120:
#line 612 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_GREATERTE; }
break;
case 121:
#line 615 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_OUTRANGE; }
break;
case 122:
#line 616 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_INRANGE; }
break;
case 123:
#line 617 "../tools/ipnat_y.y"
{ ipnat_yyval.num = FR_INCRANGE; }
break;
case 124:
#line 621 "../tools/ipnat_y.y"
{ if (ipnat_yyvsp[-6].num > 255 || ipnat_yyvsp[-4].num > 255 || ipnat_yyvsp[-2].num > 255 || ipnat_yyvsp[0].num > 255) {
			ipnat_yyerror("Invalid octet string for IP address");
			return 0;
		  }
		  ipnat_yyval.ipa.s_addr = (ipnat_yyvsp[-6].num << 24) | (ipnat_yyvsp[-4].num << 16) | (ipnat_yyvsp[-2].num << 8) | ipnat_yyvsp[0].num;
		  ipnat_yyval.ipa.s_addr = htonl(ipnat_yyval.ipa.s_addr);
		}
break;
#line 1704 "ipnat_y.c"
    }
    ipnat_yyssp -= ipnat_yym;
    ipnat_yystate = *ipnat_yyssp;
    ipnat_yyvsp -= ipnat_yym;
    ipnat_yym = ipnat_yylhs[ipnat_yyn];
    if (ipnat_yystate == 0 && ipnat_yym == 0)
    {
#if YYDEBUG
        if (ipnat_yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        ipnat_yystate = YYFINAL;
        *++ipnat_yyssp = YYFINAL;
        *++ipnat_yyvsp = ipnat_yyval;
        if (ipnat_yychar < 0)
        {
            if ((ipnat_yychar = ipnat_yylex()) < 0) ipnat_yychar = 0;
#if YYDEBUG
            if (ipnat_yydebug)
            {
                ipnat_yys = 0;
                if (ipnat_yychar <= YYMAXTOKEN) ipnat_yys = ipnat_yyname[ipnat_yychar];
                if (!ipnat_yys) ipnat_yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, ipnat_yychar, ipnat_yys);
            }
#endif
        }
        if (ipnat_yychar == 0) goto ipnat_yyaccept;
        goto ipnat_yyloop;
    }
    if ((ipnat_yyn = ipnat_yygindex[ipnat_yym]) && (ipnat_yyn += ipnat_yystate) >= 0 &&
            ipnat_yyn <= YYTABLESIZE && ipnat_yycheck[ipnat_yyn] == ipnat_yystate)
        ipnat_yystate = ipnat_yytable[ipnat_yyn];
    else
        ipnat_yystate = ipnat_yydgoto[ipnat_yym];
#if YYDEBUG
    if (ipnat_yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *ipnat_yyssp, ipnat_yystate);
#endif
    if (ipnat_yyssp >= ipnat_yysslim && ipnat_yygrowstack())
    {
        goto ipnat_yyoverflow;
    }
    *++ipnat_yyssp = ipnat_yystate;
    *++ipnat_yyvsp = ipnat_yyval;
    goto ipnat_yyloop;

ipnat_yyoverflow:
    ipnat_yyerror("yacc stack overflow");

ipnat_yyabort:
    return (1);

ipnat_yyaccept:
    return (0);
}
