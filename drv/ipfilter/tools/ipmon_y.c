#ifndef lint
static const char ipmon_yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20050813

#define YYEMPTY (-1)
#define ipmon_yyclearin    (ipmon_yychar = YYEMPTY)
#define ipmon_yyerrok      (ipmon_yyerrflag = 0)
#define YYRECOVERING (ipmon_yyerrflag != 0)

extern int ipmon_yyparse(void);

static int ipmon_yygrowstack(void);
#define YYPREFIX "ipmon_yy"
#line 7 "../tools/ipmon_y.y"
#include "ipf.h"
#include <syslog.h>
#undef	OPT_NAT
#undef	OPT_VERBOSE
#include "ipmon_l.h"
#include "ipmon.h"

#define	YYDEBUG	1

extern	void	ipmon_yyerror __P((char *));
extern	int	ipmon_yyparse __P((void));
extern	int	ipmon_yylex __P((void));
extern	int	ipmon_yydebug;
extern	FILE	*ipmon_yyin;
extern	int	ipmon_yylineNum;

typedef	struct	opt	{
	struct	opt	*o_next;
	int		o_line;
	int		o_type;
	int		o_num;
	char		*o_str;
	struct in_addr	o_ip;
} opt_t;

static	void	build_action __P((struct opt *));
static	opt_t	*new_opt __P((int));
static	void	free_action __P((ipmon_action_t *));

static	ipmon_action_t	*alist = NULL;
#line 39 "../tools/ipmon_y.y"
typedef union	{
	char	*str;
	u_32_t	num;
	struct in_addr	addr;
	struct opt	*opt;
	union	i6addr	ip6;
} YYSTYPE;
#line 61 "y.tab.c"
#define YY_NUMBER 257
#define YY_HEX 258
#define YY_STR 259
#define YY_IPV6 260
#define YY_COMMENT 261
#define YY_CMP_EQ 262
#define YY_CMP_NE 263
#define YY_CMP_LE 264
#define YY_CMP_GE 265
#define YY_CMP_LT 266
#define YY_CMP_GT 267
#define YY_RANGE_OUT 268
#define YY_RANGE_IN 269
#define IPM_MATCH 270
#define IPM_BODY 271
#define IPM_COMMENT 272
#define IPM_DIRECTION 273
#define IPM_DSTIP 274
#define IPM_DSTPORT 275
#define IPM_EVERY 276
#define IPM_EXECUTE 277
#define IPM_GROUP 278
#define IPM_INTERFACE 279
#define IPM_IN 280
#define IPM_NO 281
#define IPM_OUT 282
#define IPM_PACKET 283
#define IPM_PACKETS 284
#define IPM_POOL 285
#define IPM_PROTOCOL 286
#define IPM_RESULT 287
#define IPM_RULE 288
#define IPM_SECOND 289
#define IPM_SECONDS 290
#define IPM_SRCIP 291
#define IPM_SRCPORT 292
#define IPM_LOGTAG 293
#define IPM_WITH 294
#define IPM_DO 295
#define IPM_SAVE 296
#define IPM_SYSLOG 297
#define IPM_NOTHING 298
#define IPM_RAW 299
#define IPM_TYPE 300
#define IPM_NAT 301
#define IPM_STATE 302
#define IPM_NATTAG 303
#define IPM_IPF 304
#define YYERRCODE 256
short ipmon_yylhs[] = {                                        -1,
    0,    0,    0,    0,   27,   27,   27,   28,   29,   15,
   15,   16,   16,   16,   16,   16,   16,   16,   16,   16,
   16,   16,   16,   16,   16,   20,   20,   19,   19,   19,
   19,    2,    2,    3,    4,    4,    5,    5,    5,    5,
    7,    7,    8,   14,   17,    9,    9,   10,   11,   12,
   13,   13,   18,   26,   26,   26,    6,   21,   24,   24,
   24,   25,   22,   23,    1,
};
short ipmon_yylen[] = {                                         2,
    1,    1,    2,    2,    9,    1,    1,    4,    1,    1,
    3,    1,    1,    1,    1,    1,    1,    1,    1,    1,
    1,    1,    1,    1,    1,    1,    3,    1,    1,    1,
    1,    3,    3,    5,    3,    3,    2,    3,    2,    3,
    3,    3,    3,    3,    3,    3,    3,    3,    3,    5,
    3,    3,    3,    1,    1,    1,    2,    3,    0,    1,
    3,    1,    1,    1,    7,
};
short ipmon_yydefred[] = {                                      0,
    0,    7,    0,    6,    0,    1,    2,    9,    0,    0,
    3,    4,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   12,   13,   14,
   15,   16,   17,   18,   19,   20,   21,   22,   23,    0,
    0,   24,   25,    8,    0,    0,    0,    0,   39,   37,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,   32,   33,    0,    0,   35,   36,   40,   38,
   41,   42,   43,   46,   47,   48,   49,    0,   51,   52,
   44,   55,   56,   54,   53,   45,    0,   11,    0,    0,
    0,    0,    0,   34,   50,    0,    0,   63,   64,   28,
    0,    0,   29,   30,   31,    0,   57,   62,    0,    0,
    0,    0,    0,   58,    0,   27,    5,    0,   61,   65,
};
short ipmon_yydgoto[] = {                                       5,
   66,   28,   29,   30,   31,  100,   32,   33,   34,   35,
   36,   37,   38,   39,   40,   41,   42,   43,  101,  102,
  103,  104,  105,  109,  110,   85,    6,    7,    9,
};
short ipmon_yysindex[] = {                                   -248,
  -56,    0, -115,    0, -248,    0,    0,    0, -241, -272,
    0,    0,  -29,  -27,  -17,  -15, -257,  -14,  -13,  -12,
  -11,  -10,   -9,   -8,   -7,   -6,   -5,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,  -68,
   14,    0,    0,    0, -255, -198, -224, -261,    0,    0,
 -218, -199, -217, -197, -196, -198, -214, -194, -292, -195,
 -230, -272,    0,    0,   20,   21,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   22,    0,    0,
    0,    0,    0,    0,    0,    0,  -53,    0, -190, -186,
 -185, -260,   27,    0,    0, -184, -225,    0,    0,    0,
   32,  -48,    0,    0,    0, -179,    0,    0, -180,   36,
 -260,   23,   35,    0, -225,    0,    0, -174,    0,    0,
};
short ipmon_yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
  -41,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0, -173,    0,    0,    0,
  -40,    0,    0,    0,    0,    0,    0,    0,    0, -172,
    0,    0,    0,    0, -173,    0,    0,    0,    0,    0,
};
short ipmon_yygindex[] = {                                      0,
   33,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,   26,    0,    0,    0,    0,  -21,
    0,    0,    0,  -24,    0,    0,   87,   88,    0,
};
#define YYTABLESIZE 93
short ipmon_yytable[] = {                                      48,
   14,   15,   16,   17,    8,   18,   19,   10,   82,   83,
    1,   84,    2,   20,   21,   22,   96,   13,   23,   24,
   25,    3,   69,    4,   63,   49,   64,   26,   70,   44,
   27,   50,   67,   45,   68,   97,   98,   99,   71,   74,
   72,   75,   79,   46,   80,   47,   51,   52,   53,   54,
   55,   56,   57,   58,   59,   60,   61,   62,   65,   73,
   77,   76,   81,   86,   87,   89,   93,   90,   91,   92,
   94,   95,  106,  108,  107,  111,  112,  113,  114,  115,
  118,  117,  120,   10,   26,   59,   60,   88,   78,  116,
  119,   11,   12,
};
short ipmon_yycheck[] = {                                     257,
  273,  274,  275,  276,   61,  278,  279,  123,  301,  302,
  259,  304,  261,  286,  287,  288,  277,  259,  291,  292,
  293,  270,  284,  272,  280,  283,  282,  300,  290,   59,
  303,  289,  257,   61,  259,  296,  297,  298,  257,  257,
  259,  259,  257,   61,  259,   61,   61,   61,   61,   61,
   61,   61,   61,   61,   61,   61,  125,   44,  257,  259,
  257,  259,  257,  259,  295,   46,  257,   47,   47,  123,
  257,  257,   46,  299,  259,   44,  125,  257,  259,   44,
   46,   59,  257,  125,  125,  259,  259,   62,   56,  111,
  115,    5,    5,
};
#define YYFINAL 5
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 304
#if YYDEBUG
char *ipmon_yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,"','",0,"'.'","'/'",0,0,0,0,0,0,0,0,0,0,0,"';'",0,"'='",0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,"'{'",0,"'}'",0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"YY_NUMBER","YY_HEX","YY_STR","YY_IPV6","YY_COMMENT","YY_CMP_EQ","YY_CMP_NE",
"YY_CMP_LE","YY_CMP_GE","YY_CMP_LT","YY_CMP_GT","YY_RANGE_OUT","YY_RANGE_IN",
"IPM_MATCH","IPM_BODY","IPM_COMMENT","IPM_DIRECTION","IPM_DSTIP","IPM_DSTPORT",
"IPM_EVERY","IPM_EXECUTE","IPM_GROUP","IPM_INTERFACE","IPM_IN","IPM_NO",
"IPM_OUT","IPM_PACKET","IPM_PACKETS","IPM_POOL","IPM_PROTOCOL","IPM_RESULT",
"IPM_RULE","IPM_SECOND","IPM_SECONDS","IPM_SRCIP","IPM_SRCPORT","IPM_LOGTAG",
"IPM_WITH","IPM_DO","IPM_SAVE","IPM_SYSLOG","IPM_NOTHING","IPM_RAW","IPM_TYPE",
"IPM_NAT","IPM_STATE","IPM_NATTAG","IPM_IPF",
};
char *ipmon_yyrule[] = {
"$accept : file",
"file : line",
"file : assign",
"file : file line",
"file : file assign",
"line : IPM_MATCH '{' matching '}' IPM_DO '{' doing '}' ';'",
"line : IPM_COMMENT",
"line : YY_COMMENT",
"assign : YY_STR assigning YY_STR ';'",
"assigning : '='",
"matching : matchopt",
"matching : matchopt ',' matching",
"matchopt : direction",
"matchopt : dstip",
"matchopt : dstport",
"matchopt : every",
"matchopt : group",
"matchopt : interface",
"matchopt : protocol",
"matchopt : result",
"matchopt : rule",
"matchopt : srcip",
"matchopt : srcport",
"matchopt : logtag",
"matchopt : nattag",
"matchopt : type",
"doing : doopt",
"doing : doopt ',' doing",
"doopt : execute",
"doopt : save",
"doopt : syslog",
"doopt : nothing",
"direction : IPM_DIRECTION '=' IPM_IN",
"direction : IPM_DIRECTION '=' IPM_OUT",
"dstip : IPM_DSTIP '=' ipv4 '/' YY_NUMBER",
"dstport : IPM_DSTPORT '=' YY_NUMBER",
"dstport : IPM_DSTPORT '=' YY_STR",
"every : IPM_EVERY IPM_SECOND",
"every : IPM_EVERY YY_NUMBER IPM_SECONDS",
"every : IPM_EVERY IPM_PACKET",
"every : IPM_EVERY YY_NUMBER IPM_PACKETS",
"group : IPM_GROUP '=' YY_NUMBER",
"group : IPM_GROUP '=' YY_STR",
"interface : IPM_INTERFACE '=' YY_STR",
"logtag : IPM_LOGTAG '=' YY_NUMBER",
"nattag : IPM_NATTAG '=' YY_STR",
"protocol : IPM_PROTOCOL '=' YY_NUMBER",
"protocol : IPM_PROTOCOL '=' YY_STR",
"result : IPM_RESULT '=' YY_STR",
"rule : IPM_RULE '=' YY_NUMBER",
"srcip : IPM_SRCIP '=' ipv4 '/' YY_NUMBER",
"srcport : IPM_SRCPORT '=' YY_NUMBER",
"srcport : IPM_SRCPORT '=' YY_STR",
"type : IPM_TYPE '=' typeopt",
"typeopt : IPM_IPF",
"typeopt : IPM_NAT",
"typeopt : IPM_STATE",
"execute : IPM_EXECUTE YY_STR",
"save : IPM_SAVE saveopts YY_STR",
"saveopts :",
"saveopts : saveopt",
"saveopts : saveopt ',' saveopts",
"saveopt : IPM_RAW",
"syslog : IPM_SYSLOG",
"nothing : IPM_NOTHING",
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

int      ipmon_yydebug;
int      ipmon_yynerrs;
int      ipmon_yyerrflag;
int      ipmon_yychar;
short   *ipmon_yyssp;
YYSTYPE *ipmon_yyvsp;
YYSTYPE  ipmon_yyval;
YYSTYPE  ipmon_yylval;

/* variables for the parser stack */
static short   *ipmon_yyss;
static short   *ipmon_yysslim;
static YYSTYPE *ipmon_yyvs;
static int      ipmon_yystacksize;
#line 247 "../tools/ipmon_y.y"
static	struct	wordtab	ipmon_yywords[] = {
	{ "body",	IPM_BODY },
	{ "direction",	IPM_DIRECTION },
	{ "do",		IPM_DO },
	{ "dstip",	IPM_DSTIP },
	{ "dstport",	IPM_DSTPORT },
	{ "every",	IPM_EVERY },
	{ "execute",	IPM_EXECUTE },
	{ "group",	IPM_GROUP },
	{ "in",		IPM_IN },
	{ "interface",	IPM_INTERFACE },
	{ "ipf",	IPM_IPF },
	{ "logtag",	IPM_LOGTAG },
	{ "match",	IPM_MATCH },
	{ "nat",	IPM_NAT },
	{ "nattag",	IPM_NATTAG },
	{ "no",		IPM_NO },
	{ "nothing",	IPM_NOTHING },
	{ "out",	IPM_OUT },
	{ "packet",	IPM_PACKET },
	{ "packets",	IPM_PACKETS },
	{ "protocol",	IPM_PROTOCOL },
	{ "result",	IPM_RESULT },
	{ "rule",	IPM_RULE },
	{ "save",	IPM_SAVE },
	{ "second",	IPM_SECOND },
	{ "seconds",	IPM_SECONDS },
	{ "srcip",	IPM_SRCIP },
	{ "srcport",	IPM_SRCPORT },
	{ "state",	IPM_STATE },
	{ "syslog",	IPM_SYSLOG },
	{ "with",	IPM_WITH },
	{ NULL,		0 }
};

static int macflags[17][2] = {
	{ IPM_DIRECTION,	IPMAC_DIRECTION	},
	{ IPM_DSTIP,		IPMAC_DSTIP	},
	{ IPM_DSTPORT,		IPMAC_DSTPORT	},
	{ IPM_GROUP,		IPMAC_GROUP	},
	{ IPM_INTERFACE,	IPMAC_INTERFACE	},
	{ IPM_LOGTAG,		IPMAC_LOGTAG 	},
	{ IPM_NATTAG,		IPMAC_NATTAG 	},
	{ IPM_PACKET,		IPMAC_EVERY	},
	{ IPM_PROTOCOL,		IPMAC_PROTOCOL	},
	{ IPM_RESULT,		IPMAC_RESULT	},
	{ IPM_RULE,		IPMAC_RULE	},
	{ IPM_SECOND,		IPMAC_EVERY	},
	{ IPM_SRCIP,		IPMAC_SRCIP	},
	{ IPM_SRCPORT,		IPMAC_SRCPORT	},
	{ IPM_TYPE,		IPMAC_TYPE 	},
	{ IPM_WITH,		IPMAC_WITH 	},
	{ 0, 0 }
};

static opt_t *new_opt(type)
int type;
{
	opt_t *o;

	o = (opt_t *)malloc(sizeof(*o));
	o->o_type = type;
	o->o_line = ipmon_yylineNum;
	o->o_num = 0;
	o->o_str = (char *)0;
	o->o_next = NULL;
	return o;
}

static void build_action(olist)
opt_t *olist;
{
	ipmon_action_t *a;
	opt_t *o;
	char c;
	int i;

	a = (ipmon_action_t *)calloc(1, sizeof(*a));
	if (a == NULL)
		return;
	while ((o = olist) != NULL) {
		/*
		 * Check to see if the same comparator is being used more than
		 * once per matching statement.
		 */
		for (i = 0; macflags[i][0]; i++)
			if (macflags[i][0] == o->o_type)
				break;
		if (macflags[i][1] & a->ac_mflag) {
			fprintf(stderr, "%s redfined on line %d\n",
				ipmon_yykeytostr(o->o_type), ipmon_yylineNum);
			if (o->o_str != NULL)
				free(o->o_str);
			olist = o->o_next;
			free(o);
			continue;
		}

		a->ac_mflag |= macflags[i][1];

		switch (o->o_type)
		{
		case IPM_DIRECTION :
			a->ac_direction = o->o_num;
			break;
		case IPM_DSTIP :
			a->ac_dip = o->o_ip.s_addr;
			a->ac_dmsk = htonl(0xffffffff << (32 - o->o_num));
			break;
		case IPM_DSTPORT :
			a->ac_dport = htons(o->o_num);
			break;
		case IPM_EXECUTE :
			a->ac_exec = o->o_str;
			c = *o->o_str;
			if (c== '"'|| c == '\'') {
				if (o->o_str[strlen(o->o_str) - 1] == c) {
					a->ac_run = strdup(o->o_str + 1);
					a->ac_run[strlen(a->ac_run) - 1] ='\0';
				} else
					a->ac_run = o->o_str;
			} else
				a->ac_run = o->o_str;
			o->o_str = NULL;
			break;
		case IPM_INTERFACE :
			a->ac_iface = o->o_str;
			o->o_str = NULL;
			break;
		case IPM_GROUP :
			if (o->o_str != NULL)
				strncpy(a->ac_group, o->o_str, FR_GROUPLEN);
			else
				sprintf(a->ac_group, "%d", o->o_num);
			break;
		case IPM_LOGTAG :
			a->ac_logtag = o->o_num;
			break;
		case IPM_NATTAG :
			strncpy(a->ac_nattag, o->o_str, sizeof(a->ac_nattag));
			break;
		case IPM_PACKET :
			a->ac_packet = o->o_num;
			break;
		case IPM_PROTOCOL :
			a->ac_proto = o->o_num;
			break;
		case IPM_RULE :
			a->ac_rule = o->o_num;
			break;
		case IPM_RESULT :
			if (!strcasecmp(o->o_str, "pass"))
				a->ac_result = IPMR_PASS;
			else if (!strcasecmp(o->o_str, "block"))
				a->ac_result = IPMR_BLOCK;
			else if (!strcasecmp(o->o_str, "nomatch"))
				a->ac_result = IPMR_NOMATCH;
			else if (!strcasecmp(o->o_str, "log"))
				a->ac_result = IPMR_LOG;
			break;
		case IPM_SECOND :
			a->ac_second = o->o_num;
			break;
		case IPM_SRCIP :
			a->ac_sip = o->o_ip.s_addr;
			a->ac_smsk = htonl(0xffffffff << (32 - o->o_num));
			break;
		case IPM_SRCPORT :
			a->ac_sport = htons(o->o_num);
			break;
		case IPM_SAVE :
			if (a->ac_savefile != NULL) {
				fprintf(stderr, "%s redfined on line %d\n",
					ipmon_yykeytostr(o->o_type), ipmon_yylineNum);
				break;
			}
			a->ac_savefile = strdup(o->o_str);
			a->ac_savefp = fopen(o->o_str, "a");
			a->ac_dflag |= o->o_num & IPMDO_SAVERAW;
			break;
		case IPM_SYSLOG :
			if (a->ac_syslog != 0) {
				fprintf(stderr, "%s redfined on line %d\n",
					ipmon_yykeytostr(o->o_type), ipmon_yylineNum);
				break;
			}
			a->ac_syslog = 1;
			break;
		case IPM_TYPE :
			a->ac_type = o->o_num;
			break;
		case IPM_WITH :
			break;
		default :
			break;
		}

		olist = o->o_next;
		if (o->o_str != NULL)
			free(o->o_str);
		free(o);
	}
	a->ac_next = alist;
	alist = a;
}


int check_action(buf, log, opts, lvl)
char *buf, *log;
int opts, lvl;
{
	ipmon_action_t *a;
	struct timeval tv;
	ipflog_t *ipf;
	tcphdr_t *tcp;
	iplog_t *ipl;
	int matched;
	u_long t1;
	ip_t *ip;

	matched = 0;
	ipl = (iplog_t *)buf;
	ipf = (ipflog_t *)(ipl +1);
	ip = (ip_t *)(ipf + 1);
	tcp = (tcphdr_t *)((char *)ip + (IP_HL(ip) << 2));

	for (a = alist; a != NULL; a = a->ac_next) {
		if ((a->ac_mflag & IPMAC_DIRECTION) != 0) {
			if (a->ac_direction == IPM_IN) {
				if ((ipf->fl_flags & FR_INQUE) == 0)
					continue;
			} else if (a->ac_direction == IPM_OUT) {
				if ((ipf->fl_flags & FR_OUTQUE) == 0)
					continue;
			}
		}

		if ((a->ac_type != 0) && (a->ac_type != ipl->ipl_magic))
			continue;

		if ((a->ac_mflag & IPMAC_EVERY) != 0) {
			gettimeofday(&tv, NULL);
			t1 = tv.tv_sec - a->ac_lastsec;
			if (tv.tv_usec <= a->ac_lastusec)
				t1--;
			if (a->ac_second != 0) {
				if (t1 < a->ac_second)
					continue;
				a->ac_lastsec = tv.tv_sec;
				a->ac_lastusec = tv.tv_usec;
			}

			if (a->ac_packet != 0) {
				if (a->ac_pktcnt == 0)
					a->ac_pktcnt++;
				else if (a->ac_pktcnt == a->ac_packet) {
					a->ac_pktcnt = 0;
					continue;
				} else {
					a->ac_pktcnt++;
					continue;
				}
			}
		}

		if ((a->ac_mflag & IPMAC_DSTIP) != 0) {
			if ((ip->ip_dst.s_addr & a->ac_dmsk) != a->ac_dip)
				continue;
		}

		if ((a->ac_mflag & IPMAC_DSTPORT) != 0) {
			if (ip->ip_p != IPPROTO_UDP && ip->ip_p != IPPROTO_TCP)
				continue;
			if (tcp->th_dport != a->ac_dport)
				continue;
		}

		if ((a->ac_mflag & IPMAC_GROUP) != 0) {
			if (strncmp(a->ac_group, ipf->fl_group,
				    FR_GROUPLEN) != 0)
				continue;
		}

		if ((a->ac_mflag & IPMAC_INTERFACE) != 0) {
			if (strcmp(a->ac_iface, ipf->fl_ifname))
				continue;
		}

		if ((a->ac_mflag & IPMAC_PROTOCOL) != 0) {
			if (a->ac_proto != ip->ip_p)
				continue;
		}

		if ((a->ac_mflag & IPMAC_RESULT) != 0) {
			if ((ipf->fl_flags & FF_LOGNOMATCH) != 0) {
				if (a->ac_result != IPMR_NOMATCH)
					continue;
			} else if (FR_ISPASS(ipf->fl_flags)) {
				if (a->ac_result != IPMR_PASS)
					continue;
			} else if (FR_ISBLOCK(ipf->fl_flags)) {
				if (a->ac_result != IPMR_BLOCK)
					continue;
			} else {	/* Log only */
				if (a->ac_result != IPMR_LOG)
					continue;
			}
		}

		if ((a->ac_mflag & IPMAC_RULE) != 0) {
			if (a->ac_rule != ipf->fl_rule)
				continue;
		}

		if ((a->ac_mflag & IPMAC_SRCIP) != 0) {
			if ((ip->ip_src.s_addr & a->ac_smsk) != a->ac_sip)
				continue;
		}

		if ((a->ac_mflag & IPMAC_SRCPORT) != 0) {
			if (ip->ip_p != IPPROTO_UDP && ip->ip_p != IPPROTO_TCP)
				continue;
			if (tcp->th_sport != a->ac_sport)
				continue;
		}

		if ((a->ac_mflag & IPMAC_LOGTAG) != 0) {
			if (a->ac_logtag != ipf->fl_logtag)
				continue;
		}

		if ((a->ac_mflag & IPMAC_NATTAG) != 0) {
			if (strncmp(a->ac_nattag, ipf->fl_nattag.ipt_tag,
				    IPFTAG_LEN) != 0)
				continue;
		}

		matched = 1;

		/*
		 * It matched so now execute the command
		 */
		if (a->ac_syslog != 0) {
			syslog(lvl, "%s", log);
		}

		if (a->ac_savefp != NULL) {
			if (a->ac_dflag & IPMDO_SAVERAW)
				fwrite(ipl, 1, ipl->ipl_dsize, a->ac_savefp);
			else
				fputs(log, a->ac_savefp);
		}

		if (a->ac_exec != NULL) {
			switch (fork())
			{
			case 0 :
			{
				FILE *pi;

				pi = popen(a->ac_run, "w");
				if (pi != NULL) {
					fprintf(pi, "%s\n", log);
					if ((opts & OPT_HEXHDR) != 0) {
						dumphex(pi, 0, buf,
							sizeof(*ipl) +
							sizeof(*ipf));
					}
					if ((opts & OPT_HEXBODY) != 0) {
						dumphex(pi, 0, (char *)ip,
							ipf->fl_hlen +
							ipf->fl_plen);
					}
					pclose(pi);
				}
				exit(1);
			}
			case -1 :
				break;
			default :
				break;
			}
		}
	}

	return matched;
}


static void free_action(a)
ipmon_action_t *a;
{
	if (a->ac_savefile != NULL) {
		free(a->ac_savefile);
		a->ac_savefile = NULL;
	}
	if (a->ac_savefp != NULL) {
		fclose(a->ac_savefp);
		a->ac_savefp = NULL;
	}
	if (a->ac_exec != NULL) {
		free(a->ac_exec);
		if (a->ac_run == a->ac_exec)
			a->ac_run = NULL;
		a->ac_exec = NULL;
	}
	if (a->ac_run != NULL) {
		free(a->ac_run);
		a->ac_run = NULL;
	}
	if (a->ac_iface != NULL) {
		free(a->ac_iface);
		a->ac_iface = NULL;
	}
	a->ac_next = NULL;
	free(a);
}


int load_config(file)
char *file;
{
	ipmon_action_t *a;
	FILE *fp;
	char *s;

	s = getenv("YYDEBUG");
	if (s != NULL)
		ipmon_yydebug = atoi(s);
	else
		ipmon_yydebug = 0;

	while ((a = alist) != NULL) {
		alist = a->ac_next;
		free_action(a);
	}

	ipmon_yylineNum = 1;

	(void) ipmon_yysettab(ipmon_yywords);

	fp = fopen(file, "r");
	if (!fp) {
		perror("load_config:fopen:");
		return -1;
	}
	ipmon_yyin = fp;
	while (!feof(fp))
		ipmon_yyparse();
	fclose(fp);
	return 0;
}
#line 784 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int ipmon_yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = ipmon_yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = ipmon_yyssp - ipmon_yyss;
    newss = (ipmon_yyss != 0)
          ? (short *)realloc(ipmon_yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    ipmon_yyss  = newss;
    ipmon_yyssp = newss + i;
    newvs = (ipmon_yyvs != 0)
          ? (YYSTYPE *)realloc(ipmon_yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    ipmon_yyvs = newvs;
    ipmon_yyvsp = newvs + i;
    ipmon_yystacksize = newsize;
    ipmon_yysslim = ipmon_yyss + newsize - 1;
    return 0;
}

#define YYABORT goto ipmon_yyabort
#define YYREJECT goto ipmon_yyabort
#define YYACCEPT goto ipmon_yyaccept
#define YYERROR goto ipmon_yyerrlab
int
ipmon_yyparse(void)
{
    register int ipmon_yym, ipmon_yyn, ipmon_yystate;
#if YYDEBUG
    register const char *ipmon_yys;

    if ((ipmon_yys = getenv("YYDEBUG")) != 0)
    {
        ipmon_yyn = *ipmon_yys;
        if (ipmon_yyn >= '0' && ipmon_yyn <= '9')
            ipmon_yydebug = ipmon_yyn - '0';
    }
#endif

    ipmon_yynerrs = 0;
    ipmon_yyerrflag = 0;
    ipmon_yychar = YYEMPTY;

    if (ipmon_yyss == NULL && ipmon_yygrowstack()) goto ipmon_yyoverflow;
    ipmon_yyssp = ipmon_yyss;
    ipmon_yyvsp = ipmon_yyvs;
    *ipmon_yyssp = ipmon_yystate = 0;

ipmon_yyloop:
    if ((ipmon_yyn = ipmon_yydefred[ipmon_yystate]) != 0) goto ipmon_yyreduce;
    if (ipmon_yychar < 0)
    {
        if ((ipmon_yychar = ipmon_yylex()) < 0) ipmon_yychar = 0;
#if YYDEBUG
        if (ipmon_yydebug)
        {
            ipmon_yys = 0;
            if (ipmon_yychar <= YYMAXTOKEN) ipmon_yys = ipmon_yyname[ipmon_yychar];
            if (!ipmon_yys) ipmon_yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, ipmon_yystate, ipmon_yychar, ipmon_yys);
        }
#endif
    }
    if ((ipmon_yyn = ipmon_yysindex[ipmon_yystate]) && (ipmon_yyn += ipmon_yychar) >= 0 &&
            ipmon_yyn <= YYTABLESIZE && ipmon_yycheck[ipmon_yyn] == ipmon_yychar)
    {
#if YYDEBUG
        if (ipmon_yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, ipmon_yystate, ipmon_yytable[ipmon_yyn]);
#endif
        if (ipmon_yyssp >= ipmon_yysslim && ipmon_yygrowstack())
        {
            goto ipmon_yyoverflow;
        }
        *++ipmon_yyssp = ipmon_yystate = ipmon_yytable[ipmon_yyn];
        *++ipmon_yyvsp = ipmon_yylval;
        ipmon_yychar = YYEMPTY;
        if (ipmon_yyerrflag > 0)  --ipmon_yyerrflag;
        goto ipmon_yyloop;
    }
    if ((ipmon_yyn = ipmon_yyrindex[ipmon_yystate]) && (ipmon_yyn += ipmon_yychar) >= 0 &&
            ipmon_yyn <= YYTABLESIZE && ipmon_yycheck[ipmon_yyn] == ipmon_yychar)
    {
        ipmon_yyn = ipmon_yytable[ipmon_yyn];
        goto ipmon_yyreduce;
    }
    if (ipmon_yyerrflag) goto ipmon_yyinrecovery;

    ipmon_yyerror("syntax error");

#ifdef lint
    goto ipmon_yyerrlab;
#endif

ipmon_yyerrlab:
    ++ipmon_yynerrs;

ipmon_yyinrecovery:
    if (ipmon_yyerrflag < 3)
    {
        ipmon_yyerrflag = 3;
        for (;;)
        {
            if ((ipmon_yyn = ipmon_yysindex[*ipmon_yyssp]) && (ipmon_yyn += YYERRCODE) >= 0 &&
                    ipmon_yyn <= YYTABLESIZE && ipmon_yycheck[ipmon_yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (ipmon_yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *ipmon_yyssp, ipmon_yytable[ipmon_yyn]);
#endif
                if (ipmon_yyssp >= ipmon_yysslim && ipmon_yygrowstack())
                {
                    goto ipmon_yyoverflow;
                }
                *++ipmon_yyssp = ipmon_yystate = ipmon_yytable[ipmon_yyn];
                *++ipmon_yyvsp = ipmon_yylval;
                goto ipmon_yyloop;
            }
            else
            {
#if YYDEBUG
                if (ipmon_yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *ipmon_yyssp);
#endif
                if (ipmon_yyssp <= ipmon_yyss) goto ipmon_yyabort;
                --ipmon_yyssp;
                --ipmon_yyvsp;
            }
        }
    }
    else
    {
        if (ipmon_yychar == 0) goto ipmon_yyabort;
#if YYDEBUG
        if (ipmon_yydebug)
        {
            ipmon_yys = 0;
            if (ipmon_yychar <= YYMAXTOKEN) ipmon_yys = ipmon_yyname[ipmon_yychar];
            if (!ipmon_yys) ipmon_yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, ipmon_yystate, ipmon_yychar, ipmon_yys);
        }
#endif
        ipmon_yychar = YYEMPTY;
        goto ipmon_yyloop;
    }

ipmon_yyreduce:
#if YYDEBUG
    if (ipmon_yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, ipmon_yystate, ipmon_yyn, ipmon_yyrule[ipmon_yyn]);
#endif
    ipmon_yym = ipmon_yylen[ipmon_yyn];
    ipmon_yyval = ipmon_yyvsp[1-ipmon_yym];
    switch (ipmon_yyn)
    {
case 5:
#line 74 "../tools/ipmon_y.y"
{ build_action(ipmon_yyvsp[-6].opt); resetlexer(); }
break;
case 8:
#line 79 "../tools/ipmon_y.y"
{ set_variable(ipmon_yyvsp[-3].str, ipmon_yyvsp[-1].str);
						  resetlexer();
						  free(ipmon_yyvsp[-3].str);
						  free(ipmon_yyvsp[-1].str);
						  ipmon_yyvarnext = 0;
						}
break;
case 9:
#line 88 "../tools/ipmon_y.y"
{ ipmon_yyvarnext = 1; }
break;
case 10:
#line 92 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 11:
#line 93 "../tools/ipmon_y.y"
{ ipmon_yyvsp[-2].opt->o_next = ipmon_yyvsp[0].opt; ipmon_yyval.opt = ipmon_yyvsp[-2].opt; }
break;
case 12:
#line 97 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 13:
#line 98 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 14:
#line 99 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 15:
#line 100 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 16:
#line 101 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 17:
#line 102 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 18:
#line 103 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 19:
#line 104 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 20:
#line 105 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 21:
#line 106 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 22:
#line 107 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 23:
#line 108 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 24:
#line 109 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 25:
#line 110 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 26:
#line 114 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 27:
#line 115 "../tools/ipmon_y.y"
{ ipmon_yyvsp[-2].opt->o_next = ipmon_yyvsp[0].opt; ipmon_yyval.opt = ipmon_yyvsp[-2].opt; }
break;
case 28:
#line 119 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 29:
#line 120 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 30:
#line 121 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 31:
#line 122 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = ipmon_yyvsp[0].opt; }
break;
case 32:
#line 126 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_DIRECTION);
						  ipmon_yyval.opt->o_num = IPM_IN; }
break;
case 33:
#line 128 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_DIRECTION);
						  ipmon_yyval.opt->o_num = IPM_OUT; }
break;
case 34:
#line 132 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_DSTIP);
						  ipmon_yyval.opt->o_ip = ipmon_yyvsp[-2].addr;
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 35:
#line 138 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_DSTPORT);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 36:
#line 140 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_DSTPORT);
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 37:
#line 144 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_SECOND);
						  ipmon_yyval.opt->o_num = 1; }
break;
case 38:
#line 146 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_SECOND);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[-1].num; }
break;
case 39:
#line 148 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_PACKET);
						  ipmon_yyval.opt->o_num = 1; }
break;
case 40:
#line 150 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_PACKET);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[-1].num; }
break;
case 41:
#line 154 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_GROUP);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 42:
#line 156 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_GROUP);
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 43:
#line 161 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_INTERFACE);
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 44:
#line 165 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_LOGTAG);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 45:
#line 169 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_NATTAG);
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 46:
#line 174 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_PROTOCOL);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 47:
#line 176 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_PROTOCOL);
						  ipmon_yyval.opt->o_num = getproto(ipmon_yyvsp[0].str);
						  free(ipmon_yyvsp[0].str);
						}
break;
case 48:
#line 182 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_RESULT);
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 49:
#line 186 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_RULE);
						  ipmon_yyval.opt->o_num = YY_NUMBER; }
break;
case 50:
#line 190 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_SRCIP);
						  ipmon_yyval.opt->o_ip = ipmon_yyvsp[-2].addr;
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 51:
#line 196 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_SRCPORT);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 52:
#line 198 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_SRCPORT);
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 53:
#line 202 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_TYPE);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[0].num; }
break;
case 54:
#line 207 "../tools/ipmon_y.y"
{ ipmon_yyval.num = IPL_MAGIC; }
break;
case 55:
#line 208 "../tools/ipmon_y.y"
{ ipmon_yyval.num = IPL_MAGIC_NAT; }
break;
case 56:
#line 209 "../tools/ipmon_y.y"
{ ipmon_yyval.num = IPL_MAGIC_STATE; }
break;
case 57:
#line 213 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_EXECUTE);
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 58:
#line 217 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_SAVE);
						  ipmon_yyval.opt->o_num = ipmon_yyvsp[-1].num;
						  ipmon_yyval.opt->o_str = ipmon_yyvsp[0].str; }
break;
case 59:
#line 222 "../tools/ipmon_y.y"
{ ipmon_yyval.num = 0; }
break;
case 60:
#line 223 "../tools/ipmon_y.y"
{ ipmon_yyval.num = ipmon_yyvsp[0].num; }
break;
case 61:
#line 224 "../tools/ipmon_y.y"
{ ipmon_yyval.num = ipmon_yyvsp[-2].num | ipmon_yyvsp[0].num; }
break;
case 62:
#line 228 "../tools/ipmon_y.y"
{ ipmon_yyval.num = IPMDO_SAVERAW; }
break;
case 63:
#line 231 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = new_opt(IPM_SYSLOG); }
break;
case 64:
#line 235 "../tools/ipmon_y.y"
{ ipmon_yyval.opt = 0; }
break;
case 65:
#line 239 "../tools/ipmon_y.y"
{ if (ipmon_yyvsp[-6].num > 255 || ipmon_yyvsp[-4].num > 255 || ipmon_yyvsp[-2].num > 255 || ipmon_yyvsp[0].num > 255) {
			ipmon_yyerror("Invalid octet string for IP address");
			return 0;
		  }
		  ipmon_yyval.addr.s_addr = (ipmon_yyvsp[-6].num << 24) | (ipmon_yyvsp[-4].num << 16) | (ipmon_yyvsp[-2].num << 8) | ipmon_yyvsp[0].num;
		  ipmon_yyval.addr.s_addr = htonl(ipmon_yyval.addr.s_addr);
		}
break;
#line 1238 "y.tab.c"
    }
    ipmon_yyssp -= ipmon_yym;
    ipmon_yystate = *ipmon_yyssp;
    ipmon_yyvsp -= ipmon_yym;
    ipmon_yym = ipmon_yylhs[ipmon_yyn];
    if (ipmon_yystate == 0 && ipmon_yym == 0)
    {
#if YYDEBUG
        if (ipmon_yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        ipmon_yystate = YYFINAL;
        *++ipmon_yyssp = YYFINAL;
        *++ipmon_yyvsp = ipmon_yyval;
        if (ipmon_yychar < 0)
        {
            if ((ipmon_yychar = ipmon_yylex()) < 0) ipmon_yychar = 0;
#if YYDEBUG
            if (ipmon_yydebug)
            {
                ipmon_yys = 0;
                if (ipmon_yychar <= YYMAXTOKEN) ipmon_yys = ipmon_yyname[ipmon_yychar];
                if (!ipmon_yys) ipmon_yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, ipmon_yychar, ipmon_yys);
            }
#endif
        }
        if (ipmon_yychar == 0) goto ipmon_yyaccept;
        goto ipmon_yyloop;
    }
    if ((ipmon_yyn = ipmon_yygindex[ipmon_yym]) && (ipmon_yyn += ipmon_yystate) >= 0 &&
            ipmon_yyn <= YYTABLESIZE && ipmon_yycheck[ipmon_yyn] == ipmon_yystate)
        ipmon_yystate = ipmon_yytable[ipmon_yyn];
    else
        ipmon_yystate = ipmon_yydgoto[ipmon_yym];
#if YYDEBUG
    if (ipmon_yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *ipmon_yyssp, ipmon_yystate);
#endif
    if (ipmon_yyssp >= ipmon_yysslim && ipmon_yygrowstack())
    {
        goto ipmon_yyoverflow;
    }
    *++ipmon_yyssp = ipmon_yystate;
    *++ipmon_yyvsp = ipmon_yyval;
    goto ipmon_yyloop;

ipmon_yyoverflow:
    ipmon_yyerror("yacc stack overflow");

ipmon_yyabort:
    return (1);

ipmon_yyaccept:
    return (0);
}
