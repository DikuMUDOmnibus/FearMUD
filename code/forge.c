/* FORGE SYSTEM */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include "mud.h"

void do_forge	args ( ( CHAR_DATA *ch, char *argument ) );
void ore_alter	args ( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *slab ) );

char *const weapon_race[MAX_RACE] = 
{
   "Human", "Elven", "Dwarven", "Ogre", "Hobbit", "Fairy"
};

int get_forge_type(char *argument)
{
   char buf[MSL];

   sprintf(buf, argument);
   
   if (!str_prefix(buf, "Swords"))
      return 0;
   if (!str_prefix(buf, "Axes"))
      return 1;
   if (!str_prefix(buf, "PoleArms"))
      return 2;
   if (!str_prefix(buf, "Blunt"))
      return 3;
   if (!str_prefix(buf, "Staves"))
      return 4;
   if (!str_prefix(buf, "Daggers"))
      return 5;
   if (!str_prefix(buf, "Chest"))
      return 6;
   if (!str_prefix(buf, "Arms"))
      return 7;
   if (!str_prefix(buf, "Legs"))
      return 8;
   if (!str_prefix(buf, "Head"))
      return 9;
   if (!str_prefix(buf, "Neck"))
      return 10;
   if (!str_prefix(buf, "Shield"))
      return 11;
   if (!str_prefix(buf, "Projectile"))
      return 12;
   return -1;
}

char *get_forge_type_name(int type)
{
   switch(type)
   {
      default:
         return "Unknown";
      case 0:
         return "Swords";
      case 1:
         return "Axes";
      case 2:
         return "PoleArms";
      case 3:
         return "Blunt";
      case 4:
         return "Staves";
      case 5:
         return "Daggers";
      case 6:
         return "Chest";
      case 7:
         return "Arms";
      case 8:
         return "Legs";
      case 9:
         return "Head";
      case 10:
         return "Neck";
      case 11:
         return "Shield";
      case 12:
         return "Projectile";
   }
}

void alter_forge_obj(CHAR_DATA *ch, OBJ_DATA *fobj, OBJ_DATA *first_obj, SLAB_DATA *slab)
{
   char rstr1[MIL];
   char rstr2[MIL];
   char race[20];
   char rstr3[MIL];
   char objname[MIL];
   char buf[MIL];
   
   if(fobj->item_type == ITEM_WEAPON || (fobj->item_type == ITEM_ARMOR && IS_SET(fobj->wear_flags, ITEM_WEAR_SHIELD)) ||
      fobj->item_type == ITEM_PROJECTILE)
   {
      if(slab->adj[0] == 'A' || slab->adj[0] == 'a' || slab->adj[0] == 'E' || slab->adj[0] == 'E' || 
      slab->adj[0] == 'I' || slab->adj[0] == 'i' || slab->adj[0] == 'O' || slab->adj[0] == 'o' || 
      slab->adj[0] == 'U' || slab->adj[0] == 'U')
      {
         sprintf(rstr1, "An");
	 sprintf(rstr2, "Someone has left their");
	 sprintf(rstr3, "here.");
      }
      else
      {
         sprintf(rstr1, "A");
	 sprintf(rstr2, "Someone has left their");
	 sprintf(rstr3, "here");
      }
   }
   else if(fobj->item_type == ITEM_ARMOR)
   {
      if (fobj->short_descr[0] == 'S' || fobj->short_descr[0] == 's')
         sprintf(rstr1, "Some");
      else
      {
         if(slab->adj[0] == 'A' || slab->adj[0] == 'a' || slab->adj[0] == 'E' || slab->adj[0] == 'E' || 
         slab->adj[0] == 'I' || slab->adj[0] == 'i' || slab->adj[0] == 'O' || slab->adj[0] == 'o' || 
         slab->adj[0] == 'U' || slab->adj[0] == 'U')
         {
            sprintf(rstr1, "An");
         }
         else
         {
            sprintf(rstr1, "A");
         }
      }
      sprintf(rstr2, "Someone has left their");
      sprintf(rstr3, "here!");
   }
   sprintf(race, weapon_race[ch->race]);
   sprintf(objname, fobj->name);
   objname[0] = UPPER(objname[0]);
   STRFREE(fobj->name);
   sprintf(buf, "%s %s %s", slab->adj, race, objname);
   fobj->name = STRALLOC(buf);
   STRFREE(fobj->short_descr);
   sprintf(buf, "%s %s %s %s", rstr1, slab->adj, race, objname);
   fobj->short_descr = STRALLOC(buf);
   STRFREE(fobj->description);
   sprintf(buf, "%s %s %s %s %s", rstr2, slab->adj, race, objname, rstr3);
   fobj->description = STRALLOC(buf);	
   if(fobj->item_type == ITEM_WEAPON && !strcmp(slab->adj, "ainalear"))
   {
        xTOGGLE_BIT(fobj->extra_flags, ITEM_SANCTIFIED);
   }
   if(!strcmp(slab->adj, "adamant"))
   {
        xTOGGLE_BIT(fobj->extra_flags, ITEM_NOBREAK);
   }
   ore_alter(ch, fobj, first_obj);
}

//alter an object in your inventory to change it to a forge item
void do_forgealter(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *nobj;
   OBJ_DATA *sobj;
   SLAB_DATA *slab;
   int cnt = 1;
   int race;
   int oldrace;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: forgealter <obj name> <ore type> [race value]\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if ((obj = get_obj_here(ch, arg1)) == NULL)
   {
      ch_printf(ch, "You cannot seem to find %s", arg1);
      return;
   }
   if (!xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
   {
      send_to_char("That item is not forgeable.\n\r", ch);
      return;
   } 
   if (isdigit(arg2[0]))
   {
      for (slab = first_slab; slab ; slab = slab->next) 
      {
         if (atoi(arg2) == cnt)
            break;
         cnt++;
      }
      if (!slab)
      {
         send_to_char("There is no such ore (type forge ores)\n\r", ch);
         return;
      }      
   }
   else
   {
      send_to_char("You need to specify a number, use forge ores to get such a number.\n\r", ch);
      return;
   }
   if (argument[0] != '\0')
   {
      race = get_npc_race(argument);
      if (race < 0)
         race = atoi(argument);
      if (race < 0 || race >= MAX_RACE)
      {
         send_to_char("That is not a valid race, you can only use PC races (ex: Human, Elf, Dwarf)\n\r", ch);
         return;
      }
      oldrace = ch->race;
   }
   else
   {
      oldrace = ch->race;
      race = ch->race;
   }
   //execute change...     
   ch->race = race;
   separate_obj(obj);
   obj_from_char(obj);
   nobj = create_object(get_obj_index(obj->pIndexData->vnum), 1);
   obj_to_char(nobj, ch);
   extract_obj(obj);
   sobj = create_object(get_obj_index(slab->vnum), 1);
   alter_forge_obj(ch, nobj, sobj, slab);	
   extract_obj(sobj);
   ch->race = oldrace;
   send_to_char("Done.\n\r", ch);
   return;
}

char *const kingdoms_race[MAX_RACE] = 
{
   "Human", "Elven", "Dwarven", "Ogre", "Hobbit", "Fairy"
};

int is_kingdom_race(OBJ_DATA *obj, CHAR_DATA *ch)
{
   char *wbuf;
   char wname[100];

   wbuf = obj->name;
   wbuf = one_argument(wbuf, wname);
   wbuf = one_argument(wbuf, wname);
   
   if (!str_cmp(wname, kingdoms_race[kingdom_table[ch->pcdata->hometown]->race]))
      return 1;
   else
      return 0;
}

void do_depository(CHAR_DATA *ch, char *argument)
{
   DEPO_ORE_DATA *dore;
   DEPO_WEAPON_DATA *dweapon;
   DEPO_WEAPON_DATA *dsweapon;
   DEPO_WEAPON_DATA *d1weapon = NULL;
   DEPO_WEAPON_DATA *dsearch;
   OBJ_INDEX_DATA *iobj;
   OBJ_INDEX_DATA *oobj;
   SLAB_DATA *slab;
   OBJ_DATA *obj;
   OBJ_DATA *sobj;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char buf[70];
   char buf2[100];
   char line[150];
   char *name;
   int cnt = 0;
   int first = 0;
   int num = 0;
   int last = 0;
   OBJ_DATA *slabobj;
   
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mindepository)  
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
     
   if (!kingdom_table[ch->pcdata->hometown]->first_ore)
   {
      bug("Kingdom number %d does not have a first ore.", ch->pcdata->hometown);
      send_to_char("There has been an error with this command, tell an immortal.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to use this command.\n\r", ch);
      return;
   }
   if (ch->coord->x != ch->pcdata->town->barracks[0] || ch->coord->y != ch->pcdata->town->barracks[1]
   ||  ch->map != ch->pcdata->town->barracks[2])
   {
      send_to_char("You can only do this in the barracks room.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mindepository)  
      {
         send_to_char("Syntax: depository give <item/ore>\n\r", ch);
         send_to_char("Syntax: depository give inventory\n\r", ch);
      }
      else
      {    
         send_to_char("Syntax: depository save\n\r", ch);
         send_to_char("Syntax: depository all\n\r", ch);
         send_to_char("Syntax: depository item <item name>\n\r", ch);
         send_to_char("Syntax: depository ore <ore name>\n\r", ch);
         send_to_char("Syntax: depository slab [slab name]\n\r", ch);
         send_to_char("Syntax: depository give <item/ore>\n\r", ch);
         send_to_char("Syntax: depository give inventory\n\r", ch);
         send_to_char("Syntax: depository take ore <name> [number]\n\r", ch);
         send_to_char("Syntax: depository take item <ore> <name> [number]\n\r", ch);
         return;
      }
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   dsweapon = kingdom_table[ch->pcdata->hometown]->first_ore->first_weapon;
   
   if (!str_cmp(arg1, "give"))
   {
      if (!str_cmp(arg2, "inventory"))
      {
         for (obj = ch->first_carrying; obj; obj = sobj)
         {
            sobj = obj->next_content;
            if (xIS_SET(obj->extra_flags, ITEM_FORGEABLE) && (obj->wear_loc == WEAR_NONE) && is_kingdom_race(obj, ch))
            {
               sprintf(buf2, "give %s", obj->name);
               do_depository(ch, buf2);
            }
            for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
            {
               if (dore->vnum == obj->pIndexData->vnum)
               {
                  sprintf(buf2, "give %s", obj->name);
                  do_depository(ch, buf2);
               }
            }   
         }
         return;  
      }
      else
      {
         for (obj = ch->first_carrying; obj; obj = obj->next_content)
         {
            if (!str_cmp(arg2, obj->name))
               break;
         }
         
         for (obj = ch->first_carrying; obj; obj = obj->next_content)
         {
            if (nifty_is_name(arg2, obj->name))
               break;   
         }
         
         for (obj = ch->first_carrying; obj; obj = obj->next_content)
         {
            if (nifty_is_name_prefix(arg2, obj->name))   
               break;
         }
         
         if (!obj)
         {
            send_to_char("You are not carrying that.\n\r", ch);
            return;
         }
         if (obj->wear_loc != WEAR_NONE)
         {
            send_to_char("You can only put items that you are not wearing in the depo.\n\r", ch);
            return;
         }
         if (xIS_SET(obj->extra_flags, ITEM_FORGEABLE) && (obj->wear_loc == WEAR_NONE) && !is_kingdom_race(obj, ch))
         {
            send_to_char("That object is not of the same race of your kingdom.\n\r", ch);
            return;
         }
         for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
         {
            if (dore->vnum == obj->pIndexData->vnum) //slab
            {
               ch_printf(ch, "You add [%d] %s%s to the depository.\n\r", obj->count, obj->name, obj->count > 1 ? "s" : "");
               dore->count += obj->count;
               write_depo_list();
               extract_obj(obj);
               return;
            }
         }
         if (!xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
         {
            send_to_char("That item is not a forge item.\n\r", ch);
            return;
         }  
         if (obj->item_type == ITEM_WEAPON)
         {
            if (obj->value[0] != 1000)
            {
               send_to_char("You can only place completed repaired weapons in the depo.\n\r", ch);
               return;
            }
         }
         if (obj->item_type == ITEM_ARMOR)
         {
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
            {
               if (obj->value[0] != obj->value[1])
               {
                  send_to_char("You can only place completed repaired shields in the depo.\n\r", ch);
                  return;
               }
            }
            else
            {
               if (obj->value[3] != 1000)
               {
                  send_to_char("You can only place completed repaired shields in the depo.\n\r", ch);
                  return;
               }
            }
         }           
         for(slab = first_slab; slab; slab = slab->next)
         {
            if(is_name(slab->adj, obj->name) )
               break;
         }
         if (!slab)
         {
            send_to_char("There has been an error with that item, tell an immortal.\n\r", ch);
            return;
         }
         for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
         {
            if (dore->vnum == slab->vnum)
               break;
         }
         if (!dore)
         {
            send_to_char("There has been an error, tell an immortal.\n\r", ch);
            bug("depository give has a problem with an item %s has not being on the depo ore list.", ch->name);
            return;
         }
         for (dweapon = dore->first_weapon; dweapon; dweapon = dweapon->next)
         {
            if (dweapon->vnum == obj->pIndexData->vnum)
            {
               ch_printf(ch, "You add [%d] %s%s to the depository.\n\r", obj->count, obj->name, obj->count > 1 ? "s" : "");
               dweapon->count += obj->count;
               write_depo_list();
               extract_obj(obj);
               return;
            }
         }
         if (!dweapon)
         {
            send_to_char("There has been an error, tell an immortal.\n\r", ch);
            bug("depository give has a problem with an item %s has not being on the depo weapon list.", ch->name);
            return;
         }      
      }      
   }
   
   if (!str_cmp(arg1, "save"))
   {
      write_depo_list();
      send_to_char("Done.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg1, "take"))
   {
      argument = one_argument(argument, arg3);
      
      if (!str_cmp(arg2, "item"))
      {
         argument = one_argument(argument, arg4);
         
         for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
         {
            oobj = get_obj_index(dore->vnum);
            
            if (!oobj)
            {
               send_to_char("There has been an error, tell an immortal.\n\r", ch);
               bug("Ore %d does not exist, but it is in the depo ore list.\n\r", dore->vnum);
               return;
            }
            if (nifty_is_name(arg3, oobj->name))
            {
               for (slab = first_slab; slab; slab = slab->next)
               {
                  if (slab->vnum == dore->vnum)
                     break;
               }
               if (!slab)
               {
                  send_to_char("There has been an error, tell an immortal.\n\r", ch);
                  bug("Ore %d does not exist in the actual slab list.", dore->vnum);
                  return;
               }
               for (dweapon = dore->first_weapon; dweapon; dweapon = dweapon->next)
               {
                  int oldrace;
                  iobj = get_obj_index(dweapon->vnum);
                  
                  if (!iobj)
                  {
                     send_to_char("There has been an error, tell an immortal.\n\r", ch);
                     bug("Weapon %d does not exist, but it is in the depo weapon list.\n\r", dore->vnum);
                     return;
                  }
                  if (!str_cmp(arg4, iobj->name))
                  {                  
                     if (atoi(argument) > 0)
                         num = atoi(argument);
                     else
                         num = 1;
                  
                     if (num > dweapon->count)
                     {
                         ch_printf(ch, "You only have %d of those.\n\r", dweapon->count);
                         return;
                     }
                     
                     ch_printf(ch, "You take [%d] %s %s%s from the depository.\n\r", num, slab->adj, iobj->name, num > 1 ? "s" : "");
               
                     for (cnt = 1; cnt <= num; cnt++)
                     {           
                        obj = create_object(iobj, 1);
                        oldrace = ch->race;
                        ch->race = kingdom_table[ch->pcdata->hometown]->race;
                        slabobj = create_object(get_obj_index(slab->vnum), 1);
                        alter_forge_obj(ch, obj, slabobj, slab); 
                        ch->race = oldrace;
                        extract_obj(slabobj);
                        obj_to_char(obj, ch); 
                     }
                     dweapon->count -= num;
                     write_depo_list();
                     return;
                  }
               }
               send_to_char("That is not an actual item with that name.\n\r", ch);
               return;
            }
         }
         send_to_char("There is no ore with that name.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "ore"))
      {
         for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
         {
            iobj = get_obj_index(dore->vnum);
            
            if (!iobj)
            {
               send_to_char("There has been an error, tell an immortal.\n\r", ch);
               bug("Ore %d does not exist, but it is in the depo ore list.\n\r", dore->vnum);
               return;
            }
            if (nifty_is_name(arg3, iobj->name))
            {
               if (atoi(argument) > 0)
                  num = atoi(argument);
               else
                  num = 1;
                  
               if (num > dore->count)
               {
                  ch_printf(ch, "You only have %d of those.\n\r", dore->count);
                  return;
               }
                  
               ch_printf(ch, "You take [%d] %s%s from the depository.\n\r", num, iobj->name, num > 1 ? "s" : "");
               
               for (cnt = 1; cnt <= num; cnt++)
               {           
                  obj = create_object(iobj, 1);
                  obj_to_char(obj, ch); 
               }
               dore->count -= num;
               write_depo_list();
               return;
            }
         }
         send_to_char("There is no ore with that name.\n\r", ch);
         return;
      }
   } 
                 
   if (!str_cmp(arg1, "slab"))
   {
      if (arg2[0] != '\0')
      {
         for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
         {
            iobj = get_obj_index(dore->vnum);
            if (!iobj)
            {
               send_to_char("There has been an error, tell an immortal.\n\r", ch);
               bug("Vnum %d in the ore list of a forge does not exist.", dore->vnum);
               return;
            }
            if (nifty_is_name(arg2, iobj->name))   
               break;
         }
         if (!dore)
         {
            send_to_char("That is not a valid ore.\n\r", ch);
            return;
         }
         ch_printf(ch, "Ore Name         Slab");        
         send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
         iobj = get_obj_index(dore->vnum);
         name = iobj->name;
         name = one_argument(name, buf);   
         buf[0] = UPPER(buf[0]); 
         ch_printf(ch, "%-15s  %d\n\r", buf, dore->count);
         return;
      }
      else
      {
         ch_printf(ch, "Ore Name         Slab");        
         send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
         for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
         {
            iobj = get_obj_index(dore->vnum);
            if (!iobj)
            {
               send_to_char("There has been an error, tell an immortal.\n\r", ch);
               bug("Vnum %d in the ore list of a forge does not exist.", dore->vnum);
               return;
            }      
            iobj = get_obj_index(dore->vnum);
            name = iobj->name;
            name = one_argument(name, buf); 
            buf[0] = UPPER(buf[0]); 
            ch_printf(ch, "%-15s  %d\n\r", buf, dore->count);        
         }
         return;
      }
   }
   if (!str_cmp(arg1, "ore"))
   {
      for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
      {
         iobj = get_obj_index(dore->vnum);
         if (!iobj)
         {
            send_to_char("There has been an error, tell an immortal.\n\r", ch);
            bug("Vnum %d in the ore list of a forge does not exist.", dore->vnum);
            return;
         }
         if (nifty_is_name(arg2, iobj->name))   
            break;
      }
      if (!dore)
      {
         send_to_char("That is not a valid ore.\n\r", ch);
         return;
      }
      for (;;)
      { 
         cnt = 0;
         sprintf(line, "Ore Name         ");    
         if (first == 0)
         {
            cnt++;
            sprintf(buf2, "Slabs                 ");
            strcat(line, buf2);
         }
         for (dweapon = dsweapon; dweapon; dweapon = dweapon->next)
         {        
            if (cnt == 4)
            {
               d1weapon = dsweapon; //first on the list
               dsweapon = dweapon;  //last on the list
               send_to_char(line, ch);
               send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
               break;
            }                     
            iobj = get_obj_index(dweapon->vnum);
            sprintf(buf, iobj->name);
            sprintf(buf2, "%-20s  ", buf);
            strcat(line, buf2);
            cnt++;
            if (dweapon->next == NULL)
            {
               d1weapon = dsweapon; //first on the list
               dsweapon = NULL;  //last on the list
               send_to_char(line, ch);
               send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
               last = 1;
               break;
            }          
         }
         iobj = get_obj_index(dore->vnum);
         name = iobj->name;
         name = one_argument(name, buf);   
         sprintf(line, "%-15s  ", buf);
         line[0] = UPPER(line[0]);
         if (first == 0)
         {
            sprintf(buf, "%-20d  ", dore->count);
            strcat(line, buf);
         } 
         for (dsearch = dore->first_weapon; dsearch->vnum != d1weapon->vnum; dsearch = dsearch->next)
         {
            ;
         }
         if (dsearch)
         {
            dweapon = dsearch;
         }
         if (dsweapon != NULL)
         {
            for (dsearch = dore->first_weapon; dsearch->vnum != dsweapon->vnum; dsearch = dsearch->next)
            {
               ;
            }
            if (dsearch)
            {
               dsweapon = dsearch;
            }
         }
         for (;; dweapon = dweapon->next)
         {
            if (dweapon && dsweapon)
            {            
               if (dweapon->vnum == dsweapon->vnum)
                  break;
            }
            if (!dsweapon)
            {
               if (dweapon == NULL)
                  break;
            }
                   
            sprintf(buf, "%-20d  ", dweapon->count);            
            strcat(line, buf);
         }
         send_to_char(line, ch);
         send_to_char("\n\r\n\r", ch);
         if (last == 1)
            break;   
         if (first == 0)
            first = 1;
      }
      return;   
   }     
   if (!str_cmp(arg1, "item"))
   {
      for (dweapon = dsweapon; dweapon; dweapon = dweapon->next)
      {
         iobj = get_obj_index(dweapon->vnum);
         if (!iobj)
         {
            send_to_char("There has been an error, tell an immortal.\n\r", ch);
            bug("Vnum %d in the weapon list of a forge does not exist.", dweapon->vnum);
            return;
         }
         if (nifty_is_name(arg2, iobj->name))
         {
            sprintf(line, "Ore Name         ");
            sprintf(buf, iobj->name);
            sprintf(buf2, "%-20s  ", buf);
            strcat(line, buf2);
            send_to_char(line, ch);
            send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
            for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
            {
               for (dsweapon = dore->first_weapon; dsweapon; dsweapon = dsweapon->next)
               {
                  if (dsweapon->vnum == dweapon->vnum)
                  {
                      iobj = get_obj_index(dore->vnum);
                      name = iobj->name;
                      name = one_argument(name, buf);   
                      sprintf(line, "%-15s  ", buf);
                      line[0] = UPPER(line[0]);    
                      sprintf(buf, "%-20d  ", dsweapon->count);            
                      strcat(line, buf);
                      send_to_char(line, ch);
                      send_to_char("\n\r", ch);
                   }
                }
            }
            return;
         }
      }
      for (dweapon = dsweapon; dweapon; dweapon = dweapon->next)
      {
         iobj = get_obj_index(dweapon->vnum);
         if (!iobj)
         {
            send_to_char("There has been an error, tell an immortal.\n\r", ch);
            bug("Vnum %d in the weapon list of a forge does not exist.", dweapon->vnum);
            return;
         }
         if (nifty_is_name_prefix(arg2, iobj->name))
         {
            sprintf(line, "Ore Name         ");
            sprintf(buf, iobj->name);
            sprintf(buf2, "%-20s  ", buf);
            strcat(line, buf2);
            send_to_char(line, ch);
            send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
            for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
            {
               iobj = get_obj_index(dore->vnum);
               name = iobj->name;
               name = one_argument(name, buf);   
               sprintf(line, "%-15s  ", buf);
               line[0] = UPPER(line[0]);    
               sprintf(buf, "%-20d  ", dweapon->count);            
               strcat(line, buf);
               send_to_char(line, ch);
               send_to_char("\n\r", ch);
            }
            return;
         }
      }
      send_to_char("That is not an item that is in the depository.\n\r", ch);
      return;
   }           
   if (!str_cmp(arg1, "all"))
   {
      for (;;)
      { 
         cnt = 0;
         sprintf(line, "Ore Name         ");    
         if (first == 0)
         {
            cnt++;
            sprintf(buf2, "Slabs                 ");
            strcat(line, buf2);
         }
         for (dweapon = dsweapon; dweapon; dweapon = dweapon->next)
         {        
            if (cnt == 4)
            {
               d1weapon = dsweapon; //first on the list
               dsweapon = dweapon;  //last on the list
               send_to_char(line, ch);
               send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
               break;
            }                     
            iobj = get_obj_index(dweapon->vnum);
            sprintf(buf, iobj->name);
            sprintf(buf2, "%-20s  ", buf);
            strcat(line, buf2);
            cnt++;
            if (dweapon->next == NULL)
            {
               d1weapon = dsweapon; //first on the list
               dsweapon = NULL;  //last on the list
               send_to_char(line, ch);
               send_to_char("\n\r----------------------------------------------------------------------------------------------------\n\r", ch);
               last = 1;
               break;
            }          
         }
         for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
         {
            iobj = get_obj_index(dore->vnum);
            name = iobj->name;
            name = one_argument(name, buf);   
            sprintf(line, "%-15s  ", buf);
            line[0] = UPPER(line[0]);
            if (first == 0)
            {
               sprintf(buf, "%-20d  ", dore->count);
               strcat(line, buf);
            }   
            for (dsearch = dore->first_weapon; dsearch->vnum != d1weapon->vnum; dsearch = dsearch->next)
            {
               ;
            }
            if (dsearch)
            {
               dweapon = dsearch;
            }
            if (dsweapon != NULL)
            {
               for (dsearch = dore->first_weapon; dsearch->vnum != dsweapon->vnum; dsearch = dsearch->next)
               {
                  ;
               }
               if (dsearch)
               {
                  dsweapon = dsearch;
               }
            }
            for (;; dweapon = dweapon->next)
            {
               if (dweapon && dsweapon)
               {            
                  if (dweapon->vnum == dsweapon->vnum)
                     break;
               }
               if (!dsweapon)
               {
                  if (dweapon == NULL)
                     break;
               }
                      
               sprintf(buf, "%-20d  ", dweapon->count);            
               strcat(line, buf);
            }
            send_to_char(line, ch);
            send_to_char("\n\r", ch);
         }
         if (last == 1)
            break;
         if (first == 0)
            first = 1;
         send_to_char("\n\r", ch);    
      }   
      return;     
   }
   do_depository(ch, "");
   return;
}

void do_forge(CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *first_obj, *second_obj;
   OBJ_INDEX_DATA *wobj;
   OBJ_DATA *fobj;
   FORGE_DATA *forge;
   SLAB_DATA *slab;	
   int x;
   int race;
   int slabs;
   int cnt;
   int sln; //race slab nums	
   char *wbuf;
   char wname[MIL]; //for stripping to check race
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
	
   slabs = 0;
   sln= 0;
   first_obj=NULL;
	
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
	
   if(arg1[0] == '\0')
   {
      send_to_char( "Syntax: forge slab <ore #>\n\r", ch);
      send_to_char( "Syntax: forge create <ore #> <weapon #>\n\r", ch);
      send_to_char( "Syntax: forge break <weapon/armor name>\n\r", ch);
      send_to_char( "Syntax: forge list [type] (to see a list of forgeable types)\n\r", ch);
      send_to_char( "Syntax: forge ores (to see a list of ore slabs that can be done here.\n\r", ch);
      send_to_char( "Syntax: forge appraise <weapon/armor name>\n\r", ch);
      send_to_char( "Syntax: forge sell <ore #>\n\r", ch);
      return;
   }
	
   if (!str_cmp(arg1, "ores"))
   {
      cnt = 1;
      send_to_char("Ores.\n\r--------------------\n\r", ch);
      for (slab = first_slab; slab ; slab = slab->next)  
      {	
         ch_printf(ch, "&c&w[%-2d] &G&W%-20s\n\r", cnt, slab->adj);
         cnt++;
      }
      return;
   }
   if (!str_cmp(arg1, "list"))
   {	
      FORGE_DATA *oforge;
      int type = -1;
      int num = 0;
      int maxcnt = 3; //to break this into two groups.
      cnt = 1;
      if (arg2[0] != '\0')
      {
         type = get_forge_type(arg2);
         if (type == -1)
         {
            send_to_char("That is not a valid type, type forge list to get an idea.\n\r", ch);
            return;
         }
      }
      for (forge = first_forge; forge; forge = forge->next)
      {
         maxcnt++;
      }
      for (oforge = first_forge; oforge; oforge = oforge->next)
      {
         num++;
         if (maxcnt / 2 == num)
            break;
      }

      send_to_char("Item Name           Slabs Slots Hand Type      Item Name           Slabs Slots Hand Type\n\r----------------------------------------------------------------------------------------------\n\r", ch);
      for (forge = first_forge; forge; forge = forge->next)
      {
         int needslabs;
         wobj = get_obj_index(forge->vnum);
         
         if (ch->race == 5) //Fairy
            needslabs = forge->slabnum * 4 / 10;
         else if (ch->race == 4) //Hobbit
            needslabs = forge->slabnum * 6 / 10;
         else if (ch->race == 1) //Elf
            needslabs = forge->slabnum * 85 / 100;
         else if (ch->race == 2) //Dwarf
            needslabs = forge->slabnum *  12 / 10;
         else if (ch->race == 3) //Ogre
            needslabs = forge->slabnum * 15 / 10;
         else
            needslabs = forge->slabnum; //Human
            
         if (needslabs < 1)
            needslabs = 1;
         
         if (type >= 0)
         {
            if (type == forge->type)
            {
               ch_printf(ch, "&c&w[" MXPTAG("forge '%d'") "%-2d" MXPTAG("/forge") "] &G&W%-16s  &c&R%-2d  &c&z%-2d   &c&w%-3s  &C%-8s  \n\r", 
                  cnt, cnt, wobj->name, needslabs, wobj->imbueslots, xIS_SET(wobj->extra_flags, ITEM_TWOHANDED) ? "TWO" : "ONE", get_forge_type_name(forge->type));
            }
            cnt++;
            continue;
         }
         else
         {          
            ch_printf(ch, "&c&w[" MXPTAG("forge '%d'") "%-2d" MXPTAG("/forge") "] &G&W%-16s  &c&R%-2d  &c&z%-2d   &c&w%-3s  &C%-8s  ", 
               cnt, cnt, wobj->name, needslabs, wobj->imbueslots, xIS_SET(wobj->extra_flags, ITEM_TWOHANDED) ? "TWO" : "ONE", get_forge_type_name(forge->type));
         }
         cnt++;
         //do the other side of the list now, evilness
         if (oforge)
         {
            wobj = get_obj_index(oforge->vnum);
         
            if (ch->race == 5) //Fairy
               needslabs = oforge->slabnum * 4 / 10;
            else if (ch->race == 4) //Hobbit
               needslabs = oforge->slabnum * 6 / 10;
            else if (ch->race == 1) //Elf
               needslabs = oforge->slabnum * 85 / 100;
            else if (ch->race == 2) //Dwarf
               needslabs = oforge->slabnum *  12 / 10;
            else if (ch->race == 3) //Ogre
               needslabs = oforge->slabnum * 15 / 10;
            else
               needslabs = oforge->slabnum; //Human
            
            if (needslabs < 1)
               needslabs = 1;

            ch_printf(ch, "&c&w[" MXPTAG("forge '%d'") "%-2d" MXPTAG("/forge") "] &G&W%-16s  &c&R%-2d  &c&z%-2d   &c&w%-3s  &C%-10s\n\r", 
               num, num, wobj->name, needslabs, wobj->imbueslots, xIS_SET(wobj->extra_flags, ITEM_TWOHANDED) ? "TWO" : "ONE", get_forge_type_name(oforge->type));
            num++;
            oforge = oforge->next;
            if (!oforge && maxcnt % 2 == 1)
               return;
         }
         else
         {
            send_to_char("\n\r", ch);
            return;
         }
      }
      return;
   }
   if (!str_prefix(arg1, "slabs"))
   {
      cnt = 1;
      
      if (!isdigit(arg2[0]))
      {
         send_to_char("Need to pick a value for the slab (type forge ores)\n\r", ch);
         return;
      } 
      
      for (slab = first_slab; slab ; slab = slab->next) 
      {
         if (atoi(arg2) == cnt)
            break;
         cnt++;
      }
      if (!slab)
      {
         send_to_char("There is no such ore (type forge ores)\n\r", ch);
         return;
      }
      
      for(first_obj = ch->first_carrying; first_obj; first_obj = first_obj->next_content)
      {
         if (slab->vnum-1 == first_obj->pIndexData->vnum)
            break;
      }     
      if(first_obj == NULL)
      {
         send_to_char("You don't have any ore of that type...\n\r", ch);
         return;
      }     
      for(second_obj = ch->first_carrying; second_obj; second_obj = second_obj->next_content)
      {
         if (IS_OBJ_STAT(second_obj, ITEM_COAL))
            break;
      }
      if (!second_obj)
      {
         send_to_char("You need some charcoal to start the fire.\n\r", ch);
         return;
      }
      if(!xIS_SET(ch->in_room->room_flags, ROOM_FORGEROOM) && !wIS_SET(ch, ROOM_FORGEROOM) )
      {
         send_to_char( "You need to find a forge first!\n\r", ch);
   	 return;
      }  	 
      if (number_range(1, 100) <= 70)
      {
         wobj = get_obj_index(first_obj->pIndexData->vnum+1);
         wobj->vnum = first_obj->pIndexData->vnum+1;
         fobj = (create_object(wobj, ch->level));
         separate_obj(first_obj);
         extract_obj(first_obj);
         separate_obj(second_obj);
         extract_obj(second_obj);
   	 
         obj_to_room(fobj, ch->in_room, ch);
         REMOVE_BIT(fobj->wear_flags, ITEM_TAKE);
         add_obj_timer(fobj, TIMER_COOLING, 2, NULL, 0);
         send_to_char("You drop the ore into the forges crucible and\n\rr", ch);
         send_to_char("add in the charcoal. Then, lighting the forge,\n\r", ch);
         send_to_char("you proceed to watch the metal melt, then start\n\r", ch);
         send_to_char("to bubble. You start to work the bellows, and the\n\r", ch);
         send_to_char("metal and charcoal react with the air... a large\n\r", ch);
         send_to_char("cloud of steam rises and when it clears a glowing\n\r", ch);
         send_to_char("slab of metal sits in front of you. It is definately\n\r", ch);
         send_to_char("to hot to touch... you better wait till it cools.\n\r", ch);
         return; 	   
      }
      else
      {
         separate_obj(first_obj);
         extract_obj(first_obj);
         separate_obj(second_obj);
         extract_obj(second_obj);
         send_to_char("You drop the ore into the forges crucible and\n\r", ch);
         send_to_char("add in the charcoal. Then, lighting the forge,\n\r", ch);
         send_to_char("you proceed to watch the metal melt, then start\n\r", ch);
         send_to_char("to bubble. You start to work the bellows, and the\n\r", ch);
         send_to_char("metal and charcoal react with the air... a large\n\r", ch);
         send_to_char("cloud of steam rises and when it clears yout notice\n\r", ch);
         send_to_char("the slab is did not shape properply.  The blacksmith laughs\n\r", ch);
         send_to_char("and removes the slab and throws it somewhere in the back.\n\r", ch);
         return;    
      }
   }
   if(!str_prefix(arg1, "appraise"))
   {
       char objname[MSL];
       char *pobjname = objname;
       char objarg[MSL];
       
       for(first_obj = ch->first_carrying; first_obj; first_obj = first_obj->next_content)
       {
          if( is_name(arg2, first_obj->name) && xIS_SET(first_obj->extra_flags, ITEM_FORGEABLE)
          &&  first_obj->wear_loc == WEAR_NONE )
             break;
       }        
       if (!first_obj)
       {
          send_to_char("You do not have that in your inventory.\n\r", ch);
          return;
       }       
       if (first_obj->item_type == ITEM_PROJECTILE)
       {
          send_to_char("You cannot break down arrows.\n\r", ch);
          return;
       }

       for (forge = first_forge; forge; forge = forge->next)
       {
          if (forge->vnum == first_obj->pIndexData->vnum)
             break;
       }
       sprintf(objname, first_obj->name);
       pobjname = one_argument(pobjname, objarg);
       pobjname = one_argument(pobjname, objarg);
       if (!forge && first_obj->pIndexData->vnum >= START_STATICQUEST_VNUM && first_obj->pIndexData->vnum <= END_STATICQUEST_VNUM)
       {
          for (forge = first_forge; forge; forge = forge->next)
          {
             if (!str_cmp(forge->name, pobjname))
                break;
          }
       }
       if (!forge)
       {
          send_to_char("Error:  Error with the weapon/armor, tell an immotal.\n\r", ch);
          bug("Forge(appraise): Could not find the weapon/armor in the forge list");
          return;
       }
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
          
       slabs = slabs * .66;
       if (slabs < 1)
          slabs = 1;
          
       ch_printf(ch, "The Forge will produce %d slabs for %s\n\r", slabs, first_obj->short_descr);
       return;
   }  
   if(!str_prefix(arg1, "break"))
   { 
       int scnt;
       char objname[MSL];
       char *pobjname = objname;
       char objarg[MSL];
       
       for(first_obj = ch->first_carrying; first_obj; first_obj = first_obj->next_content)
       {
          if( is_name(arg2, first_obj->name) && xIS_SET(first_obj->extra_flags, ITEM_FORGEABLE)
          &&  first_obj->wear_loc == WEAR_NONE )
             break;
       }        
       if (!first_obj)
       {
          send_to_char("You do not have that in your inventory.\n\r", ch);
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
       sprintf(objname, first_obj->name);
       pobjname = one_argument(pobjname, objarg);
       pobjname = one_argument(pobjname, objarg);
       for (forge = first_forge; forge; forge = forge->next)
       {
          if (forge->vnum == first_obj->pIndexData->vnum)
             break;
       }
       if (!forge && first_obj->pIndexData->vnum >= START_STATICQUEST_VNUM && first_obj->pIndexData->vnum <= END_STATICQUEST_VNUM)
       {
          for (forge = first_forge; forge; forge = forge->next)
          {
             if (!str_cmp(forge->name, pobjname))
                break;
          }
       }
       if (!forge)
       {
          send_to_char("Error:  Error with the weapon/armor, tell an immotal.\n\r", ch);
          bug("Forge: Could not find the weapon/armor in the forge list");
          return;
       }
       if( !xIS_SET(ch->in_room->room_flags, ROOM_FORGEROOM) && !wIS_SET(ch, ROOM_FORGEROOM) )
       {
          send_to_char( "You need to find a forge first!\n\r", ch);
   	      return;
       } 
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
          
       slabs = slabs * .66;
       if (slabs < 1)
          slabs = 1;
       
       for (scnt = 0; scnt < slabs; scnt++)
       {         
          fobj = create_object(get_obj_index(slab->vnum), 1);
          obj_to_room(fobj, ch->in_room, ch);           
       }    
       separate_obj(first_obj);
       extract_obj(first_obj);
       send_to_char("You hand over the item to the blacksmith.  He smiles and starts hammering\n\r", ch);
       send_to_char("away at the metal trying to break it down into slabs.  He does a pretty good\n\r", ch);
       send_to_char("job of breaking it down, but there was a few metal pieces that went flying in\n\r", ch);
       send_to_char("in all directions.  The rest is on the ground waiting for you to pick it up.\n\r", ch); 
       return; 
   }
   if(!str_prefix(arg1, "Sell"))
   {
      OBJ_DATA *sobj;
      int gold;
      if (!isdigit(arg2[0]))
      {
         send_to_char("Need to pick a value for the slab (type forge ores)\n\r", ch);
         return;
      }
      if (atoi(arg3) == 0)
      {
          sprintf(arg3, "1");
      }
      cnt = 1;
      for (slab = first_slab; slab ; slab = slab->next) 
      {
         if (atoi(arg2) == cnt)
            break;
         cnt++;
      }
      if (!slab)
      {
         send_to_char("There is no such ore (type forge ores)\n\r", ch);
         return;
      }    
      if( !xIS_SET(ch->in_room->room_flags, ROOM_FORGEROOM) && !wIS_SET(ch, ROOM_FORGEROOM))
      {
         send_to_char( "You need to find a forge first!\n\r", ch);
   	 return;
      } 
      for(sobj = ch->first_carrying; sobj; sobj = sobj->next_content)
      {
         if (slab->vnum == sobj->pIndexData->vnum)
         {
            slabs += sobj->count;
            first_obj = sobj;
         }
      }
      for(sobj = ch->in_room->first_content; sobj; sobj = sobj->next_content)
      {
         if (slab->vnum == sobj->pIndexData->vnum)
         {
            slabs += sobj->count;
            first_obj = sobj;
         }
      }  
      if (!first_obj)
      {
         send_to_char("You don't have any slabs of that type here.\n\r", ch);
         return;
      } 
      if (slabs < atoi(arg3))
      {
         sprintf(arg3, "%d", slabs);
      }
      gold = atoi(arg3) * first_obj->cost/10;
      sln = atoi(arg3);
      
      //check in inventory first
      for(sobj = ch->first_carrying; sobj; sobj = sobj->next_content)
      {
         if (slab->vnum == sobj->pIndexData->vnum && sln > 0)
         {
            if (sln >= sobj->count)
            {
               sln -= sobj->count;
               extract_obj(sobj);
            }
            else
            {
               sobj->count -= sln;
               sln = 0;
            }
         }
      }    
      //check on the ground
      if (sln > 0)
      {
         for (sobj = ch->in_room->first_content; sobj; sobj = sobj->next_content)
         {
            if (slab->vnum == sobj->pIndexData->vnum && sln > 0)
            {
               if (sln >= sobj->count)
               {
                  sln -= sobj->count;
                  extract_obj(sobj);
               }
               else
               {
                  sobj->count -= sln;
                  sln = 0;
               }
            }
         }
      }    
      ch_printf(ch, "You quickly hand over %d slabs to the blacksmith and received %d gold in return.\n\r", atoi(arg3), gold);
      ch->gold += gold;
      return;
   }
   if(!str_prefix(arg1, "Create"))
   {
      int icnt = 0;
      OBJ_DATA *sobj;
      if (!isdigit(arg2[0]))
      {
         send_to_char("Need to pick a value for the slab (type forge ores)\n\r", ch);
         return;
      } 
      cnt = 1;
      for (slab = first_slab; slab ; slab = slab->next) 
      {
         if (atoi(arg2) == cnt)
            break;
         cnt++;
      }
      if (!slab)
      {
         send_to_char("There is no such ore (type forge ores)\n\r", ch);
         return;
      }
      if (!isdigit(arg3[0]))
      {
         send_to_char("Need to pick a value for the weapon (type forge weapons)\n\r", ch);
         return;
      } 
      cnt = 1;
      for (forge = first_forge; forge; forge = forge->next)
      {
         if (atoi(arg3) == cnt)
            break;
         cnt++;
      }
      if (!forge)
      {
         send_to_char("There is no such weapon (type forge weapons)\n\r", ch);
         return;
      }
      for(sobj = ch->first_carrying; sobj; sobj = sobj->next_content)
      {
         if (slab->vnum == sobj->pIndexData->vnum)
         {
            slabs += sobj->count;
            first_obj = sobj;
         }
      }      
      for(sobj = ch->in_room->first_content; sobj; sobj = sobj->next_content)
      {
         if (slab->vnum == sobj->pIndexData->vnum)
         {
            first_obj = sobj;
            slabs += sobj->count;
         }
      }        
      if (!first_obj)
      {
         send_to_char("You don't have any slabs of that type here.\n\r", ch);
         return;
      }
       
      if( !xIS_SET(ch->in_room->room_flags, ROOM_FORGEROOM) && !wIS_SET(ch, ROOM_FORGEROOM))
      {
         send_to_char( "You need to find a forge first!\n\r", ch);
   	 return;
      }  
       
      if (ch->race == 5) //Fairy
         sln = forge->slabnum * 4 / 10;
      else if (ch->race == 4) //Hobbit
         sln = forge->slabnum * 6 / 10;
      else if (ch->race == 1) //Elf
         sln = forge->slabnum * 85 / 100;
      else if (ch->race == 2) //Dwarf
         sln = forge->slabnum * 12 / 10;
      else if (ch->race == 3) //Ogre
         sln = forge->slabnum * 15 / 10;
      else //Human
         sln = forge->slabnum;
        
      if (sln < 1)
         sln = 1;
         
      if (slabs < sln)
      {
         send_to_char("You do not have enough slabs to do that!!\n\r", ch);
         return;
      }           	
      if( !xIS_SET(ch->in_room->room_flags, ROOM_FORGEROOM) && !wIS_SET(ch, ROOM_FORGEROOM))
      {
  	 send_to_char("You need to find a forge first!\n\r", ch);
  	 return;
      }
      
      //check in inventory first
      for(sobj = ch->first_carrying; sobj; sobj = sobj->next_content)
      {
         if (slab->vnum == sobj->pIndexData->vnum && sln > 0)
         {
            if (sln >= sobj->count)
            {
               sln -= sobj->count;
               extract_obj(sobj);
            }
            else
            {
               icnt += sln;
               sobj->count -= sln;
               sln = 0;
            }
         }
      }    
      //check on the ground
      if (sln > 0)
      {
         for (sobj = ch->in_room->first_content; sobj; sobj = sobj->next_content)
         {
            if (slab->vnum == sobj->pIndexData->vnum && sln > 0)
            {
               if (sln >= sobj->count)
               {
                  sln -= sobj->count;
                  extract_obj(sobj);
               }
               else
               {
                  sobj->count -= sln;
                  sln = 0;
               }
            }
         }
      }    
      //1 ore - 10 arrows
      if (forge->type == 12) //Projectiles
      {
         for (x = 1; x <= 10; x++)
         {
            fobj = create_object(get_obj_index(forge->vnum), 1);
            alter_forge_obj(ch, fobj, first_obj, slab);
            obj_to_room(fobj, ch->in_room, ch);
            REMOVE_BIT(fobj->wear_flags, ITEM_TAKE);
            add_obj_timer(fobj, TIMER_COOLING, number_range(5, 8), NULL, 0); //20 seconds to 32 seconds
         }
      }
      else
      {
         fobj = create_object(get_obj_index(forge->vnum), 1);
         alter_forge_obj(ch, fobj, first_obj, slab);
         obj_to_room(fobj, ch->in_room, ch);
         REMOVE_BIT(fobj->wear_flags, ITEM_TAKE);
         add_obj_timer(fobj, TIMER_COOLING, number_range(5, 8), NULL, 0); //20 seconds to 32 seconds
      }
      send_to_char("You hand over the slabs over to the Blacksmith and he smiles as he shoves\n\r", ch);
      send_to_char("the slabs above the fire to begin to heat.  It only takes a few moments and\n\r", ch);
      send_to_char("the heat quickly makes the metal glow red, softly at first, then brighter.\n\r", ch);
      send_to_char("Soon it is bendable and the Blacksmith bends it into shapes and drops it before\n\r", ch);
      send_to_char("your feet to cool.\n\r", ch);
      return;	
   }	
   do_forge(ch, "");	
}

void ore_alter( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *slab )
{
	if( ch == NULL )
	{
		bug("ore_alter: ch = NULL.", 0);
		return;
	}
	if( obj == NULL )
	{
		bug("ore_alter: obj = NULL.", 0);
		return;
	}
	if( slab == NULL )
	{
		bug("ore_alter: obj = NULL.", 0);
		return;
	}
	obj->sworthrestrict = slab->value[7];
	if(obj->item_type == ITEM_ARMOR && IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
	{
	   obj->value[1] = obj->pIndexData->value[1] * slab->value[0] ; //Bash armor mod
	   if (obj->value[1] < obj->pIndexData->value[1])
	      obj->value[1] = obj->pIndexData->value[1];
	   obj->value[0] = obj->value[1];
	   obj->cost = obj->pIndexData->cost *  slab->value[6] / 100;
       obj->weight = UMAX(.01,obj->pIndexData->weight * (float)slab->value[4] / 100);
           
        if(ch->race == 5)
		{
			obj->weight = obj->weight * .4;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 4)
		{
			obj->weight = obj->weight * .6;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 1)
		{
			obj->weight = obj->weight * .85;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 0)
		{
			//essentially do nothing... this is just here in case
			//we change human base stats
			obj->weight = obj->weight * 1;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 2)
		{
			obj->weight = obj->weight * 1.2;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 3)
		{
			obj->weight = obj->weight * 1.5;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
        }
	if(obj->item_type == ITEM_ARMOR && !IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
	{
		obj->value[0] = obj->pIndexData->value[0] + slab->value[0]; //bashmod
		obj->value[1] = obj->pIndexData->value[1] + slab->value[1]; //slashmod
		obj->value[2] = obj->pIndexData->value[2] + slab->value[2]; //stabmod
		obj->value[4] = obj->pIndexData->value[4] + slab->value[5]; //durability
		obj->cost = obj->pIndexData->cost *  slab->value[6] / 100;
		obj->weight = UMAX(.01,obj->pIndexData->weight * (float)slab->value[4] / 100);
		
		if(ch->race == 5)
		{
			obj->weight = obj->weight * .4;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 4)
		{
			obj->weight = obj->weight * .6;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 1)
		{
			obj->weight = obj->weight * .85;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 0)
		{
			//essentially do nothing... this is just here in case
			//we change human base stats
			obj->weight = obj->weight * 1;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 2)
		{
			obj->weight = obj->weight * 1.2;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 3)
		{
			obj->weight = obj->weight * 1.5;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
	}
	if(obj->item_type == ITEM_PROJECTILE)
	{
	   obj->value[1] = obj->pIndexData->value[1] + slab->value[3]; //dam mod
	   obj->value[2] = obj->pIndexData->value[2] + slab->value[3]; //dam mod
	   obj->value[9] = obj->pIndexData->value[9] + slab->value[2]; //stab mod
       obj->weight = UMAX(.01,obj->pIndexData->weight * (float)slab->value[4] / 100); //weight mod
   	   obj->cost = obj->pIndexData->cost *  slab->value[6] / 100;
	   //An arrow is an arrow for the time being....
    }
	if(obj->item_type == ITEM_WEAPON)
	{
		obj->value[1] = obj->pIndexData->value[1] + slab->value[3]; //dam mod
		obj->value[2] = obj->pIndexData->value[2] + slab->value[3]; //dam mod
		obj->value[3] = race_table[ch->race]->weaponmin + obj->pIndexData->value[3];
		obj->value[7] = obj->pIndexData->value[7] + slab->value[0]; //bash mod
		obj->value[8] = obj->pIndexData->value[8] + slab->value[1]; //slash mod
		obj->value[9] = obj->pIndexData->value[9] + slab->value[2]; //stab mod
		obj->weight = UMAX(1, obj->pIndexData->weight * (float)slab->value[4] / 100); //weight mod
		obj->cost = obj->pIndexData->cost *  slab->value[6] / 100;
                obj->value[10] = obj->pIndexData->value[10] + slab->value[5]; //weapon durability
	
		if(ch->race == 5)
		{
			obj->value[1] = obj->value[1] * .7;
			obj->value[2] = obj->value[2] * .7;
			if (obj->value[1] < 1)
			   obj->value[1] = 1;
			if (obj->value[2] < 2)
			   obj->value[2] = 2;
			obj->weight = obj->weight * .5;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 4)
		{
			obj->value[1] = obj->value[1] * .8;
			obj->value[2] = obj->value[2] * .8;
			if (obj->value[1] < 1)
			   obj->value[1] = 1;
			if (obj->value[2] < 2)
			   obj->value[2] = 2;
			obj->weight = obj->weight * .65;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 1)
		{
			obj->value[1] = obj->value[1] * .9;
			obj->value[2] = obj->value[2] * .9;
			if (obj->value[1] < 1)
			   obj->value[1] = 1;
			if (obj->value[2] < 2)
			   obj->value[2] = 2;
			obj->weight = obj->weight * .85;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 0)
		{
			//essentially do nothing... this is just here in case
			//we change human base stats
		}
		if(ch->race == 2)
		{
		        obj->value[1] = obj->value[1] * 1.1;
			obj->value[2] = obj->value[2] * 1.1;
			if (obj->value[1] < 1)
			   obj->value[1] = 1;
			if (obj->value[2] < 2)
			   obj->value[2] = 2;
			obj->weight = obj->weight * 1.2;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
		if(ch->race == 3)
		{
		        obj->value[1] = obj->value[1] * 1.3;
			obj->value[2] = obj->value[2] * 1.3;
			if (obj->value[1] < 1)
			   obj->value[1] = 1;
			if (obj->value[2] < 2)
			   obj->value[2] = 2;
			obj->weight = obj->weight * 1.5;
			if (obj->weight < .01)
			   obj->weight = .01;
		}
	}
}


