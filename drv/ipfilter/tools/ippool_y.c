#ifndef lint
static const char ippool_yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20050813

#define YYEMPTY (-1)
#define ippool_yyclearin    (ippool_yychar = YYEMPTY)
#define ippool_yyerrok      (ippool_yyerrflag = 0)
#define YYRECOVERING (ippool_yyerrflag != 0)

extern int ippool_yyparse(void);

static int ippool_yygrowstack(void);
#define YYPREFIX "ippool_yy"
#line 7 "../tools/ippool_y.y"
#include <sys/types.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/socket.h>
#if defined(BSD) && (BSD >= 199306)
# include <sys/cdefs.h>
#endif
#include <sys/ioctl.h>

#include <net/if.h>
#if __FreeBSD_version >= 300000
# include <net/if_var.h>
#endif
#include <netinet/in.h>

#include <arpa/inet.h>

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <ctype.h>
#include <unistd.h>

#include "ipf.h"
#include "netinet/ip_lookup.h"
#include "netinet/ip_pool.h"
#include "netinet/ip_htable.h"
#include "ippool_l.h"
#include "kmem.h"

#define	YYDEBUG	1
#define	YYSTACKSIZE	0x00ffffff

extern	int	ippool_yyparse __P((void));
extern	int	ippool_yydebug;
extern	FILE	*ippool_yyin;

static	iphtable_t	ipht;
static	iphtent_t	iphte;
static	ip_pool_t	iplo;
static	ioctlfunc_t	poolioctl = NULL;
static	char		poolname[FR_GROUPLEN];

static iphtent_t *add_htablehosts __P((char *));
static ip_pool_node_t *add_poolhosts __P((char *));

#line 57 "../tools/ippool_y.y"
typedef union	{
	char	*str;
	u_32_t	num;
	struct	in_addr	addr;
	struct	alist_s	*alist;
	struct	in_addr	adrmsk[2];
	iphtent_t	*ipe;
	ip_pool_node_t	*ipp;
	union	i6addr	ip6;
} YYSTYPE;
#line 82 "y.tab.c"
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
#define IPT_IPF 270
#define IPT_NAT 271
#define IPT_COUNT 272
#define IPT_AUTH 273
#define IPT_IN 274
#define IPT_OUT 275
#define IPT_TABLE 276
#define IPT_GROUPMAP 277
#define IPT_HASH 278
#define IPT_ROLE 279
#define IPT_TYPE 280
#define IPT_TREE 281
#define IPT_GROUP 282
#define IPT_SIZE 283
#define IPT_SEED 284
#define IPT_NUM 285
#define IPT_NAME 286
#define YYERRCODE 256
short ippool_yylhs[] = {                                        -1,
    0,    0,    0,    0,   20,   20,   20,   20,   22,   21,
   24,    2,   23,    3,    3,    1,    1,    1,    1,    4,
    9,    8,    8,   18,   18,   18,   19,   19,   27,   27,
   27,   27,    6,    6,    6,   14,   14,   14,   14,   14,
   13,   13,   13,   12,   12,    5,    5,    5,   10,   10,
   10,   11,   11,    7,    7,   15,   15,   16,   16,   25,
   26,   30,   28,   29,   17,
};
short ippool_yylen[] = {                                         2,
    1,    1,    2,    2,    4,    4,    5,    1,    1,    4,
    1,    1,    2,    1,    1,    3,    3,    3,    3,    7,
    8,    5,    4,    3,    3,    0,    3,    3,    0,    1,
    1,    2,    1,    3,    2,    1,    3,    3,    2,    2,
    1,    2,    3,    3,    1,    1,    2,    1,    1,    2,
    3,    1,    1,    3,    1,    1,    1,    1,    1,    1,
    1,    1,    3,    3,    7,
};
short ippool_yydefred[] = {                                      0,
    0,    8,   12,    0,    0,    0,    1,    2,    0,   11,
    0,   14,   15,   13,    3,    4,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   10,   16,   17,
   19,   18,    0,    9,    5,    6,    0,    0,    0,    0,
    0,    0,    0,    0,    0,   31,    0,    0,   24,   25,
    0,    0,    0,    7,    0,   60,    0,   32,    0,    0,
   28,   27,   63,   64,    0,    0,   45,   62,    0,    0,
    0,    0,   56,   41,    0,    0,    0,    0,    0,   36,
    0,    0,    0,   61,   23,    0,    0,   48,    0,    0,
    0,   46,   33,    0,    0,   22,    0,   44,   43,    0,
   54,   59,   53,   52,    0,    0,   49,   47,    0,   20,
   38,   37,    0,   21,    0,   34,    0,   51,    0,   65,
};
short ippool_yydgoto[] = {                                       5,
   18,    6,   14,   23,   90,   91,   77,   42,   24,  105,
  106,   78,   71,   79,   72,  101,   73,   27,   43,    7,
    8,   35,    9,   11,   57,   85,   44,   45,   46,   80,
};
short ippool_yysindex[] = {                                   -217,
  -47,    0,    0, -220, -217, -239,    0,    0, -239,    0,
 -187,    0,    0,    0,    0,    0,   12, -206, -228,   16,
 -226,   15,   18,   18,   17,   19, -232,    0,    0,    0,
    0,    0, -259,    0,    0,    0, -178, -177,   20,   22,
   24,   18, -218,  -39, -198,    0, -228, -228,    0,    0,
 -249, -170, -169,    0,  -39,    0,  -58,    0, -218,  -39,
    0,    0,    0,    0,  -58,   43,    0,    0,   46,   32,
  -31,   45,    0,    0,  -39,  -33,  -23,   32,  -31,    0,
 -164, -186,  -58,    0,    0, -162,  -57,    0, -160,   32,
  -31,    0,    0,  -58,  -58,    0,   52,    0,    0,   43,
    0,    0,    0,    0,  -31,   32,    0,    0,  -33,    0,
    0,    0, -158,    0,  -57,    0,   54,    0, -156,    0,
};
short ippool_yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0, -120,    0,
    0,    0,    0,    0,    0,    0,  -21,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -21,    0,  -20,    0, -118,  -19,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,  -21,    0,
    0,    0,    0,    0,    0,  -32,    0,    0,    0,    0,
    0,    4,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,  -18,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,  -17,  -16,    0,    0,    0,    0,    5,
    0,    0,    0,    0,    0,    0,    0,    0,  -15,    0,
    0,    0,    0,    0,  -14,    0,    0,    0,    0,    0,
};
short ippool_yygindex[] = {                                      0,
   96,    0,    0,    0,    0,   -3,  -48,    0,    0,   -2,
    0,  -50,   29,  -26,    0,    0,   28,   23,   33,  111,
  112,   -4,    0,    0,  -44,  -73,  -30,    0,   73,  -53,
};
#define YYTABLESIZE 226
short ippool_yytable[] = {                                      89,
   68,   68,   26,   74,   26,   96,   70,   61,   69,   62,
   65,   57,   55,   10,   57,   76,   83,  110,   47,   36,
   82,   48,   93,   94,   95,   68,   57,   92,   75,   74,
   87,  114,   70,  107,   69,   68,  109,   54,  104,   17,
  108,    1,    2,   29,   30,   31,   32,   55,   58,   39,
   40,   41,  115,   12,   13,   93,   25,   26,    3,    4,
   92,  107,   55,   58,   40,   41,  104,  111,  112,   59,
   60,   20,   21,   22,   28,   33,   34,   37,   49,   38,
   51,   50,   52,   56,   53,   41,   63,   64,   81,   82,
   68,   86,   97,   84,  100,   39,   66,  113,  117,  119,
  120,   29,   30,   26,   19,  116,   42,   40,   39,   35,
   50,   99,  118,  102,   98,   15,   16,   58,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,   26,   26,   26,   26,   26,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,   66,   66,
   67,  103,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,   66,    0,   88,
};
short ippool_yycheck[] = {                                      33,
   59,   59,  123,   57,  123,   79,   57,  257,   57,  259,
   55,   44,   43,   61,   47,   60,   70,   91,  278,   24,
   44,  281,   76,   77,   78,   59,   59,   76,   59,   83,
   75,  105,   83,   87,   83,   59,   90,   42,   87,  279,
   89,  259,  260,  270,  271,  272,  273,   44,   44,  282,
  283,  284,  106,  274,  275,  109,  285,  286,  276,  277,
  109,  115,   59,   59,  283,  284,  115,   94,   95,   47,
   48,  259,   61,  280,   59,   61,   59,   61,  257,   61,
   61,  259,   61,  123,   61,  284,  257,  257,   46,   44,
   59,   47,  257,  125,  257,  282,  257,   46,  257,   46,
  257,  123,  123,  123,    9,  109,  125,  125,  125,  125,
  125,   83,  115,   86,   82,    5,    5,   45,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,  282,  283,  284,  283,  284,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,  257,  257,
  259,  259,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,  257,   -1,  259,
};
#define YYFINAL 5
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 286
#if YYDEBUG
char *ippool_yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"'!'",0,0,0,0,0,0,0,0,0,0,"','",0,"'.'","'/'",0,0,0,0,0,0,0,0,0,0,0,"';'",0,
"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,"YY_NUMBER","YY_HEX","YY_STR","YY_COMMENT","YY_CMP_EQ","YY_CMP_NE",
"YY_CMP_LE","YY_CMP_GE","YY_CMP_LT","YY_CMP_GT","YY_RANGE_OUT","YY_RANGE_IN",
"YY_IPV6","IPT_IPF","IPT_NAT","IPT_COUNT","IPT_AUTH","IPT_IN","IPT_OUT",
"IPT_TABLE","IPT_GROUPMAP","IPT_HASH","IPT_ROLE","IPT_TYPE","IPT_TREE",
"IPT_GROUP","IPT_SIZE","IPT_SEED","IPT_NUM","IPT_NAME",
};
char *ippool_yyrule[] = {
"$accept : file",
"file : line",
"file : assign",
"file : file line",
"file : file assign",
"line : table role ipftree eol",
"line : table role ipfhash eol",
"line : groupmap role number ipfgroup eol",
"line : YY_COMMENT",
"eol : ';'",
"assign : YY_STR assigning YY_STR ';'",
"assigning : '='",
"table : IPT_TABLE",
"groupmap : IPT_GROUPMAP inout",
"inout : IPT_IN",
"inout : IPT_OUT",
"role : IPT_ROLE '=' IPT_IPF",
"role : IPT_ROLE '=' IPT_NAT",
"role : IPT_ROLE '=' IPT_AUTH",
"role : IPT_ROLE '=' IPT_COUNT",
"ipftree : IPT_TYPE '=' IPT_TREE number start addrlist end",
"ipfhash : IPT_TYPE '=' IPT_HASH number hashopts start hashlist end",
"ipfgroup : setgroup hashopts start grouplist end",
"ipfgroup : hashopts start setgrouplist end",
"number : IPT_NUM '=' YY_NUMBER",
"number : IPT_NAME '=' YY_STR",
"number :",
"setgroup : IPT_GROUP '=' YY_STR",
"setgroup : IPT_GROUP '=' YY_NUMBER",
"hashopts :",
"hashopts : size",
"hashopts : seed",
"hashopts : size seed",
"addrlist : next",
"addrlist : range next addrlist",
"addrlist : range next",
"grouplist : next",
"grouplist : groupentry next grouplist",
"grouplist : addrmask next grouplist",
"grouplist : groupentry next",
"grouplist : addrmask next",
"setgrouplist : next",
"setgrouplist : groupentry next",
"setgrouplist : groupentry next setgrouplist",
"groupentry : addrmask ',' setgroup",
"groupentry : YY_STR",
"range : addrmask",
"range : '!' addrmask",
"range : YY_STR",
"hashlist : next",
"hashlist : hashentry next",
"hashlist : hashentry next hashlist",
"hashentry : addrmask",
"hashentry : YY_STR",
"addrmask : ipaddr '/' mask",
"addrmask : ipaddr",
"ipaddr : ipv4",
"ipaddr : YY_NUMBER",
"mask : YY_NUMBER",
"mask : ipv4",
"start : '{'",
"end : '}'",
"next : ';'",
"size : IPT_SIZE '=' YY_NUMBER",
"seed : IPT_SEED '=' YY_NUMBER",
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

int      ippool_yydebug;
int      ippool_yynerrs;
int      ippool_yyerrflag;
int      ippool_yychar;
short   *ippool_yyssp;
YYSTYPE *ippool_yyvsp;
YYSTYPE  ippool_yyval;
YYSTYPE  ippool_yylval;

/* variables for the parser stack */
static short   *ippool_yyss;
static short   *ippool_yysslim;
static YYSTYPE *ippool_yyvs;
static int      ippool_yystacksize;
#line 338 "../tools/ippool_y.y"
static	wordtab_t	ippool_yywords[] = {
	{ "auth",	IPT_AUTH },
	{ "count",	IPT_COUNT },
	{ "group",	IPT_GROUP },
	{ "group-map",	IPT_GROUPMAP },
	{ "hash",	IPT_HASH },
	{ "in",		IPT_IN },
	{ "ipf",	IPT_IPF },
	{ "name",	IPT_NAME },
	{ "nat",	IPT_NAT },
	{ "number",	IPT_NUM },
	{ "out",	IPT_OUT },
	{ "role",	IPT_ROLE },
	{ "seed",	IPT_SEED },
	{ "size",	IPT_SIZE },
	{ "table",	IPT_TABLE },
	{ "tree",	IPT_TREE },
	{ "type",	IPT_TYPE },
	{ NULL,		0 }
};


int ippool_parsefile(fd, filename, iocfunc)
int fd;
char *filename;
ioctlfunc_t iocfunc;
{
	FILE *fp = NULL;
	char *s;

	ippool_yylineNum = 1;
	(void) ippool_yysettab(ippool_yywords);

	s = getenv("YYDEBUG");
	if (s)
		ippool_yydebug = atoi(s);
	else
		ippool_yydebug = 0;

	if (strcmp(filename, "-")) {
		fp = fopen(filename, "r");
		if (!fp) {
			fprintf(stderr, "fopen(%s) failed: %s\n", filename,
				STRERROR(errno));
			return -1;
		}
	} else
		fp = stdin;

	while (ippool_parsesome(fd, fp, iocfunc) == 1)
		;
	if (fp != NULL)
		fclose(fp);
	return 0;
}


int ippool_parsesome(fd, fp, iocfunc)
int fd;
FILE *fp;
ioctlfunc_t iocfunc;
{
	char *s;
	int i;

	poolioctl = iocfunc;

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
		ippool_yydebug = atoi(s);
	else
		ippool_yydebug = 0;

	ippool_yyin = fp;
	ippool_yyparse();
	return 1;
}


static iphtent_t *
add_htablehosts(url)
char *url;
{
	iphtent_t *htop, *hbot, *h;
	alist_t *a, *hlist;

	if (!strncmp(url, "file://", 7) || !strncmp(url, "http://", 7)) {
		hlist = load_url(url);
	} else {
		use_inet6 = 0;

		hlist = calloc(1, sizeof(*hlist));
		if (hlist == NULL)
			return NULL;

		if (gethost(url, &hlist->al_addr) == -1)
			ippool_yyerror("Unknown hostname");
	}

	hbot = NULL;
	htop = NULL;

	for (a = hlist; a != NULL; a = a->al_next) {
		h = calloc(1, sizeof(*h));
		if (h == NULL)
			break;

		bcopy((char *)&a->al_addr, (char *)&h->ipe_addr,
		      sizeof(h->ipe_addr));
		bcopy((char *)&a->al_mask, (char *)&h->ipe_mask,
		      sizeof(h->ipe_mask));

		if (hbot != NULL)
			hbot->ipe_next = h;
		else
			htop = h;
		hbot = h;
	}

	alist_free(hlist);

	return htop;
}


static ip_pool_node_t *
add_poolhosts(url)
char *url;
{
	ip_pool_node_t *ptop, *pbot, *p;
	alist_t *a, *hlist;

	if (!strncmp(url, "file://", 7) || !strncmp(url, "http://", 7)) {
		hlist = load_url(url);
	} else {
		use_inet6 = 0;

		hlist = calloc(1, sizeof(*hlist));
		if (hlist == NULL)
			return NULL;

		if (gethost(url, &hlist->al_addr) == -1)
			ippool_yyerror("Unknown hostname");
	}

	pbot = NULL;
	ptop = NULL;

	for (a = hlist; a != NULL; a = a->al_next) {
		p = calloc(1, sizeof(*p));
		if (p == NULL)
			break;

		p->ipn_addr.adf_len = offsetof(addrfamily_t, adf_addr) + 4;
		p->ipn_mask.adf_len = offsetof(addrfamily_t, adf_addr) + 4;

		p->ipn_info = a->al_not;

		bcopy((char *)&a->al_addr, (char *)&p->ipn_addr.adf_addr,
		      sizeof(p->ipn_addr.adf_addr));
		bcopy((char *)&a->al_mask, (char *)&p->ipn_mask.adf_addr,
		      sizeof(p->ipn_mask.adf_addr));

		if (pbot != NULL)
			pbot->ipn_next = p;
		else
			ptop = p;
		pbot = p;
	}

	alist_free(hlist);

	return ptop;
}
#line 541 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int ippool_yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = ippool_yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = ippool_yyssp - ippool_yyss;
    newss = (ippool_yyss != 0)
          ? (short *)realloc(ippool_yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    ippool_yyss  = newss;
    ippool_yyssp = newss + i;
    newvs = (ippool_yyvs != 0)
          ? (YYSTYPE *)realloc(ippool_yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    ippool_yyvs = newvs;
    ippool_yyvsp = newvs + i;
    ippool_yystacksize = newsize;
    ippool_yysslim = ippool_yyss + newsize - 1;
    return 0;
}

#define YYABORT goto ippool_yyabort
#define YYREJECT goto ippool_yyabort
#define YYACCEPT goto ippool_yyaccept
#define YYERROR goto ippool_yyerrlab
int
ippool_yyparse(void)
{
    register int ippool_yym, ippool_yyn, ippool_yystate;
#if YYDEBUG
    register const char *ippool_yys;

    if ((ippool_yys = getenv("YYDEBUG")) != 0)
    {
        ippool_yyn = *ippool_yys;
        if (ippool_yyn >= '0' && ippool_yyn <= '9')
            ippool_yydebug = ippool_yyn - '0';
    }
#endif

    ippool_yynerrs = 0;
    ippool_yyerrflag = 0;
    ippool_yychar = YYEMPTY;

    if (ippool_yyss == NULL && ippool_yygrowstack()) goto ippool_yyoverflow;
    ippool_yyssp = ippool_yyss;
    ippool_yyvsp = ippool_yyvs;
    *ippool_yyssp = ippool_yystate = 0;

ippool_yyloop:
    if ((ippool_yyn = ippool_yydefred[ippool_yystate]) != 0) goto ippool_yyreduce;
    if (ippool_yychar < 0)
    {
        if ((ippool_yychar = ippool_yylex()) < 0) ippool_yychar = 0;
#if YYDEBUG
        if (ippool_yydebug)
        {
            ippool_yys = 0;
            if (ippool_yychar <= YYMAXTOKEN) ippool_yys = ippool_yyname[ippool_yychar];
            if (!ippool_yys) ippool_yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, ippool_yystate, ippool_yychar, ippool_yys);
        }
#endif
    }
    if ((ippool_yyn = ippool_yysindex[ippool_yystate]) && (ippool_yyn += ippool_yychar) >= 0 &&
            ippool_yyn <= YYTABLESIZE && ippool_yycheck[ippool_yyn] == ippool_yychar)
    {
#if YYDEBUG
        if (ippool_yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, ippool_yystate, ippool_yytable[ippool_yyn]);
#endif
        if (ippool_yyssp >= ippool_yysslim && ippool_yygrowstack())
        {
            goto ippool_yyoverflow;
        }
        *++ippool_yyssp = ippool_yystate = ippool_yytable[ippool_yyn];
        *++ippool_yyvsp = ippool_yylval;
        ippool_yychar = YYEMPTY;
        if (ippool_yyerrflag > 0)  --ippool_yyerrflag;
        goto ippool_yyloop;
    }
    if ((ippool_yyn = ippool_yyrindex[ippool_yystate]) && (ippool_yyn += ippool_yychar) >= 0 &&
            ippool_yyn <= YYTABLESIZE && ippool_yycheck[ippool_yyn] == ippool_yychar)
    {
        ippool_yyn = ippool_yytable[ippool_yyn];
        goto ippool_yyreduce;
    }
    if (ippool_yyerrflag) goto ippool_yyinrecovery;

    ippool_yyerror("syntax error");

#ifdef lint
    goto ippool_yyerrlab;
#endif

ippool_yyerrlab:
    ++ippool_yynerrs;

ippool_yyinrecovery:
    if (ippool_yyerrflag < 3)
    {
        ippool_yyerrflag = 3;
        for (;;)
        {
            if ((ippool_yyn = ippool_yysindex[*ippool_yyssp]) && (ippool_yyn += YYERRCODE) >= 0 &&
                    ippool_yyn <= YYTABLESIZE && ippool_yycheck[ippool_yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (ippool_yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *ippool_yyssp, ippool_yytable[ippool_yyn]);
#endif
                if (ippool_yyssp >= ippool_yysslim && ippool_yygrowstack())
                {
                    goto ippool_yyoverflow;
                }
                *++ippool_yyssp = ippool_yystate = ippool_yytable[ippool_yyn];
                *++ippool_yyvsp = ippool_yylval;
                goto ippool_yyloop;
            }
            else
            {
#if YYDEBUG
                if (ippool_yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *ippool_yyssp);
#endif
                if (ippool_yyssp <= ippool_yyss) goto ippool_yyabort;
                --ippool_yyssp;
                --ippool_yyvsp;
            }
        }
    }
    else
    {
        if (ippool_yychar == 0) goto ippool_yyabort;
#if YYDEBUG
        if (ippool_yydebug)
        {
            ippool_yys = 0;
            if (ippool_yychar <= YYMAXTOKEN) ippool_yys = ippool_yyname[ippool_yychar];
            if (!ippool_yys) ippool_yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, ippool_yystate, ippool_yychar, ippool_yys);
        }
#endif
        ippool_yychar = YYEMPTY;
        goto ippool_yyloop;
    }

ippool_yyreduce:
#if YYDEBUG
    if (ippool_yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, ippool_yystate, ippool_yyn, ippool_yyrule[ippool_yyn]);
#endif
    ippool_yym = ippool_yylen[ippool_yyn];
    ippool_yyval = ippool_yyvsp[1-ippool_yym];
    switch (ippool_yyn)
    {
case 5:
#line 94 "../tools/ippool_y.y"
{ iplo.ipo_unit = ippool_yyvsp[-2].num;
					  iplo.ipo_list = ippool_yyvsp[-1].ipp;
					  load_pool(&iplo, poolioctl);
					  resetlexer();
					}
break;
case 6:
#line 99 "../tools/ippool_y.y"
{ ipht.iph_unit = ippool_yyvsp[-2].num;
					  ipht.iph_type = IPHASH_LOOKUP;
					  load_hash(&ipht, ippool_yyvsp[-1].ipe, poolioctl);
					  resetlexer();
					}
break;
case 7:
#line 105 "../tools/ippool_y.y"
{ ipht.iph_unit = ippool_yyvsp[-3].num;
					  strncpy(ipht.iph_name, ippool_yyvsp[-2].str,
						  sizeof(ipht.iph_name));
					  ipht.iph_type = IPHASH_GROUPMAP;
					  load_hash(&ipht, ippool_yyvsp[-1].ipe, poolioctl);
					  resetlexer();
					}
break;
case 10:
#line 118 "../tools/ippool_y.y"
{ set_variable(ippool_yyvsp[-3].str, ippool_yyvsp[-1].str);
					  resetlexer();
					  free(ippool_yyvsp[-3].str);
					  free(ippool_yyvsp[-1].str);
					  ippool_yyvarnext = 0;
					}
break;
case 11:
#line 127 "../tools/ippool_y.y"
{ ippool_yyvarnext = 1; }
break;
case 12:
#line 130 "../tools/ippool_y.y"
{ bzero((char *)&ipht, sizeof(ipht));
				  bzero((char *)&iphte, sizeof(iphte));
				  bzero((char *)&iplo, sizeof(iplo));
				  *ipht.iph_name = '\0';
				  iplo.ipo_flags = IPHASH_ANON;
				  iplo.ipo_name[0] = '\0';
				}
break;
case 13:
#line 140 "../tools/ippool_y.y"
{ bzero((char *)&ipht, sizeof(ipht));
				  bzero((char *)&iphte, sizeof(iphte));
				  *ipht.iph_name = '\0';
				  ipht.iph_unit = IPHASH_GROUPMAP;
				  ipht.iph_flags = ippool_yyvsp[0].num;
				}
break;
case 14:
#line 148 "../tools/ippool_y.y"
{ ippool_yyval.num = FR_INQUE; }
break;
case 15:
#line 149 "../tools/ippool_y.y"
{ ippool_yyval.num = FR_OUTQUE; }
break;
case 16:
#line 152 "../tools/ippool_y.y"
{ ippool_yyval.num = IPL_LOGIPF; }
break;
case 17:
#line 153 "../tools/ippool_y.y"
{ ippool_yyval.num = IPL_LOGNAT; }
break;
case 18:
#line 154 "../tools/ippool_y.y"
{ ippool_yyval.num = IPL_LOGAUTH; }
break;
case 19:
#line 155 "../tools/ippool_y.y"
{ ippool_yyval.num = IPL_LOGCOUNT; }
break;
case 20:
#line 160 "../tools/ippool_y.y"
{ strncpy(iplo.ipo_name, ippool_yyvsp[-3].str,
						  sizeof(iplo.ipo_name));
					  ippool_yyval.ipp = ippool_yyvsp[-1].ipp;
					}
break;
case 21:
#line 168 "../tools/ippool_y.y"
{ strncpy(ipht.iph_name, ippool_yyvsp[-4].str,
						  sizeof(ipht.iph_name));
					  ippool_yyval.ipe = ippool_yyvsp[-1].ipe;
					}
break;
case 22:
#line 176 "../tools/ippool_y.y"
{ iphtent_t *e;
					  for (e = ippool_yyvsp[-1].ipe; e != NULL;
					       e = e->ipe_next)
						if (e->ipe_group[0] == '\0')
							strncpy(e->ipe_group,
								ippool_yyvsp[-4].str,
								FR_GROUPLEN);
					  ippool_yyval.ipe = ippool_yyvsp[-1].ipe;
					}
break;
case 23:
#line 185 "../tools/ippool_y.y"
{ ippool_yyval.ipe = ippool_yyvsp[-1].ipe; }
break;
case 24:
#line 188 "../tools/ippool_y.y"
{ sprintf(poolname, "%u", ippool_yyvsp[0].num);
						  ippool_yyval.str = poolname;
						}
break;
case 25:
#line 191 "../tools/ippool_y.y"
{ ippool_yyval.str = ippool_yyvsp[0].str; }
break;
case 26:
#line 192 "../tools/ippool_y.y"
{ ippool_yyval.str = ""; }
break;
case 27:
#line 196 "../tools/ippool_y.y"
{ char tmp[FR_GROUPLEN+1];
					  strncpy(tmp, ippool_yyvsp[0].str, FR_GROUPLEN);
					  ippool_yyval.str = strdup(tmp);
					}
break;
case 28:
#line 200 "../tools/ippool_y.y"
{ char tmp[FR_GROUPLEN+1];
					  sprintf(tmp, "%u", ippool_yyvsp[0].num);
					  ippool_yyval.str = strdup(tmp);
					}
break;
case 33:
#line 213 "../tools/ippool_y.y"
{ ippool_yyval.ipp = NULL; }
break;
case 34:
#line 214 "../tools/ippool_y.y"
{ ippool_yyvsp[-2].ipp->ipn_next = ippool_yyvsp[0].ipp; ippool_yyval.ipp = ippool_yyvsp[-2].ipp; }
break;
case 35:
#line 215 "../tools/ippool_y.y"
{ ippool_yyval.ipp = ippool_yyvsp[-1].ipp; }
break;
case 36:
#line 219 "../tools/ippool_y.y"
{ ippool_yyval.ipe = NULL; }
break;
case 37:
#line 220 "../tools/ippool_y.y"
{ ippool_yyval.ipe = ippool_yyvsp[-2].ipe; ippool_yyvsp[-2].ipe->ipe_next = ippool_yyvsp[0].ipe; }
break;
case 38:
#line 221 "../tools/ippool_y.y"
{ ippool_yyval.ipe = calloc(1, sizeof(iphtent_t));
					  bcopy((char *)&(ippool_yyvsp[-2].adrmsk[0]),
						(char *)&(ippool_yyval.ipe->ipe_addr),
						sizeof(ippool_yyval.ipe->ipe_addr));
					  bcopy((char *)&(ippool_yyvsp[-2].adrmsk[1]),
						(char *)&(ippool_yyval.ipe->ipe_mask),
						sizeof(ippool_yyval.ipe->ipe_mask));
					  ippool_yyval.ipe->ipe_next = ippool_yyvsp[0].ipe;
					}
break;
case 39:
#line 230 "../tools/ippool_y.y"
{ ippool_yyval.ipe = ippool_yyvsp[-1].ipe; }
break;
case 40:
#line 231 "../tools/ippool_y.y"
{ ippool_yyval.ipe = calloc(1, sizeof(iphtent_t));
					  bcopy((char *)&(ippool_yyvsp[-1].adrmsk[0]),
						(char *)&(ippool_yyval.ipe->ipe_addr),
						sizeof(ippool_yyval.ipe->ipe_addr));
					  bcopy((char *)&(ippool_yyvsp[-1].adrmsk[1]),
						(char *)&(ippool_yyval.ipe->ipe_mask),
						sizeof(ippool_yyval.ipe->ipe_mask));
					}
break;
case 41:
#line 242 "../tools/ippool_y.y"
{ ippool_yyval.ipe = NULL; }
break;
case 42:
#line 243 "../tools/ippool_y.y"
{ ippool_yyval.ipe = ippool_yyvsp[-1].ipe; }
break;
case 43:
#line 244 "../tools/ippool_y.y"
{ ippool_yyvsp[-2].ipe->ipe_next = ippool_yyvsp[0].ipe; ippool_yyval.ipe = ippool_yyvsp[-2].ipe; }
break;
case 44:
#line 248 "../tools/ippool_y.y"
{ ippool_yyval.ipe = calloc(1, sizeof(iphtent_t));
					  bcopy((char *)&(ippool_yyvsp[-2].adrmsk[0]),
						(char *)&(ippool_yyval.ipe->ipe_addr),
						sizeof(ippool_yyval.ipe->ipe_addr));
					  bcopy((char *)&(ippool_yyvsp[-2].adrmsk[1]),
						(char *)&(ippool_yyval.ipe->ipe_mask),
						sizeof(ippool_yyval.ipe->ipe_mask));
					  strncpy(ippool_yyval.ipe->ipe_group, ippool_yyvsp[0].str,
						  FR_GROUPLEN);
					  free(ippool_yyvsp[0].str);
					}
break;
case 45:
#line 259 "../tools/ippool_y.y"
{ ippool_yyval.ipe = add_htablehosts(ippool_yyvsp[0].str); }
break;
case 46:
#line 262 "../tools/ippool_y.y"
{ ippool_yyval.ipp = calloc(1, sizeof(*ippool_yyval.ipp));
			  ippool_yyval.ipp->ipn_info = 0;
			  ippool_yyval.ipp->ipn_addr.adf_len = sizeof(ippool_yyval.ipp->ipn_addr) + 4;
			  ippool_yyval.ipp->ipn_addr.adf_addr.in4.s_addr = ippool_yyvsp[0].adrmsk[0].s_addr;
			  ippool_yyval.ipp->ipn_mask.adf_len = sizeof(ippool_yyval.ipp->ipn_mask) + 4;
			  ippool_yyval.ipp->ipn_mask.adf_addr.in4.s_addr = ippool_yyvsp[0].adrmsk[1].s_addr;
			}
break;
case 47:
#line 269 "../tools/ippool_y.y"
{ ippool_yyval.ipp = calloc(1, sizeof(*ippool_yyval.ipp));
			  ippool_yyval.ipp->ipn_info = 1;
			  ippool_yyval.ipp->ipn_addr.adf_len = sizeof(ippool_yyval.ipp->ipn_addr) + 4;
			  ippool_yyval.ipp->ipn_addr.adf_addr.in4.s_addr = ippool_yyvsp[0].adrmsk[0].s_addr;
			  ippool_yyval.ipp->ipn_mask.adf_len = sizeof(ippool_yyval.ipp->ipn_mask) + 4;
			  ippool_yyval.ipp->ipn_mask.adf_addr.in4.s_addr = ippool_yyvsp[0].adrmsk[1].s_addr;
			}
break;
case 48:
#line 276 "../tools/ippool_y.y"
{ ippool_yyval.ipp = add_poolhosts(ippool_yyvsp[0].str); }
break;
case 49:
#line 279 "../tools/ippool_y.y"
{ ippool_yyval.ipe = NULL; }
break;
case 50:
#line 280 "../tools/ippool_y.y"
{ ippool_yyval.ipe = ippool_yyvsp[-1].ipe; }
break;
case 51:
#line 281 "../tools/ippool_y.y"
{ ippool_yyvsp[-2].ipe->ipe_next = ippool_yyvsp[0].ipe; ippool_yyval.ipe = ippool_yyvsp[-2].ipe; }
break;
case 52:
#line 285 "../tools/ippool_y.y"
{ ippool_yyval.ipe = calloc(1, sizeof(iphtent_t));
					  bcopy((char *)&(ippool_yyvsp[0].adrmsk[0]),
						(char *)&(ippool_yyval.ipe->ipe_addr),
						sizeof(ippool_yyval.ipe->ipe_addr));
					  bcopy((char *)&(ippool_yyvsp[0].adrmsk[1]),
						(char *)&(ippool_yyval.ipe->ipe_mask),
						sizeof(ippool_yyval.ipe->ipe_mask));
					}
break;
case 53:
#line 293 "../tools/ippool_y.y"
{ ippool_yyval.ipe = add_htablehosts(ippool_yyvsp[0].str); }
break;
case 54:
#line 297 "../tools/ippool_y.y"
{ ippool_yyval.adrmsk[0] = ippool_yyvsp[-2].addr; ippool_yyval.adrmsk[1].s_addr = ippool_yyvsp[0].addr.s_addr;
				  ippool_yyexpectaddr = 0;
				}
break;
case 55:
#line 300 "../tools/ippool_y.y"
{ ippool_yyval.adrmsk[0] = ippool_yyvsp[0].addr; ippool_yyval.adrmsk[1].s_addr = 0xffffffff;
				  ippool_yyexpectaddr = 0;
				}
break;
case 56:
#line 305 "../tools/ippool_y.y"
{ ippool_yyval.addr = ippool_yyvsp[0].addr; }
break;
case 57:
#line 306 "../tools/ippool_y.y"
{ ippool_yyval.addr.s_addr = htonl(ippool_yyvsp[0].num); }
break;
case 58:
#line 309 "../tools/ippool_y.y"
{ ntomask(4, ippool_yyvsp[0].num, (u_32_t *)&ippool_yyval.addr.s_addr); }
break;
case 59:
#line 310 "../tools/ippool_y.y"
{ ippool_yyval.addr = ippool_yyvsp[0].addr; }
break;
case 60:
#line 313 "../tools/ippool_y.y"
{ ippool_yyexpectaddr = 1; }
break;
case 61:
#line 316 "../tools/ippool_y.y"
{ ippool_yyexpectaddr = 0; }
break;
case 62:
#line 319 "../tools/ippool_y.y"
{ ippool_yyexpectaddr = 1; }
break;
case 63:
#line 322 "../tools/ippool_y.y"
{ ipht.iph_size = ippool_yyvsp[0].num; }
break;
case 64:
#line 325 "../tools/ippool_y.y"
{ ipht.iph_seed = ippool_yyvsp[0].num; }
break;
case 65:
#line 329 "../tools/ippool_y.y"
{ if (ippool_yyvsp[-6].num > 255 || ippool_yyvsp[-4].num > 255 || ippool_yyvsp[-2].num > 255 || ippool_yyvsp[0].num > 255) {
			ippool_yyerror("Invalid octet string for IP address");
			return 0;
		  }
		  ippool_yyval.addr.s_addr = (ippool_yyvsp[-6].num << 24) | (ippool_yyvsp[-4].num << 16) | (ippool_yyvsp[-2].num << 8) | ippool_yyvsp[0].num;
		  ippool_yyval.addr.s_addr = htonl(ippool_yyval.addr.s_addr);
		}
break;
#line 1045 "y.tab.c"
    }
    ippool_yyssp -= ippool_yym;
    ippool_yystate = *ippool_yyssp;
    ippool_yyvsp -= ippool_yym;
    ippool_yym = ippool_yylhs[ippool_yyn];
    if (ippool_yystate == 0 && ippool_yym == 0)
    {
#if YYDEBUG
        if (ippool_yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        ippool_yystate = YYFINAL;
        *++ippool_yyssp = YYFINAL;
        *++ippool_yyvsp = ippool_yyval;
        if (ippool_yychar < 0)
        {
            if ((ippool_yychar = ippool_yylex()) < 0) ippool_yychar = 0;
#if YYDEBUG
            if (ippool_yydebug)
            {
                ippool_yys = 0;
                if (ippool_yychar <= YYMAXTOKEN) ippool_yys = ippool_yyname[ippool_yychar];
                if (!ippool_yys) ippool_yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, ippool_yychar, ippool_yys);
            }
#endif
        }
        if (ippool_yychar == 0) goto ippool_yyaccept;
        goto ippool_yyloop;
    }
    if ((ippool_yyn = ippool_yygindex[ippool_yym]) && (ippool_yyn += ippool_yystate) >= 0 &&
            ippool_yyn <= YYTABLESIZE && ippool_yycheck[ippool_yyn] == ippool_yystate)
        ippool_yystate = ippool_yytable[ippool_yyn];
    else
        ippool_yystate = ippool_yydgoto[ippool_yym];
#if YYDEBUG
    if (ippool_yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *ippool_yyssp, ippool_yystate);
#endif
    if (ippool_yyssp >= ippool_yysslim && ippool_yygrowstack())
    {
        goto ippool_yyoverflow;
    }
    *++ippool_yyssp = ippool_yystate;
    *++ippool_yyvsp = ippool_yyval;
    goto ippool_yyloop;

ippool_yyoverflow:
    ippool_yyerror("yacc stack overflow");

ippool_yyabort:
    return (1);

ippool_yyaccept:
    return (0);
}
