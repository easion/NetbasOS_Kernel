#ifndef lint
static const char ipscan_yysccsid[] = "@(#)yaccpar	1.9 (Berkeley) 02/21/93";
#endif

#include <stdlib.h>

#define YYBYACC 1
#define YYMAJOR 1
#define YYMINOR 9
#define YYPATCH 20050813

#define YYEMPTY (-1)
#define ipscan_yyclearin    (ipscan_yychar = YYEMPTY)
#define ipscan_yyerrok      (ipscan_yyerrflag = 0)
#define YYRECOVERING (ipscan_yyerrflag != 0)

extern int ipscan_yyparse(void);

static int ipscan_yygrowstack(void);
#define YYPREFIX "ipscan_yy"
#line 7 "../tools/ipscan_y.y"
#include <sys/types.h>
#include <sys/ioctl.h>
#include "ipf.h"
#include "opts.h"
#include "kmem.h"
#include "ipscan_l.h"
#include "netinet/ip_scan.h"

#define	YYDEBUG	1

extern	char	*optarg;
extern	void	ipscan_yyerror __P((char *));
extern	int	ipscan_yyparse __P((void));
extern	int	ipscan_yylex __P((void));
extern	int	ipscan_yydebug;
extern	FILE	*ipscan_yyin;
extern	int	ipscan_yylineNum;
extern	void	printbuf __P((char *, int, int));


void		printent __P((ipscan_t *));
void		showlist __P((void));
int		getportnum __P((char *));
struct in_addr	gethostip __P((char *));
struct in_addr	combine __P((int, int, int, int));
char		**makepair __P((char *, char *));
void		addtag __P((char *, char **, char **, struct action *));
int		cram __P((char *, char *));
void		usage __P((char *));
int		main __P((int, char **));

int		opts = 0;
int		fd = -1;


#line 44 "../tools/ipscan_y.y"
typedef union	{
	char	*str;
	char	**astr;
	u_32_t	num;
	struct	in_addr	ipa;
	struct	action	act;
	union	i6addr	ip6;
} YYSTYPE;
#line 67 "y.tab.c"
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
#define IPSL_START 270
#define IPSL_STARTGROUP 271
#define IPSL_CONTENT 272
#define IPSL_CLOSE 273
#define IPSL_TRACK 274
#define IPSL_EOF 275
#define IPSL_REDIRECT 276
#define IPSL_ELSE 277
#define YYERRCODE 256
short ipscan_yylhs[] = {                                        -1,
    0,    0,    0,    0,    0,   10,   10,   10,   12,   12,
   12,   13,   14,   14,   14,   11,   19,   15,   16,   17,
   18,   18,    1,    7,    7,    2,    2,    4,    4,    4,
    8,    9,    3,    3,    5,    5,    6,    6,
};
short ipscan_yylen[] = {                                         2,
    2,    2,    3,    3,    1,    2,    2,    2,    1,    1,
    1,    5,    1,    1,    1,    3,    1,    5,    8,    7,
    1,    3,    1,    1,    1,    1,    3,    1,    1,    1,
    3,    5,    4,    6,    7,    1,    1,    1,
};
short ipscan_yydefred[] = {                                      0,
    0,    5,    0,    0,    0,    0,    0,    0,   17,    0,
   23,    0,    6,    9,   10,   11,    0,    7,    8,   13,
   14,   15,    0,    0,    1,    2,   16,    0,    0,    3,
    4,    0,    0,   24,   25,   21,    0,    0,    0,    0,
    0,    0,    0,   31,    0,    0,   28,   29,    0,   18,
   30,    0,    0,    0,   12,   22,    0,    0,    0,    0,
    0,   32,    0,    0,   36,    0,   27,   20,   19,    0,
   33,    0,    0,   37,   38,    0,    0,   34,    0,    0,
   35,
};
short ipscan_yydgoto[] = {                                       6,
   12,   50,   51,   52,   66,   76,   33,   34,   35,    7,
    8,   13,   18,   19,   14,   15,   16,   37,   10,
};
short ipscan_yysindex[] = {                                   -256,
  -42,    0, -228, -227, -228, -246,  -26,  -25,    0, -224,
    0,  -22,    0,    0,    0,    0,  -21,    0,    0,    0,
    0,    0,  -20,  -19,    0,    0,    0,   -2, -218,    0,
    0,  -41,  -43,    0,    0,    0,  -39,  -32,   -1, -266,
    2, -266, -215,    0, -214,    2,    0,    0,    6,    0,
    0, -226, -212,  -13,    0,    0,    8,  -11, -230, -266,
 -266,    0, -266,    7,    0,  -24,    0,    0,    0, -205,
    0, -229,    9,    0,    0,   13, -201,    0,   11, -199,
    0,
};
short ipscan_yyrindex[] = {                                      0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,  -25,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,
};
short ipscan_yygindex[] = {                                      0,
    0,  -40,    0,    1,    0,    0,  -35,    0,    0,   53,
   54,    0,    0,    0,   57,   58,   59,    0,    0,
};
#define YYTABLESIZE 218
short ipscan_yytable[] = {                                      39,
   41,   55,    1,    2,   43,   54,   47,   48,   44,   49,
   58,   45,    1,    3,    4,    5,   71,   40,    9,   72,
   68,   42,   69,    3,    4,    5,   64,   74,   65,   75,
   11,   17,   25,   26,   27,   28,   29,   32,   30,   31,
   36,   53,   46,   56,   57,   59,   38,   61,   62,   63,
   60,   73,   70,   78,   77,   79,   80,   81,   23,   24,
   67,   20,   21,   22,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,   38,
};
short ipscan_yycheck[] = {                                      41,
   44,   42,  259,  260,   44,   41,  273,  274,   41,  276,
   46,   44,  259,  270,  271,  272,   41,   61,   61,   44,
   61,   61,   63,  270,  271,  272,  257,  257,  259,  259,
  259,  259,   59,   59,  259,   58,   58,   40,   59,   59,
  259,   40,   44,  259,  259,   40,  259,   61,   41,   61,
  277,  257,   46,   41,   46,  257,   46,  257,    6,    6,
   60,    5,    5,    5,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
   -1,   -1,   -1,   -1,   -1,   -1,   -1,  259,
};
#define YYFINAL 6
#ifndef YYDEBUG
#define YYDEBUG 0
#endif
#define YYMAXTOKEN 277
#if YYDEBUG
char *ipscan_yyname[] = {
"end-of-file",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,"'('","')'",0,0,"','",0,"'.'",0,0,0,0,0,0,0,0,0,0,0,"':'","';'",0,
"'='",0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
"YY_NUMBER","YY_HEX","YY_STR","YY_COMMENT","YY_CMP_EQ","YY_CMP_NE","YY_CMP_LE",
"YY_CMP_GE","YY_CMP_LT","YY_CMP_GT","YY_RANGE_OUT","YY_RANGE_IN","YY_IPV6",
"IPSL_START","IPSL_STARTGROUP","IPSL_CONTENT","IPSL_CLOSE","IPSL_TRACK",
"IPSL_EOF","IPSL_REDIRECT","IPSL_ELSE",
};
char *ipscan_yyrule[] = {
"$accept : file",
"file : line ';'",
"file : assign ';'",
"file : file line ';'",
"file : file assign ';'",
"file : YY_COMMENT",
"line : IPSL_START dline",
"line : IPSL_STARTGROUP gline",
"line : IPSL_CONTENT oline",
"dline : cline",
"dline : sline",
"dline : csline",
"gline : YY_STR ':' glist '=' action",
"oline : cline",
"oline : sline",
"oline : csline",
"assign : YY_STR assigning YY_STR",
"assigning : '='",
"cline : tag ':' matchup '=' action",
"sline : tag ':' '(' ')' ',' matchup '=' action",
"csline : tag ':' matchup ',' matchup '=' action",
"glist : YY_STR",
"glist : glist ',' YY_STR",
"tag : YY_STR",
"matchup : onehalf",
"matchup : twohalves",
"action : result",
"action : result IPSL_ELSE result",
"result : IPSL_CLOSE",
"result : IPSL_TRACK",
"result : redirect",
"onehalf : '(' YY_STR ')'",
"twohalves : '(' YY_STR ',' YY_STR ')'",
"redirect : IPSL_REDIRECT '(' ipaddr ')'",
"redirect : IPSL_REDIRECT '(' ipaddr ',' portnum ')'",
"ipaddr : YY_NUMBER '.' YY_NUMBER '.' YY_NUMBER '.' YY_NUMBER",
"ipaddr : YY_STR",
"portnum : YY_NUMBER",
"portnum : YY_STR",
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

int      ipscan_yydebug;
int      ipscan_yynerrs;
int      ipscan_yyerrflag;
int      ipscan_yychar;
short   *ipscan_yyssp;
YYSTYPE *ipscan_yyvsp;
YYSTYPE  ipscan_yyval;
YYSTYPE  ipscan_yylval;

/* variables for the parser stack */
static short   *ipscan_yyss;
static short   *ipscan_yysslim;
static YYSTYPE *ipscan_yyvs;
static int      ipscan_yystacksize;
#line 183 "../tools/ipscan_y.y"


static	struct	wordtab	ipscan_yywords[] = {
	{ "close",		IPSL_CLOSE },
	{ "content",		IPSL_CONTENT },
	{ "else",		IPSL_ELSE },
	{ "start-group",	IPSL_STARTGROUP },
	{ "redirect",		IPSL_REDIRECT },
	{ "start",		IPSL_START },
	{ "track",		IPSL_TRACK },
	{ NULL,		0 }
};


int cram(dst, src)
char *dst;
char *src;
{
	char c, *s, *t, *u;
	int i, j, k;

	c = *src;
	s = src + 1;
	t = strchr(s, c);
	*t = '\0';
	for (u = dst, i = 0; (i <= ISC_TLEN) && (s < t); ) {
		c = *s++;
		if (c == '\\') {
			if (s >= t)
				break;
			j = k = 0;
			do {
				c = *s++;
				if (j && (!ISDIGIT(c) || (c > '7') ||
				     (k >= 248))) {
					*u++ = k, i++;
					j = k = 0;
					s--;
					break;
				}
				i++;

				if (ISALPHA(c) || (c > '7')) {
					switch (c)
					{
					case 'n' :
						*u++ = '\n';
						break;
					case 'r' :
						*u++ = '\r';
						break;
					case 't' :
						*u++ = '\t';
						break;
					default :
						*u++ = c;
						break;
					}
				} else if (ISDIGIT(c)) {
					j = 1;
					k <<= 3;
					k |= (c - '0');
					i--;
				} else
						*u++ = c;
			} while ((i <= ISC_TLEN) && (s <= t) && (j > 0));
		} else
			*u++ = c, i++;
	}
	return i;
}


void printent(isc)
ipscan_t *isc;
{
	char buf[ISC_TLEN+1];
	u_char *u;
	int i, j;

	buf[ISC_TLEN] = '\0';
	bcopy(isc->ipsc_ctxt, buf, ISC_TLEN);
	printf("%s : (\"", isc->ipsc_tag);
	printbuf(isc->ipsc_ctxt, isc->ipsc_clen, 0);

	bcopy(isc->ipsc_cmsk, buf, ISC_TLEN);
	printf("\", \"%s\"), (\"", buf);

	printbuf(isc->ipsc_stxt, isc->ipsc_slen, 0);

	bcopy(isc->ipsc_smsk, buf, ISC_TLEN);
	printf("\", \"%s\") = ", buf);

	switch (isc->ipsc_action)
	{
	case ISC_A_TRACK :
		printf("track");
		break;
	case ISC_A_REDIRECT :
		printf("redirect");
		printf("(%s", inet_ntoa(isc->ipsc_ip));
		if (isc->ipsc_port)
			printf(",%d", isc->ipsc_port);
		printf(")");
		break;
	case ISC_A_CLOSE :
		printf("close");
		break;
	default :
		break;
	}

	if (isc->ipsc_else != ISC_A_NONE) {
		printf(" else ");
		switch (isc->ipsc_else)
		{
		case ISC_A_TRACK :
			printf("track");
			break;
		case ISC_A_REDIRECT :
			printf("redirect");
			printf("(%s", inet_ntoa(isc->ipsc_eip));
			if (isc->ipsc_eport)
				printf(",%d", isc->ipsc_eport);
			printf(")");
			break;
		case ISC_A_CLOSE :
			printf("close");
			break;
		default :
			break;
		}
	}
	printf("\n");

	if (opts & OPT_DEBUG) {
		for (u = (u_char *)isc, i = sizeof(*isc); i; ) {
			printf("#");
			for (j = 32; (j > 0) && (i > 0); j--, i--)
				printf("%s%02x", (j & 7) ? "" : " ", *u++);
			printf("\n");
		}
	}
	if (opts & OPT_VERBOSE) {
		printf("# hits %d active %d fref %d sref %d\n",
			isc->ipsc_hits, isc->ipsc_active, isc->ipsc_fref,
			isc->ipsc_sref);
	}
}


void addtag(tstr, cp, sp, act)
char *tstr;
char **cp, **sp;
struct action *act;
{
	ipscan_t isc, *iscp;

	bzero((char *)&isc, sizeof(isc));

	strncpy(isc.ipsc_tag, tstr, sizeof(isc.ipsc_tag));
	isc.ipsc_tag[sizeof(isc.ipsc_tag) - 1] = '\0';

	if (cp) {
		isc.ipsc_clen = cram(isc.ipsc_ctxt, cp[0]);
		if (cp[1]) {
			if (cram(isc.ipsc_cmsk, cp[1]) != isc.ipsc_clen) {
				fprintf(stderr,
					"client text/mask strings different length\n");
				return;
			}
		}
	}

	if (sp) {
		isc.ipsc_slen = cram(isc.ipsc_stxt, sp[0]);
		if (sp[1]) {
			if (cram(isc.ipsc_smsk, sp[1]) != isc.ipsc_slen) {
				fprintf(stderr,
					"server text/mask strings different length\n");
				return;
			}
		}
	}

	if (act->act_val == IPSL_CLOSE) {
		isc.ipsc_action = ISC_A_CLOSE;
	} else if (act->act_val == IPSL_TRACK) {
		isc.ipsc_action = ISC_A_TRACK;
	} else if (act->act_val == IPSL_REDIRECT) {
		isc.ipsc_action = ISC_A_REDIRECT;
		isc.ipsc_ip = act->act_ip;
		isc.ipsc_port = act->act_port;
		fprintf(stderr, "%d: redirect unsupported\n", ipscan_yylineNum + 1);
	}

	if (act->act_else == IPSL_CLOSE) {
		isc.ipsc_else = ISC_A_CLOSE;
	} else if (act->act_else == IPSL_TRACK) {
		isc.ipsc_else = ISC_A_TRACK;
	} else if (act->act_else == IPSL_REDIRECT) {
		isc.ipsc_else = ISC_A_REDIRECT;
		isc.ipsc_eip = act->act_eip;
		isc.ipsc_eport = act->act_eport;
		fprintf(stderr, "%d: redirect unsupported\n", ipscan_yylineNum + 1);
	}

	if (!(opts & OPT_DONOTHING)) {
		iscp = &isc;
		if (opts & OPT_REMOVE) {
			if (ioctl(fd, SIOCRMSCA, &iscp) == -1)
				perror("SIOCADSCA");
		} else {
			if (ioctl(fd, SIOCADSCA, &iscp) == -1)
				perror("SIOCADSCA");
		}
	}

	if (opts & OPT_VERBOSE)
		printent(&isc);
}


char **makepair(s1, s2)
char *s1, *s2;
{
	char **a;

	a = malloc(sizeof(char *) * 2);
	a[0] = s1;
	a[1] = s2;
	return a;
}


struct in_addr combine(a1, a2, a3, a4)
int a1, a2, a3, a4;
{
	struct in_addr in;

	a1 &= 0xff;
	in.s_addr = a1 << 24;
	a2 &= 0xff;
	in.s_addr |= (a2 << 16);
	a3 &= 0xff;
	in.s_addr |= (a3 << 8);
	a4 &= 0xff;
	in.s_addr |= a4;
	in.s_addr = htonl(in.s_addr);
	return in;
}


struct in_addr gethostip(host)
char *host;
{
	struct hostent *hp;
	struct in_addr in;

	in.s_addr = 0;

	hp = gethostbyname(host);
	if (!hp)
		return in;
	bcopy(hp->h_addr, (char *)&in, sizeof(in));
	return in;
}


int getportnum(port)
char *port;
{
	struct servent *s;

	s = getservbyname(port, "tcp");
	if (s == NULL)
		return -1;
	return s->s_port;
}


void showlist()
{
	ipscanstat_t ipsc, *ipscp = &ipsc;
	ipscan_t isc;

	if (ioctl(fd, SIOCGSCST, &ipscp) == -1)
		perror("ioctl(SIOCGSCST)");
	else if (opts & OPT_SHOWLIST) {
		while (ipsc.iscs_list != NULL) {
			if (kmemcpy((char *)&isc, (u_long)ipsc.iscs_list,
				    sizeof(isc)) == -1) {
				perror("kmemcpy");
				break;
			} else {
				printent(&isc);
				ipsc.iscs_list = isc.ipsc_next;
			}
		}
	} else {
		printf("scan entries loaded\t%d\n", ipsc.iscs_entries);
		printf("scan entries matches\t%ld\n", ipsc.iscs_acted);
		printf("negative matches\t%ld\n", ipsc.iscs_else);
	}
}


void usage(prog)
char *prog;
{
	fprintf(stderr, "Usage:\t%s [-dnrv] -f <filename>\n", prog);
	fprintf(stderr, "\t%s [-dlv]\n", prog);
	exit(1);
}


int main(argc, argv)
int argc;
char *argv[];
{
	FILE *fp = NULL;
	int c;

	(void) ipscan_yysettab(ipscan_yywords);

	if (argc < 2)
		usage(argv[0]);

	while ((c = getopt(argc, argv, "df:lnrsv")) != -1)
		switch (c)
		{
		case 'd' :
			opts |= OPT_DEBUG;
			ipscan_yydebug++;
			break;
		case 'f' :
			if (!strcmp(optarg, "-"))
				fp = stdin;
			else {
				fp = fopen(optarg, "r");
				if (!fp) {
					perror("open");
					exit(1);
				}
			}
			ipscan_yyin = fp;
			break;
		case 'l' :
			opts |= OPT_SHOWLIST;
			break;
		case 'n' :
			opts |= OPT_DONOTHING;
			break;
		case 'r' :
			opts |= OPT_REMOVE;
			break;
		case 's' :
			opts |= OPT_STAT;
			break;
		case 'v' :
			opts |= OPT_VERBOSE;
			break;
		}

	if (!(opts & OPT_DONOTHING)) {
		fd = open(IPL_SCAN, O_RDWR);
		if (fd == -1) {
			perror("open(IPL_SCAN)");
			exit(1);
		}
	}

	if (fp != NULL) {
		ipscan_yylineNum = 1;

		while (!feof(fp))
			ipscan_yyparse();
		fclose(fp);
		exit(0);
	}

	if (opts & (OPT_SHOWLIST|OPT_STAT)) {
		showlist();
		exit(0);
	}
	exit(1);
}
#line 674 "y.tab.c"
/* allocate initial stack or double stack size, up to YYMAXDEPTH */
static int ipscan_yygrowstack(void)
{
    int newsize, i;
    short *newss;
    YYSTYPE *newvs;

    if ((newsize = ipscan_yystacksize) == 0)
        newsize = YYINITSTACKSIZE;
    else if (newsize >= YYMAXDEPTH)
        return -1;
    else if ((newsize *= 2) > YYMAXDEPTH)
        newsize = YYMAXDEPTH;

    i = ipscan_yyssp - ipscan_yyss;
    newss = (ipscan_yyss != 0)
          ? (short *)realloc(ipscan_yyss, newsize * sizeof(*newss))
          : (short *)malloc(newsize * sizeof(*newss));
    if (newss == 0)
        return -1;

    ipscan_yyss  = newss;
    ipscan_yyssp = newss + i;
    newvs = (ipscan_yyvs != 0)
          ? (YYSTYPE *)realloc(ipscan_yyvs, newsize * sizeof(*newvs))
          : (YYSTYPE *)malloc(newsize * sizeof(*newvs));
    if (newvs == 0)
        return -1;

    ipscan_yyvs = newvs;
    ipscan_yyvsp = newvs + i;
    ipscan_yystacksize = newsize;
    ipscan_yysslim = ipscan_yyss + newsize - 1;
    return 0;
}

#define YYABORT goto ipscan_yyabort
#define YYREJECT goto ipscan_yyabort
#define YYACCEPT goto ipscan_yyaccept
#define YYERROR goto ipscan_yyerrlab
int
ipscan_yyparse(void)
{
    register int ipscan_yym, ipscan_yyn, ipscan_yystate;
#if YYDEBUG
    register const char *ipscan_yys;

    if ((ipscan_yys = getenv("YYDEBUG")) != 0)
    {
        ipscan_yyn = *ipscan_yys;
        if (ipscan_yyn >= '0' && ipscan_yyn <= '9')
            ipscan_yydebug = ipscan_yyn - '0';
    }
#endif

    ipscan_yynerrs = 0;
    ipscan_yyerrflag = 0;
    ipscan_yychar = YYEMPTY;

    if (ipscan_yyss == NULL && ipscan_yygrowstack()) goto ipscan_yyoverflow;
    ipscan_yyssp = ipscan_yyss;
    ipscan_yyvsp = ipscan_yyvs;
    *ipscan_yyssp = ipscan_yystate = 0;

ipscan_yyloop:
    if ((ipscan_yyn = ipscan_yydefred[ipscan_yystate]) != 0) goto ipscan_yyreduce;
    if (ipscan_yychar < 0)
    {
        if ((ipscan_yychar = ipscan_yylex()) < 0) ipscan_yychar = 0;
#if YYDEBUG
        if (ipscan_yydebug)
        {
            ipscan_yys = 0;
            if (ipscan_yychar <= YYMAXTOKEN) ipscan_yys = ipscan_yyname[ipscan_yychar];
            if (!ipscan_yys) ipscan_yys = "illegal-symbol";
            printf("%sdebug: state %d, reading %d (%s)\n",
                    YYPREFIX, ipscan_yystate, ipscan_yychar, ipscan_yys);
        }
#endif
    }
    if ((ipscan_yyn = ipscan_yysindex[ipscan_yystate]) && (ipscan_yyn += ipscan_yychar) >= 0 &&
            ipscan_yyn <= YYTABLESIZE && ipscan_yycheck[ipscan_yyn] == ipscan_yychar)
    {
#if YYDEBUG
        if (ipscan_yydebug)
            printf("%sdebug: state %d, shifting to state %d\n",
                    YYPREFIX, ipscan_yystate, ipscan_yytable[ipscan_yyn]);
#endif
        if (ipscan_yyssp >= ipscan_yysslim && ipscan_yygrowstack())
        {
            goto ipscan_yyoverflow;
        }
        *++ipscan_yyssp = ipscan_yystate = ipscan_yytable[ipscan_yyn];
        *++ipscan_yyvsp = ipscan_yylval;
        ipscan_yychar = YYEMPTY;
        if (ipscan_yyerrflag > 0)  --ipscan_yyerrflag;
        goto ipscan_yyloop;
    }
    if ((ipscan_yyn = ipscan_yyrindex[ipscan_yystate]) && (ipscan_yyn += ipscan_yychar) >= 0 &&
            ipscan_yyn <= YYTABLESIZE && ipscan_yycheck[ipscan_yyn] == ipscan_yychar)
    {
        ipscan_yyn = ipscan_yytable[ipscan_yyn];
        goto ipscan_yyreduce;
    }
    if (ipscan_yyerrflag) goto ipscan_yyinrecovery;

    ipscan_yyerror("syntax error");

#ifdef lint
    goto ipscan_yyerrlab;
#endif

ipscan_yyerrlab:
    ++ipscan_yynerrs;

ipscan_yyinrecovery:
    if (ipscan_yyerrflag < 3)
    {
        ipscan_yyerrflag = 3;
        for (;;)
        {
            if ((ipscan_yyn = ipscan_yysindex[*ipscan_yyssp]) && (ipscan_yyn += YYERRCODE) >= 0 &&
                    ipscan_yyn <= YYTABLESIZE && ipscan_yycheck[ipscan_yyn] == YYERRCODE)
            {
#if YYDEBUG
                if (ipscan_yydebug)
                    printf("%sdebug: state %d, error recovery shifting\
 to state %d\n", YYPREFIX, *ipscan_yyssp, ipscan_yytable[ipscan_yyn]);
#endif
                if (ipscan_yyssp >= ipscan_yysslim && ipscan_yygrowstack())
                {
                    goto ipscan_yyoverflow;
                }
                *++ipscan_yyssp = ipscan_yystate = ipscan_yytable[ipscan_yyn];
                *++ipscan_yyvsp = ipscan_yylval;
                goto ipscan_yyloop;
            }
            else
            {
#if YYDEBUG
                if (ipscan_yydebug)
                    printf("%sdebug: error recovery discarding state %d\n",
                            YYPREFIX, *ipscan_yyssp);
#endif
                if (ipscan_yyssp <= ipscan_yyss) goto ipscan_yyabort;
                --ipscan_yyssp;
                --ipscan_yyvsp;
            }
        }
    }
    else
    {
        if (ipscan_yychar == 0) goto ipscan_yyabort;
#if YYDEBUG
        if (ipscan_yydebug)
        {
            ipscan_yys = 0;
            if (ipscan_yychar <= YYMAXTOKEN) ipscan_yys = ipscan_yyname[ipscan_yychar];
            if (!ipscan_yys) ipscan_yys = "illegal-symbol";
            printf("%sdebug: state %d, error recovery discards token %d (%s)\n",
                    YYPREFIX, ipscan_yystate, ipscan_yychar, ipscan_yys);
        }
#endif
        ipscan_yychar = YYEMPTY;
        goto ipscan_yyloop;
    }

ipscan_yyreduce:
#if YYDEBUG
    if (ipscan_yydebug)
        printf("%sdebug: state %d, reducing by rule %d (%s)\n",
                YYPREFIX, ipscan_yystate, ipscan_yyn, ipscan_yyrule[ipscan_yyn]);
#endif
    ipscan_yym = ipscan_yylen[ipscan_yyn];
    ipscan_yyval = ipscan_yyvsp[1-ipscan_yym];
    switch (ipscan_yyn)
    {
case 9:
#line 82 "../tools/ipscan_y.y"
{ resetlexer(); }
break;
case 10:
#line 83 "../tools/ipscan_y.y"
{ resetlexer(); }
break;
case 11:
#line 84 "../tools/ipscan_y.y"
{ resetlexer(); }
break;
case 16:
#line 96 "../tools/ipscan_y.y"
{ set_variable(ipscan_yyvsp[-2].str, ipscan_yyvsp[0].str);
						  resetlexer();
						  free(ipscan_yyvsp[-2].str);
						  free(ipscan_yyvsp[0].str);
						  ipscan_yyvarnext = 0;
						}
break;
case 17:
#line 105 "../tools/ipscan_y.y"
{ ipscan_yyvarnext = 1; }
break;
case 18:
#line 108 "../tools/ipscan_y.y"
{ addtag(ipscan_yyvsp[-4].str, ipscan_yyvsp[-2].astr, NULL, &ipscan_yyvsp[0].act); }
break;
case 19:
#line 111 "../tools/ipscan_y.y"
{ addtag(ipscan_yyvsp[-7].str, NULL, ipscan_yyvsp[-2].astr, &ipscan_yyvsp[0].act); }
break;
case 20:
#line 114 "../tools/ipscan_y.y"
{ addtag(ipscan_yyvsp[-6].str, ipscan_yyvsp[-4].astr, ipscan_yyvsp[-2].astr, &ipscan_yyvsp[0].act); }
break;
case 23:
#line 121 "../tools/ipscan_y.y"
{ ipscan_yyval.str = ipscan_yyvsp[0].str; }
break;
case 24:
#line 125 "../tools/ipscan_y.y"
{ ipscan_yyval.astr = ipscan_yyvsp[0].astr; }
break;
case 25:
#line 126 "../tools/ipscan_y.y"
{ ipscan_yyval.astr = ipscan_yyvsp[0].astr; }
break;
case 26:
#line 129 "../tools/ipscan_y.y"
{ ipscan_yyval.act.act_val = ipscan_yyvsp[0].act.act_val;
					  ipscan_yyval.act.act_ip = ipscan_yyvsp[0].act.act_ip;
					  ipscan_yyval.act.act_port = ipscan_yyvsp[0].act.act_port; }
break;
case 27:
#line 132 "../tools/ipscan_y.y"
{ ipscan_yyval.act.act_val = ipscan_yyvsp[-2].act.act_val;
					  ipscan_yyval.act.act_else = ipscan_yyvsp[0].act.act_val;
					  if (ipscan_yyvsp[-2].act.act_val == IPSL_REDIRECT) {
						  ipscan_yyval.act.act_ip = ipscan_yyvsp[-2].act.act_ip;
						  ipscan_yyval.act.act_port = ipscan_yyvsp[-2].act.act_port;
					  }
					  if (ipscan_yyvsp[0].act.act_val == IPSL_REDIRECT) {
						  ipscan_yyval.act.act_eip = ipscan_yyvsp[0].act.act_eip;
						  ipscan_yyval.act.act_eport = ipscan_yyvsp[0].act.act_eport;
					  }
					}
break;
case 28:
#line 144 "../tools/ipscan_y.y"
{ ipscan_yyval.act.act_val = IPSL_CLOSE; }
break;
case 29:
#line 145 "../tools/ipscan_y.y"
{ ipscan_yyval.act.act_val = IPSL_TRACK; }
break;
case 30:
#line 146 "../tools/ipscan_y.y"
{ ipscan_yyval.act.act_val = IPSL_REDIRECT;
						  ipscan_yyval.act.act_ip = ipscan_yyvsp[0].act.act_ip;
						  ipscan_yyval.act.act_port = ipscan_yyvsp[0].act.act_port; }
break;
case 31:
#line 152 "../tools/ipscan_y.y"
{ ipscan_yyval.astr = makepair(ipscan_yyvsp[-1].str, NULL); }
break;
case 32:
#line 156 "../tools/ipscan_y.y"
{ ipscan_yyval.astr = makepair(ipscan_yyvsp[-3].str, ipscan_yyvsp[-1].str); }
break;
case 33:
#line 160 "../tools/ipscan_y.y"
{ ipscan_yyval.act.act_ip = ipscan_yyvsp[-1].ipa;
						  ipscan_yyval.act.act_port = 0; }
break;
case 34:
#line 163 "../tools/ipscan_y.y"
{ ipscan_yyval.act.act_ip = ipscan_yyvsp[-3].ipa;
						  ipscan_yyval.act.act_port = ipscan_yyvsp[-1].num; }
break;
case 35:
#line 169 "../tools/ipscan_y.y"
{ ipscan_yyval.ipa = combine(ipscan_yyvsp[-6].num,ipscan_yyvsp[-4].num,ipscan_yyvsp[-2].num,ipscan_yyvsp[0].num); }
break;
case 36:
#line 170 "../tools/ipscan_y.y"
{ ipscan_yyval.ipa = gethostip(ipscan_yyvsp[0].str);
						  free(ipscan_yyvsp[0].str);
						}
break;
case 37:
#line 176 "../tools/ipscan_y.y"
{ ipscan_yyval.num = htons(ipscan_yyvsp[0].num); }
break;
case 38:
#line 177 "../tools/ipscan_y.y"
{ ipscan_yyval.num = getportnum(ipscan_yyvsp[0].str);
						  free(ipscan_yyvsp[0].str);
						}
break;
#line 973 "y.tab.c"
    }
    ipscan_yyssp -= ipscan_yym;
    ipscan_yystate = *ipscan_yyssp;
    ipscan_yyvsp -= ipscan_yym;
    ipscan_yym = ipscan_yylhs[ipscan_yyn];
    if (ipscan_yystate == 0 && ipscan_yym == 0)
    {
#if YYDEBUG
        if (ipscan_yydebug)
            printf("%sdebug: after reduction, shifting from state 0 to\
 state %d\n", YYPREFIX, YYFINAL);
#endif
        ipscan_yystate = YYFINAL;
        *++ipscan_yyssp = YYFINAL;
        *++ipscan_yyvsp = ipscan_yyval;
        if (ipscan_yychar < 0)
        {
            if ((ipscan_yychar = ipscan_yylex()) < 0) ipscan_yychar = 0;
#if YYDEBUG
            if (ipscan_yydebug)
            {
                ipscan_yys = 0;
                if (ipscan_yychar <= YYMAXTOKEN) ipscan_yys = ipscan_yyname[ipscan_yychar];
                if (!ipscan_yys) ipscan_yys = "illegal-symbol";
                printf("%sdebug: state %d, reading %d (%s)\n",
                        YYPREFIX, YYFINAL, ipscan_yychar, ipscan_yys);
            }
#endif
        }
        if (ipscan_yychar == 0) goto ipscan_yyaccept;
        goto ipscan_yyloop;
    }
    if ((ipscan_yyn = ipscan_yygindex[ipscan_yym]) && (ipscan_yyn += ipscan_yystate) >= 0 &&
            ipscan_yyn <= YYTABLESIZE && ipscan_yycheck[ipscan_yyn] == ipscan_yystate)
        ipscan_yystate = ipscan_yytable[ipscan_yyn];
    else
        ipscan_yystate = ipscan_yydgoto[ipscan_yym];
#if YYDEBUG
    if (ipscan_yydebug)
        printf("%sdebug: after reduction, shifting from state %d \
to state %d\n", YYPREFIX, *ipscan_yyssp, ipscan_yystate);
#endif
    if (ipscan_yyssp >= ipscan_yysslim && ipscan_yygrowstack())
    {
        goto ipscan_yyoverflow;
    }
    *++ipscan_yyssp = ipscan_yystate;
    *++ipscan_yyvsp = ipscan_yyval;
    goto ipscan_yyloop;

ipscan_yyoverflow:
    ipscan_yyerror("yacc stack overflow");

ipscan_yyabort:
    return (1);

ipscan_yyaccept:
    return (0);
}
