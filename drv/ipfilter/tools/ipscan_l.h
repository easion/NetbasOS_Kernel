/*
 * Copyright (C) 2002-2004 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 */

typedef	struct	wordtab	{
	char	*w_word;
	int	w_value;
} wordtab_t;

#ifdef	NO_YACC
#define	YY_COMMENT	1000
#define	YY_CMP_NE	1001
#define	YY_CMP_LE	1002
#define	YY_RANGE_OUT	1003
#define	YY_CMP_GE	1004
#define	YY_RANGE_IN	1005
#define	YY_HEX		1006
#define	YY_NUMBER	1007
#define	YY_IPV6		1008
#define	YY_STR		1009
#define	YY_IPADDR	1010
#endif

#define	YYBUFSIZ	8192

extern	wordtab_t	*ipscan_yysettab __P((wordtab_t *));
extern	void		ipscan_yysetdict __P((wordtab_t *));
extern	int		ipscan_yylex __P((void));
extern	void		ipscan_yyerror __P((char *));
extern	char		*ipscan_yykeytostr __P((int));
extern	void		ipscan_yyresetdict __P((void));

extern	FILE	*ipscan_yyin;
extern	int	ipscan_yylineNum;
extern	int	ipscan_yyexpectaddr;
extern	int	ipscan_yybreakondot;
extern	int	ipscan_yyvarnext;

