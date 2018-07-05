//Part of Rafermand Code
/* Treasure system created by Skan with Xerves help -- 1/4/01	*/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/* Dummy obj define */

#define DUMMY 1

void do_gem args((CHAR_DATA * ch, char *argument));

bool tchance args((int num));

bool tchance(int num)
{
   if (number_range(1, 100) <= num)
      return TRUE;
   else
      return FALSE;
}

OBJ_DATA *generate_tbox(CHAR_DATA *ch)
{
   BOX_DATA *box;
   OBJ_DATA *tbox;
   OBJ_INDEX_DATA *obj;
   int goldamt;
   int x;
   int lvl;
   int i;

   obj = NULL;

   // Check to see which a box is generated 

   tbox = create_object(get_obj_index(DUMMY), 0);

   x = number_range(1, box_num);
   i = 0;

   for (box = first_box; box; box = box->next)
   {
      i++;
      if (i == x)
      {
         obj = get_obj_index(box->vnum);
         break;
      }
   }

   if (obj == NULL)
   {
      bug("No box was created");
      return NULL;
   }
   else
      tbox = create_object(obj, 1);

   if (IN_WILDERNESS(ch))
   {
      lvl = ch->m1;
      
      if (lvl >0)
      {
         int lckmod;
         goldamt = number_range(50*lvl*lvl, 300*lvl*lvl);
         lckmod = (get_curr_lck(ch) - 14) * 4;
         if (number_range(1, 100) < lckmod)
            goldamt *= number_range(2, 4);
         obj_to_obj(create_money(goldamt), tbox);
      }
   }

   return tbox;
}

OBJ_DATA *generate_treasure(CHAR_DATA * mob)
{
   GEM_DATA *gem;
   OBJ_DATA *treas;
   OBJ_INDEX_DATA *obj;
   int x;
   int i;
   int lckmod;
   int lvl;

   obj = NULL;
   treas = NULL;
   lvl = mob->m1;
   lckmod = (get_curr_lck(mob) - 14) * 4;
   
   lckmod += 8*(lvl-1);


   x = number_range(1, gem_num);
   i = 0;

   for (gem = first_gem; gem; gem = gem->next)
   {
      i++;
      if (i == x)
      {
         obj = get_obj_index(gem->vnum);
         break;
      }
   }

   if (obj == NULL)
   {
      bug("No gems are loaded, using default");
      return create_object(get_obj_index(sysdata.gem_vnum), 1);
   }
   else
   {
      if (gem->rarity >1)
      {
         if (gem->rarity == 2)
         {
            if (tchance(10+lckmod))
               return create_object(obj, 1);
            else
               return create_object(get_obj_index(sysdata.gem_vnum), 1);
         }
         if (gem->rarity == 3)
         {
            if (tchance(lckmod))
               return create_object(obj, 1);
            else
               return create_object(get_obj_index(sysdata.gem_vnum), 1);
         }
         if (gem->rarity == 4)
         {
            if (tchance(lckmod/2))
               return create_object(obj, 1);
            else
               return create_object(get_obj_index(sysdata.gem_vnum), 1);
         }
      }
      return create_object(obj, 1);
   }
   treas = create_object(obj, 1);
   return treas;
}


void do_gem(CHAR_DATA * ch, char *argument)
{
   GEM_DATA *gem;
   char buf[MSL];
   char arg[MIL];
   CHAR_DATA *gemcutter;
   OBJ_DATA *obj;
   int cost;
   OBJ_INDEX_DATA *ogem;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: gem <list/sell> [item]", ch);
      return;
   }
   argument = one_argument(argument, arg);

   /* Checks for a character in the room with spec_gemcutter set. This special
      procedure must be defined in special.c. You could instead use an
      ACT_GEMCUTTER flag instead of a special procedure. */

   for (gemcutter = ch->in_room->first_person; gemcutter; gemcutter = gemcutter->next_in_room)
   {
      if (!IS_NPC(gemcutter))
         continue;
      if (gemcutter->spec_fun == spec_lookup("spec_gemcutter"))
         break;
   }

   if (!gemcutter)
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }

   if (gemcutter->position == POS_FIGHTING)
   {
      send_to_char("Wait until the fighting stops.\n\r", ch);
      return;
   }

   if (!first_gem)
   {
      send_to_char("No gems exist, tell an immortal!\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "list"))
   {
      for (gem = first_gem; gem; gem = gem->next)
      {
         ogem = get_obj_index(gem->vnum);
         ch_printf(ch, "&G&W%-6d	&c&w%-40s\n\r", ogem->cost, ogem->short_descr);
      }
      send_to_char("If you have a gem to sell me, use gem sell <obj>.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "sell"))
   {
      if ((obj = get_obj_carry(ch, argument)) == NULL)
      {
         act(AT_TELL, "$n tells you 'You don't have that item.'", gemcutter, NULL, ch, TO_VICT);
         ch->reply = gemcutter;
         return;
      }
      if (!can_drop_obj(ch, obj))
      {
         send_to_char("You can't let go of it!\n\r", ch);
         return;
      }
      /* Prevents giving of special items (rare items) -- Xerves 3/24/98 */
      if (IS_OBJ_STAT(obj, ITEM_NOGIVE))
      {
         send_to_char("Xerves should personally smack you for trying to find a loophole.  Cannot sell this item.\n\r", ch);
         return;
      }
      if (obj->timer > 0)
      {
         act(AT_TELL, "$n tells you, '$p is depreciating in value too quickly...'", gemcutter, obj, ch, TO_VICT);
         return;
      }
      cost = 0;
      for (gem = first_gem; gem; gem = gem->next)
      {
         if (obj->pIndexData->vnum == gem->vnum)
         {
            cost = number_range(gem->cost * .8, gem->cost * 1.2);
            break;
         }
      }

      if (cost <= 0)
      {
         act(AT_ACTION, "$n looks uninterested in $p.", gemcutter, obj, ch, TO_VICT);
         return;
      }

      separate_obj(obj);
      act(AT_ACTION, "$n sells $p.", ch, obj, NULL, TO_ROOM);
      sprintf(buf, "You sell $p for %d gold piece%s.", cost, cost == 1 ? "" : "s");
      act(AT_ACTION, buf, ch, obj, NULL, TO_CHAR);

      ch->gold += cost;
      obj_from_char(obj);
      return;
   }
   do_gem(ch, "");
   return;
}
