/*
Fear 2.0 map to bitmap converter
Code by Rameti
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int c_line;
char *c_file;


#define SMBUFSIZE 4096
int sm_fd;
unsigned char sm_buffer[ SMBUFSIZE ];
unsigned char *sm_cur;
unsigned char *sm_lim;

#define MAX_X 1500
#define MAX_Y 1000
int map[MAX_Y * MAX_X];


#define sm_eof 256


int smxfatal(int status)
{
	exit(status);
}



int sm_getc(void)
{
	int res;
	if(sm_cur < sm_lim)
		return *(unsigned char *)sm_cur++;
	
reload:
	res = read(sm_fd, sm_buffer, SMBUFSIZE);
	if(res == 0)
		return sm_eof;
	if(res == -1)
	{
		if(errno == EINTR)
			goto reload;
		fprintf(stderr,"%s:%i Read error %s\n", c_file, c_line, strerror(errno));
		smxfatal(1);
	}
	sm_cur = sm_buffer;
	sm_lim = sm_cur + res;
	return *(unsigned char *)sm_cur++;
}


#define smxgetc() ( (sm_cur < sm_lim) ? *sm_cur++ : sm_getc() )
#define smxungetc(ch) (*(--sm_cur) = (ch))

#define MAXIDSTR 256
char sm_idstr[MAXIDSTR];
char *sm_idcur;
long sm_value;

#define MAXSTRLEN 4096
char sm_str[MAXSTRLEN];
char *sm_curstr;


enum
{
	smx_error=-1,
	smx_eof=0,
	smx_idtag,
	smx_string,
	smx_semicolon,
	smx_period,
	smx_set,
	smx_cmpeq,
	smx_value,
	smx_setmul,
	smx_setdiv,
	smx_setpower,
	smx_mul,
	smx_div,
	smx_power,
	smx_ptrderef,
};

char *dbsmx[] =
{
	"eof",
	"idtag",
	"string",
	"semicolon",
	"period",
	"set",
	"cmpeq",
	"value",
	"setmul",
	"setdiv",
	"setpower",
	"mul",
	"div",
	"power",
	"ptrderef"
};



#define smxtodigit(c)    ((c) - '0')
#define smxisdigit(c) ((unsigned)smxtodigit(c) <= 9)

#define smxlctodigit(c)    ((c) - 'a')
#define smxuctodigit(c)    ((c) - 'A')

#define smxlcisdigit(c) ((unsigned)smxlctodigit(c) <= 25)
#define smxucisdigit(c) ((unsigned)smxuctodigit(c) <= 25)

#define smxtoxdigit(c) (smxisdigit(c) ? smxtodigit(c) : smxlcisdigit(c) ? smxlctodigit(c)+10 : smxucisdigit(c) ? smxuctodigit(c)+10 : -1)

enum
{
	asc_nul,
	asc_soh,
	asc_stx,
	asc_etx,
	asc_eot,
	asc_enq,
	asc_ack,
	asc_bel,
	asc_bs,
	asc_ht,
	asc_nl,
	asc_vt,
	asc_np,
	asc_cr,
	asc_so,
	asc_si,
	asc_dle,
	asc_dc1,
	asc_dc2,
	asc_dc3,
	asc_dc4,
	asc_nak,
	asc_syn,
	asc_etb,
	asc_can,
	asc_em,
	asc_sub,
	asc_esc,
	asc_fs,
	asc_gs,
	asc_rs,
	asc_us,
	asc_sp
};

/*
	000 nul  001 soh  002 stx  003 etx  004 eot  005 enq  006 ack  007 bel
     010 bs   011 ht   012 nl   013 vt   014 np   015 cr   016 so   017 si
     020 dle  021 dc1  022 dc2  023 dc3  024 dc4  025 nak  026 syn  027 etb
     030 can  031 em   032 sub  033 esc  034 fs   035 gs   036 rs   037 us
     040 sp   041  !   042  "   043  #   044  $   045  %   046  &   047  '
     050  (   051  )   052  *   053  +   054  ,   055  -   056  .   057  /
     060  0   061  1   062  2   063  3   064  4   065  5   066  6   067  7
     070  8   071  9   072  :   073  ;   074  <   075  =   076  >   077  ?
     100  @   101  A   102  B   103  C   104  D   105  E   106  F   107  G
     110  H   111  I   112  J   113  K   114  L   115  M   116  N   117  O
     120  P   121  Q   122  R   123  S   124  T   125  U   126  V   127  W
     130  X   131  Y   132  Z   133  [   134  \   135  ]   136  ^   137  _
     140     141  a   142  b   143  c   144  d   145  e   146  f   147  g
     150  h   151  i   152  j   153  k   154  l   155  m   156  n   157  o
     160  p   161  q   162  r   163  s   164  t   165  u   166  v   167  w
     170  x   171  y   172  z   173  {   174  |   175  }   176  ~   177 del
*/                              

int smxlexescape(void)
{
	int tnum;
	int num;
	int ch;

	num = 0;
	ch = smxgetc();
	/* accellerate some escapes */
	switch(ch)
	{
		case '\\':
			return '\\';
		case 'n':
			return asc_nl;
		case 'r':
			return asc_cr;
		case 'f':
			return asc_np;
		case 'b':
			return asc_bs;
		case 'a':
			return asc_bel;
		case 'v':
			return asc_vt;
		case 'e':
			return asc_esc;
	}
		

	if(ch == 'x')
	{
		ch = smxgetc();
		if(ch == sm_eof)
			return sm_eof;

		num = smxtoxdigit(ch);
		if(num == -1 || num > 15)
		{
			smxungetc(ch);
			return 'x';
		}

		for(;;)
		{
			ch = smxgetc();
			if(ch == sm_eof)
				return sm_eof;
		
			tnum = smxtoxdigit(ch);
			if(tnum == -1 || tnum > 15)
			{
				smxungetc(ch);
				return (unsigned char)num;
			}
			num *= 16;
			num += tnum;
		}
	}
	
	if(ch >= '0' && ch <= '7')
	{
		num = ch - '0';
		for(;;)
		{
			ch = smxgetc();
			if(ch == sm_eof)
				return sm_eof;
		
			tnum = smxtodigit(ch);
			if(tnum == -1 || tnum > 7)
			{
				smxungetc(ch);
				return (unsigned char)num;
			}
			num *= 8;
			num += tnum;
		}
	}
	
	

	return ch;
}

int smxlex(void)
{
	int ch;

smxnext:
	ch = smxgetc();
	if(ch == sm_eof)
		return 0;
		
	/* ignore whitespace */
	if(ch == '\n')
	{
		c_line++;
		goto smxnext;
	}
	if(ch == ' ' || ch == '\r' || ch == '\t')
		goto smxnext;
		
	/* check identifier */
	if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' || ch == '#')
	{
		sm_idcur = sm_idstr;
		do
		{
			if(sm_idcur >= (sm_idstr + MAXIDSTR))
				goto idstroverflow;
			*sm_idcur++ = ch;
			ch = smxgetc();
		} while((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || ch == '_' || (ch >= '0' && ch <= '9'));
		if(ch != sm_eof)
			smxungetc(ch);
		if(sm_idcur >= (sm_idstr + MAXIDSTR))
			goto idstroverflow;
		*sm_idcur = '\0';
		return smx_idtag;
idstroverflow:
		fprintf(stderr, "%s:%i\n Identifier too long, max %d\n", c_file, c_line, MAXIDSTR);
		return smx_error;				
	}
	if(ch == ';')
		return smx_semicolon;
	if(ch == '.')
		return smx_period;
	
	
	if((ch >= '0' && ch <= '9'))
	{
		sm_value = ch - '0';
		for(;;)
		{
			ch = smxgetc();
			if(ch >= '0' && ch <= '9')
			{
				sm_value *= 10;
				sm_value += ch - '0';
				continue;
			}
			if(ch != sm_eof)
				smxungetc(ch);
			return smx_value;
		}
	}
//handle * *= ** **=

	if(ch == '*')
	{
		ch = smxgetc();
		if(ch == '*')
		{
			ch = smxgetc();
			if(ch == '=')
				return smx_setpower;
			if(ch != sm_eof)
				smxungetc(ch);
			return smx_power;
		}
		if(ch == '=')
			return smx_setmul;

		if(ch == '(' || ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z')))
		{
			smxungetc(ch);
			return smx_ptrderef;
		}
		if(ch != sm_eof)
			smxungetc(ch);
		return smx_mul;
	}
//handle / /* // /=

	if(ch == '/')
	{
		ch = smxgetc();
		if(ch == sm_eof)
			return 0;
		if(ch == '*')
		{
			for(;;)
			{
				ch = smxgetc();
				if(ch == '\n')
					c_line++;
				if(ch == sm_eof)
					goto err_eofincommentblk;
				if(ch == '*')
				{
					ch = smxgetc();
					if(ch == sm_eof)
						goto err_eofincommentblk;
					if(ch == '/')
						break;
					if(ch == '\n')
						c_line++;
				}
			}
			goto smxnext;
		}
		if(ch == '/')
		{
			for(;;)
			{
				ch = smxgetc();
				if(ch == '\n')
				{
					c_line++;
					goto smxnext;
				}
				if(ch == sm_eof)
					return smx_eof;
			}
		}
		if(ch == '=')
			return smx_setdiv;
		if(ch != sm_eof)
			smxungetc(ch);
		return smx_div;
	}
//handle = ==
	if(ch == '=')
	{
		ch = smxgetc();
		if(ch == '=')
			return smx_cmpeq;
		smxungetc(ch);
		return smx_set;
	
	}
//handle quoted strings
	if(ch == '"')
	{
		sm_curstr = sm_str;
		for(;;)
		{
			ch = smxgetc();
			if(ch == sm_eof)
				goto quotestreoferror;
			if(ch == '\n')
				c_line++;
			if(ch == '"')
				break;
			if(ch == '\\')
			{
				ch = smxlexescape();
				if(ch == sm_eof)
					goto quotestreoferror;
			}
			if(sm_curstr >= (sm_str + MAXSTRLEN))
				goto quotestroverflow;
			*sm_curstr++ = ch;
		}
		if(sm_curstr >= (sm_str + MAXSTRLEN))
			goto quotestroverflow;
		*sm_curstr = '\0';
		
		return smx_string;
quotestreoferror:
		fprintf(stderr, "%s:%i End of file inside string\n", c_file, c_line);
		return smx_error;		
quotestroverflow:
		fprintf(stderr, "%s:%i String lenth exceeds maximum lenth of %i\n", MAXSTRLEN);
		return smx_error;
	}

//handle anything unable to be resolved
	fprintf(stderr, "%s:%i Syntax error\n", c_file, c_line);
	return -1;
err_eofincommentblk:
	fprintf(stderr, "%s:%i End of file while in comment block\n", c_file, c_line);
	return -1;


}


typedef signed char bm8;
typedef unsigned char bmu8;

typedef signed short bm16;
typedef unsigned short bmu16;

typedef signed long bm32;
typedef unsigned long bmu32;

#define wrcachelen 4096
char wrcache[wrcachelen];
char *wrdest = wrcache;
#define wrcacheeod (wrcache + wrcachelen)

int wrrawnocache(int fd, void *buf, int length)
{
	int res;
	char *src;
	char *eos;
	src = (char *)buf;
	eos = src + length;
	for(;;)
	{
		if(src >= eos)
			break;
		res = write(fd, buf, eos - src);
		if(res == -1)
		{
			perror("write file");
			return 1;
		}
		src += res;
	}
	return 0;
}

int wrflush(int fd)
{
	int res;
	res = 0;
	if(wrdest > wrcache)
	{
		res = wrrawnocache(fd, wrcache, wrdest - wrcache);
		wrdest = wrcache;
	}
	return res;
}

int wrraw(int fd, void *buf, int length)
{
	int res;
	char *src;
	char *eos;
	src = (char *)buf;
	eos = src + length;
	for(;;)
	{
		if(src >= eos)
			break;
		
		if(wrdest > wrcacheeod)
		{
			res = wrflush(fd);
			if(res) return res;
		}
		*wrdest++ = *src++;
	}
	return 0;
}


int wrbm8(int fd, bm8 value)
{
	return wrraw(fd, &value, sizeof(value));
}

int wrbmu8(int fd, bmu8 value)
{
	return wrraw(fd, &value, sizeof(value));
}

int wrbm16(int fd, bm16 value)
{
	return wrraw(fd, &value, sizeof(value));
}

int wrbmu16(int fd, bmu16 value)
{
	return wrraw(fd, &value, sizeof(value));
}


int wrbm32(int fd, bm32 value)
{
	return wrraw(fd, &value, sizeof(value));
}

int wrbmu32(int fd, bmu32 value)
{
	return wrraw(fd, &value, sizeof(value));
}


unsigned char cpal[] =
{
	0x00,0x00,0x00,0x00,
	0x80,0x00,0x00,0x00,
	0x00,0x80,0x00,0x00,
	0x80,0x80,0x00,0x00,
	0x00,0x00,0x80,0x00,
	0x80,0x00,0x80,0x00,
	0x00,0x80,0x80,0x00,
	0xc0,0xc0,0xc0,0x00,
	0x80,0x80,0x80,0x00,
	0xff,0x00,0x00,0x00,
	0x00,0xff,0x00,0x00,
	0xff,0xff,0x00,0x00,
	0x00,0x00,0xff,0x00,
	0xff,0x00,0xff,0x00,
	0x00,0xff,0xff,0x00,
	0xff,0xff,0xff,0x00,
};

//#define MAKETAB
#undef MAKETAB



#ifdef MAKETAB
unsigned char scolor[256] =
{

};
#else
unsigned char scolor[] = 
{
	15,15,10,2,10,3,14,6,12,14,11,0,4,3,7,15,
	11,9,11,7,2,7,7,4,12,7,3,4,7,10,12,2,
	11,15,15,4,9,10,7,3,7,15,15,2,2,3,7,14,
	0,7,9,1,7,7,11,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
	10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10
};
#endif

enum
{
	bp_red        = 1<<0,
	bp_green      = 1<<1,
	bp_blue       = 1<<2,
	bp_bold       = 1<<3,
	bp_bred       = 1<<4,
	bp_bgreen     = 1<<5,
	bp_bblue      = 1<<6,
	bp_blink      = 1<<7,
	at_colors     = 1<<8,
	bm_rgb        = bp_red | bp_green | bp_blue,	
	bm_brgb       = bp_bred | bp_bgreen | bp_bblue,

	at_default    = -1,
	at_black      = 0,
	at_red        = bp_red,
	at_green      = bp_green,
	at_brown      = bp_green | bp_red,
	at_blue       = bp_blue,
	at_magenta    = bp_blue | bp_red,
	at_cyan       = bp_blue | bp_green,
	at_white      = bp_blue | bp_green | bp_red,
	at_grey       = bp_bold,
	at_lt_red     = bp_bold | at_red,
	at_lt_green   = bp_bold | at_green,
	at_yellow     = bp_bold | at_brown,
	at_lt_blue    = bp_bold | at_blue,
	at_lt_magenta = bp_bold | at_magenta,
	at_lt_cyan    = bp_bold | at_cyan,
	at_hi_white   = bp_bold | at_white,
	at_st_green   = bp_bold | at_green | at_cyan
};


int clparse(char *fmt)
{
	char *cp;
	char ch;
	int col;
	col = 0;

	col = at_white;
	for(;;)
	{
		for(cp = fmt; (ch = *fmt) && ch != '&'; fmt++);
		if(ch == '\0')
			break;
		fmt++; /* skip the '&' */
		ch = *fmt++;
		if(ch == '\0')
			break;
		switch(ch)
		{
			case 'x':
				col = at_black;
				break;
			case 'g':
				col = at_green;
				break;
			case 'b':
				col = at_blue;
				break;
			case 'c':
				col = at_cyan;
				break;
			case 'z':
				col = at_white;
				break;
			case 'G':
				col = at_lt_green;
				break;
			case 'B':
				col = at_lt_blue;
				break;
			case 'C':
				col = at_lt_cyan;
				break;
		        case 'E':
		                col = at_st_green;
		                break;
			case 'r':
				col = at_red;
				break;
			case 'O':
				col = at_brown;
				break;
			case 'p':
				col = at_magenta;
				break;
			case 'w':
				col = at_white;
				break;
			case 'R':
				col = at_lt_red;
				break;
			case 'Y':
				col = at_yellow;
				break;
			case 'P':
				col = at_lt_magenta;
				break;
			case 'W':
				col = at_hi_white;
				break;
		}
	}
	return col;
}


#define MAX_MAPSECTOR 55

typedef enum
{
  SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
  SECT_WATER_SWIM, SECT_WATER_NOSWIM, SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
  SECT_DUNNO, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_ROAD, SECT_ENTER,
  SECT_MINEGOLD, SECT_MINEIRON, SECT_HCORN, SECT_HGRAIN, SECT_STREE, SECT_NTREE,
  SECT_SGOLD, SECT_NGOLD, SECT_SIRON, SECT_NIRON, SECT_SCORN, SECT_NCORN,
  SECT_SGRAIN, SECT_NGRAIN, SECT_RIVER, SECT_JUNGLE, SECT_SHORE, SECT_TUNDRA,
  SECT_ICE, SECT_OCEAN, SECT_LAVA, SECT_TREE, SECT_NOSTONE, SECT_QUICKSAND,
  SECT_WALL, SECT_GLACIER, SECT_EXIT, SECT_SWAMP, SECT_PATH, SECT_PLAINS,
  SECT_PAVE, SECT_BRIDGE, SECT_VOID, SECT_STABLE, SECT_FIRE, SECT_BURNT, 
  SECT_STONE, SECT_SSTONE, SECT_NSTONE, SECT_MAX
} sector_types;



char *ns(int sector)
{
	switch(sector)
	{
		case SECT_INSIDE:   	return "&G&W%";
		case SECT_CITY:		return "&G&W#";
		case SECT_FIELD:		return "&G\"";
		case SECT_FOREST:		return "&E@";
		case SECT_HILLS:		return "&G^^";
		case SECT_MOUNTAIN:		return "&O^^";
		case SECT_WATER_SWIM:	return "&C~";  
		case SECT_WATER_NOSWIM:	return "&c~";
		case SECT_UNDERWATER: 	return "&B~";
		case SECT_AIR:			return "&C%";
		case SECT_DESERT:		return "&Y=";
		case SECT_DUNNO:		return "&x?";
		case SECT_OCEANFLOOR:	return "&b~";
		case SECT_UNDERGROUND:	return "&O#";
		case SECT_ROAD:		return "&c&w+";
		case SECT_ENTER:		return "&G&W#";
		case SECT_MINEGOLD:		return "&Y^^";
		case SECT_MINEIRON:		return "&R^^";
		case SECT_HCORN:		return "&Y\"";
		case SECT_HGRAIN:		return "&z\"";
		case SECT_STREE:		return "&g@";
		case SECT_NTREE:		return "&c&w@";
		case SECT_SGOLD:		return "&c&w^^";
		case SECT_NGOLD:		return "&b^^";
		case SECT_SIRON:		return "&B^^";
		case SECT_NIRON:		return "&z^^";
		case SECT_SCORN:		return "&O\"";
		case SECT_NCORN:		return "&b\"";
		case SECT_SGRAIN:		return "&z\"";
		case SECT_NGRAIN:		return "&G\"";
		case SECT_RIVER:		return "&B-";
		case SECT_JUNGLE:		return "&g*";
		case SECT_SHORE:		return "&Y.";
		case SECT_TUNDRA:		return "&G&W+";
		case SECT_ICE:			return "&G&WI";
		case SECT_OCEAN:		return "&b~";
		case SECT_LAVA:		return "&R:";
		case SECT_TREE:		return "&G*";
		case SECT_NOSTONE:		return "&c&w^^";
		case SECT_QUICKSAND:	return "&O%";
		case SECT_WALL:		return "&c&wI";
		case SECT_GLACIER:		return "&G&W=";
		case SECT_EXIT:		return "&G&W#";
		case SECT_SWAMP:		return "&g%";
		case SECT_PATH:		return "&g+";
		case SECT_PLAINS:		return "&O~";
		case SECT_PAVE:		return "&z#";
		case SECT_BRIDGE:		return "&C=";
		case SECT_VOID:		return "&x ";
		case SECT_STABLE:		return "&c&w#";
		case SECT_FIRE:		return "&R#";
		case SECT_BURNT:		return "&r+";
		case SECT_STONE:		return "&c&w*";
		case SECT_SSTONE:		return "&z*";
		case SECT_NSTONE:		return "&Y*";
		default:			return "&G?";
	}
}

int wrbmp(int fd)
{
	int p;
	unsigned char fg;
	unsigned char fred;
	unsigned char fgreen;
	unsigned char fblue;
	unsigned char *lc;
	int x;
	int y;
#define BMFHSIZE 14
#define BMIHSIZE 40
	/* BMFH */
	int res;
	res = wrbmu16(fd, ('B' | 'M' <<8)); /* bfType */
	if(res)return res;
	res = wrbmu32(fd, (MAX_Y * MAX_X) + BMFHSIZE + BMIHSIZE + (256 * 4)); /* bfSize */
	if(res) return res;
	res = wrbmu16(fd, 0); /* bfReserved1 */
	if(res) return res;	
	res = wrbmu16(fd, 0); /* bfReserved2 */
	if(res) return res;
	res = wrbmu32(fd, BMFHSIZE + BMIHSIZE + (256 * 4 )); /* bfOffBits */
	if(res) return res;
	/* BMIH */
	res = wrbmu32(fd, BMIHSIZE); /* biSize */
	if(res) return res;
	res = wrbm32(fd, MAX_X); /* biWidth */
	if(res) return res;
	res = wrbm32(fd, MAX_Y); /* biHeight */
	if(res) return res;
	res = wrbm16(fd, 1); /* biPlanes */
	if(res) return res;
	res = wrbm16(fd, 8); /* biBitCount */
	if(res) return res;
	res = wrbmu32(fd, 0); /* biCompression */
	if(res) return res;
	res = wrbmu32(fd, 0); /* biSizeImage */
	if(res) return res;
	res = wrbm32(fd, 3780); /* biXPelsPerMeter */
	if(res) return res;
	res = wrbm32(fd, 3780); /* biYpelsPerMeter */
	if(res) return res;
	res = wrbmu32(fd, 0); /* biClrUsed */
	if(res) return res;
	res = wrbmu32(fd, 0); /* biClrImportant */
	if(res) return res;
	for(p = 0; p < 256; p++)
	{
		fg = p & 0xf;
		lc = cpal + (4 * fg);
		fred = *lc++;
		fgreen = *lc++;
		fblue = *lc++;
#define RGB(r,g,b) ((bmu32)(((bmu8)(r)|((bmu16)(g)<<8))| (((bmu32)(bmu8)(b))<<16)))
    
#if RGB
		res = wrbmu32(fd, RGB(fred,fgreen,fblue));
		if(res) return res;
#else
		res = wrbmu8(fd, fblue); /* blue */
		if(res) return res;
		res = wrbmu8(fd, fgreen); /* green */
		if(res) return res;
		res = wrbmu8(fd, fred); /* red */
		if(res) return res;
		res = wrbmu8(fd, 0); /* reserved */
		if(res) return res;
#endif
	}
	
	for(y = MAX_Y - 1; y >= 0; y--)
	{
		for(x = 0; x < MAX_X; x++)
		{
			res = wrbmu8(fd, scolor[map[y * MAX_X + x]]);
			if(res) return res;
		}
	}
	res = wrflush(fd);
	if(res) return res;
	return 0;	
}






int main(int argc, char **argv)
{
	int bmfd;
	int res;
	int clsrc;
	int x;
	int y;
	int cnt;
	int inmap;
	
	int mapstate;
	int startx;
	int starty;
	int endx;
	int endy;
	int sector;
	char *src;
	
#ifdef MAKETAB
	int p;
	int col;
	printf("unsigned char scolor[] = \n{\n	");
	col = 0;
//	for(p = 0; p < MAX_MAPSECTOR; p++)
	for(p = 0; p < 256; p++, col++)
	{
		src = ns(p);
//		printf("%i %i %s\n", p, clparse(src), src);
		scolor[p] = clparse(src);
		if(col > 15)
		{
			printf("\n	");
			col = 0;
		}
		printf("%i,", clparse(src));
	}
	printf("\n};\n");
#endif
	
	if(argc < 3)
	{
		fprintf(stderr, "syntax: map2bmp <file.map> <file.bmp>\n");
		return 1;
	}
	
	c_file = argv[1];
	c_line = 1;
	sm_fd = open(c_file, O_RDONLY);
	if(sm_fd == -1)
	{
		perror("open source file");
		return 1;
	}
	
	inmap = 0;
	x = y = 0;
	for(;;)
	{
		res = smxlex();
		if(res == smx_error)
		{
			fprintf(stderr, "error\n");
			goto failed;
		}
		if(res == smx_eof)
			break;

		if(res == smx_idtag)
		{
			int preproc;
			
			src = sm_idstr;
			if(*src == '#')
			{
				preproc = 1;
				src++;
			}
			else
				preproc = 0;
			
			if(!strcasecmp("map", src))
			{
//				printf("map\n");
				memset(map, 0, sizeof(map));
				cnt = 0;
				y = x = 0;
				inmap = 1;
				mapstate = 0;
				continue;
			}
			if(!strncasecmp("end", src))
			{
//				printf("end\n");
				break;
			}
			fprintf(stderr, "Undefined map directive %s\n", src);
			continue;
		}

		if(inmap)
		{
			if(res == smx_value)
			{
				switch(mapstate)
				{
					case 0:
						mapstate++;
						startx = sm_value;
						continue;
					case 1:
						mapstate++;
						starty = sm_value;
						continue;
					case 2:
						mapstate++;
						endx = sm_value;
						continue;
					case 3:
						mapstate++;
						endy = sm_value;
						continue;
					case 4:
						mapstate = 0;
						sector = sm_value;
						break;
				}
				/* got a full set */
				startx--;
				endx--;
				starty--;
				endy--;
#if 0
				if(startx < 0 || endx < 0 || starty < 0 || endy < 0 || sector < 0
				|| startx > MAX_X || endx > MAX_X || starty > MAX_Y || endy > MAX_Y
				|| startx > endx || starty > endy
				|| sector > SECT_MAX)
				{
					fprintf(stderr, "bad meta data in map %i %i %i %i %i\n", startx, starty, endx, endy, sector);
					goto failed;
				}
#endif
				for(x = startx; x <= endx; x++)
				{
					y = (x == startx) ? starty : 1;
					for(;;y++)
					{
						if(x == endx && y > endy || y == MAX_Y)
							break;
						map[ y * MAX_X + x] = sector;
					
					}
				
				}
#if 0
				for(y = starty; y < endy; y++)
				{
					for(x = startx; x < endx; x++)
					{
						printf("adding %i %i %i\n", x, y, sector);
						map[y * MAX_X + x] = sector;
					}
				}
#endif
			}
			else
			{
				fprintf(stderr, "non-value in map section\n");
				goto failed;
			}
			continue;
		}
		fprintf(stderr, "Syntax error: %i\n", res);
		break;
	}
	
	if(mapstate > 0)
	{
		fprintf(stderr, "incomplete meta data in map section\n");
	}

	close(sm_fd);
	
	bmfd = open(argv[2], O_WRONLY | O_TRUNC | O_CREAT, 0666);
	if(bmfd == -1)
	{
		perror("open dest file");
		return 1;
	}	

	wrbmp(bmfd);
	
	return 0;	

failed:
	fprintf(stderr, "failed to process\n");
	close(sm_fd);
	
	return 1;
}

