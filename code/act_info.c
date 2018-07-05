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
 *			     Informational module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "mud.h"



char *const where_name[] = {
   "<used as light>      ",
   "<worn on finger>     ",
   "<worn on finger>     ",
   "<worn on neck>       ",
   "<worn around neck>   ",
   "<worn on body>       ",
   "<worn on head>       ",
   "<worn on left leg>   ",
   "<worn on right leg>  ",
   "<worn on left arm>   ",
   "<worn on right arm>  ",
   "<worn as shield>     ",
   "<worn about waist>   ",
   "<wielded>            ",
   "<dual wielded>       ",
   "<missile wielded>    ",
   "<lodged in ribs>     ",
   "<lodged in arm>      ",
   "<lodged in leg>      ",
   "<nocked>             ",
   "<worn on back>       "
};


/*
 * Local functions.
 */
void show_char_to_char_0 args((CHAR_DATA * victim, CHAR_DATA * ch));
void show_char_to_char_1 args((CHAR_DATA * victim, CHAR_DATA * ch, int type));
void show_char_to_char args((CHAR_DATA * list, CHAR_DATA * ch));
void show_condition args((CHAR_DATA * ch, CHAR_DATA * victim));
//Similar Helpfile Snippet Declarations
sh_int str_similarity( const char *astr, const char *bstr );
sh_int str_prefix_level( const char *astr, const char *bstr );
void similar_help_files(CHAR_DATA *ch, char *argument);

int showcollapse;

/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing				-Thoric
 */
HELP_DATA *get_help(CHAR_DATA * ch, char *argument)
{
   char argall[MIL];
   char argone[MIL];
   char argnew[MIL];
   HELP_DATA *pHelp;
   int lev;

   if (argument[0] == '\0')
      argument = "summary";

   if (isdigit(argument[0]))
   {
      lev = number_argument(argument, argnew);
      argument = argnew;
   }
   else
      lev = -2;
   /*
    * Tricky argument handling so 'help a b' doesn't match a.
    */
   argall[0] = '\0';
   while (argument[0] != '\0')
   {
      argument = one_argument(argument, argone);
      if (argall[0] != '\0')
         strcat(argall, " ");
      strcat(argall, argone);
   }

   for (pHelp = first_help; pHelp; pHelp = pHelp->next)
   {
      if (pHelp->level > get_trust(ch))
         continue;
      if (lev != -2 && pHelp->level != lev)
         continue;

      if (is_name(argall, pHelp->keyword))
         return pHelp;
   }

   return NULL;
}

char *format_obj_to_char(OBJ_DATA * obj, CHAR_DATA * ch, bool fShort, bool colors)
{
   static char buf[MSL];
   sh_int dam;
   sh_int color;

   color = AT_OBJECT;
   buf[0] = '\0';
   if (colors == TRUE)
   {
      switch (obj->item_type)
      {
         default:
            color = AT_OBJECT;
            break;
         case ITEM_BLOOD:
            color = AT_BLOOD;
            break;
         case ITEM_MONEY:
         case ITEM_TREASURE:
            color = AT_YELLOW;
            break;
         case ITEM_COOK:
         case ITEM_FOOD:
            color = AT_HUNGRY;
            break;
         case ITEM_DRINK_CON:
         case ITEM_FOUNTAIN:
            color = AT_THIRSTY;
            break;
         case ITEM_FIRE:
            color = AT_FIRE;
            break;
         case ITEM_SCROLL:
         case ITEM_WAND:
         case ITEM_STAFF:
         case ITEM_SPELLBOOK:
            color = AT_MAGIC;
            break;
      }
   }
   set_char_color(color, ch);
   switch (obj->item_type)
   {
      default:
         strcat(buf, "&c&w* ");
         strcat(buf, char_color_str(color, ch));
         break;

      case ITEM_ARMOR:
         dam = obj->value[3];
         if (IS_OBJ_STAT(obj, ITEM_CLOAK))
         {
            strcat(buf, "&c&w* ");
            strcat(buf, char_color_str(color, ch));
            break;
         }
         if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
         {
            if (obj->value[1] == 0)
               dam = 100;
            else
               dam = obj->value[0] * 1000 / obj->value[1];
         }
         if (dam <= 199)
            strcat(buf, "&r* ");
         if (dam >= 200 && dam <= 399)
            strcat(buf, "&R* ");
         if (dam >= 400 && dam <= 599)
            strcat(buf, "&Y* ");
         if (dam >= 600 && dam <= 799)
            strcat(buf, "&B* ");
         if (dam >= 800)
            strcat(buf, "&P* ");
         strcat(buf, char_color_str(color, ch));
         break;

      case ITEM_WEAPON:
      case ITEM_SHEATH:
         if (obj->item_type == ITEM_WEAPON)
            dam = obj->value[0];
         else
            dam = obj->value[3];
         if (dam <= 199)
            strcat(buf, "&r* ");
         if (dam >= 200 && dam <= 399)
            strcat(buf, "&R* ");
         if (dam >= 400 && dam <= 599)
            strcat(buf, "&Y* ");
         if (dam >= 600 && dam <= 799)
            strcat(buf, "&B* ");
         if (dam >= 800)
            strcat(buf, "&P* ");
         strcat(buf, char_color_str(color, ch));
         break;

   }

   if (IS_OBJ_STAT(obj, ITEM_INVIS))
      strcat(buf, "(Invis) ");
   if (IS_AFFECTED(ch, AFF_DETECT_EVIL))
      strcat(buf, "(Red Aura) ");

   if (IS_AFFECTED(ch, AFF_DETECT_MAGIC) && IS_OBJ_STAT(obj, ITEM_MAGIC))
      strcat(buf, "(Magical) ");
   if (IS_OBJ_STAT(obj, ITEM_GLOW))
      strcat(buf, "(Glowing) ");
   if (IS_OBJ_STAT(obj, ITEM_HUM))
      strcat(buf, "(Humming) ");
   if (IS_OBJ_STAT(obj, ITEM_HIDDEN))
      strcat(buf, "(Hidden) ");
   if (IS_OBJ_STAT(obj, ITEM_BURIED))
      strcat(buf, "(Buried) ");
   if (IS_IMMORTAL(ch) && IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
      strcat(buf, "(PROTO) ");
   if (IS_AFFECTED(ch, AFF_DETECTTRAPS) && is_trapped(obj))
      strcat(buf, "(Trap) ");
   if (IS_OBJ_STAT(obj, ITEM_NORESET) && IS_IMMORTAL(ch))
      strcat(buf, "(NORESET) ");
   if (IS_OBJ_STAT(obj, ITEM_TIMERESET) && IS_IMMORTAL(ch))
      strcat(buf, "(TIMERESET) ");

   if (fShort)
   {
      if (obj->short_descr)
         strcat(buf, obj->short_descr);  
   }
   else
   {
      if (obj->description && obj->item_type != ITEM_CORPSE_PC)
         strcat(buf, obj->description);
      if (obj->description && obj->item_type == ITEM_CORPSE_PC)
      {
         char arg[MIL];
         char *argh;
         argh = obj->short_descr;
         argh = one_argument(argh, arg);
         argh = one_argument(argh, arg);
         argh = one_argument(argh, arg);
         argh = one_argument(argh, arg);
         if (!str_cmp(arg, ch->name))
         {
            strcat(buf, "Your corpse lies here rotting away.");
         }
         else
         {
            strcat(buf, obj->description);
         }
      }
   }

   return buf;
}

OBJ_DATA *get_wear_hidden_cloak(CHAR_DATA *ch)
{
   OBJ_DATA *obj;
   
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
   {
      if (obj->wear_loc == WEAR_BACK && IS_OBJ_STAT(obj, ITEM_CLOAK) && IS_OBJ_STAT(obj, ITEM_HIDEIDENTITY))
         return obj;
   }
   return NULL;
}

OBJ_DATA *get_wear_cloak(CHAR_DATA *ch)
{
   OBJ_DATA *obj;
   
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
   {
      if (obj->wear_loc == WEAR_BACK && IS_OBJ_STAT(obj, ITEM_CLOAK))
         return obj;
   }
   return NULL;
}

void do_cloak(CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *cloak;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: hood <on/off>\n\r", ch);
      return;
   }
   if ((cloak = get_wear_cloak(ch)) == NULL)
   {
      send_to_char("You are not wearing a cloak.\n\r", ch);
      return;
   }
   if (ch->position < POS_SITTING)
   {
      send_to_char("You can only pull your hood on/off if you are sitting/standing/mounted.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "on"))
   {
      if (xIS_SET(cloak->extra_flags, ITEM_HIDEIDENTITY))
      {
         send_to_char("It is already on.\n\r", ch);
         return;
      }
      act(AT_ACTION, "$n pulls the hood up on $p to cover $s face", ch, cloak, NULL, TO_ROOM);
      act(AT_ACTION, "You pull up the hood on your $p to cover your face", ch, cloak, NULL, TO_CHAR);
      xSET_BIT(cloak->extra_flags, ITEM_HIDEIDENTITY);
      return;
   }
   if (!str_cmp(argument, "off"))
   {
      if (!xIS_SET(cloak->extra_flags, ITEM_HIDEIDENTITY))
      {
         send_to_char("It is already off.\n\r", ch);
         return;
      }
      xREMOVE_BIT(cloak->extra_flags, ITEM_HIDEIDENTITY);
      act(AT_ACTION, "$n pulls down the hood on $p to reveil $s face", ch, cloak, NULL, TO_ROOM);
      act(AT_ACTION, "You pull down the hood on your $p to reveil your face", ch, cloak, NULL, TO_CHAR);
      return;
   }
   do_cloak(ch, "");
   return;
}

char *show_pers_title(CHAR_DATA *victim, CHAR_DATA *ch)
{
   if (!str_cmp(show_pers_output(victim, ch, 1, -1), victim->name))
      return victim->pcdata->title;
   else
      return "";
}

char *show_pers_pretitle(CHAR_DATA *victim, CHAR_DATA *ch)
{
   if (!str_cmp(show_pers_output(victim, ch, 1, -1), victim->name))
      return victim->pcdata->pretit;
   else
      return "";
}
      

char *show_pers_output(CHAR_DATA *ch, CHAR_DATA *looker, int type, int kingdom)
{
   char name[50];
   char hair[50];
   char height[50];
   char who[50];
   static char fullname[150];
   OBJ_DATA *cloak;
   INTRO_DATA *intro;
   strcpy(fullname, "");
   if (ch != NULL && looker != NULL && ch == looker)
   {
      if (IS_NPC(ch))
         return ch->short_descr;
      else
      {
         strcpy(fullname, "");
         sprintf(fullname, "%s%s %s", ch->pcdata->pretit, ch->name, ch->last_name);
         return &fullname[0];
      }
   }
   if (type == 0)
   {
      if (!can_see(looker, ch))
         return "someone";
   }
   if (type == 1 || type == 3)
   {
      if (!can_see_map(looker, ch))
         return "someone";
   }
   if (looker && IS_NPC(looker))
      return ch->name;
   if (type !=  2)
   {
      if (IS_NPC(ch) && type != 3)
         return ch->short_descr;
      if (IS_NPC(ch) && type == 3)
         return ch->name;
      if (IS_NPC(looker) && type != 3)
         return ch->short_descr;
      if (IS_NPC(looker) && type == 3)
         return ch->name;
         
      if ((!IS_NPC(looker) && xIS_SET(looker->act, PLR_SHOWNAMES)) || (!IS_NPC(ch) && xIS_SET(ch->act, PLR_UKNOWN)))
      {
         if (IS_NPC(ch))
            return ch->short_descr;
         else
         {
            strcpy(fullname, "");
            sprintf(fullname, "%s%s %s", ch->pcdata->pretit, ch->name, ch->last_name);
            return &fullname[0];
         }
      }
         
      if ((cloak = get_wear_hidden_cloak(ch)) != NULL)
      {
         return "a cloaked individual";
      }
            
      for (intro = looker->pcdata->first_introduction; intro; intro = intro->next)
      {
         if (intro->pid == ch->pcdata->pid && abs(intro->value) > 100000)
         {
            strcpy(who, "");
            if (IS_SET(intro->flags, INTRO_ATTACKER))
               strcat(who, "[ATTACKER] ");
            if (IS_SET(intro->flags, INTRO_KILLER))
               strcat(who, "[KILLER] ");
            if (IS_SET(intro->flags, INTRO_MYATTACKER))
               strcat(who, "[MY-ATTACKER] ");
            if (IS_SET(intro->flags, INTRO_MYKILLER))
               strcat(who, "[MY-KILLER] ");
            if (IS_SET(intro->flags, INTRO_THIEF))
               strcat(who, "[THIEF] ");
            if (IS_SET(intro->flags, INTRO_MYTHIEF))
               strcat(who, "[MY-THIEF] ");
            
            strcpy(fullname, "");
            sprintf(fullname, "%s%s%s %s", who, ch->pcdata->pretit, ch->name, ch->last_name);
            return &fullname[0];
         }
         if (intro->pid == ch->pcdata->pid)
            break;
      }
   }
   else
   {
      if (kingdom >= sysdata.max_kingdom)
      {
         bug("show_pers_ouput - %s is in an invalid kingdom", ch->name);
      }
      if (IS_NPC(ch))
         return ch->short_descr;
      if (kingdom < 2)
      {
         strcpy(fullname, "");
         sprintf(fullname, "%s%s %s", ch->pcdata->pretit, ch->name, ch->last_name);
         return &fullname[0];
      }
      if (ch->pcdata->hometown == kingdom)
      {
         strcpy(fullname, "");
         sprintf(fullname, "%s%s %s", ch->pcdata->pretit, ch->name, ch->last_name);
         return &fullname[0];
      }
      if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_UKNOWN))
      {
         strcpy(fullname, "");
         sprintf(fullname, "%s%s %s", ch->pcdata->pretit, ch->name, ch->last_name);
         return &fullname[0];
      }
      if ((cloak = get_wear_hidden_cloak(ch)) != NULL)
      {
         return "a cloaked individual";
      }
      else
      {
         for (intro = kingdom_table[kingdom]->first_introduction; intro; intro = intro->next)
         {
            if (intro->pid == ch->pcdata->pid && abs(intro->value) > 100000)
            {
               strcpy(who, "");
               if (IS_SET(intro->flags, INTRO_ATTACKER))
                  strcat(who, "[ATTACKER] ");
               if (IS_SET(intro->flags, INTRO_KILLER))
                  strcat(who, "[KILLER] ");
               if (IS_SET(intro->flags, INTRO_MYATTACKER))
                  strcat(who, "[MY-ATTACKER] ");
               if (IS_SET(intro->flags, INTRO_MYKILLER))
                  strcat(who, "[MY-KILLER] ");
               if (IS_SET(intro->flags, INTRO_THIEF))
                  strcat(who, "[THIEF] ");
               if (IS_SET(intro->flags, INTRO_MYTHIEF))
                  strcat(who, "[MY-THIEF] ");
            
               strcpy(fullname, "");
               sprintf(fullname, "%s%s%s %s", who, ch->pcdata->pretit, ch->name, ch->last_name);
               return &fullname[0];
            }
            if (intro->pid == ch->pcdata->pid)
               break;
         }
      }
   }
   if (ch->race == RACE_OGRE)
   {
      if (ch->pcdata->skincolor >= 1 && ch->pcdata->skincolor <= 5)
         sprintf(name, "green ogre");
      if (ch->pcdata->skincolor >= 6 && ch->pcdata->skincolor <= 9)
         sprintf(name, "brown ogre");
      if (ch->pcdata->skincolor >= 10 && ch->pcdata->skincolor >= 14)
         sprintf(name, "black ogre");
   }
   if (ch->race == RACE_FAIRY)
   {
      if (ch->pcdata->skincolor >= 1 && ch->pcdata->skincolor <= 5)
         sprintf(name, "blue fairy");
      if (ch->pcdata->skincolor >= 6 && ch->pcdata->skincolor <= 10)
         sprintf(name, "green fairy");
      if (ch->pcdata->skincolor >= 11 && ch->pcdata->skincolor <= 15)
         sprintf(name, "pink fairy");
   }
   if (ch->race == RACE_HUMAN)
   {
      if (ch->pcdata->skincolor >= 1 && ch->pcdata->skincolor <= 6)
         sprintf(name, "white human");
      if (ch->pcdata->skincolor >= 7 && ch->pcdata->skincolor <= 10)
         sprintf(name, "brown human");
      if (ch->pcdata->skincolor >= 11 && ch->pcdata->skincolor <= 15)
         sprintf(name, "black human");
   }
   if (ch->race == RACE_ELF)
   {
      if (ch->pcdata->skincolor >= 1 && ch->pcdata->skincolor <= 6)
         sprintf(name, "white elf");
      if (ch->pcdata->skincolor >= 7 && ch->pcdata->skincolor <= 10)
         sprintf(name, "brown elf");
      if (ch->pcdata->skincolor >= 11 && ch->pcdata->skincolor <= 15)
         sprintf(name, "black elf");
   }
   if (ch->race == RACE_DWARF)
   {
      if (ch->pcdata->skincolor >= 1 && ch->pcdata->skincolor <= 6)
         sprintf(name, "white dwarf");
      if (ch->pcdata->skincolor >= 7 && ch->pcdata->skincolor <= 10)
         sprintf(name, "brown dwarf");
      if (ch->pcdata->skincolor >= 11 && ch->pcdata->skincolor <= 15)
         sprintf(name, "black dwarf");
   }
   if (ch->race == RACE_HOBBIT)
   {
      if (ch->pcdata->skincolor >= 1 && ch->pcdata->skincolor <= 6)
         sprintf(name, "white hobbit");
      if (ch->pcdata->skincolor >= 7 && ch->pcdata->skincolor <= 9)
         sprintf(name, "brown hobbit");
      if (ch->pcdata->skincolor >= 11 && ch->pcdata->skincolor <= 15)
         sprintf(name, "black hobbit");
   }

   if (ch->pcdata->hairlength == 1)
      sprintf(hair, "bald, ");
   else if (ch->pcdata->haircolor >= 1 && ch->pcdata->haircolor <= 3)
      sprintf(hair, "white haired, ");
   else if (ch->pcdata->haircolor >= 4 && ch->pcdata->haircolor <= 7)
      sprintf(hair, "grey haired, ");
   else if (ch->pcdata->haircolor >= 8 && ch->pcdata->haircolor <= 11)
      sprintf(hair, "brown haired, ");
   else if (ch->pcdata->haircolor >= 12 && ch->pcdata->haircolor <= 16)
      sprintf(hair, "black haired, ");
   else if (ch->pcdata->haircolor >= 17 && ch->pcdata->haircolor <= 20)
      sprintf(hair, "blonde haired, ");
   else if (ch->pcdata->haircolor >= 21 && ch->pcdata->haircolor <= 25)
      sprintf(hair, "red haired, ");
   else if (ch->pcdata->haircolor >= 26 && ch->pcdata->haircolor <= 30)
      sprintf(hair, "green haired, ");
   else if (ch->pcdata->haircolor >= 31 && ch->pcdata->haircolor <= 35)
      sprintf(hair, "blue haired, ");
   else if (ch->pcdata->haircolor >= 36 && ch->pcdata->haircolor <= 40)
      sprintf(hair, "purple haired, ");
      
   if (intro)
   {
      if (intro->value > 80000)
         sprintf(who, "a familiar ");
      else if (intro->value > 50000)
         sprintf(who, "a somewhat familiar ");
      else if (intro->value > 25000)
         sprintf(who, "a vaguely familiar ");
      else if (abs(intro->value) <= 10000)
         sprintf(who, "an almost unknown ");
      else if (intro->value >= -25000)
      {
         strcpy(who, "");
         if (IS_SET(intro->flags, INTRO_ATTACKER))
            strcat(who, "[ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_KILLER))
            strcat(who, "[KILLER] ");
         if (IS_SET(intro->flags, INTRO_MYATTACKER))
            strcat(who, "[MY-ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_MYKILLER))
            strcat(who, "[MY-KILLER] ");
         if (IS_SET(intro->flags, INTRO_THIEF))
            strcat(who, "[THIEF] ");
         if (IS_SET(intro->flags, INTRO_MYTHIEF))
            strcat(who, "[MY-THIEF] ");
            
         strcat(who, "an almost unknown ");
      }
      else if (intro->value >= -50000)
      {
         strcpy(who, "");
         if (IS_SET(intro->flags, INTRO_ATTACKER))
            strcat(who, "[ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_KILLER))
            strcat(who, "[KILLER] ");
         if (IS_SET(intro->flags, INTRO_MYATTACKER))
            strcat(who, "[MY-ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_MYKILLER))
            strcat(who, "[MY-KILLER] ");
         if (IS_SET(intro->flags, INTRO_THIEF))
            strcat(who, "[THIEF] ");
         if (IS_SET(intro->flags, INTRO_MYTHIEF))
            strcat(who, "[MY-THIEF] ");
            
         strcat(who, "a vaguely familiar ");
      }
      else if (intro->value >= -80000)  
      {
         strcpy(who, "");
         if (IS_SET(intro->flags, INTRO_ATTACKER))
            strcat(who, "[ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_KILLER))
            strcat(who, "[KILLER] ");
         if (IS_SET(intro->flags, INTRO_MYATTACKER))
            strcat(who, "[MY-ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_MYKILLER))
            strcat(who, "[MY-KILLER] ");
         if (IS_SET(intro->flags, INTRO_THIEF))
            strcat(who, "[THIEF] ");
         if (IS_SET(intro->flags, INTRO_MYTHIEF))
            strcat(who, "[MY-THIEF] ");
            
         strcat(who, "a somewhat familiar ");
      }
      else if (intro->value >= -100000)
      {
         strcpy(who, "");
         if (IS_SET(intro->flags, INTRO_ATTACKER))
            strcat(who, "[ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_KILLER))
            strcat(who, "[KILLER] ");
         if (IS_SET(intro->flags, INTRO_MYATTACKER))
            strcat(who, "[MY-ATTACKER] ");
         if (IS_SET(intro->flags, INTRO_MYKILLER))
            strcat(who, "[MY-KILLER] ");
         if (IS_SET(intro->flags, INTRO_THIEF))
            strcat(who, "[THIEF] ");
         if (IS_SET(intro->flags, INTRO_MYTHIEF))
            strcat(who, "[MY-THIEF] ");
            
         strcat(who, "a familiar ");
      }   
      if (ch->pcdata->cheight >= 1 && ch->pcdata->cheight <= 5)
         sprintf(height, "short, ");
      if (ch->pcdata->cheight >= 6 && ch->pcdata->cheight <= 8)
         sprintf(height, "average, ");
      if (ch->pcdata->cheight >= 9 && ch->pcdata->cheight <= 13)
         sprintf(height, "tall, ");
   }
   else
   {
      strcpy(who, "");
      if (ch->pcdata->cheight >= 1 && ch->pcdata->cheight <= 5)
         sprintf(height, "a short, ");
      if (ch->pcdata->cheight >= 6 && ch->pcdata->cheight <= 8)
         sprintf(height, "an average, ");
      if (ch->pcdata->cheight >= 9 && ch->pcdata->cheight <= 13)
         sprintf(height, "a tall, ");
   }
   if(ch->fame >= 4000)
   {
      strcpy(fullname, "");
      sprintf(fullname, "%s%s", ch->pcdata->pretit, ch->last_name);
      return &fullname[0];
   }  
   else if(ch->fame >= 10000)
   {
      strcpy(fullname, "");
      sprintf(fullname, "%s%s %s", ch->pcdata->pretit, ch->name, ch->last_name);
      return &fullname[0];
   }
   else
   {
   	strcpy(fullname, "");
   	sprintf(fullname, "%s%s%s%s", who, height, hair, name);
   	return &fullname[0];
   }     
}

char *get_weapontype(sh_int wtype)
{
   switch (wtype)
   {
      default:
         return "Unknown";
      case 0:
         return "Hit";
      case 1:
         return "Slash";
      case 2:
         return "Pierce";
      case 3:
         return "Stab";
      case 4:
         return "Whip";
      case 5:
         return "Claw";
      case 6:
         return "Pound";
      case 7:
         return "Crush";
      case 8:
         return "Bolt";
      case 9:
         return "Arrow";
      case 10:
         return "Stone";
      case 11:
         return "Chop";
      case 12:
         return "Thrust";
      case 13:
         return "Swing";
      case 14:
         return "Rip";
      case 15:
         return "Knife";
      case 16:
         return "Cleave";
      case 17:
         return "Fist";
      case 18:
         return "Immolation";
      case 19:
         return "Freeze";
      case 20:
         return "Acid";
      case 21:
         return "Electrocution";
      case 22:
         return "Blessing";
      case 23:
         return "Curse";
   }
}
char *mastery_color(sh_int sn, sh_int cl)
{
   if (skill_table[sn]->mastery[cl] == 1)
      return "&Y";
   if (skill_table[sn]->mastery[cl] == 2)
      return "&c";
   if (skill_table[sn]->mastery[cl] == 3)
      return "&C";
   if (skill_table[sn]->mastery[cl] == 4)
      return "&R";
   if (skill_table[sn]->mastery[cl] == 5)
      return "&c&w";
   if (skill_table[sn]->mastery[cl] == 6)
      return "&w&W";

   return "&P";
}

char *mastery_diffcolor(sh_int sn, sh_int cl)
{
   if (skill_table[sn]->masterydiff[0] == 1)
      return "&Y";
   if (skill_table[sn]->masterydiff[0] == 2)
      return "&G&c";
   if (skill_table[sn]->masterydiff[0] == 3)
      return "&G&C";
   if (skill_table[sn]->masterydiff[0] == 4)
      return "&R";

   return "&P";
}

char *group_color(sh_int group, sh_int cl)
{
   int sn;
   sh_int max = -1;

   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      if (skill_table[sn]->group[cl] == group)
      {
         if (max == -1)
         {
            max = skill_table[sn]->mastery[cl];
         }
         else
         {
            if (skill_table[sn]->mastery[cl] > max)
               max = skill_table[sn]->mastery[cl];
         }
      }
   }
   if (max == 1)
      return "&Y";
   if (max == 2)
      return "&c";
   if (max == 3)
      return "&C";
   if (max == 4)
      return "&R";

   return "&P";
}

// Can use at a portal spot to see what is available to portal to
void freeportals(CHAR_DATA * ch)
{
   int p;
   int count = 0;
   int fnd = 0;
   int found = 0;
   char buf[MSL];
   OMAP_DATA *mobj;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *room;
   char finbuf[MSL];

   if (ch->in_room->vnum != ROOM_VNUM_PORTAL)
   {
      for (p = 0; p < sysdata.last_portal; p++)
      {
         if (xIS_SET(ch->pcdata->portalfnd, p))
         {
            if (ch->coord->x == portal_show[p]->x && ch->coord->y == portal_show[p]->y && ch->map == portal_show[p]->map)
            {
               fnd++;
            }
         }
      }
   }
   else
      fnd++;

   if (fnd == 0)
   {
      send_to_char("You need to be at a portal to get a list of free ones.\n\r", ch);
      return;
   }
   fnd = 0;
   sprintf(finbuf, "---------------------------------------------------------\n\r");
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->item_type == ITEM_PORTAL)
      {
         if (obj->in_room && obj->in_room->vnum == ROOM_VNUM_PORTAL)
         {
            found = 1;
         }
      }
   }
   if (found == 0)
   {
      fnd++;
      room = get_room_index(ROOM_VNUM_PORTAL);
      if (ch->in_room->vnum != room->vnum)
      {
         sprintf(buf, "[HM]       %s\n\r", room->name);
         strcat(finbuf, buf);
      }
   }
   for (p = 0; p < sysdata.last_portal; p++)
   {
      found = 0;
      if (xIS_SET(ch->pcdata->portalfnd, p))
      {
         count++;
         for (mobj = first_wilderobj; mobj; mobj = mobj->next)
         {
            if (mobj->mapobj->item_type == ITEM_PORTAL)
            {
               if (mobj->mapobj->value[0] == portal_show[p]->x && mobj->mapobj->value[1] == portal_show[p]->y
                  && mobj->mapobj->value[2] == portal_show[p]->map)
               {
                  found = 1;
               }
            }
         }
         if (found == 0)
         {
            fnd++;
            if (ch->coord->x != portal_show[p]->x || ch->coord->y != portal_show[p]->y || ch->map != portal_show[p]->map)
            {
               sprintf(buf, "[%2d]       %s\n\r", count, portal_show[p]->desc);
               strcat(finbuf, buf);
            }
         }
      }
   }
   if (count == 0)
   {
      send_to_char("You have not found any portals.\n\r", ch);
      return;
   }
   if (fnd == 0)
   {
      send_to_char("All portal locations are in use right now.\n\r", ch);
      return;
   }
   else
   {
      send_to_char("Number     Location\n\r", ch);
      send_to_char(finbuf, ch);
   }
   return;
}

// Will allow players to search for new portals, have to be in portal room to switch the flag
void do_huntportals(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("Only Players can Hunt Portals.\n\r", ch);
      return;
   }
   if (ch->in_room->vnum != ROOM_VNUM_PORTAL)
   {
      send_to_char("You can only choose to start/shop hunting portals in the City Portal.\n\r", ch);
      return;
   }
   if (xIS_SET(ch->act, PLR_PORTALHUNT))
   {
      act(AT_MAGIC, "You sake as the power to hunt portals is removed from you.", ch, NULL, NULL, TO_CHAR);
      act(AT_MAGIC, "$n shakes a bit as the power to hunt portals is sucked from $m.", ch, NULL, NULL, TO_ROOM);
      xREMOVE_BIT(ch->act, PLR_PORTALHUNT);
   }
   else
   {
      act(AT_MAGIC, "$n eyes turn blue and fade back to normal as $e prepares to hunt portals.", ch, NULL, NULL, TO_ROOM);
      act(AT_MAGIC, "Your eyes turn blue and fade back to normal as you prepare to hunt portals.", ch, NULL, NULL, TO_CHAR);
      xSET_BIT(ch->act, PLR_PORTALHUNT);
   }
}
void do_markportal(CHAR_DATA * ch, char *argument)
{
   int p;
   int kdm;
   char buf[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("Only Players can Mark Portals.\n\r", ch);
      return;
   }
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only mark portals out in the Wilderness.\n\r", ch);
      return;
   }
   if (!xIS_SET(ch->act, PLR_PORTALHUNT))
   {
      send_to_char("You have to be magically blessed to hunt or mark portals.\n\r", ch);
      return;
   }
   for (p = 0; p < sysdata.last_portal; p++)
   {
      if (portal_show[p]->x == ch->coord->x && portal_show[p]->y == ch->coord->y && portal_show[p]->map == ch->map)
      {
         for (kdm = 0; kdm < sysdata.max_kingdom; kdm++)
         {
            if (kdm > 1)
            {
               sprintf(buf, "%s Portal", kingdom_table[kdm]->name);
               if (!str_cmp(portal_show[p]->desc, buf))
               {
                  send_to_char("Cannot mark Kingdom portals, you get them when you join.\n\r", ch);
                  return;
               }
            }
         }
         xSET_BIT(ch->pcdata->portalfnd, p);
         send_to_char("This portal has been marked.  listportals for portals you have found.\n\r", ch);
         return;
      }
   }
}

void do_listportals(CHAR_DATA * ch, char *argument)
{
   int p;
   int count = 0;
   char buf[MSL];
   char finbuf[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("Only Players can List Portals.\n\r", ch);
      return;
   }

   if (!str_cmp(argument, "free"))
   {
      freeportals(ch);
      return;
   }

   sprintf(finbuf, "---------------------------------------------------------\n\r");

   for (p = 0; p < sysdata.last_portal; p++)
   {
      if (xIS_SET(ch->pcdata->portalfnd, p))
      {
         count++;
         sprintf(buf, "[%2d]       %s\n\r", count, portal_show[p]->desc);
         strcat(finbuf, buf);
      }
   }
   if (count == 0)
   {
      send_to_char("You have not found any portals.\n\r", ch);
      return;
   }
   else
   {
      send_to_char("Number     Location\n\r", ch);
      send_to_char(finbuf, ch);
   }
   return;
}

void do_settoadvance(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   int sphere;
   int group;
   int tier;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  settoadvance show <sphere/group> <value/name>\n\r", ch);
      send_to_char("Syntax:  settoadvance set <sphere> <group> <tier> <value>\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   
   if (!str_cmp(arg1, "show"))
   {
      argument = one_argument(argument, arg1);
      if (!str_cmp(arg1, "sphere"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) > 0)
            sphere = atoi(arg1);
         else
            sphere = issphere(arg1);
         if (sphere > 0)
         {
            send_to_char("Sphere                   Group                    Tiers\n\r", ch);
            send_to_char("-------------------------------------------------------------------------\n\r", ch);
            for (group = 0; group < MAX_GROUP; group++)
            {
               if (sysdata.toadvance[sphere-1][group][0] > 0)
               {
                  strcpy(buf, "");
                  for (tier = 2; tier <= MAX_TIER; tier++)
                  {
                     sprintf(arg1, "   %-2d", sysdata.toadvance[sphere-1][group][tier-2]);
                     strcat(buf, arg1);
                  } 
                  ch_printf(ch, "%-20s     %-20s    %s\n\r", get_sphere_name2(sphere), get_group_name(group+1), buf);
               }
            }
            return;
         }
         send_to_char("That is not a valid sphere.\n\r", ch);
         return;
      }
      if (!str_cmp(arg1, "group"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) > 0)
            group = atoi(arg1);
         else
            group = isgroup(arg1);
         if (group > 0)
         {
            send_to_char("Sphere                   Group                    Tiers\n\r", ch);
            send_to_char("-------------------------------------------------------------------------\n\r", ch);
            for (sphere = 0; sphere < MAX_SPHERE; sphere++)
            {
               if (sysdata.toadvance[sphere][group-1][0] > 0)
               {
                  strcpy(buf, "");
                  for (tier = 2; tier <= MAX_TIER; tier++)
                  {
                     sprintf(arg1, "   %-2d", sysdata.toadvance[sphere-1][group][tier-2]);
                     strcat(buf, arg1);
                  } 
                  ch_printf(ch, "%-20s     %-20s    %s\n\r", get_sphere_name2(sphere+1), get_group_name(group), buf);
               }
            }
            return;
         }
         send_to_char("That is not a valid group.\n\r", ch);
         return;
      }
   }
   
   if (!str_cmp(arg1, "set"))
   {
      argument = one_argument(argument, arg1);
      if (atoi(arg1) > 0)
         sphere = atoi(arg1);
      else
         sphere = issphere(arg1);
      if (sphere < 1 || sphere > MAX_SPHERE)
      {
         ch_printf(ch, "Range is 1 to %d", MAX_SPHERE);
         return;
      }
      
      argument = one_argument(argument, arg1);
      if (atoi(arg1) > 0)
         group = atoi(arg1);
      else
         group = isgroup(arg1);
      if (group < 1 || group > MAX_GROUP)
      {
         ch_printf(ch, "Range is 1 to %d", MAX_GROUP);
         return;
      }  
      
      argument = one_argument(argument, arg1);
      if (atoi(arg1) < 2 || atoi(arg1) > MAX_TIER)
      {
         ch_printf(ch, "Range is 2 to %d", MAX_TIER);
         return;
      }
      tier = atoi(arg1);
      
      if (atoi(argument) < 0)
      {
         send_to_char("The value has to be greater than or equal to 0.\n\r", ch);
         return;
      }
      sysdata.toadvance[sphere-1][group-1][tier-2] = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   do_settoadvance(ch, "");
   return;
}

//Minimum requirements of mastery to learn a mastery above yours (like Novice to Expert)
int min_to_learn(int sn)
{
   int mdiff = skill_table[sn]->masterydiff[0];
   return sysdata.toadvance[skill_table[sn]->stype-1][skill_table[sn]->group[0]-1][mdiff-2];
}

void generate_webskill_help(CHAR_DATA *ch, char *skillname, char *bfilename)
{
   HELP_DATA *pHelp;
   char buf[MSL];
   char parse[MSL*2];
   char aparse[MSL];
   FILE *fp;
   char filename[MIL];
   int x;
   int y;
   
   sprintf(filename, "%sskill%s.htm", SKILLOUT_DIR, skillname);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      send_to_char("Cannot write to the webhelp file (Help).\n\r", ch);
      return;
   }
   if ((pHelp = get_help(ch, skillname)) == NULL)
   {
      ch_printf(ch, "Cannot find helpfile for %s, aborting the write.", skillname);
      return;
   }
   sprintf(buf, "<html><head><title>Helpfile</title></head><body bgcolor=\"#DCE4E1\">-------------------------------------------------------<br>"
                "%s<br>-------------------------------------------------------<br><pre>", pHelp->keyword);
   for (x = 0,y = 0;;)
   {
      if (pHelp->text[x] == '&')
      {
         if (pHelp->text[x+1] == '&')
         {
            parse[y++] = '&';
         }
         x+=2;
      }
      else if (pHelp->text[x] == '\r')
      {
         x++;
      }
      else if (pHelp->text[x] == '\0')
      {
         parse[y] = '\0';
         break;
      }
      else
      {
         parse[y++] = pHelp->text[x++];
      }
   }
   sprintf(aparse, "</pre><p><a href=\"%s\">Go back</a>", bfilename);
   strcat(parse, aparse);
   for (x = 0;;x++)
   {
      if (buf[x] != '\0')
         putc(buf[x], fp);
      else
         break;
   }
   for (x = 0;;x++)
   {
      if (parse[x] != '\0')
         putc(parse[x], fp);
      else
         break;
   }
   fclose(fp);
   return;
}   

char *get_webtiercolor(int x)
{
   if (x == 1)
      return "<font color=\"#FFFF00\">";
   if (x == 2)
      return "<font color=\"#008080\">";
   if (x == 3)
      return "<font color=\"#00FFFF\">";
   if (x == 4)
      return "<font color=\"#FF0000\">";
      
   return "<font color=\"#FFFF00\">";
}
int global_skillcount;
void generate_group_files(CHAR_DATA *ch, int sn)
{
   int csn;
   FILE *fp;
   char wbuf[MSL];
   char abuf[MSL];
   char filename[MSL];
   int x;
   int col = 0;
   
   sprintf(filename, "%s%s%s.htm", SKILLOUT_DIR, get_group_name(skill_table[sn]->group[0]),
                     get_sphere_name(skill_table[sn]->stype));
   if ((fp = fopen(filename, "w")) == NULL)
   {
      send_to_char("Cannot write to the webhelp file (Group).\n\r", ch);
      return;
   }
   sprintf(wbuf, "<html>\n<head>\n<title>Skill and Spell Groups</title>\n</head>\n<body bgcolor=\"#000000\">\n");
   sprintf(abuf, "<table width=600><tr><td><center><font color=red>%s</font></center></td></tr></table><p>\n<table width=600><tr>", get_sphere_name(skill_table[sn]->stype));
   strcat(wbuf, abuf);                  
   for (x = 1; x <= 4; x++)
   {
      for (csn = 0; csn < top_sn && skill_table[csn] && skill_table[csn]->name; csn++)
      {
         if (skill_table[csn]->group[0] == skill_table[sn]->group[0] && skill_table[csn]->stype == skill_table[sn]->stype 
         &&  skill_table[csn]->masterydiff[0] == x)
         {
            sprintf(abuf, "<td width=200><a href=\"skill%s.htm\">%s%s</font></a></td>", skill_table[csn]->name, get_webtiercolor(x), skill_table[csn]->name);
            global_skillcount++;
            generate_webskill_help(ch, skill_table[csn]->name, filename);
            strcat(wbuf, abuf);
            if (++col % 3 == 0)
               strcat(wbuf, "</tr>\n<tr>");
         }
      }
   }
   strcat(wbuf, "</tr></table>");
   sprintf(abuf, "<p><a href=\"skillout.htm\">Go back</a>");
   strcat(wbuf, abuf);
   col = 0;
   for (x = 0;;x++)
   {
      if (wbuf[x] != '\0')
         putc(wbuf[x], fp);
      else
         break;
   }
   fclose(fp);
}
/* List available groups and later individual skills */
void do_listgroups(CHAR_DATA * ch, char *argument)
{
   sh_int sn;
   sh_int csn;
   char buf[MSL];
   char buf2[MSL];
   char arg[MIL];
   sh_int sbuf[MAX_SPHERE+1];
   sh_int gbuf[MAX_GROUP+1];
   sh_int ranking[MAX_RANKING+1];
   sh_int ranking2[MAX_RANKING+1];
   int col = 0;
   int x;
   sh_int y;
   sh_int group;

   for (y = 1; y <= MAX_GROUP; y++)
      gbuf[y] = 0;
   for (y = 1; y <= MAX_SPHERE; y++)
      sbuf[y] = 0; 
   for (y = 1; y <= MAX_RANKING; y++)
   {
      ranking2[y] = 0;
      ranking[y] = 0;
   }

   set_char_color(AT_MAGIC, ch);
   if (argument[0] == '\0')
   {
      sprintf(buf, MXPFTAG("Command 'listgroups allgroups' desc='Click here to see a list of spell/skill groups'", "listgroups allgroups", "/Command"));
      ch_printf(ch, "Syntax: %s\n\r", buf);
      send_to_char("Syntax: listgroups <group name>\n\r", ch);
      send_to_char("Syntax: listgroups <skill name>\n\r", ch);     
      send_to_char("Syntax: listgroups toadvance <group name>\n\r", ch);
      if (get_trust(ch) == LEVEL_ADMIN)
         send_to_char("Syntax:  listgroups webprint\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   
   if (!str_cmp(arg, "toadvance"))
   {
      if ((group = isgroup(argument)) != -1)
      {
         for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
         {
            if (skill_table[sn]->group[0] == group)
            {
               if (skill_table[sn]->group[0] <= 5 && skill_table[sn]->stype == 4)
               {
                  if (skill_table[sn]->masterydiff[0] > 1 && ranking2[skill_table[sn]->masterydiff[0]] == 0)
                  {   
                     ranking2[skill_table[sn]->masterydiff[0]] = min_to_learn(sn);
                  }
               }
               else
               {
                  if (skill_table[sn]->masterydiff[0] > 1 && ranking[skill_table[sn]->masterydiff[0]] == 0)
                  {
                     ranking[skill_table[sn]->masterydiff[0]] = min_to_learn(sn);
                  }
               }
            }
         }
         if (group <= 5)
         {
            ch_printf(ch, "                                    %s\n\r--------------------------------------------------------------------------------\n\r", get_sphere_name2(3));
            if (ranking[2] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking[2], get_tier_name(1), get_mastery_name(2), get_tier_name(2));
            }
            if (ranking[3] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking[3], get_tier_name(2), get_mastery_name(3), get_tier_name(3));
            }
            if (ranking[4] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking[4], get_tier_name(3), get_mastery_name(4), get_tier_name(4));
            }
            ch_printf(ch, "--------------------------------------------------------------------------------\n\r                                    %s\n\r", get_sphere_name2(4));
            send_to_char("--------------------------------------------------------------------------------\n\r", ch);
            if (ranking2[2] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking2[2], get_tier_name(1), get_mastery_name(2), get_tier_name(2));
            }
            if (ranking2[3] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking2[3], get_tier_name(2), get_mastery_name(3), get_tier_name(3));
            }
            if (ranking2[4] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking2[4], get_tier_name(3), get_mastery_name(4), get_tier_name(4));
            }
            return;
         }
         else
         {
            if (ranking[2] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking[2], get_tier_name(1), get_mastery_name(2), get_tier_name(2));
            }
            if (ranking[3] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking[3], get_tier_name(2), get_mastery_name(3), get_tier_name(3));
            }
            if (ranking[4] > 0)
            {
               ch_printf(ch, "It will take (-%d-) %s skills/spells at %s to learn %s skills/spells.\n\r", ranking[4], get_tier_name(3), get_mastery_name(4), get_tier_name(4));
            }
            return;
         }
      }
   }
   if (!str_cmp(arg, "webprint"))
   {
      FILE *fp;
      char wbuf[MSL];
      char abuf[MSL];
      char filename[MSL];
      sprintf(filename, SKILLOUT_FILE);
      if ((fp = fopen(filename, "w")) == NULL)
      {
         send_to_char("Cannot write to the webhelp file.\n\r", ch);
         return;
      }
      global_skillcount = 0;
      sprintf(wbuf, "<html>\n<head>\n<title>Skill and Spell Groups</title>\n</head>\n<body text=white bgcolor=\"#000000\" link=\"#00FF00\" vlink=\"#00FFFF\">\n");
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (skill_table[sn]->group[0] > 0)
         {
            if (sbuf[skill_table[sn]->stype] == 0) //Sphere hasn't appeared yet
            {
               sprintf(abuf, "<table width=600><tr><td><center><font color=red>%s</font></center></td></tr></table><p>\n<table width=600><tr>", get_sphere_name(skill_table[sn]->stype));
               strcat(wbuf, abuf);
               csn = sn;
               for (sn = csn; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
               {
                  if (gbuf[skill_table[sn]->group[0]] == 0 && skill_table[sn]->stype == skill_table[csn]->stype)
                  {                     
                     if (col++ % 3 == 0)
                     {
                        strcat(wbuf, "</tr>\n<tr>");
                     }
                     sprintf(abuf, "<td width=200><a href=\"%s%s.htm\">%s</a></td>", get_group_name(skill_table[sn]->group[0]),
                     get_sphere_name(skill_table[sn]->stype), get_group_name(skill_table[sn]->group[0]));
                     strcat(wbuf, abuf);
                     gbuf[skill_table[sn]->group[0]] = 1;
                     generate_group_files(ch, sn);
                  }
               }
               sn = csn;
               sbuf[skill_table[sn]->stype] = 1;
               for (y = 1; y < MAX_GROUP; y++)
                  gbuf[y] = 0;
               col = 0;
               strcat(wbuf, "</tr></table><p>\n");
            }
         }
      }
      sprintf(abuf, "<br>%d Total Skill/Spells available<br><a href=\"/\">Back to Rafermand website</a>", global_skillcount);
      strcat(wbuf, abuf);
      col = 0;
      for (x = 0;;x++)
      {
         if (wbuf[x] != '\0')
            putc(wbuf[x], fp);
         else
            break;
      }
      fclose(fp);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "allgroups"))
   {
      send_to_char("Your available groups are...\n\r\n\r", ch);
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (skill_table[sn]->group[0] > 0)
         {
            if (sbuf[skill_table[sn]->stype] == 0) //Sphere hasn't appeared yet
            {
               ch_printf(ch, "\n\r                              &R%s&C                              \n\r", get_sphere_name(skill_table[sn]->stype));
               csn = sn;
               for (sn = csn; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
               {
                  if (gbuf[skill_table[sn]->group[0]] == 0 && skill_table[sn]->stype == skill_table[csn]->stype)
                  {                     
                     if (col++ % 3 == 0)
                        send_to_char("\n\r", ch);
                     sprintf(buf, MXPFTAG("Listgroups '%s'", "%s", "/Listgroups") "%s", 
                        get_group_name(skill_table[sn]->group[0]), get_group_name(skill_table[sn]->group[0]), 
                        add_space(strlen(get_group_name(skill_table[sn]->group[0])), 24));
                     //sprintf(buf, "%-24s  ", get_group_name(skill_table[sn]->group[0]));
                     send_to_char(buf, ch);
                        gbuf[skill_table[sn]->group[0]] = 1;
                  }
               }
               sn = csn;
               sbuf[skill_table[sn]->stype] = 1;
               for (y = 1; y < MAX_GROUP; y++)
                  gbuf[y] = 0;
               col = 0;
               send_to_char("\n\r", ch);
            }
         }
      }
      if (col % 3 != 0)
         send_to_char("\n\r", ch);
      col = 0;
      set_char_color(AT_MAGIC, ch);
      send_to_char("\n\r\n\rYour available solo skills are...\n\r\n\r", ch);
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (skill_table[sn]->group[0] == 0 && skill_table[sn]->type == SKILL_SKILL)
         {
            sprintf(buf, "&c%-24s  ", skill_table[sn]->name);
            send_to_char(buf, ch);
            if (++col % 3 == 0)
               send_to_char("\n\r", ch);
         }
      }
      if (col % 3 != 0)
         send_to_char("\n\r", ch);
      return;
   }
   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      if (!str_cmp(arg, skill_table[sn]->name))
      {
         if (skill_table[sn]->group[0] == 0)
         {
            sprintf(buf, "%s is a stand alone skill.\n\r", skill_table[sn]->name);
            send_to_char(buf, ch);
            return;
         }
         else
         {
            sprintf(buf2, MXPFTAG("Command 'listgroups \"%s\"' desc='Click here to view other skills/spells in this sphere'", "%s", "/Command"),
               get_group_name(skill_table[sn]->group[0]), get_group_name(skill_table[sn]->group[0]));
            sprintf(buf, "%s belongs to group %s sphere %s\n\r", skill_table[sn]->name, buf2, get_sphere_name(skill_table[sn]->stype));
            send_to_char(buf, ch);
            return;
         }
      }
   }
   if ((group = isgroup(arg)) != -1)
   {
      y = 0;
      for (y = 1; y <= MAX_SPHERE; y++)
         sbuf[y] = 0;

      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (skill_table[sn]->group[0] == group)
         {
            if (sbuf[skill_table[sn]->stype] == 0) //Sphere not used
            {
               csn = sn;
               ch_printf(ch, "\n\r                              &R%s&C                              \n\r", get_sphere_name(skill_table[sn]->stype));
               for (x = 1; x <= 4; x++)
               {
                  for (sn = csn; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
                  {
                     if (skill_table[sn]->group[0] == group && skill_table[sn]->stype == skill_table[csn]->stype 
                     &&  skill_table[sn]->masterydiff[0] == x)
                     {
                        y++;
                        if (y == 1)
                        {
                           send_to_char("\n\r\n\r", ch);
                        }
                         sprintf(buf2, MXPFTAG("Command 'help \"%s\"' desc='Click here to view a helpfile for the following skill/spell'", "%s", "/Command") "%s",
                            skill_table[sn]->name, skill_table[sn]->name, add_space(strlen(skill_table[sn]->name), 30));
                        sprintf(buf, "       %s%-30s%s", mastery_diffcolor(sn, 0), buf2, char_color_str(AT_MAGIC, ch));
                        send_to_char(buf, ch);
                        if (++col % 2 == 0)
                           send_to_char("\n\r", ch);
                     }
                  }
               }
               sn = csn;
               if (col % 2 != 0)
                  send_to_char("\n\r", ch);
               col = 0;
               sbuf[skill_table[sn]->stype] = 1;
            }
         }
      }
      if (col != 0 && y > 0)
      {
         send_to_char("\n\r", ch);
      }
      if (y == 0)
      {
         send_to_char("You have no skills in that particular group.\n\r", ch);
         bug("%d group has no skills", group);
      }
      return;
   }
   do_listgroups(ch, "");
   return;
}

//Start time parsing functions :-)
int getmin(void) //Gets the minutes 0 - 59
{
   int min;
   int difftime;

   difftime = time(0) - sysdata.start_calender;
   
   min = (difftime / cvttime(90));
   min = difftime - (min * cvttime(90));
   min = min * 10 / (cvttime(15));
   return min;
}
int gethour(void) //Gets the hours 0 - 23
{
   int hour;
   int difftime;

   difftime = time(0) - sysdata.start_calender;
   hour = (difftime / cvttime(2160));
   hour = difftime - (hour * cvttime(2160));
   hour = hour / (cvttime(2160) / 24);
   return hour;
}
int getday(void) //Gets the days 0 - 359
{
   int day;
   int difftime;

   difftime = time(0) - sysdata.start_calender;
   day = (difftime / cvttime(777600));
   day = difftime - (day * cvttime(777600));
   day = day / (cvttime(777600) / 360);
   return day;
}
int getyear(void) //Gets the year 0+
{
   int difftime;

   difftime = time(0) - sysdata.start_calender;
   return (difftime / cvttime(777600));
}
int getdayofmonth(int day) //Parses a day into a day in a month 1 - 30
{
   int eday;

   eday = (day / 30);
   eday = day - (eday * 30);
   return eday + 1;
}
int get_value_month(void) //Gets the day of the month
{
   return (getday()/30)+1;
}
   
char *getmonth(int day) //Gets a valid name of a month
{
   int month;

   month = day / 30;
   if (month == 0)
      return "Jan";
   if (month == 1)
      return "Feb";
   if (month == 2)
      return "Mar";
   if (month == 3)
      return "Apr";
   if (month == 4)
      return "May";
   if (month == 5)
      return "Jun";
   if (month == 6)
      return "Jul";
   if (month == 7)
      return "Aug";
   if (month == 8)
      return "Sep";
   if (month == 9)
      return "Oct";
   if (month == 10)
      return "Nov";
   if (month == 11)
      return "Dec";

   return "Unk";
}

int get12hour(int hour) //Returns the hour in 12 hour format
{
   int ehour;

   ehour = hour;

   if (ehour > 11)
      ehour = ehour - 12;

   if (ehour == 0)
      ehour = 12;

   return ehour;
}

char *getampm(int hour) //Returns AM or PM
{
   if (hour >= 0 && hour <= 11)
      return "AM";
   else if (hour >= 12 && hour <= 23)
      return "PM";
   else
      return "AM";
}

char *getdayofweek(void) //Returns Mon, etc
{
   int day;
   int lday;
   int difftime;

   difftime = time(0) - sysdata.start_calender;
   day = (difftime / cvttime(2160));
   lday = day / 7;
   day = day - (lday * 7);
   if (day == 0)
      return "Sun";
   if (day == 1)
      return "Mon";
   if (day == 2)
      return "Tue";
   if (day == 3)
      return "Wed";
   if (day == 4)
      return "Thu";
   if (day == 5)
      return "Fri";
   if (day == 6)
      return "Sat";

   return "Sun";
}


/*
 * Actual Game time for Rafermand.  Keeps track of how much time has passed
 */
char *getgametime(void)
{
   int min;
   int hour;
   int day;
   int year;
   char month[10];
   char dayofweek[10];
   char ampm[10];
   char buf[MSL];
   char *rptr;

   min = hour = day = year = 0;


   /* 60 - min 3600 - hour  86400 - day  31,536,000 - year (RL)
      Game Time - Normal
      90 - hour 2160 - day  64,800 - month 777600 - year
      1.5 min - 1 hour 36 min - 1 day  18 hours - 1 month  9 days - 1 year 
      Game Time - Reset Game
      30 - hour 720 - day  21,600 - month 259200 - year
      .5 min - 1 hour 12 min - 1 day  6 hours - 1 month  3 days - 1 year 
      */

   min = getmin();
   hour = gethour();
   day = getday();
   year = getyear();
   sprintf(ampm, "%s", getampm(hour));
   sprintf(month, "%s", getmonth(day));
   day = getdayofmonth(day);
   hour = get12hour(hour);
   sprintf(dayofweek, "%s", getdayofweek());

   sprintf(buf, "The Gametime is:          %s %s %2.2d %2.2d:%2.2d %s %d", dayofweek, month, day, hour, min, ampm, 1000 + year);
   rptr = buf;
   return rptr;
}

/*
 * Some increasingly freaky hallucinated objects		-Thoric
 * (Hats off to Albert Hoffman's "problem child")
 */
char *hallucinated_object(int ms, bool fShort)
{
   int sms = URANGE(1, (ms + 10) / 5, 20);

   if (fShort)
      switch (number_range(6 - URANGE(1, sms / 2, 5), sms))
      {
         case 1:
            return "a sword";
         case 2:
            return "a stick";
         case 3:
            return "something shiny";
         case 4:
            return "something";
         case 5:
            return "something interesting";
         case 6:
            return "something colorful";
         case 7:
            return "something that looks cool";
         case 8:
            return "a nifty thing";
         case 9:
            return "a cloak of flowing colors";
         case 10:
            return "a mystical flaming sword";
         case 11:
            return "a swarm of insects";
         case 12:
            return "a deathbane";
         case 13:
            return "a figment of your imagination";
         case 14:
            return "your gravestone";
         case 15:
            return "the long lost boots of Ranger Qutius";
         case 16:
            return "a glowing tome of arcane knowledge";
         case 17:
            return "a long sought secret";
         case 18:
            return "the meaning of it all";
         case 19:
            return "the answer";
         case 20:
            return "the key to life, the universe and everything";
      }
   switch (number_range(6 - URANGE(1, sms / 2, 5), sms))
   {
      case 1:
         return "A nice looking sword catches your eye.";
      case 2:
         return "The ground is covered in small sticks.";
      case 3:
         return "Something shiny catches your eye.";
      case 4:
         return "Something catches your attention.";
      case 5:
         return "Something interesting catches your eye.";
      case 6:
         return "Something colorful flows by.";
      case 7:
         return "Something that looks cool calls out to you.";
      case 8:
         return "A nifty thing of great importance stands here.";
      case 9:
         return "A cloak of flowing colors asks you to wear it.";
      case 10:
         return "A mystical flaming sword awaits your grasp.";
      case 11:
         return "A swarm of insects buzzes in your face!";
      case 12:
         return "The extremely rare Deathbane lies at your feet.";
      case 13:
         return "A figment of your imagination is at your command.";
      case 14:
         return "You notice a gravestone here... upon closer examination, it reads your name.";
      case 15:
         return "The long lost boots of Ranger Qutius lie off to the side.";
      case 16:
         return "A glowing tome of arcane knowledge hovers in the air before you.";
      case 17:
         return "A long sought secret of all mankind is now clear to you.";
      case 18:
         return "The meaning of it all, so simple, so clear... of course!";
      case 19:
         return "The answer.  One.  It's always been One.";
      case 20:
         return "The key to life, the universe and everything awaits your hand.";
   }
   return "Whoa!!!";
}


/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
//list_to_char just pumps it into list_to_char_type, it will take an option item type for use...
void show_list_to_char_type(OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing, int itemtype, const int iDefaultAction)
{
   char **prgpstrShow;
   char **prgpstrName;        /* for MXP */
   char **prgpstrShortName;   /* for MXP */
   int *prgnShow;
   int *pitShow;
   char *pstrShow;
   char *pstrName;            /* for MXP */
   char *pstrShortName;       /* for MXP */
   OBJ_DATA *obj;
   int nShow;
   int iShow;
   int type = 0;
   int count, offcount, tmp, ms, cnt;
   bool fCombine;
   char * pAction = NULL;

   if (!ch->desc)
      return;

   if (itemtype > 0)
      type = 1;


   /*
    * if there's no list... then don't do all this crap!  -Thoric
    */
   if (!list)
   {
      if (fShowNothing)
      {
         if (IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE))
            send_to_char("     ", ch);
         set_char_color(AT_OBJECT, ch);
         send_to_char("Nothing.\n\r", ch);
      }
      return;
   }
   
   /* work out which MXP tag to use */ 
   switch (iDefaultAction)
   {
      case eItemGet:  pAction = "Get"; break;   /* item on ground */
      case eItemDrop: pAction = "Drop"; break;   /* item in inventory */
      case eItemBid:  pAction = "Bid"; break;   /* auction item */
   } /* end of switch on action */

   /*
    * Alloc space for output lines.
    */
   count = 0;
   for (obj = list; obj; obj = obj->next_content)
   {
      if (IS_ONMAP_FLAG(ch) || IS_OBJ_STAT(obj, ITEM_ONMAP))
      {
         if (obj->map != ch->map || obj->coord->x != ch->coord->x || obj->coord->y != ch->coord->y)
            continue;
      }
      count++;
   }

   ms = (ch->mental_state ? ch->mental_state : 1)
      * (IS_NPC(ch) ? 1 : (ch->pcdata->condition[COND_DRUNK] ? (ch->pcdata->condition[COND_DRUNK] / 12) : 1));

   /*
    * If not mentally stable...
    */
   if (abs(ms) > 40)
   {
      offcount = URANGE(-(count), (count * ms) / 100, count * 2);
      if (offcount < 0)
         offcount += number_range(0, abs(offcount));
      else if (offcount > 0)
         offcount -= number_range(0, offcount);
   }
   else
      offcount = 0;

   if (count + offcount <= 0)
   {
      if (fShowNothing)
      {
         if (IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE))
            send_to_char("     ", ch);
         set_char_color(AT_OBJECT, ch);
         send_to_char("Nothing.\n\r", ch);
      }
      return;
   }

   CREATE(prgpstrShow, char *, count + ((offcount > 0) ? offcount : 0));
   CREATE( prgpstrName,	char*,	count + ((offcount > 0) ? offcount : 0) );
   CREATE( prgpstrShortName,	char*,	count + ((offcount > 0) ? offcount : 0) );
   CREATE(prgnShow, int, count + ((offcount > 0) ? offcount : 0));
   CREATE(pitShow, int, count + ((offcount > 0) ? offcount : 0));

   nShow = 0;
   tmp = (offcount > 0) ? offcount : 0;
   cnt = 0;

   /*
    * Format the list of objects.
    */
   for (obj = list; obj; obj = obj->next_content)
   {
      if (offcount < 0 && ++cnt > (count + offcount))
         break;

      if (IS_ONMAP_FLAG(ch) || IS_OBJ_STAT(obj, ITEM_ONMAP))
      {
         if (obj->map != ch->map || obj->coord->x != ch->coord->x || obj->coord->y != ch->coord->y)
            continue;
      }
      if (type == 1) // For Examine, will only show a certain itemtype if prompted
      {
         if (obj->item_type != itemtype)
            continue;
      }
      if (tmp > 0 && number_bits(1) == 0)
      {
         prgpstrShow[nShow] = str_dup(hallucinated_object(ms, fShort));
         prgpstrName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgpstrShortName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range(ITEM_LIGHT, ITEM_BOOK);
         nShow++;
         --tmp;
      }
      if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && (obj->item_type != ITEM_TRAP || IS_AFFECTED(ch, AFF_DETECTTRAPS)))
      {
         pstrShow = format_obj_to_char(obj, ch, fShort, TRUE);
         pstrName = obj->name;
         pstrShortName = obj->short_descr;
         fCombine = FALSE;

         if (IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE))
         {
            /*
             * Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwords.
             */
            for (iShow = nShow - 1; iShow >= 0; iShow--)
            {
               if (!strcmp(prgpstrShow[iShow], pstrShow))
               {
                  prgnShow[iShow] += obj->count;
                  fCombine = TRUE;
                  break;
               }
            }
         }

         pitShow[nShow] = obj->item_type;
         /*
          * Couldn't combine, or didn't want to.
          */
         if (!fCombine)
         {
            prgpstrShow[nShow] = str_dup(pstrShow);
            prgpstrName [nShow] = str_dup( pstrName );
      	    prgpstrShortName [nShow] = str_dup( pstrShortName );
            prgnShow[nShow] = obj->count;
            nShow++;
         }
         if (IS_ONMAP_FLAG(ch) || IS_OBJ_STAT(obj, ITEM_ONMAP))
         {
            if (obj->map != ch->map || obj->coord->x != ch->coord->x || obj->coord->y != ch->coord->y)
               continue;
         }
      }
   }
   if (tmp > 0)
   {
      int x;

      for (x = 0; x < tmp; x++)
      {
         prgpstrShow[nShow] = str_dup(hallucinated_object(ms, fShort));
         prgpstrName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgpstrShortName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range(ITEM_LIGHT, ITEM_BOOK);
         nShow++;
      }
   }

   /*
    * Output the formatted list.  -Color support by Thoric
    */
   for (iShow = 0; iShow < nShow; iShow++)
   {
      switch (pitShow[iShow])
      {
         default:
            set_char_color(AT_OBJECT, ch);
            break;
         case ITEM_BLOOD:
            set_char_color(AT_BLOOD, ch);
            break;
         case ITEM_MONEY:
         case ITEM_TREASURE:
            set_char_color(AT_YELLOW, ch);
            break;
         case ITEM_COOK:
         case ITEM_FOOD:
            set_char_color(AT_HUNGRY, ch);
            break;
         case ITEM_DRINK_CON:
         case ITEM_FOUNTAIN:
            set_char_color(AT_THIRSTY, ch);
            break;
         case ITEM_FIRE:
            set_char_color(AT_FIRE, ch);
            break;
         case ITEM_SCROLL:
         case ITEM_WAND:
         case ITEM_STAFF:
         case ITEM_SPELLBOOK:
            set_char_color(AT_MAGIC, ch);
            break;
      }
      if (fShowNothing)
         send_to_char("     ", ch);
      if (pAction && ch->desc && ch->desc->mxp)
      {
         ch_printf (ch, MXPTAG ("%s '%s'"), pAction, prgpstrName[iShow]);   
      }
      send_to_char(prgpstrShow[iShow], ch);
      if (pAction && ch->desc && ch->desc->mxp)
         ch_printf (ch, MXPTAG ("/%s"), pAction);
/*	if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) ) */
      {
         if (prgnShow[iShow] != 1)
            ch_printf(ch, " (%d)", prgnShow[iShow]);
      }

      send_to_char("\n\r", ch);
      DISPOSE(prgpstrShow[iShow]);
      if (prgpstrName[iShow])
         DISPOSE( prgpstrName[iShow] );
      if (prgpstrShortName[iShow])
         DISPOSE( prgpstrShortName[iShow] );
   }

   if (fShowNothing && nShow == 0)
   {
      if (IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE))
         send_to_char("     ", ch);
      set_char_color(AT_OBJECT, ch);
      send_to_char("Nothing.\n\r", ch);
   }

   /*
    * Clean up.
    */
   DISPOSE(prgpstrShow);
   DISPOSE( prgpstrName );
   DISPOSE( prgpstrShortName );
   DISPOSE(prgnShow);
   DISPOSE(pitShow);
   return;
}

void show_list_to_char(OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing, const int iDefaultAction)
{
   show_list_to_char_type(list, ch, fShort, fShowNothing, 0, iDefaultAction);
   return;
}

/*
 * Show fancy descriptions for certain spell affects		-Thoric
 */
void show_visible_affects_to_char(CHAR_DATA * victim, CHAR_DATA * ch)
{
   char buf[MSL];
   char name[MSL];

   if (IS_NPC(victim))
      strcpy(name, victim->short_descr);
   else
      strcpy(name, PERS_MAP(victim, ch));
   name[0] = toupper(name[0]);

   if (IS_AFFECTED(victim, AFF_SANCTUARY))
   {
      set_char_color(AT_WHITE, ch);
      ch_printf(ch, "%s is shrouded in flowing shadow and light.\n\r", name);
   }
   if (IS_AFFECTED(victim, AFF_FIRESHIELD))
   {
      set_char_color(AT_FIRE, ch);
      ch_printf(ch, "%s is engulfed within a blaze of mystical flame.\n\r", name);
   }
   if (IS_AFFECTED(victim, AFF_SHOCKSHIELD))
   {
      set_char_color(AT_BLUE, ch);
      ch_printf(ch, "%s is surrounded by cascading torrents of energy.\n\r", name);
   }
/*Scryn 8/13*/
   if (IS_AFFECTED(victim, AFF_ICESHIELD))
   {
      set_char_color(AT_LBLUE, ch);
      ch_printf(ch, "%s is ensphered by shards of glistening ice.\n\r", name);
   }
   if (IS_AFFECTED(victim, AFF_CHARM) && !xIS_SET(victim->act, ACT_MOUNTABLE) && !xIS_SET(victim->act, ACT_PET))
   {
      set_char_color(AT_MAGIC, ch);
      ch_printf(ch, "%s wanders in a dazed, zombie-like state.\n\r", name);
   }
   if (!IS_NPC(victim) && !victim->desc && victim->switched && IS_AFFECTED(victim->switched, AFF_POSSESS))
   {
      set_char_color(AT_MAGIC, ch);
      strcpy(buf, PERS(victim, ch));
      strcat(buf, " appears to be in a deep trance...\n\r");
   }
}

void show_char_to_char_0(CHAR_DATA * victim, CHAR_DATA * ch)
{
   char buf[MSL];
   char buf1[MSL];
   char message[MSL];

   buf[0] = '\0';

   set_char_color(AT_PERSON, ch);
   if (!IS_NPC(victim) && !victim->desc)
   {
      if (!victim->switched)
         send_to_char_color("&P[(Link Dead)] ", ch);
      else if (!IS_AFFECTED(victim, AFF_POSSESS))
         strcat(buf, "(Switched) ");
   }
   if (IS_NPC(victim) && IS_AFFECTED(victim, AFF_POSSESS) && IS_IMMORTAL(ch) && victim->desc)
   {
      sprintf(buf1, "(%s)", PERS_MAP(victim->desc->original, ch));
      strcat(buf, buf1);
   }
   if (sysdata.resetgame && !IS_NPC(victim) && !get_wear_hidden_cloak(victim))
   {
      sprintf(buf1, "&w&R[%d] %s", get_player_statlevel(victim), char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (!IS_NPC(victim) && !xIS_SET(victim->act, PLR_SHOWASIMM) && xIS_SET(ch->act, PLR_SHOWPC))
      strcat(buf, "[PC] ");
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_SHOWASIMM))
      strcat(buf, "[IMM] ");
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK))
      strcat(buf, "[AFK] ");

   if (!IS_NPC(victim) && !xIS_SET(victim->act, PLR_AFK) && xIS_SET(victim->act, PLR_AWAY))
      strcat(buf, "[AWAY] ");

   if ((!IS_NPC(victim) && xIS_SET(victim->act, PLR_WIZINVIS)) || (IS_NPC(victim) && xIS_SET(victim->act, ACT_MOBINVIS)))
   {
      if (!IS_NPC(victim))
         sprintf(buf1, "(Invis %d) ", victim->pcdata->wizinvis);
      else
         sprintf(buf1, "(Mobinvis %d) ", victim->mobinvis);
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_FORGEMOB))
   {
      sprintf(buf1, "&G&W[" MXPFTAG("Command 'forge list' desc='Click here to get a forge list'", "FORGE", "/Command") "] %s",
         char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_TRAINER))
   {
      sprintf(buf1, "&G&W[" MXPFTAG("Command 'learn \"%s\"' desc='Click here to see what %s Teaches'", "TRAINER", "/Command") "] %s", 
      victim->name, victim->name, char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_SBSELLER)) 
   {
      sprintf(buf1, "&G&W[" MXPFTAG("Command 'sbook list' desc='Click here to get a list of spellbooks from %s'", "SPELLBOOKS", "/Command") "] %s", 
      victim->name, char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_PRACTICE))
   {
      sprintf(buf1, "&G&W[PRACTICE] %s", char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && IS_IMMORTAL(ch) && !IS_NPC(ch) && xIS_SET(victim->act, ACT_NORESET))
   {
      sprintf(buf1, "&G&W[ONE RESET] %s", char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && IS_IMMORTAL(ch) && !IS_NPC(ch) && xIS_SET(victim->act, ACT_TIMERESET))
   {
      sprintf(buf1, "&G&W[TIMERESET] %s", char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_EXTRACTMOB))
   {
      if (IS_NPC(victim) && xIS_SET(victim->act, ACT_EXTRACTGOODS))
      {
         sprintf(buf1, "&w&C[EXTRACTING] %s", char_color_str(AT_PERSON, ch));
         strcat(buf, buf1);
      }
      else if (IS_NPC(victim) && xIS_SET(victim->act, ACT_DUMPGOODS))
      {
         sprintf(buf1, "&w&c[DUMPING] %s", char_color_str(AT_PERSON, ch));
         strcat(buf, buf1);
      }
      else
      {
         sprintf(buf1, "&c&w[PLANNING] %s", char_color_str(AT_PERSON, ch));
         strcat(buf, buf1);
      }
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_QUESTMOB))
   {
      sprintf(buf1, "&w&W[QUESTGIVER] %s", char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_CAPTAIN))
   {
      sprintf(buf1, "&w&C[CAPTAIN] %s", char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_BOSS))
   {
      sprintf(buf1, "&w&R[BOSS] %s", char_color_str(AT_PERSON, ch));
      strcat(buf, buf1);
   }
/*
   if (!IS_NPC(victim))
   {
      if (victim->pcdata->caste == 1)
         strcat(buf, "&Y(Serf)&P ");
      else if (victim->pcdata->caste == 2)
         strcat(buf, "&Y(Peasant)&P ");
      else if (victim->pcdata->caste == 3)
         strcat(buf, "&Y(Laborer)&P ");
      else if (victim->pcdata->caste == 4)
         strcat(buf, "&Y(Apprentice)&P ");
      else if (victim->pcdata->caste == 5)
         strcat(buf, "&Y(Journeymen)&P ");
      else if (victim->pcdata->caste == 6)
         strcat(buf, "&Y(Master)&P ");
      else if (victim->pcdata->caste == 7)
         strcat(buf, "&Y(Merchant)&P ");
      else if (victim->pcdata->caste == 8)
         strcat(buf, "&Y(Trader)&P ");
      else if (victim->pcdata->caste == 9 && victim->sex == 0)
         strcat(buf, "&Y(Businessman)&P ");
      else if (victim->pcdata->caste == 9 && victim->sex == 1)
         strcat(buf, "&Y(Businessman)&P ");
      else if (victim->pcdata->caste == 9 && victim->sex == 2)
         strcat(buf, "&Y(Businesswoman)&P ");
      else if (victim->pcdata->caste == 10)
         strcat(buf, "&Y(Mayor)&P ");

      else if (victim->pcdata->caste == 11)
         strcat(buf, "&P(Page)&P ");
      else if (victim->pcdata->caste == 12)
         strcat(buf, "&P(Squire)&P ");
      else if (victim->pcdata->caste == 13)
         strcat(buf, "&P(Knight)&P ");
      else if (victim->pcdata->caste == 14)
         strcat(buf, "&P(Baronet)&P ");
      else if (victim->pcdata->caste == 15)
         strcat(buf, "&P(Baron)&P ");
      else if (victim->pcdata->caste == 16)
         strcat(buf, "&P(Earl)&P ");
      else if (victim->pcdata->caste == 17)
         strcat(buf, "&P(Viscount)&P ");
      else if (victim->pcdata->caste == 18)
         strcat(buf, "&P(Count)&P ");
      else if (victim->pcdata->caste == 19)
         strcat(buf, "&P(Duke)&P ");
      else if (victim->pcdata->caste == 20)
         strcat(buf, "&P(Marquis)&P ");

      else if (victim->pcdata->caste == 21)
         strcat(buf, "&C(Vassal)&P ");
      else if (victim->pcdata->caste == 22)
         strcat(buf, "&C(Lord-Vassal)&P ");
      else if (victim->pcdata->caste == 23)
         strcat(buf, "&C(Lord)&P ");
      else if (victim->pcdata->caste == 24)
         strcat(buf, "&C(Hi-Lord)&P ");
      else if (victim->pcdata->caste == 25)
         strcat(buf, "&C(Captain)&P ");
      else if (victim->pcdata->caste == 26)
         strcat(buf, "&C(Minister)&P ");
      else if (victim->pcdata->caste == 27 && victim->sex == 0)
         strcat(buf, "&C(Prince)&P ");
      else if (victim->pcdata->caste == 27 && victim->sex == 1)
         strcat(buf, "&C(Prince)&P ");
      else if (victim->pcdata->caste == 27 && victim->sex == 2)
         strcat(buf, "&C(Princess)&P ");
      else if (victim->pcdata->caste == 28 && victim->sex == 0)
         strcat(buf, "&C(King)&P ");
      else if (victim->pcdata->caste == 28 && victim->sex == 1)
         strcat(buf, "&C(King)&P ");
      else if (victim->pcdata->caste == 28 && victim->sex == 2)
         strcat(buf, "&C(Queen)&P ");
      else if (victim->pcdata->caste == 29)
         strcat(buf, "&O(Avatar)&P ");
      else if (victim->pcdata->caste == 30)
         strcat(buf, "&O(Legend)&P ");

      else if (victim->pcdata->caste == 31)
         strcat(buf, "&G&W(Ascender)&P ");
      else if (victim->pcdata->caste == 32)
         strcat(buf, "&G&W(Immortal)&P ");
      else if (victim->pcdata->caste == 33)
         strcat(buf, "&G&W(God)&P ");
      else if (victim->pcdata->caste == 34)
         strcat(buf, "&G&W(Staff)&P ");
      else if (victim->pcdata->caste == 35)
         strcat(buf, "&G&W(Admin)&P ");
      else
         strcat(buf, "&r(Casteless)&P ");
   }
*/

   if (!IS_NPC(victim)
      && victim->pcdata->clan
      && victim->pcdata->clan->badge && (victim->pcdata->clan->clan_type != CLAN_ORDER && victim->pcdata->clan->clan_type != CLAN_GUILD))
      ch_printf_color(ch, "%s ", victim->pcdata->clan->badge);
   else
      set_char_color(AT_PERSON, ch);

   if (IS_AFFECTED(victim, AFF_INVISIBLE))
      strcat(buf, "(Invis) ");
   if (IS_AFFECTED(victim, AFF_HIDE))
      strcat(buf, "(Hide) ");
   if (IS_AFFECTED(victim, AFF_STALK))
      strcat(buf, "(Stalking) ");
   if (IS_AFFECTED(victim, AFF_PASS_DOOR))
      strcat(buf, "(Translucent) ");
   if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
      strcat(buf, "(Pink Aura) ");


   if (IS_AFFECTED(victim, AFF_BERSERK))
      strcat(buf, "(Wild-eyed) ");
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_ATTACKER))
      strcat(buf, "(ATTACKER) ");
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_KILLER))
      strcat(buf, "(KILLER) ");
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_THIEF))
      strcat(buf, "(THIEF) ");
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_LITTERBUG))
      strcat(buf, "(LITTERBUG) ");
   if (IS_NPC(victim) && IS_IMMORTAL(ch) && xIS_SET(victim->act, ACT_PROTOTYPE))
      strcat(buf, "(PROTO) ");
   if (IS_NPC(victim) && ch->mount && ch->mount == victim && ch->in_room == ch->mount->in_room)
      strcat(buf, "(Mount) ");
   if (victim->desc && victim->desc->connected == CON_EDITING)
      strcat(buf, "(Writing) ");
   if (victim->morph != NULL)
      strcat(buf, "(Morphed) ");

   set_char_color(AT_PERSON, ch);
   if ((victim->position == victim->defposition && victim->long_descr[0] != '\0')
      || (victim->morph && victim->morph->morph && victim->morph->morph->defpos == victim->position))
   {
      if (victim->morph != NULL)
      {
         if (!IS_IMMORTAL(ch))
         {
            if (victim->morph->morph != NULL)
               strcat(buf, victim->morph->morph->long_desc);
            else
               strcat(buf, victim->long_descr);
         }
         else
         {
            strcat(buf, PERS(victim, ch));
            if (!IS_NPC(victim) && !xIS_SET(ch->act, PLR_BRIEF))
               strcat(buf, show_pers_title(victim, ch));
            strcat(buf, ".\n\r");
         }
      }
      else
      {
         if (ch->desc && ch->desc->mxp)
            sprintf(buf1, MXPFTAG("Mobile '%s' desc='%s'", "%s", "/Mobile"), victim->name, victim->short_descr, victim->long_descr);
         else
            sprintf(buf1, "%s", victim->long_descr);
         strcat(buf, buf1);
      }
      send_to_char(buf, ch);
      show_visible_affects_to_char(victim, ch);
      return;
   }
   else
   {
      if (victim->morph != NULL && victim->morph->morph != NULL && !IS_IMMORTAL(ch))
         strcat(buf, MORPHPERS(victim, ch));
        
      sprintf(buf1, MXPFTAG("Look '%s' desc='%s'", "%s", "/Look"), PERS(victim, ch), PERS(victim, ch), PERS(victim, ch));
      strcat(buf, buf1);
   }

   if (!IS_NPC(victim) && !xIS_SET(ch->act, PLR_BRIEF))
      strcat(buf, show_pers_title(victim, ch));

   switch (victim->position)
   {
      case POS_DEAD:
         strcat(buf, "&P is DEAD!!&G");
         break;
      case POS_MORTAL:
         strcat(buf, "&P is mortally wounded.&G");
         break;
      case POS_INCAP:
         strcat(buf, "&P is incapacitated.&G");
         break;
      case POS_STUNNED:
         strcat(buf, "&P is lying here stunned.&G");
         break;
      case POS_SLEEPING:
         if (victim->on != NULL)
         {
            if (IS_SET(victim->on->value[2], SLEEP_AT))
            {
               sprintf(message, "&P is sleeping at %s.", victim->on->short_descr);
               strcat(buf, message);
            }
            else if (IS_SET(victim->on->value[2], SLEEP_ON))
            {
               sprintf(message, "&P is sleeping on %s.", victim->on->short_descr);
               strcat(buf, message);
            }
            else
            {
               sprintf(message, "&P is sleeping in %s.", victim->on->short_descr);
               strcat(buf, message);
            }
         }
         else
         {
            if (ch->position == POS_SITTING || ch->position == POS_RESTING)
               strcat(buf, "&P is sleeping nearby.&G");
            else
               strcat(buf, "&P is deep in slumber here.&G");
         }
         break;
      case POS_RESTING:
         if (victim->on != NULL)
         {
            if (IS_SET(victim->on->value[2], REST_AT))
            {
               sprintf(message, "&P is resting at %s.", victim->on->short_descr);
               strcat(buf, message);
            }
            else if (IS_SET(victim->on->value[2], REST_ON))
            {
               sprintf(message, "&P is resting on %s.", victim->on->short_descr);
               strcat(buf, message);
            }
            else
            {
               sprintf(message, "&P is resting in %s.", victim->on->short_descr);
               strcat(buf, message);
            }
         }
         else
         {
            if (ch->position == POS_RESTING)
               strcat(buf, "&P is sprawled out alongside you.&G");
            else if (ch->position == POS_MOUNTED)
               strcat(buf, "&P is sprawled out at the foot of your mount.&G");
            else
               strcat(buf, "&P is sprawled out here.&G");
         }
         break;
      case POS_SITTING:
         if (victim->on != NULL)
         {
            if (IS_SET(victim->on->value[2], SIT_AT))
            {
               sprintf(message, "&P is sitting at %s.", victim->on->short_descr);
               strcat(buf, message);
            }
            else if (IS_SET(victim->on->value[2], SIT_ON))
            {
               sprintf(message, "&P is sitting on %s.", victim->on->short_descr);
               strcat(buf, message);
            }
            else
            {
               sprintf(message, "&P is sitting in %s.", victim->on->short_descr);
               strcat(buf, message);
            }
         }
         else
            strcat(buf, "&P is sitting here.");
         break;
      case POS_STANDING:
         if (victim->on != NULL)
         {
            if (IS_SET(victim->on->value[2], STAND_AT))
            {
               sprintf(message, "&P is standing at %s", victim->on->short_descr);
               strcat(buf, message);
            }
            else if (IS_SET(victim->on->value[2], STAND_ON))
            {
               sprintf(message, "&P is standing on %s", victim->on->short_descr);
               strcat(buf, message);
            }
            else
            {
               sprintf(message, "&P is standing in %s", victim->on->short_descr);
               strcat(buf, message);
            }
         }
         else if (IS_IMMORTAL(victim))
            strcat(buf, "&P is here before you");
         else if ((victim->in_room->sector_type == SECT_UNDERWATER) && !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim))
            strcat(buf, "&P is drowning here");
         else if (victim->in_room->sector_type == SECT_UNDERWATER)
            strcat(buf, "&P is here in the water");
         else if ((victim->in_room->sector_type == SECT_OCEANFLOOR) && !IS_AFFECTED(victim, AFF_AQUA_BREATH) && !IS_NPC(victim))
            strcat(buf, "&P is drowning here");
         else if (victim->in_room->sector_type == SECT_OCEANFLOOR)
            strcat(buf, "&P is standing here in the water");
         else if (IS_AFFECTED(victim, AFF_FLOATING) || IS_AFFECTED(victim, AFF_FLYING))
            strcat(buf, "&P is hovering here");
         else
            strcat(buf, "&P is standing here");
         
         if (victim->rider)
         {
            sprintf(message, " with %s on %s back.&G", PERS(victim->rider, ch), victim->sex==1 ? "his" : "her");
            strcat(buf, message);
         }
         else
            strcat(buf, ".&G");
         break;
      case POS_SHOVE:
         strcat(buf, "&P is being shoved around.&G");
         break;
      case POS_DRAG:
         strcat(buf, "&P is being dragged around.&G");
         break;
      case POS_MOUNTED: case POS_RIDING:
         strcat(buf, "&P is here, upon &G");
         if (victim->riding)
         {
            sprintf(message, "&Pthe back of %s.", PERS(victim->riding, ch));
            strcat(buf, message);
         }
         else if (!victim->mount)
            strcat(buf, "&Pthin air???");
         else if (victim->mount == ch)
            strcat(buf, "&Pyour back.&G");
         else if (victim->in_room == victim->mount->in_room)
         {
            strcat(buf, PERS(victim->mount, ch));
            strcat(buf, ".");
         }
         else
            strcat(buf, "&Psomeone who left??&G");
         break;
      case POS_FIGHTING:
      case POS_EVASIVE:
      case POS_DEFENSIVE:
      case POS_AGGRESSIVE:
      case POS_BERSERK:
         strcat(buf, "&P is here, fighting &G");
         if (!victim->fighting)
         {
            strcat(buf, "&Pthin air???&G");

            /* some bug somewhere.... kinda hackey fix -h */
            if (!victim->mount)
               victim->position = POS_STANDING;
            else
               victim->position = POS_MOUNTED;
         }
         else if (who_fighting(victim) == ch)
            strcat(buf, "&PYOU!&G");
         else if (victim->in_room == victim->fighting->who->in_room)
         {
            strcat(buf, PERS(victim->fighting->who, ch));
            strcat(buf, ".");
         }
         else
            strcat(buf, "&Psomeone who left??&G");
         break;
   }

   strcat(buf, "\n\r");
   buf[0] = UPPER(buf[0]);
   send_to_char(buf, ch);
   show_visible_affects_to_char(victim, ch);
   return;
}



void show_char_to_char_1(CHAR_DATA * victim, CHAR_DATA * ch, int type)
{
   OBJ_DATA *obj;
   int iWear;
   bool found;
   sh_int mastery;
   OBJ_DATA *cloak;
   int cloaked = 0;

   mastery = POINT_LEVEL(LEARNED(ch, gsn_peek), MASTERED(ch, gsn_peek));

   if (IS_ONMAP_FLAG(ch))
   {
      if (victim->map != ch->map || victim->coord->x != ch->coord->x || victim->coord->y != ch->coord->y)
         return;
   }

   if (can_see(victim, ch) && !IS_NPC(ch) && ch->level < LEVEL_IMMORTAL && type == 0)
   {
      act(AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT);
      if (victim != ch)
         act(AT_ACTION, "$n looks at $N.", ch, NULL, victim, TO_NOTVICT);
      else
         act(AT_ACTION, "$n looks at $mself.", ch, NULL, victim, TO_NOTVICT);
   }

   if (!get_wear_hidden_cloak(victim) || (get_wear_hidden_cloak(victim) && ch == victim))
   {
      if (victim->description[0] != '\0')
      {
         if (victim->morph != NULL && victim->morph->morph != NULL)
         {
            send_to_char(victim->morph->morph->description, ch);
            set_char_color(AT_ACTION, ch);
         }
         else
         {
            send_to_char(victim->description, ch);
            set_char_color(AT_ACTION, ch);
         }
      }
      else
      {
         if (victim->morph != NULL && victim->morph->morph != NULL)
            send_to_char(victim->morph->morph->description, ch);
         else if (IS_NPC(victim))
            act(AT_PLAIN, "You see nothing special about $M.", ch, NULL, victim, TO_CHAR);
         else if (ch != victim)
            act(AT_PLAIN, "$E isn't much to look at...", ch, NULL, victim, TO_CHAR);
         else
            act(AT_PLAIN, "You're not much to look at...", ch, NULL, NULL, TO_CHAR);
      }
   }

   show_race_line(ch, victim);
   show_condition(ch, victim);
   
   if (IS_AFFECTED(victim, AFF_FLYING))
   {
      if (victim == ch)
         send_to_char("You are flying\n\r", ch);
      else
         act(AT_PLAIN, "$N is flying above your head", ch, NULL, victim, TO_CHAR);
   }
   if (type == 1)
      return;
   if ((cloak = get_wear_cloak(victim)) != NULL)
   {
      if (ch != victim)
         cloaked = 1;
   }   
   found = FALSE;
   for (iWear = 0; iWear < MAX_WEAR; iWear++)
   {
      if ((obj = get_eq_char(victim, iWear)) != NULL && can_see_obj(ch, obj))
      {
         if (IS_IMMORTAL(ch) || !cloaked || (cloaked && (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD) || IS_SET(obj->wear_flags, ITEM_WEAR_HEAD)
         ||  IS_SET(obj->wear_flags, ITEM_WIELD) || IS_SET(obj->wear_flags, ITEM_DUAL_WIELD)
         ||  IS_SET(obj->wear_flags, ITEM_MISSILE_WIELD) || IS_SET(obj->wear_flags, ITEM_LODGE_RIB)
         ||  IS_SET(obj->wear_flags, ITEM_LODGE_ARM) || IS_SET(obj->wear_flags, ITEM_LODGE_LEG)
         ||  IS_SET(obj->wear_flags, ITEM_WEAR_NOCKED) || IS_SET(obj->wear_flags, ITEM_WEAR_BACK))))
         {
            if (!found)
            {
               send_to_char("\n\r", ch);
               if (victim != ch)
                  act(AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR);
               else
                  act(AT_PLAIN, "You are using:", ch, NULL, NULL, TO_CHAR);
               found = TRUE;
            }
            send_to_char(where_name[iWear], ch);
            send_to_char(format_obj_to_char(obj, ch, TRUE, FALSE), ch);
            send_to_char("\n\r", ch);
            set_char_color(AT_ACTION, ch);
         }
      }
   }

   /*
    * Crash fix here by Thoric
    */
   if (IS_NPC(ch) || victim == ch)
      return;

   if (IS_IMMORTAL(ch))
   {
      if (IS_NPC(victim))
         ch_printf(ch, "\n\rMobile #%d '%s' ", victim->pIndexData->vnum, victim->name);
      else
         ch_printf(ch, "\n\r%s ", PERS(victim, ch));
      ch_printf(ch, "is a %s.\n\r",
         victim->race < max_npc_race && victim->race >= 0 ? print_npc_race(victim->race) : "unknown");
   }

   if (number_percent() < 40+(mastery) && MASTERED(ch, gsn_peek) > 0 && ch->pcdata->learned[gsn_peek] > 0)
   {
      ch_printf(ch, "\n\rYou peek at %s inventory:\n\r", victim->sex == 1 ? "his" : victim->sex == 2 ? "her" : "its");
      show_list_to_char(victim->first_carrying, ch, TRUE, TRUE, eItemNothing );
      learn_from_success(ch, gsn_peek, victim);
   }
   else if (ch->pcdata->learned[gsn_peek] > 0 && ch->pcdata->ranking[gsn_peek] > 0)
      learn_from_failure(ch, gsn_peek, victim);

   return;
}


void show_char_to_char(CHAR_DATA * list, CHAR_DATA * ch)
{
   CHAR_DATA *rch;

   for (rch = list; rch; rch = rch->next_in_room)
   {
      if (rch == ch)
         continue;

      if (IS_ONMAP_FLAG(ch))
      {
         if (rch->map != ch->map || rch->coord->x != ch->coord->x || rch->coord->y != ch->coord->y)
            continue;
      }

      if (can_see(ch, rch))
      {
         show_char_to_char_0(rch, ch);
      }
      else if (room_is_dark(ch->in_room) && IS_AFFECTED(rch, AFF_INFRARED) && !(!IS_NPC(rch) && IS_IMMORTAL(rch)))
      {
         set_char_color(AT_BLOOD, ch);
         send_to_char("The red form of a living creature is here.\n\r", ch);
      }
   }

   return;
}



bool check_blind(CHAR_DATA * ch)
{
   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT))
      return TRUE;

   if (IS_AFFECTED(ch, AFF_TRUESIGHT))
      return TRUE;

   if (IS_AFFECTED(ch, AFF_BLIND))
   {
      send_to_char("You can't see a thing!\n\r", ch);
      return FALSE;
   }

   return TRUE;
}

/*
 * Returns classical DIKU door direction based on text in arg	-Thoric
 */
int get_door(char *arg)
{
   int door;

   if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
      door = 0;
   else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
      door = 1;
   else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
      door = 2;
   else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
      door = 3;
   else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
      door = 4;
   else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
      door = 5;
   else if (!str_cmp(arg, "ne") || !str_cmp(arg, "northeast") || !str_cmp(arg, "f"))
      door = 6;
   else if (!str_cmp(arg, "nw") || !str_cmp(arg, "northwest") || !str_cmp(arg, "g"))
      door = 7;
   else if (!str_cmp(arg, "se") || !str_cmp(arg, "southeast") || !str_cmp(arg, "h"))
      door = 8;
   else if (!str_cmp(arg, "sw") || !str_cmp(arg, "southwest") || !str_cmp(arg, "i"))
      door = 9;
   else
      door = -1;
   return door;
}

void do_showcontrol(CHAR_DATA *ch, char *argument)
{
   if (IS_ONMAP_FLAG(ch))
   {
      display_map(ch, -1, -1, 1);
      return;
   }
}

void do_look(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   ROOM_INDEX_DATA *original;
   char *pdesc;
   bool doexaprog;
   sh_int door;
   int number, cnt;

   if (!ch->desc)
      return;
   if (ch->position < POS_SLEEPING)
   {
      send_to_char("You can't see anything but stars!\n\r", ch);
      return;
   }

   if (ch->position == POS_SLEEPING)
   {
      send_to_char("You can't see anything, you're sleeping!\n\r", ch);
      return;
   }
   if (!check_blind(ch))
      return;

   if (!IS_NPC(ch) && !xIS_SET(ch->act, PLR_HOLYLIGHT) && !IS_AFFECTED(ch, AFF_INFRARED) && !IS_AFFECTED(ch, AFF_TRUESIGHT) && room_is_dark(ch->in_room))
   {
      set_char_color(AT_DGREY, ch);
      send_to_char("It is pitch black ... \n\r", ch);
      show_char_to_char(ch->in_room->first_person, ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   doexaprog = str_cmp("noprog", arg2) && str_cmp("noprog", arg3);

   if (arg1[0] == '\0' || !str_cmp(arg1, "auto") || !str_cmp(arg1, "nowilderness"))
   {
      if (IS_ONMAP_FLAG(ch))
      {
         display_map(ch, -1, -1, 0);
         show_list_to_char(ch->in_room->first_content, ch, FALSE, FALSE, eItemGet);
         show_char_to_char(ch->in_room->first_person, ch);
         return;
      }
      /* 'look' or 'look auto' */
      /* Check to see if room is wilderness first -- Xerves 11/99 */
      if (xIS_SET(ch->in_room->room_flags, ROOM_WILDERNESS) && str_cmp(arg1, "nowilderness") && !IS_NPC(ch))
      {
         do_map(ch, "");
         return;
      }

      set_char_color(AT_RMNAME, ch);
      send_to_char(ch->in_room->name, ch);
      send_to_char("\n\r", ch);
      set_char_color(AT_RMDESC, ch);

      if (arg1[0] == '\0' || (!IS_NPC(ch) && !xIS_SET(ch->act, PLR_BRIEF)))
      {
         if (ch->desc && !ch->desc->run_buf)
         {
            send_to_char( MXPTAG ("rdesc"), ch);
            send_to_char(ch->in_room->description, ch);
            send_to_char( MXPTAG ("/rdesc"), ch);
         }
      }

      if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOMAP)) /* maps */
      {
         if (ch->in_room->map != NULL)
         {
            do_lookmap(ch, NULL);
         }
      }

      if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOEXIT))
         do_exits(ch, "auto");


      show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE, eItemGet );
      show_char_to_char(ch->in_room->first_person, ch);
      return;
   }

   if (!str_cmp(arg1, "under"))
   {
      int count;

      /* 'look under' */
      if (arg2[0] == '\0')
      {
         send_to_char("Look beneath what?\n\r", ch);
         return;
      }

      if ((obj = get_obj_here(ch, arg2)) == NULL)
      {
         send_to_char("You do not see that here.\n\r", ch);
         return;
      }
      if (!CAN_WEAR(obj, ITEM_TAKE) && ch->level < sysdata.level_getobjnotake)
      {
         send_to_char("You can't seem to get a grip on it.\n\r", ch);
         return;
      }
      if (get_ch_carry_weight(ch) + obj->weight > can_carry_w(ch))
      {
         send_to_char("It's too heavy for you to look under.\n\r", ch);
         return;
      }
      count = obj->count;
      obj->count = 1;
      act(AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR);
      act(AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM);
      obj->count = count;
      if (IS_OBJ_STAT(obj, ITEM_COVERING))
         show_list_to_char( obj->first_content, ch, TRUE, TRUE, eItemNothing );
      else
         send_to_char("Nothing.\n\r", ch);
      if (doexaprog)
         oprog_examine_trigger(ch, obj);
      return;
   }

   if (!str_cmp(arg1, "i") || !str_cmp(arg1, "in"))
   {
      int count;

      /* 'look in' */
      if (arg2[0] == '\0')
      {
         send_to_char("Look in what?\n\r", ch);
         return;
      }

      if ((obj = get_obj_here(ch, arg2)) == NULL)
      {
         send_to_char("You do not see that here.\n\r", ch);
         return;
      }

      switch (obj->item_type)
      {
         default:
            send_to_char("That is not a container.\n\r", ch);
            break;

         case ITEM_DRINK_CON:
            if (obj->value[1] <= 0)
            {
               send_to_char("It is empty.\n\r", ch);
               if (doexaprog)
                  oprog_examine_trigger(ch, obj);
               break;
            }

            ch_printf(ch, "It's %s full of a %s liquid.\n\r",
               obj->value[1] < obj->value[0] / 4
               ? "less than" : obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq_table[obj->value[2]].liq_color);

            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            break;

         case ITEM_PORTAL:
            for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
            {
               if (pexit->vdir == DIR_PORTAL && IS_SET(pexit->exit_info, EX_PORTAL))
               {
                  if (room_is_private(pexit->to_room) && get_trust(ch) < sysdata.level_override_private)
                  {
                     set_char_color(AT_WHITE, ch);
                     send_to_char("That room is private buster!\n\r", ch);
                     return;
                  }
                  original = ch->in_room;
                  char_from_room(ch);
                  char_to_room(ch, pexit->to_room);
                  do_look(ch, "auto");
                  char_from_room(ch);
                  char_to_room(ch, original);
                  return;
               }
            }
            send_to_char("You see swirling chaos...\n\r", ch);
            break;
         case ITEM_SHEATH:
            ch_printf(ch, "It's a sheath for %s weapons and holds a max weight of %d.\n\r", get_weapontype(obj->value[1]), obj->value[0]);
         case ITEM_CONTAINER:
         case ITEM_QUIVER:
         case ITEM_CORPSE_NPC:
         case ITEM_CORPSE_PC:
            if (IS_SET(obj->value[1], CONT_CLOSED) && obj->item_type != ITEM_SHEATH)
            {
               send_to_char("It is closed.\n\r", ch);
               break;
            }

         case ITEM_KEYRING:
            count = obj->count;
            obj->count = 1;
            if (obj->item_type == ITEM_CONTAINER)
               act(AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR);
            else if (obj->item_type == ITEM_CORPSE_PC)
               ch_printf(ch, "The PC corpse contains:\n\r");
            else
               act(AT_PLAIN, "$p holds:", ch, obj, NULL, TO_CHAR);
            obj->count = count;
            show_list_to_char_type( obj->first_content, ch, TRUE, TRUE, atoi(argument), eItemNothing );
            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            break;
      }
      return;
   }

   if ((pdesc = get_extra_descr(arg1, ch->in_room->first_extradesc)) != NULL)
   {
      send_to_char_color(pdesc, ch);
      return;
   }

   door = get_door(arg1);
   if ((pexit = find_door(ch, arg1, TRUE)) != NULL)
   {
      if (IS_SET(pexit->exit_info, EX_CLOSED) && !IS_SET(pexit->exit_info, EX_WINDOW))
      {
         if ((IS_SET(pexit->exit_info, EX_SECRET) || IS_SET(pexit->exit_info, EX_DIG)) && door != -1)
            send_to_char("Nothing special there.\n\r", ch);
         else
            act(AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
         return;
      }
      if (IS_SET(pexit->exit_info, EX_BASHED))
         act(AT_RED, "The $d has been bashed from its hinges!", ch, NULL, pexit->keyword, TO_CHAR);

      if (pexit->description && pexit->description[0] != '\0')
         send_to_char(pexit->description, ch);
      else
         send_to_char("Nothing special there.\n\r", ch);

      /*
       * Ability to look into the next room   -Thoric
       */
      if (pexit->to_room
         && (IS_AFFECTED(ch, AFF_SCRYING) || IS_SET(pexit->exit_info, EX_xLOOK) || get_trust(ch) >= LEVEL_IMMORTAL))
      {
         if (!IS_SET(pexit->exit_info, EX_xLOOK) && get_trust(ch) < LEVEL_IMMORTAL)
         {
            set_char_color(AT_MAGIC, ch);
            send_to_char("You attempt to scry...\n\r", ch);
            /*
             * Change by Narn, Sept 96 to allow characters who don't have the
             * scry spell to benefit from objects that are affected by scry.
             */
            if (!IS_NPC(ch))
            {
               int percent = LEARNED(ch, skill_lookup("scry"));

               if (!percent)
               {
                  percent = 55; /* 95 was too good -Thoric */
               }

               if (number_percent() > percent)
               {
                  send_to_char("You fail.\n\r", ch);
                  return;
               }
            }
         }
         if (room_is_private(pexit->to_room) && get_trust(ch) < sysdata.level_override_private)
         {
            set_char_color(AT_WHITE, ch);
            send_to_char("That room is private buster!\n\r", ch);
            return;
         }
         if (!IS_NPC(ch))
         {
            if (xIS_SET(pexit->to_room->room_flags, ROOM_IMP) && ch->pcdata->caste < caste_Staff)
            {
               set_char_color(AT_WHITE, ch);
               send_to_char("That room is for staff only!\n\r", ch);
               return;
            }
         }
         else
         {
            if (xIS_SET(pexit->to_room->room_flags, ROOM_IMP))
               return;
         }
         original = ch->in_room;
         char_from_room(ch);
         char_to_room(ch, pexit->to_room);
         do_look(ch, "auto");
         char_from_room(ch);
         char_to_room(ch, original);
      }
      return;
   }
   else if (door != -1)
   {
      send_to_char("Nothing special there.\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) != NULL)
   {
      show_char_to_char_1(victim, ch, 0);
      return;
   }


   /* finally fixed the annoying look 2.obj desc bug -Thoric */
   number = number_argument(arg1, arg);
   for (cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content)
   {
      if (can_see_obj(ch, obj))
      {
         if (IS_OBJ_STAT(obj, ITEM_ONMAP))
         {
            if (!IS_ONMAP_FLAG(ch))
               return;

            if (ch->map != obj->map
               || ch->coord->x != obj->coord->x
               || ch->coord->y != obj->coord->y)
            {
               send_to_char("You do not see that here.\n\r", ch);
               return;
            }
         }
         if ((pdesc = get_extra_descr(arg, obj->first_extradesc)) != NULL)
         {
            if ((cnt += obj->count) < number)
               continue;
            send_to_char_color(pdesc, ch);
            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            return;
         }

         if ((pdesc = get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL)
         {
            if ((cnt += obj->count) < number)
               continue;
            send_to_char_color(pdesc, ch);
            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            return;
         }
         if (nifty_is_name_prefix(arg, obj->name))
         {
            if ((cnt += obj->count) < number)
               continue;
            pdesc = get_extra_descr(obj->name, obj->pIndexData->first_extradesc);
            if (!pdesc)
               pdesc = get_extra_descr(obj->name, obj->first_extradesc);
            if (!pdesc)
               send_to_char_color("You see nothing special.\r\n", ch);
            else
               send_to_char_color(pdesc, ch);
            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            return;
         }
      }
   }

   for (obj = ch->in_room->last_content; obj; obj = obj->prev_content)
   {
      if (can_see_obj(ch, obj))
      {
         if (IS_OBJ_STAT(obj, ITEM_ONMAP))
         {
            if (!IS_ONMAP_FLAG(ch))
               return;

            if (ch->map != obj->map
               || ch->coord->x != obj->coord->x
               || ch->coord->y != obj->coord->y)
            {
               send_to_char("You do not see that here.\n\r", ch);
               return;
            }
         }
         if ((pdesc = get_extra_descr(arg, obj->first_extradesc)) != NULL)
         {
            if ((cnt += obj->count) < number)
               continue;
            send_to_char_color(pdesc, ch);
            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            return;
         }

         if ((pdesc = get_extra_descr(arg, obj->pIndexData->first_extradesc)) != NULL)
         {
            if ((cnt += obj->count) < number)
               continue;
            send_to_char_color(pdesc, ch);
            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            return;
         }
         if (nifty_is_name_prefix(arg, obj->name))
         {
            if ((cnt += obj->count) < number)
               continue;
            pdesc = get_extra_descr(obj->name, obj->pIndexData->first_extradesc);
            if (!pdesc)
               pdesc = get_extra_descr(obj->name, obj->first_extradesc);
            if (!pdesc)
               send_to_char("You see nothing special.\r\n", ch);
            else
               send_to_char_color(pdesc, ch);
            if (doexaprog)
               oprog_examine_trigger(ch, obj);
            return;
         }
      }
   }

   send_to_char("You do not see that here.\n\r", ch);
   return;
}
char *get_look_skin_color(CHAR_DATA *victim)
{
   int race = victim->race;
   int value = victim->pcdata->skincolor;
   
   if (race == RACE_HUMAN || race == RACE_ELF || race == RACE_DWARF || race == RACE_HOBBIT)
   {
      if (value == 1)
         return "Pale White";
      if (value == 2)
         return "Fair White";
      if (value == 3)
         return "White";
      if (value == 4)
         return "Light Tan";
      if (value == 5)
         return "Moderate Tan";
      if (value == 6)
         return "Tanned";
      if (value == 7)
         return "Soft Brown";
      if (value == 8)
         return "Light Brown";
      if (value == 9)
         return "Fair Brown";
      if (value == 10)
         return "Brown";
      if (value == 11)
         return "Light Black";
      if (value == 12)
         return "Fair Black";
      if (value == 13)
         return "Black";
      if (value == 14)
         return "Dark Black";
      if (value == 15)
         return "Pure Black";
   }
   if (race == RACE_OGRE)
   {
      if (value == 1)
         return "Pale Green";
      if (value == 2)
         return "Fair Green";
      if (value == 3)
         return "Green";
      if (value == 4)
         return "Dark Green";
      if (value == 5)
         return "Pure Green";
      if (value == 6)
         return "Soft Brown";
      if (value == 7)
         return "Light Brown";
      if (value == 8)
         return "Fair Brown";
      if (value == 9)
         return "Brown";
      if (value == 10)
         return "Light Black";
      if (value == 11)
         return "Fair Black";
      if (value == 12)
         return "Black";
      if (value == 13)
         return "Dark Black";
      if (value == 14)
         return "Pure Black";
   }
   if (race == RACE_FAIRY)
   {
      if (value == 1)
         return "Pale Blue";
      if (value == 2)
         return "Fair Blue";
      if (value == 3)
         return "Blue";
      if (value == 4)
         return "Dark Blue";
      if (value == 5)
         return "Pure Blue";
      if (value == 6)
         return "Pale Green";
      if (value == 7)
         return "Fair Green";
      if (value == 8)
         return "Green";
      if (value == 9)
         return "Dark Green";
      if (value == 10)
         return "Pure Green";
      if (value == 11)
         return "Pale Pink";
      if (value == 12)
         return "Fair Pink";
      if (value == 13)
         return "Pink";
      if (value == 14)
         return "Pale Red";
      if (value == 15)
         return "Fair Red";
   }
   bug("get_look_skin_color: reached end of function on %s", victim->name);
   return "White";
}

char *get_look_hair_color(CHAR_DATA *victim)
{
   int value = victim->pcdata->haircolor;
   
   switch (value)
   {
      case 1:  return "Soft White";
      case 2:  return "Fair White";
      case 3:  return "White";
      case 4:  return "Soft Grey";
      case 5:  return "Fair Grey";
      case 6:  return "Grey";
      case 7:  return "Dark Grey";
      case 8:  return "Soft Brown";
      case 9:  return "Fair Brown";
      case 10: return "Brown";
      case 11: return "Dark Brown";
      case 12: return "Soft Black";
      case 13: return "Fair Black";
      case 14: return "Black";
      case 15: return "Dark Black";
      case 16: return "Pure Black";
      case 17: return "Soft Blonde";
      case 18: return "Fair Blonde";
      case 19: return "Blonde";
      case 20: return "True Blonde";
      case 21: return "Soft Red";
      case 22: return "Fair Red";
      case 23: return "Red";
      case 24: return "Dark Red";
      case 25: return "Pure Red";
      case 26: return "Soft Green";
      case 27: return "Fair Green";
      case 28: return "Green";
      case 29: return "Dark Green";
      case 30: return "Pure Green";
      case 31: return "Soft Blue";
      case 32: return "Fair Blue";
      case 33: return "Blue";
      case 34: return "Dark Blue";
      case 35: return "Pure Blue";
      case 36: return "Soft Purple";
      case 37: return "Fair Purple";
      case 38: return "Purple";
      case 39: return "Dark Purple";
      case 40: return "Pure Purple";
      default:
         bug("get_look_hair_color: Invalid value for %s", victim->name);
         return "Black";
   }
   return "Black"; 
}

char *get_hair_info(CHAR_DATA *victim)
{
   int length = victim->pcdata->hairlength;
   int style = victim->pcdata->hairstyle;
   static char buf[MIL];
   
   if (length == 1)  //bald
      return "to have no hair of any length on $s head";
      
   switch (style)
   {
      case 1:  sprintf(buf, "to have straight, thin hair "); break;
      case 2:  sprintf(buf, "to have straight, normal hair "); break;
      case 3:  sprintf(buf, "to have straight, think hair "); break;
      case 4:  sprintf(buf, "to have curled, thin hair "); break;
      case 5:  sprintf(buf, "to have curled, normal hair "); break;
      case 6:  sprintf(buf, "to have curled, think hair "); break;
      case 7:  sprintf(buf, "to have permed hair "); break;
      case 8:  sprintf(buf, "to have spiked hair "); break;
      case 9:  sprintf(buf, "to have a mohawk "); break;
      case 10: sprintf(buf, "to have a straight mullet "); break;
      case 11: sprintf(buf, "to have a curled mullet "); break;
      case 12: sprintf(buf, "to have a permed mullet "); break;
      case 13: sprintf(buf, "to have dreadlocks "); break;
      case 14: sprintf(buf, "to have braided hair "); break;
      case 15: sprintf(buf, "to have a fluffy afro "); break;
      default: 
         bug("get_hair_info: invalid style on %s", victim->name);
         sprintf(buf, "to have straight, normal hair "); 
         break;
   }
   switch (length)
   {
     case 2:  strcat(buf, "that is freshly shaved"); break;
     case 3:  strcat(buf, "that has light stubbles"); break;
     case 4:  strcat(buf, "that has a length of a crew cut"); break;
     case 5:  strcat(buf, "that has a short length"); break;
     case 6:  strcat(buf, "that can drop down to the temples"); break;
     case 7:  strcat(buf, "that can drop down below the ears"); break;
     case 8:  strcat(buf, "that can drop down to the shoulders"); break;
     case 9:  strcat(buf, "that drops down to the upper back"); break;
     case 10: strcat(buf, "that drops down to the middle back"); break;
     case 11: strcat(buf, "that drops down to the lower back"); break;
     case 12: strcat(buf, "that drops down below the waist"); break;
     case 13: strcat(buf, "that drops down all the way to the floor"); break;
     default:
        bug("get_hair_info: invalid length on %s", victim->name);
        strcat(buf, "that has a short length");
        break;
  }
  return buf;
}

char *get_look_eye_color(CHAR_DATA *victim)
{
   int color = victim->pcdata->eyecolor;
   
   switch (color)
   {
      case 1:  return "Dull Blue"; break;
      case 2:  return "Blue"; break;
      case 3:  return "Radiant Blue"; break;
      case 4:  return "Dull Green"; break;
      case 5:  return "Green"; break;
      case 6:  return "Radiant Green"; break;
      case 7:  return "Dull Brown"; break;
      case 8:  return "Brown"; break;
      case 9:  return "Radiant Brown"; break;
      case 10:  return "Dull Red"; break;
      case 11:  return "Red"; break;
      case 12:  return "Radiant Red"; break;
      case 13:  return "Dull Black"; break;
      case 14:  return "Black"; break;
      case 15:  return "Radiant Black"; break;
      case 16:  return "Dull White"; break;
      case 17:  return "White"; break;
      case 18:  return "Radiant White"; break;
      case 19:  return "Dull Yellow"; break;
      case 20:  return "Yellow"; break;
      case 21:  return "Radiant Yellow"; break;
      case 22:  return "Dull Purple"; break;
      case 23:  return "Purple"; break;
      case 24:  return "Radiant Purple"; break;
      default:
         bug("get_look_eye_color: Invalid color on %s", victim->name);
         return "Blue";
   }

}     

char *get_look_height(CHAR_DATA *victim)
{
   int height = victim->pcdata->cheight;
   
   switch(height)
   {
      case 1:  return "Extremely Short";
      case 2:  return "Moderately Short";
      case 3:  return "Adequately Short";
      case 4:  return "Short";
      case 5:  return "Below Average in Height";
      case 6:  return "Slightly Below Average in Height";
      case 7:  return "of Average Height";
      case 8:  return "Slightly Above Average in Height";
      case 9:  return "Above Average in Height";
      case 10: return "Tall";
      case 11: return "Adequately Towering in Height";
      case 12: return "Moderately Towering in Height";
      case 13: return "Extremely Towering in Height";
      default:
         bug("get_look_height: Invalid height on %s", victim->name);
         return "of Average Height";
   }
   return "of Average Height";
}

char *get_look_weight(CHAR_DATA *victim)
{
   int weight = victim->pcdata->cweight;
   
   switch(weight)
   {
      case 1:  return "Extremely Slender";
      case 2:  return "Moderately Slender";
      case 3:  return "Adequately Slender";
      case 4:  return "Slender";
      case 5:  return "Below Average in Build";
      case 6:  return "Slightly Below Average in Build";
      case 7:  return "of Average Build";
      case 8:  return "Slightly Above Average in Build";
      case 9:  return "Above Average in Build";
      case 10: return "Thick";
      case 11: return "Adequately Thick in Build";
      case 12: return "Moderately Thick in Build";
      case 13: return "Extremely Thick in Build";
      default:
         bug("get_look_height: Invalid weight on %s", victim->name);
         return "of Average Build";
   }
   return "of Average Build";
}
  
void show_race_line(CHAR_DATA * ch, CHAR_DATA * victim)
{
   char buf[MSL];
   int feet, inches, weight;

   if (!IS_NPC(victim) && (!get_wear_hidden_cloak(victim) || (get_wear_hidden_cloak(victim) && ch == victim)))
   {
      send_to_char("----------------------------------------------------------------------------\n\r", ch);
      sprintf(buf, "You are looking at %s %s with %s hair.", get_look_skin_color(victim), capitalize(print_npc_race(victim->race)), get_look_hair_color(victim));
      act(AT_ACTION, buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$E appears %s.", get_hair_info(victim));
      act(AT_ACTION, buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "Gazing into $S eyes you notice they are %s.", get_look_eye_color(victim));
      act(AT_ACTION, buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$E appears to be %s and %s.", get_look_height(victim), get_look_weight(victim));
      act(AT_ACTION, buf, ch, NULL, victim, TO_CHAR);
      send_to_char("----------------------------------------------------------------------------\n\r", ch);
   }
   if (!IS_NPC(victim) && (victim == ch))
   {
      feet = victim->height / 12;
      inches = victim->height % 12;
      sprintf(buf, "You are %d'%d\" and weigh %d pounds.\n\r", feet, inches, victim->weight);
      send_to_char(buf, ch);
      return;
   }
   if (!IS_NPC(victim) && (victim != ch))
   {
      inches = victim->height  / 4;
      inches = inches * 4;
      weight = victim->weight / 10;
      weight = weight * 10;
      feet = inches / 12;
      inches = inches % 12;
      sprintf(buf, "%s appears to be about %d'%d\" and weights around %d pounds.\n\r", PERS_MAP(victim, ch), feet, inches, weight);     
      send_to_char(buf, ch);
      return;
   }

}


void show_condition(CHAR_DATA * ch, CHAR_DATA * victim)
{
   char buf[MSL];
   int percent;

   if (victim->max_hit > 0)
      percent = (100 * victim->hit) / victim->max_hit;
   else
      percent = -1;


   if (victim != ch)
   {
      strcpy(buf, PERS(victim, ch));
      if (percent >= 100)
         strcat(buf, " is in perfect health.\n\r");
      else if (percent >= 90)
         strcat(buf, " is slightly scratched.\n\r");
      else if (percent >= 80)
         strcat(buf, " has a few bruises.\n\r");
      else if (percent >= 70)
         strcat(buf, " has some cuts.\n\r");
      else if (percent >= 60)
         strcat(buf, " has several wounds.\n\r");
      else if (percent >= 50)
         strcat(buf, " has many nasty wounds.\n\r");
      else if (percent >= 40)
         strcat(buf, " is bleeding freely.\n\r");
      else if (percent >= 30)
         strcat(buf, " is covered in blood.\n\r");
      else if (percent >= 20)
         strcat(buf, " is leaking guts.\n\r");
      else if (percent >= 10)
         strcat(buf, " is almost dead.\n\r");
      else
         strcat(buf, " is DYING.\n\r");
   }
   else
   {
      strcpy(buf, "You");
      if (percent >= 100)
         strcat(buf, " are in perfect health.\n\r");
      else if (percent >= 90)
         strcat(buf, " are slightly scratched.\n\r");
      else if (percent >= 80)
         strcat(buf, " have a few bruises.\n\r");
      else if (percent >= 70)
         strcat(buf, " have some cuts.\n\r");
      else if (percent >= 60)
         strcat(buf, " have several wounds.\n\r");
      else if (percent >= 50)
         strcat(buf, " have many nasty wounds.\n\r");
      else if (percent >= 40)
         strcat(buf, " are bleeding freely.\n\r");
      else if (percent >= 30)
         strcat(buf, " are covered in blood.\n\r");
      else if (percent >= 20)
         strcat(buf, " are leaking guts.\n\r");
      else if (percent >= 10)
         strcat(buf, " are almost dead.\n\r");
      else
         strcat(buf, " are DYING.\n\r");
   }

   buf[0] = UPPER(buf[0]);
   send_to_char(buf, ch);
   return;
}

//Used to look at the player without telling the place.  Gives the basic description
//of the player but not the inventory -- Xerves
void do_glance(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   CHAR_DATA *victim;

   if (!ch->desc)
      return;

   if (ch->position < POS_SLEEPING)
   {
      send_to_char("You can't see anything but stars!\n\r", ch);
      return;
   }

   if (ch->position == POS_SLEEPING)
   {
      send_to_char("You can't see anything, you're sleeping!\n\r", ch);
      return;
   }

   if (!check_blind(ch))
      return;

   set_char_color(AT_ACTION, ch);
   argument = one_argument(argument, arg1);

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("They're not here.\n\r", ch);
      return;
   }
   else
   {
      show_char_to_char_1(victim, ch, 1);
      return;
   }
   return;
}

void do_viewmount(CHAR_DATA * ch, char *argument)
{
   int found = 0;
   CHAR_DATA *mount;
   char buf[MSL];
   char percent[MSL];
   
   if (IS_NPC(ch))
   {
      send_to_char("This is for PCs only.\n\r", ch);
      return;
   }

   for (mount = ch->in_room->first_person; mount; mount = mount->next_in_room)
   {
      if (xIS_SET(mount->act, ACT_MOUNTSAVE) && ch->pcdata->mount && (ch->pcdata->mount->description == mount->description))
         if (can_see(ch, mount))
         {
            found = 1;
            break;
         }
   }
   if (found == 0)
   {
      send_to_char("Your mount is not in this room to view or you cannot see it.\n\r", ch);
      return;
   }
   if (mount->m1 <= 200)
      sprintf(percent, "&G&W&R*****");
   else if (mount->m1 <= 400)
      sprintf(percent, "&G&W*&R****");
   else if (mount->m1 <= 600)
      sprintf(percent, "&G&W**&R***");
   else if (mount->m1 <= 800)
      sprintf(percent, "&G&W***&R**");
   else if (mount->m1 < 1000)
      sprintf(percent, "&G&W****&R*");
   else
      sprintf(percent, "&G&W&R*****");
   sprintf(buf, "&G&WEndurance &r[&c&w %-2.2d: %s &r]    &G&WMovement  &r[&c&w %-4.4d &r]    &G&WHp  &r[&c&w %-3.3d &r]\n\r", mount->mover, percent, mount->move, mount->hit);
   send_to_char(buf, ch);
   return;
}

//pardons a person for attacking, stealing, etc
void do_pardon(CHAR_DATA * ch, char *argument)
{
   INTRO_DATA *intro;
   CHAR_DATA *victim;
   
   if (IS_NPC(ch))
   {
      send_to_char("NPCs cannot use this command.\n\r", ch);
      return;
   }
   if (!(victim = get_char_room_new(ch, argument, 1))) 
   {
      send_to_char("That person is not here in this room.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPCs.\n\r", ch);
      return;
   }
   if (!can_see(ch, victim))
   {
      send_to_char("It helps if you can see who you want to pardon.\n\r", ch);
      return;
   }
   if (!can_see_intro(ch, victim))
   {
      send_to_char("You don't know who that is, why pardon them?\n\r", ch);
      return;
   }
   
   for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
   {
      if (intro->pid == victim->pcdata->pid)
      {
         if (intro->value < 0)
         {
            intro->value *=-1;
            REMOVE_BIT(intro->flags, INTRO_ATTACKER);
            REMOVE_BIT(intro->flags, INTRO_KILLER);
            REMOVE_BIT(intro->flags, INTRO_THIEF);
            REMOVE_BIT(intro->flags, INTRO_MYATTACKER);
            REMOVE_BIT(intro->flags, INTRO_MYKILLER);
            REMOVE_BIT(intro->flags, INTRO_MYTHIEF);
            ch_printf(ch, "You pardon %s's past.\n\r", victim->name);
            return;
         }
      }
   }
   send_to_char("There is nothing to pardon.\n\r", ch);
   return;
}

void do_examine(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   OBJ_DATA *obj;
   BOARD_DATA *board;
   int dam;
   sh_int value;
   sh_int f1 = 0;
   sh_int f2 = 0;
   sh_int f3 = 0;
   sh_int f4 = 0;

   if (!argument)
   {
      bug("do_examine: null argument.", 0);
      return;
   }

   if (!ch)
   {
      bug("do_examine: null ch.", 0);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Examine what?\n\r", ch);
      return;
   }

   sprintf(buf, "%s noprog", arg);
   do_look(ch, buf);

   /*
    * Support for looking at boards, checking equipment conditions,
    * and support for trigger positions by Thoric
    */
   if ((obj = get_obj_here(ch, arg)) != NULL)
   {
      if ((board = get_board(obj)) != NULL)
      {
         if (board->num_posts)
            ch_printf(ch, "There are about %d notes posted here.  Type 'note list' to list them.\n\r", board->num_posts);
         else
            send_to_char("There aren't any notes posted here.\n\r", ch);
      }

      switch (obj->item_type)
      {
         default:
            break;
            
         case ITEM_REPAIR:
            if (obj->value[1] == 0)
               dam = 1;
            else
               dam = obj->value[0] * 10 / obj->value[1];
            strcpy(buf, "As you look more closely, you notice that it is ");
            if (dam >= 10)
               strcat(buf, "in superb condition.");
            else if (dam == 9)
               strcat(buf, "in very good condition.");
            else if (dam == 8)
               strcat(buf, "in good shape.");
            else if (dam == 7)
               strcat(buf, "showing a bit of wear.");
            else if (dam == 6)
               strcat(buf, "a little run down.");
            else if (dam == 5)
               strcat(buf, "in need of replacement.");
            else if (dam == 4)
               strcat(buf, "in great need of replacement.");
            else if (dam == 3)
               strcat(buf, "in dire need of replacement.");
            else if (dam == 2)
               strcat(buf, "very badly worn.");
            else if (dam == 1)
               strcat(buf, "practically worthless.");
            else if (dam <= 0)
               strcat(buf, "nearly broken.");
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
            break;

         case ITEM_ARMOR:
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))
            {
               if (obj->value[1] == 0)
                  dam = 1;
               else
                  dam = obj->value[0] * 10 / obj->value[1];
            }
            else
            { 
               dam = (obj->value[3] * 10 / INIT_ARMOR_CONDITION);
            }
            strcpy(buf, "As you look more closely, you notice that it is ");
            if (dam >= 10)
               strcat(buf, "in superb condition.");
            else if (dam == 9)
               strcat(buf, "in very good condition.");
            else if (dam == 8)
               strcat(buf, "in good shape.");
            else if (dam == 7)
               strcat(buf, "showing a bit of wear.");
            else if (dam == 6)
               strcat(buf, "a little run down.");
            else if (dam == 5)
               strcat(buf, "in need of repair.");
            else if (dam == 4)
               strcat(buf, "in great need of repair.");
            else if (dam == 3)
               strcat(buf, "in dire need of repair.");
            else if (dam == 2)
               strcat(buf, "very badly worn.");
            else if (dam == 1)
               strcat(buf, "practically worthless.");
            else if (dam <= 0)
               strcat(buf, "nearly broken.");
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
            break;

         case ITEM_WEAPON:
            dam = (obj->value[0] * 10 / INIT_ARMOR_CONDITION);
            strcpy(buf, "As you look more closely, you notice that it is ");
            if (dam >= 10)
               strcat(buf, "in superb condition.");
            else if (dam == 9)
               strcat(buf, "in very good condition.");
            else if (dam == 8)
               strcat(buf, "in good shape.");
            else if (dam == 7)
               strcat(buf, "showing a bit of wear.");
            else if (dam == 6)
               strcat(buf, "a little run down.");
            else if (dam == 5)
               strcat(buf, "in need of repair.");
            else if (dam == 4)
               strcat(buf, "in great need of repair.");
            else if (dam == 3)
               strcat(buf, "in dire need of repair.");
            else if (dam == 2)
               strcat(buf, "very badly worn.");
            else if (dam == 1)
               strcat(buf, "practically worthless.");
            else if (dam <= 0)
               strcat(buf, "nearly broken.");
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
            break;
         case ITEM_COOK:
            strcpy(buf, "As you examine it carefully you notice that it ");
            dam = obj->value[2];
            if (dam >= 3)
               strcat(buf, "is burned to a crisp.");
            else if (dam == 1)
               strcat(buf, "is a little over cooked.");
            else if (dam == 1)
               strcat(buf, "is perfectly roasted.");
            else
               strcat(buf, "is raw.");
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
         case ITEM_FOOD:
            if (obj->timer > 0 && obj->value[1] > 0)
               dam = (obj->timer * 10) / obj->value[1];
            else
               dam = 10;
            if (obj->item_type == ITEM_FOOD)
               strcpy(buf, "As you examine it carefully you notice that it ");
            else
               strcpy(buf, "Also it ");
            if (dam >= 10)
               strcat(buf, "is fresh.");
            else if (dam == 9)
               strcat(buf, "is nearly fresh.");
            else if (dam == 8)
               strcat(buf, "is perfectly fine.");
            else if (dam == 7)
               strcat(buf, "looks good.");
            else if (dam == 6)
               strcat(buf, "looks ok.");
            else if (dam == 5)
               strcat(buf, "is a little stale.");
            else if (dam == 4)
               strcat(buf, "is a bit stale.");
            else if (dam == 3)
               strcat(buf, "smells slightly off.");
            else if (dam == 2)
               strcat(buf, "smells quite rank.");
            else if (dam == 1)
               strcat(buf, "smells revolting!");
            else if (dam <= 0)
               strcat(buf, "is crawling with maggots!");
            strcat(buf, "\n\r");
            send_to_char(buf, ch);
            break;


         case ITEM_SWITCH:
         case ITEM_LEVER:
         case ITEM_PULLCHAIN:
            if (IS_SET(obj->value[0], TRIG_UP))
               send_to_char("You notice that it is in the up position.\n\r", ch);
            else
               send_to_char("You notice that it is in the down position.\n\r", ch);
            break;
         case ITEM_FURNITURE:
            if (IS_SET(obj->value[2], SIT_ON) && !IS_SET(obj->value[2], SIT_AT))
               f1 = 1;
            if (IS_SET(obj->value[2], SIT_IN) && !IS_SET(obj->value[2], SIT_ON) && !IS_SET(obj->value[2], SIT_AT))
               f1 = 2;
            if (IS_SET(obj->value[2], SIT_AT))
               f1 = 3;

            if (IS_SET(obj->value[2], STAND_ON) && !IS_SET(obj->value[2], STAND_AT))
               f2 = 1;
            if (IS_SET(obj->value[2], STAND_IN) && !IS_SET(obj->value[2], STAND_ON) && !IS_SET(obj->value[2], STAND_AT))
               f2 = 2;
            if (IS_SET(obj->value[2], STAND_AT))
               f2 = 3;

            if (IS_SET(obj->value[2], REST_ON) && !IS_SET(obj->value[2], REST_AT))
               f3 = 1;
            if (IS_SET(obj->value[2], REST_IN) && !IS_SET(obj->value[2], REST_ON) && !IS_SET(obj->value[2], REST_AT))
               f3 = 2;
            if (IS_SET(obj->value[2], REST_AT))
               f3 = 3;

            if (IS_SET(obj->value[2], SLEEP_ON) && !IS_SET(obj->value[2], SLEEP_AT))
               f4 = 1;
            if (IS_SET(obj->value[2], SLEEP_IN) && !IS_SET(obj->value[2], SLEEP_ON) && !IS_SET(obj->value[2], SLEEP_AT))
               f4 = 2;
            if (IS_SET(obj->value[2], SLEEP_AT))
               f4 = 3;

            if (f1 > 0 || f2 > 0 || f3 > 0 || f4 > 0)
            {
               sprintf(buf, "\n\rYou can ");
               if (f1 == 1)
                  strcat(buf, "sit on it");
               if (f1 == 2)
                  strcat(buf, "sit in it");
               if (f1 == 3)
                  strcat(buf, "sit at it");
               if (f2 > 0 || f3 > 0 || f4 > 0)
               {
                  if (f3 < 1 && f4 < 1)
                  {
                     strcat(buf, ", and");
                     if (f2 == 1)
                        strcat(buf, " stand on it");
                     if (f2 == 2)
                        strcat(buf, " stand in it");
                     if (f2 == 3)
                        strcat(buf, " stand at it");
                  }
                  else
                  {
                     if (f2 == 1)
                        strcat(buf, ", stand on it");
                     if (f2 == 2)
                        strcat(buf, ", stand in it");
                     if (f2 == 3)
                        strcat(buf, ", stand at it");
                  }
               }
               if (f3 > 0 || f4 > 0)
               {
                  if (f4 < 1)
                  {
                     strcat(buf, ", and");
                     if (f3 == 1)
                        strcat(buf, " rest on it");
                     if (f3 == 2)
                        strcat(buf, " rest in it");
                     if (f3 == 3)
                        strcat(buf, " rest at it");
                  }
                  else
                  {
                     if (f3 == 1)
                        strcat(buf, ", rest on it");
                     if (f3 == 2)
                        strcat(buf, ", rest in it");
                     if (f3 == 3)
                        strcat(buf, ", rest at it");
                  }
               }
               if (f4 > 0)
               {
                  if (f4 == 1)
                     strcat(buf, ", and sleep on it");
                  if (f4 == 2)
                     strcat(buf, ", and sleep in it");
                  if (f4 == 3)
                     strcat(buf, ", and sleep at it");
               }
               strcat(buf, ".\n\r");
            }
            strcat(buf, "Also, it seems it ");
            dam = ((obj->value[0] / 3) + 2);
            if (obj->value[0] < 2)
               dam = 1;
            if (obj->value[0] == 2)
               dam = 2;
            if (dam >= 6)
               strcat(buf, "can hold a huge party!");
            if (dam == 5)
               strcat(buf, "can hold a great number.");
            if (dam == 4)
               strcat(buf, "can hold a few.");
            if (dam == 3)
               strcat(buf, "can hold a family.");
            if (dam == 2)
               strcat(buf, "can hold only 2 people.");
            if (dam <= 1)
               strcat(buf, "can hold only 1 person.");

            dam = ((obj->value[1] / 300) + 1);
            if (obj->value[1] < 201)
               dam = 1;
            if (obj->value[1] == 0)
               dam = 3;
            if (dam == 7)
               strcat(buf, "\n\rIn addition, it appears to be able to hold the weight of the immortal's egos.");
            if (dam == 6)
               strcat(buf, "\n\rIn addition, it appears to be able to hold the weight of an army.");
            if (dam == 5)
               strcat(buf, "\n\rIn addition, it appears to be able to hold the weight of a tank.");
            if (dam == 4)
               strcat(buf, "\n\rIn addition, it appears to be able to hold the weight of an elephant.");
            if (dam == 3)
               strcat(buf, "\n\rIn addition, it appears to be able to hold the weight of a horse.");
            if (dam == 2)
               strcat(buf, "\n\rIn addition, it appears to be able to hold the weight of a few humans.");
            if (dam <= 1)
               strcat(buf, "\n\rIn addition, it appears to be able to hold a slight amount of weight.");

            dam = ((obj->value[3] * obj->value[4]) / 200);

            if (obj->value[3] == 0 && obj->value[4] != 0)
               dam = ((100 * obj->value[4]) / 200);
            if (obj->value[3] != 0 && obj->value[4] == 0)
               dam = ((obj->value[3] * 100) / 200);
            if (obj->value[3] == 0 && obj->value[4] == 0)
               dam = 100;

            if (dam > 1500)
               strcat(buf, "\n\rFinally, it appears to be the point where the power of healing first started.\n\r");
            if (dam <= 1500 && dam >= 1001)
               strcat(buf, "\n\rFinally, it appears to be a piece of furniture soaked in healing potions.\n\r");
            if (dam <= 1000 && dam >= 701)
               strcat(buf, "\n\rFinally, it appears to be a magical rejuvenating center.\n\r");
            if (dam <= 700 && dam >= 550)
               strcat(buf, "\n\rFinally, it appears to be a restoratin center.\n\r");
            if (dam <= 549 && dam >= 400)
               strcat(buf, "\n\rFinally, it appears to be a fallen cloud.\n\r");
            if (dam <= 399 && dam >= 300)
               strcat(buf, "\n\rFinally, it appears to be a great place to sleep.\n\r");
            if (dam <= 299 && dam >= 200)
               strcat(buf, "\n\rFinally, it appears to be a well crafted piece of furniture.\n\r");
            if (dam <= 199 && dam >= 150)
               strcat(buf, "\n\rFinally, it appears to be a comfortable looking piece of furniture.\n\r");
            if (dam <= 149 && dam >= 100)
               strcat(buf, "\n\rFinally, it appears to be a typical crafted piece of furniture.\n\r");
            if (dam <= 99 && dam >= 66)
               strcat(buf, "\n\rFinally, it appears to be used and worn in.\n\r");
            if (dam <= 65 && dam >= 50)
               strcat(buf, "\n\rFinally, it appears to be almost broke and looks uncomforable.\n\r");
            if (dam <= 49 && dam >= 35)
               strcat(buf, "\n\rFinally, it appears to be a bed made out of sharp rocks.\n\r");
            if (dam <= 34 && dam >= 25)
               strcat(buf, "\n\rFinally, it looks like a weapon, eh it is furniture?\n\r");
            if (dam < 25)
               strcat(buf, "\n\rFinally, hmm, this is furniture?  Better to sleep in fire.\n\r");
            send_to_char(buf, ch);
            break;
         case ITEM_EXTRACTOBJ:
            strcpy(buf, "\n\rAs you examine it carefully, you notice that it is used for");
            switch (obj->value[0])
            {
               case 1:
                  strcat(buf, " extracting Gold.\n\r");
                  break;
               case 2:
                  strcat(buf, " extracting Ore.\n\r");
                  break;
               case 3:
                  strcat(buf, " extracting Corn.\n\r");
                  break;
               case 4:
                  strcat(buf, " extracting Grain.\n\r");
                  break;
               case 5:
                  strcat(buf, " extracting Wood.\n\r");
                  break;
               case 6:
                  strcat(buf, " extracting Stone.\n\r");
                  break;
            }
            strcat(buf, "In addition, it appears to have a rating of:\n\r");
            if (obj->value[1] == 0)
               strcat(buf, "   Nothing\n\r");
            if (obj->value[1] < 11 && obj->value[1] > 0) // 1 - 10
               strcat(buf, "   *\n\r");
            if (obj->value[1] < 21 && obj->value[1] > 10) // 11 - 20
               strcat(buf, "   **\n\r");
            if (obj->value[1] < 31 && obj->value[1] > 20) // 21 - 30
               strcat(buf, "   ***\n\r");
            if (obj->value[1] < 41 && obj->value[1] > 30) // 31 - 40
               strcat(buf, "   ****\n\r");
            if (obj->value[1] < 51 && obj->value[1] > 40) // 41 - 50
               strcat(buf, "   *****\n\r");
            if (obj->value[1] < 61 && obj->value[1] > 50) // 51 - 60
               strcat(buf, "   ***** *\n\r");
            if (obj->value[1] < 71 && obj->value[1] > 60) // 61 - 70
               strcat(buf, "   ***** **\n\r");
            if (obj->value[1] < 81 && obj->value[1] > 70) // 71 - 80
               strcat(buf, "   ***** ***\n\r");
            if (obj->value[1] < 91 && obj->value[1] > 80) // 81 - 90
               strcat(buf, "   ***** ****\n\r");
            if (obj->value[1] >= 91)
               strcat(buf, "   ***** *****\n\r"); // 91+
            send_to_char(buf, ch);
            break;

         case ITEM_HOLDRESOURCE:
            strcpy(buf, "\n\rAs you examine if carefully, you notice that it can hold:\n\r");
            switch (obj->value[1])
            {
               case 1:
                  strcat(buf, "   * Gold\n\r");
                  break;
               case 2:
                  strcat(buf, "   * Ore\n\r");
                  break;
               case 3:
                  strcat(buf, "   * Corn\n\r");
                  break;
               case 4:
                  strcat(buf, "   * Grain\n\r");
                  break;
               case 5:
                  strcat(buf, "   * Wood\n\r");
                  break;
               case 6:
                  strcat(buf, "   * Stone\n\r");
                  break;
            }
            switch (obj->value[3])
            {
               case 1:
                  strcat(buf, "   * Gold\n\r");
                  break;
               case 2:
                  strcat(buf, "   * Ore\n\r");
                  break;
               case 3:
                  strcat(buf, "   * Corn\n\r");
                  break;
               case 4:
                  strcat(buf, "   * Grain\n\r");
                  break;
               case 5:
                  strcat(buf, "   * Wood\n\r");
                  break;
               case 6:
                  strcat(buf, "   * Stone\n\r");
                  break;
            }

            strcat(buf, "Also, it appears that the first bin is ");
            if (obj->value[2] > 0)
            {
               if (obj->value[2] == 0)
                  dam = 0;
               else
                  dam = (obj->value[2] * 100 / obj->value[0]);
               if (dam < 25) // 1 - 24
                  strcat(buf, "less than one-quarters full.\n\r");
               if (dam < 50 && dam >= 25) // 25 - 49
                  strcat(buf, "less than half full.\n\r");
               if (dam < 75 && dam >= 50) // 50 - 74
                  strcat(buf, "more than half full.\n\r");
               if (dam >= 75 && dam < 100) // 75 - 99
                  strcat(buf, "almost full.\n\r");
               if (dam == 100)
                  strcat(buf, "completely full.\n\r");
               if (dam > 100)
                  bug("Holdresource: %d has more than it can hold", obj->pIndexData->vnum);
            }
            else
               strcat(buf, "completely empty.\n\r");

            strcat(buf, "Lastly, it appears that the second bin is ");
            if (obj->value[4] > 0)
            {
               if (obj->value[4] == 0)
                  dam = 0;
               else
                  dam = (obj->value[4] * 100 / obj->value[0]);
               if (dam < 25) // 1 - 24
                  strcat(buf, "less than one-quarters full.\n\r");
               if (dam < 50 && dam >= 25) // 25 - 49
                  strcat(buf, "less than half full.\n\r");
               if (dam < 75 && dam >= 50) // 50 - 74
                  strcat(buf, "more than half full.\n\r");
               if (dam >= 75 && dam < 100) // 75 - 99
                  strcat(buf, "almost full.\n\r");
               if (dam == 100)
                  strcat(buf, "completely full.\n\r");
               if (dam > 100)
                  bug("Holdresource: %d has more than it can hold", obj->pIndexData->vnum);
            }
            else
               strcat(buf, "completely empty.\n\r");
            send_to_char(buf, ch);
            break;

         case ITEM_BUTTON:
            if (IS_SET(obj->value[0], TRIG_UP))
               send_to_char("You notice that it is depressed.\n\r", ch);
            else
               send_to_char("You notice that it is not depressed.\n\r", ch);
            break;

/* Not needed due to check in do_look already
	case ITEM_PORTAL:
	    sprintf( buf, "in %s noprog", arg );
	    do_look( ch, buf );
	    break;
*/

         case ITEM_CORPSE_PC:
         case ITEM_CORPSE_NPC:
            {
               sh_int timerfrac = obj->timer;
               
               if (!IS_NPC(ch))
               {
                  int cdays, rtime, cmin, chour;
                  char cbuf[MSL];
                  
                  if (obj->timer > 1440)
                  {
                     cdays = obj->timer / 1440;
                     rtime = obj->timer % 1440;
                  }
                  else
                  {
                     cdays = 0;
                     rtime = obj->timer;
                  }
                  chour = rtime / 60;
                  cmin = rtime % 60;
                  sprintf(cbuf, "Time Left till corpse is gone:  D:%d H:%d M:%d\n\r", cdays, chour, cmin);
                  send_to_char(cbuf, ch);
               }

               if (obj->item_type == ITEM_CORPSE_PC)
                  timerfrac = (int) obj->timer / 480 + 1;

               switch (timerfrac)
               {
                  default:
                     send_to_char("This corpse has recently been slain.\n\r", ch);
                     break;
                  case 4:
                     send_to_char("This corpse was slain a little while ago.\n\r", ch);
                     break;
                  case 3:
                     send_to_char("A foul smell rises from the corpse, and it is covered in flies.\n\r", ch);
                     break;
                  case 2:
                     send_to_char("A writhing mass of maggots and decay, you can barely go near this corpse.\n\r", ch);
                     break;
                  case 1:
                  case 0:
                     send_to_char("Little more than bones, there isn't much left of this corpse.\n\r", ch);
                     break;
               }
            }
         case ITEM_CONTAINER:
            if (IS_OBJ_STAT(obj, ITEM_COVERING))
               break;
         case ITEM_DRINK_CON:
         case ITEM_QUIVER:
         case ITEM_SHEATH:
            send_to_char("When you look inside, you see:\n\r", ch);
         case ITEM_KEYRING:
            value = get_otype(argument);
            if (value > 0)
               sprintf(buf, "in %s noprog %d", arg, value);
            else
               sprintf(buf, "in %s noprog", arg);
            do_look(ch, buf);
            break;
      }
      if (IS_OBJ_STAT(obj, ITEM_COVERING))
      {
         sprintf(buf, "under %s noprog", arg);
         do_look(ch, buf);
      }
      oprog_examine_trigger(ch, obj);
      if (char_died(ch) || obj_extracted(obj))
         return;

      check_for_trap(ch, obj, TRAP_EXAMINE, NEW_TRAP_EXAMINE);
   }
   return;
}

//Used to offer a lastname and accept or deny it by the target...
void do_offername(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: offername offer <player>\n\r", ch);
      send_to_char("Syntax: offername accept [lastname]\n\r", ch);
      send_to_char("Syntax: offername deny <lastname>\n\r", ch);
      return;
   }   
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "offer"))
   {
      if ((victim = get_char_room_new(ch, argument, 1)) == NULL)  
      {
         send_to_char("That person is not here to accept such a title.\n\r", ch);
         return;
      }
      if (victim == ch)
      {
         send_to_char("You cannot offer yourself your lastname.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Cannot make offers to NPCs.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, victim->last_name))
      {
         send_to_char("That person already belongs in your house.\n\r", ch);
         return;
      }
      if (victim->pcdata->offeredlname)
      {
         send_to_char("That person has already been made an offer, they need to first deny it to take yours.\n\r", ch);
         return;
      }
      ch_printf(victim, "%s has made you an offer to join his house of %s\n\r", ch->name, ch->last_name);
      if (victim->pcdata->offeredlname)
         STRFREE(victim->pcdata->offeredlname);  
      victim->pcdata->offeredlname = STRALLOC(ch->last_name);
      ch_printf(ch, "You have offered entry to your house to %s\n\r", victim->name);
      return;
   }
   if (!str_cmp(arg, "deny"))
   {
      if (!ch->pcdata->offeredlname || str_cmp(argument, ch->pcdata->offeredlname))
      {
         send_to_char("You have not been offered that lastname.  To see your offers type offername accept.\n\r", ch);
         return;
      }
      STRFREE(ch->pcdata->offeredlname);
      send_to_char("You deny the offer.\n\r", ch);
      return;
   }       
   if (!str_cmp(arg, "accept"))
   {
      if (argument[0] == '\0')
      {
         if (ch->pcdata->offeredlname)
         {
            ch_printf(ch, "You have been made an offer to join the house of %s\n\r", ch->pcdata->offeredlname);
         }
         else
         {
            send_to_char("You have been made no offers to join any house.\n\r", ch);
            return;
         }
      }
      else
      {
         if (!ch->pcdata->offeredlname)
         {
            send_to_char("You must first have an offer to accept one.\n\r", ch);
            return;
         }
         for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
         {
            if (!str_cmp(victim->last_name, ch->pcdata->offeredlname) && can_see(ch, victim))
               break;
         }
         if (!victim)
         {
            send_to_char("You can only accept an offer if a member of the house is present and can be seen.\n\r", ch);
            return;
         }
         act(AT_WHITE, "$n gladly accepts an invitation from $N's house to join it.", ch, NULL, victim, TO_NOTVICT);
         act(AT_WHITE, "You glady accept an invitation from $N's house to join it.", ch, NULL, victim, TO_CHAR);
         act(AT_WHITE, "$n accepts an invitation to join your house.", ch, NULL, victim, TO_VICT);
         remove_from_lastname_file(ch->last_name, ch->name);
         if (ch->last_name)
            STRFREE(ch->last_name);
         ch->last_name = STRALLOC(victim->last_name);
         write_lastname_file(ch->last_name, ch->name);
         return;
      }
   }
}
int get_bagility args((CHAR_DATA *ch));
char *tiny_affect_loc_name args((int location));

void do_weboutput_score(CHAR_DATA *ch, FILE *fp)
{
   char buf[MSL];
   char pbuf[MSL];
   char caste_name[MSL];
   char agimeter_name[MSL];
   int agimeter;
   int x;
   int y;
   char element_name[MSL];
   char outputbuf[10000];
   char parsebuf[10000];
   AFFECT_DATA *paf;
   int hplayed = ((ch->played + (current_time - ch->pcdata->logon)) / 3600);
   
    /* Caste Info -- Xerves 7/7/90 */
   if (ch->pcdata->caste == 1)
      sprintf(caste_name, "&Y     Serf     &G&W");
   else if (ch->pcdata->caste == 2)
      sprintf(caste_name, "&Y   Peasant    &G&W");
   else if (ch->pcdata->caste == 3)
      sprintf(caste_name, "&Y   Laborer    &G&W");
   else if (ch->pcdata->caste == 4)
      sprintf(caste_name, "&Y  Apprentice  &G&W");
   else if (ch->pcdata->caste == 5)
      sprintf(caste_name, "&Y  Journeyman  &G&W");
   else if (ch->pcdata->caste == 6)
      sprintf(caste_name, "&Y    Master    &G&W");
   else if (ch->pcdata->caste == 7)
      sprintf(caste_name, "&Y   Merchant   &G&W");
   else if (ch->pcdata->caste == 8)
      sprintf(caste_name, "&Y    Trader    &G&W");
   else if (ch->pcdata->caste == 9 && ch->sex == 0)
      sprintf(caste_name, "&Y Businessman  &G&W");
   else if (ch->pcdata->caste == 9 && ch->sex == 1)
      sprintf(caste_name, "&Y Businessman  &G&W");
   else if (ch->pcdata->caste == 9 && ch->sex == 2)
      sprintf(caste_name, "&Y Businesswoman&G&W");

   else if (ch->pcdata->caste == 10)
      sprintf(caste_name, "&Y    Mayor     &G&W");

   else if (ch->pcdata->caste == 11)
      sprintf(caste_name, "&P     Page     &G&W");
   else if (ch->pcdata->caste == 12)
      sprintf(caste_name, "&P    Squire    &G&W");
   else if (ch->pcdata->caste == 13)
      sprintf(caste_name, "&P    Knight    &G&W");
   else if (ch->pcdata->caste == 14)
      sprintf(caste_name, "&P   Baronet    &G&W");
   else if (ch->pcdata->caste == 15)
      sprintf(caste_name, "&P    Baron     &G&W");
   else if (ch->pcdata->caste == 16)
      sprintf(caste_name, "&P     Earl     &G&W");
   else if (ch->pcdata->caste == 17)
      sprintf(caste_name, "&P   Viscount   &G&W");
   else if (ch->pcdata->caste == 18)
      sprintf(caste_name, "&P    Count     &G&W");
   else if (ch->pcdata->caste == 19)
      sprintf(caste_name, "&P     Duke     &G&W");
   else if (ch->pcdata->caste == 20)
      sprintf(caste_name, "&P   Marquis    &G&W");

   else if (ch->pcdata->caste == 21)
      sprintf(caste_name, "&C    Vasal     &G&W");
   else if (ch->pcdata->caste == 22)
      sprintf(caste_name, "&C Lord-Vassal  &G&W");
   else if (ch->pcdata->caste == 23)
      sprintf(caste_name, "&C     Lord     &G&W");
   else if (ch->pcdata->caste == 24)
      sprintf(caste_name, "&C   Hi-Lord    &G&W");
   else if (ch->pcdata->caste == 25)
      sprintf(caste_name, "&C   Captain    &G&W");
   else if (ch->pcdata->caste == 26)
      sprintf(caste_name, "&C   Minister   &G&W");
   else if (ch->pcdata->caste == 27 && ch->sex == 0)
      sprintf(caste_name, "&C    Prince    &G&W");
   else if (ch->pcdata->caste == 27 && ch->sex == 1)
      sprintf(caste_name, "&C    Prince    &G&W");
   else if (ch->pcdata->caste == 27 && ch->sex == 2)
      sprintf(caste_name, "&C   Princess   &G&W");
   else if (ch->pcdata->caste == 28 && ch->sex == 0)
      sprintf(caste_name, "&C     King     &G&W");
   else if (ch->pcdata->caste == 28 && ch->sex == 1)
      sprintf(caste_name, "&C     King     &G&W");
   else if (ch->pcdata->caste == 28 && ch->sex == 2)
      sprintf(caste_name, "&C    Queen     &G&W");
   else if (ch->pcdata->caste == 29)
      sprintf(caste_name, "&O    Avatar    &G&W");
   else if (ch->pcdata->caste == 30)
      sprintf(caste_name, "&O    Legend    &G&W");

   else if (ch->pcdata->caste == 31)
      sprintf(caste_name, "&G&W   Ascender   &G&W");
   else if (ch->pcdata->caste == 32)
      sprintf(caste_name, "&G&W   Immortal   &G&W");
   else if (ch->pcdata->caste == 33)
      sprintf(caste_name, "&G&W     God      &G&W");
   else if (ch->pcdata->caste == 34)
      sprintf(caste_name, "&G&W    Staff     &G&W");
   else if (ch->pcdata->caste == 35)
      sprintf(caste_name, "&G&W    Admin     &G&W");
   else
      sprintf(caste_name, "&r  Casteless   &G&W");
      
  if (IS_SET(ch->elementb, ELEMENT_FIRE))
      sprintf(element_name, "&r Fire ");
   if (IS_SET(ch->elementb, ELEMENT_WATER))
      sprintf(element_name, "&C Water");
   if (IS_SET(ch->elementb, ELEMENT_AIR))
      sprintf(element_name, "&c  Air");
   if (IS_SET(ch->elementb, ELEMENT_EARTH))
      sprintf(element_name, "&O Earth");
   if (IS_SET(ch->elementb, ELEMENT_ENERGY))
      sprintf(element_name, "&c&wEnergy");
   if (IS_SET(ch->elementb, ELEMENT_DIVINE))
      sprintf(element_name, "&w&WDivine");

   set_pager_color(AT_SCORE, ch);
   //ch_printf(ch, "%s", MXPTAG("FRAME Name=\"Map\" FLOATING Left=\"-55c\" Top=\"0\" Width=\"55c\" Height=\"21c\""));  
   //ch_printf(ch, "%s", MXPTAG("FRAME Name=\"Map\" FLOATING Left=\"-20c\" Top=\"0\" Width=\"20c\" Height=\"20c\""));
   //ch_printf(ch, "%s %s %s %s %s", MXPTAG("DEST Map EOF"),  MXPTAG("/DEST"), MXPTAG("DEST Map"), MXPTAG("Image notepad.jpg align=bottom"), MXPTAG("/DEST"));

   sprintf(outputbuf, "\n\r               &P%s%s%s.\n\r", ch->name, ch->pcdata->title, get_wear_hidden_cloak(ch) ? " (Cloaked)" : "");

   sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
   strcat(outputbuf, buf);
   
   sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
   strcat(outputbuf, buf);
   
   sprintf(buf, "&g&w| &CStrength:     &r[&G&W%2.2d/%2.2d&r] &g&w| &CHealth: &r[&G&W%-5d/%5d&r] &g&w| &CCaste:  &r[&G&W%14s&r] &c&w|\n\r",
      get_curr_str(ch), ch->perm_str, ch->hit, ch->max_hit, caste_name);
   strcat(outputbuf, buf);
   
   switch (ch->style)
   {
      case STYLE_EVASIVE:
         sprintf(pbuf, "Evasive");
         break;
      case STYLE_DIVINE:
         sprintf(pbuf, "divine");
         break;
      case STYLE_WIZARDRY:
         sprintf(pbuf, "wizardry");
         break;
      case STYLE_DEFENSIVE:
         sprintf(pbuf, "Defensive");
         break;
      case STYLE_AGGRESSIVE:
         sprintf(pbuf, "Aggressive");
         break;
      case STYLE_BERSERK:
         sprintf(pbuf, "Berserk");
         break;
      default:
         sprintf(pbuf, "Standard");
         break;
   }
   sprintf(buf, "&g&w| &CIntelligence: &r[&G&W%2.2d/%2.2d&r] &g&w| &CMana:   &r[&G&W%-5d/%5d&r] &g&w| &CStyle:  &r[&G&W  %10s  &r] &c&w|\n\r",
      get_curr_int(ch), ch->perm_int, ch->mana, ch->max_mana, pbuf);
   strcat(outputbuf, buf);

   switch (ch->position)
   {
      case POS_DEAD:
         sprintf(pbuf, " Decomposing  ");
         break;
      case POS_MORTAL:
         sprintf(pbuf, "   Wounded    ");
         break;
      case POS_INCAP:
         sprintf(pbuf, " Knocked out ");
         break;
      case POS_STUNNED:
         sprintf(pbuf, "   Stunned    ");
         break;
      case POS_SLEEPING:
         sprintf(pbuf, "   Sleeping   ");
         break;
      case POS_RESTING:
         sprintf(pbuf, "   Resting    ");
         break;
      case POS_STANDING:
         sprintf(pbuf, "   Standing   ");
         break;
      case POS_FIGHTING:
         sprintf(pbuf, "   Fighting   ");
         break;
      case POS_EVASIVE:
         sprintf(pbuf, "   Evasive    "); /* Fighting style support -haus */
         break;
      case POS_DEFENSIVE:
         sprintf(pbuf, "  Defensive   ");
         break;
      case POS_AGGRESSIVE:
         sprintf(pbuf, "  Aggressive  ");
         break;
      case POS_BERSERK:
         sprintf(pbuf, "   Berserk    ");
         break;
      case POS_MOUNTED:
         sprintf(pbuf, "   Mounted    ");
         break;
      case POS_RIDING:
         sprintf(pbuf, "    Riding    ");
         break;
      case POS_SITTING:
         sprintf(pbuf, "   Sitting    ");
         break;
   }

   sprintf(buf, "&g&w| &CWisdom:       &r[&G&W%2.2d/%2.2d&r] &g&w| &CMoves:  &r[&G&W%-5d/%5d&r] &g&w| &CPos:    &r[&G&W%-14s&r] &c&w|\n\r",
      get_curr_wis(ch), ch->perm_wis, ch->move, ch->max_move, pbuf);
   strcat(outputbuf, buf);
      
   sprintf(buf, "&g&w| &CDexterity:    &r[&G&W%2.2d/%2.2d&r] &g&w| &CGold:   &r[&G&W %-10.10s&r] &g&w| &CQPS:    &r[&G&W %-10d   &r] &c&w|\n\r",
      get_curr_dex(ch), ch->perm_dex, punct(ch->gold), ch->pcdata->quest_curr);
   strcat(outputbuf, buf);

   sprintf(buf, "&g&w| &CConstitution: &r[&G&W%2.2d/%2.2d&r] &g&w| &CBank:   &r[&G&W %-10.10s&r] &g&w| &CAQPS:   &r[&G&W %-10d   &r] &c&w|\n\r",
      get_curr_con(ch), ch->perm_con, punct(ch->pcdata->balance), ch->pcdata->quest_accum);
   strcat(outputbuf, buf);

   sprintf(buf, "&g&w| &CLuck:         &r[&G&W%2.2d/%2.2d&r] &g&w| &CAgi:    &r[&G&W %3d       &r] &c&w| &CHand:   &r[&G&W    %s     &r] &c&w|\n\r",
      get_curr_lck(ch), ch->perm_lck, get_curr_agi(ch), ch->pcdata->righthanded == 0 ? " Left" : "Right");
   strcat(outputbuf, buf);
   
   sprintf(buf, "&g&w| &CEndurance:    &r[ &G&W%3.3d &r] &g&w| &CKgndom: &r[&G&W %-10.10s&r] &g&w| &CTown:   &r[&G&W %-13.13s&r] &c&w|\n\r",
      ch->mover, kingdom_table[ch->pcdata->hometown]->name, ch->pcdata->town ? ch->pcdata->town->name : "None");
   strcat(outputbuf, buf);
   if (sysdata.resetgame)
   {
      sprintf(buf, "&g&w| &CPower Ranking:&r[&G&W%-5d&r] &g&w| &CSPoint: &r[&G&W %-9d &r] &g&w| &CSWorth: &r[ &G&W%12.12s &r]&c&w |\n\r",
         ch->pcdata->power_ranking, ch->pcdata->spoints, punct(player_stat_worth(ch)));
      strcat(outputbuf, buf);
   }
   else
   {
      sprintf(buf, "&g&w|                       &g&w| &CSPoint: &r[&G&W %-9d &r] &g&w| &CSWorth: &r[ &G&W%12.12s &r]&c&w |\n\r",
         ch->pcdata->spoints, punct(player_stat_worth(ch)));
      strcat(outputbuf, buf);
   }
   if (sysdata.resetgame)
   {
      sprintf(buf, "&g&w| &CTwink Points: &r[ &G&W%-3d &r] &g&w| &CPLevel: &r[&G&W %-9d &r] &g&w| &CEWorth: &r[ &G&W%12.12s &r]&c&w |\n\r",
         ch->pcdata->twink_points, get_player_statlevel(ch), punct(player_equipment_worth(ch)));
      strcat(outputbuf, buf);
   }
   sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
   strcat(outputbuf, buf);
   sprintf(buf, "&g&w| &CElement:     &r[%s&r] &g&w| &CItems:  &r[&G&W%5d/%-5d&r] &c&w| &CWeight:&r[&G&W%8.2f/%-7d&r]&c&w|\n\r",
      element_name, get_ch_carry_number(ch), can_carry_n(ch), get_ch_carry_weight(ch), can_carry_w(ch));
   strcat(outputbuf, buf);

   sprintf(buf, "&g&w| &CWimpy:        &r[&G&W %4d&r] &g&w| &CRace:   &r[&G&W %-10s&r] &g&w| &CThirst: &r[&G&W     %3d      &r] &c&w|\n\r",
      ch->wimpy, capitalize(get_race(ch)), ch->pcdata->condition[COND_THIRST]);
   strcat(outputbuf, buf);
   sprintf(buf, "&g&w| &CAge:          &r[&G&W %4d&r] &g&w| &CGender: &r[&G&W %-7s   &r] &g&w| &CHunger: &r[&G&W     %3d      &r] &c&w|\n\r", get_age(ch),
      ch->sex == SEX_MALE ? "Male" : ch->sex == SEX_FEMALE ? "Female" : "Neither", ch->pcdata->condition[COND_FULL]);
   strcat(outputbuf, buf);
   sprintf(buf, "&g&w| &CHours:        &r[&G&W%5d&r] &g&w| &CReward: &r[&G&W%5d/%-5d&r] &c&w| &CDrunk:  &r[&G&W     %3d      &r] &c&w|\n\r",
      hplayed, ch->pcdata->reward_curr, ch->pcdata->reward_accum, ch->pcdata->condition[COND_DRUNK]);
   strcat(outputbuf, buf);
   sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
   strcat(outputbuf, buf);

   if (ch->con_rarm == -1)
      sprintf(caste_name, "&rxxxxx");
   else if (ch->con_rarm <= 200)
      sprintf(caste_name, "&G&W*&rxxxx");
   else if (ch->con_rarm <= 400)
      sprintf(caste_name, "&G&W**&rxxx");
   else if (ch->con_rarm <= 600)
      sprintf(caste_name, "&G&W***&rxx");
   else if (ch->con_rarm <= 800)
      sprintf(caste_name, "&G&W****&rx");
   else
      sprintf(caste_name, "&G&W*****");
      
   sprintf(buf, "&g&w| &CR-Arm %s ", caste_name);
   strcat(outputbuf, buf);
   
   if (ch->con_larm == -1)
      sprintf(caste_name, "&rxxxxx");
   else if (ch->con_larm <= 200)
      sprintf(caste_name, "&G&W*&rxxxx");
   else if (ch->con_larm <= 400)
      sprintf(caste_name, "&G&W**&rxxx");
   else if (ch->con_larm <= 600)
      sprintf(caste_name, "&G&W***&rxx");
   else if (ch->con_larm <= 800)
      sprintf(caste_name, "&G&W****&rx");
   else
      sprintf(caste_name, "&G&W*****");
      
   sprintf(buf, "&CL-Arm %s ", caste_name);
   strcat(outputbuf, buf);
   
   if (ch->con_rleg == -1)
      sprintf(caste_name, "&rxxxxx");
   else if (ch->con_rleg <= 200)
      sprintf(caste_name, "&G&W*&rxxxx");
   else if (ch->con_rleg <= 400)
      sprintf(caste_name, "&G&W**&rxxx");
   else if (ch->con_rleg <= 600)
      sprintf(caste_name, "&G&W***&rxx");
   else if (ch->con_rleg <= 800)
      sprintf(caste_name, "&G&W****&rx");
   else
      sprintf(caste_name, "&G&W*****");
      
   sprintf(buf, "&CR-Leg %s ", caste_name);
   strcat(outputbuf, buf);
   
   if (ch->con_lleg == -1)
      sprintf(caste_name, "&rxxxxx");
   else if (ch->con_lleg <= 200)
      sprintf(caste_name, "&G&W*&rxxxx");
   else if (ch->con_lleg <= 400)
      sprintf(caste_name, "&G&W**&rxxx");
   else if (ch->con_lleg <= 600)
      sprintf(caste_name, "&G&W***&rxx");
   else if (ch->con_lleg <= 800)
      sprintf(caste_name, "&G&W****&rx");
   else
      sprintf(caste_name, "&G&W*****");
      
   sprintf(buf, "&CL-Leg %s ", caste_name);
   strcat(outputbuf, buf);
   
   if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 20)
      sprintf(caste_name, "&G&W*****");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 40)
      sprintf(caste_name, "&G&W****&rx");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 60)
      sprintf(caste_name, "&G&W***&rxx");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 80)
      sprintf(caste_name, "&G&W**&rxxx");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 100)
      sprintf(caste_name, "&G&W*&rxxxx");
   else
      sprintf(caste_name, "&w&rxxxxx");
   
   sprintf(buf, "&CHBurn %s ", caste_name);
   strcat(outputbuf, buf);
   
   if (ch->pcdata->mana_cnt * 100 / 60 < 20)
      sprintf(caste_name, "&G&W*****");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 40)
      sprintf(caste_name, "&G&W****&rx");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 60)
      sprintf(caste_name, "&G&W***&rxx");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 80)
      sprintf(caste_name, "&G&W**&rxxx");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 100)
      sprintf(caste_name, "&G&W*&rxxxx");
   else
      sprintf(caste_name, "&w&rxxxxx");
   
   sprintf(buf, "&CMBurn %s  &g&w|\n\r", caste_name);
   strcat(outputbuf, buf);
   agimeter = get_bagility(ch);
   x = 50;
   
   strcpy(agimeter_name, "");
   for (;agimeter >= x && x < 2401;)
   {
      strcat(agimeter_name, "&w&P+");
      if (x % 200 == 0)
         strcat(agimeter_name, "&w&B|");
      x += 50;
   }
   for (;x < 2401;)
   {
      strcat(agimeter_name, "&c&w*");
      if (x % 200 == 0)
         strcat(agimeter_name, "&w&B|");
      x += 50;
   }
   sprintf(buf, "&g&w|  &CAgiMeter: %s  &g&w|\n\r", agimeter_name);
   strcat(outputbuf, buf);
   sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
   strcat(outputbuf, buf);


   if (ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD)
   {
      sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
      strcat(outputbuf, buf);
      sprintf(buf, "&g&w| &CCLAN:   &W%-14.14s&g&w| &CAvPkills:  &W%-5d      &g&w| &CNonAvpkills:  &W%-5d      &c&w|\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->pkills[5],
         (ch->pcdata->clan->pkills[0] + ch->pcdata->clan->pkills[1] + ch->pcdata->clan->pkills[2] + ch->pcdata->clan->pkills[3] +
            ch->pcdata->clan->pkills[4]));
      strcat(outputbuf, buf);
      sprintf(buf, "&g&w| &CAvPdeaths:   &W%-5d    &g&w| &CNonAvpdeaths: &W%-5d   &g&w|                          &c&w|\n\r",
         ch->pcdata->clan->pdeaths[5],
         (ch->pcdata->clan->pdeaths[0] + ch->pcdata->clan->pdeaths[1] + ch->pcdata->clan->pdeaths[2] + ch->pcdata->clan->pdeaths[3] +
            ch->pcdata->clan->pdeaths[4]));
      strcat(outputbuf, buf);
            
      sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
      strcat(outputbuf, buf);
   }

   if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER)
   {
      sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
      strcat(outputbuf, buf);
      sprintf(buf, "&g&w| &COrder: &W%-15s&g&w| &CMkills:  &W%-6d       &g&w| &CMDeaths:     &W%-6d      &c&w|\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
      strcat(outputbuf, buf);
      sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
      strcat(outputbuf, buf);

   }
   if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD)
   {
      sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
      strcat(outputbuf, buf);
      sprintf(buf, "&g&w| &CGuild: &W%-15s&g&w| &CMkills:    &W%-6d      &g&w| &CMDeaths:     &W%-6d     &c&w|\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
      strcat(outputbuf, buf);
      sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
      strcat(outputbuf, buf);
   }

   if (ch->position != POS_SLEEPING)
      switch (ch->mental_state / 10)
      {
         default:
            sprintf(buf, "&g&w| &GCondition:      &OYou're completely messed up!\n\r");
            break;
         case -10:
            sprintf(buf, "&g&w| &GCondition:      &OYou're barely conscious.\n\r");
            break;
         case -9:
            sprintf(buf, "&g&w| &GCondition:      &OYou can barely keep your eyes open.\n\r");
            break;
         case -8:
            sprintf(buf, "&g&w| &GCondition:      &OYou're extremely drowsy.\n\r");
            break;
         case -7:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel very unmotivated.\n\r");
            break;
         case -6:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel sedated.\n\r");
            break;
         case -5:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel sleepy.\n\r");
            break;
         case -4:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel tired.\n\r");
            break;
         case -3:
            sprintf(buf, "&g&w| &GCondition:      &OYou could use a rest.\n\r");
            break;
         case -2:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel a little under the weather.\n\r");
            break;
         case -1:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel fine.\n\r");
            break;
         case 0:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel great.\n\r");
            break;
         case 1:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel energetic.\n\r");
            break;
         case 2:
            sprintf(buf, "&g&w| &GCondition:      &OYour mind is racing.\n\r");
            break;
         case 3:
            sprintf(buf, "&g&w| &GCondition:      &OYou can't think straight.\n\r");
            break;
         case 4:
            sprintf(buf, "&g&w| &GCondition:      &OYour mind is going 100 miles an hour.\n\r");
            break;
         case 5:
            sprintf(buf, "&g&w| &GCondition:      &OYou're high as a kite.\n\r");
            break;
         case 6:
            sprintf(buf, "&g&w| &GCondition:      &OYour mind and body are slipping apart.\n\r");
            break;
         case 7:
            sprintf(buf, "&g&w| &GCondition:      &OReality is slipping away.\n\r");
            break;
         case 8:
            sprintf(buf, "&g&w| &GCondition:      &OYou have no idea what is real, and what is not.\n\r");
            break;
         case 9:
            sprintf(buf, "&g&w| &GCondition:      &OYou feel immortal.\n\r");
            break;
         case 10:
            sprintf(buf, "&g&w| &GCondition:      &OYou are a Supreme Entity.\n\r");
            break;
      }
   else if (ch->mental_state > 45)
      sprintf(buf, "&g&w| &GCondition:      &OYour sleep is filled with strange and vivid dreams.\n\r");
   else if (ch->mental_state > 25)
      sprintf(buf, "&g&w| &GCondition:      &OYour sleep is uneasy.\n\r");
   else if (ch->mental_state < -35)
      sprintf(buf, "&g&w| &GCondition:      &OYou are deep in a much needed sleep.\n\r");
   else if (ch->mental_state < -25)
      sprintf(buf, "&g&w| &GCondition:      &OYou are in deep slumber.\n\r");   
   
   strcat(outputbuf, buf);
   
   if (ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0')
      sprintf(buf, "&g&w| &CBestowment(s):  &Y%s\n\r", ch->pcdata->bestowments);
   strcat(outputbuf, buf);

   sprintf(buf, "&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
   strcat(outputbuf, buf);

   if (ch->first_affect)
   {
      int i;
      SKILLTYPE *sktmp;
      char timeb[MSL];

      i = 0;
      set_char_color(AT_WHITE, ch);
      send_to_pager("AFFECT DATA:                           ", ch);
      for (paf = ch->first_affect; paf; paf = paf->next)
      {
         if ((sktmp = get_skilltype(paf->type)) == NULL)
            continue;
         if (ch->level < LEVEL_IMMORTAL && (!IS_AFFECTED(ch, AFF_DETECT_MAGIC)))
         {
            sprintf(buf, "[%-34.34s]    ", sktmp->name);
            strcat(outputbuf, buf);
            if (i == 0)
               i = 2;
            if ((++i % 3) == 0)
            {
               sprintf(buf, "\n\r");
               strcat(outputbuf, buf);
            }
         }
         if (ch->level >= LEVEL_IMMORTAL || (IS_AFFECTED(ch, AFF_DETECT_MAGIC)))
         {
            sprintf(timeb, "M:%d S:%d", paf->duration/60, paf->duration%60);
            strcat(outputbuf, buf);
            if (paf->modifier == 0)
            {
               sprintf(buf, "[%-24.24s&c&R|&G&W%-10.10s]  ", sktmp->name, timeb);
               strcat(outputbuf, buf);
            }
            else if (paf->modifier > 999)
            {
               sprintf(buf, "[%-15.15s&c&R|&G&W %7.7s&c&R|&G&W%-10.10s]  ", sktmp->name, tiny_affect_loc_name(paf->location), timeb);               
               strcat(outputbuf, buf);
            }
            else
            {
               sprintf(buf, "[%-11.11s&c&R|&G&W%+-3.3d %7.7s&c&R|&G&W%-10.10s]  ",
                  sktmp->name, paf->modifier, tiny_affect_loc_name(paf->location), timeb);
               strcat(outputbuf, buf);
            }
            if (i == 0)
               i = 1;
            if ((++i % 2) == 0)
            {
               sprintf(buf, "\n\r");
               strcat(outputbuf, buf);               
            }
         }
      }
   }
   sprintf(buf, "\n\r&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r");
   strcat(outputbuf, buf);
   
   x=y=0;
   for (;;)
   {
      if (outputbuf[x] == '\0')
      {
         parsebuf[y] = '\0';
         break;
      }
      if (outputbuf[x] == '^')
      {
         if (outputbuf[x+1] == '^')
         {
            parsebuf[y] = '^';
            y++;
            x+=2;
            continue;
         }
         else
         {
            x+=2;
            continue;
         }
      }
      if (outputbuf[x] == '&')
      {
         if (outputbuf[x+1] == '&')
         {
            parsebuf[y] = '&';
            x+=2;
            y++;
            continue;
         }
         else
         {
            x+=2;
            continue;
         }
      }
      if (outputbuf[x] == '\r')
      {
         x+=1;
         continue;
      }
      parsebuf[y] = outputbuf[x];
      x++;
      y++;
   }
   fprintf(fp, "%s", parsebuf);
   return;
}

char *const web_where_name[] = {
   "used as light      ",
   "worn on finger     ",
   "worn on finger     ",
   "worn on neck       ",
   "worn around neck   ",
   "worn on body       ",
   "worn on head       ",
   "worn on left leg   ",
   "worn on right leg  ",
   "worn on left arm   ",
   "worn on right arm  ",
   "worn as shield     ",
   "worn about waist   ",
   "wielded            ",
   "dual wielded       ",
   "missile wielded    ",
   "lodged in ribs     ",
   "lodged in arm      ",
   "lodged in leg      ",
   "nocked             ",
   "worn on back       "
};

//Parses Color and New lines into web format.
char *parse_color_web(char *argument)
{
   static char rbuf[MSL];
   
   int x, y;
   
   x=y=0;
   
   for (;;)
   {
      if (argument[x] == '\0')
      {
         rbuf[y] = '\0';
         break;
      }
      if (argument[x] == '\n' && argument[x+1] == '\r')
      {
         rbuf[y] = '\0';
         strcat(rbuf, "<br>");
         x+=2;
         y+=4;
         continue;
      }
      if (argument[x] == '\n')
      {
         rbuf[y] = '\0';
         strcat(rbuf, "<br>");
         x+=1;
         y+=4;
         continue;
      }
      if (argument[x] == '^')
      {
         if (argument[x+1] == '^')
         {
            rbuf[y] = '^';
            y++;
            x+=2;
            continue;
         }
         else
         {
            x+=2;
            continue;
         }
      }
      if (argument[x] == '&')
      {
         if (argument[x+1] == 'r' || argument[x+1] == 'R')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=red>");
            x+=2;
            y+=16;
            continue;
         }
         if (argument[x+1] == 'B' || argument[x+1] == 'b')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=blue>");
            x+=2;
            y+=17;
            continue;
         }
         if (argument[x+1] == 'c' || argument[x+1] == 'C')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=cyan>");
            x+=2;
            y+=17;
            continue;
         }
         if (argument[x+1] == 'g' || argument[x+1] == 'G')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=green>");
            x+=2;
            y+=18;
            continue;
         }
         if (argument[x+1] == 'Y')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=yellow>");
            x+=2;
            y+=19;
            continue;
         }
         if (argument[x+1] == 'P')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=pink>");
            x+=2;
            y+=17;
            continue;
         }
         if (argument[x+1] == 'p')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=purple>");
            x+=2;
            y+=19;
            continue;
         }
         if (argument[x+1] == 'W')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=white>");
            x+=2;
            y+=18;
            continue;
         }
         if (argument[x+1] == 'w' || argument[x+1] == 'z' || argument[x+1] == 'x')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=grey>");
            x+=2;
            y+=17;
            continue;
         }
         if (argument[x+1] == 'o')
         {
            rbuf[y] = '\0';
            strcat(rbuf, "<font color=orange");
            x+=2;
            y+=19;
            continue;
         }
         if (argument[x+1] == '&')
         {
            rbuf[y] = '&';
            y++;
            x+=2;
            continue;
         }
      }
      rbuf[y] = argument[x];
      y++;
      x++;
   }
   return &(rbuf[0]);
}

int local_actcount;

void do_weboutput_identify(CHAR_DATA *ch, char *rand, OBJ_DATA *obj, char *psbuf)
{
   FILE *fp;
   char filename[MSL];
   char pbuf[MSL];
   OBJ_DATA *nobj;
   
   sprintf(filename, "%s%s%s%d.htm", PLAYERSTAT_DIR, ch->name, rand, local_actcount++);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      perror(filename);
      send_to_char("Failed to write the file, tell an immortal.\n\r", ch);
      return;
   }
   sprintf(filename, "%s%s%d.htm", ch->name, rand, local_actcount-1);
   fprintf(fp, "<html><head><title>ID - %s</title></head><body bgcolor=black text=white>", obj->short_descr);
   fprintf(fp, "<a href=\"%s\"><---Go Back</a><p>\n\r", psbuf);
   fprintf(fp, "IDENTIFY - %s<br>_____________<br>", parse_color_web(obj->short_descr));
   code_identify(ch, obj, NULL, 1, pbuf);
   fprintf(fp, "%s<p><font color=white>Contents<br>_____________<br>", parse_color_web(pbuf));
   
   for (nobj = obj->first_content; nobj; nobj = nobj->next_content)
   {
      fprintf(fp, "<a href=\"%s%s%d.htm\">%s</a><br>", ch->name, rand, local_actcount, parse_color_web(format_obj_to_char(nobj, ch, TRUE, FALSE)));
      do_weboutput_identify(ch, rand, nobj, filename);
   }
   fclose(fp);
}

void do_weboutput_equip(CHAR_DATA *ch, FILE *fp, char *rand)
{
   OBJ_DATA *obj;
   int iWear;
   char pbuf[MSL];
   bool found;
   
   sprintf(pbuf, "%s%s.htm", ch->name, rand);

   found = FALSE;
   fprintf(fp, "<table size=100% border=0><tr><td width=30%><p></p></td><td width=70%><p></p></td></tr>");
   for (iWear = 0; iWear < MAX_WEAR; iWear++)
   {
      for (obj = ch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->wear_loc == iWear)
         {
            fprintf(fp, "<tr><td>%s</td>", web_where_name[iWear]);

            if (can_see_obj(ch, obj))
            {
               fprintf(fp, "<td><a href=\"%s%s%d.htm\">%s</a></td><tr>", ch->name, rand, local_actcount, parse_color_web(format_obj_to_char(obj, ch, TRUE, FALSE)));
               do_weboutput_identify(ch, rand, obj, pbuf);
            }
            else
               fprintf(fp, "<td>Something</td><tr>");
            found = TRUE;
         } /* Checks to see if char is using the slot -- Xerves */
      }
      if ((obj = get_eq_char(ch, iWear)) == NULL)
      {
         if (iWear >= WEAR_LODGE_RIB && iWear <= WEAR_NOCKED)
            continue;
         fprintf(fp, "<tr><td>%s</td>", web_where_name[iWear]);
         fprintf(fp, "<td>Nothing<td></td>");
         found = TRUE;
      }
   }
   fprintf(fp, "</table>");
   return;
}

void do_weboutput_inventory(CHAR_DATA *ch, FILE *fp, char *rand)
{
   char **prgpstrShow;
   char **prgpstrName;        /* for MXP */
   char **prgpstrShortName;   /* for MXP */
   OBJ_DATA **objptrlist;
   int *prgnShow;
   int *pitShow;
   char *pstrShow;
   char *pstrName;            /* for MXP */
   char *pstrShortName;       /* for MXP */
   OBJ_DATA *obj;
   int nShow;
   int iShow;
   int count, offcount, tmp, ms, cnt;
   bool fCombine;
   char buf[MSL];
   char sbuf[MSL];
   OBJ_DATA *list = ch->first_carrying;   
   char pbuf[MSL];
   
   sprintf(pbuf, "%s%s.htm", ch->name, rand);

   if (!ch->desc)
      return;


   /*
    * if there's no list... then don't do all this crap!  -Thoric
    */
   if (!list)
   {
      return;
   }

   /*
    * Alloc space for output lines.
    */
   count = 0;
   for (obj = list; obj; obj = obj->next_content)
   {
      if (IS_ONMAP_FLAG(ch) || IS_OBJ_STAT(obj, ITEM_ONMAP))
      {
         if (obj->map != ch->map || obj->coord->x != ch->coord->x || obj->coord->y != ch->coord->y)
            continue;
      }
      count++;
   }

   ms = (ch->mental_state ? ch->mental_state : 1)
      * (IS_NPC(ch) ? 1 : (ch->pcdata->condition[COND_DRUNK] ? (ch->pcdata->condition[COND_DRUNK] / 12) : 1));

   /*
    * If not mentally stable...
    */
   if (abs(ms) > 40)
   {
      offcount = URANGE(-(count), (count * ms) / 100, count * 2);
      if (offcount < 0)
         offcount += number_range(0, abs(offcount));
      else if (offcount > 0)
         offcount -= number_range(0, offcount);
   }
   else
      offcount = 0;

   if (count + offcount <= 0)
   {
      return;
   }

   CREATE(prgpstrShow, char *, count + ((offcount > 0) ? offcount : 0));
   CREATE( prgpstrName,	char*,	count + ((offcount > 0) ? offcount : 0) );
   CREATE( prgpstrShortName,	char*,	count + ((offcount > 0) ? offcount : 0) );
   CREATE(objptrlist, OBJ_DATA*, count + ((offcount > 0) ? offcount : 0));
   CREATE(prgnShow, int, count + ((offcount > 0) ? offcount : 0));
   CREATE(pitShow, int, count + ((offcount > 0) ? offcount : 0));

   nShow = 0;
   tmp = (offcount > 0) ? offcount : 0;
   cnt = 0;

   /*
    * Format the list of objects.
    */
   for (obj = list; obj; obj = obj->next_content)
   {
      if (offcount < 0 && ++cnt > (count + offcount))
         break;

      if (IS_ONMAP_FLAG(ch) || IS_OBJ_STAT(obj, ITEM_ONMAP))
      {
         if (obj->map != ch->map || obj->coord->x != ch->coord->x || obj->coord->y != ch->coord->y)
            continue;
      }
      if (tmp > 0 && number_bits(1) == 0)
      {
         prgpstrShow[nShow] = str_dup(hallucinated_object(ms, TRUE));
         prgpstrName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgpstrShortName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range(ITEM_LIGHT, ITEM_BOOK);
         objptrlist[nShow] = obj;
         nShow++;
         --tmp;
      }
      if (obj->wear_loc == WEAR_NONE && can_see_obj(ch, obj) && (obj->item_type != ITEM_TRAP || IS_AFFECTED(ch, AFF_DETECTTRAPS)))
      {
         pstrShow = format_obj_to_char(obj, ch, TRUE, TRUE);
         pstrName = obj->name;
         pstrShortName = obj->short_descr;
         fCombine = FALSE;

         if (IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE))
         {
            /*
             * Look for duplicates, case sensitive.
             * Matches tend to be near end so run loop backwords.
             */
            for (iShow = nShow - 1; iShow >= 0; iShow--)
            {
               if (!strcmp(prgpstrShow[iShow], pstrShow))
               {
                  prgnShow[iShow] += obj->count;
                  fCombine = TRUE;
                  break;
               }
            }
         }

         pitShow[nShow] = obj->item_type;
         /*
          * Couldn't combine, or didn't want to.
          */
         if (!fCombine)
         {
            prgpstrShow[nShow] = str_dup(pstrShow);
            objptrlist[nShow] = obj;
            prgpstrName [nShow] = str_dup( pstrName );
      	    prgpstrShortName [nShow] = str_dup( pstrShortName );
            prgnShow[nShow] = obj->count;
            nShow++;
         }
         if (IS_ONMAP_FLAG(ch) || IS_OBJ_STAT(obj, ITEM_ONMAP))
         {
            if (obj->map != ch->map || obj->coord->x != ch->coord->x || obj->coord->y != ch->coord->y)
               continue;
         }
      }
   }
   if (tmp > 0)
   {
      int x;

      for (x = 0; x < tmp; x++)
      {
         prgpstrShow[nShow] = str_dup(hallucinated_object(ms, TRUE));
         prgpstrName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgpstrShortName [nShow] = str_dup( hallucinated_object(ms, TRUE) );
         prgnShow[nShow] = 1;
         pitShow[nShow] = number_range(ITEM_LIGHT, ITEM_BOOK);
         nShow++;
      }
   }

   /*
    * Output the formatted list.  -Color support by Thoric
    */
   for (iShow = 0; iShow < nShow; iShow++)
   {
      strcpy(sbuf, "");
      switch (pitShow[iShow])
      {
         default:
            sprintf(buf, "&w&G");
            strcat(sbuf, buf);
            break;
         case ITEM_BLOOD:
            sprintf(buf, "&w&r");
            strcat(sbuf, buf);
            break;
         case ITEM_MONEY:
         case ITEM_TREASURE:
            sprintf(buf, "&w&Y");
            strcat(sbuf, buf);
            break;
         case ITEM_COOK:
         case ITEM_FOOD:
            sprintf(buf, "&w&O");
            strcat(sbuf, buf);
            break;
         case ITEM_DRINK_CON:
         case ITEM_FOUNTAIN:
            sprintf(buf, "&w&B");
            strcat(sbuf, buf);
            break;
         case ITEM_FIRE:
            sprintf(buf, "&w&R");
            strcat(sbuf, buf);
            break;
         case ITEM_SCROLL:
         case ITEM_WAND:
         case ITEM_STAFF:
         case ITEM_SPELLBOOK:
            sprintf(buf, "&w&B");
            strcat(sbuf, buf);
            break;
      }
      sprintf(buf, prgpstrShow[iShow]);
      strcat(sbuf, buf);
      {
         if (prgnShow[iShow] != 1)
         {
            sprintf(buf, " (%d)", prgnShow[iShow]);
            strcat(sbuf, buf);
         }
      }
      fprintf(fp, "<a href=\"%s%s%d.htm\">%s</a><br>", ch->name, rand, local_actcount, parse_color_web(sbuf));
      do_weboutput_identify(ch, rand, objptrlist[iShow], pbuf);
      
      DISPOSE(prgpstrShow[iShow]);
      DISPOSE( prgpstrName[iShow] );
      DISPOSE( prgpstrShortName[iShow] );
   }

   /*
    * Clean up.
    */
   DISPOSE(prgpstrShow);
   DISPOSE( prgpstrName );
   DISPOSE( prgpstrShortName );
   DISPOSE(prgnShow);
   DISPOSE(pitShow);
   return;
}

void do_webstats(CHAR_DATA * ch, char *argument)
{  
   int x = 0;
   static char rand[10];
   char filename[MSL];
   FILE *fp;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: webstats generate\n\r", ch);
      return;
   }
   
   if (!str_cmp("generate", argument))
   {
      for (x = 0; x <= 4; x++)
      {
         rand[x] = number_range(48, 57);
      }
      rand[5] = '\0';
      local_actcount = 0;
      sprintf(filename, "%s%s%s.htm", PLAYERSTAT_DIR, ch->name, rand);
      if ((fp = fopen(filename, "w")) == NULL)
      {
         perror(filename);
         send_to_char("Failed to write the file, tell an immortal.\n\r", ch);
         return;
      }
      fprintf(fp, "<html>\n<head>\n<title>%s - Player Profile</title>\n</head>\n<body bgcolor=black text=white>\n<pre>\n", ch->name);
      do_weboutput_score(ch, fp);
      fprintf(fp, "</pre><p>");
      do_weboutput_equip(ch, fp, rand);
      fprintf(fp, "<p>INVENTORY<br>________________<p>");
      do_weboutput_inventory(ch, fp, rand);
      fclose(fp);
      send_to_char("Done.\n\r", ch);
      return;
   }
   do_webstats(ch, "");
   return;
}
         
   
void do_exits(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   EXIT_DATA *pexit;
   bool found;
   bool fAuto;
   int  spaces;

   set_char_color(AT_EXITS, ch);
   buf[0] = '\0';
   fAuto = !str_cmp(argument, "auto");

   if (!check_blind(ch))
      return;

   if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
   {
      send_to_char("You cannot use this command while on the map.", ch);
      return;
   }
   if (xIS_SET(ch->in_room->room_flags, ROOM_NOEXIT))
      return;

   strcpy(buf, fAuto ? "Exits:" : "Obvious exits:\n\r");

   found = FALSE;
   for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
   {
      if (pexit->to_room
         && (!IS_SET(pexit->exit_info, EX_WINDOW) || IS_SET(pexit->exit_info, EX_ISDOOR)) && !IS_SET(pexit->exit_info, EX_HIDDEN))
      {
         found = TRUE;
         if (fAuto)
         {
            strcat( buf, " " );
            if (IS_SET(pexit->exit_info, EX_LOCKED))
               strcat(buf, "<");
            else if (IS_SET(pexit->exit_info, EX_CLOSED))
               strcat(buf, "[");
            strcat (buf, MXPTAG ("Ex"));
  	        strcat( buf, dir_name[pexit->vdir] );
            strcat (buf, MXPTAG ("/Ex"));
            if (IS_SET(pexit->exit_info, EX_LOCKED))
               strcat(buf, ">");
            else if (IS_SET(pexit->exit_info, EX_CLOSED))
               strcat(buf, "]");
         }
  	 else
  	 {
            /* I don't want to underline spaces, so I'll calculate the number we need */
             spaces = 5 - strlen (dir_name[pexit->vdir]);
             if (spaces < 0)
                spaces = 0;
   	     sprintf( buf + strlen(buf), "%s%s%s%s%s%*s - %s\n\r",IS_SET(pexit->exit_info, EX_CLOSED) ? "[" : "", MXPTAG("Ex"),
   	        capitalize( dir_name[pexit->vdir] ),MXPTAG("/Ex"),IS_SET(pexit->exit_info, EX_CLOSED) ? "]" : "", spaces,"",      
  		    room_is_dark( pexit->to_room ) && !IS_AFFECTED(ch, AFF_INFRARED) ? "Too dark to tell" : pexit->to_room->name);
         }
      }
   }

   if (!found)
      strcat(buf, fAuto ? " none.\n\r" : "None.\n\r");
   else if (fAuto)
      strcat(buf, ".\n\r");
   send_to_char(buf, ch);
   return;
}

char *const day_name[] = {
   "the Moon", "the Bull", "Deception", "Thunder", "Freedom",
   "the Great Gods", "the Sun"
};

char *const month_name[] = {
   "Winter", "the Winter Wolf", "the Frost Giant", "the Old Forces",
   "the Grand Struggle", "the Spring", "Nature", "Futility", "the Dragon",
   "the Sun", "the Heat", "the Battle", "the Dark Shades", "the Shadows",
   "the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time(CHAR_DATA * ch, char *argument)
{
   extern char str_boot_time[];
   extern char reboot_time[];
   char buf[MSL];
   struct tm *tptr;
   char *dtime;
   
   if (copyover_time)
   {
      tptr = localtime(&copyover_time);
      dtime = asctime(tptr);
      sprintf(buf, "Scheduled Copyover at:    %s\n\r", dtime);
   }
   else
   {
      sprintf(buf, "No scheduled copyover present\n\r");
   }

   set_char_color(AT_YELLOW, ch);
   ch_printf(ch,
      "%s\n\r"
      "The mud started up at:    %s\r"
      "The system time (E.S.T.): %s\r" "Next Reboot is set for:   %s\r%s", 
         getgametime(), str_boot_time, (char *) ctime(&current_time), reboot_time, buf);

   return;
}

/*
 * Produce a description of the weather based on area weather using
 * the following sentence format:
 *		<combo-phrase> and <single-phrase>.
 * Where the combo-phrase describes either the precipitation and
 * temperature or the wind and temperature. The single-phrase
 * describes either the wind or precipitation depending upon the
 * combo-phrase.
 * Last Modified: July 31, 1997
 * Fireblade - Under Construction
 */
void do_weather(CHAR_DATA * ch, char *argument)
{
   if (!IS_OUTSIDE(ch) || NO_WEATHER_SECT(ch->in_room->sector_type))
   {
      send_to_char("It is hard to see the sky inside.\n\r", ch);
      return;
   }
   if (!IN_WILDERNESS(ch))
   {
      if (ch->in_room->area->x < 1 || ch->in_room->area->y < 1 || ch->in_room->map < 0)
      {
         send_to_char("This area needs weather adjustments, please tell an imm.\n\r", ch);
         return;
      }
      generate_wind_dir(ch, ch->in_room->area->x, ch->in_room->area->y, ch->in_room->area->map);
      show_temp(ch, ch->in_room->area->x, ch->in_room->area->y, ch->in_room->area->map);
      return;
   }
   generate_wind_dir(ch, -1, -1, -1);
   show_temp(ch, -1, -1, -1);
   return;
}

void do_forecast(CHAR_DATA * ch, char *argument)
{
   if (!IS_OUTSIDE(ch) || NO_WEATHER_SECT(ch->in_room->sector_type))
   {
      send_to_char("It is hard to see the sky inside.\n\r", ch);
      return;
   }
   if (!IN_WILDERNESS(ch))
   {
      if (ch->in_room->area->x < 1 || ch->in_room->area->y < 1 || ch->in_room->map < 0)
      {
         send_to_char("This area needs weather adjustments, please tell an imm.\n\r", ch);
         return;
      }
      generate_forecast(ch, ch->in_room->area->x, ch->in_room->area->y, ch->in_room->area->map);
      return;
   }
   generate_forecast(ch, -1, -1, -1);
   return;
}

/*
 * LAWS command
 */
void do_laws(CHAR_DATA * ch, char *argument)
{
   char buf[1024];

   if (argument == NULL)
      do_help(ch, "laws");
   else
   {
      sprintf(buf, "law %s", argument);
      do_help(ch, buf);
   }
}

//  Ranks by number of matches between two whole words. Coded for the Similar Helpfiles
//  Snippet by Senir.
sh_int str_similarity( const char *astr, const char *bstr )
{
   sh_int matches=0;

    if (!astr || !bstr)
       return matches;

    for ( ; *astr; astr++)
    {
        if ( LOWER(*astr) == LOWER(*bstr) )
           matches++;
        
        if (++bstr == '\0')
           return matches;                
    }
    
    return matches;
}

//  Ranks by number of matches until there's a nonmatching character between two words.
//  Coded for the Similar Helpfiles Snippet by Senir.
sh_int str_prefix_level( const char *astr, const char *bstr )
{
   sh_int matches=0;

    if (!astr || !bstr)
       return matches;

    for ( ; *astr; astr++)
    {
        if ( LOWER(*astr) == LOWER(*bstr) )
           matches++;
        else
           return matches;

        if (++bstr == '\0')
        return matches;
    }

    return matches;
}

// Main function of Similar Helpfiles Snippet by Senir. It loops through all of the
// helpfiles, using the string matching function defined to find the closest matching
// helpfiles to the argument. It then checks for singles. Then, if matching helpfiles
// are found at all, it loops through and prints out the closest matching helpfiles.
// If its a single(there's only one), it opens the helpfile.
void similar_help_files(CHAR_DATA *ch, char *argument)
{
   HELP_DATA *pHelp=NULL;
   char buf[MSL];
   char *extension;
   sh_int lvl=0;
   bool single=FALSE;
    
        
    send_to_pager_color( "&C&BSimilar Help Files:\n\r", ch);
        
    for ( pHelp = first_help; pHelp; pHelp=pHelp->next)
    {
        buf[0]='\0';      
        extension=pHelp->keyword;
        
        if (pHelp->level > get_trust(ch))
           continue;

        while ( extension[0] != '\0' )
        {
              extension= one_argument(extension, buf);
              
              if ( str_similarity(argument, buf) > lvl)
              {
                 lvl=str_similarity(argument, buf);
                 single=TRUE;
              }        
              else if ( str_similarity(argument, buf) == lvl && lvl > 0)
              {
                 single=FALSE;
              }
        }
    }
        
    if (lvl==0)
    {
       send_to_pager_color( "&C&GNo similar help files.\n\r", ch);   
       return;
    }

    for ( pHelp = first_help; pHelp; pHelp=pHelp->next)
    {
        buf[0]='\0';      
        extension=pHelp->keyword;

        while ( extension[0] != '\0' )
        {
              extension=one_argument(extension, buf);

              if ( str_similarity(argument, buf) >= lvl
                   && pHelp->level <= get_trust(ch))
              {
                 if (single)
                 {
                    send_to_pager_color( "&C&GOpening only similar helpfile.&C\n\r", ch);
                    do_help( ch, buf);
                    return;
                 }

                 pager_printf_color(ch, "&C&G   %s\n\r", pHelp->keyword);
                 break;

              }

        }
    }
    return;
}

//maxlength should not be over 50
char *add_wspace(int length, int maxlength)
{
   int x = maxlength - length;
   static char wbuf[55];
   strcpy(wbuf, "");
   
   if (length >= maxlength)
   {
      return "";
   }
   else
   {
      for(;;)
      {     
         strcat(wbuf, " ");     
         if (--x == 0)
            break;
      }
   }
   return wbuf;
}

void do_helpweb(CHAR_DATA *ch, char *argument)
{
    HELP_DATA *pHelp;
    char buf[MSL];
    char parse[MSL*2];
    FILE *fp;
    char filename[MIL];
    int x;
    int y;
   
    sprintf(filename, "%s", WEBHELP_FILE);
    if ((fp = fopen(filename, "w")) == NULL)
    {
       send_to_char("Cannot write to the webhelp file.\n\r", ch);
       return;
    }
    
    for (pHelp = first_help; pHelp; pHelp = pHelp->next)
    {
      if (pHelp->level < LEVEL_IMMORTAL)
      {
         if (sizeof(pHelp->text) > MSL)
         {
            ch_printf(ch, "Helpfile %s's text is too long to read in, aborting.\n\r", pHelp->keyword);
            fclose(fp);
            return;
         }
         sprintf(buf, "-------------------------------------------------------\n%s\n-------------------------------------------------------\n", pHelp->keyword);
         for (x = 0,y = 0;;)
         {
            if (pHelp->text[x] == '&')
            {
               if (pHelp->text[x+1] == '&')
               {
                  parse[y++] = '&';
               }
               x+=2;
            }
            else if (pHelp->text[x] == '\r')
            {
               x++;
            }
            else if (pHelp->text[x] == '\0')
            {
               parse[y] = '\0';
               break;
            }
            else
            {
               parse[y++] = pHelp->text[x++];
            }
         }
         for (x = 0;;x++)
         {
            if (buf[x] != '\0')
               putc(buf[x], fp);
            else
               break;
         }
         for (x = 0;;x++)
         {
            if (parse[x] != '\0')
               putc(parse[x], fp);
            else
               break;
         }
         fprintf(fp, "\n\n");
      }
   }
   send_to_char("Done.\n\r", ch);
   fclose(fp);
   return;
}

/*
 * Now this is cleaner
 */
void do_help(CHAR_DATA * ch, char *argument)
{
   HELP_DATA *pHelp;
   char nohelp[MSL];
   HINDEX_DATA *hindex;
   HINDEX_POINTER *hpointer;
   char arg[MIL];
   int cnt;
   int nshow = 0;

   one_argument(argument, arg);
   
   if (!str_cmp(arg, "_clean_"))
   {
      nshow = 1;
      argument = one_argument(argument, arg);
   }   
   strcpy(nohelp, argument); /* For Finding "needed" helpfiles */

   if ((pHelp = get_help(ch, argument)) == NULL)
   {
      //  Looks better printing out the missed argument before going to similar
      //  helpfiles. - Senir
      pager_printf_color( ch, "&C&wNo help on \'%s\' found.\n\r", argument );
      similar_help_files(ch, argument);
      append_file(ch, HELP_FILE, nohelp);
      return;
   }
   /* Make newbies do a help start. --Shaddai */
   if (!IS_NPC(ch) && !str_cmp(argument, "start"))
      SET_BIT(ch->pcdata->flags, PCFLAG_HELPSTART);

   if (pHelp->level >= 0 && str_cmp(argument, "imotd"))
   {
      send_to_pager(pHelp->keyword, ch);
      send_to_pager("\n\r", ch);
   }
   /*
    * Strip leading '.' to allow initial blanks.
    */
   if (pHelp->text[0] == '.')
      send_to_pager_color(pHelp->text + 1, ch);
   else
      send_to_pager_color(pHelp->text, ch);
      
   //HelpIndex Code Below
   /*if (!xIS_SET(ch->act, PLR_NOSIMILIAR) && nshow == 0)
   {
      for (hpointer = pHelp->first_hindex; hpointer; hpointer = hpointer->next)
      {
         cnt = 0;
         hindex = hpointer->pointer;
         #ifdef HINDEX_MXP  
            ch_printf(ch, "\n\r&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CSimiliar Helpfiles in " MXPTAG ("hindex '%s'") "%s" MXPTAG ("/hindex") " Index&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r\n\r", hindex->keyword, hindex->keyword);
         #else
            ch_printf(ch, "\n\r&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CSimiliar Helpfiles in %s Index&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r\n\r", hindex->keyword);    
         #endif
         for (hipointer = hindex->first_help; hipointer; hipointer = hipointer->next)
         {
            ihelp = hipointer->pointer;
            helpstring = ihelp->keyword;
            for (;;)
            {
               if (helpstring[0] == '\0')
                  break;
               else
               {
                  helpstring = one_argument(helpstring, helpfile);
                  if (cnt + strlen(helpfile) > 76)
                  {
                     send_to_char("\n\r", ch);
                     cnt = 0;
                  }
                  #ifdef HINDEX_MXP  
                     ch_printf(ch, MXPTAG ("help '%s'") "&w&R%s"  MXPTAG ("/help") "    ", helpfile, helpfile);
                  #else
                     ch_printf(ch, "&w&R%s    ", helpfile);
                  #endif
                  cnt+= strlen(helpfile) + 4;
               }
            }
         }  
         send_to_char("\n\r", ch);   
      }
   } */
  // else if (xIS_SET(ch->act, PLR_NOSIMILIAR) && nshow == 0)
   {
      cnt = 0;
      ch_printf(ch, "\n\r&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CHelpIndex(es) for %s&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r\n\r", pHelp->keyword);
      for (hpointer = pHelp->first_hindex; hpointer; hpointer = hpointer->next)
      {
         hindex = hpointer->pointer;
         if (cnt + strlen(hindex->keyword) > 76)
         {
             send_to_char("\n\r", ch);
             cnt = 0;
         }
         #ifdef HINDEX_MXP  
            ch_printf(ch, MXPTAG ("hindex '%s'") "&w&W%s"  MXPTAG ("/hindex") "    ", hindex->keyword, hindex->keyword);
         #else
            ch_printf(ch, "&w&W%s    ", hindex->keyword);
         #endif
         cnt+= strlen(hindex->keyword) + 4;  
      }
      send_to_char("\n\r", ch);
   } 
   return;
}

void do_news(CHAR_DATA * ch, char *argument)
{
   set_pager_color(AT_NOTE, ch);
   do_help(ch, "news");
}

extern char *help_greeting; /* so we can edit the greeting online */

/*
 * Help editor							-Thoric
 */
void do_hedit(CHAR_DATA * ch, char *argument)
{
   HELP_DATA *pHelp;

   if (!ch->desc)
   {
      send_to_char("You have no descriptor.\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;
      case SUB_HELP_EDIT:
         if ((pHelp = ch->dest_buf) == NULL)
         {
            bug("hedit: sub_help_edit: NULL ch->dest_buf", 0);
            stop_editing(ch);
            return;
         }
         if (help_greeting == pHelp->text)
            help_greeting = NULL;
         STRFREE(pHelp->text);
         pHelp->text = copy_buffer(ch);
         if (!help_greeting)
            help_greeting = pHelp->text;
         stop_editing(ch);
         return;
   }
   if ((pHelp = get_help(ch, argument)) == NULL) /* new help */
   {
      char argnew[MIL];
      int lev;

      if (isdigit(argument[0]))
      {
         lev = number_argument(argument, argnew);
         argument = argnew;
      }
      else
         lev = get_trust(ch);
      CREATE(pHelp, HELP_DATA, 1);
      pHelp->keyword = STRALLOC(strupper(argument));
      pHelp->text = STRALLOC("");
      pHelp->level = lev;
      //Add it into the general index...
      parse_helpfile_index(NULL, pHelp, HINDEX_GENERAL_NAME);
      add_help(pHelp);
   }
   ch->substate = SUB_HELP_EDIT;
   ch->dest_buf = pHelp;
   start_editing(ch, pHelp->text);
   editor_desc_printf(ch, "Help topic, keyword '%s', level %d.", pHelp->keyword, pHelp->level);
}

/*
 * Stupid leading space muncher fix				-Thoric
 */
char *help_fix(char *text)
{
   char *fixed;

   if (!text)
      return "";
   fixed = strip_cr(text);
   if (fixed[0] == ' ')
      fixed[0] = '.';
   return fixed;
}

void do_pretitle(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   int value;

   if (IS_NPC(ch))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }

   if (IS_SET(ch->pcdata->flags, PCFLAG_NOTITLE))
   {
      set_char_color(AT_IMMORT, ch);
      send_to_char("The Gods prohibit you from changing your title or pretitle.\n\r", ch);
      return;
   }

   if (ch->pcdata->pretit == '\0')
      ch->pcdata->pretit = "&G";

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: pretitle show\n\r", ch);
      send_to_char("Syntax: pretitle clear\n\r", ch);
      send_to_char("Syntax: pretitle <text>\n\r", ch);
      return;
   }

   if (!str_cmp(argument, "show"))
   {
      sprintf(buf, "Your current pretitle is '%s&g'.\n\r", ch->pcdata->pretit);
      send_to_char(buf, ch);
      return;
   }

   if (!str_cmp(argument, "clear"))
   {
      /* ch->pcdata->pretit = str_dup( "" ); */
      ch->pcdata->pretit = "&G";
      send_to_char("Pretitle Removed.\n\r", ch);
      return;
   }

   if (strlen(argument) > 25)
   {
      argument[25] = '&';
      argument[26] = 'G';
      argument[27] = '\0';
   }
   else
   {
      value = strlen(argument);
      argument[value] = '&';
      argument[value + 1] = 'G';
      argument[value + 2] = '\0';
   }


   ch->pcdata->pretit = str_dup(argument);
   send_to_char("Done.\n\r", ch);
   return;

}

/* Used in hindex to do varius functions, it is controlled by the use value
0 - Show full map
1 - Search for a value
2 - Output for collapse
*/
HINDEX_DATA *hindex_search_function(int level, CHAR_DATA *ch, HINDEX_DATA *hindex, HINDEX_DATA *tindex, char *argument, int use)
{
   char sbuf[200];
   int cnt = 0;
   int ddown = 0;
   int dnext = 0;
   HINDEX_DATA *rvalue = NULL;
   
   if (use == 0)
   {
      strcpy(sbuf, "");   
      for (;;)
      {
         if (cnt < level)
         {
            strcat(sbuf, "               ");
         }
         else
            break;
         cnt++;
      }
      #ifdef HINDEX_MXP  
         ch_printf(ch, "%s" MXPTAG("hindex '%s'") "&w&W%s\n\r" MXPTAG("/hindex"), sbuf, hindex->keyword, hindex->keyword); 
      #else
         ch_printf(ch, "&w&W%s%s\n\r", sbuf, hindex->keyword); 
      #endif
   }
   if (use == 1)
   {
      if (!str_cmp(hindex->keyword, argument))
         return hindex;
   }
   if (use == 2)
   {
      sprintf(sbuf, "%s collapse", hindex->keyword);
      do_hindex(ch, sbuf);
   }
      
   for (;;)
   {   
      if (hindex->first_hindex && ddown == 0)   
      {
         rvalue = hindex_search_function(level+1, ch, hindex->first_hindex, hindex, argument, use);
         if (rvalue && use == 1)
            return rvalue;
         ddown = 1;
      }
      else if (hindex->next && dnext == 0)
      {
         rvalue = hindex_search_function(level, ch, hindex->next, tindex, argument, use);
         if (rvalue && use == 1)
            return rvalue;
         dnext = 1;
      }
      else
         return rvalue; //Go back up a level or finally exit
   }
}
   
void do_hindex(CHAR_DATA * ch, char *argument)
{
   HINDEX_DATA *hindex;
   HINDEX_DATA *oindex;
   HINDEX_IPOINTER *hipointer;
   HELP_DATA *help;
   int cnt = -1;
   char helpfile[100];
   char *helpstring;
   char arg[MIL];
   char arg2[MIL];
   
   if (argument[0] == '\0')
   {
      #ifdef HINDEX_MXP  
         send_to_char("Syntax:  " MXPTAG ("hindex toplevel")"hindex toplevel\n\r" MXPTAG ("/hindex"), ch);
         send_to_char("Syntax:  hindex <index> [nohfiles/collapse]\n\r", ch);
         send_to_char("Syntax:  " MXPTAG ("hindex map")"hindex map\n\r" MXPTAG ("/hindex"), ch);
         send_to_char("Help hindex for more information\n\r", ch);
      #else
         send_to_char("Syntax:  hindex toplevel\n\r", ch);
         send_to_char("Syntax:  hindex <index> [nohfiles/collapse]\n\r", ch);
         send_to_char("Syntax:  hindex map\n\r", ch);
         send_to_char("Help hindex for more information\n\r", ch);
      #endif
      return;
   }
   argument = one_argument(argument, arg);
   
   if (!str_cmp(arg, "toplevel"))//Show only the top level indexes, no helpfiles
   {
      send_to_char("&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CTop Level of the Index&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-\n\r", ch);
      for (hindex = first_hindex; hindex; hindex = hindex->next)
      {
         cnt++;
         if (cnt % 3 == 0)
            send_to_char("\n\r", ch);
         #ifdef HINDEX_MXP
            ch_printf(ch, MXPTAG ("hindex '%s'") "&w&R%s" MXPTAG ("/hindex") "%s", hindex->keyword, hindex->keyword, add_wspace(strlen(hindex->keyword), 25));
         #else
            ch_printf(ch, "&w&R%s%s", hindex->keyword, add_wspace(strlen(hindex->keyword), 25));   
         #endif
      }
      send_to_char("\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "map"))//Show a map of the index top to bottom
   {
      send_to_char("&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CFull Map of the Index&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-\n\r", ch);
      hindex_search_function(0, ch, first_hindex, NULL, NULL, 0); //Need to do this because I need multiple variables to hold the loop
      return;
   }
   if ((hindex = hindex_search_function(0, ch, first_hindex, NULL, arg, 1)) != NULL) //Need to do this because I need multiple variables to hold the loop)
   {     
      if (argument[0] == '\0') //do not collapse, show helpfiles
      {
         #ifdef HINDEX_MXP
            ch_printf(ch, "&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CHelpfiles in " MXPTAG ("hindex '%s'") "%s" MXPTAG ("/hindex") " Index&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r", hindex->keyword, hindex->keyword);
         #else
            ch_printf(ch, "&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CHelpfiles in %s Index&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r", hindex->keyword);   
         #endif
         {
            for (hipointer = hindex->first_help; hipointer; hipointer = hipointer->next)
            {
               help = hipointer->pointer;
               helpstring = help->keyword;
               for (;;)
               {
                  if (helpstring[0] == '\0')
                     break;
                  else
                  {
                     helpstring = one_argument(helpstring, helpfile);
                     cnt++;
                     if (cnt % 3 == 0)
                        send_to_char("\n\r", ch);
                     #ifdef HINDEX_MXP   
                        ch_printf(ch, MXPTAG ("help '%s'") "&w&R%s"  MXPTAG ("/help") "%s", helpfile, helpfile, add_wspace(strlen(helpfile), 25));
                     #else
                        ch_printf(ch, "&w&R%-25s     ", helpfile);
                     #endif
                  }
               }
            }
            cnt = -1;
            ch_printf(ch, "\n\r\n\r&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CIndexes Above %s&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-\n\r", hindex->keyword);
            for (oindex = hindex->first_top_hindex; oindex; oindex = oindex->tnext)
            {
               cnt++;
               if (cnt % 3 == 0)
                  send_to_char("\n\r", ch);
               #ifdef HINDEX_MXP
                  ch_printf(ch, MXPTAG ("hindex '%s'") "&c&w%s" MXPTAG ("/hindex") "%s", oindex->keyword, oindex->keyword, add_wspace(strlen(oindex->keyword), 25));
               #else
                  ch_printf(ch, "&c&w%s%s", oindex->keyword, add_wspace(strlen(oindex->keyword), 25));
               #endif
            }  
            cnt = -1;
            ch_printf(ch, "\n\r\n\r&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CIndexes Below %s&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-\n\r", hindex->keyword);
            for (oindex = hindex->first_hindex; oindex; oindex = oindex->next)
            {
               cnt++;
               if (cnt % 3 == 0)
                  send_to_char("\n\r", ch);
               #ifdef HINDEX_MXP
                  ch_printf(ch, MXPTAG ("hindex '%s'") "&w&W%s" MXPTAG ("/hindex") "%s", oindex->keyword, oindex->keyword, add_wspace(strlen(oindex->keyword), 25));
               #else
                  ch_printf(ch, "&w&W%s%s", oindex->keyword, add_wspace(strlen(oindex->keyword), 25));
               #endif
            }         
            send_to_char("\n\r", ch);    
            #ifdef HINDEX_MXP
               ch_printf(ch, "\n\r&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-"MXPTAG ("hindex map")"&w&CFull index" MXPTAG ("/hindex") " &w&Cabove/below %s &w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r", hindex->keyword, hindex->keyword);
            #endif
            return; 
         }
      }
      argument = one_argument(argument, arg2);
      if ((!str_cmp(arg2, "nohfiles") && !str_cmp(argument, "collapse")) || (!str_cmp(arg2, "collapse") && !str_cmp(argument, "nohfiles")))
      {
         send_to_char("You can only choose one, not both.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "nohfiles")) //Show only the index
      {
         ch_printf(ch, "&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&W-&w&CFull Index Below %s&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-\n\r", hindex->keyword);
            hindex_search_function(0, ch, hindex, NULL, NULL, 0); 
      }
      if (!str_cmp(arg2, "collapse")) //Show the complete index below this one + Helpfiles
      {  
         if (showcollapse == 0)
         {
            ch_printf(ch, "&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&W-&w&CFull Index Below %s&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-\n\r", hindex->keyword);
            hindex_search_function(0, ch, hindex, NULL, NULL, 0); 
            
            showcollapse = 1;
            hindex_search_function(0, ch, hindex, NULL, NULL, 2); 
            showcollapse = 0;
            send_to_char("\n\r", ch);
            return;
         }
         else
         {
            cnt = -1;
            #ifdef HINDEX_MXP
               ch_printf(ch, "\n\r&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CHelpfiles in " MXPTAG ("hindex '%s'") "%s" MXPTAG ("/hindex") " Index&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r", hindex->keyword, hindex->keyword);
            #else
               ch_printf(ch, "\n\r&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&CHelpfiles in %s Index&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-&w&W-&w&c-\n\r", hindex->keyword);
            #endif
            for (hipointer = hindex->first_help; hipointer; hipointer = hipointer->next)
            {
               help = hipointer->pointer;
               helpstring = help->keyword;
               for (;;)
               {
                  if (helpstring[0] == '\0')
                     break;
                  else
                  {
                     helpstring = one_argument(helpstring, helpfile);
                     cnt++;
                     if (cnt % 3 == 0)
                        send_to_char("\n\r", ch);
                     #ifdef HINDEX_MXP
                        ch_printf(ch, MXPTAG ("help '%s'") "&w&R%s"  MXPTAG ("/help") "%s", helpfile, helpfile, add_wspace(strlen(helpfile), 25));
                     #else
                        ch_printf(ch, "&w&R%-25s     ", helpfile);
                     #endif
                  }
               }
            }
            send_to_char("\n\r", ch);
            return;
         }
      }
   }
   else
   {
      send_to_char("That is not an available index, if you cannot find one, view them all with hindex map.\n\r", ch);
      return;
   }
} 

HINDEX_DATA *get_hindex_name(char *argument, HELP_DATA *help)
{
   char index[100];
   HINDEX_DATA *hindex = NULL;
   HINDEX_POINTER *hpointer;
   
   for (;;) //Get the index's name
   {
      argument = one_argument(argument, index);
      if (argument[0] == '\0')
         break;
   }
   for (hpointer = help->first_hindex; hpointer; hpointer = hpointer->next)
   {
      if (!str_cmp(hpointer->pointer->keyword, index))
      {
         hindex = hpointer->pointer;
         break;
      }
   }
   if (!hindex)
   {
      bug("Hindex %s in helpfile %s is invalid", index, help->keyword);
      return NULL;
   }
   return hindex;
}

HINDEX_POINTER *get_hindex_pointer(HINDEX_DATA *hindex, HINDEX_POINTER *first)
{
   HINDEX_POINTER *hpointer;
   for (hpointer = first; hpointer; hpointer = hpointer->next)
   {
      if (hpointer->pointer == hindex)
         break;
   }
   if (!hpointer)
   {
      bug("hpointer error in helpfile %s", hindex->keyword);
      return NULL;
   }
   return hpointer;
}

   
void do_hset(CHAR_DATA * ch, char *argument)
{
   HELP_DATA *pHelp;
   HINDEX_NAME *hname;
   HINDEX_POINTER *hpointer = NULL;
   HINDEX_POINTER *chpointer = NULL;
   HINDEX_IPOINTER *hipointer;
   char buf[300];
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: hset <field> [value] [help page]\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Field being one of:\n\r", ch);
      send_to_char("  level keyword remove save\n\r", ch);
      send_to_char("------------------------------------------------\n\r", ch);
      send_to_char("Syntax: hset hindex <help page> view\n\r", ch);
      send_to_char("Syntax: hset hindex <help page> place <help index name>\n\r", ch);
      send_to_char("Syntax: hset hindex <help page> create <help index name>\n\r", ch);
      send_to_char("Syntax: hset hindex <help page> delete <number>\n\r", ch);
      send_to_char("Syntax: hset hindex <help page> shift <number> <up/down>\n\r", ch);
      send_to_char("Syntax: hset hindex crosslink <recepient index> <target index>\n\r", ch);
      send_to_char("------------------------------------------------\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg1, "hindex"))
   {
      HINDEX_DATA *hindex;
      HINDEX_DATA *chindex;
      HINDEX_NAME *chname;
      int value = 0;
      int cnt = 1;
      
      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);
      if (!str_cmp(arg2, "crosslink"))
      {
         for (hindex = first_fhindex; hindex; hindex = hindex->fnext)
         {
            if (!str_cmp(arg3, hindex->keyword))
               break;
         }
         if (!hindex)
         {
            ch_printf(ch, "%s does not exist as an index.\n\r", arg3);
         }
         for (chindex = first_fhindex; chindex; chindex = chindex->fnext)
         {
            if (!str_cmp(argument, chindex->keyword))
               break;
         }
         if (!chindex)
         {
            ch_printf(ch, "%s does not exist as an index.\n\r", argument);
         }
         LINK(chindex, hindex->first_hindex, hindex->last_hindex, next, prev);
         LINK(hindex, chindex->first_top_hindex, chindex->last_top_hindex, tnext, tprev);
         ch_printf(ch, "%s has been linked below %s, might check hindex map to make sure it doesn't loop.\n\r", argument, arg3);
         return;
      } 
                 
      if ((pHelp = get_help(ch, arg2)) == NULL)
      {
         send_to_char("There is no helpfile available by that name.\n\r", ch);
         return;
      }
      if (!str_cmp(arg3, "view")) //Limited information compared to hindex, mainly to get the number for delete
      {
         for (hname = pHelp->first_iname; hname; hname = hname->next)
         {   
            ch_printf(ch, "%-2d>  %s\n\r", cnt++, hname->name);
         }
         return;
      }
      if (!str_cmp(arg3, "create")) //Cannot exist, will create a new one :-)
      {
         sprintf(buf, argument);
         for (;;)
         {
            argument = one_argument(argument, arg4);
            if (argument[0] != '\0')
               continue;
            else //time to do a name check to make sure it exists
            {
               for (hindex = first_fhindex; hindex; hindex = hindex->fnext)
               {
                  if (!str_cmp(arg4, hindex->keyword))//Alright can add this helpfile now
                  {
                     ch_printf(ch, "%s already exists and therefore cannot be created\n\r", arg4);
                     return;
                  }
               }
               if (!hindex)
               {
                  argument = &buf[0];
                  parse_helpfile_index(NULL, pHelp, argument);
                  ch_printf(ch, "%s has been created and %s added to it!\n\r", arg4, arg2);
                  return;
               }
            }
         }
      }      
      if (!str_cmp(arg3, "place")) //Has to exist, mainly for error checking and level seperation for the two...
      {
         sprintf(buf, argument);
         for (;;)
         {
            argument = one_argument(argument, arg4);
            if (argument[0] != '\0')
               continue;
            else //time to do a name check to make sure it exists
            {
               for (hindex = first_fhindex; hindex; hindex = hindex->fnext)
               {
                  if (!str_cmp(arg4, hindex->keyword))//Alright can add this helpfile now
                  {
                     CREATE(hipointer, HINDEX_IPOINTER, 1);
                     hipointer->pointer = pHelp;
                     LINK(hipointer, hindex->first_help, hindex->last_help, next, prev);
                     CREATE(hpointer, HINDEX_POINTER, 1);
                     hpointer->pointer = hindex;
                     LINK(hpointer, pHelp->first_hindex, pHelp->last_hindex, next, prev);
                     CREATE(hname, HINDEX_NAME, 1);
                     hname->name = STRALLOC(buf);
                     LINK(hname, pHelp->first_iname, pHelp->last_iname, next, prev); 
                     ch_printf(ch, "%s has been placed in %s\n\r", arg2, arg4);
                     return;
                  }
               }
               if (!hindex)
               {
                  ch_printf(ch, "%s is an index that does not exist\n\r", arg4);
                  return;
               }
            }
         }
      }                     
      argument = one_argument(argument, arg4);    
      if (!str_cmp(arg3, "shift")) //Shift the help index up/down on a helpfile
      {
         value = atoi(arg4);
         if (!value)
         {
            send_to_char("You need to specify the number listed in view.\n\r", ch);
            return;
         }
         cnt = 1;
         if (!str_cmp(argument, "down"))
         {
            value = 0;
            for (hname = pHelp->first_iname; hname; hname = hname->next)
            {
               value++;
            }
            if (atoi(arg4) == value)
            {
               send_to_char("You cannot shift that down, it is at the bottom.\n\r", ch);
               return;
            }
            value = atoi(arg4);
            for (hname = pHelp->first_iname; hname; hname = hname->next)
            {
               if (cnt++ == value)
               {
                  if ((hindex = get_hindex_name(hname->name, pHelp)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  if ((hpointer = get_hindex_pointer(hindex, pHelp->first_hindex)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  break;
               }
            }
            if (!hname)
            {
               ch_printf(ch, "That is not in the valid range.\n\r", ch);
               return;
            }
            cnt = 1;
            for (chname = pHelp->first_iname; chname; chname = chname->next)
            {
               if (cnt++ == value+1)
               {
                  if ((chindex = get_hindex_name(chname->name, pHelp)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  if ((chpointer = get_hindex_pointer(chindex, pHelp->first_hindex)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  break;
               }
            }
            if (!hname)
            {
               ch_printf(ch, "That is not in the valid range.\n\r", ch);
               return;
            }
            SHIFT_DOWN(hname, chname, pHelp->last_iname);
            SHIFT_DOWN(hpointer, chpointer, pHelp->last_hindex); 
              
            ch_printf(ch, "%d is shifted below %d.\n\r", value, value+1);
            return;
         }
                 
         if (!str_cmp(argument, "up"))
         {
            if (value == 1)
            {
               ch_printf(ch, "You cannot shift that up, it is number 1 in the list.\n\r", ch);
               return;
            }
            for (hname = pHelp->first_iname; hname; hname = hname->next)
            {
               if (cnt++ == value)
               {
                  if ((hindex = get_hindex_name(hname->name, pHelp)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  if ((hpointer = get_hindex_pointer(hindex, pHelp->first_hindex)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  break;
               }
            }
            if (!hname)
            {
               ch_printf(ch, "That is not in the valid range.\n\r", ch);
               return;
            }
            cnt = 1;
            for (chname = pHelp->first_iname; chname; chname = chname->next)
            {
               if (cnt++ == value-1)
               {
                  if ((chindex = get_hindex_name(chname->name, pHelp)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  if ((chpointer = get_hindex_pointer(chindex, pHelp->first_hindex)) == NULL)
                  {
                     send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                     return;
                  }
                  break;
               }
            }
            if (!chname)
            {
               ch_printf(ch, "That is not in the valid range.\n\r", ch);
               return;
            }
            SHIFT_UP(hname, chname, pHelp->first_iname);
            SHIFT_UP(hpointer, chpointer, pHelp->first_hindex); 
            /*
            if (pHelp->first_iname == chname) 
               pHelp->first_iname = hname;
              
            hname->next = chname->next;
            if (hname->next)
               hname->next->prev = hname;
            chname->prev = hname->prev;
            if (chname->prev)
               chname->prev->next = chname;
            
            hname->prev = chname;
            chname->next = hname; */
            ch_printf(ch, "%d is shifted above %d.\n\r", value, value-1);
            return;
         }
      }
      if (!str_cmp(arg3, "delete"))
      {
         for (hname = pHelp->first_iname; hname; hname = hname->next)
         { 
            if (cnt++ == atoi(arg4)) //Delete out this hindex from the helpfile, it DOES NOT delete the hindex.  Reboot/copyover to do this
            {
               hindex = get_hindex_name(hname->name, pHelp);
               if (!hindex)
               {
                  send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                  return;
               }
               for (hpointer = pHelp->first_hindex; hpointer; hpointer = hpointer->next)
               {
                  if (hpointer->pointer == hindex)
                  {
                     break;
                  }
               }
               if (!hpointer)
               {
                  bug("Helpfile %s is not pointing correctly to a hindex", pHelp->keyword);
                  send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                  return;
               }  
               for (hipointer = hindex->first_help; hipointer; hipointer = hipointer->next)
               {
                  if (hipointer->pointer == pHelp)
                  {
                     break;
                  }
               }
               if (!hipointer)
               {
                  bug("Helpfile %s is not showing up in a hindex", pHelp->keyword);
                  send_to_char("There was a bug, notify an immortal if one doesn't know already.\n\r", ch);
                  return;
               }  
               ch_printf(ch, "%s was removed from the helpfile and the helpfile from the index.\n\r", hname->name);
               UNLINK(hipointer, hindex->first_help, hindex->last_help, next, prev);
               UNLINK(hpointer, pHelp->first_hindex, pHelp->last_hindex, next, prev);
               STRFREE(hname->name);
               UNLINK(hname, pHelp->first_iname, pHelp->last_iname, next, prev);
               return;
            }
         }
         send_to_char("That is not a valid number.\n\r", ch);
         return;
      }
   } 
   if (!str_cmp(arg1, "save"))
   {
      FILE *fpout;

      log_string_plus("Saving help.are...", LOG_NORMAL, LEVEL_STAFF); /* Tracker1 */

      rename("help.are", "help.are.bak");
      fclose(fpReserve);
      if ((fpout = fopen("help.are", "w")) == NULL)
      {
         bug("hset save: fopen", 0);
         perror("help.are");
         fpReserve = fopen(NULL_FILE, "r");
         return;
      }

      fprintf(fpout, "#HELPS\n\n");
      for (pHelp = first_help; pHelp; pHelp = pHelp->next)
      {
         fprintf(fpout, "%d %s~\n", pHelp->level, pHelp->keyword);
         if (!pHelp->first_iname)
            fprintf(fpout, "#%s~\n", HINDEX_GENERAL_NAME);
         else
         {
            for (hname = pHelp->first_iname; hname; hname = hname->next)
               fprintf(fpout, "#%s~\n", hname->name);
         }
         fprintf(fpout, "#END~\n%s~\n\n", help_fix(pHelp->text));
      }

      fprintf(fpout, "0 $~\n\n\n#$\n");
      fclose(fpout);
      fpReserve = fopen(NULL_FILE, "r");
      send_to_char("Saved.\n\r", ch);
      return;
   }
   
   if (str_cmp(arg1, "remove"))
      argument = one_argument(argument, arg2);

   if ((pHelp = get_help(ch, argument)) == NULL)
   {
      send_to_char("Cannot find help on that subject.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "remove"))
   {
      UNLINK(pHelp, first_help, last_help, next, prev);
      STRFREE(pHelp->text);
      STRFREE(pHelp->keyword);
      DISPOSE(pHelp);
      send_to_char("Removed.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "level"))
   {
      pHelp->level = atoi(arg2);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "keyword"))
   {
      STRFREE(pHelp->keyword);
      pHelp->keyword = STRALLOC(strupper(arg2));
      send_to_char("Done.\n\r", ch);
      return;
   }

   do_hset(ch, "");
}

//Shows the pkill status of the room you are in.
void do_pstatus(CHAR_DATA * ch, char *argument)
{
   int roomstat;

   roomstat = check_room_pk(ch);
   if (roomstat == 1)
   {
      send_to_char("This room is safe from pkill.\n\r", ch);
      return;
   }
   else if (roomstat == 4)
   {
      send_to_char("This room is full loot, be careful.\n\r", ch);
      return;
   }
   else if (roomstat == 3)
   {
      send_to_char("This room is an item dropped on death.\n\r", ch);
      return;
   }
   else if (roomstat == 2)
   {
      send_to_char("This room is open to pkill, but no looting.\n\r", ch);
      return;
   }
   else
   {
      send_to_char("This room is open to pkill, but no looting.\n\r", ch);
      return;
   }
}

void do_hl(CHAR_DATA * ch, char *argument)
{
   send_to_char("If you want to use HLIST, spell it out.\n\r", ch);
   return;
}

/*
 * Show help topics in a level range				-Thoric
 * Idea suggested by Gorog
 * prefix keyword indexing added by Fireblade
 */
void do_hlist(CHAR_DATA * ch, char *argument)
{
   int min, max, minlimit, maxlimit, cnt;
   char arg[MIL];
   HELP_DATA *help;
   bool minfound, maxfound;
   char *idx;

   maxlimit = get_trust(ch);
   minlimit = maxlimit >= LEVEL_STAFF ? -1 : 0; /* Tracker1 */

   min = minlimit;
   max = maxlimit;

   idx = NULL;
   minfound = FALSE;
   maxfound = FALSE;
   
   if (!str_cmp(argument, "sort"))
   {
      for (help = first_help; help; help = help->next)
      {
         if (help->level > 1)
         {
            if (help->level <= 50)
               help->level = 1;
            if (help->level > 50 && help->level < 55)
               help->level = 2;
            if (help->level > 54 && help->level < 61)
               help->level = 3;
            if (help->level > 60 && help->level < 71)
               help->level = 4;
            if (help->level >= 71)
               help->level = 6;
         }
      }
      return;
   }
   for (argument = one_argument(argument, arg); arg[0] != '\0'; argument = one_argument(argument, arg))
   {
      if (!isdigit(arg[0]))
      {
         if (idx)
         {
            set_char_color(AT_GREEN, ch);
            ch_printf(ch, "You may only use a single keyword to index the list.\n\r");
            return;
         }
         idx = STRALLOC(arg);
      }
      else
      {
         if (!minfound)
         {
            min = URANGE(minlimit, atoi(arg), maxlimit);
            minfound = TRUE;
         }
         else if (!maxfound)
         {
            max = URANGE(minlimit, atoi(arg), maxlimit);
            maxfound = TRUE;
         }
         else
         {
            set_char_color(AT_GREEN, ch);
            ch_printf(ch, "You may only use two level limits.\n\r");
            return;
         }
      }
   }

   if (min > max)
   {
      int temp = min;

      min = max;
      max = temp;
   }

   set_pager_color(AT_GREEN, ch);
   pager_printf(ch, "Help Topics in level range %d to %d:\n\r\n\r", min, max);
   for (cnt = 0, help = first_help; help; help = help->next)
      if (help->level >= min && help->level <= max && (!idx || nifty_is_name_prefix(idx, help->keyword)))
      {
         pager_printf(ch, "  %3d %s\n\r", help->level, help->keyword);
         ++cnt;
      }
   if (cnt)
      pager_printf(ch, "\n\r%d pages found.\n\r", cnt);
   else
      send_to_char("None found.\n\r", ch);

   if (idx)
      STRFREE(idx);

   return;
}


/* 
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 *
 * Who group by Altrag, Feb 28/97
 */
struct whogr_s
{
   struct whogr_s *next;
   struct whogr_s *follower;
   struct whogr_s *l_follow;
   DESCRIPTOR_DATA *d;
   int indent;
}      *first_whogr, *last_whogr;

struct whogr_s *find_whogr(DESCRIPTOR_DATA * d, struct whogr_s *first)
{
   struct whogr_s *whogr, *whogr_t;

   for (whogr = first; whogr; whogr = whogr->next)
      if (whogr->d == d)
         return whogr;
      else if (whogr->follower && (whogr_t = find_whogr(d, whogr->follower)))
         return whogr_t;
   return NULL;
}

void indent_whogr(CHAR_DATA * looker, struct whogr_s *whogr, int ilev)
{
   for (; whogr; whogr = whogr->next)
   {
      if (whogr->follower)
      {
         int nlev = ilev;
         CHAR_DATA *wch = (whogr->d->original ? whogr->d->original : whogr->d->character);

         if (can_see_map(looker, wch) && !IS_IMMORTAL(wch))
            nlev += 3;
         indent_whogr(looker, whogr->follower, nlev);
      }
      whogr->indent = ilev;
   }
}

/* This a great big mess to backwards-structure the ->leader character
   fields */
void create_whogr(CHAR_DATA * looker)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *wch;
   struct whogr_s *whogr, *whogr_t;
   int dc = 0, wc = 0;

   while ((whogr = first_whogr) != NULL)
   {
      first_whogr = whogr->next;
      DISPOSE(whogr);
   }
   first_whogr = last_whogr = NULL;
   /* Link in the ones without leaders first */
   for (d = last_descriptor; d; d = d->prev)
   {
      if (d->connected != CON_PLAYING && d->connected != CON_EDITING)
         continue;
      ++dc;
      wch = (d->original ? d->original : d->character);
      if (!wch->leader || wch->leader == wch || !wch->leader->desc || IS_NPC(wch->leader) || IS_IMMORTAL(wch) || IS_IMMORTAL(wch->leader))
      {
         CREATE(whogr, struct whogr_s, 1);

         if (!last_whogr)
            first_whogr = last_whogr = whogr;
         else
         {
            last_whogr->next = whogr;
            last_whogr = whogr;
         }
         whogr->next = NULL;
         whogr->follower = whogr->l_follow = NULL;
         whogr->d = d;
         whogr->indent = 0;
         ++wc;
      }
   }
   /* Now for those who have leaders.. */
   while (wc < dc)
      for (d = last_descriptor; d; d = d->prev)
      {
         if (d->connected != CON_PLAYING && d->connected != CON_EDITING)
            continue;
         if (find_whogr(d, first_whogr))
            continue;
         wch = (d->original ? d->original : d->character);
         if (wch->leader && wch->leader != wch && wch->leader->desc &&
            !IS_NPC(wch->leader) && !IS_IMMORTAL(wch) && !IS_IMMORTAL(wch->leader) && (whogr_t = find_whogr(wch->leader->desc, first_whogr)))
         {
            CREATE(whogr, struct whogr_s, 1);

            if (!whogr_t->l_follow)
               whogr_t->follower = whogr_t->l_follow = whogr;
            else
            {
               whogr_t->l_follow->next = whogr;
               whogr_t->l_follow = whogr;
            }
            whogr->next = NULL;
            whogr->follower = whogr->l_follow = NULL;
            whogr->d = d;
            whogr->indent = 0;
            ++wc;
         }
      }
   /* Set up indentation levels */
   indent_whogr(looker, first_whogr, 0);

   /* And now to linear link them.. */
   for (whogr_t = NULL, whogr = first_whogr; whogr;)
      if (whogr->l_follow)
      {
         whogr->l_follow->next = whogr;
         whogr->l_follow = NULL;
         if (whogr_t)
            whogr_t->next = whogr = whogr->follower;
         else
            first_whogr = whogr = whogr->follower;
      }
      else
      {
         whogr_t = whogr;
         whogr = whogr->next;
      }
}

void do_who(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char council_name[MIL];
   char invis_str[MIL];
   char char_name[MIL];
   char add_len[MIL];
   int sp1; /* Strlen color checks :-) defeat the color, addlen -- Xerves */
   int sx; /* For the For statement, addlen -- Xerves */
   char class_text[MIL];
   struct whogr_s *whogr, *whogr_p;
   DESCRIPTOR_DATA *d;
   int iRace;
   int iLevelLower;
   int iLevelUpper;
   int nNumber;
   int nMatch;
   bool rgfRace[MAX_RACE];
   bool fClassRestrict;
   bool fRaceRestrict;
   bool fImmortalOnly;
   bool fLeader;
   bool fPkill;
   bool fShowHomepage;
   bool fClanMatch; /* SB who clan (order),who guild, and who council */
   bool fCouncilMatch;
   bool fDeityMatch;
   bool fGroup;
   CLAN_DATA *pClan = NULL;
   COUNCIL_DATA *pCouncil = NULL;
   DEITY_DATA *pDeity = NULL;
   FILE *whoout = NULL;

   /*
      #define WT_IMM    0;
      #define WT_MORTAL 1;
      #define WT_DEADLY 2;
    */

   WHO_DATA *cur_who = NULL;
   WHO_DATA *next_who = NULL;
   WHO_DATA *first_mortal = NULL;
   WHO_DATA *first_imm = NULL;
   WHO_DATA *first_deadly = NULL;
   WHO_DATA *first_grouped = NULL;
   WHO_DATA *first_groupwho = NULL;


   /*
    * Set default arguments.
    */
   iLevelLower = 0;
   iLevelUpper = MAX_LEVEL;
   fClassRestrict = FALSE;
   fRaceRestrict = FALSE;
   fImmortalOnly = FALSE;
   fPkill = FALSE;
   fShowHomepage = FALSE;
   fClanMatch = FALSE; /* SB who clan (order), who guild, who council */
   fCouncilMatch = FALSE;
   fDeityMatch = FALSE;
   fGroup = FALSE; /* Alty who group */
   fLeader = FALSE;
   for (iRace = 0; iRace < MAX_RACE; iRace++)
      rgfRace[iRace] = FALSE;

#ifdef REQWHOARG
   /*
    * The who command must have at least one argument because we often
    * have up to 500 players on. Too much spam if a player accidentally
    * types "who" with no arguments.           --Gorog
    */
   if (ch && argument[0] == '\0')
   {
      send_to_pager_color("\n\r&GYou must specify at least one argument.\n\rUse 'who 1' to view the entire who list.\n\r", ch);
      return;
   }
#endif

   /*
    * Parse arguments.
    */
   nNumber = 0;
   for (;;)
   {
      char arg[MSL];

      argument = one_argument(argument, arg);
      if (arg[0] == '\0')
         break;

      if (is_number(arg))
      {
         switch (++nNumber)
         {
            case 1:
               iLevelLower = atoi(arg);
               break;
            case 2:
               iLevelUpper = atoi(arg);
               break;
            default:
               send_to_char("Only two level numbers allowed.\n\r", ch);
               return;
         }
      }
      else
      {
         if (strlen(arg) < 3)
         {
            send_to_char("Arguments must be longer than that.\n\r", ch);
            return;
         }

         /*
          * Look for classes to turn on.
          */
         if (!str_cmp(arg, "deadly") || !str_cmp(arg, "pkill"))
            fPkill = TRUE;
         else if (!str_cmp(arg, "imm") || !str_cmp(arg, "gods"))
            fImmortalOnly = TRUE;
         else if (!str_cmp(arg, "leader"))
            fLeader = TRUE;
         else if (!str_cmp(arg, "www"))
            fShowHomepage = TRUE;
         else if (!str_cmp(arg, "group") && ch)
            fGroup = TRUE;
         else /* SB who clan (order), guild, council */ if ((pClan = get_clan(arg)))
            fClanMatch = TRUE;
         else if ((pCouncil = get_council(arg)))
            fCouncilMatch = TRUE;
         else if ((pDeity = get_deity(arg)))
            fDeityMatch = TRUE;
         else
         {
            for (iRace = 0; iRace < MAX_RACE; iRace++)
            {
               if (!str_cmp(arg, race_table[iRace]->race_name))
               {
                  rgfRace[iRace] = TRUE;
                  break;
               }
            }
            if (iRace != MAX_RACE)
               fRaceRestrict = TRUE;

            if (iRace == MAX_RACE && fClanMatch == FALSE && fCouncilMatch == FALSE && fDeityMatch == FALSE)
            {
               send_to_char("That's not a race, order, guild," " council or deity.\n\r", ch);
               return;
            }
         }
      }
   }

   /*
    * Now find matching chars.
    */
   nMatch = 0;
   buf[0] = '\0';
   if (ch)
      set_pager_color(AT_GREEN, ch);
   else
   {
      if (fShowHomepage)
         whoout = fopen(WEBWHO_FILE, "w");
      else
         whoout = fopen(WHO_FILE, "w");
      if (!whoout)
      {
         bug("do_who: cannot open who file!");
         return;
      }
   }

/* start from last to first to get it in the proper order */
   if (fGroup)
   {
      create_whogr(ch);
      whogr = first_whogr;
      d = whogr->d;
   }
   else
   {
      whogr = NULL;
      d = last_descriptor;
   }
   whogr_p = NULL;
   for (; d; whogr_p = whogr, whogr = (fGroup ? whogr->next : NULL), d = (fGroup ? (whogr ? whogr->d : NULL) : d->prev))
   {
      CHAR_DATA *wch;

      if ((d->connected != CON_PLAYING && d->connected != CON_EDITING) || !can_see_map(ch, d->character))
         continue;

      wch = d->original ? d->original : d->character;

      if(IS_IMMORTAL(wch) && !xIS_SET(wch->act, PLR_ONDUTY) && get_trust(ch) == LEVEL_PC)
		continue;

      if (wch->level < iLevelLower
         || wch->level > iLevelUpper
         || (fPkill)
         || (fImmortalOnly && wch->level < LEVEL_IMMORTAL)
         || (fRaceRestrict && !rgfRace[wch->race]) || (fClanMatch && (pClan != wch->pcdata->clan)) /* SB */
         || (fCouncilMatch && (pCouncil != wch->pcdata->council)) /* SB */
         || (fDeityMatch && (pDeity != wch->pcdata->deity)))
         continue;
      if (fLeader && !(wch->pcdata->council &&
            ((wch->pcdata->council->head2 &&
!str_cmp(wch->pcdata->council->head2, wch->name)) ||
(wch->pcdata->council->head &&
!str_cmp(wch->pcdata->council->head, wch->name)))) &&
!(wch->pcdata->clan && ((wch->pcdata->clan->deity &&
!str_cmp(wch->pcdata->clan->deity, wch->name))
|| (wch->pcdata->clan->leader && !str_cmp(wch->pcdata->clan->leader, wch->name)) || (wch->pcdata->clan->number1 && !str_cmp(wch->pcdata->clan->number1, wch->name)) || (wch->pcdata->clan->number2 && !str_cmp(wch->pcdata->clan->number2, wch->name)))))
         continue;

      if (fGroup && !wch->leader && !IS_SET(wch->pcdata->flags, PCFLAG_GROUPWHO) && (!whogr_p || !whogr_p->indent))
         continue;

      nMatch++;

      if (fShowHomepage && wch->pcdata->homepage && wch->pcdata->homepage[0] != '\0')
         sprintf(char_name, "<A HREF=\"%s\">%s</A>", show_tilde(wch->pcdata->homepage), PERS_MAP(wch, ch));
      else
         sprintf(char_name, MXPTAG ("player '%s'") "%s"  MXPTAG ("/player"),PERS_MAP(wch, ch), PERS_MAP(wch, ch));
      /* Start addlen -- Xerves */
      /* Track length and then add, color problems :-), addlen -- Xerves */
      if (!can_see_map(ch, wch) || (!xIS_SET(ch->act, PLR_SHOWNAMES) && !xIS_SET(wch->act, PLR_UKNOWN) && get_wear_hidden_cloak(wch)))
      {
         sprintf(class_text, "&R[&G&WUnknown&R]&G    ");
      }
      else
      { 
         sp1 = strlen(race_table[wch->race]->race_name);
         sprintf(add_len, " ");
         /*  add_len = ""; */

         for (sx = 10; sx > sp1; sx--) /* sx is longest race */
            strcat(add_len, " ");

         sprintf(class_text, "&R[&G&W%s&R]&G%s", race_table[wch->race]->race_name, add_len);
      }
      if (!IS_NPC(ch))
         ch->pcdata->whonum = 1; //so they can't use the other whonumbers!

      if (wch->pcdata->council && !get_wear_hidden_cloak(wch)) //had a request for this...
      {
         strcpy(council_name, " [");
         if (wch->pcdata->council->head2 == NULL)
         {
            if (!str_cmp(wch->name, wch->pcdata->council->head))
               strcat(council_name, "Head of ");
         }
         else
         {
            if (!str_cmp(wch->name, wch->pcdata->council->head) || !str_cmp(wch->name, wch->pcdata->council->head2))
               strcat(council_name, "Co-Head of ");
         }
         strcat(council_name, wch->pcdata->council_name);
         strcat(council_name, "]");
         
         if (!str_cmp(wch->pcdata->council->name, "Newbie Council") && !IS_NPC(ch))
         {
            if (!IS_IMMORTAL(ch) && (ch->played + (current_time - ch->pcdata->logon)) > 14400)
               strcpy(council_name, "");
         }                         
      }
      else
         council_name[0] = '\0';

      if (xIS_SET(wch->act, PLR_WIZINVIS))
         sprintf(invis_str, "(%d) ", wch->pcdata->wizinvis);
      else
         invis_str[0] = '\0';
        
      /* addlin, changed the length to 26 :-), up or lower according to how much you changes
         sx -- Xerves */
/* New Suppot for multiple who layouts, starting at whonum = 1 */
      if (!IS_NPC(ch))
      {
         if (ch->pcdata->whonum == 1)
         {
            char mxpbuf[100];
            char plevel[100];
            
            if (!str_infix(wch->name, PERS_MAP(wch, ch)))
               sprintf(plevel, "&w&R[&w&W%d&w&R]&w&G ", get_player_statlevel(wch));
            else
               strcpy(plevel, "");
            
            if (IS_IMMORTAL(ch) && wch->desc && wch->desc->mxp)
            {
               if (wch->desc->mxpclient && !str_cmp(wch->desc->mxpclient, "zmud") && wch->desc->mxpversion >= 6.5)
                  sprintf(mxpbuf, " &w&B[&w&PMXP&w&B]&w");
               else
                  sprintf(mxpbuf, " [MXP]");
            }
            else
               strcpy(mxpbuf, "");
	        sprintf(buf, "%*s%s%s%s%s%s%s%s%s%s%s%s%s%s.%s%s%s\n\r",
               (fGroup ? whogr->indent : 0), "",
               //wch->level,
               //caste_name,
               class_text,
               invis_str,
               plevel,
               (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
               xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
               !IS_NPC(wch) ? NOT_AUTHED(wch) ? "[AUTH] " : "" : "",
               xIS_SET(wch->act, PLR_AWAY) ? xIS_SET(wch->act, PLR_AFK) ? "" : "[AWAY] " : "",
               xIS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
               xIS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
               xIS_SET(wch->act, PLR_THIEF) ? "(THIEF) " : "", show_pers_pretitle(wch, ch), char_name, show_pers_title(wch, ch), council_name, 
               #ifdef MCCP
                  IS_IMMORTAL(ch) ? wch->desc ? wch->desc->out_compress ? " [MCCP]" : "" : "" : "",
               #else
                  "",
               #endif
               mxpbuf);
         }
/* For 640x480 screens, no class/race stuff, just level */
/*
         else if (ch->pcdata->whonum == 2)
         {
            sprintf(buf, "%*s&c(&C%-2d&c) %s%s%s%s%s%s%s%s%s%s%s.%s%s\n\r",
               (fGroup ? whogr->indent : 0), "",
               wch->level,
               caste_name,
               invis_str,
               (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
               xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
               xIS_SET(wch->act, PLR_AWAY) ? xIS_SET(wch->act, PLR_AFK) ? "" : "[AWAY] " : "",
               xIS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
               xIS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
               xIS_SET(wch->act, PLR_THIEF) ? "(THIEF) " : "", show_pers_pretitle(wch, ch), char_name, show_pers_title(wch, ch), extra_title, council_name);
         }
*/
/* More Clan/Council Info */
/*
         else if (ch->pcdata->whonum == 3)
         {
            sprintf(buf, "%*s%-21s %s%s%s%s%s%s%s%s%s%s%s.%s\n\r",
               (fGroup ? whogr->indent : 0), "",
               clan_name,
               caste_name,
               invis_str,
               (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
               xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
               xIS_SET(wch->act, PLR_AWAY) ? xIS_SET(wch->act, PLR_AFK) ? "" : "[AWAY] " : "",
               xIS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
               xIS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
               xIS_SET(wch->act, PLR_THIEF) ? "(THIEF) " : "", show_pers_pretitle(wch, ch), char_name, show_pers_title(wch, ch), extra_title);
         }
         else if (ch->pcdata->whonum == 4)
         {
            sprintf(buf, "%*s&c(&C%-2d&c) &c%s&G %s&G%s    %s%s%s%s%s\n\r",
               (fGroup ? whogr->indent : 0), "",
               wch->level,
               home_name,
               caste_name,
               job_name,
               invis_str,
               (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
               xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "", xIS_SET(wch->act, PLR_AWAY) ? xIS_SET(wch->act, PLR_AFK) ? "" : "[AWAY] " : "", char_name);
         }
*/
         /* Same as 1, just in case they don't have the pcdata stuff set */
/*
         else
         {
            sprintf(buf, "%*s%-26s %s%s%s%s%s%s%s%s%s%s%s.%s%s\n\r",
               (fGroup ? whogr->indent : 0), "",
               class,
               caste_name,
               invis_str,
               (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
               xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
               xIS_SET(wch->act, PLR_AWAY) ? xIS_SET(wch->act, PLR_AFK) ? "" : "[AWAY] " : "",
               xIS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
               xIS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
               xIS_SET(wch->act, PLR_THIEF) ? "(THIEF) " : "", show_pers_pretitle(wch, ch), char_name, show_pers_title(wch, ch), extra_title, council_name);
         }
*/
      }

      /* For mobs, sends out the shorter version (640x480) */
      else
      {
	 sprintf(buf, "%*s%s%s%s%s%s%s%s%s%s%s%s%s.%s\n\r",
         //sprintf(buf, "%*s&c(&C%-2d&c) %s%s%s%s%s%s%s%s%s%s%s.%s%s\n\r",
            (fGroup ? whogr->indent : 0), "",
            //wch->level,
            //caste_name,
            class_text,
            invis_str,
            (wch->desc && wch->desc->connected) ? "[WRITING] " : "",
            xIS_SET(wch->act, PLR_AFK) ? "[AFK] " : "",
            !IS_NPC(wch) ? NOT_AUTHED(wch) ? "[AUTH] " : "" : "",
            xIS_SET(wch->act, PLR_AWAY) ? xIS_SET(wch->act, PLR_AFK) ? "" : "[AWAY] " : "",
            xIS_SET(wch->act, PLR_ATTACKER) ? "(ATTACKER) " : "",
            xIS_SET(wch->act, PLR_KILLER) ? "(KILLER) " : "",
            xIS_SET(wch->act, PLR_THIEF) ? "(THIEF) " : "", show_pers_pretitle(wch, ch), char_name, show_pers_title(wch, ch), council_name);
      }

      /*
       * This is where the old code would display the found player to the ch.
       * What we do instead is put the found data into a linked list
       */

      /* First make the structure. */
      CREATE(cur_who, WHO_DATA, 1);
      cur_who->text = str_dup(buf);
      if (IS_IMMORTAL(wch))
         cur_who->type = WT_IMM;
      else if (fGroup)
         if (wch->leader || (whogr_p && whogr_p->indent))
            cur_who->type = WT_GROUPED;
         else
            cur_who->type = WT_GROUPWHO;
      else
         cur_who->type = WT_MORTAL;

      /* Then put it into the appropriate list. */
      switch (cur_who->type)
      {
         case WT_MORTAL:
            cur_who->next = first_mortal;
            first_mortal = cur_who;
            break;
         case WT_DEADLY:
            cur_who->next = first_deadly;
            first_deadly = cur_who;
            break;
         case WT_GROUPED:
            cur_who->next = first_grouped;
            first_grouped = cur_who;
            break;
         case WT_GROUPWHO:
            cur_who->next = first_groupwho;
            first_groupwho = cur_who;
            break;
         case WT_IMM:
            cur_who->next = first_imm;
            first_imm = cur_who;
            break;
      }

   }


   /* Ok, now we have three separate linked lists and what remains is to 
    * display the information and clean up.
    */
   /*
    * Two extras now for grouped and groupwho (wanting group). -- Alty
    */
   if (first_mortal)
   {
      if (!ch)
         fprintf(whoout, "\n\r&pVisible Moratls&G\n\r\n\r");
      else
         send_to_pager("\n\r&pVisible Mortals&G\n\r\n\r", ch);
   }

   for (cur_who = first_mortal; cur_who; cur_who = next_who)
   {
      if (!ch)
      {
         fprintf(whoout, cur_who->text);
         set_char_color(AT_GREEN, ch);
      }
      else
      {
         send_to_pager(cur_who->text, ch);
         set_char_color(AT_GREEN, ch);
      }
      next_who = cur_who->next;
      DISPOSE(cur_who->text);
      DISPOSE(cur_who);
   }

   if (first_deadly)
   {
      if (!ch)
         fprintf(whoout, "\n\r-------------------------------[ DEADLY CHARACTERS ]-------------------------\n\r\n\r");
      else
         send_to_pager("\n\r-------------------------------[ DEADLY CHARACTERS ]--------------------------\n\r\n\r", ch);
      if (ch->pcdata->whonum == 4)
      {
         send_to_pager
            ("&c&wLv   Hometown         Caste           Job             Name\n\r----------------------------------------------------------------------------\n\r",
            ch);
      }
   }

   for (cur_who = first_deadly; cur_who; cur_who = next_who)
   {
      if (!ch)
      {
         fprintf(whoout, cur_who->text);
         set_char_color(AT_GREEN, ch);
      }
      else
      {
         send_to_pager(cur_who->text, ch);
         set_char_color(AT_GREEN, ch);
      }
      next_who = cur_who->next;
      DISPOSE(cur_who->text);
      DISPOSE(cur_who);
   }

   if (first_grouped)
   {
/*      if ( !ch )
        fprintf( whoout, "\n\r-----------------------------[ GROUPED CHARACTERS ]---------------------------\n\r\n\r" );
      else*/
      send_to_pager("\n\r-----------------------------[ GROUPED CHARACTERS ]---------------------------\n\r\n\r", ch);
   }
   for (cur_who = first_grouped; cur_who; cur_who = next_who)
   {
/*      if ( !ch )
        fprintf( whoout, cur_who->text );
      else*/
      send_to_pager(cur_who->text, ch);
      next_who = cur_who->next;
      DISPOSE(cur_who->text);
      DISPOSE(cur_who);
   }

   if (first_groupwho)
   {
      if (!ch)
         fprintf(whoout, "\n\r-------------------------------[ WANTING GROUP ]------------------------------\n\r\n\r");
      else
         send_to_pager("\n\r-------------------------------[ WANTING GROUP ]------------------------------\n\r\n\r", ch);
   }
   for (cur_who = first_groupwho; cur_who; cur_who = next_who)
   {
      if (!ch)
         fprintf(whoout, cur_who->text);
      else
         send_to_pager(cur_who->text, ch);
      next_who = cur_who->next;
      DISPOSE(cur_who->text);
      DISPOSE(cur_who);
   }

   if (first_imm)
   {
      if (!ch)
         fprintf(whoout, "\n\r&pVisible Immortals&G\n\r\n\r");
      else
         send_to_pager("\n\r&pVisible Immortals&G\n\r\n\r", ch);
    /*  if (ch->pcdata->whonum == 4)
      {
         send_to_pager
            ("&c&wLv   Hometown         Caste           Job             Name\n\r----------------------------------------------------------------------------\n\r",
            ch);
      } */
   }

   for (cur_who = first_imm; cur_who; cur_who = next_who)
   {
      if (!ch)
      {
         fprintf(whoout, cur_who->text);
         set_char_color(AT_GREEN, ch);
      }
      else
      {
         send_to_pager(cur_who->text, ch);
         set_char_color(AT_GREEN, ch);
      }
      next_who = cur_who->next;
      DISPOSE(cur_who->text);
      DISPOSE(cur_who);
   }

   if (!ch)
   {
      fprintf(whoout, "%d player%s with a Max of %d player%s since last Reboot.\n\r",
         nMatch, nMatch == 1 ? "" : "s", sysdata.maxplayers, sysdata.maxplayers == 1 ? "" : "s");
      fclose(whoout);
      return;
   }

   set_char_color(AT_YELLOW, ch);
   ch_printf(ch, "\n\r%d player%s with a Max of %d player%s since last Reboot.\n\r",
      nMatch, nMatch == 1 ? "" : "s", sysdata.maxplayers, sysdata.maxplayers == 1 ? "" : "s");
   return;
}

//used to introduce yourself to a character or introduce someone you know.  :-)
void do_introduce(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim, *other; //victim - introducer, other - Optional target introducee
   char arg[MIL];
   OBJ_DATA *light;
   OBJ_DATA *light2;
   INTRO_DATA *intro;
   int mod;
   other=NULL;
   
   if (IS_NPC(ch))
   {
      send_to_char("huh?\n\r", ch);
      return;
   }
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: introduce [self/other target/kingdom] [victim]\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg);
   
   if (!str_cmp(arg, "kingdom"))
   {
      int ht;
      if ((victim = get_char_room_new(ch, argument, 0)) == NULL)
      {
         send_to_char("The person who is receiving the introduction does not exist in this world.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
         ht = victim->m4;
      else
         ht = victim->pcdata->hometown;
      if (victim == ch)
      {
         send_to_char("You cannot target yourself to be introduced to a kingdom.\n\r", ch);
         return;
      }  
      if (ht < 2 || ht >= sysdata.max_kingdom)
      {
         send_to_char("That is not a valid kingdom that the victim is in.\n\r", ch);
         return;
      }
      for (intro = kingdom_table[ht]->first_introduction; intro; intro = intro->next)
      {
         if (intro->pid == other->pcdata->pid && abs(intro->value) > 100000)
         {
            ch_printf(ch, "The kingdom of %s already knows your face.\n\r", kingdom_table[ht]->name);
            return;
         }
      }
      if (!can_see_map(ch, victim) || !can_see_map(victim, ch))
      {
         send_to_char("It is not possible to introduce two people when one cannot see another.\n\r", ch);
         return;
      }
      if (IN_WILDERNESS(victim))
      {
         if (!IN_WILDERNESS(ch))
         {
            send_to_char("You are not in the wilderness with your target!.\n\r", ch);
            return;
         } 
         if (gethour() > 21 || gethour() < 6)
         {
            mod = 2;
            light = get_eq_char(victim, WEAR_LIGHT);
            light2 = get_eq_char(ch, WEAR_LIGHT);
            if (light && light2 && light->item_type == ITEM_LIGHT && light2->item_type == ITEM_LIGHT)
               mod = 4;
         }
         else
         {
            mod = 5;
         }
         if (abs(ch->coord->x - victim->coord->x) > mod || abs(ch->coord->y - victim->coord->y) > mod)
         {
            send_to_char("It is not possible to introduce yourself if both of you cannot see eachother.\n\r", ch);
            return;
         } 
      }
      else
      {
         if (!IN_SAME_ROOM(ch, victim))
         {
            send_to_char("It is not possible to introduce yourself if both parties cannot see eachother.\n\r", ch);
            return;
         } 
      } 
      intro = NULL;
      for (intro = kingdom_table[ht]->first_introduction; intro; intro = intro->next)
      {
         if (ch->pcdata->pid == intro->pid)
         {
            intro->value = 150000;
            intro->lastseen = time(0);
            break;
         }
      }
      if (!intro)
      {
         CREATE(intro, INTRO_DATA, 1);
         intro->value = 150000;
         intro->pid = ch->pcdata->pid;
         intro->lastseen = time(0);
         LINK(intro, kingdom_table[ht]->first_introduction, kingdom_table[ht]->last_introduction, next, prev);
      }
   
      act(AT_ACTION, "$n extends an introduction to $N's kingdom.", ch, NULL, victim, TO_NOTVICT);
      act(AT_ACTION, "You extend an introduction to $N's kingdom.", ch, NULL, victim, TO_CHAR);
      act(AT_ACTION, "$n extends an introduction to your kingdom.", ch, NULL, victim, TO_VICT);
      return;
   }
            
   if (str_cmp(arg, "self"))
   {
      if ((other = get_char_room_new(ch, arg, 0)) == NULL)
      {
         send_to_char("Your target you plan on introducing is not here.\n\r", ch);
         return;
      }
      if (IS_NPC(other))
      {
         send_to_char("That is a NPC!\n\r", ch);
         return;
      }
      for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
      {
         if (intro->pid == other->pcdata->pid && abs(intro->value) > 100000)
         {
            break;
         }
      }
      if (!intro)
      {
         send_to_char("You want to introduce who?\n\r", ch);
         return;
      }
   }
   if ((victim = get_char_room_new(ch, argument, 0)) == NULL)
   {
      send_to_char("The person who is receiving the introduction is not here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("That is a NPC!\n\r", ch);
      return;
   }
   if (victim == ch)
   {
      send_to_char("You cannot target yourself to be introduced to someone.\n\r", ch);
      return;
   }
   if (!other)
      other = ch;
      
   for (intro = victim->pcdata->first_introduction; intro; intro = intro->next)
   {
      if (intro->pid == other->pcdata->pid && abs(intro->value) > 100000)
      {
         if (other == ch)
            ch_printf(ch, "%s already knows you.\n\r", PERS_MAP(victim, ch));
         else
            ch_printf(ch, "%s already knows %s.\n\r", PERS_MAP(victim, ch), PERS_MAP(other, ch));
         return;
      }
   }
   
   if (IN_WILDERNESS(victim))
   {
      if (!IN_WILDERNESS(other))
      {
         send_to_char("It is not possible to introduce two people who cannot see eachother.\n\r", ch);
         return;
      } 
      if (gethour() > 21 || gethour() < 6)
      {
         mod = 2;
         light = get_eq_char(victim, WEAR_LIGHT);
         light2 = get_eq_char(other, WEAR_LIGHT);
         if (light && light2 && light->item_type == ITEM_LIGHT && light2->item_type == ITEM_LIGHT)
            mod = 4;
      }
      else
      {
         mod = 5;
      }
      if (abs(other->coord->x - victim->coord->x) > mod || abs(other->coord->y - victim->coord->y) > mod)
      {
         send_to_char("It is not possible to introduce two people who cannot see eachother.\n\r", ch);
         return;
      } 
   }
   else
   {
      if (!IN_SAME_ROOM(other, victim))
      {
         send_to_char("It is not possible to introduce two people who cannot see eachother.\n\r", ch);
         return;
      } 
   } 
   if (!can_see_map(other, victim) || !can_see_map(victim, other))
   {
      send_to_char("It is not possible to introduce two people when one cannot see another.\n\r", ch);
      return;
   }
   intro = NULL;
   for (intro = victim->pcdata->first_introduction; intro; intro = intro->next)
   {
      if (other->pcdata->pid == intro->pid)
      {
         intro->value = 150000;
         intro->lastseen = time(0);
         break;
      }
   }
   if (!intro)
   {
      CREATE(intro, INTRO_DATA, 1);
      intro->value = 150000;
      intro->pid = other->pcdata->pid;
      intro->lastseen = time(0);
      LINK(intro, victim->pcdata->first_introduction, victim->pcdata->last_introduction, next, prev);
   }
   
   if (ch == other)
   {
      act(AT_ACTION, "$n extends an introduction to $N.", other, NULL, victim, TO_NOTVICT);
      act(AT_ACTION, "You extend an introduction to $N.", other, NULL, victim, TO_CHAR);
      act(AT_ACTION, "$n extends an introduction to you.", other, NULL, victim, TO_VICT);
      
      ch->fame = ch->fame + 5;
   }
   else
   {
      ch_printf(ch, "You introduce %s to %s.\n\r", PERS_MAP(other, ch), PERS_MAP(victim, ch));
      ch_printf(other, "%s introduces you to %s.\n\r", PERS_MAP(ch, other), PERS_MAP(victim, other));
      ch_printf(victim, "%s introduces %s to you.\n\r", PERS_MAP(ch, victim), PERS_MAP(other, victim));
      act(AT_ACTION, "$n is introduced to $N.", other, NULL, victim, TO_NOTVICT);
   	
      ch->fame = ch->fame + 5;
   }
   return;
}
        
void do_compare(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   OBJ_DATA *obj1;
   OBJ_DATA *obj2;
   int value1;
   int value2;
   char *msg;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Compare what to what?\n\r", ch);
      return;
   }

   if ((obj1 = get_obj_carry(ch, arg1)) == NULL)
   {
      send_to_char("You do not have that item.\n\r", ch);
      return;
   }

   if (arg2[0] == '\0')
   {
      for (obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content)
      {
         if (obj2->wear_loc != WEAR_NONE
            && can_see_obj(ch, obj2) && obj1->item_type == obj2->item_type && (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE) != 0)
            break;
      }

      if (!obj2)
      {
         send_to_char("You aren't wearing anything comparable.\n\r", ch);
         return;
      }
   }
   else
   {
      if ((obj2 = get_obj_carry(ch, arg2)) == NULL)
      {
         send_to_char("You do not have that item.\n\r", ch);
         return;
      }
   }

   msg = NULL;
   value1 = 0;
   value2 = 0;

   if (obj1 == obj2)
   {
      msg = "You compare $p to itself.  It looks about the same.";
   }
   else if (obj1->item_type != obj2->item_type)
   {
      msg = "You can't compare $p and $P.";
   }
   else
   {
      switch (obj1->item_type)
      {
         default:
            msg = "You can't compare $p and $P.";
            break;

         case ITEM_ARMOR:
            value1 = obj1->value[0];
            value2 = obj2->value[0];
            break;

         case ITEM_WEAPON:
            value1 = obj1->value[1] + obj1->value[2];
            value2 = obj2->value[1] + obj2->value[2];
            break;
      }
   }

   if (!msg)
   {
      if (value1 == value2)
         msg = "$p and $P look about the same.";
      else if (value1 > value2)
         msg = "$p looks better than $P.";
      else
         msg = "$p looks worse than $P.";
   }

   act(AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR);
   return;
}



void do_where(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   bool found;

   one_argument(argument, arg);

   set_pager_color(AT_PERSON, ch);
   if (arg[0] == '\0')
   {
      pager_printf(ch, "\n\rPlayers near you in %s:\n\r", ch->in_room->area->name);
      found = FALSE;
      for (d = first_descriptor; d; d = d->next)
         if ((d->connected == CON_PLAYING || d->connected == CON_EDITING)
            && (victim = d->character) != NULL
            && !IS_NPC(victim) && victim->in_room && victim->in_room->area == ch->in_room->area && can_see_map(ch, victim))
         {
            found = TRUE;
/*		if ( CAN_PKILL( victim ) )
		  set_pager_color( AT_PURPLE, ch );
		else
		  set_pager_color( AT_PERSON, ch );
*/
            pager_printf_color(ch, "&P%-13s  ", PERS_MAP(victim, ch));
            if (victim->pcdata->clan && victim->pcdata->clan->clan_type != CLAN_ORDER && victim->pcdata->clan->clan_type != CLAN_GUILD)
               pager_printf_color(ch, "%-18s\t", victim->pcdata->clan->badge);
            else
               send_to_pager("\t\t\t", ch);
            pager_printf_color(ch, "&P%s\n\r", victim->in_room->name);
         }
      if (!found)
         send_to_char("None\n\r", ch);
   }
   else
   {
      found = FALSE;
      for (victim = first_char; victim; victim = victim->next)
         if (victim->in_room
            && victim->in_room->area == ch->in_room->area && !IS_AFFECTED(victim, AFF_STALK)
            && !IS_AFFECTED(victim, AFF_HIDE) && !IS_AFFECTED(victim, AFF_SNEAK) && can_see_map(ch, victim) && is_name(arg, PERS_MAP(victim, ch)))
         {
            found = TRUE;
            pager_printf(ch, "%-28s %s\n\r", PERS_MAP(victim, ch), victim->in_room->name);
            break;
         }
      if (!found)
         act(AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR);
   }

   return;
}

//Redone to base stats off of the mob table to give a more reliable consider
void do_consider(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   char *msg;
   int diff;
   sh_int hbefore, hafter, hp; //hp finding
   sh_int damf, daml, damnum, damsize, damplus, avgdam1, avgdam2;
   sh_int l;
   const int holdhp[12] = {
      0, 65, 120, 270, 430, 860, 1080, 1510, 1940, 2375, 3025, 3900
   };

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Consider killing whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They're not here.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("You can only consider mobs for now.\n\r", ch);
      return;
   }
   l = ch->level;
   //Base the HP off the HP chart based on character level.
   if (l % 5 == 0)
   {
      hp = holdhp[l / 5];
   }
   else
   {
      hbefore = l / 5;
      hafter = hbefore + 1;
      hp = ((holdhp[hafter] - holdhp[hbefore]) / 5) * abs((hafter * 5) - l - 5);
      hp = holdhp[hbefore] + hp;
   }

   damf = (l - 2) * .65;
   daml = (l - 2) * .35;

   if (damf < 1)
      damf = 1;
   if (daml < 1)
      daml = 1;

   damnum = l / 8;
   if (damnum < 1)
      damnum = 1;

   damsize = damf / damnum;
   if (damsize < 1)
      damnum = 1;

   damplus = daml;
   if (damplus < 1)
      damplus = 1;
   avgdam1 = damnum * damsize + damplus;
   avgdam2 = (victim->pIndexData->damnodice * victim->pIndexData->damsizedice) + victim->pIndexData->damplus;

   diff = victim->level - ch->level;

   if (diff <= -10)
      msg = "You are far more experienced than $N.";
   else if (diff <= -5)
      msg = "$N is not nearly as experienced as you.";
   else if (diff <= -2)
      msg = "You are more experienced than $N.";
   else if (diff <= 1)
      msg = "You are equals with $N.";
   else if (diff <= 4)
      msg = "$N is slightly more experienced than you.";
   else if (diff <= 6)
      msg = "$N is moderately more experienced than you.";
   else if (diff <= 9)
      msg = "$N is far more experienced than you!";
   else
      msg = "$N would make a great teacher for you!";
   act(AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR);

   diff = (int) (((victim->max_hit - hp) * 100) / hp);

   if (diff <= -50)
      msg = "$N will be destroyed in a few hits.";
   else if (diff <= -30)
      msg = "$N will fall quickly to your fury.";
   else if (diff <= -15)
      msg = "$N will not quite last with you blow to blow.";
   else if (diff <= -5)
      msg = "$N will be able to go with you blow to blow.";
   else if (diff <= 10)
      msg = "$N might have a small health advantage on you.";
   else if (diff <= 40)
      msg = "You might have a small problem with $N.";
   else if (diff <= 90)
      msg = "$N can take more damage than you can.";
   else if (diff <= 180)
      msg = "$N can take a great deal more damage than you can.";
   else if (diff <= 300)
      msg = "This battle might take awhile.";
   else if (diff <= 500)
      msg = "$N will DOMINATE YOU!!";
   else
      msg = "$N will OWN YOUR DEAD BODY!!";
   act(AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR);

   diff = (int) (((avgdam2 - avgdam1) * 100) / avgdam1);

   if (diff <= -50)
      msg = "$N cannot even cut a piece of paper.";
   else if (diff <= -30)
      msg = "$N has problems cutting into an apple.";
   else if (diff <= -15)
      msg = "$N swings fairly hard, but only enough to scratch.";
   else if (diff <= -5)
      msg = "If $N hits you, it is going to leave a small mark.";
   else if (diff <= 10)
      msg = "If $N lands a blow, it is going to hurt a bit.";
   else if (diff <= 40)
      msg = "If $N strikes you, it is going to bleed.";
   else if (diff <= 90)
      msg = "If I was you, I would dodge the attacks.";
   else if (diff <= 180)
      msg = "$N can cut off limbs in a few strikes.";
   else if (diff <= 300)
      msg = "$N can cut off limbs in one strike.";
   else if (diff <= 500)
      msg = "$N can take you apart in a few strikes.";
   else
      msg = "$N might as well be dropping a bomb on you.";
   act(AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR);

   return;
}

void do_version(CHAR_DATA *ch, char *argument)
{
   ch_printf(ch, "Mud Version: %s    Code Version: %s\n\r", sysdata.mversion, sysdata.cversion);
   return;
}

void do_viewskills(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char buf[MSL];
   char arg[MIL];
   int sn;
   sh_int type = 0;
   int col;
   sh_int cnt;

   if (IS_NPC(ch))
      return;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: sskills <target> <type>\n\rType being - magic skills tongues\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   
   if (arg[0] == '\0')
   {
      send_to_char("View who?\n\r", ch);
      return;
   }

   victim = get_char_world(ch, arg);

   if (victim == NULL)
   {
      send_to_char("No one like that in the whole world...\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on mobs.\n\r", ch);
      return;
   }

   if (!str_cmp(argument, "magic"))
      type = 1;
   else if (!str_cmp(argument, "skills"))
      type = 2;
   else if (!str_cmp(argument, "tongues"))
      type = 3;
   else
   {
      do_viewskills(ch, "");
      return;
   }
   col = cnt = 0;
   set_pager_color(AT_MAGIC, ch);
   if (type == 1)
   {
      if (!CAN_CAST(victim))
      {
         send_to_char("The target cannot cast spells.\n\r", ch);
         return;
      }
   }
   set_pager_color(AT_MAGIC, ch);
   if (type == 1)
   {
      send_to_pager_color("---------------------------------------------[Magic]---------------------------------------------\n\r", ch);
   }
   if (type == 2)
   {
      send_to_pager_color("---------------------------------------------[Skills]--------------------------------------------\n\r", ch);
   }
   if (type == 3)
   {
      send_to_pager_color("--------------------------------------------[Tongues]--------------------------------------------\n\r", ch);
   }
   for (sn = 0; sn < top_sn; sn++)
   {
      if (!skill_table[sn]->name)
         break;
      if (type == 1)
      {
         if (skill_table[sn]->type != SKILL_SPELL)
            continue;
      }
      if (type == 2)
      {
         if (skill_table[sn]->type != SKILL_SKILL)
            continue;
      }
      if (type == 3)
      {
         if (skill_table[sn]->type != SKILL_TONGUE)
            continue;
      }
      if (victim->pcdata->learned[sn] <= 0)
         continue;
      if (victim->pcdata->ranking[sn] <= 0)
         continue;


      ++cnt;
      set_pager_color(AT_MAGIC, ch);
      pager_printf(ch, "%20.20s", skill_table[sn]->name);
      send_to_char("&c&w", ch);
      if (victim->pcdata->ranking[sn] == 1)
         sprintf(buf, "Beginner");
      if (victim->pcdata->ranking[sn] == 2)
         sprintf(buf, "&w&cNovice  %s", char_color_str(AT_MAGIC, ch));
      if (victim->pcdata->ranking[sn] == 3)
         sprintf(buf, "&w&CExpert  %s", char_color_str(AT_MAGIC, ch));
      if (victim->pcdata->ranking[sn] == 4)
         sprintf(buf, "&w&RMaster  %s", char_color_str(AT_MAGIC, ch));
      if (victim->pcdata->ranking[sn] == 5)
         sprintf(buf, "&c&wElite   %s", char_color_str(AT_MAGIC, ch));
      if (victim->pcdata->ranking[sn] == 6)
         sprintf(buf, "&w&WFlawless%s", char_color_str(AT_MAGIC, ch));
         
      pager_printf(ch, " %s &G%2d %8s %4d ", get_rating(victim->pcdata->spercent[sn], 10000), victim->pcdata->learned[sn], buf, victim->pcdata->spercent[sn]);
      if (++col % 2 == 0)
         send_to_pager("\n\r", ch);
   }

   if (cnt == 0)
   {
      send_to_pager_color("None.\n\r", ch);
   }
   if (col % 2 != 0)
      send_to_pager("\n\r", ch);
}

//Sells SpellBooks
void do_sbook(CHAR_DATA * ch, char *argument)
{
   char arg1[MSL];
   char arg2[MSL];
   char arg3[MSL];
   char logb[MSL];
   char cbuf[5];
   char gbuf[30];
   char sbuf[30];
   CHAR_DATA *victim;
   int cl;
   OBJ_INDEX_DATA *obj;
   OBJ_DATA *bobj;
   TOWN_DATA *town = NULL;
   int vnum;
   int sn;
   int gr;
   int cost;

   if (IS_NPC(ch))
      return;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: sbook list\n\r", ch);
      send_to_char("Syntax: sbook list <group>\n\r", ch);
      send_to_char("Syntax: sbook buy <book>\n\r", ch);
      if (get_trust(ch) >= LEVEL_STAFF)
      {
         send_to_char("Syntax: sbook blist\n\r", ch);
         send_to_char("Syntax: sbook lbook yes\n\r", ch);
         send_to_char("Syntax: sbook abook <sn> <book vnum> [num]\n\r", ch);
         send_to_char("Syntax: sbook create <sn> <book vnum> <cost>\n\r", ch);
      }
      return;
   }
   argument = one_argument(argument, arg1);

   if (!str_cmp(arg1, "create") && get_trust(ch) >= LEVEL_STAFF)
   {
      int sn;
      int cost;
      
      argument = one_argument(argument, arg2);   
      argument = one_argument(argument, arg3);
      
      sn = atoi(arg2);
      
      if (sn < 1 || sn >= top_sn || !skill_table[sn] || !skill_table[sn]->name)
      {
         send_to_char("Not a valid sn.\n\r", ch);
         return;
      }
      if ((bobj = get_obj_here(ch, arg3)) == NULL)
      {
         ch_printf(ch, "You cannot seem to find %s", arg3);
         return;
      }
      cost = atoi(argument);
      if (cost < 1)
      {
         send_to_char("You can only specify a cost greater than 0.\n\r", ch);
         return;
      }

      sprintf(logb, "spbook book of %s", skill_table[sn]->name);
      STRFREE(bobj->pIndexData->name);
      bobj->pIndexData->name = STRALLOC(logb);
      STRFREE(bobj->name);
      bobj->name = STRALLOC(logb);
      
      sprintf(logb, "a book of %s", skill_table[sn]->name);
      STRFREE(bobj->pIndexData->short_descr);
      bobj->pIndexData->short_descr = STRALLOC(logb);
      STRFREE(bobj->short_descr);
      bobj->short_descr = STRALLOC(logb);
      
      sprintf(logb, "a book of %s awaits its next reader.", skill_table[sn]->name);
      STRFREE(bobj->pIndexData->description);
      bobj->pIndexData->description = STRALLOC(logb);
      bobj->pIndexData->value[1] = sn;
      bobj->pIndexData->cost = cost;
      SET_BIT(bobj->pIndexData->wear_flags, ITEM_TAKE);
      bobj->pIndexData->weight = 3;
      STRFREE(bobj->description);
      bobj->description = STRALLOC(logb);
      bobj->value[1] = sn;
      bobj->cost = cost;
      SET_BIT(bobj->wear_flags, ITEM_TAKE);
      bobj->weight = 0;
      bobj->pIndexData->item_type = ITEM_SPELLBOOK;
      bobj->item_type = ITEM_SPELLBOOK;
      xREMOVE_BIT(bobj->pIndexData->extra_flags, ITEM_PROTOTYPE);
      xREMOVE_BIT(bobj->extra_flags, ITEM_PROTOTYPE);
      xREMOVE_BIT(bobj->pIndexData->extra_flags, ITEM_ONMAP);
      xREMOVE_BIT(bobj->extra_flags, ITEM_ONMAP);
      send_to_char("Done.\n\r", ch);
      return;
   }  

   if (!str_cmp(arg1, "abook") && get_trust(ch) >= LEVEL_STAFF)
   {
      int sn;

      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);

      if (is_number(arg2))
      {
         if (((atoi(arg3) >= OBJ_FIRST_SBOOK && atoi(arg3) <= OBJ_LAST_SBOOK) || (!str_cmp(arg3, "keep")))
            && atoi(arg2) >= 1 && atoi(arg2) < MAX_SKILL && skill_table[atoi(arg2)])
         {
            sn = atoi(arg2);
            if (str_cmp(arg3, "keep"))
               skill_table[sn]->bookv = atoi(arg3);
            else
               sprintf(arg3, "%d", skill_table[sn]->bookv);
            ch_printf(ch, "Skill '%s' is now used by book vnum %d\n\r", skill_table[sn]->name, atoi(arg3));
         }
         else
            sn = -1;
      }
      else
      {
         sn = skill_lookup(arg2);
         if (((atoi(arg3) >= OBJ_FIRST_SBOOK && atoi(arg3) <= OBJ_LAST_SBOOK) || (!str_cmp(arg3, "keep"))) && sn >= 0 && skill_table[sn])
         {
            if (str_cmp(arg3, "keep"))
               skill_table[sn]->bookv = atoi(arg3);
            else
               sprintf(arg3, "%d", skill_table[sn]->bookv);
            ch_printf(ch, "Skill %s is now used by book vnum %d\n\r", skill_table[sn]->name, atoi(arg3));
         }
      }
      cl = 0;
      if (argument[0] != '\0' && sn >= 1)
      {
         if (!str_cmp(argument, "keep"))
            return;
         else if (is_number(argument))
            skill_table[sn]->bookinfo[cl] = atoi(argument);
         else
            skill_table[sn]->bookinfo[cl] = 1;
      }
      else
      {
         if (sn >= 1)
            skill_table[sn]->bookinfo[cl] = 1;
      }
      return;
   }
   cl = 0;
   if (!str_cmp(arg1, "blist") && get_trust(ch) >= LEVEL_STAFF)
   {
      send_to_char("&RName of spell                          Vnum     Num/Mob Vnum\n\r", ch);
      send_to_char("&R------------------------------------------------------------\n\r", ch);
      for (sn = 1; sn < MAX_SKILL; sn++)
      {
         if (skill_table[sn] && skill_table[sn]->bookv > 0) //Book exists
         {
            ch_printf(ch, "&c&w%-40s  %-5d    %-d\n\r", skill_table[sn]->name, skill_table[sn]->bookv, skill_table[sn]->bookinfo[cl]);
         }
      }
      return;
   }
   if (!str_cmp(arg1, "lbook") && get_trust(ch) >= LEVEL_STAFF)
   {
      if (str_cmp(argument, "yes"))
      {
         send_to_char("You need to type yes with this command, please read helpfile first though.\n\r", ch);
         return;
      }
      for (sn = 1; sn < MAX_SKILL; sn++)
      {
         for (vnum = OBJ_FIRST_SBOOK; vnum <= OBJ_LAST_SBOOK; vnum++)
         {
            if ((obj = get_obj_index(vnum)) && skill_table[sn] && skill_table[sn]->type == SKILL_SPELL &&
               nifty_is_name(skill_table[sn]->name, obj->name))
            {
               if (skill_table[sn]->bookv != vnum && skill_table[sn]->bookv != -1 && skill_table[sn]->bookv != 0)
               {
                  bug("Sn %d was set to book vnum %d which already had book %d.\n\r", sn, vnum, skill_table[sn]->bookv);
               }
               skill_table[sn]->bookv = vnum;
            }
         }
      }
      return;
   }
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (xIS_SET(victim->act, ACT_SBSELLER))
         break;
   }
   if (victim == NULL)
   {
      send_to_char("There is no merchant here selling Spell Books.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "buy"))
   {
      int sn;
      int tax;

      sn = skill_lookup(argument);
      if (sn >= 0 && skill_table[sn])
      {
         if (skill_table[sn]->bookinfo[cl] == victim->m1 && skill_table[sn]->masterydiff[cl] > 0)
         {
            obj = get_obj_index(skill_table[sn]->bookv);
            if (obj)
            {
               int kv;
               cost = obj->cost;
               
               if (IN_WILDERNESS(ch))
                  kv = kingdom_sector[ch->map][ch->coord->x][ch->coord->y];
               else
                  kv = 0;
               if (kv > 1 && kv < sysdata.max_kingdom)
               {
                  if (kingdom_table[kv]->bvisitor < 100)
                     kingdom_table[kv]->bvisitor = 100;
                  if (kingdom_table[kv]->tier1book < 100)
                     kingdom_table[kv]->tier1book = 100;
                  if (kingdom_table[kv]->tier2book < 100)
                     kingdom_table[kv]->tier2book = 100;
                  if (kingdom_table[kv]->tier3book < 100)
                     kingdom_table[kv]->tier3book = 100;
                  if (kingdom_table[kv]->tier4book < 100)
                     kingdom_table[kv]->tier4book = 100;
                  if (skill_table[sn]->masterydiff[0] == 1) //Tier 1 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier1book / 100;
                     }
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                        cost = cost * kingdom_table[kv]->tier1book / 100 * kingdom_table[kv]->bvisitor / 100;
                  }
                  if (skill_table[sn]->masterydiff[0] == 2) //Tier 2 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier2book / 100;
                     } 
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                       cost = cost * kingdom_table[kv]->tier2book / 100 * kingdom_table[kv]->bvisitor / 100;
                  }
                  if (skill_table[sn]->masterydiff[0] == 3) //Tier 3 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier3book / 100;
                     }  
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                        cost = cost * kingdom_table[kv]->tier3book / 100 * kingdom_table[kv]->bvisitor / 100;
                  
                  }
                  if (skill_table[sn]->masterydiff[0] == 4) //Tier 4 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier4book / 100;
                     }  
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                        cost = cost * kingdom_table[kv]->tier4book / 100 * kingdom_table[kv]->bvisitor / 100;
                  }
               }
               if (cost > ch->gold)
               {
                  ch_printf(ch, "You do not have enough money to buy the spellbook of %s\n\r", obj->short_descr);
                  return;
               }

               bobj = create_object(obj, ch->level);
               obj_to_char(bobj, ch);
               ch_printf(ch, "You purchased the spellbook of %s for %d coins\n\r", obj->short_descr, cost);
               ch->gold -= cost;
               tax = cost - obj->cost;
               
               if (kv && (town = find_town(ch->coord->x, ch->coord->y, ch->map)) != NULL)
               {
                  town->coins += tax;
               }
               if (tax > 0 && town)
               {
                  sprintf(logb, "%s was taxed %d for using %s to purchase %s", PERS_KINGDOM(ch, ch->in_room->area->kingdom), tax, victim->name, obj->short_descr);
                  write_kingdom_logfile(town->kingdom, logb, KLOG_BOOKTAX);  
               }
               return;
            }
            else
            {
               send_to_char("There is a problem with this spellbook, please report it to an imm.\n\r", ch);
               return;
            }
         }
         else
         {
            if (skill_table[sn]->bookinfo[cl] != victim->m1)
               ch_printf(ch, "This book seller does not sell the book of %s\n\r", skill_table[sn]->name);
            else
               ch_printf(ch, "No such book exists.\n\r", skill_table[sn]->name);

            return;
         }
      }
      else
      {
         send_to_char("That is not a spell.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg1, "list"))
   {
      if (argument[0] == '\0') //List all available books
      {
         int kv;
         send_to_char("&RName of Spell                   Group                Sphere          Cost    Mastery\n\r", ch);
         send_to_char("--------------------------------------------------------------------------------------\n\r", ch);
         for (sn = 1; sn < MAX_SKILL; sn++)
         {
            if (skill_table[sn] && skill_table[sn]->bookinfo[cl] == victim->m1 && skill_table[sn]->masterydiff[cl] > 0 && skill_table[sn]->bookv > 0) //This seller sells this book
            {
               if (skill_table[sn]->masterydiff[cl] == 1)
                  sprintf(cbuf, "&YTier 1");
               else if (skill_table[sn]->masterydiff[cl] == 2)
                  sprintf(cbuf, "&cTier 2");
               else if (skill_table[sn]->masterydiff[cl] == 3)
                  sprintf(cbuf, "&CTier 3");
               else
                  sprintf(cbuf, "&RTier 4");
               cost = -1;
               obj = get_obj_index(skill_table[sn]->bookv);
               
               if (obj)
                  cost = obj->cost;
               else
               {
                  bug("Vnum %d is being used for a spellbook (sn %d), and the vnum does not exist.", skill_table[sn]->bookv, sn);
                  continue;
               }
               
               if (IN_WILDERNESS(ch))
                  kv = kingdom_sector[ch->map][ch->coord->x][ch->coord->y];
               else
                  kv = 0;
               if (kv > 1 && kv < sysdata.max_kingdom)
               {
                  if (skill_table[sn]->masterydiff[0] == 1) //Tier 1 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier1book / 100;
                     }
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                        cost = cost * kingdom_table[kv]->tier1book / 100 * kingdom_table[kv]->bvisitor / 100;
                  }
                  if (skill_table[sn]->masterydiff[0] == 2) //Tier 2 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier2book / 100;
                     } 
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                       cost = cost * kingdom_table[kv]->tier2book / 100 * kingdom_table[kv]->bvisitor / 100;
                  }
                  if (skill_table[sn]->masterydiff[0] == 3) //Tier 3 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier3book / 100;
                     }  
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                        cost = cost * kingdom_table[kv]->tier3book / 100 * kingdom_table[kv]->bvisitor / 100;
                  
                  }
                  if (skill_table[sn]->masterydiff[0] == 4) //Tier 4 skill/spell
                  {
                     if (ch->pcdata->hometown == kv
                     &&  ch->pcdata->caste < kingdom_table[kv]->minbooktax)
                     {             
                        cost = cost * kingdom_table[kv]->tier4book / 100;
                     }  
                     if (kv != ch->pcdata->hometown && kingdom_table[kv]->bvisitor > 99)
                        cost = cost * kingdom_table[kv]->tier4book / 100 * kingdom_table[kv]->bvisitor / 100;
                  }
               }
                               
               sprintf(gbuf, "%s", get_group_name(skill_table[sn]->group[0]));
               sprintf(sbuf, "%s", get_sphere_name2(skill_table[sn]->stype));
          
               ch_printf(ch, "&c&w" MXPFTAG("Command 'sbook buy %s' desc='Click on this to buy %s'", "%s", "/Command") "%s  %-18s   %-12s   %6d   %-8s\n\r", 
                  skill_table[sn]->name, skill_table[sn]->name, skill_table[sn]->name, add_wspace(strlen(skill_table[sn]->name), 35),
                  gbuf, sbuf, cost, cbuf);
            }
         }
      }
      else
      {
         int sp;
         if ((gr = isgroup(argument)) == -1)
         {
            send_to_char("That is not a name of a group.\n\r", ch);
            return;
         }
         send_to_char("&RName of Spell                   Group                Sphere          Cost    Mastery\n\r", ch);
         send_to_char("--------------------------------------------------------------------------------------\n\r", ch);
         for (sp = 1; sp <= MAX_SPHERE; sp++)
         {
            for (sn = 1; sn < MAX_SKILL; sn++)
            {
               if (skill_table[sn] && skill_table[sn]->bookinfo[cl] == victim->m1 && skill_table[sn]->group[cl] == gr && skill_table[sn]->bookv > 0 && skill_table[sn]->stype == sp) //This seller sells this book
               {
                  if (skill_table[sn]->masterydiff[cl] == 1)
                     sprintf(cbuf, "&YTier 1");
                  else if (skill_table[sn]->masterydiff[cl] == 2)
                     sprintf(cbuf, "&cTier 2");
                  else if (skill_table[sn]->masterydiff[cl] == 3)
                     sprintf(cbuf, "&CTier 3");
                  else
                     sprintf(cbuf, "&RTier 4");
                  cost = -1;
                  obj = get_obj_index(skill_table[sn]->bookv);
                
                  if (obj)
                     cost = obj->cost;
                  else
                  {
                     bug("Vnum %d is being used for a spellbook (sn %d), and the vnum does not exist.", skill_table[sn]->bookv, sn);
                     continue;
                  }
                  
                  if (skill_table[sn]->masterydiff[0] == 1 && ch->in_room->area->kingdom > 1) //Tier 1 skill/spell
                  {
                     if (ch->pcdata->hometown == ch->in_room->area->kingdom
                     &&  ch->pcdata->caste < kingdom_table[ch->in_room->area->kingdom]->minbooktax)
                     {             
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier1book / 100;
                     }
                     if (ch->in_room->area->kingdom != ch->pcdata->hometown && kingdom_table[ch->in_room->area->kingdom]->bvisitor > 99)
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier1book / 100 * kingdom_table[ch->in_room->area->kingdom]->bvisitor / 100;
                  }
                  if (skill_table[sn]->masterydiff[0] == 2 && ch->in_room->area->kingdom > 1) //Tier 2 skill/spell
                  {
                     if (ch->pcdata->hometown == ch->in_room->area->kingdom
                     &&  ch->pcdata->caste < kingdom_table[ch->in_room->area->kingdom]->minbooktax)
                     {             
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier2book / 100;
                     } 
                     if (ch->in_room->area->kingdom != ch->pcdata->hometown && kingdom_table[ch->in_room->area->kingdom]->bvisitor > 99)
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier2book / 100 * kingdom_table[ch->in_room->area->kingdom]->bvisitor / 100;
                  }
                  if (skill_table[sn]->masterydiff[0] == 3 && ch->in_room->area->kingdom > 1) //Tier 3 skill/spell
                  {
                     if (ch->pcdata->hometown == ch->in_room->area->kingdom
                     &&  ch->pcdata->caste < kingdom_table[ch->in_room->area->kingdom]->minbooktax)
                     {             
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier3book / 100;
                     }  
                     if (ch->in_room->area->kingdom != ch->pcdata->hometown && kingdom_table[ch->in_room->area->kingdom]->bvisitor > 99)
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier3book / 100 * kingdom_table[ch->in_room->area->kingdom]->bvisitor / 100;
                     
                  }
                  if (skill_table[sn]->masterydiff[0] == 4 && ch->in_room->area->kingdom > 1) //Tier 4 skill/spell
                  {
                     if (ch->pcdata->hometown == ch->in_room->area->kingdom
                     &&  ch->pcdata->caste < kingdom_table[ch->in_room->area->kingdom]->minbooktax)
                     {             
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier4book / 100;
                     }  
                     if (ch->in_room->area->kingdom != ch->pcdata->hometown && kingdom_table[ch->in_room->area->kingdom]->bvisitor > 99)
                        cost += cost * kingdom_table[ch->in_room->area->kingdom]->tier4book / 100 * kingdom_table[ch->in_room->area->kingdom]->bvisitor / 100;
                  }
                 
                  sprintf(gbuf, "%s", get_group_name(skill_table[sn]->group[0]));
                  sprintf(sbuf, "%s", get_sphere_name2(skill_table[sn]->stype));
          
                  ch_printf(ch, "&c&w%-30s  %-18s   %-12s   %6d   %-8s\n\r", skill_table[sn]->name, gbuf, sbuf, cost, cbuf);
               }
            }
         }
      }
      return;
   }
   do_sbook(ch, "");
   return;
}

//Forget command, used to forget stuff and keep some things from going up
void do_forget(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int x, sn;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: forget list\n\r", ch);
      send_to_char("Syntax: forget unlearn <number> <skill/spell/stop>\n\r", ch);
      send_to_char("Syntax: forget stoplearning <number> <skill/spell/stop>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (!str_cmp(arg1, "stoplearning"))
   {
      if (atoi(arg2) < 1 || atoi(arg2) > 5)
      {
         send_to_char("The number value is 1 to 5.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "stop"))
      {
         ch->pcdata->nolearn[atoi(arg2)-1] = 0;
         ch_printf(ch, "%d has been removed from your stop learning list.\n\r", atoi(arg2));
         return;
      }
      sn = skill_lookup(argument);
      if (sn > 0)
      {
         ch->pcdata->nolearn[atoi(arg2)-1] = sn;
         ch_printf(ch, "You have added %s to your stop learning list.\n\r", skill_table[sn]->name);
         return;
      }
      else
      {
         send_to_char("That is not a valid skill/spell.\n\r", ch);
         return;
      }
   }
   
   if (!str_cmp(arg1, "unlearn"))
   {
      if (atoi(arg2) < 1 || atoi(arg2) > 5)
      {
         send_to_char("The number value is 1 to 5.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "stop"))
      {
         ch->pcdata->forget[atoi(arg2)-1] = 0;
         ch_printf(ch, "%d has been removed from your forget list.\n\r", atoi(arg2));
         return;
      }
      sn = skill_lookup(argument);
      if (sn > 0)
      {
         ch->pcdata->forget[atoi(arg2)-1] = sn;
         ch_printf(ch, "You have added %s to your forget list.\n\r", skill_table[sn]->name);
         return;
      }
      else
      {
         send_to_char("That is not a valid skill/spell.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg1, "list"))
   {
      send_to_char("Forget List\n\r-------------------------------------\n\r", ch);
      for (x = 0; x < 5; x++)
      {
         if (ch->pcdata->forget[x] > 0)
         {
            ch_printf(ch, "%s\n\r", skill_table[ch->pcdata->forget[x]]->name);
         }
         else
            ch_printf(ch, "None\n\r", ch);
      }
      send_to_char("\n\rStopLearning List\n\r-------------------------------------\n\r", ch);
      for (x = 0; x < 5; x++)
      {
         if (ch->pcdata->nolearn[x] > 0)
         {
            ch_printf(ch, "%s\n\r", skill_table[ch->pcdata->nolearn[x]]->name);
         }
         else
            ch_printf(ch, "None\n\r", ch);
      }
   }
}
   
   
/* Below is what is left of the old practice skill info routine.  This one
   returns either skills/magic/weapon/tongue and their skill/mastery -- Xerves 00*/
#define CANT_PRAC "Tongue"
void do_skills(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   int sn;
   sh_int type = 0;
   int col;
   sh_int cnt;

   if (IS_NPC(ch))
      return;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: skills <type>\n\rType being - ", ch);
      sprintf(buf, MXPFTAG("Command 'skills magic' desc='Click here to view the magic type'", "magic", "/Command"));
      ch_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("Command 'skills skills' desc='Click here to view the skill type'", "skills", "/Command"));
      ch_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("Command 'skills tongues' desc='Click here to view the tongue type'", "tongues", "/Command"));
      ch_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("Command 'skills percent' desc='Click here to view the percent type'", "percent", "/Command"));
      ch_printf(ch, "%s \n\r", buf);
      return;
   }
   if (!str_cmp(argument, "magic"))
      type = 1;
   else if (!str_cmp(argument, "skills"))
      type = 2;
   else if (!str_cmp(argument, "tongues"))
      type = 3;
   else if (!str_cmp(argument, "percent"))
      type = 4;
   else
   {
      do_skills(ch, "");
      return;
   }
   col = cnt = 0;
   if (type == 4)
   {
      int stotal, gtotal;
      int x, y, sn, diff;
      
      stotal=gtotal=diff=0;
      
      
      for (x = 1; x <= MAX_SPHERE; x++)
         ch->pcdata->spherepoints[x] = 0;
      for (x = 1; x <= MAX_GROUP+5; x++)
         ch->pcdata->grouppoints[x] = 0;
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
      
      for (x = 1; x <= MAX_SPHERE; x++)
         stotal += ch->pcdata->spherepoints[x];
         
      if (stotal == 0)
      {
         send_to_char("You have no skills or spells at this moment, so this command is useless.\n\r", ch);
         return;
      }
      send_to_char("&w&R-------------------------------------------------------------------------------\n\r", ch);   
      for (x = 1; x <= MAX_SPHERE; x++)
      {
         ch_printf(ch, "&w&WYou have %d percent focused in %s.\n\r", (100 * ch->pcdata->spherepoints[x] / stotal), get_sphere_name2(x));
      }
      send_to_char("&w&R-------------------------------------------------------------------------------\n\r", ch);
      for (x = 1; x <= MAX_SPHERE; x++)
      {
         if (ch->pcdata->spherepoints[x] == stotal)
            break;
      }
      if (x <= MAX_SPHERE)
      {
         ch_printf(ch, "&c&wYou are a specialist in %s, therefore you will learn everything in it at 100 percent.\n\r", get_sphere_name2(x));
         return;
      }
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if ((ch->pcdata->ranking[sn] >= 2 || ch->pcdata->learned[sn] >= 6) 
         &&  (skill_table[sn]->type == SKILL_SPELL || skill_table[sn]->type == SKILL_SKILL))
         {
            diff = 1; //Beyond the "grace period" above the curve
            break;
         }
      }
      if (diff == 0)
      {
         send_to_char("&c&wYou are still in the grace period, you will learn 100 percent in the dominant sphere\n\rand 70 percent in the rest.\n\r", ch);
         return;
      }
      else
      {
         for (x = 1; x <= MAX_SPHERE; x++)
         {
            if (ch->pcdata->spherepoints[x] == stotal)
               break;
         }  
         for (x = 1; x <= MAX_SPHERE; x++)
         {
            if (ch->pcdata->spherepoints[x] > stotal/2)
               break;
         }
         if (x <= MAX_SPHERE)
         {
            ch_printf(ch, "&c&wYou have a Clear Mastery in %s.\n\r", get_sphere_name2(x));
            for (y = 1; y <= MAX_GROUP+5; y++)
            {
               if (is_part_sphere(x, y))
                  gtotal += ch->pcdata->grouppoints[y];
            }
            for (y = 1; y <= MAX_GROUP+5; y++)
            {
               if (is_part_sphere(x, y))
                  ch_printf(ch, "&c&cYou will learn %s at %d percent.\n\r", get_group_name2(y), 60+ (40 * ch->pcdata->grouppoints[y] / gtotal));   
            }
            return;
         }
         send_to_char("You will learn the spheres at the above percentages listed.\n\r", ch);
         return;
      }
   }
   if (type == 1)
   {
      if (!CAN_CAST(ch) && !IS_IMMORTAL(ch))
      {
         send_to_char("You cannot cast spells.  Therefore you have no spells.\n\r", ch);
         return;
      }
   }
   set_pager_color(AT_MAGIC, ch);
   if (type == 1)
   {
      send_to_pager_color("----------------------------------------[Magic]----------------------------------------\n\r", ch);
   }
   if (type == 2)
   {
      send_to_pager_color("----------------------------------------[Skills]---------------------------------------\n\r", ch);
   }
   if (type == 3)
   {
      send_to_pager_color("---------------------------------------[Tongues]---------------------------------------\n\r", ch);
   }
   for (sn = 0; sn < top_sn; sn++)
   {
      if (!skill_table[sn]->name)
         break;
      if (type == 1)
      {
         if (skill_table[sn]->type != SKILL_SPELL)
            continue;
      }
      if (type == 2)
      {
         if (skill_table[sn]->type != SKILL_SKILL)
            continue;
      }
      if (type == 3)
      {
         if (skill_table[sn]->type != SKILL_TONGUE)
            continue;
      }
      if (ch->pcdata->learned[sn] <= 0)
         continue;
      if (ch->pcdata->ranking[sn] <= 0)
         continue;
      ++cnt;
      set_pager_color(AT_MAGIC, ch);
      pager_printf(ch, "%20.20s", skill_table[sn]->name);
      send_to_char("&c&w", ch);
      if (ch->pcdata->ranking[sn] == 1)
         sprintf(buf, "Beginner");
      if (ch->pcdata->ranking[sn] == 2)
         sprintf(buf, "&w&cNovice  %s", char_color_str(AT_MAGIC, ch));
      if (ch->pcdata->ranking[sn] == 3)
         sprintf(buf, "&w&CExpert  %s", char_color_str(AT_MAGIC, ch));
      if (ch->pcdata->ranking[sn] == 4)
         sprintf(buf, "&w&RMaster  %s", char_color_str(AT_MAGIC, ch));
      if (ch->pcdata->ranking[sn] == 5)
         sprintf(buf, "&c&wElite   %s", char_color_str(AT_MAGIC, ch));
      if (ch->pcdata->ranking[sn] == 6)
         sprintf(buf, "&w&WFlawless%s", char_color_str(AT_MAGIC, ch));
         
      pager_printf(ch, " %s &w&G%2d %8s ", get_rating(ch->pcdata->spercent[sn], 10000), ch->pcdata->learned[sn], buf);
      if (++col % 2 == 0)
         send_to_pager("\n\r", ch);
   } 

   if (cnt == 0)
   {
      send_to_pager_color("None.\n\r", ch);
   }
   if (col % 2 != 0)
      send_to_pager("\n\r", ch);
}

sh_int check_teacher(CHAR_DATA * ch, sh_int group, CHAR_DATA * mob)
{
   int sn;
   char teacher[MSL];
   char buf[MSL];
   sh_int fteach = -1;

   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      if (skill_table[sn]->group[0] == group)
      {
         if (skill_table[sn]->teachers && skill_table[sn]->teachers[0] != '\0')
         {
            if (fteach == 0)
            {
               act(AT_TELL, "$n tells you, 'I am sorry, I cannot teach you this due to a bug.'", mob, NULL, ch, TO_VICT);
               bug("sn %d in group %d has a teacher and shouldn't.", sn, group);
               return 0;
            }
            if (teacher[0] == '\0')
            {
               sprintf(teacher, "%s", skill_table[sn]->teachers);
               fteach = 1;
            }
            else
            {
               if (!str_cmp(teacher, skill_table[sn]->teachers))
               {
                  act(AT_TELL, "$n tells you, 'I am sorry, I cannot teach you this due to a bug.'", mob, NULL, ch, TO_VICT);
                  bug("sn %d in group %d has a different teacher.", sn, group);
                  return 0;
               }
            }
         }
         else
         {
            if (fteach == 1)
            {
               act(AT_TELL, "$n tells you, 'I am sorry, I cannot teach you this due to a bug.'", mob, NULL, ch, TO_VICT);
               bug("sn %d in group %d needs a teacher.", sn, group);
               return 0;
            }
            if (fteach == -1)
            {
               fteach = 0;
            }
         }
      }
   }
   if (fteach == 0)
      return 1;

   sprintf(buf, "%d", mob->pIndexData->vnum);
   if (!is_name(buf, teacher))
   {
      act(AT_TELL, "$n tells you, 'I know not know how to teach that.'", mob, NULL, ch, TO_VICT);
      return 0;
   }
   act(AT_TELL, "$n tells you, 'I am sorry, I cannot teach you this.'", mob, NULL, ch, TO_VICT);
   bug("Got to the end of check_teacher without exiting.", 0);
   return 0;
}

void update_skills(CHAR_DATA * ch, sh_int group, sh_int value)
{
   int sn;

   if ((group < 23 || group > 29) && (group < 31 || group > 34))
      ch->pcdata->spellpoints[group] = value;

   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      if (skill_table[sn]->group[0] == group)
      {
         if (ch->pcdata->ranking[sn] == 0)
            continue;
         else
            ch->pcdata->learned[sn] = value;

      }
      if (skill_table[sn]->group[0] == 30 && (group == 6 || group == 7))
      {
         if (ch->pcdata->ranking[sn] == 0)
            continue;
         else
            ch->pcdata->learned[sn] = value;
      }
   }
   return;
}

bool can_learn_skill(int sn, CHAR_DATA *ch)
{
   int mlearn = min_to_learn(sn);
   int csn = sn;
   int cnt = 0;
   
   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      if (skill_table[sn]->group[0] == skill_table[csn]->group[0]
      &&  skill_table[sn]->stype == skill_table[csn]->stype
      &&  skill_table[sn]->masterydiff[0] == skill_table[csn]->masterydiff[0]-1)
      {
         if (ch->pcdata->ranking[sn] >= skill_table[csn]->masterydiff[0])
            cnt++;
      }
      //Can now learn from that tier if you have something novice or better in it
      if (skill_table[sn]->group[0] == skill_table[csn]->group[0]
      &&  skill_table[sn]->stype == skill_table[csn]->stype
      &&  skill_table[sn]->masterydiff[0] >= skill_table[csn]->masterydiff[0]
      &&  !IS_NPC(ch) && ch->pcdata->ranking[sn] >= 2)
      {
         return TRUE;
      }
  }
  if (cnt >= mlearn)
     return TRUE;
  else
     return FALSE;
}

// 1 Tier 4   1 Tier 3      3 Tier 2        OR
// 0 Tier 3   3 Tier 3      2 Tier 3
int check_sphere_mastery(int sn, CHAR_DATA *ch)
{
   int tier3[MAX_SPHERE+1];
   int tier4[MAX_SPHERE+1];
   int t3cnt = 0;
   int t4cnt = 0;
   int x;
   int ssn;
   
   if (skill_table[sn]->masterydiff[0] <= 2)
      return FALSE;
   for (ssn = 0; ssn < top_sn && skill_table[ssn] && skill_table[ssn]->name; ssn++)
   {
      if (ch->pcdata->ranking[ssn] > 0)
      {
         if (skill_table[ssn]->masterydiff[0] == 3)
            tier3[skill_table[ssn]->stype] = 1;        
            
         if (skill_table[ssn]->masterydiff[0] == 4)
         {
            tier4[skill_table[ssn]->stype] = 1;  
            tier3[skill_table[ssn]->stype] = 1;  
         }
      }
   }
   for (x = 1; x <= MAX_SPHERE; x++)
   {
     if (tier3[x] == 1)
        t3cnt++;
     if (tier4[x] == 1)
        t4cnt++;
   }
   if (t4cnt == 1 && skill_table[sn]->masterydiff[0] == 3 && t3cnt <= 1)
      return FALSE;
   if (t4cnt == 0 && skill_table[sn]->masterydiff[0] == 3 && t3cnt <= 2)
      return FALSE;
   if (skill_table[sn]->masterydiff[0] == 3 && t3cnt >= 4)
      return TRUE;
   else if (skill_table[sn]->masterydiff[0] == 3)
   {
      for (x = 1; x <= MAX_SPHERE; x++)
      {
         if (tier3[x] == 1)
         {
            if (x == skill_table[sn]->stype)
               return FALSE;
         }
      }
      return TRUE;
   }   
   if (skill_table[sn]->masterydiff[0] == 4 && t3cnt >= 3)
      return TRUE;
   if (skill_table[sn]->masterydiff[0] == 4 && t4cnt <= 0)  
      return FALSE;
      
   else if (skill_table[sn]->masterydiff[0] == 4)
   {
      for (x = 1; x <= MAX_SPHERE; x++)
      {
         if (tier4[x] == 1)
         {
            if (x == skill_table[sn]->stype)
               return FALSE;
         }
      }
      return TRUE;    
   }
   return FALSE;
}
      

// Used to increase mastery now instead of mobprogs
void do_learn(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TRAINER_DATA *tdata;
   BUYKTRAINER_DATA *ktrainer = NULL;
   OBJ_DATA *spellbook;
   OBJ_INDEX_DATA *sbook;
   char arg1[MIL];
   char arg2[MIL];
   char buf1[MSL];
   char buf2[MSL];
   char logb[MSL];
   TOWN_DATA *town;
   int cost, x, y, z, a;
   int fnd = 0;
   int mastery;
   int kv;

   a = z = -1;
   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobiles.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: learn <mobile> [skill/group] [mastery]\n\r", ch);
      send_to_char("learn with only the mobile argument will list available skills/groups.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("The target is not in this room.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Only works on NPCs.\n\r", ch);
      return;
   }
   //Find the trainer
   for (tdata = first_trainer; tdata; tdata = tdata->next)
   {
      if (tdata->vnum == victim->pIndexData->vnum)
      {
         break; //Trainer found, now break;
      }
   }
   if (!tdata)
   {
      for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
      {
         if (victim->pIndexData->vnum == MOB_VNUM_TRAINER && victim->m2 == ktrainer->pid)
            break;
      }
   }
   if (tdata == NULL && ktrainer == NULL)
   {
      send_to_char("&GThat mobile is not a trainer, trainers have &G&W[TRAINER}&G before their name.\n\r", ch);
      return;
   }
   if (arg2[0] == '\0') //Provided mob only
   {
      sh_int used[MAX_SKILL];
      int ccnt = 0;
      char costbuf[MSL];
      char mbuf[MSL];

      //for error checking
      for (x = 0; x <= MAX_SKILL; x++)
      {
         used[x] = 0;
      }
      
      for (y = MAX_RANKING; y > 0; y--)
      {
         for (x = 0; x < 20; x++)
         {
            ccnt = 0;
            if (tdata)
            {
               if (tdata->mastery[x] == y)
               {
                  if (tdata->mastery[x] < used[tdata->sn[x]])
                  {
                     bug("Trainer %d has two or more occurances of sn %d", victim->pIndexData->vnum, tdata->sn[x]);
                  }
                  if (skill_table[tdata->sn[x]]->group[0] == -1)
                  {
                     bug("Trainer %d is trying to teach a -1 group skill/spell of sn %d", victim->pIndexData->vnum, tdata->sn[x]);
                     continue;
                  }
                  strcpy(costbuf, "");
                  sprintf(mbuf, "&c&wSkill               Cost:  ");
                  for (ccnt = 0; ccnt < tdata->mastery[x]; ccnt++)
                  {
                     if (ccnt == 0)
                     {
                        strcat(mbuf, "Begi  ");
                        sprintf(buf1, "%-4d  ", 500);    
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 1)
                     {
                        strcat(mbuf, "Novice ");
                        sprintf(buf1, "%-5d  ", 3000);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 2)
                     {
                        strcat(mbuf, "Expert  ");
                        sprintf(buf1, "%-6d  ", 20000);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 3)
                     {
                        strcat(mbuf, "Master   ");
                        sprintf(buf1, "%-7d  ", 100000);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 4)
                     {
                        strcat(mbuf, "Elite    ");
                        sprintf(buf1, "%-7d  ", 500000);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 5)
                     {
                        strcat(mbuf, "Flawless");
                        sprintf(buf1, "%-8d", 2000000);      
                        strcat(costbuf, buf1);  
                     }
                  }
                  if (fnd == 0)
                     ch_printf(ch, "%s\n\r", mbuf);
                  ch_printf(ch, "" MXPFTAG("learn '%s' '%s'", "&w&c%s", "/learn") "%s  &w&c%s\n\r",
                     victim->name, skill_table[tdata->sn[x]]->name, skill_table[tdata->sn[x]]->name, 
                     add_wspace(strlen(skill_table[tdata->sn[x]]->name), 25), costbuf);
                  fnd++;
                  used[tdata->sn[x]] = tdata->mastery[x];
               }
            }
            else
            {
               if (ktrainer->mastery[x] == y)
               {
                  if (ktrainer->mastery[x] < used[ktrainer->sn[x]])
                  {
                     bug("Trainer %d has two or more occurances of sn %d", victim->pIndexData->vnum, ktrainer->sn[x]);
                  }
                  if (skill_table[ktrainer->sn[x]]->group[0] == -1)
                  {
                     bug("Trainer %d is trying to teach a -1 group skill/spell of sn %d", victim->pIndexData->vnum, ktrainer->sn[x]);
                     continue;
                  }
                                                         
                  strcpy(costbuf, "");
                  sprintf(mbuf, "&c&wSkill               Cost:  ");
                  for (ccnt = 0; ccnt < ktrainer->mastery[x]; ccnt++)
                  {
                     if (ccnt == 0)
                        cost = 500;
                     else if (ccnt == 1)
                        cost = 3000;
                     else if (ccnt == 2)
                        cost = 20000;
                     else if (ccnt == 3)
                        cost = 100000;
                     else if (ccnt == 4)
                        cost = 500000;
                     else
                        cost = 2000000;
                     kv = kingdom_sector[ch->map][ch->coord->x][ch->coord->y];
                     if (kv > 1 && kv < sysdata.max_kingdom)
                     {
                        if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 1) //Tier 1 skill/spell
                        {
                           if (ch->pcdata->hometown == kv
                           &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                           {             
                              cost = cost * kingdom_table[kv]->tier1 / 100;
                           }
                           if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                              cost = cost * kingdom_table[kv]->tier1 / 100 * kingdom_table[kv]->tvisitor / 100;
                        }
                        if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 2) //Tier 2 skill/spell
                        {
                           if (ch->pcdata->hometown == kv
                           &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                           {             
                              cost = cost * kingdom_table[kv]->tier2 / 100;
                           }
                           if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                              cost = cost * kingdom_table[kv]->tier2 / 100 * kingdom_table[kv]->tvisitor / 100;
                        }
                        if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 3) //Tier 3 skill/spell
                        {
                           if (ch->pcdata->hometown == kv
                           &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                           {             
                              cost = cost * kingdom_table[kv]->tier3 / 100;
                           }
                           if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                             cost = cost * kingdom_table[kv]->tier3 / 100 * kingdom_table[kv]->tvisitor / 100;
                        }
                        if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 4) //Tier 4 skill/spell
                        {
                           if (ch->pcdata->hometown == kv
                           &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                           {             
                              cost = cost * kingdom_table[kv]->tier4 / 100;
                           }
                           if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                              cost = cost * kingdom_table[kv]->tier4 / 100 * kingdom_table[kv]->tvisitor / 100;
                        }
                     }
                     if (ccnt == 0)
                     {
                        strcat(mbuf, "Begi  ");
                        sprintf(buf1, "%-4d  ", cost);    
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 1)
                     {
                        strcat(mbuf, "Novice ");
                        sprintf(buf1, "%-5d  ", cost);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 2)
                     {
                        strcat(mbuf, "Expert  ");
                        sprintf(buf1, "%-6d  ", cost);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 3)
                     {
                        strcat(mbuf, "Master   ");
                        sprintf(buf1, "%-7d  ", cost);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 4)
                     {
                        strcat(mbuf, "Elite    ");
                        sprintf(buf1, "%-7d  ", cost);      
                        strcat(costbuf, buf1);  
                     }
                     if (ccnt == 5)
                     {
                        strcat(mbuf, "Flawless");
                        sprintf(buf1, "%-8d", cost);      
                        strcat(costbuf, buf1);  
                     }
                  }
                  if (fnd == 0)
                     ch_printf(ch, "%s\n\r", mbuf);
                  ch_printf(ch, "" MXPFTAG("learn '%s' '%s'", "&w&c%s", "/learn") "%s  &w&c%s\n\r",
                     victim->name, skill_table[ktrainer->sn[x]]->name, skill_table[ktrainer->sn[x]]->name, 
                     add_wspace(strlen(skill_table[ktrainer->sn[x]]->name), 25), costbuf);
                  fnd++;
                  used[ktrainer->sn[x]] = ktrainer->mastery[x];
               }
            }
         }
      }
      if (fnd == 0)
      {
         send_to_char("This trainer has nothing that you can learn.\n\r", ch);
         return;
      }
      return;
   }
   else //Time to learn something
   {
      int atnum = -1; //number of tdata array
      int masterym = 0; //Highest mastery teach teaches

      if (argument[0] == '\0')
      {
         do_learn(ch, "");
         return;
      }
      for (x = 0; x < 20; x++)
      {
         if (tdata)
         {
            if (skill_table[tdata->sn[x]]->group[0] == -1)
            {
               bug("Trainer %d is trying to teach a -1 group skill/spell of sn %d", victim->pIndexData->vnum, tdata->sn[x]);
               continue;
            }
            if (tdata->sn[x] > 0 && tdata->mastery[x] > 0)
            {
               if ((!str_prefix(skill_table[tdata->sn[x]]->name, arg2)) || (tdata->sn[x] == atoi(arg2)))
               {
                  if (masterym != 0)
                  {
                      bug("Trainer %d has two or more occurances of sn %d", victim->pIndexData->vnum, tdata->sn[x]);
                  }
                  if (tdata->mastery[x] > masterym)
                  {
                     masterym = tdata->mastery[x];
                     atnum = x;
                  }
               }
            }
         }
         else
         {
            if (skill_table[ktrainer->sn[x]]->group[0] == -1)
            {
               bug("Trainer %d is trying to teach a -1 group skill/spell of sn %d", victim->pIndexData->vnum, ktrainer->sn[x]);
               continue;
            }
            if (ktrainer->sn[x] > 0 && ktrainer->mastery[x] > 0)
            {
               if ((!str_prefix(skill_table[ktrainer->sn[x]]->name, arg2)) || (ktrainer->sn[x] == atoi(arg2)))
               {
                  if (masterym != 0)
                  {
                      bug("Trainer %d has two or more occurances of sn %d", victim->pIndexData->vnum, ktrainer->sn[x]);
                  }
                  if (ktrainer->mastery[x] > masterym)
                  {
                     masterym = ktrainer->mastery[x];
                     atnum = x;
                  }
               }
            }
         }
                
      }
      if (atnum >= 0)
         x = atnum;
      if (x == 20)
      {
         send_to_char("You cannot learn that, make sure you spelled it right.\n\r", ch);
         return;
      }
      //A skill or group was found, now time to check a few things :-)
      if (isdigit(argument[0]))
         mastery = atoi(argument);
      else
         mastery = get_mastery_num(argument);
      if (mastery <= 0)
      {
         send_to_char("That mastery is invalid, make sure you spelled it right.\n\r", ch);
         return;
      }
      if (tdata)
      {
         if (tdata->sn[x] > 0)
         {
            int sn;

            sn = tdata->sn[x];
            if ((ch->pcdata->ranking[sn] != mastery - 1))
            {
               if (mastery == 1)
               {
                  ch_printf(ch, "You can only learn Beginner if you don't know the Skill.\n\r");
                  return;
               }
               if (ch->pcdata->ranking[sn] >= mastery)
               {
                  ch_printf(ch, "You are already meeting or exceeding this mastery.\n\r", ch);
                  return;
               }
               ch_printf(ch, "In order to advance to %s you need to be possess the %s mastery \n\r", get_mastery_name(mastery), get_mastery_name(mastery - 1));
               return;
            }
            if (mastery == 2 && ch->pcdata->learned[sn] < 4)
            {
               ch_printf(ch, "In order to advance to %s you need 4 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 3 && ch->pcdata->learned[sn] < 7)
            {
               ch_printf(ch, "In order to advance to %s you need 7 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 4 && ch->pcdata->learned[sn] < 10)
            {
               ch_printf(ch, "In order to advance to %s you need 10 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 5 && ch->pcdata->learned[sn] < 14)
            {
               ch_printf(ch, "In order to advance to %s you need 14 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 6 && ch->pcdata->learned[sn] < 17)
            {
               ch_printf(ch, "In order to advance to %s you need 17 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 1 && !can_learn_skill(tdata->sn[x], ch))
            {
               ch_printf(ch, "You need %d %s skills/spells at %s to learn %s skills/spells (help skillrequirements for more info).\n\r", min_to_learn(tdata->sn[x]), get_tier_name(skill_table[sn]->masterydiff[0]-1), get_mastery_name(skill_table[sn]->masterydiff[0]), get_tier_name(skill_table[sn]->masterydiff[0]));
               return;
            }
            if (mastery == 1 && check_sphere_mastery(tdata->sn[x], ch))
            {
               ch_printf(ch, "You cannot learn this due to mastery restrictions (HELP MASTERY RESTRICTIONS).\n\r", ch);
               return;
            }
            if (mastery > masterym)
            {
               ch_printf(ch, "This trainer only teaches mastery %s and lower.\n\r", get_mastery_name(masterym));
               return;
            }
            if (mastery == 1)
               cost = 500;
            else if (mastery == 2)
               cost = 3000;
            else if (mastery == 3)
               cost = 20000;
            else if (mastery == 4)
               cost = 100000;
            else if (mastery == 5)
               cost = 500000;
            else
               cost = 2000000;
               
               
            if (ch->gold < cost)
            {
               sprintf(buf1, "You need %d gold to advance to %s.\n\r", cost, get_mastery_name(mastery));
               send_to_char(buf1, ch);
               return;
            }
            if (skill_table[tdata->sn[x]]->type == SKILL_SPELL && mastery == 1)
            {
               if ((sbook = get_obj_index(skill_table[sn]->bookv)) == NULL)
               {
                  bug("Spell %d does not have a spellbook.", tdata->sn[x]);
                  send_to_char("You do not have the spellbook for this spell.\n\r", ch);
                  return;
               }
               if ((spellbook = get_obj_carry(ch, sbook->name)) == NULL)
               {
                  ch_printf(ch, "You need (-%s-) to learn that spell.\n\r", sbook->short_descr);
                  return;
               }
               separate_obj(spellbook);
               obj_from_char(spellbook);
               act(AT_ACTION, "$n reads from $p to learn a new spell.", ch, spellbook, NULL, TO_ROOM);
               act(AT_ACTION, "You read from $p to learn a new spell.", ch, spellbook, NULL, TO_CHAR);
               extract_obj(spellbook);
            }
            ch->gold = ch->gold - cost;
            ch->pcdata->ranking[sn] = mastery;
            if (ch->pcdata->learned[sn] == 0)
            {
               ch->pcdata->learned[sn] = 1;
               ch->pcdata->spercent[sn] = 200;
               if (ch->pcdata->spherepoints[1] == -1 || ch->pcdata->grouppoints[1] == -1)
               {
                  if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
                  {
                     ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP]++;
                     ch->pcdata->spherepoints[skill_table[sn]->stype]++;
                  }
                  else
                  {
                    ch->pcdata->grouppoints[skill_table[sn]->group[0]]++;
                     ch->pcdata->spherepoints[skill_table[sn]->stype]++;
                  }
               }
            }
            if (ch->pcdata->ranking[sn] == 1)
               sprintf(buf2, "Beginner");
            if (ch->pcdata->ranking[sn] == 2)
               sprintf(buf2, "Novice");
            if (ch->pcdata->ranking[sn] == 3)
               sprintf(buf2, "Expert");
            if (ch->pcdata->ranking[sn] == 4)
               sprintf(buf2, "Master");
            if (ch->pcdata->ranking[sn] == 5)
               sprintf(buf2, "Elite");
            if (ch->pcdata->ranking[sn] == 6)
               sprintf(buf2, "Flawless");

            sprintf(buf1, "You advanced to %s in %s\n\r", buf2, skill_table[sn]->name);
            send_to_char(buf1, ch);
            return;
         }
      }
      else
      {
         int taxes;
         if (ktrainer->sn[x] > 0)
         {
            int sn;

            sn = ktrainer->sn[x];
            if ((ch->pcdata->ranking[sn] != mastery - 1))
            {
               if (mastery == 1)
               {
                  ch_printf(ch, "You can only learn Beginner if you don't know the Skill.\n\r");
                  return;
               }
               if (ch->pcdata->ranking[sn] >= mastery)
               {
                  ch_printf(ch, "You are already meeting or exceeding this mastery.\n\r", ch);
                  return;
               }
               ch_printf(ch, "In order to advance to %s you need to be possess the %s mastery \n\r", get_mastery_name(mastery), get_mastery_name(mastery - 1));
               return;
            }
            if (mastery == 2 && ch->pcdata->learned[sn] < 4)
            {
               ch_printf(ch, "In order to advance to %s you need 4 points.\n\r", get_mastery_name(mastery));
              return;
            }
            if (mastery == 3 && ch->pcdata->learned[sn] < 7)
            {
               ch_printf(ch, "In order to advance to %s you need 7 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 4 && ch->pcdata->learned[sn] < 10)
            {
               ch_printf(ch, "In order to advance to %s you need 10 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 5 && ch->pcdata->learned[sn] < 14)
            {
               ch_printf(ch, "In order to advance to %s you need 14 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 6 && ch->pcdata->learned[sn] < 17)
            {
               ch_printf(ch, "In order to advance to %s you need 17 points.\n\r", get_mastery_name(mastery));
               return;
            }
            if (mastery == 1 && !can_learn_skill(ktrainer->sn[x], ch))
            {
               ch_printf(ch, "You need (-%d-) %s skills/spells at %s to learn %s skills/spells (help skillrequirements for more info).\n\r", min_to_learn(ktrainer->sn[x]), get_tier_name(skill_table[sn]->masterydiff[0]-1), get_mastery_name(skill_table[sn]->masterydiff[0]), get_tier_name(skill_table[sn]->masterydiff[0]));
               return;
            }
            if (mastery > masterym)
            {
               ch_printf(ch, "This trainer only teaches mastery %s and lower.\n\r", get_mastery_name(masterym));
               return;
            }
            if (mastery == 1)
               cost = 500;
            else if (mastery == 2)
               cost = 3000;
            else if (mastery == 3)
               cost = 20000;
            else if (mastery == 4)
               cost = 100000;
            else if (mastery == 5)
               cost = 500000;
            else
               cost = 2000000;
               
               
            kv = kingdom_sector[ch->map][ch->coord->x][ch->coord->y];
            if (kv > 1 && kv < sysdata.max_kingdom)
            {
               if (kingdom_table[kv]->tvisitor < 100)
                  kingdom_table[kv]->tvisitor = 100;
               if (kingdom_table[kv]->tier1 < 100)
                  kingdom_table[kv]->tier1 = 100;
               if (kingdom_table[kv]->tier2 < 100)
                  kingdom_table[kv]->tier2 = 100;
               if (kingdom_table[kv]->tier3 < 100)
                  kingdom_table[kv]->tier3 = 100;
               if (kingdom_table[kv]->tier4 < 100)
                  kingdom_table[kv]->tier4 = 100;
               if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 1) //Tier 1 skill/spell
               {
                  if (ch->pcdata->hometown == kv
                  &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                  {             
                     cost = cost * kingdom_table[kv]->tier1 / 100;
                  }
                  if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                     cost = cost * kingdom_table[kv]->tier1 / 100 * kingdom_table[kv]->tvisitor / 100;
               }
               if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 2) //Tier 2 skill/spell
               {
                  if (ch->pcdata->hometown == kv
                  &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                  {             
                     cost = cost * kingdom_table[kv]->tier2 / 100;
                  }
                  if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                     cost = cost * kingdom_table[kv]->tier2 / 100 * kingdom_table[kv]->tvisitor / 100;
               }
               if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 3) //Tier 3 skill/spell
               {
                  if (ch->pcdata->hometown == kv
                  &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                  {             
                     cost = cost * kingdom_table[kv]->tier3 / 100;
                  }
                  if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                    cost = cost * kingdom_table[kv]->tier3 / 100 * kingdom_table[kv]->tvisitor / 100;
               }
               if (skill_table[ktrainer->sn[x]]->masterydiff[0] == 4) //Tier 4 skill/spell
               {
                  if (ch->pcdata->hometown == kv
                  &&  ch->pcdata->caste < kingdom_table[kv]->mintrainertax)
                  {             
                     cost = cost * kingdom_table[kv]->tier4 / 100;
                  }
                  if (kv != ch->pcdata->hometown && kingdom_table[kv]->tvisitor > 99)
                     cost = cost * kingdom_table[kv]->tier4 / 100 * kingdom_table[kv]->tvisitor / 100;
               }
            }
            if (ch->gold < cost)
            {
               sprintf(buf1, "You need %d gold to advance to %s.\n\r", cost, get_mastery_name(mastery));
               send_to_char(buf1, ch);
               return;
            }
            if (skill_table[ktrainer->sn[x]]->type == SKILL_SPELL && mastery == 1)
            {
               if ((sbook = get_obj_index(skill_table[sn]->bookv)) == NULL)
               {
                  bug("Spell %d does not have a spellbook.", tdata->sn[x]);
                  send_to_char("You do not have the spellbook for this spell.\n\r", ch);
                  return;
               }
               if ((spellbook = get_obj_carry(ch, sbook->name)) == NULL)
               {
                  ch_printf(ch, "You need %s to learn that spell.\n\r", sbook->short_descr);
                  return;
               }
               separate_obj(spellbook);
               obj_from_char(spellbook);
               act(AT_ACTION, "$n reads from $p to learn a new spell.", ch, spellbook, NULL, TO_ROOM);
               act(AT_ACTION, "You read from $p to learn a new spell.", ch, spellbook, NULL, TO_CHAR);
               extract_obj(spellbook);
            }
            ch->gold = ch->gold - cost;
            if (mastery == 1)
               taxes = cost - 500;
            else if (mastery == 2)
               taxes = cost - 3000;
            else if (mastery == 3)
               taxes = cost - 20000;
            else if (mastery == 4)
               taxes = cost - 100000;
            else if (mastery == 5)
               taxes = cost - 500000;
            else
               taxes = cost - 2000000;
               
            if ((town = find_town(ch->coord->x, ch->coord->y, ch->map)) != NULL)
            {  
               town->coins += taxes;
            }            
            if (taxes > 0 && town)
            {
               sprintf(logb, "%s paid %d to use Trainer %s to learn %s", PERS_KINGDOM(ch, ch->in_room->area->kingdom), taxes, ktrainer->name, skill_table[sn]->name);
               write_kingdom_logfile(town->kingdom, logb, KLOG_TRAINERTAX);  
            }
            ch->pcdata->ranking[sn] = mastery;
            if (ch->pcdata->learned[sn] == 0)
            {
               ch->pcdata->learned[sn] = 1;
               ch->pcdata->spercent[sn] = 200;
               if (ch->pcdata->spherepoints[1] == -1 || ch->pcdata->grouppoints[1] == -1)
               {
                  if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
                  {
                     ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP]++;
                     ch->pcdata->spherepoints[skill_table[sn]->stype]++;
                  }
                  else
                  {
                    ch->pcdata->grouppoints[skill_table[sn]->group[0]]++;
                     ch->pcdata->spherepoints[skill_table[sn]->stype]++;
                  }
               }
            }
            if (ch->pcdata->ranking[sn] == 1)
               sprintf(buf2, "Beginner");
            if (ch->pcdata->ranking[sn] == 2)
               sprintf(buf2, "Novice");
            if (ch->pcdata->ranking[sn] == 3)
               sprintf(buf2, "Expert");
            if (ch->pcdata->ranking[sn] == 4)
               sprintf(buf2, "Master");
            if (ch->pcdata->ranking[sn] == 5)
               sprintf(buf2, "Elite");
            if (ch->pcdata->ranking[sn] == 6)
               sprintf(buf2, "Flawless");

            sprintf(buf1, "You advanced to %s in %s, which costs you %d\n\r", buf2, skill_table[sn]->name, cost);
            send_to_char(buf1, ch);
            return;
         }
      }
   }
   do_learn(ch, "");
}

void do_whonumber(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   int wnum;

   set_char_color(AT_YELLOW, ch);
   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Your choices are....\n\r", ch);
      send_to_char("1 - Infomation (For 800x600 screens or better)\n\r", ch);
      send_to_char("2 - General Info (For 640x480 Screens)\n\r", ch);
      send_to_char("3 - Clan Info (Recommended 800x600 or better)\n\r", ch);
      send_to_char("4 - Caste/Hometown/Job info (For 640x480 Screens)\n\r", ch);
      return;
   }
   wnum = atoi(arg);

   if ((wnum < 1) || (wnum > 4))
   {
      send_to_char("Your choices are....\n\r", ch);
      send_to_char("1 - Infomation (For 800x600 screens or better)\n\r", ch);
      send_to_char("2 - General Info (For 640x480 Screens)\n\r", ch);
      send_to_char("3 - Clan Info (Recommended 800x600 or better)\n\r", ch);
      send_to_char("4 - Caste/Hometown/Job info (For 640x480 Screens)\n\r", ch);
      return;
   }
   ch->pcdata->whonum = 1;
   ch_printf(ch, "Whonumber set to 1. (Whonumber has been disabled!)\n\r", wnum);
   return;
}

void do_wimpy(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   int wimpy;

   set_char_color(AT_YELLOW, ch);
   one_argument(argument, arg);
   if (!str_cmp(arg, "max"))
   {
      wimpy = (int) ch->max_hit / 1.8;
   }
   else if (arg[0] == '\0')
      wimpy = (int) ch->max_hit / 5;
   else
      wimpy = atoi(arg);

   if (wimpy < 0)
   {
      send_to_char("Your courage exceeds your wisdom.\n\r", ch);
      return;
   }
   else if (wimpy > (int) ch->max_hit / 1.8)
   {
      send_to_char("Such cowardice ill becomes you.\n\r", ch);
      return;
   }
   ch->wimpy = wimpy;
   ch_printf(ch, "Wimpy set to %d hit points.\n\r", wimpy);
   return;
}

void do_password(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char *pArg;
   char *pwdnew;
   char *p;
   char cEnd;

   if (IS_NPC(ch))
      return;

   /*
    * Can't use one_argument here because it smashes case.
    * So we just steal all its code.  Bleagh.
    */
   pArg = arg1;
   while (isspace(*argument))
      argument++;

   cEnd = ' ';
   if (*argument == '\'' || *argument == '"')
      cEnd = *argument++;

   while (*argument != '\0')
   {
      if (*argument == cEnd)
      {
         argument++;
         break;
      }
      *pArg++ = *argument++;
   }
   *pArg = '\0';

   pArg = arg2;
   while (isspace(*argument))
      argument++;

   cEnd = ' ';
   if (*argument == '\'' || *argument == '"')
      cEnd = *argument++;

   while (*argument != '\0')
   {
      if (*argument == cEnd)
      {
         argument++;
         break;
      }
      *pArg++ = *argument++;
   }
   *pArg = '\0';

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Syntax: password <new> <again>.\n\r", ch);
      return;
   }

/*
    if ( strcmp( crypt( arg1, ch->pcdata->pwd ), ch->pcdata->pwd ) )
    {
	WAIT_STATE( ch, 40 );
	send_to_char( "Wrong password.  Wait 10 seconds.\n\r", ch );
	return;
    }
*/

/* This should stop all the mistyped password problems --Shaddai */
   if (strcmp(arg1, arg2))
   {
      send_to_char("Passwords don't match try again.\n\r", ch);
      return;
   }
   if (strlen(arg2) < 5)
   {
      send_to_char("New password must be at least five characters long.\n\r", ch);
      return;
   }

   /*
    * No tilde allowed because of player file format.
    */
   pwdnew = crypt(arg2, ch->name);
   for (p = pwdnew; *p != '\0'; p++)
   {
      if (*p == '~')
      {
         send_to_char("New password not acceptable, try again.\n\r", ch);
         return;
      }
   }

   DISPOSE(ch->pcdata->pwd);
   ch->pcdata->pwd = str_dup(pwdnew);
   if (IS_SET(sysdata.save_flags, SV_PASSCHG))
      save_char_obj(ch);
   send_to_char("Ok.\n\r", ch);
   return;
}



void do_socials(CHAR_DATA * ch, char *argument)
{
   int iHash;
   int col = 0;
   SOCIALTYPE *social;

   set_pager_color(AT_PLAIN, ch);
   for (iHash = 0; iHash < 27; iHash++)
      for (social = social_index[iHash]; social; social = social->next)
      {
         pager_printf(ch, "%-12s", social->name);
         if (++col % 6 == 0)
            send_to_pager("\n\r", ch);
      }

   if (col % 6 != 0)
      send_to_pager("\n\r", ch);
   return;
}


void do_commands(CHAR_DATA * ch, char *argument)
{
   int col;
   bool found;
   int hash;
   CMDTYPE *command;

   col = 0;
   set_pager_color(AT_PLAIN, ch);
   if (argument[0] == '\0')
   {
      for (hash = 0; hash < 126; hash++)
         for (command = command_hash[hash]; command; command = command->next)
            if (command->level < LEVEL_GUEST && command->level <= get_trust(ch) && (command->name[0] != 'm' && command->name[1] != 'p'))
            {
               pager_printf(ch, "%-16s", command->name);
               if (++col % 6 == 0)
                  send_to_pager("\n\r", ch);
            }
      if (col % 6 != 0)
         send_to_pager("\n\r", ch);
   }
   else
   {
      found = FALSE;
      for (hash = 0; hash < 126; hash++)
         for (command = command_hash[hash]; command; command = command->next)
            if (command->level < LEVEL_GUEST
               && command->level <= get_trust(ch) && !str_prefix(argument, command->name) && (command->name[0] != 'm' && command->name[1] != 'p'))
            {
               pager_printf(ch, "%-16s", command->name);
               found = TRUE;
               if (++col % 6 == 0)
                  send_to_pager("\n\r", ch);
            }

      if (col % 6 != 0)
         send_to_pager("\n\r", ch);
      if (!found)
         ch_printf(ch, "No command found under %s.\n\r", argument);
   }
   return;
}

bool is_player_kingdom(int ht)
{
   if (!str_cmp(kingdom_table[ht]->name, "Rafermand") || !str_cmp(kingdom_table[ht]->name, "NewThalos"))
      return FALSE;
   else
      return TRUE;
}

void do_channels(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   one_argument(argument, arg);

   if (IS_NPC(ch))
      return;

   if (arg[0] == '\0')
   {
      if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_SILENCE))
      {
         set_char_color(AT_GREEN, ch);
         send_to_char("You are silenced.\n\r", ch);
         return;
      }

      /* Channels everyone sees regardless of affiliation --Blodkai */
      send_to_char_color("\n\r &gPublic channels  (severe penalties for abuse)&G:\n\r  ", ch);
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_RACETALK) ? " &G+RACETALK" : " &g-racetalk");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_CHAT) ? " &G+CHAT" : " &g-chat");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_GSOCIAL) ? " &G+GSOCIAL" : " &g-gsocial");
      if (get_trust(ch) > 2 && !NOT_AUTHED(ch))
         ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_AUCTION) ? " &G+AUCTION" : " &g-auction");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_TALKQUEST) ? " &G+QUEST" : " &g-quest");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_WARTALK) ? " &G+WARTALK" : " &g-wartalk");
      if (IS_HERO(ch))
         ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_AVTALK) ? " &G+AVATAR" : " &g-avatar");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_MUSIC) ? " &G+MUSIC" : " &g-music");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_ASK) ? " &G+ASK" : " &g-ask");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_SHOUT) ? " &G+SHOUT" : " &g-shout");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_YELL) ? " &G+YELL" : " &g-yell");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_INFO) ? " &G+INFO" : " &g-info");


      /* For organization channels (orders, clans, guilds, councils) */
      send_to_char_color("\n\r &gPrivate channels (severe penalties for abuse)&G:\n\r ", ch);
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_TELLS) ? " &G+TELLS" : " &g-tells");
      ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_WHISPER) ? " &G+WHISPER" : " &g-whisper");
      if (!IS_NPC(ch) && ch->pcdata->clan)
      {
         if (ch->pcdata->clan->clan_type == CLAN_ORDER)
            send_to_char_color(!IS_SET(ch->deaf, CHANNEL_ORDER) ? " &G+ORDER" : " &g-order", ch);

         else if (ch->pcdata->clan->clan_type == CLAN_GUILD)
            send_to_char_color(!IS_SET(ch->deaf, CHANNEL_GUILD) ? " &G+GUILD" : " &g-guild", ch);
         else
            send_to_char_color(!IS_SET(ch->deaf, CHANNEL_CLAN) ? " &G+CLAN" : " &g-clan", ch);
      }
      if (!IS_NPC(ch) && is_player_kingdom(ch->pcdata->hometown))
      {
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_KINGDOM) ? " &G+KINGDOM" : " &G-kingdom", ch);
      }
      if (IS_IMMORTAL(ch) || (ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council")))
         ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_NEWBIE) ? " &G+NEWBIE" : " &g-newbie");
      if (!IS_NPC(ch) && ch->pcdata->council)
         ch_printf_color(ch, "%s", !IS_SET(ch->deaf, CHANNEL_COUNCIL) ? " &G+COUNCIL" : " &g-council");

      /* Immortal channels */
      if (IS_IMMORTAL(ch))
      {
         send_to_char_color("\n\r &gImmortal Channels&G:\n\r  ", ch);
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_IMMTALK) ? " &G+IMMTALK" : " &g-immtalk", ch);
/*          send_to_char_color( !IS_SET( ch->deaf, CHANNEL_PRAY )       ?
		" &G+PRAY"	:	" &g-pray", ch ); */
         if (get_trust(ch) >= sysdata.muse_level)
            send_to_char_color(!IS_SET(ch->deaf, CHANNEL_HIGHGOD) ? " &G+MUSE" : " &g-muse", ch);
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_MONITOR) ? " &G+MONITOR" : " &g-monitor", ch);
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_AUTH) ? " &G+AUTH" : " &g-auth", ch);
      }
      if (get_trust(ch) >= sysdata.log_level)
      {
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_LOG) ? " &G+LOG" : " &g-log", ch);
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_BUILD) ? " &G+BUILD" : " &g-build", ch);
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_COMM) ? " &G+COMM" : " &g-comm", ch);
         send_to_char_color(!IS_SET(ch->deaf, CHANNEL_WARN) ? " &G+WARN" : " &g-warn", ch);
         if (get_trust(ch) >= sysdata.think_level)
            send_to_char_color(!IS_SET(ch->deaf, CHANNEL_HIGH) ? " &G+HIGH" : " &g-high", ch);
      }
      send_to_char("\n\r", ch);
   }
   else
   {
      bool fClear;
      bool ClearAll;
      int bit;

      bit = 0;
      ClearAll = FALSE;

      if (arg[0] == '+')
         fClear = TRUE;
      else if (arg[0] == '-')
         fClear = FALSE;
      else
      {
         send_to_char("Channels -channel or +channel?\n\r", ch);
         return;
      }

      if (!str_cmp(arg + 1, "auction"))
         bit = CHANNEL_AUCTION;
      else if (!str_cmp(arg + 1, "chat"))
         bit = CHANNEL_CHAT;
      else if (!str_cmp(arg + 1, "clan"))
         bit = CHANNEL_CLAN;
      else if (!str_cmp(arg + 1, "council"))
         bit = CHANNEL_COUNCIL;
      else if (!str_cmp(arg + 1, "gsocial"))
         bit = CHANNEL_GSOCIAL;
      else if (!str_cmp(arg + 1, "guild"))
         bit = CHANNEL_GUILD;
      else if (!str_cmp(arg + 1, "quest"))
         bit = CHANNEL_TALKQUEST;
      else if (!str_cmp(arg + 1, "tells"))
         bit = CHANNEL_TELLS;
      else if (!str_cmp(arg + 1, "immtalk"))
         bit = CHANNEL_IMMTALK;
      else if (!str_cmp(arg + 1, "log"))
         bit = CHANNEL_LOG;
      else if (!str_cmp(arg + 1, "build"))
         bit = CHANNEL_BUILD;
      else if (!str_cmp(arg + 1, "kingdom"))
         bit = CHANNEL_KINGDOM;
      else if (!str_cmp(arg + 1, "high"))
         bit = CHANNEL_HIGH;
      else if (!str_cmp(arg + 1, "pray"))
         bit = CHANNEL_PRAY;
      else if (!str_cmp(arg + 1, "avatar"))
         bit = CHANNEL_AVTALK;
      else if (!str_cmp(arg + 1, "monitor"))
         bit = CHANNEL_MONITOR;
      else if (!str_cmp(arg + 1, "auth"))
         bit = CHANNEL_AUTH;
      else if (!str_cmp(arg + 1, "newbie"))
         bit = CHANNEL_NEWBIE;
      else if (!str_cmp(arg + 1, "music"))
         bit = CHANNEL_MUSIC;
      else if (!str_cmp(arg + 1, "muse"))
         bit = CHANNEL_HIGHGOD;
      else if (!str_cmp(arg + 1, "ask"))
         bit = CHANNEL_ASK;
      else if (!str_cmp(arg + 1, "shout"))
         bit = CHANNEL_SHOUT;
      else if (!str_cmp(arg + 1, "yell"))
         bit = CHANNEL_YELL;
      else if (!str_cmp(arg + 1, "comm"))
         bit = CHANNEL_COMM;
      else if (!str_cmp(arg + 1, "warn"))
         bit = CHANNEL_WARN;
      else if (!str_cmp(arg + 1, "order"))
         bit = CHANNEL_ORDER;
      else if (!str_cmp(arg + 1, "wartalk"))
         bit = CHANNEL_WARTALK;
      else if (!str_cmp(arg + 1, "whisper"))
         bit = CHANNEL_WHISPER;
      else if (!str_cmp(arg + 1, "racetalk"))
         bit = CHANNEL_RACETALK;
      else if (!str_cmp(arg + 1, "info"))
         bit = CHANNEL_INFO;
      else if (!str_cmp(arg + 1, "all"))
         ClearAll = TRUE;
      else
      {
         send_to_char("Set or clear which channel?\n\r", ch);
         return;
      }

      if ((fClear) && (ClearAll))
      {
         REMOVE_BIT(ch->deaf, CHANNEL_RACETALK);
         REMOVE_BIT(ch->deaf, CHANNEL_AUCTION);
         REMOVE_BIT(ch->deaf, CHANNEL_CHAT);
         REMOVE_BIT(ch->deaf, CHANNEL_GSOCIAL);
         REMOVE_BIT(ch->deaf, CHANNEL_TALKQUEST);
         REMOVE_BIT(ch->deaf, CHANNEL_WARTALK);
         REMOVE_BIT(ch->deaf, CHANNEL_PRAY);
         REMOVE_BIT(ch->deaf, CHANNEL_MUSIC);
         REMOVE_BIT(ch->deaf, CHANNEL_ASK);
         REMOVE_BIT(ch->deaf, CHANNEL_SHOUT);
         REMOVE_BIT(ch->deaf, CHANNEL_YELL);
         REMOVE_BIT(ch->deaf, CHANNEL_INFO);

         /*     if (ch->pcdata->clan)
            REMOVE_BIT (ch->deaf, CHANNEL_CLAN);

            if (ch->pcdata->council)
            REMOVE_BIT (ch->deaf, CHANNEL_COUNCIL);

            if (ch->pcdata->guild)
            REMOVE_BIT (ch->deaf, CHANNEL_GUILD);
          */
         if (is_player_kingdom(ch->pcdata->hometown))
            REMOVE_BIT(ch->deaf, CHANNEL_KINGDOM);

         if (ch->level >= LEVEL_IMMORTAL)
            REMOVE_BIT(ch->deaf, CHANNEL_AVTALK);

         if (ch->level >= sysdata.log_level)
            REMOVE_BIT(ch->deaf, CHANNEL_COMM);

      }
      else if ((!fClear) && (ClearAll))
      {
         SET_BIT(ch->deaf, CHANNEL_RACETALK);
         SET_BIT(ch->deaf, CHANNEL_AUCTION);
         SET_BIT(ch->deaf, CHANNEL_CHAT);
         SET_BIT(ch->deaf, CHANNEL_GSOCIAL);
         SET_BIT(ch->deaf, CHANNEL_TALKQUEST);
         SET_BIT(ch->deaf, CHANNEL_PRAY);
         SET_BIT(ch->deaf, CHANNEL_MUSIC);
         SET_BIT(ch->deaf, CHANNEL_ASK);
         SET_BIT(ch->deaf, CHANNEL_SHOUT);
         SET_BIT(ch->deaf, CHANNEL_WARTALK);
         SET_BIT(ch->deaf, CHANNEL_YELL);
         SET_BIT(ch->deaf, CHANNEL_INFO);

         if (is_player_kingdom(ch->pcdata->hometown))
            SET_BIT(ch->deaf, CHANNEL_KINGDOM);

         /*     if (ch->pcdata->clan)
            SET_BIT (ch->deaf, CHANNEL_CLAN);

            if (ch->pcdata->council)
            SET_BIT (ch->deaf, CHANNEL_COUNCIL);

            if ( IS_GUILDED(ch) )
            SET_BIT (ch->deaf, CHANNEL_GUILD);
          */
         if (ch->level >= LEVEL_IMMORTAL)
            SET_BIT(ch->deaf, CHANNEL_AVTALK);

         if (ch->level >= sysdata.log_level)
            SET_BIT(ch->deaf, CHANNEL_COMM);

      }
      else if (fClear)
      {
         REMOVE_BIT(ch->deaf, bit);
      }
      else
      {
         SET_BIT(ch->deaf, bit);
      }

      send_to_char("Ok.\n\r", ch);
   }

   return;
}


/*
 * display WIZLIST file						-Thoric
 */
void do_wizlist(CHAR_DATA * ch, char *argument)
{
   set_pager_color(AT_IMMORT, ch);
   show_file(ch, WIZLIST_FILE);
}

/*
 * Contributed by Grodyn.
 * Display completely overhauled, 2/97 -- Blodkai
 */
void do_config(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   if (IS_NPC(ch))
      return;

   argument = one_argument(argument, arg);

   set_char_color(AT_GREEN, ch);

   if (arg[0] == '\0')
   {
      set_char_color(AT_DGREEN, ch);
      send_to_char("\n\rConfigurations ", ch);
      set_char_color(AT_GREEN, ch);
      send_to_char("(use 'config +/- <keyword>' to toggle, see 'help config')\n\r\n\r", ch);
      set_char_color(AT_DGREEN, ch);
      send_to_char("Display:   ", ch);
      set_char_color(AT_GREY, ch);
      ch_printf(ch, "%-12s   %-12s   %-12s   %-12s\n\r           %-12s   %-12s   %-12s   %-12s\n\r           %-12s   %-12s  %-12s   %-12s",
         IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? "[+] PAGER"
         : "[-] pager",
         IS_SET(ch->pcdata->flags, PCFLAG_GAG) ? "[+] GAG"
         : "[-] gag",
         xIS_SET(ch->act, PLR_BRIEF) ? "[+] BRIEF"
         : "[-] brief",
         xIS_SET(ch->act, PLR_COMBINE) ? "[+] COMBINE"
         : "[-] combine",
         xIS_SET(ch->act, PLR_BLANK) ? "[+] BLANK"
         : "[-] blank",
         xIS_SET(ch->act, PLR_PROMPT) ? "[+] PROMPT"
         : "[-] prompt",
         xIS_SET(ch->act, PLR_ANSI) ? "[+] ANSI"
         : "[-] ansi",
         xIS_SET(ch->act, PLR_RIP) ? "[+] RIP"
         : "[-] rip", 
         xIS_SET(ch->act, PLR_POV) ? "[+] PoV" 
         : "[-] pov", 
         xIS_SET(ch->act, PLR_NOWEATHER) ? "[+] NOWEATHER" 
         : "[-] noweather",
         xIS_SET(ch->act, PLR_SHOWPC) ? "[+] SHOWPC"
         : "[-] showpc",
         xIS_SET(ch->act, PLR_NOSIMILIAR) ? "[+] SHORTHELP"
         : "[-] shorthelp");
      set_char_color(AT_DGREEN, ch);
      send_to_char("\n\r\n\rAuto:      ", ch);
      set_char_color(AT_GREY, ch);
      ch_printf(ch, "%-12s   %-12s   %-12s   %-12s",
         xIS_SET(ch->act, PLR_AUTOSAC) ? "[+] AUTOSAC"
         : "[-] autosac",
         xIS_SET(ch->act, PLR_AUTOGOLD) ? "[+] AUTOGOLD"
         : "[-] autogold",
         xIS_SET(ch->act, PLR_AUTOLOOT) ? "[+] AUTOLOOT" : "[-] autoloot", xIS_SET(ch->act, PLR_AUTOEXIT) ? "[+] AUTOEXIT" : "[-] autoexit");
      ch_printf(ch, "\n\r           %-12s  %-12s",
         xIS_SET(ch->act, PLR_AUTOSPLIT) ? "[+] AUTOSPLIT" : "[-] autosplit",
         xIS_SET(ch->act, PLR_QUESTLOOT) ? "[+] QUESTLOOT" : "[-] questloot");
      set_char_color(AT_DGREEN, ch);
      send_to_char("\n\r\n\rSafeties:  ", ch);
      set_char_color(AT_GREY, ch);
      ch_printf(ch, "%-12s   %-12s   %-12s   %-12s\n\r",
         IS_SET(ch->pcdata->flags, PCFLAG_NORECALL) ? "[+] NORECALL"
         : "[-] norecall",
         IS_SET(ch->pcdata->flags, PCFLAG_NOSUMMON) ? "[+] NOSUMMON"
         : "[-] nosummon",
         IS_SET(ch->pcdata->flags, PCFLAG_NOBEEP) ? "[+] NOBEEP"
         : "[-] nobeep", IS_SET(ch->pcdata->flags, PCFLAG_NOFOLLOW) ? "[+] NOFOLLOW" : "[-] nofollow");

      ch_printf(ch, "           %-12s   %-12s", 
         xIS_SET(ch->act, PLR_NICE) ? "[+] NICE" : "[-] nice",
         IS_SET(ch->pcdata->flags, PCFLAG_CNOASSIST) ? "[+] CNOASSIST" : "[-] cnoassist");

      set_char_color(AT_DGREEN, ch);
      send_to_char("\n\r\n\rMisc:      ", ch);
      set_char_color(AT_GREY, ch);
      ch_printf(ch, "%-12s   %-12s   %-12s   %-12s\n\r           %-12s",
         xIS_SET(ch->act, PLR_TELNET_GA) ? "[+] TELNETGA"
         : "[-] telnetga",
         IS_SET(ch->pcdata->flags, PCFLAG_GROUPWHO) ? "[+] GROUPWHO"
         : "[-] groupwho", 
         IS_SET(ch->pcdata->flags, PCFLAG_NOINTRO) ? "[+] NOINTRO" 
         : "[-] nointro",
         xIS_SET(ch->act, PLR_TARGET) ? "[+] AUTO-TARGET"
         : "[-] auto-target",
         xIS_SET(ch->act, PLR_NOTOHIT) ? "[+] NOTOHIT" : "[-] notohit");
         
      set_char_color(AT_DGREEN, ch);
      send_to_char("\n\r\n\rSettings:  ", ch);
      set_char_color(AT_GREY, ch);
      ch_printf_color(ch, "Pager Length (%d)   Wimpy (&G&W%d&c&w)          WhoNumber (&G&W%d&c&w)", ch->pcdata->pagerlen, ch->wimpy, ch->pcdata->whonum);
      set_char_color(AT_DGREEN, ch);
      send_to_char("\n\r\n\rTimeouts:  ", ch);
      set_char_color(AT_GREY, ch);
      ch_printf_color(ch, "Login (&G&W%-5d&c&w)       Notes (&G&W%-5d&c&w)       Idle (&G&W%-5d&c&w) ",
         ch->pcdata->timeout_login/4, ch->pcdata->timeout_notes/4, ch->pcdata->timeout_idle/4);

      if (IS_IMMORTAL(ch))
      {
         set_char_color(AT_DGREEN, ch);
         send_to_char("\n\r\n\rImmortal:  ", ch);
         set_char_color(AT_GREY, ch);
         ch_printf(ch, "%-12s   %-13s  %-13s\n\r           %-12s   %-13s %-13s  %-12s\n\r           %-12s   %-12s   %-12s   %-13s",
            xIS_SET(ch->act, PLR_ROOMVNUM) ?"[+] ROOMVNUM" : "[-] roomvnum",
            xIS_SET(ch->act, PLR_AUTOMAP) ? "[+] AUTOMAP" : "[-] automap",
            xIS_SET(ch->act, PLR_NORIDERS) ? "[+] NORIDERS" : "[-] noriders",
            IS_SET(ch->pcdata->flags, PCFLAG_NOFINGER) ? "[+] NOFINGER" : "[-] nofinger", 
            xIS_SET(ch->act, PLR_NOTRANS) ? "[+] NOTRANSFER" : "[-] notransfer",
            xIS_SET(ch->act, PLR_SHOWASIMM) ? "[+] SHOWASIMM" : "[-] showasimm",
            xIS_SET(ch->act, PLR_SHOWNAMES) ? "[+] SHOWNAMES" : "[-] shownames",
            xIS_SET(ch->act, PLR_UKNOWN) ? "[+] UKNOWN" : "[-] uknown",
 	    xIS_SET(ch->act, PLR_ONDUTY) ? "[+] ONDUTY" : "[-] onduty",
 	    xIS_SET(ch->act, PLR_MMOBILES) ? "[+] MMOBILES" : "[-] mmobiles",
 	    IS_SET(ch->pcdata->flags, PCFLAG_AUTOPROTO) ? "[+] AUTOPROTO" : "[-] autoproto");
      }

      set_char_color(AT_DGREEN, ch);
      send_to_char("\n\r\n\rSentences imposed on you (if any):", ch);
      set_char_color(AT_YELLOW, ch);
      ch_printf(ch, "\n\r%s%s%s%s%s%s",
         xIS_SET(ch->act, PLR_SILENCE) ?
         " For your abuse of channels, you are currently silenced.\n\r" : "",
         xIS_SET(ch->act, PLR_NO_EMOTE) ?
         " The gods have removed your emotes.\n\r" : "",
         xIS_SET(ch->act, PLR_NO_TELL) ?
         " You are not permitted to send 'tells' to others.\n\r" : "",
         xIS_SET(ch->act, PLR_LITTERBUG) ?
         " A convicted litterbug.  You cannot drop anything.\n\r" : "",
         xIS_SET(ch->act, PLR_THIEF) ?
         " A proven thief, you will be hunted by the authorities.\n\r" : "",
         xIS_SET(ch->act, PLR_KILLER) ? " For the crime of murder you are sentenced to death...\n\r" : "");
      send_to_char("\n\rSyntax:  config +(value)  config -(value)  config <idle/notes> <seconds>\n\r", ch);
   }
   else
   {
      bool fSet;
      int bit = 0;

      if (arg[0] == '+')
         fSet = TRUE;
      else if (arg[0] == '-')
         fSet = FALSE;
      else if (!str_cmp(arg, "idle") || !str_cmp(arg, "notes"))
      {
         if (!str_cmp(arg, "idle"))
         {
            if (IS_IMMORTAL(ch))
            {
               if ((atoi(argument) < 60 && atoi(argument) != 0) || atoi(argument) > 10800)
               {
                   send_to_char("Range is 60 seconds to 10800 seconds (3 hours).\n\r", ch);
                   return;
               }
            }
            else
            {
               if ((atoi(argument) < 60 && atoi(argument) != 0) || atoi(argument) > 86400)
               {
                   send_to_char("Range is 60 seconds to 86400 seconds (24 hours) or 0 for infinite.\n\r", ch);
                   return;
               }
            }
            ch->pcdata->timeout_idle = atoi(argument)*4;
            send_to_char("Set.\n\r", ch);
            return;
         }
         if (!str_cmp(arg, "notes"))
         {
            if (IS_IMMORTAL(ch))
            {
               if ((atoi(argument) < 180 && atoi(argument) != 0) || atoi(argument) > 7200)
               {
                   send_to_char("Range is 180 seconds to 7200 seconds (2 hours).\n\r", ch);
                   return;
               }
            }
            else
            {
               if ((atoi(argument) < 180 && atoi(argument) != 0) || atoi(argument) > 43200)
               {
                   send_to_char("Range is 180 seconds to 43200 seconds (12 hours) or 0 for infinite.\n\r", ch);
                   return;
               }
            }
            ch->pcdata->timeout_notes = atoi(argument)*4;
            send_to_char("Set.\n\r", ch);
            return;
         }
         return;
      }
      else
      {
         send_to_char("Config -option or +option?\n\r", ch);
         send_to_char("Config idle <value> or Config notes <value> (For Setting Idle Time)\n\rYou cannot change Login Timeout!\n\r", ch);
         return;
      }

      if (!str_cmp(arg + 1, "autoexit"))
         bit = PLR_AUTOEXIT;
      else if (!str_cmp(arg + 1, "autoloot"))
         bit = PLR_AUTOLOOT;
      else if (!str_cmp(arg + 1, "autosplit"))
         bit = PLR_AUTOSPLIT;
      else if (!str_cmp(arg + 1, "autosac"))
         bit = PLR_AUTOSAC;
      else if (!str_cmp(arg + 1, "autogold"))
         bit = PLR_AUTOGOLD;
      else if (!str_cmp(arg + 1, "questloot"))
         bit = PLR_QUESTLOOT;
      else if (!str_cmp(arg + 1, "blank"))
         bit = PLR_BLANK;
      else if (!str_cmp(arg + 1, "brief"))
         bit = PLR_BRIEF;
      else if (!str_cmp(arg + 1, "combine"))
         bit = PLR_COMBINE;
      else if (!str_cmp(arg + 1, "prompt"))
         bit = PLR_PROMPT;
      else if (!str_cmp(arg + 1, "pov"))
         bit = PLR_POV;
      else if (!str_cmp(arg + 1, "shorthelp"))
         bit = PLR_NOSIMILIAR;
      else if (!str_cmp(arg + 1, "noweather"))
         bit = PLR_NOWEATHER;
      else if (!str_cmp(arg + 1, "telnetga"))
         bit = PLR_TELNET_GA;
      else if (!str_cmp(arg + 1, "notohit"))
         bit = PLR_NOTOHIT;
      else if (!str_cmp(arg + 1, "showpc"))
         bit = PLR_SHOWPC;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "notransfer"))
         bit = PLR_NOTRANS;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "noriders"))
         bit = PLR_NORIDERS;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "showasimm"))
         bit = PLR_SHOWASIMM;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "uknown"))
         bit = PLR_UKNOWN;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "onduty"))
	 bit = PLR_ONDUTY;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "shownames"))
         bit = PLR_SHOWNAMES;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "mmobiles"))
         bit = PLR_MMOBILES;
      else if (!str_cmp(arg + 1, "ansi"))
         bit = PLR_ANSI;
      else if (!str_cmp(arg + 1, "target") || !str_cmp(arg + 1, "auto-target") || !str_cmp(arg + 1, "autotarget"))
         bit = PLR_TARGET;
      else if (!str_cmp(arg + 1, "rip"))
         bit = PLR_RIP;
/*	else if ( !str_prefix( arg+1, "flee"     ) ) bit = PLR_FLEE; */
      else if (!str_cmp(arg + 1, "nice"))
         bit = PLR_NICE;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "vnum"))
         bit = PLR_ROOMVNUM;
      else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "map"))
         bit = PLR_AUTOMAP; /* maps */

      if (bit)
      {
         if (fSet)
            xSET_BIT(ch->act, bit);
         else
            xREMOVE_BIT(ch->act, bit);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      else
      {
         if (!str_cmp(arg + 1, "norecall"))
            bit = PCFLAG_NORECALL;
         else if (!str_cmp(arg + 1, "nofollow"))
            bit = PCFLAG_NOFOLLOW;
         else if (!str_cmp(arg + 1, "nointro"))
            bit = PCFLAG_NOINTRO;
         else if (!str_cmp(arg + 1, "nosummon"))
            bit = PCFLAG_NOSUMMON;
         else if (!str_cmp(arg + 1, "gag"))
            bit = PCFLAG_GAG;
         else if (!str_cmp(arg + 1, "pager"))
            bit = PCFLAG_PAGERON;
         else if (IS_IMMORTAL(ch) && !str_cmp(arg + 1, "nofinger"))
            bit = PCFLAG_NOFINGER;
         else if (!str_cmp(arg + 1, "nobeep"))
            bit = PCFLAG_NOBEEP;
         else if (!str_cmp(arg + 1, "groupwho"))
            bit = PCFLAG_GROUPWHO;
         else if (!str_cmp(arg + 1, "@hgflag_"))
            bit = PCFLAG_HIGHGAG;
         else if (IS_STAFF(ch) && !str_cmp(arg + 1, "autoproto"))
            bit = PCFLAG_AUTOPROTO;
         else if (!str_cmp(arg + 1, "cnoassist"))
            bit = PCFLAG_CNOASSIST;
         else
         {
            send_to_char("Config which option?\n\r", ch);
            return;
         }

         if (fSet)
            SET_BIT(ch->pcdata->flags, bit);
         else
            REMOVE_BIT(ch->pcdata->flags, bit);

         send_to_char("Ok.\n\r", ch);
         return;
      }
   }

   return;
}
int get_star_worth(CHAR_DATA *ch, int sn, int points)
{
   int value;
   int flux;
   flux = URANGE(1, get_skillflux_value(ch, sn), 100);
   
   if (points <= 2)
      value = 100;
   else if (points <= 4)
      value = 500;
   else if (points <= 6)
      value = 1500;
   else if (points <= 8)
      value = 4500;
   else if (points <= 10)
      value = 13500;
   else if (points <= 14)
      value = 40000;
   else if (points <= 17)
      value = 120000;
   else
      value = 350000;
      
   if (skill_table[sn]->masterydiff[0] == 2)
      value = value * 150 /100;
   if (skill_table[sn]->masterydiff[0] == 3)
      value = value * 200 /100;
   if (skill_table[sn]->masterydiff[0] == 4)
      value = value * 300 /100;
      
   value = value * 100 / flux;
   return value;
}

/*1-2   1k
  2-4   5k
  5-6   15k
  7-8   45k
  9-10  135k
  11-14 400k
  15-17 1200k
  18-20 3500k
*/

int matches_valid_stat(char *argument)
{
   if (!str_cmp(argument, "strength") || !str_cmp(argument, "str"))
      return 1;
   if (!str_cmp(argument, "dexterity") || !str_cmp(argument, "dex"))
      return 2;
   if (!str_cmp(argument, "intelligence") || !str_cmp(argument, "int"))
      return 3;
   if (!str_cmp(argument, "wisdom") || !str_cmp(argument, "wis"))
      return 4;
   if (!str_cmp(argument, "luck") || !str_cmp(argument, "lck") || !str_cmp(argument, "luc"))
      return 5;
   if (!str_cmp(argument, "constitution") || !str_cmp(argument, "con"))
      return 6;
   if (!str_cmp(argument, "agility") || !str_cmp(argument, "agi"))
      return 7;
   if (!str_cmp(argument, "endurance") || !str_cmp(argument, "end") || !str_cmp(argument, "movement") || !str_cmp(argument, "move")
   ||  !str_cmp(argument, "stanima") || !str_cmp(argument, "sta"))
      return 8;
   if (!str_cmp(argument, "hp") || !str_cmp(argument, "health"))
      return 9;
   if (!str_cmp(argument, "mana") || !str_cmp(argument, "magic") || !str_cmp(argument, "mp"))
      return 10;
      
   return 0;
}

int get_statstar_worth(CHAR_DATA *ch, int stat)
{
   int range = -1;
   if (stat >= 1 && stat <= 6)
   {
      if (stat == 1)
      {
         range = 14 + race_table[ch->race]->str_plus - 5 + race_table[ch->race]->str_range;
         range = ch->perm_str - range;
      }
      else if (stat == 2)
      {
         range = 14 + race_table[ch->race]->dex_plus - 5 + race_table[ch->race]->dex_range;
         range = ch->perm_dex - range;
      }
      else if (stat == 3)
      {
         range = 14 + race_table[ch->race]->int_plus - 5 + race_table[ch->race]->int_range;
         range = ch->perm_int - range;
      }
      else if (stat == 4)
      {
         range = 14 + race_table[ch->race]->wis_plus - 5 + race_table[ch->race]->wis_range;
         range = ch->perm_wis - range;
      }
      else if (stat == 5)
      {
         range = 22 + race_table[ch->race]->lck_plus - 9;
         range = (ch->perm_lck - 9) * 100 / range;
         if (range <= 20)
            return 75 * ((ch->max_hit+ch->max_mana)/7);
         else if (range <= 40)
            return 110 * ((ch->max_hit+ch->max_mana)/7);
         else if (range <= 60)
            return 150 * ((ch->max_hit+ch->max_mana)/7);
         else if (range <= 80)
            return 300 * ((ch->max_hit+ch->max_mana)/7);
         else
            return 500 * ((ch->max_hit+ch->max_mana)/7); 
      }
      else if (stat == 6)
      {
         range = 14 + race_table[ch->race]->con_plus - 5 + race_table[ch->race]->con_range;
         range = ch->perm_con - range;
      }
      if (range == 1)
         return 75 * ((ch->max_hit+ch->max_mana)/5);
      else if (range == 2)
         return 90 * ((ch->max_hit+ch->max_mana)/5);
      else if (range == 3)
         return 150 * ((ch->max_hit+ch->max_mana)/5);
      else if (range == 4)
         return 300 * ((ch->max_hit+ch->max_mana)/5);
      else if (range == 5)
         return 500 * ((ch->max_hit+ch->max_mana)/5);
      else if (range == 6)
         return 600 * ((ch->max_hit+ch->max_mana)/5);
      else if (range == 7)
         return 700 * ((ch->max_hit+ch->max_mana)/5);
      else if (range == 8)
         return 900 * ((ch->max_hit+ch->max_mana)/5);
      else //0
         return 50 * ((ch->max_hit+ch->max_mana)/5);
   }
   else if (stat == 7)
   {
      range = race_table[ch->race]->agi_start - race_table[ch->race]->agi_range;
      range = URANGE(0, ch->perm_agi - range, race_table[ch->race]->agi_start + race_table[ch->race]->agi_range);
      range = range * 100 / race_table[ch->race]->agi_range*2;
      if (range <= 20)
         return 75 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 40)
         return 110 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 60)
         return 150 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 80)
         return 300 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 100)
         return 500 * ((ch->max_hit+ch->max_mana)/15); 
      else
         return 700 * ((ch->max_hit+ch->max_mana)/15);
   }
   else if (stat == 8) //Move
   {
      range = MAX_PC_ENDURANCE - ch->mover;
      range = range * 100 / MAX_PC_ENDURANCE - 30;
      if (range < 0)
         return 700 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 20)
         return 500 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 40)
         return 300 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 60)
         return 150 * ((ch->max_hit+ch->max_mana)/15);
      else if (range <= 80)
         return 110 * ((ch->max_hit+ch->max_mana)/15);
      else
         return 75 * ((ch->max_hit+ch->max_mana)/15); 
 
   }
  
   
   else if (stat == 9) //hp
   {
      int hp;
      if (ch->max_hit <= 50)
         hp = 50;
      else if (ch->max_hit <= 75)
         hp = 125;
      else if (ch->max_hit <= 100)
         hp = 300;
      else if (ch->max_hit <= 150)
         hp = 800;
      else if (ch->max_hit <= 200)
         hp = 2000;
      else if (ch->max_hit <= 300)
         hp = 5500;
      else if (ch->max_hit <= 400)
         hp = 12000;
      else if (ch->max_hit <= 500)
         hp = 25000;
      else if (ch->max_hit <= 700)
         hp = 40000;
      else if (ch->max_hit <= 1000)
         hp = 50000;
      else if (ch->max_hit <= 1500)
         hp = 60000;
      else
         hp = 70000;         
      if (ch->race == RACE_OGRE)
         return hp/3;
      else if (ch->race == RACE_FAIRY)
         return hp*3;
      else
         return hp;
   }
   else if (stat == 10) //mana
   {
      int mp;
      if (ch->max_mana <= 50)
         mp = 50;
      else if (ch->max_mana <= 100)
         mp = 125;
      else if (ch->max_mana <= 150)
         mp = 300;
      else if (ch->max_mana <= 200)
         mp = 800;
      else if (ch->max_mana <= 300)
         mp = 2000;
      else if (ch->max_mana <= 400)
         mp = 5500;
      else if (ch->max_mana <= 500)
         mp = 12000;
      else if (ch->max_mana <= 600)
         mp = 25000;
      else if (ch->max_mana <= 800)
         mp = 40000;
      else if (ch->max_mana <= 1000)
         mp = 50000;
      else if (ch->max_mana <= 1500)
         mp = 60000;
      else
         mp = 70000;     
         
      if (ch->race == RACE_OGRE)
         return mp*5;
      else if (ch->race == RACE_FAIRY)
         return mp/3;
      else if (ch->race == RACE_ELF)
         return mp/3*2;
      else
         return mp;   
   }
   return -1;
}

int get_currentvalue(CHAR_DATA *ch, int stat)
{
   if (stat == 1)
      return ch->pcdata->per_str;
   else if (stat == 2)
      return ch->pcdata->per_dex;
   else if (stat == 3)
      return ch->pcdata->per_int;
   else if (stat == 4)
      return ch->pcdata->per_wis;
   else if (stat == 5)
      return ch->pcdata->per_lck;
   else if (stat == 6)
      return ch->pcdata->per_con;
   else if (stat == 7)
      return ch->pcdata->per_agi;
   else if (stat == 8)
      return ch->pcdata->per_move;
   else if (stat == 9)
      return ch->pcdata->per_hp;
   else if (stat == 10)
      return ch->pcdata->per_mana;
   else
      return -1;
}

int get_maxstat_value(CHAR_DATA *ch, int stat)
{
   if (stat >= 1 && stat <= 6)
      return 10000;
   else
      return 1000;
}

int get_talent_increase(CHAR_DATA *ch, int stat)
{
   if (IS_NPC(ch))
      return 0;
   if (stat == 1)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_STR3))
         return 3;
      if (xIS_SET(ch->pcdata->talent, TALENT_STR2))
         return 2;
      if (xIS_SET(ch->pcdata->talent, TALENT_STR1))
         return 1;
      return 0;
   }
   if (stat == 2)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_DEX3))
         return 3;
      if (xIS_SET(ch->pcdata->talent, TALENT_DEX2))
         return 2;
      if (xIS_SET(ch->pcdata->talent, TALENT_DEX1))
         return 1;
      return 0;
   }
   if (stat == 3)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_INT3))
         return 3;
      if (xIS_SET(ch->pcdata->talent, TALENT_INT2))
         return 2;
      if (xIS_SET(ch->pcdata->talent, TALENT_INT1))
         return 1;
      return 0;
   }
   if (stat == 4)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_WIS3))
         return 3;
      if (xIS_SET(ch->pcdata->talent, TALENT_WIS2))
         return 2;
      if (xIS_SET(ch->pcdata->talent, TALENT_WIS1))
         return 1;
      return 0;
   }
   if (stat == 6)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_CON3))
         return 3;
      if (xIS_SET(ch->pcdata->talent, TALENT_CON2))
         return 2;
      if (xIS_SET(ch->pcdata->talent, TALENT_CON1))
         return 1;
      return 0;
   }
   if (stat == 7)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_AGI3))
         return 15;
      if (xIS_SET(ch->pcdata->talent, TALENT_AGI2))
         return 10;
      if (xIS_SET(ch->pcdata->talent, TALENT_AGI1))
         return 5;
      return 0;
   }
   if (stat == 8)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_END3))
         return 30;
      if (xIS_SET(ch->pcdata->talent, TALENT_END2))
         return 20;
      if (xIS_SET(ch->pcdata->talent, TALENT_END1))
         return 10;
      return 0;
   }
   if (stat == 9)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_HP5))
         return 1300;
      if (xIS_SET(ch->pcdata->talent, TALENT_HP4))
         return 800;
      if (xIS_SET(ch->pcdata->talent, TALENT_HP3))
         return 500;
      if (xIS_SET(ch->pcdata->talent, TALENT_HP2))
         return 300;
      if (xIS_SET(ch->pcdata->talent, TALENT_HP1))
         return 100;
      return 0;
   }
   if (stat == 10)
   {
      if (xIS_SET(ch->pcdata->talent, TALENT_MP5))
         return 1200;
      if (xIS_SET(ch->pcdata->talent, TALENT_MP4))
         return 700;
      if (xIS_SET(ch->pcdata->talent, TALENT_MP3))
         return 500;
      if (xIS_SET(ch->pcdata->talent, TALENT_MP2))
         return 300;
      if (xIS_SET(ch->pcdata->talent, TALENT_MP1))
         return 100;
      return 0;
   }
   return 0;
}

int get_maxstat(CHAR_DATA *ch, int stat)
{
   if (stat == 1)
      return 14 + race_table[ch->race]->str_plus + race_table[ch->race]->str_range + get_talent_increase(ch, stat);
   else if (stat == 2)
      return 14 + race_table[ch->race]->dex_plus + race_table[ch->race]->dex_range + get_talent_increase(ch, stat);
   else if (stat == 3)
      return 14 + race_table[ch->race]->int_plus + race_table[ch->race]->int_range + get_talent_increase(ch, stat);
   else if (stat == 4)
      return 14 + race_table[ch->race]->wis_plus + race_table[ch->race]->wis_range + get_talent_increase(ch, stat);
   else if (stat == 5)
      return 22 + race_table[ch->race]->lck_plus + get_talent_increase(ch, stat);
   else if (stat == 6)
      return 14 + race_table[ch->race]->con_plus + race_table[ch->race]->con_range + get_talent_increase(ch, stat);
   else if (stat == 7)
      return race_table[ch->race]->agi_start + race_table[ch->race]->agi_range + get_talent_increase(ch, stat);
   else if (stat == 8)
      return MAX_PC_ENDURANCE + get_talent_increase(ch, stat);
   else if (stat == 9)
      return 700 + get_talent_increase(ch, stat);
   else if (stat == 10)
      return 800 + get_talent_increase(ch, stat);
   else
      return -1;
}

int get_actual_stat_value(CHAR_DATA *ch, int stat)
{
   if (stat == 1)
      return ch->perm_str;
   else if (stat == 2)
      return ch->perm_dex;
   else if (stat == 3)
      return ch->perm_int;
   else if (stat == 4)
      return ch->perm_wis;
   else if (stat == 5)
      return ch->perm_lck;
   else if (stat == 6)
      return ch->perm_con;
   else if (stat == 7)
      return ch->perm_agi;
   else if (stat == 8)
      return ch->mover;
   else if (stat == 9)
      return ch->max_hit;
   else if (stat == 10)
      return ch->max_mana;
   else
      return -1;
}       

int increase_stat_value(CHAR_DATA *ch, int stat, int stars, int add)
{
   int increase = 0;
   
   if (stars == 0)
   {
      if (stat <= 8)
         increase = 1;
      if (stat == 1)
         ch->perm_str++;
      if (stat == 2)
         ch->perm_dex++;
      if (stat == 3)
         ch->perm_int++;
      if (stat == 4)
         ch->perm_wis++;
      if (stat == 5)
         ch->perm_lck++;
      if (stat == 6)
         ch->perm_con++;
      if (stat == 7)
         ch->perm_agi++;
      if (stat == 8)
         ch->mover++;
      if (stat == 9)
      {
         if (sysdata.stat_gain <= 1)
         {
            if (ch->max_hit < 70)
               increase += number_range(5, 7);
            else if (ch->max_hit < 100)
               increase += number_range(4, 6);
            else if (ch->max_hit < 150)
               increase += number_range(2, 4);
            else if (ch->max_hit < 200)
               increase += number_range(1, 3);
            else
               increase += 1;
         }
         else if (sysdata.stat_gain <= 2)
         {
            if (ch->max_hit < 70)
               increase += number_range(5, 7);
            else if (ch->max_hit < 100)
               increase += number_range(4, 6);
            else if (ch->max_hit < 200)
               increase += number_range(3, 5);
            else if (ch->max_hit < 300)
               increase += number_range(2, 4);
            else
               increase += number_range(1, 2);
         }
         else if (sysdata.stat_gain >= 4)
         {
            if (ch->max_hit < 150)
               increase += number_range(5, 7);
            else if (ch->max_hit < 300)
               increase += number_range(4, 6);
            else if (ch->max_hit < 600)
               increase += number_range(3, 5);
            else if (ch->max_hit < 900)
               increase += number_range(2, 4);
            else
               increase += number_range(1, 2);
         }
         ch->max_hit += increase;
      }
      if (stat == 10)
      {
         if (sysdata.stat_gain <= 1)
         {
            if (ch->max_mana < 70)
               increase += number_range(5, 7);
            else if (ch->max_mana < 100)
               increase += number_range(4, 6);
            else if (ch->max_mana < 150)
               increase += number_range(2, 4);
            else if (ch->max_mana< 200)
               increase += number_range(1, 3);
            else
               increase += 1;
         }
         else if (sysdata.stat_gain <= 2)
         {
            if (ch->max_mana < 70)
               increase += number_range(5, 7);
            else if (ch->max_mana < 100)
               increase += number_range(4, 6);
            else if (ch->max_mana < 200)
               increase += number_range(3, 5);
            else if (ch->max_mana < 300)
               increase += number_range(2, 4);
            else
               increase += number_range(1, 2);
         }
         else if (sysdata.stat_gain >= 4)
         {
            if (ch->max_mana < 150)
               increase += number_range(5, 7);
            else if (ch->max_mana < 300)
               increase+= number_range(4, 6);
            else if (ch->max_mana < 600)
               increase += number_range(3, 5);
            else if (ch->max_mana < 900)
               increase += number_range(2, 4);
            else
               increase += number_range(1, 2);
         }
         ch->max_mana += increase;
      }
         
      if (stat == 1)
         ch->pcdata->per_str=add;
      if (stat == 2)
         ch->pcdata->per_dex=add;
      if (stat == 3)
         ch->pcdata->per_int=add;
      if (stat == 4)
         ch->pcdata->per_wis=add;
      if (stat == 5)
         ch->pcdata->per_lck=add;
      if (stat == 6)
         ch->pcdata->per_con=add;
      if (stat == 7)
         ch->pcdata->per_agi=add;
      if (stat == 8)
         ch->pcdata->per_move=add;
      if (stat == 9)
         ch->pcdata->per_hp=add;
      if (stat == 10)
         ch->pcdata->per_mana=add;
   }
   else
   {
      if (stat == 1)
         ch->pcdata->per_str+=stars;
      if (stat == 2)
         ch->pcdata->per_dex+=stars;
      if (stat == 3)
         ch->pcdata->per_int+=stars;
      if (stat == 4)
         ch->pcdata->per_wis+=stars;
      if (stat == 5)
         ch->pcdata->per_lck+=stars;
      if (stat == 6)
         ch->pcdata->per_con+=stars;
      if (stat == 7)
         ch->pcdata->per_agi+=stars;
      if (stat == 8)
         ch->pcdata->per_move+=stars;
      if (stat == 9)
         ch->pcdata->per_hp+=stars;
      if (stat == 10)
         ch->pcdata->per_mana+=stars;
   }  
   return increase;
}

int get_spoint_talent(int flag)
{
   if (flag == TALENT_SP1)
      return 3000000;
   if (flag == TALENT_HP1 || flag == TALENT_MP1)
      return 5000000;
   if (flag == TALENT_STR1 || flag == TALENT_CON1 || flag == TALENT_INT1
   ||  flag == TALENT_WIS1 || flag == TALENT_DEX1 || flag == TALENT_AGI1
   ||  flag == TALENT_SP2 || flag == TALENT_END1)
      return 10000000;
   if (flag == TALENT_HP2 || flag == TALENT_MP2)
      return 15000000;
   if (flag == TALENT_SP3)
      return 20000000;
   if (flag == TALENT_STR2 || flag == TALENT_CON2 || flag == TALENT_INT2
   ||  flag == TALENT_WIS2 || flag == TALENT_DEX2 || flag == TALENT_AGI2
   ||  flag == TALENT_END2 || flag == TALENT_DAMCAP1)
      return 25000000;
   if (flag == TALENT_HP3 || flag == TALENT_MP3)
      return 30000000;
   if (flag == TALENT_SP4)
      return 35000000;
   if (flag == TALENT_STR3 || flag == TALENT_CON3 || flag == TALENT_INT3
   ||  flag == TALENT_WIS3 || flag == TALENT_DEX3 || flag == TALENT_AGI3
   ||  flag == TALENT_HP4 || flag == TALENT_MP4 || flag == TALENT_SP5
   ||  flag == TALENT_END3 || flag == TALENT_DAMCAP2)
      return 50000000;
   if (flag == TALENT_HP5 || flag == TALENT_MP5 || flag == TALENT_DAMCAP3)
      return 100000000;
   bug("Invalid talent flag of %d", flag);
   return -1;
}  

char *get_talent_name(int x)
{
   if (x == 1)
      return "HPBONUS 1";
   if (x == 2)
      return "HPBONUS 2";
   if (x == 3)
      return "HPBONUS 3";
   if (x == 4)
      return "HPBONUS 4";
   if (x == 5)
      return "HPBONUS 5";
   if (x == 6)
      return "MPBONUS 1";
   if (x == 7)
      return "MPBONUS 2";
   if (x == 8)
      return "MPBONUS 3";
   if (x == 9)
      return "MPBONUS 4";
   if (x == 10)
      return "MPBONUS 5";
   if (x == 11)
      return "STRBONUS 1";
   if (x == 12)
      return "STRBONUS 2";
   if (x == 13)
      return "STRBONUS 3";
   if (x == 14)
      return "CONBONUS 1";
   if (x == 15)
      return "CONBONUS 2";
   if (x == 16)
      return "CONBONUS 3";
   if (x == 17)
      return "INTBONUS 1";
   if (x == 18)
      return "INTBONUS 2";
   if (x == 19)
      return "INTBONUS 3";
   if (x == 20)
      return "WISBONUS 1";
   if (x == 21)
      return "WISBONUS 2";
   if (x == 22)
      return "WISBONUS 3";
   if (x == 23)
      return "DEXBONUS 1";
   if (x == 24)
      return "DEXBONUS 2";
   if (x == 25)
      return "DEXBONUS 3";
   if (x == 26)
      return "AGIBONUS 1";
   if (x == 27)
      return "AGIBONUS 2";
   if (x == 28)
      return "AGIBONUS 3";
   if (x == 29)
      return "ENDBONUS 1";
   if (x == 30)
      return "ENDBONUS 2";
   if (x == 31)
      return "ENDBONUS 3";
   if (x == 32)
      return "SPOINTBONUS 1";
   if (x == 33)
      return "SPOINTBONUS 2";
   if (x == 34)
      return "SPOINTBONUS 3";
   if (x == 35)
      return "SPOINTBONUS 4";
   if (x == 36)
      return "SPOINTBONUS 5";
   if (x == 37)
      return "DAMCAP 1";
   if (x == 38)
      return "DAMCAP 2";
   if (x == 39)
      return "DAMCAP 3"; 
      
   return "???????????";
}

int can_obtain_talent(CHAR_DATA *ch, int x)
{
   int flag = x-1;
   
   if (xIS_SET(ch->pcdata->talent, flag)) //Already have that...
      return 0;
      
   //First level is available for all
   if (flag == TALENT_HP1 || flag == TALENT_MP1 || flag == TALENT_CON1
   ||  flag == TALENT_STR1 || flag == TALENT_INT1 || flag == TALENT_WIS1
   ||  flag == TALENT_DEX1 || flag == TALENT_AGI1 || flag == TALENT_END1
   ||  flag == TALENT_SP1)
      return 1;  
      
   //Damcap is for ogres/humans/dwarves.
   if (flag == TALENT_DAMCAP1)
   {
      if (ch->race == RACE_FAIRY || ch->race == RACE_ELF || ch->race == RACE_HOBBIT)
         return 0;
      else
         return 1;
   }
   if (flag == TALENT_DAMCAP2 || flag == TALENT_DAMCAP3)
   {
      if (ch->race != RACE_OGRE)
         return 0;
      if (!xIS_SET(ch->pcdata->talent, flag-1)) //Need 1 before 2, etc
         return 0;
      else
         return 1;
   }
      
   //Humans can learn everything (race bonus) others are restricted based
   //on how they are setup by stats, etc
   if (ch->race == RACE_DWARF)
   {
      if (flag == TALENT_INT3)
         return 0;
      if (flag == TALENT_DEX3)
         return 0;
      if (flag == TALENT_AGI3)
         return 0;
      if (flag >= TALENT_SP4 && flag <= TALENT_SP5)
         return 0;
      if (flag == TALENT_HP5)
         return 0;
      if (flag >= TALENT_MP4 && flag <= TALENT_MP5)
         return 0;
   }
   if (ch->race == RACE_ELF)
   {
      if (flag == TALENT_WIS3)
         return 0;
      if (flag == TALENT_STR3)
         return 0;
      if (flag == TALENT_CON3)
         return 0;
      if (flag == TALENT_DEX3)
         return 0;
      if (flag == TALENT_SP4 && flag <= TALENT_SP5)
         return 0;
      if (flag == TALENT_AGI3)
         return 0;
      if (flag == TALENT_END3)
         return 0;
      if (flag >= TALENT_HP4 && flag <= TALENT_HP5)
         return 0;
      if (flag >= TALENT_MP5)
         return 0;
   }
   if (ch->race == RACE_HOBBIT)
   {
      if (flag == TALENT_WIS3)
         return 0;
      if (flag >= TALENT_INT2)
         return 0;
      if (flag >= TALENT_END2 && flag <= TALENT_END3)
         return 0;
      if (flag >= TALENT_STR2 && flag <= TALENT_STR3)
         return 0;
      if (flag >= TALENT_CON2 && flag <= TALENT_CON3)
         return 0;
      if (flag >= TALENT_HP3 && flag <= TALENT_HP5)
         return 0;
      if (flag >= TALENT_MP3 && flag <= TALENT_MP5)
         return 0;
      if (flag >= TALENT_SP4 && flag <= TALENT_SP5)
         return 0;
   }
   if (ch->race == RACE_OGRE)
   {
      if (flag >= TALENT_WIS2 && flag <= TALENT_WIS3)
         return 0;
      if (flag >= TALENT_INT2 && flag <= TALENT_INT2)
         return 0;
      if (flag >= TALENT_DEX2 && flag <= TALENT_DEX3)
         return 0;
      if (flag == TALENT_AGI3)
         return 0;
      if (flag >= TALENT_MP2 && flag <= TALENT_MP5)
         return 0;
      if (flag >= TALENT_SP4 && flag <= TALENT_SP5)
         return 0;
   }
   if (ch->race == RACE_FAIRY)
   {
      if (flag >= TALENT_STR2 && flag <= TALENT_STR3)
         return 0;
      if (flag >= TALENT_CON2 && flag <= TALENT_CON2)
         return 0;
      if (flag == TALENT_DEX3)
         return 0;
      if (flag >= TALENT_END2 && flag <= TALENT_END3)
         return 0;
      if (flag >= TALENT_HP2 && flag <= TALENT_HP5)
         return 0;
      if (flag >= TALENT_SP4 && flag <= TALENT_SP5)
         return 0;
   }
   if (!xIS_SET(ch->pcdata->talent, flag-1)) //Need 1 before 2, etc
      return 0;
   // Ok done checking now.
   return 1;
}
      
   
//Used to spend on skills/spells and then on talents once you get enough of em
void do_talent(CHAR_DATA *ch, char *argument)
{
   int sn;
   char arg1[MIL];
   char arg2[MIL];
   float pcheck;
   int sworth;
   int stars;
   int stat;
   int cvalue;
   int mvalue;
   int mstat;
   int increase = 0;
   int pvalue;
   int svalue;
   
   if (check_npc(ch))
      return;
      
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  talent <skill/spell> spend <#stars>\n\r", ch);
      send_to_char("Syntax:  talent <skill/spell> spend point\n\r", ch);
      send_to_char("Syntax:  talent <skill/spell> available\n\r", ch);
      send_to_char("Syntax:  talent <skill/spell> cost\n\r", ch);
      send_to_char("Syntax:  talent <stat name> cost\n\r", ch);
      send_to_char("Syntax:  talent <stat name> available\n\r", ch);
      send_to_char("Syntax:  talent <stat name> spend <#stars>\n\r", ch);
      send_to_char("Syntax:  talent <stat name> spend point\n\r", ch);
      send_to_char("Syntax:  talent list [obtained]\n\r", ch);
      send_to_char("Syntax:  talent purchase <number>\n\r", ch);
      send_to_char("\n\rType help talent <name of talent> for a description of the talent.\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (!str_cmp(arg1, "purchase"))
   {
      if (atoi(arg2) < 1 || atoi(arg2) > TALENT_MAX)
      {
         ch_printf(ch, "Purchase range is 1 to %d.\n\r", TALENT_MAX);
         return;
      }
      if (get_spoint_talent(atoi(arg2)-1) == -1)
      {
         send_to_char("There is an error tell an imm.\n\r", ch);
         return;
      }
      if (!can_obtain_talent(ch, atoi(arg2)))
      {
         send_to_char("You cannot obtain that talent yet or you already have it.\n\r", ch);
         return;
      }
      if (ch->pcdata->spoints < get_spoint_talent(atoi(arg2)-1))
      {
         ch_printf(ch, "That costs %d spoints and you don't have that many.\n\r", get_spoint_talent(atoi(arg2)-1));
         return;
      }
      ch->pcdata->spoints -= get_spoint_talent(atoi(arg2)-1);
      xSET_BIT(ch->pcdata->talent, atoi(arg2)-1);
      send_to_char("Done.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg1, "list"))
   {
      if (arg2[0] == '\0')
      {
         int x;
         send_to_char("#   Talent                        Cost (in Spoints)\n\r", ch);
         send_to_char("---------------------------------------------\n\r", ch);
         for (x = 1; x <= TALENT_MAX; x++)
         {
            if (can_obtain_talent(ch, x))
            {
               ch_printf(ch, "%-2d> %-28s  %s\n\r", x, get_talent_name(x), punct(get_spoint_talent(x-1)));
            }
         }
         return;
      }
      if (!str_cmp(arg2, "obtained"))
      {
         int x;
         send_to_char("#   Talent                        \n\r", ch);
         send_to_char("----------------------------------\n\r", ch);
         for (x = 1; x <= TALENT_MAX; x++)
         {
            if (xIS_SET(ch->pcdata->talent, x-1))
            {
               ch_printf(ch, "%-2d> %-28s\n\r", x, get_talent_name(x));
            }
         }
         return;
      }   
      do_talent(ch, "");
   }
   if ((stat = matches_valid_stat(arg1)) > 0)
   {
      if (!str_cmp(arg2, "cost"))
      {
         sworth = get_statstar_worth(ch, stat);
         cvalue = get_currentvalue(ch, stat);
         mvalue = get_maxstat_value(ch, stat);
         if (sworth == -1)
         {
            send_to_char("There was an error calculating worth, tell an immortal.\n\r", ch);
            return;
         }
         if (cvalue == -1)
         {
            send_to_char("There was an error finding your current value, tell an immortal.\n\r", ch);
            return;
         }
         ch_printf(ch, "It will cost you %d spoints to raise %s 1 star.\n\r", sworth, arg1);
         pcheck = (((float)mvalue - (float)cvalue) * (float)sworth) / ((float)mvalue/10);
         sworth = (int)pcheck;
         ch_printf(ch, "It would cost you %d skill points to raise %s 1 point\n\r", sworth, arg1);
         return;
      }
      if (!str_cmp(arg2, "available"))
      {
         sworth = get_statstar_worth(ch, stat);
         cvalue = get_currentvalue(ch, stat);
         mvalue = get_maxstat_value(ch, stat);
         stars = ch->pcdata->spoints / sworth;
         
         if (sworth == -1)
         {
            send_to_char("There was an error calculating worth, tell an immortal.\n\r", ch);
            return;
         }
         if (cvalue == -1)
         {
            send_to_char("There was an error finding your current value, tell an immortal.\n\r", ch);
            return;
         }
         
         if (stars == 0)
            ch_printf(ch, "You do not have enough skill points to even raise 1 star in %s\n\r", arg1);
         else if (cvalue + (stars*mvalue/10) >= mvalue)
            ch_printf(ch, "You have enough skill points to raise a full point in %s\n\r", arg1);
         else
            ch_printf(ch, "You have enough skill points to raise %d stars in %s\n\r", stars, arg1);
         return;
      }  
      
      
      if (!str_cmp(arg2, "spend"))
      {
         if (!str_cmp(argument, "point"))
         {
            sworth = get_statstar_worth(ch, stat);  //Worth of a star in skill points
            cvalue = get_currentvalue(ch, stat);  //Points you already have in that stat (per_con)
            mvalue = get_maxstat_value(ch, stat); //Max value in a stat before increase (1k or 10k)
            mstat = get_maxstat(ch, stat);     //Max points you can have in a stat (17-23, etc)
            pvalue = mvalue*3/10;              //The 3000 or 300 limit in a stat
            svalue = get_actual_stat_value(ch, stat);   //Actual stat number you have ex: str 12
            
            if (svalue == -1)
            {
               send_to_char("You have an invalid stat value tell an immortal.\n\r", ch);
               return;
            } 
            if (sworth == -1)
            {
               send_to_char("There was an error calculating worth, tell an immortal.\n\r", ch);
               return;
            }
            if (cvalue == -1)
            {
               send_to_char("There was an error finding your current value, tell an immortal.\n\r", ch);
               return;
            }
            stars = cvalue / sworth;
            pcheck = (((float)mvalue - (float)cvalue) * (float)sworth) / ((float)mvalue/10);
            stars= (int) pcheck;
            if (svalue >= mstat)
            {
               send_to_char("You have that stat maxed you cannot spend any more skill points on it.\n\r", ch);
               return;
            }
            if (stars > ch->pcdata->spoints)
            {
               ch_printf(ch, "It would take %d points to go up a whole point in %s\n\r", stars, arg1);
               return;
            }
            else
            {               
               ch->pcdata->spoints -= stars;
               increase = increase_stat_value(ch, stat, 0, mvalue * 3 / 100);
               if (increase <= 1)
                  ch_printf(ch, "You added (ONE POINT) to %s\n\r", arg1);
               else
                  ch_printf(ch, "You added (%d POINTS) to %s\n\r", increase, arg1);
               return;
            }
         }              
         else
         {
            sworth = get_statstar_worth(ch, stat);  //Worth of a star in skill points
            cvalue = get_currentvalue(ch, stat);  //Points you already have in that stat (per_con)
            mvalue = get_maxstat_value(ch, stat); //Max value in a stat before increase (1k or 10k)
            mstat = get_maxstat(ch, stat);     //Max points you can have in a stat (17-23, etc)
            pvalue = mvalue*3/10;              //The 3000 or 300 limit in a stat
            svalue = get_actual_stat_value(ch, stat);   //Actual stat number you have ex: str 12
            
            if (atoi(argument) <= 0)
            {
               send_to_char("You must specify how many stars you want to purchase.\n\r", ch);
               return;
            }
            if (svalue == -1)
            {
               send_to_char("You have an invalid stat value tell an immortal.\n\r", ch);
               return;
            } 
            if (sworth == -1)
            {
               send_to_char("There was an error calculating worth, tell an immortal.\n\r", ch);
               return;
            }
            if (cvalue == -1)
            {
               send_to_char("There was an error finding your current value, tell an immortal.\n\r", ch);
               return;
            }
            stars = ch->pcdata->spoints / sworth;
            if (stars < atoi(argument))
            {
               ch_printf(ch, "You don't have enough skill points to raise %d stars in %s\n\r", atoi(argument), arg1);
               return;
            }
            else if (cvalue + (atoi(argument)*(mvalue/10)) >= mvalue)
            {
               ch_printf(ch, "That would put you over 10 stars in %s.  Use the point argument to do that!\n\r", arg1);
               return;
            }
            else
            {
               if ((svalue >= mstat && cvalue >= pvalue) || svalue > mstat)
               {
                  send_to_char("You have that stat maxed you cannot spend any more skill points on it.\n\r", ch);
                  return;
               }
               increase_stat_value(ch, stat, atoi(argument)*mvalue/10, 0);
               ch->pcdata->spoints -= atoi(argument) * sworth;
               ch_printf(ch, "You added %d stars to %s\n\r", atoi(argument), arg1);
               return;
            }
         }  
      }     
   }
   
   if ((sn = ch_slookup(ch, arg1)) == -1)
   {
      send_to_char("Either that skill/spell is invalid or you haven't learned it yet.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "spend"))
   {
      if (!str_cmp(argument, "point"))
      {
         sworth = get_star_worth(ch, sn, ch->pcdata->learned[sn]); 
         pcheck = ((10000 - (float)ch->pcdata->spercent[sn]) * (float)sworth) / 1000;
         stars= (int) pcheck;
         if (ch->pcdata->learned[sn] == 20)
         {
            send_to_char("You can only have 20 points in a skill/spell.\n\r", ch);
            return;
         }
         if (stars > ch->pcdata->spoints)
         {
            ch_printf(ch, "It would take %d points to go up a whole point in %s\n\r", stars, skill_table[sn]->name);
            return;
         }
         else
         {
            ch->pcdata->spoints -= stars;
            ch->pcdata->learned[sn]++;
            ch->pcdata->spercent[sn] = 200; //So skill doesn't go down in a few minutes
            if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
            {
               ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP]++;
               ch->pcdata->spherepoints[skill_table[sn]->stype]++;
            }
            else
            {
               ch->pcdata->grouppoints[skill_table[sn]->group[0]]++;
                ch->pcdata->spherepoints[skill_table[sn]->stype]++;
            }
            ch_printf(ch, "You added (ONE POINT) to %s\n\r", skill_table[sn]->name);
            return;
         }
               
      }
      else
      {
         sworth = get_star_worth(ch, sn, ch->pcdata->learned[sn]); 
         stars = ch->pcdata->spoints / sworth;
         if (atoi(argument) <= 0)
         {
            send_to_char("You must specify how many stars you want to purchase.\n\r", ch);
            return;
         }
         if (stars < atoi(argument))
         {
            ch_printf(ch, "You don't have enough skill points to raise %d stars in %s\n\r", atoi(argument), skill_table[sn]->name);
            return;
         }
         else if (ch->pcdata->spercent[sn] + (atoi(argument)*1000) >= 10000)
         {
            ch_printf(ch, "That would put you over 10 stars in %s.  Use the point argument to do that!\n\r", skill_table[sn]->name);
            return;
         }
         else
         {
            if (ch->pcdata->learned[sn] == 20 && (ch->pcdata->spercent[sn] + (atoi(argument)*1000)) > 3000)
            {
               send_to_char("You can only have 3 stars in a 20 point skill.\n\r", ch);
               return;
            }
            ch->pcdata->spercent[sn] += atoi(argument)*1000;
            ch->pcdata->spoints -= atoi(argument) * sworth;
            ch_printf(ch, "You added %d stars to %s\n\r", atoi(argument), skill_table[sn]->name);
            return;
         }  
      }
   }
   if (!str_cmp(arg2, "cost"))
   {
      sworth = get_star_worth(ch, sn, ch->pcdata->learned[sn]); 
      ch_printf(ch, "It would cost you %d skill points to raise %s 1 star\n\r", sworth, skill_table[sn]->name); 
      pcheck = ((10000 - (float)ch->pcdata->spercent[sn]) * (float)sworth) / 1000;
      sworth = (int) pcheck;
      ch_printf(ch, "It would cost you %d skill points to raise %s 1 point\n\r", sworth, skill_table[sn]->name);
      return;
   }  
   if (!str_cmp(arg2, "available"))
   {
      sworth = get_star_worth(ch, sn, ch->pcdata->learned[sn]); 
      stars = ch->pcdata->spoints / sworth;
      if (stars == 0)
         ch_printf(ch, "You do not have enough skill points to even raise 1 star in %s\n\r", skill_table[sn]->name);
      else if (ch->pcdata->spercent[sn] + (stars*1000) >= 10000)
         ch_printf(ch, "You have enough skill points to raise a full point in %s\n\r", skill_table[sn]->name);
      else
         ch_printf(ch, "You have enough skill points to raise %d stars in %s\n\r", stars, skill_table[sn]->name);
      return;
   }    
   do_talent(ch, "");
   return;  
   
}
void do_target(CHAR_DATA *ch, char *argument)
{
    char arg[MIL];
    if (IS_NPC(ch))
    {
       send_to_char("For PCs only.\n\r", ch);
       return;
    }
    if (argument[0] == '\0')
    {
        send_to_char("Syntax:  Target <limb>\n\r", ch);
        send_to_char("Syntax:  Target <grip>\n\r", ch);
        send_to_char("Syntax:  Target off\n\r", ch);
        send_to_char("Syntax:  Target autocommand <command>\n\r", ch);
        send_to_char("Limb: body rarm larm rleg lleg neck head        Grip: bash slash stab\n\r", ch);
        return;
    }
    one_argument(argument, arg);
    if (!str_cmp(arg, "autocommand"))
    {
       argument = one_argument(argument, arg);
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->autocommand = STRALLOC(argument);
       ch_printf(ch, "You have set your autocommand to: %s", argument);
       return;
    }
    if (!str_prefix(argument, "off"))
    {
       xREMOVE_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       send_to_char("Auto-Target turned off.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "bash"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target = GRIP_BASH;
       send_to_char("You are now using bash as your grip.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "slash"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target = GRIP_SLASH;
       send_to_char("You are now using slash as your slash.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "stab"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target = GRIP_STAB;
       send_to_char("You are now using stab as your grip.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "body"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target_limb = LM_BODY;
       send_to_char("You are now aiming at the body.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "rarm"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target_limb = LM_RARM;
       send_to_char("You are now aiming at the right arm.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "larm"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target_limb = LM_LARM;
       send_to_char("You are now aiming at the left arm.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "rleg"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target_limb = LM_RLEG;
       send_to_char("You are now aiming at the right leg.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "lleg"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target_limb = LM_LLEG;
       send_to_char("You are now aiming at the left leg.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "head"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target_limb = LM_HEAD;
       send_to_char("You are now aiming at the head.\n\r", ch);
       return;
    }
    if (!str_prefix(argument, "neck"))
    {
       xSET_BIT(ch->act, PLR_TARGET);
       if (ch->pcdata->autocommand)
          STRFREE(ch->pcdata->autocommand);
       ch->pcdata->target_limb = LM_NECK;
       send_to_char("You are now aiming at the neck.\n\r", ch);
       return;
    }
    do_target(ch, "");
    return;
}

void do_credits(CHAR_DATA * ch, char *argument)
{
   do_help(ch, "credits");
}


extern int top_area;

/*
void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea1;
    AREA_DATA *pArea2;
    int iArea;
    int iAreaHalf;

    iAreaHalf = (top_area + 1) / 2;
    pArea1    = first_area;
    pArea2    = first_area;
    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
	pArea2 = pArea2->next;

    for ( iArea = 0; iArea < iAreaHalf; iArea++ )
    {
	ch_printf( ch, "%-39s%-39s\n\r",
	    pArea1->name, pArea2 ? pArea2->name : "" );
	pArea1 = pArea1->next;
	if ( pArea2 )
	    pArea2 = pArea2->next;
    }

    return;
}
*/

/* 
 * New do_areas with soft/hard level ranges 
void do_areas( CHAR_DATA *ch, char *argument )
{
    AREA_DATA *pArea;

    set_pager_color( AT_PLAIN, ch );
    send_to_pager("\n\r   Author    |             Area                     | Recommended |  Enforced\n\r", ch);
    send_to_pager("-------------+--------------------------------------+-------------+-----------\n\r", ch);

    for ( pArea = first_area; pArea; pArea = pArea->next )
	pager_printf(ch, "%-12s | %-36s | %4d - %-4d | %3d - %-3d \n\r", 
	 	pArea->author, pArea->name, pArea->low_soft_range, 
		pArea->hi_soft_range, pArea->low_hard_range, 
		pArea->hi_hard_range);
    return;
}
*/

/*
 * New do_areas, written by Fireblade, last modified - 4/27/97
 *
 *   Syntax: area            ->      lists areas in alphanumeric order
 *           area <a>        ->      lists areas with soft max less than
 *                                                    parameter a
 *           area <a> <b>    ->      lists areas with soft max bewteen
 *                                                    numbers a and b
 *           area old        ->      list areas in order loaded
 *
 */
void do_areas(CHAR_DATA * ch, char *argument)
{
   char *header_string1 = "\n\r   Author    |             Area" "                     | " "Recommended |  Enforced\n\r";
   char *header_string2 = "-------------+-----------------" "---------------------+----" "---------+-----------\n\r";
   char *print_string = "%-12s | %-36s | %4d - %-4d | %3d - " "%-3d \n\r";

   AREA_DATA *pArea;
   int lower_bound = 0;
   int upper_bound = MAX_LEVEL + 1;

   /* make sure is to init. > max area level */
   char arg[MSL];

   argument = one_argument(argument, arg);

   if (arg[0] != '\0')
   {
      if (!is_number(arg))
      {
         if (!strcmp(arg, "old"))
         {
            set_pager_color(AT_PLAIN, ch);
            send_to_pager(header_string1, ch);
            send_to_pager(header_string2, ch);
            for (pArea = first_area; pArea; pArea = pArea->next)
            {
               pager_printf(ch, print_string,
                  pArea->author, pArea->name, pArea->low_soft_range, pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range);
            }
            return;
         }
         else
         {
            send_to_char("Area may only be followed by numbers, or 'old'.\n\r", ch);
            return;
         }
      }

      upper_bound = atoi(arg);
      lower_bound = upper_bound;

      argument = one_argument(argument, arg);

      if (arg[0] != '\0')
      {
         if (!is_number(arg))
         {
            send_to_char("Area may only be followed by numbers.\n\r", ch);
            return;
         }

         upper_bound = atoi(arg);

         argument = one_argument(argument, arg);
         if (arg[0] != '\0')
         {
            send_to_char("Only two level numbers allowed.\n\r", ch);
            return;
         }
      }
   }

   if (lower_bound > upper_bound)
   {
      int swap = lower_bound;

      lower_bound = upper_bound;
      upper_bound = swap;
   }

   set_pager_color(AT_PLAIN, ch);
   send_to_pager(header_string1, ch);
   send_to_pager(header_string2, ch);

   for (pArea = first_area_name; pArea; pArea = pArea->next_sort_name)
   {
      if (pArea->hi_soft_range >= lower_bound && pArea->low_soft_range <= upper_bound)
      {
         pager_printf(ch, print_string,
            pArea->author, pArea->name, pArea->low_soft_range, pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range);
      }
   }
   return;
}

void do_afk(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
      return;

   if xIS_SET
      (ch->act, PLR_AFK)
   {
      xREMOVE_BIT(ch->act, PLR_AFK);
      send_to_char("You are no longer afk.\n\r", ch);
/*	act(AT_GREY,"$n is no longer afk.", ch, NULL, NULL, TO_ROOM);*/
      act(AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_CANSEE);
   }
   else
   {
      xSET_BIT(ch->act, PLR_AFK);
      send_to_char("You are now afk.\n\r", ch);
/*	act(AT_GREY,"$n is now afk.", ch, NULL, NULL, TO_ROOM);*/
      act(AT_GREY, "$n is now afk.", ch, NULL, NULL, TO_CANSEE);
      return;
   }

}

void do_slist(CHAR_DATA * ch, char *argument)
{
   do_listgroups(ch, "allgroups");
   send_to_char("_____Please use listgroups instead of slist._____\n\r", ch);
   return;
}

void do_whois(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char buf[MSL];
   char buf2[MSL];
   char wbuf[MSL];

   buf[0] = '\0';

   if (IS_NPC(ch))
      return;

   if (argument[0] == '\0')
   {
      send_to_pager("You must input the name of an online character.\n\r", ch);
      return;
   }

   strcat(buf, "0.");
   strcat(buf, argument);
   if (((victim = get_char_world(ch, buf)) == NULL))
   {
      send_to_pager("No such character online.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_pager("That's not a player!\n\r", ch);
      return;
   }
   sprintf(wbuf, "%s%s %s", victim->pcdata->pretit, victim->name, victim->last_name);
   if (str_cmp(PERS_MAP(victim, ch), wbuf))
   {
      send_to_char("You cannot finger a player you do not know.\n\r", ch);
      return;
   }
   set_pager_color(AT_GREY, ch);
   pager_printf(ch, "\n\r %s&c&w%s %s%s.\n\r", show_pers_pretitle(victim, ch), PERS_MAP(victim, ch), victim->last_name, show_pers_title(victim, ch));

   set_pager_color(AT_GREY, ch);

   pager_printf(ch, " %s is a %s %s, %d years of age.\n\r",
      victim->sex == SEX_MALE ? "He" :
      victim->sex == SEX_FEMALE ? "She" : "It",
      victim->sex == SEX_MALE ? "male" :
      victim->sex == SEX_FEMALE ? "female" : "neutral", capitalize(print_npc_race(victim->race)), get_age(victim));


   set_pager_color(AT_GREY, ch);

   pager_printf(ch, " %s is a nondeadly player", victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It");

   if (victim->pcdata->clan)
   {
      if (victim->pcdata->clan->clan_type == CLAN_ORDER)
         send_to_pager(", and belongs to the Order ", ch);
      else if (victim->pcdata->clan->clan_type == CLAN_GUILD)
         send_to_pager(", and belongs to the ", ch);
      else
         send_to_pager(", and belongs to Clan ", ch);
      send_to_pager(victim->pcdata->clan->name, ch);
   }
   send_to_pager(".\n\r", ch);
   if (is_player_kingdom(victim->pcdata->hometown))
   {
      pager_printf(ch, " %s belongs to the Kingdom of %s.\n\r",
         victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", kingdom_table[victim->pcdata->hometown]->name);
   }
   if (victim->pcdata->town)
   {
      pager_printf(ch, " %s belongs to the Town of %s.\n\r",
         victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", victim->pcdata->town->name);
   }

   set_pager_color(AT_GREY, ch);
   if (victim->pcdata->council)
      pager_printf(ch, " %s holds a seat on:  %s\n\r",
         victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", victim->pcdata->council->name);
   set_pager_color(AT_GREY, ch);
   if (victim->pcdata->deity)
      pager_printf(ch, " %s has found succor in the deity %s.\n\r",
         victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", victim->pcdata->deity->name);
   set_pager_color(AT_GREY, ch);
   if (victim->pcdata->homepage && victim->pcdata->homepage[0] != '\0')
      pager_printf(ch, " %s homepage can be found at %s\n\r",
         victim->sex == SEX_MALE ? "His" : victim->sex == SEX_FEMALE ? "Her" : "Its", show_tilde(victim->pcdata->homepage));
   if (victim->pcdata->bio && victim->pcdata->bio[0] != '\0')
      pager_printf(ch, " %s's personal bio:\n\r%s", PERS_MAP(victim, ch), victim->pcdata->bio);
   else
      pager_printf(ch, " %s has yet to create a bio.\n\r", PERS_MAP(victim, ch));
   set_pager_color(AT_GREY, ch);
   if (IS_IMMORTAL(ch))
   {
      send_to_pager("-------------------\n\r", ch);
      send_to_pager("Info for immortals:\n\r", ch);

      if (victim->pcdata->authed_by && victim->pcdata->authed_by[0] != '\0')
         pager_printf(ch, "%s was authorized by %s.\n\r", PERS_MAP(victim, ch), victim->pcdata->authed_by);

      pager_printf(ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\n\r",
         PERS_MAP(victim, ch), victim->pcdata->mkills, victim->pcdata->mdeaths);
      if (victim->pcdata->pkills || victim->pcdata->pdeaths)
         pager_printf(ch, "%s has killed %d players, and been killed by a player %d times.\n\r",
            PERS_MAP(victim, ch), victim->pcdata->pkills, victim->pcdata->pdeaths);
      if (victim->pcdata->illegal_pk)
         pager_printf(ch, "%s has committed %d illegal player kills.\n\r", PERS_MAP(victim, ch), victim->pcdata->illegal_pk);

      pager_printf(ch, "%s is %shelled at the moment.\n\r", PERS_MAP(victim, ch), (victim->pcdata->release_date == 0) ? "not " : "");

      if (victim->pcdata->nuisance)
      {
         pager_printf_color(ch, "&RNuisance   &cStage: (&R%d&c/%d)  Power:  &w%d  &cTime:  &w%s.\n\r", victim->pcdata->nuisance->flags,
            MAX_NUISANCE_STAGE, victim->pcdata->nuisance->power, ctime(&victim->pcdata->nuisance->time));
      }
      if (victim->pcdata->release_date != 0)
         pager_printf(ch, "%s was helled by %s, and will be released on %24.24s.\n\r",
            victim->sex == SEX_MALE ? "He" :
            victim->sex == SEX_FEMALE ? "She" : "It", victim->pcdata->helled_by, ctime(&victim->pcdata->release_date));

      if (get_trust(victim) < get_trust(ch))
      {
         sprintf(buf2, "list %s", buf);
         do_comment(ch, buf2);
      }

      if (xIS_SET(victim->act, PLR_SILENCE) || xIS_SET(victim->act, PLR_NO_EMOTE)
         || xIS_SET(victim->act, PLR_NO_TELL) || xIS_SET(victim->act, PLR_THIEF) || xIS_SET(victim->act, PLR_KILLER))
      {
         sprintf(buf2, "This player has the following flags set:");
         if (xIS_SET(victim->act, PLR_SILENCE))
            strcat(buf2, " silence");
         if (xIS_SET(victim->act, PLR_NO_EMOTE))
            strcat(buf2, " noemote");
         if (xIS_SET(victim->act, PLR_NO_TELL))
            strcat(buf2, " notell");
         if (xIS_SET(victim->act, PLR_THIEF))
            strcat(buf2, " thief");
         if (xIS_SET(victim->act, PLR_KILLER))
            strcat(buf2, " killer");
         strcat(buf2, ".\n\r");
         send_to_pager(buf2, ch);
      }
      if (victim->desc && victim->desc->host[0] != '\0') /* added by Gorog */
      {
         sprintf(buf2, "%s's IP info: %s ", PERS_MAP(victim, ch), victim->desc->host);
         if (get_trust(ch) >= LEVEL_STAFF) /* Tracker1 */
            strcat(buf2, victim->desc->user);
         strcat(buf2, "\n\r");
         send_to_pager(buf2, ch);
      }
   }
}

void do_pager(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   if (IS_NPC(ch))
      return;
   set_char_color(AT_NOTE, ch);
   argument = one_argument(argument, arg);
   if (!*arg)
   {
      if (IS_SET(ch->pcdata->flags, PCFLAG_PAGERON))
      {
         send_to_char("Pager disabled.\n\r", ch);
         do_config(ch, "-pager");
      }
      else
      {
         ch_printf(ch, "Pager is now enabled at %d lines.\n\r", ch->pcdata->pagerlen);
         do_config(ch, "+pager");
      }
      return;
   }
   if (!is_number(arg))
   {
      send_to_char("Set page pausing to how many lines?\n\r", ch);
      return;
   }
   ch->pcdata->pagerlen = atoi(arg);
   if (ch->pcdata->pagerlen < 5)
      ch->pcdata->pagerlen = 5;
   ch_printf(ch, "Page pausing set to %d lines.\n\r", ch->pcdata->pagerlen);
   return;
}

/*
 * The ignore command allows players to ignore up to MAX_IGN
 * other players. Players may ignore other characters whether
 * they are online or not. This is to prevent people from
 * spamming someone and then logging off quickly to evade
 * being ignored.
 * Syntax:
 *	ignore		-	lists players currently ignored
 *	ignore none	-	sets it so no players are ignored
 *	ignore <player>	-	start ignoring player if not already
 *				ignored otherwise stop ignoring player
 *	ignore reply	-	start ignoring last player to send a
 *				tell to ch, to deal with invis spammers
 * Last Modified: June 26, 1997
 * - Fireblade
 */
void do_ignore(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   IGNORE_DATA *temp, *next;
   char fname[1024];
   struct stat fst;
   CHAR_DATA *victim;

   if (IS_NPC(ch))
      return;

   argument = one_argument(argument, arg);

   sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), capitalize(arg));

   victim = NULL;

   /* If no arguements, then list players currently ignored */
   if (arg[0] == '\0')
   {
      set_char_color(AT_DIVIDER, ch);
      ch_printf(ch, "\n\r----------------------------------------\n\r");
      set_char_color(AT_DGREEN, ch);
      ch_printf(ch, "You are currently ignoring:\n\r");
      set_char_color(AT_DIVIDER, ch);
      ch_printf(ch, "----------------------------------------\n\r");
      set_char_color(AT_IGNORE, ch);

      if (!ch->pcdata->first_ignored)
      {
         ch_printf(ch, "\t    no one\n\r");
         return;
      }

      for (temp = ch->pcdata->first_ignored; temp; temp = temp->next)
      {
         ch_printf(ch, "\t  - %s\n\r", temp->name);
      }

      return;
   }
   /* Clear players ignored if given arg "none" */
   else if (!strcmp(arg, "none"))
   {
      for (temp = ch->pcdata->first_ignored; temp; temp = next)
      {
         next = temp->next;
         UNLINK(temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev);
         STRFREE(temp->name);
         DISPOSE(temp);
      }

      set_char_color(AT_IGNORE, ch);
      ch_printf(ch, "You now ignore no one.\n\r");

      return;
   }
   /* Prevent someone from ignoring themself... */
   else if (!strcmp(arg, "self") || nifty_is_name(arg, ch->name))
   {
      set_char_color(AT_IGNORE, ch);
      ch_printf(ch, "Did you type something?\n\r");
      return;
   }
   else
   {
      int i;

      /* get the name of the char who last sent tell to ch */
      if (!strcmp(arg, "reply"))
      {
         if (!ch->reply)
         {
            set_char_color(AT_IGNORE, ch);
            ch_printf(ch, "They're not here.\n\r");
            return;
         }
         else
         {
            strcpy(arg, ch->reply->name);
         }
      }

      /* Loop through the linked list of ignored players */
      /*  keep track of how many are being ignored     */
      for (temp = ch->pcdata->first_ignored, i = 0; temp; temp = temp->next, i++)
      {
         /* If the argument matches a name in list remove it */
         if (!strcmp(temp->name, capitalize(arg)))
         {
            UNLINK(temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev);
            set_char_color(AT_IGNORE, ch);
            ch_printf(ch, "You no longer ignore %s.\n\r", temp->name);
            STRFREE(temp->name);
            DISPOSE(temp);
            return;
         }
      }

      /* if there wasn't a match check to see if the name   */
      /* is valid. This if-statement may seem like overkill */
      /* but it is intended to prevent people from doing the */
      /* spam and log thing while still allowing ya to      */
      /* ignore new chars without pfiles yet...             */
      if (stat(fname, &fst) == -1 && (!(victim = get_char_world(ch, arg)) || IS_NPC(victim) || strcmp(capitalize(arg), PERS(victim, ch)) != 0))
      {
         set_char_color(AT_IGNORE, ch);
         ch_printf(ch, "No player exists by that" " name.\n\r");
         return;
      }

      if (victim)
      {
         strcpy(capitalize(arg), PERS(victim, ch));
      }

      /* If its valid and the list size limit has not been */
      /* reached create a node and at it to the list      */
      if (i < MAX_IGN)
      {
         IGNORE_DATA *new;

         CREATE(new, IGNORE_DATA, 1);
         new->name = STRALLOC(capitalize(arg));
         new->next = NULL;
         new->prev = NULL;
         LINK(new, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev);
         set_char_color(AT_IGNORE, ch);
         ch_printf(ch, "You now ignore %s.\n\r", new->name);
         return;
      }
      else
      {
         set_char_color(AT_IGNORE, ch);
         ch_printf(ch, "You may only ignore %d players.\n\r", MAX_IGN);
         return;
      }
   }
}

/*
 * This function simply checks to see if ch is ignoring ign_ch.
 * Last Modified: October 10, 1997
 * - Fireblade
 */
bool is_ignoring(CHAR_DATA * ch, CHAR_DATA * ign_ch)
{
   IGNORE_DATA *temp;

   if (IS_NPC(ch) || IS_NPC(ign_ch))
      return FALSE;

   for (temp = ch->pcdata->first_ignored; temp; temp = temp->next)
   {
      if (nifty_is_name(temp->name, ign_ch->name))
         return TRUE;
   }

   return FALSE;
}

void do_imbue(CHAR_DATA * ch, char *argument)
{
    char mana_arg[MIL];
    char item_arg[MIL];
    ROOM_INDEX_DATA *room;
    OBJ_DATA *obj;
    
    room = ch->in_room;
        
    if(argument[0] == '\0')
    {
        send_to_char("Syntax: imbue <amount> [item]\n\r", ch);
        send_to_char("imbue with amount and no item specified will release mana into the room's\n\r", ch);
        send_to_char("node, if there is one.\n\r", ch);
        return;
    }
       
    argument = one_argument(argument, mana_arg);
/*
    if(mana_arg[0] == '\0');
    {
        send_to_char("Set a mana amount.\n\r", ch);
        return;
    }
*/    
    argument = one_argument(argument, item_arg);
    
    if(item_arg[0] == '\0')
    {
        if(!isdigit(mana_arg[0]) )
        {
            send_to_char("That is not an amount!!\r\n", ch);
            return;
        }
        
        if(atoi(mana_arg) > ch->mana)
        {
            send_to_char("You don't have that much mana!\r\n", ch);
            return;
        }
        
        if(atoi(mana_arg) == 0)
        {
            send_to_char("If you have no mana, how can you release it?\r\n", ch);
            return;
        }
        
        if(!xIS_SET(room->room_flags, ROOM_MANANODE))
        {
            ch->mana -= atoi(mana_arg);
            send_to_char("You release some mana...\n\r", ch);
            act(AT_ACTION, "$n releases some mana...", ch, NULL, NULL, TO_ROOM);
            return;
        }
        if(xIS_SET(room->room_flags, ROOM_MANANODE))
        {
            ch->mana -= atoi(mana_arg);
            room->node_mana += atoi(mana_arg);
            send_to_char("You release some mana, and watch as it gets absorbed into the very air.\n\r", ch);
            act(AT_ACTION, "$n releases some mana and you watch as it gets absorbed into the very air.", ch, NULL, NULL, TO_ROOM);
            
            return;
        }
    }
    if(item_arg[0] != '\0')
    {
        for (obj = ch->first_carrying; obj; obj = obj->next_content)
         {
            if (!str_cmp(item_arg, obj->name))
               break;
         }
         
         for (obj = ch->first_carrying; obj; obj = obj->next_content)
         {
            if (nifty_is_name(item_arg, obj->name))
               break;   
         }
         
         for (obj = ch->first_carrying; obj; obj = obj->next_content)
         {
            if (nifty_is_name_prefix(item_arg, obj->name))   
               break;
         }
         
         if (!obj)
         {
            send_to_char("You are not carrying that.\n\r", ch);
            return;
         }
         if (obj->wear_loc != WEAR_NONE)
         {
            send_to_char("You can only imbue items you are not wearing.\n\r", ch);
            return;
         }
         if(!xIS_SET(obj->extra_flags, ITEM_IMBUABLE))
         {
            send_to_char("That is not imbuable!!\n\r", ch);
            return;
         }
         if(atoi(mana_arg) > ch->mana)
         {
            send_to_char("You do not have enough mana!\n\r", ch);
            return;
         }
         if(atoi(mana_arg) > obj->value[10])
         {
            send_to_char("It can not hold that much!\n\r", ch);
            return;
         }
         if(obj->value[9] >= obj->value[10])
         {
            send_to_char("It's already full, you can not imbue it any more!\n\r", ch);
            return;
         }
         if(atoi(mana_arg) <= (obj->value[10] - obj->value[9]) )
         {
            act(AT_MAGIC, "You imbue $p with mana!\n\r", ch, obj, NULL, TO_CHAR);
            act(AT_MAGIC, "$n imbues $p with mana!\n\r", ch, obj, NULL, TO_ROOM);
            obj->value[9] += atoi(mana_arg);
            ch->mana -= atoi(mana_arg);
            
         }
         
    }
    

}

/**********************************************************************
*   Function to format big numbers, so you can easy understand it.    *
*    Added by Desden, el Chaman Tibetano (J.L.Sogorb) in Oct-1998     *
*                Email: jlalbatros@mx2.redestb.es                     *
*                                                                                     * 
**********************************************************************/

char *punct(int foo)
{
   int index, index_new, rest;
   char buf[16];
   static char buf_new[16];

   strcpy(buf_new, "");
   sprintf(buf, "%d", foo);
   rest = strlen(buf) % 3;

   for (index = index_new = 0; index < strlen(buf); index++, index_new++)
   {
      if (index != 0 && (index - rest) % 3 == 0)
      {
         buf_new[index_new] = '.';
         index_new++;
         buf_new[index_new] = buf[index];
      }
      else
         buf_new[index_new] = buf[index];
   }
   buf_new[index_new] = '\0';
   return buf_new;
}

/* 
 * Title    : Help Check Plus v1.0
 * Author   : Chris Coulter (aka Gabriel Androctus)
 * MUD Page : http://www.perils.org/
 * Res. Page: http://www.perils.org/goodies/
 * Descr.   : A ridiculously simple routine that runs through the command and skill tables
 *            checking for help entries of the same name. If not found, it outputs a line
 *            to the pager. Priceless tool for finding those pesky missing help entries.
 */ 
void do_helpcheck( CHAR_DATA *ch, char *argument )
{
  CMDTYPE *command;
  HELP_DATA *help;
  int hash, sn;
  int total = 0;
  bool fSkills  = FALSE;
  bool fCmds    = FALSE;

  if ( argument[0] == '\0' )
  {
    set_pager_color( AT_YELLOW, ch );
    send_to_pager( "Syntax: helpcheck [ skills | commands | all ]\n\r", ch );
    return;
  }

  /* check arguments and set appropriate switches */
  if ( !str_cmp( argument, "skills" ) )
    fSkills = TRUE;
  if ( !str_cmp( argument, "commands" ) )
    fCmds = TRUE;
  if ( !str_cmp( argument, "all" ) )
  {
    fSkills = TRUE;
    fCmds = TRUE;
  }

  if ( fCmds ) /* run through command table */
  {
    for ( hash = 0; hash < 126; hash++ )
      for ( command = command_hash[hash]; command; command = command->next )
      {
         if ( !( help = get_help( ch, command->name ) ) ) /* no help entry */
         {
           pager_printf_color( ch, "&cNot found: &C%s&w\n\r", command->name );
           total++;
         }
         else
           continue;
      }
  }

  if ( fSkills ) /* run through skill table */
  {
    for ( sn = 0; sn < top_sn; sn++ )
    {
      if ( !( help = get_help( ch, skill_table[sn]->name ) ) ) /* no help entry */
      {
        pager_printf_color( ch, "&gNot found: &G%s&w\n\r", skill_table[sn]->name );
        total++;
      }
      else
        continue;
    }
  }

  /* tally up the total number of missing entries and finish up */
  pager_printf_color( ch, "\n\r&Y%d missing entries found.&w\n\r", total );
  return;
}
