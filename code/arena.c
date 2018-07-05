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
 *		            Rafermand Arena Code		            *
 ****************************************************************************
 *   The below code is custom code created for us in Rafermand, it allows   *
 *   players and immortals to fight in the arena at an equal level and      *
 *   skill.  It will allow for a few different kind of games and a few      *
 *   other things.                                                          *
 *   Part of this code includes the Arena snippet by Kevin Hoogheem, which  * 
 *   was ported by LrdElder.  The rest is Coded by Xerves                   *
 ****************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#ifndef WIN32
#include <dirent.h>
#endif
#include "mud.h"

#define ARENA_SVNUM 60001
#define ARENA_EVNUM 60061
#define PREP_START  60000 /* vnum of first prep room */
#define PREP_END    60000 /* vnum of last prep room */
#define ARENA_END   60069 /* vnum of last real arena room */
#define ROBJ_START 13060
#define UOBJ_START 13250
#define ARENA_SPOTION  13050
#define ARENA_EPOTION  13053

void start_arena();
void show_jack_pot();
void do_game();
int num_in_arena();
void find_game_winner();
void do_end_game();
void start_game();
void silent_end();
void find_bet_winners(CHAR_DATA * winner, bool type);

int ppl_challenged = 0;
int ppl_in_arena = 0;
int ppl_at_start = 0;
int in_start_arena = 0;
int start_time;
int game_length;
int lo_lim;
int hi_lim;
int game_type;
int time_to_start;
int time_left_in_game;
int arena_pot;
int bet_pot;
int barena = 0;
int arena_ritem = 0;
int arena_uitem = 0;

extern int parsebet(const int currentbet, char *s);
extern int advatoi(char *s);
extern void read_obj_arena(CHAR_DATA * ch, char *argument);

//type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 6 - kill 7 - pkill
//8 - pdeath 9 - pranking
//Updates the barena linked list, global
void update_barena(CHAR_DATA * ch, int type)
{
   BARENA_DATA *bdata;

   if (barena == 0)
      return;

   if (type == 0)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->wins += 1;
            return;
         }
      }
   }
   if (type == 1)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->losses += 1;
            return;
         }
      }
   }
   if (type == 2)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->ties += 1;
            return;
         }
      }
   }
   if (type == 3)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->games += 1;
            return;
         }
      }
   }
   if (type == 4)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->numavg += num_in_arena();
            return;
         }
      }
   }
   if (type == 5)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            return;
         }
      }
      CREATE(bdata, BARENA_DATA, 1);
      bdata->wins = 0;
      bdata->losses = 0;
      bdata->ties = 0;
      bdata->games = 0;
      bdata->numavg = 0;
      bdata->pkills = ch->pcdata->pkills;
      bdata->pdeaths = ch->pcdata->pdeaths;
      bdata->pranking = 0;
      bdata->name = STRALLOC(ch->name);
      LINK(bdata, first_barena, last_barena, next, prev);
      return;
   }
   if (type == 6)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->kills += 1;
            return;
         }
      }
   }
   if (type == 7)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->pkills = ch->pcdata->pkills;
            return;
         }
      }
   }
   if (type == 8)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->pdeaths = ch->pcdata->pdeaths;
            return;
         }
      }
   }
   if (type == 9)
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (!str_cmp(bdata->name, ch->name))
         {
            bdata->pranking = ch->pcdata->pranking;
            return;
         }
      }
   }
}

void do_startarena(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   int v1, v2;

   v1 = v2 = 0;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for NPCs.\n\r", ch);
      return;
   }
   if (get_trust(ch) < LEVEL_IMM && ch->pcdata->caste < caste_Knight)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: [start time] [end time]   or just type start.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "start"))
   {
      do_chaos(ch, "1 5 1 8 1");
      return;
   }
   argument = one_argument(argument, arg1);

   if (isdigit(arg1[0]))
      v1 = atoi(arg1);
   if (isdigit(argument[0]))
      v2 = atoi(argument);

   if (v1 > 3)
   {
      send_to_char("Max time for start is 3 minutes.\n\r", ch);
      return;
   }
   if (v2 > 15)
   {
      send_to_char("Max time for a game is 15 minutes.\n\r", ch);
      return;
   }
   if (atoi(arg1) < 1 || atoi(argument) < 1)
   {
      send_to_char("Syntax: <start time> <end time>  (must be greater than 0)\n\r", ch);
      return;
   }

   sprintf(buf, "1 5 %d %d 1", v1, v2);
   do_chaos(ch, buf);
   return;
}



// Shows the rankings in the Battle Arena
void do_rankings(CHAR_DATA * ch, char *argument)
{
   BARENA_DATA *bdata;
   int x;
   int max = 0;
   int navg = 0;
   int times = 0;
   int fnd = 0;

   if (argument[0] == '\0')
   {
      send_to_char("rankings <argument>\n\rArena arguments = wins losses ties games numavg kills\n\rPkill arguments = pkills pdeaths pranking\n\r",
         ch);
      return;
   }
   if (!str_cmp(argument, "pkills"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->pkills > max)
         {
            max = bdata->pkills;
         }
      }
      if (max == 0)
      {
         send_to_char("There are no one with pkills yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  PKills    PDeaths   PRanking\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->pkills == x)
               {
                  ch_printf(ch, "&G&W%-20s  %-4d&c&w    &R [&C%-4d      %-4d    &R]\n\r", bdata->name, bdata->pkills, bdata->pdeaths,
                     bdata->pranking);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "pdeaths"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->pdeaths > max)
         {
            max = bdata->pdeaths;
         }
      }
      if (max == 0)
      {
         send_to_char("There are no one with pdeaths yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  PDeaths   PKills    PRanking\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->pdeaths == x)
               {
                  ch_printf(ch, "&G&W%-20s  %-4d&c&w    &R [&C%-4d      %-4d    &R]\n\r", bdata->name, bdata->pdeaths, bdata->pkills,
                     bdata->pranking);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "pranking"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->pranking > max)
         {
            max = bdata->pranking;
         }
      }
      if (max == 0)
      {
         send_to_char("There are no one with a pranking yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  PRanking  PKills    PDeaths\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->pranking == x)
               {
                  ch_printf(ch, "&G&W%-20s  %-4d&c&w   &R  [&C%-4d      %-4d   &R]\n\r", bdata->name, bdata->pranking, bdata->pkills, bdata->pdeaths);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "kills"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->kills > max)
         {
            max = bdata->kills;
         }
      }
      if (max == 0)
      {
         send_to_char("There are no one with kills yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  Kills     Wins  Losses  Ties  Games  Numavg\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->kills == x)
               {
                  navg = 0;
                  if (bdata->numavg && bdata->games)
                     navg = bdata->numavg / bdata->games;
                  ch_printf(ch, "&G&W%-20s  %-4d&c&w   &R [&C%-4d  %-4d    %-4d  %-5d  %-2d    &R]\n\r", bdata->name, bdata->kills, bdata->wins,
                     bdata->losses, bdata->ties, bdata->games, navg);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "wins"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->wins > max)
         {
            max = bdata->wins;
         }
      }
      if (max == 0)
      {
         send_to_char("There are no one with wins yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  Wins      Losses  Kills  Ties  Games  Numavg\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->wins == x)
               {
                  navg = 0;
                  if (bdata->numavg && bdata->games)
                     navg = bdata->numavg / bdata->games;
                  ch_printf(ch, "&G&W%-20s  %-4d&c&w    &R [&C%-4d    %-4d   %-4d  %-5d  %-2d    &R]\n\r", bdata->name, bdata->wins, bdata->losses,
                     bdata->kills, bdata->ties, bdata->games, navg);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "losses"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->losses > max)
         {
            max = bdata->losses;
         }
      }
      if (max == 0)
      {
         send_to_char("There are no one with losses yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  Losses    Wins   Kills  Ties  Games  Numavg\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->losses == x)
               {
                  navg = 0;
                  if (bdata->numavg && bdata->games)
                     navg = bdata->numavg / bdata->games;
                  ch_printf(ch, "&G&W%-20s  %-4d&c&w    &R [&C%-4d   %-4d   %-4d  %-5d  %-2d    &R]\n\r", bdata->name, bdata->losses, bdata->wins,
                     bdata->kills, bdata->ties, bdata->games, navg);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "ties"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->ties > max)
         {
            max = bdata->ties;
         }
      }
      if (max == 0)
      {
         send_to_char("There are no one with ties yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  Ties      Wins  Losses   Kills  Games  Numavg\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->ties == x)
               {
                  navg = 0;
                  if (bdata->numavg && bdata->games)
                     navg = bdata->numavg / bdata->games;
                  ch_printf(ch, "&G&W%-20s  %-4d&c&w    &R [&C%-4d  %-4d     %-4d   %-5d  %-2d    &R]\n\r", bdata->name, bdata->ties, bdata->wins,
                     bdata->losses, bdata->kills, bdata->games, navg);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "games"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->games > max)
         {
            max = bdata->games;
         }
      }
      if (max == 0)
      {
         send_to_char("No one has participated in Battle Arena yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  Games     Wins  Losses   Kills  Ties  Numavg\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; x--)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->games == x)
               {
                  navg = 0;
                  if (bdata->numavg && bdata->games)
                     navg = bdata->numavg / bdata->games;
                  ch_printf(ch, "&G&W%-20s  %-5d&c&w   &R [&C%-4d  %-4d     %-4d   %-4d  %-2d    &R]\n\r", bdata->name, bdata->games, bdata->wins,
                     bdata->losses, bdata->kills, bdata->ties, navg);
                  times++;
               }
            }
         }
      }
   }
   if (!str_cmp(argument, "numavg"))
   {
      for (bdata = first_barena; bdata; bdata = bdata->next)
      {
         if (bdata->numavg && bdata->games && bdata->numavg / bdata->games > max)
         {
            max = bdata->numavg / bdata->games;
         }
      }
      if (max == 0)
      {
         send_to_char("There is not a person with numavg yet.\n\r", ch);
         return;
      }
      else
      {
         send_to_char
            ("&G&WName                  Numavg    Wins  Losses   Kills  Ties  Games\n\r&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = max; x > 0; --x)
         {
            fnd = 0;
            if (times >= 10)
               return;
            for (bdata = first_barena; bdata; bdata = bdata->next)
            {
               if (bdata->numavg && bdata->games && bdata->numavg / bdata->games == x)
               {
                  navg = 0;
                  if (bdata->numavg && bdata->games)
                     navg = bdata->numavg / bdata->games;
                  ch_printf(ch, "&G&W%-20s  %-2d&c&w      &R [&C%-4d  %-4d     %-4d   %-4d  %-5d&R]\n\r", bdata->name, navg, bdata->wins,
                     bdata->losses, bdata->kills, bdata->ties, bdata->games);
                  times++;
               }
            }
         }
      }
   }
   return;
}

void do_bet(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MIL];
   char buf1[MIL];
   int newbet;

   argument = one_argument(argument, arg);
   one_argument(argument, buf1);

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cant bet on the arena.\r\n", ch);
      return;
   }

   if (arg[0] == '\0')
   {
      send_to_char("Usage: bet <player> <amt>\r\n", ch);
      return;
   }
   else if (!in_start_arena && !ppl_challenged)
   {
      send_to_char("Sorry the arena is closed, wait until it opens up to bet.\r\n", ch);
      return;
   }
   else if (ppl_in_arena)
   {
      send_to_char("Sorry Arena has already started, no more bets.\r\n", ch);
      return;
   }
   else if (ch->in_room->vnum == PREP_START)
      send_to_char("Sorry, you cannot bet while you are in the Arena.\n\r", ch);
   else if (!(ch->pcdata->betted_on = get_char_world(ch, arg)))
      send_to_char("No such person exists in Rafermand.\n\r", ch);
   else if (ch->pcdata->betted_on == ch)
      send_to_char("That doesn't make much sense, does it?\r\n", ch);
   else if (!IN_ARENA(ch->pcdata->betted_on))
      send_to_char("Sorry that person is not in the arena.\r\n", ch);
   else
   {
      if (GET_BET_AMT(ch) > 0)
      {
         send_to_char("Sorry you have already bet.\r\n", ch);
         return;
      }
      GET_BETTED_ON(ch) = ch->pcdata->betted_on;
      newbet = parsebet(bet_pot, buf1);
      if (newbet == 0)
      {
         send_to_char("Bet some gold why dont you!\r\n", ch);
         return;
      }
      if (newbet > ch->gold)
      {
         send_to_char("You don't have that much money!\n\r", ch);
         return;
      }
      if (newbet > 10000000)
      {
         send_to_char("Sorry the House max is 10 million.\r\n", ch);
         return;
      }

      ch->gold -= newbet;
      arena_pot += (newbet / 2);
      bet_pot += (newbet / 2);
      GET_BET_AMT(ch) = newbet;
      sprintf(buf, "You place %d coins on %s.\r\n", newbet, PERS_MAP(ch->pcdata->betted_on, ch));
      send_to_char(buf, ch);
      sprintf(buf, "&w&RINFO: &w&W$n has placed %d coins on $N.", newbet);
      act(AT_RED, buf, ch, NULL, ch->pcdata->betted_on, TO_MUD);
      act(AT_RED, buf, ch, NULL, ch->pcdata->betted_on, TO_VICT);
   }
}

/* Take care of updating the player */
void do_arena(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot play in the arena.\r\n", ch);
      return;
   }
   if (!in_start_arena)
   {
      send_to_char("The killing fields are closed right now.\r\n", ch);
      return;
   }
   
   if (get_player_statlevel(ch) < lo_lim || get_player_statlevel(ch) > hi_lim)
   {
      send_to_char("You are not in the required statlevel to fight in the arena.\n\r", ch);
      return;
   }

   if (IN_ARENA(ch))
   {
      send_to_char("You are in the arena already\r\n", ch);
      return;
   }
   else
   {
      act(AT_RED, "$n has been whisked away to the killing fields.", ch, NULL, NULL, TO_ROOM);
      update_players_map(ch, -1, -1, -1, 1, get_room_index(PREP_START));
      act(AT_WHITE, "$n is droped from the sky.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You have been taken to the killing fields\r\n", ch);
      do_look(ch, "auto");
      act(AT_RED, "&w&RINFO: &w&W$n has joined the blood bath.", ch, NULL, NULL, TO_MUD);
      ppl_at_start++;
      ch->hit = ch->max_hit;
      ch->mana = ch->max_mana;
      ch->move = ch->max_move;
      return;
   }
}

void do_chaos(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL], arg2[MIL], arg3[MIL];

   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  chaos <low plevel limit> <hi plevel limit> <start delay> <round length>\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   
   lo_lim = atoi(arg1);
   hi_lim = atoi(arg2);
   start_time = atoi(arg3);
   game_length = atoi(argument);
   
   if ((lo_lim || hi_lim || start_time || game_length) <= 0)
   {
      send_to_char("All values must be greater than 0.\r\n", ch);
      return;
   }

   if (lo_lim >= hi_lim)
   {
      send_to_char("Low Level needs to be less than High Level.\r\n", ch);
      return;
   }

   ppl_in_arena = 0;
   in_start_arena = 1;
   time_to_start = start_time;
   time_left_in_game = 0;
   ppl_at_start = 0;
   arena_pot = 0;
   bet_pot = 0;
   barena = 1;
   start_arena();

}

void start_arena()
{
   char buf1[MIL];
   char buf[MIL];
   DESCRIPTOR_DATA *d;

   if (!(ppl_challenged))
   {
      if (time_to_start == 0)
      {
         in_start_arena = 0;
         show_jack_pot();
         ppl_in_arena = 1; /* start the blood shed */
         time_left_in_game = game_length;
         start_game();
         return;
      }
      else
      {
         if (time_to_start > 1)
         {
            sprintf(buf1, "&WThe Killing Fields are open to Power levels &R%d &Wthru &R%d\r\n", lo_lim, hi_lim);
            sprintf(buf1, "%s%d &Whours to start\r\n", buf1, time_to_start);
            sprintf(buf1, "%s\r\nType &Rarena &Wto enter.\r\n", buf1);
         }
         else
         {
            sprintf(buf1, "&WThe Killing Fields are open to Power levels &R%d &Wthru &R%d\r\n", lo_lim, hi_lim);
            sprintf(buf1, "%s1 &Whour to start\r\n", buf1);
            sprintf(buf1, "%s\r\nType &Rarena &Wto enter.\r\n", buf1);
         }
      }
      for (d = first_descriptor; d; d = d->next)
      {
         if (d->connected == CON_PLAYING)
         {
            if ((d->character->level >= lo_lim && d->character->level <= hi_lim) || game_type == 1)
               send_to_char(buf1, d->character);
            else
            {
               sprintf(buf, "&WThe arena has been opened. &R%d &Whour(s) to start.\r\n", time_to_start);
               sprintf(buf, "%sPlace your bets!!!\r\n", buf);
               send_to_char(buf, d->character);
            }
         }
      }
      /* echo_to_all(AT_WHITE, buf1, ECHOTAR_ALL); */
      time_to_start--;
   }
   else
   {
      if (!(ppl_in_arena))
      {
         if (time_to_start == 0)
         {
            ppl_challenged = 0;
            show_jack_pot();
            ppl_in_arena = 1; /* start the blood shed */
            time_left_in_game = 5;
            start_game();
         }
         else
         {
            if (time_to_start > 1)
            {
               sprintf(buf1, "&w&RINFO: &w&WThe dual will start in %d hours. Place your bets!", time_to_start);
            }
            else
            {
               sprintf(buf1, "&w&RINFO: &w&WThe dual will start in 1 hour. Place your bets!");
            }
            act(AT_RED, buf1, supermob, NULL, NULL, TO_MUD);
            time_to_start--;
         }
      }
   }
}

void start_game()
{
   CHAR_DATA *i;
   DESCRIPTOR_DATA *d;
   int toroom;

   for (d = first_descriptor; d; d = d->next)
      if (!d->connected)
      {
         i = d->character;
         if (d->connected == CON_PLAYING)
         {
            if (i && i->in_room && i->in_room->vnum == PREP_START)
            {
               toroom = number_range(ARENA_SVNUM, ARENA_EVNUM);
               send_to_char("\r\nThe floor falls out from bellow, droping you in the arena\r\n", i);
               update_players_map(i, -1, -1, -1, 1, get_room_index(toroom));
               do_look(i, "auto");
               //type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 
               update_barena(i, 5);
               if (num_in_arena() > 1)
               {
                  update_barena(i, 3);
                  update_barena(i, 4);
               }
            }
         }
      }
   do_game();
}

void do_game()
{
   char buf[MIL];

   if (num_in_arena() == 1)
   {
      ppl_in_arena = 0;
      ppl_challenged = 0;
      find_game_winner();
   }
   else if (time_left_in_game == 0)
   {
      do_end_game();
   }
   else if (num_in_arena() == 0)
   {
      ppl_in_arena = 0;
      ppl_challenged = 0;
      silent_end();
   }
   else if (time_left_in_game % 5)
   {
      sprintf(buf, "&w&RINFO: &w&WWith %d hours left in the game there are %d players left.", time_left_in_game, num_in_arena());
   }
   else if (time_left_in_game == 1)
   {
      sprintf(buf, "&w&RINFO: &w&WWith 1 hour left in the game there are %d players left.", num_in_arena());
   }
   else if (time_left_in_game <= 4)
   {
      sprintf(buf, "&w&RINFO: &w&WWith %d hours left in the game there are %d players left.", time_left_in_game, num_in_arena());
   }
   act(AT_RED, buf, supermob, NULL, NULL, TO_MUD);
   time_left_in_game--;
}

void find_game_winner()
{
   char buf[MIL];
   char buf2[MIL];
   CHAR_DATA *i;
   DESCRIPTOR_DATA *d;
   int vnum = 0;

   for (d = first_descriptor; d; d = d->next)
   {
      i = d->character;
      if (d->connected == CON_PLAYING)
      {
         if (IN_ARENA(i) && (i->level < LEVEL_IMMORTAL))
         {
            i->hit = i->max_hit;
            i->mana = i->max_mana;
            i->move = i->max_move;
            i->pcdata->challenged = NULL;

            if (!IS_NPC(i) && i->pcdata->clan)
               vnum = i->pcdata->clan->recall;

            if (!vnum)
               vnum = ROOM_VNUM_TEMPLE;

            char_from_room(i);
            char_to_room(i, get_room_index(vnum));
            do_look(i, "auto");
            act(AT_YELLOW, "$n falls from the sky.", i, NULL, NULL, TO_ROOM);
            if (time_left_in_game == 1)
            {
               act(AT_RED, "&w&RINFO: &w&WAfter 1 hour of battle $n is declared the winner", i, NULL, NULL, TO_MUD);
            }
            else
            {
               sprintf(buf, "&w&RINFO: &w&WAfter %d hours of battle $n is declared the winner", game_length - time_left_in_game);
               act(AT_RED, buf, i, NULL, NULL, TO_MUD);
            }
            i->gold += arena_pot;
            sprintf(buf, "You have been awarded %d coins for winning the arena\r\n", (arena_pot));
            send_to_char(buf, i);
            sprintf(buf2, "&w&RINFO: &w&W$n awarded %d coins for winning arena", (arena_pot));
            act(AT_RED, buf2, i, NULL, NULL, TO_MUD);
            find_bet_winners(i, TRUE);
            //type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 
            if (ppl_at_start > 1)
               update_barena(i, 0);
            ppl_in_arena = 0;
            ppl_challenged = 0;
         }
      }
   }
   save_barena_data();
}

void show_jack_pot()
{
   char buf1[MIL];

   sprintf(buf1, "\r\nMay the CHAOS BEGIN!!!!!!!!!\r\n");
   sprintf(buf1, "%sThe jack pot for this arena is %d coins\r\n", buf1, arena_pot);
   sprintf(buf1, "%s%d coins have been bet on this arena.\r\n\r\n", buf1, bet_pot);
   echo_to_all(AT_WHITE, buf1, ECHOTAR_ALL);

}

void silent_end()
{
   ppl_in_arena = 0;
   ppl_challenged = 0;
   in_start_arena = 0;
   start_time = 0;
   game_length = 0;
   time_to_start = 0;
   time_left_in_game = 0;
   arena_pot = 0;
   bet_pot = 0;
   act(AT_RED, "&w&RINFO: &w&WIt looks like no one was brave enough to enter the Arena.", supermob, NULL, NULL, TO_MUD);
}

void do_end_game()
{
   char buf[MIL];
   CHAR_DATA *i;
   DESCRIPTOR_DATA *d;

   for (d = first_descriptor; d; d = d->next)
      if (!d->connected)
      {
         i = d->character;
         if (d->connected == CON_PLAYING)
         {
            if (IN_ARENA(i))
            {
               i->hit = i->max_hit;
               i->mana = i->max_mana;
               i->move = i->max_move;
               i->pcdata->challenged = NULL;
               stop_fighting(i, TRUE);
               char_from_room(i);
               char_to_room(i, get_room_index(ROOM_VNUM_TEMPLE));
               do_look(i, "auto");
               act(AT_TELL, "$n falls from the sky.", i, NULL, NULL, TO_ROOM);
               //type 0 - win 1 - losses 2 - ties 3 - games 4 - numavg 5 - update 
               update_barena(i, 2);
            }
         }
      }
   sprintf(buf, "&w&RINFO: &w&WAfter %d hours of battle the Match is a draw", game_length);
   act(AT_RED, buf, NULL, NULL, NULL, TO_MUD);
   find_bet_winners(NULL, FALSE);
   time_left_in_game = 0;
   ppl_in_arena = 0;
   ppl_challenged = 0;
   save_barena_data();
}

int num_in_arena()
{
   CHAR_DATA *i;
   DESCRIPTOR_DATA *d;
   int num = 0;

   for (d = first_descriptor; d; d = d->next)
   {
      i = d->character;
      if (d->connected == CON_PLAYING)
      {
         if (IN_ARENA(i))
         {
            if (i->level < LEVEL_IMMORTAL)
               num++;
         }
      }
   }
   return num;
}

void do_awho(CHAR_DATA * ch, char *argument)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *tch;
   char buf[MIL];
   char buf2[MIL];
   char buf3[MIL];
   int num = num_in_arena();

   if (num == 0)
   {
      send_to_char("There is no one in the arena right now.\r\n", ch);
      return;
   }

   sprintf(buf, "&W  Players in the &BRafermand&W Arena\r\n");
   sprintf(buf, "%s-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-", buf);
   sprintf(buf, "%s&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-\r\n", buf);
   sprintf(buf, "%sGame Length = &R%-3d   &WTime To Start &R%-3d\r\n", buf, game_length, time_to_start);
   sprintf(buf, "%s&WLevel Limits &R%d &Wto &R%d\r\n", buf, lo_lim, hi_lim);
   sprintf(buf, "%s         &WJackpot = &R%d\r\n", buf, arena_pot);
   sprintf(buf, "%s&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B", buf);
   sprintf(buf, "%s-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B-&W-&B\r\n", buf);
   send_to_char(buf, ch);
   for (d = first_descriptor; d; d = d->next)
      if (!d->connected)
      {
         if (d->connected == CON_PLAYING)
         {
            tch = d->character;
            if (IN_ARENA(tch) && (tch->level < LEVEL_IMMORTAL))
            {
               if (tch->pcdata->clan)
               {
                  CLAN_DATA *pclan = tch->pcdata->clan;

                  strcpy(buf3, pclan->name);
               }
               else
                  strcpy(buf3, "");
               sprintf(buf2, "&W%s         %-11.11s\n\r", PERS_MAP(tch, ch), buf3);
               send_to_char(buf2, ch);
            }
         }
      }
   return;
}

void do_ahall(CHAR_DATA * ch, char *argument)
{
   return;
   /*
      char site[MIL], format[MAX_INPUT_LENGTH], *timestr;
      char format2[MIL];
      struct hall_of_fame_element *fame_node;

      char buf[MIL];
      char buf2[MIL];

      if (!fame_list)
      {
      send_to_char("No-one is in the Hall of Fame.\r\n", ch);
      return;
      }

      sprintf(buf2, "&B|---------------------------------------|\r\n");
      strcat(buf2, "| &WPast Winners of The Rafermand Arena&B  |\r\n");
      strcat(buf2, "|---------------------------------------|\r\n\r\n"); 

      send_to_char(buf2, ch);
      strcpy(format, "%-25.25s  %-10.10s  %-16.16s\r\n");
      sprintf(buf, format,
      "&RName",
      "&RDate",
      "&RAward Amt");
      send_to_char(buf, ch);
      sprintf(buf, format,
      "&B---------------------------------",
      "&B---------------------------------",
      "&B---------------------------------");

      send_to_char(buf, ch);
      strcpy(format2, "&W%-25.25s  &R%-10.10s  &Y%-16d\r\n");
      for (fame_node = fame_list; fame_node; fame_node = fame_node->next)
      {
      if (fame_node->date)
      {
      timestr = asctime(localtime(&(fame_node->date)));
      *(timestr + 10) = 0;
      strcpy(site, timestr);
      }
      else
      strcpy(site, "Unknown");
      sprintf(buf, format2, fame_node->name, site, fame_node->award);
      send_to_char(buf, ch);
      }
      return; */
}


/* Type = 0(False) - Tie    1(True) - Winner */
void find_bet_winners(CHAR_DATA * winner, bool type)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *wch;

   char buf1[MIL];

   for (d = first_descriptor; d; d = d->next)
      if (!d->connected)
      {
         wch = d->character;
         if (type == TRUE)
         {
            if ((!IS_NPC(wch)) && (GET_BET_AMT(wch) > 0) && (GET_BETTED_ON(wch) == winner))
            {
               sprintf(buf1, "You have won %d coins on your bet.\r\n", (GET_BET_AMT(wch)) * 2);
               send_to_char(buf1, wch);
               wch->gold += GET_BET_AMT(wch) * 2;
               GET_BETTED_ON(wch) = NULL;
               GET_BET_AMT(wch) = 0;
            }
            else
            {
               GET_BETTED_ON(wch) = NULL;
               GET_BET_AMT(wch) = 0;
            }
         }
         else
         {
            if ((!IS_NPC(wch)) && (GET_BET_AMT(wch) > 0))
            {
               sprintf(buf1, "Due to a tie, you will be returned %d coins on your bet.\n\r", GET_BET_AMT(wch));
               send_to_char(buf1, wch);
               wch->gold += GET_BET_AMT(wch);
               GET_BETTED_ON(wch) = NULL;
               GET_BET_AMT(wch) = 0;
            }
            else
            {
               GET_BETTED_ON(wch) = NULL;
               GET_BET_AMT(wch) = 0;
            }
         }

      }
}

void do_challenge(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char buf[MIL];
   char arg1[MIL];
   int type = 0;


   if (argument[0] == '\0')
   {
      send_to_char("Syntax: challenge <player>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);

   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("&WThat character is not of these realms!\n\r", ch);
      return;
   }
   if (((ch->level > LEVEL_IMMORTAL) || (victim->level > LEVEL_IMMORTAL)) && type != 1)
   {
      send_to_char("Sorry, Immortal's are not allowed to participate in the arena.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("&WYou cannot challenge mobiles!\n\r", ch);
      return;
   }
   if (victim->name == ch->name)
   {
      send_to_char("&WYou cannot challenge yourself!", ch);
      return;
   }

   if (num_in_arena() > 0)
   {
      send_to_char("&WSomeone is already in the arena!\n\r", ch);
      return;
   }
   sprintf(buf, "&R%s &Whas challenged you to a non-fatal dual!\n\r", PERS_MAP(ch, victim));
   send_to_char(buf, victim);
   send_to_char("&WPlease either accept or decline the challenge.\n\r\n\r", victim);
  
   act(AT_RED, "&w&RINFO:  &w&W$n has challenged $N to a non-fatal dual!!!", ch, NULL, victim, TO_MUD);
   victim->pcdata->challenged = ch;
}

void do_accept(CHAR_DATA * ch, char *argument)
{
   if (num_in_arena() > 0)
   {
      send_to_char("Please wait until the current arena is closed before you accept.\n\r", ch);
      return;
   }
   if (IS_NPC(ch))
   {
      send_to_char("You cannot fight in the arena silly mob.\n\r", ch);
      return;
   }

   if (!(ch->pcdata->challenged))
   {
      send_to_char("You have not been challenged!\n\r", ch);
      return;
   }
   else
   {
      CHAR_DATA *dch;

      dch = ch->pcdata->challenged;
      act(AT_RED, "&w&RINFO:  &w&W$n has accepted $N's challenge!", ch, NULL, dch, TO_MUD);
      ch->pcdata->challenged = NULL;
      update_players_map(ch, -1, -1, -1, 1, get_room_index(PREP_START));
      do_look(ch, "auto");
      update_players_map(dch, -1, -1, -1, 1, get_room_index(PREP_START));
      do_look(dch, "auto");
      barena = 0;
      ppl_in_arena = 0;
      ppl_challenged = 1;
      time_to_start = 2;
      ppl_at_start = 2;
      time_left_in_game = 0;
      arena_pot = 0;
      bet_pot = 0;
      start_arena();
      return;
   }
}

void do_decline(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("You cannot fight in the Arena you silly mob.\n\r", ch);
      return;
   }
   if (ch->pcdata->challenged)
   {
      act(AT_RED, "&w&RINFO: &w&W$n has DECLINED %N's challenge! WHAT A WUSS!!!\n\r", ch, NULL, ch->pcdata->challenged, TO_MUD);
      ch->pcdata->challenged = NULL;
      return;
   }
   else
   {
      send_to_char("You have not been challenged!\n\r", ch);
      return;
   }
}
