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
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/*
 *  Locals
 */
char *tiny_affect_loc_name(int location);

void do_gold(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
      return;
   set_char_color(AT_GOLD, ch);
   ch_printf(ch, "You have %s gold\n\r", punct(ch->gold));
   ch_printf(ch, "You have %s gold in the bank\n\r", punct(ch->pcdata->balance));
   return;
}


void do_worth(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];

   if (IS_NPC(ch))
      return;

   set_pager_color(AT_SCORE, ch);
   pager_printf(ch, "\n\rWorth for %s%s.\n\r", ch->name, ch->pcdata->title);
   send_to_pager(" ----------------------------------------------------------------------------\n\r", ch);
   if (!ch->pcdata->deity)
      sprintf(buf, "N/A");
   else if (ch->pcdata->favor > 2250)
      sprintf(buf, "loved");
   else if (ch->pcdata->favor > 2000)
      sprintf(buf, "cherished");
   else if (ch->pcdata->favor > 1750)
      sprintf(buf, "honored");
   else if (ch->pcdata->favor > 1500)
      sprintf(buf, "praised");
   else if (ch->pcdata->favor > 1250)
      sprintf(buf, "favored");
   else if (ch->pcdata->favor > 1000)
      sprintf(buf, "respected");
   else if (ch->pcdata->favor > 750)
      sprintf(buf, "liked");
   else if (ch->pcdata->favor > 250)
      sprintf(buf, "tolerated");
   else if (ch->pcdata->favor > -250)
      sprintf(buf, "ignored");
   else if (ch->pcdata->favor > -750)
      sprintf(buf, "shunned");
   else if (ch->pcdata->favor > -1000)
      sprintf(buf, "disliked");
   else if (ch->pcdata->favor > -1250)
      sprintf(buf, "dishonored");
   else if (ch->pcdata->favor > -1500)
      sprintf(buf, "disowned");
   else if (ch->pcdata->favor > -1750)
      sprintf(buf, "abandoned");
   else if (ch->pcdata->favor > -2000)
      sprintf(buf, "despised");
   else if (ch->pcdata->favor > -2250)
      sprintf(buf, "hated");
   else
      sprintf(buf, "damned");
      
   pager_printf(ch, "|Favor: %-10s\n\r", buf);
   send_to_pager(" ----------------------------------------------------------------------------\n\r", ch);
   switch (ch->style)
   {
      case STYLE_EVASIVE:
         sprintf(buf, "evasive");
         break;
      case STYLE_DIVINE:
         sprintf(buf, "divine");
         break;
      case STYLE_WIZARDRY:
         sprintf(buf, "wizardry");
         break;
      case STYLE_DEFENSIVE:
         sprintf(buf, "defensive");
         break;
      case STYLE_AGGRESSIVE:
         sprintf(buf, "aggressive");
         break;
      case STYLE_BERSERK:
         sprintf(buf, "berserk");
         break;
      default:
         sprintf(buf, "standard");
         break;
   }
   pager_printf(ch, "|Glory: %-4d |Weight: %-9.2f |Style: %-13s |Gold: %-14d |\n\r", ch->pcdata->quest_curr, get_ch_carry_weight(ch), buf, ch->gold);
   send_to_pager(" ----------------------------------------------------------------------------\n\r", ch);
   return;
}

void do_delet(CHAR_DATA * ch, char *argument)
{
   send_to_char("If you want to DELETE, spell it out.\n\r", ch);
   return;
}

void do_delete(CHAR_DATA * ch, char *argument)
{
   send_to_char("You delete players at the account menu now.\n\r", ch);
   return;
}

/*
 * New score command by Haus
 */
void do_newscore(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   AFFECT_DATA *paf;
   int iLang;

   if (IS_NPC(ch))
   {
      do_oldscore(ch, argument);
      return;
   }
   set_pager_color(AT_SCORE, ch);

   pager_printf(ch, "\n\rScore for %s%s.\n\r", ch->name, ch->pcdata->title);
   if (get_trust(ch) != ch->level)
      pager_printf(ch, "You are trusted at level %d.\n\r", get_trust(ch));

   send_to_pager("----------------------------------------------------------------------------\n\r", ch);

   pager_printf(ch, "LEVEL: %-3d         Race : %-10.10s        Played: %d hours\n\r", ch->level, capitalize(get_race(ch)), (get_age(ch) - 17) * 20);

   pager_printf(ch, "YEARS: %-6d      Log In: %s\r", get_age(ch), ctime(&(ch->pcdata->logon)));

   if (ch->level >= 7)
   {
      pager_printf(ch, "STR  : %2.2d(%2.2d)    HitRoll: %-4d              Saved:  %s\r",
         get_curr_str(ch), ch->perm_str, GET_HITROLL(ch), ch->pcdata->save_time ? ctime(&(ch->pcdata->save_time)) : "no save this session\n");

      pager_printf(ch, "INT  : %2.2d(%2.2d)    DamRoll: %-4d              Time:   %s\r",
         get_curr_int(ch), ch->perm_int, GET_DAMROLL(ch), ctime(&current_time));
   }
   else
   {
      pager_printf(ch, "STR  : %2.2d(%2.2d)                               Saved:  %s\r",
         get_curr_str(ch), ch->perm_str, ch->pcdata->save_time ? ctime(&(ch->pcdata->save_time)) : "no\n");

      pager_printf(ch, "INT  : %2.2d(%2.2d)                               Time:   %s\r", get_curr_int(ch), ch->perm_int, ctime(&current_time));
   }

   if (GET_AC(ch) >= 101)
      sprintf(buf, "the rags of a beggar");
   else if (GET_AC(ch) >= 80)
      sprintf(buf, "improper for adventure");
   else if (GET_AC(ch) >= 55)
      sprintf(buf, "shabby and threadbare");
   else if (GET_AC(ch) >= 40)
      sprintf(buf, "of poor quality");
   else if (GET_AC(ch) >= 20)
      sprintf(buf, "scant protection");
   else if (GET_AC(ch) >= 10)
      sprintf(buf, "that of a knave");
   else if (GET_AC(ch) >= 0)
      sprintf(buf, "moderately crafted");
   else if (GET_AC(ch) >= -10)
      sprintf(buf, "well crafted");
   else if (GET_AC(ch) >= -20)
      sprintf(buf, "the envy of squires");
   else if (GET_AC(ch) >= -40)
      sprintf(buf, "excellently crafted");
   else if (GET_AC(ch) >= -60)
      sprintf(buf, "the envy of knights");
   else if (GET_AC(ch) >= -80)
      sprintf(buf, "the envy of barons");
   else if (GET_AC(ch) >= -100)
      sprintf(buf, "the envy of dukes");
   else if (GET_AC(ch) >= -200)
      sprintf(buf, "the envy of emperors");
   else
      sprintf(buf, "that of an avatar");
   if (ch->level > 8)
      pager_printf(ch, "WIS  : %2.2d(%2.2d)      Armor: %4.4d, %s\n\r", get_curr_wis(ch), ch->perm_wis, GET_AC(ch), buf);
   else
      pager_printf(ch, "WIS  : %2.2d(%2.2d)      Armor: %s \n\r", get_curr_wis(ch), ch->perm_wis, buf);

   if (ch->alignment > 900)
      sprintf(buf, "devout");
   else if (ch->alignment > 700)
      sprintf(buf, "noble");
   else if (ch->alignment > 350)
      sprintf(buf, "honorable");
   else if (ch->alignment > 100)
      sprintf(buf, "worthy");
   else if (ch->alignment > -100)
      sprintf(buf, "neutral");
   else if (ch->alignment > -350)
      sprintf(buf, "base");
   else if (ch->alignment > -700)
      sprintf(buf, "evil");
   else if (ch->alignment > -900)
      sprintf(buf, "ignoble");
   else
      sprintf(buf, "fiendish");
   if (ch->level < 5)
      pager_printf(ch, "DEX  : %2.2d(%2.2d)      Align: %-20.20s    Items: %5.5d   (max %5.5d)\n\r",
         get_curr_dex(ch), ch->perm_dex, buf, get_ch_carry_number(ch), can_carry_n(ch));
   else
      pager_printf(ch, "DEX  : %2.2d(%2.2d)      Align: %+4.4d, %-14.14s   Items: %5.5d   (max %5.5d)\n\r",
         get_curr_dex(ch), ch->perm_dex, ch->alignment, buf, get_ch_carry_number(ch), can_carry_n(ch));

   switch (ch->position)
   {
      case POS_DEAD:
         sprintf(buf, "slowly decomposing");
         break;
      case POS_MORTAL:
         sprintf(buf, "mortally wounded");
         break;
      case POS_INCAP:
         sprintf(buf, "incapacitated");
         break;
      case POS_STUNNED:
         sprintf(buf, "stunned");
         break;
      case POS_SLEEPING:
         sprintf(buf, "sleeping");
         break;
      case POS_RESTING:
         sprintf(buf, "resting");
         break;
      case POS_STANDING:
         sprintf(buf, "standing");
         break;
      case POS_FIGHTING:
         sprintf(buf, "fighting");
         break;
      case POS_EVASIVE:
         sprintf(buf, "fighting (evasive)"); /* Fighting style support -haus */
         break;
      case POS_DEFENSIVE:
         sprintf(buf, "fighting (defensive)");
         break;
      case POS_AGGRESSIVE:
         sprintf(buf, "fighting (aggressive)");
         break;
      case POS_BERSERK:
         sprintf(buf, "fighting (berserk)");
         break;
      case POS_MOUNTED:
         sprintf(buf, "mounted");
         break;
      case POS_RIDING:
         sprintf(buf, "riding");
         break;
      case POS_SITTING:
         sprintf(buf, "sitting");
         break;
   }
   pager_printf(ch, "CON  : %2.2d(%2.2d)      Pos'n: %-21.21s  Weight: %5.5d (max %7.7d)\n\r",
      get_curr_con(ch), ch->perm_con, buf, get_ch_carry_weight(ch), can_carry_w(ch));


   /*
    * Fighting style support -haus
    */
   pager_printf(ch, "CHA  : %2.2d(%2.2d)      Wimpy: %-5d      ", get_curr_cha(ch), ch->perm_cha, ch->wimpy);

   switch (ch->style)
   {
      case STYLE_EVASIVE:
         sprintf(buf, "evasive");
         break;
      case STYLE_DIVINE:
         sprintf(buf, "divine");
         break;
      case STYLE_WIZARDRY:
         sprintf(buf, "wizardry");
         break;
      case STYLE_DEFENSIVE:
         sprintf(buf, "defensive");
         break;
      case STYLE_AGGRESSIVE:
         sprintf(buf, "aggressive");
         break;
      case STYLE_BERSERK:
         sprintf(buf, "berserk");
         break;
      default:
         sprintf(buf, "standard");
         break;
   }
   pager_printf(ch, "Style: %-10.10s\n\r", buf);

   pager_printf(ch, "LCK  : %2.2d(%2.2d) \n\r", get_curr_lck(ch), ch->perm_lck);

   pager_printf(ch, "Glory: %4.4d(%4.4d)    Balance: %-10d\n\r", ch->pcdata->quest_curr, ch->pcdata->quest_accum, ch->pcdata->balance);

   pager_printf(ch, "PRACT: %3.3d         Hitpoints: %-5d of %5d   Pager: (%c) %3d    AutoExit(%c)\n\r",
      ch->practice, ch->hit, ch->max_hit,
      IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ', ch->pcdata->pagerlen, xIS_SET(ch->act, PLR_AUTOEXIT) ? 'X' : ' ');

   pager_printf(ch, "Mana: %-5d of %5d   MKills:  %-5.5d    AutoLoot(%c)\n\r",
      ch->mana, ch->max_mana, ch->pcdata->mkills, xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');

   pager_printf(ch, "GOLD : %-10d       Move: %-5d of %5d   Mdeaths: %-5.5d    AutoSac (%c)\n\r",
      ch->gold, ch->move, ch->max_move, ch->pcdata->mdeaths, xIS_SET(ch->act, PLR_AUTOSAC) ? 'X' : ' ');

   if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK] > 10)
      send_to_pager("You are drunk.\n\r", ch);
   if (!IS_NPC(ch) && ch->pcdata->condition[COND_THIRST] == 0)
      send_to_pager("You are in danger of dehydrating.\n\r", ch);
   if (!IS_NPC(ch) && ch->pcdata->condition[COND_FULL] == 0)
      send_to_pager("You are starving to death.\n\r", ch);
   if (ch->position != POS_SLEEPING)
      switch (ch->mental_state / 10)
      {
         default:
            send_to_pager("You're completely messed up!\n\r", ch);
            break;
         case -10:
            send_to_pager("You're barely conscious.\n\r", ch);
            break;
         case -9:
            send_to_pager("You can barely keep your eyes open.\n\r", ch);
            break;
         case -8:
            send_to_pager("You're extremely drowsy.\n\r", ch);
            break;
         case -7:
            send_to_pager("You feel very unmotivated.\n\r", ch);
            break;
         case -6:
            send_to_pager("You feel sedated.\n\r", ch);
            break;
         case -5:
            send_to_pager("You feel sleepy.\n\r", ch);
            break;
         case -4:
            send_to_pager("You feel tired.\n\r", ch);
            break;
         case -3:
            send_to_pager("You could use a rest.\n\r", ch);
            break;
         case -2:
            send_to_pager("You feel a little under the weather.\n\r", ch);
            break;
         case -1:
            send_to_pager("You feel fine.\n\r", ch);
            break;
         case 0:
            send_to_pager("You feel great.\n\r", ch);
            break;
         case 1:
            send_to_pager("You feel energetic.\n\r", ch);
            break;
         case 2:
            send_to_pager("Your mind is racing.\n\r", ch);
            break;
         case 3:
            send_to_pager("You can't think straight.\n\r", ch);
            break;
         case 4:
            send_to_pager("Your mind is going 100 miles an hour.\n\r", ch);
            break;
         case 5:
            send_to_pager("You're high as a kite.\n\r", ch);
            break;
         case 6:
            send_to_pager("Your mind and body are slipping apart.\n\r", ch);
            break;
         case 7:
            send_to_pager("Reality is slipping away.\n\r", ch);
            break;
         case 8:
            send_to_pager("You have no idea what is real, and what is not.\n\r", ch);
            break;
         case 9:
            send_to_pager("You feel immortal.\n\r", ch);
            break;
         case 10:
            send_to_pager("You are a Supreme Entity.\n\r", ch);
            break;
      }
   else if (ch->mental_state > 45)
      send_to_pager("Your sleep is filled with strange and vivid dreams.\n\r", ch);
   else if (ch->mental_state > 25)
      send_to_pager("Your sleep is uneasy.\n\r", ch);
   else if (ch->mental_state < -35)
      send_to_pager("You are deep in a much needed sleep.\n\r", ch);
   else if (ch->mental_state < -25)
      send_to_pager("You are in deep slumber.\n\r", ch);
   send_to_pager("Languages: ", ch);
   for (iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++)
      if (knows_language(ch, lang_array[iLang], ch) || (IS_NPC(ch) && ch->speaks == 0))
      {
         if (lang_array[iLang] & ch->speaking || (IS_NPC(ch) && !ch->speaking))
            set_pager_color(AT_RED, ch);
         send_to_pager(lang_names[iLang], ch);
         send_to_pager(" ", ch);
         set_pager_color(AT_SCORE, ch);
      }
   send_to_pager("\n\r", ch);

   if (ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0')
      pager_printf(ch, "You are bestowed with the command(s): %s.\n\r", ch->pcdata->bestowments);

   if (ch->morph && ch->morph->morph)
   {
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);
      if (IS_IMMORTAL(ch))
         pager_printf(ch, "Morphed as (%d) %s with a timer of %d.\n\r", ch->morph->morph->vnum, ch->morph->morph->short_desc, ch->morph->timer);
      else
         pager_printf(ch, "You are morphed into a %s.\n\r", ch->morph->morph->short_desc);
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);
   }
   send_to_pager("----------------------------------------------------------------------------\n\r", ch);
   pager_printf(ch, "PKILL DATA:  Pkills (%3.3d)     Illegal Pkills (%3.3d)     Pdeaths (%3.3d)\n\r",
      ch->pcdata->pkills, ch->pcdata->illegal_pk, ch->pcdata->pdeaths);
   if (ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD)
   {
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);
      pager_printf(ch, "CLAN STATS:  %-14.14s  Clan AvPkills : %-5d  Clan NonAvpkills : %-5d\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->pkills[6],
         (ch->pcdata->clan->pkills[1] + ch->pcdata->clan->pkills[2] +
ch->pcdata->clan->pkills[3] + ch->pcdata->clan->pkills[4] + ch->pcdata->clan->pkills[5]));
      pager_printf(ch, "                             Clan AvPdeaths: %-5d  Clan NonAvpdeaths: %-5d\n\r",
         ch->pcdata->clan->pdeaths[6],
         (ch->pcdata->clan->pdeaths[1] + ch->pcdata->clan->pdeaths[2] +
ch->pcdata->clan->pdeaths[3] + ch->pcdata->clan->pdeaths[4] + ch->pcdata->clan->pdeaths[5]));
   }
   if (ch->pcdata->deity)
   {
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);
      if (ch->pcdata->favor > 2250)
         sprintf(buf, "loved");
      else if (ch->pcdata->favor > 2000)
         sprintf(buf, "cherished");
      else if (ch->pcdata->favor > 1750)
         sprintf(buf, "honored");
      else if (ch->pcdata->favor > 1500)
         sprintf(buf, "praised");
      else if (ch->pcdata->favor > 1250)
         sprintf(buf, "favored");
      else if (ch->pcdata->favor > 1000)
         sprintf(buf, "respected");
      else if (ch->pcdata->favor > 750)
         sprintf(buf, "liked");
      else if (ch->pcdata->favor > 250)
         sprintf(buf, "tolerated");
      else if (ch->pcdata->favor > -250)
         sprintf(buf, "ignored");
      else if (ch->pcdata->favor > -750)
         sprintf(buf, "shunned");
      else if (ch->pcdata->favor > -1000)
         sprintf(buf, "disliked");
      else if (ch->pcdata->favor > -1250)
         sprintf(buf, "dishonored");
      else if (ch->pcdata->favor > -1500)
         sprintf(buf, "disowned");
      else if (ch->pcdata->favor > -1750)
         sprintf(buf, "abandoned");
      else if (ch->pcdata->favor > -2000)
         sprintf(buf, "despised");
      else if (ch->pcdata->favor > -2250)
         sprintf(buf, "hated");
      else
         sprintf(buf, "damned");
      pager_printf(ch, "Deity:  %-20s  Favor: %s\n\r", ch->pcdata->deity->name, buf);
   }
   if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER)
   {
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);
      pager_printf(ch, "Order:  %-20s  Order Mkills:  %-6d   Order MDeaths:  %-6d\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
   }
   if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD)
   {
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);
      pager_printf(ch, "Guild:  %-20s  Guild Mkills:  %-6d   Guild MDeaths:  %-6d\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
   }
   if (IS_IMMORTAL(ch))
   {
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);

      pager_printf(ch, "IMMORTAL DATA:  Wizinvis [%s]  Wizlevel (%d)\n\r", xIS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ", ch->pcdata->wizinvis);

      pager_printf(ch, "Bamfin:  %s %s\n\r", ch->name, (ch->pcdata->bamfin[0] != '\0') ? ch->pcdata->bamfin : "appears in a swirling mist.");
      pager_printf(ch, "Bamfout: %s %s\n\r", ch->name, (ch->pcdata->bamfout[0] != '\0') ? ch->pcdata->bamfout : "leaves in a swirling mist.");


      /* Area Loaded info - Scryn 8/11 */
      if (ch->pcdata->area)
      {
         pager_printf(ch, "Vnums:   Room (%-5.5d - %-5.5d)   Object (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)\n\r",
            ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
            ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum, ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
         pager_printf(ch, "Area Loaded [%s]\n\r", (IS_SET(ch->pcdata->area->status, AREA_LOADED)) ? "yes" : "no");
      }
   }
   if (ch->first_affect)
   {
      int i;
      SKILLTYPE *sktmp;

      i = 0;
      send_to_pager("----------------------------------------------------------------------------\n\r", ch);
      send_to_pager("AFFECT DATA:                            ", ch);
      for (paf = ch->first_affect; paf; paf = paf->next)
      {
         if ((sktmp = get_skilltype(paf->type)) == NULL)
            continue;
         if (ch->level < 10)
         {
            pager_printf(ch, "[%-34.34s]    ", sktmp->name);
            if (i == 0)
               i = 2;
            if ((++i % 3) == 0)
               send_to_pager("\n\r", ch);
         }
         if (ch->level >= 10)
         {
            if (paf->modifier == 0)
               pager_printf(ch, "[%-24.24s;%5d rds]    ", sktmp->name, paf->duration);
            else if (paf->modifier > 999)
               pager_printf(ch, "[%-15.15s; %7.7s;%5d rds]    ", sktmp->name, tiny_affect_loc_name(paf->location), paf->duration);
            else
               pager_printf(ch, "[%-11.11s;%+-3.3d %7.7s;%5d rds]    ",
                  sktmp->name, paf->modifier, tiny_affect_loc_name(paf->location), paf->duration);
            if (i == 0)
               i = 1;
            if ((++i % 2) == 0)
               send_to_pager("\n\r", ch);
         }
      }
   }
   send_to_pager("\n\r", ch);
   return;
}
char *get_rating(int stat, int base)
{
   
   if (base == 1000)
      stat *= 10;
   
   if (stat <= 1000)
      return "&G&W&R**********";
   else if (stat <= 2000)
      return "&G&W*&R*********";
   else if (stat <= 3000)
      return "&G&W**&R********";
   else if (stat <= 4000)
      return "&G&W***&R*******";
   else if (stat < 5000)
      return "&G&W****&R******";
   else if (stat <= 6000)
      return "&G&W*****&R*****";
   else if (stat <= 7000)
      return "&G&W******&R****";
   else if (stat <= 8000)
      return "&G&W*******&R***";
   else if (stat <= 9000)
      return "&G&W********&R**";
   else if (stat <= 10000)
      return "&G&W*********&R*";
   else
      return "&G&W**********";
}
//Growth Score
void do_gscore(CHAR_DATA *ch, char *argument)
{
   ch_printf(ch, "&c&wStrength:       %s\n\r", get_rating(ch->pcdata->per_str, 10000));
   ch_printf(ch, "&c&wWisdom:         %s\n\r", get_rating(ch->pcdata->per_wis, 10000));
   ch_printf(ch, "&c&wDexterity:      %s\n\r", get_rating(ch->pcdata->per_dex, 10000));
   ch_printf(ch, "&c&wConstitution:   %s\n\r", get_rating(ch->pcdata->per_con, 10000));
   ch_printf(ch, "&c&wIntelligence:   %s\n\r", get_rating(ch->pcdata->per_int, 10000));
   ch_printf(ch, "&c&wLuck:           %s\n\r", get_rating(ch->pcdata->per_lck, 10000));
   ch_printf(ch, "&c&wAgility:        %s\n\r", get_rating(ch->pcdata->per_agi, 1000));
   ch_printf(ch, "&c&wHealth:         %s\n\r", get_rating(ch->pcdata->per_hp, 1000));
   ch_printf(ch, "&c&wMana:           %s\n\r", get_rating(ch->pcdata->per_mana, 1000));
   ch_printf(ch, "&c&wMovement:       %s\n\r", get_rating(ch->pcdata->per_move, 1000));
   return;
}

int prv args((int value));
   
   
int get_bagility(CHAR_DATA *ch)
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
   int extra;
   
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
   //Two things for dual wield, a little much...
   ch->agi_meter = 1000 - (number_range(agi*7/2, agi*5));
   if (tweight > 0) 
   {
      tweight = tweight * 100 / str_app[get_curr_str(ch)].weight * 7;
      ch->agi_meter += number_range(tweight*7/10, tweight*1);
   } 
   
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_combatart] > 0 && weapon)
   {
      ch->agi_meter -= URANGE(6, POINT_LEVEL(LEARNED(ch, gsn_combatart), MASTERED(ch, gsn_combatart))*2/3, 50);
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_attack_frenzy] > 0 && weapon)
   {
      ch->agi_meter -= URANGE(15, POINT_LEVEL(LEARNED(ch, gsn_attack_frenzy), MASTERED(ch, gsn_attack_frenzy))*4/3, 100);
   }
   wtype = wielding_skill_weapon(ch, 0);
   if (!IS_NPC(ch) && wtype)
   {
      int wskill = wielding_skill_weapon(ch, 1);
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
      }
   }       
   extra = (timer * ch->agi_meter / 10) % 100;  
   timer = timer * ch->agi_meter / 1000;       
   return timer*100+extra;
}
   
void do_score(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char caste_name[MSL];
   char agimeter_name[MSL];
   int agimeter;
   int x;
   char element_name[MSL];
   AFFECT_DATA *paf;
   int hplayed = ((ch->played + (current_time - ch->pcdata->logon)) / 3600);

   if (IS_NPC(ch))
   {
      do_oldscore(ch, argument);
      return;
   }
   if (!str_cmp(argument, "more"))
   {
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);

      pager_printf_color(ch, "&g&w| &CMdeaths: &W%5d        &g&w| &CAutoSac (&W%c&C)           &g&w| &CMKills:  &W%5d           &g&w|\n\r",
         ch->pcdata->mdeaths, xIS_SET(ch->act, PLR_AUTOSAC) ? 'X' : ' ', ch->pcdata->mkills);

      pager_printf_color(ch, "&g&w| &CPager: (&W%c&C) &W%3d        &g&w| &CAutoExit(&W%c&C)           &g&w| &CAutoLoot(&W%c&C)              &c&w|\n\r",
         IS_SET(ch->pcdata->flags, PCFLAG_PAGERON) ? 'X' : ' ', ch->pcdata->pagerlen, xIS_SET(ch->act, PLR_AUTOEXIT) ? 'X' : ' ', xIS_SET(ch->act, PLR_AUTOLOOT) ? 'X' : ' ');

      pager_printf_color(ch, "&g&w| &CPkills (&W%-4d&C)         &g&w| &CPranking (&W%-3d&C)        &g&w| &CPdeaths (&W%-4d&C)           &c&w|\n\r",
         ch->pcdata->pkills, ch->pcdata->pranking, ch->pcdata->pdeaths);
      
      if (ch->level >= LEVEL_IMMORTAL || IS_AFFECTED(ch, AFF_DETECT_MAGIC))
      {
         send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
         pager_printf(ch, "&c&w| &w&CArmor:&w&W%-2d     &w&CStone:&w&W%-2d    &w&CTohit:&w&W%-2d     &w&CSanctify:&w&W%-2d  &w&CShield:&w&W%-2d    &w&CWmod:&w&W%-3d &c&w|\n\r",
            UMIN(10, ch->apply_armor), UMIN(6, ch->apply_stone), UMIN(10, ch->apply_tohit), UMIN(15, ch->apply_sanctify),
            UMIN(50, ch->apply_shield), ch->apply_wmod);
         pager_printf(ch, "&c&w| &w&CFasting:&w&W%-3d  &w&CMFuse:&w&W%-3d   &w&CMShell:&w&W%-3d   &w&CMShield:&w&W%-3d  &w&CMGuard:&w&W%-3d            &c&w|\n\r",
            ch->apply_fasting, ch->apply_manafuse, ch->apply_manashell, ch->apply_manashield, ch->apply_managuard);
         pager_printf(ch, "&c&w| &w&CMburn:&w&W%-2d     &w&CWClamp:&w&W%-3d  &w&CACatch:&w&W%-3d   &w&CBracing:&w&W%-3d  &w&CHardning:&w&W%-2d           &c&w|\n\r",
            UMIN(10, ch->apply_manaburn), ch->apply_weaponclamp, ch->apply_arrowcatch, ch->apply_bracing, 
            UMIN(4, ch->apply_hardening));
         pager_printf(ch, "&c&w| &w&CHRegen:&w&W%-3d   &w&CMRegen:&w&W%-3d                                                  &c&w|\n\r",
            UMIN(700, ch->hpgen), UMIN(700, ch->managen));
         send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);         
         pager_printf_color(ch, "&c&w| &w&CRFire:   &w&W%-3d  &w&CRWater:    &w&W%-3d  &w&CRAir:   &w&W%-3d  &w&CREarth:  &w&W%-3d  &w&CREnergy: &w&W%-3d    &c&w|\n\r",
            prv(ch->apply_res_fire[0]), prv(ch->apply_res_water[0]), prv(ch->apply_res_air[0]),  prv(ch->apply_res_earth[0]), 
            prv(ch->apply_res_energy[0]));
         pager_printf_color(ch, "&c&w| &w&CRMagic:  &w&W%-3d  &w&CRNonMagic: &w&W%-3d  &w&CRBlunt: &w&W%-3d  &w&CRPierce: &w&W%-3d  &w&CRSlash:  &w&W%-3d    &c&w|\n\r",
            prv(ch->apply_res_magic[0]),  prv(ch->apply_res_nonmagic[0]), prv(ch->apply_res_blunt[0]),  prv(ch->apply_res_pierce[0]), 
            prv(ch->apply_res_slash[0]));
         pager_printf_color(ch, "&c&w| &w&CRPoison: &w&W%-3d  &w&CRPara:     &w&W%-3d  &w&CRHoly:  &w&W%-3d  &w&CRUnholy: &w&W%-3d  &w&CRUndead: &w&W%-3d    &c&w|\n\r",
            prv(ch->apply_res_poison[0]),  prv(ch->apply_res_paralysis[0]), prv(ch->apply_res_holy[0]), prv(ch->apply_res_unholy[0]), 
            prv(ch->apply_res_undead[0]));
         pager_printf_color(ch, "&c&w| &w&CSPoiDea: &w&W%-3d  &w&CSParaPetr: &w&W%-3d  &w&CSWand:  &w&W%-3d  &w&CSBreath: &w&W%-3d  &w&CSSpell:  &w&W%-3d    &c&w|\n\r",
            ch->saving_poison_death, ch->saving_wand, ch->saving_para_petri, ch->saving_breath,
            ch->saving_spell_staff);
         send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
      }
      return;
   }   

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
   if (IS_SET(ch->elementb, ELEMENT_UNHOLY))
      sprintf(element_name, "&w&zUnholy");
   set_pager_color(AT_SCORE, ch);
   //ch_printf(ch, "%s", MXPTAG("FRAME Name=\"Map\" FLOATING Left=\"-55c\" Top=\"0\" Width=\"55c\" Height=\"21c\""));  
   //ch_printf(ch, "%s", MXPTAG("FRAME Name=\"Map\" FLOATING Left=\"-20c\" Top=\"0\" Width=\"20c\" Height=\"20c\""));
   //ch_printf(ch, "%s %s %s %s %s", MXPTAG("DEST Map EOF"),  MXPTAG("/DEST"), MXPTAG("DEST Map"), MXPTAG("Image notepad.jpg align=bottom"), MXPTAG("/DEST"));

   pager_printf_color(ch, "\n\r               &P%s%s%s.\n\r", ch->name, ch->pcdata->title, get_wear_hidden_cloak(ch) ? " (Cloaked)" : "");

   send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);

   pager_printf_color(ch, "&g&w| &CStrength:     &r[&G&W%2.2d/%2.2d&r] &g&w| &CHealth: &r[&G&W%-5d/%5d&r] &g&w| &CCaste:  &r[&G&W%14s&r] &c&w|\n\r",
      get_curr_str(ch), ch->perm_str, ch->hit, ch->max_hit, caste_name);

   switch (ch->style)
   {
      case STYLE_EVASIVE:
         sprintf(buf, "Evasive");
         break;
      case STYLE_DIVINE:
         sprintf(buf, "divine");
         break;
      case STYLE_WIZARDRY:
         sprintf(buf, "wizardry");
         break;
      case STYLE_DEFENSIVE:
         sprintf(buf, "Defensive");
         break;
      case STYLE_AGGRESSIVE:
         sprintf(buf, "Aggressive");
         break;
      case STYLE_BERSERK:
         sprintf(buf, "Berserk");
         break;
      default:
         sprintf(buf, "Standard");
         break;
   }
   pager_printf_color(ch,
      "&g&w| &CIntelligence: &r[&G&W%2.2d/%2.2d&r] &g&w| &CMana:   &r[&G&W%-5d/%5d&r] &g&w| &CStyle:  &r[&G&W  %10s  &r] &c&w|\n\r",
      get_curr_int(ch), ch->perm_int, ch->mana, ch->max_mana, buf);

   switch (ch->position)
   {
      case POS_DEAD:
         sprintf(buf, " Decomposing  ");
         break;
      case POS_MORTAL:
         sprintf(buf, "   Wounded    ");
         break;
      case POS_INCAP:
         sprintf(buf, " Knocked out ");
         break;
      case POS_STUNNED:
         sprintf(buf, "   Stunned    ");
         break;
      case POS_SLEEPING:
         sprintf(buf, "   Sleeping   ");
         break;
      case POS_RESTING:
         sprintf(buf, "   Resting    ");
         break;
      case POS_STANDING:
         sprintf(buf, "   Standing   ");
         break;
      case POS_FIGHTING:
         sprintf(buf, "   Fighting   ");
         break;
      case POS_EVASIVE:
         sprintf(buf, "   Evasive    "); /* Fighting style support -haus */
         break;
      case POS_DEFENSIVE:
         sprintf(buf, "  Defensive   ");
         break;
      case POS_AGGRESSIVE:
         sprintf(buf, "  Aggressive  ");
         break;
      case POS_BERSERK:
         sprintf(buf, "   Berserk    ");
         break;
      case POS_MOUNTED:
         sprintf(buf, "   Mounted    ");
         break;
      case POS_RIDING:
         sprintf(buf, "    Riding    ");
         break;
      case POS_SITTING:
         sprintf(buf, "   Sitting    ");
         break;
   }

   pager_printf_color(ch, "&g&w| &CWisdom:       &r[&G&W%2.2d/%2.2d&r] &g&w| &CMoves:  &r[&G&W%-5d/%5d&r] &g&w| &CPos:    &r[&G&W%-14s&r] &c&w|\n\r",
      get_curr_wis(ch), ch->perm_wis, ch->move, ch->max_move, buf);
      
   pager_printf_color(ch,
      "&g&w| &CDexterity:    &r[&G&W%2.2d/%2.2d&r] &g&w| &CGold:   &r[&G&W %-10.10s&r] &g&w| &CQPS:    &r[&G&W %-10d   &r] &c&w|\n\r",
      get_curr_dex(ch), ch->perm_dex, punct(ch->gold), ch->pcdata->quest_curr);

   pager_printf_color(ch,
      "&g&w| &CConstitution: &r[&G&W%2.2d/%2.2d&r] &g&w| &CBank:   &r[&G&W %-10.10s&r] &g&w| &CAQPS:   &r[&G&W %-10d   &r] &c&w|\n\r",
      get_curr_con(ch), ch->perm_con, punct(ch->pcdata->balance), ch->pcdata->quest_accum);

   pager_printf_color(ch,
      "&g&w| &CLuck:         &r[&G&W%2.2d/%2.2d&r] &g&w| &CAgi:    &r[&G&W %3d       &r] &c&w| &CHand:   &r[&G&W    %s     &r] &c&w|\n\r",
      get_curr_lck(ch), ch->perm_lck, get_curr_agi(ch), ch->pcdata->righthanded == 0 ? " Left" : "Right");
   
   pager_printf_color(ch, "&g&w| &CEndurance:    &r[ &G&W%3.3d &r] &g&w| &CKgndom: &r[&G&W %-10.10s&r] &g&w| &CTown:   &r[&G&W %-13.13s&r] &c&w|\n\r",
      ch->mover, kingdom_table[ch->pcdata->hometown]->name, ch->pcdata->town ? ch->pcdata->town->name : "None");
   if (sysdata.resetgame)
   {
      pager_printf_color(ch, "&g&w| &CPower Ranking:&r[&G&W%-5d&r] &g&w| &CSPoint: &r[&G&W %-9d &r] &g&w| &CSWorth: &r[ &G&W%12.12s &r]&c&w |\n\r",
         ch->pcdata->power_ranking, ch->pcdata->spoints, punct(player_stat_worth(ch)));
   }
   else
   {
      pager_printf_color(ch, "&g&w|                       &g&w| &CSPoint: &r[&G&W %-9d &r] &g&w| &CSWorth: &r[ &G&W%12.12s &r]&c&w |\n\r",
         ch->pcdata->spoints, punct(player_stat_worth(ch)));
   }
   if (sysdata.resetgame)
   {
      pager_printf_color(ch, "&g&w| &CTwink Points: &r[ &G&W%-3d &r] &g&w| &CPLevel: &r[&G&W %-9d &r] &g&w| &CEWorth: &r[ &G&W%12.12s &r]&c&w |\n\r",
         ch->pcdata->twink_points, get_player_statlevel(ch), punct(player_equipment_worth(ch)));
   }
   send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
   pager_printf_color(ch,
      "&g&w| &CElement:     &r[%s&r] &g&w| &CItems:  &r[&G&W%5d/%-5d&r] &c&w| &CWeight:&r[&G&W%8.2f/%-7d&r]&c&w|\n\r",
      element_name, get_ch_carry_number(ch), can_carry_n(ch), get_ch_carry_weight(ch), can_carry_w(ch));

   pager_printf_color(ch, "&g&w| &CWimpy:        &r[&G&W %4d&r] &g&w| &CRace:   &r[&G&W %-10s&r] &g&w| &CThirst: &r[&G&W     %3d      &r] &c&w|\n\r",
      ch->wimpy, capitalize(get_race(ch)), ch->pcdata->condition[COND_THIRST]);
   pager_printf_color(ch,
      "&g&w| &CAge:          &r[&G&W %4d&r] &g&w| &CGender: &r[&G&W %-7s   &r] &g&w| &CHunger: &r[&G&W     %3d      &r] &c&w|\n\r", get_age(ch),
      ch->sex == SEX_MALE ? "Male" : ch->sex == SEX_FEMALE ? "Female" : "Neither", ch->pcdata->condition[COND_FULL]);
   pager_printf_color(ch, "&g&w| &CHours:        &r[&G&W%5d&r] &g&w| &CReward: &r[&G&W%5d/%-5d&r] &c&w| &CDrunk:  &r[&G&W     %3d      &r] &c&w|\n\r",
      hplayed, ch->pcdata->reward_curr, ch->pcdata->reward_accum, ch->pcdata->condition[COND_DRUNK]);
   if (IS_IMMORTAL(ch))
   {
      pager_printf(ch, "&g&w| &CWizinvis [%s]          &g&w| &CWizlevel (%2d)         &g&w|                          &c&w|\n\r",
         xIS_SET(ch->act, PLR_WIZINVIS) ? "X" : " ", ch->pcdata->wizinvis);
   }
   send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);

   if (ch->con_rarm == -1)
      sprintf(caste_name, "&r*****");
   else if (ch->con_rarm <= 200)
      sprintf(caste_name, "&G&W*&r****");
   else if (ch->con_rarm <= 400)
      sprintf(caste_name, "&G&W**&r***");
   else if (ch->con_rarm <= 600)
      sprintf(caste_name, "&G&W***&r**");
   else if (ch->con_rarm <= 800)
      sprintf(caste_name, "&G&W****&r*");
   else
      sprintf(caste_name, "&G&W*****");
      
   pager_printf_color(ch, "&g&w| &CR-Arm %s ", caste_name);
   
   if (ch->con_larm == -1)
      sprintf(caste_name, "&r*****");
   else if (ch->con_larm <= 200)
      sprintf(caste_name, "&G&W*&r****");
   else if (ch->con_larm <= 400)
      sprintf(caste_name, "&G&W**&r***");
   else if (ch->con_larm <= 600)
      sprintf(caste_name, "&G&W***&r**");
   else if (ch->con_larm <= 800)
      sprintf(caste_name, "&G&W****&r*");
   else
      sprintf(caste_name, "&G&W*****");
      
   pager_printf_color(ch, "&CL-Arm %s ", caste_name);
   
   if (ch->con_rleg == -1)
      sprintf(caste_name, "&r*****");
   else if (ch->con_rleg <= 200)
      sprintf(caste_name, "&G&W*&r****");
   else if (ch->con_rleg <= 400)
      sprintf(caste_name, "&G&W**&r***");
   else if (ch->con_rleg <= 600)
      sprintf(caste_name, "&G&W***&r**");
   else if (ch->con_rleg <= 800)
      sprintf(caste_name, "&G&W****&r*");
   else
      sprintf(caste_name, "&G&W*****");
      
   pager_printf_color(ch, "&CR-Leg %s ", caste_name);
   
   if (ch->con_lleg == -1)
      sprintf(caste_name, "&r*****");
   else if (ch->con_lleg <= 200)
      sprintf(caste_name, "&G&W*&r****");
   else if (ch->con_lleg <= 400)
      sprintf(caste_name, "&G&W**&r***");
   else if (ch->con_lleg <= 600)
      sprintf(caste_name, "&G&W***&r**");
   else if (ch->con_lleg <= 800)
      sprintf(caste_name, "&G&W****&r*");
   else
      sprintf(caste_name, "&G&W*****");
      
   pager_printf_color(ch, "&CL-Leg %s ", caste_name);
   
   if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 20)
      sprintf(caste_name, "&G&W*****");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 40)
      sprintf(caste_name, "&G&W****&r*");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 60)
      sprintf(caste_name, "&G&W***&r**");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 80)
      sprintf(caste_name, "&G&W**&r***");
   else if (ch->pcdata->hit_cnt * 100 / get_sore_rate(ch->race, ch->max_hit) < 100)
      sprintf(caste_name, "&G&W*&r****");
   else
      sprintf(caste_name, "&w&r*****");
   
   pager_printf_color(ch, "&CHBurn %s ", caste_name);
   
   if (ch->pcdata->mana_cnt * 100 / 60 < 20)
      sprintf(caste_name, "&G&W*****");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 40)
      sprintf(caste_name, "&G&W****&r*");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 60)
      sprintf(caste_name, "&G&W***&r**");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 80)
      sprintf(caste_name, "&G&W**&r***");
   else if (ch->pcdata->mana_cnt * 100 / 60 < 100)
      sprintf(caste_name, "&G&W*&r****");
   else
      sprintf(caste_name, "&w&r*****");
   
   pager_printf_color(ch, "&CMBurn %s  &g&w|\n\r", caste_name);
   agimeter = get_bagility(ch);
   x = 50;
   
   strcpy(agimeter_name, "");
   for (;agimeter >= x && x < 2401;)
   {
      strcat(agimeter_name, "&w&P*");
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
   ch_printf(ch, "&g&w|  &CAgiMeter: %s  &g&w|\n\r", agimeter_name);
   send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);


   if (ch->pcdata->clan && ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD)
   {
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
      pager_printf_color(ch, "&g&w| &CCLAN:   &W%-14.14s&g&w| &CAvPkills:  &W%-5d      &g&w| &CNonAvpkills:  &W%-5d      &c&w|\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->pkills[5],
         (ch->pcdata->clan->pkills[0] + ch->pcdata->clan->pkills[1] + ch->pcdata->clan->pkills[2] + ch->pcdata->clan->pkills[3] +
            ch->pcdata->clan->pkills[4]));
      pager_printf_color(ch, "&g&w| &CAvPdeaths:   &W%-5d    &g&w| &CNonAvpdeaths: &W%-5d   &g&w|                          &c&w|\n\r",
         ch->pcdata->clan->pdeaths[5],
         (ch->pcdata->clan->pdeaths[0] + ch->pcdata->clan->pdeaths[1] + ch->pcdata->clan->pdeaths[2] + ch->pcdata->clan->pdeaths[3] +
            ch->pcdata->clan->pdeaths[4]));
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
   }

   if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_ORDER)
   {
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
      pager_printf_color(ch, "&g&w| &COrder: &W%-15s&g&w| &CMkills:  &W%-6d       &g&w| &CMDeaths:     &W%-6d      &c&w|\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);

   }
   if (ch->pcdata->clan && ch->pcdata->clan->clan_type == CLAN_GUILD)
   {
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
      pager_printf_color(ch, "&g&w| &CGuild: &W%-15s&g&w| &CMkills:    &W%-6d      &g&w| &CMDeaths:     &W%-6d     &c&w|\n\r",
         ch->pcdata->clan->name, ch->pcdata->clan->mkills, ch->pcdata->clan->mdeaths);
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
   }

   if (ch->position != POS_SLEEPING)
      switch (ch->mental_state / 10)
      {
         default:
            send_to_pager("&g&w| &GCondition:      &OYou're completely messed up!\n\r", ch);
            break;
         case -10:
            send_to_pager("&g&w| &GCondition:      &OYou're barely conscious.\n\r", ch);
            break;
         case -9:
            send_to_pager("&g&w| &GCondition:      &OYou can barely keep your eyes open.\n\r", ch);
            break;
         case -8:
            send_to_pager("&g&w| &GCondition:      &OYou're extremely drowsy.\n\r", ch);
            break;
         case -7:
            send_to_pager("&g&w| &GCondition:      &OYou feel very unmotivated.\n\r", ch);
            break;
         case -6:
            send_to_pager("&g&w| &GCondition:      &OYou feel sedated.\n\r", ch);
            break;
         case -5:
            send_to_pager("&g&w| &GCondition:      &OYou feel sleepy.\n\r", ch);
            break;
         case -4:
            send_to_pager("&g&w| &GCondition:      &OYou feel tired.\n\r", ch);
            break;
         case -3:
            send_to_pager("&g&w| &GCondition:      &OYou could use a rest.\n\r", ch);
            break;
         case -2:
            send_to_pager("&g&w| &GCondition:      &OYou feel a little under the weather.\n\r", ch);
            break;
         case -1:
            send_to_pager("&g&w| &GCondition:      &OYou feel fine.\n\r", ch);
            break;
         case 0:
            send_to_pager("&g&w| &GCondition:      &OYou feel great.\n\r", ch);
            break;
         case 1:
            send_to_pager("&g&w| &GCondition:      &OYou feel energetic.\n\r", ch);
            break;
         case 2:
            send_to_pager("&g&w| &GCondition:      &OYour mind is racing.\n\r", ch);
            break;
         case 3:
            send_to_pager("&g&w| &GCondition:      &OYou can't think straight.\n\r", ch);
            break;
         case 4:
            send_to_pager("&g&w| &GCondition:      &OYour mind is going 100 miles an hour.\n\r", ch);
            break;
         case 5:
            send_to_pager("&g&w| &GCondition:      &OYou're high as a kite.\n\r", ch);
            break;
         case 6:
            send_to_pager("&g&w| &GCondition:      &OYour mind and body are slipping apart.\n\r", ch);
            break;
         case 7:
            send_to_pager("&g&w| &GCondition:      &OReality is slipping away.\n\r", ch);
            break;
         case 8:
            send_to_pager("&g&w| &GCondition:      &OYou have no idea what is real, and what is not.\n\r", ch);
            break;
         case 9:
            send_to_pager("&g&w| &GCondition:      &OYou feel immortal.\n\r", ch);
            break;
         case 10:
            send_to_pager("&g&w| &GCondition:      &OYou are a Supreme Entity.\n\r", ch);
            break;
      }
   else if (ch->mental_state > 45)
      send_to_pager("&g&w| &GCondition:      &OYour sleep is filled with strange and vivid dreams.\n\r", ch);
   else if (ch->mental_state > 25)
      send_to_pager("&g&w| &GCondition:      &OYour sleep is uneasy.\n\r", ch);
   else if (ch->mental_state < -35)
      send_to_pager("&g&w| &GCondition:      &OYou are deep in a much needed sleep.\n\r", ch);
   else if (ch->mental_state < -25)
      send_to_pager("&g&w| &GCondition:      &OYou are in deep slumber.\n\r", ch);
/*  send_to_pager("Languages: ", ch );
    for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
	if ( knows_language( ch, lang_array[iLang], ch )
	||  (IS_NPC(ch) && ch->speaks == 0) )
	{
	    if ( lang_array[iLang] & ch->speaking
	    ||  (IS_NPC(ch) && !ch->speaking) )
		set_pager_color( AT_RED, ch );
	    send_to_pager( lang_names[iLang], ch );
	    send_to_pager( " ", ch );
	    set_pager_color( AT_SCORE, ch );
	}
    send_to_pager( "\n\r", ch );
*/
   if (ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0')
      pager_printf_color(ch, "&g&w| &CBestowment(s):  &Y%s\n\r", ch->pcdata->bestowments);

   if (IS_IMMORTAL(ch))
   {
      pager_printf(ch, "&g&w| &RBamfin:         &g%s\n\r", (ch->pcdata->bamfin[0] != '\0') ? ch->pcdata->bamfin : "");     

      //pager_printf(ch, "&g&w| &RBamfin:         &g%s %s\n\r", ch->name, 
//(ch->pcdata->bamfin[0] != '\0')
         //? ch->pcdata->bamfin : "&gappears in a swirling mist.");

      pager_printf(ch, "&g&w| &RBamfout:        &g%s\n\r", (ch->pcdata->bamfout[0] != '\0') ? ch->pcdata->bamfout : "");

      //pager_printf(ch, "&g&w| &RBamfout:        &g%s %s\n\r", ch->name, 
//(ch->pcdata->bamfout[0] != '\0')
         //? ch->pcdata->bamfout : "&gleaves in a swirling mist.");
   }

   /* Area Loaded info - Scryn 8/11 */
   if (ch->pcdata->area)
   {
      pager_printf(ch, "&g&w| &RVnums:          &WRoom (%-5.5d - %-5.5d)   Object (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)\n\r",
         ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
         ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum, ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum);
      pager_printf(ch, "&g&w| &RArea Loaded [%s]\n\r", (IS_SET(ch->pcdata->area->status, AREA_LOADED)) ? "yes" : "no");
   }
   send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);

   if (ch->morph && ch->morph->morph)
   {
      if (IS_IMMORTAL(ch))
         pager_printf(ch, "Morphed as (%d) %s with a timer of %d.\n\r", ch->morph->morph->vnum, ch->morph->morph->short_desc, ch->morph->timer);
      else
         pager_printf(ch, "You are morphed into a %s.\n\r", ch->morph->morph->short_desc);
      send_to_pager_color("&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r", ch);
   }

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
            pager_printf(ch, "[%-34.34s]    ", sktmp->name);
            if (i == 0)
               i = 2;
            if ((++i % 3) == 0)
               send_to_pager("\n\r", ch);
         }
         if (ch->level >= LEVEL_IMMORTAL || (IS_AFFECTED(ch, AFF_DETECT_MAGIC)))
         {
            sprintf(timeb, "M:%d S:%d", paf->duration/60, paf->duration%60);
            if (paf->modifier == 0)
               pager_printf(ch, "[%-24.24s&c&R|&G&W%-10.10s]  ", sktmp->name, timeb);
            else if (paf->modifier > 999)
               pager_printf(ch, "[%-15.15s&c&R|&G&W %7.7s&c&R|&G&W%-10.10s]  ", sktmp->name, tiny_affect_loc_name(paf->location), timeb);
            else
               pager_printf(ch, "[%-11.11s&c&R|&G&W%+-3.3d %7.7s&c&R|&G&W%-10.10s]  ",
                  sktmp->name, paf->modifier, tiny_affect_loc_name(paf->location), timeb);
            if (i == 0)
               i = 1;
            if ((++i % 2) == 0)
               send_to_pager("\n\r", ch);
         }
      }
   }
   send_to_pager_color("\n\r&g&w&c*&B-----------------------&c*&B-----------------------&c*&B--------------------------&c*&B\n\r&w&R                   Type \"score more\" for more of your stats\n\r", ch);
   send_to_pager("\n\r", ch);
   return;
}

/*
 * Return ascii name of an affect location.
 */
char *tiny_affect_loc_name(int location)
{
   switch (location)
   {
      case APPLY_NONE:
         return "NIL";
      case APPLY_STR:
         return " STR  ";
      case APPLY_DEX:
         return " DEX  ";
      case APPLY_INT:
         return " INT  ";
      case APPLY_WIS:
         return " WIS  ";
      case APPLY_CON:
         return " CON  ";
      case APPLY_CHA:
         return " CHA  ";
      case APPLY_LCK:
         return " LCK  ";
      case APPLY_SEX:
         return " SEX  ";
      case APPLY_CLASS:
         return " CLASS";
      case APPLY_LEVEL:
         return " LVL  ";
      case APPLY_AGE:
         return " AGE  ";
      case APPLY_MANA:
         return " MANA ";
      case APPLY_HIT:
         return " HP   ";
      case APPLY_MOVE:
         return " MOVE ";
      case APPLY_GOLD:
         return " GOLD ";
      case APPLY_EXP:
         return " EXP  ";
      case APPLY_AC:
         return " -----";
      case APPLY_HITROLL:
         return " HITRL";
      case APPLY_DAMROLL:
         return " DAMRL";
      case APPLY_SAVING_POISON:
         return "SV POI";
      case APPLY_SAVING_ROD:
         return "SV ROD";
      case APPLY_SAVING_PARA:
         return "SV PARA";
      case APPLY_SAVING_BREATH:
         return "SV BRTH";
      case APPLY_SAVING_SPELL:
         return "SV SPLL";
      case APPLY_HEIGHT:
         return "HEIGHT";
      case APPLY_WEIGHT:
         return "WEIGHT";
      case APPLY_AFFECT: case APPLY_EXT_AFFECT:
         return "AFF BY";
      case APPLY_RESISTANT:
         return "RESIST";
      case APPLY_IMMUNE:
         return "IMMUNE";
      case APPLY_SUSCEPTIBLE:
         return "SUSCEPT";
      case APPLY_WEAPONSPELL:
         return " WEAPON";
      case APPLY_BACKSTAB:
         return "BACKSTB";
      case APPLY_PICK:
         return " PICK  ";
      case APPLY_TRACK:
         return " TRACK ";
      case APPLY_STEAL:
         return " STEAL ";
      case APPLY_SNEAK:
         return " SNEAK ";
      case APPLY_HIDE:
         return " HIDE  ";
      case APPLY_PALM:
         return " PALM  ";
      case APPLY_DETRAP:
         return " DETRAP";
      case APPLY_DODGE:
         return " DODGE ";
      case APPLY_PEEK:
         return " PEEK  ";
      case APPLY_SCAN:
         return " SCAN  ";
      case APPLY_GOUGE:
         return " GOUGE ";
      case APPLY_SEARCH:
         return " SEARCH";
      case APPLY_MOUNT:
         return " MOUNT ";
      case APPLY_DISARM:
         return " DISARM";
      case APPLY_KICK:
         return " KICK  ";
      case APPLY_PARRY:
         return " PARRY ";
      case APPLY_BASH:
         return " BASH  ";
      case APPLY_STUN:
         return " STUN  ";
      case APPLY_PUNCH:
         return " PUNCH ";
      case APPLY_CLIMB:
         return " CLIMB ";
      case APPLY_GRIP:
         return " GRIP  ";
      case APPLY_SCRIBE:
         return " SCRIBE";
      case APPLY_BREW:
         return " BREW  ";
      case APPLY_WEARSPELL:
         return " WEAR  ";
      case APPLY_REMOVESPELL:
         return " REMOVE";
      case APPLY_EMOTION:
         return "EMOTION";
      case APPLY_MENTALSTATE:
         return " MENTAL";
      case APPLY_STRIPSN:
         return " DISPEL";
      case APPLY_REMOVE:
         return " REMOVE";
      case APPLY_DIG:
         return " DIG   ";
      case APPLY_FULL:
         return " HUNGER";
      case APPLY_THIRST:
         return " THIRST";
      case APPLY_DRUNK:
         return " DRUNK ";
      case APPLY_BLOOD:
         return " BLOOD ";
      case APPLY_COOK:
         return " COOK  ";
      case APPLY_RECURRINGSPELL:
         return " RECURR";
      case APPLY_CONTAGIOUS:
         return "CONTGUS";
      case APPLY_ODOR:
         return " ODOR  ";
      case APPLY_ROOMFLAG:
         return " RMFLG ";
      case APPLY_SECTORTYPE:
         return " SECTOR";
      case APPLY_ROOMLIGHT:
         return " LIGHT ";
      case APPLY_TELEVNUM:
         return " TELEVN";
      case APPLY_TELEDELAY:
         return " TELEDY";
      case APPLY_AGI:
         return " AGI  ";
      case APPLY_ARMOR:
         return " AC   ";
      case APPLY_SHIELD:
         return "DEFLECT";
      case APPLY_STONE:
         return " ABSORB";
      case APPLY_SANCTIFY:   
         return " DAMAGE";
      case APPLY_WMOD:   
         return " WMOD";
      case APPLY_MANAFUSE:
         return "MANAFUSE";
      case APPLY_FASTING:
         return " FASTING";
      case APPLY_TOHIT:
         return " TOHIT ";
      case APPLY_MANATICK:
         return " MANAGEN";
      case APPLY_HPTICK:
         return " HPGEN";         
      case APPLY_MANASHELL:
         return "MSHELL";
      case APPLY_MANASHIELD:
         return "MSHIELD";
      case APPLY_MANAGUARD:
         return "MGUARD";
      case APPLY_MANABURN:
         return "MBURN";
      case APPLY_WEAPONCLAMP:
         return "WPNCLAMP";
      case APPLY_ARROWCATCH:
         return "ACATCH";
      case APPLY_BRACING:
         return "BRACING";
      case APPLY_HARDENING:
         return "HARDNING";
      case APPLY_RFIRE:
         return "RFIRE"; 
      case APPLY_RWATER:
         return "RWATER"; 
      case APPLY_RAIR:
         return "RAIR"; 
      case APPLY_REARTH:
         return "REARTH"; 
      case APPLY_RENERGY:
         return "RENERGY"; 
      case APPLY_RMAGIC:
         return "RMAGIC"; 
      case APPLY_RNONMAGIC:
         return "RNMAGIC";    
      case APPLY_RBLUNT:
         return "RBLUNT";  
      case APPLY_RPIERCE:
         return "RPIERCE";  
      case APPLY_RSLASH:
         return "RSLASH";
      case APPLY_RPOISON:
         return "RPOISON";  
      case APPLY_RPARALYSIS:
         return "RPARA";  
      case APPLY_RHOLY:
         return "RHOLY";  
      case APPLY_RUNHOLY:
         return "RUNHOLY";  
      case APPLY_RUNDEAD:
         return "RUNDEAD"; 
   };

   bug("Affect_location_name: unknown location %d.", location);
   return "(???)";
}

char *get_class(CHAR_DATA * ch)
{
   return (npc_class[0]);
}


char *get_race(CHAR_DATA * ch)
{
   if (ch->race < MAX_RACE && ch->race >= 0)
      return (race_table[ch->race]->race_name);
   if (ch->race < max_npc_race && ch->race >= 0)
      return (print_npc_race(ch->race));
   return ("Unknown");
}

void do_oldscore(CHAR_DATA * ch, char *argument)
{
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   set_pager_color(AT_SCORE, ch);
   pager_printf(ch,
      "You are %s%s, level %d, %d years old (%d hours).\n\r",
      ch->name, IS_NPC(ch) ? "" : ch->pcdata->title, ch->level, get_age(ch), (get_age(ch) - 17) * 20);

   if (get_trust(ch) != ch->level)
      pager_printf(ch, "You are trusted at level %d.\n\r", get_trust(ch));

   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOBINVIS))
      pager_printf(ch, "You are mobinvis at level %d.\n\r", ch->mobinvis);

   if (!IS_NPC(ch) && IS_VAMPIRE(ch))
      pager_printf(ch,
         "You have %d/%d hit, %d/%d blood level, %d/%d movement, %d practices.\n\r",
         ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move, ch->practice);
   else
      pager_printf(ch,
         "You have %d/%d hit, %d/%d mana, %d/%d movement, %d practices.\n\r",
         ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move, ch->practice);

   pager_printf(ch, "You are carrying %d/%d items with weight %d/%d kg.\n\r", get_ch_carry_number(ch), can_carry_n(ch), get_ch_carry_weight(ch), can_carry_w(ch));

   pager_printf(ch,
      "Str: %d  Int: %d  Wis: %d  Dex: %d  Con: %d  Cha: %d  Lck: %d.\n\r",
      get_curr_str(ch), get_curr_int(ch), get_curr_wis(ch), get_curr_dex(ch), get_curr_con(ch), get_curr_cha(ch), get_curr_lck(ch));

   pager_printf(ch, "You have %d gold coins.\n\r", ch->gold);

   if (!IS_NPC(ch))
      pager_printf(ch, "You have achieved %d glory during your life, and currently have %d.\n\r", ch->pcdata->quest_accum, ch->pcdata->quest_curr);

   pager_printf(ch,
      "Autoexit: %s   Autoloot: %s   Autosac: %s   Autogold: %s\n\r",
      (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOEXIT)) ? "yes" : "no",
      (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOLOOT)) ? "yes" : "no",
      (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOSAC)) ? "yes" : "no", (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOGOLD)) ? "yes" : "no");

   pager_printf(ch, "Wimpy set to %d hit points.\n\r", ch->wimpy);

   if (!IS_NPC(ch))
   {
      if (ch->pcdata->condition[COND_DRUNK] > 10)
         send_to_pager("You are drunk.\n\r", ch);
      if (ch->pcdata->condition[COND_THIRST] == 0)
         send_to_pager("You are thirsty.\n\r", ch);
      if (ch->pcdata->condition[COND_FULL] == 0)
         send_to_pager("You are hungry.\n\r", ch);
   }

   switch (ch->mental_state / 10)
   {
      default:
         send_to_pager("You're completely messed up!\n\r", ch);
         break;
      case -10:
         send_to_pager("You're barely conscious.\n\r", ch);
         break;
      case -9:
         send_to_pager("You can barely keep your eyes open.\n\r", ch);
         break;
      case -8:
         send_to_pager("You're extremely drowsy.\n\r", ch);
         break;
      case -7:
         send_to_pager("You feel very unmotivated.\n\r", ch);
         break;
      case -6:
         send_to_pager("You feel sedated.\n\r", ch);
         break;
      case -5:
         send_to_pager("You feel sleepy.\n\r", ch);
         break;
      case -4:
         send_to_pager("You feel tired.\n\r", ch);
         break;
      case -3:
         send_to_pager("You could use a rest.\n\r", ch);
         break;
      case -2:
         send_to_pager("You feel a little under the weather.\n\r", ch);
         break;
      case -1:
         send_to_pager("You feel fine.\n\r", ch);
         break;
      case 0:
         send_to_pager("You feel great.\n\r", ch);
         break;
      case 1:
         send_to_pager("You feel energetic.\n\r", ch);
         break;
      case 2:
         send_to_pager("Your mind is racing.\n\r", ch);
         break;
      case 3:
         send_to_pager("You can't think straight.\n\r", ch);
         break;
      case 4:
         send_to_pager("Your mind is going 100 miles an hour.\n\r", ch);
         break;
      case 5:
         send_to_pager("You're high as a kite.\n\r", ch);
         break;
      case 6:
         send_to_pager("Your mind and body are slipping appart.\n\r", ch);
         break;
      case 7:
         send_to_pager("Reality is slipping away.\n\r", ch);
         break;
      case 8:
         send_to_pager("You have no idea what is real, and what is not.\n\r", ch);
         break;
      case 9:
         send_to_pager("You feel immortal.\n\r", ch);
         break;
      case 10:
         send_to_pager("You are a Supreme Entity.\n\r", ch);
         break;
   }

   switch (ch->position)
   {
      case POS_DEAD:
         send_to_pager("You are DEAD!!\n\r", ch);
         break;
      case POS_MORTAL:
         send_to_pager("You are mortally wounded.\n\r", ch);
         break;
      case POS_INCAP:
         send_to_pager("You are incapacitated.\n\r", ch);
         break;
      case POS_STUNNED:
         send_to_pager("You are stunned.\n\r", ch);
         break;
      case POS_SLEEPING:
         send_to_pager("You are sleeping.\n\r", ch);
         break;
      case POS_RESTING:
         send_to_pager("You are resting.\n\r", ch);
         break;
      case POS_STANDING:
         send_to_pager("You are standing.\n\r", ch);
         break;
      case POS_FIGHTING:
         send_to_pager("You are fighting.\n\r", ch);
         break;
      case POS_MOUNTED:
         send_to_pager("Mounted.\n\r", ch);
         break;
      case POS_RIDING:
         send_to_pager("Riding.\n\r", ch);
         break;
      case POS_SHOVE:
         send_to_pager("Being shoved.\n\r", ch);
         break;
      case POS_DRAG:
         send_to_pager("Being dragged.\n\r", ch);
         break;
   }

   if (ch->level >= 12)
      pager_printf(ch, "AC: %d.  ", GET_AC(ch));

   send_to_pager("You are ", ch);
   if (GET_AC(ch) >= 101)
      send_to_pager("WORSE than naked!\n\r", ch);
   else if (GET_AC(ch) >= 80)
      send_to_pager("naked.\n\r", ch);
   else if (GET_AC(ch) >= 60)
      send_to_pager("wearing clothes.\n\r", ch);
   else if (GET_AC(ch) >= 40)
      send_to_pager("slightly armored.\n\r", ch);
   else if (GET_AC(ch) >= 20)
      send_to_pager("somewhat armored.\n\r", ch);
   else if (GET_AC(ch) >= 0)
      send_to_pager("armored.\n\r", ch);
   else if (GET_AC(ch) >= -20)
      send_to_pager("well armored.\n\r", ch);
   else if (GET_AC(ch) >= -40)
      send_to_pager("strongly armored.\n\r", ch);
   else if (GET_AC(ch) >= -60)
      send_to_pager("heavily armored.\n\r", ch);
   else if (GET_AC(ch) >= -80)
      send_to_pager("superbly armored.\n\r", ch);
   else if (GET_AC(ch) >= -100)
      send_to_pager("divinely armored.\n\r", ch);
   else
      send_to_pager("invincible!\n\r", ch);

   if (ch->level >= 7)
      pager_printf(ch, "Hitroll: %d  Damroll: %d.\n\r", GET_HITROLL(ch), GET_DAMROLL(ch));

   if (ch->level >= 5)
      pager_printf(ch, "Alignment: %d.  ", ch->alignment);

   send_to_pager("You are ", ch);
   if (ch->alignment > 900)
      send_to_pager("angelic.\n\r", ch);
   else if (ch->alignment > 700)
      send_to_pager("saintly.\n\r", ch);
   else if (ch->alignment > 350)
      send_to_pager("good.\n\r", ch);
   else if (ch->alignment > 100)
      send_to_pager("kind.\n\r", ch);
   else if (ch->alignment > -100)
      send_to_pager("neutral.\n\r", ch);
   else if (ch->alignment > -350)
      send_to_pager("mean.\n\r", ch);
   else if (ch->alignment > -700)
      send_to_pager("evil.\n\r", ch);
   else if (ch->alignment > -900)
      send_to_pager("demonic.\n\r", ch);
   else
      send_to_pager("satanic.\n\r", ch);

   if (ch->first_affect)
   {
      send_to_pager("You are affected by:\n\r", ch);
      for (paf = ch->first_affect; paf; paf = paf->next)
         if ((skill = get_skilltype(paf->type)) != NULL)
         {
            pager_printf(ch, "Spell: '%s'", skill->name);

            if (ch->level >= 10)
               pager_printf(ch, " modifies %s by %d for %d rounds", affect_loc_name(paf->location), paf->modifier, paf->duration);

            send_to_pager(".\n\r", ch);
         }
   }

   if (!IS_NPC(ch) && IS_IMMORTAL(ch))
   {
      pager_printf(ch, "\n\rWizInvis level: %d   WizInvis is %s\n\r", ch->pcdata->wizinvis, xIS_SET(ch->act, PLR_WIZINVIS) ? "ON" : "OFF");
      if (ch->pcdata->r_range_lo && ch->pcdata->r_range_hi)
         pager_printf(ch, "Room Range: %d - %d\n\r", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi);
      if (ch->pcdata->o_range_lo && ch->pcdata->o_range_hi)
         pager_printf(ch, "Obj Range : %d - %d\n\r", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi);
      if (ch->pcdata->m_range_lo && ch->pcdata->m_range_hi)
         pager_printf(ch, "Mob Range : %d - %d\n\r", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi);
   }

   return;
}

/* 1997, Blodkai */
void do_remains(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   OBJ_DATA *obj;
   bool found = FALSE;

   if (IS_NPC(ch))
      return;
   set_char_color(AT_MAGIC, ch);
   if (!ch->pcdata->deity)
   {
      send_to_pager("You have no deity from which to seek such assistance...\n\r", ch);
      return;
   }
   if (ch->pcdata->favor < ch->level * 2)
   {
      send_to_pager("Your favor is insufficient for such assistance...\n\r", ch);
      return;
   }
   pager_printf(ch, "%s appears in a vision, revealing that your remains... ", ch->pcdata->deity->name);
   sprintf(buf, "the corpse of %s", ch->name);
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->in_room && !str_cmp(buf, obj->short_descr) && (obj->pIndexData->vnum == 11))
      {
         found = TRUE;
         pager_printf(ch, "\n\r  - at %s will endure for %d ticks", obj->in_room->name, obj->timer);
      }
   }
   if (!found)
      send_to_pager(" no longer exist.\n\r", ch);
   else
   {
      send_to_pager("\n\r", ch);
      ch->pcdata->favor -= ch->level * 2;
   }
   return;
}

/* Affects-at-a-glance, Blodkai */
void do_affected(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   AFFECT_DATA *paf;
   SKILLTYPE *skill;

   if (IS_NPC(ch))
      return;

   set_char_color(AT_SCORE, ch);

   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "by"))
   {
      send_to_char_color("\n\r&BImbued with:\n\r", ch);
      ch_printf_color(ch, "&C%s\n\r", !xIS_EMPTY(ch->affected_by) ? affect_bit_name(&ch->affected_by) : "nothing");
      if (ch->level >= 10)
      {
         send_to_char("\n\r", ch);
         if (ch->resistant > 0)
         {
            send_to_char_color("&BResistances:  ", ch);
            ch_printf_color(ch, "&C%s\n\r", flag_string(ch->resistant, ris_flags));
         }
         if (ch->immune > 0)
         {
            send_to_char_color("&BImmunities:   ", ch);
            ch_printf_color(ch, "&C%s\n\r", flag_string(ch->immune, ris_flags));
         }
         if (ch->susceptible > 0)
         {
            send_to_char_color("&BSuscepts:     ", ch);
            ch_printf_color(ch, "&C%s\n\r", flag_string(ch->susceptible, ris_flags));
         }
      }
      return;
   }

   if (!ch->first_affect)
   {
      send_to_char_color("\n\r&CNo cantrip or skill affects you.\n\r", ch);
   }
   else
   {
      send_to_char("\n\r", ch);
      for (paf = ch->first_affect; paf; paf = paf->next)
         if ((skill = get_skilltype(paf->type)) != NULL)
         {
            set_char_color(AT_BLUE, ch);
            send_to_char("Affected:  ", ch);
            set_char_color(AT_SCORE, ch);
            if (ch->level >= 10)
            {
               if (paf->duration < 25)
                  set_char_color(AT_WHITE, ch);
               if (paf->duration < 6)
                  set_char_color(AT_WHITE + AT_BLINK, ch);
               ch_printf(ch, "(%5d)   ", paf->duration);
            }
            ch_printf(ch, "%-18s\n\r", skill->name);
         }
   }
   return;
}

void do_inventory(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_RED, ch);
   send_to_char("You are carrying:\n\r", ch);
   show_list_to_char(ch->first_carrying, ch, TRUE, TRUE, eItemDrop);
   return;
}

/* Added support for nothing to show up -- Xerves 7/9/99 */
void do_equipment(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   int iWear;
   bool found;

   set_char_color(AT_RED, ch);
   send_to_char("You are using:\n\r", ch);
   found = FALSE;
   set_char_color(AT_OBJECT, ch);
   for (iWear = 0; iWear < MAX_WEAR; iWear++)
   {
      for (obj = ch->first_carrying; obj; obj = obj->next_content)
         if (obj->wear_loc == iWear)
         {
            if (iWear >= WEAR_LODGE_RIB && iWear <= WEAR_NOCKED)
               ch_printf(ch, "&R%s", where_name[iWear]);
            else
               send_to_char(where_name[iWear], ch);

            if (can_see_obj(ch, obj))
            {
               send_to_char(format_obj_to_char(obj, ch, TRUE, FALSE), ch);
               set_char_color(AT_OBJECT, ch);
               send_to_char("\n\r", ch);
            }
            else
               send_to_char("something.\n\r", ch);
            found = TRUE;
         } /* Checks to see if char is using the slot -- Xerves */
      if ((obj = get_eq_char(ch, iWear)) == NULL)
      {
         if (iWear >= WEAR_LODGE_RIB && iWear <= WEAR_NOCKED)
            continue;
         send_to_char(where_name[iWear], ch);
         send_to_char("&c&w* <Nothing>&G\n\r", ch);
         set_char_color(AT_OBJECT, ch);
         found = TRUE;
      }
   }

   if (!found)
      send_to_char("Nothing.\n\r", ch);

   return;
}



void set_title(CHAR_DATA * ch, char *title)
{
   char buf[MSL];

   if (IS_NPC(ch))
   {
      bug("Set_title: NPC.", 0);
      return;
   }

   if (isalpha(title[0]) || isdigit(title[0]))
   {
      buf[0] = ' ';
      strcpy(buf + 1, title);
   }
   else
      strcpy(buf, title);

   STRFREE(ch->pcdata->title);
   ch->pcdata->title = STRALLOC(buf);
   return;
}



void do_title(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
      return;

   set_char_color(AT_SCORE, ch);
   if (ch->level < 3)
   {
      send_to_char("Sorry... you must be at least level 3 to set your title...\n\r", ch);
      return;
   }
   if (IS_SET(ch->pcdata->flags, PCFLAG_NOTITLE))
   {
      set_char_color(AT_IMMORT, ch);
      send_to_char("The Gods prohibit you from changing your title.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Change your title to what?\n\r", ch);
      return;
   }

   if (strlen(argument) > 50)
      argument[50] = '\0';

   smash_tilde(argument);
   set_title(ch, argument);
   send_to_char("Ok.\n\r", ch);
}


void do_homepage(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];

   if (IS_NPC(ch))
      return;

   if (ch->level < 3)
   {
      send_to_char("Sorry... you must be at least level 3 to do that.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      if (!ch->pcdata->homepage)
         ch->pcdata->homepage = str_dup("");
      ch_printf(ch, "Your homepage is: %s\n\r", show_tilde(ch->pcdata->homepage));
      return;
   }

   if (!str_cmp(argument, "clear"))
   {
      if (ch->pcdata->homepage)
         DISPOSE(ch->pcdata->homepage);
      ch->pcdata->homepage = str_dup("");
      send_to_char("Homepage cleared.\n\r", ch);
      return;
   }

   if (strstr(argument, "://"))
      strcpy(buf, argument);
   else
      sprintf(buf, "http://%s", argument);
   if (strlen(buf) > 70)
      buf[70] = '\0';

   hide_tilde(buf);
   if (ch->pcdata->homepage)
      DISPOSE(ch->pcdata->homepage);
   ch->pcdata->homepage = str_dup(buf);
   send_to_char("Homepage set.\n\r", ch);
}



/*
 * Set your personal description				-Thoric
 */
void do_description(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("Monsters are too dumb to do that!\n\r", ch);
      return;
   }

   if (!ch->desc)
   {
      bug("do_description: no descriptor", 0);
      return;
   }

   switch (ch->substate)
   {
      default:
         bug("do_description: illegal substate", 0);
         return;

      case SUB_RESTRICTED:
         send_to_char("You cannot use this command from within another command.\n\r", ch);
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_DESC;
         ch->dest_buf = ch;
         start_editing(ch, ch->description);
         editor_desc_printf(ch, "Your description (%s)", ch->name);
         return;

      case SUB_PERSONAL_DESC:
         STRFREE(ch->description);
         ch->description = copy_buffer(ch);
         stop_editing(ch);
         return;
   }
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot set a bio.\n\r", ch);
      return;
   }
   if (!ch->desc)
   {
      bug("do_bio: no descriptor", 0);
      return;
   }

   switch (ch->substate)
   {
      default:
         bug("do_bio: illegal substate", 0);
         return;

      case SUB_RESTRICTED:
         send_to_char("You cannot use this command from within another command.\n\r", ch);
         return;

      case SUB_NONE:
         ch->substate = SUB_PERSONAL_BIO;
         ch->dest_buf = ch;
         start_editing(ch, ch->pcdata->bio);
         editor_desc_printf(ch, "Your bio (%s).", ch->name);
         return;

      case SUB_PERSONAL_BIO:
         STRFREE(ch->pcdata->bio);
         ch->pcdata->bio = copy_buffer(ch);
         stop_editing(ch);
         return;
   }
}



/*
 * New stat and statreport command coded by Morphina
 * Bug fixes by Shaddai
 */

void do_statreport(CHAR_DATA * ch, char *argument)
{
   char buf[MIL];

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (IS_VAMPIRE(ch))
   {
      ch_printf(ch, "You report: %d/%d hp %d/%d blood %d/%d mv.\n\r",
         ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move);
      sprintf(buf, "$n reports: %d/%d hp %d/%d blood %d/%d mv.",
         ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move);
      act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);
   }
   else
   {
      ch_printf(ch, "You report: %d/%d hp %d/%d mana %d/%d mv.\n\r",
         ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
      sprintf(buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv.", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
      act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);
   }

   ch_printf(ch, "Your base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\n\r",
      ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck);
   sprintf(buf, "$n's base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.",
      ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck);
   act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);

   ch_printf(ch, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\n\r",
      get_curr_str(ch), get_curr_wis(ch), get_curr_int(ch), get_curr_dex(ch), get_curr_con(ch), get_curr_cha(ch), get_curr_lck(ch));
   sprintf(buf, "$n's current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.",
      get_curr_str(ch), get_curr_wis(ch), get_curr_int(ch), get_curr_dex(ch), get_curr_con(ch), get_curr_cha(ch), get_curr_lck(ch));
   act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);
   return;
}

void do_stat(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (IS_VAMPIRE(ch))
      ch_printf(ch, "You report: %d/%d hp %d/%d blood %d/%d mv.\n\r",
         ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move);
   else
      ch_printf(ch, "You report: %d/%d hp %d/%d mana %d/%d mv.\n\r",
         ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);

   ch_printf(ch, "Your base stats:    %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\n\r",
      ch->perm_str, ch->perm_wis, ch->perm_int, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck);

   ch_printf(ch, "Your current stats: %-2d str %-2d wis %-2d int %-2d dex %-2d con %-2d cha %-2d lck.\n\r",
      get_curr_str(ch), get_curr_wis(ch), get_curr_int(ch), get_curr_dex(ch), get_curr_con(ch), get_curr_cha(ch), get_curr_lck(ch));
   return;
}


void do_report(CHAR_DATA * ch, char *argument)
{
   char buf[MIL];

   if (IS_NPC(ch) && ch->fighting)
      return;

   if (IS_AFFECTED(ch, AFF_POSSESS))
   {
      send_to_char("You can't do that in your current state of mind!\n\r", ch);
      return;
   }


   if (IS_VAMPIRE(ch))
      ch_printf(ch,
         "You report: %d/%d hp %d/%d blood %d/%d mv.\n\r",
         ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move);
   else
      ch_printf(ch,
         "You report: %d/%d hp %d/%d mana %d/%d mv.\n\r", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);

   if (IS_VAMPIRE(ch))
      sprintf(buf, "$n reports: %d/%d hp %d/%d blood %d/%d mv.\n\r",
         ch->hit, ch->max_hit, ch->pcdata->condition[COND_BLOODTHIRST], 10 + ch->level, ch->move, ch->max_move);
   else
      sprintf(buf, "$n reports: %d/%d hp %d/%d mana %d/%d mv.", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);

   act(AT_REPORT, buf, ch, NULL, NULL, TO_ROOM);

   return;
}

void do_gprompt(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_GREY, ch);

   if (IS_NPC(ch))
   {
      send_to_char("NPC's can't change their group prompt..\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  gprompt <fight/both/off>\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "fight"))
      ch->pcdata->gprompt = 0;
   else if (!str_cmp(argument, "both"))
      ch->pcdata->gprompt = 1;
   else if (!str_cmp(argument, "off"))
      ch->pcdata->gprompt = 2;
   else
   {
      do_gprompt(ch, "");
      return;
   }
}

void do_fprompt(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   set_char_color(AT_GREY, ch);

   if (IS_NPC(ch))
   {
      send_to_char("NPC's can't change their prompt..\n\r", ch);
      return;
   }
   smash_tilde(argument);
   one_argument(argument, arg);
   if (!*arg || !str_cmp(arg, "display"))
   {
      send_to_char("Your current fighting prompt string:\n\r", ch);
      set_char_color(AT_WHITE, ch);
      ch_printf(ch, "%s\n\r", !str_cmp(ch->pcdata->fprompt, "") ? "(default prompt)" : ch->pcdata->fprompt);
      set_char_color(AT_GREY, ch);
      send_to_char("Type 'help prompt' for information on changing your prompt.\n\r", ch);
      return;
   }
   send_to_char("Replacing old prompt of:\n\r", ch);
   set_char_color(AT_WHITE, ch);
   ch_printf(ch, "%s\n\r", !str_cmp(ch->pcdata->fprompt, "") ? "(default prompt)" : ch->pcdata->fprompt);
   if (ch->pcdata->fprompt)
      STRFREE(ch->pcdata->fprompt);
   if (strlen(argument) > 128)
      argument[128] = '\0';

   /* Can add a list of pre-set prompts here if wanted.. perhaps
      'prompt 1' brings up a different, pre-set prompt */
   if (!str_cmp(arg, "default"))
      ch->pcdata->fprompt = STRALLOC("");
   else
      ch->pcdata->fprompt = STRALLOC(argument);
   return;
}

void do_prompt(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   set_char_color(AT_GREY, ch);

   if (IS_NPC(ch))
   {
      send_to_char("NPC's can't change their prompt..\n\r", ch);
      return;
   }
   smash_tilde(argument);
   one_argument(argument, arg);
   if (!*arg || !str_cmp(arg, "display"))
   {
      send_to_char("Your current prompt string:\n\r", ch);
      set_char_color(AT_WHITE, ch);
      ch_printf(ch, "%s\n\r", !str_cmp(ch->pcdata->prompt, "") ? "(default prompt)" : ch->pcdata->prompt);
      set_char_color(AT_GREY, ch);
      send_to_char("Type 'help prompt' for information on changing your prompt.\n\r", ch);
      return;
   }
   send_to_char("Replacing old prompt of:\n\r", ch);
   set_char_color(AT_WHITE, ch);
   ch_printf(ch, "%s\n\r", !str_cmp(ch->pcdata->prompt, "") ? "(default prompt)" : ch->pcdata->prompt);
   if (ch->pcdata->prompt)
      STRFREE(ch->pcdata->prompt);
   if (strlen(argument) > 128)
      argument[128] = '\0';

   /* Can add a list of pre-set prompts here if wanted.. perhaps
      'prompt 1' brings up a different, pre-set prompt */
   if (!str_cmp(arg, "default"))
      ch->pcdata->prompt = STRALLOC("");
   else
      ch->pcdata->prompt = STRALLOC(argument);
   return;
}

/*
 * Figured this belonged here seeing it involves players... 
 * really simple little function to tax players with a large
 * amount of gold to help reduce the overall gold pool...
 *  --TRI
 */
void tax_player(CHAR_DATA * ch)
{
/*
  int gold = ch->gold;
  int mgold = (ch->level * 2000000);
  int tax = (ch->gold * .05);

  if ( gold > mgold )
  {
    set_char_color( AT_WHITE, ch );
    ch_printf( ch, "You are level %d and carry more than %d coins.\n\r",
	ch->level, mgold );
    ch_printf( ch, "You are being taxed \%5 percent (%d coins) of your %d coins,\n\r",
	tax, ch->gold );
    ch_printf( ch, "and that leaves you with %d coins.\n\r",
	(ch->gold - tax));
    ch->gold -= tax;

  }
  return;
*/
}
