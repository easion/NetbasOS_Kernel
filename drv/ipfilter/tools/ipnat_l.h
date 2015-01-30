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

extern	wordtab_t	*ipnat_yysettab __P((wordtab_t *));
extern	void		ipnat_yysetdict __P((wordtab_t *));
extern	int		ipnat_yylex __P((void));
extern	void		ipnat_yyerror __P((char *));
extern	char		*ipnat_yykeytostr __P((int));
extern	void		ipnat_yyresetdict __P((void));

extern	FILE	*ipnat_yyin;
extern	int	ipnat_yylineNum;
extern	int	ipnat_yyexpectaddr;
extern	int	ipnat_yybreakondot;
extern	int	ipnat_yyvarnext;

