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
} ippool_yylval;
#endif
#include "ippool_l.h"
#include "ippool_y.h"

FILE *ippool_yyin;

#define	ishex(c)	(ISDIGIT(c) || ((c) >= 'a' && (c) <= 'f') || \
			 ((c) >= 'A' && (c) <= 'F'))
#define	TOOLONG		-3

extern int	string_start;
extern int	string_end;
extern char	*string_val;
extern int	pos;
extern int	ippool_yydebug;

char		*ippool_yystr = NULL;
int		ippool_yytext[YYBUFSIZ+1];
char		ippool_yychars[YYBUFSIZ+1];
int		ippool_yylineNum = 1;
int		ippool_yypos = 0;
int		ippool_yylast = -1;
int		ippool_yyexpectaddr = 0;
int		ippool_yybreakondot = 0;
int		ippool_yyvarnext = 0;
int		ippool_yytokentype = 0;
wordtab_t	*ippool_yywordtab = NULL;
int		ippool_yysavedepth = 0;
wordtab_t	*ippool_yysavewords[30];


static	wordtab_t	*ippool_yyfindkey __P((char *));
static	int		ippool_yygetc __P((int));
static	void		ippool_yyunputc __P((int));
static	int		ippool_yyswallow __P((int));
static	char		*ippool_yytexttostr __P((int, int));
static	void		ippool_yystrtotext __P((char *));
static	char		*ippool_yytexttochar __P((void));

static int ippool_yygetc(docont)
int docont;
{
	int c;

	if (ippool_yypos < ippool_yylast) {
		c = ippool_yytext[ippool_yypos++];
		if (c == '\n')
			ippool_yylineNum++;
		return c;
	}

	if (ippool_yypos == YYBUFSIZ)
		return TOOLONG;

	if (pos >= string_start && pos <= string_end) {
		c = string_val[pos - string_start];
		ippool_yypos++;
	} else {
		c = fgetc(ippool_yyin);
		if (docont && (c == '\\')) {
			c = fgetc(ippool_yyin);
			if (c == '\n') {
				ippool_yylineNum++;
				c = fgetc(ippool_yyin);
			}
		}
	}
	if (c == '\n')
		ippool_yylineNum++;
	ippool_yytext[ippool_yypos++] = c;
	ippool_yylast = ippool_yypos;
	ippool_yytext[ippool_yypos] = '\0';

	return c;
}


static void ippool_yyunputc(c)
int c;
{
	if (c == '\n')
		ippool_yylineNum--;
	ippool_yytext[--ippool_yypos] = c;
}


static int ippool_yyswallow(last)
int last;
{
	int c;

	while (((c = ippool_yygetc(0)) > '\0') && (c != last))
		;

	if (c != EOF)
		ippool_yyunputc(c);
	if (c == last)
		return 0;
	return -1;
}


static char *ippool_yytexttochar()
{
	int i;

	for (i = 0; i < ippool_yypos; i++)
		ippool_yychars[i] = (char)(ippool_yytext[i] & 0xff);
	ippool_yychars[i] = '\0';
	return ippool_yychars;
}


static void ippool_yystrtotext(str)
char *str;
{
	int len;
	char *s;

	len = strlen(str);
	if (len > YYBUFSIZ)
		len = YYBUFSIZ;

	for (s = str; *s != '\0' && len > 0; s++, len--)
		ippool_yytext[ippool_yylast++] = *s;
	ippool_yytext[ippool_yylast] = '\0';
}


static char *ippool_yytexttostr(offset, max)
int offset, max;
{
	char *str;
	int i;

	if ((ippool_yytext[offset] == '\'' || ippool_yytext[offset] == '"') &&
	    (ippool_yytext[offset] == ippool_yytext[offset + max - 1])) {
		offset++;
		max--;
	}

	if (max > ippool_yylast)
		max = ippool_yylast;
	str = malloc(max + 1);
	if (str != NULL) {
		for (i = offset; i < max; i++)
			str[i - offset] = (char)(ippool_yytext[i] & 0xff);
		str[i - offset] = '\0';
	}
	return str;
}


int ippool_yylex()
{
	int c, n, isbuilding, rval, lnext, nokey = 0;
	char *name;

	isbuilding = 0;
	lnext = 0;
	rval = 0;

	if (ippool_yystr != NULL) {
		free(ippool_yystr);
		ippool_yystr = NULL;
	}

nextchar:
	c = ippool_yygetc(0);
	if (ippool_yydebug > 1)
		printf("ippool_yygetc = (%x) %c [%*.*s]\n", c, c, ippool_yypos, ippool_yypos, ippool_yytexttochar());

	switch (c)
	{
	case '\n' :
		lnext = 0;
		nokey = 0;
	case '\t' :
	case '\r' :
	case ' ' :
		if (isbuilding == 1) {
			ippool_yyunputc(c);
			goto done;
		}
		if (ippool_yylast > ippool_yypos) {
			bcopy(ippool_yytext + ippool_yypos, ippool_yytext,
			      sizeof(ippool_yytext[0]) * (ippool_yylast - ippool_yypos + 1));
		}
		ippool_yylast -= ippool_yypos;
		ippool_yypos = 0;
		lnext = 0;
		nokey = 0;
		goto nextchar;

	case '\\' :
		if (lnext == 0) {
			lnext = 1;
			if (ippool_yylast == ippool_yypos) {
				ippool_yylast--;
				ippool_yypos--;
			} else
				ippool_yypos--;
			if (ippool_yypos == 0)
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
			ippool_yyunputc(c);
			goto done;
		}
		ippool_yyswallow('\n');
		rval = YY_COMMENT;
		goto done;

	case '$' :
		if (isbuilding == 1) {
			ippool_yyunputc(c);
			goto done;
		}
		n = ippool_yygetc(0);
		if (n == '{') {
			if (ippool_yyswallow('}') == -1) {
				rval = -2;
				goto done;
			}
			(void) ippool_yygetc(0);
		} else {
			if (!ISALPHA(n)) {
				ippool_yyunputc(n);
				break;
			}
			do {
				n = ippool_yygetc(1);
			} while (ISALPHA(n) || ISDIGIT(n) || n == '_');
			ippool_yyunputc(n);
		}

		name = ippool_yytexttostr(1, ippool_yypos);		/* skip $ */

		if (name != NULL) {
			string_val = get_variable(name, NULL, ippool_yylineNum);
			free(name);
			if (string_val != NULL) {
				name = ippool_yytexttostr(ippool_yypos, ippool_yylast);
				if (name != NULL) {
					ippool_yypos = 0;
					ippool_yylast = 0;
					ippool_yystrtotext(string_val);
					ippool_yystrtotext(name);
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
			n = ippool_yygetc(1);
			if (n == EOF || n == TOOLONG) {
				rval = -2;
				goto done;
			}
			if (n == '\n') {
				ippool_yyunputc(' ');
				ippool_yypos++;
			}
		} while (n != c);
		rval = YY_STR;
		goto done;
		/* NOTREACHED */

	case EOF :
		ippool_yylineNum = 1;
		ippool_yypos = 0;
		ippool_yylast = -1;
		ippool_yyexpectaddr = 0;
		ippool_yybreakondot = 0;
		ippool_yyvarnext = 0;
		ippool_yytokentype = 0;
		return 0;
	}

	if (strchr("=,/;{}()@", c) != NULL) {
		if (isbuilding == 1) {
			ippool_yyunputc(c);
			goto done;
		}
		rval = c;
		goto done;
	} else if (c == '.') {
		if (isbuilding == 0) {
			rval = c;
			goto done;
		}
		if (ippool_yybreakondot != 0) {
			ippool_yyunputc(c);
			goto done;
		}
	}

	switch (c)
	{
	case '-' :
		if (ippool_yyexpectaddr)
			break;
		if (isbuilding == 1)
			break;
		n = ippool_yygetc(0);
		if (n == '>') {
			isbuilding = 1;
			goto done;
		}
		ippool_yyunputc(n);
		rval = '-';
		goto done;

	case '!' :
		if (isbuilding == 1) {
			ippool_yyunputc(c);
			goto done;
		}
		n = ippool_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_NE;
			goto done;
		}
		ippool_yyunputc(n);
		rval = '!';
		goto done;

	case '<' :
		if (ippool_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ippool_yyunputc(c);
			goto done;
		}
		n = ippool_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_LE;
			goto done;
		}
		if (n == '>') {
			rval = YY_RANGE_OUT;
			goto done;
		}
		ippool_yyunputc(n);
		rval = YY_CMP_LT;
		goto done;

	case '>' :
		if (ippool_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ippool_yyunputc(c);
			goto done;
		}
		n = ippool_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_GE;
			goto done;
		}
		if (n == '<') {
			rval = YY_RANGE_IN;
			goto done;
		}
		ippool_yyunputc(n);
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
	if (ippool_yyexpectaddr == 1 && isbuilding == 0 && (ishex(c) || c == ':')) {
		char ipv6buf[45 + 1], *s, oc;
		int start;

		start = ippool_yypos;
		s = ipv6buf;
		oc = c;

		/*
		 * Perhaps we should implement stricter controls on what we
		 * swallow up here, but surely it would just be duplicating
		 * the code in inet_pton() anyway.
		 */
		do {
			*s++ = c;
			c = ippool_yygetc(1);
		} while ((ishex(c) || c == ':' || c == '.') &&
			 (s - ipv6buf < 46));
		ippool_yyunputc(c);
		*s = '\0';

		if (inet_pton(AF_INET6, ipv6buf, &ippool_yylval.ip6) == 1) {
			rval = YY_IPV6;
			ippool_yyexpectaddr = 0;
			goto done;
		}
		ippool_yypos = start;
		c = oc;
	}
#endif

	if (c == ':') {
		if (isbuilding == 1) {
			ippool_yyunputc(c);
			goto done;
		}
		rval = ':';
		goto done;
	}

	if (isbuilding == 0 && c == '0') {
		n = ippool_yygetc(0);
		if (n == 'x') {
			do {
				n = ippool_yygetc(1);
			} while (ishex(n));
			ippool_yyunputc(n);
			rval = YY_HEX;
			goto done;
		}
		ippool_yyunputc(n);
	}

	/*
	 * No negative numbers with leading - sign..
	 */
	if (isbuilding == 0 && ISDIGIT(c)) {
		do {
			n = ippool_yygetc(1);
		} while (ISDIGIT(n));
		ippool_yyunputc(n);
		rval = YY_NUMBER;
		goto done;
	}

	isbuilding = 1;
	goto nextchar;

done:
	ippool_yystr = ippool_yytexttostr(0, ippool_yypos);

	if (ippool_yydebug)
		printf("isbuilding %d ippool_yyvarnext %d nokey %d\n",
		       isbuilding, ippool_yyvarnext, nokey);
	if (isbuilding == 1) {
		wordtab_t *w;

		w = NULL;
		isbuilding = 0;

		if ((ippool_yyvarnext == 0) && (nokey == 0)) {
			w = ippool_yyfindkey(ippool_yystr);
			if (w == NULL && ippool_yywordtab != NULL) {
				ippool_yyresetdict();
				w = ippool_yyfindkey(ippool_yystr);
			}
		} else
			ippool_yyvarnext = 0;
		if (w != NULL)
			rval = w->w_value;
		else
			rval = YY_STR;
	}

	if (rval == YY_STR && ippool_yysavedepth > 0)
		ippool_yyresetdict();

	ippool_yytokentype = rval;

	if (ippool_yydebug)
		printf("lexed(%s) [%d,%d,%d] => %d @%d\n", ippool_yystr, string_start,
			string_end, pos, rval, ippool_yysavedepth);

	switch (rval)
	{
	case YY_NUMBER :
		sscanf(ippool_yystr, "%u", &ippool_yylval.num);
		break;

	case YY_HEX :
		sscanf(ippool_yystr, "0x%x", (u_int *)&ippool_yylval.num);
		break;

	case YY_STR :
		ippool_yylval.str = strdup(ippool_yystr);
		break;

	default :
		break;
	}

	if (ippool_yylast > 0) {
		bcopy(ippool_yytext + ippool_yypos, ippool_yytext,
		      sizeof(ippool_yytext[0]) * (ippool_yylast - ippool_yypos + 1));
		ippool_yylast -= ippool_yypos;
		ippool_yypos = 0;
	}

	return rval;
}


static wordtab_t *ippool_yyfindkey(key)
char *key;
{
	wordtab_t *w;

	if (ippool_yywordtab == NULL)
		return NULL;

	for (w = ippool_yywordtab; w->w_word != 0; w++)
		if (strcasecmp(key, w->w_word) == 0)
			return w;
	return NULL;
}


char *ippool_yykeytostr(num)
int num;
{
	wordtab_t *w;

	if (ippool_yywordtab == NULL)
		return "<unknown>";

	for (w = ippool_yywordtab; w->w_word; w++)
		if (w->w_value == num)
			return w->w_word;
	return "<unknown>";
}


wordtab_t *ippool_yysettab(words)
wordtab_t *words;
{
	wordtab_t *save;

	save = ippool_yywordtab;
	ippool_yywordtab = words;
	return save;
}


void ippool_yyerror(msg)
char *msg;
{
	char *txt, letter[2];
	int freetxt = 0;

	if (ippool_yytokentype < 256) {
		letter[0] = ippool_yytokentype;
		letter[1] = '\0';
		txt =  letter;
	} else if (ippool_yytokentype == YY_STR || ippool_yytokentype == YY_HEX ||
		   ippool_yytokentype == YY_NUMBER) {
		if (ippool_yystr == NULL) {
			txt = ippool_yytexttostr(ippool_yypos, YYBUFSIZ);
			freetxt = 1;
		} else
			txt = ippool_yystr;
	} else {
		txt = ippool_yykeytostr(ippool_yytokentype);
	}
	fprintf(stderr, "%s error at \"%s\", line %d\n", msg, txt, ippool_yylineNum);
	if (freetxt == 1)
		free(txt);
	exit(1);
}


void ippool_yysetdict(newdict)
wordtab_t *newdict;
{
	if (ippool_yysavedepth == sizeof(ippool_yysavewords)/sizeof(ippool_yysavewords[0])) {
		fprintf(stderr, "%d: at maximum dictionary depth\n",
			ippool_yylineNum);
		return;
	}

	ippool_yysavewords[ippool_yysavedepth++] = ippool_yysettab(newdict);
	if (ippool_yydebug)
		printf("ippool_yysavedepth++ => %d\n", ippool_yysavedepth);
}

void ippool_yyresetdict()
{
	if (ippool_yydebug)
		printf("ippool_yyresetdict(%d)\n", ippool_yysavedepth);
	if (ippool_yysavedepth > 0) {
		ippool_yysettab(ippool_yysavewords[--ippool_yysavedepth]);
		if (ippool_yydebug)
			printf("ippool_yysavedepth-- => %d\n", ippool_yysavedepth);
	}
}



#ifdef	TEST_LEXER
int main(argc, argv)
int argc;
char *argv[];
{
	int n;

	ippool_yyin = stdin;

	while ((n = ippool_yylex()) != 0)
		printf("%d.n = %d [%s] %d %d\n",
			ippool_yylineNum, n, ippool_yystr, ippool_yypos, ippool_yylast);
}
#endif
