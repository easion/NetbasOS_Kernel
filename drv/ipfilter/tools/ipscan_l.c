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
} ipscan_yylval;
#endif
#include "ipscan_l.h"
#include "ipscan_y.h"

FILE *ipscan_yyin;

#define	ishex(c)	(ISDIGIT(c) || ((c) >= 'a' && (c) <= 'f') || \
			 ((c) >= 'A' && (c) <= 'F'))
#define	TOOLONG		-3

extern int	string_start;
extern int	string_end;
extern char	*string_val;
extern int	pos;
extern int	ipscan_yydebug;

char		*ipscan_yystr = NULL;
int		ipscan_yytext[YYBUFSIZ+1];
char		ipscan_yychars[YYBUFSIZ+1];
int		ipscan_yylineNum = 1;
int		ipscan_yypos = 0;
int		ipscan_yylast = -1;
int		ipscan_yyexpectaddr = 0;
int		ipscan_yybreakondot = 0;
int		ipscan_yyvarnext = 0;
int		ipscan_yytokentype = 0;
wordtab_t	*ipscan_yywordtab = NULL;
int		ipscan_yysavedepth = 0;
wordtab_t	*ipscan_yysavewords[30];


static	wordtab_t	*ipscan_yyfindkey __P((char *));
static	int		ipscan_yygetc __P((int));
static	void		ipscan_yyunputc __P((int));
static	int		ipscan_yyswallow __P((int));
static	char		*ipscan_yytexttostr __P((int, int));
static	void		ipscan_yystrtotext __P((char *));
static	char		*ipscan_yytexttochar __P((void));

static int ipscan_yygetc(docont)
int docont;
{
	int c;

	if (ipscan_yypos < ipscan_yylast) {
		c = ipscan_yytext[ipscan_yypos++];
		if (c == '\n')
			ipscan_yylineNum++;
		return c;
	}

	if (ipscan_yypos == YYBUFSIZ)
		return TOOLONG;

	if (pos >= string_start && pos <= string_end) {
		c = string_val[pos - string_start];
		ipscan_yypos++;
	} else {
		c = fgetc(ipscan_yyin);
		if (docont && (c == '\\')) {
			c = fgetc(ipscan_yyin);
			if (c == '\n') {
				ipscan_yylineNum++;
				c = fgetc(ipscan_yyin);
			}
		}
	}
	if (c == '\n')
		ipscan_yylineNum++;
	ipscan_yytext[ipscan_yypos++] = c;
	ipscan_yylast = ipscan_yypos;
	ipscan_yytext[ipscan_yypos] = '\0';

	return c;
}


static void ipscan_yyunputc(c)
int c;
{
	if (c == '\n')
		ipscan_yylineNum--;
	ipscan_yytext[--ipscan_yypos] = c;
}


static int ipscan_yyswallow(last)
int last;
{
	int c;

	while (((c = ipscan_yygetc(0)) > '\0') && (c != last))
		;

	if (c != EOF)
		ipscan_yyunputc(c);
	if (c == last)
		return 0;
	return -1;
}


static char *ipscan_yytexttochar()
{
	int i;

	for (i = 0; i < ipscan_yypos; i++)
		ipscan_yychars[i] = (char)(ipscan_yytext[i] & 0xff);
	ipscan_yychars[i] = '\0';
	return ipscan_yychars;
}


static void ipscan_yystrtotext(str)
char *str;
{
	int len;
	char *s;

	len = strlen(str);
	if (len > YYBUFSIZ)
		len = YYBUFSIZ;

	for (s = str; *s != '\0' && len > 0; s++, len--)
		ipscan_yytext[ipscan_yylast++] = *s;
	ipscan_yytext[ipscan_yylast] = '\0';
}


static char *ipscan_yytexttostr(offset, max)
int offset, max;
{
	char *str;
	int i;

	if ((ipscan_yytext[offset] == '\'' || ipscan_yytext[offset] == '"') &&
	    (ipscan_yytext[offset] == ipscan_yytext[offset + max - 1])) {
		offset++;
		max--;
	}

	if (max > ipscan_yylast)
		max = ipscan_yylast;
	str = malloc(max + 1);
	if (str != NULL) {
		for (i = offset; i < max; i++)
			str[i - offset] = (char)(ipscan_yytext[i] & 0xff);
		str[i - offset] = '\0';
	}
	return str;
}


int ipscan_yylex()
{
	int c, n, isbuilding, rval, lnext, nokey = 0;
	char *name;

	isbuilding = 0;
	lnext = 0;
	rval = 0;

	if (ipscan_yystr != NULL) {
		free(ipscan_yystr);
		ipscan_yystr = NULL;
	}

nextchar:
	c = ipscan_yygetc(0);
	if (ipscan_yydebug > 1)
		printf("ipscan_yygetc = (%x) %c [%*.*s]\n", c, c, ipscan_yypos, ipscan_yypos, ipscan_yytexttochar());

	switch (c)
	{
	case '\n' :
		lnext = 0;
		nokey = 0;
	case '\t' :
	case '\r' :
	case ' ' :
		if (isbuilding == 1) {
			ipscan_yyunputc(c);
			goto done;
		}
		if (ipscan_yylast > ipscan_yypos) {
			bcopy(ipscan_yytext + ipscan_yypos, ipscan_yytext,
			      sizeof(ipscan_yytext[0]) * (ipscan_yylast - ipscan_yypos + 1));
		}
		ipscan_yylast -= ipscan_yypos;
		ipscan_yypos = 0;
		lnext = 0;
		nokey = 0;
		goto nextchar;

	case '\\' :
		if (lnext == 0) {
			lnext = 1;
			if (ipscan_yylast == ipscan_yypos) {
				ipscan_yylast--;
				ipscan_yypos--;
			} else
				ipscan_yypos--;
			if (ipscan_yypos == 0)
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
			ipscan_yyunputc(c);
			goto done;
		}
		ipscan_yyswallow('\n');
		rval = YY_COMMENT;
		goto done;

	case '$' :
		if (isbuilding == 1) {
			ipscan_yyunputc(c);
			goto done;
		}
		n = ipscan_yygetc(0);
		if (n == '{') {
			if (ipscan_yyswallow('}') == -1) {
				rval = -2;
				goto done;
			}
			(void) ipscan_yygetc(0);
		} else {
			if (!ISALPHA(n)) {
				ipscan_yyunputc(n);
				break;
			}
			do {
				n = ipscan_yygetc(1);
			} while (ISALPHA(n) || ISDIGIT(n) || n == '_');
			ipscan_yyunputc(n);
		}

		name = ipscan_yytexttostr(1, ipscan_yypos);		/* skip $ */

		if (name != NULL) {
			string_val = get_variable(name, NULL, ipscan_yylineNum);
			free(name);
			if (string_val != NULL) {
				name = ipscan_yytexttostr(ipscan_yypos, ipscan_yylast);
				if (name != NULL) {
					ipscan_yypos = 0;
					ipscan_yylast = 0;
					ipscan_yystrtotext(string_val);
					ipscan_yystrtotext(name);
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
			n = ipscan_yygetc(1);
			if (n == EOF || n == TOOLONG) {
				rval = -2;
				goto done;
			}
			if (n == '\n') {
				ipscan_yyunputc(' ');
				ipscan_yypos++;
			}
		} while (n != c);
		rval = YY_STR;
		goto done;
		/* NOTREACHED */

	case EOF :
		ipscan_yylineNum = 1;
		ipscan_yypos = 0;
		ipscan_yylast = -1;
		ipscan_yyexpectaddr = 0;
		ipscan_yybreakondot = 0;
		ipscan_yyvarnext = 0;
		ipscan_yytokentype = 0;
		return 0;
	}

	if (strchr("=,/;{}()@", c) != NULL) {
		if (isbuilding == 1) {
			ipscan_yyunputc(c);
			goto done;
		}
		rval = c;
		goto done;
	} else if (c == '.') {
		if (isbuilding == 0) {
			rval = c;
			goto done;
		}
		if (ipscan_yybreakondot != 0) {
			ipscan_yyunputc(c);
			goto done;
		}
	}

	switch (c)
	{
	case '-' :
		if (ipscan_yyexpectaddr)
			break;
		if (isbuilding == 1)
			break;
		n = ipscan_yygetc(0);
		if (n == '>') {
			isbuilding = 1;
			goto done;
		}
		ipscan_yyunputc(n);
		rval = '-';
		goto done;

	case '!' :
		if (isbuilding == 1) {
			ipscan_yyunputc(c);
			goto done;
		}
		n = ipscan_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_NE;
			goto done;
		}
		ipscan_yyunputc(n);
		rval = '!';
		goto done;

	case '<' :
		if (ipscan_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipscan_yyunputc(c);
			goto done;
		}
		n = ipscan_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_LE;
			goto done;
		}
		if (n == '>') {
			rval = YY_RANGE_OUT;
			goto done;
		}
		ipscan_yyunputc(n);
		rval = YY_CMP_LT;
		goto done;

	case '>' :
		if (ipscan_yyexpectaddr)
			break;
		if (isbuilding == 1) {
			ipscan_yyunputc(c);
			goto done;
		}
		n = ipscan_yygetc(0);
		if (n == '=') {
			rval = YY_CMP_GE;
			goto done;
		}
		if (n == '<') {
			rval = YY_RANGE_IN;
			goto done;
		}
		ipscan_yyunputc(n);
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
	if (ipscan_yyexpectaddr == 1 && isbuilding == 0 && (ishex(c) || c == ':')) {
		char ipv6buf[45 + 1], *s, oc;
		int start;

		start = ipscan_yypos;
		s = ipv6buf;
		oc = c;

		/*
		 * Perhaps we should implement stricter controls on what we
		 * swallow up here, but surely it would just be duplicating
		 * the code in inet_pton() anyway.
		 */
		do {
			*s++ = c;
			c = ipscan_yygetc(1);
		} while ((ishex(c) || c == ':' || c == '.') &&
			 (s - ipv6buf < 46));
		ipscan_yyunputc(c);
		*s = '\0';

		if (inet_pton(AF_INET6, ipv6buf, &ipscan_yylval.ip6) == 1) {
			rval = YY_IPV6;
			ipscan_yyexpectaddr = 0;
			goto done;
		}
		ipscan_yypos = start;
		c = oc;
	}
#endif

	if (c == ':') {
		if (isbuilding == 1) {
			ipscan_yyunputc(c);
			goto done;
		}
		rval = ':';
		goto done;
	}

	if (isbuilding == 0 && c == '0') {
		n = ipscan_yygetc(0);
		if (n == 'x') {
			do {
				n = ipscan_yygetc(1);
			} while (ishex(n));
			ipscan_yyunputc(n);
			rval = YY_HEX;
			goto done;
		}
		ipscan_yyunputc(n);
	}

	/*
	 * No negative numbers with leading - sign..
	 */
	if (isbuilding == 0 && ISDIGIT(c)) {
		do {
			n = ipscan_yygetc(1);
		} while (ISDIGIT(n));
		ipscan_yyunputc(n);
		rval = YY_NUMBER;
		goto done;
	}

	isbuilding = 1;
	goto nextchar;

done:
	ipscan_yystr = ipscan_yytexttostr(0, ipscan_yypos);

	if (ipscan_yydebug)
		printf("isbuilding %d ipscan_yyvarnext %d nokey %d\n",
		       isbuilding, ipscan_yyvarnext, nokey);
	if (isbuilding == 1) {
		wordtab_t *w;

		w = NULL;
		isbuilding = 0;

		if ((ipscan_yyvarnext == 0) && (nokey == 0)) {
			w = ipscan_yyfindkey(ipscan_yystr);
			if (w == NULL && ipscan_yywordtab != NULL) {
				ipscan_yyresetdict();
				w = ipscan_yyfindkey(ipscan_yystr);
			}
		} else
			ipscan_yyvarnext = 0;
		if (w != NULL)
			rval = w->w_value;
		else
			rval = YY_STR;
	}

	if (rval == YY_STR && ipscan_yysavedepth > 0)
		ipscan_yyresetdict();

	ipscan_yytokentype = rval;

	if (ipscan_yydebug)
		printf("lexed(%s) [%d,%d,%d] => %d @%d\n", ipscan_yystr, string_start,
			string_end, pos, rval, ipscan_yysavedepth);

	switch (rval)
	{
	case YY_NUMBER :
		sscanf(ipscan_yystr, "%u", &ipscan_yylval.num);
		break;

	case YY_HEX :
		sscanf(ipscan_yystr, "0x%x", (u_int *)&ipscan_yylval.num);
		break;

	case YY_STR :
		ipscan_yylval.str = strdup(ipscan_yystr);
		break;

	default :
		break;
	}

	if (ipscan_yylast > 0) {
		bcopy(ipscan_yytext + ipscan_yypos, ipscan_yytext,
		      sizeof(ipscan_yytext[0]) * (ipscan_yylast - ipscan_yypos + 1));
		ipscan_yylast -= ipscan_yypos;
		ipscan_yypos = 0;
	}

	return rval;
}


static wordtab_t *ipscan_yyfindkey(key)
char *key;
{
	wordtab_t *w;

	if (ipscan_yywordtab == NULL)
		return NULL;

	for (w = ipscan_yywordtab; w->w_word != 0; w++)
		if (strcasecmp(key, w->w_word) == 0)
			return w;
	return NULL;
}


char *ipscan_yykeytostr(num)
int num;
{
	wordtab_t *w;

	if (ipscan_yywordtab == NULL)
		return "<unknown>";

	for (w = ipscan_yywordtab; w->w_word; w++)
		if (w->w_value == num)
			return w->w_word;
	return "<unknown>";
}


wordtab_t *ipscan_yysettab(words)
wordtab_t *words;
{
	wordtab_t *save;

	save = ipscan_yywordtab;
	ipscan_yywordtab = words;
	return save;
}


void ipscan_yyerror(msg)
char *msg;
{
	char *txt, letter[2];
	int freetxt = 0;

	if (ipscan_yytokentype < 256) {
		letter[0] = ipscan_yytokentype;
		letter[1] = '\0';
		txt =  letter;
	} else if (ipscan_yytokentype == YY_STR || ipscan_yytokentype == YY_HEX ||
		   ipscan_yytokentype == YY_NUMBER) {
		if (ipscan_yystr == NULL) {
			txt = ipscan_yytexttostr(ipscan_yypos, YYBUFSIZ);
			freetxt = 1;
		} else
			txt = ipscan_yystr;
	} else {
		txt = ipscan_yykeytostr(ipscan_yytokentype);
	}
	fprintf(stderr, "%s error at \"%s\", line %d\n", msg, txt, ipscan_yylineNum);
	if (freetxt == 1)
		free(txt);
	exit(1);
}


void ipscan_yysetdict(newdict)
wordtab_t *newdict;
{
	if (ipscan_yysavedepth == sizeof(ipscan_yysavewords)/sizeof(ipscan_yysavewords[0])) {
		fprintf(stderr, "%d: at maximum dictionary depth\n",
			ipscan_yylineNum);
		return;
	}

	ipscan_yysavewords[ipscan_yysavedepth++] = ipscan_yysettab(newdict);
	if (ipscan_yydebug)
		printf("ipscan_yysavedepth++ => %d\n", ipscan_yysavedepth);
}

void ipscan_yyresetdict()
{
	if (ipscan_yydebug)
		printf("ipscan_yyresetdict(%d)\n", ipscan_yysavedepth);
	if (ipscan_yysavedepth > 0) {
		ipscan_yysettab(ipscan_yysavewords[--ipscan_yysavedepth]);
		if (ipscan_yydebug)
			printf("ipscan_yysavedepth-- => %d\n", ipscan_yysavedepth);
	}
}



#ifdef	TEST_LEXER
int main(argc, argv)
int argc;
char *argv[];
{
	int n;

	ipscan_yyin = stdin;

	while ((n = ipscan_yylex()) != 0)
		printf("%d.n = %d [%s] %d %d\n",
			ipscan_yylineNum, n, ipscan_yystr, ipscan_yypos, ipscan_yylast);
}
#endif
