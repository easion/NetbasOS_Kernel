/*
 * Copyright (C) 2002-2006 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 */
#include <ctype.h>
#include "ipf.h"
#ifdef	IPFILTER_SCAN
# include "netinet/ip_scan.h"
#endif
#include <sys/ioctl.h>
#include <syslog.h>
#ifdef	TEST_LEXER
# define	NO_YACC
union	{
	int		num;
	char		*str;
	struct in_addr	ipa;
	i6addr_t	ip6;
} ipmon_yylval;
#endif
#include "ipmon_l.h"
#include "ipmon_y.h"

FILE *ipmon_yyin;

#define	ishex(c)	(ISDIGIT(c) || ((c) >= 'a' && (c) <= 'f') || \
			 ((c) >= 'A' && (c) <= 'F'))
#define	TOOLONG		-3

extern int	string_start;
extern int	string_end;
extern char	*string_val;
extern int	pos;
extern int	ipmon_yydebug;

char		*ipmon_yystr = NULL;
int		ipmon_yytext[YYBUFSIZ+1];
char		ipmon_yychars[YYBUFSIZ+1];
int		ipmon_yylineNum = 1;
int		ipmon_yypos = 0;
int		ipmon_yylast = -1;
int		ipmon_yyexpectaddr = 0;
int		ipmon_yybreakondot = 0;
int		ipmon_yyvarnext = 0;
int		ipmon_yytokentype = 0;
wordtab_t	*ipmon_yywordtab = NULL;
int		ipmon_yysavedepth = 0;
wordtab_t	*ipmon_yysavewords[30];


static	wordtab_t	*ipmon_yyfindkey __P((char *));
static	int		ipmon_yygetc __P((int));
static	void		ipmon_yyunputc __P((int));
static	int		ipmon_yyswallow __P((int));
static	char		*ipmon_yytexttostr __P((int, int));
static	void		ipmon_yystrtotext __P((char *));
static	char		*ipmon_yytexttochar __P((void));

static int ipmon_yygetc(docont)
int docont;
{
	int c;

	if (ipmon_yypos < ipmon_yylast) {
		c = ipmon_yytext[ipmon_yypos++];
		if (c == '\n')
			ipmon_yylineNum++;
		return c;
	}

	if (ipmon_yypos == YYBUFSIZ)
		return TOOLONG;

	if (pos >= string_start && pos <= string_end) {
		c = string_val[pos - string_start];
		ipmon_yypos++;
	} else {
		c = fgetc(ipmon_yyin);
		if (docont && (c == '\\')) {
			c = fgetc(ipmon_yyin);
			if (c == '\n') {
				ipmon_yylineNum++;
				c = fgetc(ipmon_yyin);
			}
		}
	}
	if (c == '\n')
		ipmon_yylineNum++;
	ipmon_yytext[ipmon_yypos++] = c;
	ipmon_yylast = ipmon_yypos;
	ipmon_yytext[ipmon_yypos] = '\0';

	return c;
}


static void ipmon_yyunputc(c)
int c;
{
	if (c == '\n')
		ipmon_yylineNum--;
	ipmon_yytext[--ipmon_yypos] = c;
}


static int ipmon_yyswallow(last)
int last;
{
	int c;

	while (((c = ipmon_yygetc(0)) > '\0') && (c != last))
		;

	if (c != EOF)
		ipmon_yyunputc(c);
	if (c == last)
		return 0;
	return -1;
}


static char *ipmon_yytexttochar()
{
	int i;

	for (i = 0; i < ipmon_yypos; i++)
		ipmon_yychars[i] = (char)(ipmon_yytext[i] & 0xff);
	ipmon_yychars[i] = '\0';
	return ipmon_yychars;
}


static void ipmon_yystrtotext(str)
char *str;
{
	int len;
	char *s;

	len = strlen(str);
	if (len > YYBUFSIZ)
		len = YYBUFSIZ;

	for (s = str; *s != '\0' && len > 0; s++, len--)
		ipmon_yytext[ipmon_yylast++] = *s;
	ipmon_yytext[ipmon_yylast] = '\0';
}


static char *ipmon_yytexttostr(offset, max)
int offset, max;
{
	char *str;
	int i;

	if ((ipmon_yytext[offset] == '\'' || ipmon_yytext[offset] == '"') &&
	    (ipmon_yytext[offset] == ipmon_yytext[offset + max - 1])) {
		offset++;
		max--;
	}

	if (max > ipmon_yylast)
		max = ipmon_yylast;
	str = malloc(max + 1);
	if (str != NULL) {
		for (i = offset; i < max; i++)
			str[i - offset] = (char)(ipmon_yytext[i] & 0xff);
		str[i - offset] = '\0';
	}
	return str;
}


int ipmon_yylex()
{
	int c, n, isbuilding, rval, lnext, nokey = 0;
	char *name;

	isbuilding = 0;
	lnext = 0;
	rval = 0;

	if (ipmon_yystr != NULL) {
		free(ipmon_yystr);
		ipmon_yystr = NULL;
	}

nextchar:
	c = ipmon_yygetc(0);
	if (ipmon_yydebug > 1)
		printf("ipmon_yygetc = (%x) %c [%*.*s]\n", c, c, ipmon_yypos, ipmon_yypos, ipmon_yytexttochar());

	switch (c)
	{
	case '\n' :
		lnext = 0;
		nokey = 0;
	case '\t' :
	case '\r' :
	case ' ' :
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		if (ipmon_yylast > ipmon_yypos) {
			bcopy(ipmon_yytext + ipmon_yypos, ipmon_yytext,
			      sizeof(ipmon_yytext[0]) * (ipmon_yylast - ipmon_yypos + 1));
		}
		ipmon_yylast -= ipmon_yypos;
		ipmon_yypos = 0;
		lnext = 0;
		nokey = 0;
		goto nextchar;

	case '\\' :
		if (lnext == 0) {
			lnext = 1;
			if (ipmon_yylast == ipmon_yypos) {
				ipmon_yylast--;
				ipmon_yypos--;
			} else
				ipmon_yypos--;
			if (ipmon_yypos == 0)
				nokey = 1;
			goto nextchar;
		}
		break;
	}

	if (lnext == 1) {
		lnext = 0;
		if ((isbuilding == 0) && !ISALNUM(c)) {
			return c;
		}
		goto nextchar;
	}

	switch (c)
	{
	case '#' :
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		ipmon_yyswallow('\n');
		rval = YY_COMMENT;
		goto done;

	case '$' :
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		n = ipmon_yygetc(0);
		if (n == '{') {
			if (ipmon_yyswallow('}') == -1) {
				rval = -2;
				goto done;
			}
			(void) ipmon_yygetc(0);
		} else {
			if (!ISALPHA(n)) {
				ipmon_yyunputc(n);
				break;
			}
			do {
				n = ipmon_yygetc(1);
			} while (ISALPHA(n) || ISDIGIT(n) || n == '_');
			ipmon_yyunputc(n);
		}

		name = ipmon_yytexttostr(1, ipmon_yypos);		/* skip $ */

		if (name != NULL) {
			string_val = get_variable(name, NULL, ipmon_yylineNum);
			free(name);
			if (string_val != NULL) {
				name = ipmon_yytexttostr(ipmon_yypos, ipmon_yylast);
				if (name != NULL) {
					ipmon_yypos = 0;
					ipmon_yylast = 0;
					ipmon_yystrtotext(string_val);
					ipmon_yystrtotext(name);
					free(string_val);
					free(name);
					goto nextchar;
				}
				free(string_val);
			}
		}
		break;

	case '\'':
	case '"' :
		if (isbuilding == 1) {
			goto done;
		}
		do {
			n = ipmon_yygetc(1);
			if (n == EOF || n == TOOLONG) {
				rval = -2;
				goto done;
			}
			if (n == '\n') {
				ipmon_yyunputc(' ');
				ipmon_yypos++;
			}
		} while (n != c);
		rval = YY_STR;
		goto done;
		/* NOTREACHED */

	case EOF :
		ipmon_yylineNum = 1;
		ipmon_yypos = 0;
		ipmon_yylast = -1;
		ipmon_yyexpectaddr = 0;
		ipmon_yybreakondot = 0;
		ipmon_yyvarnext = 0;
		ipmon_yytokentype = 0;
		return 0;
	}

	if (strchr("=,/;{}()@", c) != NULL) {
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		rval = c;
		goto done;
	} else if (c == '.') {
		if (isbuilding == 0) {
			rval = c;
			goto done;
		}
		if (ipmon_yybreakondot != 0) {
			ipmon_yyunputc(c);
			goto done;
		}
	}

	switch (c)
	{
	case '-' :
		if (ipmon_yyexpectaddr)
			break;
		if (isbuilding == 1)
			break;
		n = ipmon_yygetc(0);
		if (n == '>') {
			isbuilding = 1;
			goto done;
		}
		ipmon_yyunputc(n);
		rval = '-';
		goto done;

	case '!' :
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		n = ipmon_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_NE;
			goto done;
		}
		ipmon_yyunputc(n);
		rval = '!';
		goto done;

	case '<' :
		if (ipmon_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		n = ipmon_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_LE;
			goto done;
		}
		if (n == '>') {
			rval = YY_RANGE_OUT;
			goto done;
		}
		ipmon_yyunputc(n);
		rval = YY_CMP_LT;
		goto done;

	case '>' :
		if (ipmon_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		n = ipmon_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_GE;
			goto done;
		}
		if (n == '<') {
			rval = YY_RANGE_IN;
			goto done;
		}
		ipmon_yyunputc(n);
		rval = YY_CMP_GT;
		goto done;
	}

	/*
	 * Now for the reason this is here...IPv6 address parsing.
	 * The longest string we can expect is of this form:
	 * 0000:0000:0000:0000:0000:0000:000.000.000.000
	 * not:
	 * 0000:0000:0000:0000:0000:0000:0000:0000
	 */
#ifdef	USE_INET6
	if (ipmon_yyexpectaddr == 1 && isbuilding == 0 && (ishex(c) || c == ':')) {
		char ipv6buf[45 + 1], *s, oc;
		int start;

		start = ipmon_yypos;
		s = ipv6buf;
		oc = c;

		/*
		 * Perhaps we should implement stricter controls on what we
		 * swallow up here, but surely it would just be duplicating
		 * the code in inet_pton() anyway.
		 */
		do {
			*s++ = c;
			c = ipmon_yygetc(1);
		} while ((ishex(c) || c == ':' || c == '.') &&
			 (s - ipv6buf < 46));
		ipmon_yyunputc(c);
		*s = '\0';

		if (inet_pton(AF_INET6, ipv6buf, &ipmon_yylval.ip6) == 1) {
			rval = YY_IPV6;
			ipmon_yyexpectaddr = 0;
			goto done;
		}
		ipmon_yypos = start;
		c = oc;
	}
#endif

	if (c == ':') {
		if (isbuilding == 1) {
			ipmon_yyunputc(c);
			goto done;
		}
		rval = ':';
		goto done;
	}

	if (isbuilding == 0 && c == '0') {
		n = ipmon_yygetc(0);
		if (n == 'x') {
			do {
				n = ipmon_yygetc(1);
			} while (ishex(n));
			ipmon_yyunputc(n);
			rval = YY_HEX;
			goto done;
		}
		ipmon_yyunputc(n);
	}

	/*
	 * No negative numbers with leading - sign..
	 */
	if (isbuilding == 0 && ISDIGIT(c)) {
		do {
			n = ipmon_yygetc(1);
		} while (ISDIGIT(n));
		ipmon_yyunputc(n);
		rval = YY_NUMBER;
		goto done;
	}

	isbuilding = 1;
	goto nextchar;

done:
	ipmon_yystr = ipmon_yytexttostr(0, ipmon_yypos);

	if (ipmon_yydebug)
		printf("isbuilding %d ipmon_yyvarnext %d nokey %d\n",
		       isbuilding, ipmon_yyvarnext, nokey);
	if (isbuilding == 1) {
		wordtab_t *w;

		w = NULL;
		isbuilding = 0;

		if ((ipmon_yyvarnext == 0) && (nokey == 0)) {
			w = ipmon_yyfindkey(ipmon_yystr);
			if (w == NULL && ipmon_yywordtab != NULL) {
				ipmon_yyresetdict();
				w = ipmon_yyfindkey(ipmon_yystr);
			}
		} else
			ipmon_yyvarnext = 0;
		if (w != NULL)
			rval = w->w_value;
		else
			rval = YY_STR;
	}

	if (rval == YY_STR && ipmon_yysavedepth > 0)
		ipmon_yyresetdict();

	ipmon_yytokentype = rval;

	if (ipmon_yydebug)
		printf("lexed(%s) [%d,%d,%d] => %d @%d\n", ipmon_yystr, string_start,
			string_end, pos, rval, ipmon_yysavedepth);

	switch (rval)
	{
	case YY_NUMBER :
		sscanf(ipmon_yystr, "%u", &ipmon_yylval.num);
		break;

	case YY_HEX :
		sscanf(ipmon_yystr, "0x%x", (u_int *)&ipmon_yylval.num);
		break;

	case YY_STR :
		ipmon_yylval.str = strdup(ipmon_yystr);
		break;

	default :
		break;
	}

	if (ipmon_yylast > 0) {
		bcopy(ipmon_yytext + ipmon_yypos, ipmon_yytext,
		      sizeof(ipmon_yytext[0]) * (ipmon_yylast - ipmon_yypos + 1));
		ipmon_yylast -= ipmon_yypos;
		ipmon_yypos = 0;
	}

	return rval;
}


static wordtab_t *ipmon_yyfindkey(key)
char *key;
{
	wordtab_t *w;

	if (ipmon_yywordtab == NULL)
		return NULL;

	for (w = ipmon_yywordtab; w->w_word != 0; w++)
		if (strcasecmp(key, w->w_word) == 0)
			return w;
	return NULL;
}


char *ipmon_yykeytostr(num)
int num;
{
	wordtab_t *w;

	if (ipmon_yywordtab == NULL)
		return "<unknown>";

	for (w = ipmon_yywordtab; w->w_word; w++)
		if (w->w_value == num)
			return w->w_word;
	return "<unknown>";
}


wordtab_t *ipmon_yysettab(words)
wordtab_t *words;
{
	wordtab_t *save;

	save = ipmon_yywordtab;
	ipmon_yywordtab = words;
	return save;
}


void ipmon_yyerror(msg)
char *msg;
{
	char *txt, letter[2];
	int freetxt = 0;

	if (ipmon_yytokentype < 256) {
		letter[0] = ipmon_yytokentype;
		letter[1] = '\0';
		txt =  letter;
	} else if (ipmon_yytokentype == YY_STR || ipmon_yytokentype == YY_HEX ||
		   ipmon_yytokentype == YY_NUMBER) {
		if (ipmon_yystr == NULL) {
			txt = ipmon_yytexttostr(ipmon_yypos, YYBUFSIZ);
			freetxt = 1;
		} else
			txt = ipmon_yystr;
	} else {
		txt = ipmon_yykeytostr(ipmon_yytokentype);
	}
	fprintf(stderr, "%s error at \"%s\", line %d\n", msg, txt, ipmon_yylineNum);
	if (freetxt == 1)
		free(txt);
	exit(1);
}


void ipmon_yysetdict(newdict)
wordtab_t *newdict;
{
	if (ipmon_yysavedepth == sizeof(ipmon_yysavewords)/sizeof(ipmon_yysavewords[0])) {
		fprintf(stderr, "%d: at maximum dictionary depth\n",
			ipmon_yylineNum);
		return;
	}

	ipmon_yysavewords[ipmon_yysavedepth++] = ipmon_yysettab(newdict);
	if (ipmon_yydebug)
		printf("ipmon_yysavedepth++ => %d\n", ipmon_yysavedepth);
}

void ipmon_yyresetdict()
{
	if (ipmon_yydebug)
		printf("ipmon_yyresetdict(%d)\n", ipmon_yysavedepth);
	if (ipmon_yysavedepth > 0) {
		ipmon_yysettab(ipmon_yysavewords[--ipmon_yysavedepth]);
		if (ipmon_yydebug)
			printf("ipmon_yysavedepth-- => %d\n", ipmon_yysavedepth);
	}
}



#ifdef	TEST_LEXER
int main(argc, argv)
int argc;
char *argv[];
{
	int n;

	ipmon_yyin = stdin;

	while ((n = ipmon_yylex()) != 0)
		printf("%d.n = %d [%s] %d %d\n",
			ipmon_yylineNum, n, ipmon_yystr, ipmon_yypos, ipmon_yylast);
}
#endif
