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
typedef union	{
	char	*str;
	char	**astr;
	u_32_t	num;
	struct	in_addr	ipa;
	struct	action	act;
	union	i6addr	ip6;
} YYSTYPE;
extern YYSTYPE ipscan_yylval;
