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
 *			   Smaug banking support module                           *
 ****************************************************************************/
/***************************************************************************  
 *                          SMAUG Banking Support Code                     *
 ***************************************************************************
 *                                                                         *
 * This code may be used freely, as long as credit is given in the help    *
 * file. Thanks.                                                           *
 *								                           *
 *                                        -= Minas Ravenblood =-           *
 *                                 Implementor of The Apocalypse Theatre   *
 *                                      (email: krisco7@hotmail.com)       *
 *									                     *
 ***************************************************************************/

/* Modifications to original source by Samson */

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* You can add this or just put it in the do_bank code. I don't really know
   why I made a seperate function for this, but I did. If you do add it,
   don't forget to declare it - Minas */
/* Finds banker mobs in a room. Installed by Samson on unknown date */
/* NOTE: Smaug 1.02a Users - Your compiler probably died on this
   function - if so, remove the x in front of IS_SET and recompile */
CHAR_DATA *find_banker(CHAR_DATA * ch)
{
   CHAR_DATA *banker;

   for (banker = ch->in_room->first_person; banker; banker = banker->next_in_room)
      if (IN_SAME_ROOM(ch, banker) && IS_NPC(banker) && xIS_SET(banker->act, ACT_BANKER))
         break;

   return banker;
}

int get_bank_weight(CHAR_DATA *ch)
{
   int weight = 0;
   OBJ_DATA *obj;
   
   for (obj = ch->pcdata->first_bankobj; obj; obj = obj->next_content)
   {
      weight += get_obj_weight(obj);   
   }
   weight += ch->pcdata->balance/10000;
   return weight;
}

int get_townbank_weight(TOWN_DATA *town)
{
   int weight = 0;
   OBJ_DATA *obj;
   
   for (obj = town->first_bankobj; obj; obj = obj->next_content)
   {
      weight += get_obj_weight(obj);   
   }
   weight += town->balance/10000;
   return weight;
}

/* SMAUG Bank Support
 * Coded by Minas Ravenblood for The Apocalypse Theatre
 * (email: krisco7@hotmail.com)
 */
/* Installed by Samson on unknown date */
/* Deposit, withdraw, balance and transfer commands */
void do_deposit(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *banker;
   char arg1[MIL];
   char buf[MSL];
   int amount;
   OBJ_DATA *obj;

   if (!(banker = find_banker(ch)))
   {
      send_to_char("You're not in a bank!\n\r", ch);
      return;
   }

   if (IS_NPC(ch))
   {
      sprintf(buf, "Sorry, %s, we don't do business with mobs.", ch->short_descr);
      do_say(banker, buf);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  deposit <item name>\n\r", ch);
      send_to_char("Syntax:  deposit <gold amount>\n\r", ch);
      send_to_char("Syntax:  deposit all\n\r", ch);
      send_to_char("Syntax:  deposit town <item name>\n\r", ch);
      send_to_char("Syntax:  deposit town <gold amount>\n\r", ch);
      send_to_char("Syntax:  deposit town all\n\r", ch);
      if (sysdata.resetgame)
         send_to_char("Syntax:  deposit vault <gold ammount|all>\n\r", ch);
   }

   argument = one_argument(argument, arg1);

   if (arg1 == '\0')
   {
      sprintf(buf, "%s Do you wish to deposit gold or equipment?", ch->name);
      do_tell(banker, buf);
      return;
   }
   
   if (sysdata.resetgame)
   {
      if (!str_cmp(arg1, "vault"))
      {
         TOWN_DATA *town;
      
         if (!ch->pcdata->town)
         {
            send_to_char("You have to belong to a town to use this command.\n\r", ch);
            return;
         }
         town = find_town(ch->coord->x, ch->coord->y, ch->map);
         if (!town)
         {
            send_to_char("You have to be in the AOC of your town to use this.\n\r", ch);
            return;
         }
         if (ch->pcdata->town != town)
         {
            send_to_char("The town you are in is not your own.\n\r", ch);
            return;
         }
         if (str_cmp(argument, "all") && !is_number(argument))
         {
            sprintf(buf, "%s How much gold do you wish to deposit?", ch->name);
            do_tell(banker, buf);
            return;
         }

         if (!str_cmp(argument, "all"))
            amount = ch->gold;
         else
            amount = atoi(argument);

         if (amount > ch->gold)
         {
            sprintf(buf, "%s Sorry, but you don't have that much gold to deposit.", ch->name);
            do_tell(banker, buf);
            return;
         }

         if (amount <= 9)
         {
            sprintf(buf, "%s Oh, I see.. I didn't know i was doing business with a comedian.", ch->name);
            do_tell(banker, buf);
            return;
         }
         if (get_current_hold(town) + (amount/1000) > town->hold)
         {
            sprintf(buf, "%s You do not have enough town hold for that.", ch->name);
            do_tell(banker, buf);
            return;
         }   

         ch->gold -= amount;
         town->coins += amount/10;
         amount /= 10;
         sprintf(buf, "You deposit %d gold coin%s into the town vault.\n\r", amount, (amount != 1) ? "s" : "");
         set_char_color(AT_PLAIN, ch);
         send_to_char(buf, ch);
         sprintf(buf, "$n deposits %d gold coin%s into the town vault.\n\r", amount, (amount != 1) ? "s" : "");
         act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
         save_char_obj(ch);
         sprintf(buf, "_____DEPOSIT_VAULT_____ %d coins Deposited into %s's vault", amount, town->name);
         write_kingdom_logfile(town->kingdom, buf, KLOG_DEPOSIT);
         write_kingdom_file(town->kingdom);
         return;
      }
   }
   if (!str_cmp(arg1, "town"))
   {
      TOWN_DATA *town;
      
      if (!ch->pcdata->town)
      {
         send_to_char("You have to belong to a town to use this command.\n\r", ch);
         return;
      }
      town = find_town(ch->coord->x, ch->coord->y, ch->map);
      if (!town)
      {
         send_to_char("You have to be in the AOC of your town to use this.\n\r", ch);
         return;
      }
      if (ch->pcdata->town != town)
      {
         send_to_char("The town you are in is not your own.\n\r", ch);
         return;
      }
      if ((obj = get_obj_carry(ch, argument)) != NULL)
      {
         if (get_townbank_weight(town) + get_obj_weight(obj) > town->banksize)
         {
            sprintf(buf, "%s You do not have enough storage for that.", ch->name);
            do_tell(banker, buf);
            return;
         }
         separate_obj(obj);
         obj_from_char(obj);
         obj_to_townbank(obj, town);
         sprintf(buf, "%s It is done, your %s is safe now.", ch->name, obj->short_descr);
         do_tell(banker, buf);
         save_char_obj(ch);
         sprintf(buf, "_____DEPOSIT_____ %s Deposited into %s's bank", obj->short_descr, town->name);
         write_kingdom_logfile(town->kingdom, buf, KLOG_DEPOSIT);
         write_kingdom_file(town->kingdom);
         return;
      }

      if (str_cmp(argument, "all") && !is_number(argument))
      {
         sprintf(buf, "%s How much gold do you wish to deposit?", ch->name);
         do_tell(banker, buf);
         return;
      }

      if (!str_cmp(argument, "all"))
         amount = ch->gold;
      else
         amount = atoi(argument);

      if (amount > ch->gold)
      {
         sprintf(buf, "%s Sorry, but you don't have that much gold to deposit.", ch->name);
         do_tell(banker, buf);
         return;
      }

      if (amount <= 0)
      {
         sprintf(buf, "%s Oh, I see.. I didn't know i was doing business with a comedian.", ch->name);
         do_tell(banker, buf);
         return;
      }
   
      if (get_townbank_weight(town) + amount/10000 > town->banksize)
      {
         sprintf(buf, "%s You do not have enough storage for that.", ch->name);
         do_tell(banker, buf);
         return;
      }   

      ch->gold -= amount;
      town->balance += amount;
      sprintf(buf, "You deposit %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
      set_char_color(AT_PLAIN, ch);
      send_to_char(buf, ch);
      sprintf(buf, "$n deposits %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
      act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
      save_char_obj(ch);
      sprintf(buf, "_____DEPOSIT_____ %d coins Deposited into %s's bank", amount, town->name);
      write_kingdom_logfile(town->kingdom, buf, KLOG_DEPOSIT);
      write_kingdom_file(town->kingdom);
      return;
   }      
   if ((obj = get_obj_carry(ch, arg1)) != NULL)
   {
      if (get_bank_weight(ch) + get_obj_weight(obj) > ch->pcdata->banksize)
      {
         sprintf(buf, "%s You do not have enough storage for that.", ch->name);
         do_tell(banker, buf);
         return;
      }
      separate_obj(obj);
      obj_from_char(obj);
      obj_to_bank(obj, ch);
      sprintf(buf, "%s It is done, your %s is safe now.", ch->name, obj->short_descr);
      do_tell(banker, buf);
      save_char_obj(ch);
      return;
   }

   if (str_cmp(arg1, "all") && !is_number(arg1))
   {
      sprintf(buf, "%s How much gold do you wish to deposit?", ch->name);
      do_tell(banker, buf);
      return;
   }

   if (!str_cmp(arg1, "all"))
      amount = ch->gold;
   else
      amount = atoi(arg1);

   if (amount > ch->gold)
   {
      sprintf(buf, "%s Sorry, but you don't have that much gold to deposit.", ch->name);
      do_tell(banker, buf);
      return;
   }

   if (amount <= 0)
   {
      sprintf(buf, "%s Oh, I see.. I didn't know i was doing business with a comedian.", ch->name);
      do_tell(banker, buf);
      return;
   }
   
   if (get_bank_weight(ch) + amount/10000 > ch->pcdata->banksize)
   {
      sprintf(buf, "%s You do not have enough storage for that.", ch->name);
      do_tell(banker, buf);
      return;
   }   

   ch->gold -= amount;
   ch->pcdata->balance += amount;
   sprintf(buf, "You deposit %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
   set_char_color(AT_PLAIN, ch);
   send_to_char(buf, ch);
   sprintf(buf, "$n deposits %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
   act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
   save_char_obj(ch);
   return;
}

void separate_obj_bank(OBJ_DATA * obj, CHAR_DATA *ch)
{
   OBJ_DATA *rest;

   if (obj->count <= 1)
      return;
   rest = clone_object(obj);
   --obj->pIndexData->count; /* since clone_object() ups this value */
   --numobjsloaded;
   rest->count = obj->count - 1;
   obj->count = 1;

   LINK(rest, ch->pcdata->first_bankobj, ch->pcdata->last_bankobj, next_content, prev_content);
   rest->carried_by = NULL;
   rest->possessed_by = NULL;
   rest->in_room = NULL;
   rest->in_obj = NULL;
   return;
}
void separate_obj_townbank(OBJ_DATA * obj, TOWN_DATA *town)
{
   OBJ_DATA *rest;

   if (obj->count <= 1)
      return;
   rest = clone_object(obj);
   --obj->pIndexData->count; /* since clone_object() ups this value */
   --numobjsloaded;
   rest->count = obj->count - 1;
   obj->count = 1;

   LINK(rest, town->first_bankobj, town->last_bankobj, next_content, prev_content);
   rest->carried_by = NULL;
   rest->possessed_by = NULL;
   rest->in_room = NULL;
   rest->in_obj = NULL;
   return;
}

/*
 * Find an obj in a list...going the other way			-Thoric
 */
OBJ_DATA *get_obj_list_rev_bank(CHAR_DATA * ch, char *argument, OBJ_DATA * list)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int number;
   int count;

   number = number_argument(argument, arg);
   count = 0;
   for (obj = list; obj; obj = obj->prev_content)
      if (nifty_is_name(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = list; obj; obj = obj->prev_content)
      if (nifty_is_name_prefix(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   return NULL;
}

void do_withdraw(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *banker;
   char arg1[MIL];
   char buf[MSL];
   int amount;
   OBJ_DATA *obj;

   if (!(banker = find_banker(ch)))
   {
      send_to_char("You're not in a bank!\n\r", ch);
      return;
   }

   if (IS_NPC(ch))
   {
      sprintf(buf, "Sorry, %s, we don't do business with mobs.", ch->short_descr);
      do_say(banker, buf);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  withdraw <item name>\n\r", ch);
      send_to_char("Syntax:  withdraw <gold amount>\n\r", ch);
      send_to_char("Syntax:  withdraw all\n\r", ch);
      send_to_char("Syntax:  withdraw list\n\r", ch);
      send_to_char("Syntax:  withdraw town <item name>\n\r", ch);
      send_to_char("Syntax:  withdraw town <gold amount>\n\r", ch);
      send_to_char("Syntax:  withdraw town all\n\r", ch);
      send_to_char("Syntax:  withdraw town list\n\r", ch);
      if (sysdata.resetgame)
         send_to_char("Syntax:  withdraw vault <gold ammount|all>\n\r", ch);
   }

   argument = one_argument(argument, arg1);

   if (arg1 == '\0')
   {
      sprintf(buf, "%s Do you wish to withdraw gold or equipment?", ch->name);
      do_tell(banker, buf);
      return;
   }
   if (sysdata.resetgame)
   {
      if (!str_cmp(arg1, "vault"))
      {
         TOWN_DATA *town;
         if (!ch->pcdata->town)
         {
            send_to_char("You have to belong to a town to use this command.\n\r", ch);
            return;
         }
         town = find_town(ch->coord->x, ch->coord->y, ch->map);
         if (!town)
         {
            send_to_char("You have to be in the AOC of your town to use this.\n\r", ch);
            return;
         }
         if (ch->pcdata->town != town)
         {
            send_to_char("The town you are in is not your own.\n\r", ch);
            return;
         }
         if (ch->pcdata->caste < town->minwithdraw || ch->pcdata->caste < kingdom_table[town->kingdom]->minwithdraw)
         {
            send_to_char("You do not have permission to withdraw from this town.\n\r", ch);
            return;
         }
         if (str_cmp(argument, "all") && !is_number(argument))
         {
            sprintf(buf, "%s How much gold do you wish to withdraw?", ch->name);
            do_tell(banker, buf);
            return;
         }

         if (!str_cmp(argument, "all"))
            amount = town->coins;
         else
            amount = atoi(argument);

         if (amount > town->coins)
         {
            sprintf(buf, "%s But you do not have that much gold in your town's vault!", ch->name);
            do_tell(banker, buf);
            return;
         }
         if (amount <= 0)
         {
            sprintf(buf, "%s Oh I see.. I didn't know i was doing business with a comedian.", ch->name);
            do_tell(banker, buf);
            return;
         }
         town->coins -= amount;
         ch->gold += amount*10;
         sprintf(buf, "You withdraw %d gold coin%s from the vault.\n\r", amount, (amount != 1) ? "s" : "");
         set_char_color(AT_PLAIN, ch);
         send_to_char(buf, ch);
         sprintf(buf, "$n withdraws %d gold coin%s from the vault.\n\r", amount, (amount != 1) ? "s" : "");
         act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
         save_char_obj(ch);
         write_kingdom_file(town->kingdom);
         sprintf(buf, "_____WITHDRAW_VAULT_____ %d coins withdrawed from %s's vault", amount, town->name);
         write_kingdom_logfile(town->kingdom, buf, KLOG_WITHDRAW);
         return;
      }
   }
   if (!str_cmp(arg1, "town"))
   {
      TOWN_DATA *town;
      
      if (!ch->pcdata->town)
      {
         send_to_char("You have to belong to a town to use this command.\n\r", ch);
         return;
      }
      town = find_town(ch->coord->x, ch->coord->y, ch->map);
      if (!town)
      {
         send_to_char("You have to be in the AOC of your town to use this.\n\r", ch);
         return;
      }
      if (ch->pcdata->town != town)
      {
         send_to_char("The town you are in is not your own.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "list"))
      {
         for (obj = town->first_bankobj; obj; obj = obj->next_content)
         {
            if (obj->count > 1)
               sprintf(buf, " (%d)", obj->count);
            else
               strcpy(buf, "");
            ch_printf(ch, "%s%s&w\n\r", obj->short_descr, buf);
         }
         return;
      }
      if (ch->pcdata->caste < town->minwithdraw || ch->pcdata->caste < kingdom_table[town->kingdom]->minwithdraw)
      {
         send_to_char("You do not have permission to withdraw from this town.\n\r", ch);
         return;
      }
      if ((obj = get_obj_list_rev_bank(ch, argument, town->last_bankobj)) != NULL)
      {
         if (get_ch_carry_number(ch) + (get_obj_number(obj) / obj->count) > can_carry_n(ch))
         {
            sprintf(buf, "%s Your hands are too full to take that.", ch->name);
            do_tell(banker, buf);
            return;
         }

         if (get_ch_carry_weight(ch) + (get_obj_weight(obj) / obj->count) > can_carry_w(ch))
         {
            sprintf(buf, "%s You cannot hope to carry that much weight.", ch->name);
            do_tell(banker, buf);
            return;
         }
         separate_obj_townbank(obj, town);
         townbank_to_char(obj, town, ch);
         obj_to_char(obj, ch);
         update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
         update_container(obj, ch->coord->x, ch->coord->y, ch->map, 0, 0, 0);
         sprintf(buf, "%s It is done, your %s is now in your own hands.", ch->name, obj->short_descr);
         do_tell(banker, buf);
         save_char_obj(ch);
         sprintf(buf, "_____WITHDRAW_____ %s Withdrawed from %s's bank", obj->short_descr, town->name);
         write_kingdom_logfile(town->kingdom, buf, KLOG_WITHDRAW);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (str_cmp(argument, "all") && !is_number(argument))
      {
         sprintf(buf, "%s How much gold do you wish to withdraw?", ch->name);
         do_tell(banker, buf);
         return;
      }

      if (!str_cmp(argument, "all"))
         amount = town->balance;
      else
         amount = atoi(argument);

      if (amount > town->balance)
      {
         sprintf(buf, "%s But you do not have that much gold in your town's account!", ch->name);
         do_tell(banker, buf);
         return;
      }
      if (amount <= 0)
      {
         sprintf(buf, "%s Oh I see.. I didn't know i was doing business with a comedian.", ch->name);
         do_tell(banker, buf);
         return;
      }
      town->balance -= amount;
      ch->gold += amount;
      sprintf(buf, "You withdraw %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
      set_char_color(AT_PLAIN, ch);
      send_to_char(buf, ch);
      sprintf(buf, "$n withdraws %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
      act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
      save_char_obj(ch);
      sprintf(buf, "_____WITHDRAW_____ %d coins withdrawed from %s's bank", amount, town->name);
      write_kingdom_logfile(town->kingdom, buf, KLOG_WITHDRAW);
      write_kingdom_file(town->kingdom);
      return;
   }      
   if ((obj = get_obj_list_rev_bank(ch, arg1, ch->pcdata->last_bankobj)) != NULL)
   {
      if (get_ch_carry_number(ch) + (get_obj_number(obj) / obj->count) > can_carry_n(ch))
      {
         sprintf(buf, "%s Your hands are too full to take that.", ch->name);
         do_tell(banker, buf);
         return;
      }

      if (get_ch_carry_weight(ch) + (get_obj_weight(obj) / obj->count) > can_carry_w(ch))
      {
         sprintf(buf, "%s You cannot hope to carry that much weight.", ch->name);
         do_tell(banker, buf);
         return;
      }
      separate_obj_bank(obj, ch);
      bank_to_char(obj, ch);
      obj_to_char(obj, ch);
      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
      update_container(obj, ch->coord->x, ch->coord->y, ch->map, 0, 0, 0);
      sprintf(buf, "%s It is done, your %s is now in your own hands.", ch->name, obj->short_descr);
      do_tell(banker, buf);
      save_char_obj(ch);
      return;
   }
   if (!str_cmp(arg1, "list"))
   {
      for (obj = ch->pcdata->first_bankobj; obj; obj = obj->next_content)
      {
         if (obj->count > 1)
            sprintf(buf, " (%d)", obj->count);
         else
            strcpy(buf, "");
         ch_printf(ch, "%s%s&w\n\r", obj->short_descr, buf);
      }
      return;
   }
   if (str_cmp(arg1, "all") && !is_number(arg1))
   {
      sprintf(buf, "%s How much gold do you wish to withdraw?", ch->name);
      do_tell(banker, buf);
      return;
   }

   if (!str_cmp(arg1, "all"))
      amount = ch->pcdata->balance;
   else
      amount = atoi(arg1);

   if (amount > ch->pcdata->balance)
   {
      sprintf(buf, "%s But you do not have that much gold in your account!", ch->name);
      do_tell(banker, buf);
      return;
   }
   if (amount <= 0)
   {
      sprintf(buf, "%s Oh I see.. I didn't know i was doing business with a comedian.", ch->name);
      do_tell(banker, buf);
      return;
   }
   ch->pcdata->balance -= amount;
   ch->gold += amount;
   sprintf(buf, "You withdraw %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
   set_char_color(AT_PLAIN, ch);
   send_to_char(buf, ch);
   sprintf(buf, "$n withdraws %d gold coin%s.\n\r", amount, (amount != 1) ? "s" : "");
   act(AT_PLAIN, buf, ch, NULL, NULL, TO_ROOM);
   save_char_obj(ch);
   return;
}

void do_balance(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *banker;
   char arg1[MIL];
   char buf[MSL];
   TOWN_DATA *town;

   if (!(banker = find_banker(ch)))
   {
      send_to_char("You're not in a bank!\n\r", ch);
      return;
   }

   if (IS_NPC(ch))
   {
      sprintf(buf, "%s Sorry, we don't do business with mobs.", ch->name);
      do_tell(banker, buf);
      return;
   }
   argument = one_argument(argument, arg1);

   if (!str_cmp(arg1, "buy"))
   {
      if (argument[0] == '\0')
      {
         sprintf(buf, "%s We are currently offering the following storage at the following price.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 1  > 300 units   - 10,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 2  > 500 units   - 20,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 3  > 750 units   - 50,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 4  > 1000 units  - 100,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 5  > 1500 units  - 200,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 6  > 2000 units  - 500,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 7  > 3000 units  - 1,000,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 8  > 4000 units  - 2,000,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 9  > 5000 units  - 5,000,000 gold.", ch->name);
         do_tell(banker, buf);
         sprintf(buf, "%s 10 > 10000 units - 10,000,000 gold.", ch->name);
         do_tell(banker, buf);
         return;
      }
      if (atoi(argument) == 1)
      {
         if (ch->gold < 10000)
         {
            sprintf(buf, "%s You will need 10,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 300;
         ch->gold -= 10000;
         sprintf(buf, "%s You now have 300 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 2)
      {
         if (ch->gold < 20000)
         {
            sprintf(buf, "%s You will need 20,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 500;
         ch->gold -= 20000;
         sprintf(buf, "%s You now have 500 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 3)
      {
         if (ch->gold < 50000)
         {
            sprintf(buf, "%s You will need 50,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 750;
         ch->gold -= 50000;
         sprintf(buf, "%s You now have 750 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 4)
      {
         if (ch->gold < 100000)
         {
            sprintf(buf, "%s You will need 100,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 1000;
         ch->gold -= 100000;
         sprintf(buf, "%s You now have 1000 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 5)
      {
         if (ch->gold < 200000)
         {
            sprintf(buf, "%s You will need 200,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 1500;
         ch->gold -= 200000;
         sprintf(buf, "%s You now have 1500 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 6)
      {
         if (ch->gold < 500000)
         {
            sprintf(buf, "%s You will need 500,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 2000;
         ch->gold -= 500000;
         sprintf(buf, "%s You now have 2000 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 7)
      {
         if (ch->gold < 1000000)
         {
            sprintf(buf, "%s You will need 1,000,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 3000;
         ch->gold -= 1000000;
         sprintf(buf, "%s You now have 3000 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 8)
      {
         if (ch->gold < 2000000)
         {
            sprintf(buf, "%s You will need 2,000,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 4000;
         ch->gold -= 2000000;
         sprintf(buf, "%s You now have 4000 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 9)
      {
         if (ch->gold < 5000000)
         {
            sprintf(buf, "%s You will need 5,000,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 5000;
         ch->gold -= 5000000;
         sprintf(buf, "%s You now have 5000 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else if (atoi(argument) == 10)
      {
         if (ch->gold < 10000000)
         {
            sprintf(buf, "%s You will need 10,000,000 gold to purchase that option.", ch->name);
            do_tell(banker, buf);
            return;
         }
         ch->pcdata->banksize = 10000;
         ch->gold -= 10000000;
         sprintf(buf, "%s You now have 10000 units of storage with the National Rafermand Bank.", ch->name);
         do_tell(banker, buf);
         return;
      }
      else
      {
         sprintf(buf, "%s That is not a viable option.", ch->name);
         do_tell(banker, buf);
         return;    
      }
   }  
   set_char_color(AT_PLAIN, ch);
   sprintf(buf, "You have %d gold coin%s in the bank.\n\r", ch->pcdata->balance, (ch->pcdata->balance == 1) ? "" : "s");
   send_to_char(buf, ch);
   sprintf(buf, "Your bank hold is %d out of %d.\n\r", get_bank_weight(ch), ch->pcdata->banksize);
   send_to_char(buf, ch);
   if (!ch->pcdata->town)
      return;
   town = find_town(ch->coord->x, ch->coord->y, ch->map);
   if (!town)
      return;
   if (ch->pcdata->town != town)
      return;
   sprintf(buf, "You have %d gold coin%s in the town bank.\n\r", town->balance, (town->balance == 1) ? "" : "s");
   send_to_char(buf, ch);
   sprintf(buf, "Your town bank hold is %d out of %d.\n\r", get_townbank_weight(town), town->banksize);
   send_to_char(buf, ch);
   return;
}

void do_trans(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *banker;
   CHAR_DATA *victim;
   char arg1[MIL];
   char arg2[MIL];

   char buf[MSL];
   int amount;

   if (!(banker = find_banker(ch)))
   {
      send_to_char("You're not in a bank!\n\r", ch);
      return;
   }

   if (IS_NPC(ch))
   {
      sprintf(buf, "Sorry, %s, we don't do business with mobs.", ch->short_descr);
      do_say(banker, buf);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  trans <gold amount|all> <player>\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1 == '\0' || arg2 == '\0')
   {
      sprintf(buf, "%s How much gold do you wish to send to who?", ch->name);
      do_tell(banker, buf);
      return;
   }
   if (str_cmp(arg1, "all") && !is_number(arg1))
   {
      sprintf(buf, "%s How much gold do you wish to send to who?", ch->name);
      do_tell(banker, buf);
      return;
   }

   if (!(victim = get_char_world(ch, arg2)))
   {
      sprintf(buf, "%s %s could not be located.", ch->name, capitalize(arg2));
      do_tell(banker, buf);
      return;
   }

   if (IS_NPC(victim))
   {
      sprintf(buf, "%s We do not do business with mobiles...", ch->name);
      do_tell(banker, buf);
      return;
   }

   if (!str_cmp(arg1, "all"))
      amount = ch->pcdata->balance;
   else
      amount = atoi(arg1);

   if (amount > ch->pcdata->balance)
   {
      sprintf(buf, "%s You are very generous, but you don't have that much gold!", ch->name);
      do_tell(banker, buf);
      return;
   }

   if (amount <= 0)
   {
      sprintf(buf, "%s Oh I see.. I didn't know I was doing business with a comedian.", ch->name);
      do_tell(banker, buf);
      return;
   }
   
   if (get_bank_weight(victim) + amount/10000 > victim->pcdata->banksize)
   {
      sprintf(buf, "%s Your target does not have enough space in his/her bank for that.", ch->name);
      do_tell(banker, buf);
      return;
   }

   ch->pcdata->balance -= amount;
   victim->pcdata->balance += amount;
   sprintf(buf, "You transfer %d gold coin%s to %s's bank account.\n\r", amount, (amount != 1) ? "s" : "", PERS_MAP(victim, ch));
   set_char_color(AT_GREEN, ch);
   send_to_char(buf, ch);
   sprintf(buf, "%s just transferred %d gold coin%s to your bank account.\n\r", PERS_MAP(ch, victim), amount, (amount != 1) ? "s" : "");
   set_char_color(AT_GREEN, victim);
   send_to_char(buf, victim);
   save_char_obj(ch);
   save_char_obj(victim);
   return;
}

/* End of new bank support */
