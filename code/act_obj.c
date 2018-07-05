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
 *			   Object manipulation module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"
#include "bet.h"

extern int start_marketpid;

/*double sqrt( double x );*/
int sacall = 0;

/*
 * External functions
 */

void show_list_to_char args((OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing, const int iDefaultAction));
void write_corpses args((CHAR_DATA * ch, char *name, OBJ_DATA * objrem));

/*
 * Local functions.
 */
void get_obj args((CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container));

OBJ_DATA *recursive_note_find args((OBJ_DATA * obj, char *argument));

/*
 * how resistant an object is to damage				-Thoric
 */
sh_int get_obj_resistance(OBJ_DATA * obj, CHAR_DATA *ch)
{
   int resist;

   if (obj->item_type == ITEM_WEAPON)
      resist = obj->value[10];
   else
      resist = obj->value[4];  

   /* magical items are more resistant */
   if (IS_OBJ_STAT(obj, ITEM_MAGIC))
      resist += number_range(1, 2);
   /* glowing objects should have a little bonus */
   if (IS_OBJ_STAT(obj, ITEM_GLOW))
      resist += 1;
   /* lets make store inventory pretty tough */
   if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
      resist += 3;
   
   resist += ch->apply_armor;

   /* and lasty... take armor or weapon's condition into consideration */
   if (obj->item_type == ITEM_ARMOR)
      resist = resist * obj->value[3] / 1000;
   if (obj->item_type == ITEM_WEAPON)
      resist = resist * obj->value[0] / 1000;

   return URANGE(1, resist, 20);
}

bool is_kingdom_chest(OBJ_DATA * obj)
{
   KCHEST_DATA *kchest;

   for (kchest = first_kchest; kchest; kchest = kchest->next)
   {
      if (kchest->obj == obj)
         return TRUE;
   }
   return FALSE;
}

void get_obj(CHAR_DATA * ch, OBJ_DATA * obj, OBJ_DATA * container)
{
   CLAN_DATA *clan;
   int weight;
   int amt; /* gold per-race multipliers */
   char *pd;
   char name[MIL];
   CHAR_DATA *fch;

   if (!CAN_WEAR(obj, ITEM_TAKE) && (ch->level < sysdata.level_getobjnotake))
   {
      send_to_char("You can't take that.\n\r", ch);
      return;
   }
   if (obj->in_room)
   {
      for(fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room)
      {
         if (fch->on == obj)
         {
            send_to_char("You cannot pick up a piece of furniture with someone on it.\n\r", ch);
            return;
         }
      }
   }

   if (IS_SET(obj->magic_flags, ITEM_PKDISARMED))
   {
      REMOVE_BIT(obj->magic_flags, ITEM_PKDISARMED);
      obj->value[5] = 0;
   }
   
   if (container && container->item_type == ITEM_CORPSE_PC && IS_OBJ_STAT(obj, ITEM_NOGIVE) && !sysdata.resetgame)
   {
      pd = container->short_descr;
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      if (IS_NPC(ch) || str_cmp(name, ch->name))
      {
         send_to_char("A nogive item cannot be looted from a corpse.\n\r", ch);
         return;
      }
   }
   
   if (IS_UNIQUE(ch, obj) && container && container->carried_by && container->carried_by == ch)
   {
      ;
   }
   else if (IS_UNIQUE(ch, obj))
   {
      send_to_char("That item is unique and you already have one on you.\n\r", ch);
      return;
   }

   if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE) && !can_take_proto(ch))
   {
      send_to_char("A godly force prevents you from getting close to it.\n\r", ch);
      return;
   }

   if (get_ch_carry_number(ch) + get_obj_number(obj) > can_carry_n(ch))
   {
      act(AT_PLAIN, "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR);
      return;
   }

   if (IS_OBJ_STAT(obj, ITEM_COVERING))
      weight = obj->weight;
   else
      weight = get_obj_weight(obj);

   if (get_ch_carry_weight(ch) + weight > can_carry_w(ch))
   {
      act(AT_PLAIN, "$d: you can't carry that much weight.", ch, NULL, obj->name, TO_CHAR);
      return;
   }

   if (container)
   {
      if (container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING))
      {
         act(AT_ACTION, "You remove $p from $P", ch, obj, container, TO_CHAR);
         act(AT_ACTION, "$n removes $p from $P", ch, obj, container, TO_ROOM);
      }
      else
      {
         act(AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ? "You get $p from beneath $P." : "You get $p from $P", ch, obj, container, TO_CHAR);
         act(AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ? "$n gets $p from beneath $P." : "$n gets $p from $P", ch, obj, container, TO_ROOM);
      }
      if (IS_OBJ_STAT(container, ITEM_CLANCORPSE) && !IS_NPC(ch) && str_cmp(container->name + 7, ch->name))
         container->value[5]++;
      obj_from_obj(obj);
   }
   else
   {
      act(AT_ACTION, "You get $p.", ch, obj, container, TO_CHAR);
      act(AT_ACTION, "$n gets $p.", ch, obj, container, TO_ROOM);
      obj_from_room(obj);
   }
   check_for_trap(ch, obj, -1, NEW_TRAP_GETOBJ);
   if (char_died(ch))
      return;
   if (global_retcode == rOBJ_SCRAPPED)
      return;
   // Check to see if it is a kingdom chest
   if (container && is_kingdom_chest(container))
      save_kingdom_chests(ch);
   /* Clan storeroom checks */
   if (xIS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) && (!container || container->carried_by == NULL))
   {
/*	if (!char_died) save_char_obj(ch); */
      for (clan = first_clan; clan; clan = clan->next)
         if (clan->storeroom == ch->in_room->vnum)
            save_clan_storeroom(ch, clan);
   }

   if (obj->item_type == ITEM_MONEY)
   {

      amt = obj->value[0];

/*
 *  The idea was to make some races more adroit at money handling,
 *  however, this resulted in elves dropping 1M gps and picking 
 *  up 1.1M, repeating, and getting rich.  The only solution would
 *  be to fuzzify the "drop coins" code, but that seems like it'd
 *  lead to more confusion than it warrants.  -h
 *
 *  When you work on this again, make it so that amt is NEVER multiplied
 *  by more than 1.0.  Use less than 1.0 for ogre, orc, troll, etc.
 *  (Ie: a penalty rather than a bonus)
 */
#ifdef GOLD_MULT
      switch (ch->race)
      {
         case (1):
            amt *= 1.1;
            break; /* elf */
         case (2):
            amt *= 0.97;
            break; /* dwarf */
         case (3):
            amt *= 1.02;
            break; /* halfling */
         case (4):
            amt *= 1.08;
            break; /* pixie */
         case (6):
            amt *= 0.92;
            break; /* half-ogre */
         case (7):
            amt *= 0.94;
            break; /* half-orc */
         case (8):
            amt *= 0.90;
            break; /* half-troll */
         case (9):
            amt *= 1.04;
            break; /* half-elf */
         case (10):
            amt *= 1.06;
            break; /* gith */
      }
#endif
   
      ch->gold += amt;
      if (xIS_SET(ch->act, PLR_AUTOSPLIT))
      {
         char buf1[20];
         sprintf(buf1, "%d", amt);
         do_split(ch, buf1);
      }
      extract_obj(obj);
   }
   else
   {
      obj = obj_to_char(obj, ch);
   }

   if (char_died(ch) || obj_extracted(obj))
      return;
   oprog_get_trigger(ch, obj);
   return;
}


void do_get(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   OBJ_DATA *container;
   sh_int number;
   bool found;

   argument = one_argument(argument, arg1);
   if (is_number(arg1))
   {
      number = atoi(arg1);
      if (number < 1)
      {
         send_to_char("That was easy...\n\r", ch);
         return;
      }
      if ((get_ch_carry_number(ch) + number) > can_carry_n(ch))
      {
         send_to_char("You can't carry that many.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg1);
   }
   else
      number = 0;
   argument = one_argument(argument, arg2);
   /* munch optional words */
   if (!str_cmp(arg2, "from") && argument[0] != '\0')
      argument = one_argument(argument, arg2);

   /* Get type. */
   if (arg1[0] == '\0')
   {
      send_to_char("Get what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;
      
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
   {
      send_to_char("You cannot do that.\n\r", ch);
      return;
   }

   if (arg2[0] == '\0')
   {
      if (number <= 1 && str_cmp(arg1, "all") && str_prefix("all.", arg1))
      {
         /* 'get obj' */
         obj = get_obj_list(ch, arg1, ch->in_room->first_content);
         if (!obj)
         {
            act(AT_PLAIN, "I see no $T here.", ch, NULL, arg1, TO_CHAR);
            return;
         }
         separate_obj(obj);
         get_obj(ch, obj, NULL);
         if (char_died(ch))
            return;
         if (IS_SET(sysdata.save_flags, SV_GET))
            save_char_obj(ch);
      }
      else
      {
         sh_int cnt = 0;
         bool fAll;
         char *chk;

         if (xIS_SET(ch->in_room->room_flags, ROOM_DONATION))
         {
            send_to_char("The gods frown upon such a display of greed!\n\r", ch);
            return;
         }
         if (!str_cmp(arg1, "all"))
            fAll = TRUE;
         else
            fAll = FALSE;
         if (number > 1)
            chk = arg1;
         else
            chk = &arg1[4];
         /* 'get all' or 'get all.obj' */
         found = FALSE;
         for (obj = ch->in_room->last_content; obj; obj = obj_next)
         {
            obj_next = obj->prev_content;
            if ((fAll || nifty_is_name(chk, obj->name)) && can_see_obj(ch, obj))
            {

               if (IS_OBJ_STAT(obj, ITEM_ONMAP))
               {
                  if (ch->map != obj->map
                     || ch->coord->x != obj->coord->x
                     || ch->coord->y != obj->coord->y)
                  {
                     found = FALSE;
                     continue;
                  }
               }
               found = TRUE;
               if (number && (cnt + obj->count) > number)
                  split_obj(obj, number - cnt);
               cnt += obj->count;
               get_obj(ch, obj, NULL);
               if (char_died(ch) || get_ch_carry_number(ch) >= can_carry_n(ch) || get_ch_carry_weight(ch) >= can_carry_w(ch) || (number && cnt >= number))
               {
                  if (IS_SET(sysdata.save_flags, SV_GET) && !char_died(ch))
                     save_char_obj(ch);
                  return;
               }
            }
         }

         if (!found)
         {
            if (fAll)
               send_to_char("I see nothing here.\n\r", ch);
            else
               act(AT_PLAIN, "I see no $T here.", ch, NULL, chk, TO_CHAR);
         }
         else if (IS_SET(sysdata.save_flags, SV_GET))
            save_char_obj(ch);
      }
   }
   else
   {
      /* 'get ... container' */
      if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2))
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }

      if ((container = get_obj_here(ch, arg2)) == NULL)
      {
         act(AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR);
         return;
      }

      switch (container->item_type)
      {
         default:
            if (!IS_OBJ_STAT(container, ITEM_COVERING))
            {
               send_to_char("That's not a container.\n\r", ch);
               return;
            }
            if (get_ch_carry_weight(ch) + container->weight > can_carry_w(ch))
            {
               send_to_char("It's too heavy for you to lift.\n\r", ch);
               return;
            }
            break;

         case ITEM_CONTAINER:
         case ITEM_CORPSE_NPC:
         case ITEM_KEYRING:
         case ITEM_QUIVER:
         case ITEM_SHEATH:
            break;

         case ITEM_CORPSE_PC:
            {
               bool fGroup;
               char name[MIL];
               CHAR_DATA *gch;
               char *pd;

               fGroup = FALSE;

               if (IS_NPC(ch))
               {
                  send_to_char("You can't do that.\n\r", ch);
                  return;
               }

               pd = container->short_descr;
               pd = one_argument(pd, name);
               pd = one_argument(pd, name);
               pd = one_argument(pd, name);
               pd = one_argument(pd, name);

               /* Killer/owner loot only if die to pkill blow --Blod */
               if (IS_OBJ_STAT(container, ITEM_CLANCORPSE)
                  && !IS_NPC(ch)
                  && container->action_desc[0] != '\0'
                  && str_cmp(name, ch->name) && str_cmp(container->action_desc, ch->name) && container->timer > 2878)
               {
                  send_to_char("The corpse is too fresh to loot at this moment..\n\r", ch);
                  return;
               }

               if (IS_OBJ_STAT(container, ITEM_CLANCORPSE)
                  && !IS_NPC(ch) && container->action_desc[0] != '\0' && !str_cmp(container->action_desc, ch->name))
               {
                  if (check_room_pk(ch) == 4)
                     fGroup = TRUE;
               }

               if (IS_OBJ_STAT(container, ITEM_CLANCORPSE) && container->timer < 2879)
               {
                  if (check_room_pk(ch) == 4)
                     fGroup = TRUE;
               }

               if (str_cmp(name, ch->name) && !IS_IMMORTAL(ch))
               {
                  for (gch = first_char; gch; gch = gch->next)
                  {
                     if (!IS_NPC(gch) && is_same_group(ch, gch) && !str_cmp(name, gch->name))
                     {
                        fGroup = TRUE;
                        break;
                     }
                  }

                  if (!fGroup)
                  {
                     send_to_char("That's someone else's corpse.\n\r", ch);
                     return;
                  }
               }
            }
      }
      if (container->item_type == ITEM_CONTAINER || container->item_type == ITEM_QUIVER)
      {
         if (!IS_OBJ_STAT(container, ITEM_COVERING) && IS_SET(container->value[1], CONT_CLOSED))
         {
            act(AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR);
            return;
         }
      }

      if (number <= 1 && str_cmp(arg1, "all") && str_prefix("all.", arg1))
      {
         /* 'get obj container' */
         obj = get_obj_list(ch, arg1, container->first_content);
         if (!obj)
         {
            act(AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
               "I see nothing like that beneath the $T." : "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR);
            return;
         }
         separate_obj(obj);
         check_for_trap(ch, container, TRAP_GET, NEW_TRAP_GET);
         if (char_died(ch))
            return;
         if (global_retcode == rOBJ_SCRAPPED)
            return;
         get_obj(ch, obj, container);
         /* Oops no wonder corpses were duping oopsie did I do that
          * --Shaddai
          */
         if (container->item_type == ITEM_CORPSE_PC)
            write_corpses(NULL, container->short_descr + 14, NULL);
         if (IS_SET(sysdata.save_flags, SV_GET))
            save_char_obj(ch);
      }
      else
      {
         int cnt = 0;
         bool fAll;
         char *chk;

         /* 'get all container' or 'get all.obj container' */
         if (IS_OBJ_STAT(container, ITEM_DONATION))
         {
            send_to_char("The gods frown upon such an act of greed!\n\r", ch);
            return;
         }

         if (!str_cmp(arg1, "all"))
            fAll = TRUE;
         else
            fAll = FALSE;
         if (number > 1)
            chk = arg1;
         else
            chk = &arg1[4];
         found = FALSE;
         check_for_trap(ch, container, TRAP_GET, NEW_TRAP_GET);
         if (char_died(ch))
            return;
         if (global_retcode == rOBJ_SCRAPPED)
            return;
         for (obj = container->first_content; obj; obj = obj_next)
         {
            obj_next = obj->next_content;
            if ((fAll || nifty_is_name(chk, obj->name)) && can_see_obj(ch, obj))
            {
               found = TRUE;
               if (number && (cnt + obj->count) > number)
                  split_obj(obj, number - cnt);
               cnt += obj->count;
               get_obj(ch, obj, container);
               if (char_died(ch) || get_ch_carry_number(ch) >= can_carry_n(ch) || get_ch_carry_weight(ch) >= can_carry_w(ch) || (number && cnt >= number))
                  return;
            }
         }

         if (!found)
         {
            if (fAll)
            {
               if (container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING))
                  act(AT_PLAIN, "The $T holds no keys.", ch, NULL, arg2, TO_CHAR);
               else
                  act(AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
                     "I see nothing beneath the $T." : "I see nothing in the $T.", ch, NULL, arg2, TO_CHAR);
            }
            else
            {
               if (container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING))
                  act(AT_PLAIN, "The $T does not hold that key.", ch, NULL, arg2, TO_CHAR);
               else
                  act(AT_PLAIN, IS_OBJ_STAT(container, ITEM_COVERING) ?
                     "I see nothing like that beneath the $T." : "I see nothing like that in the $T.", ch, NULL, arg2, TO_CHAR);
            }
         }
         /* Oops no wonder corpses were duping oopsie did I do that
          * --Shaddai
          */
         if (container && container->item_type == ITEM_CORPSE_PC)
            write_corpses(NULL, container->short_descr + 14, NULL);
         if (found && IS_SET(sysdata.save_flags, SV_GET))
            save_char_obj(ch);
      }
   }
   return;
}



void do_put(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   OBJ_DATA *container;
   OBJ_DATA *obj;
   OBJ_DATA *sheath;
   OBJ_DATA *obj_next;
   CLAN_DATA *clan;
   sh_int count;
   int number;
   bool save_char = FALSE;

   argument = one_argument(argument, arg1);
   if (is_number(arg1))
   {
      number = atoi(arg1);
      if (number < 1)
      {
         send_to_char("That was easy...\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg1);
   }
   else
      number = 0;
   argument = one_argument(argument, arg2);
   /* munch optional words */
   if ((!str_cmp(arg2, "into") || !str_cmp(arg2, "inside")
         || !str_cmp(arg2, "in") || !str_cmp(arg2, "under") || !str_cmp(arg2, "onto") || !str_cmp(arg2, "on")) && argument[0] != '\0')
      argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Put what in what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if (!str_cmp(arg2, "all") || !str_prefix("all.", arg2))
   {
      send_to_char("You can't do that.\n\r", ch);
      return;
   }

   if ((container = get_obj_here(ch, arg2)) == NULL)
   {
      act(AT_PLAIN, "I see no $T here.", ch, NULL, arg2, TO_CHAR);
      return;
   }

   if (!container->carried_by && IS_SET(sysdata.save_flags, SV_PUT))
      save_char = TRUE;

   if (IS_OBJ_STAT(container, ITEM_COVERING))
   {
      if (get_ch_carry_weight(ch) + container->weight > can_carry_w(ch))
      {
         send_to_char("It's too heavy for you to lift.\n\r", ch);
         return;
      }
   }
   else
   {
      if (container->item_type != ITEM_CONTAINER
         && container->item_type != ITEM_KEYRING && container->item_type != ITEM_QUIVER && container->item_type != ITEM_SHEATH)
      {
         send_to_char("That's not a container.\n\r", ch);
         return;
      }
      if (container->item_type == ITEM_CONTAINER || container->item_type == ITEM_QUIVER)
      {
         if (IS_SET(container->value[1], CONT_CLOSED))
         {
            act(AT_PLAIN, "The $d is closed.", ch, NULL, container->name, TO_CHAR);
            return;
         }
      }
   }

   if (number <= 1 && str_cmp(arg1, "all") && str_prefix("all.", arg1))
   {
      /* 'put obj container' */
      if ((obj = get_obj_carry(ch, arg1)) == NULL)
      {
         send_to_char("You do not have that item.\n\r", ch);
         return;
      }

      if (obj == container)
      {
         send_to_char("You can't fold it into itself.\n\r", ch);
         return;
      }

      if (!can_drop_obj(ch, obj))
      {
         send_to_char("You can't let go of it.\n\r", ch);
         return;
      }

/* Check to see if they are trying to cheat by putting the item in a container --Xerves 3/24/99 */


      if ((IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(container, ITEM_NOGIVE))
         || (IS_OBJ_STAT(container, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NOGIVE)))
      {
         send_to_char("You failed again my friend!\n\r", ch);
         return;
      }
      
      if (container && (IS_UNIQUE(ch, container) || IS_UNIQUE(ch, obj)) && obj->item_type == ITEM_CONTAINER)
      {
         send_to_char("You cannot put a unique container in any kind of container.\n\r", ch);
         return;
      }

      if (container->item_type == ITEM_KEYRING && obj->item_type != ITEM_KEY)
      {
         send_to_char("That's not a key.\n\r", ch);
         return;
      }

      if (container->item_type == ITEM_QUIVER && obj->item_type != ITEM_PROJECTILE)
      {
         send_to_char("That's not a projectile.\n\r", ch);
         return;
      }
      if (container->item_type == ITEM_SHEATH && (container->value[1] != obj->value[3]))
      {
         send_to_char("You can only put a weapon made for the sheath in the sheath.\n\r", ch);
         return;
      }
      if (container->item_type == ITEM_SHEATH && ((container->value[2] != obj->pIndexData->vnum) && container->value[2] != 0))
      {
         send_to_char("Your sheath was made for only one weapon and the weapon you have it not it.\n\r", ch);
         return;
      }
      if ((IS_OBJ_STAT(container, ITEM_COVERING)
            && (get_obj_weight(obj) / obj->count) > ((get_obj_weight(container) / container->count) - container->weight)))
      {
         send_to_char("It won't fit under there.\n\r", ch);
         return;
      }

      /* note use of get_real_obj_weight */
      if ((get_real_obj_weight(obj) / obj->count) + (get_real_obj_weight(container) / container->count) > container->value[0])
      {
         send_to_char("It won't fit.\n\r", ch);
         return;
      }
      /* Don't need all of this, but will probably use it later on containers --Xerves */
      if (container->item_type == ITEM_SHEATH)
      {
         count = 0;
         if (container->first_content)
         {
            for (sheath = container->first_content; sheath; sheath = sheath->next_content)
               count++;
         }

         if (count == 1)
         {
            send_to_char("Sheath's only fit one weapon, remove the one in it first.\n\r", ch);
            return;
         }
      }
      separate_obj(obj);
      separate_obj(container);
      obj_from_char(obj);
      obj = obj_to_obj(obj, container);
      count = obj->count;
      obj->count = 1;
      if (container->item_type == ITEM_KEYRING && !IS_OBJ_STAT(container, ITEM_COVERING))
      {
         act(AT_ACTION, "$n slips $p onto $P.", ch, obj, container, TO_ROOM);
         act(AT_ACTION, "You slip $p onto $P.", ch, obj, container, TO_CHAR);
      }
      else
      {
         act(AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ? "$n hides $p beneath $P." : "$n puts $p in $P.", ch, obj, container, TO_ROOM);
         act(AT_ACTION, IS_OBJ_STAT(container, ITEM_COVERING) ? "You hide $p beneath $P." : "You put $p in $P.", ch, obj, container, TO_CHAR);
      }
      obj->count = count;
      check_for_trap(ch, container, TRAP_PUT, NEW_TRAP_PUT);
      if (char_died(ch))
         return;
      if (global_retcode == rOBJ_SCRAPPED)
         return;
      check_for_trap(ch, obj, -1, NEW_TRAP_PUTOBJ);
      if (char_died(ch))
         return;
      if (global_retcode == rOBJ_SCRAPPED)
         return;
      /* Oops no wonder corpses were duping oopsie did I do that
       * --Shaddai
       */
      if (container->item_type == ITEM_CORPSE_PC)
         write_corpses(NULL, container->short_descr + 14, NULL);

      if (save_char)
         save_char_obj(ch);

      // Check to see if it is a kingdom chest
      if (container && is_kingdom_chest(container))
         save_kingdom_chests(ch);
      /* Clan storeroom check */
      if (xIS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) && container->carried_by == NULL)
      {
/*	   if (!char_died && !save_char ) save_char_obj(ch); */
         for (clan = first_clan; clan; clan = clan->next)
            if (clan->storeroom == ch->in_room->vnum)
               save_clan_storeroom(ch, clan);
      }
   }
   else
   {
      bool found = FALSE;
      int cnt = 0;
      bool fAll;
      char *chk;

      if (container->item_type == ITEM_SHEATH)
      {
         send_to_char("You cannot put all your items in a sheath, only one weapon.\n\r", ch);
         return;
      }

      if (!str_cmp(arg1, "all"))
         fAll = TRUE;
      else
         fAll = FALSE;
      if (number > 1)
         chk = arg1;
      else
         chk = &arg1[4];

      separate_obj(container);
      check_for_trap(ch, container, TRAP_PUT, NEW_TRAP_PUT);
      if (char_died(ch))
         return;
      if (global_retcode == rOBJ_SCRAPPED)
         return; 
      /* 'put all container' or 'put all.obj container' */
      for (obj = ch->first_carrying; obj; obj = obj_next)
      {
         obj_next = obj->next_content;

         if ((fAll || nifty_is_name(chk, obj->name))
            && can_see_obj(ch, obj)
            && obj->wear_loc == WEAR_NONE
            && obj != container
            && can_drop_obj(ch, obj)
            && (container->item_type != ITEM_KEYRING || obj->item_type == ITEM_KEY)
            && (container->item_type != ITEM_QUIVER || obj->item_type == ITEM_PROJECTILE)
            && get_obj_weight(obj) + get_obj_weight(container) <= container->value[0])
         {
            found = TRUE;
            if ((IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_OBJ_STAT(container, ITEM_NOGIVE))
            || (IS_OBJ_STAT(container, ITEM_NOGIVE) && !IS_OBJ_STAT(obj, ITEM_NOGIVE)))
               continue;
      
            if (container && (IS_UNIQUE(ch, container) || IS_UNIQUE(ch, obj)) && obj->item_type == ITEM_CONTAINER)
               continue; 
            if (number && (cnt + obj->count) > number)
               split_obj(obj, number - cnt);
            cnt += obj->count;
            obj_from_char(obj);
            if (container->item_type == ITEM_KEYRING)
            {
               act(AT_ACTION, "$n slips $p onto $P.", ch, obj, container, TO_ROOM);
               act(AT_ACTION, "You slip $p onto $P.", ch, obj, container, TO_CHAR);
            }
            else
            {
               act(AT_ACTION, "$n puts $p in $P.", ch, obj, container, TO_ROOM);
               act(AT_ACTION, "You put $p in $P.", ch, obj, container, TO_CHAR);
            }
            obj = obj_to_obj(obj, container);
            check_for_trap(ch, obj, -1, NEW_TRAP_PUTOBJ);
            if (char_died(ch))
               return;
            if (global_retcode == rOBJ_SCRAPPED)
               return;
            if (number && cnt >= number)
               break;
         }
      }
      check_for_trap(ch, container, TRAP_PUT, NEW_TRAP_PUT);
      if (char_died(ch))
         return;
      if (global_retcode == rOBJ_SCRAPPED)
         return;
      /*
       * Don't bother to save anything if nothing was dropped   -Thoric
       */
      if (!found)
      {
         if (fAll)
            act(AT_PLAIN, "You are not carrying anything.", ch, NULL, NULL, TO_CHAR);
         else
            act(AT_PLAIN, "You are not carrying any $T.", ch, NULL, chk, TO_CHAR);
         return;
      }
      else
      {
         send_to_char("Done.\n\r", ch);
      }
      
      if (save_char)
         save_char_obj(ch);
      /* Oops no wonder corpses were duping oopsie did I do that
       * --Shaddai
       */
      if (container->item_type == ITEM_CORPSE_PC)
         write_corpses(NULL, container->short_descr + 14, NULL);
      // Check to see if it is a kingdom chest
      if (container && is_kingdom_chest(container))
         save_kingdom_chests(ch);
      /* Clan storeroom check */
      if (xIS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM) && container->carried_by == NULL)
      {
/*	  if (!char_died && !save_char) save_char_obj(ch); */
         for (clan = first_clan; clan; clan = clan->next)
            if (clan->storeroom == ch->in_room->vnum)
               save_clan_storeroom(ch, clan);
      }
   }

   return;
}


void do_drop(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   bool found;
   CLAN_DATA *clan;
   int number;

   argument = one_argument(argument, arg);
   if (is_number(arg))
   {
      number = atoi(arg);
      if (number < 1)
      {
         send_to_char("That was easy...\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg);
   }
   else
      number = 0;

   if (arg[0] == '\0')
   {
      send_to_char("Drop what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if (xIS_SET(ch->act, PLR_LITTERBUG))
   {
      set_char_color(AT_YELLOW, ch);
      send_to_char("A godly force prevents you from dropping anything...\n\r", ch);
      return;
   }
   
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
   {
      send_to_char("You cannot do that.\n\r", ch);
      return;
   }

   if ((wIS_SET(ch, ROOM_NODROP) || xIS_SET(ch->in_room->room_flags, ROOM_NODROP)) && ch != supermob)
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("A magical force stops you!\n\r", ch);
      set_char_color(AT_TELL, ch);
      send_to_char("Someone tells you, 'No littering here!'\n\r", ch);
      return;
   }

   if (number > 0)
   {
      /* 'drop NNNN coins' */

      if (!str_cmp(arg, "coins") || !str_cmp(arg, "coin"))
      {
         if (ch->gold < number)
         {
            send_to_char("You haven't got that many coins.\n\r", ch);
            return;
         }

         ch->gold -= number;

         for (obj = ch->in_room->first_content; obj; obj = obj_next)
         {
            obj_next = obj->next_content;

            if (ch->coord->x != obj->coord->x || ch->coord->y != obj->coord->y || ch->map != obj->map)
               continue;

            switch (obj->pIndexData->vnum)
            {
               case OBJ_VNUM_MONEY_ONE:
                  number += 1;
                  extract_obj(obj);
                  break;

               case OBJ_VNUM_MONEY_SOME:
                  number += obj->value[0];
                  extract_obj(obj);
                  break;
            }
         }

         act(AT_ACTION, "$n drops some gold.", ch, NULL, NULL, TO_ROOM);
         obj_to_room(create_money(number), ch->in_room, ch);
         send_to_char("OK.\n\r", ch);
         if (IS_SET(sysdata.save_flags, SV_DROP))
            save_char_obj(ch);
         return;
      }
   }

   if (number <= 1 && str_cmp(arg, "all") && str_prefix("all.", arg))
   {
      /* 'drop obj' */
      if ((obj = get_obj_carry(ch, arg)) == NULL)
      {
         send_to_char("You do not have that item.\n\r", ch);
         return;
      }

      if (!can_drop_obj(ch, obj))
      {
         send_to_char("You can't let go of it.\n\r", ch);
         return;
      }

      separate_obj(obj);
      act(AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM);
      act(AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR);
         
      obj_from_char(obj);
      obj = obj_to_room(obj, ch->in_room, ch);
      check_for_trap(ch, obj, -1, NEW_TRAP_DROPOBJ);
      if (char_died(ch))
         return;
      if (global_retcode == rOBJ_SCRAPPED)
         return; 
      oprog_drop_trigger(ch, obj); /* mudprogs */

      if (char_died(ch) || obj_extracted(obj))
         return;

      /* Clan storeroom saving */
      if (xIS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM))
      {
/*	   if (!char_died) save_char_obj(ch); */
         for (clan = first_clan; clan; clan = clan->next)
            if (clan->storeroom == ch->in_room->vnum)
               save_clan_storeroom(ch, clan);
      }
   }
   else
   {
      int cnt = 0;
      char *chk;
      bool fAll;

      if (!str_cmp(arg, "all"))
         fAll = TRUE;
      else
         fAll = FALSE;
      if (number > 1)
         chk = arg;
      else
         chk = &arg[4];
      /* 'drop all' or 'drop all.obj' */
      if (wIS_SET(ch, ROOM_NODROPALL) || xIS_SET(ch->in_room->room_flags, ROOM_NODROPALL) 
      ||  xIS_SET(ch->in_room->room_flags, ROOM_CLANSTOREROOM))
      {
         send_to_char("You can't seem to do that here...\n\r", ch);
         return;
      }
      found = FALSE;
      for (obj = ch->first_carrying; obj; obj = obj_next)
      {
         obj_next = obj->next_content;

         if ((fAll || nifty_is_name(chk, obj->name)) && can_see_obj(ch, obj) && obj->wear_loc == WEAR_NONE && can_drop_obj(ch, obj))
         {
            found = TRUE;
            if (HAS_PROG(obj->pIndexData, DROP_PROG) && obj->count > 1)
            {
               ++cnt;
               separate_obj(obj);
               obj_from_char(obj);
               if (!obj_next)
                  obj_next = ch->first_carrying;
            }
            else
            {
               if (number && (cnt + obj->count) > number)
                  split_obj(obj, number - cnt);
               cnt += obj->count;
               obj_from_char(obj);
            }
            act(AT_ACTION, "$n drops $p.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You drop $p.", ch, obj, NULL, TO_CHAR);
            obj = obj_to_room(obj, ch->in_room, ch);
            check_for_trap(ch, obj, -1, NEW_TRAP_DROPOBJ);
            if (char_died(ch))
               return;
            if (global_retcode == rOBJ_SCRAPPED)
               return;
            oprog_drop_trigger(ch, obj); /* mudprogs */
            if (char_died(ch))
               return;
            if (number && cnt >= number)
               break;
         }
      }

      if (!found)
      {
         if (fAll)
            act(AT_PLAIN, "You are not carrying anything.", ch, NULL, NULL, TO_CHAR);
         else
            act(AT_PLAIN, "You are not carrying any $T.", ch, NULL, chk, TO_CHAR);
      }
   }
   if (IS_SET(sysdata.save_flags, SV_DROP))
      save_char_obj(ch); /* duping protector */
   return;
}



void do_give(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char buf[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   SLAB_DATA *slab;
   int cnt;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (!str_cmp(arg2, "to") && argument[0] != '\0')
      argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Give what to whom?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if (is_number(arg1))
   {
      /* 'give NNNN coins victim' */
      int amount;

      amount = atoi(arg1);
      if (amount <= 0 || (str_cmp(arg2, "coins") && str_cmp(arg2, "coin")))
      {
         send_to_char("Sorry, you can't do that.\n\r", ch);
         return;
      }

      argument = one_argument(argument, arg2);
      if (!str_cmp(arg2, "to") && argument[0] != '\0')
         argument = one_argument(argument, arg2);
      if (arg2[0] == '\0')
      {
         send_to_char("Give what to whom?\n\r", ch);
         return;
      }

      if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }

      if (ch->gold < amount)
      {
         send_to_char("Very generous of you, but you haven't got that much gold.\n\r", ch);
         return;
      }

      ch->gold -= amount;
      victim->gold += amount;
      strcpy(buf, "$n gives you ");
      strcat(buf, arg1);
      strcat(buf, (amount > 1) ? " coins." : " coin.");

      act(AT_ACTION, buf, ch, NULL, victim, TO_VICT);
      act(AT_ACTION, "$n gives $N some gold.", ch, NULL, victim, TO_NOTVICT);
      act(AT_ACTION, "You give $N some gold.", ch, NULL, victim, TO_CHAR);
      send_to_char("OK.\n\r", ch);
      mprog_bribe_trigger(victim, ch, amount);
      if (IS_SET(sysdata.save_flags, SV_GIVE) && !char_died(ch))
         save_char_obj(ch);
      if (IS_SET(sysdata.save_flags, SV_RECEIVE) && !char_died(victim))
         save_char_obj(victim);
      return;
   }

   if ((obj = get_obj_carry(ch, arg1)) == NULL)
   {
      send_to_char("You do not have that item.\n\r", ch);
      return;
   }

   if (obj->wear_loc != WEAR_NONE)
   {
      send_to_char("You must remove it first.\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (!can_drop_obj(ch, obj))
   {
      send_to_char("You can't let go of it.\n\r", ch);
      return;
   }

   /* Prevents giving of special items (rare items) -- Xerves 3/24/99 */
   if (IS_OBJ_STAT(obj, ITEM_NOGIVE) && !IS_IMMORTAL(ch) && !IS_IMMORTAL(victim))
   {
      send_to_char("You cannot give this item to another, it is special.\n\r", ch);
      return;
   }
   if (IS_UNIQUE(victim, obj))
   {
      send_to_char("That item is unique and your target already has one.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      if (xIS_SET(victim->act, ACT_CASTEMOB))
      {
         send_to_char("Sorry, I do not accept donations.\n\r", ch);
         return;
      }
      if (xIS_SET(victim->act, ACT_MILITARY))
      {
         send_to_char("You cannot give objects to a military mobile.\n\r", ch);
         return;
      }
   }
   if (get_ch_carry_number(victim) + (get_obj_number(obj) / obj->count) > can_carry_n(victim))
   {
      act(AT_PLAIN, "$N has $S hands full.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (get_ch_carry_weight(victim) + (get_obj_weight(obj) / obj->count) > can_carry_w(victim))
   {
      act(AT_PLAIN, "$N can't carry that much weight.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (!can_see_obj(victim, obj))
   {
      act(AT_PLAIN, "$N can't see it.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE) && !can_take_proto(victim))
   {
      act(AT_PLAIN, "You cannot give that to $N!", ch, NULL, victim, TO_CHAR);
      return;
   }
   cnt = 1;
   if (isdigit(argument[0]) && IS_IMMORTAL(ch) && xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
   {
      for (slab = first_slab; slab ; slab = slab->next) 
      {
         if (atoi(argument) == cnt)
            break;
         cnt++;
      }
      if (!slab)
      {
         send_to_char("There is no such ore (type forge ores)\n\r", ch);
         return;
      }
      obj->value[6] = slab->vnum;
      ch_printf(ch, "That item will now load as %s.\n\r", slab->adj);
   }

   separate_obj(obj);
   act(AT_ACTION, "$n gives $p to $N.", ch, obj, victim, TO_NOTVICT);
   act(AT_ACTION, "$n gives you $p.", ch, obj, victim, TO_VICT);
   act(AT_ACTION, "You give $p to $N.", ch, obj, victim, TO_CHAR);
   obj_from_char(obj);
   obj = obj_to_char(obj, victim);
   mprog_give_trigger(victim, ch, obj);
   if (IS_SET(sysdata.save_flags, SV_GIVE) && !char_died(ch))
      save_char_obj(ch);
   if (IS_SET(sysdata.save_flags, SV_RECEIVE) && !char_died(victim))
      save_char_obj(victim);
   check_for_trap(ch, obj, -1, NEW_TRAP_GIVEOBJ);
   if (char_died(ch))
      return;
   if (global_retcode == rOBJ_SCRAPPED)
      return;
   return;
}

/*
 * Damage an object.						-Thoric
 * Affect player's AC if necessary.
 * Make object into scraps if necessary.
 * Send message about damaged object.
 */
obj_ret damage_obj(OBJ_DATA * obj, CHAR_DATA * attacker, int proj, int dam)
{
   CHAR_DATA *ch;
   OBJ_DATA *wpn;
   obj_ret objcode;
   int durability;

   ch = obj->carried_by;
   objcode = rNONE;

   // Promote that carnage!!!  -- Xerves
   if (in_arena(ch))
   {
      act(AT_OBJECT, "(Xerves magically prevents $p from being damaged)", ch, obj, NULL, TO_CHAR);
      return objcode;
   }

   if (IS_OBJ_STAT(obj, ITEM_NOBREAK))
   {

      if (obj->item_type != ITEM_LIGHT)
         oprog_damage_trigger(ch, obj);
      else if (!in_arena(ch))
         oprog_damage_trigger(ch, obj);

      return objcode;
   }

   separate_obj(obj);

   if (obj->item_type != ITEM_LIGHT)
      oprog_damage_trigger(ch, obj);
   else if (!in_arena(ch))
      oprog_damage_trigger(ch, obj);

   if (obj_extracted(obj))
      return global_objcode;

   switch (obj->item_type)
   {
      default:
         make_scraps(obj, attacker);
         objcode = rOBJ_SCRAPPED;
         break;
      case ITEM_CONTAINER:
      case ITEM_KEYRING:
      case ITEM_QUIVER:
      case ITEM_SHEATH:
         if (--obj->value[3] <= 0)
         {
            if (!in_arena(ch))
            {
               make_scraps(obj, attacker);
               objcode = rOBJ_SCRAPPED;
            }
            else
               obj->value[3] = 1;
         }
         break;
      case ITEM_LIGHT:
         if (--obj->value[0] <= 0)
         {
            if (!in_arena(ch))
            {
               make_scraps(obj, attacker);
               objcode = rOBJ_SCRAPPED;
            }
            else
               obj->value[0] = 1;
         }
         break;
      case ITEM_WEAPON:
         if (attacker && !in_arena(ch))
         {  
            durability = UMAX(1, obj->value[10]);
            dam = dam * 2 / durability;
            if (proj == 1) //Less wear on the actual bows
               dam /= 3;
            if (dam < 1)
               dam = 1;
            obj->value[0] -= dam;
            
            if (obj->value[0] <= 0)
            {
               make_scraps(obj, attacker);
               objcode = rOBJ_SCRAPPED;
            }
            else if (obj->value[0] < 200)
            {
               act(AT_RED, "$p is almost broken, you need to fix it immediately!", ch, obj, NULL, TO_CHAR);
            }
            else if (obj->value[0] < 400)
            {
               act(AT_WHITE, "$p is damaged, you need to fix it immediately!", ch, obj, NULL, TO_CHAR);
            }
            break;
         }
      case ITEM_ARMOR:
         durability = UMAX(1, obj->value[4]);
         if (attacker && !in_arena(ch))
         {
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
            {
               dam = number_range(dam*5/10, dam*7/10);
               if (proj == 1)
                  dam /= 3;
               if (dam < 1)
                  dam = 1;
               obj->value[0] -= dam;
            
               if (obj->value[0] <= 0)
               {
                  make_scraps(obj, attacker);
                  objcode = rOBJ_SCRAPPED;
               }
               break;
            }
            if (proj == 0)
               wpn = get_eq_char(attacker, WEAR_WIELD);
            else
               wpn = get_eq_char(attacker, WEAR_MISSILE_WIELD);
               
            if (proj == 1) //Stab
            {
               dam = dam * 6 / durability/ 10;
               obj->value[3] -= URANGE(1, dam, 40);
            }
            else
            {
               if (attacker->grip == GRIP_BASH)
               {
                  dam = dam * 6 / durability;
                  obj->value[3] -= URANGE(2, dam, 60);
               }
               if (attacker->grip == GRIP_STAB)
               {
                  dam = dam * 2 / durability;
                  obj->value[3] -= URANGE(1, dam, 30);
               }
               if (attacker->grip == GRIP_SLASH)
               {
                  dam = dam * 4 / durability;
                  obj->value[3] -= URANGE(1, dam, 40);
               }
            }
         }
         else
         {
            dam = dam / (1+(durability/5));
            obj->value[3] -= UMAX(1, dam);   
         }
         if (obj->value[3] <= 0)
         {
            make_scraps(obj, attacker);
            objcode = rOBJ_SCRAPPED;
         }
         else if (obj->value[3] < 200 && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
         {
            act(AT_RED, "$p is almost broken, you need to fix it immediately!", ch, obj, NULL, TO_CHAR);
         }
         else if (obj->value[3] < 400 && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
         {
            act(AT_WHITE, "$p is damaged, you need to fix it immediately!", ch, obj, NULL, TO_CHAR);
         }
         else if (obj->value[1] && obj->value[0] * 1000 / obj->value[1] < 200 && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
         {
            act(AT_RED, "$p is almost broken, you need to fix it immediately!", ch, obj, NULL, TO_CHAR);
         }
         else if (obj->value[1] && obj->value[0] * 1000 / obj->value[1] < 400 && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
         {
            act(AT_WHITE, "$p is damaged, you need to fix it immediately!", ch, obj, NULL, TO_CHAR);
         }
         break;
   }
   return objcode;
}


/*
 * Remove an object.
 * Added support to check only and not remove, it is fReplace 2, 0 and 1 are FALSE/TRUE
 */
bool remove_obj(CHAR_DATA * ch, int iWear, sh_int fReplace)
{
   OBJ_DATA *obj, *tmpobj;

   obj = get_eq_char(ch, iWear);
   if (!obj)
   {
      return TRUE;
   }

   if (!fReplace && get_ch_carry_number(ch) + get_obj_number(obj) > can_carry_n(ch))
   {
      act(AT_PLAIN, "$d: you can't carry that many items.", ch, NULL, obj->name, TO_CHAR);
      return FALSE;
   }

   if (!fReplace)
      return FALSE;
   
   if (IS_OBJ_STAT(obj, ITEM_NOREMOVE))
   {
      act(AT_PLAIN, "You can't remove $p.", ch, obj, NULL, TO_CHAR);
      return FALSE;
   }
   if (ch->fighting)
   {
      if (IS_SET(obj->wear_flags, ITEM_LODGE_RIB) || IS_SET(obj->wear_flags, ITEM_LODGE_ARM) || IS_SET(obj->wear_flags, ITEM_LODGE_LEG))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 1;
      }
      if (IS_SET(obj->wear_flags, ITEM_WIELD) || IS_SET(obj->wear_flags, ITEM_DUAL_WIELD) || IS_SET(obj->wear_flags, ITEM_MISSILE_WIELD)
      ||  IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 2;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_FINGER) || IS_SET(obj->wear_flags, ITEM_WEAR_ABOUT_NECK) || IS_SET(obj->wear_flags, ITEM_WEAR_BACK)
      ||  IS_SET(obj->wear_flags, ITEM_WEAR_WAIST))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 2;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_HEAD) || IS_SET(obj->wear_flags, ITEM_WEAR_NECK) || IS_SET(obj->wear_flags, ITEM_WEAR_ARMS))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 3;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_LEGS))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 4;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_BODY))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 6;
      }
   }  
   if (fReplace == 2)
      return TRUE;
   else
   {
      if (obj == get_eq_char(ch, WEAR_WIELD) && (tmpobj = get_eq_char(ch, WEAR_DUAL_WIELD)) != NULL)
         tmpobj->wear_loc = WEAR_WIELD;
      unequip_char(ch, obj);

      act(AT_ACTION, "$n stops using $p.", ch, obj, NULL, TO_ROOM);
      act(AT_ACTION, "You stop using $p.", ch, obj, NULL, TO_CHAR);
      oprog_remove_trigger(ch, obj);
      return TRUE;
   }
}

/*
 * See if char could be capable of dual-wielding		-Thoric
 */
bool could_dual(CHAR_DATA * ch)
{
   if (IS_NPC(ch) || (ch->pcdata->learned[gsn_dual_wield] && ch->pcdata->ranking[gsn_dual_wield]))
      return TRUE;

   return FALSE;
}

/*
 * See if char can dual wield at this time			-Thoric
 */
bool can_dual(CHAR_DATA * ch)
{
   if (!could_dual(ch))
      return FALSE;

   if (get_eq_char(ch, WEAR_DUAL_WIELD))
   {
      send_to_char("You are already wielding two weapons!\n\r", ch);
      return FALSE;
   }
   if (get_eq_char(ch, WEAR_SHIELD))
   {
      send_to_char("You cannot dual wield while holding a shield!\n\r", ch);
      return FALSE;
   }
   if (ch->con_rarm == -1 || ch->con_larm == -1)
   {
      send_to_char("You cannot dual wield without the use of an arm\n\r", ch);
      return FALSE;
   }
   return TRUE;
}


/*
 * Check to see if there is room to wear another object on this location
 * (Layered clothing support)
 */
bool can_layer(CHAR_DATA * ch, OBJ_DATA * obj, sh_int wear_loc)
{
   OBJ_DATA *otmp;
   sh_int bitlayers = 0;
   sh_int objlayers = obj->pIndexData->layers;

   for (otmp = ch->first_carrying; otmp; otmp = otmp->next_content)
   {
      if (otmp->wear_loc == wear_loc)
      {
         if (!otmp->pIndexData->layers)
            return FALSE;
         else
            bitlayers |= otmp->pIndexData->layers;
      }
   }

   if ((bitlayers && !objlayers) || bitlayers > objlayers)
      return FALSE;
   if (!bitlayers || ((bitlayers & ~objlayers) == bitlayers))
      return TRUE;

   return FALSE;
}

/*
 * Wear one object.
 * Optional replacement of existing objects.
 * Big repetitive code, ick.
 *
 * Restructured a bit to allow for specifying body location	-Thoric
 * & Added support for layering on certain body locations
 */
void wear_obj(CHAR_DATA * ch, OBJ_DATA * obj, bool fReplace, sh_int wear_bit)
{
   char buf[MSL];
   char *wbuf;
   char name[MSL];
   OBJ_DATA *tmpobj = NULL;
   sh_int bit, tmp;
   int race = -1;

   separate_obj(obj);
   if (get_trust(ch) < obj->level && !IS_OBJ_STAT(obj, ITEM_LODGED))
   {
      sprintf(buf, "You must be level %d to use this object.\n\r", obj->level);
      send_to_char(buf, ch);
      act(AT_ACTION, "$n tries to use $p, but is too inexperienced.", ch, obj, NULL, TO_ROOM);
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
   
   wbuf = obj->name;
   wbuf = one_argument(wbuf, name);
   wbuf = one_argument(wbuf, name);
       
   name[0] = UPPER(name[0]);
   if (!str_cmp(name, "Fairy"))
      race = 5;
   else if (!str_cmp(name, "Hobbit"))
      race = 4;
   else if (!str_cmp(name, "Ogre"))
      race = 3;
   else if (!str_cmp(name, "Dwarven"))
      race = 2;
   else if (!str_cmp(name, "Elven"))
      race = 1;
   else if (!str_cmp(name, "Human"))
      race = 0;
   else
   {
      if (xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
      {
         bug("wear_obj: Invalid Race Name %s, on player %s", name, ch->name);
         send_to_char("Error: Invalid Race Name, tell an immortal.\n\r", ch);
         return;
      }
   }
   
   if (xIS_SET(obj->extra_flags, ITEM_FORGEABLE) && ch->race != race && obj->item_type == ITEM_ARMOR)
   {
      ch_printf(ch, "That item is %s in nature and you are not that race.\n\r", name);
      return;
   }
   if (race > -1 && ch->race != race && obj->item_type == ITEM_ARMOR)
   {
      ch_printf(ch, "That item is %s in nature and you are not that race.\n\r", name);
      return;
   }
   if (obj->item_type == ITEM_ARMOR && obj->value[5] >= ASIZE_HEAVY && ch->race == RACE_HOBBIT)
   {
      ch_printf(ch, "Hobbits cannot wear heavy armor.\n\r");
      return;
   }  
   
   if (obj->sworthrestrict > player_stat_worth(ch))
   {
      send_to_char("You are not powerful enough to use this.\n\r", ch);
      return;
   }
   
   if (ch->fighting)
   {
      if (IS_SET(obj->wear_flags, ITEM_LODGE_RIB) || IS_SET(obj->wear_flags, ITEM_LODGE_ARM) || IS_SET(obj->wear_flags, ITEM_LODGE_LEG))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 1;
      }
      if (IS_SET(obj->wear_flags, ITEM_WIELD) || IS_SET(obj->wear_flags, ITEM_DUAL_WIELD) || IS_SET(obj->wear_flags, ITEM_MISSILE_WIELD)
      ||  IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 2;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_FINGER) || IS_SET(obj->wear_flags, ITEM_WEAR_ABOUT_NECK) || IS_SET(obj->wear_flags, ITEM_WEAR_BACK)
      ||  IS_SET(obj->wear_flags, ITEM_WEAR_WAIST))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 2;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_HEAD) || IS_SET(obj->wear_flags, ITEM_WEAR_NECK) || IS_SET(obj->wear_flags, ITEM_WEAR_ARMS))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 3;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_LEGS))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 4;
      }
      if (IS_SET(obj->wear_flags, ITEM_WEAR_BODY))
      {
         if (!IS_NPC(ch))
            ch->fight_timer = 6;
      }
   }  

   if (wear_bit > -1)
   {
      bit = wear_bit;
      if (!CAN_WEAR(obj, 1 << bit))
      {
         if (fReplace)
         {
            switch (1 << bit)
            {
               case ITEM_WIELD:
               case ITEM_MISSILE_WIELD:
                  send_to_char("You cannot wield that.\n\r", ch);
                  break;
               case ITEM_WEAR_NOCKED:
                  send_to_char("You cannot nock that.\n\r", ch);
                  break;
               default:
                  sprintf(buf, "You cannot wear that on your %s.\n\r", w_flags[bit]);
                  send_to_char(buf, ch);
            }
         }
         return;
      }
   }
   else
   {
      for (bit = -1, tmp = 1; tmp < 31; tmp++)
      {
         if (CAN_WEAR(obj, 1 << tmp))
         {
            bit = tmp;
            break;
         }
      }
   }

   /* currently cannot have a light in non-light position */
   if (obj->item_type == ITEM_LIGHT)
   {
      if (!remove_obj(ch, WEAR_LIGHT, fReplace))
         return;
      if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
      {
         act(AT_ACTION, "$n holds $p as a light.", ch, obj, NULL, TO_ROOM);
         act(AT_ACTION, "You hold $p as your light.", ch, obj, NULL, TO_CHAR);
      }
      equip_char(ch, obj, WEAR_LIGHT);
      oprog_wear_trigger(ch, obj);
      return;
   }

   if (bit == -1)
   {
      if (fReplace)
         send_to_char("You can't wear, wield, or hold that.\n\r", ch);
      return;
   }

   switch (1 << bit)
   {
      default:
         bug("wear_obj: uknown/unused item_wear bit %d", bit);
         if (fReplace)
            send_to_char("You can't wear, wield, or hold that.\n\r", ch);
         return;

      case ITEM_WEAR_NOCKED:
         if (!remove_obj(ch, WEAR_NOCKED, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n nocks $p upon $s weapon.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You nock $p upon your weapon.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_NOCKED);
         oprog_wear_trigger(ch, obj);
         return;
   
      case ITEM_WEAR_FINGER:
         if (get_eq_char(ch, WEAR_FINGER_L) != NULL
            && get_eq_char(ch, WEAR_FINGER_R) != NULL && !remove_obj(ch, WEAR_FINGER_L, fReplace) && !remove_obj(ch, WEAR_FINGER_R, fReplace))
            return;

         if (!get_eq_char(ch, WEAR_FINGER_L))
         {
            if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
            {
               act(AT_ACTION, "$n slides $p on $s left ring finger.", ch, obj, NULL, TO_ROOM);
               act(AT_ACTION, "You slide $p on your left ring finger.", ch, obj, NULL, TO_CHAR);
            }
            equip_char(ch, obj, WEAR_FINGER_L);
            oprog_wear_trigger(ch, obj);
            return;
         }

         if (!get_eq_char(ch, WEAR_FINGER_R))
         {
            if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
            {
               act(AT_ACTION, "$n slides $p on $s right ring finger.", ch, obj, NULL, TO_ROOM);
               act(AT_ACTION, "You slide $p on your right ring finger.", ch, obj, NULL, TO_CHAR);
            }
            equip_char(ch, obj, WEAR_FINGER_R);
            oprog_wear_trigger(ch, obj);
            return;
         }

         bug("Wear_obj: no free finger.", 0);
         send_to_char("You already wear two finger items.\n\r", ch);
         return;
  
      case ITEM_WEAR_ABOUT_NECK:
         if (!remove_obj(ch, WEAR_ABOUT_NECK, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n clamps $p around $s neck.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You clamp $p around your neck.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_ABOUT_NECK);
         oprog_wear_trigger(ch, obj);
         return;

      case ITEM_WEAR_NECK:
         if (!remove_obj(ch, WEAR_NECK, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n fastens $p around $s neck.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You fasten $p around your neck.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_NECK);
         oprog_wear_trigger(ch, obj);
         return;
         
      case ITEM_WEAR_BACK:
      
         if (!can_layer(ch, obj, WEAR_BACK))
         {
            send_to_char("It won't fit overtop of what you're already wearing.\n\r", ch);
            return;
         }
         
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n slings $p upon $s back.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You sling $p upon your back.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_BACK);
         oprog_wear_trigger(ch, obj);
         return;

      case ITEM_WEAR_ARMS:
         if (get_eq_char(ch, WEAR_ARM_L) != NULL
            && get_eq_char(ch, WEAR_ARM_R) != NULL && !remove_obj(ch, WEAR_ARM_L, fReplace) && !remove_obj(ch, WEAR_ARM_R, fReplace))
            return;

         if (!get_eq_char(ch, WEAR_ARM_L) && ch->con_larm != -1)
         {
            if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
            {
               act(AT_ACTION, "$n slides $p around $s left arm.", ch, obj, NULL, TO_ROOM);
               act(AT_ACTION, "You slide $p around your left arm.", ch, obj, NULL, TO_CHAR);
            }
            equip_char(ch, obj, WEAR_ARM_L);
            oprog_wear_trigger(ch, obj);
            return;
         }

         if (!get_eq_char(ch, WEAR_ARM_R) && ch->con_rarm != -1)
         {
            if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
            {
               act(AT_ACTION, "$n slides $p around $s right arm.", ch, obj, NULL, TO_ROOM);
               act(AT_ACTION, "You slide $p around your right arm.", ch, obj, NULL, TO_CHAR);
            }
            equip_char(ch, obj, WEAR_ARM_R);
            oprog_wear_trigger(ch, obj);
            return;
         }
         
         if ((ch->con_rarm == -1 || ch->con_larm == -1) && (!get_eq_char(ch, WEAR_ARM_R) || !get_eq_char(ch, WEAR_ARM_L)))
         {
            send_to_char("You cannot wear that around your arm because you arm is broken off.\n\r", ch);
            return;
         } 

         bug("Wear_obj: no free arm.", 0);
         send_to_char("You already wear two arm items.\n\r", ch);
         return;

      case ITEM_WEAR_LEGS:
         if (get_eq_char(ch, WEAR_LEG_L) != NULL
            && get_eq_char(ch, WEAR_LEG_R) != NULL && !remove_obj(ch, WEAR_LEG_L, fReplace) && !remove_obj(ch, WEAR_LEG_R, fReplace))
            return;

         if (!get_eq_char(ch, WEAR_LEG_L) && ch->con_lleg != -1)
         {
            if (ch->con_lleg == -1)
            {
               send_to_char("You cannot wear that around your leg because you leg is broken off.\n\r", ch);
               return;
            }
            if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
            {
               act(AT_ACTION, "$n slides $p around $s left leg.", ch, obj, NULL, TO_ROOM);
               act(AT_ACTION, "You slide $p around your left leg.", ch, obj, NULL, TO_CHAR);
            }
            equip_char(ch, obj, WEAR_LEG_L);
            oprog_wear_trigger(ch, obj);
            return;
         }

         if (!get_eq_char(ch, WEAR_LEG_R) && ch->con_rleg != -1)
         {
            if (ch->con_rleg == -1)
            {
               send_to_char("You cannot wear that around your leg because you leg is broken off.\n\r", ch);
               return;
            }
            if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
            {
               act(AT_ACTION, "$n slides $p around $s right leg.", ch, obj, NULL, TO_ROOM);
               act(AT_ACTION, "You slide $p around your right leg.", ch, obj, NULL, TO_CHAR);
            }
            equip_char(ch, obj, WEAR_LEG_R);
            oprog_wear_trigger(ch, obj);
            return;
         }
         
         if ((ch->con_rleg == -1 || ch->con_lleg == -1) && (!get_eq_char(ch, WEAR_LEG_R) || !get_eq_char(ch, WEAR_LEG_L)))
         {
            send_to_char("You cannot wear that around your arm because you arm is broken off.\n\r", ch);
            return;
         } 

         bug("Wear_obj: no free leg.", 0);
         send_to_char("You already wear two leg items.\n\r", ch);
         return;


      case ITEM_WEAR_BODY:
         if (!remove_obj(ch, WEAR_BODY, fReplace))
            return;
         //I hate layering, bye -- Xerves
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n fits $p on $s body.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You fit $p on your body.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_BODY);
         oprog_wear_trigger(ch, obj);
         return;

      case ITEM_WEAR_HEAD:
         if (!remove_obj(ch, WEAR_HEAD, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n dons $p upon $s head.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You don $p upon your head.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_HEAD);
         oprog_wear_trigger(ch, obj);
         return;

      case ITEM_LODGE_RIB:
         if (!remove_obj(ch, WEAR_LODGE_RIB, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$p lodges itself in $n's ribs.", ch, obj, NULL, TO_ROOM);
            sprintf(buf, "$p &G&W[&RLODGES&G&W]%s itself in your ribs.", char_color_str(AT_ACTION, ch));
            act(AT_ACTION, buf, ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_LODGE_RIB);
         oprog_wear_trigger(ch, obj);
         return;


      case ITEM_LODGE_ARM:
         if (!remove_obj(ch, WEAR_LODGE_ARM, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$p lodges itself in $n's ribs.", ch, obj, NULL, TO_ROOM);
            sprintf(buf, "$p &G&W[&RLODGES&G&W]%s itself in your arm.", char_color_str(AT_ACTION, ch));
            act(AT_ACTION, buf, ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_LODGE_ARM);
         oprog_wear_trigger(ch, obj);
         return;

      case ITEM_LODGE_LEG:
         if (!remove_obj(ch, WEAR_LODGE_LEG, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$p lodges itself in $n's ribs.", ch, obj, NULL, TO_ROOM);
            sprintf(buf, "$p &G&W[&RLODGES&G&W]%s itself in your leg.", char_color_str(AT_ACTION, ch));
            act(AT_ACTION, buf, ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_LODGE_LEG);
         oprog_wear_trigger(ch, obj);
         return;

         //Only layerable slot for now because of sheaths    
      case ITEM_WEAR_WAIST:
/*
	    if ( !remove_obj( ch, WEAR_WAIST, fReplace ) )
	      return;
*/
         if (!can_layer(ch, obj, WEAR_WAIST))
         {
            send_to_char("It won't fit overtop of what you're already wearing.\n\r", ch);
            return;
         }
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n buckles $p about $s waist.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You buckle $p about your waist.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_WAIST);
         oprog_wear_trigger(ch, obj);
         return;

      case ITEM_WEAR_SHIELD:
         if (get_eq_char(ch, WEAR_DUAL_WIELD) || (get_eq_char(ch, WEAR_WIELD) && get_eq_char(ch, WEAR_MISSILE_WIELD)))
         {
            send_to_char("You can't use a shield AND two weapons!\n\r", ch);
            return;
         }
         if (get_eq_char(ch, WEAR_WIELD) && (ch->con_rarm == -1 || ch->con_larm == -1))
         {
            send_to_char("You don't have an available hand seeing how it is broken off.\n\r", ch);
            return;
         }
         if ((tmpobj = (get_eq_char(ch, WEAR_MISSILE_WIELD))) != NULL)
         {
            if (IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
            {
               if (IS_NPC(ch) || ch->pcdata->learned[gsn_inhuman_strength] <= 0)
               {
                  send_to_char("The weapon you are wielding now is two-handed.\n\r", ch);
                  return;
               }
            }
         }
         if ((tmpobj = (get_eq_char(ch, WEAR_WIELD))) != NULL)
         {
            if (IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
            {
               if (IS_NPC(ch) || ch->pcdata->learned[gsn_inhuman_strength] <= 0)
               {
                  send_to_char("The weapon you are wielding now is two-handed.\n\r", ch);
                  return;
               }
            }
         }
         if (IS_OBJ_STAT(obj, ITEM_TWOHANDED))
         {
            if (get_eq_char(ch, WEAR_WIELD) || get_eq_char(ch, WEAR_DUAL_WIELD) || get_eq_char(ch, WEAR_MISSILE_WIELD))
            {
               send_to_char("That shield is two-handed.  You cannot wield a weapon with it.\n\r", ch);
               return;
            }
         }
         if (ch->con_rarm == -1 && ch->con_larm == -1)
         {
            send_to_char("You have no available hands to hold a shield because you have no hands.\n\r", ch);
            return;
         }
         if (!remove_obj(ch, WEAR_SHIELD, fReplace))
            return;
         if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
         {
            act(AT_ACTION, "$n uses $p as a shield.", ch, obj, NULL, TO_ROOM);
            act(AT_ACTION, "You use $p as a shield.", ch, obj, NULL, TO_CHAR);
         }
         equip_char(ch, obj, WEAR_SHIELD);
         oprog_wear_trigger(ch, obj);
         return;

      case ITEM_MISSILE_WIELD:
      case ITEM_WIELD:
         if (!could_dual(ch))
         {
            if (!remove_obj(ch, WEAR_MISSILE_WIELD, fReplace))
               return;
            if (!remove_obj(ch, WEAR_WIELD, fReplace))
               return;
            if ((tmpobj = get_eq_char(ch, WEAR_SHIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
            {
               send_to_char("The Shield you are holding requires two hands.\n\r", ch);
               return;
            }
            if (IS_OBJ_STAT(obj, ITEM_TWOHANDED) && tmpobj)
            {
               if (IS_NPC(ch) || ch->pcdata->learned[gsn_inhuman_strength] <= 0)
               {
                  send_to_char("If you want to use that two-handed weapon, remove your shield first.\n\r", ch);
                  return;
               }
            }
            tmpobj = NULL;
            if ((tmpobj = get_eq_char(ch, WEAR_SHIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
            {
               send_to_char("The Shield you are holding requires two hands.\n\r", ch);
               return;
            }
            if (get_eq_char(ch, WEAR_SHIELD) && (ch->con_rarm == -1 || ch->con_larm == -1))
            {
               send_to_char("Hard to wield something when one hand is chopped off.\n\r", ch);
               return;
            }
            if (ch->con_rarm == -1 && ch->con_larm == -1)
            {
               send_to_char("Hard to wield something when you have NO hands to wield it with.\n\r", ch);
               return;
            }
            tmpobj = NULL;
         }
         else
         {
            if ((tmpobj = get_eq_char(ch, WEAR_SHIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
            {
               send_to_char("The Shield you are holding requires two hands.\n\r", ch);
               return;
            }
            if (IS_OBJ_STAT(obj, ITEM_TWOHANDED) && tmpobj)
            {
               if (IS_NPC(ch) || ch->pcdata->learned[gsn_inhuman_strength] <= 0)
               {
                  send_to_char("If you want to use that two-handed weapon, remove your shield first.\n\r", ch);
                  return;
               }
            }
            tmpobj = NULL;
            if ((tmpobj = get_eq_char(ch, WEAR_WIELD)) != NULL && (get_eq_char(ch, WEAR_MISSILE_WIELD) || get_eq_char(ch, WEAR_DUAL_WIELD)))
            {
               send_to_char("You're already wielding two weapons.\n\r", ch);
               return;
            }
            if (IS_OBJ_STAT(obj, ITEM_TWOHANDED) && get_eq_char(ch, WEAR_MISSILE_WIELD))
            {
               send_to_char("That weapon is two-handed and you are weilding a missile weapon.\n\r", ch);
               return;
            }
            if (get_eq_char(ch, WEAR_MISSILE_WIELD))
            {
               if ((tmpobj = get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
               {
                  send_to_char("You are wielding a two-handed missile weapon!\n\r", ch);
                  return;
               }
            }
            if (get_eq_char(ch, WEAR_WIELD))
            {
               if ((tmpobj = get_eq_char(ch, WEAR_WIELD)) != NULL && get_eq_char(ch, WEAR_SHIELD))
               {
                  send_to_char("You must first remove that shield you are holding.\n\r", ch);
                  return;
               }
            }
            if (get_eq_char(ch, WEAR_WIELD) && (ch->con_rarm == -1 || ch->con_larm == -1))
            {
               send_to_char("You do not have availability of one hand because it is chopped off.\n\r", ch);
               return;
            }
            if (!tmpobj && ch->con_rarm == -1 && ch->con_larm == -1)
            {
               send_to_char("Hard to wield something when you have no hands.\n\r", ch);
               return;
            }
         }

         if (tmpobj)
         {
            if (can_dual(ch))
            {
               if (get_obj_weight(obj) + get_obj_weight(tmpobj) > str_app[get_curr_str(ch)].wield)
               {
                  send_to_char("&RWielding such heavy weapons is going to slow you down!!&w.\n\r", ch);
               }
               /* New Weapon Handling code -- Xerves 6/1/99 */
               if (!IS_NPC(ch))
               {
                  if (obj->value[3] < race_table[ch->race]->weaponmin)
                  {
                     act(AT_ACTION, "You try to wield $p, but you cannot grasp it with your hands.", ch, obj, NULL, TO_CHAR);
                     return;
                  }
                  if (obj->value[3] > race_table[ch->race]->weaponmax)
                  {
                     act(AT_ACTION, "You try to wield $p, but you cannot even lift it off the ground.", ch, obj, NULL, TO_CHAR);
                     return;
                  }
               }
               if (IS_OBJ_STAT(obj, ITEM_TWOHANDED))
               {
                  send_to_char("That weapon requires two hands to wield.\n\r", ch);
                  return;
               }
               if (IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
               {
                  send_to_char("The weapon you are wielding now is two-handed.\n\r", ch);
                  return;
               }
                   
               if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
               {
                  act(AT_ACTION, "$n dual-wields $p.", ch, obj, NULL, TO_ROOM);
                  act(AT_ACTION, "You dual-wield $p.", ch, obj, NULL, TO_CHAR);
               }
               if (1 << bit == ITEM_MISSILE_WIELD)
                  equip_char(ch, obj, WEAR_MISSILE_WIELD);
               else
                  equip_char(ch, obj, WEAR_DUAL_WIELD);
               oprog_wear_trigger(ch, obj);
            }
            return;
         }
         if (get_obj_weight(obj) > str_app[get_curr_str(ch)].wield)
         {
            send_to_char("&RWielding such a heavy weapon is going to slow you down!!&w.\n\r", ch);
         }
         /* New weapon handling code -- Xerves 6/1/90 */
         if (!IS_NPC(ch))
         {
            if (obj->value[3] < race_table[ch->race]->weaponmin)
            {
               act(AT_ACTION, "You try to wield $p, but you cannot grasp it with your hands.", ch, obj, NULL, TO_CHAR);
               return;
            }
            if (obj->value[3] > race_table[ch->race]->weaponmax)
            {
               act(AT_ACTION, "You try to wield $p, but you cannot even lift it off the ground.", ch, obj, NULL, TO_CHAR);
               return;
            }
         }
         if (!IS_NPC(ch))
         {
            if ((ch->pcdata->righthanded == 0 && ch->con_larm == -1) || (ch->pcdata->righthanded == 1 && ch->con_rarm == -1))
            {
               if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
               {
                   act(AT_ACTION, "$n wields $p with $s off-hand.", ch, obj, NULL, TO_ROOM);
                   act(AT_ACTION, "You wield $p with your off-hand.", ch, obj, NULL, TO_CHAR);
               }
            }
            else
            {
               if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
               {
                  act(AT_ACTION, "$n wields $p.", ch, obj, NULL, TO_ROOM);
                  act(AT_ACTION, "You wield $p.", ch, obj, NULL, TO_CHAR);
               }   
            }
         }
         else
         {
            if (ch->con_rarm == -1)
            {
               if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
               {
                   act(AT_ACTION, "$n wields $p with $s off-hand.", ch, obj, NULL, TO_ROOM);
                   act(AT_ACTION, "You wield $p with your off-hand.", ch, obj, NULL, TO_CHAR);
               }
            }
            else
            {
               if (!oprog_use_trigger(ch, obj, NULL, NULL, NULL))
               {
                  act(AT_ACTION, "$n wields $p.", ch, obj, NULL, TO_ROOM);
                  act(AT_ACTION, "You wield $p.", ch, obj, NULL, TO_CHAR);
               }   
            }   
         }
         if (1 << bit == ITEM_MISSILE_WIELD)
            equip_char(ch, obj, WEAR_MISSILE_WIELD);
         else
            equip_char(ch, obj, WEAR_WIELD);
         oprog_wear_trigger(ch, obj);
         if (ch->fighting)
         {
            SET_BIT(ch->fight_state, FSTATE_WIELD);
            if (ch->fight_timer <= 2)
               ch->fight_timer = 2;
         }
         return;
   }
}

// Used to unequip a weapon and put it back in a sheath.
// You sheath going from DUAL to Primary
void do_sheath(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *sheath;
   OBJ_DATA *weapon = NULL;
   char buf[MSL];
   int found = 0;
   int rev = 0;
   int count = 0;

   if (!str_cmp(argument, "go reverse"))
      rev = 1;

   if (!str_cmp(argument, "go reverse again"))
      rev = 2;

   if (ms_find_obj(ch))
      return;

   if (get_eq_char(ch, WEAR_DUAL_WIELD))
   {
      weapon = get_eq_char(ch, WEAR_DUAL_WIELD);
   }
   else
   {
      if (get_eq_char(ch, WEAR_WIELD))
      {
         weapon = get_eq_char(ch, WEAR_WIELD);
      }
   }
   if (!weapon)
   {
      send_to_char("You need to be wielding a weapon to sheath it.\n\r", ch);
      return;
   }
   if (IS_OBJ_STAT(weapon, ITEM_NOREMOVE))
   {
      act(AT_PLAIN, "You can't remove $p.", ch, weapon, NULL, TO_CHAR);
      return;
   }
   /* Total of 3 possible sheath locations, will loop through three times to find
      a good one if it has to */
   for (sheath = ch->first_carrying; sheath; sheath = sheath->next_content)
   {
      if (sheath->item_type == ITEM_SHEATH && sheath->wear_loc == WEAR_WAIST)
      {
         found = 1;
         if (rev != count)
         {
            count++;
            continue;
         }
         if (sheath->value[1] == weapon->value[3] && get_obj_weight(weapon) <= sheath->value[0] && !sheath->first_content)
         {
            if (sheath->value[2] != 0 && sheath->value[2] == weapon->pIndexData->vnum)
               break;
            else
            {
               if (sheath->value[2] == 0)
                  break;
               else
                  continue;
            }

            break;
         }
      }
   }
   if (found == 0)
   {
      send_to_char("You need to be wearing a sheath before you can sheath a weapon.\n\r", ch);
      return;
   }
   if (!sheath)
   {
      send_to_char("You have an improper sheath or it is full, help sheath for more info.\n\r", ch);
      return;
   }
   if (IS_OBJ_STAT(weapon, ITEM_NOGIVE) && !IS_OBJ_STAT(sheath, ITEM_NOGIVE))
   {
      if (rev == 0)
      {
         do_sheath(ch, "go reverse");
         return;
      }
      else if (rev == 1)
      {
         do_sheath(ch, "go reverse again");
         return;
      }
      else
      {
         send_to_char("Your container needs to be nogive if your weapon is nogive.\n\r", ch);
         return;
      }
   }
   if (IS_OBJ_STAT(sheath, ITEM_NOGIVE) && !IS_OBJ_STAT(weapon, ITEM_NOGIVE))
   {
      if (rev == 0)
      {
         do_sheath(ch, "go reverse");
         return;
      }
      else if (rev == 1)
      {
         do_sheath(ch, "go reverse again");
         return;
      }
      else
      {
         send_to_char("Your weapon needs to be nogive if your container is nogive.\n\r", ch);
         return;
      }
   }
   if ((weapon->pIndexData->vnum != sheath->value[2]) && sheath->value[2] != 0)
   {
      if (rev == 0)
      {
         do_sheath(ch, "go reverse");
         return;
      }
      else if (rev == 1)
      {
         do_sheath(ch, "go reverse again");
         return;
      }
      else
      {
         send_to_char("Your sheath was made for a certain weapon and this one is not it.\n\r", ch);
         return;
      }
   }
   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }
   sprintf(buf, "'%s'", weapon->name);
   do_remove(ch, buf);
   sprintf(buf, "'%s' '%s'", weapon->name, sheath->name);
   do_put(ch, buf);

   if (!sheath->first_content)
   {
      sprintf(buf, "'%s'", weapon->name);
      do_wear(ch, buf);
   }
   return;
}

// Used to draw a weapon from a sheath for combat.
void do_draw(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *sheath;
   OBJ_DATA *weapon = NULL;
   OBJ_DATA *tmpobj;
   char buf[MSL];
   char arg[MIL];
   int found = 0;
   int sn;
   int limb;

   if (ms_find_obj(ch))
      return;

   for (sheath = ch->first_carrying; sheath; sheath = sheath->next_content)
   {
      if (sheath->item_type == ITEM_SHEATH && sheath->wear_loc == WEAR_WAIST)
      {
         found = 1;
         if (sheath->first_content)
         {
            weapon = sheath->first_content;
            break;
         }
      }
   }
   if (found == 0)
   {
      send_to_char("You need to be wearing a sheath on your waist in order to draw.\n\r", ch);
      return;
   }
   if (!weapon)
   {
      send_to_char("You don't have anything in your sheath to wield you fool.\n\r", ch);
      return;
   }
   /* Quick checks to see if it is wieldable before removing, will safe some text spam on the
      player plus problems */
   
   if (!could_dual(ch))
   {
      if (!remove_obj(ch, WEAR_MISSILE_WIELD, 2))
         return;
      if (!remove_obj(ch, WEAR_WIELD, 2))
         return;
      if ((tmpobj = get_eq_char(ch, WEAR_SHIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
      {
         if (IS_NPC(ch) || ch->pcdata->learned[gsn_inhuman_strength] <= 0)
            return;
      }
      if (get_eq_char(ch, WEAR_SHIELD) && (ch->con_rarm == -1 || ch->con_larm == -1))
      {
         return;
      }
      if (ch->con_rarm == -1 && ch->con_larm == -1)
      {
         return;
      }
      tmpobj = NULL;   
       
      if (get_obj_weight(weapon) > str_app[get_curr_str(ch)].wield)
      {
         send_to_char("&RWielding such a heavy weapon is going to slow you down!!&w.\n\r", ch);
      }
   }   
   else
   {
      if ((tmpobj = get_eq_char(ch, WEAR_SHIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
      {
         return;
      }
      if (IS_OBJ_STAT(weapon, ITEM_TWOHANDED) && tmpobj)
      {
         if (IS_NPC(ch) || ch->pcdata->learned[gsn_inhuman_strength] <= 0)
            return;
      }
      tmpobj = NULL;
      if ((tmpobj = get_eq_char(ch, WEAR_WIELD)) != NULL && (get_eq_char(ch, WEAR_MISSILE_WIELD) || get_eq_char(ch, WEAR_DUAL_WIELD)))
         return;

      if (IS_OBJ_STAT(weapon, ITEM_TWOHANDED) && get_eq_char(ch, WEAR_MISSILE_WIELD))
      {
         return;
      }
      if (get_eq_char(ch, WEAR_MISSILE_WIELD))
      {
         if ((tmpobj = get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
         {
            return;
         }
      }
      if (get_eq_char(ch, WEAR_WIELD))
      {
         if ((tmpobj = get_eq_char(ch, WEAR_WIELD)) != NULL && get_eq_char(ch, WEAR_SHIELD))
         {
            return;
         }
      }
      if (get_eq_char(ch, WEAR_WIELD) && (ch->con_rarm == -1 || ch->con_larm == -1))
      {
         return;
      }
      if (!tmpobj && ch->con_rarm == -1 && ch->con_larm == -1)
      {
         return;
      }
      if (tmpobj)
      {
         if (get_obj_weight(weapon) + get_obj_weight(tmpobj) > str_app[get_curr_str(ch)].wield)
         {
            send_to_char("&RWielding such heavy weapons is going to slow you down!!&w.\n\r", ch);
         }
      } 
      else
      {
         if (get_obj_weight(weapon) > str_app[get_curr_str(ch)].wield)
         {
            send_to_char("&RWielding such a heavy weapon is going to slow you down!!&w.\n\r", ch);
         }
      }
   }
   /* New Weapon Handling code -- Xerves 6/1/99 */
   if (!IS_NPC(ch))
   {
      if (weapon->value[3] < race_table[ch->race]->weaponmin)
      {
         act(AT_ACTION, "You try to wield $p, but you cannot grasp it with your hands.", ch, weapon, NULL, TO_CHAR);
         return;
      }
      if (weapon->value[3] > race_table[ch->race]->weaponmax)
      {
         act(AT_ACTION, "You try to wield $p, but you cannot even lift it off the ground.", ch, weapon, NULL, TO_CHAR);
         return;
      }
   }
   if ((IS_OBJ_STAT(weapon, ITEM_NOGIVE) && !IS_OBJ_STAT(sheath, ITEM_NOGIVE))
      || (IS_OBJ_STAT(sheath, ITEM_NOGIVE) && !IS_OBJ_STAT(weapon, ITEM_NOGIVE)))
   {
      send_to_char("Either the weapon or sheath has a nogive flag, bug reported\n\r", ch);
      bug("%s has a nogive flag weapon or container, trying to draw", ch->name);
      return;
   }
   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }
   sprintf(buf, "'%s' '%s'", weapon->name, sheath->name);
   do_get(ch, buf);
   sprintf(buf, "'%s'", weapon->name);
   do_wear(ch, buf);
   if (weapon->wear_loc != WEAR_WIELD && weapon->wear_loc != WEAR_MISSILE_WIELD && weapon->wear_loc != WEAR_DUAL_WIELD)
   {
      sprintf(buf, "'%s' '%s'", weapon->name, sheath->name);
      do_put(ch, buf);
   }
   if (argument[0] != '\0')
   {
      CHAR_DATA *victim;

      argument = one_argument(argument, arg);

      if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }

      if (IS_NPC(victim) && victim->morph)
      {
         send_to_char("This creature appears strange to you.  Look upon it more closely before attempting to kill it.", ch);
         return;
      }

      if (victim == ch)
      {
         send_to_char("You hit yourself.  Ouch!\n\r", ch);
         return;
      }

      if (is_safe(ch, victim))
         return;

      if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
      {
         act(AT_PLAIN, "$N is your beloved master.", ch, NULL, victim, TO_CHAR);
         return;
      }

      if (ch->position == POS_FIGHTING
         || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
      {
         send_to_char("You do the best you can!\n\r", ch);
         return;
      }
      if (argument[0] == '\0')
      {
         send_to_char("Need to choose a limb: rarm, larm, rleg, lleg, body, head, neck.\n\r", ch);
         return;
      }
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
         send_to_char("Need to choose a limb: rarm, larm, rleg, lleg, body, head, neck.\n\r", ch);
         return;
      }  
      check_attacker(ch, victim);
      one_hit(ch, victim, gsn_unsheath, limb);
      sn = sheath->value[4];
      if (sn > 0)
      {
         if (IS_VALID_SN(sn) && skill_table[sn]->spell_fun)
         {
            if (sheath->value[5] > 0)
            {
               if (number_range(1, 100) > sheath->value[5])
                  return;
            }
            else
            {
               if (number_range(1, 100) > 25)
                  return;
            }
            (*skill_table[sn]->spell_fun) (sn, 1, ch, victim);
            if (char_died(ch) || char_died(victim))
               return;
         }
      }
      return;
   }
   if (ch->fighting && ch->fighting->who != ch)
   {
      check_attacker(ch, ch->fighting->who);
      if (argument[0] == '\0')
      {
         send_to_char("Need to choose a limb: rarm, larm, rleg, lleg, body, head, neck\n\r", ch);
         return;
      }
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
      one_hit(ch, ch->fighting->who, gsn_unsheath, limb);
   }
   sn = sheath->value[4];
   if (sn > 0)
   {
      if (IS_VALID_SN(sn) && skill_table[sn]->spell_fun && ch->fighting && ch->fighting->who != ch)
      {
         if (sheath->value[5] > 0)
         {
            if (number_range(1, 100) > sheath->value[5])
               return;
         }
         else
         {
            if (number_range(1, 100) > 25)
               return;
         }
         (*skill_table[sn]->spell_fun) (sn, 1, ch, ch->fighting->who);
         if (char_died(ch) || char_died(ch->fighting->who))
            return;
      }
   }
   return;
}
void do_wear(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   OBJ_DATA *obj;
   sh_int wear_bit;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if ((!str_cmp(arg2, "on") || !str_cmp(arg2, "upon") || !str_cmp(arg2, "around")) && argument[0] != '\0')
      argument = one_argument(argument, arg2);

   if (arg1[0] == '\0')
   {
      send_to_char("Wear, wield, or hold what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if (!str_cmp(arg1, "all"))
   {
      OBJ_DATA *obj_next;

      for (obj = ch->first_carrying; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         if (IS_SET(obj->wear_flags, ITEM_LODGE_RIB) || IS_SET(obj->wear_flags, ITEM_LODGE_ARM) || IS_SET(obj->wear_flags, ITEM_LODGE_LEG))
            continue;

         if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj))
         {
            wear_obj(ch, obj, FALSE, -1);
            if (char_died(ch))
               return;
         }
      }
      return;
   }
   else
   {
      if ((obj = get_obj_carry(ch, arg1)) == NULL)
      {
         send_to_char("You do not have that item.\n\r", ch);
         return;
      }
      if (IS_SET(obj->wear_flags, ITEM_LODGE_RIB) || IS_SET(obj->wear_flags, ITEM_LODGE_ARM) || IS_SET(obj->wear_flags, ITEM_LODGE_LEG))
      {
         send_to_char("I do not think you want to wear that.\n\r", ch);
         return;
      }
      if (arg2[0] != '\0')
         wear_bit = get_wflag(arg2);
      else
         wear_bit = -1;
      wear_obj(ch, obj, TRUE, wear_bit);
   }

   return;
}



void do_remove(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj, *obj_next;


   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Remove what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if (!str_cmp(arg, "all")) /* SB Remove all */
   {
      act(AT_PLAIN, "$n starts to remove it all.", ch, NULL, NULL, TO_ROOM);
      for (obj = ch->first_carrying; obj != NULL; obj = obj_next)
      {
         obj_next = obj->next_content;
         if (IS_SET(obj->wear_flags, ITEM_LODGE_RIB) || IS_SET(obj->wear_flags, ITEM_LODGE_ARM) || IS_SET(obj->wear_flags, ITEM_LODGE_LEG))
            continue;

         if (obj->wear_loc != WEAR_NONE && can_see_obj(ch, obj))
            remove_obj(ch, obj->wear_loc, TRUE);
      }
      return;
   }

   if ((obj = get_obj_wear(ch, arg)) == NULL)
   {
      send_to_char("You are not using that item.\n\r", ch);
      return;
   }
   if ((obj_next = get_eq_char(ch, obj->wear_loc)) != obj)
   {
      act(AT_PLAIN, "You must remove $p first.", ch, obj_next, NULL, TO_CHAR);
      return;
   }
   if (IS_SET(obj->wear_flags, ITEM_LODGE_RIB) || IS_SET(obj->wear_flags, ITEM_LODGE_ARM) || IS_SET(obj->wear_flags, ITEM_LODGE_LEG))
   {
      send_to_char("You cannot remove those kind items, use dislodge.\n\r", ch);
      return;
   }

   remove_obj(ch, obj->wear_loc, TRUE);
   return;
}


void do_bury(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   bool shovel;
   sh_int move;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("What do you wish to bury?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   shovel = FALSE;
   for (obj = ch->first_carrying; obj; obj = obj->next_content)
      if (obj->item_type == ITEM_SHOVEL)
      {
         shovel = TRUE;
         break;
      }

   obj = get_obj_list_rev(ch, arg, ch->in_room->last_content);
   if (!obj)
   {
      send_to_char("You can't find it.\n\r", ch);
      return;
   }

   separate_obj(obj);
   if (!CAN_WEAR(obj, ITEM_TAKE))
   {
      if (!IS_OBJ_STAT(obj, ITEM_CLANCORPSE) || IS_NPC(ch))
      {
         act(AT_PLAIN, "You cannot bury $p.", ch, obj, 0, TO_CHAR);
         return;
      }
   }

   switch (ch->in_room->sector_type)
   {
      case SECT_CITY:
      case SECT_INSIDE:
         send_to_char("The floor is too hard to dig through.\n\r", ch);
         return;
      case SECT_WATER_SWIM:
      case SECT_WATER_NOSWIM:
      case SECT_UNDERWATER:
         send_to_char("You cannot bury something here.\n\r", ch);
         return;
      case SECT_AIR:
         send_to_char("What?  In the air?!\n\r", ch);
         return;
   }

   if (obj->weight > (UMAX(5, (can_carry_w(ch) / 10))) && !shovel)
   {
      send_to_char("You'd need a shovel to bury something that big.\n\r", ch);
      return;
   }

   move = (obj->weight * 50 * (shovel ? 1 : 5)) / UMAX(1, can_carry_w(ch));
   move = URANGE(2, move, 1000);
   if (move > ch->move)
   {
      send_to_char("You don't have the energy to bury something of that size.\n\r", ch);
      return;
   }
   ch->move -= move;
   if (obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC)
      adjust_favor(ch, 6, 1);

   act(AT_ACTION, "You solemnly bury $p...", ch, obj, NULL, TO_CHAR);
   act(AT_ACTION, "$n solemnly buries $p...", ch, obj, NULL, TO_ROOM);
   xSET_BIT(obj->extra_flags, ITEM_BURIED);
   WAIT_STATE(ch, URANGE(10, move / 2, 100));
   return;
}

void do_sacrifice(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MSL];
   char name[50];
   OBJ_DATA *obj;
   OBJ_DATA *obj_prev;
   int x;

   one_argument(argument, arg);
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  sacrifice <name of object>\n\r", ch);
      send_to_char("Syntax:  sacrifice all\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "all"))
   {
      sacall = 1;
      for (x = 1; x <= 15; x++) //Due to separate obj this needs to be run a few times to clean up the mess
      {
         for (obj = ch->in_room->last_content; obj; obj = obj_prev)
         {
            obj_prev = obj->prev_content;
            if (IN_SAME_ROOM_OBJ(ch, obj))
            {
               sprintf(buf, "%s", obj->name);
               do_sacrifice(ch, buf);
            }
         }
      }
      sacall = 0;
      return;
   }

   if (!str_cmp(arg, ch->name))
   {
      act(AT_ACTION, "$n offers $mself to $s deity, who graciously declines.", ch, NULL, NULL, TO_ROOM);
      send_to_char("Your deity appreciates your offer and may accept it later.\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   obj = get_obj_list_rev(ch, arg, ch->in_room->last_content);
   if (!obj)
   {
      send_to_char("You can't find it.\n\r", ch);
      return;
   }

   separate_obj(obj);
   if (!CAN_WEAR(obj, ITEM_TAKE))
   {
      if (sacall == 0)
         act(AT_PLAIN, "$p is not an acceptable sacrifice.", ch, obj, 0, TO_CHAR);
      return;
   }
   if (!IS_NPC(ch) && ch->pcdata->deity && ch->pcdata->deity->name[0] != '\0')
   {
      strcpy(name, ch->pcdata->deity->name);
   }
   else if (!IS_NPC(ch) && IS_GUILDED(ch) && sysdata.guild_overseer[0] != '\0')
   {
      strcpy(name, sysdata.guild_overseer);
   }
   else if (!IS_NPC(ch) && ch->pcdata->clan && ch->pcdata->clan->deity[0] != '\0')
   {
      strcpy(name, ch->pcdata->clan->deity);
   }
   else
   {
      strcpy(name, "Xerves");
   }
   ch->gold += 1;
   if (obj->item_type == ITEM_CORPSE_NPC || obj->item_type == ITEM_CORPSE_PC)
      adjust_favor(ch, 5, 1);
   sprintf(buf, "%s gives you one gold coin for your sacrifice.\n\r", name);
   send_to_char(buf, ch);
   sprintf(buf, "$n sacrifices $p to %s.", name);
   act(AT_ACTION, buf, ch, obj, NULL, TO_ROOM);
   oprog_sac_trigger(ch, obj);
   if (obj_extracted(obj))
      return;
   if (cur_obj == obj->serial)
      global_objcode = rOBJ_SACCED;
   check_for_trap(ch, obj, -1, NEW_TRAP_SACOBJ);
   if (global_retcode == rOBJ_SCRAPPED)
      return;   
   if (obj)
      extract_obj(obj);
   return;
}

//Used on runes, for now, just for moving to a portal
void do_rub(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *rune;
   char arg1[MIL];
   int p, x, y, map;
   int count = 0;
   int found = 0;
   int targetRoomVnum;
   ROOM_INDEX_DATA *targetRoom;

   p = 0;

   if (IS_NPC(ch))
   {
      send_to_char("This command is for PCs only, sorry.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("rub <rune> <portal>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);

   if ((rune = get_obj_carry(ch, arg1)) == NULL)
   {
      send_to_char("You do not have that item in your inventory.\n\r", ch);
      return;
   }
   if (rune->item_type != ITEM_RUNE)
   {
      send_to_char("You can only rub runes.\n\r", ch);
      return;
   }
   if (rune->value[0] != 1)
   {
      send_to_char("You try to rub the rune, but it appears to not be enchanted.\n\r", ch);
      return;
   }
   if (can_use_portal(ch, 3) == FALSE)
      return;

   WAIT_STATE(ch, 3 * PULSE_VIOLENCE);

   if (isdigit(argument[0]))
   {
      for (p = 0; p < sysdata.last_portal; p++)
      {
         if (xIS_SET(ch->pcdata->portalfnd, p))
         {
            count++;
         }
         if (count == atoi(argument))
         {
            found = 1;
            break;
         }
      }
   }
   if (!str_cmp(argument, "home"))
   {
      if (ch->in_room->vnum == ROOM_VNUM_PORTAL)
      {
         send_to_char("You cannot use the rune to portal to a portal you are at.\n\r", ch);
         return;
      }
      else
         count = 10000; //Should be enough, god forbid 10000 portal spots -- Xerves
   }
   if (found == 0 && count < 10000)
   {
      send_to_char("Your rune will only work with portals on your list, or 'home' or 'newbie'.\n\r", ch);
      return;
   }
   if (count == 0)
   {
      send_to_char("You have no portals to goto.\n\r", ch);
      return;
   }
   if (count == 10000)
      targetRoomVnum = ROOM_VNUM_PORTAL;
   else
      targetRoomVnum = OVERLAND_SOLAN;

   if (count != 10000 && count != 10001)
   {
      x = portal_show[p]->x;
      y = portal_show[p]->y;
      map = portal_show[p]->map;
      if (ch->coord->x == x && ch->coord->y == y && ch->map == map)
      {
         send_to_char("You cannot use the rune to portal to a portal you are at.\n\r", ch);
         return;
      }
   }
   else
   {
      x = -1;
      y = -1;
      map = -1;
      if (ch->in_room->vnum == targetRoomVnum)
      {
         send_to_char("You cannot use the rune to portal to a portal you are at.\n\r", ch);
         return;
      }
   }
   targetRoom = get_room_index(targetRoomVnum);
   if (x > -1 || y > -1 || map > -1)
   {
      SET_ONMAP_FLAG(ch);
      ch->coord->x = x;
      ch->coord->y = y;
      ch->map = map;
      char_from_room(ch);
      char_to_room(ch, targetRoom);
      update_objects(ch, x, y, ch->map);
      if (ch->rider)
      {
         SET_ONMAP_FLAG(ch->rider);
         ch->rider->coord->x = x;
         ch->rider->coord->y = y;
         ch->rider->map = map;
         char_from_room(ch->rider);
         char_to_room(ch->rider, targetRoom); 
         update_objects(ch->rider, x, y, ch->map);  
      }
   }
   else
   {
      REMOVE_ONMAP_FLAG(ch);
      ch->coord->x = -1;
      ch->coord->y = -1;
      ch->map = -1;
      char_from_room(ch);
      char_to_room(ch, targetRoom);
      update_objects(ch, x, y, ch->map);
      if (ch->rider)
      {
         REMOVE_ONMAP_FLAG(ch->rider);
         ch->rider->coord->x = -1;
         ch->rider->coord->y = -1;
         ch->rider->map = -1;
         char_from_room(ch->rider);
         char_to_room(ch->rider, targetRoom);
         update_objects(ch->rider, x, y, ch->map);
      }
   }
   if (ch->on)
   {
      ch->on = NULL;
      ch->position = POS_STANDING;
   }
   if (ch->position != POS_STANDING)
   {
      ch->position = POS_STANDING;
   }
   act(AT_MAGIC, "$n appears in a flash of light!", ch, NULL, NULL, TO_ROOM);
   do_look(ch, "auto");

   if (--rune->value[1] <= 0)
   {
      act(AT_MAGIC, "$p lights up and suddenly and vanishes from $n", ch, rune, NULL, TO_ROOM);
      act(AT_MAGIC, "$p lights up and suddenly and vanishes from your inventory.", ch, rune, NULL, TO_CHAR);
      if (rune->serial == cur_obj)
         global_objcode = rOBJ_USED;
      separate_obj(rune);
      extract_obj(rune);
   }
   return;
}


void do_brandish(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim = NULL;
   OBJ_DATA *staff;
   int value = 0;
   ch_ret retcode;
   static char extra[MSL];
   int passarg = 0;
   int sn;

   if ((staff = get_eq_char(ch, WEAR_WIELD)) == NULL)
   {
      send_to_char("You not wielding anything to brandish.\n\r", ch);
      return;
   }

   if ((sn = staff->value[4]) < 0 || sn >= top_sn || skill_table[sn]->spell_fun == NULL)
   {
      bug("Do_brandish: bad sn %d.", sn);
      return;
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }
   if (ch->fighting)
      ch->fight_timer = staff->value[5] - SPOWER_MIN + 2;
   else
      WAIT_STATE(ch, staff->value[5] - SPOWER_MIN + 2);

   if (staff->value[5] > 0)
   {
      if (!oprog_use_trigger(ch, staff, NULL, NULL, NULL))
      {
         act(AT_MAGIC, "$n brandishes $p.", ch, staff, NULL, TO_ROOM);
         act(AT_MAGIC, "You brandish $p.", ch, staff, NULL, TO_CHAR);
      }
      if (skill_table[sn]->target == TAR_CHAR_OFFENSIVE && ch->fighting)
         victim = ch->fighting->who;
      if (skill_table[sn]->target == TAR_CHAR_DEFENSIVE)
         victim = ch;
      if (staff->value[4] != skill_lookup("word of recall"))
      {
         if (argument[0] != '\0')
         {
            if ((victim=get_char_room(ch, argument)) == NULL)
            {
               send_to_char("Your target is not in this room.\n\r", ch);
               return;
            }
         }
      }
      else
      {
         victim = ch;
         sprintf(extra, argument);
         target_name = extra;
         passarg = 1;
      }
      if (skill_table[sn]->target == TAR_CHAR_SELF)
         victim = ch;
      if (skill_table[sn]->target != TAR_IGNORE)
      {
         if (!victim)
         {
            send_to_char("You need a target to brandish your weapon on.\n\r", ch);
            return;
         }
      }
      if (victim && !IS_NPC(victim) && xIS_SET(victim->act, PLR_WIZINVIS) && victim->pcdata->wizinvis >= LEVEL_IMMORTAL)
      {
         send_to_char("Your target is not in this room.\n\r", ch);
         return;
      }
      switch (skill_table[sn]->target)
      {
         case TAR_CHAR_SELF:
            if (victim != ch)
            {
               send_to_char("Can only target yourself with this weapon.\n\r", ch);
               return;
            }
      }
      if (staff->value[5] < SPOWER_MIN || staff->value[5] > SPOWER_GREATEST)
      {
         staff->value[5] = SPOWER_MIN;
         bug("Spower on object %s on %s is bad.", staff->name, ch->name);
      }
      if (!passarg)
         retcode = obj_cast_spell(staff->value[4], staff->value[5], ch, victim, staff);
      else
         retcode = obj_cast_spell(staff->value[4]+10000, staff->value[5], ch, victim, staff);
      if (retcode == rCHAR_DIED || retcode == rBOTH_DIED)
      {
         bug("do_brandish: char died", 0);
         return;
      }

      if (staff->value[5] == SPOWER_MIN)
         value = 15;
      if (staff->value[5] == SPOWER_LOW)
         value = 25;
      if (staff->value[5] == SPOWER_MED)
         value = 40;
      if (staff->value[5] == SPOWER_HI)
         value = 60;
      if (staff->value[5] == SPOWER_GREAT)
         value = 90;
      if (staff->value[5] == SPOWER_GREATER)
         value = 130;
      if (staff->value[5] == SPOWER_GREATEST)
         value = 180;
      value = value * (100-(staff->value[10]*4)) / 100;
      staff->value[0] -= number_range(value*80/100, value*120/100);
      if (staff->value[0] <= 0)
      {
         act(AT_MAGIC, "$p blazes bright and vanishes from $n's hands!", ch, staff, NULL, TO_ROOM);
         act(AT_MAGIC, "$p blazes bright and is gone!", ch, staff, NULL, TO_CHAR);
         if (staff->serial == cur_obj)
            global_objcode = rOBJ_USED;
         extract_obj(staff);
      }
   }

   return;
}


//No reason, just use brandish
void do_zap(CHAR_DATA * ch, char *argument)
{
   do_brandish(ch, argument);
   return;
}

/*
 * Save items in a clan storage room			-Scryn & Thoric
 */
void save_clan_storeroom(CHAR_DATA * ch, CLAN_DATA * clan)
{
   FILE *fp;
   char filename[256];
   OBJ_DATA *contents;

   if (!clan)
   {
      bug("save_clan_storeroom: Null clan pointer!", 0);
      return;
   }

   if (!ch)
   {
      bug("save_clan_storeroom: Null ch pointer!", 0);
      return;
   }

   sprintf(filename, "%s%s.vault", CLAN_DIR, clan->filename);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_clan_storeroom: fopen", 0);
      perror(filename);
   }
   else
   {
      fprintf(fp, "#VERSION\n");
      fprintf(fp, "Version    %d\n", SAVEVERSION);
      fprintf(fp, "End\n\n");
      contents = ch->in_room->last_content;
      if (contents)
      {
         fwrite_obj(ch, contents, fp, 0, OS_CARRY);
      }
      fprintf(fp, "#END\n");
      fclose(fp);
      return;
   }
   return;
}

/* put an item on , or see the stats on the current item or bet */
void do_auction(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];
   int i;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);

   set_char_color(AT_LBLUE, ch);

   if (IS_NPC(ch)) /* NPC can be extracted at any time and thus can't auction! */
      return;

   /*  I pity the foo that doesn't work nights....
   if ((gethour() > 18 || gethour() < 9) && auction->item == NULL && !IS_IMMORTAL(ch))
   {
      send_to_char("\n\rThe auctioneer works between the hours of 9 AM and 6 PM\n\r", ch);
      return;
   } */

   if (arg1[0] == '\0')
   {
      if (auction->item != NULL)
      {

         obj = auction->item;

         /* show item data here */
         if (auction->bet > 0)
            sprintf(buf, "\n\rCurrent bid on this item is %d gold.\n\r", auction->bet);
         else
            sprintf(buf, "\n\rNo bids on this item have been received.\n\r");
         set_char_color(AT_BLUE, ch);
         send_to_char(buf, ch);

         code_identify(ch, obj, NULL, 1, NULL);

         if ((obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_KEYRING || obj->item_type == ITEM_QUIVER) && obj->first_content)
         {
            set_char_color(AT_OBJECT, ch);
            send_to_char("Contents:\n\r", ch);
            show_list_to_char( obj->first_content, ch, TRUE, FALSE, eItemBid );
         }

         if (IS_IMMORTAL(ch))
         {
            sprintf(buf, "Seller: %s.  Bidder: %s.  Round: %d.\n\r", auction->seller->name, auction->buyer->name, (auction->going + 1));
            send_to_char(buf, ch);
            sprintf(buf, "Time left in round: %d.\n\r", auction->pulse);
            send_to_char(buf, ch);
         }
         return;
      }
      else
      {
         set_char_color(AT_LBLUE, ch);
         send_to_char("\n\rThere is nothing being auctioned right now.  What would you like to auction?\n\r", ch);
         return;
      }
   }

   if (IS_IMMORTAL(ch) && !str_cmp(arg1, "stop"))
   {
      if (auction->item == NULL)
      {
         send_to_char("There is no auction to stop.\n\r", ch);
         return;
      }
      else /* stop the auction */
      {
         set_char_color(AT_LBLUE, ch);
         sprintf(buf, "Sale of %s has been stopped by an Immortal.", auction->item->short_descr);
         talk_auction(buf);
         obj_to_char(auction->item, auction->seller);
         if (IS_SET(sysdata.save_flags, SV_AUCTION))
            save_char_obj(auction->seller);
         auction->item = NULL;
         if (auction->buyer != NULL && auction->buyer != auction->seller) /* return money to the buyer */
         {
            auction->buyer->gold += auction->bet;
            send_to_char("Your money has been returned.\n\r", auction->buyer);
         }
         return;
      }
   }

   if (!str_cmp(arg1, "bid"))
   {
      if (auction->item != NULL)
      {
         int newbet;

         if (ch->level < auction->item->level)
         {
            send_to_char("This object's level is too high for your use.\n\r", ch);
            return;
         }

         if (ch == auction->seller)
         {
            send_to_char("You can't bid on your own item!\n\r", ch);
            return;
         }
         
         if (IS_UNIQUE(ch, auction->item))
         {
            send_to_char("That item is unique and you already have one.\n\r", ch);
            return;
         }

         /* make - perhaps - a bet now */
         if (arg2[0] == '\0')
         {
            send_to_char("Bid how much?\n\r", ch);
            return;
         }

         newbet = parsebet(auction->bet, arg2);
/*	    ch_printf( ch, "Bid: %d\n\r",newbet);	*/

         if (newbet < auction->starting)
         {
            send_to_char("You must place a bid that is higher than the starting bet.\n\r", ch);
            return;
         }

         /* to avoid slow auction, use a bigger amount than 100 if the bet
            is higher up - changed to 10000 for our high economy
          */
         /* Blah, changed it back -- Xerves 11/99 */

         if (newbet < (auction->bet + 100))
         {
            send_to_char("You must at least bid 100 coins over the current bid.\n\r", ch);
            return;
         }

         if (newbet > ch->gold)
         {
            send_to_char("You don't have that much money!\n\r", ch);
            return;
         }

         if (newbet > 2000000000)
         {
            send_to_char("You can't bid over 2 billion coins.\n\r", ch);
            return;
         }

         /* Is it the item they really want to bid on? --Shaddai */
         if (arg3[0] != '\0' && !nifty_is_name(arg3, auction->item->name))
         {
            send_to_char("That item is not being auctioned right now.\n\r", ch);
            return;
         }
         /* the actual bet is OK! */

         /* return the gold to the last buyer, if one exists */
         if (auction->buyer != NULL && auction->buyer != auction->seller)
            auction->buyer->gold += auction->bet;

         ch->gold -= newbet; /* substract the gold - important :) */
         if (IS_SET(sysdata.save_flags, SV_AUCTION))
            save_char_obj(ch);
         auction->buyer = ch;
         auction->bet = newbet;
         auction->going = 0;
         auction->pulse = PULSE_AUCTION; /* start the auction over again */

         sprintf(buf, "A bid of %d gold has been received on %s.\n\r", newbet, auction->item->short_descr);
         talk_auction(buf);
         return;


      }
      else
      {
         send_to_char("There isn't anything being auctioned right now.\n\r", ch);
         return;
      }
   }
   /* finally... */
   if (ms_find_obj(ch))
      return;

   obj = get_obj_carry(ch, arg1); /* does char have the item ? */

   if (obj == NULL)
   {
      send_to_char("You aren't carrying that.\n\r", ch);
      return;
   }

   if (obj->timer > 0)
   {
      send_to_char("You can't auction objects that are decaying.\n\r", ch);
      return;
   }
   if (get_timer(ch, TIMER_AUCTION) >= 1)
   {
      send_to_char("There is a 30 second wait after you auction an item, please wait.\n\r", ch);
      return;
   }

   /* prevent repeat auction items */
   for (i = 0; i < AUCTION_MEM && auction->history[i]; i++)
   {
      if (auction->history[i] == obj->pIndexData)
      {
         send_to_char("Such an item has been auctioned " "recently, try again later.\n\r", ch);
         return;
      }
   }


   if (arg2[0] == '\0')
   {
      auction->starting = obj->cost;
      sprintf(arg2, "%d", obj->cost);
   }
   
   if (!str_cmp(arg2, "lowest"))
   {
      auction->starting = UMAX(1, obj->cost/10);
      sprintf(arg2, "%d", UMAX(1, obj->cost/10));
   }

   if (!is_number(arg2))
   {
      send_to_char("You must input a number at which to start the auction.\n\r", ch);
      return;
   }

   if (atoi(arg2) < UMAX(1,obj->cost/10))
   {
      send_to_char("You can't auction something for less than 1/10 cost of the object!\n\r", ch);
      return;
   }

/* Cannot auction a nogive item, another block --Xerves 3/24/99 */
   if (IS_OBJ_STAT(obj, ITEM_NOGIVE))
   {
      send_to_char("Please give up!  You cannot auction this item!n\r", ch);
      return;
   }

   if (auction->item == NULL)
      switch (obj->item_type)
      {

         default:
            act(AT_TELL, "You cannot auction $Ts.", ch, NULL, item_type_name(obj), TO_CHAR);
            return;

/* insert any more item types here... items with a timer MAY NOT BE
   AUCTIONED!
*/
         case ITEM_LIGHT:
         case ITEM_TREASURE:
         case ITEM_POTION:
         case ITEM_CONTAINER:
         case ITEM_KEYRING:
         case ITEM_QUIVER:
         case ITEM_DRINK_CON:
         case ITEM_FOOD:
         case ITEM_COOK:
         case ITEM_PEN:
         case ITEM_BOAT:
         case ITEM_PIPE:
         case ITEM_INCENSE:
         case ITEM_FIRE:
         case ITEM_RUNEPOUCH:
         case ITEM_MAP:
         case ITEM_BOOK:
         case ITEM_RUNE:
         case ITEM_TGEM:
         case ITEM_MATCH:
         case ITEM_WEAPON:
         case ITEM_ARMOR:
         case ITEM_SCROLL:
            separate_obj(obj);
            obj_from_char(obj);
            if (IS_SET(sysdata.save_flags, SV_AUCTION))
               save_char_obj(ch);
            auction->item = obj;
            auction->bet = 0;
            auction->buyer = ch;
            auction->seller = ch;
            auction->pulse = PULSE_AUCTION;
            auction->going = 0;
            auction->starting = atoi(arg2);

            /* add the new item to the history */
            if (AUCTION_MEM > 0)
            {
               memmove((char *) auction->history + sizeof(OBJ_INDEX_DATA *), auction->history, (AUCTION_MEM - 1) * sizeof(OBJ_INDEX_DATA *));
               auction->history[0] = obj->pIndexData;
            }

            /* reset the history timer */
            auction->hist_timer = 0;


            if (auction->starting > 0)
               auction->bet = auction->starting;

            sprintf(buf, "A new item is being auctioned: %s at %d gold.", obj->short_descr, auction->starting);
            talk_auction(buf);

            return;

      } /* switch */
   else
   {
      act(AT_TELL, "Try again later - $p is being auctioned right now!", ch, auction->item, NULL, TO_CHAR);
      return;
   }
}

//EQ like Bazaar.  Will allow players to put up equipment to sell.  There will
//be a fee though and you don't have to sit around to sell the objects.
void do_market(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char buf[MSL];
   char buf2[MSL];
   int count = 0;
   int price = 0;
   int sellname = 0;
   OBJ_DATA *obj;
   OBJ_DATA *rest;
   MARKET_DATA *market;
   MARKET_DATA *marketnext;
   int num = 0;
   
   if (check_npc(ch))
      return;
   if (!xIS_SET(ch->in_room->room_flags, ROOM_MARKETPLACE))
   {
      send_to_char("You can only use this command at a marketplace.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  market list [search option] [value]\n\r", ch);
      send_to_char("Syntax:  market sell <name of item> [count|all] [cost|lowest] [list seller name <yes|no>]\n\r", ch);
      send_to_char("Syntax:  market buy <number> <count|all>\n\r", ch);
      send_to_char("Syntax:  market edit <number> <list|cost> <new value>\n\r", ch);
      send_to_char("Syntax:  market view <number>\n\r", ch);
      send_to_char("Syntax:  market claim\n\r", ch);
      send_to_char("Syntax:  seach options: all name maxcost mincost rangecost itemtype\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   if (!str_cmp(arg1, "claim"))
   {
      for (market = first_market; market; market = marketnext)
      {
         marketnext = market->next;
         if (market->count < market->scount && market->pid == ch->pcdata->pid)
         {
            price = market->cost * (market->scount - market->count);
            ch->gold += price - (price * 5 / 100);  
            num++;
            ch_printf(ch, "You sold %d of %d of %s and you earned %d (fee of %d)\n\r", 
               market->scount - market->count, market->scount, market->obj->short_descr, 
               price - (price * 5 / 100), price * 5 / 100);
            if (market->count == 0)
            {
               STRFREE(market->name);
               extract_obj(market->obj);
               UNLINK(market, first_market, last_market, next, prev);
               DISPOSE(market);
            }
            else
            {
               market->scount = market->count;
            }
         }
      }
      if (num == 0)
      {
         send_to_char("None of your goods have sold yet.\n\r", ch);
         return;
      }
      else
      {
         save_market_data();
         return;
      }
   }
   if (!str_cmp(arg1, "edit"))
   {
      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);  
      for (market = first_market; market; market = market->next)
      {
         num++;
         if (market->mpid == atoi(arg2))
            break;
      }
      if (!market)
      {
         send_to_char("That item number does not exist.\n\r", ch);
         return;
      } 
      if (market->pid != ch->pcdata->pid)
      {
         send_to_char("This object does not belong to you.\n\r", ch);
         return;
      }
      if (!str_cmp(arg3, "cost"))
      {
         if (atoi(argument) < market->obj->cost)
         {
            send_to_char("Asking price cannot be lower than the cost of the object.\n\r", ch);
            return;
         }   
         market->cost = atoi(argument);
         ch_printf(ch, "Changed the cost on item %d to %d\n\r", atoi(arg2), atoi(argument));
         save_market_data();
         return;
      }
      if (!str_cmp(arg3, "list"))
      {
         if (!str_cmp(argument, "no"))
         {
            if (market->name)
               STRFREE(market->name);
            market->name = STRALLOC("Unlisted");
         }
         else if (!str_cmp(argument, "yes"))
         {
            if (market->name)
               STRFREE(market->name);
            market->name = STRALLOC(ch->name);
         }
         else
         {
            do_market(ch, "");
            return;
         }
         send_to_char("Changed your name listing to what you provided.\n\r", ch);
         save_market_data();
         return;
      }
      do_market(ch, "");
      return;
   }
   if (!str_cmp(arg1, "view"))
   {
      for (market = first_market; market; market = market->next)
      {
         num++;
         if (market->mpid == atoi(argument))
            break;
      }
      if (!market)
      {
         send_to_char("That item number does not exist.\n\r", ch);
         return;
      }
      code_identify(ch, market->obj, NULL, 1, NULL);
      return;
   }
   if (!str_cmp(arg1, "buy"))
   {
      argument = one_argument(argument, arg2);
      if (atoi(arg2) <= 0)
      {
         send_to_char("You must specify the number of the item you wish to purchase.\n\r", ch);
         return;
      }
      for (market = first_market; market; market = market->next)
      {
         if (market->mpid == atoi(arg2))
            break;
      }
      if (!market)
      {
         send_to_char("That item number does not exist.\n\r", ch);
         return;
      }
      if (atoi(argument) > 0)
      {
         if (market->count < atoi(argument))
         {
            ch_printf(ch, "There is only %d of those for sale.\n\r", market->count);
            return;
         }
         if (market->count == atoi(argument))
            count = -1;
         else
            count = atoi(argument);
      }
      else if (!str_cmp(argument, "all"))
      {
         count = -1;
      }
      else
      {
         send_to_char("Invalid option.\n\r", ch);
         return;
      }
      if (count == -1)
         price = market->count * market->cost;
      else
         price = market->cost * count;
      if (price > ch->gold)
      {
         send_to_char("You do not have enough gold to pay for that.\n\r", ch);
         return;
      }
      if (count == -1)
      {  
         if (get_ch_carry_weight(ch) + (get_obj_weight(market->obj) * market->count) > can_carry_w(ch))
         {
            send_to_char("You cannot carry that much weight.\n\r", ch);
            return;
         }
         if (get_ch_carry_number(ch) + market->count > can_carry_n(ch))
         {
            send_to_char("You cannot carry that many items.\n\r", ch);
            return;
         }
         obj = clone_object(market->obj);
         count = market->count;
         obj->count = market->count;
         market->count = 0;
         obj_to_char(obj, ch);
      }
      else
      {
         if (get_ch_carry_weight(ch) + (get_obj_weight(market->obj) * count) > can_carry_w(ch))
         {
            send_to_char("You cannot carry that much weight.\n\r", ch);
            return;
         }
         if (get_ch_carry_number(ch) + count > can_carry_n(ch))
         {
            send_to_char("You cannot carry that many items.\n\r", ch);
            return;
         }
         obj = clone_object(market->obj);
         obj->count = count;
         market->count -= count;
         obj_to_char(obj, ch);
      }
      ch->gold -= price;
      save_market_data();
      ch_printf(ch, "You have purchased %d of %s\n\r", count, obj->short_descr);
      return;
   }
      
   if (!str_cmp(arg1, "list"))
   {
      argument = one_argument(argument, arg2);
      if (!str_cmp(arg2, "name"))
      {
         ch_printf(ch, "&w&RNum      Item Name                       Cnt Cost       Asking     Itemtype        Seller\n\r----------------------------------------------------------------------------------------------\n\r", ch);
         for (market = first_market; market; market = market->next)
         {
            num++;
            if (market->count)
            {
               if (nifty_is_name(argument, market->obj->name))
               {
                  sprintf(buf2, "%d", market->mpid);
                  sprintf(buf, MXPFTAG("Command 'market view %d' desc='Click here to id this item'", "%d", "/Command") "%s", 
                     market->mpid, market->mpid, add_space(strlen(buf2), 6));
                  ch_printf(ch, "&c&w%s>  " MXPFTAG("PCommand 'market buy %d 1' desc='Click here to buy this item'", "&w&W%s", "/PCommand") 
                     "%s&w&W  %-2d  &w&c%-10d &w&C%-10d ", 
                     buf, market->mpid, market->obj->short_descr, add_wspace(strlen_color(market->obj->short_descr), 30),
                     market->count, market->obj->cost, market->cost);   
                  ch_printf(ch, MXPFTAG("Command 'market list itemtype %s' desc='Click here to view other items of this itemtype'", "&w&G%s", "/Command")
                     "%s  &w&O%-15s\n\r", item_type_name(market->obj), item_type_name(market->obj), 
                     add_wspace(strlen(item_type_name(market->obj)), 14), market->name);
                }
            }
         }
         return;
      }   
      if (!str_cmp(arg2, "mincost"))
      {
         ch_printf(ch, "&w&RNum      Item Name                       Cnt Cost       Asking     Itemtype        Seller\n\r----------------------------------------------------------------------------------------------\n\r", ch);
         for (market = first_market; market; market = market->next)
         {
            num++;
            if (market->count)
            {
               if (market->cost >= atoi(argument))
               {
                  sprintf(buf2, "%d", market->mpid);
                  sprintf(buf, MXPFTAG("Command 'market view %d' desc='Click here to id this item'", "%d", "/Command") "%s", 
                     market->mpid, market->mpid, add_space(strlen(buf2), 6));
                  ch_printf(ch, "&c&w%s>  " MXPFTAG("PCommand 'market buy %d 1' desc='Click here to buy this item'", "&w&W%s", "/PCommand") 
                     "%s&w&W  %-2d  &w&c%-10d &w&C%-10d ", 
                     buf, market->mpid, market->obj->short_descr, add_wspace(strlen_color(market->obj->short_descr), 30),
                     market->count, market->obj->cost, market->cost);   
                  ch_printf(ch, MXPFTAG("Command 'market list itemtype %s' desc='Click here to view other items of this itemtype'", "&w&G%s", "/Command")
                     "%s  &w&O%-15s\n\r", item_type_name(market->obj), item_type_name(market->obj), 
                     add_wspace(strlen(item_type_name(market->obj)), 14), market->name);
                }
            }
         }
         return;
      }   
      if (!str_cmp(arg2, "rangecost"))
      {
         argument = one_argument(argument, arg3);
         if (atoi(arg3) > atoi(argument))
         {
            send_to_char("The first value in the range has to be less than the second.\n\r", ch);
            return;
         }
         ch_printf(ch, "&w&RNum      Item Name                       Cnt Cost       Asking     Itemtype        Seller\n\r----------------------------------------------------------------------------------------------\n\r", ch);
         for (market = first_market; market; market = market->next)
         {
            num++;
            if (market->count)
            {
               if (market->cost >= atoi(arg3) && market->cost <= atoi(argument))
               {
                  sprintf(buf2, "%d", market->mpid);
                  sprintf(buf, MXPFTAG("Command 'market view %d' desc='Click here to id this item'", "%d", "/Command") "%s", 
                     market->mpid, market->mpid, add_space(strlen(buf2), 6));
                  ch_printf(ch, "&c&w%s>  " MXPFTAG("PCommand 'market buy %d 1' desc='Click here to buy this item'", "&w&W%s", "/PCommand") 
                     "%s&w&W  %-2d  &w&c%-10d &w&C%-10d ", 
                     buf, market->mpid, market->obj->short_descr, add_wspace(strlen_color(market->obj->short_descr), 30),
                     market->count, market->obj->cost, market->cost);   
                  ch_printf(ch, MXPFTAG("Command 'market list itemtype %s' desc='Click here to view other items of this itemtype'", "&w&G%s", "/Command")
                     "%s  &w&O%-15s\n\r", item_type_name(market->obj), item_type_name(market->obj), 
                     add_wspace(strlen(item_type_name(market->obj)), 14), market->name);
                }
            }
         }
         return;
      }
      if (!str_cmp(arg2, "maxcost"))
      {
         ch_printf(ch, "&w&RNum      Item Name                       Cnt Cost       Asking     Itemtype        Seller\n\r----------------------------------------------------------------------------------------------\n\r", ch);
         for (market = first_market; market; market = market->next)
         {
            num++;
            if (market->count)
            {
               if (market->cost <= atoi(argument))
               {
                  sprintf(buf2, "%d", market->mpid);
                  sprintf(buf, MXPFTAG("Command 'market view %d' desc='Click here to id this item'", "%d", "/Command") "%s", 
                     market->mpid, market->mpid, add_space(strlen(buf2), 6));
                  ch_printf(ch, "&c&w%s>  " MXPFTAG("PCommand 'market buy %d 1' desc='Click here to buy this item'", "&w&W%s", "/PCommand") 
                     "%s&w&W  %-2d  &w&c%-10d &w&C%-10d ", 
                     buf, market->mpid, market->obj->short_descr, add_wspace(strlen_color(market->obj->short_descr), 30),
                     market->count, market->obj->cost, market->cost);   
                  ch_printf(ch, MXPFTAG("Command 'market list itemtype %s' desc='Click here to view other items of this itemtype'", "&w&G%s", "/Command")
                     "%s  &w&O%-15s\n\r", item_type_name(market->obj), item_type_name(market->obj), 
                     add_wspace(strlen(item_type_name(market->obj)), 14), market->name);
                }
            }
         }
         return;
      }
      if (!str_cmp(arg2, "itemtype"))
      {
         int ivalue = get_otype(argument);
         if (ivalue < 1)
         {
            ch_printf(ch, "Unknown type: %s\n\r", argument);
            return;
         }
         ch_printf(ch, "&w&RNum      Item Name                       Cnt Cost       Asking     Itemtype        Seller\n\r----------------------------------------------------------------------------------------------\n\r", ch);
         for (market = first_market; market; market = market->next)
         {
            num++;
            if (market->count)
            {
               if (market->obj->item_type == ivalue)
               {
                  sprintf(buf2, "%d", market->mpid);
                  sprintf(buf, MXPFTAG("Command 'market view %d' desc='Click here to id this item'", "%d", "/Command") "%s", 
                     market->mpid, market->mpid, add_space(strlen(buf2), 6));
                  ch_printf(ch, "&c&w%s>  " MXPFTAG("PCommand 'market buy %d 1' desc='Click here to buy this item'", "&w&W%s", "/PCommand") 
                     "%s&w&W  %-2d  &w&c%-10d &w&C%-10d ", 
                     buf, market->mpid, market->obj->short_descr, add_wspace(strlen_color(market->obj->short_descr), 30),
                     market->count, market->obj->cost, market->cost);   
                  ch_printf(ch, MXPFTAG("Command 'market list itemtype %s' desc='Click here to view other items of this itemtype'", "&w&G%s", "/Command")
                     "%s  &w&O%-15s\n\r", item_type_name(market->obj), item_type_name(market->obj), 
                     add_wspace(strlen(item_type_name(market->obj)), 14), market->name);
                }
            }
         }
         return;
      }
      if (!str_cmp(arg2, "all"))
      {
         ch_printf(ch, "&w&RNum      Item Name                       Cnt Cost       Asking     Itemtype        Seller\n\r---------------------------------------------------------------------------------------------\n\r", ch);
         for (market = first_market; market; market = market->next)
         {
            num++;
            if (market->count)
            {
               sprintf(buf2, "%d", market->mpid);
               sprintf(buf, MXPFTAG("Command 'market view %d' desc='Click here to id this item'", "%d", "/Command") "%s", 
                  market->mpid, market->mpid, add_space(strlen(buf2), 6));
               ch_printf(ch, "&c&w%s>  " MXPFTAG("PCommand 'market buy %d 1' desc='Click here to buy this item'", "&w&W%s", "/PCommand") 
                  "%s&w&W  %-2d  &w&c%-10d &w&C%-10d ", 
                  buf, market->mpid, market->obj->short_descr, add_wspace(strlen_color(market->obj->short_descr), 30),
                  market->count, market->obj->cost, market->cost);   
               ch_printf(ch, MXPFTAG("Command 'market list itemtype %s' desc='Click here to view other items of this itemtype'", "&w&G%s", "/Command")
                  "%s  &w&O%-15s\n\r", item_type_name(market->obj), item_type_name(market->obj), 
                  add_wspace(strlen(item_type_name(market->obj)), 14), market->name);
            }
         }
         return;
      }
      send_to_char("Your options are: all name maxcost mincost rangecost itemtype\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "sell"))
   {
      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);  
      argument = one_argument(argument, arg4);  
      if ((obj = get_obj_carry(ch, arg2)) == NULL)
      {
         send_to_char("You don't have that item in your inventory.\n\r", ch);
         return;
      }
      if (IS_OBJ_STAT(obj, ITEM_NOGIVE) || IS_OBJ_STAT(obj, ITEM_NODROP) || IS_OBJ_STAT(obj, ITEM_ARTIFACT))
      {
         send_to_char("You cannot attempt to sell an item with a nogive/nodrop/artifact flag on it.\n\r", ch);
         return;
      }
      if (obj->first_content)
      {
         send_to_char("You cannot sell a container with something in it.\n\r", ch);
         return;
      }
      if (atoi(arg3) <= 0 && str_cmp(arg3, "all"))
      {
         send_to_char("You need to specify a number of the item to sell or type all.\n\r", ch);
         return;
      }
      if (arg3[0] == '\0')
      {
         count = 1;
      }
      else
      {
         if (atoi(arg3) > 0)
         {
            if (obj->count < atoi(arg3))
            {
               ch_printf(ch, "You only have %d of those.  Turn off combine in config to see how they are matched.\n\r", obj->count);
               return;
            }
            if (obj->count == atoi(arg3))
               count = -1;
            else
               count = atoi(arg3);
         }
         else if (!str_cmp(arg3, "all"))
         {
            count = -1;
         }
         else
         {
            send_to_char("Invalid option.\n\r", ch);
            return;
         }
      }
      if (arg4[0] == '\0')
      {
         price = obj->cost;
      }
      else
      {
         if (!str_cmp(arg4, "lowest"))
         {
            price = UMAX(1, obj->cost/10);
         }
         else if (atoi(arg4) < UMAX(1, obj->cost/10))
         {
            send_to_char("Asking price cannot be lower than 1/10 the cost of the object.\n\r", ch);
            return;
         }
         else
         {
           price = atoi(arg4);
         }
      }
      if (!str_cmp(argument, "yes"))
         sellname = 1;
      CREATE(market, MARKET_DATA, 1);
      market->mpid = ++start_marketpid;
      market->cost = price;
      if (count == -1)
         market->count = obj->count;
      else
         market->count = count;
      market->pid = ch->pcdata->pid;
      if (sellname)
         market->name = STRALLOC(ch->name);
      else
         market->name = STRALLOC("Unlisted");
      if (count == -1)
      {  
         obj_from_char(obj);
         obj->count = 1;
         market->obj = obj;
      }
      else
      {
         rest = separate_obj(obj);
         obj_from_char(obj);
         obj->count = 1;
         market->obj = obj;
         rest->count = rest->count - market->count + 1;
      }
      market->scount = market->count;
      LINK(market, first_market, last_market, next, prev);
      save_market_data();
      ch_printf(ch, "You have put up %d of %s for sale.\n\r", market->count, market->obj->short_descr);
      return;
   }
   do_market(ch, "");
   return;
}     

/* Make objects in rooms that are nofloor fall - Scryn 1/23/96 */
void obj_fall(OBJ_DATA * obj, bool through)
{
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *to_room;
   static int fall_count;
   char buf[MSL];
   static bool is_falling; /* Stop loops from the call to obj_to_room()  -- Altrag */

   if (!obj->in_room || is_falling)
      return;

   if (fall_count > 30)
   {
      bug("object falling in loop more than 30 times", 0);
      extract_obj(obj);
      fall_count = 0;
      return;
   }

   if (xIS_SET(obj->in_room->room_flags, ROOM_NOFLOOR) && CAN_GO(obj, DIR_DOWN) && !IS_OBJ_STAT(obj, ITEM_MAGIC))
   {

      pexit = get_exit(obj->in_room, DIR_DOWN);
      to_room = pexit->to_room;

      if (through)
         fall_count++;
      else
         fall_count = 0;

      if (obj->in_room == to_room)
      {
         sprintf(buf, "Object falling into same room, room %d", to_room->vnum);
         bug(buf, 0);
         extract_obj(obj);
         return;
      }

      if (obj->in_room->first_person)
      {
         act(AT_PLAIN, "$p falls far below...", obj->in_room->first_person, obj, NULL, TO_ROOM);
         act(AT_PLAIN, "$p falls far below...", obj->in_room->first_person, obj, NULL, TO_CHAR);
      }
      obj_from_room(obj);
      is_falling = TRUE;
      obj = obj_to_room(obj, to_room, obj->in_room->first_person);
      is_falling = FALSE;

      if (obj->in_room->first_person)
      {
         act(AT_PLAIN, "$p falls from above...", obj->in_room->first_person, obj, NULL, TO_ROOM);
         act(AT_PLAIN, "$p falls from above...", obj->in_room->first_person, obj, NULL, TO_CHAR);
      }

      if (!xIS_SET(obj->in_room->room_flags, ROOM_NOFLOOR) && through)
      {
/*		int dam = (int)9.81*sqrt(fall_count*2/9.81)*obj->weight/2;
*/ int dam = fall_count * obj->weight / 2;

         /* Damage players */
         if (obj->in_room->first_person && number_percent() > 15)
         {
            CHAR_DATA *rch;
            CHAR_DATA *vch = NULL;
            int chcnt = 0;

            for (rch = obj->in_room->first_person; rch; rch = rch->next_in_room, chcnt++)
               if (number_range(0, chcnt) == 0)
                  vch = rch;
            act(AT_WHITE, "$p falls on $n!", vch, obj, NULL, TO_ROOM);
            act(AT_WHITE, "$p falls on you!", vch, obj, NULL, TO_CHAR);
            damage(vch, vch, dam, TYPE_UNDEFINED, 0, -1);
         }
      }
      obj_fall(obj, TRUE);
   }
   return;
}

/* Scryn, by request of Darkur, 12/04/98 */
/* Reworked recursive_note_find to fix crash bug when the note was left 
 * blank.  7/6/98 -- Shaddai
 */

void do_findnote(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("You must specify at least one keyword.\n\r", ch);
      return;
   }

   obj = recursive_note_find(ch->first_carrying, argument);

   if (obj)
   {
      if (obj->in_obj)
      {
         obj_from_obj(obj);
         obj = obj_to_char(obj, ch);
      }
      wear_obj(ch, obj, TRUE, -1);
   }
   else
      send_to_char("Note not found.\n\r", ch);
   return;
}

OBJ_DATA *recursive_note_find(OBJ_DATA * obj, char *argument)
{
   OBJ_DATA *returned_obj;
   bool match = TRUE;
   char *argcopy;
   char *subject;

   char arg[MIL];
   char subj[MSL];

   if (!obj)
      return NULL;

   switch (obj->item_type)
   {
      case ITEM_PAPER:

         if ((subject = get_extra_descr("_subject_", obj->first_extradesc)) == NULL)
            break;
         sprintf(subj, "%s", strlower(subject));
         subject = strlower(subj);

         argcopy = argument;

         while (match)
         {
            argcopy = one_argument(argcopy, arg);

            if (arg[0] == '\0')
               break;

            if (!strstr(subject, arg))
               match = FALSE;
         }


         if (match)
            return obj;
         break;

      case ITEM_CONTAINER:
      case ITEM_CORPSE_NPC:
      case ITEM_CORPSE_PC:
         if (obj->first_content)
         {
            returned_obj = recursive_note_find(obj->first_content, argument);
            if (returned_obj)
               return returned_obj;
         }
         break;

      default:
         break;
   }

   return recursive_note_find(obj->next_content, argument);
}

/* Junk command installed by Shai'tan 7-25-99
   Code courtesy of Stu, from the mailing list. Allows player to
   destroy item in inventory. */

void do_junk(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj, *obj_next;
   char arg[MIL];
   char *chk;
   bool found = FALSE;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Junk what?\n\r", ch);
      return;
   }
   chk = arg;
   
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
   {
      send_to_char("You cannot do that.\n\r", ch);
      return;
   }

   found = FALSE;
   for (obj = ch->first_carrying; obj; obj = obj_next)
   {
      obj_next = obj->next_content;

      if ((nifty_is_name(chk, obj->name)) && can_see_obj(ch, obj) && obj->wear_loc == WEAR_NONE)
      {
         found = TRUE;
         break;
      }
   }
   if (found == TRUE)
   {
      separate_obj(obj);
      obj_from_char(obj);
      extract_obj(obj);
      act(AT_ACTION, "$n junks $p.", ch, obj, NULL, TO_ROOM);
      act(AT_ACTION, "You junk $p.", ch, obj, NULL, TO_CHAR);
   }
   return;
}

int get_used_imbueslots(OBJ_DATA *obj)
{
   int cnt = 0;
   IMBUE_DATA *imbue;
   
   for (imbue = obj->first_imbue; imbue; imbue = imbue->next)
   {
      if (imbue->plevel == -1)
         continue;
      if (imbue->plevel == 0)
         cnt++;
      else
         cnt += imbue->plevel;
   }   
   return cnt;
}

extern int top_affect;
void save_sysdata args((SYSTEM_DATA sys));


//1000 - Damage  1001 - Durability  1002 - TohitBash  1003 - TohitStab   1004 - TohitSlash
//1005 - Weight  1006 - Shieldlag   1007 - Blocking % 1008 - Proj Range  1009 - Parry Chance 1010 - Stop Parry
//1011 - SpellSN 1012 - SpellStr    1013 - Unbeakable 1014 - Nodisarm    1015 - Sanctified 1016 - Change Size
//1017 - Saves
void set_otheraff_gem(int type, int value, OBJ_DATA *obj, int gemnum)
{
   AFFECT_DATA *paf;
   
   if (type == 1000)
   {
      if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_PROJECTILE || obj->item_type == ITEM_MISSILE_WEAPON)
      {
         obj->value[1] += value;
         obj->value[2] += value;
      }
   }
   if (type == 1001)
   {
      if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON)   
      {
         obj->value[10] = UMIN(25, obj->value[10]+value);
      }
      if (obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         obj->value[4] = UMIN(25, obj->value[4]+value);
      }
      if (obj->item_type == ITEM_ARMOR && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         obj->value[1] += obj->value[1] * value / 50;
      }
   }
   if (type == 1002)
   {
      if (obj->item_type == ITEM_WEAPON)   
      {
         obj->value[7] += value;
      }
      if (obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         obj->value[0] += value;
      }   
   }
   if (type == 1003)
   {
      if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_PROJECTILE || obj->item_type == ITEM_MISSILE_WEAPON)   
      {
         obj->value[9] += value;
      }
      if (obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         obj->value[2] += value;
      }   
   }
   if (type == 1004)
   {
      if (obj->item_type == ITEM_WEAPON)   
      {
         obj->value[8] += value;
      }
      if (obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         obj->value[1] += value;
      }   
   }
   if (type == 1005)
      obj->weight = UMAX(.01, obj->weight-=abs(value));
   if (type == 1006)
   {
      if (obj->item_type == ITEM_ARMOR && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
         obj->value[3] = UMAX(0, obj->value[3]-abs(value));
   }
   if (type == 1007)
   {
      if (obj->item_type == ITEM_ARMOR && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
         obj->value[2] += value;
   }
   if (type == 1008)
   {
      if (obj->item_type == ITEM_MISSILE_WEAPON)
         obj->value[4] += value;
   }
   if (type == 1009)
   {
      if (obj->item_type == ITEM_WEAPON)
         obj->value[12] += value;
   }
   if (type == 1010)
   {
      if (obj->item_type == ITEM_WEAPON)
         obj->value[13] += value;
   }
   if (type == 1011)
   {
      if (obj->item_type == ITEM_WEAPON)
         obj->value[4] = value;
   }
   if (type == 1012)
   {
      if (obj->item_type == ITEM_WEAPON)
         obj->value[5] = value;
   }
   if (type == 1013)
   {
      xSET_BIT(obj->extra_flags, ITEM_NOBREAK);
   }
   if (type == 1014)
   {
      xSET_BIT(obj->extra_flags, ITEM_NODISARM);
   }
   if (type == 1015)
   {
      xSET_BIT(obj->extra_flags, ITEM_SANCTIFIED);
   }
   if (type == 1016)
   {
      if (obj->item_type == ITEM_WEAPON)
         obj->value[3] = URANGE(1, obj->value[3]+value, 12);
   }
   if (type == 1017)
   {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = 20;
      paf->modifier = value;
      paf->gemnum = gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, obj->first_affect, obj->last_affect, next, prev);
      ++top_affect;
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = 21;
      paf->modifier = value;
      paf->gemnum = gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, obj->first_affect, obj->last_affect, next, prev);
      ++top_affect;
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = 22;
      paf->modifier = value;
      paf->gemnum = gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, obj->first_affect, obj->last_affect, next, prev);
      ++top_affect;
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = 23;
      paf->modifier = value;
      paf->gemnum = gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, obj->first_affect, obj->last_affect, next, prev);
      ++top_affect;
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = 24;
      paf->modifier = value;
      paf->gemnum = gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, obj->first_affect, obj->last_affect, next, prev);
      ++top_affect;
   }
}


/* Connect pieces of an ITEM -- Originally from ACK!  *
 * Modified for Smaug by Zarius 5/19/2000             *
 *						      *
 * Zarius' connect-item code altered for	      *
 * Rafermand mini-nodes by Skan 1/26/02		      *
 * 						      *
 * do_connect altered... changed to do_setgem         */
//Changed altogether for something else -- Xerves
 
void do_setgem(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *first_obj;
   OBJ_DATA *second_obj;
   IMBUE_DATA *imbue;
   AFFECT_DATA *paf;
   int usedslots;

   char arg1[MSL], arg2[MAX_STRING_LENGTH];

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Syntax: setgem <gem> <item receiving gem>.\n\r", ch);
      return;
   }

   if ((first_obj = get_obj_carry(ch, arg1)) == NULL)
   {
      send_to_char("You must be holding both parts!!\n\r", ch);
      return;
   }
   separate_obj(first_obj);

   if ((second_obj = get_obj_carry(ch, arg2)) == NULL)
   {
      send_to_char("You mus be holding both parts!!\n\r", ch);
      return;
   }
   separate_obj(second_obj);

   if (first_obj->item_type != ITEM_TGEM)
   {
      send_to_char("The first item is not a gem.\n\r", ch);
      return;
   }
   
   if (first_obj->value[12] == -1)
      usedslots = 0;
   else if (first_obj->value[12] == 0)
      usedslots = 1;
   else
      usedslots = first_obj->value[12];
      
   if (get_used_imbueslots(second_obj)+usedslots > second_obj->imbueslots)
   {
      send_to_char("The item is already full and cannot be imbued anymore.\n\r", ch);
      return;
   }
   CREATE(imbue, IMBUE_DATA, 1);
   LINK(imbue, second_obj->first_imbue, second_obj->last_imbue, next, prev);
   imbue->type = first_obj->value[0];
   imbue->sworth = first_obj->value[1];
   imbue->lowvalue = first_obj->value[2];
   imbue->highvalue = first_obj->value[3];
   imbue->value = number_range(imbue->lowvalue, imbue->highvalue);
   imbue->type2 = first_obj->value[4];
   imbue->sworth2 = first_obj->value[5];
   imbue->lowvalue2 = first_obj->value[6];
   imbue->highvalue2 = first_obj->value[7];
   imbue->value2 = number_range(imbue->lowvalue2, imbue->highvalue2);
   imbue->type3 = first_obj->value[8];
   imbue->sworth3 = first_obj->value[9];
   imbue->lowvalue3 = first_obj->value[10];
   imbue->highvalue3 = first_obj->value[11];
   imbue->value3 = number_range(imbue->lowvalue3, imbue->highvalue3);
   imbue->plevel = first_obj->value[12];
   imbue->gemnum = ++sysdata.top_gem_num;
   save_sysdata(sysdata);
   second_obj->sworthrestrict += imbue->sworth;
   second_obj->sworthrestrict += imbue->sworth2;
   second_obj->sworthrestrict += imbue->sworth3;
   
   if (imbue->type > 0 && imbue->type < 1000)
   {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = imbue->type;
      paf->modifier = imbue->value;
      paf->gemnum = imbue->gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, second_obj->first_affect, second_obj->last_affect, next, prev);
      ++top_affect;
   }
   if (imbue->type >= 1000)
   {
      set_otheraff_gem(imbue->type, imbue->value, second_obj, imbue->gemnum);
   }
   if (imbue->type2 > 0 && imbue->type2 < 1000)
   {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = imbue->type2;
      paf->modifier = imbue->value2;
      paf->gemnum = imbue->gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, second_obj->first_affect, second_obj->last_affect, next, prev);
      ++top_affect;
   }
   if (imbue->type2 >= 1000)
   {
      set_otheraff_gem(imbue->type2, imbue->value2, second_obj, imbue->gemnum);
   }
   if (imbue->type3 > 0 && imbue->type3 < 1000)
   {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = imbue->type3;
      paf->modifier = imbue->value3;
      paf->gemnum = imbue->gemnum;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, second_obj->first_affect, second_obj->last_affect, next, prev);
      ++top_affect;
   }
   if (imbue->type3 >= 1000)
   {
      set_otheraff_gem(imbue->type3, imbue->value3, second_obj, imbue->gemnum);
   }
      
   act(AT_ACTION, "$n sets $p in $P.", ch, first_obj, second_obj, TO_ROOM);
   act(AT_ACTION, "You set $p in $P.", ch, first_obj, second_obj, TO_CHAR);   
   extract_obj(first_obj);
   return;
}
/*Start Alchemy Section - Corellon - 7/15/02*/
 int get_alch_success(CHAR_DATA *ch, OBJ_DATA *obj)
 {
        return URANGE(30, (30 + 2*get_curr_int(ch) + get_curr_wis(ch) + obj->value[0] + URANGE(-6, get_curr_lck(ch)-14, 6)), 90);
 }
 void do_mixpotion(CHAR_DATA *ch, char *argument)
 {
 	char		arg[MIL];
 	char		arg2[MIL];
 	char		arg3[MIL];
 	char		arg4[MIL];
 	char		buf1[MSL];
 	char		buf2[MSL];
 	char		buf3[MSL];
 	OBJ_DATA	*obj;
 	OBJ_DATA	*obj2;
 	OBJ_DATA	*obj3;
 	OBJ_DATA 	*obj4;
 	int 		mod;
 	int			perm;
 	
 	argument = one_argument(argument,arg);
 	argument = one_argument(argument,arg2);
 	argument = one_argument(argument,arg3);
 	argument = one_argument(argument,arg4);
 	
 	if (IS_NPC(ch))
 	{
 	   send_to_char("Not for NPCs.\n\r", ch);
 	   return;
 	}
 	if(arg[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0')
 	{
 		send_to_char("Usage: mixpotion <power reagant> <affecting reagant> <flask> <mortar>\n\r",ch);
 		return;
 	}
 	if((obj = get_obj_carry(ch,arg)) == NULL)
 	{
 		send_to_char("You don't have the specified power reagant.\n\r",ch);
 		send_to_char("Usage: mixpotion <power reagant> <affecting reagant> <flask> <mortar>\n\r",ch);
 		return;
 	}
 	if((obj2 = get_obj_carry(ch,arg2)) == NULL)
 	{
 		send_to_char("You don't have the specified affect reagant.\n\r",ch);
 		send_to_char("Usage: mixpotion <power reagant> <affecting reagant> <flask> <mortar>\n\r",ch);
 		return;
 	}
 	if((obj3 = get_obj_carry(ch,arg3)) == NULL)
 	{
 		send_to_char("You don't have that container.\n\r",ch);
 		send_to_char("Usage: mixpotion <power reagant> <affecting reagant> <flask> <mortar>\n\r",ch);
 		return;
 	}
 	if((obj4 = get_obj_carry(ch,arg4)) == NULL)
 	{
 		send_to_char("You don't have that mortar and pestle\n\r",ch);
 		send_to_char("Usage: mixpotion <power reagant> <affecting reagant> <flask> <mortar>\n\r",ch);
 		return;
 	}
 	if( obj3->pIndexData->vnum != OBJ_VNUM_FLASK_BREWING || obj3->value[1] != -1)
 	{
 		send_to_char( "You must be holding an empty flask to mix a potion.\n\r", ch );
 		return;
 	}
 	
 	if (!xIS_SET(obj->extra_flags,ITEM_POWREAG))
 	{
 		send_to_char( "That's not a proper power reagant!\n\r",ch);
 		return;
 	}
 	if (!xIS_SET(obj2->extra_flags,ITEM_AFFREAG))
 	{
 	    send_to_char( "That's not a proper reagant!\n\r", ch );
 	    return;
 	}
 	if(!xIS_SET(obj4->extra_flags,ITEM_MORTAR))
 	{
 		send_to_char("Not a mortar\n\r",ch);
 		return;
 	}
 	if(obj->value[5] >= 1008 && xIS_SET(obj2->extra_flags,ITEM_PERMREAG))
 	{
 		perm = TRUE;
 	}
 	else if((obj->value[5] >= 1008 && !xIS_SET(obj2->extra_flags,ITEM_PERMREAG)) || (obj->value[5] < 1008 && xIS_SET(obj2->extra_flags,ITEM_PERMREAG)))
 	{
 		perm = FALSE;
 	}
 	else 
 	{
 		perm = 2;
 	}
 	if(get_alch_success(ch,obj4) < number_range(1,100))
 	{
 		separate_obj(obj3);
 			/* create the potion */
 		mod = obj2->value[1]; // set spell - will be expanded later to account for adding in new skills
 		obj3->value[1] = mod;
 		if(perm==TRUE) //is it perm?
 		{
 			obj3->item_type = ITEM_TREASURE;
 			obj3->value[0] = obj2->value[0];
 			xTOGGLE_BIT(obj3->extra_flags,ITEM_MIXED);
 			STRFREE(obj3->description);
 			sprintf(buf2,"A flask labelled 'permanent' contains a strange mixed liquid");
 			obj3->description = STRALLOC(buf2);
 			STRFREE(obj3->short_descr);
 			sprintf(buf1, "permanent potion");
 			obj3->short_descr = STRALLOC( aoran(buf1));
 			STRFREE(obj3->name);
 			sprintf(buf3, "flask permanent");
 			obj3->name = STRALLOC(buf3);
 			separate_obj(obj2);
 			extract_obj(obj2);
 			separate_obj(obj);
 			extract_obj(obj);
 			return;
 		}
 		else if(perm==2)
 		{
 			obj3->value[5] = obj->value[5];
 			sprintf(buf1, "%s potion", skill_table[mod]->name);
 			STRFREE(obj3->short_descr);
 			obj3->short_descr = STRALLOC( aoran(buf1) );
 	
 			sprintf(buf2, "A flask labelled '%s' contains a strange, mixed liquid.", skill_table[mod]->name);
 			STRFREE(obj3->description);
 			obj3->description = STRALLOC(buf2);
 	
 			sprintf(buf3, "flask potion %s", skill_table[mod]->name);
 			STRFREE(obj3->name);
 			obj3->name = STRALLOC(buf3);
 		}
 		else
 		{
 			send_to_char("You are trying to mix a non-permanent and a permanent reagant. BAD PLAYER, BAD!!\n\r",ch);
 			return;
 		}
 	}
 	else
 	{
 		if(perm==FALSE)
 		{
 		        send_to_char("You are trying to mix a non-permanent and a permanent reagant. BAD PLAYER, BAD!!\n\r",ch);
 			return;
 		}
 		send_to_char("You failed.\n\r",ch);
 	}
 	/* remove the reagants from inventory */
 	separate_obj(obj2);
 	extract_obj(obj2);
 	separate_obj(obj);
 	extract_obj(obj);
 	return;
 
 }
 void do_enhance(CHAR_DATA *ch, char *argument)
 {
 	char		arg[MIL];
 	char		arg2[MIL];
 	char		arg3[MIL];
 	OBJ_DATA	*obj;
 	OBJ_DATA	*obj2;
 	OBJ_DATA  	*obj3;
 	AFFECT_DATA	*paf;
 	argument = one_argument(argument,arg);
 	argument = one_argument(argument,arg2);
 	argument = one_argument(argument,arg3);
 	
 	if (IS_NPC(ch))
 	{
 	   send_to_char("Not for NPCs.\n\r", ch);
 	   return;
 	}
 		
 	if((obj = get_obj_carry(ch,arg)) == NULL)
 	{
 		send_to_char("Usage: modweapon <enhancer> <weapon> <mortar>\n\r",ch);
 		return;
 	}
 	if((obj2 = get_obj_carry(ch,arg2)) == NULL)
 	{
 		send_to_char("Usage: modweapon <enhancer> <weapon> <mortar> \n\r",ch);
 		return;
 	}
 	if((obj3 = get_obj_carry(ch,arg3)) == NULL)
 	{
 		send_to_char("Usage: modweapon <enhancer> <weapon> <mortar> \n\r",ch);
 			return;
 	}
 		
 	if(!xIS_SET(obj->extra_flags,ITEM_MIXED))
 	{
 		send_to_char("That is not potion with weapon enhancing power\n\r",ch);
 		return;
 	}
 	if(xIS_SET(obj2->extra_flags,ITEM_MAGIC))
 	{
 		send_to_char("This item is magical and cannot be enhanced.\n\r",ch);
 		return;
 	}
 	if(!xIS_SET(obj3->extra_flags,ITEM_MORTAR))
 	{
 		send_to_char("Usage: modweapon <enhancer> <weapon> <mortar> \n\r",ch);
 		return;
 	}
 	/* Add the appropriate affects */
 	if(get_alch_success(ch,obj3) < number_range(1,100))
 	{
 		
 		CREATE(paf, AFFECT_DATA, 1);
 		paf->type = -1;
 		paf->duration = -1;
 		if(obj->value[0] == 0 || obj->value[0] > 18) // If number is out of range, default to wearspell
 		{	
 			paf->location = APPLY_WEARSPELL;
 		}	
 		else
 		{
 			paf->location = obj->value[0];
 		}
 		paf->modifier = obj->value[1];
 		LINK(paf, obj2->first_affect, obj2->last_affect, next, prev);
 		send_to_char("You have enhanced your weapon.\n\r", ch);
 		xTOGGLE_BIT(obj2->extra_flags,ITEM_MAGIC);
 	}
 	else
 	{
 		send_to_char("You failed.\n\r",ch);
 	}
 	separate_obj(obj);		//remove items
 	extract_obj(obj);
 	return;
}
