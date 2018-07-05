/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 * 			Database management module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
#include <dirent.h>
#else
#define strcasecmp strcmp
#endif
#include "mud.h"


extern int _filbuf args((FILE *));

#if defined(KEY)
#undef KEY
#endif

void init_supermob();

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}


/*
 * Globals.
 */

WIZENT *first_wiz;
WIZENT *last_wiz;

time_t last_restore_all_time = 0;

HELP_DATA *first_help;
HELP_DATA *last_help;

HINDEX_DATA *first_hindex;
HINDEX_DATA *last_hindex;

HINDEX_DATA *first_fhindex; //First flat hindex
HINDEX_DATA *last_fhindex; //Last flat hindex

SHOP_DATA *first_shop;
SHOP_DATA *last_shop;

REPAIR_DATA *first_repair;
REPAIR_DATA *last_repair;

TELEPORT_DATA *first_teleport;
TELEPORT_DATA *last_teleport;

PROJECT_DATA *first_project;
PROJECT_DATA *last_project;

AUTHORIZE_DATA *first_authorized;
AUTHORIZE_DATA *last_authorized;

FORGE_DATA *first_forge;
FORGE_DATA *last_forge;
SLAB_DATA *first_slab;
SLAB_DATA *last_slab;
WBLOCK_DATA *first_wblock;
WBLOCK_DATA *last_wblock;

QMOB_DATA *first_qmob;
QMOB_DATA *last_qmob;
QOBJ_DATA *first_qobj;
NPCRACE_DATA *first_npcrace;
NPCRACE_DATA *last_npcrace;
QOBJ_DATA *last_qobj;
QUEST_DATA *first_quest;
QUEST_DATA *last_quest;
TRAP_DATA *first_trap;
TRAP_DATA *last_trap;

BARENA_DATA *first_barena;
BARENA_DATA *last_barena;
BIN_DATA *first_bin;
BIN_DATA *last_bin;
CMEMBER_DATA *first_clanmember;
CMEMBER_DATA *last_clanmember;
KMEMBER_DATA *first_kingdommember;
KMEMBER_DATA *last_kingdommember;
TORNADO_DATA *first_tornado;
TORNADO_DATA *last_tornado;
TRAINER_DATA *first_trainer;
TRAINER_DATA *last_trainer;
CONQUER_DATA *first_conquer;
CONQUER_DATA *last_conquer;
GEM_DATA *first_gem;
GEM_DATA *last_gem;
AGGRO_DATA *first_global_aggro;
AGGRO_DATA *last_global_aggro;
TRADE_DATA *first_trade;
TRADE_DATA *last_trade;
BOX_DATA *first_box;
BOX_DATA *last_box;
TRAINING_DATA *first_training;
TRAINING_DATA *last_training;
CHANNEL_HISTORY *first_channelhistory;
CHANNEL_HISTORY *last_channelhistory;
SHIP_DATA *first_ship;
SHIP_DATA *last_ship;
MARKET_DATA *first_market;
MARKET_DATA *last_market;

OBJ_DATA *extracted_obj_queue;
EXTRACT_CHAR_DATA *extracted_char_queue;

char bug_buf[2 * MIL];
CHAR_DATA *first_char;
CHAR_DATA *last_char;

/* Below are used for Wilderness Mobs only, they belong to both lists, just makes it
   easier to search for them --Xerves 04/00*/
CMAP_DATA *first_wilderchar;
CMAP_DATA *last_wilderchar;
OMAP_DATA *first_wilderobj;
OMAP_DATA *last_wilderobj;
char *help_greeting;
char log_buf[2 * MIL];

OBJ_DATA *first_object;
OBJ_DATA *last_object;
TIME_INFO_DATA time_info;
int max_npc_race;

int rand_factor;
int climate_factor;
int max_vector;

int cur_qobjs;
int cur_qchars;
int nummobsloaded;
int numobjsloaded;
int physicalobjects;
int last_pkroom;
bool serial_list[MAX_LOADED_MOBS];
int serialmobsloaded; //not quite the same as nummobsloaded, it has the ability to use past serials for "reset mobs" to reset mobs properly.

MAP_INDEX_DATA *first_map; /* maps */

AUCTION_DATA *auction; /* auctions */

FILE *fpLOG;

//2.1 skills
sh_int gsn_combatart;
sh_int gsn_weapon_axe;
sh_int gsn_weapon_sword;
sh_int gsn_weapon_polearm;
sh_int gsn_weapon_blunt;
sh_int gsn_weapon_staff;
sh_int gsn_weapon_projectile;
sh_int gsn_weapon_dagger;
sh_int gsn_roar;
sh_int gsn_bash;
sh_int gsn_inhuman_strength;
sh_int gsn_krundo_style;
sh_int gsn_rwundo_style;
sh_int gsn_krundi_style;
sh_int gsn_rwundi_style;
sh_int gsn_pincer;
sh_int gsn_weaponbreak;
sh_int gsn_powerslice;
sh_int gsn_deshield;
sh_int gsn_perfect_shot;
sh_int gsn_drive;
sh_int gsn_insult;
sh_int gsn_draw_aggression;
sh_int gsn_greater_draw_aggression;
sh_int gsn_focus_aggression;
sh_int gsn_greater_focus_aggression;
sh_int gsn_weapon_twohanded;

sh_int gsn_escapism;
sh_int gsn_prawl;
sh_int gsn_nightprawl;
sh_int gsn_lightprawl;
sh_int gsn_shadowfoot;
sh_int gsn_strongfoot;
sh_int gsn_swimming;
sh_int gsn_retreat;
sh_int gsn_gag;
sh_int gsn_climbwall;
sh_int gsn_vanish;
sh_int gsn_begging;
sh_int gsn_thiefeye;
sh_int gsn_cutpurse;
sh_int gsn_grab;
sh_int gsn_haggling;
sh_int gsn_swindling;
sh_int gsn_assassinate;
sh_int gsn_forage;
sh_int gsn_kickdirt;
sh_int gsn_weapon_daggerstudy;
sh_int gsn_weapon_daggerstrike;
sh_int gsn_startfire;
sh_int gsn_featherfoot;
sh_int gsn_cleansing;
sh_int gsn_concentration;
sh_int gsn_manafuse;
sh_int gsn_fasting;
sh_int gsn_nervepinch;
sh_int gsn_featherback;
sh_int gsn_manashot;
sh_int gsn_manaburst;
sh_int gsn_quickcombo;
sh_int gsn_nervestrike;
sh_int gsn_battle_knowledge;
/* weaponry */
sh_int gsn_hit;

/* thief */
sh_int gsn_detrap;
sh_int gsn_backstab;
sh_int gsn_circle;
sh_int gsn_dodge;
sh_int gsn_hide;
sh_int gsn_peek;
sh_int gsn_pick_lock;
sh_int gsn_sneak;
sh_int gsn_steal;
sh_int gsn_gouge;
sh_int gsn_poison_weapon;
sh_int gsn_stalk;

/* thief & warrior */
sh_int gsn_disarm;
sh_int gsn_enhanced_damage;
sh_int gsn_parry;
sh_int gsn_rescue;
sh_int gsn_dual_wield;
sh_int gsn_stun;
sh_int gsn_daze;
sh_int gsn_bashdoor;
sh_int gsn_grip;
sh_int gsn_berserk;
sh_int gsn_hitall;
sh_int gsn_tumble;
sh_int gsn_kick_back;
sh_int gsn_deadly_accuracy;
sh_int gsn_attack_frenzy;
sh_int gsn_lore;
sh_int gsn_critical;
sh_int gsn_counter;
sh_int gsn_shieldblock;
sh_int gsn_repair;

/* other   */
sh_int gsn_unsheath;
sh_int gsn_aid;
sh_int gsn_track;
sh_int gsn_search;
sh_int gsn_dig;;
sh_int gsn_scribe;
sh_int gsn_brew;
sh_int gsn_climb;
sh_int gsn_cook;
sh_int gsn_scan;
sh_int gsn_slice;
sh_int gsn_mountain_climb;
sh_int gsn_study;
sh_int gsn_manatap;

/* Monk/Attack Skills */
sh_int gsn_roundhouse;
sh_int gsn_spinkick;
sh_int gsn_tornadokick;
sh_int gsn_niburo;
sh_int gsn_neckpinch;
sh_int gsn_neckchop;
sh_int gsn_neckrupture;
sh_int gsn_emeru;
sh_int gsn_elbowjab;
sh_int gsn_elbowstab;
sh_int gsn_elbowbreak;
sh_int gsn_amberio;
sh_int gsn_sidekick;
sh_int gsn_kneestrike;
sh_int gsn_kneecrusher;
sh_int gsn_lembecu;
sh_int gsn_blitz;
sh_int gsn_spear;
sh_int gsn_ribpuncture;
sh_int gsn_timmuru;

/* spells */
sh_int gsn_possess;
sh_int gsn_aqua_breath;
sh_int gsn_blindness;
sh_int gsn_charm_person;
sh_int gsn_curse;
sh_int gsn_invis;
sh_int gsn_mass_invis;
sh_int gsn_poison;
sh_int gsn_sleep;
sh_int gsn_wizardeye;
sh_int gsn_eye_of_god;
sh_int gsn_summon_corpse;
sh_int gsn_resurrection;
sh_int gsn_greater_resurrection;
sh_int gsn_lesser_resurrection;
sh_int gsn_web;
sh_int gsn_snare;
sh_int gsn_revitalize_spirit;
sh_int gsn_extradimensional_portal;
sh_int gsn_holy_cleansing;

//sh_int gsn_fireball;
//sh_int gsn_chill_touch;
//sh_int gsn_lightning_bolt;

/* languages */
sh_int gsn_common;
sh_int gsn_elven;
sh_int gsn_dwarven;
sh_int gsn_pixie;
sh_int gsn_ogre;
sh_int gsn_orcish;
sh_int gsn_trollish;
sh_int gsn_goblin;
sh_int gsn_halfling;

/* for searching */
sh_int gsn_first_spell;
sh_int gsn_first_skill;
sh_int gsn_first_weapon;
sh_int gsn_first_tongue;
sh_int gsn_top_sn;

//For Battle
sh_int gsn_balance;

/* For styles?  Trying to rebuild from some kind of accident here - Blod */
sh_int gsn_style_evasive;
sh_int gsn_style_divine;
sh_int gsn_style_wizardry;
sh_int gsn_style_defensive;
sh_int gsn_style_standard;
sh_int gsn_style_aggressive;
sh_int gsn_style_berserk;

/*
 * Locals.
 */
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];

AREA_DATA *first_area;
AREA_DATA *last_area;
AREA_DATA *first_area_name; /*Used for alphanum. sort */
AREA_DATA *last_area_name;
AREA_DATA *first_build;
AREA_DATA *last_build;
AREA_DATA *first_asort;
AREA_DATA *last_asort;
AREA_DATA *first_bsort;
AREA_DATA *last_bsort;

SYSTEM_DATA sysdata;

int top_affect;
int top_area;
int top_ed;
int top_exit;
int top_help;
int top_mob_index;
int top_obj_index;
int top_reset;
int top_room;
int top_shop;
int top_repair;
int top_vroom;
int top_map_mob;
sh_int res_rotation;

/*
 * Semi-locals.
 */
bool fBootDb;
FILE *fpArea;
char strArea[MIL];



/*
 * Local booting procedures.
 */
void init_mm args((void));

void load_maps args((void)); /* Load in Overland Maps - Samson 8-1-99 */
void boot_log args((const char *str, ...));
void load_area args((FILE * fp));
void load_area_kingdom args((AREA_DATA * tarea, FILE * fp));   //no longer used, left for old area files
void load_kowner args((AREA_DATA * tarea, FILE * fp));
void load_author args((AREA_DATA * tarea, FILE * fp));
void load_economy args((AREA_DATA * tarea, FILE * fp));
void load_resetmsg args((AREA_DATA * tarea, FILE * fp)); /* Rennard */
void load_flags args((AREA_DATA * tarea, FILE * fp));
void load_helps args((AREA_DATA * tarea, FILE * fp));
void load_mobiles args((AREA_DATA * tarea, FILE * fp));
void load_objects args((AREA_DATA * tarea, FILE * fp));
void load_projects args((void));
void load_resets args((AREA_DATA * tarea, FILE * fp));
void load_rooms args((AREA_DATA * tarea, FILE * fp));
void load_shops args((AREA_DATA * tarea, FILE * fp));
void load_repairs args((AREA_DATA * tarea, FILE * fp));
void load_specials args((AREA_DATA * tarea, FILE * fp));
void load_ranges args((AREA_DATA * tarea, FILE * fp));
void load_climate args((AREA_DATA * tarea, FILE * fp)); /* FB */
void load_buildlist args((void));
bool load_systemdata args((SYSTEM_DATA * sys));
void load_banlist args((void));
void load_version args((AREA_DATA * tarea, FILE * fp));
void load_watchlist args((void));
void load_reserved args((void));
void initialize_economy args((void));
void fix_exits args((void));
void sort_reserved args((RESERVE_DATA * pRes));
void init_area_weather args((void));
bool load_weatherdata args((void));
PROJECT_DATA *read_project args((char *filename, FILE * fp));
NOTE_DATA *read_log args((FILE * fp));
void save_sysdata args((SYSTEM_DATA sys));

/*
 * External booting function
 */
void load_corpses args((void));
void renumber_put_resets args((AREA_DATA * pArea));

/*
 * MUDprogram locals
 */

int mprog_name_to_type args((char *name));
MPROG_DATA *mprog_file_read args((char *f, MPROG_DATA * mprg, MOB_INDEX_DATA * pMobIndex));

/* int 		oprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *oprog_file_read args((char *f, MPROG_DATA * mprg, OBJ_INDEX_DATA * pObjIndex));

/* int 		rprog_name_to_type	args ( ( char* name ) ); */
MPROG_DATA *rprog_file_read args((char *f, MPROG_DATA * mprg, ROOM_INDEX_DATA * pRoomIndex));
void load_mudprogs args((AREA_DATA * tarea, FILE * fp));
void load_objprogs args((AREA_DATA * tarea, FILE * fp));
void load_roomprogs args((AREA_DATA * tarea, FILE * fp));
void mprog_read_programs args((FILE * fp, MOB_INDEX_DATA * pMobIndex));
void oprog_read_programs args((FILE * fp, OBJ_INDEX_DATA * pObjIndex));
void rprog_read_programs args((FILE * fp, ROOM_INDEX_DATA * pRoomIndex));


void shutdown_mud(char *reason)
{
   FILE *fp;

   if ((fp = fopen(SHUTDOWN_FILE, "a")) != NULL)
   {
      fprintf(fp, "%s\n", reason);
      fclose(fp);
   }
}


/*
 * Big mama top level function.
 */
void boot_db(bool fCopyOver)
{
   sh_int wear;
   AREA_DATA *tarea;
   int x;

   show_hash(32);
   unlink(BOOTLOG_FILE);
   boot_log("---------------------[ Boot Log ]--------------------");

   log_string("Loading commands");
   load_commands();

   log_string("Loading sysdata configuration...");

   /* default values */
   first_forge = NULL; 
   last_forge = NULL;
   first_slab = NULL;
   last_slab = NULL;
   sysdata.read_all_mail = LEVEL_STAFF;
   sysdata.read_mail_free = LEVEL_IMMORTAL;
   sysdata.write_mail_free = LEVEL_IMMORTAL;
   sysdata.take_others_mail = LEVEL_STAFF;
   sysdata.imc_mail_vnum = 0;
   sysdata.muse_level = LEVEL_HI_IMM;
   sysdata.gem_vnum = 0;
   sysdata.lastaccountreset = time(0);
   sysdata.max_accounts = 15;
   sysdata.max_account_changes = 15;
   sysdata.think_level = LEVEL_HIGOD;
   sysdata.build_level = LEVEL_IMM;
   sysdata.log_level = LEVEL_LOG;
   sysdata.level_modify_proto = LEVEL_HI_IMM;
   sysdata.level_override_private = LEVEL_STAFF;
   sysdata.level_mset_player = LEVEL_HI_IMM;
   sysdata.stun_plr_vs_plr = 65;
   sysdata.stun_regular = 15;
   sysdata.gouge_nontank = 0;
   sysdata.gouge_plr_vs_plr = 0;
   sysdata.bash_nontank = 0;
   sysdata.bash_plr_vs_plr = 0;   
   sysdata.timeout_login = 720; //3 minutes
   sysdata.timeout_notes = 2400; //10 minuts
   sysdata.timeout_idle = 4800; //20 minutes
   sysdata.dodge_mod = 2;
   sysdata.parry_mod = 2;
   sysdata.tumble_mod = 4;
   sysdata.top_pid = 0;
   sysdata.dam_plr_vs_plr = 100;
   sysdata.dam_plr_vs_mob = 100;
   sysdata.dam_mob_vs_plr = 100;
   sysdata.dam_mob_vs_mob = 100;
   sysdata.last_invtrap_uid = START_INV_TRAP;
   sysdata.last_trap_uid = 0;
   sysdata.level_getobjnotake = LEVEL_HI_IMM;
   sysdata.save_frequency = 20; /* minutes */
   sysdata.bestow_dif = 5;
   sysdata.check_imm_host = 1;
   sysdata.morph_opt = 1;
   sysdata.save_pets = 0;
   sysdata.firstrun = 1;
   sysdata.accountemail = 0;
   sysdata.resetgame = 0;
   sysdata.save_flags = SV_DEATH | SV_PASSCHG | SV_AUTO | SV_PUT | SV_DROP | SV_GIVE | SV_AUCTION | SV_ZAPDROP | SV_IDLE;
   if (!load_systemdata(&sysdata))
   {
      log_string("Not found.  Creating new configuration.");
      sysdata.alltimemax = 0;
      sysdata.mud_name = str_dup("(Name not set)");
   }
   if (sysdata.firstrun == 1) //Sets these times back if this is the first run
   {
      sysdata.lasttaxcheck = time(0);
      sysdata.lastpopcheck = time(0);
      sysdata.lastrescheck = time(0);
      sysdata.start_calender = time(0);
      sysdata.firstrun = 0;
      save_sysdata(sysdata);
   }


   log_string("Loading overland maps....");
   load_maps();

   log_string("Loading snow info....");
   load_snow();
   
   first_wblock = NULL;
   last_wblock = NULL;
   
   log_string("Loading WilderBlocks....");
   load_wblock_data();

   log_string("Loading socials");
   load_socials();

   log_string("Loading skill table");
   load_skill_table();
   sort_skill_table();

   gsn_first_spell = 0;
   gsn_first_skill = 0;
   gsn_first_weapon = 0;
   gsn_first_tongue = 0;
   gsn_top_sn = top_sn;

   for (x = 0; x < top_sn; x++)
      if (!gsn_first_spell && skill_table[x]->type == SKILL_SPELL)
         gsn_first_spell = x;
      else if (!gsn_first_skill && skill_table[x]->type == SKILL_SKILL)
         gsn_first_skill = x;
      else if (!gsn_first_weapon && skill_table[x]->type == SKILL_WEAPON)
         gsn_first_weapon = x;
      else if (!gsn_first_tongue && skill_table[x]->type == SKILL_TONGUE)
         gsn_first_tongue = x;
   
   remap_slot_numbers(); /* must be after the sort */
   log_string("Loading classes");
   load_classes();

   log_string("Loading races");
   load_races();

   //  log_string("Loading Hall of Fame");
   //  load_hall_of_fame();

   log_string("Loading herb table");
   load_herb_table();

   log_string("Loading tongues");
   load_tongues();

   log_string("Making wizlist");
   make_wizlist();
   
   log_string("Loading Forge Items");
   load_forge_data();
   log_string("Loading Slab Items");
   load_slab_data();

   log_string("Initializing request pipe");
   init_request_pipe();

   fBootDb = TRUE;

   nummobsloaded = 0;
   numobjsloaded = 0;
   serialmobsloaded = 0;
   physicalobjects = 0;
   max_npc_race = 0;
   global_drop_equip_message = 0;
   for (x = 0; x < MAX_LOADED_MOBS; x++)
      serial_list[x] = 0;
   sysdata.maxplayers = 0;
   first_object = NULL;
   last_object = NULL;
   first_char = NULL;
   last_char = NULL;
   first_ship = NULL;
   last_ship = NULL;
   first_market = NULL;
   last_market = NULL;
   first_wilderchar = NULL;
   last_wilderchar = NULL;
   first_area = NULL;
   first_area_name = NULL; /*Used for alphanum. sort */
   last_area_name = NULL;
   last_area = NULL;
   first_bin = NULL;
   last_bin = NULL;
   first_tornado = NULL;
   last_tornado = NULL;
   first_build = NULL;
   last_area = NULL;
   first_shop = NULL;
   last_shop = NULL;
   first_repair = NULL;
   first_conquer = NULL;
   last_conquer = NULL;
   last_repair = NULL;
   first_teleport = NULL;
   last_teleport = NULL;
   first_asort = NULL;
   last_asort = NULL;
   first_trainer = NULL;
   last_trainer = NULL;
   first_gem = NULL;
   last_gem = NULL;
   first_global_aggro = NULL;
   last_global_aggro = NULL;
   first_trade = NULL;
   last_trade = NULL;
   first_box = NULL;
   last_box = NULL;
   first_training = NULL;
   last_training = NULL;
   extracted_obj_queue = NULL;
   extracted_char_queue = NULL;
   cur_qobjs = 0;
   cur_qchars = 0;
   res_rotation = 1;
   cur_char = NULL;
   cur_obj = 0;
   cur_obj_serial = 0;
   cur_char_died = FALSE;
   cur_obj_extracted = FALSE;
   cur_room = NULL;
   quitting_char = NULL;
   loading_char = NULL;
   saving_char = NULL;
   last_pkroom = 1;
   immortal_host_start = NULL;
   immortal_host_end = NULL;
   first_ban_class = NULL;
   last_ban_class = NULL;
   first_ban_race = NULL;
   last_ban_race = NULL;
   first_ban = NULL;
   last_ban = NULL;
   first_qmob = NULL;
   last_qmob = NULL;
   first_qobj = NULL;
   last_qobj = NULL;
   first_npcrace = NULL;
   last_npcrace = NULL;
   first_quest = NULL;
   last_quest = NULL;
   first_trap = NULL;
   last_trap = NULL;

   CREATE(auction, AUCTION_DATA, 1);
   auction->item = NULL;
   auction->hist_timer = 0;
   for (x = 0; x < AUCTION_MEM; x++)
      auction->history[x] = NULL;

   rand_factor = 2;
   climate_factor = 1;

   for (wear = 0; wear < MAX_WEAR; wear++)
      for (x = 0; x < MAX_LAYERS; x++)
         save_equipment[wear][x] = NULL;



   /*
    * Init random number generator.
    */
   log_string("Initializing random number generator");
   init_mm();

   /*
    * Set time and weather.
    */
   {

      log_string("Setting time and weather");

      if (gethour() < 5)
         time_info.sunlight = SUN_DARK;
      else if (gethour() < 6)
         time_info.sunlight = SUN_RISE;
      else if (gethour() < 19)
         time_info.sunlight = SUN_LIGHT;
      else if (gethour() < 20)
         time_info.sunlight = SUN_SET;
      else
         time_info.sunlight = SUN_DARK;

      /*
         weather_info.change = 0;
         weather_info.mmhg = 960;
         if ( time_info.month >= 7 && time_info.month <=12 )
         weather_info.mmhg += number_range( 1, 50 );
         else
         weather_info.mmhg += number_range( 1, 80 );

         if ( weather_info.mmhg <=  980 ) weather_info.sky = SKY_LIGHTNING;
         else if ( weather_info.mmhg <= 1000 ) weather_info.sky = SKY_RAINING;
         else if ( weather_info.mmhg <= 1020 ) weather_info.sky = SKY_CLOUDY;
         else                                  weather_info.sky = SKY_CLOUDLESS;
       */
   }

   /*
    * Assign gsn's for skills which need them.
    */
   {
      log_string("Assigning gsn's");
      ASSIGN_GSN(gsn_combatart, "combat art");
      ASSIGN_GSN(gsn_weapon_axe, "axes");
      ASSIGN_GSN(gsn_weapon_sword, "swords");
      ASSIGN_GSN(gsn_weapon_polearm, "polearms");
      ASSIGN_GSN(gsn_weapon_blunt, "blunt");
      ASSIGN_GSN(gsn_weapon_staff, "staves");
      ASSIGN_GSN(gsn_weapon_twohanded, "twohanded");
      ASSIGN_GSN(gsn_weapon_projectile, "projectiles");
      ASSIGN_GSN(gsn_weapon_dagger, "daggers");     
      ASSIGN_GSN(gsn_roar, "roar");
      ASSIGN_GSN(gsn_battle_knowledge, "battle knowledge");
      ASSIGN_GSN(gsn_bash, "bash");
      ASSIGN_GSN(gsn_inhuman_strength, "inhuman strength");
      ASSIGN_GSN(gsn_krundo_style, "krundo style");
      ASSIGN_GSN(gsn_rwundo_style, "rwundo style");
      ASSIGN_GSN(gsn_krundi_style, "krundi style");
      ASSIGN_GSN(gsn_rwundi_style, "rwundi style");
      ASSIGN_GSN(gsn_pincer, "pincer");
      ASSIGN_GSN(gsn_weaponbreak, "weaponbreak");
      ASSIGN_GSN(gsn_powerslice, "powerslice");
      ASSIGN_GSN(gsn_deshield, "deshield");
      ASSIGN_GSN(gsn_perfect_shot, "perfect shot");
      ASSIGN_GSN(gsn_drive, "drive");
      ASSIGN_GSN(gsn_insult, "taunt");
      ASSIGN_GSN(gsn_draw_aggression, "draw aggression");
      ASSIGN_GSN(gsn_greater_draw_aggression, "greater draw aggression");
      ASSIGN_GSN(gsn_focus_aggression, "focus aggression");
      ASSIGN_GSN(gsn_greater_focus_aggression, "greater focus aggression");      
      ASSIGN_GSN(gsn_escapism, "escapism");
      ASSIGN_GSN(gsn_prawl, "prawl");
      ASSIGN_GSN(gsn_nightprawl, "nightprawl");
      ASSIGN_GSN(gsn_lightprawl, "lightprawl");
      ASSIGN_GSN(gsn_shadowfoot, "shadowfoot");
      ASSIGN_GSN(gsn_strongfoot, "strongfoot");
      ASSIGN_GSN(gsn_swimming, "swimming");
      ASSIGN_GSN(gsn_retreat, "retreat");
      ASSIGN_GSN(gsn_gag, "gag");
      ASSIGN_GSN(gsn_climbwall, "climbwall");
      ASSIGN_GSN(gsn_vanish, "vanish");
      ASSIGN_GSN(gsn_begging, "begging");
      ASSIGN_GSN(gsn_thiefeye, "thiefeye");
      ASSIGN_GSN(gsn_cutpurse, "cutpurse");
      ASSIGN_GSN(gsn_grab, "grab");
      ASSIGN_GSN(gsn_haggling, "haggling");
      ASSIGN_GSN(gsn_swindling, "swindling");
      ASSIGN_GSN(gsn_assassinate, "assassinate");
      ASSIGN_GSN(gsn_forage, "forage");
      ASSIGN_GSN(gsn_kickdirt, "kickdirt");
      ASSIGN_GSN(gsn_weapon_daggerstudy, "daggerstudy");
      ASSIGN_GSN(gsn_weapon_daggerstrike, "daggerstrike");
      ASSIGN_GSN(gsn_startfire, "startfire");
      ASSIGN_GSN(gsn_featherfoot, "featherfoot");
      ASSIGN_GSN(gsn_cleansing, "cleansing");
      ASSIGN_GSN(gsn_concentration, "concentration");
      ASSIGN_GSN(gsn_manafuse, "manafuse");
      ASSIGN_GSN(gsn_fasting, "fasting");
      ASSIGN_GSN(gsn_nervepinch, "nervepinch");
      ASSIGN_GSN(gsn_featherback, "featherback");
      ASSIGN_GSN(gsn_manashot, "manashot");
      ASSIGN_GSN(gsn_manaburst, "manaburst");
      ASSIGN_GSN(gsn_quickcombo, "quickcombo");
      ASSIGN_GSN(gsn_nervestrike, "nervestrike");
     
      ASSIGN_GSN(gsn_style_evasive, "evasive style");
      ASSIGN_GSN(gsn_style_divine, "divine style");
      ASSIGN_GSN(gsn_style_wizardry, "wizardry style");
      ASSIGN_GSN(gsn_style_defensive, "defensive style");
      ASSIGN_GSN(gsn_style_standard, "standard style");
      ASSIGN_GSN(gsn_style_aggressive, "aggressive style");
      ASSIGN_GSN(gsn_style_berserk, "berserk style");
      ASSIGN_GSN(gsn_hit, "hit");
      ASSIGN_GSN(gsn_shieldblock, "shieldblock");
      ASSIGN_GSN(gsn_repair, "repair");
      
      ASSIGN_GSN(gsn_roundhouse, "roundhouse");
      ASSIGN_GSN(gsn_spinkick, "spinkick");
      ASSIGN_GSN(gsn_tornadokick, "tornadokick");
      ASSIGN_GSN(gsn_niburo, "niburo");
      ASSIGN_GSN(gsn_neckpinch, "neckpinch");
      ASSIGN_GSN(gsn_neckchop, "neckchop");
      ASSIGN_GSN(gsn_neckrupture, "neckrupture");
      ASSIGN_GSN(gsn_emeru, "emeru");
      ASSIGN_GSN(gsn_elbowjab, "elbowjab");
      ASSIGN_GSN(gsn_elbowstab, "elbowstab");
      ASSIGN_GSN(gsn_elbowbreak, "elbowbreak");
      ASSIGN_GSN(gsn_amberio, "amberio");
      ASSIGN_GSN(gsn_sidekick, "sidekick");
      ASSIGN_GSN(gsn_kneestrike, "kneestrike");
      ASSIGN_GSN(gsn_kneecrusher, "kneecrusher");
      ASSIGN_GSN(gsn_lembecu, "lembecu");
      ASSIGN_GSN(gsn_blitz, "blitz");
      ASSIGN_GSN(gsn_spear, "spear");
      ASSIGN_GSN(gsn_ribpuncture, "ribpuncture");
      ASSIGN_GSN(gsn_timmuru, "timmuru");
      
      ASSIGN_GSN(gsn_balance, "balance");

      ASSIGN_GSN(gsn_unsheath, "unsheath");
      ASSIGN_GSN(gsn_detrap, "detrap");
      ASSIGN_GSN(gsn_backstab, "backstab");
      ASSIGN_GSN(gsn_circle, "circle");
      ASSIGN_GSN(gsn_tumble, "tumble");
      ASSIGN_GSN(gsn_dodge, "dodge");
      ASSIGN_GSN(gsn_hide, "hide");
      ASSIGN_GSN(gsn_stalk, "stalk");
      ASSIGN_GSN(gsn_peek, "peek");
      ASSIGN_GSN(gsn_pick_lock, "pick lock");
      ASSIGN_GSN(gsn_sneak, "sneak");
      ASSIGN_GSN(gsn_steal, "steal");
      ASSIGN_GSN(gsn_gouge, "gouge");
      ASSIGN_GSN(gsn_poison_weapon, "poison weapon");
      ASSIGN_GSN(gsn_disarm, "disarm");
      ASSIGN_GSN(gsn_enhanced_damage, "enhanced damage");
      ASSIGN_GSN(gsn_parry, "parry");
      ASSIGN_GSN(gsn_rescue, "rescue");
      ASSIGN_GSN(gsn_dual_wield, "dual wield");
      ASSIGN_GSN(gsn_stun, "stun");
      ASSIGN_GSN(gsn_daze, "daze");
      ASSIGN_GSN(gsn_bashdoor, "doorbash");
      ASSIGN_GSN(gsn_grip, "grip");
      ASSIGN_GSN(gsn_berserk, "berserk");
      ASSIGN_GSN(gsn_hitall, "hitall");
      ASSIGN_GSN(gsn_kick_back, "kickback");
      ASSIGN_GSN(gsn_deadly_accuracy, "deadly accuracy");
      ASSIGN_GSN(gsn_attack_frenzy, "attack frenzy");
      ASSIGN_GSN(gsn_lore, "lore");
      ASSIGN_GSN(gsn_critical, "critical strike");
      ASSIGN_GSN(gsn_counter, "counter attack");
      ASSIGN_GSN(gsn_aid, "aid");
      ASSIGN_GSN(gsn_track, "track");
      ASSIGN_GSN(gsn_search, "search");
      ASSIGN_GSN(gsn_manatap, "manatap");
      ASSIGN_GSN(gsn_dig, "dig");
      ASSIGN_GSN(gsn_scribe, "scribe");
      ASSIGN_GSN(gsn_brew, "brew");
      ASSIGN_GSN(gsn_climb, "climb");
      ASSIGN_GSN(gsn_cook, "cook");
      ASSIGN_GSN(gsn_scan, "scan");
      ASSIGN_GSN(gsn_slice, "slice");
      ASSIGN_GSN(gsn_summon_corpse, "summon corpse");
      ASSIGN_GSN(gsn_resurrection, "resurrection");
      ASSIGN_GSN(gsn_lesser_resurrection, "lesser resurrection");
      ASSIGN_GSN(gsn_greater_resurrection, "greater resurrection");
      ASSIGN_GSN(gsn_web, "web");
      ASSIGN_GSN(gsn_snare, "snare");
      ASSIGN_GSN(gsn_revitalize_spirit, "revitalize spirit");
      ASSIGN_GSN(gsn_holy_cleansing, "holy cleansing");
      ASSIGN_GSN(gsn_extradimensional_portal, "extradimensional portal");
     // ASSIGN_GSN(gsn_fireball, "fireball");
      ASSIGN_GSN(gsn_wizardeye, "wizardeye");
    //  ASSIGN_GSN(gsn_chill_touch, "chill touch");
    //  ASSIGN_GSN(gsn_lightning_bolt, "lightning bolt");
      ASSIGN_GSN(gsn_eye_of_god, "eye of god");
      ASSIGN_GSN(gsn_aqua_breath, "aqua breath");
      ASSIGN_GSN(gsn_possess, "possess");
      ASSIGN_GSN(gsn_blindness, "blindness");
      ASSIGN_GSN(gsn_charm_person, "charm person");
      ASSIGN_GSN(gsn_curse, "curse");
      ASSIGN_GSN(gsn_invis, "invis");
      ASSIGN_GSN(gsn_mass_invis, "mass invis");
      ASSIGN_GSN(gsn_poison, "poison");
      ASSIGN_GSN(gsn_mountain_climb, "mountain climbing");
      ASSIGN_GSN(gsn_study, "study");
      ASSIGN_GSN(gsn_sleep, "sleep");
      ASSIGN_GSN(gsn_common, "common");
      ASSIGN_GSN(gsn_elven, "elven");
      ASSIGN_GSN(gsn_dwarven, "dwarven");
      ASSIGN_GSN(gsn_pixie, "pixie");
      ASSIGN_GSN(gsn_ogre, "ogre");
      ASSIGN_GSN(gsn_orcish, "orcish");
      ASSIGN_GSN(gsn_trollish, "trollese");
      ASSIGN_GSN(gsn_goblin, "goblin");
      ASSIGN_GSN(gsn_halfling, "halfling");
   }

   log_string( "Loading DNS cache..." ); /* Samson 1-30-02 */
   load_dns();
   log_string("Reading in plane file...");
   load_planes();
   log_string("Loading in NPC Races...");
   load_npcrace_file();

   /*
    * Read in all the area files.
    */
   {
      FILE *fpList;

      log_string("Reading in area files...");
      if ((fpList = fopen(AREA_LIST, "r")) == NULL)
      {
         perror(AREA_LIST);
         shutdown_mud("Unable to open area list");
         exit(1);
      }

      for (;;)
      {
         strcpy(strArea, fread_word(fpList));
         if (strArea[0] == '$')
            break;

         load_area_file(last_area, strArea);

      }
      fclose(fpList);
   }
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (tarea->kingdom >= sysdata.max_kingdom || tarea->kingdom < 0)
      {
         tarea->kingdom = 0;
         tarea->kpid = 0;
         fold_area(tarea, tarea->filename, FALSE, 1);
      }         
      if (tarea->kpid == 0 && tarea->kingdom > 0)
      {
         tarea->kpid = kingdom_table[tarea->kingdom]->kpid;
         fold_area(tarea, tarea->filename, FALSE, 1);
      }
   }

   log_string("Making sure rooms are planed...");
   check_planes(NULL);
   /*
    *   initialize supermob.
    *    must be done before reset_area!
    *
    */
   init_supermob();
   /*
    * Has some bad memory bugs in it
    */

   /*
    * Fix up exits.
    * Declare db booting over.
    * Reset all areas once.
    * Load up the notes file.
    */
   {
      log_string("Fixing exits");
      fix_exits();
      fBootDb = FALSE;
      log_string("Initializing economy");
      initialize_economy();
      log_string("Loading Trap Data"); //Must be after the area content is loaded and before the resets are run
      load_trap_file(NULL);
      log_string("Resetting areas");
      area_update();
      log_string("Loading buildlist");
      load_buildlist();
      log_string("Loading kingdoms");
      load_kingdoms();
      log_string("Loading Portals");
      load_portal_file();
      log_string("Loading boards");
      load_boards();
      log_string("Loading Global Boards");
      load_global_boards();
      log_string("Loading Battle Descriptions");
      fread_battle_descriptions();
      log_string("Loading clans");
      load_clans();
      log_string("Loading councils");
      load_deity();
      log_string("Loading deities");
      load_councils();
      log_string("Loading watches");
      load_watchlist();
      log_string("Loading bans");
      load_banlist();
      log_string("Loading reserved names");
      load_reserved();
      log_string("Loading corpses");
      load_corpses();
      log_string("Loading Immortal Hosts");
      log_string("Loading slay table"); /* Online slay table - Samson 8-3-98 */
      load_slays();
      load_imm_host();
      log_string("Loading Projects");
      load_projects();
      log_string("Loading Conquer List");
      load_conquer_file();
      log_string("Loading Channel History");
      read_channelhistory_file();
      log_string("Loading Authorized List");
      load_authlist();
      log_string("Loading Trainers");
      load_trainer_data();
      log_string("Loading Treasure");
      load_gem_data();
      log_string("Loading Treasure Boxes");
      load_box_data();
      log_string("Loading Depositories");
      load_kingdom_depo();
      log_string("Loading Default Depositories");
      load_default_depo();
      log_string("Loading Battle Arena List");
      load_barena_data();
      log_string("Loading Wilderness Bin List");
      load_bin_data();
      log_string("Loading Trading List");
      load_trade_file();
      log_string("Loading Members List");
      scan_players();
      log_string("Loading Kingdom Chests");
      load_kchest_file();
      log_string("Loading Kingdom Military");
      load_mlist_data();
      log_string("Loading Extraction Mobiles");
      load_extraction_data();
      log_string("Loading Kingdom Buy List");
      load_buykingdom_data();
      log_string("Loading Training Data for Kingdoms");
      fread_training_list();
      log_string("Loading Quest Mobile List");
      load_qmob_data();
      log_string("Loading Quest Object List");
      load_qobj_data();
      log_string("Loading Quest Contents");
      load_quest_contents();
      log_string("Loading Quest Data");
      load_quest_data();
      log_string("Loading Ship Data");
      load_ship_data();
      log_string("Loading Market Data");
      load_market_data();
/* Morphs MUST be loaded after class and race tables are set up --Shaddai */
      log_string("Loading Morphs");
      load_morphs();
      MOBtrigger = TRUE;
      if (fCopyOver)
      {
         log_string("Running copyover_recover.");
         copyover_recover();
      }
   }

   /* Initialize area weather data */
   if (load_weatherdata() == FALSE)
      init_area_weather();

   /* init_maps ( ); */

   return;
}


/*
 * Load an 'area' header line.
 */
void load_area(FILE * fp)
{
   AREA_DATA *pArea;

   CREATE(pArea, AREA_DATA, 1);
   pArea->first_reset = NULL;
   pArea->last_reset = NULL;
   pArea->name = fread_string_nohash(fp);
   pArea->author = STRALLOC("unknown");
   pArea->filename = str_dup(strArea);
   pArea->age = 15;
   pArea->nplayer = 0;
   pArea->low_r_vnum = 0;
   pArea->low_o_vnum = 0;
   pArea->low_m_vnum = 0;
   pArea->hi_r_vnum = 0;
   pArea->hi_o_vnum = 0;
   pArea->hi_m_vnum = 0;
   pArea->low_soft_range = 0;
   pArea->hi_soft_range = MAX_LEVEL;
   pArea->low_hard_range = 0;
   pArea->hi_hard_range = MAX_LEVEL;
   pArea->kingdom = -1;

   area_version = 0;
   LINK(pArea, first_area, last_area, next, prev);
   top_area++;
   return;
}


/* Load the version number of the area file if none exists, then it
 * is set to version 0 when #AREA is read in which is why we check for
 * the #AREA here.  --Shaddai
 */

void load_version(AREA_DATA * tarea, FILE * fp)
{
   if (!tarea)
   {
      bug("Load_author: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   area_version = fread_number(fp);
   return;
}

/*
 * Load an author section. Scryn 2/1/96
 */
void load_author(AREA_DATA * tarea, FILE * fp)
{
   if (!tarea)
   {
      bug("Load_author: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   if (tarea->author)
      STRFREE(tarea->author);
   tarea->author = fread_string(fp);
   return;
}

void load_kowner(AREA_DATA * tarea, FILE * fp)
{
   if (!tarea)
   {
      bug("Load_kowner: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   if (tarea->kowner)
      STRFREE(tarea->kowner);
   tarea->kowner = fread_string(fp);
   return;
}

//no longer used, left for old area files
void load_area_kingdom(AREA_DATA * tarea, FILE * fp)
{
   int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12;
   char *ln;

   if (!tarea)
   {
      bug("Load_ranges: no #AREA seen yet.");
      shutdown_mud("No #AREA");
      exit(1);
   }

   for (;;)
   {
      ln = fread_line(fp);

      if (ln[0] == '$')
         break;

      x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = 0;
      if (area_version < 41)
         sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11);
      else
         sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12);
   }
   tarea->res_tree = x1;
   tarea->res_corn = x2;
   tarea->res_grain = x3;
   tarea->res_iron = x4;
   tarea->res_gold = x5;
   tarea->res_stone = x6;
   tarea->gold = x7;
   tarea->salestax = x8;
   tarea->poptax = x9;
   tarea->death = x10;
   tarea->recall = x11;
   tarea->lasttaxchange = x12;

   if (tarea->recall <= 0)
      tarea->recall = ROOM_VNUM_TEMPLE;
   if (tarea->death <= 0)
      tarea->death = ROOM_VNUM_ALTAR;


   return;
}

/*
 * Load an economy section. Thoric
 */
void load_economy(AREA_DATA * tarea, FILE * fp)
{
   if (!tarea)
   {
      bug("Load_economy: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   tarea->high_economy = fread_number(fp);
   tarea->low_economy = fread_number(fp);
   if (area_version >= 25)
      tarea->population = fread_number(fp);
   return;
}

/* Reset Message Load, Rennard */
void load_resetmsg(AREA_DATA * tarea, FILE * fp)
{
   if (!tarea)
   {
      bug("Load_resetmsg: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   if (tarea->resetmsg)
      DISPOSE(tarea->resetmsg);
   tarea->resetmsg = fread_string_nohash(fp);
   return;
}

/*
 * Load area flags. Narn, Mar/96 
 */
void load_flags(AREA_DATA * tarea, FILE * fp)
{
   char *ln;
   int x1, x2;

   if (!tarea)
   {
      bug("Load_flags: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }
   ln = fread_line(fp);
   x1 = x2 = 0;
   sscanf(ln, "%d %d", &x1, &x2);
   tarea->flags = x1;
   tarea->reset_frequency = x2;
   if (x2)
      tarea->age = x2;
   return;
}

/*
 * Adds a help page to the list if it is not a duplicate of an existing page.
 * Page is insert-sorted by keyword.			-Thoric
 * (The reason for sorting is to keep do_hlist looking nice)
 */
void add_help(HELP_DATA * pHelp)
{
   HELP_DATA *tHelp;
   int match;

   for (tHelp = first_help; tHelp; tHelp = tHelp->next)
      if (pHelp->level == tHelp->level && strcmp(pHelp->keyword, tHelp->keyword) == 0)
      {
         bug("add_help: duplicate: %s.  Deleting.", pHelp->keyword);
         STRFREE(pHelp->text);
         STRFREE(pHelp->keyword);
         DISPOSE(pHelp);
         return;
      }
      else
         if ((match = strcmp(pHelp->keyword[0] == '\'' ? pHelp->keyword + 1 : pHelp->keyword,
             tHelp->keyword[0] == '\'' ? tHelp->keyword + 1 : tHelp->keyword)) < 0 || (match == 0 && pHelp->level > tHelp->level))
      {
         if (!tHelp->prev)
            first_help = pHelp;
         else
            tHelp->prev->next = pHelp;
         pHelp->prev = tHelp->prev;
         pHelp->next = tHelp;
         tHelp->prev = pHelp;
         break;
      }

   if (!tHelp)
      LINK(pHelp, first_help, last_help, next, prev);

   top_help++;
}

int parse_helpfile_index(FILE *fp, HELP_DATA *pHelp, char *argument)
{
   HINDEX_DATA * hindex = NULL;
   HINDEX_DATA * hindex2;
   HINDEX_DATA *findex;
   HINDEX_NAME * hiname;
   HINDEX_POINTER *hpointer;
   HINDEX_IPOINTER *hipointer;
   HELP_DATA *help;
   char keyword[100];
   char fullkeyword[200];
   int value = 0;
   char *keystring;
   
   //if there is a file pointer, then it is being read from file, if not, it is being used in hset
   if (fp)
   {
      keystring = fread_string(fp);
      if (keystring[0] != '#')
         return 1;
      keystring++; //advance past the #
      sprintf(fullkeyword, "%s", keystring);
      keystring = one_argument(keystring, keyword);
      if (!str_cmp(keyword, "END"))
         return 0;
   }
   else
   {
      sprintf(fullkeyword, argument);
      keystring = argument;
      keystring = one_argument(keystring, keyword);
   }
   CREATE(hiname, HINDEX_NAME, 1);
   hiname->name = STRALLOC(fullkeyword);
   LINK(hiname, pHelp->first_iname, pHelp->last_iname, next, prev);
   
   if (!first_hindex)
   {
      CREATE(hindex, HINDEX_DATA, 1);
      hindex->keyword = STRALLOC(keyword);
      hindex->first_help = NULL;
      hindex->last_help = NULL;
      hindex->first_hindex = NULL;
      hindex->last_hindex = NULL;
      LINK(hindex, first_hindex, last_hindex, next, prev);
      LINK(hindex, first_fhindex, last_fhindex, fnext, fprev);
   }
   if (keystring[0] == '\0') //Only was one keyword, no need to start looping through the keywords
   {
      if (!hindex)
      {
         for (hindex = first_hindex; hindex; hindex = hindex->next)
         {
            if (!str_cmp(hindex->keyword, keyword))
               break;
         }
         if (!hindex)
         {
            for (findex = first_fhindex; findex; findex = findex->fnext)
            {
               if (!str_cmp(findex->keyword, keyword))
                  break;
            }
            if (!findex)
            {    
               CREATE(hindex, HINDEX_DATA, 1);
               hindex->keyword = STRALLOC(keyword);
               hindex->first_help = NULL;
               hindex->last_help = NULL;
               hindex->first_hindex = NULL;
               hindex->last_hindex = NULL;
            }
            else
               hindex = findex;
            LINK(hindex, first_hindex, last_hindex, next, prev);  
            LINK(hindex, first_fhindex, last_fhindex, fnext, fprev);
         }
      }  
      if (!hindex->first_help)
      {
         CREATE(hipointer, HINDEX_IPOINTER, 1);
         hipointer->pointer = pHelp;
         LINK(hipointer, hindex->first_help, hindex->last_help, next, prev);
         CREATE(hpointer, HINDEX_POINTER, 1);
         hpointer->pointer = hindex;
         LINK(hpointer, pHelp->first_hindex, pHelp->last_hindex, next, prev);
      }
      else
      {
         for (hipointer = hindex->first_help; hipointer; hipointer = hipointer->next)
         {
            help = hipointer->pointer;
            if (!str_cmp(pHelp->keyword, help->keyword)) // Same helpfile
               break;
         }
         if (!hipointer)
         {
            CREATE(hipointer, HINDEX_IPOINTER, 1);
            hipointer->pointer = pHelp;
            LINK(hipointer, hindex->first_help, hindex->last_help, next, prev);
            CREATE(hpointer, HINDEX_POINTER, 1);
            hpointer->pointer = hindex;
            LINK(hpointer, pHelp->first_hindex, pHelp->last_hindex, next, prev);
         }
      } 
      if (argument)
         return 1;
      //another round...
      value = parse_helpfile_index(fp, pHelp, NULL);
         return value; //once it reaches a value it will loop back out and return that value
   }
   //Will never reach this loop unless there are more than 1 keyword.....
   for (hindex = first_hindex; hindex; hindex = hindex->next)
   {
      if (!hindex->next && str_cmp(keyword, hindex->keyword)) //don't want to break out of this loop just yet...
      {
         for (findex = first_fhindex; findex; findex = findex->fnext)
         {
            if (!str_cmp(findex->keyword, keyword))
               break;
         }
         if (!findex)
         {
            CREATE(hindex2, HINDEX_DATA, 1);
            hindex2->keyword = STRALLOC(keyword);
            hindex2->first_help = NULL;
            hindex2->last_help = NULL;
            hindex2->first_hindex = NULL;
            hindex2->last_hindex = NULL;
            hindex2->first_top_hindex = NULL;
            hindex2->last_top_hindex = NULL;
         }
         else
            hindex2 = findex;
         LINK(hindex2, first_hindex, last_hindex, next, prev);
         LINK(hindex2, first_fhindex, last_fhindex, fnext, fprev);
         hindex = hindex2;
      }  
      if (!str_cmp(keyword, hindex->keyword))
      {             
         for (;;)
         {
            keystring = one_argument(keystring, keyword);
            if (keyword[0] == '\0')
            {
               if (!hindex->first_help)
               {
                  CREATE(hipointer, HINDEX_IPOINTER, 1);
                  hipointer->pointer = pHelp;
                  LINK(hipointer, hindex->first_help, hindex->last_help, next, prev);
                  CREATE(hpointer, HINDEX_POINTER, 1);
                  hpointer->pointer = hindex;
                  LINK(hpointer, pHelp->first_hindex, pHelp->last_hindex, next, prev);
               }
               else
               {
                  for (hipointer = hindex->first_help; hipointer; hipointer = hipointer->next)
                  {
                     help = hipointer->pointer;
                     if (!str_cmp(pHelp->keyword, help->keyword)) // Same helpfile
                     {
                        break;
                     }
                  }
                  if (!hipointer)
                  {
                     CREATE(hipointer, HINDEX_IPOINTER, 1);
                     hipointer->pointer = pHelp;
                     LINK(hipointer, hindex->first_help, hindex->last_help, next, prev);
                     CREATE(hpointer, HINDEX_POINTER, 1);
                     hpointer->pointer = hindex;
                     LINK(hpointer, pHelp->first_hindex, pHelp->last_hindex, next, prev);
                  }
               }       
               break;
            }   
            if (!hindex->first_hindex)
            {
               for (findex = first_fhindex; findex; findex = findex->fnext)
               {
                  if (!str_cmp(findex->keyword, keyword))
                     break;
               }
               if (!findex)
               {
                  CREATE(hindex2, HINDEX_DATA, 1);
                  hindex2->keyword = STRALLOC(keyword);
                  hindex2->first_help = NULL;
                  hindex2->last_help = NULL;
                  hindex2->first_hindex = NULL;
                  hindex2->last_hindex = NULL;
                  hindex2->first_top_hindex = NULL;
                  hindex2->last_top_hindex = NULL;
               }
               else
                  hindex2 = findex;
               LINK(hindex2, hindex->first_hindex, hindex->last_hindex, next, prev);
               LINK(hindex, hindex2->first_top_hindex, hindex2->last_top_hindex, tnext, tprev);
               LINK(hindex2, first_fhindex, last_fhindex, fnext, fprev);
            }
            for (hindex2 = hindex->first_hindex; hindex2; hindex2 = hindex2->next)
            {
               if (!str_cmp(keyword, hindex2->keyword))
                  break;
            }
            if (!hindex2)
            {
               for (findex = first_fhindex; findex; findex = findex->fnext)
               {
                  if (!str_cmp(findex->keyword, keyword))
                     break;
               }
               if (!findex)
               {
                  CREATE(hindex2, HINDEX_DATA, 1);
                  hindex2->keyword = STRALLOC(keyword);
                  hindex2->first_help = NULL;
                  hindex2->last_help = NULL;
                  hindex2->first_hindex = NULL;
                  hindex2->last_hindex = NULL;
                  hindex2->first_top_hindex = NULL;
                  hindex2->last_top_hindex = NULL;
               }
               else
                  hindex2 = findex;
               LINK(hindex2, hindex->first_hindex, hindex->last_hindex, next, prev);
               LINK(hindex, hindex2->first_top_hindex, hindex2->last_top_hindex, tnext, tprev);
               LINK(hindex2, first_fhindex, last_fhindex, fnext, fprev);
            }
            hindex = hindex2; //So this can go in an infinite loop theoretically
         }
         break;  
      } 
   }
   //another round...
   if (argument)
      return 1;
   value = parse_helpfile_index(fp, pHelp, NULL);
      return value; //once it reaches a value it will loop back out and return that value
}
/*
 * Load a help section.
 */
void load_helps(AREA_DATA * tarea, FILE * fp)
{
   HELP_DATA *pHelp;

   for (;;)
   {
      CREATE(pHelp, HELP_DATA, 1);
      pHelp->first_hindex = NULL;
      pHelp->last_hindex = NULL;
      pHelp->level = fread_number(fp);
      pHelp->keyword = fread_string(fp);
      pHelp->first_iname = NULL;
      pHelp->last_hindex = NULL;
      if (pHelp->keyword[0] == '$')
      {
         STRFREE(pHelp->keyword);
         DISPOSE(pHelp);
         break;
      }
      //Parses and creates the helpfile index
      if (parse_helpfile_index(fp, pHelp, NULL) == 1) 
      {
         bug("%s has no hindex, exiting!!!!!", pHelp->keyword);
         STRFREE(pHelp->keyword);
         DISPOSE(pHelp);
         exit(0);
      }  
      pHelp->text = fread_string(fp);
      if (pHelp->keyword[0] == '\0')
      {
         STRFREE(pHelp->text);
         STRFREE(pHelp->keyword);
         DISPOSE(pHelp);
         continue;
      }

      if (!str_cmp(pHelp->keyword, "greeting"))
         help_greeting = pHelp->text;
      add_help(pHelp);
   }
   return;
}


/*
 * Add a character to the list of all characters		-Thoric
 */
void add_char(CHAR_DATA * ch)
{
   //   CMAP_DATA *mch;
   LINK(ch, first_char, last_char, next, prev);
   /* if (IS_NPC(ch) && (ch->pIndexData->vnum >= OVERLAND_LOW_MOB && ch->pIndexData->vnum <= OVERLAND_HI_MOB))
      {
      CREATE(mch, CMAP_DATA, 1);
      mch->mapch = ch;
      LINK( mch, first_wilderchar, last_wilderchar, next, prev );
      top_map_mob++;
      }  */
}


/*
 * Load a mob section.
 */
void load_mobiles(AREA_DATA * tarea, FILE * fp)
{
   MOB_INDEX_DATA *pMobIndex;
   char *ln;
   int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
   int dummy;
   int toss;

   if (!tarea)
   {
      bug("Load_mobiles: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   for (;;)
   {
      char buf[MSL];
      sh_int vnum;
      char letter;
      int iHash;
      bool oldmob;
      bool tmpBootDb;

      letter = fread_letter(fp);
      if (letter != '#')
      {
         bug("Load_mobiles: # not found.");
         if (fBootDb)
         {
            shutdown_mud("# not found");
            exit(1);
         }
         else
            return;
      }

      vnum = fread_number(fp);
      if (vnum == 0)
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if (get_mob_index(vnum))
      {
         if (tmpBootDb)
         {
            bug("Load_mobiles: vnum %d duplicated.", vnum);
            shutdown_mud("duplicate vnum");
            exit(1);
         }
         else
         {
            pMobIndex = get_mob_index(vnum);
            sprintf(buf, "Cleaning mobile: %d", vnum);
            log_string_plus(buf, LOG_BUILD, sysdata.log_level);
            clean_mob(pMobIndex);
            oldmob = TRUE;
         }
      }
      else
      {
         oldmob = FALSE;
         CREATE(pMobIndex, MOB_INDEX_DATA, 1);
      }
      fBootDb = tmpBootDb;

      pMobIndex->vnum = vnum;
      if (fBootDb)
      {
         if (!tarea->low_m_vnum)
            tarea->low_m_vnum = vnum;
         if (vnum > tarea->hi_m_vnum)
            tarea->hi_m_vnum = vnum;
      }
      pMobIndex->player_name = fread_string(fp);
      pMobIndex->short_descr = fread_string(fp);
      pMobIndex->long_descr = fread_string(fp);
      pMobIndex->description = fread_string(fp);

      pMobIndex->long_descr[0] = UPPER(pMobIndex->long_descr[0]);
      pMobIndex->description[0] = UPPER(pMobIndex->description[0]);

      pMobIndex->act = fread_bitvector(fp);
      xSET_BIT(pMobIndex->act, ACT_IS_NPC);
      if (area_version >= 27)
         pMobIndex->miflags = fread_bitvector(fp);
      pMobIndex->affected_by = fread_bitvector(fp);
      pMobIndex->pShop = NULL;
      pMobIndex->rShop = NULL;
      pMobIndex->alignment = fread_number(fp);
      letter = fread_letter(fp);
      if (area_version < 32 || area_version > 33)
      {
         x1 = fread_number(fp);
         x1 = fread_number(fp);
         x1 = 0;
         pMobIndex->ac = fread_number(fp);
      }
      if (area_version >= 36)
      {
         pMobIndex->tohitbash = fread_number(fp);
         pMobIndex->tohitslash = fread_number(fp);
         pMobIndex->tohitstab = fread_number(fp);
      }
      pMobIndex->hitnodice = fread_number(fp);
      /* 'd'  */ fread_letter(fp);
      pMobIndex->hitsizedice = fread_number(fp);
      /* '+'  */ fread_letter(fp);
      pMobIndex->hitplus = fread_number(fp);
      pMobIndex->damnodice = fread_number(fp);
      /* 'd'  */ fread_letter(fp);
      pMobIndex->damsizedice = fread_number(fp);
      /* '+'  */ fread_letter(fp);
      pMobIndex->damplus = fread_number(fp);
      if (area_version > 17)
         toss = fread_number(fp);
      if (area_version >= 52)
      {
         pMobIndex->damaddlow = fread_number(fp);
         pMobIndex->damaddhi = fread_number(fp);
      }
      pMobIndex->gold = fread_number(fp);
      x1 = fread_number(fp); //use to be xp
      x1 = 0;

      /* pMobIndex->position  = fread_number( fp ); */
      pMobIndex->position = fread_number(fp);
      if (pMobIndex->position < 100)
      {
         switch (pMobIndex->position)
         {
            default:
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
               break;
            case 5:
               pMobIndex->position = 6;
               break;
            case 6:
               pMobIndex->position = 8;
               break;
            case 7:
               pMobIndex->position = 9;
               break;
            case 8:
               pMobIndex->position = 12;
               break;
            case 9:
               pMobIndex->position = 13;
               break;
            case 10:
               pMobIndex->position = 14;
               break;
            case 11:
               pMobIndex->position = 15;
               break;
         }
      }
      else
      {
         pMobIndex->position -= 100;
      }

      /* pMobIndex->defposition  = fread_number( fp ); */
      pMobIndex->defposition = fread_number(fp);
      if (pMobIndex->defposition < 100)
      {
         switch (pMobIndex->defposition)
         {
            default:
            case 0:
            case 1:
            case 2:
            case 3:
            case 4:
               break;
            case 5:
               pMobIndex->defposition = 6;
               break;
            case 6:
               pMobIndex->defposition = 8;
               break;
            case 7:
               pMobIndex->defposition = 9;
               break;
            case 8:
               pMobIndex->defposition = 12;
               break;
            case 9:
               pMobIndex->defposition = 13;
               break;
            case 10:
               pMobIndex->defposition = 14;
               break;
            case 11:
               pMobIndex->defposition = 15;
               break;
         }
      }
      else
      {
         pMobIndex->defposition -= 100;
      }


      /*
       * Back to meaningful values.
       */
      pMobIndex->sex = fread_number(fp);

      if (letter != 'S' && letter != 'C')
      {
         bug("Load_mobiles: vnum %d: letter '%c' not S or C.", vnum, letter);
         shutdown_mud("bad mob data");
         exit(1);
      }
      if (letter == 'C') /* Realms complex mob  -Thoric */
      {
         pMobIndex->perm_str = fread_number(fp);
         pMobIndex->perm_int = fread_number(fp);
         pMobIndex->perm_wis = fread_number(fp);
         pMobIndex->perm_dex = fread_number(fp);
         pMobIndex->perm_con = fread_number(fp);
         pMobIndex->perm_cha = fread_number(fp);
         pMobIndex->perm_lck = fread_number(fp);
         if (area_version >= 37)
            pMobIndex->perm_agi = fread_number(fp);
         pMobIndex->saving_poison_death = fread_number(fp);
         pMobIndex->saving_wand = fread_number(fp);
         pMobIndex->saving_para_petri = fread_number(fp);
         pMobIndex->saving_breath = fread_number(fp);
         pMobIndex->saving_spell_staff = fread_number(fp);
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
         sscanf(ln, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7);
         pMobIndex->race = x1;
 //      class was set to 0
         pMobIndex->height = x3;
         pMobIndex->weight = x4;
         pMobIndex->speaks = 0;
         pMobIndex->speaking = 0;
         pMobIndex->perm_agi = x7;
         if (area_version >= 12)
         {
            pMobIndex->m1 = fread_number(fp);
            pMobIndex->m2 = fread_number(fp);
            pMobIndex->m3 = fread_number(fp);
            pMobIndex->m4 = fread_number(fp);
            pMobIndex->m5 = fread_number(fp);
         }
         if (area_version >= 13)
         {
            pMobIndex->m6 = fread_number(fp);
         }
         if (area_version >= 26)
         {
            pMobIndex->m7 = fread_number(fp);
            pMobIndex->m8 = fread_number(fp);
            pMobIndex->m9 = fread_number(fp);
         }
         if (area_version >= 15)
         {
            pMobIndex->cident = fread_number(fp);
         }
         if (area_version >= 45)
         {
            pMobIndex->m10 = fread_number(fp);
            pMobIndex->m11 = fread_number(fp);
            pMobIndex->m12 = fread_number(fp);
         }
         if (area_version >= 50)
         {
            ln = fread_line(fp);
            x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = x15 = 0;
            sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7,
               &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15);
            pMobIndex->apply_res_fire = (x1 == 0 ? 100 : x1);
            pMobIndex->apply_res_water = (x2 == 0 ? 100 : x2);
            pMobIndex->apply_res_air = (x3 == 0 ? 100 : x3);
            pMobIndex->apply_res_earth = (x4 == 0 ? 100 : x4);
            pMobIndex->apply_res_energy = (x5 == 0 ? 100 : x5);
            pMobIndex->apply_res_magic = (x6 == 0 ? 100 : x6);
            pMobIndex->apply_res_nonmagic = (x7 == 0 ? 100 : x7);
            pMobIndex->apply_res_blunt = (x8 == 0 ? 100 : x8);
            pMobIndex->apply_res_pierce = (x9 == 0 ? 100 : x9);
            pMobIndex->apply_res_slash = (x10 == 0 ? 100 : x10);
            pMobIndex->apply_res_poison = (x11 == 0 ? 100 : x11);
            pMobIndex->apply_res_paralysis = (x12 == 0 ? 100 : x12);
            pMobIndex->apply_res_holy = (x13 == 0 ? 100 : x13);
            pMobIndex->apply_res_unholy = (x14 == 0 ? 100 : x14);
            pMobIndex->apply_res_undead = (x15 == 0 ? 100 : x15);
         }
         

         if (!pMobIndex->speaks)
            pMobIndex->speaks = LANG_COMMON;
         if (!pMobIndex->speaking)
            pMobIndex->speaking = LANG_COMMON;

#ifndef XBI
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
         if (area_version >= 29)
            sscanf(ln, "%d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9);
         else
            sscanf(ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8);

         pMobIndex->hitroll = x1;
         pMobIndex->damroll = x2;
         pMobIndex->xflags = x3;
         pMobIndex->resistant = x4;
         pMobIndex->immune = x5;
         pMobIndex->susceptible = x6;
         if (area_version >= 29)
         {
            pMobIndex->elementb = x7;
            pMobIndex->attacks = x8;
            pMobIndex->defenses = x9;
         }
         else
         {
            pMobIndex->attacks = x7;
            pMobIndex->defenses = x8;
         }
#else
         pMobIndex->hitroll = fread_number(fp);
         pMobIndex->damroll = fread_number(fp);
         pMobIndex->xflags = fread_number(fp);
         pMobIndex->resistant = fread_number(fp);
         pMobIndex->immune = fread_number(fp);
         pMobIndex->susceptible = fread_number(fp);
         if (area_version >= 29)
         {
            pMobIndex->elementb = fread_number(fp);
            pMobIndex->attacks = fread_bitvector(fp);
            pMobIndex->defenses = fread_bitvector(fp);
         }
         else
         {
            pMobIndex->attacks = fread_bitvector(fp);
            pMobIndex->defenses = fread_bitvector(fp);
         }
#endif
      }
      else
      {
         pMobIndex->perm_str = 13;
         pMobIndex->perm_dex = 13;
         pMobIndex->perm_int = 13;
         pMobIndex->perm_wis = 13;
         pMobIndex->perm_cha = 13;
         pMobIndex->perm_con = 13;
         pMobIndex->perm_lck = 13;
         pMobIndex->perm_agi = 15;
         pMobIndex->race = 0;
         pMobIndex->xflags = 0;
         pMobIndex->resistant = 0;
         pMobIndex->immune = 0;
         pMobIndex->susceptible = 0;
#ifdef XBI
         xCLEAR_BITS(pMobIndex->attacks);
         xCLEAR_BITS(pMobIndex->defenses);
#else
         pMobIndex->attacks = 0;
         pMobIndex->defenses = 0;
#endif
      }
      letter = fread_letter(fp);
      if (letter == 'M')
         dummy = fread_number(fp);
      else
      {
         dummy = MAX_MOB_COUNT;
         ungetc(letter, fp);
      }
      letter = fread_letter(fp);
      if (letter == '>')
      {
         ungetc(letter, fp);
         mprog_read_programs(fp, pMobIndex);
      }
      else
         ungetc(letter, fp);

      if (!oldmob)
      {
         iHash = vnum % MAX_KEY_HASH;
         pMobIndex->next = mob_index_hash[iHash];
         mob_index_hash[iHash] = pMobIndex;
         top_mob_index++;
      }
   }

   return;
}


/*
 * Load an obj section.
 */
void load_objects(AREA_DATA * tarea, FILE * fp)
{
   OBJ_INDEX_DATA *pObjIndex;
   char letter;
   char *ln;
   int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14;

   if (!tarea)
   {
      bug("Load_objects: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   for (;;)
   {
      char buf[MSL];
      int vnum;
      int iHash;
      bool tmpBootDb;
      bool oldobj;

      letter = fread_letter(fp);
      if (letter != '#')
      {
         bug("Load_objects: # not found.");
         if (fBootDb)
         {
            shutdown_mud("# not found");
            exit(1);
         }
         else
            return;
      }

      vnum = fread_number(fp);
      if (vnum == 0)
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if (get_obj_index(vnum))
      {
         if (tmpBootDb)
         {
            bug("Load_objects: vnum %d duplicated.", vnum);
            shutdown_mud("duplicate vnum");
            exit(1);
         }
         else
         {
            pObjIndex = get_obj_index(vnum);
            sprintf(buf, "Cleaning object: %d", vnum);
            log_string_plus(buf, LOG_BUILD, sysdata.log_level);
            clean_obj(pObjIndex);
            oldobj = TRUE;
         }
      }
      else
      {
         oldobj = FALSE;
         CREATE(pObjIndex, OBJ_INDEX_DATA, 1);
      }
      fBootDb = tmpBootDb;

      pObjIndex->vnum = vnum;
      if (fBootDb)
      {
         if (!tarea->low_o_vnum)
            tarea->low_o_vnum = vnum;
         if (vnum > tarea->hi_o_vnum)
            tarea->hi_o_vnum = vnum;
      }
      pObjIndex->name = fread_string(fp);
      pObjIndex->short_descr = fread_string(fp);
      pObjIndex->description = fread_string(fp);
      pObjIndex->action_desc = fread_string(fp);

      /* Commented out by Narn, Apr/96 to allow item short descs like
         Bonecrusher and Oblivion */
      /*pObjIndex->short_descr[0] = LOWER(pObjIndex->short_descr[0]); */
      pObjIndex->description[0] = UPPER(pObjIndex->description[0]);

      pObjIndex->item_type = fread_number(fp);
      pObjIndex->extra_flags = fread_bitvector(fp);
      ln = fread_line(fp);
      x1 = x2 = 0;
      sscanf(ln, "%d %d", &x1, &x2);
      pObjIndex->wear_flags = x1;
      pObjIndex->layers = x2;

      if (area_version < 22)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = 0;
         sscanf(ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6);
         pObjIndex->value[0] = x1;
         pObjIndex->value[1] = x2;
         pObjIndex->value[2] = x3;
         pObjIndex->value[3] = x4;
         pObjIndex->value[4] = x5;
         pObjIndex->value[5] = x6;
      }
      else if (area_version < 30)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
         sscanf(ln, "%d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9);
         pObjIndex->value[0] = x1;
         pObjIndex->value[1] = x2;
         pObjIndex->value[2] = x3;
         pObjIndex->value[3] = x4;
         pObjIndex->value[4] = x5;
         pObjIndex->value[5] = x6;
         pObjIndex->value[6] = x7;
         pObjIndex->value[7] = x8;
         pObjIndex->value[8] = x9;
      }
      else if (area_version < 31)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = 0;
         sscanf(ln, "%d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10);
         pObjIndex->value[0] = x1;
         pObjIndex->value[1] = x2;
         pObjIndex->value[2] = x3;
         pObjIndex->value[3] = x4;
         pObjIndex->value[4] = x5;
         pObjIndex->value[5] = x6;
         pObjIndex->value[6] = x7;
         pObjIndex->value[7] = x8;
         pObjIndex->value[8] = x9;
         pObjIndex->value[9] = x10;
      }
      else if (area_version < 42)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = 0;
         sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11);
         pObjIndex->value[0] = x1;
         pObjIndex->value[1] = x2;
         pObjIndex->value[2] = x3;
         pObjIndex->value[3] = x4;
         pObjIndex->value[4] = x5;
         pObjIndex->value[5] = x6;
         pObjIndex->value[6] = x7;
         pObjIndex->value[7] = x8;
         pObjIndex->value[8] = x9;
         pObjIndex->value[9] = x10;
         pObjIndex->value[10] = x11;
      }
      else if (area_version < 46)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = 0;
         sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12);
         pObjIndex->value[0] = x1;
         pObjIndex->value[1] = x2;
         pObjIndex->value[2] = x3;
         pObjIndex->value[3] = x4;
         pObjIndex->value[4] = x5;
         pObjIndex->value[5] = x6;
         pObjIndex->value[6] = x7;
         pObjIndex->value[7] = x8;
         pObjIndex->value[8] = x9;
         pObjIndex->value[9] = x10;
         pObjIndex->value[10] = x11;
         pObjIndex->value[11] = x12;
      }
      else
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = 0;
         sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
            &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14);
         pObjIndex->value[0] = x1;
         pObjIndex->value[1] = x2;
         pObjIndex->value[2] = x3;
         pObjIndex->value[3] = x4;
         pObjIndex->value[4] = x5;
         pObjIndex->value[5] = x6;
         pObjIndex->value[6] = x7;
         pObjIndex->value[7] = x8;
         pObjIndex->value[8] = x9;
         pObjIndex->value[9] = x10;
         pObjIndex->value[10] = x11;
         pObjIndex->value[11] = x12;
         pObjIndex->value[12] = x13;
         pObjIndex->value[13] = x14;
      }
      if (area_version == 11 || area_version == 12)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = 0;
         sscanf(ln, "%d %d %d %d", &x1, &x2, &x3, &x4);

         pObjIndex->weight = x1;
         pObjIndex->weight = UMAX(1, pObjIndex->weight);
         pObjIndex->cost = x2;
         pObjIndex->rent = x3; /* unused */
         pObjIndex->cvnum = x4;
      }
      else if (area_version >= 51)
      {
         float f1;
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = 0;
         sscanf(ln, "%f %d %d %d %d %d %d", &f1, &x2, &x3, &x4, &x5, &x6, &x7);

         pObjIndex->weight = f1;
         pObjIndex->weight = UMAX(.01, pObjIndex->weight);
         pObjIndex->cost = x2;
         pObjIndex->rent = x3; /* unused */
         pObjIndex->cvnum = x4;
         pObjIndex->cident = x5;
         pObjIndex->sworthrestrict = x6;
         pObjIndex->imbueslots = x7;
      }
      else if (area_version >= 49)
      {
         float f1;
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = 0;
         sscanf(ln, "%f %d %d %d %d %d", &f1, &x2, &x3, &x4, &x5, &x6);

         pObjIndex->weight = f1;
         pObjIndex->weight = UMAX(.01, pObjIndex->weight);
         pObjIndex->cost = x2;
         pObjIndex->rent = x3; /* unused */
         pObjIndex->cvnum = x4;
         pObjIndex->cident = x5;
         pObjIndex->sworthrestrict = x6;
      }    
      else if (area_version >= 48)
      {
         float f1;
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = 0;
         sscanf(ln, "%f %d %d %d %d", &f1, &x2, &x3, &x4, &x5);

         pObjIndex->weight = f1;
         pObjIndex->weight = UMAX(.01, pObjIndex->weight);
         pObjIndex->cost = x2;
         pObjIndex->rent = x3; /* unused */
         pObjIndex->cvnum = x4;
         pObjIndex->cident = x5;
      }  
      else if (area_version > 12)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = 0;
         sscanf(ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5);

         pObjIndex->weight = x1;
         pObjIndex->weight = UMAX(1, pObjIndex->weight);
         pObjIndex->cost = x2;
         pObjIndex->rent = x3; /* unused */
         pObjIndex->cvnum = x4;
         pObjIndex->cident = x5;
      }
      /* I THINK THIS IS BROKEN, TAKE IT OUT? */
      /*
      else if (area_version > 38)
      {
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = 0;
         sscanf(ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6);
         
         pObjIndex->weight = x1;
         pObjIndex->weight = UMAX(1, pObjIndex->weight);
         pObjIndex->cost = x2;
         pObjIndex->bless_dur = x3;
         pObjIndex->rent = x4;
         pObjIndex->cvnum = x5;
         pObjIndex->cident = x6;
         
      }*/
      else
      {
         ln = fread_line(fp);
         x1 = x2 = x3 == 0;
         sscanf(ln, "%d %d %d", &x1, &x2, &x3);

         pObjIndex->weight = x1;
         pObjIndex->weight = UMAX(1, pObjIndex->weight);
         pObjIndex->cost = x2;
         pObjIndex->rent = x3; /* unused */
      }
      if (area_version >= 1)
      {
         switch (pObjIndex->item_type)
         {
            case ITEM_WEAPON:
               if (area_version >= 32)
               {
                  pObjIndex->value[4] = skill_lookup(fread_word(fp));
               }
               break;
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
               pObjIndex->value[1] = skill_lookup(fread_word(fp));
               pObjIndex->value[2] = skill_lookup(fread_word(fp));
               pObjIndex->value[3] = skill_lookup(fread_word(fp));
               break;
            case ITEM_STAFF:
            case ITEM_WAND:
               pObjIndex->value[3] = skill_lookup(fread_word(fp));
               break;
            case ITEM_SALVE:
               pObjIndex->value[4] = skill_lookup(fread_word(fp));
               pObjIndex->value[5] = skill_lookup(fread_word(fp));
               break;
            case ITEM_SPELLBOOK:
               pObjIndex->value[1] = skill_lookup(fread_word(fp));
               break;
            case ITEM_SHEATH:
               pObjIndex->value[4] = skill_lookup(fread_word(fp));
               break;
			case ITEM_TREASURE:
 			    if(xIS_SET(pObjIndex->extra_flags,ITEM_AFFREAG))
 					pObjIndex->value[1] = skill_lookup(fread_word(fp));
 				break;
         }
      }
      for (;;)
      {
         letter = fread_letter(fp);

         if (letter == 'A')
         {
            AFFECT_DATA *paf;

            CREATE(paf, AFFECT_DATA, 1);
            paf->type = -1;
            paf->duration = -1;
            paf->location = fread_number(fp);
            if (paf->location == APPLY_WEAPONSPELL
               || paf->location == APPLY_WEARSPELL
               || paf->location == APPLY_REMOVESPELL || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL)
               paf->modifier = slot_lookup(fread_number(fp));
            else
               paf->modifier = fread_number(fp);
            xCLEAR_BITS(paf->bitvector);
            if (area_version >= 53)
               paf->gemnum = fread_number(fp);
            LINK(paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev);
            top_affect++;
         }

         else if (letter == 'E')
         {
            EXTRA_DESCR_DATA *ed;

            CREATE(ed, EXTRA_DESCR_DATA, 1);
            ed->keyword = fread_string(fp);
            ed->description = fread_string(fp);
            LINK(ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev);
            top_ed++;
         }
         else if (letter == '>')
         {
            ungetc(letter, fp);
            oprog_read_programs(fp, pObjIndex);
         }

         else
         {
            ungetc(letter, fp);
            break;
         }
      }

      /*
       * Translate spell "slot numbers" to internal "skill numbers."
       */
      if (area_version == 0)
         switch (pObjIndex->item_type)
         {
            case ITEM_PILL:
            case ITEM_POTION:
            case ITEM_SCROLL:
               pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
               pObjIndex->value[2] = slot_lookup(pObjIndex->value[2]);
               pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
               break;

            case ITEM_STAFF:
            case ITEM_WAND:
               pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
               break;
            case ITEM_SALVE:
               pObjIndex->value[4] = slot_lookup(pObjIndex->value[4]);
               pObjIndex->value[5] = slot_lookup(pObjIndex->value[5]);
               break;
            case ITEM_SPELLBOOK:
               pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
               break;
            case ITEM_SHEATH:
               pObjIndex->value[4] = slot_lookup(pObjIndex->value[4]);
               break;
			case ITEM_TREASURE:
 			    if(xIS_SET(pObjIndex->extra_flags,ITEM_AFFREAG))
 					pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
 				break;
         }

      if (!oldobj)
      {
         iHash = vnum % MAX_KEY_HASH;
         pObjIndex->next = obj_index_hash[iHash];
         obj_index_hash[iHash] = pObjIndex;
         top_obj_index++;
      }
   }

   return;
}



/*
 * Load a reset section.
 */
void load_resets(AREA_DATA * tarea, FILE * fp)
{
   char buf[MSL];
   bool not01 = FALSE;
   int count = 0;

   if (!tarea)
   {
      bug("Load_resets: no #AREA seen yet.");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   if (tarea->first_reset)
   {
      if (fBootDb)
      {
         RESET_DATA *rtmp;

         bug("load_resets: WARNING: resets already exist for this area.");
         for (rtmp = tarea->first_reset; rtmp; rtmp = rtmp->next)
            ++count;
      }
      else
      {
         /*
          * Clean out the old resets
          */
         sprintf(buf, "Cleaning resets: %s", tarea->name);
         log_string_plus(buf, LOG_BUILD, sysdata.log_level);
         clean_resets(tarea);
      }
   }

   for (;;)
   {
      ROOM_INDEX_DATA *pRoomIndex;
      EXIT_DATA *pexit;
      char letter;
      int extra, arg1, arg2, arg3, arg4, arg5, arg6, arg7, resetlast, resettime;

      if ((letter = fread_letter(fp)) == 'S')
         break;

      if (letter == '*')
      {
         fread_to_eol(fp);
         continue;
      }

      extra = fread_number(fp);
      arg1 = fread_number(fp);
      arg2 = fread_number(fp);
      if (letter == 'G')
      {
         if (area_version >= 33)
            arg3 = fread_number(fp);
         else
            arg3 = -1;
      }         
      else
      {
         arg3 = (letter == 'R' || letter == 'A') ? 0 : fread_number(fp);
      }
      if (area_version == 20)
      {
         if (letter == 'M')
         {
            arg4 = fread_number(fp);
            arg5 = fread_number(fp);
            arg6 = fread_number(fp);
         }
         else
         {
            arg4 = -1;
            arg5 = -1;
            arg6 = -1;
         }
      }
      else if (area_version >= 21)
      {
         if (letter == 'O' || letter == 'M')
         {
            arg4 = fread_number(fp);
            arg5 = fread_number(fp);
            arg6 = fread_number(fp);
         }
         else
         {
            arg4 = -1;
            arg5 = -1;
            arg6 = -1;
         }
      }
      else
      {
         arg4 = -1;
         arg5 = -1;
         arg6 = -1;
      }
      if (area_version >= 33)
      {
         if (letter == 'O')
         {
            arg7 = fread_number(fp);
         }
         else
         {
            arg7 = -1;
         }
      }
      else
      {
         arg7 = -1;
      }
      if (area_version >= 35)
      {
          if (letter == 'P' || letter == 'E')
          {
             arg4 = fread_number(fp);
          }
      }
      if (area_version >= 43)
      {
         if (letter == 'G')
            arg4 = fread_number(fp);
      }
      //NOTE This has to be the last thing read unless you put any new values after resetlast and resettime
      if ((letter == 'P' || letter == 'E' || letter == 'O' || letter == 'M' || letter == 'G') && area_version >= 40)
      {
         resetlast = fread_number(fp);
         resettime = fread_number(fp);
      }
      else
      {
         resetlast = 0;
         resettime = 0;
      }
         
      fread_to_eol(fp);

      ++count;

      /*
       * Validate parameters.
       * We're calling the index functions for the side effect.
       */
      switch (letter)
      {
         default:
            bug("Load_resets: bad command '%c'.", letter);
            if (fBootDb)
               boot_log("Load_resets: %s (%d) bad command '%c'.", tarea->filename, count, letter);
            return;

         case 'A':
            break;

         case 'M':
            if (get_mob_index(arg1) == NULL && fBootDb)
               boot_log("Load_resets: %s (%d) 'M': mobile %d doesn't exist.", tarea->filename, count, arg1);
            if (get_room_index(arg3) == NULL && fBootDb)
               boot_log("Load_resets: %s (%d) 'M': room %d doesn't exist.", tarea->filename, count, arg3);
            break;

         case 'O':
            if (get_obj_index(arg1) == NULL && fBootDb)
               boot_log("Load_resets: %s (%d) '%c': object %d doesn't exist.", tarea->filename, count, letter, arg1);
            if (get_room_index(arg3) == NULL && fBootDb)
               boot_log("Load_resets: %s (%d) '%c': room %d doesn't exist.", tarea->filename, count, letter, arg3);
            if (arg7 > 99 && get_obj_index(arg7) == NULL && fBootDb)
               boot_log("Load resets: %s (%d) '%c': ore %d doesn't exist.", tarea->filename, count, letter, arg7);
            break;
               
         case 'P':
            if (get_obj_index(arg1) == NULL && fBootDb)
               boot_log("Load_resets: %s (%d) '%c': object %d doesn't exist.", tarea->filename, count, letter, arg1);
            if (arg3 > 0)
            {
               if (get_obj_index(arg3) == NULL && fBootDb)
                  boot_log("Load_resets: %s (%d) 'P': destination object %d doesn't exist.", tarea->filename, count, arg3);
            }
            else
            {
               if (extra > 1)
                  not01 = TRUE;
            }
            if (arg4 > 99 && get_obj_index(arg4) == NULL && fBootDb)
               boot_log("Load resets: %s (%d) '%c': ore %d doesn't exist.", tarea->filename, count, letter, arg4);
            break;
         case 'G':
            if (get_obj_index(arg1) == NULL && fBootDb)
               boot_log("Load_resets: %s (%d) '%c': object %d doesn't exist.", tarea->filename, count, letter, arg1);
            if (arg3 > 99 && get_obj_index(arg3) == NULL && fBootDb)
               boot_log("Load resets: %s (%d) '%c': ore %d doesn't exist.", tarea->filename, count, letter, arg3);
            break;
         case 'E':
            if (get_obj_index(arg1) == NULL && fBootDb)
               boot_log("Load_resets: %s (%d) '%c': object %d doesn't exist.", tarea->filename, count, letter, arg1);
            if (arg4 > 99 && get_obj_index(arg4) == NULL && fBootDb)
               boot_log("Load resets: %s (%d) '%c': ore %d doesn't exist.", tarea->filename, count, letter, arg4);
            break;

         case 'T':
            break;

         case 'H':
            if (arg1 > 0)
               if (get_obj_index(arg1) == NULL && fBootDb)
                  boot_log("Load_resets: %s (%d) 'H': object %d doesn't exist.", tarea->filename, count, arg1);
            break;

         case 'B':
            switch (arg2 & BIT_RESET_TYPE_MASK)
            {
               case BIT_RESET_DOOR:
                  {
                     int door;

                     pRoomIndex = get_room_index(arg1);
                     if (!pRoomIndex)
                     {
                        bug("Load_resets: 'B': room %d doesn't exist.", arg1);
                        bug("Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3);
                        if (fBootDb)
                           boot_log("Load_resets: %s (%d) 'B': room %d doesn't exist.", tarea->filename, count, arg1);
                     }

                     door = (arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;

                     if (!(pexit = get_exit(pRoomIndex, door)))
                     {
                        bug("Load_resets: 'B': exit %d not door.", door);
                        bug("Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3);
                        if (fBootDb)
                           boot_log("Load_resets: %s (%d) 'B': exit %d not door.", tarea->filename, count, door);
                     }
                  }
                  break;
               case BIT_RESET_ROOM:
                  if (get_room_index(arg1) == NULL)
                  {
                     bug("Load_resets: 'B': room %d doesn't exist.", arg1);
                     bug("Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3);
                     if (fBootDb)
                        boot_log("Load_resets: %s (%d) 'B': room %d doesn't exist.", tarea->filename, count, arg1);
                  }
                  break;
               case BIT_RESET_OBJECT:
                  if (arg1 > 0)
                     if (get_obj_index(arg1) == NULL && fBootDb)
                        boot_log("Load_resets: %s (%d) 'B': object %d doesn't exist.", tarea->filename, count, arg1);
                  break;
               case BIT_RESET_MOBILE:
                  if (arg1 > 0)
                     if (get_mob_index(arg1) == NULL && fBootDb)
                        boot_log("Load_resets: %s (%d) 'B': mobile %d doesn't exist.", tarea->filename, count, arg1);
                  break;
               default:
                  boot_log("Load_resets: %s (%d) 'B': bad type flag (%d).", tarea->filename, count, arg2 & BIT_RESET_TYPE_MASK);
                  break;
            }
            break;

         case 'D':
            pRoomIndex = get_room_index(arg1);
            if (!pRoomIndex)
            {
               bug("Load_resets: 'D': room %d doesn't exist.", arg1);
               bug("Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3);
               if (fBootDb)
                  boot_log("Load_resets: %s (%d) 'D': room %d doesn't exist.", tarea->filename, count, arg1);
               break;
            }

            if (arg2 < 0 || arg2 > MAX_DIR + 1 || (pexit = get_exit(pRoomIndex, arg2)) == NULL || !IS_SET(pexit->exit_info, EX_ISDOOR))
            {
               bug("Load_resets: 'D': exit %d not door.", arg2);
               bug("Reset: %c %d %d %d %d", letter, extra, arg1, arg2, arg3);
               if (fBootDb)
                  boot_log("Load_resets: %s (%d) 'D': exit %d not door.", tarea->filename, count, arg2);
            }

            if (arg3 < 0 || arg3 > 2)
            {
               bug("Load_resets: 'D': bad 'locks': %d.", arg3);
               if (fBootDb)
                  boot_log("Load_resets: %s (%d) 'D': bad 'locks': %d.", tarea->filename, count, arg3);
            }
            break;

         case 'R':
            pRoomIndex = get_room_index(arg1);
            if (!pRoomIndex && fBootDb)
               boot_log("Load_resets: %s (%d) 'R': room %d doesn't exist.", tarea->filename, count, arg1);

            if (arg2 < 0 || arg2 > 6)
            {
               bug("Load_resets: 'R': bad exit %d.", arg2);
               if (fBootDb)
                  boot_log("Load_resets: %s (%d) 'R': bad exit %d.", tarea->filename, count, arg2);
               break;
            }

            break;
      }

      /* finally, add the reset */
      add_reset(tarea, letter, extra, arg1, arg2, arg3, arg4, arg5, arg6, arg7, resetlast, resettime);
   }

   if (!not01)
      renumber_put_resets(tarea);

   return;
}



/*
 * Load a room section.
 */
void load_rooms(AREA_DATA * tarea, FILE * fp)
{
   ROOM_INDEX_DATA *pRoomIndex;
   char buf[MSL];
   char *ln;

   if (!tarea)
   {
      bug("Load_rooms: no #AREA seen yet.");
      shutdown_mud("No #AREA");
      exit(1);
   }

   for (;;)
   {
      int vnum;
      char letter;
      int door;
      int iHash;
      bool tmpBootDb;
      bool oldroom;
      int x1, x2, x3, x4, x5, x6;

      letter = fread_letter(fp);
      if (letter != '#')
      {
         bug("Load_rooms: # not found.");
         if (fBootDb)
         {
            shutdown_mud("# not found");
            exit(1);
         }
         else
            return;
      }

      vnum = fread_number(fp);
      if (vnum == 0)
         break;

      tmpBootDb = fBootDb;
      fBootDb = FALSE;
      if (get_room_index(vnum) != NULL)
      {
         if (tmpBootDb)
         {
            bug("Load_rooms: vnum %d duplicated.", vnum);
            shutdown_mud("duplicate vnum");
            exit(1);
         }
         else
         {
            pRoomIndex = get_room_index(vnum);
            sprintf(buf, "Cleaning room: %d", vnum);
            log_string_plus(buf, LOG_BUILD, sysdata.log_level);
            clean_room(pRoomIndex);
            oldroom = TRUE;
         }
      }
      else
      {
         oldroom = FALSE;
         CREATE(pRoomIndex, ROOM_INDEX_DATA, 1);
         pRoomIndex->first_person = NULL;
         pRoomIndex->last_person = NULL;
         pRoomIndex->first_content = NULL;
         pRoomIndex->last_content = NULL;
      }

      fBootDb = tmpBootDb;
      pRoomIndex->area = tarea;
      pRoomIndex->vnum = vnum;
      pRoomIndex->first_extradesc = NULL;
      pRoomIndex->last_extradesc = NULL;

      if (fBootDb)
      {
         if (!tarea->low_r_vnum)
            tarea->low_r_vnum = vnum;
         if (vnum > tarea->hi_r_vnum)
            tarea->hi_r_vnum = vnum;
      }
      pRoomIndex->name = fread_string(fp);
      pRoomIndex->description = fread_string(fp);

      /* Area number     fread_number( fp ); */
/*	ln = fread_line( fp );
	x1=x2=x3=x4=x5=x6=0;
	sscanf( ln, "%d %s %d %d %d %d",
	      &x1, &x2, &x3, &x4, &x5, &x6 );

	pRoomIndex->room_flags		= x2;
	pRoomIndex->sector_type		= x3;
	pRoomIndex->tele_delay		= x4;
	pRoomIndex->tele_vnum		= x5;
	pRoomIndex->tunnel		= x6;

        pRoomIndex->room_flags          = fread_bitvector(fp);
        pRoomIndex->sector_type         = fread_number( fp );
        pRoomIndex->tele_delay          = fread_number( fp );
        pRoomIndex->tele_vnum           = fread_number( fp );
        pRoomIndex->tunnel              = fread_number( fp ); */
      if (area_version > 15)
      {
         pRoomIndex->room_flags = fread_bitvector(fp);
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = x6 = 0;
         sscanf(ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6);

         pRoomIndex->sector_type = x2;
         pRoomIndex->tele_delay = x3;
         pRoomIndex->tele_vnum = x4;
         pRoomIndex->tunnel = x5;
         pRoomIndex->resource = x6;
      }
      else if (area_version > 13 && area_version < 16)
      {
         pRoomIndex->room_flags = fread_bitvector(fp);
         ln = fread_line(fp);
         x1 = x2 = x3 = x4 = x5 = 0;
         sscanf(ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5);

         pRoomIndex->sector_type = x2;
         pRoomIndex->tele_delay = x3;
         pRoomIndex->tele_vnum = x4;
         pRoomIndex->tunnel = x5;
      }
      else /* Convert OLD BVs into new xBVs */
      {
         char *oldflag = NULL;
         char oneflag[MIL];
         int newvalue, oldvalue;

         x1 = x2 = x3 = x4 = x5 = x6 = 0;
         ln = fread_line(fp);
         sscanf(ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6);

         oldvalue = x2;
         pRoomIndex->sector_type = x3;
         pRoomIndex->tele_delay = x4;
         pRoomIndex->tele_vnum = x5;
         pRoomIndex->tunnel = x6;

         oldflag = flag_string(oldvalue, r_flags);

         while (oldflag[0] != '\0')
         {
            oldflag = one_argument(oldflag, oneflag);
            newvalue = get_rflag(oneflag);
            if (newvalue < 0 || newvalue > MAX_BITS)
            {
               sprintf(log_buf, "Unknown room flag: %s", oneflag);
               log_string(log_buf);
            }
            else
               xSET_BIT(pRoomIndex->room_flags, newvalue);
         }
      }
      if (area_version > 16)
      {
         pRoomIndex->quad = fread_number(fp);
      }
      if (area_version > 37)
      {
        pRoomIndex->node_mana = fread_number(fp);
      }
      if (pRoomIndex->sector_type < 0 || pRoomIndex->sector_type == SECT_MAX)
      {
         bug("Fread_rooms: vnum %d has bad sector_type %d.", vnum, pRoomIndex->sector_type);
         pRoomIndex->sector_type = 1;
      }
      pRoomIndex->light = 0;
      pRoomIndex->first_exit = NULL;
      pRoomIndex->last_exit = NULL;

      for (;;)
      {
         letter = fread_letter(fp);

         if (letter == 'S')
            break;

         if (letter == 'D')
         {
            EXIT_DATA *pexit;
            int locks;

            door = fread_number(fp);
            if (door < 0 || door > 10)
            {
               bug("Fread_rooms: vnum %d has bad door number %d.", vnum, door);
               if (fBootDb)
                  exit(1);
            }
            pexit = make_exit(pRoomIndex, NULL, door);
            pexit->description = fread_string(fp);
            pexit->keyword = fread_string(fp);
            pexit->exit_info = 0;
            ln = fread_line(fp);
            x1 = x2 = x3 = x4 = x5 = x6 = 0;
            sscanf(ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6);

            locks = x1;
            pexit->key = x2;
            pexit->vnum = x3;
            pexit->vdir = door;
            pexit->distance = x4;
            pexit->pulltype = x5;
            pexit->pull = x6;

            if (area_version >= 19)
            {
               ln = fread_line(fp);
               x1 = x2 = 0;
               sscanf(ln, "%d %d", &x1, &x2);

               pexit->coord->x = x1;
               pexit->coord->y = x2;
            }

            switch (locks)
            {
               case 1:
                  pexit->exit_info = EX_ISDOOR;
                  break;
               case 2:
                  pexit->exit_info = EX_ISDOOR | EX_PICKPROOF;
                  break;
               default:
                  pexit->exit_info = locks;

            }
         }
         else if (letter == 'E')
         {
            EXTRA_DESCR_DATA *ed;

            CREATE(ed, EXTRA_DESCR_DATA, 1);
            ed->keyword = fread_string(fp);
            ed->description = fread_string(fp);
            LINK(ed, pRoomIndex->first_extradesc, pRoomIndex->last_extradesc, next, prev);
            top_ed++;
         }
         else if (letter == 'M') /* maps */
         {
            MAP_DATA *map;
            MAP_INDEX_DATA *map_index;
            int i, j;

            CREATE(map, MAP_DATA, 1);
            map->vnum = fread_number(fp);
            map->x = fread_number(fp);
            map->y = fread_number(fp);
            map->entry = fread_letter(fp);

            pRoomIndex->map = map;
            if ((map_index = get_map_index(map->vnum)) == NULL)
            {
               CREATE(map_index, MAP_INDEX_DATA, 1);
               map_index->vnum = map->vnum;
               map_index->next = first_map;
               first_map = map_index;
               for (i = 0; i < 49; i++)
               {
                  for (j = 0; j < 79; j++)
                  {
                     map_index->map_of_vnums[i][j] = -1;
                     /* map_index->map_of_ptrs[i][j] = NULL; */
                  }
               }
            }
            if ((map->y < 0) || (map->y > 48))
            {
               bug("Map y coord out of range.  Room %d\n\r", map->y);

            }
            if ((map->x < 0) || (map->x > 78))
            {
               bug("Map x coord out of range.  Room %d\n\r", map->x);

            }
            if ((map->x > 0) && (map->x < 80) && (map->y > 0) && (map->y < 48))
               map_index->map_of_vnums[map->y][map->x] = pRoomIndex->vnum;
         }
         else if (letter == '>')
         {
            ungetc(letter, fp);
            rprog_read_programs(fp, pRoomIndex);
         }
         else
         {
            bug("Load_rooms: vnum %d has flag '%c' not 'DES'.", vnum, letter);
            shutdown_mud("Room flag not DES");
            exit(1);
         }

      }

      if (!oldroom)
      {
         iHash = vnum % MAX_KEY_HASH;
         pRoomIndex->next = room_index_hash[iHash];
         room_index_hash[iHash] = pRoomIndex;
         top_room++;
      }
   }

   return;
}



/*
 * Load a shop section.
 */
void load_shops(AREA_DATA * tarea, FILE * fp)
{
   SHOP_DATA *pShop;

   for (;;)
   {
      MOB_INDEX_DATA *pMobIndex;
      int iTrade;

      CREATE(pShop, SHOP_DATA, 1);
      pShop->keeper = fread_number(fp);
      if (pShop->keeper == 0)
      {
         DISPOSE(pShop);
         break;
      }
      for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
         pShop->buy_type[iTrade] = fread_number(fp);
      pShop->profit_buy = fread_number(fp);
      pShop->profit_sell = fread_number(fp);
      pShop->profit_buy = URANGE(pShop->profit_sell + 5, pShop->profit_buy, 1000);
      pShop->profit_sell = URANGE(0, pShop->profit_sell, pShop->profit_buy - 5);
      pShop->open_hour = fread_number(fp);
      pShop->close_hour = fread_number(fp);
      fread_to_eol(fp);
      pMobIndex = get_mob_index(pShop->keeper);
      pMobIndex->pShop = pShop;

      if (!first_shop)
         first_shop = pShop;
      else
         last_shop->next = pShop;
      pShop->next = NULL;
      pShop->prev = last_shop;
      last_shop = pShop;
      top_shop++;
   }
   return;
}

/*
 * Load a repair shop section.					-Thoric
 */
void load_repairs(AREA_DATA * tarea, FILE * fp)
{
   REPAIR_DATA *rShop;

   for (;;)
   {
      MOB_INDEX_DATA *pMobIndex;
      int iFix;

      CREATE(rShop, REPAIR_DATA, 1);
      rShop->keeper = fread_number(fp);
      if (rShop->keeper == 0)
      {
         DISPOSE(rShop);
         break;
      }
      for (iFix = 0; iFix < MAX_FIX; iFix++)
         rShop->fix_type[iFix] = fread_number(fp);
      rShop->profit_fix = fread_number(fp);
      rShop->shop_type = fread_number(fp);
      rShop->open_hour = fread_number(fp);
      rShop->close_hour = fread_number(fp);
      fread_to_eol(fp);
      pMobIndex = get_mob_index(rShop->keeper);
      pMobIndex->rShop = rShop;

      if (!first_repair)
         first_repair = rShop;
      else
         last_repair->next = rShop;
      rShop->next = NULL;
      rShop->prev = last_repair;
      last_repair = rShop;
      top_repair++;
   }
   return;
}


/*
 * Load spec proc declarations.
 */
void load_specials(AREA_DATA * tarea, FILE * fp)
{
   for (;;)
   {
      MOB_INDEX_DATA *pMobIndex;
      char letter;

      switch (letter = fread_letter(fp))
      {
         default:
            bug("Load_specials: letter '%c' not *MS.", letter);
            exit(1);

         case 'S':
            return;

         case '*':
            break;

         case 'M':
            pMobIndex = get_mob_index(fread_number(fp));
            pMobIndex->spec_fun = spec_lookup(fread_word(fp));
            if (pMobIndex->spec_fun == 0)
            {
               bug("Load_specials: 'M': vnum %d.", pMobIndex->vnum);
               exit(1);
            }
            break;
      }

      fread_to_eol(fp);
   }
}


/*
 * Load soft / hard area ranges.
 */
void load_ranges(AREA_DATA * tarea, FILE * fp)
{
   int x1, x2, x3, x4, x5, x6, x7, x8, x9;
   char *ln;

   if (!tarea)
   {
      bug("Load_ranges: no #AREA seen yet.");
      shutdown_mud("No #AREA");
      exit(1);
   }

   for (;;)
   {
      ln = fread_line(fp);

      if (ln[0] == '$')
         break;

      x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = 0;
      if (area_version >= 44)
      {
         sscanf(ln, "%d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9);
      }
      
      if (area_version >= 28)
      {
         sscanf(ln, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8);
      }
      if (area_version >= 23)
      {
         sscanf(ln, "%d %d %d %d %d", &x1, &x2, &x3, &x4, &x5);
      }
      else
      {
         sscanf(ln, "%d %d %d %d", &x1, &x2, &x3, &x4);
      }


      tarea->low_soft_range = x1;
      tarea->hi_soft_range = x2;
      tarea->low_hard_range = x3;
      tarea->hi_hard_range = x4;
      if (area_version >= 23)
      {
         tarea->kingdom = x5;
      }
      if (area_version >= 28)
      {
         tarea->x = x6;
         tarea->y = x7;
         tarea->map = x8;
      }
      if (area_version >= 44)
         tarea->kpid = x9;
         
   }
   return;

}

/*
 * Load climate information for the area
 * Last modified: July 13, 1997
 * Fireblade
 * REMOVED, left here for porting purposes, it just loads the old
 * numbers and saves as 0 for now -- Xerves
 */
void load_climate(AREA_DATA * tarea, FILE * fp)
{
   int st1;

   if (!tarea)
   {
      bug("load_climate: no #AREA seen yet");
      if (fBootDb)
      {
         shutdown_mud("No #AREA");
         exit(1);
      }
      else
         return;
   }

   st1 = fread_number(fp);
   st1 = fread_number(fp);
   st1 = fread_number(fp);

   return;
}


/*
 * Go through all areas, and set up initial economy based on mob
 * levels and gold
 */
void initialize_economy(void)
{
   AREA_DATA *tarea;
   MOB_INDEX_DATA *mob;
   int idx, gold, rng;

   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      /* skip area if they already got some gold */
      if (tarea->high_economy > 0 || tarea->low_economy > 10000)
         continue;
      rng = tarea->hi_soft_range - tarea->low_soft_range;
      if (rng)
         rng /= 2;
      else
         rng = 25;
      gold = rng * rng * 50000;
      boost_economy(tarea, gold);
      for (idx = tarea->low_m_vnum; idx < tarea->hi_m_vnum; idx++)
         if ((mob = get_mob_index(idx)) != NULL)
            boost_economy(tarea, mob->gold * 10);
   }
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 * Check for bad reverse exits.
 */
void fix_exits(void)
{
   ROOM_INDEX_DATA *pRoomIndex;
   EXIT_DATA *pexit, *pexit_next, *rev_exit;
   int iHash;

   for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
   {
      for (pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next)
      {
         bool fexit;

         fexit = FALSE;
         for (pexit = pRoomIndex->first_exit; pexit; pexit = pexit_next)
         {
            pexit_next = pexit->next;
            pexit->rvnum = pRoomIndex->vnum;
            if (pexit->vnum <= 0 || (pexit->to_room = get_room_index(pexit->vnum)) == NULL)
            {
               if (fBootDb)
                  boot_log("Fix_exits: room %d, exit %s leads to bad vnum (%d)", pRoomIndex->vnum, dir_name[pexit->vdir], pexit->vnum);

               bug("Deleting %s exit in room %d", dir_name[pexit->vdir], pRoomIndex->vnum);
               extract_exit(pRoomIndex, pexit);
            }
            else
               fexit = TRUE;
         }
         if (!fexit)
            xSET_BIT(pRoomIndex->room_flags, ROOM_NO_MOB);
      }
   }

   /* Set all the rexit pointers  -Thoric */
   for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
   {
      for (pRoomIndex = room_index_hash[iHash]; pRoomIndex; pRoomIndex = pRoomIndex->next)
      {
         for (pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next)
         {
            if (pexit->to_room && !pexit->rexit)
            {
               rev_exit = get_exit_to(pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum);
               if (rev_exit)
               {
                  pexit->rexit = rev_exit;
                  rev_exit->rexit = pexit;
               }
            }
         }
      }
   }

   return;
}


/*
 * Get diku-compatable exit by number				-Thoric
 */
EXIT_DATA *get_exit_number(ROOM_INDEX_DATA * room, int xit)
{
   EXIT_DATA *pexit;
   int count;

   count = 0;
   for (pexit = room->first_exit; pexit; pexit = pexit->next)
      if (++count == xit)
         return pexit;
   return NULL;
}

/*
 * (prelude...) This is going to be fun... NOT!
 * (conclusion) QSort is f*cked!
 */
int exit_comp(EXIT_DATA ** xit1, EXIT_DATA ** xit2)
{
   int d1, d2;

   d1 = (*xit1)->vdir;
   d2 = (*xit2)->vdir;

   if (d1 < d2)
      return -1;
   if (d1 > d2)
      return 1;
   return 0;
}

void sort_exits(ROOM_INDEX_DATA * room)
{
   EXIT_DATA *pexit; /* *texit *//* Unused */
   EXIT_DATA *exits[MAX_REXITS];
   int x, nexits;

   nexits = 0;
   for (pexit = room->first_exit; pexit; pexit = pexit->next)
   {
      exits[nexits++] = pexit;
      if (nexits > MAX_REXITS)
      {
         bug("sort_exits: more than %d exits in room... fatal", nexits);
         return;
      }
   }
   qsort(&exits[0], nexits, sizeof(EXIT_DATA *), (int (*)(const void *, const void *)) exit_comp);
   for (x = 0; x < nexits; x++)
   {
      if (x > 0)
         exits[x]->prev = exits[x - 1];
      else
      {
         exits[x]->prev = NULL;
         room->first_exit = exits[x];
      }
      if (x >= (nexits - 1))
      {
         exits[x]->next = NULL;
         room->last_exit = exits[x];
      }
      else
         exits[x]->next = exits[x + 1];
   }
}

void randomize_exits(ROOM_INDEX_DATA * room, sh_int maxdir)
{
   EXIT_DATA *pexit;
   int nexits, /* maxd, */ d0, d1, count, door; /* Maxd unused */
   int vdirs[MAX_REXITS];

   nexits = 0;
   for (pexit = room->first_exit; pexit; pexit = pexit->next)
      vdirs[nexits++] = pexit->vdir;

   for (d0 = 0; d0 < nexits; d0++)
   {
      if (vdirs[d0] > maxdir)
         continue;
      count = 0;
      while (vdirs[(d1 = number_range(d0, nexits - 1))] > maxdir || ++count > 5) ;
      if (vdirs[d1] > maxdir)
         continue;
      door = vdirs[d0];
      vdirs[d0] = vdirs[d1];
      vdirs[d1] = door;
   }
   count = 0;
   for (pexit = room->first_exit; pexit; pexit = pexit->next)
      pexit->vdir = vdirs[count++];

   sort_exits(room);
}


/*
 * Repopulate areas periodically.
 */
void area_update(void)
{
   AREA_DATA *pArea;
   int vnum;
   
   for (pArea = first_area; pArea; pArea = pArea->next)
   {
      CHAR_DATA *pch;
      int reset_age = pArea->reset_frequency ? pArea->reset_frequency : 15;

      if ((reset_age == -1 && pArea->age == -1) || ++pArea->age < (reset_age - 1))
         continue;

      /*
       * Check for PC's.
       */
      if (pArea->nplayer > 0 && pArea->age == (reset_age - 1))
      {
         char buf[MSL];

         /* Rennard */
         if (pArea->resetmsg)
            sprintf(buf, "%s\n\r", pArea->resetmsg);
         else
            strcpy(buf, "You hear some squeaking sounds...\n\r");
         for (pch = first_char; pch; pch = pch->next)
         {
            if (!IS_NPC(pch) && IS_AWAKE(pch) && pch->in_room && pch->in_room->area == pArea)
            {
               set_char_color(AT_RESET, pch);
               send_to_char(buf, pch);
            }
         }
      }

      /*
       * Check age and reset.
       * Note: Mud Academy resets every 3 minutes (not 15).
       */
      if (pArea->nplayer == 0 || pArea->age >= reset_age)
      {
         /* fprintf( stderr, "Resetting: %s\n", pArea->filename ); */
         reset_area(pArea, 0);
         if (reset_age == -1)
            pArea->age = -1;
         else
            pArea->age = number_range(0, reset_age / 5);
      }
      
      for(vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++)
    {
        CHAR_DATA *pch;
        ROOM_INDEX_DATA *room;
        char buf[MSL];
        
        room = get_room_index(vnum);
        
        if(room == NULL)
        {
            continue;
        }
        
        if(xIS_SET(room->room_flags, ROOM_MANANODE))
        {
            if(pArea->reset_frequency < 6)
                room->node_mana += 2;
            if(pArea->reset_frequency >= 6)
                room->node_mana += (pArea->reset_frequency / 3);
                
            fold_area(pArea, pArea->filename, FALSE, 1);
                
            if(room->node_mana <= 100)
            {
                strcpy(buf, "The air shimmers faintly for a second...\n\r");
            }
            if(room->node_mana > 100 && room->node_mana <= 200)
            {
                strcpy(buf, "The air begins to shimmer faintly and does not fade...\n\r");
            }
            if(room->node_mana > 200 && room->node_mana <= 300)
            {
                strcpy(buf, "The air shimmers, fades, and then begins to glow with a dim blue light...\n\r");
            }
            if(room->node_mana > 300 && room->node_mana <= 400)
            {
                strcpy(buf, "The air shimmers with a dim blue light that seems to be increasing in intensity...\n\r");
            }
            if(room->node_mana > 400 && room->node_mana <= 500)
            {
                strcpy(buf, "The air gives off a bright blue light which suddenly focuses into a myriad of tiny bright blue balls that flit around...\n\r");
            }
            if(room->node_mana > 500 && room->node_mana <= 750)
            {
                strcpy(buf, "Tiny bolts of blue lightning arc through the air...\n\r");
            }
            if(room->node_mana > 750 && room->node_mana <= 1000)
            {
                strcpy(buf, "The air is filled with rather large bolts of blue lightning...\n\r");
            }
            if(room->node_mana > 1000 && room->node_mana <= 2000)
            {
                strcpy(buf, "Huge bolts of blue lightning arc slowly through the air...\n\r");
            }
            if(room->node_mana > 2000 && room->node_mana <= 3000)
            {
                strcpy(buf, "Huge bolts of blue lightning arc through the air with increasing frequency...\n\r");
            }
            if(room->node_mana > 3000 && room->node_mana <= 4000)
            {
                strcpy(buf, "Huge bolts of blue lightning arc through the air every few seconds...\n\r");
            }
            if(room->node_mana > 4000 && room->node_mana <= 5000)
            {
                strcpy(buf, "Huge bolts of blue lightning arc contiuously through the air, reflecting off of eachother in all directions...\n\r");
            }
            if(room->node_mana > 5000 && room->node_mana <= 10000)
            {
                strcpy(buf, "The air shimmers a deep blue and crackles uncontrollably...\n\r");
            }
            if(room->node_mana > 10000)
            {
                strcpy(buf, "Suddenly a sense of power begins to emanate from the very air, and you can feel it throbbing in your ears and against your skin...\n\r");
            }
            
            for(pch = first_char; pch; pch = pch->next)
            {
                if(!IS_NPC(pch) && IS_AWAKE(pch) && pch->in_room == room)
                {
                    set_char_color(AT_MAGIC, pch);
                    send_to_char(buf, pch);
                }
            }
                
        }
      }

   }
   return;
}


/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mobile(MOB_INDEX_DATA * pMobIndex)
{
   CHAR_DATA *mob;

   if (!pMobIndex)
   {
      bug("Create_mobile: NULL pMobIndex.");
      exit(1);
   }

   CREATE(mob, CHAR_DATA, 1);
   clear_char(mob);
   if (!mob->coord)
      CREATE(mob->coord, COORD_DATA, 1);
   mob->pIndexData = pMobIndex;

   mob->editor = NULL;
   mob->name = QUICKLINK(pMobIndex->player_name);
   mob->short_descr = QUICKLINK(pMobIndex->short_descr);
   mob->long_descr = QUICKLINK(pMobIndex->long_descr);
   mob->description = QUICKLINK(pMobIndex->description);
   mob->spec_fun = pMobIndex->spec_fun;
   mob->mpscriptpos = 0;
   mob->level = 0;
   mob->con_rarm = 1000;
   mob->con_larm = 1000;
   mob->con_rleg = 1000;
   mob->con_lleg = 1000;
/*    mob->level			= number_fuzzy( pMobIndex->level ); */
   mob->act = pMobIndex->act;
   mob->miflags = pMobIndex->miflags;
   mob->map = -1;
   mob->coord->x = -1;
   mob->coord->y = -1;
   mob->grip = GRIP_BASH;

   if (xIS_SET(mob->act, ACT_MOBINVIS))
      mob->mobinvis = mob->level;

   mob->affected_by = pMobIndex->affected_by;
   mob->alignment = pMobIndex->alignment;
   mob->sex = pMobIndex->sex;

   /*
    * Bug fix from mailing list by stu (sprice@ihug.co.nz)
    * was:  if ( !pMobIndex->ac )
    */

   if (!pMobIndex->hitnodice)
      mob->max_hit = 100;
   else
      mob->max_hit = pMobIndex->hitnodice * number_range(1, pMobIndex->hitsizedice) + pMobIndex->hitplus;
   mob->hit = mob->max_hit;
   mob->max_move = 1000;
   mob->move = mob->max_move;
   /* lets put things back the way they used to be! -Thoric */
   mob->gold = pMobIndex->gold;
   mob->position = pMobIndex->position;
   mob->defposition = pMobIndex->defposition;
   mob->barenumdie = pMobIndex->damnodice;
   mob->baresizedie = pMobIndex->damsizedice;
   mob->armor = pMobIndex->ac;
   mob->mobthac0 = pMobIndex->mobthac0;
   mob->hitplus = pMobIndex->hitplus;
   mob->damplus = pMobIndex->damplus;
   mob->damaddlow = pMobIndex->damaddlow;
   mob->damaddhi = pMobIndex->damaddhi;

   mob->perm_str = pMobIndex->perm_str;
   mob->perm_dex = pMobIndex->perm_dex;
   mob->perm_wis = pMobIndex->perm_wis;
   mob->perm_int = pMobIndex->perm_int;
   mob->perm_con = pMobIndex->perm_con;
   mob->perm_cha = pMobIndex->perm_cha;
   mob->perm_lck = pMobIndex->perm_lck;
   mob->perm_agi = pMobIndex->perm_agi;
   /* Create a copy here for now, might be a security hazard though -- Xerves */
   mob->m1 = pMobIndex->m1;
   mob->m2 = pMobIndex->m2;
   mob->m3 = pMobIndex->m3;
   mob->m4 = pMobIndex->m4;
   mob->m5 = pMobIndex->m5;
   mob->m6 = pMobIndex->m6;
   mob->m7 = pMobIndex->m7;
   mob->m8 = pMobIndex->m8;
   mob->m9 = pMobIndex->m9;
   mob->m10 = pMobIndex->m10;
   mob->m11 = pMobIndex->m11;
   mob->m12 = pMobIndex->m12;
   mob->apply_res_fire[0] = pMobIndex->apply_res_fire;
   mob->apply_res_water[0] = pMobIndex->apply_res_water;
   mob->apply_res_air[0] = pMobIndex->apply_res_air;
   mob->apply_res_earth[0] = pMobIndex->apply_res_earth;
   mob->apply_res_energy[0] = pMobIndex->apply_res_energy;
   mob->apply_res_magic[0] = pMobIndex->apply_res_magic;
   mob->apply_res_nonmagic[0] = pMobIndex->apply_res_nonmagic;
   mob->apply_res_blunt[0] = pMobIndex->apply_res_blunt;
   mob->apply_res_pierce[0] = pMobIndex->apply_res_pierce;
   mob->apply_res_slash[0] = pMobIndex->apply_res_slash;
   mob->apply_res_poison[0] = pMobIndex->apply_res_poison;
   mob->apply_res_paralysis[0] = pMobIndex->apply_res_paralysis;
   mob->apply_res_holy[0] = pMobIndex->apply_res_holy;
   mob->apply_res_unholy[0] = pMobIndex->apply_res_unholy;
   mob->apply_res_undead[0] = pMobIndex->apply_res_undead;
   mob->tohitbash = pMobIndex->tohitbash;
   mob->tohitslash = pMobIndex->tohitslash;
   mob->tohitstab = pMobIndex->tohitstab;
   mob->cident = pMobIndex->cident;
   mob->resx = -1;
   mob->resy = -1;
   mob->resmap = -1;
   mob->stx = -1;
   mob->sty = -1;
   mob->stmap = -1;
   mob->hitroll = pMobIndex->hitroll;
   mob->damroll = pMobIndex->damroll;
   mob->race = pMobIndex->race;
   mob->xflags = pMobIndex->xflags;
   mob->saving_poison_death = pMobIndex->saving_poison_death;
   mob->saving_wand = pMobIndex->saving_wand;
   mob->saving_para_petri = pMobIndex->saving_para_petri;
   mob->saving_breath = pMobIndex->saving_breath;
   mob->saving_spell_staff = pMobIndex->saving_spell_staff;
   mob->height = pMobIndex->height;
   mob->weight = pMobIndex->weight;
   mob->resistant = pMobIndex->resistant;
   mob->immune = pMobIndex->immune;
   mob->susceptible = pMobIndex->susceptible;
   mob->attacks = pMobIndex->attacks;
   mob->defenses = pMobIndex->defenses;
   mob->speaks = pMobIndex->speaks;
   mob->speaking = pMobIndex->speaking;
   
   if (mob->apply_res_fire[0] == 0 && mob->apply_res_water[0] && mob->apply_res_air[0] == 0 && mob->apply_res_earth[0] == 0
   && mob->apply_res_energy[0] == 0 && mob->apply_res_magic[0] == 0 && mob->apply_res_nonmagic[0] == 0 && mob->apply_res_blunt[0] == 0
   && mob->apply_res_pierce[0] == 0 && mob->apply_res_slash[0] == 0 && mob->apply_res_poison[0] == 0 && mob->apply_res_paralysis[0] == 0
   && mob->apply_res_holy[0] == 0 && mob->apply_res_unholy[0] == 0 && mob->apply_res_undead[0] == 0)
   {
      mob->apply_res_fire[0] = 100;
      mob->apply_res_water[0] = 100;
      mob->apply_res_air[0] = 100;
      mob->apply_res_earth[0] = 100;
      mob->apply_res_energy[0] = 100;
      mob->apply_res_magic[0] = 100;
      mob->apply_res_nonmagic[0] = 100;
      mob->apply_res_blunt[0] = 100;
      mob->apply_res_pierce[0] = 100;
      mob->apply_res_slash[0] = 100;
      mob->apply_res_poison[0] = 100;
      mob->apply_res_paralysis[0] = 100;
      mob->apply_res_holy[0] = 100;
      mob->apply_res_unholy[0] = 100;
      mob->apply_res_undead[0] = 100; 
   }

   /*
    * Perhaps add this to the index later --Shaddai
    */
   xCLEAR_BITS(mob->no_affected_by);
   mob->no_resistant = 0;
   mob->no_immune = 0;
   mob->no_susceptible = 0;
   /*
    * Insert in list.
    */
   add_char(mob);
   pMobIndex->count++;
   nummobsloaded++;
   serialmobsloaded++;
   mob->serial = serialmobsloaded;
   serial_list[mob->serial] = TRUE;
   if ((serialmobsloaded * 100 / MAX_LOADED_MOBS) >= 90)
      bug("Serialmobsloaded has reached %d, which is over 90 percent of the total of %d", serialmobsloaded, MAX_LOADED_MOBS);
   if (serialmobsloaded == MAX_LOADED_MOBS)
   {
      bug("Serialmobsloaded has reached the maximum value allowed, would suggest increasing MAX_LOADED_MOBS, till then, shutting down the mud!!!!");
      exit(1);
   }
   return mob;
}



/*
 * Create an instance of an object.
 */
OBJ_DATA *create_object(OBJ_INDEX_DATA * pObjIndex, int level)
{
   OBJ_DATA *obj;

   if (!pObjIndex)
   {
      bug("Create_object: NULL pObjIndex.");
      exit(1);
   }

   CREATE(obj, OBJ_DATA, 1);

   obj->pIndexData = pObjIndex;
   obj->in_room = NULL;
   obj->level = level;
   obj->wear_loc = -1;
   obj->count = 1;
   cur_obj_serial = UMAX((cur_obj_serial + 1) & (BV30 - 1), 1);
   obj->serial = obj->pIndexData->serial = cur_obj_serial;

   obj->name = QUICKLINK(pObjIndex->name);
   obj->short_descr = QUICKLINK(pObjIndex->short_descr);
   obj->description = QUICKLINK(pObjIndex->description);
   obj->action_desc = QUICKLINK(pObjIndex->action_desc);
   obj->item_type = pObjIndex->item_type;
   obj->extra_flags = pObjIndex->extra_flags;
   obj->wear_flags = pObjIndex->wear_flags;
   obj->value[0] = pObjIndex->value[0];
   obj->value[1] = pObjIndex->value[1];
   obj->value[2] = pObjIndex->value[2];
   obj->value[3] = pObjIndex->value[3];
   obj->value[4] = pObjIndex->value[4];
   obj->value[5] = pObjIndex->value[5];
   obj->value[6] = pObjIndex->value[6];
   obj->value[7] = pObjIndex->value[7];
   obj->value[8] = pObjIndex->value[8];
   obj->value[9] = pObjIndex->value[9];
   obj->value[10] = pObjIndex->value[10];
   obj->value[11] = pObjIndex->value[11];
   obj->value[12] = pObjIndex->value[12];
   obj->value[13] = pObjIndex->value[13];
   obj->weight = pObjIndex->weight;
   obj->cost = pObjIndex->cost;
   CREATE(obj->coord, COORD_DATA, 1);
   obj->coord->x = -1;
   obj->coord->y = -1;
   obj->map = -1;
   obj->cvnum = pObjIndex->cvnum;
   obj->cident = pObjIndex->cident;
   obj->sworthrestrict = pObjIndex->sworthrestrict;
   obj->imbueslots = pObjIndex->imbueslots;
   /*
      obj->cost  = number_fuzzy( 10 )
      * number_fuzzy( level ) * number_fuzzy( level );
    */

   /*
    * Mess with object properties.
    */
   switch (obj->item_type)
   {
      default:
         bug("Read_object: vnum %d bad type.", pObjIndex->vnum);
         bug("------------------------>     ", obj->item_type);
         break;

      case ITEM_LIGHT:
      case ITEM_TREASURE:
      case ITEM_FURNITURE:
      case ITEM_TRASH:
      case ITEM_CONTAINER:
      case ITEM_DRINK_CON:
      case ITEM_KEY:
      case ITEM_KEYRING:
      case ITEM_ODOR:
      case ITEM_HOLDRESOURCE:
      case ITEM_EXTRACTOBJ:
      case ITEM_SPELLBOOK:
      case ITEM_SHEATH:
         break;
      case ITEM_COOK:
      case ITEM_FOOD:
         /*
          * optional food condition (rotting food)  -Thoric
          * value1 is the max condition of the food
          * value4 is the optional initial condition
          */
         if (obj->value[4])
            obj->timer = obj->value[4];
         else
            obj->timer = obj->value[1];
         break;
      case ITEM_BOAT:
      case ITEM_NOTEBOARD:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
      case ITEM_FOUNTAIN:
      case ITEM_BLOOD:
      case ITEM_BLOODSTAIN:
      case ITEM_SCRAPS:
      case ITEM_PIPE:
      case ITEM_HERB_CON:
      case ITEM_HERB:
      case ITEM_INCENSE:
      case ITEM_FIRE:
      case ITEM_BOOK:
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
      case ITEM_BUTTON:
      case ITEM_DIAL:
      case ITEM_RUNE:
      case ITEM_RUNEPOUCH:
      case ITEM_MATCH:
      case ITEM_TRAP:
      case ITEM_MAP:
      case ITEM_PORTAL:
      case ITEM_PAPER:
      case ITEM_PEN:
      case ITEM_TINDER:
      case ITEM_LOCKPICK:
      case ITEM_SPIKE:
      case ITEM_DISEASE:
      case ITEM_OIL:
      case ITEM_FUEL:
      case ITEM_QUIVER:
      case ITEM_PILL:
      case ITEM_SHOVEL:
      case ITEM_WAND:
      case ITEM_STAFF:
      case ITEM_MOUNTFOOD:
      case ITEM_ARMOR:
      case ITEM_REPAIR:
      case ITEM_MCLIMB:
      case ITEM_GAG:
      case ITEM_QTOKEN:
      case ITEM_FLINT:
      case ITEM_TRAPTOOL:
      case ITEM_TGEM:
         break;

      case ITEM_SCROLL:
         obj->value[0] = number_fuzzy(obj->value[0]);
         break;

      case ITEM_WEAPON:
      case ITEM_MISSILE_WEAPON:
         if (!obj->value[1] || !obj->value[2])
         {
            obj->value[1] = 1;
            obj->value[2] = 2;
         }
         if (obj->value[0] == 0)
            obj->value[0] = INIT_ARMOR_CONDITION;

         if (obj->wear_loc > WEAR_NOCKED)
            obj->wear_loc = WEAR_NONE;
         break;

      case ITEM_PROJECTILE:
         if (!obj->value[1] || !obj->value[2])
         {
            obj->value[1] = 1;
            obj->value[2] = 2;
         }
         if (obj->value[0] == 0)
            obj->value[0] = INIT_ARMOR_CONDITION;
         break;

      case ITEM_POTION:
         obj->value[0] = number_fuzzy(number_fuzzy(obj->value[0]));
         break;

      case ITEM_MONEY:
         obj->value[0] = obj->cost;
         if (obj->value[0] == 0)
            obj->value[0] = 1;
         break;
   }

   LINK(obj, first_object, last_object, next, prev);
   ++pObjIndex->count;
   ++numobjsloaded;
   ++physicalobjects;

   return obj;
}


/*
 * Clear a new character.
 */
void clear_char(CHAR_DATA * ch)
{
   ch->editor = NULL;
   ch->hunting = NULL;
   ch->fearing = NULL;
   ch->hating = NULL;
   ch->name = NULL;
   ch->short_descr = NULL;
   ch->long_descr = NULL;
   ch->description = NULL;
   ch->tone = NULL;
   ch->movement = NULL;
   ch->next = NULL;
   ch->prev = NULL;
   ch->reply = NULL;
   ch->retell = NULL;
   ch->first_carrying = NULL;
   ch->last_carrying = NULL;
   ch->next_in_room = NULL;
   ch->prev_in_room = NULL;
   ch->fighting = NULL;
   ch->switched = NULL;
   ch->first_affect = NULL;
   ch->last_affect = NULL;
   ch->prev_cmd = NULL; /* maps */
   ch->last_cmd = NULL;
   ch->dest_buf = NULL;
   ch->alloc_ptr = NULL;
   ch->spare_ptr = NULL;
   ch->mount = NULL;
   ch->morph = NULL;
   xCLEAR_BITS(ch->affected_by);
   ch->armor = 0;
   ch->position = POS_STANDING;
   ch->practice = 0;
   ch->hit = 20;
   ch->max_hit = 20;
   ch->mana = 100;
   ch->max_mana = 100;
   ch->move = 1000;
   ch->max_move = 1000;
   ch->height = 72;
   ch->weight = 180;
   ch->xflags = 0;
   ch->race = 0;
   ch->speaking = LANG_COMMON;
   ch->speaks = LANG_COMMON;
   ch->barenumdie = 1;
   ch->baresizedie = 4;
   ch->substate = 0;
   ch->tempnum = 0;
   ch->perm_str = 13;
   ch->perm_dex = 13;
   ch->perm_int = 13;
   ch->perm_wis = 13;
   ch->perm_cha = 13;
   ch->perm_con = 13;
   ch->perm_lck = 13;
   ch->perm_agi = 15;
   ch->apply_res_fire[0] = 100;
   ch->apply_res_water[0] = 100;
   ch->apply_res_air[0] = 100;
   ch->apply_res_earth[0] = 100;
   ch->apply_res_energy[0] = 100;
   ch->apply_res_magic[0] = 100;
   ch->apply_res_nonmagic[0] = 100;
   ch->apply_res_blunt[0] = 100;
   ch->apply_res_pierce[0] = 100;
   ch->apply_res_slash[0] = 100;
   ch->apply_res_poison[0] = 100;
   ch->apply_res_paralysis[0] = 100;
   ch->apply_res_holy[0] = 100;
   ch->apply_res_unholy[0] = 100;
   ch->apply_res_undead[0] = 100;
   ch->apply_res_fire[1] = 100;
   ch->apply_res_water[1] = 100;
   ch->apply_res_air[1] = 100;
   ch->apply_res_earth[1] = 100;
   ch->apply_res_energy[1] = 100;
   ch->apply_res_magic[1] = 100;
   ch->apply_res_nonmagic[1] = 100;
   ch->apply_res_blunt[1] = 100;
   ch->apply_res_pierce[1] = 100;
   ch->apply_res_slash[1] = 100;
   ch->apply_res_poison[1] = 100;
   ch->apply_res_paralysis[1] = 100;
   ch->apply_res_holy[1] = 100;
   ch->apply_res_unholy[1] = 100;
   ch->apply_res_undead[1] = 100;
   ch->apply_res_fire[2] = 100;
   ch->apply_res_water[2] = 100;
   ch->apply_res_air[2] = 100;
   ch->apply_res_earth[2] = 100;
   ch->apply_res_energy[2] = 100;
   ch->apply_res_magic[2] = 100;
   ch->apply_res_nonmagic[2] = 100;
   ch->apply_res_blunt[2] = 100;
   ch->apply_res_pierce[2] = 100;
   ch->apply_res_slash[2] = 100;
   ch->apply_res_poison[2] = 100;
   ch->apply_res_paralysis[2] = 100;
   ch->apply_res_holy[2] = 100;
   ch->apply_res_unholy[2] = 100;
   ch->apply_res_undead[2] = 100;
   ch->mod_str = 0;
   ch->mod_dex = 0;
   ch->mod_int = 0;
   ch->mod_wis = 0;
   ch->mod_cha = 0;
   ch->mod_con = 0;
   ch->mod_lck = 0;
   ch->mod_agi = 0;
   ch->mover = 0;
   CREATE(ch->coord, COORD_DATA, 1); /* Overland Map - Samson 7-31-99 */
   ch->coord->x = -1;
   ch->coord->y = -1;
   ch->map = -1;
   return;
}

void free_runbuf(DESCRIPTOR_DATA * d)
{
   if (d && d->run_buf)
   {
      DISPOSE(d->run_buf);
      d->run_buf = NULL;
      d->run_head = NULL;
   }
   return;
}

/*
 * Free a character.
 */
void free_char(CHAR_DATA * ch)
{
   OBJ_DATA *obj;
   AFFECT_DATA *paf;
   TIMER *timer;
   MPROG_ACT_LIST *mpact, *mpact_next;
   NOTE_DATA *comments, *comments_next;
   AGGRO_DATA *aggro;
   AGGRO_DATA *naggro;

   if (!ch)
   {
      bug("Free_char: null ch!");
      return;
   }

   if (ch->desc)
      bug("Free_char: char still has descriptor.");

   if (ch->morph)
      free_char_morph(ch->morph);

   while ((obj = ch->last_carrying) != NULL)
      extract_obj(obj);

   while ((paf = ch->last_affect) != NULL)
      affect_remove(ch, paf);

   while ((timer = ch->first_timer) != NULL)
      extract_timer(ch, timer);

   if (ch->editor)
      stop_editing(ch);

   STRFREE(ch->name);
   STRFREE(ch->short_descr);
   STRFREE(ch->long_descr);
   STRFREE(ch->description);
   
   if (ch->movement)
      STRFREE(ch->movement);
   if (ch->tone)
      STRFREE(ch->tone);

   stop_hunting(ch);
   stop_hating(ch);
   stop_fearing(ch);
   free_fight(ch);
   
   for (aggro = ch->first_aggro; aggro; aggro = naggro)
   {
      naggro = aggro->next;
      UNLINK(aggro, first_global_aggro, last_global_aggro, next_global, prev_global);
      UNLINK(aggro, ch->first_aggro, ch->last_aggro, next, prev);
      DISPOSE(aggro);
   }

   if (ch->pnote)
      free_note(ch->pnote);

   if (ch->coord)
      DISPOSE(ch->coord);
   if (ch->midata)
   {
      STRFREE(ch->midata->command);
      DISPOSE(ch->midata);
   }
   if (ch->pcdata)
   {
      IGNORE_DATA *temp, *next;

      /* free up memory allocated to stored ignored names */
      for (temp = ch->pcdata->first_ignored; temp; temp = next)
      {
         next = temp->next;
         UNLINK(temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev);
         STRFREE(temp->name);
         DISPOSE(temp);
      }

      STRFREE(ch->pcdata->filename);
      STRFREE(ch->pcdata->deity_name);
      STRFREE(ch->pcdata->clan_name);
      STRFREE(ch->pcdata->council_name);
      if (ch->pcdata->pwd)
         DISPOSE(ch->pcdata->pwd); /* no hash */
      if (ch->pcdata->autocommand)
         STRFREE(ch->pcdata->autocommand);
      DISPOSE(ch->pcdata->bamfin); /* no hash */
      DISPOSE(ch->pcdata->bamfout); /* no hash */
      DISPOSE(ch->pcdata->rank);
      STRFREE(ch->pcdata->title);
      if (ch->pcdata->bio)
         STRFREE(ch->pcdata->bio);
      if (ch->pcdata->rreply)
         DISPOSE(ch->pcdata->rreply); /* no hash */
      if (ch->pcdata->rreply_name)
         DISPOSE(ch->pcdata->rreply_name); /* no hash */
      DISPOSE(ch->pcdata->bestowments); /* no hash */
      if (ch->pcdata->homepage)
         DISPOSE(ch->pcdata->homepage); /* no hash */
      if (ch->pcdata->pretit)
         DISPOSE(ch->pcdata->pretit); /* no hash/Memory Leak -- Xerves */
      if (ch->pcdata->email)
         DISPOSE(ch->pcdata->email); /* no hash */
      STRFREE(ch->pcdata->authed_by);
      STRFREE(ch->pcdata->prompt);
      STRFREE(ch->pcdata->fprompt);
      if (ch->pcdata->helled_by)
         STRFREE(ch->pcdata->helled_by);
      if (ch->pcdata->subprompt)
         STRFREE(ch->pcdata->subprompt);
      free_aliases(ch);
      if (ch->pcdata->tell_history)
      {
         int i;

         for (i = 0; i < 26; i++)
         {
            if (ch->pcdata->tell_history[i])
               STRFREE(ch->pcdata->tell_history[i]);
         }
         DISPOSE(ch->pcdata->tell_history);
      }
      if (ch->pcdata->ice_listen)
         DISPOSE(ch->pcdata->ice_listen);
      DISPOSE(ch->pcdata);
   }

   for (mpact = ch->mpact; mpact; mpact = mpact_next)
   {
      mpact_next = mpact->next;
      DISPOSE(mpact->buf);
      DISPOSE(mpact);
   }

   for (comments = ch->comments; comments; comments = comments_next)
   {
      comments_next = comments->next;
      STRFREE(comments->text);
      STRFREE(comments->to_list);
      STRFREE(comments->subject);
      STRFREE(comments->sender);
      STRFREE(comments->date);
      DISPOSE(comments);
   }
   DISPOSE(ch);
   return;
}



/*
 * Get an extra description from a list.
 */
char *get_extra_descr(const char *name, EXTRA_DESCR_DATA * ed)
{
   for (; ed; ed = ed->next)
      if (is_name(name, ed->keyword))
         return ed->description;

   return NULL;
}



/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(sh_int vnum)
{
   MOB_INDEX_DATA *pMobIndex;

   if (vnum < 0)
      vnum = 0;

   for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH]; pMobIndex; pMobIndex = pMobIndex->next)
      if (pMobIndex->vnum == vnum)
         return pMobIndex;

   if (fBootDb)
      bug("Get_mob_index: bad vnum %d.", vnum);

   return NULL;
}



/*
 * Translates obj virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum)
{
   OBJ_INDEX_DATA *pObjIndex;

   if (vnum < 0)
      vnum = 0;

   for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH]; pObjIndex; pObjIndex = pObjIndex->next)
      if (pObjIndex->vnum == vnum)
         return pObjIndex;

   if (fBootDb)
      bug("Get_obj_index: bad vnum %d.", vnum);

   return NULL;
}



/*
 * Translates room virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum)
{
   ROOM_INDEX_DATA *pRoomIndex;

   if (vnum < 0)
      vnum = 0;

   for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH]; pRoomIndex; pRoomIndex = pRoomIndex->next)
      if (pRoomIndex->vnum == vnum)
         return pRoomIndex;

   if (fBootDb)
      bug("Get_room_index: bad vnum %d.", vnum);

   return NULL;
}



/*
 * Added lots of EOF checks, as most of the file crashes are based on them.
 * If an area file encounters EOF, the fread_* functions will shutdown the
 * MUD, as all area files should be read in in full or bad things will
 * happen during the game.  Any files loaded in without fBootDb which
 * encounter EOF will return what they have read so far.   These files
 * should include player files, and in-progress areas that are not loaded
 * upon bootup.
 * -- Altrag
 */


/*
 * Read a letter from a file.
 */
char fread_letter(FILE * fp)
{
   char c;

   do
   {
      if (feof(fp))
      {
         bug("fread_letter: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return '\0';
      }
      c = getc(fp);
   }
   while (isspace(c));

   return c;
}



/*
 * Read a number from a file.
 */
int fread_number(FILE * fp)
{
   int number;
   bool sign;
   char c;

   do
   {
      if (feof(fp))
      {
         bug("fread_number: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return 0;
      }
      c = getc(fp);
   }
   while (isspace(c));

   number = 0;

   sign = FALSE;
   if (c == '+')
   {
      c = getc(fp);
   }
   else if (c == '-')
   {
      sign = TRUE;
      c = getc(fp);
   }

   if (!isdigit(c))
   {
      bug("Fread_number: bad format. (%c)", c);
      if (fBootDb)
         exit(1);
      return 0;
   }

   while (isdigit(c))
   {
      if (feof(fp))
      {
         bug("fread_number: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return number;
      }
      number = number * 10 + c - '0';
      c = getc(fp);
   }

   if (sign)
      number = 0 - number;

   if (c == '|')
      number += fread_number(fp);
   else if (c != ' ')
      ungetc(c, fp);

   return number;
}

/*
 * Read a float from a file.
 */
float fread_float(FILE * fp)
{
   float number;
   bool sign;
   char c;
   char buf[MSL];
   int x = 0;

   do
   {
      if (feof(fp))
      {
         bug("fread_number: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return 0;
      }
      c = getc(fp);
   }
   while (isspace(c));

   number = 0;

   sign = FALSE;
   if (c == '+')
   {
      c = getc(fp);
   }
   else if (c == '-')
   {
      sign = TRUE;
      c = getc(fp);
   }

   if (!isdigit(c) && c != '.')
   {
      bug("Fread_number: bad format. (%c)", c);
      if (fBootDb)
         exit(1);
      return 0;
   }

   while (isdigit(c) || c == '.')
   {
      if (feof(fp))
      {
         bug("fread_number: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return number;
      }
      buf[x++] = c;
      c = getc(fp);
   }
   buf[x] = '\0';
   number = atof(buf);

   if (sign)
      number = 0 - number;

   if (c == '|')
      number += fread_float(fp);
   else if (c != ' ')
      ungetc(c, fp);

   return number;
}

/*
 * custom str_dup using create					-Thoric
 */
 
#ifdef MTRACE
char *_str_dup( const char *str, char *file_n, char *funct_n, int line_n )
#else
char *str_dup(char const *str)
#endif
{
   static char *ret;
   int len;
   #ifdef MTRACE
   FILE *fp;
   #endif

   if (!str)
      return NULL;

   len = strlen(str) + 1;

   CREATE(ret, char, len);
   strcpy(ret, str);
   #ifdef MTRACE
   fp = fopen( "str_dup.log", "a" );
   fprintf( fp, "%p:%s:%s:%d: %s\n",ret, file_n, funct_n, line_n, ret ); 
   fclose( fp );
   #endif
   return ret;
}

/*
 * Read a string from file fp
 */
char *fread_string(FILE * fp)
{
   char buf[MSL];
   char *plast;
   char c;
   int ln;

   plast = buf;
   buf[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if (feof(fp))
      {
         bug("fread_string: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return STRALLOC("");
      }
      c = getc(fp);
   }
   while (isspace(c));

   if ((*plast++ = c) == '~')
      return STRALLOC("");

   for (;;)
   {
      if (ln >= (MSL - 1))
      {
         bug("fread_string: string too long");
         *plast = '\0';
         return STRALLOC(buf);
      }
      switch (*plast = getc(fp))
      {
         default:
            plast++;
            ln++;
            break;

         case EOF:
            bug("Fread_string: EOF");
            if (fBootDb)
               exit(1);
            *plast = '\0';
            return STRALLOC(buf);
            break;

         case '\n':
            plast++;
            ln++;
            *plast++ = '\r';
            ln++;
            break;

         case '\r':
            break;

         case '~':
            *plast = '\0';
            return STRALLOC(buf);
      }
   }
}

/*
 * Read a string from file fp using str_dup (ie: no string hashing)
 */
char *fread_string_nohash(FILE * fp)
{
   char buf[MSL];
   char *plast;
   char c;
   int ln;

   plast = buf;
   buf[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if (feof(fp))
      {
         bug("fread_string_no_hash: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return str_dup("");
      }
      c = getc(fp);
   }
   while (isspace(c));

   if ((*plast++ = c) == '~')
      return str_dup("");

   for (;;)
   {
      if (ln >= (MSL - 1))
      {
         bug("fread_string_no_hash: string too long");
         *plast = '\0';
         return str_dup(buf);
      }
      switch (*plast = getc(fp))
      {
         default:
            plast++;
            ln++;
            break;

         case EOF:
            bug("Fread_string_no_hash: EOF");
            if (fBootDb)
               exit(1);
            *plast = '\0';
            return str_dup(buf);
            break;

         case '\n':
            plast++;
            ln++;
            *plast++ = '\r';
            ln++;
            break;

         case '\r':
            break;

         case '~':
            *plast = '\0';
            return str_dup(buf);
      }
   }
}



/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE * fp)
{
   char c;

   do
   {
      if (feof(fp))
      {
         bug("fread_to_eol: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         return;
      }
      c = getc(fp);
   }
   while (c != '\n' && c != '\r');

   do
   {
      c = getc(fp);
   }
   while (c == '\n' || c == '\r');

   ungetc(c, fp);
   return;
}

/*
 * Read to end of line into static buffer			-Thoric
 */
char *fread_line(FILE * fp)
{
   static char line[MSL];
   char *pline;
   char c;
   int ln;

   pline = line;
   line[0] = '\0';
   ln = 0;

   /*
    * Skip blanks.
    * Read first char.
    */
   do
   {
      if (feof(fp))
      {
         bug("fread_line: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         strcpy(line, "");
         return line;
      }
      c = getc(fp);
   }
   while (isspace(c));

   ungetc(c, fp);
   do
   {
      if (feof(fp))
      {
         bug("fread_line: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         *pline = '\0';
         return line;
      }
      c = getc(fp);
      *pline++ = c;
      ln++;
      if (ln >= (MSL - 1))
      {
         bug("fread_line: line too long");
         break;
      }
   }
   while (c != '\n' && c != '\r');

   do
   {
      c = getc(fp);
   }
   while (c == '\n' || c == '\r');

   ungetc(c, fp);
   *pline = '\0';
   return line;
}



/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE * fp)
{
   static char word[MIL];
   char *pword;
   char cEnd;

   do
   {
      if (feof(fp))
      {
         bug("fread_word: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         word[0] = '\0';
         return word;
      }
      cEnd = getc(fp);
   }
   while (isspace(cEnd));

   if (cEnd == '\'' || cEnd == '"')
   {
      pword = word;
   }
   else
   {
      word[0] = cEnd;
      pword = word + 1;
      cEnd = ' ';
   }

   for (; pword < word + MIL; pword++)
   {
      if (feof(fp))
      {
         bug("fread_word: EOF encountered on read.\n\r");
         if (fBootDb)
            exit(1);
         *pword = '\0';
         return word;
      }
      *pword = getc(fp);
      if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd)
      {
         if (cEnd == ' ')
            ungetc(*pword, fp);
         *pword = '\0';
         return word;
      }
   }

   bug("Fread_word: word too long");
   exit(1);
   return NULL;
}

void do_memory(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   int hash;

   set_char_color(AT_PLAIN, ch);
   argument = one_argument(argument, arg);
   send_to_char_color("\n\r&wSystem Memory [arguments - hash, check, showhigh]\n\r", ch);
   ch_printf_color(ch, "&wAffects: &W%5d\t\t\t&wAreas:   &W%5d\n\r", top_affect, top_area);
   ch_printf_color(ch, "&wExtDes:  &W%5d\t\t\t&wExits:   &W%5d\n\r", top_ed, top_exit);
   ch_printf_color(ch, "&wHelps:   &W%5d\t\t\t&wResets:  &W%5d\n\r", top_help, top_reset);
   ch_printf_color(ch, "&wIdxMobs: &W%5d\t\t\t&wMobiles: &W%5d\n\r", top_mob_index, nummobsloaded);
   ch_printf_color(ch, "&wIdxObjs: &W%5d\t\t\t&wObjs:    &W%5d(%d)\n\r", top_obj_index, numobjsloaded, physicalobjects);
   ch_printf_color(ch, "&wRooms:   &W%5d\t\t\t&wVRooms:  &W%5d\n\r", top_room, top_vroom);
   ch_printf_color(ch, "&wShops:   &W%5d\t\t\t&wRepShps: &W%5d\n\r", top_shop, top_repair);
   ch_printf_color(ch, "&wCurOq's: &W%5d\t\t\t&wCurCq's: &W%5d\n\r", cur_qobjs, cur_qchars);
   ch_printf_color(ch, "&wPlayers: &W%5d\t\t\t&wMaxplrs: &W%5d\n\r", num_descriptors, sysdata.maxplayers);
   ch_printf_color(ch, "&wMaxEver: &W%5d\t\t\t&wTopsn:   &W%5d(%d)\n\r", sysdata.alltimemax, top_sn, MAX_SKILL);
   ch_printf_color(ch, "&wMaxEver was recorded on:  &W%s\n\r\n\r", sysdata.time_of_max);
   ch_printf_color(ch, "&wPotion Val:  &W%-16d   &wScribe/Brew: &W%d/%d\n\r", sysdata.upotion_val, sysdata.scribed_used, sysdata.brewed_used);
   ch_printf_color(ch, "&wPill Val:    &W%-16d   &wGlobal loot: &W%d\n\r", sysdata.upill_val, sysdata.global_looted);
   ch_printf_color(ch, "&wMap Mobiles: &G%-16d   &wSerialCnt:   &W%d\n\r", top_map_mob, serialmobsloaded);
   ch_printf_color(ch, "&WTrans X :    &W%-3d                &wTrans Y : &W%-3d\n\r", global_x[0], global_y[0]);


   if (!str_cmp(arg, "check"))
   {
#ifdef HASHSTR
      send_to_char(check_hash(argument), ch);
#else
      send_to_char("Hash strings not enabled.\n\r", ch);
#endif
      return;
   }
   if (!str_cmp(arg, "showhigh"))
   {
#ifdef HASHSTR
      show_high_hash(atoi(argument));
#else
      send_to_char("Hash strings not enabled.\n\r", ch);
#endif
      return;
   }
   if (argument[0] != '\0')
      hash = atoi(argument);
   else
      hash = -1;
   if (!str_cmp(arg, "hash"))
   {
#ifdef HASHSTR
      ch_printf(ch, "Hash statistics:\n\r%s", hash_stats());
      if (hash != -1)
         hash_dump(hash);
#else
      send_to_char("Hash strings not enabled.\n\r", ch);
#endif
   }
   return;
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number)
{
   switch (number_bits(2))
   {
      case 0:
         number -= 1;
         break;
      case 3:
         number += 1;
         break;
   }

   return UMAX(1, number);
}



/*
 * Generate a random number.
 */
int number_range(int from, int to)
{
   if ((to - from) < 1)
      return from;
   return ((number_mm() % (to - from + 1)) + from);
}

sh_int snumber_range(sh_int from, sh_int to)
{
   if ((to - from) < 1)
      return from;
   return ((snumber_mm() % (to - from + 1)) + from);
}

/*
 * Generate a percentile roll.
 */
int number_percent(void)
{
/*    int percent;

    while ( ( percent = number_mm( ) & (128-1) ) > 99 )
	;

    return 1 + percent;*/
   return (number_mm() % 100) + 1;
}



/*
 * Generate a random door.
 */
int number_door(void)
{
   int door;

   while ((door = number_mm() & (16 - 1)) > 9)
      ;

   return door;
/*    return number_mm() & 10; */
}



int number_bits(int width)
{
   return number_mm() & ((1 << width) - 1);
}



/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */
static int rgiState[2 + 55];

void init_mm()
{
   int *piState;
   int iState;

   piState = &rgiState[2];

   piState[-2] = 55 - 55;
   piState[-1] = 55 - 24;

   piState[0] = ((int) current_time) & ((1 << 30) - 1);
   piState[1] = 1;
   for (iState = 2; iState < 55; iState++)
   {
      piState[iState] = (piState[iState - 1] + piState[iState - 2]) & ((1 << 30) - 1);
   }
   return;
}



int number_mm(void)
{
   int *piState;
   int iState1;
   int iState2;
   int iRand;

   piState = &rgiState[2];
   iState1 = piState[-2];
   iState2 = piState[-1];
   iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
   piState[iState1] = iRand;
   if (++iState1 == 55)
      iState1 = 0;
   if (++iState2 == 55)
      iState2 = 0;
   piState[-2] = iState1;
   piState[-1] = iState2;
   return iRand >> 6;
}

sh_int snumber_mm(void)
{
   int *piState;
   sh_int iState1;
   sh_int iState2;
   sh_int iRand;

   piState = &rgiState[2];
   iState1 = piState[-2];
   iState2 = piState[-1];
   iRand = (piState[iState1] + piState[iState2]) & ((1 << 30) - 1);
   piState[iState1] = iRand;
   if (++iState1 == 55)
      iState1 = 0;
   if (++iState2 == 55)
      iState2 = 0;
   piState[-2] = iState1;
   piState[-1] = iState2;
   return iRand >> 6;
}

/*
 * Roll some dice.						-Thoric
 */
int dice(int number, int size)
{
   int idice;
   int sum;

   switch (size)
   {
      case 0:
         return 0;
      case 1:
         return number;
   }

   for (idice = 0, sum = 0; idice < number; idice++)
      sum += number_range(1, size);

   return sum;
}

/*
 * Same thing as dice, but use sh_int format
 */
sh_int sdice(sh_int number, sh_int size)
{
   sh_int idice;
   sh_int sum;

   switch (size)
   {
      case 0:
         return 0;
      case 1:
         return number;
   }

   for (idice = 0, sum = 0; idice < number; idice++)
      sum += snumber_range(1, size);

   return sum;
}



/*
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32)
{
   return value_00 + level * (value_32 - value_00) / 32;
}


/*
 * Removes the tildes from a string.
 * Used for player-entered strings that go into disk files.
 */
void smash_tilde(char *str)
{
   for (; *str != '\0'; str++)
      if (*str == '~')
         *str = '-';

   return;
}

/*
 * Encodes the tildes in a string.				-Thoric
 * Used for player-entered strings that go into disk files.
 */
void hide_tilde(char *str)
{
   for (; *str != '\0'; str++)
      if (*str == '~')
         *str = HIDDEN_TILDE;

   return;
}

char *show_tilde(char *str)
{
   static char buf[MSL];
   char *bufptr;

   bufptr = buf;
   for (; *str != '\0'; str++, bufptr++)
   {
      if (*str == HIDDEN_TILDE)
         *bufptr = '~';
      else
         *bufptr = *str;
   }
   *bufptr = '\0';

   return buf;
}



/*
 * Compare strings, case insensitive.
 * Return TRUE if different
 *   (compatibility with historical functions).
 */
bool str_cmp(const char *astr, const char *bstr)
{
   if (!astr)
   {
      //bug("Str_cmp: null astr.");
      if (bstr)
         fprintf(stderr, "str_cmp: astr: (null)  bstr: %s\n", bstr);
      return TRUE;
   }

   if (!bstr)
   {
      //bug("Str_cmp: null bstr.");
      if (astr)
         fprintf(stderr, "str_cmp: astr: %s  bstr: (null)\n", astr);
      return TRUE;
   }

   for (; *astr || *bstr; astr++, bstr++)
   {
      if (LOWER(*astr) != LOWER(*bstr))
         return TRUE;
   }

   return FALSE;
}



/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr)
{
   if (!astr)
   {
      //bug("Strn_cmp: null astr.");
      return TRUE;
   }

   if (!bstr)
   {
      //bug("Strn_cmp: null bstr.");
      return TRUE;
   }

   for (; *astr; astr++, bstr++)
   {
      if (LOWER(*astr) != LOWER(*bstr))
         return TRUE;
   }

   return FALSE;
}



/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr)
{
   int sstr1;
   int sstr2;
   int ichar;
   char c0;

   if ((c0 = LOWER(astr[0])) == '\0')
      return FALSE;

   sstr1 = strlen(astr);
   sstr2 = strlen(bstr);

   for (ichar = 0; ichar <= sstr2 - sstr1; ichar++)
      if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
         return FALSE;

   return TRUE;
}



/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr)
{
   int sstr1;
   int sstr2;

   sstr1 = strlen(astr);
   sstr2 = strlen(bstr);
   if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
      return FALSE;
   else
      return TRUE;
}



/*
 * Returns an initial-capped string.
 */
char *capitalize(const char *str)
{
   static char strcap[MSL];
   int i;

   for (i = 0; str[i] != '\0'; i++)
      strcap[i] = LOWER(str[i]);
   strcap[i] = '\0';
   strcap[0] = UPPER(strcap[0]);
   return strcap;
}


/*
 * Returns a lowercase string.
 */
char *strlower(const char *str)
{
   static char strlow[MSL];
   int i;

   for (i = 0; str[i] != '\0'; i++)
      strlow[i] = LOWER(str[i]);
   strlow[i] = '\0';
   return strlow;
}

/*
 * Returns an uppercase string.
 */
char *strupper(const char *str)
{
   static char strup[MSL];
   int i;

   for (i = 0; str[i] != '\0'; i++)
      strup[i] = UPPER(str[i]);
   strup[i] = '\0';
   return strup;
}

/*
 * Returns TRUE or FALSE if a letter is a vowel			-Thoric
 */
bool isavowel(char letter)
{
   char c;

   c = LOWER(letter);
   if (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u')
      return TRUE;
   else
      return FALSE;
}

/*
 * Shove either "a " or "an " onto the beginning of a string	-Thoric
 */
char *aoran(const char *str)
{
   static char temp[MSL];

   if (!str)
   {
      bug("Aoran(): NULL str");
      return "";
   }

   if (isavowel(str[0]) || (strlen(str) > 1 && LOWER(str[0]) == 'y' && !isavowel(str[1])))
      strcpy(temp, "an ");
   else
      strcpy(temp, "a ");
   strcat(temp, str);
   return temp;
}


/*
 * Append a string to a file.
 */
void append_file(CHAR_DATA * ch, char *file, char *str)
{
   FILE *fp;

   if (IS_NPC(ch) || str[0] == '\0')
      return;

   fclose(fpLOG);
   if ((fp = fopen(file, "a")) == NULL)
   {
      perror(file);
      send_to_char("Could not open the file!\n\r", ch);
   }
   else
   {
      fprintf(fp, "[%5d] %s: %s\n", ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
      fclose(fp);
   }

   fpLOG = fopen(NULL_FILE, "r");
   return;
}

/*
 * Append a string to a file.
 */
void append_to_file(char *file, char *str)
{
   FILE *fp;

   if ((fp = fopen(file, "a")) == NULL)
      perror(file);
   else
   {
      fprintf(fp, "%s\n", str);
      fclose(fp);
   }

   return;
}


/*
 * Reports a bug.
 */
void bug( const char *str, ... )
{
    char buf[MSL];
    FILE *fp;
    struct stat fst;

    if ( fpArea != NULL )
    {
   	int iLine;
	   int iChar;

   	if ( fpArea == stdin )
	   {
	      iLine = 0;
   	}
	   else
	   {
	      iChar = ftell( fpArea );
	      fseek( fpArea, 0, 0 );
	      for ( iLine = 0; ftell( fpArea ) < iChar; iLine++ )
	      {
	     	while ( getc( fpArea ) != '\n' )
		        ;
	      }
	      fseek( fpArea, iChar, 0 );
	   }

   	sprintf( buf, "[*****] FILE: %s LINE: %d", strArea, iLine );
	   log_string( buf );

   	if ( stat( SHUTDOWN_FILE, &fst ) != -1 )	/* file exists */
	   {
	      if ( ( fp = fopen( SHUTDOWN_FILE, "a" ) ) != NULL )
	      {
	     	fprintf( fp, "[*****] %s\n", buf );
     		fclose( fp );
	      }
	   }
    }

    strcpy( buf, "[*****] BUG: " );
    {
   	va_list param;
    
   	va_start(param, str);
   	vsprintf( buf + strlen(buf), str, param );
	   va_end(param);
    }
    log_string( buf );

    fclose( fpLOG );
    /*if ( ( fp = fopen( BUG_FILE, "a" ) ) != NULL )
    {
   	fprintf( fp, "%s\n", buf );
   	fclose( fp );
    }*/
    fpLOG = fopen( NULL_FILE, "r" );
    return;
}

/*
 * Add a string to the boot-up log				-Thoric
 */
void boot_log(const char *str, ...)
{
   char buf[MSL];
   FILE *fp;
   va_list param;

   strcpy(buf, "[*****] BOOT: ");
   va_start(param, str);
   vsprintf(buf + strlen(buf), str, param);
   va_end(param);
   log_string(buf);

   fclose(fpLOG);
   if ((fp = fopen(BOOTLOG_FILE, "a")) != NULL)
   {
      fprintf(fp, "%s\n", buf);
      fclose(fp);
   }
   fpLOG = fopen(NULL_FILE, "r");

   return;
}

/*
 * Dump a text file to a player, a line at a time		-Thoric
 */
void show_file(CHAR_DATA * ch, char *filename)
{
   FILE *fp;
   char buf[MSL];
   int c;
   int num = 0;

   fclose(fpReserve);
   if ((fp = fopen(filename, "r")) != NULL)
   {
      while (!feof(fp))
      {
         while ((buf[num] = fgetc(fp)) != EOF && buf[num] != '\n' && buf[num] != '\r' && num < (MSL - 2))
            num++;
         c = fgetc(fp);
         if ((c != '\n' && c != '\r') || c == buf[num])
            ungetc(c, fp);
         buf[num++] = '\n';
         buf[num++] = '\r';
         buf[num] = '\0';
         send_to_pager_color(buf, ch);
         num = 0;
      }
      /* Thanks to stu <sprice@ihug.co.nz> from the mailing list in pointing
       *  This out. */
      fclose(fp);
   }
   fpReserve = fopen(NULL_FILE, "r");
}

/*
 * Show the boot log file					-Thoric
 */
void do_dmesg(CHAR_DATA * ch, char *argument)
{
   set_pager_color(AT_LOG, ch);
   show_file(ch, BOOTLOG_FILE);
}

/*
 * Writes a string to the log, extended version			-Thoric
 */
void log_string_plus(const char *str, sh_int log_type, sh_int level)
{
   char *strtime;
   int offset;

   strtime = ctime(&current_time);
   strtime[strlen(strtime) - 1] = '\0';
   fprintf(stderr, "%s :: %s\n", strtime, str);
   if (strncmp(str, "Log ", 4) == 0)
      offset = 4;
   else
      offset = 0;
   switch (log_type)
   {
      default:
         to_channel(str + offset, CHANNEL_LOG, "Log", level);
         break;
      case LOG_BUILD:
         to_channel(str + offset, CHANNEL_BUILD, "Build", level);
         break;
      case LOG_COMM:
         to_channel(str + offset, CHANNEL_COMM, "Comm", level);
         break;
      case LOG_WARN:
         to_channel(str + offset, CHANNEL_WARN, "Warn", level);
         break;
      case LOG_ALL:
         break;
   }
   return;
}

/*
 * wizlist builder!						-Thoric
 */

void towizfile(const char *line)
{
   int filler, xx;
   char outline[MSL];
   FILE *wfp;

   outline[0] = '\0';

   if (line && line[0] != '\0')
   {
      filler = (78 - strlen(line));
      if (filler < 1)
         filler = 1;
      filler /= 2;
      for (xx = 0; xx < filler; xx++)
         strcat(outline, " ");
      strcat(outline, line);
   }
   strcat(outline, "\n\r");
   wfp = fopen(WIZLIST_FILE, "a");
   if (wfp)
   {
      fputs(outline, wfp);
      fclose(wfp);
   }
}

void add_to_wizlist(char *name, int level)
{
   WIZENT *wiz, *tmp;

#ifdef DEBUG
   log_string("Adding to wizlist...");
#endif

   CREATE(wiz, WIZENT, 1);
   wiz->name = str_dup(name);
   wiz->level = level;

   if (!first_wiz)
   {
      wiz->last = NULL;
      wiz->next = NULL;
      first_wiz = wiz;
      last_wiz = wiz;
      return;
   }

   /* insert sort, of sorts */
   for (tmp = first_wiz; tmp; tmp = tmp->next)
      if (level > tmp->level)
      {
         if (!tmp->last)
            first_wiz = wiz;
         else
            tmp->last->next = wiz;
         wiz->last = tmp->last;
         wiz->next = tmp;
         tmp->last = wiz;
         return;
      }

   wiz->last = last_wiz;
   wiz->next = NULL;
   last_wiz->next = wiz;
   last_wiz = wiz;
   return;
}

/*
 * Wizlist builder						-Thoric
 */
void make_wizlist()
{
   DIR *dp;
   struct dirent *dentry;
   FILE *gfp;
   char *word;
   int ilevel, iflags;
   WIZENT *wiz, *wiznext;
   char buf[MSL];

   first_wiz = NULL;
   last_wiz = NULL;

   dp = opendir(GOD_DIR);

   ilevel = 0;
   dentry = readdir(dp);
   while (dentry)
   {
      if (dentry->d_name[0] != '.')
      {
         sprintf(buf, "%s%s", GOD_DIR, dentry->d_name);
         gfp = fopen(buf, "r");
         if (gfp)
         {
            word = feof(gfp) ? "End" : fread_word(gfp);
            ilevel = fread_number(gfp);
            fread_to_eol(gfp);
            word = feof(gfp) ? "End" : fread_word(gfp);
            if (!str_cmp(word, "Pcflags"))
               iflags = fread_number(gfp);
            else
               iflags = 0;
            fclose(gfp);
            if (IS_SET(iflags, PCFLAG_RETIRED)) /* Tracker1 -- Xerves 4/10/99 */
               ilevel = LEVEL_IMMORTAL;
            if (IS_SET(iflags, PCFLAG_GUEST))
               ilevel = LEVEL_IMMORTAL;
            add_to_wizlist(dentry->d_name, ilevel);
         }
      }
      dentry = readdir(dp);
   }
   closedir(dp);

   unlink(WIZLIST_FILE);
   sprintf(buf, " The EVIL ONES of %s!", sysdata.mud_name);
   towizfile(buf);
/*  towizfile( " Masters of the Realms of Despair!" );*/
   buf[0] = '\0';
   ilevel = 65535;
   for (wiz = first_wiz; wiz; wiz = wiz->next)
   {
      if (wiz->level < ilevel)
      {
         if (buf[0])
         {
            towizfile(buf);
            buf[0] = '\0';
         }
         towizfile("");
         ilevel = wiz->level;
         switch (ilevel)
         {
            case MAX_LEVEL - 0:
               towizfile("&c&w       Admin (7)&Y");
               break; /* Tracker1 -- Xerves 4/10/99 */
            case MAX_LEVEL - 1:
               towizfile("&c&w       Hi-Staff (6)&Y");
               break;
            case MAX_LEVEL - 2:
               towizfile("&c&w       Staff (5)&Y");
               break;
            case MAX_LEVEL - 3:
               towizfile("&c&w       Hi-Imm (4)&Y");
               break;
            case MAX_LEVEL - 4:
               towizfile("&c&w       Imm (3)&Y");
               break;
            case MAX_LEVEL - 5:
               towizfile("&c&w       Guests/Retired (2)&Y");
               break;           
            default:
               towizfile("&c&w Servants    &Y");
               break;
         }
      }
      if (strlen(buf) + strlen(wiz->name) > 76)
      {
         towizfile(buf);
         buf[0] = '\0';
      }
      strcat(buf, " ");
      strcat(buf, wiz->name);
      if (strlen(buf) > 70)
      {
         towizfile(buf);
         buf[0] = '\0';
      }
   }

   if (buf[0])
      towizfile(buf);

   for (wiz = first_wiz; wiz; wiz = wiznext)
   {
      wiznext = wiz->next;
      DISPOSE(wiz->name);
      DISPOSE(wiz);
   }
   first_wiz = NULL;
   last_wiz = NULL;
}


void do_makewizlist(CHAR_DATA * ch, char *argument)
{
   make_wizlist();
}


/* mud prog functions */

/* This routine reads in scripts of MUDprograms from a file */

int mprog_name_to_type(char *name)
{
   if (!str_cmp(name, "in_file_prog"))
      return IN_FILE_PROG;
   if (!str_cmp(name, "act_prog"))
      return ACT_PROG;
   if (!str_cmp(name, "speech_prog"))
      return SPEECH_PROG;
   if (!str_cmp(name, "rand_prog"))
      return RAND_PROG;
   if (!str_cmp(name, "fight_prog"))
      return FIGHT_PROG;
   if (!str_cmp(name, "hitprcnt_prog"))
      return HITPRCNT_PROG;
   if (!str_cmp(name, "death_prog"))
      return DEATH_PROG;
   if (!str_cmp(name, "entry_prog"))
      return ENTRY_PROG;
   if (!str_cmp(name, "greet_prog"))
      return GREET_PROG;
   if (!str_cmp(name, "all_greet_prog"))
      return ALL_GREET_PROG;
   if (!str_cmp(name, "give_prog"))
      return GIVE_PROG;
   if (!str_cmp(name, "bribe_prog"))
      return BRIBE_PROG;
   if (!str_cmp(name, "time_prog"))
      return TIME_PROG;
   if (!str_cmp(name, "hour_prog"))
      return HOUR_PROG;
   if (!str_cmp(name, "wear_prog"))
      return WEAR_PROG;
   if (!str_cmp(name, "remove_prog"))
      return REMOVE_PROG;
   if (!str_cmp(name, "sac_prog"))
      return SAC_PROG;
   if (!str_cmp(name, "look_prog"))
      return LOOK_PROG;
   if (!str_cmp(name, "exa_prog"))
      return EXA_PROG;
   if (!str_cmp(name, "zap_prog"))
      return ZAP_PROG;
   if (!str_cmp(name, "get_prog"))
      return GET_PROG;
   if (!str_cmp(name, "drop_prog"))
      return DROP_PROG;
   if (!str_cmp(name, "damage_prog"))
      return DAMAGE_PROG;
   if (!str_cmp(name, "repair_prog"))
      return REPAIR_PROG;
   if (!str_cmp(name, "greet_prog"))
      return GREET_PROG;
   if (!str_cmp(name, "randiw_prog"))
      return RANDIW_PROG;
   if (!str_cmp(name, "speechiw_prog"))
      return SPEECHIW_PROG;
   if (!str_cmp(name, "pull_prog"))
      return PULL_PROG;
   if (!str_cmp(name, "push_prog"))
      return PUSH_PROG;
   if (!str_cmp(name, "sleep_prog"))
      return SLEEP_PROG;
   if (!str_cmp(name, "rest_prog"))
      return REST_PROG;
   if (!str_cmp(name, "rfight_prog"))
      return FIGHT_PROG;
   if (!str_cmp(name, "enter_prog"))
      return ENTRY_PROG;
   if (!str_cmp(name, "leave_prog"))
      return LEAVE_PROG;
   if (!str_cmp(name, "rdeath_prog"))
      return DEATH_PROG;
   if (!str_cmp(name, "script_prog"))
      return SCRIPT_PROG;
   if (!str_cmp(name, "use_prog"))
      return USE_PROG;
   return (ERROR_PROG);
}

MPROG_DATA *mprog_file_read(char *f, MPROG_DATA * mprg, MOB_INDEX_DATA * pMobIndex)
{

   char MUDProgfile[MIL];
   FILE *progfile;
   char letter;
   MPROG_DATA *mprg_next, *mprg2;
   bool done = FALSE;

   sprintf(MUDProgfile, "%s%s", PROG_DIR, f);

   progfile = fopen(MUDProgfile, "r");
   if (!progfile)
   {
      bug("Mob: %d couldn't open mudprog file", pMobIndex->vnum);
      exit(1);
   }

   mprg2 = mprg;
   switch (letter = fread_letter(progfile))
   {
      case '>':
         break;
      case '|':
         bug("empty mudprog file.");
         exit(1);
         break;
      default:
         bug("in mudprog file syntax error.");
         exit(1);
         break;
   }

   while (!done)
   {
      mprg2->type = mprog_name_to_type(fread_word(progfile));
      switch (mprg2->type)
      {
         case ERROR_PROG:
            bug("mudprog file type error");
            exit(1);
            break;
         case IN_FILE_PROG:
            bug("mprog file contains a call to file.");
            exit(1);
            break;
         default:
            xSET_BIT(pMobIndex->progtypes, mprg2->type);
            mprg2->arglist = fread_string(progfile);
            mprg2->comlist = fread_string(progfile);
            switch (letter = fread_letter(progfile))
            {
               case '>':
                  CREATE(mprg_next, MPROG_DATA, 1);
                  mprg_next->next = mprg2;
                  mprg2 = mprg_next;
                  break;
               case '|':
                  done = TRUE;
                  break;
               default:
                  bug("in mudprog file syntax error.");
                  exit(1);
                  break;
            }
            break;
      }
   }
   fclose(progfile);
   return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_mudprogs(AREA_DATA * tarea, FILE * fp)
{
   MOB_INDEX_DATA *iMob;
   MPROG_DATA *original;
   MPROG_DATA *working;
   char letter;
   int value;

   for (;;)
      switch (letter = fread_letter(fp))
      {
         default:
            bug("Load_mudprogs: bad command '%c'.", letter);
            exit(1);
            break;
         case 'S':
         case 's':
            fread_to_eol(fp);
            return;
         case '*':
            fread_to_eol(fp);
            break;
         case 'M':
         case 'm':
            value = fread_number(fp);
            if ((iMob = get_mob_index(value)) == NULL)
            {
               bug("Load_mudprogs: vnum %d doesnt exist", value);
               exit(1);
            }

            /* Go to the end of the prog command list if other commands
               exist */

            if ((original = iMob->mudprogs) != NULL)
               for (; original->next; original = original->next) ;

            CREATE(working, MPROG_DATA, 1);
            if (original)
               original->next = working;
            else
               iMob->mudprogs = working;
            working = mprog_file_read(fread_word(fp), working, iMob);
            working->next = NULL;
            fread_to_eol(fp);
            break;
      }

   return;

}

/* This procedure is responsible for reading any in_file MUDprograms.
 */

void mprog_read_programs(FILE * fp, MOB_INDEX_DATA * pMobIndex)
{
   MPROG_DATA *mprg;
   char letter;
   bool done = FALSE;

   if ((letter = fread_letter(fp)) != '>')
   {
      bug("Load_mobiles: vnum %d MUDPROG char", pMobIndex->vnum);
      exit(1);
   }
   CREATE(mprg, MPROG_DATA, 1);
   pMobIndex->mudprogs = mprg;

   while (!done)
   {
      mprg->type = mprog_name_to_type(fread_word(fp));
      switch (mprg->type)
      {
         case ERROR_PROG:
            bug("Load_mobiles: vnum %d MUDPROG type.", pMobIndex->vnum);
            exit(1);
            break;
         case IN_FILE_PROG:
            mprg = mprog_file_read(fread_string(fp), mprg, pMobIndex);
            fread_to_eol(fp);
            switch (letter = fread_letter(fp))
            {
               case '>':
                  CREATE(mprg->next, MPROG_DATA, 1);
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol(fp);
                  done = TRUE;
                  break;
               default:
                  bug("Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum);
                  exit(1);
                  break;
            }
            break;
         default:
            xSET_BIT(pMobIndex->progtypes, mprg->type);
            mprg->arglist = fread_string(fp);
            fread_to_eol(fp);
            mprg->comlist = fread_string(fp);
            fread_to_eol(fp);
            switch (letter = fread_letter(fp))
            {
               case '>':
                  CREATE(mprg->next, MPROG_DATA, 1);
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol(fp);
                  done = TRUE;
                  break;
               default:
                  bug("Load_mobiles: vnum %d bad MUDPROG.", pMobIndex->vnum);
                  exit(1);
                  break;
            }
            break;
      }
   }

   return;

}



/*************************************************************/
/* obj prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */


MPROG_DATA *oprog_file_read(char *f, MPROG_DATA * mprg, OBJ_INDEX_DATA * pObjIndex)
{

   char MUDProgfile[MIL];
   FILE *progfile;
   char letter;
   MPROG_DATA *mprg_next, *mprg2;
   bool done = FALSE;

   sprintf(MUDProgfile, "%s%s", PROG_DIR, f);

   progfile = fopen(MUDProgfile, "r");
   if (!progfile)
   {
      bug("Obj: %d couldnt open mudprog file", pObjIndex->vnum);
      exit(1);
   }

   mprg2 = mprg;
   switch (letter = fread_letter(progfile))
   {
      case '>':
         break;
      case '|':
         bug("empty objprog file.");
         exit(1);
         break;
      default:
         bug("in objprog file syntax error.");
         exit(1);
         break;
   }

   while (!done)
   {
      mprg2->type = mprog_name_to_type(fread_word(progfile));
      switch (mprg2->type)
      {
         case ERROR_PROG:
            bug("objprog file type error");
            exit(1);
            break;
         case IN_FILE_PROG:
            bug("objprog file contains a call to file.");
            exit(1);
            break;
         default:
            xSET_BIT(pObjIndex->progtypes, mprg2->type);
            mprg2->arglist = fread_string(progfile);
            mprg2->comlist = fread_string(progfile);
            switch (letter = fread_letter(progfile))
            {
               case '>':
                  CREATE(mprg_next, MPROG_DATA, 1);
                  mprg_next->next = mprg2;
                  mprg2 = mprg_next;
                  break;
               case '|':
                  done = TRUE;
                  break;
               default:
                  bug("in objprog file syntax error.");
                  exit(1);
                  break;
            }
            break;
      }
   }
   fclose(progfile);
   return mprg2;
}

/* Load a MUDprogram section from the area file.
 */
void load_objprogs(AREA_DATA * tarea, FILE * fp)
{
   OBJ_INDEX_DATA *iObj;
   MPROG_DATA *original;
   MPROG_DATA *working;
   char letter;
   int value;

   for (;;)
      switch (letter = fread_letter(fp))
      {
         default:
            bug("Load_objprogs: bad command '%c'.", letter);
            exit(1);
            break;
         case 'S':
         case 's':
            fread_to_eol(fp);
            return;
         case '*':
            fread_to_eol(fp);
            break;
         case 'M':
         case 'm':
            value = fread_number(fp);
            if ((iObj = get_obj_index(value)) == NULL)
            {
               bug("Load_objprogs: vnum %d doesnt exist", value);
               exit(1);
            }

            /* Go to the end of the prog command list if other commands
               exist */

            if ((original = iObj->mudprogs) != NULL)
               for (; original->next; original = original->next) ;

            CREATE(working, MPROG_DATA, 1);
            if (original)
               original->next = working;
            else
               iObj->mudprogs = working;
            working = oprog_file_read(fread_word(fp), working, iObj);
            working->next = NULL;
            fread_to_eol(fp);
            break;
      }

   return;

}

/* This procedure is responsible for reading any in_file OBJprograms.
 */

void oprog_read_programs(FILE * fp, OBJ_INDEX_DATA * pObjIndex)
{
   MPROG_DATA *mprg;
   char letter;
   bool done = FALSE;

   if ((letter = fread_letter(fp)) != '>')
   {
      bug("Load_objects: vnum %d OBJPROG char", pObjIndex->vnum);
      exit(1);
   }
   CREATE(mprg, MPROG_DATA, 1);
   pObjIndex->mudprogs = mprg;

   while (!done)
   {
      mprg->type = mprog_name_to_type(fread_word(fp));
      switch (mprg->type)
      {
         case ERROR_PROG:
            bug("Load_objects: vnum %d OBJPROG type.", pObjIndex->vnum);
            exit(1);
            break;
         case IN_FILE_PROG:
            mprg = oprog_file_read(fread_string(fp), mprg, pObjIndex);
            fread_to_eol(fp);
            switch (letter = fread_letter(fp))
            {
               case '>':
                  CREATE(mprg->next, MPROG_DATA, 1);
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol(fp);
                  done = TRUE;
                  break;
               default:
                  bug("Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum);
                  exit(1);
                  break;
            }
            break;
         default:
            xSET_BIT(pObjIndex->progtypes, mprg->type);
            mprg->arglist = fread_string(fp);
            fread_to_eol(fp);
            mprg->comlist = fread_string(fp);
            fread_to_eol(fp);
            switch (letter = fread_letter(fp))
            {
               case '>':
                  CREATE(mprg->next, MPROG_DATA, 1);
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol(fp);
                  done = TRUE;
                  break;
               default:
                  bug("Load_objects: vnum %d bad OBJPROG.", pObjIndex->vnum);
                  exit(1);
                  break;
            }
            break;
      }
   }

   return;

}


/*************************************************************/
/* room prog functions */
/* This routine transfers between alpha and numeric forms of the
 *  mob_prog bitvector types. This allows the use of the words in the
 *  mob/script files.
 */

/* This routine reads in scripts of OBJprograms from a file */
MPROG_DATA *rprog_file_read(char *f, MPROG_DATA * mprg, ROOM_INDEX_DATA * RoomIndex)
{

   char MUDProgfile[MIL];
   FILE *progfile;
   char letter;
   MPROG_DATA *mprg_next, *mprg2;
   bool done = FALSE;

   sprintf(MUDProgfile, "%s%s", PROG_DIR, f);

   progfile = fopen(MUDProgfile, "r");
   if (!progfile)
   {
      bug("Room: %d couldnt open roomprog file", RoomIndex->vnum);
      exit(1);
   }

   mprg2 = mprg;
   switch (letter = fread_letter(progfile))
   {
      case '>':
         break;
      case '|':
         bug("empty roomprog file.");
         exit(1);
         break;
      default:
         bug("in roomprog file syntax error.");
         exit(1);
         break;
   }

   while (!done)
   {
      mprg2->type = mprog_name_to_type(fread_word(progfile));
      switch (mprg2->type)
      {
         case ERROR_PROG:
            bug("roomprog file type error");
            exit(1);
            break;
         case IN_FILE_PROG:
            bug("roomprog file contains a call to file.");
            exit(1);
            break;
         default:
            xSET_BIT(RoomIndex->progtypes, mprg2->type);
            mprg2->arglist = fread_string(progfile);
            mprg2->comlist = fread_string(progfile);
            switch (letter = fread_letter(progfile))
            {
               case '>':
                  CREATE(mprg_next, MPROG_DATA, 1);
                  mprg_next->next = mprg2;
                  mprg2 = mprg_next;
                  break;
               case '|':
                  done = TRUE;
                  break;
               default:
                  bug("in roomprog file syntax error.");
                  exit(1);
                  break;
            }
            break;
      }
   }
   fclose(progfile);
   return mprg2;
}

/* Load a ROOMprogram section from the area file.
 */
void load_roomprogs(AREA_DATA * tarea, FILE * fp)
{
   ROOM_INDEX_DATA *iRoom;
   MPROG_DATA *original;
   MPROG_DATA *working;
   char letter;
   int value;

   for (;;)
      switch (letter = fread_letter(fp))
      {
         default:
            bug("Load_objprogs: bad command '%c'.", letter);
            exit(1);
            break;
         case 'S':
         case 's':
            fread_to_eol(fp);
            return;
         case '*':
            fread_to_eol(fp);
            break;
         case 'M':
         case 'm':
            value = fread_number(fp);
            if ((iRoom = get_room_index(value)) == NULL)
            {
               bug("Load_roomprogs: vnum %d doesnt exist", value);
               exit(1);
            }

            /* Go to the end of the prog command list if other commands
               exist */

            if ((original = iRoom->mudprogs) != NULL)
               for (; original->next; original = original->next) ;

            CREATE(working, MPROG_DATA, 1);
            if (original)
               original->next = working;
            else
               iRoom->mudprogs = working;
            working = rprog_file_read(fread_word(fp), working, iRoom);
            working->next = NULL;
            fread_to_eol(fp);
            break;
      }

   return;

}

/* This procedure is responsible for reading any in_file ROOMprograms.
 */

void rprog_read_programs(FILE * fp, ROOM_INDEX_DATA * pRoomIndex)
{
   MPROG_DATA *mprg;
   char letter;
   bool done = FALSE;

   if ((letter = fread_letter(fp)) != '>')
   {
      bug("Load_rooms: vnum %d ROOMPROG char", pRoomIndex->vnum);
      exit(1);
   }
   CREATE(mprg, MPROG_DATA, 1);
   pRoomIndex->mudprogs = mprg;

   while (!done)
   {
      mprg->type = mprog_name_to_type(fread_word(fp));
      switch (mprg->type)
      {
         case ERROR_PROG:
            bug("Load_rooms: vnum %d ROOMPROG type.", pRoomIndex->vnum);
            exit(1);
            break;
         case IN_FILE_PROG:
            mprg = rprog_file_read(fread_string(fp), mprg, pRoomIndex);
            fread_to_eol(fp);
            switch (letter = fread_letter(fp))
            {
               case '>':
                  CREATE(mprg->next, MPROG_DATA, 1);
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol(fp);
                  done = TRUE;
                  break;
               default:
                  bug("Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum);
                  exit(1);
                  break;
            }
            break;
         default:
            xSET_BIT(pRoomIndex->progtypes, mprg->type);
            mprg->arglist = fread_string(fp);
            fread_to_eol(fp);
            mprg->comlist = fread_string(fp);
            fread_to_eol(fp);
            switch (letter = fread_letter(fp))
            {
               case '>':
                  CREATE(mprg->next, MPROG_DATA, 1);
                  mprg = mprg->next;
                  break;
               case '|':
                  mprg->next = NULL;
                  fread_to_eol(fp);
                  done = TRUE;
                  break;
               default:
                  bug("Load_rooms: vnum %d bad ROOMPROG.", pRoomIndex->vnum);
                  exit(1);
                  break;
            }
            break;
      }
   }

   return;

}

//Used for removing loaded quest zones, note there are no resets in quest zones so if you need that
//support you code it right in
void delete_area(AREA_DATA *tarea)
{
   STRFREE(tarea->author);
   DISPOSE(tarea->name);
   DISPOSE(tarea->filename);
   UNLINK(tarea, first_area, last_area, next, prev);
   UNLINK(tarea, first_area_name, last_area_name, next_sort_name, prev_sort_name);
   UNLINK(tarea, first_asort, last_asort, next_sort, prev_sort);
   DISPOSE(tarea);
   top_area--;
}

/*************************************************************/
/* Function to delete a room index.  Called from do_rdelete in build.c
   Narn, May/96
   Don't ask me why they return bool.. :).. oh well.. -- Alty
*/
bool delete_room(ROOM_INDEX_DATA * room)
{
   int hash;
   ROOM_INDEX_DATA *prev, *limbo = get_room_index(ROOM_VNUM_LIMBO);
   OBJ_DATA *o;
   CHAR_DATA *ch;
   EXTRA_DESCR_DATA *ed;
   EXIT_DATA *ex;
   MPROG_ACT_LIST *mpact;
   MPROG_DATA *mp;

   while ((ch = room->first_person) != NULL)
   {
      if (!IS_NPC(ch))
      {
         char_from_room(ch);
         char_to_room(ch, limbo);
      }
      else
         extract_char(ch, TRUE);
   }
   while ((o = room->first_content) != NULL)
      extract_obj(o);
   while ((ed = room->first_extradesc) != NULL)
   {
      room->first_extradesc = ed->next;
      STRFREE(ed->keyword);
      STRFREE(ed->description);
      DISPOSE(ed);
      --top_ed;
   }
   while ((ex = room->first_exit) != NULL)
      extract_exit(room, ex);
   while ((mpact = room->mpact) != NULL)
   {
      room->mpact = mpact->next;
      DISPOSE(mpact->buf);
      DISPOSE(mpact);
   }
   while ((mp = room->mudprogs) != NULL)
   {
      room->mudprogs = mp->next;
      STRFREE(mp->arglist);
      STRFREE(mp->comlist);
      DISPOSE(mp);
   }
   if (room->map)
   {
      MAP_INDEX_DATA *mapi;

      if ((mapi = get_map_index(room->map->vnum)) != NULL)
         if (room->map->x > 0 && room->map->x < 80 && room->map->y > 0 && room->map->y < 48)
            mapi->map_of_vnums[room->map->y][room->map->x] = -1;
      DISPOSE(room->map);
   }
   STRFREE(room->name);
   STRFREE(room->description);

   hash = room->vnum % MAX_KEY_HASH;
   if (room == room_index_hash[hash])
      room_index_hash[hash] = room->next;
   else
   {
      for (prev = room_index_hash[hash]; prev; prev = prev->next)
         if (prev->next == room)
            break;
      if (prev)
         prev->next = room->next;
      else
         bug("delete_room: room %d not in hash bucket %d.", room->vnum, hash);
   }
   DISPOSE(room);
   --top_room;
   return TRUE;
}

/* See comment on delete_room. */
bool delete_obj(OBJ_INDEX_DATA * obj)
{
   int hash;
   OBJ_INDEX_DATA *prev;
   OBJ_DATA *o, *o_next;
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *af;
   MPROG_DATA *mp;
   int auc;

   /* Remove references to object index */
   for (o = first_object; o; o = o_next)
   {
      o_next = o->next;
      if (o->pIndexData == obj)
         extract_obj(o);
   }
   while ((ed = obj->first_extradesc) != NULL)
   {
      obj->first_extradesc = ed->next;
      STRFREE(ed->keyword);
      STRFREE(ed->description);
      DISPOSE(ed);
      --top_ed;
   }
   while ((af = obj->first_affect) != NULL)
   {
      obj->first_affect = af->next;
      DISPOSE(af);
      --top_affect;
   }
   while ((mp = obj->mudprogs) != NULL)
   {
      obj->mudprogs = mp->next;
      STRFREE(mp->arglist);
      STRFREE(mp->comlist);
      DISPOSE(mp);
   }
   STRFREE(obj->name);
   STRFREE(obj->short_descr);
   STRFREE(obj->description);
   STRFREE(obj->action_desc);

   for (auc = 0; auc < AUCTION_MEM; ++auc)
      if (auction->history[auc] == obj)
      {
         if (auc < AUCTION_MEM - 1)
            memmove(&auction->history[auc], &auction->history[auc + 1], (AUCTION_MEM - auc - 1) * sizeof(OBJ_INDEX_DATA *));
         auction->history[AUCTION_MEM - 1] = NULL;
         --auc;
      }

   hash = obj->vnum % MAX_KEY_HASH;
   if (obj == obj_index_hash[hash])
      obj_index_hash[hash] = obj->next;
   else
   {
      for (prev = obj_index_hash[hash]; prev; prev = prev->next)
         if (prev->next == obj)
            break;
      if (prev)
         prev->next = obj->next;
      else
         bug("delete_obj: object %d not in hash bucket %d.", obj->vnum, hash);
   }
   DISPOSE(obj);
   --top_obj_index;
   return TRUE;
}

/* See comment on delete_room. */
bool delete_mob(MOB_INDEX_DATA * mob)
{
   int hash;
   MOB_INDEX_DATA *prev;
   CHAR_DATA *ch, *ch_next;
   MPROG_DATA *mp;

   for (ch = first_char; ch; ch = ch_next)
   {
      ch_next = ch->next;
      if (ch->pIndexData == mob)
         extract_char(ch, TRUE);
   }
   while ((mp = mob->mudprogs) != NULL)
   {
      mob->mudprogs = mp->next;
      STRFREE(mp->arglist);
      STRFREE(mp->comlist);
      DISPOSE(mp);
   }

   if (mob->pShop)
   {
      UNLINK(mob->pShop, first_shop, last_shop, next, prev);
      DISPOSE(mob->pShop);
      --top_shop;
   }

   if (mob->rShop)
   {
      UNLINK(mob->rShop, first_repair, last_repair, next, prev);
      DISPOSE(mob->rShop);
      --top_repair;
   }

   STRFREE(mob->player_name);
   STRFREE(mob->short_descr);
   STRFREE(mob->long_descr);
   STRFREE(mob->description);

   hash = mob->vnum % MAX_KEY_HASH;
   if (mob == mob_index_hash[hash])
      mob_index_hash[hash] = mob->next;
   else
   {
      for (prev = mob_index_hash[hash]; prev; prev = prev->next)
         if (prev->next == mob)
            break;
      if (prev)
         prev->next = mob->next;
      else
         bug("delete_mob: mobile %d not in hash bucket %d.", mob->vnum, hash);
   }
   DISPOSE(mob);
   --top_mob_index;
   return TRUE;
}

/*
 * Creat a new room (for online building)			-Thoric
 */
ROOM_INDEX_DATA *make_room(int vnum)
{
   ROOM_INDEX_DATA *pRoomIndex;
   int iHash;

   CREATE(pRoomIndex, ROOM_INDEX_DATA, 1);
   pRoomIndex->first_person = NULL;
   pRoomIndex->last_person = NULL;
   pRoomIndex->first_content = NULL;
   pRoomIndex->last_content = NULL;
   pRoomIndex->first_extradesc = NULL;
   pRoomIndex->last_extradesc = NULL;
   pRoomIndex->area = NULL;
   pRoomIndex->vnum = vnum;
   pRoomIndex->name = STRALLOC("Floating in a void");
   pRoomIndex->description = STRALLOC("");
   xCLEAR_BITS(pRoomIndex->room_flags);
   xSET_BIT(pRoomIndex->room_flags, ROOM_PROTOTYPE);
/*	pRoomIndex->room_flags		= ROOM_PROTOTYPE;   */
   pRoomIndex->sector_type = 1;
   pRoomIndex->light = 0;
   pRoomIndex->resource = 0;
   pRoomIndex->first_exit = NULL;
   pRoomIndex->last_exit = NULL;

   iHash = vnum % MAX_KEY_HASH;
   pRoomIndex->next = room_index_hash[iHash];
   room_index_hash[iHash] = pRoomIndex;
   top_room++;

   return pRoomIndex;
}

/*
 * Create a new INDEX object (for online building)		-Thoric
 * Option to clone an existing index object.
 */
OBJ_INDEX_DATA *make_object(int vnum, int cvnum, char *name, int cprog)
{
   OBJ_INDEX_DATA *pObjIndex, *cObjIndex;
   char buf[MSL];
   int iHash;

   if (cvnum > 0)
      cObjIndex = get_obj_index(cvnum);
   else
      cObjIndex = NULL;
   CREATE(pObjIndex, OBJ_INDEX_DATA, 1);
   pObjIndex->vnum = vnum;
   pObjIndex->name = STRALLOC(name);
   pObjIndex->first_affect = NULL;
   pObjIndex->last_affect = NULL;
   pObjIndex->first_extradesc = NULL;
   pObjIndex->last_extradesc = NULL;
   if (!cObjIndex)
   {
      sprintf(buf, "A newly created %s", name);
      pObjIndex->short_descr = STRALLOC(buf);
      sprintf(buf, "Some god dropped a newly created %s here.", name);
      pObjIndex->description = STRALLOC(buf);
      pObjIndex->action_desc = STRALLOC("");
      pObjIndex->short_descr[0] = LOWER(pObjIndex->short_descr[0]);
      pObjIndex->description[0] = UPPER(pObjIndex->description[0]);
      pObjIndex->item_type = ITEM_TRASH;
      xCLEAR_BITS(pObjIndex->extra_flags);
      xSET_BIT(pObjIndex->extra_flags, ITEM_PROTOTYPE);
      pObjIndex->wear_flags = 0;
      SET_BIT(pObjIndex->wear_flags, ITEM_TAKE);
      pObjIndex->value[0] = 0;
      pObjIndex->value[1] = 0;
      pObjIndex->value[2] = 0;
      pObjIndex->value[3] = 0;
      pObjIndex->value[4] = 0;
      pObjIndex->value[5] = 0;
      pObjIndex->value[6] = 0;
      pObjIndex->value[7] = 0;
      pObjIndex->value[8] = 0;
      pObjIndex->value[9] = 0;
      pObjIndex->value[10] = 0;
      pObjIndex->value[11] = 0;
      pObjIndex->value[12] = 0;
      pObjIndex->value[13] = 0;
      pObjIndex->weight = 1;
      pObjIndex->cost = 0;
      pObjIndex->cvnum = 0;
      pObjIndex->cident = 0;
      pObjIndex->sworthrestrict = 0;
      pObjIndex->imbueslots = 0;
   }
   else
   {
      EXTRA_DESCR_DATA *ed, *ced;
      AFFECT_DATA *paf, *cpaf;

      pObjIndex->short_descr = QUICKLINK(cObjIndex->short_descr);
      pObjIndex->description = QUICKLINK(cObjIndex->description);
      pObjIndex->action_desc = QUICKLINK(cObjIndex->action_desc);
      pObjIndex->item_type = cObjIndex->item_type;
      pObjIndex->extra_flags = cObjIndex->extra_flags;
      xSET_BIT(pObjIndex->extra_flags, ITEM_PROTOTYPE);
      pObjIndex->wear_flags = cObjIndex->wear_flags;
      SET_BIT(pObjIndex->wear_flags, ITEM_TAKE);
      pObjIndex->value[0] = cObjIndex->value[0];
      pObjIndex->value[1] = cObjIndex->value[1];
      pObjIndex->value[2] = cObjIndex->value[2];
      pObjIndex->value[3] = cObjIndex->value[3];
      pObjIndex->value[4] = cObjIndex->value[4];
      pObjIndex->value[5] = cObjIndex->value[5];
      pObjIndex->value[6] = cObjIndex->value[6];
      pObjIndex->value[7] = cObjIndex->value[7];
      pObjIndex->value[8] = cObjIndex->value[8];
      pObjIndex->value[9] = cObjIndex->value[9];
      pObjIndex->value[10] = cObjIndex->value[10];
      pObjIndex->value[11] = cObjIndex->value[11];
      pObjIndex->value[12] = cObjIndex->value[12];
      pObjIndex->value[13] = cObjIndex->value[13];
      pObjIndex->weight = cObjIndex->weight;
      pObjIndex->cost = cObjIndex->cost;
      pObjIndex->cvnum = cObjIndex->vnum;
      pObjIndex->cident = cObjIndex->cident;
      pObjIndex->sworthrestrict = cObjIndex->sworthrestrict;
      pObjIndex->imbueslots = cObjIndex->imbueslots;
      for (ced = cObjIndex->first_extradesc; ced; ced = ced->next)
      {
         CREATE(ed, EXTRA_DESCR_DATA, 1);
         ed->keyword = QUICKLINK(ced->keyword);
         ed->description = QUICKLINK(ced->description);
         LINK(ed, pObjIndex->first_extradesc, pObjIndex->last_extradesc, next, prev);
         top_ed++;
      }
      for (cpaf = cObjIndex->first_affect; cpaf; cpaf = cpaf->next)
      {
         CREATE(paf, AFFECT_DATA, 1);
         paf->type = cpaf->type;
         paf->duration = cpaf->duration;
         paf->location = cpaf->location;
         paf->modifier = cpaf->modifier;
         paf->bitvector = cpaf->bitvector;
         LINK(paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev);
         top_affect++;
      }
   }
   pObjIndex->count = 0;
   iHash = vnum % MAX_KEY_HASH;
   pObjIndex->next = obj_index_hash[iHash];
   obj_index_hash[iHash] = pObjIndex;
   top_obj_index++;

   return pObjIndex;
}

/*
 * Create a new INDEX mobile (for online building)		-Thoric
 * Option to clone an existing index mobile.
 */
MOB_INDEX_DATA *make_mobile(sh_int vnum, sh_int cvnum, char *name)
{
   MOB_INDEX_DATA *pMobIndex, *cMobIndex;
   char buf[MSL];
   int iHash;

   if (cvnum > 0)
      cMobIndex = get_mob_index(cvnum);
   else
      cMobIndex = NULL;
   CREATE(pMobIndex, MOB_INDEX_DATA, 1);
   pMobIndex->vnum = vnum;
   pMobIndex->count = 0;
   pMobIndex->killed = 0;
   pMobIndex->player_name = STRALLOC(name);
   if (!cMobIndex)
   {
      sprintf(buf, "A newly created %s", name);
      pMobIndex->short_descr = STRALLOC(buf);
      sprintf(buf, "Some god abandoned a newly created %s here.\n\r", name);
      pMobIndex->long_descr = STRALLOC(buf);
      pMobIndex->description = STRALLOC("");
      pMobIndex->short_descr[0] = LOWER(pMobIndex->short_descr[0]);
      pMobIndex->long_descr[0] = UPPER(pMobIndex->long_descr[0]);
      pMobIndex->description[0] = UPPER(pMobIndex->description[0]);
      xCLEAR_BITS(pMobIndex->act);
      xCLEAR_BITS(pMobIndex->miflags);
      pMobIndex->elementb = 0;
      xSET_BIT(pMobIndex->act, ACT_IS_NPC);
      xSET_BIT(pMobIndex->act, ACT_PROTOTYPE);
      xCLEAR_BITS(pMobIndex->affected_by);
      pMobIndex->pShop = NULL;
      pMobIndex->rShop = NULL;
      pMobIndex->spec_fun = NULL;
      pMobIndex->mudprogs = NULL;
      xCLEAR_BITS(pMobIndex->progtypes);
      pMobIndex->alignment = 0;
      pMobIndex->level = 0;
      pMobIndex->hitnodice = 0;
      pMobIndex->hitsizedice = 0;
      pMobIndex->hitplus = 0;
      pMobIndex->damnodice = 0;
      pMobIndex->damsizedice = 0;
      pMobIndex->damplus = 0;
      pMobIndex->damaddhi = 0;
      pMobIndex->damaddlow = 0;
      pMobIndex->max_move = 1000;
      pMobIndex->gold = 0;
      pMobIndex->position = 12;
      pMobIndex->tohitslash = 0;
      pMobIndex->tohitstab = 0;
      pMobIndex->tohitbash = 0;
      pMobIndex->defposition = 12;
      pMobIndex->apply_res_fire = 100;
      pMobIndex->apply_res_water = 100;
      pMobIndex->apply_res_air = 100;
      pMobIndex->apply_res_earth = 100;
      pMobIndex->apply_res_energy = 100;
      pMobIndex->apply_res_magic = 100;
      pMobIndex->apply_res_nonmagic = 100;
      pMobIndex->apply_res_blunt = 100;
      pMobIndex->apply_res_pierce = 100;
      pMobIndex->apply_res_slash = 100;
      pMobIndex->apply_res_poison = 100;
      pMobIndex->apply_res_paralysis = 100;
      pMobIndex->apply_res_holy = 100;
      pMobIndex->apply_res_unholy = 100;
      pMobIndex->apply_res_undead = 100;
      
      pMobIndex->sex = 0;
      pMobIndex->m1 = 0;
      pMobIndex->m2 = 0;
      pMobIndex->m3 = 0;
      pMobIndex->m4 = 0;
      pMobIndex->m5 = 0;
      pMobIndex->m6 = 0;
      pMobIndex->m7 = 0;
      pMobIndex->m8 = 0;
      pMobIndex->m9 = 0;
      pMobIndex->m10 = 0;
      pMobIndex->m11 = 0;
      pMobIndex->m12 = 0;
      pMobIndex->cident = 0;
      pMobIndex->perm_str = 13;
      pMobIndex->perm_dex = 13;
      pMobIndex->perm_int = 13;
      pMobIndex->perm_wis = 13;
      pMobIndex->perm_cha = 13;
      pMobIndex->perm_con = 13;
      pMobIndex->perm_lck = 13;
      pMobIndex->perm_agi = 15;
      pMobIndex->race = 0;
      pMobIndex->xflags = 0;
      pMobIndex->resistant = 0;
      pMobIndex->immune = 0;
      pMobIndex->susceptible = 0;
      xCLEAR_BITS(pMobIndex->attacks);
      xCLEAR_BITS(pMobIndex->defenses);
   }
   else
   {
      pMobIndex->short_descr = QUICKLINK(cMobIndex->short_descr);
      pMobIndex->long_descr = QUICKLINK(cMobIndex->long_descr);
      pMobIndex->description = QUICKLINK(cMobIndex->description);
      pMobIndex->act = cMobIndex->act;
      pMobIndex->miflags = cMobIndex->miflags;
      xSET_BIT(pMobIndex->act, ACT_PROTOTYPE);
      pMobIndex->affected_by = cMobIndex->affected_by;
      pMobIndex->pShop = NULL;
      pMobIndex->rShop = NULL;
      pMobIndex->spec_fun = cMobIndex->spec_fun;
      pMobIndex->mudprogs = NULL;
      xCLEAR_BITS(pMobIndex->progtypes);
      pMobIndex->alignment = cMobIndex->alignment;
      pMobIndex->level = 0;
      pMobIndex->tohitbash = cMobIndex->tohitbash;
      pMobIndex->tohitslash = cMobIndex->tohitslash;
      pMobIndex->tohitstab = cMobIndex->tohitstab;
      pMobIndex->mobthac0 = cMobIndex->mobthac0;
      pMobIndex->ac = cMobIndex->ac;
      pMobIndex->hitnodice = cMobIndex->hitnodice;
      pMobIndex->hitsizedice = cMobIndex->hitsizedice;
      pMobIndex->hitplus = cMobIndex->hitplus;
      pMobIndex->damnodice = cMobIndex->damnodice;
      pMobIndex->damsizedice = cMobIndex->damsizedice;
      pMobIndex->damplus = cMobIndex->damplus;
      pMobIndex->damaddhi = cMobIndex->damaddhi;
      pMobIndex->damaddlow = cMobIndex->damaddlow;
      pMobIndex->max_move = cMobIndex->max_move;
      pMobIndex->gold = cMobIndex->gold;
      pMobIndex->position = cMobIndex->position;
      pMobIndex->defposition = cMobIndex->defposition;
      pMobIndex->apply_res_fire = cMobIndex->apply_res_fire;
      pMobIndex->apply_res_water = cMobIndex->apply_res_water;
      pMobIndex->apply_res_air = cMobIndex->apply_res_air;
      pMobIndex->apply_res_earth = cMobIndex->apply_res_earth;
      pMobIndex->apply_res_energy = cMobIndex->apply_res_energy;
      pMobIndex->apply_res_magic = cMobIndex->apply_res_magic;
      pMobIndex->apply_res_nonmagic = cMobIndex->apply_res_nonmagic;
      pMobIndex->apply_res_blunt = cMobIndex->apply_res_blunt;
      pMobIndex->apply_res_pierce = cMobIndex->apply_res_pierce;
      pMobIndex->apply_res_slash = cMobIndex->apply_res_slash;
      pMobIndex->apply_res_poison = cMobIndex->apply_res_poison;
      pMobIndex->apply_res_paralysis = cMobIndex->apply_res_paralysis;
      pMobIndex->apply_res_holy = cMobIndex->apply_res_holy;
      pMobIndex->apply_res_unholy = cMobIndex->apply_res_unholy;
      pMobIndex->apply_res_undead = cMobIndex->apply_res_undead;
      pMobIndex->sex = cMobIndex->sex;
      pMobIndex->m1 = cMobIndex->m1;
      pMobIndex->m2 = cMobIndex->m2;
      pMobIndex->m3 = cMobIndex->m3;
      pMobIndex->m4 = cMobIndex->m4;
      pMobIndex->m5 = cMobIndex->m5;
      pMobIndex->m6 = cMobIndex->m6;
      pMobIndex->m7 = cMobIndex->m7;
      pMobIndex->m8 = cMobIndex->m8;
      pMobIndex->m9 = cMobIndex->m9;
      pMobIndex->m10 = cMobIndex->m10;
      pMobIndex->m11 = cMobIndex->m11;
      pMobIndex->m12 = cMobIndex->m12;
      pMobIndex->perm_str = cMobIndex->perm_str;
      pMobIndex->perm_dex = cMobIndex->perm_dex;
      pMobIndex->perm_int = cMobIndex->perm_int;
      pMobIndex->perm_wis = cMobIndex->perm_wis;
      pMobIndex->perm_cha = cMobIndex->perm_cha;
      pMobIndex->perm_con = cMobIndex->perm_con;
      pMobIndex->perm_lck = cMobIndex->perm_lck;
      pMobIndex->perm_agi = cMobIndex->perm_agi;
      pMobIndex->race = cMobIndex->race;
      pMobIndex->xflags = cMobIndex->xflags;
      pMobIndex->resistant = cMobIndex->resistant;
      pMobIndex->immune = cMobIndex->immune;
      pMobIndex->susceptible = cMobIndex->susceptible;
      pMobIndex->attacks = cMobIndex->attacks;
      pMobIndex->defenses = cMobIndex->defenses;
   }
   iHash = vnum % MAX_KEY_HASH;
   pMobIndex->next = mob_index_hash[iHash];
   mob_index_hash[iHash] = pMobIndex;
   top_mob_index++;

   return pMobIndex;
}

/*
 * Creates a simple exit with no fields filled but rvnum and optionally
 * to_room and vnum.						-Thoric
 * Exits are inserted into the linked list based on vdir.
 */
EXIT_DATA *make_exit(ROOM_INDEX_DATA * pRoomIndex, ROOM_INDEX_DATA * to_room, sh_int door)
{
   EXIT_DATA *pexit, *texit;
   bool broke;

   CREATE(pexit, EXIT_DATA, 1);
   pexit->vdir = door;
   pexit->rvnum = pRoomIndex->vnum;
   pexit->to_room = to_room;
   pexit->distance = 1;
   CREATE(pexit->coord, COORD_DATA, 1);
   pexit->coord->x = 0;
   pexit->coord->y = 0;
   if (to_room)
   {
      pexit->vnum = to_room->vnum;
      texit = get_exit_to(to_room, rev_dir[door], pRoomIndex->vnum);
      if (texit) /* assign reverse exit pointers */
      {
         texit->rexit = pexit;
         pexit->rexit = texit;
      }
   }
   broke = FALSE;
   for (texit = pRoomIndex->first_exit; texit; texit = texit->next)
      if (door < texit->vdir)
      {
         broke = TRUE;
         break;
      }
   if (!pRoomIndex->first_exit)
      pRoomIndex->first_exit = pexit;
   else
   {
      /* keep exits in incremental order - insert exit into list */
      if (broke && texit)
      {
         if (!texit->prev)
            pRoomIndex->first_exit = pexit;
         else
            texit->prev->next = pexit;
         pexit->prev = texit->prev;
         pexit->next = texit;
         texit->prev = pexit;
         top_exit++;
         return pexit;
      }
      pRoomIndex->last_exit->next = pexit;
   }
   pexit->next = NULL;
   pexit->prev = pRoomIndex->last_exit;
   pRoomIndex->last_exit = pexit;
   top_exit++;
   return pexit;
}

void fix_area_exits(AREA_DATA * tarea)
{
   ROOM_INDEX_DATA *pRoomIndex;
   EXIT_DATA *pexit, *rev_exit;
   int rnum;
   bool fexit;

   for (rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++)
   {
      if ((pRoomIndex = get_room_index(rnum)) == NULL)
         continue;

      fexit = FALSE;
      for (pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next)
      {
         fexit = TRUE;
         pexit->rvnum = pRoomIndex->vnum;
         if (pexit->vnum <= 0)
            pexit->to_room = NULL;
         else
            pexit->to_room = get_room_index(pexit->vnum);
      }
      if (!fexit)
         xSET_BIT(pRoomIndex->room_flags, ROOM_NO_MOB);
   }


   for (rnum = tarea->low_r_vnum; rnum <= tarea->hi_r_vnum; rnum++)
   {
      if ((pRoomIndex = get_room_index(rnum)) == NULL)
         continue;

      for (pexit = pRoomIndex->first_exit; pexit; pexit = pexit->next)
      {
         if (pexit->to_room && !pexit->rexit)
         {
            rev_exit = get_exit_to(pexit->to_room, rev_dir[pexit->vdir], pRoomIndex->vnum);
            if (rev_exit)
            {
               pexit->rexit = rev_exit;
               rev_exit->rexit = pexit;
            }
         }
      }
   }
}

void load_area_file(AREA_DATA * tarea, char *filename)
{
/*    FILE *fpin;
    what intelligent person stopped using fpArea?????
    if fpArea isn't being used, then no filename or linenumber
    is printed when an error occurs during loading the area..
    (bug uses fpArea)
      --TRI  */

   if (fBootDb)
      tarea = last_area;
   if (!fBootDb && !tarea)
   {
      bug("Load_area: null area!");
      return;
   }

   if ((fpArea = fopen(filename, "r")) == NULL)
   {
      perror(filename);
      bug("load_area: error loading file (can't open)");
      bug(filename);
      return;
   }
   area_version = 0;
   for (;;)
   {
      char *word;

      if (fread_letter(fpArea) != '#')
      {
         bug(tarea->filename);
         bug("load_area: # not found.");
         exit(1);
      }

      word = fread_word(fpArea);

      if (word[0] == '$')
         break;
      else if (!str_cmp(word, "AREA"))
      {
         if (fBootDb)
         {
            load_area(fpArea);
            tarea = last_area;
         }
         else
         {
            DISPOSE(tarea->name);
            tarea->name = fread_string_nohash(fpArea);
         }
      }
      else if (!str_cmp(word, "AUTHOR"))
         load_author(tarea, fpArea);
      else if (!str_cmp(word, "FLAGS"))
         load_flags(tarea, fpArea);
      else if (!str_cmp(word, "RANGES"))
         load_ranges(tarea, fpArea);
      else if (!str_cmp(word, "ECONOMY"))
         load_economy(tarea, fpArea);
      else if (!str_cmp(word, "KINGDOM"))  //no longer used, left for old area files
         load_area_kingdom(tarea, fpArea);
      else if (!str_cmp(word, "KOWNER"))
         load_kowner(tarea, fpArea);
      else if (!str_cmp(word, "RESETMSG"))
         load_resetmsg(tarea, fpArea);
      /* Rennard */
      else if (!str_cmp(word, "HELPS"))
         load_helps(tarea, fpArea);
      else if (!str_cmp(word, "MOBILES"))
         load_mobiles(tarea, fpArea);
      else if (!str_cmp(word, "MUDPROGS"))
         load_mudprogs(tarea, fpArea);
      else if (!str_cmp(word, "OBJECTS"))
         load_objects(tarea, fpArea);
      else if (!str_cmp(word, "OBJPROGS"))
         load_objprogs(tarea, fpArea);
      else if (!str_cmp(word, "RESETS"))
         load_resets(tarea, fpArea);
      else if (!str_cmp(word, "ROOMS"))
         load_rooms(tarea, fpArea);
      else if (!str_cmp(word, "SHOPS"))
         load_shops(tarea, fpArea);
      else if (!str_cmp(word, "REPAIRS"))
         load_repairs(tarea, fpArea);
      else if (!str_cmp(word, "SPECIALS"))
         load_specials(tarea, fpArea);
      else if (!str_cmp(word, "CLIMATE"))
         load_climate(tarea, fpArea);
      else if (!str_cmp(word, "VERSION"))
         load_version(tarea, fpArea);
      else
      {
         bug(tarea->filename);
         bug("load_area: bad section name.");
         if (fBootDb)
            exit(1);
         else
         {
            fclose(fpArea);
            fpArea=NULL;
            return;
         }
      }
   }
   fclose(fpArea);
   fpArea=NULL;
   if (tarea)
   {
      if (fBootDb)
      {
         sort_area_by_name(tarea); /* 4/27/97 */
         sort_area(tarea, FALSE);
      }
      fprintf(stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d Mobs: %5d - %d\n",
         tarea->filename, tarea->low_r_vnum, tarea->hi_r_vnum, tarea->low_o_vnum, tarea->hi_o_vnum, tarea->low_m_vnum, tarea->hi_m_vnum);
      if (!tarea->author)
         tarea->author = STRALLOC("");
      SET_BIT(tarea->status, AREA_LOADED);
   }
   else
      fprintf(stderr, "(%s)\n", filename);
}

void load_reserved(void)
{
   RESERVE_DATA *res;
   FILE *fp;

   if (!(fp = fopen(SYSTEM_DIR RESERVED_LIST, "r")))
      return;

   for (;;)
   {
      if (feof(fp))
      {
         bug("Load_reserved: no $ found.");
         fclose(fp);
         return;
      }
      CREATE(res, RESERVE_DATA, 1);
      res->name = fread_string_nohash(fp);
      if (*res->name == '$')
         break;
      sort_reserved(res);
   }
   DISPOSE(res->name);
   DISPOSE(res);
   fclose(fp);
   return;
}

/* Build list of in_progress areas.  Do not load areas.
 * define AREA_READ if you want it to build area names rather than reading
 * them out of the area files. -- Altrag */
void load_buildlist(void)
{
   DIR *dp;
   struct dirent *dentry;
   FILE *fp;
   char buf[MSL];
   AREA_DATA *pArea;
   char line[81];
   char word[81];
   int low, hi;
   int mlow, mhi, olow, ohi, rlow, rhi;
   bool badfile = FALSE;
   char temp;

   dp = opendir(GOD_DIR);
   dentry = readdir(dp);
   while (dentry)
   {
      if (dentry->d_name[0] != '.')
      {
         sprintf(buf, "%s%s", GOD_DIR, dentry->d_name);
         if (!(fp = fopen(buf, "r")))
         {
            bug("Load_buildlist: invalid file");
            perror(buf);
            dentry = readdir(dp);
            continue;
         }
         log_string(buf);
         badfile = FALSE;
         rlow = rhi = olow = ohi = mlow = mhi = 0;
         while (!feof(fp) && !ferror(fp))
         {
            low = 0;
            hi = 0;
            word[0] = 0;
            line[0] = 0;
            if ((temp = fgetc(fp)) != EOF)
               ungetc(temp, fp);
            else
               break;

            fgets(line, 80, fp);
            sscanf(line, "%s %d %d", word, &low, &hi);
            if (!strcmp(word, "Level"))
            {
               if (low < LEVEL_IMMORTAL)
               {
                  sprintf(buf, "%s: God file with level %d < %d", dentry->d_name, low, LEVEL_IMMORTAL);
                  badfile = TRUE;
               }
            }
            if (!strcmp(word, "RoomRange"))
               rlow = low, rhi = hi;
            else if (!strcmp(word, "MobRange"))
               mlow = low, mhi = hi;
            else if (!strcmp(word, "ObjRange"))
               olow = low, ohi = hi;
         }
         fclose(fp);
         if (rlow && rhi && !badfile)
         {
            sprintf(buf, "%s%s.are", BUILD_DIR, dentry->d_name);
            if (!(fp = fopen(buf, "r")))
            {
               bug("Load_buildlist: cannot open area file for read");
               perror(buf);
               dentry = readdir(dp);
               continue;
            }
#if !defined(READ_AREA) /* Dont always want to read stuff.. dunno.. shrug */
            strcpy(word, fread_word(fp));
            if (word[0] != '#' || strcmp(&word[1], "AREA"))
            {
               sprintf(buf, "Make_buildlist: %s.are: no #AREA found.", dentry->d_name);
               fclose(fp);
               dentry = readdir(dp);
               continue;
            }
#endif
            CREATE(pArea, AREA_DATA, 1);
            sprintf(buf, "%s.are", dentry->d_name);
            pArea->author = STRALLOC(dentry->d_name);
            pArea->filename = str_dup(buf);
#if !defined(READ_AREA)
            pArea->name = fread_string_nohash(fp);
#else
            sprintf(buf, "{PROTO} %s's area in progress", dentry->d_name);
            pArea->name = str_dup(buf);
#endif
            fclose(fp);
            pArea->low_r_vnum = rlow;
            pArea->hi_r_vnum = rhi;
            pArea->low_m_vnum = mlow;
            pArea->hi_m_vnum = mhi;
            pArea->low_o_vnum = olow;
            pArea->hi_o_vnum = ohi;
            pArea->low_soft_range = -1;
            pArea->hi_soft_range = -1;
            pArea->low_hard_range = -1;
            pArea->hi_hard_range = -1;

            pArea->first_reset = NULL;
            pArea->last_reset = NULL;
            LINK(pArea, first_build, last_build, next, prev);
            fprintf(stderr, "%-14s: Rooms: %5d - %-5d Objs: %5d - %-5d "
               "Mobs: %5d - %-5d\n",
               pArea->filename, pArea->low_r_vnum, pArea->hi_r_vnum, pArea->low_o_vnum, pArea->hi_o_vnum, pArea->low_m_vnum, pArea->hi_m_vnum);
            sort_area(pArea, TRUE);
         }
      }
      dentry = readdir(dp);
   }
   closedir(dp);
}

/* Rebuilt from broken copy, but bugged - commented out for now - Blod */
void sort_reserved(RESERVE_DATA * pRes)
{
   RESERVE_DATA *res = NULL;

   if (!pRes)
   {
      bug("Sort_reserved: NULL pRes");
      return;
   }

   pRes->next = NULL;
   pRes->prev = NULL;

   for (res = first_reserved; res; res = res->next)
   {
      if (strcasecmp(pRes->name, res->name) > 0)
      {
         INSERT(pRes, res, first_reserved, next, prev);
         break;
      }
   }

   if (!res)
   {
      LINK(pRes, first_reserved, last_reserved, next, prev);
   }

   return;
}


/*
 * Sort areas by name alphanumercially
 *      - 4/27/97, Fireblade
 */
void sort_area_by_name(AREA_DATA * pArea)
{
   AREA_DATA *temp_area;

   if (!pArea)
   {
      bug("Sort_area_by_name: NULL pArea");
      return;
   }
   for (temp_area = first_area_name; temp_area; temp_area = temp_area->next_sort_name)
   {
      if (strcmp(pArea->name, temp_area->name) < 0)
      {
         INSERT(pArea, temp_area, first_area_name, next_sort_name, prev_sort_name);
         break;
      }
   }
   if (!temp_area)
   {
      LINK(pArea, first_area_name, last_area_name, next_sort_name, prev_sort_name);
   }
   return;
}

/*
 * Sort by room vnums					-Altrag & Thoric
 */
void sort_area(AREA_DATA * pArea, bool proto)
{
   AREA_DATA *area = NULL;
   AREA_DATA *first_sort, *last_sort;
   bool found;

   if (!pArea)
   {
      bug("Sort_area: NULL pArea");
      return;
   }

   if (proto)
   {
      first_sort = first_bsort;
      last_sort = last_bsort;
   }
   else
   {
      first_sort = first_asort;
      last_sort = last_asort;
   }

   found = FALSE;
   pArea->next_sort = NULL;
   pArea->prev_sort = NULL;

   if (!first_sort)
   {
      pArea->prev_sort = NULL;
      pArea->next_sort = NULL;
      first_sort = pArea;
      last_sort = pArea;
      found = TRUE;
   }
   else
      for (area = first_sort; area; area = area->next_sort)
         if (pArea->low_r_vnum < area->low_r_vnum)
         {
            if (!area->prev_sort)
               first_sort = pArea;
            else
               area->prev_sort->next_sort = pArea;
            pArea->prev_sort = area->prev_sort;
            pArea->next_sort = area;
            area->prev_sort = pArea;
            found = TRUE;
            break;
         }

   if (!found)
   {
      pArea->prev_sort = last_sort;
      pArea->next_sort = NULL;
      last_sort->next_sort = pArea;
      last_sort = pArea;
   }

   if (proto)
   {
      first_bsort = first_sort;
      last_bsort = last_sort;
   }
   else
   {
      first_asort = first_sort;
      last_asort = last_sort;
   }
}


/*
 * Display vnums currently assigned to areas		-Altrag & Thoric
 * Sorted, and flagged if loaded.
 */
void show_vnums(CHAR_DATA * ch, int low, int high, bool proto, bool shownl, char *loadst, char *notloadst)
{
   AREA_DATA *pArea, *first_sort;
   int count, loaded;

   count = 0;
   loaded = 0;
   set_pager_color(AT_PLAIN, ch);
   if (proto)
      first_sort = first_bsort;
   else
      first_sort = first_asort;
   for (pArea = first_sort; pArea; pArea = pArea->next_sort)
   {
      if (IS_SET(pArea->status, AREA_DELETED))
         continue;
      if (pArea->low_r_vnum < low)
         continue;
      if (pArea->hi_r_vnum > high)
         break;
      if (IS_SET(pArea->status, AREA_LOADED))
         loaded++;
      else if (!shownl)
         continue;
      pager_printf(ch, "%-22s| Rooms: %6d - %-6d"
         " Objs: %6d - %-6d Mobs: %6d - %-6d%s\n\r",
         (pArea->filename ? pArea->filename : "(invalid)"),
         pArea->low_r_vnum, pArea->hi_r_vnum,
         pArea->low_o_vnum, pArea->hi_o_vnum, pArea->low_m_vnum, pArea->hi_m_vnum, IS_SET(pArea->status, AREA_LOADED) ? loadst : notloadst);
      count++;
   }
   pager_printf(ch, "Areas listed: %d  Loaded: %d\n\r", count, loaded);
   return;
}

/*
 * Shows prototype vnums ranges, and if loaded
 */
void do_vnums(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int low, high;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   low = 0;
   high = MAX_VNUM;
   if (arg1[0] != '\0')
   {
      low = atoi(arg1);
      if (arg2[0] != '\0')
         high = atoi(arg2);
   }
   show_vnums(ch, low, high, TRUE, TRUE, " *", "");
}

/*
 * Shows installed areas, sorted.  Mark unloaded areas with an X
 */
void do_zones(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int low, high;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   low = 0;
   high = MAX_VNUM;
   if (arg1[0] != '\0')
   {
      low = atoi(arg1);
      if (arg2[0] != '\0')
         high = atoi(arg2);
   }
   show_vnums(ch, low, high, FALSE, TRUE, "", " X");
}

/*
 * Show prototype areas, sorted.  Only show loaded areas
 */
void do_newzones(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int low, high;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   low = 0;
   high = MAX_VNUM;
   if (arg1[0] != '\0')
   {
      low = atoi(arg1);
      if (arg2[0] != '\0')
         high = atoi(arg2);
   }
   show_vnums(ch, low, high, TRUE, FALSE, "", " X");
}

/*
 * Save system info to data file
 */
void save_sysdata(SYSTEM_DATA sys)
{
   FILE *fp;
   char filename[MIL];
   int x, y, z;

   sprintf(filename, "%ssysdata.dat", SYSTEM_DIR);

   fclose(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_sysdata: fopen");
      perror(filename);
   }
   else
   {
      fprintf(fp, "#SYSTEM\n");
      fprintf(fp, "MudName	     %s~\n", sys.mud_name);
      fprintf(fp, "CodeVersion   %s~\n", sys.cversion);
      fprintf(fp, "MudVersion    %s~\n", sys.mversion);
      fprintf(fp, "TopPin	  %d\n", sys.top_pid);
      fprintf(fp, "TopKPin        %d\n", sys.top_kpid);
      fprintf(fp, "TopTPin        %d\n", sys.top_tpid);
      fprintf(fp, "Highplayers    %d\n", sys.alltimemax);
      fprintf(fp, "Highplayertime %s~\n", sys.time_of_max);
      fprintf(fp, "CheckImmHost   %d\n", sys.check_imm_host);
      fprintf(fp, "Nameresolving  %d\n", sys.NO_NAME_RESOLVING);
      fprintf(fp, "Waitforauth    %d\n", sys.WAIT_FOR_AUTH);
      fprintf(fp, "Readallmail    %d\n", sys.read_all_mail);
      fprintf(fp, "FirstRun       %d\n", sys.firstrun);
      fprintf(fp, "AccountEmail   %d\n", sys.accountemail);
      fprintf(fp, "ResetGame      %d\n", sys.resetgame);
      fprintf(fp, "Readmailfree   %d\n", sys.read_mail_free);
      fprintf(fp, "Writemailfree  %d\n", sys.write_mail_free);
      fprintf(fp, "Takeothersmail %d\n", sys.take_others_mail);
      fprintf(fp, "IMCMailVnum    %d\n", sys.imc_mail_vnum);
      fprintf(fp, "Muse           %d\n", sys.muse_level);
      fprintf(fp, "MaxAccounts    %d\n", sys.max_accounts);
      fprintf(fp, "MaxAccountChanges %d\n", sys.max_account_changes);
      fprintf(fp, "GemVnum        %d\n", sys.gem_vnum);
      fprintf(fp, "TimeoutLogin   %d\n", sys.timeout_login);
      fprintf(fp, "TimeoutNotes   %d\n", sys.timeout_notes);
      fprintf(fp, "TimeoutIdle    %d\n", sys.timeout_idle);
      fprintf(fp, "StartCalender  %d\n", sys.start_calender);
      fprintf(fp, "LastResCheck   %d\n", sys.lastrescheck);
      fprintf(fp, "LastPopCheck   %d\n", sys.lastpopcheck);
      fprintf(fp, "LastTaxCheck   %d\n", sys.lasttaxcheck);
      fprintf(fp, "Think          %d\n", sys.think_level);
      fprintf(fp, "Build          %d\n", sys.build_level);
      fprintf(fp, "Log            %d\n", sys.log_level);
      fprintf(fp, "Protoflag      %d\n", sys.level_modify_proto);
      fprintf(fp, "Overridepriv   %d\n", sys.level_override_private);
      fprintf(fp, "Msetplayer     %d\n", sys.level_mset_player);
      fprintf(fp, "Stunplrvsplr   %d\n", sys.stun_plr_vs_plr);
      fprintf(fp, "Stunregular    %d\n", sys.stun_regular);
      fprintf(fp, "Gougepvp       %d\n", sys.gouge_plr_vs_plr);
      fprintf(fp, "Gougenontank   %d\n", sys.gouge_nontank);
      fprintf(fp, "Bashpvp        %d\n", sys.bash_plr_vs_plr);
      fprintf(fp, "Bashnontank    %d\n", sys.bash_nontank);
      fprintf(fp, "Dodgemod       %d\n", sys.dodge_mod);
      fprintf(fp, "Parrymod       %d\n", sys.parry_mod);
      fprintf(fp, "Tumblemod      %d\n", sys.tumble_mod);
      fprintf(fp, "Damplrvsplr    %d\n", sys.dam_plr_vs_plr);
      fprintf(fp, "Damplrvsmob    %d\n", sys.dam_plr_vs_mob);
      fprintf(fp, "Dammobvsplr    %d\n", sys.dam_mob_vs_plr);
      fprintf(fp, "Dammobvsmob    %d\n", sys.dam_mob_vs_mob);
      fprintf(fp, "Forcepc        %d\n", sys.level_forcepc);
      fprintf(fp, "Guildoverseer  %s~\n", sys.guild_overseer);
      fprintf(fp, "Guildadvisor   %s~\n", sys.guild_advisor);
      fprintf(fp, "Saveflags      %d\n", sys.save_flags);
      fprintf(fp, "Savefreq       %d\n", sys.save_frequency);
      fprintf(fp, "Bestowdif      %d\n", sys.bestow_dif);
      fprintf(fp, "BanSiteLevel   %d\n", sys.ban_site_level);
      fprintf(fp, "BanRaceLevel   %d\n", sys.ban_race_level);
      fprintf(fp, "BanClassLevel  %d\n", sys.ban_class_level);
      fprintf(fp, "MorphOpt       %d\n", sys.morph_opt);
      fprintf(fp, "MaxKingdom     %d\n", sys.max_kingdom);
      fprintf(fp, "LastPortal     %d\n", sys.last_portal);
      fprintf(fp, "LastTrap       %d\n", sys.last_trap_uid);
      fprintf(fp, "LastInvTrap    %d\n", sys.last_invtrap_uid);
      fprintf(fp, "PetSave	     %d\n", sys.save_pets);
      fprintf(fp, "IdentTries     %d\n", sys.ident_retries);
      fprintf(fp, "Newbie_purge	%d\n", sys.newbie_purge);
      fprintf(fp, "Regular_purge	%d\n", sys.regular_purge);
      fprintf(fp, "Autopurge		%d\n", sys.CLEANPFILES);
      fprintf(fp, "Purgetime		%ld\n", sys.purgetime);
      fprintf(fp, "Exp_Percent     %d\n", sys.exp_percent);
      fprintf(fp, "Stat_Gain       %d\n", sys.stat_gain);
      fprintf(fp, "Top_Gem_Num     %d\n", sys.top_gem_num);
      fprintf(fp, "Quest_item1     %d\n", sys.quest_item1);
      fprintf(fp, "Quest_value1    %d\n", sys.quest_value1);
      fprintf(fp, "Quest_item2     %d\n", sys.quest_item2);
      fprintf(fp, "Quest_value2    %d\n", sys.quest_value2);
      fprintf(fp, "Quest_item3     %d\n", sys.quest_item3);
      fprintf(fp, "Quest_value3    %d\n", sys.quest_value3);
      fprintf(fp, "Quest_item4     %d\n", sys.quest_item4);
      fprintf(fp, "Quest_value4    %d\n", sys.quest_value4);
      fprintf(fp, "Quest_item5     %d\n", sys.quest_item5);
      fprintf(fp, "Quest_value5    %d\n", sys.quest_value5);
      fprintf(fp, "Quest_item6     %d\n", sys.quest_item6);
      fprintf(fp, "Quest_value6    %d\n", sys.quest_value6);
      fprintf(fp, "Quest_item7     %d\n", sys.quest_item7);
      fprintf(fp, "Quest_value7    %d\n", sys.quest_value7);
      fprintf(fp, "Quest_item8     %d\n", sys.quest_item8);
      fprintf(fp, "Quest_value8    %d\n", sys.quest_value8);
      fprintf(fp, "Quest_item9     %d\n", sys.quest_item9);
      fprintf(fp, "Quest_value9    %d\n", sys.quest_value9);
      fprintf(fp, "Quest_item10     %d\n", sys.quest_item10);
      fprintf(fp, "Quest_value10    %d\n", sys.quest_value10);
      fprintf(fp, "Quest_item11     %d\n", sys.quest_item11);
      fprintf(fp, "Quest_value11    %d\n", sys.quest_value11);
      fprintf(fp, "Quest_item12     %d\n", sys.quest_item12);
      fprintf(fp, "Quest_value12    %d\n", sys.quest_value12);
      for (x = 0; x <= MAX_SPHERE-1; x++)
      {
         fprintf(fp, "Toadvance%d      ", x+1);
         for (y = 0; y <= MAX_GROUP-1; y++)
         {
            for (z = 0; z <= MAX_TIER-2; z++)
               fprintf(fp, " %d", sys.toadvance[x][y][z]);  
         }
         fprintf(fp, "\n"); 
      }
      fprintf(fp, "End\n\n");
      fprintf(fp, "#END\n");
   }
   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}


void fread_sysdata(SYSTEM_DATA * sys, FILE * fp)
{
   char *word;
   bool fMatch;
   char *ln;
   int y, z;
   char arg[MIL];

   sys->time_of_max = NULL;
   sys->mud_name = NULL;
   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
         case 'A':
            KEY("AccountEmail", sys->accountemail, fread_number(fp));
            KEY("Autopurge", sys->CLEANPFILES, fread_number(fp));
            break;
         case 'B':
            KEY("Bashpvp", sys->bash_plr_vs_plr, fread_number(fp));
            KEY("Bashnontank", sys->bash_nontank, fread_number(fp));
            KEY("Bestowdif", sys->bestow_dif, fread_number(fp));
            KEY("Build", sys->build_level, fread_number(fp));
            KEY("BanSiteLevel", sys->ban_site_level, fread_number(fp));
            KEY("BanClassLevel", sys->ban_class_level, fread_number(fp));
            KEY("BanRaceLevel", sys->ban_race_level, fread_number(fp));
            break;

         case 'C':
            KEY("CheckImmHost", sys->check_imm_host, fread_number(fp));
            KEY("CodeVersion", sys->cversion, fread_string_nohash(fp));
            break;

         case 'D':
            KEY("Damplrvsplr", sys->dam_plr_vs_plr, fread_number(fp));
            KEY("Damplrvsmob", sys->dam_plr_vs_mob, fread_number(fp));
            KEY("Dammobvsplr", sys->dam_mob_vs_plr, fread_number(fp));
            KEY("Dammobvsmob", sys->dam_mob_vs_mob, fread_number(fp));
            KEY("Dodgemod", sys->dodge_mod, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               if (!sys->time_of_max)
                  sys->time_of_max = str_dup("(not recorded)");
               if (!sys->mud_name)
                  sys->mud_name = str_dup("(Name Not Set)");
               new_pfile_time_t = sys->purgetime; /* Samson 5-9-99 */
               return;
            }
            KEY("Exp_Percent", sys->exp_percent, fread_number(fp));
            break;

         case 'F':
            KEY("FirstRun", sys->firstrun, fread_number(fp));
            KEY("Forcepc", sys->level_forcepc, fread_number(fp));
            break;

         case 'G':
            KEY("GemVnum", sys->gem_vnum, fread_number(fp));
            KEY("Gougepvp", sys->gouge_plr_vs_plr, fread_number(fp));
            KEY("Gougenontank", sys->gouge_nontank, fread_number(fp));
            KEY("Guildoverseer", sys->guild_overseer, fread_string(fp));
            KEY("Guildadvisor", sys->guild_advisor, fread_string(fp));
            break;

         case 'H':
            KEY("Highplayers", sys->alltimemax, fread_number(fp));
            KEY("Highplayertime", sys->time_of_max, fread_string_nohash(fp));
            break;

         case 'I':
            KEY("IdentTries", sys->ident_retries, fread_number(fp));
            KEY("IMCMailVnum", sys->imc_mail_vnum, fread_number(fp));
            break;

         case 'L':
            KEY("LastPortal", sys->last_portal, fread_number(fp));
            KEY("LastTrap", sys->last_trap_uid, fread_number(fp));
            KEY("LastInvTrap", sys->last_invtrap_uid, fread_number(fp));
            KEY("LastPopCheck", sys->lastpopcheck, fread_number(fp));
            KEY("LastResCheck", sys->lastrescheck, fread_number(fp));
            KEY("LastTaxCheck", sys->lasttaxcheck, fread_number(fp));
            KEY("Log", sys->log_level, fread_number(fp));
            break;

         case 'M':
            KEY("MaxKingdom", sys->max_kingdom, fread_number(fp));
            KEY("MorphOpt", sys->morph_opt, fread_number(fp));
            KEY("Msetplayer", sys->level_mset_player, fread_number(fp));
            KEY("MudName", sys->mud_name, fread_string_nohash(fp));
            KEY("Muse", sys->muse_level, fread_number(fp));
            KEY("MudVersion", sys->mversion, fread_string_nohash(fp));
            KEY("MaxAccounts", sys->max_accounts, fread_number(fp));
            KEY("MaxAccountChanges", sys->max_account_changes, fread_number(fp));
            break;

         case 'N':
            KEY("Nameresolving", sys->NO_NAME_RESOLVING, fread_number(fp));
            KEY("Newbie_purge", sys->newbie_purge, fread_number(fp));
            break;

         case 'O':
            KEY("Overridepriv", sys->level_override_private, fread_number(fp));
            break;

         case 'P':
            KEY("Parrymod", sys->parry_mod, fread_number(fp));
            KEY("PetSave", sys->save_pets, fread_number(fp));
            KEY("Protoflag", sys->level_modify_proto, fread_number(fp));
            KEY("Purgetime", sys->purgetime, fread_number(fp)); /* Samson 5-9-99 */
            break;

         case 'Q':
            KEY("Quest_item1", sys->quest_item1, fread_number(fp));
            KEY("Quest_value1", sys->quest_value1, fread_number(fp));
            KEY("Quest_item2", sys->quest_item2, fread_number(fp));
            KEY("Quest_value2", sys->quest_value2, fread_number(fp));
            KEY("Quest_item3", sys->quest_item3, fread_number(fp));
            KEY("Quest_value3", sys->quest_value3, fread_number(fp));
            KEY("Quest_item4", sys->quest_item4, fread_number(fp));
            KEY("Quest_value4", sys->quest_value4, fread_number(fp));
            KEY("Quest_item5", sys->quest_item5, fread_number(fp));
            KEY("Quest_value5", sys->quest_value5, fread_number(fp));
            KEY("Quest_item6", sys->quest_item6, fread_number(fp));
            KEY("Quest_value6", sys->quest_value6, fread_number(fp));
            KEY("Quest_item7", sys->quest_item7, fread_number(fp));
            KEY("Quest_value7", sys->quest_value7, fread_number(fp));
            KEY("Quest_item8", sys->quest_item8, fread_number(fp));
            KEY("Quest_value8", sys->quest_value8, fread_number(fp));
            KEY("Quest_item9", sys->quest_item9, fread_number(fp));
            KEY("Quest_value9", sys->quest_value9, fread_number(fp));
            KEY("Quest_item10", sys->quest_item10, fread_number(fp));
            KEY("Quest_value10", sys->quest_value10, fread_number(fp));
            KEY("Quest_item11", sys->quest_item11, fread_number(fp));
            KEY("Quest_value11", sys->quest_value11, fread_number(fp));
            KEY("Quest_item12", sys->quest_item12, fread_number(fp));
            KEY("Quest_value12", sys->quest_value12, fread_number(fp));


         case 'R':
            KEY("Readallmail", sys->read_all_mail, fread_number(fp));
            KEY("Readmailfree", sys->read_mail_free, fread_number(fp));
            KEY("Regular_purge", sys->regular_purge, fread_number(fp));
            KEY("ResetGame", sys->resetgame, fread_number(fp));
            break;

         case 'S':
            KEY("Startcalender", sys->start_calender, fread_number(fp));
            KEY("Stunplrvsplr", sys->stun_plr_vs_plr, fread_number(fp));
            KEY("Stunregular", sys->stun_regular, fread_number(fp));
            KEY("Saveflags", sys->save_flags, fread_number(fp));
            KEY("Stat_Gain", sys->stat_gain, fread_number(fp));
            KEY("Savefreq", sys->save_frequency, fread_number(fp));
            break;

         case 'T':
            KEY("Takeothersmail", sys->take_others_mail, fread_number(fp));
            KEY("TimeoutLogin", sys->timeout_login, fread_number(fp));
            KEY("TimeoutNotes", sys->timeout_notes, fread_number(fp));
            KEY("TimeoutIdle", sys->timeout_idle, fread_number(fp));
            KEY("Top_Gem_Num", sys->top_gem_num, fread_number(fp));
            KEY("Think", sys->think_level, fread_number(fp));
            if (!str_cmp(word, "Toadvance1"))
            {               
               ln = fread_line(fp);
               for (y = 0; y <= MAX_GROUP-1; y++)
               {
                  for (z = 0; z <= MAX_TIER-2; z++)
                  {
                     ln = one_argument(ln, arg);   
                     sys->toadvance[0][y][z] = atoi(arg);
                  }
               }
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Toadvance2"))
            {               
               ln = fread_line(fp);
               for (y = 0; y <= MAX_GROUP-1; y++)
               {
                  for (z = 0; z <= MAX_TIER-2; z++)
                  {
                     ln = one_argument(ln, arg);   
                     sys->toadvance[1][y][z] = atoi(arg);
                  }
               }
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Toadvance3"))
            {               
               ln = fread_line(fp);
               for (y = 0; y <= MAX_GROUP-1; y++)
               {
                  for (z = 0; z <= MAX_TIER-2; z++)
                  {
                     ln = one_argument(ln, arg);   
                     sys->toadvance[2][y][z] = atoi(arg);
                  }
               }
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Toadvance4"))
            {               
               ln = fread_line(fp);
               for (y = 0; y <= MAX_GROUP-1; y++)
               {
                  for (z = 0; z <= MAX_TIER-2; z++)
                  {
                     ln = one_argument(ln, arg);   
                     sys->toadvance[3][y][z] = atoi(arg);
                  }
               }
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Toadvance5"))
            {               
               ln = fread_line(fp);
               for (y = 0; y <= MAX_GROUP-1; y++)
               {
                  for (z = 0; z <= MAX_TIER-2; z++)
                  {
                     ln = one_argument(ln, arg);   
                     sys->toadvance[4][y][z] = atoi(arg);
                  }
               }
               fMatch = TRUE;
            }
            KEY("TopPin", sys->top_pid, fread_number(fp));
            KEY("TopKPin", sys->top_kpid, fread_number(fp));
            KEY("TopTPin", sys->top_tpid, fread_number(fp));
            KEY("Tumblemod", sys->tumble_mod, fread_number(fp));
            break;


         case 'W':
            KEY("Waitforauth", sys->WAIT_FOR_AUTH, fread_number(fp));
            KEY("Writemailfree", sys->write_mail_free, fread_number(fp));
            break;
      }


      if (!fMatch)
      {
         bug("Fread_sysdata: no match: %s", word);
      }
   }
}



/*
 * Load the sysdata file
 */
bool load_systemdata(SYSTEM_DATA * sys)
{
   char filename[MIL];
   FILE *fp;
   bool found;

   found = FALSE;
   sprintf(filename, "%ssysdata.dat", SYSTEM_DIR);

   if ((fp = fopen(filename, "r")) != NULL)
   {

      found = TRUE;
      for (;;)
      {
         char letter;
         char *word;

         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            bug("Load_sysdata_file: # not found.");
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "SYSTEM"))
         {
            fread_sysdata(sys, fp);
            break;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_sysdata_file: bad section.");
            break;
         }
      }
      fclose(fp);
   }

   if (!sysdata.guild_overseer)
      sysdata.guild_overseer = STRALLOC("");
   if (!sysdata.guild_advisor)
      sysdata.guild_advisor = STRALLOC("");
   return found;
}

void load_watchlist(void)
{
   WATCH_DATA *pwatch;
   FILE *fp;
   int number;
   CMDTYPE *cmd;

   if (!(fp = fopen(SYSTEM_DIR WATCH_LIST, "r")))
      return;

   for (;;)
   {
      if (feof(fp))
      {
         bug("Load_watchlist: no -1 found.");
         fclose(fp);
         return;
      }
      number = fread_number(fp);
      if (number == -1)
      {
         fclose(fp);
         return;
      }

      CREATE(pwatch, WATCH_DATA, 1);
      pwatch->imm_level = number;
      pwatch->imm_name = fread_string_nohash(fp);
      pwatch->target_name = fread_string_nohash(fp);
      if (strlen(pwatch->target_name) < 2)
         DISPOSE(pwatch->target_name);
      pwatch->player_site = fread_string_nohash(fp);
      if (strlen(pwatch->player_site) < 2)
         DISPOSE(pwatch->player_site);

      /* Check for command watches */
      if (pwatch->target_name)
         for (cmd = command_hash[(int) pwatch->target_name[0]]; cmd; cmd = cmd->next)
         {
            if (!str_cmp(pwatch->target_name, cmd->name))
            {
               SET_BIT(cmd->flags, CMD_WATCH);
               break;
            }
         }

      LINK(pwatch, first_watch, last_watch, next, prev);
   }
}


/* Check to make sure range of vnums is free - Scryn 2/27/96 */

void do_check_vnums(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char buf2[MSL];
   AREA_DATA *pArea;
   char arg1[MSL];
   char arg2[MSL];
   bool room, mob, obj, all, area_conflict;
   int low_range, high_range;

   room = FALSE;
   mob = FALSE;
   obj = FALSE;
   all = FALSE;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0')
   {
      send_to_char("Please specify room, mob, object, or all as your first argument.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "room"))
      room = TRUE;

   else if (!str_cmp(arg1, "mob"))
      mob = TRUE;

   else if (!str_cmp(arg1, "object"))
      obj = TRUE;

   else if (!str_cmp(arg1, "all"))
      all = TRUE;
   else
   {
      send_to_char("Please specify room, mob, or object as your first argument.\n\r", ch);
      return;
   }

   if (arg2[0] == '\0')
   {
      send_to_char("Please specify the low end of the range to be searched.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Please specify the high end of the range to be searched.\n\r", ch);
      return;
   }

   low_range = atoi(arg2);
   high_range = atoi(argument);

   if (low_range < 1 || low_range > MAX_VNUM)
   {
      send_to_char("Invalid argument for bottom of range.\n\r", ch);
      return;
   }

   if (high_range < 1 || high_range > MAX_VNUM)
   {
      send_to_char("Invalid argument for top of range.\n\r", ch);
      return;
   }

   if (high_range < low_range)
   {
      send_to_char("Bottom of range must be below top of range.\n\r", ch);
      return;
   }

   if (all)
   {
      sprintf(buf, "room %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "mob %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      sprintf(buf, "object %d %d", low_range, high_range);
      do_check_vnums(ch, buf);
      return;
   }
   set_char_color(AT_PLAIN, ch);

   for (pArea = first_asort; pArea; pArea = pArea->next_sort)
   {
      area_conflict = FALSE;
      if (IS_SET(pArea->status, AREA_DELETED))
         continue;
      else if (room)
      {
         if (low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range)
            area_conflict = TRUE;

         if ((low_range >= pArea->low_r_vnum) && (low_range <= pArea->hi_r_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_r_vnum) && (high_range >= pArea->low_r_vnum))
            area_conflict = TRUE;
      }

      if (mob)
      {
         if (low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range)
            area_conflict = TRUE;
         if ((low_range >= pArea->low_m_vnum) && (low_range <= pArea->hi_m_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_m_vnum) && (high_range >= pArea->low_m_vnum))
            area_conflict = TRUE;
      }

      if (obj)
      {
         if (low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range)
            area_conflict = TRUE;

         if ((low_range >= pArea->low_o_vnum) && (low_range <= pArea->hi_o_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_o_vnum) && (high_range >= pArea->low_o_vnum))
            area_conflict = TRUE;
      }

      if (area_conflict)
      {
         sprintf(buf, "Conflict:%-15s| ", (pArea->filename ? pArea->filename : "(invalid)"));
         if (room)
            sprintf(buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, pArea->hi_r_vnum);
         if (mob)
            sprintf(buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, pArea->hi_m_vnum);
         if (obj)
            sprintf(buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, pArea->hi_o_vnum);

         strcat(buf, buf2);
         send_to_char(buf, ch);
      }
   }
   for (pArea = first_bsort; pArea; pArea = pArea->next_sort)
   {
      area_conflict = FALSE;
      if (IS_SET(pArea->status, AREA_DELETED))
         continue;
      else if (room)
      {
         if (low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range)
            area_conflict = TRUE;

         if ((low_range >= pArea->low_r_vnum) && (low_range <= pArea->hi_r_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_r_vnum) && (high_range >= pArea->low_r_vnum))
            area_conflict = TRUE;
      }

      if (mob)
      {
         if (low_range < pArea->low_m_vnum && pArea->low_m_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_m_vnum && pArea->hi_m_vnum < high_range)
            area_conflict = TRUE;
         if ((low_range >= pArea->low_m_vnum) && (low_range <= pArea->hi_m_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_m_vnum) && (high_range >= pArea->low_m_vnum))
            area_conflict = TRUE;
      }

      if (obj)
      {
         if (low_range < pArea->low_o_vnum && pArea->low_o_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_o_vnum && pArea->hi_o_vnum < high_range)
            area_conflict = TRUE;

         if ((low_range >= pArea->low_o_vnum) && (low_range <= pArea->hi_o_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_o_vnum) && (high_range >= pArea->low_o_vnum))
            area_conflict = TRUE;
      }

      if (area_conflict)
      {
         sprintf(buf, "Conflict:%-15s| ", (pArea->filename ? pArea->filename : "(invalid)"));
         if (room)
            sprintf(buf2, "Rooms: %5d - %-5d\n\r", pArea->low_r_vnum, pArea->hi_r_vnum);
         if (mob)
            sprintf(buf2, "Mobs: %5d - %-5d\n\r", pArea->low_m_vnum, pArea->hi_m_vnum);
         if (obj)
            sprintf(buf2, "Objects: %5d - %-5d\n\r", pArea->low_o_vnum, pArea->hi_o_vnum);

         strcat(buf, buf2);
         send_to_char(buf, ch);
      }
   }

/*
    for ( pArea = first_asort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  ch_printf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }

    for ( pArea = first_bsort; pArea; pArea = pArea->next_sort )
    {
        area_conflict = FALSE;
	if ( IS_SET( pArea->status, AREA_DELETED ) )
	   continue;
	else
	if (room)
	  if((pArea->low_r_vnum >= low_range) 
	  && (pArea->hi_r_vnum <= high_range))
	    area_conflict = TRUE;

	if (mob)
	  if((pArea->low_m_vnum >= low_range) 
	  && (pArea->hi_m_vnum <= high_range))
	    area_conflict = TRUE;

	if (obj)
	  if((pArea->low_o_vnum >= low_range) 
	  && (pArea->hi_o_vnum <= high_range))
	    area_conflict = TRUE;

	if (area_conflict)
	  sprintf(ch, "Conflict:%-15s| Rooms: %5d - %-5d"
		     " Objs: %5d - %-5d Mobs: %5d - %-5d\n\r",
		(pArea->filename ? pArea->filename : "(invalid)"),
		pArea->low_r_vnum, pArea->hi_r_vnum,
		pArea->low_o_vnum, pArea->hi_o_vnum,
		pArea->low_m_vnum, pArea->hi_m_vnum );
    }
*/
   return;
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void)
{
   return;
}


/*
 * Load weather data from appropriate file in system dir
 * Last Modified: July 24, 1997
 * Fireblade
 */
bool load_weatherdata()
{
   char filename[MIL];
   FILE *fp;
   FRONT_DATA *fnt;
   TORNADO_DATA *torn;
   bool fMatch;

   sprintf(filename, "%sweather.dat", SYSTEM_DIR);

   if ((fp = fopen(filename, "r")) != NULL)
   {
      for (;;)
      {
         char letter;
         char *word;

         letter = fread_letter(fp);

         if (letter != '#')
         {
            bug("load_weatherdata: # not found");
            return FALSE;
         }

         word = fread_word(fp);

         if (!str_cmp(word, "FRONT"))
         {
            CREATE(fnt, FRONT_DATA, 1);
            for (;;)
            {
               word = feof(fp) ? "End" : fread_word(fp);
               switch (UPPER(word[0]))
               {
                  case '*':
                     fMatch = TRUE;
                     fread_to_eol(fp);
                     break;

                  case 'M':
                     KEY("Map", fnt->map, fread_number(fp));
                     break;
                  case 'S':
                     KEY("Size", fnt->size, fread_number(fp));
                     KEY("Speed", fnt->speed, fread_number(fp));
                     break;
                  case 'T':
                     KEY("Type", fnt->type, fread_number(fp));
                     KEY("Typec", fnt->typec, fread_number(fp));
                     break;
                  case 'X':
                     KEY("X", fnt->x, fread_number(fp));
                     break;
                  case 'Y':
                     KEY("Y", fnt->y, fread_number(fp));
                     break;
               }
               if (!str_cmp(word, "End"))
               {
                  int ttype;
                  int cnt;

                  ttype = fnt->typec; //Creates 8 different kinds, produces slightly different angles.
                  for (cnt = 0; cnt < 30; cnt++)
                     fnt->f[cnt] = -1;
                  for (cnt = 0; cnt < fnt->size; cnt++)
                  {
                     fnt->f[cnt] = front_cr[ttype][cnt];
                  }
                  LINK(fnt, first_front, last_front, next, prev);
                  break;
               }
            }
         }
         else if (!str_cmp(word, "TORNADO"))
         {
            CREATE(torn, TORNADO_DATA, 1);
            for (;;)
            {
               word = feof(fp) ? "End" : fread_word(fp);
               switch (UPPER(word[0]))
               {
                  case '*':
                     fMatch = TRUE;
                     fread_to_eol(fp);
                     break;
                  case 'D':
                     KEY("Dir", torn->power, fread_number(fp));
                     break;
                  case 'M':
                     KEY("Map", torn->map, fread_number(fp));
                     break;
                  case 'P':
                     KEY("Power", torn->power, fread_number(fp));
                     break;
                  case 'T':
                     KEY("Turns", torn->turns, fread_number(fp));
                     break;
                  case 'X':
                     KEY("X", torn->x, fread_number(fp));
                     break;
                  case 'Y':
                     KEY("Y", torn->y, fread_number(fp));
                     break;
               }
               if (!str_cmp(word, "End"))
               {
                  LINK(torn, first_tornado, last_tornado, next, prev);
                  break;
               }
            }
         }
         else if (!str_cmp(word, "END"))
         {
            fclose(fp);
            return TRUE;
         }
         else
         {
            bug("load_weatherdata: unknown field");
            fclose(fp);
            break;
         }
      }
   }
   else
      return FALSE;


   return FALSE;
}

/*
 * Write data for global weather parameters
 * Last Modified: July 24, 1997
 * Fireblade
 */
void save_weatherdata()
{
   char filename[MIL];
   FILE *fp;
   FRONT_DATA *frt;
   TORNADO_DATA *torn;

   sprintf(filename, "%sweather.dat", SYSTEM_DIR);

   if ((fp = fopen(filename, "w")) != NULL)
   {
      for (frt = first_front; frt; frt = frt->next)
      {
         fprintf(fp, "#FRONT\n");
         fprintf(fp, "X          %d\n", frt->x);
         fprintf(fp, "Y          %d\n", frt->y);
         fprintf(fp, "Map        %d\n", frt->map);
         fprintf(fp, "Speed      %d\n", frt->speed);
         fprintf(fp, "Size       %d\n", frt->size);
         fprintf(fp, "Type       %d\n", frt->type);
         fprintf(fp, "Typec      %d\n", frt->typec);
         fprintf(fp, "End\n\n");
      }
      for (torn = first_tornado; torn; torn = torn->next)
      {
         fprintf(fp, "#TORNADO\n");
         fprintf(fp, "X         %d\n", torn->x);
         fprintf(fp, "Y         %d\n", torn->y);
         fprintf(fp, "Map       %d\n", torn->map);
         fprintf(fp, "Power     %d\n", torn->power);
         fprintf(fp, "Turns     %d\n", torn->turns);
         fprintf(fp, "Dir       %d\n", torn->dir);
         fprintf(fp, "End\n\r");
      }
      fprintf(fp, "\n#END\n");
      fclose(fp);
   }
   else
   {
      bug("save_weatherdata: could not open file");
   }

   return;
}

void load_projects(void) /* Copied load_boards structure for simplicity */
{
   char filename[MIL];
   FILE *fp;
   PROJECT_DATA *project;

   first_project = NULL;
   last_project = NULL;
   sprintf(filename, "%s", PROJECTS_FILE);
   if ((fp = fopen(filename, "r")) == NULL)
      return;

   while ((project = read_project(filename, fp)) != NULL)
      LINK(project, first_project, last_project, next, prev);

   return;
}

PROJECT_DATA *read_project(char *filename, FILE * fp)
{
   PROJECT_DATA *project;
   NOTE_DATA *log, *tlog;
   char *word;
   char buf[MSL];
   bool fMatch;
   char letter;

   do
   {
      letter = getc(fp);
      if (feof(fp))
      {
         fclose(fp);
         return NULL;
      }
   }
   while (isspace(letter));
   ungetc(letter, fp);

   CREATE(project, PROJECT_DATA, 1);

#ifdef KEY
#undef KEY
#endif
#define KEY( literal, field, value )                                    \
                                if ( !str_cmp( word, literal ) )        \
                                {                                       \
                                    field  = value;                     \
                                    fMatch = TRUE;                      \
                                    break;                              \
                                }
   project->first_log = NULL;
   project->last_log = NULL;
   project->next = NULL;
   project->prev = NULL;
   project->coder = NULL;
   project->description = STRALLOC("");
   project->name = STRALLOC("");
   project->owner = STRALLOC("");
   project->date = STRALLOC("Not Set?!");
   project->status = STRALLOC("No update.");
   project->rewardee = STRALLOC("None");

   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
         case 'C':
            KEY("Coder", project->coder, fread_string_nohash(fp));
            break;
         case 'D':
            if (!str_cmp(word, "Date"))
               STRFREE(project->date);
            else if (!str_cmp(word, "Description"))
               STRFREE(project->description);
            KEY("Date", project->date, fread_string(fp));
            KEY("Description", project->description, fread_string(fp));
            break;
         case 'E':
            if (!str_cmp(word, "End"))
            {
               if (!project->description)
                  project->description = STRALLOC("");
               if (!project->name)
                  project->name = STRALLOC("");
               if (!project->owner)
                  project->owner = STRALLOC("");
               if (!project->date)
                  project->date = STRALLOC("Not Set?!");
               if (!project->status)
                  project->status = STRALLOC("No update.");
               if (str_cmp(project->owner, "None"))
                  project->taken = TRUE;
               return project;
            }
            break;
         case 'L':
            if (!str_cmp(word, "Log"))
            {
               fread_to_eol(fp);
               log = read_log(fp);
               if (!log)
               {
                  sprintf(buf, "read_project: couldn't read log, aborting");
                  bug(buf, 0);
                  exit(1);
               }
               if (!log->sender)
                  log->sender = STRALLOC("");
               if (!log->date)
                  log->date = STRALLOC("");
               if (!log->subject)
                  log->subject = STRALLOC("None");
               log->to_list = STRALLOC("");
               LINK(log, project->first_log, project->last_log, next, prev);
               fMatch = TRUE;
               break;
            }
            break;
         case 'N':
            if (!str_cmp(word, "Name"))
               STRFREE(project->name);
            KEY("Name", project->name, fread_string_nohash(fp));
            break;
         case 'O':
            if (!str_cmp(word, "Owner"))
               STRFREE(project->owner);
            KEY("Owner", project->owner, fread_string(fp));
            break;
         case 'P':
            KEY("Points", project->points, fread_number(fp));
            break; 
             
         case 'R':
            if (!str_cmp(word, "Rewardee"))
               STRFREE(project->rewardee);
            KEY("Rewardee", project->rewardee, fread_string(fp));
            KEY("Rewardedpoints", project->rewardedpoints, fread_number(fp));
            break;
            
         case 'S':
            if (!str_cmp(word, "Status"))
               STRFREE(project->status);
            KEY("Status", project->status, fread_string(fp));
            break;
         case 'T':
            KEY("Time", project->time, fread_number(fp));
            KEY("Type", project->type, fread_number(fp));
            break;
      }
      if (!fMatch)
      {
         sprintf(buf, "read_project: no match: %s", word);
         bug(buf, 0);
      }
   }
   log = project->last_log;
   while (log)
   {
      UNLINK(log, project->first_log, project->last_log, next, prev);
      tlog = log->prev;
      free_note(log);
      log = tlog;
   }
   if (project->coder)
      DISPOSE(project->coder);
   if (project->description)
      STRFREE(project->description);
   if (project->name)
      STRFREE(project->name);
   if (project->owner)
      STRFREE(project->owner);
   if (project->date)
      STRFREE(project->date);
   if (project->status)
      STRFREE(project->status);
   if (project->rewardee)
      STRFREE(project->rewardee);
   DISPOSE(project);
   return project;
}

NOTE_DATA *read_log(FILE * fp)
{
   NOTE_DATA *log;
   char *word;

   CREATE(log, NOTE_DATA, 1);

   for (;;)
   {
      word = fread_word(fp);

      if (!str_cmp(word, "Sender"))
         log->sender = fread_string(fp);
      else if (!str_cmp(word, "Date"))
         log->date = fread_string(fp);
      else if (!str_cmp(word, "Subject"))
         log->subject = fread_string(fp);
      else if (!str_cmp(word, "Text"))
         log->text = fread_string(fp);
      else if (!str_cmp(word, "Endlog"))
      {
         fread_to_eol(fp);
         log->next = NULL;
         log->prev = NULL;
         return log;
      }
      else
      {
         DISPOSE(log);
         bug("read_log: bad key word.", 0);
         return NULL;
      }
   }
}


void write_projects()
{
   PROJECT_DATA *project;
   NOTE_DATA *log;
   FILE *fpout;
   char filename[MIL];

   sprintf(filename, "%s", PROJECTS_FILE);
   fpout = fopen(filename, "w");
   if (!fpout)
   {
      bug("FATAL: cannot open projects.txt for writing!\n\r", 0);
      return;
   }
   for (project = first_project; project; project = project->next)
   {
      fprintf(fpout, "Name		   %s~\n", project->name);
      fprintf(fpout, "Owner		   %s~\n", (project->owner) ? project->owner : "None");
      if (project->coder)
         fprintf(fpout, "Coder		    %s~\n", project->coder);
      fprintf(fpout, "Type             %d\n", project->type);
      fprintf(fpout, "Time             %d\n", project->time);
      fprintf(fpout, "Status		   %s~\n", (project->status) ? project->status : "No update.");
      fprintf(fpout, "Date		   %s~\n", (project->date) ? project->date : "Not Set?!?");
      fprintf(fpout, "Rewardee     %s~\n", (project->rewardee) ? project->rewardee : "None");
      fprintf(fpout, "Points       %d\n", project->points);
      fprintf(fpout, "Rewardedpoints  %d\n", project->rewardedpoints);
      if (project->description)
         fprintf(fpout, "Description         %s~\n", project->description);
      for (log = project->first_log; log; log = log->next)
         fprintf(fpout, "Log\nSender %s~\nDate %s~\nSubject %s~\nText %s~\nEndlog\n", log->sender, log->date, log->subject, log->text);

      fprintf(fpout, "End\n");
   }
   fclose(fpout);
}
