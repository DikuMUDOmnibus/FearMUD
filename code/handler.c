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
 *		        Main structure manipulation module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


extern int top_exit;
extern int top_ed;
extern int top_affect;
extern int cur_qobjs;
extern int cur_qchars;
extern CHAR_DATA *gch_prev;
extern OBJ_DATA *gobj_prev;

CHAR_DATA *cur_char;
ROOM_INDEX_DATA *cur_room;
bool cur_char_died;
ch_ret global_retcode;

int saving_mount_on_quit;
int cur_obj;
int cur_obj_serial;
int top_map_mob;
bool cur_obj_extracted;
obj_ret global_objcode;

OBJ_DATA *group_object(OBJ_DATA * obj1, OBJ_DATA * obj2);
bool in_magic_container(OBJ_DATA * obj);

/* Strlen_color by Rusty, useful for skipping over colors */
int strlen_color(char *argument)
{
   char *str;
   int i, length;

   str = argument;
   if (argument == NULL)
      return 0;

   for (length = i = 0; i < strlen(argument); ++i)
   {
      if ((str[i] != '&') && (str[i] != '^'))
         ++length;
      if ((str[i] == '&') || (str[i] == '^'))
      {
         if ((str[i] == '&') && (str[i + 1] == '&'))
            length = 2 + length;
         else if ((str[i] == '^') && (str[i + 1] == '^'))
            length = 2 + length;
         else
            --length;
      }
   }

   return length;
}

/*
 * Retrieve a character's trusted level for permission checking.
 */
sh_int get_trust(CHAR_DATA * ch)
{
   if (IS_NPC(ch))
      return LEVEL_PC;
   if (ch->desc && ch->desc->original)
      ch = ch->desc->original;
   if (ch->trust != 0)
      return ch->trust;
   if (ch->level >= LEVEL_IMMORTAL && IS_RETIRED(ch)) /* Tracker1 */
      return LEVEL_IMMORTAL; /* Tracker1 */

   return ch->level;
}

/*
 * New FLevel trust return, for use of the flevel attribute. - Xerves 3-12-98
 */
sh_int get_ftrust(CHAR_DATA * ch)
{
   if (ch->desc && ch->desc->original)
      ch = ch->desc->original;

   if (IS_NPC(ch))
      return ch->level;

   if (IS_RETIRED(ch))
      return LEVEL_IMMORTAL; /* Tracker1 */

   if (ch->level > ch->pcdata->flevel)
      return ch->level;

   return ch->pcdata->flevel;
}


/*
 * Retrieve a character's age.
 */
sh_int get_age(CHAR_DATA * ch)
{
   if (IS_NPC(ch))
      return 17;
   return 17 + (ch->played + (current_time - ch->pcdata->logon)) / 72000;
}



/*
 * Retrieve character's current strength.
 */
sh_int get_curr_str(CHAR_DATA * ch)
{
   sh_int max;

   max = 25;

   return URANGE(3, ch->perm_str + URANGE(-4, ch->mod_str, 4), max);
}



/*
 * Retrieve character's current intelligence.
 */
sh_int get_curr_int(CHAR_DATA * ch)
{
   sh_int max;

   max = 25;

   return URANGE(3, ch->perm_int + URANGE(-4, ch->mod_int, 4), max);
}



/*
 * Retrieve character's current wisdom.
 */
sh_int get_curr_wis(CHAR_DATA * ch)
{
   sh_int max;

   max = 25;

   return URANGE(3, ch->perm_wis + URANGE(-4, ch->mod_wis, 4), max);
}



/*
 * Retrieve character's current dexterity.
 */
sh_int get_curr_dex(CHAR_DATA * ch)
{
   sh_int max;

   max = 25;

   return URANGE(3, ch->perm_dex + URANGE(-4, ch->mod_dex, 4), max);
}



/*
 * Retrieve character's current constitution.
 */
sh_int get_curr_con(CHAR_DATA * ch)
{
   sh_int max;

   max = 25;

   return URANGE(3, ch->perm_con + URANGE(-4, ch->mod_con, 4), max);
}

/*
 * Retrieve character's current charisma.
 */
sh_int get_curr_cha(CHAR_DATA * ch)
{
   sh_int max;

   max = 25;

   return URANGE(3, ch->perm_cha + URANGE(-4, ch->mod_cha, 4), max);
}

/*
 * Retrieve character's current luck.
 */
sh_int get_curr_lck(CHAR_DATA * ch)
{
   sh_int max;

   max = 25;

   return URANGE(3, ch->perm_lck + URANGE(-4, ch->mod_lck, 4), max);
}

sh_int get_curr_agi(CHAR_DATA * ch)
{
   sh_int max;

   max = 100;

   return URANGE(10, ch->perm_agi + URANGE(-15, ch->mod_agi, 15), max);
}

/* Below 2 functions are used to return points and masteries of spells.  Mainly to
   compute an estimated value for mobs -- Xerves 2/00 */
sh_int get_mastered_value(CHAR_DATA *ch, int sn)
{
   if (!IS_NPC(ch))
   {
      return URANGE(0, ch->pcdata->ranking[sn], MAX_RANKING);
   }
   else
   {
      if (ch->max_hit <= 28)
         return get_mastery_value(ch, sn, 0, SPOWER_MIN);
      else if (ch->max_hit <= 77)
         return get_mastery_value(ch, sn, 0, SPOWER_LOW);
      else if (ch->max_hit <= 200)
         return get_mastery_value(ch, sn, 0, SPOWER_MED);
      else if (ch->max_hit <= 694)
         return get_mastery_value(ch, sn, 0, SPOWER_HI);
      else if (ch->max_hit <= 1022)
         return get_mastery_value(ch, sn, 0, SPOWER_GREAT);
      else if (ch->max_hit <= 3000)
         return get_mastery_value(ch, sn, 0, SPOWER_GREATER);
      else
         return get_mastery_value(ch, sn, 0, SPOWER_GREATEST);
   }
}
sh_int get_learned_value(CHAR_DATA *ch, int sn)
{
   if (!IS_NPC(ch))
   {
      return URANGE(0, ch->pcdata->learned[sn], MAX_SKPOINTS);
   }
   else
   {
      if (ch->max_hit <= 28)
         return get_point_value(ch, sn, 0, SPOWER_MIN);
      else if (ch->max_hit <= 77)
         return get_point_value(ch, sn, 0, SPOWER_LOW);
      else if (ch->max_hit <= 200)
         return get_point_value(ch, sn, 0, SPOWER_MED);
      else if (ch->max_hit <= 694)
         return get_point_value(ch, sn, 0, SPOWER_HI);
      else if (ch->max_hit <= 1022)
         return get_point_value(ch, sn, 0, SPOWER_GREAT);   
      else if (ch->max_hit <= 3000)
         return get_point_value(ch, sn, 0, SPOWER_GREATER);
      else
         return get_point_value(ch, sn, 0, SPOWER_GREATEST);
   }
}

/* Below 2 functions are used to return points and masteries of spells.  Mainly to
   compute an estimated value for mobs -- Xerves 2/00 */
sh_int get_point_value(CHAR_DATA * ch, int sn, int isobj, int lv)
{
   if (!IS_NPC(ch) && isobj == 0 && ch->pcdata->learned[sn] > 1)
      return ch->pcdata->learned[sn];
   else if (lv >= SPOWER_MIN)
   {
      if (lv == SPOWER_MIN)
         return number_range(1, 2);
      if (lv == SPOWER_LOW)
         return number_range(2, 4);
      if (lv == SPOWER_MED)
         return number_range(3, 6);
      if (lv == SPOWER_HI)
         return number_range(5, 8);        
      if (lv == SPOWER_GREAT)
         return number_range(7, 10);
      if (lv == SPOWER_GREATER)
         return number_range(8, 12);
      if (lv == SPOWER_GREATEST)
         return number_range(10, 14);
   }
   else if (IS_NPC(ch) && isobj == 0)
   {
      if (ch->max_hit <= 28)
         return get_point_value(ch, sn, 0, SPOWER_MIN);
      else if (ch->max_hit <= 77)
         return get_point_value(ch, sn, 0, SPOWER_LOW);
      else if (ch->max_hit <= 200)
         return get_point_value(ch, sn, 0, SPOWER_MED);
      else if (ch->max_hit <= 694)
         return get_point_value(ch, sn, 0, SPOWER_HI);
      else if (ch->max_hit <= 1022)
         return get_point_value(ch, sn, 0, SPOWER_GREAT);   
      else if (ch->max_hit <= 3000)
         return get_point_value(ch, sn, 0, SPOWER_GREATER);
      else
         return get_point_value(ch, sn, 0, SPOWER_GREATEST);
   }
   return 1;
}

sh_int get_mastery_value(CHAR_DATA * ch, int sn, int isobj, int lv)
{
   int di;

   if (!IS_NPC(ch) && isobj == 0 && ch->pcdata->ranking[sn] > 1)
      return ch->pcdata->ranking[sn];
   else if (lv >= SPOWER_MIN)
   {
      //mob or obj
      di = dice(1, 20);
      
      if (lv == SPOWER_MIN)
         return 1;
      if (lv == SPOWER_LOW)
      {
         if (di >= 15)
            return 2;
         else
            return 1;
      }
      if (lv == SPOWER_MED)
      {
         if (di >= 14)
            return 2;
         else
            return 1;
      }
      if (lv == SPOWER_HI)
      {
         if (di >= 12)
            return 3;
         else
            return 2;
      }            
      if (lv == SPOWER_GREAT)
      {
         if (di >= 4)
            return 4;
         else
            return 3;
      }
      if (lv == SPOWER_GREATER)
      {
         if (di <= 15)
            return 4;
         else
            return 5;
      }
      if (lv == SPOWER_GREATEST)
      {
         if (di <= 5)
            return 4;
         else if (di >= 16)
            return 6;
         else
            return 5;
      }
   }
   else if (IS_NPC(ch) && isobj == 0)
   {
      if (ch->max_hit <= 28)
         return get_mastery_value(ch, sn, 0, SPOWER_MIN);
      else if (ch->max_hit <= 77)
         return get_mastery_value(ch, sn, 0, SPOWER_LOW);
      else if (ch->max_hit <= 200)
         return get_mastery_value(ch, sn, 0, SPOWER_MED);
      else if (ch->max_hit <= 694)
         return get_mastery_value(ch, sn, 0, SPOWER_HI);
      else if (ch->max_hit <= 1022)
         return get_mastery_value(ch, sn, 0, SPOWER_GREAT);
      else if (ch->max_hit <= 3000)
         return get_mastery_value(ch, sn, 0, SPOWER_GREATER);
      else
         return get_mastery_value(ch, sn, 0, SPOWER_GREATEST);
   }
   return 1;
}

/* Below 6 handler functions are used in information obtaining in the new skill system.
   There are other functions in act_info.c and in mud_comm.c -- Xerves 2/00 */
// Retreives a Mastery name from its Number.
char *get_mastery_name(int mastery)
{
   switch (mastery)
   {
      default:
         return "Unknown";
      case 1:
         return "Beginner";
      case 2:
         return "Novice";
      case 3:
         return "Expert";
      case 4:
         return "Master";
      case 5:
         return "Elite";
      case 6:
         return "Flawless";
   }
}

char *get_tier_name(int mastery)
{
   switch (mastery)
   {
      default:
         return "Unknown";
      case 1:
         return "Tier 1";
      case 2:
         return "Tier 2";
      case 3:
         return "Tier 3";
      case 4:
         return "Tier 4";
   }
}

char *get_wplevel(int power)
{
   switch (power)
   {
      default:
         return "None";
      case SPOWER_MIN:
         return "Minimum";
      case SPOWER_LOW:
         return "Low";
      case SPOWER_MED:
         return "Medium";
      case SPOWER_HI:
         return "High";
      case SPOWER_GREAT:
         return "Great";
      case SPOWER_GREATER:
         return "Greater";
      case SPOWER_GREATEST:
         return "Greatest";
   }
}

char *get_sphere_name2(int sphere)
{
   switch(sphere)
   {
      default:
         return "Unknown";
      case 1:
         return "Combat";
      case 2:
         return "Agent";
      case 3:
         return "Wizardry";
      case 4:
         return "Divine";
      case 5:
         return "Spiritual";
   }
}

char *get_sphere_name(int sphere)
{
   switch(sphere)
   {
      default:
         return "  Unknown   ";
      case 1:
         return "   Combat   ";
      case 2:
         return "   Agent    ";
      case 3:
         return "  Wizardry  ";
      case 4:
         return "   Divine   ";
      case 5:
         return " Spiritual  ";
   }
}

/*
 * Retreives a Group name from its Number. -- Xerves 1/00
 */
char *get_group_name(int group)
{
   switch (group)
   {
      default:
         return "Unknown";
      case 1:
         return "Air";
      case 2:
         return "Earth";
      case 3:
         return "Energy";
      case 4:
         return "Fire";
      case 5:
         return "Water";
      case 6:
         return "Divine";
      case 7:
         return "Strikes";
      case 8:
         return "Body";
      case 9:
         return "Mind";
      case 10:
         return "-------";
      case 11:
         return "Survival";
      case 12:
         return "Stealth";
      case 13:
         return "Thieving";
      case 14:
         return "Battle Skills";
      case 15:
         return "-------";
      case 16:
         return "Tactical";
      case 17:
         return "Styles";
      case 18:
         return "-------";
   }
}

/*
 * Retreives a Group name from its Number.  Should be used in do_skills only since it
 * includes extra values to break up Fire for Cleric and Fire for Mage
 */
char *get_group_name2(int group)
{
   switch (group)
   {
      default:
         return "Unknown";
      case 1:
         return "Air";
      case 2:
         return "Earth";
      case 3:
         return "Energy";
      case 4:
         return "Fire";
      case 5:
         return "Water";
      case 6:
         return "Divine";
      case 7:
         return "Strikes";
      case 8:
         return "Body";
      case 9:
         return "Mind";
      case 10:
         return "-------";
      case 11:
         return "Survival";
      case 12:
         return "Stealth";
      case 13:
         return "Thieving";
      case 14:
         return "Battle Skills";
      case 15:
         return "--------";
      case 16:
         return "Tactical";
      case 17:
         return "Styles";
      case 18:
         return "-------";
      case 19:
         return "Air";
      case 20:
         return "Earth";
      case 21:
         return "Energy";
      case 22:
         return "Fire";
      case 23:
         return "Water";
   }
}

//Used in do_skills to determine percents
int is_part_sphere(int sphere, int group)
{  
   if (sphere == 1) //Warrior
      if (group == 14 || group == 16 || group == 17)
         return 1;
   if (sphere == 2) //Thief
      if (group == 11 || group == 12 || group == 13)
         return 1;
   if (sphere == 3) //Mage
      if (group == 1 || group == 2 || group == 3 || group == 4 || group == 5)
         return 1;
   if (sphere == 4) //Cleric
      if (group == 6 || group == 19 || group == 20 || group == 21 || group == 22 || group == 23)
         return 1;   
   if (sphere == 5) //Hand to Hand
      if (group == 7 || group == 8 || group == 9)
         return 1;
   return 0;
}
// Returns Mastery number for the name
int get_mastery_num(char *mastery)
{
   char buf[MSL];

   sprintf(buf, mastery);
   buf[0] = UPPER(buf[0]);

   if (!str_cmp(buf, "Beginner"))
      return 1;
   if (!str_cmp(buf, "Novice") || !str_prefix(buf, "Apprentice"))
      return 2;
   if (!str_cmp(buf, "Expert"))
      return 3;
   if (!str_cmp(buf, "Master"))
      return 4;
   if (!str_cmp(buf, "Elite"))
      return 5;
   if (!str_cmp(buf, "Flawless"))
      return 6;

   return -1;
}

// Get sphere number from sphere name -- Xerves 7/04
sh_int issphere(char *argument)
{
   sh_int sphere = -1;
   if (!str_cmp(argument, "Combat"))
      sphere = 1;
   if (!str_cmp(argument, "Agent"))
      sphere = 2;
   if (!str_cmp(argument, "Wizardry"))
      sphere = 3;
   if (!str_cmp(argument, "Divine"))
      sphere = 4;
   if (!str_cmp(argument, "Spiritual"))
      sphere = 5;
      
   return sphere;
}
 
/* Get group number from group name -- Xerves 1/00 */
sh_int isgroup(char *argument)
{
   sh_int group = -1;
   if (!str_cmp(argument, "Air"))
      group = 1;
   if (!str_cmp(argument, "Earth"))
      group = 2;
   if (!str_cmp(argument, "Energy"))
      group = 3;
   if (!str_cmp(argument, "Fire"))
      group = 4;
   if (!str_cmp(argument, "Water"))
      group = 5;
   if (!str_cmp(argument, "Divine"))
      group = 6;
   if (!str_cmp(argument, "Strikes"))
      group = 7;
   if (!str_cmp(argument, "Body"))
      group = 8;
   if (!str_cmp(argument, "Mind"))
      group = 9;
   if (!str_cmp(argument, "-------"))
      group = 10;
   if (!str_cmp(argument, "Survival"))
      group = 11;
   if (!str_cmp(argument, "Stealth"))
      group = 12;
   if (!str_cmp(argument, "Thieving"))
      group = 13;
   if (!str_cmp(argument, "Battle Skills"))
      group = 14;
   if (!str_cmp(argument, "--------"))
      group = 15;
   if (!str_cmp(argument, "Tactical"))
      group = 16;
   if (!str_cmp(argument, "Styles"))
      group = 17;
   if (!str_cmp(argument, "-------"))
      group = 18;
      
   return group;
}

//Movement lag, more lag the more tired you get (adding in later running, etc)
int movement_lag(CHAR_DATA *ch, int beats)
{
   int percent;
   int level;
   
   if (beats < 0)
      beats = 1;
   
   percent = ch->move * 100 / 1000;
   
   if (IS_NPC(ch) || LEARNED(ch, gsn_shadowfoot) >= 1)
   {
      if (percent >= 70)
         beats = 1;
      else if (percent >= 60)
         beats = beats * 3 / 10;
      else if (percent >= 50)
         beats = beats * 5 / 10;
      else if (percent >= 40)
         beats = beats * 8 / 10;
      else if (percent >= 30)
         beats = beats * 11 / 10;
      else if (percent >= 15)
         beats = beats * 15 / 10;
      else if (percent < 15)
         beats = beats * 20 / 10;
   }
   else
   {
      if (percent >= 80)
         beats = 1;
      else if (percent >= 70)
         beats = beats * 5 / 10;
      else if (percent >= 60)
         beats = beats * 8 / 10;
      else if (percent >= 50)
         beats = beats * 12 / 10;
      else if (percent >= 40)
         beats = beats * 15 / 10;
      else if (percent >= 15)
         beats = beats * 20 / 10;
      else if (percent < 15)
         beats = beats * 30 / 10;
   }   
   level = POINT_LEVEL(LEARNED(ch, gsn_featherfoot), MASTERED(ch, gsn_featherfoot));
   if (level > 0 && !IS_NPC(ch))
   {
      beats = beats * (100-(level/2)) / 100;
   }
   if (beats < 1)
      beats = 1;
   if (!IS_NPC(ch) && LEARNED(ch, gsn_featherfoot) >= 1)
      learn_from_success(ch, gsn_featherfoot, NULL);
   if (!IS_NPC(ch) && LEARNED(ch, gsn_featherback) >= 1)
      learn_from_success(ch, gsn_featherback, NULL);
   return UMIN(20, beats); //5 seconds max, anything over is just evil
}

void update_movement_points(CHAR_DATA *ch, int move)
{
   int mv;
   
   if (IS_NPC(ch))
   {
      if (!xIS_SET(ch->act, ACT_MOUNTSAVE))
         return;
   }
   
   if (!IS_NPC(ch))
   {
      mv = number_range(move*.15, move*.3);
      if (sysdata.stat_gain <= 3)
         mv = number_range(move*.25, move*.5);
      else if (sysdata.stat_gain >= 5)
         mv = number_range(move*.4, move*.8);
      
   
      if (ch->mover >= 35 && ch->mover < 45)
         mv *= .8;
      else if (ch->mover < 55)
         mv *= .6;
      else if (ch->mover < 65)
         mv *= .5;
      else if (ch->mover <= 74)
         mv *= .3;
      else if (ch->mover < 94)
         mv *= .25;
      else
         mv *= .2;
      
      if (mv == 0)
         mv = 1;
      
      if (ch->mover >= (74+get_talent_increase(ch, 8)) && ch->pcdata->per_move >= 300)
         mv = 0;
            
      ch->pcdata->per_move += mv;
         
      if (ch->pcdata->per_move > 1000)
      {
         ch->mover++;
         send_to_char("&C*****************************************\n\r", ch);
         send_to_char("&C******You Gain 1 Point in Endurance******\n\r", ch);
         send_to_char("&C*****************************************\n\r", ch);
         ch->pcdata->per_move = 0;
      }  
      if (ch->pcdata->per_move < 0)
      {
         ch->mover--;
         send_to_char("&c*****************************************\n\r", ch);
         send_to_char("&c******You Lose 1 Point in Endurance******\n\r", ch);
         send_to_char("&c*****************************************\n\r", ch);
         ch->pcdata->per_move = 999;
      }
   }
   else
   {  
      mv = number_range(move*.1, move*.25);
      if (sysdata.stat_gain <= 3)
         mv = number_range(move*.2, move*.4);
      else if (sysdata.stat_gain >= 5)
         mv = number_range(move*.35, move*.7);
   
   
      if (ch->mover >= 65 && ch->mover < 75)
         mv *= .7;
      else if (ch->mover < 85)
         mv *= .5;
      else if (ch->mover < 95)
         mv *= .3;
      else if (ch->mover < 105)
         mv *= .2;
      else if (ch->mover < 115)
         mv *= .15;
      else if (ch->mover < 125)
         mv *= .1;
      else if (ch->mover < 135)
         mv *= .07;
      
      if (mv == 0)
         mv = 1;
      
      if (ch->mover == 135 && ch->m1 >= 300)
         mv = 0;
            
      ch->m1 += mv;
         
      if (ch->m1 > 1000)
      {
         ch->mover++;
         send_to_char("&C*************************************************\n\r", ch);
         send_to_char("&C******Your Mount Gains 1 Point in Endurance******\n\r", ch);
         send_to_char("&C*************************************************\n\r", ch);
         ch->m1 = 0;
      }  
   }
}

void str_load_increase(CHAR_DATA *ch, int load)
{
   int mstr = 0;
   int str = ch->perm_str;
   int bstr;
   
   if (load >= 51 && load <= 55)
      mstr += number_range(0, 2);
   else if (load >= 56 && load <= 60)
      mstr += number_range(1, 2);
   else if (load >= 61 && load <= 65)
      mstr += number_range(2, 3);
   else if (load >= 66 && load <= 70)
      mstr += number_range(2, 4);
   else if (load >= 71 && load <= 75)
      mstr += number_range(3, 4);
   else if (load >= 76 && load <= 80)
      mstr += number_range(3, 5);
   else if (load >= 81 && load <= 85)
      mstr += number_range(4, 5);
   else if (load >= 86 && load <= 90)
      mstr += number_range(4, 6);
   else if (load >= 91 && load <= 96)
      mstr += number_range(5, 6);
   else
      mstr += number_range(6, 7);
      
   if (sysdata.stat_gain <= 3)
      mstr = number_range(150*mstr/100, 180*mstr/100);
   else if (sysdata.stat_gain >= 5)
      mstr = number_range(250*mstr/100, 300*mstr/100);
      
   bstr = 14 + race_table[ch->race]->str_plus;
   if (str == bstr - 4)
      mstr *= 2;
   if (str == bstr - 3)
      mstr *= 1.7;
   if (str == bstr - 2)
      mstr *= 1.5;
   if (str == bstr - 1)
      mstr *= 1.2;
   if (str == bstr)
      mstr *= 1;
   if (str == bstr + 1)
      mstr *= .85;
   if (str == bstr + 2)
      mstr *= .7;
   if (str == bstr + 3)
      mstr *= .6;
   if (str == bstr + 4)
      mstr *= .4;
   if (str == bstr + 5)
      mstr *= .3;
   if (str > bstr + 5) //Base + 5 should be the max unless you screwed it up
      mstr = 0;
   else
   {
      if (mstr == 0)
         mstr = 1;
   }
      
   if (ch->perm_str == (14 + race_table[ch->race]->str_plus + race_table[ch->race]->str_range) && ch->pcdata->per_str >= 3000)
      mstr = 0;
   ch->pcdata->per_str += mstr;
   if (ch->pcdata->per_str > 10000)
   {
      ch->perm_str++;
      send_to_char("&R**************************************\n\r", ch);
      send_to_char("&R*****You Gain 1 Point of Strength*****\n\r", ch);
      send_to_char("&R**************************************\n\r", ch);
      ch->pcdata->per_str = 0;
   }
}

int calculate_movement_cost(int move, CHAR_DATA *ch)
{
   int load;
   int weight;
   int level;
   int addmove = 0;
   
   if (ch->con_lleg <= -1)
      addmove += UMAX(1, number_range(move*1.5, move*2.3));
   else if (ch->con_lleg <= 10)
      addmove += UMAX(1, number_range(move*1.3, move*1.5));
   else if (ch->con_lleg <= 30)
      addmove += UMAX(1, number_range(move*1.15, move*1.3));
   else if (ch->con_lleg <= 50)
      addmove += UMAX(1, number_range(move*1.05, move*1.15));
      
   if (ch->con_rleg <= -1)
      addmove += UMAX(1, number_range(move*1.5, move*2.3));
   else if (ch->con_rleg <= 10)
      addmove += UMAX(1, number_range(move*1.3, move*1.5));
   else if (ch->con_rleg <= 30)
      addmove += UMAX(1, number_range(move*1.15, move*1.3));
   else if (ch->con_rleg <= 50)
      addmove += UMAX(1, number_range(move*1.05, move*1.15));
   
   move+=addmove;
   
   if ((IS_NPC(ch) && !xIS_SET(ch->act, ACT_MOUNTSAVE)) || IS_IMMORTAL(ch))
      return 1; //for now
   else
   {
      if (ch->mover < 15)
         move = move * (5 - (.1 * (ch->mover-5)));
      else if (ch->mover < 25)
         move = move * (4 - (.1 * (ch->mover-15)));
      else if (ch->mover < 35)
         move = move * (3 - (.1 * (ch->mover-25)));
      else if (ch->mover < 45)
         move = move * (2 - (.1 * (ch->mover-35)));
      else if (ch->mover < 55)
         move = move * (1.3 - (.07 * (ch->mover-45)));
      else if (ch->mover < 65)
         move = move * (.95 - (.035 * (ch->mover-55)));
      else if (ch->mover < 75)
         move = move * (.8 - (.015 * (ch->mover-65)));
      else if (ch->mover < 85)
         move = move * (.7 - (.01 * (ch->mover-75)));
      else if (ch->mover < 95)
         move = move * (.6 - (.01 * (ch->mover-85)));
      else if (ch->mover < 105)
         move = move * (.5 - (.01 * (ch->mover-95)));
      else if (ch->mover < 115)
         move = move * (.4 - (.01 * (ch->mover-105)));
      else if (ch->mover < 125)
         move = move * (.3 - (.01 * (ch->mover-115)));
      else if (ch->mover < 135)
         move = move * (.2 - (.01 * (ch->mover-125)));
      else if (ch->mover >= 135)
         move = move * .1;
         
      if (!IS_NPC(ch) && ch->pcdata->learned[gsn_shadowfoot] > 0)
      {
         level = POINT_LEVEL(GET_POINTS(ch, gsn_shadowfoot, 0, 1), GET_MASTERY(ch, gsn_shadowfoot, 0, 1));  
         move = move * (100 - (15+level/2)) / 100;
         learn_from_success(ch, gsn_shadowfoot, NULL);
      }
      if (move < 1)
         move = 1;
      
      if (!IS_NPC(ch))
         weight = get_ch_carry_weight(ch);
      else
      {
         if (ch->master)
            weight = get_ch_carry_weight(ch->master);
         else
            weight = 1;
      }
      
      load = 100 * weight / can_carry_w(ch);   
      if (load > 50)
      {
         if (!IS_NPC(ch))
            str_load_increase(ch, load);
         if (!IS_NPC(ch) && ch->pcdata->learned[gsn_strongfoot] > 0)
         {
            level = POINT_LEVEL(GET_POINTS(ch, gsn_strongfoot, 0, 1), GET_MASTERY(ch, gsn_strongfoot, 0, 1));  
            load = load - (level*2/5);
            learn_from_success(ch, gsn_strongfoot, NULL);
         }
         if (load <= 50)
            ;
         else if (load >= 51 && load <= 55)
            move *= 1.1;
         else if (load >= 56 && load <= 60)
            move *= 1.3;
         else if (load >= 61 && load <= 65)
            move *= 1.5;
         else if (load >= 66 && load <= 70)
            move *= 1.7;
         else if (load >= 71 && load <= 75)
            move *= 2;
         else if (load >= 76 && load <= 80)
            move *= 2.3;
         else if (load >= 81 && load <= 85)
            move *= 2.6;
         else if (load >= 86 && load <= 90)
            move *= 3;
         else if (load >= 91 && load <= 96)
            move *= 4;
         else
            move *= 5;
      }
   }
   if (ch->rider)
      move *= 2;
   return move;
}

/* Below are hitroll/damroll checks to get the hitroll normally plus bonuses based
   on mastery */
int get_hit_roll(CHAR_DATA * ch)
{
   int hitroll;

   hitroll = ch->hitroll + str_app[get_curr_str(ch)].tohit + (2 - (abs(ch->mental_state) / 10));

   return hitroll;
}

int get_dam_roll(CHAR_DATA * ch)
{
   int damroll;


   damroll = ch->damroll + ch->damplus + str_app[get_curr_str(ch)].todam;

   if (ch->mental_state > 5 && ch->mental_state < 15)
      damroll++;
   return damroll;
}

//Defines what the room is, 1 is safe, 2 is noloot, 3 is anitem, 4 is freekill                       
//Default is noloot      
int check_room_pk(CHAR_DATA * ch)
{
   //Do the Wilderness first
   //Wilderness checks, the room itself should ALWAYS be set to noloot
   if (IN_WILDERNESS(ch))
   {
      int stx, sty, endx, endy, x, y;

      if (sysdata.resetgame)
         return 4;
      stx = ch->coord->x - 5;
      sty = ch->coord->y - 5;
      endx = ch->coord->x + 5;
      endy = ch->coord->y + 5;

      if (stx < 1)
         stx = 1;
      if (sty < 1)
         sty = 1;
      if (endx > MAX_X)
         endx = MAX_X;
      if (endy > MAX_Y)
         endy = MAX_Y;
      for (y = sty; y < endy + 1; y += 1)
      {
         for (x = stx; x < endx + 1; x += 1)
         {
            if (map_sector[ch->map][x][y] == SECT_ROAD)
               return 2;
            if (map_sector[ch->map][x][y] == SECT_BRIDGE)
               return 2;
            if (map_sector[ch->map][x][y] == SECT_PATH)
            {
               if (abs(ch->coord->y - y) <= 3 && abs(ch->coord->x - x) <= 3)
                  return 2;
            }
         }
      }
      return 3;
   }
   //A safe room is a safe room no matter what.
   if (xIS_SET(ch->in_room->room_flags, ROOM_TSAFE))
      return 1;
   if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
      return 1;
   if (xIS_SET(ch->in_room->room_flags, ROOM_FREEKILL))
      return 4;
   if (xIS_SET(ch->in_room->room_flags, ROOM_ANITEM))
      return 3;
   if (xIS_SET(ch->in_room->room_flags, ROOM_NOLOOT))
      return 2;

   //Check areas next, room goes before area in priority
   if (IS_SET(ch->in_room->area->flags, AFLAG_NOKILL))
      return 1;
   if (IS_SET(ch->in_room->area->flags, AFLAG_NOLOOT))
      return 2;
   if (IS_SET(ch->in_room->area->flags, AFLAG_ANITEM))
      return 3;
   if (IS_SET(ch->in_room->area->flags, AFLAG_FREEKILL))
      return 4;

   if (sysdata.resetgame)
      return 4;
   else
      return 2;
}

bool in_same_room(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (IN_WILDERNESS(ch))
   {
      if (ch->coord->x == victim->coord->x && ch->coord->y == victim->coord->y
         && ch->map == victim->map)
         return TRUE;
   }
   else
   {
      if (ch->in_room == victim->in_room)
         return TRUE;
   }
   return FALSE;
}

bool in_hellmaze(CHAR_DATA * ch)
{
   return FALSE;
}

// Send out message, doesn't use a channel, don't want to spam poor players
void hunt_message(CHAR_DATA * ch, CHAR_DATA * victim, int range)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *tochar;
   int message = 1;
   char buf[MSL];
   char buf2[MSL];

   for (d = first_descriptor; d; d = d->next)
   {

      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2)
         tochar = d->character;
      else
         continue;

      if (tochar->coord->x > -1 || tochar->coord->y > -1 || tochar->map > -1)
      {
         if (abs(ch->coord->x - tochar->coord->x) <= range && abs(ch->coord->y - tochar->coord->y) <= range)
         {
            message = number_range(1, 6);
            if (message == 1)
               sprintf(buf, "My first target has escaped, so &R[%s&R]%s will DIE instead.", victim->name, char_color_str(AT_GOSSIP, tochar));
            else if (message == 2)
               sprintf(buf, "I seek more battles, and &R[%s&R]%s looks like an easy target.", victim->name, char_color_str(AT_GOSSIP, tochar));
            else if (message == 3)
               sprintf(buf, "&R[%s&R]%s shall die today, I will take delight in my victory.", victim->name, char_color_str(AT_GOSSIP, tochar));
            else if (message == 4)
               sprintf(buf, "Come here &R[%s&R]%s, I wish to introduce you to your maker.", victim->name, char_color_str(AT_GOSSIP, tochar));
            else if (message == 5)
               sprintf(buf, "Poor &R[%s&R]%s, today was just not a good day to step outside.", victim->name, char_color_str(AT_GOSSIP, tochar));
            else
               sprintf(buf, "Life shall end for &R[%s&R]%s today, how sad.", victim->name, char_color_str(AT_GOSSIP, tochar));

            sprintf(buf2, "&c&w%s [&cyells&c&w] %s'%s'", ch->short_descr, char_color_str(AT_GOSSIP, tochar), buf);
            act(AT_GOSSIP, buf2, ch, NULL, tochar, TO_VICT);
         }
      }
   }
   return;
}

void find_next_target(CHAR_DATA * ch)
{
   CHAR_DATA *victim;
   int range, num, count, npeace, ht, x;
   CHAR_DATA *pvictim = NULL;
   int eqlevel = 8;

   if (xIS_SET(ch->miflags, KM_ATTACKN) || xIS_SET(ch->miflags, KM_ATTACKE) || xIS_SET(ch->miflags, KM_ATTACKA))
   {  
      count = x = 0;

      range = ch->m6;

      if (xIS_SET(ch->miflags, KM_ATTACKN))
         npeace = 1;
      else if (xIS_SET(ch->miflags, KM_ATTACKE))
         npeace = 0;
      else
         npeace = -1;

      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if (victim->coord->x > -1 || victim->coord->y > -1 || victim->map > -1)
         {
            if (abs(ch->coord->x - victim->coord->x) <= range && abs(ch->coord->y - victim->coord->y) <= range)
            {
               if (IS_NPC(victim))
                  ht = victim->m4;
               else
                  ht = victim->pcdata->hometown; 

               if (victim->map == ch->map && victim->level < LEVEL_IMMORTAL && can_see(ch, victim) && ch->m4 != ht 
               && (npeace == -1 || kingdom_table[ch->m4]->peace[ht] <= npeace) && !xIS_SET(victim->act, ACT_PACIFIST))
               {
                  if (!get_eq_char(victim, WEAR_NECK))
                  {
                     pvictim = victim;
                     eqlevel = 1;
                  }
                  if (!get_eq_char(victim, WEAR_HEAD) && eqlevel > 2)
                  {
                     pvictim = victim;
                     eqlevel = 2;
                  }
                  if (!get_eq_char(victim, WEAR_ARM_R) && eqlevel > 3)
                  {
                     pvictim = victim;
                     eqlevel = 3;
                  }
                  if (!get_eq_char(victim, WEAR_ARM_L) && eqlevel > 4)
                  {
                     pvictim = victim;
                     eqlevel = 4;
                  }
                  if (!get_eq_char(victim, WEAR_LEG_R) && eqlevel > 5)
                  {
                     pvictim = victim;
                     eqlevel = 5;
                  }
                  if (!get_eq_char(victim, WEAR_LEG_L) && eqlevel > 6)
                  {
                     pvictim = victim;
                     eqlevel = 6;
                  }
                  if (!get_eq_char(victim, WEAR_BODY) && eqlevel > 7)
                  {
                     pvictim = victim;
                     eqlevel = 7;
                  }
                  if (!pvictim)
                     pvictim = victim;
               }
            }
         }
      }
      if (!pvictim)
      {
         stop_hating(ch);
         stop_hunting(ch);
         do_yell(ch, "Lost my target, no new ones sited, standing down.");
         return;
      }
      else
      {
         hunt_message(ch, pvictim, range + 1);
         start_hunting(ch, pvictim);
         return;
      }
   }
   if (xIS_SET(ch->miflags, KM_ATTACKC) || xIS_SET(ch->miflags, KM_ATTACKH))
   {
      count = x = 0;
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if ((xIS_SET(ch->miflags, KM_ATTACKC) && get_wear_cloak(victim)) || 
             (xIS_SET(ch->miflags, KM_ATTACKH) && get_wear_hidden_cloak(victim)))
         {
            if (IN_SAME_ROOM(ch, victim))
            {
               if (IS_NPC(victim))
                  ht = victim->m4;
               else
                  ht = victim->pcdata->hometown;
               if (can_see_map(ch, victim) && !is_safe(ch, victim) && ch->m4 != ht && !IS_IMMORTAL(victim))
               {
                  count++;
               }
            }
         }
      }
      if (count == 0)
      {
         stop_hating(ch);
         stop_hunting(ch);
         do_yell(ch, "Lost my target, no new ones sited, standing down.");
         return;
      }
      num = number_range(1, count);
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if ((xIS_SET(ch->miflags, KM_ATTACKC) && get_wear_cloak(victim)) || 
             (xIS_SET(ch->miflags, KM_ATTACKH) && get_wear_hidden_cloak(victim)))
         {
            if (IN_SAME_ROOM(ch, victim))
            {
               if (IS_NPC(victim))
                  ht = victim->m4;
               else
                  ht = victim->pcdata->hometown;
               if (x == num && can_see_map(ch, victim) && !is_safe(ch, victim) && ch->m4 != ht && !IS_IMMORTAL(victim))
               {
                  hunt_message(ch, victim, 3);
                  start_hunting(ch, victim);
                  return;
               }
               else
               {
                  if (can_see_map(ch, victim) && !is_safe(ch, victim) && ch->m4 != ht && !IS_IMMORTAL(victim))
                     x++;
               }
            }
         }
      }
   }   
}

// 0 is out of Battle, 1 is while on map, range is different
void find_next_hunt(CHAR_DATA * ch, int type)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   int count = 0;
   int x = 1;
   int num;
   int range;

   if (type == 0)
      range = 10;
   else
      range = 10;

   if (xIS_SET(ch->act, ACT_MILITARY)) //They hunt mobs too, different function
   {
      find_next_target(ch);
      return;
   }

   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2)
         victim = d->character;
      else
         continue;

      if (victim->coord->x > -1 || victim->coord->y > -1 || victim->map > -1)
      {
         if (abs(ch->coord->x - victim->coord->x) <= range && abs(ch->coord->y - victim->coord->y) <= range)
         {
            if (victim->map == ch->map && can_see(ch, victim) && victim->level < LEVEL_IMMORTAL)
            {
               count++;
            }
         }
      }
   }
   if (count == 0)
   {
      extract_char(ch, TRUE);
      return;
   }
   num = number_range(1, count);
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2)
         victim = d->character;
      else
         continue;

      if (victim->coord->x > -1 || victim->coord->y > -1 || victim->map > -1)
      {
         if (abs(ch->coord->x - victim->coord->x) <= range && abs(ch->coord->y - victim->coord->y) <= range)
         {
            if (x == num && (ch->map == victim->map && victim->level < LEVEL_IMMORTAL && can_see(ch, victim)) )
            {
               hunt_message(ch, victim, range + 1);
               start_hunting(ch, victim);
               start_hating(ch, victim);
               return;
            }
            else
            {
               if (ch->map == victim->map && victim->level < LEVEL_IMMORTAL && can_see(ch, victim))
                  x++;
            }
         }
      }
   }
}

bool find_sector(CHAR_DATA * ch, int sector)
{
   if (IS_ONMAP_FLAG(ch))
   {
      if (map_sector[ch->map][ch->coord->x][ch->coord->y] == sector)
      {
         return TRUE;
      }
   }
   else
   {
      if (ch->in_room->sector_type == sector)
      {
         return TRUE;
      }
   }

   return FALSE;
}

/*
 * Retrieve a character's carry capacity.
 * Vastly reduced (finally) due to containers		-Thoric
 */
int can_carry_n(CHAR_DATA * ch)
{
   int penalty = 0;

   if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
      return 1000;

   if (IS_NPC(ch) && (xIS_SET(ch->act, ACT_PET) || xIS_SET(ch->act, ACT_MOUNTSAVE)))
      return 0;

   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_IMMORTAL))
      return 1000;

   if (get_eq_char(ch, WEAR_WIELD))
      ++penalty;
   if (get_eq_char(ch, WEAR_DUAL_WIELD))
      ++penalty;
   if (get_eq_char(ch, WEAR_MISSILE_WIELD))
      ++penalty;
   if (get_eq_char(ch, WEAR_NOCKED))
      ++penalty;
   if (get_eq_char(ch, WEAR_SHIELD))
      ++penalty;
   return URANGE(8, 8 + get_curr_dex(ch) - 12 - penalty, 20);
}



/*
 * Retrieve a character's carry capacity.
 */
int can_carry_w(CHAR_DATA * ch)
{
   int level = POINT_LEVEL(LEARNED(ch, gsn_featherback), MASTERED(ch, gsn_featherback));
   int weight;
   
   if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
      return 1000000;

   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOUNTSAVE))
      return str_app[get_curr_str(ch)].carry*2;

   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_IMMORTAL))
      return 1000000;

   weight = str_app[get_curr_str(ch)].carry;
   if (level > 0)
   {
      weight += weight * UMIN(40, level/2) / 100;
   }
   return weight;
}


/*
 * See if a player/mob can take a piece of prototype eq		-Thoric
 */
bool can_take_proto(CHAR_DATA * ch)
{
   if (IS_IMMORTAL(ch))
      return TRUE;
   else if (IS_NPC(ch) && xIS_SET(ch->act, ACT_PROTOTYPE))
      return TRUE;
   else
      return FALSE;
}


/*
 * See if a string is one of the names of an object.
 */
bool is_name(const char *str, char *namelist)
{
   char name[MIL];

   for (;;)
   {
      namelist = one_argument(namelist, name);
      if (name[0] == '\0')
         return FALSE;
      if (!str_cmp(str, name))
         return TRUE;
   }
}

/* Will count users on furniture -- Xerves */
int count_users(OBJ_DATA * obj)
{
   CHAR_DATA *fch;
   int count = 0;

   if (obj->in_room == NULL)
      return 0;

   for (fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room)
      if (fch->on == obj)
         count++;

   return count;
}

int max_weight(OBJ_DATA * obj)
{
   CHAR_DATA *fch;
   int weight = 0;

   if (obj->in_room == NULL)
      return 200000;

   for (fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room)
      if (fch->on == obj)
         weight = weight + fch->weight;

   return weight;
}

bool is_name_prefix(const char *str, char *namelist)
{
   char name[MIL];

   for (;;)
   {
      namelist = one_argument(namelist, name);
      if (name[0] == '\0')
         return FALSE;
      if (!str_prefix(str, name))
         return TRUE;
   }
}

/*
 * See if a string is one of the names of an object.		-Thoric
 * Treats a dash as a word delimiter as well as a space
 */
bool is_name2(const char *str, char *namelist)
{
   char name[MIL];

   for (;;)
   {
      namelist = one_argument2(namelist, name);
      if (name[0] == '\0')
         return FALSE;
      if (!str_cmp(str, name))
         return TRUE;
   }
}

bool is_name2_prefix(const char *str, char *namelist)
{
   char name[MIL];

   for (;;)
   {
      namelist = one_argument2(namelist, name);
      if (name[0] == '\0')
         return FALSE;
      if (!str_prefix(str, name))
         return TRUE;
   }
}

/*								-Thoric
 * Checks if str is a name in namelist supporting multiple keywords
 */
bool nifty_is_name(char *str, char *namelist)
{
   char name[MIL];

   if (!str || str[0] == '\0')
      return FALSE;

   for (;;)
   {
      str = one_argument2(str, name);
      if (name[0] == '\0')
         return TRUE;
      if (!is_name2(name, namelist))
         return FALSE;
   }
}

bool nifty_is_name_prefix(char *str, char *namelist)
{
   char name[MIL];

   if (!str || str[0] == '\0')
      return FALSE;

   for (;;)
   {
      str = one_argument2(str, name);
      if (name[0] == '\0')
         return TRUE;
      if (!is_name2_prefix(name, namelist))
         return FALSE;
   }
}

void room_affect(ROOM_INDEX_DATA * pRoomIndex, AFFECT_DATA * paf, bool fAdd)
{
   if (fAdd)
   {
      switch (paf->location)
      {
         case APPLY_ROOMFLAG:
         case APPLY_SECTORTYPE:
            break;
         case APPLY_ROOMLIGHT:
            pRoomIndex->light += paf->modifier;
            break;
         case APPLY_TELEVNUM:
         case APPLY_TELEDELAY:
            break;
      }
   }
   else
   {
      switch (paf->location)
      {
         case APPLY_ROOMFLAG:
         case APPLY_SECTORTYPE:
            break;
         case APPLY_ROOMLIGHT:
            pRoomIndex->light -= paf->modifier;
            break;
         case APPLY_TELEVNUM:
         case APPLY_TELEDELAY:
            break;
      }
   }
}

/*
 * Modify a skill (hopefully) properly			-Thoric
 *
 * On "adding" a skill modifying affect, the value set is unimportant
 * upon removing the affect, the skill it enforced to a proper range.
 */
void modify_skill(CHAR_DATA * ch, int sn, int mod, bool fAdd)
{
   if (!IS_NPC(ch))
   {
      if (fAdd)
         ch->pcdata->learned[sn] += mod;
      else
         ch->pcdata->learned[sn] = URANGE(0, ch->pcdata->learned[sn] + mod, MAX_SKPOINTS);
   }
}

int base_resis_values(CHAR_DATA *ch, int type)
{
   int fire, water, air, earth, energy, holy, unholy, nonmagic, magic, poison, paralysis;
   
   fire=water=air=earth=energy=holy=unholy=nonmagic=magic=poison=paralysis=100;
   
   if (IS_SET(ch->elementb, ELEMENT_FIRE))
   {
      fire -=25;
      water +=50;
   }
   if (IS_SET(ch->elementb, ELEMENT_WATER))
   {
      fire +=35;
      water -=20;
   }
   if (IS_SET(ch->elementb, ELEMENT_AIR))
   {
      air -=35;
      earth +=50;
   }
   if (IS_SET(ch->elementb, ELEMENT_EARTH))
   {
      air +=35;
      earth -=25;
   }
   if (IS_SET(ch->elementb, ELEMENT_ENERGY))
   {
      energy -=40;
      air +=15;
      earth +=15;
      fire +=15;
      water +=15;
   }
   if (IS_SET(ch->elementb, ELEMENT_DIVINE))
   {
      holy -=60;
      unholy +=200;
      energy -= 10;
      air -=10;
      earth -=10;
      fire -=10;
      water -=10;
   }
   if (IS_SET(ch->elementb, ELEMENT_UNHOLY))
   {
      unholy -=60;
      holy +=200;
      energy -= 10;
      air -=10;
      earth -=10;
      fire -=10;
      water -=10;
   }        
   if (ch->race == RACE_OGRE)
   {
      nonmagic -= 20;
      fire -= 25;
      water -= 30;
      magic += 100;
      poison -= 30;
      paralysis -= 30;
   }
   if (ch->race == RACE_DWARF)
   {  
      water -= 25;
   }
   if (ch->race == RACE_HOBBIT)
   {   
      fire -= 25;
   }
   if (ch->race == RACE_FAIRY)
   {
      magic -= 35;
      nonmagic += 50;
   }         
   if (type == 1)
      return fire; 
   if (type == 2)
      return water;
   if (type == 3)
      return air;
   if (type == 4)
      return earth;
   if (type == 5)
      return energy;
   if (type == 6)
      return holy;
   if (type == 7)
      return unholy;
   if (type == 8)
      return nonmagic;
   if (type == 9)
      return magic;
   if (type == 10)
      return poison;
   if (type == 11)
      return paralysis;
      
   return 100;
}     

/*
 * Apply or remove an affect to a character.
 */
void affect_modify(CHAR_DATA * ch, AFFECT_DATA * paf, sh_int fAdd)
{
   OBJ_DATA *wield;
   int mod;
   sh_int eq = 0;
   struct skill_type *skill;
   ch_ret retcode;
   int resist;
   int susc;
   int diff;
   //AFFECT_DATA *saf;

   mod = paf->modifier;

   if (fAdd > 1)
   {
      fAdd -= 2;
      eq = 1;
   }


   if (fAdd)
   {
      xSET_BITS(ch->affected_by, paf->bitvector);
      if (paf->location % REVERSE_APPLY == APPLY_RECURRINGSPELL)
      {
         mod = abs(mod);
         if (IS_VALID_SN(mod) && (skill = skill_table[mod]) != NULL && skill->type == SKILL_SPELL)
            xSET_BIT(ch->affected_by, AFF_RECURRINGSPELL);
         else
            bug("affect_modify(%s) APPLY_RECURRINGSPELL with bad sn %d", ch->name, mod);
         return;
      }
   }
   else
   {
      xREMOVE_BITS(ch->affected_by, paf->bitvector);
      /*
       * might be an idea to have a duration removespell which returns
       * the spell after the duration... but would have to store
       * the removed spell's information somewhere...  -Thoric
       * (Though we could keep the affect, but disable it for a duration)
       */
      if ((paf->location % REVERSE_APPLY) == APPLY_REMOVESPELL)
         return;

      if (paf->location % REVERSE_APPLY == APPLY_RECURRINGSPELL)
      {
         mod = abs(mod);
         if (!IS_VALID_SN(mod) || (skill = skill_table[mod]) == NULL || skill->type != SKILL_SPELL)
            bug("affect_modify(%s) APPLY_RECURRINGSPELL with bad sn %d", ch->name, mod);
         xREMOVE_BIT(ch->affected_by, AFF_RECURRINGSPELL);
         return;
      }

      switch (paf->location % REVERSE_APPLY)
      {
         case APPLY_AFFECT:
            REMOVE_BIT(ch->affected_by.bits[0], mod);
            return;
         case APPLY_EXT_AFFECT:
            xREMOVE_BIT(ch->affected_by, mod);
            return;
         case APPLY_RESISTANT:
            REMOVE_BIT(ch->resistant, mod);
            return;
         case APPLY_IMMUNE:
            REMOVE_BIT(ch->immune, mod);
            return;
         case APPLY_SUSCEPTIBLE:
            REMOVE_BIT(ch->susceptible, mod);
            return;
         case APPLY_WEARSPELL: /* affect only on wear */
            return;
         case APPLY_REMOVE:
            SET_BIT(ch->affected_by.bits[0], mod);
            return;
      }
      mod = 0 - mod;
   }

   switch (paf->location % REVERSE_APPLY)
   {
      default:
         bug("Affect_modify: unknown location %d.", paf->location);
         return;

      case APPLY_NONE:
         break;
      case APPLY_ARMOR:
         ch->apply_armor += mod;
         break;
      case APPLY_SHIELD:
         ch->apply_shield += mod;
         break;
      case APPLY_STONE:
         ch->apply_stone += mod;
         break;
      case APPLY_SANCTIFY:
         ch->apply_sanctify += mod;
         break;
      case APPLY_WMOD:
         if (!fAdd)
         {
            if (mod == -ch->apply_wmod)
               ch->apply_wmod += mod;
            if (ch->apply_wmod < 0)
               ch->apply_wmod = 0;
         }
         else
         {
            if (mod < ch->apply_wmod || ch->apply_wmod == 0)
            {
               ch->apply_wmod = mod;
            }
         }
         break;
         
      case APPLY_RENERGY:
         resist = (ch->apply_res_energy[1] == 0 ? 100 : ch->apply_res_energy[1]);
         susc = (ch->apply_res_energy[2] == 0 ? 100 : ch->apply_res_energy[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_energy[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_energy[1])
                  ch->apply_res_energy[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_energy[2])
                  ch->apply_res_energy[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_energy[1] = 100;
               diff = base_resis_values(ch, 5)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_energy[2]) //Susc
            {
               ch->apply_res_energy[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_energy[1]) //Resist
            {
               ch->apply_res_energy[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_energy[1] = -500;
         }
         resist = (ch->apply_res_energy[1] == 0 ? 100 : ch->apply_res_energy[1]);
         susc = (ch->apply_res_energy[2] == 0 ? 100 : ch->apply_res_energy[2]);
         if (ch->apply_res_energy[1] == -500)
            ch->apply_res_energy[0] = -500;
         else
            ch->apply_res_energy[0] = 100 + diff - (100-resist) + (susc-100);
         break;  
         
      case APPLY_RWATER:
         resist = (ch->apply_res_water[1] == 0 ? 100 : ch->apply_res_water[1]);
         susc = (ch->apply_res_water[2] == 0 ? 100 : ch->apply_res_water[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_water[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_water[1])
                  ch->apply_res_water[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_water[2])
                  ch->apply_res_water[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_water[1] = 100;
               diff = base_resis_values(ch, 2)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_water[2]) //Susc
            {
               ch->apply_res_water[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_water[1]) //Resist
            {
               ch->apply_res_water[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_water[1] = -500;
         }
         resist = (ch->apply_res_water[1] == 0 ? 100 : ch->apply_res_water[1]);
         susc = (ch->apply_res_water[2] == 0 ? 100 : ch->apply_res_water[2]);
         if (ch->apply_res_water[1] == -500)
            ch->apply_res_water[0] = -500;
         else
            ch->apply_res_water[0] = 100 + diff - (100-resist) + (susc-100);
         break;   
         
      case APPLY_REARTH:
         resist = (ch->apply_res_earth[1] == 0 ? 100 : ch->apply_res_earth[1]);
         susc = (ch->apply_res_earth[2] == 0 ? 100 : ch->apply_res_earth[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_earth[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_earth[1])
                  ch->apply_res_earth[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_earth[2])
                  ch->apply_res_earth[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_earth[1] = 100;
               diff = base_resis_values(ch, 4)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_earth[2]) //Susc
            {
               ch->apply_res_earth[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_earth[1]) //Resist
            {
               ch->apply_res_earth[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_earth[1] = -500;
         }
         resist = (ch->apply_res_earth[1] == 0 ? 100 : ch->apply_res_earth[1]);
         susc = (ch->apply_res_earth[2] == 0 ? 100 : ch->apply_res_earth[2]);
         if (ch->apply_res_earth[1] == -500)
            ch->apply_res_earth[0] = -500;
         else
            ch->apply_res_earth[0] = 100 + diff - (100-resist) + (susc-100);
         break;
         
      case APPLY_RAIR:
         resist = (ch->apply_res_air[1] == 0 ? 100 : ch->apply_res_air[1]);
         susc = (ch->apply_res_air[2] == 0 ? 100 : ch->apply_res_air[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_air[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_air[1])
                  ch->apply_res_air[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_air[2])
                  ch->apply_res_air[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_air[1] = 100;
               diff = base_resis_values(ch, 3)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_air[2]) //Susc
            {
               ch->apply_res_air[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_air[1]) //Resist
            {
               ch->apply_res_air[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_air[1] = -500;
         }
         resist = (ch->apply_res_air[1] == 0 ? 100 : ch->apply_res_air[1]);
         susc = (ch->apply_res_air[2] == 0 ? 100 : ch->apply_res_air[2]);
         if (ch->apply_res_air[1] == -500)
            ch->apply_res_air[0] = -500;
         else
            ch->apply_res_air[0] = 100 + diff - (100-resist) + (susc-100);
         break;
         
      case APPLY_RFIRE:
         resist = (ch->apply_res_fire[1] == 0 ? 100 : ch->apply_res_fire[1]);
         susc = (ch->apply_res_fire[2] == 0 ? 100 : ch->apply_res_fire[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_fire[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_fire[1])
                  ch->apply_res_fire[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_fire[2])
                  ch->apply_res_fire[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_fire[1] = 100;
               diff = base_resis_values(ch, 1)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_fire[2]) //Susc
            {
               ch->apply_res_fire[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_fire[1]) //Resist
            {
               ch->apply_res_fire[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_fire[1] = -500;
         }
         resist = (ch->apply_res_fire[1] == 0 ? 100 : ch->apply_res_fire[1]);
         susc = (ch->apply_res_fire[2] == 0 ? 100 : ch->apply_res_fire[2]);
         if (ch->apply_res_fire[1] == -500)
            ch->apply_res_fire[0] = -500;
         else
            ch->apply_res_fire[0] = 100 + diff - (100-resist) + (susc-100);
         break;
         
      case APPLY_RMAGIC:
         resist = (ch->apply_res_magic[1] == 0 ? 100 : ch->apply_res_magic[1]);
         susc = (ch->apply_res_magic[2] == 0 ? 100 : ch->apply_res_magic[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_magic[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_magic[1])
                  ch->apply_res_magic[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_magic[2])
                  ch->apply_res_magic[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_magic[1] = 100;
               diff = base_resis_values(ch, 9)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_magic[2]) //Susc
            {
               ch->apply_res_magic[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_magic[1]) //Resist
            {
               ch->apply_res_magic[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_magic[1] = -500;
         }
         resist = (ch->apply_res_magic[1] == 0 ? 100 : ch->apply_res_magic[1]);
         susc = (ch->apply_res_magic[2] == 0 ? 100 : ch->apply_res_magic[2]);
         if (ch->apply_res_magic[1] == -500)
            ch->apply_res_magic[0] = -500;
         else
            ch->apply_res_magic[0] = 100 + diff - (100-resist) + (susc-100);
         break;
         
      case APPLY_RNONMAGIC:
         resist = (ch->apply_res_nonmagic[1] == 0 ? 100 : ch->apply_res_nonmagic[1]);
         susc = (ch->apply_res_nonmagic[2] == 0 ? 100 : ch->apply_res_nonmagic[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_nonmagic[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_nonmagic[1])
                  ch->apply_res_nonmagic[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_nonmagic[2])
                  ch->apply_res_nonmagic[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_nonmagic[1] = 100;
               diff = base_resis_values(ch, 8)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_nonmagic[2]) //Susc
            {
               ch->apply_res_nonmagic[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_nonmagic[1]) //Resist
            {
               ch->apply_res_nonmagic[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_nonmagic[1] = -500;
         }
         resist = (ch->apply_res_nonmagic[1] == 0 ? 100 : ch->apply_res_nonmagic[1]);
         susc = (ch->apply_res_nonmagic[2] == 0 ? 100 : ch->apply_res_nonmagic[2]);
         if (ch->apply_res_nonmagic[1] == -500)
            ch->apply_res_nonmagic[0] = -500;
         else
            ch->apply_res_nonmagic[0] = 100 + diff - (100-resist) + (susc-100);
         break;
         
      case APPLY_RBLUNT:
         resist = (ch->apply_res_blunt[1] == 0 ? 100 : ch->apply_res_blunt[1]);
         susc = (ch->apply_res_blunt[2] == 0 ? 100 : ch->apply_res_blunt[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_blunt[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_blunt[1])
                  ch->apply_res_blunt[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_blunt[2])
                  ch->apply_res_blunt[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_blunt[1] = 100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_blunt[2]) //Susc
            {
               ch->apply_res_blunt[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_blunt[1]) //Resist
            {
               ch->apply_res_blunt[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_blunt[1] = -500;
         }
         resist = (ch->apply_res_blunt[1] == 0 ? 100 : ch->apply_res_blunt[1]);
         susc = (ch->apply_res_blunt[2] == 0 ? 100 : ch->apply_res_blunt[2]);
         if (ch->apply_res_blunt[1] == -500)
            ch->apply_res_blunt[0] = -500;
         else
            ch->apply_res_blunt[0] = 100 + diff - (100-resist) + (susc-100);
         break;  
         
      case APPLY_RPIERCE:
         resist = (ch->apply_res_pierce[1] == 0 ? 100 : ch->apply_res_pierce[1]);
         susc = (ch->apply_res_pierce[2] == 0 ? 100 : ch->apply_res_pierce[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_pierce[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_pierce[1])
                  ch->apply_res_pierce[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_pierce[2])
                  ch->apply_res_pierce[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_pierce[1] = 100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_pierce[2]) //Susc
            {
               ch->apply_res_pierce[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_pierce[1]) //Resist
            {
               ch->apply_res_pierce[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_pierce[1] = -500;
         }
         resist = (ch->apply_res_pierce[1] == 0 ? 100 : ch->apply_res_pierce[1]);
         susc = (ch->apply_res_pierce[2] == 0 ? 100 : ch->apply_res_pierce[2]);
         if (ch->apply_res_pierce[1] == -500)
            ch->apply_res_pierce[0] = -500;
         else
            ch->apply_res_pierce[0] = 100 + diff - (100-resist) + (susc-100);
         break;  
         
     case APPLY_RSLASH:
         resist = (ch->apply_res_slash[1] == 0 ? 100 : ch->apply_res_slash[1]);
         susc = (ch->apply_res_slash[2] == 0 ? 100 : ch->apply_res_slash[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_slash[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_slash[1])
                  ch->apply_res_slash[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_slash[2])
                  ch->apply_res_slash[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_slash[1] = 100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_slash[2]) //Susc
            {
               ch->apply_res_slash[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_slash[1]) //Resist
            {
               ch->apply_res_slash[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_slash[1] = -500;
         }
         resist = (ch->apply_res_slash[1] == 0 ? 100 : ch->apply_res_slash[1]);
         susc = (ch->apply_res_slash[2] == 0 ? 100 : ch->apply_res_slash[2]);
         if (ch->apply_res_slash[1] == -500)
            ch->apply_res_slash[0] = -500;
         else
            ch->apply_res_slash[0] = 100 + diff - (100-resist) + (susc-100);
         break;      
         
      case APPLY_RPOISON:
         resist = (ch->apply_res_poison[1] == 0 ? 100 : ch->apply_res_poison[1]);
         susc = (ch->apply_res_poison[2] == 0 ? 100 : ch->apply_res_poison[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_poison[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_poison[1])
                  ch->apply_res_poison[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_poison[2])
                  ch->apply_res_poison[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_poison[1] = 100;
               diff = base_resis_values(ch, 10)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_poison[2]) //Susc
            {
               ch->apply_res_poison[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_poison[1]) //Resist
            {
               ch->apply_res_poison[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_poison[1] = -500;
         }
         resist = (ch->apply_res_poison[1] == 0 ? 100 : ch->apply_res_poison[1]);
         susc = (ch->apply_res_poison[2] == 0 ? 100 : ch->apply_res_poison[2]);
         if (ch->apply_res_poison[1] == -500)
            ch->apply_res_poison[0] = -500;
         else
            ch->apply_res_poison[0] = 100 + diff - (100-resist) + (susc-100);
         break;      
         
      case APPLY_RPARALYSIS:
         resist = (ch->apply_res_paralysis[1] == 0 ? 100 : ch->apply_res_paralysis[1]);
         susc = (ch->apply_res_paralysis[2] == 0 ? 100 : ch->apply_res_paralysis[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_paralysis[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_paralysis[1])
                  ch->apply_res_paralysis[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_paralysis[2])
                  ch->apply_res_paralysis[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_paralysis[1] = 100;
               diff = base_resis_values(ch, 11)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_paralysis[2]) //Susc
            {
               ch->apply_res_poison[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_paralysis[1]) //Resist
            {
               ch->apply_res_paralysis[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_paralysis[1] = -500;
         }
         resist = (ch->apply_res_paralysis[1] == 0 ? 100 : ch->apply_res_paralysis[1]);
         susc = (ch->apply_res_paralysis[2] == 0 ? 100 : ch->apply_res_paralysis[2]);
         if (ch->apply_res_paralysis[1] == -500)
            ch->apply_res_paralysis[0] = -500;
         else
            ch->apply_res_paralysis[0] = 100 + diff - (100-resist) + (susc-100);
         break;      
         
      case APPLY_RHOLY:
         resist = (ch->apply_res_holy[1] == 0 ? 100 : ch->apply_res_holy[1]);
         susc = (ch->apply_res_holy[2] == 0 ? 100 : ch->apply_res_holy[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_holy[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_holy[1])
                  ch->apply_res_holy[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_holy[2])
                  ch->apply_res_holy[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_holy[1] = 100;
               diff = base_resis_values(ch, 6)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_holy[2]) //Susc
            {
               ch->apply_res_holy[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_holy[1]) //Resist
            {
               ch->apply_res_holy[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_holy[1] = -500;
         }
         resist = (ch->apply_res_holy[1] == 0 ? 100 : ch->apply_res_holy[1]);
         susc = (ch->apply_res_holy[2] == 0 ? 100 : ch->apply_res_holy[2]);
         if (ch->apply_res_holy[1] == -500)
            ch->apply_res_holy[0] = -500;
         else
            ch->apply_res_holy[0] = 100 + diff - (100-resist) + (susc-100);
         break; 
         
      case APPLY_RUNHOLY:
         resist = (ch->apply_res_unholy[1] == 0 ? 100 : ch->apply_res_unholy[1]);
         susc = (ch->apply_res_unholy[2] == 0 ? 100 : ch->apply_res_unholy[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_unholy[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_unholy[1])
                  ch->apply_res_unholy[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_unholy[2])
                  ch->apply_res_unholy[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_unholy[1] = 100;
               diff = base_resis_values(ch, 7)-100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_unholy[2]) //Susc
            {
               ch->apply_res_unholy[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_unholy[1]) //Resist
            {
               ch->apply_res_unholy[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_unholy[1] = -500;
         }
         resist = (ch->apply_res_unholy[1] == 0 ? 100 : ch->apply_res_unholy[1]);
         susc = (ch->apply_res_unholy[2] == 0 ? 100 : ch->apply_res_unholy[2]);
         if (ch->apply_res_unholy[1] == -500)
            ch->apply_res_unholy[0] = -500;
         else
            ch->apply_res_unholy[0] = 100 + diff - (100-resist) + (susc-100);
         break;  
         
      case APPLY_RUNDEAD:
         resist = (ch->apply_res_undead[1] == 0 ? 100 : ch->apply_res_undead[1]);
         susc = (ch->apply_res_undead[2] == 0 ? 100 : ch->apply_res_undead[2]);
         //Getting the natural "susc/resist" to add back in later...
         diff = (ch->apply_res_undead[0] + (100-resist) - (susc-100)) - 100;
         if (!fAdd)
         {
            if (mod == 0 || mod == -1) //0 and 1 will not work
               ;
            if (mod > -100) //Resist
            {
               if (mod == -ch->apply_res_undead[1])
                  ch->apply_res_undead[1] = 100;
            }
            if (mod < -100) //Susc
            {
               if (mod == -ch->apply_res_undead[2])
                  ch->apply_res_undead[2] = 100;   
            }
            if (mod == 1) //Immune
            {
               ch->apply_res_undead[1] = 100;
            }
         }
         else
         {
            if (mod > 100 && mod > ch->apply_res_undead[2]) //Susc
            {
               ch->apply_res_undead[2] = mod;
            }
            if (mod < 100 && mod > 1 && mod < ch->apply_res_undead[1]) //Resist
            {
               ch->apply_res_undead[1] = mod;
            }
            if (mod == -1) //Immune
               ch->apply_res_undead[1] = -500;
         }
         resist = (ch->apply_res_undead[1] == 0 ? 100 : ch->apply_res_undead[1]);
         susc = (ch->apply_res_undead[2] == 0 ? 100 : ch->apply_res_undead[2]);
         if (ch->apply_res_undead[1] == -500)
            ch->apply_res_undead[0] = -500;
         else
            ch->apply_res_undead[0] = 100 + diff - (100-resist) + (susc-100);
         break;        
         
      case APPLY_MANAFUSE:
         if (mod > ch->apply_manafuse)
         {
            ch->apply_manafuse = mod;
         }
         if (mod < 0 && mod + ch->apply_manafuse == 0)
            ch->apply_manafuse = 0;
         break;
      case APPLY_FASTING:
         if (mod > ch->apply_fasting)
         {
            ch->apply_fasting = mod;
         }
         if (mod < 0 && mod + ch->apply_fasting == 0)
            ch->apply_fasting = 0;
         break;
      case APPLY_MANASHELL:
         if (mod > ch->apply_manashell)
         {
            ch->apply_manashell = mod;
         }
         if (mod < 0 && mod + ch->apply_manashell == 0)
            ch->apply_manashell = 0;
         break;
      case APPLY_MANASHIELD:
         if (mod > ch->apply_manashield)
         {
            ch->apply_manashield = mod;
         }
         if (mod < 0 && mod + ch->apply_manashield == 0)
            ch->apply_manashield = 0;
         break;
      case APPLY_MANAGUARD:
         if (mod > ch->apply_managuard)
         {
            ch->apply_managuard = mod;
         }
         if (mod < 0 && mod + ch->apply_managuard == 0)
            ch->apply_managuard = 0;
         break;
         
      case APPLY_MANABURN:
         if (mod > ch->apply_manaburn)
         {
            ch->apply_manaburn = mod;
         }
         if (mod < 0 && mod + ch->apply_manaburn == 0)
            ch->apply_manaburn = 0;
         break;
      case APPLY_WEAPONCLAMP:
         if (mod > ch->apply_weaponclamp)
         {
            ch->apply_weaponclamp = mod;
         }
         if (mod < 0 && mod + ch->apply_weaponclamp == 0)
            ch->apply_weaponclamp = 0;
         break;  
      case APPLY_ARROWCATCH:
         if (mod > ch->apply_arrowcatch)
         {
            ch->apply_arrowcatch = mod;
         }
         if (mod < 0 && mod + ch->apply_arrowcatch == 0)
            ch->apply_arrowcatch = 0;
         break;  
      case APPLY_BRACING:
         if (mod > ch->apply_bracing)
         {
            ch->apply_bracing = mod;
         }
         if (mod < 0 && mod + ch->apply_bracing == 0)
            ch->apply_bracing = 0;
         break;    
      case APPLY_HARDENING:
         if (mod > ch->apply_hardening)
         {
            ch->apply_hardening = mod;
         }
         if (mod < 0 && mod + ch->apply_hardening == 0)
            ch->apply_hardening = 0;
         break;   
      case APPLY_TOHIT:
         ch->apply_tohit += mod;
         break;
      case APPLY_MANATICK:
         if (mod > ch->managen)
         {
            ch->managen = mod;
         }
         if (mod < 0 && mod + ch->managen == 0)
            ch->managen = 0;
         break;
      case APPLY_HPTICK:
         if (mod > ch->hpgen)
         {
            ch->hpgen = mod;
         }
         if (mod < 0 && mod + ch->hpgen == 0)
            ch->hpgen = 0;
         break;
      case APPLY_STR:
         ch->mod_str += mod;
         break;
      case APPLY_DEX:
         ch->mod_dex += mod;
         break;
      case APPLY_INT:
         ch->mod_int += mod;
         break;
      case APPLY_WIS:
         ch->mod_wis += mod;
         break;
      case APPLY_CON:
         ch->mod_con += mod;
         break;
      case APPLY_CHA:
         ch->mod_cha += mod;
         break;
      case APPLY_LCK:
         ch->mod_lck += mod;
         break;
      case APPLY_AGI:
         ch->mod_agi += mod;
         break;
      case APPLY_SEX:
         ch->sex = (ch->sex + mod) % 3;
         if (ch->sex < 0)
            ch->sex += 2;
         ch->sex = URANGE(0, ch->sex, 2);
         break;

         /*
          * These are unused due to possible problems.  Enable at your own risk.
          */
      case APPLY_CLASS:
         break;
      case APPLY_LEVEL:
         break;
      case APPLY_AGE:
         break;
      case APPLY_GOLD:
         break;
      case APPLY_EXP:
         break;

         /*
          * Regular apply types
          */
      case APPLY_HEIGHT:
         ch->height += mod;
         break;
      case APPLY_WEIGHT:
         ch->weight += mod;
         break;
      case APPLY_MANA:
         /*if (mod > ch->max_mana)
            mod = paf->modifier = ch->max_mana;
         for (saf = ch->first_affect; saf; saf = saf->next)
         {
            if ((saf->location % REVERSE_APPLY) == APPLY_MANA && saf != paf && fAdd)
               paf->modifier = mod = 0;
         }*/
         ch->max_mana += mod;
         break;
      case APPLY_HIT:
         /*if (mod > ch->max_hit)
            mod = paf->modifier = ch->max_hit; 
         for (saf = ch->first_affect; saf; saf = saf->next)
         {
            if ((saf->location % REVERSE_APPLY) == APPLY_HIT && saf != paf && fAdd)
               paf->modifier = mod = 0;
         } */
         ch->max_hit += mod;
         break;
      case APPLY_MOVE:
         ch->max_move += mod;
         break;
      case APPLY_HITROLL:
         ch->hitroll += mod;
         break;
      case APPLY_DAMROLL:
         ch->damroll += mod;
         break;
      case APPLY_SAVING_POISON:
         ch->saving_poison_death += mod;
         break;
      case APPLY_SAVING_ROD:
         ch->saving_wand += mod;
         break;
      case APPLY_SAVING_PARA:
         ch->saving_para_petri += mod;
         break;
      case APPLY_SAVING_BREATH:
         ch->saving_breath += mod;
         break;
      case APPLY_SAVING_SPELL:
         ch->saving_spell_staff += mod;
         break;

         /*
          * Bitvector modifying apply types
          */
      case APPLY_AFFECT:
         SET_BIT(ch->affected_by.bits[0], mod);
         break;
      case APPLY_EXT_AFFECT:
         xSET_BIT(ch->affected_by, mod);
         break;
      case APPLY_RESISTANT:
         SET_BIT(ch->resistant, mod);
         break;
      case APPLY_IMMUNE:
         SET_BIT(ch->immune, mod);
         break;
      case APPLY_SUSCEPTIBLE:
         SET_BIT(ch->susceptible, mod);
         break;
      case APPLY_WEAPONSPELL: /* see fight.c */
         break;
      case APPLY_REMOVE:
         REMOVE_BIT(ch->affected_by.bits[0], mod);
         break;

         /*
          * Player condition modifiers
          */
      case APPLY_FULL:
         if (!IS_NPC(ch))
            ch->pcdata->condition[COND_FULL] = URANGE(0, ch->pcdata->condition[COND_FULL] + mod, 48);
         break;

      case APPLY_THIRST:
         if (!IS_NPC(ch))
            ch->pcdata->condition[COND_THIRST] = URANGE(0, ch->pcdata->condition[COND_THIRST] + mod, 48);
         break;

      case APPLY_DRUNK:
         if (!IS_NPC(ch))
            ch->pcdata->condition[COND_DRUNK] = URANGE(0, ch->pcdata->condition[COND_DRUNK] + mod, 48);
         break;

      case APPLY_BLOOD:
         if (!IS_NPC(ch))
            ch->pcdata->condition[COND_BLOODTHIRST] = URANGE(0, ch->pcdata->condition[COND_BLOODTHIRST] + mod, ch->level + 10);
         break;

      case APPLY_MENTALSTATE:
         ch->mental_state = URANGE(-100, ch->mental_state + mod, 100);
         break;
      case APPLY_EMOTION:
         ch->emotional_state = URANGE(-100, ch->emotional_state + mod, 100);
         break;


         /*
          * Specialty modfiers
          */
      case APPLY_CONTAGIOUS:
         break;
      case APPLY_ODOR:
         break;
      case APPLY_STRIPSN:
         if (IS_VALID_SN(mod))
            affect_strip(ch, mod);
         else
            bug("affect_modify: APPLY_STRIPSN invalid sn %d", mod);
         break;

/* spell cast upon wear/removal of an object	-Thoric */
      case APPLY_WEARSPELL:
      case APPLY_REMOVESPELL:
         if (xIS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) || wIS_SET(ch, ROOM_NO_MAGIC) || is_immune(ch, -1, RIS_MAGIC) 
         ||  saving_char == ch /* so save/quit doesn't trigger */  || loading_char == ch) /* so loading doesn't trigger */
            return;

         mod = abs(mod);
         if (IS_VALID_SN(mod) && (skill = skill_table[mod]) != NULL && skill->type == SKILL_SPELL)
         {
            if (skill->target == TAR_IGNORE || skill->target == TAR_OBJ_INV || skill->target == TAR_OBJ_ROOM)
            {
               bug("APPLY_WEARSPELL trying to apply bad target spell.  SN is %d.", mod);
               return;
            }
            if ((retcode = (*skill->spell_fun) (mod, ch->level, ch, ch)) == rCHAR_DIED || char_died(ch))
               return;
         }
         break;


         /*
          * Skill apply types
          */
      case APPLY_PALM: /* not implemented yet */
         break;
      case APPLY_TRACK:
         modify_skill(ch, gsn_track, mod, fAdd);
         break;
      case APPLY_HIDE:
         modify_skill(ch, gsn_hide, mod, fAdd);
         break;
      case APPLY_STEAL:
         modify_skill(ch, gsn_steal, mod, fAdd);
         break;
      case APPLY_SNEAK:
         modify_skill(ch, gsn_sneak, mod, fAdd);
         break;
      case APPLY_PICK:
         modify_skill(ch, gsn_pick_lock, mod, fAdd);
         break;
      case APPLY_BACKSTAB:
         modify_skill(ch, gsn_backstab, mod, fAdd);
         break;
      case APPLY_DETRAP:
         modify_skill(ch, gsn_detrap, mod, fAdd);
         break;
      case APPLY_DODGE:
         modify_skill(ch, gsn_dodge, mod, fAdd);
         break;
      case APPLY_PEEK:
         modify_skill(ch, gsn_peek, mod, fAdd);
         break;
      case APPLY_SCAN:
         modify_skill(ch, gsn_scan, mod, fAdd);
         break;
      case APPLY_GOUGE:
         modify_skill(ch, gsn_gouge, mod, fAdd);
         break;
      case APPLY_SEARCH:
         modify_skill(ch, gsn_search, mod, fAdd);
         break;
      case APPLY_DIG:
         modify_skill(ch, gsn_dig, mod, fAdd);
         break;
      case APPLY_MOUNT:
         break;
      case APPLY_DISARM:
         modify_skill(ch, gsn_disarm, mod, fAdd);
         break;
      case APPLY_KICK:
         break;
      case APPLY_PARRY:
         modify_skill(ch, gsn_parry, mod, fAdd);
         break;
      case APPLY_BASH:
         break;
      case APPLY_STUN:
         modify_skill(ch, gsn_stun, mod, fAdd);
         break;
      case APPLY_PUNCH:
         break;
      case APPLY_CLIMB:
         modify_skill(ch, gsn_climb, mod, fAdd);
         break;
      case APPLY_GRIP:
         modify_skill(ch, gsn_grip, mod, fAdd);
         break;
      case APPLY_SCRIBE:
         modify_skill(ch, gsn_scribe, mod, fAdd);
         break;
      case APPLY_BREW:
         modify_skill(ch, gsn_brew, mod, fAdd);
         break;
      case APPLY_COOK:
         modify_skill(ch, gsn_cook, mod, fAdd);
         break;

         /*
          * Room apply types
          */
      case APPLY_ROOMFLAG:
      case APPLY_SECTORTYPE:
      case APPLY_ROOMLIGHT:
      case APPLY_TELEVNUM:
         break;

         /*
          * Object apply types
          */
   }

   /*
    * Check for weapon wielding.
    * Guard against recursion (for weapons with affects).
    */
   if (!IS_NPC(ch) && saving_char != ch && (wield = get_eq_char(ch, WEAR_WIELD)) != NULL && get_obj_weight(wield) > str_app[get_curr_str(ch)].wield)
   {
      static int depth;

      if (depth == 0)
      {
         depth++;
         act(AT_ACTION, "You notice your strength decrease thus taking more time to swing your weapon!", ch, wield, NULL, TO_CHAR);
         depth--;
      }
   }

   return;
}



/*
 * Give an affect to a char.
 */
void affect_to_char(CHAR_DATA * ch, AFFECT_DATA * paf)
{
   AFFECT_DATA *paf_new;

   if (!ch)
   {
      bug("Affect_to_char(NULL, %d)", paf ? paf->type : 0);
      return;
   }

   if (!paf)
   {
      bug("Affect_to_char(%s, NULL)", ch->name);
      return;
   }

   CREATE(paf_new, AFFECT_DATA, 1);
   LINK(paf_new, ch->first_affect, ch->last_affect, next, prev);
   paf_new->type = paf->type;
   paf_new->duration = paf->duration;
   paf_new->location = paf->location;
   paf_new->modifier = paf->modifier;
   paf_new->bitvector = paf->bitvector;

   affect_modify(ch, paf_new, TRUE);
   return;
}


/*
 * Remove an affect from a char.
 */
void affect_remove(CHAR_DATA * ch, AFFECT_DATA * paf)
{
   if (!ch->first_affect)
   {
      bug("Affect_remove(%s, %d): no affect.", ch->name, paf ? paf->type : 0);
      return;
   }

   affect_modify(ch, paf, FALSE);

   UNLINK(paf, ch->first_affect, ch->last_affect, next, prev);
   DISPOSE(paf);
   return;
}

/*
 * Strip all affects of a given sn.
 */
void affect_strip(CHAR_DATA * ch, int sn)
{
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;

   for (paf = ch->first_affect; paf; paf = paf_next)
   {
      paf_next = paf->next;
      if (paf->type == sn)
         affect_remove(ch, paf);
   }

   return;
}



/*
 * Return true if a char is affected by a spell.
 */
bool is_affected(CHAR_DATA * ch, int sn)
{
   AFFECT_DATA *paf;

   for (paf = ch->first_affect; paf; paf = paf->next)
      if (paf->type == sn)
         return TRUE;

   return FALSE;
}


/*
 * Add or enhance an affect.
 * Limitations put in place by Thoric, they may be high... but at least
 * they're there :)
 */
void affect_join(CHAR_DATA * ch, AFFECT_DATA * paf)
{
   AFFECT_DATA *paf_old;

   for (paf_old = ch->first_affect; paf_old; paf_old = paf_old->next)
      if (paf_old->type == paf->type)
      {
         paf->duration = UMIN(1000000, paf->duration + paf_old->duration);
         if (paf->modifier < 0)
         {
            if (paf_old->modifier < paf->modifier)
               paf->modifier = paf_old->modifier;
         }
         else if (paf->modifier > 0)
         {
            if (paf_old->modifier > paf->modifier)
               paf->modifier = paf_old->modifier;
         }
         affect_remove(ch, paf_old);
         break;
      }

   affect_to_char(ch, paf);
   return;
}


/*
 * Apply only affected and RIS on a char
 */
void aris_affect(CHAR_DATA * ch, AFFECT_DATA * paf)
{
   xSET_BITS(ch->affected_by, paf->bitvector);
   switch (paf->location % REVERSE_APPLY)
   {
      case APPLY_AFFECT:
         SET_BIT(ch->affected_by.bits[0], paf->modifier);
         break;
      case APPLY_EXT_AFFECT:
         xSET_BIT(ch->affected_by, paf->modifier);
         
      case APPLY_RESISTANT:
         SET_BIT(ch->resistant, paf->modifier);
         break;
      case APPLY_IMMUNE:
         SET_BIT(ch->immune, paf->modifier);
         break;
      case APPLY_SUSCEPTIBLE:
         SET_BIT(ch->susceptible, paf->modifier);
         break;
   }
}

/*
 * Update affecteds and RIS for a character in case things get messed.
 * This should only really be used as a quick fix until the cause
 * of the problem can be hunted down. - FB
 * Last modified: June 30, 1997
 *
 * Quick fix?  Looks like a good solution for a lot of problems.
 */

/* Temp mod to bypass immortals so they can keep their mset affects,
 * just a band-aid until we get more time to look at it -- Blodkai */
void update_aris(CHAR_DATA * ch)
{
   AFFECT_DATA *paf;
   OBJ_DATA *obj;
   int hiding;

   if (IS_NPC(ch) || IS_IMMORTAL(ch))
      return;

   /* So chars using hide skill will continue to hide */
   hiding = IS_AFFECTED(ch, AFF_HIDE);

   xCLEAR_BITS(ch->affected_by);
   ch->resistant = 0;
   ch->immune = 0;
   ch->susceptible = 0;
   xCLEAR_BITS(ch->no_affected_by);
   ch->no_resistant = 0;
   ch->no_immune = 0;
   ch->no_susceptible = 0;

   /* Add in effects from race */
   xSET_BITS(ch->affected_by, race_table[ch->race]->affected);
   SET_BIT(ch->resistant, race_table[ch->race]->resist);
   SET_BIT(ch->susceptible, race_table[ch->race]->suscept);

   /* Add in effects from deities */
   if (ch->pcdata->deity)
   {
      if (ch->pcdata->favor > ch->pcdata->deity->affectednum)
         xSET_BITS(ch->affected_by, ch->pcdata->deity->affected);
      if (ch->pcdata->favor > ch->pcdata->deity->elementnum)
         SET_BIT(ch->resistant, ch->pcdata->deity->element);
      if (ch->pcdata->favor < ch->pcdata->deity->susceptnum)
         SET_BIT(ch->susceptible, ch->pcdata->deity->suscept);
   }

   /* Add in effect from spells */
   for (paf = ch->first_affect; paf; paf = paf->next)
      aris_affect(ch, paf);

   /* Add in effects from equipment */
   for (obj = ch->first_carrying; obj; obj = obj->next_content)
   {
      if (obj->wear_loc != WEAR_NONE)
      {
         for (paf = obj->first_affect; paf; paf = paf->next)
            aris_affect(ch, paf);

         for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
            aris_affect(ch, paf);
      }
   }

   /* Add in effects from the room */
   if (ch->in_room) /* non-existant char booboo-fix --TRI */
      for (paf = ch->in_room->first_affect; paf; paf = paf->next)
         aris_affect(ch, paf);

   /* Add in effects for polymorph */
   if (ch->morph)
   {
      xSET_BITS(ch->affected_by, ch->morph->affected_by);
      SET_BIT(ch->immune, ch->morph->immune);
      SET_BIT(ch->resistant, ch->morph->resistant);
      SET_BIT(ch->susceptible, ch->morph->suscept);
      /* Right now only morphs have no_ things --Shaddai */
      xSET_BITS(ch->no_affected_by, ch->morph->no_affected_by);
      SET_BIT(ch->no_immune, ch->morph->no_immune);
      SET_BIT(ch->no_resistant, ch->morph->no_resistant);
      SET_BIT(ch->no_susceptible, ch->morph->no_suscept);
   }

   /* If they were hiding before, make them hiding again */
   if (hiding)
      xSET_BIT(ch->affected_by, AFF_HIDE);

   return;
}


/*
 * Move a char out of a room.
 */
void char_from_room(CHAR_DATA * ch)
{
   OBJ_DATA *obj;
   AFFECT_DATA *paf;
   CMAP_DATA *mch;

   if (!ch->in_room)
   {
      bug("Char_from_room: NULL.", 0);
      return;
   }

   if (!IS_NPC(ch))
      --ch->in_room->area->nplayer;

   if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room->light > 0)
      --ch->in_room->light;

   /*
    * Character's affect on the room
    */
   for (paf = ch->first_affect; paf; paf = paf->next)
      room_affect(ch->in_room, paf, FALSE);

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_MAPWINDOW) && ch->in_room->vnum == OVERLAND_SOLAN)
   {
      ch_printf(ch, "%s", MXPTAG("FRAME Map CLOSE"));
      ch->pcdata->xsize = 0;
      ch->pcdata->ysize = 0;
   }

   /*
    * Room's affect on the character
    */
   if (!char_died(ch))
   {
      for (paf = ch->in_room->first_affect; paf; paf = paf->next)
         affect_modify(ch, paf, FALSE);

      if (char_died(ch)) /* could die from removespell, etc */
         return;
   }
   if (IS_NPC(ch))
   {
      for (mch = first_wilderchar; mch; mch = mch->next)
      {
         if (mch->mapch == ch && (mch->mapch->coord->x == ch->coord->x && mch->mapch->coord->y == ch->coord->y && mch->mapch->map == ch->map))
         {
            UNLINK(mch, first_wilderchar, last_wilderchar, next, prev);
            top_map_mob--;
         }
      }
   }

   UNLINK(ch, ch->in_room->first_person, ch->in_room->last_person, next_in_room, prev_in_room);
   ch->was_in_room = ch->in_room;
   ch->in_room = NULL;
   ch->next_in_room = NULL;
   ch->prev_in_room = NULL;

   if (!IS_NPC(ch) && get_timer(ch, TIMER_SHOVEDRAG) > 0)
      remove_timer(ch, TIMER_SHOVEDRAG);

   return;
}

/*
 * Enough resources to build? Docks if needed -- Xerves 12/99
 */
bool enough_resources(CHAR_DATA * ch, int hometown, int needed, int ctype)
{
   if (needed > ch->pcdata->town->lumber)
   {
      ch_printf(ch, "You need %d units, your hometown cannot afford that.\n\r", needed);
      return FALSE;
   }
   if (ctype == 0)
      ch->pcdata->town->lumber -= needed;
   return TRUE;
}

/*
 * Move a char into a room.
 */
void char_to_room(CHAR_DATA * ch, ROOM_INDEX_DATA * pRoomIndex)
{
   OBJ_DATA *obj;
   AFFECT_DATA *paf;
   CMAP_DATA *mch;

   if (!ch)
   {
      bug("Char_to_room: NULL ch!", 0);
      return;
   }
   if (!pRoomIndex)
   {
      bug("Char_to_room: %s -> NULL room!  Putting char in limbo (%d)", ch->name, ROOM_VNUM_LIMBO);
      /*
       * This used to just return, but there was a problem with crashing
       * and I saw no reason not to just put the char in limbo.  -Narn
       */
      pRoomIndex = get_room_index(ROOM_VNUM_LIMBO);
   }

   ch->in_room = pRoomIndex;
   LINK(ch, pRoomIndex->first_person, pRoomIndex->last_person, next_in_room, prev_in_room);

   if (IS_NPC(ch) && ch->in_room->vnum == OVERLAND_SOLAN)
   {
      CREATE(mch, CMAP_DATA, 1);
      mch->mapch = ch;
      LINK(mch, first_wilderchar, last_wilderchar, next, prev);
      top_map_mob++;
   }

   if (!IS_NPC(ch))
      if (++pRoomIndex->area->nplayer > pRoomIndex->area->max_players)
         pRoomIndex->area->max_players = pRoomIndex->area->nplayer;

   if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
      ++pRoomIndex->light;
      
   if (ch->pcdata && ch->pcdata->quest && ch->pcdata->quest->questarea == ch->in_room->area 
   && ch->pcdata->quest->timeleft > 0 && ch->pcdata->quest->mission == 2)
   {
      int x;
      CHAR_DATA *gch;
      
      ch->pcdata->quest->killed++;
      if (ch->pcdata->quest->traveltime > -1)
         ch->pcdata->quest->traveltime = 0;
      if (pRoomIndex->vnum == pRoomIndex->area->hi_r_vnum) //Win
      {
         for (x = 0; x <= 5; x++)
         {
            if (ch->pcdata->quest->player[x] == ch->pcdata->pid)
            {
               ch_printf(ch, "\n\r^r&CYou have completed your quest, your reward is %d QP^x\n\r\n\r", ch->pcdata->quest->qp[x]);
               ch->pcdata->quest_curr += ch->pcdata->quest->qp[x];
               ch->pcdata->quest_accum += ch->pcdata->quest->qp[x];
               ch->pcdata->quest_wins++;
            }
         }
         for (gch = first_char; gch; gch = gch->next)
         {
            if (IS_NPC(gch))
               continue;
            if (gch == ch)
               continue;
            if (!gch->pcdata->quest)
               continue;
            if (gch->pcdata->quest->questarea != ch->in_room->area)
               continue;
            if (gch->pcdata->quest != ch->pcdata->quest)
               continue;
            for (x = 0; x <= 5; x++)
            {
               if (gch->pcdata->quest->player[x] == gch->pcdata->pid)
               {
                  ch_printf(gch, "\n\r^r&CYou have completed your quest, your reward is %d QP^x\n\r\n\r", gch->pcdata->quest->qp[x]);
                  gch->pcdata->quest_curr += gch->pcdata->quest->qp[x];
                  gch->pcdata->quest_accum += gch->pcdata->quest->qp[x];
                  gch->pcdata->quest_wins++;
                  gch->pcdata->quest->timeleft = -1;
                  gch->pcdata->quest->tillnew = 25;
               }
            }
         }
         ch->pcdata->quest->timeleft = -1;
         ch->pcdata->quest->tillnew = 25;
      }
   }      

   /*
    * Room's effect on the character
    */
   if (!char_died(ch))
   {
      for (paf = pRoomIndex->first_affect; paf; paf = paf->next)
         affect_modify(ch, paf, TRUE);

      if (char_died(ch)) /* could die from a wearspell, etc */
         return;
   }

   /*
    * Character's effect on the room
    */
   for (paf = ch->first_affect; paf; paf = paf->next)
      room_affect(pRoomIndex, paf, TRUE);


   if (!IS_NPC(ch) && (check_room_pk(ch) == 1))
   {
      if (get_timer(ch, TIMER_SHOVEDRAG) > 0)
         remove_timer(ch, TIMER_SHOVEDRAG);
      add_timer(ch, TIMER_SHOVEDRAG, 4, NULL, 0);/*-12 Seconds-*/
   }

   /*
    * Delayed Teleport rooms     -Thoric
    * Should be the last thing checked in this function
    */
   if (xIS_SET(pRoomIndex->room_flags, ROOM_TELEPORT) && pRoomIndex->tele_delay > 0)
   {
      TELEPORT_DATA *tele;

      for (tele = first_teleport; tele; tele = tele->next)
         if (tele->room == pRoomIndex)
            return;

      CREATE(tele, TELEPORT_DATA, 1);
      LINK(tele, first_teleport, last_teleport, next, prev);
      tele->room = pRoomIndex;
      tele->timer = pRoomIndex->tele_delay;
   }
   if (ch->on)
   {
      ch->on = NULL;
      ch->position = POS_STANDING;
   }
   if (ch->mount)
   {
      ch->mount->on = NULL;
      ch->mount->position = POS_STANDING;
   }
   if (!ch->was_in_room)
      ch->was_in_room = ch->in_room;
   return;
}

//Checks carrying weight/number, replaced the integer due to it being inaccurate at times
float get_ch_carry_weight(CHAR_DATA *ch)
{
   OBJ_DATA *obj;
   float weight = 0;
   
   for (obj = ch->first_carrying; obj; obj = obj->next_content) 
   {
      weight += get_obj_weight(obj);
   }
   if (ch->rider)
   {
      weight += (float)ch->rider->weight;
      weight += get_ch_carry_weight(ch->rider);
   }
   return weight;
}
int get_ch_carry_number(CHAR_DATA *ch)
{
   OBJ_DATA *obj;
   int weight = 0;
   
   for (obj = ch->first_carrying; obj; obj = obj->next_content) 
   {
      if (obj->wear_loc == WEAR_NONE)
         weight += get_obj_number(obj);
   }
   return weight;
}

OBJ_DATA *bank_to_char(OBJ_DATA * obj, CHAR_DATA *ch)
{
   if (ch != NULL)
   {
      if (IN_WILDERNESS(ch))
      {
         SET_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = ch->map;
         obj->coord->x = ch->coord->x;
         obj->coord->y = ch->coord->y;
      }
      else
      {
         REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = -1;
         obj->coord->x = -1;
         obj->coord->y = -1;
      }
   }
   else
   {
      REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
      obj->map = -1;
      obj->coord->x = -1;
      obj->coord->y = -1;
   }   
   UNLINK(obj, ch->pcdata->first_bankobj, ch->pcdata->last_bankobj, next_content, prev_content);
   obj->carried_by = NULL;
   obj->in_room = NULL;
   obj->in_obj = NULL;
   return obj;
}

OBJ_DATA *townbank_to_char(OBJ_DATA * obj, TOWN_DATA *town, CHAR_DATA *ch)
{
   if (ch != NULL)
   {
      if (IN_WILDERNESS(ch))
      {
         SET_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = ch->map;
         obj->coord->x = ch->coord->x;
         obj->coord->y = ch->coord->y;
      }
      else
      {
         REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = -1;
         obj->coord->x = -1;
         obj->coord->y = -1;
      }
   }
   else
   {
      REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
      obj->map = -1;
      obj->coord->x = -1;
      obj->coord->y = -1;
   }   
   UNLINK(obj, town->first_bankobj, town->last_bankobj, next_content, prev_content);
   obj->carried_by = NULL;
   obj->in_room = NULL;
   obj->in_obj = NULL;
   return obj;
}
   

OBJ_DATA *obj_to_bank(OBJ_DATA * obj, CHAR_DATA *ch)
{
   OBJ_DATA *otmp;
   OBJ_DATA *oret = NULL;
   int grouped = FALSE;
   
   REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
   obj->map = -1;
   obj->coord->x = -1;
   obj->coord->y = -1;
   
   for (otmp = ch->pcdata->first_bankobj; otmp; otmp = otmp->next_content)
   {
      if ((oret = group_object(otmp, obj)) == otmp)  
      {
         grouped = TRUE;
         break;
      }
   }
   if (!grouped)      
   {
      LINK(obj, ch->pcdata->first_bankobj, ch->pcdata->last_bankobj, next_content, prev_content);
      obj->carried_by = NULL;
      obj->in_room = NULL;
      obj->in_obj = NULL;
      return obj;
   }
   else
      return oret;
}

OBJ_DATA *obj_to_townbank(OBJ_DATA * obj, TOWN_DATA *town)
{
   OBJ_DATA *otmp;
   OBJ_DATA *oret = NULL;
   int grouped = FALSE;
   
   REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
   obj->map = -1;
   obj->coord->x = -1;
   obj->coord->y = -1;
   
   for (otmp = town->first_bankobj; otmp; otmp = otmp->next_content)
   {
      if ((oret = group_object(otmp, obj)) == otmp)  
      {
         grouped = TRUE;
         break;
      }
   }
   if (!grouped)      
   {
      LINK(obj, town->first_bankobj, town->last_bankobj, next_content, prev_content);
      obj->carried_by = NULL;
      obj->in_room = NULL;
      obj->in_obj = NULL;
      return obj;
   }
   else
      return oret;
}      
/*
 * Give an obj to a char.
 */
OBJ_DATA *obj_to_char(OBJ_DATA * obj, CHAR_DATA * ch)
{
   OBJ_DATA *otmp;
   OBJ_DATA *oret = obj;
   bool skipgroup, grouped;
   int x;
   CHAR_DATA *gch;

   skipgroup = FALSE;
   grouped = FALSE;
   
   if (obj->trap && !IS_NPC(ch))
   {
      if (obj->trap->uid < START_INV_TRAP)
         copy_trap(obj->trap, obj);
   }

   if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
   {
      if (!IS_IMMORTAL(ch) && (IS_NPC(ch) && !xIS_SET(ch->act, ACT_PROTOTYPE)))
         return obj_to_room(obj, ch->in_room, ch);
   }

   if (ch != NULL)
   {
      if (IN_WILDERNESS(ch))
      {
         SET_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = ch->map;
         obj->coord->x = ch->coord->x;
         obj->coord->y = ch->coord->y;
      }
      else
      {
         REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = -1;
         obj->coord->x = -1;
         obj->coord->y = -1;
      }
   }
   else
   {
      REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
      obj->map = -1;
      obj->coord->x = -1;
      obj->coord->y = -1;
   }

   if (loading_char == ch)
   {
      int x, y;

      for (x = 0; x < MAX_WEAR; x++)
         for (y = 0; y < MAX_LAYERS; y++)
            if (save_equipment[x][y] == obj)
            {
               skipgroup = TRUE;
               break;
            }
   }
   if (!skipgroup)
      for (otmp = ch->first_carrying; otmp; otmp = otmp->next_content)
         if ((oret = group_object(otmp, obj)) == otmp)
         {
            grouped = TRUE;
            break;
         }
   if (!grouped)
   {
      if (!IS_NPC(ch) || !ch->pIndexData->pShop)
      {
         LINK(obj, ch->first_carrying, ch->last_carrying, next_content, prev_content);
         obj->carried_by = ch;
         obj->in_room = NULL;
         obj->in_obj = NULL;
      }
      else
      {
         /* If ch is a shopkeeper, add the obj using an insert sort */
         for (otmp = ch->first_carrying; otmp; otmp = otmp->next_content)
         {
            if (obj->level > otmp->level)
            {
               INSERT(obj, otmp, ch->first_carrying, next_content, prev_content);
               break;
            }
            else if (obj->level == otmp->level && strcmp(obj->short_descr, otmp->short_descr) < 0)
            {
               INSERT(obj, otmp, ch->first_carrying, next_content, prev_content);
               break;
            }
         }

         if (!otmp)
         {
            LINK(obj, ch->first_carrying, ch->last_carrying, next_content, prev_content);
         }

         obj->carried_by = ch;
         obj->in_room = NULL;
         obj->in_obj = NULL;
      }
   }
   if (obj->item_type == ITEM_QTOKEN)
   {
      if (!xIS_SET(obj->extra_flags, ITEM_QTOKEN_LOOTED))
      {
         if (!ch->in_room || (IS_NPC(ch) && ch->pIndexData->vnum >= ch->in_room->area->low_r_vnum && ch->pIndexData->vnum <= ch->in_room->area->hi_r_vnum))
            ;
         else
            xSET_BIT(obj->extra_flags, ITEM_QTOKEN_LOOTED);
         if (!IS_NPC(ch) && ch->pcdata->quest && ch->pcdata->quest->questarea == ch->in_room->area 
         && ch->pcdata->quest->timeleft > 0 && ch->pcdata->quest->mission == 4)
         {
            ch->pcdata->quest->killed += obj->count;
            if (ch->pcdata->quest->traveltime > -1)
               ch->pcdata->quest->traveltime = -1;
            if (ch->pcdata->quest->killed >= ch->pcdata->quest->tokill) //Win
            {
               for (x = 0; x <= 5; x++)
               {
                  if (ch->pcdata->quest->player[x] == ch->pcdata->pid)
                  {
                     ch_printf(ch, "\n\r^r&CYou have completed your quest, your reward is %d QP^x\n\r\n\r", ch->pcdata->quest->qp[x]);
                     ch->pcdata->quest_curr += ch->pcdata->quest->qp[x];
                     ch->pcdata->quest_accum += ch->pcdata->quest->qp[x];
                     ch->pcdata->quest_wins++;
                  }
               }
               for (gch = first_char; gch; gch = gch->next)
               {
                  if (IS_NPC(gch))
                     continue;
                  if (gch == ch)
                     continue;
                  if (!gch->pcdata->quest)
                     continue;
                  if (gch->pcdata->quest->questarea != ch->in_room->area)
                     continue;
                  if (gch->pcdata->quest != ch->pcdata->quest)
                     continue;
                  for (x = 0; x <= 5; x++)
                  {
                     if (gch->pcdata->quest->player[x] == gch->pcdata->pid)
                     {
                        ch_printf(gch, "\n\r^r&CYou have completed your quest, your reward is %d QP^x\n\r\n\r", gch->pcdata->quest->qp[x]);
                        gch->pcdata->quest_curr += gch->pcdata->quest->qp[x];
                        gch->pcdata->quest_accum += gch->pcdata->quest->qp[x];
                        gch->pcdata->quest_wins++;
                        gch->pcdata->quest->timeleft = -1;
                        gch->pcdata->quest->tillnew = 25;
                     }
                  }
               }
               ch->pcdata->quest->timeleft = -1;
               ch->pcdata->quest->tillnew = 25;
            }
         }
      }
   }
   return (oret ? oret : obj);
}

void check_time_resets(OBJ_DATA *obj)
{
   AREA_DATA *pArea;
   RESET_DATA *pReset = NULL;
   
   if (xIS_SET(obj->extra_flags, ITEM_TIMERESET))
   {
      if (obj->carried_by && obj->carried_by->in_room)
         pArea = obj->carried_by->in_room->area;
      else if (obj->in_obj && obj->in_obj->carried_by && obj->in_obj->carried_by->in_room)
         pArea = obj->in_obj->carried_by->in_room->area;
      else if (obj->in_room)
         pArea = obj->in_room->area;
      else if (obj->in_obj && obj->in_obj->in_room)
         pArea = obj->in_obj->in_room->area;
      else
      {
         //if no one "was" carrying it, start looping
         for (pArea = first_area; pArea; pArea = pArea->next)
         {
             for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
             {
                if (pReset->resettime > 0 && (pReset->command == 'O' || pReset->command == 'P' || pReset->command == 'E' || pReset->command == 'G') && pReset->arg1 == obj->pIndexData->vnum )
                {
                   break;
                }
             }
             if (pReset)
                break;
         }     
      }
      if (!pReset && pArea)
      {
         for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
         {
             if (pReset->resettime > 0 && (pReset->command == 'O' || pReset->command == 'P' || pReset->command == 'E' || pReset->command == 'G') && pReset->arg1 == obj->pIndexData->vnum)
                break;
         } 
      } 
      if (!pArea || (pArea && !pReset))
      {
         if (!pArea)
            bug("Could not find the area to fit obj %d's reset.", obj->pIndexData->vnum);
         else
            bug("Could not find the proper Reset for obj %d", obj->pIndexData->vnum);
      }
      else
      {;
         pReset->resetlast = time(0);  
         xREMOVE_BIT(obj->extra_flags, ITEM_TIMERESET);
         bug("TIMERESET LOOTED:  %d has been looted.", obj->pIndexData->vnum); //just in case some brainful player says an item loaded that did not
         fold_area(pArea, pArea->filename, FALSE, 1); 
      }
   }   
      
   if (xIS_SET(obj->extra_flags, ITEM_NORESET))
   {
      if (obj->carried_by && obj->carried_by->in_room)
         pArea = obj->carried_by->in_room->area;
      else if (obj->in_obj && obj->in_obj->carried_by && obj->in_obj->carried_by->in_room)
         pArea = obj->in_obj->carried_by->in_room->area;
      else if (obj->in_room)
         pArea = obj->in_room->area;
      else if (obj->in_obj && obj->in_obj->in_room)
         pArea = obj->in_obj->in_room->area;
      else
      {
         //if no one "was" carrying it, start looping
         for (pArea = first_area; pArea; pArea = pArea->next)
         {
             for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
             {
                if (pReset->resettime == -1 && (pReset->command == 'O' || pReset->command == 'P' || pReset->command == 'E' || pReset->command == 'G') && pReset->arg1 == obj->pIndexData->vnum )
                {
                   break;
                }
             }
             if (pReset)
                break;
         }     
      }
      if (!pReset && pArea)
      {
         for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
         {
             if (pReset->resettime == -1 && (pReset->command == 'O' || pReset->command == 'P' || pReset->command == 'E' || pReset->command == 'G') && pReset->arg1 == obj->pIndexData->vnum)
                break;
         } 
      } 
      if (!pArea || (pArea && !pReset))
      {
         if (!pArea)
            bug("Could not find the area to fit obj %d's reset.", obj->pIndexData->vnum);
         else
            bug("Could not find the proper Reset for obj %d", obj->pIndexData->vnum);
      }
      else
      {
         delete_reset(pArea, pReset);
         xREMOVE_BIT(obj->extra_flags, ITEM_NORESET);
         bug("NORESET LOOTED:  %d has been looted.", obj->pIndexData->vnum); //just in case some brainful player says an item loaded that did not
         fold_area(pArea, pArea->filename, FALSE, 1); 
      }
   }
}      

/*
 * Take an obj from its character.
 */
void obj_from_char(OBJ_DATA * obj)
{
   CHAR_DATA *ch;

   if ((ch = obj->carried_by) == NULL)
   {
      bug("Obj_from_char: null ch.", 0);
      return;
   }

   if (obj->wear_loc != WEAR_NONE)
      unequip_char(ch, obj);

   /* obj may drop during unequip... */
   if (!obj->carried_by)
      return;

   UNLINK(obj, ch->first_carrying, ch->last_carrying, next_content, prev_content);

   if (IS_OBJ_STAT(obj, ITEM_COVERING) && obj->first_content)
      empty_obj(obj, NULL, NULL);

   check_time_resets(obj);

   obj->in_room = NULL;
   obj->carried_by = NULL;
   obj->possessed_by = NULL;
   return;
}


/*
 * Find the ac value of an obj, including position effect.
 */
int apply_ac(OBJ_DATA * obj, int iWear)
{
   return 0;
}



/*
 * Find a piece of eq on a character.
 * Will pick the top layer if clothing is layered.		-Thoric
 */
OBJ_DATA *get_eq_char(CHAR_DATA * ch, int iWear)
{
   OBJ_DATA *obj, *maxobj = NULL;

   for (obj = ch->first_carrying; obj; obj = obj->next_content)
   {
      if (obj->wear_loc == iWear)
      {
         if (!obj->pIndexData->layers)
         {
            return obj;
         }
         else
         {
            if (!maxobj || obj->pIndexData->layers > maxobj->pIndexData->layers)
               maxobj = obj;
         }
      }
   }

   return maxobj;
}



/*
 * Equip a char with an obj.
 */
void equip_char(CHAR_DATA * ch, OBJ_DATA * obj, int iWear)
{
   AFFECT_DATA *paf;
   OBJ_DATA *otmp;

   if ((otmp = get_eq_char(ch, iWear)) != NULL && (!otmp->pIndexData->layers || !obj->pIndexData->layers))
   {
      if (global_drop_equip_message == 0)
         bug("Equip_char: already equipped (%d).", iWear);
      return;
   }
   if (IS_OBJ_STAT(obj, ITEM_KINGDOMEQ) && !IS_OBJ_STAT(obj, ITEM_LODGED))
   {
      if (!IS_NPC(ch) || (IS_NPC(ch) && !xIS_SET(ch->act, ACT_MILITARY)))
      {
         send_to_char("Cannot equip this item, it is kingdom eq\n\r", ch);
         return;
      }
   }

   separate_obj(obj); /* just in case */ 
   obj->wear_loc = iWear;

   for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
      affect_modify(ch, paf, 3);
   for (paf = obj->first_affect; paf; paf = paf->next)
      affect_modify(ch, paf, 3);
   if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room)
      ++ch->in_room->light;
   check_for_trap(ch, obj, -1, NEW_TRAP_WEAROBJ);
   return;
}



/*
 * Unequip a char with an obj.
 */
void unequip_char(CHAR_DATA * ch, OBJ_DATA * obj)
{
   AFFECT_DATA *paf;

   if (obj->wear_loc == WEAR_NONE)
   {
      if (global_drop_equip_message == 0)
         bug("Unequip_char: already unequipped.", 0);
      return;
   }
   
   if (obj->item_type == ITEM_PROJECTILE)
      REMOVE_OBJ_STAT(obj, ITEM_LODGED);

   obj->wear_loc = -1;

   for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
      affect_modify(ch, paf, 2);
   if (obj->carried_by)
      for (paf = obj->first_affect; paf; paf = paf->next)
         affect_modify(ch, paf, 2);

   update_aris(ch);

   if (!obj->carried_by)
   {
      bug("here");
      return;
   }

   if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0 && ch->in_room && ch->in_room->light > 0)
      --ch->in_room->light;
   check_for_trap(ch, obj, TRAP_PUT, NEW_TRAP_REMOVEOBJ); 

   return;
}



/*
 * Count occurrences of an obj in a list.
 */
int count_obj_list(OBJ_INDEX_DATA * pObjIndex, OBJ_DATA * list)
{
   OBJ_DATA *obj;
   int nMatch = 0;

   for (obj = list; obj; obj = obj->next_content)
   {
      if (obj->pIndexData->vnum == pObjIndex->vnum)
      {
         if (obj->count > 1)
            nMatch += obj->count;
         else
            nMatch++;
      }
   }
   return nMatch;
}

int count_mob_in_room(MOB_INDEX_DATA * pMobIndex, ROOM_INDEX_DATA * room)
{
   CHAR_DATA *mob;
   int nMatch = 0;

   for (mob = room->first_person; mob; mob = mob->next_in_room)
      if (IS_NPC(mob) && mob->pIndexData->vnum == pMobIndex->vnum)
         nMatch++;

   return nMatch;
}



/*
 * Move an obj out of a room.
 */
void write_corpses args((CHAR_DATA * ch, char *name, OBJ_DATA * objrem));

int falling;

void obj_from_room(OBJ_DATA * obj)
{
   ROOM_INDEX_DATA *in_room;
   AFFECT_DATA *paf;
   OMAP_DATA *omap;

   if ((in_room = obj->in_room) == NULL)
   {
      bug("obj_from_room: NULL.", 0);
      return;
   }

   for (paf = obj->first_affect; paf; paf = paf->next)
      room_affect(in_room, paf, FALSE);

   for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
      room_affect(in_room, paf, FALSE);

   if (obj->pIndexData->vnum != OBJ_VNUM_MONEY_ONE && obj->pIndexData->vnum != OBJ_VNUM_MONEY_SOME)
   {
      for (omap = first_wilderobj; omap; omap = omap->next)
      {
         if (omap->mapobj == obj)
            UNLINK(omap, first_wilderobj, last_wilderobj, next, prev);
      }
   }

   UNLINK(obj, in_room->first_content, in_room->last_content, next_content, prev_content);



   /* uncover contents */
   if (IS_OBJ_STAT(obj, ITEM_COVERING) && obj->first_content)
      empty_obj(obj, NULL, obj->in_room);

   if (obj->item_type == ITEM_FIRE)
      obj->in_room->light -= obj->count;
      
   check_time_resets(obj);

   obj->carried_by = NULL;
   obj->possessed_by = NULL;
   obj->in_obj = NULL;
   obj->in_room = NULL;
   if (obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling < 1)
      write_corpses(NULL, obj->short_descr + 14, obj);
   return;
}

/*
 * Move an obj into a room.
 */
OBJ_DATA *obj_to_room(OBJ_DATA * obj, ROOM_INDEX_DATA * pRoomIndex, CHAR_DATA * ch)
{
   OBJ_DATA *otmp, *oret;
   sh_int count = obj->count;
   sh_int item_type = obj->item_type;
   AFFECT_DATA *paf;
   OMAP_DATA *omap;

   if (IS_OBJ_STAT(obj, ITEM_GROUNDROT))
   {
      xREMOVE_BIT(obj->extra_flags, ITEM_GROUNDROT);
      obj->timer = 1;
   }

   for (paf = obj->first_affect; paf; paf = paf->next)
      room_affect(pRoomIndex, paf, TRUE);

   for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
      room_affect(pRoomIndex, paf, TRUE);

   for (otmp = pRoomIndex->first_content; otmp; otmp = otmp->next_content)
      if ((oret = group_object(otmp, obj)) == otmp)
      {
         if (item_type == ITEM_FIRE)
            pRoomIndex->light += count;
         return oret;
      }
   LINK(obj, pRoomIndex->first_content, pRoomIndex->last_content, next_content, prev_content);

   if (obj->pIndexData->vnum != OBJ_VNUM_MONEY_ONE && obj->pIndexData->vnum != OBJ_VNUM_MONEY_SOME &&
      pRoomIndex->vnum == OVERLAND_SOLAN)
   {
      CREATE(omap, OMAP_DATA, 1);
      omap->mapobj = obj;
      LINK(omap, first_wilderobj, last_wilderobj, next, prev);
   }

   obj->in_room = pRoomIndex;
   obj->carried_by = NULL;
   obj->in_obj = NULL;
   if (item_type == ITEM_FIRE)
      pRoomIndex->light += count;
   falling++;
   obj_fall(obj, FALSE);
   falling--;

/* Hoping that this will cover all instances of objects from character to room - Samson 8-22-99 */
   if (ch != NULL)
   {
      if (IN_WILDERNESS(ch))
      {
         SET_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = ch->map;
         obj->coord->x = ch->coord->x;
         obj->coord->y = ch->coord->y;
      }
      else
      {
         REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = -1;
         obj->coord->x = -1;
         obj->coord->y = -1;
      }
   }
   if (obj->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && falling < 1)
      write_corpses(NULL, obj->short_descr + 14, NULL);

   return obj;
}


/*
 * Who's carrying an item -- recursive for nested objects	-Thoric
 */
CHAR_DATA *carried_by(OBJ_DATA * obj)
{
   if (obj->in_obj)
      return carried_by(obj->in_obj);

   return obj->carried_by;
}


/*
 * Move an object into an object.
 */
OBJ_DATA *obj_to_obj(OBJ_DATA * obj, OBJ_DATA * obj_to)
{
   OBJ_DATA *otmp, *oret;

   if (obj == obj_to)
   {
      bug("Obj_to_obj: trying to put object inside itself: vnum %d", obj->pIndexData->vnum);
      return obj;
   }
      
   for (otmp = obj_to->first_content; otmp; otmp = otmp->next_content)
      if ((oret = group_object(otmp, obj)) == otmp)
         return oret;

   LINK(obj, obj_to->first_content, obj_to->last_content, next_content, prev_content);

   obj->in_obj = obj_to;
   obj->in_room = NULL;
   obj->carried_by = NULL;

/* This will hopefully cover all objs going into containers on maps - Samson 8-22-99 */
   if (IN_WILDERNESS_OBJ(obj_to))
   {
      SET_OBJ_STAT(obj, ITEM_ONMAP);
      obj->map = obj_to->map;
      obj->coord->x = obj_to->coord->x;
      obj->coord->y = obj_to->coord->y;
   }

   return obj;
}


/*
 * Move an object out of an object.
 */
void obj_from_obj(OBJ_DATA * obj)
{
   OBJ_DATA *obj_from;
   bool magic;

   if ((obj_from = obj->in_obj) == NULL)
   {
      bug("Obj_from_obj: null obj_from.", 0);
      return;
   }

   magic = in_magic_container(obj_from);

   UNLINK(obj, obj_from->first_content, obj_from->last_content, next_content, prev_content);

   /* uncover contents */
   if (IS_OBJ_STAT(obj, ITEM_COVERING) && obj->first_content)
      empty_obj(obj, obj->in_obj, NULL);
      
   check_time_resets(obj);

   obj->in_obj = NULL;
   obj->in_room = NULL;
   obj->carried_by = NULL;

/* This will hopefully cover all objs coming from containers going to the maps - Samson 8-22-99 */
   if (IN_WILDERNESS_OBJ(obj_from))
   {
      SET_OBJ_STAT(obj, ITEM_ONMAP);
      obj->map = obj_from->map;
      obj->coord->x = obj_from->coord->x;
      obj->coord->y = obj_from->coord->y;
   }
   return;
}




/*
 * Extract an obj from the world.
 */
void extract_obj(OBJ_DATA * obj)
{
   OBJ_DATA *obj_content;
   DESCRIPTOR_DATA *d;

   if (obj_extracted(obj))
   {
      bug("extract_obj: obj %d already extracted!", obj->pIndexData->vnum);
      return;
   }

   if (obj->item_type == ITEM_PORTAL)
      remove_portal(obj);

   if (obj->carried_by)
      obj_from_char(obj);
   else if (obj->in_room)
      obj_from_room(obj);
   else if (obj->in_obj)
      obj_from_obj(obj);

   while ((obj_content = obj->last_content) != NULL)
      extract_obj(obj_content);

   /* remove affects */
   {
      AFFECT_DATA *paf;
      AFFECT_DATA *paf_next;

      for (paf = obj->first_affect; paf; paf = paf_next)
      {
         paf_next = paf->next;
         DISPOSE(paf);
      }
      obj->first_affect = obj->last_affect = NULL;
   }
   
   // Remove traps
   if (obj->trap)
   {
      if (obj->trap->uid >= START_INV_TRAP)
      {
         UNLINK(obj->trap, first_trap, last_trap, next, prev);
         DISPOSE(obj->trap);   
         obj->trap = NULL;
         save_trap_file(NULL, NULL);
      }
      else
      {
         obj->trap->area = NULL;
         obj->trap->obj = NULL;
         obj->trap = NULL;
      }
   }

   /* remove extra descriptions */
   {
      EXTRA_DESCR_DATA *ed;
      EXTRA_DESCR_DATA *ed_next;

      for (ed = obj->first_extradesc; ed; ed = ed_next)
      {
         ed_next = ed->next;
         STRFREE(ed->description);
         STRFREE(ed->keyword);
         DISPOSE(ed);
      }
      obj->first_extradesc = obj->last_extradesc = NULL;
   }

   if (obj == gobj_prev)
      gobj_prev = obj->prev;

   for (d = first_descriptor; d; d = d->next)
   {
      OBJ_DATA *dobj;
      if (d->character && d->character->substate && d->character->dest_buf)
      {
         dobj = d->character->dest_buf;
         if ((d->character->substate == SUB_RESTRICTED || d->character->substate == SUB_REPEATCMD)
         &&  dobj == obj)
         {
            send_to_char("Your obj you are modifying is being deleted, please type done to get out of this mode.\n\r", d->character);
            d->character->substate = SUB_NONE;
            d->character->dest_buf = NULL;
            if (d->character->pcdata && d->character->pcdata->subprompt)
            {
               STRFREE(d->character->pcdata->subprompt);
               d->character->pcdata->subprompt = NULL;
               if (xIS_SET(d->character->act, PLR_OSET)) /* oset flags, Crash no like -- Xerves */
                  xREMOVE_BIT(d->character->act, PLR_OSET);
            }
         }
      }
   }

   UNLINK(obj, first_object, last_object, next, prev);

   /* shove onto extraction queue */
   queue_extracted_obj(obj);

   obj->pIndexData->count -= obj->count;
   numobjsloaded -= obj->count;
   --physicalobjects;
   if (obj->serial == cur_obj)
   {
      cur_obj_extracted = TRUE;
      if (global_objcode == rNONE)
         global_objcode = rOBJ_EXTRACTED;
   }
   return;
}

void check_aff_learn(CHAR_DATA *ch, char *skillname, int sn, CHAR_DATA *victim, int success)
{
   if (IS_NPC(ch))
      return;
   if (sn < 1)
      sn = skill_lookup(skillname);
   if (sn < 1)
      return;
   if (LEARNED(ch, sn) <= 0)
      return;
   if (success)
      learn_from_success(ch, sn, victim);
   else
      learn_from_failure(ch, sn, victim);
   return;
}

bool check_npc(CHAR_DATA *ch)
{
   if (IS_NPC(ch))
   {
      send_to_char("This is not for a NPC to use.\n\r", ch);
      return TRUE;
   }
   return FALSE;
}


/*
 * Extract a char from the world.
 */
void extract_char(CHAR_DATA * ch, bool fPull)
{
   CHAR_DATA *wch;
   OBJ_DATA *obj;
   char buf[MSL];
   ROOM_INDEX_DATA *location;
   AREA_DATA *pArea;
   TOWN_DATA *town;
   AGGRO_DATA *aggro;
   AGGRO_DATA *naggro;

   if (!ch)
   {
      bug("Extract_char: NULL ch.", 0);
      return;
   }

   if (!ch->in_room)
   {
      bug("Extract_char: %s in NULL room.", ch->name ? ch->name : "???");
      return;
   }

   if (ch == supermob)
   {
      bug("Extract_char: ch == supermob!", 0);
      return;
   }

   if (char_died(ch))
   {
      bug("extract_char: %s already died!", ch->name);
      return;
   }

   if (ch == cur_char)
      cur_char_died = TRUE;

   /* shove onto extraction queue */
   queue_extracted_char(ch, fPull);

   if (gch_prev == ch)
      gch_prev = ch->prev;

/* DELETE LATER SHADDAI
    if ( fPull && !xIS_SET(ch->act, ACT_POLYMORPHED))
*/
   if (fPull)
      die_follower(ch);

   stop_fighting(ch, TRUE);
   
   pArea = ch->in_room->area;

   if (xIS_SET(ch->in_room->room_flags, ROOM_ARENA))
   {
      ch->hit = ch->max_hit;
      ch->mana = ch->max_mana;
      ch->move = ch->max_move;
   }

   if (ch->rider)
   {
      act(AT_SOCIAL, "Since you are leaving it is a good time for $N to get off of you", ch, NULL, ch->rider, TO_CHAR);
      act(AT_SOCIAL, "$n is leaving, time for your free ride to stop.", ch, NULL, ch->rider, TO_VICT);
      act(AT_PLAIN, "$n is leaving, sadly $N must hop off of $s back.", ch, NULL, ch->rider, TO_ROOM);
      ch->rider->position = POS_STANDING;
      ch->rider->riding = NULL;
      ch->rider = NULL;
   }
   if (ch->riding)
   {
      act(AT_SOCIAL, "You hop off the back of $N before taking your leave.", ch, NULL, ch->riding, TO_CHAR);
      act(AT_SOCIAL, "$n hops off the back of $N before leaving.", ch, NULL, ch->riding, TO_VICT);
      act(AT_PLAIN, "$n hops off the back of $N before departing.", ch, NULL, ch->riding, TO_ROOM);
      ch->rider->riding = NULL;
      ch->rider = NULL;    
      ch->position = POS_STANDING;
   }
   if (ch->mount)
   {
      xREMOVE_BIT(ch->mount->act, ACT_MOUNTED);
      ch->mount = NULL;
      ch->position = POS_STANDING;
   }

   if (IS_NPC(ch))
   {
      if (xIS_SET(ch->act, ACT_MILITARY))
      {
         bug("%s of kingdom %s is being extracted.", ch->name, kingdom_table[ch->m4]->name);
         town = get_town_tpid(ch->m4, ch->m1);
         if (town)
         {
            write_kingdom_file(ch->m4);
         }
      }
      if (xIS_SET(ch->act, ACT_EXTRACTMOB))
      {
         bug("%s of kingdom %s is being extracted.", ch->name, kingdom_table[ch->m4]->name);
         town = get_town_tpid(ch->m4, ch->m7);
         if (town)
         {
            write_kingdom_file(ch->m4);
         }
      }
   }

   /*
    * check if this NPC was a mount or a pet
    */
   if (IS_NPC(ch))
   {
      for (wch = first_char; wch; wch = wch->next)
      {
         if (wch->mount == ch)
         {
            wch->mount = NULL;
            wch->position = POS_STANDING;
            if (wch->in_room == ch->in_room && saving_mount_on_quit == 0)
            {
               act(AT_SOCIAL, "Your faithful mount, $N collapses beneath you...", wch, NULL, ch, TO_CHAR);
               act(AT_SOCIAL, "Sadly you dismount $M for the last time.", wch, NULL, ch, TO_CHAR);
               act(AT_PLAIN, "$n sadly dismounts $N for the last time.", wch, NULL, ch, TO_ROOM);
            }
         }
         if (wch->pcdata && wch->pcdata->pet == ch)
         {
            wch->pcdata->pet = NULL;
            if (wch->in_room == ch->in_room)
               act(AT_SOCIAL, "You mourn for the loss of $N.", wch, NULL, ch, TO_CHAR);
         }
         if (wch->pcdata && wch->pcdata->mount == ch)
         {
            wch->pcdata->mount = NULL;
            if (wch->in_room == ch->in_room && saving_mount_on_quit == 0)
               act(AT_SOCIAL, "You mourn for the loss of $N, but it had to be....", wch, NULL, ch, TO_CHAR);
         }
         if (wch->pcdata && wch->pcdata->aimtarget == ch)
            wch->pcdata->aimtarget = NULL;
      }
   }
   xREMOVE_BIT(ch->act, ACT_MOUNTED);

   while ((obj = ch->last_carrying) != NULL)
      extract_obj(obj);

   if (fPull)
      char_from_room(ch);

   if (!fPull)
   {
      location = NULL;

      if (!IS_NPC(ch) && ch->pcdata->clan)
         location = get_room_index(ch->pcdata->clan->recall);

      if (!IS_NPC(ch) && ch->pcdata->caste < 2)
         location = get_room_index(5644);

      if (!location && !IS_NPC(ch) && ch->pcdata->town)
         location = get_room_index(OVERLAND_SOLAN);

      if (!location)
         location = get_room_index(ROOM_VNUM_ALTAR);

      if (!location)
         location = get_room_index(1);

      if (IS_ONMAP_FLAG(ch))
      {
         REMOVE_ONMAP_FLAG(ch);
         REMOVE_PLR_FLAG(ch, PLR_MAPEDIT); /* Just in case they were editing */

         ch->coord->x = -1;
         ch->coord->y = -1;
         ch->map = -1;
      }

      if (ch->pcdata->mount)
      {

         char_from_room(ch->pcdata->mount);
         SET_ONMAP_FLAG(ch->pcdata->mount);

         ch->pcdata->mount->coord->x = -1;
         ch->pcdata->mount->coord->y = -1;
         ch->pcdata->mount->map = -1;
         char_to_room(ch->pcdata->mount, location);
         ch->pcdata->mount->position = POS_STANDING;
      }
      char_from_room(ch);
      char_to_room(ch, location);
      
      if (location->vnum == OVERLAND_SOLAN)
      {
         ch->coord->x = ch->pcdata->town->death[0];
         ch->coord->y = ch->pcdata->town->death[1];
         ch->map = ch->pcdata->town->death[2];
         SET_ONMAP_FLAG(ch);
         if (ch->pcdata->mount)
         {
            SET_ONMAP_FLAG(ch->pcdata->mount);
            ch->pcdata->mount->coord->x = ch->coord->x;
            ch->pcdata->mount->coord->y = ch->coord->y;
            ch->pcdata->mount->map = ch->map;
         }
      }
      update_objects(ch, ch->map, ch->coord->x, ch->coord->y);
      /*
       * Make things a little fancier    -Thoric
       */
      if ((wch = get_char_room_new(ch, "healer", 1)) != NULL)
      {
         act(AT_MAGIC, "$n mutters a few incantations, waves $s hands and points $s finger.", wch, NULL, NULL, TO_ROOM);
         act(AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM);
         sprintf(buf, "Welcome back to the land of the living, %s", capitalize(ch->name));
         do_tell(wch, buf);
      }
      else
         act(AT_MAGIC, "$n appears from some strange swirling mists!", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_RESTING;
      return;
   }
   if (!IS_NPC(ch))
   {
      while ((obj = ch->pcdata->last_bankobj) != NULL)
      {
         bank_to_char(obj, ch);
         extract_obj(obj);   
      }
   }

   if (IS_NPC(ch))
   {
      RESET_DATA *pReset;
      DESCRIPTOR_DATA *d;
      
      for (d = first_descriptor; d; d = d->next)
      {
         CHAR_DATA *dch;
         if (d->character && d->character->substate && d->character->dest_buf)
         {
            dch = d->character->dest_buf;
            if ((d->character->substate == SUB_RESTRICTED || d->character->substate == SUB_REPEATCMD)
            &&  dch == ch)
            {
               send_to_char("The mobile you are modifying is being deleted, please type done to get out of this mode.\n\r", d->character);
               d->character->substate = SUB_NONE;
               d->character->dest_buf = NULL;
               if (d->character->pcdata && d->character->pcdata->subprompt)
               {
                  STRFREE(d->character->pcdata->subprompt);
                  d->character->pcdata->subprompt = NULL;
                  if (xIS_SET(d->character->act, PLR_MSET)) /* oset flags, Crash no like -- Xerves */
                     xREMOVE_BIT(d->character->act, PLR_MSET);
               }
            }
         }
      }
   
      --ch->pIndexData->count;
      --nummobsloaded;
      if (xIS_SET(ch->act, ACT_NORESET)) //remove the reset....
      {          
         for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
         {
            if (pReset->command == 'M' && pReset->serial == ch->serial)
               break;
         }
         if (!pReset)
         {
            bug("Mobile %d in Area %s cannot find the proper reset to remove itself (No reset mob)", ch->pIndexData->vnum, pArea->name);
         }
         else
         {
            delete_reset(pArea, pReset);
            fold_area(pArea, pArea->filename, FALSE, 1); 
         }
      }
      if (xIS_SET(ch->act, ACT_TIMERESET)) //adjust the looted bit
      {
         for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
         {
            if (pReset->command == 'M' && pReset->serial == ch->serial)
               break;
         }
         if (!pReset)
         {
            bug("Mobile %d in Area %s cannot find the proper reset to remove itself (No reset mob)", ch->pIndexData->vnum, pArea->name);
         }
         else
         {
            pReset->resetlast = time(0);  
            fold_area(pArea, pArea->filename, FALSE, 1); 
         }
      }
      serial_list[ch->serial] = FALSE;
   }

   /* Not sure this should stay or not Shaddai */
/*
    if ( ch->morph )
        do_unmorph( ch );
*/

   if (ch->desc && ch->desc->original)
      do_return(ch, "");
      
   for (aggro = first_global_aggro; aggro; aggro = naggro)
   {
      naggro = aggro->next_global;
      if (aggro->ch == ch)
      {
         UNLINK(aggro, first_global_aggro, last_global_aggro, next_global, prev_global);
         UNLINK(aggro, aggro->owner->first_aggro, aggro->owner->last_aggro, next, prev);
         DISPOSE(aggro);
      }
   }

   for (wch = first_char; wch; wch = wch->next)
   {
      if (wch->reply == ch)
         wch->reply = NULL;
      if (wch->retell == ch)
         wch->retell = NULL;
   }
   /*
      if (IS_NPC(ch))
      {
      for (mch = first_wilderchar; mch; mch = mch->next)
      {
      if (mch->mapch->coord->x == ch->coord->x && mch->mapch->coord->y == ch->coord->y && mch->mapch->map == ch->map)
      {
      UNLINK( mch, first_wilderchar, last_wilderchar, next, prev );
      top_map_mob--;
      }
      }
      }   */

   UNLINK(ch, first_char, last_char, next, prev);



   if (ch->desc)
   {
      if (ch->desc->character != ch)
         bug("Extract_char: char's descriptor points to another char", 0);
      else
      {
         ch->desc->character = NULL;
         close_socket(ch->desc, FALSE);
         ch->desc = NULL;
      }
   }

   return;
}

/*
 * Find a char in the room.
 */
//use get_char_room unless you need to call get_char_room_new for the type (wilderness check)
CHAR_DATA *get_char_room_new(CHAR_DATA * ch, char *argument, int type)
{
   char arg[MIL];
   CHAR_DATA *rch;
   int number, count, vnum;

   number = number_argument(argument, arg);
   if (!str_cmp(arg, "self"))
      return ch;

   if (get_trust(ch) >= LEVEL_IMM && is_number(arg)) /* Tracker1 */
      vnum = atoi(arg);
   else
      vnum = -1;

   count = 0;

   for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
      if (can_see_map(ch, rch) && (nifty_is_name(arg, PERS_MAP_NAME(rch, ch)) || (IS_NPC(rch) && vnum == rch->pIndexData->vnum)))
      {
         if (type == 1)
         {
            if (IS_ONMAP_FLAG(ch) && IS_ONMAP_FLAG(rch))
            {
               if (ch->map != rch->map || ch->coord->x != rch->coord->x || ch->coord->y != rch->coord->y)
                  continue;
            }
         }
         if (number == 0 && !IS_NPC(rch))
            return rch;
         else if (++count == number)
            return rch;
      }

   if (vnum != -1)
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of characters
    * again looking for prefix matching, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
   {
      if (!can_see_map(ch, rch) || !nifty_is_name_prefix(arg, PERS_MAP_NAME(rch, ch)))
         continue;
      if (type == 1)
      {
         if (IS_ONMAP_FLAG(ch) && IS_ONMAP_FLAG(rch))
         {
            if (ch->map != rch->map || ch->coord->x != rch->coord->x || ch->coord->y != rch->coord->y)
               continue;
         }
      }
      if (number == 0 && !IS_NPC(rch))
         return rch;
      else if (++count == number)
         return rch;
   }

   return NULL;
}

//Checks Wilderness by default now
CHAR_DATA *get_char_room(CHAR_DATA * ch, char *argument)
{
   return get_char_room_new(ch, argument, 1);
}

CHAR_DATA *get_char_wilder(CHAR_DATA * ch, char *argument)
{
   return get_char_room_new(ch, argument, -1);
}




/*
 * Find a char in the world.
 */
CHAR_DATA *get_char_world(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *wch;
   int number, count, vnum;

   number = number_argument(argument, arg);
   count = 0;
   if (!str_cmp(arg, "self"))
      return ch;

   /*
    * Allow reference by vnum for saints+   -Thoric
    */
   if (get_trust(ch) >= LEVEL_IMM && is_number(arg)) /* Tracker1 */
      vnum = atoi(arg);
   else
      vnum = -1;

   /* check the room for an exact match */
   for (wch = ch->in_room->first_person; wch; wch = wch->next_in_room)
      if (can_see_map(ch, wch) && (nifty_is_name(arg, PERS_MAP_NAME(wch, ch)) || (IS_NPC(wch) && vnum == wch->pIndexData->vnum)))
      {
         if (number == 0 && !IS_NPC(wch))
            return wch;
         else if (++count == number)
            return wch;
      }

   count = 0;



   /* check the world for an exact match */
   for (wch = first_char; wch; wch = wch->next)
      if (can_see_map(ch, wch) && (nifty_is_name(arg, PERS_MAP_NAME(wch, ch)) || (IS_NPC(wch) && vnum == wch->pIndexData->vnum)))
      {
         if (number == 0 && !IS_NPC(wch))
            return wch;
         else if (++count == number)
            return wch;
      }

   /* bail out if looking for a vnum match */
   if (vnum != -1)
      return NULL;

   /*
    * If we didn't find an exact match, check the room for
    * for a prefix match, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (wch = ch->in_room->first_person; wch; wch = wch->next_in_room)
   {
      if (!can_see_map(ch, wch) || !nifty_is_name_prefix(arg, PERS_MAP_NAME(wch, ch)))
         continue;
      if (number == 0 && !IS_NPC(wch))
         return wch;
      else if (++count == number)
         return wch;
   }

   /*
    * If we didn't find a prefix match in the room, run through the full list
    * of characters looking for prefix matching, ie gu == guard.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (wch = first_char; wch; wch = wch->next)
   {
      if (!can_see_map(ch, wch) || !nifty_is_name_prefix(arg, PERS_MAP_NAME(wch, ch)))
         continue;
      if (number == 0 && !IS_NPC(wch))
         return wch;
      else if (++count == number)
         return wch;
   }

   return NULL;
}



/*
 * Find some object with a given index data.
 * Used by area-reset 'P', 'T' and 'H' commands.
 */
OBJ_DATA *get_obj_type(OBJ_INDEX_DATA * pObjIndex)
{
   OBJ_DATA *obj;

   for (obj = last_object; obj; obj = obj->prev)
      if (obj->pIndexData == pObjIndex)
         return obj;

   return NULL;
}


/*
 * Find an obj in a list.
 */
OBJ_DATA *get_obj_list(CHAR_DATA * ch, char *argument, OBJ_DATA * list)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int number;
   int count;

   number = number_argument(argument, arg);
   count = 0;
   for (obj = list; obj; obj = obj->next_content)
      if (can_see_obj(ch, obj) && nifty_is_name(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = list; obj; obj = obj->next_content)
      if (can_see_obj(ch, obj) && nifty_is_name_prefix(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   return NULL;
}

/*
 * Find an obj in a list...going the other way			-Thoric
 */
OBJ_DATA *get_obj_list_rev(CHAR_DATA * ch, char *argument, OBJ_DATA * list)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int number;
   int count;

   number = number_argument(argument, arg);
   count = 0;
   for (obj = list; obj; obj = obj->prev_content)
      if (can_see_obj(ch, obj) && nifty_is_name(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = list; obj; obj = obj->prev_content)
      if (can_see_obj(ch, obj) && nifty_is_name_prefix(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   return NULL;
}

/*
 * Find an obj in player's inventory or wearing via a vnum -Shaddai
 */

OBJ_DATA *get_obj_vnum(CHAR_DATA * ch, int vnum)
{
   OBJ_DATA *obj;

   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
      if (can_see_obj(ch, obj) && obj->pIndexData->vnum == vnum)
         return obj;
   return NULL;
}

/*
 * Find an obj in player's inventory.
 */
OBJ_DATA *get_obj_carry(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int number, count, vnum;

   number = number_argument(argument, arg);
   if (get_trust(ch) >= LEVEL_IMM && is_number(arg)) /* Tracker1 */
      vnum = atoi(arg);
   else
      vnum = -1;

   count = 0;
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
      if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && (nifty_is_name(arg, obj->name) || obj->pIndexData->vnum == vnum))
         if ((count += obj->count) >= number)
            return obj;

   if (vnum != -1)
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
      if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && nifty_is_name_prefix(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   return NULL;
}



/*
 * Find an obj in player's equipment.
 */
OBJ_DATA *get_obj_wear(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int number, count, vnum;

   number = number_argument(argument, arg);

   if (get_trust(ch) >= LEVEL_IMM && is_number(arg)) /* Tracker1 */
      vnum = atoi(arg);
   else
      vnum = -1;

   count = 0;
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
      if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj) && (nifty_is_name(arg, obj->name) || obj->pIndexData->vnum == vnum))
         if (++count == number)
            return obj;

   if (vnum != -1)
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
      if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj) && nifty_is_name_prefix(arg, obj->name))
         if (++count == number)
            return obj;

   return NULL;
}
      
/*
 * Find an obj in the room or in inventory.
 */
OBJ_DATA *get_obj_here(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;

   obj = get_obj_list_rev(ch, argument, ch->in_room->last_content);
   if (obj)
      return obj;

   if ((obj = get_obj_carry(ch, argument)) != NULL)
      return obj;

   if ((obj = get_obj_wear(ch, argument)) != NULL)
      return obj;

   return NULL;
}



/*
 * Find an obj in the world.
 */
OBJ_DATA *get_obj_world(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int number, count, vnum;

   if ((obj = get_obj_here(ch, argument)) != NULL)
      return obj;

   number = number_argument(argument, arg);

   /*
    * Allow reference by vnum for saints+   -Thoric
    */
   if (get_trust(ch) >= LEVEL_IMM && is_number(arg)) /* Tracker1 */
      vnum = atoi(arg);
   else
      vnum = -1;

   count = 0;
   for (obj = first_object; obj; obj = obj->next)
      if (can_see_obj_map(ch, obj) && (nifty_is_name(arg, obj->name) || vnum == obj->pIndexData->vnum))
         if ((count += obj->count) >= number)
            return obj;

   /* bail out if looking for a vnum */
   if (vnum != -1)
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = first_object; obj; obj = obj->next)
      if (can_see_obj_map(ch, obj) && nifty_is_name_prefix(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   return NULL;
}

/*
 * Find an obj only if it is on a mob...
 */
OBJ_DATA *get_obj_mobworld(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *mobj; /* Check to see if item is same as in inv */
   int number, count, vnum;

   if ((obj = get_obj_here(ch, argument)) != NULL)
      mobj = obj;
   else
      mobj = NULL;

   number = number_argument(argument, arg);

   /*
    * Allow reference by vnum for saints+   -Thoric
    */
   if (get_trust(ch) >= LEVEL_IMM && is_number(arg)) /* Tracker1 */
      vnum = atoi(arg);
   else
      vnum = -1;

   count = 0;
   for (obj = first_object; obj; obj = obj->next)
   {
      if (can_see_obj_map(ch, obj) && (nifty_is_name(arg, obj->name) || vnum == obj->pIndexData->vnum))
         if ((count += obj->count) >= number)
            if ((obj->carried_by != NULL) && (mobj != NULL))
            {
               if ((IS_NPC(obj->carried_by)) && (obj->name == mobj->name) && (obj->pIndexData->vnum > 99))
                  return obj;
            }
   }
   /* bail out if looking for a vnum */
   if (vnum != -1)
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = first_object; obj; obj = obj->next)
   {
      if (can_see_obj_map(ch, obj) && nifty_is_name_prefix(arg, obj->name))
         if ((count += obj->count) >= number)
            if ((obj->carried_by != NULL) && (mobj != NULL))
            {
               if ((IS_NPC(obj->carried_by)) && (obj->name == mobj->name) && (obj->pIndexData->vnum > 99))
                  return obj;
            }
   }
   return NULL;
}

/*
 * How mental state could affect finding an object		-Thoric
 * Used by get/drop/put/quaff/recite/etc
 * Increasingly freaky based on mental state and drunkeness
 */
bool ms_find_obj(CHAR_DATA * ch)
{
   int ms = ch->mental_state;
   int drunk = IS_NPC(ch) ? 0 : ch->pcdata->condition[COND_DRUNK];
   char *t;

   /*
    * we're going to be nice and let nothing weird happen unless
    * you're a tad messed up
    */
   drunk = UMAX(1, drunk);
   if (abs(ms) + (drunk / 3) < 30)
      return FALSE;
   if ((number_percent() + (ms < 0 ? 15 : 5)) > abs(ms) / 2 + drunk / 4)
      return FALSE;
   if (ms > 15) /* range 1 to 20 -- feel free to add more */
      switch (number_range(UMAX(1, (ms / 5 - 15)), (ms + 4) / 5))
      {
         default:
         case 1:
            t = "As you reach for it, you forgot what it was...\n\r";
            break;
         case 2:
            t = "As you reach for it, something inside stops you...\n\r";
            break;
         case 3:
            t = "As you reach for it, it seems to move out of the way...\n\r";
            break;
         case 4:
            t = "You grab frantically for it, but can't seem to get a hold of it...\n\r";
            break;
         case 5:
            t = "It disappears as soon as you touch it!\n\r";
            break;
         case 6:
            t = "You would if it would stay still!\n\r";
            break;
         case 7:
            t = "Whoa!  It's covered in blood!  Ack!  Ick!\n\r";
            break;
         case 8:
            t = "Wow... trails!\n\r";
            break;
         case 9:
            t = "You reach for it, then notice the back of your hand is growing something!\n\r";
            break;
         case 10:
            t = "As you grasp it, it shatters into tiny shards which bite into your flesh!\n\r";
            break;
         case 11:
            t = "What about that huge dragon flying over your head?!?!?\n\r";
            break;
         case 12:
            t = "You stratch yourself instead...\n\r";
            break;
         case 13:
            t = "You hold the universe in the palm of your hand!\n\r";
            break;
         case 14:
            t = "You're too scared.\n\r";
            break;
         case 15:
            t = "Your mother smacks your hand... 'NO!'\n\r";
            break;
         case 16:
            t = "Your hand grasps the worst pile of revoltingness that you could ever imagine!\n\r";
            break;
         case 17:
            t = "You stop reaching for it as it screams out at you in pain!\n\r";
            break;
         case 18:
            t = "What about the millions of burrow-maggots feasting on your arm?!?!\n\r";
            break;
         case 19:
            t = "That doesn't matter anymore... you've found the true answer to everything!\n\r";
            break;
         case 20:
            t = "A supreme entity has no need for that.\n\r";
            break;
      }
   else
   {
      int sub = URANGE(1, abs(ms) / 2 + drunk, 60);

      switch (number_range(1, sub / 10))
      {
         default:
         case 1:
            t = "In just a second...\n\r";
            break;
         case 2:
            t = "You can't find that...\n\r";
            break;
         case 3:
            t = "It's just beyond your grasp...\n\r";
            break;
         case 4:
            t = "...but it's under a pile of other stuff...\n\r";
            break;
         case 5:
            t = "You go to reach for it, but pick your nose instead.\n\r";
            break;
         case 6:
            t = "Which one?!?  I see two... no three...\n\r";
            break;
      }
   }
   send_to_char(t, ch);
   return TRUE;
}


/*
 * Generic get obj function that supports optional containers.	-Thoric
 * currently only used for "eat" and "quaff".
 */
OBJ_DATA *find_obj(CHAR_DATA * ch, char *argument, bool carryonly)
{
   char arg1[MIL];
   char arg2[MIL];
   OBJ_DATA *obj = NULL;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (!str_cmp(arg2, "from") && argument[0] != '\0')
      argument = one_argument(argument, arg2);

   if (arg2[0] == '\0')
   {
      if (carryonly && (obj = get_obj_carry(ch, arg1)) == NULL)
      {
         send_to_char("You do not have that item.\n\r", ch);
         return NULL;
      }
      else if (!carryonly && (obj = get_obj_here(ch, arg1)) == NULL)
      {
         act(AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR);
         return NULL;
      }
      return obj;
   }
   else
   {
      OBJ_DATA *container = NULL;

      if (carryonly && (container = get_obj_carry(ch, arg2)) == NULL && (container = get_obj_wear(ch, arg2)) == NULL)
      {
         send_to_char("You do not have that item.\n\r", ch);
         return NULL;
      }
      if (!carryonly && (container = get_obj_here(ch, arg2)) == NULL)
      {
         act(AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR);
         return NULL;
      }

      if (!IS_OBJ_STAT(container, ITEM_COVERING) && IS_SET(container->value[1], CONT_CLOSED))
      {
         act(AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR);
         return NULL;
      }

      obj = get_obj_list(ch, arg1, container->first_content);
      if (!obj)
         act(AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
            "I see nothing like that beneath $p." : "I see nothing like that in $p.", ch, container, NULL, TO_CHAR);
      return obj;
   }
   return NULL;
}

int get_obj_number(OBJ_DATA * obj)
{
   return obj->count;
}

/*
 * Return TRUE if an object is, or nested inside a magic container
 */
bool in_magic_container(OBJ_DATA * obj)
{
   if (obj->item_type == ITEM_CONTAINER && IS_OBJ_STAT(obj, ITEM_MAGIC))
      return TRUE;
   if (obj->in_obj)
      return in_magic_container(obj->in_obj);
   return FALSE;
}

int objonchar;

void search_container_for_obj(OBJ_DATA * obj, OBJ_DATA * pobj)
{
   OBJ_DATA *cobj;
   
   for (cobj = obj->first_content; cobj; cobj = cobj->next_content)
   {
      if (objonchar == 1)
         break;
      if ((cobj->pIndexData->vnum == pobj->pIndexData->vnum && pobj->pIndexData->vnum != OBJ_VNUM_QUESTOBJ)
      ||  (cobj->pIndexData->vnum == pobj->pIndexData->vnum && pobj->pIndexData->vnum == OBJ_VNUM_QUESTOBJ
      &&   !str_cmp(cobj->name, pobj->name)))
      {
         objonchar = 1;
         break;
      }
      if (cobj->first_content)
         search_container_for_obj(cobj, pobj);
   }
}
         
int get_obj_on_char(CHAR_DATA *ch, OBJ_DATA * pobj)
{
   OBJ_DATA *obj;
   objonchar = 0;
   
   if (IS_NPC(ch))
      return 0;
   
   if (xIS_SET(pobj->extra_flags, ITEM_UNIQUE))
   {
      for (obj = ch->first_carrying; obj; obj = obj->next_content) 
      {
         if (objonchar == 1)
            break;
         if ((obj->pIndexData->vnum == pobj->pIndexData->vnum && pobj->pIndexData->vnum != OBJ_VNUM_QUESTOBJ)
         ||  (obj->pIndexData->vnum == pobj->pIndexData->vnum && pobj->pIndexData->vnum == OBJ_VNUM_QUESTOBJ
         &&   !str_cmp(obj->name, pobj->name)))
         {
            objonchar = 1;
            break;
         }
         if (obj->first_content)
            search_container_for_obj(obj, pobj);
      }   
      for (obj = ch->pcdata->first_bankobj; obj; obj = obj->next_content) 
      {
         if (objonchar == 1)
            break;
         if ((obj->pIndexData->vnum == pobj->pIndexData->vnum && pobj->pIndexData->vnum != OBJ_VNUM_QUESTOBJ)
         ||  (obj->pIndexData->vnum == pobj->pIndexData->vnum && pobj->pIndexData->vnum == OBJ_VNUM_QUESTOBJ
         &&   !str_cmp(obj->name, pobj->name)))
         {
            objonchar = 1;
            break;
         }
         if (obj->first_content)
            search_container_for_obj(obj, pobj);
      }   
   }
   if (pobj->first_content)
   {
      for (obj = pobj->first_content; obj; obj = obj->next_content)
      {
         if (objonchar == 1)
            break;
         if (IS_UNIQUE(ch, obj))
            objonchar = 1;
      }
   }
   return objonchar;
}

/*
 * Return weight of an object, including weight of contents (unless magic).
 */
float get_obj_weight(OBJ_DATA * obj)
{
   float weight;
   float reduction = 100;
   CHAR_DATA *ch = obj->carried_by;

   weight = (float)obj->count * obj->weight;
   
   if (obj->item_type == ITEM_CONTAINER)
   {
      if (ch && ch->apply_wmod > 0)
         reduction = reduction * URANGE(30, (float)ch->apply_wmod, 200) / 100;
         
      weight = weight * reduction / 100;
      
      if (obj->value[2] > 0)
         reduction = (float)obj->value[2];
   }
   else
   {
      if (ch && ch->apply_wmod > 0)
         reduction = reduction * URANGE(30, (float)ch->apply_wmod, 200) / 100;   
      weight = weight * reduction / 100;
   }
   
   /* magic containers */
   for (obj = obj->first_content; obj; obj = obj->next_content)
      weight += get_obj_weight(obj) * reduction / 100;

   return weight;
}

/*
 * Return real weight of an object, including weight of contents.
 */
int get_real_obj_weight(OBJ_DATA * obj)
{
   int weight;

   weight = obj->count * obj->weight;

   for (obj = obj->first_content; obj; obj = obj->next_content)
      weight += get_real_obj_weight(obj);

   return weight;
}



/*
 * True if room is dark.
 */
bool room_is_dark(ROOM_INDEX_DATA * pRoomIndex)
{
   if (!pRoomIndex)
   {
      bug("room_is_dark: NULL pRoomIndex", 0);
      return TRUE;
   }

   if (pRoomIndex->light > 0)
      return FALSE;

   if (xIS_SET(pRoomIndex->room_flags, ROOM_DARK))
      return TRUE;

   if (pRoomIndex->sector_type == SECT_INSIDE || pRoomIndex->sector_type == SECT_CITY || pRoomIndex->sector_type == SECT_ROAD)
      return FALSE;
      
   if (pRoomIndex->vnum == OVERLAND_SOLAN)
      return FALSE;
      
   if (pRoomIndex->sector_type == SECT_UNDERWATER || pRoomIndex->sector_type == SECT_OCEANFLOOR || pRoomIndex->sector_type == SECT_UNDERGROUND)
      return TRUE;

   if (time_info.sunlight == SUN_SET || time_info.sunlight == SUN_DARK)
      return TRUE;

   return FALSE;
}

//True if room is private for wilderness
bool room_is_private_wilderness(CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex, int x, int y, int map)
{
   CHAR_DATA *rch;
   int count;
   int rvalue;
   int oldx, oldy, oldmap;
   ROOM_INDEX_DATA *oldroom;

   if (!pRoomIndex)
   {
      bug("room_is_private: NULL pRoomIndex", 0);
      return FALSE;
   }
   
   oldx = ch->coord->x;
   oldy = ch->coord->y;
   oldmap = ch->map;
   oldroom = ch->in_room;
   
   ch->coord->x = x;
   ch->coord->y = y;
   ch->map = map;
   ch->in_room = pRoomIndex;
   
   if (ch->in_room->vnum == OVERLAND_SOLAN)
      SET_ONMAP_FLAG(ch);
   else
      REMOVE_ONMAP_FLAG(ch);   
   
   if (!IN_WILDERNESS(ch))
   {
      ch->coord->x = oldx;
      ch->coord->y = oldy;
      ch->map = oldmap;
      ch->in_room = oldroom;
      if (ch->in_room->vnum == OVERLAND_SOLAN)
         SET_ONMAP_FLAG(ch);
      else
         REMOVE_ONMAP_FLAG(ch);
      return FALSE;
   }
      
   count = 0;
   for (rch = pRoomIndex->first_person; rch; rch = rch->next_in_room)
   {
      if (rch->coord->x == x && rch->coord->y == y && rch->map == map)
         count++;
   }

   if (wIS_SET(ch, ROOM_PRIVATE) && count >= 2)
      rvalue = TRUE;
   else if (wIS_SET(ch, ROOM_SOLITARY) && count >= 1)
      rvalue = TRUE;
   else
      rvalue = FALSE;
   
   ch->coord->x = oldx;
   ch->coord->y = oldy;
   ch->map = oldmap;
   ch->in_room = oldroom;
   if (ch->in_room->vnum == OVERLAND_SOLAN)
      SET_ONMAP_FLAG(ch);
   else
      REMOVE_ONMAP_FLAG(ch);
   return FALSE;
}
/*
 * True if room is private.
 */
bool room_is_private(ROOM_INDEX_DATA * pRoomIndex)
{
   CHAR_DATA *rch;
   int count;

   if (!pRoomIndex)
   {
      bug("room_is_private: NULL pRoomIndex", 0);
      return FALSE;
   }

   count = 0;
   for (rch = pRoomIndex->first_person; rch; rch = rch->next_in_room)
      count++;

   if (xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE) && count >= 2)
      return TRUE;

   if (xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY) && count >= 1)
      return TRUE;

   return FALSE;
}



/*
 * True if char can see victim.
 */
bool can_see(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!ch)
   {
      if (IS_AFFECTED(victim, AFF_INVISIBLE) || IS_AFFECTED(victim, AFF_STALK) || IS_AFFECTED(victim, AFF_HIDE) || xIS_SET(victim->act, PLR_WIZINVIS))
         return FALSE;
      else
         return TRUE;
   }

   if (ch == victim)
      return TRUE;

   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_WIZINVIS) && get_trust(ch) < victim->pcdata->wizinvis)
      return FALSE;

   /* SB */
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MOBINVIS) && get_trust(ch) <= LEVEL_GUEST)
      return FALSE;

   if (!IS_NPC(victim))
   {
      if (!IS_NPC(ch))
      {
         if (IN_WILDERNESS(ch) || IN_WILDERNESS(victim))
         {
            if (ch->map != victim->map
               || ch->coord->x != victim->coord->x
               || ch->coord->y != victim->coord->y)
               return FALSE;
         }
      }
      else
      {
         if (IN_WILDERNESS(ch) || IN_WILDERNESS(victim))
         {
            if (ch->map != victim->map
               || ch->coord->x != victim->coord->x
               || ch->coord->y != victim->coord->y)
               return FALSE;
         }
      }
   }
   else
   {
      if (!IS_NPC(ch))
      {
         if (IN_WILDERNESS(ch) || IN_WILDERNESS(victim))
         {
            if (ch->map != victim->map
               || ch->coord->x != victim->coord->x
               || ch->coord->y != victim->coord->y)
               return FALSE;
         }
      }
      else
      {
         if (IN_WILDERNESS(ch) || IN_WILDERNESS(victim))
         {
            if (ch->map != victim->map
               || ch->coord->x != victim->coord->x
               || ch->coord->y != victim->coord->y)
               return FALSE;
         }
      }
   }


   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT))
      return TRUE;
   if (IS_AFFECTED(victim, AFF_STALK))
      return FALSE;
   /* The miracle cure for blindness? -- Altrag */
   if (!IS_AFFECTED(ch, AFF_TRUESIGHT))
   {
      if (IS_AFFECTED(ch, AFF_BLIND))
         return FALSE;

      if (room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
         return FALSE;

      if (IS_AFFECTED(victim, AFF_INVISIBLE) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
         return FALSE;

      if (IS_AFFECTED(victim, AFF_HIDE)
         && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
         return FALSE;
   }

   /* Redone by Narn to let newbie council members see pre-auths. */
   if (NOT_AUTHED(victim))
   {
      if (NOT_AUTHED(ch) || IS_IMMORTAL(ch) || IS_NPC(ch))
         return TRUE;

      if (ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council"))
         return TRUE;

      return FALSE;
   }

/* Commented out for who list purposes
    if (!NOT_AUTHED(victim) && NOT_AUTHED(ch) && !IS_IMMORTAL(victim)
    && !IS_NPC(victim))
   	return FALSE;*/
   return TRUE;
}

//Global can_see check, like below but used mob_index, don't want to load mobiles that aren't going to attack (etc)
bool can_see_index(MOB_INDEX_DATA *ch, CHAR_DATA *victim)
{
   if (IS_NPC(victim) && ch == victim->pIndexData)
      return TRUE;
    
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_WIZINVIS))
      return FALSE;

   /* SB */
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MOBINVIS))
      return FALSE;   

   /* The miracle cure for blindness? -- Altrag */
   if (!xIS_SET(ch->affected_by, AFF_TRUESIGHT))
   {
      if (xIS_SET(ch->affected_by, AFF_BLIND))
         return FALSE;

      if (IS_AFFECTED(victim, AFF_INVISIBLE) && !xIS_SET(ch->affected_by, AFF_DETECT_INVIS))
         return FALSE;

      if (IS_AFFECTED(victim, AFF_HIDE)
         && !xIS_SET(ch->affected_by, AFF_DETECT_HIDDEN))
         return FALSE;
   }
   return TRUE;
}
   
   
/* Like can_see, but is used for global things like who */
bool can_see_map(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!ch)
   {
      if (IS_AFFECTED(victim, AFF_INVISIBLE) || IS_AFFECTED(victim, AFF_HIDE) || xIS_SET(victim->act, PLR_WIZINVIS))
         return FALSE;
      else
         return TRUE;
   }

   if (ch == victim)
      return TRUE;

   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_WIZINVIS) && get_trust(ch) < victim->pcdata->wizinvis)
      return FALSE;

   /* SB */
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MOBINVIS) && get_trust(ch) <= LEVEL_GUEST)
      return FALSE;

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT))
      return TRUE;    

   /* The miracle cure for blindness? -- Altrag */
   if (!IS_AFFECTED(ch, AFF_TRUESIGHT))
   {
      if (IS_AFFECTED(ch, AFF_BLIND))
         return FALSE;

      if (room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
         return FALSE;

      if (IS_AFFECTED(victim, AFF_INVISIBLE) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
         return FALSE;

      if (IS_AFFECTED(victim, AFF_HIDE)
         && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
         return FALSE;
   }

   /* Redone by Narn to let newbie council members see pre-auths. */
   /*
   if (NOT_AUTHED(victim))
   {
      if (NOT_AUTHED(ch) || IS_IMMORTAL(ch) || IS_NPC(ch))
         return TRUE;

      if (ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council"))
         return TRUE;

      return FALSE;
   } */

   return TRUE;
}

/*
 * True if char can see obj.
 */
bool can_see_obj(CHAR_DATA * ch, OBJ_DATA * obj)
{

   if (ch->coord->x > 0 || ch->coord->y > 0 || ch->map > -1
   ||  obj->coord->x > 0 || obj->coord->y > 0 || obj->map > -1)
   {
      if (ch->map != obj->map
         || ch->coord->x != obj->coord->x
         || ch->coord->y != obj->coord->y)
         return FALSE;
   }

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT))
      return TRUE;

   if (IS_NPC(ch) && ch->pIndexData->vnum == 3)
      return TRUE;

   if (IS_OBJ_STAT(obj, ITEM_BURIED))
      return FALSE;

   if (IS_AFFECTED(ch, AFF_TRUESIGHT))
      return TRUE;

   if (IS_AFFECTED(ch, AFF_BLIND))
      return FALSE;

   if (IS_OBJ_STAT(obj, ITEM_HIDDEN))
      return FALSE;

   if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
      return TRUE;

   if (room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
      return FALSE;

   if (IS_OBJ_STAT(obj, ITEM_INVIS) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
      return FALSE;

   return TRUE;
}

/* Check for global obj checks on the map */
bool can_see_obj_map(CHAR_DATA * ch, OBJ_DATA * obj)
{
   /*  if ( IS_OBJ_STAT( obj, ITEM_ONMAP ) )
      {
      if( ch->map != obj->map
      || ch->coord->x != obj->coord->x
      || ch->coord->y != obj->coord->y )
      return FALSE;
      }   */

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT))
      return TRUE;

   if (IS_NPC(ch) && ch->pIndexData->vnum == 3)
      return TRUE;

   if (IS_OBJ_STAT(obj, ITEM_BURIED))
      return FALSE;

   if (IS_AFFECTED(ch, AFF_TRUESIGHT))
      return TRUE;

   if (IS_AFFECTED(ch, AFF_BLIND))
      return FALSE;

   if (IS_OBJ_STAT(obj, ITEM_HIDDEN))
      return FALSE;

   if (obj->item_type == ITEM_LIGHT && obj->value[2] != 0)
      return TRUE;

   if (room_is_dark(ch->in_room) && !IS_AFFECTED(ch, AFF_INFRARED))
      return FALSE;

   if (IS_OBJ_STAT(obj, ITEM_INVIS) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
      return FALSE;

   return TRUE;
}

/*
 * True if char can drop obj.
 */
bool can_drop_obj(CHAR_DATA * ch, OBJ_DATA * obj)
{
   if (!IS_OBJ_STAT(obj, ITEM_NODROP))
      return TRUE;

   if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
      return TRUE;

   if (IS_NPC(ch) && ch->pIndexData->vnum == 3)
      return TRUE;

   return FALSE;
}

// Need Color too -- 12/00
char *get_caste_color(int caste)
{
   if (caste >= 1 && caste <= 10)
      return "&Y";
   if (caste >= 11 && caste <= 20)
      return "&P";
   if (caste >= 21 && caste <= 28)
      return "&C";
   if (caste >= 29 && caste <= 30)
      return "&O";
   if (caste >= 31 && caste <= 35)
      return "&G&W";

   return "&z";
}

/*
 * Was getting tired of doing this manually -- Xerves 12/99
 */
char *get_caste_name(int caste, int sex)
{
   if (caste > MAX_CASTE)
   {
      bug("get_caste_name: A caste that exceeded the max was found.", 0);
      return "Casteless";
   }
   if (caste == 1)
      return "Serf";
   else if (caste == 2)
      return "Peasant";
   else if (caste == 3)
      return "Laborer";
   else if (caste == 4)
      return "Apprentice";
   else if (caste == 5)
      return "Journeyman";
   else if (caste == 6)
      return "Master";
   else if (caste == 7)
      return "Merchant";
   else if (caste == 8)
      return "Trader";
   else if (caste == 9 && sex == 0)
      return "Businessman";
   else if (caste == 9 && sex == 1)
      return "Businessman";
   else if (caste == 9 && sex == 2)
      return "Businesswoman";

   else if (caste == 10)
      return "Mayor";

   else if (caste == 11)
      return "Page";
   else if (caste == 12)
      return "Squire";
   else if (caste == 13)
      return "Knight";
   else if (caste == 14)
      return "Baronet";
   else if (caste == 15)
      return "Baron";
   else if (caste == 16)
      return "Earl";
   else if (caste == 17)
      return "Viscount";
   else if (caste == 18)
      return "Count";
   else if (caste == 19)
      return "Duke";
   else if (caste == 20)
      return "Marquis";

   else if (caste == 21)
      return "Vassal";
   else if (caste == 22)
      return "Lord-Vassal";
   else if (caste == 23)
      return "Lord";
   else if (caste == 24)
      return "Hi-Lord";
   else if (caste == 25)
      return "Captain";
   else if (caste == 26)
      return "Minister";
   else if (caste == 27 && sex == 0)
      return "Prince";
   else if (caste == 27 && sex == 1)
      return "Prince";
   else if (caste == 27 && sex == 2)
      return "Princess";
   else if (caste == 28 && sex == 0)
      return "King";
   else if (caste == 28 && sex == 1)
      return "King";
   else if (caste == 28 && sex == 2)
      return "Queen";
   else if (caste == 29)
      return "Avatar";
   else if (caste == 30)
      return "Legend";

   else if (caste == 31)
      return "Ascender";
   else if (caste == 32)
      return "Immortal";
   else if (caste == 33)
      return "Being";
   else if (caste == 34)
      return "Staff";
   else if (caste == 35)
      return "Admin";
   else
      return "Casteless";
}

/*
 * Return ascii name of an item type.
 */
char *item_type_name(OBJ_DATA * obj)
{
   if (obj->item_type < 1 || obj->item_type > MAX_ITEM_TYPE)
   {
      bug("Item_type_name: unknown type %d.", obj->item_type);
      return "(unknown)";
   }

   return o_types[obj->item_type];
}



/*
 * Return ascii name of an affect location.
 */
char *affect_loc_name(int location)
{
   switch (location)
   {
      case APPLY_NONE:
         return "none";
      case APPLY_ARMOR:
         return "armor class";
      case APPLY_SHIELD:
         return "shield";
      case APPLY_STONE:
         return "stone skin";
      case APPLY_SANCTIFY:
         return "damage";
      case APPLY_WMOD:
         return "weightmod";
      case APPLY_MANAFUSE:
         return "manafuse";
      case APPLY_FASTING:
         return "fasting";
      case APPLY_MANASHELL:
         return "manashell";   
      case APPLY_MANASHIELD:
         return "manashield"; 
      case APPLY_MANAGUARD:
         return "managuard"; 
      case APPLY_MANABURN:
         return "manaburn"; 
      case APPLY_WEAPONCLAMP:
         return "weaponclamp"; 
      case APPLY_ARROWCATCH:
         return "arrowcatch"; 
      case APPLY_RFIRE:
         return "rfire"; 
      case APPLY_RWATER:
         return "rwater"; 
      case APPLY_RAIR:
         return "rair"; 
      case APPLY_REARTH:
         return "rearth"; 
      case APPLY_RENERGY:
         return "renergy"; 
      case APPLY_RMAGIC:
         return "rmagic"; 
      case APPLY_RNONMAGIC:
         return "rnonmagic";    
      case APPLY_RBLUNT:
         return "rblunt";  
      case APPLY_RPIERCE:
         return "rpierce";  
      case APPLY_RSLASH:
         return "rslash";
      case APPLY_RPOISON:
         return "rpoison";  
      case APPLY_RPARALYSIS:
         return "rparalysis";  
      case APPLY_RHOLY:
         return "rholy";  
      case APPLY_RUNHOLY:
         return "runholy";  
      case APPLY_RUNDEAD:
         return "rundead";  
      case APPLY_BRACING:
         return "bracing"; 
      case APPLY_HARDENING:
         return "hardening"; 
      case APPLY_TOHIT:
         return "tohit";
      case APPLY_MANATICK:
         return "managen";
      case APPLY_HPTICK:
         return "hpgen";
      case APPLY_STR:
         return "strength";
      case APPLY_DEX:
         return "dexterity";
      case APPLY_INT:
         return "intelligence";
      case APPLY_WIS:
         return "wisdom";
      case APPLY_CON:
         return "constitution";
      case APPLY_CHA:
         return "charisma";
      case APPLY_LCK:
         return "luck";
      case APPLY_SEX:
         return "sex";
      case APPLY_CLASS:
         return "class";
      case APPLY_LEVEL:
         return "level";
      case APPLY_AGE:
         return "age";
      case APPLY_MANA:
         return "mana";
      case APPLY_HIT:
         return "hp";
      case APPLY_MOVE:
         return "moves";
      case APPLY_GOLD:
         return "gold";
      case APPLY_EXP:
         return "experience";
      case APPLY_AC:
         return "----------";
      case APPLY_HITROLL:
         return "hit roll";
      case APPLY_DAMROLL:
         return "damage roll";
      case APPLY_SAVING_POISON:
         return "save vs poison";
      case APPLY_SAVING_ROD:
         return "save vs rod";
      case APPLY_SAVING_PARA:
         return "save vs paralysis";
      case APPLY_SAVING_BREATH:
         return "save vs breath";
      case APPLY_SAVING_SPELL:
         return "save vs spell";
      case APPLY_HEIGHT:
         return "height";
      case APPLY_WEIGHT:
         return "weight";
      case APPLY_AFFECT:
         return "affected_by";
      case APPLY_EXT_AFFECT:
         return "xaffected_by";
      case APPLY_RESISTANT:
         return "resistant";
      case APPLY_IMMUNE:
         return "immune";
      case APPLY_SUSCEPTIBLE:
         return "susceptible";
      case APPLY_BACKSTAB:
         return "backstab";
      case APPLY_PICK:
         return "pick";
      case APPLY_TRACK:
         return "track";
      case APPLY_STEAL:
         return "steal";
      case APPLY_SNEAK:
         return "sneak";
      case APPLY_HIDE:
         return "hide";
      case APPLY_PALM:
         return "palm";
      case APPLY_DETRAP:
         return "detrap";
      case APPLY_DODGE:
         return "dodge";
      case APPLY_PEEK:
         return "peek";
      case APPLY_SCAN:
         return "scan";
      case APPLY_GOUGE:
         return "gouge";
      case APPLY_SEARCH:
         return "search";
      case APPLY_MOUNT:
         return "mount";
      case APPLY_DISARM:
         return "disarm";
      case APPLY_KICK:
         return "kick";
      case APPLY_PARRY:
         return "parry";
      case APPLY_BASH:
         return "bash";
      case APPLY_STUN:
         return "stun";
      case APPLY_PUNCH:
         return "punch";
      case APPLY_CLIMB:
         return "climb";
      case APPLY_GRIP:
         return "grip";
      case APPLY_SCRIBE:
         return "scribe";
      case APPLY_BREW:
         return "brew";
      case APPLY_COOK:
         return "cook";
      case APPLY_WEAPONSPELL:
         return "weapon spell";
      case APPLY_WEARSPELL:
         return "wear spell";
      case APPLY_REMOVESPELL:
         return "remove spell";
      case APPLY_MENTALSTATE:
         return "mental state";
      case APPLY_EMOTION:
         return "emotional state";
      case APPLY_STRIPSN:
         return "dispel";
      case APPLY_REMOVE:
         return "remove";
      case APPLY_DIG:
         return "dig";
      case APPLY_FULL:
         return "hunger";
      case APPLY_THIRST:
         return "thirst";
      case APPLY_DRUNK:
         return "drunk";
      case APPLY_BLOOD:
         return "blood";
      case APPLY_RECURRINGSPELL:
         return "recurring spell";
      case APPLY_CONTAGIOUS:
         return "contagious";
      case APPLY_ODOR:
         return "odor";
      case APPLY_ROOMFLAG:
         return "roomflag";
      case APPLY_SECTORTYPE:
         return "sectortype";
      case APPLY_ROOMLIGHT:
         return "roomlight";
      case APPLY_TELEVNUM:
         return "teleport vnum";
      case APPLY_TELEDELAY:
         return "teleport delay";
      case APPLY_AGI:
         return "agility";  
   };

   bug("Affect_location_name: unknown location %d.", location);
   return "(unknown)";
}



/*
 * Return ascii name of an affect bit vector.
 */
char *affect_bit_name(EXT_BV * vector)
{
   static char buf[512];

   buf[0] = '\0';
   if (xIS_SET(*vector, AFF_BLIND))
      strcat(buf, " blind");
   if (xIS_SET(*vector, AFF_INVISIBLE))
      strcat(buf, " invisible");
   if (xIS_SET(*vector, AFF_DETECT_EVIL))
      strcat(buf, " detect_evil");
   if (xIS_SET(*vector, AFF_DETECT_INVIS))
      strcat(buf, " detect_invis");
   if (xIS_SET(*vector, AFF_DETECT_MAGIC))
      strcat(buf, " detect_magic");
   if (xIS_SET(*vector, AFF_DETECT_HIDDEN))
      strcat(buf, " detect_hidden");
   if (xIS_SET(*vector, AFF_HOLD))
      strcat(buf, " hold");
   if (xIS_SET(*vector, AFF_SANCTUARY))
      strcat(buf, " sanctuary");
   if (xIS_SET(*vector, AFF_FAERIE_FIRE))
      strcat(buf, " faerie_fire");
   if (xIS_SET(*vector, AFF_INFRARED))
      strcat(buf, " infrared");
   if (xIS_SET(*vector, AFF_CURSE))
      strcat(buf, " curse");
   if (xIS_SET(*vector, AFF_FLAMING))
      strcat(buf, " flaming");
   if (xIS_SET(*vector, AFF_POISON))
      strcat(buf, " poison");
   if (xIS_SET(*vector, AFF_PROTECT))
      strcat(buf, " protect");
   if (xIS_SET(*vector, AFF_PARALYSIS))
      strcat(buf, " paralysis");
   if (xIS_SET(*vector, AFF_SLEEP))
      strcat(buf, " sleep");
   if (xIS_SET(*vector, AFF_SNEAK))
      strcat(buf, " sneak");
   if (xIS_SET(*vector, AFF_HIDE))
      strcat(buf, " hide");
   if (xIS_SET(*vector, AFF_CHARM))
      strcat(buf, " charm");
   if (xIS_SET(*vector, AFF_POSSESS))
      strcat(buf, " possess");
   if (xIS_SET(*vector, AFF_FLYING))
      strcat(buf, " flying");
   if (xIS_SET(*vector, AFF_PASS_DOOR))
      strcat(buf, " pass_door");
   if (xIS_SET(*vector, AFF_FLOATING))
      strcat(buf, " floating");
   if (xIS_SET(*vector, AFF_TRUESIGHT))
      strcat(buf, " true_sight");
   if (xIS_SET(*vector, AFF_DETECTTRAPS))
      strcat(buf, " detect_traps");
   if (xIS_SET(*vector, AFF_SCRYING))
      strcat(buf, " scrying");
   if (xIS_SET(*vector, AFF_FIRESHIELD))
      strcat(buf, " fireshield");
   if (xIS_SET(*vector, AFF_SHOCKSHIELD))
      strcat(buf, " shockshield");
   if (xIS_SET(*vector, AFF_ICESHIELD))
      strcat(buf, " iceshield");
   if (xIS_SET(*vector, AFF_BERSERK))
      strcat(buf, " berserk");
   if (xIS_SET(*vector, AFF_AQUA_BREATH))
      strcat(buf, " aqua_breath");
   if (xIS_SET(*vector, AFF_RECURRINGSPELL))
      strcat(buf, " recurringspell");
   if (xIS_SET(*vector, AFF_CONTAGIOUS))
      strcat(buf, " contagious");
   if (xIS_SET(*vector, AFF_WIZARDEYE))
      strcat(buf, " wizardeye");
   if (xIS_SET(*vector, AFF_E_WIZARDEYE))
      strcat(buf, " wizardeye");
   if (xIS_SET(*vector, AFF_M_WIZARDEYE))
      strcat(buf, " wizardeye");
   if (xIS_SET(*vector, AFF_BALANCE))
      strcat(buf, " balance");
   if (xIS_SET(*vector, AFF_NOHUNGER))
      strcat(buf, " nohunger");
   if (xIS_SET(*vector, AFF_NOTHIRST))
      strcat(buf, " nothirst");
   if (xIS_SET(*vector, AFF_GAGGED))
      strcat(buf, " gagged");
   if (xIS_SET(*vector, AFF_REZ))
      strcat(buf, " rez");
   if (xIS_SET(*vector, AFF_WEB))
      strcat(buf, " web");
   if (xIS_SET(*vector, AFF_SNARE))
      strcat(buf, " snare");
   if (xIS_SET(*vector, AFF_NERVEPINCH))
      strcat(buf, " nervepinch");
   if (xIS_SET(*vector, AFF_NYIJI))
      strcat(buf, " nyiji");
   if (xIS_SET(*vector, AFF_STALK))
      strcat(buf, " stalk");
   return (buf[0] != '\0') ? buf + 1 : "none";
}



/*
 * Return ascii name of extra flags vector.
 */
char *extra_bit_name(EXT_BV * extra_flags)
{
   static char buf[512];

   buf[0] = '\0';
   if (xIS_SET(*extra_flags, ITEM_GLOW))
      strcat(buf, " glow");
   if (xIS_SET(*extra_flags, ITEM_HUM))
      strcat(buf, " hum");
   if (xIS_SET(*extra_flags, ITEM_DARK))
      strcat(buf, " dark");
   if (xIS_SET(*extra_flags, ITEM_LOYAL))
      strcat(buf, " loyal");
   if (xIS_SET(*extra_flags, ITEM_EVIL))
      strcat(buf, " evil");
   if (xIS_SET(*extra_flags, ITEM_INVIS))
      strcat(buf, " invis");
   if (xIS_SET(*extra_flags, ITEM_MAGIC))
      strcat(buf, " magic");
   if (xIS_SET(*extra_flags, ITEM_NODROP))
      strcat(buf, " nodrop");
   if (xIS_SET(*extra_flags, ITEM_BLESS))
      strcat(buf, " bless");
   if (xIS_SET(*extra_flags, ITEM_ANTI_GOOD))
      strcat(buf, " anti-good");
   if (xIS_SET(*extra_flags, ITEM_ANTI_EVIL))
      strcat(buf, " anti-evil");
   if (xIS_SET(*extra_flags, ITEM_ANTI_NEUTRAL))
      strcat(buf, " anti-neutral");
   if (xIS_SET(*extra_flags, ITEM_NOREMOVE))
      strcat(buf, " noremove");
   if (xIS_SET(*extra_flags, ITEM_INVENTORY))
      strcat(buf, " inventory");
   if (xIS_SET(*extra_flags, ITEM_DEATHROT))
      strcat(buf, " deathrot");
   if (xIS_SET(*extra_flags, ITEM_GROUNDROT))
      strcat(buf, " groundrot");
   if (xIS_SET(*extra_flags, ITEM_ANTI_MAGE))
      strcat(buf, " anti-mage");
   if (xIS_SET(*extra_flags, ITEM_ANTI_THIEF))
      strcat(buf, " anti-thief");
   if (xIS_SET(*extra_flags, ITEM_ANTI_WARRIOR))
      strcat(buf, " anti-warrior");
   if (xIS_SET(*extra_flags, ITEM_ANTI_CLERIC))
      strcat(buf, " anti-cleric");
   if (xIS_SET(*extra_flags, ITEM_ANTI_DRUID))
      strcat(buf, " anti-druid");
   if (xIS_SET(*extra_flags, ITEM_ANTI_VAMPIRE))
      strcat(buf, " anti-vampire");
   if (xIS_SET(*extra_flags, ITEM_ANTI_PALADIN))
      strcat(buf, " anti-paladin");
   if (xIS_SET(*extra_flags, ITEM_ANTI_MONK))
      strcat(buf, " anti-monk");
   if (xIS_SET(*extra_flags, ITEM_ANTI_AUGURER))
      strcat(buf, " anti-augurer");
   if (xIS_SET(*extra_flags, ITEM_ANTI_RANGER))
      strcat(buf, " anti-ranger");
   if (xIS_SET(*extra_flags, ITEM_ORGANIC))
      strcat(buf, " organic");
   if (xIS_SET(*extra_flags, ITEM_METAL))
      strcat(buf, " metal");
   if (xIS_SET(*extra_flags, ITEM_DONATION))
      strcat(buf, " donation");
   if (xIS_SET(*extra_flags, ITEM_CLANOBJECT))
      strcat(buf, " clan");
   if (xIS_SET(*extra_flags, ITEM_CLANCORPSE))
      strcat(buf, " clanbody");
   if (xIS_SET(*extra_flags, ITEM_PROTOTYPE))
      strcat(buf, " prototype");
   if (xIS_SET(*extra_flags, ITEM_NOGIVE))
      strcat(buf, " nogive");
   if (xIS_SET(*extra_flags, ITEM_NOPURGE))
      strcat(buf, " nopurge");
   return (buf[0] != '\0') ? buf + 1 : "none";
}

/*
 * Return ascii name of magic flags vector. - Scryn
 */
char *magic_bit_name(int magic_flags)
{
   static char buf[512];

   buf[0] = '\0';
   if (magic_flags & ITEM_RETURNING)
      strcat(buf, " returning");
   return (buf[0] != '\0') ? buf + 1 : "none";
}

/*
 * Return ascii name of pulltype exit setting.
 */
char *pull_type_name(int pulltype)
{
   if (pulltype >= PT_FIRE)
      return ex_pfire[pulltype - PT_FIRE];
   if (pulltype >= PT_AIR)
      return ex_pair[pulltype - PT_AIR];
   if (pulltype >= PT_EARTH)
      return ex_pearth[pulltype - PT_EARTH];
   if (pulltype >= PT_WATER)
      return ex_pwater[pulltype - PT_WATER];
   if (pulltype < 0)
      return "ERROR";

   return ex_pmisc[pulltype];
}

/*
 * Set off a trap (obj) upon character (ch)			-Thoric
 */
ch_ret spring_trap(CHAR_DATA * ch, OBJ_DATA * obj, TRAP_DATA *trap)
{
   int dam;
   int typ;
   int lev;
   char *txt;
   char buf[MSL];
   ch_ret retcode;
   CHAR_DATA *victim;
   CHAR_DATA *victim_next;

   if (trap)
   { 
      typ = trap->type;
      lev = (trap->damlow + trap->damhigh) / 2;
      lev = URANGE(3, lev, 90);
   }
   else
   {  
      typ = obj->value[1];
      lev = obj->value[2];
   }

   retcode = rNONE;

   switch (typ)
   {
      default:
         txt = "hit by a trap";
         break;
      case TRAP_TYPE_POISON_GAS:
         txt = "surrounded by a green cloud of gas";
         break;
      case TRAP_TYPE_POISON_DART:
         txt = "hit by a dart";
         break;
      case TRAP_TYPE_POISON_NEEDLE:
         txt = "pricked by a needle";
         break;
      case TRAP_TYPE_POISON_DAGGER:
         txt = "stabbed by a dagger";
         break;
      case TRAP_TYPE_POISON_ARROW:
         txt = "struck with an arrow";
         break;
      case TRAP_TYPE_BLINDNESS_GAS:
         txt = "surrounded by a red cloud of gas";
         break;
      case TRAP_TYPE_SLEEPING_GAS:
         txt = "surrounded by a yellow cloud of gas";
         break;
      case TRAP_TYPE_FLAME:
         txt = "struck by a burst of flame";
         break;
      case TRAP_TYPE_EXPLOSION:
         txt = "hit by an explosion";
         break;
      case TRAP_TYPE_ACID_SPRAY:
         txt = "covered by a spray of acid";
         break;
      case TRAP_TYPE_ELECTRIC_SHOCK:
         txt = "suddenly shocked";
         break;
      case TRAP_TYPE_BLADE:
         txt = "sliced by a razor sharp blade";
         break;
      case TRAP_TYPE_SEX_CHANGE:
         txt = "surrounded by a mysterious aura";
         break;
   }
   if (trap)
      dam = number_range(trap->damlow, trap->damhigh);
   else
      dam = number_range(obj->value[2], obj->value[2] * 2);
   sprintf(buf, "You are %s!", txt);
   act(AT_RED, buf, ch, NULL, NULL, TO_CHAR);
   sprintf(buf, "$n is %s!", txt);
   act(AT_RED, buf, ch, NULL, NULL, TO_ROOM);
   if (trap)
   {
      RESET_DATA *pReset;
      int rfound = 0;
      
      --trap->charges;
      if (trap->onetime == 1 && trap->area)
      {
         for (pReset = trap->area->first_reset; pReset; pReset = pReset->next)
         {
            if (pReset->command == 'A' && pReset->arg1 == trap->uid)
            {
               delete_reset(trap->area, pReset);
               fold_area(trap->area, trap->area->filename, FALSE, 1);
               trap->obj->trap = NULL;
               trap->obj = NULL;
               trap->area = NULL;
               rfound = 1;
               break;
            }
         }
         if (!rfound)
            bug("Trap of uid %d was flagged onetime but could not be found to delete reset.", trap->uid);
      }
   }
   else
   {
      --obj->value[0];
      if (obj->value[0] <= 0)
         extract_obj(obj);
   }
   //spring it on others in the room, disable room fire then re-enable it
   if (trap && trap->room == 1)
   {     
      int charges;
      
      charges = trap->charges;
      trap->room = 0;
      for (victim = ch->in_room->first_person; victim; victim = victim_next)
      {
         victim_next = victim->next_in_room;
         if (IN_SAME_ROOM(ch, victim) && victim != ch)
            spring_trap(victim, NULL, trap);
      }
      trap->charges = charges;
      trap->room = 1;
   }
             
   switch (typ)
   {
      default:
      case TRAP_TYPE_POISON_DART:
      case TRAP_TYPE_POISON_NEEDLE:
      case TRAP_TYPE_POISON_DAGGER:
      case TRAP_TYPE_POISON_ARROW:
         /* hmm... why not use spell_poison() here? */
         retcode = obj_cast_spell(gsn_poison, lev, ch, ch, NULL);
         if (retcode == rNONE)
            retcode = damage(ch, ch, dam, TYPE_UNDEFINED, 0, number_range(LM_BODY, LM_NECK));
         break;
      case TRAP_TYPE_POISON_GAS:
         retcode = obj_cast_spell(gsn_poison, lev, ch, ch, NULL);
         break;
      case TRAP_TYPE_BLINDNESS_GAS:
         retcode = obj_cast_spell(gsn_blindness, lev, ch, ch, NULL);
         break;
      case TRAP_TYPE_SLEEPING_GAS:
         retcode = obj_cast_spell(skill_lookup("sleep"), lev, ch, ch, NULL);
         break;
      case TRAP_TYPE_SEX_CHANGE:
         break;
      case TRAP_TYPE_ACID_SPRAY:
      case TRAP_TYPE_FLAME:
      case TRAP_TYPE_EXPLOSION:;
      case TRAP_TYPE_ELECTRIC_SHOCK:
      case TRAP_TYPE_BLADE:
         retcode = damage(ch, ch, dam, TYPE_UNDEFINED, 0, number_range(LM_BODY, LM_NECK));
   }
   return retcode;
}

ch_ret pre_spring_trap(CHAR_DATA * ch, OBJ_DATA * obj, TRAP_DATA *trap, OBJ_DATA *trapobj)
{
   int frag = 0;
   if (trap)
   {
      if (trap->frag)
      {
         if (number_range(1, 100) <= trap->frag)
         {
            act(AT_DGREY, "The trap armed on $p fires.  The force of the trap shatters $p into pieces", ch, trapobj, NULL, TO_ROOM);
            act(AT_DGREY, "The trap armed on $p fires.  The force of the trap shatters $p into pieces", ch, trapobj, NULL, TO_CHAR);
            frag = 1;
         }
      }
      global_retcode = spring_trap(ch, obj, trap);
      if (trap->charges <= 0 && trap->uid >= START_INV_TRAP)
      {
         UNLINK(trap, first_trap, last_trap, next, prev);
         trapobj->trap = NULL;
         DISPOSE(trapobj->trap);   
         save_trap_file(NULL, NULL);
      }
      if (frag == 1)
      {
         extract_obj(trapobj);
         global_retcode = rOBJ_SCRAPPED;
      }
   }
   else
   {
      global_retcode = spring_trap(ch, obj, obj->trap);
   }
   return global_retcode;   
}

/*
 * Check an object for a trap					-Thoric
 */
ch_ret check_for_trap(CHAR_DATA * ch, OBJ_DATA * obj, int flag, int newflag)
{
   OBJ_DATA *check;
   ch_ret retcode;

   global_retcode = rNONE;
   
   if (obj->trap)
   {
      if (newflag == -1)
         return rNONE;
      if (obj->trap->charges < 1)
         return rNONE;
      if (xIS_SET(obj->trap->trapflags, newflag))
      {
         if (newflag == NEW_TRAP_GET)
         {
            act(AT_RED, "As $n attempts to get something from $p, a trap is fired.", ch, obj, NULL, TO_ROOM);
            act(AT_RED, "As you attempt to get something from $p, a trap is fired.", ch, obj, NULL, TO_CHAR);
         }
         return pre_spring_trap(ch, NULL, obj->trap, obj);
      }
      else
         return rNONE;
   }
   
   if (flag == -1)
      return rNONE;

   if (!obj->first_content)
      return rNONE;

   retcode = rNONE;

   for (check = obj->first_content; check; check = check->next_content)
      if (check->item_type == ITEM_TRAP && IS_SET(check->value[3], flag))
      {
         retcode = spring_trap(ch, check, NULL);
         if (retcode != rNONE)
            return retcode;
      }
   return retcode;
}

/*
 * Check the room for a trap					-Thoric
 */
ch_ret check_room_for_traps(CHAR_DATA * ch, int flag)
{
   OBJ_DATA *check;
   ch_ret retcode;

   retcode = rNONE;

   if (!ch)
      return rERROR;
   if (!ch->in_room || !ch->in_room->first_content)
      return rNONE;

   for (check = ch->in_room->first_content; check; check = check->next_content)
   {
      if (check->item_type == ITEM_TRAP && IS_SET(check->value[3], flag))
      {
         retcode = spring_trap(ch, check, NULL);
         if (retcode != rNONE)
            return retcode;
      }
   }
   return retcode;
}

/*
 * return TRUE if an object contains a trap			-Thoric
 */
bool is_trapped(OBJ_DATA * obj)
{
   OBJ_DATA *check;

   if (obj->trap)
   {
      if (obj->trap->charges > 0)
         return TRUE;
   }

   if (!obj->first_content)
      return FALSE;

   for (check = obj->first_content; check; check = check->next_content)
      if (check->item_type == ITEM_TRAP)
         return TRUE;

   return FALSE;
}

/*
 * If an object contains a trap, return the pointer to the trap	-Thoric
 */
OBJ_DATA *get_trap(OBJ_DATA * obj)
{
   OBJ_DATA *check;

   if (!obj->first_content)
      return NULL;

   for (check = obj->first_content; check; check = check->next_content)
      if (check->item_type == ITEM_TRAP)
         return check;

   return NULL;
}

/*
 * Return a pointer to the first object of a certain type found that
 * a player is carrying/wearing
 */
OBJ_DATA *get_objtype(CHAR_DATA * ch, sh_int type)
{
   OBJ_DATA *obj;

   for (obj = ch->first_carrying; obj; obj = obj->next_content)
      if (obj->item_type == type)
         return obj;

   return NULL;
}

/*
 * Remove an exit from a room					-Thoric
 */
void extract_exit(ROOM_INDEX_DATA * room, EXIT_DATA * pexit)
{
   UNLINK(pexit, room->first_exit, room->last_exit, next, prev);
   if (pexit->rexit)
      pexit->rexit->rexit = NULL;
   STRFREE(pexit->keyword);
   STRFREE(pexit->description);
   if (pexit->coord)
      DISPOSE(pexit->coord);
   DISPOSE(pexit);
}

/*
 * Remove a room
 */
void extract_room(ROOM_INDEX_DATA * room)
{
   bug("extract_room: not implemented", 0);
   /*
      (remove room from hash table)
      clean_room( room )
      DISPOSE( room );
    */
   return;
}

/*
 * clean out a room (leave list pointers intact )		-Thoric
 */
void clean_room(ROOM_INDEX_DATA * room)
{
   EXTRA_DESCR_DATA *ed, *ed_next;
   EXIT_DATA *pexit, *pexit_next;

   STRFREE(room->description);
   STRFREE(room->name);
   for (ed = room->first_extradesc; ed; ed = ed_next)
   {
      ed_next = ed->next;
      STRFREE(ed->description);
      STRFREE(ed->keyword);
      DISPOSE(ed);
      top_ed--;
   }
   room->first_extradesc = NULL;
   room->last_extradesc = NULL;
   for (pexit = room->first_exit; pexit; pexit = pexit_next)
   {
      pexit_next = pexit->next;
      STRFREE(pexit->keyword);
      STRFREE(pexit->description);
      DISPOSE(pexit);
      top_exit--;
   }
   room->first_exit = NULL;
   room->last_exit = NULL;
   xCLEAR_BITS(room->room_flags);
   room->sector_type = 0;
   room->light = 0;
}

/*
 * clean out an object (index) (leave list pointers intact )	-Thoric
 */
void clean_obj(OBJ_INDEX_DATA * obj)
{
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;
   EXTRA_DESCR_DATA *ed;
   EXTRA_DESCR_DATA *ed_next;

   STRFREE(obj->name);
   STRFREE(obj->short_descr);
   STRFREE(obj->description);
   STRFREE(obj->action_desc);
   obj->item_type = 0;
   xCLEAR_BITS(obj->extra_flags);
   obj->wear_flags = 0;
   obj->count = 0;
   obj->weight = 0;
   obj->cost = 0;
   obj->value[0] = 0;
   obj->value[1] = 0;
   obj->value[2] = 0;
   obj->value[3] = 0;
   for (paf = obj->first_affect; paf; paf = paf_next)
   {
      paf_next = paf->next;
      DISPOSE(paf);
      top_affect--;
   }
   obj->first_affect = NULL;
   obj->last_affect = NULL;
   for (ed = obj->first_extradesc; ed; ed = ed_next)
   {
      ed_next = ed->next;
      STRFREE(ed->description);
      STRFREE(ed->keyword);
      DISPOSE(ed);
      top_ed--;
   }
   obj->first_extradesc = NULL;
   obj->last_extradesc = NULL;
}

/*
 * clean out a mobile (index) (leave list pointers intact )	-Thoric
 */
void clean_mob(MOB_INDEX_DATA * mob)
{
   MPROG_DATA *mprog, *mprog_next;

   STRFREE(mob->player_name);
   STRFREE(mob->short_descr);
   STRFREE(mob->long_descr);
   STRFREE(mob->description);
   mob->spec_fun = NULL;
   mob->pShop = NULL;
   mob->rShop = NULL;
   xCLEAR_BITS(mob->progtypes);

   for (mprog = mob->mudprogs; mprog; mprog = mprog_next)
   {
      mprog_next = mprog->next;
      STRFREE(mprog->arglist);
      STRFREE(mprog->comlist);
      DISPOSE(mprog);
   }
   mob->count = 0;
   mob->killed = 0;
   mob->sex = 0;
   mob->level = 0;
   xCLEAR_BITS(mob->act);
   xCLEAR_BITS(mob->affected_by);
   mob->alignment = 0;
   mob->mobthac0 = 0;
   mob->ac = 0;
   mob->hitnodice = 0;
   mob->hitsizedice = 0;
   mob->hitplus = 0;
   mob->damnodice = 0;
   mob->damsizedice = 0;
   mob->damplus = 0;
   mob->gold = 0;
   mob->position = 0;
   mob->defposition = 0;
   mob->height = 0;
   mob->weight = 0; /* mob->vnum  = 0; */
   xCLEAR_BITS(mob->attacks);
   xCLEAR_BITS(mob->defenses);
}

extern int top_reset;

/*
 * Remove all resets from an area				-Thoric
 */
void clean_resets(AREA_DATA * tarea)
{
   RESET_DATA *pReset, *pReset_next;

   for (pReset = tarea->first_reset; pReset; pReset = pReset_next)
   {
      pReset_next = pReset->next;
      DISPOSE(pReset);
      --top_reset;
   }
   tarea->first_reset = NULL;
   tarea->last_reset = NULL;
}


/*
 * "Roll" players stats based on the character name		-Thoric
 */
 /*
    void name_stamp_stats( CHAR_DATA *ch )
    {
    int x, a, b, c;

    for ( x = 0; x < strlen(ch->name); x++ )
    {
    c = ch->name[x] + x;
    b = c % 14;
    a = (c % 1) + 1;
    switch (b)
    {
    case  0:
    ch->perm_str = UMIN( 18, ch->perm_str + a );
    break;
    case  1:
    ch->perm_dex = UMIN( 18, ch->perm_dex + a );
    break;
    case  2:
    ch->perm_wis = UMIN( 18, ch->perm_wis + a );
    break;
    case  3:
    ch->perm_int = UMIN( 18, ch->perm_int + a );
    break;
    case  4:
    ch->perm_con = UMIN( 18, ch->perm_con + a );
    break;
    case  5:
    ch->perm_cha = UMIN( 18, ch->perm_cha + a );
    break;
    case  6:
    ch->perm_lck = UMIN( 18, ch->perm_lck + a );
    break;
    case  7:
    ch->perm_agi = UMIN( 18, ch->perm_agi + a);
    break;
    case  8:
    ch->perm_str = UMAX(  9, ch->perm_str - a );
    break;
    case  9:
    ch->perm_dex = UMAX(  9, ch->perm_dex - a );
    break;
    case  10:
    ch->perm_wis = UMAX(  9, ch->perm_wis - a );
    break;
    case 11:
    ch->perm_int = UMAX(  9, ch->perm_int - a );
    break;
    case 12:
    ch->perm_con = UMAX(  9, ch->perm_con - a );
    break;
    case 13:
    ch->perm_cha = UMAX(  9, ch->perm_cha - a );
    break;
    case 14:
    ch->perm_lck = UMAX(  9, ch->perm_lck - a );
    break;
    case 15:
    ch->perm_agi = UMAX(  9, ch->perm_agi - a );
    break;
    }
    }

    }
    --Removed for Stat rolling Xerves */

/*
 * "Roll" players stats based on the character name             -Thoric
 */
/* Rewritten by Whir. Thanks to Vor/Casteele for help 2-1-98 */
/* Racial bonus calculations moved to this function and removed
   from comm.c - Samson 2-2-98 */
/* Updated to AD&D standards by Samson 9-5-98 */
/* Changed to use internal random number generator instead of
   OS dependant random() function - Samson 9-5-98 */
/* OOH long list, Added support for Prime Stats, and adding
   some HP/Mana/Prac/Train support also -- Xerves 8-1-99*/
// More Changes for 2.0 the comments GROW!!!! -- Xerves 6-01

void name_stamp_stats(CHAR_DATA * ch)
{
   ch->perm_str = 14 + race_table[ch->race]->str_plus;
   ch->perm_dex = 14 + race_table[ch->race]->dex_plus;	
   ch->perm_wis = 14 + race_table[ch->race]->wis_plus;
   ch->perm_int = 14 + race_table[ch->race]->int_plus;
   ch->perm_con = 14 + race_table[ch->race]->con_plus;
   ch->perm_lck = 14 + race_table[ch->race]->lck_plus;
   ch->perm_agi = race_table[ch->race]->agi_start;
   ch->perm_cha = 14;
   ch->pcdata->lore = dice(5, 2);
   ch->max_hit = 18 + dice(3, 3);
   ch->max_mana = 50 + dice(5, 5);
   ch->max_move = 1000;
   ch->gold = 12000 + dice(200, 10);
   ch->max_hit += race_table[ch->race]->hit;
   ch->max_mana += race_table[ch->race]->mana;
   ch->move = ch->max_move;
   ch->hit = ch->max_hit;
   ch->mana = ch->max_mana;

}

/*
 * "Fix" a character's stats					-Thoric
 */
void fix_char(CHAR_DATA * ch)
{
   AFFECT_DATA *aff;
   CHAR_DATA *temp;
   OBJ_DATA *carry[200]; //if they have more than this they need to have them removed
   OBJ_DATA *obj;
   int x, ncarry;

   de_equip_char(ch);

   ncarry = 0;
   while ((obj = ch->first_carrying) != NULL)
   {
      if (ncarry >= 200)
      {
         bug("%s has too many objects in his/her inventory.", ch->name);
         break;
      }
      carry[ncarry++] = obj;
      obj_from_char(obj);
   }
   

   for (aff = ch->first_affect; aff; aff = aff->next)
      affect_modify(ch, aff, FALSE);

   xCLEAR_BITS(ch->affected_by);
   xSET_BITS(ch->affected_by, race_table[ch->race]->affected);
   ch->mental_state = -10;
   ch->hit = UMAX(1, ch->hit);
   ch->mana = UMAX(1, ch->mana);
   ch->move = UMAX(1, ch->move);
   ch->armor = 0;
   ch->mod_str = 0;
   ch->mod_dex = 0;
   ch->mod_wis = 0;
   ch->mod_int = 0;
   ch->mod_con = 0;
   ch->mod_cha = 0;
   ch->mod_lck = 0;
   ch->mod_agi = 0;
   ch->damroll = 0;
   ch->hitroll = 0;
   ch->alignment = URANGE(-1000, ch->alignment, 1000);
   ch->saving_breath = 0;
   ch->saving_wand = 0;
   ch->saving_para_petri = 0;
   ch->saving_spell_staff = 0;
   ch->saving_poison_death = 0;

   for (aff = ch->first_affect; aff; aff = aff->next)
      affect_modify(ch, aff, TRUE);

   for (x = 0; x < ncarry; x++)
   {
      temp = loading_char;
      loading_char = ch;
      obj_to_char(carry[x], ch);
      loading_char = temp;
   }

   re_equip_char(ch);
}

char *specgem_loc_name(int type)
{
   //1000 - Damage  1001 - Durability  1002 - TohitBash  1003 - TohitStab   1004 - TohitSlash
//1005 - Weight  1006 - Shieldlag   1007 - Blocking % 1008 - Proj Range  1009 - Parry Chance 1010 - Stop Parry
//1011 - SpellSN 1012 - SpellStr    1013 - Unbeakable 1014 - Nodisarm    1015 - Sanctified 1016 - Change Size
   if (type == 1000)
      return "Spec Damage";
   else if (type == 1001)
      return "Spec Durability";
   else if (type == 1002)
      return "Spec ToHitBash";
   else if (type == 1003)
      return "Spec ToHitStab";
   else if (type == 1004)
      return "Spec ToHitSlash";
   else if (type == 1005)
      return "Spec Weight";
   else if (type == 1006)
      return "Spec Shieldlag";
   else if (type == 1007)
      return "Spec Blocking Percent";
   else if (type == 1008)
      return "Spec Projectile Range";
   else if (type == 1009)
      return "Spec Parry Chance";
   else if (type == 1010)
      return "Spec Stop Parry";
   else if (type == 1011)
      return "Spec Spell";
   else if (type == 1012)
      return "Spec Spell Strength";
   else if (type == 1013)
      return "Spec Unbreakable Flag";
   else if (type == 1014)
      return "Spec Nodisarm Flag";
   else if (type == 1015)
      return "Spec Sanctified Flag";
   else if (type == 1016)
      return "Spec Change Size";
   else if (type == 1017)
      return "Spec Saves";
   else
      return "Unknown Spec";
}

/*
 * Show an affect verbosely to a character			-Thoric
 */
char *showgemaff(CHAR_DATA * ch, OBJ_DATA *obj, int passbuf, IMBUE_DATA *imbue)
{
   char buf[MSL];
   static char pbuf[MSL];
   char buf2[MSL];
   int x;
   int cnt;
   int v1, v2, v3, v4;

   strcpy(pbuf, "");
   if (!obj && !imbue)
   {
      bug("showgemaff: NULL obj and imbue", 0);
      return NULL;
   }
   for (cnt = 0; cnt < 12; cnt+=4)
   {
      strcpy(buf, "");
      if (obj)
      {
         v1 = obj->value[cnt];
         v2 = obj->value[cnt+1];
         v3 = obj->value[cnt+2];
         v4 = obj->value[cnt+3];
      }
      else
      {
         if (cnt == 0)
         {
            v1 = imbue->type;
            v2 = imbue->sworth;
            v3 = imbue->lowvalue;
            v4 = imbue->highvalue;
         }
         else if (cnt == 4)
         {
            v1 = imbue->type2;
            v2 = imbue->sworth2;
            v3 = imbue->lowvalue2;
            v4 = imbue->highvalue2;
         }
         else
         {
            v1 = imbue->type3;
            v2 = imbue->sworth3;
            v3 = imbue->lowvalue3;
            v4 = imbue->highvalue3;
         }
      }
      if (v1 != APPLY_NONE && v3 != 0)
      {
         switch (v1)
         {
            default:
               if (v1 >= 1000)
               {            
                  if (v1 == 1011)
                  {
                     sprintf(buf, "    Adds %s %s (%d Sworth).\n\r", specgem_loc_name(v1),  skill_table[v3]->name, 
                        v2);
                  }
                  else if (v1 == 1012)
                  {
                     sprintf(buf, "    Adds %s %s from %s (%d Sworth).\n\r", specgem_loc_name(v1), get_wplevel(v3), 
                        get_wplevel(v4), v2);
                  }
                  else
                  {
                     sprintf(buf, "    Adds %s by %d to %d (%d Sworth).\n\r", specgem_loc_name(v1), v3, 
                        v4, v2);
                  }
               }
               else
               {
                  sprintf(buf, "    Adds %s by %d to %d (%d Sworth).\n\r", affect_loc_name(v1), v3, 
                     v4, v2);
               }
               break;
            case APPLY_EXT_AFFECT:
               sprintf(buf, "     Adds %s by %s (%d Sworth).\n\r", affect_loc_name(v1), a_flags[v3], v2);
               break;
            case APPLY_AFFECT:
               sprintf(buf, "    Adds %s by", affect_loc_name(v1));
               for (x = 0; x < 32; x++)
                  if (IS_SET(v3, 1 << x))
                  {
                     strcat(buf, " ");
                        strcat(buf, a_flags[x]);
                  }
               sprintf(buf2, " (%d Sworth)", v2);
               strcat(buf, buf2);
               strcat(buf, "\n\r");
               break;
            case APPLY_WEAPONSPELL:
            case APPLY_WEARSPELL:
            case APPLY_REMOVESPELL:
               sprintf(buf, "    Adds spell '%s' (%d Sworth)\n\r", IS_VALID_SN(v3) ? skill_table[v3]->name : "unknown",
                  v2);
               break;
            case APPLY_RESISTANT:
            case APPLY_IMMUNE:
            case APPLY_SUSCEPTIBLE:
               sprintf(buf, "    Adds %s by", affect_loc_name(v1));
               for (x = 0; x < 32; x++)
                  if (IS_SET(v3, 1 << x))
                  {
                     strcat(buf, " ");
                     strcat(buf, ris_flags[x]);
                  }
               sprintf(buf2, " (%d Sworth)", v2);
               strcat(buf, buf2);
               strcat(buf, "\n\r");
               break;
         }
      }
      strcat(pbuf, buf);
   }
   if (!passbuf)
      send_to_char(pbuf, ch);
    else
      return pbuf;
   return NULL;
}


/*
 * Show an affect verbosely to a character			-Thoric
 */
char *showaffect(CHAR_DATA * ch, AFFECT_DATA * paf, int passbuf)
{
   static char buf[MSL];
   int x;

   strcpy(buf, "");
   if (!paf)
   {
      bug("showaffect: NULL paf", 0);
      return NULL;
   }
   if (paf->location != APPLY_NONE && paf->modifier != 0)
   {
      switch (paf->location)
      {
         default:
            sprintf(buf, "Affects %s by %d.\n\r", affect_loc_name(paf->location), paf->modifier);
            break;
         case APPLY_EXT_AFFECT:
            sprintf(buf, "Affects %s by %s.\n\r", affect_loc_name(paf->location), a_flags[paf->modifier]);
            break;
         case APPLY_AFFECT:
            sprintf(buf, "Affects %s by", affect_loc_name(paf->location));
            for (x = 0; x < 32; x++)
               if (IS_SET(paf->modifier, 1 << x))
               {
                  strcat(buf, " ");
                  strcat(buf, a_flags[x]);
               }
            strcat(buf, "\n\r");
            break;
         case APPLY_WEAPONSPELL:
         case APPLY_WEARSPELL:
         case APPLY_REMOVESPELL:
            sprintf(buf, "Casts spell '%s'\n\r", IS_VALID_SN(paf->modifier) ? skill_table[paf->modifier]->name : "unknown");
            break;
         case APPLY_RESISTANT:
         case APPLY_IMMUNE:
         case APPLY_SUSCEPTIBLE:
            sprintf(buf, "Affects %s by", affect_loc_name(paf->location));
            for (x = 0; x < 32; x++)
               if (IS_SET(paf->modifier, 1 << x))
               {
                  strcat(buf, " ");
                  strcat(buf, ris_flags[x]);
               }
            strcat(buf, "\n\r");
            break;
      }
      if (!passbuf)
         send_to_char(buf, ch);
      else
         return buf;
   }
   return NULL;
}

/*
 * Set the current global object to obj				-Thoric
 */
void set_cur_obj(OBJ_DATA * obj)
{
   cur_obj = obj->serial;
   cur_obj_extracted = FALSE;
   global_objcode = rNONE;
}

/*
 * Check the recently extracted object queue for obj		-Thoric
 */
bool obj_extracted(OBJ_DATA * obj)
{
   OBJ_DATA *cod;

   if (obj->serial == cur_obj && cur_obj_extracted)
      return TRUE;

   for (cod = extracted_obj_queue; cod; cod = cod->next)
      if (obj == cod)
         return TRUE;
   return FALSE;
}

/*
 * Stick obj onto extraction queue
 */
void queue_extracted_obj(OBJ_DATA * obj)
{

   ++cur_qobjs;
   obj->next = extracted_obj_queue;
   extracted_obj_queue = obj;
}

/*
 * Clean out the extracted object queue
 */
void clean_obj_queue()
{
   OBJ_DATA *obj;

   while (extracted_obj_queue)
   {
      obj = extracted_obj_queue;
      extracted_obj_queue = extracted_obj_queue->next;
      STRFREE(obj->name);
      STRFREE(obj->description);
      STRFREE(obj->short_descr);
      STRFREE(obj->action_desc);
      if (obj->coord)
         DISPOSE(obj->coord);
      DISPOSE(obj);
      --cur_qobjs;
   }
}

/*
 * Set the current global character to ch			-Thoric
 */
void set_cur_char(CHAR_DATA * ch)
{
   cur_char = ch;
   cur_char_died = FALSE;
   cur_room = ch->in_room;
   global_retcode = rNONE;
}

/*
 * Check to see if ch died recently				-Thoric
 */
bool char_died(CHAR_DATA * ch)
{
   EXTRACT_CHAR_DATA *ccd;

   if (ch == cur_char && cur_char_died)
      return TRUE;

   for (ccd = extracted_char_queue; ccd; ccd = ccd->next)
      if (ccd->ch == ch)
         return TRUE;
   return FALSE;
}

/*
 * Add ch to the queue of recently extracted characters		-Thoric
 */
void queue_extracted_char(CHAR_DATA * ch, bool extract)
{
   EXTRACT_CHAR_DATA *ccd;

   if (!ch)
   {
      bug("queue_extracted char: ch = NULL", 0);
      return;
   }
   CREATE(ccd, EXTRACT_CHAR_DATA, 1);
   ccd->ch = ch;
   ccd->room = ch->in_room;
   ccd->extract = extract;
   if (ch == cur_char)
      ccd->retcode = global_retcode;
   else
      ccd->retcode = rCHAR_DIED;
   ccd->next = extracted_char_queue;
   extracted_char_queue = ccd;
   cur_qchars++;
}

/*
 * clean out the extracted character queue
 */
void clean_char_queue()
{
   EXTRACT_CHAR_DATA *ccd;

   for (ccd = extracted_char_queue; ccd; ccd = extracted_char_queue)
   {
      extracted_char_queue = ccd->next;
      if (ccd->extract)
         free_char(ccd->ch);
      DISPOSE(ccd);
      --cur_qchars;
   }
}

/*
 * Add a timer to ch						-Thoric
 * Support for "call back" time delayed commands
 */
void add_timer(CHAR_DATA * ch, sh_int type, sh_int count, DO_FUN * fun, int value)
{
   TIMER *timer;

   for (timer = ch->first_timer; timer; timer = timer->next)
      if (timer->type == type)
      {
         timer->count = count;
         timer->do_fun = fun;
         timer->value = value;
         break;
      }
   if (!timer)
   {
      CREATE(timer, TIMER, 1);
      timer->count = count;
      timer->type = type;
      timer->do_fun = fun;
      timer->value = value;
      LINK(timer, ch->first_timer, ch->last_timer, next, prev);
   }
}

void add_obj_timer( OBJ_DATA *obj, sh_int type, sh_int count, DO_FUN *fun, int value )
{
    TIMER *timer;
 
    for ( timer = obj->first_timer; timer; timer = timer->next )
        if ( timer->type == type )
        {
           timer->count  = count;
           timer->do_fun = fun;
           timer->value  = value;
           break;
        }
    if ( !timer )
    {
        CREATE( timer, TIMER, 1 );
        timer->count    = count;
        timer->type     = type;
        timer->do_fun   = fun;
        timer->value    = value;
        LINK( timer, obj->first_timer, obj->last_timer, next, prev );
    }
}   

TIMER *get_timerptr(CHAR_DATA * ch, sh_int type)
{
   TIMER *timer;

   for (timer = ch->first_timer; timer; timer = timer->next)
      if (timer->type == type)
         return timer;
   return NULL;
}


TIMER *get_obj_timerptr( OBJ_DATA *obj, sh_int type )
{
    TIMER *timer;
 
    for ( timer = obj->first_timer; timer; timer = timer->next )
      if ( timer->type == type )
        return timer;
    return NULL;
}    

sh_int get_timer(CHAR_DATA * ch, sh_int type)
{
   TIMER *timer;

   if ((timer = get_timerptr(ch, type)) != NULL)
      return timer->count;
   else
      return 0;
}

sh_int get_obj_timer( OBJ_DATA *obj, sh_int type )
{
    TIMER *timer;
 
    if ( (timer = get_obj_timerptr( obj, type )) != NULL )
      return timer->count;
    else
      return 0;
}    

void extract_timer(CHAR_DATA * ch, TIMER * timer)
{
   if (!timer)
   {
      bug("extract_timer: NULL timer", 0);
      return;
   }

   UNLINK(timer, ch->first_timer, ch->last_timer, next, prev);
   DISPOSE(timer);
   return;
}

void extract_obj_timer( OBJ_DATA *obj, TIMER *timer )
{
    if ( !timer )
    {
        bug( "extract_timer: NULL timer", 0 );
        return;
    }
    separate_obj(obj);
    UNLINK( timer, obj->first_timer, obj->last_timer, next, prev );
    DISPOSE( timer );
    return;
}     

void remove_timer(CHAR_DATA * ch, sh_int type)
{
   TIMER *timer;

   for (timer = ch->first_timer; timer; timer = timer->next)
      if (timer->type == type)
         break;

   if (timer)
      extract_timer(ch, timer);
}

void remove_obj_timer( OBJ_DATA *obj, sh_int type )
{
    TIMER *timer;
 
    for ( timer = obj->first_timer; timer; timer = timer->next )
       if ( timer->type == type )
         break;
 
    if ( timer )
      extract_obj_timer( obj, timer );
} 

bool in_soft_range(CHAR_DATA * ch, AREA_DATA * tarea)
{
   if (IS_IMMORTAL(ch))
      return TRUE;
   else if (IS_NPC(ch))
      return TRUE;
   else if (ch->level >= tarea->low_soft_range || ch->level <= tarea->hi_soft_range)
      return TRUE;
   else
      return FALSE;
}    

bool can_astral(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (victim == ch
      || !victim->in_room
      || xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
      || wIS_SET(victim, ROOM_PRIVATE)
      || xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
      || wIS_SET(victim, ROOM_SOLITARY)
      || xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
      || wIS_SET(victim, ROOM_NO_ASTRAL)
      || xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
      || xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
      || (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE)) || (IS_NPC(victim) && saves_spell_staff(ch->level, victim)))
      return FALSE;
   else
      return TRUE;
}

bool in_hard_range(CHAR_DATA * ch, AREA_DATA * tarea)
{
   if (IS_IMMORTAL(ch))
      return TRUE;
   else if (IS_NPC(ch))
      return TRUE;
   else if (ch->level >= tarea->low_hard_range && ch->level <= tarea->hi_hard_range)
      return TRUE;
   else
      return FALSE;
}


/*
 * Scryn, standard luck check 2/2/96
 */
bool chance(CHAR_DATA * ch, sh_int percent)
{
/*  sh_int clan_factor, ms;*/
   sh_int deity_factor, ms;

   if (!ch)
   {
      bug("Chance: null ch!", 0);
      return FALSE;
   }

   if (IS_DEVOTED(ch))
      deity_factor = ch->pcdata->favor / -500;
   else
      deity_factor = 0;

   ms = 10 - abs(ch->mental_state);

   if ((number_percent() - get_curr_lck(ch) + 13 - ms) + deity_factor <= percent)
      return TRUE;
   else
      return FALSE;
}

bool chance_attrib(CHAR_DATA * ch, sh_int percent, sh_int attrib)
{
/* Scryn, standard luck check + consideration of 1 attrib 2/2/96*/
   sh_int deity_factor;

   if (!ch)
   {
      bug("Chance: null ch!", 0);
      return FALSE;
   }

   if (IS_DEVOTED(ch))
      deity_factor = ch->pcdata->favor / -500;
   else
      deity_factor = 0;

   if (number_percent() - get_curr_lck(ch) + 13 - attrib + 13 + deity_factor <= percent)
      return TRUE;
   else
      return FALSE;

}


/*
 * Make a simple clone of an object (no extras...yet)		-Thoric
 */
OBJ_DATA *clone_object(OBJ_DATA * obj)
{
   OBJ_DATA *clone;
   EXTRA_DESCR_DATA *ed, *ced;
   AFFECT_DATA *paf, *cpaf;

   CREATE(clone, OBJ_DATA, 1);
   clone->pIndexData = obj->pIndexData;
   clone->name = QUICKLINK(obj->name);
   clone->short_descr = QUICKLINK(obj->short_descr);
   clone->description = QUICKLINK(obj->description);
   clone->action_desc = QUICKLINK(obj->action_desc);
   clone->item_type = obj->item_type;
   clone->extra_flags = obj->extra_flags;
   clone->magic_flags = obj->magic_flags;
   clone->wear_flags = obj->wear_flags;
   clone->wear_loc = obj->wear_loc;
   clone->weight = obj->weight;
   clone->cost = obj->cost;
   clone->level = obj->level;
   clone->timer = obj->timer;
   clone->cident = obj->cident;
   CREATE(clone->coord, COORD_DATA, 1);
   clone->map = obj->map;
   clone->coord->x = obj->coord->x;
   clone->coord->y = obj->coord->y;
   clone->value[0] = obj->value[0];
   clone->value[1] = obj->value[1];
   clone->value[2] = obj->value[2];
   clone->value[3] = obj->value[3];
   clone->value[4] = obj->value[4];
   clone->value[5] = obj->value[5];
   clone->value[6] = obj->value[6];
   clone->value[7] = obj->value[7];
   clone->value[8] = obj->value[8];
   clone->value[9] = obj->value[9];
   clone->value[10] = obj->value[10];
   clone->value[11] = obj->value[11];
   clone->value[12] = obj->value[12];
   clone->value[13] = obj->value[13];
   clone->count = 1;
   for (ced = obj->first_extradesc; ced; ced = ced->next)
   {
      CREATE(ed, EXTRA_DESCR_DATA, 1);
      ed->keyword = QUICKLINK(ced->keyword);
      ed->description = QUICKLINK(ced->description);
      LINK(ed, clone->first_extradesc, clone->last_extradesc, next, prev);
      top_ed++;
   }
   for (cpaf = obj->first_affect; cpaf; cpaf = cpaf->next)
   {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = cpaf->type;
      paf->duration = cpaf->duration;
      paf->location = cpaf->location;
      paf->modifier = cpaf->modifier;
      paf->bitvector = cpaf->bitvector;
      LINK(paf, clone->first_affect, clone->last_affect, next, prev);
      top_affect++;
   }   
   ++obj->pIndexData->count;
   ++numobjsloaded;
   ++physicalobjects;
   cur_obj_serial = UMAX((cur_obj_serial + 1) & (BV30 - 1), 1);
   clone->serial = clone->pIndexData->serial = cur_obj_serial;
   LINK(clone, first_object, last_object, next, prev);
   return clone;
}

/*
 * If possible group obj2 into obj1				-Thoric
 * This code, along with clone_object, obj->count, and special support
 * for it implemented throughout handler.c and save.c should show improved
 * performance on MUDs with players that hoard tons of potions and scrolls
 * as this will allow them to be grouped together both in memory, and in
 * the player files.
 */
OBJ_DATA *group_object(OBJ_DATA * obj1, OBJ_DATA * obj2)
{
   if (!obj1 || !obj2)
      return NULL;
   if (obj1 == obj2)
      return obj1;

   if (obj1->pIndexData == obj2->pIndexData
/*
    &&	!obj1->pIndexData->mudprogs
    &&  !obj2->pIndexData->mudprogs
*/
      && QUICKMATCH(obj1->name, obj2->name)
      && QUICKMATCH(obj1->short_descr, obj2->short_descr)
      && QUICKMATCH(obj1->description, obj2->description)
      && QUICKMATCH(obj1->action_desc, obj2->action_desc)
      && obj1->item_type == obj2->item_type
      && xSAME_BITS(obj1->extra_flags, obj2->extra_flags)
      && obj1->magic_flags == obj2->magic_flags
      && obj1->wear_flags == obj2->wear_flags
      && obj1->wear_loc == obj2->wear_loc
      && obj1->weight == obj2->weight
      && obj1->cost == obj2->cost
      && obj1->level == obj2->level
      && obj1->timer == obj2->timer
      && obj1->value[0] == obj2->value[0]
      && obj1->value[1] == obj2->value[1]
      && obj1->value[2] == obj2->value[2]
      && obj1->value[3] == obj2->value[3]
      && obj1->value[4] == obj2->value[4]
      && obj1->value[5] == obj2->value[5]
      && obj1->value[6] == obj2->value[6]
      && obj1->value[7] == obj2->value[7]
      && obj1->value[8] == obj2->value[8]
      && obj1->value[9] == obj2->value[9]
      && obj1->value[10] == obj2->value[10]
      && obj1->value[11] == obj2->value[11]
      && obj1->value[12] == obj2->value[12]
      && obj1->value[13] == obj2->value[13]
      && !obj1->first_extradesc && !obj2->first_extradesc
      && !obj1->first_affect && !obj2->first_affect && !obj1->first_content && !obj2->first_content && obj1->count + obj2->count > 0 /* prevent count overflow */
      && obj1->map == obj2->map
      && obj1->coord->x == obj2->coord->x
      && obj1->coord->y == obj2->coord->y
      && !obj1->trap && !obj2->trap)
   {
      obj1->count += obj2->count;
      obj1->pIndexData->count += obj2->count; /* to be decremented in */
      numobjsloaded += obj2->count; /* extract_obj */
      extract_obj(obj2);
      return obj1;
   }
   return obj2;
}

/*
 * Split off a grouped object					-Thoric
 * decreased obj's count to num, and creates a new object containing the rest
 */
OBJ_DATA *split_obj(OBJ_DATA * obj, int num)
{
   int count = obj->count;
   OBJ_DATA *rest;

   if (count <= num || num == 0)
      return NULL;

   rest = clone_object(obj);
   --obj->pIndexData->count; /* since clone_object() ups this value */
   --numobjsloaded;
   rest->count = obj->count - num;
   obj->count = num;

   if (obj->carried_by)
   {
      LINK(rest, obj->carried_by->first_carrying, obj->carried_by->last_carrying, next_content, prev_content);
      rest->carried_by = obj->carried_by;
      rest->possessed_by = obj->possessed_by;
      rest->in_room = NULL;
      rest->in_obj = NULL;
   }
   else if (obj->in_room)
   {
      LINK(rest, obj->in_room->first_content, obj->in_room->last_content, next_content, prev_content);
      rest->carried_by = NULL;
      rest->possessed_by = NULL;
      rest->in_room = obj->in_room;
      rest->in_obj = NULL;
   }
   else if (obj->in_obj)
   {
      LINK(rest, obj->in_obj->first_content, obj->in_obj->last_content, next_content, prev_content);
      rest->in_obj = obj->in_obj;
      rest->in_room = NULL;
      rest->carried_by = NULL;
      rest->possessed_by = NULL;
   }
   return rest;
}

OBJ_DATA *separate_obj(OBJ_DATA * obj)
{
   return split_obj(obj, 1);
}

/*
 * Empty an obj's contents... optionally into another obj, or a room
 */
bool empty_obj(OBJ_DATA * obj, OBJ_DATA * destobj, ROOM_INDEX_DATA * destroom)
{
   OBJ_DATA *otmp, *otmp_next;
   CHAR_DATA *ch = obj->carried_by;
   bool movedsome = FALSE;

   if (!obj)
   {
      bug("empty_obj: NULL obj", 0);
      return FALSE;
   }
   if (destobj || (!destroom && !ch && (destobj = obj->in_obj) != NULL))
   {
      for (otmp = obj->first_content; otmp; otmp = otmp_next)
      {
         otmp_next = otmp->next_content;
         /* only keys on a keyring */
         if (destobj->item_type == ITEM_KEYRING && otmp->item_type != ITEM_KEY)
            continue;
         if (destobj->item_type == ITEM_QUIVER && otmp->item_type != ITEM_PROJECTILE)
            continue;
         if ((destobj->item_type == ITEM_CONTAINER || destobj->item_type == ITEM_KEYRING
               || destobj->item_type == ITEM_QUIVER) && get_real_obj_weight(otmp) + get_real_obj_weight(destobj) > destobj->value[0])
            continue;
         obj_from_obj(otmp);
         obj_to_obj(otmp, destobj);
         movedsome = TRUE;
      }
      return movedsome;
   }
   if (destroom || (!ch && (destroom = obj->in_room) != NULL))
   {
      for (otmp = obj->first_content; otmp; otmp = otmp_next)
      {
         otmp_next = otmp->next_content;
         if (ch && HAS_PROG(otmp->pIndexData, DROP_PROG) && otmp->count > 1)
         {
            separate_obj(otmp);
            obj_from_obj(otmp);
            if (!otmp_next)
               otmp_next = obj->first_content;
         }
         else
            obj_from_obj(otmp);
         otmp = obj_to_room(otmp, destroom, ch);
         if (ch)
         {
            oprog_drop_trigger(ch, otmp); /* mudprogs */
            if (char_died(ch))
               ch = NULL;
         }
         movedsome = TRUE;
      }
      return movedsome;
   }
   if (ch)
   {
      for (otmp = obj->first_content; otmp; otmp = otmp_next)
      {
         otmp_next = otmp->next_content;
         obj_from_obj(otmp);
         obj_to_char(otmp, ch);
         movedsome = TRUE;
      }
      return movedsome;
   }
   bug("empty_obj: could not determine a destination for vnum %d", obj->pIndexData->vnum);
   return FALSE;
}

/*
 * Improve mental state						-Thoric
 */
void better_mental_state(CHAR_DATA * ch, int mod)
{
   int c = URANGE(0, abs(mod), 20);
   int con = get_curr_con(ch);

   c += number_percent() < con ? 1 : 0;

   if (ch->mental_state < 0)
      ch->mental_state = URANGE(-100, ch->mental_state + c, 0);
   else if (ch->mental_state > 0)
      ch->mental_state = URANGE(0, ch->mental_state - c, 100);
}

/*
 * Deteriorate mental state					-Thoric
 */
void worsen_mental_state(CHAR_DATA * ch, int mod)
{
   int c = URANGE(0, abs(mod), 20);
   int con = get_curr_con(ch);

   c -= number_percent() < con ? 1 : 0;
   if (c < 1)
      return;

   /* Nuisance flag makes state worsen quicker. --Shaddai */
   if (!IS_NPC(ch) && ch->pcdata->nuisance && ch->pcdata->nuisance->flags > 2)
      c += .4 * ((ch->pcdata->nuisance->flags - 2) * ch->pcdata->nuisance->power);

   if (ch->mental_state < 0)
      ch->mental_state = URANGE(-100, ch->mental_state - c, 100);
   else if (ch->mental_state > 0)
      ch->mental_state = URANGE(-100, ch->mental_state + c, 100);
   else
      ch->mental_state -= c;
}


/*
 * Add gold to an area's economy				-Thoric
 */
void boost_economy(AREA_DATA * tarea, int gold)
{
   while (gold >= 1000000000)
   {
      ++tarea->high_economy;
      gold -= 1000000000;
   }
   tarea->low_economy += gold;
   while (tarea->low_economy >= 1000000000)
   {
      ++tarea->high_economy;
      tarea->low_economy -= 1000000000;
   }
}

/*
 * Take gold from an area's economy				-Thoric
 */
void lower_economy(AREA_DATA * tarea, int gold)
{
   while (gold >= 1000000000)
   {
      --tarea->high_economy;
      gold -= 1000000000;
   }
   tarea->low_economy -= gold;
   while (tarea->low_economy < 0)
   {
      --tarea->high_economy;
      tarea->low_economy += 1000000000;
   }
}

/*
 * Check to see if economy has at least this much gold		   -Thoric
 */
bool economy_has(AREA_DATA * tarea, int gold)
{
   int hasgold = ((tarea->high_economy > 0) ? 1 : 0) * 1000000000 + tarea->low_economy;

   if (hasgold >= gold)
      return TRUE;
   return FALSE;
}

/*
 * Used in db.c when resetting a mob into an area		    -Thoric
 * Makes sure mob doesn't get more than 10% of that area's gold,
 * and reduces area economy by the amount of gold given to the mob
 */
void economize_mobgold(CHAR_DATA * mob)
{
   int gold;
   AREA_DATA *tarea;

   /* make sure it isn't way too much */
   mob->gold = UMIN(mob->gold, 100000);
   if (!mob->in_room)
      return;
   tarea = mob->in_room->area;

   gold = ((tarea->high_economy > 0) ? 1 : 0) * 1000000000 + tarea->low_economy;
   mob->gold = URANGE(0, mob->gold, gold / 10);
   if (mob->gold)
      lower_economy(tarea, mob->gold);
}


/*
 * Add another notch on that there belt... ;)
 * Keep track of the last so many kills by vnum			-Thoric
 */
void add_kill(CHAR_DATA * ch, CHAR_DATA * mob)
{
   int x;
   sh_int vnum, track;

   if (IS_NPC(ch))
   {
      bug("add_kill: trying to add kill to npc", 0);
      return;
   }
   if (!IS_NPC(mob))
   {
      bug("add_kill: trying to add kill non-npc", 0);
      return;
   }
   vnum = mob->pIndexData->vnum;
   track = MAX_KILLTRACK;
   for (x = 0; x < track; x++)
      if (ch->pcdata->killed[x].vnum == vnum)
      {
         if (ch->pcdata->killed[x].count < 50)
            ++ch->pcdata->killed[x].count;
         return;
      }
      else if (ch->pcdata->killed[x].vnum == 0)
         break;
   memmove((char *) ch->pcdata->killed + sizeof(KILLED_DATA), ch->pcdata->killed, (track - 1) * sizeof(KILLED_DATA));
   ch->pcdata->killed[0].vnum = vnum;
   ch->pcdata->killed[0].count = 1;
   if (track < MAX_KILLTRACK)
      ch->pcdata->killed[track].vnum = 0;
}

void update_pkpower(CHAR_DATA * ch)
{
   //Boost the power if they reach a certain point
   if (ch->pcdata->pranking >= 15 && ch->pcdata->pkpower < 1)
   {
      act(AT_RED, "$n flashes with a &BBlue&R Light as power flows through $s vains.", ch, NULL, NULL, TO_CANSEE);
      act(AT_RED, "Your body twitches as a &BBlue&R Light pulsates throughout your vains.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower++;
      ch->max_hit += 75;
      ch->max_mana += 75;
      ch->hit += 75;
      ch->move += 75;
      ch->mana += 75;
      ch->perm_agi += 5;
   }
   if (ch->pcdata->pranking >= 35 && ch->pcdata->pkpower < 2)
   {
      act(AT_RED, "$n flashes with a &GGreen&R Light as power flows through $s vains.", ch, NULL, NULL, TO_CANSEE);
      act(AT_RED, "Your body twitches as a &GGreen&R Light pulsates throughout your vains.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower++;
      ch->max_hit += 100;
      ch->max_mana += 100;
      ch->hit += 100;
      ch->move += 100;
      ch->mana += 100;
      ch->perm_agi += 10;
   }
   if (ch->pcdata->pranking >= 60 && ch->pcdata->pkpower < 3)
   {
      act(AT_RED, "$n flashes with a &G&WPURE WHITE&R Light as power flows through $s vains.", ch, NULL, NULL, TO_CANSEE);
      act(AT_RED, "Your body twitches as a &G&WPURE WHITE&R Light pulsates throughout your vains.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower++;
      ch->max_hit += 150;
      ch->max_mana += 150;
      ch->hit += 150;
      ch->move += 150;
      ch->mana += 150;
      ch->perm_agi += 20;
   }

   //now take it away if they go below it
   if (ch->pcdata->pranking < 15 && ch->pcdata->pkpower == 1)
   {
      act(AT_RED, "$n flashes with a &BBlue&R Light as power flows from $s vains.", ch, NULL, NULL, TO_CANSEE);
      act(AT_RED, "Your body twitches as a &BBlue&R Light leaves your body.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower--;
      ch->max_hit -= 75;
      ch->max_mana -= 75;
      ch->perm_agi -= 5;
   }
   if (ch->pcdata->pranking < 35 && ch->pcdata->pkpower == 2)
   {
      act(AT_RED, "$n flashes with a &GGreen&R Light as power flows from $s vains.", ch, NULL, NULL, TO_CANSEE);
      act(AT_RED, "Your body twitches as a &GGreen&R Light leaves your body.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower--;
      ch->max_hit -= 100;
      ch->max_mana -= 100;
      ch->perm_agi -= 10;
   }
   if (ch->pcdata->pranking < 60 && ch->pcdata->pkpower == 3)
   {
      act(AT_RED, "$n flashes with a &G&WPURE WHITE&R Light as power flows from $s vains.", ch, NULL, NULL, TO_CANSEE);
      act(AT_RED, "Your body twitches as a &G&WPURE WHITE&R Light leaves your body.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower--;
      ch->max_hit -= 150;
      ch->max_mana -= 150;
      ch->perm_agi -= 20;
   }

   //For those who just die a lot, zap them for a few, must be over level 20
   if (ch->pcdata->pranking < -20 && ch->pcdata->pkpower >= 0)
   {
      act(AT_ORANGE, "$n flashes with a &rBLOOD RED&O as $e is punished for $s defeats.", ch, NULL, NULL, TO_CANSEE);
      act(AT_ORANGE, "Your body shakes as a &rBLOOD RED&O Light enters your defeated body.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower--;
      ch->max_hit -= 60;
      ch->max_mana -= 60;
      ch->perm_agi -= 15;
   }
   //Now just fix em
   if (ch->pcdata->pranking >= -20 && ch->pcdata->pkpower == -1)
   {
      act(AT_ORANGE, "$n flashes with a &rBLOOD RED&O Light as the curse leaves $s body.", ch, NULL, NULL, TO_CANSEE);
      act(AT_ORANGE, "Your body shakes as a &rBLOOD RED&O Light leaves your body.", ch, NULL, NULL, TO_CHAR);
      ch->pcdata->pkpower++;
      ch->max_hit += 60;
      ch->max_mana += 60;
      ch->perm_agi += 15;
      ch->hit += 60;
      ch->mana += 60;
   }
}

//Update the pranking (pkill Ranking) for a kill
void update_pranking(CHAR_DATA * ch, CHAR_DATA * victim)
{
   PKILLED_DATA *pkl;
   int cnt = 0;
   int camt, vamt, diff;

   camt = vamt = 0;
   
   //For now I don't want to add any pranking
   return;

   if (IN_ARENA(ch) || IN_ARENA(victim))
      return;
   camt = 1;
   vamt = 1;
   //Tack on an extra point for every 5 points of power difference
   diff = victim->pcdata->pranking - ch->pcdata->pranking;
   for (cnt = 5; cnt <= diff; cnt += 5)
   {
      camt++;
      vamt++;
   }
   cnt = 0;
   for (pkl = ch->pcdata->first_pkilled; pkl; pkl = pkl->next)
   {
      if (!str_cmp(pkl->name, victim->name))
         cnt++;
   }
   if (cnt > 4)
      camt = 0;

   if (diff < -20)
      camt = 0;

   ch->pcdata->pranking += camt;
   victim->pcdata->pranking -= vamt;

   if (victim->pcdata->pranking < -30)
      victim->pcdata->pranking = -30;

   update_pkpower(ch);
   update_pkpower(victim);
}

//Add the name of the victim to the pkill list on the player 
void add_pkill(CHAR_DATA * ch, CHAR_DATA * victim)
{
   PKILLED_DATA *pkl;

   if (IS_NPC(victim))
   {
      bug("add_pkill: %s is trying to add %s to the pkill listing", ch->name, victim->name);
      return;
   }
   if (IS_NPC(ch))
   {
      bug("add_pkill: %s is a NPC trying to add a PC to its listing", ch->name);
      return;
   }
   if (ch->pcdata->pkilled < MAX_PKILLTRACK)
   {
      CREATE(pkl, PKILLED_DATA, 1);
      pkl->name = STRALLOC(victim->name);
      LINK(pkl, ch->pcdata->first_pkilled, ch->pcdata->last_pkilled, next, prev);
      ch->pcdata->pkilled++;
   }
   else
   {
      pkl = ch->pcdata->first_pkilled;
      UNLINK(pkl, ch->pcdata->first_pkilled, ch->pcdata->last_pkilled, next, prev);
      STRFREE(pkl->name);
      DISPOSE(pkl);

      CREATE(pkl, PKILLED_DATA, 1);
      pkl->name = STRALLOC(victim->name);
      LINK(pkl, ch->pcdata->first_pkilled, ch->pcdata->last_pkilled, next, prev);
   }
}

/*
 * Return how many times this player has killed this mob	-Thoric
 * Only keeps track of so many (MAX_KILLTRACK), and keeps track by vnum
 */
int times_killed(CHAR_DATA * ch, CHAR_DATA * mob)
{
   int x;
   sh_int vnum, track;

   if (IS_NPC(ch))
   {
      bug("times_killed: ch is not a player", 0);
      return 0;
   }
   if (!IS_NPC(mob))
   {
      bug("add_kill: mob is not a mobile", 0);
      return 0;
   }

   vnum = mob->pIndexData->vnum;
   track = MAX_KILLTRACK;
   for (x = 0; x < track; x++)
      if (ch->pcdata->killed[x].vnum == vnum)
         return ch->pcdata->killed[x].count;
      else if (ch->pcdata->killed[x].vnum == 0)
         break;
   return 0;
}

/*
 * returns area with name matching input string
 * Last Modified : July 21, 1997
 * Fireblade
 */
AREA_DATA *get_area(char *name)
{
   AREA_DATA *pArea;

   if (!name)
   {
      bug("get_area: NULL input string.");
      return NULL;
   }

   for (pArea = first_area; pArea; pArea = pArea->next)
   {
      if (nifty_is_name(name, pArea->name))
         break;
   }

   if (!pArea)
   {
      for (pArea = first_build; pArea; pArea = pArea->next)
      {
         if (nifty_is_name(name, pArea->name))
            break;
      }
   }

   return pArea;
}

/* Hometown Ivan Code -- Xerves */
int get_hometown(char *argument)
{
   int i = 0;

   while (i < sysdata.max_kingdom)
   {
      if (!str_cmp(argument, kingdom_table[i]->name))
         return i;
      i++;
   }
   return -1;
}
