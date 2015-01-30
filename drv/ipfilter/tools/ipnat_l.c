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
} ipnat_yylval;
#endif
#include "ipnat_l.h"
#include "ipnat_y.h"

FILE *ipnat_yyin;

#define	ishex(c)	(ISDIGIT(c) || ((c) >= 'a' && (c) <= 'f') || \
			 ((c) >= 'A' && (c) <= 'F'))
#define	TOOLONG		-3

extern int	string_start;
extern int	string_end;
extern char	*string_val;
extern int	pos;
extern int	ipnat_yydebug;

char		*ipnat_yystr = NULL;
int		ipnat_yytext[YYBUFSIZ+1];
char		ipnat_yychars[YYBUFSIZ+1];
int		ipnat_yylineNum = 1;
int		ipnat_yypos = 0;
int		ipnat_yylast = -1;
int		ipnat_yyexpectaddr = 0;
int		ipnat_yybreakondot = 0;
int		ipnat_yyvarnext = 0;
int		ipnat_yytokentype = 0;
wordtab_t	*ipnat_yywordtab = NULL;
int		ipnat_yysavedepth = 0;
wordtab_t	*ipnat_yysavewords[30];


static	wordtab_t	*ipnat_yyfindkey __P((char *));
static	int		ipnat_yygetc __P((int));
static	void		ipnat_yyunputc __P((int));
static	int		ipnat_yyswallow __P((int));
static	char		*ipnat_yytexttostr __P((int, int));
static	void		ipnat_yystrtotext __P((char *));
static	char		*ipnat_yytexttochar __P((void));

static int ipnat_yygetc(docont)
int docont;
{
	int c;

	if (ipnat_yypos < ipnat_yylast) {
		c = ipnat_yytext[ipnat_yypos++];
		if (c == '\n')
			ipnat_yylineNum++;
		return c;
	}

	if (ipnat_yypos == YYBUFSIZ)
		return TOOLONG;

	if (pos >= string_start && pos <= string_end) {
		c = string_val[pos - string_start];
		ipnat_yypos++;
	} else {
		c = fgetc(ipnat_yyin);
		if (docont && (c == '\\')) {
			c = fgetc(ipnat_yyin);
			if (c == '\n') {
				ipnat_yylineNum++;
				c = fgetc(ipnat_yyin);
			}
		}
	}
	if (c == '\n')
		ipnat_yylineNum++;
	ipnat_yytext[ipnat_yypos++] = c;
	ipnat_yylast = ipnat_yypos;
	ipnat_yytext[ipnat_yypos] = '\0';

	return c;
}


static void ipnat_yyunputc(c)
int c;
{
	if (c == '\n')
		ipnat_yylineNum--;
	ipnat_yytext[--ipnat_yypos] = c;
}


static int ipnat_yyswallow(last)
int last;
{
	int c;

	while (((c = ipnat_yygetc(0)) > '\0') && (c != last))
		;

	if (c != EOF)
		ipnat_yyunputc(c);
	if (c == last)
		return 0;
	return -1;
}


static char *ipnat_yytexttochar()
{
	int i;

	for (i = 0; i < ipnat_yypos; i++)
		ipnat_yychars[i] = (char)(ipnat_yytext[i] & 0xff);
	ipnat_yychars[i] = '\0';
	return ipnat_yychars;
}


static void ipnat_yystrtotext(str)
char *str;
{
	int len;
	char *s;

	len = strlen(str);
	if (len > YYBUFSIZ)
		len = YYBUFSIZ;

	for (s = str; *s != '\0' && len > 0; s++, len--)
		ipnat_yytext[ipnat_yylast++] = *s;
	ipnat_yytext[ipnat_yylast] = '\0';
}


static char *ipnat_yytexttostr(offset, max)
int offset, max;
{
	char *str;
	int i;

	if ((ipnat_yytext[offset] == '\'' || ipnat_yytext[offset] == '"') &&
	    (ipnat_yytext[offset] == ipnat_yytext[offset + max - 1])) {
		offset++;
		max--;
	}

	if (max > ipnat_yylast)
		max = ipnat_yylast;
	str = malloc(max + 1);
	if (str != NULL) {
		for (i = offset; i < max; i++)
			str[i - offset] = (char)(ipnat_yytext[i] & 0xff);
		str[i - offset] = '\0';
	}
	return str;
}


int ipnat_yylex()
{
	int c, n, isbuilding, rval, lnext, nokey = 0;
	char *name;

	isbuilding = 0;
	lnext = 0;
	rval = 0;

	if (ipnat_yystr != NULL) {
		free(ipnat_yystr);
		ipnat_yystr = NULL;
	}

nextchar:
	c = ipnat_yygetc(0);
	if (ipnat_yydebug > 1)
		printf("ipnat_yygetc = (%x) %c [%*.*s]\n", c, c, ipnat_yypos, ipnat_yypos, ipnat_yytexttochar());

	switch (c)
	{
	case '\n' :
		lnext = 0;
		nokey = 0;
	case '\t' :
	case '\r' :
	case ' ' :
		if (isbuilding == 1) {
			ipnat_yyunputc(c);
			goto done;
		}
		if (ipnat_yylast > ipnat_yypos) {
			bcopy(ipnat_yytext + ipnat_yypos, ipnat_yytext,
			      sizeof(ipnat_yytext[0]) * (ipnat_yylast - ipnat_yypos + 1));
		}
		ipnat_yylast -= ipnat_yypos;
		ipnat_yypos = 0;
		lnext = 0;
		nokey = 0;
		goto nextchar;

	case '\\' :
		if (lnext == 0) {
			lnext = 1;
			if (ipnat_yylast == ipnat_yypos) {
				ipnat_yylast--;
				ipnat_yypos--;
			} else
				ipnat_yypos--;
			if (ipnat_yypos == 0)
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
			ipnat_yyunputc(c);
			goto done;
		}
		ipnat_yyswallow('\n');
		rval = YY_COMMENT;
		goto done;

	case '$' :
		if (isbuilding == 1) {
			ipnat_yyunputc(c);
			goto done;
		}
		n = ipnat_yygetc(0);
		if (n == '{') {
			if (ipnat_yyswallow('}') == -1) {
				rval = -2;
				goto done;
			}
			(void) ipnat_yygetc(0);
		} else {
			if (!ISALPHA(n)) {
				ipnat_yyunputc(n);
				break;
			}
			do {
				n = ipnat_yygetc(1);
			} while (ISALPHA(n) || ISDIGIT(n) || n == '_');
			ipnat_yyunputc(n);
		}

		name = ipnat_yytexttostr(1, ipnat_yypos);		/* skip $ */

		if (name != NULL) {
			string_val = get_variable(name, NULL, ipnat_yylineNum);
			free(name);
			if (string_val != NULL) {
				name = ipnat_yytexttostr(ipnat_yypos, ipnat_yylast);
				if (name != NULL) {
					ipnat_yypos = 0;
					ipnat_yylast = 0;
					ipnat_yystrtotext(string_val);
					ipnat_yystrtotext(name);
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
			n = ipnat_yygetc(1);
			if (n == EOF || n == TOOLONG) {
				rval = -2;
				goto done;
			}
			if (n == '\n') {
				ipnat_yyunputc(' ');
				ipnat_yypos++;
			}
		} while (n != c);
		rval = YY_STR;
		goto done;
		/* NOTREACHED */

	case EOF :
		ipnat_yylineNum = 1;
		ipnat_yypos = 0;
		ipnat_yylast = -1;
		ipnat_yyexpectaddr = 0;
		ipnat_yybreakondot = 0;
		ipnat_yyvarnext = 0;
		ipnat_yytokentype = 0;
		return 0;
	}

	if (strchr("=,/;{}()@", c) != NULL) {
		if (isbuilding == 1) {
			ipnat_yyunputc(c);
			goto done;
		}
		rval = c;
		goto done;
	} else if (c == '.') {
		if (isbuilding == 0) {
			rval = c;
			goto done;
		}
		if (ipnat_yybreakondot != 0) {
			ipnat_yyunputc(c);
			goto done;
		}
	}

	switch (c)
	{
	case '-' :
		if (ipnat_yyexpectaddr)
			break;
		if (isbuilding == 1)
			break;
		n = ipnat_yygetc(0);
		if (n == '>') {
			isbuilding = 1;
			goto done;
		}
		ipnat_yyunputc(n);
		rval = '-';
		goto done;

	case '!' :
		if (isbuilding == 1) {
			ipnat_yyunputc(c);
			goto done;
		}
		n = ipnat_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_NE;
			goto done;
		}
		ipnat_yyunputc(n);
		rval = '!';
		goto done;

	case '<' :
		if (ipnat_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipnat_yyunputc(c);
			goto done;
		}
		n = ipnat_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_LE;
			goto done;
		}
		if (n == '>') {
			rval = YY_RANGE_OUT;
			goto done;
		}
		ipnat_yyunputc(n);
		rval = YY_CMP_LT;
		goto done;

	case '>' :
		if (ipnat_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipnat_yyunputc(c);
			goto done;
		}
		n = ipnat_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_GE;
			goto done;
		}
		if (n == '<') {
			rval = YY_RANGE_IN;
			goto done;
		}
		ipnat_yyunputc(n);
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
	if (ipnat_yyexpectaddr == 1 && isbuilding == 0 && (ishex(c) || c == ':')) {
		char ipv6buf[45 + 1], *s, oc;
		int start;

		start = ipnat_yypos;
		s = ipv6buf;
		oc = c;

		/*
		 * Perhaps we should implement stricter controls on what we
		 * swallow up here, but surely it would just be duplicating
		 * the code in inet_pton() anyway.
		 */
		do {
			*s++ = c;
			c = ipnat_yygetc(1);
		} while ((ishex(c) || c == ':' || c == '.') &&
			 (s - ipv6buf < 46));
		ipnat_yyunputc(c);
		*s = '\0';

		if (inet_pton(AF_INET6, ipv6buf, &ipnat_yylval.ip6) == 1) {
			rval = YY_IPV6;
			ipnat_yyexpectaddr = 0;
			goto done;
		}
		ipnat_yypos = start;
		c = oc;
	}
#endif

	if (c == ':') {
		if (isbuilding == 1) {
			ipnat_yyunputc(c);
			goto done;
		}
		rval = ':';
		goto done;
	}

	if (isbuilding == 0 && c == '0') {
		n = ipnat_yygetc(0);
		if (n == 'x') {
			do {
				n = ipnat_yygetc(1);
			} while (ishex(n));
			ipnat_yyunputc(n);
			rval = YY_HEX;
			goto done;
		}
		ipnat_yyunputc(n);
	}

	/*
	 * No negative numbers with leading - sign..
	 */
	if (isbuilding == 0 && ISDIGIT(c)) {
		do {
			n = ipnat_yygetc(1);
		} while (ISDIGIT(n));
		ipnat_yyunputc(n);
		rval = YY_NUMBER;
		goto done;
	}

	isbuilding = 1;
	goto nextchar;

done:
	ipnat_yystr = ipnat_yytexttostr(0, ipnat_yypos);

	if (ipnat_yydebug)
		printf("isbuilding %d ipnat_yyvarnext %d nokey %d\n",
		       isbuilding, ipnat_yyvarnext, nokey);
	if (isbuilding == 1) {
		wordtab_t *w;

		w = NULL;
		isbuilding = 0;

		if ((ipnat_yyvarnext == 0) && (nokey == 0)) {
			w = ipnat_yyfindkey(ipnat_yystr);
			if (w == NULL && ipnat_yywordtab != NULL) {
				ipnat_yyresetdict();
				w = ipnat_yyfindkey(ipnat_yystr);
			}
		} else
			ipnat_yyvarnext = 0;
		if (w != NULL)
			rval = w->w_value;
		else
			rval = YY_STR;
	}

	if (rval == YY_STR && ipnat_yysavedepth > 0)
		ipnat_yyresetdict();

	ipnat_yytokentype = rval;

	if (ipnat_yydebug)
		printf("lexed(%s) [%d,%d,%d] => %d @%d\n", ipnat_yystr, string_start,
			string_end, pos, rval, ipnat_yysavedepth);

	switch (rval)
	{
	case YY_NUMBER :
		sscanf(ipnat_yystr, "%u", &ipnat_yylval.num);
		break;

	case YY_HEX :
		sscanf(ipnat_yystr, "0x%x", (u_int *)&ipnat_yylval.num);
		break;

	case YY_STR :
		ipnat_yylval.str = strdup(ipnat_yystr);
		break;

	default :
		break;
	}

	if (ipnat_yylast > 0) {
		bcopy(ipnat_yytext + ipnat_yypos, ipnat_yytext,
		      sizeof(ipnat_yytext[0]) * (ipnat_yylast - ipnat_yypos + 1));
		ipnat_yylast -= ipnat_yypos;
		ipnat_yypos = 0;
	}

	return rval;
}


static wordtab_t *ipnat_yyfindkey(key)
char *key;
{
	wordtab_t *w;

	if (ipnat_yywordtab == NULL)
		return NULL;

	for (w = ipnat_yywordtab; w->w_word != 0; w++)
		if (strcasecmp(key, w->w_word) == 0)
			return w;
	return NULL;
}


char *ipnat_yykeytostr(num)
int num;
{
	wordtab_t *w;

	if (ipnat_yywordtab == NULL)
		return "<unknown>";

	for (w = ipnat_yywordtab; w->w_word; w++)
		if (w->w_value == num)
			return w->w_word;
	return "<unknown>";
}


wordtab_t *ipnat_yysettab(words)
wordtab_t *words;
{
	wordtab_t *save;

	save = ipnat_yywordtab;
	ipnat_yywordtab = words;
	return save;
}


void ipnat_yyerror(msg)
char *msg;
{
	char *txt, letter[2];
	int freetxt = 0;

	if (ipnat_yytokentype < 256) {
		letter[0] = ipnat_yytokentype;
		letter[1] = '\0';
		txt =  letter;
	} else if (ipnat_yytokentype == YY_STR || ipnat_yytokentype == YY_HEX ||
		   ipnat_yytokentype == YY_NUMBER) {
		if (ipnat_yystr == NULL) {
			txt = ipnat_yytexttostr(ipnat_yypos, YYBUFSIZ);
			freetxt = 1;
		} else
			txt = ipnat_yystr;
	} else {
		txt = ipnat_yykeytostr(ipnat_yytokentype);
	}
	fprintf(stderr, "%s error at \"%s\", line %d\n", msg, txt, ipnat_yylineNum);
	if (freetxt == 1)
		free(txt);
	exit(1);
}


void ipnat_yysetdict(newdict)
wordtab_t *newdict;
{
	if (ipnat_yysavedepth == sizeof(ipnat_yysavewords)/sizeof(ipnat_yysavewords[0])) {
		fprintf(stderr, "%d: at maximum dictionary depth\n",
			ipnat_yylineNum);
		return;
	}

	ipnat_yysavewords[ipnat_yysavedepth++] = ipnat_yysettab(newdict);
	if (ipnat_yydebug)
		printf("ipnat_yysavedepth++ => %d\n", ipnat_yysavedepth);
}

void ipnat_yyresetdict()
{
	if (ipnat_yydebug)
		printf("ipnat_yyresetdict(%d)\n", ipnat_yysavedepth);
	if (ipnat_yysavedepth > 0) {
		ipnat_yysettab(ipnat_yysavewords[--ipnat_yysavedepth]);
		if (ipnat_yydebug)
			printf("ipnat_yysavedepth-- => %d\n", ipnat_yysavedepth);
	}
}



#ifdef	TEST_LEXER
int main(argc, argv)
int argc;
char *argv[];
{
	int n;

	ipnat_yyin = stdin;

	while ((n = ipnat_yylex()) != 0)
		printf("%d.n = %d [%s] %d %d\n",
			ipnat_yylineNum, n, ipnat_yystr, ipnat_yypos, ipnat_yylast);
}
#endif
