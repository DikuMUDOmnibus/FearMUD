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
 *			           Archery Module  			                *
 ****************************************************************************

Bowfire Code v1.0 (c)1997-99 Feudal Realms 

This is my bow and arrow code that I wrote based off of a thrown weapon code
that I had from long ago (if you wrote it let me know so I can give you
credit for that part, I do not have it anymore), it's a little more complex 
than I had originally wanted, but well, it works.  There are a couple things 
that are involved which if you don't want to use, remove them, that simple.  
One of them are the use of the "lodged" wearbits.  The code is designed to 
lodge an arrow in a victim, not just do damage to them once, and there are three
places it can lodge, etc.  Included are all of the pieces of code for quivers,
arrows, drawing arrows, dislodging, etc.  Use whatever of this code that you
want, if you have a credits page, add me on there, and please drop me an email
at mustang@roscoe.mudservices.com so I know its out there somewhere being used.

Any bugs that people find, if you email me, I will fix, unless it's something
from a modification that you made, and if that's the case, I will probably
help you figure out what's up with it if I can.  My code is not stock, and I
tried to add in everything that people might need to add in this feature.

Thanks,
Tch

===============================================================================
Features in v1.0

- Bowfire from adjacent rooms at targets
- Arrows lodge in various body parts (leg, arm, and chest)
- Quiver and arrow new item types
- Shoulder wearbit used for quivers (if you want it)
- Dislodging arrows does damage
- OLC support for arrows and quivers

===============================================================================

Add in a bow weapon type with the other ones on the list.(if you don't know 
how to do this, grep/search for sword and add in bow in the respective places)
		
=============================================================================== 

Bowfire code ported for Smaug 1.4a by Samson.
Combined with portions of the Smaug 1.4a archery code.
Additional portions by Samson.
*/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include "mud.h"

int weapon_prof_bonus_check(CHAR_DATA * ch, OBJ_DATA * wield, int *gsn_ptr);
int obj_hitroll(OBJ_DATA * obj);
ch_ret spell_attack(int, int, CHAR_DATA *, void *);


OBJ_DATA *find_quiver(CHAR_DATA * ch)
{
   OBJ_DATA *obj;

   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
   {
      if (can_see_obj(ch, obj))
      {
         if (obj->item_type == ITEM_QUIVER && !IS_SET(obj->value[1], CONT_CLOSED))
            return obj;
      }
   }
   return NULL;
}

OBJ_DATA *find_projectile(CHAR_DATA * ch, OBJ_DATA * quiver)
{
   OBJ_DATA *obj;

   for (obj = quiver->last_content; obj; obj = obj->prev_content)
   {
      if (can_see_obj(ch, obj))
      {
         if (obj->item_type == ITEM_PROJECTILE)
            return obj;
      }
   }
   return NULL;
}

/* Bowfire code -- used to draw an arrow from a quiver */
void do_nock(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *bow;
   OBJ_DATA *arrow;
   OBJ_DATA *carrow;
   OBJ_DATA *quiver;
   OBJ_DATA *oweapon;
   int hand_count = 0;

   if ((bow = get_eq_char(ch, WEAR_MISSILE_WIELD)) == NULL)
   {
      send_to_char("You are not wielding a missile weapon!\n\r", ch);
      return;
   }

   if ((quiver = find_quiver(ch)) == NULL)
   {
      send_to_char("You aren't wearing a quiver where you can get to it!\n\r", ch);
      return;
   }

   if ((oweapon = get_eq_char(ch, WEAR_SHIELD)) != NULL)
   {
      if (IS_OBJ_STAT(oweapon, ITEM_TWOHANDED))
         hand_count+=2;
      else
         hand_count+=1;
   }
   if ((oweapon = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
   {
      if (IS_OBJ_STAT(oweapon, ITEM_TWOHANDED))
         hand_count+=2;
      else
         hand_count+=1;
   }
   if ((oweapon = get_eq_char(ch, WEAR_WIELD)) != NULL)
   {
      if (IS_OBJ_STAT(oweapon, ITEM_TWOHANDED))
         hand_count+=2;
      else
         hand_count+=1;
   }
   if (hand_count > 1)
   {
      send_to_char("You need a free hand to draw with.\n\r", ch);
      return;
   }

   if (get_eq_char(ch, WEAR_NOCKED) != NULL)
   {
      send_to_char("Your hand is not empty!\n\r", ch);
      return;
   }

   if ((arrow = find_projectile(ch, quiver)) == NULL)
   {
      send_to_char("Your quiver is empty!!\n\r", ch);
      return;
   }

   if (bow->value[7] != arrow->value[7])
   {
      send_to_char("You drew the wrong projectile type for this weapon!\n\r", ch);
      separate_obj(arrow);
      obj_from_obj(arrow);
      obj_to_char(arrow, ch);
      return;
   }

   WAIT_STATE(ch, 6);
   sprintf(log_buf, "$n draws a %s from $p.", arrow->short_descr);
   act(AT_ACTION, log_buf, ch, quiver, NULL, TO_ROOM);
   sprintf(log_buf, "You draw a %s from $p.", arrow->short_descr);
   act(AT_ACTION, log_buf, ch, quiver, NULL, TO_CHAR);
   separate_obj(arrow);
   obj_from_obj(arrow);
   if (IS_NPC(ch))
   {
      carrow = clone_object(arrow);
      obj_to_obj(carrow, quiver);
   }
   wear_obj(ch, arrow, TRUE, get_wflag("nocked"));
   obj_to_char(arrow, ch);
   return;
}

/* Bowfire code -- Used to dislodge an arrow already lodged */
void do_dislodge(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *arrow = NULL;
   int dam = 0;

   if (get_eq_char(ch, WEAR_LODGE_RIB) != NULL)
   {
      arrow = get_eq_char(ch, WEAR_LODGE_RIB);
      act(AT_CARNAGE, "With a wrenching pull, you dislodge $p from your chest.", ch, arrow, NULL, TO_CHAR);
      act(AT_CARNAGE, "$n winces in pain as $e dislodges $p from $s chest.", ch, arrow, NULL, TO_ROOM);
      unequip_char(ch, arrow);
      REMOVE_BIT(arrow->wear_flags, ITEM_LODGE_RIB);
      //REMOVE_WEAR_FLAG( arrow, ITEM_LODGE_RIB );
      REMOVE_OBJ_STAT(arrow, ITEM_LODGED);
      dam = number_range((3 * arrow->value[1]), (3 * arrow->value[2]));
      damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
      return;
   }
   else if (get_eq_char(ch, WEAR_LODGE_LEG) != NULL)
   {
      arrow = get_eq_char(ch, WEAR_LODGE_LEG);
      act(AT_CARNAGE, "With a tug you dislodge $p from your leg.", ch, arrow, NULL, TO_CHAR);
      act(AT_CARNAGE, "$n winces in pain as $e dislodges $p from $s leg.", ch, arrow, NULL, TO_ROOM);
      unequip_char(ch, arrow);
      REMOVE_BIT(arrow->wear_flags, ITEM_LODGE_LEG);
      //REMOVE_WEAR_FLAG( arrow, ITEM_LODGE_LEG );
      REMOVE_OBJ_STAT(arrow, ITEM_LODGED);
      dam = number_range((3 * arrow->value[1]), (2 * arrow->value[2]));
      damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
      return;
   }
   else if (get_eq_char(ch, WEAR_LODGE_ARM) != NULL)
   {
      arrow = get_eq_char(ch, WEAR_LODGE_ARM);
      act(AT_CARNAGE, "With a tug you dislodge $p from your arm.", ch, arrow, NULL, TO_CHAR);
      act(AT_CARNAGE, "$n winces in pain as $e dislodges $p from $s arm.", ch, arrow, NULL, TO_ROOM);
      unequip_char(ch, arrow);
      REMOVE_BIT(arrow->wear_flags, ITEM_LODGE_ARM);
      //REMOVE_WEAR_FLAG( arrow, ITEM_LODGE_ARM );
      REMOVE_OBJ_STAT(arrow, ITEM_LODGED);
      dam = number_range((2 * arrow->value[1]), (2 * arrow->value[2]));
      damage(ch, ch, dam, TYPE_UNDEFINED, 0, -1);
      return;
   }
   else
   {
      send_to_char("You have nothing lodged in your body.\n\r", ch);
      return;
   }
}

/*
 * Hit one guy with a projectile.
 * Handles use of missile weapons (wield = missile weapon)
 * or thrown items/weapons
 */
ch_ret projectile_hit(CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * wield, OBJ_DATA * projectile, sh_int dist, int dt)
{
   OBJ_DATA *eq = NULL;
   OBJ_DATA *shield;
   sh_int tohit;
   int victim_ac = 0;
   int weapon_ac = 0;
   int armor_ac = 0;
   int plusris;
   int dam = 0;
   int prof_bonus;
   int prof_gsn = -1;
   int wear_loc;
   int limb;
   int proj_bonus;
   int chance;
   int fighting = 0;
   ch_ret retcode;
   int ogrip;
   int crit = 0;
   int cond;
   int suc = 0;
   int block = 0;
   int noarmor = 0;
   int level, hnum, mastery;
   BUYKMOB_DATA *kmob;

   if (!projectile)
      return rNONE;
      
   if (ch->fighting)
      fighting = 1;

   if (projectile->item_type == ITEM_PROJECTILE || projectile->item_type == ITEM_WEAPON)
   {
      if (dt != gsn_perfect_shot)
         dt = TYPE_HIT;
      if (wield)
         proj_bonus = number_range(wield->value[1], wield->value[2]);
      else
         proj_bonus = 0;
   }
   else
   {
      if (dt != gsn_perfect_shot)
         dt = TYPE_UNDEFINED;
      proj_bonus = 0;
   }
   
   blockdam = 0;

   /*
    * Can't beat a dead char!
    */
   if (victim->position == POS_DEAD || char_died(victim))
   {
      extract_obj(projectile);
      return rVICT_DIED;
   }

   if (wield)
      prof_bonus = weapon_prof_bonus_check(ch, wield, &prof_gsn);
   else
      prof_bonus = 0;

   if (dt == TYPE_UNDEFINED)
   {
      dt = TYPE_HIT;
   }
   chance = number_range(1, 20);
   
   if (chance >= 1 && chance <= 5)
   {
      wear_loc = number_range(WEAR_ARM_R, WEAR_ARM_L);
      if (wear_loc == WEAR_ARM_R)
         limb = LM_RARM;
      else
         limb = LM_LARM;
   }
   else if (chance >= 6 && chance <= 10)
   {
      wear_loc = number_range(WEAR_LEG_R, WEAR_LEG_L);
      if (wear_loc == WEAR_LEG_R)
         limb = LM_RLEG;
      else
         limb = LM_LLEG;
   }
   else if (chance >= 11 && chance <= 17)
   {
      wear_loc = WEAR_BODY;
      limb = LM_BODY;
   }
   else if (chance >= 18 && chance <= 19)
   {
      wear_loc = WEAR_HEAD;
      limb = LM_HEAD;
   }
   else
   {
      wear_loc = WEAR_NECK;
      limb = LM_NECK;
   }
   if (dt == gsn_perfect_shot)
   {
      chance = 20;
      wear_loc = WEAR_NECK;
      limb = LM_NECK;
   }
   /*
    * Replaces that thaco crap, back to a easier to identify base 20 ac
    */
   switch (chance)
   {
      case 1:
      case 2:
      case 3: /* Hit in the arm */
      case 4:
      case 5:
         if ((eq = get_eq_char(victim, wear_loc)) == NULL)
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
            {
               armor_ac = 0;
               noarmor = 1;
            }
         }
         break;
      case 6:
      case 7:
      case 8: /* Hit in the leg */
      case 9:
      case 10:
         if ((eq = get_eq_char(victim, wear_loc)) == NULL)
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
            {
               noarmor = 1;
               armor_ac = 0;
            }
         }
         break;
      case 11:
      case 12:
      case 13:
      case 14: /* Hit in the chest */
      case 15:
      case 16:
      case 17:
         if ((eq = get_eq_char(victim, wear_loc)) == NULL)
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
            {
               armor_ac = 0;
               noarmor = 1;
            }
         }
         break;
      case 18:
      case 19: //Head
         if ((eq = get_eq_char(victim, wear_loc)) == NULL)
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
            {
               noarmor = 1;        
               armor_ac = 0;
            }
         }
         break;
      case 20: //Neck
         if ((eq = get_eq_char(victim, wear_loc)) == NULL)
         {
            if (IS_NPC(victim))
            {
               armor_ac = victim->armor;
               if (victim->armor == 0)
                  noarmor = 1;
               if (limb == LM_HEAD)
                  armor_ac += 2;
               else if (limb == LM_NECK)
               {
                  if (dt == gsn_perfect_shot)
                     armor_ac += 3 - (MASTERED(ch, gsn_perfect_shot)-1);
                  else
                     armor_ac += 3;        
               }  
            }
            else
            {
               noarmor = 1;
               armor_ac = 0;
            }
         }
         break;
   }
   
   if (eq)
   {
      //For Stab, all projectiles do stab/pierce like damage
      armor_ac = eq->value[2];
   }
   
   if (wield)
      weapon_ac += wield->value[9];
      
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
   
   if (projectile)
      weapon_ac += projectile->value[9];
   
   if (eq)
      cond = eq->value[10];
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
   victim_ac = armor_ac - weapon_ac;  //negative is a better to hit
   
   /* if you can't see what's coming... */
   if (wield && projectile && !can_see_obj(victim, projectile))
      victim_ac -= 1;

   //Small bonus for the attacker since most attacks aren't seen
   victim_ac -= 1;
   
    /*
    * The moment of excitement!
    */
   if (eq)
      cond = eq->value[10];
   else
      cond = 100*victim->hit/victim->max_hit;
   
   tohit = get_hit_or_miss(ch, victim, victim_ac, GRIP_STAB, limb, noarmor, wield, dt);     
   
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
   
   if (victim->apply_arrowcatch && victim != ch)
   {
      int sn = skill_lookup("arrowcatch");
      if (number_range(1, 100) <= UMAX(60, victim->apply_arrowcatch*2/3))
      {
         tohit = DM_MISS;
         if (sn > 0 && LEARNED(victim, sn) > 0)
            learn_from_success(victim, sn, ch);
         act(AT_WHITE, "$n quickly sticks out his hand and grabs an arrow out of mid air!!!", victim, NULL, NULL, TO_ROOM);
         act(AT_WHITE, "You quickly stick out your hand and grab an arrow out of mid air!!!", victim, NULL, NULL, TO_CHAR);
      }
      else
      {
         if (sn > 0 && LEARNED(victim, sn) > 0)
            learn_from_failure(victim, sn, ch);
      }
   }     
   if (tohit == DM_MISS || block) //miss or block
   {
      /* Miss. */
      if (!block)
      {
         ogrip = ch->grip;
         ch->grip = GRIP_STAB;
         damage(ch, victim, 0, TYPE_PROJECTILE, DM_MISS, LM_BODY);
         ch->grip = ogrip;
         tail_chain();
         return rNONE;
      }
   }

   /*
    * Hit.
    * Calc damage.
    */

   switch (chance)
   {
      case 1:
      case 2:
      case 3: /* Hit in the arm */
      case 4:
      case 5:
         dam = number_range((3*projectile->value[1]/2), (2 * projectile->value[2])) + proj_bonus+1;
         break;
      case 6:
      case 7:
      case 8: /* Hit in the leg */
      case 9:
      case 10:
         dam = number_range((3*projectile->value[1]/2), (2 * projectile->value[2])) + proj_bonus+2;
         break;
      case 11:
      case 12:
      case 13:
      case 14: /* Hit in the chest */
      case 15:
      case 16:
      case 17:
         dam = number_range((3*projectile->value[1]/2), (2 * projectile->value[2])) + proj_bonus;
         break;
      case 18:
      case 19: //Head
      	 dam = number_range((2 * projectile->value[1]), (2 * projectile->value[2])) + proj_bonus;
      	 break;
      case 20: //Neck
         dam = number_range((3*projectile->value[1]/2), (2 * projectile->value[2])) + proj_bonus;
         break;
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

   dam -= URANGE(-6, victim->apply_stone, 6);
   dam -= URANGE(-4, victim->apply_hardening, 4);
   if (victim->apply_hardening > 0)
      check_aff_learn(victim, "bracing", 0, ch, 1);
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
   /*
    * Calculate Damage Modifiers from Victim's Fighting Style
    */
   if (suc)
   {
      get_fightingstyle_dam(victim, dam, ch, 0, 1);
      get_fightingstyle_dam(victim, dam, ch, 1, 1);
   }
   else
   {
      get_fightingstyle_dam(victim, dam, ch, 0, 0);
      get_fightingstyle_dam(victim, dam, ch, 1, 0);
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
      if (suc)
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
      if (suc)
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
      if (suc)
         learn_from_success(ch, gsn_deadly_accuracy, victim);
   }
   if (!IS_AWAKE(victim))
      dam *= 2;

   if (dam <= 0)
      dam = 1;
      
   if (IS_NPC(ch))
   {
      dam += number_range(ch->damaddlow, ch->damaddhi);
   }  

   plusris = 0;
   
   if (block == 1)
   {
      ogrip = ch->grip;
      ch->grip = GRIP_STAB;
      blockdam = dam;
      damage(ch, victim, 0, TYPE_PROJECTILE, DM_BLOCK, LM_BODY);
      ch->grip = ogrip;
   }

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
         {
            if (number_percent() < 50)
               extract_obj(projectile);
            else
            {
               if (projectile->carried_by)
                  obj_from_char(projectile);
               obj_to_room(projectile, victim->in_room, victim);
            }
            return rNONE;
         }
      }
      dam = 0;
      crit = DM_MISS;
   }
   
   if(xIS_SET(victim->act, ACT_UNDEAD))
   {
       if(!xIS_SET(wield->extra_flags, ITEM_BLESS) && !xIS_SET(wield->extra_flags, ITEM_SANCTIFIED))
       {
           crit = DM_UNDEAD;
           dam = 0;
       }
       if(xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur > 0)
       {
          if (dam)
             wield->bless_dur -= 1;
          if(xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur <= 0)
          {
             xTOGGLE_BIT(wield->extra_flags, ITEM_BLESS);
             act(AT_MAGIC, "Your $p stops glowing...\n\r", ch, wield, NULL, TO_CHAR);
          }
       }
   }
   if(xIS_SET(victim->act, ACT_LIVING_DEAD))
   {
       if(!xIS_SET(wield->extra_flags, ITEM_BLESS) && !xIS_SET(wield->extra_flags, ITEM_SANCTIFIED))
       {
          dam = dam / 2;
       }
       if(xIS_SET(wield->extra_flags, ITEM_BLESS) || xIS_SET(wield->extra_flags, ITEM_SANCTIFIED))
       {
          dam = dam * 2;
       }
       if(xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur > 0)
       {
          if (dam)
             wield->bless_dur -= 1;
          if(xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur <= 0)
          {
              xTOGGLE_BIT(wield->extra_flags, ITEM_BLESS);
              act(AT_MAGIC, "Your $p stops glowing...\n\r", ch, wield, NULL, TO_CHAR);
          }
       }
   }     
   if (tohit >= 300 && crit != DM_UNDEAD)
      crit = DM_CRITICAL;
   if (block == 1 && crit != DM_UNDEAD)
      crit = DM_BLOCK;
   if (tohit == DM_DEATH && crit != DM_UNDEAD)
   {
      crit = DM_DEATH;
      tohit = 200;
   }
   if (!IS_NPC(ch))
   {
      dam += UMAX(1, dam * POINT_LEVEL(LEARNED(ch, gsn_weapon_projectile), MASTERED(ch, gsn_weapon_projectile)) / 600);
      if (tohit != DM_MISS)
         learn_from_success(ch, gsn_weapon_projectile, victim);   
   }
   dam = dam * (tohit-100) / 100;
   
   if (crit == DM_HIT)
   {
      dam += URANGE(-15, ch->apply_sanctify, 15);
      dam += URANGE(-10, ch->apply_manaburn, 10); 
      if (ch->apply_manaburn > 0 && skill_lookup("arrowcatch") > 0 && LEARNED(ch, skill_lookup("arrowcatch")) > 0)
         learn_from_success(ch, skill_lookup("arrowcatch"), victim);
   }
   //Want the melee's to pinch a bit, mainly need this vs players.   
   if (crit == DM_HIT && IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
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
   
   if (crit == DM_MISS || crit == DM_UNDEAD || crit == DM_BLOCK)
      dam = 0;
   
   ogrip = ch->grip;
   ch->grip = GRIP_STAB;     
   if (dt != gsn_perfect_shot)
      dt = TYPE_PROJECTILE;
   retcode = damage(ch, victim, dam, dt, crit, limb);
   ch->grip = ogrip;
   if (dam > 0 && !char_died(victim) && !char_died(ch))
   {
      /*if( projectile->value[3] == DAM_STONE )
         extract_obj( projectile );
         else
         { */
      obj_from_char(projectile);
      obj_to_char(projectile, victim);
      SET_OBJ_STAT(projectile, ITEM_LODGED);

      switch (chance)
      {
         case 1:
         case 2:
         case 3: /* Hit in the arm */
         case 4:
         case 5:
            SET_BIT(projectile->wear_flags, ITEM_LODGE_ARM);
            //   SET_WEAR_FLAG( projectile, ITEM_LODGE_ARM );
            wear_obj(victim, projectile, TRUE, get_wflag("lodge_arm"));
            break;
         case 6:
         case 7:
         case 8: /* Hit in the leg */
         case 9:
         case 10:
            SET_BIT(projectile->wear_flags, ITEM_LODGE_LEG);
            //   SET_WEAR_FLAG( projectile, ITEM_LODGE_LEG );
            wear_obj(victim, projectile, TRUE, get_wflag("lodge_leg"));
            break;
         case 11:
         case 12:
         case 13:
         case 14: /* Hit in the chest */
         case 15:
         case 16:
         case 17:
            SET_BIT(projectile->wear_flags, ITEM_LODGE_RIB);
            //   SET_WEAR_FLAG( projectile, ITEM_LODGE_RIB );
            wear_obj(victim, projectile, TRUE, get_wflag("lodge_rib"));
            break;
      }
      return retcode;
   }
   if (char_died(ch))
   {
      extract_obj(projectile);
      return rCHAR_DIED;
   }
   if (char_died(victim))
   {
      extract_obj(projectile);
      send_to_char("&RYou see your victim fall over from your fatal shot!!!!\n\r", ch);
      return rVICT_DIED;
   }

   retcode = rNONE;
   if (dam == 0)
   {
      if (number_percent() < 50)
         extract_obj(projectile);
      else
      {
         if (projectile->carried_by)
            obj_from_char(projectile);
         obj_to_room(projectile, victim->in_room, victim);
      }
      return retcode;
   }

/* weapon spells	-Thoric */
   if (wield && !is_immune(victim, -1, RIS_MAGIC) && !wIS_SET(victim, ROOM_NO_MAGIC) 
   &&  !xIS_SET(victim->in_room->room_flags, ROOM_NO_MAGIC))
   {
      AFFECT_DATA *aff;

      for (aff = wield->pIndexData->first_affect; aff; aff = aff->next)
         if (aff->location == APPLY_WEAPONSPELL && IS_VALID_SN(aff->modifier) && skill_table[aff->modifier]->spell_fun)
            retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, 7, ch, victim);
      if (retcode != rNONE || char_died(ch) || char_died(victim))
      {
         extract_obj(projectile);
         return retcode;
      }
      for (aff = wield->first_affect; aff; aff = aff->next)
         if (aff->location == APPLY_WEAPONSPELL && IS_VALID_SN(aff->modifier) && skill_table[aff->modifier]->spell_fun)
            retcode = (*skill_table[aff->modifier]->spell_fun) (aff->modifier, 7, ch, victim);
      if (retcode != rNONE || char_died(ch) || char_died(victim))
      {
         extract_obj(projectile);
         return retcode;
      }
   }

   extract_obj(projectile);

   tail_chain();
   return retcode;
}

/*
 * Perform the actual attack on a victim			-Thoric
 */
ch_ret ranged_got_target(CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * weapon,
   OBJ_DATA * projectile, sh_int dist, sh_int dt, char *stxt, sh_int color)
{
   /* added wtype for check to determine skill used for ranged attacks - Grimm */
   sh_int wtype = 0;

   if (is_safe(ch, victim))
   {
      /* safe room, bubye projectile */
      if (projectile)
      {
         ch_printf(ch, "Your %s is blasted from existance by a godly presense.", myobj(projectile));
         act(color, "A godly presence smites $p!", victim, projectile, NULL, TO_ROOM);
         extract_obj(projectile);
      }
      else
      {
         ch_printf(ch, "Your %s is blasted from existance by a godly presense.", stxt);
         act(color, "A godly presence smites $t!", victim, aoran(stxt), NULL, TO_ROOM);
      }
      return rNONE;
   }

   /* check dam type of projectile to determin value of wtype 
    * wtype points to same "sh_int" as the skill assigned to that
    * range by the code and as such the proper skill will be used. 
    * Grimm 
    */
   wtype = TYPE_HIT;
   //Stationary mobs will take fire, but a chance they will break out of it and give chase
   if (xIS_SET(victim->act, ACT_MILITARY) && xIS_SET(victim->miflags, KM_STATIONARY))
   {
      if (number_range(1, 100) <= 15)
      {
         ch_printf(ch, "$n becomes enranged and leaves his station to come for YOU.\n\r", IS_NPC(victim) ? victim->short_descr : victim->name);
         xREMOVE_BIT(victim->miflags, KM_STATIONARY);
      }
   }
   if (projectile && weapon && !xIS_SET(victim->act, ACT_SENTINEL))
   {
      // Maybe later, now now...
      /*if( IS_NPC( victim ) ) 
         REMOVE_ACT_FLAG( victim, ACT_SENTINEL ); */
      if (projectile)
         global_retcode = projectile_hit(ch, victim, weapon, projectile, dist, dt);
      else
         global_retcode = spell_attack(dt, ch->level, ch, victim);
   }
   else
   {
      if (!xIS_SET(victim->act, ACT_SENTINEL))
         global_retcode = damage(ch, victim, 0, TYPE_UNDEFINED, 0, -1);

      if (projectile)
      {
         /* 50% chance of getting lost */
         if (number_percent() < 50)
            extract_obj(projectile);
         else
         {
            if (projectile->carried_by)
               obj_from_char(projectile);
            obj_to_room(projectile, victim->in_room, victim);
         }
      }
   }
   return global_retcode;
}

/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.	-Thoric
 */
//Pretty obvious, I need different functions for each.
CHAR_DATA *scan_wilderness_for_victim(CHAR_DATA * ch, int vdir, char *name)
{
   CHAR_DATA *victim = NULL;
   CHAR_DATA *vch;
   sh_int shortdis = 10;

   if (IS_AFFECTED(ch, AFF_BLIND))
      return NULL;

   if ((victim = get_char_wilder(ch, name)) == NULL)
      return NULL;

   // Find an instance of the mob, and if there are more, find which one is to the east
   for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
   {
      if (!str_cmp(victim->name, vch->name))
      {
         if ((IN_WILDERNESS(ch) && !IN_WILDERNESS(vch)) || (!IN_WILDERNESS(ch) && IN_WILDERNESS(vch)))
            continue;

         switch (vdir)
         {
            case 0:
               if ((ch->coord->x == vch->coord->x) && ((ch->coord->y - vch->coord->y <= 10) && (ch->coord->y - vch->coord->y > 0)))
               {
                  if (ch->coord->y - vch->coord->y < shortdis)
                  {
                     shortdis = ch->coord->y - vch->coord->y;
                     victim = vch;
                  }
               }
               break;
            case 1:
               if ((ch->coord->y == vch->coord->y) && ((vch->coord->x - ch->coord->x <= 10) && (vch->coord->x - ch->coord->x > 0)))
               {
                  if (ch->coord->x - vch->coord->x < shortdis)
                  {
                     shortdis = ch->coord->x - vch->coord->x;
                     victim = vch;
                  }
               }
            case 2:
               if ((ch->coord->x == vch->coord->x) && ((vch->coord->y - ch->coord->y <= 10) && (vch->coord->y - ch->coord->y > 0)))
               {
                  if (vch->coord->y - ch->coord->y < shortdis)
                  {
                     shortdis = ch->coord->y = vch->coord->y;
                     victim = vch;
                  }
               }
               break;
            case 3:

               if ((ch->coord->y == vch->coord->y) && ((ch->coord->x - vch->coord->x <= 10) && (ch->coord->x - vch->coord->x > 0)))
               {
                  if (vch->coord->x - ch->coord->x < shortdis)
                  {
                     shortdis = ch->coord->x = vch->coord->x;
                     victim = vch;
                  }
               }
               break;
         }
      }
   }
   return victim;
}

CHAR_DATA *scan_for_victim(CHAR_DATA * ch, EXIT_DATA * pexit, char *name)
{
   CHAR_DATA *victim;
   ROOM_INDEX_DATA *was_in_room;
   sh_int dist, dir;
   sh_int max_dist = 10;

   if (IS_AFFECTED(ch, AFF_BLIND) || !pexit)
      return NULL;

   was_in_room = ch->in_room;

   for (dist = 1; dist <= max_dist;)
   {
      if (IS_SET(pexit->exit_info, EX_CLOSED))
         break;

      if (room_is_private(pexit->to_room) && ch->level < LEVEL_STAFF)
         break;

      char_from_room(ch);
      char_to_room(ch, pexit->to_room);

      if ((victim = get_char_wilder(ch, name)) != NULL)
      {
         char_from_room(ch);
         char_to_room(ch, was_in_room);
         return victim;
      }

      switch (ch->in_room->sector_type)
      {
         default:
            dist++;
            break;
         case SECT_AIR:
            if (number_percent() < 80)
               dist++;
            break;
         case SECT_INSIDE:
         case SECT_FIELD:
         case SECT_UNDERGROUND:
            dist++;
            break;
         case SECT_FOREST:
         case SECT_CITY:
         case SECT_DESERT:
         case SECT_HILLS:
            dist += 2;
            break;
         case SECT_WATER_SWIM:
         case SECT_WATER_NOSWIM:
         case SECT_RIVER:
            dist += 3;
            break;
         case SECT_MOUNTAIN:
         case SECT_UNDERWATER:
         case SECT_OCEANFLOOR:
            dist += 4;
            break;
      }

      if (dist >= max_dist)
         break;

      dir = pexit->vdir;
      if ((pexit = get_exit(ch->in_room, dir)) == NULL)
         break;
   }

   char_from_room(ch);
   char_to_room(ch, was_in_room);

   return NULL;
}

//Use this till sqrt will work
int find_sqrt(int dist)
{
   int x = 1;

   for (;;)
   {
      if ((x * x) >= dist)
         return x;
      x++;
   }
   return 1;
}
int get_distform(int x, int y, int vx, int vy)
{
   int dist;
   int sq;

   dist = (((x - vx) * (x - vx)) + ((y - vy) * (y - vy)));
   sq = find_sqrt(dist);
   return sq;
}


/*int get_distform(int x, int y, int vx, int vy)
{
   double dist;
   int dist2;
   int sq;
   sq = (((x-vx)*(x-vx))+((y-vy)*(y-vy)));
   dist = sqrt(sq);
   dist2 = dist;
   return dist2;
} */
void do_aim(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim = NULL;
   CHAR_DATA *vch;
   CHAR_DATA *vtarget;
   OBJ_DATA *bow;
   int shortdist = 10;
   int max_dist;

   if (IS_NPC(ch))
   {
      send_to_char("This command is for PCs only.\n\r", ch);
      return;
   }
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("You can only use this command while in the Wildereness.\n\r", ch);
      return;
   }
   if ((bow = get_eq_char(ch, WEAR_MISSILE_WIELD)) == NULL)
   {
      send_to_char("You need to be wielding a Missile Weapon to aim.\n\r", ch);
      return;
   }
   max_dist = URANGE(1, bow->value[4], 10);

   if (argument[0] == '\0')
   {
      for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
      {
         if ((IN_WILDERNESS(ch) && !IN_WILDERNESS(vch)) || (!IN_WILDERNESS(ch) && IN_WILDERNESS(vch)))
            continue;

         if (vch == ch->pcdata->pet || vch == ch->pcdata->mount)
            continue;

         if (!IS_NPC(vch))
            continue;

         if (IN_WILDERNESS(ch))
            if (get_distform(ch->coord->x, ch->coord->y, vch->coord->x, vch->coord->y) <= shortdist)
            {
               victim = vch;
               shortdist = get_distform(ch->coord->x, ch->coord->y, vch->coord->x, vch->coord->y);
            }
      }
      if (victim)
      {
         ch->pcdata->aimtarget = victim;
         ch_printf(ch, "You are now aiming at %s.\n\r", PERS_MAP(ch->pcdata->aimtarget, ch));
         return;
      }
      else
      {
         send_to_char("You could not find anyone close enough to aim at.\n\r", ch);
         return;
      }
   }
   else
   {
      if ((vtarget = get_char_wilder(ch, argument)) == NULL)
      {
         ch_printf(ch, "Hard to aim at %s when he/she/it does not exist.\n\r", argument);
         return;
      }
      // Lets Find a mob or two...
      for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
      {
         if (!str_cmp(vtarget->name, vch->name))
         {
            if ((IN_WILDERNESS(ch) && !IN_WILDERNESS(vch)) || (!IN_WILDERNESS(ch) && IN_WILDERNESS(vch)))
               continue;

            if (vch == ch->pcdata->pet || vch == ch->pcdata->mount)
               continue;

            if (IN_WILDERNESS(ch))
               if (get_distform(ch->coord->x, ch->coord->y, vch->coord->x, vch->coord->y) <= shortdist)
               {
                  victim = vch;
                  shortdist = get_distform(ch->coord->x, ch->coord->y, vch->coord->x, vch->coord->y);
               }
         }
      }
      if (victim == NULL)
      {
         ch_printf(ch, "Hard to aim at %s when he/she/it does not exist.\n\r", argument);
         return;
      }
      else
      {
         ch->pcdata->aimtarget = victim;
         ch_printf(ch, "You are now aiming at %s.\n\r", PERS_MAP(ch->pcdata->aimtarget, ch));
         return;
      }
   }
}

// valid dir or not?
int get_valid_dir(char *dir)
{
   if (!str_cmp(dir, "n") || !str_cmp(dir, "e") || !str_cmp(dir, "s") || !str_cmp(dir, "w")
      || !str_cmp(dir, "north") || !str_cmp(dir, "east") || !str_cmp(dir, "south") || !str_cmp(dir, "west"))
      return 1;

   if (!str_cmp(dir, "ne") || !str_cmp(dir, "nw") || !str_cmp(dir, "sw") || !str_cmp(dir, "se")
      || !str_cmp(dir, "northeast") || !str_cmp(dir, "northwest") || !str_cmp(dir, "southwest")
      || !str_cmp(dir, "southeast") || !str_cmp(dir, "up") || !str_cmp(dir, "u") || !str_cmp(dir, "down") || !str_cmp(dir, "d"))
      return 2;

   return -1;
}
void back_to_room(CHAR_DATA * ch, int x, int y, int map)
{
   ch->coord->x = x;
   ch->coord->y = y;
   ch->map = map;
}
CHAR_DATA *find_victim_path(CHAR_DATA * ch, int dir, int range)
{
   CHAR_DATA *vch;
   CHAR_DATA *victim = NULL;
   sh_int shortdis = range;

   // Going to find if there is a mob OR player in that direction and
   // fire at it, oh yeah :-)
   for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
   {
      if ((IN_WILDERNESS(ch) && !IN_WILDERNESS(vch)) || (!IN_WILDERNESS(ch) && IN_WILDERNESS(vch)))
         continue;

      switch (dir)
      {
         case 0:

            if ((ch->coord->x == vch->coord->x) && ((ch->coord->y - vch->coord->y <= range) && (ch->coord->y - vch->coord->y > 0)))
            {
               if (ch->coord->y - vch->coord->y < shortdis)
               {
                  shortdis = ch->coord->y - vch->coord->y;
                  victim = vch;
               }
            }
            break;
         case 1:
            if ((ch->coord->y == vch->coord->y) && ((vch->coord->x - ch->coord->x <= range) && (vch->coord->x - ch->coord->x > 0)))
            {
               if (ch->coord->x - vch->coord->x < shortdis)
               {
                  shortdis = ch->coord->x - vch->coord->x;
                  victim = vch;
               }
            }
            break;
         case 2:
            if ((ch->coord->x == vch->coord->x) && ((vch->coord->y - ch->coord->y <= range) && (vch->coord->y - ch->coord->y > 0)))
            {
               if (vch->coord->y - ch->coord->y < shortdis)
               {
                  shortdis = ch->coord->y = vch->coord->y;
                  victim = vch;
               }
            }
            break;
         case 3:
            if ((ch->coord->y == vch->coord->y) && ((ch->coord->x - vch->coord->x <= range) && (ch->coord->x - vch->coord->x > 0)))
            {
               if (vch->coord->x - ch->coord->x < shortdis)
               {
                  shortdis = ch->coord->x = vch->coord->x;
                  victim = vch;
               }
            }
            break;
      }
   }
   return victim;
}

int get_target_dir(CHAR_DATA * ch, CHAR_DATA * victim)
{
   int x, y, vx, vy;

   x = ch->coord->x;
   y = ch->coord->y;
   vx = victim->coord->x;
   vy = victim->coord->y;

   if (x == vx && y == vy)
      return -2;

   if (y > vy && x == vx) //n
      return 0;
   if (y == vy && x < vx) //e
      return 1;
   if (y < vy && x == vx) //s
      return 2;
   if (y == vy && x > vx) //w
      return 3;

   if (y > vy && x < vx) // ne
      return 6;
   if (y > vy && x > vx) // nw
      return 7;
   if (y < vy && x > vx) // sw
      return 9;
   if (y < vy && x < vx) // se
      return 8;

   return -1;
}

/*
 * Generic use ranged attack function			-Thoric & Tricops
 */
ch_ret ranged_attack(CHAR_DATA * ch, char *argument, OBJ_DATA * weapon, OBJ_DATA * projectile, sh_int dt, sh_int range)
{
   CHAR_DATA *victim, *vch, *ach;
   EXIT_DATA *pexit = NULL;
   ROOM_INDEX_DATA *was_in_room;
   char arg[MIL];
   char arg1[MIL];
   char temp[MIL];
   char buf[MSL];
   int startp[3];
   int startm[3];
   int godist;
   int targeted;
   SKILLTYPE *skill = NULL;
   sh_int dir = -1, dist = 0, color = AT_GREY;
   char *dtxt = "somewhere";
   char *stxt = "burst of energy";
   int count;
   int fnd = 0;

   startm[0]=startm[1]=startm[2] = -1;

   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
      ach = ch->hunting->who;
   else if (!IS_NPC(ch) && ch->pcdata->aimtarget)
      ach = ch->pcdata->aimtarget;
   else
      ach = NULL;

   if (argument && argument[0] != '\0' && argument[0] == '\'')
   {
      one_argument(argument, temp);
      argument = temp;
   }

   victim = NULL;
   vch = NULL;
   startp[0] = ch->coord->x;
   startp[1] = ch->coord->y;
   startp[2] = ch->map;

   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg1);

   if (!str_cmp(arg, "target"))
      targeted = 1;
   else
      targeted = 0;

   if (arg[0] == '\0')
   {
      if (ch->fighting && ch->fighting->who)
      {
         sprintf(arg, "%s", ch->fighting->who->name);
      }
      else
      {
         send_to_char("Where?  At who?\n\r", ch);
         return rNONE;
      }
   }
   projectile->coord->x = ch->coord->x;
   projectile->coord->y = ch->coord->y;
   projectile->map = ch->map;

   victim = NULL;
   //Fire in a direction in the Wilderness
   if (IN_WILDERNESS(ch))
   {
      //get_dir, just will return -1 if the direction is not found, not fool proof though
      // get_char_room, just another option, yay
      if (get_valid_dir(arg) == -1 && !targeted)
      {
         if ((victim = get_char_room_new(ch, arg, 1)) == NULL && !targeted)
         {
            send_to_char("Aim in what direction?\n\r", ch);
            return rNONE;
         }
         else
         {
            if (targeted)
            {
               if (get_char_room_new(ch, ach->name, 1) != NULL)
                  victim = get_char_room_new(ch, ach->name, 1);
            }
         /*   if (who_fighting(ch) == victim)
            {
               send_to_char("They are too close to release that type of attack!\n\r", ch);
               return rNONE;
            } */
         }
      }
      else
      {
         if (get_valid_dir(arg) == 2)
         {
            send_to_char("Can only shoot north, south, east, or west.\n\r", ch);
            return rNONE;
         }
         dir = get_dir(arg);
      }
   }
   else
   {
      /* get an exit or a victim */
      if ((pexit = find_door(ch, arg, TRUE)) == NULL)
      {
         if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
         {
            send_to_char("Aim in what direction?\n\r", ch);
            return rNONE;
         }
         else
         {
            /*if (who_fighting(ch) == victim)
            {
               send_to_char("They are too close to release that type of attack!\n\r", ch);
               return rNONE;
            }*/
         }
      }
      else
         dir = pexit->vdir;


      /* check for ranged attacks from private rooms, etc */
      if (!victim)
      {
         if (xIS_SET(ch->in_room->room_flags, ROOM_PRIVATE) || xIS_SET(ch->in_room->room_flags, ROOM_SOLITARY))
         {
            send_to_char("You cannot perform a ranged attack from a private room.\n\r", ch);
            return rNONE;
         }
         if (wIS_SET(ch, ROOM_PRIVATE) || wIS_SET(ch, ROOM_SOLITARY))
         {
            send_to_char("You cannot perform a ranged attack from a private room.\n\r", ch);
            return rNONE;
         }
         if (ch->in_room->tunnel > 0)
         {
            count = 0;
            for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
               ++count;
            if (count >= ch->in_room->tunnel)
            {
               send_to_char("This room is too cramped to perform such an attack.\n\r", ch);
               return rNONE;
            }
         }
      }
   }
   if (IS_VALID_SN(dt))
      skill = skill_table[dt];
   if (!IN_WILDERNESS(ch))
   {
      if (pexit && !pexit->to_room)
      {
         send_to_char("Are you expecting to fire through a wall!?\n\r", ch);
         return rNONE;
      }


      /* Check for obstruction */
      if (pexit && IS_SET(pexit->exit_info, EX_CLOSED))
      {
         if (IS_SET(pexit->exit_info, EX_SECRET) || IS_SET(pexit->exit_info, EX_DIG))
            send_to_char("Are you expecting to fire through a wall!?\n\r", ch);
         else
            send_to_char("Are you expecting to fire through a door!?\n\r", ch);
         return rNONE;
      }
   }
   vch = NULL;
   if (IN_WILDERNESS(ch))
   {
      if (arg1[0] != '\0' || targeted)
      {
         if ((vch = scan_wilderness_for_victim(ch, dir, arg1)) == NULL && !targeted)
         {
            send_to_char("You cannot see your target.\n\r", ch);
            return rNONE;
         }
         if (targeted)
            vch = ach;
         startm[0] = vch->coord->x;
         startm[1] = vch->coord->y;
         startm[2] = vch->map;
         /* can't properly target someone heavily in battle */
         if (vch->num_fighting > max_fight(vch))
         {
            send_to_char("There is too much activity there for you to get a clear shot.\n\r", ch);
            return rNONE;
         }
      }
   }
   else
   {
      if (pexit && arg1[0] != '\0')
      {
         if ((vch = scan_for_victim(ch, pexit, arg1)) == NULL)
         {
            send_to_char("You cannot see your target.\n\r", ch);
            return rNONE;
         }

         /*don't allow attacks on mobs that are in a no-missile room --Shaddai */
         if (xIS_SET(vch->in_room->room_flags, ROOM_NOMISSILE))
         {
            send_to_char("You can't get a clean shot off.\n\r", ch);
            return rNONE;
         }

         /* can't properly target someone heavily in battle */
         if (vch->num_fighting > max_fight(vch))
         {
            send_to_char("There is too much activity there for you to get a clear shot.\n\r", ch);
            return rNONE;
         }
      }
   }
   if (vch)
   {
      if (!IS_NPC(vch) && !IS_NPC(ch) && xIS_SET(ch->act, PLR_NICE))
      {
         send_to_char("Your too nice to do that!\n\r", ch);
         return rNONE;
      }
      if (vch && is_safe(ch, vch))
         return rNONE;
   }
   if (targeted)
   {
      dir = get_target_dir(ch, ach);
      if (dir == -2)
         victim = ach;
      if (dir == -1)
      {
         send_to_char("Error with your target, terminating.\n\r", ch);
         bug("%s firing direction is not working", ch->name);
         return rNONE;
      }
      if ((IN_WILDERNESS(ch) && get_distform(ch->coord->x, ch->coord->y, vch->coord->x, vch->coord->y) > range))
      {
         if (weapon)
         {
            act(AT_GREY, "$N is too far out of range to fire at.", ch, projectile, ach, TO_CHAR);
            act(AT_GREY, "$n prepares to fire at $N, but suddenly stops.", ch, projectile, ach, TO_ROOM);
            return rNONE;
         }
         else
         {
            act(AT_GREY, "$N is too far out of range to throw at.", ch, projectile, ach, TO_CHAR);
            act(AT_GREY, "$n prepares to throw at $N, but suddenly stops.", ch, projectile, ach, TO_ROOM);
            return rNONE;
         }
      }
   }
   was_in_room = ch->in_room;
   if (projectile)
   {
      separate_obj(projectile);
      if (IN_WILDERNESS(ch))
         back_to_room(ch, startp[0], startp[1], startp[2]);
      if (dir > -1 || pexit)
      {
         if (weapon)
         {
            act(AT_GREY, "You fire $p $T.", ch, projectile, dir_name[dir], TO_CHAR);
            act(AT_GREY, "$n fires $p $T.", ch, projectile, dir_name[dir], TO_ROOM);
         }
         else
         {
            act(AT_GREY, "You throw $p $T.", ch, projectile, dir_name[dir], TO_CHAR);
            act(AT_GREY, "$n throw $p $T.", ch, projectile, dir_name[dir], TO_ROOM);
         }
      }
      else
      {
         if (weapon)
         {
            act(AT_GREY, "You fire $p at $N.", ch, projectile, victim, TO_CHAR);
            act(AT_GREY, "$n fires $p at $N.", ch, projectile, victim, TO_NOTVICT);
            act(AT_GREY, "$n fires $p at you!", ch, projectile, victim, TO_VICT);
         }
         else
         {
            act(AT_GREY, "You throw $p at $N.", ch, projectile, victim, TO_CHAR);
            act(AT_GREY, "$n throws $p at $N.", ch, projectile, victim, TO_NOTVICT);
            act(AT_GREY, "$n throws $p at you!", ch, projectile, victim, TO_VICT);
         }
      }
   }
   else if (skill)
   {
      if (skill->noun_damage && skill->noun_damage[0] != '\0')
         stxt = skill->noun_damage;
      else
         stxt = skill->name;
      /* a plain "spell" flying around seems boring */
      if (!str_cmp(stxt, "spell"))
         stxt = "magical burst of energy";
      if (skill->type == SKILL_SPELL)
      {
         color = AT_MAGIC;
         if (dir || pexit)
         {
            act(AT_MAGIC, "You release $t $T.", ch, aoran(stxt), dir_name[dir], TO_CHAR);
            act(AT_MAGIC, "$n releases $s $t $T.", ch, stxt, dir_name[dir], TO_ROOM);
         }
         else
         {
            act(AT_MAGIC, "You release $t at $N.", ch, aoran(stxt), victim, TO_CHAR);
            act(AT_MAGIC, "$n releases $s $t at $N.", ch, stxt, victim, TO_NOTVICT);
            act(AT_MAGIC, "$n releases $s $t at you!", ch, stxt, victim, TO_VICT);
         }
      }
   }
   else
   {
      bug("Ranged_attack: no projectile, no skill dt %d", dt);
      return rNONE;
   }
   /* victim in same room */
   if (victim)
   {
      if (sysdata.resetgame && check_powerlevel(ch, victim) 
      &&  (get_player_statlevel(ch) > get_player_statlevel(victim) + 5))
      {
         send_to_char("You cannot fire on that individual due to Power Level Restrictions.\n\r", ch);
         return rNONE;
      }
      check_illegal_pk(ch, victim);
      check_attacker(ch, victim);
      return ranged_got_target(ch, victim, weapon, projectile, 0, dt, stxt, color);
   }

   /* assign scanned victim */
   victim = vch;
   /* reverse direction text from move_char */
   if (!IN_WILDERNESS(ch))
   {
      dtxt = rev_exit(pexit->vdir);

      while (dist <= range)
      {
         char_from_room(ch);
         char_to_room(ch, pexit->to_room);

         if (IS_SET(pexit->exit_info, EX_CLOSED))
         {
            /* whadoyahknow, the door's closed */
            if (projectile)
               sprintf(buf, "You see your %s pierce a door in the distance to the %s.", myobj(projectile), dir_name[dir]);
            else
               sprintf(buf, "You see your %s hit a door in the distance to the %s.", stxt, dir_name[dir]);
            act(color, buf, ch, NULL, NULL, TO_CHAR);
            if (projectile)
            {
               sprintf(buf, "$p flies in from %s and implants itself solidly in the %sern door.", dtxt, dir_name[dir]);
               act(color, buf, ch, projectile, NULL, TO_ROOM);
            }
            else
            {
               sprintf(buf, "%s flies in from %s and implants itself solidly in the %sern door.", aoran(stxt), dtxt, dir_name[dir]);
               buf[0] = UPPER(buf[0]);
               act(color, buf, ch, NULL, NULL, TO_ROOM);
            }
            break;
         }

         /* no victim? pick a random one */
         if (!victim)
         {
            for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
            {
               if (((IS_NPC(ch) && !IS_NPC(vch)) || (!IS_NPC(ch) && IS_NPC(vch))) && number_bits(1) == 0)
               {
                  victim = vch;
                  break;
               }
            }
            if (victim && is_safe(ch, victim))
            {
               char_from_room(ch);
               char_to_room(ch, was_in_room);
               return rNONE;
            }
         }

         /* In the same room as our victim? */
         if (victim && ch->in_room == victim->in_room)
         {
            if (projectile)
               act(color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM);
            else
               act(color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM);

            /* get back before the action starts */
            char_from_room(ch);
            char_to_room(ch, was_in_room);

            if (sysdata.resetgame && check_powerlevel(ch, victim) 
            &&  (get_player_statlevel(ch) > get_player_statlevel(victim) + 5))
            {
               send_to_char("You cannot fire on that individual due to Power Level Restrictions.\n\r", ch);
               return rNONE;
            }
            check_illegal_pk(ch, victim);
            check_attacker(ch, victim);
            return ranged_got_target(ch, victim, weapon, projectile, dist, dt, stxt, color);
         }

         if (dist == range)
         {
            if (projectile)
            {
               if (victim)
                  check_attacker(ch, victim);
               act(color, "Your $t falls harmlessly to the ground to the $T.", ch, myobj(projectile), dir_name[dir], TO_CHAR);
               act(color, "$p flies in from $T and falls harmlessly to the ground here.", ch, projectile, dtxt, TO_ROOM);
               if (projectile->in_obj)
                  obj_from_obj(projectile);
               if (projectile->carried_by)
                  obj_from_char(projectile);
               obj_to_room(projectile, ch->in_room, ch);
            }
            else
            {
               act(color, "Your $t fizzles out harmlessly to the $T.", ch, stxt, dir_name[dir], TO_CHAR);
               act(color, "$t flies in from $T and fizzles out harmlessly.", ch, aoran(stxt), dtxt, TO_ROOM);
            }
            break;
         }

         if ((pexit = get_exit(ch->in_room, dir)) == NULL)
         {
            if (projectile)
            {
               act(color, "Your $t hits a wall and bounces harmlessly to the ground to the $T.", ch, myobj(projectile), dir_name[dir], TO_CHAR);
               act(color, "$p strikes the $Tsern wall and falls harmlessly to the ground.", ch, projectile, dir_name[dir], TO_ROOM);
               if (projectile->in_obj)
                  obj_from_obj(projectile);
               if (projectile->carried_by)
                  obj_from_char(projectile);
               obj_to_room(projectile, ch->in_room, ch);
            }
            else
            {
               act(color, "Your $t harmlessly hits a wall to the $T.", ch, stxt, dir_name[dir], TO_CHAR);
               act(color, "$t strikes the $Tsern wall and falls harmlessly to the ground.", ch, aoran(stxt), dir_name[dir], TO_ROOM);
            }
            break;
         }
         if (projectile)
            act(color, "$p flies in from $T.", ch, projectile, dtxt, TO_ROOM);
         else
            act(color, "$t flies in from $T.", ch, aoran(stxt), dtxt, TO_ROOM);
         dist++;
      }
   }
   else
   {
      dtxt = rev_exit(dir);
      if (IN_WILDERNESS(ch))
         back_to_room(ch, startp[0], startp[1], startp[2]);
      if (victim && !targeted)
      {
         switch (dir)
         {
            case 0:
               if ((ch->coord->x == victim->coord->x) && ((ch->coord->y - victim->coord->y <= range) && (ch->coord->y - victim->coord->y > 0)))
                  fnd = 1;
               break;
            case 1:
               if ((ch->coord->y == victim->coord->y) && ((victim->coord->x - ch->coord->x <= range) && (victim->coord->x - ch->coord->x > 0)))
                  fnd = 1;
               break;
            case 2:
               if ((ch->coord->x == victim->coord->x) && ((victim->coord->y - ch->coord->y <= range) && (victim->coord->y - ch->coord->y > 0)))
                  fnd = 1;
               break;
            case 3:
               if ((ch->coord->y == victim->coord->y) && ((ch->coord->x - victim->coord->x <= range) && (ch->coord->x - victim->coord->x > 0)))
                  fnd = 1;
               break;
         }
      }
      else
      {
         if (!targeted)
         {
            victim = find_victim_path(ch, dir, range);
            if (victim)
            {
               startm[0] = victim->coord->x;
               startm[1] = victim->coord->y;
               startm[2] = victim->map;
               fnd = 1;
            }

         }
         else
            fnd = 1;
      }
      if (victim)
         if (IN_WILDERNESS(victim) && startm[0] != -1 && startm[1] != -1 && startm[2] != -1)
            back_to_room(victim, startm[0], startm[1], startm[2]);

      if (fnd == 1)
      {
         projectile->coord->x = victim->coord->x;
         projectile->coord->y = victim->coord->y;
         projectile->map = victim->map;
         if (projectile)
            act(color, "$p flies in from $T.", victim, projectile, dtxt, TO_ROOM);
         else
            act(color, "$t flies in from $T.", victim, aoran(stxt), dtxt, TO_ROOM);

         if (sysdata.resetgame && check_powerlevel(ch, victim) 
         &&  (get_player_statlevel(ch) > get_player_statlevel(victim) + 5))
         {
            send_to_char("You cannot fire on that individual due to Power Level Restrictions.\n\r", ch);
            return rNONE;
         }
         check_illegal_pk(ch, victim);
         check_attacker(ch, victim);
         return ranged_got_target(ch, victim, weapon, projectile, dist, dt, stxt, color);
      }
      if (!victim || fnd == 0)
      {
         godist = 0;
         switch (dir)
         {
            case 0:
            case 3:
               godist = -range - (dice(1, 3));
               break;
            case 1:
            case 2:
               godist = range + dice(1, 3);
               break;
         }
         if (projectile)
         {
            act(color, "Your $t flies through the air, but strikes nothing.", ch, myobj(projectile), dir_name[dir], TO_CHAR);
            if (projectile->in_obj)
               obj_from_obj(projectile);
            if (projectile->carried_by)
               obj_from_char(projectile);
            obj_to_room(projectile, ch->in_room, ch);
            if (dir == 0 || dir == 2)
               projectile->coord->y = URANGE(1, projectile->coord->y + godist, MAX_Y);
            else
               projectile->coord->x = URANGE(1, projectile->coord->x + godist, MAX_X);
         }
         return rNONE;
      }
   }
   char_from_room(ch);
   char_to_room(ch, was_in_room);

   return rNONE;
}

/* Bowfire code -- actual firing function */
void do_fire(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim = NULL;
   OBJ_DATA *arrow;
   OBJ_DATA *bow;
   sh_int max_dist;

   if ((bow = get_eq_char(ch, WEAR_MISSILE_WIELD)) == NULL)
   {
      send_to_char("But you are not wielding a missile weapon!!\n\r", ch);
      return;
   }

   one_argument(argument, arg);
   if (arg[0] == '\0' && ch->fighting == NULL)
   {
      send_to_char("Fire at whom or what?\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "none") || !str_cmp(arg, "self") || victim == ch)
   {
      send_to_char("How exactly did you plan on firing at yourself?\n\r", ch);
      return;
   }

   if ((arrow = get_eq_char(ch, WEAR_NOCKED)) == NULL)
   {
      do_nock(ch, "");
      if ((arrow = get_eq_char(ch, WEAR_NOCKED)) == NULL)
      {
         send_to_char("You are not holding a projectile!\n\r", ch);
         return;
      }
   }

   if (arrow->item_type != ITEM_PROJECTILE)
   {
      send_to_char("You are not holding a projectile!\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "target"))
   {
      if (ch->pcdata->aimtarget == NULL)
      {
         send_to_char("Your target does not exist anymore, sorry.\n\r", ch);
         return;
      }
      if (!IN_WILDERNESS(ch))
      {
         send_to_char("Can only fire at a target out in the Wilderness.\n\r", ch);
         return;
      }
   }

   /* modify maximum distance based on bow-type and ch's class/str/etc */
   max_dist = URANGE(1, bow->value[4], 10);

   if (bow->value[7] != arrow->value[7])
   {
      char *msg = "You have nothing to fire...\n\r";

      send_to_char(msg, ch);
      return;
   }

   /* Add wait state to fire for pkill, etc... */
   ch->fight_timer = get_btimer(ch, 1000, NULL);


   /* handle the ranged attack */
   ranged_attack(ch, argument, bow, arrow, TYPE_HIT, max_dist);

   return;
}

/*
 * Attempt to fire at a victim.
 * Returns FALSE if no attempt was made
 */
bool mob_fire(CHAR_DATA * ch, char *name, int vdir)
{
   OBJ_DATA *arrow;
   OBJ_DATA *bow;
   sh_int max_dist;
   char buf[MSL];

   if (is_room_safe(ch))
      return FALSE;

   if ((bow = get_eq_char(ch, WEAR_MISSILE_WIELD)) == NULL)
      return FALSE;

   if ((arrow = get_eq_char(ch, WEAR_NOCKED)) == NULL)
   {
      do_nock(ch, "");
      if ((arrow = get_eq_char(ch, WEAR_NOCKED)) == NULL)
         return FALSE;
   }

   if (arrow->item_type != ITEM_PROJECTILE)
      return FALSE;

   if (bow->value[7] != arrow->value[7])
      return FALSE;

   /* modify maximum distance based on bow-type and ch's class/str/etc */
   max_dist = URANGE(1, bow->value[4], 10);
   if (xIS_SET(ch->act, ACT_MILITARY))
      sprintf(buf, "target");
   else
      sprintf(buf, "%s %s", dir_name[vdir], name);
   ranged_attack(ch, buf, bow, arrow, TYPE_HIT, max_dist);
   /* Add wait state to fire for pkill, etc... */
   ch->fight_timer = get_btimer(ch, 1000, NULL);

   return TRUE;
}
