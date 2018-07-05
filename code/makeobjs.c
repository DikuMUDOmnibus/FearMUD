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
 *			Specific object creation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"



/*
 * Make a fire.
 */
void make_fire(ROOM_INDEX_DATA * in_room, sh_int timer)
{
   OBJ_DATA *fire;

   fire = create_object(get_obj_index(OBJ_VNUM_FIRE), 0);
   fire->timer = number_fuzzy(timer);
   obj_to_room(fire, in_room, in_room->first_person);
   return;
}

/*
 * Make a trap.
 */
OBJ_DATA *make_trap(int v0, int v1, int v2, int v3)
{
   OBJ_DATA *trap;

   trap = create_object(get_obj_index(OBJ_VNUM_TRAP), 0);
   trap->timer = 0;
   trap->value[0] = v0;
   trap->value[1] = v1;
   trap->value[2] = v2;
   trap->value[3] = v3;
   return trap;
}


/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps(OBJ_DATA * obj, CHAR_DATA *ch)
{
   char buf[MSL];
   OBJ_DATA *scraps, *tmpobj;

   separate_obj(obj);
   scraps = create_object(get_obj_index(OBJ_VNUM_SCRAPS), 0);
   scraps->timer = number_range(5, 15);

   if (IS_OBJ_STAT(obj, ITEM_ONMAP))
   {
      SET_OBJ_STAT(scraps, ITEM_ONMAP);
      scraps->map = obj->map;
      scraps->coord->x = obj->coord->x;
      scraps->coord->y = obj->coord->y;
   }

   /* don't make scraps of scraps of scraps of ... */
   if (obj->pIndexData->vnum == OBJ_VNUM_SCRAPS)
   {
      STRFREE(scraps->short_descr);
      scraps->short_descr = STRALLOC("some debris");
      STRFREE(scraps->description);
      scraps->description = STRALLOC("Bits of debris lie on the ground here.");
   }
   else
   {
      sprintf(buf, scraps->short_descr, obj->short_descr);
      STRFREE(scraps->short_descr);
      scraps->short_descr = STRALLOC(buf);
      sprintf(buf, scraps->description, obj->short_descr);
      STRFREE(scraps->description);
      scraps->description = STRALLOC(buf);
   }

   if (obj->carried_by)
   {
      act(AT_OBJECT, "$p falls to the ground in scraps!", obj->carried_by, obj, NULL, TO_CHAR);
      if (obj == get_eq_char(obj->carried_by, WEAR_WIELD) && (tmpobj = get_eq_char(obj->carried_by, WEAR_DUAL_WIELD)) != NULL)
         tmpobj->wear_loc = WEAR_WIELD;

      obj_to_room(scraps, obj->carried_by->in_room, ch);
   }
   else if (obj->in_room)
   {
      if (ch)
      {
         act(AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_ROOM);
         act(AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_CHAR);
      }
      obj_to_room(scraps, obj->in_room, ch);
   }
   if ((obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_KEYRING
         || obj->item_type == ITEM_QUIVER || obj->item_type == ITEM_CORPSE_PC) && obj->first_content)
   {
      if (ch && ch->in_room)
      {
         act(AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_ROOM);
         act(AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_CHAR);
      }
      if (obj->carried_by)
         empty_obj(obj, NULL, obj->carried_by->in_room);
      else if (obj->in_room)
         empty_obj(obj, NULL, obj->in_room);
      else if (obj->in_obj)
         empty_obj(obj, obj->in_obj, NULL);
   }
   extract_obj(obj);
}


void update_container(OBJ_DATA * corpse, int x, int y, int map, int fx, int fy, int fmap)
{
   OBJ_DATA *obj;

   for (obj = corpse->first_content; obj; obj = obj->next_content)
   {
      if (x > -1 || y > -1 || map > -1)
         REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
      else
         SET_OBJ_STAT(obj, ITEM_ONMAP);

      obj->coord->x = x;
      obj->coord->y = y;
      obj->map = map;

      if (obj->first_content)
         update_container(obj, x, y, map, 0, 0, 0);

   }
}

bool check_quest_drop(CHAR_DATA *victim, OBJ_DATA *obj)
{
   if (!victim->pcdata->quest || victim->pcdata->quest->questarea != victim->in_room->area)
      return FALSE;
   if ((get_ch_carry_number(victim) + obj->count) > can_carry_n(victim))
      return FALSE;
   if ((get_ch_carry_weight(victim) + get_obj_weight(obj)) > can_carry_w(victim))
      return FALSE;
      
   if (obj->item_type == ITEM_KEY)
      return TRUE;
   if (obj->item_type == ITEM_TGEM)
      return TRUE;
   if (obj->item_type == ITEM_QTOKEN)
      return TRUE;
   return FALSE;
}
/*
 * Make a corpse out of a character.
 */
void make_corpse(CHAR_DATA * ch, CHAR_DATA * killer)
{
   char buf[MSL];
   OBJ_DATA *corpse;
   OBJ_DATA *treas;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   OBJ_DATA *box;
   char *name;
   int chances;
   int counter;

   if (IS_NPC(ch))
   {
      name = ch->short_descr;
      corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_NPC), 0);
      corpse->timer = 6;
      if (ch->gold > 0)
      {
         if (ch->in_room)
         {
            ch->in_room->area->gold_looted += ch->gold;
            sysdata.global_looted += ch->gold / 100;
         }
         obj_to_obj(create_money(ch->gold), corpse);
         ch->gold = 0;
      }

      /* FOR TREASURE SYSTEM -- Skan 1/4/01 */
      chances = number_range(1, 2);
      counter = 0;
      if (sysdata.gem_vnum > 0 && get_obj_index(sysdata.gem_vnum))
      {
         int lmod;

         lmod = (get_curr_lck(killer) - 14) * 4;
         
         if (!IN_WILDERNESS(ch))
            lmod = -10; //only in wilderness for treasure;
            
         if (ch->race < 0 || ch->race >= MAX_RACE || xIS_SET(ch->act, ACT_EXTRACTMOB) || xIS_SET(ch->act, ACT_KINGDOMMOB)
         || ch->pIndexData->vnum < OVERLAND_LOW_MOB || ch->pIndexData->vnum > OVERLAND_HI_MOB || xIS_SET(ch->act, ACT_MILITARY))
            lmod = -10; //only in pc mobs

         if (tchance(10 + lmod))
         {
            box = generate_tbox(ch);
            if (tchance(lmod/2))
            {
               treas = generate_treasure(ch);
               if (treas)
                  obj_to_obj(treas, corpse);
               if (box)
                  obj_to_obj(box, corpse);
            }
            else
            {
               if (box)
                  obj_to_obj(box, corpse);
            }
         }
      }
      else
         bug("Need to set an actual default gem vnum with cset");


/* Cannot use these!  They are used.
	corpse->value[0] = (int)ch->pIndexData->vnum;
	corpse->value[1] = (int)ch->max_hit;
*/
/*	Using corpse cost to cheat, since corpses not sellable */
      corpse->cost = (-(int) ch->pIndexData->vnum);
      corpse->value[2] = corpse->timer;
   }
   else
   {
      name = ch->name;
      corpse = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
      if (in_arena(ch))
         corpse->timer = 0;
      else
         corpse->timer = 2880;
      corpse->value[2] = (int) (corpse->timer / 480);
      corpse->value[4] = 1;
      if (!IS_NPC(ch))
         xSET_BIT(corpse->extra_flags, ITEM_CLANCORPSE);
      /* Pkill corpses get save timers, in ticks (approx 70 seconds)
         This should be anough for the killer to type 'get all corpse'. */
      if (!IS_NPC(ch) && !IS_NPC(killer))
         corpse->value[3] = 1;
      else
         corpse->value[3] = 0;
   }

   if (!IS_NPC(ch) && !IS_NPC(killer) && ch != killer)
   {
      sprintf(buf, "%s", killer->name);
      STRFREE(corpse->action_desc);
      corpse->action_desc = STRALLOC(buf);
   }

   /* Added corpse name - make locate easier , other skills */
   sprintf(buf, "corpse PC");
   STRFREE(corpse->name);
   corpse->name = STRALLOC(buf);

   sprintf(buf, corpse->short_descr, name);
   STRFREE(corpse->short_descr);
   corpse->short_descr = STRALLOC(buf);

   if (IS_NPC(ch))
   {
      sprintf( buf, corpse->description, name );
      STRFREE( corpse->description );
      corpse->description = STRALLOC( buf );
   }
   else
   {
      sprintf(buf, "A corpse of a once breathing individual is here.");
      STRFREE(corpse->description);
      corpse->description = STRALLOC(buf);
   }

   for (obj = ch->first_carrying; obj; obj = obj_next)
   {
      obj_next = obj->next_content;
      obj_from_char(obj);
      if (IS_OBJ_STAT(obj, ITEM_INVENTORY) || IS_OBJ_STAT(obj, ITEM_DEATHROT))
         extract_obj(obj);
      else if (!IS_NPC(killer) && IS_NPC(ch) && xIS_SET(killer->act, PLR_QUESTLOOT) && killer != ch && check_quest_drop(killer, obj))
      {
         obj_to_char(obj, killer);
         if (obj->carried_by)
            act(AT_YELLOW, ">>>You loot $p&w&Y from the corpse<<<", killer, obj, NULL, TO_CHAR);
         else
            act(AT_ORANGE, ">>>You loot $p&w&O from the corpse, but you have no room for it!<<<", killer, obj, NULL, TO_CHAR);
      }
      else
         obj_to_obj(obj, corpse);
   }

   if (xIS_SET(ch->in_room->room_flags, ROOM_PERMDEATH))
   {
      ROOM_INDEX_DATA *proom;
      
      proom = get_room_index(ROOM_VNUM_TEMPLE);
      obj_to_room(corpse, proom, ch);
   }
   else
      obj_to_room(corpse, ch->in_room, ch);
   update_container(corpse, ch->coord->x, ch->coord->y, ch->map, 0, 0, 0);

   return;
}

void make_blood(CHAR_DATA * ch)
{
   OBJ_DATA *obj;

   if (!xIS_SET(ch->act, ACT_UNDEAD) && !xIS_SET(ch->act, ACT_LIVING_DEAD))
   {
      obj = create_object(get_obj_index(OBJ_VNUM_BLOOD), 0);
      obj->timer = number_range(2, 4);
      obj->value[1] = number_range(3, 15);
      obj_to_room(obj, ch->in_room, ch);
   }
}


void make_bloodstain(CHAR_DATA * ch)
{
   OBJ_DATA *obj;

   if (!xIS_SET(ch->act, ACT_UNDEAD) && !xIS_SET(ch->act, ACT_LIVING_DEAD))
   {
      obj = create_object(get_obj_index(OBJ_VNUM_BLOODSTAIN), 0);
      obj->timer = number_range(1, 2);
      obj_to_room(obj, ch->in_room, ch);
   }
}


/*
 * make some coinage
 */
OBJ_DATA *create_money(int amount)
{
   char buf[MSL];
   OBJ_DATA *obj;

   if (amount <= 0)
   {
      bug("Create_money: zero or negative money %d.", amount);
      amount = 1;
   }

   if (amount == 1)
   {
      obj = create_object(get_obj_index(OBJ_VNUM_MONEY_ONE), 0);
   }
   else
   {
      obj = create_object(get_obj_index(OBJ_VNUM_MONEY_SOME), 0);
      sprintf(buf, obj->short_descr, amount);
      STRFREE(obj->short_descr);
      obj->short_descr = STRALLOC(buf);
      obj->value[0] = amount;
   }

   return obj;
}
