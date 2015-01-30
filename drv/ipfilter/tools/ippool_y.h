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
#define IPT_IPF 270
#define IPT_NAT 271
#define IPT_COUNT 272
#define IPT_AUTH 273
#define IPT_IN 274
#define IPT_OUT 275
#define IPT_TABLE 276
#define IPT_GROUPMAP 277
#define IPT_HASH 278
#define IPT_ROLE 279
#define IPT_TYPE 280
#define IPT_TREE 281
#define IPT_GROUP 282
#define IPT_SIZE 283
#define IPT_SEED 284
#define IPT_NUM 285
#define IPT_NAME 286
typedef union	{
	char	*str;
	u_32_t	num;
	struct	in_addr	addr;
	struct	alist_s	*alist;
	struct	in_addr	adrmsk[2];
	iphtent_t	*ipe;
	ip_pool_node_t	*ipp;
	union	i6addr	ip6;
} YYSTYPE;
extern YYSTYPE ippool_yylval;
