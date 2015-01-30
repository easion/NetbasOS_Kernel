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
} ipf_yylval;
#endif
#include "ipf_l.h"
#include "ipf_y.h"

FILE *ipf_yyin;

#define	ishex(c)	(ISDIGIT(c) || ((c) >= 'a' && (c) <= 'f') || \
			 ((c) >= 'A' && (c) <= 'F'))
#define	TOOLONG		-3

extern int	string_start;
extern int	string_end;
extern char	*string_val;
extern int	pos;
extern int	ipf_yydebug;

char		*ipf_yystr = NULL;
int		ipf_yytext[YYBUFSIZ+1];
char		ipf_yychars[YYBUFSIZ+1];
int		ipf_yylineNum = 1;
int		ipf_yypos = 0;
int		ipf_yylast = -1;
int		ipf_yyexpectaddr = 0;
int		ipf_yybreakondot = 0;
int		ipf_yyvarnext = 0;
int		ipf_yytokentype = 0;
wordtab_t	*ipf_yywordtab = NULL;
int		ipf_yysavedepth = 0;
wordtab_t	*ipf_yysavewords[30];


static	wordtab_t	*ipf_yyfindkey __P((char *));
static	int		ipf_yygetc __P((int));
static	void		ipf_yyunputc __P((int));
static	int		ipf_yyswallow __P((int));
static	char		*ipf_yytexttostr __P((int, int));
static	void		ipf_yystrtotext __P((char *));
static	char		*ipf_yytexttochar __P((void));

static int ipf_yygetc(docont)
int docont;
{
	int c;

	if (ipf_yypos < ipf_yylast) {
		c = ipf_yytext[ipf_yypos++];
		if (c == '\n')
			ipf_yylineNum++;
		return c;
	}

	if (ipf_yypos == YYBUFSIZ)
		return TOOLONG;

	if (pos >= string_start && pos <= string_end) {
		c = string_val[pos - string_start];
		ipf_yypos++;
	} else {
		c = fgetc(ipf_yyin);
		if (docont && (c == '\\')) {
			c = fgetc(ipf_yyin);
			if (c == '\n') {
				ipf_yylineNum++;
				c = fgetc(ipf_yyin);
			}
		}
	}
	if (c == '\n')
		ipf_yylineNum++;
	ipf_yytext[ipf_yypos++] = c;
	ipf_yylast = ipf_yypos;
	ipf_yytext[ipf_yypos] = '\0';

	return c;
}


static void ipf_yyunputc(c)
int c;
{
	if (c == '\n')
		ipf_yylineNum--;
	ipf_yytext[--ipf_yypos] = c;
}


static int ipf_yyswallow(last)
int last;
{
	int c;

	while (((c = ipf_yygetc(0)) > '\0') && (c != last))
		;

	if (c != EOF)
		ipf_yyunputc(c);
	if (c == last)
		return 0;
	return -1;
}


static char *ipf_yytexttochar()
{
	int i;

	for (i = 0; i < ipf_yypos; i++)
		ipf_yychars[i] = (char)(ipf_yytext[i] & 0xff);
	ipf_yychars[i] = '\0';
	return ipf_yychars;
}


static void ipf_yystrtotext(str)
char *str;
{
	int len;
	char *s;

	len = strlen(str);
	if (len > YYBUFSIZ)
		len = YYBUFSIZ;

	for (s = str; *s != '\0' && len > 0; s++, len--)
		ipf_yytext[ipf_yylast++] = *s;
	ipf_yytext[ipf_yylast] = '\0';
}


static char *ipf_yytexttostr(offset, max)
int offset, max;
{
	char *str;
	int i;

	if ((ipf_yytext[offset] == '\'' || ipf_yytext[offset] == '"') &&
	    (ipf_yytext[offset] == ipf_yytext[offset + max - 1])) {
		offset++;
		max--;
	}

	if (max > ipf_yylast)
		max = ipf_yylast;
	str = malloc(max + 1);
	if (str != NULL) {
		for (i = offset; i < max; i++)
			str[i - offset] = (char)(ipf_yytext[i] & 0xff);
		str[i - offset] = '\0';
	}
	return str;
}


int ipf_yylex()
{
	int c, n, isbuilding, rval, lnext, nokey = 0;
	char *name;

	isbuilding = 0;
	lnext = 0;
	rval = 0;

	if (ipf_yystr != NULL) {
		free(ipf_yystr);
		ipf_yystr = NULL;
	}

nextchar:
	c = ipf_yygetc(0);
	if (ipf_yydebug > 1)
		printf("ipf_yygetc = (%x) %c [%*.*s]\n", c, c, ipf_yypos, ipf_yypos, ipf_yytexttochar());

	switch (c)
	{
	case '\n' :
		lnext = 0;
		nokey = 0;
	case '\t' :
	case '\r' :
	case ' ' :
		if (isbuilding == 1) {
			ipf_yyunputc(c);
			goto done;
		}
		if (ipf_yylast > ipf_yypos) {
			bcopy(ipf_yytext + ipf_yypos, ipf_yytext,
			      sizeof(ipf_yytext[0]) * (ipf_yylast - ipf_yypos + 1));
		}
		ipf_yylast -= ipf_yypos;
		ipf_yypos = 0;
		lnext = 0;
		nokey = 0;
		goto nextchar;

	case '\\' :
		if (lnext == 0) {
			lnext = 1;
			if (ipf_yylast == ipf_yypos) {
				ipf_yylast--;
				ipf_yypos--;
			} else
				ipf_yypos--;
			if (ipf_yypos == 0)
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
			ipf_yyunputc(c);
			goto done;
		}
		ipf_yyswallow('\n');
		rval = YY_COMMENT;
		goto done;

	case '$' :
		if (isbuilding == 1) {
			ipf_yyunputc(c);
			goto done;
		}
		n = ipf_yygetc(0);
		if (n == '{') {
			if (ipf_yyswallow('}') == -1) {
				rval = -2;
				goto done;
			}
			(void) ipf_yygetc(0);
		} else {
			if (!ISALPHA(n)) {
				ipf_yyunputc(n);
				break;
			}
			do {
				n = ipf_yygetc(1);
			} while (ISALPHA(n) || ISDIGIT(n) || n == '_');
			ipf_yyunputc(n);
		}

		name = ipf_yytexttostr(1, ipf_yypos);		/* skip $ */

		if (name != NULL) {
			string_val = get_variable(name, NULL, ipf_yylineNum);
			free(name);
			if (string_val != NULL) {
				name = ipf_yytexttostr(ipf_yypos, ipf_yylast);
				if (name != NULL) {
					ipf_yypos = 0;
					ipf_yylast = 0;
					ipf_yystrtotext(string_val);
					ipf_yystrtotext(name);
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
			n = ipf_yygetc(1);
			if (n == EOF || n == TOOLONG) {
				rval = -2;
				goto done;
			}
			if (n == '\n') {
				ipf_yyunputc(' ');
				ipf_yypos++;
			}
		} while (n != c);
		rval = YY_STR;
		goto done;
		/* NOTREACHED */

	case EOF :
		ipf_yylineNum = 1;
		ipf_yypos = 0;
		ipf_yylast = -1;
		ipf_yyexpectaddr = 0;
		ipf_yybreakondot = 0;
		ipf_yyvarnext = 0;
		ipf_yytokentype = 0;
		return 0;
	}

	if (strchr("=,/;{}()@", c) != NULL) {
		if (isbuilding == 1) {
			ipf_yyunputc(c);
			goto done;
		}
		rval = c;
		goto done;
	} else if (c == '.') {
		if (isbuilding == 0) {
			rval = c;
			goto done;
		}
		if (ipf_yybreakondot != 0) {
			ipf_yyunputc(c);
			goto done;
		}
	}

	switch (c)
	{
	case '-' :
		if (ipf_yyexpectaddr)
			break;
		if (isbuilding == 1)
			break;
		n = ipf_yygetc(0);
		if (n == '>') {
			isbuilding = 1;
			goto done;
		}
		ipf_yyunputc(n);
		rval = '-';
		goto done;

	case '!' :
		if (isbuilding == 1) {
			ipf_yyunputc(c);
			goto done;
		}
		n = ipf_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_NE;
			goto done;
		}
		ipf_yyunputc(n);
		rval = '!';
		goto done;

	case '<' :
		if (ipf_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipf_yyunputc(c);
			goto done;
		}
		n = ipf_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_LE;
			goto done;
		}
		if (n == '>') {
			rval = YY_RANGE_OUT;
			goto done;
		}
		ipf_yyunputc(n);
		rval = YY_CMP_LT;
		goto done;

	case '>' :
		if (ipf_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipf_yyunputc(c);
			goto done;
		}
		n = ipf_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_GE;
			goto done;
		}
		if (n == '<') {
			rval = YY_RANGE_IN;
			goto done;
		}
		ipf_yyunputc(n);
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
	if (ipf_yyexpectaddr == 1 && isbuilding == 0 && (ishex(c) || c == ':')) {
		char ipv6buf[45 + 1], *s, oc;
		int start;

		start = ipf_yypos;
		s = ipv6buf;
		oc = c;

		/*
		 * Perhaps we should implement stricter controls on what we
		 * swallow up here, but surely it would just be duplicating
		 * the code in inet_pton() anyway.
		 */
		do {
			*s++ = c;
			c = ipf_yygetc(1);
		} while ((ishex(c) || c == ':' || c == '.') &&
			 (s - ipv6buf < 46));
		ipf_yyunputc(c);
		*s = '\0';

		if (inet_pton(AF_INET6, ipv6buf, &ipf_yylval.ip6) == 1) {
			rval = YY_IPV6;
			ipf_yyexpectaddr = 0;
			goto done;
		}
		ipf_yypos = start;
		c = oc;
	}
#endif

	if (c == ':') {
		if (isbuilding == 1) {
			ipf_yyunputc(c);
			goto done;
		}
		rval = ':';
		goto done;
	}

	if (isbuilding == 0 && c == '0') {
		n = ipf_yygetc(0);
		if (n == 'x') {
			do {
				n = ipf_yygetc(1);
			} while (ishex(n));
			ipf_yyunputc(n);
			rval = YY_HEX;
			goto done;
		}
		ipf_yyunputc(n);
	}

	/*
	 * No negative numbers with leading - sign..
	 */
	if (isbuilding == 0 && ISDIGIT(c)) {
		do {
			n = ipf_yygetc(1);
		} while (ISDIGIT(n));
		ipf_yyunputc(n);
		rval = YY_NUMBER;
		goto done;
	}

	isbuilding = 1;
	goto nextchar;

done:
	ipf_yystr = ipf_yytexttostr(0, ipf_yypos);

	if (ipf_yydebug)
		printf("isbuilding %d ipf_yyvarnext %d nokey %d\n",
		       isbuilding, ipf_yyvarnext, nokey);
	if (isbuilding == 1) {
		wordtab_t *w;

		w = NULL;
		isbuilding = 0;

		if ((ipf_yyvarnext == 0) && (nokey == 0)) {
			w = ipf_yyfindkey(ipf_yystr);
			if (w == NULL && ipf_yywordtab != NULL) {
				ipf_yyresetdict();
				w = ipf_yyfindkey(ipf_yystr);
			}
		} else
			ipf_yyvarnext = 0;
		if (w != NULL)
			rval = w->w_value;
		else
			rval = YY_STR;
	}

	if (rval == YY_STR && ipf_yysavedepth > 0)
		ipf_yyresetdict();

	ipf_yytokentype = rval;

	if (ipf_yydebug)
		printf("lexed(%s) [%d,%d,%d] => %d @%d\n", ipf_yystr, string_start,
			string_end, pos, rval, ipf_yysavedepth);

	switch (rval)
	{
	case YY_NUMBER :
		sscanf(ipf_yystr, "%u", &ipf_yylval.num);
		break;

	case YY_HEX :
		sscanf(ipf_yystr, "0x%x", (u_int *)&ipf_yylval.num);
		break;

	case YY_STR :
		ipf_yylval.str = strdup(ipf_yystr);
		break;

	default :
		break;
	}

	if (ipf_yylast > 0) {
		bcopy(ipf_yytext + ipf_yypos, ipf_yytext,
		      sizeof(ipf_yytext[0]) * (ipf_yylast - ipf_yypos + 1));
		ipf_yylast -= ipf_yypos;
		ipf_yypos = 0;
	}

	return rval;
}


static wordtab_t *ipf_yyfindkey(key)
char *key;
{
	wordtab_t *w;

	if (ipf_yywordtab == NULL)
		return NULL;

	for (w = ipf_yywordtab; w->w_word != 0; w++)
		if (strcasecmp(key, w->w_word) == 0)
			return w;
	return NULL;
}


char *ipf_yykeytostr(num)
int num;
{
	wordtab_t *w;

	if (ipf_yywordtab == NULL)
		return "<unknown>";

	for (w = ipf_yywordtab; w->w_word; w++)
		if (w->w_value == num)
			return w->w_word;
	return "<unknown>";
}


wordtab_t *ipf_yysettab(words)
wordtab_t *words;
{
	wordtab_t *save;

	save = ipf_yywordtab;
	ipf_yywordtab = words;
	return save;
}


void ipf_yyerror(msg)
char *msg;
{
	char *txt, letter[2];
	int freetxt = 0;

	if (ipf_yytokentype < 256) {
		letter[0] = ipf_yytokentype;
		letter[1] = '\0';
		txt =  letter;
	} else if (ipf_yytokentype == YY_STR || ipf_yytokentype == YY_HEX ||
		   ipf_yytokentype == YY_NUMBER) {
		if (ipf_yystr == NULL) {
			txt = ipf_yytexttostr(ipf_yypos, YYBUFSIZ);
			freetxt = 1;
		} else
			txt = ipf_yystr;
	} else {
		txt = ipf_yykeytostr(ipf_yytokentype);
	}
	fprintf(stderr, "%s error at \"%s\", line %d\n", msg, txt, ipf_yylineNum);
	if (freetxt == 1)
		free(txt);
	exit(1);
}


void ipf_yysetdict(newdict)
wordtab_t *newdict;
{
	if (ipf_yysavedepth == sizeof(ipf_yysavewords)/sizeof(ipf_yysavewords[0])) {
		fprintf(stderr, "%d: at maximum dictionary depth\n",
			ipf_yylineNum);
		return;
	}

	ipf_yysavewords[ipf_yysavedepth++] = ipf_yysettab(newdict);
	if (ipf_yydebug)
		printf("ipf_yysavedepth++ => %d\n", ipf_yysavedepth);
}

void ipf_yyresetdict()
{
	if (ipf_yydebug)
		printf("ipf_yyresetdict(%d)\n", ipf_yysavedepth);
	if (ipf_yysavedepth > 0) {
		ipf_yysettab(ipf_yysavewords[--ipf_yysavedepth]);
		if (ipf_yydebug)
			printf("ipf_yysavedepth-- => %d\n", ipf_yysavedepth);
	}
}



#ifdef	TEST_LEXER
int main(argc, argv)
int argc;
char *argv[];
{
	int n;

	ipf_yyin = stdin;

	while ((n = ipf_yylex()) != 0)
		printf("%d.n = %d [%s] %d %d\n",
			ipf_yylineNum, n, ipf_yystr, ipf_yypos, ipf_yylast);
}
#endif
