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
 *			    Battle & death module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


extern char lastplayercmd[MIL];
extern CHAR_DATA *gch_prev;
extern int barena;

int blockdam;
int g_tohit;
int g_roll;
int g_lowtohit;
int g_hitohit;

/*
 * Local functions.
 */
void new_dam_message args((CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, OBJ_DATA * obj, int spec, int limb));
void death_cry args((CHAR_DATA * ch));
void group_gain args((CHAR_DATA * ch, CHAR_DATA * victim));
int xp_compute args((CHAR_DATA * gch, CHAR_DATA * victim));
int align_compute args((CHAR_DATA * gch, CHAR_DATA * victim));
ch_ret one_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt, int limb));
int obj_hitroll args((OBJ_DATA * obj));
void show_condition args((CHAR_DATA * ch, CHAR_DATA * victim));

/*
 * Check to see if player's attacks are (still?) suppressed
 * #ifdef TRI
 */
bool is_attack_supressed(CHAR_DATA * ch)
{
   TIMER *timer;

   if (IS_NPC(ch))
      return FALSE;

   timer = get_timerptr(ch, TIMER_ASUPRESSED);

   if (!timer)
      return FALSE;

   /* perma-supression -- bard? (can be reset at end of fight, or spell, etc) */
   if (timer->value == -1)
      return TRUE;

   /* this is for timed supressions */
   if (timer->count >= 1)
      return TRUE;

   return FALSE;
}

int get_empty_limb(CHAR_DATA *victim)
{
   int limb;
   
   if(get_eq_char(victim, WEAR_NECK) == NULL)
   {
      limb = LM_NECK;
      return limb;
   }
   if(get_eq_char(victim, WEAR_HEAD) == NULL)
   {
      limb = LM_HEAD;
      return limb;
   }         
   if(get_eq_char(victim, WEAR_BODY) == NULL)
   {
      limb = LM_BODY;
      return limb;
   }
   if(get_eq_char(victim, WEAR_ARM_R) == NULL)
   {
      limb = LM_RARM;
      return limb;
   }
   if(get_eq_char(victim, WEAR_ARM_L) == NULL)
   {
      limb = LM_LARM;
      return limb;
   }
   if(get_eq_char(victim, WEAR_LEG_R) == NULL)
   {
      limb = LM_RLEG;
      return limb;
   }
   if(get_eq_char(victim, WEAR_LEG_L) == NULL)
   {
      limb = LM_LLEG;
      return limb;
   }
   return -1;
}

int get_random_limb(CHAR_DATA *victim, int rnum)
{
   int limb = LM_BODY;
   
   if(rnum == 1)
      limb = LM_RARM;
   if(rnum == 2)
      limb = LM_LARM;
   if(rnum == 3)
      limb = LM_RLEG;
   if(rnum == 4)
      limb = LM_LLEG;
   if(rnum == 5)
      limb = LM_BODY;
   if(rnum == 6)
      limb = LM_HEAD;
   if(rnum == 7)
     limb = LM_NECK;
                
   return limb;
}

int get_weapon_limb(CHAR_DATA *victim)
{
   int limb;
   
   if (IS_NPC(victim))
      return -1;
      
   if (get_eq_char(victim, WEAR_WIELD) != NULL)
   {
      if (victim->pcdata->righthanded == 1)
         limb = LM_RARM;
      else
         limb = LM_LARM;
      return limb;
   }
   else
      return -1;
}

int get_tohit_diff(CHAR_DATA *ch, CHAR_DATA *victim, int wear, int type)
{
    OBJ_DATA *weapon;
    OBJ_DATA *armor;
    int bashmod = 0;
    int slashmod = 0;
    int stabmod = 0;
    
    weapon = get_eq_char(ch, WEAR_WIELD);
    if (!weapon)
    {
       bashmod += ch->tohitbash;
       stabmod += ch->tohitstab;
       slashmod += ch->tohitslash;
    }
    else
    {
       bashmod += weapon->value[7];
       stabmod += weapon->value[9];
       slashmod += weapon->value[8];
    }
    armor = get_eq_char(victim, wear);
    if (!armor && IS_NPC(victim))
    {
       bashmod -= victim->armor;
       stabmod -= victim->armor;
       slashmod -= victim->armor;
    }
    else if (armor)
    {
       bashmod -= armor->value[0];
       stabmod -= armor->value[2];
       slashmod -= armor->value[1];
    }
    if (bashmod > stabmod && bashmod > slashmod)
    {
       if (type == 1)
          return bashmod;
       else
          return GRIP_BASH;    
    }
    if (stabmod > bashmod && stabmod > slashmod)
    {
       if (type == 1)
          return stabmod;
       else
          return GRIP_STAB;  
    }
    if (slashmod > stabmod && slashmod > bashmod)
    {
       if (type == 1)
          return slashmod;
       else
          return GRIP_SLASH;  
    }
    if (bashmod == slashmod || bashmod == stabmod)
    {
       if (type == 1)
          return bashmod;
       else
          return GRIP_BASH;  
    }
          
    if (stabmod == slashmod)
    {
       
       if (type == 1)
          return slashmod;
       else
          return GRIP_SLASH;            
    }      
    
    if (type == 1)
       return -30;
    else
       return GRIP_BASH;
}
    
int get_best_grip(CHAR_DATA *ch, CHAR_DATA *victim)
{
   int grip = GRIP_BASH;
   int diff = -30;
   
   grip = get_tohit_diff(ch, victim, WEAR_NECK, 2);
   diff = get_tohit_diff(ch, victim, WEAR_NECK, 1);
   if (get_tohit_diff(ch, victim, WEAR_HEAD, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_HEAD, 1);
      grip = get_tohit_diff(ch, victim, WEAR_HEAD, 2);
   }
   if (get_tohit_diff(ch, victim, WEAR_ARM_R, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_ARM_R, 1); 
      grip = get_tohit_diff(ch, victim, WEAR_ARM_R, 2);
   }
   if (get_tohit_diff(ch, victim, WEAR_ARM_L, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_ARM_L, 1);
      grip = get_tohit_diff(ch, victim, WEAR_ARM_L, 2);
   }
   if (get_tohit_diff(ch, victim, WEAR_LEG_R, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_LEG_R, 1);
      grip = get_tohit_diff(ch, victim, WEAR_LEG_R, 2);
   }
   if (get_tohit_diff(ch, victim, WEAR_LEG_L, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_LEG_L, 1);
      grip = get_tohit_diff(ch, victim, WEAR_LEG_L, 2);
   }
   if (get_tohit_diff(ch, victim, WEAR_BODY, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_BODY, 1);
      grip = get_tohit_diff(ch, victim, WEAR_BODY, 2);
   } 
   return grip;
}       
    
int get_best_diff(CHAR_DATA *ch, CHAR_DATA *victim, int type)
{
   int diff = -30;
   int limb = LM_BODY;
   
   diff = get_tohit_diff(ch, victim, WEAR_NECK, 1);
   limb = LM_NECK;
   if (get_tohit_diff(ch, victim, WEAR_HEAD, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_HEAD, 1);
      limb = LM_HEAD;
   }
   if (get_tohit_diff(ch, victim, WEAR_ARM_R, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_ARM_R, 1);
      limb = LM_RARM;
   }
   if (get_tohit_diff(ch, victim, WEAR_ARM_L, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_ARM_L, 1);
      limb = LM_LARM;
   }
   if (get_tohit_diff(ch, victim, WEAR_LEG_R, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_LEG_R, 1);
      limb = LM_RLEG;
   }
   if (get_tohit_diff(ch, victim, WEAR_LEG_L, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_LEG_L, 1);
      limb = LM_LLEG;
   }
   if (get_tohit_diff(ch, victim, WEAR_BODY, 1) > diff)
   {
      diff = get_tohit_diff(ch, victim, WEAR_BODY, 1);
      limb = LM_BODY;
   }
   
   if (type == 1)
      return diff;
   else
      return limb;   
}  

void do_emptycorpses(CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *corpse;
   OBJ_DATA *contents;
   OBJ_DATA *next_content;
   
   for (corpse = ch->in_room->first_content; corpse; corpse = corpse->next_content)
   {
      if (IN_SAME_ROOM_OBJ(ch, corpse))
      {
         if (corpse->item_type == ITEM_CORPSE_NPC)
         {
            for (contents = corpse->first_content; contents; contents = next_content)
            {
               next_content = contents->next_content;
               obj_from_obj(contents);
               obj_to_room(contents, ch->in_room, ch);
            }
         }
      }
   }
   send_to_char("All NPC corpses in this room are now empty.\n\r", ch);
   return;
}

/* Target Limb choosing function */
int target_chooser(CHAR_DATA * ch, CHAR_DATA * victim )
{
    int mobint, mobwis, rnum, limb;
    OBJ_DATA *weapon;
    int diff;
    
    /*
    LM_RARM
    LM_LARM
    LM_RLEG
    LM_LLEG
    LM_BODY
    LM_HEAD
    LM_NECK
    */
    mobint = get_curr_int(ch);
    mobwis = get_curr_wis(ch);
    
    if ((ch->pIndexData->vnum >= OVERLAND_LOW_MOB && ch->pIndexData->vnum <= OVERLAND_HI_MOB) && ch->m1 == 1) //can just set the stats low, but incase
    {
       limb = LM_BODY;
       return limb;
    }
    if(mobint < 8) //target body
    {
       limb = LM_BODY;
       return limb;
    }
    if(mobint >= 8 && mobint <= 11) //target random part
    {
       rnum = number_range(1,7);
       limb = get_random_limb(victim, rnum);
       return limb;
    }
    if ((weapon = get_eq_char(ch, WEAR_WIELD)) != NULL)
    {      
       if (weapon->value[7] > weapon->value[8] && weapon->value[7] > weapon->value[9])
          ch->grip = GRIP_BASH;
       if (weapon->value[8] > weapon->value[7] && weapon->value[8] > weapon->value[9])
          ch->grip = GRIP_SLASH;
       if (weapon->value[9] > weapon->value[7] && weapon->value[9] > weapon->value[8])
          ch->grip = GRIP_STAB;
       if (weapon->value[7] == weapon->value[8] || weapon->value[7] == weapon->value[9])
          ch->grip = GRIP_BASH;
       if (weapon->value[8] == weapon->value[9])
          ch->grip = GRIP_SLASH;
    }
    if(mobint >= 12 && mobint <= 14) //target empty limb or random part
    {
       limb = get_empty_limb(victim);
       if (limb < 0)
       {
          rnum = number_range(1,7);
          limb = get_random_limb(victim, rnum);
       }
       return limb;
    }
    if (mobint >= 15 && mobint <= 17) //target empty slots, weapon hand, or random part
    {
        if (mobwis < 15)
        {
           limb = get_empty_limb(victim);
           if (limb < 0)
              limb = get_weapon_limb(victim);
           if (limb < 0)
           {
              rnum = number_range(1,7);
              limb = get_random_limb(victim, rnum);
           }          
           return limb;
        }
        else //target arms or legs instead of rest of target (if no weapon/empty slots)
        {          
           limb = get_empty_limb(victim);
           if (limb < 0)
           {
              if (number_range(1, 5) <= 2)
              {
                 limb = get_weapon_limb(victim);
              }
           }
           if (limb < 0)
           {
              rnum = number_range(1,4);
              limb = get_random_limb(victim, rnum);
           }    
        }
        return limb;
     }
     if (mobint >= 18 && mobint <= 21) //target limb based on equipment matchups, higher the wisdom, the better it can matchup
     {
        if (mobwis < 18)
        {
           limb = get_empty_limb(victim);
           if (limb < 0)
           {
              diff = get_best_diff(ch, victim, 1);
              if (diff >= 3)
              {
                 limb = get_best_diff(ch, victim, 2);
                 ch->grip = get_best_grip(ch, victim);
              }
           }
           if (limb < 0)
           {
              if (number_range(1, 5) >= 3)
                 limb = get_weapon_limb(victim);
           }
           if (limb < 0)
           {
              rnum = number_range(1,4);
              limb = get_random_limb(victim, rnum);
           }
           return limb;
        }
        else
        {  
           limb = get_empty_limb(victim);
           if (limb < 0)
           {
              diff = get_best_diff(ch, victim, 1);
              if (diff >= 1)
              {
                 limb = get_best_diff(ch, victim, 2);
                 ch->grip = get_best_grip(ch, victim);
              }
           }
           if (limb < 0)
              limb = get_weapon_limb(victim);
           if (limb < 0)
           {
              rnum = number_range(1,4);
              limb = get_random_limb(victim, rnum);
           }
           return limb;
        }
     }
     if (mobint > 21)  //Target best possible limb, high wisdom mobs will make BEST decision possible
     {
        if (mobwis < 22)
        {
           limb = get_empty_limb(victim);
           if (limb < 0)
           {
              diff = get_best_diff(ch, victim, 1);
              limb = get_best_diff(ch, victim, 2);
              ch->grip = get_best_grip(ch, victim);
           }
           return limb;
        }
        else
        {
           limb = get_empty_limb(victim); //empty slot?
           if (limb < 0) //Good armor target?
           {
              diff = get_best_diff(ch, victim, 1);
              if (diff >= 1)
              {
                 limb = get_best_diff(ch, victim, 2);
                 ch->grip = get_best_grip(ch, victim);
              }
           }
           if (limb < 0) //Hurt limb?
           {
              if (victim->con_larm < 400 || victim->con_rarm < 400 || victim->con_rleg < 400 || victim->con_rleg < 400) //target hurt limbs
              {
                 if (victim->con_rleg < 400 && victim->con_rleg <= victim->con_lleg && victim->con_rleg <= victim->con_rarm
                 && victim->con_rleg <= victim->con_larm)
                    return LM_RLEG;
                 
                 if (victim->con_lleg < 400 && victim->con_lleg <= victim->con_rleg && victim->con_lleg <= victim->con_rarm
                 && victim->con_lleg <= victim->con_larm)
                    return LM_LLEG;
                 
                 if (!IS_NPC(victim) && victim->pcdata->righthanded == 0) //in case of a tie target the wielding limb
                 {
                    if (victim->con_rarm < 400 && victim->con_rarm <= victim->con_larm && victim->con_rarm <= victim->con_rleg
                    && victim->con_rarm <= victim->con_lleg)
                       return LM_RARM;
                    
                    if (victim->con_larm < 400 && victim->con_larm <= victim->con_rarm && victim->con_larm <= victim->con_rleg
                    && victim->con_larm <= victim->con_lleg)
                       return LM_LARM;
                 }
                 else
                 {                 
                    if (victim->con_larm < 400 && victim->con_larm <= victim->con_rarm && victim->con_larm <= victim->con_rleg
                    && victim->con_larm <= victim->con_lleg)
                       return LM_LARM;
                    
                    if (victim->con_rarm < 400 && victim->con_rarm <= victim->con_larm && victim->con_rarm <= victim->con_rleg
                    && victim->con_rarm <= victim->con_lleg)
                       return LM_RARM;
                 }
              }
           }
           if (limb < 0) //damaged eq?
           {
              OBJ_DATA *eq;
              if (((eq = get_eq_char(victim, WEAR_NECK)) != NULL) && eq->value[3] < 300)
                 return LM_NECK;
              if (((eq = get_eq_char(victim, WEAR_HEAD)) != NULL) && eq->value[3] < 300)          
                 return LM_HEAD;
              if (!IS_NPC(victim) && victim->pcdata->righthanded == 0)
              {
                 if (((eq = get_eq_char(victim, WEAR_ARM_R)) != NULL) && eq->value[3] < 300)
                    return LM_RARM;
              }
              else
              {
                 if (((eq = get_eq_char(victim, WEAR_ARM_L)) != NULL) && eq->value[3] < 300)               
                     return LM_LARM;
              }
              if (((eq = get_eq_char(victim, WEAR_LEG_R)) != NULL) && eq->value[3] < 300)
                 return LM_RLEG;
              if (((eq = get_eq_char(victim, WEAR_LEG_L)) != NULL) && eq->value[3] < 300)
                 return LM_LLEG;
              if (((eq = get_eq_char(victim, WEAR_BODY)) != NULL) && eq->value[3] < 300)
                 return LM_BODY;
           }
           if (limb < 0) //all else fails just find the best
           {
              limb = get_best_diff(ch, victim, 2);
              ch->grip = get_best_grip(ch, victim);
              return limb;
           }
           return limb;
        }
     }
     return LM_BODY;
}

/*
 * Check to see if weapon is poisoned.
 */
bool is_wielding_poisoned(CHAR_DATA * ch)
{
   OBJ_DATA *obj;

   if ((obj = get_eq_char(ch, WEAR_WIELD)) != NULL && IS_OBJ_STAT(obj, ITEM_POISONED))
      return TRUE;

   return FALSE;
}

/*
 * hunting, hating and fearing code				-Thoric
 */
bool is_hunting(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!ch->hunting || ch->hunting->who != victim)
      return FALSE;

   return TRUE;
}

bool is_hating(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!ch->hating || ch->hating->who != victim)
      return FALSE;

   return TRUE;
}

bool is_fearing(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!ch->fearing || ch->fearing->who != victim)
      return FALSE;

   return TRUE;
}

void stop_hunting(CHAR_DATA * ch)
{
   if (ch->hunting)
   {
      STRFREE(ch->hunting->name);
      DISPOSE(ch->hunting);
      ch->hunting = NULL;
   }
   return;
}

void stop_hating(CHAR_DATA * ch)
{
   if (ch->hating)
   {
      STRFREE(ch->hating->name);
      DISPOSE(ch->hating);
      ch->hating = NULL;
   }
   return;
}

void stop_fearing(CHAR_DATA * ch)
{
   if (ch->fearing)
   {
      STRFREE(ch->fearing->name);
      DISPOSE(ch->fearing);
      ch->fearing = NULL;
   }
   return;
}

void start_hunting(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!IS_NPC(ch))
      return;
   if (ch->hunting)
      stop_hunting(ch);

   CREATE(ch->hunting, HHF_DATA, 1);
   ch->hunting->name = QUICKLINK(victim->name);
   ch->hunting->who = victim;
   return;
}

void start_hating(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!IS_NPC(ch))
      return;
   if (ch->hating)
      stop_hating(ch);

   CREATE(ch->hating, HHF_DATA, 1);
   ch->hating->name = QUICKLINK(victim->name);
   ch->hating->who = victim;
   return;
}

void start_fearing(CHAR_DATA * ch, CHAR_DATA * victim)
{
   if (!IS_NPC(ch))
      return;
   if (ch->fearing)
      stop_fearing(ch);

   CREATE(ch->fearing, HHF_DATA, 1);
   ch->fearing->name = QUICKLINK(victim->name);
   ch->fearing->who = victim;
   return;
}

/*
 * Get the current armor class for a vampire based on time of day
 */
sh_int VAMP_AC(CHAR_DATA * ch)
{
   if (IS_VAMPIRE(ch) && IS_OUTSIDE(ch))
   {
      switch (time_info.sunlight)
      {
         case SUN_DARK:
            return -10;
         case SUN_RISE:
            return 5;
         case SUN_LIGHT:
            return 10;
         case SUN_SET:
            return 2;
         default:
            return 0;
      }
   }
   else
      return 0;
}

int max_fight(CHAR_DATA * ch)
{
   return 20;
}

bool has_artifact(CHAR_DATA * ch)
{
   OBJ_DATA *obj;

   if (xIS_SET(ch->in_room->room_flags, ROOM_TSAFE))
      return FALSE;
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
   {
      if (IS_OBJ_STAT(obj, ITEM_ARTIFACT)) //Found the artifact
         return TRUE;
   }
   return FALSE;
}

void do_stack(CHAR_DATA *ch, char *argument)
{
   char buf[MSL];
   if (check_npc(ch))
      return;   
      
    if (strlen(argument) + strlen(ch->pcdata->stackbuf) >= MSL-5)
    {
       send_to_char("You can only have roughly 4000 characters in your command stack, clear it if there is a problem.\n\n", ch);
       return;
    }
    if (argument[0] == '\0')
    {
       send_to_char("You must provide an argument.\n\r", ch);
       return;
    }
    sprintf(buf, "/%s/", argument);;
       
    strcat(ch->pcdata->stackbuf, buf);
    return;
}

void do_clearstack(CHAR_DATA *ch, char *argument)
{
   if (check_npc(ch))
      return; 
      
    strcpy(ch->pcdata->stackbuf, "");
    send_to_char("Cleared.\n\r", ch);
    return;
}
    
void parse_attack_buffer(char *fullbuf, char *cbuf)
{
   int x = 0;
   int z = 0;
   int cnt = 0;
   char buf[MSL];
   int y = 0;
   for (;;)
   {
      if (fullbuf[x] == '\0')
      {
         strcpy(fullbuf, "");
         strcpy(cbuf, "");
         return;
      }
      if (fullbuf[x] == '/')
      {
         cnt++;
         if (cnt == 2)
         {
            cbuf[z] = '\0';
            x++;
            for (;;)
            {
               if (fullbuf[x] != '\0')
                   buf[y++] = fullbuf[x++];
               else
                   break;
            }
            buf[y] = '\0';
            sprintf(fullbuf, buf);
            return;
         }
         x++;
      }
      else
      {
         cbuf[z++] = fullbuf[x++];
      }
   }
}
      

/*
 * Control the fights going on.
 * Called periodically by update_handler.
 * Many hours spent fixing bugs in here by Thoric, as noted by residual
 * debugging checks.  If you never get any of these error messages again
 * in your logs... then you can comment out some of the checks without
 * worry.
 *
 * Note:  This function also handles some non-violence updates.
 */
void violence_update(void)
{
   char buf[MSL];
   CHAR_DATA *ch;
   CHAR_DATA *lst_ch;
   CHAR_DATA *ich;
   CHAR_DATA *victim;
   CHAR_DATA *rch, *rch_next;
   TIMER *timer, *timer_next;
   ch_ret retcode;
   OBJ_DATA *shield;
   int attacktype, cnt;
   static int pulse = 0;
   int range;
   AGGRO_DATA *aggro;
   AGGRO_DATA *naggro;
   int haggro = -1;
   char abuf[100];
   char target[10];
   char commandbuf[MSL];

   lst_ch = NULL;
   pulse = (pulse + 1) % 100;

   for (ch = last_char; ch; lst_ch = ch, ch = gch_prev)
   {
      set_cur_char(ch);

      if (ch == first_char && ch->prev)
      {
         bug("ERROR: first_char->prev != NULL, fixing...", 0);
         ch->prev = NULL;
      }

      gch_prev = ch->prev;

      if (gch_prev && gch_prev->next != ch)
      {
         if (!ch->name)
         {
            bug("short-cutting");
            UNLINK(ch, first_char, last_char, next, prev);
            continue;
         }
         sprintf(buf, "FATAL: violence_update: %s->prev->next doesn't point to ch.", ch->name);
         bug(buf, 0);
         bug("Short-cutting here", 0);
         ch->prev = NULL;
         gch_prev = NULL;
         do_shout(ch, "Thoric says, 'Prepare for the worst!'");
      }

      /*
       * See if we got a pointer to someone who recently died...
       * if so, either the pointer is bad... or it's a player who
       * "died", and is back at the healer...
       * Since he/she's in the char_list, it's likely to be the later...
       * and should not already be in another fight already
       */
      if (char_died(ch))
         continue;

      /*
       * See if we got a pointer to some bad looking data...
       */
      if (!ch->in_room || !ch->name)
      {
         log_string("violence_update: bad ch record!  (Shortcutting.)");
         sprintf(buf, "ch: %d  ch->in_room: %d  ch->prev: %d  ch->next: %d", (int) ch, (int) ch->in_room, (int) ch->prev, (int) ch->next);
         log_string(buf);
         log_string(lastplayercmd);
         if (lst_ch)
            sprintf(buf, "lst_ch: %d  lst_ch->prev: %d  lst_ch->next: %d", (int) lst_ch, (int) lst_ch->prev, (int) lst_ch->next);
         else
            strcpy(buf, "lst_ch: NULL");
         log_string(buf);
         gch_prev = NULL;
         continue;
      }

      /*
       * Experience gained during battle deceases as battle drags on
       */
      if (ch->fighting)
         if ((++ch->fighting->duration % 24) == 0)
            ch->fighting->xp = ((ch->fighting->xp * 9) / 10);

      if (ch->fighting && ch->fighting->who && ch->fighting->who->name)
      {     
         int hking;
         int iking;
         
         if (IS_NPC(ch->fighting->who))
            hking = ch->fighting->who->m4;
         else
            hking = ch->fighting->who->pcdata->hometown;
            
         for (ich = ch->in_room->first_person; ich; ich = ich->next_in_room)
         {
            if (ich == ch)
               continue;
            range = ich->m5;
            if (IS_NPC(ich))
               iking = ich->m4;
            else
               iking = ich->pcdata->hometown;
                  
            if (IN_WILDERNESS(ch))
            {
               if (abs(ich->coord->x - ch->coord->x) <= range && abs(ich->coord->y - ch->coord->y) <= range && ich->map == ch->map)
               {
                  if (IS_NPC(ich) && ch->fighting && !ich->hating && !ich->hunting && !ich->fighting && ch->fighting->who != ich
                     && number_range(1, 100) <= 70 && xIS_SET(ich->act, ACT_MILITARY) && !xIS_SET(ich->miflags, KM_NOASSIST)
                     && can_see_map(ich, ch->fighting->who) && hking != iking && !is_safe(ich, ch->fighting->who))
                  {
                     start_hating(ich, ch->fighting->who);
                     start_hunting(ich, ch->fighting->who);
                     break;
                  }
               }
            }
            else
            {
               if (IS_NPC(ich) && ch->fighting && !ich->hating && !ich->hunting && !ich->fighting && ch->fighting->who != ich
                  && number_range(1, 100) <= 70 && xIS_SET(ich->act, ACT_MILITARY) && !xIS_SET(ich->miflags, KM_NOASSIST)
                  && can_see_map(ich, ch->fighting->who) && hking != iking && !is_safe(ich, ch->fighting->who))
               {
                  start_hating(ich, ch->fighting->who);
                  start_hunting(ich, ch->fighting->who);
                  break;
               }
            }
         }
      }
      for (timer = ch->first_timer; timer; timer = timer_next)
      {
         timer_next = timer->next;
         if (--timer->count <= 0)
         {
            if (timer->type == TIMER_ASUPRESSED)
            {
               if (timer->value == -1)
               {
                  timer->count = 1000;
                  continue;
               }
            }
            if (timer->type == TIMER_NUISANCE)
            {
               DISPOSE(ch->pcdata->nuisance);
            }

            if (timer->type == TIMER_DO_FUN)
            {
               int tempsub;

               tempsub = ch->substate;
               ch->substate = timer->value;
               (timer->do_fun) (ch, "");
               if (char_died(ch))
                  break;
               ch->substate = tempsub;
            }
            extract_timer(ch, timer);
         }
      }

      if (char_died(ch))
         continue;
      
      /*
      for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
      {
         if (aggro->ch)
            bug("%d > %s", aggro->value, aggro->ch->name);
      }
      */
         
      for (aggro = ch->first_aggro; aggro; aggro = naggro)
      {
         naggro = aggro->next;
         if (number_range(1, 20) == 1) //10 seconds roughly
            aggro->value-=1;
         if (aggro->value <= 0)
         {
            UNLINK(aggro, ch->first_aggro, ch->last_aggro, next, prev);
            UNLINK(aggro, first_global_aggro, last_global_aggro, next_global, prev_global);
            DISPOSE(aggro);
         }
      }
         
      //Do not attack this round if there is a fight timer
      if (ch->fight_timer > 0)
      {
         ch->fight_timer--;
         if (ch->fight_timer == 0)
            ch->fight_state = 0;
         continue;
      }
      if (IS_NPC(ch) && xIS_SET(ch->act, ACT_NOATTACK))
         continue;

      /* check for exits moving players around */
      if ((retcode = pullcheck(ch, pulse)) == rCHAR_DIED || char_died(ch))
         continue;

      /* Let the battle begin! */

      if ((victim = who_fighting(ch)) == NULL || IS_AFFECTED(ch, AFF_PARALYSIS))
         continue;
         
      if (!IN_SAME_ROOM(ch, victim))
      {
         stop_fighting(ch, FALSE);
         continue;
      }
      
  if( IS_NPC(ch) )
  {
      retcode = rNONE;

      if (is_safe(victim, ch) && !has_artifact(ch) && !has_artifact(victim))
      {
         sprintf(buf, "violence_update: %s fighting %s in a SAFE room.", ch->name, victim->name);
         log_string(buf);
         stop_fighting(ch, TRUE);
      }
      else if (IS_AWAKE(ch) && in_same_room(ch, victim))
      {
         int fired = 0;
         /*
          *  Mob triggers
          */
         rprog_rfight_trigger(ch);
         if (char_died(ch))
            continue;
         mprog_hitprcnt_trigger(ch, victim);
         if (char_died(ch))
            continue;
         mprog_fight_trigger(ch, victim);
         if (char_died(ch))
            continue;
       
         /*
          * NPC special attack flags    -Thoric
          */
         if (IS_NPC(ch))
         {
            if (!xIS_EMPTY(ch->attacks))
            {
               attacktype = -1;
               if (15 >= number_percent())
               {
                  cnt = 0;
                  for (;;)
                  {
                     if (cnt++ > 100)
                     {
                        attacktype = -1;
                        break;
                     }
                     attacktype = number_range(7, MAX_ATTACK_TYPE - 1);
                     if (xIS_SET(ch->attacks, attacktype))
                     {
                        fired = 1;
                        break;
                     }
                  }
                  switch (attacktype)
                  {
                     case ATCK_BASH:
                        break;
                     case ATCK_STUN:
                        do_stun(ch, "");
                        retcode = global_retcode;
                        break;
                     case ATCK_GOUGE:
                        do_gouge(ch, "");
                        retcode = global_retcode;
                        break;
                     case ATCK_FEED:
                        break;
                     case ATCK_DRAIN:
                        break;
                     case ATCK_FIREBREATH:
                        retcode = spell_fire_breath(skill_lookup("fire breath"), ch->level, ch, victim);
                        break;
                     case ATCK_FROSTBREATH:
                        retcode = spell_frost_breath(skill_lookup("frost breath"), ch->level, ch, victim);
                        break;
                     case ATCK_ACIDBREATH:
                        retcode = spell_acid_breath(skill_lookup("acid breath"), ch->level, ch, victim);
                        break;
                     case ATCK_LIGHTNBREATH:
                        retcode = spell_lightning_breath(skill_lookup("lightning breath"), ch->level, ch, victim);
                        break;
                     case ATCK_GASBREATH:
                        retcode = spell_gas_breath(skill_lookup("gas breath"), ch->level, ch, victim);
                        break;
                     case ATCK_SPIRALBLAST:
                        break;
                     case ATCK_POISON:
                        retcode = spell_poison(gsn_poison, ch->level, ch, victim);
                        break;
                     case ATCK_NASTYPOISON:
                        /*
                           retcode = spell_nasty_poison( skill_lookup( "nasty poison" ), ch->level, ch, victim );
                         */
                        break;
                     case ATCK_GAZE:
                        /*
                           retcode = spell_gaze( skill_lookup( "gaze" ), ch->level, ch, victim );
                         */
                        break;
                     case ATCK_BLINDNESS:
                        retcode = spell_blindness(gsn_blindness, ch->level, ch, victim);
                        break;
                     case ATCK_CAUSESERIOUS:
                        retcode = spell_cause_serious(skill_lookup("cause serious"), ch->level, ch, victim);
                        break;
                     case ATCK_EARTHQUAKE:
                        retcode = spell_earthquake(skill_lookup("earthquake"), ch->level, ch, victim);
                        break;
                     case ATCK_CAUSECRITICAL:
                        retcode = spell_cause_critical(skill_lookup("cause critical"), ch->level, ch, victim);
                        break;
                     case ATCK_CURSE:
                        retcode = spell_curse(skill_lookup("curse"), ch->level, ch, victim);
                        break;
                     case ATCK_FLAMESTRIKE:;
                        break;
                     case ATCK_HARM:
                        retcode = spell_harm(skill_lookup("harm"), ch->level, ch, victim);
                        break;
                     case ATCK_FIREBALL:
                        break;
                     case ATCK_COLORSPRAY:
                        break;
                     case ATCK_WEAKEN:
                        retcode = spell_weaken(skill_lookup("weaken"), ch->level, ch, victim);
                        break;
                  }
                  ch->fight_timer = 0;
                  if (attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)))
                     continue;
               }
            }
            /*
             * NPC special defense flags    -Thoric
             */
            if (!xIS_EMPTY(ch->defenses))
            {
               attacktype = -1;
               if (15 > number_percent())
               {
                  cnt = 0;
                  for (;;)
                  {
                     if (cnt++ > 100)
                     {
                        attacktype = -1;
                        break;
                     }
                     attacktype = number_range(2, MAX_DEFENSE_TYPE - 1);
                     if (xIS_SET(ch->defenses, attacktype))
                     {
                        fired = 1;
                        break;
                     }
                  }
                  switch (attacktype)
                  {
                     case DFND_CURELIGHT:
                        act(AT_MAGIC, "$n mutters a few incantations...and looks a little better.", ch, NULL, NULL, TO_ROOM);
                        retcode = spell_smaug(skill_lookup("cure light"), ch->level, ch, ch);
                        break;
                     case DFND_CURESERIOUS:
                        act(AT_MAGIC, "$n mutters a few incantations...and looks a bit better.", ch, NULL, NULL, TO_ROOM);
                        retcode = spell_smaug(skill_lookup("cure serious"), ch->level, ch, ch);
                        break;
                     case DFND_CURECRITICAL:
                        act(AT_MAGIC, "$n mutters a few incantations...and looks healthier.", ch, NULL, NULL, TO_ROOM);
                        retcode = spell_smaug(skill_lookup("cure critical"), ch->level, ch, ch);
                        break;
                     case DFND_HEAL:
                        act(AT_MAGIC, "$n mutters a few incantations...and looks much healthier.", ch, NULL, NULL, TO_ROOM);
                        retcode = spell_smaug(skill_lookup("heal"), ch->level, ch, ch);
                        break;
                     case DFND_DISPELMAGIC:
                        act(AT_MAGIC, "$n utters an incantation...", ch, NULL, NULL, TO_ROOM);
                        retcode = spell_dispel_magic(skill_lookup("dispel magic"), ch->level, ch, victim);
                        break;
                     case DFND_DISPELEVIL:
                        break;
                     case DFND_SHOCKSHIELD:
                        if (!IS_AFFECTED(ch, AFF_SHOCKSHIELD))
                        {
                           act(AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
                           retcode = spell_smaug(skill_lookup("shockshield"), ch->level, ch, ch);
                        }
                        else
                           retcode = rNONE;
                        break;
                     case DFND_FIRESHIELD:
                        if (!IS_AFFECTED(ch, AFF_FIRESHIELD))
                        {
                           act(AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
                           retcode = spell_smaug(skill_lookup("fireshield"), ch->level, ch, ch);
                        }
                        else
                           retcode = rNONE;
                        break;
                     case DFND_ICESHIELD:
                        if (!IS_AFFECTED(ch, AFF_ICESHIELD))
                        {
                           act(AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
                           retcode = spell_smaug(skill_lookup("iceshield"), ch->level, ch, ch);
                        }
                        else
                           retcode = rNONE;
                        break;
                     case DFND_TRUESIGHT:
                        if (!IS_AFFECTED(ch, AFF_TRUESIGHT))
                           retcode = spell_smaug(skill_lookup("true"), ch->level, ch, ch);
                        else
                           retcode = rNONE;
                        break;
                     case DFND_SANCTUARY:
                        if (!IS_AFFECTED(ch, AFF_SANCTUARY))
                        {
                           act(AT_MAGIC, "$n utters a few incantations...", ch, NULL, NULL, TO_ROOM);
                           retcode = spell_smaug(skill_lookup("sanctuary"), ch->level, ch, ch);
                        }
                        else
                           retcode = rNONE;
                        break;
                  }
                  ch->fight_timer = 0;
                  if (attacktype != -1 && (retcode == rCHAR_DIED || char_died(ch)))
                     continue;
               }
            }
         }
         if (ch->fight_timer > 0)
            continue;
         //Still want it to fire roughly every 12 seconds per unit even if in battle, roughly comes out to about 16 or
         //so seconds with the 65 percent chance.  At 5 units that is roughly a heal every 3 seconds.
         //Mage units will fire at a much faster rate since magic is their primary way of damage and survival
         if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY) && ch->m4 > 1 && ch->m4 < sysdata.max_kingdom
         &&  ch->fight_timer == 0 && ch->fighting && ch->fighting->who && number_range(1, 24) <= get_btimer(ch, -1, NULL))
         { 
            int csn;
            BUYKMOB_DATA *kmob;
            char snbuf[MSL];
     
            for (kmob = first_buykmob; kmob; kmob = kmob->next)
            {
               if (ch->pIndexData->vnum == kmob->vnum)
               {
                  if (xIS_SET(kmob->flags, KMOB_MAGE))
                  {   
                     if (xIS_SET(kmob->flags, KMOB_MAGEDAMAGE1) || xIS_SET(kmob->flags, KMOB_MAGEDAMAGE2)
                     ||  xIS_SET(kmob->flags, KMOB_MAGEDAMAGE3) || xIS_SET(kmob->flags, KMOB_MAGEDAMAGE4))
                     {
                        int num;
                        for (;;)
                        {
                           num = number_range(1, 4);                           
                           if (num == 1 && xIS_SET(kmob->flags, KMOB_MAGEDAMAGE1))
                           {
                              num = number_range(1, 10);
                              if (num == 1)
                                 csn = skill_lookup("jagged spike");
                              else if (num == 2)
                                 csn = skill_lookup("jagged stone");
                              else if (num == 3)
                                 csn = skill_lookup("fire arrow");
                              else if (num == 4)
                                 csn = skill_lookup("fire blast");
                              else if (num == 5)
                                 csn = skill_lookup("wind chill");
                              else if (num == 6)
                                 csn = skill_lookup("wind gust");
                              else if (num == 7)
                                 csn = skill_lookup("cold ball");
                              else if (num == 8)
                                 csn = skill_lookup("frozen dart");
                              else if (num == 9)
                                 csn = skill_lookup("energy blast");
                              else
                                 csn = skill_lookup("power drain");
                              if (csn <= 0)
                                 continue;
                              sprintf(snbuf, "$n waves $s hands and casts %s on $N", skill_table[csn]->name);
                              act(AT_ACTION, snbuf, ch, NULL, ch->fighting->who, TO_ROOM);
                              spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, ch->fighting->who);
                              break;        
                           }
                           if (num == 2 && xIS_SET(kmob->flags, KMOB_MAGEDAMAGE2))
                           {
                              num = number_range(1, 10);
                              if (num == 1)
                                 csn = skill_lookup("acid blast");
                              else if (num == 2)
                                 csn = skill_lookup("acidic ball");
                              else if (num == 3)
                                 csn = skill_lookup("fire bolt");
                              else if (num == 4)
                                 csn = skill_lookup("scolding blast");
                              else if (num == 5)
                                 csn = skill_lookup("funnel");
                              else if (num == 6)
                                 csn = skill_lookup("wind spike");
                              else if (num == 7)
                                 csn = skill_lookup("frozen ball");
                              else if (num == 8)
                                 csn = skill_lookup("frozen spike");
                              else if (num == 9)
                                 csn = skill_lookup("electric blast");
                              else
                                 csn = skill_lookup("energy drain");
                              if (csn <= 0)
                                 continue;
                              sprintf(snbuf, "$n waves $s hands and casts %s on $N", skill_table[csn]->name);
                              act(AT_ACTION, snbuf, ch, NULL, ch->fighting->who, TO_ROOM);
                              spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, ch->fighting->who);
                              break;        
                           }
                           if (num == 3 && xIS_SET(kmob->flags, KMOB_MAGEDAMAGE3))
                           {
                              num = number_range(1, 10);
                              if (num == 1)
                                 csn = skill_lookup("acidic blast");
                              else if (num == 2)
                                 csn = skill_lookup("earth clamp");
                              else if (num == 3)
                                 csn = skill_lookup("blazing burst");
                              else if (num == 4)
                                 csn = skill_lookup("fireball");
                              else if (num == 5)
                                 csn = skill_lookup("tornado");
                              else if (num == 6)
                                 csn = skill_lookup("blitzing wave");
                              else if (num == 7)
                                 csn = skill_lookup("frigid ball");
                              else if (num == 8)
                                 csn = skill_lookup("frozen blast");
                              else if (num == 9)
                                 csn = skill_lookup("deanimation");
                              else
                                 csn = skill_lookup("jolting burst");
                              if (csn <= 0)
                                 continue;
                              sprintf(snbuf, "$n waves $s hands and casts %s on $N", skill_table[csn]->name);
                              act(AT_ACTION, snbuf, ch, NULL, ch->fighting->who, TO_ROOM);
                              spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, ch->fighting->who);
                              break;        
                           }
                           if (num == 4 && xIS_SET(kmob->flags, KMOB_MAGEDAMAGE4))
                           {
                              num = number_range(1, 10);
                              if (num == 1)
                                 csn = skill_lookup("acidic oblivion");
                              else if (num == 2)
                                 csn = skill_lookup("demolic blast");
                              else if (num == 3)
                                 csn = skill_lookup("dancing fire");
                              else if (num == 4)
                                 csn = skill_lookup("eradication");
                              else if (num == 5)
                                 csn = skill_lookup("heavenly blast");
                              else if (num == 6)
                                 csn = skill_lookup("meteor");
                              else if (num == 7)
                                 csn = skill_lookup("frigid death");
                              else if (num == 8)
                                 csn = skill_lookup("tidal wave");
                              else if (num == 9)
                                 csn = skill_lookup("molecular degeneration");
                              else
                                 csn = skill_lookup("quantum spike");
                              if (csn <= 0)
                                 continue;
                              sprintf(snbuf, "$n waves $s hands and casts %s on $N", skill_table[csn]->name);
                              act(AT_ACTION, snbuf, ch, NULL, ch->fighting->who, TO_ROOM);
                              spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, ch->fighting->who);
                              break;        
                           }
                        }
                        break;
                     }            
                  }              
                  if (xIS_SET(kmob->flags, KMOB_CLERIC))
                  {
                     CHAR_DATA *rch;
                     int num = number_range(1, 20);  
                     if (num > 13 && (xIS_SET(kmob->flags, KMOB_HARM) || xIS_SET(kmob->flags, KMOB_CLERICDAMAGE)))
                     {
                        num = number_range(1, 2);
                        
                        if (num == 1 && xIS_SET(kmob->flags, KMOB_HARM))
                        {
                           csn = skill_lookup("harm");
                           if (csn <= 0)
                              continue;
                           act(AT_ACTION, "$n waves $s hands and casts harm on $N.", ch, NULL, ch->fighting->who, TO_ROOM);
                           spell_harm(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, ch->fighting->who);
                           break;      
                        }
                        if (num == 2 && xIS_SET(kmob->flags, KMOB_CLERICDAMAGE))
                        {
                           num = number_range(1, 5);
                           if (num == 1)
                              csn = skill_lookup("gurori");
                           else if (num == 2)
                              csn = skill_lookup("furori");
                           else if (num == 3)
                              csn = skill_lookup("aurori");
                           else if (num == 4)
                              csn = skill_lookup("wurori");
                           else
                              csn = skill_lookup("eurori");
                           if (csn <= 0)
                              continue;
                           sprintf(snbuf, "$n waves $s hands and casts %s on $N", skill_table[csn]->name);
                           act(AT_ACTION, snbuf, ch, NULL, ch->fighting->who, TO_ROOM);
                           spell_smaug(csn, POINT_LEVEL(LEARNED(ch, csn), MASTERED(ch, csn)), ch, ch->fighting->who);
                           break;      
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
            continue;
         }
         else
         {
            retcode = one_hit(ch, victim, TYPE_HIT, LM_BODY);
            if (fired == 1)
               ch->fight_timer += 2;
         }
      }
      else
         stop_fighting(ch, FALSE);

      if (char_died(ch))
         continue;

      if (retcode == rCHAR_DIED || (victim = who_fighting(ch)) == NULL)
         continue;
      
      //Don't want to target some bum with a giant shield all the time do we!!
      if (ch->fighting && ch->fighting->who && (shield = get_eq_char(ch->fighting->who, WEAR_SHIELD)) != NULL && IS_NPC(ch) && !xIS_SET(ch->act, ACT_MILITARY))
      {
         if (shield->value[2] >= 30) //Lower aggression rate against this target
         {
            for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
            {
               if (aggro->ch && aggro->ch == ch->fighting->who)
                  break;
            }
            if (aggro)
            {
               aggro->value -= UMAX(1, (shield->value[2]- 30) / 4);
            }
         }
      }
      //stop kingdom buffering, mobs will target unequiped foes first....
      if (ch->fighting && ch->fighting->who && ch->fighting->who->name && xIS_SET(ch->act, ACT_MILITARY))
      {     
         int eqlevel;
         int fnd = 0;
         int npeace;
         int ht;
         int bpercent = 100;
         CHAR_DATA *newtarget = NULL;
         
         if (!get_eq_char(ch->fighting->who, WEAR_NECK))
            eqlevel = 1;
         else if (!get_eq_char(ch->fighting->who, WEAR_HEAD))
            eqlevel = 2;
         else if (!get_eq_char(ch->fighting->who, WEAR_ARM_R))
            eqlevel = 3;
         else if (!get_eq_char(ch->fighting->who, WEAR_ARM_L))
            eqlevel = 4;
         else if (!get_eq_char(ch->fighting->who, WEAR_LEG_R))
            eqlevel = 5;
         else if (!get_eq_char(ch->fighting->who, WEAR_LEG_L))
            eqlevel = 6; 
         else if (!get_eq_char(ch->fighting->who, WEAR_BODY))
            eqlevel = 7;
         else
            eqlevel = 8;
            
         if (eqlevel > 1)
         {            
            if (xIS_SET(ch->miflags, KM_ATTACKN))
               npeace = 1;
            else if (xIS_SET(ch->miflags, KM_ATTACKE))
               npeace = 0;
            else
               npeace = -1;
            if (IS_NPC(ch->fighting->who))
               ht = ch->fighting->who->m4;
            else
               ht = ch->fighting->who->pcdata->hometown;
         
            
            for (ich = ch->in_room->first_person; ich; ich = ich->next_in_room)
            {
               if (ich == ch)
                  continue;
                  
               if (IN_WILDERNESS(ch))
               {
                  if (ich->coord->x == ch->coord->x && ich->coord->y == ch->coord->y && ich->map == ch->map)
                  {
                     if (can_see_map(ch, ich) && !is_safe(ch, ich) && ch->m4 != ht && (npeace == -1 || kingdom_table[ch->m4]->peace[ht] <= npeace) && !IS_IMMORTAL(ich))
                     {
                        if (!get_eq_char(ich, WEAR_NECK))
                        {
                           for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                              if (aggro->ch && aggro->ch == ich)
                                 aggro->value = 10000;
                           fnd = 1;
                           break;
                        }
                        if (!get_eq_char(ich, WEAR_HEAD) && eqlevel > 2)
                        {
                           for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                              if (aggro->ch && aggro->ch == ich)
                                 aggro->value = 10000;
                           fnd = 1;
                           break;
                        }
                        if (!get_eq_char(ich, WEAR_ARM_R) && eqlevel > 3)
                        {
                           for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                              if (aggro->ch && aggro->ch == ich)
                                 aggro->value = 10000;
                           fnd = 1;
                           break;
                        }
                        if (!get_eq_char(ich, WEAR_ARM_L) && eqlevel > 4)
                        {
                           for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                              if (aggro->ch && aggro->ch == ich)
                                 aggro->value = 10000;
                           fnd = 1;
                           break;
                        }
                        if (!get_eq_char(ich, WEAR_LEG_R) && eqlevel > 5)
                        {
                           for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                              if (aggro->ch && aggro->ch == ich)
                                 aggro->value = 10000;
                           fnd = 1;
                           break;
                        }
                        if (!get_eq_char(ich, WEAR_LEG_L) && eqlevel > 6)
                        {
                           for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                              if (aggro->ch && aggro->ch == ich)
                                 aggro->value = 10000;
                           fnd = 1;
                           break;
                        }
                        if (!get_eq_char(ich, WEAR_BODY) && eqlevel > 7)
                        {
                           for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                              if (aggro->ch && aggro->ch == ich)
                                 aggro->value = 10000;
                           fnd = 1;
                           break;
                        }
                        if (eqlevel == 8)
                        {        
                           shield = get_eq_char(ich, WEAR_SHIELD);
                           if (shield && shield->value[2] >= 40) //switch targets on a decent block percent
                           {
                              if (bpercent == 100)
                              {
                                 shield = get_eq_char(ch->fighting->who, WEAR_SHIELD);
                                 if (shield)
                                    bpercent = shield->value[2];
                                 else
                                    bpercent = 0;
                              }
                              shield = get_eq_char(ich, WEAR_SHIELD);
            
                              if (!shield && ich != ch && !IS_NPC(ich) && !IS_IMMORTAL(ich))
                              {
                                 newtarget = ich;
                                 bpercent = 0;
                                 continue;
                              }
                              if (shield && shield->value[2] < bpercent && ich != ch && !IS_NPC(ich) && !IS_IMMORTAL(ich))
                              {
                                 newtarget = ich;
                                 bpercent = shield->value[2];
                              }
                           }
                        }
                     }
                  }
               }
               else
               {
                  if (can_see_map(ch, ich) && !is_safe(ch, ich) && ch->m4 != ht && (npeace == -1 || kingdom_table[ch->m4]->peace[ht] <= npeace) && !IS_IMMORTAL(ich))
                  {
                     if (!get_eq_char(ich, WEAR_NECK))
                     {
                        for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                           if (aggro->ch && aggro->ch == ich)
                              aggro->value = 10000;
                        fnd = 1;
                        break;
                     }
                     if (!get_eq_char(ich, WEAR_HEAD) && eqlevel > 2)
                     {
                        for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                           if (aggro->ch && aggro->ch == ich)
                              aggro->value = 10000;
                        fnd = 1;
                        break;
                     }
                     if (!get_eq_char(ich, WEAR_ARM_R) && eqlevel > 3)
                     {
                        for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                           if (aggro->ch && aggro->ch == ich)
                              aggro->value = 10000;
                        fnd = 1;
                        break;
                     }
                     if (!get_eq_char(ich, WEAR_ARM_L) && eqlevel > 4)
                     {
                        for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                           if (aggro->ch && aggro->ch == ich)
                              aggro->value = 10000;
                        fnd = 1;
                        break;
                     }
                     if (!get_eq_char(ich, WEAR_LEG_R) && eqlevel > 5)
                     {
                        for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                           if (aggro->ch && aggro->ch == ich)
                              aggro->value = 10000;
                        fnd = 1;
                        break;
                     }
                     if (!get_eq_char(ich, WEAR_LEG_L) && eqlevel > 6)
                     {
                        for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                           if (aggro->ch && aggro->ch == ich)
                              aggro->value = 10000;
                        fnd = 1;
                        break;
                     }
                     if (!get_eq_char(ich, WEAR_BODY) && eqlevel > 7)
                     {
                        for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                           if (aggro->ch && aggro->ch == ich)
                              aggro->value = 10000;
                        fnd = 1;
                        break;
                     }
                     if (eqlevel == 8)
                     {                       
                        shield = get_eq_char(ich, WEAR_SHIELD);
                        if (shield && shield->value[2] >= 40) //switch targets on a decent block percent
                        {
                           if (bpercent == 100)
                           {
                              shield = get_eq_char(ch->fighting->who, WEAR_SHIELD);
                              if (shield)
                                 bpercent = shield->value[2];
                              else
                                 bpercent = 0;
                           }
                           shield = get_eq_char(ich, WEAR_SHIELD);
                           
                           if (!shield && ich != ch && !IS_NPC(ich) && !IS_IMMORTAL(ich))
                           {
                              newtarget = ich;
                              bpercent = 0;
                              continue;
                           }
                           if (shield && shield->value[2] < bpercent && ich != ch && !IS_NPC(ich) && !IS_IMMORTAL(ich))
                           {
                              newtarget = ich;
                              bpercent = shield->value[2];
                           }
                        }
                     }
                  }
               }
            }
            if (fnd == 0 && newtarget)
            {
               for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
                  if (aggro->ch && aggro->ch == newtarget)
                     aggro->value = 10000;
            }
         }
      }
   }
   /*
    * Fun for the whole family!
    */
   for (rch = ch->in_room->first_person; rch; rch = rch_next)
   {
      rch_next = rch->next_in_room;
      
      if (rch->coord->x != ch->coord->x || rch->coord->y != ch->coord->y || rch->map != ch->map)
      {
         continue;
      }

      if (IS_AWAKE(rch) && !rch->fighting)
      {
         if (!IS_NPC(ch) && IS_NPC(rch) && !IS_SET(ch->pcdata->flags, PCFLAG_CNOASSIST))
         {
            if (IN_SAME_ROOM(rch, ch))
            {
               if (IS_NPC(rch) && IS_AFFECTED(rch, AFF_CHARM) && !IS_ACT_FLAG(rch, ACT_MOUNTSAVE) && rch->master == ch
               &&  !rch->fighting)
               {
                  one_hit(rch, victim, TYPE_HIT, LM_BODY);
                  continue;
               }
            }
         }
         /*
          * PC's auto-assist others in their group.
          */
         if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
         {
            if ((!IS_NPC(rch) || IS_AFFECTED(rch, AFF_CHARM)) && is_same_group(ch, rch))
               one_hit(rch, victim, TYPE_HIT, LM_BODY);
            continue;
         }
         /*
          * NPC's assist NPC's of same type or 12.5% chance regardless.
          */
         if (IS_NPC(rch) && !IS_AFFECTED(rch, AFF_CHARM) && !xIS_SET(rch->act, ACT_NOASSIST))
         {
            if (char_died(ch))
               break;
            if (rch->pIndexData == ch->pIndexData || number_bits(3) == 0)
            {
               CHAR_DATA *vch;
               CHAR_DATA *target;
               int number;
               int hking;
               int iking;

               target = NULL;
               number = 0;
                 
               if (IS_NPC(rch))
                  hking = rch->m4;
               else
                  hking = rch->pcdata->hometown;
            
               for (vch = ch->in_room->first_person; vch; vch = vch->next)
               {
                  if (IS_NPC(vch))
                     iking = vch->m4;
                  else
                     iking = vch->pcdata->hometown;
                        
                  if (can_see_map(rch, vch) && is_same_group(vch, victim) && number_range(0, number) && !is_safe(rch, vch) && (!xIS_SET(vch->act, ACT_MILITARY) || (xIS_SET(vch->act, ACT_MILITARY) && iking != hking)))
                  {
                     if (vch->mount && vch->mount == rch)
                        target = NULL;
                     else
                     {
                        target = vch;
                        number++;
                     }
                  }
               }

               if (target)
                  one_hit(rch, target, TYPE_HIT, LM_BODY);
            }
         }
      }
   }
   for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
   {
      if (!aggro->ch)
         continue;
      if (haggro == -1)
      {
         if (IN_SAME_ROOM(ch, aggro->ch))
            haggro = aggro->value;
      }
      if (aggro->value > haggro)
      {
         if (IN_SAME_ROOM(ch, aggro->ch))
            haggro = aggro->value;
      }
   }
   for (aggro = ch->first_aggro; aggro; aggro = aggro->next)
   {
      if (haggro == aggro->value)
         break;
   }
   if (aggro && ch->fighting && ch->fighting->who && aggro->ch != ch->fighting->who)
   {
      ch->fighting->who = aggro->ch;
      act(AT_WHITE, "$n turns $s head toward $N and focuses $s rage on $N!", ch, NULL, ch->fighting->who, TO_NOTVICT);
      act(AT_WHITE, "$n turns $s head toward you and foruses $s rage on YOU!", ch, NULL, ch->fighting->who, TO_VICT);
   }
   
   if (!IS_NPC(ch) && ch->fight_timer == 0 && ch->fighting && ch->fighting->who)
   {
      if (xIS_SET(ch->act, PLR_TARGET) && !ch->pcdata->autocommand)
      {
         if (ch->pcdata->target_limb == LM_HEAD)
            sprintf(target, "head");
         else if (ch->pcdata->target_limb == LM_BODY)
            sprintf(target, "body");
         else if (ch->pcdata->target_limb == LM_NECK)
            sprintf(target, "neck");
         else if (ch->pcdata->target_limb == LM_RARM)
            sprintf(target, "rarm");
         else if (ch->pcdata->target_limb == LM_LARM)
            sprintf(target, "larm");
         else if (ch->pcdata->target_limb == LM_LLEG)
            sprintf(target, "lleg");
         else if (ch->pcdata->target_limb == LM_RLEG)
            sprintf(target, "rleg");
         else
            sprintf(target, "body");
         sprintf(abuf, "'%s' %s", ch->fighting->who->name, target);
         
         if (ch->pcdata->target == GRIP_SLASH)
            ch->grip = GRIP_SLASH;
         else if (ch->pcdata->target == GRIP_STAB)
            ch->grip = GRIP_STAB;
         else
            ch->grip = GRIP_BASH;
            
         if (ch->position < POS_BERSERK)
            ;
         else if (get_char_room_new(ch, ch->fighting->who->name, 1) == NULL)
            ;
         else if (ch->pcdata->stackbuf[0] != '\0')
         {
            strcpy(commandbuf, "");
            parse_attack_buffer(ch->pcdata->stackbuf, commandbuf);
            if (commandbuf[0] == '\0' && ch->pcdata->stackbuf[0] == '\0')
               do_attack(ch, abuf);
            else
               interpret(ch, commandbuf);
         }
         else
            do_attack(ch, abuf);
      }
   }
   if (xIS_SET(ch->act, PLR_TARGET) && ch->pcdata->autocommand)
   {
      if (ch->pcdata->stackbuf[0] != '\0')
      {
         strcpy(commandbuf, "");
         parse_attack_buffer(ch->pcdata->stackbuf, commandbuf);
         if (commandbuf[0] == '\0' && ch->pcdata->stackbuf[0] == '\0')
            interpret(ch, ch->pcdata->autocommand);
         else
            interpret(ch, commandbuf);
      }
      else
         interpret(ch, ch->pcdata->autocommand);
  }
/*
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2)
      {
         if (d->character->fighting && can_see(d->character, d->character->fighting->who))
         {
            int percent;

            ch = d->character;
            victim = d->character->fighting->who;
            percent = ((victim->hit * 100) / victim->max_hit);
            if (ch->level >= 15)
               sprintf(buf2, "(%2d%%)", percent);
            else
               sprintf(buf2, "     ");
            ch_printf(ch, "^x&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*\n\r");
            if (victim->hit == victim->max_hit)
               ch_printf(ch, "&R*    &G&W(&r**&R***&Y***&G***&G&W) %s    &R*\n\r", buf2);
            else if (percent >= 90)
               ch_printf(ch, "&R*    &G&W(&r**&R***&Y***&G** &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 80)
               ch_printf(ch, "&R*    &G&W(&r**&R***&Y***&G*  &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 70)
               ch_printf(ch, "&R*    &G&W(&r**&R***&Y***   &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 60)
               ch_printf(ch, "&R*    &G&W(&r**&R***&Y**    &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 50)
               ch_printf(ch, "&R*    &G&W(&r**&R***&Y*     &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 40)
               ch_printf(ch, "&R*    &G&W(&r**&R***      &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 30)
               ch_printf(ch, "&R*    &G&W(&r**&R**       &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 20)
               ch_printf(ch, "&R*    &G&W(&r**&R*        &G&W) %s    &R*\n\r", buf2);
            else if (percent >= 10)
               ch_printf(ch, "&R*    &G&W(&r**         &G&W) %s    &R*\n\r", buf2);
            else
               ch_printf(ch, "&R*    &G&W(&r*          &G&W) %s    &R*\n\r", buf2);
            ch_printf(ch, "&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*&r~&R*\n\r");
         }
      }
   }*/
 }
   return;
}

/*
 * Weapon types, haus
 */
int weapon_prof_bonus_check(CHAR_DATA * ch, OBJ_DATA * wield, int *gsn_ptr)
{
   return 0;
}

/*
 * Calculate the tohit bonus on the object and return RIS values.
 * -- Altrag
 */
int obj_hitroll(OBJ_DATA * obj)
{
   return 0;
}

/*
 * Offensive shield level modifier
 */
sh_int off_shld_lvl(CHAR_DATA * ch)
{
   if (get_curr_int(ch) < 13) //1-12
      return SPOWER_MIN;
   if (get_curr_int(ch) < 17) //13-16
      return SPOWER_MED;
   if (get_curr_int(ch) < 21) //17-20
      return SPOWER_HI;
   if (get_curr_int(ch) < 24) //21-23
      return SPOWER_GREATER;
   else
      return SPOWER_GREATEST; //24+
   
      
   return URANGE(5, (get_curr_int(ch)-12)*5,  50);      
}

void do_grip(CHAR_DATA * ch, char *argument)
{
   if (!str_prefix(argument, "slash"))
   {
      ch->grip = GRIP_SLASH;
      send_to_char("Grip changed to slash.\n\r", ch);
      if (ch->fighting)
      {
         SET_BIT(ch->fight_state, FSTATE_GRIP);
         ch->fight_timer = 2;
      }
      return;
   }
   else if (!str_prefix(argument, "bash"))
   {
      ch->grip = GRIP_BASH;
      send_to_char("Grip changed to bash.\n\r", ch);
      if (ch->fighting)
      {
         SET_BIT(ch->fight_state, FSTATE_GRIP);
         ch->fight_timer = 2;
      }
      return;
   }
   else if (!str_prefix(argument, "stab"))
   {
      ch->grip = GRIP_STAB;
      send_to_char("Grip changed to stab.\n\r", ch);
      if (ch->fighting)
      {
         SET_BIT(ch->fight_state, FSTATE_GRIP);
         ch->fight_timer = 2;
      }
      return;
   }
   else
   {
      send_to_char("Syntax: grip <argument>    argument = slash, bash, stab\n\r", ch);
      return;
   }
}

//Adjusts strength/agility from attack, part of the stat increase/decrease with no levels
//More of a strength drop now for using lighter weapons
void adjust_agi_str(CHAR_DATA *ch, OBJ_DATA *weapon, CHAR_DATA *target)
{
   //1000 for agility 10000 for strengh since it is base 25 and agi is base 100
   int str, agi, mod, mstr, magi, bstr, astr, alow, apercent;
   
   str = ch->perm_str;
   agi = ch->perm_agi;
   mstr = magi = 0;
   
   if (get_eq_char(ch, WEAR_DUAL_WIELD) != NULL)
      learn_from_success(ch, gsn_dual_wield, NULL);
   
   if(weapon == NULL)
      mod = 1;
   else
      mod = weapon->value[3] - race_table[ch->race]->weaponmin + 1;
     
   if (mod == 7)    //largest weapon
   {
      magi += number_range(-1, -2);
      mstr += number_range(2, 3);   
   } 
   if (mod == 6) //2nd largest weapon
   {
      magi += number_range(0, -1);
      mstr += number_range(1, 2);
   }   
   if (mod == 5) //average weapon/heavy, light change in both areas
   {
      magi += number_range(-1, 2);
      mstr += number_range(1, 0);
   }
   if (mod == 4) //average weapon/light, light change in both areas.
   {
      magi += number_range(1, 2);
      mstr += number_range(-1, 1);
   }
   if (mod == 3) //Lighter weapon, increase agi
   {
      magi += number_range(2, 3);
      mstr += number_range(0, -1);
   }
   if (mod == 2) //Lightest weapon, increases agi
   {
      magi += number_range(3, 5);
      mstr += number_range(-2, -3);
   }
   if (mod == 1) //no weapon
   {
      magi += number_range(5, 7);
      mstr += number_range(-3, -5);   
   }
   
   //Increase agility if under 30 no matter what
   if (agi <= 30)
   {
      if (mod == 7)
         magi = 1;
      if (mod == 6)
         magi = number_range(1, 2);
      if (mod == 5)
         magi = number_range(1, 3);
      if (mod == 4)
         magi = number_range(1, 4);
      if (mod == 3)
         magi = number_range(2, 5);
      if (mod == 2)
         magi = number_range(4, 10);
      if (mod == 1)
         magi = number_range(12, 20);
  }
  if (sysdata.stat_gain <= 3)
   {
      if (magi > 0)
         magi = number_range(150*magi/100, 180*magi/100);
      if (mstr > 0)
         mstr = number_range(150*mstr/100, 180*mstr/100);
   }
   else if (sysdata.stat_gain >= 5)
   {
      if (magi > 0)
         magi = number_range(250*magi/100, 300*magi/100);
      if (mstr > 0)
         magi = number_range(250*mstr/100, 300*mstr/100);
   }
   
   
   //strength mods
   if (mstr > 0)
   {
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
      if (str == bstr + 6)
         mstr *= .275;
      if (str == bstr + 7)
         mstr *= .25;
      if (str == bstr + 8)
         mstr *= .225;
      if (str > bstr + 8) //Base + 8 should be the max unless you screwed it up
         mstr = 0;
      else
      {
         if (mstr == 0)
            mstr = 1;
      }
   }
   else
   {
      bstr = 14 + race_table[ch->race]->str_plus;
      if (str == bstr - 4)
         mstr = 0;
      if (str == bstr - 3)
         mstr *= .3;
      if (str == bstr - 2)
         mstr *= .4;
      if (str == bstr - 1)
         mstr *= .6;
      if (str == bstr)
         mstr *= 1;
      if (str == bstr + 1)
         mstr *= 1.2;
      if (str == bstr + 2)
         mstr *= 1.4;
      if (str == bstr + 3)
         mstr *= 1.6;
      if (str == bstr + 4)
         mstr *= 1.8;
      if (str >= bstr + 5)
         mstr *= 2;
   }
   //agility mods
   if (magi > 0)
   {
      astr = race_table[ch->race]->agi_start;
      alow = astr - race_table[ch->race]->agi_range;
      apercent = (agi - alow) * 100 / (astr - alow);
      magi = magi * (200 - apercent) / 100;
      if (magi <= 0)
         magi = 1;
   }
   else
   {
      astr = race_table[ch->race]->agi_start;
      alow = astr - race_table[ch->race]->agi_range;
      apercent = (agi - alow) * 100 / (astr - alow);
      magi = magi * apercent / 100;
   }
   if (ch->perm_str >= (14 + race_table[ch->race]->str_plus + race_table[ch->race]->str_range + get_talent_increase(ch, 1)) && ch->pcdata->per_str >= 3000 && mstr > 0)
      mstr = 0;
   if (ch->perm_str <= (14 + race_table[ch->race]->str_plus - 5 + race_table[ch->race]->str_range) && ch->pcdata->per_str <= 3000 && mstr < 0)
      mstr = 0;
   if (ch->perm_agi >= (race_table[ch->race]->agi_start + race_table[ch->race]->agi_range + get_talent_increase(ch, 7)) && ch->pcdata->per_agi >= 300)
      magi = 0;     
   if (ch->perm_agi <= (race_table[ch->race]->agi_start - race_table[ch->race]->agi_range) && ch->pcdata->per_agi <= 300 && magi < 0)
      magi = 0;
   if (target && IS_NPC(target) && xIS_SET(target->act, ACT_MOUNTSAVE))
   {
      if (mstr > 0)
         mstr = 0;
      if (magi > 0)
         magi = 0;
   }
   if (magi > 1 && magi < -1)
      magi = magi * race_table[ch->race]->agi_plus/100;
   ch->pcdata->per_agi += magi;
   ch->pcdata->per_str += mstr;
   
   if (ch->pcdata->per_agi > 1000)
   {
      ch->perm_agi++;
      if (ch->perm_agi < 16)
         ch->perm_agi = 16;
      send_to_char("&R*************************************\n\r", ch);
      send_to_char("&R*****You Gain 1 Point of Agility*****\n\r", ch);
      send_to_char("&R*************************************\n\r", ch);
      ch->pcdata->per_agi = 0;
   }
   if (ch->pcdata->per_str > 10000)
   {
      ch->perm_str++;
      send_to_char("&R**************************************\n\r", ch);
      send_to_char("&R*****You Gain 1 Point of Strength*****\n\r", ch);
      send_to_char("&R**************************************\n\r", ch);
      ch->pcdata->per_str = 0;
   }
   if (ch->pcdata->per_agi < 0)
   {
      ch->perm_agi--;
      send_to_char("&r*************************************\n\r", ch);
      send_to_char("&r*****You Lose 1 Point of Agility*****\n\r", ch);
      send_to_char("&r*************************************\n\r", ch);
      ch->pcdata->per_agi = 999;
   }
   if (ch->pcdata->per_str < 0)
   {
      ch->perm_str--;
      send_to_char("&r**************************************\n\r", ch);
      send_to_char("&r*****You Lose 1 Point of Strength*****\n\r", ch);
      send_to_char("&r**************************************\n\r", ch);
      ch->pcdata->per_str = 9999;
   }
}
   
int get_limb_location(int limb)
{
   if (limb == LM_BODY)
      return WEAR_BODY;   
   if (limb == LM_NECK)
      return WEAR_NECK;
   if (limb == LM_HEAD)
      return WEAR_HEAD;
   if (limb == LM_RARM)
      return WEAR_ARM_R;
   if (limb == LM_LARM)
      return WEAR_ARM_L;
   if (limb == LM_RLEG)
      return WEAR_LEG_R;
   if (limb == LM_LLEG)
      return WEAR_LEG_L;
      
   return WEAR_BODY;
}   

//the routine for hit or miss check
//-ac (better tohit) +ac (better armor)
int get_hit_or_miss(CHAR_DATA *ch, CHAR_DATA *victim, int ac, int grip, int limb, int noarmor, OBJ_DATA *wield, int sn)
{
   int crit = 0;
   int spread = 0;
   int value = 0;
   int percent;
   int points;
   AFFECT_DATA *paf;
   int roll;
   int pslice = 0;
   int perfect = 0;
   int bracing;
   int nodeath = 0;
   int addspread = 0;
   
   //critical skill check
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_critical] > 0 && ch->pcdata->ranking[gsn_critical] > 0)
   {
      percent = number_range(1, 10000);
      points =  POINT_LEVEL(LEARNED(ch, gsn_critical), MASTERED(ch, gsn_critical));
      
      points = number_range(points * 7, points * 10);


      if (percent <= points)
      {
         crit = 1;
         learn_from_success(ch, gsn_critical, victim); 
      }
      else
         learn_from_failure(ch, gsn_critical, victim);
   }
   if (sn == gsn_powerslice)
   {
      int level;
      level = POINT_LEVEL(LEARNED(ch, gsn_powerslice), MASTERED(ch, gsn_powerslice))*2;
      level += (get_curr_str(ch) - get_curr_str(victim)) * 2;
      level += number_range(level*3, level*4);
      level = UMAX(50, level);
      if (number_range(1, 10000) <= level)
         pslice = 1;
         
      if (pslice == 1)
         learn_from_success(ch, gsn_powerslice, victim);   
      else
         learn_from_failure(ch, gsn_powerslice, victim);   
   }
   if (sn == gsn_perfect_shot)
   {
      int level;
      level = POINT_LEVEL(LEARNED(ch, gsn_perfect_shot), MASTERED(ch, gsn_perfect_shot))*3/2;
      level += (get_curr_str(ch) - get_curr_str(victim)) * 2;
      level += number_range(level*2, level*3);
      level = UMAX(50, level);
      if (number_range(1, 10000) <= level)
      {
         perfect = 1;
      }
   }
   if (wield && IS_OBJ_STAT(wield, ITEM_TWOHANDED) && ch->race < MAX_PC_RACE)
   {
      int tpoints = POINT_LEVEL(LEARNED(ch, gsn_weapon_twohanded), MASTERED(ch, gsn_weapon_twohanded));
      int mod = wield->value[3] - race_table[ch->race]->weaponmin + 1;
      if (mod >= 5 && LEARNED(ch, gsn_weapon_twohanded))
      {
         if (tpoints >= 90)
            ac-=3;
         else if (tpoints >= 60)
            ac-=2;
         else if (tpoints >= 30)
            ac-=1;
      }
   }
   ac*=2; //this should make it more defined if you have a better weapon/armor
   ac -= URANGE(-10, ch->apply_tohit, 10);
   ac += URANGE(-10, victim->apply_armor, 10);
   for (paf = victim->first_affect; paf; paf = paf->next)
   {
      if (paf->type == gsn_berserk);
      {
         ac -= UMIN(6, paf->modifier/12+1);
         break;
      }
   }
   if (!IS_NPC(victim) && victim->pcdata->learned[gsn_krundo_style] > 0)
   {
      int level;
      level = POINT_LEVEL(LEARNED(victim, gsn_krundo_style), MASTERED(victim, gsn_krundo_style));
      level = number_range(level*4, level*5);
      level = URANGE(1, level, 400);
      ac += level/100;
      level = level % 100;
      if (number_range(1, 100) <= level)
      {
         ac++;
      }
      learn_from_success(victim, gsn_krundo_style, ch);  
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_rwundo_style] > 0)
   {
      int level;
      level = POINT_LEVEL(LEARNED(ch, gsn_rwundo_style), MASTERED(ch, gsn_rwundo_style));
      level = number_range(level*4, level*5);
      level = URANGE(1, level, 400);
      ac -= level/100;
      level = level % 100;
      if (number_range(1, 100) <= level)
      {
         ac--;
      }
      learn_from_success(ch, gsn_rwundo_style, victim);  
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_concentration] > 0)
   {
      int level;
      level = POINT_LEVEL(LEARNED(ch, gsn_concentration), MASTERED(ch, gsn_concentration));
      level = number_range(level*3, level*4);
      level = URANGE(1, level, 300);
      ac -= level/100;
      level = level % 100;
      if (number_range(1, 100) <= level)
      {
         ac--;
      }
   }
   g_tohit = ac;
   if (grip == GRIP_BASH)
      spread = 6;
   else if (grip == GRIP_SLASH)
      spread = 4;
   else if (grip == GRIP_STAB)
      spread = 2;
      
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_rwundi_style] > 0)
   {
      int level;
      level = POINT_LEVEL(LEARNED(ch, gsn_rwundi_style), MASTERED(ch, gsn_rwundi_style));
      level = number_range(level*2, level*3);
      level = URANGE(1, level, 200);
      spread += level/100;
      level = level % 100;
      if (number_range(1, 100) <= level)
      {
         addspread++;
      }
      learn_from_success(ch, gsn_rwundi_style, victim);  
   }     
      
   roll = number_range(1, TOHIT_SYS);
   if (victim->apply_bracing)
   {
      int sroll;
      bracing = UMAX(1, TOHIT_SYS * victim->apply_bracing / 120);
      sroll = number_range(1, TOHIT_SYS+bracing);
      if (sroll == TOHIT_SYS+bracing || pslice || perfect)
         crit = 1;
      check_aff_learn(victim, "bracing", 0, ch, 1);
   }
   else
   {
      if (roll == TOHIT_SYS || pslice || perfect)
         crit = 1;
   }
   if (limb == LM_HEAD && roll >= TOHIT_SYS-(UMAX(1,TOHIT_SYS/20)))
      crit = 1;
   else if (roll == 1 && !pslice && !perfect)
      crit = -1;
   if (roll > ((TOHIT_SYS/2)-(TOHIT_SYS/10) + ac + spread + (TOHIT_SYS/6)) && limb == LM_NECK)
   {
      if (crit == 0)
         nodeath = 1;
      crit = 1;
   }
   g_roll = roll;
   g_lowtohit = UMAX(1, (TOHIT_SYS/2)-(TOHIT_SYS/10) + ac - spread - addspread);
   g_hitohit = UMAX(1, (TOHIT_SYS/2)-(TOHIT_SYS/10) + ac + spread);
   if (noarmor && crit == 0) //always land against no armor
      return 200;
      
   if (crit && sn == gsn_pincer) //Enough is enough on this one
      return 200;
     
   if (roll < ((TOHIT_SYS/2)-(TOHIT_SYS/10) + ac - spread - addspread) && crit != 1 && crit != -1)
      return DM_MISS;
   if (roll > ((TOHIT_SYS/2)-(TOHIT_SYS/10) + ac + spread) && crit != 1 && crit != -1)
      return 200; //100 percent 200 - 100
   if (crit != 1 && crit != -1)
   {
      value = (TOHIT_SYS/2)-(TOHIT_SYS/10) + ac + spread - roll;
      spread += addspread;
      if (spread == 2)
      {
         value -= addspread*2;
         if (value <= 0)
            return 183;
         else if (value == 1)
            return 166;
         else if (value == 2)
            return 150;
         else if (value == 3)
            return 133;
         else if (value == 4)
            return 116;
      }
      if (spread == 3)
      {
         value -= addspread*2;
         if (value <= 0) 
            return 187;
         else if (value == 1)
            return 175;
         else if (value == 2)
            return 162;
         else if (value == 3)
            return 150;
         else if (value == 4)
            return 137;
         else if (value == 5)
            return 125;
         else if (value == 6)
            return 112;
      }
      if (spread == 4)
      {
         value -= addspread*2;
         if (value <= 0)
            return 190;
         else if (value == 1)
            return 180;
         else if (value == 2)
            return 170;
         else if (value == 3)
            return 160;
         else if (value == 4)
            return 150;
         else if (value == 5)
            return 140;
         else if (value == 6)
            return 130;
         else if (value == 7)
            return 120;
         else if (value == 8)
            return 110;
      }
      if (spread == 5) //8.33
      {
         value -= addspread*2;
         if (value <= 0)
            return 192;
         else if (value == 1)
            return 184;
         else if (value == 2)
            return 175;
         else if (value == 3)
            return 167;
         else if (value == 4)
            return 159;
         else if (value == 5)
            return 150;
         else if (value == 6)
            return 141;
         else if (value == 7)
            return 133;
         else if (value == 8)
            return 125;
         else if (value == 9)
            return 116;
         else if (value == 10)
            return 108;
      }
      if (spread == 6) //7.14
      {
         value -= addspread*2;
         if (value <= 0)
            return 192;
         else if (value == 1)
            return 185;
         else if (value == 2)
            return 178;
         else if (value == 3)
            return 171;
         else if (value == 4)
            return 164;
         else if (value == 5)
            return 157;
         else if (value == 6)
            return 150;
         else if (value == 7)
            return 142;
         else if (value == 8)
            return 135;
         else if (value == 9)
            return 128;
         else if (value == 10)
            return 121;
         else if (value == 11)
            return 114;
         else if (value == 12)
            return 107;
      }
      if (spread == 7) //6.25
      {
         value -= addspread*2;
         if (value <= 0)
            return 193;
         else if (value == 1)
            return 187;
         else if (value == 2)
            return 181;
         else if (value == 3)
            return 175;
         else if (value == 4)
            return 168;
         else if (value == 5)
            return 162;
         else if (value == 6)
            return 156;
         else if (value == 7)
            return 150;
         else if (value == 8)
            return 143;
         else if (value == 9)
            return 137;
         else if (value == 10)
            return 131;
         else if (value == 11)
            return 125;
         else if (value == 12)
            return 118;
         else if (value == 13)
            return 112;
         else if (value == 14)
            return 106;
      }
      if (spread == 8) //5.55
      {
         value -= addspread*2;
         if (value <= 0)
            return 194; 
         else if (value == 1)
            return 188;
         else if (value == 2)
            return 183;
         else if (value == 3)
            return 177;
         else if (value == 4)
            return 172;
         else if (value == 5)
            return 166;
         else if (value == 6)
            return 161;
         else if (value == 7)
            return 155;
         else if (value == 8)
            return 150;
         else if (value == 9)
            return 144;
         else if (value == 10)
            return 138;
         else if (value == 11)
            return 133;
         else if (value == 12)
            return 127;
         else if (value == 13)
            return 122;
         else if (value == 14)
            return 116;
         else if (value == 15)
            return 111;
         else if (value == 16)
            return 105;
      }
   }
   if (crit == -1) //crit miss
   {
      roll = number_range(1, TOHIT_SYS);
      if (roll >= 8 && roll <= 10 && wield && !IS_OBJ_STAT(wield, ITEM_NOBREAK))
      {
         send_to_char("You damage your weapon with your lousy attack.\n\r", ch);
         if (wield->item_type == ITEM_MISSILE_WEAPON)
            wield->value[0] -= number_range(25, 60);
         else
            wield->value[0] -= number_range(50, 100);
         if (wield->value[0] <= 0)
         {
            make_scraps(wield, ch);
         }
      }
      else if (roll >= 14 && roll <= 19)
      {
         send_to_char("Your attack was so bad that you lose your balance and need to take some time to regain it.\n\r", ch);
         ch->fight_timer += number_range(UMAX(3, ch->fight_timer*1), UMAX(6, ch->fight_timer*2));
      }
   }
   if (crit == 1) //crit hit
   {
      int toinsta;
      int blimb = 0;
      roll = number_range(1,TOHIT_SYS);
      if (ch->grip == GRIP_STAB)
      {
         if (roll < TOHIT_SYS-2)
            roll+= 3;
         else
            roll = TOHIT_SYS;
      }
      if (perfect == 1)
         roll = TOHIT_SYS;
      if (roll == TOHIT_SYS && (limb == LM_HEAD || limb == LM_NECK || limb == LM_BODY) && !nodeath)
         return DM_DEATH;
      if (limb == LM_NECK && !nodeath)
      {
         toinsta = URANGE(TOHIT_SYS-14, TOHIT_SYS-10+ac, TOHIT_SYS-4);
         if (roll > toinsta)
            return DM_DEATH;
      }
      else if (limb == LM_HEAD)
      {
         toinsta = URANGE(TOHIT_SYS-10, TOHIT_SYS-6+ac, TOHIT_SYS-2);
         if (roll > toinsta)
            return DM_DEATH;
      }
      else
      {
         if (grip == GRIP_SLASH)
         {
            toinsta = number_range(1, 100)-ac;
            if (toinsta > 95)   
               blimb = 1;
         }
         if (grip == GRIP_BASH)
         {
            toinsta = number_range(1, 100)-ac;
            if (toinsta > 99)   
               blimb = 1;
         }
         if (pslice == 1)
            blimb = 1;
      }
      if (blimb == 0)
         return number_range(250, 350);
      else
         return number_range(-250, -350);
   }
   return DM_MISS;
}   
   	
int fight_unarmed(CHAR_DATA *ch)
{
   BUYKMOB_DATA *kmob;

   if (!IS_NPC(ch))
      return 0;
   if (!IN_WILDERNESS(ch))
      return 0;
   if (!xIS_SET(ch->act, ACT_MILITARY))
      return 0;
     
   for (kmob = first_buykmob; kmob; kmob = kmob->next)
   {
      if (kmob->vnum == ch->pIndexData->vnum)
      {
         if (xIS_SET(kmob->flags, KMOB_SOLDIERUNARMED))
            return 1;
      }
   }   	
   return 0;
}

int wielding_skill_weapon(CHAR_DATA *ch, int missile)
{
   OBJ_DATA *weapon;
   
    if ((weapon = get_eq_char(ch, WEAR_WIELD)) == NULL)
    {
       if (get_eq_char(ch, WEAR_MISSILE_WIELD) && missile)
          return 6;
       else
          return 0;
    }
    if (get_eq_char(ch, WEAR_MISSILE_WIELD) && missile)
       return 6;
    if (weapon->pIndexData->vnum >= OBJ_FORGE_HAND_AXE && weapon->pIndexData->vnum <= OBJ_FORGE_BATTLE_AXE)
       return 1;
    if (weapon->pIndexData->vnum >= OBJ_FORGE_SHORT_SWORD && weapon->pIndexData->vnum <= OBJ_FORGE_FLAMBERGE)
       return 2;
    if (weapon->pIndexData->vnum >= OBJ_FORGE_KNIFE && weapon->pIndexData->vnum <= OBJ_FORGE_STILETTO )
       return 7;
    if (weapon->pIndexData->vnum >= OBJ_FORGE_PILUM && weapon->pIndexData->vnum <= OBJ_FORGE_TRIDENT)
       return 3;
    if (weapon->pIndexData->vnum >= OBJ_FORGE_CLUB && weapon->pIndexData->vnum <= OBJ_FORGE_MAUL)
       return 4;
    if (weapon->pIndexData->vnum >= OBJ_FORGE_SCEPTRE && weapon->pIndexData->vnum <= OBJ_FORGE_BLADED_STAFF)
       return 5;
       
    if (IS_OBJ_STAT(weapon, ITEM_TYPE_AXE))
       return 1;
    if (IS_OBJ_STAT(weapon, ITEM_TYPE_SWORD))
       return 2;
    if (IS_OBJ_STAT(weapon, ITEM_TYPE_DAGGER))
       return 7;
    if (IS_OBJ_STAT(weapon, ITEM_TYPE_POLEARM))
       return 3;
    if (IS_OBJ_STAT(weapon, ITEM_TYPE_BLUNT))
       return 4;
    if (IS_OBJ_STAT(weapon, ITEM_TYPE_STAVES))
       return 5;
       
    return 0;
}  

//Type 0 - (ch is being hit) 1 - (ch is doing the hitting)
int get_fightingstyle_dam(CHAR_DATA *ch, int dam, CHAR_DATA *victim, int type, int learn)
{
   int level;
   int gsnstyle = gsn_style_standard;
   int newdam = 0;
   int dam1;
   int dam2;
   
   if (IS_NPC(ch))
      return dam;
      
   if (ch->style == STYLE_EVASIVE)
      gsnstyle = gsn_style_evasive;
   if (ch->style == STYLE_WIZARDRY)
      gsnstyle = gsn_style_wizardry;
   if (ch->style == STYLE_DIVINE)
      gsnstyle = gsn_style_divine;
   if (ch->position == POS_AGGRESSIVE)
      gsnstyle = gsn_style_aggressive;
   if (ch->position == POS_FIGHTING)
      gsnstyle = gsn_style_standard;
   if (ch->position == POS_BERSERK)
      gsnstyle = gsn_style_berserk;
   if (ch->position == POS_DEFENSIVE)
      gsnstyle = gsn_style_defensive;
      
   if (GET_MASTERY(ch, gsnstyle, 0, ch->level) <= 0)
      learn = 0;
         
   level = POINT_LEVEL(LEARNED(ch, gsnstyle), MASTERED(ch, gsnstyle)); 
   
   if (ch->position == POS_BERSERK)
   {
      if (type == 0)
         newdam = 5 - level/12;
      else
         newdam = 4 + level/12;
   }
   else if (ch->position == POS_AGGRESSIVE)
   {
      if (type == 0)
         newdam = 2 - level/12;
      else
         newdam = 2 + level/12;
   }
   else if (ch->position == POS_FIGHTING)
   {
      if (type == 0)
         newdam = 0 - level/12;
      else
         newdam = 0 + level/12;
   }
   else if (ch->position == POS_DEFENSIVE)
   {
      if (type == 0)
         newdam = -2 - level/12;
      else
         newdam = -2 + level/12;
   }
   else if (ch->position == POS_EVASIVE)
   {
      if (type == 0)
         newdam = -5 - level/12;
      else
         newdam = -4 + level/12;
   }
   dam1 = dam + newdam;
   dam2 = dam * (100+(newdam*3)) / 100;
   
   //If you are hitting choose the least amount of damage, if you are being hit choose the most
   if ((type == 1 && dam1 < dam2) || (type == 0 && dam1 > dam2))
      dam = dam1;
   else
      dam = dam2;
      
   if (learn)
      learn_from_success(ch, gsnstyle, victim);
   return dam;
}

	
/*
 * Hit one guy once.
 */
ch_ret one_hit(CHAR_DATA * ch, CHAR_DATA * victim, int dt, int limb)
{
   OBJ_DATA *wield;
   OBJ_DATA *dual;
   OBJ_DATA *shield;
   OBJ_DATA *eq;
   int victim_ac = 0;
   int armor_ac = 0;
   int weapon_ac = 0;
   int tohit;
   int plusris;
   int dam;
   int level;
   int miss = 0;
   int spec = DM_HIT;
   int holder = 1;
   int percent = 1;
   int counter = 0;
   int attacktype, cnt;
   int prof_bonus;
   ch_ret retcode = rNONE;
   sh_int mastery;
   sh_int hnum;
   int crit = 0;
   int block = 0;
   int suc = 0;
   int slimb = 0;
   int cond;
   AFFECT_DATA *paf;
   int noarmor = 0;
   BUYKMOB_DATA *kmob;

   dual = NULL;
   blockdam = 0;
   g_tohit = g_roll = g_lowtohit = g_hitohit = 0;
   /*
    * Can't beat a dead char!
    * Guard against weird room-leavings.
    */
   if (victim->position == POS_DEAD || ch->in_room != victim->in_room)
      return rVICT_DIED;

   if (get_eq_char(ch, WEAR_DUAL_WIELD) != NULL && dt != gsn_backstab)
   {
      if (number_range(1, 2) == 1)
         wield = get_eq_char(ch, WEAR_WIELD);
      else
         wield = get_eq_char(ch, WEAR_DUAL_WIELD);
   }
   else
      wield = get_eq_char(ch, WEAR_WIELD);
   
   //moved this here so it can use the right weapon for finding values....was moved from violence_update   
   if (IS_NPC(ch))
   {
      if (xIS_SET(ch->act, ACT_NOATTACK))
         return rNONE;
      if (wield && wield == get_eq_char(ch, WEAR_DUAL_WIELD))
      {
         dual = get_eq_char(ch, WEAR_WIELD);
         wield->wear_loc = WEAR_WIELD;
         dual->wear_loc = WEAR_DUAL_WIELD;
         limb = target_chooser(ch, victim);  //sets limb and grip
         dual->wear_loc = WEAR_WIELD;
         wield->wear_loc = WEAR_DUAL_WIELD;
      }      
      else
         limb = target_chooser(ch, victim); //sets limb and grip
   }
          
   //remove this later, but for now they need to be reset;  
   if (wield)
   {
      if (xIS_SET(wield->pIndexData->extra_flags, ITEM_TWOHANDED))
         xSET_BIT(wield->extra_flags, ITEM_TWOHANDED);
   }  
   
   if (victim->position == POS_RIDING)
   {
      victim->position = POS_STANDING;
      if (victim->riding)
      {
         victim->riding->rider = NULL;
         victim->riding = NULL;
      }
   }
   if (victim->rider)
   {
      victim->rider->riding = NULL;
      victim->rider->position = POS_STANDING;
      victim->rider = NULL;
   }

   prof_bonus = 0;
   
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_combatart] > 0 && wield)
   {
      learn_from_success(ch, gsn_combatart, victim);
   }
   if (dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back)
   {
      ch->fight_timer = get_btimer(ch, -1, victim);
   }
   else if (dt < TYPE_PROJECTILE)
   {
      ch->fight_timer = get_btimer(ch, dt, victim);
   }
   else
   {
      ch->fight_timer = get_btimer(ch, -1, victim);
   }

   if (ch->fighting /* make sure fight is already started */
      && dt == TYPE_UNDEFINED && IS_NPC(ch) && !xIS_EMPTY(ch->attacks))
   {
      cnt = 0;
      for (;;)
      {
         attacktype = number_range(0, 6);
         if (xIS_SET(ch->attacks, attacktype))
            break;
         if (cnt++ > 16)
         {
            attacktype = -1;
            break;
         }
      }
      if (attacktype == ATCK_BACKSTAB)
         attacktype = -1;
      if (wield && number_percent() > 15)
         attacktype = -1;
      if (!wield && number_percent() > 30)
         attacktype = -1;

      switch (attacktype)
      {
         default:
            break;
         case ATCK_BITE:
            break;
         case ATCK_CLAWS:
            break;
         case ATCK_TAIL:
            break;
         case ATCK_STING:
            break;
         case ATCK_PUNCH:
            break;
         case ATCK_KICK:
            break;
         case ATCK_TRIP:
            attacktype = 0;
            break;
      }
      if (attacktype >= 0)
         return retcode;
   }

   if (dt == TYPE_UNDEFINED)
   {
      dt = TYPE_HIT;
   }

   /*
    * Replaces that thaco crap, back to a easier to identify base 20 ac
    */
   if ((eq = get_eq_char(victim, get_limb_location(limb))) == NULL)
   {
      if (IS_NPC(victim))
      {
         armor_ac = victim->armor;
         if (victim->armor == 0)
            noarmor = 1;
         if (limb == LM_HEAD)
            armor_ac += 2;
         else if (limb == LM_NECK)
            armor_ac += 3;          
      }
      else
         armor_ac = 0;
         
      if (ch->grip == GRIP_BASH)
      {
         if (wield)
            weapon_ac += wield->value[7];
         else
         {
            if (IS_NPC(ch))
               weapon_ac += ch->tohitbash;
         }
      }
      else if (ch->grip == GRIP_STAB)
      {
         if (wield)
            weapon_ac += wield->value[9];
         else
         {
            if (IS_NPC(ch))
               weapon_ac += ch->tohitstab;
         }
      }
      else if (ch->grip == GRIP_SLASH)
      {
         if (wield)
            weapon_ac += wield->value[8];
         else
         {
            if (IS_NPC(ch))
               weapon_ac += ch->tohitslash;
         }
      }
      else
      {
         if (wield)
            weapon_ac += wield->value[7];
         bug("one_hit: %s has a 0 value for grip.", ch->name);
      }
   }
   else
   {
      if (ch->grip == GRIP_BASH)
      {
         armor_ac = eq->value[0];
         if (wield)
            weapon_ac += wield->value[7];
         else
         {
            if (IS_NPC(ch))
               weapon_ac += ch->tohitbash;
         }
      }
      else if (ch->grip == GRIP_STAB)
      {
         armor_ac = eq->value[2];
         if (wield)
           weapon_ac += wield->value[9];
         else
         {
            if (IS_NPC(ch))
               weapon_ac += ch->tohitstab;
         }
      }
      else if (ch->grip == GRIP_SLASH)
      {
         armor_ac = eq->value[1];
         if (wield)
            weapon_ac += wield->value[8];
         else
         {
            if (IS_NPC(ch))
               weapon_ac += ch->tohitslash;
         }
      }
      else
      {
         armor_ac = eq->value[0];
         if (wield)
            weapon_ac += wield->value[7];
         bug("one_hit: %s has a 0 value for grip.", ch->name);
      }
   }
   if (armor_ac == 0)
      noarmor = 1;
   if (dt == gsn_backstab) //small bonus for backstab because it is not seen
      weapon_ac += 1;
      
   if (MASTERED(ch, gsn_backstab) >= 3 && dt == gsn_backstab) //another bonus for expert and master
      weapon_ac += 1;
      
   if (!wield && fight_unarmed(ch))
      weapon_ac += 5;
   if (eq)
      cond = eq->value[3];
   else
      cond = 1000*victim->hit/victim->max_hit;
   
   if (armor_ac > 0)
   {
      armor_ac -= (3 -(cond / 251));
      if (armor_ac < 1)
         armor_ac = 1;
   }
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
   {
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (kmob->vnum == ch->pIndexData->vnum)
         {
            if (xIS_SET(kmob->flags, KMOB_TOHIT1))
               weapon_ac+=1;
            if (xIS_SET(kmob->flags, KMOB_TOHIT2))
               weapon_ac+=2;
            if (xIS_SET(kmob->flags, KMOB_TOHIT3))
               weapon_ac+=4;
            if (xIS_SET(kmob->flags, KMOB_TOHIT4))
               weapon_ac+=6;
         }
      }
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MILITARY))
   {
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (kmob->vnum == victim->pIndexData->vnum)
         {
            if (xIS_SET(kmob->flags, KMOB_AC1))
               armor_ac+=1;
            if (xIS_SET(kmob->flags, KMOB_AC2))
               armor_ac+=2;
            if (xIS_SET(kmob->flags, KMOB_AC3))
               armor_ac+=4;
            if (xIS_SET(kmob->flags, KMOB_AC4))
               armor_ac+=6;
         }
      }
   }         
   
   if (wield)
      cond = wield->value[0];
   else
      cond = 1000;
      
   if (weapon_ac > 0)
   {
      weapon_ac -= (3 -(cond / 251));
      if (weapon_ac < 1)
         weapon_ac = 1;
   }
   victim_ac = armor_ac - weapon_ac;  //negative is a better to hit
   /* if you can't see what's coming... */
   if (!can_see(ch, victim))
      victim_ac += 1;
   if (!can_see(victim, ch))
      victim_ac -= 1;

   /*
    * The moment of excitement!
    */
    
   if ((shield = get_eq_char(victim, WEAR_SHIELD)) != NULL && IS_AWAKE(victim))
   {
      int spoints = POINT_LEVEL(GET_POINTS(victim, gsn_shieldblock, 0, 1), GET_MASTERY(victim, gsn_shieldblock, 0, 1));
      if (spoints > 0 && number_range(1, 100) <= UMIN(95, (UMIN(15, URANGE(-20, victim->apply_shield, 20)/2))+ shield->value[2]+ (spoints/4)))
      {
         learn_from_success(victim, gsn_shieldblock, NULL);
         block = 1;
      }
      else
      {
         if (spoints > 0)
            learn_from_failure(victim, gsn_shieldblock, NULL);
      }
   }
   else if (victim->apply_shield > 0)
   {
      if (number_range(1, 100) <= URANGE(-20, victim->apply_shield, 20))
         block = 1;
   }
   
   tohit = get_hit_or_miss(ch, victim, victim_ac, ch->grip, limb, noarmor, wield, dt);
   if (dt == gsn_powerslice) //Don't really need this any longer after the hit or miss check
      dt = TYPE_HIT;
   if (tohit <= -200) //Crit and sliced off a limb
   {
      tohit = tohit*-1;
      slimb = 1;
   }
   if (tohit == DM_MISS || block) //miss or block
   {
      /* Miss. */
      miss = 1;
      if (!IS_NPC(ch) && number_range(1, 4) == 1)
         adjust_agi_str(ch, wield, victim); //adjust agility and strength from attack
   }
   else //hit
   {
      if (!IS_NPC(ch))
         adjust_agi_str(ch, wield, victim); //adjust agility and strength from attack
   }
   /*
    * Calc damage.
    */

   if (!wield) /* bare hand dice formula fixed by Thoric */
   {
      dam = number_range(ch->barenumdie+ch->damplus, (ch->baresizedie * ch->barenumdie)+ch->damplus);
      if (fight_unarmed(ch))
         dam+=number_range(4, 6);
   }
   else
   {
      dam = number_range(wield->value[1], wield->value[2]);
      if (IS_OBJ_STAT(wield, ITEM_TWOHANDED) && ch->race < MAX_PC_RACE)
      {
         int tpoints = POINT_LEVEL(LEARNED(ch, gsn_weapon_twohanded), MASTERED(ch, gsn_weapon_twohanded));
         int tmod = wield->value[3] - race_table[ch->race]->weaponmin + 1;
         int tpercent = 0;
         if (tmod >= 5 && LEARNED(ch, gsn_weapon_twohanded))
         {
            learn_from_success(ch, gsn_weapon_twohanded, victim);
            tpercent += tpoints*2/5;
            if (tmod == 5)
               dam += dam * tpercent / 100;
            if (tmod == 6)
               dam += dam * tpercent * 2 / 100;
            if (tmod == 7)
               dam += dam * tpercent * 3 / 100;
         }
         
      }
   }

   dam += str_app[get_curr_str(ch)].todam; //Strength Bonus   
   if (dam <= 3)
   {
      if (number_range(1, 5) == 1)
         suc = 1;
   }
   else if (dam <= 6)
   {
      if (number_range(1, 3) == 1)
         suc = 1;
   }
   else if (dam <= 10)
   {
      if (number_range(1, 2) == 1)
         suc = 1;
   }
   else
      suc = 1;
   
   if (miss == 0 && (IS_NPC(victim) && !xIS_SET(victim->act, ACT_NOINSTA))) //Cannot crit mobs flagged noinsta
   {
      if (tohit == DM_DEATH)
         crit = 1; //Death
   }
   if (miss != 1)
   {
      dam -= URANGE(-6, victim->apply_stone, 6);
      dam -= URANGE(-4, victim->apply_hardening, 4);
      if (victim->apply_hardening > 0)
         check_aff_learn(victim, "hardening", 0, ch, 1);
      if (!IS_NPC(victim) && victim->pcdata->learned[gsn_krundi_style] > 0)
      {
         int level;
         level = POINT_LEVEL(LEARNED(victim, gsn_krundi_style), MASTERED(victim, gsn_krundi_style));
         level = number_range(level*3, level*4);
         level += 100;
         level = URANGE(100, level, 600);
         dam -= level/100;
         level = level % 100;
         if (number_range(1, 100) <= level)
         {
            dam--;
         }
         learn_from_success(victim, gsn_krundi_style, ch);  
      }  
      dam = UMAX(1, dam);   
   }
   //Calculated fightstyle damage
   if (miss != 1 && suc)
   {
      dam = get_fightingstyle_dam(victim, dam, ch, 0, 1);
      dam = get_fightingstyle_dam(ch, dam, victim, 1, 1);
   }
   else
   {
      dam = get_fightingstyle_dam(victim, dam, ch, 0, 0);
      dam = get_fightingstyle_dam(ch, dam, victim, 1, 0);   
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_concentration] > 0)
   {
      int dam1, dam2;
      level = POINT_LEVEL(LEARNED(ch, gsn_concentration), MASTERED(ch, gsn_concentration));
      mastery = MASTERED(ch, gsn_concentration);
      
      dam1 = level/15;

      if (mastery == 6)
         hnum = number_range(35, 30);
      else if (mastery == 5)
         hnum = number_range(30, 25);
      else if (mastery == 4)
         hnum = number_range(18, 25);
      else if (mastery == 3)
         hnum = number_range(10, 17);
      else if (mastery == 2)
         hnum = number_range(5, 10);
      else
         hnum = number_range(1, 5);
      dam2 = (int) (dam * hnum / 150);
      
      if (dam1 < dam2)
         dam += dam1;
      else
         dam += dam2;
      if (miss != 1 && suc)
         learn_from_success(ch, gsn_concentration, victim);
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0)
   {
      int dam1, dam2;
      level = POINT_LEVEL(LEARNED(ch, gsn_enhanced_damage), MASTERED(ch, gsn_enhanced_damage));
      mastery = MASTERED(ch, gsn_enhanced_damage);
      
      dam1 = level/10;

      if (mastery == 6)
         hnum = number_range(35, 30);
      else if (mastery == 5)
         hnum = number_range(30, 25);
      else if (mastery == 4)
         hnum = number_range(18, 25);
      else if (mastery == 3)
         hnum = number_range(10, 17);
      else if (mastery == 2)
         hnum = number_range(5, 10);
      else
         hnum = number_range(1, 5);
      dam2 = (int) (dam * hnum / 100);
      
      if (dam1 < dam2)
         dam += dam1;
      else
         dam += dam2;
      if (miss != 1 && suc)
         learn_from_success(ch, gsn_enhanced_damage, victim);
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_deadly_accuracy] > 0)
   {
      int dam1, dam2;
      level = POINT_LEVEL(LEARNED(ch, gsn_deadly_accuracy), MASTERED(ch, gsn_deadly_accuracy));
      mastery = MASTERED(ch, gsn_deadly_accuracy);
      
      dam1 = level/15;

      if (mastery == 6)
         hnum = number_range(35, 30);
      else if (mastery == 5)
         hnum = number_range(30, 25);
      else if (mastery == 4)
         hnum = number_range(18, 25);
      else if (mastery == 3)
         hnum = number_range(10, 17);
      else if (mastery == 2)
         hnum = number_range(5, 10);
      else
         hnum = number_range(1, 5);

      dam2 = (int) (dam * hnum / 150);
      
      if (dam1 < dam2)
         dam += dam1;
      else
         dam += dam2;
      if (miss != 1 && suc)
         learn_from_success(ch, gsn_deadly_accuracy, victim);
   }
   if (!IS_NPC(ch) && wielding_skill_weapon(ch, 0))
   {
      int wskill = wielding_skill_weapon(ch, 0);
      if (wskill == 1)
      {
         if (ch->pcdata->learned[gsn_weapon_axe] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_axe), MASTERED(ch, gsn_weapon_axe)) / 600);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_axe, victim);
         }
      }
      if (wskill == 2)
      {
         if (ch->pcdata->learned[gsn_weapon_sword] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_sword), MASTERED(ch, gsn_weapon_sword)) / 600);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_sword, victim);
         }
      }
      if (wskill == 3)
      {
         if (ch->pcdata->learned[gsn_weapon_polearm] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_polearm), MASTERED(ch, gsn_weapon_polearm)) / 600);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_polearm, victim);
         }
      }
      if (wskill == 4)
      {
         if (ch->pcdata->learned[gsn_weapon_blunt] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_blunt), MASTERED(ch, gsn_weapon_blunt)) / 600);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_blunt, victim);
         }
      }
      if (wskill == 5)
      {
         if (ch->pcdata->learned[gsn_weapon_staff] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_staff), MASTERED(ch, gsn_weapon_staff)) / 600);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_staff, victim);
         }
      }
      if (wskill == 7)
      {
         if (ch->pcdata->learned[gsn_weapon_dagger] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_dagger), MASTERED(ch, gsn_weapon_dagger)) / 600);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_dagger, victim);
         }
         if (ch->pcdata->learned[gsn_weapon_daggerstudy] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_daggerstudy), MASTERED(ch, gsn_weapon_daggerstudy)) / 600);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_daggerstudy, victim);
         }
         if (ch->pcdata->learned[gsn_weapon_daggerstrike] > 0)
         {
            dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_daggerstrike), MASTERED(ch, gsn_weapon_daggerstrike)) / 300);
            if (miss != 1 && suc)
               learn_from_success(ch, gsn_weapon_daggerstrike, victim);
         }
      }
   }
   for (paf = ch->first_affect; paf; paf = paf->next)
   {
      if (paf->type == gsn_berserk);
      {
         dam += UMAX(1, dam * UMIN(paf->modifier/2, 40) / 150);
         break;
      }
   }
      
   if (!IS_AWAKE(victim))
      dam *= 2;
   if (dt == gsn_backstab)
   {
      level = POINT_LEVEL(LEARNED(ch, gsn_backstab), MASTERED(ch, gsn_backstab));
      dam = dam * (150 + (URANGE(25, ((get_curr_dex(ch) - get_curr_dex(victim))*12) + (level*2), 200))) / 100;
   } 
   if (dt == gsn_circle)
   {
      level = POINT_LEVEL(LEARNED(ch, gsn_circle), MASTERED(ch, gsn_circle));
      dam = dam * (150 + (URANGE(25, ((get_curr_dex(ch) - get_curr_dex(victim))*12) + (level*2), 200))) / 150;
   }
   if (dt == gsn_unsheath)
   {
      dam = dam * (number_range(110, 180) / 100);
   }
   /* New Circle like Attack for Swordsman -- Xerves 8/6/99 */
   if (dt == gsn_kick_back)
   {
      level = POINT_LEVEL(LEARNED(ch, gsn_kick_back), MASTERED(ch, gsn_kick_back));
      dam = dam * (150 + (URANGE(25, ((get_curr_str(ch) - get_curr_str(ch))*12) + (level*2), 200))) / 150;
   }
   if (dt == gsn_pincer)
   {
      int plevel;
      plevel = POINT_LEVEL(LEARNED(ch, gsn_pincer), MASTERED(ch, gsn_pincer));
      dam = dam * URANGE(170, (170 + plevel), 240) / 100;
   }
   if (!IS_NPC(victim) && victim->pcdata->learned[gsn_counter] > 0 && victim->pcdata->ranking[gsn_counter] > 0 && !xIS_SET(victim->act, PLR_COUNTER))
   {
      percent = number_range(1, 10000);
      mastery = MASTERED(ch, gsn_counter);

      if (mastery == 6)
         hnum = number_range(220, 200);
      else if (mastery == 5)
         hnum = number_range(180, 160);
      else if (mastery == 4)
         hnum = number_range(135, 150);
      else if (mastery == 3)
         hnum = number_range(90, 105);
      else if (mastery == 2)
         hnum = number_range(50, 90);
      else
         hnum = number_range(20, 45);

      holder = 100 * (hnum / 20);

      if (victim->pcdata->tier > 1)
         holder = holder + 200;
      if (victim->pcdata->caste > 5)
         holder = holder + 100;
      holder += 100 * (get_curr_dex(victim) - get_curr_dex(ch));
      holder += 50 * (get_curr_lck(victim) - get_curr_lck(ch));
      holder += 100 * (get_curr_str(victim) - get_curr_str(ch));
      if (percent < holder)
      {
         counter = 1;
         learn_from_success(victim, gsn_counter, ch);
      }
   } 
   if (dam <= 0)
      dam = 1;
 
   if (IS_NPC(ch))
   {
      dam += number_range(ch->damaddlow, ch->damaddhi);
   }  

   plusris = 0;
   /* immune to damage */
   if (dam == -1)
   {
      if (dt >= 0 && dt < top_sn)
      {
         SKILLTYPE *skill = skill_table[dt];
         bool found = FALSE;

         if (skill->imm_char && skill->imm_char[0] != '\0')
         {
            act(AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR);
            found = TRUE;
         }
         if (skill->imm_vict && skill->imm_vict[0] != '\0')
         {
            act(AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT);
            found = TRUE;
         }
         if (skill->imm_room && skill->imm_room[0] != '\0')
         {
            act(AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT);
            found = TRUE;
         }
         if (found)
            return rNONE;
      }
      dam = 0;
   }
   /* Counter Attack Check -- Xerves 8/25/99 */
   if (counter == 1)
   {  
      retcode = damage(victim, ch, dam, TYPE_HIT, DM_COUNTER, LM_BODY);
      return retcode;
   }
   if (block == 1)
   {
      blockdam = dam;
      dam = 0;    
   }
   if (miss == 0)
      spec = DM_HIT;
   if (miss == 1)
      spec = DM_MISS;
   if (crit == 1)
      spec = DM_DEATH;
   if (dt == gsn_pincer && tohit >= 300)
   {
      spec = DM_HIT;
      tohit = 200;
   }
   if (spec == DM_HIT)
   {
      dam = dam * (tohit-100) / 100;
      if (dam <= 0)
         dam = 1;
   }
   if (tohit >= 250)
   {
      if (slimb == 1)
         spec = DM_SLICEDLIMB;
      else
         spec = DM_CRITICAL;
   }
   if (block == 1)
      spec = DM_BLOCK;
      
   if (limb == LM_RLEG || limb == LM_LLEG)
      dam = dam * 110 / 100;
   if (limb == LM_LARM || limb == LM_RARM)
      dam = dam * 105 / 100;
   if (limb == LM_HEAD)
      dam = dam * 150 / 100;  
   //Already critical don't change
   if (limb == LM_NECK)
      dam = dam * 100 / 100;  
   
   if (spec == DM_HIT)
   {
      dam += URANGE(-15, ch->apply_sanctify, 15);  
      
      if (ch->apply_manaburn > 0)
         check_aff_learn(ch, "manaburn", 0, victim, 1);
      dam += URANGE(-10, ch->apply_manaburn, 10);  
   }
   //Want the melee's to pinch a bit, mainly need this vs players.   
   if (spec == DM_HIT && IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
   {
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (kmob->vnum == ch->pIndexData->vnum)
         {
            if (xIS_SET(kmob->flags, KMOB_DAM1))
               dam+=2;
            if (xIS_SET(kmob->flags, KMOB_DAM2))
               dam+=3;
            if (xIS_SET(kmob->flags, KMOB_DAM3))
               dam+=5;
            if (xIS_SET(kmob->flags, KMOB_DAM4))
               dam+=7;
         }
      }
   }
      
   if (miss == 0 && dam <= 0)
      dam = 1;

   if (wield && wield->wear_loc == WEAR_DUAL_WIELD)
   {
      if ((dual = get_eq_char(ch, WEAR_WIELD)) != NULL)
      {
         wield->wear_loc = WEAR_WIELD;
         dual->wear_loc = WEAR_DUAL_WIELD;
      }
   }
   if ((retcode = damage(ch, victim, dam, dt, spec, limb)) != rNONE)
   {
      if (dual && dual->wear_loc != -1)
         dual->wear_loc = WEAR_WIELD;
      if (wield && dual && wield->wear_loc != -1)
         wield->wear_loc = WEAR_DUAL_WIELD;
      if (wield && !dual && wield->wear_loc != -1)
         wield->wear_loc = WEAR_WIELD;
   }
   else
   {
      if (dual && dual->wear_loc != -1)
         dual->wear_loc = WEAR_WIELD;
      if (wield && dual && wield->wear_loc != -1)
         wield->wear_loc = WEAR_DUAL_WIELD;
      if (wield && !dual && wield->wear_loc != -1)
         wield->wear_loc = WEAR_WIELD;
   }
   if (char_died(ch))
      return rCHAR_DIED;
   if (char_died(victim))
      return rVICT_DIED;

   retcode = rNONE;
   if (dam == 0)
      return retcode;

   tail_chain();
   return retcode;
}

/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */

// Moved to archery.c
//ch_ret projectile_hit( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
//         OBJ_DATA *projectile, sh_int dist )

/*
 * Calculate damage based on resistances, immunities and suceptibilities
 *					-Thoric
 */
sh_int ris_damage(CHAR_DATA * ch, sh_int dam, int ris)
{
   sh_int modifier = 0;
   
   if (ris == RIS_FIRE)
      modifier = ch->apply_res_fire[0];
   if (ris == RIS_WATER)
      modifier = ch->apply_res_water[0];
   if (ris == RIS_EARTH)
      modifier = ch->apply_res_earth[0];
   if (ris == RIS_ENERGY)
      modifier = ch->apply_res_energy[0];
   if (ris == RIS_AIR)
      modifier = ch->apply_res_air[0];
   if (ris == RIS_NONMAGIC)
      modifier = ch->apply_res_nonmagic[0];
   if (ris == RIS_MAGIC)
      modifier = ch->apply_res_magic[0];
   if (ris == RIS_BLUNT)
      modifier = ch->apply_res_blunt[0];
   if (ris == RIS_PIERCE)
      modifier = ch->apply_res_pierce[0];
   if (ris == RIS_SLASH)
      modifier = ch->apply_res_slash[0];
   if (ris == RIS_PARALYSIS)
      modifier = ch->apply_res_paralysis[0];
   if (ris == RIS_POISON)
      modifier = ch->apply_res_poison[0];
   if (ris == RIS_UNDEAD)
      modifier = ch->apply_res_undead[0];
   if (ris == RIS_UNHOLY)
      modifier = ch->apply_res_unholy[0];
   if (ris == RIS_HOLY)
      modifier = ch->apply_res_holy[0];
      
   if (modifier >= -499 && modifier <= 0)
      modifier = 1;
   if (modifier == -500)
      return -1;
      
   if ((ris == RIS_FIRE || ris == RIS_WATER || ris == RIS_EARTH || ris == RIS_ENERGY || ris == RIS_AIR
   || ris == RIS_UNHOLY || ris == RIS_HOLY || ris == RIS_UNDEAD) && ch->apply_res_magic[0] != 0)
   {
      if (IS_SET(ch->immune, RIS_MAGIC) && !IS_SET(ch->no_immune, RIS_MAGIC))
         return -1;
      if (ch->apply_res_magic[0] == -500)
         return -1;
      if (modifier == 100)
         modifier = ch->apply_res_magic[0];
      if (modifier == 0)
         modifier = 100;
      else if (modifier < 100 && ch->apply_res_magic[0] < 100 && ch->apply_res_magic[0] < modifier)
         modifier = ch->apply_res_magic[0];
      else if (modifier > 100 && ch->apply_res_magic[0] > 100 && ch->apply_res_magic[0] > modifier)
         modifier = ch->apply_res_magic[0];
      else if ((modifier < 100 && ch->apply_res_magic[0] > 100) || (modifier > 100 && ch->apply_res_magic[0] < 100))
      {
         if (modifier > 100)
            modifier = modifier - (100-ch->apply_res_magic[0]);
         else
            modifier = ch->apply_res_magic[0] - (100-modifier);
      }
      if (IS_SET(ch->resistant, RIS_MAGIC) && !IS_SET(ch->no_resistant, RIS_MAGIC) && modifier == 100)
         modifier -= 30;
      if (IS_SET(ch->susceptible, RIS_MAGIC) && !IS_SET(ch->no_susceptible, RIS_MAGIC) && modifier == 100)
         modifier += 50;
   }
   if ((ris == RIS_BLUNT || ris == RIS_PIERCE || ris == RIS_SLASH || ris == RIS_POISON || ris == RIS_PARALYSIS)
   &&  ch->apply_res_nonmagic[0] != 0)
   {
      if (IS_SET(ch->immune, RIS_NONMAGIC) && !IS_SET(ch->no_immune, RIS_NONMAGIC))
         return -1;
      if (ch->apply_res_nonmagic[0] == -500)
         return -1;
      if (modifier == 100)
         modifier = ch->apply_res_nonmagic[0];
      if (modifier < 100 && ch->apply_res_nonmagic[0] < 100 && ch->apply_res_nonmagic[0] < modifier)
         modifier = ch->apply_res_nonmagic[0];
      if (modifier > 100 && ch->apply_res_nonmagic[0] > 100 && ch->apply_res_nonmagic[0] > modifier)
         modifier = ch->apply_res_nonmagic[0];
      if ((modifier < 100 && ch->apply_res_nonmagic[0] > 100) || (modifier > 100 && ch->apply_res_nonmagic[0] < 100))
      {
         if (modifier > 100)
            modifier = modifier - (100-ch->apply_res_nonmagic[0]);
         else
            modifier = ch->apply_res_nonmagic[0] - (100-modifier);
      }
      if (IS_SET(ch->resistant, RIS_NONMAGIC) && !IS_SET(ch->no_resistant, RIS_NONMAGIC) && modifier == 100)
         modifier -= 30;
      if (IS_SET(ch->susceptible, RIS_NONMAGIC) && !IS_SET(ch->no_susceptible, RIS_NONMAGIC) && modifier == 100)
         modifier += 50;
   }

   if (IS_SET(ch->immune, ris) && !IS_SET(ch->no_immune, ris))
      return -1;
   if (IS_SET(ch->resistant, ris) && !IS_SET(ch->no_resistant, ris) && modifier == 100)
      modifier -= 30;
   if (IS_SET(ch->susceptible, ris) && !IS_SET(ch->no_susceptible, ris) && modifier == 100)
      modifier += 50;
   return (dam * modifier) / 100;
}

int get_sore_rate(int race, int maxhp)
{
   if (race == RACE_HUMAN)
      maxhp*=3;
   if (race == RACE_ELF)
      maxhp*=2.8;
   if (race == RACE_HOBBIT)
      maxhp*=2.5;
   if (race == RACE_FAIRY)
      maxhp*=2.1;
   if (race == RACE_DWARF)
      maxhp*=4;
   if (race == RACE_OGRE)
      maxhp*=5;
      
   return URANGE(1, maxhp, 9999);
}

int adjust_mil_damage(CHAR_DATA *ch, int dam)
{
   CHAR_DATA *victim;
   BUYKMOB_DATA *kmob;

   if (!IS_NPC(ch))
      return 0;
   if (!IN_WILDERNESS(ch))
      return 0;
   if (!xIS_SET(ch->act, ACT_MILITARY))
      return 0;
   if (dam == 0)
      return 0;
   
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (!xIS_SET(victim->act, ACT_MILITARY))
         continue;
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (kmob->vnum == victim->pIndexData->vnum)
            if (xIS_SET(kmob->flags, KMOB_SOLDIERDAM))  
               if (abs(ch->coord->x - victim->coord->x) <= 10 && abs(ch->coord->y - victim->coord->y) <= 10
               && ch->m4 == victim->m4)
                  break;
      }
      if (kmob)
         return UMAX(1, dam*25/100);
   }
   return 0;
}  

int player_stat_worth(CHAR_DATA *ch)
{
   int points = 0;
   int sn;
   
   if (IS_NPC(ch))
      return 0;
      
   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      points += ch->pcdata->learned[sn];
   }
   points /=3;
   points += ch->max_hit/2;
   points += ch->max_mana/2;
   points += (ch->perm_str - (13+race_table[ch->race]->str_plus)) * 5;
   points += (ch->perm_lck - (13+race_table[ch->race]->lck_plus)) * 5;
   points += (ch->perm_con - (13+race_table[ch->race]->con_plus)) * 5;
   points += (ch->perm_dex - (13+race_table[ch->race]->dex_plus)) * 5;
   points += (ch->perm_int - (13+race_table[ch->race]->int_plus)) * 5;
   points += (ch->perm_wis - (13+race_table[ch->race]->wis_plus)) * 5;
   points += (ch->perm_str - (13+race_table[ch->race]->str_plus)) * 5;
   points += (ch->perm_agi - 15)/2;
   points *= points/2;
   return points;
}

int eqworth_affcheck2(OBJ_DATA *obj, AFFECT_DATA *paf)
{
   int mod;
   int points = 0;
   mod = paf->modifier;
   switch (paf->location % REVERSE_APPLY)
   {
      default:
         bug("eqworth_affcheck2: unknown location %d.", paf->location);
         return 0;
         
      case APPLY_ARMOR:
         if (mod > 0)
            points += 5000*mod;
         break;
      case APPLY_SHIELD:
         if (mod > 0)
            points += 2000*mod;
         break;
      case APPLY_STONE:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_SANCTIFY:
         if (mod > 0)
            points += 10000*mod;
         break;
      case APPLY_WMOD:
         if (mod < 100)
            points += 500*(100-mod);
         break;         
         
      case APPLY_RFIRE:
      case APPLY_RWATER:
      case APPLY_RAIR:
      case APPLY_REARTH:
      case APPLY_RENERGY:
      case APPLY_RMAGIC:
      case APPLY_RNONMAGIC:
      case APPLY_RBLUNT:
      case APPLY_RPIERCE:
      case APPLY_RSLASH:
      case APPLY_RPOISON:
      case APPLY_RPARALYSIS:
      case APPLY_RHOLY:
      case APPLY_RUNHOLY:
      case APPLY_RUNDEAD:
         if (mod < 100)
            points += 500*(100-mod);
         break;
      case APPLY_MANAFUSE:
         if (mod > 0)
            points += 800*mod;
      case APPLY_FASTING:
         if (mod > 0)
            points += 800*mod;
         break;
      case APPLY_MANASHELL:
         if (mod > 0)
            points += 800*mod;
         break;
      case APPLY_MANASHIELD:
         if (mod > 0)
            points += 800*mod;
         break;
      case APPLY_MANAGUARD:
         if (mod > 0)
            points += 800*mod;
         break;
         
      case APPLY_MANABURN:
         if (mod > 0)
            points += 800*mod;
         break;
      case APPLY_WEAPONCLAMP:
         if (mod > 0)
            points += 800*mod;
         break;  
      case APPLY_ARROWCATCH:
         if (mod > 0)
            points += 800*mod;
         break;  
      case APPLY_BRACING:
         if (mod > 0)
            points += 800*mod;
         break;    
      case APPLY_HARDENING:
         if (mod > 0)
            points += 800*mod;
         break;   
      case APPLY_TOHIT:
         if (mod > 0)
            points += 3000*mod;
         break;
      case APPLY_MANATICK:
         if (mod > 100)
            points += 100*(mod-100);
         break;
      case APPLY_HPTICK:
         if (mod > 100)
            points += 100*(mod-100);
         break;
      case APPLY_STR:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_DEX:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_INT:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_WIS:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_CON:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_CHA:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_LCK:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_AGI:
         if (mod > 0)
            points += 7000*mod;
         break;
      case APPLY_MANA:
         if (mod > 0)
            points += 800*mod;
         break;
      case APPLY_HIT:
         if (mod > 0)
            points += 800*mod;
         break;
      case APPLY_SAVING_POISON:
         if (mod < 0)
            points += -300*mod;
         break;
      case APPLY_SAVING_ROD:
         if (mod < 0)
            points += -300*mod;
         break;
      case APPLY_SAVING_PARA:
         if (mod < 0)
            points += -300*mod;
         break;
      case APPLY_SAVING_BREATH:
         if (mod < 0)
            points += -300*mod;
         break;
      case APPLY_SAVING_SPELL:
         if (mod < 0)
            points += -300*mod;
         break;
      case APPLY_AFFECT:
         points += 5000;
         break;
      case APPLY_EXT_AFFECT:
         points += 5000;
         break;
      case APPLY_RESISTANT:
         points += 5000;
         break;
      case APPLY_IMMUNE:
         points += 50000;
         break;
      case APPLY_WEAPONSPELL: /* see fight.c */
         points += 50000;
         break;

      case APPLY_WEARSPELL:
      case APPLY_REMOVESPELL:
         points += 30000;
         break;
       
      case APPLY_TRACK:
      case APPLY_HIDE:
      case APPLY_STEAL:
      case APPLY_SNEAK:
      case APPLY_PICK:
      case APPLY_BACKSTAB:
      case APPLY_DETRAP:
      case APPLY_DODGE:
      case APPLY_PEEK:
      case APPLY_SCAN:
      case APPLY_GOUGE:
      case APPLY_SEARCH:
      case APPLY_DIG:
      case APPLY_DISARM:
      case APPLY_PARRY:
      case APPLY_STUN:
      case APPLY_CLIMB:
      case APPLY_GRIP:
      case APPLY_SCRIBE:
      case APPLY_BREW:
      case APPLY_COOK:
         if (mod > 0)
            points += 5000*mod;
         break;

      case APPLY_ROOMFLAG:
      case APPLY_KICK:
      case APPLY_SUSCEPTIBLE:
      case APPLY_REMOVE:
      case APPLY_SEX:
      case APPLY_CLASS:
      case APPLY_LEVEL:
      case APPLY_AGE:
      case APPLY_NONE:
      case APPLY_GOLD:
      case APPLY_EXP:
      case APPLY_HEIGHT:
      case APPLY_WEIGHT:
      case APPLY_FULL:
      case APPLY_THIRST:
      case APPLY_DRUNK:
      case APPLY_MOVE:
      case APPLY_HITROLL:
      case APPLY_DAMROLL:
      case APPLY_BLOOD:
      case APPLY_MENTALSTATE:
      case APPLY_EMOTION:
      case APPLY_CONTAGIOUS:
      case APPLY_ODOR:
      case APPLY_STRIPSN:
      case APPLY_PALM: 
      case APPLY_MOUNT:
      case APPLY_BASH:
      case APPLY_SECTORTYPE:
      case APPLY_ROOMLIGHT:
      case APPLY_PUNCH:
      case APPLY_TELEVNUM:
         break;
   }
   return points;
}

int eqworth_affcheck(OBJ_DATA *obj)
{
   AFFECT_DATA *paf;
   int points = 0;
   
   for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
      points += eqworth_affcheck2(obj, paf);
   for (paf = obj->first_affect; paf; paf = paf->next)
      points += eqworth_affcheck2(obj, paf);
      
   return points;
}

int eqworth_statcheck(OBJ_DATA *obj)
{
   int points = 0;
   int avdam;
   int mdiff;
   if (obj->item_type == ITEM_MISSILE_WEAPON)
   {
      avdam = (obj->value[1]+obj->value[2])/2;
      points += UMAX(1, avdam * avdam/2)*100;  
      points += UMAX(1, obj->value[9] * obj->value[9]/2)*100;  
      points += UMAX(1, obj->value[10] * obj->value[10]/2)*100;  
   }     
   if (obj->item_type == ITEM_PROJECTILE)
   {
      avdam = (obj->value[1]+obj->value[2])/2;
      points += UMAX(1, avdam * avdam/2)*100;   
      points += UMAX(1, obj->value[9] * obj->value[9]/2)*100;  
   }
   if (obj->item_type == ITEM_WEAPON)
   {
      avdam = (obj->value[1]+obj->value[2])/2;
      points += UMAX(1, avdam * avdam/2)*100;
      if (obj->value[4] > 0 && obj->value[5] >= 1001 && obj->value[5] <= 1007)
      {
         mdiff = skill_table[obj->value[4]]->masterydiff[0];
         points += 1000*mdiff*mdiff*mdiff;
      }
      points += UMAX(1, obj->value[7] * obj->value[7]/2)*100;   
      points += UMAX(1, obj->value[8] * obj->value[8]/2)*100;   
      points += UMAX(1, obj->value[9] * obj->value[9]/2)*100;  
      points += UMAX(1, obj->value[10] * obj->value[10]/2)*100;  
   }
   if (obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
   {
      points += UMAX(1, obj->value[0] * obj->value[0]/2)*100;   
      points += UMAX(1, obj->value[1] * obj->value[1]/2)*100;   
      points += UMAX(1, obj->value[2] * obj->value[2]/2)*100;   
      points += UMAX(1, obj->value[4] * obj->value[4]/2)*100;  
   }
   if (obj->item_type == ITEM_ARMOR && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
   {
      points += obj->value[0]*10;
   }
   return points;
}
   
int player_equipment_worth(CHAR_DATA *ch)
{
   int points = 0;
   OBJ_DATA *obj;
   
   if ((obj=get_eq_char(ch, WEAR_FINGER_R)) != NULL)
      points += eqworth_affcheck(obj);
   if ((obj=get_eq_char(ch, WEAR_FINGER_L)) != NULL)
      points += eqworth_affcheck(obj);
   if ((obj=get_eq_char(ch, WEAR_ABOUT_NECK)) != NULL)
      points += eqworth_affcheck(obj);
   if ((obj=get_eq_char(ch, WEAR_NOCKED)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_BACK)) != NULL)
   {
      points += eqworth_affcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_WIELD)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_BODY)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_HEAD)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_LEG_L)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_LEG_R)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_ARM_L)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_ARM_R)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_SHIELD)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_WAIST)) != NULL)
   {
      points += eqworth_affcheck(obj);
   }
   if ((obj=get_eq_char(ch, WEAR_NECK)) != NULL)
   {
      points += eqworth_affcheck(obj);
      points += eqworth_statcheck(obj);
   }
   return points;
}

int get_player_statlevel(CHAR_DATA *ch)
{
   int ox = 0;
   int x = 0;
   int cnt;
   int points = player_equipment_worth(ch) + player_stat_worth(ch);
   
   for (cnt = 1;cnt <= 2000;cnt++)
   {
      ox = x;
      x = ox +(50*cnt*cnt/3);
      if (points >= ox && points <= x)
         break;
   }
   return cnt;
   
   
   return points / 10000;
}  

void spoint_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
   CHAR_DATA *gch;
   int members = 0;
   int tpoints = 0;
   int points = 0;
   int tgpoints = 0;   //team points
   int tglpoints = 2000000000;  //team lowest
   int tghpoints = 0;  //team highest
   int gain;
   
   if (victim == ch)
      return;
      
   //Not on mounts
   if (xIS_SET(victim->act, ACT_MOUNTSAVE))
      return;
      
   //Not on players either
   if (!IS_NPC(victim))
      return;
      
   tpoints = UMAX(1, ((victim->max_hit) - 10) * 3);
   tpoints = UMAX(1, (tpoints * number_range(90, 110) / 100));
   
   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM) && ch->master && ch->master->in_room == ch->in_room && !IS_NPC(ch->master))
   {
      tpoints = tpoints * 8 / 10;
      ch = ch->master;
   }
   else if (IS_NPC(ch))
      return;
      
   for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
   {
      if (is_same_group(gch, ch) && !IS_NPC(gch))
      {
         members++;
         points = player_stat_worth(gch);
         tgpoints += points;
         if (points < tglpoints)
            tglpoints = points;
         if (points > tghpoints)
            tghpoints = points;
      }
   }
   if (members <= 1)
   {
      if (ch->race == RACE_HUMAN)
         tpoints = tpoints * 120/100;
      if (xIS_SET(ch->pcdata->talent, TALENT_SP5))
         tpoints = tpoints * 300/100;
      else if (xIS_SET(ch->pcdata->talent, TALENT_SP4))
         tpoints = tpoints * 250/100;
      else if (xIS_SET(ch->pcdata->talent, TALENT_SP3))
         tpoints = tpoints * 200/100;
      else if (xIS_SET(ch->pcdata->talent, TALENT_SP2))
         tpoints = tpoints * 166/100;
      else if (xIS_SET(ch->pcdata->talent, TALENT_SP1))
         tpoints = tpoints * 133/100;
      #ifdef MCCP         
      if (ch->desc && ch->desc->out_compress)
         tpoints = tpoints * 105/100;
      #endif
      if (ch->desc && ch->desc->mxp && !str_cmp(ch->desc->mxpclient, "zmud") && ch->desc->mxpversion >= 6.5)
         tpoints = tpoints * 110/100;
      else if (ch->desc && ch->desc->mxp)
         tpoints = tpoints * 105/100;
         
      tpoints = UMAX(1, tpoints * sysdata.exp_percent / 100);
      ch->pcdata->spoints += tpoints;
      ch_printf(ch, "&w&WYou gain %d skill points&w\n\r", tpoints);
   }
   else
   {
      if ((tghpoints/2) > tglpoints && tghpoints >= 20 && tglpoints < 150000 && !sysdata.resetgame)
         ;
      else
         tpoints += ((members-1)*35)*tpoints/100;
      for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
      {
         if (is_same_group(gch, ch) && !IS_NPC(gch))
         {
            if (tglpoints >= 150000 || sysdata.resetgame)
               gain = tpoints / members;
            else
               gain = UMAX(1, player_stat_worth(gch) * 500 / tgpoints * tpoints / 500);
            
            if (gch->race == RACE_HUMAN)
               gain = gain * 120/100;
            if (xIS_SET(gch->pcdata->talent, TALENT_SP5))
               gain = gain * 300/100;
            else if (xIS_SET(gch->pcdata->talent, TALENT_SP4))
               gain = gain * 250/100;
            else if (xIS_SET(gch->pcdata->talent, TALENT_SP3))
               gain = gain * 200/100;
            else if (xIS_SET(gch->pcdata->talent, TALENT_SP2))
               gain = gain * 166/100;
            else if (xIS_SET(gch->pcdata->talent, TALENT_SP1))
               gain = gain * 133/100;
            #ifdef MCCP
            if (gch->desc && gch->desc->out_compress)
               gain = gain * 105/100;
            #endif
            if (gch->desc && gch->desc->mxp && !str_cmp(gch->desc->mxpclient, "zmud") && gch->desc->mxpversion >= 6.5)
               gain = gain * 110/100;
            else if (gch->desc && gch->desc->mxp)
               gain = gain * 105/100;
            gain = UMAX(1, gain * sysdata.exp_percent / 100);
            gch->pcdata->spoints += gain;
            ch_printf(gch, "&w&WYou gain %d skill points&w\n\r", gain);
         }
      }
   }   
}

//0 - Straight damage
//1 - Magical damage
//2 - Taunt
//3 - A heal
//4 - A manual adjustment, typically used in skills
void adjust_aggression_list(CHAR_DATA *victim, CHAR_DATA *ch, int dam, int type, int sn)
{
   AGGRO_DATA *aggro = NULL;
   int add = 0;
   int level;
   CHAR_DATA *vch;
   
   if (!IS_NPC(victim) && type != 3)
      return;
   
   if (type != 3)
   {
      for (aggro = victim->first_aggro; aggro; aggro = aggro->next)
      {
         if (aggro->ch && ch->name == aggro->ch->name)
            break;
      }
      if (!aggro)
      {
         CREATE(aggro, AGGRO_DATA, 1);
         aggro->ch = ch;
         aggro->owner = victim;
         LINK(aggro, victim->first_aggro, victim->last_aggro, next, prev);
         LINK(aggro, first_global_aggro, last_global_aggro, next_global, prev_global);
      }
   }
   //Don't like magic, not as much as heals though
   if (type == 1)
   {
      add = dam*3/2;
         
      if (victim->fighting && victim->fighting->who && victim->fighting->who == ch)
      {
         if (get_curr_int(victim) >= 19)
            add = add*5/6;
         if (get_curr_int(victim) >= 22)
            add = add*4/6;         
      }
      else
      {         
         if (get_curr_int(victim) >= 16)
            add = add*5/4;
         if (get_curr_int(victim) >= 19)
            add = add*6/4;
         if (get_curr_int(victim) >= 22)
            add = add*7/4;
      }
         
   }
   //Really don't like heals....
   else if (type == 3)
   {
      add = dam*4/2;
      
      for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
      {
         if (!IN_SAME_ROOM(vch, ch) || !IS_NPC(vch))
            continue;
         if (vch->fighting && vch->fighting->who && is_same_group( ch, vch->fighting->who ))
         {
            for (aggro = vch->first_aggro; aggro; aggro = aggro->next)
            {
               if (aggro->ch && ch->name == aggro->ch->name)
                  break;
            }
            if (!aggro)
            {
               CREATE(aggro, AGGRO_DATA, 1);
               aggro->ch = ch;
               aggro->owner = vch;
               LINK(aggro, first_global_aggro, last_global_aggro, next_global, prev_global);
               LINK(aggro, vch->first_aggro, vch->last_aggro, next, prev);
            }         
            if (vch->fighting->who == ch)
            {
               if (get_curr_int(vch) >= 16)
                  add = add*7/8;
               if (get_curr_int(vch) >= 19)
                  add = add*5/6;
               if (get_curr_int(vch) >= 22)
                  add = add*4/6;         
            }
            else
            {
               if (get_curr_int(vch) >= 16)
                 add = add*6/4;
               if (get_curr_int(vch) >= 19)
                 add = add*7/4;
               if (get_curr_int(vch) >= 22)
                 add = add*8/4;         
            }
            aggro->value += add;
         }
      }
      return;
   }
   else if (type == 2)
   {
      level = POINT_LEVEL(LEARNED(ch, sn), MASTERED(ch, sn));
      add = URANGE(1, level/15, 5);
      add += dam;
      if (victim->fighting->who != ch)
         add/=2;
   }
   else if (type == 0)
   {
      add = dam;
      if (get_eq_char(ch, WEAR_SHIELD))
         add = add * 2 / 3;
      if (victim->fighting && victim->fighting->who && victim->fighting->who != ch)
         add = add * 3 / 2;
   }
   else if (type == 4)
   {
      add = dam;
   }
   aggro->value += add;
}   
        
void check_limb_status(CHAR_DATA *victim, int limb, int dam, int slimb, CHAR_DATA *ch, int spec)
{  
   int smod;
   int hmod;
   OBJ_DATA *obj;
   
   if (spec == DM_CRITICAL)
      spec = number_range(35,75);
   else
      spec = 0;
   
   if (get_curr_str(victim) < 8)
      smod = 100*2;
   else if (get_curr_str(victim) > 8 && get_curr_str(victim) <= 12)
      smod = 85*2;
   else if (get_curr_str(victim) >= 13 && get_curr_str(victim) <= 16)
      smod = 70*2;
   else if (get_curr_str(victim) >= 17 && get_curr_str(victim) <= 19)
      smod = 55*2;
   else if (get_curr_str(victim) >= 20 && get_curr_str(victim) <= 22)
      smod = 35*2;
   else if (get_curr_str(victim) >= 23 && get_curr_str(victim) >= 24)
      smod = 25*2;
   else
      smod = 15*2;
      
   //arm 14  leg 15 (vnum)
   if (limb == LM_LARM && victim->con_larm > -1 && dam > 0)
   {
      hmod = UMAX(1, dam * smod / victim->max_hit)+spec; 
      victim->con_larm -= hmod;
      if (victim->con_larm < 0 || slimb == 1) //Bye bye Arm
      {
         int gotweapon = 0;
         victim->con_larm = -1;
         //create limb
         obj = create_object(get_obj_index(14), 0);
         obj->timer = number_range(4, 7);
         if (IS_AFFECTED(victim, AFF_POISON))
            obj->value[3] = 10;
         obj = obj_to_room(obj, victim->in_room, victim);
         if (slimb == 1)
         {
            act(AT_WHITE, "[CrItIcAl] $n's left arm is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "[CrItIcAl] Your left arm is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "[CrItIcAl] You cleanly slice off $n's left arm with your attack.", victim, NULL, ch, TO_VICT);
         }
         else
         {   
            act(AT_WHITE, "$n's left arm breaks off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "Your left arm breaks off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "You break off $n's left arm with your attack.", victim, NULL, ch, TO_VICT);
         }
         //remove armor
         obj = get_eq_char(victim, WEAR_ARM_L);
         if (obj)
         {
            unequip_char(victim, obj);
            obj->wear_loc = -1;
            if (IS_NPC(victim))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);   
            }
            else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
            && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);
            }
            act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
            act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
            act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
         }
         //remove weapon
         if (!IS_NPC(victim) && victim->pcdata->righthanded == 0)
            gotweapon = 1;
         if (IS_NPC(victim) && (number_range(1, 2) == 1 || victim->con_rarm == -1))
            gotweapon = 1;
         if (gotweapon == 0)
         {
            obj = get_eq_char(victim, WEAR_DUAL_WIELD);
            if (obj)
            {
               unequip_char(victim, obj);
               obj->wear_loc = -1;
               if (IS_NPC(victim))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);   
               }
               else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
               && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);
               }
               act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
               act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
               act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
            }
            obj = get_eq_char(victim, WEAR_SHIELD);
            if (obj)
            {
               unequip_char(victim, obj);
               obj->wear_loc = -1;
               if (IS_NPC(victim))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);   
               }
               else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
               && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);
               }
               act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
               act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
               act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
            }
         }
         if (gotweapon == 1)
         {
            obj = get_eq_char(victim, WEAR_WIELD);
            if (obj)
            {
               unequip_char(victim, obj);
               obj->wear_loc = -1;
               if (IS_NPC(victim))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);   
               }
               else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
               && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);
               }
               act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
               act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
               act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
               //if Dual Wielding change it to Wield
               obj = get_eq_char(victim, WEAR_DUAL_WIELD);
               if (obj)
                  obj->wear_loc = WEAR_WIELD;
            }
         }
      }
   }
   if (limb == LM_RARM && victim->con_rarm > -1 && dam > 0)
   {
      hmod = UMAX(1, dam * smod / victim->max_hit)+spec;
      victim->con_rarm -= hmod;
      if (victim->con_rarm < 0 || slimb == 1) //Bye bye Arm
      {
         int gotweapon = 0;
         victim->con_rarm = -1;
         //create limb
         obj = create_object(get_obj_index(14), 0);
         obj->timer = number_range(4, 7);
         if (IS_AFFECTED(victim, AFF_POISON))
            obj->value[3] = 10;
         obj = obj_to_room(obj, victim->in_room, victim);
         if (slimb == 1)
         {
            act(AT_WHITE, "[CrItIcAl] $n's right arm is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "[CrItIcAl] Your right arm is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "[CrItIcAl] You cleanly slice off $n's right arm with your attack.", victim, NULL, ch, TO_VICT);
         }
         else
         {   
            act(AT_WHITE, "$n's right arm breaks off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "Your right arm breaks off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "You break off $n's right arm with your attack.", victim, NULL, ch, TO_VICT);
         }
         //remove armor
         obj = get_eq_char(victim, WEAR_ARM_R);
         if (obj)
         {
            unequip_char(victim, obj);
            obj->wear_loc = -1;
            if (IS_NPC(victim))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);   
            }
            else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
            && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);
            }
            act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
            act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
            act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
         }
         //remove weapon
         if (!IS_NPC(victim) && victim->pcdata->righthanded == 1)
            gotweapon = 1;
         if (IS_NPC(victim) && (number_range(1, 2) == 1 || victim->con_larm == -1))
            gotweapon = 1;
         if (gotweapon == 0)
         {
            obj = get_eq_char(victim, WEAR_DUAL_WIELD);
            if (obj)
            {
               unequip_char(victim, obj);
               obj->wear_loc = -1;
               if (IS_NPC(victim))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);   
               }
               else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
               && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);
               }
               act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
               act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
               act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
            }
            obj = get_eq_char(victim, WEAR_SHIELD);
            if (obj)
            {
               unequip_char(victim, obj);
               obj->wear_loc = -1;
               if (IS_NPC(victim))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);   
               }
               else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
               && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);
               }
               act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
               act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
               act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
            }
         }
         if (gotweapon == 1)
         {
            obj = get_eq_char(victim, WEAR_WIELD);
            if (obj)
            {
               unequip_char(victim, obj);
               obj->wear_loc = -1;
               if (IS_NPC(victim))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);   
               }
               else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
               && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
               {
                  separate_obj(obj);
                  obj_from_char(obj);
                  obj_to_room(obj, victim->in_room, victim);
               }
               act(AT_WHITE, "$n's $p drops to the ground along with $s arm.", victim, obj, NULL, TO_NOTVICT);
               act(AT_WHITE, "Your $p falls to the ground along with your arm.", victim, obj, NULL, TO_CHAR);
               act(AT_WHITE, "$n's $p falls to the ground along with $s arm.", victim, obj, ch, TO_VICT);
               //if Dual Wielding change it to Wield
               obj = get_eq_char(victim, WEAR_DUAL_WIELD);
               if (obj)
                  obj->wear_loc = WEAR_WIELD;
            }
            
         }
      }
   }
   if (limb == LM_RLEG && victim->con_rleg > -1 && dam > 0)
   {
      hmod = UMAX(1, dam * 80 * smod / victim->max_hit/ 100)+spec;
      victim->con_rleg -= hmod;
      if (victim->con_rleg < 0 || slimb == 1) //Bye bye Leg
      {
         victim->con_rleg = -1;
         //create limb
         obj = create_object(get_obj_index(15), 0);
         obj->timer = number_range(4, 7);
         if (IS_AFFECTED(victim, AFF_POISON))
            obj->value[3] = 10;
         obj = obj_to_room(obj, victim->in_room, victim);
         if (slimb == 1)
         {
            act(AT_WHITE, "[CrItIcAl] $n's right leg is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "[CrItIcAl] Your right leg is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "[CrItIcAl] You cleanly slice off $n's right leg with your attack.", victim, NULL, ch, TO_VICT);
         }
         else
         {   
            act(AT_WHITE, "$n's right leg breaks off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "Your right leg breaks off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "You break off $n's right leg with your attack.", victim, NULL, ch, TO_VICT);
         }
         //remove armor
         obj = get_eq_char(victim, WEAR_LEG_R);
         if (obj)
         {
            unequip_char(victim, obj);
            obj->wear_loc = -1;
            if (IS_NPC(victim))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);   
            }
            else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
            && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);
            }
            act(AT_WHITE, "$n's $p drops to the ground along with $s leg.", victim, obj, NULL, TO_NOTVICT);
            act(AT_WHITE, "Your $p falls to the ground along with your leg.", victim, obj, NULL, TO_CHAR);
            act(AT_WHITE, "$n's $p falls to the ground along with $s leg.", victim, obj, ch, TO_VICT);
         }
      }
   }
   if (limb == LM_LLEG && victim->con_lleg > -1 && dam > 0)
   {
      hmod = UMAX(1, dam * 80 * smod / victim->max_hit/ 100)+spec;
      victim->con_lleg -= hmod;
      if (victim->con_lleg < 0 || slimb == 1) //Bye bye Leg
      {
         victim->con_lleg = -1;
         //create limb
         obj = create_object(get_obj_index(15), 0);
         obj->timer = number_range(4, 7);
         if (IS_AFFECTED(victim, AFF_POISON))
            obj->value[3] = 10;
         obj = obj_to_room(obj, victim->in_room, victim);
         if (slimb == 1)
         {
            act(AT_WHITE, "[CrItIcAl] $n's left leg is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "[CrItIcAl] Your left leg is cleanly sliced off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "[CrItIcAl] You cleanly slice off $n's left leg with your attack.", victim, NULL, ch, TO_VICT);
         }
         else
         {   
            act(AT_WHITE, "$n's left leg breaks off from $N's attack.", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "Your left leg breaks off from $N's attack.", victim, NULL, ch, TO_CHAR);
            act(AT_WHITE, "You break off $n's left leg with your attack.", victim, NULL, ch, TO_VICT);
         }
         //remove armor
         obj = get_eq_char(victim, WEAR_LEG_L);
         if (obj)
         {
            unequip_char(victim, obj);
            obj->wear_loc = -1;
            if (IS_NPC(victim))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);   
            }
            else if (!IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NODROP) 
            && (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area))
            {
               separate_obj(obj);
               obj_from_char(obj);
               obj_to_room(obj, victim->in_room, victim);
            }
            act(AT_WHITE, "$n's $p drops to the ground along with $s leg.", victim, obj, NULL, TO_NOTVICT);
            act(AT_WHITE, "Your $p falls to the ground along with your leg.", victim, obj, NULL, TO_CHAR);
            act(AT_WHITE, "$n's $p falls to the ground along with $s leg.", victim, obj, ch, TO_VICT);
         }
      }
   }   
}  

/*
 * Inflict damage from a hit.   This is one damn big function.
 */
ch_ret damage(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, int spec, int limb)
{
   char buf[MSL];
   char filename[256];
   OBJ_DATA *obj;
   sh_int dameq;
   sh_int maxdam;
   bool npcvict;
   bool loot;
   OBJ_DATA *damobj;
   ch_ret retcode;
   sh_int dampmod;
   CHAR_DATA *gch /*, *lch */ ;
   sh_int anopc = 0; /* # of (non-pkill) pc in a (ch) */
   sh_int bnopc = 0; /* # of (non-pkill) pc in b (victim) */
   sh_int proj = 0;
   int slimb = 0;
   
   if (spec == DM_SLICEDLIMB)
   {
      spec = DM_CRITICAL;
      slimb = 1;
   }

   if (dt == TYPE_PROJECTILE)
   {
      dt = TYPE_HIT;
      proj = 1;
   }


   retcode = rNONE;

   if (!ch)
   {
      bug("Damage: null ch!", 0);
      return rERROR;
   }
   if (!victim)
   {
      bug("Damage: null victim!", 0);
      return rVICT_DIED;
   }

   if (victim->position == POS_DEAD)
      return rVICT_DIED;

   npcvict = IS_NPC(victim);
   
   if (victim->position == POS_RIDING)
   {
      victim->position = POS_STANDING;
      if (victim->riding)
      {
         victim->riding->rider = NULL;
         victim->riding = NULL;
      }
   }
   if (victim->rider)
   {
      victim->rider->riding = NULL;
      victim->rider->position = POS_STANDING;
      victim->rider = NULL;
   }

   /*
    * Check damage types for RIS    -Thoric
    */
   /* 
    * New Weapon Overhall                             -Xerves
    */
   if (dam && dt != TYPE_UNDEFINED)
   {
      if (IS_FIRE(dt))
         dam = ris_damage(victim, dam, RIS_FIRE);
      else if (IS_WATER(dt))
         dam = ris_damage(victim, dam, RIS_WATER);
      else if (IS_EARTH(dt))
         dam = ris_damage(victim, dam, RIS_EARTH);
      else if (IS_ENERGY(dt))
         dam = ris_damage(victim, dam, RIS_ENERGY);
      else if (IS_HOLY(dt))
         dam = ris_damage(victim, dam, RIS_HOLY);
      else if (IS_UNHOLY(dt))
         dam = ris_damage(victim, dam, RIS_UNHOLY);
      else if (IS_UNDEAD(dt))
         dam = ris_damage(victim, dam, RIS_UNDEAD);
      else if (IS_AIR(dt))
         dam = ris_damage(victim, dam, RIS_AIR);
      else if (dt == gsn_poison)
         dam = ris_damage(victim, dam, RIS_POISON);
      else if (ch->grip == GRIP_BASH)
         dam = ris_damage(victim, dam, RIS_BLUNT);
      else if (ch->grip == GRIP_STAB)
         dam = ris_damage(victim, dam, RIS_PIERCE);
      else if (ch->grip == GRIP_SLASH)
         dam = ris_damage(victim, dam, RIS_SLASH);
      else if (skill_table[dt] && skill_table[dt]->type && skill_table[dt]->type == SKILL_SPELL)
         dam = ris_damage(victim, dam, RIS_MAGIC);
      else if (skill_table[dt] && (dt == gsn_manashot || dt == gsn_manaburst))
         dam = ris_damage(victim, dam, RIS_MAGIC);
      else
         dam = ris_damage(victim, dam, RIS_NONMAGIC);

      if (dam == -1)
      {
         if (dt >= 0 && dt < top_sn)
         {
            bool found = FALSE;
            SKILLTYPE *skill = skill_table[dt];

            if (skill->imm_char && skill->imm_char[0] != '\0')
            {
               act(AT_HIT, skill->imm_char, ch, NULL, victim, TO_CHAR);
               found = TRUE;
            }
            if (skill->imm_vict && skill->imm_vict[0] != '\0')
            {
               act(AT_HITME, skill->imm_vict, ch, NULL, victim, TO_VICT);
               found = TRUE;
            }
            if (skill->imm_room && skill->imm_room[0] != '\0')
            {
               act(AT_ACTION, skill->imm_room, ch, NULL, victim, TO_NOTVICT);
               found = TRUE;
            }
            if (found)
               return rNONE;
         }
         dam = 0;
         spec = DM_MISS;
      }
   }
   //Why was this not Here, ponder......Ha ha
   if (dt > TYPE_UNDEFINED && dt < TYPE_PROJECTILE)
   {
      if (victim->apply_manashell > 0 && victim->mana > 0 && skill_table[dt] && skill_table[dt]->type 
      &&  skill_table[dt]->type == SKILL_SPELL && dt != gsn_poison)
      {
         int mana;
         int nsn = skill_lookup("manashell");
         mana = UMAX(1, dam * 50 / victim->apply_manashell);
         if (nsn > 0 && LEARNED(victim, nsn) > 0)
            learn_from_success(victim, nsn, ch);
         if (mana <= victim->mana)
         {
            dam = 0;
            victim->mana -= mana;
            if (number_range(1, 10) == 1)
               gain_mana_per(victim, ch, mana);
            act(AT_WHITE, "$n's manashell flashes blue and absorbs the magical damage from $N", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "$n's manashell flashes blue and absorbs the magical damage from you", victim, NULL, ch, TO_VICT);
            act(AT_WHITE, "Your manashell flashes blue and absorbs the magical damage from $N", victim, NULL, ch, TO_CHAR);
            return rNONE;
         }
         else
         {
            dam -= dam * victim->mana / mana;   
            if (number_range(1, 10) == 1)
               gain_mana_per(victim, ch, mana);
            victim->mana = 0;
            act(AT_WHITE, "$n's manashell flashes red and partially absorbs the magical damage from $N", victim, NULL, ch, TO_NOTVICT);
            act(AT_WHITE, "$n's manashell flashes red and partially absorbs the magical damage from you", victim, NULL, ch, TO_VICT);
            act(AT_WHITE, "Your manashell flashes red and partially absorbs the magical damage from $N", victim, NULL, ch, TO_CHAR);
         }
      }
   }
   if ((dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) && in_same_room(ch, victim)) 
        if (check_dodge(ch, victim, limb))
            return rNONE;
   //Managuard 
   if (dam > 0 && victim->apply_managuard > 0 && spec != DM_MISS && spec != DM_BLOCK && ((dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) || dt == TYPE_UNDEFINED || dt == TYPE_PROJECTILE 
   || (skill_table[dt] && skill_table[dt]->type && skill_table[dt]->type == SKILL_SKILL)))
   {
      int mana;
      int nsn = skill_lookup("managuard");
      mana = UMAX(1, dam/2 * 50 / victim->apply_managuard);
      if (mana <= victim->mana)
      {
         dam /= 2;
         victim->mana -= mana;
         if (number_range(1, 10) == 1)
            gain_mana_per(victim, ch, mana);
         act(AT_CYAN, "$n's managuard flashes blue and absorbs half the physical damage from $N", victim, NULL, ch, TO_NOTVICT);
         act(AT_CYAN, "$n's managuard flashes blue and absorbs half the physical damage from you", victim, NULL, ch, TO_VICT);
         act(AT_CYAN, "Your managuard flashes blue and absorbs half the physical damage from $N", victim, NULL, ch, TO_CHAR);
         if (dam < 1)
            dam = 1;
      }
      else
      {
         dam -= dam/2 * victim->mana / mana;   
         if (number_range(1, 10) == 1)
            gain_mana_per(victim, ch, mana);
         victim->mana = 0;
         act(AT_CYAN, "$n's managuard flashes red and partially absorbs the physical damage from $N", victim, NULL, ch, TO_NOTVICT);
         act(AT_CYAN, "$n's managuard flashes red and partially absorbs the physical damage from you", victim, NULL, ch, TO_VICT);
         act(AT_CYAN, "Your managuard flashes red and partially absorbs the physical damage from $N", victim, NULL, ch, TO_CHAR);
      }
      if (nsn > 0 && LEARNED(victim, nsn) > 0)
         learn_from_success(victim, nsn, ch);
   }
   /*
    * Precautionary step mainly to prevent people in Hell from finding
    * a way out. --Shaddai
    */
   if (xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
   {
      dam = 0;
      spec = DM_MISS;
   }

   if (dam && npcvict && ch != victim)
   {
      if (!xIS_SET(victim->act, ACT_SENTINEL))
      {
         if (victim->hunting)
         {
            if (victim->hunting->who != ch)
            {
               STRFREE(victim->hunting->name);
               victim->hunting->name = QUICKLINK(ch->name);
               victim->hunting->who = ch;
            }
         }
         else if (!xIS_SET(victim->act, ACT_PACIFIST)) /* Gorog */
            start_hunting(victim, ch);
      }

      if (victim->hating)
      {
         if (victim->hating->who != ch)
         {
            STRFREE(victim->hating->name);
            victim->hating->name = QUICKLINK(ch->name);
            victim->hating->who = ch;
         }
      }
      else if (!xIS_SET(victim->act, ACT_PACIFIST)) /* Gorog */
         start_hating(victim, ch);
   }

   /*
    * Stop up any residual loopholes.
    */
   if (dt == gsn_backstab)
      maxdam = 300;
   else
      maxdam = 250;
   if (!IS_NPC(ch))
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_DAMCAP3))
         maxdam += 200;
      else if (xIS_SET(ch->pcdata->talent, TALENT_DAMCAP3))
         maxdam += 100;
      else
         maxdam += 50;
   }
   if (IS_NPC(ch))
      maxdam *= 2;
   if (dt > 0 && dt < top_sn && skill_table[dt] && skill_table[dt]->name && skill_table[dt]->type == SKILL_SPELL && !IS_NPC(ch))
      maxdam = maxdam * 5/4;
   if (dam > maxdam)
      dam = maxdam;
   if (dam > maxdam*2)
   {
      sprintf(buf, "Damage: %d more than %d points!", dam, maxdam);
      bug(buf, dam);
      sprintf(buf, "** %s (lvl %d) -> %s **", ch->name, ch->level, victim->name);
      bug(buf, 0);
   }
   if (IS_AFFECTED(victim, AFF_WEB) || IS_AFFECTED(victim, AFF_SNARE))
   {
      int bchance;
      int bdam = dam;
      
      if (dt > 0 && dt < top_sn && skill_table[dt] && skill_table[dt]->name && skill_table[dt]->type == SKILL_SPELL)
      {
         if (IS_FIRE(dt))
            bdam *= 4;
         else
            bdam *= 2;
      }      
      if (proj == 1)
         bdam *= 2;
      if (IS_AFFECTED(victim, AFF_SNARE))
         bdam /=2;
      if (!IS_NPC(victim))
         bdam*=2;
      bchance = URANGE(1, bdam/6, 80);
      if (number_range(1, 100) <= bdam)
      {
         act(AT_WHITE, "$n's last attack knocked the entanglement loose that was holding back $N.", ch, NULL, victim, TO_NOTVICT);
         act(AT_WHITE, "Your last attack knocked the entanglement loose that was holding back $N.", ch, NULL, victim, TO_CHAR);
         act(AT_WHITE, "%n's last attack knocked the entanglement loose that was holding you back!", ch, NULL, victim, TO_VICT);
         affect_strip(victim, gsn_web);
         affect_strip(victim, gsn_snare);
      }
   }        
   if (victim != ch)
   {
      /*
       * Certain attacks are forbidden.
       * Most other attacks are returned.
       */
      if (is_safe(ch, victim))
         return rNONE;
      check_attacker(ch, victim);
      /* Check for no wander which WAS not in the code -- Xerves 11/99 */
      if (xIS_SET(ch->act, ACT_NOWANDER))
      {
         xREMOVE_BIT(ch->act, ACT_NOWANDER);
         if (xIS_SET(ch->act, ACT_MOUNTABLE))
            xSET_BIT(ch->act, ACT_SCARED);
      }
       
      if (victim->position > POS_STUNNED)
      {
         if (!victim->fighting && victim->in_room == ch->in_room)
         {
            if (((dt >= 0 && dt < top_sn && skill_table[dt] && skill_table[dt]->type == SKILL_SPELL) ||  proj == 1) 
            && (!IS_AWAKE(victim) || IS_AFFECTED(victim, AFF_WEB) || IS_AFFECTED(victim, AFF_SNARE))) //Don't want to start a fight if it is magic or a projectile
               ;
            else
               set_fighting(victim, ch);
         }

         /*
            vwas: victim->position = POS_FIGHTING; 
          */
         if (IS_NPC(victim) && victim->fighting)
            victim->position = POS_FIGHTING;
         else if (victim->fighting)
         {
            switch (victim->style)
            {
               case (STYLE_EVASIVE): case (STYLE_DIVINE): case (STYLE_WIZARDRY):
                  victim->position = POS_EVASIVE;
                  break;
               case (STYLE_DEFENSIVE):
                  victim->position = POS_DEFENSIVE;
                  break;
               case (STYLE_AGGRESSIVE):
                  victim->position = POS_AGGRESSIVE;
                  break;
               case (STYLE_BERSERK):
                  victim->position = POS_BERSERK;
                  break;
               default:
                  victim->position = POS_FIGHTING;
            }

         }

      }

      if (victim->position > POS_STUNNED)
      {
         if (!ch->fighting)
         {
            if (((dt >= 0 && dt < top_sn && skill_table[dt] && skill_table[dt]->type == SKILL_SPELL) ||  proj == 1) 
            && (!IS_AWAKE(victim) || IS_AFFECTED(victim, AFF_WEB) || IS_AFFECTED(victim, AFF_SNARE))) //Don't want to start a fight if it is magic or a projectile
               ;
            else
               set_fighting(ch, victim);
         }

         /*
          * If victim is charmed, ch might attack victim's master.
          */
         if (IS_NPC(ch)
            && npcvict && IS_AFFECTED(victim, AFF_CHARM) && victim->master && victim->master->in_room == ch->in_room && number_bits(3) == 0)
         {
            stop_fighting(ch, FALSE);
            retcode = one_hit(ch, victim->master, TYPE_HIT, LM_BODY);
            return retcode;
         }
      }
      
      /*
       * More charm stuff.
       */
      if (victim->master == ch)
         stop_follower(victim);

      /*
       * Pkill stuff.  If a deadly attacks another deadly or is attacked by
       * one, then ungroup any nondealies.  Disabled untill I can figure out
       * the right way to do it.
       */

#ifdef UNUNSED
      /* count the # of non-pkill pc in a ( not including == ch ) */
// for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
//     if (is_same_group(ch, gch) && !IS_NPC(gch)
//  && !IS_PKILL(gch) && (ch != gch))
//  anopc++;

      /* count the # of non-pkill pc in b ( not including == victim ) */
// for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
//     if (is_same_group(victim, gch) && !IS_NPC(gch)
//  && !IS_PKILL(gch) && (victim != gch))
//  bnopc++;


      /* only consider disbanding if both groups have 1(+) non-pk pc */
      if ((bnopc > 0) && (anopc > 0))
      {
         /* look at group a through ch's leader first */
         lch = ch->leader ? ch->leader : ch;
         if (lch != ch)
         {
            /* stop following leader if it isn't pk */
            if (!IS_NPC(lch) && !IS_PKILL(lch))
               stop_follower(ch);
            else
            {
               /* disband non-pk members from lch's group if it is pk */
               for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
               {
                  if (is_same_group(lch, gch) && (lch != gch) && !IS_NPC(gch) && !IS_PKILL(gch))
                     stop_follower(gch);
               }
            }
         }
         else
            for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
            {
               /* ch is leader - disband non-pks from group */
               if (is_same_group(ch, gch) && (ch != gch) && (!IS_PKILL(gch) && !IS_NPC(gch)))
                  stop_follower(gch);
            }

         /* time to look at the victims group through its leader */
         lch = victim->leader ? victim->leader : victim;

         if (lch != victim)
         {
            /* if leader isn't deadly, stop following lch */
            if (!IS_PKILL(lch) && !IS_NPC(lch))
               stop_follower(victim);
            else
            {
               /* lch is pk, disband non-pk's from group */
               for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
               {
                  if (is_same_group(lch, gch) && (lch != gch) && (!IS_PKILL(gch) && !IS_NPC(gch)))
                     stop_follower(gch);
               }
            }
         }
         else
         {
            /* victim is leader of group - disband non-pks */
            for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
            {
               if (is_same_group(victim, gch) && (victim != gch) && !IS_PKILL(gch) && !IS_NPC(gch))
                  stop_follower(gch);
            }
         }
      }
#endif

      /*
         count the # of non-pkill pc in a ( not including == ch ) 
       */
// for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
//     if (is_same_group(ch, gch) && !IS_NPC(gch)
//  && !IS_PKILL(gch) && (ch != gch))
//  anopc++;

      /*
         count the # of non-pkill pc in b ( not including == victim ) 
       */
// for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
//     if (is_same_group(victim, gch) && !IS_NPC(gch)
//  && !IS_PKILL(gch) && (victim != gch))
//  bnopc++;


      /*
         only consider disbanding if both groups have 1(+) non-pk pc,
         or when one participant is pc, and the other group has 1(+)
         pk pc's (in the case that participant is only pk pc in group) 
       */
      if ((bnopc > 0 && anopc > 0) || (bnopc > 0 && !IS_NPC(ch)) || (anopc > 0 && !IS_NPC(victim)))
      {
         /*
            Disband from same group first 
          */
         if (is_same_group(ch, victim))
         {
            /*
               Messages to char and master handled in stop_follower
             */
            act(AT_ACTION, "$n disbands from $N's group.",
               (ch->leader == victim) ? victim : ch, NULL, (ch->leader == victim) ? victim->master : ch->master, TO_NOTVICT);
            if (ch->leader == victim)
               stop_follower(victim);
            else
               stop_follower(ch);
         }
         /*
            if leader isnt pkill, leave the group and disband ch 
          */
         if (ch->leader != NULL && !IS_NPC(ch->leader) && !IS_PKILL(ch->leader))
         {
            act(AT_ACTION, "$n disbands from $N's group.", ch, NULL, ch->master, TO_NOTVICT);
            stop_follower(ch);
         }
         else
         {
            for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
               if (is_same_group(gch, ch) && !IS_NPC(gch) && !IS_PKILL(gch) && gch != ch)
               {
                  act(AT_ACTION, "$n disbands from $N's group.", ch, NULL, gch->master, TO_NOTVICT);
                  stop_follower(gch);
               }
         }
         /*
            if leader isnt pkill, leave the group and disband victim 
          */
         if (victim->leader != NULL && !IS_NPC(victim->leader) && !IS_PKILL(victim->leader))
         {
            act(AT_ACTION, "$n disbands from $N's group.", victim, NULL, victim->master, TO_NOTVICT);
            stop_follower(victim);
         }
         else
         {
            for (gch = victim->in_room->first_person; gch; gch = gch->next_in_room)
               if (is_same_group(gch, victim) && !IS_NPC(gch) && !IS_PKILL(gch) && gch != victim)
               {
                  act(AT_ACTION, "$n disbands from $N's group.", gch, NULL, gch->master, TO_NOTVICT);
                  stop_follower(gch);
               }
         }
      }

/*
	for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	{
	    if ( is_same_group( ch, gch )
	    && ( IS_PKILL( ch ) != IS_PKILL( gch ) ) )
	    {
		stop_follower( ch );
		stop_follower( gch );
	    }
	}

	for ( gch = victim->in_room->first_person; gch; gch = gch->next_in_room )
	{
	    if ( is_same_group( victim, gch ) 
	    && ( IS_PKILL( victim ) != IS_PKILL( gch ) ) )
	    {
		stop_follower( victim ); 
		stop_follower( gch );
	    }
	}
 */

      /*
       * Inviso attacks ... not.
       */
      if (IS_AFFECTED(ch, AFF_INVISIBLE))
      {
         affect_strip(ch, gsn_invis);
         affect_strip(ch, gsn_mass_invis);
         xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
         act(AT_MAGIC, "$n fades into existence.", ch, NULL, NULL, TO_ROOM);
      }

      /* Take away Hide */
      if (IS_AFFECTED(ch, AFF_HIDE))
      {
         xREMOVE_BIT(ch->affected_by, AFF_HIDE);
         act(AT_RED, "$n quickly appears from the shadows.", ch, NULL, NULL, TO_ROOM);
      }
      /* Take away STALK */
      if (IS_AFFECTED(ch, AFF_STALK))
      {
         xREMOVE_BIT(ch->affected_by, AFF_STALK);
         affect_strip(ch, gsn_stalk);
         act(AT_RED, "$n quickly appears from the shadows.", ch, NULL, NULL, TO_ROOM);
      }         
      if (dt > 0 && dt < top_sn && skill_table[dt] && skill_table[dt]->name && skill_table[dt]->type == SKILL_SPELL && ch->race == RACE_FAIRY)
         dam = dam * 135 / 100;
         
      if (dt > 0 && dt < top_sn && skill_table[dt] && skill_table[dt]->name && skill_table[dt]->type == SKILL_SPELL && ch->race == RACE_OGRE)
         dam = dam * 50 / 100;        
         
      if ((dt < TYPE_HIT && dt != gsn_backstab && dt != gsn_circle && dt != gsn_kick_back) || proj)
         ;
      else
      {
         if(IS_NPC(victim) && xIS_SET(victim->act, ACT_UNDEAD))
         {
            obj = get_eq_char(ch, WEAR_WIELD);
            if(!obj)
            {
              dam = 0;
              spec = DM_UNDEAD;
            }
           
            if(obj)
            {  
               if(!xIS_SET(obj->extra_flags, ITEM_BLESS) && !xIS_SET(obj->extra_flags, ITEM_SANCTIFIED))
               {
                   dam = 0;
                   spec = DM_UNDEAD;
               }
               if(xIS_SET(obj->extra_flags, ITEM_BLESS) && obj->bless_dur > 0)
               {
                   if(dam > 0)
                   {
                       obj->bless_dur -= 1;
                   }
               }
            
               if(xIS_SET(obj->extra_flags, ITEM_BLESS) && obj->bless_dur <= 0)
               {
                   xTOGGLE_BIT(obj->extra_flags, ITEM_BLESS);
                   act(AT_MAGIC, "Your $p stops glowing...\n\r", ch, obj, NULL, TO_CHAR);
               }
            }
         }
         if(IS_NPC(victim) && xIS_SET(victim->act, ACT_LIVING_DEAD))
         {
            obj = get_eq_char(ch, WEAR_WIELD);
            if(!obj)
            {
               dam = dam / 2;
            }
          
            if(obj)
            {
               if(!xIS_SET(obj->extra_flags, ITEM_BLESS) && !xIS_SET(obj->extra_flags, ITEM_SANCTIFIED))
               {
                  dam = dam / 2;
               }
          
               if(xIS_SET(obj->extra_flags, ITEM_BLESS) || xIS_SET(obj->extra_flags, ITEM_SANCTIFIED))
               {
                  dam = dam * 2;
               }
          
          
               if(xIS_SET(obj->extra_flags, ITEM_BLESS) && obj->bless_dur == 0)
               {
                  xTOGGLE_BIT(obj->extra_flags, ITEM_BLESS);
                  act(AT_MAGIC, "Your $p stops glowing...\n\r", ch, obj, NULL, TO_CHAR);
               }
               if(xIS_SET(obj->extra_flags, ITEM_BLESS) && obj->bless_dur > 0)
               {
                  if(dam > 0)
                  {
                     obj->bless_dur -= 1;
                  }
               }
            }
         }
      }
      /*
       * Check for disarm, trip, parry, and dodge.
       */
      if ((dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) && in_same_room(ch, victim))
      {
         int pvalue = 0;

         if (IS_NPC(ch) && xIS_SET(ch->defenses, DFND_DISARM) && number_percent() < 12) /* Was 2 try this --Shaddai */
            disarm(ch, victim);

         if (IS_NPC(ch) && xIS_SET(ch->attacks, ATCK_TRIP) && number_percent() <= 15)
            trip(ch, victim);

         if ((!IS_NPC(victim) && xIS_SET(victim->act, PLR_PARRY)) || IS_NPC(victim))
         {
            pvalue = check_parry(ch, victim);     
            if (pvalue > 0)
            {
               OBJ_DATA *dweapon = get_eq_char(victim, WEAR_WIELD);
               if (dweapon)
               {
                  if (number_range(1, 100) > (40 + (get_obj_resistance(dweapon, victim) * 3)))
                  {
                     set_cur_obj(dweapon);   
                     damage_obj(dweapon, victim, proj, dam);
                  }
               }
            }    
            if (pvalue == TRUE)
               return rNONE;
            if (pvalue > 1)
            {
               dam = dam * pvalue / 100;
               dam = UMAX(1, dam);
            }
         }
      }
      /*
       * Check control panel settings and modify damage
       */
      if (IS_NPC(ch))
      {
         if (npcvict)
            dampmod = sysdata.dam_mob_vs_mob;
         else
            dampmod = sysdata.dam_mob_vs_plr;
      }
      else
      {
         if (npcvict)
            dampmod = sysdata.dam_plr_vs_mob;
         else
            dampmod = sysdata.dam_plr_vs_plr;
      }
      if (dampmod > 0)
         dam = (dam * dampmod) / 100;
         
      if (dam > 0 && spec != DM_UNDEAD && spec != DM_BLOCK)
      {
         if (spec == DM_MISS)
         {
            if (ch->grip == GRIP_BASH)
               dam/=2;
            if (ch->grip == GRIP_SLASH)
               dam/=4;
            if (ch->grip == GRIP_STAB)
               dam/=8;
         }
         check_limb_status(victim, limb, dam, slimb, ch, spec);
      }
         
      if (spec == DM_UNDEAD || spec == DM_MISS || spec == DM_BLOCK)
         dam = 0;
      else
      {
         if (dam < 1)
            dam = 1;
      }
      adjust_aggression_list(victim, ch, dam, 0, -1);
      if (IS_NPC(ch) && !IS_NPC(victim))
      {
         int per = 0;
         int level;
         
         if (LEARNED(victim, gsn_greater_focus_aggression) > 0)
         {
            level = POINT_LEVEL(LEARNED(victim, gsn_greater_focus_aggression), MASTERED(victim, gsn_greater_focus_aggression));
            per = 35+level/4;
            learn_from_success(victim, gsn_greater_focus_aggression, ch);
         }
         else if (LEARNED(victim, gsn_focus_aggression) > 0)
         {
            level = POINT_LEVEL(LEARNED(victim, gsn_focus_aggression), MASTERED(victim, gsn_focus_aggression));
            per = 20+level/5;
            learn_from_success(victim, gsn_focus_aggression, ch);
         }
         else if (LEARNED(victim, gsn_greater_draw_aggression) > 0)
         {
            level = POINT_LEVEL(LEARNED(victim, gsn_greater_draw_aggression), MASTERED(victim, gsn_greater_draw_aggression));
            per = 10+level/7;
            learn_from_success(victim, gsn_greater_draw_aggression, ch);
         }
         else if (LEARNED(victim, gsn_draw_aggression) > 0)
         {
            level = POINT_LEVEL(LEARNED(victim, gsn_draw_aggression), MASTERED(victim, gsn_draw_aggression));
            per = 2+level/10;
            learn_from_success(victim, gsn_draw_aggression, ch);
         }
         if (per > 0)
         {
            per = UMAX(1, dam * per /100);
            adjust_aggression_list(ch, victim, per, 4, -1);   
         }
      }
      if (proj == 1)
         new_dam_message(ch, victim, dam, TYPE_PROJECTILE, NULL, spec, limb);
      else
         new_dam_message(ch, victim, dam, dt, NULL, spec, limb);
   }
          
   /*
    * Makes since that if it does not penetrate it might damage armor, 
    * will also make newbie armor last longer
    */
   //Insert limb code here :-)
   if ((dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) && limb > -1 && (spec == DM_HIT || spec == DM_CRITICAL || spec == DM_SLICEDLIMB))
   {
      /* damage eq */
      dameq = get_limb_location(limb);
      damobj = get_eq_char(victim, dameq);
      if (damobj)
      {
         if (number_range(1, 100) > (50 + (get_obj_resistance(damobj, victim) * 5/2)))
         {
            set_cur_obj(damobj);
            damage_obj(damobj, victim, proj, dam);
         }
      }
   }
   damobj = NULL;
   // Weapon deteriates over time...
   if ((dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) && (spec == DM_HIT || spec == DM_CRITICAL || spec == DM_SLICEDLIMB))
   {
      damobj = get_eq_char(ch, WEAR_WIELD);
      if (damobj)
      {
         if (number_range(1, 100) > (40 + (get_obj_resistance(damobj, ch) * 3)))
         {
            set_cur_obj(damobj);   
            damage_obj(damobj, ch, proj, dam);
         }
      }
   } 
   if ((dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) && spec == DM_BLOCK)
   {
      damobj = get_eq_char(victim, WEAR_SHIELD);
      if (damobj)
      {
         set_cur_obj(damobj);
         damage_obj(damobj, ch, proj, blockdam);
      }
   }
   dam = dam + adjust_mil_damage(ch, dam);
   /*
    * Hurt the victim.
    * Inform the victim of his new state.
    */
   victim->hit -= dam;
   
   if (victim->apply_manafuse > 0 && dam > 0)
   {
      victim->mana += UMAX(1, dam * UMIN(40, victim->apply_manafuse) / 100); 
      if (victim->mana > victim->max_mana)
         victim->mana = victim->max_mana;
      if (LEARNED(victim, gsn_manafuse) >= 1)
         learn_from_success(victim, gsn_manafuse, ch);
   }
   
   //Health additions
   if (!IS_NPC(victim) && dam > 0)
   {
      int mhp = 1;
      int hpi = 1;
      
      victim->pcdata->hit_cnt+=number_range(dam*6/10, dam*8/10);
      
      if (sysdata.stat_gain <= 1)
         mhp = number_range(dam*80/100, dam*105/100);
      else if (sysdata.stat_gain <= 3)
         mhp = number_range(dam*110/100, dam*135/100);
      else if (sysdata.stat_gain >= 5)
         mhp = number_range(dam*150/100, dam*200/100);
      
      if (victim->race == RACE_OGRE) //Just a slight bonus
      mhp = mhp * 125 / 100;
      
      //500 is map hp      
      if (victim->max_hit < 30)
         mhp *= 5;
      else if (victim->max_hit < 40)
         mhp *= 4.5;
      else if (victim->max_hit < 50)
         mhp *= 4;
      else if (victim->max_hit < 60)
         mhp *= 3;
      else if (victim->max_hit < 70)
         mhp *= 2.5;
      else if (victim->max_hit < 80)
         mhp *= 2;
      else if (victim->max_hit < 90)
         mhp *= 1.5;
      else if (victim->max_hit < 125)
         mhp *= 1.1;
      else if (victim->max_hit < 150)
         mhp *= .95;
      else if (victim->max_hit < 175)
         mhp *= .9;
      else if (victim->max_hit < 200)
         mhp *= .8;
      else if (victim->max_hit < 300)
         mhp *= .7;
      else if (victim->max_hit < 400)
         mhp *= .6;
      else if (victim->max_hit < 600)
         mhp *= .5;
      else if (victim->max_hit < 700)
         mhp *= .4;
      else if (victim->max_hit < 1000)
         mhp *= .35;
      else if (victim->max_hit < 1500)
         mhp *= .3;
      else
         mhp *= .25;
      
      if (victim->perm_con < 11)
         mhp *= .8;
      if (victim->perm_con >= 11 && victim->perm_con <= 13)
         mhp *= .9;
      if (victim->perm_con >= 14 && victim->perm_con <= 16)  
         mhp *= 1.05;
      if (victim->perm_con >= 17 && victim->perm_con <= 19)
         mhp *= 1.15;
      if (victim->perm_con >= 20 && victim->perm_con <= 22)
         mhp *= 1.3;
      if (victim->perm_con >= 23 && victim->perm_con < 25)
         mhp *= 1.5;
      if (victim->perm_con == 25) //25 is max
         mhp *= 2;
         
      if (victim->pcdata->hit_cnt >= get_sore_rate(victim->race, victim->max_hit))
      {
         send_to_char("&rYou are starting to develop sores where you are being hit, might want to rest.\n\r", victim);
         victim->hit -= 1;
         mhp = number_range(-3, -8);
      }
         
      if (mhp == 0)
         mhp = 1;   
      
      if (victim->race == RACE_OGRE) //Just a slight bonus
      {
          if (mhp == 1)
             mhp = 2;
      }
      
      if (mhp > 0 && victim->max_hit >= (700+get_talent_increase(victim, 9)) && victim->pcdata->per_hp >= 300)
         mhp = 0;
         
      if (mhp < 0 && victim->max_hit <= 20)
         mhp = 0;
         
      if (victim == ch)
         mhp = 0;
         
      if (victim && IS_NPC(victim) && xIS_SET(victim->act, ACT_MOUNTSAVE))
      {
         if (mhp > 0)
            mhp = 0;
      } 
      
      victim->pcdata->per_hp += mhp;
         
      if (victim->pcdata->per_hp > 1000)
      {
         if (sysdata.stat_gain <= 1)
         {
            if (victim->max_hit < 70)
               hpi = number_range(5, 7);
            else if (victim->max_hit < 100)
               hpi = number_range(4, 6);
            else if (victim->max_hit < 150)
               hpi = number_range(2, 4);
            else if (victim->max_hit < 200)
               hpi = number_range(1, 3);
            else
               hpi = 1;
         }
         else if (sysdata.stat_gain <= 2)
         {
            if (victim->max_hit < 70)
               hpi = number_range(5, 7);
            else if (victim->max_hit < 100)
               hpi = number_range(4, 6);
            else if (victim->max_hit < 200)
               hpi = number_range(3, 5);
            else if (victim->max_hit < 300)
               hpi = number_range(2, 4);
            else
               hpi = number_range(1, 2);
         }
         else if (sysdata.stat_gain >= 4)
         {
            if (victim->max_hit < 150)
               hpi = number_range(5, 7);
            else if (victim->max_hit < 300)
               hpi = number_range(4, 6);
            else if (victim->max_hit < 600)
               hpi = number_range(3, 5);
            else if (victim->max_hit < 900)
               hpi = number_range(2, 4);
            else
               hpi = number_range(1, 2);
         }
            
         victim->max_hit += hpi;
         send_to_char("&R**************************************\n\r", victim);
         ch_printf(victim, "&R******You Gain %d Point of Health******\n\r", hpi);
         send_to_char("&R**************************************\n\r", victim);
         victim->pcdata->per_hp = 0;
      }  
      if (victim->pcdata->per_hp < 0)
      {
         if (victim->max_hit < 50)
            hpi = number_range(5, 7);
         else if (victim->max_hit < 75)
            hpi = number_range(4, 6);
         else if (victim->max_hit < 100)
            hpi = number_range(2, 4);
         else if (victim->max_hit < 150)
            hpi = number_range(1, 3);
         else
            hpi = 1;
            
         victim->max_hit -= hpi;
         send_to_char("&r**************************************\n\r", victim);
         ch_printf(victim, "&R******You Lose %d Point of Health******\n\r", hpi);
         send_to_char("&r**************************************\n\r", victim);
         victim->pcdata->per_hp = 999;
      }     
   }
      
   if (victim->hit < victim->max_hit /2 && !IS_NPC(victim) && dam > 0)
   {
      int mcon, bcon;
      
      mcon = number_range(15, 30);
      
      if (sysdata.stat_gain <= 1)
         mcon = number_range(15, 30);
      else if (sysdata.stat_gain <= 3)
         mcon = number_range(35, 50);
      else if (sysdata.stat_gain >= 5)
         mcon = number_range(60, 80);
         
      bcon = 14 + race_table[victim->race]->con_plus;
      if (victim->perm_con == bcon - 4)
         mcon *= 2;
      if (victim->perm_con == bcon - 3)
         mcon *= 1.7;
      if (victim->perm_con == bcon - 2)
         mcon *= 1.5;
      if (victim->perm_con == bcon - 1)
         mcon *= 1.2;
      if (victim->perm_con == bcon)
         mcon *= 1;
      if (victim->perm_con == bcon + 1)
         mcon *= .85;
      if (victim->perm_con == bcon + 2)
         mcon *= .7;
      if (victim->perm_con == bcon + 3)
         mcon *= .6;
      if (victim->perm_con == bcon + 4)
         mcon *= .4;
      if (victim->perm_con == bcon + 5)
         mcon *= .3;
      if (victim->perm_con == bcon + 6)
         mcon *= .275;
      if (victim->perm_con == bcon + 7)
         mcon *= .25;
      if (victim->perm_con == bcon + 8)
         mcon *= .225;
      if (victim->perm_con > bcon + 8) //Base + 8 should be the max unless you screwed it up
         mcon = 0;
      else
      {
         if (mcon == 0)
            mcon = 1;
      }
         
      if (victim->hit < victim->max_hit * 3/10)
         mcon *= 1.2;
      else if (victim->hit < victim->max_hit * 15/100)
         mcon *= 1.35;
      else if (victim->hit < victim->max_hit * 5/100)
         mcon *= 1.5;
         
      if (victim->hit < 0)
      {
         mcon = number_range(-300, -500);
         victim->pcdata->lostcon += mcon;
      }
         
         
      if (mcon == 0)
         mcon = 1;    
         
      if (victim->perm_con >= (14 + race_table[victim->race]->con_plus + race_table[victim->race]->con_range + get_talent_increase(victim, 6)) &&
          mcon > 0 && victim->pcdata->per_con >= 3000)
         mcon = 0;
      if (victim->perm_con <= (14 + race_table[victim->race]->con_plus - 5 + race_table[victim->race]->con_range) &&
          mcon < 0)
         mcon = 0;
         
      if (ch && IS_NPC(ch) && xIS_SET(ch->act, ACT_MOUNTSAVE))
      {
         if (mcon > 0)
            mcon = 0;
      } 
      
      if (victim == ch)
         mcon = 0;
         
      victim->pcdata->per_con += mcon;
      if (victim->pcdata->per_con > 10000)
      {
         victim->perm_con++;
         send_to_char("&R******************************************\n\r", victim);
         send_to_char("&R*****You Gain 1 Point of Constitution*****\n\r", victim);
         send_to_char("&R******************************************\n\r", victim);
         victim->pcdata->per_con = 0;
      }
      if (victim->pcdata->per_con < 0)
      {
         victim->perm_con--;
         send_to_char("&r******************************************\n\r", victim);
         send_to_char("&r*****You Lose 1 Point of Constitution*****\n\r", victim);
         send_to_char("&r******************************************\n\r", victim);
         victim->pcdata->per_con = 9999;
      }
   }
   
   if (spec == DM_DEATH)
      victim->hit = -11;
   if (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL && victim->hit < 1)
      victim->hit = 1;

   /* Make sure newbies dont die */

   if (!IS_NPC(victim) && NOT_AUTHED(victim) && victim->hit < 1)
      victim->hit = 1;

   if (dam > 0 && (dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back)
      && !IS_AFFECTED(victim, AFF_POISON)
      && is_wielding_poisoned(ch) && !is_immune(victim, -1, RIS_POISON) && !saves_poison_death(ch->level, victim))
   {
      AFFECT_DATA af;

      af.type = gsn_poison;
      af.duration = 20;
      af.location = APPLY_STR;
      af.modifier = -1;
      af.bitvector = meb(AFF_POISON);
      affect_join(victim, &af);
      victim->mental_state = URANGE(20, (victim->mental_state + 2), 100);
   }

   /*
    * Vampire self preservation    -Thoric
    */
   if (IS_VAMPIRE(victim))
   {
      if (dam >= (victim->max_hit / 10)) /* get hit hard, lose blood */
         gain_condition(victim, COND_BLOODTHIRST, -1);
      if (victim->hit <= (victim->max_hit / 8) && victim->pcdata->condition[COND_BLOODTHIRST] > 5)
      {
         gain_condition(victim, COND_BLOODTHIRST, -number_range(2, 6));
         victim->hit += URANGE(4, (victim->max_hit / 30), 15);
         set_char_color(AT_BLOOD, victim);
         send_to_char("You howl with rage as the beast within stirs!\n\r", victim);
      }
   }

   if (!npcvict && get_trust(victim) >= LEVEL_IMMORTAL && get_trust(ch) >= LEVEL_IMMORTAL && victim->hit < 1)
      victim->hit = 1;
      
   update_pos(victim);

   switch (victim->position)
   {
      case POS_MORTAL:
         act(AT_DYING, "$n is mortally wounded, and will die soon, if not aided.", victim, NULL, NULL, TO_ROOM);
         act(AT_DANGER, "You are mortally wounded, and will die soon, if not aided.^x", victim, NULL, NULL, TO_CHAR);
         break;

      case POS_INCAP:
         act(AT_DYING, "$n is incapacitated and will slowly die, if not aided.", victim, NULL, NULL, TO_ROOM);
         act(AT_DANGER, "You are incapacitated and will slowly die, if not aided.^x", victim, NULL, NULL, TO_CHAR);
         break;

      case POS_STUNNED:
         if (!IS_AFFECTED(victim, AFF_PARALYSIS))
         {
            act(AT_ACTION, "$n is stunned, but will probably recover.", victim, NULL, NULL, TO_ROOM);
            act(AT_HURT, "You are stunned, but will probably recover.", victim, NULL, NULL, TO_CHAR);
         }
         break;

      case POS_DEAD:
         if (dt >= 0 && dt < top_sn)
         {
            SKILLTYPE *skill = skill_table[dt];

            if (skill->die_char && skill->die_char[0] != '\0')
               act(AT_DEAD, skill->die_char, ch, NULL, victim, TO_CHAR);
            if (skill->die_vict && skill->die_vict[0] != '\0')
               act(AT_DEAD, skill->die_vict, ch, NULL, victim, TO_VICT);
            if (skill->die_room && skill->die_room[0] != '\0')
               act(AT_DEAD, skill->die_room, ch, NULL, victim, TO_NOTVICT);
         }
         act(AT_DEAD, "$n is DEAD!!", victim, 0, 0, TO_ROOM);
         act(AT_DEAD, "You have been KILLED!!\n\r", victim, 0, 0, TO_CHAR);
         break;

      default:
         if (dam > victim->max_hit / 4)
         {
            act(AT_HURT, "That really did HURT!", victim, 0, 0, TO_CHAR);
            if (number_bits(3) == 0)
               worsen_mental_state(victim, 1);
         }
         if (victim->hit < victim->max_hit / 4)

         {
            act(AT_DANGER, "You wish that your wounds would stop BLEEDING so much!^x", victim, 0, 0, TO_CHAR);
            if (number_bits(2) == 0)
               worsen_mental_state(victim, 1);
         }
         break;
   }

   /*
    * Sleep spells and extremely wounded folks.
    */
   //Change this so no-aggro mobs will sometimes finish a player off and nomercy flagged mobs will do it 100 percent (same as aggressive
   //for area mobs)
   if (!IS_AWAKE(victim) /* lets make NPC's not slaughter PC's */
      && !IS_AFFECTED(victim, AFF_PARALYSIS) && !xIS_SET(ch->act, ACT_NOMERCY) && number_range(1,100) <= 85 )
   {
      if (victim->fighting && victim->fighting->who->hunting && victim->fighting->who->hunting->who == victim)
         stop_hunting(victim->fighting->who);

      if (victim->fighting && victim->fighting->who->hating && victim->fighting->who->hating->who == victim)
         stop_hating(victim->fighting->who);

      if (!npcvict && IS_NPC(ch))
         stop_fighting(victim, TRUE);
      else
         stop_fighting(victim, FALSE);
   }

   /*
    * Payoff for killing things.
    */
   if (victim->position == POS_DEAD)
   {
      group_gain(ch, victim);
      spoint_gain(ch, victim);

      if (!npcvict)
      {
         sprintf(log_buf, "%s killed by %s at %d",
            victim->name, (IS_NPC(ch) ? ch->short_descr : ch->name), victim->in_room->vnum);
         log_string(log_buf);
         to_channel(log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);

         if (!IS_NPC(ch) && !IS_IMMORTAL(ch) && ch->pcdata->clan
            && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD && victim != ch)
         {
            sprintf(filename, "%s%s.record", CLAN_DIR, ch->pcdata->clan->name);
            sprintf(log_buf, "&P%-12s &wvs &P%s ... &w%s",
               ch->name, victim->name, ch->in_room->area->name);
            if (victim->pcdata->clan && victim->pcdata->clan->name == ch->pcdata->clan->name)
               ;
            else
               append_to_file(filename, log_buf);
         }
      }
      else if (!IS_NPC(ch)) /* keep track of mob vnum killed */
         add_kill(ch, victim);

      if (!IS_NPC(ch) && !IS_NPC(victim))
      {
         add_pkill(ch, victim); //Keep track of player killings for pranking
         update_pranking(ch, victim); //Updating the pranking
      }
      check_killer(ch, victim);

      if ((IN_WILDERNESS(ch) && ch->coord->x == victim->coord->x && ch->coord->y == victim->coord->y
            && ch->map == victim->map) 
            || (!IN_WILDERNESS(ch) && ch->in_room == victim->in_room))
         loot = legal_loot(ch, victim);
      else
         loot = FALSE;

      set_cur_char(victim);
      raw_kill(ch, victim);
      victim = NULL;

      if (!IS_NPC(ch) && loot)
      {
         /* Autogold by Scryn 8/12 */
         if (xIS_SET(ch->act, PLR_AUTOGOLD))
         {
            do_get(ch, "coins corpse");
         }
         if (xIS_SET(ch->act, PLR_AUTOLOOT) && victim != ch) /* prevent nasty obj problems -- Blodkai */
            do_get(ch, "all corpse");
         else
            do_look(ch, "in corpse");
         

         if (xIS_SET(ch->act, PLR_AUTOSAC))
            do_sacrifice(ch, "corpse");
      }

      if (IS_SET(sysdata.save_flags, SV_KILL))
         save_char_obj(ch);
      return rVICT_DIED;
   }

   if (victim == ch)
      return rNONE;

   /*
    * Take care of link dead people.
    */
   if (!npcvict && !victim->desc && !IS_SET(victim->pcdata->flags, PCFLAG_NORECALL))
   {
      if (number_range(0, victim->wait) == 0)
      {
         do_recall(victim, "");
         return rNONE;
      }
   }
   if (victim->position >= POS_BERSERK && victim->apply_weaponclamp > 0 && get_eq_char(ch, WEAR_WIELD) && ch != victim)
   {
      int chance;
      int sn = skill_lookup("weaponclamp");
      chance = URANGE(1, victim->apply_weaponclamp/10, 10);
      if (get_eq_char(victim, WEAR_DUAL_WIELD) == NULL)
         chance = UMAX(1, chance/2);
      if (number_range(1, 100) <= chance)
      {
         act(AT_WHITE,"$n quickly attempts a weaponclamp against $N after $N's attack.", victim, NULL, ch, TO_NOTVICT);
         act(AT_WHITE,"You quickly attempt a weaponclamp against $N after $N's attack.", victim, NULL, ch, TO_CHAR);
         act(AT_WHITE,"$n quickly attempts a weaponclamp against you after your attack.", victim, NULL, ch, TO_VICT);
         if (sn > 0 && LEARNED(victim, sn) > 0)
            learn_from_success(victim, sn, ch);
         disarm(victim, ch);
      }
   }
   /*
    * Wimp out?
    */
   if (npcvict && dam > 0)
   {
      if ((xIS_SET(victim->act, ACT_WIMPY) && number_bits(1) == 0 && victim->fight_timer == 0
            && victim->hit < victim->max_hit / 2) || (IS_AFFECTED(victim, AFF_CHARM) && victim->master && victim->master->in_room != victim->in_room))
      {
         start_fearing(victim, ch);
         stop_hunting(victim);
         do_flee(victim, "");
      }
   }

   if (!npcvict && victim->hit > 0 && victim->hit <= victim->wimpy && victim->wait == 0 && victim->fight_timer == 0)
      do_flee(victim, "");
   else if (!npcvict && xIS_SET(victim->act, PLR_FLEE))
      do_flee(victim, "");
      
   if (ch && victim && dam)
   {
      OBJ_DATA *wield;
      /*
       * Weapon spell support    -Thoric
       * Each successful hit casts a spell
       */
      if ((dt == TYPE_HIT ||dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) && ((wield=get_eq_char(ch, WEAR_WIELD)) != NULL) && !is_immune(victim, -1, RIS_MAGIC) && !wIS_SET(victim, ROOM_NO_MAGIC) 
      &&  !xIS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC))
      {
         AFFECT_DATA *aff;

         for (aff = wield->pIndexData->first_affect; aff; aff = aff->next)
            if (aff->location == APPLY_WEAPONSPELL && IS_VALID_SN(aff->modifier) && skill_table[aff->modifier]->spell_fun)
               retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (wield->level + 3) / 3, ch, victim);
         if (retcode != rNONE || char_died(ch) || char_died(victim))
            return retcode;
         for (aff = wield->first_affect; aff; aff = aff->next)
            if (aff->location == APPLY_WEAPONSPELL && IS_VALID_SN(aff->modifier) && skill_table[aff->modifier]->spell_fun)
               retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, (wield->level + 3) / 3, ch, victim);
         if (retcode != rNONE || char_died(ch) || char_died(victim))
            return retcode;
      }

      /*
       * magic shields that retaliate    -Thoric
       */
      if (dt != skill_lookup("flare") && dt != skill_lookup("iceshard") && dt != skill_lookup("torrent")
      &&  dt != skill_lookup("manatorrent"))
      {
         if (IS_AFFECTED(victim, AFF_FIRESHIELD) && !IS_AFFECTED(ch, AFF_FIRESHIELD))
            retcode = spell_smaug(skill_lookup("flare"), off_shld_lvl(victim), victim, ch);
         if (retcode != rNONE || char_died(ch) || char_died(victim))
            return retcode;

         if (IS_AFFECTED(victim, AFF_ICESHIELD) && !IS_AFFECTED(ch, AFF_ICESHIELD))
            retcode = spell_smaug(skill_lookup("iceshard"), off_shld_lvl(victim), victim, ch);
         if (retcode != rNONE || char_died(ch) || char_died(victim))
            return retcode;

         if (IS_AFFECTED(victim, AFF_SHOCKSHIELD) && !IS_AFFECTED(ch, AFF_SHOCKSHIELD))
            retcode = spell_smaug(skill_lookup("torrent"), off_shld_lvl(victim), victim, ch);
         if (retcode != rNONE || char_died(ch) || char_died(victim))
            return retcode;
      
         if (victim->apply_manashield > 0)
         {
            int nsn = skill_lookup("manashield");
            retcode = spell_smaug(skill_lookup("manatorrent"), victim->apply_manashield, victim, ch);
            if (retcode != rNONE || char_died(ch) || char_died(victim))
               return retcode;
            if (nsn > 0 && LEARNED(victim, nsn) > 0)
               learn_from_success(victim, nsn, ch);
         }
      }
   }
   tail_chain();
   return rNONE;
}

//just in case later if some more things need to be checked to determine room safe
bool is_room_safe(CHAR_DATA * ch)
{
   if (ch->in_room && xIS_SET(ch->in_room->room_flags, ROOM_SAFE))
      return TRUE;

   return FALSE;
}

const char *cpstatus[MAX_PEACEVALUE] = {
   "WAR", "Neutral", "Trading", "Peace"
};

bool is_safe(CHAR_DATA * ch, CHAR_DATA * victim)
{
   /* Thx Josh! */
   if (who_fighting(ch) == ch)
      return FALSE;

   if (!victim) /*Gonna find this is_safe crash bug -Blod */
   {
      bug("Is_safe: %s opponent does not exist!", ch->name);
      return TRUE;
   }
   if (!victim->in_room)
   {
      bug("Is_safe: %s has no physical location!", victim->name);
      return TRUE;
   }
   if (!IS_NPC(victim) && (xIS_SET(victim->act, PLR_ATTACKER) || xIS_SET(victim->act, PLR_KILLER) || xIS_SET(victim->act, PLR_THIEF)))
   {
      return FALSE; // They are never safe.
   }
   if (has_artifact(victim) == TRUE) //Hunt the pig
      return FALSE;

   if ((is_room_safe(ch) || is_room_safe(victim)))
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("A magical force prevents you from attacking.\n\r", ch);
      return TRUE;
   }

   if ((IS_PACIFIST(ch)) && !IN_ARENA(ch)) /* Fireblade */
   {
      set_char_color(AT_MAGIC, ch);
      ch_printf(ch, "You are a pacifist and will not fight.\n\r");
      return TRUE;
   }

   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_PACIFIST)) /* Gorog */
   {
      char buf[MSL];

      sprintf(buf, "%s is a pacifist and will not fight.\n\r", capitalize(victim->short_descr));
      set_char_color(AT_MAGIC, ch);
      send_to_char(buf, ch);
      return TRUE;
   }

   if (!IS_NPC(ch) && ch->level >= LEVEL_IMMORTAL)
      return FALSE;
      
   if (!IS_NPC(victim) && victim->level >= LEVEL_IMMORTAL)
      return FALSE;

   if (!IS_NPC(ch) && !IS_NPC(victim) && ch != victim && ((check_room_pk(ch) == 1) || check_room_pk(victim) == 1))
   {
      set_char_color(AT_IMMORT, ch);
      send_to_char("The gods have forbidden player killing in this area.\n\r", ch);
      return TRUE;
   }
   if (!IS_NPC(ch) && !IS_NPC(victim) && IN_ARENA(ch) && IN_ARENA(victim))
      return FALSE;
   //Check for same clan/same kingdom/same ip, etc.
   if (!IS_NPC(ch) && !IS_NPC(victim))
   {
      /*if (ch->desc && victim->desc && ch->desc->host && victim->desc->host && !str_cmp(ch->desc->host, victim->desc->host))
      {
         send_to_char("Killing a multi is NOT ALLOWED.\n\r", ch);
         return TRUE;
      }*/
      //Check to see if they can kill the player while on caste map
      if (kingdom_table[ch->pcdata->hometown]->peace[victim->pcdata->hometown] >= 2)
      {
         ch_printf(ch, "Your kingdom has a policy of %s with %s's kingdom.\n\r",
            cpstatus[kingdom_table[ch->pcdata->hometown]->peace[victim->pcdata->hometown]], victim->name);
         return TRUE;
      }
      if (IS_SET(ch->in_room->area->flags, AFLAG_CARPENTER))
      {
         if (kingdom_table[ch->pcdata->hometown]->peace[victim->pcdata->hometown] >= 3)
         {
            ch_printf(ch, "You are at peace with %s's kingdom.\n\r", PERS_MAP(victim, ch));
            return TRUE;
         }
      }
   }
   if (IS_NPC(ch) || IS_NPC(victim))
      return FALSE;

   return FALSE;
}

/*
 * just verify that a corpse looting is legal
 */
bool legal_loot(CHAR_DATA * ch, CHAR_DATA * victim)
{
   /* anyone can loot mobs */
   if (IS_NPC(victim))
      return TRUE;
   /* non-charmed mobs can loot anything */
   if (IS_NPC(ch) && !ch->master)
      return TRUE;
   /* members of different clans can loot too! -Thoric */
   if (check_room_pk(ch) == 4)
      return TRUE;
   return FALSE;
}

/*
 * just verify that stealing coins is legal
 */
bool legal_loot_coins(CHAR_DATA * ch, CHAR_DATA * victim)
{
   /* anyone can loot mobs */
   if (IS_NPC(victim))
      return TRUE;
   /* non-charmed mobs can loot anything */
   if (IS_NPC(ch) && !ch->master)
      return TRUE;
   /* members of different clans can loot too! -Thoric */
   if (check_room_pk(ch) >= 2)
      return TRUE;
   return FALSE;
}

bool can_see_intro(CHAR_DATA *ch, CHAR_DATA *victim)
{
   int mod;
   OBJ_DATA *light;
   
   if (get_wear_hidden_cloak(victim))
      return FALSE;
      
   if (!IN_WILDERNESS(ch))
   {
      if (!can_see(ch, victim))
         return FALSE;
   }
   
   if (!can_see_map(ch, victim))
      return FALSE;
   if (gethour() > 21 || gethour() < 6)
   {
      mod = 2;
      light = get_eq_char(victim, WEAR_LIGHT);
      if (light && light->item_type == ITEM_LIGHT)
         mod = 4;
   }
   else
   {
      mod = 5;
   }
   if (abs(ch->coord->x - victim->coord->x) > mod || abs(ch->coord->y - victim->coord->y) > mod)
   {
      return FALSE;
   } 
   return TRUE;
}

/*
 * See if an attack justifies a KILLER flag.
 */
void check_killer(CHAR_DATA * ch, CHAR_DATA * victim)
{
   int oldb;
   CHAR_DATA *rch;
   INTRO_DATA *intro;

   oldb = barena;
   /*
    * NPC's are fair game.
    */
   if (IS_NPC(victim))
   {
      if (!IS_NPC(ch))
      {
         int level_ratio;

         level_ratio = number_range(2, 5);
         if (ch->pcdata->clan)
            ch->pcdata->clan->mkills++;
         ch->pcdata->mkills++;
	 ch->fame = ch->fame + 1;
         ch->in_room->area->mkills++;
         if (ch->pcdata->deity)
         {
            if (victim->race == ch->pcdata->deity->npcrace)
               adjust_favor(ch, 3, level_ratio);
            else if (victim->race == ch->pcdata->deity->npcfoe)
               adjust_favor(ch, 17, level_ratio);
            else
               adjust_favor(ch, 2, level_ratio);
         }
      }
      return;
   }
   //NPC and Charmees first....
   /*
    * Charm-o-rama.
    */
   if (IS_AFFECTED(ch, AFF_CHARM))
   {
      if (!ch->master)
      {
         char buf[MSL];

         sprintf(buf, "Check_killer: %s bad AFF_CHARM", IS_NPC(ch) ? ch->short_descr : ch->name);
         bug(buf, 0);
         affect_strip(ch, gsn_charm_person);
         xREMOVE_BIT(ch->affected_by, AFF_CHARM);
         return;
      }

      /* stop_follower( ch ); */
      if (ch->master)
         check_killer(ch->master, victim);
      return;
   }

   /*
    * NPC's are cool of course (as long as not charmed).
    * Hitting yourself is cool too (bleeding).
    * So is being immortal (Alander's idea).
    * And current killers stay as they are.
    */
   if (IS_NPC(ch))
   {
      if (!IS_NPC(victim))
      {
         int level_ratio;

         if (victim->pcdata->clan)
            victim->pcdata->clan->mdeaths++;
         victim->pcdata->mdeaths++;
         victim->in_room->area->mdeaths++;
         level_ratio = number_range(2, 5);
         if (victim->pcdata->deity)
         {
            if (ch->race == victim->pcdata->deity->npcrace)
               adjust_favor(victim, 12, level_ratio);
            else if (ch->race == victim->pcdata->deity->npcfoe)
               adjust_favor(victim, 15, level_ratio);
            else
               adjust_favor(victim, 11, level_ratio);
         }
      }
      return;
   }


   /*
    * If you kill yourself nothing happens.
    */

   if (ch == victim || ch->level >= LEVEL_IMMORTAL)
      return;
      
   //PC Stuff now that we are all returned

   if ((xIS_SET(victim->act, PLR_KILLER) || xIS_SET(victim->act, PLR_THIEF) || xIS_SET(victim->act, PLR_ATTACKER)) && !IS_NPC(victim))
   {
      if (!IS_NPC(ch))
      {
         if (ch->pcdata->clan)
         {
            ch->pcdata->clan->pkills[1]++;
         }
         ch->pcdata->pkills++;
         ch->in_room->area->pkills++;
         ch->hit = ch->max_hit;
         ch->hit += ch->max_hit / 3;
         ch->mana = ch->max_mana;
         ch->mana += ch->max_mana / 3;
         ch->move = ch->max_move;
         if (ch->pcdata)
            ch->pcdata->condition[COND_BLOODTHIRST] = (10 + ch->level);
         update_pos(victim);
         if (victim != ch)
         {
            act(AT_MAGIC, "Bolts of red energy rise from the corpse, seeping into $n.", ch, NULL, NULL, TO_ROOM);
            act(AT_MAGIC, "Bolts of red energy rise from the corpse, seeping into you.", ch, NULL, NULL, TO_CHAR);
         }
         if (victim->pcdata->clan)
         {
            victim->pcdata->clan->pdeaths[1]++;

         }
         victim->pcdata->pdeaths++;
         adjust_favor(victim, 11, 1);
         adjust_favor(ch, 2, 1);
         barena = 1;
         //type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 6 - kill 7 - pkill
//8 - pdeath 9 - pranking
         update_barena(ch, 5);
         update_barena(ch, 7);
         update_barena(ch, 9);
         update_barena(victim, 5);
         update_barena(victim, 8);
         update_barena(victim, 9);
         save_barena_data();
         barena = oldb;
         WAIT_STATE(victim, 6 * PULSE_VIOLENCE);
      }
   }
   else if (!IS_NPC(ch) && !IS_NPC(victim))
   {
      /* not of same clan? Go ahead and kill!!! */
      if (check_room_pk(ch) > 1)
      {
         if (!ch->pcdata->clan || !victim->pcdata->clan || ch->pcdata->clan != victim->pcdata->clan)
         {
            if (ch->pcdata->clan)
            {
               ch->pcdata->clan->pkills[1]++;
            }
            ch->pcdata->pkills++;
	        ch->fame = ch->fame + 7;
            ch->hit = ch->max_hit;
            ch->mana = ch->max_mana;
            ch->move = ch->max_move;
            if (ch->pcdata)
               ch->pcdata->condition[COND_BLOODTHIRST] = (10 + ch->level);
            update_pos(victim);
            if (victim != ch)
            {
               act(AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into $n.", ch, NULL, NULL, TO_ROOM);
               act(AT_MAGIC, "Bolts of blue energy rise from the corpse, seeping into you.", ch, NULL, NULL, TO_CHAR);
            }
            if (victim->pcdata->clan)
            {
               victim->pcdata->clan->pdeaths[1]++;
            }
            victim->pcdata->pdeaths++;
            adjust_favor(victim, 11, 1);
            adjust_favor(ch, 2, 1);
            barena = 1;
            //type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 6 - kill 7 - pkill
            //8 - pdeath 9 - pranking
            update_barena(ch, 5);
            update_barena(ch, 7);
            update_barena(ch, 9);
            update_barena(victim, 5);
            update_barena(victim, 8);
            update_barena(victim, 9);
            save_barena_data();
            barena = oldb;
            WAIT_STATE(victim, 3 * PULSE_VIOLENCE);
         }
      }
   }
   else if (!IS_NPC(victim))
   {
      if (victim->pcdata->clan)
      {
         victim->pcdata->clan->pdeaths[1]++;
      }
      victim->pcdata->pdeaths++;
      victim->in_room->area->pdeaths++;
      barena = 1;
      //type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 6 - kill 7 - pkill
      //8 - pdeath 9 - pranking
      update_barena(victim, 5);
      update_barena(victim, 8);
      update_barena(victim, 9);
      save_barena_data();
      barena = oldb;
   }
   
   //No need to mess with intros in an arena fight.
   if (IN_ARENA(ch) || IN_ARENA(victim))
   {
      save_char_obj(victim);       
      save_char_obj(ch);
      return;
   }
   for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
   {
      if (intro->pid == victim->pcdata->pid)
      {
         if (intro->value < 0)
            intro->value *=-1;

         REMOVE_BIT(intro->flags, INTRO_ATTACKER);
         REMOVE_BIT(intro->flags, INTRO_KILLER);
         REMOVE_BIT(intro->flags, INTRO_THIEF);
         REMOVE_BIT(intro->flags, INTRO_MYATTACKER);
         REMOVE_BIT(intro->flags, INTRO_MYKILLER);
         REMOVE_BIT(intro->flags, INTRO_MYTHIEF);
         break;
      }
   }

   for (intro = victim->pcdata->first_introduction; intro; intro = intro->next)
   {
      if (intro->pid == ch->pcdata->pid && can_see_intro(ch, victim))
      {
         if (intro->value > 0)
            intro->value *=-1;
         intro->value -=15000;
         if (intro->value < -150000)
            intro->value = -150000;
         SET_BIT(intro->flags, INTRO_MYKILLER);
         REMOVE_BIT(intro->flags, INTRO_KILLER);
         intro->lastseen = time(0);
         break;
      }
   }
   
   if (!intro && can_see_intro(ch, victim))
   {
      CREATE(intro, INTRO_DATA, 1);
      intro->pid = ch->pcdata->pid;
      intro->value = -15000;
      intro->lastseen = time(0);
      SET_BIT(intro->flags, INTRO_MYKILLER);
      LINK(intro, victim->pcdata->first_introduction, victim->pcdata->last_introduction, next, prev);   
   }

   for (rch = ch->in_room->first_person; rch; rch = rch->next)
   {
      if (!IS_NPC(rch))
      {
         for (intro = rch->pcdata->first_introduction; intro; intro = intro->next)
         {
            if (ch->pcdata->pid == intro->pid && rch != ch && rch != victim && can_see_intro(rch, victim))
            {
               if (intro->value > 100000)
                  continue;
               if (intro->value > 0)
                  intro->value *=-1;
               intro->value -=15000;
               if (intro->value < -150000)
                  intro->value = -150000;
               intro->lastseen = time(0);
              
               if (!IS_SET(intro->flags, INTRO_MYKILLER))
                   SET_BIT(intro->flags, INTRO_KILLER);
               break;
            }
         }
         if (!intro && can_see_intro(rch, victim) && rch != ch && rch != victim)
         {
            CREATE(intro, INTRO_DATA, 1);
            intro->pid = ch->pcdata->pid;
            intro->value = -15000;
            intro->lastseen = time(0);
            SET_BIT(intro->flags, INTRO_KILLER);
            LINK(intro, rch->pcdata->first_introduction, rch->pcdata->last_introduction, next, prev);   
         }  
         save_char_obj(rch);
      }
   }
   save_char_obj(victim);       
   save_char_obj(ch);
   return;
}

/*
 * See if an attack justifies a ATTACKER flag.
 */
void check_attacker(CHAR_DATA * ch, CHAR_DATA * victim)
{
   CHAR_DATA *rch;
   INTRO_DATA *intro;
/* 
 * Made some changes to this function Apr 6/96 to reduce the prolifiration
 * of attacker flags in the realms. -Narn
 */
   /*
    * NPC's are fair game.
    * So are killers and thieves.
    */
   if (IS_NPC(victim) || xIS_SET(victim->act, PLR_KILLER) || xIS_SET(victim->act, PLR_THIEF) || xIS_SET(victim->act, PLR_ATTACKER))
      return;

   //no attack flag for legal kills you know :-) 
   if (in_arena(ch))
      return;

/* Pkiller versus pkiller will no longer ever make an attacker flag
    { if ( !(ch->pcdata->clan && victim->pcdata->clan
      && ch->pcdata->clan == victim->pcdata->clan ) )  return; }
*/

   /*
    * Charm-o-rama.
    */
   if (IS_AFFECTED(ch, AFF_CHARM))
   {
      if (!ch->master)
      {
         char buf[MSL];

         sprintf(buf, "Check_attacker: %s bad AFF_CHARM", IS_NPC(ch) ? ch->short_descr : ch->name);
         bug(buf, 0);
         affect_strip(ch, gsn_charm_person);
         xREMOVE_BIT(ch->affected_by, AFF_CHARM);
         return;
      }

      /* Won't have charmed mobs fighting give the master an attacker 
         flag.  The killer flag stays in, and I'll put something in 
         do_murder. -Narn */
      /* xSET_BIT(ch->master->act, PLR_ATTACKER); */
      /* stop_follower( ch ); */
      return;
   }

   /*
    * NPC's are cool of course (as long as not charmed).
    * Hitting yourself is cool too (bleeding).
    * So is being immortal (Alander's idea).
    * And current killers stay as they are.
    */
   if (IS_NPC(ch) || ch == victim || ch->level >= LEVEL_IMMORTAL || xIS_SET(ch->act, PLR_ATTACKER) || xIS_SET(ch->act, PLR_KILLER) || IN_ARENA(ch))
      return;
      
   for (intro = victim->pcdata->first_introduction; intro; intro = intro->next)
   {
      if (intro->pid == ch->pcdata->pid && can_see_intro(ch, victim))
      {
         if (intro->value > 0)
            intro->value *=-1;
         intro->value -=15000;
         if (intro->value < -150000)
            intro->value = -150000;
         if (!IS_SET(intro->flags, INTRO_MYKILLER))
            SET_BIT(intro->flags, INTRO_MYATTACKER);
         REMOVE_BIT(intro->flags, INTRO_ATTACKER);
         intro->lastseen = time(0);
         break;
      }
   }
   
   if (!intro && can_see_intro(ch, victim))
   {
      CREATE(intro, INTRO_DATA, 1);
      intro->pid = ch->pcdata->pid;
      intro->value = -15000;
      intro->lastseen = time(0);
      SET_BIT(intro->flags, INTRO_MYATTACKER);
      LINK(intro, victim->pcdata->first_introduction, victim->pcdata->last_introduction, next, prev);   
   }

   for (rch = ch->in_room->first_person; rch; rch = rch->next)
   {
      if (!IS_NPC(rch))
      {
         for (intro = rch->pcdata->first_introduction; intro; intro = intro->next)
         {
            if (rch->pcdata->pid == intro->pid && rch != ch && rch != victim && can_see_intro(rch, victim))
            {
               if (intro->value > 100000)
                  continue;
               if (intro->value > 0)
                  intro->value *=-1;
               intro->value -=15000;
               if (intro->value < -150000)
                  intro->value = -150000;
               intro->lastseen = time(0);
               if (!IS_SET(intro->flags, INTRO_MYKILLER) || !IS_SET(intro->flags, INTRO_MYATTACKER) || !IS_SET(intro->flags, INTRO_KILLER))
                   SET_BIT(intro->flags, INTRO_ATTACKER);
               break;
            }
         }
         if (!intro && can_see_intro(rch, victim) && rch != ch && rch != victim)
         {
            CREATE(intro, INTRO_DATA, 1);
            intro->pid = ch->pcdata->pid;
            intro->value = -15000;
            intro->lastseen = time(0);
            SET_BIT(intro->flags, INTRO_ATTACKER);
            LINK(intro, rch->pcdata->first_introduction, rch->pcdata->last_introduction, next, prev);   
         }  
         save_char_obj(rch);
      }
   }
   save_char_obj(victim);       
   save_char_obj(ch);
   return;
}

//I want to die now :-)
void do_giveup(CHAR_DATA *ch, char *argument)
{
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  giveup now\n\rThis command will kill you if your health is <= -3\n\r", ch);
      return;
   }
   if (ch->hit <= -3)
   {
      act(AT_RED, "It looks like $n give up hope in surviving and passed on.", ch, NULL, NULL, TO_ROOM);
      ch_printf(ch, "You giveup the battle and die!\n\r", ch);
      ch->hit = -16;
      update_pos(ch);
      return;
   }
   else
   {
      send_to_char("You can only use this if your health is at -3 or below.\n\r", ch);
      return;
   }
}

/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA * victim)
{
   if (!victim)
   {
      bug("update_pos: null victim", 0);
      return;
   }

   if (victim->hit > 0)
   {
      if (victim->position <= POS_STUNNED)
         victim->position = POS_STANDING;
      if (IS_AFFECTED(victim, AFF_PARALYSIS))
         victim->position = POS_STUNNED;
      return;
   }

   if (IS_NPC(victim) || victim->hit < -15)
   {
      if (victim->mount)
      {
         act(AT_ACTION, "$n falls from $N.", victim, NULL, victim->mount, TO_ROOM);
         xREMOVE_BIT(victim->mount->act, ACT_MOUNTED);
         victim->mount = NULL;
      }
      victim->position = POS_DEAD;
      return;
   }

   if (victim->hit < -11)
      victim->position = POS_MORTAL;
   else if (victim->hit < -3)
      victim->position = POS_INCAP;
   else
      victim->position = POS_STUNNED;

   if (victim->position > POS_STUNNED && IS_AFFECTED(victim, AFF_PARALYSIS))
      victim->position = POS_STUNNED;

   if (victim->mount)
   {
      act(AT_ACTION, "$n falls unconscious from $N.", victim, NULL, victim->mount, TO_ROOM);
      xREMOVE_BIT(victim->mount->act, ACT_MOUNTED);
      victim->mount = NULL;
   }
   return;
}


/*
 * Start fights.
 */
void set_fighting(CHAR_DATA * ch, CHAR_DATA * victim)
{
   FIGHT_DATA *fight;

   if (ch->fighting)
   {
      char buf[MSL];

      sprintf(buf, "Set_fighting: %s -> %s (already fighting %s)", ch->name, victim->name, ch->fighting->who->name);
      bug(buf, 0);
      return;
   }
   
   if (ch->coord->x != victim->coord->x || ch->coord->y != victim->coord->y || ch->map != victim->map)
      return;

   if (IS_AFFECTED(ch, AFF_SLEEP))
      affect_strip(ch, gsn_sleep);

   /* Limit attackers -Thoric */
   if (victim->num_fighting > max_fight(victim))
   {
      send_to_char("There are too many people fighting for you to join in.\n\r", ch);
      return;
   }

   CREATE(fight, FIGHT_DATA, 1);
   fight->who = victim;
   if (sysdata.resetgame && check_powerlevel(ch, victim) 
   &&  (get_player_statlevel(ch) > get_player_statlevel(victim) + 15))
   {
      fight->twinkpoints = get_player_statlevel(ch) - get_player_statlevel(victim) - 11;
      fight->twinkpoints = UMIN(25, fight->twinkpoints);
   }
   fight->xp = (int) xp_compute(ch, victim) * 0.85;
   /*
      fight->align = align_compute( ch, victim );
    */
   if (!IS_NPC(ch) && IS_NPC(victim))
      fight->timeskilled = times_killed(ch, victim);
   ch->num_fighting = 1;
   ch->fighting = fight;
   /* ch->position = POS_FIGHTING; */
   if (IS_NPC(ch))
      ch->position = POS_FIGHTING;
   else
      switch (ch->style)
      {
         case (STYLE_EVASIVE): case (STYLE_DIVINE): case (STYLE_WIZARDRY):
            ch->position = POS_EVASIVE;
            break;
         case (STYLE_DEFENSIVE):
            ch->position = POS_DEFENSIVE;
            break;
         case (STYLE_AGGRESSIVE):
            ch->position = POS_AGGRESSIVE;
            break;
         case (STYLE_BERSERK):
            ch->position = POS_BERSERK;
            break;
         default:
            ch->position = POS_FIGHTING;
      }
   victim->num_fighting++;
   if (victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS))
   {
      send_to_char("You are disturbed!\n\r", victim->switched);
      do_return(victim->switched, "");
   }
   return;
}

void do_parry(CHAR_DATA *ch, char *argument)
{
   if (argument[0] == '\0')
   {
      send_to_char("parry [on/off]\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "on"))
   {
      xSET_BIT(ch->act, PLR_PARRY);
      send_to_char("You will now parry in battle.\n\r", ch);
      return;
   }
   else if (!str_cmp(argument, "off"))
   {
      xREMOVE_BIT(ch->act, PLR_PARRY);
      send_to_char("You will no longer parry in battle.\n\r", ch);
      return;
   }
   else
   {
      do_parry(ch, "");
      return;
   }
}

void do_dodge(CHAR_DATA *ch, char *argument)
{
   if (argument[0] == '\0')
   {
      send_to_char("dodge [on/off]\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "on"))
   {
      xREMOVE_BIT(ch->act, PLR_DODGE);
      send_to_char("You will now dodge in battle.\n\r", ch);
      return;
   }
   else if (!str_cmp(argument, "off"))
   {
      xSET_BIT(ch->act, PLR_DODGE);
      send_to_char("You will no longer dodge in battle.\n\r", ch);
      return;
   }
   else
   {
      do_dodge(ch, "");
      return;
   }
}

void do_tumble(CHAR_DATA *ch, char *argument)
{
   if (argument[0] == '\0')
   {
      send_to_char("tumble [on/off]\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "on"))
   {
      xREMOVE_BIT(ch->act, PLR_TUMBLE);
      send_to_char("You will now tumble in battle.\n\r", ch);
      return;
   }
   else if (!str_cmp(argument, "off"))
   {
      xSET_BIT(ch->act, PLR_TUMBLE);
      send_to_char("You will no longer tumble in battle.\n\r", ch);
      return;
   }
   else
   {
      do_tumble(ch, "");
      return;
   }
}

void do_counter(CHAR_DATA *ch, char *argument)
{
   if (argument[0] == '\0')
   {
      send_to_char("counter [on/off]\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "on"))
   {
      xREMOVE_BIT(ch->act, PLR_COUNTER);
      send_to_char("You will now counter in battle.\n\r", ch);
      return;
   }
   else if (!str_cmp(argument, "off"))
   {
      xSET_BIT(ch->act, PLR_COUNTER);
      send_to_char("You will no longer counter in battle.\n\r", ch);
      return;
   }
   else
   {
      do_counter(ch, "");
      return;
   }
}


CHAR_DATA *who_fighting(CHAR_DATA * ch)
{
   if (!ch)
   {
      bug("who_fighting: null ch", 0);
      return NULL;
   }
   if (!ch->fighting)
      return NULL;
   return ch->fighting->who;
}

void free_fight(CHAR_DATA * ch)
{
   if (!ch)
   {
      bug("Free_fight: null ch!", 0);
      return;
   }
   if (ch->fighting)
   {
      if (!char_died(ch->fighting->who))
         --ch->fighting->who->num_fighting;
      DISPOSE(ch->fighting);
   }
   ch->fighting = NULL;
   if (ch->mount)
      ch->position = POS_MOUNTED;
   else
      ch->position = POS_STANDING;
   /* Berserk wears off after combat. -- Altrag */
   if (IS_AFFECTED(ch, AFF_BERSERK))
   {
      affect_strip(ch, gsn_berserk);
      set_char_color(AT_WEAROFF, ch);
      send_to_char(skill_table[gsn_berserk]->msg_off, ch);
      send_to_char("\n\r", ch);
   }
   return;
}


/*
 * Stop fights.
 */
void stop_fighting(CHAR_DATA * ch, bool fBoth)
{
   CHAR_DATA *fch;

   free_fight(ch);
   update_pos(ch);
   
   //Used with flee timer, don't remove the npc check
   if (!IS_NPC(ch))
      ch->fight_timer = 0;
   ch->agi_meter = 0;
   if (!IS_NPC(ch))
      strcpy(ch->pcdata->stackbuf, "");
   
   if (!fBoth) /* major short cut here by Thoric */
      return;

   for (fch = first_char; fch; fch = fch->next)
   {
      if (who_fighting(fch) == ch)
      {
         free_fight(fch);
         update_pos(fch);
      }
   }
   return;
}

/* Vnums for the various bodyparts */
int part_vnums[] = { 12, /* Head */
   14, /* arms */
   15, /* legs */
   13, /* heart */
   44, /* brains */
   16, /* guts */
   45, /* hands */
   46, /* feet */
   47, /* fingers */
   48, /* ear */
   49, /* eye */
   50, /* long_tongue */
   51, /* eyestalks */
   52, /* tentacles */
   53, /* fins */
   54, /* wings */
   55, /* tail */
   56, /* scales */
   59, /* claws */
   87, /* fangs */
   58, /* horns */
   57, /* tusks */
   55, /* tailattack */
   85, /* sharpscales */
   84, /* beak */
   86, /* haunches */
   83, /* hooves */
   82, /* paws */
   81, /* forelegs */
   80, /* feathers */
   0, /* r1 */
   0 /* r2 */
};

/* Messages for flinging off the various bodyparts */
char *part_messages[] = {
   "$n's severed head plops from its neck.",
   "$n's arm is sliced from $s dead body.",
   "$n's leg is sliced from $s dead body.",
   "$n's heart is torn from $s chest.",
   "$n's brains spill grotesquely from $s head.",
   "$n's guts spill grotesquely from $s torso.",
   "$n's hand is sliced from $s dead body.",
   "$n's foot is sliced from $s dead body.",
   "A finger is sliced from $n's dead body.",
   "$n's ear is sliced from $s dead body.",
   "$n's eye is gouged from its socket.",
   "$n's tongue is torn from $s mouth.",
   "An eyestalk is sliced from $n's dead body.",
   "A tentacle is severed from $n's dead body.",
   "A fin is sliced from $n's dead body.",
   "A wing is severed from $n's dead body.",
   "$n's tail is sliced from $s dead body.",
   "A scale falls from the body of $n.",
   "A claw is torn from $n's dead body.",
   "$n's fangs are torn from $s mouth.",
   "A horn is wrenched from the body of $n.",
   "$n's tusk is torn from $s dead body.",
   "$n's tail is sliced from $s dead body.",
   "A ridged scale falls from the body of $n.",
   "$n's beak is sliced from $s dead body.",
   "$n's haunches are sliced from $s dead body.",
   "A hoof is sliced from $n's dead body.",
   "A paw is sliced from $n's dead body.",
   "$n's foreleg is sliced from $s dead body.",
   "Some feathers fall from $n's dead body.",
   "r1 message.",
   "r2 message."
};

/*
 * Improved Death_cry contributed by Diavolo.
 * Additional improvement by Thoric (and removal of turds... sheesh!)  
 * Support for additional bodyparts by Fireblade
 */
void death_cry(CHAR_DATA * ch)
{
   ROOM_INDEX_DATA *was_in_room;
   char *msg;
   EXIT_DATA *pexit;
   int vnum, shift, index, i;

   if (!ch)
   {
      bug("DEATH_CRY: null ch!", 0);
      return;
   }

   vnum = 0;
   msg = NULL;

   switch (number_range(0, 5))
   {
      default:
         msg = "You hear $n's death cry.";
         break;
      case 0:
      case 1:
         msg = "$n hits the ground ... DEAD.";
         break;
      case 2:
      case 3:
         msg = "$n splatters blood on your armor.";
         break;
      case 4:
      case 5:
         shift = number_range(0, 31);
         index = 1 << shift;

         for (i = 0; i < 32 && ch->xflags; i++)
         {
            if (HAS_BODYPART(ch, index) && i != 1 && i != 2) //no arm/leg
            {
               msg = part_messages[shift];
               vnum = part_vnums[shift];
               break;
            }
            else
            {
               shift = number_range(0, 31);
               index = 1 << shift;
            }
         }

         if (!msg)
            msg = "You hear $n's death cry.";
         break;
   }

   act(AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM);

   if (vnum)
   {
      OBJ_DATA *obj;

      if (!get_obj_index(vnum))
      {
         bug("death_cry: invalid vnum", 0);
         return;
      }

      obj = create_object(get_obj_index(vnum), 0);
      obj->timer = number_range(4, 7);
      if (IS_AFFECTED(ch, AFF_POISON))
         obj->value[3] = 10;

      obj = obj_to_room(obj, ch->in_room, ch);
   }

   if (IS_NPC(ch))
      msg = "You hear something's death cry.";
   else
      msg = "You hear someone's death cry.";

   was_in_room = ch->in_room;
   for (pexit = was_in_room->first_exit; pexit; pexit = pexit->next)
   {
      if (pexit->to_room && pexit->to_room != was_in_room)
      {
         ch->in_room = pexit->to_room;
         act(AT_CARNAGE, msg, ch, NULL, NULL, TO_ROOM);
      }
   }
   ch->in_room = was_in_room;

   return;
}

//2 hour tick down
char *get_duel_time(int starttime)
{
   static char buf[MSL];
   int seconds = starttime + 7200 - time(0);
   int minutes;
   int hours;
   
   hours = seconds / 3600;
   minutes = seconds / 60;
   minutes = minutes % 60;
   seconds = seconds % 60;
   sprintf(buf, "%d:%2.2d:%2.2d", hours, minutes, seconds);
   return buf;
}

void do_duel(CHAR_DATA *ch, char *argument)
{
   DESCRIPTOR_DATA *d;
   ROOM_INDEX_DATA *room;
   CHAR_DATA *victim;
   
   if (check_npc(ch))
      return;
      
   if (!sysdata.resetgame)
      return;
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  duel show\n\r", ch);
      send_to_char("Syntax:  duel <player>\n\r", ch);
      send_to_char("Syntax:  duel accept\n\r", ch);
      send_to_char("Syntax:  duel deny\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "deny"))
   {
      if (ch->pcdata->dual_receive_name > -1)
      {
         for (d = first_descriptor; d; d = d->next)
         {
            if (d->character && d->character->pcdata->pid == ch->pcdata->dual_receive_name)
            {
               break;
            }
         }
         if (!d)
         {
            send_to_char("The challenger is no longer online, you escaped this time.\n\r", ch);
            act(AT_RED, "$n lucked out of $s duel.", ch, NULL, NULL, TO_MUD);
            ch->pcdata->dual_receive_name = -1;
            ch->pcdata->duel_receive_pranking = 0;
            ch->pcdata->duel_receive_time = 0;
            return;
         }
         else
         {
            ch->pcdata->power_ranking -= ch->pcdata->duel_receive_pranking;
            d->character->pcdata->power_ranking += ch->pcdata->duel_receive_pranking;
            ch_printf(ch, "You lose %d Power Ranking points for your cowardness.\n\r", ch->pcdata->duel_receive_pranking);
            ch_printf(d->character, "Your challenge for a duel was denied, you are rewarded %d Power Ranking Points",
               ch->pcdata->duel_receive_pranking);
            act(AT_RED, "$n has turned down $N'd duel, what a coward!.", ch, NULL, d->character, TO_MUD);
            ch->pcdata->dual_receive_name = -1;
            ch->pcdata->duel_receive_pranking = 0;
            ch->pcdata->duel_receive_time = 0;
            d->character->pcdata->duel_offer_name = -1;
            d->character->pcdata->duel_offer_pranking = 0;
            d->character->pcdata->duel_offer_time = 0;
            return;
         }
      }
      else
      {
         send_to_char("You have not received a challenge for a duel let.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(argument, "accept"))
   {
      if (ch->position < POS_STANDING)
      {
         send_to_char("You need to be standing to accept a duel.\n\r", ch);
         return;
      }
      if (ch->pcdata->dual_receive_name > -1)
      {
         for (d = first_descriptor; d; d = d->next)
         {
            if (d->character && d->character->pcdata->pid == ch->pcdata->dual_receive_name)
            {
               break;
            }
         }
         if (!d)
         {
            send_to_char("The challenger is no longer online, you escaped this time.\n\r", ch);
            act(AT_RED, "$n lucked out of $s duel.", ch, NULL, NULL, TO_MUD);
            ch->pcdata->dual_receive_name = -1;
            ch->pcdata->duel_receive_pranking = 0;
            ch->pcdata->duel_receive_time = 0;
            return;
         }
         else
         {
            room = get_room_index(number_range(60097, 60094));
            char_from_room(ch);
            char_to_room(ch, room); 
            REMOVE_ONMAP_FLAG(ch);
            ch->coord->x = -1;
            ch->coord->y = -1;
            ch->map = -1;
            if (d->character->fighting)
               stop_fighting(d->character, TRUE);
            room = get_room_index(number_range(60070, 60073));
            char_from_room(d->character);
            char_to_room(d->character, room); 
            REMOVE_ONMAP_FLAG(d->character);
            d->character->coord->x = -1;
            d->character->coord->y = -1;
            d->character->map = -1;
            send_to_char("You accepted the duel and your challenger is online, prepare to fight to the death!.\n\r", ch);
            send_to_char("Your duel has been accepted, prepare to fight to the death.\n\r", d->character);
            act(AT_RED, "$n accepted $N's challenge for a duel.", ch, NULL, d->character, TO_MUD);
            ch->pcdata->dual_receive_name = -1;
            ch->pcdata->duel_receive_pranking = 0;
            ch->pcdata->duel_receive_time = 0;
            d->character->pcdata->duel_offer_name = -1;
            d->character->pcdata->duel_offer_pranking = 0;
            d->character->pcdata->duel_offer_time = 0;
            do_look(ch, "auto");
            do_look(d->character, "auto");
            save_char_obj(ch);
            save_char_obj(d->character);
            return;
         }
      }
      else
      {
         send_to_char("You have not received a challenge for a duel let.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(argument, "show"))
   {
      if (ch->pcdata->duel_offer_name > -1)
      {
         for (d = first_descriptor; d; d = d->next)
         {
            if (d->character && d->character->pcdata->pid == ch->pcdata->duel_offer_name)
            {
               ch_printf(ch, "Your challenges : %s for %d points with %s time left\n\r\n\r", 
                  PERS_MAP(d->character, ch), ch->pcdata->duel_offer_pranking, get_duel_time(ch->pcdata->duel_offer_time));
               break;
            }
         }
         if (!d)
            ch_printf(ch, "Your challenges : Offline for %d points with %s time left\n\r\n\r",
               ch->pcdata->duel_offer_pranking, get_duel_time(ch->pcdata->duel_offer_time));
      }
      else
         send_to_char("You have no outstanding challenges that you have issued.\n\r\n\r", ch);
      if (ch->pcdata->dual_receive_name > -1)
      {
         for (d = first_descriptor; d; d = d->next)
         {
            if (d->character && d->character->pcdata->pid == ch->pcdata->dual_receive_name)
            {
               ch_printf(ch, "Challenges you received : %s for %d points with %s time left\n\r", 
                  PERS_MAP(d->character, ch), ch->pcdata->duel_receive_pranking, get_duel_time(ch->pcdata->duel_receive_time));
               break;
            }
         }
         if (!d)
            ch_printf(ch, "Challenges you received : Offline for %d points with %s time left\n\r",
               ch->pcdata->duel_receive_pranking, get_duel_time(ch->pcdata->duel_receive_time));
      }
      else
         send_to_char("You have no outstanding challenges that you have received.\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, argument)) != NULL)
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPCs.\n\r", ch);
         return;
      }
      if (victim == ch)
      {
         send_to_char("You cannot duel yourself.\n\r", ch);
         return;
      }
      if (get_player_statlevel(ch) -5 > get_player_statlevel(victim))
      {
         send_to_char("You have to be within 5 Power Levels to challenge someone.\n\r", ch);
         return;
      }
      act(AT_RED, "$n has challenged $N to a duel.", ch, NULL, victim, TO_MUD);
      act(AT_RED, "You have challenged $N to a duel.", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n challenged you to a duel.", ch, NULL, victim, TO_VICT);
      victim->pcdata->duel_receive_time = time(0);
      ch->pcdata->duel_offer_time = time(0);
      victim->pcdata->dual_receive_name = ch->pcdata->pid;
      ch->pcdata->duel_offer_name = victim->pcdata->pid;
      victim->pcdata->duel_receive_pranking = victim->pcdata->power_ranking/4;
      ch->pcdata->duel_offer_pranking = victim->pcdata->power_ranking/4;
      return;
   }
   do_duel(ch, "");
}

void do_spar(CHAR_DATA *ch, char *argument)
{
   return;
}

void raw_kill(CHAR_DATA * ch, CHAR_DATA * victim)
{
   OBJ_DATA *dobj;
   CHAR_DATA *gch;
   char buf[MIL];
   int x;
   int twinkpoints = 0;

   if (!victim)
   {
      bug("raw_kill: null victim!", 0);
      return;
   }
/* backup in case hp goes below 1 */
   if (NOT_AUTHED(victim))
   {
      bug("raw_kill: killing unauthed", 0);
      return;
   }
   if (!IS_NPC(ch) && !IN_ARENA(victim) && ch->fighting && victim != ch->fighting->who && !IS_NPC(victim))
   {
      if (sysdata.resetgame && check_powerlevel(ch, victim) 
      &&  (get_player_statlevel(ch) > get_player_statlevel(victim) + 15))
      {
         twinkpoints = get_player_statlevel(ch) - get_player_statlevel(victim) - 11;
         twinkpoints = UMIN(25, ch->fighting->twinkpoints);
         ch->pcdata->twink_points += twinkpoints;   
         ch_printf(ch, "You have been assigned %d Twink Points, if you are now over 100 you will be deleted in a few seconds.", ch->fighting->twinkpoints);
      }
   }
   else
   {
      if (sysdata.resetgame && !IN_ARENA(victim) && !IS_NPC(ch) && ch->fighting && ch->fighting->twinkpoints)
      {
         ch->pcdata->twink_points += ch->fighting->twinkpoints;   
         twinkpoints = ch->fighting->twinkpoints;
         ch_printf(ch, "You have been assigned %d Twink Points, if you are now over 100 you will be deleted in a few seconds.", ch->fighting->twinkpoints);
      }
   }
   //No Power Ranking points for twink kills
   if (sysdata.resetgame && ch != victim && !IN_ARENA(victim) && !IS_NPC(ch) && !IS_NPC(victim) && !twinkpoints)
   {
      int perranking;
      int persworth;
      int percent;
      int points;
      
      perranking = (100 * victim->pcdata->power_ranking / ch->pcdata->power_ranking)-100;
      persworth = (100 * get_player_statlevel(victim) / get_player_statlevel(ch))-100;
      percent = 10 + ((perranking+persworth)/2);
      points = URANGE(1, percent * victim->pcdata->power_ranking / 100, victim->pcdata->power_ranking/2);
      points = UMIN(victim->pcdata->power_ranking-1, points);
      ch->pcdata->power_ranking += points;
      victim->pcdata->power_ranking -= points;
      ch_printf(ch, "&w&RYou earned %d Power Ranking points for your kill.&w\n\r", points);
      ch_printf(victim, "&w&RYou lost %d Power Ranking points for your death.&w\n\r", points);
   }
      
   stop_fighting(victim, TRUE);
   /* Take care of morphed characters */
   if (victim->morph)
   {
      do_unmorph_char(victim);
      raw_kill(ch, victim);
      return;
   }
   
   if (!IS_NPC(ch) && IS_NPC(victim))
   {
      if (ch->pcdata->quest && ch->pcdata->quest->questarea == ch->in_room->area 
      && ch->pcdata->quest->timeleft > 0 && ch->pcdata->quest->mission == 3 && xIS_SET(victim->act, ACT_BOSS) 
      && !IS_AFFECTED(victim, AFF_CHARM) && !IS_ACT_FLAG(victim, ACT_MOUNTSAVE))
      {
         if (ch->pcdata->quest->traveltime > -1)
            ch->pcdata->quest->traveltime = -1;
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
      
      if (ch->pcdata->quest && ch->pcdata->quest->questarea == ch->in_room->area 
      && ch->pcdata->quest->timeleft > 0 && ch->pcdata->quest->mission == 1 
      && !IS_AFFECTED(victim, AFF_CHARM) && !IS_ACT_FLAG(victim, ACT_MOUNTSAVE))
      {
         ch->pcdata->quest->killed++;
         if (ch->pcdata->quest->traveltime > -1)
            ch->pcdata->quest->traveltime = -1;
         if (ch->pcdata->quest->killed == ch->pcdata->quest->tokill) //Win
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
   if (IN_WILDERNESS(victim) && IS_NPC(victim))
   {
      WBLOCK_DATA *wblock;
      
      for (wblock = first_wblock; wblock; wblock = wblock->next)
      {
         if (victim->coord->x >= wblock->stx && victim->coord->x <= wblock->endx
         &&  victim->coord->y >= wblock->sty && victim->coord->y <= wblock->endy && victim->map == wblock->map)
         {
            wblock->kills++;
         }
      }
   } 
   if (in_arena(victim) && !IS_NPC(victim) && !xIS_SET(victim->in_room->room_flags, ROOM_PERMDEATH))
   {
      if (ch->desc && ch->desc->arena)
      {
         make_corpse(victim, ch);
         make_blood(victim);
         update_players_map(victim, -1, -1, -1, 1, get_room_index(21001));
         victim->hit = victim->max_hit;
         victim->position = POS_RESTING;
         //type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 6 - kill
         update_barena(victim, 1);
         update_barena(ch, 6);
         do_look(victim, "auto");
         send_to_char("You lost, better luck next time.\n\r", ch);
      }
      else
      {
         make_blood(victim);
         update_players_map(victim, -1, -1, -1, 1, get_room_index(21001));
         victim->hit = victim->max_hit;
         victim->position = POS_RESTING;
         do_look(victim, "auto");
         send_to_char("You lost, better luck next time.\n\r", ch);
      }
      return;
   }
   if (IS_NPC(victim))
   {
      if (xIS_SET(victim->act, ACT_MOUNTSAVE))
      {
         if (!in_arena(victim))
         {
            victim->m1 = 0;
            victim->mover -= number_range(1, 2);
         }
         victim->hunting = NULL;
         victim->hating = NULL;
         update_players_map(victim, -1, -1, -1, 2, get_room_index(21001));
         victim->hit = 1;
         victim->move = 200;
         victim->position = POS_STANDING;
         return;
      }
   }
   mprog_death_trigger(ch, victim);
   if (char_died(victim))
      return;
   /* death_cry( victim ); */

   rprog_death_trigger(ch, victim);
   if (char_died(victim))
      return;


   //So this only works on an actual pkill
   if (check_room_pk(victim) == 3 && !IS_NPC(ch) && !IS_NPC(victim))
   {
      int ocnt = -1;
      int opk;

      for (dobj = victim->first_carrying; dobj; dobj = dobj->next_content)
      {
         if (dobj->wear_loc != WEAR_NONE && !IS_OBJ_STAT(dobj, ITEM_NODROP))
         {
            ++ocnt;
         }
      }
      if (ocnt != -1)
      {
         opk = number_range(0, ocnt);
         ocnt = -1;
         for (dobj = victim->first_carrying; dobj; dobj = dobj->next_content)
         {
            if (dobj->wear_loc != WEAR_NONE)
            {
               if (++ocnt == opk && !IS_OBJ_STAT(dobj, ITEM_NODROP))
               {
                  obj_from_char(dobj);
                  obj_to_room(dobj, victim->in_room, victim);
                  act(AT_ACTION, "$p falls to the ground right before $n dies.", victim, dobj, NULL, TO_CANSEE);
                  act(AT_ACTION, "$P falls from you right before you DIE.", victim, dobj, NULL, TO_CHAR);
                  break;
               }
            }
         }
      }
   }
   if (!IS_NPC(ch) && !IS_NPC(victim))
   {
      for (dobj = victim->first_carrying; dobj; dobj = dobj->next_content)
      {
         if (IS_OBJ_STAT(dobj, ITEM_ARTIFACT))
         {
            obj_from_char(dobj);
            obj_to_room(dobj, victim->in_room, victim);
            act(AT_ACTION, "The ARTIFACT $p falls to the ground right before $n dies.", victim, dobj, NULL, TO_CANSEE);
            act(AT_ACTION, "The ARTIFACT $P falls from you right before you DIE.", victim, dobj, NULL, TO_CHAR);
            break;
         }
      }
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_KINGDOMMOB))
   {
      char buf[MIL];
      int change =  victim->in_room->area->population / 100;
      victim->in_room->area->population -= change;
      if (victim->in_room->area->population < 1000)
         victim->in_room->area->population = 1000;
      ch_printf(ch, "&G&W>>>>>The population of this kingdom has dropped by %d by killing these civilians.<<<<<\n\r", change);
      sprintf(buf, "%s has slaughtered your population and dropped it by %d", PERS_KINGDOM(ch, victim->in_room->area->kingdom), change);
      write_kingdom_logfile(victim->in_room->area->kingdom, buf, KLOG_POP_VICTIM);
      if ((!IS_NPC(ch) && ch->pcdata->hometown > 1 && ch->pcdata->hometown < sysdata.max_kingdom)
      || (IS_NPC(ch) && xIS_SET(victim->act, ACT_MILITARY) && ch->m4 > 1 && ch->m4 < sysdata.max_kingdom))
      {
         int sking;
         if (IS_NPC(ch))
            sking = ch->m4;
         else
            sking = ch->pcdata->hometown;
            
         sprintf(buf, "%s has slaughtered %d in the kingdom of %s", PERS_KINGDOM(ch, sking), change, kingdom_table[victim->in_room->area->kingdom]->name);
         write_kingdom_logfile(sking, buf, KLOG_POP_ATTACKER);
      }
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MILITARY))
   {
      do_kingdomtalk(victim, "May my death not be in vane my lords.");
      sprintf(buf, "%s has been dealt a decise death blow by %s", PERS_KINGDOM(victim, victim->m4), PERS_KINGDOM(ch, victim->m4));
      write_kingdom_logfile(victim->m4, buf, KLOG_MIL_VICTIM);
      sprintf(buf, "%s of %s has been killed by %s", victim->name, kingdom_table[victim->m4]->name, ch->name);
      bug(buf, 0);
      if ((IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY)) || (!IS_NPC(ch) && ch->pcdata->hometown > 1 && ch->pcdata->hometown < sysdata.max_kingdom))
      {
         if (IS_NPC(ch))
         {
            sprintf(buf, "%s has been dispatched.", PERS_KINGDOM(victim, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown));
            do_kingdomtalk(ch, buf);
         }
         sprintf(buf, "%s has been dispatched by %s.", PERS_KINGDOM(victim, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown), PERS_KINGDOM(ch, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown));
         if (IS_NPC(ch))
            write_kingdom_logfile(ch->m4, buf, KLOG_MIL_ATTACKER);
         else
            write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_MIL_ATTACKER);
      }
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_EXTRACTMOB))
   {
      do_kingdomtalk(victim, "May my death not be in vane my lords.");
      sprintf(buf, "%s has been dealt a decise death blow by %s", PERS_KINGDOM(victim, victim->m4), PERS_KINGDOM(ch, victim->m4));
      write_kingdom_logfile(victim->m4, buf, KLOG_MIL_VICTIM);
      sprintf(buf, "%s of %s has been killed by %s", victim->name, kingdom_table[victim->m4]->name, ch->name);
      bug(buf, 0);
      if ((IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY)) || (!IS_NPC(ch) && ch->pcdata->hometown > 1 && ch->pcdata->hometown < sysdata.max_kingdom))
      {
         if (IS_NPC(ch))
         {
            sprintf(buf, "%s has been dispatched.", PERS_KINGDOM(victim, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown));
            do_kingdomtalk(ch, buf);
         }
         sprintf(buf, "%s has been dispatched by %s.", PERS_KINGDOM(victim, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown), PERS_KINGDOM(ch, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown));
         if (IS_NPC(ch))
            write_kingdom_logfile(ch->m4, buf, KLOG_MIL_ATTACKER);
         else
            write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_MIL_ATTACKER);
      }
   }
   if (!IS_NPC(victim) && victim->pcdata->hometown > 1 && victim->pcdata->hometown < sysdata.max_kingdom)
   {
      if ((IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY)) || (!IS_NPC(ch) && ch->pcdata->hometown > 1 && ch->pcdata->hometown < sysdata.max_kingdom))
      {
         if (IS_NPC(ch))
         {
            sprintf(buf, "%s has been dispatched.", PERS_KINGDOM(victim, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown));
            do_kingdomtalk(ch, buf);
         }
         sprintf(buf, "%s has been dealt a decise death blow by %s", PERS_KINGDOM(victim, victim->pcdata->hometown), PERS_KINGDOM(ch, victim->pcdata->hometown));
         write_kingdom_logfile(victim->pcdata->hometown, buf, KLOG_MIL_VICTIM);
         sprintf(buf, "%s has been dispatched by %s.", PERS_KINGDOM(victim, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown), PERS_KINGDOM(ch, IS_NPC(ch) ? ch->m4 : ch->pcdata->hometown));
         if (IS_NPC(ch))     
            write_kingdom_logfile(ch->m4, buf, KLOG_MIL_ATTACKER);
         else
            write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_MIL_ATTACKER);
      }
   }    
   make_corpse(victim, ch);
   if (victim->in_room->sector_type == SECT_OCEANFLOOR
      || victim->in_room->sector_type == SECT_UNDERWATER
      || victim->in_room->sector_type == SECT_WATER_SWIM || victim->in_room->sector_type == SECT_WATER_NOSWIM)
      act(AT_BLOOD, "$n's blood slowly clouds the surrounding water.", victim, NULL, NULL, TO_ROOM);
   else if (victim->in_room->sector_type == SECT_AIR)
      act(AT_BLOOD, "$n's blood sprays wildly through the air.", victim, NULL, NULL, TO_ROOM);
   else
      make_blood(victim);
      
   if (!IS_NPC(ch) && !ch->fighting)
   {
      for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
      {
         if (!IN_SAME_ROOM(gch, ch) || gch == victim)
            continue;
         if (gch->fighting && gch->fighting->who && gch->fighting->who == ch)
         {
            set_fighting(ch, gch);
            break;
         }
      }
   }

   if (IS_NPC(victim))
   {
      victim->pIndexData->killed++;
      extract_char(victim, TRUE);
      victim = NULL;
      return;
   }
   if (xIS_SET(ch->in_room->room_flags, ROOM_PERMDEATH) && sysdata.resetgame && !IS_IMMORTAL(victim))
   {
      char dirname[MSL];
      char sname[MSL];
      ROOM_INDEX_DATA *proom;
         
      sprintf(sname, victim->name);
      act(AT_RED, "$n has fallen in a Permdeath battle.", victim, NULL, NULL, TO_MUD);
      send_to_char("You died in a duel or a permdeath arena battle, your pfile is being deleted.\n\r", victim);      
      extract_char(victim, TRUE);
      sprintf(dirname, "%s%c/", PLAYER_DIR, LOWER(sname[0]));
      read_pfile2(dirname, sname);
      proom = get_room_index(ROOM_VNUM_TEMPLE);
      char_from_room(ch);
      char_to_room(ch, proom);
      save_char_obj(ch);
      do_look(ch, "auto");
      return;
   }

   set_char_color(AT_DIEMSG, victim);
   if (victim->pcdata->mdeaths + victim->pcdata->pdeaths < 3)
      do_help(victim, "new_death");
   else
      do_help(victim, "_DIEMSG_");

   extract_char(victim, FALSE);
   if (!victim)
   {
      bug("oops! raw_kill: extract_char destroyed pc char", 0);
      return;
   }
   while (victim->first_affect)
      affect_remove(victim, victim->first_affect);
   victim->affected_by = race_table[victim->race]->affected;
   victim->resistant = 0;
   victim->susceptible = 0;
   victim->immune = 0;
   victim->attacks = race_table[victim->race]->attacks;
   victim->defenses = race_table[victim->race]->defenses;
   victim->mod_str = 0;
   victim->mod_dex = 0;
   victim->mod_wis = 0;
   victim->mod_int = 0;
   victim->mod_con = 0;
   victim->mod_cha = 0;
   victim->mod_lck = 0;
   victim->damroll = 0;
   victim->hitroll = 0;
   victim->mental_state = -10;
   victim->alignment = URANGE(-1000, victim->alignment, 1000);
/*  victim->alignment		= race_table[victim->race]->alignment;
-- switched lines just for now to prevent mortals from building up
days of bellyaching about their angelic or satanic humans becoming
neutral when they die given the difficulting of changing align */

   victim->saving_poison_death = race_table[victim->race]->saving_poison_death;
   victim->saving_wand = race_table[victim->race]->saving_wand;
   victim->saving_para_petri = race_table[victim->race]->saving_para_petri;
   victim->saving_breath = race_table[victim->race]->saving_breath;
   victim->saving_spell_staff = race_table[victim->race]->saving_spell_staff;
   victim->position = POS_RESTING;
   victim->hit = 1;
   victim->mana = 1;
   victim->move = 200;
   victim->con_rarm = 1;
   victim->con_larm = 1;
   victim->con_rleg = 1;
   victim->con_lleg = 1;

   /*
    * Pardon crimes...      -Thoric
    */
   if (xIS_SET(victim->act, PLR_ATTACKER) && !IS_NPC(ch))
   {
      xREMOVE_BIT(victim->act, PLR_ATTACKER);
      send_to_char("The gods have pardoned you for your mistakes.\n\r", victim);
   }
//    if ( xIS_SET( victim->act, PLR_KILLER) && !IS_NPC(ch) )
//    {
//      xREMOVE_BIT( victim->act, PLR_KILLER);
//      send_to_char("The gods have pardoned you for your murderous acts.\n\r",victim);
//    }
//    if ( xIS_SET( victim->act, PLR_THIEF) && !IS_NPC(ch) )
//    {
//      xREMOVE_BIT( victim->act, PLR_THIEF);
//      send_to_char("The gods have pardoned you for your thievery.\n\r",victim);
//    }
   victim->pcdata->condition[COND_FULL] = 12;
   victim->pcdata->condition[COND_THIRST] = 12;
   if (IS_VAMPIRE(victim))
      victim->pcdata->condition[COND_BLOODTHIRST] = (victim->level / 2);

   if (IS_SET(sysdata.save_flags, SV_DEATH))
      save_char_obj(victim);
   return;
}

void group_gain(CHAR_DATA * ch, CHAR_DATA * victim)
{
   CHAR_DATA *gch;
   CHAR_DATA *lch;
   int members;

   /*
    * Monsters don't get kill xp's or alignment changes.
    * Dying of mortal wounds or poison doesn't give xp to anyone!
    */
   if (IS_NPC(ch) || victim == ch)
      return;

   members = 0;
   for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
   {
      if (is_same_group(gch, ch))
         members++;
   }

   if (members == 0)
   {
      bug("Group_gain: members.", members);
      members = 1;
   }

   lch = ch->leader ? ch->leader : ch;

   for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
   {
      OBJ_DATA *obj;
      OBJ_DATA *obj_next;

      if (!is_same_group(gch, ch))
         continue;

      /*
         gch->alignment = align_compute( gch, victim );
       */

      for (obj = ch->first_carrying; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         if (obj->wear_loc == WEAR_NONE)
            continue;
      }
   }

   return;
}

/* The below is not used anymore -- Xerves 8/1/99 */
int align_compute(CHAR_DATA * gch, CHAR_DATA * victim)
{
   int align, newalign, divalign;

   align = gch->alignment - victim->alignment;

   /* slowed movement in good & evil ranges by a factor of 5, h */
   /* Added divalign to keep neutral chars shifting faster -- Blodkai */
   /* This is obviously gonna take a lot more thought */

   if (gch->alignment > -350 && gch->alignment < 350)
      divalign = 4;
   else
      divalign = 20;

   if (align > 500)
      newalign = UMIN(gch->alignment + (align - 500) / divalign, 1000);
   else if (align < -500)
      newalign = UMAX(gch->alignment + (align + 500) / divalign, -1000);
   else
      newalign = gch->alignment - (int) (gch->alignment / divalign);

   return 0;
}


/*
 * Calculate how much XP gch should gain for killing victim
 * Lots of redesigning for new exp system by Thoric
 */
/*
 * New calculations for a new exp system
 * March 6 1999 - Xerves
 */

 // A few more modifications for Remorts :-)
int xp_compute(CHAR_DATA * gch, CHAR_DATA * victim)
{   
   return 0;
}


/*
 * Revamped by Thoric to be more realistic
 * Added code to produce different messages based on weapon type - FB
 * Added better bug message so you can track down the bad dt's -Shaddai
 */
void new_dam_message(CHAR_DATA * ch, CHAR_DATA * victim, int dam, int dt, OBJ_DATA * obj, int spec, int limb)
{
   char buf1[256], buf2[256], buf3[256], lbuf[20], gbuf[20], sbuf[100], awbuf[100];
   char bugbuf[MSL];
   char bknowledgea[MSL];
   char bknowledgev[MSL];
   unsigned char bmessagec[180];
   unsigned char bmessager[180];
   unsigned char bmessagev[180];
   char ddesc[50];
   char dcolor[10];
   char dchar[10];
   char dtt[20];
   unsigned char c, d, e;
   int x, y, z;
   const char *vs;
   const char *vp;
   const char *attack;
   char vc[10];
   char cc[10];
   char rc[10];
   char punct;
   sh_int dampc;
   struct skill_type *skill = NULL;
   bool gcflag = FALSE;
   bool gvflag = FALSE;
   int d_index, w_index;
   int dpercent;
   ROOM_INDEX_DATA *was_in_room;
   OBJ_DATA *weapon, *shield, *aweapon;
   char wbuf[128];
   
   strcpy(sbuf, "");

   if (!dam)
      dampc = 0;
   else
      dampc = ((dam * 1000) / victim->max_hit) + (50 - ((victim->hit * 50) / victim->max_hit));
   dpercent = ((100 * dam) / victim->max_hit);
   
  if (limb == LM_RARM)
     sprintf(lbuf, "right arm");
  else if (limb == LM_LARM)
     sprintf(lbuf, "left arm");
  else if (limb == LM_RLEG)
     sprintf(lbuf, "right leg");
  else if (limb == LM_LLEG)
     sprintf(lbuf, "left leg");
  else if (limb == LM_HEAD)
     sprintf(lbuf, "head");
  else if (limb == LM_NECK)
     sprintf(lbuf, "neck");
  else if (limb == LM_BODY)
     sprintf(lbuf, "chest");
  else
     strcpy(lbuf, "");
     
  if (dt == TYPE_PROJECTILE)
     strcpy(gbuf, "pierce");
  else
  {   
     if (ch->grip == GRIP_BASH)
        sprintf(gbuf, "bash");
     else if (ch->grip == GRIP_SLASH)
        sprintf(gbuf, "slash");
     else    
        sprintf(gbuf, "stab");
   }
   if (ch->in_room != victim->in_room)
   {
      was_in_room = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, victim->in_room);
   }
   else
      was_in_room = NULL;
      
   /* Get the weapon index */
   /*  if ( dt == TYPE_HIT)
      {
      w_index = 0;
      }
      else
      if ( dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table)/sizeof(attack_table[0]) )
      {
      w_index = dt - TYPE_HIT;
      }
      else
      {
      sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.",
      dt, ch->name, ch->in_room->vnum );
      bug( bugbuf, 0);
      dt = TYPE_HIT;
      w_index = 0;
      } */
   w_index = 0;
   if (dt < TYPE_UNDEFINED || dt > TYPE_HIT)
   {
      sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.", dt, ch->name, ch->in_room->vnum);
      bug(bugbuf, 0);
      dt = TYPE_HIT;
   }
   /* get the damage index */
   if (dam == 0)
      d_index = 0;
   else if (dampc < 0)
      d_index = 1;
   else if (dampc <= 100)
      d_index = 1 + dampc / 10;
   else if (dampc <= 200)
      d_index = 11 + (dampc - 100) / 20;
   else if (dampc <= 900)
      d_index = 16 + (dampc - 200) / 100;
   else
      d_index = 23;
   
   /*   
   &w&w 0
   &w&W 1 - 3
   &w&z 4 - 6
   &w&Y 7 - 9
   &w&O 10 - 12
   &w&P 13 - 15
   &w&R 17 - 18
   &w&r 19 - 21
   
   ----- 1 - 21
   ***** 22 - 42
   >>>>> 43 - 63
   +++++ 64 - 84
   DDDDD 85 - 105
   KKKKK 106 - 126
   !!!!! 127 - 147 */
   
   if (dam <= 0)
      sprintf(ddesc, " &w&B[&c&w NODAM &w&B]");
   else
   {
      if (dam % 24 == 0)
         sprintf(dchar, "*******");
      else if (dam % 24 <= 3)
         sprintf(dchar, "-------");
      else if (dam % 24 <= 6)
         sprintf(dchar, "*------");
      else if (dam % 24 <= 9)
         sprintf(dchar, "**-----");
      else if (dam % 24 <= 12)
         sprintf(dchar, "***----");
      else if (dam % 24 <= 15)
         sprintf(dchar, "****---");
      else if (dam % 24 <= 18)
         sprintf(dchar, "*****--");
      else if (dam % 24 <= 21)
         sprintf(dchar, "******-");
      else if (dam % 24 <= 23)
         sprintf(dchar, "*******");
         
      if (dam <= 24)
         sprintf(dcolor, "&w&z");
      else if (dam <= 48)
         sprintf(dcolor, "&w&W");
      else if (dam <= 72)
         sprintf(dcolor, "&w&g");
      else if (dam <= 96)
         sprintf(dcolor, "&w&G");
      else if (dam <= 120)
         sprintf(dcolor, "&w&c");
      else if (dam <= 144)
         sprintf(dcolor, "&w&C");
      else if (dam <= 168)
         sprintf(dcolor, "&w&O");
      else if (dam <= 192)
         sprintf(dcolor, "&w&Y");
      else if (dam <= 216)
         sprintf(dcolor, "&w&p");
      else if (dam <= 240)
         sprintf(dcolor, "&w&P");
      else if (dam <= 264)
         sprintf(dcolor, "&w&R");
      else
         sprintf(dcolor, "&w&r");  
         
      sprintf(ddesc, " &w&B[%s%s&w&B]", dcolor, dchar);
   }

   if (spec == DM_COUNTER)
      sprintf(dtt, "[Counter] ");
   else if (spec == DM_CRITICAL)
      sprintf(dtt, "[CriTicAL] ");
   else if (spec == DM_DEATH)
   {
      sprintf(dtt, "[DeaTH] ");
   }
   else if (spec == DM_UNDEAD)
   {
        sprintf(dtt, "[Undead] ");
   }
   else if (spec == DM_BLOCK)
   {
      sprintf(dtt, "[BLOCKED] ");
   }
   else
      strcpy(dtt, "");
      
   if ((shield = get_eq_char(victim, WEAR_SHIELD)) != NULL)
   {
      sprintf(sbuf, "%s", shield->short_descr);
   }

   /* Lookup the damage message */
   vs = s_message_table[w_index][d_index];
   vp = p_message_table[w_index][d_index];


   punct = (dampc <= 30) ? '.' : '!';

   if (dam == 0 && (!IS_NPC(ch) && (IS_SET(ch->pcdata->flags, PCFLAG_GAG))))
      gcflag = TRUE;

   if (dam == 0 && (!IS_NPC(victim) && (IS_SET(victim->pcdata->flags, PCFLAG_GAG))))
      gvflag = TRUE;

   if (dt >= 0 && dt < top_sn)
      skill = skill_table[dt];
      
   sprintf(vc, "%s", char_color_str(AT_HITME, ch));
   sprintf(cc, "%s", char_color_str(AT_HIT, ch));
   sprintf(rc, "%s", char_color_str(AT_ACTION, ch));
   
   strcpy(bknowledgea, "");
   strcpy(bknowledgev, "");
   if (g_roll > 0)
   {
      if (!IS_NPC(ch) && MASTERED(ch, gsn_battle_knowledge) >= 1)
      {
         if (MASTERED(ch, gsn_battle_knowledge) >= 5)
         {
            if (xIS_SET(ch->act, PLR_NOTOHIT))
               sprintf(bknowledgea, " &w&P<&w&W%d&w&P>", dam);
            else
               sprintf(bknowledgea, " &w&P<&w&W%d&w&P>&w&R<&c&w%d|%d-%d r%d&w&R>", dam, g_tohit, g_lowtohit, g_hitohit, g_roll);
         }
         else if (MASTERED(ch, gsn_battle_knowledge) >= 3 && !xIS_SET(ch->act, PLR_NOTOHIT))
            sprintf(bknowledgea, " &w&R<&c&w%d|%d-%d r%d&w&R>", g_tohit, g_lowtohit, g_hitohit, g_roll);
         else if (!xIS_SET(ch->act, PLR_NOTOHIT))
            sprintf(bknowledgea, " &w&R<&w&W%d&w&R>", g_tohit);
         learn_from_success(ch, gsn_battle_knowledge, victim);
      }
      if (!IS_NPC(victim) && MASTERED(victim, gsn_battle_knowledge) >= 2)
      {
         if (MASTERED(victim, gsn_battle_knowledge) >= 6)
         {
            if (xIS_SET(victim->act, PLR_NOTOHIT))
               sprintf(bknowledgev, " &w&P<&w&W%d&w&P>", dam);
            else
               sprintf(bknowledgev, " &w&P<&w&W%d&w&P>&w&R<&c&w%d|%d-%d r%d&w&R>", dam, g_tohit, g_lowtohit, g_hitohit, g_roll);
         }
         else if (MASTERED(victim, gsn_battle_knowledge) >= 4 && !xIS_SET(victim->act, PLR_NOTOHIT))
            sprintf(bknowledgev, " &w&R<&c&w%d|%d-%d r%d&w&R>", g_tohit, g_lowtohit, g_hitohit, g_roll);
         else if (!xIS_SET(victim->act, PLR_NOTOHIT))
            sprintf(bknowledgev, " &w&R<&w&W%d&w&R>", g_tohit);
         learn_from_success(victim, gsn_battle_knowledge, ch);
      }
   }
   else
   {
      if (!IS_NPC(ch) && MASTERED(ch, gsn_battle_knowledge) >= 5)
      {
         if (MASTERED(ch, gsn_battle_knowledge) >= 5)
            sprintf(bknowledgea, " &w&P<&w&W%d&w&P>", dam);
         learn_from_success(ch, gsn_battle_knowledge, victim);
      }
      if (!IS_NPC(victim) && MASTERED(victim, gsn_battle_knowledge) >= 6)
      {
         if (MASTERED(victim, gsn_battle_knowledge) >= 6)
            sprintf(bknowledgev, " &w&P<&w&W%d&w&P>", dam);
         learn_from_success(victim, gsn_battle_knowledge, ch);
      }
   }   
   
   weapon = get_eq_char(ch, WEAR_WIELD);
   
   if (dt == TYPE_PROJECTILE)
      sprintf(wbuf, "projectile");
   else if (weapon)
      sprintf(wbuf, "%s", weapon->name);
   else
      sprintf(wbuf, "fist");
      
   aweapon = get_eq_char(victim, WEAR_WIELD);
   
   if (aweapon)
      sprintf(awbuf, "%s", aweapon->name);
   else
      sprintf(awbuf, "fist");
      
   if ((dt == gsn_backstab || dt == gsn_circle || dt == gsn_kick_back) && spec == DM_UNDEAD)
   {
      if(weapon)
      {
         if(!xIS_SET(weapon->extra_flags, ITEM_BLESS) && !xIS_SET(weapon->extra_flags, ITEM_SANCTIFIED))
         {
            sprintf(buf1, "%s$n's weapon passes right through $N.", dtt);
            sprintf(buf2, "%sYour weapon passes right through $N.", dtt);
            sprintf(buf3, "%s$n's weapon passes right through you.", dtt);
         }
      }
      else if(!weapon)
      {
         sprintf(buf1, "%s$n's fist passes right through $N.", dtt);
         sprintf(buf2, "%sYour fist passes right through $N.", dtt);
         sprintf(buf3, "%s$n's fist passes right through you.", dtt);
      }
   }
   else if (dt == TYPE_HIT || dt == TYPE_PROJECTILE)
   {
      if (spec == DM_UNDEAD)
      {
        if(weapon)
        {
            if(!xIS_SET(weapon->extra_flags, ITEM_BLESS) && !xIS_SET(weapon->extra_flags, ITEM_SANCTIFIED))
            {
                sprintf(buf1, "%s$n's weapon passes right through $N.", dtt);
                sprintf(buf2, "%sYour weapon passes right through $N.", dtt);
                sprintf(buf3, "%s$n's weapon passes right through you.", dtt);
            }
        }
        else if(!weapon)
        {
            sprintf(buf1, "%s$n's fist passes right through $N.", dtt);
            sprintf(buf2, "%sYour fist passes right through $N.", dtt);
            sprintf(buf3, "%s$n's fist passes right through you.", dtt);
        }
      }
      else if (dt != gsn_backstab && dt != gsn_circle && dt != gsn_kick_back)
      {
        if (spec == DM_DEATH)
        {
           vp = "___KILLS___";
           dpercent = 100;
        }
        //wbuf - Weapon buffer
        //lbuf - Limb buffer
        //gbuf - Grip buffer
        
        if (ch->grip == GRIP_BASH)
           x = 0;
        else if (ch->grip == GRIP_SLASH)
           x = 1;
        else
           x = 2;
           
        z = 0;
        if (dam < 1)
           z = 1; //miss
        if (spec == DM_BLOCK)
           z = 2;
        if (spec == DM_DEATH)
           z = 6;
        
        y = number_range(0, high_value[z][x]-1);
        
        //To the Char
        d = -1;
        for (c = 0; c < 100; c++)
        {
           if (battle_descriptions[z][x][y][c] == '$')
           {
              if (battle_descriptions[z][x][y][c+1] == 'B')
              {
                 e = 0;
                 for (;;)
                 {
                    if (lbuf[e] != '\0')
                    {
                       bmessagec[++d] = lbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'a')
              {
                 bmessagec[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'A')
              {
                 bmessagec[++d] = 'e';
                 bmessagec[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'o')
              {
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'O')
              {
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'b')
              {
                 e = 0;
                 if (sbuf[0] != '\0')
                 {
                    for (;;)
                    {
                       if (sbuf[e] != '\0')
                       {
                          bmessagec[++d] = sbuf[e++];
                       }
                       else
                          break;
                    }
                 }
                 else
                 {
                     bmessagec[++d] = 's';
                     bmessagec[++d] = 'h';
                     bmessagec[++d] = 'i';
                     bmessagec[++d] = 'e';
                     bmessagec[++d] = 'l';
                     bmessagec[++d] = 'd';
                 }
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'p')
              {
                 e = 0;
                 bmessagec[++d] = 'y';
                 bmessagec[++d] = 'o';
                 bmessagec[++d] = 'u';
                 bmessagec[++d] = 'r';
                 /*
                 for (;;)
                 {
                    if (attacker[e] != '\0')
                    {
                       bmessaget[++d] = attacker[e++];
                    }
                    else
                       break;
                 } */
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'P')
              {
                 e = 0;
                 bmessagec[++d] = '$';
                 bmessagec[++d] = 'N';
                 bmessagec[++d] = 39; // ASCII for the ' character
                 bmessagec[++d] = 's';
                 e = 0;
                 c++;
              }  
              if (battle_descriptions[z][x][y][c+1] == 'G')
              {
                 e = 0;
                 for (;;)
                 {
                    if (gbuf[e] != '\0')
                    {
                       bmessagec[++d] = gbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'W')
              {
                 e = 0;
                 for (;;)
                 {
                    if (wbuf[e] != '\0')
                    {
                       bmessagec[++d] = wbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'w')
              {
                 e = 0;
                 for (;;)
                 {
                    if (awbuf[e] != '\0')
                    {
                       bmessagec[++d] = awbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }  
              if (battle_descriptions[z][x][y][c+1] == 'e' || battle_descriptions[z][x][y][c+1] == 'E'
              ||  battle_descriptions[z][x][y][c+1] == 'm' || battle_descriptions[z][x][y][c+1] == 'M'
              ||  battle_descriptions[z][x][y][c+1] == 's' || battle_descriptions[z][x][y][c+1] == 'S'
              ||  battle_descriptions[z][x][y][c+1] == 'n' || battle_descriptions[z][x][y][c+1] == 'N')
              {
                 d++;
                 bmessagec[d] = battle_descriptions[z][x][y][c];
              }
                 
           }
           else
           {
              d++;
              bmessagec[d] = battle_descriptions[z][x][y][c];
              if (battle_descriptions[z][x][y][c] == '\0')
              {
                 break;
              }
           }
        }
        
        //To the Victim
        d = -1;
        for (c = 0; c < 100; c++)
        {
           if (battle_descriptions[z][x][y][c] == '$')
           {
              if (battle_descriptions[z][x][y][c+1] == 'B')
              {
                 e = 0;
                 for (;;)
                 {
                    if (lbuf[e] != '\0')
                    {
                       bmessagev[++d] = lbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'a')
                 c++;
              if (battle_descriptions[z][x][y][c+1] == 'A') 
                 c++;
              if (battle_descriptions[z][x][y][c+1] == 'o')
              {
                 bmessagev[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'O')
              {
                 bmessagev[++d] = 'e';
                 bmessagev[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'b')
              {
                 e = 0;
                 if (sbuf[0] != '\0')
                 {
                    for (;;)
                    {
                       if (sbuf[e] != '\0')
                       {
                          bmessagev[++d] = sbuf[e++];
                       }
                       else
                          break;
                    }
                 }
                 else
                 {
                     bmessagev[++d] = 's';
                     bmessagev[++d] = 'h';
                     bmessagev[++d] = 'i';
                     bmessagev[++d] = 'e';
                     bmessagev[++d] = 'l';
                     bmessagev[++d] = 'd';
                 }
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'p')
              {
                 e = 0;
                 bmessagev[++d] = '$';
                 bmessagev[++d] = 'n';
                 bmessagev[++d] = 39; // ASCII for the ' character
                 bmessagev[++d] = 's';
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'P')
              {
                 e = 0;
                 bmessagev[++d] = 'y';
                 bmessagev[++d] = 'o';
                 bmessagev[++d] = 'u';
                 bmessagev[++d] = 'r';
                 /*
                 for (;;)
                 {
                    if (attackee[e] != '\0')
                    {
                       bmessaget[++d] = attackee[e++];
                    }
                    else
                       break;
                 }
                 bmessaget[++d] = 39; // ASCII for the ' character
                 bmessaget[++d] = 's';
                 */
                 e = 0;
                 c++;
              }  
              if (battle_descriptions[z][x][y][c+1] == 'G')
              {
                 e = 0;
                 for (;;)
                 {
                    if (gbuf[e] != '\0')
                    {
                       bmessagev[++d] = gbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'W')
              {
                 e = 0;
                 for (;;)
                 {
                    if (wbuf[e] != '\0')
                    {
                       bmessagev[++d] = wbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'w')
              {
                 e = 0;
                 for (;;)
                 {
                    if (awbuf[e] != '\0')
                    {
                       bmessagev[++d] = awbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }  
              if (battle_descriptions[z][x][y][c+1] == 'e' || battle_descriptions[z][x][y][c+1] == 'E'
              ||  battle_descriptions[z][x][y][c+1] == 'm' || battle_descriptions[z][x][y][c+1] == 'M'
              ||  battle_descriptions[z][x][y][c+1] == 's' || battle_descriptions[z][x][y][c+1] == 'S'
              ||  battle_descriptions[z][x][y][c+1] == 'n' || battle_descriptions[z][x][y][c+1] == 'N')
              {
                 d++;
                 bmessagev[d] = battle_descriptions[z][x][y][c];
              } 
           }
           else
           {
              d++;
              bmessagev[d] = battle_descriptions[z][x][y][c];
              if (battle_descriptions[z][x][y][c] == '\0')
              {
                 break;
              }
           }
        }
        
        //To the Room
        d = -1;
        for (c = 0; c < 100; c++)
        {
           if (battle_descriptions[z][x][y][c] == '$')
           {
              if (battle_descriptions[z][x][y][c+1] == 'B')
              {
                 e = 0;
                 for (;;)
                 {
                    if (lbuf[e] != '\0')
                    {
                       bmessager[++d] = lbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'a')
              {
                 bmessager[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'A')
              {
                 bmessager[++d] = 'e';
                 bmessager[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'o')
              {
                 bmessager[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'O')
              {
                 bmessager[++d] = 'e';
                 bmessager[++d] = 's';
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'b')
              {
                 e = 0;
                 if (sbuf)
                 {
                    for (;;)
                    {
                       if (sbuf[e] != '\0')
                       {
                          bmessager[++d] = sbuf[e++];
                       }
                       else
                          break;
                    }
                 }
                 else
                 {
                     bmessager[++d] = 's';
                     bmessager[++d] = 'h';
                     bmessager[++d] = 'i';
                     bmessager[++d] = 'e';
                     bmessager[++d] = 'l';
                     bmessager[++d] = 'd';
                 }
                 e = 0;
                 c++;
              }  
              if (battle_descriptions[z][x][y][c+1] == 'p')
              {
                 e = 0;
                 bmessager[++d] = '$';
                 bmessager[++d] = 'n';
                 bmessager[++d] = 39; // ASCII for the ' character
                 bmessager[++d] = 's';
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'P')
              {
                 e = 0;
                 bmessager[++d] = '$';
                 bmessager[++d] = 'N';
                 bmessager[++d] = 39; // ASCII for the ' character
                 bmessager[++d] = 's';
                 e = 0;
                 c++;
              }  
              if (battle_descriptions[z][x][y][c+1] == 'G')
              {
                 e = 0;
                 for (;;)
                 {
                    if (gbuf[e] != '\0')
                    {
                       bmessager[++d] = gbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              } 
              if (battle_descriptions[z][x][y][c+1] == 'W')
              {
                 e = 0;
                 for (;;)
                 {
                    if (wbuf[e] != '\0')
                    {
                       bmessager[++d] = wbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }
              if (battle_descriptions[z][x][y][c+1] == 'w')
              {
                 e = 0;
                 for (;;)
                 {
                    if (awbuf[e] != '\0')
                    {
                       bmessager[++d] = awbuf[e++];
                    }
                    else
                       break;
                 }
                 e = 0;
                 c++;
              }  
              if (battle_descriptions[z][x][y][c+1] == 'e' || battle_descriptions[z][x][y][c+1] == 'E'
              ||  battle_descriptions[z][x][y][c+1] == 'm' || battle_descriptions[z][x][y][c+1] == 'M'
              ||  battle_descriptions[z][x][y][c+1] == 's' || battle_descriptions[z][x][y][c+1] == 'S'
              ||  battle_descriptions[z][x][y][c+1] == 'n' || battle_descriptions[z][x][y][c+1] == 'N')
              {
                 d++;
                 bmessager[d] = battle_descriptions[z][x][y][c];
              } 
           }
           else
           {
              d++;
              bmessager[d] = battle_descriptions[z][x][y][c];
              if (battle_descriptions[z][x][y][c] == '\0')
              {
                 break;
              }
           }
        }
        bmessager[0] = UPPER(bmessager[0]);
        bmessagec[0] = UPPER(bmessagec[0]);
        bmessagev[0] = UPPER(bmessagev[0]);
        
        sprintf(buf1, "%s%s %s", dtt,bmessager, ddesc); //to notvict (room)
        sprintf(buf2, "%s%s %s%s", dtt,bmessagec, ddesc, bknowledgea); //to char
        sprintf(buf3, "%s%s %s%s", dtt,bmessagev, ddesc, bknowledgev); //to char
      }
   }
   else
   {
      if (skill)
      {
         attack = skill->noun_damage;
         if (dam == 0)
         {
            bool found = FALSE;

            if (skill->miss_char && skill->miss_char[0] != '\0')
            {
               act(AT_HIT, skill->miss_char, ch, NULL, victim, TO_CHAR);
               found = TRUE;
            }
            if (skill->miss_vict && skill->miss_vict[0] != '\0')
            {
               act(AT_HITME, skill->miss_vict, ch, NULL, victim, TO_VICT);
               found = TRUE;
            }
            if (skill->miss_room && skill->miss_room[0] != '\0')
            {
               if (strcmp(skill->miss_room, "supress"))
                  act(AT_ACTION, skill->miss_room, ch, NULL, victim, TO_NOTVICT);
               found = TRUE;
            }
            if (found) /* miss message already sent */
            {
               if (was_in_room)
               {
                  char_from_room(ch);
                  char_to_room(ch, was_in_room);
               }
               return;
            }
         }
         else
         {
            bool found = FALSE;
            
            if (skill->hit_char && skill->hit_char[0] != '\0')
            {
               sprintf(buf2, "%s %s%s", skill->hit_char, ddesc, bknowledgea);
               act(AT_HIT, buf2, ch, NULL, victim, TO_CHAR);
               found = TRUE;
            }
            if (skill->hit_vict && skill->hit_vict[0] != '\0')
            { 
               sprintf(buf3, "%s %s%s", skill->hit_vict, ddesc, bknowledgev);
               act(AT_HITME, buf3, ch, NULL, victim, TO_VICT);
               found = TRUE;
            }
            if (skill->hit_room && skill->hit_room[0] != '\0')
            {
               sprintf(buf1, "%s %s", skill->hit_room, ddesc);
               act(AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT);
               found = TRUE;
            }
            if (found) /* miss message already sent */
            {
               if (was_in_room)
               {
                  char_from_room(ch);
                  char_to_room(ch, was_in_room);
               }
               return;
            }
         }
      }
      else if (dt >= TYPE_UNDEFINED || (dt >= TYPE_HIT && dt < TYPE_HIT + sizeof(attack_table) / sizeof(attack_table[0])))
      {
         if (obj)
            attack = obj->short_descr;
         else
            attack = attack_table[0];
      }
      else
      {
         sprintf(bugbuf, "Dam_message: bad dt %d from %s in %d.", dt, ch->name, ch->in_room->vnum);
         bug(bugbuf, 0);
         dt = TYPE_HIT;
         attack = attack_table[0];
      }
      if (dt == gsn_unsheath)
         attack = "UnShEaThEd AtTaCk";
         
      if (spec == DM_DEATH)
      {
         vp = "___KILLS___";
         vs = "___KILL___";
         dpercent = 100;
      }
      
      sprintf(buf1, "%s$n's %s %s $N%c %s", dtt, attack, vp, punct, ddesc);
      sprintf(buf2, "%sYour %s %s $N%c %s%s", dtt, attack, vp, punct, ddesc, bknowledgea);
      sprintf(buf3, "%s$n's %s %s you%c %s%s", dtt, attack, vp, punct, ddesc, bknowledgev);
   }


   if (dam > 0)
      act(AT_ACTION, buf1, ch, NULL, victim, TO_NOTVICT);
   if (!gcflag)
      act(AT_HIT, buf2, ch, NULL, victim, TO_CHAR);
   if (!gvflag)
      act(AT_HITME, buf3, ch, NULL, victim, TO_VICT);

   if (was_in_room)
   {
      char_from_room(ch);
      char_to_room(ch, was_in_room);
   }
   return;
}

//Used to step out of battle if your enemies cannot move
void do_stepback(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   int cnt = 0;
   
   if (check_npc(ch))
      return;
   
   if (!ch->fighting)
   {
      send_to_char("You need to be fighting to use this command.\n\r", ch);
      return;
   }
   //If affected by web or paralysis (stun) allow a step out of battle
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (!IN_SAME_ROOM(ch, victim))
         continue;
      if (IS_AFFECTED(victim, AFF_WEB) || IS_AFFECTED(victim, AFF_SNARE))  
      {
         stop_fighting(victim, TRUE);
         cnt++;
         continue;
      }
      if (IS_AFFECTED(victim, AFF_PARALYSIS))
      {
         stop_fighting(victim, TRUE);
         cnt++;
         continue;
      }
   }   
   if (!ch->fighting)
   {
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if (!IN_SAME_ROOM(ch, victim))
            continue;  
         if (victim->fighting && victim->fighting->who && victim->fighting->who == ch)
         {
            set_fighting(ch, victim);
            break;
         }
      }
   }
   if (cnt > 0 && ch->fighting)
   {
      send_to_char("You step back out of battle but you still have a few enemies.\n\r", ch);
      return;
   }
   else if (cnt > 0 && !ch->fighting)
   {
      send_to_char("You step back out of battle and no one gives chase.\n\r", ch);
      return;
   }
   else
   {
      send_to_char("You step back out of battle but everyone gives chase.\n\r", ch);
      return;
   }
}

int check_powerlevel(CHAR_DATA *ch, CHAR_DATA *victim)
{
   INTRO_DATA *intro;
   
   if (IS_NPC(ch) || IS_NPC(victim))
      return FALSE;
   if (in_arena(ch))
      return FALSE;
  if (in_arena(victim))
      return FALSE;
   for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
   {
      if (intro->pid == victim->pcdata->pid)
         if (IS_SET(intro->flags, INTRO_MYTHIEF) || IS_SET(intro->flags, INTRO_MYATTACKER)
         ||  IS_SET(intro->flags, INTRO_MYKILLER))
            return FALSE;
   }
   if (get_wear_hidden_cloak(victim))
      return FALSE;
   if (ch->pcdata->hometown < 2)
      return TRUE;
   if (victim->pcdata->hometown < 2)
      return TRUE;
   if (kingdom_table[ch->pcdata->hometown]->peace[victim->pcdata->hometown] > 0)
      return TRUE;
      
   return FALSE;
}
void do_attack(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *shield;
   int limb;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Attack whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   
   if (ch->position == POS_RIDING)
   {
      send_to_char("You need to stop riding if you want to fight.\n\r", ch);
      return;
   }
   
   if (ch->rider)
   {
      send_to_char("You have someone on your back, hard to fight like that you know!.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Not wise to attack yourself.\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;

   if (IS_AFFECTED(ch, AFF_CHARM))
   {
      if (ch->master == victim)
      {
         act(AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR);
         return;
      }
   }

   if (ch->position == POS_FIGHTING
      || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
   {
      if( ch->fight_timer > 0)
      {
        send_to_char("You do the best you can!\n\r", ch);
        return;
      }
   }
   
   if ((shield = get_eq_char(ch, WEAR_SHIELD)) != NULL && IS_OBJ_STAT(shield, ITEM_TWOHANDED))
   {
      send_to_char("You cannot attack while holding a two-handed shield.\n\r", ch);
      return;
   }
   
   if (ch->position <= POS_STUNNED)
   {
      send_to_char("Sorry, kind of hard to do that when you cannot move.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && xIS_SET(ch->act, PLR_NICE))
   {
      send_to_char("You feel too nice to do that!\n\r", ch);
      return;
   }
/*
    if ( !IS_NPC( victim ) && xIS_SET(victim->act, PLR_PK ) )
*/
   if (!IS_NPC(victim) && !IS_NPC(ch) && get_trust(ch) >= LEVEL_IMMORTAL)
   {
      sprintf(log_buf, "%s: immortal attempting player murder of %s.", ch->name, victim->name);
      log_string_plus(log_buf, LOG_NORMAL, LEVEL_ADMIN);
      send_to_char("Sorry, you cannot murder players.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && !IS_NPC(ch) && get_trust(victim) >= LEVEL_IMMORTAL)
   {
      sprintf(log_buf, "%s: player attempted immortal murder of %s.", ch->name, victim->name);
      log_string_plus(log_buf, LOG_NORMAL, LEVEL_ADMIN);
      send_to_char("Sorry, you cannot murder immortals.\n\r", ch);
      return;
   }
   /*if (!IS_NPC(victim))
   {
      sprintf(log_buf, "%s: murder %s.", ch->name, victim->name);
      log_string_plus(log_buf, LOG_NORMAL, ch->level);
   }*/
   
   if (sysdata.resetgame && check_powerlevel(ch, victim) 
   &&  (get_player_statlevel(ch) > get_player_statlevel(victim) + 5))
   {
      argument = one_argument(argument, arg);
      if (str_cmp(arg, "lowbie"))
      {
         send_to_char("Your Power Level is too high for your target, you will need to type:\n\r", ch);
         send_to_char("kill <target> lowbie [limb] if you want to do such a thing.\n\r", ch);
         return;
      }
      send_to_char("You will be assigned twink points for your kill, if you reach 100 you will be DELETED!\n\r", ch);
   }

   check_illegal_pk(ch, victim);
   check_attacker(ch, victim);
   if (argument[0] == '\0')
   {
      one_hit(ch, victim, TYPE_HIT, LM_BODY);
   }
   else
   {
      if (!str_cmp(argument, "rarm"))
         limb = LM_RARM;
      else if (!str_cmp(argument, "larm"))
         limb = LM_LARM;
      else if (!str_cmp(argument, "rleg"))
         limb = LM_RLEG;
      else if (!str_cmp(argument, "lleg"))
         limb = LM_LLEG;
      else if (!str_cmp(argument, "body"))
         limb = LM_BODY;
      else if (!str_cmp(argument, "head"))
         limb = LM_HEAD;
      else if (!str_cmp(argument, "neck"))
         limb = LM_NECK;
      else
      {
         send_to_char("Need to choose a limb: rarm, larm, rleg, lleg, body, head, neck", ch);
         return;
      }  
      one_hit(ch, victim, TYPE_HIT, limb);
   }
   
   if ( ch && char_died(ch))          
        stop_fighting(ch, TRUE);
   
   return;
}
int npcrace_agi(int race)
{
   switch(race)
   {
      case 6: return 6;  //half-ogre
      case 7: return 5;  //half-orc
      case 8: return 5;  //half-troll
      case 9: return 4;  //half-elf
      case 10: return 3; //gith
      case 11: return 3; //drow
      case 12: return 3; //sea-elf
      case 13: return 5; //lizardman
      case 14: return 2; //gnome
      case 15: return 4; //gnoll
      case 16: return 4; //gnome
      case 17: return 4; //goblin
      case 18: return 4; //kobold
      case 19: return 5; //troll
      case 20: return 5; //minotaur
      case 21: return 4; //skeleton
   }
   return 0;
}
       	
int add_agi_found(CHAR_DATA *ch)
{
   CHAR_DATA *victim;
   BUYKMOB_DATA *kmob;

   if (!IS_NPC(ch))
      return 0;
   if (!IN_WILDERNESS(ch))
      return 0;
   if (!xIS_SET(ch->act, ACT_MILITARY))
      return 0;
      
   for (kmob = first_buykmob; kmob; kmob = kmob->next)
   {
      if (kmob->vnum == ch->pIndexData->vnum)
         if (xIS_SET(kmob->flags, KMOB_SOLDIERADDAGI))  
            break;
   }
   if (!kmob)
      return 0;
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (!xIS_SET(victim->act, ACT_MILITARY))
         continue;
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (kmob->vnum == victim->pIndexData->vnum)
            if (xIS_SET(kmob->flags, KMOB_SOLDIERAGI))  
               if (abs(ch->coord->x - victim->coord->x) <= 10 && abs(ch->coord->y - victim->coord->y) <= 10
               && ch->m4 == victim->m4)
                  break;
      }
      if (kmob)
         return 1;
   }
   return 0;
} 
      

int get_btimer(CHAR_DATA *ch, int sn, CHAR_DATA *victim)
{
   int timer = 1;
   int mweap = 0;
   int tweight = 0;
   int agi;
   OBJ_DATA *armor;
   OBJ_DATA *shield;
   OBJ_DATA *weapon = NULL;
   OBJ_DATA *dual = NULL;
   int wtype;
   int percent = 0;
   int cweight = 0;
   int combatsn = 0;
   int extra;
   
   if (sn == gsn_pincer || sn == gsn_backstab || sn == gsn_circle || sn == gsn_kick_back || sn == gsn_powerslice
   ||  sn == gsn_drive )
      combatsn = 1;
   
   if (sn > -1 && sn != 1000 && !combatsn)  //1000 is for firing a weapon
   {
      timer = skill_table[sn]->beats; 
   }
   else
   {
      if (ch->race < LAST_H_RACE)
      {
         if (ch->race < MAX_RACE)
            mweap = race_table[ch->race]->weaponmin;
         else
            mweap = npcrace_agi(ch->race);
         weapon = get_eq_char(ch, WEAR_WIELD);
         dual = get_eq_char(ch, WEAR_DUAL_WIELD);
      }
   
      if (weapon == NULL)
      {
         if (!IS_NPC(ch))
            timer = 3;
         else
         {
            timer = 6;
         }
      }      
      else
      {
         timer = 3 + (weapon->value[3] - mweap);
         if (dual)
         {
            cweight += dual->weight;
            percent = 95 - (POINT_LEVEL(LEARNED(ch, gsn_dual_wield), MASTERED(ch, gsn_dual_wield))/2.5);
            timer = timer * percent / 100;
            if ((((3 + (weapon->value[3] - mweap)) * percent) % 100) >= 50)
               timer++;
         }
         cweight += weapon->weight;
         if (cweight > str_app[get_curr_str(ch)].wield)
         {
            timer += UMAX(0, (cweight - str_app[get_curr_str(ch)].wield) * 3 / 2);
         }
      }
      if (sn == 1000) //firing weapons
         timer += 2;
   }
   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_PARRY))
   {
      if (MASTERED(ch, gsn_parry) == 2)
         timer += 1;
      else if (MASTERED(ch, gsn_parry) >= 3)
         timer += 0;
      else
         timer += 2;
   }
   armor = get_eq_char(ch, WEAR_BODY);
   if (armor)
      tweight += armor->weight;
   armor = get_eq_char(ch, WEAR_HEAD); 
   if (armor)
      tweight += armor->weight;
   armor = get_eq_char(ch, WEAR_NECK);  
   if (armor)
      tweight += armor->weight;
   armor = get_eq_char(ch, WEAR_ARM_R);
   if (armor)
      tweight += armor->weight;
   armor = get_eq_char(ch, WEAR_ARM_L); 
   if (armor)
      tweight += armor->weight;
   armor = get_eq_char(ch, WEAR_LEG_R);
   if (armor)
      tweight += armor->weight;
   armor = get_eq_char(ch, WEAR_LEG_L);  
   if (armor)
      tweight += armor->weight;
      
   if ((shield = get_eq_char(ch, WEAR_SHIELD)) != NULL)
   {
      int mshield;
      mshield = MASTERED(ch, gsn_shieldblock);
      if (mshield == 2)
         mshield = 1;
      else if (mshield == 3)
         mshield = 2;
      else if (mshield == 4)
         mshield = 4;
      timer += UMAX(0, shield->value[3]-mshield);
   }
      
   tweight = tweight - str_app[get_curr_str(ch)].weight;
      
   agi = get_curr_agi(ch);
   if (add_agi_found(ch))
      agi += 10;
   //Two things for dual wield, a little much...
   ch->agi_meter = 1000 - (number_range(agi*7/2, agi*5));
   if (tweight > 0) 
   {
      tweight = tweight * 100 / str_app[get_curr_str(ch)].weight * 7;
      ch->agi_meter += number_range(tweight*7/10, tweight*1);
   }   
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_combatart] > 0 && weapon && (sn == -1 || sn == 1000))
   {
      ch->agi_meter -= URANGE(6, POINT_LEVEL(LEARNED(ch, gsn_combatart), MASTERED(ch, gsn_combatart))*2/3, 50);
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_attack_frenzy] > 0 && weapon && (sn == -1 || sn == 1000))
   {
      ch->agi_meter -= URANGE(15, POINT_LEVEL(LEARNED(ch, gsn_attack_frenzy), MASTERED(ch, gsn_attack_frenzy))*4/3, 100);
      if (victim)
         learn_from_success(ch, gsn_attack_frenzy, victim);
   }
   if (sn == 1000)
      wtype = wielding_skill_weapon(ch, 1);
   else if (sn == -1)
      wtype = wielding_skill_weapon(ch, 0);
   else
      wtype = 0;
   if (!IS_NPC(ch) && wtype)
   {
      int wskill = wielding_skill_weapon(ch, 1);
      //No gains here, there is also the damage mod in one_hit
      if (wskill == 1)
      {
         if (ch->pcdata->learned[gsn_weapon_axe] > 0)
         {
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_axe), MASTERED(ch, gsn_weapon_axe))*2/5, 25);
         }
      }
      if (wskill == 2)
      {
         if (ch->pcdata->learned[gsn_weapon_sword] > 0)
         {
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_sword), MASTERED(ch, gsn_weapon_sword))*2/5, 25);
         }
      }
      if (wskill == 3)
      {
         if (ch->pcdata->learned[gsn_weapon_polearm] > 0)
         {
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_polearm), MASTERED(ch, gsn_weapon_polearm))*2/5, 25);
         }
      }
      if (wskill == 4)
      {
         if (ch->pcdata->learned[gsn_weapon_blunt] > 0)
         {
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_blunt), MASTERED(ch, gsn_weapon_blunt))*2/5, 25);
         }
      }
      if (wskill == 5)
      {
         if (ch->pcdata->learned[gsn_weapon_staff] > 0)
         {
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_staff), MASTERED(ch, gsn_weapon_staff))*2/5, 25);
         }
      }
      if (wskill == 7)
      {
         if (ch->pcdata->learned[gsn_weapon_dagger] > 0)
         {
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_dagger), MASTERED(ch, gsn_weapon_dagger))*2/5, 25);
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_daggerstudy), MASTERED(ch, gsn_weapon_daggerstudy))*3/5, 50);
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_daggerstrike), MASTERED(ch, gsn_weapon_daggerstrike))*2/5, 25);
         }
      }
      if (wskill == 6)
      {
         if (ch->pcdata->learned[gsn_weapon_projectile] > 0)
         {
            ch->agi_meter -= URANGE(3, POINT_LEVEL(LEARNED(ch, gsn_weapon_projectile), MASTERED(ch, gsn_weapon_projectile))*2/5, 25);
         }
      }
   }
   if (weapon == NULL && IS_NPC(ch))
   {
      if (IS_NPC(ch) && agi >= 50) //50
         timer -= 1;
      if (IS_NPC(ch) && agi >= 70) //70
         timer -= 1;
      if (IS_NPC(ch) && agi >= 90) //90
         timer -= 1;
   }
   if (weapon && shield && !IS_NPC(ch))
   {
      if (IS_OBJ_STAT(weapon, ITEM_TWOHANDED))
      {
         if (ch->pcdata->learned[gsn_inhuman_strength] <= 0)
            bug("%s is wearing a two-handed weapon and a shield!", ch->name);
         else if (MASTERED(ch, gsn_inhuman_strength) == 1)
            timer+= 4;
         else if (MASTERED(ch, gsn_inhuman_strength) == 2)  
            timer+= 3;
         else if (MASTERED(ch, gsn_inhuman_strength) == 3)  
            timer+= 2;
         else if (MASTERED(ch, gsn_inhuman_strength) == 4)  
            timer+= 0;
            
         if (victim)
            learn_from_success(ch, gsn_inhuman_strength, victim);
      }
   }    
   extra = (timer * ch->agi_meter / 10) % 100;  
   timer = timer * ch->agi_meter / 1000;       
   if (number_range(1, 100) <= extra)
      timer++;
   if (sn == gsn_pincer) //a double attack essentially
   {
      int plevel = POINT_LEVEL(LEARNED(ch, gsn_pincer), MASTERED(ch, gsn_pincer));
      timer = timer * (200 - (plevel/5)) /100;
   }
   //sorry cannot go below the minimum beats on a spell....
   if (sn > -1 && sn != 1000 && !combatsn)  //1000 is for firing a weapon
   {
      if (skill_table[sn]->beats > timer)
         return skill_table[sn]->beats;
   }
   if (sn == gsn_pincer && timer < 2)
      timer = 2;
   if (IS_AFFECTED(ch, AFF_NERVEPINCH))
      timer*=2;
   if (!ch->fighting)
      WAIT_STATE(ch, timer*2);
   return URANGE(1, timer, 24);
}
      

/*
 * Check to see if the player is in an "Arena".
 */
bool in_arena(CHAR_DATA * ch)
{
   if (xIS_SET(ch->in_room->room_flags, ROOM_ARENA))
      return TRUE;
   if (!str_cmp(ch->in_room->area->filename, "arena.are"))
      return TRUE;

   return FALSE;
}

bool check_illegal_pk(CHAR_DATA * ch, CHAR_DATA * victim)
{
   char buf[MSL];
   char buf2[MSL];

   if (victim && !IS_NPC(victim) && !IS_NPC(ch) && victim->in_room)
   {
      if (((check_room_pk(ch) == 1) && !IN_ARENA(ch) && ch != victim) || (IS_IMMORTAL(ch) || IS_IMMORTAL(victim)))
      {
         if (IS_NPC(ch))
            sprintf(buf, " (%s)", ch->name);
         if (IS_NPC(victim))
            sprintf(buf2, " (%s)", victim->name);

         sprintf(log_buf, "&p%s on %s%s in &W***&rILLEGAL PKILL&W*** &pattempt at %d",
            (lastplayercmd), (IS_NPC(victim) ? victim->short_descr : victim->name), (IS_NPC(victim) ? buf2 : ""), victim->in_room->vnum);
         last_pkroom = victim->in_room->vnum;
         log_string(log_buf);
         to_channel(log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
         return TRUE;
      }
   }
   return FALSE;
}

int bad_flee_sector(CHAR_DATA *ch, int x, int y)
{
   OBJ_DATA *obj;
   if (IS_MOUNTAIN(map_sector[ch->map][x][y]))
   {
      if (MASTERED(ch, gsn_mountain_climb) < 1)
      {
         if ((obj = get_objtype(ch, ITEM_MCLIMB)) == NULL)
         {
            return TRUE;
         }
      }
   }
   if (IS_NOSWIM(map_sector[ch->map][x][y]))
   {
      if ((ch->mount && !IS_FLOATING(ch->mount)) || !IS_FLOATING(ch))
      {
         /*
          * Look for a boat.
          * We can use the boat obj for a more detailed description.
          */
         if ((obj = get_objtype(ch, ITEM_BOAT)) == NULL)
         {
            return TRUE;
         }
      }
   }
   return FALSE;
}

int get_mapmovement_value(CHAR_DATA *ch, int dir, int type)
{
   if (type == 0) //x
   {
      if (dir == 1) //e
         return +1;
      if (dir == 3) //w
         return -1;
      if (dir == 6) //ne
         return +1;
      if (dir == 7) //nw
         return -1;
      if (dir == 8) //se
         return +1;
      if (dir == 9) //sw
         return -1;
      return 0;
   }
   else  //y
   {
      if (dir == 0) //n
         return -1;
      if (dir == 2) //s
         return +1;
      if (dir == 6) //ne
         return -1;
      if (dir == 7) //nw
         return -1;
      if (dir == 8) //se
         return +1;
      if (dir == 9) //sw
         return +1;
      return 0;   
   }
   return 0;
}
   
void adjust_flee_timers(CHAR_DATA *ch)
{
   CHAR_DATA *victim;
   
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (victim->fighting && victim->fighting->who == ch)
      {
         victim->fight_timer = 6;
         WAIT_STATE(victim, 12);
      }
   }
}
void do_flee(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_in;
   ROOM_INDEX_DATA *now_in;
   CHAR_DATA *foch;
   CHAR_DATA *wf;
   int attempt;
   sh_int door;
   int oldmap;
   int x = 0;
   int broken = 0;
   sh_int addx = 0, addy = 0;
   EXIT_DATA *pexit = NULL;
   AFFECT_DATA af;
   int escapism = 0;
   int retreatchance = 0;
   int retreat = 0;
   int vanish = 0;
   int oldx = 0, oldy = 0, kingdom = 0;
   
   oldx = ch->coord->x;
   oldy = ch->coord->y;

   oldmap = ch->fightm;

   if (!who_fighting(ch))
   {
      if (ch->position == POS_FIGHTING
      || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
      {
         if (ch->mount)
            ch->position = POS_MOUNTED;
         else
            ch->position = POS_STANDING;
      } 
      send_to_char("You aren't fighting anyone.\n\r", ch);
      return;
   }
   if (IS_AFFECTED(ch, AFF_BERSERK))
   {
      send_to_char("You aren't thinking very clearly...\n\r", ch);
      return;
   }
   if (ch->move <= 0)
   {
      send_to_char("You're too exhausted to flee from combat!\n\r", ch);
      return;
   }
   /* No fleeing while more aggressive than standard or hurt. - Haus */
   if (!IS_NPC(ch) && ch->position < POS_FIGHTING && ch->position != POS_AGGRESSIVE)
   {
      send_to_char("Not right now ...\n\r", ch);
      return;
   }
   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }
   if (IS_NPC(ch) && ch->position <= POS_SLEEPING)
      return;
      
   if (ch->con_rleg == -1 && ch->con_lleg == -1)
   {
      send_to_char("It is impossible to flee when you have no legs.\n\r", ch);
      return;
   }
   //add broken/damaged legs mod, harder to flee if your legs aren't there or hurt
   broken = 0;
   if (ch->con_rleg <= -1)
      broken += 25;
   else if (ch->con_rleg <= 100)
      broken += 15;
   else if (ch->con_rleg <= 300)
      broken += 10;
   else if (ch->con_rleg <= 500)
      broken += 5;   
   
   if (ch->con_lleg <= -1)
      broken += 25;
   else if (ch->con_lleg <= 100)
      broken += 15;
   else if (ch->con_lleg <= 300)
      broken += 10;
   else if (ch->con_lleg <= 500)
      broken += 5;    
      
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_escapism] > 0)
   {
      escapism = POINT_LEVEL(GET_POINTS(ch, gsn_escapism, 0, 1), GET_MASTERY(ch, gsn_escapism, 0, 1));
      escapism = UMAX(5, escapism*2/5);
   }
   if (ch->fighting && IS_AFFECTED(ch->fighting->who, AFF_BLIND))
      escapism += 20;
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_vanish] > 0)
   {
      vanish = POINT_LEVEL(GET_POINTS(ch, gsn_vanish, 0, 1), GET_MASTERY(ch, gsn_vanish, 0, 1));   
      if ((60 + (vanish*2/3)) >= number_range(1, 100))
         vanish = TRUE;
      else
         vanish = FALSE;
   }

   if (argument[0] != '\0')
   {
      if (check_npc(ch))
         return;
         
      if (ch->pcdata->learned[gsn_retreat] > 0)
         retreatchance = POINT_LEVEL(GET_POINTS(ch, gsn_retreat, 0, 1), GET_MASTERY(ch, gsn_retreat, 0, 1));  
         
      if (ch->pcdata->learned[gsn_retreat] <= 0)
      {
         send_to_char("You can only specify a direction if you are knowledgeable in the retreat skill.\n\r", ch);
         return;
      }
      else if (number_range(1, 100) > 50+(retreatchance*3/4))
      {
         learn_from_failure(ch, gsn_retreat, who_fighting(ch));
         send_to_char("&R**********You attempt a planned retreat but you can only find a random exit.**********&w\n\r", ch);
      }
      else
      {
         int dir;
         if ((dir = get_new_dir(argument, 1)) == -1)
         {
            send_to_char("That is not a valid direction.\n\r", ch);
            return;
         }
         if (!IN_WILDERNESS(ch))
         {
            if ((pexit = get_exit(ch->in_room, dir)) == NULL
            || !pexit->to_room
            || IS_SET(pexit->exit_info, EX_NOFLEE)
            || (IS_SET(pexit->exit_info, EX_CLOSED)
            && !IS_AFFECTED(ch, AFF_PASS_DOOR)) || (IS_NPC(ch) && xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)))
            {
               send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
               return;
            }
            else
            {
               retreat = 1;
               learn_from_success(ch, gsn_retreat, who_fighting(ch));
            }
         }
         else
         {
            addx = get_mapmovement_value(ch, dir, 0);
            addy = get_mapmovement_value(ch, dir, 1);
            ch->coord->x += addx;
            ch->coord->y += addy;
            if (ch->coord->x < 1 || ch->coord->x > MAX_X)
            {
               ch->coord->x = oldx;
               ch->coord->y = oldy;
               send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
               return;
            }
            if (ch->coord->y < 1 || ch->coord->y > MAX_Y)
            {
               ch->coord->x = oldx;
               ch->coord->y = oldy;
               send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
               return;
            }
            if (sect_show[(int)map_sector[ch->map][ch->coord->x][ch->coord->y]].canpass == TRUE)
            {             
               kingdom = kingdom_sector[ch->map][ch->coord->x][ch->coord->y];
                          
               if (bad_flee_sector(ch, ch->coord->x, ch->coord->y)) //mountains and rivers and stuff
               {
                  ch->coord->x = oldx;
                  ch->coord->y = oldy;
                  send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
                  return;
               }
               
               if (IS_NPC(ch) && wIS_SET(ch, ROOM_NO_MOB))
               {
                  ch->coord->x = oldx;
                  ch->coord->y = oldy;
                  send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
                  return;
               }
               if (room_is_private_wilderness(ch, ch->in_room, ch->coord->x, ch->coord->y, ch->map))
               {
                  ch->coord->x = oldx;
                  ch->coord->y = oldy;
                  send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
                  return;
               }
               if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != kingdom) //don't want them fleeing into a blocked out town
               {
                  ch->coord->x = oldx;
                  ch->coord->y = oldy;
                  send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
                  return;
               }   
            }
            else
            {
               ch->coord->x = oldx;
               ch->coord->y = oldy;
               send_to_char("There is something blocking you from retreating in that direction.\n\r", ch);
               return;
            }
            retreat = 1;
            learn_from_success(ch, gsn_retreat, who_fighting(ch));
         }
      }
   }
   
   // Check to see if player is fleeing while on a map (but not fleeing from one map to another)
   if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
   {
      x = 0;

      if (vanish || number_range(1, 100) <= UMIN(85, (45 - broken + escapism + UMIN(5, (get_curr_lck(ch) - 14)) + UMIN(5, (get_curr_dex(ch) - 14)))))
      {
         wf = who_fighting(ch);
         if (retreat == 0)
         {
            for (;;)
            {
               if (x >= 1000)
               {
                  bug("do_flee:  Search for an escape coord 1000 times and did not find one.  %d %d", ch->coord->x, ch->coord->y);
                  return;
               }
               addx = number_range(-5, 5);
               addy = number_range(-5, 5);
               if (addx == 0 && addy == 0)
               {
                  if (number_range(1, 2) == 1)
                     addx += 1;
                  else
                     addy += 1;
               }
               if (ch->coord->x + addx < 1 || ch->coord->x + addx > MAX_X)
               {
                  x++;
                  continue;
               }
               if (ch->coord->y + addy < 1 || ch->coord->y + addy > MAX_Y)
               {
                  x++;
                  continue;
               }
               if (sect_show[(int)map_sector[ch->map][ch->coord->x + addx][ch->coord->y + addy]].canpass == TRUE)
               {                 
                  kingdom = kingdom_sector[ch->map][ch->coord->x][ch->coord->y];
                  
                  oldx = ch->coord->x;
                  oldy = ch->coord->y;
               
                  if (bad_flee_sector(ch, ch->coord->x + addx, ch->coord->y + addy)) //mountains and rivers and stuff
                  {
                     x++;
                     continue;
                  }
               
                  ch->coord->x += addx;
                  ch->coord->y += addy;
                  if (IS_NPC(ch) && wIS_SET(ch, ROOM_NO_MOB))
                  {
                     ch->coord->x = oldx;
                     ch->coord->y = oldy;
                     x++;
                     continue;
                  }
                  if (room_is_private_wilderness(ch, ch->in_room, ch->coord->x, ch->coord->y, ch->map))
                  {
                     ch->coord->x = oldx;
                     ch->coord->y = oldy;
                     x++;
                     continue;
                  }
                  if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != kingdom) //don't want them fleeing into a blocked out town
                  {
                     ch->coord->x = oldx;
                     ch->coord->y = oldy;
                     x++;
                     continue;
                  }
                  break;
               }
               else
                  x++;
            }
         }
         adjust_flee_timers(ch);
         if (ch->mount)
         {
            send_to_char("Just before you flee, you yell for your mount to leave.\n\r", ch);
            stop_fighting(ch->mount, TRUE);
            ch->mount->coord->x += addx;
            ch->mount->coord->y += addy;
            ch->mount->map = ch->map;
            update_objects(ch->mount, -1, -1, -1); 
         }
         for (foch = ch->in_room->first_person; foch; foch = foch->next_in_room)
         {
            if (foch != ch /* loop room bug fix here by Thoric */
               && foch->master == ch && (foch->position == POS_STANDING || foch->position == POS_MOUNTED) && foch != ch->mount && IS_NPC(foch))
            {
               if (foch->coord->x == ch->coord->x && foch->coord->y == ch->coord->y && foch->map == ch->map)
               {
                  stop_fighting(foch, TRUE);
                  foch->coord->x += addx;
                  foch->coord->y += addy;
                  foch->map = ch->map;
                  update_objects(foch, -1, -1, -1); 
               }
            }
         }

         affect_strip(ch, gsn_sneak);
         xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
         if (ch->mount && ch->mount->fighting)
            stop_fighting(ch->mount, TRUE);
         act(AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM);
          //ch->coord->x += addx;
          //ch->coord->y += addy;
          update_objects(ch, -1, -1, -1);
          add_timer(ch, TIMER_RECENTFIGHT, 60, NULL, 0);/*-60 Seconds-*/ 

         act(AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM);
         act(AT_FLEE, "You flee head over heels from combat!", ch, NULL, NULL, TO_CHAR);
         if (!IS_NPC(ch) && ch->pcdata->learned[gsn_escapism] > 0)
            learn_from_success(ch, gsn_escapism, who_fighting(ch));
         if (!IS_NPC(ch) && ch->pcdata->learned[gsn_vanish] > 0)
            learn_from_success(ch, gsn_vanish, who_fighting(ch));
         if (wf && !IS_NPC(ch) && ch->pcdata->deity)
         {
            int level_ratio = number_range(2, 5);

            if (wf && wf->race == ch->pcdata->deity->npcrace)
               adjust_favor(ch, 1, level_ratio);
            else if (wf && wf->race == ch->pcdata->deity->npcfoe)
               adjust_favor(ch, 16, level_ratio);
            else
               adjust_favor(ch, 0, level_ratio);
         }
         stop_fighting(ch, TRUE);

         do_look(ch, "auto");
         return;
      }
      ch->coord->x = oldx;
      ch->coord->y = oldy;
      act(AT_FLEE, "You attempt to flee from combat, but can't escape!", ch, NULL, NULL, TO_CHAR);
      act(AT_FLEE, "$n tries to flee, but fails miserably!", ch, NULL, NULL, TO_ROOM);
      if (!IS_NPC(ch) && ch->pcdata->learned[gsn_escapism] > 0)
         learn_from_failure(ch, gsn_escapism, who_fighting(ch));
      if (!IS_NPC(ch) && ch->pcdata->learned[gsn_vanish] > 0)
         learn_from_failure(ch, gsn_vanish, who_fighting(ch));
      ch->fight_timer = 3;
      if (number_range(1, 2) == 1)
      {
         wf = who_fighting(ch); 
         af.type = gsn_balance;
         af.duration = 3;
         af.location = APPLY_ARMOR;
         af.modifier = -1;
         af.bitvector = meb(AFF_BALANCE); 
         affect_to_char(ch, &af);
         act(AT_RED, "You are nearly tripped up with your failed attempt to escape", ch, NULL, NULL, TO_CHAR);
         act(AT_RED, "$n tries to flee, but nearly trips trying to escape!", ch, NULL, NULL, TO_ROOM);
      }
      return;
   }

   was_in = ch->in_room;
   if (retreat == 1) //god this is getting messy, rofl
   {
      if (vanish || number_range(1, 100) <= UMIN(85, (45 - broken + escapism + UMIN(5, (get_curr_lck(ch) - 14)) + UMIN(5, (get_curr_dex(ch) - 14)))))
      {
         affect_strip(ch, gsn_sneak);
         xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
         adjust_flee_timers(ch);
         if (ch->mount && ch->mount->fighting)
            stop_fighting(ch->mount, TRUE);
         move_char(ch, pexit, 0);
         now_in = ch->in_room;
         ch->in_room = was_in;
         act(AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM);
         ch->in_room = now_in;
         add_timer(ch, TIMER_RECENTFIGHT, 60, NULL, 0);/*-60 Seconds-*/ 
         act(AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM);
         wf = who_fighting(ch);
         if (!IS_NPC(ch))
         {
            act(AT_FLEE, "You flee head over heels from combat!", ch, NULL, NULL, TO_CHAR);
            if (!IS_NPC(ch) && ch->pcdata->learned[gsn_escapism] > 0)
               learn_from_success(ch, gsn_escapism, who_fighting(ch));
            if (!IS_NPC(ch) && ch->pcdata->learned[gsn_vanish] > 0)
               learn_from_success(ch, gsn_vanish, who_fighting(ch));
            if (wf && ch->pcdata->deity)
            {
               int level_ratio = number_range(2, 5);

               if (wf && wf->race == ch->pcdata->deity->npcrace)
                  adjust_favor(ch, 1, level_ratio);
               else if (wf && wf->race == ch->pcdata->deity->npcfoe)
                  adjust_favor(ch, 16, level_ratio);
               else
                  adjust_favor(ch, 0, level_ratio);
            }
         }
         stop_fighting(ch, TRUE);
         return;
      }
      act(AT_FLEE, "You attempt to flee from combat, but can't escape!", ch, NULL, NULL, TO_CHAR);
      act(AT_FLEE, "$n tries to flee, but fails miserably!", ch, NULL, NULL, TO_ROOM);
      if (!IS_NPC(ch) && ch->pcdata->learned[gsn_escapism] > 0)
         learn_from_failure(ch, gsn_escapism, who_fighting(ch));
      if (!IS_NPC(ch) && ch->pcdata->learned[gsn_vanish] > 0)
         learn_from_failure(ch, gsn_vanish, who_fighting(ch));
      ch->fight_timer = 3;
      if (number_range(1, 2) == 1)
      {
         wf = who_fighting(ch); 
         af.type = gsn_balance;
         af.duration = 3;
         af.location = APPLY_ARMOR;
         af.modifier = -1;
         af.bitvector = meb(AFF_BALANCE);
         affect_to_char(ch, &af);
         act(AT_RED, "You are nearly tripped up with your failed attempt to escape", ch, NULL, NULL, TO_CHAR);
         act(AT_RED, "$n tries to flee, but nearly trips trying to escape!", ch, NULL, NULL, TO_ROOM);
      }
      return;
   }   
   for (attempt = 0; attempt < 50; attempt++)
   {
      door = number_door();
      if ((pexit = get_exit(was_in, door)) == NULL
         || !pexit->to_room
         || IS_SET(pexit->exit_info, EX_NOFLEE)
         || (IS_SET(pexit->exit_info, EX_CLOSED)
&& !IS_AFFECTED(ch, AFF_PASS_DOOR)) || (IS_NPC(ch) && xIS_SET(pexit->to_room->room_flags, ROOM_NO_MOB)))
         continue;
         
      if (number_range(1, 100) <= UMIN(85, (45 - broken + escapism + UMIN(5, (get_curr_lck(ch) - 14)) + UMIN(5, (get_curr_dex(ch) - 14)))))
      {
         affect_strip(ch, gsn_sneak);
         xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
         if (ch->mount && ch->mount->fighting)
            stop_fighting(ch->mount, TRUE);
         adjust_flee_timers(ch);
         move_char(ch, pexit, 0);
         if ((now_in = ch->in_room) == was_in)
            continue;
         ch->in_room = was_in;
         act(AT_FLEE, "$n flees head over heels!", ch, NULL, NULL, TO_ROOM);
         ch->in_room = now_in;
         add_timer(ch, TIMER_RECENTFIGHT, 60, NULL, 0);/*-60 Seconds-*/ 
         act(AT_FLEE, "$n glances around for signs of pursuit.", ch, NULL, NULL, TO_ROOM);
         wf = who_fighting(ch);
         if (!IS_NPC(ch))
         {
            act(AT_FLEE, "You flee head over heels from combat!", ch, NULL, NULL, TO_CHAR);
            if (!IS_NPC(ch) && ch->pcdata->learned[gsn_escapism] > 0)
               learn_from_success(ch, gsn_escapism, who_fighting(ch));
            if (!IS_NPC(ch) && ch->pcdata->learned[gsn_vanish] > 0)
               learn_from_success(ch, gsn_vanish, who_fighting(ch));
            if (wf && ch->pcdata->deity)
            {
               int level_ratio = number_range(2, 5);

               if (wf && wf->race == ch->pcdata->deity->npcrace)
                  adjust_favor(ch, 1, level_ratio);
               else if (wf && wf->race == ch->pcdata->deity->npcfoe)
                  adjust_favor(ch, 16, level_ratio);
               else
                  adjust_favor(ch, 0, level_ratio);
            }
         }
         stop_fighting(ch, TRUE);
         return;
      }
      else
         break;
   }
   act(AT_FLEE, "You attempt to flee from combat, but can't escape!", ch, NULL, NULL, TO_CHAR);
   act(AT_FLEE, "$n tries to flee, but fails miserably!", ch, NULL, NULL, TO_ROOM);
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_escapism] > 0)
      learn_from_failure(ch, gsn_escapism, who_fighting(ch));
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_vanish] > 0)
      learn_from_failure(ch, gsn_vanish, who_fighting(ch));
   ch->fight_timer = 3;
   if (number_range(1, 2) == 1)
   {
      wf = who_fighting(ch); 
      af.type = gsn_balance;
      af.duration = 3;
      af.location = APPLY_ARMOR;
      af.modifier = -1;
      af.bitvector = meb(AFF_BALANCE);
      affect_to_char(ch, &af);
      act(AT_RED, "You are nearly tripped up with your failed attempt to escape", ch, NULL, NULL, TO_CHAR);
      act(AT_RED, "$n tries to flee, but nearly trips trying to escape!", ch, NULL, NULL, TO_ROOM);
   }
   return;
}


void do_sla(CHAR_DATA * ch, char *argument)
{
   send_to_char("If you want to SLAY, spell it out.\n\r", ch);
   return;
}

/*old slay code editted out 7/25/99 by Shai'tan
**
**void do_slay( CHAR_DATA *ch, char *argument )
**{
**    CHAR_DATA *victim;
**    char arg[MIL];
**    char arg2[MIL];
**
**    argument = one_argument( argument, arg );
**    one_argument( argument, arg2 );
**    if ( arg[0] == '\0' )
**    {
**	send_to_char( "Slay whom?\n\r", ch );
**	return;
**    }
**
**    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
**    {
**	send_to_char( "They aren't here.\n\r", ch );
**	return;
**    }
**
**    if ( ch == victim )
**    {
**	send_to_char( "Suicide is a mortal sin.\n\r", ch );
**	return;
**    }
**
**    if ( !IS_NPC(victim) && get_trust( victim ) >= get_trust( ch ) )
**    {
**	send_to_char( "You failed.\n\r", ch );
**	return;
**    }
**
**    if ( !str_cmp( arg2, "immolate" ) )
**    {
**      act( AT_FIRE, "Your fireball turns $N into a blazing inferno.",  ch, NULL, victim, TO_CHAR    );
**      act( AT_FIRE, "$n releases a searing fireball in your direction.", ch, NULL, victim, TO_VICT    );
**      act( AT_FIRE, "$n points at $N, who bursts into a flaming inferno.",  ch, NULL, victim, TO_NOTVICT );
**    }
**
**    else if ( !str_cmp( arg2, "shatter" ) )
**  {
**    act( AT_LBLUE, "You freeze $N with a glance and shatter the frozen corpse into tiny shards.",  ch, NULL, victim, TO_CHAR    );
**    act( AT_LBLUE, "$n freezes you with a glance and shatters your frozen body into tiny shards.", ch, NULL, victim, TO_VICT    );
**    act( AT_LBLUE, "$n freezes $N with a glance and shatters the frozen body into tiny shards.",  ch, NULL, victim, TO_NOTVICT );
**  }
**
**  else if ( !str_cmp( arg2, "demon" ) )
**  {
**    act( AT_IMMORT, "You gesture, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_CHAR );
**    act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_CHAR );
**    act( AT_IMMORT, "$n gestures, and a slavering demon appears.  The foul creature turns on",  ch, NULL, victim, TO_VICT );
**   act( AT_IMMORT, "you with a horrible grin.   You scream in panic before being eaten alive.",  ch, NULL, victim, TO_VICT );
**    act( AT_IMMORT, "$n gestures, and a slavering demon appears.  With a horrible grin, the",  ch, NULL, victim, TO_NOTVICT );
**    act( AT_IMMORT, "foul creature turns on $N, who screams in panic before being eaten alive.",  ch, NULL, victim, TO_NOTVICT );
**  }
**
**  else if ( !str_cmp( arg2, "pounce" ) )
**  {
**    act( AT_BLOOD, "Leaping upon $N with bared fangs, you tear open $S throat and toss the corpse to the ground...",  ch, NULL, victim, TO_CHAR );
**    act( AT_BLOOD, "In a heartbeat, $n rips $s fangs through your throat!  Your blood sprays and pours to the ground as your life ends...", ch, NULL, victim, TO_VICT );
**    act( AT_BLOOD, "Leaping suddenly, $n sinks $s fangs into $N's throat.  As blood sprays and gushes to the ground, $n tosses $N's dying body away.",  ch, NULL, victim, TO_NOTVICT );
**  }
**
**  else if ( !str_cmp( arg2, "slit" ) )
**  {
**    act( AT_BLOOD, "You calmly slit $N's throat.", ch, NULL, victim, TO_CHAR );
**    act( AT_BLOOD, "$n reaches out with a clawed finger and calmly slits your throat.", ch, NULL, victim, TO_VICT );
**      act( AT_BLOOD, "$n calmly slits $N's throat.", ch, NULL, victim, TO_NOTVICT );
**    }
**
**    else if ( !str_cmp( arg2, "dog" ) )
**    {
**      act( AT_BLOOD, "You order your dogs to rip $N to shreds.", ch, NULL, victim, TO_CHAR );
**      act( AT_BLOOD, "$n orders $s dogs to rip you apart.", ch, NULL, victim, TO_VICT );
**     act( AT_BLOOD, "$n orders $s dogs to rip $N to shreds.", ch, NULL, victim, TO_NOTVICT );
**    }
**
**    else
**    {
**      act( AT_IMMORT, "You slay $N in cold blood!",  ch, NULL, victim, TO_CHAR    );
**      act( AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT    );
**     act( AT_IMMORT, "$n slays $N in cold blood!",  ch, NULL, victim, TO_NOTVICT );
**    }
**
**    set_cur_char(victim);
**    raw_kill( ch, victim );
**    return;
**}
*/
