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
#define IPNY_MAPBLOCK 270
#define IPNY_RDR 271
#define IPNY_PORT 272
#define IPNY_PORTS 273
#define IPNY_AUTO 274
#define IPNY_RANGE 275
#define IPNY_MAP 276
#define IPNY_BIMAP 277
#define IPNY_FROM 278
#define IPNY_TO 279
#define IPNY_MASK 280
#define IPNY_PORTMAP 281
#define IPNY_ANY 282
#define IPNY_ROUNDROBIN 283
#define IPNY_FRAG 284
#define IPNY_AGE 285
#define IPNY_ICMPIDMAP 286
#define IPNY_PROXY 287
#define IPNY_TCP 288
#define IPNY_UDP 289
#define IPNY_TCPUDP 290
#define IPNY_STICKY 291
#define IPNY_MSSCLAMP 292
#define IPNY_TAG 293
#define IPNY_TLATE 294
#define IPNY_SEQUENTIAL 295
typedef union	{
	char	*str;
	u_32_t	num;
	struct	in_addr	ipa;
	frentry_t	fr;
	frtuc_t	*frt;
	u_short	port;
	struct	{
		u_short	p1;
		u_short	p2;
		int	pc;
	} pc;
	struct	{
		struct	in_addr	a;
		struct	in_addr	m;
	} ipp;
	union	i6addr	ip6;
} YYSTYPE;
extern YYSTYPE ipnat_yylval;
