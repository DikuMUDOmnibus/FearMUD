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
 *			 Shop and repair shop module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"



/*
 * Local functions
 */

#define	CD	CHAR_DATA
CD *find_keeper args((CHAR_DATA * ch));
CD *find_fixer args((CHAR_DATA * ch));
int get_cost args((CHAR_DATA * ch, CHAR_DATA * keeper, OBJ_DATA * obj, int fBuy));
int get_repaircost args((CHAR_DATA * keeper, OBJ_DATA * obj));

#undef CD

/*
 * Shopping commands.
 */
CHAR_DATA *find_keeper(CHAR_DATA * ch)
{
   char buf[MSL];
   CHAR_DATA *keeper, *whof;
   SHOP_DATA *pShop;
   int speakswell;

   pShop = NULL;
   for (keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
      if (IS_NPC(keeper) && (pShop = keeper->pIndexData->pShop) != NULL)
         break;

   if (!pShop)
   {
      send_to_char("You can't do that here.\n\r", ch);
      return NULL;
   }


   /*
    * Disallow sales during battle
    */
   if ((whof = who_fighting(keeper)) != NULL)
   {
      if (whof == ch)
         send_to_char("I don't think that's a good idea...\n\r", ch);
      else
         do_say(keeper, "I'm too busy for that!");
      return NULL;
   }

   if (who_fighting(ch))
   {
      ch_printf(ch, "%s doesn't seem to want to get involved.\n\r", PERS(keeper, ch));
      return NULL;
   }

   /*
    * Check to see if show is open.
    * Supports closing times after midnight
    */
   if (pShop->open_hour > pShop->close_hour)
   {
      if (gethour() < pShop->open_hour && gethour() > pShop->close_hour)
      {
         do_say(keeper, "Sorry, come back later.");
         return NULL;
      }
   }
   else
   {
      if (gethour() < pShop->open_hour)
      {
         do_say(keeper, "Sorry, come back later.");
         return NULL;
      }
      if (gethour() > pShop->close_hour)
      {
         do_say(keeper, "Sorry, come back tomorrow.");
         return NULL;
      }
   }

   if (keeper->position == POS_SLEEPING)
   {
      send_to_char("While they're asleep?\n\r", ch);
      return NULL;
   }

   if (keeper->position < POS_SLEEPING)
   {
      send_to_char("I don't think they can hear you...\n\r", ch);
      return NULL;
   }

   /*
    * Invisible or hidden people.
    */
   if (!can_see(keeper, ch))
   {
      do_say(keeper, "I don't trade with folks I can't see.");
      return NULL;
   }

   speakswell = UMIN(knows_language(keeper, ch->speaking, ch), knows_language(ch, ch->speaking, keeper));

   if ((number_percent() % 65) > speakswell)
   {
      if (speakswell > 60)
         sprintf(buf, "%s Could you repeat that?  I didn't quite catch it.", ch->name);
      else if (speakswell > 50)
         sprintf(buf, "%s Could you say that a little more clearly please?", ch->name);
      else if (speakswell > 40)
         sprintf(buf, "%s Sorry... What was that you wanted?", ch->name);
      else
         sprintf(buf, "%s I can't understand you.", ch->name);
      do_tell(keeper, buf);
      return NULL;
   }

   return keeper;
}

/*
 * repair commands.
 */
CHAR_DATA *find_fixer(CHAR_DATA * ch)
{
   char buf[MSL];
   CHAR_DATA *keeper, *whof;
   REPAIR_DATA *rShop;
   int speakswell;

   rShop = NULL;
   for (keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
      if (IS_NPC(keeper) && IN_SAME_ROOM(keeper, ch) && (rShop = keeper->pIndexData->rShop) != NULL)
         break;

   if (!rShop)
   {
      send_to_char("You can't do that here.\n\r", ch);
      return NULL;
   }


   /*
    * Disallow sales during battle
    */
   if ((whof = who_fighting(keeper)) != NULL)
   {
      if (whof == ch)
         send_to_char("I don't think that's a good idea...\n\r", ch);
      else
         do_say(keeper, "I'm too busy for that!");
      return NULL;
   }

   /* According to rlog, this is the second time I've done this
    * so mobiles can repair in combat.  -- Blod, 1/98
    */
   if (!IS_NPC(ch) && who_fighting(ch))
   {
      ch_printf(ch, "%s doesn't seem to want to get involved.\n\r", PERS(keeper, ch));
      return NULL;
   }

   /*
    * Check to see if show is open.
    * Supports closing times after midnight
    */
   if (rShop->open_hour > rShop->close_hour)
   {
      if (gethour() < rShop->open_hour && gethour() > rShop->close_hour)
      {
         do_say(keeper, "Sorry, come back later.");
         return NULL;
      }
   }
   else
   {
      if (gethour() < rShop->open_hour)
      {
         do_say(keeper, "Sorry, come back later.");
         return NULL;
      }
      if (gethour() > rShop->close_hour)
      {
         do_say(keeper, "Sorry, come back tomorrow.");
         return NULL;
      }
   }

   if (keeper->position == POS_SLEEPING)
   {
      send_to_char("While they're asleep?\n\r", ch);
      return NULL;
   }

   if (keeper->position < POS_SLEEPING)
   {
      send_to_char("I don't think they can hear you...\n\r", ch);
      return NULL;
   }

   /*
    * Invisible or hidden people.
    */
   if (!can_see(keeper, ch))
   {
      do_say(keeper, "I don't trade with folks I can't see.");
      return NULL;
   }

   speakswell = UMIN(knows_language(keeper, ch->speaking, ch), knows_language(ch, ch->speaking, keeper));

   if ((number_percent() % 65) > speakswell)
   {
      if (speakswell > 60)
         sprintf(buf, "%s Could you repeat that?  I didn't quite catch it.", ch->name);
      else if (speakswell > 50)
         sprintf(buf, "%s Could you say that a little more clearly please?", ch->name);
      else if (speakswell > 40)
         sprintf(buf, "%s Sorry... What was that you wanted?", ch->name);
      else
         sprintf(buf, "%s I can't understand you.", ch->name);
      do_tell(keeper, buf);
      return NULL;
   }

   return keeper;
}



int get_cost(CHAR_DATA * ch, CHAR_DATA * keeper, OBJ_DATA * obj, int fBuy)
{
   SHOP_DATA *pShop;
   int cost;
   int tax;
   int taxk;
   bool richcustomer;
   int profitmod;

   if (!obj || (pShop = keeper->pIndexData->pShop) == NULL)
      return 0;

   richcustomer = FALSE;

   if (fBuy == 1 || fBuy == 2)
   {
      profitmod = 0;
      cost = (int) (obj->cost * UMAX((pShop->profit_sell + 1), pShop->profit_buy + profitmod)) / 100;

      /* Thanks to Nick Gammon for pointing out this line
         (it was the first line in this block, making it useless) */

      switch (ch->race) /* racism... should compare against shopkeeper's race */
      {
         case (1):
            cost /= 1.1;
            break; /* elf */
         case (2):
            cost /= 0.97;
            break; /* dwarf */
         case (3):
            cost /= 1.02;
            break; /* halfling */
         case (4):
            cost /= 1.08;
            break; /* pixie */
         case (6):
            cost /= 0.92;
            break; /* half-ogre */
         case (7):
            cost /= 0.94;
            break; /* half-orc */
         case (8):
            cost /= 0.90;
            break; /* half-troll */
         case (9):
            cost /= 1.04;
            break; /* half-elf */
         case (10):
            cost /= 1.06;
            break; /* gith */
      }
   }
   else
   {
      OBJ_DATA *obj2;
      int itype;

      profitmod = get_curr_cha(ch) - 13 - (richcustomer ? 15 : 0);
      cost = 0;
      for (itype = 0; itype < MAX_TRADE; itype++)
      {
         if (obj->item_type == pShop->buy_type[itype])
         {
            cost = (int) (obj->cost * UMIN((pShop->profit_buy - 1), pShop->profit_sell + profitmod)) / 100;
            break;
         }
      }
      /* Checks to see if an obj is on the merchants vnums and if so, will check
         vnums and cvnum against eachother to see if they are a copy. -- Xerves 12/99 */
      for (obj2 = keeper->first_carrying; obj2; obj2 = obj2->next_content)
      {
         if (obj->pIndexData->vnum == obj2->pIndexData->vnum)
         {
            cost = 0;
            break;
         }
         if (obj2->pIndexData->vnum >= 12100 || obj2->pIndexData->vnum <= 13100)
         {
            if (obj2->pIndexData->cvnum == obj->pIndexData->vnum)
            {
               cost = 0;
               break;
            }
         }
      }
   }
   if (fBuy == 2)
   {
      if (ch->in_room->area->kingdom > 1)
      {
         taxk = (cost * kingdom_table[ch->in_room->area->kingdom]->salestax) / 1000;
         cost = cost + taxk;
         tax = (cost * ch->in_room->area->salestax) / 1000;
         cost = cost + tax;
      }
   }
   return cost;
}

int get_repaircost(CHAR_DATA * keeper, OBJ_DATA * obj)
{
   REPAIR_DATA *rShop;
   int cost;
   int itype;
   bool found;

   if (!obj || (rShop = keeper->pIndexData->rShop) == NULL)
      return 0;

   cost = 0;
   found = FALSE;
   for (itype = 0; itype < MAX_FIX; itype++)
   {
      if (obj->item_type == rShop->fix_type[itype])
      {
         cost = (int) (obj->cost * rShop->profit_fix / 1000);
         found = TRUE;
         break;
      }
   }

   if (!found)
      cost = -1;

   if (cost == 0)
      cost = 1;

   if (found && cost > 0)
   {
      switch (obj->item_type)
      {
         case ITEM_ARMOR:
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
            {
               if (obj->value[0] == obj->value[1])
                  cost = -2;
               else
                  cost = cost * (obj->value[1] - obj->value[0]) / obj->value[1];
            }
            else
            {
               if (obj->value[3] == INIT_ARMOR_CONDITION)
                  cost = -2;
               else
                  cost = cost * (1000 - obj->value[3]) / INIT_ARMOR_CONDITION;
            }
            break;
         case ITEM_WEAPON:
            if (INIT_ARMOR_CONDITION == obj->value[0])
               cost = -2;
            else
               cost = cost * (1000 - obj->value[0]) / INIT_ARMOR_CONDITION;
            break;
         case ITEM_SHEATH:
            if (obj->value[3] == INIT_ARMOR_CONDITION)
               cost = -2;
            else
               cost = cost * (INIT_ARMOR_CONDITION - obj->value[3]);
            break;
      }
   }

   return cost;
}


/*****************************************
    Below five functions are used in
    shopkeeper options for their
    Owners (Merchant Job) -- Xerves
 *****************************************/

void do_keeperstat(CHAR_DATA * ch, char *argument)
{
   SHOP_DATA *shop;
   CHAR_DATA *keeper;
   int found = 0;

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot have shopkeepers\n\r", ch);
      return;
   }
   if (!ch->pcdata || !ch->pcdata->keeper || (ch->pcdata->caste < 7))
   {
      send_to_char("You need to have a shop keeper to see its stats.\n\r", ch);
      return;
   }

   for (keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
   {
      if ((IS_NPC(keeper)) && (xIS_SET(keeper->act, ACT_CASTEMOB)) && (keeper->pIndexData->vnum == ch->pcdata->keeper))
      {
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      send_to_char("Either you don't have a shopkeeper or you are not in the room with it\n\r", ch);
      return;
   }
   if (!keeper->pIndexData->pShop)
   {
      send_to_char("This mobile doesn't keep a shop.\n\r", ch);
      return;
   }
   shop = keeper->pIndexData->pShop;

   ch_printf_color(ch, "&G&WKeeper: &c&w%d  %s\n\r", shop->keeper, keeper->short_descr);
   ch_printf_color(ch,
      "&G&Wbuy0 &c&w[%s]  &G&Wbuy1 &c&w[%s]  &G&Wbuy2 &c&w[%s]  \n\r&G&Wbuy3 &c&w[%s]  &G&Wbuy4 &c&w[%s]  &G&Wgold &c&w[&Y%d&c&w]\n\r",
      o_types[shop->buy_type[0]], o_types[shop->buy_type[1]], o_types[shop->buy_type[2]], o_types[shop->buy_type[3]], o_types[shop->buy_type[4]],
      keeper->gold);
   ch_printf_color(ch, "&G&WProfit:  &c&wbuy &R%3d%%  &c&wsell &R%3d%%\n\r", shop->profit_buy, shop->profit_sell);
   ch_printf_color(ch, "&G&WHours:   &c&wopen &R%2d  &c&wclose &R%2d\n\r", shop->open_hour, shop->close_hour);
   ch_printf_color(ch, "&G&WMinGold: &Y%d  &G&WMaxGold: &Y%d  &G&WLimitGold: &Y%d\n\r", keeper->pIndexData->m1, keeper->pIndexData->m2,
      keeper->pIndexData->m3);
   ch_printf_color(ch, "&G&WHoldamt: &g&w%d\n\r", keeper->pIndexData->m6);
   return;
}
void do_keeperset(CHAR_DATA * ch, char *argument)
{
   SHOP_DATA *shop;
   CHAR_DATA *keeper;
   int found = 0;
   char arg1[MIL];
   int value;

   argument = one_argument(argument, arg1);


   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot have a shopkeeper\n\r", ch);
      return;
   }
   if (!ch->pcdata || !ch->pcdata->keeper || (ch->pcdata->caste < 7))
   {
      send_to_char("You need to have a shop keeper to see its stats.\n\r", ch);
      return;
   }

   for (keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
   {
      if ((IS_NPC(keeper)) && (xIS_SET(keeper->act, ACT_CASTEMOB)) && (keeper->pIndexData->vnum == ch->pcdata->keeper))
      {
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      send_to_char("Either you don't have a shopkeeper or you are not in the room with it\n\r", ch);
      return;
   }
   if (!keeper->pIndexData->pShop)
   {
      send_to_char("This mobile doesn't keep a shop.\n\r", ch);
      bug("Mob %d is not a keeper, but set as a player keeper", keeper->pIndexData->vnum);
      return;
   }
   if (arg1[0] == '\0')
   {
      send_to_char("Usage: keeperset <field> value\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("buy0 buy1 buy2 buy3 buy4 buy sell open close\n\r", ch);
      send_to_char("gold0 gold1 gold2\n\r", ch);
      send_to_char("holdamt\n\r", ch);
      return;
   }
   shop = keeper->pIndexData->pShop;
   value = atoi(argument);

   if (!str_cmp(arg1, "buy0"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[0] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "buy1"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[1] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "buy2"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[2] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "buy3"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[3] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "buy4"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[4] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "buy"))
   {
      if (value <= (shop->profit_sell + 5) || value > 1000)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->profit_buy = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "sell"))
   {
      if (value < 0 || value >= (shop->profit_buy - 5))
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->profit_sell = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "open"))
   {
      if (value < 0 || value > 23)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->open_hour = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "close"))
   {
      if (value < 0 || value > 23)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->close_hour = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "gold0"))
   {
      if (value > keeper->pIndexData->m2)
      {
         send_to_char("Your minimum cannot be higher than the maximum\n\r", ch);
         return;
      }
      if (value < 1)
      {
         send_to_char("Value needs to be 1 or greater\n\r", ch);
         return;
      }
      keeper->pIndexData->m1 = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "gold1"))
   {
      if (value < keeper->pIndexData->m1)
      {
         send_to_char("You maximum cannot be lower than the minimum\n\r", ch);
         return;
      }
      if (value < 1)
      {
         send_to_char("Value needs to be 1 or greater\n\r", ch);
         return;
      }
      keeper->pIndexData->m2 = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "gold2"))
   {
      if (value < 1)
      {
         send_to_char("Value needs to be 1 or greater\n\r", ch);
         return;
      }
      keeper->pIndexData->m3 = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "holdamt"))
   {
      if (value < 1 || value > 300)
      {
         send_to_char("Range is 1 to 300\n\r", ch);
         return;
      }
      keeper->pIndexData->m6 = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   do_keeperset(ch, "");
   return;
}
void do_goldtake(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *keeper;
   char arg[MIL];
   int found = 0;
   int amount;

   argument = one_argument(argument, arg);

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot have shopkeepers\n\r", ch);
      return;
   }
   if (!ch->pcdata || !ch->pcdata->keeper || (ch->pcdata->caste < 7))
   {
      send_to_char("You need to have a shop keeper to exchange cash.\n\r", ch);
      return;
   }

   for (keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
   {
      if ((IS_NPC(keeper)) && (xIS_SET(keeper->act, ACT_CASTEMOB)) && (keeper->pIndexData->vnum == ch->pcdata->keeper))
      {
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      send_to_char("Either you don't have a shopkeeper or you are not in the room with it\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Syntax: goldtake <amount>\n\r", ch);
      return;
   }
   amount = atoi(arg);
   if (!is_number(arg))
   {
      send_to_char("Amount needs to be a number\n\r", ch);
      return;
   }
   if (amount > keeper->gold || amount > keeper->pIndexData->gold)
   {
      send_to_char("Your keeper does not have that much gold.\n\r", ch);
      return;
   }
   ch->gold += amount;
   keeper->pIndexData->gold -= amount;
   keeper->gold -= amount;
   send_to_char("Amount transfered\n\r", ch);
   return;
}
void do_goldgive(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *keeper;
   char arg[MIL];
   int found = 0;
   int amount;

   argument = one_argument(argument, arg);

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot have shopkeepers\n\r", ch);
      return;
   }
   if (!ch->pcdata || !ch->pcdata->keeper || (ch->pcdata->caste < 7))
   {
      send_to_char("You need to have a shop keeper to exchange cash.\n\r", ch);
      return;
   }

   for (keeper = ch->in_room->first_person; keeper; keeper = keeper->next_in_room)
   {
      if ((IS_NPC(keeper)) && (xIS_SET(keeper->act, ACT_CASTEMOB)) && (keeper->pIndexData->vnum == ch->pcdata->keeper))
      {
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      send_to_char("Either you don't have a shopkeeper or you are not in the room with it\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Syntax: goldgive <amount>\n\r", ch);
      return;
   }
   amount = atoi(arg);
   if (!is_number(arg))
   {
      send_to_char("Amount needs to be a number\n\r", ch);
      return;
   }
   if (amount > ch->gold)
   {
      send_to_char("You do not have that much gold on you.\n\r", ch);
      return;
   }
   ch->gold -= amount;
   keeper->pIndexData->gold += amount;
   keeper->gold += amount;
   send_to_char("Amount transfered\n\r", ch);
   return;
}
void do_makekeeper(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int amount;
   CHAR_DATA *victim;

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot give a player a keeper.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: makekeeper <char> <vnum>\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1 == '\0' || arg2 == '\0')
   {
      send_to_char("Syntax: makekeeper <char> <vnum>\n\r", ch);
      return;
   }

   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("NOT ON NPCs.\n\r", ch);
      return;
   }

   if (get_trust(ch) <= get_trust(victim) && ch != victim)
   {
      send_to_char("Don't do that AGAIN!.\n\r", ch);
      return;
   }

   if (!is_number(arg2))
   {
      send_to_char("Syntax: makekeeper <char> <vnum>\n\r", ch);
      return;
   }

   amount = atoi(arg2);

   if (amount < 1 || amount > MAX_VNUM)
   {
      send_to_char("Not a valid vnum\n\r", ch);
      return;
   }
   victim->pcdata->keeper = amount;
   send_to_char("Keeper set\n\r", ch);
   send_to_char("You now have a keeper\n\r", victim);
   return;
}
void do_pcshops(CHAR_DATA * ch, char *argument)
{
   SHOP_DATA *shop;
   MOB_INDEX_DATA *keeper;

   if (!first_shop)
   {
      send_to_char("There are no shops.\n\r", ch);
      return;
   }
   set_char_color(AT_NOTE, ch);
   send_to_char("--------------------------------------------------------------------------\n\r", ch);
   for (shop = first_shop; shop; shop = shop->next)
   {
      if ((keeper = get_mob_index(shop->keeper)) == NULL)
      {
         bug("Shop %d found, but keeper doesn't exsist", shop->keeper);
         send_to_char("Error, returning out of pcshops", ch);
      }
      if (xIS_SET(keeper->act, ACT_CASTEMOB))
      {
         ch_printf(ch,
            "&GKeeper: %5d &G&WBuy: &B%3d &G&WSell: &B%3d &G&WOpen: &C%2d &G&WClose: &C%2d &G&WBuy: &c%2d %2d %2d %2d %2d\n\r              &G&WMinGold: &Y%-8d &G&WMaxGold: &Y%-8d &G&WLimGold: &Y%-8d\n\r              &G&WMinLevel: &c&w%2d &G&WMaxlevel: &c&w%2d\n\r",
            shop->keeper, shop->profit_buy, shop->profit_sell, shop->open_hour, shop->close_hour, shop->buy_type[0], shop->buy_type[1],
            shop->buy_type[2], shop->buy_type[3], shop->buy_type[4], keeper->m1, keeper->m2, keeper->m3, keeper->m4, keeper->m5);
         send_to_char("&G--------------------------------------------------------------------------\n\r", ch);
      }
   }
   return;
}
void do_buy(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   int maxgold;
   int ctax = 0;
   int tax = 0;
   int taxk = 0;
   TOWN_DATA *town;
   int cost;
   TOWN_DATA *dtown;
   int level;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Buy what?\n\r", ch);
      return;
   }
   
   if (xIS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP))
   {
      char buf[MSL];
      CHAR_DATA *mount;
      ROOM_INDEX_DATA *pRoomIndexNext;
      ROOM_INDEX_DATA *in_room;

      if (IS_NPC(ch))
         return;

      pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
      if (!pRoomIndexNext)
      {
         bug("Do_buy: bad mount shop at vnum %d.", ch->in_room->vnum);
         send_to_char("Sorry, you can't buy that here.\n\r", ch);
         return;
      }

      in_room = ch->in_room;
      ch->in_room = pRoomIndexNext;
      mount = get_char_room_new(ch, arg, 1);
      ch->in_room = in_room;

      if (mount == NULL || !IS_NPC(mount) || !xIS_SET(mount->act, ACT_MOUNTSAVE))
      {
         send_to_char("Sorry, you can't buy that here.\n\r", ch);
         return;
      }

      if (xIS_SET(ch->act, PLR_BOUGHT_MOUNT))
      {
         send_to_char("You already have a mount\n\r", ch);
         return;
      }
      if (ch->in_room->area->kingdom > 1)
      {
         tax = kingdom_table[ch->in_room->area->kingdom]->salestax;
         taxk = ch->in_room->area->salestax;
      }
      else
      {
         tax = 0;
         taxk = 0;
      }
      cost = mount->m2;
      if (ch->race == RACE_HOBBIT)
         cost = cost * 85 / 100;
      if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
      {
         level = POINT_LEVEL(LEARNED(ch, gsn_haggling), MASTERED(ch, gsn_haggling));
         cost = cost * (100-(level/2)) / 100;
      }
      ctax = (cost * tax) / 1000;
      taxk = (cost * taxk) / 1000;
      if (ch->gold < (mount->m2 + ctax + taxk))
      {
         send_to_char("You can't afford it.\n\r", ch);
         return;
      }
      if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
         learn_from_success(ch, gsn_haggling, NULL);
      maxgold = cost;
      ch->gold -= (maxgold + ctax + taxk);
      
      if (IN_WILDERNESS(ch))
      {
         if ((town = find_town(ch->coord->x, ch->coord->y, ch->map)) != NULL)
         {         
            if (tax)
            {
               if ((dtown = find_town(ch->coord->x, ch->coord->y, ch->map)) != NULL)
               {
                  if ((get_current_hold(dtown) + (ctax/100)) <= dtown->hold)
                     dtown->coins += ctax;
               }
            }
            if (taxk)
            {
               if ((get_current_hold(town) + (taxk/100)) <= town->hold)
                  town->coins += taxk;
            }
         }
      }
      boost_economy(ch->in_room->area, maxgold);
      mount = create_mobile(mount->pIndexData);
      xSET_BIT(ch->act, PLR_BOUGHT_MOUNT);
      xSET_BIT(mount->act, ACT_MOUNTSAVE);
      xSET_BIT(mount->affected_by, AFF_CHARM);
      mount->mover = mount->m3;
      mount->m4 = ch->pcdata->hometown;
/*	This isn't needed anymore since you can order your pets --Shaddai
	xSET_BIT(pet->affected_by, AFF_CHARM);
*/

      argument = one_argument(argument, arg);
      if (arg[0] != '\0')
      {
         sprintf(buf, "%s %s", mount->name, arg);
         STRFREE(mount->name);
         mount->name = STRALLOC(buf);
      }

      sprintf(buf, "%sA neck tag says 'I am a mount, don't kill me'.\n\r", mount->description);
      STRFREE(mount->description);
      mount->description = STRALLOC(buf);

      char_to_room(mount, ch->in_room);
      add_follower(mount, ch);
      send_to_char("Enjoy your mount.\n\r", ch);
      act(AT_ACTION, "$n bought $N as a mount.", ch, NULL, mount, TO_ROOM);
      return;
   }
   else
   {
      CHAR_DATA *keeper;
      OBJ_DATA *obj;
      OBJ_DATA *fobj;
      AREA_DATA *pArea; /* Passing resets to special mobs */
      ROOM_INDEX_DATA *pRoom;
      int cost;
      int noi = 1; /* Number of items */
      sh_int mnoi = 20; /* Max number of items to be bought at once */

      if ((keeper = find_keeper(ch)) == NULL)
         return;

      pRoom = keeper->in_room;
      pArea = pRoom->area;
      maxgold = 5000000;

      if (is_number(arg))
      {
         noi = atoi(arg);
         argument = one_argument(argument, arg);
         if (noi > mnoi)
         {
            act(AT_TELL, "$n tells you 'I don't sell that many items at" " once.'", keeper, NULL, ch, TO_VICT);
            ch->reply = keeper;
            return;
         }
      }

      obj = get_obj_carry(keeper, arg);
      cost = (get_cost(ch, keeper, obj, TRUE) * noi);
      if (cost <= 0 || !can_see_obj(ch, obj))
      {
         act(AT_TELL, "$n tells you 'I don't sell that -- try 'list'.'", keeper, NULL, ch, TO_VICT);
         ch->reply = keeper;
         return;
      }

      if (!IS_OBJ_STAT(obj, ITEM_INVENTORY) && (noi > 1))
      {
         interpret(keeper, "laugh");
         act(AT_TELL, "$n tells you 'I don't have enough of those in stock" " to sell more than one at a time.'", keeper, NULL, ch, TO_VICT);
         ch->reply = keeper;
         return;
      }
      if (IS_UNIQUE(ch, obj))
      {
         act(AT_TELL, "$n tells you 'You can only have 1 of those items on you at a time!", keeper, NULL, ch, TO_VICT);
         ch->reply = keeper;
         return;
      }
      if (ch->race == RACE_HOBBIT)
      {
         cost = cost * 85 / 100;
         if (cost < obj->cost)
            cost = obj->cost;
      }
      if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
      {
         level = POINT_LEVEL(LEARNED(ch, gsn_haggling), MASTERED(ch, gsn_haggling));
         cost = cost * (100-(level/2)) / 100;
         //Cannot go below actual cost of the item
         if (cost < obj->cost)
            cost = obj->cost;
         if (MASTERED(ch, gsn_haggling) == 4 && cost > obj->cost)
            cost = obj->cost;
      }
      if (ch->in_room->area->kingdom > 1)
      {
         tax = kingdom_table[ch->in_room->area->kingdom]->salestax;
         taxk = ch->in_room->area->salestax;
      }
      else
      {
         tax = 0;
         taxk = 0;
      }

      ctax = (cost * tax) / 1000;
      taxk = (cost * taxk) / 1000;
      if (ch->gold < (cost + ctax + taxk))
      {
         act(AT_TELL, "$n tells you 'You can't afford to buy $p.'", keeper, obj, ch, TO_VICT);
         ch->reply = keeper;
         return;
      }
      if (obj->level > ch->level)
      {
         act(AT_TELL, "$n tells you 'You can't use $p yet.'", keeper, obj, ch, TO_VICT);
         ch->reply = keeper;
         return;
      }

      if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE) && get_trust(ch) < LEVEL_IMMORTAL)
      {
         act(AT_TELL, "$n tells you 'This is a only a prototype!  I can't sell you that...'", keeper, NULL, ch, TO_VICT);
         ch->reply = keeper;
         return;
      }

      if (get_ch_carry_number(ch) + get_obj_number(obj) > can_carry_n(ch))
      {
         send_to_char("You can't carry that many items.\n\r", ch);
         return;
      }

      if (get_ch_carry_weight(ch) + (get_obj_weight(obj) * noi) + (noi > 1 ? 2 : 0) > can_carry_w(ch))
      {
         send_to_char("You can't carry that much weight.\n\r", ch);
         return;
      }
      if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
         learn_from_success(ch, gsn_haggling, NULL);
      if (noi == 1)
      {
         act(AT_ACTION, "$n buys $p.", ch, obj, NULL, TO_ROOM);
         act(AT_ACTION, "You buy $p.", ch, obj, NULL, TO_CHAR);
      }
      else
      {
         sprintf(arg, "$n buys %d $p%s.", noi, (obj->short_descr[strlen(obj->short_descr) - 1] == 's' ? "" : "s"));
         act(AT_ACTION, arg, ch, obj, NULL, TO_ROOM);
         sprintf(arg, "You buy %d $p%s.", noi, (obj->short_descr[strlen(obj->short_descr) - 1] == 's' ? "" : "s"));
         act(AT_ACTION, arg, ch, obj, NULL, TO_CHAR);
         act(AT_ACTION, "$N puts them into a bag and hands it to you.", ch, NULL, keeper, TO_CHAR);
      }
      if (ch->pcdata->keeper == keeper->pIndexData->vnum)
      {
         cost = 0;
      }
      ch->gold -= (cost + ctax + taxk);
      keeper->gold += cost;
      
      if (IN_WILDERNESS(ch))
      {
         if ((town = find_town(ch->coord->x, ch->coord->y, ch->map)) != NULL)
         {         
            if (tax)
            {
               if ((dtown = find_town(ch->coord->x, ch->coord->y, ch->map)) != NULL)
               {
                  if ((get_current_hold(dtown) + (ctax/100)) <= dtown->hold)
                     dtown->coins += ctax;
               }
            }
            if (taxk)
            {
               if ((get_current_hold(town) + (taxk/100)) <= town->hold)
                  town->coins += taxk;
            }
         }
      }
      
      if (xIS_SET(keeper->act, ACT_CASTEMOB))
         keeper->pIndexData->gold += cost;

      if (!xIS_SET(keeper->act, ACT_CASTEMOB))
      {
         if (keeper->gold > maxgold)
         {
            boost_economy(keeper->in_room->area, keeper->gold - maxgold / 2);
            keeper->gold = maxgold / 2;
            act(AT_ACTION, "$n puts some gold into a large safe.", keeper, NULL, NULL, TO_ROOM);
         }
      }

      if (IS_OBJ_STAT(obj, ITEM_INVENTORY))
      {
         OBJ_DATA *buy_obj, *bag;
         SLAB_DATA *slab;
         int race;

         buy_obj = create_object(obj->pIndexData, obj->level);
         if (obj->value[6] > 99 && (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_ARMOR) && xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
         {
            for (slab = first_slab; slab; slab = slab->next)
            {
               if (slab->vnum == obj->value[6])
                  break;
            }
            if (!slab)
            {
               bug("Do_Buy, Obj %d on Mob %d has an invalid slab.", obj->pIndexData->vnum, keeper->pIndexData->vnum);
            }
            else
            {         
               race = keeper->race;
               if (keeper->race < 0 || keeper->race >= MAX_RACE)
                  keeper->race = 0; //Needs a valid race      
               alter_forge_obj(keeper, buy_obj, create_object(get_obj_index(slab->vnum), 1), slab);	
               keeper->race = race;
            }
         }         

         /*
          * Due to grouped objects and carry limitations in SMAUG
          * The shopkeeper gives you a bag with multiple-buy,
          * and also, only one object needs be created with a count
          * set to the number bought.  -Thoric
          */
         if (noi > 1)
         {
            bag = create_object(get_obj_index(OBJ_VNUM_SHOPPING_BAG), 1);
            xSET_BIT(bag->extra_flags, ITEM_GROUNDROT);
            //bag->timer = 10; /* Blodkai, 4/97 */
            /* perfect size bag ;) */
            bag->value[0] = bag->weight + (buy_obj->weight * noi);
            buy_obj->count = noi;
            obj->pIndexData->count += (noi - 1);
            numobjsloaded += (noi - 1);
            obj_to_obj(buy_obj, bag);
            obj_to_char(bag, ch);
         }
         else
            obj_to_char(buy_obj, ch);
      }
      else
      {
         if (xIS_SET(keeper->act, ACT_CASTEMOB))
         {
            fobj = shop_oclean(keeper, obj);
            fobj->level = 1;
            obj_from_char(obj);
            obj_to_char(fobj, ch);
            delete_obj(obj->pIndexData);
            kupkeep(pArea, pRoom);
            fdarea(keeper, pArea->filename);
         }
         else
         {
            obj_from_char(obj);
            obj_to_char(obj, ch);
         }
      }

      return;
   }
}

/*
 * This is a new list function which allows level limits to follow as
 * arguments.  This code relies heavily on the items held by the shopkeeper
 * being sorted in descending order by level.  obj_to_char in handler.c was
 * modified to achieve this.  Anyways, this list command will now throw flags
 * at the levels passed as arguments.  This helps pick out equipment which is
 * usable by the char, etc.  This was done rather than just producing a list
 * of equip at desired level because there would be an inconsistency between
 * #.item on one list and #.item on the other.
 * Syntax:
 *      list            -       list the items for sale, should be sorted by
 *                              level
 *      list #          -       list items and throw a flag at #
 *      list #1 #2      -       list items and throw flags at #1 and #2
 * Note that this won't work in pets stores. Since you can't control
 * the order in which the pets repop you can't guarantee a sorted list.
 * Last Modified: May 25, 1997 -- Fireblade
 */
void do_list(CHAR_DATA * ch, char *argument)
{
   /* Constants for producing the flags */
   char *divleft = "-----------------------------------[ ";
   char *divright = " ]-----------------------------------";
   int cost;
   int level;


   if (xIS_SET(ch->in_room->room_flags, ROOM_MOUNT_SHOP))
   {
      ROOM_INDEX_DATA *pRoomIndexNext;
      CHAR_DATA *mount;
      bool found;

      pRoomIndexNext = get_room_index(ch->in_room->vnum + 1);
      if (!pRoomIndexNext)
      {
         bug("Do_list: bad mount shop at vnum %d.", ch->in_room->vnum);
         send_to_char("You can't do that here.\n\r", ch);
         return;
      }

      found = FALSE;

      for (mount = pRoomIndexNext->first_person; mount; mount = mount->next_in_room)
      {
         if (xIS_SET(mount->act, ACT_MOUNTSAVE) && IS_NPC(mount))
         {
            if (!found)
            {
               found = TRUE;
               send_to_pager("Mounts for sale:\n\r", ch);
               send_to_pager("Endurance  Armor  HP     MaxWeight  Cost     Name\n\r", ch);
               send_to_pager("----------------------------------------------------------------\n\r", ch);
            }
            cost = mount->m2;
            if (ch->race == RACE_HOBBIT)
            {
               cost = cost * 85 / 100;
            }
            if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
            {
               level = POINT_LEVEL(LEARNED(ch, gsn_haggling), MASTERED(ch, gsn_haggling));
               cost = cost * (100-(level/2)) / 100;
            }
            pager_printf(ch, "%s[%2d]       [%2d]   [%3d]  [%4d]    %8d  %s\n\r", char_color_str(AT_PERSON, ch), mount->m3, mount->armor, mount->max_hit, can_carry_w(mount), mount->m2, mount->short_descr);
         }
      }
      if (!found)
         send_to_char("Sorry, we're out of mounts right now.\n\r", ch);
      return;
   }
   else
   {
      char arg[MIL];
      char *rest;
      CHAR_DATA *keeper;
      OBJ_DATA *obj;
      bool found;

/*      bool listall; */
      int lower, upper;

      rest = one_argument(argument, arg);

      if ((keeper = find_keeper(ch)) == NULL)
         return;

      found = FALSE;
      lower = -2;
      upper = -1;

      /* Get the level limits for the flags */
      if (is_number(arg))
      {
         lower = atoi(arg);
         rest = one_argument(rest, arg);

         if (is_number(arg))
         {
            upper = atoi(arg);
            rest = one_argument(rest, arg);
         }
      }

      /* Fix the limits if reversed */
      if (lower >= upper)
      {
         int temp;

         temp = lower;
         lower = upper;
         upper = temp;
      }

      /* Loop until you see an object higher level than char */
      /* Note that this depends on the keeper having a sorted list */
      for (obj = keeper->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->wear_loc == WEAR_NONE
            && can_see_obj(ch, obj) && (cost = get_cost(ch, keeper, obj, 2)) > 0 && (arg[0] == '\0' || nifty_is_name(arg, obj->name)))
         {
            if (!found)
            {
               found = TRUE;
               send_to_pager("&c&w[Lv Price] Item\n\r---------------\n\r", ch);
            }

            if (obj->level <= upper)
            {
               pager_printf(ch, "%s%2d%s\n\r", divleft, upper, divright);
               upper = -1;
            }

            if (obj->level < lower)
            {
               pager_printf(ch, "%s%2d%s\n\r", divleft, lower, divright);
               lower = -1;
            }
            if (ch->race == RACE_HOBBIT)
            {
               cost = cost * 85 / 100;
               if (cost < obj->cost)
                  cost = obj->cost;
            }
            if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
            {
               level = POINT_LEVEL(LEARNED(ch, gsn_haggling), MASTERED(ch, gsn_haggling));
               cost = cost * (100-(level/2)) / 100;
               //Cannot go below actual cost of the item
               if (cost < obj->cost)
                  cost = obj->cost;
               if (MASTERED(ch, gsn_haggling) == 4 && cost > obj->cost)
                  cost = obj->cost;
            }
            pager_printf( ch, "[%2d %5d] "MXPTAG ("list '%s' '%s'")"%s"MXPTAG ("/list") ".\n\r",obj->level, cost, obj->name, 
              obj->short_descr, capitalize( obj->short_descr ) );
         }
      }

      if (lower >= 0)
      {
         pager_printf(ch, "%s%2d%s\n\r", divleft, lower, divright);
      }

      if (!found)
      {
         if (arg[0] == '\0')
            send_to_char("You can't buy anything here.\n\r", ch);
         else
            send_to_char("You can't buy that here.\n\r", ch);
      }
      return;
   }
}

void do_sell(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *pArea; /* Passing resets to special mobs */
   ROOM_INDEX_DATA *pRoom;
   char buf[MSL];
   char arg[MIL];
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   OBJ_DATA *iobj;
   OBJ_DATA *fobj;
   int cost;
   int outcome = 0;
   int objcount = 0;
   int maxaobj = 0;
   int level;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Sell what?\n\r", ch);
      return;
   }

   if ((keeper = find_keeper(ch)) == NULL)
      return;

   if ((obj = get_obj_carry(ch, arg)) == NULL)
   {
      act(AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT);
      ch->reply = keeper;
      return;
   }

   if (!can_drop_obj(ch, obj))
   {
      send_to_char("You can't let go of it!\n\r", ch);
      return;
   }

/* Prevents giving of special items (rare items) -- Xerves 3/24/98*/
   if (IS_OBJ_STAT(obj, ITEM_NOGIVE))
   {
      send_to_char("Xerves should personally smack you for trying to find a loophole.  Cannot sell this item.\n\r", ch);
      return;
   }


   if (obj->timer > 0)
   {
      act(AT_TELL, "$n tells you, '$p is depreciating in value too quickly...'", keeper, obj, ch, TO_VICT);
      return;
   }

   if ((cost = get_cost(ch, keeper, obj, FALSE)) <= 0)
   {
      act(AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
   }
   if (ch->race == RACE_HOBBIT)
   {
      cost = cost * 115 / 100;
      if (cost > obj->cost)
         cost = obj->cost;
   }
   if (!IS_NPC(ch) && LEARNED(ch, gsn_swindling) > 0)
   {
      level = POINT_LEVEL(LEARNED(ch, gsn_swindling), MASTERED(ch, gsn_swindling));
      cost = cost * (100+(level/2)) / 100;
      //Cannot go above actual cost of the item
      if (cost > obj->cost)
         cost = obj->cost;
      if (MASTERED(ch, gsn_swindling) == 4 && cost < obj->cost)
         cost = obj->cost;
      learn_from_success(ch, gsn_swindling, NULL);
   }
   if (xIS_SET(keeper->act, ACT_CASTEMOB))
      if ((keeper->pIndexData->m1 != 0)
         && (keeper->pIndexData->m2 != 0) && (keeper->pIndexData->m3 != 0) && (keeper->pIndexData->m4 != 0) && (keeper->pIndexData->m5 != 0))
      {
         outcome = keeper->gold - cost;
         if (outcome < keeper->m3)
         {
            act(AT_TELL, "$n tells you, 'I cannot afford to sell you $p, sorry'", keeper, obj, ch, TO_VICT);
            return;
         }
         if (obj->cost < keeper->m1 || obj->cost > keeper->m2)
         {
            sprintf(buf, "$n tells you, '$p is not worth %d to %d gold'", keeper->m1, keeper->m2);
            act(AT_TELL, buf, keeper, obj, ch, TO_VICT);
            return;
         }
      }
   if (xIS_SET(keeper->act, ACT_CASTEMOB))
   {
      if (keeper->pIndexData->m6 == 0)
         maxaobj = 20;
      else
         maxaobj = keeper->pIndexData->m6;

      for (iobj = keeper->first_carrying; iobj; iobj = iobj->next_content)
      {
         if (iobj->wear_loc == WEAR_NONE)
            objcount++;
      }
      if (objcount + 1 > maxaobj)
      {
         sprintf(buf, "$n tells you, '$p exceeds my limit of %d items.'", maxaobj);
         act(AT_TELL, buf, keeper, obj, ch, TO_VICT);
         return;
      }
   }


   if (cost >= keeper->gold)
   {
      act(AT_TELL, "$n tells you, '$p is worth more than I can afford...'", keeper, obj, ch, TO_VICT);
      return;
   }

   separate_obj(obj);
   act(AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM);
   sprintf(buf, "You sell $p for %d gold piece%s.", cost, cost == 1 ? "" : "s");
   act(AT_ACTION, buf, ch, obj, NULL, TO_CHAR);
   if (ch->pcdata->keeper == keeper->pIndexData->vnum)
   {
      cost = 0;
   }
   ch->gold += cost;
   keeper->gold -= cost;
   if (xIS_SET(keeper->act, ACT_CASTEMOB))
      keeper->pIndexData->gold -= cost;
   if (keeper->gold < 0)
      keeper->gold = 0;

   pRoom = keeper->in_room;
   pArea = pRoom->area;

   if (obj->item_type == ITEM_TRASH)
   {
      extract_obj(obj);
      if (xIS_SET(keeper->act, ACT_CASTEMOB))
         kupkeep(pArea, pRoom);
   }
   else
   {
      if (xIS_SET(keeper->act, ACT_CASTEMOB))
      {
         fobj = shop_ocreate(keeper, obj);
         obj_from_char(obj);
         extract_obj(obj);
         obj_to_char(fobj, keeper);
         kupkeep(pArea, pRoom);
         fdarea(keeper, pArea->filename);

      }
      else
      {
         obj_from_char(obj);
         obj_to_char(obj, keeper);
      }

   }

   return;
}

void do_value(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   int level;
   int cost;

   if (argument[0] == '\0')
   {
      send_to_char("Value what?\n\r", ch);
      return;
   }

   if ((keeper = find_keeper(ch)) == NULL)
      return;

   if ((obj = get_obj_carry(ch, argument)) == NULL)
   {
      act(AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT);
      ch->reply = keeper;
      return;
   }

   if (!can_drop_obj(ch, obj))
   {
      send_to_char("You can't let go of it!\n\r", ch);
      return;
   }

   if ((cost = get_cost(ch, keeper, obj, FALSE)) <= 0)
   {
      act(AT_ACTION, "$n looks uninterested in $p.", keeper, obj, ch, TO_VICT);
      return;
   }
   if (ch->race == RACE_HOBBIT)
   {
      cost = cost * 115 / 100;
      if (cost > obj->cost)
         cost = obj->cost;
   }
   if (!IS_NPC(ch) && LEARNED(ch, gsn_swindling) > 0)
   {
      level = POINT_LEVEL(LEARNED(ch, gsn_swindling), MASTERED(ch, gsn_swindling));
      cost = cost * (100+(level/2)) / 100;
      //Cannot go above actual cost of the item
      if (cost > obj->cost)
         cost = obj->cost;
      if (MASTERED(ch, gsn_swindling) == 4 && cost < obj->cost)
         cost = obj->cost;
   }
   sprintf(buf, "$n tells you 'I'll give you %d gold coins for $p.'", cost);
   act(AT_TELL, buf, keeper, obj, ch, TO_VICT);
   ch->reply = keeper;

   return;
}

/*
 * Repair a single object. Used when handling "repair all" - Gorog
 */
void repair_one_obj(CHAR_DATA * ch, CHAR_DATA * keeper, OBJ_DATA * obj, char *arg, int maxgold, char *fixstr, char *fixstr2)
{
   char buf[MSL];
   char arg1[MIL];
   int cost = 0;
   int level;
   
   if (obj->item_type == ITEM_WEAPON && (obj->value[4] > 0  || obj->value[5] > 0))
   {
      act(AT_TELL, "$n tells you, 'Sorry, I cannot repair $p because a great magical power prevents me.'", keeper, obj, ch, TO_VICT);
      return;
   }

   if (!can_drop_obj(ch, obj))
      ch_printf(ch, "You can't let go of %s.\n\r", obj->name);
   else if ((cost = get_repaircost(keeper, obj)) < 0)
   {
      if (cost != -2)
         act(AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'", keeper, obj, ch, TO_VICT);
      else
         act(AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT);
   }
   /* "repair all" gets a 10% surcharge - Gorog */

   else if ((cost = strcmp("all", arg) ? cost : 11 * cost / 10) > ch->gold)
   {
      if (ch->race == RACE_HOBBIT)
      {
         cost = cost * 85 / 100;
      }
      if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
      {
         level = POINT_LEVEL(LEARNED(ch, gsn_haggling), MASTERED(ch, gsn_haggling));
         cost = cost * (100-(level/2)) / 100;
      }
      sprintf(buf, "$N tells you, 'It will cost %d piece%s of gold to %s %s...'", cost, cost == 1 ? "" : "s", fixstr, obj->name);
      act(AT_TELL, buf, ch, NULL, keeper, TO_CHAR);
      act(AT_TELL, "$N tells you, 'Which I see you can't afford.'", ch, NULL, keeper, TO_CHAR);
   }
   else
   {
      if (ch->race == RACE_HOBBIT)
      {
         cost = cost * 85 / 100;
      }
      if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
      {
         level = POINT_LEVEL(LEARNED(ch, gsn_haggling), MASTERED(ch, gsn_haggling));
         cost = cost * (100-(level/2)) / 100;
      }
      one_argument(arg, arg1);
      if (!str_cmp(arg1, "cost"))
      {
         sprintf(buf, "$N informs you it will cost %d to fix $p.", cost);
         act(AT_ACTION, buf, ch, obj, keeper, TO_CHAR);
         return;
      }
      if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
         learn_from_success(ch, gsn_haggling, NULL);
      sprintf(buf, "$n gives $p to $N, who quickly %s it.", fixstr2);
      act(AT_ACTION, buf, ch, obj, keeper, TO_ROOM);
      sprintf(buf, "$N charges you %d gold piece%s to %s $p.", cost, cost == 1 ? "" : "s", fixstr);
      act(AT_ACTION, buf, ch, obj, keeper, TO_CHAR);
      ch->gold -= cost;
      keeper->gold += cost;
      if (keeper->gold < 0)
         keeper->gold = 0;
      else if (keeper->gold > maxgold)
      {
         boost_economy(keeper->in_room->area, keeper->gold - maxgold / 2);
         keeper->gold = maxgold / 2;
         act(AT_ACTION, "$n puts some gold into a large safe.", keeper, NULL, NULL, TO_ROOM);
      }

      switch (obj->item_type)
      {
         default:
            send_to_char("For some reason, you think you got ripped off...\n\r", ch);
            break;
         case ITEM_ARMOR:
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
            {
               obj->value[0] = obj->value[1];
            }
            else
               obj->value[3] = INIT_ARMOR_CONDITION;
            break;
         case ITEM_WEAPON:
            obj->value[0] = INIT_ARMOR_CONDITION;
            break;
         case ITEM_SHEATH:
            obj->value[3] = INIT_ARMOR_CONDITION;
      }

      oprog_repair_trigger(ch, obj);
   }
}

void do_break(CHAR_DATA *ch, char *argument)
{
    int scnt;
    OBJ_DATA *first_obj, *fobj;
    SLAB_DATA *slab;
    FORGE_DATA *forge;
    char arg[MIL];
    int race;
    int slabs;
    char *wbuf;
    char wname[MIL]; //for stripping to check race
    OBJ_DATA *repair;
    int chance;
    
    argument = one_argument(argument, arg);
       
    for(first_obj = ch->first_carrying; first_obj; first_obj = first_obj->next_content)
    {
       if( is_name(arg, first_obj->name) && xIS_SET(first_obj->extra_flags, ITEM_FORGEABLE)
       &&  first_obj->wear_loc == WEAR_NONE )
          break;
    }  
    if (!first_obj)
    {  
       for(first_obj = ch->in_room->first_content; first_obj; first_obj = first_obj->next_content)
       {
          if( is_name(arg, first_obj->name) && xIS_SET(first_obj->extra_flags, ITEM_FORGEABLE)
          &&  first_obj->wear_loc == WEAR_NONE )
             break;
       }          
    }
    if (!first_obj)
    {
       send_to_char("That item is either not in your inventory or on the ground.\n\r", ch);
       return;
    }       
    if (first_obj->item_type == ITEM_PROJECTILE)
    {
       send_to_char("You cannot break down arrows.\n\r", ch);
       return;
    }
    if (IS_OBJ_STAT(first_obj, ITEM_KINGDOMEQ))
    {
       send_to_char("Cannot break this item, it is kingdom eq\n\r", ch);
       return;
    }
    for(slab = first_slab; slab; slab = slab->next)
    {
       if(is_name(slab->adj, first_obj->name) )
          break;
    }
    if (!slab)
    {
       send_to_char("Error:  Did not find the Ore, tell an immortal.\n\r", ch);
       bug("forge: Forgeable has an invalid ore.");
       return;
    }
    for (forge = first_forge; forge; forge = forge->next)
    {
       if (forge->vnum == first_obj->pIndexData->vnum)
          break;
    }
    if (!forge)
    {
       send_to_char("Error:  Error with the weapon/armor, tell an immotal.\n\r", ch);
       bug("Forge: Could not find the weapon/armor in the forge list");
       return;
    }
    for (repair = ch->first_carrying; repair; repair = repair->next_content)
    {
       if (repair->item_type == ITEM_REPAIR)
          break;
    }
    if (!repair)
    {
      send_to_char("You do not have a hammer in which to break with.\n\r", ch);
       return;
    }
    separate_obj(first_obj);
    separate_obj(repair);

    wbuf = first_obj->name;
    wbuf = one_argument(wbuf, wname);
    wbuf = one_argument(wbuf, wname);
    wname[0] = UPPER(wname[0]);
    if (!str_prefix(wname, "Fairy"))
       race = 5;
    else if (!str_prefix(wname, "Hobbit"))
       race = 4;
    else if (!str_prefix(wname, "Ogre"))
       race = 3;
    else if (!str_prefix(wname, "Dwarven"))
       race = 2;
    else if (!str_prefix(wname, "Elven"))
       race = 1;
    else if (!str_prefix(wname, "Human"))
       race = 0;
    else
    {
       bug("Invalid Race Name %s, on player %s", wname, ch->name);
       send_to_char("Error: Invalid Race Name, tell an immortal.\n\r", ch);
       return;
    }
       
    if (race == 5) //Fairy
       slabs = forge->slabnum * 4 / 10;
    else if (race == 4) //Hobbit
       slabs = forge->slabnum * 6 / 10;
    else if (race == 1) //Elf
       slabs = forge->slabnum * 85 / 100;
    else if (race == 2) //Dwarf
       slabs = forge->slabnum * 12 / 10;
    else if (race == 3) //Ogre
       slabs = forge->slabnum * 15 / 10;
    else //Human
       slabs = forge->slabnum;
       
    if (ch->race == RACE_DWARF)
       slabs = slabs * number_range(58, 63) / 100;
    else
       slabs = slabs * number_range(40, 53) / 100;
       
    if (slabs < 1)
       slabs = 1;
       
    chance = 70;
    if (ch->race == RACE_DWARF)
       chance = 95;
    if (number_range(1, 100) <= chance)
    {        
       for (scnt = 0; scnt < slabs; scnt++)
       {         
          fobj = create_object(get_obj_index(slab->vnum), 1);
          obj_to_room(fobj, ch->in_room, ch);           
       }    
       ch_printf(ch, "You pull out your handy repair hammer and begin to beat the hell out of %s.\n\r", first_obj->short_descr);
       ch_printf(ch, "You get lucky and successfully split it into %d slabs.\n\r", slabs);
       extract_obj(first_obj);
       chance = 20;
       if (ch->race == RACE_DWARF)
          chance = 5;
       if (number_range(1, 100) <= chance)
       {
          act(AT_RED, "$p starts to show some wear after its use.", ch, repair, NULL, TO_CHAR);
          if (--repair->value[0] <= 0)
          {
             act(AT_RED, "With the last strike, $p breaks and becomes useless.", ch, repair, NULL, TO_CHAR);
             separate_obj(repair);
             extract_obj(repair);
          }
       }
       return; 
    }
    else
    {
       ch_printf(ch, "You pull out your handy repair hammer and begin to beat the hell out of %s.\n\r", first_obj->short_descr);
       ch_printf(ch, "However, the only thing you get is a broken, useless %s.\n\r", first_obj->short_descr);
       extract_obj(first_obj);
       if (number_range(1, 100) <= 20)
       {
          act(AT_RED, "$p starts to show some wear after its use.", ch, repair, NULL, TO_CHAR);
          if (--repair->value[0] <= 0)
          {
             act(AT_RED, "With the last strike, $p breaks and becomes useless.", ch, repair, NULL, TO_CHAR);
             separate_obj(repair);
             extract_obj(repair);
          }
       }
    }
}
void do_fix(CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *obj;
   OBJ_DATA *repair;
   int points;
   char arg[MIL];
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  fix <item needing repaired> <item which to do the repairing>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   
   if ((obj = get_obj_carry(ch, arg)) == NULL)
   {
      if ((obj= get_obj_wear(ch, arg)) == NULL)
      {
         send_to_char("The item you are looking for is not in your inventory or being wielded.\n\r", ch);
         return;
      }
   }
   if (obj->item_type != ITEM_ARMOR && obj->item_type != ITEM_WEAPON && obj->item_type != ITEM_SHEATH)
   {
      send_to_char("You can only fix armor, weapons, and sheaths.\n\r", ch);
      return;
   }
   for (repair = ch->first_carrying; repair; repair = repair->next_content)
   {
      if (repair->item_type == ITEM_REPAIR)
         break;
   }
   if (!repair)
   {
      send_to_char("You do not have a hammer in which to repair with.\n\r", ch);
      return;
   }
   if (obj->item_type == ITEM_WEAPON && (obj->value[4] > 0  || obj->value[5] > 0))
   {
      send_to_char("There is a magical property to this weapon preventing normal repairs.\n\r", ch);
      return;
   }
   if (obj->item_type == ITEM_WEAPON)
   {
      if (obj->value[0] == INIT_ARMOR_CONDITION)
      {
         send_to_char("That item is fully repaired!\n\r", ch);
         return;
      }
   }       
   if (obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
   {
      if (obj->value[3] == INIT_ARMOR_CONDITION)
      {
         send_to_char("That item is fully repaired!\n\r", ch);
         return;
      }
   }
   if (obj->item_type == ITEM_ARMOR && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
   {
      if (obj->value[0] == obj->value[1])
      {
         send_to_char("That item is fully repaired!\n\r", ch);
         return;
      }
   }
   if (obj->item_type == ITEM_SHEATH)
   {
      if (obj->value[3] == INIT_ARMOR_CONDITION)
      {
         send_to_char("That item is fully repaired!\n\r", ch);
         return;
      }
   }
   points = POINT_LEVEL(GET_POINTS(ch, gsn_repair, 0, 1), GET_MASTERY(ch, gsn_repair, 0, 1));  
   separate_obj(obj);
   separate_obj(repair);
   if (ch->race == RACE_DWARF)
      points += 60;
   if (number_range(1, 100) <= UMIN(95, repair->value[2] + (points / 3)))
   {
      int fixed = 0;
      int repairv = number_range(70*repair->value[3], 130*repair->value[3]);
      repairv = repairv / 100;
      repairv = repairv+points/2;
      if (obj->item_type == ITEM_WEAPON)
      {
         obj->value[0] += UMAX(1, repairv);
         if (obj->value[0] >= INIT_ARMOR_CONDITION)
         {
            obj->value[0] = INIT_ARMOR_CONDITION;
            fixed = 1;
         }
      }
      if (obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         obj->value[3] += UMAX(1, repairv);
         if (obj->value[3] >= INIT_ARMOR_CONDITION)
         {
            obj->value[3] = INIT_ARMOR_CONDITION;
            fixed = 1;
         }
      }
      if (obj->item_type == ITEM_ARMOR && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
      {
         obj->value[0] += UMAX(1, repairv * obj->value[1] / 1000);
         if (obj->value[0] >= obj->value[1])
         {
            obj->value[0] = obj->value[1];
            fixed = 1;
         }
      }
      if (obj->item_type == ITEM_SHEATH)
      {
         obj->value[3] += UMAX(1, repairv);
         if (obj->value[3] >= INIT_ARMOR_CONDITION)
         {
            obj->value[3] = INIT_ARMOR_CONDITION;
            fixed = 1;
         }
      }
      if (fixed == 1)
         act(AT_GREEN, "You successfully repair $p to its perfect form.", ch, obj, NULL, TO_CHAR);
      else
         act(AT_GREEN, "You successfully repair $p.", ch, obj, NULL, TO_CHAR);
      learn_from_success(ch, gsn_repair, NULL);  
   }
   else
   {
      act(AT_DGREEN, "You fail to repair $p,", ch, obj, NULL, TO_CHAR);
      learn_from_failure(ch, gsn_repair, NULL);  
   }
   points = POINT_LEVEL(GET_POINTS(ch, gsn_repair, 0, 1), GET_MASTERY(ch, gsn_repair, 0, 1));  
   if (ch->race == RACE_DWARF)
      points += 60;
   points = 60+(points/3);
   points += URANGE(7, get_curr_lck(ch)-14, -5);
   points = URANGE(60, points, 95);
   if (number_range(1,100) > points)
   {
      act(AT_RED, "$p starts to show some wear after its use.", ch, repair, NULL, TO_CHAR);
      if (--repair->value[0] <= 0)
      {
         act(AT_RED, "With the last strike, $p breaks and becomes useless.", ch, repair, NULL, TO_CHAR);
         extract_obj(repair);
      }
   }
   return;
}  
      
void do_repair(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   char *fixstr;
   char arg1[MSL];
   char arg2[MSL];
   char *arg2p = arg2;
   char *fixstr2;
   int maxgold;

   if (argument[0] == '\0')
   {
      send_to_char("Repair what?\n\r", ch);
      return;
   }

   if ((keeper = find_fixer(ch)) == NULL)
      return;

   maxgold = 10000000;
   switch (keeper->pIndexData->rShop->shop_type)
   {
      default:
      case SHOP_FIX:
         fixstr = "repair";
         fixstr2 = "repairs";
         break;
      case SHOP_RECHARGE:
         fixstr = "recharge";
         fixstr2 = "recharges";
         break;
   }

   if (!strcmp(argument, "all"))
   {
      for (obj = ch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && can_see_obj(keeper, obj) && (obj->item_type == ITEM_ARMOR
               ||   obj->item_type == ITEM_WEAPON
               || obj->item_type == ITEM_SHEATH))
            repair_one_obj(ch, keeper, obj, argument, maxgold, fixstr, fixstr2);
      }
      return;
   }
   one_argument(argument, arg1);
   if (!str_cmp(arg1, "cost"))
      arg2p = one_argument(argument, arg1);
   else
      arg2p = argument;
   if ((obj = get_obj_carry(ch, arg2p)) == NULL)
   {
      act(AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT);
      ch->reply = keeper;
      return;
   }

   repair_one_obj(ch, keeper, obj, argument, maxgold, fixstr, fixstr2);
}

void appraise_all(CHAR_DATA * ch, CHAR_DATA * keeper, char *fixstr)
{
   OBJ_DATA *obj;
   char buf[MSL], *pbuf = buf;
   int cost = 0, total = 0;

   for (obj = ch->first_carrying; obj != NULL; obj = obj->next_content)
   {
      if (obj->wear_loc == WEAR_NONE
         && can_see_obj(ch, obj)
         && (obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON))
      {
         if (!can_drop_obj(ch, obj))
            ch_printf(ch, "You can't let go of %s.\n\r", obj->name);
         else if ((cost = get_repaircost(keeper, obj)) < 0)
         {
            if (cost != -2)
               act(AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'", keeper, obj, ch, TO_VICT);
            else
               act(AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT);
         }
         else
         {
            sprintf(buf, "$N tells you, 'It will cost %d piece%s of gold to %s %s'", cost, cost == 1 ? "" : "s", fixstr, obj->name);
            act(AT_TELL, buf, ch, NULL, keeper, TO_CHAR);
            total += cost;
         }
      }
   }
   if (total > 0)
   {
      send_to_char("\n\r", ch);
      sprintf(buf, "$N tells you, 'It will cost %d piece%s of gold in total.'", total, cost == 1 ? "" : "s");
      act(AT_TELL, buf, ch, NULL, keeper, TO_CHAR);
      strcpy(pbuf, "$N tells you, 'Remember there is a 10% surcharge for repair all.'");
      act(AT_TELL, buf, ch, NULL, keeper, TO_CHAR);
   }
}


void do_appraise(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   CHAR_DATA *keeper;
   OBJ_DATA *obj;
   int cost;
   char *fixstr;
   int level;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Appraise what?\n\r", ch);
      return;
   }

   if ((keeper = find_fixer(ch)) == NULL)
      return;

   switch (keeper->pIndexData->rShop->shop_type)
   {
      default:
      case SHOP_FIX:
         fixstr = "repair";
         break;
      case SHOP_RECHARGE:
         fixstr = "recharge";
         break;
   }

   if (!strcmp(arg, "all"))
   {
      appraise_all(ch, keeper, fixstr);
      return;
   }

   if ((obj = get_obj_carry(ch, arg)) == NULL)
   {
      act(AT_TELL, "$n tells you 'You don't have that item.'", keeper, NULL, ch, TO_VICT);
      ch->reply = keeper;
      return;
   }

   if (!can_drop_obj(ch, obj))
   {
      send_to_char("You can't let go of it.\n\r", ch);
      return;
   }

   if ((cost = get_repaircost(keeper, obj)) < 0)
   {
      if (cost != -2)
         act(AT_TELL, "$n tells you, 'Sorry, I can't do anything with $p.'", keeper, obj, ch, TO_VICT);
      else
         act(AT_TELL, "$n tells you, '$p looks fine to me!'", keeper, obj, ch, TO_VICT);
      return;
   }
   if (ch->race == RACE_HOBBIT)
   {
     cost = cost * 85 / 100;
   }
   if (!IS_NPC(ch) && LEARNED(ch, gsn_haggling) > 0)
   {
      level = POINT_LEVEL(LEARNED(ch, gsn_haggling), MASTERED(ch, gsn_haggling));
      cost = cost * (100-(level/2)) / 100;
   }
   sprintf(buf, "$N tells you, 'It will cost %d piece%s of gold to %s that...'", cost, cost == 1 ? "" : "s", fixstr);
   act(AT_TELL, buf, ch, NULL, keeper, TO_CHAR);
   if (cost > ch->gold)
      act(AT_TELL, "$N tells you, 'Which I see you can't afford.'", ch, NULL, keeper, TO_CHAR);

   return;
}


/* ------------------ Shop Building and Editing Section ----------------- */


void do_makeshop(CHAR_DATA * ch, char *argument)
{
   SHOP_DATA *shop;
   sh_int vnum;
   MOB_INDEX_DATA *mob;

   if (!argument || argument[0] == '\0')
   {
      send_to_char("Usage: makeshop <mobvnum>\n\r", ch);
      return;
   }

   vnum = atoi(argument);

   if ((mob = get_mob_index(vnum)) == NULL)
   {
      send_to_char("Mobile not found.\n\r", ch);
      return;
   }

   if (!can_medit(ch, mob))
      return;

   if (mob->pShop)
   {
      send_to_char("This mobile already has a shop.\n\r", ch);
      return;
   }

   CREATE(shop, SHOP_DATA, 1);

   LINK(shop, first_shop, last_shop, next, prev);
   shop->keeper = vnum;
   shop->profit_buy = 120;
   shop->profit_sell = 90;
   shop->open_hour = 0;
   shop->close_hour = 23;
   mob->pShop = shop;
   send_to_char("Done.\n\r", ch);
   return;
}


void do_shopset(CHAR_DATA * ch, char *argument)
{
   SHOP_DATA *shop;
   MOB_INDEX_DATA *mob, *mob2;
   char arg1[MIL];
   char arg2[MIL];
   sh_int vnum;
   int value;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Usage: shopset <mob vnum> <field> value\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("  buy0 buy1 buy2 buy3 buy4 buy sell open close keeper\n\r", ch);
      return;
   }

   vnum = atoi(arg1);

   if ((mob = get_mob_index(vnum)) == NULL)
   {
      send_to_char("Mobile not found.\n\r", ch);
      return;
   }

   if (!can_medit(ch, mob))
      return;

   if (!mob->pShop)
   {
      send_to_char("This mobile doesn't keep a shop.\n\r", ch);
      return;
   }
   shop = mob->pShop;
   value = atoi(argument);

   if (!str_cmp(arg2, "buy0"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[0] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "buy1"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[1] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "buy2"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[2] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "buy3"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[3] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "buy4"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      shop->buy_type[4] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "buy"))
   {
      if (value <= (shop->profit_sell + 5) || value > 1000)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->profit_buy = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "sell"))
   {
      if (value < 0 || value >= (shop->profit_buy - 5))
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->profit_sell = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "open"))
   {
      if (value < 0 || value > 23)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->open_hour = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "close"))
   {
      if (value < 0 || value > 23)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      shop->close_hour = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "keeper"))
   {
      if ((mob2 = get_mob_index(vnum)) == NULL)
      {
         send_to_char("Mobile not found.\n\r", ch);
         return;
      }
      if (!can_medit(ch, mob))
         return;
      if (mob2->pShop)
      {
         send_to_char("That mobile already has a shop.\n\r", ch);
         return;
      }
      mob->pShop = NULL;
      mob2->pShop = shop;
      shop->keeper = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   do_shopset(ch, "");
   return;
}


void do_shopstat(CHAR_DATA * ch, char *argument)
{
   SHOP_DATA *shop;
   MOB_INDEX_DATA *mob;
   sh_int vnum;

   if (argument[0] == '\0')
   {
      send_to_char("Usage: shopstat <keeper vnum>\n\r", ch);
      return;
   }

   vnum = atoi(argument);

   if ((mob = get_mob_index(vnum)) == NULL)
   {
      send_to_char("Mobile not found.\n\r", ch);
      return;
   }

   if (!mob->pShop)
   {
      send_to_char("This mobile doesn't keep a shop.\n\r", ch);
      return;
   }
   shop = mob->pShop;

   ch_printf(ch, "Keeper: %d  %s\n\r", shop->keeper, mob->short_descr);
   ch_printf(ch, "buy0 [%s]  buy1 [%s]  buy2 [%s]  buy3 [%s]  buy4 [%s]\n\r",
      o_types[shop->buy_type[0]], o_types[shop->buy_type[1]], o_types[shop->buy_type[2]], o_types[shop->buy_type[3]], o_types[shop->buy_type[4]]);
   ch_printf(ch, "Profit:  buy %3d%%  sell %3d%%\n\r", shop->profit_buy, shop->profit_sell);
   ch_printf(ch, "Hours:   open %2d  close %2d\n\r", shop->open_hour, shop->close_hour);
   return;
}


void do_shops(CHAR_DATA * ch, char *argument)
{
   SHOP_DATA *shop;

   if (!first_shop)
   {
      send_to_char("There are no shops.\n\r", ch);
      return;
   }

   set_char_color(AT_NOTE, ch);
   for (shop = first_shop; shop; shop = shop->next)
      ch_printf(ch, "Keeper: %5d Buy: %3d Sell: %3d Open: %2d Close: %2d Buy: %2d %2d %2d %2d %2d\n\r",
         shop->keeper, shop->profit_buy, shop->profit_sell,
         shop->open_hour, shop->close_hour, shop->buy_type[0], shop->buy_type[1], shop->buy_type[2], shop->buy_type[3], shop->buy_type[4]);
   return;
}

/* -------------- Repair Shop Building and Editing Section -------------- */


void do_makerepair(CHAR_DATA * ch, char *argument)
{
   REPAIR_DATA *repair;
   sh_int vnum;
   MOB_INDEX_DATA *mob;

   if (!argument || argument[0] == '\0')
   {
      send_to_char("Usage: makerepair <mobvnum>\n\r", ch);
      return;
   }

   vnum = atoi(argument);

   if ((mob = get_mob_index(vnum)) == NULL)
   {
      send_to_char("Mobile not found.\n\r", ch);
      return;
   }

   if (!can_medit(ch, mob))
      return;

   if (mob->rShop)
   {
      send_to_char("This mobile already has a repair shop.\n\r", ch);
      return;
   }

   CREATE(repair, REPAIR_DATA, 1);

   LINK(repair, first_repair, last_repair, next, prev);
   repair->keeper = vnum;
   repair->profit_fix = 100;
   repair->shop_type = SHOP_FIX;
   repair->open_hour = 0;
   repair->close_hour = 23;
   mob->rShop = repair;
   send_to_char("Done.\n\r", ch);
   return;
}


void do_repairset(CHAR_DATA * ch, char *argument)
{
   REPAIR_DATA *repair;
   MOB_INDEX_DATA *mob, *mob2;
   char arg1[MIL];
   char arg2[MIL];
   sh_int vnum;
   int value;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Usage: repairset <mob vnum> <field> value\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("  fix0 fix1 fix2 profit type open close keeper\n\r", ch);
      return;
   }

   vnum = atoi(arg1);

   if ((mob = get_mob_index(vnum)) == NULL)
   {
      send_to_char("Mobile not found.\n\r", ch);
      return;
   }

   if (!can_medit(ch, mob))
      return;

   if (!mob->rShop)
   {
      send_to_char("This mobile doesn't keep a repair shop.\n\r", ch);
      return;
   }
   repair = mob->rShop;
   value = atoi(argument);

   if (!str_cmp(arg2, "fix0"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      repair->fix_type[0] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "fix1"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      repair->fix_type[1] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "fix2"))
   {
      if (!is_number(argument))
         value = get_otype(argument);
      if (value < 0 || value > MAX_ITEM_TYPE)
      {
         send_to_char("Invalid item type!\n\r", ch);
         return;
      }
      repair->fix_type[2] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "profit"))
   {
      if (value < 1 || value > 1000)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      repair->profit_fix = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "type"))
   {
      if (value < 1 || value > 2)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      repair->shop_type = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "open"))
   {
      if (value < 0 || value > 23)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      repair->open_hour = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "close"))
   {
      if (value < 0 || value > 23)
      {
         send_to_char("Out of range.\n\r", ch);
         return;
      }
      repair->close_hour = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "keeper"))
   {
      if ((mob2 = get_mob_index(vnum)) == NULL)
      {
         send_to_char("Mobile not found.\n\r", ch);
         return;
      }
      if (!can_medit(ch, mob))
         return;
      if (mob2->rShop)
      {
         send_to_char("That mobile already has a repair shop.\n\r", ch);
         return;
      }
      mob->rShop = NULL;
      mob2->rShop = repair;
      repair->keeper = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   do_repairset(ch, "");
   return;
}


void do_repairstat(CHAR_DATA * ch, char *argument)
{
   REPAIR_DATA *repair;
   MOB_INDEX_DATA *mob;
   sh_int vnum;

   if (argument[0] == '\0')
   {
      send_to_char("Usage: repairstat <keeper vnum>\n\r", ch);
      return;
   }

   vnum = atoi(argument);

   if ((mob = get_mob_index(vnum)) == NULL)
   {
      send_to_char("Mobile not found.\n\r", ch);
      return;
   }

   if (!mob->rShop)
   {
      send_to_char("This mobile doesn't keep a repair shop.\n\r", ch);
      return;
   }
   repair = mob->rShop;

   ch_printf(ch, "Keeper: %d  %s\n\r", repair->keeper, mob->short_descr);
   ch_printf(ch, "fix0 [%s]  fix1 [%s]  fix2 [%s]\n\r", o_types[repair->fix_type[0]], o_types[repair->fix_type[1]], o_types[repair->fix_type[2]]);
   ch_printf(ch, "Profit: %3d%%  Type: %d\n\r", repair->profit_fix, repair->shop_type);
   ch_printf(ch, "Hours:   open %2d  close %2d\n\r", repair->open_hour, repair->close_hour);
   return;
}


void do_repairshops(CHAR_DATA * ch, char *argument)
{
   REPAIR_DATA *repair;

   if (!first_repair)
   {
      send_to_char("There are no repair shops.\n\r", ch);
      return;
   }

   set_char_color(AT_NOTE, ch);
   for (repair = first_repair; repair; repair = repair->next)
      ch_printf(ch, "Keeper: %5d Profit: %3d Type: %d Open: %2d Close: %2d Fix: %2d %2d %2d\n\r",
         repair->keeper, repair->profit_fix, repair->shop_type,
         repair->open_hour, repair->close_hour, repair->fix_type[0], repair->fix_type[1], repair->fix_type[2]);
   return;
}
