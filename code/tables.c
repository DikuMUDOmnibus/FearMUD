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
 * 			Table load/save Module				                       *
 ****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "mud.h"


#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

bool load_race_file(char *fname);
bool load_kingdom_file(char *fname);
void write_race_file(int ra);


/* global variables */
int top_sn;
int top_herb;
int gem_num;
int box_num;
int forge_num;
int slab_num;  
int boughtsaveversion;
int globaltownload;
TOWN_DATA *globaltownptr;
MARKET_DATA *globalmarketptr;
SKILLTYPE *skill_table[MAX_SKILL];
struct class_type *class_table[1];
KINGDOM_DATA *kingdom_table[MAX_KINGDOM];
NPCRACE_DATA *npcrace_table[MAX_NPCRACE_TABLE];
PORTAL_DATA *portal_show[LAST_PORTAL];
RACE_TYPE *race_table[MAX_RACE];
SKILLTYPE *herb_table[MAX_HERB];
SKILLTYPE *disease_table[MAX_DISEASE];
unsigned char battle_descriptions[7][3][100][60];
int high_value[7][3];

LANG_DATA *first_lang;
LANG_DATA *last_lang;

KCHEST_DATA *first_kchest;
KCHEST_DATA *last_kchest;

char *const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Racial", "Disease" };

SPELL_FUN *spell_function(char *name)
{
   if (!str_cmp(name, "spell_smaug"))
      return spell_smaug;
   if (!str_cmp(name, "spell_eye_of_god"))
      return spell_eye_of_god;
   if (!str_cmp(name, "spell_resurrection"))
      return spell_resurrection;
   if (!str_cmp(name, "spell_summon_corpse"))
      return spell_summon_corpse;
   if (!str_cmp(name, "spell_greater_resurrection"))
      return spell_greater_resurrection;
   if (!str_cmp(name, "spell_lesser_resurrection"))
      return spell_lesser_resurrection;
   if (!str_cmp(name, "spell_web"))
      return spell_web;
   if (!str_cmp(name, "spell_revitalize_spirit"))
      return spell_revitalize_spirit;
   if (!str_cmp(name, "spell_snare"))
      return spell_snare;
   if (!str_cmp(name, "spell_extradimensional_portal"))
      return spell_extradimensional_portal;
   if (!str_cmp(name, "spell_animate_dead"))
      return spell_animate_dead;
   if (!str_cmp(name, "spell_astral_walk"))
      return spell_astral_walk;
   if (!str_cmp(name, "spell_bless_weapon"))
      return spell_bless_weapon;
   if (!str_cmp(name, "spell_blindness"))
      return spell_blindness;
   if (!str_cmp(name, "spell_cause_critical"))
      return spell_cause_critical;
   if (!str_cmp(name, "spell_restore_limb"))
      return spell_restore_limb;
   if (!str_cmp(name, "spell_revitalize_limb"))
      return spell_revitalize_limb;
   if (!str_cmp(name, "spell_cause_light"))
      return spell_cause_light;
   if (!str_cmp(name, "spell_cause_serious"))
      return spell_cause_serious;
   if (!str_cmp(name, "spell_charm_person"))
      return spell_charm_person;
   if (!str_cmp(name, "spell_create_food"))
      return spell_create_food;
   if (!str_cmp(name, "spell_create_water"))
      return spell_create_water;
   if (!str_cmp(name, "spell_bless_weapon"))
      return spell_bless_weapon;
   if (!str_cmp(name, "spell_cure_blindness"))
      return spell_cure_blindness;
   if (!str_cmp(name, "spell_cure_poison"))
      return spell_cure_poison;
   if (!str_cmp(name, "spell_curse"))
      return spell_curse;
   if (!str_cmp(name, "spell_detect_poison"))
      return spell_detect_poison;
   if (!str_cmp(name, "spell_disenchant_weapon"))
      return spell_disenchant_weapon;
   if (!str_cmp(name, "spell_dispel_magic"))
      return spell_dispel_magic;
   if (!str_cmp(name, "spell_earthquake"))
      return spell_earthquake;
   if (!str_cmp(name, "spell_enchant_weapon"))
      return spell_enchant_weapon;
   if (!str_cmp(name, "spell_faerie_fire"))
      return spell_faerie_fire;
   if (!str_cmp(name, "spell_faerie_fog"))
      return spell_faerie_fog;
   if (!str_cmp(name, "spell_farsight"))
      return spell_farsight;
   if (!str_cmp(name, "spell_knock"))
      return spell_knock;
   if (!str_cmp(name, "spell_harm"))
      return spell_harm;
   if (!str_cmp(name, "spell_holy_food"))
      return spell_holy_food;
   if (!str_cmp(name, "spell_identify"))
      return spell_identify;
   if (!str_cmp(name, "spell_invis"))
      return spell_invis;
   if (!str_cmp(name, "spell_knowenemy"))
      return spell_knowenemy;
   if (!str_cmp(name, "spell_locate_object"))
      return spell_locate_object;
   if (!str_cmp(name, "spell_pass_door"))
      return spell_pass_door;
   if (!str_cmp(name, "spell_poison"))
      return spell_poison;
   if (!str_cmp(name, "spell_polymorph"))
      return spell_polymorph;
   if (!str_cmp(name, "spell_possess"))
      return spell_possess;
   if (!str_cmp(name, "spell_recharge"))
      return spell_recharge;
   if (!str_cmp(name, "spell_remove_curse"))
      return spell_remove_curse;
   if (!str_cmp(name, "spell_remove_invis"))
      return spell_remove_invis;
   if (!str_cmp(name, "spell_remove_trap"))
      return spell_remove_trap;
   if (!str_cmp(name, "spell_sleep"))
      return spell_sleep;
   if (!str_cmp(name, "spell_summon"))
      return spell_summon;
   if (!str_cmp(name, "spell_wizard_eye"))
      return spell_wizard_eye;
   if (!str_cmp(name, "spell_weaken"))
      return spell_weaken;
   if (!str_cmp(name, "spell_word_of_recall"))
      return spell_word_of_recall;
   if (!str_cmp(name, "spell_acid_breath"))
      return spell_acid_breath;
   if (!str_cmp(name, "spell_fire_breath"))
      return spell_fire_breath;
   if (!str_cmp(name, "spell_frost_breath"))
      return spell_frost_breath;
   if (!str_cmp(name, "spell_gas_breath"))
      return spell_gas_breath;
   if (!str_cmp(name, "spell_lightning_breath"))
      return spell_lightning_breath;
   if (!str_cmp(name, "spell_portal"))
      return spell_portal;
   if (!str_cmp(name, "spell_bethsaidean_touch"))
      return spell_bethsaidean_touch;
   if (!str_cmp(name, "spell_expurgation"))
      return spell_expurgation;
   if (!str_cmp(name, "spell_sacral_divinity"))
      return spell_sacral_divinity;
   if (!str_cmp(name, "spell_holy_cleansing"))
      return spell_holy_cleansing;

   if (!str_cmp(name, "reserved"))
      return NULL;
   if (!str_cmp(name, "spell_null"))
      return spell_null;
   return spell_notfound;
}

DO_FUN *skill_function(char *name)
{
   switch (name[3])
   {
         /*//T1*/
   case 'a':
      if ( !str_cmp( name, "do_aassign"                   ))    return do_aassign;
      if ( !str_cmp( name, "do_accept"                    ))    return do_accept;
      if ( !str_cmp( name, "do_accounts"                  ))    return do_accounts;
      if ( !str_cmp( name, "do_add_imm_host"              ))    return do_add_imm_host;
      if ( !str_cmp( name, "do_addbox"                    ))    return do_addbox;
      if ( !str_cmp( name, "do_addforge"                  ))    return do_addforge;
      if ( !str_cmp( name, "do_addgem"                    ))    return do_addgem;
      if ( !str_cmp( name, "do_addslab"                   ))    return do_addslab;
      if ( !str_cmp( name, "do_advance"                   ))    return do_advance;
      if ( !str_cmp( name, "do_advanceclock"              ))    return do_advanceclock;
      if ( !str_cmp( name, "do_affected"                  ))    return do_affected;
      if ( !str_cmp( name, "do_afk"                       ))    return do_afk;
      if ( !str_cmp( name, "do_ahall"                     ))    return do_ahall;
      if ( !str_cmp( name, "do_ahelp"                     ))    return do_ahelp;
      if ( !str_cmp( name, "do_aid"                       ))    return do_aid;
      if ( !str_cmp( name, "do_aim"                       ))    return do_aim;
      if ( !str_cmp( name, "do_alias"                     ))    return do_alias;
      if ( !str_cmp( name, "do_allow"                     ))    return do_allow;
      if ( !str_cmp( name, "do_amberio"                   ))    return do_amberio;
      if ( !str_cmp( name, "do_ansi"                      ))    return do_ansi;
      if ( !str_cmp( name, "do_answer"                    ))    return do_answer;
      if ( !str_cmp( name, "do_apply"                     ))    return do_apply;
      if ( !str_cmp( name, "do_appraise"                  ))    return do_appraise;
      if ( !str_cmp( name, "do_areas"                     ))    return do_areas;
      if ( !str_cmp( name, "do_arena"                     ))    return do_arena;
      if ( !str_cmp( name, "do_armmilitary"               ))    return do_armmilitary;
      if ( !str_cmp( name, "do_aset"                      ))    return do_aset;
      if ( !str_cmp( name, "do_ask"                       ))    return do_ask;
      if ( !str_cmp( name, "do_assassinate"               ))    return do_assassinate;
      if ( !str_cmp( name, "do_astat"                     ))    return do_astat;
      if ( !str_cmp( name, "do_at"                        ))    return do_at;
      if ( !str_cmp( name, "do_atmob"                     ))    return do_atmob;
      if ( !str_cmp( name, "do_atobj"                     ))    return do_atobj;
      if ( !str_cmp( name, "do_attack"			  ))	return do_attack;
      if ( !str_cmp( name, "do_auction"                   ))    return do_auction;
      if ( !str_cmp( name, "do_authorize"                 ))    return do_authorize;
      if ( !str_cmp( name, "do_avtalk"                    ))    return do_avtalk;
      if ( !str_cmp( name, "do_awho"                      ))    return do_awho;
      break;
   case 'b':
      if ( !str_cmp( name, "do_backstab"                  ))    return do_backstab;
      if ( !str_cmp( name, "do_balance"                   ))    return do_balance;
      if ( !str_cmp( name, "do_balzhur"                   ))    return do_balzhur;
      if ( !str_cmp( name, "do_bamfin"                    ))    return do_bamfin;
      if ( !str_cmp( name, "do_bamfout"                   ))    return do_bamfout;
      if ( !str_cmp( name, "do_ban"                       ))    return do_ban;
      if ( !str_cmp( name, "do_bash"                      ))    return do_bash;
      if ( !str_cmp( name, "do_bashdoor"                  ))    return do_bashdoor;
      if ( !str_cmp( name, "do_bcard"                     ))    return do_bcard;
      if ( !str_cmp( name, "do_beep"                      ))    return do_beep;
      if ( !str_cmp( name, "do_begging"                   ))    return do_begging;
      if ( !str_cmp( name, "do_berserk"                   ))    return do_berserk;
      if ( !str_cmp( name, "do_bestow"                    ))    return do_bestow;
      if ( !str_cmp( name, "do_bestowarea"                ))    return do_bestowarea;
      if ( !str_cmp( name, "do_bet"                       ))    return do_bet;
      if ( !str_cmp( name, "do_bhold"                     ))    return do_bhold;
      if ( !str_cmp( name, "do_bio"                       ))    return do_bio;
      if ( !str_cmp( name, "do_blackjack"                 ))    return do_blackjack;
      if ( !str_cmp( name, "do_blitz"                     ))    return do_blitz;
      if ( !str_cmp( name, "do_blockmove"                 ))    return do_blockmove;
      if ( !str_cmp( name, "do_boards"                    ))    return do_boards;
      if ( !str_cmp( name, "do_bodybag"                   ))    return do_bodybag;
      if ( !str_cmp( name, "do_brandish"                  ))    return do_brandish;
      if ( !str_cmp( name, "do_break"                     ))    return do_break;
      if ( !str_cmp( name, "do_breakwall"                 ))    return do_breakwall;
      if ( !str_cmp( name, "do_brew"                      ))    return do_brew;
      if ( !str_cmp( name, "do_bset"                      ))    return do_bset;
      if ( !str_cmp( name, "do_bstat"                     ))    return do_bstat;
      if ( !str_cmp( name, "do_bug"                       ))    return do_bug;
      if ( !str_cmp( name, "do_build"                     ))    return do_build;
      if ( !str_cmp( name, "do_buildroom"                 ))    return do_buildroom;
      if ( !str_cmp( name, "do_bury"                      ))    return do_bury;
      if ( !str_cmp( name, "do_buy"                       ))    return do_buy;
      if ( !str_cmp( name, "do_buycaste"                  ))    return do_buycaste;
      break;
   case 'c':
      if ( !str_cmp( name, "do_cache"                     ))    return do_cache;
      if ( !str_cmp( name, "do_calmmount"                 ))    return do_calmmount;
      if ( !str_cmp( name, "do_carrybin"                  ))    return do_carrybin;
      if ( !str_cmp( name, "do_cast"                      ))    return do_cast;
      if ( !str_cmp( name, "do_caste"                     ))    return do_caste;
      if ( !str_cmp( name, "do_castevalues"               ))    return do_castevalues;
      if ( !str_cmp( name, "do_cedit"                     ))    return do_cedit;
      if ( !str_cmp( name, "do_challenge"                 ))    return do_challenge;
      if ( !str_cmp( name, "do_changes"                   ))    return do_changes;
      if ( !str_cmp( name, "do_channels"                  ))    return do_channels;
      if ( !str_cmp( name, "do_chaos"                     ))    return do_chaos;
      if ( !str_cmp( name, "do_chat"                      ))    return do_chat;
      if ( !str_cmp( name, "do_check_vnums"               ))    return do_check_vnums;
      if ( !str_cmp( name, "do_chistory"                  ))    return do_chistory;
      if ( !str_cmp( name, "do_circle"                    ))    return do_circle;
      if ( !str_cmp( name, "do_cityvalues"                ))    return do_cityvalues;
      if ( !str_cmp( name, "do_clans"                     ))    return do_clans;
      if ( !str_cmp( name, "do_cleansing"                 ))    return do_cleansing;
      if ( !str_cmp( name, "do_clearques"                 ))    return do_clearques;
      if ( !str_cmp( name, "do_clearstack"                ))    return do_clearstack;
      if ( !str_cmp( name, "do_climate"                   ))    return do_climate;
      if ( !str_cmp( name, "do_climb"                     ))    return do_climb;
      if ( !str_cmp( name, "do_climbwall"                 ))    return do_climbwall;
      if ( !str_cmp( name, "do_cloak"                     ))    return do_cloak;
      if ( !str_cmp( name, "do_clones"                    ))    return do_clones;
      if ( !str_cmp( name, "do_close"                     ))    return do_close;
      if ( !str_cmp( name, "do_cmdtable"                  ))    return do_cmdtable;
      if ( !str_cmp( name, "do_color"                     ))    return do_color;
      if ( !str_cmp( name, "do_command"                   ))    return do_command;
      if ( !str_cmp( name, "do_commands"                  ))    return do_commands;
      if ( !str_cmp( name, "do_comment"                   ))    return do_comment;
      if ( !str_cmp( name, "do_compare"                   ))    return do_compare;
      if ( !str_cmp( name, "do_compress"                  ))    return do_compress;
      if ( !str_cmp( name, "do_config"                    ))    return do_config;
      if ( !str_cmp( name, "do_conquer"                   ))    return do_conquer;
      if ( !str_cmp( name, "do_consent"                   ))    return do_consent;
      if ( !str_cmp( name, "do_consider"                  ))    return do_consider;
      if ( !str_cmp( name, "do_cook"                      ))    return do_cook;
      if ( !str_cmp( name, "do_coords"                    ))    return do_coords;
      if ( !str_cmp( name, "do_copyover"                  ))    return do_copyover;
      if ( !str_cmp( name, "do_council_induct"            ))    return do_council_induct;
      if ( !str_cmp( name, "do_council_outcast"           ))    return do_council_outcast;
      if ( !str_cmp( name, "do_councils"                  ))    return do_councils;
      if ( !str_cmp( name, "do_counciltalk"               ))    return do_counciltalk;
      if ( !str_cmp( name, "do_counter"                   ))    return do_counter;
      if ( !str_cmp( name, "do_credits"                   ))    return do_credits;
      if ( !str_cmp( name, "do_cset"                      ))    return do_cset;
      if ( !str_cmp( name, "do_cutgag"                    ))    return do_cutgag;
      if ( !str_cmp( name, "do_cutpurse"                  ))    return do_cutpurse;
      break;
   case 'd':
      if ( !str_cmp( name, "do_daze"                      ))    return do_daze;
      if ( !str_cmp( name, "do_declare"                   ))    return do_declare;
      if ( !str_cmp( name, "do_decline"                   ))    return do_decline;
      if ( !str_cmp( name, "do_deities"                   ))    return do_deities;
      if ( !str_cmp( name, "do_delay"                     ))    return do_delay;
      if ( !str_cmp( name, "do_delet"                     ))    return do_delet;
      if ( !str_cmp( name, "do_delete"                    ))    return do_delete;
      if ( !str_cmp( name, "do_deny"                      ))    return do_deny;
      if ( !str_cmp( name, "do_deposit"                   ))    return do_deposit;
      if ( !str_cmp( name, "do_depository"                ))    return do_depository;
      if ( !str_cmp( name, "do_description"               ))    return do_description;
      if ( !str_cmp( name, "do_deshield"                  ))    return do_deshield;
      if ( !str_cmp( name, "do_destro"                    ))    return do_destro;
      if ( !str_cmp( name, "do_destroy"                   ))    return do_destroy;
      if ( !str_cmp( name, "do_destroyslay"               ))    return do_destroyslay;
      if ( !str_cmp( name, "do_detrap"                    ))    return do_detrap;
      if ( !str_cmp( name, "do_devote"                    ))    return do_devote;
      if ( !str_cmp( name, "do_diagnose"                  ))    return do_diagnose;
      if ( !str_cmp( name, "do_dig"                       ))    return do_dig;
      if ( !str_cmp( name, "do_disarm"                    ))    return do_disarm;
      if ( !str_cmp( name, "do_disconnect"                ))    return do_disconnect;
      if ( !str_cmp( name, "do_dislodge"                  ))    return do_dislodge;
      if ( !str_cmp( name, "do_dismiss"                   ))    return do_dismiss;
      if ( !str_cmp( name, "do_dismount"                  ))    return do_dismount;
      if ( !str_cmp( name, "do_dmesg"                     ))    return do_dmesg;
      if ( !str_cmp( name, "do_dodge"                     ))    return do_dodge;
      if ( !str_cmp( name, "do_down"                      ))    return do_down;
      if ( !str_cmp( name, "do_drag"                      ))    return do_drag;
      if ( !str_cmp( name, "do_draw"                      ))    return do_draw;
      if ( !str_cmp( name, "do_drink"                     ))    return do_drink;
      if ( !str_cmp( name, "do_drive"                     ))    return do_drive;
      if ( !str_cmp( name, "do_drop"                      ))    return do_drop;
      if ( !str_cmp( name, "do_duel"                      ))    return do_duel;
      if ( !str_cmp( name, "do_dumpgoods"                 ))    return do_dumpgoods;
      break;
   case 'e':
      if ( !str_cmp( name, "do_east"                      ))    return do_east;
      if ( !str_cmp( name, "do_eat"                       ))    return do_eat;
      if ( !str_cmp( name, "do_echo"                      ))    return do_echo;
      if ( !str_cmp( name, "do_elbowbreak"                ))    return do_elbowbreak;
      if ( !str_cmp( name, "do_elbowjab"                  ))    return do_elbowjab;
      if ( !str_cmp( name, "do_elbowstab"                 ))    return do_elbowstab;
      if ( !str_cmp( name, "do_email"                     ))    return do_email;
      if ( !str_cmp( name, "do_emeru"                     ))    return do_emeru;
      if ( !str_cmp( name, "do_emote"                     ))    return do_emote;
      if ( !str_cmp( name, "do_empty"                     ))    return do_empty;
      if ( !str_cmp( name, "do_emptycorpses"              ))    return do_emptycorpses;
      if ( !str_cmp( name, "do_enter"                     ))    return do_enter;
      if ( !str_cmp( name, "do_entership"                 ))    return do_entership;
      if ( !str_cmp( name, "do_equipment"                 ))    return do_equipment;
	  if ( !str_cmp( name, "do_enhance"					  ))    return do_enhance;
      if ( !str_cmp( name, "do_examine"                   ))    return do_examine;
      if ( !str_cmp( name, "do_exits"                     ))    return do_exits;
      if ( !str_cmp( name, "do_extract"                   ))    return do_extract;
      break;
   case 'f':
      if ( !str_cmp( name, "do_feedmount"                 ))    return do_feedmount;
      if ( !str_cmp( name, "do_fightoutput"               ))    return do_fightoutput;
      if ( !str_cmp( name, "do_fill"                      ))    return do_fill;
      if ( !str_cmp( name, "do_findnote"                  ))    return do_findnote;
      if ( !str_cmp( name, "do_finger"                    ))    return do_finger;
      if ( !str_cmp( name, "do_fire"                      ))    return do_fire;
      if ( !str_cmp( name, "do_fix"                       ))    return do_fix;
      if ( !str_cmp( name, "do_fixchar"                   ))    return do_fixchar;
      if ( !str_cmp( name, "do_fixed"                     ))    return do_fixed;
      if ( !str_cmp( name, "do_fixgemslots"               ))    return do_fixgemslots;
      if ( !str_cmp( name, "do_flee"                      ))    return do_flee;
      if ( !str_cmp( name, "do_flevel"                    ))    return do_flevel;
      if ( !str_cmp( name, "do_flipcoin"                  ))    return do_flipcoin;
      if ( !str_cmp( name, "do_foldarea"                  ))    return do_foldarea;
      if ( !str_cmp( name, "do_foldqarea"                 ))    return do_foldqarea;
      if ( !str_cmp( name, "do_follow"                    ))    return do_follow;
      if ( !str_cmp( name, "do_for"                       ))    return do_for;
      if ( !str_cmp( name, "do_forage"                    ))    return do_forage;
      if ( !str_cmp( name, "do_force"                     ))    return do_force;
      if ( !str_cmp( name, "do_forceclose"                ))    return do_forceclose;
      if ( !str_cmp( name, "do_forecast"                  ))    return do_forecast;
      if ( !str_cmp( name, "do_forge"                     ))    return do_forge;
      if ( !str_cmp( name, "do_forgealter"                ))    return do_forgealter;
      if ( !str_cmp( name, "do_forget"                    ))    return do_forget;
      if ( !str_cmp( name, "do_form_password"             ))    return do_form_password;
      if ( !str_cmp( name, "do_fprompt"                   ))    return do_fprompt;
      if ( !str_cmp( name, "do_fquit"                     ))    return do_fquit;
      if ( !str_cmp( name, "do_free_vnums"                ))    return do_free_vnums;
      if ( !str_cmp( name, "do_freerooms"                 ))    return do_freerooms;
      if ( !str_cmp( name, "do_freeze"                    ))    return do_freeze;
      break;
   case 'g':
      if ( !str_cmp( name, "do_gag"                       ))    return do_gag;
      if ( !str_cmp( name, "do_gamereset"                 ))    return do_gamereset;
      if ( !str_cmp( name, "do_gaso"                      ))    return do_gaso;
      if ( !str_cmp( name, "do_gathertinder"              ))    return do_gathertinder;
      if ( !str_cmp( name, "do_gem"                       ))    return do_gem;
      if ( !str_cmp( name, "do_generatename"              ))    return do_generatename;
      if ( !str_cmp( name, "do_get"                       ))    return do_get;
      if ( !str_cmp( name, "do_getresources"              ))    return do_getresources;
      if ( !str_cmp( name, "do_gfighting"                 ))    return do_gfighting;
      if ( !str_cmp( name, "do_give"                      ))    return do_give;
      if ( !str_cmp( name, "do_givecrown"                 ))    return do_givecrown;
      if ( !str_cmp( name, "do_giveorders"                ))    return do_giveorders;
      if ( !str_cmp( name, "do_giveup"                    ))    return do_giveup;
      if ( !str_cmp( name, "do_glance"                    ))    return do_glance;
      if ( !str_cmp( name, "do_global_boards"             ))    return do_global_boards;
      if ( !str_cmp( name, "do_global_note"               ))    return do_global_note;
      if ( !str_cmp( name, "do_goauth"                    ))    return do_goauth;
      if ( !str_cmp( name, "do_gold"                      ))    return do_gold;
      if ( !str_cmp( name, "do_goldgive"                  ))    return do_goldgive;
      if ( !str_cmp( name, "do_goldtake"                  ))    return do_goldtake;
      if ( !str_cmp( name, "do_goto"                      ))    return do_goto;
      if ( !str_cmp( name, "do_gouge"                     ))    return do_gouge;
      if ( !str_cmp( name, "do_gprompt"                   ))    return do_gprompt;
      if ( !str_cmp( name, "do_grab"                      ))    return do_grab;
      if ( !str_cmp( name, "do_grantlicense"              ))    return do_grantlicense;
      if ( !str_cmp( name, "do_grip"                      ))    return do_grip;
      if ( !str_cmp( name, "do_group"                     ))    return do_group;
      if ( !str_cmp( name, "do_grub"                      ))    return do_grub;
      if ( !str_cmp( name, "do_gscore"                    ))    return do_gscore;
      if ( !str_cmp( name, "do_gsocial"                   ))    return do_gsocial;
      if ( !str_cmp( name, "do_gtell"                     ))    return do_gtell;
      if ( !str_cmp( name, "do_guilds"                    ))    return do_guilds;
      if ( !str_cmp( name, "do_guildtalk"                 ))    return do_guildtalk;
      if ( !str_cmp( name, "do_gwhere"                    ))    return do_gwhere;
      break;
   case 'h':
      if ( !str_cmp( name, "do_heal"                      ))    return do_heal;
      if ( !str_cmp( name, "do_hedit"                     ))    return do_hedit;
      if ( !str_cmp( name, "do_hell"                      ))    return do_hell;
      if ( !str_cmp( name, "do_help"                      ))    return do_help;
      if ( !str_cmp( name, "do_helpcheck"                 ))    return do_helpcheck;
      if ( !str_cmp( name, "do_helpweb"                   ))    return do_helpweb;
      if ( !str_cmp( name, "do_hide"                      ))    return do_hide;
      if ( !str_cmp( name, "do_hindex"                    ))    return do_hindex;
      if ( !str_cmp( name, "do_hitall"                    ))    return do_hitall;
      if ( !str_cmp( name, "do_hl"                        ))    return do_hl;
      if ( !str_cmp( name, "do_hlist"                     ))    return do_hlist;
      if ( !str_cmp( name, "do_holylight"                 ))    return do_holylight;
      if ( !str_cmp( name, "do_homepage"                  ))    return do_homepage;
      if ( !str_cmp( name, "do_hset"                      ))    return do_hset;
      if ( !str_cmp( name, "do_huntportals"               ))    return do_huntportals;
      break;
   case 'i':
      if ( !str_cmp( name, "do_icq_number"                ))    return do_icq_number;
      if ( !str_cmp( name, "do_ide"                       ))    return do_ide;
      if ( !str_cmp( name, "do_idea"                      ))    return do_idea;
      if ( !str_cmp( name, "do_ignore"                    ))    return do_ignore;
      if ( !str_cmp( name, "do_imbue"			  ))	return do_imbue;
      if ( !str_cmp( name, "do_imm_morph"                 ))    return do_imm_morph;
      if ( !str_cmp( name, "do_imm_unmorph"               ))    return do_imm_unmorph;
      if ( !str_cmp( name, "do_immreminder"               ))    return do_immreminder;
      if ( !str_cmp( name, "do_immtalk"                   ))    return do_immtalk;
      if ( !str_cmp( name, "do_induct"                    ))    return do_induct;
      if ( !str_cmp( name, "do_installarea"               ))    return do_installarea;
      if ( !str_cmp( name, "do_instaroom"                 ))    return do_instaroom;
      if ( !str_cmp( name, "do_instazone"                 ))    return do_instazone;
      if ( !str_cmp( name, "do_insult"                    ))    return do_insult;
      if ( !str_cmp( name, "do_insults"                   ))    return do_insults;
      if ( !str_cmp( name, "do_introduce"                 ))    return do_introduce;
      if ( !str_cmp( name, "do_inventory"                 ))    return do_inventory;
      if ( !str_cmp( name, "do_invis"                     ))    return do_invis;
      if ( !str_cmp( name, "do_ipcompare"                 ))    return do_ipcompare;
      break;
   case 'j':
      if ( !str_cmp( name, "do_jog"                       ))    return do_jog;
      if ( !str_cmp( name, "do_joinkingdom"               ))    return do_joinkingdom;
      if ( !str_cmp( name, "do_junk"                      ))    return do_junk;
      break;
   case 'k':
      if ( !str_cmp( name, "do_keeperset"                 ))    return do_keeperset;
      if ( !str_cmp( name, "do_keeperstat"                ))    return do_keeperstat;
      if ( !str_cmp( name, "do_keys"                      ))    return do_keys;
      if ( !str_cmp( name, "do_khistory"                  ))    return do_khistory;
      if ( !str_cmp( name, "do_kick_back"                 ))    return do_kick_back;
      if ( !str_cmp( name, "do_kickdirt"                  ))    return do_kickdirt;
      if ( !str_cmp( name, "do_kickout"                   ))    return do_kickout;
      if ( !str_cmp( name, "do_kinduct"                   ))    return do_kinduct;
      if ( !str_cmp( name, "do_kingdomlog"                ))    return do_kingdomlog;
      if ( !str_cmp( name, "do_kingdomtalk"               ))    return do_kingdomtalk;
      if ( !str_cmp( name, "do_kneecrusher"               ))    return do_kneecrusher;
      if ( !str_cmp( name, "do_kneestrike"                ))    return do_kneestrike;
      if ( !str_cmp( name, "do_kremove"                   ))    return do_kremove;
      break;
   case 'l':
      if ( !str_cmp( name, "do_languages"                 ))    return do_languages;
      if ( !str_cmp( name, "do_last"                      ))    return do_last;
      if ( !str_cmp( name, "do_lastname"                  ))    return do_lastname;
      if ( !str_cmp( name, "do_laws"                      ))    return do_laws;
      if ( !str_cmp( name, "do_learn"                     ))    return do_learn;
      if ( !str_cmp( name, "do_leave"                     ))    return do_leave;
      if ( !str_cmp( name, "do_leavekingdom"              ))    return do_leavekingdom;
      if ( !str_cmp( name, "do_leaveship"                 ))    return do_leaveship;
      if ( !str_cmp( name, "do_lembecu"                   ))    return do_lembecu;
      if ( !str_cmp( name, "do_light"                     ))    return do_light;
      if ( !str_cmp( name, "do_list"                      ))    return do_list;
      if ( !str_cmp( name, "do_listgroups"                ))    return do_listgroups;
      if ( !str_cmp( name, "do_listportals"               ))    return do_listportals;
      if ( !str_cmp( name, "do_litterbug"                 ))    return do_litterbug;
      if ( !str_cmp( name, "do_loadarea"                  ))    return do_loadarea;
      if ( !str_cmp( name, "do_loadgem"                   ))    return do_loadgem;
      if ( !str_cmp( name, "do_loadquest"                 ))    return do_loadquest;
      if ( !str_cmp( name, "do_loadup"                    ))    return do_loadup;
      if ( !str_cmp( name, "do_lock"                      ))    return do_lock;
      if ( !str_cmp( name, "do_log"                       ))    return do_log;
      if ( !str_cmp( name, "do_logsettings"               ))    return do_logsettings;
      if ( !str_cmp( name, "do_look"                      ))    return do_look;
      if ( !str_cmp( name, "do_lookaround"                ))    return do_lookaround;
      if ( !str_cmp( name, "do_lookmap"                   ))    return do_lookmap;
      if ( !str_cmp( name, "do_loop"                      ))    return do_loop;
      if ( !str_cmp( name, "do_lore"                      ))    return do_lore;
      if ( !str_cmp( name, "do_low_purge"                 ))    return do_low_purge;
      break;
   case 'm':
      if ( !str_cmp( name, "do_mailroom"                  ))    return do_mailroom;
      if ( !str_cmp( name, "do_make"                      ))    return do_make;
      if ( !str_cmp( name, "do_make_wilderness_exits"     ))    return do_make_wilderness_exits;
      if ( !str_cmp( name, "do_make_wilderness_exits2"    ))    return do_make_wilderness_exits2;
      if ( !str_cmp( name, "do_makeboard"                 ))    return do_makeboard;
      if ( !str_cmp( name, "do_makeclan"                  ))    return do_makeclan;
      if ( !str_cmp( name, "do_makecouncil"               ))    return do_makecouncil;
      if ( !str_cmp( name, "do_makedeity"                 ))    return do_makedeity;
      if ( !str_cmp( name, "do_makekeeper"                ))    return do_makekeeper;
      if ( !str_cmp( name, "do_makerepair"                ))    return do_makerepair;
      if ( !str_cmp( name, "do_makeroom"                  ))    return do_makeroom;
      if ( !str_cmp( name, "do_makeshop"                  ))    return do_makeshop;
      if ( !str_cmp( name, "do_makeslay"                  ))    return do_makeslay;
      if ( !str_cmp( name, "do_makestable"                ))    return do_makestable;
      if ( !str_cmp( name, "do_makewizlist"               ))    return do_makewizlist;
      if ( !str_cmp( name, "do_makeworker"                ))    return do_makeworker;
      if ( !str_cmp( name, "do_manaburst"                 ))    return do_manaburst;
      if ( !str_cmp( name, "do_manashot"                  ))    return do_manashot;
      if ( !str_cmp( name, "do_manatap"                   ))    return do_manatap;
      if ( !str_cmp( name, "do_map"                       ))    return do_map;
      if ( !str_cmp( name, "do_mapat"                     ))    return do_mapat;
      if ( !str_cmp( name, "do_mapedit"                   ))    return do_mapedit;
      if ( !str_cmp( name, "do_mapline"                   ))    return do_mapline;
      if ( !str_cmp( name, "do_mapout"                    ))    return do_mapout;
      if ( !str_cmp( name, "do_market"                    ))    return do_market;
      if ( !str_cmp( name, "do_markportal"                ))    return do_markportal;
      if ( !str_cmp( name, "do_massgoto"                  ))    return do_massgoto;
      if ( !str_cmp( name, "do_massign"                   ))    return do_massign;
      if ( !str_cmp( name, "do_mcreate"                   ))    return do_mcreate;
      if ( !str_cmp( name, "do_mdelete"                   ))    return do_mdelete;
      if ( !str_cmp( name, "do_memory"                    ))    return do_memory;
      if ( !str_cmp( name, "do_mfind"                     ))    return do_mfind;
      if ( !str_cmp( name, "do_minfo"                     ))    return do_minfo;
      if ( !str_cmp( name, "do_minvoke"                   ))    return do_minvoke;
	  if ( !str_cmp( name, "do_mixpotion"				  ))    return do_mixpotion;
      if ( !str_cmp( name, "do_mlist"                     ))    return do_mlist;
      if ( !str_cmp( name, "do_moblog"                    ))    return do_moblog;
      if ( !str_cmp( name, "do_morgue"                    ))    return do_morgue;
      if ( !str_cmp( name, "do_morphcreate"               ))    return do_morphcreate;
      if ( !str_cmp( name, "do_morphdestroy"              ))    return do_morphdestroy;
      if ( !str_cmp( name, "do_morphset"                  ))    return do_morphset;
      if ( !str_cmp( name, "do_morphstat"                 ))    return do_morphstat;
      if ( !str_cmp( name, "do_mortalize"                 ))    return do_mortalize;
      if ( !str_cmp( name, "do_mount"                     ))    return do_mount;
      if ( !str_cmp( name, "do_movement"                  ))    return do_movement;
      if ( !str_cmp( name, "do_mp_close_passage"          ))    return do_mp_close_passage;
      if ( !str_cmp( name, "do_mp_damage"                 ))    return do_mp_damage;
      if ( !str_cmp( name, "do_mp_deposit"                ))    return do_mp_deposit;
      if ( !str_cmp( name, "do_mp_fill_in"                ))    return do_mp_fill_in;
      if ( !str_cmp( name, "do_mp_log"                    ))    return do_mp_log;
      if ( !str_cmp( name, "do_mp_open_passage"           ))    return do_mp_open_passage;
      if ( !str_cmp( name, "do_mp_practice"               ))    return do_mp_practice;
      if ( !str_cmp( name, "do_mp_restore"                ))    return do_mp_restore;
      if ( !str_cmp( name, "do_mp_slay"                   ))    return do_mp_slay;
      if ( !str_cmp( name, "do_mp_withdraw"               ))    return do_mp_withdraw;
      if ( !str_cmp( name, "do_mpadvance"                 ))    return do_mpadvance;
      if ( !str_cmp( name, "do_mpapply"                   ))    return do_mpapply;
      if ( !str_cmp( name, "do_mpapplyb"                  ))    return do_mpapplyb;
      if ( !str_cmp( name, "do_mpasound"                  ))    return do_mpasound;
      if ( !str_cmp( name, "do_mpasupress"                ))    return do_mpasupress;
      if ( !str_cmp( name, "do_mpat"                      ))    return do_mpat;
      if ( !str_cmp( name, "do_mpbodybag"                 ))    return do_mpbodybag;
      if ( !str_cmp( name, "do_mpdelay"                   ))    return do_mpdelay;
      if ( !str_cmp( name, "do_mpdream"                   ))    return do_mpdream;
      if ( !str_cmp( name, "do_mpecho"                    ))    return do_mpecho;
      if ( !str_cmp( name, "do_mpechoaround"              ))    return do_mpechoaround;
      if ( !str_cmp( name, "do_mpechoat"                  ))    return do_mpechoat;
      if ( !str_cmp( name, "do_mpechozone"                ))    return do_mpechozone;
      if ( !str_cmp( name, "do_mpedit"                    ))    return do_mpedit;
      if ( !str_cmp( name, "do_mpfavor"                   ))    return do_mpfavor;
      if ( !str_cmp( name, "do_mpforce"                   ))    return do_mpforce;
      if ( !str_cmp( name, "do_mpgive"                    ))    return do_mpgive;
      if ( !str_cmp( name, "do_mpgoto"                    ))    return do_mpgoto;
      if ( !str_cmp( name, "do_mpinvis"                   ))    return do_mpinvis;
      if ( !str_cmp( name, "do_mpjunk"                    ))    return do_mpjunk;
      if ( !str_cmp( name, "do_mpkill"                    ))    return do_mpkill;
      if ( !str_cmp( name, "do_mpmload"                   ))    return do_mpmload;
      if ( !str_cmp( name, "do_mpmorph"                   ))    return do_mpmorph;
      if ( !str_cmp( name, "do_mpmset"                    ))    return do_mpmset;
      if ( !str_cmp( name, "do_mpmusic"                   ))    return do_mpmusic;
      if ( !str_cmp( name, "do_mpmusicaround"             ))    return do_mpmusicaround;
      if ( !str_cmp( name, "do_mpmusicat"                 ))    return do_mpmusicat;
      if ( !str_cmp( name, "do_mpnothing"                 ))    return do_mpnothing;
      if ( !str_cmp( name, "do_mpoload"                   ))    return do_mpoload;
      if ( !str_cmp( name, "do_mposet"                    ))    return do_mposet;
      if ( !str_cmp( name, "do_mppardon"                  ))    return do_mppardon;
      if ( !str_cmp( name, "do_mppeace"                   ))    return do_mppeace;
      if ( !str_cmp( name, "do_mppkset"                   ))    return do_mppkset;
      if ( !str_cmp( name, "do_mppurge"                   ))    return do_mppurge;
      if ( !str_cmp( name, "do_mpscatter"                 ))    return do_mpscatter;
      if ( !str_cmp( name, "do_mpsound"                   ))    return do_mpsound;
      if ( !str_cmp( name, "do_mpsoundaround"             ))    return do_mpsoundaround;
      if ( !str_cmp( name, "do_mpsoundat"                 ))    return do_mpsoundat;
      if ( !str_cmp( name, "do_mpstat"                    ))    return do_mpstat;
      if ( !str_cmp( name, "do_mptake"                    ))    return do_mptake;
      if ( !str_cmp( name, "do_mpteach"                   ))    return do_mpteach;
      if ( !str_cmp( name, "do_mptransfer"                ))    return do_mptransfer;
      if ( !str_cmp( name, "do_mpunmorph"                 ))    return do_mpunmorph;
      if ( !str_cmp( name, "do_mpunnuisance"              ))    return do_mpunnuisance;
      if ( !str_cmp( name, "do_mpvalue"                   ))    return do_mpvalue;
      if ( !str_cmp( name, "do_mrange"                    ))    return do_mrange;
      if ( !str_cmp( name, "do_mset"                      ))    return do_mset;
      if ( !str_cmp( name, "do_mstat"                     ))    return do_mstat;
      if ( !str_cmp( name, "do_muse"                      ))    return do_muse;
      if ( !str_cmp( name, "do_music"                     ))    return do_music;
      if ( !str_cmp( name, "do_mwhere"                    ))    return do_mwhere;
      if ( !str_cmp( name, "do_mxp"                       ))    return do_mxp;
      break;
   case 'n':
      if ( !str_cmp( name, "do_name"                      ))    return do_name;
      if ( !str_cmp( name, "do_neckchop"                  ))    return do_neckchop;
      if ( !str_cmp( name, "do_neckpinch"                 ))    return do_neckpinch;
      if ( !str_cmp( name, "do_neckrupture"               ))    return do_neckrupture;
      if ( !str_cmp( name, "do_nervepinch"                ))    return do_nervepinch;
      if ( !str_cmp( name, "do_nervestrike"               ))    return do_nervestrike;
      if ( !str_cmp( name, "do_newbiechat"                ))    return do_newbiechat;
      if ( !str_cmp( name, "do_newbieset"                 ))    return do_newbieset;
      if ( !str_cmp( name, "do_news"                      ))    return do_news;
      if ( !str_cmp( name, "do_newscore"                  ))    return do_newscore;
      if ( !str_cmp( name, "do_newzones"                  ))    return do_newzones;
      if ( !str_cmp( name, "do_niburo"                    ))    return do_niburo;
      if ( !str_cmp( name, "do_nock"                      ))    return do_nock;
      if ( !str_cmp( name, "do_noemote"                   ))    return do_noemote;
      if ( !str_cmp( name, "do_noresolve"                 ))    return do_noresolve;
      if ( !str_cmp( name, "do_north"                     ))    return do_north;
      if ( !str_cmp( name, "do_northeast"                 ))    return do_northeast;
      if ( !str_cmp( name, "do_northwest"                 ))    return do_northwest;
      if ( !str_cmp( name, "do_notell"                    ))    return do_notell;
      if ( !str_cmp( name, "do_noteroom"                  ))    return do_noteroom;
      if ( !str_cmp( name, "do_notitle"                   ))    return do_notitle;
      if ( !str_cmp( name, "do_npcrace"                   ))    return do_npcrace;
      if ( !str_cmp( name, "do_nuisance"                  ))    return do_nuisance;
      if ( !str_cmp( name, "do_nuisance"                  ))    return do_nuisance;
      break;
   case 'o':
      if ( !str_cmp( name, "do_oassign"                   ))    return do_oassign;
      if ( !str_cmp( name, "do_ocreate"                   ))    return do_ocreate;
      if ( !str_cmp( name, "do_odelete"                   ))    return do_odelete;
      if ( !str_cmp( name, "do_offered"                   ))    return do_offered;
      if ( !str_cmp( name, "do_offername"                 ))    return do_offername;
      if ( !str_cmp( name, "do_offers"                    ))    return do_offers;
      if ( !str_cmp( name, "do_ofind"                     ))    return do_ofind;
      if ( !str_cmp( name, "do_ogrub"                     ))    return do_ogrub;
      if ( !str_cmp( name, "do_oinvoke"                   ))    return do_oinvoke;
      if ( !str_cmp( name, "do_oldscore"                  ))    return do_oldscore;
      if ( !str_cmp( name, "do_olist"                     ))    return do_olist;
      if ( !str_cmp( name, "do_opedit"                    ))    return do_opedit;
      if ( !str_cmp( name, "do_open"                      ))    return do_open;
   // if ( !str_cmp( name, "do_opentourney"               ))    return do_opentourney;
      if ( !str_cmp( name, "do_opstat"                    ))    return do_opstat;
      if ( !str_cmp( name, "do_orange"                    ))    return do_orange;
      if ( !str_cmp( name, "do_order"                     ))    return do_order;
      if ( !str_cmp( name, "do_orders"                    ))    return do_orders;
      if ( !str_cmp( name, "do_ordertalk"                 ))    return do_ordertalk;
      if ( !str_cmp( name, "do_oscatter"                  ))    return do_oscatter;
      if ( !str_cmp( name, "do_oset"                      ))    return do_oset;
      if ( !str_cmp( name, "do_ostat"                     ))    return do_ostat;
      if ( !str_cmp( name, "do_outcast"                   ))    return do_outcast;
      if ( !str_cmp( name, "do_owhere"                    ))    return do_owhere;
      break;
   case 'p':
      if ( !str_cmp( name, "do_pager"                     ))    return do_pager;
      if ( !str_cmp( name, "do_pardon"                    ))    return do_pardon;
      if ( !str_cmp( name, "do_parry"                     ))    return do_parry;
      if ( !str_cmp( name, "do_password"                  ))    return do_password;
      if ( !str_cmp( name, "do_pcrename"                  ))    return do_pcrename;
      if ( !str_cmp( name, "do_pcshops"                   ))    return do_pcshops;
      if ( !str_cmp( name, "do_peace"                     ))    return do_peace;
      if ( !str_cmp( name, "do_peasant"                   ))    return do_peasant;
      if ( !str_cmp( name, "do_perfectshot"               ))    return do_perfectshot;
      if ( !str_cmp( name, "do_pfiles"                    ))    return do_pfiles;
      if ( !str_cmp( name, "do_pick"                      ))    return do_pick;
      if ( !str_cmp( name, "do_piggyback"                 ))    return do_piggyback;
      if ( !str_cmp( name, "do_pincer"                    ))    return do_pincer;
      if ( !str_cmp( name, "do_pkillcheck"                ))    return do_pkillcheck;
      if ( !str_cmp( name, "do_placemob"                  ))    return do_placemob;
      if ( !str_cmp( name, "do_placeobj"                  ))    return do_placeobj;
      if ( !str_cmp( name, "do_placetrainer"              ))    return do_placetrainer;
      if ( !str_cmp( name, "do_plist"                     ))    return do_plist;
      if ( !str_cmp( name, "do_poison_weapon"             ))    return do_poison_weapon;
      if ( !str_cmp( name, "do_portal"                    ))    return do_portal;
      if ( !str_cmp( name, "do_powerslice"                ))    return do_powerslice;
      if ( !str_cmp( name, "do_pretitle"                  ))    return do_pretitle;
      if ( !str_cmp( name, "do_project"                   ))    return do_project;
      if ( !str_cmp( name, "do_prompt"                    ))    return do_prompt;
      if ( !str_cmp( name, "do_pset"                      ))    return do_pset;
      if ( !str_cmp( name, "do_pstat"                     ))    return do_pstat;
      if ( !str_cmp( name, "do_pstatus"                   ))    return do_pstatus;
      if ( !str_cmp( name, "do_pull"                      ))    return do_pull;
      if ( !str_cmp( name, "do_purge"                     ))    return do_purge;
      if ( !str_cmp( name, "do_push"                      ))    return do_push;
      if ( !str_cmp( name, "do_put"                       ))    return do_put;
      break;
   case 'q':
      if ( !str_cmp( name, "do_qmob"                      ))    return do_qmob;
      if ( !str_cmp( name, "do_qobj"                      ))    return do_qobj;
      if ( !str_cmp( name, "do_qpset"                     ))    return do_qpset;
      if ( !str_cmp( name, "do_qpstat"                    ))    return do_qpstat;
      if ( !str_cmp( name, "do_quaff"                     ))    return do_quaff;
      if ( !str_cmp( name, "do_qui"                       ))    return do_qui;
      if ( !str_cmp( name, "do_quickcombo"                ))    return do_quickcombo;
      if ( !str_cmp( name, "do_quit"                      ))    return do_quit;
      break;
   case 'r':
      if ( !str_cmp( name, "do_racetalk"                  ))    return do_racetalk;
      if ( !str_cmp( name, "do_rank"                      ))    return do_rank;
      if ( !str_cmp( name, "do_rankings"                  ))    return do_rankings;
      if ( !str_cmp( name, "do_rap"                       ))    return do_rap;
      if ( !str_cmp( name, "do_rassign"                   ))    return do_rassign;
      if ( !str_cmp( name, "do_rat"                       ))    return do_rat;
      if ( !str_cmp( name, "do_rdelete"                   ))    return do_rdelete;
      if ( !str_cmp( name, "do_reboo"                     ))    return do_reboo;
      if ( !str_cmp( name, "do_reboot"                    ))    return do_reboot;
      if ( !str_cmp( name, "do_recall"                    ))    return do_recall;
      if ( !str_cmp( name, "do_recho"                     ))    return do_recho;
      if ( !str_cmp( name, "do_recite"                    ))    return do_recite;
      if ( !str_cmp( name, "do_redit"                     ))    return do_redit;
      if ( !str_cmp( name, "do_regoto"                    ))    return do_regoto;
      if ( !str_cmp( name, "do_remains"                   ))    return do_remains;
      if ( !str_cmp( name, "do_remove"                    ))    return do_remove;
      if ( !str_cmp( name, "do_removekingdom"             ))    return do_removekingdom;
      if ( !str_cmp( name, "do_removetown"                ))    return do_removetown;
      if ( !str_cmp( name, "do_rent"                      ))    return do_rent;
      if ( !str_cmp( name, "do_repair"                    ))    return do_repair;
      if ( !str_cmp( name, "do_repairset"                 ))    return do_repairset;
      if ( !str_cmp( name, "do_repairshops"               ))    return do_repairshops;
      if ( !str_cmp( name, "do_repairstat"                ))    return do_repairstat;
      if ( !str_cmp( name, "do_repairwall"                ))    return do_repairwall;
      if ( !str_cmp( name, "do_repeat"                    ))    return do_repeat;
      if ( !str_cmp( name, "do_reply"                     ))    return do_reply;
      if ( !str_cmp( name, "do_report"                    ))    return do_report;
      if ( !str_cmp( name, "do_rescue"                    ))    return do_rescue;
      if ( !str_cmp( name, "do_reserve"                   ))    return do_reserve;
      if ( !str_cmp( name, "do_reset"                     ))    return do_reset;
      if ( !str_cmp( name, "do_resetkeeper"               ))    return do_resetkeeper;
      if ( !str_cmp( name, "do_rest"                      ))    return do_rest;
      if ( !str_cmp( name, "do_restore"                   ))    return do_restore;
      if ( !str_cmp( name, "do_restorelimbs"              ))    return do_restorelimbs;
      if ( !str_cmp( name, "do_restoretime"               ))    return do_restoretime;
      if ( !str_cmp( name, "do_restrict"                  ))    return do_restrict;
      if ( !str_cmp( name, "do_resurrection"              ))    return do_resurrection;
      if ( !str_cmp( name, "do_retell"                    ))    return do_retell;
      if ( !str_cmp( name, "do_retire"                    ))    return do_retire;
      if ( !str_cmp( name, "do_retran"                    ))    return do_retran;
      if ( !str_cmp( name, "do_return"                    ))    return do_return;
      if ( !str_cmp( name, "do_reward"                    ))    return do_reward;
      if ( !str_cmp( name, "do_rgrub"                     ))    return do_rgrub;
      if ( !str_cmp( name, "do_ribpuncture"               ))    return do_ribpuncture;
      if ( !str_cmp( name, "do_rip"                       ))    return do_rip;
      if ( !str_cmp( name, "do_rlist"                     ))    return do_rlist;
      if ( !str_cmp( name, "do_roar"                      ))    return do_roar;
      if ( !str_cmp( name, "do_roll"                      ))    return do_roll;
      if ( !str_cmp( name, "do_roomstat"                  ))    return do_roomstat;
      if ( !str_cmp( name, "do_roundhouse"                ))    return do_roundhouse;
      if ( !str_cmp( name, "do_rpedit"                    ))    return do_rpedit;
      if ( !str_cmp( name, "do_rpstat"                    ))    return do_rpstat;
      if ( !str_cmp( name, "do_rreset"                    ))    return do_rreset;
      if ( !str_cmp( name, "do_rset"                      ))    return do_rset;
      if ( !str_cmp( name, "do_rstat"                     ))    return do_rstat;
      if ( !str_cmp( name, "do_rub"                       ))    return do_rub;
      if ( !str_cmp( name, "do_run"                       ))    return do_run;
      break;
   case 's':
      if ( !str_cmp( name, "do_sacrifice"                 ))    return do_sacrifice;
      if ( !str_cmp( name, "do_save"                      ))    return do_save;
      if ( !str_cmp( name, "do_savearea"                  ))    return do_savearea;
      if ( !str_cmp( name, "do_say"                       ))    return do_say;
      if ( !str_cmp( name, "do_say_to_char"               ))    return do_say_to_char;
      if ( !str_cmp( name, "do_sbook"                     ))    return do_sbook;
      if ( !str_cmp( name, "do_scan"                      ))    return do_scan;
      if ( !str_cmp( name, "do_scatter"                   ))    return do_scatter;
      if ( !str_cmp( name, "do_schedule"                  ))    return do_schedule;
      if ( !str_cmp( name, "do_score"                     ))    return do_score;
      if ( !str_cmp( name, "do_scribe"                    ))    return do_scribe;
      if ( !str_cmp( name, "do_search"                    ))    return do_search;
      if ( !str_cmp( name, "do_sedit"                     ))    return do_sedit;
      if ( !str_cmp( name, "do_seeorders"                 ))    return do_seeorders;
      if ( !str_cmp( name, "do_sell"                      ))    return do_sell;
      if ( !str_cmp( name, "do_sendmail"                  ))    return do_sendmail;
      if ( !str_cmp( name, "do_set_boot_time"             ))    return do_set_boot_time;
      if ( !str_cmp( name, "do_setcaste"                  ))    return do_setcaste;
      if ( !str_cmp( name, "do_setclan"                   ))    return do_setclan;
      if ( !str_cmp( name, "do_setcouncil"                ))    return do_setcouncil;
      if ( !str_cmp( name, "do_setdeity"                  ))    return do_setdeity;
      if ( !str_cmp( name, "do_setfree"                   ))    return do_setfree;
      if ( !str_cmp( name, "do_setgambler"                ))    return do_setgambler;
      if ( !str_cmp( name, "do_setgem"			  ))	return do_setgem;
      if ( !str_cmp( name, "do_setjob"                    ))    return do_setjob;
      if ( !str_cmp( name, "do_setkingdom"                ))    return do_setkingdom;
      if ( !str_cmp( name, "do_setrace"                   ))    return do_setrace;
      if ( !str_cmp( name, "do_setslay"                   ))    return do_setslay;
      if ( !str_cmp( name, "do_settoadvance"              ))    return do_settoadvance;
      if ( !str_cmp( name, "do_setweather"                ))    return do_setweather;
      if ( !str_cmp( name, "do_setwilderness"             ))    return do_setwilderness;
      if ( !str_cmp( name, "do_sheath"                    ))    return do_sheath;
      if ( !str_cmp( name, "do_shells"                    ))    return do_shells;
      if ( !str_cmp( name, "do_ships"                     ))    return do_ships;
      if ( !str_cmp( name, "do_shops"                     ))    return do_shops;
      if ( !str_cmp( name, "do_shopset"                   ))    return do_shopset;
      if ( !str_cmp( name, "do_shopstat"                  ))    return do_shopstat;
      if ( !str_cmp( name, "do_shout"                     ))    return do_shout;
      if ( !str_cmp( name, "do_shove"                     ))    return do_shove;
      if ( !str_cmp( name, "do_show"                      ))    return do_show;
      if ( !str_cmp( name, "do_showascii"                 ))    return do_showascii;
      if ( !str_cmp( name, "do_showclan"                  ))    return do_showclan;
      if ( !str_cmp( name, "do_showcontrol"               ))    return do_showcontrol;
      if ( !str_cmp( name, "do_showcouncil"               ))    return do_showcouncil;
      if ( !str_cmp( name, "do_showdeity"                 ))    return do_showdeity;
      if ( !str_cmp( name, "do_showentrances"             ))    return do_showentrances;
      if ( !str_cmp( name, "do_showgambler"               ))    return do_showgambler;
      if ( !str_cmp( name, "do_showhouse"                 ))    return do_showhouse;
      if ( !str_cmp( name, "do_showkingdoms"              ))    return do_showkingdoms;
      if ( !str_cmp( name, "do_showlayers"                ))    return do_showlayers;
      if ( !str_cmp( name, "do_showlist"                  ))    return do_showlist;
      if ( !str_cmp( name, "do_showpic"                   ))    return do_showpic;
      if ( !str_cmp( name, "do_showrace"                  ))    return do_showrace;
      if ( !str_cmp( name, "do_showresources"             ))    return do_showresources;
      if ( !str_cmp( name, "do_showslay"                  ))    return do_showslay;
      if ( !str_cmp( name, "do_showweather"               ))    return do_showweather;
      if ( !str_cmp( name, "do_shutdow"                   ))    return do_shutdow;
      if ( !str_cmp( name, "do_shutdown"                  ))    return do_shutdown;
      if ( !str_cmp( name, "do_sidekick"                  ))    return do_sidekick;
      if ( !str_cmp( name, "do_silence"                   ))    return do_silence;
      if ( !str_cmp( name, "do_sing"                      ))    return do_sing;
      if ( !str_cmp( name, "do_sit"                       ))    return do_sit;
      if ( !str_cmp( name, "do_skills"                    ))    return do_skills;
      if ( !str_cmp( name, "do_skin"                      ))    return do_skin;
      if ( !str_cmp( name, "do_sla"                       ))    return do_sla;
      if ( !str_cmp( name, "do_slay"                      ))    return do_slay;
      if ( !str_cmp( name, "do_sleep"                     ))    return do_sleep;
      if ( !str_cmp( name, "do_slice"                     ))    return do_slice;
      if ( !str_cmp( name, "do_slist"                     ))    return do_slist;
      if ( !str_cmp( name, "do_slookup"                   ))    return do_slookup;
      if ( !str_cmp( name, "do_smoke"                     ))    return do_smoke;
      if ( !str_cmp( name, "do_snoop"                     ))    return do_snoop;
      if ( !str_cmp( name, "do_sober"                     ))    return do_sober;
      if ( !str_cmp( name, "do_socials"                   ))    return do_socials;
      if ( !str_cmp( name, "do_south"                     ))    return do_south;
      if ( !str_cmp( name, "do_southeast"                 ))    return do_southeast;
      if ( !str_cmp( name, "do_southwest"                 ))    return do_southwest;
      if ( !str_cmp( name, "do_spar"                      ))    return do_spar;
      if ( !str_cmp( name, "do_speak"                     ))    return do_speak;
      if ( !str_cmp( name, "do_spear"                     ))    return do_spear;
      if ( !str_cmp( name, "do_speed"                     ))    return do_speed;
      if ( !str_cmp( name, "do_spinkick"                  ))    return do_spinkick;
      if ( !str_cmp( name, "do_split"                     ))    return do_split;
      if ( !str_cmp( name, "do_sset"                      ))    return do_sset;
      if ( !str_cmp( name, "do_sslist"                    ))    return do_sslist;
      if ( !str_cmp( name, "do_stable"                    ))    return do_stable;
      if ( !str_cmp( name, "do_stack"                     ))    return do_stack;
      if ( !str_cmp( name, "do_stalk"                     ))    return do_stalk;
      if ( !str_cmp( name, "do_stand"                     ))    return do_stand;
      if ( !str_cmp( name, "do_startarena"                ))    return do_startarena;
      if ( !str_cmp( name, "do_startfire"                 ))    return do_startfire;
      if ( !str_cmp( name, "do_startkingdom"              ))    return do_startkingdom;
      if ( !str_cmp( name, "do_startroom"                 ))    return do_startroom;
      if ( !str_cmp( name, "do_stat"                      ))    return do_stat;
      if ( !str_cmp( name, "do_statreport"                ))    return do_statreport;
      if ( !str_cmp( name, "do_steal"                     ))    return do_steal;
      if ( !str_cmp( name, "do_steership"                 ))    return do_steership;
      if ( !str_cmp( name, "do_stepback"                  ))    return do_stepback;
      if ( !str_cmp( name, "do_strew"                     ))    return do_strew;
      if ( !str_cmp( name, "do_strip"                     ))    return do_strip;
      if ( !str_cmp( name, "do_study"                     ))    return do_study;
      if ( !str_cmp( name, "do_stun"                      ))    return do_stun;
      if ( !str_cmp( name, "do_style"                     ))    return do_style;
      if ( !str_cmp( name, "do_supplicate"                ))    return do_supplicate;
      if ( !str_cmp( name, "do_survey"                    ))    return do_survey;
      if ( !str_cmp( name, "do_switch"                    ))    return do_switch;
      break;
   case 't':
      if ( !str_cmp( name, "do_talent"                    ))    return do_talent;
      if ( !str_cmp( name, "do_talkquest"                 ))    return do_talkquest;
      if ( !str_cmp( name, "do_tamp"                      ))    return do_tamp;
      if ( !str_cmp( name, "do_target"                    ))    return do_target;
      if ( !str_cmp( name, "do_tcreate"                   ))    return do_tcreate;
      if ( !str_cmp( name, "do_tdelete"                   ))    return do_tdelete;
      if ( !str_cmp( name, "do_tell"                      ))    return do_tell;
      if ( !str_cmp( name, "do_terraform"                 ))    return do_terraform;
      if ( !str_cmp( name, "do_think"                     ))    return do_think;
      if ( !str_cmp( name, "do_time"                      ))    return do_time;
      if ( !str_cmp( name, "do_timecmd"                   ))    return do_timecmd;
      if ( !str_cmp( name, "do_timmuru"                   ))    return do_timmuru;
      if ( !str_cmp( name, "do_tinduct"                   ))    return do_tinduct;
      if ( !str_cmp( name, "do_title"                     ))    return do_title;
      if ( !str_cmp( name, "do_tkickout"                  ))    return do_tkickout;
      if ( !str_cmp( name, "do_tone"                      ))    return do_tone;
      if ( !str_cmp( name, "do_tornadokick"               ))    return do_tornadokick;
      if ( !str_cmp( name, "do_toss"                      ))    return do_toss;
      if ( !str_cmp( name, "do_track"                     ))    return do_track;
      if ( !str_cmp( name, "do_tradegoods"                ))    return do_tradegoods;
      if ( !str_cmp( name, "do_traderoutes"               ))    return do_traderoutes;
      if ( !str_cmp( name, "do_training"                  ))    return do_training;
      if ( !str_cmp( name, "do_trans"                     ))    return do_trans;
      if ( !str_cmp( name, "do_transfer"                  ))    return do_transfer;
      if ( !str_cmp( name, "do_trap"                      ))    return do_trap;
      if ( !str_cmp( name, "do_trust"                     ))    return do_trust;
      if ( !str_cmp( name, "do_tset"                      ))    return do_tset;
      if ( !str_cmp( name, "do_tstat"                     ))    return do_tstat;
      if ( !str_cmp( name, "do_tumble"                    ))    return do_tumble;
      if ( !str_cmp( name, "do_typo"                      ))    return do_typo;
      break;
   case 'u':
      if ( !str_cmp( name, "do_unfoldarea"                ))    return do_unfoldarea;
      if ( !str_cmp( name, "do_unhell"                    ))    return do_unhell;
      if ( !str_cmp( name, "do_unloadqarea"               ))    return do_unloadqarea;
      if ( !str_cmp( name, "do_unlock"                    ))    return do_unlock;
      if ( !str_cmp( name, "do_unnuisance"                ))    return do_unnuisance;
      if ( !str_cmp( name, "do_unsilence"                 ))    return do_unsilence;
      if ( !str_cmp( name, "do_up"                        ))    return do_up;
      if ( !str_cmp( name, "do_updatearea"                ))    return do_updatearea;
      if ( !str_cmp( name, "do_updateskills"              ))    return do_updateskills;
      if ( !str_cmp( name, "do_uselicense"                ))    return do_uselicense;
      if ( !str_cmp( name, "do_users"                     ))    return do_users;
      break;
   case 'v':
      if ( !str_cmp( name, "do_value"                     ))    return do_value;
      if ( !str_cmp( name, "do_vassign"                   ))    return do_vassign;
      if ( !str_cmp( name, "do_version"                   ))    return do_version;
      if ( !str_cmp( name, "do_victories"                 ))    return do_victories;
      if ( !str_cmp( name, "do_viewmount"                 ))    return do_viewmount;
      if ( !str_cmp( name, "do_viewskills"                ))    return do_viewskills;
      if ( !str_cmp( name, "do_visible"                   ))    return do_visible;
      if ( !str_cmp( name, "do_vnums"                     ))    return do_vnums;
      if ( !str_cmp( name, "do_vsearch"                   ))    return do_vsearch;
      break;
   case 'w':
      if ( !str_cmp( name, "do_wake"                      ))    return do_wake;
      if ( !str_cmp( name, "do_warn"                      ))    return do_warn;
      if ( !str_cmp( name, "do_wartalk"                   ))    return do_wartalk;
      if ( !str_cmp( name, "do_watch"                     ))    return do_watch;
      if ( !str_cmp( name, "do_wblock"                    ))    return do_wblock;
      if ( !str_cmp( name, "do_weaponbreak"               ))    return do_weaponbreak;
      if ( !str_cmp( name, "do_wear"                      ))    return do_wear;
      if ( !str_cmp( name, "do_weather"                   ))    return do_weather;
      if ( !str_cmp( name, "do_webstats"                  ))    return do_webstats;
      if ( !str_cmp( name, "do_west"                      ))    return do_west;
      if ( !str_cmp( name, "do_where"                     ))    return do_where;
      if ( !str_cmp( name, "do_whisper"                   ))    return do_whisper;
      if ( !str_cmp( name, "do_who"                       ))    return do_who;
      if ( !str_cmp( name, "do_whois"                     ))    return do_whois;
      if ( !str_cmp( name, "do_whonumber"                 ))    return do_whonumber;
      if ( !str_cmp( name, "do_wimpy"                     ))    return do_wimpy;
      if ( !str_cmp( name, "do_withdraw"                  ))    return do_withdraw;
      if ( !str_cmp( name, "do_wizhelp"                   ))    return do_wizhelp;
      if ( !str_cmp( name, "do_wizlist"                   ))    return do_wizlist;
      if ( !str_cmp( name, "do_wizlock"                   ))    return do_wizlock;
      if ( !str_cmp( name, "do_wizzap"                    ))    return do_wizzap;
      if ( !str_cmp( name, "do_worth"                     ))    return do_worth;
      break;
   case 'y':
      if ( !str_cmp( name, "do_yell"                      ))    return do_yell;
      break;
   case 'z':
      if ( !str_cmp( name, "do_zap"                       ))    return do_zap;
      if ( !str_cmp( name, "do_zones"                     ))    return do_zones;
      break;
         /*//T2*/
   }
   return skill_notfound;
}

char *spell_name(SPELL_FUN * spell)
{
   if (spell == spell_smaug)
      return "spell_smaug";
   if (spell == spell_eye_of_god)
      return "spell_eye_of_god";
   if (spell == spell_resurrection)
      return "spell_resurrection";
   if (spell == spell_summon_corpse)
      return "spell_summon_corpse";
   if (spell == spell_greater_resurrection)
      return "spell_greater_resurrection";
   if (spell == spell_lesser_resurrection)
      return "spell_lesser_resurrection";
   if (spell == spell_web)
      return "spell_web";
   if (spell == spell_revitalize_spirit)
      return "spell_revitalize_spirit";
   if (spell == spell_snare)
      return "spell_snare";
   if (spell == spell_extradimensional_portal)
      return "spell_extradimensional_portal";
   if (spell == spell_animate_dead)
      return "spell_animate_dead";
   if (spell == spell_astral_walk)
      return "spell_astral_walk";
   if (spell == spell_blindness)
      return "spell_blindness";
   if (spell == spell_cause_critical)
      return "spell_cause_critical";
   if (spell == spell_restore_limb)
      return "spell_restore_limb";
   if (spell == spell_revitalize_limb)
      return "spell_revitalize_limb";
   if (spell == spell_cause_light)
      return "spell_cause_light";
   if (spell == spell_cause_serious)
      return "spell_cause_serious";
   if (spell == spell_charm_person)
      return "spell_charm_person";
   if (spell == spell_create_food)
      return "spell_create_food";
   if (spell == spell_create_water)
      return "spell_create_water";
   if (spell == spell_bless_weapon)
      return "spell_bless_weapon";
   if (spell == spell_cure_blindness)
      return "spell_cure_blindness";
   if (spell == spell_cure_poison)
      return "spell_cure_poison";
   if (spell == spell_curse)
      return "spell_curse";
   if (spell == spell_detect_poison)
      return "spell_detect_poison";
   if (spell == spell_disenchant_weapon)
      return "spell_disenchant_weapon";
   if (spell == spell_dispel_magic)
      return "spell_dispel_magic";
   if (spell == spell_earthquake)
      return "spell_earthquake";
   if (spell == spell_enchant_weapon)
      return "spell_enchant_weapon";
   if (spell == spell_faerie_fire)
      return "spell_faerie_fire";
   if (spell == spell_faerie_fog)
      return "spell_faerie_fog";
   if (spell == spell_farsight)
      return "spell_farsight";
   if (spell == spell_knock)
      return "spell_knock";
   if (spell == spell_harm)
      return "spell_harm";
   if (spell == spell_holy_food)
      return "spell_holy_food";
   if (spell == spell_identify)
      return "spell_identify";
   if (spell == spell_invis)
      return "spell_invis";
   if (spell == spell_knowenemy)
      return "spell_knowenemy";
   if (spell == spell_locate_object)
      return "spell_locate_object";
   if (spell == spell_pass_door)
      return "spell_pass_door";
   if (spell == spell_poison)
      return "spell_poison";
   if (spell == spell_polymorph)
      return "spell_polymorph";
   if (spell == spell_possess)
      return "spell_possess";
   if (spell == spell_recharge)
      return "spell_recharge";
   if (spell == spell_remove_curse)
      return "spell_remove_curse";
   if (spell == spell_remove_invis)
      return "spell_remove_invis";
   if (spell == spell_remove_trap)
      return "spell_remove_trap";
   if (spell == spell_sleep)
      return "spell_sleep";
   if (spell == spell_summon)
      return "spell_summon";
   if (spell == spell_weaken)
      return "spell_weaken";
   if (spell == spell_word_of_recall)
      return "spell_word_of_recall";
   if (spell == spell_acid_breath)
      return "spell_acid_breath";
   if (spell == spell_fire_breath)
      return "spell_fire_breath";
   if (spell == spell_frost_breath)
      return "spell_frost_breath";
   if (spell == spell_gas_breath)
      return "spell_gas_breath";
   if (spell == spell_lightning_breath)
      return "spell_lightning_breath";
   if (spell == spell_bethsaidean_touch)
      return "spell_bethsaidean_touch";
   if (spell == spell_expurgation)
      return "spell_expurgation";
   if (spell == spell_sacral_divinity)
      return "spell_sacral_divinity";
   if (spell == spell_wizard_eye)
      return "spell_wizard_eye";
   if (spell == spell_holy_cleansing)
      return "spell_holy_cleansing";

   if (spell == spell_null)
      return "spell_null";
   return "reserved";
}

char *skill_name(DO_FUN * skill)
{
   static char buf[64];

   if (skill == NULL)
      return "reserved";
   /*//T3*/
      if ( skill == do_aassign                     )    return "do_aassign";
      if ( skill == do_accept                      )    return "do_accept";
      if ( skill == do_accounts                    )    return "do_accounts";
      if ( skill == do_add_imm_host                )    return "do_add_imm_host";
      if ( skill == do_addbox                      )    return "do_addbox";
      if ( skill == do_addforge                    )    return "do_addforge";
      if ( skill == do_addgem                      )    return "do_addgem";
      if ( skill == do_addslab                     )    return "do_addslab";
      if ( skill == do_advance                     )    return "do_advance";
      if ( skill == do_advanceclock                )    return "do_advanceclock";
      if ( skill == do_affected                    )    return "do_affected";
      if ( skill == do_afk                         )    return "do_afk";
      if ( skill == do_ahall                       )    return "do_ahall";
      if ( skill == do_ahelp                       )    return "do_ahelp";
      if ( skill == do_aid                         )    return "do_aid";
      if ( skill == do_aim                         )    return "do_aim";
      if ( skill == do_alias                       )    return "do_alias";
      if ( skill == do_allow                       )    return "do_allow";
      if ( skill == do_amberio                     )    return "do_amberio";
      if ( skill == do_ansi                        )    return "do_ansi";
      if ( skill == do_answer                      )    return "do_answer";
      if ( skill == do_apply                       )    return "do_apply";
      if ( skill == do_appraise                    )    return "do_appraise";
      if ( skill == do_areas                       )    return "do_areas";
      if ( skill == do_arena                       )    return "do_arena";
      if ( skill == do_armmilitary                 )    return "do_armmilitary";
      if ( skill == do_aset                        )    return "do_aset";
      if ( skill == do_ask                         )    return "do_ask";
      if ( skill == do_assassinate                 )    return "do_assassinate";
      if ( skill == do_astat                       )    return "do_astat";
      if ( skill == do_at                          )    return "do_at";
      if ( skill == do_atmob                       )    return "do_atmob";
      if ( skill == do_atobj                       )    return "do_atobj";
      if ( skill == do_attack			   )    return "do_attack";
      if ( skill == do_auction                     )    return "do_auction";
      if ( skill == do_authorize                   )    return "do_authorize";
      if ( skill == do_avtalk                      )    return "do_avtalk";
      if ( skill == do_awho                        )    return "do_awho";
      if ( skill == do_backstab                    )    return "do_backstab";
      if ( skill == do_balance                     )    return "do_balance";
      if ( skill == do_balzhur                     )    return "do_balzhur";
      if ( skill == do_bamfin                      )    return "do_bamfin";
      if ( skill == do_bamfout                     )    return "do_bamfout";
      if ( skill == do_ban                         )    return "do_ban";
      if ( skill == do_bash                        )    return "do_bash";
      if ( skill == do_bashdoor                    )    return "do_bashdoor";
      if ( skill == do_bcard                       )    return "do_bcard";
      if ( skill == do_beep                        )    return "do_beep";
      if ( skill == do_begging                     )    return "do_begging";
      if ( skill == do_berserk                     )    return "do_berserk";
      if ( skill == do_bestow                      )    return "do_bestow";
      if ( skill == do_bestowarea                  )    return "do_bestowarea";
      if ( skill == do_bet                         )    return "do_bet";
      if ( skill == do_bhold                       )    return "do_bhold";
      if ( skill == do_bio                         )    return "do_bio";
      if ( skill == do_blackjack                   )    return "do_blackjack";
      if ( skill == do_blitz                       )    return "do_blitz";
      if ( skill == do_blockmove                   )    return "do_blockmove";
      if ( skill == do_boards                      )    return "do_boards";
      if ( skill == do_bodybag                     )    return "do_bodybag";
      if ( skill == do_brandish                    )    return "do_brandish";
      if ( skill == do_break                       )    return "do_break";
      if ( skill == do_breakwall                   )    return "do_breakwall";
      if ( skill == do_brew                        )    return "do_brew";
      if ( skill == do_bset                        )    return "do_bset";
      if ( skill == do_bstat                       )    return "do_bstat";
      if ( skill == do_bug                         )    return "do_bug";
      if ( skill == do_build                       )    return "do_build";
      if ( skill == do_buildroom                   )    return "do_buildroom";
      if ( skill == do_bury                        )    return "do_bury";
      if ( skill == do_buy                         )    return "do_buy";
      if ( skill == do_buycaste                    )    return "do_buycaste";
      if ( skill == do_cache                       )    return "do_cache";
      if ( skill == do_calmmount                   )    return "do_calmmount";
      if ( skill == do_carrybin                    )    return "do_carrybin";
      if ( skill == do_cast                        )    return "do_cast";
      if ( skill == do_caste                       )    return "do_caste";
      if ( skill == do_castevalues                 )    return "do_castevalues";
      if ( skill == do_cedit                       )    return "do_cedit";
      if ( skill == do_challenge                   )    return "do_challenge";
      if ( skill == do_changes                     )    return "do_changes";
      if ( skill == do_channels                    )    return "do_channels";
      if ( skill == do_chaos                       )    return "do_chaos";
      if ( skill == do_chat                        )    return "do_chat";
      if ( skill == do_check_vnums                 )    return "do_check_vnums";
      if ( skill == do_chistory                    )    return "do_chistory";
      if ( skill == do_circle                      )    return "do_circle";
      if ( skill == do_cityvalues                  )    return "do_cityvalues";
      if ( skill == do_clans                       )    return "do_clans";
      if ( skill == do_clantalk                    )    return "do_clantalk";
      if ( skill == do_cleansing                   )    return "do_cleansing";
      if ( skill == do_clearques                   )    return "do_clearques";
      if ( skill == do_clearstack                  )    return "do_clearstack";
      if ( skill == do_climate                     )    return "do_climate";
      if ( skill == do_climb                       )    return "do_climb";
      if ( skill == do_climbwall                   )    return "do_climbwall";
      if ( skill == do_cloak                       )    return "do_cloak";
      if ( skill == do_clones                      )    return "do_clones";
      if ( skill == do_close                       )    return "do_close";
      if ( skill == do_cmdtable                    )    return "do_cmdtable";
      if ( skill == do_color                       )    return "do_color";
      if ( skill == do_command                     )    return "do_command";
      if ( skill == do_commands                    )    return "do_commands";
      if ( skill == do_comment                     )    return "do_comment";
      if ( skill == do_compare                     )    return "do_compare";
      if ( skill == do_compress                    )    return "do_compress";
      if ( skill == do_config                      )    return "do_config";
      if ( skill == do_conquer                     )    return "do_conquer";
      if ( skill == do_consent                     )    return "do_consent";
      if ( skill == do_consider                    )    return "do_consider";
      if ( skill == do_cook                        )    return "do_cook";
      if ( skill == do_coords                      )    return "do_coords";
      if ( skill == do_copyover                    )    return "do_copyover";
      if ( skill == do_council_induct              )    return "do_council_induct";
      if ( skill == do_council_outcast             )    return "do_council_outcast";
      if ( skill == do_councils                    )    return "do_councils";
      if ( skill == do_counciltalk                 )    return "do_counciltalk";
      if ( skill == do_counter                     )    return "do_counter";
      if ( skill == do_credits                     )    return "do_credits";
      if ( skill == do_cset                        )    return "do_cset";
      if ( skill == do_cutgag                      )    return "do_cutgag";
      if ( skill == do_cutpurse                    )    return "do_cutpurse";
      if ( skill == do_daze                        )    return "do_daze";
      if ( skill == do_declare                     )    return "do_declare";
      if ( skill == do_decline                     )    return "do_decline";
      if ( skill == do_deities                     )    return "do_deities";
      if ( skill == do_delay                       )    return "do_delay";
      if ( skill == do_delet                       )    return "do_delet";
      if ( skill == do_delete                      )    return "do_delete";
      if ( skill == do_deny                        )    return "do_deny";
      if ( skill == do_deposit                     )    return "do_deposit";
      if ( skill == do_depository                  )    return "do_depository";
      if ( skill == do_description                 )    return "do_description";
      if ( skill == do_deshield                    )    return "do_deshield";
      if ( skill == do_destro                      )    return "do_destro";
      if ( skill == do_destroy                     )    return "do_destroy";
      if ( skill == do_destroyslay                 )    return "do_destroyslay";
      if ( skill == do_detrap                      )    return "do_detrap";
      if ( skill == do_devote                      )    return "do_devote";
      if ( skill == do_diagnose                    )    return "do_diagnose";
      if ( skill == do_dig                         )    return "do_dig";
      if ( skill == do_disarm                      )    return "do_disarm";
      if ( skill == do_disconnect                  )    return "do_disconnect";
      if ( skill == do_dislodge                    )    return "do_dislodge";
      if ( skill == do_dismiss                     )    return "do_dismiss";
      if ( skill == do_dismount                    )    return "do_dismount";
      if ( skill == do_dmesg                       )    return "do_dmesg";
      if ( skill == do_dodge                       )    return "do_dodge";
      if ( skill == do_down                        )    return "do_down";
      if ( skill == do_drag                        )    return "do_drag";
      if ( skill == do_draw                        )    return "do_draw";
      if ( skill == do_drink                       )    return "do_drink";
      if ( skill == do_drive                       )    return "do_drive";
      if ( skill == do_drop                        )    return "do_drop";
      if ( skill == do_duel                        )    return "do_duel";
      if ( skill == do_dumpgoods                   )    return "do_dumpgoods";
      if ( skill == do_east                        )    return "do_east";
      if ( skill == do_eat                         )    return "do_eat";
      if ( skill == do_echo                        )    return "do_echo";
      if ( skill == do_elbowbreak                  )    return "do_elbowbreak";
      if ( skill == do_elbowjab                    )    return "do_elbowjab";
      if ( skill == do_elbowstab                   )    return "do_elbowstab";
      if ( skill == do_email                       )    return "do_email";
      if ( skill == do_emeru                       )    return "do_emeru";
      if ( skill == do_emote                       )    return "do_emote";
      if ( skill == do_empty                       )    return "do_empty";
      if ( skill == do_emptycorpses                )    return "do_emptycorpses";
      if ( skill == do_enter                       )    return "do_enter";
	  if ( skill == do_enhance					   )    return "do_enhance";
      if ( skill == do_entership                   )    return "do_entership";
      if ( skill == do_equipment                   )    return "do_equipment";
      if ( skill == do_examine                     )    return "do_examine";
      if ( skill == do_exits                       )    return "do_exits";
      if ( skill == do_extract                     )    return "do_extract";
      if ( skill == do_feedmount                   )    return "do_feedmount";
      if ( skill == do_fightoutput                 )    return "do_fightoutput";
      if ( skill == do_fill                        )    return "do_fill";
      if ( skill == do_findnote                    )    return "do_findnote";
      if ( skill == do_finger                      )    return "do_finger";
      if ( skill == do_fire                        )    return "do_fire";
      if ( skill == do_fix                         )    return "do_fix";
      if ( skill == do_fixchar                     )    return "do_fixchar";
      if ( skill == do_fixed                       )    return "do_fixed";
      if ( skill == do_fixgemslots                 )    return "do_fixgemslots";
      if ( skill == do_flee                        )    return "do_flee";
      if ( skill == do_flevel                      )    return "do_flevel";
      if ( skill == do_flipcoin                    )    return "do_flipcoin";
      if ( skill == do_foldarea                    )    return "do_foldarea";
      if ( skill == do_foldqarea                   )    return "do_foldqarea";
      if ( skill == do_follow                      )    return "do_follow";
      if ( skill == do_for                         )    return "do_for";
      if ( skill == do_forage                      )    return "do_forage";
      if ( skill == do_force                       )    return "do_force";
      if ( skill == do_forceclose                  )    return "do_forceclose";
      if ( skill == do_forecast                    )    return "do_forecast";
      if ( skill == do_forge                       )    return "do_forge";
      if ( skill == do_forgealter                  )    return "do_forgealter";
      if ( skill == do_forget                      )    return "do_forget";
      if ( skill == do_form_password               )    return "do_form_password";
      if ( skill == do_fprompt                     )    return "do_fprompt";
      if ( skill == do_fquit                       )    return "do_fquit";
      if ( skill == do_free_vnums                  )    return "do_free_vnums";
      if ( skill == do_freerooms                   )    return "do_freerooms";
      if ( skill == do_freeze                      )    return "do_freeze";
      if ( skill == do_gag                         )    return "do_gag";
      if ( skill == do_gamereset                   )    return "do_gamereset";
      if ( skill == do_gaso                        )    return "do_gaso";
      if ( skill == do_gathertinder                )    return "do_gathertinder";
      if ( skill == do_gem                         )    return "do_gem";
      if ( skill == do_generatename                )    return "do_generatename";
      if ( skill == do_get                         )    return "do_get";
      if ( skill == do_getresources                )    return "do_getresources";
      if ( skill == do_gfighting                   )    return "do_gfighting";
      if ( skill == do_give                        )    return "do_give";
      if ( skill == do_givecrown                   )    return "do_givecrown";
      if ( skill == do_giveorders                  )    return "do_giveorders";
      if ( skill == do_giveup                      )    return "do_giveup";
      if ( skill == do_glance                      )    return "do_glance";
      if ( skill == do_global_boards               )    return "do_global_boards";
      if ( skill == do_global_note                 )    return "do_global_note";
      if ( skill == do_goauth                      )    return "do_goauth";
      if ( skill == do_gold                        )    return "do_gold";
      if ( skill == do_goldgive                    )    return "do_goldgive";
      if ( skill == do_goldtake                    )    return "do_goldtake";
      if ( skill == do_goto                        )    return "do_goto";
      if ( skill == do_gouge                       )    return "do_gouge";
      if ( skill == do_gprompt                     )    return "do_gprompt";
      if ( skill == do_grab                        )    return "do_grab";
      if ( skill == do_grantlicense                )    return "do_grantlicense";
      if ( skill == do_grip                        )    return "do_grip";
      if ( skill == do_group                       )    return "do_group";
      if ( skill == do_grub                        )    return "do_grub";
      if ( skill == do_gscore                      )    return "do_gscore";
      if ( skill == do_gsocial                     )    return "do_gsocial";
      if ( skill == do_gtell                       )    return "do_gtell";
      if ( skill == do_guilds                      )    return "do_guilds";
      if ( skill == do_guildtalk                   )    return "do_guildtalk";
      if ( skill == do_gwhere                      )    return "do_gwhere";
      if ( skill == do_heal                        )    return "do_heal";
      if ( skill == do_hedit                       )    return "do_hedit";
      if ( skill == do_hell                        )    return "do_hell";
      if ( skill == do_help                        )    return "do_help";
      if ( skill == do_helpcheck                   )    return "do_helpcheck";
      if ( skill == do_helpweb                     )    return "do_helpweb";
      if ( skill == do_hide                        )    return "do_hide";
      if ( skill == do_hindex                      )    return "do_hindex";
      if ( skill == do_hitall                      )    return "do_hitall";
      if ( skill == do_hl                          )    return "do_hl";
      if ( skill == do_hlist                       )    return "do_hlist";
      if ( skill == do_holylight                   )    return "do_holylight";
      if ( skill == do_homepage                    )    return "do_homepage";
      if ( skill == do_hset                        )    return "do_hset";
      if ( skill == do_huntportals                 )    return "do_huntportals";
      if ( skill == do_icq_number                  )    return "do_icq_number";
      if ( skill == do_ide                         )    return "do_ide";
      if ( skill == do_idea                        )    return "do_idea";
      if ( skill == do_ignore                      )    return "do_ignore";
      if ( skill == do_imbue			   )	return "do_imbue";
      if ( skill == do_imm_morph                   )    return "do_imm_morph";
      if ( skill == do_imm_unmorph                 )    return "do_imm_unmorph";
      if ( skill == do_immreminder                 )    return "do_immreminder";
      if ( skill == do_immtalk                     )    return "do_immtalk";
      if ( skill == do_induct                      )    return "do_induct";
      if ( skill == do_installarea                 )    return "do_installarea";
      if ( skill == do_instaroom                   )    return "do_instaroom";
      if ( skill == do_instazone                   )    return "do_instazone";
      if ( skill == do_insult                      )    return "do_insult";
      if ( skill == do_insults                     )    return "do_insults";
      if ( skill == do_introduce                   )    return "do_introduce";
      if ( skill == do_inventory                   )    return "do_inventory";
      if ( skill == do_invis                       )    return "do_invis";
      if ( skill == do_ipcompare                   )    return "do_ipcompare";
      if ( skill == do_jog                         )    return "do_jog";
      if ( skill == do_joinkingdom                 )    return "do_joinkingdom";
      if ( skill == do_junk                        )    return "do_junk";
      if ( skill == do_keeperset                   )    return "do_keeperset";
      if ( skill == do_keeperstat                  )    return "do_keeperstat";
      if ( skill == do_keys                        )    return "do_keys";
      if ( skill == do_khistory                    )    return "do_khistory";
      if ( skill == do_kick_back                   )    return "do_kick_back";
      if ( skill == do_kickdirt                    )    return "do_kickdirt";
      if ( skill == do_kickout                     )    return "do_kickout";
      if ( skill == do_kinduct                     )    return "do_kinduct";
      if ( skill == do_kingdomlog                  )    return "do_kingdomlog";
      if ( skill == do_kingdomtalk                 )    return "do_kingdomtalk";
      if ( skill == do_kneecrusher                 )    return "do_kneecrusher";
      if ( skill == do_kneestrike                  )    return "do_kneestrike";
      if ( skill == do_kremove                     )    return "do_kremove";
      if ( skill == do_languages                   )    return "do_languages";
      if ( skill == do_last                        )    return "do_last";
      if ( skill == do_lastname                    )    return "do_lastname";
      if ( skill == do_laws                        )    return "do_laws";
      if ( skill == do_learn                       )    return "do_learn";
      if ( skill == do_leave                       )    return "do_leave";
      if ( skill == do_leavekingdom                )    return "do_leavekingdom";
      if ( skill == do_leaveship                   )    return "do_leaveship";
      if ( skill == do_lembecu                     )    return "do_lembecu";
      if ( skill == do_light                       )    return "do_light";
      if ( skill == do_list                        )    return "do_list";
      if ( skill == do_listgroups                  )    return "do_listgroups";
      if ( skill == do_listportals                 )    return "do_listportals";
      if ( skill == do_litterbug                   )    return "do_litterbug";
      if ( skill == do_loadarea                    )    return "do_loadarea";
      if ( skill == do_loadgem                     )    return "do_loadgem";
      if ( skill == do_loadquest                   )    return "do_loadquest";
      if ( skill == do_loadup                      )    return "do_loadup";
      if ( skill == do_lock                        )    return "do_lock";
      if ( skill == do_log                         )    return "do_log";
      if ( skill == do_logsettings                 )    return "do_logsettings";
      if ( skill == do_look                        )    return "do_look";
      if ( skill == do_lookaround                  )    return "do_lookaround";
      if ( skill == do_lookmap                     )    return "do_lookmap";
      if ( skill == do_loop                        )    return "do_loop";
      if ( skill == do_lore                        )    return "do_lore";
      if ( skill == do_low_purge                   )    return "do_low_purge";
      if ( skill == do_mailroom                    )    return "do_mailroom";
      if ( skill == do_make                        )    return "do_make";
      if ( skill == do_make_wilderness_exits       )    return "do_make_wilderness_exits";
      if ( skill == do_make_wilderness_exits2      )    return "do_make_wilderness_exits2";
      if ( skill == do_makeboard                   )    return "do_makeboard";
      if ( skill == do_makeclan                    )    return "do_makeclan";
      if ( skill == do_makecouncil                 )    return "do_makecouncil";
      if ( skill == do_makedeity                   )    return "do_makedeity";
      if ( skill == do_makekeeper                  )    return "do_makekeeper";
      if ( skill == do_makerepair                  )    return "do_makerepair";
      if ( skill == do_makeroom                    )    return "do_makeroom";
      if ( skill == do_makeshop                    )    return "do_makeshop";
      if ( skill == do_makeslay                    )    return "do_makeslay";
      if ( skill == do_makestable                  )    return "do_makestable";
      if ( skill == do_makewizlist                 )    return "do_makewizlist";
      if ( skill == do_makeworker                  )    return "do_makeworker";
      if ( skill == do_manaburst                   )    return "do_manaburst";
      if ( skill == do_manashot                    )    return "do_manashot";
      if ( skill == do_manatap                     )    return "do_manatap";
      if ( skill == do_map                         )    return "do_map";
      if ( skill == do_mapat                       )    return "do_mapat";
      if ( skill == do_mapedit                     )    return "do_mapedit";
      if ( skill == do_mapline                     )    return "do_mapline";
      if ( skill == do_mapout                      )    return "do_mapout";
      if ( skill == do_market                      )    return "do_market";
      if ( skill == do_markportal                  )    return "do_markportal";
      if ( skill == do_massgoto                    )    return "do_massgoto";
      if ( skill == do_massign                     )    return "do_massign";
      if ( skill == do_mcreate                     )    return "do_mcreate";
      if ( skill == do_mdelete                     )    return "do_mdelete";
      if ( skill == do_memory                      )    return "do_memory";
      if ( skill == do_mfind                       )    return "do_mfind";
      if ( skill == do_minfo                       )    return "do_minfo";
      if ( skill == do_minvoke                     )    return "do_minvoke";
	  if ( skill == do_mixpotion				   )    return "do_mixpotion";
      if ( skill == do_mlist                       )    return "do_mlist";
      if ( skill == do_moblog                      )    return "do_moblog";
      if ( skill == do_morgue                      )    return "do_morgue";
      if ( skill == do_morphcreate                 )    return "do_morphcreate";
      if ( skill == do_morphdestroy                )    return "do_morphdestroy";
      if ( skill == do_morphset                    )    return "do_morphset";
      if ( skill == do_morphstat                   )    return "do_morphstat";
      if ( skill == do_mortalize                   )    return "do_mortalize";
      if ( skill == do_mount                       )    return "do_mount";
      if ( skill == do_movement                    )    return "do_movement";
      if ( skill == do_mp_close_passage            )    return "do_mp_close_passage";
      if ( skill == do_mp_damage                   )    return "do_mp_damage";
      if ( skill == do_mp_deposit                  )    return "do_mp_deposit";
      if ( skill == do_mp_fill_in                  )    return "do_mp_fill_in";
      if ( skill == do_mp_log                      )    return "do_mp_log";
      if ( skill == do_mp_open_passage             )    return "do_mp_open_passage";
      if ( skill == do_mp_practice                 )    return "do_mp_practice";
      if ( skill == do_mp_restore                  )    return "do_mp_restore";
      if ( skill == do_mp_slay                     )    return "do_mp_slay";
      if ( skill == do_mp_withdraw                 )    return "do_mp_withdraw";
      if ( skill == do_mpadvance                   )    return "do_mpadvance";
      if ( skill == do_mpapply                     )    return "do_mpapply";
      if ( skill == do_mpapplyb                    )    return "do_mpapplyb";
      if ( skill == do_mpasound                    )    return "do_mpasound";
      if ( skill == do_mpasupress                  )    return "do_mpasupress";
      if ( skill == do_mpat                        )    return "do_mpat";
      if ( skill == do_mpbodybag                   )    return "do_mpbodybag";
      if ( skill == do_mpdelay                     )    return "do_mpdelay";
      if ( skill == do_mpdream                     )    return "do_mpdream";
      if ( skill == do_mpecho                      )    return "do_mpecho";
      if ( skill == do_mpechoaround                )    return "do_mpechoaround";
      if ( skill == do_mpechoat                    )    return "do_mpechoat";
      if ( skill == do_mpechozone                  )    return "do_mpechozone";
      if ( skill == do_mpedit                      )    return "do_mpedit";
      if ( skill == do_mpfavor                     )    return "do_mpfavor";
      if ( skill == do_mpforce                     )    return "do_mpforce";
      if ( skill == do_mpgive                      )    return "do_mpgive";
      if ( skill == do_mpgoto                      )    return "do_mpgoto";
      if ( skill == do_mpinvis                     )    return "do_mpinvis";
      if ( skill == do_mpjunk                      )    return "do_mpjunk";
      if ( skill == do_mpkill                      )    return "do_mpkill";
      if ( skill == do_mpmload                     )    return "do_mpmload";
      if ( skill == do_mpmorph                     )    return "do_mpmorph";
      if ( skill == do_mpmset                      )    return "do_mpmset";
      if ( skill == do_mpmusic                     )    return "do_mpmusic";
      if ( skill == do_mpmusicaround               )    return "do_mpmusicaround";
      if ( skill == do_mpmusicat                   )    return "do_mpmusicat";
      if ( skill == do_mpnothing                   )    return "do_mpnothing";
      if ( skill == do_mpnuisance                  )    return "do_mpnuisance";
      if ( skill == do_mpoload                     )    return "do_mpoload";
      if ( skill == do_mposet                      )    return "do_mposet";
      if ( skill == do_mppardon                    )    return "do_mppardon";
      if ( skill == do_mppeace                     )    return "do_mppeace";
      if ( skill == do_mppkset                     )    return "do_mppkset";
      if ( skill == do_mppurge                     )    return "do_mppurge";
      if ( skill == do_mpscatter                   )    return "do_mpscatter";
      if ( skill == do_mpsound                     )    return "do_mpsound";
      if ( skill == do_mpsoundaround               )    return "do_mpsoundaround";
      if ( skill == do_mpsoundat                   )    return "do_mpsoundat";
      if ( skill == do_mpstat                      )    return "do_mpstat";
      if ( skill == do_mptake                      )    return "do_mptake";
      if ( skill == do_mpteach                     )    return "do_mpteach";
      if ( skill == do_mptransfer                  )    return "do_mptransfer";
      if ( skill == do_mpunmorph                   )    return "do_mpunmorph";
      if ( skill == do_mpunnuisance                )    return "do_mpunnuisance";
      if ( skill == do_mpvalue                     )    return "do_mpvalue";
      if ( skill == do_mrange                      )    return "do_mrange";
      if ( skill == do_mset                        )    return "do_mset";
      if ( skill == do_mstat                       )    return "do_mstat";
      if ( skill == do_muse                        )    return "do_muse";
      if ( skill == do_music                       )    return "do_music";
      if ( skill == do_mwhere                      )    return "do_mwhere";
      if ( skill == do_mxp                         )    return "do_mxp";
      if ( skill == do_name                        )    return "do_name";
      if ( skill == do_neckchop                    )    return "do_neckchop";
      if ( skill == do_neckpinch                   )    return "do_neckpinch";
      if ( skill == do_neckrupture                 )    return "do_neckrupture";
      if ( skill == do_nervepinch                  )    return "do_nervepinch";
      if ( skill == do_nervestrike                 )    return "do_nervestrike";
      if ( skill == do_newbiechat                  )    return "do_newbiechat";
      if ( skill == do_newbieset                   )    return "do_newbieset";
      if ( skill == do_news                        )    return "do_news";
      if ( skill == do_newscore                    )    return "do_newscore";
      if ( skill == do_newzones                    )    return "do_newzones";
      if ( skill == do_niburo                      )    return "do_niburo";
      if ( skill == do_nock                        )    return "do_nock";
      if ( skill == do_noemote                     )    return "do_noemote";
      if ( skill == do_noresolve                   )    return "do_noresolve";
      if ( skill == do_north                       )    return "do_north";
      if ( skill == do_northeast                   )    return "do_northeast";
      if ( skill == do_northwest                   )    return "do_northwest";
      if ( skill == do_notell                      )    return "do_notell";
      if ( skill == do_noteroom                    )    return "do_noteroom";
      if ( skill == do_notitle                     )    return "do_notitle";
      if ( skill == do_npcrace                     )    return "do_npcrace";
      if ( skill == do_nuisance                    )    return "do_nuisance";
      if ( skill == do_oassign                     )    return "do_oassign";
      if ( skill == do_ocreate                     )    return "do_ocreate";
      if ( skill == do_odelete                     )    return "do_odelete";
      if ( skill == do_offered                     )    return "do_offered";
      if ( skill == do_offername                   )    return "do_offername";
      if ( skill == do_offers                      )    return "do_offers";
      if ( skill == do_ofind                       )    return "do_ofind";
      if ( skill == do_ogrub                       )    return "do_ogrub";
      if ( skill == do_oinvoke                     )    return "do_oinvoke";
      if ( skill == do_oldscore                    )    return "do_oldscore";
      if ( skill == do_olist                       )    return "do_olist";
      if ( skill == do_opedit                      )    return "do_opedit";
      if ( skill == do_open                        )    return "do_open";
   // if ( skill == do_opentourney                 )    return "do_opentourney";
      if ( skill == do_opstat                      )    return "do_opstat";
      if ( skill == do_orange                      )    return "do_orange";
      if ( skill == do_order                       )    return "do_order";
      if ( skill == do_orders                      )    return "do_orders";
      if ( skill == do_ordertalk                   )    return "do_ordertalk";
      if ( skill == do_oscatter                    )    return "do_oscatter";
      if ( skill == do_oset                        )    return "do_oset";
      if ( skill == do_ostat                       )    return "do_ostat";
      if ( skill == do_outcast                     )    return "do_outcast";
      if ( skill == do_owhere                      )    return "do_owhere";
      if ( skill == do_pager                       )    return "do_pager";
      if ( skill == do_pardon                      )    return "do_pardon";
      if ( skill == do_parry                       )    return "do_parry";
      if ( skill == do_password                    )    return "do_password";
      if ( skill == do_pcrename                    )    return "do_pcrename";
      if ( skill == do_pcshops                     )    return "do_pcshops";
      if ( skill == do_peace                       )    return "do_peace";
      if ( skill == do_peasant                     )    return "do_peasant";
      if ( skill == do_perfectshot                 )    return "do_perfectshot";
      if ( skill == do_pfiles                      )    return "do_pfiles";
      if ( skill == do_pick                        )    return "do_pick";
      if ( skill == do_piggyback                   )    return "do_piggyback";
      if ( skill == do_pincer                      )    return "do_pincer";
      if ( skill == do_pkillcheck                  )    return "do_pkillcheck";
      if ( skill == do_placemob                    )    return "do_placemob";
      if ( skill == do_placeobj                    )    return "do_placeobj";
      if ( skill == do_placetrainer                )    return "do_placetrainer";
      if ( skill == do_plist                       )    return "do_plist";
      if ( skill == do_poison_weapon               )    return "do_poison_weapon";
      if ( skill == do_portal                      )    return "do_portal";
      if ( skill == do_powerslice                  )    return "do_powerslice";
      if ( skill == do_pretitle                    )    return "do_pretitle";
      if ( skill == do_project                     )    return "do_project";
      if ( skill == do_prompt                      )    return "do_prompt";
      if ( skill == do_pset                        )    return "do_pset";
      if ( skill == do_pstat                       )    return "do_pstat";
      if ( skill == do_pstatus                     )    return "do_pstatus";
      if ( skill == do_pull                        )    return "do_pull";
      if ( skill == do_purge                       )    return "do_purge";
      if ( skill == do_push                        )    return "do_push";
      if ( skill == do_put                         )    return "do_put";
      if ( skill == do_qmob                        )    return "do_qmob";
      if ( skill == do_qobj                        )    return "do_qobj";
      if ( skill == do_qpset                       )    return "do_qpset";
      if ( skill == do_qpstat                      )    return "do_qpstat";
      if ( skill == do_quaff                       )    return "do_quaff";
      if ( skill == do_qui                         )    return "do_qui";
      if ( skill == do_quickcombo                  )    return "do_quickcombo";
      if ( skill == do_quit                        )    return "do_quit";
      if ( skill == do_racetalk                    )    return "do_racetalk";
      if ( skill == do_rank                        )    return "do_rank";
      if ( skill == do_rankings                    )    return "do_rankings";
      if ( skill == do_rap                         )    return "do_rap";
      if ( skill == do_rassign                     )    return "do_rassign";
      if ( skill == do_rat                         )    return "do_rat";
      if ( skill == do_rdelete                     )    return "do_rdelete";
      if ( skill == do_reboo                       )    return "do_reboo";
      if ( skill == do_reboot                      )    return "do_reboot";
      if ( skill == do_recall                      )    return "do_recall";
      if ( skill == do_recho                       )    return "do_recho";
      if ( skill == do_recite                      )    return "do_recite";
      if ( skill == do_redit                       )    return "do_redit";
      if ( skill == do_regoto                      )    return "do_regoto";
      if ( skill == do_remains                     )    return "do_remains";
      if ( skill == do_remove                      )    return "do_remove";
      if ( skill == do_removekingdom               )    return "do_removekingdom";
      if ( skill == do_removetown                  )    return "do_removetown";
      if ( skill == do_rent                        )    return "do_rent";
      if ( skill == do_repair                      )    return "do_repair";
      if ( skill == do_repairset                   )    return "do_repairset";
      if ( skill == do_repairshops                 )    return "do_repairshops";
      if ( skill == do_repairstat                  )    return "do_repairstat";
      if ( skill == do_repairwall                  )    return "do_repairwall";
      if ( skill == do_repeat                      )    return "do_repeat";
      if ( skill == do_reply                       )    return "do_reply";
      if ( skill == do_report                      )    return "do_report";
      if ( skill == do_rescue                      )    return "do_rescue";
      if ( skill == do_reserve                     )    return "do_reserve";
      if ( skill == do_reset                       )    return "do_reset";
      if ( skill == do_resetkeeper                 )    return "do_resetkeeper";
      if ( skill == do_rest                        )    return "do_rest";
      if ( skill == do_restore                     )    return "do_restore";
      if ( skill == do_restorelimbs                )    return "do_restorelimbs";
      if ( skill == do_restoretime                 )    return "do_restoretime";
      if ( skill == do_restrict                    )    return "do_restrict";
      if ( skill == do_resurrection                )    return "do_resurrection";
      if ( skill == do_retell                      )    return "do_retell";
      if ( skill == do_retire                      )    return "do_retire";
      if ( skill == do_retran                      )    return "do_retran";
      if ( skill == do_return                      )    return "do_return";
      if ( skill == do_reward                      )    return "do_reward";
      if ( skill == do_rgrub                       )    return "do_rgrub";
      if ( skill == do_ribpuncture                 )    return "do_ribpuncture";
      if ( skill == do_rip                         )    return "do_rip";
      if ( skill == do_rlist                       )    return "do_rlist";
      if ( skill == do_roar                        )    return "do_roar";
      if ( skill == do_roll                        )    return "do_roll";
      if ( skill == do_roomstat                    )    return "do_roomstat";
      if ( skill == do_roundhouse                  )    return "do_roundhouse";
      if ( skill == do_rpedit                      )    return "do_rpedit";
      if ( skill == do_rpstat                      )    return "do_rpstat";
      if ( skill == do_rreset                      )    return "do_rreset";
      if ( skill == do_rset                        )    return "do_rset";
      if ( skill == do_rstat                       )    return "do_rstat";
      if ( skill == do_rub                         )    return "do_rub";
      if ( skill == do_run                         )    return "do_run";
      if ( skill == do_sacrifice                   )    return "do_sacrifice";
      if ( skill == do_save                        )    return "do_save";
      if ( skill == do_savearea                    )    return "do_savearea";
      if ( skill == do_say                         )    return "do_say";
      if ( skill == do_say_to_char                 )    return "do_say_to_char";
      if ( skill == do_sbook                       )    return "do_sbook";
      if ( skill == do_scan                        )    return "do_scan";
      if ( skill == do_scatter                     )    return "do_scatter";
      if ( skill == do_schedule                    )    return "do_schedule";
      if ( skill == do_score                       )    return "do_score";
      if ( skill == do_scribe                      )    return "do_scribe";
      if ( skill == do_search                      )    return "do_search";
      if ( skill == do_sedit                       )    return "do_sedit";
      if ( skill == do_seeorders                   )    return "do_seeorders";
      if ( skill == do_sell                        )    return "do_sell";
      if ( skill == do_sendmail                    )    return "do_sendmail";
      if ( skill == do_set_boot_time               )    return "do_set_boot_time";
      if ( skill == do_setcaste                    )    return "do_setcaste";
      if ( skill == do_setclan                     )    return "do_setclan";
      if ( skill == do_setcouncil                  )    return "do_setcouncil";
      if ( skill == do_setdeity                    )    return "do_setdeity";
      if ( skill == do_setfree                     )    return "do_setfree";
      if ( skill == do_setgambler                  )    return "do_setgambler";
      if ( skill == do_setgem			   )    return "do_setgem";
      if ( skill == do_setjob                      )    return "do_setjob";
      if ( skill == do_setkingdom                  )    return "do_setkingdom";
      if ( skill == do_setrace                     )    return "do_setrace";
      if ( skill == do_setslay                     )    return "do_setslay";
      if ( skill == do_settoadvance                )    return "do_settoadvance";
      if ( skill == do_setweather                  )    return "do_setweather";
      if ( skill == do_setwilderness               )    return "do_setwilderness";
      if ( skill == do_sheath                      )    return "do_sheath";
      if ( skill == do_shells                      )    return "do_shells";
      if ( skill == do_ships                       )    return "do_ships";
      if ( skill == do_shops                       )    return "do_shops";
      if ( skill == do_shopset                     )    return "do_shopset";
      if ( skill == do_shopstat                    )    return "do_shopstat";
      if ( skill == do_shout                       )    return "do_shout";
      if ( skill == do_shove                       )    return "do_shove";
      if ( skill == do_show                        )    return "do_show";
      if ( skill == do_showascii                   )    return "do_showascii";
      if ( skill == do_showclan                    )    return "do_showclan";
      if ( skill == do_showcontrol                 )    return "do_showcontrol";
      if ( skill == do_showcouncil                 )    return "do_showcouncil";
      if ( skill == do_showdeity                   )    return "do_showdeity";
      if ( skill == do_showentrances               )    return "do_showentrances";
      if ( skill == do_showgambler                 )    return "do_showgambler";
      if ( skill == do_showhouse                   )    return "do_showhouse";
      if ( skill == do_showkingdoms                )    return "do_showkingdoms";
      if ( skill == do_showlayers                  )    return "do_showlayers";
      if ( skill == do_showlist                    )    return "do_showlist";
      if ( skill == do_showpic                     )    return "do_showpic";
      if ( skill == do_showrace                    )    return "do_showrace";
      if ( skill == do_showresources               )    return "do_showresources";
      if ( skill == do_showslay                    )    return "do_showslay";
      if ( skill == do_showweather                 )    return "do_showweather";
      if ( skill == do_shutdow                     )    return "do_shutdow";
      if ( skill == do_shutdown                    )    return "do_shutdown";
      if ( skill == do_sidekick                    )    return "do_sidekick";
      if ( skill == do_silence                     )    return "do_silence";
      if ( skill == do_sing                        )    return "do_sing";
      if ( skill == do_sit                         )    return "do_sit";
      if ( skill == do_skills                      )    return "do_skills";
      if ( skill == do_skin                        )    return "do_skin";
      if ( skill == do_sla                         )    return "do_sla";
      if ( skill == do_slay                        )    return "do_slay";
      if ( skill == do_sleep                       )    return "do_sleep";
      if ( skill == do_slice                       )    return "do_slice";
      if ( skill == do_slist                       )    return "do_slist";
      if ( skill == do_slookup                     )    return "do_slookup";
      if ( skill == do_smoke                       )    return "do_smoke";
      if ( skill == do_snoop                       )    return "do_snoop";
      if ( skill == do_sober                       )    return "do_sober";
      if ( skill == do_socials                     )    return "do_socials";
      if ( skill == do_south                       )    return "do_south";
      if ( skill == do_southeast                   )    return "do_southeast";
      if ( skill == do_southwest                   )    return "do_southwest";
      if ( skill == do_spar                        )    return "do_spar";
      if ( skill == do_speak                       )    return "do_speak";
      if ( skill == do_spear                       )    return "do_spear";
      if ( skill == do_speed                       )    return "do_speed";
      if ( skill == do_spinkick                    )    return "do_spinkick";
      if ( skill == do_split                       )    return "do_split";
      if ( skill == do_sset                        )    return "do_sset";
      if ( skill == do_sslist                      )    return "do_sslist";
      if ( skill == do_stable                      )    return "do_stable";
      if ( skill == do_stack                       )    return "do_stack";
      if ( skill == do_stalk                       )    return "do_stalk";
      if ( skill == do_stand                       )    return "do_stand";
      if ( skill == do_startarena                  )    return "do_startarena";
      if ( skill == do_startfire                   )    return "do_startfire";
      if ( skill == do_startkingdom                )    return "do_startkingdom";
      if ( skill == do_startroom                   )    return "do_startroom";
      if ( skill == do_stat                        )    return "do_stat";
      if ( skill == do_statreport                  )    return "do_statreport";
      if ( skill == do_steal                       )    return "do_steal";
      if ( skill == do_steership                   )    return "do_steership";
      if ( skill == do_stepback                    )    return "do_stepback";
      if ( skill == do_strew                       )    return "do_strew";
      if ( skill == do_strip                       )    return "do_strip";
      if ( skill == do_study                       )    return "do_study";
      if ( skill == do_stun                        )    return "do_stun";
      if ( skill == do_style                       )    return "do_style";
      if ( skill == do_supplicate                  )    return "do_supplicate";
      if ( skill == do_survey                      )    return "do_survey";
      if ( skill == do_switch                      )    return "do_switch";
      if ( skill == do_talent                      )    return "do_talent";
      if ( skill == do_talkquest                   )    return "do_talkquest";
      if ( skill == do_tamp                        )    return "do_tamp";
      if ( skill == do_target                      )    return "do_target";
      if ( skill == do_tcreate                     )    return "do_tcreate";
      if ( skill == do_tdelete                     )    return "do_tdelete";
      if ( skill == do_tell                        )    return "do_tell";
      if ( skill == do_terraform                   )    return "do_terraform";
      if ( skill == do_think                       )    return "do_think";
      if ( skill == do_time                        )    return "do_time";
      if ( skill == do_timecmd                     )    return "do_timecmd";
      if ( skill == do_timmuru                     )    return "do_timmuru";
      if ( skill == do_tinduct                     )    return "do_tinduct";
      if ( skill == do_title                       )    return "do_title";
      if ( skill == do_tkickout                    )    return "do_tkickout";
      if ( skill == do_tone                        )    return "do_tone";
      if ( skill == do_tornadokick                 )    return "do_tornadokick";
      if ( skill == do_toss                        )    return "do_toss";
      if ( skill == do_track                       )    return "do_track";
      if ( skill == do_tradegoods                  )    return "do_tradegoods";
      if ( skill == do_traderoutes                 )    return "do_traderoutes";
      if ( skill == do_training                    )    return "do_training";
      if ( skill == do_trans                       )    return "do_trans";
      if ( skill == do_transfer                    )    return "do_transfer";
      if ( skill == do_trap                        )    return "do_trap";
      if ( skill == do_trust                       )    return "do_trust";
      if ( skill == do_tset                        )    return "do_tset";
      if ( skill == do_tstat                       )    return "do_tstat";
      if ( skill == do_tumble                      )    return "do_tumble";
      if ( skill == do_typo                        )    return "do_typo";
      if ( skill == do_unfoldarea                  )    return "do_unfoldarea";
      if ( skill == do_unhell                      )    return "do_unhell";
      if ( skill == do_unloadqarea                 )    return "do_unloadqarea";
      if ( skill == do_unlock                      )    return "do_unlock";
      if ( skill == do_unnuisance                  )    return "do_unnuisance";
      if ( skill == do_unsilence                   )    return "do_unsilence";
      if ( skill == do_up                          )    return "do_up";
      if ( skill == do_updatearea                  )    return "do_updatearea";
      if ( skill == do_updateskills                )    return "do_updateskills";
      if ( skill == do_uselicense                  )    return "do_uselicense";
      if ( skill == do_users                       )    return "do_users";
      if ( skill == do_value                       )    return "do_value";
      if ( skill == do_vassign                     )    return "do_vassign";
      if ( skill == do_version                     )    return "do_version";
      if ( skill == do_victories                   )    return "do_victories";
      if ( skill == do_viewmount                   )    return "do_viewmount";
      if ( skill == do_viewskills                  )    return "do_viewskills";
      if ( skill == do_visible                     )    return "do_visible";
      if ( skill == do_vnums                       )    return "do_vnums";
      if ( skill == do_vsearch                     )    return "do_vsearch";
      if ( skill == do_wake                        )    return "do_wake";
      if ( skill == do_warn                        )    return "do_warn";
      if ( skill == do_wartalk                     )    return "do_wartalk";
      if ( skill == do_watch                       )    return "do_watch";
      if ( skill == do_wblock                      )    return "do_wblock";
      if ( skill == do_weaponbreak                 )    return "do_weaponbreak";
      if ( skill == do_wear                        )    return "do_wear";
      if ( skill == do_weather                     )    return "do_weather";
      if ( skill == do_webstats                    )    return "do_webstats";
      if ( skill == do_west                        )    return "do_west";
      if ( skill == do_where                       )    return "do_where";
      if ( skill == do_whisper                     )    return "do_whisper";
      if ( skill == do_who                         )    return "do_who";
      if ( skill == do_whois                       )    return "do_whois";
      if ( skill == do_whonumber                   )    return "do_whonumber";
      if ( skill == do_wimpy                       )    return "do_wimpy";
      if ( skill == do_withdraw                    )    return "do_withdraw";
      if ( skill == do_wizhelp                     )    return "do_wizhelp";
      if ( skill == do_wizlist                     )    return "do_wizlist";
      if ( skill == do_wizlock                     )    return "do_wizlock";
      if ( skill == do_wizzap                      )    return "do_wizzap";
      if ( skill == do_worth                       )    return "do_worth";
      if ( skill == do_yell                        )    return "do_yell";
      if ( skill == do_zap                         )    return "do_zap";
      if ( skill == do_zones                       )    return "do_zones";
   /*//T4*/

   sprintf(buf, "(%p)", skill);
   return buf;
}

// Market data.  Allows players to place objects up for sale.
void save_market_data()
{
   FILE *fp;
   char filename[256];
   MARKET_DATA *market;

   sprintf(filename, "%s", MARKET_FILE);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_market_data: fopen", 0);
      perror(filename);
   }
   else
   {
      fprintf(fp, "Startpid   %d\n", start_marketpid);
      for (market = first_market; market; market = market->next)
      {
         fprintf(fp, "#MARKET\n");
         fprintf(fp, "Cost       %d\n", market->cost);
         fprintf(fp, "Name       %s~\n", market->name);
         fprintf(fp, "Pid        %d\n", market->pid);
         fprintf(fp, "MPid       %d\n", market->mpid);
         fprintf(fp, "Count      %d\n", market->count);
         fprintf(fp, "SCount     %d\n", market->scount);
         fprintf(fp, "End\n");
         fwrite_obj(NULL, market->obj, fp, 0, OS_MARKET);
      }
   }
   fprintf(fp, "#END\n");
   fclose(fp);
   return;
}

void fread_market(FILE * fp, MARKET_DATA *market)
{
   bool fMatch;
   char *word;

   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      switch (UPPER(word[0]))
      {         
         case 'C':
            KEY("Cost", market->cost, fread_number(fp));
            KEY("Count", market->count, fread_number(fp));
            break;
            
         case 'M':
            KEY("MPid", market->mpid, fread_number(fp));
            break;
            
         case 'N':
            KEY("Name", market->name, fread_string(fp));
            break;
            
         case 'P':
            KEY("Pid", market->pid, fread_number(fp));
            break;
            
         case 'S':
            KEY("Startpid", start_marketpid, fread_number(fp));
            KEY("SCount", market->scount, fread_number(fp));
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
               return;
      }
   }
}

void load_market_data()
{
   char filename[256];
   MARKET_DATA *market = NULL;
   FILE *fp;
   bool found;
   char letter;
   char *word;

   found = FALSE;

   sprintf(filename, "%s", MARKET_FILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
      word = fread_word(fp);
      start_marketpid = fread_number(fp);
      for (;;)
      {
         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            bug("load_market_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "MARKET"))
         {
            CREATE(market, MARKET_DATA, 1);
            fread_market(fp, market);
            LINK(market, first_market, last_market, next, prev);
            continue;
         }
         else if (!str_cmp(word, "MARKETOBJ"))
         {
            globalmarketptr = market;
            fread_obj(NULL, fp, OS_MARKET);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("load_market_data: bad section: %s.", word);
            break;
         }
      }
      fclose(fp);
   }
}

// Kingdom Storage Bins, work like the Clan ones mainly
void save_kingdom_chests(CHAR_DATA * ch)
{
   FILE *fp;
   char filename[256];
   KCHEST_DATA *kchest;

   if (!ch)
   {
      bug("save_kingdom_chests: Null ch pointer!", 0);
      return;
   }

   sprintf(filename, "%s%s", CASTE_DIR, KCHEST_FILE);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_kingdom_chests: fopen", 0);
      perror(filename);
   }
   else
   {
      for (kchest = first_kchest; kchest; kchest = kchest->next)
      {
         fprintf(fp, "#CHEST\n");
         fprintf(fp, "_VNUM_\n");
         fprintf(fp, "Vnum       %d\n", kchest->obj->in_room->vnum);
         fprintf(fp, "X          %d\n", kchest->obj->coord->x);
         fprintf(fp, "Y          %d\n", kchest->obj->coord->y);
         fprintf(fp, "Map        %d\n", kchest->obj->map);
         fprintf(fp, "Version    %d\n", SAVEVERSION);
         fprintf(fp, "End\n\n");
         fprintf(fp, "_CHEST_\n");
         fwrite_obj(ch, kchest->obj, fp, 0, OS_KINGDOM);
         fprintf(fp, "#STOP\n\n\n");
      }
   }
   fprintf(fp, "#END\n");
   fclose(fp);
   return;
}

void fread_chest(FILE * fp, KCHEST_DATA * kchest)
{
   int vnum = 0;
   int found = 0;
   int x, y, map;
   bool fMatch;
   char *word;
   char letter;
   ROOM_INDEX_DATA *room = NULL;
   OBJ_DATA *tobj, *tobj_next;

   x=y=map=-1;
   for (;;)
   {
      if (found == 1)
         break;
      word = feof(fp) ? "End" : fread_word(fp);

      if (!str_cmp(word, "_CHEST_"))
      {
         for (;;)
         {
            letter = fread_letter(fp);
            if (letter == '*')
            {
               fread_to_eol(fp);
               continue;
            }
            if (letter != '#')
            {
               bug("fread_chest: # not found.", 0);
               break;
            }
            word = fread_word(fp);
            if (vnum != 0)
               room = get_room_index(vnum);

            if (!str_cmp(word, "OBJECT"))
            {
               if (room)
               {
                  rset_supermob(room);
                  fread_obj(supermob, fp, OS_CARRY);
               }
               else
               {
                  bug("fread_chest: no room for the chest");
                  return;
               }
            }
            else
            {
               if (room)
               {
                  for (tobj = supermob->first_carrying; tobj; tobj = tobj_next)
                  {
                     tobj_next = tobj->next_content;
                     kchest->obj = tobj;
                     LINK(kchest, first_kchest, last_kchest, next, prev);
                     obj_from_char(tobj);
                     obj_to_room(tobj, room, supermob);
                     tobj->coord->x = x;
                     tobj->coord->y = y;
                     tobj->map = map;
                  }
                  release_supermob();
                  found = 1;
                  break;
               }
            }
         }
      }
      else if (!str_cmp(word, "_VNUM_"))
      {
         int fnd = 0;

         for (;;)
         {
            word = feof(fp) ? "End" : fread_word(fp);

            switch (UPPER(word[0]))
            {
               case 'M':
                  KEY("Map", map, fread_number(fp));
                  break;
                  
               case 'V':
                  KEY("Version", file_ver, fread_number(fp));
                  KEY("Vnum", vnum, fread_number(fp));
                  break;
                  
               case 'X':
                  KEY("X", x, fread_number(fp));
                  break;
                 
               case 'Y':
                  KEY("Y", y, fread_number(fp));
                  break;

               case 'E':
                  if (!str_cmp(word, "End"))
                  {
                     fnd = 1;
                     break;
                  }
            }
            if (fnd == 1)
               break;
         }
      }
      else
         break;
   }
   file_ver = 0;
}

void load_kchest_file()
{
   char filename[256];
   KCHEST_DATA *kchest;
   FILE *fp;
   bool found;

   found = FALSE;

   sprintf(filename, "%s%s", CASTE_DIR, KCHEST_FILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("Load_kchest_file: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "CHEST"))
         {
            CREATE(kchest, KCHEST_DATA, 1);
            fread_chest(fp, kchest);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            char buf[MSL];

            sprintf(buf, "Load_kchest_file: bad section: %s.", word);
            bug(buf, 0);
            break;
         }
      }
      fclose(fp);
   }
}

void save_trap_file(TRAP_DATA *strap, FILE *sfp)
{
   char buf[MSL];
   FILE *fp;
   TRAP_DATA *trap;
   if (strap)
   {
      fprintf(sfp, "Start\n");
      fprintf(sfp, "Charges        %d\n", strap->charges);
      fprintf(sfp, "MaxCharges     %d\n", strap->maxcharges);
      fprintf(sfp, "Uid            %d\n", strap->uid);
      fprintf(sfp, "Type           %d\n", strap->type);
      fprintf(sfp, "Damlow         %d\n", strap->damlow);
      fprintf(sfp, "Damhigh        %d\n", strap->damhigh);
      fprintf(sfp, "Room           %d\n", strap->room);
      fprintf(sfp, "Difficulty     %d\n", strap->difficulty);
      fprintf(sfp, "Toolkit        %d\n", strap->toolkit);
      fprintf(sfp, "Onetime        %d\n", strap->onetime);
      fprintf(sfp, "Disarmed       %d\n", strap->disarmed);
      fprintf(sfp, "Trapflags      %s\n", print_bitvector(&strap->trapflags));
      fprintf(sfp, "Resetvalue     %d\n", strap->resetvalue);
      fprintf(sfp, "Toolnegate     %d\n", strap->toolnegate);
      fprintf(sfp, "Frag           %d\n", strap->frag);
      fprintf(sfp, "End\n");
      fprintf(sfp, "Stop\n");
      return;
   }   
   sprintf(buf, "%s", TRAP_FILE);
   if ((fp = fopen(buf, "w")) == NULL)
   {
      perror(buf);
      return;
   }
   for (trap = first_trap; trap; trap = trap->next)
   {
      if (trap->uid < START_INV_TRAP)
      {
         fprintf(fp, "Start\n");
         fprintf(fp, "Charges        %d\n", trap->charges);
         fprintf(fp, "MaxCharges     %d\n", trap->maxcharges);
         fprintf(fp, "Uid            %d\n", trap->uid);
         fprintf(fp, "Type           %d\n", trap->type);
         fprintf(fp, "Damlow         %d\n", trap->damlow);
         fprintf(fp, "Damhigh        %d\n", trap->damhigh);
         fprintf(fp, "Room           %d\n", trap->room);
         fprintf(fp, "Difficulty     %d\n", trap->difficulty);
         fprintf(fp, "Toolkit        %d\n", trap->toolkit);
         fprintf(fp, "Onetime        %d\n", trap->onetime);
         fprintf(fp, "Disarmed       %d\n", trap->disarmed);
         fprintf(fp, "Trapflags      %s\n", print_bitvector(&trap->trapflags));
         fprintf(fp, "Resetvalue     %d\n", trap->resetvalue);
         fprintf(fp, "Toolnegate     %d\n", trap->toolnegate);
         fprintf(fp, "Frag           %d\n", trap->frag);
         fprintf(fp, "End\n");
      }
   }
   fprintf(fp, "Stop\n");
   fclose(fp);
}   

TRAP_DATA *load_trap_file(FILE *sfp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   TRAP_DATA *trap = NULL;
   FILE *fp;
   
   if (!sfp)
   {
      sprintf(buf, "%s", TRAP_FILE);
      if ((fp = fopen(buf, "r")) == NULL)
      {
         perror(buf);
         return NULL;;
      }
   }
   else
      fp = sfp;

   for (;;)
   {
      word = feof(fp) ? "Stop" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
            
         case 'C':
            KEY("Charges", trap->charges, fread_number(fp));
            break;

         case 'D':
            KEY("Damlow", trap->damlow, fread_number(fp));
            KEY("Damhigh", trap->damhigh, fread_number(fp));
            KEY("Difficulty", trap->difficulty, fread_number(fp));
            KEY("Disarmed", trap->disarmed, fread_number(fp));
            break;
            
         case 'F':
            KEY("Frag", trap->frag, fread_number(fp));
            break;
            
         case 'M':
            KEY("MaxCharges", trap->maxcharges, fread_number(fp));
            break;
            
         case 'O':
            KEY("Onetime", trap->onetime, fread_number(fp));
            break;
            
         case 'R':
            KEY("Resetvalue", trap->resetvalue, fread_number(fp));
            KEY("Room", trap->room, fread_number(fp));
            break;
            
         case 'T':
            KEY("Type", trap->type, fread_number(fp));
            KEY("Toolkit", trap->toolkit, fread_number(fp));
            KEY("Trapflags", trap->trapflags, fread_bitvector(fp));
            KEY("Toolnegate", trap->toolnegate, fread_number(fp));
            break;
            
         case 'U':
            KEY("Uid", trap->uid, fread_number(fp));
            break;
            
         case 'S':
            if (!str_cmp(word, "Start"))
            {
               CREATE(trap, TRAP_DATA, 1);
               fMatch = TRUE;
            }   
            if (!str_cmp(word, "Stop"))
            {
               if (!sfp)
                  fclose(fp);
               return trap;
            }
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(trap, first_trap, last_trap, next, prev);
               if (sfp && trap->uid > sysdata.last_invtrap_uid)
               {
                  bug("Trap with uid %d is higher than sysdata.last_invtrap_uid.  Fixing");
                  sysdata.last_invtrap_uid = trap->uid;
               }
               if (!sfp && trap->uid > sysdata.last_trap_uid)
               {
                  bug("Trap with uid %d is higher than sysdata.last_trap_uid.  Fixing");
                  sysdata.last_trap_uid = trap->uid;
               }
               fMatch = TRUE;
               break;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "load_portal_file: no match: %s", word);
         bug(buf, 0);
      }
   }
   fclose(fp);
   return NULL;
} 
   
void fwrite_ship_data()
{
   char buf[MSL];
   FILE *fp;
   SHIP_DATA *ship;
   
   sprintf(buf, "%s", SHIP_FILE);
   if ((fp = fopen(buf, "w")) == NULL)
   {
      perror(buf);
      return;
   }
   fprintf(fp, "CURUID          %d\n", cur_ship_uid);
   for (ship = first_ship; ship; ship = ship->next)
   {
      fprintf(fp, "X               %d\n", ship->x);
      fprintf(fp, "Y               %d\n", ship->y);
      fprintf(fp, "Map             %d\n", ship->map);
      fprintf(fp, "TX              %d\n", ship->tx);
      fprintf(fp, "TY              %d\n", ship->ty);
      fprintf(fp, "TMap            %d\n", ship->tmap);
      fprintf(fp, "Direction       %d\n", ship->direction);
      fprintf(fp, "Size            %d\n", ship->size);
      fprintf(fp, "Ticket          %d\n", ship->ticket);
      fprintf(fp, "Occupants       %d\n", ship->occupants);
      fprintf(fp, "Uid             %d\n", ship->uid);
      fprintf(fp, "Travelroute     %s~\n", ship->travelroute);
      fprintf(fp, "Routeplace      %d\n", ship->routeplace);
      fprintf(fp, "Routedir        %d\n", ship->routedir);
      fprintf(fp, "Routetime       %d\n", ship->routetime);
      fprintf(fp, "Routetick       %d\n", ship->routetick);
      fprintf(fp, "End             \n");
   }
   fprintf(fp, "Stop\n");
   fclose(fp);   
}

void load_ship_data()
{
   char buf[MSL];
   char *word;
   bool fMatch;
   SHIP_DATA *ship = NULL;
   FILE *fp;

   cur_ship_uid = 0;
   sprintf(buf, "%s", SHIP_FILE);
   if ((fp = fopen(buf, "r")) == NULL)
   {
      perror(buf);
      return;
   }

   for (;;)
   {
      word = feof(fp) ? "Stop" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
         case 'C':
            KEY("CURUID", cur_ship_uid, fread_number(fp));
            break;

         case 'D':
            KEY("Direction", ship->direction, fread_number(fp));
            break;
            
         case 'U':
            KEY("Uid", ship->uid, fread_number(fp));
            break;

         case 'M':
            KEY("Map", ship->map, fread_number(fp));
            break;
            
         case 'O':
            KEY("Occupants", ship->occupants, fread_number(fp));
            break;
            
         case 'R':
            KEY("Routeplace", ship->routeplace, fread_number(fp));
            KEY("Routedir", ship->routedir, fread_number(fp));
            KEY("Routetime", ship->routetime, fread_number(fp));
            KEY("Routetick", ship->routetick, fread_number(fp));
            break;
            
         case 'T':
            KEY("Ticket", ship->ticket, fread_number(fp));
            KEY("Travelroute", ship->travelroute, fread_string(fp));
            KEY("TX", ship->tx, fread_number(fp));
            KEY("TY", ship->ty, fread_number(fp));
            KEY("TMap", ship->tmap, fread_number(fp));
            break;

         case 'X':
            if (!str_cmp(word, "X"))
            {
               CREATE(ship, SHIP_DATA, 1);
               ship->x = fread_number(fp);
               fMatch = TRUE;
            }
            break;

         case 'Y':
            KEY("Y", ship->y, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(ship, first_ship, last_ship, next, prev);
               fMatch = TRUE;
               set_ship_sector(ship, 0, 0);
            }
            break;
            
         case 'S':
            KEY("Size", ship->size, fread_number(fp));
            if (!str_cmp(word, "Stop"))
            {
               fclose(fp);
               return;
            }
            break;
      }
      if (!fMatch)
      {
         sprintf(buf, "load_ship_data: no match: %s", word);
         bug(buf, 0);
      }
   }
   return;
}

void load_portal_file()
{
   char buf[MSL];
   char *word;
   bool fMatch;
   PORTAL_DATA *portal;
   FILE *fp;
   int num = 0;

   sprintf(buf, "%s%s", KINGDOM_DIR, PORTAL_FILE);
   if ((fp = fopen(buf, "r")) == NULL)
   {
      perror(buf);
      return;
   }

   CREATE(portal, struct portal_data, 1);

   for (;;)
   {
      word = feof(fp) ? "Stop" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'D':
            KEY("Description", portal->desc, fread_string(fp));
            break;

         case 'M':
            KEY("Map", portal->map, fread_number(fp));
            break;

         case 'X':
            KEY("X", portal->x, fread_number(fp));
            break;

         case 'Y':
            KEY("Y", portal->y, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               portal_show[num] = portal;
               num++;
               fMatch = TRUE;
               CREATE(portal, struct portal_data, 1);

               break;
            }
         case 'S':
            if (!str_cmp(word, "Stop"))
            {
               fclose(fp);
               sysdata.last_portal = num;
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "load_portal_file: no match: %s", word);
         bug(buf, 0);
      }
   }
   sysdata.last_portal = num;
   fclose(fp);
   return;
}

void save_portal_file(void)
{
   char buf[MSL];
   FILE *fp;
   int num = 0;

   sprintf(buf, "%s%s", KINGDOM_DIR, PORTAL_FILE);
   if ((fp = fopen(buf, "w")) == NULL)
   {
      perror(buf);
      return;
   }
            
   for (num = 0; num < sysdata.last_portal; num++)
   {
      fprintf(fp, "Description    %s~\n", portal_show[num]->desc);
      fprintf(fp, "Map            %d\n", portal_show[num]->map);
      fprintf(fp, "X              %d\n", portal_show[num]->x);
      fprintf(fp, "Y              %d\n", portal_show[num]->y);
      fprintf(fp, "End\n");
   }
   fprintf(fp, "Stop\n");
   fclose(fp);
}

void save_npcrace_file(void)
{
   char buf[MSL];
   FILE *fp;
   NPCRACE_DATA *npcrace;
   int x;
   
   sprintf(buf, "%s", NPCRACE_FILE);
   if ((fp = fopen(buf, "w")) == NULL)
   {
      perror(buf);
      return;
   }
   fprintf(fp, "MAXQUESTDIFF     %d\n", MAX_QUEST_DIFF);      
   for (npcrace = first_npcrace; npcrace; npcrace = npcrace->next)
   {
      fprintf(fp, "Racenum          %d\n", npcrace->racenum);
      fprintf(fp, "Racename         %s~\n", npcrace->racename);
      fprintf(fp, "Willload        ");
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         fprintf(fp, " %d", npcrace->willload[x]);
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         fprintf(fp, "\nDescription %d %s~", x, npcrace->description[x]);
      fprintf(fp, "\nSex             ");
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         fprintf(fp, " %d", npcrace->sex[x]);
      fprintf(fp, "\nFlags           ");
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         fprintf(fp, " %s", print_bitvector(&npcrace->flags[x]));
      fprintf(fp, "\nNFlags          ");
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         fprintf(fp, " %s", print_bitvector(&npcrace->nflags[x]));
      fprintf(fp, "\nFulldescription ");
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         fprintf(fp, " %d", npcrace->fulldescription[x]);
      fprintf(fp, "\nEnd\n");
   }
   fprintf(fp, "Stop\n");
   fclose(fp);
}

void load_npcrace_file()
{
   char buf[MSL];
   char *word;
   bool fMatch;
   NPCRACE_DATA *npcrace = NULL;
   int x;
   FILE *fp;
   int maxnpc = -1;
   int maxqdiff = MAX_QUEST_DIFF; 

   sprintf(buf, "%s", NPCRACE_FILE);
   if ((fp = fopen(buf, "r")) == NULL)
   {
      perror(buf);
      return;
   }

   for (;;)
   {
      word = feof(fp) ? "Stop" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'D':
            if (!str_cmp(word, "Description"))
            {
               x = fread_number(fp);
               npcrace->description[x] = fread_string(fp);
               fMatch = TRUE;
            }
            break;
            
         case 'F':
            if (!str_cmp(word, "Fulldescription"))
            {
               for (x = 0; x <= maxqdiff-1; x++)
                  npcrace->fulldescription[x] = fread_number(fp);
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Flags"))
            {
               for (x = 0; x <= maxqdiff-1; x++)
                  npcrace->flags[x] = fread_bitvector(fp);
               fMatch = TRUE;
            }
            break;
         
         case 'M':
            KEY("MAXQUESTDIFF", maxqdiff, fread_number(fp));
            break;
            
         case 'N':
            if (!str_cmp(word, "NFlags"))
            {
               for (x = 0; x <= maxqdiff-1; x++)
                  npcrace->nflags[x] = fread_bitvector(fp);
               fMatch = TRUE;
            }
            break;
            
         case 'R':
            if (!str_cmp(word, "Racenum"))
            {
               CREATE(npcrace, NPCRACE_DATA, 1);
               npcrace->racenum = fread_number(fp);
               fMatch = TRUE;
            }
            KEY("Racename", npcrace->racename, fread_string(fp));
            break;
            
         case 'S':
            if (!str_cmp(word, "Sex"))
            {
               for (x = 0; x <= maxqdiff-1; x++)
                  npcrace->sex[x] = fread_number(fp);
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Stop"))
            {
               for (x = 0; x <= MAX_NPCRACE_TABLE-1; x++)
                  npcrace_table[MAX_NPCRACE_TABLE] = NULL;
               for (npcrace = first_npcrace; npcrace; npcrace = npcrace->next)
               {
                  if (npcrace->racenum > maxnpc)
                     maxnpc = npcrace->racenum;
                  if (npcrace_table[npcrace->racenum])
                     bug("There are two enteries for number %d in the npcrace table", npcrace->racenum);
                  npcrace_table[npcrace->racenum] = npcrace;
               }
               max_npc_race = maxnpc+1;
               fclose(fp);
               return;
            }
            break;
            
         case 'W':
            if (!str_cmp(word, "Willload"))
            {
               for (x = 0; x <= maxqdiff-1; x++)
                  npcrace->willload[x] = fread_number(fp);
               fMatch = TRUE;
            }
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(npcrace, first_npcrace, last_npcrace, next, prev);
               fMatch = TRUE;
            }
            break;
      }
      if (!fMatch)
      {
         sprintf(buf, "load_npcrace_file: no match: %s", word);
         bug(buf, 0);
      }
   }
   fclose(fp);
   return;
}

void copy_files_contents(FILE *fsource, FILE *fdestination)
{
   int ch;
   int cnt = 1;
   
   for (;;)
   {
      ch = fgetc( fsource );
      if (!feof(fsource))
      {
          fputc( ch, fdestination);
          if (ch == '\n')
          {
             cnt++;
             if (cnt >= LAST_FILE_SIZE) //limit size of this file please :-)
                break;
          }
      }
      else
          break;
   }
}

void write_last_file(char *entry)
{
   FILE *fpout;
   FILE *fptemp;
   char filename[MIL];
   char tempname[MIL];
   
   sprintf(filename, "%s", LAST_LIST);
   sprintf(tempname, "%s", LAST_TEMP_LIST);
   if ((fptemp = fopen(tempname, "w")) == NULL)
   {
      bug("Cannot open: %s for writing", tempname);
      return;
   }
   fprintf(fptemp, "%s\n", entry); //adds new entry to top of the file
   if ((fpout = fopen(filename, "r")) != NULL)
   {
      copy_files_contents(fpout, fptemp); //copy the rest to the file
      fclose(fpout); //close the files since writing is done
   }
   fclose(fptemp);
   
   if (remove(filename) != 0 && fopen(filename, "r") != NULL)
   {
      bug("Do not have permission to delete the %s file", filename);
      return;
   }
   if (rename(tempname, filename) != 0)
   {
      bug("Do not have permission to rename the %s file", tempname);
      return;
   }
   return;
}

void read_last_file(CHAR_DATA *ch, int count, char *name)
{
   FILE *fpout;
   char filename[MIL];
   char charname[100];
   int cnt = 0;
   int letter = 0;
   char *ln;
   char *c;
   char d, e;
   struct tm *tme;
   time_t now;
   char day[MIL];
   char sday[5];
   int fnd = 0;
   
   sprintf(filename, "%s", LAST_LIST);
   if ((fpout = fopen(filename, "r")) == NULL)
   {
      send_to_char("There is no last file to look at.\n\r", ch);
      return;
   }
   
   for (;;)
   {
      if (feof(fpout))
      {
         fclose(fpout);
         ch_printf(ch, "---------------------------------------------------------------------------\n\r%d Entries Listed.\n\r", cnt);
         return;
      }
      else
      {
         if (count == -2 || ++cnt <= count || count == -1)
         {
            ln = fread_line(fpout);
            strcpy(charname, "");
            if (name) //looking for a certain name
            {
               c = ln; 
               for (;;)
               {
                  if (isalpha(*c) && !isspace(*c))
                  {
                     charname[letter] = *c;   
                     letter++;
                     c++;
                  }
                  else
                  {
                     charname[letter] = '\0';
                     if (!str_cmp(charname, name))
                     {
                        ch_printf(ch, "%s", ln);
                        letter = 0;
                        strcpy(charname, "");
                        break;  
                     }
                     else
                     {
                        if (!feof(fpout))
                        {
                           fread_line(fpout);    
                           c = ln;
                           letter = 0;
                           strcpy(charname, "");
                           continue;
                        }
                        else
                        {
                           cnt--;
                           break;
                        }
                     }
                  }
               }
            }
            else if (count == -2) //only today's entries
            {
               c = ln;
               now = time(0);
               tme = localtime(&now);
               strftime(day, 10, "%d", tme);
               for (;;)
               {
                  if (!isdigit(*c))
                  {
                     c++;
                  }
                  else
                  {
                     d = *c;
                     c++;
                     e = *c;
                     sprintf(sday, "%c%c", d, e);
                     if (!str_cmp(sday, day))
                     {
                         fnd = 1;
                         cnt++;
                         ch_printf(ch, "%s", ln);
                         break;
                     }
                     else
                     {
                        if (fnd == 1)
                        {
                           fclose(fpout);
                           ch_printf(ch, "---------------------------------------------------------------------------\n\r%d Entries Listed.\n\r", cnt);
                           return;
                        }
                        else
                           break;
                     }
                  }
               }
            }
            else               
            {
               ch_printf(ch, "%s", ln);     
            }
         }
         else
         {
            fclose(fpout);
            ch_printf(ch, "--------------------------------------------------------------------------\n\r%d Entries Listed.\n\r", count);
            return;
         }
      }
   }
}

void fwrite_battle_descriptions()
{
   FILE *fpout;
   char filename[MIL];
   int x;
   int y;
   int z;
   
   sprintf(filename, "%s", BDESCRIPTION_LIST);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      bug("Cannot open: %s for writing", filename);
      return;
   }     
   for (z = 0; z < 7; z++)
   {
      for (x = 0; x < 3; x++)  
      {
         for (y = 0; y < 100; y++)
         {
            if (x == 0 && y == 0)
            {
               if (z == 0)
                  fprintf(fpout, "#HIT\n");
               if (z == 1)
                  fprintf(fpout, "#MISS\n");
               if (z == 2)
                  fprintf(fpout, "#BLOCK\n");
               if (z == 3)
                  fprintf(fpout, "#PARRY\n");
               if (z == 4)
                  fprintf(fpout, "#DODGE\n");
               if (z == 5) 
                  fprintf(fpout, "#CRIT\n");
               if (z == 6)
                  fprintf(fpout, "#INSTA\n");
            }
            if (y == 0)
            {
               if (x == 0)
                  fprintf(fpout, "#BASH\n");
               else if (x == 1)
                  fprintf(fpout, "#SLASH\n");
               else if (x == 2)
                  fprintf(fpout, "#STAB\n");
            }
            if (battle_descriptions[z][x][y][0] != '\0')
            {
               fprintf(fpout, "%s~\n", battle_descriptions[z][x][y]);
            }
            if (y == 99)
               fprintf(fpout, "End~\n\n");
         }
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fwrite_training_list()
{
   FILE *fpout;
   char filename[MIL];
   TRAINING_DATA *training;

   sprintf(filename, "%s", TRAINING_LIST);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (training = first_training; training; training = training->next)
   {
      if (!training->kmob)
         continue;
      fprintf(fpout, "#TrainingStart\n");
      fprintf(fpout, "Kmob       %d\n", training->kmob->vnum);
      fprintf(fpout, "Speed      %d\n", training->speed);
      fprintf(fpout, "Stime      %d\n", training->stime);
      fprintf(fpout, "Etime      %d\n", training->etime);
      fprintf(fpout, "Town       %d\n", training->town);
      fprintf(fpout, "Kingdom    %d\n", training->kingdom);
      if (training->x)
      {
         fprintf(fpout, "X          %d\n", training->x);
         fprintf(fpout, "Y          %d\n", training->y);
         fprintf(fpout, "Map        %d\n", training->map);
         fprintf(fpout, "Bin        %d\n", training->bin);
         fprintf(fpout, "Resource   %d\n", training->resource);
      }
      fprintf(fpout, "#TrainingEnd\n");
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}
   
void fread_training_list()
{
   FILE *fp;
   char filename[MIL];
   char buf[MSL];
   char *word;
   int fMatch;
   int vnum = -1;
   BUYKMOB_DATA *kmob;
   TRAINING_DATA *training = NULL;
   
   sprintf(filename, "%s", TRAINING_LIST);
   if ((fp = fopen(filename, "r")) != NULL)
   {
      for (;;)
      {
         word = feof(fp) ? "#End" : fread_word(fp);
         fMatch = FALSE;

         switch (UPPER(word[0]))
         {
            case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
            
         case 'B':
            KEY("Bin", training->bin, fread_number(fp));
            break;  
            
         case 'E':
            KEY("Etime", training->etime, fread_number(fp));
            break;
            
         case 'K':
            KEY("Kingdom", training->kingdom, fread_number(fp));
            if (!str_cmp(word, "Kmob"))
            {
               fMatch = TRUE;
               vnum = fread_number(fp);
               for (kmob = first_buykmob; kmob; kmob = kmob->next)
               {
                  if (kmob->vnum == vnum)
                     break;
               }
               if (kmob)
                  training->kmob = kmob;
               else
                  vnum = -1;
            }
            break;       
            
         case 'M':
            KEY("Map", training->map, fread_number(fp));
            break;
            
         case 'R':
            KEY("Resource", training->resource, fread_number(fp));
            break;
            
         case 'S':
            KEY("Speed", training->speed, fread_number(fp));
            KEY("Stime", training->stime, fread_number(fp));
            break;
            
         case 'T':
            KEY("Town", training->town, fread_number(fp));
            break;
            
         case 'X':
            KEY("X", training->x, fread_number(fp));
            break;
            
         case 'Y':
            KEY("Y", training->y, fread_number(fp));
            break;
            
         case '#':
            if (!str_cmp(word, "#Trainingstart"))
            {
               CREATE(training, TRAINING_DATA, 1);
               fMatch = TRUE;
            }
            if (!str_cmp(word, "#Trainingend"))
            {
               fMatch = TRUE;
               if (vnum != -1)
                  LINK(training, first_training, last_training, next, prev);
               else
                  DISPOSE(training); //Bad node, could not find kmob in list, dispose of...
            }
            if (!str_cmp(word, "#END"))
            {
                fclose(fp);
                return;
            }
         }  
         if (!fMatch)
         {
            sprintf(buf, "fread_training_list: no match: %s", word);
            bug(buf, 0);
         }
      }
   }
}
   
void fread_battle_descriptions()
{
   FILE *fp;
   char filename[MIL];
   char *desc;
   int y;
   int z = 0;
   int x = -1;
   int cnt = 0;
   
   sprintf(filename, "%s", BDESCRIPTION_LIST);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("Load_conquer_file: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "HIT"))
         {
            z = 0;
            continue;
         }
         else if (!str_cmp(word, "MISS"))
         {
            z = 1;
            continue;
         }
         else if (!str_cmp(word, "BLOCK"))
         {
            z = 2;
            continue;
         }
         else if (!str_cmp(word, "PARRY"))
         {
            z = 3;
            continue;
         }
         else if (!str_cmp(word, "DODGE"))
         {
            z = 4;
            continue;
         }
         else if (!str_cmp(word, "CRIT"))
         {
            z = 5;
            continue;
         }
         else if (!str_cmp(word, "INSTA"))
         {
            z = 6;
            continue;
         }
         else if (!str_cmp(word, "BASH"))
            y = 0;     
         else if (!str_cmp(word, "SLASH"))
            y = 1;
         else if (!str_cmp(word, "STAB"))
            y = 2;
         else if (!str_cmp(word, "END"))
         {
            fclose(fp);
            return;
         }
         else
         {
            bug("fread_battle_description: bad section.", 0);
            exit(0);
         }
         for (;;)
         {        
            if (feof(fp))
            {
               bug("fread_battle_description: End of file was reached");
               exit(0);
            }    
            desc = fread_string(fp);
            if (!str_cmp(desc, "End"))
            {
               x = -1;
               high_value[z][y] = cnt;
               cnt = 0;
               break;
            }
            else
            {
               sprintf((char *) battle_descriptions[z][y][++x], desc);
               cnt++;
            }
         }
      }
   }
   else
   {
      bug("fread_battle_description: The file was not found");
      exit(0);
   }
}        
   
      
void write_portal_file()
{
   FILE *fpout;
   char filename[MIL];
   int num;

   sprintf(filename, "%s%s", KINGDOM_DIR, PORTAL_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (num = 0; num < sysdata.last_portal; num++)
   {
      fprintf(fpout, "Description  %s~\n", portal_show[num]->desc);
      fprintf(fpout, "X            %d\n", portal_show[num]->x);
      fprintf(fpout, "Y            %d\n", portal_show[num]->y);
      fprintf(fpout, "Map          %d\n", portal_show[num]->map);
      fprintf(fpout, "End\n\n");
   }
   fprintf(fpout, "Stop\n");
   fclose(fpout);
}

void save_conquer_file()
{
   FILE *fpout;
   char filename[MIL];
   CONQUER_DATA *conquer;

   sprintf(filename, "%s%s", KINGDOM_DIR, CONQUER_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (conquer = first_conquer; conquer; conquer = conquer->next)
   {
      fprintf(fpout, "#CONQUER\n");
      fprintf(fpout, "AKingdom   %d\n", conquer->akingdom);   
      fprintf(fpout, "RKingdom   %d\n", conquer->rkingdom);
      fprintf(fpout, "Time       %d\n", conquer->time);
      fprintf(fpout, "Town       %s~\n", conquer->ntown);
      fprintf(fpout, "End\n");
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_conquer_data(FILE *fp)
{
   bool fMatch;         
   char *word;
   CONQUER_DATA *conquer;
   
   CREATE(conquer, CONQUER_DATA, 1);
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
            KEY("AKingdom", conquer->akingdom, fread_number(fp));
            break;
            
         case 'R':
            KEY("RKingdom", conquer->rkingdom, fread_number(fp));
            break;
            
         case 'T':
            KEY("Time", conquer->time, fread_number(fp));
            KEY("Town", conquer->ntown, fread_string(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               conquer->occupied = 1;
               LINK(conquer, first_conquer, last_conquer, next, prev);
               return;
            }
      }
   }
}

void load_conquer_file()
{
   FILE *fp;
   char filename[MIL];
   CONQUER_DATA *conquer;
   CONQUER_DATA *conquernext;
   TOWN_DATA *town;
   
   sprintf(filename, "%s%s", KINGDOM_DIR, CONQUER_FILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("Load_conquer_file: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "CONQUER"))
         {
            fread_conquer_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
         {
            for (conquer = first_conquer; conquer; conquer = conquernext)
            {
               conquernext = conquer->next;
               if ((town = get_town(conquer->ntown)) == NULL)
               {
                  bug("Conquer Data %d conquering %d has an invalid name of %s, removing", conquer->akingdom, conquer->rkingdom, conquer->ntown);
                  STRFREE(conquer->ntown);
                  UNLINK(conquer, first_conquer, last_conquer, next, prev);
                  DISPOSE(conquer);
               }
               else
                  conquer->town = town;
            }
            fclose(fp);
            return;
         }
         else
         {
            bug("Load_conquer_file: bad section.", 0);
            continue;
         }
      }
   }
   else
   {
      bug("Cannot open the conquer file", 0);
      return;
   }
}  
void save_trade_file()
{
   FILE *fpout;
   char filename[MIL];
   TRADE_DATA *trade;

   sprintf(filename, "%s%s", KINGDOM_DIR, TRADE_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (trade = first_trade; trade; trade = trade->next)
   {
      fprintf(fpout, "#TRADE\n");
      fprintf(fpout, "OfferKingdom    %d\n", trade->offering_kingdom); 
      fprintf(fpout, "ReceiveKingdom  %d\n", trade->receiving_kingdom);
      fprintf(fpout, "OfferLumber     %d\n", trade->offering_res_tree);
      fprintf(fpout, "OfferCorn       %d\n", trade->offering_res_corn);
      fprintf(fpout, "OfferFish       %d\n", trade->offering_res_fish);
      fprintf(fpout, "OfferGrain      %d\n", trade->offering_res_grain);
      fprintf(fpout, "OfferIron       %d\n", trade->offering_res_iron);
      fprintf(fpout, "OfferGold       %d\n", trade->offering_res_gold);
      fprintf(fpout, "OfferStone      %d\n", trade->offering_res_stone);
      fprintf(fpout, "OfferCoins      %d\n", trade->offering_gold);
      fprintf(fpout, "ReceiveLumber   %d\n", trade->receiving_res_tree);
      fprintf(fpout, "ReceiveCorn     %d\n", trade->receiving_res_corn);
      fprintf(fpout, "ReceiveFish     %d\n", trade->receiving_res_fish);
      fprintf(fpout, "ReceiveGrain    %d\n", trade->receiving_res_grain);
      fprintf(fpout, "ReceiveIron     %d\n", trade->receiving_res_iron);
      fprintf(fpout, "ReceiveGold     %d\n", trade->receiving_res_gold);
      fprintf(fpout, "ReceiveStone    %d\n", trade->receiving_res_stone);
      fprintf(fpout, "ReceiveCoins    %d\n", trade->receiving_gold);
      fprintf(fpout, "OfferRead       %d\n", trade->offering_read); 
      fprintf(fpout, "ReceiveRead     %d\n", trade->receiving_read);
      fprintf(fpout, "Time            %d\n", trade->time); 
      fprintf(fpout, "Posted          %d\n", trade->posted);
      fprintf(fpout, "End\n");
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_trade_data(FILE *fp)
{
   bool fMatch;         
   char *word;
   TRADE_DATA *trade;
   
   CREATE(trade, TRADE_DATA, 1);
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
            
         case 'O':
            KEY("OfferKingdom", trade->offering_kingdom, fread_number(fp));
            KEY("OfferLumber", trade->offering_res_tree, fread_number(fp));
            KEY("OfferCorn", trade->offering_res_corn, fread_number(fp));
            KEY("OfferFish", trade->offering_res_fish, fread_number(fp));
            KEY("OfferGrain", trade->offering_res_grain, fread_number(fp));
            KEY("OfferIron", trade->offering_res_iron, fread_number(fp));
            KEY("OfferGold", trade->offering_res_gold, fread_number(fp));
            KEY("OfferStone", trade->offering_res_stone, fread_number(fp));
            KEY("OfferCoins", trade->offering_gold, fread_number(fp));
            KEY("OfferRead", trade->offering_read, fread_number(fp));
            break;
          
         case 'P':
            KEY("Posted", trade->posted, fread_number(fp));
            break; 
            
         case 'R':
            KEY("ReceiveKingdom", trade->receiving_kingdom, fread_number(fp));
            KEY("ReceiveLumber", trade->receiving_res_tree, fread_number(fp));
            KEY("ReceiveCorn", trade->receiving_res_corn, fread_number(fp));
            KEY("ReceiveFish", trade->receiving_res_fish, fread_number(fp));
            KEY("ReceiveGrain", trade->receiving_res_grain, fread_number(fp));
            KEY("ReceiveIron", trade->receiving_res_iron, fread_number(fp));
            KEY("ReceiveGold", trade->receiving_res_gold, fread_number(fp));
            KEY("ReceiveStone", trade->receiving_res_stone, fread_number(fp));
            KEY("ReceiveRead", trade->receiving_read, fread_number(fp));
            break;
            
         case 'T':
            KEY("Time", trade->time, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(trade, first_trade, last_trade, next, prev);
               return;
            }
      }
   }
}

void load_trade_file()
{
   FILE *fp;
   char filename[MIL];
   
   sprintf(filename, "%s%s", KINGDOM_DIR, TRADE_FILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("Load_kingdom_depo: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "TRADE"))
         {
            fread_trade_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
         {
            fclose(fp);
            return;
         }
         else
         {
            bug("Load_kingdom_depo: bad section.", 0);
            continue;
         }
      }
   }
   else
   {
      bug("Cannot open the trade file", 0);
      return;
   }
}  

void load_default_depo()
{
   int kingdom;
   DEPO_ORE_DATA *dore;
   DEPO_ORE_DATA *ohold;
   DEPO_WEAPON_DATA *dweapon;
   DEPO_WEAPON_DATA *dweap;
   DEPO_WEAPON_DATA *dhold;
   SLAB_DATA *slab;	
   FORGE_DATA *forge;
   
   for (kingdom = 0; kingdom <= sysdata.max_kingdom-1; kingdom++)
   {
      if (kingdom_table[kingdom]->first_ore == NULL) //no depository loaded up yet...
      {
         for (slab = first_slab; slab ; slab = slab->next)
         {
            CREATE(dore, DEPO_ORE_DATA, 1);
            dore->vnum = slab->vnum;
            LINK(dore, kingdom_table[kingdom]->first_ore, kingdom_table[kingdom]->last_ore, next, prev);
            for (forge = first_forge; forge; forge = forge->next)
            {
               CREATE(dweapon, DEPO_WEAPON_DATA, 1);
               dweapon->vnum = forge->vnum;
               LINK(dweapon, dore->first_weapon, dore->last_weapon, next, prev);
            }
         }
      }
      else //make sure the lists match the ore/weapon list...if they don't, remove or create....
      {
         //check the weapon list now.....only need to do one check and then update the whole list if needed....
         for (forge = first_forge; forge; forge = forge->next)
         {
            for (dweapon = kingdom_table[kingdom]->first_ore->first_weapon; dweapon; dweapon = dweapon->next)
            {
               if (forge->vnum == dweapon->vnum)
                  break;
            }
            if (!dweapon) //Create the weapon in all the lists
            {
               for (dore = kingdom_table[kingdom]->first_ore; dore; dore = dore->next)
               {
                  CREATE(dweapon, DEPO_WEAPON_DATA, 1);
                  dweapon->vnum = forge->vnum;
                  LINK(dweapon, dore->first_weapon, dore->last_weapon, next, prev);    
               }
            }
         }
         for (dweapon = kingdom_table[kingdom]->first_ore->first_weapon; dweapon; dweapon = dhold)
         {
            dhold = dweapon->next;
            for (forge = first_forge; forge; forge = forge->next)
            {
               if (forge->vnum == dweapon->vnum)
                  break;
            } 
            if (!forge)
            {
               for (dore = kingdom_table[kingdom]->first_ore; dore; dore = dore->next)
               {
                  for (dweap = dore->first_weapon; dweap; dweap = dweap->next)
                  {
                     if (dweap->vnum == dweapon->vnum)
                     {
                        UNLINK(dweapon, dore->first_weapon, dore->last_weapon, next, prev);
                        DISPOSE(dweapon);
                     }
                  }
               }
            }
         }
         //check the ore lists last....
         for (slab = first_slab; slab; slab = slab->next)
         {
            for (dore = kingdom_table[kingdom]->first_ore; dore; dore = dore->next)
            {
                if (slab->vnum == dore->vnum)
                   break;
            }
            if (!dore) //doesn't exist in depository, create it...
            {
               CREATE(dore, DEPO_ORE_DATA, 1);
               dore->vnum = slab->vnum;
               LINK(dore, kingdom_table[kingdom]->first_ore, kingdom_table[kingdom]->last_ore, next, prev);  
               for (forge = first_forge; forge; forge = forge->next)
               {
                  CREATE(dweapon, DEPO_WEAPON_DATA, 1);
                  dweapon->vnum = forge->vnum;
                  LINK(dweapon, dore->first_weapon, dore->last_weapon, next, prev);
               }   
            }
         }
         for (dore = kingdom_table[kingdom]->first_ore; dore; dore = ohold)
         {
            ohold = dore->next;
            for (slab = first_slab; slab; slab = slab->next)
            {
               if (slab->vnum == dore->vnum)
                  break;
            }
            if (!slab) //doesn't exist in slab list, delete it....
            {
               UNLINK(dore, kingdom_table[kingdom]->first_ore, kingdom_table[kingdom]->last_ore, next, prev);
               for (dweapon = dore->first_weapon; dweapon; dweapon = dweapon->next)
               {
                  UNLINK(dweapon, dore->first_weapon, dore->last_weapon, next, prev);
                  DISPOSE(dweapon);   
               }
               DISPOSE(dore); 
            }
         }
      }                    
   }
}    

void write_depo_list()
{
   FILE *fpout;
   DEPO_ORE_DATA *dore;
   DEPO_WEAPON_DATA *dweapon;
   char buf[MSL];
   char filename[MIL];
   int x;  
   
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      sprintf(filename, "%s%s.depo", KINGDOM_DIR, kingdom_table[x]->name);
      if ((fpout = fopen(filename, "w")) == NULL)
      {
         sprintf(buf, "Cannot open: %s for writing", filename);
         bug(buf, 0);
         return;
      }
      for (dore = kingdom_table[x]->first_ore; dore; dore = dore->next)
      {
         fprintf(fpout, "#ORE\n");
         fprintf(fpout, "Vnum       %d\n", dore->vnum);
         fprintf(fpout, "Count      %d\n", dore->count);
         fprintf(fpout, "End\n");
         for (dweapon = dore->first_weapon; dweapon; dweapon = dweapon->next)
         {
            fprintf(fpout, "#WEAPON\n");
            fprintf(fpout, "Vnum       %d\n", dweapon->vnum);
            fprintf(fpout, "Count      %d\n", dweapon->count);
            fprintf(fpout, "End\n");
         }
         fprintf(fpout, "#NEXT\n");
      }         
      fprintf(fpout, "#END\n");
      fclose(fpout);
   }
   return; 
}

void fread_depoweapon_data(FILE *fp, DEPO_ORE_DATA *dore)
{
   bool fMatch;         
   char *word;
   int vnum = 0;
   int cnt = 0;
   DEPO_WEAPON_DATA *dweapon;
   FORGE_DATA *forge;
   
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
            KEY("Count", cnt, fread_number(fp));
            break;
            
         case 'V':
            KEY("Vnum", vnum, fread_number(fp));
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
            {
               for (forge = first_forge; forge; forge = forge->next)
               {
                  if (forge->vnum == vnum)
                  {
                     CREATE(dweapon, DEPO_WEAPON_DATA, 1);
                     dweapon->vnum = forge->vnum;
                     dweapon->count = cnt;
                     LINK(dweapon, dore->first_weapon, dore->last_weapon, next, prev);
                     break;
                  }
               }
               return;
            }
      }
   }
}
   
void fread_depoore_data(FILE *fp, int kingdom)
{
   bool fMatch;
   char letter;         
   char *word;
   int vnum = 0;
   char buf[MSL];
   int cnt = 0;
   DEPO_ORE_DATA *dore = NULL;
   SLAB_DATA *slab;	
   
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
            KEY("Count", cnt, fread_number(fp));
            break;
            
         case 'V':
            KEY("Vnum", vnum, fread_number(fp));
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
            {
               for (slab = first_slab; slab ; slab = slab->next)
               {
                  if (slab->vnum == vnum)
                  {
                     CREATE(dore, DEPO_ORE_DATA, 1);
                     dore->vnum = slab->vnum;
                     dore->count = cnt;
                     LINK(dore, kingdom_table[kingdom]->first_ore, kingdom_table[kingdom]->last_ore, next, prev);
                     break;
                  }
               }
               if (!slab)
               {
                  for(;;)
                  {
                     word = fread_word(fp);
                     if (!str_cmp(word, "#ORE"))
                     {
                        fread_depoore_data(fp, kingdom);
                        return;
                     }
                     if (feof(fp))
                        return;
                  }
               }
               else
               {
                  for(;;)
                  {
                     letter = fread_letter(fp);
                     if (letter == '*')
                     {
                        fread_to_eol(fp);
                        continue;
                     }
                     if (letter != '#')
                     {
                        bug("Load_kingdom_depo: # not found.", 0);
                        break;
                     }

                     word = fread_word(fp);
                     if (!str_cmp(word, "WEAPON"))
                     {
                        fread_depoweapon_data(fp, dore);
                        continue;
                     }
                     else if (!str_cmp(word, "NEXT"))
                     {
                        //done break now
                        return;
                     }
                  }
               }
               break;       
            }
         }
         if (!fMatch)
         {
            sprintf(buf, "fread_deopore_data: no match: %s", word);
            bug(buf, 0);
         }
   }
}     
                   
void load_kingdom_depo()
{
   FILE *fp;
   char filename[MIL];
   int x;
   
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      sprintf(filename, "%s%s.depo", KINGDOM_DIR, kingdom_table[x]->name);
      if ((fp = fopen(filename, "r")) != NULL)
      {
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
               bug("Load_kingdom_depo: # not found.", 0);
               break;
            }

            word = fread_word(fp);
            if (!str_cmp(word, "ORE"))
            {
               fread_depoore_data(fp, x);
               continue;
            }
            else if (!str_cmp(word, "END"))
            {
               break;
            }
            else
            {
               bug("Load_kingdom_depo: bad section.", 0);
               continue;
            }
         }
      }
      else
      {
         bug("Cannot open a kingdom depo file", 0);
         continue;
      }
      fclose(fp);
   }
}  

int get_control_size(int size)
{
   switch(size)
   {
      case 1:
         return 3;
      case 2:
         return 4;
      case 3:
         return 6;
      case 4:
         return 7;
      case 5:
         return 9;
      case 6:
         return 11;
      case 7:
         return 13;
      case 8:
         return 15;
      case 9:
         return 18;
      case 10:
         return 25;
      default:
         return 3;
   }
}  

//Loads town data out of the kingdom
void load_town_data(KINGDOM_DATA *kingdom, FILE *fp)
{
   char *word;
   char buf[MSL];
   bool fMatch;
   int x, y, map;
   int size;
   TOWN_DATA *town;
   DOOR_DATA *ddata = NULL;
   DOOR_LIST *dlist;
   SCHEDULE_DATA *schedule;
   
   CREATE(town, TOWN_DATA, 1);
   
   //I love it how a random 1 just appears in things....
   for (x = 0; x <= 59; x++)
   {
      for (y = 0; y <= 59; y++)
      {
         town->usedpoint[x][y] = 0;
      }
   }

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
            
         case '#':
            if (!strcmp(word, "#BANK")) 
            {
               globaltownload = 1;
               globaltownptr = town;
               fread_obj(NULL, fp, OS_BANK);
               globaltownload = 0;
               globaltownptr = NULL;
               fMatch = TRUE;
            }
            
         case 'A':
            KEY("Allowexpansions", town->allowexpansions, fread_number(fp));
            break;
            
         case 'B':  
            KEY("Banksize", town->banksize, fread_number(fp));
            KEY("Balance", town->balance, fread_number(fp));        
            if (!str_cmp(word, "Barracks"))
            {
               town->barracks[0] = fread_number(fp);
               town->barracks[1] = fread_number(fp);
               town->barracks[2] = fread_number(fp);
               fMatch = TRUE;
            }
            if (!str_cmp(word, "BinCoords"))
            {
               for (y = 0; y <= 59; y++)
               {
                  for (x = 0; x <= 59; x++)
                  {
                     town->bincoords[x][y] = fread_number(fp);
                  }
               }
               fMatch = TRUE;
               break;
            }
            break;           
         case 'C':
            KEY("Corn", town->corn, fread_number(fp));
            KEY("Coinconsump", town->coinconsump, fread_number(fp));
            KEY("Coins", town->coins, fread_number(fp));
            if (!str_cmp(word, "Coords"))
            {
               char *string;
               x = fread_number(fp);
               town->roomcoords[x][0] = fread_number(fp);
               town->roomcoords[x][1] = fread_number(fp);
               town->roomcoords[x][2] = fread_number(fp);
               kingdom_sector[town->roomcoords[x][2]][town->roomcoords[x][0]][town->roomcoords[x][1]] = town->kingdom;
               town->roomflags[x] = fread_bitvector(fp);
               string = fread_string(fp);
               sprintf(town->roomtitles[x], string);
               fMatch = TRUE;
               break;
            }
            KEY("CTax", town->ctax, fread_number(fp));
            break;
         case 'F':
            KEY("Fish", town->fish, fread_number(fp));
            KEY("Foodconsump", town->foodconsump, fread_number(fp));
            break;
         case 'L':
            KEY("Lumber", town->lumber, fread_number(fp));
            KEY("Lumberconsump", town->lumberconsump, fread_number(fp));
            KEY("LastTaxChange", town->lasttaxchange, fread_number(fp));
            break;
         case 'S':
            KEY("Salestax", town->salestax, fread_number(fp));
            if (!str_cmp(word, "Schedule"))
            {
               CREATE(schedule, SCHEDULE_DATA, 1);
               schedule->start_period = fread_number(fp);
               schedule->end_period = fread_number(fp);
               schedule->resource = fread_number(fp);
               schedule->reoccur = fread_number(fp);
               schedule->ran = fread_number(fp);
               schedule->x = fread_number(fp);
               schedule->y = fread_number(fp);
               schedule->map = fread_number(fp);
               LINK(schedule, town->first_schedule, town->last_schedule, next, prev);
               fMatch = TRUE;
               break;
            }
            KEY("Size", town->size, fread_number(fp));
            KEY("Startx", town->startx, fread_number(fp));
            KEY("Starty", town->starty, fread_number(fp));
            KEY("Startmap", town->startmap, fread_number(fp));
            KEY("Stone", town->stone, fread_number(fp));
            KEY("Stoneconsump", town->stoneconsump, fread_number(fp));
            break;      
         case 'N':
            KEY("Name", town->name, fread_string(fp));
            break;
         case 'H':
            KEY("Hold", town->hold, fread_number(fp));
            break;
         case 'U':
            if (!str_cmp(word, "UsedPoint"))
            {
               for (y = 0; y <= 59; y++)
               {
                  for (x = 0; x <= 59; x++)
                  {
                     town->usedpoint[x][y] = fread_number(fp);
                  }
               }
               fMatch = TRUE;
               break;
            }
            KEY("Units", town->units, fread_number(fp));
            KEY("Unitstraining", town->unitstraining, fread_number(fp));
            break;
            
         case 'M':
            KEY("Mayor", town->mayor, fread_string(fp));
            KEY("MaxDvalue", town->max_dvalue, fread_number(fp));
            KEY("MaxSize", town->maxsize, fread_number(fp));
            KEY("MinHAppoint", town->minhappoint, fread_number(fp));
            KEY("MinWithdraw", town->minwithdraw, fread_number(fp));
            KEY("Moral", town->moral, fread_number(fp));
            KEY("Month", town->month, fread_number(fp));
            break;
         case 'K':
            if (!str_cmp(word, "Key"))
            {
               KEY_DATA *key;
               CREATE(key, KEY_DATA, 1);
               LINK(key, town->first_key, town->last_key, next, prev);
               for (;;)
               {
                  word = feof(fp) ? "End" : fread_word(fp);
                  if (!str_cmp(word, "End"))
                  {
                     bug("%s in kingdom %d did not load properly, looped in key!", town->name, town->kingdom);
                     break;
                  }
                  if (!str_cmp(word, "ENDKEY"))
                     break; 
                  if (!str_cmp(word, "Flag"))
                  {
                     key->flag = fread_number(fp);
                     continue;
                  }
                  if (!str_cmp(word, "Name"))
                  {
                     key->name = fread_string(fp);
                     continue;
                  }
               }
               fMatch = TRUE;
               break;
            }   
            KEY("Kpid", town->kpid, fread_number(fp));
            KEY("Kingdom", town->kingdom, fread_number(fp));
            break;
         case 'G':
            KEY("Grain", town->grain, fread_number(fp));
            KEY("Growth", town->growth, fread_number(fp));
            KEY("Growthcheck", town->growthcheck, fread_number(fp));
            KEY("Gold", town->gold, fread_number(fp));
            break;
         case 'I':
            KEY("Iron", town->iron, fread_number(fp));
            break;
         
         case 'R':
            if (!str_cmp(word, "Recall"))
            {
               town->recall[0] = fread_number(fp);
               town->recall[1] = fread_number(fp);
               town->recall[2] = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            KEY("Rooms", town->rooms, fread_number(fp));
            break;
         case 'D':
            if (!str_cmp(word, "Death"))
            {
               town->death[0] = fread_number(fp);
               town->death[1] = fread_number(fp);
               town->death[2] = fread_number(fp);
               fMatch = TRUE;
               break;
            }
      
            if (!str_cmp(word, "Doordata"))
            {
               CREATE(dlist, DOOR_LIST, 1);
               LINK(dlist, town->first_doorlist, town->last_doorlist, next, prev);
               for (;;)
               {
                  word = feof(fp) ? "End" : fread_word(fp);
                  if (!str_cmp(word, "End"))
                  {
                     bug("%s in kingdom %d did not load properly, looped in doordata!", town->name, town->kingdom);
                     break;
                  }
                  if (!str_cmp(word, "Enddoor"))
                     break;
                  if (!str_cmp(word, "DCoords"))
                  {
                     x = fread_number(fp);
                     ddata->roomcoordx[x] = fread_number(fp);  
                     ddata->roomcoordy[x] = fread_number(fp); 
                     ddata->roomcoordmap[x] = fread_number(fp); 
                     continue;
                  }
                  if (!str_cmp(word, "Doorvalue"))
                  {
                     CREATE(ddata, DOOR_DATA, 1);
                     LINK(ddata, dlist->first_door, dlist->last_door, next, prev);
                     ddata->doorvalue[0] = fread_number(fp);
                     ddata->doorvalue[1] = fread_number(fp);
                     ddata->doorvalue[2] = fread_number(fp);
                     ddata->doorvalue[3] = fread_number(fp);
                     ddata->doorvalue[4] = fread_number(fp);
                     ddata->doorvalue[5] = fread_number(fp);
                     ddata->doorvalue[6] = fread_number(fp);
                     ddata->doorvalue[7] = fread_number(fp);
                     ddata->doorvalue[8] = fread_number(fp);
                     ddata->doorvalue[9] = fread_number(fp);
                     continue;
                  }       
                  if (!str_cmp(word, "Cansee"))
                  {
                     ddata->cansee = fread_number(fp);
                     continue;
                  }
               }
               fMatch = TRUE;
               break;
            }
               
            if (!str_cmp(word, "Doorstate"))
            {
               x = fread_number(fp);
               town->doorstate[0][x] = fread_number(fp);
               town->doorstate[1][x] = fread_number(fp);
               town->doorstate[2][x] = fread_number(fp);
               town->doorstate[3][x] = fread_number(fp);
               town->doorstate[4][x] = fread_number(fp);
               town->doorstate[5][x] = fread_number(fp);
               town->doorstate[6][x] = fread_number(fp);
               town->doorstate[7][x] = fread_number(fp);
               //Map doesn't save when a door is changed, this is to save processing for
               //slower machines and to keep the doors in sync by checking here...
               if (town->doorstate[5][x] > -1 && town->doorstate[6][x] > -1 && town->doorstate[7][x] > -1)
               {
                  if (town->doorstate[0][x] == 0)
                     map_sector[town->doorstate[7][x]][town->doorstate[5][x]][town->doorstate[6][x]] = SECT_DOOR;
                  if (town->doorstate[0][x] == 1)
                     map_sector[town->doorstate[7][x]][town->doorstate[5][x]][town->doorstate[6][x]] = SECT_CDOOR;
                  if (town->doorstate[0][x] == 2)
                     map_sector[town->doorstate[7][x]][town->doorstate[5][x]][town->doorstate[6][x]] = SECT_LDOOR;
               }
               fMatch = TRUE;
               break;
            }
         case 'P':
            KEY("Poptax", town->poptax, fread_number(fp));
            break;
         
         case 'T':
            KEY("TPid", town->tpid, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(town, kingdom->first_town, kingdom->last_town, next, prev);
               map = town->roomcoords[1][2];
               size = get_control_size(town->size);
               for (x = town->startx - size; x <= town->startx+size; x++)
               {
                  for (y = town->starty - size; y <= town->starty+size; y++)
                  {
                     kingdom_sector[map][x][y] = town->kingdom;
                  }
               }     
               return;         
            }
            KEY("Expansions", town->expansions, fread_number(fp));
      }
      if (!fMatch)
      {
         sprintf(buf, "load_town_data: no match: %s", word);
         bug(buf, 0);
      }
   }
   return;
}

     
/* First attempt to add a structure.  Will replace the old hometown code and
   will allowing for editing online...Spiffy -- Xerves two days before Christmas 99 */
bool load_kingdom_file(char *fname)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   int kpeace[MAX_KINGDOM];
   INTRO_DATA *intro;
   struct kingdom_data *kingdom;
   int x;
   FILE *fp;
   int x1, x2, x3, x4;
   char *ln;
   int toss;

   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      kpeace[x] = 0;
   }

   sprintf(buf, "%s%s", KINGDOM_DIR, fname);
   if ((fp = fopen(buf, "r")) == NULL)
   {
      perror(buf);
      return FALSE;
   }

   CREATE(kingdom, struct kingdom_data, 1);
   kingdom->num = -1;
   kingdom->first_introduction = NULL;
   kingdom->last_introduction = NULL;
   kingdom->first_town = NULL;
   kingdom->last_town = NULL;

   //Init the que to 0
   for (x = 0; x <= 25; x++)
   {
      kingdom->mob_que[x] = 0;
      kingdom->obj_que[x] = 0;
   }
   for (x = 0; x < sysdata.max_kingdom; x++)
      kingdom->cpeace[x] = -1;

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
            KEY("AllowJoin", kingdom->allowjoin, fread_number(fp));
            break;

         case 'B':
            KEY("BVisitorTax", kingdom->bvisitor, fread_number(fp));
            break;

         case 'C':
            if (!str_cmp(word, "CPeace"))
            {
               ln = fread_line(fp);
               x1 = x2 = 0;
               sscanf(ln, "%d %d", &x1, &x2);
               kingdom->cpeace[x1] = x2;
               fMatch = TRUE;
            }
            KEY("CTax", kingdom->ctax, fread_number(fp));
            break;

         case 'D':
            KEY("DTown", kingdom->dtown, fread_string(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               fclose(fp);
               if (kingdom->num < 0 || kingdom->num > sysdata.max_kingdom)
               {
                  sprintf(buf, "Load_kingdom_file: Kingdom (%s) bad/not found (%d)", kingdom->name ? kingdom->name : "name not found", kingdom->num);
                  bug(buf, 0);
                  if (kingdom->name)
                     STRFREE(kingdom->name);
                  DISPOSE(kingdom);
                  return FALSE;
               }

               //Add default of Neutral when first loaded, or new kingdom arrives
               for (x = 0; x < sysdata.max_kingdom; x++)
               {
                  if (kpeace[x] == 0)
                     kingdom->peace[x] = 1;
               }
               kingdom_table[kingdom->num] = kingdom;
               return TRUE;
            }
         case 'G':
            //Are no longer used, kept in for support for old files
            KEY("Gold", toss, fread_number(fp));
            break;
            
         case 'I':
            if (!str_cmp(word, "Intro"))
            {
               ln = fread_line(fp);
               x1 = x2 = x3 = 0;
               sscanf(ln, "%d %d %d %d", &x1, &x2, &x3, &x4);
               CREATE(intro, INTRO_DATA, 1);
               intro->pid = x1;
               intro->flags = x2;
               intro->value = x3;
               intro->lastseen = x4;
               LINK(intro, kingdom->first_introduction, kingdom->last_introduction, next, prev);
               fMatch = TRUE;
            }
            break;

         case 'L':
            KEY("LastIntroCheck", kingdom->lastintrocheck, fread_number(fp));
            KEY("LastTax", kingdom->lasttaxchange, fread_number(fp));
            KEY("LogFile", kingdom->logfile, fread_string(fp));
            KEY("Logsettings", kingdom->logsettings, fread_bitvector(fp));
            break;

         case 'K':
            KEY("Kingdom", kingdom->num, fread_number(fp));
            KEY("KPid", kingdom->kpid, fread_number(fp));
            break;

         case 'M':
            KEY("MaxLineLog", kingdom->maxlinelog, fread_number(fp));
            KEY("MaxTimeLog", kingdom->maxtimelog, fread_number(fp));
            KEY("MinBuild", kingdom->minbuild, fread_number(fp));
            KEY("MinDepository", kingdom->mindepository, fread_number(fp));
            KEY("MinPlace", kingdom->minplace, fread_number(fp));
            KEY("MinReadLog", kingdom->minreadlog, fread_number(fp));
            KEY("MinCommand", kingdom->mincommand, fread_number(fp));
            KEY("MinAppoint", kingdom->minappoint, fread_number(fp));
            KEY("MinHAppoint", kingdom->minhappoint, fread_number(fp));
            KEY("MinWithdraw", kingdom->minwithdraw, fread_number(fp));
            KEY("MinTrainerTax", kingdom->mintrainertax, fread_number(fp));
            KEY("MinSwitchTown", kingdom->minswitchtown, fread_number(fp));
            KEY("MinBookTax", kingdom->minbooktax, fread_number(fp));
            KEY("MinLogSettings", kingdom->minlogsettings, fread_number(fp));
            KEY("MinGeneral", kingdom->mingeneral, fread_number(fp));
            break;

         case 'N':
            KEY("Name", kingdom->name, fread_string(fp));
            KEY("Number1", kingdom->number1, fread_string(fp));
            KEY("Number2", kingdom->number2, fread_string(fp));
            break;

         case 'P':
            if (!str_cmp(word, "Peace"))
            {
               ln = fread_line(fp);
               x1 = x2 = 0;
               sscanf(ln, "%d %d", &x1, &x2);
               kingdom->peace[x1] = x2;
               kpeace[x1] = 1;
               fMatch = TRUE;
            }
            KEY("PopTax", kingdom->poptax, fread_number(fp));
            break;

         case 'R':
            KEY("Race", kingdom->race, fread_number(fp));
            KEY("Raceset", kingdom->raceset, fread_number(fp));
            //Are no longer used, kept in for support for old files
            KEY("ResGold", toss, fread_number(fp));
            KEY("ResIron", toss, fread_number(fp));
            KEY("ResTree", toss, fread_number(fp));
            KEY("ResCorn", toss, fread_number(fp));
            KEY("ResGrain", toss, fread_number(fp));
            KEY("ResStone", toss, fread_number(fp));
            KEY("Ruler", kingdom->ruler, fread_string(fp));
            break;

         case 'S':
            KEY("SalesTax", kingdom->salestax, fread_number(fp));
            break;

         case 'T':
            if (!str_cmp(word, "Towndata"))
            {
               load_town_data(kingdom, fp);
               fMatch = TRUE;
               break;
            }
            KEY("TaxCorn", kingdom->corn_tax, fread_number(fp));
            KEY("TaxFish", kingdom->fish_tax, fread_number(fp));
            KEY("TaxGrain", kingdom->grain_tax, fread_number(fp));
            KEY("TaxTree", kingdom->tree_tax, fread_number(fp));
            KEY("TaxIron", kingdom->iron_tax, fread_number(fp));
            KEY("TaxGold", kingdom->gold_tax, fread_number(fp));
            KEY("TaxStone", kingdom->stone_tax, fread_number(fp));
            KEY("Tier1Tax", kingdom->tier1, fread_number(fp));
            KEY("Tier2Tax", kingdom->tier2, fread_number(fp));
            KEY("Tier3Tax", kingdom->tier3, fread_number(fp));
            KEY("Tier4Tax", kingdom->tier4, fread_number(fp));
            KEY("TVisitorTax", kingdom->tvisitor, fread_number(fp));
            KEY("Tier1TaxBook", kingdom->tier1book, fread_number(fp));
            KEY("Tier2TaxBook", kingdom->tier2book, fread_number(fp));
            KEY("Tier3TaxBook", kingdom->tier3book, fread_number(fp));
            KEY("Tier4TaxBook", kingdom->tier4book, fread_number(fp));
            break;

      }
      if (!fMatch)
      {
         sprintf(buf, "load_kingdom_file: no match: %s", word);
         bug(buf, 0);
      }
   }
   return FALSE;
}

char *parse_save_file(char *name)
{
   static char pname[100];
   int x = 0;
   
   for (;;)
   {
      if (*name == '\0')
      {
         pname[x] = *name;
         break;
      }
      if ((*name >= 65 && *name <= 90) || (*name >= 97 && *name <= 122))
         pname[x++] = *name++;
      else
         name++;
   }
   return &pname[0];
}   

void write_kingdom_list()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   int x;

   sprintf(filename, "%s%s", KINGDOM_DIR, KINGDOM_LIST);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   for (x = 0; x < sysdata.max_kingdom; x++)
      fprintf(fpout, "%s.kingdom\n", parse_save_file(kingdom_table[x]->name));
   fprintf(fpout, "$");
   fclose(fpout);
   return;
}

void write_kingdom_file(int cl)
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   int x, y;
   struct kingdom_data *kingdom = kingdom_table[cl];
   TOWN_DATA *town;
   INTRO_DATA *intro;
   DOOR_LIST *dlist;
   DOOR_DATA *ddata;
   SCHEDULE_DATA *schedule;

   sprintf(filename, "%s%s.kingdom", KINGDOM_DIR, parse_save_file(kingdom->name));
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   fprintf(fpout, "Kingdom     %d\n", cl);
   fprintf(fpout, "KPid        %d\n", kingdom->kpid);
   fprintf(fpout, "Name        %s~\n", kingdom->name);
   if (kingdom->logfile)
      fprintf(fpout, "LogFile     %s~\n", kingdom->logfile);
   else
   {
      sprintf(buf, "%s%slog.txt", KINGDOM_DIR, kingdom->name);
      kingdom->logfile = STRALLOC(buf);
      fprintf(fpout, "LogFile     %s~\n", kingdom->logfile);
   }
   if (kingdom->dtown)
      fprintf(fpout, "DTown           %s~\n", kingdom->dtown);
   /*  --Removed
   fprintf(fpout, "ResGold         %d\n", kingdom->res_gold);
   fprintf(fpout, "ResIron         %d\n", kingdom->res_iron);
   fprintf(fpout, "ResStone        %d\n", kingdom->res_stone);
   fprintf(fpout, "ResCorn         %d\n", kingdom->res_corn);
   fprintf(fpout, "ResGrain        %d\n", kingdom->res_grain);
   fprintf(fpout, "ResTree         %d\n", kingdom->res_tree);
   */
   fprintf(fpout, "TaxTree         %d\n", kingdom->tree_tax);
   fprintf(fpout, "TaxGrain        %d\n", kingdom->grain_tax);
   fprintf(fpout, "TaxCorn         %d\n", kingdom->corn_tax);
   fprintf(fpout, "TaxFish         %d\n", kingdom->fish_tax);
   fprintf(fpout, "TaxStone        %d\n", kingdom->stone_tax);
   fprintf(fpout, "TaxIron         %d\n", kingdom->iron_tax);
   fprintf(fpout, "TaxGold         %d\n", kingdom->gold_tax);
   //fprintf(fpout, "Gold            %d\n", kingdom->gold);
   fprintf(fpout, "MinBuild        %d\n", kingdom->minbuild);
   fprintf(fpout, "MinPlace        %d\n", kingdom->minplace);
   fprintf(fpout, "MinAppoint      %d\n", kingdom->minappoint);
   fprintf(fpout, "MinHAppoint     %d\n", kingdom->minhappoint);
   fprintf(fpout, "MinWithdraw     %d\n", kingdom->minwithdraw);
   fprintf(fpout, "MinReadLog      %d\n", kingdom->minreadlog);
   fprintf(fpout, "MinLogSettings  %d\n", kingdom->minlogsettings);
   fprintf(fpout, "MinCommand      %d\n", kingdom->mincommand);
   fprintf(fpout, "MinDepository   %d\n", kingdom->mindepository);
   fprintf(fpout, "MinTrainerTax   %d\n", kingdom->mintrainertax);
   fprintf(fpout, "MinBookTax      %d\n", kingdom->minbooktax);
   fprintf(fpout, "MinSwitchTown   %d\n", kingdom->minswitchtown);
   fprintf(fpout, "MinGeneral      %d\n", kingdom->mingeneral);
   fprintf(fpout, "AllowJoin       %d\n", kingdom->allowjoin);
   fprintf(fpout, "SalesTax        %d\n", kingdom->salestax);
   fprintf(fpout, "PopTax          %d\n", kingdom->poptax);
   fprintf(fpout, "CTax            %d\n", kingdom->ctax);
   fprintf(fpout, "Ruler	       %s~\n", kingdom->ruler);
   fprintf(fpout, "Race            %d\n", kingdom->race);
   fprintf(fpout, "Raceset         %d\n", kingdom->raceset);
   fprintf(fpout, "LastTax         %d\n", kingdom->lasttaxchange);
   fprintf(fpout, "Tier1Tax        %d\n", kingdom->tier1);
   fprintf(fpout, "Tier2Tax        %d\n", kingdom->tier2);
   fprintf(fpout, "Tier3Tax        %d\n", kingdom->tier3);
   fprintf(fpout, "Tier4Tax        %d\n", kingdom->tier4);
   fprintf(fpout, "TVisitorTax     %d\n", kingdom->tvisitor);
   fprintf(fpout, "Tier1TaxBook    %d\n", kingdom->tier1book);
   fprintf(fpout, "Tier2TaxBook    %d\n", kingdom->tier2book);
   fprintf(fpout, "Tier3TaxBook    %d\n", kingdom->tier3book);
   fprintf(fpout, "Tier4TaxBook    %d\n", kingdom->tier4book);
   fprintf(fpout, "BVisitorTax     %d\n", kingdom->bvisitor);
   fprintf(fpout, "MaxLineLog      %d\n", kingdom->maxlinelog);
   fprintf(fpout, "MaxTimeLog      %d\n", kingdom->maxtimelog);
   
   if (kingdom->number1)
      fprintf(fpout, "Number1         %s~\n", kingdom->number1);
   if (kingdom->number2)
      fprintf(fpout, "Number2         %s~\n", kingdom->number2);
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      fprintf(fpout, "Peace           %d %d\n", x, kingdom->peace[x]);
   }
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      fprintf(fpout, "CPeace          %d %d\n", x, kingdom->cpeace[x]);
   }
   fprintf(fpout, "Logsettings     %s\n", print_bitvector(&kingdom->logsettings));  
   fprintf(fpout, "LastIntroCheck  %d\n", kingdom->lastintrocheck);
   for (intro = kingdom->first_introduction; intro; intro = intro->next)
   {
      fprintf(fpout, "Intro %d %d %d %d\n", intro->pid, intro->flags, intro->value, intro->lastseen);
   }
   for (town = kingdom->first_town; town; town = town->next)
   {
      fprintf(fpout, "TownData\n");
      fprintf(fpout, "Name        %s~\n", town->name);
      fprintf(fpout, "Mayor       %s~\n", town->mayor);
      fprintf(fpout, "Kpid        %d\n",  town->kpid);
      fprintf(fpout, "Tpid        %d\n",  town->tpid);
      fprintf(fpout, "Kingdom     %d\n",  town->kingdom);
      fprintf(fpout, "Corn        %d\n",  town->corn);
      fprintf(fpout, "Fish        %d\n",  town->fish);
      fprintf(fpout, "Grain       %d\n",  town->grain);
      fprintf(fpout, "Gold        %d\n",  town->gold);
      fprintf(fpout, "Iron        %d\n",  town->iron);
      fprintf(fpout, "Lumber      %d\n",  town->lumber);
      fprintf(fpout, "Coins       %d\n",  town->coins);
      fprintf(fpout, "Stone       %d\n",  town->stone);
      fprintf(fpout, "Hold        %d\n",  town->hold);
      fprintf(fpout, "Barracks    %d %d %d\n", town->barracks[0], town->barracks[1], town->barracks[2]);
      fprintf(fpout, "Recall      %d %d %d\n", town->recall[0], town->recall[1], town->recall[2]);
      fprintf(fpout, "Death       %d %d %d\n", town->death[0], town->death[1], town->death[2]);
      fprintf(fpout, "Startx      %d\n", town->startx);
      fprintf(fpout, "Starty      %d\n", town->starty);
      fprintf(fpout, "Startmap    %d\n", town->startmap);
      fprintf(fpout, "Poptax      %d\n", town->poptax);
      fprintf(fpout, "CTax        %d\n", town->ctax);
      fprintf(fpout, "Salestax    %d\n", town->salestax);
      fprintf(fpout, "Units       %d\n", town->units);
      fprintf(fpout, "Unitstraining %d\n", town->unitstraining);
      fprintf(fpout, "Size        %d\n", town->size);
      fprintf(fpout, "Rooms       %d\n", town->rooms);
      fprintf(fpout, "MaxSize     %d\n", town->maxsize);
      fprintf(fpout, "AllowExpansions %d\n", town->allowexpansions);
      fprintf(fpout, "Expansions  %d\n", town->expansions);
      fprintf(fpout, "MinHAppoint %d\n", town->minhappoint);
      fprintf(fpout, "MinWithdraw %d\n", town->minwithdraw);
      fprintf(fpout, "LastTaxChange %d\n", town->lasttaxchange);
      fprintf(fpout, "Moral       %d\n", town->moral);
      fprintf(fpout, "Month       %d\n", town->month);
      fprintf(fpout, "Growth      %d\n", town->growth);
      fprintf(fpout, "Growthcheck %d\n", town->growthcheck);
      fprintf(fpout, "Foodconsump %d\n", town->foodconsump);
      fprintf(fpout, "Stoneconsump %d\n", town->stoneconsump);
      fprintf(fpout, "Lumberconsump %d\n", town->lumberconsump);
      fprintf(fpout, "Coinconsump %d\n", town->coinconsump);
      fprintf(fpout, "Banksize    %d\n", town->banksize);
      fprintf(fpout, "Balance     %d\n", town->balance);      
      for (schedule = town->first_schedule; schedule; schedule = schedule->next)
      {
         fprintf(fpout, "Schedule %d %d %d %d %d %d %d %d\n", schedule->start_period, schedule->end_period,
            schedule->resource, schedule->reoccur, schedule->ran, schedule->x, schedule->y, schedule->map);
      }
      fprintf(fpout, "BinCoords  ");
      for (y = 0; y <= 59; y++)
      {
          if (y > 0) //Cosmetic thing, ha ha they must line up!
             fprintf(fpout, "           ");
          for (x = 0; x <= 59; x++)
          {
            fprintf(fpout, " %d", town->bincoords[x][y]); 
          }
          fprintf(fpout, "\n");
      }
      for (x = 1; x <= 150; x++)
      {
         if (town->roomcoords[x][1] > 0)
         {
            fprintf(fpout, "Coords       %d %d %d %d %s %s~\n", x, town->roomcoords[x][0], town->roomcoords[x][1], town->roomcoords[x][2], 
                                                                print_bitvector(&town->roomflags[x]), town->roomtitles[x]);
         }
      }
      for (x = 0; x <= 99; x++)
      {
         if (town->doorstate[4][x] > 0)
            fprintf(fpout, "Doorstate   %d %d %d %d %d %d %d %d %d\n", x, town->doorstate[0][x], town->doorstate[1][x],
               town->doorstate[2][x], town->doorstate[3][x], town->doorstate[4][x], town->doorstate[5][x],
               town->doorstate[6][x], town->doorstate[7][x]);
      }
      fprintf(fpout, "UsedPoint  ");
      for (y = 0; y <= 59; y++)
      {
          if (y > 0) //Cosmetic thing, ha ha they must line up!
             fprintf(fpout, "           ");
          for (x = 0; x <= 59; x++)
          {
            fprintf(fpout, " %d", town->usedpoint[x][y]); 
          }
          fprintf(fpout, "\n");
      }
      fprintf(fpout, "MaxDvalue   %d\n", town->max_dvalue);
      if (town->first_doorlist)
      {
         for (dlist = town->first_doorlist; dlist; dlist = dlist->next)
         {
            fprintf(fpout, "DOORDATA\n");
            for (ddata = dlist->first_door; ddata; ddata = ddata->next)
            {
               //Doorvalue has to be first, the loading routine creates ddata when it finds it, so this has
               //to be the first entry here!!!!!
               fprintf(fpout, "Doorvalue    %d %d %d %d %d %d %d %d %d %d\n", ddata->doorvalue[0], ddata->doorvalue[1], 
                  ddata->doorvalue[2], ddata->doorvalue[3], ddata->doorvalue[4], ddata->doorvalue[5], 
                  ddata->doorvalue[6], ddata->doorvalue[7], ddata->doorvalue[8], ddata->doorvalue[9]);
               for (x = 0; x <= MAX_HPOINTS-1; x++)   
               {
                  if (ddata->roomcoordx[x])
                  {
                     fprintf(fpout, "DCoords    %d %d %d %d\n", x, ddata->roomcoordx[x], ddata->roomcoordy[x],
                        ddata->roomcoordmap[x]);
                  }
                  else
                     break;
               }
               fprintf(fpout, "Cansee      %d\n", ddata->cansee);
            }
            fprintf(fpout, "ENDDOOR\n");
         }
      }      
      if (town->first_key)
      {
         KEY_DATA *key;
         for (key = town->first_key; key; key = key->next)
         {
            fprintf(fpout, "KEY\n");
            fprintf(fpout, "Flag          %d\n", key->flag);
            fprintf(fpout, "Name          %s~\n", key->name);
            fprintf(fpout, "ENDKEY\n");
         }
      }
      if (town->first_bankobj)
         fwrite_obj(NULL, town->last_bankobj, fpout, 0, OS_BANK);
      fprintf(fpout, "End\n");
   }
   fprintf(fpout, "End\n");
   fclose(fpout);
}

/*
 * Load in all the kingdom files.
 */
void load_kingdoms()
{
   FILE *fpList;
   char *filename;
   int num = 0;
   char kingdomlist[256];
   char buf[MSL];

   sprintf(kingdomlist, "%s%s", KINGDOM_DIR, KINGDOM_LIST);
   if ((fpList = fopen(kingdomlist, "r")) == NULL)
   {
      perror(kingdomlist);
      exit(1);
   }

   for (;;)
   {
      filename = feof(fpList) ? "$" : fread_word(fpList);
      if (filename[0] == '$')
         break;

      if (load_kingdom_file(filename))
      {
         num++;
      }
      else
      {
         sprintf(buf, "Cannot load kingdom file: %s", filename);
         bug(buf, 0);
      }
   }
   sysdata.max_kingdom = num;
   fclose(fpList);
   return;
}

void write_channelhistory_file()
{
   FILE *fpout;
   char filename[MIL];
   char buf[MSL];
   CHANNEL_HISTORY *chistory;

   sprintf(filename, "%s", CHISTORY_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   for (chistory = first_channelhistory; chistory; chistory = chistory->next)
   {
      fprintf(fpout, "CHISTORY\n");
      fprintf(fpout, "%d %d %d %d %d %s~ %s~\n", chistory->channel, chistory->pid, chistory->flags, chistory->level, 
                      chistory->kpid, chistory->sender, chistory->text);
   }
   fprintf(fpout, "END\n");
   fclose(fpout);
   return;
}

void read_channelhistory_file()
{
   char *word;
   bool fMatch;
   FILE *fp;
   char buf[MSL];
   CHANNEL_HISTORY *chistory = NULL;

   sprintf(buf, "%s", CHISTORY_FILE);
   if ((fp = fopen(buf, "r")) == NULL)
   {
      bug("Cannot open: %s for writing", buf);
      return;
   }

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
            if (!str_cmp(word, "CHISTORY"))
            {
               CREATE(chistory, CHANNEL_HISTORY, 1);
               LINK(chistory, first_channelhistory, last_channelhistory, next, prev);
               chistory->channel = fread_number(fp);
               chistory->pid = fread_number(fp);
               chistory->flags = fread_number(fp);
               chistory->level = fread_number(fp);
               chistory->kpid = fread_number(fp);
               chistory->sender = fread_string(fp);
               chistory->text = fread_string(fp);
               fMatch = TRUE;
               break;
            }
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
            {
               fclose(fp);
               return;
            }    
      }
      if (!fMatch)
      {
         bug("read_channelhistory_file: %s", word);
      }
   }
   return;
}

bool load_class_file(char *fname)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   struct class_type *class;
   int cl = -1;
   FILE *fp;

   sprintf(buf, "%s%s", CLASS_DIR, fname);
   if ((fp = fopen(buf, "r")) == NULL)
   {
      perror(buf);
      return FALSE;
   }

   CREATE(class, struct class_type, 1);

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
            KEY("AttrPrime", class->attr_prime, fread_number(fp));
            break;

         case 'C':
            KEY("Class", cl, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               fclose(fp);
               if (cl < 0 || cl > 0)
               {
                  sprintf(buf, "Load_class_file: Class (%s) bad/not found (%d)", class->who_name ? class->who_name : "name not found", cl);
                  bug(buf, 0);
                  if (class->who_name)
                     STRFREE(class->who_name);
                  DISPOSE(class);
                  return FALSE;
               }
               class_table[cl] = class;
               return TRUE;
            }
            KEY("ExpBase", class->exp_base, fread_number(fp));
            break;

         case 'G':
            KEY("Guild", class->guild, fread_number(fp));
            break;

         case 'H':
            KEY("HpMax", class->hp_max, fread_number(fp));
            KEY("HpMin", class->hp_min, fread_number(fp));
            break;

         case 'M':
            KEY("Mana", class->fMana, fread_number(fp));
            break;

         case 'N':
            KEY("Name", class->who_name, fread_string(fp));
            break;

         case 'R':
            KEY("RemortClass", class->remort_class, fread_number(fp));
            break;

         case 'S':
            if (!str_cmp(word, "Skill"))
            {         
               int sn, masterydiff, group, bookinfo, bookv, stype;

               word = fread_word(fp);
               group = fread_number(fp);
               masterydiff = fread_number(fp);
               stype = fread_number(fp);
               bookinfo = fread_number(fp);
               bookv = fread_number(fp);
               sn = skill_lookup(word);
               if (cl < 0 || cl > 0)
               {
                  sprintf(buf, "load_class_file: Skill %s -- class bad/not found (%d)", word, cl);
                  bug(buf, 0);
               }
               else if (!IS_VALID_SN(sn))
               {
                  sprintf(buf, "load_class_file: Skill %s unknown", word);
                  bug(buf, 0);
               }
               else
               {
                  skill_table[sn]->masterydiff[cl] = masterydiff;
                  skill_table[sn]->group[cl] = group;
                  skill_table[sn]->bookinfo[cl] = bookinfo;
                  skill_table[sn]->bookv = bookv;
                  skill_table[sn]->stype = stype;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'T':
            KEY("Thac0", class->thac0_00, fread_number(fp));
            KEY("Thac32", class->thac0_32, fread_number(fp));
            break;

         case 'W':
            KEY("Weapon", class->weapon, fread_number(fp));
            break;
      }
      if (!fMatch)
      {
         sprintf(buf, "load_class_file: no match: %s", word);
         bug(buf, 0);
      }
   }
   return FALSE;
}

/*
 * Load in all the class files.
 */
void load_classes()
{
   FILE *fpList;
   char *filename;
   char classlist[256];
   char buf[MSL];

   sprintf(classlist, "%s%s", CLASS_DIR, CLASS_LIST);
   if ((fpList = fopen(classlist, "r")) == NULL)
   {
      perror(classlist);
      exit(1);
   }

   for (;;)
   {
      filename = feof(fpList) ? "$" : fread_word(fpList);
      if (filename[0] == '$')
         break;

      if (!load_class_file(filename))
      {
         sprintf(buf, "Cannot load class file: %s", filename);
         bug(buf, 0);
      }
   }
   fclose(fpList);
   return;
}


void write_class_file(int cl)
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   struct class_type *class = class_table[cl];
   int x;

   sprintf(filename, "%s%s.class", CLASSDIR, class->who_name);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   fprintf(fpout, "Name        %s~\n", class->who_name);
   fprintf(fpout, "Class       %d\n", cl);
   fprintf(fpout, "Attrprime   %d\n", class->attr_prime);
   fprintf(fpout, "Weapon      %d\n", class->weapon);
   fprintf(fpout, "Guild       %d\n", class->guild);
   fprintf(fpout, "Thac0       %d\n", class->thac0_00);
   fprintf(fpout, "Thac32      %d\n", class->thac0_32);
   fprintf(fpout, "Hpmin       %d\n", class->hp_min);
   fprintf(fpout, "Hpmax       %d\n", class->hp_max);
   fprintf(fpout, "Mana        %d\n", class->fMana);
   fprintf(fpout, "Expbase     %d\n", class->exp_base);
   fprintf(fpout, "RemortClass %d\n", class->remort_class);
   for (x = 0; x < top_sn; x++)
   {
      if (!skill_table[x]->name || skill_table[x]->name[0] == '\0')
         break;
      fprintf(fpout, "Skill '%s' %d %d %d %d %d\n",
         skill_table[x]->name, skill_table[x]->group[cl], skill_table[x]->masterydiff[cl],
         skill_table[x]->stype, skill_table[x]->bookinfo[cl], skill_table[x]->bookv);
   }
   fprintf(fpout, "End\n");
   fclose(fpout);
}


/*
 * Load in all the race files.
 */
void load_races()
{
   FILE *fpList;
   char *filename;
   char racelist[256];
   char buf[MSL];
   int i = 0;

   /*
    * Pre-init the race_table with blank races
    */
   for (i = 0; i < MAX_RACE; i++)
      race_table[i] = NULL;

   sprintf(racelist, "%s%s", RACEDIR, RACE_LIST);
   if ((fpList = fopen(racelist, "r")) == NULL)
   {
      perror(racelist);
      exit(1);
   }

   for (;;)
   {
      filename = feof(fpList) ? "$" : fread_word(fpList);
      if (filename[0] == '$')
         break;

      if (!load_race_file(filename))
      {
         sprintf(buf, "Cannot load race file: %s", filename);
         bug(buf, 0);
      }
   }
   for (i = 0; i < MAX_RACE; i++)
   {
      if (race_table[i] == NULL)
      {
         CREATE(race_table[i], struct race_type, 1);

         sprintf(race_table[i]->race_name, "%s", "unused");
      }
   }
   fclose(fpList);
   return;
}

void write_race_file(int ra)
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   struct race_type *race = race_table[ra];
   int i = 0;
   int x, y;

   if (!race->race_name)
   {
      sprintf(buf, "Race %d has null name, not writing .race file.", ra);
      bug(buf, 0);
      return;
   }

   sprintf(filename, "%s%s.race", RACEDIR, race->race_name);
   if ((fpout = fopen(filename, "w+")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }

   fprintf(fpout, "Name        %s~\n", race->race_name);
   fprintf(fpout, "Race        %d\n", ra);
   fprintf(fpout, "Str_Plus    %d\n", race->str_plus);
   fprintf(fpout, "Dex_Plus    %d\n", race->dex_plus);
   fprintf(fpout, "Wis_Plus    %d\n", race->wis_plus);
   fprintf(fpout, "Int_Plus    %d\n", race->int_plus);
   fprintf(fpout, "Con_Plus    %d\n", race->con_plus);
   fprintf(fpout, "Cha_Plus    %d\n", race->cha_plus);
   fprintf(fpout, "Lck_Plus    %d\n", race->lck_plus);
   fprintf(fpout, "Agi_Plus    %d\n", race->agi_plus);
   fprintf(fpout, "Agi_Start   %d\n", race->agi_start);
   fprintf(fpout, "Range_Agi   %d\n", race->agi_range);
   fprintf(fpout, "Range_Str   %d\n", race->str_range);
   fprintf(fpout, "Range_Dex   %d\n", race->dex_range);
   fprintf(fpout, "Range_Con   %d\n", race->con_range);
   fprintf(fpout, "Range_Wis   %d\n", race->wis_range);
   fprintf(fpout, "Range_Int   %d\n", race->int_range);
   fprintf(fpout, "Hit         %d\n", race->hit);
   fprintf(fpout, "Mana        %d\n", race->mana);
   fprintf(fpout, "Affected    %s\n", print_bitvector(&race->affected));
   fprintf(fpout, "Resist      %d\n", race->resist);
   fprintf(fpout, "Suscept     %d\n", race->suscept);
   fprintf(fpout, "Language    %d\n", race->language);
   fprintf(fpout, "Align       %d\n", race->alignment);
   fprintf(fpout, "Min_Align  %d\n", race->minalign);
   fprintf(fpout, "Max_Align	%d\n", race->maxalign);
   fprintf(fpout, "AC_Plus    %d\n", race->ac_plus);
   fprintf(fpout, "Exp_Mult   %d\n", race->exp_multiplier);
   fprintf(fpout, "Attacks    %s\n", print_bitvector(&race->attacks));
   fprintf(fpout, "Defenses   %s\n", print_bitvector(&race->defenses));
   fprintf(fpout, "Height     %d\n", race->height);
   fprintf(fpout, "Weight     %d\n", race->weight);
   fprintf(fpout, "Weaponmin  %d\n", race->weaponmin);
   fprintf(fpout, "Weaponstd  %d\n", race->weaponstd); /* not used, take out if needed -- Xerves */
   fprintf(fpout, "Weaponmax  %d\n", race->weaponmax);
   fprintf(fpout, "RemortRace %d\n", race->remort_race);
   fprintf(fpout, "Hunger_Mod  %d\n", race->hunger_mod);
   fprintf(fpout, "Thirst_mod  %d\n", race->thirst_mod);
   fprintf(fpout, "Mana_Regen  %d\n", race->mana_regen);
   fprintf(fpout, "HP_Regen    %d\n", race->hp_regen);
   fprintf(fpout, "DodgeBonus  %d\n", race->dodge_bonus);
   fprintf(fpout, "Race_Recall %d\n", race->race_recall);
   for (i = 0; i <= MAX_WHERE_NAME; i++)
      fprintf(fpout, "WhereName  %s~\n", race->where_name[i]);

   for (x = 0; x < top_sn; x++)
   {
      if (!skill_table[x]->name || skill_table[x]->name[0] == '\0')
         break;
      if ((y = skill_table[x]->race_level[ra]) < LEVEL_IMMORTAL)
         fprintf(fpout, "Skill '%s' %d %d\n", skill_table[x]->name, y, skill_table[x]->race_adept[ra]);
   }
   fprintf(fpout, "End\n");
   fclose(fpout);
}

bool load_race_file(char *fname)
{
   char buf[MSL];
   char *word;
   char *race_name = NULL;
   bool fMatch;
   struct race_type *race;
   int ra = -1;
   FILE *fp;
   int i = 0;
   int wear = 0;

   sprintf(buf, "%s%s", RACEDIR, fname);
   if ((fp = fopen(buf, "r")) == NULL)
   {
      perror(buf);
      return FALSE;
   }

   CREATE(race, struct race_type, 1);
   for (i = 0; i < MAX_WHERE_NAME; i++)
      race->where_name[i] = where_name[i];

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
            KEY("Align", race->alignment, fread_number(fp));
            KEY("AC_Plus", race->ac_plus, fread_number(fp));
            KEY("Agi_Start", race->agi_start, fread_number(fp));
            KEY("Agi_Plus", race->agi_plus, fread_number(fp));
            KEY("Affected", race->affected, fread_bitvector(fp));
            KEY("Attacks", race->attacks, fread_bitvector(fp));
            break;

         case 'C':
            KEY("Con_Plus", race->con_plus, fread_number(fp));
            KEY("Cha_Plus", race->cha_plus, fread_number(fp));
            break;


         case 'D':
            KEY("Dex_Plus", race->dex_plus, fread_number(fp));
            KEY("Defenses", race->defenses, fread_bitvector(fp));
            KEY("DodgeBonus", race->dodge_bonus, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               fclose(fp);
               if (ra < 0 || ra >= MAX_RACE)
               {
                  sprintf(buf, "Load_race_file: Race (%s) bad/not found (%d)", race->race_name ? race->race_name : "name not found", ra);
                  bug(buf, 0);
                  DISPOSE(race);
                  return FALSE;
               }
               race_table[ra] = race;
               return TRUE;
            }

            KEY("Exp_Mult", race->exp_multiplier, fread_number(fp));

            break;


         case 'I':
            KEY("Int_Plus", race->int_plus, fread_number(fp));
            break;

         case 'H':
            KEY("Height", race->height, fread_number(fp));
            KEY("Hit", race->hit, fread_number(fp));
            KEY("HP_Regen", race->hp_regen, fread_number(fp));
            KEY("Hunger_mod", race->hunger_mod, fread_number(fp));
            break;

         case 'L':
            KEY("Language", race->language, fread_number(fp));
            KEY("Lck_Plus", race->lck_plus, fread_number(fp));
            break;


         case 'M':
            KEY("Mana", race->mana, fread_number(fp));
            KEY("Mana_Regen", race->mana_regen, fread_number(fp));
            KEY("Min_Align", race->minalign, fread_number(fp));
            race->minalign = -1000;
            KEY("Max_Align", race->maxalign, fread_number(fp));
            race->maxalign = 1000;
            break;

         case 'N':
            KEY("Name", race_name, fread_string(fp));
            break;

         case 'R':
            KEY("Race", ra, fread_number(fp));
            KEY("Race_Recall", race->race_recall, fread_number(fp));
            KEY("Range_Agi", race->agi_range, fread_number(fp));
            KEY("Range_Str", race->str_range, fread_number(fp));
            KEY("Range_Dex", race->dex_range, fread_number(fp));
            KEY("Range_Con", race->con_range, fread_number(fp));
            KEY("Range_Wis", race->wis_range, fread_number(fp));
            KEY("Range_Int", race->int_range, fread_number(fp));
            KEY("RemortRace", race->remort_race, fread_number(fp));
            KEY("Resist", race->resist, fread_number(fp));
            break;

         case 'S':
            KEY("Str_Plus", race->str_plus, fread_number(fp));
            KEY("Suscept", race->suscept, fread_number(fp));
            if (!str_cmp(word, "Skill"))
            {
               int sn, lev, adp;

               word = fread_word(fp);
               lev = fread_number(fp);
               adp = fread_number(fp);
               sn = skill_lookup(word);
               if (ra < 0 || ra >= MAX_RACE)
               {
                  sprintf(buf, "load_race_file: Skill %s -- race bad/not found (%d)", word, ra);
                  bug(buf, 0);
               }
               else if (!IS_VALID_SN(sn))
               {
                  sprintf(buf, "load_race_file: Skill %s unknown", word);
                  bug(buf, 0);
               }
               else
               {
                  skill_table[sn]->race_level[ra] = lev;
                  skill_table[sn]->race_adept[ra] = adp;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'T':
            KEY("Thirst_Mod", race->thirst_mod, fread_number(fp));
            break;

         case 'W':
            KEY("Weaponmin", race->weaponmin, fread_number(fp));
            KEY("Weaponstd", race->weaponstd, fread_number(fp));
            KEY("Weaponmax", race->weaponmax, fread_number(fp));
            KEY("Weight", race->weight, fread_number(fp));
            KEY("Wis_Plus", race->wis_plus, fread_number(fp));
            if (!str_cmp(word, "WhereName"))
            {
               if (ra < 0 || ra >= MAX_RACE)
               {
                  char *tmp;

                  sprintf(buf, "load_race_file: Title -- race bad/not found (%d)", ra);
                  bug(buf, 0);
                  tmp = fread_string_nohash(fp);
                  DISPOSE(tmp);
                  tmp = fread_string_nohash(fp);
                  DISPOSE(tmp);
               }
               else if (wear < MAX_WHERE_NAME + 1)
               {
                  race->where_name[wear] = fread_string_nohash(fp);
                  ++wear;
               }
               else
                  bug("load_race_file: Too many where_names");
               fMatch = TRUE;
               break;
            }
            break;
      }

      if (race_name != NULL)
         sprintf(race->race_name, "%-.16s", race_name);

      if (!fMatch)
      {
         sprintf(buf, "load_race_file: no match: %s", word);
         bug(buf, 0);
      }
   }
   return FALSE;
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp(SKILLTYPE ** sk1, SKILLTYPE ** sk2)
{
   SKILLTYPE *skill1 = (*sk1);
   SKILLTYPE *skill2 = (*sk2);

   if (!skill1 && skill2)
      return 1;
   if (skill1 && !skill2)
      return -1;
   if (!skill1 && !skill2)
      return 0;
   if (skill1->type < skill2->type)
      return -1;
   if (skill1->type > skill2->type)
      return 1;
   return strcmp(skill1->name, skill2->name);
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table()
{
   log_string("Sorting skill table...");
   qsort(&skill_table[1], top_sn - 1, sizeof(SKILLTYPE *), (int (*)(const void *, const void *)) skill_comp);
}


/*
 * Remap slot numbers to sn values
 */
 //No longer sorts slot numbers since this crap was useless and no longer used.  Instead, it will search for
 //the names of the skills/spells and map it to the new sn.
void remap_slot_numbers()
{
   SKILLTYPE *skill;
   SMAUG_AFF *aff;
   char tmp[32];
   int sn;

   log_string("Remapping slots to sns");

   for (sn = 0; sn <= top_sn; sn++)
   {
      if ((skill = skill_table[sn]) != NULL)
      {
         for (aff = skill->affects; aff; aff = aff->next)
            if (aff->location == APPLY_WEAPONSPELL
               || aff->location == APPLY_WEARSPELL
               || aff->location == APPLY_REMOVESPELL || aff->location == APPLY_STRIPSN || aff->location == APPLY_RECURRINGSPELL)
            {
               if (atoi(aff->modifier) >= 0)
               {
                  sprintf(tmp, "%d", skill_lookup(aff->modifier));
                  DISPOSE(aff->modifier);
                  aff->modifier = str_dup(tmp);
               }
            }
      }
   }
}

/*
 * Write skill data to a file
 */
void fwrite_skill(FILE * fpout, SKILLTYPE * skill)
{
   SMAUG_AFF *aff;
   int modifier;

   fprintf(fpout, "Name         %s~\n", skill->name);
   fprintf(fpout, "Type         %s\n", skill_tname[skill->type]);
   fprintf(fpout, "Info         %d\n", skill->info);
   if (skill->made_char && skill->made_char[0] != '\0')
      fprintf(fpout, "MadeBy       %s~\n", skill->made_char);
   if (skill->prototype)
      fprintf(fpout, "Prototype    %d\n", skill->prototype);
   fprintf(fpout, "Flags        %d\n", skill->flags);
   if (skill->target)
      fprintf(fpout, "Target       %d\n", skill->target);
   /*
    * store as new minpos (minpos>=100 flags new style in character loading)
    */
   if (skill->minimum_position)
      fprintf(fpout, "Minpos       %d\n", skill->minimum_position + 100);
   if (skill->saves)
      fprintf(fpout, "Saves        %d\n", skill->saves);
   if (skill->slot)
      fprintf(fpout, "Slot         %d\n", skill->slot);
   if (skill->min_mana)
      fprintf(fpout, "Mana         %d\n", skill->min_mana);
   if (skill->beats)
      fprintf(fpout, "Rounds       %d\n", skill->beats);
   if (skill->targetlimb > 0)
      fprintf(fpout, "Targetlimb   %d\n", skill->targetlimb);
   if (skill->range)
      fprintf(fpout, "Range        %d\n", skill->range);
   if (skill->guild != -1)
      fprintf(fpout, "Guild        %d\n", skill->guild);
   if (skill->skill_fun)
      fprintf(fpout, "Code         %s\n", skill_name(skill->skill_fun));
   else if (skill->spell_fun)
      fprintf(fpout, "Code         %s\n", spell_name(skill->spell_fun));
   fprintf(fpout, "Dammsg       %s~\n", skill->noun_damage);
   if (skill->msg_off && skill->msg_off[0] != '\0')
      fprintf(fpout, "Wearoff      %s~\n", skill->msg_off);

   if (skill->hit_char && skill->hit_char[0] != '\0')
      fprintf(fpout, "Hitchar      %s~\n", skill->hit_char);
   if (skill->hit_vict && skill->hit_vict[0] != '\0')
      fprintf(fpout, "Hitvict      %s~\n", skill->hit_vict);
   if (skill->hit_room && skill->hit_room[0] != '\0')
      fprintf(fpout, "Hitroom      %s~\n", skill->hit_room);
   if (skill->hit_dest && skill->hit_dest[0] != '\0')
      fprintf(fpout, "Hitdest      %s~\n", skill->hit_dest);

   if (skill->miss_char && skill->miss_char[0] != '\0')
      fprintf(fpout, "Misschar     %s~\n", skill->miss_char);
   if (skill->miss_vict && skill->miss_vict[0] != '\0')
      fprintf(fpout, "Missvict     %s~\n", skill->miss_vict);
   if (skill->miss_room && skill->miss_room[0] != '\0')
      fprintf(fpout, "Missroom     %s~\n", skill->miss_room);

   if (skill->die_char && skill->die_char[0] != '\0')
      fprintf(fpout, "Diechar      %s~\n", skill->die_char);
   if (skill->die_vict && skill->die_vict[0] != '\0')
      fprintf(fpout, "Dievict      %s~\n", skill->die_vict);
   if (skill->die_room && skill->die_room[0] != '\0')
      fprintf(fpout, "Dieroom      %s~\n", skill->die_room);

   if (skill->imm_char && skill->imm_char[0] != '\0')
      fprintf(fpout, "Immchar      %s~\n", skill->imm_char);
   if (skill->imm_vict && skill->imm_vict[0] != '\0')
      fprintf(fpout, "Immvict      %s~\n", skill->imm_vict);
   if (skill->imm_room && skill->imm_room[0] != '\0')
      fprintf(fpout, "Immroom      %s~\n", skill->imm_room);

   if (skill->dice && skill->dice[0] != '\0')
      fprintf(fpout, "Dice         %s~\n", skill->dice);
   if (skill->value)
      fprintf(fpout, "Value        %d\n", skill->value);
   if (skill->difficulty)
      fprintf(fpout, "Difficulty   %d\n", skill->difficulty);
   if (skill->participants)
      fprintf(fpout, "Participants %d\n", skill->participants);
   if (skill->components && skill->components[0] != '\0')
      fprintf(fpout, "Components   %s~\n", skill->components);
   if (skill->teachers && skill->teachers[0] != '\0')
      fprintf(fpout, "Teachers     %s~\n", skill->teachers);
   for (aff = skill->affects; aff; aff = aff->next)
   {
      fprintf(fpout, "Affect       '%s' %d ", aff->duration, aff->location);
      modifier = atoi(aff->modifier);
      if ((aff->location == APPLY_WEAPONSPELL
            || aff->location == APPLY_WEARSPELL
            || aff->location == APPLY_REMOVESPELL
            || aff->location == APPLY_STRIPSN || aff->location == APPLY_RECURRINGSPELL) && IS_VALID_SN(modifier))
         fprintf(fpout, "'%s' ", skill_table[modifier]->name);
      else
         fprintf(fpout, "'%s' ", aff->modifier);
      fprintf(fpout, "%d\n", aff->bitvector);
   }

   if (skill->type != SKILL_HERB)
   {
      int y;
      int min = 1000;

      min = 1000;
      for (y = 0; y < MAX_RACE; y++)
         if (skill->race_level[y] < min)
            min = skill->race_level[y];

   }
   fprintf(fpout, "End\n\n");
}

/*
 * Save the skill table to disk
 */
void save_skill_table()
{
   int x;
   FILE *fpout;

   if ((fpout = fopen(SKILL_FILE, "w")) == NULL)
   {
      bug("Cannot open skills.dat for writting", 0);
      perror(SKILL_FILE);
      return;
   }

   for (x = 0; x < top_sn; x++)
   {
      if (!skill_table[x]->name || skill_table[x]->name[0] == '\0')
         break;
      fprintf(fpout, "#SKILL\n");
      fwrite_skill(fpout, skill_table[x]);
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

/*
 * Save the herb table to disk
 */
void save_herb_table()
{
   int x;
   FILE *fpout;

   if ((fpout = fopen(HERB_FILE, "w")) == NULL)
   {
      bug("Cannot open herbs.dat for writting", 0);
      perror(HERB_FILE);
      return;
   }

   for (x = 0; x < top_herb; x++)
   {
      if (!herb_table[x]->name || herb_table[x]->name[0] == '\0')
         break;
      fprintf(fpout, "#HERB\n");
      fwrite_skill(fpout, herb_table[x]);
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

/*
 * Save the socials to disk
 */
void save_socials()
{
   FILE *fpout;
   SOCIALTYPE *social;
   int x;

   if ((fpout = fopen(SOCIAL_FILE, "w")) == NULL)
   {
      bug("Cannot open socials.dat for writting", 0);
      perror(SOCIAL_FILE);
      return;
   }

   for (x = 0; x < 27; x++)
   {
      for (social = social_index[x]; social; social = social->next)
      {
         if (!social->name || social->name[0] == '\0')
         {
            bug("Save_socials: blank social in hash bucket %d", x);
            continue;
         }
         fprintf(fpout, "#SOCIAL\n");
         fprintf(fpout, "Name        %s~\n", social->name);
         if (social->char_no_arg)
            fprintf(fpout, "CharNoArg   %s~\n", social->char_no_arg);
         else
            bug("Save_socials: NULL char_no_arg in hash bucket %d", x);
         if (social->others_no_arg)
            fprintf(fpout, "OthersNoArg %s~\n", social->others_no_arg);
         if (social->char_found)
            fprintf(fpout, "CharFound   %s~\n", social->char_found);
         if (social->others_found)
            fprintf(fpout, "OthersFound %s~\n", social->others_found);
         if (social->vict_found)
            fprintf(fpout, "VictFound   %s~\n", social->vict_found);
         if (social->char_auto)
            fprintf(fpout, "CharAuto    %s~\n", social->char_auto);
         if (social->others_auto)
            fprintf(fpout, "OthersAuto  %s~\n", social->others_auto);
         fprintf(fpout, "End\n\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

int get_skill(char *skilltype)
{
   if (!str_cmp(skilltype, "Race"))
      return SKILL_RACIAL;
   if (!str_cmp(skilltype, "Spell"))
      return SKILL_SPELL;
   if (!str_cmp(skilltype, "Skill"))
      return SKILL_SKILL;
   if (!str_cmp(skilltype, "Weapon"))
      return SKILL_WEAPON;
   if (!str_cmp(skilltype, "Tongue"))
      return SKILL_TONGUE;
   if (!str_cmp(skilltype, "Herb"))
      return SKILL_HERB;
   return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 * Added flags Aug 25, 1997 --Shaddai
 */
void save_commands()
{
   FILE *fpout;
   CMDTYPE *command;
   int x;

   if ((fpout = fopen(COMMAND_FILE, "w")) == NULL)
   {
      bug("Cannot open commands.dat for writing", 0);
      perror(COMMAND_FILE);
      return;
   }

   for (x = 0; x < 126; x++)
   {
      for (command = command_hash[x]; command; command = command->next)
      {
         if (!command->name || command->name[0] == '\0')
         {
            bug("Save_commands: blank command in hash bucket %d", x);
            continue;
         }
         fprintf(fpout, "#COMMAND\n");
         fprintf(fpout, "Name        %s~\n", command->name);
         fprintf(fpout, "Code        %s\n", skill_name(command->do_fun));
/* Oops I think this may be a bad thing so I changed it -- Shaddai */
         if (command->position < 100)
            fprintf(fpout, "Position    %d\n", command->position + 100);
         else
            fprintf(fpout, "Position    %d\n", command->position);
         fprintf(fpout, "Level       %d\n", command->level);
         fprintf(fpout, "FCommand    %d\n", command->fcommand);
         fprintf(fpout, "Log         %d\n", command->log);
         if (command->flags)
            fprintf(fpout, "Flags       %d\n", command->flags);
         fprintf(fpout, "End\n\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

SKILLTYPE *fread_skill(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   bool got_info = FALSE;
   SKILLTYPE *skill;
   int x;

   CREATE(skill, SKILLTYPE, 1);
   skill->slot = 0;
   skill->min_mana = 0;

   for (x = 0; x < MAX_RACE; x++)
   {
      skill->race_level[x] = LEVEL_IMMORTAL;
      skill->race_adept[x] = 95;
   }
   skill->guild = -1;
   skill->target = 0;
   skill->skill_fun = NULL;
   skill->spell_fun = NULL;

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
            if (!str_cmp(word, "Affect"))
            {
               SMAUG_AFF *aff;

               CREATE(aff, SMAUG_AFF, 1);
               aff->duration = str_dup(fread_word(fp));
               aff->location = fread_number(fp);
               aff->modifier = str_dup(fread_word(fp));
               aff->bitvector = fread_number(fp);
               if (!got_info)
               {
                  for (x = 0; x < 32; x++)
                  {
                     if (IS_SET(aff->bitvector, 1 << x))
                     {
                        aff->bitvector = x;
                        break;
                     }
                  }
                  if (x == 32)
                     aff->bitvector = -1;
               }
               aff->next = skill->affects;
               skill->affects = aff;
               fMatch = TRUE;
               break;
            }
            break;

         case 'C':
            if (!str_cmp(word, "Code"))
            {
               SPELL_FUN *spellfun;
               DO_FUN *dofun;
               char *w = fread_word(fp);

               fMatch = TRUE;
               if ((spellfun = spell_function(w)) != spell_notfound)
               {
                  skill->spell_fun = spellfun;
                  skill->skill_fun = NULL;
               }
               else if ((dofun = skill_function(w)) != skill_notfound)
               {
                  skill->skill_fun = dofun;
                  skill->spell_fun = NULL;
               }
               else
               {
                  bug("fread_skill: unknown skill/spell %s", w);
                  skill->spell_fun = spell_null;
               }
               break;
            }
            KEY("Code", skill->spell_fun, spell_function(fread_word(fp)));
            KEY("Components", skill->components, fread_string_nohash(fp));
            break;

         case 'D':
            KEY("Dammsg", skill->noun_damage, fread_string_nohash(fp));
            KEY("Dice", skill->dice, fread_string_nohash(fp));
            KEY("Diechar", skill->die_char, fread_string_nohash(fp));
            KEY("Dieroom", skill->die_room, fread_string_nohash(fp));
            KEY("Dievict", skill->die_vict, fread_string_nohash(fp));
            KEY("Difficulty", skill->difficulty, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               if (skill->saves != 0 && SPELL_SAVE(skill) == SE_NONE)
               {
                  bug("fread_skill(%s):  Has saving throw (%d) with no saving effect.", skill->name, skill->saves);
                  SET_SSAV(skill, SE_NEGATE);
               }
               return skill;
            }
            break;

         case 'F':
            if (!str_cmp(word, "Flags"))
            {
               skill->flags = fread_number(fp);
               /*
                * convert to new style   -Thoric
                */
               if (!got_info)
               {
                  skill->info = skill->flags & (BV11 - 1);
                  if (IS_SET(skill->flags, OLD_SF_SAVE_NEGATES))
                  {
                     if (IS_SET(skill->flags, OLD_SF_SAVE_HALF_DAMAGE))
                     {
                        SET_SSAV(skill, SE_QUARTERDAM);
                        REMOVE_BIT(skill->flags, OLD_SF_SAVE_HALF_DAMAGE);
                     }
                     else
                        SET_SSAV(skill, SE_NEGATE);
                     REMOVE_BIT(skill->flags, OLD_SF_SAVE_NEGATES);
                  }
                  else if (IS_SET(skill->flags, OLD_SF_SAVE_HALF_DAMAGE))
                  {
                     SET_SSAV(skill, SE_HALFDAM);
                     REMOVE_BIT(skill->flags, OLD_SF_SAVE_HALF_DAMAGE);
                  }
                  skill->flags >>= 11;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'G':
            KEY("Guild", skill->guild, fread_number(fp));
            break;

         case 'H':
            KEY("Hitchar", skill->hit_char, fread_string_nohash(fp));
            KEY("Hitdest", skill->hit_dest, fread_string_nohash(fp));
            KEY("Hitroom", skill->hit_room, fread_string_nohash(fp));
            KEY("Hitvict", skill->hit_vict, fread_string_nohash(fp));
            break;

         case 'I':
            KEY("Immchar", skill->imm_char, fread_string_nohash(fp));
            KEY("Immroom", skill->imm_room, fread_string_nohash(fp));
            KEY("Immvict", skill->imm_vict, fread_string_nohash(fp));
            if (!str_cmp(word, "Info"))
            {
               skill->info = fread_number(fp);
               got_info = TRUE;
               fMatch = TRUE;
               break;
            }
            break;

         case 'M':
            KEY("MadeBy", skill->made_char, fread_string_nohash(fp));
            KEY("Mana", skill->min_mana, fread_number(fp));
            if (!str_cmp(word, "Minlevel"))
            {
               fread_to_eol(fp);
               fMatch = TRUE;
               break;
            }
            /*KEY( "Minpos", skill->minimum_position, fread_number( fp ) ); */
            /*

             */
            if (!str_cmp(word, "Minpos"))
            {
               fMatch = TRUE;
               skill->minimum_position = fread_number(fp);
               if (skill->minimum_position < 100)
               {
                  switch (skill->minimum_position)
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        skill->minimum_position = 6;
                        break;
                     case 6:
                        skill->minimum_position = 8;
                        break;
                     case 7:
                        skill->minimum_position = 9;
                        break;
                     case 8:
                        skill->minimum_position = 12;
                        break;
                     case 9:
                        skill->minimum_position = 13;
                        break;
                     case 10:
                        skill->minimum_position = 14;
                        break;
                     case 11:
                        skill->minimum_position = 15;
                        break;
                  }
               }
               else
                  skill->minimum_position -= 100;
               break;
            }

            KEY("Misschar", skill->miss_char, fread_string_nohash(fp));
            KEY("Missroom", skill->miss_room, fread_string_nohash(fp));
            KEY("Missvict", skill->miss_vict, fread_string_nohash(fp));
            break;

         case 'N':
            KEY("Name", skill->name, fread_string_nohash(fp));
            break;

         case 'P':
            KEY("Participants", skill->participants, fread_number(fp));
            KEY("Prototype", skill->prototype, fread_number(fp));
            break;

         case 'R':
            KEY("Range", skill->range, fread_number(fp));
            KEY("Rounds", skill->beats, fread_number(fp));
            if (!str_cmp(word, "Race"))
            {
               int race = fread_number(fp);

               skill->race_level[race] = fread_number(fp);
               skill->race_adept[race] = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            break;

         case 'S':
            KEY("Slot", skill->slot, fread_number(fp));
            KEY("Saves", skill->saves, fread_number(fp));
            break;

         case 'T':
            KEY("Target", skill->target, fread_number(fp));
            KEY("Teachers", skill->teachers, fread_string_nohash(fp));
            KEY("Type", skill->type, get_skill(fread_word(fp)));
            KEY("Targetlimb", skill->targetlimb, fread_number(fp));
            break;

         case 'V':
            KEY("Value", skill->value, fread_number(fp));
            break;

         case 'W':
            KEY("Wearoff", skill->msg_off, fread_string_nohash(fp));
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_skill: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_skill_table()
{
   FILE *fp;

   if ((fp = fopen(SKILL_FILE, "r")) != NULL)
   {
      top_sn = 0;
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
            bug("Load_skill_table: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "SKILL"))
         {
            if (top_sn >= MAX_SKILL)
            {
               bug("load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL);
               fclose(fp);
               return;
            }
            skill_table[top_sn++] = fread_skill(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_skill_table: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open skills.dat", 0);
      exit(0);
   }
}

void load_herb_table()
{
   FILE *fp;

   if ((fp = fopen(HERB_FILE, "r")) != NULL)
   {
      top_herb = 0;
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
            bug("Load_herb_table: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "HERB"))
         {
            if (top_herb >= MAX_HERB)
            {
               bug("load_herb_table: more herbs than MAX_HERB %d", MAX_HERB);
               fclose(fp);
               return;
            }
            herb_table[top_herb++] = fread_skill(fp);
            if (herb_table[top_herb - 1]->slot == 0)
               herb_table[top_herb - 1]->slot = top_herb - 1;
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_herb_table: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open herbs.dat", 0);
      exit(0);
   }
}

void save_buykingdom_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   BUYKMOB_DATA *kmob;
   BUYKOBJ_DATA *kobj;
   BUYKTRAINER_DATA *ktrain;
   BUYKBIN_DATA *kbin;
   BTRAINER_DATA *btrain;

   sprintf(filename, "%s", BUYK_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   fprintf(fpout, "#Version %d\n\n", CASTEBUYVERSION);
   for (kmob = first_buykmob; kmob; kmob = kmob->next)
   {
      fprintf(fpout, "#MOB\n");
      fprintf(fpout, "Tree      %d\n", kmob->tree);
      fprintf(fpout, "Corn      %d\n", kmob->corn);
      fprintf(fpout, "Grain     %d\n", kmob->grain);
      fprintf(fpout, "Iron      %d\n", kmob->iron);
      fprintf(fpout, "Gold      %d\n", kmob->gold);
      fprintf(fpout, "Stone     %d\n", kmob->stone);
      fprintf(fpout, "Coins     %d\n", kmob->coins);
      fprintf(fpout, "Vnum      %d\n", kmob->vnum);
      fprintf(fpout, "MinCaste  %d\n", kmob->mincaste);
      fprintf(fpout, "Flags     %s\n", print_bitvector(&kmob->flags));
      fprintf(fpout, "End       \n");
   }
   for (kobj = first_buykobj; kobj; kobj = kobj->next)
   {
      fprintf(fpout, "#OBJECT\n");
      fprintf(fpout, "Tree      %d\n", kobj->tree);
      fprintf(fpout, "Corn      %d\n", kobj->corn);
      fprintf(fpout, "Grain     %d\n", kobj->grain);
      fprintf(fpout, "Iron      %d\n", kobj->iron);
      fprintf(fpout, "Gold      %d\n", kobj->gold);
      fprintf(fpout, "Stone     %d\n", kobj->stone);
      fprintf(fpout, "Coins     %d\n", kobj->coins);
      fprintf(fpout, "Vnum      %d\n", kobj->vnum);
      fprintf(fpout, "MinCaste  %d\n", kobj->mincaste);
      fprintf(fpout, "Flags     %s\n", print_bitvector(&kobj->flags));
      fprintf(fpout, "End       \n");
   }  
   for (ktrain = first_buyktrainer; ktrain; ktrain = ktrain->next)
   {
      fprintf(fpout, "#TRAINER\n");
      fprintf(fpout, "Name      %s~\n", ktrain->name);
      fprintf(fpout, "Sn1       '%s'\n", IS_VALID_SN(ktrain->sn[0]) ? skill_table[ktrain->sn[0]]->name : "NONE");
      fprintf(fpout, "Sn2       '%s'\n", IS_VALID_SN(ktrain->sn[1]) ? skill_table[ktrain->sn[1]]->name : "NONE");
      fprintf(fpout, "Sn3       '%s'\n", IS_VALID_SN(ktrain->sn[2]) ? skill_table[ktrain->sn[2]]->name : "NONE");
      fprintf(fpout, "Sn4       '%s'\n", IS_VALID_SN(ktrain->sn[3]) ? skill_table[ktrain->sn[3]]->name : "NONE");
      fprintf(fpout, "Sn5       '%s'\n", IS_VALID_SN(ktrain->sn[4]) ? skill_table[ktrain->sn[4]]->name : "NONE");
      fprintf(fpout, "Sn6       '%s'\n", IS_VALID_SN(ktrain->sn[5]) ? skill_table[ktrain->sn[5]]->name : "NONE");
      fprintf(fpout, "Sn7       '%s'\n", IS_VALID_SN(ktrain->sn[6]) ? skill_table[ktrain->sn[6]]->name : "NONE");
      fprintf(fpout, "Sn8       '%s'\n", IS_VALID_SN(ktrain->sn[7]) ? skill_table[ktrain->sn[7]]->name : "NONE");
      fprintf(fpout, "Sn9       '%s'\n", IS_VALID_SN(ktrain->sn[8]) ? skill_table[ktrain->sn[8]]->name : "NONE");
      fprintf(fpout, "Sn10      '%s'\n", IS_VALID_SN(ktrain->sn[9]) ? skill_table[ktrain->sn[9]]->name : "NONE");
      fprintf(fpout, "Sn11      '%s'\n", IS_VALID_SN(ktrain->sn[10]) ? skill_table[ktrain->sn[10]]->name : "NONE");
      fprintf(fpout, "Sn12      '%s'\n", IS_VALID_SN(ktrain->sn[11]) ? skill_table[ktrain->sn[11]]->name : "NONE");
      fprintf(fpout, "Sn13      '%s'\n", IS_VALID_SN(ktrain->sn[12]) ? skill_table[ktrain->sn[12]]->name : "NONE");
      fprintf(fpout, "Sn14      '%s'\n", IS_VALID_SN(ktrain->sn[13]) ? skill_table[ktrain->sn[13]]->name : "NONE");
      fprintf(fpout, "Sn15      '%s'\n", IS_VALID_SN(ktrain->sn[14]) ? skill_table[ktrain->sn[14]]->name : "NONE");
      fprintf(fpout, "Sn16      '%s'\n", IS_VALID_SN(ktrain->sn[15]) ? skill_table[ktrain->sn[15]]->name : "NONE");
      fprintf(fpout, "Sn17      '%s'\n", IS_VALID_SN(ktrain->sn[16]) ? skill_table[ktrain->sn[16]]->name : "NONE");
      fprintf(fpout, "Sn18      '%s'\n", IS_VALID_SN(ktrain->sn[17]) ? skill_table[ktrain->sn[17]]->name : "NONE");
      fprintf(fpout, "Sn19      '%s'\n", IS_VALID_SN(ktrain->sn[18]) ? skill_table[ktrain->sn[18]]->name : "NONE");
      fprintf(fpout, "Sn20      '%s'\n", IS_VALID_SN(ktrain->sn[19]) ? skill_table[ktrain->sn[19]]->name : "NONE");
      fprintf(fpout, "Mastery1  %d\n", ktrain->mastery[0]);
      fprintf(fpout, "Mastery2  %d\n", ktrain->mastery[1]);
      fprintf(fpout, "Mastery3  %d\n", ktrain->mastery[2]);
      fprintf(fpout, "Mastery4  %d\n", ktrain->mastery[3]);
      fprintf(fpout, "Mastery5  %d\n", ktrain->mastery[4]);
      fprintf(fpout, "Mastery6  %d\n", ktrain->mastery[5]);
      fprintf(fpout, "Mastery7  %d\n", ktrain->mastery[6]);
      fprintf(fpout, "Mastery8  %d\n", ktrain->mastery[7]);
      fprintf(fpout, "Mastery9  %d\n", ktrain->mastery[8]);
      fprintf(fpout, "Mastery10 %d\n", ktrain->mastery[9]);
      fprintf(fpout, "Mastery11 %d\n", ktrain->mastery[10]);
      fprintf(fpout, "Mastery12 %d\n", ktrain->mastery[11]);
      fprintf(fpout, "Mastery13 %d\n", ktrain->mastery[12]);
      fprintf(fpout, "Mastery14 %d\n", ktrain->mastery[13]);
      fprintf(fpout, "Mastery15 %d\n", ktrain->mastery[14]);
      fprintf(fpout, "Mastery16 %d\n", ktrain->mastery[15]);
      fprintf(fpout, "Mastery17 %d\n", ktrain->mastery[16]);
      fprintf(fpout, "Mastery18 %d\n", ktrain->mastery[17]);
      fprintf(fpout, "Mastery19 %d\n", ktrain->mastery[18]);
      fprintf(fpout, "Mastery20 %d\n", ktrain->mastery[19]);
      fprintf(fpout, "Cost      %d\n", ktrain->cost);
      fprintf(fpout, "Pid       %d\n", ktrain->pid);
      fprintf(fpout, "End       \n");
   }  
   for (btrain = first_boughttrainer; btrain; btrain = btrain->next)
   {
      fprintf(fpout, "#BOUGHT\n");
      fprintf(fpout, "Rvnum     %d\n", btrain->rvnum);
      fprintf(fpout, "X         %d\n", btrain->x);
      fprintf(fpout, "Y         %d\n", btrain->y);
      fprintf(fpout, "Map       %d\n", btrain->map);
      fprintf(fpout, "Pid       %d\n", btrain->pid);
      fprintf(fpout, "End\n");
   }
   for (kbin = first_buykbin; kbin; kbin = kbin->next)
   {
      fprintf(fpout, "#Bin\n");
      fprintf(fpout, "Name      %s~\n", kbin->name);
      fprintf(fpout, "Stone     %d\n", kbin->stone);
      fprintf(fpout, "Coins     %d\n", kbin->coins);
      fprintf(fpout, "Hold      %d\n", kbin->hold);
      fprintf(fpout, "MinCaste  %d\n", kbin->mincaste);
      fprintf(fpout, "End\n");
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_buybought_data(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   BTRAINER_DATA *btrain;

   CREATE(btrain, BTRAINER_DATA, 1);

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
            
         case 'M':
            KEY("Map", btrain->map, fread_number(fp));
            break; 
            
         case 'P':
            KEY("Pid", btrain->pid, fread_number(fp));
            break;

         case 'R':
            KEY("Rvnum", btrain->rvnum, fread_number(fp));
            break;    
            
         case 'X':
            KEY("X", btrain->x, fread_number(fp));
            break;
            
         case 'Y':
            KEY("Y", btrain->y, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(btrain, first_boughttrainer, last_boughttrainer, next, prev);
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_buybought_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void fread_buyktrainer_data(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   BUYKTRAINER_DATA *ktrain;

   CREATE(ktrain, BUYKTRAINER_DATA, 1);

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
            KEY("Cost", ktrain->cost, fread_number(fp));
            break;
            
         case 'M':
            KEY("Mastery1", ktrain->mastery[0], fread_number(fp));
            KEY("Mastery2", ktrain->mastery[1], fread_number(fp));
            KEY("Mastery3", ktrain->mastery[2], fread_number(fp));
            KEY("Mastery4", ktrain->mastery[3], fread_number(fp));
            KEY("Mastery5", ktrain->mastery[4], fread_number(fp));   
            KEY("Mastery6", ktrain->mastery[5], fread_number(fp));
            KEY("Mastery7", ktrain->mastery[6], fread_number(fp));
            KEY("Mastery8", ktrain->mastery[7], fread_number(fp));
            KEY("Mastery9", ktrain->mastery[8], fread_number(fp));
            KEY("Mastery10", ktrain->mastery[9], fread_number(fp));  
            KEY("Mastery11", ktrain->mastery[10], fread_number(fp));  
            KEY("Mastery12", ktrain->mastery[11], fread_number(fp));  
            KEY("Mastery13", ktrain->mastery[12], fread_number(fp));  
            KEY("Mastery14", ktrain->mastery[13], fread_number(fp));  
            KEY("Mastery15", ktrain->mastery[14], fread_number(fp));  
            KEY("Mastery16", ktrain->mastery[15], fread_number(fp));  
            KEY("Mastery17", ktrain->mastery[16], fread_number(fp));  
            KEY("Mastery18", ktrain->mastery[17], fread_number(fp));  
            KEY("Mastery19", ktrain->mastery[18], fread_number(fp));  
            KEY("Mastery20", ktrain->mastery[19], fread_number(fp));  
            break;
            
         case 'N':
            KEY("Name", ktrain->name, fread_string(fp));
            break;
            
         case 'P':
            KEY("Pid", ktrain->pid, fread_number(fp));
            break;
            
         case 'S':
            if (boughtsaveversion <= 1)
            {
               KEY("Sn1", ktrain->sn[0], fread_number(fp));
               KEY("Sn2", ktrain->sn[1], fread_number(fp));
               KEY("Sn3", ktrain->sn[2], fread_number(fp));
               KEY("Sn4", ktrain->sn[3], fread_number(fp));
               KEY("Sn5", ktrain->sn[4], fread_number(fp));    
               KEY("Sn6", ktrain->sn[5], fread_number(fp));
               KEY("Sn7", ktrain->sn[6], fread_number(fp));
               KEY("Sn8", ktrain->sn[7], fread_number(fp));
               KEY("Sn9", ktrain->sn[8], fread_number(fp));
               KEY("Sn10", ktrain->sn[9], fread_number(fp));   
               KEY("Sn11", ktrain->sn[10], fread_number(fp));   
               KEY("Sn12", ktrain->sn[11], fread_number(fp));   
               KEY("Sn13", ktrain->sn[12], fread_number(fp));   
               KEY("Sn14", ktrain->sn[13], fread_number(fp));   
               KEY("Sn15", ktrain->sn[14], fread_number(fp));   
               KEY("Sn16", ktrain->sn[15], fread_number(fp));   
               KEY("Sn17", ktrain->sn[16], fread_number(fp));   
               KEY("Sn18", ktrain->sn[17], fread_number(fp));   
               KEY("Sn19", ktrain->sn[18], fread_number(fp));   
               KEY("Sn20", ktrain->sn[19], fread_number(fp));   
            }
            else
            {
               if (!str_cmp(word, "Sn1"))
               {
                  ktrain->sn[0] = skill_lookup(fread_word(fp));  
                  if (ktrain->sn[0] < 0)
                     ktrain->sn[0] = 0;
               }
               if (!str_cmp(word, "Sn2"))
               {
                  ktrain->sn[1] = skill_lookup(fread_word(fp));  
                  if (ktrain->sn[1] < 0)
                     ktrain->sn[1] = 0;  
               } 
               if (!str_cmp(word, "Sn3"))
               {
                  ktrain->sn[2] = skill_lookup(fread_word(fp));  
                  if (ktrain->sn[2] < 0)
                     ktrain->sn[2] = 0;  
               } 
               if (!str_cmp(word, "Sn4"))
               {
                  ktrain->sn[3] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[3] < 0)
                     ktrain->sn[3] = 0;   
               } 
               if (!str_cmp(word, "Sn5"))
               {
                  ktrain->sn[4] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[4] < 0)
                     ktrain->sn[4] = 0;  
               }
               if (!str_cmp(word, "Sn6"))
               {
                  ktrain->sn[5] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[5] < 0)
                     ktrain->sn[5] = 0;  
               }  
               if (!str_cmp(word, "Sn7"))
               {
                  ktrain->sn[6] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[6] < 0)
                     ktrain->sn[6] = 0;  
               }  
               if (!str_cmp(word, "Sn8"))
               {
                  ktrain->sn[7] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[7] < 0)
                     ktrain->sn[7] = 0;  
               }  
               if (!str_cmp(word, "Sn9"))
               {
                  ktrain->sn[8] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[8] < 0)
                     ktrain->sn[8] = 0;  
               }  
               if (!str_cmp(word, "Sn10"))
               {
                  ktrain->sn[9] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[9] < 0)
                     ktrain->sn[9] = 0;  
               }    
               if (!str_cmp(word, "Sn11"))
               {
                  ktrain->sn[10] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[10] < 0)
                     ktrain->sn[10] = 0;  
               }  
               if (!str_cmp(word, "Sn12"))
               {
                  ktrain->sn[11] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[11] < 0)
                     ktrain->sn[11] = 0;  
               }  
               if (!str_cmp(word, "Sn13"))
               {
                  ktrain->sn[12] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[12] < 0)
                     ktrain->sn[12] = 0;  
               }  
               if (!str_cmp(word, "Sn14"))
               {
                  ktrain->sn[13] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[13] < 0)
                     ktrain->sn[13] = 0;  
               }  
               if (!str_cmp(word, "Sn15"))
               {
                  ktrain->sn[14] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[14] < 0)
                     ktrain->sn[14] = 0;  
               }  
               if (!str_cmp(word, "Sn16"))
               {
                  ktrain->sn[15] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[15] < 0)
                     ktrain->sn[15] = 0;  
               }  
               if (!str_cmp(word, "Sn17"))
               {
                  ktrain->sn[16] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[16] < 0)
                     ktrain->sn[16] = 0;  
               }  
               if (!str_cmp(word, "Sn18"))
               {
                  ktrain->sn[17] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[17] < 0)
                     ktrain->sn[17] = 0;  
               }  
               if (!str_cmp(word, "Sn19"))
               {
                  ktrain->sn[18] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[18] < 0)
                     ktrain->sn[18] = 0;  
               }  
               if (!str_cmp(word, "Sn20"))
               {
                  ktrain->sn[19] = skill_lookup(fread_word(fp)); 
                  if (ktrain->sn[19] < 0)
                     ktrain->sn[19] = 0;  
               }    
               fMatch = TRUE;
            }
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(ktrain, first_buyktrainer, last_buyktrainer, next, prev);         
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_buykmob_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void fread_buykmob_data(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   BUYKMOB_DATA *kmob;

   CREATE(kmob, BUYKMOB_DATA, 1);

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
            KEY("Coins", kmob->coins, fread_number(fp));
            KEY("Corn", kmob->corn, fread_number(fp));
            break;
         case 'F':
            KEY("Flags", kmob->flags, fread_bitvector(fp));
            break;
         case 'G':
            KEY("Gold", kmob->gold, fread_number(fp));
            KEY("Grain", kmob->grain, fread_number(fp));
            break;
         case 'I':
            KEY("Iron", kmob->iron, fread_number(fp));
            break;
         case 'M':
            KEY("MinCaste", kmob->mincaste, fread_number(fp));
            break;
         case 'S':
            KEY("Stone", kmob->stone, fread_number(fp));
            break;
         case 'T':
            KEY("Tree", kmob->tree, fread_number(fp));
            break;
         case 'V':
            KEY("Vnum", kmob->vnum, fread_number(fp));
            break;        

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(kmob, first_buykmob, last_buykmob, next, prev);
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_buykmob_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void fread_buykbin_data(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   BUYKBIN_DATA *kbin;

   CREATE(kbin, BUYKBIN_DATA, 1);

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
            KEY("Coins", kbin->coins, fread_number(fp));
            break;
            
         case 'H':
            KEY("Hold", kbin->hold, fread_number(fp));
            break;

         case 'M':
            KEY("MinCaste", kbin->mincaste, fread_number(fp));
            break;
            
         case 'N':
            KEY("Name", kbin->name, fread_string(fp));
            break;
            
         case 'S':
            KEY("Stone", kbin->stone, fread_number(fp));
            break;      

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(kbin, first_buykbin, last_buykbin, next, prev);
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_buykbin_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void fread_buykobj_data(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   BUYKOBJ_DATA *kobj;

   CREATE(kobj, BUYKOBJ_DATA, 1);

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
            KEY("Coins", kobj->coins, fread_number(fp));
            KEY("Corn", kobj->corn, fread_number(fp));
            break;
         case 'F':
            KEY("Flags", kobj->flags, fread_bitvector(fp));
            break;
         case 'G':
            KEY("Gold", kobj->gold, fread_number(fp));
            KEY("Grain", kobj->grain, fread_number(fp));
            break;
         case 'I':
            KEY("Iron", kobj->iron, fread_number(fp));
            break;
         case 'M':
            KEY("MinCaste", kobj->mincaste, fread_number(fp));
            break;
         case 'S':
            KEY("Stone", kobj->stone, fread_number(fp));
            break;
         case 'T':
            KEY("Tree", kobj->tree, fread_number(fp));
            break;
         case 'V':
            KEY("Vnum", kobj->vnum, fread_number(fp));
            break;        

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(kobj, first_buykobj, last_buykobj, next, prev);
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_buykobj_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_buykingdom_data()
{
   FILE *fp;
   char name[100];
   ROOM_INDEX_DATA *room;
   CHAR_DATA *victim;
   BTRAINER_DATA *btrain;
   BUYKTRAINER_DATA *ktrain;
   
   boughtsaveversion = 1; //Just in case.....

   if ((fp = fopen(BUYK_FILE, "r")) != NULL)
   {
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
            bug("Load_buycaste_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "Version"))
         {
            boughtsaveversion = fread_number(fp);
            continue;
         }
         else if (!str_cmp(word, "OBJECT"))
         {
            fread_buykobj_data(fp);
            continue;
         }
         else if (!str_cmp(word, "MOB"))
         {
            fread_buykmob_data(fp);
            continue;
         }
         else if (!str_cmp(word, "TRAINER"))
         {
            fread_buyktrainer_data(fp);
            continue;
         }
         else if (!str_cmp(word, "BOUGHT"))
         {
            fread_buybought_data(fp);
            continue;
         }
         else if (!str_cmp(word, "BIN"))
         {
            fread_buykbin_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
         {
            for (btrain = first_boughttrainer; btrain; btrain = btrain->next)
            {
               for (ktrain = first_buyktrainer; ktrain; ktrain = ktrain->next)
               {
                  if (btrain->pid == ktrain->pid) //load mobile
                  {
                     room = get_room_index(btrain->rvnum);
                     if (!room)
                     {
                         bug("fread_buyktrainer_data:  Invalid room %d to put a trainer in.  Removing from list.", btrain->rvnum);
                         continue;
                     }
                     //Load mobile and place it Now!
                     victim = create_mobile(get_mob_index(MOB_VNUM_TRAINER));
                     char_to_room(victim, room);
                     victim->m2 = ktrain->pid;
                     victim->coord->x = btrain->x;
                     victim->coord->y = btrain->y;
                     victim->map = btrain->map;
                     SET_ONMAP_FLAG(victim);
               
                     STRFREE(victim->name);
                     sprintf(name, ktrain->name);
                     victim->name = STRALLOC(name);
               
                     STRFREE(victim->short_descr);
                     sprintf(name, ktrain->name);
                     victim->short_descr = STRALLOC(name);
               
                     STRFREE(victim->long_descr);
                     sprintf(name, "%s is here training all that are interested.\n\r", ktrain->name);
                     victim->long_descr = STRALLOC(name);   
         
                  }
               }
            }
            break;
         }
         else
         {
            bug("Load_buykingdom_data: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open buykingdom file", 0);
      exit(0);
   }
}

//Saves extraction mobs to file
void save_extraction_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   CHAR_DATA *mob;

   sprintf(filename, "%s", EXTRACTION_FILE);
   
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   for (mob = last_char; mob; mob = mob->prev)
   {
      //m1 - Resource AMT   m2 - Resource MAX   m3 - Cost   m4 - Kingdom   m5 - Resource Type   m6 - Extraction effic.
         //m9 - Time started  
      if (IS_NPC(mob) && IS_ACT_FLAG(mob, ACT_EXTRACTMOB) && mob->in_room)
      {  
         fprintf(fpout, "#EXTRACT\n");
         fprintf(fpout, "Vnum      %d\n", mob->pIndexData->vnum);
         fprintf(fpout, "Name      %s~\n", mob->name);
         fprintf(fpout, "Short     %s~\n", mob->short_descr);
         fprintf(fpout, "Long      %s~\n", mob->long_descr);
         fprintf(fpout, "Race      %d\n", mob->race);
         fprintf(fpout, "Str       %d\n", mob->perm_str);
         fprintf(fpout, "Int       %d\n", mob->perm_int);
         fprintf(fpout, "Wis       %d\n", mob->perm_wis);
         fprintf(fpout, "Lck       %d\n", mob->perm_lck);
         fprintf(fpout, "Dex       %d\n", mob->perm_dex);
         fprintf(fpout, "Agi       %d\n", mob->perm_agi);
         fprintf(fpout, "Con       %d\n", mob->perm_con);
         fprintf(fpout, "X         %d\n", mob->coord->x);
         fprintf(fpout, "Y         %d\n", mob->coord->y);
         fprintf(fpout, "Stx       %d\n", mob->stx);
         fprintf(fpout, "Sty       %d\n", mob->sty);
         fprintf(fpout, "Map       %d\n", mob->map);
         fprintf(fpout, "Room      %d\n", mob->in_room->vnum);
         fprintf(fpout, "Flags     %s\n", print_bitvector(&mob->act));
         fprintf(fpout, "RAMT      %d\n", mob->m1);
         fprintf(fpout, "Cost      %d\n", mob->m3);
         fprintf(fpout, "Kingdom   %d\n", mob->m4);
         fprintf(fpout, "Resource  %d\n", mob->m5);
         fprintf(fpout, "Town      %d\n", mob->m7); 
         fprintf(fpout, "Time      %d\n", mob->m9);
         fprintf(fpout, "ETime     %d\n", mob->m10);
         fprintf(fpout, "End\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_extraction_data(FILE * fp)
{
   char *word;
   int vnum, x, y, map, room, ramt, cost, kingdom, ptime, etime;
   char *name = NULL;
   char *shortn = NULL;
   char *longn = NULL;
   int race, str, inte, wis, lck, dex, agi, con, resource, stx, sty, town;
   EXT_BV flags;
   bool fMatch;
   
   vnum=x=y=map=room=ramt=cost=kingdom=ptime=etime=0;
   race=str=inte=wis=lck=dex=agi=con=resource=stx=sty=town=0;

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
            KEY("Agi", agi, fread_number(fp));
         
         case 'C':
            KEY("Con", con, fread_number(fp));
            KEY("Cost", cost, fread_number(fp));
            break;
            
         case 'D':
            KEY("Dex", dex, fread_number(fp));
            break;
          
         case 'F':
            KEY("Flags", flags, fread_bitvector(fp));
            break;
            
         case 'I':
            KEY("Int", inte, fread_number(fp));
            break;

         case 'K':
            KEY("Kingdom", kingdom, fread_number(fp));
            break;
            
         case 'L':
            KEY("Lck", lck, fread_number(fp));
            KEY("Long", longn, fread_string(fp));
            break;

         case 'M':
            KEY("Map", map, fread_number(fp));
            break;
            
         case 'N':
            KEY("Name", name, fread_string(fp));
            break;          

         case 'R':
            KEY("Race", race, fread_number(fp));
            KEY("RAMT", ramt, fread_number(fp));
            KEY("Resource", resource, fread_number(fp));
            KEY("Room", room, fread_number(fp));
            break;

         case 'S':
            KEY("Short", shortn, fread_string(fp));
            KEY("Str", str, fread_number(fp));
            KEY("Stx", stx, fread_number(fp));
            KEY("Sty", sty, fread_number(fp));
            break;

         case 'T':
            KEY("Time", ptime, fread_number(fp));
            KEY("Town", town, fread_number(fp));
            break;

         case 'V':
            KEY("Vnum", vnum, fread_number(fp));
            break;
            
         case 'W':
            KEY("Wis", wis, fread_number(fp));
            break;

         case 'X':
            KEY("X", x, fread_number(fp));
            break;

         case 'Y':
            KEY("Y", y, fread_number(fp));
            break;
            

         case 'E':
            KEY("ETime", etime, fread_number(fp));
            if (!str_cmp(word, "End"))
            {
               CHAR_DATA *victim;

              // if (time(0) - ptime > 10800) //More than 3 hours, don't load the extraction mobs....
             //     return;
               victim = create_mobile(get_mob_index(vnum));
               char_to_room(victim, get_room_index(room));
               
               STRFREE(victim->name);
               victim->name = STRALLOC(name); 
               STRFREE(victim->long_descr);
               victim->long_descr = STRALLOC(longn); 
               STRFREE(victim->short_descr);
               victim->short_descr = STRALLOC(shortn);   
               victim->race = race;
               victim->perm_str = str;
               victim->perm_int = inte;
               victim->perm_wis = wis;
               victim->perm_lck = lck;
               victim->perm_dex = dex;
               victim->perm_agi = agi;
               victim->perm_con = con;
               victim->coord->x = x;
               victim->coord->y = y;
               victim->map = map;
               victim->stx = stx;
               victim->sty = sty;
               victim->stmap = map;
               victim->m1 = ramt;
               victim->m3 = cost;
               victim->m4 = kingdom;
               victim->m5 = resource;
               victim->m7 = town;
               victim->dumptown = get_town_tpid(kingdom, town);
               victim->m9 = ptime;
               victim->m10 = etime;
               xCLEAR_BITS(victim->act); 
               victim->act = flags;
               return;
            }
            break;
      }
   }
}

void load_extraction_data()
{
   FILE *fp;

   if ((fp = fopen(EXTRACTION_FILE, "r")) != NULL)
   {
      for (;;)
      {
         char letter;
         char *word;

         if (feof(fp))
            break;

         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            fread_to_eol(fp);
            continue;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "EXTRACT"))
         {
            fread_extraction_data(fp);
            continue;
         }
         if (!str_cmp(word, "END"))
         {
            fclose(fp);
            return;
         }
      }
      fclose(fp);
   }
}
         
//Saves military data to file, appends the file and writes to the bottom.
void save_mlist_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   CHAR_DATA *mob;

   sprintf(filename, "%s", MILIST_FILE);
   
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   for (mob = last_char; mob; mob = mob->prev)
   {
      if (IS_NPC(mob) && IS_ACT_FLAG(mob, ACT_MILITARY) && mob->in_room)
      {
          fprintf(fpout, "#MIDATA\n");
          fprintf(fpout, "Name      %s~\n", mob->name);
          fprintf(fpout, "Short     %s~\n", mob->short_descr);
          fprintf(fpout, "Long      %s~\n", mob->long_descr);
          fprintf(fpout, "Race      %d\n", mob->race);
          fprintf(fpout, "Str       %d\n", mob->perm_str);
          fprintf(fpout, "Int       %d\n", mob->perm_int);
          fprintf(fpout, "Wis       %d\n", mob->perm_wis);
          fprintf(fpout, "Lck       %d\n", mob->perm_lck);
          fprintf(fpout, "Dex       %d\n", mob->perm_dex);
          fprintf(fpout, "Agi       %d\n", mob->perm_agi);
          fprintf(fpout, "Con       %d\n", mob->perm_con);
          fprintf(fpout, "Cost      %d\n", mob->m3);
          fprintf(fpout, "X         %d\n", mob->coord->x);
          fprintf(fpout, "Y         %d\n", mob->coord->y);
          fprintf(fpout, "Map       %d\n", mob->map);
          fprintf(fpout, "Vnum      %d\n", mob->pIndexData->vnum);
          fprintf(fpout, "Room      %d\n", mob->in_room->vnum);
          fprintf(fpout, "Kingdom   %d\n", mob->m4);
          fprintf(fpout, "Flags     %s\n", print_bitvector(&mob->miflags));
          fprintf(fpout, "Time      %d\n", mob->m9);
          fprintf(fpout, "Speed     %d\n", mob->m10);
          fprintf(fpout, "Town      %d\n", mob->m1);
          if (xIS_SET(mob->miflags, KM_INVITE))
             fprintf(fpout, "IRange    %d\n", mob->m5);
          if (xIS_SET(mob->miflags, KM_PATROL))
          {
             fprintf(fpout, "PRange    %d\n", mob->m2);
             fprintf(fpout, "PX        %d\n", mob->m7);
             fprintf(fpout, "PY        %d\n", mob->m8);
          }
          if (xIS_SET(mob->miflags, KM_WARN) || xIS_SET(mob->miflags, KM_ATTACKA) || xIS_SET(mob->miflags, KM_ATTACKN) || xIS_SET(mob->miflags, KM_ATTACKE))
             fprintf(fpout, "WaRange   %d\n", mob->m6);
          if (mob->position == POS_MOUNTED && mob->mount)
             fprintf(fpout, "Mount     1\n");
          fprintf(fpout, "End\n");
          file_ver = SAVEVERSION;
          de_equip_char(mob);
          if (mob->first_carrying)
             fwrite_obj(mob, mob->last_carrying, fpout, 0, OS_CARRY);
          re_equip_char(mob);
          fprintf(fpout, "#END\n");
       }
    }
    fclose(fpout);
}


void fread_mlist_data(FILE * fp)
{
   char *word;
   bool fMatch;
   int mount = 0;
   CHAR_DATA *lmount;
   MLIST_DATA *mlist;
   int m1 = -1;

   CREATE(mlist, MLIST_DATA, 1);

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
            KEY("Agi", mlist->agi, fread_number(fp));
            break;
            
         case 'C':
            KEY("Con", mlist->con, fread_number(fp));
            KEY("Cost", mlist->cost, fread_number(fp));
            break;
         
         case 'D':
            KEY("Dex", mlist->dex, fread_number(fp));
            break;
          
         case 'F':
            KEY("Flags", mlist->miflags, fread_bitvector(fp));
            break;

         case 'I':
            KEY("Int", mlist->intelligence, fread_number(fp));
            KEY("IRange", mlist->irange, fread_number(fp));
            break;

         case 'K':
            KEY("Kingdom", mlist->kingdom, fread_number(fp));
            break;
            
         case 'L':
            KEY("Lck", mlist->lck, fread_number(fp));
            KEY("Long", mlist->long_descr, fread_string(fp));
            break;

         case 'M':
            KEY("Map", mlist->map, fread_number(fp));
            KEY("Mount", mount, fread_number(fp));             
            break;
            
         case 'N':
            KEY("Name", mlist->name, fread_string(fp));
            break;

         case 'P':
            KEY("PX", mlist->px, fread_number(fp));
            KEY("PY", mlist->py, fread_number(fp));
            KEY("PRange", mlist->prange, fread_number(fp));
            break;

         case 'R':
            KEY("Race", mlist->race, fread_number(fp));
            KEY("Room", mlist->room, fread_number(fp));
            break;

         case 'S':
            KEY("Short", mlist->short_descr, fread_string(fp));
            KEY("Speed", mlist->speed, fread_number(fp));
            KEY("Str", mlist->str, fread_number(fp));
            break;

         case 'T':
            KEY("Time", mlist->time, fread_number(fp));
            KEY("Town", m1, fread_number(fp));
            break;

         case 'V':
            KEY("Vnum", mlist->vnum, fread_number(fp));
            break;

         case 'W':
            KEY("WaRange", mlist->warange, fread_number(fp));
            KEY("Wis", mlist->wis, fread_number(fp));
            break;

         case 'X':
            KEY("X", mlist->x, fread_number(fp));
            break;

         case 'Y':
            KEY("Y", mlist->y, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               CHAR_DATA *victim;

               victim = create_mobile(get_mob_index(mlist->vnum));
               char_to_room(victim, get_room_index(mlist->room));
               victim->coord->x = mlist->x;
               victim->coord->y = mlist->y;
               victim->map = mlist->map;
               if (mlist->x > 0 && mlist->y > 0 && mlist->map > -1)
                  SET_ONMAP_FLAG(victim);
               if (mount)
               {
                  lmount = create_mobile(get_mob_index(MOB_KMOB_HORSE));
                  char_to_room(lmount, get_room_index(OVERLAND_SOLAN));
                  lmount->coord->x = victim->coord->x;
                  lmount->coord->y = victim->coord->y;
                  lmount->map = victim->map;
                  lmount->m4 = mlist->kingdom;
                  SET_ONMAP_FLAG(lmount); 
                  xSET_BIT(lmount->act, ACT_MOUNTED);
                  victim->mount = lmount;
                  victim->position = POS_MOUNTED;
               }
               victim->m1 = m1;
               victim->m3 = mlist->cost;
               victim->m4 = mlist->kingdom;
               victim->m9 = mlist->time;
               victim->m10 = mlist->speed;
               victim->miflags = mlist->miflags;
               if (xIS_SET(victim->miflags, KM_INVITE))
                  victim->m5 = mlist->irange;
               if (xIS_SET(victim->miflags, KM_PATROL))
               {
                  victim->m2 = mlist->prange;
                  victim->m7 = mlist->px;
                  victim->m8 = mlist->py;
               }
               if (mlist->name)
               {
                  STRFREE(victim->name);
                  victim->name = STRALLOC(mlist->name); 
               }
               if (mlist->long_descr)
               {
                  STRFREE(victim->long_descr);
                  victim->long_descr = STRALLOC(mlist->long_descr); 
               }
               if (mlist->short_descr)
               {
                  STRFREE(victim->short_descr);
                  victim->short_descr = STRALLOC(mlist->short_descr);   
               }
               victim->race = mlist->race;
               victim->perm_str = mlist->str;
               victim->perm_int = mlist->intelligence;
               victim->perm_wis = mlist->wis;
               victim->perm_lck = mlist->lck;
               victim->perm_dex = mlist->dex;
               victim->perm_agi = mlist->agi;
               victim->perm_con = mlist->con;
               SET_BIT(victim->resistant, race_table[victim->race]->resist);
               SET_BIT(victim->susceptible, race_table[victim->race]->suscept);
               xSET_BITS(victim->affected_by, race_table[victim->race]->affected); 
               if (xIS_SET(victim->miflags, KM_WARN) || xIS_SET(victim->miflags, KM_ATTACKA)
                  || xIS_SET(victim->miflags, KM_ATTACKN) || xIS_SET(victim->miflags, KM_ATTACKE))
                  victim->m6 = mlist->warange;
               STRFREE(mlist->name);
               STRFREE(mlist->long_descr);
               STRFREE(mlist->short_descr);
               DISPOSE(mlist);
               for (;;)
               {
                  char letter;

                  if (feof(fp))
                     break;

                  letter = fread_letter(fp);
                  if (letter == '*')
                  {
                     fread_to_eol(fp);
                     continue;
                  }

                  if (letter != '#')
                  {
                     fread_to_eol(fp);
                     continue;
                  }

                  word = fread_word(fp);
               
                  if (!str_cmp(word, "OBJECT")) /* Objects */
                  {
                     fread_obj(victim, fp, OS_CARRY);
                  }
                  else if (!str_cmp(word, "END")) //end the milist data
                  {
                     return;
                  }   
               }
               return;
            }
      }
   }
}

void load_mlist_data()
{
   FILE *fp;

   if ((fp = fopen(MILIST_FILE, "r")) != NULL)
   {
      for (;;)
      {
         char letter;
         char *word;

         if (feof(fp))
            break;

         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            fread_to_eol(fp);
            continue;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "MIDATA"))
         {
            fread_mlist_data(fp);
            continue;
         }
      }
      fclose(fp);
   }
}

void save_bin_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   OBJ_DATA *obj;

   sprintf(filename, "%s", BIN_LIST);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->item_type == ITEM_HOLDRESOURCE && !obj->carried_by)
      {
         fprintf(fpout, "#BIN\n");
         fprintf(fpout, "Bin1       %d\n", obj->value[2]);
         fprintf(fpout, "Bin2       %d\n", obj->value[4]);
         fprintf(fpout, "X          %d\n", obj->coord->x);
         fprintf(fpout, "Y          %d\n", obj->coord->y);
         fprintf(fpout, "Map        %d\n", obj->map);
         fprintf(fpout, "Room       %d\n", obj->in_room->vnum);
         fprintf(fpout, "Vnum       %d\n", obj->pIndexData->vnum);
         fprintf(fpout, "End\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_bin_data(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   BIN_DATA *blist;

   CREATE(blist, BIN_DATA, 1);

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

         case 'B':
            KEY("Bin1", blist->bin1, fread_number(fp));
            KEY("Bin2", blist->bin2, fread_number(fp));
            break;
         case 'M':
            KEY("Map", blist->map, fread_number(fp));
            break;
         case 'R':
            KEY("Room", blist->room, fread_number(fp));
            break;
         case 'V':
            KEY("Vnum", blist->vnum, fread_number(fp));
            break;
         case 'X':
            KEY("X", blist->x, fread_number(fp));
            break;
         case 'Y':
            KEY("Y", blist->y, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               OBJ_INDEX_DATA *iobj;
               OBJ_DATA *obj;
               ROOM_INDEX_DATA *room;

               if (blist->x && blist->y && blist->room && blist->vnum)
               {
                  room = get_room_index(blist->room);
                  iobj = get_obj_index(blist->vnum);
                  if (iobj != NULL && room != NULL)
                  {
                     obj = create_object(iobj, 0);
                     blist->serial = obj->serial;
                     obj_to_room(obj, room, room->first_person);
                     obj->coord->x = blist->x;
                     obj->coord->y = blist->y;
                     obj->map = blist->map;
                     SET_OBJ_STAT(obj, ITEM_ONMAP);
                     obj->value[2] = blist->bin1;
                     obj->value[4] = blist->bin2;
                  }
               }
               LINK(blist, first_bin, last_bin, next, prev);
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_bin_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_bin_data()
{
   FILE *fp;

   if ((fp = fopen(BIN_LIST, "r")) != NULL)
   {
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
            bug("Load_bin_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "BIN"))
         {
            fread_bin_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_bin_data: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open bin file", 0);
      exit(0);
   }
}

WBLOCK_DATA* fread_wblock_data(FILE * fp)
{
    char *word;
    bool fMatch;
    WBLOCK_DATA *wlist;

    CREATE(wlist, WBLOCK_DATA, 1);

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
             
          case 'K':
             KEY("Kills", wlist->kills, fread_number(fp));
             break;
             
          case 'L':
             KEY("Level", wlist->lvl, fread_number(fp));
             break;
             
          case 'M':
             KEY("Map", wlist->map, fread_number(fp));
             break; 

          case 'S':
             KEY("Startx", wlist->stx, fread_number(fp));
             KEY("Starty", wlist->sty, fread_number(fp));
             break;
             
          case 'T':
             KEY("TimeCheck", wlist->timecheck, fread_number(fp));
             break;

          case 'E':
             KEY("Endx", wlist->endx, fread_number(fp));
             KEY("Endy", wlist->endy, fread_number(fp));
             if (!str_cmp(word, "End"))
             {
                LINK(wlist, first_wblock, last_wblock, next, prev);
                return wlist;
             }
       }
    }
    return NULL;
}

void fread_iblock_data(FILE * fp, WBLOCK_DATA *wblock)
{
    char *word;
    bool fMatch;
    WINFO_DATA *ilist;
    
    if (wblock == NULL)
    {
       bug("fread_iblock_data: Trying to add an iblock structure to a Null wblock structure");
       return;
    }

    CREATE(ilist, WINFO_DATA, 1);

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
             
          case 'P':
             KEY("Pid", ilist->pid, fread_number(fp));
             break;
             
          case 'T':
             KEY("Time", ilist->time, fread_number(fp));
             break;

          case 'E':
             if (!str_cmp(word, "End"))
             {
                LINK(ilist, wblock->first_player, wblock->last_player, next, prev);
                return;
             }
       }
    }
    return;
}

void create_basic_blocks()
{
   int x;
   int y;
   int map;
   WBLOCK_DATA *wblock;
   
   for (map = 0; map < MAP_MAX; map++)
   {
      for (x = 1; x <= MAX_X; x = x+50)
      {
         for (y = 1; y <= MAX_Y; y = y+50)
         {
            CREATE(wblock, WBLOCK_DATA, 1);
            wblock->stx = x;
            wblock->endx = x+49;
            wblock->sty = y;
            wblock->endy = y+49;
            wblock->map = map;
            wblock->kills = 0;
            wblock->lvl = 10;
            wblock->first_player = NULL;
            wblock->last_player = NULL;
            LINK(wblock, first_wblock, last_wblock, next, prev);
         }
      }
   }
   save_wblock_data();
}

void load_wblock_data()
{
    FILE *fp;
    WBLOCK_DATA *wblock = NULL;
    int blocks = 0;

    if ((fp = fopen(WBLOCK_FILE, "r")) != NULL)
    {
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
             bug("Load_wblock_data: # not found.", 0);
             break;
          }

          word = fread_word(fp);
          if (!str_cmp(word, "WBLOCK"))
          {
             wblock = fread_wblock_data(fp);
             blocks++;
             continue;
          }
          else if (!str_cmp(word, "IBLOCK"))
          {
             fread_iblock_data(fp, wblock);
             continue;
          }
          else if (!str_cmp(word, "END"))
          {
             if (blocks == 0)
             {
                create_basic_blocks();
             }
             break;
          }
          else
          {
             bug("Load_wblock_data: bad section.", 0);
             continue;
          }
       }
       fclose(fp);
    }
    else
    {
       bug("Cannot open wblock file", 0);
       exit(0);
    }
}

void save_wblock_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   WBLOCK_DATA *blist;
   WINFO_DATA *ilist;

   sprintf(filename, "%s", WBLOCK_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   if (first_wblock)
   {
      for (blist = first_wblock; blist; blist = blist->next)
      {
         fprintf(fpout, "#WBLOCK\n");
         fprintf(fpout, "Startx       %d\n", blist->stx);
         fprintf(fpout, "Starty       %d\n", blist->sty);
         fprintf(fpout, "Endx         %d\n", blist->endx);
         fprintf(fpout, "Endy         %d\n", blist->endy);
         fprintf(fpout, "Map          %d\n", blist->map);
         fprintf(fpout, "Level        %d\n", blist->lvl);
         fprintf(fpout, "Kills        %d\n", blist->kills);
         fprintf(fpout, "TimeCheck    %d\n", blist->timecheck);
         fprintf(fpout, "End          \n");
         if (blist->first_player)
         {
            for (ilist = blist->first_player; ilist; ilist = ilist->next)
            {
               fprintf(fpout, "#IBLOCK\n");
               fprintf(fpout, "Pid       %d\n", ilist->pid);
               fprintf(fpout, "Time      %d\n", ilist->time);
               fprintf(fpout, "End       \n");
            }
         }
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void save_forge_data()
{
    FILE *fpout;
    char buf[MSL];
    char filename[MIL];
    FORGE_DATA *flist;

    sprintf(filename, "%s", FORGE_LIST);
    if ((fpout = fopen(filename, "w")) == NULL)
    {
       sprintf(buf, "Cannot open: %s for writing", filename);
       bug(buf, 0);
       return;
    }
    if (first_forge)
    {
       for (flist = first_forge; flist; flist = flist->next)
       {
          fprintf(fpout, "#FORGE\n");
          fprintf(fpout, "name            %s~\n", flist->name);
          fprintf(fpout, "vnum            %d\n", flist->vnum);
          fprintf(fpout, "slabs           %d\n", flist->slabnum);
          fprintf(fpout, "type            %d\n", flist->type);
          fprintf(fpout, "End\n");
       }
    }
    fprintf(fpout, "#END\n");
    fclose(fpout);
}

void fread_forge_data(FILE * fp)
 {
    char *word;
    bool fMatch;
    FORGE_DATA *flist;

    CREATE(flist, FORGE_DATA, 1);

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

          case 'N':
             KEY("Name", flist->name, fread_string(fp));
             break;

          case 'S':
             KEY("Slabs", flist->slabnum, fread_number(fp));
             break;
             
          case 'T':
             KEY("Type", flist->type, fread_number(fp));
             break;

          case 'V':
             KEY("Vnum", flist->vnum, fread_number(fp));
             break;

          case 'E':
             if (!str_cmp(word, "End"))
             {
                LINK(flist, first_forge, last_forge, next, prev);
                return;
             }
       }
 //    if ( !fMatch )
 // {
 //       sprintf( buf, "Fread_gem_data: no match: %s", word );
 //    bug( buf, 0 );
 // }
    }
}

void load_forge_data()
{
    FILE *fp;

    forge_num = 0;

    if ((fp = fopen(FORGE_LIST, "r")) != NULL)
    {
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
             bug("Load_forge_data: # not found.", 0);
             break;
          }

          word = fread_word(fp);
          if (!str_cmp(word, "FORGE"))
          {
             forge_num += 1;
             fread_forge_data(fp);
             continue;
          }
          else if (!str_cmp(word, "END"))
             break;
          else
          {
             bug("Load_forge_data: bad section.", 0);
             continue;
          }
       }
       fclose(fp);
    }
    else
    {
       bug("Cannot open forge file", 0);
       exit(0);
    }
}

void save_slab_data()
{
    FILE *fpout;
    char buf[MSL];
    char filename[MIL];
    SLAB_DATA *slist;

    sprintf(filename, "%s", SLAB_LIST);
    if ((fpout = fopen(filename, "w")) == NULL)
    {
       sprintf(buf, "Cannot open: %s for writing", filename);
       bug(buf, 0);
       return;
    }
    if (first_slab)
    {
       for (slist = first_slab; slist; slist = slist->next)
       {
          fprintf(fpout, "#SLAB\n");
          fprintf(fpout, "name        %s~\n", slist->name);
          fprintf(fpout, "vnum        %d\n", slist->vnum);
          fprintf(fpout, "adjective   %s~\n", slist->adj);
          fprintf(fpout, "kmob        %d\n", slist->kmob);
          fprintf(fpout, "qps         %d\n", slist->qps);
          fprintf(fpout, "End\n");
       }
    }
    fprintf(fpout, "#END\n");
    fclose(fpout);
}

void fread_slab_data(FILE * fp)
{
    char *word;
    bool fMatch;
    SLAB_DATA *slist;

    CREATE(slist, SLAB_DATA, 1);

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

          case 'N':
             KEY("Name", slist->name, fread_string(fp));
             break;

          case 'A':
             KEY("Adjective", slist->adj, fread_string(fp));
             break;
             
          case 'K':
             KEY("Kmob", slist->kmob, fread_number(fp));
             break;
             
          case 'Q':
             KEY("Qps", slist->qps, fread_number(fp));
             break;

          case 'V':
             KEY("Vnum", slist->vnum, fread_number(fp));
             break;

          case 'E':
             if (!str_cmp(word, "End"))
             {
                LINK(slist, first_slab, last_slab, next, prev);
                return;
             }
       }
 //    if ( !fMatch )
 // {
 //       sprintf( buf, "Fread_slab_data: no match: %s", word );
 //    bug( buf, 0 );
 // }
    }
}

 void load_slab_data()
 {
    FILE *fp;

    slab_num = 0;

    if ((fp = fopen(SLAB_LIST, "r")) != NULL)
    {
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
             bug("Load_slab_data: # not found.", 0);
             break;
          }

          word = fread_word(fp);
          if (!str_cmp(word, "SLAB"))
          {
             slab_num += 1;
             fread_slab_data(fp);
             continue;
          }
          else if (!str_cmp(word, "END"))
             break;
          else
          {
             bug("Load_slab_data: bad section.", 0);
             continue;
          }
       }
       fclose(fp);
    }
    else
    {
       bug("Cannot open slab file", 0);    
       exit(0);
    }
}      

void save_trainer_data()
{
   FILE *fp;
   char filename[MIL];
   char buf[MSL];
   TRAINER_DATA *tdata;

   sprintf(filename, "%s", TRAINER_LIST);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing.", filename);
      bug(buf, 0);
      return;
   }
   if (first_trainer)
   {
      for (tdata = first_trainer; tdata; tdata = tdata->next)
      {
         fprintf(fp, "#TRAINER\n");
         fprintf(fp, "Version    %d\n", TRAINERLISTVERSION);
         fprintf(fp, "Vnum       %d\n", tdata->vnum);
         fprintf(fp, "Sn         '%s' '%s' '%s' '%s' '%s'\n",
            IS_VALID_SN(tdata->sn[0]) ? skill_table[tdata->sn[0]]->name : "NONE",  
            IS_VALID_SN(tdata->sn[1]) ? skill_table[tdata->sn[1]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[2]) ? skill_table[tdata->sn[2]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[3]) ? skill_table[tdata->sn[3]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[4]) ? skill_table[tdata->sn[4]]->name : "NONE");
         fprintf(fp, "Sn2        '%s' '%s' '%s' '%s' '%s'\n",
            IS_VALID_SN(tdata->sn[5]) ? skill_table[tdata->sn[5]]->name : "NONE",  
            IS_VALID_SN(tdata->sn[6]) ? skill_table[tdata->sn[6]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[7]) ? skill_table[tdata->sn[7]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[8]) ? skill_table[tdata->sn[8]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[9]) ? skill_table[tdata->sn[9]]->name : "NONE"); 
         fprintf(fp, "Sn3        '%s' '%s' '%s' '%s' '%s'\n",
            IS_VALID_SN(tdata->sn[10]) ? skill_table[tdata->sn[10]]->name : "NONE",  
            IS_VALID_SN(tdata->sn[11]) ? skill_table[tdata->sn[11]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[12]) ? skill_table[tdata->sn[12]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[13]) ? skill_table[tdata->sn[13]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[14]) ? skill_table[tdata->sn[14]]->name : "NONE"); 
         fprintf(fp, "Sn4        '%s' '%s' '%s' '%s' '%s'\n",
            IS_VALID_SN(tdata->sn[15]) ? skill_table[tdata->sn[15]]->name : "NONE",  
            IS_VALID_SN(tdata->sn[16]) ? skill_table[tdata->sn[16]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[17]) ? skill_table[tdata->sn[17]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[18]) ? skill_table[tdata->sn[18]]->name : "NONE", 
            IS_VALID_SN(tdata->sn[19]) ? skill_table[tdata->sn[19]]->name : "NONE"); 
       
         fprintf(fp, "Mastery    %d %d %d %d %d %d %d %d %d %d\n", tdata->mastery[0], tdata->mastery[1],
            tdata->mastery[2], tdata->mastery[3], tdata->mastery[4], tdata->mastery[5], tdata->mastery[6],
            tdata->mastery[7], tdata->mastery[8], tdata->mastery[9]);
         fprintf(fp, "Mastery2   %d %d %d %d %d %d %d %d %d %d\n", tdata->mastery[10], tdata->mastery[11],
            tdata->mastery[12], tdata->mastery[13], tdata->mastery[14], tdata->mastery[15],
            tdata->mastery[16], tdata->mastery[17], tdata->mastery[18], tdata->mastery[19]);
         fprintf(fp, "End\n");
      }
   }
   fprintf(fp, "#END\n");
   fclose(fp);
}

void fread_trainer_data(FILE * fp)
{
   char buf[MSL];
   char *word;
   char *ln;
   int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10;
   bool fMatch;
   TRAINER_DATA *tdata;

   CREATE(tdata, TRAINER_DATA, 1);
   
   boughtsaveversion = 1; //just in case....

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

         case 'M':
            if (!str_cmp(word, "Mastery"))
            {
               ln = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = 0;
               sscanf(ln, "%d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10);
               tdata->mastery[0] = x1;
               tdata->mastery[1] = x2;
               tdata->mastery[2] = x3;
               tdata->mastery[3] = x4;
               tdata->mastery[4] = x5;
               tdata->mastery[5] = x6;
               tdata->mastery[6] = x7;
               tdata->mastery[7] = x8;
               tdata->mastery[8] = x9;
               tdata->mastery[9] = x10;
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Mastery2"))
            {
               ln = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = 0;
               sscanf(ln, "%d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10);
               tdata->mastery[10] = x1;
               tdata->mastery[11] = x2;
               tdata->mastery[12] = x3;
               tdata->mastery[13] = x4;
               tdata->mastery[14] = x5;
               tdata->mastery[15] = x6;
               tdata->mastery[16] = x7;
               tdata->mastery[17] = x8;
               tdata->mastery[18] = x9;
               tdata->mastery[19] = x10;
               fMatch = TRUE;
            }
            break;

         case 'S':
            if (!str_cmp(word, "Sn"))
            {
               if (boughtsaveversion <= 1)
               {
                  ln = fread_line(fp);
                  x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = 0;
                  sscanf(ln, "%d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10);
                  tdata->sn[0] = x1;
                  tdata->sn[1] = x2;
                  tdata->sn[2] = x3;
                  tdata->sn[3] = x4;
                  tdata->sn[4] = x5;
                  tdata->sn[5] = x6;
                  tdata->sn[6] = x7;
                  tdata->sn[7] = x8;
                  tdata->sn[8] = x9;
                  tdata->sn[9] = x10;
                  fMatch = TRUE;
               }
               else
               {
                  tdata->sn[0] = skill_lookup(fread_word(fp));
                  tdata->sn[1] = skill_lookup(fread_word(fp));
                  tdata->sn[2] = skill_lookup(fread_word(fp));
                  tdata->sn[3] = skill_lookup(fread_word(fp));
                  tdata->sn[4] = skill_lookup(fread_word(fp));
                  if (tdata->sn[0] < 0)
                     tdata->sn[0] = 0;
                  if (tdata->sn[1] < 0)
                     tdata->sn[1] = 0;
                  if (tdata->sn[2] < 0)
                     tdata->sn[2] = 0;
                  if (tdata->sn[3] < 0)
                     tdata->sn[3] = 0;
                  if (tdata->sn[4] < 0)
                     tdata->sn[4] = 0;
                  fMatch = TRUE;
               }
            }
            if (!str_cmp(word, "Sn2"))
            {
               if (boughtsaveversion <= 1)
               {
                  ln = fread_line(fp);
                  x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = 0;
                  sscanf(ln, "%d %d %d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10);
                  tdata->sn[10] = x1;
                  tdata->sn[11] = x2;
                  tdata->sn[12] = x3;
                  tdata->sn[13] = x4;
                  tdata->sn[14] = x5;
                  tdata->sn[15] = x6;
                  tdata->sn[16] = x7;
                  tdata->sn[17] = x8;
                  tdata->sn[18] = x9;
                  tdata->sn[19] = x10;
                  fMatch = TRUE;
               }
               else
               {
                  tdata->sn[5] = skill_lookup(fread_word(fp));
                  tdata->sn[6] = skill_lookup(fread_word(fp));
                  tdata->sn[7] = skill_lookup(fread_word(fp));
                  tdata->sn[8] = skill_lookup(fread_word(fp));
                  tdata->sn[9] = skill_lookup(fread_word(fp));
                  if (tdata->sn[5] < 0)
                     tdata->sn[5] = 0;
                  if (tdata->sn[6] < 0)
                     tdata->sn[6] = 0;
                  if (tdata->sn[7] < 0)
                     tdata->sn[7] = 0;
                  if (tdata->sn[8] < 0)
                     tdata->sn[8] = 0;
                  if (tdata->sn[9] < 0)
                     tdata->sn[9] = 0;
                  fMatch = TRUE;
               }
            }
            if (!str_cmp(word, "Sn3"))
            {
               tdata->sn[10] = skill_lookup(fread_word(fp));
               tdata->sn[11] = skill_lookup(fread_word(fp));
               tdata->sn[12] = skill_lookup(fread_word(fp));
               tdata->sn[13] = skill_lookup(fread_word(fp));
               tdata->sn[14] = skill_lookup(fread_word(fp));
               if (tdata->sn[10] < 0)
                  tdata->sn[10] = 0;
               if (tdata->sn[11] < 0)
                  tdata->sn[11] = 0;
               if (tdata->sn[12] < 0)
                  tdata->sn[12] = 0;
               if (tdata->sn[13] < 0)
                  tdata->sn[13] = 0;
               if (tdata->sn[14] < 0)
                  tdata->sn[14] = 0;
               fMatch = TRUE;
            }
            if (!str_cmp(word, "Sn4"))
            {
               tdata->sn[15] = skill_lookup(fread_word(fp));
               tdata->sn[16] = skill_lookup(fread_word(fp));
               tdata->sn[17] = skill_lookup(fread_word(fp));
               tdata->sn[18] = skill_lookup(fread_word(fp));
               tdata->sn[19] = skill_lookup(fread_word(fp));
               if (tdata->sn[15] < 0)
                  tdata->sn[15] = 0;
               if (tdata->sn[16] < 0)
                  tdata->sn[16] = 0;
               if (tdata->sn[17] < 0)
                  tdata->sn[17] = 0;
               if (tdata->sn[18] < 0)
                  tdata->sn[18] = 0;
               if (tdata->sn[19] < 0)
                  tdata->sn[19] = 0;
               fMatch = TRUE;
            }
            break;

         case 'V':
            KEY("Version", boughtsaveversion, fread_number(fp));
            KEY("Vnum", tdata->vnum, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(tdata, first_trainer, last_trainer, next, prev);
               return;
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_trainer_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_trainer_data()
{
   FILE *fp;

   if ((fp = fopen(TRAINER_LIST, "r")) != NULL)
   {
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
            bug("Load_trainer_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "TRAINER"))
         {
            fread_trainer_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_trainer_data: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open trainer file", 0);
      exit(0);
   }
}

void save_barena_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   BARENA_DATA *blist;

   sprintf(filename, "%s", BARENA_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   if (first_barena)
   {
      for (blist = first_barena; blist; blist = blist->next)
      {
         fprintf(fpout, "#BARENA\n");
         fprintf(fpout, "Name            %s~\n", blist->name);
         fprintf(fpout, "Wins            %d\n", blist->wins);
         fprintf(fpout, "Losses          %d\n", blist->losses);
         fprintf(fpout, "Ties            %d\n", blist->ties);
         fprintf(fpout, "Games           %d\n", blist->games);
         fprintf(fpout, "Numavg          %d\n", blist->numavg);
         fprintf(fpout, "Kills		   %d\n", blist->kills);
         fprintf(fpout, "Pkills          %d\n", blist->pkills);
         fprintf(fpout, "Pdeaths         %d\n", blist->pdeaths);
         fprintf(fpout, "Pranking        %d\n", blist->pranking);
         fprintf(fpout, "End\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_barena_data(FILE * fp)
{
   char *word;
   bool fMatch;
   BARENA_DATA *blist;

   CREATE(blist, BARENA_DATA, 1);

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

         case 'G':
            KEY("Games", blist->games, fread_number(fp));
            break;

         case 'K':
            KEY("Kills", blist->kills, fread_number(fp));
            break;

         case 'L':
            KEY("Losses", blist->losses, fread_number(fp));
            break;

         case 'N':
            KEY("Name", blist->name, fread_string_nohash(fp));
            KEY("Numavg", blist->numavg, fread_number(fp));
            break;

         case 'P':
            KEY("Pdeaths", blist->pdeaths, fread_number(fp));
            KEY("Pkills", blist->pkills, fread_number(fp));
            KEY("Pranking", blist->pranking, fread_number(fp));
            break;

         case 'T':
            KEY("Ties", blist->ties, fread_number(fp));
            break;

         case 'W':
            KEY("Wins", blist->wins, fread_number(fp));

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(blist, first_barena, last_barena, next, prev);
               return;
            }
      }
//    if ( !fMatch )
// {
//       sprintf( buf, "Fread_barena_data: no match: %s", word );
//    bug( buf, 0 );
// }
   }
}

void load_barena_data()
{
   FILE *fp;

   if ((fp = fopen(BARENA_FILE, "r")) != NULL)
   {
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
            bug("Load_barena_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "BARENA"))
         {
            fread_barena_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_barena_data: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open barena file", 0);
      exit(0);
   }
}

void save_box_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   BOX_DATA *boxlist;

   sprintf(filename, "%s", BOX_LIST);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   if (first_box)
   {
      for (boxlist = first_box; boxlist; boxlist = boxlist->next)
      {
         fprintf(fpout, "#BOX\n");
         fprintf(fpout, "vnum		%d\n", boxlist->vnum);
         fprintf(fpout, "End\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_box_data(FILE * fp)
{
   char *word;
   bool fMatch;
   BOX_DATA *boxlist;

   CREATE(boxlist, BOX_DATA, 1);

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

         case 'V':
            KEY("Vnum", boxlist->vnum, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(boxlist, first_box, last_box, next, prev);
               return;
            }
      }
//    if ( !fMatch )
// {
//       sprintf( buf, "Fread_box_data: no match: %s", word );
//    bug( buf, 0 );
// }
   }
}

void load_box_data()
{
   FILE *fp;

   box_num = 0;

   if ((fp = fopen(BOX_LIST, "r")) != NULL)
   {
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
            bug("Load_box_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "BOX"))
         {
            box_num += 1;
            fread_box_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_box_data: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open box file", 0);
      exit(0);
   }
}

void save_gem_data()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   GEM_DATA *glist;

   sprintf(filename, "%s", GEM_LIST);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   if (first_gem)
   {
      for (glist = first_gem; glist; glist = glist->next)
      {
         fprintf(fpout, "#GEM\n");
         fprintf(fpout, "vnum		%d\n", glist->vnum);
         fprintf(fpout, "cost           %d\n", glist->cost);
         fprintf(fpout, "rarity         %d\n", glist->rarity);
         fprintf(fpout, "End\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_gem_data(FILE * fp)
{
   char *word;
   bool fMatch;
   GEM_DATA *glist;

   CREATE(glist, GEM_DATA, 1);

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
            KEY("Cost", glist->cost, fread_number(fp));
            break;
        
         case 'R':
            KEY("Rarity", glist->rarity, fread_number(fp));
            break;

         case 'V':
            KEY("Vnum", glist->vnum, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(glist, first_gem, last_gem, next, prev);
               return;
            }
      }
//    if ( !fMatch )
// {
//       sprintf( buf, "Fread_gem_data: no match: %s", word );
//    bug( buf, 0 );
// }
   }
}

void load_gem_data()
{
   FILE *fp;

   gem_num = 0;

   if ((fp = fopen(GEM_LIST, "r")) != NULL)
   {
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
            bug("Load_gem_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "GEM"))
         {
            gem_num += 1;
            fread_gem_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_gem_data: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open gem file", 0);
      exit(0);
   }
}

void fwrite_authlist()
{
   FILE *fpout;
   char buf[MSL];
   char filename[MIL];
   AUTHORIZE_DATA *alist;

   sprintf(filename, "%s", AUTHLIST_FILE);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      sprintf(buf, "Cannot open: %s for writing", filename);
      bug(buf, 0);
      return;
   }
   if (first_authorized)
   {
      for (alist = first_authorized; alist; alist = alist->next)
      {
         fprintf(fpout, "#AUTH\n");
         fprintf(fpout, "Name            %s~\n", alist->name);
         fprintf(fpout, "LastName        %s~\n", alist->lastname);
         fprintf(fpout, "Authedby        %s~\n", alist->authedby);
         fprintf(fpout, "Authdate        %s~\n", alist->authdate);
         fprintf(fpout, "Ip		   %s~\n", alist->host);
         fprintf(fpout, "End\n");
      }
   }
   fprintf(fpout, "#END\n");
   fclose(fpout);
}

void fread_authlist(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   AUTHORIZE_DATA *alist;

   CREATE(alist, AUTHORIZE_DATA, 1);

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
            KEY("Authedby", alist->authedby, fread_string_nohash(fp));
            KEY("Authdate", alist->authdate, fread_string_nohash(fp));
            break;
         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(alist, first_authorized, last_authorized, next, prev);
               return;
            }
         case 'I':
            KEY("Ip", alist->host, fread_string_nohash(fp));
            break;
         case 'L':
            KEY("LastName", alist->lastname, fread_string_nohash(fp));
            break;
         case 'N':
            KEY("Name", alist->name, fread_string_nohash(fp));
            break;
      }
      if (!fMatch)
      {
         sprintf(buf, "Fread_social: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_authlist()
{
   FILE *fp;

   if ((fp = fopen(AUTHLIST_FILE, "r")) != NULL)
   {
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
            bug("Load_authlist: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "AUTH"))
         {
            fread_authlist(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_authlist: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open authlist file", 0);
      exit(0);
   }
}


void fread_social(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   SOCIALTYPE *social;

   CREATE(social, SOCIALTYPE, 1);

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
            KEY("CharNoArg", social->char_no_arg, fread_string_nohash(fp));
            KEY("CharFound", social->char_found, fread_string_nohash(fp));
            KEY("CharAuto", social->char_auto, fread_string_nohash(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               if (!social->name)
               {
                  bug("Fread_social: Name not found", 0);
                  free_social(social);
                  return;
               }
               if (!social->char_no_arg)
               {
                  bug("Fread_social: CharNoArg not found", 0);
                  free_social(social);
                  return;
               }
               add_social(social);
               return;
            }
            break;

         case 'N':
            KEY("Name", social->name, fread_string_nohash(fp));
            break;

         case 'O':
            KEY("OthersNoArg", social->others_no_arg, fread_string_nohash(fp));
            KEY("OthersFound", social->others_found, fread_string_nohash(fp));
            KEY("OthersAuto", social->others_auto, fread_string_nohash(fp));
            break;

         case 'V':
            KEY("VictFound", social->vict_found, fread_string_nohash(fp));
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_social: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_socials()
{
   FILE *fp;

   if ((fp = fopen(SOCIAL_FILE, "r")) != NULL)
   {
      top_sn = 0;
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
            bug("Load_socials: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "SOCIAL"))
         {
            fread_social(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_socials: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open socials.dat", 0);
      exit(0);
   }
}

/*
 *  Added the flags Aug 25, 1997 --Shaddai
 */

void fread_command(FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;
   CMDTYPE *command;

   CREATE(command, CMDTYPE, 1);
   command->lag_count = 0; /* can't have caused lag yet... FB */
   command->flags = 0; /* Default to no flags set */

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
            KEY("Code", command->do_fun, skill_function(fread_word(fp)));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               if (!command->name)
               {
                  bug("Fread_command: Name not found", 0);
                  free_command(command);
                  return;
               }
               if (!command->do_fun)
               {
                  bug("Fread_command: Function not found", 0);
                  free_command(command);
                  return;
               }
               add_command(command);
               return;
            }
            break;

         case 'F':
            KEY("FCommand", command->fcommand, fread_number(fp));
            KEY("Flags", command->flags, fread_number(fp));
            break;

         case 'L':
            KEY("Level", command->level, fread_number(fp));
            KEY("Log", command->log, fread_number(fp));
            break;

         case 'N':
            KEY("Name", command->name, fread_string_nohash(fp));
            break;

         case 'P':
            /* KEY( "Position", command->position, fread_number(fp) ); */
            if (!str_cmp(word, "Position"))
            {
               fMatch = TRUE;
               command->position = fread_number(fp);
               if (command->position < 100)
               {
                  switch (command->position)
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        command->position = 6;
                        break;
                     case 6:
                        command->position = 8;
                        break;
                     case 7:
                        command->position = 9;
                        break;
                     case 8:
                        command->position = 12;
                        break;
                     case 9:
                        command->position = 13;
                        break;
                     case 10:
                        command->position = 14;
                        break;
                     case 11:
                        command->position = 15;
                        break;
                  }
               }
               else
                  command->position -= 100;
               break;
            }
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_command: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_commands()
{
   FILE *fp;

   if ((fp = fopen(COMMAND_FILE, "r")) != NULL)
   {
      top_sn = 0;
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
            bug("Load_commands: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "COMMAND"))
         {
            fread_command(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_commands: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
   }
   else
   {
      bug("Cannot open commands.dat", 0);
      exit(0);
   }

}

void save_classes()
{
   write_class_file(0);
}

/*
 * Tongues / Languages loading/saving functions			-Altrag
 */
void fread_cnv(FILE * fp, LCNV_DATA ** first_cnv, LCNV_DATA ** last_cnv)
{
   LCNV_DATA *cnv;
   char letter;

   for (;;)
   {
      letter = fread_letter(fp);
      if (letter == '~' || letter == EOF)
         break;
      ungetc(letter, fp);
      CREATE(cnv, LCNV_DATA, 1);

      cnv->old = str_dup(fread_word(fp));
      cnv->olen = strlen(cnv->old);
      cnv->new = str_dup(fread_word(fp));
      cnv->nlen = strlen(cnv->new);
      fread_to_eol(fp);
      LINK(cnv, *first_cnv, *last_cnv, next, prev);
   }
}

void load_tongues()
{
   FILE *fp;
   LANG_DATA *lng;
   char *word;
   char letter;

   if (!(fp = fopen(TONGUE_FILE, "r")))
   {
      perror("Load_tongues");
      return;
   }
   for (;;)
   {
      letter = fread_letter(fp);
      if (letter == EOF)
         return;
      else if (letter == '*')
      {
         fread_to_eol(fp);
         continue;
      }
      else if (letter != '#')
      {
         bug("Letter '%c' not #.", letter);
         exit(0);
      }
      word = fread_word(fp);
      if (!str_cmp(word, "end"))
         return;
      fread_to_eol(fp);
      CREATE(lng, LANG_DATA, 1);
      lng->name = STRALLOC(word);
      fread_cnv(fp, &lng->first_precnv, &lng->last_precnv);
      lng->alphabet = fread_string(fp);
      fread_cnv(fp, &lng->first_cnv, &lng->last_cnv);
      fread_to_eol(fp);
      LINK(lng, first_lang, last_lang, next, prev);
   }
   return;
}

void fwrite_langs(void)
{
   FILE *fp;
   LANG_DATA *lng;
   LCNV_DATA *cnv;

   if (!(fp = fopen(TONGUE_FILE, "w")))
   {
      perror("fwrite_langs");
      return;
   }
   for (lng = first_lang; lng; lng = lng->next)
   {
      fprintf(fp, "#%s\n", lng->name);
      for (cnv = lng->first_precnv; cnv; cnv = cnv->next)
         fprintf(fp, "'%s' '%s'\n", cnv->old, cnv->new);
      fprintf(fp, "~\n%s~\n", lng->alphabet);
      for (cnv = lng->first_cnv; cnv; cnv = cnv->next)
         fprintf(fp, "'%s' '%s'\n", cnv->old, cnv->new);
      fprintf(fp, "\n");
   }
   fprintf(fp, "#end\n\n");
   fclose(fp);
   return;
}

//not used anymore
const struct hometown_type hometown_table[] = {

/*      {       "name",         recall, death   },      */

   {"Rafermand", 21001, 21194},

   {"New Thalos", 31405, 31404},

   {NULL, 21001, 21194}

};
