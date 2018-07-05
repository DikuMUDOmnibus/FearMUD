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
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 *			    Main mud header file			    *
 ****************************************************************************/

#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#ifdef MCCP
  #include <zlib.h>
#endif
#ifdef MTRACE
#include <mcheck.h> /* get the mtrace definitions */
#endif

/* Tracker arrangements, for greping code changes fast -- Xerves
 * Tracker1 - Imm Structure Changes
 * Tracker2 - Caste Changes
 */

/* force the who command to require an argument (should use cset) */
/* #define REQWHOARG */
#ifdef WIN32
#include <winsock.h>
#include <sys/types.h>
#pragma warning( disable: 4018 4244 4761)
 /* #define NOCRYPT */
#define index strchr
#define rindex strrchr
#else
#include <unistd.h>
#ifndef SYSV
#include <sys/cdefs.h>
#else
#include <re_comp.h>
#endif
#include <sys/time.h>
#endif

typedef int ch_ret;
typedef int obj_ret;

/*
 * Accommodate old non-Ansi compilers.
 */
#if defined(TRADITIONAL)
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	ch_ret fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#endif


/*
 * Short scalar types.
 * Diavolo reports AIX compiler has bugs with short types.
 */
#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

#if	defined(_AIX)
#if	!defined(const)
#define const
#endif
typedef int sh_int;
typedef int bool;

#define unix
#else
typedef int sh_int;
typedef unsigned char bool;
#endif

/*
 * Structure types.
 */
typedef struct channel_history CHANNEL_HISTORY;
typedef struct affect_data AFFECT_DATA;
typedef struct authorize_data AUTHORIZE_DATA; // Keeps Auth data
typedef struct area_data AREA_DATA;
typedef struct introduction_data INTRO_DATA; //introduction area
typedef struct kingdom_data KINGDOM_DATA;
typedef struct town_data TOWN_DATA;
typedef struct door_list DOOR_LIST;
typedef struct door_data DOOR_DATA;
typedef struct npcrace_data NPCRACE_DATA;
typedef struct quest_mob_data QMOB_DATA;
typedef struct quest_obj_data QOBJ_DATA;
typedef struct quest_data QUEST_DATA;
typedef struct trap_data TRAP_DATA;
typedef struct key_data KEY_DATA;
typedef struct market_data MARKET_DATA;
typedef struct schedule_data SCHEDULE_DATA;
typedef struct training_data TRAINING_DATA;
typedef struct ship_data SHIP_DATA;
typedef struct buy_holdingbin_data BUYKBIN_DATA;
typedef struct buy_kmob_data BUYKMOB_DATA;
typedef struct buy_kobj_data BUYKOBJ_DATA;
typedef struct buy_ktrainer_data BUYKTRAINER_DATA;
typedef struct bought_trainer_data BTRAINER_DATA;
typedef struct wilderblock_data WBLOCK_DATA; //Mob blocks in Wilderness
typedef struct wilderinfo_data WINFO_DATA; //Char/Time info
typedef struct auction_data AUCTION_DATA; /* auction data */
typedef struct watch_data WATCH_DATA;
typedef struct ban_data BAN_DATA;
typedef struct front_data FRONT_DATA; //front node
typedef struct tornado_data TORNADO_DATA;
typedef struct trade_data TRADE_DATA;
typedef struct imbue_data IMBUE_DATA;
typedef struct kingdom_chest_data KCHEST_DATA;
typedef struct wilderness_bin_data BIN_DATA;
typedef struct battle_arena_data BARENA_DATA;
typedef struct gem_data GEM_DATA;
typedef struct box_data BOX_DATA;
typedef struct forge_data FORGE_DATA;
typedef struct depo_ore_data DEPO_ORE_DATA;
typedef struct conquer_data CONQUER_DATA;
typedef struct depo_weapon_data DEPO_WEAPON_DATA;
typedef struct slab_data SLAB_DATA;    
typedef struct char_map_data CMAP_DATA;
typedef struct obj_map_data OMAP_DATA;
typedef struct stable_data STABLE_DATA;
typedef struct extracted_char_data EXTRACT_CHAR_DATA;
typedef struct char_data CHAR_DATA;
typedef struct midata MI_DATA;
typedef struct kingdom_military_list MLIST_DATA;
typedef struct clan_member_list CMEMBER_DATA;
typedef struct kingdom_member_list KMEMBER_DATA;
typedef struct trainer_data TRAINER_DATA;
typedef struct save_arena_data SARENA_DATA;
typedef struct hunt_hate_fear HHF_DATA;
typedef struct aggression_data AGGRO_DATA;
typedef struct mpsleep_data MPSLEEP_DATA;
typedef struct fighting_data FIGHT_DATA;
typedef struct descriptor_data DESCRIPTOR_DATA;
typedef struct account_data ACCOUNT_DATA;
typedef struct account_name ACCOUNT_NAME;
typedef struct exit_data EXIT_DATA;
typedef struct extra_descr_data EXTRA_DESCR_DATA;
typedef struct help_index_data HINDEX_DATA;
typedef struct help_data_pointer HINDEX_POINTER;
typedef struct help_index_pointer HINDEX_IPOINTER;
typedef struct help_index_name HINDEX_NAME;
typedef struct help_data HELP_DATA;
typedef struct menu_data MENU_DATA;
typedef struct mob_index_data MOB_INDEX_DATA;
typedef struct char_morph CHAR_MORPH;
typedef struct morph_data MORPH_DATA;
typedef struct nuisance_data NUISANCE_DATA;
typedef struct note_data NOTE_DATA;
typedef struct comment_data COMMENT_DATA;
typedef struct board_data BOARD_DATA;
typedef struct obj_data OBJ_DATA;
typedef struct obj_index_data OBJ_INDEX_DATA;
typedef struct pc_data PC_DATA;
typedef struct plane_data PLANE_DATA;
typedef struct reset_data RESET_DATA;
typedef struct map_index_data MAP_INDEX_DATA; /* maps */
typedef struct map_data MAP_DATA; /* maps */
typedef struct room_index_data ROOM_INDEX_DATA;
typedef struct shop_data SHOP_DATA;
typedef struct race_type RACE_TYPE;
typedef struct repairshop_data REPAIR_DATA;
typedef struct reserve_data RESERVE_DATA;
typedef struct time_info_data TIME_INFO_DATA;
typedef struct hour_min_sec HOUR_MIN_SEC;
typedef struct clan_data CLAN_DATA;
typedef struct council_data COUNCIL_DATA;
typedef struct tourney_data TOURNEY_DATA;
typedef struct mob_prog_data MPROG_DATA;
typedef struct mob_prog_act_list MPROG_ACT_LIST;
typedef struct editor_data EDITOR_DATA;
typedef struct teleport_data TELEPORT_DATA;
typedef struct timer_data TIMER;
typedef struct godlist_data GOD_DATA;
typedef struct system_data SYSTEM_DATA;
typedef struct smaug_affect SMAUG_AFF;
typedef struct who_data WHO_DATA;
typedef struct skill_type SKILLTYPE;
typedef struct social_type SOCIALTYPE;
typedef struct cmd_type CMDTYPE;
typedef struct killed_data KILLED_DATA;
typedef struct pkilled_data PKILLED_DATA;
typedef struct deity_data DEITY_DATA;
typedef struct wizent WIZENT;
typedef struct ignore_data IGNORE_DATA;
typedef struct immortal_host IMMORTAL_HOST;
typedef struct project_data PROJECT_DATA;
typedef struct extended_bitvector EXT_BV;
typedef struct lcnv_data LCNV_DATA;
typedef struct lang_data LANG_DATA;


/*
 * Function types.
 */
typedef void DO_FUN args((CHAR_DATA * ch, char *argument));
typedef bool SPEC_FUN args((CHAR_DATA * ch));
typedef ch_ret SPELL_FUN args((int sn, int level, CHAR_DATA * ch, void *vo));

#define DUR_CONV	23.333333333333333333333333
#define HIDDEN_TILDE	'*'

/* 32bit bitvector defines */
#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 4096*2 /* buf */
#define MAX_INPUT_LENGTH	 1024*2 /* arg */
#define MAX_INBUF_SIZE		 1024*2
#define MSL                  MAX_STRING_LENGTH
#define MIL                  MAX_INPUT_LENGTH
#define MAX_VNUM                 300000
#define MAX_IMM_VNUM             100000
#define MAX_HPMANA               100000
#define MAX_QUEST_DIFF       13 //Maximum quest difficulty
#define LAST_FILE_SIZE       500  //maximum entries in the last file
#define MAX_LOADED_MOBS      100000 //maximum serials for mobs...if for some reason you have more than 100k, increase in 100k incriments, this
                                    //will increase RAM 12.5K a shot, but force another 100k item loop in db.c, so don't make it too high
#define START_QUEST_VNUM     100000
#define END_QUEST_VNUM       199999
#define START_STATICQUEST_VNUM 200000
#define END_STATICQUEST_VNUM   299999
#define GAME_RESET_VALUE       1 // Set to 1 for reset games, 0 for normal games
          
#define HINDEX_GENERAL_NAME "General"
//Uncomment this below to turn on MXP support for hindex code
#define HINDEX_MXP 
//#define HASHSTR /* use string hashing */

#define	MAX_LAYERS		 8 /* maximum clothing layers */
#define MAX_NEST	       100 /* maximum container nesting */

#define MAX_KILLTRACK		25 /* track mob vnums killed */
#define MAX_PKILLTRACK       10 /* track players who are killed */

#define MAX_QDIFF           13
#define MAX_QDIFF_VALUE     130

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_REXITS		   20 /* Maximum exits allowed in 1 room */
/* Below two are used in the new skill system.  It takes a point ranking and
a mastery ranking.  The higher the better -- Xerves 00*/
#define MAX_SKPOINTS               20
#define MAX_RANKING                6
#define MAX_GROUP                  18 /* The top number of skill groups -- Xerves 1/00 */
#define MAX_SPHERE                 5 //Spheres of Groups
#define MAX_TIER                   5 //Maximum tier, 5 for now
#define MAX_SKILL		  400
#define MAX_NPC_CLASS		   26
#define START_INV_TRAP             10000000 //First uid for inventory traps
/*#define MAX_RACE                 20  Trying to fix a bunch of problems-- Scryn*/
#define MAX_RACE                   6 /* 6 Races now -- Xerves */
#define LAST_H_RACE                     22 //Last humanoid npc race
#define MAX_KINGDOM                20
#define MAX_PEACEVALUE             4
#define MAX_NPCRACE_TABLE      200
#define MAX_LEVEL		   7 /* Tracker1 -- Xerves 4/10/99 */
#define MAX_CLAN		   50
#define MAX_DEITY		   50
#define MAX_TOHITAC        50
#define MAX_CPD			    4 /* Maximum council power level difference */
#define	MAX_HERB		   20
#define	MAX_DISEASE		   20
#define MAX_PERSONAL		    5 /* Maximum personal skills */
#define MAX_WHERE_NAME             29
#define MAX_MOB_COUNT              10
#define TOHIT_SYS          50 //Value that is rolled in the tohit system.  Making this a larger margin will increase hits and
                              //making this smaller will decrease it.  Want to keep this around 30-70 due to how values are
                              //applied from spells/skills
#define MAX_PC_ENDURANCE      74
#define TRAINING_TIME 6480   //6480 - 3 days GT   1.8 Hours RL time Time for 1 session of training time, max on stock is 36

#define LEVEL_PC                   (MAX_LEVEL - 6) //1
#define LEVEL_NPC                  (MAX_LEVEL - 7) //0
#define LEVEL_GUEST                (MAX_LEVEL - 5) //2
#define LEVEL_IMM                  (MAX_LEVEL - 4) //3
#define LEVEL_HI_IMM               (MAX_LEVEL - 3) //4
#define LEVEL_STAFF                (MAX_LEVEL - 2) //5
#define LEVEL_HI_STAFF             (MAX_LEVEL - 1) //6
#define LEVEL_ADMIN                (MAX_LEVEL)     //7
#define LEVEL_IMMORTAL		     (MAX_LEVEL - 5) //2

#define LEVEL_LOG		          LEVEL_STAFF
#define LEVEL_HIGOD		        LEVEL_HI_IMM

/* Caste numbers...Each Set gets 10 */

#define caste_Serf         1
#define caste_Peasant      2
#define caste_Laborer      3
#define caste_Apprentice   4
#define caste_Journeymen   5
#define caste_Master       6
#define caste_Merchant     7
#define caste_Trader       8
#define caste_Businessman  9
#define caste_Mayor       10

#define caste_Page        11
#define caste_Squire      12
#define caste_Knight      13
#define caste_Baronet     14
#define caste_Baron       15
#define caste_Earl        16
#define caste_Viscount    17
#define caste_Count       18
#define caste_Duke        19
#define caste_Marquis     20

#define caste_Vassal      21
#define caste_LordVassal  22
#define caste_Lord        23
#define caste_HiLord      24
#define caste_Captain     25
#define caste_Minister    26
#define caste_Prince      27
#define caste_King        28
#define caste_Avatar      29
#define caste_Legend      30


#define caste_Ascender    31
#define caste_Immortal    32
#define caste_God         33
#define caste_Staff       34
#define caste_Admin       35

#define MAX_CASTE         caste_Admin


/*
 * Include .h section to help break up the mud.h file
 */
#include "color.h"
#include "bank.h"
#include "pfiles.h"
#include "finger.h"
#include "board.h"
#include "alias.h"
#include "slay.h"
#include "overland.h"
#include "dns.h"

#define is_full_name is_name

#define	SECONDS_PER_TICK			 70

#define PULSE_PER_SECOND			  4
#define PULSE_VIOLENCE				  2
#define PULSE_MOBILE				 (4 * PULSE_PER_SECOND)
#define PULSE_TICK		    (SECONDS_PER_TICK * PULSE_PER_SECOND)
#define PULSE_AREA				 (60 * PULSE_PER_SECOND)
#define PULSE_AUCTION				 (20 * PULSE_PER_SECOND)
#define PULSE_WEATHER                 (8 * PULSE_PER_SECOND)
#define PULSE_ARENA                              (60 * PULSE_PER_SECOND)


/*
 * Stuff for area versions --Shaddai
 */
int area_version;

#define HAS_SPELL_INDEX     -1

/*
 * Increment with every major format change.
 */

#define TRAINERLISTVERSION 2
/**************************************
  Version Write History
  1 - Pre SN Fix
  2 - Post SN FIX
 **************************************/ 
 
#define CASTEBUYVERSION 2
/**************************************
  Version Write History
  1 - Pre SN Fix
  2 - Post SN FIX
 **************************************/
 
#define SAVEVERSION	5
/**************************************
  Version Write History
  3 - Stock SMAUG
  4 - Added the aff mods for sanctify, hpgen, etc 
  5 - For gemnum affect to save on fwrite_obj
 **************************************/

#define AREA_VERSION_WRITE 53
/**************************************
  Version Write History
  1 - Stock SMAUG
  11 - Added Cvnum
  12 - Add goldmin goldmax goldlim
       levelmin levelmax
  13 - Added caste slot on mobs/obj to
       see what kinda of mob/obj it is.
  14 - Version write for new room extends
  15 - goldmin, etc changed to m1 to m5
       while m6 was added for this version
  16 - Fields for Mining/etc
  17 - Adding quad (Quadrants) for wilderness
  18 - Added movement save for mobs
  19 - Solan Maps
  20 - Mob Coord Resets
  21 - Obj Coord Resets
  22 - V6 added for levels for all items
  23 - Kingdom variable in the area file
  24 - v7 and v8 added for do_connect -- altered to do_setgem
  25 - Population Variable in Areas
  26 - m7, m8, m9.  Used for variable things
  27 - miflags for mobs
  28 - x,y,map for Areas (weather)
  29 - Element for ch/mob
  30 - v9 added for objects, probably will not use do_connect (altered to do_setgem, WILL be used!)
  31 - v10 added for weapon durability (man it is getting full)
  32 - Spells on weapons (staves)
  33 - Added the Forging Resets
  34 - Added ac back in, kind of need it....
  35 - Fixed Forging REsets
  36 - Tohit values added to mobs
  37 - Agility added to mobs :-)
  38 - Mana nodes
  39 - bless value for weapons
  40 - New Time based Resets added into the reset code (there is no area_version for the serial)
  41 - LastTaxChange for Hometowns (should of done this years ago)
  42 - v11 added for more FUN!!!
  43 - Reset value added for race on an object (G only)
  44 - kpid on areas
  45 - m10-m12 Added, More Kingdom related fun
  46 - v12-v13 Added for Parry on weapons, god I am running out of room, rofl
  47 - A Reset added for new Trap code
  48 - Weight on objects changed to a floating value
  49 - New Sworth Restrictions for Equipment
  50 - New Resist percent values for players/mobs
  51 - New Value for imbueslots
  52 - Damadd for mobs, REALLY need this bad to keep up with damage output
  53 - GEM ID on an affect
  **************************************/
  /*************************************
    Cident values

    1 - Manuel
    2 - Shopkeeper
    3 - Shells Gambler
    4 - Flipcoin Gambler
    5 - Blackjack Gambler
    ***********************************/

/*
 * Command logging types.
 */
typedef enum
{
   LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM,
   LOG_WARN, LOG_ALL
}
log_types;

/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
/* Chewed used for mount food -- Xerves 11/99 */
typedef enum
{
   rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
   rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
   rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
   rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE, rCHAR_AND_OBJ_EXTRACTED = 128,
   rERROR, rSTOP = 255, rOBJ_CHEWED
}
ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2

/* defines for new do_who */
#define WT_MORTAL	0
#define WT_DEADLY	1
#define WT_IMM		2
#define WT_GROUPED	3
#define WT_GROUPWHO	4

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
#define INTBITS	32
#endif
#define XBM		31 /* extended bitmask   ( INTBITS - 1 ) */
#define RSV		5 /* right-shift value  ( sqrt(XBM+1) ) */
#define XBI		4 /* integers in an extended bitvector */
#define MAX_BITS	XBI * INTBITS
/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
   int bits[XBI];
};

/*
 * Structure for a morph -- Shaddai
 */
/*
 *  Morph structs.
 */

#define ONLY_PKILL  	1
#define ONLY_PEACEFULL  2

struct char_morph
{
   MORPH_DATA *morph;
   EXT_BV affected_by; /* New affected_by added */
   int immune; /* Immunities added */
   EXT_BV no_affected_by; /* Prevents affects from being added */
   int no_immune; /* Prevents Immunities */
   int no_resistant; /* Prevents resistances */
   int no_suscept; /* Prevents Susceptibilities */
   int resistant; /* Resistances added */
   int suscept; /* Suscepts added */
   int timer; /* How much time is left */
   sh_int ac;
   sh_int blood;
   sh_int cha;
   sh_int con;
   sh_int damroll;
   sh_int dex;
   sh_int dodge;
   sh_int hit;
   sh_int hitroll;
   sh_int inte;
   sh_int lck;
   sh_int mana;
   sh_int move;
   sh_int parry;
   sh_int saving_breath;
   sh_int saving_para_petri;
   sh_int saving_poison_death;
   sh_int saving_spell_staff;
   sh_int saving_wand;
   sh_int str;
   sh_int tumble;
   sh_int wis;
};

struct morph_data
{
   MORPH_DATA *next; /* Next morph file */
   MORPH_DATA *prev; /* Previous morph file */
   char *blood; /* Blood added vamps only */
   char *damroll;
   char *deity;
   char *description;
   char *help; /* What player sees for info on morph */
   char *hit; /* Hitpoints added */
   char *hitroll;
   char *key_words; /* Keywords added to your name */
   char *long_desc; /* New long_desc for player */
   char *mana; /* Mana added not for vamps */
   char *morph_other; /* What others see when you morph */
   char *morph_self; /* What you see when you morph */
   char *move; /* Move added */
   char *name; /* Name used to polymorph into this */
   char *short_desc; /* New short desc for player */
   char *no_skills; /* Prevented Skills */
   char *skills;
   char *unmorph_other; /* What others see when you unmorph */
   char *unmorph_self; /* What you see when you unmorph */
   EXT_BV affected_by; /* New affected_by added */
   int class; /* Classes not allowed to use this */
   int defpos; /* Default position */
   int immune; /* Immunities added */
   EXT_BV no_affected_by; /* Prevents affects from being added */
   int no_immune; /* Prevents Immunities */
   int no_resistant; /* Prevents resistances */
   int no_suscept; /* Prevents Susceptibilities */
   int obj[3]; /* Object needed to morph you */
   int race; /* Races not allowed to use this */
   int resistant; /* Resistances added */
   int suscept; /* Suscepts added */
   int timer; /* Timer for how long it lasts */
   int used; /* How many times has this morph been used */
   int vnum; /* Unique identifier */
   sh_int ac;
   sh_int bloodused; /* Amount of blood morph requires Vamps only */
   sh_int cha; /* Amount Cha gained/Lost */
   sh_int con; /* Amount of Con gained/Lost */
   sh_int dayfrom; /* Starting Day you can morph into this */
   sh_int dayto; /* Ending Day you can morph into this */
   sh_int dex; /* Amount of dex added */
   sh_int dodge; /* Percent of dodge added IE 1 = 1% */
   sh_int favourused; /* Amount of favour to morph */
   sh_int gloryused; /* Amount of glory used to morph */
   sh_int hpused; /* Amount of hps used to morph */
   sh_int inte; /* Amount of Int gained/lost */
   sh_int lck; /* Amount of Lck gained/lost */
   sh_int level; /* Minimum level to use this morph */
   sh_int manaused; /* Amount of mana used to morph */
   sh_int moveused; /* Amount of move used to morph */
   sh_int parry; /* Percent of parry added IE 1 = 1% */
   sh_int pkill; /* Pkill Only, Peacefull Only or Both */
   sh_int saving_breath; /* Below are saving adjusted */
   sh_int saving_para_petri;
   sh_int saving_poison_death;
   sh_int saving_spell_staff;
   sh_int saving_wand;
   sh_int sex; /* The sex that can morph into this */
   sh_int str; /* Amount of str gained lost */
   sh_int timefrom; /* Hour starting you can morph */
   sh_int timeto; /* Hour ending that you can morph */
   sh_int tumble; /* Percent of tumble added IE 1 = 1% */
   sh_int wis; /* Amount of Wis gained/lost */
   bool no_cast; /* Can you cast a spell to morph into it */
   bool objuse[3]; /* Objects needed to morph */
};

/*
 * Tongues / Languages structures
 */

struct lcnv_data
{
   LCNV_DATA *next;
   LCNV_DATA *prev;
   char *old;
   int olen;
   char *new;
   int nlen;
};

struct wilderblock_data
{
   WBLOCK_DATA *next;
   WBLOCK_DATA *prev;
   int stx;
   int sty;
   int endx;
   int endy;
   int map;
   int kills;
   int timecheck;
   WINFO_DATA *first_player;
   WINFO_DATA *last_player;
   int lvl;
};

struct wilderinfo_data
{
   WINFO_DATA *next;
   WINFO_DATA *prev;
   int pid; //Player pid value, to uniquely track players
   int time; //Time last in, so players just cannot walk back/forth to clear out an area.  
};

struct buy_kobj_data
{
   BUYKOBJ_DATA *next;
   BUYKOBJ_DATA *prev;
   int tree;
   int corn;
   int grain;
   int iron;
   int gold;
   int stone;
   int coins;
   int vnum;
   EXT_BV flags;
   int mincaste;
};

typedef enum
{
   KOBJ_HOMETOWN, KOBJ_WILDERNESS, KOBJ_CONTAINER, KOBJ_SIGIL, KOBJ_NOTEBOARD, KOBJ_TOROOM,
   KOBJ_TOCHAR, KOBJ_ADDRESET, KOBJ_BIN, KOBJ_WALLREPAIR
}
kobj_type;

struct bought_trainer_data
{
   BTRAINER_DATA *next;
   BTRAINER_DATA *prev;
   int rvnum;
   int x;
   int y;
   int map;
   int pid;
};

struct buy_ktrainer_data
{
   BUYKTRAINER_DATA *next;
   BUYKTRAINER_DATA *prev;
   int sn[20];
   int mastery[20];
   char *name;
   int pid;
   int cost;
};

struct buy_holdingbin_data
{
   BUYKBIN_DATA *next;
   BUYKBIN_DATA *prev;
   char *name;
   int stone;
   int coins;
   int hold;
   int mincaste;
};

struct buy_kmob_data
{
   BUYKMOB_DATA *next;
   BUYKMOB_DATA *prev;
   int tree;
   int corn;
   int grain;
   int iron;
   int gold;
   int stone;
   int coins;
   int vnum;
   EXT_BV flags;
   int mincaste;
};

typedef enum
{
   KMOB_WILDERNESS, KMOB_MILITARY, KMOB_HOUR, KMOB_4MONTH, KMOB_ADDRESET, KMOB_REPAIR, KMOB_FORGE, KMOB_CLERIC,
   KMOB_WORKER, KMOB_GUARD, KMOB_SOLDIER, KMOB_SCOUT, KMOB_MAGE, KMOB_SOLDIERAGI, KMOB_SOLDIERMOVE, KMOB_SOLDIERDAM,
   KMOB_SOLDIERUNARMED, KMOB_MOUNTED, KMOB_SOLDIERADDAGI, KMOB_SOLDIERADDMOVE, KMOB_SOLDIERONEMOVE,
   KMOB_TOHIT1, KMOB_TOHIT2, KMOB_TOHIT3, KMOB_TOHIT4, KMOB_AC1, KMOB_AC2, KMOB_AC3, KMOB_AC4, KMOB_DAM1,
   KMOB_DAM2, KMOB_DAM3, KMOB_DAM4, KMOB_CURELIGHT, KMOB_CURESERIOUS, KMOB_CURECRITICAL, KMOB_HEAL, KMOB_DIVINITY,
   KMOB_POWERHEAL, KMOB_BLESS, KMOB_ARMOR, KMOB_STONESKIN, KMOB_SANCTIFY, KMOB_FLEETARMS, KMOB_SANCTUARY,
   KMOB_HARM, KMOB_CLERICDAMAGE, KMOB_LEATHERONLY, KMOB_LIGHTONLY, KMOB_MEDIUMONLY, KMOB_SHIELD, KMOB_KINDRED,
   KMOB_SLINK, KMOB_FIRESHIELD, KMOB_ICESHIELD, KMOB_SHOCKSHIELD, KMOB_ANTIMAGICSHELL, KMOB_MAGEDAMAGE1,
   KMOB_MAGEDAMAGE2, KMOB_MAGEDAMAGE3, KMOB_MAGEDAMAGE4, KMOB_ARCHER
   
}
kmob_type;

struct conquer_data
{
   CONQUER_DATA *next;
   CONQUER_DATA *prev;
   int time;
   int akingdom;
   int rkingdom;
   TOWN_DATA *town;
   char *ntown;
   int occupied;
};

struct door_list  //really don't need this, but I will leave it in.....
{
   DOOR_LIST *next;
   DOOR_LIST *prev;
   DOOR_DATA *first_door;
   DOOR_DATA *last_door;
};

//Note:  There is a 30 point hardcount per room, if you wish to change this, change these
//values int roomcoordx[30]  int roomcoordy[30]   int roomcoordmap[30] in mud.h to whatever
//value you want, and make sure to change it below to search more than 30.  Also, don't
//set it higher than you need, it is to keep players from screwing things up
#define MAX_HPOINTS 30

struct key_data
{
   KEY_DATA *next;
   KEY_DATA *prev;
   int flag;
   char *name;
};

struct door_data
{
   DOOR_DATA *next;
   DOOR_DATA *prev;
   int doorvalue[10]; //max of 10 doors
   int roomcoordx[30]; //30 rooms max per "room"
   int roomcoordy[30];
   int roomcoordmap[30];
   int cansee; //0 - can't see  1 - can see
   int cannotsee;
};

struct training_data
{
   TRAINING_DATA *next;
   TRAINING_DATA *prev;
   BUYKMOB_DATA *kmob;
   int speed;
   int kingdom;
   int town;
   int stime;
   int etime;
   int x;
   int y;
   int map;
   int bin;
   int resource;
};

struct schedule_data
{
   SCHEDULE_DATA *next;
   SCHEDULE_DATA *prev;
   int start_period;
   int end_period;
   int resource;
   int reoccur;
   int ran;
   int x;
   int y;
   int map;
};
   


struct town_data
{
   TOWN_DATA *next;
   TOWN_DATA *prev;
   SCHEDULE_DATA *first_schedule;
   SCHEDULE_DATA *last_schedule;
   KEY_DATA *first_key;
   KEY_DATA *last_key;
   char *name;
   char *mayor;
   int kingdom;
   int kpid;
   int tpid;
   int corn;
   int grain;
   int coins;
   int gold;
   int iron;
   int lumber;
   int stone;
   int fish;
   int hold;
   int recall[3];
   int death[3];
   int poptax;
   int ctax;
   int salestax;
   int size;
   int rooms;
   int maxsize;
   int moral;
   int month;
   int growthcheck;
   int growth;
   int startx;
   int starty;
   int units;
   int unitstraining;
   int startmap;
   int foodconsump;
   int stoneconsump;
   int lumberconsump;
   int coinconsump;
   int barracks[3]; //Vnum used to store soldiers
   int roomcoords[151][3]; //x, y, map
   char roomtitles[151][80]; //title
   EXT_BV roomflags[151]; //flags
   int minhappoint;
   int minwithdraw;
   int lasttaxchange;
   int allowexpansions;
   int expansions; //expansions allowed per year
   DOOR_LIST *first_doorlist;
   DOOR_LIST *last_doorlist;
   int doorstate[8][100]; //max 100 doors, 0 - open/close, 1 - locked, 2 - unique key value, 3 - master door, 4 - unique door value
                          //5 - x coord, 6 - y coord, 7 - map coord
   int max_dvalue; //it is possible to have "made" more than 100 doors, just in case they all get destroyed or something...
   int usedpoint[60][60]; //Check to see if a point is already connected to a room, make sure there are no overlaps.
   int bincoords[60][60]; //For bins...
   int banksize; //Adding in banks for towns, allows storage of player gold and other crap
   OBJ_DATA *first_bankobj;
   OBJ_DATA *last_bankobj;
   int balance; //Gold in the bank...
};

struct kingdom_data
{
   TOWN_DATA *first_town;
   TOWN_DATA *last_town;
   char *name;
   char *ruler;
   char *logfile;
   char *dtown;
   char *number1;
   char *number2;
   int num;
   int kpid;
   sh_int tree_tax;
   sh_int corn_tax;
   sh_int grain_tax;
   sh_int iron_tax;
   sh_int gold_tax;
   sh_int stone_tax;
   sh_int fish_tax;
   int salestax;
   int ctax;
   int poptax;
   int minbuild;
   int minplace;
   int minappoint;
   int minhappoint;
   int mintoperate;
   int minwithdraw;
   int mincommand; //Minimum level to command local troops
   int mingeneral; //Minimum level to command all troops
   int minreadlog;
   int mindepository;
   int minlogsettings;
   int mintrainertax;
   int minswitchtown;
   int minbooktax;
   int tier1;
   int tier2;
   int tier3;
   int tier4;
   int tvisitor;
   int tier1book;
   int tier2book;
   int tier3book;
   int tier4book;
   int bvisitor;
   int allowjoin; //Allow players to join?
   sh_int mob_que[26]; //26 ALWAYS needs to be 0
   sh_int obj_que[26]; //26 ALWAYS needs to be 0
   sh_int trainer_que[26]; //26 ALWAYS needs to be 0
   sh_int peace[MAX_KINGDOM]; //At peace/war with other kingdoms?
   sh_int cpeace[MAX_KINGDOM]; //For changing peace with other kingdoms
   int lasttaxchange;
   int race;
   int raceset;
   EXT_BV logsettings; //What things are being logged (flag on means it is not logged)
   DEPO_ORE_DATA *first_ore;
   DEPO_ORE_DATA *last_ore;
   INTRO_DATA *first_introduction;
   INTRO_DATA *last_introduction;
   int lastintrocheck;
   int maxlinelog;
   int maxtimelog;
};

struct trade_data
{
   TRADE_DATA *prev;
   TRADE_DATA *next;
   int offering_kingdom; //kingdom offering (starting) the trade
   int receiving_kingdom; //kingdom receiving the trade
   int offering_res_tree;
   int offering_res_corn;
   int offering_res_grain;
   int offering_res_iron;
   int offering_res_gold;
   int offering_res_stone;
   int offering_res_fish;
   int offering_gold;
   int receiving_res_tree;
   int receiving_res_corn;
   int receiving_res_grain;
   int receiving_res_iron;
   int receiving_res_gold;
   int receiving_res_stone;
   int receiving_res_fish;
   int receiving_gold;
   int offering_read;   //marks the deal as read, so kingdoms know they have received trade offers/counter offers
   int receiving_read;  //marks the deal as read, so kingdoms know they have received trade offers/counter offers 
   int time;            //time it was posted, trades are removed after so much time has been spent
   int posted;          //rather it has been sent or not.
};

struct lang_data
{
   LANG_DATA *next;
   LANG_DATA *prev;
   char *name;
   LCNV_DATA *first_precnv;
   LCNV_DATA *last_precnv;
   char *alphabet;
   LCNV_DATA *first_cnv;
   LCNV_DATA *last_cnv;
};



/*
 * do_who output structure -- Narn
 */
struct who_data
{
   WHO_DATA *prev;
   WHO_DATA *next;
   char *text;
   int type;
};

/*
 * Player watch data structure  --Gorog
 */
struct watch_data
{
   WATCH_DATA *next;
   WATCH_DATA *prev;
   sh_int imm_level;
   char *imm_name; /* imm doing the watching */
   char *target_name; /* player or command being watched   */
   char *player_site; /* site being watched     */
};

/*
 * Nuisance structure
 */

#define MAX_NUISANCE_STAGE 10 /* How many nuisance stages */
struct nuisance_data
{
   long int time; /* The time nuisance flag was set */
   long int max_time; /* Time for max penalties */
   int flags; /* Stage of nuisance */
   int power; /* Power of nuisance */
};

/*
 * Ban Types --- Shaddai
 */
#define BAN_SITE        1
#define BAN_CLASS       2
#define BAN_RACE        3
#define BAN_WARN        -1

/*
 * Site ban structure.
 */
struct ban_data
{
   BAN_DATA *next;
   BAN_DATA *prev;
   char *name; /* Name of site/class/race banned */
   char *user; /* Name of user from site */
   char *note; /* Why it was banned */
   char *ban_by; /* Who banned this site */
   char *ban_time; /* Time it was banned */
   int flag; /* Class or Race number */
   int unban_date; /* When ban expires */
   sh_int duration; /* How long it is banned for */
   sh_int level; /* Level that is banned */
   bool warn; /* Echo on warn channel */
   bool prefix; /* Use of *site */
   bool suffix; /* Use of site* */
};



/*
 * Yeesh.. remind us of the old MERC ban structure? :)
 */
struct reserve_data
{
   RESERVE_DATA *next;
   RESERVE_DATA *prev;
   char *name;
};



/*
 * Time and weather stuff.
 */
typedef enum
{
   SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
}
sun_positions;

typedef enum
{
   SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
}
sky_conditions;

struct time_info_data
{
   int sunlight;
};

struct hour_min_sec
{
   int hour;
   int min;
   int sec;
   int manual;
};

/* Define maximum number of climate settings - FB */
#define MAX_CLIMATE 5

/*
 * Structure used to build wizlist
 */
struct wizent
{
   WIZENT *next;
   WIZENT *last;
   char *name;
   sh_int level;
};

/*
 * Structure to only allow immortals domains to access their chars.
 */
struct immortal_host
{
   IMMORTAL_HOST *next;
   IMMORTAL_HOST *prev;
   char *name;
   char *host;
   bool prefix;
   bool suffix;
};

struct authorize_data
{
   AUTHORIZE_DATA *next;
   AUTHORIZE_DATA *prev;
   char *name;
   char *lastname;
   char *authedby;
   char *authdate;
   char *host;
};

struct project_data
{
   PROJECT_DATA *next; /* Next project in list     */
   PROJECT_DATA *prev; /* Previous project in list    */
   NOTE_DATA *first_log; /* First log on project     */
   NOTE_DATA *last_log; /* Last log  on project     */
   char *name;
   char *owner;
   char *coder;
   char *status;
   char *date;
   char *description;
   char *rewardee;
   int points;
   int rewardedpoints;
   int time;
   int type;
   bool taken; /* Has someone taken project?      */
};


/*
 * Connected state for a channel.
 */
typedef enum
{
   CON_PLAYING, CON_GET_ACCOUNT, CON_CONFIRM_NEW_ACCOUNT, CON_GET_EMAIL,
   CON_CONFIRM_EMAIL, CON_CONFIRM_ACCOUNT_PASSWORD, CON_SHOW_ACCOUNT_MENU,
   CON_IMPORT_MENU, CON_NEWPASS_MENU, CON_NEWEMAIL_MENU, CON_DELETEACCOUNT_MENU,
   CON_DELETEPLAYER_MENU, CON_RELEASEPLAYER_MENU,
   CON_GET_NAME, CON_CREATE_NAME, CON_LAST_MENU_OPTION, 
   CON_CONFIRM_NEW_NAME, CON_GET_NEW_PASSWORD, CON_CONFIRM_NEW_PASSWORD,
   CON_GET_NEW_SEX, CON_GET_NEW_CLASS, CON_READ_MOTD,
   CON_GET_NEW_RACE, CON_GET_EMULATION, CON_EDITING,
   CON_GET_WANT_RIPANSI, CON_TITLE, CON_PRESS_ENTER,
   CON_WAIT_1, CON_WAIT_2, CON_WAIT_3,
   CON_ACCEPTED, CON_GET_PKILL, CON_READ_IMOTD,
   CON_GET_HOMETOWN, CON_BEGIN_REMORT,
   CON_ROLL_STATS, CON_GET_ALIGNMENT, CON_NOTE_TO,
   CON_NOTE_SUBJECT, CON_NOTE_EXPIRE, CON_NOTE_FINISH,
   CON_LOGIN_MENU, CON_FIRST_SCREEN, CON_FIRST_SKILLS,
   CON_COPYOVER_RECOVER, CON_GETDNS, CON_HAND, CON_ELEMENT, CON_SOURCE,
   CON_SKIN, CON_HAIRCOLOR, CON_HAIRLENGTH, CON_HAIRSTYLE, CON_EYECOLOR,
   CON_HEIGHT, CON_WEIGHT, CON_GET_LAST_NAME, CON_CONFIRM_LAST_NAME,
   CON_BYPASS_LOGIN, CON_CHANGE_EDIT_STATUS, CON_CHOOSE_EMAIL,
   CON_CHANGE_EMAILSTATUS, CON_USE_RESET_CHAR
}
connection_types;

/*
 * Character substates
 */
typedef enum
{
   SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_BAN_DESC, SUB_OBJ_SHORT,
   SUB_OBJ_LONG, SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC,
   SUB_ROOM_EXTRA, SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT,
   SUB_HELP_EDIT, SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD,
   SUB_RESTRICTED, SUB_DEITYDESC, SUB_MORPH_DESC, SUB_MORPH_HELP,
   SUB_PROJ_DESC, SUB_SLAYCMSG, SUB_SLAYVMSG, SUB_SLAYRMSG, SUB_WRITING_EMAIL,
   SUB_CHANGES,
   /* timer types ONLY below this point */
   SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
}
char_substates;

struct account_name
{
   ACCOUNT_NAME *next;
   ACCOUNT_NAME *prev;
   char *name;
};

#define ABAN_BAN 1
#define ABAN_ALLOW 0

struct account_data
{
   ACCOUNT_NAME *first_player;
   ACCOUNT_NAME *last_player;
   int editing;
   int changes; //changes you made in the last hour...
   int lasttimereset; //Last time reset on an account...
   int timesave;
   int ban;
   int skiplmenu;
   int noemail;
   char *name;
   char *passwd;
   char *email;
   char *qplayer1;
   char *qplayer2;
   char *qplayer3;
   char *qplayer4;
   //Hrm it kind of hit me I need these 4 on the accounts so they don't get written over by another account
   int passvalue;
   char *pbuffer;
   char pbuf[150];
   char nbuf[100];
};

/*
 * Descriptor (channel) structure.
 */
struct descriptor_data
{
   DESCRIPTOR_DATA *next;
   DESCRIPTOR_DATA *prev;
   DESCRIPTOR_DATA *snoop_by;
   CHAR_DATA *character;
   CHAR_DATA *original;
   ACCOUNT_DATA *account;
   SARENA_DATA *arena;
#ifndef   DNS_SLAVE
   char *host;
#endif
   int port;
   int descriptor;
   sh_int connected;
   sh_int idle;
   sh_int lines;
   sh_int scrlen;
   sh_int  mxp;
   char *mxpclient;
   float mxpversion;
   bool fcommand;
   char inbuf[MAX_INBUF_SIZE];
   char incomm[MAX_INPUT_LENGTH];
   char inlast[MAX_INPUT_LENGTH];
   int repeat;
   char *outbuf;
   unsigned long outsize;
   int outtop;
   char *pagebuf;
   unsigned long pagesize;
   int pagetop;
   char *pagepoint;
   char pagecmd;
   char pagecolor;
   char *user;
   int newstate;
   unsigned char prevcolor;
   #ifdef MCCP
    unsigned char	compressing;
    z_stream *          out_compress;
    unsigned char *     out_compress_buf;
   #endif     
   char *run_buf;
   char *run_head;
   sh_int speed; /* descriptor speed settings */
#ifdef DNS_SLAVE
   int wait; /*wait for how many loops */
   long site_info;
   char host[60];
   u_long addr;
   time_t contime;
#endif
   int ifd;
   pid_t ipid;
};

/*
 * Attribute bonus structures.
 */
struct str_app_type
{
   sh_int tohit;
   sh_int todam;
   sh_int carry;
   sh_int wield;
   sh_int weight;
};

struct int_app_type
{
   sh_int learn;
   sh_int lore; /* Lore Int bonus */
};

struct wis_app_type
{
   sh_int practice;
   sh_int lore;
};

struct dex_app_type
{
   sh_int defensive;
};

struct con_app_type
{
   sh_int hitp;
   sh_int shock;
};

struct cha_app_type
{
   sh_int charm;
};

struct lck_app_type
{
   sh_int luck;
};

/* the races */
typedef enum
{
   RACE_HUMAN, RACE_ELF, RACE_DWARF, RACE_OGRE, RACE_HOBBIT, RACE_FAIRY, MAX_PC_RACE
}
race_types;

/* npc races */
#define	RACE_DRAGON	    31

/*
 * Languages -- Altrag
 */
#define LANG_COMMON      BV00 /* Human base language */
#define LANG_ELVEN       BV01 /* Elven base language */
#define LANG_DWARVEN     BV02 /* Dwarven base language */
#define LANG_OGRE        BV03 /* Ogre base language */
#define LANG_HOBBIT      BV04 /* Hobbit language */
#define LANG_FAIRY       BV05 /* Fairy language */
#define LANG_TROLLISH    BV06 /* Troll base language */
#define LANG_RODENT      BV07 /* Small mammals */
#define LANG_INSECTOID   BV08 /* Insects */
#define LANG_MAMMAL      BV09 /* Larger mammals */
#define LANG_REPTILE     BV10 /* Small reptiles */
#define LANG_DRAGON      BV11 /* Large reptiles, Dragons */
#define LANG_SPIRITUAL   BV12 /* Necromancers or undeads/spectres */
#define LANG_MAGICAL     BV13 /* Spells maybe?  Magical creatures */
#define LANG_GOBLIN      BV14 /* Goblin base language */
#define LANG_GOD         BV15 /* Clerics possibly?  God creatures */
#define LANG_ANCIENT     BV16 /* Prelude to a glyph read skill? */
#define LANG_HALFLING    BV17 /* Halfling base language */
#define LANG_CLAN	    BV18 /* Clan language */
#define LANG_GITH	    BV19 /* Gith Language */
#define LANG_UNKNOWN        0 /* Anything that doesnt fit a category */
#define VALID_LANGS    ( LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_HOBBIT  \
		       | LANG_OGRE | LANG_FAIRY  )
/* 18 Languages */

/*
 * TO types for act.
 */
/* CAUTION!!!  Using allmud on anything else in the code might cause crashes or
   problems, consult Xerves before using -- XERVES */
typedef enum
{ TO_ROOM, TO_NOTVICT, TO_VICT, TO_CHAR, TO_CANSEE, TO_MUD, TO_ALLMUD, TO_FIGHT }
to_types;

/*
 * Real action "TYPES" for act.
 */
/*
#define AT_BLACK	    0
#define AT_BLOOD	    1
#define AT_DGREEN           2
#define AT_ORANGE	    3
#define AT_DBLUE	    4
#define AT_PURPLE	    5
#define AT_CYAN	  	    6
#define AT_GREY		    7
#define AT_DGREY	    8
#define AT_RED		    9
#define AT_GREEN	   10
#define AT_YELLOW	   11
#define AT_BLUE		   12
#define AT_PINK		   13
#define AT_LBLUE	   14
#define AT_WHITE	   15
#define AT_BLINK	   16
#define AT_PLAIN	   AT_GREY
#define AT_ACTION	   AT_GREY
#define AT_SAY		   AT_LBLUE
#define AT_GOSSIP	   AT_LBLUE
#define AT_YELL	           AT_GREEN
#define AT_TELL		   AT_WHITE
#define AT_WHISPER	   AT_YELLOW
#define AT_HIT		   AT_WHITE
#define AT_HITME	   AT_LBLUE
#define AT_IMMORT	   AT_YELLOW
#define AT_HURT		   AT_RED
#define AT_FALLING	   AT_WHITE + AT_BLINK
#define AT_DANGER	   AT_RED + AT_BLINK
#define AT_MAGIC	   AT_BLUE
#define AT_CONSIDER	   AT_GREY
#define AT_REPORT	   AT_GREY
#define AT_POISON	   AT_GREEN
#define AT_SOCIAL	   AT_CYAN
#define AT_DYING	   AT_YELLOW
#define AT_DEAD		   AT_RED
#define AT_SKILL	   AT_GREEN
#define AT_CARNAGE	   AT_BLOOD
#define AT_DAMAGE	   AT_WHITE
#define AT_FLEE		   AT_YELLOW
#define AT_RMNAME	   AT_WHITE
#define AT_RMDESC	   AT_LBLUE
#define AT_OBJECT	   AT_GREEN
#define AT_PERSON	   AT_PINK
#define AT_LIST		   AT_BLUE
#define AT_BYE		   AT_GREEN
#define AT_GOLD		   AT_YELLOW
#define AT_GTELL	   AT_BLUE
#define AT_NOTE		   AT_GREEN
#define AT_HUNGRY	   AT_ORANGE
#define AT_THIRSTY	   AT_BLUE
#define	AT_FIRE		   AT_RED
#define AT_SOBER	   AT_WHITE
#define AT_WEAROFF	   AT_YELLOW
#define AT_EXITS	   AT_WHITE
#define AT_SCORE	   AT_LBLUE
#define AT_RESET	   AT_DGREEN
#define AT_LOG		   AT_PURPLE
#define AT_DIEMSG	   AT_WHITE
#define AT_WARTALK         AT_RED
#define AT_RACETALK	   AT_DGREEN
#define AT_IGNORE	   AT_GREEN
#define AT_DIVIDER	   AT_PLAIN
#define AT_MORPH           AT_GREY
*/

#define INIT_ARMOR_CONDITION     1000
#define MAX_ITEM_IMPACT		 30

/*
 * Help Index Table
 */
struct help_index_data
{
   char *keyword;
   HINDEX_DATA *next; //Next index
   HINDEX_DATA *prev; //Prev index
   HINDEX_DATA *fnext; //next flat index
   HINDEX_DATA *fprev; //prev flat index
   HINDEX_DATA *tnext; //Next index at the top level
   HINDEX_DATA *tprev; //Prev index at the top level
   HINDEX_DATA *first_hindex; //First index below this one
   HINDEX_DATA *last_hindex; //Last index below this one
   HINDEX_DATA *first_top_hindex; //First index above this one
   HINDEX_DATA *last_top_hindex; //Last index below this one
   HINDEX_IPOINTER *first_help;  //First helpfile in this index
   HINDEX_IPOINTER *last_help;  //Last helpfile in this index
};

//Points back to help index data since I need to store this data on every helpfile....
struct help_data_pointer
{
   HINDEX_POINTER *next;
   HINDEX_POINTER *prev;
   HINDEX_DATA *pointer;
};

//Points back to the help data since I need to store this data on every index....
struct help_index_pointer
{
   HINDEX_IPOINTER *next;
   HINDEX_IPOINTER *prev;
   HELP_DATA *pointer;
};

//Used to store the actual groups on a helpfile since it is really hard to track them back...
struct help_index_name
{
    HINDEX_NAME *next;
    HINDEX_NAME *prev;
    char *name;
};

/*
 * Help table types.
 */
struct help_data
{
   HELP_DATA *next;
   HELP_DATA *prev;
   HINDEX_POINTER *first_hindex; //First index structure helpfile is in
   HINDEX_POINTER *last_hindex; //Last index structure helpfile is in
   sh_int level;
   char *keyword;
   HINDEX_NAME *first_iname;
   HINDEX_NAME *last_iname;
   char *text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct shop_data
{
   SHOP_DATA *next; /* Next shop in list  */
   SHOP_DATA *prev; /* Previous shop in list */
   int keeper; /* Vnum of shop keeper mob */
   sh_int buy_type[MAX_TRADE]; /* Item types shop will buy */
   sh_int profit_buy; /* Cost multiplier for buying */
   sh_int profit_sell; /* Cost multiplier for selling */
   sh_int open_hour; /* First opening hour  */
   sh_int close_hour; /* First closing hour  */
};

#define MAX_FIX		3
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct repairshop_data
{
   REPAIR_DATA *next; /* Next shop in list  */
   REPAIR_DATA *prev; /* Previous shop in list */
   int keeper; /* Vnum of shop keeper mob */
   sh_int fix_type[MAX_FIX]; /* Item types shop will fix */
   sh_int profit_fix; /* Cost multiplier for fixing */
   sh_int shop_type; /* Repair shop type  */
   sh_int open_hour; /* First opening hour  */
   sh_int close_hour; /* First closing hour  */
};


/* Mob program structures */
/* Mob program structures and defines */
/* Moved these defines here from mud_prog.c as I need them -rkb */
#define MAX_IFS 20 /* should always be generous */
#define IN_IF 0
#define IN_ELSE 1
#define DO_IF 2
#define DO_ELSE 3

#define MAX_PROG_NEST 20

struct act_prog_data
{
   struct act_prog_data *next;
   void *vo;
};

struct mob_prog_act_list
{
   MPROG_ACT_LIST *next;
   char *buf;
   CHAR_DATA *ch;
   OBJ_DATA *obj;
   void *vo;
};

struct mob_prog_data
{
   MPROG_DATA *next;
   sh_int type;
   bool triggered;
   int resetdelay;
   char *arglist;
   char *comlist;
};

/* Used to store sleeping mud progs. -rkb */
typedef enum
{ MP_MOB, MP_ROOM, MP_OBJ }
mp_types;
struct mpsleep_data
{
   MPSLEEP_DATA *next;
   MPSLEEP_DATA *prev;

   int timer; /* Pulses to sleep */
   mp_types type; /* Mob, Room or Obj prog */
   ROOM_INDEX_DATA *room; /* Room when type is MP_ROOM */

   /* mprog_driver state variables */
   int ignorelevel;
   int iflevel;
   bool ifstate[MAX_IFS][DO_ELSE + 1]; //bug fix 

   /* mprog_driver arguments */
   char *com_list;
   CHAR_DATA *mob;
   CHAR_DATA *actor;
   OBJ_DATA *obj;
   void *vo;
   bool single_step;
};

bool MOBtrigger;

/*
 * Per-class stuff.
 */
struct class_type
{
   char *who_name; /* Name for 'who'  */
   sh_int attr_prime; /* Prime attribute  */
   int weapon; /* First weapon   */
   int guild; /* Vnum of guild room  */
   sh_int thac0_00; /* Thac0 for level  0  */
   sh_int thac0_32; /* Thac0 for level 32  */
   sh_int hp_min; /* Min hp gained on leveling */
   sh_int hp_max; /* Max hp gained on leveling */
   bool fMana; /* Class gains mana on level */
   int exp_base; /* Class base exp  */
   bool remort_class;
};

/* race dedicated stuff */
struct race_type
{
   char race_name[16]; /* Race name   */
   EXT_BV affected; /* Default affect bitvectors */
   sh_int str_plus; /* Str bonus/penalty  */
   sh_int dex_plus; /* Dex      "   */
   sh_int wis_plus; /* Wis      "   */
   sh_int int_plus; /* Int      "   */
   sh_int con_plus; /* Con      "   */
   sh_int cha_plus; /* Cha      "   */
   sh_int lck_plus; /* Lck      "   */
   sh_int agi_plus; // Agi      "
   sh_int agi_start; // Agi Start
   sh_int agi_range; // Range of values + and - for agility
   sh_int str_range; // Range of values + and - for str
   sh_int dex_range; // Range of values + and - for dex
   sh_int int_range; // Range of values + and - for int
   sh_int wis_range; // Range of values + and - for wis
   sh_int con_range; // Range of values + and - for con
   sh_int hit;
   sh_int mana;
   int resist;
   int suscept;
   int class_restriction; /* Flags for illegal classes */
   int language; /* Default racial language      */
   sh_int ac_plus;
   sh_int alignment;
   EXT_BV attacks;
   EXT_BV defenses;
   sh_int minalign;
   sh_int maxalign;
   sh_int exp_multiplier;
   sh_int height;
   sh_int weight;
   sh_int weaponmin; /* Weapon size Min -- Xerves */
   sh_int weaponstd; /* Weapon size Standard (not Used) -- Xerves */
   sh_int weaponmax; /* Weapon sise Max -- Xerves */
   bool remort_race;
   sh_int hunger_mod;
   sh_int thirst_mod;
   sh_int saving_poison_death;
   sh_int saving_wand;
   sh_int dodge_bonus;
   sh_int saving_para_petri;
   sh_int saving_breath;
   sh_int saving_spell_staff;
   char *where_name[MAX_WHERE_NAME];
   sh_int mana_regen;
   sh_int hp_regen;
   sh_int race_recall;
};

typedef enum
{
   CLAN_PLAIN, CLAN_VAMPIRE, CLAN_WARRIOR, CLAN_DRUID, CLAN_MAGE, CLAN_CELTIC,
   CLAN_THIEF, CLAN_CLERIC, CLAN_UNDEAD, CLAN_CHAOTIC, CLAN_NEUTRAL, CLAN_LAWFUL,
   CLAN_NOKILL, CLAN_ORDER, CLAN_GUILD
}
clan_types;

typedef enum
{
   GROUP_CLAN, GROUP_COUNCIL, GROUP_GUILD
}
group_types;


struct clan_data
{
   CLAN_DATA *next; /* next clan in list   */
   CLAN_DATA *prev; /* previous clan in list  */
   char *filename; /* Clan filename   */
   char *name; /* Clan name    */
   char *motto; /* Clan motto    */
   char *description; /* A brief description of the clan */
   char *deity; /* Clan's deity    */
   char *leader; /* Head clan leader   */
   char *number1; /* First officer   */
   char *number2; /* Second officer   */
   char *badge; /* Clan badge on who/where/to_room      */
   int pkills[7]; /* Number of pkills on behalf of clan */
   int pdeaths[7]; /* Number of pkills against clan */
   int mkills; /* Number of mkills on behalf of clan */
   int mdeaths; /* Number of clan deaths due to mobs */
   int illegal_pk; /* Number of illegal pk's by clan */
   int score; /* Overall score   */
   sh_int clan_type; /* See clan type defines  */
   sh_int favour; /* Deities favour upon the clan  */
   sh_int strikes; /* Number of strikes against the clan */
   sh_int members; /* Number of clan members  */
   sh_int mem_limit; /* Number of clan members allowed */
   sh_int alignment; /* Clan's general alignment  */
   int board; /* Vnum of clan board   */
   int clanobj1; /* Vnum of first clan obj  */
   int clanobj2; /* Vnum of second clan obj  */
   int clanobj3; /* Vnum of third clan obj  */
   int clanobj4; /* Vnum of fourth clan obj  */
   int clanobj5; /* Vnum of fifth clan obj  */
   int recall; /* Vnum of clan's recall room  */
   int storeroom; /* Vnum of clan's store room  */
   int guard1; /* Vnum of clan guard type 1  */
   int guard2; /* Vnum of clan guard type 2  */
   int class; /* For guilds    */
};

struct council_data
{
   COUNCIL_DATA *next; /* next council in list   */
   COUNCIL_DATA *prev; /* previous council in list  */
   char *filename; /* Council filename   */
   char *name; /* Council name    */
   char *description; /* A brief description of the council */
   char *head; /* Council head    */
   char *head2; /* Council co-head                      */
   char *powers; /* Council powers   */
   sh_int members; /* Number of council members  */
   int board; /* Vnum of council board  */
   int meeting; /* Vnum of council's meeting room */
};

struct deity_data
{
   DEITY_DATA *next;
   DEITY_DATA *prev;
   char *filename;
   char *name;
   char *description;
   sh_int alignment;
   sh_int worshippers;
   sh_int scorpse;
   sh_int sdeityobj;
   sh_int savatar;
   sh_int srecall;
   sh_int flee;
   sh_int flee_npcrace;
   sh_int flee_npcfoe;
   sh_int kill;
   sh_int kill_magic;
   sh_int kill_npcrace;
   sh_int kill_npcfoe;
   sh_int sac;
   sh_int bury_corpse;
   sh_int aid_spell;
   sh_int aid;
   sh_int backstab;
   sh_int steal;
   sh_int die;
   sh_int die_npcrace;
   sh_int die_npcfoe;
   sh_int spell_aid;
   sh_int dig_corpse;
   int race;
   int race2;
   int class;
   int element;
   int sex;
   EXT_BV affected;
   int npcrace;
   int npcfoe;
   int suscept;
   int susceptnum;
   int elementnum;
   int affectednum;
   int objstat;
};


struct tourney_data
{
   int open;
   int low_level;
   int hi_level;
};

/*
 * Data structure for notes.
 */
struct note_data
{
   NOTE_DATA *next;
   NOTE_DATA *prev;
   char *sender;
   char *date;
   char *to_list;
   char *subject;
   int voting;
   char *yesvotes;
   char *novotes;
   char *abstentions;
   char *text;
   time_t expire;
   time_t date_stamp;
};

struct board_data
{
   BOARD_DATA *next; /* Next board in list     */
   BOARD_DATA *prev; /* Previous board in list    */
   NOTE_DATA *first_note; /* First note on board     */
   NOTE_DATA *last_note; /* Last note on board     */
   char *note_file; /* Filename to save notes to    */
   char *read_group; /* Can restrict a board to a       */
   char *post_group; /* council, clan, guild etc        */
   char *extra_readers; /* Can give read rights to players */
   char *extra_removers; /* Can give remove rights to players */
   int board_obj; /* Vnum of board object     */
   sh_int num_posts; /* Number of notes on this board   */
   sh_int min_read_level; /* Minimum level to read a note    */
   sh_int min_post_level; /* Minimum level to post a note    */
   sh_int min_remove_level; /* Minimum level to remove a note  */
   sh_int max_posts; /* Maximum amount of notes allowed */
   int type; /* Normal board or mail board? */
};


/*
 * An affect.
 *
 * So limited... so few fields... should we add more?
 */
struct affect_data
{
   AFFECT_DATA *next;
   AFFECT_DATA *prev;
   sh_int type;
   sh_int duration;
   sh_int location;
   int modifier;
   EXT_BV bitvector;
   int gemnum;
};


/*
 * A SMAUG spell
 */
struct smaug_affect
{
   SMAUG_AFF *next;
   char *duration;
   sh_int location;
   char *modifier;
   int bitvector; /* this is the bit number */
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_VAMPIRE	   80
#define MOB_VNUM_ANIMATED_CORPSE   5
#define MOB_VNUM_POLY_WOLF	   10
#define MOB_VNUM_POLY_MIST	   11
#define MOB_VNUM_POLY_BAT	   12
#define MOB_VNUM_POLY_HAWK	   13
#define MOB_VNUM_POLY_CAT	   14
#define MOB_VNUM_POLY_DOVE	   15
#define MOB_VNUM_POLY_FISH	   16
#define MOB_VNUM_DEITY		   17
#define MOB_VNUM_TRAINER           30
#define MOB_BEGIN_LUMBERJACK       16201
#define MOB_INTER_LUMBERJACK       16202
#define MOB_ADVAN_LUMBERJACK       16203
#define MOB_EXPERT_LUMBERJACK      16204
#define MOB_MASTER_LUMBERJACK      16205
#define MOB_CORN_WORKER            16206
#define MOB_CORN_T_WORKER          16207
#define MOB_CORN_M_WORKER          16208
#define MOB_WHEAT_WORKER           16209
#define MOB_WHEAT_T_WORKER         16210
#define MOB_WHEAT_M_WORKER         16211
#define MOB_IRON_WORKER            16212
#define MOB_IRON_T_WORKER          16213
#define MOB_IRON_M_WORKER          16214
#define MOB_GOLD_WORKER            16215
#define MOB_GOLD_T_WORKER          16216
#define MOB_GOLD_M_WORKER          16217
//Start Mobs that can be used to attack/defend
#define MOB_KWARRIOR               16218
#define MOB_KCLERIC                16219
#define MOB_KMAGE                  16220
#define MOB_KARCHER                16221
#define MOB_KBARBARIAN             16222
#define MOB_KHORSEMAN              16223
#define MOB_KHORSE                 16224
#define MOB_KGUARD                 16225
#define MOB_KKNIGHT                16226
#define MOB_KSLINGER               16227
#define MOB_KCHAPLAIN              16228
#define MOB_KCROSSBOW              16229
#define MOB_KTHIEF                 16230
#define MOB_KMOB_HORSE             16300
#define MOB_PEASANT_START          20
#define MOB_PEASANT_END            25

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
 /* Due to mob flags being shared with other flags on the act, don't use the
    R# flags -- Xerves */
/*typedef enum
{
  ACT_IS_NPC, ACT_SENTINEL, ACT_SCAVENGER, ACT_BANKER, ACT_CASTEMOB, ACT_AGGRESSIVE,
  ACT_STAY_AREA, ACT_WIMPY, ACT_PET, ACT_TRAIN, ACT_PRACTICE, ACT_IMMORTAL,
  ACT_NOQUEST, ACT_POLYSELF, ACT_META_AGGR, ACT_GUARDIAN, ACT_RUNNING, ACT_NOWANDER,
  ACT_MOUNTABLE, ACT_MOUNTED, ACT_SCHOLAR, ACT_SECRETIVE, ACT_NOTRACK, ACT_MOBINVIS,
  ACT_NOASSIST, ACT_AUTONOMOUS, ACT_PACIFIST, ACT_NOATTACK, ACT_ANNOYING, ACT_HEALER,
  ACT_PROTOTYPE, ACT_R1, ACT_PROTECT, ACT_SCARED
} mob_act_types; */

//Fight states for players, just a reason to lag :-)
#define FSTATE_GRIP    BV01
#define FSTATE_LIMB    BV02
#define FSTATE_WIELD   BV03

//Elemental types for mobiles/players
#define ELEMENT_AIR    BV00
#define ELEMENT_EARTH  BV01
#define ELEMENT_WATER  BV02
#define ELEMENT_FIRE   BV03
#define ELEMENT_ENERGY BV04
#define ELEMENT_UNHOLY BV05
#define ELEMENT_DIVINE BV06
#define ELEMENT_UNDEAD BV07

#define INTRO_ATTACKER   BV00
#define INTRO_KILLER     BV01
#define INTRO_MYATTACKER BV02
#define INTRO_MYKILLER   BV03
#define INTRO_THIEF      BV04
#define INTRO_MYTHIEF    BV05


#define ACT_IS_NPC		  0 /* Auto set for mobs */
#define ACT_SENTINEL		  1 /* Stays in one room */
#define ACT_SCAVENGER		  2 /* Picks up objects */
#define ACT_BANKER                3 /* Can store money      */
#define ACT_CASTEMOB              4 /* Can change caste     */
#define ACT_AGGRESSIVE		  5 /* Attacks PC's  */
#define ACT_STAY_AREA		  6 /* Won't leave area */
#define ACT_WIMPY		  7 /* Flees when hurt */
#define ACT_PET			  8 /* Auto set for pets */
#define ACT_TRAIN		  9 /* Can train PC's */
#define ACT_PRACTICE		 10 /* Can practice PC's */
#define ACT_IMMORTAL		 11 /* Cannot be killed */
#define ACT_NOQUEST		 12 /* Deadly removed because it was not used  */
#define ACT_POLYSELF		 13
#define ACT_META_AGGR		 14 /* Attacks other mobs */
#define ACT_GUARDIAN		 15 /* Protects master */
#define ACT_RUNNING		 16 /* Hunts quickly */
#define ACT_NOWANDER		 17 /* Doesn't wander */
#define ACT_MOUNTABLE		 18 /* Can be mounted */
#define ACT_MOUNTED		 19 /* Is mounted  */
#define ACT_SCHOLAR              20 /* Can teach languages  */
#define ACT_SECRETIVE		 21 /* actions aren't seen */
#define ACT_NOTRACK            22 /* Cannot Track --Xerves */
#define ACT_MOBINVIS		 23 /* Like wizinvis */
#define ACT_NOASSIST		 24 /* Doesn't assist mobs */
#define ACT_AUTONOMOUS		 25 /* Doesn't auto switch tanks */
#define ACT_PACIFIST             26 /* Doesn't ever fight   */
#define ACT_NOATTACK		 27 /* No physical attacks */
#define ACT_ANNOYING		 28 /* Other mobs will attack */
#define ACT_HEALER             29 /* Sell spells */
#define ACT_PROTOTYPE		 30 /* A prototype mob */
#define ACT_R1                   31 /* DO NOT USE!!!! DO NOT REPLACE!!! */
#define ACT_PROTECT              32 /* Protects against lower level changes */
#define ACT_SCARED               33 /* Scared flag, flipside of nowander -- Xerves */
#define ACT_TRAINER              34 /* Can teach skills/groups, part of new system */
#define ACT_ONMAP                35 /* Solon Map */
#define ACT_MOUNTSAVE            36 /* Saveable Mount */
#define ACT_WATERMOB             37 /* Mob only loads in the water areas of Rafermand */
#define ACT_EXTRACTMOB		 38 // Can Hire this mob to do work -- Xerves
#define ACT_DUMPGOODS		 39 // Is in the process of dumping goods -- Xerves
#define ACT_NOUPDN		 40 // Cannot automatically move up or down. -- Xerves
#define ACT_MOVEMAP		 41 // Can move on the map -- Xerves
#define ACT_UNDEAD		42 //Only hurt by blessed/sanctified weapons... noblood -- SKAN
#define ACT_KINGDOMMOB     43 //To keep track of number of mobs loading in a kingdom
#define ACT_MILITARY       44 //Soldier mob buyable by kingdoms
#define ACT_SBSELLER       45 //Sells Spell Books
#define ACT_RESTORELIMBS   46 //Mob that can restore limbs
#define ACT_LIVING_DEAD	   47 //Unblessed/sanctified weapons do 1/2 damage, blessed/sanct'd weapons do 2x dmg -- SKAN
#define ACT_NORESET        48 //Flag that is added to a mob that does not reset, do not set manually....
#define ACT_TIMERESET      49 //Flag that is added to a mob that is a time based reset, do not set manually....
#define ACT_EXTRACTGOODS   50 //Is in the process of extracting goods -- Xerves
#define ACT_REPAIR         51 //Repair mobile, only really needed for kingdom purchased repair mobiles
#define ACT_NOINSTA        52 //No Instant kill
#define ACT_NOMERCY        53 //Ha ha show nomercy to a player and kill them if they are wounded!!!!
#define ACT_FORGEMOB       54 //More for mxp than anything, the flag isn't need (it is in the room)
#define ACT_EXTRACTTOWN    55 //Will dump goods into a town instead of a bin
#define ACT_GRABBED        56 //Has had a grab attempt done, will not allow more than 1
#define ACT_CAPTAIN        57 //Mob is a captain in a quest
#define ACT_BOSS           58 //Mob is a boss in a quest
#define ACT_QUESTMOB       59 //Mob hands out quests
#define ACT_ALLOWRIDE      60 //Mob allows itself to be piggybacked

/* Used in portalloc for PCs, able to go to portal locations using it */
typedef enum
{
   PT_P1, PT_P2, PT_P3, PT_P4, PT_P5, PT_P6, PT_P7, PT_P8, PT_P9, PT_P10,
   PT_P11, PT_P12, PT_P13, PT_P14, PT_P15, PT_P16, PT_P17, PT_P18, PT_P19, PT_P20,
   PT_P21, PT_P22, PT_P23, PT_P24, PT_P25, PT_P26, PT_P27, PT_P28, PT_P29, PT_P30,
   PT_P31, PT_P32, PT_P33, PT_P34, PT_P35, PT_P36, PT_P37, PT_P38, PT_P39, PT_P40,
   PT_P41, PT_P42, PT_P43, PT_P44, PT_P45, PT_P46, PT_P47, PT_P48, PT_P49, PT_P50,
   PT_P51, PT_P52, PT_P53, PT_P54, PT_P55, PT_P56, PT_P57, PT_P58, PT_P59, PT_P60,
   PT_P61, PT_P62, PT_P63, PT_P64, PT_P65, PT_P66, PT_P67, PT_P68, PT_P69, PT_P70,
   PT_P71, PT_P72, PT_P73, PT_P74, PT_P75, PT_P76, PT_P77, PT_P78, PT_P79, PT_P80,
}
portal_types;

typedef enum
{
   KM_STATIONARY, KM_PATROL, KM_WARN, KM_ATTACKE, KM_ATTACKN, KM_ATTACKA,
   KM_NOPASS, KM_REPORT, KM_INVITE, KM_NOASSIST, KM_SENTINEL, KM_CONQUER,
   KM_NOCLOAK, KM_NOHOOD, KM_ATTACKH, KM_ATTACKC, KM_NEEDINTRO, KM_KEQUIPED
}
kmob_types;

//KLOG_LASTENTRY Needs to be the last flag
typedef enum
{
   KLOG_SETJOB, KLOG_TRADEGOODS, KLOG_RESOURCES, KLOG_PLACEOBJ, KLOG_KREMOVE_TRAINER, KLOG_KREMOVE_OBJ, KLOG_PLACETRAINER,
   KLOG_PLACEMOB, KLOG_PLANTGRAIN, KLOG_PLANTCORN, KLOG_PLANTGRASS, KLOG_PLANTTREE, KLOG_STOPFIRE, KLOG_PLAINS, KLOG_SURVEY,
   KLOG_SURVEY_STRIKES, KLOG_LEAVEKINGDOM, KLOG_TKICKOUT, KLOG_KICKOUT, KLOG_TINDUCT, KLOG_KINDUCT, KLOG_JOINKINGDOM,
   KLOG_DECLARE, KLOG_COMMAND, KLOG_GIVEORDERS, KLOG_USELICENSE, KLOG_GRANTLICENSE, KLOG_ROADS, KLOG_FIRE, KLOG_CUTPATH,
   KLOG_MAKEWORKER, KLOG_GETRESOURCES, KLOG_DEPOSIT, KLOG_WITHDRAW, KLOG_CREATE_ROOM, KLOG_EDIT_ROOM, KLOG_POP_VICTIM,
   KLOG_POP_ATTACKER, KLOG_MIL_VICTIM, KLOG_MIL_ATTACKER, KLOG_MIL_COLLECTION, KLOG_TAX, KLOG_POPULATION, KLOG_EXTRACTION,
   KLOG_TRAINERTAX, KLOG_BOOKTAX, KLOG_CASTEVALUES, KLOG_TOWNVALUES, KLOG_KREMOVE_MOB, KLOG_WARLOSSES, KLOG_UNKNOWN,
   KLOG_SCHEDULE, KLOG_ARMMILITARY, KLOG_LASTENTRY
}
klog_types;
/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 *
 * hold and flaming are yet uncoded
 */
 
//Aqua Breath - 32, everything after must use APPLY_EXT_AFFECT to apply the affect
typedef enum
{
   AFF_BLIND, AFF_INVISIBLE, AFF_DETECT_EVIL, AFF_DETECT_INVIS,
   AFF_DETECT_MAGIC, AFF_DETECT_HIDDEN, AFF_HOLD, AFF_SANCTUARY,
   AFF_FAERIE_FIRE, AFF_INFRARED, AFF_CURSE, AFF_FLAMING, AFF_POISON,
   AFF_PROTECT, AFF_PARALYSIS, AFF_SNEAK, AFF_HIDE, AFF_SLEEP, AFF_CHARM,
   AFF_FLYING, AFF_PASS_DOOR, AFF_FLOATING, AFF_TRUESIGHT, AFF_DETECTTRAPS,
   AFF_SCRYING, AFF_FIRESHIELD, AFF_SHOCKSHIELD, AFF_HAUS1, AFF_ICESHIELD,
   AFF_POSSESS, AFF_BERSERK, AFF_AQUA_BREATH, AFF_RECURRINGSPELL,
   AFF_CONTAGIOUS, AFF_WIZARDEYE, AFF_E_WIZARDEYE, AFF_M_WIZARDEYE, AFF_BALANCE, 
   AFF_NOHUNGER, AFF_NOTHIRST, AFF_GAGGED, AFF_REZ, AFF_WEB, AFF_SNARE, 
   AFF_NERVEPINCH, AFF_NYIJI, AFF_STALK, MAX_AFFECTED_BY
}
affected_by_types;

typedef enum
{
   TALENT_HP1, TALENT_HP2, TALENT_HP3, TALENT_HP4, TALENT_HP5, TALENT_MP1, TALENT_MP2,
   TALENT_MP3, TALENT_MP4, TALENT_MP5, TALENT_STR1, TALENT_STR2, TALENT_STR3, TALENT_CON1,
   TALENT_CON2, TALENT_CON3, TALENT_INT1, TALENT_INT2, TALENT_INT3, TALENT_WIS1, TALENT_WIS2,
   TALENT_WIS3, TALENT_DEX1, TALENT_DEX2, TALENT_DEX3, TALENT_AGI1, TALENT_AGI2, TALENT_AGI3,
   TALENT_END1, TALENT_END2, TALENT_END3, TALENT_SP1, TALENT_SP2, TALENT_SP3, TALENT_SP4,
   TALENT_SP5, TALENT_DAMCAP1, TALENT_DAMCAP2, TALENT_DAMCAP3, TALENT_MAX
}
talent_types;

/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  BV00
#define RIS_WATER   	  BV01
#define RIS_EARTH	     BV02
#define RIS_ENERGY		BV03
#define RIS_BLUNT		 BV04
#define RIS_PIERCE		BV05
#define RIS_SLASH		 BV06
#define RIS_ACID		  BV07 //Not used
#define RIS_POISON		BV08
#define RIS_DRAIN		 BV09 //Not used
#define RIS_SLEEP		 BV10
#define RIS_CHARM		 BV11
#define RIS_HOLD		  BV12 //Not used
#define RIS_NONMAGIC	  BV13
#define RIS_PLUS1		 BV14
#define RIS_PLUS2		 BV15
#define RIS_PLUS3		 BV16
#define RIS_PLUS4		 BV17
#define RIS_PLUS5		 BV18
#define RIS_PLUS6		 BV19
#define RIS_MAGIC		 BV20
#define RIS_PARALYSIS	 BV21 //Stun
#define RIS_AIR           BV22
#define RIS_UNHOLY        BV23
#define RIS_HOLY          BV24
#define RIS_UNDEAD        BV25
/* 26 RIS's*/

/* 
 * Attack types
 */
typedef enum
{
   ATCK_BITE, ATCK_CLAWS, ATCK_TAIL, ATCK_STING, ATCK_PUNCH, ATCK_KICK,
   ATCK_TRIP, ATCK_BASH, ATCK_STUN, ATCK_GOUGE, ATCK_BACKSTAB, ATCK_FEED,
   ATCK_DRAIN, ATCK_FIREBREATH, ATCK_FROSTBREATH, ATCK_ACIDBREATH,
   ATCK_LIGHTNBREATH, ATCK_GASBREATH, ATCK_POISON, ATCK_NASTYPOISON, ATCK_GAZE,
   ATCK_BLINDNESS, ATCK_CAUSESERIOUS, ATCK_EARTHQUAKE, ATCK_CAUSECRITICAL,
   ATCK_CURSE, ATCK_FLAMESTRIKE, ATCK_HARM, ATCK_FIREBALL, ATCK_COLORSPRAY,
   ATCK_WEAKEN, ATCK_SPIRALBLAST, MAX_ATTACK_TYPE
}
attack_types;

/*
 * Defense types
 */
typedef enum
{
   DFND_PARRY, DFND_DODGE, DFND_HEAL, DFND_CURELIGHT, DFND_CURESERIOUS,
   DFND_CURECRITICAL, DFND_DISPELMAGIC, DFND_DISPELEVIL, DFND_SANCTUARY,
   DFND_FIRESHIELD, DFND_SHOCKSHIELD, DFND_SHIELD, DFND_BLESS, DFND_STONESKIN,
   DFND_TELEPORT, DFND_MONSUM1, DFND_MONSUM2, DFND_MONSUM3, DFND_MONSUM4,
   DFND_DISARM, DFND_ICESHIELD, DFND_GRIP, DFND_TRUESIGHT, MAX_DEFENSE_TYPE
}
defense_types;

/*
 * Body parts
 */
#define PART_HEAD		  BV00
#define PART_ARMS		  BV01
#define PART_LEGS		  BV02
#define PART_HEART		  BV03
#define PART_BRAINS		  BV04
#define PART_GUTS		  BV05
#define PART_HANDS		  BV06
#define PART_FEET		  BV07
#define PART_FINGERS		  BV08
#define PART_EAR		  BV09
#define PART_EYE		  BV10
#define PART_LONG_TONGUE	  BV11
#define PART_EYESTALKS		  BV12
#define PART_TENTACLES		  BV13
#define PART_FINS		  BV14
#define PART_WINGS		  BV15
#define PART_TAIL		  BV16
#define PART_SCALES		  BV17
/* for combat */
#define PART_CLAWS		  BV18
#define PART_FANGS		  BV19
#define PART_HORNS		  BV20
#define PART_TUSKS		  BV21
#define PART_TAILATTACK		  BV22
#define PART_SHARPSCALES	  BV23
#define PART_BEAK		  BV24

#define PART_HAUNCH		  BV25
#define PART_HOOVES		  BV26
#define PART_PAWS		  BV27
#define PART_FORELEGS		  BV28
#define PART_FEATHERS		  BV29

/*
 * Autosave flags
 */
#define SV_DEATH		  BV00 /* Save on death */
#define SV_KILL			  BV01 /* Save when kill made */
#define SV_PASSCHG		  BV02 /* Save on password change */
#define SV_DROP			  BV03 /* Save on drop */
#define SV_PUT			  BV04 /* Save on put */
#define SV_GIVE			  BV05 /* Save on give */
#define SV_AUTO			  BV06 /* Auto save every x minutes (define in cset) */
#define SV_ZAPDROP		  BV07 /* Save when eq zaps */
#define SV_AUCTION		  BV08 /* Save on auction */
#define SV_GET			  BV09 /* Save on get */
#define SV_RECEIVE		  BV10 /* Save when receiving */
#define SV_IDLE			  BV11 /* Save when char goes idle */
#define SV_BACKUP		  BV12 /* Make backup of pfile on save */
#define SV_QUITBACKUP		  BV13 /* Backup on quit only --Blod */

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT		  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH		  BV08

/*
 * Flags for act_string -- Shaddai
 */
#define STRING_NONE               0
#define STRING_IMM                BV01


/*
 * old flags for conversion purposes -- will not conflict with the flags below
 */
#define OLD_SF_SAVE_HALF_DAMAGE	  BV18 /* old save for half damage */
#define OLD_SF_SAVE_NEGATES	  BV19 /* old save negates affect */

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		    BV00
#define SF_EARTH		    BV01
#define SF_AIR			  BV02
#define SF_ASTRAL	 	  BV03
#define SF_AREA			 BV04 /* is an area spell  */
#define SF_DISTANT		  BV05 /* affects something far away */
#define SF_REVERSE		  BV06
#define SF_NOSELF		   BV07 /* Can't target yourself! */
#define SF_UNUSED2		  BV08 /* free for use!  */
#define SF_ACCUMULATIVE     BV09 /* is accumulative  */
#define SF_RECASTABLE 	  BV10 /* can be refreshed  */
#define SF_NOSCRIBE		 BV11 /* cannot be scribed  */
#define SF_NOBREW		   BV12 /* cannot be brewed  */
#define SF_GROUPSPELL 	  BV13 /* only affects group members */
#define SF_OBJECT		   BV14 /* directed at an object */
#define SF_CHARACTER	    BV15 /* directed at a character */
#define SF_SECRETSKILL	  BV16 /* hidden unless learned */
#define SF_PKSENSITIVE	  BV17 /* much harder for plr vs. plr */
#define SF_STOPONFAIL	   BV18 /* stops spell on first failure */
#define SF_NOFIGHT		  BV19 /* stops if char fighting       */
#define SF_NODISPEL         BV20 /* stops spell from being dispelled */
#define SF_DEQ              BV21 /* Spell damages equipment */
#define SF_NOAPPLYSTACK     BV22 /* Won't allow an affect to stack, such if there are 3 strengths of a spell */
typedef enum
{ SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI,
   SS_BREATH, SS_SPELL_STAFF
}
save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)
#define SSAV_MASK		ALL_BITS & ~(BV11 | BV12 | BV13)

typedef enum
{ SD_NONE, SD_FIRE, SD_WATER, SD_EARTH, SD_ENERGY, SD_AIR,
   SD_HOLY, SD_UNHOLY, SD_UNDEAD
}
spell_dam_types;

typedef enum
{ SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
   SA_DIVINATE, SA_OBSCURE, SA_CHANGE
}
spell_act_types;

typedef enum
{ SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR }
spell_power_types;

typedef enum
{ SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
   SC_LIFE, SC_DEATH, SC_ILLUSION
}
spell_class_types;

typedef enum
{ SE_NONE, SE_NEGATE, SE_EIGHTHDAM, SE_QUARTERDAM, SE_HALFDAM,
   SE_3QTRDAM, SE_REFLECT, SE_ABSORB
}
spell_save_effects;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum
{ SEX_NEUTRAL, SEX_MALE, SEX_FEMALE }
sex_types;

typedef enum
{
   TRAP_TYPE_POISON_GAS = 1, TRAP_TYPE_POISON_DART, TRAP_TYPE_POISON_NEEDLE,
   TRAP_TYPE_POISON_DAGGER, TRAP_TYPE_POISON_ARROW, TRAP_TYPE_BLINDNESS_GAS,
   TRAP_TYPE_SLEEPING_GAS, TRAP_TYPE_FLAME, TRAP_TYPE_EXPLOSION,
   TRAP_TYPE_ACID_SPRAY, TRAP_TYPE_ELECTRIC_SHOCK, TRAP_TYPE_BLADE,
   TRAP_TYPE_SEX_CHANGE
}
trap_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

//Normal trap flags, DO NOT add any new flags here unless you want to use old trap code
#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

//New trap flags for the Linked list system using the trap command.  Add new flags here.
typedef enum
{
  NEW_TRAP_ROOM, NEW_TRAP_OBJ, NEW_TRAP_ENTER_ROOM, NEW_TRAP_LEAVE_ROOM, NEW_TRAP_OPEN, NEW_TRAP_CLOSE, 
  NEW_TRAP_GET, NEW_TRAP_PUT, NEW_TRAP_PICK, NEW_TRAP_UNLOCK, NEW_TRAP_N, NEW_TRAP_S, NEW_TRAP_E, NEW_TRAP_W, 
  NEW_TRAP_U, NEW_TRAP_D, NEW_TRAP_EXAMINE, NEW_TRAP_NE, NEW_TRAP_NW, NEW_TRAP_SE, NEW_TRAP_SW,
  NEW_TRAP_GETOBJ, NEW_TRAP_PUTOBJ, NEW_TRAP_WEAROBJ, NEW_TRAP_DROPOBJ, NEW_TRAP_IDENTOBJ, NEW_TRAP_GIVEOBJ,
  NEW_TRAP_SACOBJ, NEW_TRAP_REMOVEOBJ
}
new_trap_flags;

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      2
#define OBJ_VNUM_MONEY_SOME	      3

#define OBJ_VNUM_CORPSE_NPC	     10
#define OBJ_VNUM_CORPSE_PC	     11
#define OBJ_VNUM_SEVERED_HEAD	     12
#define OBJ_VNUM_TORN_HEART	     13
#define OBJ_VNUM_SLICED_ARM	     14
#define OBJ_VNUM_SLICED_LEG	     15
#define OBJ_VNUM_SPILLED_GUTS	     16
#define OBJ_VNUM_BLOOD		     17
#define OBJ_VNUM_BLOODSTAIN	     18
#define OBJ_VNUM_SCRAPS		     19

#define OBJ_VNUM_MUSHROOM	     20
#define OBJ_VNUM_LIGHT_BALL	     21
#define OBJ_VNUM_SPRING		     22

#define OBJ_VNUM_SKIN		     23
#define OBJ_VNUM_SLICE		     24
#define OBJ_VNUM_SHOPPING_BAG	     25

#define OBJ_VNUM_BLOODLET	     26

#define OBJ_VNUM_FIRE		     30
#define OBJ_VNUM_TRAP		     31
#define OBJ_VNUM_PORTAL		     32
#define OBJ_VNUM_IDOL            88
#define OBJ_VNUM_TINDER          89

#define OBJ_VNUM_BLACK_POWDER	       33
#define OBJ_VNUM_SCROLL_SCRIBING       34
#define OBJ_VNUM_FLASK_BREWING         35
#define OBJ_VNUM_NOTE		           36
#define OBJ_VNUM_FLASK_BREWING_TIER2   38
#define OBJ_VNUM_FLASK_BREWING_TIER3   39
#define OBJ_VNUM_SCROLL_SCRIBING_TIER2 40
#define OBJ_VNUM_SCROLL_SCRIBING_TIER3 41
#define OBJ_EXTRADIMENSIONAL_PORTAL    63
#define OBJ_VNUM_DEITY		           64
#define OBJ_VNUM_QUESTOBJ              66

//Kingdom vnums
#define OBJ_KINGDOM_MERCHANT      9114
#define OBJ_KINGDOM_MAYOR         9113
#define OBJ_KINGDOM_QUIVER        16175
#define OBJ_KINGDOM_KEY           16128
#define OBJ_MIL_CROSSBOW          91
#define OBJ_MIL_QUIVER            21326
//forge shit
#define OBJ_FORGE_COPPERSLAB		      21101
#define OBJ_FORGE_BRONZESLAB		      21103
#define OBJ_FORGE_IRONSLAB                    21105
#define OBJ_FORGE_STEELSLAB                   21107
#define OBJ_FORGE_D_STEELSLAB                 21109
#define OBJ_FORGE_MITHRILSLAB                 21111
#define OBJ_FORGE_D_MITHRIL                   21113
#define OBJ_FORGE_BRASS                       21115
#define OBJ_FORGE_COBALT                      21117
#define OBJ_FORGE_TITANIUM                    21119
#define OBJ_FORGE_TUNGSTEN                    21121

#define OBJ_FORGE_MAEGLUIN                    21123
#define OBJ_FORGE_GURTHNAI                    21125
#define OBJ_FORGE_DARGLIN                     21127
#define OBJ_FORGE_VALTHRAN                    21129
#define OBJ_FORGE_SARNOS                      21131

#define OBJ_FORGE_ANGANAR                     21133
#define OBJ_FORGE_FALASIN                     21135
#define OBJ_FORGE_KHELEKIR                    21137
#define OBJ_FORGE_URVAAL                      21139
#define OBJ_FORGE_RAUKONAR                    21141

#define OBJ_FORGE_HAND_AXE                    21000
#define OBJ_FORGE_AXE                         21001
#define OBJ_FORGE_WARAXE                      21002
#define OBJ_FORGE_DOUBLE_AXE                  21003
#define OBJ_FORGE_MATTOCK                     21004
#define OBJ_FORGE_GREAT_AXE                   21005
#define OBJ_FORGE_BATTLE_AXE                  21006
#define OBJ_FORGE_SHORT_SWORD                 21007
#define OBJ_FORGE_CUTLASS                     21008
#define OBJ_FORGE_RAPIER                      21009
#define OBJ_FORGE_KATANA                      21010
#define OBJ_FORGE_BROAD_SWORD                 21011
#define OBJ_FORGE_LONG_SWORD                  21012
#define OBJ_FORGE_BASTARD_SWORD               21013
#define OBJ_FORGE_CLAYMORE                    21014
#define OBJ_FORGE_FLAMBERGE                   21015
#define OBJ_FORGE_KNIFE                       21016
#define OBJ_FORGE_DAGGER                      21017
#define OBJ_FORGE_DIRK                        21018
#define OBJ_FORGE_KRIS                        21019
#define OBJ_FORGE_CLEAVER                     21020
#define OBJ_FORGE_MAIN_GAUCHE                 21021
#define OBJ_FORGE_STILETTO                    21022
#define OBJ_FORGE_PILUM                       21023
#define OBJ_FORGE_LANCE                       21024
#define OBJ_FORGE_SPEAR                       21025
#define OBJ_FORGE_HALBERD                     21026
#define OBJ_FORGE_GLAIVE                      21027
#define OBJ_FORGE_GUISARME                    21028
#define OBJ_FORGE_TRIDENT                     21029
#define OBJ_FORGE_CLUB                        21030
#define OBJ_FORGE_HAMMER                      21031
#define OBJ_FORGE_MACE                        21032
#define OBJ_FORGE_WAR_HAMMER                  21033
#define OBJ_FORGE_FLAIL                       21034
#define OBJ_FORGE_GREAT_FLAIL                 21035
#define OBJ_FORGE_MORNING_STAR                21036
#define OBJ_FORGE_MAUL                        21037
#define OBJ_FORGE_SCEPTRE                     21038
#define OBJ_FORGE_ROD                         21039
#define OBJ_FORGE_WEIGHTED_ROD                21040
#define OBJ_FORGE_STAFF                       21041
#define OBJ_FORGE_QUARTER_STAFF               21042
#define OBJ_FORGE_BATTLE_STAFF                21043
#define OBJ_FORGE_BLADED_STAFF                21044

#define OBJ_FORGE_STUDDED_LEATHER_ARMOR       21206
#define OBJ_FORGE_STUDDED_LEATHER_GAUNTLET    21207
#define OBJ_FORGE_STUDDED_LEATHER_GREAVE      21208
#define OBJ_FORGE_STUDDED_LEATHER_NECK        21209
#define OBJ_FORGE_STUDDED_LEATHER_HEAD        21210

#define OBJ_FORGE_CHAIN_MAIL                  21045
#define OBJ_FORGE_CHAIN_HAUBERK               21046
#define OBJ_FORGE_RING_MAIL                   21047
#define OBJ_FORGE_DOUBLE_RING_MAIL            21048
#define OBJ_FORGE_BREASTPLATE                 21049
#define OBJ_FORGE_CUIRASS                     21050
#define OBJ_FORGE_CHAIN_GAUNTLET              21051
#define OBJ_FORGE_RING_GAUNTLET               21052
#define OBJ_FORGE_GAUNTLET                    21053
#define OBJ_FORGE_VAMBRACE                    21054
#define OBJ_FORGE_CHAIN_GREAVE	              21055
#define OBJ_FORGE_RING_GREAVE	              21056
#define OBJ_FORGE_GREAVE                      21057
#define OBJ_FORGE_CUISS                       21058
#define OBJ_FORGE_AVENTAIL                    21059
#define OBJ_FORGE_COIF                        21060
#define OBJ_FORGE_DOUBLE_COIF                 21061
#define OBJ_FORGE_GORGET                      21062
#define OBJ_FORGE_CABASSET	              21063
#define OBJ_FORGE_CASQUE 	              21064
#define OBJ_FORGE_ARMET 	              21065
#define OBJ_FORGE_HEAUME                      21066
#define OBJ_FORGE_BUCKLER                     21067
#define OBJ_FORGE_ROUNDSHIELD                 21068
#define OBJ_FORGE_HEATER                      21069
#define OBJ_FORGE_KITESHIELD                  21070
#define OBJ_FORGE_TOWERSHIELD                 21071
#define OBJ_FORGE_PAVIS                       21072
#define OBJ_FORGE_ARROW                       21073

//spellbooks
#define OBJ_FIRST_SBOOK           4100
#define OBJ_LAST_SBOOK            4399


/*
 * Item types.
 * Used in #OBJECTS.
 */
/* 66 at Holdresource -- Xerves 12/99 */
typedef enum
{
   ITEM_NONE, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON,
   ITEM_FIREWEAPON, ITEM_MISSILE, ITEM_TREASURE, ITEM_ARMOR, ITEM_POTION,
   ITEM_WORN, ITEM_FURNITURE, ITEM_TRASH, ITEM_OLDTRAP, ITEM_CONTAINER,
   ITEM_NOTE, ITEM_DRINK_CON, ITEM_KEY, ITEM_FOOD, ITEM_MONEY, ITEM_PEN,
   ITEM_BOAT, ITEM_CORPSE_NPC, ITEM_CORPSE_PC, ITEM_FOUNTAIN, ITEM_PILL,
   ITEM_BLOOD, ITEM_BLOODSTAIN, ITEM_SCRAPS, ITEM_PIPE, ITEM_HERB_CON,
   ITEM_HERB, ITEM_INCENSE, ITEM_FIRE, ITEM_BOOK, ITEM_SWITCH, ITEM_LEVER,
   ITEM_PULLCHAIN, ITEM_BUTTON, ITEM_DIAL, ITEM_RUNE, ITEM_RUNEPOUCH,
   ITEM_MATCH, ITEM_TRAP, ITEM_MAP, ITEM_PORTAL, ITEM_PAPER,
   ITEM_TINDER, ITEM_LOCKPICK, ITEM_SPIKE, ITEM_DISEASE, ITEM_OIL, ITEM_FUEL,
   ITEM_EMPTY1, ITEM_EMPTY2, ITEM_MISSILE_WEAPON, ITEM_PROJECTILE, ITEM_QUIVER,
   ITEM_SHOVEL, ITEM_SALVE, ITEM_COOK, ITEM_KEYRING, ITEM_ODOR, ITEM_MOUNTFOOD,
   ITEM_HOLDRESOURCE, ITEM_EXTRACTOBJ, ITEM_SPELLBOOK, ITEM_SHEATH, ITEM_NOTEBOARD,
   ITEM_REPAIR, ITEM_MCLIMB, ITEM_GAG, ITEM_FLINT, ITEM_TRAPTOOL, ITEM_TGEM,
   ITEM_QTOKEN,
   ITEM_LASTTYPE
}
item_types;
#define MAX_ITEM_TYPE		     ITEM_LASTTYPE-1

//Different kind of "hits/misses"
#define DM_MISS                  -1
#define DM_HIT                   0
#define DM_COUNTER               1
#define DM_PENETRATE_PERCUSSION  2
#define DM_CRITICAL              3
#define DM_DEATH                 4
#define DM_UNDEAD		 5
#define DM_BLOCK                 6
#define DM_SLICEDLIMB            7

//Defines Armor Size types
#define ASIZE_LEATHER            1
#define ASIZE_LIGHT              2
#define ASIZE_MEDIUM             3
#define ASIZE_HEAVY              4
#define ASIZE_HEAVIEST           5

//Different limbs that you can target
#define LM_BODY                  0
#define LM_RARM		         1
#define LM_LARM			 2
#define LM_RLEG			 3
#define LM_LLEG			 4
#define LM_HEAD			 5
#define LM_NECK			 6


/*
 * Extra flags.
 * Used in #OBJECTS.
 */
typedef enum
{
   ITEM_GLOW, ITEM_HUM, ITEM_DARK, ITEM_LOYAL, ITEM_EVIL, ITEM_INVIS, ITEM_MAGIC,
   ITEM_NODROP, ITEM_BLESS, ITEM_ANTI_GOOD, ITEM_ANTI_EVIL, ITEM_ANTI_NEUTRAL,
   ITEM_NOREMOVE, ITEM_INVENTORY, ITEM_ANTI_MAGE, ITEM_ANTI_THIEF,
   ITEM_ANTI_WARRIOR, ITEM_ANTI_CLERIC, ITEM_ORGANIC, ITEM_METAL, ITEM_DONATION,
   ITEM_CLANOBJECT, ITEM_CLANCORPSE, ITEM_ANTI_VAMPIRE, ITEM_ANTI_DRUID,
   ITEM_HIDDEN, ITEM_POISONED, ITEM_COVERING, ITEM_DEATHROT, ITEM_BURIED,
   ITEM_PROTOTYPE, ITEM_NOLOCATE, ITEM_GROUNDROT, ITEM_NOGIVE, ITEM_NOPURGE,
   ITEM_ANTI_MONK, ITEM_ANTI_PALADIN, ITEM_ANTI_RANGER, ITEM_ANTI_AUGURER,
   ITEM_NODISARM, ITEM_NOBREAK, ITEM_ONMAP, ITEM_LODGED, ITEM_PIECE, ITEM_ARTIFACT,
   ITEM_FORGEABLE, ITEM_COAL, ITEM_ORE, ITEM_SLAB, ITEM_TWOHANDED,ITEM_IMBUABLE,
   ITEM_GEM, ITEM_GEM_SETTING, ITEM_SANCTIFIED, ITEM_NORESET, ITEM_TIMERESET,
   ITEM_CLOAK, ITEM_MORTAR, ITEM_POWREAG, ITEM_AFFREAG, ITEM_MIXED, ITEM_PERMREAG, ITEM_HIDEIDENTITY, 
   ITEM_REPAIRWALL, ITEM_KINGDOMKEY, ITEM_KINGDOMEQ, ITEM_GAGREMOVE, 
   ITEM_CORPSEREVIVE, ITEM_UNIQUE, ITEM_QUESTOBJ, ITEM_TYPE_SWORD, ITEM_TYPE_AXE, ITEM_TYPE_DAGGER,
   ITEM_TYPE_POLEARM, ITEM_TYPE_STAVES, ITEM_TYPE_BLUNT, ITEM_QTOKEN_LOOTED, 
   ITEM_MONKWEAPON,
   ITEM_MAX_ITEM_FLAG
}
item_extra_flags;

/* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING		BV00
#define ITEM_BACKSTABBER  	BV01
#define ITEM_BANE		BV02
#define ITEM_MAGIC_LOYAL	BV03
#define ITEM_HASTE		BV04
#define ITEM_DRAIN		BV05
#define ITEM_LIGHTNING_BLADE  	BV06
#define ITEM_PKDISARMED		BV07 /* Maybe temporary, not a perma flag */

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02

// Grip flags, used for how you hold a weapon, example: slash

#define GRIP_BASH   BV00
#define GRIP_STAB   BV01
#define GRIP_SLASH  BV02

/*
 * Wear flags.
 * Used in #OBJECTS.
 */

#define ITEM_TAKE		     BV00
#define ITEM_WEAR_FINGER      BV01
#define ITEM_WEAR_NECK 	   BV02
#define ITEM_WEAR_BODY 	   BV03
#define ITEM_WEAR_HEAD 	   BV04
#define ITEM_WEAR_LEGS	    BV05
#define ITEM_WEAR_ARMS 	   BV06
#define ITEM_WEAR_SHIELD      BV07
#define ITEM_WEAR_WAIST       BV08
#define ITEM_WIELD		    BV09
#define ITEM_DUAL_WIELD       BV10
#define ITEM_MISSILE_WIELD    BV11
#define ITEM_LODGE_RIB	    BV12
#define ITEM_LODGE_ARM	    BV13
#define ITEM_LODGE_LEG	    BV14
#define ITEM_WEAR_ABOUT_NECK  BV15
#define ITEM_WEAR_NOCKED      BV16
#define ITEM_WEAR_BACK        BV17 //added back in for backpacks, etc

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
//94 at arrowcatch
typedef enum
{
   APPLY_NONE, APPLY_STR, APPLY_DEX, APPLY_INT, APPLY_WIS, APPLY_CON,
   APPLY_SEX, APPLY_CLASS, APPLY_LEVEL, APPLY_AGE, APPLY_HEIGHT, APPLY_WEIGHT,
   APPLY_MANA, APPLY_HIT, APPLY_MOVE, APPLY_GOLD, APPLY_EXP, APPLY_AC,
   APPLY_HITROLL, APPLY_DAMROLL, APPLY_SAVING_POISON, APPLY_SAVING_ROD,
   APPLY_SAVING_PARA, APPLY_SAVING_BREATH, APPLY_SAVING_SPELL, APPLY_CHA,
   APPLY_AFFECT, APPLY_RESISTANT, APPLY_IMMUNE, APPLY_SUSCEPTIBLE,
   APPLY_WEAPONSPELL, APPLY_LCK, APPLY_BACKSTAB, APPLY_PICK, APPLY_TRACK,
   APPLY_STEAL, APPLY_SNEAK, APPLY_HIDE, APPLY_PALM, APPLY_DETRAP, APPLY_DODGE,
   APPLY_PEEK, APPLY_SCAN, APPLY_GOUGE, APPLY_SEARCH, APPLY_MOUNT, APPLY_DISARM,
   APPLY_KICK, APPLY_PARRY, APPLY_BASH, APPLY_STUN, APPLY_PUNCH, APPLY_CLIMB,
   APPLY_GRIP, APPLY_SCRIBE, APPLY_BREW, APPLY_WEARSPELL, APPLY_REMOVESPELL,
   APPLY_EMOTION, APPLY_MENTALSTATE, APPLY_STRIPSN, APPLY_REMOVE, APPLY_DIG,
   APPLY_FULL, APPLY_THIRST, APPLY_DRUNK, APPLY_BLOOD, APPLY_COOK,
   APPLY_RECURRINGSPELL, APPLY_CONTAGIOUS, APPLY_EXT_AFFECT, APPLY_ODOR,
   APPLY_ROOMFLAG, APPLY_SECTORTYPE, APPLY_ROOMLIGHT, APPLY_TELEVNUM,
   APPLY_TELEDELAY, APPLY_AGI, APPLY_ARMOR, APPLY_SHIELD, APPLY_STONE,
   APPLY_SANCTIFY, APPLY_TOHIT, APPLY_MANATICK, APPLY_HPTICK, APPLY_WMOD,
   APPLY_MANAFUSE, APPLY_FASTING, APPLY_MANASHELL, APPLY_MANASHIELD, APPLY_MANAGUARD,
   APPLY_MANABURN, APPLY_WEAPONCLAMP, APPLY_ARROWCATCH, APPLY_BRACING,
   APPLY_HARDENING, APPLY_RFIRE, APPLY_RWATER, APPLY_RAIR, APPLY_REARTH,
   APPLY_RENERGY, APPLY_RMAGIC, APPLY_RNONMAGIC, APPLY_RBLUNT, APPLY_RPIERCE,
   APPLY_RSLASH, APPLY_RPOISON, APPLY_RPARALYSIS, APPLY_RHOLY, APPLY_RUNHOLY, APPLY_RUNDEAD,
   MAX_APPLY_TYPE
}
apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		   BV00
#define CONT_PICKPROOF		   BV01
#define CONT_CLOSED		   BV02
#define CONT_LOCKED		   BV03
#define CONT_EATKEY		   BV04

/*
 * Sitting/Standing/Sleeping/Sitting on/in/at Objects - Xerves
 * Used for furniture (value[2]) in the #OBJECTS Section
 */
#define SIT_ON     BV00
#define SIT_IN     BV01
#define SIT_AT     BV02

#define STAND_ON   BV03
#define STAND_IN   BV04
#define STAND_AT   BV05

#define SLEEP_ON   BV06
#define SLEEP_IN   BV07
#define SLEEP_AT   BV08

#define REST_ON     BV09
#define REST_IN     BV10
#define REST_AT     BV11

/*
 * Weapon weight type, used for weapons at V4
 * Defined in #OBJECTS.  -- Xerves 5-31-99
 */
#define WEIGHT_TINY      1
#define WEIGHT_LITTLE    2
#define WEIGHT_SMALL     3
#define WEIGHT_LIGHT     4
#define WEIGHT_MODERATE  5
#define WEIGHT_HEAVY     6
#define WEIGHT_LARGE     7
#define WEIGHT_HUGE      8
#define WEIGHT_MASSIVE   9
#define WEIGHT_GIGANTIC  10

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		    2
#define ROOM_VNUM_POLY		     3
#define ROOM_VNUM_CHAT		     9003
#define ROOM_VNUM_TEMPLE           21303
#define ROOM_VNUM_ALTAR            21303
#define ROOM_VNUM_SCHOOL	       21303
#define ROOM_AUTH_START		    21300
#define ROOM_VNUM_PORTAL           21034
#define ROOM_VNUM_STOWN            60000
#define ROOM_VNUM_ETOWN            90000
#define VNUM_ROOM_MORGUE           21101
/*
 * Room flags.           Holy cow!  Talked about stripped away..
 * Used in #ROOMS.       Those merc guys know how to strip code down.
 *			 Lets put it all back... ;)
 */

typedef enum
{
   ROOM_DARK, ROOM_DEATH, ROOM_NO_MOB, ROOM_INDOORS, ROOM_LAWFUL, ROOM_NEUTRAL,
   ROOM_CHAOTIC, ROOM_NO_MAGIC, ROOM_TUNNEL, ROOM_PRIVATE, ROOM_SAFE, ROOM_SOLITARY,
   ROOM_PET_SHOP, ROOM_NO_RECALL, ROOM_DONATION, ROOM_NODROPALL, ROOM_SILENCE,
   ROOM_LOGSPEECH, ROOM_NODROP, ROOM_CLANSTOREROOM, ROOM_NO_SUMMON, ROOM_NO_ASTRAL,
   ROOM_TELEPORT, ROOM_TELESHOWDESC, ROOM_NOFLOOR, ROOM_NOSUPPLICATE, ROOM_ARENA,
   ROOM_NOMISSILE, ROOM_NOEXIT, ROOM_IMP, ROOM_PROTOTYPE, ROOM_CASTEROOM, ROOM_MARK,
   ROOM_WILDERNESS, ROOM_KEEPDESC, ROOM_MAP, ROOM_MOUNT_SHOP, ROOM_PORTAL, ROOM_FREEKILL,
   ROOM_ANITEM, ROOM_NOLOOT, ROOM_TSAFE, ROOM_NOWDAM, ROOM_FORGEROOM, ROOM_MANANODE,
   ROOM_NOMILITARY, ROOM_MARKETPLACE, ROOM_PERMDEATH
}
eroom_flags;

/* 32 defines Maxed, do not add anymore till it is extended!!! --Xerves */
//#define ROOM_DARK        BV00
//#define ROOM_DEATH  BV01
//#define ROOM_NO_MOB  BV02
//#define ROOM_INDOORS  BV03
//#define ROOM_LAWFUL  BV04
//#define ROOM_NEUTRAL  BV05
//#define ROOM_CHAOTIC  BV06
//#define ROOM_NO_MAGIC  BV07
//#define ROOM_TUNNEL  BV08
//#define ROOM_PRIVATE  BV09
//#define ROOM_SAFE        BV10
//#define ROOM_SOLITARY  BV11
//#define ROOM_PET_SHOP  BV12
//#define ROOM_NO_RECALL  BV13
//#define ROOM_DONATION  BV14
//#define ROOM_NODROPALL  BV15
//#define ROOM_SILENCE  BV16
//#define ROOM_LOGSPEECH  BV17
//#define ROOM_NODROP  BV18
//#define ROOM_CLANSTOREROOM BV19
//#define ROOM_NO_SUMMON  BV20
//#define ROOM_NO_ASTRAL  BV21
//#define ROOM_TELEPORT  BV22
//#define ROOM_TELESHOWDESC BV23
//#define ROOM_NOFLOOR  BV24
//#define ROOM_NOSUPPLICATE     BV25
//#define ROOM_ARENA  BV26
//#define ROOM_NOMISSILE  BV27
//#define ROOM_NOEXIT           BV28
//#define ROOM_IMP              BV29
//#define ROOM_PROTOTYPE       BV30
//#define ROOM_CASTEROOM        BV31
/* The above group is Maxed, do not add anymore till it is EXTENDED --Xerves */

/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
   DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
   DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
}
dir_types;

#define PT_WATER	100
#define PT_AIR		200
#define PT_EARTH	300
#define PT_FIRE		400

/*
 * Push/pull types for exits					-Thoric
 * To differentiate between the current of a river, or a strong gust of wind
 */
typedef enum
{
   PULL_UNDEFINED, PULL_VORTEX, PULL_VACUUM, PULL_SLIP, PULL_ICE, PULL_MYSTERIOUS,
   PULL_CURRENT = PT_WATER, PULL_WAVE, PULL_WHIRLPOOL, PULL_GEYSER,
   PULL_WIND = PT_AIR, PULL_STORM, PULL_COLDWIND, PULL_BREEZE,
   PULL_LANDSLIDE = PT_EARTH, PULL_SINKHOLE, PULL_QUICKSAND, PULL_EARTHQUAKE,
   PULL_LAVA = PT_FIRE, PULL_HOTAIR
}
dir_pulltypes;


#define MAX_DIR			DIR_SOUTHWEST /* max for normal walking */
#define DIR_PORTAL		DIR_SOMEWHERE /* portal direction   */


/*
 * Exit flags.			EX_RES# are reserved for use by the
 * Used in #ROOMS.		SMAUG development team
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_EATKEY		  BV09
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES1			  BV14
#define EX_RES2			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_NOFLEE	  	  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define EX_OVERLAND               BV27
#define MAX_EXFLAG		  27

/*
 * Sector types.
 * Used in #ROOMS.
 */
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
   SECT_STONE, SECT_SSTONE, SECT_NSTONE, SECT_DWALL, SECT_NBWALL, 
   SECT_DOOR, SECT_CDOOR, SECT_LDOOR, SECT_HOLD, SECT_QEXIT, 
   SECT_SHIP, SECT_MAX
}
sector_types;

#define KRES_UNKNOWN -1
#define KRES_GOLD 1
#define KRES_IRON 2
#define KRES_CORN 3
#define KRES_GRAIN 4
#define KRES_LUMBER 5
#define KRES_STONE 6
#define KRES_FISH 7

/* Job defines, mostly for bug checks -- Xerves 12/99 */
#define MAX_RTYPE         KRES_FISH
/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
 
#define SPOWER_MIN      1001
#define SPOWER_LOW      1002
#define SPOWER_MED      1003
#define SPOWER_HI       1004
#define SPOWER_GREAT    1005
#define SPOWER_GREATER  1006
#define SPOWER_GREATEST 1007

typedef enum
{
   WEAR_NONE = -1, WEAR_LIGHT = 0, WEAR_FINGER_L, WEAR_FINGER_R, WEAR_NECK,
   WEAR_ABOUT_NECK, WEAR_BODY, WEAR_HEAD, WEAR_LEG_L, WEAR_LEG_R, WEAR_ARM_L,
   WEAR_ARM_R, WEAR_SHIELD, WEAR_WAIST, WEAR_WIELD, WEAR_DUAL_WIELD, WEAR_MISSILE_WIELD,
   WEAR_LODGE_RIB, WEAR_LODGE_ARM, WEAR_LODGE_LEG, WEAR_NOCKED, WEAR_BACK, MAX_WEAR
}
wear_locations;

/* Board Types */
typedef enum
{ BOARD_NOTE, BOARD_MAIL }
board_types;

/* Auth Flags */
#define FLAG_WRAUTH		      1
#define FLAG_AUTH		      2

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/

/*
 * Conditions.
 */
typedef enum
{
   COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
}
conditions;

/*
 * Positions.
 */
typedef enum
{
   POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_BERSERK,
   POS_RESTING, POS_AGGRESSIVE, POS_SITTING, POS_FIGHTING, POS_DEFENSIVE,
   POS_EVASIVE, POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG, POS_RIDING
}
positions;

/*
 * Styles.
 */
typedef enum
{
   STYLE_BERSERK, STYLE_AGGRESSIVE, STYLE_FIGHTING, STYLE_DEFENSIVE,
   STYLE_EVASIVE, STYLE_DIVINE, STYLE_WIZARDRY
}
styles;

/*
 * ACT bits for players.
 */
typedef enum
{
   PLR_IS_NPC, PLR_BOUGHT_PET, PLR_SHOVEDRAG, PLR_AUTOEXIT, PLR_AUTOLOOT,
   PLR_AUTOSAC, PLR_BLANK, PLR_OUTCAST, PLR_BRIEF, PLR_COMBINE, PLR_PROMPT,
   PLR_TELNET_GA, PLR_HOLYLIGHT, PLR_WIZINVIS, PLR_ROOMVNUM, PLR_SILENCE,
   PLR_NO_EMOTE, PLR_ATTACKER, PLR_NO_TELL, PLR_LOG, PLR_DENY, PLR_FREEZE,
   PLR_THIEF, PLR_KILLER, PLR_LITTERBUG, PLR_ANSI, PLR_RIP, PLR_NICE, PLR_FLEE,
   PLR_AUTOGOLD, PLR_AUTOMAP, PLR_AFK, PLR_INVISPROMPT, PLR_QUESTOR, PLR_REMORT,
   PLR_STATQUESTOR, PLR_OSET, PLR_MSET, PLR_REDIT, PLR_GAMBLER, PLR_NOTRANS,
   PLR_ONMAP, PLR_MAPEDIT, PLR_BOUGHT_MOUNT, PLR_POV, PLR_PORTALHUNT, PLR_ARENACHAR, PLR_AWAY,
   PLR_PKRESET, PLR_WARNED, PLR_NOWEATHER, PLR_TARGET, PLR_CARRYBIN, PLR_MXP, PLR_RPSETUP,
   PLR_SHOWASIMM, PLR_SHOWNAMES, PLR_UKNOWN, PLR_SHOWPC, PLR_HASLASTNAME, PLR_ONDUTY,
   PLR_NOSIMILIAR, PLR_KRESET, PLR_MMOBILES, PLR_PARRY, PLR_DODGE, PLR_TUMBLE,
   PLR_MAPWINDOW, PLR_AUTOSPLIT, PLR_WILDERTILES, PLR_NORIDERS, PLR_NOTOHIT,
   PLR_QUESTLOOT, PLR_COUNTER
}
player_flags;

/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
#define PCFLAG_DEADLY              BV01
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_GROUPWHO		   BV11
#define PCFLAG_DIAGNOSE		   BV12
#define PCFLAG_HIGHGAG		   BV13
#define PCFLAG_WATCH		   BV14 /* see function "do_watch" */
#define PCFLAG_HELPSTART	   BV15 /* Force new players to help start */
#define PCFLAG_NOBEEP              BV16 /* Added by Samson 2-15-98 */
#define PCFLAG_NOFINGER            BV17 /* Finger at will? -- Xerves */
#define PCFLAG_NOFOLLOW	           BV18 /* Follow at will -- Xerves */
#define PCFLAG_AUTOPROTO       BV19 //Will treat all objs/mobs as if they are prototype
#define PCFLAG_CNOASSIST       BV20 //Charmee will not auto assist


typedef enum
{
   TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
   TIMER_APPLIED, TIMER_PKILLED, TIMER_ASUPRESSED, TIMER_NUISANCE,
   TIMER_FORGE, TIMER_COOLING, TIMER_AUCTION
}
timer_types;

struct timer_data
{
   TIMER *prev;
   TIMER *next;
   DO_FUN *do_fun;
   int value;
   sh_int type;
   int count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_CHAT		   BV01
#define	CHANNEL_TALKQUEST		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_MUSIC		   BV04
#define	CHANNEL_ASK		   BV05
#define	CHANNEL_SHOUT		   BV06
#define	CHANNEL_YELL		   BV07
#define CHANNEL_MONITOR		   BV08
#define CHANNEL_LOG		   BV09
#define CHANNEL_HIGHGOD		   BV10
#define CHANNEL_CLAN		   BV11
#define CHANNEL_BUILD		   BV12
#define CHANNEL_HIGH		   BV13
#define CHANNEL_AVTALK		   BV14
#define CHANNEL_PRAY		   BV15
#define CHANNEL_COUNCIL 	   BV16
#define CHANNEL_GUILD              BV17
#define CHANNEL_COMM		   BV18
#define CHANNEL_TELLS		   BV19
#define CHANNEL_ORDER              BV20
#define CHANNEL_NEWBIE             BV21
#define CHANNEL_WARTALK            BV22
#define CHANNEL_RACETALK           BV23
#define CHANNEL_WARN               BV24
#define CHANNEL_WHISPER		   BV25
#define CHANNEL_AUTH		   BV26
#define CHANNEL_INFO		   BV27
#define CHANNEL_GSOCIAL          BV28
#define CHANNEL_IMMREMINDER        BV29
#define CHANNEL_KINGDOM		   BV30

struct channel_history
{
   CHANNEL_HISTORY *next;
   CHANNEL_HISTORY *prev;
   int channel;
   char *sender;
   char *text;
   int pid;
   int flags;
   int level;
   int kpid;
};

#define CHISTORY_CLOAKED       BV00
#define CHISTORY_INVIS         BV01
#define CHISTORY_WIZINVIS      BV02
#define CHISTORY_FAMOUS        BV03

/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOKILL               BV00 // S in FANS
#define AFLAG_FREEKILL		      BV01 // F in FANS
#define AFLAG_NOTELEPORT	        BV02
#define AFLAG_NOQUEST               BV03 /* No mob/obj Quest in area -- Xerves 9/14/99 */
#define AFLAG_CHANGED               BV04
#define AFLAG_RESOURCE              BV05 /* Save some processing, check to see if the area
                                            even has natural resources -- Xerves 10/27/99 */
#define AFLAG_CARPENTER             BV06 /* Editable by carpenter job -- Xerves 12/99 */
#define AFLAG_NOAREA	 	       BV07 // No Area Attacks like hitall in the area
#define AFLAG_ANITEM                BV08 // A in FANS
#define AFLAG_NOLOOT                BV09 // N in FANS
#define AFLAG_NOWDAM                BV10 // No damage from weather received


struct npcrace_data //Linked list for NPC Race Data, used with quest system too
{
   NPCRACE_DATA *next;
   NPCRACE_DATA *prev;
   int racenum;
   char *racename;
   int willload[MAX_QUEST_DIFF];
   char *description[MAX_QUEST_DIFF];
   int sex[MAX_QUEST_DIFF];
   int fulldescription[MAX_QUEST_DIFF];
   EXT_BV flags[MAX_QUEST_DIFF];
   EXT_BV nflags[MAX_QUEST_DIFF];
};

//119 Flags at DETECTHIDDEN, if more than 128 is needed add another flag variable
typedef enum
{
   QMOB_HP, QMOB_AGI, QMOB_STR, QMOB_STR2, QMOB_INT, QMOB_INT2, QMOB_LINT, QMOB_LINT2,
   QMOB_LHP, QMOB_LAGI, QMOB_LSTR, QMOB_LSTR2, QMOB_DEX, QMOB_LDEX, QMOB_CON, QMOB_LCON,
   QMOB_WIS, QMOB_WIS2, QMOB_LWIS, QMOB_LWIS2,
   QMOB_LCK, QMOB_LLCK, QMOB_ARMOR, QMOB_ARMOR2, QMOB_LARMOR, QMOB_LARMOR2, QMOB_BASH,
   QMOB_LBASH, QMOB_SLASH, QMOB_LSLASH, QMOB_STAB, QMOB_LSTAB, QMOB_DAM, QMOB_DAM2,
   QMOB_LDAM, QMOB_LDAM2, QMOB_NOGOLD, QMOB_GOLD1, QMOB_GOLD2, QMOB_LGOLD1, QMOB_LGOLD2,
   QMOB_NOAGGRO, QMOB_RUNNING, QMOB_UNDEAD, QMOB_LIVINGDEAD, QMOB_SENTINEL, QMOB_DODGE, QMOB_PARRY,
   QMOB_LIGHT, QMOB_SERIOUS, QMOB_CRITICAL, QMOB_HEAL, QMOB_SANCTUARY, QMOB_FSHIELD,
   QMOB_SSHIELD, QMOB_ISHIELD, QMOB_STONESKIN, QMOB_DISARM, QMOB_TRIP, QMOB_ABASH,
   QMOB_STUN, QMOB_GOUGE, QMOB_BACKSTAB, QMOB_BLINDNESS, QMOB_LBREATH, QMOB_GBREATH,  
   QMOB_FIREBREATH, QMOB_FROSTBREATH, QMOB_ACIDBREATH, QMOB_CURSE, QMOB_HARM, QMOB_FIREBALL,
   QMOB_WEAKEN, QMOB_POISON, QMOB_SFIRE, QMOB_SCOLD, QMOB_SELECT, QMOB_SENERGY, QMOB_SBLUNT,
   QMOB_SPIERCE, QMOB_SSLASH, QMOB_SSLEEP, QMOB_SCHARM, QMOB_SNONMAGIC, QMOB_SMAGIC,
   QMOB_SPARALYSIS, QMOB_SAIR, QMOB_RFIRE, QMOB_RCOLD, QMOB_RELECT, QMOB_RENERGY, QMOB_RBLUNT,
   QMOB_RPIERCE, QMOB_RSLASH, QMOB_RSLEEP, QMOB_RCHARM, QMOB_RNONMAGIC, QMOB_RMAGIC,
   QMOB_RPARALYSIS, QMOB_RAIR, QMOB_IFIRE, QMOB_ICOLD, QMOB_IELECT, QMOB_IENERGY, QMOB_IBLUNT,
   QMOB_IPIERCE, QMOB_ISLASH, QMOB_ISLEEP, QMOB_ICHARM, QMOB_INONMAGIC, QMOB_IMAGIC,
   QMOB_IPARALYSIS, QMOB_IAIR, QMOB_INVISIBLE, QMOB_DETECTINVIS, QMOB_HIDE,
   QMOB_TRUESIGHT, QMOB_SNEAK, QMOB_DETECTHIDDEN
}
qmob_types;

struct quest_mob_data //Structure for quest mobs
{
   QMOB_DATA *next;
   QMOB_DATA *prev;
   char *name;
   int lowdiff;
   int hidiff;
   int race;
   int sex;
   int boss;
   EXT_BV flags;
};

//FULL 128 FLAGS, DO NOT ADD MORE!!!
typedef enum
{
   QOBJ_FINGER, QOBJ_NECK, QOBJ_BODY, QOBJ_HEAD, QOBJ_LEGS, QOBJ_ARMS, QOBJ_WSHIELD,
   QOBJ_ANECK, QOBJ_WAIST, QOBJ_WIELD, QOBJ_MAGIC, QOBJ_NODROP, QOBJ_BLESS, QOBJ_INVENTORY,
   QOBJ_ORGANIC, QOBJ_METAL, QOBJ_POISONED, QOBJ_DEATHROT, QOBJ_NOLOCATE, QOBJ_GROUNDROT,
   QOBJ_NOGIVE, QOBJ_NODISARM, QOBJ_NOBREAK, QOBJ_ARTIFACT, QOBJ_TWOHANDED, QOBJ_IMBUABLE,
   QOBJ_GEM, QOBJ_GEM_SETTING, QOBJ_SANCTIFIED, QOBJ_CLOAK, QOBJ_UNIQUE, QOBJ_STR, QOBJ_STR2,
   QOBJ_STR3, QOBJ_LSTR, QOBJ_LSTR2, QOBJ_DEX, QOBJ_DEX2, QOBJ_DEX3, QOBJ_LDEX, QOBJ_LDEX2,
   QOBJ_INT, QOBJ_INT2, QOBJ_INT3, QOBJ_LINT, QOBJ_LINT2, QOBJ_WIS, QOBJ_WIS2, QOBJ_WIS3,
   QOBJ_LWIS, QOBJ_LWIS2, QOBJ_CON, QOBJ_CON2, QOBJ_CON3, QOBJ_LCON, QOBJ_LCON2,
   QOBJ_HP, QOBJ_HP2, QOBJ_HP3, QOBJ_HP4, QOBJ_HP5, QOBJ_HP6, QOBJ_HP7, QOBJ_LHP, QOBJ_LHP2,
   QOBJ_LHP3, QOBJ_MP, QOBJ_MP2, QOBJ_MP3, QOBJ_MP4, QOBJ_MP5, QOBJ_MP6, QOBJ_MP7, QOBJ_LMP,
   QOBJ_LMP2, QOBJ_LMP3, QOBJ_DAM, QOBJ_DAM2, QOBJ_DAM3, QOBJ_DAM4, QOBJ_DAM5, QOBJ_LDAM,
   QOBJ_LDAM2, QOBJ_TOHIT, QOBJ_TOHIT2, QOBJ_TOHIT3, QOBJ_LTOHIT, QOBJ_LTOHIT2, QOBJ_SSKIN,
   QOBJ_SSKIN2, QOBJ_SSKIN3, QOBJ_SSKIN4, QOBJ_SSKIN5, QOBJ_LSSKIN, QOBJ_LSSKIN2, QOBJ_SAVE,
   QOBJ_SAVE2, QOBJ_SAVE3, QOBJ_SAVE4, QOBJ_SAVE5, QOBJ_LSAVE, QOBJ_LSAVE2, QOBJ_LSAVE3,
   QOBJ_RBLUNT, QOBJ_RSLASH, QOBJ_RPIERCE, QOBJ_RFIRE, QOBJ_RCOLD, QOBJ_RELECT, QOBJ_RENERGY,
   QOBJ_RAIR, QOBJ_RNONMAGIC, QOBJ_RMAGIC, QOBJ_SBLUNT, QOBJ_SSLASH, QOBJ_SPIERCE, QOBJ_SFIRE,
   QOBJ_SCOLD, QOBJ_SELECT, QOBJ_SENERGY, QOBJ_SAIR, QOBJ_SNONMAGIC, QOBJ_SMAGIC,
   QOBJ_IBLUNT, QOBJ_ISLASH, QOBJ_IPIERCE, QOBJ_IFIRE, QOBJ_ICOLD
}
qobj_types;   
   
//36 Flags at LMPGEN2, ADD HERE!
//71 at LWMOD3
//73 at NOTHIRST
typedef enum
{
   QOBJ_IELECT, QOBJ_IENERGY, QOBJ_IAIR, QOBJ_INONMAGIC, QOBJ_IMAGIC, QOBJ_INVISIBLE, QOBJ_DETECT_INVIS, QOBJ_DETECT_MAGIC,
   QOBJ_DETECT_HIDDEN, QOBJ_SANCTUARY, QOBJ_FLYING, QOBJ_PASS_DOOR, QOBJ_FLOATING, QOBJ_TRUESIGHT,
   QOBJ_DETECT_TRAPS, QOBJ_SCRYING, QOBJ_FIRESHIELD, QOBJ_SHOCKSHIELD, QOBJ_ICESHIELD, QOBJ_AQUA_BREATH,
   QOBJ_WIZARDEYE, QOBJ_EWIZARDEYE, QOBJ_MWIZARDEYE, QOBJ_HPGEN, QOBJ_HPGEN2, QOBJ_HPGEN3, QOBJ_HPGEN4,
   QOBJ_HPGEN5, QOBJ_LHPGEN, QOBJ_LHPGEN2, QOBJ_MPGEN, QOBJ_MPGEN2, QOBJ_MPGEN3, QOBJ_MPGEN4, QOBJ_MPGEN5,
   QOBJ_LMPGEN, QOBJ_LMPGEN2, QOBJ_SHIELD, QOBJ_SHIELD2, QOBJ_SHIELD3, QOBJ_SHIELD4, QOBJ_SHIELD5,
   QOBJ_LSHIELD, QOBJ_LSHIELD2, QOBJ_AGI, QOBJ_AGI2, QOBJ_AGI3, QOBJ_AGI4, QOBJ_AGI5, QOBJ_LAGI, QOBJ_LAGI2,
   QOBJ_LAGI3, QOBJ_LCK, QOBJ_LCK2, QOBJ_LCK3, QOBJ_LLCK, QOBJ_LLCK2, QOBJ_WMOD, QOBJ_WMOD2, QOBJ_WMOD3,
   QOBJ_WMOD4, QOBJ_WMOD5, QOBJ_LWMOD, QOBJ_LWMOD2, QOBJ_LWMOD3, QOBJ_NOHUNGER, QOBJ_NOTHIRST, QOBJ_STAT,
   QOBJ_STAT2, QOBJ_STAT3, QOBJ_STAT4, QOBJ_STAT5, QOBJ_LSTAT, QOBJ_LSTAT2, QOBJ_BATTLE, QOBJ_BATTLE2, QOBJ_BATTLE3,
   QOBJ_BATTLE4, QOBJ_LBATTLE, QOBJ_LBATTLE2, QOBJ_BACK
}
qobj_types2;

struct quest_obj_data //Structure for quest objs
{
   QOBJ_DATA *next;
   QOBJ_DATA *prev;
   int value[14];
   int type;
   int gold;
   float weight;
   char *name;
   int lowdiff;
   int hidiff;
   int race; //If set the object will modify itself based on race
   int boss;
   int qps;
   EXT_BV flags;
   EXT_BV flags2;
};

struct quest_data //Structure for the actual quests
{
   QUEST_DATA *next;
   QUEST_DATA *prev;
   int player[6];
   int qp[6];
   int timeleft;
   int traveltime;
   int tillnew;
   int mission;
   int tokill;
   int killed;
   int x;
   int y;
   int map;
   int difficulty;
   AREA_DATA *questarea;
};   

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct mob_index_data
{
   MOB_INDEX_DATA *next;
   MOB_INDEX_DATA *next_sort;
   SPEC_FUN *spec_fun;
   SHOP_DATA *pShop;
   REPAIR_DATA *rShop;
   MPROG_DATA *mudprogs;
   EXT_BV progtypes;
   char *player_name;
   char *short_descr;
   char *long_descr;
   char *description;
   sh_int vnum;
   sh_int count;
   sh_int killed;
   sh_int sex;
   sh_int level;
   EXT_BV act;
   EXT_BV affected_by;
   sh_int alignment;
   sh_int mobthac0; /* Unused */
   sh_int ac;
   sh_int hitnodice;
   sh_int hitsizedice;
   sh_int hitplus;
   sh_int damnodice;
   sh_int damsizedice;
   sh_int damplus;
   sh_int damaddlow;
   sh_int damaddhi;
   sh_int max_move; /* Will save movement on mobs now -- Xerves */
   int gold;
   int xflags;
   int resistant;
   int immune;
   int susceptible;
   int elementb;
   int mover; //move rating
   EXT_BV attacks;
   EXT_BV defenses;
   int speaks;
   int speaking;
   sh_int position;
   sh_int defposition;
   int height;
   int weight;
   sh_int race;
   sh_int class;
   sh_int hitroll;
   sh_int damroll;
   /* Going to change these to m1 to m6 -- Xerves */
   int m1; /* goldmin */
   int m2; /* goldmax */
   int m3; /* goldlim */
   sh_int m4; /* levelmin */
   sh_int m5; /* levelmax */
   sh_int m6; /* 3rd sh_int */
   int m7; /* For whatever */
   int m8; /* For whatever */
   int m9; // For whatever
   int m10;
   int m11;
   int m12;
   //For Perm applies
   int apply_res_fire;
   int apply_res_water;
   int apply_res_air;
   int apply_res_earth;
   int apply_res_energy;
   int apply_res_magic;
   int apply_res_nonmagic;
   int apply_res_blunt;
   int apply_res_pierce;
   int apply_res_slash;
   int apply_res_poison;
   int apply_res_paralysis;
   int apply_res_holy;
   int apply_res_unholy;
   int apply_res_undead;
   EXT_BV miflags;
   sh_int cident; /* Used to identify the Caste Mob --Xerves */
   sh_int perm_str;
   sh_int perm_int;
   sh_int perm_wis;
   sh_int perm_dex;
   sh_int perm_con;
   sh_int perm_cha;
   sh_int perm_lck;
   sh_int perm_agi;
   sh_int tohitbash;
   sh_int tohitslash;
   sh_int tohitstab;
   sh_int saving_poison_death;
   sh_int saving_wand;
   sh_int saving_para_petri;
   sh_int saving_breath;
   sh_int saving_spell_staff;
};


struct hunt_hate_fear
{
   char *name;
   CHAR_DATA *who;
};

struct fighting_data
{
   CHAR_DATA *who;
   int xp;
   int twinkpoints;
   sh_int align;
   sh_int duration;
   sh_int timeskilled;
};

/*
struct	editor_data
{
    sh_int		numlines;
    sh_int		on_line;
    sh_int		size;
    char		line[49][81];
};  */

struct char_map_data
{
   CMAP_DATA *next;
   CMAP_DATA *prev;
   CHAR_DATA *mapch;
};

struct obj_map_data
{
   OMAP_DATA *next;
   OMAP_DATA *prev;
   OBJ_DATA *mapobj;
};

struct kingdom_chest_data
{
   KCHEST_DATA *next;
   KCHEST_DATA *prev;
   OBJ_DATA *obj;
};

struct tornado_data
{
   TORNADO_DATA *next;
   TORNADO_DATA *prev;
   sh_int x;
   sh_int y;
   sh_int map;
   sh_int power;
   sh_int turns;
   sh_int dir;
};

struct front_data
{
   FRONT_DATA *next;
   FRONT_DATA *prev;
   sh_int type;
   sh_int typec;
   sh_int x;
   sh_int y;
   sh_int map;
   sh_int speed;
   sh_int size;
   sh_int f[35];
};


struct wilderness_bin_data
{
   BIN_DATA *next;
   BIN_DATA *prev;
   int bin1;
   int bin2;
   int x;
   int y;
   int map;
   int room;
   int vnum;
   int serial;
};

struct battle_arena_data
{
   BARENA_DATA *next;
   BARENA_DATA *prev;
   char *name;
   int wins;
   int losses;
   int ties;
   int games;
   int numavg;
   int kills;
   int pranking;
   int pkills;
   int pdeaths;
};

struct gem_data
{
   GEM_DATA *next;
   GEM_DATA *prev;
   int cost;
   int vnum;
   int rarity;
};

struct box_data
{
   BOX_DATA *next;
   BOX_DATA *prev;
   int vnum;
};

struct stable_data
{
   STABLE_DATA *next;
   STABLE_DATA *prev;
   sh_int vnum;
   char *name;
   char *short_descr;
   char *long_descr;
   char *description;
   sh_int level;
   sh_int exp;
   sh_int hit;
   sh_int max_hit;
   sh_int move;
   sh_int max_move;
};

struct extracted_char_data
{
   EXTRACT_CHAR_DATA *next;
   CHAR_DATA *ch;
   ROOM_INDEX_DATA *room;
   ch_ret retcode;
   bool extract;
};

struct clan_member_list
{
   CMEMBER_DATA *next;
   CMEMBER_DATA *prev;
   char *name;
   char *rank;
   char *clan;
   int level;
   int class;
   int race;
   int sex;
};

struct kingdom_member_list
{
   KMEMBER_DATA *next;
   KMEMBER_DATA *prev;
   char *name;
   int caste;
   int kingdom;
   int kpid;
   int level;
   int class;
   int race;
   int sex;
};

struct trainer_data
{
   TRAINER_DATA *next;
   TRAINER_DATA *prev;
   int vnum;
   sh_int sn[20]; //Up to 20 skills/groups
   sh_int mastery[20];
};

struct military_eq
{
   sh_int obj1;
   sh_int obj2;
   sh_int obj3;
   sh_int obj4;
   sh_int obj5;
   sh_int obj6;
};

//Saves some critical info for putting back into the character.
struct save_arena_data
{
   int pflags;
   EXT_BV act;
   sh_int perm_str;
   sh_int perm_int;
   sh_int perm_wis;
   sh_int perm_dex;
   sh_int perm_con;
   sh_int perm_cha;
   sh_int perm_lck;
   sh_int perm_agi;
   sh_int mod_str;
   sh_int mod_int;
   sh_int mod_wis;
   sh_int mod_dex;
   sh_int mod_con;
   sh_int mod_cha;
   sh_int mod_lck;
   sh_int mod_agi;
   sh_int hit;
   sh_int mana;
   sh_int move;
   sh_int max_hit;
   sh_int max_move;
   sh_int max_mana;
   int resistant;
   int susceptible;
   int immune;
   sh_int pcondition[MAX_CONDS];
   sh_int pwizinvis;
   sh_int mental_state;
   sh_int saving_poison_death;
   sh_int saving_wand;
   sh_int saving_para_petri;
   sh_int saving_breath;
   sh_int saving_spell_staff;
   sh_int style;
   int wimpy;
   EXT_BV affected_by;
   CHAR_DATA *ppet;
   CHAR_DATA *pmount;
   char *pbamfin;
   char *pbamfout;
   char *prank;
   char *pbestowments;
   sh_int level;
   sh_int trust;
   sh_int hitroll;
   sh_int damroll;
   sh_int armor;
   sh_int plearned[MAX_SKILL];
   sh_int pranking[MAX_SKILL];
   sh_int pspellgroups[MAX_GROUP + 1];
   sh_int pspellpoints[MAX_GROUP + 1];
};
struct forge_data
{
   FORGE_DATA *next;
   FORGE_DATA *prev;
   sh_int vnum;
   int slabnum;
   char *name;
   int type;
};
struct slab_data
{
   SLAB_DATA *next;
   SLAB_DATA *prev;
   sh_int vnum;
   sh_int kmob; //Used by the armmilitary routine for default eq.
   char *name;
   char *adj;
   int qps;
};

struct depo_ore_data
{
   DEPO_ORE_DATA *next;
   DEPO_ORE_DATA *prev;
   int vnum; //just for a bit of checking
   int count; //amount of raw slabs
   DEPO_WEAPON_DATA *first_weapon;
   DEPO_WEAPON_DATA *last_weapon;
};

struct depo_weapon_data
{
   DEPO_WEAPON_DATA *next;
   DEPO_WEAPON_DATA *prev;
   int vnum; //match the vnum
   int count; //number in kingdom
};   

struct kingdom_military_list
{   
   char *name;
   char *short_descr;
   char *long_descr;
   sh_int race;
   sh_int str;
   sh_int intelligence;
   sh_int wis;
   sh_int lck;
   sh_int dex;
   sh_int agi;
   sh_int con;
   sh_int vnum;
   sh_int room;
   sh_int kingdom;
   sh_int cost;
   sh_int x;
   sh_int y;
   sh_int map;
   int speed;
   int time;
   EXT_BV miflags;
   sh_int irange;
   sh_int warange;
   sh_int prange;
   sh_int px;
   sh_int py;
};

//For military mobiles, contains some important info.  Done to save memory
struct midata
{
   char *command; //Command buffer, works like the run for players
   sh_int x;
   sh_int y;
   sh_int map;
   ROOM_INDEX_DATA *in_room;
   sh_int mspeed;
   sh_int mtick;
};

struct aggression_data
{
   AGGRO_DATA *next;
   AGGRO_DATA *prev;
   AGGRO_DATA *next_global;
   AGGRO_DATA *prev_global;
   CHAR_DATA *ch;
   CHAR_DATA *owner;
   int value;
};

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
struct char_data
{
   CHAR_DATA *next;
   CHAR_DATA *prev;
   CHAR_DATA *next_in_room;
   CHAR_DATA *prev_in_room;
   CHAR_DATA *next_ship;
   CHAR_DATA *prev_ship;
   CHAR_DATA *master;
   CHAR_DATA *leader;
   FIGHT_DATA *fighting;
   CHAR_DATA *reply;
   CHAR_DATA *retell;
   CHAR_DATA *switched;
   CHAR_DATA *mount;
   SHIP_DATA *ship;
   HHF_DATA *hunting;
   HHF_DATA *fearing;
   HHF_DATA *hating;
   AGGRO_DATA *first_aggro;
   AGGRO_DATA *last_aggro;
   SPEC_FUN *spec_fun;
   TOWN_DATA *dumptown; //Used by extraction workers to make sure they extract to a town...
   MPROG_ACT_LIST *mpact;
   int mpactnum;
   sh_int mpscriptpos;
   MOB_INDEX_DATA *pIndexData;
   DESCRIPTOR_DATA *desc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   NOTE_DATA *pnote;
   NOTE_DATA *comments;
   OBJ_DATA *first_carrying;
   OBJ_DATA *last_carrying;
   OBJ_DATA *on;
   ROOM_INDEX_DATA *in_room;
   ROOM_INDEX_DATA *was_in_room;
   PC_DATA *pcdata;
   MI_DATA *midata; //for moving or anything else military needed.  Save memory
   DO_FUN *last_cmd;
   DO_FUN *prev_cmd; /* mapping */
   void *dest_buf; /* This one is to assign to differen things */
   char *alloc_ptr; /* Must str_dup and free this one */
   void *spare_ptr;
   int tempnum;
   int fight_state; // Used for different fight lags...
   sh_int fight_timer; // Timer in rounds
   EDITOR_DATA *editor;
   TIMER *first_timer;
   TIMER *last_timer;
   CHAR_MORPH *morph;
   char *name;
   char *short_descr;
   char *long_descr;
   char *description;
   sh_int num_fighting;
   sh_int substate;
   sh_int sex;
   sh_int class;
   sh_int race;
   sh_int level;
   sh_int trust;
   int managen;
   int hpgen;
   int begatt; //Begging attemps, to prevent someone getting rich by doing this for 12 hours...
   int apply_armor;
   int apply_stone;
   int apply_tohit;
   int apply_sanctify;
   int apply_shield;
   int apply_wmod;
   int apply_fasting;
   int apply_manafuse;
   int apply_manashell;
   int apply_manashield;
   int apply_managuard;
   int apply_manaburn;
   int apply_weaponclamp;
   int apply_arrowcatch;
   int apply_bracing;
   int apply_hardening;
   int apply_res_fire[2];
   int apply_res_water[2];
   int apply_res_air[2];
   int apply_res_earth[2];
   int apply_res_energy[2];
   int apply_res_magic[2];
   int apply_res_nonmagic[2];
   int apply_res_blunt[2];
   int apply_res_pierce[2];
   int apply_res_slash[2];
   int apply_res_poison[2];
   int apply_res_paralysis[2];
   int apply_res_holy[2];
   int apply_res_unholy[2];
   int apply_res_undead[2];
   int elementb;
   sh_int speed; /* descriptor speed settings */
   int played;
   sh_int timer;
   sh_int wait;
   sh_int hit;
   sh_int max_hit;
   sh_int mana;
   sh_int max_mana;
   sh_int move;
   sh_int max_move;
   //Below 4 are health on limbs, increases by time/curing
   sh_int con_rarm;
   sh_int con_larm;
   sh_int con_lleg;
   sh_int con_rleg;
   sh_int practice;
   int gold;
   int grip;
   EXT_BV act;
   EXT_BV affected_by;
   EXT_BV no_affected_by;
   int carry_weight;
   int carry_number;
   int xflags;
   int resistant;
   int mover; //Move rating
   int no_resistant;
   int immune;
   int no_immune;
   int susceptible;
   int no_susceptible;
   EXT_BV attacks;
   EXT_BV defenses;
   int speaks;
   int speaking;
   sh_int saving_poison_death;
   sh_int saving_wand;
   sh_int saving_para_petri;
   sh_int saving_breath;
   sh_int saving_spell_staff;
   sh_int alignment;
   sh_int barenumdie;
   sh_int baresizedie;
   sh_int mobthac0;
   sh_int hitroll;
   sh_int damroll;
   sh_int hitplus;
   sh_int damplus;
   sh_int damaddlow;
   sh_int damaddhi;
   sh_int position;
   sh_int defposition;
   sh_int style;
   sh_int height;
   sh_int weight;
   sh_int armor;
   sh_int wimpy;
   int deaf;
   /* Going to change these to m1 to m6 -- Xerves */
   int m1; /* goldmin */
   int m2; /* goldmax */
   int m3; /* goldlim */
   sh_int m4; /* levelmin */
   sh_int m5; /* levelmax */
   sh_int m6; /* 3rd sh_int */
   int m7; /* For whatever */
   int m8; /* For whatever */
   int m9; // For whatever
   int m10; // Speed of the unit
   int m11; // Extra
   int m12; // Extra
   EXT_BV miflags;
   sh_int cident; /* Caste Identify -- Xerves */
   sh_int perm_str;
   sh_int perm_int;
   sh_int perm_wis;
   sh_int perm_dex;
   sh_int perm_con;
   sh_int perm_cha;
   sh_int perm_lck;
   sh_int perm_agi;
   sh_int mod_str;
   sh_int mod_int;
   sh_int mod_wis;
   sh_int mod_dex;
   sh_int mod_con;
   sh_int mod_cha;
   sh_int mod_lck;
   sh_int mod_agi;
   sh_int agi_meter;
   sh_int tohitbash;
   sh_int tohitslash;
   sh_int tohitstab;
   sh_int mental_state; /* simplified */
   sh_int emotional_state; /* simplified */
   int retran;
   int retran_x;
   int retran_y;
   int retran_map;
   int regoto;
   int regoto_x;
   int regoto_y;
   int regoto_map;
   int tcount; //For mprogs, can be used for any kind of things, mainly counting or TRUE/FALSE check on ifchecks
   sh_int mobinvis; /* Mobinvis level SB */
   sh_int resx;
   sh_int resy;
   sh_int resmap;
   sh_int stx;
   sh_int sty;
   sh_int stmap;
   COORD_DATA *coord; /* Coordinates on the overland map - Samson 7-31-99 */
   sh_int map; /* Which map are they on? - Samson 8-3-99 */
   sh_int fightm; /* Fighting map - Xerves 5-13-00 */
   sh_int fcounter;
   sh_int cmd_recurse;
   sh_int hit_regen_counter;
   sh_int mana_regen_counter;
   sh_int move_regen_counter;
   sh_int hburn_regen_counter;
   sh_int mburn_regen_counter;
   int serial;
   char *last_name;
   int fame;
   char *  tone;
   char *  movement;
   CHAR_DATA *rider;   //Person who is being rode
   CHAR_DATA *riding;  //Person you are riding
};


struct killed_data
{
   sh_int vnum;
   char count;
};

struct pkilled_data
{
   PKILLED_DATA *next;
   PKILLED_DATA *prev;
   char *name;
};

/* Structure for link list of ignored players */
struct ignore_data
{
   IGNORE_DATA *next;
   IGNORE_DATA *prev;
   char *name;
};

/* Max number of people you can ignore at once */
#define MAX_IGN		6

struct introduction_data
{
   INTRO_DATA *next;
   INTRO_DATA *prev;
   int pid;
   int lastseen;
   int flags;
   int value;
};

/*
 * Data which only PC's have.
 */
struct pc_data
{
   CHAR_DATA *pet;
   CHAR_DATA *mount;
   CHAR_DATA *aimtarget; //Target for the aim command
   CLAN_DATA *clan;
   COUNCIL_DATA *council;
   AREA_DATA *area;
   TOWN_DATA *town;
   DEITY_DATA *deity;
   STABLE_DATA *first_stable;
   STABLE_DATA *last_stable;
   PKILLED_DATA *first_pkilled;
   PKILLED_DATA *last_pkilled;
   INTRO_DATA *first_introduction;
   INTRO_DATA *last_introduction;
   CHANNEL_HISTORY *first_messagehistory;
   CHANNEL_HISTORY *last_messagehistory;
   OBJ_DATA *first_bankobj;
   OBJ_DATA *last_bankobj;
   int banksize;
   int gprompt;
   int xsize;
   int ysize;
   int consent;
   char *sendmail; //buffer used to send an email
   char *homepage;
   char *offeredlname;
   char *clan_name;
   char *council_name;
   char *deity_name;
   char *pwd;
   char *bamfin;
   char *bamfout;
   char *filename; /* For the safe mset name -Shaddai */
   char *rank;
   char *title;
   char *bestowments; /* Special bestowed commands    */
   char *autocommand; //For auto-target skills/spells/etc.. Will parse the command into this pointer.
   int quest_curr; //Quest points left
   int quest_accum; //Quest points earned lifetime
   int reward_curr; //Reward points, for reporting bugs/typos/bringing newbies
   int reward_accum; //Total reward points accumulated in lifetime of player
   QUEST_DATA *quest; //Pointer to the quest data
   int quest_wins; //Quest wins
   int quest_losses; //Quest losses
   int twink_points; //Twink points, 100 and delete!
   int timeout_login; //Login timeout, default 3 minutes
   int timeout_notes; //Notes/Menu timeout, default 10 minutes
   int timeout_idle; //Playing the game, default 20 minutes
   int balance; /*Balance of money in the bank   */
   int lastinterest; //Checks when the last time interest was done on the account
   int lastintrocheck; //Last time introductions on a player where checked   
   int lastprankingcheck; //Last Power Ranking check   
   int caste; /* Caste, for caste - Xerves */
   int lostcon; //Con lost after death, for resurrections
   int rezpercent; //Resurrection percentage
   int duel_offer_time;
   int duel_receive_time;
   int duel_offer_name;
   int dual_receive_name;
   int duel_offer_pranking; 
   int duel_receive_pranking;
   int spar_offer_name;
   int spar_receive_name;
   ROOM_INDEX_DATA *rezroom; //Vnum of the room the rez will happen in
   OBJ_DATA *rezcorpse; //Pointer to which corpse the rez will work on
   int rezx;
   int rezy;
   int rezmap;
   EXT_BV talent; /* Flags for talents */
   sh_int hometown; /* Hometown - Xerves */
   sh_int kingdompid; //Unique pid for their kingdom, mainly used in error checking
   char *pretit; /* Pretitle -- Xerves 8/1/99 */
   sh_int train; /* Training point */
   sh_int incarnations;
   sh_int gt_remort;
   sh_int tier;
   int pid; //Unique Indentifier for each player
   sh_int pkpower; //Power level in the pkill range
   sh_int colors[MAX_COLORS];
   sh_int job; /* What job are they?? -- Xerves 12/99 */
   sh_int authwait;
   sh_int mapdir; /* What direction are they facing on the map, used for the view -- Xerves */
   int flevel; /* For commands so level 55s can do some level 60 stuff */
   int lore;
   int spoints; //Skill points gained from winning a fight
   int animate; //num of mobs animated, to keep track :-)
   int power_ranking; //Power Ranking for ToC
   sh_int target;
   sh_int target_limb;
   sh_int stable; /* Stable vnum */
   sh_int stablenum; /* Amount your stable can hold in it */
   sh_int stablecurr; /* Amount you have in your stable ATM */
   int whonum; /* Which Who?  Config option --Xerves */
   int flags; /* Whether the player is deadly and whatever else we add.      */
   int pkills; /* Number of pkills on behalf of clan */
   int pdeaths; /* Number of times pkilled (legally)  */
   int pranking;
   int mkills; /* Number of mobs killed     */
   int mdeaths; /* Number of deaths due to mobs       */
   int illegal_pk; /* Number of illegal pk's committed   */
   long int outcast_time; /* The time at which the char was outcast */
   NUISANCE_DATA *nuisance; /* New Nuisance structure */
   EXT_BV portalfnd; /* Portal list for the player */
   long int restore_time; /* The last time the char did a restore all */
   sh_int r_range_lo; /* room range */
   sh_int r_range_hi;
   sh_int m_range_lo; /* mob range  */
   sh_int m_range_hi;
   sh_int o_range_lo; /* obj range  */
   sh_int o_range_hi;
   sh_int per_str; //percent in str
   sh_int per_int; //percent in int
   sh_int per_wis; //percent in wis
   sh_int per_dex; //percent in dex
   sh_int per_con; //percent in con
   sh_int per_lck; //percent in luck
   sh_int per_agi; //percent in agility
   sh_int per_hp;  //percent in hp
   sh_int per_mana;//percent in mana
   sh_int per_move;//percent in movement
   sh_int hit_cnt; //so players don't go out naked and ramp up hp/con
   sh_int mana_cnt;
   sh_int keeper; /* PC's Shop Keeper */
   sh_int wizinvis; /* wizinvis level */
   sh_int min_snoop; /* minimum snoop level */
   sh_int snum[5]; /* Used to hold the number of skills they can get in the beginning -- Xer */
   sh_int condition[MAX_CONDS];
   sh_int learned[MAX_SKILL];
   sh_int ranking[MAX_SKILL]; /* Mastery ranking -- Xerves */
   sh_int spercent[MAX_SKILL]; //Percent of a point earned for a skill/spell
   sh_int spellgroups[MAX_GROUP + 1]; /* Not used anymore*/
   sh_int spellpoints[MAX_GROUP + 1]; /* Not used anymore*/
   int spherepoints[MAX_SPHERE+1];
   int grouppoints[MAX_GROUP+6];
   /* Please note use above 2 only for spells unless you change the system -- Xerves */
   KILLED_DATA killed[MAX_KILLTRACK];
   int pkilled; //keep track of how many pkilled
   sh_int favor; /* deity favor */
   int auth_state;
   time_t release_date; /* Auto-helling.. Altrag */
   char *helled_by;
   char *bio; /* Personal Bio */
   char *came_from; //Where did they come from anyway?
   char *authed_by; /* what crazy imm authed this name ;) */
   SKILLTYPE *special_skills[MAX_PERSONAL]; /* personalized skills/spells */
   char *prompt; /* User config prompts */
   char *fprompt; /* Fight prompts */
   char *subprompt; /* Substate prompt */
   sh_int pagerlen; /* For pager (NOT menus) */
   bool openedtourney;
   bool righthanded;
   bool slastname; //used to see if they have saved their lastname yet -- for newbies
   IGNORE_DATA *first_ignored; /* keep track of who to ignore */
   IGNORE_DATA *last_ignored;
   char **tell_history; /* for immortal only command lasttell */
   sh_int lt_index; /* last_tell index */
   char *email; /* Email address for finger - Samson */
   int icq; /* ICQ# for finger - Samson 1-4-99 */

   long imc_deaf; /* IMC channel def flags */
   long imc_allow; /* IMC channel allow flags */
   long imc_deny; /* IMC channel deny flags */
   char *rreply; /* IMC reply-to */
   char *rreply_name; /* IMC reply-to shown to char */
   char *ice_listen; /* ICE channels */
   ALIAS_DATA *first_alias;
   ALIAS_DATA *last_alias;
   GLOBAL_BOARD_DATA *board;
   time_t last_note[MAX_BOARD];
   int secedit; /* Overland Map OLC - Samson 8-1-99 */
   NOTE_DATA *in_progress;
   int battletype; //Arena
   CHAR_DATA *challenged; /* Arena *///pcdata - dump
   CHAR_DATA *betted_on; /* Arena *///pcdata - dump
   int bet_amt; /* Arena */
   int resource; //Pcdata-dump
   sh_int resourcetype; //Pcdata-dump
   time_t logon; //pcdata -Dump 
   time_t save_time; //pcdata -Dump
   sh_int forget[5];
   sh_int nolearn[5];
   char stackbuf[MAX_STRING_LENGTH];
   sh_int skincolor;
   sh_int haircolor;
   sh_int hairlength;
   sh_int hairstyle;
   sh_int eyecolor;
   sh_int cheight;
   sh_int cweight;
};



/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		18

struct liq_type
{
   char *liq_name;
   char *liq_color;
   sh_int liq_affect[3];
};


/*
 * Damage types from the attack_table[]
 * 24 types for new weapon types -- Xerves
 */
typedef enum
{
   DAM_HIT
}
damage_types;

#define MAX_DAM 1;


/*
 * Extra description data for a room or object.
 */
struct extra_descr_data
{
   EXTRA_DESCR_DATA *next; /* Next in list                     */
   EXTRA_DESCR_DATA *prev; /* Previous in list                 */
   char *keyword; /* Keyword in look/examine          */
   char *description; /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct obj_index_data
{
   OBJ_INDEX_DATA *next;
   OBJ_INDEX_DATA *next_sort;
   EXTRA_DESCR_DATA *first_extradesc;
   EXTRA_DESCR_DATA *last_extradesc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   MPROG_DATA *mudprogs; /* objprogs */
   EXT_BV progtypes; /* objprogs */
   char *name;
   char *short_descr;
   char *description;
   char *action_desc;
   int vnum;
   sh_int level;
   sh_int item_type;
   EXT_BV extra_flags;
   int magic_flags; /*Need more bitvectors for spells - Scryn */
   int wear_flags;
   sh_int count;
   float weight;
   int cost;
   int value[14];
   int serial;
   int imbueslots;
   sh_int layers;
   int rent; /* Unused */
   int cvnum; /* Copy of a Obj?  Used for merchants */
   sh_int cident; /* Identify the Caste Obj */
   int bless_dur;
   int sworthrestrict; //Minimum sworth restriction
};

//Trap data
struct trap_data
{
   TRAP_DATA *next;
   TRAP_DATA *prev;
   int uid; //Unique id for the trap
   int charges;
   int maxcharges;
   int type;
   int damlow;
   int damhigh;
   int room;  // 0 - Single target  1 - Damage all in room
   int difficulty;  //0 - 300.  Sets the difficulty to disarm without the tool set.  Use tool negate to set the value with the kit
   int toolkit; //Unique ID set in v0 of the tool kit.  Can use 1 vnum for all tool kits if wanted
   OBJ_DATA *obj; 
   AREA_DATA *area; //Save us some time of finding the reset this way.
   int onetime;
   int disarmed;
   EXT_BV trapflags;
   int resetvalue; //-1 Reset every time area resets, 0 - Reset everytime obj resets, 1 > reset every minute (if obj exists)
   int toolnegate; //How much of the difficulty does the tool negate, 100 = all 50 = half 0 = none (tool must be set/found)
   int frag;  //Destroy obj if triggered.
   int sworthrestrict; //Minimum sworth restriction
};

struct imbue_data
{
   IMBUE_DATA *next;
   IMBUE_DATA *prev;
   int type;
   int sworth;
   int lowvalue;
   int highvalue;
   int value;
   int type2;
   int sworth2;
   int lowvalue2;
   int highvalue2;
   int value2;
   int type3;
   int sworth3;
   int lowvalue3;
   int highvalue3;
   int value3;
   int plevel;
   int gemnum;
};

/*
 * One object.
 */
struct obj_data
{
   OBJ_DATA *next;
   OBJ_DATA *prev;
   OBJ_DATA *next_content;
   OBJ_DATA *prev_content;
   OBJ_DATA *first_content;
   OBJ_DATA *last_content;
   OBJ_DATA *in_obj;
   CHAR_DATA *carried_by;
   CHAR_DATA *possessed_by;
   EXTRA_DESCR_DATA *first_extradesc;
   EXTRA_DESCR_DATA *last_extradesc;
   AFFECT_DATA *first_affect;
   AFFECT_DATA *last_affect;
   OBJ_INDEX_DATA *pIndexData;
   ROOM_INDEX_DATA *in_room;
   TIMER *first_timer;
   TIMER *last_timer;
   TRAP_DATA *trap;
   IMBUE_DATA *first_imbue;
   IMBUE_DATA *last_imbue;
   int imbueslots;
   char *name;
   char *short_descr;
   char *description;
   char *action_desc;
   sh_int item_type;
   sh_int mpscriptpos;
   EXT_BV extra_flags;
   int magic_flags; /*Need more bitvectors for spells - Scryn */
   int wear_flags;
   MPROG_ACT_LIST *mpact; /* mudprogs */
   int mpactnum; /* mudprogs */
   sh_int wear_loc;
   float weight;
   int cost;
   sh_int level;
   sh_int timer;
   int value[14];
   sh_int count; /* support for object grouping */
   int serial; /* serial number        */
   int cvnum; /* Copy of Obj?  Merchant use LP1 */
   sh_int cident; /* Used to identify Caste Objs */
   COORD_DATA *coord; /* Object coordinates on overland maps - Samson 8-21-99 */
   sh_int map; /* Which map is it on? - Samson 8-21-99 */
   sh_int fightm;
   int bless_dur;
   int sworthrestrict; //Minimum sworth restriction
};


/*
 * Exit data.
 */
struct exit_data
{
   EXIT_DATA *prev; /* previous exit in linked list */
   EXIT_DATA *next; /* next exit in linked list */
   EXIT_DATA *rexit; /* Reverse exit pointer  */
   ROOM_INDEX_DATA *to_room; /* Pointer to destination room */
   char *keyword; /* Keywords for exit or door */
   char *description; /* Description of exit  */
   int vnum; /* Vnum of room exit leads to */
   int rvnum; /* Vnum of room in opposite dir */
   int exit_info; /* door states & other flags */
   int key; /* Key vnum   */
   sh_int vdir; /* Physical "direction"  */
   sh_int distance; /* how far to the next room */
   sh_int pull; /* pull of direction (current) */
   sh_int pulltype; /* type of pull (current, wind) */
   COORD_DATA *coord; /* Coordinates to Overland Map - Samson 7-31-99 */

};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct reset_data
{
   RESET_DATA *next;
   RESET_DATA *prev;
   char command;
   int extra;
   int arg1;
   int arg2;
   int arg3;
   /* New Three are for new Wilderness/Overland..x, y, map */
   int arg4;
   int arg5;
   int arg6;
   int arg7; //for Forge Item Resets
   int serial; //For mobile Resets, removing the "numbered" crap for resets.
   int resetlast;
   int resettime;
};

/* Constants for arg2 of 'B' resets. */
#define	BIT_RESET_DOOR			0
#define BIT_RESET_OBJECT		1
#define BIT_RESET_MOBILE		2
#define BIT_RESET_ROOM			3
#define BIT_RESET_TYPE_MASK		0xFF /* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK		0xFF00 /* 256 should be enough */
#define BIT_RESET_SET			BV30
#define BIT_RESET_TOGGLE		BV31
#define BIT_RESET_FREEBITS	  0x3FFF0000 /* For reference */


// Ship structure, for ships.  Rejoice
struct ship_data
{
   SHIP_DATA *next;
   SHIP_DATA *prev;
   CHAR_DATA *first_char;
   CHAR_DATA *last_char;
   int uid;
   int x;
   int y;
   int map;
   int tx;
   int ty;
   int occupants;
   int tmap;
   int direction;
   int size;
   int ticket;
   char *travelroute;
   int routeplace;
   int routedir;
   int routetime; //Amount of ticks (5 seconds a tick) to wait before running back down route
   int routetick;
};

/*
 * Area definition.
 */
struct area_data
{
   AREA_DATA *next;
   AREA_DATA *prev;
   AREA_DATA *next_sort;
   AREA_DATA *prev_sort;
   AREA_DATA *next_sort_name; /* Used for alphanum. sort */
   AREA_DATA *prev_sort_name; /* Ditto, Fireblade */
   RESET_DATA *first_reset;
   RESET_DATA *last_reset;
   char *name;
   char *filename;
   int flags;
   sh_int status; /* h, 8/11 */
   sh_int age;
   sh_int nplayer;
   int res_tree;
   int res_corn;
   int res_grain;
   int res_iron;
   int res_gold;
   int res_stone;
   int gold;
   int salestax;
   int poptax;
   char *kowner;
   int recall;
   int death;
   int minhappoint;
   int minwithdraw;
   int lasttaxchange;
   sh_int reset_frequency;
   int low_r_vnum;
   int hi_r_vnum;
   int low_o_vnum;
   int hi_o_vnum;
   sh_int low_m_vnum;
   sh_int hi_m_vnum;
   int low_soft_range;
   int hi_soft_range;
   int low_hard_range;
   int hi_hard_range;
   char *author; /* Scryn */
   char *resetmsg; /* Rennard */
   RESET_DATA *last_mob_reset;
   RESET_DATA *last_obj_reset;
   sh_int max_players;
   int population; //For Kingdoms, actual peasant population
   int numroom; //Number of rooms, for population control
   int mload; //Number of "population" mobs loaded
   int mloadtop; //Max number of "population" loadable
   int mkills;
   int mdeaths;
   int pkills;
   int pdeaths;
   int gold_looted;
   int illegal_pk;
   int high_economy;
   int low_economy;
   int kingdom;
   int kpid;
   sh_int map;
   sh_int x;
   sh_int y;
};



/*
 * Load in the gods building data. -- Altrag
 */
struct godlist_data
{
   GOD_DATA *next;
   GOD_DATA *prev;
   int level;
   int low_r_vnum;
   int hi_r_vnum;
   int low_o_vnum;
   int hi_o_vnum;
   sh_int low_m_vnum;
   sh_int hi_m_vnum;
};

int start_marketpid;
//Market data
struct market_data
{
   MARKET_DATA *next;
   MARKET_DATA *prev;
   int cost;
   char *name;
   int pid;
   int mpid;
   int count;
   int scount;
   OBJ_DATA *obj;
};
/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct system_data
{
   int maxplayers; /* Maximum players this boot   */
   int alltimemax; /* Maximum players ever   */
   int global_looted; /* Gold looted this boot */
   int upill_val; /* Used pill value */
   int upotion_val; /* Used potion value */
   int brewed_used; /* Brewed potions used */
   int scribed_used; /* Scribed scrolls used */
   int start_calender; //RL start time of the calender
   int lastrescheck; //Last time a resource was checked, again real time
   int lastpopcheck; //Last time a population was checked, again real time
   int lasttaxcheck; //Last time a population tax was checked
   int accounts; //Accounts made this hour
   int max_accounts; //Maximum accounts allowed per hour
   int max_account_changes; //Maximum amount of account changes per hour
   int lastaccountreset; //Last time the account per hour time was reset
   char *time_of_max; /* Time of max ever */
   char *mud_name; /* Name of mud */
   char *mversion; //Version of the mud (The version of your code, can change to whatever you like)
   char *cversion; //Version of the code used (the original code, like Fear 1.0.1)
   int firstrun;
   bool NO_NAME_RESOLVING; /* Hostnames are not resolved  */
   bool DENY_NEW_PLAYERS; /* New players cannot connect  */
   bool WAIT_FOR_AUTH; /* New players must be auth'ed */
   sh_int read_all_mail; /* Read all player mail(was 54) */
   sh_int read_mail_free; /* Read mail for free (was 51) */
   sh_int write_mail_free; /* Write mail for free(was 51) */
   sh_int take_others_mail; /* Take others mail (was 54)   */
   sh_int imc_mail_vnum; /* Board vnum for IMC mail     */
   sh_int imc_mail_level; /* Min level to send IMC mail  */
   sh_int muse_level; /* Level of muse channel */
   int timeout_login; //Login timeout, default 3 minutes
   int timeout_notes; //Notes/Menu timeout, default 10 minutes
   int timeout_idle; //Playing the game, default 20 minutes
   int gem_vnum; // Default gem vnum
   sh_int think_level; /* Level of think channel LEVEL_HIGOD */
   sh_int build_level; /* Level of build channel LEVEL_BUILD */
   sh_int log_level; /* Level of log channel LEVEL LOG */
   sh_int level_modify_proto; /* Level to modify prototype stuff LEVEL_LESSER */
   sh_int level_override_private; /* override private flag */
   sh_int level_mset_player; /* Level to mset a player */
   sh_int bash_plr_vs_plr; /* Bash mod player vs. player */
   sh_int bash_nontank; /* Bash mod basher != primary attacker */
   sh_int gouge_plr_vs_plr; /* Gouge mod player vs. player */
   sh_int gouge_nontank; /* Gouge mod player != primary attacker */
   sh_int stun_plr_vs_plr; /* Stun mod player vs. player */
   sh_int stun_regular; /* Stun difficult */
   sh_int dodge_mod; /* Divide dodge chance by */
   sh_int parry_mod; /* Divide parry chance by */
   sh_int tumble_mod; /* Divide tumble chance by */
   sh_int max_kingdom; // Max kingdoms, don't adjust unless you know what you are doing.
   sh_int last_portal;
   sh_int last_trap_uid;
   sh_int last_invtrap_uid;
   sh_int dam_plr_vs_plr; /* Damage mod player vs. player */
   sh_int dam_plr_vs_mob; /* Damage mod player vs. mobile */
   sh_int dam_mob_vs_plr; /* Damage mod mobile vs. player */
   sh_int dam_mob_vs_mob; /* Damage mod mobile vs. mobile */
   sh_int level_getobjnotake; /* Get objects without take flag */
   sh_int level_forcepc; /* The level at which you can use force on players. */
   sh_int bestow_dif; /* Max # of levels between trust and command level for a bestow to work --Blodkai */
   sh_int max_sn; /* Max skills */
   int    top_pid; //Top pid
   int    top_kpid; //Top Kingdom pid
   int    top_tpid; //Top Town Pid
   char *guild_overseer; /* Pointer to char containing the name of the */
   char *guild_advisor; /* guild overseer and advisor. */
   int save_flags; /* Toggles for saving conditions */
   sh_int save_frequency; /* How old to autosave someone */
   sh_int check_imm_host; /* Do we check immortal's hosts? */
   sh_int morph_opt; /* Do we optimize morph's? */
   sh_int save_pets; /* Do pets save? */
   sh_int ban_site_level; /* Level to ban sites */
   sh_int ban_class_level; /* Level to ban classes */
   sh_int ban_race_level; /* Level to ban races */
   sh_int ident_retries; /* Number of times to retry broken pipes. */
   sh_int newbie_purge; /* Level to auto-purge newbies at - Samson 12-27-98 */
   sh_int regular_purge; /* Level to purge normal players at - Samson 12-27-98 */
   bool CLEANPFILES; /* Should the mud clean up pfiles daily? - Samson 12-27-98 */
   time_t purgetime; /* Time at which to purge the files - Samson 5-9-99 */
   int top_gem_num; //Top number for a gem;
   sh_int exp_percent; /* Now adjusts spoints gained in a victory, is a percentage.  Also affects learn_from_succ */
   sh_int stat_gain; // Adjusts the stat gain formulas for the game, search the variable for values */
   sh_int quest_item1; //No longer used, really need to delete sometime soon.
   sh_int quest_value1;
   sh_int quest_item2;
   sh_int quest_value2;
   sh_int quest_item3;
   sh_int quest_value3;
   sh_int quest_item4;
   sh_int quest_value4;
   sh_int quest_item5;
   sh_int quest_value5;
   sh_int quest_item6;
   sh_int quest_value6;
   sh_int quest_item7;
   sh_int quest_value7;
   sh_int quest_item8;
   sh_int quest_value8;
   sh_int quest_item9;
   sh_int quest_value9;
   sh_int quest_item10;
   sh_int quest_value10;
   sh_int quest_item11;
   sh_int quest_value11;
   sh_int quest_item12;
   sh_int quest_value12;
   //number of spheres by number of groups by Tier 2 and up.
   sh_int toadvance[MAX_SPHERE][MAX_GROUP][MAX_TIER-1]; //Stores the toadvance values used in listgroups and learn.
   int accountemail;
   int resetgame; //Reset game or not...
};



struct plane_data
{
   PLANE_DATA *next;
   PLANE_DATA *prev;
   char *name;
};



/*
 * Room type.
 */
struct room_index_data
{
   ROOM_INDEX_DATA *next;
   ROOM_INDEX_DATA *next_sort;
   CHAR_DATA *first_person; /* people in the room */
   CHAR_DATA *last_person; /*  ..  */
   OBJ_DATA *first_content; /* objects on floor  */
   OBJ_DATA *last_content; /*  ..  */
   EXTRA_DESCR_DATA *first_extradesc; /* extra descriptions */
   EXTRA_DESCR_DATA *last_extradesc; /*  ..  */
   AREA_DATA *area;
   EXIT_DATA *first_exit; /* exits from the room */
   EXIT_DATA *last_exit; /*  ..  */
   AFFECT_DATA *first_affect; /* effects on the room */
   AFFECT_DATA *last_affect; /*  ..  */
   MAP_DATA *map; /* maps */
   PLANE_DATA *plane; /* do it by room rather than area */
   MPROG_ACT_LIST *mpact; /* mudprogs */
   int mpactnum; /* mudprogs */
   MPROG_DATA *mudprogs; /* mudprogs */
   sh_int mpscriptpos;
   char *name;
   char *description;
   int vnum;
   EXT_BV room_flags;
   EXT_BV progtypes; /* mudprogs */
   sh_int light; /* amount of light in the room */
   sh_int sector_type;
   int tele_vnum;
   sh_int tele_delay;
   sh_int tunnel; /* max people that will fit */
   int resource; /* For Mining and Fielding --Xerves */
   int quad; /* Used for Quadrants in the Wilderness */
   int node_mana; /* if room is a node, how much mana is stored there */

};

/*
 * Delayed teleport type.
 */
struct teleport_data
{
   TELEPORT_DATA *next;
   TELEPORT_DATA *prev;
   ROOM_INDEX_DATA *room;
   sh_int timer;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               -1
#define TYPE_PROJECTILE              999 //Projectile hit
#define TYPE_HIT                     1000 /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000 /* allows for 1000 attack types  */
#define TYPE_PERSONAL		     3000 /* allows for 1000 herb types    */
#define TYPE_RACIAL		     4000 /* allows for 1000 personal types */
#define TYPE_DISEASE		     5000 /* allows for 1000 racial types  */

/*
 *  Target types.
 */
typedef enum
{
   TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF,
   TAR_OBJ_INV, TAR_OBJ_ROOM
}
target_types;

typedef enum
{
   SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
   SKILL_HERB, SKILL_RACIAL, SKILL_DISEASE
}
skill_types;



struct timerset
{
   int num_uses;
   struct timeval total_time;
   struct timeval min_time;
   struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct skill_type
{
   char *name; /* Name of skill  */
   sh_int bookv; // Vnum of book if it exists */
   sh_int race_level[MAX_RACE]; /* Racial abilities: level      */
   sh_int race_adept[MAX_RACE]; /* Racial abilities: adept      */
   /* The below's values are in handler.c in add_group */
   sh_int group[1]; /* In a group, 0 is no          */
   sh_int stype; //Sphere Type, Warrior, Mage, etc
   sh_int masterydiff[1]; /* At what mastery can they learn it? */
   sh_int mastery[1]; /* not really used anymore */
   sh_int bookinfo[1]; /* Who sells the spell book */
   SPELL_FUN *spell_fun; /* Spell pointer (for spells) */
   DO_FUN *skill_fun; /* Skill pointer (for skills) */
   sh_int target; /* Legal targets  */
   sh_int minimum_position; /* Position for caster / user */
   sh_int slot; /* Slot for #OBJECT loading */
   sh_int min_mana; /* Minimum mana used  */
   sh_int beats; /* Rounds required to use skill */
   sh_int targetlimb;
   char *noun_damage; /* Damage message  */
   char *msg_off; /* Wear off message  */
   sh_int guild; /* Which guild the skill belongs to */
   sh_int min_level; /* Minimum level to be able to cast */
   sh_int type; /* Spell/Skill/Weapon/Tongue */
   sh_int range; /* Range of spell (rooms) */
   int info; /* Spell action/class/etc */
   int flags; /* Flags   */
   char *hit_char; /* Success message to caster */
   char *hit_vict; /* Success message to victim */
   char *hit_room; /* Success message to room */
   char *hit_dest; /* Success message to dest room */
   char *miss_char; /* Failure message to caster */
   char *miss_vict; /* Failure message to victim */
   char *miss_room; /* Failure message to room */
   char *die_char; /* Victim death msg to caster */
   char *die_vict; /* Victim death msg to victim */
   char *die_room; /* Victim death msg to room */
   char *imm_char; /* Victim immune msg to caster */
   char *imm_vict; /* Victim immune msg to victim */
   char *imm_room; /* Victim immune msg to room */
   char *dice; /* Dice roll   */
   char *made_char; /* Made by??? -- Xerves 8/7/99 */
   sh_int prototype; /* Check for new spells/skills -- Xerves */
   int value; /* Misc value   */
   char saves; /* What saving spell applies */
   char difficulty; /* Difficulty of casting/learning */
   SMAUG_AFF *affects; /* Spell affects, if any */
   char *components; /* Spell components, if any */
   char *teachers; /* Skill requires a special teacher */
   char participants; /* # of required participants */
   struct timerset userec; /* Usage record   */
};


/* how many items to track.... prevent repeat auctions */
//This removes this function, I really don't care much for it anyway
#define AUCTION_MEM 0

struct auction_data
{
   OBJ_DATA *item; /* a pointer to the item */
   CHAR_DATA *seller; /* a pointer to the seller - which may NOT quit */
   CHAR_DATA *buyer; /* a pointer to the buyer - which may NOT quit */
   int bet; /* last bet - or 0 if noone has bet anything */
   sh_int going; /* 1,2, sold */
   sh_int pulse; /* how many pulses (.25 sec) until another call-out ? */
   int starting;
   OBJ_INDEX_DATA *history[AUCTION_MEM]; /* store auction history */
   sh_int hist_timer; /* clear out history buffer if auction is idle */
};

/*
 * So we can have different configs for different ports -- Shaddai
 */
extern int port;

/*
 * These are skill_lookup return values for common skills and spells.
 */
//New 2.1 skills, should be a lot :-)
extern sh_int gsn_combatart; 
extern sh_int gsn_weapon_axe;
extern sh_int gsn_weapon_sword;
extern sh_int gsn_weapon_polearm;
extern sh_int gsn_weapon_blunt;
extern sh_int gsn_weapon_staff;
extern sh_int gsn_weapon_projectile;
extern sh_int gsn_weapon_dagger;
extern sh_int gsn_roar;
extern sh_int gsn_bash;
extern sh_int gsn_inhuman_strength;
extern sh_int gsn_krundo_style;
extern sh_int gsn_rwundo_style;
extern sh_int gsn_krundi_style;
extern sh_int gsn_rwundi_style;
extern sh_int gsn_pincer;
extern sh_int gsn_weaponbreak;
extern sh_int gsn_powerslice;
extern sh_int gsn_deshield;
extern sh_int gsn_perfect_shot;
extern sh_int gsn_drive;
extern sh_int gsn_insult;
extern sh_int gsn_draw_aggression;
extern sh_int gsn_greater_draw_aggression;
extern sh_int gsn_focus_aggression;
extern sh_int gsn_greater_focus_aggression;
extern sh_int gsn_weapon_twohanded;

extern sh_int gsn_escapism;
extern sh_int gsn_prawl;
extern sh_int gsn_nightprawl;
extern sh_int gsn_lightprawl;
extern sh_int gsn_shadowfoot;
extern sh_int gsn_strongfoot;
extern sh_int gsn_swimming;
extern sh_int gsn_retreat;
extern sh_int gsn_gag;
extern sh_int gsn_climbwall;
extern sh_int gsn_vanish;
extern sh_int gsn_begging;
extern sh_int gsn_thiefeye;
extern sh_int gsn_cutpurse;
extern sh_int gsn_grab;
extern sh_int gsn_haggling;
extern sh_int gsn_swindling;
extern sh_int gsn_assassinate;
extern sh_int gsn_forage;
extern sh_int gsn_kickdirt;
extern sh_int gsn_weapon_daggerstudy;
extern sh_int gsn_weapon_daggerstrike;
extern sh_int gsn_startfire;
extern sh_int gsn_featherfoot;
extern sh_int gsn_cleansing;
extern sh_int gsn_concentration;
extern sh_int gsn_manafuse;
extern sh_int gsn_fasting;
extern sh_int gsn_nervepinch;
extern sh_int gsn_featherback;
extern sh_int gsn_manashot;
extern sh_int gsn_manaburst;
extern sh_int gsn_quickcombo;
extern sh_int gsn_nervestrike;

extern sh_int gsn_style_evasive;
extern sh_int gsn_style_wizardry;
extern sh_int gsn_style_divine;
extern sh_int gsn_style_defensive;
extern sh_int gsn_style_standard;
extern sh_int gsn_style_aggressive;
extern sh_int gsn_style_berserk;

extern sh_int gsn_detrap;
extern sh_int gsn_backstab;
extern sh_int gsn_circle;
extern sh_int gsn_cook;
extern sh_int gsn_study; /* Samson */
extern sh_int gsn_mountain_climb;
extern sh_int gsn_dodge;
extern sh_int gsn_hide;
extern sh_int gsn_peek;
extern sh_int gsn_pick_lock;
extern sh_int gsn_scan;
extern sh_int gsn_sneak;
extern sh_int gsn_steal;
extern sh_int gsn_gouge;
extern sh_int gsn_track;
extern sh_int gsn_search;
extern sh_int gsn_dig;
extern sh_int gsn_bashdoor;
extern sh_int gsn_berserk;
extern sh_int gsn_hitall;
extern sh_int gsn_stun;
extern sh_int gsn_daze;
extern sh_int gsn_repair;

extern sh_int gsn_balance;

extern sh_int gsn_unsheath;
extern sh_int gsn_disarm;
extern sh_int gsn_enhanced_damage;
extern sh_int gsn_parry;
extern sh_int gsn_rescue;
extern sh_int gsn_dual_wield;
extern sh_int gsn_shieldblock;
extern sh_int gsn_battle_knowledge;

/* Attack Skills -- Xerves */
extern sh_int gsn_roundhouse;
extern sh_int gsn_spinkick;
extern sh_int gsn_tornadokick;
extern sh_int gsn_niburo;
extern sh_int gsn_neckpinch;
extern sh_int gsn_neckchop;
extern sh_int gsn_neckrupture;
extern sh_int gsn_emeru;
extern sh_int gsn_elbowjab;
extern sh_int gsn_elbowstab;
extern sh_int gsn_elbowbreak;
extern sh_int gsn_amberio;
extern sh_int gsn_sidekick;
extern sh_int gsn_kneestrike;
extern sh_int gsn_kneecrusher;
extern sh_int gsn_lembecu;
extern sh_int gsn_blitz;
extern sh_int gsn_spear;
extern sh_int gsn_ribpuncture;
extern sh_int gsn_timmuru;

/* Swordsman new skills -- Xerves */
extern sh_int gsn_kick_back;
extern sh_int gsn_deadly_accuracy;
extern sh_int gsn_attack_frenzy;
extern sh_int gsn_critical;
extern sh_int gsn_counter;
extern sh_int gsn_possess;
extern sh_int gsn_lore;
extern sh_int gsn_aid;

/* used to do specific lookups */
extern sh_int gsn_first_spell;
extern sh_int gsn_first_skill;
extern sh_int gsn_first_weapon;
extern sh_int gsn_first_tongue;
extern sh_int gsn_top_sn;

/* spells */
extern sh_int gsn_blindness;
extern sh_int gsn_charm_person;
extern sh_int gsn_aqua_breath;
extern sh_int gsn_curse;
extern sh_int gsn_invis;
extern sh_int gsn_mass_invis;
extern sh_int gsn_poison;
extern sh_int gsn_sleep;
extern sh_int gsn_wizardeye;
extern sh_int gsn_eye_of_god;
extern sh_int gsn_summon_corpse;
extern sh_int gsn_lesser_resurrection;
extern sh_int gsn_resurrection;
extern sh_int gsn_greater_resurrection;
extern sh_int gsn_web;
extern sh_int gsn_snare;
extern sh_int gsn_extradimensional_portal;
extern sh_int gsn_revitalize_spirit;
extern sh_int gsn_holy_cleansing;

//extern sh_int gsn_fireball; /* for fireshield  */
//extern sh_int gsn_chill_touch; /* for iceshield   */
//extern sh_int gsn_lightning_bolt; /* for shockshield */

/* newer attack skills */
extern sh_int gsn_poison_weapon;
extern sh_int gsn_scribe;
extern sh_int gsn_brew;
extern sh_int gsn_climb;
extern sh_int gsn_manatap;
extern sh_int gsn_stalk;

extern sh_int gsn_hit;
extern sh_int gsn_grip;
extern sh_int gsn_slice;
extern sh_int gsn_tumble;
/* Language gsns. -- Altrag */
extern sh_int gsn_common;
extern sh_int gsn_elven;
extern sh_int gsn_dwarven;
extern sh_int gsn_pixie;
extern sh_int gsn_ogre;
extern sh_int gsn_orcish;
extern sh_int gsn_trollish;
extern sh_int gsn_goblin;
extern sh_int gsn_halfling;

/*
 * Cmd flag names --Shaddai
 */
extern char *const cmd_flags[];

/*
 * Utility macros.
 */
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))


/*
 * Old-style Bit manipulation macros
 *
 * The bit passed is the actual value of the bit (Use the BV## defines)
 */
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define TOGGLE_BIT(var, bit)	((var) ^= (bit))
#define CH(d)                  ((d)->original ? (d)->original : (d)->character)


/*
 * Macros for accessing virtually unlimited bitvectors.		-Thoric
 *
 * Note that these macros use the bit number rather than the bit value
 * itself -- which means that you can only access _one_ bit at a time
 *
 * This code uses an array of integers
 */

/*
 * The functions for these prototypes can be found in misc.c
 * They are up here because they are used by the macros below
 */
bool ext_is_empty args((EXT_BV * bits));
void ext_clear_bits args((EXT_BV * bits));
int ext_has_bits args((EXT_BV * var, EXT_BV * bits));
bool ext_same_bits args((EXT_BV * var, EXT_BV * bits));
void ext_set_bits args((EXT_BV * var, EXT_BV * bits));
void ext_remove_bits args((EXT_BV * var, EXT_BV * bits));
void ext_toggle_bits args((EXT_BV * var, EXT_BV * bits));

/*
 * Here are the extended bitvector macros:
 */
#define wIS_SET(ch, bit)        (is_set_wilderness((ch), (bit), (ch->coord->x), (ch->coord->y), (ch->map)))
#define wREMOVE_BIT(ch, bit)    (remove_bit_wilderness((ch), (bit)))
#define wSET_BIT(ch, bit)       (set_bit_wilderness((ch), (bit)))
#define xIS_SET(var, bit)	((var).bits[(bit) >> RSV] & 1 << ((bit) & XBM))
#define xSET_BIT(var, bit)	((var).bits[(bit) >> RSV] |= 1 << ((bit) & XBM))
#define xSET_BITS(var, bit)	(ext_set_bits(&(var), &(bit)))
#define xREMOVE_BIT(var, bit)	((var).bits[(bit) >> RSV] &= ~(1 << ((bit) & XBM)))
#define xREMOVE_BITS(var, bit)	(ext_remove_bits(&(var), &(bit)))
#define xTOGGLE_BIT(var, bit)	((var).bits[(bit) >> RSV] ^= 1 << ((bit) & XBM))
#define xTOGGLE_BITS(var, bit)	(ext_toggle_bits(&(var), &(bit)))
#define xCLEAR_BITS(var)	(ext_clear_bits(&(var)))
#define xIS_EMPTY(var)		(ext_is_empty(&(var)))
#define xHAS_BITS(var, bit)	(ext_has_bits(&(var), &(bit)))
#define xSAME_BITS(var, bit)	(ext_same_bits(&(var), &(bit)))

/*
 * Memory allocation macros.
 */

#define CREATE(result, type, number)				\
do								\
{								\
    if (!((result) = (type *) calloc ((number), sizeof(type))))	\
    {								\
	perror("malloc failure");				\
	fprintf(stderr, "Malloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)

#define RECREATE(result,type,number)				\
do								\
{								\
    if (!((result) = (type *) realloc ((result), sizeof(type) * (number))))\
    {								\
	perror("realloc failure");				\
	fprintf(stderr, "Realloc failure @ %s:%d\n", __FILE__, __LINE__ ); \
	abort();						\
    }								\
} while(0)


#define DISPOSE(point) 						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "DISPOSEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free(point);						\
  point = NULL;							\
} while(0)

#ifdef HASHSTR
#ifdef MTRACE
#define STRALLOC(point)		_str_alloc((point), __FILE__, __FUNCTION__, __LINE__)
#else
#define STRALLOC(point)		str_alloc((point))
#endif
#define QUICKLINK(point)	quick_link((point))
#define QUICKMATCH(p1, p2)	(int) (p1) == (int) (p2)
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else if (str_free((point))==-1) 				\
    fprintf( stderr, "STRFREEing bad pointer in %s, line %d\n", __FILE__, __LINE__ ); \
  point = NULL;							\
} while(0)
#else
#ifdef MTRACE
#define STRALLOC(point)		_str_dup((point), __FILE__, __FUNCTION__, __LINE__)
#define QUICKLINK(point)	_str_dup((point), __FILE__, __FUNCTION__, __LINE__)
#else
#define STRALLOC(point)		str_dup((point))
#define QUICKLINK(point)	str_dup((point))
#endif
#define QUICKMATCH(p1, p2)	strcmp((p1), (p2)) == 0
#define STRFREE(point)						\
do								\
{								\
  if (!(point))							\
  {								\
	bug( "Freeing null pointer %s:%d", __FILE__, __LINE__ ); \
	fprintf( stderr, "STRFREEing NULL in %s, line %d\n", __FILE__, __LINE__ ); \
  }								\
  else free((point));						\
  point = NULL;							\
} while(0)
#endif

/* double-linked list handling macros -Thoric */

#define LINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (last)->next		= (link);			\
    (link)->next		= NULL;				\
    (link)->prev		= (last);			\
    (last)			= (link);			\
} while(0)

#define MLINK(link, list, first, last, next, prev)			\
do								\
{								\
    if ( !(first) )						\
      (first)			= (link);			\
    else							\
      (list)->next		= (link);			\
    (list)->next		= NULL;				\
    (list)->prev		= (last);			\
    (last)			= (link);			\
} while(0)


//link - Item being inserted (the new pointer being added in)
//insert - Spot that is being inserted (the link is inserted before the insert) (the old pointer being shifted)
#define INSERT(link, insert, first, next, prev)			\
do								\
{								\
    (link)->prev		= (insert)->prev;		\
    if ( !(insert)->prev )					\
      (first)			= (link);			\
    else							\
      (insert)->prev->next	= (link);			\
    (insert)->prev		= (link);			\
    (link)->next		= (insert);			\
} while(0)        
     
//Shifts a link down one spot...       
#define SHIFT_DOWN(slink, olink, last)                                  \
do                                                                      \
{                                                                       \
   if ( (last) == (olink) )                                             \
      (last) = (slink);                                                 \
                                                                        \
    (olink)->prev = (slink)->prev;                                      \
    if ((olink)->prev)                                                  \
       (olink)->prev->next = (olink);                                   \
    (slink)->next = (olink)->next;                                      \
    if ((slink)->next)                                                  \
       (slink)->next->prev = (slink);                                   \
                                                                        \
    (olink)->next = (slink);                                            \
    (slink)->prev = (olink);                                            \
} while(0)      
             
//Shifts a link up one spot... 
#define SHIFT_UP(slink, olink, first)                                   \
do                                                                      \
{                                                                       \
   if ((first) == (olink))                                              \
      (first) = (slink);                                                \
                                                                        \
    (slink)->prev = (olink)->prev;                                      \
    if ((slink)->prev)                                                  \
       (slink)->prev->next = (slink);                                   \
    (olink)->next = (slink)->next;                                      \
    if ((olink)->next)                                                  \
       (olink)->next->prev = (olink);                                   \
                                                                        \
    (slink)->next = (olink);                                            \
    (olink)->prev = (slink);                                            \
} while(0)
   
          

#define UNLINK(link, first, last, next, prev)			\
do								\
{								\
    if ( !(link)->prev )					\
      (first)			= (link)->next;			\
    else							\
      (link)->prev->next	= (link)->next;			\
    if ( !(link)->next )					\
      (last)			= (link)->prev;			\
    else							\
      (link)->next->prev	= (link)->prev;			\
} while(0)

#define CHECK_LINKS(first, last, next, prev, type)		\
do {								\
  type *ptr, *pptr = NULL;					\
  if ( !(first) && !(last) )					\
    break;							\
  if ( !(first) )						\
  {								\
    bug( "CHECK_LINKS: last with NULL first!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (last); ptr->prev; ptr = ptr->prev );		\
    (first) = ptr;						\
  }								\
  else if ( !(last) )						\
  {								\
    bug( "CHECK_LINKS: first with NULL last!  %s.",		\
        __STRING(first) );					\
    for ( ptr = (first); ptr->next; ptr = ptr->next );		\
    (last) = ptr;						\
  }								\
  if ( (first) )						\
  {								\
    for ( ptr = (first); ptr; ptr = ptr->next )			\
    {								\
      if ( ptr->prev != pptr )					\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->prev = pptr;					\
      }								\
      if ( ptr->prev && ptr->prev->next != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->prev->next != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->prev->next = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
    pptr = NULL;						\
  }								\
  if ( (last) )							\
  {								\
    for ( ptr = (last); ptr; ptr = ptr->prev )			\
    {								\
      if ( ptr->next != pptr )					\
      {								\
        bug( "CHECK_LINKS (%s): %p:->next != %p.  Fixing.",	\
            __STRING(first), ptr, pptr );			\
        ptr->next = pptr;					\
      }								\
      if ( ptr->next && ptr->next->prev != ptr )		\
      {								\
        bug( "CHECK_LINKS(%s): %p:->next->prev != %p.  Fixing.",\
            __STRING(first), ptr, ptr );			\
        ptr->next->prev = ptr;					\
      }								\
      pptr = ptr;						\
    }								\
  }								\
} while(0)


#define ASSIGN_GSN(gsn, skill)					\
do								\
{								\
    if ( ((gsn) = skill_lookup((skill))) == -1 )		\
	fprintf( stderr, "ASSIGN_GSN: Skill %s not found.\n",	\
		(skill) );					\
} while(0)

#define CHECK_SUBRESTRICTED(ch)					\
do								\
{								\
    if ( (ch)->substate == SUB_RESTRICTED )			\
    {								\
	send_to_char( "You cannot use this command from within another command.\n\r", ch );	\
	return;							\
    }								\
} while(0)


/*
 * Character macros.
 */
/* Skill routines...Some are new, see skillmacros in /doc folder for more info */
#define GET_POINTS(ch, sn, isobj, lv)      (get_point_value((ch), (sn), (isobj), (lv)))
#define POINT_LEVEL(points, mastery)   (UMIN(100, (10*(mastery-1))+(3*(points))))
#define GET_MASTERY(ch, sn, isobj, lv)     (get_mastery_value((ch), (sn), (isobj), (lv)))
#define MASTERED(ch, sn)           (get_mastered_value((ch), (sn)))
#define LEARNED(ch,sn)            (get_learned_value((ch), (sn)))  
#define IS_VALID_COORDS(x, y)    ((x) > 0 && (y) > 0 && (x) <= MAX_X && (y) <= MAX_Y)
#define IN_WILDERNESS(ch)       ((ch)->coord->x > 0 && (ch)->coord->y > 0 && (ch)->map > -1)
#define IN_WILDERNESS_OBJ(obj)       ((obj)->coord->x > 0 && (obj)->coord->y > 0 && (obj)->map > -1)
#define IN_SAME_ROOM(ch, victim) ((ch)->in_room && (victim)->in_room && (ch)->in_room->vnum == (victim)->in_room->vnum && \
                                 (ch)->coord->x == (victim)->coord->x && (ch)->coord->y == (victim)->coord->y && (ch)->map == (victim)->map)
#define IN_SAME_ROOM_OBJ(ch, obj) ((ch)->in_room && (obj)->in_room && (ch)->in_room->vnum == (obj)->in_room->vnum && \
                                 (ch)->coord->x == (obj)->coord->x && (ch)->coord->y == (obj)->coord->y && (ch)->map == (obj)->map)
#define IS_NPC(ch)		(xIS_SET((ch)->act, ACT_IS_NPC))
#define IS_WILDERMOB(ch)        ((ch)->pIndexData->vnum >= OVERLAND_LOW_MOB && (ch)->pIndexData->vnum <= OVERLAND_HI_MOB)
#define IS_IMMORTAL(ch)		(get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust((ch)) >= LEVEL_IMMORTAL)
#define IS_AFFECTED(ch, sn)	(xIS_SET((ch)->affected_by, (sn)))
#define HAS_BODYPART(ch, part)	((ch)->xflags == 0 || IS_SET((ch)->xflags, (part)))
#define IS_STAFF(ch)            (!IS_NPC(ch)                                \
                                && (get_trust((ch)) >= LEVEL_STAFF  \
                                || (ch)->pcdata->caste >= caste_Staff)) /* Staff check -- Xerves */

#define CAN_CAST(ch)		(1)

#define IS_VAMPIRE(ch)		(0)
#define IS_QUESTOR(ch)     (xIS_SET((ch)->act, PLR_QUESTOR)) /* Questor -- Xerves */

#define IS_AWAKE(ch)		((ch)->position > POS_SLEEPING)
#define GET_AC(ch)		((ch)->armor				    \
				    + ( IS_AWAKE(ch)			    \
				    ? dex_app[get_curr_dex(ch)].defensive   \
				    : 0 )				    \
				    + VAMP_AC(ch))
/* Thanks to Chriss Baeke for noticing damplus was unused */
/* Hitroll/Damroll was changed into a function due to checking based on mastery */

/* A bunch of flag checking functions from Samson's code   */

/* Problems with mobs/players, so make a finding routine -- Xerves */
#define IS_ONMAP_FLAG(ch)           (IS_NPC(ch) ? xIS_SET((ch)->act, ACT_ONMAP) \
                                                : xIS_SET((ch)->act, PLR_ONMAP))
#define REMOVE_ONMAP_FLAG(ch)       (IS_NPC(ch) ? xREMOVE_BIT((ch)->act, ACT_ONMAP) \
                                                : xREMOVE_BIT((ch)->act, PLR_ONMAP))
#define SET_ONMAP_FLAG(ch)          (IS_NPC(ch) ? xSET_BIT((ch)->act, ACT_ONMAP) \
                                                : xSET_BIT((ch)->act, PLR_ONMAP))


#define REMOVE_PLR_FLAG(ch, flag)   (xREMOVE_BIT((ch)->act, flag))
#define IS_PLR_FLAG(ch, flag)       (xIS_SET((ch)->act, flag))
#define SET_PLR_FLAG(ch, flag)      (xSET_BIT((ch)->act, flag))

#define REMOVE_ACT_FLAG(ch, flag)   (xREMOVE_BIT((ch)->act, flag))
#define IS_ACT_FLAG(ch, flag)       (xIS_SET((ch)->act, flag))
#define SET_ACT_FLAG(ch, flag)      (xSET_BIT((ch)->act, flag))

#define REMOVE_OBJ_STAT(obj, flag)  (xREMOVE_BIT((obj)->extra_flags, flag))
#define SET_OBJ_STAT(obj, flag)     (xSET_BIT((obj)->extra_flags, flag))

#define GET_DAMROLL(ch)         (get_dam_roll(ch))
#define GET_HITROLL(ch)         (get_hit_roll(ch))
#define IN_SECTOR(ch, sect)     (find_sector(ch, sect))

#define IS_OUTSIDE(ch)          (IS_ONMAP_FLAG((ch)) ? !IN_SECTOR((ch), SECT_INSIDE) \
                                                     : !xIS_SET((ch)->in_room->room_flags, ROOM_INDOORS) \
                                                    && !xIS_SET((ch)->in_room->room_flags, ROOM_TUNNEL))

#define NO_WEATHER_SECT(sect)  (  sect == SECT_UNDERWATER ||                         \
                                  sect == SECT_OCEANFLOOR ||                         \
                                  sect == SECT_SGOLD || sect == SECT_NGOLD ||        \
                                  sect == SECT_SIRON || sect == SECT_NIRON ||        \
                                  sect == SECT_MINEGOLD || sect == SECT_MINEIRON ||  \
                                  sect == SECT_UNDERGROUND )
                                  
#define IS_MOUNTAIN(sect)      (  sect == SECT_MOUNTAIN || sect == SECT_SGOLD || \
                                  sect == SECT_NGOLD || sect == SECT_SIRON || sect == SECT_NIRON || \
                                  sect == SECT_MINEGOLD || sect == SECT_MINEIRON )
                                  
#define IS_NOSWIM(sect)         ( sect == SECT_WATER_NOSWIM || sect == SECT_RIVER)                                  

#define IS_DRUNK(ch, drunk)     (number_percent() < \
			        ( (ch)->pcdata->condition[COND_DRUNK] \
				* 2 / (drunk) ) )

#define IS_CLANNED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_ORDERED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_ORDER)

#define IS_GUILDED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type == CLAN_GUILD)

#define IS_DEADLYCLAN(ch)	(!IS_NPC((ch))				    \
				&& (ch)->pcdata->clan			    \
				&& (ch)->pcdata->clan->clan_type != CLAN_NOKILL) \
				&& (ch)->pcdata->clan->clan_type != CLAN_ORDER)  \
				&& (ch)->pcdata->clan->clan_type != CLAN_GUILD)

#define IS_DEVOTED(ch)		(!IS_NPC((ch))				    \
				&& (ch)->pcdata->deity)

#define IS_ROOM_SAFE(ch)         (ch->in_room && (xIS_SET(ch->in_room->room_flags, ROOM_SAFE) \
                                              || IS_SET(ch->in_room->area->flags, AFLAG_NOKILL)))

#define IS_ROOM_NOLOOT(ch)       (ch->in_room && (xIS_SET(ch->in_room->room_flags, ROOM_NOLOOT) \
                                              || IS_SET(ch->in_room->area->flags, AFLAG_NOLOOT)))

#define IS_ROOM_ANITEM(ch)       (ch->in_room && (xIS_SET(ch->in_room->room_flags, ROOM_ANITEM) \
                                              || IS_SET(ch->in_room->area->flags, AFLAG_ANITEM)))

#define IS_ROOM_FREEKILL(ch)     (ch->in_room && (xIS_SET(ch->in_room->room_flags, ROOM_FREEKILL) \
                                              || IS_SET(ch->in_room->area->flags, AFLAG_FREEKILL)))

#define IS_PKILL(ch)            (ch->pcdata)

#define CAN_PKILL(ch)           (ch->pcdata)

/* Addition to make people with nuisance flag have more wait */

#define HAS_WAIT(ch)            (ch->pcdata && ch->wait && ch->fighting)

#define WAIT_STATE(ch, npulse) ((ch)->wait=(!IS_NPC(ch)&&ch->pcdata->nuisance&&\
			      (ch->pcdata->nuisance->flags>4))?UMAX((ch)->wait,\
			      (npulse+((ch)->pcdata->nuisance->flags-4)+ \
               		      ch->pcdata->nuisance->power)): \
			      UMAX((ch)->wait, (IS_IMMORTAL(ch) ? 0 :(npulse))))


#define EXIT(ch, door)		( get_exit( (ch)->in_room, door ) )

#define CAN_GO(ch, door)	(EXIT((ch),(door))			 \
				&& (EXIT((ch),(door))->to_room != NULL)  \
                          	&& !IS_SET(EXIT((ch), (door))->exit_info, EX_CLOSED))

#define IS_FLOATING(ch)		( IS_AFFECTED((ch), AFF_FLYING) || IS_AFFECTED((ch), AFF_FLOATING) )

#define IS_VALID_SN(sn)		( (sn) >=0 && (sn) < MAX_SKILL		     \
				&& skill_table[(sn)]			     \
				&& skill_table[(sn)]->name )

#define IS_VALID_HERB(sn)	( (sn) >=0 && (sn) < MAX_HERB		     \
				&& herb_table[(sn)]			     \
				&& herb_table[(sn)]->name )

#define IS_VALID_DISEASE(sn)	( (sn) >=0 && (sn) < MAX_DISEASE	     \
				&& disease_table[(sn)]			     \
				&& disease_table[(sn)]->name )

#define IS_PACIFIST(ch)		(IS_NPC(ch) && xIS_SET(ch->act, ACT_PACIFIST))
#define IS_WITHIN(num, diff)    ( ((num) <= (num)+(diff)) && ((num) >= (num)-(diff)))

#define SPELL_FLAG(skill, flag)	( IS_SET((skill)->flags, (flag)) )
#define SPELL_DAMAGE(skill)	( ((skill)->info      ) & 7 )
#define SPELL_ACTION(skill)	( ((skill)->info >>  3) & 7 )
#define SPELL_CLASS(skill)	( ((skill)->info >>  6) & 7 )
#define SPELL_POWER(skill)	( ((skill)->info >>  9) & 3 )
#define SPELL_SAVE(skill)	( ((skill)->info >> 11) & 7 )
#define SET_SDAM(skill, val)	( (skill)->info =  ((skill)->info & SDAM_MASK) + ((val) & 7) )
#define SET_SACT(skill, val)	( (skill)->info =  ((skill)->info & SACT_MASK) + (((val) & 7) << 3) )
#define SET_SCLA(skill, val)	( (skill)->info =  ((skill)->info & SCLA_MASK) + (((val) & 7) << 6) )
#define SET_SPOW(skill, val)	( (skill)->info =  ((skill)->info & SPOW_MASK) + (((val) & 3) << 9) )
#define SET_SSAV(skill, val)	( (skill)->info =  ((skill)->info & SSAV_MASK) + (((val) & 7) << 11) )

/* Retired and guest imms. */
#define IS_RETIRED(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_RETIRED))
#define IS_GUEST(ch) (ch->pcdata && IS_SET(ch->pcdata->flags,PCFLAG_GUEST))

/* RIS by gsn lookups. -- Altrag.
   Will need to add some || stuff for spells that need a special GSN. */

#define IS_FIRE(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_FIRE )
#define IS_WATER(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_WATER )
#define IS_UNDEAD(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_UNDEAD )
#define IS_EARTH(dt)	( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_EARTH)
#define IS_AIR(dt)      ( IS_VALID_SN(dt) &&                \
                SPELL_DAMAGE(skill_table[(dt)]) == SD_AIR )
#define IS_ENERGY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_ENERGY )

#define IS_HOLY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_HOLY )

#define IS_UNHOLY(dt)		( IS_VALID_SN(dt) &&			     \
				SPELL_DAMAGE(skill_table[(dt)]) == SD_UNHOLY )


#define NOT_AUTHED(ch)		(!IS_NPC(ch) && ch->pcdata->auth_state <= 4  \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

#define IS_WAITING_FOR_AUTH(ch) (!IS_NPC(ch) && ch->desc		     \
			      && ch->pcdata->auth_state == 1		     \
			      && IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED) )

/*
 * Object macros.
 */
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(xIS_SET((obj)->extra_flags, (stat)))
#define IS_UNIQUE(ch, obj)  (get_obj_on_char((ch), (obj)))

/*
 * MudProg macros.						-Thoric
 */
#define HAS_PROG(what, prog)	(xIS_SET((what)->progtypes, (prog)))

/*
 * Description macros.
 */
#define PERS(ch, looker)        ( show_pers_output((ch), (looker), 0, -1)) //Has to be in the same room
#define PERS_MAP(ch, looker)    ( show_pers_output((ch), (looker), 1, -1)) //Global
#define PERS_KINGDOM(ch, kingdom) ( show_pers_output((ch), NULL, 2, (kingdom))) //For Kingdoms
#define PERS_MAP_NAME(ch, looker) ( show_pers_output((ch), (looker), 3, -1)) //Forces ch->name for NPCs
#define MORPHPERS(ch, looker)   ( can_see( (looker), (ch) ) ?           \
                                (ch)->morph->morph->short_desc       \
                                : "someone" )
                                
#define IN_VALID_KINGDOM(num)   ( (num) >= 0 && (num)< sysdata.max_kingdom)                                
#define IN_PLAYER_KINGDOM(num)  ( (num) >= 2 && (num) < sysdata.max_kingdom)


#define log_string(txt)		( log_string_plus( (txt), LOG_NORMAL, LEVEL_LOG ) )

/*
 *  Defines for the command flags. --Shaddai
 */
#define	CMD_FLAG_POSSESS	BV00
#define CMD_FLAG_POLYMORPHED	BV01
#define CMD_WATCH		BV02 /* FB */

/*
 * Structure for a command in the command lookup table.
 */
struct cmd_type
{
   CMDTYPE *next;
   char *name;
   DO_FUN *do_fun;
   int flags; /* Added for Checking interpret stuff -Shaddai */
   sh_int position;
   sh_int level;
   sh_int log;
   sh_int fcommand; //Fight command 1 - Cannot use if timered, 0 Can
   struct timerset userec;
   int lag_count; /* count lag flags for this cmd - FB */
};



/*
 * Structure for a social in the socials table.
 */
struct social_type
{
   SOCIALTYPE *next;
   char *name;
   char *char_no_arg;
   char *others_no_arg;
   char *char_found;
   char *others_found;
   char *vict_found;
   char *char_auto;
   char *others_auto;
};



/*
 * Global constants.
 */
extern time_t last_restore_all_time;
extern time_t boot_time; /* this should be moved down */
extern HOUR_MIN_SEC *set_boot_time;
extern struct tm *new_boot_time;
extern time_t new_boot_time_t;
extern time_t copyover_time;
extern int global_eworth;
extern int global_sworth;
extern int global_plevel;

extern const struct str_app_type str_app[26];
extern const struct int_app_type int_app[26];
extern const struct wis_app_type wis_app[26];
extern const struct dex_app_type dex_app[26];
extern const struct con_app_type con_app[26];
extern const struct cha_app_type cha_app[26];
extern const struct lck_app_type lck_app[26];
//extern const struct agi_app_type agi_app[201];

extern char *const sector_message[SECT_MAX];

extern const struct sect_color_type sect_show[];
extern PORTAL_DATA *portal_show[LAST_PORTAL];
extern struct race_type *race_table[MAX_RACE];
extern const struct liq_type liq_table[LIQ_MAX];

/* 24 New attacks, much more exact -- Xerves */
extern char *const attack_table[1];
extern char **const s_message_table[1];
extern char **const p_message_table[1];

extern char *const skill_tname[];
extern sh_int const movement_loss[SECT_MAX];
extern char *const dir_name[];
extern char *const where_name[MAX_WHERE_NAME];
extern const sh_int rev_dir[];
extern const int trap_door[];
extern char *const r_flags[];
extern char *const ex_flags[];
extern char *const w_flags[];
extern char *const o_flags[];
extern char *const talent_flags[];
extern char *const buykobj_types[];
extern char *const buykmob_types[];
extern char *const a_flags[];
extern char *const qmob_flags[];
extern char *const o_types[];
extern char *const a_types[];
extern char *const act_flags[];
extern char *const mi_flags[];
extern char *const plr_flags[];
extern char *const pc_flags[];
extern char *const trap_flags[];
extern char *const element_flags[];
extern char *const ris_flags[];
extern char *const trig_flags[];
extern char *const part_flags[];
extern char *const npc_class[];
extern char *const defense_flags[];
extern char *const attack_flags[];
extern char *const area_flags[];
extern char *const ex_pmisc[];
extern char *const ex_pwater[];
extern char *const ex_pair[];
extern char *const ex_pearth[];
extern char *const ex_pfire[];

extern int const lang_array[];
extern char *const lang_names[];

extern char *const temp_settings[]; /* FB */
extern char *const precip_settings[];
extern char *const wind_settings[];
extern char *const preciptemp_msg[6][6];
extern char *const windtemp_msg[6][6];
extern char *const precip_msg[];
extern char *const wind_msg[];

/*
 * Global variables.
 */
extern MPSLEEP_DATA *first_mpwait; /* Storing sleeping mud progs */
extern MPSLEEP_DATA *last_mpwait; /* - */
extern MPSLEEP_DATA *current_mpwait; /* - */
extern char *bigregex;
extern char *preg;
extern int max_npc_race;

extern char *target_name;
extern char *ranged_target_name;
extern int file_ver;
extern int numobjsloaded;
extern int nummobsloaded;
extern int physicalobjects;
extern int last_pkroom;
extern int num_descriptors;
extern struct system_data sysdata;
extern int top_sn;
extern int top_vroom;
extern int top_herb;
extern int top_map_mob;
extern int global_x[1];
extern int global_y[1];
extern int global_map[1];
extern int gem_num;
extern int box_num;
extern int globaltownload; //Set to 1 when loading an object to a town bank
extern TOWN_DATA *globaltownptr;
extern MARKET_DATA *globalmarketptr;
extern int saving_mount_on_quit; //so you don't get the extraction message, etc
extern int global_drop_equip_message; //Drops equip unequip message if set...used for static quests to stop the spam

extern unsigned char battle_descriptions[7][3][100][60]; //7 types of hits, 3 messages, 6 weapons, with 100 possibilities with up to 100 characters
extern int high_value[7][3];

extern CMDTYPE *command_hash[126];

extern struct class_type *class_table[1];
extern KINGDOM_DATA *kingdom_table[MAX_KINGDOM];
extern NPCRACE_DATA *npcrace_table[MAX_NPCRACE_TABLE];

extern SKILLTYPE *skill_table[MAX_SKILL];
extern SOCIALTYPE *social_index[27];
extern CHAR_DATA *cur_char;
extern ROOM_INDEX_DATA *cur_room;
extern bool cur_char_died;
extern ch_ret global_retcode;
extern SKILLTYPE *herb_table[MAX_HERB];
extern SKILLTYPE *disease_table[MAX_DISEASE];

extern int blockdam;

extern int cur_obj;
extern int cur_obj_serial;
extern int forge_num;
extern int slab_num;
extern bool cur_obj_extracted;
extern obj_ret global_objcode;
extern bool serial_list[MAX_LOADED_MOBS];
extern int serialmobsloaded; //not quite the same as nummobsloaded, it has the ability to use past serials for "reset mobs" to reset mobs properly.
extern int cur_ship_uid;

extern HELP_DATA *first_help;
extern HELP_DATA *last_help;
extern HINDEX_DATA *first_hindex;
extern HINDEX_DATA *last_hindex;
extern HINDEX_DATA *first_fhindex; //Flat help index
extern HINDEX_DATA *last_fhindex; //Flag help index
extern SHOP_DATA *first_shop;
extern SHOP_DATA *last_shop;
extern REPAIR_DATA *first_repair;
extern REPAIR_DATA *last_repair;
extern WATCH_DATA *first_watch;
extern WATCH_DATA *last_watch;
extern CMEMBER_DATA *first_clanmember;
extern CMEMBER_DATA *last_clanmember;
extern KMEMBER_DATA *first_kingdommember;
extern KMEMBER_DATA *last_kingdommember;
extern TRAINER_DATA *first_trainer;
extern TRAINER_DATA *last_trainer;
extern GEM_DATA *first_gem;
extern GEM_DATA *last_gem;
extern AGGRO_DATA *first_global_aggro;
extern AGGRO_DATA *last_global_aggro;
extern TRADE_DATA *first_trade;
extern TRADE_DATA *last_trade;
extern TRAP_DATA *first_trap;
extern TRAP_DATA *last_trap;
extern BOX_DATA *first_box;
extern BOX_DATA *last_box;
extern FORGE_DATA *first_forge;
extern FORGE_DATA *last_forge;
extern WBLOCK_DATA *first_wblock;
extern WBLOCK_DATA *last_wblock;
extern SLAB_DATA *first_slab;
extern SLAB_DATA *last_slab;
extern KCHEST_DATA *first_kchest;
extern KCHEST_DATA *last_kchest;
extern BAN_DATA *first_ban;
extern BAN_DATA *last_ban;
extern BAN_DATA *first_ban_class;
extern BAN_DATA *last_ban_class;
extern BAN_DATA *first_ban_race;
extern BAN_DATA *last_ban_race;
extern RESERVE_DATA *first_reserved;
extern RESERVE_DATA *last_reserved;
extern CHAR_DATA *first_char;
extern CHAR_DATA *last_char;
extern MARKET_DATA *first_market;
extern MARKET_DATA *last_market;
extern SHIP_DATA *first_ship;
extern SHIP_DATA *last_ship;
extern CMAP_DATA *first_wilderchar;
extern CMAP_DATA *last_wilderchar;
extern OMAP_DATA *first_wilderobj;
extern OMAP_DATA *last_wilderobj;
extern DESCRIPTOR_DATA *first_descriptor;
extern DESCRIPTOR_DATA *last_descriptor;
extern BOARD_DATA *first_board;
extern BOARD_DATA *last_board;
extern PLANE_DATA *first_plane;
extern PLANE_DATA *last_plane;
extern PROJECT_DATA *first_project;
extern PROJECT_DATA *last_project;
extern AUTHORIZE_DATA *first_authorized;
extern AUTHORIZE_DATA *last_authorized;
extern BARENA_DATA *first_barena;
extern BARENA_DATA *last_barena;
extern BIN_DATA *first_bin;
extern BIN_DATA *last_bin;
extern OBJ_DATA *first_object;
extern OBJ_DATA *last_object;
extern CLAN_DATA *first_clan;
extern CLAN_DATA *last_clan;
extern COUNCIL_DATA *first_council;
extern COUNCIL_DATA *last_council;
extern DEITY_DATA *first_deity;
extern DEITY_DATA *last_deity;
extern AREA_DATA *first_area;
extern AREA_DATA *last_area;
extern QMOB_DATA *first_qmob;
extern QMOB_DATA *last_qmob;
extern QOBJ_DATA *first_qobj;
extern QOBJ_DATA *last_qobj;
extern NPCRACE_DATA *first_npcrace;
extern NPCRACE_DATA *last_npcrace;
extern QUEST_DATA *first_quest;
extern QUEST_DATA *last_quest;
extern AREA_DATA *first_build;
extern AREA_DATA *last_build;
extern FRONT_DATA *first_front;
extern FRONT_DATA *last_front;
extern CONQUER_DATA *first_conquer;
extern CONQUER_DATA *last_conquer;
extern TORNADO_DATA *first_tornado;
extern TORNADO_DATA *last_tornado;
extern AREA_DATA *first_asort;
extern AREA_DATA *last_asort;
extern AREA_DATA *first_bsort;
extern AREA_DATA *last_bsort;
extern AREA_DATA *first_area_name; /*alphanum. sort */
extern AREA_DATA *last_area_name; /* Fireblade */

extern LANG_DATA *first_lang;
extern LANG_DATA *last_lang;
extern BUYKBIN_DATA *first_buykbin;
extern BUYKBIN_DATA *last_buykbin;
extern BUYKMOB_DATA *first_buykmob;
extern BUYKMOB_DATA *last_buykmob;
extern BUYKOBJ_DATA *first_buykobj;
extern BUYKOBJ_DATA *last_buykobj;
extern BUYKTRAINER_DATA *first_buyktrainer;
extern BUYKTRAINER_DATA *last_buyktrainer;
extern BTRAINER_DATA *first_boughttrainer;
extern BTRAINER_DATA *last_boughttrainer;
extern TRAINING_DATA *first_training;
extern TRAINING_DATA *last_training;
extern CHANNEL_HISTORY *first_channelhistory;
extern CHANNEL_HISTORY *last_channelhistory;
/*
extern		GOD_DATA	  *	first_imm;
extern		GOD_DATA	  *	last_imm;
*/
extern TELEPORT_DATA *first_teleport;
extern TELEPORT_DATA *last_teleport;
extern OBJ_DATA *extracted_obj_queue;
extern EXTRACT_CHAR_DATA *extracted_char_queue;
extern OBJ_DATA *save_equipment[MAX_WEAR][MAX_LAYERS];
extern CHAR_DATA *quitting_char;
extern CHAR_DATA *loading_char;
extern CHAR_DATA *saving_char;
extern OBJ_DATA *all_obj;

extern char bug_buf[];
extern time_t current_time;
extern bool fLogAll;
extern FILE *fpReserve;
extern FILE *fpLOG;
extern char log_buf[];
extern TIME_INFO_DATA time_info;
extern IMMORTAL_HOST *immortal_host_start;
extern IMMORTAL_HOST *immortal_host_end;
extern int weath_unit;
extern int rand_factor;
extern int climate_factor;
extern int neigh_factor;
extern int max_vector;
extern int top_area;

extern AUCTION_DATA *auction;
extern struct act_prog_data *mob_act_list;


/*
 * Command functions.
 * Defined in act_*.c (mostly).
 */
DECLARE_DO_FUN(skill_notfound);
/*//T5*/
DECLARE_DO_FUN( do_aassign                     );
DECLARE_DO_FUN( do_accept                      );
DECLARE_DO_FUN( do_accounts                    );
DECLARE_DO_FUN( do_add_imm_host                );
DECLARE_DO_FUN( do_addbox                      );
DECLARE_DO_FUN( do_addforge                    );
DECLARE_DO_FUN( do_addgem                      );
DECLARE_DO_FUN( do_addslab                     );
DECLARE_DO_FUN( do_advance                     );
DECLARE_DO_FUN( do_advanceclock                );
DECLARE_DO_FUN( do_affected                    );
DECLARE_DO_FUN( do_afk                         );
DECLARE_DO_FUN( do_ahall                       );
DECLARE_DO_FUN( do_ahelp                       );
DECLARE_DO_FUN( do_aid                         );
DECLARE_DO_FUN( do_aim                         );
DECLARE_DO_FUN( do_allow                       );
DECLARE_DO_FUN( do_amberio                     );
DECLARE_DO_FUN( do_ansi                        );
DECLARE_DO_FUN( do_answer                      );
DECLARE_DO_FUN( do_apply                       );
DECLARE_DO_FUN( do_appraise                    );
DECLARE_DO_FUN( do_areas                       );
DECLARE_DO_FUN( do_arena                       );
DECLARE_DO_FUN( do_armmilitary                 );
DECLARE_DO_FUN( do_aset                        );
DECLARE_DO_FUN( do_ask                         );
DECLARE_DO_FUN( do_assassinate                 );
DECLARE_DO_FUN( do_astat                       );
DECLARE_DO_FUN( do_at                          );
DECLARE_DO_FUN( do_attack                      );
DECLARE_DO_FUN( do_atmob                       );
DECLARE_DO_FUN( do_atobj                       );
DECLARE_DO_FUN( do_auction                     );
DECLARE_DO_FUN( do_authorize                   );
DECLARE_DO_FUN( do_avtalk                      );
DECLARE_DO_FUN( do_awho                        );
DECLARE_DO_FUN( do_backstab                    );
DECLARE_DO_FUN( do_balzhur                     );
DECLARE_DO_FUN( do_bamfin                      );
DECLARE_DO_FUN( do_bamfout                     );
DECLARE_DO_FUN( do_ban                         );
DECLARE_DO_FUN( do_bash                        );
DECLARE_DO_FUN( do_bashdoor                    );
DECLARE_DO_FUN( do_bcard                       );
DECLARE_DO_FUN( do_beep                        );
DECLARE_DO_FUN( do_begging                     );
DECLARE_DO_FUN( do_berserk                     );
DECLARE_DO_FUN( do_bestow                      );
DECLARE_DO_FUN( do_bestowarea                  );
DECLARE_DO_FUN( do_bet                         );
DECLARE_DO_FUN( do_bhold                       );
DECLARE_DO_FUN( do_bio                         );
DECLARE_DO_FUN( do_bite                        );
DECLARE_DO_FUN( do_blackjack                   );
DECLARE_DO_FUN( do_blitz                       );
DECLARE_DO_FUN( do_blockmove                   );
DECLARE_DO_FUN( do_bloodlet                    );
DECLARE_DO_FUN( do_boards                      );
DECLARE_DO_FUN( do_bodybag                     );
DECLARE_DO_FUN( do_brandish                    );
DECLARE_DO_FUN( do_break                       );
DECLARE_DO_FUN( do_breakwall                   );
DECLARE_DO_FUN( do_brew                        );
DECLARE_DO_FUN( do_broach                      );
DECLARE_DO_FUN( do_bset                        );
DECLARE_DO_FUN( do_bstat                       );
DECLARE_DO_FUN( do_bug                         );
DECLARE_DO_FUN( do_build                       );
DECLARE_DO_FUN( do_buildroom                   );
DECLARE_DO_FUN( do_bury                        );
DECLARE_DO_FUN( do_buy                         );
DECLARE_DO_FUN( do_buycaste                    );
DECLARE_DO_FUN( do_cache                       );
DECLARE_DO_FUN( do_calmmount                   );
DECLARE_DO_FUN( do_carrybin                    );
DECLARE_DO_FUN( do_cast                        );
DECLARE_DO_FUN( do_caste                       );
DECLARE_DO_FUN( do_castevalues                 );
DECLARE_DO_FUN( do_cedit                       );
DECLARE_DO_FUN( do_challenge                   );
DECLARE_DO_FUN( do_changes                     );
DECLARE_DO_FUN( do_channels                    );
DECLARE_DO_FUN( do_chaos                       );
DECLARE_DO_FUN( do_chat                        );
DECLARE_DO_FUN( do_check_vnums                 );
DECLARE_DO_FUN( do_chistory                    );
DECLARE_DO_FUN( do_circle                      );
DECLARE_DO_FUN( do_cityvalues                  );
DECLARE_DO_FUN( do_clans                       );
DECLARE_DO_FUN( do_clantalk                    );
DECLARE_DO_FUN( do_cleansing                   );
DECLARE_DO_FUN( do_clearques                   );
DECLARE_DO_FUN( do_clearstack                  );
DECLARE_DO_FUN( do_climate                     );
DECLARE_DO_FUN( do_climb                       );
DECLARE_DO_FUN( do_climbwall                   );
DECLARE_DO_FUN( do_cloak                       );
DECLARE_DO_FUN( do_clones                      );
DECLARE_DO_FUN( do_close                       );
DECLARE_DO_FUN( do_cmdtable                    );
DECLARE_DO_FUN( do_command                     );
DECLARE_DO_FUN( do_commands                    );
DECLARE_DO_FUN( do_comment                     );
DECLARE_DO_FUN( do_compare                     );
DECLARE_DO_FUN( do_compress                    );
DECLARE_DO_FUN( do_config                      );
DECLARE_DO_FUN( do_conquer                     );
DECLARE_DO_FUN( do_consent                     );
DECLARE_DO_FUN( do_consider                    );
DECLARE_DO_FUN( do_cook                        );
DECLARE_DO_FUN( do_coords                      );
DECLARE_DO_FUN( do_copyover                    );
DECLARE_DO_FUN( do_council_induct              );
DECLARE_DO_FUN( do_council_outcast             );
DECLARE_DO_FUN( do_councils                    );
DECLARE_DO_FUN( do_counciltalk                 );
DECLARE_DO_FUN( do_counter                     );
DECLARE_DO_FUN( do_credits                     );
DECLARE_DO_FUN( do_cset                        );
DECLARE_DO_FUN( do_cutgag                      );
DECLARE_DO_FUN( do_cutpurse                    );
DECLARE_DO_FUN( do_daze                        );
DECLARE_DO_FUN( do_declare                     );
DECLARE_DO_FUN( do_decline                     );
DECLARE_DO_FUN( do_deities                     );
DECLARE_DO_FUN( do_delay                       );
DECLARE_DO_FUN( do_delet                       );
DECLARE_DO_FUN( do_delete                      );
DECLARE_DO_FUN( do_deny                        );
DECLARE_DO_FUN( do_depository                  );
DECLARE_DO_FUN( do_description                 );
DECLARE_DO_FUN( do_deshield                    );
DECLARE_DO_FUN( do_destro                      );
DECLARE_DO_FUN( do_destroy                     );
DECLARE_DO_FUN( do_detrap                      );
DECLARE_DO_FUN( do_devote                      );
DECLARE_DO_FUN( do_diagnose                    );
DECLARE_DO_FUN( do_dig                         );
DECLARE_DO_FUN( do_disarm                      );
DECLARE_DO_FUN( do_disconnect                  );
DECLARE_DO_FUN( do_dislodge                    );
DECLARE_DO_FUN( do_dismiss                     );
DECLARE_DO_FUN( do_dismount                    );
DECLARE_DO_FUN( do_dmesg                       );
DECLARE_DO_FUN( do_dodge                       );
DECLARE_DO_FUN( do_down                        );
DECLARE_DO_FUN( do_drag                        );
DECLARE_DO_FUN( do_draw                        );
DECLARE_DO_FUN( do_drink                       );
DECLARE_DO_FUN( do_drive                       );
DECLARE_DO_FUN( do_drop                        );
DECLARE_DO_FUN( do_duel                        );
DECLARE_DO_FUN( do_dumpgoods                   );
DECLARE_DO_FUN( do_east                        );
DECLARE_DO_FUN( do_eat                         );
DECLARE_DO_FUN( do_echo                        );
DECLARE_DO_FUN( do_elbowbreak                  );
DECLARE_DO_FUN( do_elbowjab                    );
DECLARE_DO_FUN( do_elbowstab                   );
DECLARE_DO_FUN( do_emeru                       );
DECLARE_DO_FUN( do_emote                       );
DECLARE_DO_FUN( do_empty                       );
DECLARE_DO_FUN( do_emptycorpses                );
DECLARE_DO_FUN( do_enter                       );
DECLARE_DO_FUN( do_enhance					   );
DECLARE_DO_FUN( do_entership                   );
DECLARE_DO_FUN( do_equipment                   );
DECLARE_DO_FUN( do_examine                     );
DECLARE_DO_FUN( do_exits                       );
DECLARE_DO_FUN( do_extract                     );
DECLARE_DO_FUN( do_feedmount                   );
DECLARE_DO_FUN( do_fightoutput                 );
DECLARE_DO_FUN( do_fill                        );
DECLARE_DO_FUN( do_findnote                    );
DECLARE_DO_FUN( do_fire                        );
DECLARE_DO_FUN( do_fix                         );
DECLARE_DO_FUN( do_fixchar                     );
DECLARE_DO_FUN( do_fixed                       );
DECLARE_DO_FUN( do_fixgemslots                 );
DECLARE_DO_FUN( do_flee                        );
DECLARE_DO_FUN( do_flevel                      );
DECLARE_DO_FUN( do_flipcoin                    );
DECLARE_DO_FUN( do_foldarea                    );
DECLARE_DO_FUN( do_foldqarea                   );
DECLARE_DO_FUN( do_follow                      );
DECLARE_DO_FUN( do_for                         );
DECLARE_DO_FUN( do_forage                      );
DECLARE_DO_FUN( do_force                       );
DECLARE_DO_FUN( do_forceclose                  );
DECLARE_DO_FUN( do_forecast                    );
DECLARE_DO_FUN( do_forge                       );
DECLARE_DO_FUN( do_forgealter                  );
DECLARE_DO_FUN( do_forget                      );
DECLARE_DO_FUN( do_form_password               );
DECLARE_DO_FUN( do_fprompt                     );
DECLARE_DO_FUN( do_fquit                       );
DECLARE_DO_FUN( do_free_vnums                  );
DECLARE_DO_FUN( do_freerooms                   );
DECLARE_DO_FUN( do_freeze                      );
DECLARE_DO_FUN( do_gag                         );
DECLARE_DO_FUN( do_gamereset                   );
DECLARE_DO_FUN( do_gaso                        );
DECLARE_DO_FUN( do_gathertinder                );
DECLARE_DO_FUN( do_gem                         );
DECLARE_DO_FUN( do_generatename                );
DECLARE_DO_FUN( do_get                         );
DECLARE_DO_FUN( do_getresources                );
DECLARE_DO_FUN( do_gfighting                   );
DECLARE_DO_FUN( do_give                        );
DECLARE_DO_FUN( do_givecrown                   );
DECLARE_DO_FUN( do_giveorders                  );
DECLARE_DO_FUN( do_giveup                      );
DECLARE_DO_FUN( do_glance                      );
DECLARE_DO_FUN( do_goauth                      );
DECLARE_DO_FUN( do_gold                        );
DECLARE_DO_FUN( do_goldgive                    );
DECLARE_DO_FUN( do_goldtake                    );
DECLARE_DO_FUN( do_goto                        );
DECLARE_DO_FUN( do_gouge                       );
DECLARE_DO_FUN( do_gprompt                     );
DECLARE_DO_FUN( do_grab                        );
DECLARE_DO_FUN( do_grantlicense                );
DECLARE_DO_FUN( do_grip                        );
DECLARE_DO_FUN( do_group                       );
DECLARE_DO_FUN( do_grub                        );
DECLARE_DO_FUN( do_gscore                      );
DECLARE_DO_FUN( do_gsocial                     );
DECLARE_DO_FUN( do_gtell                       );
DECLARE_DO_FUN( do_guilds                      );
DECLARE_DO_FUN( do_guildtalk                   );
DECLARE_DO_FUN( do_gwhere                      );
DECLARE_DO_FUN( do_heal                        );
DECLARE_DO_FUN( do_hedit                       );
DECLARE_DO_FUN( do_hell                        );
DECLARE_DO_FUN( do_help                        );
DECLARE_DO_FUN( do_helpcheck                   );
DECLARE_DO_FUN( do_helpweb                     );
DECLARE_DO_FUN( do_hide                        );
DECLARE_DO_FUN( do_hindex                      );
DECLARE_DO_FUN( do_hitall                      );
DECLARE_DO_FUN( do_hl                          );
DECLARE_DO_FUN( do_hlist                       );
DECLARE_DO_FUN( do_holylight                   );
DECLARE_DO_FUN( do_homepage                    );
DECLARE_DO_FUN( do_hset                        );
DECLARE_DO_FUN( do_huntportals                 );
DECLARE_DO_FUN( do_ide                         );
DECLARE_DO_FUN( do_idea                        );
DECLARE_DO_FUN( do_ignore                      );
DECLARE_DO_FUN( do_imbue		       );
DECLARE_DO_FUN( do_imm_morph                   );
DECLARE_DO_FUN( do_imm_unmorph                 );
DECLARE_DO_FUN( do_immreminder                 );
DECLARE_DO_FUN( do_immtalk                     );
DECLARE_DO_FUN( do_induct                      );
DECLARE_DO_FUN( do_installarea                 );
DECLARE_DO_FUN( do_instaroom                   );
DECLARE_DO_FUN( do_instazone                   );
DECLARE_DO_FUN( do_insult                      );
DECLARE_DO_FUN( do_insults                     );
DECLARE_DO_FUN( do_introduce                   );
DECLARE_DO_FUN( do_inventory                   );
DECLARE_DO_FUN( do_invis                       );
DECLARE_DO_FUN( do_ipcompare                   );
DECLARE_DO_FUN( do_jog                         );
DECLARE_DO_FUN( do_joinkingdom                 );
DECLARE_DO_FUN( do_junk                        );
DECLARE_DO_FUN( do_keeperset                   );
DECLARE_DO_FUN( do_keeperstat                  );
DECLARE_DO_FUN( do_keys                        );
DECLARE_DO_FUN( do_khistory                    );
DECLARE_DO_FUN( do_kick_back                   );
DECLARE_DO_FUN( do_kickdirt                    );
DECLARE_DO_FUN( do_kickout                     );
DECLARE_DO_FUN( do_kinduct                     );
DECLARE_DO_FUN( do_kingdomlog                  );
DECLARE_DO_FUN( do_kingdomtalk                 );
DECLARE_DO_FUN( do_kneecrusher                 );
DECLARE_DO_FUN( do_kneestrike                  );
DECLARE_DO_FUN( do_kremove                     );
DECLARE_DO_FUN( do_languages                   );
DECLARE_DO_FUN( do_last                        );
DECLARE_DO_FUN( do_lastname                    );
DECLARE_DO_FUN( do_laws                        );
DECLARE_DO_FUN( do_learn                       );
DECLARE_DO_FUN( do_leave                       );
DECLARE_DO_FUN( do_leavekingdom                );
DECLARE_DO_FUN( do_leaveship                   );
DECLARE_DO_FUN( do_lembecu                     );
DECLARE_DO_FUN( do_light                       );
DECLARE_DO_FUN( do_list                        );
DECLARE_DO_FUN( do_listgroups                  );
DECLARE_DO_FUN( do_listportals                 );
DECLARE_DO_FUN( do_litterbug                   );
DECLARE_DO_FUN( do_loadarea                    );
DECLARE_DO_FUN( do_loadgem                     );
DECLARE_DO_FUN( do_loadquest                   );
DECLARE_DO_FUN( do_loadup                      );
DECLARE_DO_FUN( do_lock                        );
DECLARE_DO_FUN( do_log                         );
DECLARE_DO_FUN( do_logsettings                 );
DECLARE_DO_FUN( do_look                        );
DECLARE_DO_FUN( do_lookaround                  );
DECLARE_DO_FUN( do_loop                        );
DECLARE_DO_FUN( do_lore                        );
DECLARE_DO_FUN( do_low_purge                   );
DECLARE_DO_FUN( do_mailroom                    );
DECLARE_DO_FUN( do_make                        );
DECLARE_DO_FUN( do_make_wilderness_exits       );
DECLARE_DO_FUN( do_make_wilderness_exits2      );
DECLARE_DO_FUN( do_makeboard                   );
DECLARE_DO_FUN( do_makeclan                    );
DECLARE_DO_FUN( do_makecouncil                 );
DECLARE_DO_FUN( do_makedeity                   );
DECLARE_DO_FUN( do_makeguild                   );
DECLARE_DO_FUN( do_makekeeper                  );
DECLARE_DO_FUN( do_makerepair                  );
DECLARE_DO_FUN( do_makeroom                    );
DECLARE_DO_FUN( do_makeshop                    );
DECLARE_DO_FUN( do_makestable                  );
DECLARE_DO_FUN( do_makewizlist                 );
DECLARE_DO_FUN( do_makeworker                  );
DECLARE_DO_FUN( do_manaburst                   );
DECLARE_DO_FUN( do_manashot                    );
DECLARE_DO_FUN( do_manatap                     );
DECLARE_DO_FUN( do_map                         );
DECLARE_DO_FUN( do_mapat                       );
DECLARE_DO_FUN( do_mapedit                     );
DECLARE_DO_FUN( do_mapline                     );
DECLARE_DO_FUN( do_market                      );
DECLARE_DO_FUN( do_markportal                  );
DECLARE_DO_FUN( do_massgoto                    );
DECLARE_DO_FUN( do_massign                     );
DECLARE_DO_FUN( do_mcreate                     );
DECLARE_DO_FUN( do_mdelete                     );
DECLARE_DO_FUN( do_memory                      );
DECLARE_DO_FUN( do_mfind                       );
DECLARE_DO_FUN( do_minfo                       );
DECLARE_DO_FUN( do_minvoke                     );
DECLARE_DO_FUN( do_mistwalk                    );
DECLARE_DO_FUN( do_mixpotion				   );
DECLARE_DO_FUN( do_mlist                       );
DECLARE_DO_FUN( do_moblog                      );
DECLARE_DO_FUN( do_morgue                      );
DECLARE_DO_FUN( do_morphcreate                 );
DECLARE_DO_FUN( do_morphdestroy                );
DECLARE_DO_FUN( do_morphset                    );
DECLARE_DO_FUN( do_morphstat                   );
DECLARE_DO_FUN( do_mortalize                   );
DECLARE_DO_FUN( do_mount                       );
DECLARE_DO_FUN( do_movement                    );
DECLARE_DO_FUN( do_mp_close_passage            );
DECLARE_DO_FUN( do_mp_damage                   );
DECLARE_DO_FUN( do_mp_deposit                  );
DECLARE_DO_FUN( do_mp_fill_in                  );
DECLARE_DO_FUN( do_mp_log                      );
DECLARE_DO_FUN( do_mp_open_passage             );
DECLARE_DO_FUN( do_mp_practice                 );
DECLARE_DO_FUN( do_mp_restore                  );
DECLARE_DO_FUN( do_mp_slay                     );
DECLARE_DO_FUN( do_mp_withdraw                 );
DECLARE_DO_FUN( do_mpadvance                   );
DECLARE_DO_FUN( do_mpapply                     );
DECLARE_DO_FUN( do_mpapplyb                    );
DECLARE_DO_FUN( do_mpasound                    );
DECLARE_DO_FUN( do_mpasupress                  );
DECLARE_DO_FUN( do_mpat                        );
DECLARE_DO_FUN( do_mpbodybag                   );
DECLARE_DO_FUN( do_mpdelay                     );
DECLARE_DO_FUN( do_mpdream                     );
DECLARE_DO_FUN( do_mpecho                      );
DECLARE_DO_FUN( do_mpechoaround                );
DECLARE_DO_FUN( do_mpechoat                    );
DECLARE_DO_FUN( do_mpechozone                  );
DECLARE_DO_FUN( do_mpedit                      );
DECLARE_DO_FUN( do_mpfavor                     );
DECLARE_DO_FUN( do_mpforce                     );
DECLARE_DO_FUN( do_mpgive                      );
DECLARE_DO_FUN( do_mpgoto                      );
DECLARE_DO_FUN( do_mpinvis                     );
DECLARE_DO_FUN( do_mpjunk                      );
DECLARE_DO_FUN( do_mpkill                      );
DECLARE_DO_FUN( do_mpmload                     );
DECLARE_DO_FUN( do_mpmorph                     );
DECLARE_DO_FUN( do_mpmset                      );
DECLARE_DO_FUN( do_mpmusic                     );
DECLARE_DO_FUN( do_mpmusicaround               );
DECLARE_DO_FUN( do_mpmusicat                   );
DECLARE_DO_FUN( do_mpnothing                   );
DECLARE_DO_FUN( do_mpnuisance                  );
DECLARE_DO_FUN( do_mpoload                     );
DECLARE_DO_FUN( do_mposet                      );
DECLARE_DO_FUN( do_mppardon                    );
DECLARE_DO_FUN( do_mppeace                     );
DECLARE_DO_FUN( do_mppkset                     );
DECLARE_DO_FUN( do_mppurge                     );
DECLARE_DO_FUN( do_mpscatter                   );
DECLARE_DO_FUN( do_mpsound                     );
DECLARE_DO_FUN( do_mpsoundaround               );
DECLARE_DO_FUN( do_mpsoundat                   );
DECLARE_DO_FUN( do_mpstat                      );
DECLARE_DO_FUN( do_mptake                      );
DECLARE_DO_FUN( do_mpteach                     );
DECLARE_DO_FUN( do_mptransfer                  );
DECLARE_DO_FUN( do_mpunmorph                   );
DECLARE_DO_FUN( do_mpunnuisance                );
DECLARE_DO_FUN( do_mpvalue                     );
DECLARE_DO_FUN( do_mrange                      );
DECLARE_DO_FUN( do_mset                        );
DECLARE_DO_FUN( do_mstat                       );
DECLARE_DO_FUN( do_muse                        );
DECLARE_DO_FUN( do_music                       );
DECLARE_DO_FUN( do_mwhere                      );
DECLARE_DO_FUN( do_mxp                         );
DECLARE_DO_FUN( do_name                        );
DECLARE_DO_FUN( do_neckchop                    );
DECLARE_DO_FUN( do_neckpinch                   );
DECLARE_DO_FUN( do_neckrupture                 );
DECLARE_DO_FUN( do_nervepinch                  );
DECLARE_DO_FUN( do_nervestrike                 );
DECLARE_DO_FUN( do_newbiechat                  );
DECLARE_DO_FUN( do_newbieset                   );
DECLARE_DO_FUN( do_news                        );
DECLARE_DO_FUN( do_newscore                    );
DECLARE_DO_FUN( do_newzones                    );
DECLARE_DO_FUN( do_niburo                      );
DECLARE_DO_FUN( do_nock                        );
DECLARE_DO_FUN( do_noemote                     );
DECLARE_DO_FUN( do_noresolve                   );
DECLARE_DO_FUN( do_north                       );
DECLARE_DO_FUN( do_northeast                   );
DECLARE_DO_FUN( do_northwest                   );
DECLARE_DO_FUN( do_notell                      );
DECLARE_DO_FUN( do_noteroom                    );
DECLARE_DO_FUN( do_notitle                     );
DECLARE_DO_FUN( do_npcrace                     );
DECLARE_DO_FUN( do_nuisance                    );
DECLARE_DO_FUN( do_oassign                     );
DECLARE_DO_FUN( do_ocreate                     );
DECLARE_DO_FUN( do_odelete                     );
DECLARE_DO_FUN( do_offered                     );
DECLARE_DO_FUN( do_offername                   );
DECLARE_DO_FUN( do_offers                      );
DECLARE_DO_FUN( do_ofind                       );
DECLARE_DO_FUN( do_ogrub                       );
DECLARE_DO_FUN( do_oinvoke                     );
DECLARE_DO_FUN( do_oldscore                    );
DECLARE_DO_FUN( do_olist                       );
DECLARE_DO_FUN( do_opedit                      );
DECLARE_DO_FUN( do_open                        );
//DECLARE_DO_FUN( do_opentourney                 );
DECLARE_DO_FUN( do_opstat                      );
DECLARE_DO_FUN( do_orange                      );
DECLARE_DO_FUN( do_order                       );
DECLARE_DO_FUN( do_orders                      );
DECLARE_DO_FUN( do_ordertalk                   );
DECLARE_DO_FUN( do_oscatter                    );
DECLARE_DO_FUN( do_oset                        );
DECLARE_DO_FUN( do_ostat                       );
DECLARE_DO_FUN( do_ot                          );
DECLARE_DO_FUN( do_outcast                     );
DECLARE_DO_FUN( do_owhere                      );
DECLARE_DO_FUN( do_pager                       );
DECLARE_DO_FUN( do_pardon                      );
DECLARE_DO_FUN( do_parry                       );
DECLARE_DO_FUN( do_password                    );
DECLARE_DO_FUN( do_pcrename                    );
DECLARE_DO_FUN( do_pcshops                     );
DECLARE_DO_FUN( do_peace                       );
DECLARE_DO_FUN( do_peasant                     );
DECLARE_DO_FUN( do_perfectshot                 );
DECLARE_DO_FUN( do_pick                        );
DECLARE_DO_FUN( do_piggyback                   );
DECLARE_DO_FUN( do_pincer                      );
DECLARE_DO_FUN( do_pkillcheck                  );
DECLARE_DO_FUN( do_placemob                    );
DECLARE_DO_FUN( do_placeobj                    );
DECLARE_DO_FUN( do_placetrainer                );
DECLARE_DO_FUN( do_plist                       );
DECLARE_DO_FUN( do_poison_weapon               );
DECLARE_DO_FUN( do_portal                      );
DECLARE_DO_FUN( do_pose                        );
DECLARE_DO_FUN( do_powerslice                  );
DECLARE_DO_FUN( do_pretitle                    );
DECLARE_DO_FUN( do_project                     );
DECLARE_DO_FUN( do_prompt                      );
DECLARE_DO_FUN( do_pset                        );
DECLARE_DO_FUN( do_pstat                       );
DECLARE_DO_FUN( do_pstatus                     );
DECLARE_DO_FUN( do_pull                        );
DECLARE_DO_FUN( do_purge                       );
DECLARE_DO_FUN( do_push                        );
DECLARE_DO_FUN( do_put                         );
DECLARE_DO_FUN( do_qmob                        );
DECLARE_DO_FUN( do_qobj                        );
DECLARE_DO_FUN( do_qpset                       );
DECLARE_DO_FUN( do_qpstat                      );
DECLARE_DO_FUN( do_quaff                       );
DECLARE_DO_FUN( do_qui                         );
DECLARE_DO_FUN( do_quickcombo                  );
DECLARE_DO_FUN( do_quit                        );
DECLARE_DO_FUN( do_racetalk                    );
DECLARE_DO_FUN( do_rank                        );
DECLARE_DO_FUN( do_rankings                    );
DECLARE_DO_FUN( do_rap                         );
DECLARE_DO_FUN( do_rassign                     );
DECLARE_DO_FUN( do_rat                         );
DECLARE_DO_FUN( do_rdelete                     );
DECLARE_DO_FUN( do_reboo                       );
DECLARE_DO_FUN( do_reboot                      );
DECLARE_DO_FUN( do_recall                      );
DECLARE_DO_FUN( do_recho                       );
DECLARE_DO_FUN( do_recite                      );
DECLARE_DO_FUN( do_redit                       );
DECLARE_DO_FUN( do_regoto                      );
DECLARE_DO_FUN( do_remains                     );
DECLARE_DO_FUN( do_remove                      );
DECLARE_DO_FUN( do_removekingdom               );
DECLARE_DO_FUN( do_removetown                  );
DECLARE_DO_FUN( do_rent                        );
DECLARE_DO_FUN( do_repair                      );
DECLARE_DO_FUN( do_repairset                   );
DECLARE_DO_FUN( do_repairshops                 );
DECLARE_DO_FUN( do_repairstat                  );
DECLARE_DO_FUN( do_repairwall                  );
DECLARE_DO_FUN( do_repeat                      );
DECLARE_DO_FUN( do_reply                       );
DECLARE_DO_FUN( do_report                      );
DECLARE_DO_FUN( do_rescue                      );
DECLARE_DO_FUN( do_reserve                     );
DECLARE_DO_FUN( do_reset                       );
DECLARE_DO_FUN( do_resetkeeper                 );
DECLARE_DO_FUN( do_rest                        );
DECLARE_DO_FUN( do_restore                     );
DECLARE_DO_FUN( do_restorelimbs                );
DECLARE_DO_FUN( do_restoretime                 );
DECLARE_DO_FUN( do_restrict                    );
DECLARE_DO_FUN( do_resurrection                );
DECLARE_DO_FUN( do_retell                      );
DECLARE_DO_FUN( do_retire                      );
DECLARE_DO_FUN( do_retran                      );
DECLARE_DO_FUN( do_return                      );
DECLARE_DO_FUN( do_revert                      );
DECLARE_DO_FUN( do_reward                      );
DECLARE_DO_FUN( do_rgrub                       );
DECLARE_DO_FUN( do_ribpuncture                 );
DECLARE_DO_FUN( do_rip                         );
DECLARE_DO_FUN( do_rlist                       );
DECLARE_DO_FUN( do_roar                        );
DECLARE_DO_FUN( do_roll                        );
DECLARE_DO_FUN( do_roomstat                    );
DECLARE_DO_FUN( do_roundhouse                  );
DECLARE_DO_FUN( do_rpedit                      );
DECLARE_DO_FUN( do_rpstat                      );
DECLARE_DO_FUN( do_rreset                      );
DECLARE_DO_FUN( do_rset                        );
DECLARE_DO_FUN( do_rstat                       );
DECLARE_DO_FUN( do_rub                         );
DECLARE_DO_FUN( do_run                         );
DECLARE_DO_FUN( do_sacrifice                   );
DECLARE_DO_FUN( do_save                        );
DECLARE_DO_FUN( do_savearea                    );
DECLARE_DO_FUN( do_say                         );
DECLARE_DO_FUN( do_say_to_char                 );
DECLARE_DO_FUN( do_sbook                       );
DECLARE_DO_FUN( do_scan                        );
DECLARE_DO_FUN( do_scatter                     );
DECLARE_DO_FUN( do_schedule                    );
DECLARE_DO_FUN( do_score                       );
DECLARE_DO_FUN( do_scribe                      );
DECLARE_DO_FUN( do_search                      );
DECLARE_DO_FUN( do_sedit                       );
DECLARE_DO_FUN( do_seeorders                   );
DECLARE_DO_FUN( do_sell                        );
DECLARE_DO_FUN( do_sendmail                    );
DECLARE_DO_FUN( do_set_boot_time               );
DECLARE_DO_FUN( do_setcaste                    );
DECLARE_DO_FUN( do_setclan                     );
DECLARE_DO_FUN( do_setcouncil                  );
DECLARE_DO_FUN( do_setdeity                    );
DECLARE_DO_FUN( do_setfree                     );
DECLARE_DO_FUN( do_setgambler                  );
DECLARE_DO_FUN( do_setgem		       );
DECLARE_DO_FUN( do_setjob                      );
DECLARE_DO_FUN( do_setkingdom                  );
DECLARE_DO_FUN( do_setrace                     );
DECLARE_DO_FUN( do_settoadvance                );
DECLARE_DO_FUN( do_setweather                  );
DECLARE_DO_FUN( do_setwilderness               );
DECLARE_DO_FUN( do_sheath                      );
DECLARE_DO_FUN( do_shells                      );
DECLARE_DO_FUN( do_ships                       );
DECLARE_DO_FUN( do_shops                       );
DECLARE_DO_FUN( do_shopset                     );
DECLARE_DO_FUN( do_shopstat                    );
DECLARE_DO_FUN( do_shout                       );
DECLARE_DO_FUN( do_shove                       );
DECLARE_DO_FUN( do_show                        );
DECLARE_DO_FUN( do_showascii                   );
DECLARE_DO_FUN( do_showclan                    );
DECLARE_DO_FUN( do_showcontrol                 );
DECLARE_DO_FUN( do_showcouncil                 );
DECLARE_DO_FUN( do_showdeity                   );
DECLARE_DO_FUN( do_showentrances               );
DECLARE_DO_FUN( do_showgambler                 );
DECLARE_DO_FUN( do_showhouse                   );
DECLARE_DO_FUN( do_showkingdoms                );
DECLARE_DO_FUN( do_showlayers                  );
DECLARE_DO_FUN( do_showlist                    );
DECLARE_DO_FUN( do_showpic                     );
DECLARE_DO_FUN( do_showrace                    );
DECLARE_DO_FUN( do_showresources               );
DECLARE_DO_FUN( do_showweather                 );
DECLARE_DO_FUN( do_shutdow                     );
DECLARE_DO_FUN( do_shutdown                    );
DECLARE_DO_FUN( do_sidekick                    );
DECLARE_DO_FUN( do_silence                     );
DECLARE_DO_FUN( do_sing                        );
DECLARE_DO_FUN( do_sit                         );
DECLARE_DO_FUN( do_skills                      );
DECLARE_DO_FUN( do_skin                        );
DECLARE_DO_FUN( do_sla                         );
DECLARE_DO_FUN( do_slay                        );
DECLARE_DO_FUN( do_sleep                       );
DECLARE_DO_FUN( do_slice                       );
DECLARE_DO_FUN( do_slist                       );
DECLARE_DO_FUN( do_slookup                     );
DECLARE_DO_FUN( do_smoke                       );
DECLARE_DO_FUN( do_sneak                       );
DECLARE_DO_FUN( do_snoop                       );
DECLARE_DO_FUN( do_sober                       );
DECLARE_DO_FUN( do_socials                     );
DECLARE_DO_FUN( do_south                       );
DECLARE_DO_FUN( do_southeast                   );
DECLARE_DO_FUN( do_southwest                   );
DECLARE_DO_FUN( do_spar                        );
DECLARE_DO_FUN( do_speak                       );
DECLARE_DO_FUN( do_spear                       );
DECLARE_DO_FUN( do_speed                       );
DECLARE_DO_FUN( do_spinkick                    );
DECLARE_DO_FUN( do_split                       );
DECLARE_DO_FUN( do_sset                        );
DECLARE_DO_FUN( do_sslist                      );
DECLARE_DO_FUN( do_stable                      );
DECLARE_DO_FUN( do_stack                       );
DECLARE_DO_FUN( do_stalk                       );
DECLARE_DO_FUN( do_stand                       );
DECLARE_DO_FUN( do_startarena                  );
DECLARE_DO_FUN( do_startfire                   );
DECLARE_DO_FUN( do_startkingdom                );
DECLARE_DO_FUN( do_startroom                   );
DECLARE_DO_FUN( do_starttourney                );
DECLARE_DO_FUN( do_stat                        );
DECLARE_DO_FUN( do_statreport                  );
DECLARE_DO_FUN( do_status                      );
DECLARE_DO_FUN( do_steal                       );
DECLARE_DO_FUN( do_steership                   );
DECLARE_DO_FUN( do_stepback                    );
DECLARE_DO_FUN( do_strew                       );
DECLARE_DO_FUN( do_strip                       );
DECLARE_DO_FUN( do_study                       );
DECLARE_DO_FUN( do_stun                        );
DECLARE_DO_FUN( do_style                       );
DECLARE_DO_FUN( do_supplicate                  );
DECLARE_DO_FUN( do_survey                      );
DECLARE_DO_FUN( do_switch                      );
DECLARE_DO_FUN( do_talent                      );
DECLARE_DO_FUN( do_talkquest                   );
DECLARE_DO_FUN( do_tamp                        );
DECLARE_DO_FUN( do_target                      );
DECLARE_DO_FUN( do_tcreate                     );
DECLARE_DO_FUN( do_tdelete                     );
DECLARE_DO_FUN( do_tell                        );
DECLARE_DO_FUN( do_terraform                   );
DECLARE_DO_FUN( do_think                       );
DECLARE_DO_FUN( do_throw                       );
DECLARE_DO_FUN( do_time                        );
DECLARE_DO_FUN( do_timecmd                     );
DECLARE_DO_FUN( do_timmuru                     );
DECLARE_DO_FUN( do_tinduct                     );
DECLARE_DO_FUN( do_title                       );
DECLARE_DO_FUN( do_tkickout                    );
DECLARE_DO_FUN( do_tone                        );
DECLARE_DO_FUN( do_tornadokick                 );
DECLARE_DO_FUN( do_toss                        );
DECLARE_DO_FUN( do_track                       );
DECLARE_DO_FUN( do_tradegoods                  );
DECLARE_DO_FUN( do_traderoutes                 );
DECLARE_DO_FUN( do_training                    );
DECLARE_DO_FUN( do_transfer                    );
DECLARE_DO_FUN( do_trap                        );
DECLARE_DO_FUN( do_trust                       );
DECLARE_DO_FUN( do_tset                        );
DECLARE_DO_FUN( do_tstat                       );
DECLARE_DO_FUN( do_tumble                      );
DECLARE_DO_FUN( do_typo                        );
DECLARE_DO_FUN( do_unfoldarea                  );
DECLARE_DO_FUN( do_unhell                      );
DECLARE_DO_FUN( do_unloadqarea                 );
DECLARE_DO_FUN( do_unlock                      );
DECLARE_DO_FUN( do_unnuisance                  );
DECLARE_DO_FUN( do_unsilence                   );
DECLARE_DO_FUN( do_up                          );
DECLARE_DO_FUN( do_updatearea                  );
DECLARE_DO_FUN( do_updateskills                );
DECLARE_DO_FUN( do_uselicense                  );
DECLARE_DO_FUN( do_users                       );
DECLARE_DO_FUN( do_value                       );
DECLARE_DO_FUN( do_vassign                     );
DECLARE_DO_FUN( do_version                     );
DECLARE_DO_FUN( do_victories                   );
DECLARE_DO_FUN( do_viewmount                   );
DECLARE_DO_FUN( do_viewskills                  );
DECLARE_DO_FUN( do_visible                     );
DECLARE_DO_FUN( do_vnums                       );
DECLARE_DO_FUN( do_vsearch                     );
DECLARE_DO_FUN( do_wake                        );
DECLARE_DO_FUN( do_warn                        );
DECLARE_DO_FUN( do_wartalk                     );
DECLARE_DO_FUN( do_watch                       );
DECLARE_DO_FUN( do_wblock                      );
DECLARE_DO_FUN( do_weaponbreak                 );
DECLARE_DO_FUN( do_wear                        );
DECLARE_DO_FUN( do_weather                     );
DECLARE_DO_FUN( do_webstats                    );
DECLARE_DO_FUN( do_west                        );
DECLARE_DO_FUN( do_where                       );
DECLARE_DO_FUN( do_whisper                     );
DECLARE_DO_FUN( do_who                         );
DECLARE_DO_FUN( do_whois                       );
DECLARE_DO_FUN( do_whonumber                   );
DECLARE_DO_FUN( do_wimpy                       );
DECLARE_DO_FUN( do_wizhelp                     );
DECLARE_DO_FUN( do_wizlist                     );
DECLARE_DO_FUN( do_wizlock                     );
DECLARE_DO_FUN( do_wizzap                      );
DECLARE_DO_FUN( do_worth                       );
DECLARE_DO_FUN( do_yell                        );
DECLARE_DO_FUN( do_zap                         );
DECLARE_DO_FUN( do_zones                       );
/*//T6*/

/*
 * Spell functions.
 * Defined in magic.c.
 */
DECLARE_SPELL_FUN(spell_null);
DECLARE_SPELL_FUN(spell_notfound);
DECLARE_SPELL_FUN(spell_animate_dead);
DECLARE_SPELL_FUN(spell_astral_walk);
DECLARE_SPELL_FUN(spell_bless_weapon);
DECLARE_SPELL_FUN(spell_blindness);
DECLARE_SPELL_FUN(spell_cause_critical);
DECLARE_SPELL_FUN(spell_restore_limb);
DECLARE_SPELL_FUN(spell_revitalize_limb);
DECLARE_SPELL_FUN(spell_cause_light);
DECLARE_SPELL_FUN(spell_cause_serious);
DECLARE_SPELL_FUN(spell_charm_person);
DECLARE_SPELL_FUN(spell_create_food);
DECLARE_SPELL_FUN(spell_create_water);
DECLARE_SPELL_FUN(spell_cure_blindness);
DECLARE_SPELL_FUN(spell_cure_poison);
DECLARE_SPELL_FUN(spell_curse);
DECLARE_SPELL_FUN(spell_detect_poison);
DECLARE_SPELL_FUN(spell_dispel_magic);
DECLARE_SPELL_FUN(spell_disenchant_weapon);
DECLARE_SPELL_FUN(spell_earthquake);
DECLARE_SPELL_FUN(spell_enchant_weapon);
DECLARE_SPELL_FUN(spell_faerie_fire);
DECLARE_SPELL_FUN(spell_faerie_fog);
DECLARE_SPELL_FUN(spell_farsight);
DECLARE_SPELL_FUN(spell_knock);
DECLARE_SPELL_FUN(spell_harm);
DECLARE_SPELL_FUN(spell_holy_food);
DECLARE_SPELL_FUN(spell_identify);
DECLARE_SPELL_FUN(spell_invis);
DECLARE_SPELL_FUN(spell_knowenemy);
DECLARE_SPELL_FUN(spell_locate_object);
DECLARE_SPELL_FUN(spell_pass_door);
DECLARE_SPELL_FUN(spell_poison);
DECLARE_SPELL_FUN(spell_polymorph);
DECLARE_SPELL_FUN(spell_possess);
DECLARE_SPELL_FUN(spell_recharge);
DECLARE_SPELL_FUN(spell_remove_curse);
DECLARE_SPELL_FUN(spell_remove_invis);
DECLARE_SPELL_FUN(spell_remove_trap);
DECLARE_SPELL_FUN(spell_sleep);
DECLARE_SPELL_FUN(spell_smaug);
DECLARE_SPELL_FUN(spell_summon);
DECLARE_SPELL_FUN(spell_wizard_eye);
DECLARE_SPELL_FUN(spell_weaken);
DECLARE_SPELL_FUN(spell_word_of_recall);
DECLARE_SPELL_FUN(spell_acid_breath);
DECLARE_SPELL_FUN(spell_fire_breath);
DECLARE_SPELL_FUN(spell_frost_breath);
DECLARE_SPELL_FUN(spell_gas_breath);
DECLARE_SPELL_FUN(spell_lightning_breath);
DECLARE_SPELL_FUN(spell_portal);
DECLARE_SPELL_FUN(spell_bethsaidean_touch);
DECLARE_SPELL_FUN(spell_expurgation);
DECLARE_SPELL_FUN(spell_sacral_divinity);
DECLARE_SPELL_FUN(spell_eye_of_god);
DECLARE_SPELL_FUN(spell_resurrection);
DECLARE_SPELL_FUN(spell_summon_corpse);
DECLARE_SPELL_FUN(spell_greater_resurrection);
DECLARE_SPELL_FUN(spell_lesser_resurrection);
DECLARE_SPELL_FUN(spell_web);
DECLARE_SPELL_FUN(spell_snare);
DECLARE_SPELL_FUN(spell_extradimensional_portal);
DECLARE_SPELL_FUN(spell_revitalize_spirit);
DECLARE_SPELL_FUN(spell_holy_cleansing);

/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(_AIX)
char *crypt args((const char *key, const char *salt));
#endif

#if	defined(apollo)
int atoi args((const char *string));
void *calloc args((unsigned nelem, size_t size));
char *crypt args((const char *key, const char *salt));
#endif

#if	defined(hpux)
char *crypt args((const char *key, const char *salt));
#endif

#if	defined(interactive)
#endif

#if defined(CYGWIN)
char *crypt args((const char *key, const char *salt));
#endif

#if	defined(linux)
char *crypt args((const char *key, const char *salt));
#endif

#if	defined(MIPS_OS)
char *crypt args((const char *key, const char *salt));
#endif

#if	defined(NeXT)
char *crypt args((const char *key, const char *salt));
#endif

#if	defined(sequent)
char *crypt args((const char *key, const char *salt));
int fclose args((FILE * stream));
int fprintf args((FILE * stream, const char *format, ...));
int fread args((void *ptr, int size, int n, FILE * stream));
int fseek args((FILE * stream, long offset, int ptrname));
void perror args((const char *s));
int ungetc args((int c, FILE * stream));
#endif

#if	defined(sun)
char *crypt args((const char *key, const char *salt));
int fclose args((FILE * stream));
int fprintf args((FILE * stream, const char *format, ...));

#if 	defined(SYSV)
size_t fread args((void *ptr, size_t size, size_t n, FILE * stream));
#else
int fread args((void *ptr, int size, int n, FILE * stream));
#endif
int fseek args((FILE * stream, long offset, int ptrname));
void perror args((const char *s));
int ungetc args((int c, FILE * stream));
#endif

#if	defined(ultrix)
char *crypt args((const char *key, const char *salt));
#endif

/*
 * The crypt(3) function is not available on some operating systems.
 * In particular, the U.S. Government prohibits its export from the
 * United States to foreign countries.
 *
 * Turn on NOCRYPT to keep passwords in plain text.
 */
#if	defined(NOCRYPT)
#define crypt(s1, s2)	(s1)
#endif



/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/" /* Player files   */
#define RESET_DIR   "../reset/" // Player Reset Files
#define LNAME_DIR       "../lname/" /* Player Lastname/Family Files */
#define ACCOUNT_DIR     "../account/" //Account Files
#define WEBDIR_DIR    "../../public_html/"  //Webpage, if you want to publish a file, well dump it here :-)
#define PLAYERSTAT_DIR    WEBDIR_DIR "toc/playerstat/"

#define SKILLOUT_DIR  WEBDIR_DIR "skillout/" //Skill output to the web
#define CASTE_DIR     "../caste/" // Caste related info
#define PPAGE_DIR       "../../../public_html/" /*  File who statement    */
#define KINGDOM_DIR     "../kingdom/" /* Kingdom files                */
#define BACKUP_DIR	"../backup/" /* Backup Player files  */
#define GOD_DIR		"../gods/" /* God Info Dir   */
#define BOARD_DIR	"../boards/" /* Board data dir  */
#define CLAN_DIR	"../clans/" /* Clan data dir  */
#define COUNCIL_DIR  	"../councils/" /* Council data dir  */
#define GUILD_DIR       "../guilds/" /* Guild data dir               */
#define DEITY_DIR	"../deity/" /* Deity data dir  */
#define NAMESET_DIR "../nameset/" /* Directory for name building files for quests */
#define BUILD_DIR       "../building/" /* Online building save dir     */
#define SYSTEM_DIR	"../system/" /* Main system files  */
#define PROG_DIR	"mudprogs/" /* MUDProg files  */
#define CORPSE_DIR	"../corpses/" /* Corpses   */
#ifdef WIN32
#define NULL_FILE	"nul" /* To reserve one stream        */
#else
#define NULL_FILE	"/dev/null" /* To reserve one stream        */
#endif

#define	CLASS_DIR	"../classes/" /* Classes   */
#define WATCH_DIR	"../watch/" /* Imm watch files --Gorog      */
/*
 * The watch directory contains a maximum of one file for each immortal
 * that contains output from "player watches". The name of each file
 * in this directory is the name of the immortal who requested the watch
 */

#define TOCSTAT_FILE       WEBDIR_DIR "toc/modules/GameStats/pntemplates/latest.htm"
#define AREA_LIST	        "area.lst" /* List of areas  */
#define WATCH_LIST              "watch.lst" /* List of watches              */
#define BAN_LIST                "ban.lst" /* List of bans                 */
#define RESERVED_LIST	        "reserved.lst" /* List of reserved names */
#define CLAN_LIST	        "clan.lst" /* List of clans  */
#define COUNCIL_LIST	        "council.lst" /* List of councils  */
#define GUILD_LIST              "guild.lst" /* List of guilds               */
#define GOD_LIST	        "gods.lst" /* List of gods   */
#define TRAP_FILE           "trap.lst" // List of traps, area independent now
#define SHIP_FILE               SYSTEM_DIR "ship.dat" //Ship files
#define BDESCRIPTION_LIST       SYSTEM_DIR "bdesc.lst" //List of battle descriptions for every weapon
#define EMAIL_FILE              SYSTEM_DIR "email.dat" //Email file for sending out email
#define CHISTORY_FILE           SYSTEM_DIR "chistory.dat" //Channel history file
#define MARKET_FILE             SYSTEM_DIR "market.dat" //Market data file
#define DEITY_LIST	        "deity.lst" /* List of deities  */
#define	CLASS_LIST	        "class.lst" /* List of classes  */
#define CONQUER_FILE            "conquer.lst" //Conquer list
#define	RACE_LIST	        "race.lst" /* List of races  */
#define KINGDOM_LIST            "kingdom.lst" /* List of kingdoms             */
#define BIN_LIST                MAP_DIR "bins.lst" /* Resource Bins for Kingdoms */
#define TRAINER_LIST            "trainer.lst" /* List of trainers */
#define PORTAL_FILE             "portals.lst" /* List of portals */
#define TRADE_FILE              "trade.lst" // List of trades in process
#define GEM_LIST		"gem.lst" /* List of gems */
#define BOX_LIST		"box.lst" /* List of boxes */
#define MORPH_FILE      "morph.dat" /* For morph data */
#define BOARD_FILE	"boards.txt" /* For bulletin boards  */
#define SHUTDOWN_FILE	"shutdown.txt" /* For 'shutdown'  */
#define IMM_HOST_FILE   SYSTEM_DIR "immortal.host" /* For stoping hackers */
#define ACCOUNT_LOG     SYSTEM_DIR "accounts.log" //Logs Account Creation/Usage
#define LAST_LIST       SYSTEM_DIR "last.lst" //last list
#define CHANGES_LIST    WEBDIR_DIR "changes.txt" //changes list
#define CCHANGES_LIST   WEBDIR_DIR "cchanges.txt" //file the new change list is written to when a post is done
#define LAST_TEMP_LIST  SYSTEM_DIR "ltemp.lst" //temp file for the last list so the data can be copyover over
#define KCHEST_FILE     "kchest.dat" //Kingdom Chest File
#define KINGDOM_MLOG_FILE   KINGDOM_DIR "kmlog.txt"
#define MILIST_FILE     KINGDOM_DIR "milist.dat" //Military mob listing file :-)
#define EXTRACTION_FILE KINGDOM_DIR "extract.dat" //Extraction mob listing file :-)
#define BUYK_FILE       KINGDOM_DIR "buykingdom.dat" //Items you can buy in a kingdom
#define TRAINING_LIST   KINGDOM_DIR "training.dat" //Training List :-)
#define RIPSCREEN_FILE	SYSTEM_DIR "mudrip.rip"
#define RIPTITLE_FILE	SYSTEM_DIR "mudtitle.rip"
#define ANSITITLE_FILE	SYSTEM_DIR "mudtitle.ans"
#define ASCTITLE_FILE	SYSTEM_DIR "mudtitle.asc"
#define AUTHLIST_FILE   SYSTEM_DIR "authlist.dat"
#define NPCRACE_FILE    RACEDIR "npcrace.lst"
#define WEBHELP_FILE    WEBDIR_DIR "webhelp.txt"
#define SKILLOUT_FILE   SKILLOUT_DIR "skillout.htm"
#define BARENA_FILE     SYSTEM_DIR "barena.dat" /* Battle ARena data, replaces hall of fame */
#define HALL_FAME_FILE  SYSTEM_DIR "halloffame.lst" /* Arena hall of fame */
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt" /* Boot up error file  */
//#define BUG_FILE	SYSTEM_DIR "bugs.txt"
#define PBUG_FILE	SYSTEM_DIR "pbugs.txt" /* For 'bug' command   */
#define IDEA_FILE	SYSTEM_DIR "ideas.txt" /* For 'idea'   */
#define TYPO_FILE	SYSTEM_DIR "typos.txt" /* For 'typo'   */
#define FIXED_FILE	SYSTEM_DIR "fixed.txt" /* For 'fixed' command */
#define LOG_FILE	SYSTEM_DIR "log.txt" /* For talking in logged rooms */
#define MOBLOG_FILE	SYSTEM_DIR "moblog.txt" /* For mplog messages  */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST" /* Wizlist   */
#define WHO_FILE	SYSTEM_DIR "WHO" /* Who output file  */
#define WEBWHO_FILE	SYSTEM_DIR "WEBWHO" /* WWW Who output file */
#define REQUEST_PIPE	SYSTEM_DIR "REQUESTS" /* Request FIFO  */
#define SKILL_FILE	SYSTEM_DIR "skills.dat" /* Skill table  */
#define HERB_FILE	SYSTEM_DIR "herbs.dat" /* Herb table   */
#define TONGUE_FILE	SYSTEM_DIR "tongues.dat" /* Tongue tables  */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat" /* Socials   */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat" /* Commands   */
#define USAGE_FILE	SYSTEM_DIR "usage.txt" /* How many people are on 
                                             every half hour - trying to
                                             determine best reboot time */
#define ECONOMY_FILE	SYSTEM_DIR "economy.txt" /* Gold looted, value of
                                                 used potions/pills  */
#define FORGE_LIST      "forge.lst" /* List of forge items */
#define WBLOCK_FILE     MAP_DIR "wblock.dat" //Wilderness Mob Blocks File 
#define SLAB_LIST             "slab.lst" /* List of Slab items */
#define PROJECTS_FILE	SYSTEM_DIR "projects.txt" /* For projects  */
#define PLANE_FILE	SYSTEM_DIR "planes.dat" /* For planes   */
#define COPYOVER_FILE  SYSTEM_DIR "copyover.dat" /* for warm reboots */
#define PIGGYBACK_FILE  SYSTEM_DIR "piggyback.dat" /* for warm reboots recording riding status */
#define QMOB_FILE      SYSTEM_DIR "qmob.dat" //Quest mob Data, lots of it
#define QOBJ_FILE      SYSTEM_DIR "qobj.dat" //Quest obj Data
#define QUEST_FILE     SYSTEM_DIR "quest.dat" //Saved data from a copyover
#define QUEST_MFILE    SYSTEM_DIR "questmob.dat" //Saved mobs from a copyover
#define QUEST_OFILE    SYSTEM_DIR "questobj.dat" //Saved objs from a copyover
#define QUEST_LAREA    "qarea.lst" //Areas that need to be loaded up
#define EXE_FILE       "../src/fear" /* executable path */
#define CLASSDIR	"../classes/"
#define RACEDIR 	"../races/"
#define HELP_FILE "help.txt" /* For undefined helps */

/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define BD	BOARD_DATA
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define	CO	COUNCIL_DATA
#define DE	DEITY_DATA
#define SK	SKILLTYPE


// Forging shit, too lazy to put in right file --Xerves
void add_obj_timer args((OBJ_DATA * obj, sh_int type, sh_int count, DO_FUN * fun, int value));
TIMER *get_obj_timeptr args((OBJ_DATA * obj, sh_int type));
sh_int get_obj_timer args((OBJ_DATA * obj, sh_int type));
void extract_obj_timer args((OBJ_DATA * obj, TIMER * timer));
void remove_obj_timer args((OBJ_DATA * obj, sh_int type));
void load_forge_data args((void));
void save_forge_data args((void));
void load_slab_data args((void));
void save_slab_data args((void));

/* forge.c */
void ore_alter args((CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * slab));
void alter_forge_obj args((CHAR_DATA * ch, OBJ_DATA *fobj, OBJ_DATA *first_obj, SLAB_DATA *slab));

/* editor.c cronel new editor */
/* All Cronel Stuff, wow nice list */
#define start_editing( ch, data ) \
	start_editing_nolimit( ch, data, MAX_STRING_LENGTH )
void start_editing_nolimit args((CHAR_DATA * ch, char *data, sh_int max_size));
void stop_editing args((CHAR_DATA * ch));
void edit_buffer args((CHAR_DATA * ch, char *argument));
char *copy_buffer args((CHAR_DATA * ch));
void set_editor_desc args((CHAR_DATA * ch, char *desc));
void editor_desc_printf args((CHAR_DATA * ch, char *desc_fmt, ...));
void talk_info args((sh_int AT_COLOR, char *argument)); /* Rantic's info channel */
void char_quit args((CHAR_DATA * ch, bool broadcast));

/* raferquest.c */
void load_quest_contents args((void));
void save_quest_data args((void));
void load_quest_data args((void));
void disposequestarea args((AREA_DATA *tarea, int difficulty));
void load_qobj_data args((void));
void load_qmob_data args((void));

/* caste.c */
void remove_all_towns args((int kingdom));
int get_kingdom_units args((int tpid));
int get_banksize args((int size));
bool is_valid_movement args((int *px, int *py, char *arg, CHAR_DATA *ch));
void process_movement_value args((CHAR_DATA *ch, int dir));
int get_truedir args((char *txt));
bool proper_resources_mobs_town args((TOWN_DATA *town, BUYKMOB_DATA *kmob));
void remove_resources_mobs_town args((TOWN_DATA *town, BUYKMOB_DATA * kmob));
void remove_town args((TOWN_DATA *town, int type));
int get_maxunits args((int size));
int klog_linecount args((char *logfile));
int search_logname args((char *logname));
char *return_logname args((int flag));
TOWN_DATA *get_town_tpid args((int kingdom, int pid));
int get_current_hold args((TOWN_DATA *town));
void update_master_stat args((CHAR_DATA *ch, int x, int y));
bool is_made_room args((int x, int y, int map, TOWN_DATA *town));
bool in_town_range args((TOWN_DATA *town, int x, int y, int map));
bool build_onmap args((int sector));
TOWN_DATA *get_town args((char *townname));
void remove_kingdom args((int kingdom));
char *get_resources_traded args((TRADE_DATA *trade, int type));
void write_kingdom_logfile args((int ht, char *buf, int flag));
void resetkeeper args((AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, bool dodoors));
bool can_kmodify args((CHAR_DATA * ch, ROOM_INDEX_DATA * room));
int get_resourcetype args((int sector, int type));
int alt_dir args((CHAR_DATA * ch, int x, int y, int dir));
void layroad args((CHAR_DATA * ch, char *argument));
void digstone args((CHAR_DATA * ch, char *argument));
void torchland args((CHAR_DATA * ch, char *argument));
void cutpath args((CHAR_DATA * ch, char *argument));
void change_plains args((CHAR_DATA * ch, char *argument));
void stopfire args((CHAR_DATA * ch, char *argument));
void planttree args((CHAR_DATA * ch, char *argument));
void plantgrass args((CHAR_DATA * ch, char *argument));
void plantgrain args((CHAR_DATA * ch, char *argument));
void plantcorn args((CHAR_DATA * ch, char *argument));
void scan_players args((void));
void add_player_list args((CHAR_DATA * ch, int type));
void remove_player_list args((CHAR_DATA * ch, int type));

/* act_comm.c */
void read_pfile2 args((char *dirname, char *filename));
int get_heightweight_percent args((int value));
bool circle_follow args((CHAR_DATA * ch, CHAR_DATA * victim));
void add_follower args((CHAR_DATA * ch, CHAR_DATA * master));
void stop_follower args((CHAR_DATA * ch));
void die_follower args((CHAR_DATA * ch));
bool is_same_group args((CHAR_DATA * ach, CHAR_DATA * bch));
void send_rip_screen args((CHAR_DATA * ch));
void send_rip_title args((CHAR_DATA * ch));
void send_ansi_title args((CHAR_DATA * ch));
void send_ascii_title args((CHAR_DATA * ch));
void to_channel args((const char *argument, int channel, const char *verb, sh_int level));
void talk_auction args((char *argument));
int knows_language args((CHAR_DATA * ch, int language, CHAR_DATA * cch));
bool can_learn_lang args((CHAR_DATA * ch, int language));
int countlangs args((int languages));
char *translate args((int percent, const char *in, const char *name));
char *obj_short args((OBJ_DATA * obj));
void init_profanity_checker args((void));

/* act_info.c */
int get_talent_increase args((CHAR_DATA *ch, int stat));
char *add_wspace args((int length, int maxlength));
OBJ_DATA *get_wear_hidden_cloak args((CHAR_DATA *ch));
OBJ_DATA *get_wear_cloak args((CHAR_DATA *ch));
char *show_pers_output args((CHAR_DATA *ch, CHAR_DATA *looker, int type, int kingdom));
bool check_blind args((CHAR_DATA * ch));

//Time functions
char *getgametime args((void));
char *getdayofweek args((void));
char *getampm args((int hour));
int get12hour args((int hour));
char *getmonth args((int day));
int getdayofmonth args((int day));
int getyear args((void));
int get_value_month args((void));
int getday args((void));
int gethour args((void));
int getmin args((void));

//End time Functions
char *punct args((int foo)); /* Punctuate for numbers */
int get_door args((char *arg));
bool is_player_kingdom args((int ht));
char *format_obj_to_char args((OBJ_DATA * obj, CHAR_DATA * ch, bool fShort, bool colors));
void show_list_to_char args((OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing, const int iDefaultAction));
bool is_ignoring args((CHAR_DATA * ch, CHAR_DATA * ign_ch));
void show_race_line args((CHAR_DATA * ch, CHAR_DATA * victim));

/* act_move.c */
int is_nighttime args((void));
int check_stalk_move args((CHAR_DATA *ch, ROOM_INDEX_DATA *room));
int check_hide_move args((CHAR_DATA *ch, int sector, ROOM_INDEX_DATA *room));
void update_indoor_status args((int doornum, CHAR_DATA *ch, TOWN_DATA *town, int x, int y, int map, int type));
char *grab_word args((char *argument, char *arg_first));
char *wordwrap args((char *txt, sh_int wrap));
void clear_vrooms args((void));
int get_hunt_cost args((ROOM_INDEX_DATA *room));
ED *find_door args((CHAR_DATA * ch, char *arg, bool quiet));
ED *get_exit args((ROOM_INDEX_DATA * room, sh_int dir));
ED *get_exit_to args((ROOM_INDEX_DATA * room, sh_int dir, int vnum));
ED *get_exit_num args((ROOM_INDEX_DATA * room, sh_int count));
ch_ret move_char args((CHAR_DATA * ch, EXIT_DATA * pexit, int fall));
void teleport args((CHAR_DATA * ch, sh_int room, int flags));
sh_int encumbrance args((CHAR_DATA * ch, sh_int move));
bool will_fall args((CHAR_DATA * ch, int fall));
ch_ret pullcheck args((CHAR_DATA * ch, int pulse));
char *rev_exit args((sh_int vdir));

/* act_obj.c */

void wear_obj args((CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace, sh_int wear_bit));
bool remove_obj args((CHAR_DATA * ch, int iWear, sh_int fReplace));
bool can_layer args((CHAR_DATA * ch, OBJ_DATA * obj, sh_int wear_loc));
bool could_dual args((CHAR_DATA * ch));
bool can_dual args((CHAR_DATA * ch));
obj_ret damage_obj args((OBJ_DATA * obj, CHAR_DATA * attacker, int proj, int dam));
sh_int get_obj_resistance args((OBJ_DATA * obj, CHAR_DATA *ch));
void save_clan_storeroom args((CHAR_DATA * ch, CLAN_DATA * clan));
void obj_fall args((OBJ_DATA * obj, bool through));

/* act_wiz.c */
TRAP_DATA *copy_trap args((TRAP_DATA *otrap, OBJ_DATA *obj));
RID *find_location args((CHAR_DATA * ch, char *arg));
void echo_to_all args((sh_int AT_COLOR, char *argument, sh_int tar));
void get_reboot_string args((void));
struct tm *update_time args((struct tm * old_time));
void free_social args((SOCIALTYPE * social));
void add_social args((SOCIALTYPE * social));
void free_command args((CMDTYPE * command));
void unlink_command args((CMDTYPE * command));
void add_command args((CMDTYPE * command));
void ld_punt args((CHAR_DATA * ch)); //--Critt

/* arena.c */
CD *switch_arena_char args((CHAR_DATA * ch));
void update_barena args((CHAR_DATA * ch, int type));

//bank.c
int get_townbank_weight args((TOWN_DATA *town));

/* boards.c */
void load_boards args((void));
BD *get_board args((OBJ_DATA * obj));
void free_note args((NOTE_DATA * pnote));

//archery.c
ch_ret ranged_attack args((CHAR_DATA * ch, char *argument, OBJ_DATA * weapon, OBJ_DATA * projectile, sh_int dt, sh_int range));
int find_sqrt args((int dist));

/* gboard.c */
NOTE_DATA *note_free;
void free_global_note args((NOTE_DATA * note));
char *add_space args((int size, int tsize));

/* build.c */
float adjust_weight args((char *argument));
TOWN_DATA *find_town args((int x, int y, int map));
void write_area_list args((void));
int get_cmdflag args((char *flag));
char *flag_string args((int bitvector, char *const flagarray[]));
char *ext_flag_string args((EXT_BV * bitvector, char *const flagarray[]));
int get_npc_race args((char *type));
char *print_npc_race args((int race));
int get_exflag args((char *flag));
int get_rflag args((char *flag));
int get_mpflag args((char *flag));
int get_qmobflag args((char *flag));
int get_dir args((char *txt));
int get_new_dir args((char *txt, int type));
char *strip_cr args((char *str));
void fdarea args((CHAR_DATA * keeper, char *argument));
OBJ_DATA *shop_ocreate args((CHAR_DATA * keeper, OBJ_DATA * obj));
OBJ_DATA *shop_oclean args((CHAR_DATA * keeper, OBJ_DATA * obj));
void odelete args((CHAR_DATA * ch, char *argument));


/* clans.c */
CL *get_clan args((char *name));
void load_clans args((void));
void save_clan args((CLAN_DATA * clan));

CO *get_council args((char *name));
void load_councils args((void));
void save_council args((COUNCIL_DATA * council));

/* deity.c */
DE *get_deity args((char *name));
void load_deity args((void));
void save_deity args((DEITY_DATA * deity));

/* comm.c */
void read_pfiles_for_stats args((char *filename));
void close_socket args((DESCRIPTOR_DATA * dclose, bool force));
void write_to_buffer args((DESCRIPTOR_DATA * d, const char *txt, int length));
void write_to_pager args((DESCRIPTOR_DATA * d, const char *txt, int length));
void send_to_char args((const char *txt, CHAR_DATA * ch));
void send_to_char_color args((const char *txt, CHAR_DATA * ch));
void send_to_pager args((const char *txt, CHAR_DATA * ch));
void send_to_pager_color args((const char *txt, CHAR_DATA * ch));
void set_char_color args((sh_int AType, CHAR_DATA * ch));
void set_pager_color args((sh_int AType, CHAR_DATA * ch));
void ch_printf args((CHAR_DATA * ch, char *fmt, ...));
void ch_printf_color args((CHAR_DATA * ch, char *fmt, ...));
void log_printf args((char *fmt, ...));
void pager_printf args((CHAR_DATA * ch, char *fmt, ...));
void pager_printf_color args((CHAR_DATA * ch, char *fmt, ...));
void act args((sh_int AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type));
void copyover_recover args((void));
void actns args((sh_int AType, const char *format, CHAR_DATA * ch, const void *arg1, const void *arg2, int type));
char *myobj args((OBJ_DATA * obj));
char *obj_short args((OBJ_DATA * obj));
void free_runbuf args((DESCRIPTOR_DATA * d));


/* reset.c */
RD *make_reset args((char letter, int extra, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int resetlast, int resettime));
RD *add_reset args((AREA_DATA * tarea, char letter, int extra, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int resetlast, int resettime));
RD *place_reset args((AREA_DATA * tarea, char letter, int extra, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int resetlast, int resettime));
void reset_area args((AREA_DATA * pArea, int ttype));
bool is_room_reset args((RESET_DATA * pReset, ROOM_INDEX_DATA * aRoom, AREA_DATA * pArea));
void add_obj_reset args((AREA_DATA * pArea, char cm, OBJ_DATA * obj, int v2, int v3));
void delete_reset args((AREA_DATA * pArea, RESET_DATA * pReset));
void instaroom args((CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, bool dodoors, bool allmap));
void edit_reset args((CHAR_DATA * ch, char *argument, AREA_DATA * pArea, ROOM_INDEX_DATA * aRoom));
void kupkeep args((AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom));
void rsmob args((AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom));

/* db.c */
float fread_float args((FILE * fp));
void delete_area args((AREA_DATA *tarea));
int parse_helpfile_index args((FILE *fp, HELP_DATA *pHelp, char *argument));
void show_file args((CHAR_DATA * ch, char *filename));
#ifdef MTRACE
#define str_dup( str ) (_str_dup( (str), __FILE__, __FUNCTION__, __LINE__ ))
char *_str_dup args( ( const char *str, char *file_n, char *funct_n, int line_n ) );
#else
char *str_dup args((char const *str));
#endif
void boot_db args((bool fCopyOver));
void area_update args((void));
void add_char args((CHAR_DATA * ch));
CD *create_mobile args((MOB_INDEX_DATA * pMobIndex));
OD *create_object args((OBJ_INDEX_DATA * pObjIndex, int level));
void clear_char args((CHAR_DATA * ch));
void free_char args((CHAR_DATA * ch));
char *get_extra_descr args((const char *name, EXTRA_DESCR_DATA * ed));
MID *get_mob_index args((sh_int vnum));
OID *get_obj_index args((int vnum));
RID *get_room_index args((int vnum));
char fread_letter args((FILE * fp));
int fread_number args((FILE * fp));
EXT_BV fread_bitvector args((FILE * fp));
void fwrite_bitvector args((EXT_BV * bits, FILE * fp));
char *print_bitvector args((EXT_BV * bits));
char *fread_string args((FILE * fp));
char *fread_string_nohash args((FILE * fp));
void fread_to_eol args((FILE * fp));
char *fread_word args((FILE * fp));
char *fread_line args((FILE * fp));
int number_fuzzy args((int number));
int number_range args((int from, int to));
sh_int snumber_range args((sh_int from, sh_int to));
int number_percent args((void));
int number_door args((void));
int number_bits args((int width));
int number_mm args((void));
sh_int snumber_mm args((void));
int dice args((int number, int size));
sh_int sdice args((sh_int number, sh_int size));
int interpolate args((int level, int value_00, int value_32));
void smash_tilde args((char *str));
void hide_tilde args((char *str));
char *show_tilde args((char *str));
bool str_cmp args((const char *astr, const char *bstr));
bool str_prefix args((const char *astr, const char *bstr));
bool str_infix args((const char *astr, const char *bstr));
bool str_suffix args((const char *astr, const char *bstr));
char *capitalize args((const char *str));
char *strlower args((const char *str));
char *strupper args((const char *str));
char *aoran args((const char *str));
void append_file args((CHAR_DATA * ch, char *file, char *str));
void append_to_file args((char *file, char *str));
void bug args((const char *str, ...));
void log_string_plus args((const char *str, sh_int log_type, sh_int level));
RID *make_room args((int vnum));
OID *make_object args((int vnum, int cvnum, char *name, int cprog));
MID *make_mobile args((sh_int vnum, sh_int cvnum, char *name));
ED *make_exit args((ROOM_INDEX_DATA * pRoomIndex, ROOM_INDEX_DATA * to_room, sh_int door));
void add_help args((HELP_DATA * pHelp));
void fix_area_exits args((AREA_DATA * tarea));
void load_area_file args((AREA_DATA * tarea, char *filename));
void randomize_exits args((ROOM_INDEX_DATA * room, sh_int maxdir));
void make_wizlist args((void));
void tail_chain args((void));
bool delete_room args((ROOM_INDEX_DATA * room));
bool delete_obj args((OBJ_INDEX_DATA * obj));
bool delete_mob args((MOB_INDEX_DATA * mob));

/* Functions to add to sorting lists. -- Altrag */
/*void	mob_sort	args( ( MOB_INDEX_DATA *pMob ) );
void	obj_sort	args( ( OBJ_INDEX_DATA *pObj ) );
void	room_sort	args( ( ROOM_INDEX_DATA *pRoom ) );*/
void sort_area args((AREA_DATA * pArea, bool proto));
void sort_area_by_name args((AREA_DATA * pArea)); /* Fireblade */
void write_projects args((void));

/* editor.c */
void discard_editdata(EDITOR_DATA * edd);

/* build.c */
/*
void	start_editing	args( ( CHAR_DATA *ch, char *data ) );
void	stop_editing	args( ( CHAR_DATA *ch ) );
void	edit_buffer	args( ( CHAR_DATA *ch, char *argument ) );
char *	copy_buffer	args( ( CHAR_DATA *ch ) );  */
bool can_rmodify args((CHAR_DATA * ch, ROOM_INDEX_DATA * room));
bool can_omodify args((CHAR_DATA * ch, OBJ_DATA * obj));
bool can_mmodify args((CHAR_DATA * ch, CHAR_DATA * mob));
bool can_medit args((CHAR_DATA * ch, MOB_INDEX_DATA * mob));
void free_reset args((AREA_DATA * are, RESET_DATA * res));
void free_area args((AREA_DATA * are));
void assign_area args((CHAR_DATA * ch));
EDD *SetRExtra args((ROOM_INDEX_DATA * room, char *keywords));
bool DelRExtra args((ROOM_INDEX_DATA * room, char *keywords));
EDD *SetOExtra args((OBJ_DATA * obj, char *keywords));
bool DelOExtra args((OBJ_DATA * obj, char *keywords));
EDD *SetOExtraProto args((OBJ_INDEX_DATA * obj, char *keywords));
bool DelOExtraProto args((OBJ_INDEX_DATA * obj, char *keywords));
void fold_area args((AREA_DATA * tarea, char *filename, bool install, int message));
int get_otype args((char *type));
int get_atype args((char *type));
int get_aflag args((char *flag));
int get_oflag args((char *flag));
int get_wflag args((char *flag));
int get_talentflag args((char *flag));
void init_area_weather args((void));
void save_weatherdata args((void));
void do_addgem args((CHAR_DATA * ch, char *argument));
void do_addbox args((CHAR_DATA * ch, char *argument));

/* fight.c */
int check_powerlevel args((CHAR_DATA *ch, CHAR_DATA *victim));
int get_player_statlevel args((CHAR_DATA *ch));
int player_equipment_worth args((CHAR_DATA *ch));
int get_fightingstyle_dam args((CHAR_DATA *ch, int dam, CHAR_DATA *victim, int type, int learn));
void adjust_aggression_list args((CHAR_DATA *victim, CHAR_DATA *ch, int dam, int type, int sn));
bool legal_loot_coins args((CHAR_DATA * ch, CHAR_DATA * victim));
int player_stat_worth args((CHAR_DATA *ch));
int wielding_skill_weapon args((CHAR_DATA *ch, int missile));
int get_sore_rate args((int race, int maxhp));
int get_hit_or_miss args((CHAR_DATA *ch, CHAR_DATA *victim, int ac, int grip, int limb, int noarmor, OBJ_DATA *wield, int sn));
bool can_see_intro args((CHAR_DATA *ch, CHAR_DATA *victim));
int max_fight args((CHAR_DATA * ch));
int npcrace_agi args((int race));
int get_btimer args((CHAR_DATA *ch, int sn, CHAR_DATA *victim));
void violence_update args((void));
ch_ret one_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt, int limb));
ch_ret projectile_hit args((CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * wield, OBJ_DATA * projectile, sh_int dist, int dt));
sh_int ris_damage args((CHAR_DATA * ch, sh_int dam, int ris));
ch_ret damage args((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int spec, int limb));
void update_pos args((CHAR_DATA * victim));
void set_fighting args((CHAR_DATA * ch, CHAR_DATA * victim));
void stop_fighting args((CHAR_DATA * ch, bool fBoth));
void free_fight args((CHAR_DATA * ch));
CD *who_fighting args((CHAR_DATA * ch));
void check_killer args((CHAR_DATA * ch, CHAR_DATA * victim));
void check_attacker args((CHAR_DATA * ch, CHAR_DATA * victim));
void death_cry args((CHAR_DATA * ch));
void stop_hunting args((CHAR_DATA * ch));
void stop_hating args((CHAR_DATA * ch));
void stop_fearing args((CHAR_DATA * ch));
void start_hunting args((CHAR_DATA * ch, CHAR_DATA * victim));
void start_hating args((CHAR_DATA * ch, CHAR_DATA * victim));
void start_fearing args((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_hunting args((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_hating args((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_fearing args((CHAR_DATA * ch, CHAR_DATA * victim));
bool is_safe args((CHAR_DATA * ch, CHAR_DATA * victim));
bool legal_loot args((CHAR_DATA * ch, CHAR_DATA * victim));
sh_int VAMP_AC args((CHAR_DATA * ch));
bool check_illegal_pk args((CHAR_DATA * ch, CHAR_DATA * victim));
void raw_kill args((CHAR_DATA * ch, CHAR_DATA * victim));
bool in_arena args((CHAR_DATA * ch));
bool can_astral args((CHAR_DATA * ch, CHAR_DATA * victim));

/* treasure.c */
OBJ_DATA *generate_tbox args((CHAR_DATA *ch));
OBJ_DATA *generate_treasure args((CHAR_DATA * mob));

/* makeobjs.c */
void update_container args((OBJ_DATA * corpse, int x, int y, int map, int fx, int fy, int fmap));
void make_corpse args((CHAR_DATA * ch, CHAR_DATA * killer));
void make_blood args((CHAR_DATA * ch));
void make_bloodstain args((CHAR_DATA * ch));
void make_scraps args((OBJ_DATA * obj, CHAR_DATA *ch));
void make_fire args((ROOM_INDEX_DATA * in_room, sh_int timer));
OD *make_trap args((int v0, int v1, int v2, int v3));
OD *create_money args((int amount));

/* misc.c */
void set_bit_wilderness args((CHAR_DATA *ch, int bits));
void remove_bit_wilderness args((CHAR_DATA *ch, int bits));
bool is_set_wilderness args((CHAR_DATA *ch, int bits, int x, int y, int map));
void actiondesc args((CHAR_DATA * ch, OBJ_DATA * obj, void *vo));
EXT_BV meb args((int bit));
EXT_BV multimeb args((int bit, ...));

/* newarena.c */
void load_hall_of_fame args((void));

/* deity.c */
void adjust_favor args((CHAR_DATA * ch, int field, int mod));

/* mud_comm.c */
char *mprog_type_to_name args((int type));

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *strstr args((const char *s1, const char *s2));
#endif

void mprog_wordlist_check args((char *arg, CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void *vo, int type));
void mprog_percent_check args((CHAR_DATA * mob, CHAR_DATA * actor, OBJ_DATA * object, void *vo, int type));
void mprog_act_trigger args((char *buf, CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj, void *vo));
void mprog_bribe_trigger args((CHAR_DATA * mob, CHAR_DATA * ch, int amount));
void mprog_entry_trigger args((CHAR_DATA * mob));
void mprog_give_trigger args((CHAR_DATA * mob, CHAR_DATA * ch, OBJ_DATA * obj));
void mprog_greet_trigger args((CHAR_DATA * mob));
void mprog_fight_trigger args((CHAR_DATA * mob, CHAR_DATA * ch));
void mprog_hitprcnt_trigger args((CHAR_DATA * mob, CHAR_DATA * ch));
void mprog_death_trigger args((CHAR_DATA * killer, CHAR_DATA * mob));
void mprog_random_trigger args((CHAR_DATA * mob));
void mprog_speech_trigger args((char *txt, CHAR_DATA * mob));
void mprog_script_trigger args((CHAR_DATA * mob));
void mprog_hour_trigger args((CHAR_DATA * mob));
void mprog_time_trigger args((CHAR_DATA * mob));
void progbug args((char *str, CHAR_DATA * mob));
void rset_supermob args((ROOM_INDEX_DATA * room));
void release_supermob args(());
void mpsleep_update args(());

/* planes.c */
PLANE_DATA *plane_lookup args((const char *name));
void load_planes args((void));
void save_planes args((void));
void check_planes args((PLANE_DATA * p));

/* player.c */
char *get_rating args((int stat, int base));
void set_title args((CHAR_DATA * ch, char *title));

/* polymorph.c */
void fwrite_morph_data args((CHAR_DATA * ch, FILE * fp));
void fread_morph_data args((CHAR_DATA * ch, FILE * fp));
void clear_char_morph args((CHAR_MORPH * morph));
CHAR_MORPH *make_char_morph args((MORPH_DATA * morph));
void free_char_morph args((CHAR_MORPH * morph));
CHAR_MORPH *make_char_morph args((MORPH_DATA * morph));
char *race_string args((int bitvector));
char *class_string args((int bitvector));
void setup_morph_vnum args((void));
void unmorph_all args((MORPH_DATA * morph));
MORPH_DATA *get_morph args((char *arg));
MORPH_DATA *get_morph_vnum args((int arg));
int do_morph_char args((CHAR_DATA * ch, MORPH_DATA * morph));
MORPH_DATA *find_morph args((CHAR_DATA * ch, char *target, bool is_cast));
void do_unmorph_char args((CHAR_DATA * ch));
void send_morph_message args((CHAR_DATA * ch, MORPH_DATA * morph, bool is_morph));
bool can_morph args((CHAR_DATA * ch, MORPH_DATA * morph, bool is_cast));
void do_morph args((CHAR_DATA * ch, MORPH_DATA * morph));
void do_unmorph args((CHAR_DATA * ch));
void save_morphs args((void));
void fwrite_morph args((FILE * fp, MORPH_DATA * morph));
void load_morphs args((void));
MORPH_DATA *fread_morph args((FILE * fp));
void free_morph args((MORPH_DATA * morph));
void morph_defaults args((MORPH_DATA * morph));
void sort_morphs args((void));


/* skills.c */
int check_twohand_shield args((CHAR_DATA *ch));
int get_skillflux_value args((CHAR_DATA *ch, int ssn));
bool can_use_skill args((CHAR_DATA * ch, int percent, int gsn));
bool check_skill args((CHAR_DATA * ch, char *command, char *argument));
void learn_from_success args((CHAR_DATA * ch, int sn, CHAR_DATA *target));
void learn_from_failure args((CHAR_DATA * ch, int sn, CHAR_DATA *target));
int check_parry args((CHAR_DATA * ch, CHAR_DATA * victim));
bool check_dodge args((CHAR_DATA * ch, CHAR_DATA * victim, int limb));
bool check_grip args((CHAR_DATA * ch, CHAR_DATA * victim));
void disarm args((CHAR_DATA * ch, CHAR_DATA * victim));
void trip args((CHAR_DATA * ch, CHAR_DATA * victim));
bool mob_fire args((CHAR_DATA * ch, char *name, int vdir));
CD *scan_for_victim args((CHAR_DATA * ch, EXIT_DATA * pexit, char *name));

/* overland.c */
void steer_ship args((CHAR_DATA *ch, SHIP_DATA *ship, int dir));
void set_ship_sector args((SHIP_DATA *ship, int reset, int save));
void adjust_wildermob args((CHAR_DATA *mob, int lvl, int wilderness));
int get_wilder_weapon args((int lvl, int dual));
int get_slab_vnum args((int lvl, int num));
void update_objects args((CHAR_DATA * ch, sh_int map, sh_int x, sh_int y));
void update_players_map args((CHAR_DATA * ch, int x, int y, int map, int who, ROOM_INDEX_DATA * room));
void add_entrance args((int tomap, int onmap, int hereX, int hereY, int thereX, int thereY, int vnum));
void save_map args((char *name, int map));
int find_mob_level args((int x, int y, int map));
void save_resources args((char *name, int map));
char *get_map_name args((int map));
int get_wilderness_hunt_cost args((CHAR_DATA *ch, int x, int y));

/* ban.c */
int add_ban args((CHAR_DATA * ch, char *arg1, char *arg2, int time, int type));
void show_bans args((CHAR_DATA * ch, int type));
void save_banlist args((void));
void load_banlist args((void));
bool check_total_bans args((DESCRIPTOR_DATA * d));
bool check_bans args((DESCRIPTOR_DATA *d, CHAR_DATA * ch, int type, int nban));

/* imm_host.c */
bool check_immortal_domain args((CHAR_DATA * ch, char *host));
int load_imm_host args((void));
int fread_imm_host args((FILE * fp, IMMORTAL_HOST * data));
void do_write_imm_host args((void));
void do_add_imm_host args((CHAR_DATA * ch, char *argument));

/* track */
void hunt_victim_map args((CHAR_DATA * ch));
int get_x args((int currx, int dir));
int get_y args((int curry, int dir));

/* name_gen.c */
void generate_randomname args((int race, int sex, char *buffer));

/* handler.c */
char *showgemaff args((CHAR_DATA * ch, OBJ_DATA *obj, int passbuf, IMBUE_DATA *imbue));
ch_ret pre_spring_trap args((CHAR_DATA * ch, OBJ_DATA * obj, TRAP_DATA *trap, OBJ_DATA *trapobj));
void check_aff_learn args((CHAR_DATA *ch, char *skillname, int sn, CHAR_DATA *victim, int success));
char *get_group_name2 args((int group));
int is_part_sphere args((int sphere, int group));
sh_int issphere args((char *argument));
OBJ_DATA *bank_to_char args((OBJ_DATA * obj, CHAR_DATA *ch));
OBJ_DATA *obj_to_bank args((OBJ_DATA * obj, CHAR_DATA *ch));
OBJ_DATA *obj_to_townbank args((OBJ_DATA * obj, TOWN_DATA *town));
OBJ_DATA *townbank_to_char args((OBJ_DATA * obj, TOWN_DATA *town, CHAR_DATA *ch));
int get_obj_on_char args((CHAR_DATA *ch, OBJ_DATA * pobj));
int get_ch_carry_number args((CHAR_DATA *ch));
float get_ch_carry_weight args((CHAR_DATA *ch));
bool room_is_private_wilderness args((CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex, int x, int y, int map));
bool check_npc args((CHAR_DATA *ch));
sh_int get_learned_value args((CHAR_DATA *ch, int sn));
sh_int get_mastered_value args((CHAR_DATA *ch, int sn));     
void find_next_target args((CHAR_DATA * ch));
char *get_wplevel args((int power));
char *get_tier_name args((int mastery));
void update_pkpower args((CHAR_DATA * ch));
void add_pkill args((CHAR_DATA * ch, CHAR_DATA * victim));
void update_pranking args((CHAR_DATA * ch, CHAR_DATA * victim));
int check_room_pk args((CHAR_DATA * ch));
bool is_room_safe args((CHAR_DATA * ch));
char *get_caste_name args((int caste, int sex));
char *get_caste_color args((int caste));
void update_movement_points args((CHAR_DATA *ch, int move));
int calculate_movement_cost args((int move, CHAR_DATA *ch));
int movement_lag args((CHAR_DATA *ch, int beats));
int get_dam_roll args((CHAR_DATA * ch));
int get_hit_roll args((CHAR_DATA * ch));
bool in_hellmaze args((CHAR_DATA * ch));
void find_next_hunt args((CHAR_DATA * ch, int type));
bool find_sector args((CHAR_DATA * ch, int sector));
char *get_sphere_name args((int sphere));
char *get_sphere_name2 args((int sphere));
int get_mastery_num args((char *mastery));
char *get_mastery_name args((int mastery));
sh_int get_mastery_value args((CHAR_DATA * ch, int sn, int isobj, int lv));
sh_int get_point_value args((CHAR_DATA * ch, int sn, int isobj, int lv));
sh_int group_high args((CHAR_DATA * ch, char *group));
sh_int isgroup args((char *argument));
bool in_same_room args((CHAR_DATA * ch, CHAR_DATA * victim));
void give_chase args((CHAR_DATA * ch, int oldmap));
char *get_group_name args((int group));
bool enough_resources args((CHAR_DATA * ch, int hometown, int needed, int ctype));
int strlen_color args((char *argument));
sh_int get_trust args((CHAR_DATA * ch));
sh_int get_ftrust args((CHAR_DATA * ch));
sh_int get_age args((CHAR_DATA * ch));
sh_int get_curr_str args((CHAR_DATA * ch));
sh_int get_curr_int args((CHAR_DATA * ch));
sh_int get_curr_wis args((CHAR_DATA * ch));
sh_int get_curr_dex args((CHAR_DATA * ch));
sh_int get_curr_con args((CHAR_DATA * ch));
sh_int get_curr_cha args((CHAR_DATA * ch));
sh_int get_curr_lck args((CHAR_DATA * ch));
sh_int get_curr_agi args((CHAR_DATA * ch));
int count_users args((OBJ_DATA * obj));
int max_weight args((OBJ_DATA * obj));
bool can_take_proto args((CHAR_DATA * ch));
int can_carry_n args((CHAR_DATA * ch));
int can_carry_w args((CHAR_DATA * ch));
bool is_name args((const char *str, char *namelist));
bool is_name_prefix args((const char *str, char *namelist));
bool nifty_is_name args((char *str, char *namelist));
bool nifty_is_name_prefix args((char *str, char *namelist));
void affect_modify args((CHAR_DATA * ch, AFFECT_DATA * paf, sh_int fAdd));
void affect_to_char args((CHAR_DATA * ch, AFFECT_DATA * paf));
void affect_remove args((CHAR_DATA * ch, AFFECT_DATA * paf));
void affect_strip args((CHAR_DATA * ch, int sn));
bool is_affected args((CHAR_DATA * ch, int sn));
void affect_join args((CHAR_DATA * ch, AFFECT_DATA * paf));
void char_from_room args((CHAR_DATA * ch));
void char_to_room args((CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex));
OD *obj_to_char args((OBJ_DATA * obj, CHAR_DATA * ch));
void obj_from_char args((OBJ_DATA * obj));
int apply_ac args((OBJ_DATA * obj, int iWear));
OD *get_eq_char args((CHAR_DATA * ch, int iWear));
void equip_char args((CHAR_DATA * ch, OBJ_DATA * obj, int iWear));
void unequip_char args((CHAR_DATA * ch, OBJ_DATA * obj));
int count_obj_list args((OBJ_INDEX_DATA * obj, OBJ_DATA * list));
int count_mob_in_room args((MOB_INDEX_DATA * mob, ROOM_INDEX_DATA * list));
void obj_from_room args((OBJ_DATA * obj));
OD *obj_to_room args((OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex, CHAR_DATA * ch));
OD *obj_to_obj args((OBJ_DATA * obj, OBJ_DATA * obj_to));
void obj_from_obj args((OBJ_DATA * obj));
void extract_obj args((OBJ_DATA * obj));
void extract_exit args((ROOM_INDEX_DATA * room, EXIT_DATA * pexit));
void extract_room args((ROOM_INDEX_DATA * room));
void clean_room args((ROOM_INDEX_DATA * room));
void clean_obj args((OBJ_INDEX_DATA * obj));
void clean_mob args((MOB_INDEX_DATA * mob));
void clean_resets args((AREA_DATA * tarea));
void extract_char args((CHAR_DATA * ch, bool fPull));
CD *get_char_room args((CHAR_DATA * ch, char *argument));
CD *get_char_room_new args((CHAR_DATA * ch, char *argument, int type));
CD *get_char_wilder args((CHAR_DATA * ch, char *argument));
CD *get_char_world args((CHAR_DATA * ch, char *argument));
OD *get_obj_type args((OBJ_INDEX_DATA * pObjIndexData));
OD *get_obj_list args((CHAR_DATA * ch, char *argument, OBJ_DATA * list));
OD *get_obj_list_rev args((CHAR_DATA * ch, char *argument, OBJ_DATA * list));
OD *get_obj_carry args((CHAR_DATA * ch, char *argument));
OD *get_obj_wear args((CHAR_DATA * ch, char *argument));
OD *get_obj_vnum args((CHAR_DATA * ch, int vnum));
OD *get_obj_here args((CHAR_DATA * ch, char *argument));
OD *get_obj_world args((CHAR_DATA * ch, char *argument));
OD *get_obj_mobworld args((CHAR_DATA * ch, char *argument));
int get_obj_number args((OBJ_DATA * obj));
float get_obj_weight args((OBJ_DATA * obj));
int get_real_obj_weight args((OBJ_DATA * obj));
bool room_is_dark args((ROOM_INDEX_DATA * pRoomIndex));
bool room_is_private args((ROOM_INDEX_DATA * pRoomIndex));
bool can_see args((CHAR_DATA * ch, CHAR_DATA * victim));
bool can_see_obj args((CHAR_DATA * ch, OBJ_DATA * obj));
bool can_see_map args((CHAR_DATA * ch, CHAR_DATA * victim));
bool can_see_index args((MOB_INDEX_DATA *ch, CHAR_DATA *victim));
bool can_see_obj_map args((CHAR_DATA * ch, OBJ_DATA * obj));
bool can_drop_obj args((CHAR_DATA * ch, OBJ_DATA * obj));
char *item_type_name args((OBJ_DATA * obj));
char *affect_loc_name args((int location));
char *affect_bit_name args((EXT_BV * vector));
char *extra_bit_name args((EXT_BV * extra_flags));
char *magic_bit_name args((int magic_flags));
char *pull_type_name args((int pulltype));
ch_ret check_for_trap args((CHAR_DATA * ch, OBJ_DATA * obj, int flag, int newflag));
ch_ret check_room_for_traps args((CHAR_DATA * ch, int flag));
bool is_trapped args((OBJ_DATA * obj));
OD *get_trap args((OBJ_DATA * obj));
ch_ret spring_trap args((CHAR_DATA * ch, OBJ_DATA * obj, TRAP_DATA *trap));
void name_stamp_stats args((CHAR_DATA * ch));
void fix_char args((CHAR_DATA * ch));
char* showaffect args((CHAR_DATA * ch, AFFECT_DATA * paf, int passbuf));
void set_cur_obj args((OBJ_DATA * obj));
bool obj_extracted args((OBJ_DATA * obj));
void queue_extracted_obj args((OBJ_DATA * obj));
void clean_obj_queue args((void));
void set_cur_char args((CHAR_DATA * ch));
bool char_died args((CHAR_DATA * ch));
void queue_extracted_char args((CHAR_DATA * ch, bool extract));
void clean_char_queue args((void));
void add_timer args((CHAR_DATA * ch, sh_int type, sh_int count, DO_FUN * fun, int value));
TIMER *get_timerptr args((CHAR_DATA * ch, sh_int type));
sh_int get_timer args((CHAR_DATA * ch, sh_int type));
void extract_timer args((CHAR_DATA * ch, TIMER * timer));
void remove_timer args((CHAR_DATA * ch, sh_int type));
bool in_soft_range args((CHAR_DATA * ch, AREA_DATA * tarea));
bool in_hard_range args((CHAR_DATA * ch, AREA_DATA * tarea));
bool chance args((CHAR_DATA * ch, sh_int percent));
bool chance_attrib args((CHAR_DATA * ch, sh_int percent, sh_int attrib));
OD *clone_object args((OBJ_DATA * obj));
OD *split_obj args((OBJ_DATA * obj, int num));
OD *separate_obj args((OBJ_DATA * obj));
bool empty_obj args((OBJ_DATA * obj, OBJ_DATA * destobj, ROOM_INDEX_DATA * destroom));
OD *find_obj args((CHAR_DATA * ch, char *argument, bool carryonly));
bool ms_find_obj args((CHAR_DATA * ch));
void worsen_mental_state args((CHAR_DATA * ch, int mod));
void better_mental_state args((CHAR_DATA * ch, int mod));
void boost_economy args((AREA_DATA * tarea, int gold));
void lower_economy args((AREA_DATA * tarea, int gold));
void economize_mobgold args((CHAR_DATA * mob));
bool economy_has args((AREA_DATA * tarea, int gold));
void add_kill args((CHAR_DATA * ch, CHAR_DATA * mob));
int times_killed args((CHAR_DATA * ch, CHAR_DATA * mob));
void update_aris args((CHAR_DATA * ch));
AREA_DATA *get_area args((char *name)); /* FB */
OD *get_objtype args((CHAR_DATA * ch, sh_int type));

/* interp.c */
bool check_pos args((CHAR_DATA * ch, sh_int position));
void interpret args((CHAR_DATA * ch, char *argument));
bool is_number args((char *arg));
int number_argument args((char *argument, char *arg));
char *one_argument args((char *argument, char *arg_first));
char *one_argument2 args((char *argument, char *arg_first));
ST *find_social args((char *command));
CMDTYPE *find_command args((char *command));
void hash_commands args(());
void displayFightTimer args((CHAR_DATA *ch));
void start_timer args((struct timeval * stime));
time_t end_timer args((struct timeval * stime));
void send_timer args((struct timerset * vtime, CHAR_DATA * ch));
void update_userec args((struct timeval * time_used, struct timerset * userec));

/* magic.c */
bool is_immune args((CHAR_DATA * ch, sh_int damtype, int othertypes));
void gain_mana_per args((CHAR_DATA *ch, CHAR_DATA *victim, int mana));
int code_identify args((CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim, int sn, char *dbuf));
bool process_spell_components args((CHAR_DATA * ch, int sn));
int ch_slookup args((CHAR_DATA * ch, const char *name));
int find_spell args((CHAR_DATA * ch, const char *name, bool know));
int find_skill args((CHAR_DATA * ch, const char *name, bool know));
int find_weapon args((CHAR_DATA * ch, const char *name, bool know));
int find_tongue args((CHAR_DATA * ch, const char *name, bool know));
int skill_lookup args((const char *name));
bool can_use_portal args((CHAR_DATA * ch, int type));
int herb_lookup args((const char *name));
int personal_lookup args((CHAR_DATA * ch, const char *name));
int slot_lookup args((int slot));
int bsearch_skill args((const char *name, int first, int top));
int bsearch_skill_exact args((const char *name, int first, int top));
int bsearch_skill_prefix args((const char *name, int first, int top));
bool saves_poison_death args((int level, CHAR_DATA * victim));
bool saves_wand args((int level, CHAR_DATA * victim));
bool saves_para_petri args((int level, CHAR_DATA * victim));
bool saves_breath args((int level, CHAR_DATA * victim));
bool saves_spell_staff args((int level, CHAR_DATA * victim));
ch_ret obj_cast_spell args((int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj));
int dice_parse args((CHAR_DATA * ch, int level, char *exp, int sn));
SK *get_skilltype args((int sn));

/* request.c */
void init_request_pipe args((void));
void check_requests args((void));

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
#define OS_KINGDOM  2
#define OS_BANK     3
#define OS_GROUND   4
#define OS_MARKET   5
void save_account args((DESCRIPTOR_DATA *d, int editing));
bool load_account args((DESCRIPTOR_DATA * d, char *name, bool preload));
char *fread_lastname_line args((FILE *fp));
void remove_from_lastname_file args((char *lastname, char *firstname));
void write_lastname_file args((char *lastname, char *firstname));
void save_char_obj args((CHAR_DATA * ch));
bool load_char_obj args((DESCRIPTOR_DATA * d, char *name, bool preload));
void set_alarm args((long seconds));
void requip_char args((CHAR_DATA * ch));
void fwrite_obj args((CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest, sh_int os_type));
void fread_obj args((CHAR_DATA * ch, FILE * fp, sh_int os_type));
void de_equip_char args((CHAR_DATA * ch));
void re_equip_char args((CHAR_DATA * ch));
void read_char_mobile args((char *argument));
void write_char_mobile args((CHAR_DATA * ch, char *argument));
CHAR_DATA *fread_mobile args((FILE * fp));
void fwrite_mobile args((FILE * fp, CHAR_DATA * mob));

/* shops.c */

/* special.c */
SF *spec_lookup args((const char *name));
char *lookup_spec args((SPEC_FUN * special));

/* tables.c */
void save_market_data args((void));
void load_market_data args((void));
void fwrite_ship_data args((void));
void load_ship_data args((void));
void load_npcrace_file args((void));
void save_npcrace_file args((void));
TRAP_DATA *load_trap_file args((FILE *sfp));
void save_trap_file args((TRAP_DATA *strap, FILE *sfp));
void save_portal_file args((void));
void read_channelhistory_file args((void));
void write_channelhistory_file args((void));
void fread_training_list args((void));
void fwrite_training_list args((void));
int get_control_size args((int size));
void fread_battle_descriptions args((void));
void fwrite_battle_descriptions args((void));
void save_conquer_file args((void));
void load_conquer_file args((void));
char *parse_save_file args((char *name));
void load_buykingdom_data args((void));
void load_default_depo args((void));
void load_trade_file args((void));
void save_trade_file args((void));
void write_depo_list args((void));
void load_kingdom_depo args((void));
void save_buykingdom_data args((void));
void load_portal_file args((void));
void write_portal_file args((void));
void save_extraction_data args((void));
void load_extraction_data args((void));
void load_mlist_data args((void));
void save_mlist_data args((void));
void load_wblock_data args((void));
void save_wblock_data args((void));
void load_trainer_data args((void));
void save_trainer_data args((void));
void load_kchest_file args((void));
void read_last_file args((CHAR_DATA *ch, int count, char *name));
void write_last_file args((char *entry));
void save_kingdom_chests args((CHAR_DATA * ch));
int get_skill args((char *skilltype));
char *spell_name args((SPELL_FUN * spell));
char *skill_name args((DO_FUN * skill));
void save_bin_data args((void));
void load_bin_data args((void));
void load_skill_table args((void));
void save_skill_table args((void));
void sort_skill_table args((void));
void remap_slot_numbers args((void));
void load_socials args((void));
void save_socials args((void));
void load_commands args((void));
void load_barena_data args((void));
void save_barena_data args((void));
void load_gem_data args((void));
void save_gem_data args((void));
void load_box_data args((void));
void save_box_data args((void));
void save_commands args((void));
SPELL_FUN *spell_function args((char *name));
DO_FUN *skill_function args((char *name));
void load_kingdoms args((void));
bool load_class_file args((char *fname));
void write_class_file args((int cl));
void write_kingdom_list args((void));
void write_kingdom_file args((int cl));
void fwrite_authlist args((void));
void load_authlist args((void));
void save_classes args((void));
void load_classes args((void));
void load_herb_table args((void));
void save_herb_table args((void));
void load_races args((void));
void load_tongues args((void));

/* track.c */
void found_prey args((CHAR_DATA * ch, CHAR_DATA * victim));
void hunt_victim args((CHAR_DATA * ch));

/* treasure.c */
OBJ_DATA *generate_treasure args(());
bool tchance args((int num));

/* update.c */
void auth_update args((void));
int cvttime args((int time));
void copyover_check args((void));
int totalrooms args((int size));
int max_allowedunits args((int size));
void weather_update args((void));
void update_interest args((CHAR_DATA * ch));
void gain_condition args((CHAR_DATA * ch, int iCond, int value));
void update_handler args((void));
void reboot_check args((time_t reset));

#if 0
void reboot_check args((char *arg));
#endif
void auction_update args((void));
void remove_portal args((OBJ_DATA * portal));
void weather_update args((void));

/* hashstr.c */
#ifdef MTRACE
#define str_alloc( str ) (_str_alloc( (str), __FILE__, __FUNCTION__, __LINE__ ) )
char *_str_alloc args( ( const char *str, char *file_n, char *funct_n, int line_n ) );
#else
char *str_alloc args((char *str));
#endif
char *quick_link args((char *str));
int str_free args((char *str));
void show_hash args((int count));
char *hash_stats args((void));
char *check_hash args((char *str));
void hash_dump args((int hash));
void show_high_hash args((int top));

/* newscore.c */
char *get_class args((CHAR_DATA * ch));
char *get_race args((CHAR_DATA * ch));

#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 *
 *  New Build Interface Stuff Follows
 *
 */


/*
 *  Data for a menu page
 */
struct menu_data
{
   char *sectionNum;
   char *charChoice;
   int x;
   int y;
   char *outFormat;
   void *data;
   int ptrType;
   int cmdArgs;
   char *cmdString;
};

#define SH_INT 1
#define INT 2
#define CHAR 3
#define STRING 4
#define SPECIAL 5

/*
#define NO_PAGE    0
#define MOB_PAGE_A 1
#define MOB_PAGE_B 2
#define MOB_PAGE_C 3
#define MOB_PAGE_D 4
#define MOB_PAGE_E 5
#define MOB_PAGE_F 17
#define MOB_HELP_PAGE 14
#define ROOM_PAGE_A 6
#define ROOM_PAGE_B 7
#define ROOM_PAGE_C 8
#define ROOM_HELP_PAGE 15
#define OBJ_PAGE_A 9
#define OBJ_PAGE_B 10
#define OBJ_PAGE_C 11
#define OBJ_PAGE_D 12
#define OBJ_PAGE_E 13
#define OBJ_HELP_PAGE 16
#define CONTROL_PAGE_A 18
#define CONTROL_HELP_PAGE 19 */

#define NO_TYPE   0
#define MOB_TYPE  1
#define OBJ_TYPE  2
#define ROOM_TYPE 3
#define CONTROL_TYPE 4

#define SUB_NORTH DIR_NORTH
#define SUB_EAST  DIR_EAST
#define SUB_SOUTH DIR_SOUTH
#define SUB_WEST  DIR_WEST
#define SUB_UP    DIR_UP
#define SUB_DOWN  DIR_DOWN
#define SUB_NE    DIR_NORTHEAST
#define SUB_NW    DIR_NORTHWEST
#define SUB_SE    DIR_SOUTHEAST
#define SUB_SW    DIR_SOUTHWEST

/*
 * defines for use with this get_affect function
 */

#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30



/*
 *   Map Structures
 */

DECLARE_DO_FUN(do_mapout);
DECLARE_DO_FUN(do_lookmap);

struct map_data /* contains per-room data */
{
   int vnum; /* which map this room belongs to */
   int x; /* horizontal coordinate */
   int y; /* vertical coordinate */
   char entry; /* code that shows up on map */
};


struct map_index_data
{
   MAP_INDEX_DATA *next;
   int vnum; /* vnum of the map */
   int map_of_vnums[49][81]; /* room vnums aranged as a map */
};


MAP_INDEX_DATA *get_map_index(int vnum);
void init_maps();


/*
 * mudprograms stuff
 */
extern CHAR_DATA *supermob;

void oprog_speech_trigger(char *txt, CHAR_DATA * ch);
void oprog_random_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_wear_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
bool oprog_use_trigger(CHAR_DATA * ch, OBJ_DATA * obj, CHAR_DATA * vict, OBJ_DATA * targ, void *vo);
void oprog_remove_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_sac_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_damage_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_repair_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_drop_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_zap_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
char *oprog_type_to_name(int type);

/*
 * MUD_PROGS START HERE
 * (object stuff)
 */
void oprog_greet_trigger(CHAR_DATA * ch);
void oprog_speech_trigger(char *txt, CHAR_DATA * ch);
void oprog_random_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_random_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_remove_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_sac_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_get_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_damage_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_repair_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_drop_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_examine_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_zap_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_pull_trigger(CHAR_DATA * ch, OBJ_DATA * obj);
void oprog_push_trigger(CHAR_DATA * ch, OBJ_DATA * obj);


/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG      -2

typedef enum
{
   ACT_PROG, SPEECH_PROG, RAND_PROG, FIGHT_PROG, DEATH_PROG, HITPRCNT_PROG,
   ENTRY_PROG, GREET_PROG, ALL_GREET_PROG, GIVE_PROG, BRIBE_PROG, HOUR_PROG,
   TIME_PROG, WEAR_PROG, REMOVE_PROG, SAC_PROG, LOOK_PROG, EXA_PROG, ZAP_PROG,
   GET_PROG, DROP_PROG, DAMAGE_PROG, REPAIR_PROG, RANDIW_PROG, SPEECHIW_PROG,
   PULL_PROG, PUSH_PROG, SLEEP_PROG, REST_PROG, LEAVE_PROG, SCRIPT_PROG,
   USE_PROG
}
prog_types;

/*
 * For backwards compatability
 */
#define RDEATH_PROG DEATH_PROG
#define ENTER_PROG  ENTRY_PROG
#define RFIGHT_PROG FIGHT_PROG
#define RGREET_PROG GREET_PROG
#define OGREET_PROG GREET_PROG

void rprog_leave_trigger(CHAR_DATA * ch);
void rprog_enter_trigger(CHAR_DATA * ch);
void rprog_sleep_trigger(CHAR_DATA * ch);
void rprog_rest_trigger(CHAR_DATA * ch);
void rprog_rfight_trigger(CHAR_DATA * ch);
void rprog_death_trigger(CHAR_DATA * killer, CHAR_DATA * ch);
void rprog_speech_trigger(char *txt, CHAR_DATA * ch);
void rprog_random_trigger(CHAR_DATA * ch);
void rprog_time_trigger(CHAR_DATA * ch);
void rprog_hour_trigger(CHAR_DATA * ch);
char *rprog_type_to_name(int type);

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger(char *buf, OBJ_DATA * mobj, CHAR_DATA * ch, OBJ_DATA * obj, void *vo);
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger(char *buf, ROOM_INDEX_DATA * room, CHAR_DATA * ch, OBJ_DATA * obj, void *vo);
#endif


/* Structure and macros for using long bit vectors */
#define CHAR_SIZE sizeof(char)

typedef char *LONG_VECTOR;

#define LV_CREATE(vector, bit_length)					\
do									\
{									\
	int i;								\
	CREATE(vector, char, 1 + bit_length/CHAR_SIZE);			\
									\
	for(i = 0; i <= bit_length/CHAR_SIZE; i++)			\
		*(vector + i) = 0;					\
}while(0)

#define LV_IS_SET(vector, index)					\
	(*(vector + index/CHAR_SIZE) & (1 << index%CHAR_SIZE))

#define LV_SET_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) |= (1 << index%CHAR_SIZE))

#define LV_REMOVE_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) &= ~(1 << index%CHAR_SIZE))

#define LV_TOGGLE_BIT(vector, index)					\
	(*(vector + index/CHAR_SIZE) ^= (1 << index%CHAR_SIZE))

/* mxp stuff - added by Nick Gammon - 18 June 2001 */

/*
To simply using MXP we'll use special tags where we want to use MXP tags
and then change them to <, > and & at the last moment.

  eg. MXP_BEG "send" MXP_END    becomes: <send>
      MXP_AMP "version;"        becomes: &version;

*/

/* strings */
#define MXP_BEG "\x03"    /* becomes < */
#define MXP_END "\x04"    /* becomes > */
#define MXP_AMP "\x05"    /* becomes & */
 
/* characters */
#define MXP_BEGc '\x03'    /* becomes < */
#define MXP_ENDc '\x04'    /* becomes > */
#define MXP_AMPc '\x05'    /* becomes & */

// constructs an MXP tag with < and > around it
#define MXPTAG(arg) MXP_BEG arg MXP_END

// All this MXPTAG crap seemed redundant, it now is all in one macro
#define MXPFTAG(arg, string, endarg)  MXP_BEG arg MXP_END string MXP_BEG endarg MXP_END

#define ESC "\x1B"  /* esc character */

#define MXPMODE(arg) ESC "[" #arg "z"

/* flags for show_list_to_char */

enum {
  eItemNothing,   /* item is not readily accessible */
  eItemGet,     /* item on ground */
  eItemDrop,    /* item in inventory */
  eItemBid     /* auction item */
  };

#define MXP_open 0   /* only MXP commands in the "open" category are allowed.  */
#define MXP_secure 1 /* all tags and commands in MXP are allowed within the line.  */
#define MXP_locked 2 /* no MXP or HTML commands are allowed in the line.  The line is not parsed for any tags at all.   */
#define MXP_reset 3  /* close all open tags */
#define MXP_secure_once 4  /* next tag is secure only */
#define MXP_perm_open 5   /* open mode until mode change  */
#define MXP_perm_secure 6 /* secure mode until mode change */
#define MXP_perm_locked 7 /* locked mode until mode change */

#ifdef WIN32
void gettimeofday(struct timeval *tv, struct timezone *tz);
void kill_timer();

/* directory scanning stuff */

typedef struct dirent
{
   char *d_name;
};

typedef struct
{
   HANDLE hDirectory;
   WIN32_FIND_DATA Win32FindData;
   struct dirent dirinfo;
   char sDirName[MAX_PATH];
} DIR;


DIR *opendir(char *sDirName);
struct dirent *readdir(DIR * dp);
void closedir(DIR * dp);

/* --------------- Stuff for Win32 services ------------------ */
/*

   NJG:

   When "exit" is called to handle an error condition, we really want to
   terminate the game thread, not the whole process.

 */

#define exit(arg) Win32_Exit(arg)
void Win32_Exit(int exit_code);

#endif



#define send_to_char send_to_char_color
#define send_to_pager send_to_pager_color

/* Hometown Code by Ivan, Adjusted by Xerves -- 4/23/99 */
struct hometown_type
{

   char *name;

   int recall;

   int death;

};



extern const struct hometown_type hometown_table[];

int get_hometown args((char *argument));

/* For Arena -- Xerves 6/4/99 Thanks LrdElder */
#define GET_BETTED_ON(ch)    ((ch)->pcdata->betted_on)
#define GET_BET_AMT(ch) ((ch)->pcdata->bet_amt)
#define IN_ARENA(ch)            (in_arena(ch))
