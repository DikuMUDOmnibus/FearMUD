/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * ------------------------------------------------------------------------ *
 *                          Color Code Header Information                   *
 ****************************************************************************/
/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997, 1998  Jesse DeFer and Heath Leach
 http://dotd.mudservices.com  dotd@dotd.mudservices.com 
 ******************************************************/

#include "ansi.h"

DECLARE_DO_FUN(do_color);

void show_colors args((CHAR_DATA * ch));
void reset_colors args((CHAR_DATA * ch));
void set_char_color args((sh_int AType, CHAR_DATA * ch));
void set_pager_color args((sh_int AType, CHAR_DATA * ch));

#define AT_BLACK	    	0
#define AT_BLOOD	    	1
#define AT_DGREEN       2
#define AT_ORANGE	    	3
#define AT_DBLUE	    	4
#define AT_PURPLE	    	5
#define AT_CYAN	  	6
#define AT_GREY		7
#define AT_DGREY	    	8
#define AT_RED		9
#define AT_GREEN	   	10
#define AT_YELLOW	   	11
#define AT_BLUE		12
#define AT_PINK		13
#define AT_LBLUE	   	14
#define AT_WHITE	   	15
#define AT_BLINK	   	16

#define AT_WHITE_BLINK	   AT_WHITE + AT_BLINK
#define AT_RED_BLINK	   AT_RED + AT_BLINK

#define AT_PLAIN		17
#define AT_ACTION		18
#define AT_SAY		19
#define AT_GOSSIP		20
#define AT_YELL		21
#define AT_TELL		22
#define AT_HIT		23
#define AT_HITME		24
#define AT_IMMORT		25
#define AT_HURT		26
#define AT_FALLING	27
#define AT_DANGER		28
#define AT_MAGIC		29
#define AT_CONSIDER	30
#define AT_REPORT		31
#define AT_POISON		32
#define AT_SOCIAL		33
#define AT_DYING		34
#define AT_DEAD		35
#define AT_SKILL		36
#define AT_CARNAGE	37
#define AT_DAMAGE		38
#define AT_FLEE		39
#define AT_RMNAME		40
#define AT_RMDESC		41
#define AT_OBJECT		42
#define AT_PERSON		43
#define AT_LIST		44
#define AT_BYE		45
#define AT_GOLD		46
#define AT_GTELL		47
#define AT_NOTE		48
#define AT_HUNGRY		49
#define AT_THIRSTY	50
#define AT_FIRE		51
#define AT_SOBER		52
#define AT_WEAROFF	53
#define AT_EXITS		54
#define AT_SCORE		55
#define AT_RESET		56
#define AT_LOG		57
#define AT_DIEMSG		58
#define AT_WARTALK      59
#define AT_ARENA        60
#define AT_MUSE         61
#define AT_THINK        62
#define AT_AFLAGS      	63 /* Added by Samson 9-29-98 for area flag display line */
#define AT_WHO	    	64 /* Added by Samson 9-29-98 for wholist */
#define AT_RACETALK   	65 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_IGNORE     	66 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_WHISPER    	67 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_DIVIDER    	68 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_MORPH      	69 /* Added by Samson 9-29-98 for version 1.4 code */
#define AT_SHOUT		70 /* Added by Samson 9-29-98 for shout channel */
#define AT_RFLAGS		71 /* Added by Samson 12-20-98 for room flag display line */
#define AT_STYPE		72 /* Added by Samson 12-20-98 for sector display line */
#define AT_ANAME		73 /* Added by Samson 12-20-98 for filename display line */
#define AT_AUCTION      74 /* Added by Samson 12-25-98 for auction channel */
#define AT_SCORE2		75 /* Added by Samson 2-3-99 for DOTD code */
#define AT_SCORE3		76 /* Added by Samson 2-3-99 for DOTD code */
#define AT_SCORE4		77 /* Added by Samson 2-3-99 for DOTD code */
#define AT_WHO2		78 /* Added by Samson 2-3-99 for DOTD code */
#define AT_WHO3		79 /* Added by Samson 2-3-99 for DOTD code */
#define AT_WHO4		80 /* Added by Samson 2-3-99 for DOTD code */

#define MAX_COLORS    81

extern const sh_int default_set[MAX_COLORS];
