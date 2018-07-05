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
 *		     Color Module -- Allow user customizable Colors.            *
 *                                   --Matthew                              *
 ****************************************************************************/

/******************************************************
            Desolation of the Dragon MUD II
      (C) 1997, 1998  Jesse DeFer and Heath Leach
 http://dotd.mudservices.com  dotd@dotd.mudservices.com 
 ******************************************************/

/*
* The following instructions assume you know at least a little bit about
* coding.  I firmly believe that if you can't code (at least a little bit),
* you don't belong running a mud.  So, with that in mind, I don't hold your
* hand through these instructions.
*
* You may use this code provided that:
*
*     1)  You understand that the authors _DO NOT_ support this code
*         Any help you need must be obtained from other sources.  The
*         authors will ignore any and all requests for help.
*     2)  You will mention the authors if someone asks about the code.
*         You will not take credit for the code, but you can take credit
*         for any enhancements you make.
*     3)  This message remains intact.
*
* If you would like to find out how to send the authors large sums of money,
* you may e-mail the following address:
*
* Matthew Bafford & Christopher Wigginton
* wiggy@mudservices.com
*/

/*
 * To add new color types:
 *
 * 1.  Edit color.h, and:
 *     1.  Add a new AT_ define.
 *     2.  Increment MAX_COLORS by however AT_'s you added.
 * 2.  Edit color.c and:
 *     1.  Add the name(s) for your new color(s) to the end of the pc_displays
 *         array.
 *     2.  Add the default color(s) to the end of the default_set array.
 */

#include <stdio.h>
#include <string.h>
#include "mud.h"

char *const pc_displays[MAX_COLORS] = {
   "black", "dred", "dgreen", "orange", "dblue",
   "purple", "cyan", "grey", "dgrey", "red",
   "green", "yellow", "blue", "pink", "lblue",
   "white", "blink", "plain", "action", "say",
   "chat", "yell", "tell", "hit", "hitme",
   "immortal", "hurting", "falling", "danger", "magic",
   "consider", "report", "poison", "social", "dying",
   "dead", "skills", "carnage", "damage", "fleeing",
   "rmname", "rmdesc", "objects", "people", "list",
   "bye", "gold", "gtells", "note", "hungry",
   "thirsty", "fire", "sober", "wearoff", "exits",
   "score", "reset", "log", "die_msg", "wartalk",
   "arena", "muse", "think", "aflags", "who",
   "racetalk", "ignore", "whisper", "divider", "morph",
   "shout", "rflags", "stype", "aname", "auction",
   "score2", "score3", "score4", "who2", "who3",
   "who4"
};

/* All defaults are set to Lands of Solan default scheme, if you don't 
like it, change it around to suite your own needs - Samson */
const sh_int default_set[MAX_COLORS] = {
   AT_BLACK, AT_BLOOD, AT_DGREEN, AT_ORANGE,
   AT_DBLUE, AT_PURPLE, AT_CYAN, AT_GREY,
   AT_DGREY, AT_RED, AT_GREEN, AT_YELLOW,
   AT_BLUE, AT_PINK, AT_LBLUE, AT_WHITE,

   AT_RED + AT_BLINK, AT_GREY, AT_GREY, AT_LBLUE,
   AT_LBLUE, AT_GREEN, AT_WHITE, AT_WHITE,
   AT_LBLUE, AT_YELLOW, AT_RED, AT_WHITE + AT_BLINK,
   AT_RED + AT_BLINK, AT_BLUE, AT_GREY, AT_GREY,
   AT_GREEN, AT_CYAN, AT_YELLOW, AT_RED,
   AT_GREEN, AT_BLOOD, AT_WHITE, AT_YELLOW,
   AT_WHITE, AT_LBLUE, AT_GREEN, AT_PINK,
   AT_BLUE, AT_GREEN, AT_YELLOW, AT_BLUE,
   AT_GREEN, AT_ORANGE, AT_BLUE, AT_RED,
   AT_WHITE, AT_YELLOW, AT_WHITE, AT_DGREEN,
   AT_DGREEN, AT_PURPLE, AT_WHITE, AT_RED,
   AT_YELLOW, AT_RED, AT_ORANGE, AT_GREEN,
   AT_GREEN, AT_DGREEN, AT_GREEN, AT_YELLOW,
   AT_GREY, AT_GREY, AT_DGREY, AT_BLUE,
   AT_BLUE, AT_BLUE, AT_GREEN, AT_GREEN,
   AT_YELLOW, AT_LBLUE, AT_DGREY, AT_BLUE,
   AT_DGREY
};

char *const valid_color[] = {
   "black",
   "dred",
   "dgreen",
   "orange",
   "dblue",
   "purple",
   "cyan",
   "grey",
   "dgrey",
   "red",
   "green",
   "yellow",
   "blue",
   "pink",
   "lblue",
   "white",
   "\0"
};

void show_colors(CHAR_DATA * ch)
{
   int count;

   set_pager_color(AT_PLAIN, ch);
   send_to_pager("Syntax: color <color type> <color>|default\n\r", ch);
   send_to_pager("Syntax: color _reset_ (Resets all colors to default set)\n\r", ch);
   send_to_pager("Syntax: color _all_ <color> (Sets all color types to <color>)\n\r", ch);

   set_pager_color(AT_WHITE, ch);
   send_to_pager("********************************[ COLORS ]*********************************\r\n", ch);

   for (count = 0; count < 16; ++count)
   {
      if ((count % 8) == 0 && count != 0)
      {
         send_to_pager("\r\n", ch);
      }
      set_pager_color(count, ch);
      pager_printf(ch, "%-10s", pc_displays[count]);
   }

   set_pager_color(AT_WHITE, ch);
   send_to_pager("\r\n\r\n******************************[ COLOR TYPES ]******************************\r\n", ch);

   for (count = 16; count < MAX_COLORS; ++count)
   {
      if ((count % 8) == 0 && count != 16)
      {
         send_to_pager("\r\n", ch);
      }
      set_pager_color(count, ch);
      pager_printf(ch, "%-10s", pc_displays[count]);
   }
   set_pager_color(AT_YELLOW, ch);
   send_to_pager("\r\n\r\nAvailable colors are:\r\n", ch);

   set_pager_color(AT_PLAIN, ch);
   for (count = 0; valid_color[count][0] != '\0'; ++count)
   {
      if ((count % 8) == 0 && count != 0)
         send_to_pager("\r\n", ch);

      pager_printf(ch, "%-10s", valid_color[count]);
   }
   send_to_pager("\n\r", ch);
   return;
}

void do_color(CHAR_DATA * ch, char *argument)
{
   bool dMatch, cMatch;
   int count = 0, y = 0;
   char arg[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];

   dMatch = FALSE;
   cMatch = FALSE;

   if (IS_NPC(ch))
   {
      send_to_pager("Only PC's can change colors.\n\r", ch);
      return;
   }

   if (!argument || argument[0] == '\0')
   {
      show_colors(ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (!str_prefix(arg, "_reset_"))
   {
      reset_colors(ch);
      send_to_pager("All color types reset to default colors.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg2);

   if (arg[0] == '\0')
   {
      send_to_char("Change which color type?\r\n", ch);
      return;
   }

   argument = one_argument(argument, arg3);

   if (!str_prefix(arg, "_all_"))
   {
      dMatch = TRUE;
      count = -1;

      /* search for a valid color setting */
      for (y = 0; y < 16; y++)
      {
         if (!str_cmp(arg2, valid_color[y]))
         {
            cMatch = TRUE;
            break;
         }
      }
   }
   else if (arg2[0] == '\0')
   {
      cMatch = FALSE;
   }
   else
   {
      /* search for the display type and strcmp */
      for (count = 0; count < MAX_COLORS; count++)
      {
         if (!str_prefix(arg, pc_displays[count]))
         {
            dMatch = TRUE;
            break;
         }
      }

      if (!dMatch)
      {
         ch_printf(ch, "%s is an invalid color type.\n\r", arg);
         send_to_char("Type color with no arguments to see available options.\n\r", ch);
         return;
      }

      if (!str_cmp(arg2, "default"))
      {
         ch->pcdata->colors[count] = default_set[count];
         sprintf(buf, "Display %s set back to default.\n\r", pc_displays[count]);
         send_to_pager(buf, ch);
         return;
      }

      /* search for a valid color setting */
      for (y = 0; y < 16; y++)
      {
         if (!str_cmp(arg2, valid_color[y]))
         {
            cMatch = TRUE;
            break;
         }
      }
   }

   if (!cMatch)
   {
      if (arg[0])
      {
         ch_printf(ch, "Invalid color for type %s.\n", arg);
      }
      else
      {
         send_to_pager("Invalid color.\n\r", ch);
      }

      send_to_pager("Choices are:\n\r", ch);

      for (count = 0; count < 16; count++)
      {
         if (count % 5 == 0 && count != 0)
            send_to_pager("\r\n", ch);

         pager_printf(ch, "%-10s", valid_color[count]);
      }

      pager_printf(ch, "%-10s\r\n", "default");
      return;
   }
   else
   {
      sprintf(buf, "Color type %s set to color %s.\n\r", count == -1 ? "_all_" : pc_displays[count], valid_color[y]);
   }

   if (!str_cmp(arg3, "blink"))
   {
      y += AT_BLINK;
   }

   if (count == -1)
   {
      int count;

      for (count = 0; count < MAX_COLORS; ++count)
      {
         ch->pcdata->colors[count] = y;
      }

      set_pager_color(y, ch);

      sprintf(buf, "All color types set to color %s%s.\r\n", valid_color[y > AT_BLINK ? y - AT_BLINK : y], y > AT_BLINK ? " [BLINKING]" : "");

      send_to_pager(buf, ch);
   }
   else
   {
      ch->pcdata->colors[count] = y;

      set_pager_color(count, ch);

      if (!str_cmp(arg3, "blink"))
         sprintf(buf, "Display %s set to color %s [BLINKING]\n\r", pc_displays[count], valid_color[y - AT_BLINK]);
      else
         sprintf(buf, "Display %s set to color %s.\n\r", pc_displays[count], valid_color[y]);

      send_to_pager(buf, ch);
   }
   set_pager_color(AT_PLAIN, ch);

   return;
}

void reset_colors(CHAR_DATA * ch)
{
   if (ch->pcdata)
      memcpy(&ch->pcdata->colors, &default_set, sizeof(default_set));
}

/* Moved from comm.c */
void set_char_color(sh_int AType, CHAR_DATA * ch)
{
   if (!ch || !ch->desc || !ch->pcdata)
      return;

   write_to_buffer(ch->desc, color_str(AType, ch), 0);
   if (!ch || !ch->desc || !ch->pcdata)
      return;
   ch->desc->prevcolor = ch->pcdata->colors[AType];
}

void set_pager_color(sh_int AType, CHAR_DATA * ch)
{
   if (!ch || !ch->desc || !ch->pcdata)
      return;

   send_to_pager(color_str(AType, ch), ch);
   if (!ch || !ch->desc || !ch->pcdata)
      return;
   ch->desc->pagecolor = ch->pcdata->colors[AType];
}
