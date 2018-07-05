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
 *			      Regular update module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "mud.h"



/*
 * Local functions.
 */
int hit_gain args((CHAR_DATA * ch));
int mana_gain args((CHAR_DATA * ch));
int move_gain args((CHAR_DATA * ch));
void mobile_update args((void));
void weather_update args((void));
void time_update args((void)); /* FB */
void char_update args((void));
void char_speed_update args((void));
void obj_speed_update args((void));
void obj_update args((void));
void aggr_update args((void));
void who_update args((void));
void room_act_update args((void));
void obj_act_update args((void));
void char_check args((void));
void drunk_randoms args((CHAR_DATA * ch));
void hallucinations args((CHAR_DATA * ch));
void subtract_times args((struct timeval * etime, struct timeval * stime));
int get_distform args((int x, int y, int vx, int vy));

/*
 * Global Variables
 */

CHAR_DATA *gch_prev;
OBJ_DATA *gobj_prev;

CHAR_DATA *timechar;

int global_x[1];
int global_y[1];
int global_map[1];
int sprevhour;
int lastrescheck;
int timesincelast;
int start_next_round;
int snow_checks;
int mweather;
sh_int sav_rotation;

char *corpse_descs[] = {
   "The corpse of %s is in the last stages of decay.",
   "The corpse of %s is crawling with vermin.",
   "The corpse of %s fills the air with a foul stench.",
   "The corpse of %s is buzzing with flies.",
   "The corpse of %s lies here."
};

extern int top_exit;
extern void start_arena();
extern void do_game();
extern int in_start_arena;
extern int ppl_in_arena;
extern sh_int res_rotation; /* Used to see how many times the resource updater has been called */
extern int ppl_challenged;
extern int num_in_arena();

/*
 * Regeneration stuff.
 */
 
int hburn_gain(CHAR_DATA *ch)
{
   int iRegen = 0;
   
   if (!IS_NPC(ch))
   {
      int mod = 100;
       
      if (ch->pcdata->hit_cnt < 0)
         return 0;
          
      if (ch->position == POS_SITTING)
         mod = 150;
      if (ch->position == POS_RESTING)
         mod = 230;
      if (ch->position == POS_SLEEPING)
         mod = 400; 
            
      if (ch->race == RACE_ELF)
         mod = mod * 95/100;
      if (ch->race == RACE_HOBBIT)
         mod = mod * 85/100;
      if (ch->race == RACE_FAIRY)
         mod = mod * 75/100;
      if (ch->race == RACE_DWARF)
         mod = mod * 110/100;
      if (ch->race == RACE_OGRE)
         mod = mod * 125/100;
      
      if (IS_VAMPIRE(ch))
      {
         if (ch->pcdata->condition[COND_BLOODTHIRST] <= 1)
            mod /= 2;
         else if (ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level))
            mod *= 2;
         if (IS_OUTSIDE(ch))
         {
            switch (time_info.sunlight)
            {
               case SUN_RISE:
               case SUN_SET:
                  mod /= 2;
                  break;
               case SUN_LIGHT:
                  mod /= 4;
                  break;
            }
         }
      }
   
      if (ch->pcdata->condition[COND_FULL] == 0)
         mod /= 2;
      if (ch->pcdata->condition[COND_THIRST] == 0)
         mod /= 2;
         
      if (get_eq_char(ch, WEAR_LODGE_RIB) != NULL)
         mod /= 3;
      else if (get_eq_char(ch, WEAR_LODGE_LEG) != NULL)
         mod /= 2.5;
      else if (get_eq_char(ch, WEAR_LODGE_ARM) != NULL)
         mod /= 2;

      if (IS_AFFECTED(ch, AFF_POISON))
         mod /= 4;

      if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && (ch->on->value[3] > 0))
         mod = mod * ch->on->value[3] / 100;
         
      if (sysdata.resetgame)
         mod *= 3;
         
      mod = UMAX(1, ch->max_hit * mod/800);
         
      if ((ch->hburn_regen_counter += mod) >= SECONDS_PER_TICK)
      {
         iRegen = ch->hburn_regen_counter / SECONDS_PER_TICK;
         ch->hburn_regen_counter %= SECONDS_PER_TICK;
      }
      
      return iRegen;
   }
   return 0;
}

int mburn_gain(CHAR_DATA *ch)
{
   int iRegen = 0;
   
   if (!IS_NPC(ch))
   {
      int mod = 100;
       
      if (ch->pcdata->mana_cnt < 0)
         return 0;
          
      if (ch->position == POS_SITTING)
         mod = 150;
      if (ch->position == POS_RESTING)
         mod = 230;
      if (ch->position == POS_SLEEPING)
         mod = 400; 
            
      if (ch->race == RACE_ELF)
         mod = mod * 120/100;
      if (ch->race == RACE_HOBBIT)
         mod = mod * 90/100;
      if (ch->race == RACE_FAIRY)
         mod = mod * 150/100;
      if (ch->race == RACE_DWARF)
         mod = mod * 95/100;
      if (ch->race == RACE_OGRE)
         mod = mod * 30/100;
   
      if (ch->pcdata->condition[COND_FULL] == 0)
         mod /= 2;
      if (ch->pcdata->condition[COND_THIRST] == 0)
         mod /= 2;        

      if (IS_AFFECTED(ch, AFF_POISON))
         mod /= 4;

      if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && (ch->on->value[4] > 0))
         mod = mod * ch->on->value[4] / 100;
         
      if (sysdata.resetgame)
         mod *= 3;         
         
      mod = UMAX(1, number_range(2, 5) * mod/100);
         
      if ((ch->mburn_regen_counter += mod) >= SECONDS_PER_TICK)
      {
         iRegen = ch->mburn_regen_counter / SECONDS_PER_TICK;
         ch->mburn_regen_counter %= SECONDS_PER_TICK;
      }
      
      return iRegen;
   }
   return 0;
}        
 
int hit_gain(CHAR_DATA * ch)
{
   int gain;
   int iRegen;

   iRegen = 0;

   gain = UMAX(5, ch->max_hit/20);

   switch (ch->position)
   {
      case POS_DEAD:
         return 0;
      case POS_MORTAL:
         gain = -16;
      case POS_INCAP:
         gain = number_range(-16, 16);
      case POS_STUNNED:
        gain = 4;
      case POS_SLEEPING:
         gain += gain*4;
         break;
      case POS_RESTING:
         gain += gain*3;
         break;
   }
   //Quicker 
   if (ch->position == POS_INCAP || ch->position == POS_MORTAL || ch->position == POS_STUNNED)
   {
      if ((ch->hit_regen_counter += gain) >= SECONDS_PER_TICK)
      {
         iRegen = ch->hit_regen_counter / SECONDS_PER_TICK;
         ch->hit_regen_counter %= SECONDS_PER_TICK;
      }
      return UMIN(iRegen, ch->max_hit - ch->hit);
   }
   if (!IS_NPC(ch))
   {
      if (IS_VAMPIRE(ch))
      {
         if (ch->pcdata->condition[COND_BLOODTHIRST] <= 1)
            gain /= 2;
         else if (ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level))
            gain *= 2;
         if (IS_OUTSIDE(ch))
         {
            switch (time_info.sunlight)
            {
               case SUN_RISE:
               case SUN_SET:
                  gain /= 2;
                  break;
               case SUN_LIGHT:
                  gain /= 4;
                  break;
            }
         }
      }
   
      if (ch->pcdata->condition[COND_FULL] == 0)
         gain /= 2;
      if (ch->pcdata->condition[COND_THIRST] == 0)
         gain /= 2;
   }
   if (get_eq_char(ch, WEAR_LODGE_RIB) != NULL)
      gain /= 3;
   else if (get_eq_char(ch, WEAR_LODGE_LEG) != NULL)
      gain /= 2.5;
   else if (get_eq_char(ch, WEAR_LODGE_ARM) != NULL)
      gain /= 2;

   if (IS_AFFECTED(ch, AFF_POISON))
      gain /= 4;

   if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && (ch->on->value[3] > 0))
      gain = gain * ch->on->value[3] / 100;
      
   if (ch->hpgen)
      gain = gain * URANGE(40, ch->hpgen, 700) / 100;
      
   if (sysdata.resetgame)
      gain *= 3;
         
   if (ch->fighting)
      gain /= 3;

   if ((ch->hit_regen_counter += gain) >= SECONDS_PER_TICK)
   {
      iRegen = ch->hit_regen_counter / SECONDS_PER_TICK;
      ch->hit_regen_counter %= SECONDS_PER_TICK;
   }

   return UMIN(iRegen, ch->max_hit - ch->hit);

   //return UMIN(gain, ch->max_hit - ch->hit);
}



int mana_gain(CHAR_DATA * ch)
{
   int gain;
   int iRegen;

   iRegen = 0;

   gain = UMAX(5, ch->max_mana/15);

   if (ch->position < POS_SLEEPING)
      return 0;
   switch (ch->position)
   {
      case POS_SLEEPING:
         gain += gain * 7;
         break;
      case POS_RESTING:
         gain += gain * 4;
         break;
   }
   if (!IS_NPC(ch))
   {
      if (ch->pcdata->condition[COND_FULL] == 0)
         gain /= 2;
      if (ch->pcdata->condition[COND_THIRST] == 0)
         gain /= 2;
   }

   if (IS_AFFECTED(ch, AFF_POISON))
      gain /= 4;

   if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && (ch->on->value[4] > 0))
      gain = gain * ch->on->value[4] / 100;
   
   if (ch->managen)
      gain = gain * URANGE(40, ch->managen, 700) / 100;
      
   if (sysdata.resetgame)
      gain *= 3;
      
   if (ch->mana > ch->max_mana) //We are tapping...
      gain = 20;

   if ((ch->mana_regen_counter += gain) >= SECONDS_PER_TICK)
   {
      iRegen = ch->mana_regen_counter / SECONDS_PER_TICK;
      ch->mana_regen_counter %= SECONDS_PER_TICK;
   }
   
   if (ch->mana > ch->max_mana)
   {
      return UMAX(iRegen*-1, ch->max_mana - ch->mana);
   }
   else
   {
      return UMIN(iRegen, ch->max_mana - ch->mana);
   }

   //return UMIN(gain, ch->max_mana - ch->mana);
}



int move_gain(CHAR_DATA * ch)
{
   int gain;
   int iRegen;

   iRegen = 0;

   gain = number_range(150, 225);

   switch (ch->position)
   {
      case POS_DEAD:
         return 0;
      case POS_MORTAL:
         return -1;
      case POS_INCAP:
         return -1;
      case POS_STUNNED:
         return 1;
      case POS_SLEEPING:
         gain += gain * 5;
         break;
      case POS_RESTING:
         gain += gain * 3;
         break;
   }
   if (!IS_NPC(ch))
   {
      if (IS_VAMPIRE(ch))
      {
         if (ch->pcdata->condition[COND_BLOODTHIRST] <= 1)
            gain /= 2;
         else if (ch->pcdata->condition[COND_BLOODTHIRST] >= (8 + ch->level))
            gain *= 2;
         if (IS_OUTSIDE(ch))
         {
            switch (time_info.sunlight)
            {
               case SUN_RISE:
               case SUN_SET:
                  gain /= 2;
                  break;
               case SUN_LIGHT:
                  gain /= 4;
                  break;
            }
         }
      }

      if (ch->pcdata->condition[COND_FULL] == 0)
         gain /= 2;

      if (ch->pcdata->condition[COND_THIRST] == 0)
         gain /= 2;
   }

   if (IS_AFFECTED(ch, AFF_POISON))
      gain /= 4;

   if (ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && (ch->on->value[3] > 0))
      gain = gain * ch->on->value[3] / 100;
      
   if (ch->hpgen)
      gain = gain * URANGE(40, ch->hpgen, 700) / 100;
      
   if (sysdata.resetgame)
      gain *= 3;
      
   if ((ch->move_regen_counter += gain) >= SECONDS_PER_TICK)
   {
      iRegen = ch->move_regen_counter / SECONDS_PER_TICK;
      ch->move_regen_counter %= SECONDS_PER_TICK;
   }

   return UMIN(iRegen, ch->max_move - ch->move);

   //return UMIN(gain, ch->max_move - ch->move);
}


void gain_condition(CHAR_DATA * ch, int iCond, int value)
{
   int condition;
   ch_ret retcode = rNONE;

   if (value == 0 || IS_NPC(ch) || ch->level >= LEVEL_IMMORTAL || NOT_AUTHED(ch))
      return;

   condition = ch->pcdata->condition[iCond];
   if (iCond == COND_BLOODTHIRST)
      ch->pcdata->condition[iCond] = URANGE(0, condition + value, 10 + ch->level);
   else
      ch->pcdata->condition[iCond] = URANGE(0, condition + value, 48);

   if (ch->pcdata->condition[iCond] == 0)
   {
      switch (iCond)
      {
         case COND_FULL:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_HUNGRY, ch);
               send_to_char("You are STARVING!\n\r", ch);
               act(AT_HUNGRY, "$n is starved half to death!", ch, NULL, NULL, TO_ROOM);
               worsen_mental_state(ch, 1);
               retcode = damage(ch, ch, 1, TYPE_UNDEFINED, 0, -1);
            }
            break;

         case COND_THIRST:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_THIRSTY, ch);
               send_to_char("You are DYING of THIRST!\n\r", ch);
               act(AT_THIRSTY, "$n is dying of thirst!", ch, NULL, NULL, TO_ROOM);
               worsen_mental_state(ch, 2);
               retcode = damage(ch, ch, 2, TYPE_UNDEFINED, 0, -1);
            }
            break;

         case COND_BLOODTHIRST:
            if (ch->level < LEVEL_IMMORTAL)
            {
               set_char_color(AT_BLOOD, ch);
               send_to_char("You are starved to feast on blood!\n\r", ch);
               act(AT_BLOOD, "$n is suffering from lack of blood!", ch, NULL, NULL, TO_ROOM);
               worsen_mental_state(ch, 2);
               retcode = damage(ch, ch, ch->max_hit / 20, TYPE_UNDEFINED, 0, -1);
            }
            break;
         case COND_DRUNK:
            if (condition != 0)
            {
               set_char_color(AT_SOBER, ch);
               send_to_char("You are sober.\n\r", ch);
            }
            retcode = rNONE;
            break;
         default:
            bug("Gain_condition: invalid condition type %d", iCond);
            retcode = rNONE;
            break;
      }
   }

   if (retcode != rNONE)
      return;

   if (ch->pcdata->condition[iCond] == 1)
   {
      switch (iCond)
      {
         case COND_FULL:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_HUNGRY, ch);
               send_to_char("You are really hungry.\n\r", ch);
               act(AT_HUNGRY, "You can hear $n's stomach growling.", ch, NULL, NULL, TO_ROOM);
               if (number_bits(1) == 0)
                  worsen_mental_state(ch, 1);
            }
            break;

         case COND_THIRST:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_THIRSTY, ch);
               send_to_char("You are really thirsty.\n\r", ch);
               worsen_mental_state(ch, 1);
               act(AT_THIRSTY, "$n looks a little parched.", ch, NULL, NULL, TO_ROOM);
            }
            break;

         case COND_BLOODTHIRST:
            if (ch->level < LEVEL_IMMORTAL)
            {
               set_char_color(AT_BLOOD, ch);
               send_to_char("You have a growing need to feast on blood!\n\r", ch);
               act(AT_BLOOD, "$n gets a strange look in $s eyes...", ch, NULL, NULL, TO_ROOM);
               worsen_mental_state(ch, 1);
            }
            break;
         case COND_DRUNK:
            if (condition != 0)
            {
               set_char_color(AT_SOBER, ch);
               send_to_char("You are feeling a little less light headed.\n\r", ch);
            }
            break;
      }
   }


   if (ch->pcdata->condition[iCond] == 2)
   {
      switch (iCond)
      {
         case COND_FULL:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_HUNGRY, ch);
               send_to_char("You are hungry.\n\r", ch);
            }
            break;

         case COND_THIRST:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_THIRSTY, ch);
               send_to_char("You are thirsty.\n\r", ch);
            }
            break;

         case COND_BLOODTHIRST:
            if (ch->level < LEVEL_IMMORTAL)
            {
               set_char_color(AT_BLOOD, ch);
               send_to_char("You feel an urgent need for blood.\n\r", ch);
            }
            break;
      }
   }

   if (ch->pcdata->condition[iCond] == 3)
   {
      switch (iCond)
      {
         case COND_FULL:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_HUNGRY, ch);
               send_to_char("You are a mite peckish.\n\r", ch);
            }
            break;

         case COND_THIRST:
            if (ch->level < LEVEL_IMMORTAL && !IS_VAMPIRE(ch))
            {
               set_char_color(AT_THIRSTY, ch);
               send_to_char("You could use a sip of something refreshing.\n\r", ch);
            }
            break;

         case COND_BLOODTHIRST:
            if (ch->level < LEVEL_IMMORTAL)
            {
               set_char_color(AT_BLOOD, ch);
               send_to_char("You feel an aching in your fangs.\n\r", ch);
            }
            break;
      }
   }

   /*
    *  Race alignment restrictions, h
    */
   if (ch->alignment < race_table[ch->race]->minalign)
   {
      set_char_color(AT_BLOOD, ch);
      send_to_char("Your actions have been incompatible with the ideals of your race.  This troubles you.\n\r", ch);
   }

   if (ch->alignment > race_table[ch->race]->maxalign)
   {
      set_char_color(AT_BLOOD, ch);
      send_to_char("Your actions have been incompatible with the ideals of your race.  This troubles you.\n\r", ch);
   }

   return;
}
int get_dest_dir(int cx, int cy, int tx, int ty)
{
   if (tx == cx && ty < cy)
      return 0;
   if (tx > cx && ty == cy)
      return 1;
   if (tx == cx && ty > cy)
      return 2;
   if (tx < cx && ty == cy)
      return 3;
   if (tx < cx && ty < cy)
      return 7;
   if (tx > cx && ty < cy)
      return 6;
   if (tx > cx && ty > cy)
      return 8;
   if (tx < cx && ty > cy)
      return 9;

   return -1;
}

//Used to move around on the map, just supply start coordinates and target coordinates
void auto_move(CHAR_DATA * ch, int cx, int cy, int cmap, int tx, int ty, int tmap)
{
   sh_int dir;
   sh_int gox, goy;

   gox = goy = -1;
   if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
   {
      bug("auto_move: Trying to move a character that is not on the map.");
      return;
   }
   if (cx < 1 || cy < 1 || cmap < 0)
   {
      bug("auto_move: Character (starting) values are not valid.");
      return;
   }
   if (tx < 1 || ty < 1 || tmap < 0)
   {
      bug("auto_move: Target values are not valid.");
      return;
   }
   dir = get_dest_dir(cx, cy, tx, ty);

   if (sect_show[(int)map_sector[ch->map][gox][goy]].canpass == FALSE || map_sector[ch->map][gox][goy] == SECT_EXIT)
   {
      dir = alt_dir(ch, cx, cy, dir);
      gox = get_x(cx, dir);
      goy = get_y(cy, dir);
   }
   else
   {
      gox = get_x(cx, dir);
      goy = get_y(cy, dir);
   }
   ch->coord->x = gox;
   ch->coord->y = goy;
   return;
}

int go_extract(CHAR_DATA * ch)
{
   int holdx, holdy, holdmap, x, y, res, low, lowcurr;
   int hx;
   int rtype, sector;
   int startx, endx, starty, endy;
   OMAP_DATA *mobj;
   OBJ_DATA *obj;

   holdx = holdy = holdmap = -1;
   lowcurr = 100;

   if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
   {
      extract_char(ch, TRUE);
      return 1;
   }
   if (xIS_SET(ch->act, ACT_EXTRACTTOWN))
   {
      if (!ch->dumptown)
      {
         extract_char(ch, TRUE);
         return 1;
      }
   }
   if (ch->stx == -1 || ch->sty == -1 || ch->stmap == -1)
   {
      ch->stx = ch->coord->x;
      ch->sty = ch->coord->y;
      ch->stmap = ch->map;
      if (xIS_SET(ch->act, ACT_EXTRACTTOWN))
      {
         if ((ch->dumptown = find_town(ch->coord->x, ch->coord->y, ch->map)) == NULL)
         {
            extract_char(ch, TRUE);
            return 1;
         }
         if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->m4)
         {
            extract_char(ch, TRUE);
            return 1;
         }
      }         
      return 0;
   }
   //m1 - Resource AMT   m2 - Resource MAX   m3 - Cost   m4 - Kingdom   m5 - Resource Type   m6 - Extraction effic.
   if (ch->resx == -1 || ch->resy == -1 || ch->resmap == -1)
   {
      startx = ch->stx - 30;
      if (startx < 1)
         startx = 1;

      endx = ch->stx + 30;
      if (endx > MAX_X)
         endx = MAX_X;

      starty = ch->sty - 30;
      if (starty < 1)
         starty = 1;

      endy = ch->sty + 30;
      if (endx > MAX_Y)
         endy = MAX_Y;

      lowcurr = 100;
      for (x = startx; x <= endx; x++)
      {
         hx = x;
         for (y = starty; y <= endy; y++)
         {
            hx = x;
            res = get_resourcetype(map_sector[ch->map][x][y], 1);
            if (res != -1)
            {
               if (ch->m5 == res)
               {
                  if (resource_sector[ch->map][x][y] <= 0)
                     continue;
                  if (holdx == -1 || holdy == -1 || holdmap == -1)
                  {
                     holdx = hx;
                     holdy = y;
                     holdmap = ch->map;
                     lowcurr = abs(hx - ch->stx) + abs(y - ch->sty);
                  }
                  else
                  {
                     low = abs(hx - ch->stx) + abs(y - ch->sty);

                     if (low < lowcurr)
                     {
                        holdx = hx;
                        holdy = y;
                        holdmap = ch->map;
                        lowcurr = abs(hx - ch->stx) + abs(y - ch->sty);
                     }
                  }
               }
            }
         }
      }
      if (holdx != -1 && holdy != -1 && holdmap != -1)
      {
         ch->resx = holdx;
         ch->resy = holdy;
         ch->resmap = ch->map;
         xSET_BIT(ch->act, ACT_EXTRACTGOODS);
         return 0;
      }
      else
      {
         extract_char(ch, TRUE);
         return 1;
      }
   }
   holdx = holdy = holdmap = -1;
   if (ch->resx != -1 && ch->resy != -1 && ch->resmap != -1)
   {
      if (xIS_SET(ch->act, ACT_DUMPGOODS))
      {
         if (ch->coord->x == ch->resx && ch->coord->y == ch->resy && ch->map == ch->resmap)
         {
            if (xIS_SET(ch->act, ACT_EXTRACTTOWN))
            {      
               if ((get_current_hold(ch->dumptown) + ch->m1) > ch->dumptown->hold)
               {
                  extract_char(ch, TRUE);
                  return 1;
               }               
               if (ch->m5 == KRES_GOLD)
                  ch->dumptown->gold += ch->m1;
               if (ch->m5 == KRES_IRON)
                  ch->dumptown->iron += ch->m1;
               if (ch->m5 == KRES_CORN)
                  ch->dumptown->corn += ch->m1;
               if (ch->m5 == KRES_GRAIN)
                  ch->dumptown->grain += ch->m1;
               if (ch->m5 == KRES_LUMBER)
                  ch->dumptown->lumber += ch->m1;
               if (ch->m5 == KRES_STONE)
                  ch->dumptown->stone += ch->m1;
               if (ch->m5 == KRES_FISH)
                  ch->dumptown->fish += ch->m1;
               ch->m1 = 0;
               ch->pIndexData->m1 = 0;
               ch->resx = -1;
               ch->resy = -1;
               ch->resmap = -1;
               write_kingdom_file(ch->dumptown->kingdom);
               xREMOVE_BIT(ch->act, ACT_DUMPGOODS);
               return 0;
            }
            else
            {
               for (mobj = first_wilderobj; mobj; mobj = mobj->next)
               {
                  obj = mobj->mapobj;

                  if (obj->item_type == ITEM_HOLDRESOURCE && obj->coord->x == ch->coord->x && obj->coord->y == ch->coord->y && obj->map == ch->map)
                  {
                     if (obj->value[1] == ch->m5 || obj->value[3] == ch->m5)
                     {
                        if (obj->value[1] == ch->m5)
                        {
                           if ((obj->value[2] + ch->m1) > obj->value[0])
                           {
                              obj->value[2] = obj->value[0];
                           }
                           else
                           {
                              obj->value[2] += ch->m1;
                           }
                           ch->m1 = 0;
                           ch->pIndexData->m1 = 0;
                           ch->resx = -1;
                           ch->resy = -1;
                           ch->resmap = -1;
                           xREMOVE_BIT(ch->act, ACT_DUMPGOODS);
                           save_bin_data();
                           return 0;
                        }
                        else
                        {
                           if ((obj->value[4] + ch->m1) > obj->value[0])
                           {
                              obj->value[4] = obj->value[0];
                           }
                           else
                           {
                              obj->value[4] += ch->m1;
                           }
                           ch->m1 = 0;
                           ch->pIndexData->m1 = 0;
                           ch->resx = -1;
                           ch->resy = -1;
                           ch->resmap = -1;
                           xREMOVE_BIT(ch->act, ACT_DUMPGOODS);
                           save_bin_data();
                           return 0;
                        }
                     }
                  }
               }
            }
         }
         else
         {
            auto_move(ch, ch->coord->x, ch->coord->y, ch->map, ch->resx, ch->resy, ch->resmap);
            return 0;
         }
      }
      if (get_resourcetype(map_sector[ch->map][ch->resx][ch->resy], 0) == ch->m5)
      {
         if (resource_sector[ch->map][ch->resx][ch->resy] <= 0)
         {
            ch->resx = -1;
            ch->resy = -1;
            ch->resmap = -1;
            xREMOVE_BIT(ch->act, ACT_EXTRACTGOODS);
            return 0;
         }
         if (ch->resx == ch->coord->x && ch->resy == ch->coord->y && ch->resmap == ch->map)
         {
            if (ch->m1 < ch->m2)
            {
               rtype = ch->m5;
               if (rtype == KRES_FISH)
                  ch->m1 += ch->m6/2;
               else
                  ch->m1 += ch->m6;
               if (rtype == KRES_FISH)
                  resource_sector[ch->map][ch->coord->x][ch->coord->y] -= ch->m6/2;
               else
                  resource_sector[ch->map][ch->coord->x][ch->coord->y] -= ch->m6;
               //update the sector if it changes
               sector = map_sector[ch->map][ch->coord->x][ch->coord->y];               
               
               if (rtype == KRES_CORN || rtype == KRES_GRAIN || rtype == KRES_LUMBER || rtype == KRES_STONE)
               {
                  if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 2400)
                  {
                     if (rtype == KRES_CORN)
                        sector = SECT_SCORN;
                     if (rtype == KRES_GRAIN)
                        sector = SECT_SGRAIN;
                     if (rtype == KRES_LUMBER)
                        sector = SECT_STREE;
                     if (rtype == KRES_STONE)
                        sector = SECT_SSTONE;
                  }
                  if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 1)
                  {
                     if (rtype == KRES_CORN)
                        sector = SECT_NCORN;
                     if (rtype == KRES_GRAIN)
                        sector = SECT_NGRAIN;
                     if (rtype == KRES_LUMBER)
                        sector = SECT_NTREE;
                     if (rtype == KRES_STONE)
                        sector = SECT_NSTONE;
                  }
               }
               if (rtype == KRES_IRON || rtype == KRES_GOLD)
               {
                  if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 1200)
                  {
                     if (rtype == KRES_IRON)
                        sector = SECT_SGOLD;
                     if (rtype == KRES_GOLD)
                        sector = SECT_SIRON;
                  }
                  if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 1)
                  {
                     if (rtype == KRES_IRON)
                        sector = SECT_NGOLD;
                     if (rtype == KRES_GOLD)
                        sector = SECT_SIRON;
                  }
               }
               map_sector[ch->map][ch->coord->x][ch->coord->y] = sector;

               if (ch->m1 > ch->m2)
                  ch->m1 = ch->m2;
            }
            else
            {
               xREMOVE_BIT(ch->act, ACT_EXTRACTGOODS);
               xSET_BIT(ch->act, ACT_DUMPGOODS);
               if (xIS_SET(ch->act, ACT_EXTRACTTOWN))
               {
                  ch->resx = ch->stx;
                  ch->resy = ch->sty;
                  ch->resmap = ch->stmap;
                  return 0;
               }
               else
               {
                  lowcurr = 100;
                  for (mobj = first_wilderobj; mobj; mobj = mobj->next)
                  {
                     obj = mobj->mapobj;
                     if (obj->item_type == ITEM_HOLDRESOURCE && (obj->value[1] == ch->m5 || obj->value[3] == ch->m5))
                     {
                        if (obj->value[1] == ch->m5)
                        {
                           if ((obj->value[2] + ch->m1) > obj->value[0])
                              continue;
                        }
                        else
                        {
                           if ((obj->value[4] + ch->m1) > obj->value[0])
                              continue;
                        }
                     }
                     else
                        continue;

                     if (abs(obj->coord->x - ch->stx) > 30 || abs(obj->coord->y - ch->sty) > 30)
                        continue;
                     else
                     {
                        if (holdx == -1 || holdy == -1 || holdmap == -1)
                        {
                           holdx = obj->coord->x;
                           holdy = obj->coord->y;
                           holdmap = obj->map;
                           lowcurr = abs(obj->coord->x - ch->stx) + abs(obj->coord->y - ch->sty);
                        }
                        else
                        {
                           low = abs(obj->coord->x - ch->stx) + abs(obj->coord->y - ch->sty);
                           
                           if (low < lowcurr)
                           {
                              holdx = obj->coord->x;
                              holdy = obj->coord->y;
                              holdmap = obj->map;
                              lowcurr = low;
                           }
                        }
                     }
                  }
                  if (holdx != -1 && holdy != -1 && holdmap != -1)
                  {
                     ch->resx = holdx;
                     ch->resy = holdy;
                     ch->resmap = ch->map;
                     return 0;
                  }
                  else
                  {
                     extract_char(ch, TRUE);
                     return 1;
                  }
               }
            }
         }
         else
         {
            auto_move(ch, ch->coord->x, ch->coord->y, ch->map, ch->resx, ch->resy, ch->resmap);
            return 0;
         }
      }
   }
   return 0;
}

//runs every 4 seconds (mobile tick).  Compared to 60 seconds this is faster :-)
void obj_speed_update(void)
{
   OBJ_DATA *obj;
   TIMER *timer, *timer_next;
   char buf[MSL];
   
   for (obj = last_object; obj; obj = gobj_prev)
   {
      CHAR_DATA *rch;
      if (obj == first_object && obj->prev)
      {
         bug("obj_update: first_object->prev != NULL... fixed", 0);
         obj->prev = NULL;
      }
      gobj_prev = obj->prev;
      if (gobj_prev && gobj_prev->next != obj)
      {
         bug("obj_update: obj->prev->next != obj", 0);
         return;
      }
      set_cur_obj(obj);

      if (obj_extracted(obj))
         continue;
         
      /* Forge Update */

      for( timer = obj->first_timer; timer; timer = timer_next)
      {
          timer_next = timer->next;
          -- timer->count;
          if( timer->count == 0 )
          {
              if( get_obj_timer( obj, TIMER_COOLING ) == 0 )
              {
                  if (obj->in_room)
                  {
                     sprintf(buf, "%s has cooled down enough to touch.", obj->short_descr);
                     rch = obj->in_room->first_person;
                     act(AT_PLAIN, buf, rch, obj, NULL, TO_ROOM);
                     act(AT_PLAIN, buf, rch, obj, NULL, TO_CHAR);
                  }
                  SET_BIT(obj->wear_flags, ITEM_TAKE);
                  remove_obj_timer(obj, TIMER_COOLING);
              }
          }
      }
      //done for now
   }
}

void set_command_buf args((CHAR_DATA * ch, char *argument));

void command_mob(char *c, int num, char *movb, CHAR_DATA * victim)
{
   int x;

   for (x = 1; x <= num; x++)
   {
      if (movb[0] != '\0')
      {
         if (!str_cmp(movb, "nw"))
         {
            do_northwest(victim, "");
         }
         if (!str_cmp(movb, "ne"))
         {
            do_northeast(victim, "");
         }
         if (!str_cmp(movb, "sw"))
         {
            do_southwest(victim, "");
         }
         if (!str_cmp(movb, "se"))
         {
            do_southeast(victim, "");
         }
      }
      else
      {
         if (*c == 'n')
            do_north(victim, "");
         if (*c == 'e')
            do_east(victim, "");
         if (*c == 'w')
            do_west(victim, "");
         if (*c == 's')
            do_south(victim, "");
         if (*c == 'u')
            do_up(victim, "");
         if (*c == 'd')
            do_down(victim, "");
      }
   }
}

//Makes the mobile move
void move_as_command(CHAR_DATA * victim, char *dir)
{
   char *d;
   char movb[3];
   char numb[5];
   int num = 0;
   int fnd = 0;

   strcpy(movb, "");

   for (d = dir; *d != '\0'; ++d)
   {
      if (fnd == 1)
      {
         set_command_buf(victim, d);
         return;
      }
      if (!isdigit(*d)) //direction
      {
         if (*(d + 1) == '+')
         {
            sprintf(movb, "%c%c", *d, *(d + 2));
         }
         if (*d == '+')
            continue;
         if (*(d - 1) == '+')
            continue;
         if (d == dir || (!isdigit(*(d - 1)) && *(d - 1) != '+'))
         {
            command_mob(d, 1, movb, victim);
            if (victim->mount)
               command_mob(d, 1, movb, victim->mount);
            fnd = 1;
            continue;
         }
         if (isdigit(*(d - 1)))
         {
            if (isdigit(*(d - 2)))
               num = atoi(&(*(d - 2)));
            else
               num += atoi(&(*(d - 1)));

            sprintf(numb, "%d", num - 1);
            strcat(numb, d);
            command_mob(d, 1, movb, victim);
            if (victim->mount)
               command_mob(d, 1, movb, victim->mount);
            if (num == 1)
            {
               fnd = 1;
               continue;
            }
            set_command_buf(victim, numb);
            return;
         }
      }
   }
   set_command_buf(victim, "");
   return;
}

//Runs every battle tick(1/2 second), use to keep track of speedier functions, only place them here if you
//need the speed, else just use mobile_update
void char_speed_update(void)
{
   CHAR_DATA *ch;
   char buf[MSL];
   
   /* Examine all mobs. */
   for (ch = last_char; ch; ch = gch_prev)
   {
      set_cur_char(ch);
      if (ch == first_char && ch->prev)
      {
         bug("mobile_update: first_char->prev != NULL... fixed", 0);
         ch->prev = NULL;
      }

      gch_prev = ch->prev;

      if (gch_prev && gch_prev->next != ch)
      {
         sprintf(buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.", ch->name);
         bug(buf, 0);
         bug("Short-cutting here", 0);
         gch_prev = NULL;
         ch->prev = NULL;
         do_shout(ch, "Thoric says, 'Prepare for the worst!'");
      }
      if (!char_died(ch))
      {
         OBJ_DATA *arrow = NULL;
         int dam = 0;

         ch->cmd_recurse = 0;
         if ((arrow = get_eq_char(ch, WEAR_LODGE_RIB)) != NULL && number_range(1, 20) == 1) // 10 sec
         {
            dam = number_range((2 * arrow->value[1]), (2 * arrow->value[2]));
            act(AT_CARNAGE, "$n suffers damage from $p stuck in $s rib.", ch, arrow, NULL, TO_ROOM);
            act(AT_CARNAGE, "You suffer damage from $p stuck in your rib.", ch, arrow, NULL, TO_CHAR);
            damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
         }
         if ((arrow = get_eq_char(ch, WEAR_LODGE_LEG)) != NULL && number_range(1, 30) == 1) // 15 sec
         {
            dam = number_range((2 * arrow->value[1]), arrow->value[2]);
            act(AT_CARNAGE, "$n suffers damage from $p stuck in $s leg.", ch, arrow, NULL, TO_ROOM);
            act(AT_CARNAGE, "You suffer damage from $p stuck in your leg.", ch, arrow, NULL, TO_CHAR);
            damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
         }
         if ((arrow = get_eq_char(ch, WEAR_LODGE_ARM)) != NULL && number_range(1, 30) == 1) // 15 sec
         {
            dam = number_range(arrow->value[1], arrow->value[2]);
            act(AT_CARNAGE, "$n suffers damage from $p stuck in $s arm.", ch, arrow, NULL, TO_ROOM);
            act(AT_CARNAGE, "You suffer damage from $p stuck in your arm.", ch, arrow, NULL, TO_CHAR);
            damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
         }

         if (char_died(ch))
            continue;
      }
      if (!char_died(ch) && (get_eq_char(ch, WEAR_LODGE_RIB) || get_eq_char(ch, WEAR_LODGE_LEG) || get_eq_char(ch, WEAR_LODGE_ARM)))
      {
         int mod = 0;
         
         if (!IS_NPC(ch)) //encourage players to remove them their selves
            mod = 18;
            
         if (number_range(1, 1000) <= 20-mod) // 50 sec, 500 sec(player)
         {
            //Don't want them to just bleed to death, try to pull it out.
            do_dislodge(ch, "");

            if (char_died(ch))
               continue;
         }
      }
      if (!xIS_SET(ch->act, ACT_RUNNING) && !xIS_SET(ch->act, ACT_SENTINEL) && !ch->fighting && ch->hunting)
      {
         /* Commented out temporarily to avoid spam - Scryn
            sprintf( buf, "%s hunting %s from %s.", ch->name,
            h->hunting->name,
            ch->in_room->name );
            log_string( buf ); */
         if (ch->fight_timer < 1)
         {
            if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
               hunt_victim_map(ch);
            else
               hunt_victim(ch);
         }
         continue;
      }
      //Create the mi info on the mob if it isn't already there
      if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
      {
         if (!ch->midata)
         {
            CREATE(ch->midata, MI_DATA, 1);
            ch->midata->x = ch->coord->x;
            ch->midata->y = ch->coord->y;
            ch->midata->map = ch->map;
            ch->midata->in_room = ch->in_room;
            ch->midata->command = STRALLOC("");
         }
      }
      if (ch->midata && ch->midata->command[0] != '\0')
      {
         int fspeed;
         if (ch->midata->mspeed)
            fspeed = ch->midata->mspeed;
         else
            fspeed = ch->m10;
            
         ch->midata->mtick++;
         if (ch->midata->mtick >= fspeed)
         {
            move_as_command(ch, ch->midata->command);
            ch->midata->mtick = 0;
         }
         continue; //Don't want to patrol if we are moving...
      }
      //Patrol
      if (xIS_SET(ch->act, ACT_MILITARY) && xIS_SET(ch->miflags, KM_PATROL) && number_range(1,3) == 1)
      {
         int stx, sty, endx, endy;
         int dir = -1;

         stx = UMAX(ch->m7 - ch->m2, 1);
         sty = UMAX(ch->m8 - ch->m2, 1);
         endx = UMIN(ch->m7 + ch->m2, MAX_X);
         endy = UMIN(ch->m8 + ch->m2, MAX_Y);

         if (ch->coord->x != stx && ch->coord->y != sty && ch->coord->x != endx && ch->coord->y != endy)
         {
            dir = 3;
         }
         else if (ch->coord->x == stx)
         {
            if (ch->coord->y == sty)
               dir = 1;
            else
               dir = 0;
         }
         else if (ch->coord->y == sty)
         {
            if (ch->coord->x == endx)
               dir = 2;
            else
               dir = 1;
         }
         else if (ch->coord->x == endx)
         {
            if (ch->coord->y == endy)
               dir = 3;
            else
               dir = 2;
         }
         else if (ch->coord->y == endy)
         {
            if (ch->coord->x == stx)
               dir = 0;
            else
               dir = 3;
         }
         if (dir == 0)
         {
            if (!sect_show[(int)map_sector[ch->map][ch->coord->x][ch->coord->y - 1]].canpass
               || map_sector[ch->map][ch->coord->x][ch->coord->y - 1] == SECT_ENTER)
            {
               xREMOVE_BIT(ch->miflags, KM_PATROL);
               xSET_BIT(ch->miflags, KM_STATIONARY);
            }
            else
               do_north(ch, "");
         }
         if (dir == 1)
         {
            if (!sect_show[(int)map_sector[ch->map][ch->coord->x + 1][ch->coord->y]].canpass
               || map_sector[ch->map][ch->coord->x + 1][ch->coord->y] == SECT_ENTER)
            {
               xREMOVE_BIT(ch->miflags, KM_PATROL);
               xSET_BIT(ch->miflags, KM_STATIONARY);
            }
            else
               do_east(ch, "");
         }
         if (dir == 2)
         {
            if (!sect_show[(int)map_sector[ch->map][ch->coord->x][ch->coord->y + 1]].canpass
               || map_sector[ch->map][ch->coord->x][ch->coord->y + 1] == SECT_ENTER)
            {
               xREMOVE_BIT(ch->miflags, KM_PATROL);
               xSET_BIT(ch->miflags, KM_STATIONARY);
            }
            else
               do_south(ch, "");
         }
         if (dir == 3)
         {
            if (!sect_show[(int)map_sector[ch->map][ch->coord->x - 1][ch->coord->y]].canpass
               || map_sector[ch->map][ch->coord->x - 1][ch->coord->y] == SECT_ENTER)
            {
               xREMOVE_BIT(ch->miflags, KM_PATROL);
               xSET_BIT(ch->miflags, KM_STATIONARY);
            }
            else
               do_west(ch, "");
         }
         if (dir == -1)
            bug("%s has -1 dir", ch->name);
      }
   }
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update(void)
{
   char buf[MSL];
   CHAR_DATA *ch;
   EXIT_DATA *pexit;
   int door;
   BUYKMOB_DATA *kmob;
   int ext;
   ch_ret retcode;

   retcode = rNONE;

   /* Examine all mobs. */
   for (ch = last_char; ch; ch = gch_prev)
   {
      set_cur_char(ch);
      if (ch == first_char && ch->prev)
      {
         bug("mobile_update: first_char->prev != NULL... fixed", 0);
         ch->prev = NULL;
      }

      gch_prev = ch->prev;

      if (gch_prev && gch_prev->next != ch)
      {
         sprintf(buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.", ch->name);
         bug(buf, 0);
         bug("Short-cutting here", 0);
         gch_prev = NULL;
         ch->prev = NULL;
         do_shout(ch, "Thoric says, 'Prepare for the worst!'");
      }

      if (ch->mount)
      {
         if (ch->mount->move < 300 && ch->mount->move >= 250)
         {
            if (number_range(1, 4) == 1)
            {
               send_to_char("&G&WYour MOUNT's breathing is starting to pick up.\n\r", ch);
            }
         }
         else if (ch->mount->move < 250 && ch->mount->move >= 200)
         {
            if (number_range(1, 3) == 1)
            {
               send_to_char("&G&WYour MOUNT's breathing is increasing...\n\r", ch);
            }
         }
         else if (ch->mount->move < 200 && ch->mount->move >= 150)
         {
            if (number_range(1, 3) == 1)
            {
               send_to_char("&cYour MOUNT is starting to breath heavily...\n\r", ch);
            }
         }
         else if (ch->mount->move < 150 && ch->mount->move >= 125)
         {
            if (number_range(1, 2) == 1)
            {
               send_to_char("&RYour MOUNT is having problems keeping a breath.\n\r", ch);
            }
         }
         else if (ch->mount->move < 125 && ch->mount->move >= 70)
         {
            send_to_char("&rYour MOUNT looks like it is about to collapse.\n\r", ch);
         }
         else if (ch->mount->move < 70)
         {
            send_to_char("&rYour MOUNT collapses right here and falls to the ground stunned.\n\r", ch);
            if (!IS_AFFECTED(ch->mount, AFF_PARALYSIS))
            {
               AFFECT_DATA af;
               af.location = APPLY_ARMOR;
               af.modifier = -1;
               af.type = gsn_stun;
               af.duration = 10;
               af.bitvector = meb(AFF_PARALYSIS);
               affect_to_char(ch->mount, &af);
               update_pos(ch->mount);
               ch->mount->move = 70;
               xREMOVE_BIT(ch->mount->act, ACT_MOUNTED);
               ch->position = POS_STANDING;
               ch->mount = NULL;
            }
         }
      }
      if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY) && ch->m4 > 1 && ch->m4 < sysdata.max_kingdom)
      { 
         int csn;
         int scnt;
         for (kmob = first_buykmob; kmob; kmob = kmob->next)
         {
            if (ch->pIndexData->vnum == kmob->vnum)
            {
               if (xIS_SET(kmob->flags, KMOB_MAGE) && !ch->fighting && number_range(1, 3) == 1)
               {
                  CHAR_DATA *rch;
                  int num;
                  if (xIS_SET(kmob->flags, KMOB_SHIELD) || xIS_SET(kmob->flags, KMOB_KINDRED)
                  || xIS_SET(kmob->flags, KMOB_SLINK) || xIS_SET(kmob->flags, KMOB_FIRESHIELD) 
                  || xIS_SET(kmob->flags, KMOB_ICESHIELD) || xIS_SET(kmob->flags, KMOB_SHOCKSHIELD)
                  || xIS_SET(kmob->flags, KMOB_ANTIMAGICSHELL))
                  {
                     for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
                     {
                        if (in_same_room(ch, rch) && IS_NPC(rch) && rch->m4 == ch->m4)
                        { 
                           for (scnt = 0; scnt < 40; scnt++)
                           {
                              num = number_range(1, 7);  
                              if (num == 1 && xIS_SET(kmob->flags, KMOB_SHIELD))
                              {
                                 csn = skill_lookup("shield");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts shield on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 2 && xIS_SET(kmob->flags, KMOB_KINDRED))
                              {
                                 csn = skill_lookup("kindred strength");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts kindred strength on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 3 && xIS_SET(kmob->flags, KMOB_SLINK))
                              {
                                 csn = skill_lookup("slink");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts slink on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 4 && xIS_SET(kmob->flags, KMOB_FIRESHIELD))
                              {
                                 csn = skill_lookup("fireshield");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts fireshield on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 5 && xIS_SET(kmob->flags, KMOB_ICESHIELD))
                              {
                                 csn = skill_lookup("iceshield");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts iceshield on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 6 && xIS_SET(kmob->flags, KMOB_SHOCKSHIELD))
                              {
                                 csn = skill_lookup("shockshield");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts shockshield on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 7 && xIS_SET(kmob->flags, KMOB_ANTIMAGICSHELL))
                              {
                                 csn = skill_lookup("antimagic shell");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts antimagic shell on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                           }
                           if (scnt < 40)
                              break;
                        }
                     }
                  }    
               } 
               if (xIS_SET(kmob->flags, KMOB_CLERIC) && !ch->fighting && number_range(1, 3) == 1)
               {
                  CHAR_DATA *rch;
                  int num = number_range(1, 20);
                  if (num > 13 && (xIS_SET(kmob->flags, KMOB_BLESS) || xIS_SET(kmob->flags, KMOB_SANCTIFY) 
                  || xIS_SET(kmob->flags, KMOB_FLEETARMS) || xIS_SET(kmob->flags, KMOB_SANCTUARY)))
                  {
                     for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
                     {
                        if (in_same_room(ch, rch) && IS_NPC(rch) && rch->m4 == ch->m4)
                        { 
                           for (scnt = 0; scnt < 40; scnt++)
                           {
                              num = number_range(1, 4);  
                              if (num == 1 && xIS_SET(kmob->flags, KMOB_BLESS))
                              {
                                 csn = skill_lookup("bless");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts bless on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 2 && xIS_SET(kmob->flags, KMOB_SANCTIFY))
                              {
                                 csn = skill_lookup("sanctify");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts sanctify on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 3 && xIS_SET(kmob->flags, KMOB_FLEETARMS))
                              {
                                 csn = skill_lookup("fleetarms");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts fleearms on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 4 && xIS_SET(kmob->flags, KMOB_SANCTUARY))
                              {
                                 csn = skill_lookup("sanctuary");
                                 if (is_affected(rch, csn))
                                    continue;
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts sanctuary on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                           }
                           if (scnt < 40)
                              break;
                        }
                     }
                  }                              
                  if (num <= 13 && (xIS_SET(kmob->flags, KMOB_CURELIGHT) || xIS_SET(kmob->flags, KMOB_CURESERIOUS)
                  || xIS_SET(kmob->flags, KMOB_CURECRITICAL) || xIS_SET(kmob->flags, KMOB_HEAL) 
                  || xIS_SET(kmob->flags, KMOB_DIVINITY) || xIS_SET(kmob->flags, KMOB_POWERHEAL)))
                  {
                     for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
                     {
                        if (in_same_room(ch, rch) && IS_NPC(rch) && rch->m4 == ch->m4 && rch->hit < rch->max_hit)
                        { 
                           for (;;)
                           {
                              num = number_range(1, 6);
                              
                              if (num == 1 && xIS_SET(kmob->flags, KMOB_CURELIGHT))
                              {
                                 csn = skill_lookup("cure light");
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts cure light on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 2 && xIS_SET(kmob->flags, KMOB_CURESERIOUS))
                              {
                                 csn = skill_lookup("cure serious");
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts cure serious on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 3 && xIS_SET(kmob->flags, KMOB_CURECRITICAL))
                              {
                                 csn = skill_lookup("cure critical");
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts cure critical on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 4 && xIS_SET(kmob->flags, KMOB_HEAL))
                              {
                                 csn = skill_lookup("heal");
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts heal on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 5 && xIS_SET(kmob->flags, KMOB_DIVINITY))
                              {
                                 csn = skill_lookup("divinity");
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts divinity on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                              if (num == 6 && xIS_SET(kmob->flags, KMOB_POWERHEAL))
                              {
                                 csn = skill_lookup("power heal");
                                 if (csn <= 0)
                                    continue;
                                 spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, rch);
                                 act(AT_ACTION, "$n waves $s hands and casts power heal on $N.", ch, NULL, rch, TO_ROOM);
                                 break;
                              }
                           }
                           break;
                        }
                     }
                  }                        
               }
               break;
            }
         }
      }
      /*
       * Recurring spell affect
       */
      if (IS_AFFECTED(ch, AFF_RECURRINGSPELL))
      {
         AFFECT_DATA *paf, *paf_next;
         SKILLTYPE *skill;
         bool found = FALSE, died = FALSE;

         for (paf = ch->first_affect; paf; paf = paf_next)
         {
            paf_next = paf->next;
            if (paf->location == APPLY_RECURRINGSPELL)
            {
               found = TRUE;
               if (IS_VALID_SN(paf->modifier) && (skill = skill_table[paf->modifier]) != NULL && skill->type == SKILL_SPELL)
               {
                  if ((*skill->spell_fun) (paf->modifier, ch->level, ch, ch) == rCHAR_DIED || char_died(ch))
                  {
                     died = TRUE;
                     break;
                  }
               }
            }
         }
         if (died)
            continue;
         if (!found)
            xREMOVE_BIT(ch->affected_by, AFF_RECURRINGSPELL);
      }
      if (!IS_NPC(ch))
      {
         drunk_randoms(ch);
         hallucinations(ch);
         if (ch->move < 300 && ch->move >= 250)
         {
            if (number_range(1, 4) == 1)
            {
               send_to_char("&G&WYour breathing is starting to pick up, you need to rest.\n\r", ch);
            }
         }
         else if (ch->move < 250 && ch->move >= 200)
         {
            if (number_range(1, 3) == 1)
            {
               send_to_char("&G&WYour breathing is increasing...you need to rest.\n\r", ch);
            }
         }
         else if (ch->move < 200 && ch->move >= 150)
         {
            if (number_range(1, 3) == 1)
            {
               send_to_char("&cYour are starting to breath heavily...you need to rest.\n\r", ch);
            }
         }
         else if (ch->move < 150 && ch->move >= 125)
         {
            if (number_range(1, 2) == 1)
            {
               send_to_char("&RYou are having problems keeping a breath...you need to rest.\n\r", ch);
            }
         }
         else if (ch->move < 125 && ch->move > 70)
         {
            send_to_char("&rYou feel like you are about to collapse....you need to rest\n\r", ch);
         }
         else if (ch->move < 70)
         {
            send_to_char("&rYou collapse right here and fall to the ground stunned.\n\r", ch);
            if (!IS_AFFECTED(ch, AFF_PARALYSIS))
            {
               AFFECT_DATA af;
               af.location = APPLY_ARMOR;
               af.modifier = -1;
               af.type = gsn_stun;
               af.duration = 10;
               af.bitvector = meb(AFF_PARALYSIS);
               affect_to_char(ch, &af);
               update_pos(ch);
               ch->move = 70;
            }
         }
         continue;
      }

      if (!ch->in_room || IS_AFFECTED(ch, AFF_CHARM) || IS_AFFECTED(ch, AFF_PARALYSIS))
         continue;
         
      if (IS_AFFECTED(ch, AFF_WEB) || IS_AFFECTED(ch, AFF_SNARE))
      {
         int bdam;
         bdam = get_curr_str(ch)*3 + get_curr_int(ch)*2;
         if (IS_AFFECTED(ch, AFF_SNARE))
            bdam /=2;
         if (!IS_NPC(ch))
            bdam*=3;
         if (number_range(1, 1000) <= bdam)
         {
            act(AT_WHITE, "$n squirms and manages to break out of $s entanglement.", ch, NULL, NULL, TO_NOTVICT);
            act(AT_WHITE, "You squirm and manage to break out of your entanglement.", ch, NULL, NULL, TO_CHAR);
            affect_strip(ch, gsn_web);
            affect_strip(ch, gsn_snare);
         }
      }      
         
      if (!xIS_SET(ch->act, ACT_RUNNING) && !xIS_SET(ch->act, ACT_SENTINEL) && !ch->fighting && ch->hunting)
         continue;

/* Clean up 'animated corpses' that are not charmed' - Scryn */

      if (ch->pIndexData->vnum == 5 && !IS_AFFECTED(ch, AFF_CHARM))
      {
         if (ch->in_room->first_person)
            act(AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM);

         if (IS_NPC(ch)) /* Guard against purging switched? */
            extract_char(ch, TRUE);
         continue;
      }

      if (xIS_SET(ch->act, ACT_EXTRACTMOB))
      {
         ext = go_extract(ch);
         // Note it is possible to extract character, so check to see if is NULL -- Xerves
         if (ext == 1)
            continue;
      }

      /* Examine call for special procedure */
      if (!xIS_SET(ch->act, ACT_RUNNING) && ch->spec_fun)
      {
         if ((*ch->spec_fun) (ch))
            continue;
         if (char_died(ch))
            continue;
      }

      /* Check for mudprogram script on mob */
      if (HAS_PROG(ch->pIndexData, SCRIPT_PROG))
      {
         mprog_script_trigger(ch);
         continue;
      }

      if (ch != cur_char)
      {
         bug("Mobile_update: ch != cur_char after spec_fun", 0);
         continue;
      }

      /* That's all for sleeping / busy monster */
      if (ch->position != POS_STANDING)
         continue;

      if (xIS_SET(ch->act, ACT_MOUNTED))
      {
         if (xIS_SET(ch->act, ACT_AGGRESSIVE) || xIS_SET(ch->act, ACT_META_AGGR))
            do_emote(ch, "snarls and growls.");
         continue;
      }

      if (is_room_safe(ch) && (xIS_SET(ch->act, ACT_AGGRESSIVE) || xIS_SET(ch->act, ACT_META_AGGR)))
         do_emote(ch, "glares around and snarls.");

      mprog_random_trigger(ch);
      if (char_died(ch))
         continue;
      if (ch->position < POS_STANDING)
         continue;

      /* MOBprogram hour trigger: do something for an hour */
      mprog_hour_trigger(ch);
      

      if (char_died(ch))
         continue;

      rprog_hour_trigger(ch);
      if (char_died(ch))
         continue;

      if (ch->position < POS_STANDING)
         continue;

      /* Scavenge */
      if (xIS_SET(ch->act, ACT_SCAVENGER) && ch->in_room->first_content && number_bits(2) == 0)
      {
         OBJ_DATA *obj;
         OBJ_DATA *obj_best;
         int max;

         max = 1;
         obj_best = NULL;
         for (obj = ch->in_room->first_content; obj; obj = obj->next_content)
         {
            if (IS_ONMAP_FLAG(ch) && IS_OBJ_STAT(obj, ITEM_ONMAP))
            {
               if (ch->map != obj->map
                  || ch->coord->x != obj->coord->x)
                  continue;
            }
            if (CAN_WEAR(obj, ITEM_TAKE) && obj->cost > max && !IS_OBJ_STAT(obj, ITEM_BURIED))
            {
               obj_best = obj;
               max = obj->cost;
            }
            if (obj->pIndexData->vnum > 9022 && obj->pIndexData->vnum < 9026)
            {
               obj_best = NULL;
               max = 1;
            }
         }

         if (obj_best)
         {
            obj_from_room(obj_best);
            obj_to_char(obj_best, ch);
            act(AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM);
         }
      }
      if (IS_ONMAP_FLAG(ch))
      {
         if ((door = number_bits(5)) <= 9
            && door != 4
            && door != 5
            && !ch->fighting
            && !xIS_SET(ch->act, ACT_SENTINEL)
            && !xIS_SET(ch->act, ACT_PROTOTYPE)
            && !xIS_SET(ch->act, ACT_MILITARY) && !xIS_SET(ch->act, ACT_MOUNTSAVE) && !xIS_SET(ch->act, ACT_NOWANDER))
         {
            int oldx;
            int oldy;
            
            oldx = ch->coord->x;
            oldy = ch->coord->y;
            if (door == 0)
               ch->coord->y -=1;
            if (door == 1)
               ch->coord->x +=1;
            if (door == 2)
               ch->coord->y +=1;
            if (door == 3)
               ch->coord->x -=1;
            if (door == 6)
            {
               ch->coord->y -=1;
               ch->coord->x +=1;
            }
            if (door == 7)
            {
               ch->coord->y -=1;
               ch->coord->x -=1;
            }
            if (door == 8)
            {
               ch->coord->y +=1;
               ch->coord->x +=1;
            }
            if (door == 9)
            {
               ch->coord->y +=1;
               ch->coord->x -=1;
            }
            if (IS_VALID_COORDS(ch->coord->x, ch->coord->y) && wIS_SET(ch, ROOM_NO_MOB))
            {
               ch->coord->x = oldx;
               ch->coord->y = oldy;
            }
            else
            {
               ch->coord->x = oldx;
               ch->coord->y = oldy;
               if (door == 0) // north
                  if ((ch->coord->y - 1) != 0)
                     do_north(ch, " ");
               if (door == 1) // east
                  if ((ch->coord->x + 1) != MAX_X + 1)
                     do_east(ch, " ");
               if (door == 2) // south
                  if ((ch->coord->y + 1) != MAX_Y + 1)
                     do_south(ch, " ");
               if (door == 3) // west
                  if ((ch->coord->x - 1) != 0)
                     do_west(ch, " ");
               // ne nw se sw
               if (door == 6)
                  if (((ch->coord->y - 1) != 0) && ((ch->coord->x + 1) != MAX_X + 1))
                     do_northeast(ch, " ");
               if (door == 7)
                  if (((ch->coord->y - 1) != 0) && ((ch->coord->x - 1) != 0))
                     do_northwest(ch, " ");
               if (door == 8)
                  if (((ch->coord->y + 1) != MAX_Y + 1) && ((ch->coord->x + 1) != MAX_X + 1))
                     do_southeast(ch, " ");
               if (door == 9)
                  if (((ch->coord->y + 1) != MAX_Y + 1) && ((ch->coord->x - 1) != 0))
                     do_southwest(ch, " ");
            }
         }
         if (char_died(ch))
            continue;
         if (xIS_SET(ch->act, ACT_SENTINEL) || ch->position < POS_STANDING || (xIS_SET(ch->act, ACT_MILITARY) && xIS_SET(ch->miflags, KM_STATIONARY)))
            continue;

      }
      else
      {
         /* Wander */
         if (!xIS_SET(ch->act, ACT_RUNNING)
            && !xIS_SET(ch->act, ACT_SENTINEL)
            && !ch->fighting
            && (!xIS_SET(ch->act, ACT_MILITARY) ||
(xIS_SET(ch->act, ACT_MILITARY) && xIS_SET(ch->miflags, KM_PATROL))) &&
!xIS_SET(ch->act, ACT_PROTOTYPE) && !xIS_SET(ch->act, ACT_NOWANDER) &&
!xIS_SET(ch->act, ACT_MOUNTSAVE) && (door = number_bits(5)) <= 9 &&
(pexit = get_exit(ch->in_room, door)) != NULL && pexit->to_room &&
!IS_SET(pexit->exit_info, EX_CLOSED) && !xIS_SET(pexit->to_room->room_flags, ROOM_FORGEROOM) && 
!xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) && !xIS_SET(pexit->to_room->room_flags, ROOM_DEATH) && (!xIS_SET(ch->act, ACT_STAY_AREA) || pexit->to_room->area == ch->in_room->area) && (pexit->to_room->quad == ch->in_room->quad || ch->in_room->quad == 
0))
         {
            // Do not want mobs going up and down in the Newbie area because they
            // are harder up in the trees
            if (xIS_SET(ch->act, ACT_NOUPDN))
            {
               if (door == 4 || door == 5)
                  continue;
            }
            retcode = move_char(ch, pexit, 0);
            /* If ch changes position due
               to it's or someother mob's
               movement via MOBProgs,
               continue - Kahn */
            if (char_died(ch))
               continue;
            if (retcode != rNONE || xIS_SET(ch->act, ACT_SENTINEL)
               || ch->position < POS_STANDING || (xIS_SET(ch->act, ACT_MILITARY) && xIS_SET(ch->miflags, KM_STATIONARY)))
               continue;
         }
      }

      /* Flee */
      if (IS_ONMAP_FLAG(ch))
      {
         if ((door = number_bits(5)) <= 9
            && door != 4
            && door != 5
            && !ch->fighting
            && !xIS_SET(ch->act, ACT_SENTINEL)
            && (!xIS_SET(ch->act, ACT_MILITARY) ||
(xIS_SET(ch->act, ACT_MILITARY) && !xIS_SET(ch->miflags, KM_STATIONARY))) && !xIS_SET(ch->act, ACT_PROTOTYPE) && !xIS_SET(ch->act, ACT_NOWANDER))
         {
            CHAR_DATA *rch;
            bool found;

            found = FALSE;
            for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
            {
               if (is_fearing(ch, rch))
               {
                  if (ch->map == rch->map
                     && ch->coord->x == rch->coord->x
                     && ch->coord->y == rch->coord->y)
                  {
                     switch (number_bits(2))
                     {
                        case 0:
                           sprintf(buf, "%s Get away from me", rch->name);
                           break;
                        case 1:
                           sprintf(buf, "%s Leave me be", rch->name);
                           break;
                        case 2:
                           sprintf(buf, "%s Quit trying to kill me, leave me alone!", rch->name);
                           break;
                        case 3:
                           sprintf(buf, "%s Someone save me dear lord!", rch->name);
                           break;
                     }
                     do_tell(ch, buf);
                     found = TRUE;
                     break;
                  }
               }
            }

            if (found)
            {
               int oldx;
               int oldy;
            
               oldx = ch->coord->x;
               oldy = ch->coord->y;
               if (door == 0)
                  ch->coord->y -=1;
               if (door == 1)
                  ch->coord->x +=1;
               if (door == 2)
                  ch->coord->y +=1;
               if (door == 3)
                  ch->coord->x -=1;
               if (door == 6)
               {
                  ch->coord->y -=1;
                  ch->coord->x +=1;
               }
               if (door == 7)
               {
                  ch->coord->y -=1;
                  ch->coord->x -=1;
               }
               if (door == 8)
               {
                  ch->coord->y +=1;
                  ch->coord->x +=1;
               }
               if (door == 9)
               {
                  ch->coord->y +=1;
                  ch->coord->x -=1;
               }
               if (IS_VALID_COORDS(ch->coord->x, ch->coord->y) && wIS_SET(ch, ROOM_NO_MOB))
               {
                  ch->coord->x = oldx;
                  ch->coord->y = oldy;
               }
               else
               {
                  if (door == 0) // north
                     if ((ch->coord->y - 1) == 0)
                        do_north(ch, " ");
                  if (door == 1) // east
                     if ((ch->coord->x + 1) == MAX_X + 1)
                        do_east(ch, " ");
                  if (door == 2) // south
                     if ((ch->coord->y + 1) == MAX_Y + 1)
                        do_south(ch, " ");
                  if (door == 3) // west
                     if ((ch->coord->x - 1) == 0)
                        do_west(ch, " ");
                  // ne nw se sw
                  if (door == 6)
                     if (((ch->coord->y - 1) == 0) || ((ch->coord->x + 1) == MAX_X + 1))
                        do_northeast(ch, " ");
                  if (door == 7)
                     if (((ch->coord->y - 1) == 0) || ((ch->coord->x - 1) == 0))
                        do_northwest(ch, " ");
                  if (door == 8)
                     if (((ch->coord->y + 1) == MAX_Y + 1) || ((ch->coord->x + 1) == MAX_X + 1))
                        do_southeast(ch, " ");
                  if (door == 9)
                     if (((ch->coord->y + 1) == MAX_Y + 1) || ((ch->coord->x - 1) == 0))
                        do_southwest(ch, " ");
               }
            }
         }
      }
      else
      {
         if (ch->hit < ch->max_hit / 2
            && (door = number_bits(4)) <= 9
            && (pexit = get_exit(ch->in_room, door)) != NULL
            && pexit->to_room && !IS_SET(pexit->exit_info, EX_CLOSED) && !xIS_SET(pexit->to_room->room_flags, ROOM_FORGEROOM) &&
            !xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB))
         {
            CHAR_DATA *rch;
            bool found;

            found = FALSE;
            for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
            {
               if (is_fearing(ch, rch))
               {
                  switch (number_bits(2))
                  {
                     case 0:
                        sprintf(buf, "%s Get away from me", rch->name);
                        break;
                     case 1:
                        sprintf(buf, "%s Leave me be", rch->name);
                        break;
                     case 2:
                        sprintf(buf, "%s Quit trying to kill me, leave me alone!", rch->name);
                        break;
                     case 3:
                        sprintf(buf, "%s Someone save me dear lord!", rch->name);
                        break;
                  }
                  do_tell(ch, buf);
                  found = TRUE;
                  break;
               }
            }
            if (found)
               retcode = move_char(ch, pexit, 0);
         }
      }
   }

   return;
}

//Updates the interest (bank interest) of the player
void update_interest(CHAR_DATA * ch)
{
   if (ch->pcdata->lastinterest == 0)
   {
      ch->pcdata->lastinterest = time(0);
      return;
   }
   while (((time(0) - ch->pcdata->lastinterest) / cvttime(64800)) > 1) //1 month of game time
   {
      ch->pcdata->balance = URANGE(0, ch->pcdata->balance + (ch->pcdata->balance * .015), 1500000000);
      ch->pcdata->lastinterest += cvttime(64800);
   }
}

OBJ_DATA *find_quiver args((CHAR_DATA * ch));

int in_dominant_sphere(CHAR_DATA *ch, int ssn)
{
   int totalvalue = 0;
   int sn;
   int i;
   
   //Check to see if points are setup yet in spherepoints and grouppoints
   if (ch->pcdata->spherepoints[1] == -1 || ch->pcdata->grouppoints[1] == -1)
   {
      for (i = 1; i <= MAX_SPHERE; i++)
         ch->pcdata->spherepoints[i] = 0;
      for (i = 1; i <= MAX_GROUP+5; i++)
         ch->pcdata->grouppoints[i] = 0;
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (skill_table[sn]->group[0] > 0)
         {
            if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
            {
               ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP] += ch->pcdata->learned[sn];
               ch->pcdata->spherepoints[skill_table[sn]->stype] += ch->pcdata->learned[sn];
            }
            else
            {
               ch->pcdata->grouppoints[skill_table[sn]->group[0]] += ch->pcdata->learned[sn];
               ch->pcdata->spherepoints[skill_table[sn]->stype] += ch->pcdata->learned[sn];
            }
         }
      }
   }
   for (i = 1; i <= MAX_SPHERE; i++)
   { 
      totalvalue += ch->pcdata->spherepoints[i];
   }
   if (ch->pcdata->spherepoints[skill_table[ssn]->stype] > totalvalue/2)
      return TRUE;
   else
      return FALSE;
}

void update_loss_skill(CHAR_DATA *ch, int sn)
{
   int change = 0;
   int x;
   
   if (ch->pcdata->learned[sn] >= 1 && ch->pcdata->learned[sn] <= 3)
      change = number_range(10, 15);
   if (ch->pcdata->learned[sn] >= 4 && ch->pcdata->learned[sn] <= 5)
      change = number_range(15, 20);
   if (ch->pcdata->learned[sn] >= 6 && ch->pcdata->learned[sn] <= 7)
      change = number_range(20, 25);
   if (ch->pcdata->learned[sn] >= 8 && ch->pcdata->learned[sn] <= 9)
      change = number_range(25, 30);
   if (ch->pcdata->learned[sn] >= 10 && ch->pcdata->learned[sn] <= 12)
      change = number_range(30, 35);
   if (ch->pcdata->learned[sn] > 12)
      change = number_range(35, 40);
      
   for (x = 0; x < 5; x++)  //forget list
   {
      if (ch->pcdata->forget[x] == sn)
      {
         change *= 60;
         break;
      }
   }
      
   if (in_dominant_sphere(ch, sn) && x == 5)
      change /= 3;
      
   //Reducing this more, it is a little too much
   if (x == 5)
      change /= 3;
      
   for (x = 0; x < 5; x++)  //no learning list
   {
      if (ch->pcdata->nolearn[x] == sn)
         return;
   }   

   ch->pcdata->spercent[sn] -= change;
   if (ch->pcdata->spercent[sn] < 0)
   {
      ch_printf(ch, "&r*********************************************************************\n\r");
      ch_printf(ch, "&r****Your skill in %s decreases a point.****\n\r", skill_table[sn]->name);
      ch->pcdata->learned[sn]--;
      ch->pcdata->spercent[sn] = 9999;
      if ((ch->pcdata->learned[sn] < 17 && ch->pcdata->ranking[sn] == 6) //flawless
      || (ch->pcdata->learned[sn] < 14 && ch->pcdata->ranking[sn] == 5) //elite
      || (ch->pcdata->learned[sn] < 10 && ch->pcdata->ranking[sn] == 4) //master
      || (ch->pcdata->learned[sn] < 7 && ch->pcdata->ranking[sn] == 3) //expert
      || (ch->pcdata->learned[sn] < 4 && ch->pcdata->ranking[sn] == 2)) //novice
      {
         ch_printf(ch, "&r****You feel your hold on %s is decreasing.****\n\r", skill_table[sn]->name);
         ch->pcdata->ranking[sn]--;
      }
      if (ch->pcdata->learned[sn] == 0)
      {
         ch_printf(ch, "&r****Lack of using %s has made your skill in it useless.****\n\r", skill_table[sn]->name);
         ch->pcdata->ranking[sn] = 0;
         ch->pcdata->learned[sn] = 0;
         ch->pcdata->spercent[sn] = 0;
      }
      if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
      {
         ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP]--;
         ch->pcdata->spherepoints[skill_table[sn]->stype]--;
      }
      else
      {
         ch->pcdata->grouppoints[skill_table[sn]->group[0]]--;
         ch->pcdata->spherepoints[skill_table[sn]->stype]--;
      }
      ch_printf(ch, "&r*********************************************************************\n\r");
   }
   return;
}
void skill_update(void)
{
   DESCRIPTOR_DATA *d;
   
   for (d = first_descriptor; d; d = d->next)
   {
      int sn;
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (d->character && d->character->pcdata->learned[sn] > 0 && !IS_IMMORTAL(d->character) && skill_table[sn]->group[0] > 0)
            update_loss_skill(d->character, sn);
      }
   }
}

int totalrooms(int size)
{     
   if (size == 1)
      return 5;
   else if (size == 2)
      return 10;
   else if (size == 3)
      return 15;
   else if (size == 4)
      return 25;
   else if (size == 5)
      return 40;
   else if (size == 6)
      return 55;
   else if (size == 7)
      return 70;
   else if (size == 8)
      return 95;
   else if (size == 9)
      return 120;
   else if (size == 10)
      return 150;
   else
      return 5;
}

int get_minmoral(int size)
{
   if (size < 4)
      return 0;
   switch (size)
   {
      case 4:
         return 3;
      case 5:
         return 4;
      case 6:
         return 5;
      case 7:
         return 6;
      case 8:
         return 7;
      case 9:
         return 8;
      case 10:
         return 11;
      default:
         return 0;
   }
   return 0;
}

//Resource 0 - Food, 1 - Lumber, 2 - Stone, 3 - Coins
int check_resource_moral(TOWN_DATA *town, int required, int resource, int value, int reduce)
{
   int growth = 0;
   
   if (resource == 0)
   {
      if (town->foodconsump == 2)
         return reduce *-1;
      if (town->corn + town->grain + town->fish < required/12)
         return reduce *-1;
      if (town->corn + town->grain + town->fish >= required/12)
         growth = value/3;
      if (town->foodconsump == 0)
      {
         if (town->corn + town->grain + town->fish >= required/6)
            growth = value;
      }
      return growth;
   }
   if (resource == 1)
   {
      if (town->lumberconsump == 2)
         return reduce *-1;
      if (town->lumber < required/12)
         return reduce *-1;
      if (town->lumber >= required/12)
         growth = value/3;
      if (town->lumberconsump == 0)
      {
         if (town->lumber >= required/6)
            growth = value;
      }
      return growth;
   }
   if (resource == 2)
   {
      if (town->stoneconsump == 2)
         return reduce *-1;
      if (town->stone < required/12)
         return reduce *-1;
      if (town->stone >= required/12)
         growth = value/3;
      if (town->stoneconsump == 0)
      {
         if (town->stone >= required/6)
            growth = value;
      }
      return growth;
   }
   if (resource == 3)
   {
      if (town->coinconsump == 2)
         return reduce *-1;
      if (town->coins < required/12)
         return reduce *-1;
      if (town->coins >= required/12)
         growth = value/3;
      if (town->coinconsump == 0)
      {
         if (town->coins >= required/6)
            growth = value;
      }
      return growth;
   }
   return 0;
}
//0 - Check To see if on value
//1 - Check for increase/increase/no change
//2 - Returns the moral need for the size provided
int on_moral_change(int size, int moral, int type)
{    
   if (sysdata.resetgame)
   {
      if (type == 0)
      {
         if ((size == 1 && moral <= 0) || (size == 2 && moral <= 5) || (size == 3 && moral <= 11) || 
             (size == 4 && moral <= 17) || (size == 5 && moral <= 23) || (size == 6 && moral <= 29) || 
             (size == 7 && moral <= 41) || (size == 8 && moral <= 53) || (size == 9 && moral <= 71) || 
             (size == 10 && moral <= 99))
            return 1;
         else
            return 0;
      }
      if (type == 1)
      {
         if ((size == 1 && moral >= 6) || (size == 2 && moral >= 12) || (size == 3 && moral >= 18) || 
             (size == 4 && moral >= 24) || (size == 5 && moral >= 30) || (size == 6 && moral >= 42) || 
             (size == 7 && moral >= 54) || (size == 8 && moral >= 72) || (size == 9 && moral >= 100))
            return 1;
         else
            return 0;
      }
      if (type == 2)
      {
         if (size == 1)
            return 0;
         if (size == 2)
            return 6;
         if (size == 3)
            return 12;
         if (size == 4)
            return 18;
         if (size == 5)
            return 24;
         if (size == 6)
            return 30;
         if (size == 7)
            return 42;
         if (size == 8)
            return 54;
         if (size == 9)
            return 72;
         if (size == 10)
            return 100;
      }
      return 0; 
   } 
   if (type == 0)
   {
      if ((size == 1 && moral <= 0) || (size == 2 && moral <= 5) || (size == 3 && moral <= 11) || 
          (size == 4 && moral <= 23) || (size == 5 && moral <= 35) || (size == 6 && moral <= 53) || 
          (size == 7 && moral <= 71) || (size == 8 && moral <= 95) || (size == 9 && moral <= 119) || 
          (size == 10 && moral <= 179))
         return 1;
      else
         return 0;
   }
   if (type == 1)
   {
      if ((size == 1 && moral >= 6) || (size == 2 && moral >= 12) || (size == 3 && moral >= 24) || 
          (size == 4 && moral >= 36) || (size == 5 && moral >= 54) || (size == 6 && moral >= 72) || 
          (size == 7 && moral >= 96) || (size == 8 && moral >= 120) || (size == 9 && moral >= 180))
         return 1;
      else
         return 0;
   }
   if (type == 2)
   {
      if (size == 1)
         return 0;
      if (size == 2)
         return 6;
      if (size == 3)
         return 12;
      if (size == 4)
         return 24;
      if (size == 5)
         return 36;
      if (size == 6)
         return 54;
      if (size == 7)
         return 72;
      if (size == 8)
         return 96;
      if (size == 9)
         return 120;
      if (size == 10)
         return 180;
   }
   return 0;
}   

//Updates some kingdom stuff
void kingdom_update(void)
{
   int x;
   INTRO_DATA *intro;
   INTRO_DATA *intro_next = NULL;
   KINGDOM_DATA *kingdom;
   TOWN_DATA *town;
   TOWN_DATA *dtown;
   TOWN_DATA *ntown;
   int minmoral;
   int lastvalue;
   char buf[MSL];
   int kingtax = 0, towntax = 0;
   int taxstd = 0, taxdiff = 0, foodreq = 0, stonereq = 0, woodreq = 0, goldreq = 0, growth = 0, foodleft = 0;
   float taxfloat;
   int thold = 1, khold = 1;
   int taxrate1;
   int taxrate2;
   
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      kingdom = kingdom_table[x];
      //Tax/Growth checks are now here
      for (town = kingdom->first_town; town; town = ntown)
      {
         ntown = town->next;
         if (town->banksize <= 0)
            town->banksize = get_banksize(town->size);
         if (town->growthcheck == 0)
         {
            town->growthcheck = time(0);
            write_kingdom_file(kingdom->num);   
         }
         if (town->month <= 12)
         {
            town->month = time(0);
            write_kingdom_file(kingdom->num);
         }
            
         //Changed this a bit, Thanks to Altis for the idea of this, it just makes
         //more sense this way.....Growth is based on a 0-100 Moral system where you
         //grow once reaching 100 and decrease once you reach 0.  Moral increase/decrease
         //is based on the size of your town and what you did.  Small towns will grow
         //faster, larger ones slower.  Still should take awhile to reach a Size 10
         //yet the growing up to Size 5-6 at a fairly decent rate...
         if (time(0) - town->growthcheck >= cvttime(64800)) //1 Game Month)
         {
            town->growth++;
            town->growthcheck += cvttime(64800);
            taxstd = 90;
            
            if (town->size == 1)
               taxstd = 90;
            if (town->size >= 2 && town->size <= 4)
               taxstd = 80;
            if (town->size == 5 || town->size == 6 || town->size == 9)
               taxstd = 70;
            if (town->size == 7 || town->size == 8)
               taxstd = 65;
            if (town->size == 10)
               taxstd = 60;
               
            taxdiff = ((kingdom_table[town->kingdom]->poptax + town->poptax) * 100) / taxstd;            
            
            if (town->size == 1)
            {
               foodreq = 3000;    
            }
            else if (town->size == 2)
            {
               foodreq = 6000;   
               woodreq = 800; 
            }
            else if (town->size == 3)
            {
               foodreq = 10000;    
               woodreq = 1500; 
            }
            else if (town->size == 4)
            {
               foodreq = 15000;  
               woodreq = 2500;
               stonereq = 1000;  
            }
            else if (town->size == 5)
            {
               foodreq = 22000;    
               woodreq = 4000;
               stonereq = 1500;
               goldreq = 2000;
            }
            else if (town->size == 6)
            {
               foodreq = 40000;  
               woodreq = 6000;
               stonereq = 2500;
               goldreq = 7000;  
            }
            
            else if (town->size == 7)
            {
               foodreq = 70000; 
               woodreq = 11000;
               stonereq = 4000;
               goldreq = 15000;   
            }
            else if (town->size == 8)
            {
               foodreq = 110000; 
               woodreq = 18000;
               stonereq = 6000;
               goldreq = 35000;   
            }
            else if (town->size == 9)
            {
               foodreq = 150000;  
               woodreq = 30000;
               stonereq = 10000;
               goldreq = 80000;
            }
            else if (town->size == 10)
            {
               foodreq = 250000; 
               woodreq = 60000;
               stonereq = 15000;
               goldreq = 150000;   
            }
            else
            {
               foodreq = 50000;
               woodreq = 4000;
               stonereq = 3000;
               goldreq = 50000;
            }
            sprintf(buf, "____RESOURCES____ %d Food %d Wood %d Stone %d Coins needed for Single Consumption", foodreq/12, woodreq/12, stonereq/12, goldreq/12);
            write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);   
            if (woodreq == 0 && stonereq == 0 && goldreq == 0)
            {
               growth += check_resource_moral(town, foodreq, 0, 100, 0); //food
            }   
            if (woodreq > 0 && stonereq == 0 && goldreq == 0)
            {
               growth += check_resource_moral(town, foodreq, 0, 60, 0);  //food
               growth += check_resource_moral(town, woodreq, 1, 40, 25);  //wood           
            }
            if (woodreq > 0 && stonereq > 0 && goldreq == 0)
            {
               growth += check_resource_moral(town, foodreq, 0, 60, 0);  //food
               growth += check_resource_moral(town, woodreq, 1, 25, 25);  //wood     
               growth += check_resource_moral(town, stonereq, 2, 15, 15);  //stone   
            }
            if (woodreq > 0 && stonereq > 0 && goldreq > 0)
            {
               growth += check_resource_moral(town, foodreq, 0, 50, 0);  //food
               growth += check_resource_moral(town, woodreq, 1, 15, 25);  //wood     
               growth += check_resource_moral(town, stonereq, 2, 15, 15);  //stone   
               growth += check_resource_moral(town, goldreq, 3, 20, 15);  //coins
            }        
            
            if (taxdiff >= 200)       
               growth = -100;
            if (taxdiff > 100)
            {
               growth -= (taxdiff-100);
            }
            if (taxdiff < 100)
            {
               growth += URANGE(1, (100 - taxdiff) * 6 / 10, 40);
            }
            if ((minmoral = get_minmoral(town->size)) != 0)
            {
               if (get_kingdom_units(town->tpid) - minmoral <= 0)
                  growth += 0;
               if (get_kingdom_units(town->tpid) - minmoral == 1)
                  growth -= 5;
               if (get_kingdom_units(town->tpid) - minmoral == 2)
                  growth -= 10;
               if (get_kingdom_units(town->tpid) - minmoral == 3)
                  growth -= 15;
               if (get_kingdom_units(town->tpid) - minmoral == 4)
                  growth -= 20;
               if (get_kingdom_units(town->tpid) - minmoral == 5)
                  growth -= 25;
               if (get_kingdom_units(town->tpid) - minmoral == 6)
                  growth -= 35;
               if (get_kingdom_units(town->tpid) - minmoral == 7)
                  growth -= 45;
               if (get_kingdom_units(town->tpid) - minmoral == 8)
                  growth -= 55;
               if (get_kingdom_units(town->tpid) - minmoral == 9)
                  growth -= 70;
            }   
            if (town->corn + town->grain + town->fish < foodreq/12)
               growth = -100;
               
            growth = URANGE(-100, growth, 100);
            
            if (town->corn + town->grain + town->fish >= foodreq/6 && town->foodconsump == 0)
               foodreq = foodreq/6;
            else if (town->corn + town->grain + town->fish >= foodreq/12 && town->foodconsump != 2)
               foodreq = foodreq/12;
            else
               foodreq = 0;
               
            if (town->lumber >= woodreq/6 && town->lumberconsump == 0)
               woodreq = woodreq/6;
            else if (town->lumber >= woodreq/12 && town->lumberconsump != 2)
               woodreq = woodreq/12;
            else
               woodreq = 0;
               
            if (town->stone >= stonereq/6 && town->stoneconsump == 0)
               stonereq = stonereq/6;
            else if (town->stone >= stonereq/12 && town->stoneconsump != 2)
               stonereq = stonereq/12;
            else
               stonereq = 0;
               
            if (town->coins >= goldreq/6 && town->coinconsump == 0)
               goldreq = goldreq/6;
            else if (town->coins >= goldreq/12 && town->coinconsump != 2)
               goldreq = goldreq/12;
            else
               goldreq = 0;
          
            town->coins -= goldreq;
            town->stone -= stonereq;
            town->lumber -= woodreq;
            
            
            if (town->corn >= foodreq/3 && town->grain >= foodreq/3 && town->fish >= foodreq/3)
            {
               town->corn -= foodreq/3;
               town->grain -= foodreq/3;
               town->fish -= foodreq/3;
            }
            else
            {
               foodleft = foodreq;
               if (foodleft)
               {
                  if (town->corn > foodleft)
                  {
                     town->corn -= foodleft;
                     foodleft = 0;
                  }
                  else
                  {
                     foodleft -= town->corn;
                     town->corn = 0;
                  }
               }
               if (foodleft)
               {
                  if (town->grain > foodleft)
                  {
                     town->grain -= foodleft;
                     foodleft = 0;
                  }
                  else
                  {
                     foodleft -= town->grain;
                     town->grain = 0;
                  }
               }
               if (foodleft)
               {
                  if (town->fish > foodleft)
                  {
                     town->fish -= foodleft;
                     foodleft = 0;
                  }
                  else
                  {
                     foodleft -= town->fish;
                     town->fish = 0;
                  }
               }
            }
            if (sysdata.resetgame)
            {
               if (growth > 0 && town->moral == 103) 
                  growth = 0;
            }
            else
            {
               if (growth > 0 && town->moral == 183)
                  growth = 0;
            }
            taxrate1 = (kingdom_table[town->kingdom]->poptax + town->poptax)/10;
            taxrate2 = (kingdom_table[town->kingdom]->poptax + town->poptax)%10;
            sprintf(buf, "____RESOURCES____ %d Food %d Wood %d Stone %d Coins used (Tax: %d.%d%%)", foodreq, woodreq, stonereq, goldreq, taxrate1, taxrate2);
            write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);     
            if (growth > 0)
            {
               if (number_range(1, 100) <= growth)
               {
                  town->moral++;
                  sprintf(buf, "_____MORAL_____ Town %s gained a moral point (%d %d%%)", town->name, town->moral, growth);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
               }
               else
               {
                  sprintf(buf, "_____MORAL_____ Town %s's moral did not change (%d %d%%)", town->name, town->moral, growth);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
               }
            }
            else if (growth < 0)
            {
               if (number_range(1, 100) <= -growth)
               {
                  town->moral--;
                  sprintf(buf, "_____MORAL_____ Town %s lost a moral point (%d %d%%)", town->name, town->moral, growth);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
               }
               else
               {
                  sprintf(buf, "_____MORAL_____ Town %s's moral did not change (%d %d%%)", town->name, town->moral, growth);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
               }
            }
            else
            {
               sprintf(buf, "_____MORAL_____ Town %s's moral did not change (%d %d%%)", town->name, town->moral, growth);
               write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
            }   
            if (on_moral_change(town->size, town->moral, 1) == 1) //increase
            {
               int x, y, size;
               int nogrow = 0;
               size = get_control_size(town->size+1);
               //Check to see if you can grow first....
               for (x = town->startx - size; x <= town->startx+size; x++)
               {
                  for (y = town->starty - size; y <= town->starty+size; y++)
                  {
                     if (kingdom_sector[town->startmap][x][y] > 1 && kingdom_sector[town->startmap][x][y] != town->kingdom)
                     {
                        sprintf(buf, "_____POPULATION____ City %s cannot increase, conflicting AOC in the way", town->name);
                        write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
                        town->moral--;
                        nogrow = 1;
                        break;
                     }
                  }
                  if (nogrow == 1)
                     break;
               }
               if (!nogrow)
               {
                  size = get_control_size(town->size);
                  for (x = town->startx - size; x <= town->startx+size; x++)
                  {
                     for (y = town->starty - size; y <= town->starty+size; y++)
                     {
                        kingdom_sector[town->startmap][x][y] = 0;
                     }
                  } 
                  size = get_control_size(++town->size);
                  for (x = town->startx - size; x <= town->startx+size; x++)
                  {
                     for (y = town->starty - size; y <= town->starty+size; y++)
                     {
                        kingdom_sector[town->startmap][x][y] = town->kingdom;
                     }
                  } 
                  sprintf(buf, "_____POPULATION____ City %s increased in size", town->name);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
                  town->maxsize = totalrooms(town->size);
                  if (town->size == 5 || town->size == 6)
                     town->expansions = 1;
                  if (town->size == 7 || town->size == 8)
                     town->expansions = 2;
                  if (town->size == 9)
                     town->expansions = 3;
                  if (town->size == 10)
                     town->expansions = 4;    
                  town->banksize = get_banksize(town->size);
               }
            }
            else if (on_moral_change(town->size, town->moral, 0) == 1) //decrease
            {
               if (town->size > 1)
               {
                  int x, y, size;
                  TOWN_DATA *otown;
                  int cmoral;
                  cmoral = on_moral_change(town->size, town->moral, 2);
                  cmoral -= ((on_moral_change(town->size, town->moral, 2) - on_moral_change(town->size-1, town->moral, 2))/2);
                  town->moral = cmoral;
                  sprintf(buf, "_____POPULATION____ City %s decreased in size", town->name);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
                  sprintf(buf, "_____POPULATION____ City %s moral decreased to %d because of the reduction in size", town->name, town->moral);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
                     
                  size = get_control_size(town->size);
                  for (x = town->startx - size; x <= town->startx+size; x++)
                  {
                     for (y = town->starty - size; y <= town->starty+size; y++)
                     {
                        kingdom_sector[town->startmap][x][y] = 0;
                     }
                  } 
                  size = get_control_size(--town->size);
                  for (x = town->startx - size; x <= town->startx+size; x++)
                  {
                     for (y = town->starty - size; y <= town->starty+size; y++)
                     {
                        kingdom_sector[town->startmap][x][y] = town->kingdom;
                     }
                  } 
                  for (otown = kingdom_table[town->kingdom]->first_town; otown; otown = otown->next)
                  {
                     if (otown != town)
                     {
                        size = get_control_size(otown->size);
                        for (x = otown->startx - size; x <= otown->startx+size; x++)
                        {
                           for (y = otown->starty - size; y <= otown->starty+size; y++)
                           {
                              kingdom_sector[otown->startmap][x][y] = otown->kingdom;
                           }
                        }   
                     }
                  }
                  town->maxsize = totalrooms(town->size);
                  if (town->size == 5 || town->size == 6)
                     town->expansions = 1;
                  if (town->size == 7 || town->size == 8)
                     town->expansions = 2;
                  if (town->size == 9)
                     town->expansions = 3;
                  if (town->size == 10)
                     town->expansions = 4;
                  town->banksize = get_banksize(town->size);
               }
               else
               {
                  sprintf(buf, "_____POPULATION____ City %s decreased in size", town->name);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
                  sprintf(buf, "_____POPULATION____ City %s is no longer, it dropped below Size 1", town->name);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_POPULATION);
                  remove_town(town, 0);
               }
            }
            if (!town)
               continue;
            if (!kingdom || kingdom->num >= sysdata.max_kingdom)
               break;
            write_kingdom_file(kingdom->num);   
            //End growth check...
            
            //Changed the Tax System, want the tax collected every month now...
            if (town->size == 1)
               taxdiff = 30000;
            if (town->size == 2)
               taxdiff = 70000;
            if (town->size == 3)
               taxdiff = 150000;
            if (town->size == 4)
               taxdiff = 250000;
            if (town->size == 5)
               taxdiff = 500000;
            if (town->size == 6)
               taxdiff = 800000;
            if (town->size == 7)
               taxdiff = 1200000;
            if (town->size == 8)
               taxdiff = 2000000;
            if (town->size == 9)
               taxdiff = 3000000;
            if (town->size == 10)
               taxdiff = 5000000;
                  
            taxfloat = (kingdom_table[town->kingdom]->poptax + town->poptax) * taxdiff / 75;
            taxdiff = (int)taxfloat;
            
            //Used to be done every year now just monthly, some extra code here, but it works...   
            if (kingdom_table[town->kingdom]->poptax > 0)
            {
               taxfloat = (float)taxdiff * (float)kingdom_table[town->kingdom]->poptax / (float)(kingdom_table[town->kingdom]->poptax + (float)town->poptax) / 12;
               kingdom_table[town->kingdom]->ctax += (int)taxfloat;
            }
            if (town->poptax > 0)
            {
               taxfloat = (float)taxdiff * (float)town->poptax / ((float)kingdom_table[town->kingdom]->poptax + (float)town->poptax) / 12;
               town->ctax += (int)taxfloat;
            }
            kingtax = kingdom_table[town->kingdom]->ctax;
            towntax = town->ctax;           
               
            if ((dtown = get_town(kingdom_table[town->kingdom]->dtown)) != NULL)
            {
               if ((get_current_hold(dtown) + (kingtax/100)) <= dtown->hold)
                  dtown->coins += kingtax;
               else
                  khold = 0;
            }
            if ((get_current_hold(town) + (towntax/100)) <= town->hold)
               town->coins += towntax;
            else
               thold = 0;
               
            if (dtown)
            {
               if (khold == 0)
               {
                  sprintf(buf, "_____NOHOLD_____ %s Could not hold the extra gold", dtown->name);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_TAX);
               }
               else
               {
                  sprintf(buf, "_____TAX_____  %d gold paid to the Kingdom town of %s.", kingtax, dtown->name);
                  write_kingdom_logfile(town->kingdom, buf, KLOG_TAX);
               }
            }
            else
            {
               sprintf(buf, "_____ERROR_____ Could not find dtown to pay taxes to");
               write_kingdom_logfile(town->kingdom, buf, KLOG_TAX);
            }
            if (thold == 0)
            {
               sprintf(buf, "_____NOHOLD_____ City %s could not hold the extra gold", town->name);
               write_kingdom_logfile(town->kingdom, buf, KLOG_TAX);
            }
            else
            {
               sprintf(buf, "_____TAX_____  City of %s made %d gold.", town->name, towntax);
               write_kingdom_logfile(town->kingdom, buf, KLOG_TAX);
            }
            kingdom_table[town->kingdom]->ctax = 0;
            town->ctax = 0;    
            write_kingdom_file(kingdom->num);         
         }
      }
      if (!kingdom)
         break;
      for (;;)
      {
         if (kingdom->lastintrocheck == 0)
         {
            kingdom->lastintrocheck = time(0);
            break;
         }
         if (time(0) - kingdom->lastintrocheck > 12000000) //5-6 months
         {
            for (intro = kingdom->first_introduction; intro; intro = intro_next)
            {
               UNLINK(intro, kingdom->first_introduction, kingdom->last_introduction, next, prev);
               DISPOSE(intro);
            }
            break;
         }    
         for (intro = kingdom->first_introduction; intro; intro = intro_next)
         {
            intro_next = intro->next;
            if (intro->value < 0)
            {
               intro->value += number_range(1, 2);
               lastvalue = -1;
            }
            else
            {
              lastvalue = 1;
              intro->value -= number_range(1, 3);
            }
            if (intro->value == 0 || (intro->value * lastvalue < 0))
            {
               UNLINK(intro, kingdom->first_introduction, kingdom->last_introduction, next, prev);
               DISPOSE(intro);
            }
         }
         kingdom->lastintrocheck+=60;
         if (time(0) - kingdom->lastintrocheck < 60)
            break;
      }
   }   
} 

 /* Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update(void)
{
   CHAR_DATA *ch;
   CHAR_DATA *ch_save;
   OBJ_DATA *quiver;
   OBJ_DATA *arrow;
   AUTHORIZE_DATA *newauth;
   char *strtime;
   char buf[MSL];
   sh_int save_count = 0;
   INTRO_DATA *intro;
   int kdm;
   PROJECT_DATA *project;
   ch_save = NULL;
   for (ch = last_char; ch; ch = gch_prev)
   {
      if (ch == first_char && ch->prev)
      {
         bug("char_update: first_char->prev != NULL... fixed", 0);
         ch->prev = NULL;
      }
      gch_prev = ch->prev;
      set_cur_char(ch);
      if (gch_prev && gch_prev->next != ch)
      {
         bug("char_update: ch->prev->next != ch", 0);
         return;
      }

      /*
         *  Do a room_prog rand check right off the bat
         *   if ch disappears (rprog might wax npc's), continue
       */
      if (!IS_NPC(ch))
         rprog_random_trigger(ch);

      if (char_died(ch))
         continue;

      if (IS_NPC(ch))
         mprog_time_trigger(ch);

      if (char_died(ch))
         continue;

      rprog_time_trigger(ch);

      if (char_died(ch))
         continue;
      //Nice memory leak here Skan :-)
      if(ch->fame < 4000 && !IS_NPC(ch) && ch->pcdata->pretit)
      {
         DISPOSE(ch->pcdata->pretit);
         ch->pcdata->pretit = str_dup("");
      }
      if(ch->fame >= 4000 && !IS_NPC(ch))
      {
         if (ch->pcdata->pretit)
               DISPOSE(ch->pcdata->pretit);
         if(ch->sex == 0 || ch->sex == 1)
         {
            if (ch->pcdata->caste >= 31)
               ch->pcdata->pretit = str_dup("Ancient ");
            else if (ch->pcdata->caste == 28)
               ch->pcdata->pretit = str_dup("King ");
            else if (ch->pcdata->caste == 27)
               ch->pcdata->pretit = str_dup("Prince ");
            else if (ch->pcdata->caste == 26)
               ch->pcdata->pretit = str_dup("Minister ");
            else if (ch->pcdata->caste >= 21)
               ch->pcdata->pretit = str_dup("Lord ");
            else if (ch->pcdata->caste >= 14)
               ch->pcdata->pretit = str_dup("Baron ");
            else
               ch->pcdata->pretit = str_dup("Master ");
         }
         if(ch->sex == 2)
         {
	    if (ch->pcdata->caste >= 31)
               ch->pcdata->pretit = str_dup("Ancient ");
            else if (ch->pcdata->caste == 28)
               ch->pcdata->pretit = str_dup("Queen ");
            else if (ch->pcdata->caste == 27)
               ch->pcdata->pretit = str_dup("Princess ");
            else if (ch->pcdata->caste == 26)
               ch->pcdata->pretit = str_dup("Minister ");
            else if (ch->pcdata->caste >= 21)
               ch->pcdata->pretit = str_dup("Lady ");
            else if (ch->pcdata->caste >= 14)
               ch->pcdata->pretit = str_dup("Baroness ");
            else
               ch->pcdata->pretit = str_dup("Mistress ");
         }
      }
      if (!IS_NPC(ch))
      {
         for (project = first_project; project; project = project->next)
         {
            if (!str_cmp(project->rewardee, ch->name))
            {
               if (project->rewardedpoints < project->points)
               {
                  ch->pcdata->reward_curr += project->points-project->rewardedpoints;
                  ch->pcdata->reward_accum += project->points-project->rewardedpoints;
                  project->rewardedpoints = project->points;
                  write_projects();
               }
            }
         }
      }
            
      /*
       * See if player should be auto-saved.
       */
      if (!IS_NPC(ch)
         && (!ch->desc || ch->desc->connected == CON_PLAYING)
         && current_time - ch->pcdata->save_time > (sysdata.save_frequency * 60))
         ch_save = ch;
      else
         ch_save = NULL;
/*
	if ( ch->position >= POS_STUNNED )
	{
	    if ( ch->hit  < ch->max_hit )
		ch->hit  += hit_gain(ch);

	    if ( ch->mana < ch->max_mana )
		ch->mana += mana_gain(ch);

	    if ( ch->move < ch->max_move )
		ch->move += move_gain(ch);
	}
*/
      if ( ch->position >= POS_STUNNED)
      {
         int mod = 100;
         
         if (ch->position == POS_SITTING)
            mod = 150;
         if (ch->position == POS_RESTING)
            mod = 300;
         if (ch->position == POS_SLEEPING)
            mod = 600; 
            
         if (ch->race == RACE_ELF)
            mod = mod * 90/100;
         if (ch->race == RACE_HOBBIT)
            mod = mod * 80/100;
         if (ch->race == RACE_FAIRY)
            mod = mod * 60/100;
         if (ch->race == RACE_DWARF)
            mod = mod * 115/100;
         if (ch->race == RACE_OGRE)
            mod = mod * 150/100;
         
         if (ch->con_rarm > -1)
         {
            ch->con_rarm = UMIN(1000, ch->con_rarm + number_range(20, 30)*mod/100);
         }
         if (ch->con_larm > -1)
         {
            ch->con_larm = UMIN(1000, ch->con_larm + number_range(20, 30)*mod/100);
         }
         if (ch->con_rleg > -1)
         {
            ch->con_rleg = UMIN(1000, ch->con_rleg + number_range(15, 25)*mod/100);
         }
         if (ch->con_lleg > -1)
         {
            ch->con_lleg = UMIN(1000, ch->con_lleg + number_range(15, 25)*mod/100);
         }
      }
      if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY)) 
      {
         CONQUER_DATA *conquer;
         char logb[200];
         
         if (xIS_SET(ch->miflags, KM_CONQUER) && kingdom_sector[ch->map][ch->coord->x][ch->coord->y] > 1
         &&  kingdom_sector[ch->map][ch->coord->x][ch->coord->y] < sysdata.max_kingdom 
         &&  kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->m4 )
         { 
            for (conquer = first_conquer; conquer; conquer = conquer->next)
            {
               if (ch->m4 == conquer->akingdom && conquer->town &&  ch->coord->x == conquer->town->startx 
               && ch->coord->y == conquer->town->starty && ch->map == conquer->town->startmap)
               {
                  conquer->occupied = 1;
                  break;
               }
            }
            if (!conquer)
            {
               TOWN_DATA *town;
               
               for (town = kingdom_table[kingdom_sector[ch->map][ch->coord->x][ch->coord->y]]->first_town; town; town = town->next)
               {
                  if (town->startx == ch->coord->x && town->starty == ch->coord->y && town->startmap == ch->map)
                  {
                     CREATE(conquer, CONQUER_DATA, 1);
                     conquer->occupied = 1;
                     conquer->akingdom = ch->m4;
                     conquer->rkingdom = kingdom_sector[ch->map][ch->coord->x][ch->coord->y];
                     conquer->ntown = STRALLOC(town->name);
                     conquer->town = town;
                     conquer->time = time(0);
                     LINK(conquer, first_conquer, last_conquer, next, prev);
                     sprintf(logb, "You have started to conquer %s's town of %s", kingdom_table[conquer->rkingdom]->name, conquer->town->name);
                     write_kingdom_logfile(conquer->akingdom, logb, KLOG_WARLOSSES);    
                     sprintf(logb, "%s has started to conquer your town of %s", kingdom_table[conquer->akingdom]->name, conquer->town->name);
                     write_kingdom_logfile(conquer->rkingdom, logb, KLOG_WARLOSSES);    
                     save_conquer_file();
                     break;
                  }
               }
            }
         }
      }
      //Introduction system, time to forget...
      if (!IS_NPC(ch))
      {
         INTRO_DATA *intro_next = NULL;
         int lastvalue;
         
         for (intro = kingdom_table[ch->pcdata->hometown]->first_introduction; intro; intro = intro->next)
         {
            if (ch->pcdata->pid == intro->pid)
            {
               intro->value = 150000;
               break;
            }
         }
         if (!intro)
         {
            CREATE(intro, INTRO_DATA, 1);
            intro->value = 150000;
            intro->pid = ch->pcdata->pid;
            intro->lastseen = time(0);
            LINK(intro, kingdom_table[ch->pcdata->hometown]->first_introduction, kingdom_table[ch->pcdata->hometown]->last_introduction, next, prev);   
         }
         intro=NULL;
         
         for (;;)
         {
            if (ch->pcdata->lastintrocheck == 0)
            {
               ch->pcdata->lastintrocheck = time(0);
               break;
            }
            if (time(0) - ch->pcdata->lastintrocheck > 12000000) //5-6 months
            {
               for (intro = ch->pcdata->first_introduction; intro; intro = intro_next)
               {
                  UNLINK(intro, ch->pcdata->first_introduction, ch->pcdata->last_introduction, next, prev);
                  DISPOSE(intro);
               }
               ch->pcdata->lastintrocheck = time(0);
               break;
            }    
            for (intro = ch->pcdata->first_introduction; intro; intro = intro_next)
            {
               intro_next = intro->next;
               if (intro->value < 0)
               {
                  intro->value += number_range(1, 2);
                  lastvalue = -1;
               }
               else
               {
                 lastvalue = 1;
                 intro->value -= number_range(1, 3);
               }
               if (intro->value == 0 || (intro->value * lastvalue < 0))
               {
                  UNLINK(intro, ch->pcdata->first_introduction, ch->pcdata->last_introduction, next, prev);
                  DISPOSE(intro);
               }
            }
            ch->pcdata->lastintrocheck+=60;
            if (time(0) - ch->pcdata->lastintrocheck < 60)
               break;
         }
      }        
      //Mainly to make sure they are not hopping and portaling, etc
      if (!IS_NPC(ch))
      {
         int p;

         if (ch->desc)
         {
            if (ch->desc->idle >= 2400) // About 10 minutes
            {
               xSET_BIT(ch->act, PLR_AFK);
               xREMOVE_BIT(ch->act, PLR_AWAY);
            }
            if (ch->desc->idle >= 720 && !xIS_SET(ch->act, PLR_AFK)) // About 3 minutes
               xSET_BIT(ch->act, PLR_AWAY);
         }


         for (p = 0; p < sysdata.last_portal; p++)
         {
            for (kdm = 0; kdm < sysdata.max_kingdom; kdm++)
            {
               if (kdm > 1)
               {
                  sprintf(buf, "%s Portal", kingdom_table[kdm]->name);
                  if (!str_cmp(portal_show[p]->desc, buf))
                  {
                     xREMOVE_BIT(ch->pcdata->portalfnd, p);
                     if (ch->pcdata->hometown == kdm)
                        xSET_BIT(ch->pcdata->portalfnd, p);
                  }
               }
            }
         }
      }
      if (!IS_NPC(ch) && ch->hit > -4 && ch->position <= POS_STUNNED)
         update_pos(ch);
      if (!IS_NPC(ch) && ch->hit < -10)
      {
         update_pos(ch);
      }
      if (char_died(ch))
         continue;
      if (IS_NPC(ch) && IS_ACT_FLAG(ch, ACT_MILITARY) && ch->in_room)
      {
         int cnt = 0;
         if ((quiver = find_quiver(ch)) != NULL)
         {
            for (arrow = quiver->last_content; arrow; arrow = arrow->prev_content)
            {
               if (can_see_obj(ch, arrow))
               {
                  if (arrow->item_type == ITEM_PROJECTILE)
                     cnt++;
               }
            }
            for (; cnt < 10; cnt++)
            {
               if (ch->pIndexData->vnum == MOB_KARCHER)
               {
                  arrow = create_object(get_obj_index(16172), ch->level);
                  obj_to_obj(arrow, quiver);
               }
               if (ch->pIndexData->vnum == MOB_KCROSSBOW)
               {
                  arrow = create_object(get_obj_index(16173), ch->level);
                  obj_to_obj(arrow, quiver);
               }
               if (ch->pIndexData->vnum == MOB_KSLINGER)
               {
                  arrow = create_object(get_obj_index(16174), ch->level);
                  obj_to_obj(arrow, quiver);
               }
            }
         }
      }

      if (IS_NPC(ch))
      {
         if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
         {
            if (!IS_ACT_FLAG(ch, ACT_PROTOTYPE) && !IS_ACT_FLAG(ch, ACT_SENTINEL) &&
               !IS_ACT_FLAG(ch, ACT_TRAINER) && !IS_ACT_FLAG(ch, ACT_PACIFIST) &&
               !IS_AFFECTED(ch, AFF_CHARM) &&
               !IS_ACT_FLAG(ch, ACT_PET) && !IS_ACT_FLAG(ch, ACT_MOUNTSAVE) && IS_AWAKE(ch) &&
               !ch->fighting && ch->hunting == NULL && !IS_ACT_FLAG(ch, ACT_EXTRACTMOB) && !IS_ACT_FLAG(ch, ACT_MOVEMAP))
            {
               extract_char(ch, TRUE);
               continue;
            }
         }
      }
      /*   Morph timer expires */

      if (ch->morph)
      {
         if (ch->morph->timer > 0)
         {
            ch->morph->timer--;
            if (ch->morph->timer == 0)
               do_unmorph_char(ch);
         }
      }

      /* To make people with a nuisance's flags life difficult
       * --Shaddai
       */

      if (!IS_NPC(ch) && ch->pcdata->nuisance)
      {
         long int temp;

         if (ch->pcdata->nuisance->flags < MAX_NUISANCE_STAGE)
         {
            temp = ch->pcdata->nuisance->max_time - ch->pcdata->nuisance->time;
            temp *= ch->pcdata->nuisance->flags;
            temp /= MAX_NUISANCE_STAGE;
            temp += ch->pcdata->nuisance->time;
            if (temp < current_time)
               ch->pcdata->nuisance->flags++;
         }
      }
      if (!IS_NPC(ch) && ch->desc && ch->pcdata->authwait != -1 && --ch->pcdata->authwait == 0)
      {
         ch->pcdata->auth_state = 3;
         SET_BIT(ch->pcdata->flags, PCFLAG_UNAUTHED);
         CREATE(newauth, AUTHORIZE_DATA, 1);
         newauth->name = QUICKLINK(ch->name);
         newauth->lastname = QUICKLINK(ch->last_name);
         newauth->authedby = STRALLOC("Auto");
         strtime = ctime(&current_time);
         strtime[strlen(strtime) - 1] = '\0';
         newauth->authdate = STRALLOC(strtime);
         if (ch->desc)
         {
            sprintf(buf, "%s", ch->desc->host);
            newauth->host = STRALLOC(buf);
         }
         LINK(newauth, first_authorized, last_authorized, next, prev);
         fwrite_authlist();
         if (ch->pcdata->authed_by)
            STRFREE(ch->pcdata->authed_by);
         ch->pcdata->authed_by = STRALLOC("Auto");
         sprintf(buf, "%s: auto-authorized", ch->name);
         to_channel(buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL); /* Tracker1 */

         /* Below sends a message to player when name is accepted - Brittany */
         ch_printf_color(ch, "\n\r&GRafermand auto accepted the name of %s.\n\r" "You may be asked later to change it.  Enter Rafermand by typing ---pull sword---\n\r", ch->name);
         ch->pcdata->authwait = -1;
      }
      if (!IS_NPC(ch) && ch->level < LEVEL_IMMORTAL)
      {
         OBJ_DATA *obj;

         if ((obj = get_eq_char(ch, WEAR_LIGHT)) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] > 0)
         {
            if (--obj->value[2] == 0 && ch->in_room)
            {
               ch->in_room->light -= obj->count;
               if (ch->in_room->light < 0)
                  ch->in_room->light = 0;
               act(AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM);
               act(AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR);
               if (obj->serial == cur_obj)
                  global_objcode = rOBJ_EXPIRED;
               extract_obj(obj);
            }
         }

         if (ch->pcdata && (ch->pcdata->timeout_idle > 3600 || ch->pcdata->timeout_idle == 0)) //15 minutes
         {
            if (++ch->timer >= 12)
            {
               if (ch->timer == 12 && ch->in_room && !ch->rider && !ch->riding)
               {
                  /*
                     ch->was_in_room = ch->in_room;
                   */
                  if (ch->fighting)
                     stop_fighting(ch, TRUE);
                  act(AT_ACTION, "$n disappears into the void.", ch, NULL, NULL, TO_ROOM);
                  send_to_char("You disappear into the void.\n\r", ch);
                  if (IS_SET(sysdata.save_flags, SV_IDLE))
                     save_char_obj(ch);
                  char_from_room(ch);
                  char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
               }
            }
         }

         if (ch->pcdata->condition[COND_DRUNK] > 8)
            worsen_mental_state(ch, ch->pcdata->condition[COND_DRUNK] / 8);
         if (ch->pcdata->condition[COND_FULL] > 1)
         {
            switch (ch->position)
            {
               case POS_SLEEPING:
                  better_mental_state(ch, 4);
                  break;
               case POS_RESTING:
                  better_mental_state(ch, 3);
                  break;
               case POS_SITTING:
               case POS_MOUNTED:
               case POS_RIDING:
                  better_mental_state(ch, 2);
                  break;
               case POS_STANDING:
                  better_mental_state(ch, 1);
                  break;
               case POS_FIGHTING:
               case POS_EVASIVE:
               case POS_DEFENSIVE:
               case POS_AGGRESSIVE:
               case POS_BERSERK:
                  if (number_bits(2) == 0)
                     better_mental_state(ch, 1);
                  break;
            }
         }
         if (ch->pcdata->condition[COND_THIRST] > 1)
         {
            switch (ch->position)
            {
               case POS_SLEEPING:
                  better_mental_state(ch, 5);
                  break;
               case POS_RESTING:
                  better_mental_state(ch, 3);
                  break;
               case POS_SITTING:
               case POS_MOUNTED:
               case POS_RIDING:
                  better_mental_state(ch, 2);
                  break;
               case POS_STANDING:
                  better_mental_state(ch, 1);
                  break;
               case POS_FIGHTING:
               case POS_EVASIVE:
               case POS_DEFENSIVE:
               case POS_AGGRESSIVE:
               case POS_BERSERK:
                  if (number_bits(2) == 0)
                     better_mental_state(ch, 1);
                  break;
            }
         }
/*
	    gain_condition( ch, COND_FULL,   -1 );
*/
         gain_condition(ch, COND_DRUNK, -1);
         if (!IS_AFFECTED(ch, AFF_NOHUNGER) && number_range(1, 100) <= UMAX(90, ch->apply_fasting))
            gain_condition(ch, COND_FULL, -1 + race_table[ch->race]->hunger_mod);
         if (LEARNED(ch, gsn_fasting) >= 1)
            learn_from_success(ch, gsn_fasting, NULL);

         if (IS_VAMPIRE(ch))
         {
            if (gethour() < 21 && gethour() > 5)
               gain_condition(ch, COND_BLOODTHIRST, -1);
         }


         if (!IS_NPC(ch) && ch->pcdata->nuisance)
         {
            int value;

            value = ((0 - ch->pcdata->nuisance->flags) * ch->pcdata->nuisance->power);
            gain_condition(ch, COND_THIRST, value);
            gain_condition(ch, COND_FULL, --value);
         }

         if (ch->in_room)
            switch (ch->in_room->sector_type)
            {
               default:
                  if (!IS_AFFECTED(ch, AFF_NOTHIRST) && number_range(1, 100) <= UMAX(90, ch->apply_fasting))
                     gain_condition(ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod);
                  break;
               case SECT_DESERT:
                  if (!IS_AFFECTED(ch, AFF_NOTHIRST) && number_range(1, 100) <= UMAX(90, ch->apply_fasting))
                     gain_condition(ch, COND_THIRST, -3 + race_table[ch->race]->thirst_mod);
                  break;
               case SECT_UNDERWATER:
               case SECT_OCEANFLOOR:
                  if (number_bits(1) == 0)
                     if (!IS_AFFECTED(ch, AFF_NOTHIRST) && number_range(1, 100) <= UMAX(90, ch->apply_fasting))
                        gain_condition(ch, COND_THIRST, -1 + race_table[ch->race]->thirst_mod);
                  break;
            }

      }
      if (!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->release_date > 0 && ch->pcdata->release_date <= current_time)
      {
         ROOM_INDEX_DATA *location;

         if (ch->pcdata->clan)
            location = get_room_index(ch->pcdata->clan->recall);
         else
            location = get_room_index(ROOM_VNUM_TEMPLE);
         if (!location)
            location = ch->in_room;
         MOBtrigger = FALSE;
         char_from_room(ch);
         char_to_room(ch, location);
         send_to_char("The gods have released you from hell as your sentance is up!\n\r", ch);
         do_look(ch, "auto");
         STRFREE(ch->pcdata->helled_by);
         ch->pcdata->release_date = 0;
         save_char_obj(ch);
      }

      if (!char_died(ch))
      {
         /*
          * Careful with the damages here,
          *   MUST NOT refer to ch after damage taken, without checking
          *   return code and/or char_died as it may be lethal damage.
          */
         if (IS_AFFECTED(ch, AFF_POISON))
         {
            act(AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM);
            act(AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR);
            ch->mental_state = URANGE(20, ch->mental_state + (IS_NPC(ch) ? 2 : 4), 100);
            damage(ch, ch, 6, gsn_poison, 0, -1);
         }
         if (char_died(ch))
            continue;


         if (ch->mental_state >= 30)
            switch ((ch->mental_state + 5) / 10)
            {
               case 3:
                  send_to_char("You feel feverish.\n\r", ch);
                  act(AT_ACTION, "$n looks kind of out of it.", ch, NULL, NULL, TO_ROOM);
                  break;
               case 4:
                  send_to_char("You do not feel well at all.\n\r", ch);
                  act(AT_ACTION, "$n doesn't look too good.", ch, NULL, NULL, TO_ROOM);
                  break;
               case 5:
                  send_to_char("You need help!\n\r", ch);
                  act(AT_ACTION, "$n looks like $e could use your help.", ch, NULL, NULL, TO_ROOM);
                  break;
               case 6:
                  send_to_char("Seekest thou a cleric.\n\r", ch);
                  act(AT_ACTION, "Someone should fetch a healer for $n.", ch, NULL, NULL, TO_ROOM);
                  break;
               case 7:
                  send_to_char("You feel reality slipping away...\n\r", ch);
                  act(AT_ACTION, "$n doesn't appear to be aware of what's going on.", ch, NULL, NULL, TO_ROOM);
                  break;
               case 8:
                  send_to_char("You begin to understand... everything.\n\r", ch);
                  act(AT_ACTION, "$n starts ranting like a madman!", ch, NULL, NULL, TO_ROOM);
                  break;
               case 9:
                  send_to_char("You are ONE with the universe.\n\r", ch);
                  act(AT_ACTION, "$n is ranting on about 'the answer', 'ONE' and other mumbo-jumbo...", ch, NULL, NULL, TO_ROOM);
                  break;
               case 10:
                  send_to_char("You feel the end is near.\n\r", ch);
                  act(AT_ACTION, "$n is muttering and ranting in tongues...", ch, NULL, NULL, TO_ROOM);
                  break;
            }
         if (ch->mental_state <= -30)
            switch ((abs(ch->mental_state) + 5) / 10)
            {
               case 10:
                  if (ch->position > POS_SLEEPING)
                  {
                     if ((ch->position == POS_STANDING || ch->position < POS_FIGHTING) && number_percent() + 10 < abs(ch->mental_state))
                        do_sleep(ch, "");
                     else
                        send_to_char("You're barely conscious.\n\r", ch);
                  }
                  break;
               case 9:
                  if (ch->position > POS_SLEEPING)
                  {
                     if ((ch->position == POS_STANDING || ch->position < POS_FIGHTING) && (number_percent() + 20) < abs(ch->mental_state))
                        do_sleep(ch, "");
                     else
                        send_to_char("You can barely keep your eyes open.\n\r", ch);
                  }
                  break;
               case 8:
                  if (ch->position > POS_SLEEPING)
                  {
                     if (ch->position < POS_SITTING && (number_percent() + 30) < abs(ch->mental_state))
                        do_sleep(ch, "");
                     else
                        send_to_char("You're extremely drowsy.\n\r", ch);
                  }
                  break;
               case 7:
                  if (ch->position > POS_RESTING)
                     send_to_char("You feel very unmotivated.\n\r", ch);
                  break;
               case 6:
                  if (ch->position > POS_RESTING)
                     send_to_char("You feel sedated.\n\r", ch);
                  break;
               case 5:
                  if (ch->position > POS_RESTING)
                     send_to_char("You feel sleepy.\n\r", ch);
                  break;
               case 4:
                  if (ch->position > POS_RESTING)
                     send_to_char("You feel tired.\n\r", ch);
                  break;
               case 3:
                  if (ch->position > POS_RESTING)
                     send_to_char("You could use a rest.\n\r", ch);
                  break;
            }
         if (ch->timer > 24)
            char_quit(ch, FALSE); /* Rantics info channel */
         else if (ch == ch_save && IS_SET(sysdata.save_flags, SV_AUTO) && ++save_count < 10) /* save max of 10 per tick */
            save_char_obj(ch);
      }
   }

   return;
}



/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update(void)
{
   OBJ_DATA *obj;
   sh_int AT_TEMP;
   
   for (obj = last_object; obj; obj = gobj_prev)
   {
      CHAR_DATA *rch;
      char *message;

      if (obj == first_object && obj->prev)
      {
         bug("obj_update: first_object->prev != NULL... fixed", 0);
         obj->prev = NULL;
      }
      gobj_prev = obj->prev;
      if (gobj_prev && gobj_prev->next != obj)
      {
         bug("obj_update: obj->prev->next != obj", 0);
         return;
      }
      set_cur_obj(obj);
      if (obj->carried_by)
         oprog_random_trigger(obj->carried_by, obj);
      else if (obj->in_room && obj->in_room->area->nplayer > 0)
         oprog_random_trigger(obj->carried_by, obj);

      if (obj_extracted(obj))
         continue;

      if (obj->item_type == ITEM_PIPE)
      {
         if (IS_SET(obj->value[3], PIPE_LIT))
         {
            if (--obj->value[1] <= 0)
            {
               obj->value[1] = 0;
               REMOVE_BIT(obj->value[3], PIPE_LIT);
            }
            else if (IS_SET(obj->value[3], PIPE_HOT))
               REMOVE_BIT(obj->value[3], PIPE_HOT);
            else
            {
               if (IS_SET(obj->value[3], PIPE_GOINGOUT))
               {
                  REMOVE_BIT(obj->value[3], PIPE_LIT);
                  REMOVE_BIT(obj->value[3], PIPE_GOINGOUT);
               }
               else
                  SET_BIT(obj->value[3], PIPE_GOINGOUT);
            }
            if (!IS_SET(obj->value[3], PIPE_LIT))
               SET_BIT(obj->value[3], PIPE_FULLOFASH);
         }
         else
            REMOVE_BIT(obj->value[3], PIPE_HOT);
      }

/* Corpse decay (npc corpses decay at 480 times the rate of pc corpses) - Narn */

      if (obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC)
      {
         sh_int timerfrac = UMAX(1, obj->timer - 1);
         
         if (obj->item_type == ITEM_CORPSE_PC && !obj->first_content && obj->timer > 5)
            obj->timer = 5;

         if (obj->item_type == ITEM_CORPSE_PC)
            timerfrac = (int) (obj->timer / 480 + 1);            
         
         if (obj->item_type == ITEM_CORPSE_PC && obj->timer > 5 && obj->timer <= 1440 && obj->in_room && obj->in_room->vnum != VNUM_ROOM_MORGUE)
         {
            obj_from_room(obj);
            obj_to_room(obj, get_room_index(VNUM_ROOM_MORGUE), NULL);
            update_container(obj, -1, -1, -1, 0, 0, 0);
         }

         if (obj->timer > 0 && obj->value[2] > timerfrac)
         {
            char buf[MSL];
            char name[MSL];
            char *bufptr;
            
            bufptr = one_argument(obj->short_descr, name);
            bufptr = one_argument(bufptr, name);
            bufptr = one_argument(bufptr, name);
            
            if (obj->item_type == ITEM_CORPSE_PC)
            {
               bufptr = "a once breathing being";
            } 

            separate_obj(obj);
            obj->value[2] = timerfrac;
            sprintf(buf, corpse_descs[UMIN(timerfrac - 1, 4)], bufptr);

            STRFREE( obj->description );
            obj->description = STRALLOC( buf ); 
         }
      }

      /* don't let inventory decay */
      if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
         continue;

      /* groundrot items only decay on the ground */
      if (IS_OBJ_STAT(obj, ITEM_GROUNDROT) && !obj->in_room)
         continue;
         
      //Keys on mobs with timers, don't want them to expire before a player can get to it.
      if (obj->carried_by && obj->item_type == ITEM_KEY && IS_NPC(obj->carried_by))
         continue;

      if ((obj->timer <= 0 || --obj->timer > 0) && !IS_OBJ_STAT(obj, ITEM_GROUNDROT))
      {
         continue;
      }

      /* if we get this far, object's timer has expired. */

      AT_TEMP = AT_PLAIN;
      switch (obj->item_type)
      {
         default:
            message = "$p mysteriously vanishes.";
            AT_TEMP = AT_PLAIN;
            break;
         case ITEM_CONTAINER:
            message = "$p falls apart, tattered from age.";
            AT_TEMP = AT_OBJECT;
            break;
         case ITEM_PORTAL:
            message = "$p unravels and winks from existence.";
            //   remove_portal(obj);
            obj->item_type = ITEM_TRASH; /* so extract_obj  */
            AT_TEMP = AT_MAGIC; /* doesn't remove_portal */
            break;
         case ITEM_FOUNTAIN:
            message = "$p dries up.";
            AT_TEMP = AT_BLUE;
            break;
         case ITEM_CORPSE_NPC:
            message = "$p decays into dust and blows away.";
            AT_TEMP = AT_OBJECT;
            break;
         case ITEM_CORPSE_PC:
            message = "$p is sucked into a swirling vortex of colors...";
            AT_TEMP = AT_MAGIC;
            break;
         case ITEM_COOK:
         case ITEM_FOOD:
            message = "$p is devoured by a swarm of maggots.";
            AT_TEMP = AT_HUNGRY;
            break;
         case ITEM_BLOOD:
            message = "$p slowly seeps into the ground.";
            AT_TEMP = AT_BLOOD;
            break;
         case ITEM_BLOODSTAIN:
            message = "$p dries up into flakes and blows away.";
            AT_TEMP = AT_BLOOD;
            break;
         case ITEM_SCRAPS:
            message = "$p crumble and decay into nothing.";
            AT_TEMP = AT_OBJECT;
            break;
         case ITEM_FIRE:
            if (obj->in_room)
            {
               --obj->in_room->light;
               if (obj->in_room->light < 0)
                  obj->in_room->light = 0;
            }
            message = "$p burns out.";
            AT_TEMP = AT_FIRE;
      }

      if (xIS_SET(obj->extra_flags, ITEM_POISONED))
      {
         message = "It appears all the poison on $p has faded away.";
         AT_TEMP = AT_RED;
         xREMOVE_BIT(obj->extra_flags, ITEM_POISONED); 
         obj->value[0] -= obj->value[0] * 6/10;
      }
      if (obj->carried_by)
      {
         act(AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR);
      }
      else if (obj->in_room && (rch = obj->in_room->first_person) != NULL && !IS_OBJ_STAT(obj, ITEM_BURIED))
      {
         act(AT_TEMP, message, rch, obj, NULL, TO_ROOM);
         act(AT_TEMP, message, rch, obj, NULL, TO_CHAR);
      }
      if (!xIS_SET(obj->extra_flags, ITEM_POISONED))
      {
         if (obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC)
         {
            OBJ_DATA *obj_content;    
            int cnt=0; //For corpses in inventory or a bug...

            while ((obj_content = obj->last_content) != NULL)
            {
               cnt++;
               if (cnt > 5000)
                  break;
               if (obj_content->in_obj && obj->in_room)
               {
                  obj_from_obj(obj_content);
                  obj_to_room(obj_content, obj->in_room, NULL);
                  obj_content->coord->x = obj->coord->x;
                  obj_content->coord->y = obj->coord->y;
                  obj_content->map = obj->map;
                  if (IS_OBJ_STAT(obj, ITEM_ONMAP))
                     SET_OBJ_STAT(obj_content, ITEM_ONMAP);
               }
            }
         }                  
         if (obj->serial == cur_obj)
            global_objcode = rOBJ_EXPIRED;
         extract_obj(obj);
      }
   }
   return;
}

void trade_check(void)
{
   TRADE_DATA *trades;
   char trade[200];
   
   for (trades = first_trade; trades; trades = trades->next)
   {
      if (time(0) - trades->time > 259200) //3 days
      {
         sprintf(trade, "***TRADE TIMEDOUT*** %s-%s", kingdom_table[trades->offering_kingdom]->name, 
              kingdom_table[trades->receiving_kingdom]->name);
         write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
         write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      
         sprintf(trade, "***TRADE TIMEDOUT*** %s", get_resources_traded(trades, 1));
         write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
         write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      
         sprintf(trade, "***TRADE TIMEDOUT*** %s", get_resources_traded(trades, 2));
         write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
         write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
         UNLINK(trades, first_trade, last_trade, next, prev);
         DISPOSE(trades); 
         save_trade_file();
      }
   }
}

/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check(void)
{
   CHAR_DATA *ch, *ch_next;
   CHAR_DATA *undead, *next_undead;
   OBJ_DATA *obj;
   EXIT_DATA *pexit; 
   static int cnt = 0;
   int door, retcode;
   sh_int x, y, map;
   sh_int sector;
   AFFECT_DATA *paf, *paf_next;
   SKILLTYPE *skill;
   TOWN_DATA *town;

   /* This little counter can be used to handle periodic events */
   cnt = (cnt + 1) % SECONDS_PER_TICK;

   for (ch = first_char; ch; ch = ch_next)
   {
      int pcheck = 0;
      set_cur_char(ch);
      ch_next = ch->next;
      will_fall(ch, 0);

      if (char_died(ch))
         continue;
      for (;;)
      {   
         if (sysdata.resetgame && !IS_NPC(ch) && time(0) - ch->pcdata->lastprankingcheck > 21600) //6 hours
         {
            if (ch->pcdata->lastprankingcheck == 0)
               ch->pcdata->lastprankingcheck = time(0);
            else
               ch->pcdata->lastprankingcheck += 21600;
            
            ch->pcdata->power_ranking += 2+(get_player_statlevel(ch)/5);
            pcheck++;
         }
         else
         {         
            if (pcheck)
               send_to_char("&w&WYou feel your Power grow&w\n\r", ch);
            break;
         }
      }
      if (!IS_NPC(ch) && ch->pcdata->duel_receive_time > 0 && time(0) >= ch->pcdata->duel_receive_time + 7201)
      {
         ch_printf(ch, "You lose %d Power Ranking points for your cowardness.\n\r", ch->pcdata->duel_receive_pranking);
         act(AT_RED, "$n failed to accept a duel, what a coward!", ch, NULL, ch, TO_MUD);
         ch->pcdata->power_ranking -= ch->pcdata->duel_receive_pranking;
         ch->pcdata->dual_receive_name = -1;
         ch->pcdata->duel_receive_pranking = 0;
         ch->pcdata->duel_receive_time = 0;
      }
      if (!IS_NPC(ch) && ch->pcdata->duel_offer_time > 0 && time(0) >= ch->pcdata->duel_offer_time + 7200)
      {
         DESCRIPTOR_DATA *d;
         
         for (d = first_descriptor; d; d = d->next)
         {
            if (d->character && d->character->pcdata->pid == ch->pcdata->duel_offer_name)
            {
               if (d->character->pcdata->duel_receive_time <= 0)
               {
                  send_to_char("You must of been logged off when the duel was removed, sorry.\n\r", ch);
                  ch->pcdata->duel_offer_name = -1;
                  ch->pcdata->duel_offer_pranking = 0;
                  ch->pcdata->duel_offer_time = 0;
                  break;
               }
               else
               {
                  ch_printf(d->character, "You lose %d Power Ranking points for your cowardness.\n\r", ch->pcdata->duel_offer_pranking);
                  ch_printf(ch, "Your challenge for a duel was denied, you are rewarded %d Power Ranking Points",
                     ch->pcdata->duel_offer_pranking);
                  act(AT_RED, "$N refused to duel $n, what a coward!.", d->character, NULL, ch, TO_MUD);
                  ch->pcdata->power_ranking += ch->pcdata->duel_offer_pranking;
                  d->character->pcdata->power_ranking -= ch->pcdata->duel_offer_pranking;
                  d->character->pcdata->dual_receive_name = -1;
                  d->character->pcdata->duel_receive_pranking = 0;
                  d->character->pcdata->duel_receive_time = 0;
                  ch->pcdata->duel_offer_name = -1;
                  ch->pcdata->duel_offer_pranking = 0;
                  ch->pcdata->duel_offer_time = 0;
                  break;
               }
            }
         }
         if (!d)
         {
            ch_printf(ch, "Your target is not online to validate your duel win, so you get the %d Points.\n\r",
               ch->pcdata->duel_offer_pranking);
            ch->pcdata->power_ranking += ch->pcdata->duel_offer_pranking;
            ch->pcdata->duel_offer_name = -1;
            ch->pcdata->duel_offer_pranking = 0;
            ch->pcdata->duel_offer_time = 0;
         }
      }
      if (sysdata.resetgame && !IS_NPC(ch) && ch->pcdata->twink_points >= 100 && !IS_IMMORTAL(ch))
      {
         char dirname[MSL];
         char sname[MSL];
         
         sprintf(sname, ch->name);
         send_to_char("You have matched or exceeded the max of 100 twink points.  You will now be deleted.\n\r", ch);       
         extract_char(ch, TRUE);
         
         sprintf(dirname, "%s%c/", PLAYER_DIR, LOWER(sname[0]));
         read_pfile2(dirname, sname);
         continue;
      }
      if (ch->position >= POS_STUNNED)
      {
         if (ch->hit < ch->max_hit)
            ch->hit += hit_gain(ch);

         if (ch->mana < ch->max_move || !IS_NPC(ch))
            ch->mana += mana_gain(ch);

         if (ch->move < ch->max_move)
            ch->move += move_gain(ch);
            
         if (!IS_NPC(ch))
         {
            if (ch->pcdata->hit_cnt > 0)
               ch->pcdata->hit_cnt -= hburn_gain(ch);               
            if (ch->pcdata->mana_cnt > 0)
               ch->pcdata->mana_cnt -= mburn_gain(ch);   
            if (ch->pcdata->hit_cnt < 0)
               ch->pcdata->hit_cnt = 0;
            if (ch->pcdata->mana_cnt < 0)
               ch->pcdata->mana_cnt = 0;
         }
      }
      //Rotting away for 5 minutes is not fun....Chance to recover from INCAP now
      if (ch->position == POS_MORTAL)
         ch->hit += hit_gain(ch);
      if (ch->position == POS_INCAP)
         ch->hit += hit_gain(ch);
      if (char_died(ch))
         continue;
      if (ch->position == POS_STUNNED)
         update_pos(ch);

      if (!IS_NPC(ch))
         update_interest(ch); //Updates bank interest
         
      if (!IS_NPC(ch))
         update_pkpower(ch); //Resets pkpower if pk stats was reset (see PKRESET flag in comm.c)
         
      if (!IS_NPC(ch) && IS_AFFECTED(ch, AFF_HIDE) && ch->position != POS_STANDING)
      {
         xREMOVE_BIT(ch->affected_by, AFF_HIDE);
         act(AT_RED, "$n silently comes into view.", ch, NULL, NULL, TO_ROOM);
      }  
      
      if (!IS_NPC(ch))
      {
         ch->pcdata->animate = 0;
         for (undead = ch->in_room->first_person; undead; undead = next_undead)
         {
            next_undead = undead->next_in_room;
            if (IS_NPC(undead) && undead->pIndexData->vnum == MOB_VNUM_ANIMATED_CORPSE && undead->coord->x == ch->coord->x 
            && undead->coord->y == ch->coord->y && undead->map == ch->map && IS_AFFECTED(undead, AFF_CHARM))
            {
               if (undead->master == ch)
               {
                  int count;
                  
                  count = GET_MASTERY(ch, skill_lookup("animate dead"), 0, 1);
                  if (ch->pcdata->animate >= count)
                  {
                     send_to_char("&r*********&G&WYou feel your hold on a corpse wither away.&r**********\n\r", ch);
                     act(AT_MAGIC, "$n returns to the dust from whence $e came.", undead, NULL, NULL, TO_ROOM);
                     if (IS_NPC(undead)) /* Guard against purging switched? */
                        extract_char(undead, TRUE);
                     continue;
                  }
                  else
                  {
                     ch->pcdata->animate++;
                  }
               }
            }
         }
      }
       /*
       * We need spells that have shorter durations than an hour.
       * So a melee round sounds good to me... -Thoric
       */
      for (paf = ch->first_affect; paf; paf = paf_next)
      {
         paf_next = paf->next;
         if (paf->duration > 0)
            paf->duration--;
         else if (paf->duration < 0)
            ;
         else
         {
            if (!paf_next || paf_next->type != paf->type || paf_next->duration > 0)
            {
               skill = get_skilltype(paf->type);
               if (paf->type > 0 && skill && skill->msg_off)
               {
                  set_char_color(AT_WEAROFF, ch);
                  send_to_char(skill->msg_off, ch);
                  send_to_char("\n\r", ch);
               }
            }
            if (paf->type == gsn_possess)
            {
               ch->desc->character = ch->desc->original;
               ch->desc->original = NULL;
               ch->desc->character->desc = ch->desc;
               ch->desc->character->switched = NULL;
               ch->desc = NULL;
            }
            affect_remove(ch, paf);
         }
      }

      if (char_died(ch))
         continue;

      if (IS_NPC(ch))
      {
         if ((cnt & 1))
            continue;

         //m1 - Resource AMT   m2 - Resource MAX   m3 - Cost   m4 - Kingdom   m5 - Resource Type   m6 - Extraction effic.
         //m9 - Time started
         if (IS_NPC(ch) && xIS_SET(ch->act, ACT_EXTRACTMOB) && ch->m4 > 1 && ch->m4 < sysdata.max_kingdom)
         {
            char logb[MIL];
            if (ch->m10 <= time(0) && ch->m10 != 0)
            {
               sprintf(logb, "%s's leaves because his scheduled service is over", ch->name);
               write_kingdom_logfile(ch->m4, logb, KLOG_SCHEDULE);
               extract_char(ch, TRUE);
               continue;
            }
            if (time(0) - ch->m9 >= cvttime(16200)) //1/4 GM (4.5 * longer now)
            {
               if ((town = get_town(kingdom_table[ch->m4]->dtown)) == NULL)
               {
                  sprintf(logb, "%s leaves you because of no default town", ch->name);
                  write_kingdom_logfile(ch->m4, logb, KLOG_EXTRACTION);
                  extract_char(ch, TRUE);
                  continue;
               } 
               if (town->coins < ch->m3/5)
               {
                  sprintf(logb, "%s leaves your service because of insufficient funding", ch->name);
                  write_kingdom_logfile(ch->m4, logb, KLOG_EXTRACTION);
                  extract_char(ch, TRUE);
                  continue;
               }
               else
               {
                  town->coins -= ch->m3/5;
                  ch->m9 += 16200;
                  sprintf(logb, "%s collects his pay of %d", ch->name, ch->m3/5);
                  write_kingdom_logfile(ch->m4, logb, KLOG_EXTRACTION);
               }
            }
         }
         if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY) && ch->m4 > 1 && ch->m4 < sysdata.max_kingdom)
         {
            char logb[MIL];
            
            if (time(0) - ch->m9 >= cvttime(64800)) //GL Month 18 RL Hours
            {
               if ((town = get_town(kingdom_table[ch->m4]->dtown)) == NULL)
               {
                  sprintf(logb, "%s leaves you because of no default town", ch->name);
                  write_kingdom_logfile(ch->m4, logb, KLOG_MIL_COLLECTION);
                  extract_char(ch, TRUE);
                  continue;
               } 
               if (town->coins < ch->m3 / 20)
               {
                  sprintf(logb, "%s leaves your service because of insufficient funding", ch->name);
                  bug("%s of kingdom %s leaves because of no funding.", ch->name, kingdom_table[ch->m4]->name);
                  write_kingdom_logfile(ch->m4, logb, KLOG_MIL_COLLECTION);
                  extract_char(ch, TRUE);
                  continue;
               }
               else
               {
                  town->coins -= ch->m3 / 20;
                  sprintf(logb, "%s collects his monthly pay of %d", ch->name, ch->m3/20);
                  bug("%s of %s collects his monthly pay of %d", ch->name, kingdom_table[ch->m4]->name, ch->m3/20);
                  write_kingdom_logfile(ch->m4, logb, KLOG_MIL_COLLECTION);
                  ch->m9 += 64800;
               }
            }
            if ((ch->pIndexData->vnum == MOB_KMAGE && !ch->fighting))
            {
               int num = number_range(1, 50);
               int cnt = 0;
               int a;
               int b;

               if (num > 40)
               {
                  CHAR_DATA *rch;

                  for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
                  {
                     if (in_same_room(ch, rch) && IS_NPC(rch) && rch->m4 == ch->m4)
                     {
                        if (num >= 41 && num <= 43 && !is_affected(rch, skill_lookup("slink")))
                           cnt++;
                        if (num >= 44 && num <= 45 && !is_affected(rch, skill_lookup("kindred strength")))
                           cnt++;
                        if (num >= 46 && num <= 47 && !is_affected(rch, skill_lookup("invis")))
                           cnt++;
                        if (num >= 48 && num <= 49 && !is_affected(rch, skill_lookup("detect invis")))
                           cnt++;
                        if (num == 50 && !is_affected(rch, skill_lookup("shockshield")))
                           cnt++;
                     }
                  }
                  if (cnt != 0)
                  {
                     a = number_range(1, cnt);
                     b = 0;
                     for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
                     {
                        if (in_same_room(ch, rch) && IS_NPC(rch) && rch->m4 == ch->m4)
                        {
                           if (num >= 41 && num <= 43 && !is_affected(rch, skill_lookup("slink")))
                              b++;
                           if (num >= 44 && num <= 45 && !is_affected(rch, skill_lookup("kindred strength")))
                              b++;
                           if (num >= 46 && num <= 47 && !is_affected(rch, skill_lookup("invis")))
                              b++;
                           if (num >= 48 && num <= 49 && !is_affected(rch, skill_lookup("detect invis")))
                              b++;
                           if (num == 50 && !is_affected(rch, skill_lookup("shockshield")))
                              b++;

                           if (a == b)
                              break;
                        }
                     }
                     if (rch)
                     {
                        if (num >= 41 && num <= 43)
                           spell_smaug(skill_lookup("slink"), ch->level, ch, rch);
                        if (num >= 44 && num <= 45)
                           spell_smaug(skill_lookup("kindred strength"), ch->level, ch, rch);
                        if (num >= 46 && num <= 47)
                           spell_invis(skill_lookup("invis"), ch->level, ch, rch);
                        if (num >= 48 && num <= 49)
                           spell_smaug(skill_lookup("detect invis"), ch->level, ch, rch);
                        if (num == 50)
                           spell_smaug(skill_lookup("shockshield"), ch->level, ch, rch);
                     }
                  }
               }
            }
         }  
         //Check for attack/warn triggers
         if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
         {
            if (xIS_SET(ch->miflags, KM_ATTACKE) || xIS_SET(ch->miflags, KM_ATTACKN)
               || xIS_SET(ch->miflags, KM_ATTACKA) || xIS_SET(ch->miflags, KM_WARN))
            {
               int stx, sty, endx, endy, dist, npeace, ht;
               CHAR_DATA *victim;
               char buf[MIL];
               CHAR_DATA *pvictim = NULL;
               int eqlevel = 8;

               dist = ch->m6;
               stx = ch->coord->x - dist;
               sty = ch->coord->y - dist;
               endx = ch->coord->x + dist;
               endy = ch->coord->y + dist;

               if (ch->coord->x < 1 && ch->coord->y < 1 && ch->map < 0)
                  stx = sty = endx = endy = -1;
               for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
               {
                  if (xIS_SET(ch->miflags, KM_WARN))
                  {
                     if (!IS_NPC(victim) && victim->coord->x >= stx &&
                        victim->coord->y >= sty && victim->coord->x <= endx && victim->coord->y <= endy && victim->map == ch->map)
                     {
                        if (!xIS_SET(victim->act, PLR_WARNED) && !IS_IMMORTAL(victim))
                        {
                           if (can_see(ch, victim) && ch->m4 != victim->pcdata->hometown &&
                              kingdom_table[ch->m4]->peace[victim->pcdata->hometown] < 2
                              && (!ch->midata || (ch->midata && (!ch->midata->command || ch->midata->command[0] == '\0'))))
                           {
                              sprintf(buf, "%s You are not welcome in the kingdom of %s, LEAVE NOW!", victim->name, kingdom_table[ch->m4]->name);
                              do_tell(ch, buf);
                              xSET_BIT(victim->act, PLR_WARNED);
                           }
                        }
                     }
                  }
                  else
                  {
                     if (ch->hunting || ch->hating)
                        break;
                     if (xIS_SET(ch->miflags, KM_ATTACKN))
                        npeace = 1;
                     else if (xIS_SET(ch->miflags, KM_ATTACKE))
                        npeace = 0;
                     else
                        npeace = -1;
                     if (IS_NPC(victim))
                        ht = victim->m4;
                     else
                        ht = victim->pcdata->hometown;

                     if (victim->coord->x >= stx && victim->coord->y >= sty
                        && victim->coord->x <= endx && victim->coord->y <= endy && victim->map == ch->map)
                     {
                        if (can_see_map(ch, victim) && !is_safe(ch, victim) && ch->m4 != ht 
                        && (npeace == -1 || kingdom_table[ch->m4]->peace[ht] <= npeace) && !IS_IMMORTAL(victim)
                        && !xIS_SET(victim->act, ACT_PACIFIST))
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
               if (pvictim)
               {
                  start_hating(ch, pvictim);
                  start_hunting(ch, pvictim);
                  set_command_buf(ch, "");
               }
            }
         }
         //Check for attackr/attackh triggers
         if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
         {
            int ht;
            CHAR_DATA *victim;
            if (xIS_SET(ch->miflags, KM_ATTACKC) || xIS_SET(ch->miflags, KM_ATTACKH))
            {
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
                           if (xIS_SET(ch->miflags, KM_ATTACKC))
                              do_say(ch, "Having a cloak will not be tolerated, you must die now!");
                           else if (xIS_SET(ch->miflags, KM_ATTACKH))
                              do_say(ch, "Having a hood on will not be tolerated, you must die now!");      
                           start_hating(ch, victim);
                           start_hunting(ch, victim);
                           set_command_buf(ch, "");
                           break;
                        }
                     }
                  }
               }
            }
         } 

         /* running mobs -Thoric */
         if (xIS_SET(ch->act, ACT_RUNNING))
         {
            if (!xIS_SET(ch->act, ACT_SENTINEL) && !ch->fighting && ch->hunting)
            {
               WAIT_STATE(ch, 6 * PULSE_VIOLENCE);
               if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
                  hunt_victim_map(ch);
               else
                  hunt_victim(ch);
               continue;
            }

            if (ch->spec_fun)
            {
               if ((*ch->spec_fun) (ch))
                  continue;
               if (char_died(ch))
                  continue;
            }


            if (!xIS_SET(ch->act, ACT_SENTINEL)
            && !ch->fighting
               && !xIS_SET(ch->act, ACT_PROTOTYPE)
               && (!xIS_SET(ch->act, ACT_MILITARY) ||
(xIS_SET(ch->act, ACT_MILITARY) && !xIS_SET(ch->miflags, KM_STATIONARY))) &&
(door = number_bits(4)) <= 9 && (pexit = get_exit(ch->in_room, door)) != NULL &&
pexit->to_room && !IS_SET(pexit->exit_info, EX_CLOSED) && !xIS_SET(pexit->to_room->room_flags, ROOM_FORGEROOM) && 
!xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB) && !xIS_SET(pexit->to_room->room_flags, ROOM_DEATH) && !xIS_SET(ch->act, ACT_MOUNTSAVE) && (!xIS_SET(ch->act, ACT_STAY_AREA) || pexit->to_room->area == ch->in_room->area))
            {
               retcode = move_char(ch, pexit, 0);
               if (char_died(ch))
                  continue;
               if (retcode != rNONE || xIS_SET(ch->act, ACT_SENTINEL)
                  || ch->position < POS_STANDING || (xIS_SET(ch->act, ACT_MILITARY) && !xIS_SET(ch->miflags, KM_STATIONARY)))
                  continue;
            }
         }
         continue;
      }
      else
      {
         if (ch->mount && !IN_SAME_ROOM(ch, ch->mount))
         {
            xREMOVE_BIT(ch->mount->act, ACT_MOUNTED);
            ch->mount = NULL;
            ch->position = POS_STANDING;
            send_to_char("No longer upon your mount, you fall to the ground...\n\rOUCH!\n\r", ch);
         }
         if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
         {
            x = y = map = -1;
            x = ch->coord->x;
            y = ch->coord->y;
            map = ch->map;
            sector = map_sector[map][x][y];

            if ((ch->in_room && sect_show[sector].sector == SECT_UNDERWATER) || (ch->in_room && sect_show[sector].sector == SECT_OCEANFLOOR))
            {
               if (!IS_AFFECTED(ch, AFF_AQUA_BREATH))
               {
                  if (ch->level < LEVEL_IMMORTAL)
                  {
                     int dam;

                     /* Changed level of damage at Brittany's request. -- Narn */
                     dam = number_range(ch->max_hit / 25, ch->max_hit / 10);
                     dam = UMAX(1, dam);
                     if (number_bits(2) == 0)
                        send_to_char("You cough and choke as you try to breathe water!\n\r", ch);
                     damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
                  }
               }
            }

            if (char_died(ch))
               continue;

            if (ch->in_room && sect_show[sector].sector == SECT_LAVA)
            {
               if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_AFFECTED(ch, AFF_FLOATING) && !ch->mount)
               {
                  if (ch->level < LEVEL_IMMORTAL)
                  {
                     int dam;

                     dam = number_range(ch->max_hit / 25, ch->max_hit / 10);
                     dam = UMAX(2, dam);
                     if (number_bits(2) == 0)
                        send_to_char("Your feet start to toast as they touch the lava.\n\r", ch);
                     damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
                  }
               }
            }

            if (char_died(ch))
               continue;

            if (ch->in_room && (sect_show[sector].sector == SECT_WATER_NOSWIM 
            || sect_show[sector].sector == SECT_OCEAN || sect_show[sector].sector == SECT_RIVER))
            {
               if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_AFFECTED(ch, AFF_FLOATING) && !IS_AFFECTED(ch, AFF_AQUA_BREATH) && !ch->mount
               &&  LEARNED(ch, gsn_swimming) <= 0)
               {
                  for (obj = ch->first_carrying; obj; obj = obj->next_content)
                     if (obj->item_type == ITEM_BOAT)
                        break;

                  if (!obj)
                  {
                     if (ch->level < LEVEL_IMMORTAL)
                     {
                        int mov;
                        int dam;

                        if (ch->move > 0)
                        {
                           mov = number_range(ch->max_move / 35, ch->max_move / 20);
                           mov = UMAX(1, mov);

                           if (ch->move - mov < 0)
                              ch->move = 0;
                           else
                              ch->move -= mov;
                              
                           if (number_bits(2) == 0)
                              send_to_char("&w&RYou struggle trying to keep from drowning, you better get out NOW!!!.\n\r", ch);
                              
                           if (ch->move == 0)
                           {
                              dam = number_range(ch->max_hit / 25, ch->max_hit / 10);
                              dam = UMAX(2, dam);

                              if (number_bits(2) == 0)
                                 send_to_char("&w&RStruggling with exhaustion, you choke on a mouthful of water.\n\r", ch);
                              damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
                           }
                        }
                     }
                  }
               }
               if (char_died(ch))
                  continue;
            }
         }
         else
         {
            if ((ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER) || (ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR))
            {
               if (!IS_AFFECTED(ch, AFF_AQUA_BREATH))
               {
                  if (ch->level < LEVEL_IMMORTAL)
                  {
                     int dam;

                     /* Changed level of damage at Brittany's request. -- Narn */
                     dam = number_range(ch->max_hit / 25, ch->max_hit / 10);
                     dam = UMAX(1, dam);
                     if (number_bits(2) == 0)
                        send_to_char("You cough and choke as you try to breathe water!\n\r", ch);
                     damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
                  }
               }
            }

            if (char_died(ch))
               continue;

            if (ch->in_room && ch->in_room->sector_type == SECT_LAVA)
            {
               if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_AFFECTED(ch, AFF_FLOATING) && !ch->mount)
               {
                  if (ch->level < LEVEL_IMMORTAL)
                  {
                     int dam;

                     dam = number_range(ch->max_hit / 25, ch->max_hit / 10);
                     dam = UMAX(1, dam);
                     if (number_bits(2) == 0)
                        send_to_char("Your feet start to toast as they touch the lava.\n\r", ch);
                     damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
                  }
               }
            }

            if (char_died(ch))
               continue;

            if (ch->in_room && ((ch->in_room->sector_type == SECT_WATER_NOSWIM) || (ch->in_room->sector_type == SECT_RIVER)))
            {
               if (!IS_AFFECTED(ch, AFF_FLYING) && !IS_AFFECTED(ch, AFF_FLOATING) && !IS_AFFECTED(ch, AFF_AQUA_BREATH) && !ch->mount
               &&  LEARNED(ch, gsn_swimming) <= 0)
               {
                  for (obj = ch->first_carrying; obj; obj = obj->next_content)
                     if (obj->item_type == ITEM_BOAT)
                        break;

                  if (!obj)
                  {
                     if (ch->level < LEVEL_IMMORTAL)
                     {
                        int mov;
                        int dam;

                        if (ch->move > 0)
                        {
                           mov = number_range(ch->max_move / 35, ch->max_move / 20);
                           mov = UMAX(1, mov);

                           if (ch->move - mov < 0)
                              ch->move = 0;
                           else
                              ch->move -= mov;
                              
                           if (number_bits(2) == 0)
                              send_to_char("&w&RYou struggle trying to keep from drowning, you better get out NOW!!!.\n\r", ch);
                        }
                        else
                        {
                           dam = number_range(ch->max_hit / 25, ch->max_hit / 10);
                           dam = UMAX(1, dam);

                           if (number_bits(2) == 0)
                              send_to_char("&w&RStruggling with exhaustion, you choke on a mouthful of water.\n\r", ch);
                           damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
                        }
                     }
                  }
               }
               if (char_died(ch))
                  continue;
            }

         }
         /* beat up on link dead players */
         if (!ch->desc)
         {
            CHAR_DATA *wch, *wch_next;

            for (wch = ch->in_room->first_person; wch; wch = wch_next)
            {
               wch_next = wch->next_in_room;

               if (!IS_NPC(wch)
                  || wch->fighting
                  || IS_AFFECTED(wch, AFF_CHARM) || !IS_AWAKE(wch) || (xIS_SET(wch->act, ACT_WIMPY) && IS_AWAKE(ch)) || !can_see(wch, ch))
                  continue;
                  
               if (IS_AFFECTED(wch, AFF_WEB) || IS_AFFECTED(wch, AFF_SNARE))
                  continue;
       

               if (is_hating(wch, ch))
               {
                  found_prey(wch, ch);
                  continue;
               }

               if ((!xIS_SET(wch->act, ACT_AGGRESSIVE) && !xIS_SET(wch->act, ACT_META_AGGR)) || xIS_SET(wch->act, ACT_MOUNTED) || is_room_safe(wch))
                  continue;
               global_retcode = one_hit(wch, ch, TYPE_UNDEFINED, LM_BODY);
            }
         }
      }
   }
}


/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
void aggr_update(void)
{
   DESCRIPTOR_DATA *d, *dnext;
   CHAR_DATA *wch;
   CHAR_DATA *ch;
   CHAR_DATA *ch_next;
   CHAR_DATA *vch;
   CHAR_DATA *vch_next;
   CHAR_DATA *victim;
   int lvl;
   struct act_prog_data *apdtmp;

#ifdef UNDEFD
   /*
    *  GRUNT!  To do
    *
    */
   if (IS_NPC(wch) && wch->mpactnum > 0 && wch->in_room->area->nplayer > 0)
   {
      MPROG_ACT_LIST *tmp_act, *tmp2_act;

      for (tmp_act = wch->mpact; tmp_act; tmp_act = tmp_act->next)
      {
         oprog_wordlist_check(tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG);
         DISPOSE(tmp_act->buf);
      }
      for (tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act)
      {
         tmp2_act = tmp_act->next;
         DISPOSE(tmp_act);
      }
      wch->mpactnum = 0;
      wch->mpact = NULL;
   }
#endif

   /* check mobprog act queue */
   while ((apdtmp = mob_act_list) != NULL)
   {
      wch = mob_act_list->vo;
      if (!char_died(wch) && wch->mpactnum > 0)
      {
         MPROG_ACT_LIST *tmp_act;

         while ((tmp_act = wch->mpact) != NULL)
         {
            if (tmp_act->obj && obj_extracted(tmp_act->obj))
               tmp_act->obj = NULL;
            if (tmp_act->ch && !char_died(tmp_act->ch))
               mprog_wordlist_check(tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG);
            wch->mpact = tmp_act->next;
            DISPOSE(tmp_act->buf);
            DISPOSE(tmp_act);
         }
         wch->mpactnum = 0;
         wch->mpact = NULL;
      }
      mob_act_list = apdtmp->next;
      DISPOSE(apdtmp);
   }


   /*
    * Just check descriptors here for victims to aggressive mobs
    * We can check for linkdead victims in char_check -Thoric
    */
   for (d = first_descriptor; d; d = dnext)
   {
      dnext = d->next;
      if (d->connected != CON_PLAYING || (wch = d->character) == NULL)
         continue;

      if (char_died(wch) || IS_NPC(wch) || wch->level >= LEVEL_IMMORTAL || !wch->in_room)
         continue;

      for (ch = wch->in_room->first_person; ch; ch = ch_next)
      {
         int count;

         ch_next = ch->next_in_room;

         if (!IS_NPC(ch)
            || ch->fighting || IS_AFFECTED(ch, AFF_CHARM) || !IS_AWAKE(ch) || (xIS_SET(ch->act, ACT_WIMPY) && IS_AWAKE(wch)) || !can_see(ch, wch))
            continue;
            
         if (IS_AFFECTED(ch, AFF_WEB) || IS_AFFECTED(ch, AFF_SNARE))
            continue;

         if (is_hating(ch, wch))
         {
            found_prey(ch, wch);
            continue;
         }

         if ((!xIS_SET(ch->act, ACT_AGGRESSIVE) && !xIS_SET(ch->act, ACT_META_AGGR)) || xIS_SET(ch->act, ACT_MOUNTED) || is_room_safe(ch))
            continue;

         /*
          * Ok we have a 'wch' player character and a 'ch' npc aggressor.
          * Now make the aggressor fight a RANDOM pc victim in the room,
          *   giving each 'vch' an equal chance of selection.
          *
          * Depending on flags set, the mob may attack another mob
          */
         count = 0;
         victim = NULL;
         for (vch = wch->in_room->first_person; vch; vch = vch_next)
         {
            vch_next = vch->next_in_room;

            if ((!IS_NPC(vch) || xIS_SET(ch->act, ACT_META_AGGR)
                  || xIS_SET(vch->act, ACT_ANNOYING))
               && vch->level < LEVEL_IMMORTAL && (!xIS_SET(ch->act, ACT_WIMPY) || !IS_AWAKE(vch)) && can_see(ch, vch))
            {
               if (number_range(0, count) == 0)
                  victim = vch;
               count++;
            }
         }

         if (!victim)
         {
            bug("Aggr_update: null victim.", count);
            continue;
         }

         /* backstabbing mobs (Thoric) */
         if (IS_NPC(ch) && xIS_SET(ch->attacks, ATCK_BACKSTAB))
         {
            OBJ_DATA *obj;

            if (!ch->mount
               && (obj = get_eq_char(ch, WEAR_WIELD)) != NULL
               && wielding_skill_weapon(ch, 0) == 7 && !victim->fighting && victim->hit >= victim->max_hit)
            {
               lvl = POINT_LEVEL(LEARNED(ch, gsn_backstab), MASTERED(ch, gsn_backstab));
               check_attacker(ch, victim);
               if (!IS_AWAKE(victim) || number_percent() < URANGE(5, lvl/2, 30))
               {
                  global_retcode = one_hit(ch, victim, gsn_backstab, LM_BODY);
                  continue;
               }
               else
               {
                  global_retcode = damage(ch, victim, 0, gsn_backstab, 0, -1);
                  continue;
               }
            }
         }
         global_retcode = one_hit(ch, victim, TYPE_UNDEFINED, LM_BODY);
      }
   }

   return;
}

/* From interp.c */
bool check_social args((CHAR_DATA * ch, char *command, char *argument));

/*
 * drunk randoms	- Tricops
 * (Made part of mobile_update	-Thoric)
 */
void drunk_randoms(CHAR_DATA * ch)
{
   CHAR_DATA *rvch = NULL;
   CHAR_DATA *vch;
   sh_int drunk;
   sh_int position;

   if (IS_NPC(ch) || ch->pcdata->condition[COND_DRUNK] <= 0)
      return;

   if (number_percent() < 30)
      return;

   drunk = ch->pcdata->condition[COND_DRUNK];
   position = ch->position;
   ch->position = POS_STANDING;

   if (number_percent() < (2 * drunk / 20))
      check_social(ch, "burp", "");
   else if (number_percent() < (2 * drunk / 20))
      check_social(ch, "hiccup", "");
   else if (number_percent() < (2 * drunk / 20))
      check_social(ch, "drool", "");
   else if (number_percent() < (2 * drunk / 20))
      check_social(ch, "fart", "");
   else if (drunk > (10 + (get_curr_con(ch) / 5)) && number_percent() < (2 * drunk / 18))
   {
      for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
         if (number_percent() < 10)
            rvch = vch;
      check_social(ch, "puke", (rvch ? rvch->name : ""));
   }

   ch->position = position;
   return;
}

/*
 * Random hallucinations for those suffering from an overly high mentalstate
 * (Hats off to Albert Hoffman's "problem child")	-Thoric
 */
void hallucinations(CHAR_DATA * ch)
{
   if (ch->mental_state >= 30 && number_bits(5 - (ch->mental_state >= 50) - (ch->mental_state >= 75)) == 0)
   {
      char *t;

      switch (number_range(1, UMIN(21, (ch->mental_state + 5) / 5)))
      {
         default:
         case 1:
            t = "You feel very restless... you can't sit still.\n\r";
            break;
         case 2:
            t = "You're tingling all over.\n\r";
            break;
         case 3:
            t = "Your skin is crawling.\n\r";
            break;
         case 4:
            t = "You suddenly feel that something is terribly wrong.\n\r";
            break;
         case 5:
            t = "Those damn little fairies keep laughing at you!\n\r";
            break;
         case 6:
            t = "You can hear your mother crying...\n\r";
            break;
         case 7:
            t = "Have you been here before, or not?  You're not sure...\n\r";
            break;
         case 8:
            t = "Painful childhood memories flash through your mind.\n\r";
            break;
         case 9:
            t = "You hear someone call your name in the distance...\n\r";
            break;
         case 10:
            t = "Your head is pulsating... you can't think straight.\n\r";
            break;
         case 11:
            t = "The ground... seems to be squirming...\n\r";
            break;
         case 12:
            t = "You're not quite sure what is real anymore.\n\r";
            break;
         case 13:
            t = "It's all a dream... or is it?\n\r";
            break;
         case 14:
            t = "You hear your grandchildren praying for you to watch over them.\n\r";
            break;
         case 15:
            t = "They're coming to get you... coming to take you away...\n\r";
            break;
         case 16:
            t = "You begin to feel all powerful!\n\r";
            break;
         case 17:
            t = "You're light as air... the heavens are yours for the taking.\n\r";
            break;
         case 18:
            t = "Your whole life flashes by... and your future...\n\r";
            break;
         case 19:
            t = "You are everywhere and everything... you know all and are all!\n\r";
            break;
         case 20:
            t = "You feel immortal!\n\r";
            break;
         case 21:
            t = "Ahh... the power of a Supreme Entity... what to do...\n\r";
            break;
      }
      send_to_char(t, ch);
   }
   return;
}
void spread_fire(int num, int x, int y, int map)
{
   if (num == 4 || num == 5)
      return;
   if (number_range(1, 100) > 70)
   {
      x = get_x(x, num);
      y = get_y(y, num);

      if (map_sector[map][x][y] != SECT_FIRE)
      {
         map_sector[map][x][y] = SECT_FIRE;
         resource_sector[map][x][y] = 500;
      }
   }
}

#define KRES_UNKNOWN -1
#define KRES_GOLD 1
#define KRES_IRON 2
#define KRES_CORN 3
#define KRES_GRAIN 4
#define KRES_LUMBER 5
#define KRES_STONE 6
#define KRES_FISH 7

int add_more_resource(int type, int amount)
{
   if (type == KRES_CORN || type == KRES_GRAIN)
   {
      if (amount < 100)
         return 2;
      else if (amount < 200)
         return 11;
      else if (amount < 500)
         return 33;
      else if (amount < 1150)
         return 55;
      else
         return 88;
   }
   if (type == KRES_LUMBER || type == KRES_STONE)
   {
      if (amount < 1000)
         return 1;
      else if (amount < 2000)
         return 2;
      else if (amount < 3000)
         return 5;
      else if (amount < 4200)
         return 9;
      else
         return 13;
   }
   if (type == KRES_FISH)
   {
      if (amount < 800)
         return 20;
      if (amount < 1500)
         return 40;
      else
         return 100;
   }
      
   return 0;
}

void save_sysdata args((SYSTEM_DATA sys));

/* Checks every second, but typically only fires ever 30 minutes or so */
void resource_population_check(void)
{
   int sector, resource;
   sh_int rtype = 0;
   int x, y;
   int start, finish;


   if (sysdata.lastrescheck == 0)
      sysdata.lastrescheck = time(0);
   /*
   if (sysdata.lastpopcheck == 0)
      sysdata.lastpopcheck = time(0);
   if (sysdata.lasttaxcheck == 0)
      sysdata.lasttaxcheck = time(0); */

   //if (res_rotation > 0 || (time(0) - sysdata.lastrescheck) >= 1944) //32.4 minutes RL
   if (res_rotation > 0 || (time(0) - sysdata.lastrescheck) >= cvttime(2160)) //36 minutes RL  1 DAY GT
   {
      int month;

      month = getday() / 30;

      if (res_rotation <= 0)
      {
         sysdata.lastrescheck += cvttime(2160);
         res_rotation = 1;
      }
      start = (res_rotation * 300) - 299;
      finish = res_rotation * 300;
      for (x = start; x <= finish; x++)
      {
         for (y = 1; y <= MAX_Y; y++)
         {
            resource = resource_sector[0][x][y];
            sector = map_sector[0][x][y];
            rtype = 0;
            if (sector == SECT_MOUNTAIN && resource_sector[0][x][y] > 0)
            {
               resource_sector[0][x][y]--;
               if (resource_sector[0][x][y] == 0)
               {
                  map_sector[0][x][y] = SECT_PLAINS;
               }
               continue;
            }
            if ((sector == SECT_SHORE || sector == SECT_DESERT) && resource_sector[0][x][y] > 0)
            {
               resource_sector[0][x][y]--;
               if (resource_sector[0][x][y] == 0)
               {
                  map_sector[0][x][y] = SECT_PLAINS;
               }
               continue;
            }            

            if (sector == SECT_MINEGOLD || sector == SECT_SGOLD || sector == SECT_NGOLD)
            {
               if (dice(1, 5) >= 3)
                  resource += 1;
               rtype = KRES_GOLD;
            }
            if (sector == SECT_MINEIRON || sector == SECT_SIRON || sector == SECT_NIRON)
            {
               if (dice(1, 5) >= 3)
                  resource += 1;
               rtype = KRES_IRON;
            }
            if (sector == SECT_HCORN || sector == SECT_SCORN || sector == SECT_NCORN)
            {
               rtype = KRES_CORN;
               if (month >= 3 && month <= 6)
                  resource += add_more_resource(rtype, resource);
               else
                  resource -= 130;
            }
            if (sector == SECT_HGRAIN || sector == SECT_SGRAIN || sector == SECT_NGRAIN)
            {
               rtype = KRES_GRAIN;
               if (month >= 5 && month <= 8)
                  resource += add_more_resource(rtype, resource);
               else
                  resource -= 130;
            }
            if (sector == SECT_FOREST || sector == SECT_STREE || sector == SECT_NTREE)
            {
               rtype = KRES_LUMBER;
               resource += add_more_resource(rtype, resource);
            }
            if (sector == SECT_STONE || sector == SECT_SSTONE || sector == SECT_NSTONE)
            {
               rtype = KRES_STONE;
               resource += add_more_resource(rtype, resource);
            }
            if (sector == SECT_RIVER || sector == SECT_WATER_NOSWIM)
            {
               rtype = KRES_FISH;
               resource += add_more_resource(rtype, resource);
            }
            if (rtype == KRES_GOLD || rtype == KRES_IRON || rtype == KRES_FISH)
            {
               if (resource > 3000)
                  resource = 3000;
               if (resource < 0)
                  resource = 0;
            }
            if (rtype == KRES_CORN || rtype == KRES_GRAIN || rtype == KRES_LUMBER || rtype == KRES_STONE)
            {
               if (resource > 6000)
                  resource = 6000;
               if (resource < 0)
                  resource = 0;
            }
            if (rtype == KRES_CORN || rtype == KRES_GRAIN || rtype == KRES_LUMBER || rtype == KRES_STONE)
            {
               if (resource > 0)
               {
                  if (rtype == KRES_CORN)
                     sector = SECT_SCORN;
                  if (rtype == KRES_GRAIN)
                     sector = SECT_SGRAIN;
                  if (rtype == KRES_LUMBER)
                     sector = SECT_STREE;
                  if (rtype == KRES_STONE)
                     sector = SECT_SSTONE;
               }
               if (resource >= 2400)
               {
                  if (rtype == KRES_CORN)
                     sector = SECT_HCORN;
                  if (rtype == KRES_GRAIN)
                     sector = SECT_HGRAIN;
                  if (rtype == KRES_LUMBER)
                     sector = SECT_FOREST;
                  if (rtype == KRES_STONE)
                     sector = SECT_STONE;
               }
            }
            if (rtype == KRES_GOLD || rtype == KRES_IRON)
            {
               if (resource > 0)
               {
                  if (rtype == KRES_GOLD)
                     sector = SECT_SGOLD;
                  if (rtype == KRES_IRON)
                     sector = SECT_SIRON;
               }
               if (resource >= 1200)
               {
                  if (rtype == KRES_GOLD)
                     sector = SECT_MINEGOLD;
                  if (rtype == KRES_IRON)
                     sector = SECT_MINEIRON;
               }
            }
            if (rtype == KRES_CORN || rtype == KRES_GRAIN)
            {
               if (resource < 2400)
               {
                  if (rtype == KRES_CORN)
                     sector = SECT_SCORN;
                  if (rtype == KRES_GRAIN)
                     sector = SECT_SGRAIN;
               }
               if (resource < 1)
               {
                  if (rtype == KRES_CORN)
                     sector = SECT_NCORN;
                  if (rtype == KRES_GRAIN)
                     sector = SECT_NGRAIN;
               }
            }
            resource_sector[0][x][y] = resource;
            map_sector[0][x][y] = sector;
         }
      }
      if (res_rotation != 5)
         res_rotation++;
      else
         res_rotation = 0;
   } 
}

void who_update(void)
{ /*
     FILE *fpout;
     char buf[MSL];
     char filename[MIL];

     sprintf( filename, "%swho.txt", PPAGE_DIR);
     fpout=fopen(filename, "w");

     fprintf( fpout, "Testing this stupid thing.\n");
     fclose( fpout);  */
}
void tele_update(void)
{
   TELEPORT_DATA *tele, *tele_next;

   if (!first_teleport)
      return;

   for (tele = first_teleport; tele; tele = tele_next)
   {
      tele_next = tele->next;
      if (--tele->timer <= 0)
      {
         if (tele->room->first_person)
         {
            if (xIS_SET(tele->room->room_flags, ROOM_TELESHOWDESC))
               teleport(tele->room->first_person, tele->room->tele_vnum, TELE_SHOWDESC | TELE_TRANSALL);
            else
               teleport(tele->room->first_person, tele->room->tele_vnum, TELE_TRANSALL);
         }
         UNLINK(tele, first_teleport, last_teleport, next, prev);
         DISPOSE(tele);
      }
   }
}

#if FALSE
/* 
 * Write all outstanding authorization requests to Log channel - Gorog
 */
void auth_update(void)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char log_buf[MIL];
   bool first_time = TRUE; /* so titles are only done once */

   for (d = first_descriptor; d; d = d->next)
   {
      victim = d->character;
      if (victim && IS_WAITING_FOR_AUTH(victim))
      {
         if (first_time)
         {
            first_time = FALSE;
            strcpy(log_buf, "Pending authorizations:");
            /*log_string( log_buf ); */
            to_channel(log_buf, CHANNEL_AUTH, "Auth", 1);
         }
         sprintf(log_buf, " %s@%s new %s", victim->name,
            victim->desc->host, race_table[victim->race]->race_name);
/*         log_string( log_buf ); */
         to_channel(log_buf, CHANNEL_AUTH, "Auth", 1);
      }
   }
}
#endif

void auth_update(void)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char buf[MIL], log_buf[MAX_INPUT_LENGTH];
   bool found_hit = FALSE; /* was at least one found? */

   strcpy(log_buf, "Pending authorizations:\n\r");
   for (d = first_descriptor; d; d = d->next)
   {
      if ((victim = d->character) && IS_WAITING_FOR_AUTH(victim))
      {
         found_hit = TRUE;
         sprintf(buf, " %s@%s new %s\n\r", victim->name,
            victim->desc->host, race_table[victim->race]->race_name);
         strcat(log_buf, buf);
      }
   }
   if (found_hit)
   {
/*	log_string( log_buf ); */
      to_channel(log_buf, CHANNEL_AUTH, "Auth", 1);
   }
}

void entrance_update(void)
{
   for (;;)
   {
      global_x[0] = number_range(1, MAX_X);
      global_y[0] = number_range(1, MAX_Y);
      global_map[0] = number_range(0, MAP_MAX - 1);
      if (global_x[0] >= 200 && global_x[0] <= 300)
         continue;
      if (global_y[0] >= 200 && global_y[0] <= 300)
         continue;
      if (sect_show[(int)map_sector[global_map[0]][global_x[0]][global_y[0]]].canpass == FALSE)
         continue;
      if (map_sector[global_map[0]][global_x[0]][global_y[0]] == SECT_EXIT)
         continue;
      break;
   }
}

int check_equiped(CHAR_DATA * ch)
{
   int neq = 0;
   int x;

   for (x = 0; x < MAX_WEAR; x++)
   {
      if (x == WEAR_NONE || x == WEAR_LIGHT || x == WEAR_WIELD || x == WEAR_DUAL_WIELD
         || x == WEAR_MISSILE_WIELD || x == WEAR_LODGE_RIB || x == WEAR_LODGE_ARM || x == WEAR_LODGE_LEG || x == WEAR_SHIELD)
      {
         continue;
      }
      else
      {
         if (get_eq_char(ch, x) == NULL)
            neq++;
      }
   }
   return neq;
}

void tornado_update(void)
{
   TORNADO_DATA *torna;
   TORNADO_DATA *tnext;
   CHAR_DATA *ch;
   ROOM_INDEX_DATA *room;
   int dir;
   int str;
   int startr;
   int endr;
   int vnum;
   int x = 0;
   int y = 0;

   startr = OVERLAND_SOLAN;
   endr = OVERLAND_SOLAN;

   for (torna = first_tornado; torna; torna = tnext)
   {
      int ddir;
      int tdir = 0;

      tnext = torna->next;

      if (torna->turns > 0)
      {
         ddir = number_range(1, 3);
         if (torna->dir == 0)
         {
            if (ddir == 1)
               tdir = 9;
            else if (ddir == 2)
               tdir = 0;
            else
               tdir = 6;
         }
         if (torna->dir == 1)
         {
            if (ddir == 1)
               tdir = 6;
            else if (ddir == 2)
               tdir = 1;
            else
               tdir = 7;
         }
         if (torna->dir == 2)
         {
            if (ddir == 1)
               tdir = 7;
            else if (ddir == 2)
               tdir = 2;
            else
               tdir = 8;
         }
         if (torna->dir == 3)
         {
            if (ddir == 1)
               tdir = 8;
            else if (ddir == 2)
               tdir = 3;
            else
               tdir = 9;
         }
         if (tdir == 9 || tdir == 0 || tdir == 6)
            torna->y--;
         if (tdir == 8 || tdir == 2 || tdir == 7)
            torna->y++;
         if (tdir == 9 || tdir == 3 || tdir == 8)
            torna->x--;
         if (tdir == 6 || tdir == 1 || tdir == 7)
            torna->x++;
            
         check_torn_damage(torna->x, torna->y, torna->map);
         torna->turns--;
      }
      else
      {
         UNLINK(torna, first_tornado, last_tornado, next, prev);
         DISPOSE(torna);
         continue;
      }
   }

   for (torna = first_tornado; torna; torna = torna->next)
   {
      for (vnum = startr; vnum <= endr; vnum++)
      {
         room = get_room_index(vnum);
         for (ch = room->first_person; ch; ch = ch->next_in_room)
         {
            if (torna->map == ch->map)
            {
               //Throwing time, oh yeah
               if (get_distform(ch->coord->x, ch->coord->y, torna->x, torna->y) <= 5)
               {
                  dir = get_curr_dir(ch->coord->x, torna->x, ch->coord->y, torna->y);
                  str = torna->power * 2.3;
                  if (dir == 0)
                  {
                     y = -6;
                  }
                  if (dir == 1)
                  {
                     y = -4;
                     x = 2;
                  }
                  if (dir == 2)
                  {
                     y = -3;
                     x = 3;
                  }
                  if (dir == 3)
                  {
                     y = -2;
                     x = 4;
                  }
                  if (dir == 4)
                  {
                     x = 6;
                  }
                  if (dir == 5)
                  {
                     x = 4;
                     y = 2;
                  }
                  if (dir == 6)
                  {
                     x = 3;
                     y = 3;
                  }
                  if (dir == 7)
                  {
                     x = 2;
                     y = 4;
                  }
                  if (dir == 8)
                  {
                     y = 6;
                  }
                  if (dir == 9)
                  {
                     y = 4;
                     x = -2;
                  }
                  if (dir == 10)
                  {
                     y = 3;
                     x = -3;
                  }
                  if (dir == 11)
                  {
                     y = 2;
                     x = -4;
                  }
                  if (dir == 12)
                  {
                     x = -6;
                  }
                  if (dir == 13)
                  {
                     x = -4;
                     y = -2;
                  }
                  if (dir == 14)
                  {
                     x = -3;
                     y = -3;
                  }
                  if (dir == 15)
                  {
                     x = -2;
                     y = -4;
                  }
                  x = (x * str / 6);
                  y = (y * str / 6);

                  x = number_range(x - 3, x + 3);
                  y = number_range(y - 3, y + 3);
                  send_to_char("&OA strong gust of wind from a Tornado sends you flying through the air.\n\r", ch);
                  ch->coord->x = URANGE(1, ch->coord->x + x, MAX_X);
                  ch->coord->y = URANGE(1, ch->coord->y + y, MAX_Y);
                  damage(ch, ch, number_range(str * 7, str * 15), TYPE_UNDEFINED, 0, -1);
                  if (char_died(ch))
                     continue;
                  do_look(ch, "");
               }
            }
         }
      }
   }
}

void wilderness_save(void)
{
   int x;
   AREA_DATA *tarea;
   
   if (sav_rotation > 3)
      sav_rotation = 1;
   
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      write_kingdom_file(x);
   }
   write_kingdom_list();
   if (sav_rotation == 1)
   {
      for (tarea = first_area; tarea; tarea = tarea->next)
      {
         if (tarea->kingdom > 1)
            fold_area(tarea, tarea->filename, FALSE, 1);
      }
   }
   if (sav_rotation == 2)
   {
      save_resources("solan", 0);
      save_sysdata(sysdata);
   }
   if (sav_rotation == 3)
   {
      save_map("solan", 0);
   }
   sav_rotation++;
}

//Weather Info Update, sends information to Players about the weather
void winfo_update(void)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ch;
   TORNADO_DATA *torna;
   int p;
   int temp;
   int inside = 0;

   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2 && d->character->map == mweather)
      {
         ch = d->character;
         if (!IN_WILDERNESS(ch))
         {
            if (ch->in_room->area->map < 0 || ch->in_room->area->x < 1 || ch->in_room->area->y < 1)
               continue;
            else
            {
               p = weather_sector[ch->in_room->area->map][ch->in_room->area->x][ch->in_room->area->y] % 10;
               temp = generate_temperature(ch, ch->in_room->area->x, ch->in_room->area->y, ch->in_room->area->map);
               if (!IS_OUTSIDE(ch) || NO_WEATHER_SECT(ch->in_room->sector_type))
               {
                  if (!IS_OUTSIDE(ch) && !NO_WEATHER_SECT(ch->in_room->sector_type)) //Indoors/Tunnel
                  {
                     if (p < 6)
                        continue;

                     inside = 1;
                  }
                  else
                     continue;
               }
            }
         }
         else
         {
            p = weather_sector[ch->map][ch->coord->x][ch->coord->y] % 10;
            temp = generate_temperature(ch, -1, -1, -1);
         }

         for (torna = first_tornado; torna; torna = torna->next)
         {
            int x;
            int y;
            int map;
            int dir;
            char dbuf[25];

            if (!IN_WILDERNESS(ch))
            {
               x = ch->in_room->area->x;
               y = ch->in_room->area->y;
               map = ch->in_room->area->map;
            }
            else
            {
               x = ch->coord->x;
               y = ch->coord->y;
               map = ch->map;
            }
            if (ch->map == torna->map && abs(torna->x - ch->coord->x) <= 50 && abs(torna->y - ch->coord->y <= 50))
            {
               dir = get_curr_dir(x, torna->x, y, torna->y);
               sprintf(dbuf, "%s", owindd[dir]);

               set_char_color(AT_ORANGE, ch);
               if (((abs(torna->x - ch->coord->x) + abs(torna->y - ch->coord->y)) / 2) <= 10)
                  ch_printf(ch, "The Blistering winds of a Tornado from the %s are VERY CLOSE to you.\n\r", dbuf);
               else if (((abs(torna->x - ch->coord->x) + abs(torna->y - ch->coord->y)) / 2) <= 20)
                  ch_printf(ch, "The howling winds of a Tornado from the %s is getting VERY CLOSE to you.\n\r", dbuf);
               else if (((abs(torna->x - ch->coord->x) + abs(torna->y - ch->coord->y)) / 2) <= 30)
                  ch_printf(ch, "The strong winds of a Tornado from the %s is just a short distance from you.\n\r", dbuf);
               else if (((abs(torna->x - ch->coord->x) + abs(torna->y - ch->coord->y)) / 2) <= 40)
                  ch_printf(ch, "The winds start to pick up as a Tornado from the %s inches closer to your location.\n\r", dbuf);
               else
                  ch_printf(ch, "The feight sounds of a Tornado can be heard from the %s.\n\r", dbuf);
            }
         }
         if (p > 0)
         {
            if (temp >= 20 && temp <= 35) //Snow
            {
               if (!xIS_SET(ch->act, PLR_NOWEATHER))
               {
                  set_char_color(AT_WHITE, ch);
                  if (p == 1)
                     send_to_char("A snowflake or two fall lightly to the ground.\n\r", ch);
                  if (p == 2)
                     send_to_char("A few snowflakes are falling from the sky.\n\r", ch);
                  if (p == 3 || p == 4)
                     send_to_char("It is snowing lightly.\n\r", ch);
                  if (p == 5 || p == 6)
                     send_to_char("Snow is falling steadily from the sky.\n\r", ch);
                  if (p == 7)
                  {
                     if (inside == 0)
                        send_to_char("Snow is falling steadily and rapidly from the sky.\n\r", ch);
                     else
                        send_to_char("Snow is falling steadily and rapidly outside of this room.\n\r", ch);
                  }
                  if (p == 8)
                  {
                     if (inside == 0)
                        send_to_char("A strong blizzard is passing through the area deliving large amounts of snow\n\r", ch);
                     else
                        send_to_char("A strong blizzard is dumping large amounts of snow right outside this room.\n\r", ch);
                  }
                  if (p == 9)
                  {
                     if (inside == 0)
                        send_to_char("A blistering blizzard is covering all the area in a thick layer of snow.\n\r", ch);
                     else
                        send_to_char("The sounds of swirling winds and a feverous blizzard rocks through the room.\n\r", ch);
                  }
               }
            }
            if (temp < 20) //Ice
            {
               if (!xIS_SET(ch->act, PLR_NOWEATHER))
               {
                  set_char_color(AT_WHITE, ch);
                  if (p == 1)
                     send_to_char("One or two very small ball of ice are falling from the sky.\n\r", ch);
                  if (p == 2)
                     send_to_char("A few very small balls of ice are coming along with the very light rain.\n\r", ch);
                  if (p == 3 || p == 4)
                     send_to_char("Balls of ice the size of snow flakes are coming down lightly.\n\r", ch);
                  if (p == 5 || p == 6)
                     send_to_char("A steady flow of small peices of ice are coming from the sky.\n\r", ch);
                  if (p == 7)
                  {
                     if (inside == 0)
                        send_to_char("Medium size peices of ice are falling all around you.\n\r", ch);
                     else
                        send_to_char("The medium chunks of ice are making slight amounts of noise as they strike the ceiling.\n\r", ch);
                  }
                  if (p == 8)
                  {
                     if (inside == 0)
                        send_to_char("Good size chunks of ice are blitzing the ground, you might want to seek shelture.\n\r", ch);
                     else
                        send_to_char("Large chunks of ice are making it sound like it is raining arrows on your ceiling.\n\r", ch);
                  }
                  if (p == 9)
                  {
                     if (inside == 0)
                        send_to_char("Large chunks of ice are blitzing the ground, chunks big enough to kill a man.\n\r", ch);
                     else
                        send_to_char("The whole room is echoing intensely with the sound of ice blitzing the ceiling.\n\r", ch);
                  }
               }

               if ((get_trust(ch) > 5 && get_trust(ch) < LEVEL_IMMORTAL) && inside == 0 &&
                  p > 4 && (get_eq_char(ch, WEAR_BODY) == NULL ||
get_eq_char(ch, WEAR_HEAD) == NULL ||
get_eq_char(ch, WEAR_LEG_L) == NULL || get_eq_char(ch, WEAR_LEG_R) == NULL || get_eq_char(ch, WEAR_ARM_L) == NULL || get_eq_char(ch, WEAR_ARM_R) == NULL || get_eq_char(ch, WEAR_WAIST) == NULL || get_eq_char(ch, WEAR_ABOUT_NECK) == NULL))
               {
                  set_char_color(AT_RED, ch);
                  send_to_char("A large chunk of ice strikes an unequiped area of your body and hurts you.\n\r", ch);
                  damage(ch, ch, (p - 4) * 10, TYPE_UNDEFINED, 0, -1);
                  if (char_died(ch))
                     continue;
               }
            }
            if (temp > 35) //Rain
            {
               if (!xIS_SET(ch->act, PLR_NOWEATHER))
               {
                  set_char_color(AT_LBLUE, ch);
                  if (temp > 85)
                  {
                     if (p == 1)
                        send_to_char("Small cold drops of rain fall around you.\n\r", ch);
                     if (p == 2)
                        send_to_char("A very light refreshing rain is cooling things down.\n\r", ch);
                     if (p == 3 || p == 4)
                        send_to_char("A light rain is cooling down the warm weather.\n\r", ch);
                     if (p == 5 || p == 6)
                        send_to_char("A cooling rain is washing away the humid weather.\n\r", ch);
                     if (p == 7)
                     {
                        if (inside == 0)
                           send_to_char("It is pouring all around you, but atleast you are not warm.\n\r", ch);
                        else
                           send_to_char("Faint sounds of rain can be heard from outside this cozy room.\n\r", ch);
                     }
                     if (p == 8)
                     {
                        if (inside == 0)
                           send_to_char("A thick rain is literally pounding the warm air underground.\n\r", ch);
                        else
                           send_to_char("Thick drops of rain echo lightly through the room you are in.\n\r", ch);
                     }
                     if (p == 9)
                     {
                        if (inside == 0)
                           send_to_char("It was hot, but the obsurb amount of rain falling is much worse.\n\r", ch);
                        else
                           send_to_char("It sounds like it is raining cats and dogs outside.\n\r", ch);
                     }
                  }
                  else
                  {
                     if (p == 1)
                        send_to_char("Small drops of rain fall around you.\n\r", ch);
                     if (p == 2)
                        send_to_char("A very light rain is cooling things down a little.\n\r", ch);
                     if (p == 3 || p == 4)
                        send_to_char("The clouds come out along with a light rain.\n\r", ch);
                     if (p == 5 || p == 6)
                        send_to_char("A steady amount of rain is falling quickly from the sky.\n\r", ch);
                     if (p == 7)
                     {
                        if (inside == 0)
                           send_to_char("It is pouring all around you, hope you like being drenched.\n\r", ch);
                        else
                           send_to_char("Faint sounds of rain can be heard from outside this cozy room.\n\r", ch);
                     }
                     if (p == 8)
                     {
                        if (inside == 0)
                           send_to_char("A thick rain is nearly flooding the area.\n\r", ch);
                        else
                           send_to_char("Thick drops of rain echo lightly through the room you are in.\n\r", ch);
                     }
                     if (p == 9)
                     {
                        if (inside == 0)
                           send_to_char("It is raining so hard it is difficult to see your hands.\n\r", ch);
                        else
                           send_to_char("It sounds like it is raining cats and dogs outside.\n\r", ch);
                     }
                  }
               }
            }
         }
      }
   }
   mweather++;
}

void lckwis_update(void)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ch;
   int mlck;
   int mwis, mcon, mint, mstr;
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2)
      {
      	 ch = d->character;
         mlck = number_range(-5000, 5000);
         mwis = number_range(15, 20);
         if (sysdata.stat_gain <= 3)
            mwis = number_range(25, 35);
         else if (sysdata.stat_gain >= 5)
            mwis = number_range(40, 50);
            
         mcon = number_range(-15, -20);
         mint = number_range(-15, -20);
         mstr = number_range(-15, -20);
         if (ch->perm_wis >= (14 + race_table[ch->race]->wis_plus + race_table[ch->race]->wis_range + get_talent_increase(ch, 4)) && ch->pcdata->per_wis >= 3000)
            mwis = 0;
         if (ch->perm_lck == (22 + race_table[ch->race]->lck_plus) && ch->pcdata->per_lck >= 3000)
            mlck = 0;
         if (ch->perm_con < 14 + race_table[ch->race]->con_plus)
            mcon = 0;
         if (ch->perm_int < 14 + race_table[ch->race]->int_plus)
            mint = 0;
         if (ch->perm_str < 14 + race_table[ch->race]->str_plus)
            mstr = 0;
         if (ch->perm_lck <= 9 && mlck < 0)
            mlck = 0;
         ch->pcdata->per_str += mstr;
         ch->pcdata->per_lck += mlck;
         ch->pcdata->per_wis += mwis;
         ch->pcdata->per_con += mcon;
         ch->pcdata->per_int += mint;

         if (ch->pcdata->per_str < 0)
         {
            ch->perm_str--;
            send_to_char("&r****************************************\n\r", ch);
            send_to_char("&r******You Lose 1 Point of Strength******\n\r", ch);
            send_to_char("&r****************************************\n\r", ch);
            ch->pcdata->per_str = 9999;
         }
         if (ch->pcdata->per_con < 0)
         {
            ch->perm_con--;
            send_to_char("&r********************************************\n\r", ch);
            send_to_char("&r******You Lose 1 Point of Constitution******\n\r", ch);
            send_to_char("&r********************************************\n\r", ch);
            ch->pcdata->per_con = 9999;
         }
         if (ch->pcdata->per_int < 0)
         {
            ch->perm_int--;
            send_to_char("&r********************************************\n\r", ch);
            send_to_char("&r******You Lose 1 Point of Intelligence******\n\r", ch);
            send_to_char("&r********************************************\n\r", ch);
            ch->pcdata->per_int = 9999;
         }
         if (ch->pcdata->per_lck > 10000)
         {
            ch->perm_lck++;
            send_to_char("&R************************************\n\r", ch);
            send_to_char("&R******You Gain 1 Point of Luck******\n\r", ch);
            send_to_char("&R************************************\n\r", ch);
            ch->pcdata->per_lck = 0;
         }
         if (ch->pcdata->per_wis > 10000)
         {
            ch->perm_wis++;
            send_to_char("&R**************************************\n\r", ch);
            send_to_char("&R******You Gain 1 Point of Wisdom******\n\r", ch);
            send_to_char("&R**************************************\n\r", ch);
            ch->pcdata->per_wis = 0;
         }
         if (ch->pcdata->per_lck < 0)
         {
            ch->perm_lck--;
            send_to_char("&r************************************\n\r", ch);
            send_to_char("&r******You Lose 1 Point of Luck******\n\r", ch);
            send_to_char("&r************************************\n\r", ch);
            ch->pcdata->per_lck = 9999;
         }
      }
   }
}

int get_moral_conquer(int size)
{
   if (sysdata.resetgame)
   {
      if (size == 1)
         return 5;
      if (size == 2)
         return 11;
      if (size == 3)
         return 17;
      if (size == 4)
         return 23;
      if (size == 5)
         return 29;
      if (size == 6)
         return 41;
      if (size == 7)
         return 53;
      if (size == 8)
         return 71;
      if (size == 9)
         return 99;
    }
      
   if (size == 1)
      return 5;
   if (size == 2)
      return 11;
   if (size == 3)
      return 23;
   if (size == 4)
      return 35;
   if (size == 5)
      return 53;
   if (size == 6)
      return 71;
   if (size == 7)
      return 95;
   if (size == 8)
      return 119;
   if (size == 9)
      return 179;
      
   return 5;
}
      
      
int modify_conquer_reset(int time)
{
   if (sysdata.resetgame)
      return time/2;
   return time;
}
void update_conquer()
{
   CONQUER_DATA *conquer;
   CONQUER_DATA *nconquer;
   CONQUER_DATA *next_conquer;
   CONQUER_DATA *next_nconquer;
   int kingdoma, kingdomb;
   int destroy = 0;
   DESCRIPTOR_DATA *d;
   char logb[500];
   int x;
   TOWN_DATA *town;
   
   for (conquer = first_conquer; conquer; conquer = next_conquer)
   {
      next_conquer = conquer->next;
      if (conquer->occupied == 0)
      {
         UNLINK(conquer, first_conquer, last_conquer, next, prev);
         sprintf(logb, "You have lost your hold on the town of %s", conquer->ntown);
         write_kingdom_logfile(conquer->akingdom, logb, KLOG_WARLOSSES);
         sprintf(logb, "You have retained your hold on the town of %s", conquer->ntown);
         write_kingdom_logfile(conquer->rkingdom, logb, KLOG_WARLOSSES);
         if (conquer->ntown)
            STRFREE(conquer->ntown);
         DISPOSE(conquer); 
         continue;
      }
      if (time(0) - conquer->time >= modify_conquer_reset(259200)) // 3 days for norm 1.5 for reset
      {
         town = get_town(kingdom_table[conquer->akingdom]->dtown);
         
         for (nconquer = first_conquer; nconquer; nconquer = next_nconquer) //check to make sure no one else is trying to claim this town
         {
            next_nconquer = nconquer->next;
            if (nconquer->town == conquer->town && conquer != nconquer)
            {
               if (next_conquer == nconquer)
                  next_conquer = nconquer->next;
               sprintf(logb, "%s has fallen to %s, perhaps you could take it back from them!\n\r", conquer->town->name, kingdom_table[conquer->akingdom]->name);
               write_kingdom_logfile(nconquer->akingdom, logb, KLOG_WARLOSSES);
               STRFREE(nconquer->ntown);
               UNLINK(nconquer, first_conquer, last_conquer, next, prev);
               DISPOSE(nconquer);
            }
         }
         //check the areas to make sure this was not the last town for the kingdom
         
         if (!str_cmp(kingdom_table[conquer->rkingdom]->dtown, conquer->town->name))
         {
            TOWN_DATA *stown;
            CHAR_DATA *ch;
            
            destroy = 1;
            for (stown = kingdom_table[conquer->rkingdom]->first_town; stown; stown = stown->next)
            {
               if (stown->kingdom == conquer->rkingdom && stown != conquer->town)
               {
                  STRFREE(kingdom_table[conquer->rkingdom]->dtown);
                  kingdom_table[conquer->rkingdom]->dtown = STRALLOC(stown->name);    
                  sprintf(logb, "Since %s was conquered %s has became your default town", conquer->town->name, stown->name);
                  write_kingdom_logfile(conquer->rkingdom, logb, KLOG_FIRE);  
                  destroy = 0;
                  for (d = first_descriptor; d; d = d->next)
                  {
                     if (d->character && d->character->pcdata && d->character->pcdata->hometown == conquer->rkingdom)
                     {
                        ch = d->character;
                        if (d->connected == CON_PLAYING)
                        {
                           send_to_char("The town you are in has been destroyed, you are being placed in the new default town.\n\r", ch);
                        }
                        ch->pcdata->town = stown;
                     }
                  }
                  break;
               }
            }
         }
         conquer->town->kingdom = conquer->akingdom;
         conquer->town->kpid = kingdom_table[conquer->akingdom]->kpid;
         if (conquer->town->size > 1)
         {
            conquer->town->size = UMAX(1, conquer->town->size-1);
            conquer->town->moral = get_moral_conquer(conquer->town->size);
         }
         STRFREE(conquer->town->mayor);
         conquer->town->mayor = STRALLOC(kingdom_table[conquer->akingdom]->ruler);
         UNLINK(conquer->town, kingdom_table[conquer->rkingdom]->first_town, kingdom_table[conquer->rkingdom]->last_town, next, prev);
         LINK(conquer->town, kingdom_table[conquer->akingdom]->first_town, kingdom_table[conquer->akingdom]->last_town, next, prev);
         kingdoma = conquer->rkingdom;
         kingdomb = conquer->akingdom;
         if (destroy == 1) //no more towns left, bye bye kingdom
         {
            if (conquer->rkingdom > 1 && conquer->rkingdom < sysdata.max_kingdom)
            {
               kingdomb = kingdom_table[conquer->akingdom]->kpid;
               sprintf(logb, "You have conquered the town of %s, %s has been completely destroyed!!!\n\r", conquer->town->name, kingdom_table[conquer->rkingdom]->name);
               write_kingdom_logfile(conquer->akingdom, logb, KLOG_WARLOSSES);
               remove_kingdom(conquer->rkingdom);
               //no need to remove conquer file, it is removed in remove kingdom
            }
         }
         else
         {
            sprintf(logb, "You have conquered the town of %s from %s.\n\r", conquer->town->name, kingdom_table[conquer->rkingdom]->name);
            write_kingdom_logfile(conquer->akingdom, logb, KLOG_WARLOSSES); 
            if (conquer->ntown)
               STRFREE(conquer->ntown);
            UNLINK(conquer, first_conquer, last_conquer, next, prev);
            DISPOSE(conquer); 
         }
         if (destroy == 0)
            write_kingdom_file(kingdoma);
         if (destroy == 1)
         {
            for (x = 0; x < sysdata.max_kingdom; x++)
            {
               if (kingdom_table[x]->kpid == kingdomb)
               {
                  kingdomb = x;
                  break;
               }
            }
         }
         if (kingdomb < sysdata.max_kingdom)
            write_kingdom_file(kingdomb); 
      }
   }
   for (conquer = first_conquer; conquer; conquer = conquer->next)
   {
      conquer->occupied = 0;
   }
   save_conquer_file();
}

void account_update(void)
{
   DESCRIPTOR_DATA *d;
   DESCRIPTOR_DATA *dd;
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->account && d->account->lasttimereset > 0)
         d->account->timesave = 0;
   }
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->account && d->account->lasttimereset > 0 && d->account->timesave == 0) //This way we know they have atleast loaded all data
      {
         if (d->account->editing == 0)
         {
            continue;
         }
         else
         {
            d->account->timesave = 1;
            if (time(0) - d->account->lasttimereset > 3600)
            {
               d->account->lasttimereset = time(0);
               d->account->changes = 0;
               save_account(d, 0);
            }
            for (dd = first_descriptor; dd; dd = dd->next)
            {
               if (dd->account && dd->account->lasttimereset > 0 && !str_cmp(dd->account->name, d->account->name))
               {
                  dd->account->timesave = 1;
                  dd->account->changes = 0;
                  dd->account->lasttimereset = d->account->lasttimereset;
               }
            }
         }
      }
   }
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->account && d->account->lasttimereset > 0 && d->account->timesave == 0) //This way we know they have atleast loaded all data
      {
         if (d->account->editing == 0)
         {
            d->account->timesave = 1;
            if (time(0) - d->account->lasttimereset > 3600)
            {
               d->account->lasttimereset = time(0);
               d->account->changes = 0;
               save_account(d, 0);
            }
         }
         else
         {
            continue;
         }
      }
   }
}

char *const kingdom_race[MAX_RACE] = 
{
   "Human", "Elven", "Dwarven", "Ogre", "Hobbit", "Fairy"
};

int max_allowedunits(int size)
{
   switch (size)
   {
      case 1:
         return 1;
      case 2:
         return 2;
      case 3:
         return 4;
      case 4:
         return 5;
      case 5:
         return 7;
      case 6:
         return 8;
      case 7:
         return 10;
      case 8:
         return 12;
      case 9:
         return 14;
      case 10:
         return 20;
      default:
         return 1;
   }
   return 1;
}

void start_training_schedule(TOWN_DATA *town, SCHEDULE_DATA *schedule)
{
   BUYKMOB_DATA *kmob;
   char buf[MSL];
   OBJ_DATA *obj;
   int diffperiod;
   TRAINING_DATA *training;
   int vnum;
   int worker = -1;
   MOB_INDEX_DATA *pMobIndex;
   OMAP_DATA *mobj; 
   
   for (kmob = first_buykmob; kmob; kmob = kmob->next)
   {
      if (xIS_SET(kmob->flags, KMOB_WORKER))
         break;
   }
   if (!kmob)
   {
      bug("There is no worker mob available for kingdoms!");
      sprintf(buf, "Could not find a worker mob, tell an immortal\n\r");
      write_kingdom_logfile(town->kingdom, buf, KLOG_SCHEDULE);
      return;
   }
   if (!proper_resources_mobs_town(town, kmob))
   {
      sprintf(buf, "You did not have enough money to add a worker on period %d to %d", schedule->start_period, schedule->end_period);
      write_kingdom_logfile(town->kingdom, buf, KLOG_SCHEDULE);
      return;
   }
   vnum = kmob->vnum;
   pMobIndex = get_mob_index(vnum);
   if (town->unitstraining == get_maxunits(town->size))
   {
      sprintf(buf, "Your training queue is full, wait for one to finish training or remove one.\n\r");
      write_kingdom_logfile(town->kingdom, buf, KLOG_SCHEDULE);
      return;
   }        
   for (mobj = first_wilderobj; mobj; mobj = mobj->next)
   {
      obj = mobj->mapobj;

      if (obj->item_type == ITEM_HOLDRESOURCE && obj->coord->x == schedule->x && obj->coord->y == schedule->y && obj->map == schedule->map)
      {
         break;
      }
   }
   if (!mobj)
   {
      if (!in_town_range(town, schedule->x, schedule->y, schedule->map))
      {
         sprintf(buf, "You can only start workers in your town or at a bin\n\r");
         write_kingdom_logfile(town->kingdom, buf, KLOG_SCHEDULE);
         return;
      }
      worker = 1;
   }         
   CREATE(training, TRAINING_DATA, 1);
   training->kmob = kmob;
   //New Units take 1 month longer to build unless 1 or 10 or "worker" units
   if (town->size >= pMobIndex->m12+2 || pMobIndex->m12 == 1 || pMobIndex->m12 == 10)
   {
      training->speed = pMobIndex->m11;
   }
   else
   {
      training->speed = pMobIndex->m11 + 2 - (town->size - pMobIndex->m12);
   }
   training->kingdom = town->kingdom;
   training->town = town->tpid;
   training->stime = time(0);
   if (schedule->start_period > schedule->end_period)
   {
      diffperiod = 48 - schedule->start_period + schedule->end_period + 1;
   }
   else
   {
      diffperiod = schedule->end_period - schedule->start_period + 1;
   }
   training->etime = (diffperiod * cvttime(16200))-1;
   if (worker == 0)
     training->bin = 1;
   training->x = schedule->x;
   training->y = schedule->y;
   training->map = schedule->map;
   training->resource = schedule->resource;
   LINK(training, first_training, last_training, next, prev);
   fwrite_training_list();
   remove_resources_mobs_town(town, kmob);
   sprintf(buf, "A worker has been placed in training on period %d", schedule->start_period);
   write_kingdom_logfile(town->kingdom, buf, KLOG_SCHEDULE);
   town->unitstraining++;
   write_kingdom_file(town->kingdom);
   return;
}

int cvttime(int time)
{
   if (sysdata.resetgame)
      return time/3;
   return time;
}

//16200 - 7.5 Game Day (Quarter GD Month)
//Checks the schedule to make sure the punks are working!
//Is not real time, so if a month is missed, a month is missed...
void schedule_check(void)
{
   SCHEDULE_DATA *schedule;
   SCHEDULE_DATA *next_schedule;
   TOWN_DATA *town;
   int kx;
   int yeartime;
   
   if ((time(0) - sysdata.start_calender) % cvttime(16200) == 0) //Worker Period (1/4 a month)
   {
      for (kx = 2; kx < sysdata.max_kingdom; kx++)
      {
         for (town = kingdom_table[kx]->first_town; town; town = town->next)
         {
            if (town->first_schedule)
            {
               for (schedule = town->first_schedule; schedule; schedule = next_schedule)
               {
                  next_schedule = schedule->next;
                  yeartime = (time(0) - sysdata.start_calender) % cvttime(777600);
               
                  if ((yeartime/cvttime(16200)+1 == 48 && schedule->start_period == 1) || (yeartime/cvttime(16200)+1 == schedule->start_period-1))
                  {
                     start_training_schedule(town, schedule);
                     if (schedule->reoccur == 0)
                     {
                        UNLINK(schedule, town->first_schedule, town->last_schedule, next, prev);
                        DISPOSE(schedule);
                     }
                     continue;
                  }
               }
            }
         }
      }
   }
}
   

//Place mobs that are ready for placing mkkay!
void training_check(void)
{
   TRAINING_DATA *training;
   TRAINING_DATA *ntraining;
   CHAR_DATA *victim;
   CHAR_DATA *mount;
   TOWN_DATA *town;
   OBJ_DATA *obj;
   char buf[MSL];
   char fir[MSL];
   char name[MSL];
   int ttime = 0;
   int x;
   
   for (training = first_training; training; training = ntraining)
   {
      ntraining = training->next;
      
      if (training->speed == 0)
         ttime = cvttime(TRAINING_TIME)/2;
      else
         ttime = (training->speed*cvttime(TRAINING_TIME));
         
      if (training->stime + ttime <= time(0))
      {   
         //do some error checking first
         if (!IN_PLAYER_KINGDOM(training->kingdom))
         {
            UNLINK(training, first_training, last_training, next, prev);
            DISPOSE(training);
            continue;
         }      
         town = get_town_tpid(training->kingdom, training->town);
         if (!town)
         {
            sprintf(buf, "A training mob was removed because of an Invalid Town in your Kingdom");
            write_kingdom_logfile(training->kingdom, buf, KLOG_PLACEMOB);
            UNLINK(training, first_training, last_training, next, prev);
            DISPOSE(training);
            continue;
         }
         if (!training->kmob)
         {
            sprintf(buf, "A training mob was removed because of invalid Kingdom Mobile Pointer");
            write_kingdom_logfile(training->kingdom, buf, KLOG_PLACEMOB);
            UNLINK(training, first_training, last_training, next, prev);
            DISPOSE(training);
            continue;
         }
         if (get_kingdom_units(town->tpid) == max_allowedunits(town->size))
         {
            sprintf(buf, "Your town is at max unit capacity, either remove the unit or wait for a spot to open.");
            write_kingdom_logfile(training->kingdom, buf, KLOG_PLACEMOB);  
            training->stime = time(0) - ttime + cvttime(2160); //1 Game Day, 36 Minutes RL
            fwrite_training_list();
            continue;
         }
         if (town->barracks[0] == 0)
         {
            sprintf(buf, "No barracks set for the town of %s, a Trained Unit is ready for deployment", town->name);
            write_kingdom_logfile(training->kingdom, buf, KLOG_PLACEMOB);  
            training->stime = time(0) - ttime + cvttime(2160); //1 Game Day, 36 Minutes RL
            fwrite_training_list();
            continue;
         }
         town->unitstraining -= 1;
         write_kingdom_file(training->kingdom);
         for (x = 1; x <= 5; x++)
         {
            victim = create_mobile(get_mob_index(training->kmob->vnum));
            //change the mobile based on race now....
            victim->race = kingdom_table[training->kingdom]->race;
            victim->perm_str += race_table[victim->race]->str_plus;
            victim->perm_dex += race_table[victim->race]->dex_plus;	
            victim->perm_wis += race_table[victim->race]->wis_plus;
            victim->perm_int += race_table[victim->race]->int_plus;
            victim->perm_con += race_table[victim->race]->con_plus;
            victim->perm_lck += race_table[victim->race]->lck_plus;
            SET_BIT(victim->resistant, race_table[victim->race]->resist);
            SET_BIT(victim->susceptible, race_table[victim->race]->suscept);
            xSET_BITS(victim->affected_by, race_table[victim->race]->affected);
            if (kingdom_table[training->kingdom]->name[0] == 'a' || kingdom_table[training->kingdom]->name[0] == 'A' 
            || kingdom_table[training->kingdom]->name[0] == 'e' || kingdom_table[training->kingdom]->name[0] == 'E' 
            || kingdom_table[training->kingdom]->name[0] == 'i' || kingdom_table[training->kingdom]->name[0] == 'I' 
            || kingdom_table[training->kingdom]->name[0] == 'o' || kingdom_table[training->kingdom]->name[0] == 'O' 
            || kingdom_table[training->kingdom]->name[0] == 'u' || kingdom_table[training->kingdom]->name[0] == 'U')
            {
               sprintf(fir, "An ");
            }
            else
               sprintf(fir, "A ");
            
            sprintf(name, "%s", victim->name);
            sprintf(buf, "%s %s %s", kingdom_table[training->kingdom]->name, kingdom_race[victim->race], name);
            STRFREE(victim->name);
            victim->name = STRALLOC(buf);
            STRFREE(victim->short_descr);
            victim->short_descr = STRALLOC(buf);
            sprintf(buf, "%s%s %s %s is here awaiting orders.\n\r", fir, kingdom_table[training->kingdom]->name, kingdom_race[victim->race], name);
            STRFREE(victim->long_descr);
            victim->long_descr = STRALLOC(buf);
            char_to_room(victim, get_room_index(OVERLAND_SOLAN));
            //m1 - Resource AMT   m2 - Resource MAX   m3 - Cost   m4 - Kingdom   m5 - Resource Type   m6 - Extraction effic.
            //m7 - Town pid  m9 - time
            if (training->x)
            {
               victim->coord->x = training->x;
               victim->coord->y = training->y;
               victim->map = training->map;
               victim->m3 = training->kmob->coins;
               victim->m4 = training->kingdom;
               victim->m5 = training->resource;
               victim->m7 = town->tpid;
               victim->m9 = time(0);
               if (training->etime > 0)
                  victim->m10 = time(0) + training->etime;
               if (IN_WILDERNESS(victim))
                  SET_ONMAP_FLAG(victim); 
               if (training->bin == 0)
               {
                  xSET_BIT(victim->act, ACT_EXTRACTTOWN);
                     victim->dumptown = town;
               }
               if (x == 5)
               {
                  sprintf(buf, "%s has been trained and is ready for command at the town of %s!", victim->name, town->name);
                  write_kingdom_logfile(training->kingdom, buf, KLOG_PLACEMOB);
                  UNLINK(training, first_training, last_training, next, prev);
                  DISPOSE(training);
                  fwrite_training_list();
                  save_extraction_data();
               }
               continue;
            }
            victim->coord->x = town->barracks[0];
            victim->coord->y = town->barracks[1];
            victim->map = town->barracks[2];
            victim->m1 = town->tpid;
            victim->m3 = training->kmob->coins;
            victim->m4 = training->kingdom;
            victim->m9 = time(0);
            xSET_BIT(victim->miflags, KM_STATIONARY);
            xSET_BIT(victim->miflags, KM_ATTACKE);
            if (IN_WILDERNESS(victim))
               SET_ONMAP_FLAG(victim); 
            if (training->kmob)
            {
               if (xIS_SET(training->kmob->flags, KMOB_MOUNTED))
               {
                  mount = create_mobile(get_mob_index(MOB_KMOB_HORSE));
                  char_to_room(mount, get_room_index(OVERLAND_SOLAN));
                  mount->coord->x = victim->coord->x;
                  mount->coord->y = victim->coord->y;
                  mount->map = victim->map;
                  mount->m4 = training->kingdom;
                  SET_ONMAP_FLAG(mount); 
                  xSET_BIT(mount->act, ACT_MOUNTED);
                  victim->mount = mount;
                  victim->position = POS_MOUNTED;
               }
            }         
            //All mobs get leather armor atleast.
            obj = create_object(get_obj_index(OBJ_FORGE_STUDDED_LEATHER_ARMOR), 1);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_BODY); 

            obj = create_object(get_obj_index(OBJ_FORGE_STUDDED_LEATHER_HEAD), 1);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_HEAD); 

            obj = create_object(get_obj_index(OBJ_FORGE_STUDDED_LEATHER_NECK), 1);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_NECK); 

            obj = create_object(get_obj_index(OBJ_FORGE_STUDDED_LEATHER_GAUNTLET), 1);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_ARM_R);  
            obj = create_object(get_obj_index(OBJ_FORGE_STUDDED_LEATHER_GAUNTLET), 1);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_ARM_L); 

            obj = create_object(get_obj_index(OBJ_FORGE_STUDDED_LEATHER_GREAVE), 1);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_LEG_R); 
            obj = create_object(get_obj_index(OBJ_FORGE_STUDDED_LEATHER_GREAVE), 1);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_LEG_L);  
            if (x == 5)
            {
               sprintf(buf, "%s has been trained and is ready for command at the town of %s!", victim->name, town->name);
               write_kingdom_logfile(training->kingdom, buf, KLOG_PLACEMOB);
               UNLINK(training, first_training, last_training, next, prev);
               DISPOSE(training);
               fwrite_training_list();
               save_mlist_data();
            }
         }
      }
   }
}

void cut_extra_lines(char *logfile, int numlines, int maxlines)
{
   int cnt = 0;
   FILE *fp;
   FILE *nfp;
   char buf[MSL];
   char *line;
   int dropline = numlines - maxlines;
   
   sprintf(buf, "%s.new", logfile);
   
   if ((fp = fopen(logfile, "r")) == NULL)
      return;
      
   if ((nfp = fopen(buf, "w")) == NULL)
   {
      fclose(fp);
      return;
   }
   
   while (!feof(fp))
   {
      line = fread_line(fp);
      cnt++;
      if (cnt <= dropline)
         continue;
      else
         fprintf(nfp, line);
   }
   fclose(fp);
   fclose(nfp);
   remove(logfile);
   rename(buf, logfile);
   return;
}

void cut_extra_time(char *logfile, int maxtime)
{
   FILE *fp;
   FILE *nfp;
   char buf[MSL];
   char gtime[MSL];
   char cbuf[MSL];
   char *pcbuf;
   char *line;
   int linetime;
   int fline = 1;
   int dtime;
   int cnt = 0;
   
   dtime = time(0) - maxtime;
   
   sprintf(buf, "%s.new", logfile);
   
   if ((fp = fopen(logfile, "r")) == NULL)
      return;
      
   if ((nfp = fopen(buf, "w")) == NULL)
   {
      fclose(fp);
      return;
   }
   
   while (!feof(fp))
   {
      line = fread_line(fp);
      sprintf(cbuf, line);
      pcbuf = cbuf;
      pcbuf = one_argument(pcbuf, gtime);  //Time
      linetime = atoi(gtime);
      
      if (fline == 1)
      {
         if (linetime >= dtime)
            break;
         else
            fline = 0;
      }
      if (linetime >= dtime)
      {
         fprintf(nfp, line);
         cnt++;
      }
      else
         continue;
   }
   fclose(fp);
   fclose(nfp);
   if (fline == 0)
   {
      if (cnt == 0)
         remove(logfile);
      else
      {
         remove(logfile);
         rename(buf, logfile);
      }
   }
   else
   {
      remove(buf);
   }
   return;
}

void update_klogs(void)
{
   int kingdom;
   
   for (kingdom = 2; kingdom < sysdata.max_kingdom; kingdom++)
   {
      if (kingdom_table[kingdom]->maxlinelog <= 0 && kingdom_table[kingdom]->maxtimelog <= 0)
         return;
      if (kingdom_table[kingdom]->maxlinelog > 0)
      {
         if (klog_linecount(kingdom_table[kingdom]->logfile) > kingdom_table[kingdom]->maxlinelog)
         {
            cut_extra_lines(kingdom_table[kingdom]->logfile, klog_linecount(kingdom_table[kingdom]->logfile),
               kingdom_table[kingdom]->maxlinelog);
         }
      }
      if (kingdom_table[kingdom]->maxtimelog > 0)
      {
         cut_extra_time(kingdom_table[kingdom]->logfile, kingdom_table[kingdom]->maxtimelog);
      }
   }
}
   
void quest_update(void)
{
   QUEST_DATA *quest;
   CHAR_DATA *ch;
   QUEST_DATA *nquest;
   
   for (quest = first_quest; quest; quest = nquest)
   {
      nquest = quest->next;
      if (quest->traveltime > -1)
      {
         quest->traveltime--;
         if (quest->traveltime == 0)
            quest->traveltime = -1;
         continue;
      }
      if (quest->timeleft > -1)
      {
         quest->timeleft--;
         if (quest->timeleft == 0)
         {
            quest->timeleft = -1;
            quest->tillnew = 25;
            for (ch = first_char; ch; ch = ch->next)
            {
               if (IS_NPC(ch))
                  continue;
               if (!ch->pcdata->quest)
                  continue;
               if (ch->pcdata->quest != quest)
                  continue;
               ch->pcdata->quest_losses++;
               send_to_char("You have ran out of time to complete your test, better luck next time.\n\r", ch);
            }
         }
      }
      if (quest->tillnew > -1)
      {
         quest->tillnew--;
         if (quest->tillnew == 0)
         {
            for (ch = first_char; ch; ch = ch->next)
            {
               if (IS_NPC(ch))
                  continue;
               if (!ch->pcdata->quest)
                  continue;
               if (ch->pcdata->quest != quest)
                  continue;
               send_to_char("You are now free to get another quest.\n\r", ch);
               ch->pcdata->quest = NULL;
            }   
            disposequestarea(quest->questarea, quest->difficulty);
            UNLINK(quest, first_quest, last_quest, next, prev);
            DISPOSE(quest);
         }
      }
   }
}
     
void wblock_update(void)         
{
   WBLOCK_DATA *wblock;
   
   for (wblock = first_wblock; wblock; wblock = wblock->next)
   {
      if (wblock->timecheck == 0)
      {
         wblock->timecheck = time(0);
         wblock->kills = 0;
      }
      else
      {
         if (time(0) - wblock->timecheck >= 21600) // 6 hours
         {
            if (wblock->kills > 1000)
            {
               wblock->lvl = UMIN(1, wblock->lvl-1);
               wblock->kills = 0;
            }
            else
            {
               wblock->kills -= number_range(2, 5);
               if (wblock->kills < -1000)
               {
                  wblock->lvl = UMIN(79, wblock->lvl+1);
                  wblock->kills = 0;
               }
            }
            wblock->timecheck+=21600;
         }
      }
   }
   save_wblock_data();
}   
   
void update_ship(void)
{
   SHIP_DATA *ship;
   int x;
   char dletter;
   int dir;
   
   for (ship = first_ship; ship; ship = ship->next)
   {
      if (ship->travelroute && ship->travelroute[0] != '\0')
      {
         if (ship->routeplace == 0)
            ship->routeplace = 1;
            
         if (ship->routetick > 0)
         {
            ship->routetick--;
            continue;
         }
         for (x = 0;;x++)
         {
            if (ship->travelroute[x] == '\0')
            {
               bug("update_ship:  The end of a route was reached, the route will need to be reset");
               break;;
            }
            if (x+1 == ship->routeplace)
            {
               dletter = ship->travelroute[x];
               if (dletter == 'n')
                  dir = 0;
               else if (dletter == 'e')
                  dir = 1;
               else if (dletter == 's')
                  dir = 2;
               else if (dletter == 'w')
                  dir = 3;
               else
               {
                  bug("update_ship:  Invalid direction in the route path.  The route will need to be reset");
                  break;;
               }
               if (ship->routedir == 1)
               {
                  if (dir == 0)
                     dir = 2;
                  else if (dir == 1)
                     dir = 3;
                  else if (dir == 2)
                     dir = 0;
                  else if (dir == 3)
                     dir = 1;
               }
               steer_ship(NULL, ship, dir);
               if (ship->routedir == 1 && ship->routeplace == 1)
               {
                  ship->routetick = ship->routetime;
                  ship->routedir = 0;
               }               
               else if (ship->routedir == 0 && ship->travelroute[x+1] == '\0')
               {
                  ship->routetick = ship->routetime;
                  ship->routedir = 1;
               }
               else if (ship->routedir == 0)
                  ship->routeplace++;
               else
                  ship->routeplace--;
               break;
            }
         }
      }
   }
   fwrite_ship_data();
}
   
/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler(void)
{
   static int pulse_area;
   static int pulse_mobile;
   static int pulse_violence;
   static int pulse_wilderness;
   static int pulse_skill;
   static int pulse_point;
   static int pulse_second;
   static int pulse_weather;
   static int pulse_lckwis;
   static int pulse_start_arena = PULSE_ARENA;
   static int pulse_arena = PULSE_ARENA;
   static int pulse_minute;
   static int pulse_ships;
   struct timeval stime;
   struct timeval etime;


   if (timechar)
   {
      set_char_color(AT_PLAIN, timechar);
      send_to_char("Starting update timer.\n\r", timechar);
      gettimeofday(&stime, NULL);
   }
   
   if (--pulse_minute <= 0)
   {
      pulse_minute = PULSE_AREA;
      quest_update();
      wblock_update();
   }
   
   if (--pulse_ships <= 0)
   {
      pulse_ships = PULSE_PER_SECOND*5;
      update_ship();
   }
   if (--pulse_wilderness <= 0)
   {
      pulse_wilderness = PULSE_AREA*5;
      if (start_next_round == 300) //Wait for atleast 5 minutes after bootup to check
         wilderness_save();
   }

   if (--pulse_area <= 0)
   {
      pulse_area = number_range(PULSE_AREA / 2, 3 * PULSE_AREA / 2);
      area_update();
      trade_check();
      save_wblock_data();
   }

   if (--pulse_mobile <= 0)
   {
      pulse_mobile = PULSE_MOBILE;
      mobile_update();
      obj_speed_update();
   }

   if (--pulse_violence <= 0)
   {
      pulse_violence = PULSE_VIOLENCE;
      violence_update();
      char_speed_update();
   }
   if (--pulse_lckwis <= 0)
   {
      pulse_lckwis = number_range(PULSE_AREA * 20, PULSE_AREA * 40);
      lckwis_update();
   }
   if (--pulse_skill <= 0)
   {
      pulse_skill = PULSE_AREA * 15;    
      skill_update(); //reduce skill in skills/spells
      if (sysdata.resetgame)
         read_pfiles_for_stats(NULL);
   }

   if (--pulse_point <= 0)
   {
      pulse_point = number_range(PULSE_TICK * 0.75, PULSE_TICK * 1.25);

      auth_update(); /* Gorog */
      char_update();
      kingdom_update();
      update_conquer();
      save_mlist_data();
      who_update();
      entrance_update();
      obj_update();
      write_channelhistory_file();
      clear_vrooms(); /* remove virtual rooms */
      update_klogs();
   }
   if (--pulse_weather <= 0)
   {
      pulse_weather = PULSE_WEATHER;
      //weather_update();
      save_weatherdata();
      winfo_update(); //Sends weather updates to players
   }
   if (--pulse_second <= 0) /* Pfiles -- Xerves */
   {
      AREA_DATA *pArea;
      pulse_second = PULSE_PER_SECOND;
      if (start_next_round == 300) //Wait for atleast 5 minutes after bootup to check
         resource_population_check();
      else
         start_next_round++;
      char_check();
      check_dns();
      training_check();
      tornado_update();
      schedule_check();
      time_update(); //time is now not done by this loop, so check every second.
      check_pfiles(0);
      reboot_check(0);
      copyover_check();
      for (pArea = first_area; pArea; pArea = pArea->next)
         reset_area(pArea, 1);
   }

   if (--auction->pulse <= 0) /* Arena -- Xerves */
   {
      auction->pulse = PULSE_AUCTION;
      auction_update();
      save_extraction_data();
   }

   if (in_start_arena || ppl_challenged)
      if (--pulse_start_arena <= 0)
      {
         pulse_start_arena = PULSE_ARENA;
         start_arena();
      }

   if (ppl_in_arena)
      if ((--pulse_arena <= 0) || (num_in_arena() == 1))
      {
         pulse_arena = PULSE_ARENA;
         do_game();
      }

   mpsleep_update(); /* Check for sleeping mud progs -rkb */
   tele_update();
   aggr_update();
   obj_act_update();
   room_act_update();
   clean_obj_queue(); /* dispose of extracted objects */
   clean_char_queue(); /* dispose of dead mobs/quitting chars */
   //muntrace();
   if (time(0) - sysdata.lastaccountreset >= 3600) // 1 hour
   {
      sysdata.accounts = 0;
      sysdata.lastaccountreset = time(0);
   }
   account_update();
   if (timechar)
   {
      gettimeofday(&etime, NULL);
      set_char_color(AT_PLAIN, timechar);
      send_to_char("Update timing complete.\n\r", timechar);
      subtract_times(&etime, &stime);
      ch_printf(timechar, "Timing took %d.%06d seconds.\n\r", etime.tv_sec, etime.tv_usec);
      timechar = NULL;
   }
   tail_chain();
   return;
}


void remove_portal(OBJ_DATA * portal)
{
   ROOM_INDEX_DATA *fromRoom, *toRoom;
   EXIT_DATA *pexit;
   bool found;

   if (!portal)
   {
      bug("remove_portal: portal is NULL", 0);
      return;
   }

   fromRoom = portal->in_room;
   found = FALSE;
   if (!fromRoom)
   {
      bug("remove_portal: portal->in_room is NULL", 0);
      return;
   }

   for (pexit = fromRoom->first_exit; pexit; pexit = pexit->next)
      if (IS_SET(pexit->exit_info, EX_PORTAL))
      {
         found = TRUE;
         break;
      }

   if (!found)
   {
      bug("remove_portal: portal not found in room %d!", fromRoom->vnum);
      return;
   }

   if (pexit->vdir != DIR_PORTAL)
      bug("remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir);

   if ((toRoom = pexit->to_room) == NULL)
      bug("remove_portal: toRoom is NULL", 0);

   extract_exit(fromRoom, pexit);

   return;
}

void copyover_check(void)
{
   static char *tmsg[] = {
      "5 seconds till copyover!  Check \"changes -1\" for new additions.",
      "10 seconds till copyover!",
      "20 seconds till copyover!",
      "30 seconds till copyover!",
      "1 minute till copyover!",
      "2 minutes till copyover!",
      "3 minutes till copyover!",
      "4 minutes till copyover!",
      "5 minutes till copyover!",
      "10 minutes till copyover.",
      "15 minutes till copyover.",
      "30 minutes till copyover."
   };
   static const int times[] = { 5, 10, 20, 30, 60, 120, 180, 240, 300, 600, 900, 1800 };
   static const int timesize = UMIN(sizeof(times) / sizeof(*times), sizeof(tmsg) / sizeof(*tmsg));
   int x;
   
  
   if (copyover_time == 0)
      return;
   
   if (copyover_time == time(0))
   {
      do_copyover(NULL, "now");
      return; //Well won't ever run but anyway..
   }
   
   for (x = 0; x <= timesize-1; x++)
   {
      if (copyover_time - time(0) == times[x])
         echo_to_all(AT_YELLOW, tmsg[x], ECHOTAR_ALL);
   }
   return;
}  

void reboot_check(time_t reset)
{
   static char *tmsg[] = {
      "5 seconds till reboot!",
      "10 seconds till reboot!",
      "20 seconds till reboot!",
      "30 seconds till reboot!",
      "1 minute till reboot!",
      "2 minutes till reboot!",
      "3 minutes till reboot!",
      "4 minutes till reboot!",
      "5 minutes till reboot!",
      "10 minutes till reboot.",
      "15 minutes till reboot.",
      "30 minutes till reboot."
   };
   static const int times[] = { 5, 10, 20, 30, 60, 120, 180, 240, 300, 600, 900, 1800 };
   static const int timesize = UMIN(sizeof(times) / sizeof(*times), sizeof(tmsg) / sizeof(*tmsg));
   char buf[MSL];
   static int trun;
   static bool init = FALSE;

   if (!init || reset >= current_time)
   {
      for (trun = timesize - 1; trun >= 0; trun--)
         if (reset >= current_time + times[trun])
            break;
      init = TRUE;
      return;
   }

   if ((current_time % 1800) == 0)
   {
      sprintf(buf, "%.24s: %d players", ctime(&current_time), num_descriptors);
      append_to_file(USAGE_FILE, buf);
      sprintf(buf, "%.24s:  %dptn  %dpll  %dsc %dbr  %d global loot",
         ctime(&current_time), sysdata.upotion_val, sysdata.upill_val, sysdata.scribed_used, sysdata.brewed_used, sysdata.global_looted);
      append_to_file(ECONOMY_FILE, buf);
   }

   if (new_boot_time_t - boot_time < 60 * 60 * 18 && !set_boot_time->manual)
      return;

   if (new_boot_time_t <= current_time)
   {
      CHAR_DATA *vch;
      extern bool mud_down;

      if (auction->item)
      {
         sprintf(buf, "Sale of %s has been stopped by mud.", auction->item->short_descr);
         talk_auction(buf);
         obj_to_char(auction->item, auction->seller);
         auction->item = NULL;
         if (auction->buyer && auction->buyer != auction->seller)
         {
            auction->buyer->gold += auction->bet;
            send_to_char("Your money has been returned.\n\r", auction->buyer);
         }
      }
      echo_to_all(AT_YELLOW, "You are forced from these realms by a strong " "magical presence\n\ras life here is reconstructed.", ECHOTAR_ALL);
      log_string("Automatic Reboot");
      for (vch = first_char; vch; vch = vch->next)
         if (!IS_NPC(vch))
            save_char_obj(vch);
      mud_down = TRUE;
      return;
   }

   if (trun != -1 && new_boot_time_t - current_time <= times[trun])
   {
      echo_to_all(AT_YELLOW, tmsg[trun], ECHOTAR_ALL);
      if (trun <= 5)
         sysdata.DENY_NEW_PLAYERS = TRUE;
      --trun;
      return;
   }
   return;
}

/* the auction update*/

void auction_update(void)
{
   int tax, pay;
   char buf[MSL];

   if (!auction->item)
   {
      if (AUCTION_MEM > 0 && auction->history[0] && ++auction->hist_timer == 6 * AUCTION_MEM)
      {
         int i;

         for (i = AUCTION_MEM - 1; i >= 0; i--)
         {
            if (auction->history[i])
            {
               auction->history[i] = NULL;
               auction->hist_timer = 0;
               break;
            }
         }
      }
      return;
   }

   switch (++auction->going) /* increase the going state */
   {
      case 1: /* going once */
      case 2: /* going twice */
         if (auction->bet > auction->starting)
            sprintf(buf, "%s: going %s for %d.", auction->item->short_descr, ((auction->going == 1) ? "once" : "twice"), auction->bet);
         else
            sprintf(buf, "%s: going %s (bid not received yet).", auction->item->short_descr, ((auction->going == 1) ? "once" : "twice"));

         talk_auction(buf);
         break;

      case 3: /* SOLD! */
         if (!auction->buyer && auction->bet)
         {
            bug("Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet);
            if (auction->seller)
            {
               add_timer(auction->seller, TIMER_AUCTION, 60, NULL, 0); //30 seconds;
            }
            auction->bet = 0;
         }
         if (auction->bet > 0 && auction->buyer != auction->seller)
         {
            sprintf(buf, "%s sold to %s for %d.",
               auction->item->short_descr, IS_NPC(auction->buyer) ? auction->buyer->short_descr : auction->buyer->name, auction->bet);
            talk_auction(buf);

            act(AT_ACTION, "The auctioneer materializes before you, and hands you $p.", auction->buyer, auction->item, NULL, TO_CHAR);
            act(AT_ACTION, "The auctioneer materializes before $n, and hands $m $p.", auction->buyer, auction->item, NULL, TO_ROOM);

            if ((get_ch_carry_weight(auction->buyer) + get_obj_weight(auction->item)) > can_carry_w(auction->buyer))
            {
               act(AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item, NULL, TO_CHAR);
               act(AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", auction->buyer, auction->item, NULL, TO_ROOM);
               obj_to_room(auction->item, auction->buyer->in_room, auction->buyer);
            }
            else
               obj_to_char(auction->item, auction->buyer);
            pay = (int) auction->bet * 0.9;
            tax = (int) auction->bet * 0.1;
            boost_economy(auction->seller->in_room->area, tax);
            auction->seller->gold += pay; /* give him the money, tax 10 % */
            sprintf(buf, "The auctioneer pays you %d gold, charging an auction fee of %d.\n\r", pay, tax);
            send_to_char(buf, auction->seller);
            if (auction->seller)
            {
               add_timer(auction->seller, TIMER_AUCTION, 60, NULL, 0); //30 seconds;
            }
            auction->item = NULL; /* reset item */
            if (IS_SET(sysdata.save_flags, SV_AUCTION))
            {
               save_char_obj(auction->buyer);
               save_char_obj(auction->seller);
            }
         }
         else /* not sold */
         {
            sprintf(buf, "No bids received for %s - removed from auction.\n\r", auction->item->short_descr);
            talk_auction(buf);
            act(AT_ACTION, "The auctioneer appears before you to return $p to you.", auction->seller, auction->item, NULL, TO_CHAR);
            act(AT_ACTION, "The auctioneer appears before $n to return $p to $m.", auction->seller, auction->item, NULL, TO_ROOM);
            if ((get_ch_carry_weight(auction->seller)+ get_obj_weight(auction->item)) > can_carry_w(auction->seller))
            {
               act(AT_PLAIN, "You drop $p as it is just too much to carry"
                  " with everything else you're carrying.", auction->seller, auction->item, NULL, TO_CHAR);
               act(AT_PLAIN, "$n drops $p as it is too much extra weight"
                  " for $m with everything else.", auction->seller, auction->item, NULL, TO_ROOM);
               obj_to_room(auction->item, auction->seller->in_room, auction->seller);
            }
            else
               obj_to_char(auction->item, auction->seller);
            tax = (int) (auction->starting - auction->item->cost) * 0.05; //5 percent of asking over cost...
            tax = UMAX(50, tax);
            boost_economy(auction->seller->in_room->area, tax);
            sprintf(buf, "The auctioneer charges you an auction fee of %d.\n\r", tax);
            send_to_char(buf, auction->seller);
            if ((auction->seller->gold - tax) < 0)
               auction->seller->gold = 0;
            else
               auction->seller->gold -= tax;
            if (auction->seller)
            {
               add_timer(auction->seller, TIMER_AUCTION, 60, NULL, 0); //30 seconds;
            }
            if (IS_SET(sysdata.save_flags, SV_AUCTION))
               save_char_obj(auction->seller);
         } /* else */
         {
            auction->item = NULL; /* clear auction */
            if (auction->seller)
            {
               add_timer(auction->seller, TIMER_AUCTION, 60, NULL, 0); //30 seconds;
            }
         }
   } /* switch */
} /* func */

void subtract_times(struct timeval *etime, struct timeval *stime)
{
   etime->tv_sec -= stime->tv_sec;
   etime->tv_usec -= stime->tv_usec;
   while (etime->tv_usec < 0)
   {
      etime->tv_usec += 1000000;
      etime->tv_sec--;
   }
   return;
}

//New Weather update, the system is a weather front system that is nicer and
//possesses more evilness -- Xerves
void weather_update()
{
   FRONT_DATA *frt;
   FRONT_DATA *fn;
   int num;
   int cnt;
   int x;
   int y;
   int map;
   int stx;
   int endx;

   if (snow_checks < 0)
      snow_checks = 0;
   if (mweather < 0 || mweather >= MAP_MAX)
      mweather = 0;
   if (mweather == 0)
      snow_checks++;
   if (snow_checks > 20)
      snow_checks = 1;

   map = mweather;

   for (x = 1; x <= MAX_X; x++)
      for (y = 1; y <= MAX_Y; y++)
      {
         if (weather_sector[mweather][x][y] < 0)
            weather_sector[mweather][x][y] = 0;

         if (weather_sector[mweather][x][y] > 10)
            weather_sector[mweather][x][y] = weather_sector[mweather][x][y] - (weather_sector[mweather][x][y] % 10);
         else
            weather_sector[mweather][x][y] = 0;
      }
   stx = (25 * snow_checks) - 24;
   endx = (25 * snow_checks);

   for (x = stx; x <= endx; x++)
   {
      for (y = 1; y <= MAX_Y; y++)
      {
         int temp;

         if (weather_sector[map][x][y] > 9) //Snow
         {
            temp = generate_temperature(NULL, x, y, map);
            if (temp >= 25 && temp <= 32)
            {
               if (number_range(1, 3) == 1)
                  weather_sector[map][x][y] -= 10;
            }
            if (temp >= 33 && temp <= 37)
            {
               weather_sector[map][x][y] -= 10;
            }
            if (temp >= 38 && temp <= 40)
            {
               weather_sector[map][x][y] -= 20;
            }
            if (temp >= 41 && temp <= 45)
            {
               weather_sector[map][x][y] -= 30;
            }
            if (temp > 45 && temp <= 60)
            {
               weather_sector[map][x][y] -= 50;
            }
            if (temp > 60)
            {
               weather_sector[map][x][y] = 0;
            }
            if (weather_sector[map][x][y] < 0)
               weather_sector[map][x][y] = 0;
         }
      }
   }
   if (number_range(1, 8) == 1)
      save_snow();

   for (frt = first_front; frt; frt = fn)
   {
      fn = frt->next;

      if (frt->map != mweather)
         continue;

      for (cnt = 1; cnt <= frt->speed; cnt++)
      {
         if (frt->type == 0) //Cold
         {
            num = number_range(1, 3);
            {
               if (num == 1) // East
               {
                  frt->x++;
               }
               if (num == 2) //South
               {
                  frt->y++;
               }
               if (num == 3) //SE
               {
                  frt->x++;
                  frt->y++;
               }
            }
         }
         else //Warm Front
         {
            num = number_range(1, 3);
            {
               if (num == 1) // West
               {
                  frt->x--;
               }
               if (num == 2) //South
               {
                  frt->y++;
               }
               if (num == 3) //SW
               {
                  frt->x--;
                  frt->y++;
               }
            }
         }
      }
      if (frt->x >= MAX_X || frt->x <= 1 || frt->y >= MAX_Y)
      {
         UNLINK(frt, first_front, last_front, next, prev);
         DISPOSE(frt);
         continue;
      }
      update_local_weather(frt, 0);
   }

   for (x = 1; x <= MAX_X; x++)
   {
      for (y = 1; y <= MAX_Y; y++)
      {
         if (weather_sector[mweather][x][y] % 10 == 9) //Tornado Time perhaps
         {
            int temp = 0;
            int tcnt = 0;
            int ty, tx;
            TORNADO_DATA *torn;

            cnt = 0;

            for (tx = x - 2; tx <= x + 2; tx++)
            {
               for (ty = y - 2; ty <= y + 2; ty++)
               {
                  if (tx < 1 || ty < 1 || tx > MAX_X || ty > MAX_Y)
                     continue;
                  if (weather_sector[mweather][tx][ty] == 9)
                  {
                     temp += generate_temperature(NULL, mweather, tx, ty);
                     tcnt++;
                     cnt++;
                  }
               }
            }
            if (tcnt > 0)
               temp = temp / tcnt;

            if (cnt > 9 && temp >= 40 && temp <= 65 && number_range(1, 1000) <= 3) //.3 percent
            {
               CREATE(torn, TORNADO_DATA, 1);
               torn->x = x;
               torn->y = y;
               torn->map = mweather;
               torn->power = cnt;
               torn->turns = 25 + cnt;
               torn->dir = number_range(0, 3);
               LINK(torn, first_tornado, last_tornado, next, prev);
            }
         }
      }
   }
   if (number_range(1, 50) == 1)
   {
      int fcnt = 0;

      for (frt = first_front; frt; frt = frt->next)
      {
         fcnt++;
      }
      if (fcnt < 30)
         create_front(number_range(0, 1));
   }
   return;
}

/*
 * get weather echo messages according to area weather...
 * stores echo message in weath_data.... must be called before
 * the vectors are adjusted
 * Last Modified: August 10, 1997
 * Fireblade
 */
void get_weather_echo()
{
   return;
}

/*
 * get echo messages according to time changes...
 * some echoes depend upon the weather so an echo must be
 * found for each area
 * Last Modified: August 10, 1997
 * Fireblade
 */
void get_time_echo(CHAR_DATA * ch)
{
   int n;

   n = number_bits(2);

   switch (gethour())
   {
      case 5:
         {
            char *echo_strings[4] = {
               "The day has begun.\n\r",
               "The day has begun.\n\r",
               "The sky slowly begins to glow.\n\r",
               "The sun slowly embarks upon a new day.\n\r"
            };

            time_info.sunlight = SUN_RISE;
            set_char_color(AT_YELLOW, ch);
            ch_printf(ch, "%s", echo_strings[n]);
            break;
         }
      case 6:
         {
            char *echo_strings[4] = {
               "The sun rises in the east.\n\r",
               "The sun rises in the east.\n\r",
               "The hazy sun rises over the horizon.\n\r",
               "Day breaks as the sun lifts into the sky.\n\r"
            };

            time_info.sunlight = SUN_LIGHT;
            set_char_color(AT_ORANGE, ch);
            ch_printf(ch, "%s", echo_strings[n]);
            break;
         }
      case 12:
         {
            time_info.sunlight = SUN_LIGHT;
            set_char_color(AT_WHITE, ch);
            send_to_char("It's noon.\n\r", ch);
            break;
         }
      case 19:
         {
            char *echo_strings[4] = {
               "The sun slowly disappears in the west.\n\r",
               "The reddish sun sets past the horizon.\n\r",
               "The sky turns a reddish orange as the sun " "ends its journey.\n\r",
               "The sun's radiance dims as it sinks in the " "sky.\n\r"
            };

            time_info.sunlight = SUN_SET;
            set_char_color(AT_RED, ch);
            ch_printf(ch, "%s", echo_strings[n]);
            break;
         }
      case 20:
         {
            char *echo_strings[2] = {
               "The moon's gentle glow diffuses " "through the night sky.\n\r",
               "The night sky gleams with " "glittering starlight.\n\r"
            };

            time_info.sunlight = SUN_DARK;
            set_char_color(AT_DBLUE, ch);
            ch_printf(ch, "%s", echo_strings[n % 2]);
            break;
         }
   }

   return;
}

/*
 * update the time
 */
void time_update()
{
   DESCRIPTOR_DATA *d;

   if (sprevhour == 0) //Have to add 1 so we know if it has been init yet.
      sprevhour = gethour() + 1;

   if (sprevhour - 1 != gethour())
   {
      sprevhour = gethour() + 1;
      switch (gethour())
      {
         case 5:
         case 6:
         case 12:
         case 19:
         case 20:
            /* for(pArea = first_area; pArea;
               pArea = (pArea == last_area) ? first_build : pArea->next)
               {
               get_time_echo(pArea->weather);
               }

               for(d = first_descriptor; d; d = d->next)
               {
               if(d->connected == CON_PLAYING &&
               IS_OUTSIDE(d->character) &&
               IS_AWAKE(d->character))
               {
               weath = d->character->in_room->area->weather;
               if(!weath->echo)
               continue;
               set_char_color(weath->echo_color,
               d->character);
               ch_printf(d->character, weath->echo);
               }
               } */
            for (d = first_descriptor; d; d = d->next)
            {
               if (d->connected == CON_PLAYING && IS_OUTSIDE(d->character) && IS_AWAKE(d->character))
               {
                  get_time_echo(d->character);
               }
            }
            break;
      }
   }
   return;
}
