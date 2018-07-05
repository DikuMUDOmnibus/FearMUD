/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *			   Wizard/god command module			    *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include "mud.h"


#define RESTORE_INTERVAL 21600

char *const save_flag[] = { "death", "kill", "passwd", "drop", "put", "give", "auto", "zap",
   "auction", "get", "receive", "idle", "backup", "quitbackup", "r14", "r15",
   "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
   "r27", "r28", "r29", "r30", "r31"
};

void save_sysdata args((SYSTEM_DATA sys));

/* from comm.c */
bool write_to_descriptor args((int desc, char *txt, int length));
bool check_parse_name args((char *name, bool newchar));

/* from boards.c */
void note_attach(CHAR_DATA * ch);

/* from build.c */
int get_risflag(char *flag);
int get_defenseflag(char *flag);
int get_attackflag(char *flag);

/* from tables.c */
void write_race_file(int ra);


/*
 * Local functions.
 */
ROOM_INDEX_DATA *find_location args((CHAR_DATA * ch, char *arg));
void save_watchlist args((void));
void save_banlist args((void));
void close_area args((AREA_DATA * pArea));

int get_color(char *argument); /* function proto */

void sort_reserved args((RESERVE_DATA * pRes));

PROJECT_DATA *get_project_by_number args((int pnum));
NOTE_DATA *get_log_by_number args((PROJECT_DATA * pproject, int pnum));

/*
 * Global variables.
 */

char reboot_time[50];
time_t new_boot_time_t;
extern struct tm new_boot_struct;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
time_t copyover_time;

int get_saveflag(char *name)
{
   int x;

   for (x = 0; x < sizeof(save_flag) / sizeof(save_flag[0]); x++)
      if (!str_cmp(name, save_flag[x]))
         return x;
   return -1;
}

int csizeof(char *argument)
{
   int x;
   
   for (x = 0;; x++)
   {
      if (argument[x] == '\0')
         break;
   }
   return x;
}
void showpic(CHAR_DATA *ch, int type, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   char arg2[MIL];
   DESCRIPTOR_DATA *d;
   DESCRIPTOR_DATA *dnext;
   int needhttp = 0;
   int x;
   int y = 0;
   int z;
   
   //Need atleast 4 characters
   if (csizeof(argument) < 4)
   {
      send_to_char("Invalid picture text count.\n\r", ch);
      return;
   }
   //Check to see if http was provided, if not it will be included later
   if (argument[0] != 'h' || argument[1] != 't' || argument[2] != 't' || argument[3] != 'p')
      needhttp = 1;
   //Checking to see if a FULL URL was provided or not
   for (x = 0;;)
   {
      if (argument[x] == '/')
         y = 1;
      if (argument[x] == '\0')
         break;
      x++;
   }
   //Player send argument and a FULL URL was passed, so lets not honor it.
   if (y == 1 && type == 3)
      return;
   if (y == 1)
   {   
      y = 0;
      for (x = 0;;)
      {
         //Found end of the string, lets to backwords so we can get the actual file name
         if (argument[x] == '\0')
         {
            for (;;)
            {
               x--;
               //File name found
               if (argument[x] == '/')
               {
                  z = x;
                  for (;;)
                  {
                     x++;
                     //Going forward to pick up the actual file name, outputing to arg (ex: picture.jpg)
                     if (argument[x] == '\0')
                     {
                        arg[y] = '\0';
                        break;
                     }
                     if (isspace(argument[x]))
                     {
                        continue;
                     }
                     arg[y++] = argument[x];
                  }
                  y = 0;
                  //Getting the path name and putting it in arg (ex: http://www.rafermand.net/)
                  for (;;)
                  {
                     if (y == z)
                     {
                        arg2[y] = argument[y];
                        arg2[y+1] = '\0';
                        break;
                     }
                     arg2[y] = argument[y];   
                     y++;
                  }
                  break;
               }
            }
            break;
         }
         x++;
      } 
      y = 1;  
   }
   
   //If 1 we are passing a full URL if not then just a cached picture request
   if (y == 1)
      sprintf(buf, "\x03Image fname=\"%s\" url=\"%s%s\" align=bottom\x04", arg, needhttp ? "http://" : "", arg2);
   else
      sprintf(buf, "\x03Image %s align=bottom\x04", argument);
      
   for (d = first_descriptor; d; d = dnext)
   {
      dnext = d->next;
      //Check only playing characters
      if (d->connected == CON_PLAYING && d->character)
      {         
         //If type 1 we are only looking for players in the same room
         if (type == 1 && !IN_SAME_ROOM(ch, d->character))
            continue;         
         act(AT_WHITE, buf, d->character, NULL, NULL, TO_CHAR);
      }
   }
   return;
}

void do_showpic(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   int type = 0;
   
   if (argument[0] == '\0')
   {
      if (IS_IMMORTAL(ch))
      {
         send_to_char("showpic <type> <picture>\n\r", ch);
         send_to_char("type = room or all\n\r", ch);
         return;
      }
      else
      {
         send_to_char("showpic <picture>\n\r", ch);
         return;
      }
   }
   //If isn't immortal send the player options
   if (!IS_IMMORTAL(ch))
   {
      showpic(ch, 3, argument);
      return;
   }
   argument = one_argument(argument, arg1);
   
   if (!str_cmp(arg1, "room"))
      type = 1;
   if (!str_cmp(arg1, "all"))
      type = 2;
   if (type == 0)
   {
      send_to_char("Not a valid type.\n\r", ch);
      return;
   }
   showpic(ch, type, argument);
   return;
}

void do_fightoutput(CHAR_DATA *ch, char *argument)
{
   int x, y, z;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  fightoutput <hit, miss, dodge, parry, block, crit, insta>\n\r", ch);
      send_to_char("Syntax:  fightoutput <hit, miss, dodge, parry, block, crit, insta> add <bash/slash/stab> <text to add>\n\r", ch);
      send_to_char("Syntax:  fightoutput <hit, miss, dodge, parry, block, crit, insta> remove <bash/slash/stab> <number>\n\r", ch);
      send_to_char("Syntax:  fightoutput <hit, miss, dodge, parry, block, crit, insta> edit <bash/slash/stab> <number> <text to add>\n\r", ch);
      send_to_char("Syntax:  fightoutput variables\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   
   if (!str_cmp(arg1, "variables"))
   {
      send_to_char("$B - Limb of the Target\n\r", ch);
      send_to_char("$n - One doing the attacking\n\r", ch);
      send_to_char("$N - One being attacked\n\r", ch);
      send_to_char("$G - Grip of the attacker\n\r", ch);
      send_to_char("$W - Weapon of the attacker\n\r", ch);
      send_to_char("$w - Weapon of the attackee\n\r", ch);
      send_to_char("$p - One doing the attacking (possessive)\n\r", ch);
      send_to_char("$P - One being attacked (possessive)\n\r", ch);
      send_to_char("$e - He/She/It/You (Attacker)\n\r", ch);
      send_to_char("$m - Him/Her/It/You (Attacker)\n\r", ch);
      send_to_char("$s - His/Her/Its/Your (Attacker)\n\r", ch);
      send_to_char("$E - He/She/It/You (Attackee)\n\r", ch);
      send_to_char("$M - Him/Her/It/You (Attackee)\n\r", ch);
      send_to_char("$S - His/Her/Its/Your (Attackee)\n\r", ch);
      send_to_char("$b - Name of Shield used to block\n\r", ch);
      send_to_char("$a - Adds s to the end of a word (for room and ch)\n\r", ch);
      send_to_char("$A - Adds es to the end of a word (for room and ch)\n\r", ch);
      send_to_char("$o - Adds s to the end of a word (for room and victim)\n\r", ch);
      send_to_char("$O - Adds es to the end of a word (for room and victim)\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg1, "hit"))
      z = 0;
   else if (!str_cmp(arg1, "miss"))
      z = 1;
   else if (!str_cmp(arg1, "block"))
      z = 2;
   else if (!str_cmp(arg1, "insta"))
      z = 6;
   else
      return;
      
   if (!str_cmp(arg2, "remove"))
   {
      if (!str_cmp(arg3, "bash"))
         x = 0;
      else if (!str_cmp(arg3, "slash"))
         x = 1;
      else if (!str_cmp(arg3, "stab"))
         x = 2;
      else
      {
         send_to_char("That is not valid, your choices are:  bash, slash, stab\n\r", ch);
         return;
      }
      if (atoi(argument) >= high_value[z][x])
      {
         send_to_char("That number does not exist.\n\r", ch);
         return;
      }
      if (atoi(argument) == 0)
      {
         send_to_char("You cannot remove the last entry, you can edit it though.\n\r", ch);
         return;
      }
      if (atoi(argument) == high_value[z][x]-1)
      {
         strcpy((char *) battle_descriptions[z][x][atoi(argument)], "");
         high_value[z][x]--;
         fwrite_battle_descriptions();
      }
      else
      {
         int dx;
         dx = atoi(argument);
         for (;;)
         {
            if (dx == high_value[z][x])
            {
               strcpy((char *) battle_descriptions[z][x][dx-1], "");
               high_value[z][x]--;
               fwrite_battle_descriptions();
               break;
            }
            else
            {
               sprintf((char *) battle_descriptions[z][x][dx], (char *) battle_descriptions[z][x][dx+1]);
               dx++;
            }
         }
      }
   }      
   if (!str_cmp(arg2, "edit"))
   {
      argument = one_argument(argument, arg4);
      if (!str_cmp(arg3, "bash"))
         x = 0;
      else if (!str_cmp(arg3, "slash"))
         x = 1;
      else if (!str_cmp(arg3, "stab"))
         x = 2;
      else
      {
         send_to_char("That is not valid, your choices are:  bash, slash, stab\n\r", ch);
         return;
      }
      if (atoi(arg4) >= high_value[z][x])
      {
         send_to_char("That number does not exist.\n\r", ch);
         return;
      }
      if (strlen(argument) > 59)
      {
         send_to_char("You can only have 59 characters or less.\n\r", ch);
      }
      else
      {
         sprintf((char *) battle_descriptions[z][x][atoi(arg4)], argument);
         fwrite_battle_descriptions();
      }
   }
   if (!str_cmp(arg2, "add"))
   {
      if (!str_cmp(arg3, "bash"))
         x = 0;
      else if (!str_cmp(arg3, "slash"))
         x = 1;
      else if (!str_cmp(arg3, "stab"))
         x = 2;
      else
      {
         send_to_char("That is not valid, your choices are:  bash, slash, stab\n\r", ch);
         return;
      }
      if (high_value[0][x] >= 100)
      {
         send_to_char("You already have 100 on that grouping, you cannot add anymore.\n\r", ch);
         return;
      }
      if (strlen(argument) > 59)
      {
         send_to_char("You can only have 59 characters or less.\n\r", ch);
      }
      else
      {
         sprintf((char *) battle_descriptions[z][x][high_value[z][x]], argument);
         high_value[z][x]++;
         fwrite_battle_descriptions();
      }
   }            
   if (z == 0)
      send_to_char("&w&YHit Output\n\r\n\r", ch);
   else if (z == 1)
      send_to_char("&w&YMiss Output\n\r\n\r", ch);
   else if (z == 2)
      send_to_char("&w&YBlock Output\n\r\n\r", ch);
   else if (z == 6)
      send_to_char("&w&YInstant Kill Output\n\r\n\r", ch);
      
   for (x = 0; x < 3; x++)
   {
      for (y = 0; y < 100; y++)
      {
         if (y == 0)
         {
            if (x == 0)
               send_to_char("&w&W--------------------------------------&w&RBASH&w&W--------------------------------------\n\r", ch);
            if (x == 1)
               send_to_char("&w&W--------------------------------------&w&BSLASH&w&W-------------------------------------\n\r", ch);
            if (x == 2)
               send_to_char("&w&W--------------------------------------&w&CSTAB&w&W--------------------------------------\n\r", ch);
         }
         if (battle_descriptions[z][x][y][0] != '\0')
         {
            ch_printf(ch, "&c&w%3d > %s\n\r", y, battle_descriptions[z][x][y]);
         }
      }
   }
   return;
}

/*
 * The "watch" facility allows imms to specify the name of a player or
 * the name of a site to be watched. It is like "logging" a player except
 * the results are written to a file in the "watch" directory named with
 * the same name as the imm. The idea is to allow lower level imms to
 * watch players or sites without having to have access to the log files.
 */
void do_watch(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char arg3[MIL];
   WATCH_DATA *pw;

   if (IS_NPC(ch))
      return;

   argument = one_argument(argument, arg);
   set_pager_color(AT_IMMORT, ch);

   if (arg[0] == '\0' || !str_cmp(arg, "help"))
   {
      send_to_pager("Syntax Examples:\n\r", ch);
      /*
       * Only Staff can see all the watches. The rest can just see their own.
       */
      if (get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
         send_to_pager("   watch show all          show all watches\n\r", ch);
      send_to_pager("   watch show              show all my watches\n\r"
         "   watch size              show the size of my watch file\n\r"
         "   watch player joe        add a new player watch\n\r"
         "   watch site 2.3.123      add a new site watch\n\r"
         "   watch command make      add a new command watch\n\r"
         "   watch site 2.3.12       matches 2.3.12x\n\r"
         "   watch site 2.3.12.      matches 2.3.12.x\n\r"
         "   watch delete n          delete my nth watch\n\r"
         "   watch print 500         print watch file starting at line 500\n\r"
         "   watch print 500 1000    print 1000 lines starting at line 500\n\r" "   watch clear             clear my watch file\n\r", ch);
      return;
   }

   set_pager_color(AT_PLAIN, ch);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);

/*
 * Clear watch file
 */
   if (!str_cmp(arg, "clear"))
   {
      char fname[MIL];

      sprintf(fname, "%s%s", WATCH_DIR, strlower(ch->name));
      if (0 == remove(fname))
      {
         send_to_pager("Ok. Your watch file has been cleared.\n\r", ch);
         return;
      }
      send_to_pager("You have no valid watch file to clear.\n\r", ch);
      return;
   }

/*
 * Display size of watch file
 */
   if (!str_cmp(arg, "size"))
   {
      FILE *fp;
      char fname[MIL], s[MSL];
      int rec_count = 0;

      sprintf(fname, "%s%s", WATCH_DIR, strlower(ch->name));

      if (!(fp = fopen(fname, "r")))
      {
         send_to_pager("You have no watch file. Perhaps you cleared it?\n\r", ch);
         return;
      }

      fgets(s, MSL, fp);
      while (!feof(fp))
      {
         rec_count++;
         fgets(s, MSL, fp);
      }
      pager_printf(ch, "You have %d lines in your watch file.\n\r", rec_count);
      fclose(fp);
      return;
   }

/*
 * Print watch file
 */
   if (!str_cmp(arg, "print"))
   {
      FILE *fp;
      char fname[MIL], s[MSL];
      const int MAX_DISPLAY_LINES = 1000;
      int start, limit, disp_count = 0, rec_count = 0;

      if (arg2[0] == '\0')
      {
         send_to_pager("Sorry. You must specify a starting line number.\n\r", ch);
         return;
      }

      start = atoi(arg2);
      limit = (arg3[0] == '\0') ? MAX_DISPLAY_LINES : atoi(arg3);
      limit = UMIN(limit, MAX_DISPLAY_LINES);

      sprintf(fname, "%s%s", WATCH_DIR, strlower(ch->name));
      if (!(fp = fopen(fname, "r")))
         return;
      fgets(s, MSL, fp);

      while ((disp_count < limit) && (!feof(fp)))
      {
         if (++rec_count >= start)
         {
            send_to_pager(s, ch);
            disp_count++;
         }
         fgets(s, MSL, fp);
      }
      send_to_pager("\n\r", ch);
      if (disp_count >= MAX_DISPLAY_LINES)
         send_to_pager("Maximum display lines exceeded. List is terminated.\n\r"
            "Type 'help watch' to see how to print the rest of the list.\n\r"
            "\n\r" "Your watch file is large. Perhaps you should clear it?\n\r", ch);

      fclose(fp);
      return;
   }

/*
 * Display all watches
 * Only STAFF can see all the watches. The rest can just see their own.
 */
   if (get_trust(ch) >= LEVEL_HI_STAFF /* Tracker1 */
      && !str_cmp(arg, "show") && !str_cmp(arg2, "all"))
   {
      pager_printf(ch, "%-12s %-14s %-15s\n\r", "Imm Name", "Player/Command", "Player Site");
      if (first_watch)
         for (pw = first_watch; pw; pw = pw->next)
            if (get_trust(ch) >= pw->imm_level)
               pager_printf(ch, "%-14s %-12s %-15s\n\r",
                  pw->imm_name, pw->target_name ? pw->target_name : " ", pw->player_site ? pw->player_site : " ");
      return;
   }

/*
 * Display only those watches belonging to the requesting imm 
 */
   if (!str_cmp(arg, "show") && arg2[0] == '\0')
   {
      int cou = 0;

      pager_printf(ch, "%-3s %-12s %-14s %-15s\n\r", " ", "Imm Name", "Player/Command", "Player Site");
      if (first_watch)
         for (pw = first_watch; pw; pw = pw->next)
            if (!str_cmp(ch->name, pw->imm_name))
               pager_printf(ch, "%3d %-12s %-14s %-15s\n\r",
                  ++cou, pw->imm_name, pw->target_name ? pw->target_name : " ", pw->player_site ? pw->player_site : " ");
      return;
   }

/*
 * Delete a watch belonging to the requesting imm
 */
   if (!str_cmp(arg, "delete") && isdigit(*arg2))
   {
      int cou = 0;
      int num;

      num = atoi(arg2);
      if (first_watch)
         for (pw = first_watch; pw; pw = pw->next)
            if (!str_cmp(ch->name, pw->imm_name))
               if (num == ++cou)
               {
                  /* Oops someone forgot to clear up the memory --Shaddai */
                  if (pw->imm_name)
                     DISPOSE(pw->imm_name);
                  if (pw->player_site)
                     DISPOSE(pw->player_site);
                  if (pw->target_name)
                     DISPOSE(pw->target_name);
                  /* Now we can unlink and then clear up that final
                   * pointer -- Shaddai 
                   */
                  UNLINK(pw, first_watch, last_watch, next, prev);
                  DISPOSE(pw);
                  save_watchlist();
                  send_to_pager("Deleted.\n\r", ch);
                  return;
               }
      send_to_pager("Sorry. I found nothing to delete.\n\r", ch);
      return;
   }

/*
 * Watch a specific player
 */
   if (!str_cmp(arg, "player") && *arg2)
   {
      WATCH_DATA *pinsert;
      CHAR_DATA *vic;
      char buf[MIL];

      if (first_watch) /* check for dups */
         for (pw = first_watch; pw; pw = pw->next)
            if (!str_cmp(ch->name, pw->imm_name) && pw->target_name && !str_cmp(arg2, pw->target_name))
            {
               send_to_pager("You are already watching that player.\n\r", ch);
               return;
            }

      CREATE(pinsert, WATCH_DATA, 1); /* create new watch */
      pinsert->imm_level = get_trust(ch);
      pinsert->imm_name = str_dup(strlower(ch->name));
      pinsert->target_name = str_dup(strlower(arg2));
      pinsert->player_site = NULL;

      /* stupid get_char_world returns ptr to "samantha" when given "sam" */
      /* so I do a str_cmp to make sure it finds the right player --Gorog */

      sprintf(buf, "0.%s", arg2);
      if ((vic = get_char_world(ch, buf))) /* if vic is in game now */
         if ((!IS_NPC(vic)) && !str_cmp(arg2, vic->name))
            SET_BIT(vic->pcdata->flags, PCFLAG_WATCH);

      if (first_watch) /* ins new watch if app */
         for (pw = first_watch; pw; pw = pw->next)
            if (strcmp(pinsert->imm_name, pw->imm_name) < 0)
            {
               INSERT(pinsert, pw, first_watch, next, prev);
               save_watchlist();
               send_to_pager("Ok. That player will be watched.\n\r", ch);
               return;
            }

      LINK(pinsert, first_watch, last_watch, next, prev); /* link new watch */
      save_watchlist();
      send_to_pager("Ok. That player will be watched.\n\r", ch);
      return;
   }

/*
 * Watch a specific site
 */
   if (!str_cmp(arg, "site") && *arg2)
   {
      WATCH_DATA *pinsert;
      CHAR_DATA *vic;

      if (first_watch) /* check for dups */
         for (pw = first_watch; pw; pw = pw->next)
            if (!str_cmp(ch->name, pw->imm_name) && pw->player_site && !str_cmp(arg2, pw->player_site))
            {
               send_to_pager("You are already watching that site.\n\r", ch);
               return;
            }

      CREATE(pinsert, WATCH_DATA, 1); /* create new watch */
      pinsert->imm_level = get_trust(ch);
      pinsert->imm_name = str_dup(strlower(ch->name));
      pinsert->player_site = str_dup(strlower(arg2));
      pinsert->target_name = NULL;

      for (vic = first_char; vic; vic = vic->next)
         if (!IS_NPC(vic) && vic->desc && *pinsert->player_site
            && !str_prefix(pinsert->player_site, vic->desc->host) && get_trust(vic) < pinsert->imm_level)
            SET_BIT(vic->pcdata->flags, PCFLAG_WATCH);

      if (first_watch) /* ins new watch if app */
         for (pw = first_watch; pw; pw = pw->next)
            if (strcmp(pinsert->imm_name, pw->imm_name) < 0)
            {
               INSERT(pinsert, pw, first_watch, next, prev);
               save_watchlist();
               send_to_pager("Ok. That site will be watched.\n\r", ch);
               return;
            }

      LINK(pinsert, first_watch, last_watch, next, prev);
      save_watchlist();
      send_to_pager("Ok. That site will be watched.\n\r", ch);
      return;
   }

/*
 * Watch a specific command - FB
 */
   if (!str_cmp(arg, "command") && *arg2)
   {
      WATCH_DATA *pinsert;
      CMDTYPE *cmd;
      bool found = FALSE;

      for (pw = first_watch; pw; pw = pw->next)
      {
         if (!str_cmp(ch->name, pw->imm_name) && pw->target_name && !str_cmp(arg2, pw->target_name))
         {
            send_to_pager("You are already watching that command.\n\r", ch);
            return;
         }
      }

      for (cmd = command_hash[LOWER(arg2[0]) % 126]; cmd; cmd = cmd->next)
      {
         if (!strcmp(arg2, cmd->name))
         {
            found = TRUE;
            break;
         }
      }

      if (!found)
      {
         send_to_pager("No such command exists.\n\r", ch);
         return;
      }
      else
      {
         SET_BIT(cmd->flags, CMD_WATCH);
      }

      CREATE(pinsert, WATCH_DATA, 1);
      pinsert->imm_level = get_trust(ch);
      pinsert->imm_name = str_dup(strlower(ch->name));
      pinsert->player_site = NULL;
      pinsert->target_name = str_dup(arg2);

      for (pw = first_watch; pw; pw = pw->next)
      {
         if (strcmp(pinsert->imm_name, pw->imm_name) < 0)
         {
            INSERT(pinsert, pw, first_watch, next, prev);
            save_watchlist();
            send_to_pager("Ok, That command will be watched.\n\r", ch);
            return;
         }
      }

      LINK(pinsert, first_watch, last_watch, next, prev);
      save_watchlist();
      send_to_pager("Ok. That site will be watched.\n\r", ch);
      return;
   }

   send_to_pager("Sorry. I can't do anything with that. " "Please read the help file.\n\r", ch);
   return;
}

void do_wizhelp(CHAR_DATA * ch, char *argument)
{
   CMDTYPE *cmd;
   char wizcols[MAX_LEVEL - (LEVEL_PC - 1)] = "YOwPCBG";
   int col, hash;
   int curr_lvl;

   col = 0;
   set_pager_color(AT_PLAIN, ch);
   for (curr_lvl = LEVEL_PC; curr_lvl <= get_trust(ch); curr_lvl++)
   {
      pager_printf_color(ch, "\n\r&c&w[&G&WLEVEL %-2d&c&w] \n\r", curr_lvl);
      col = 0;
      for (hash = 0; hash < 126; hash++)
         for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
            if ((cmd->level == curr_lvl) && cmd->level <= get_trust(ch))
            {
               pager_printf_color(ch, "&%c%-17s", wizcols[URANGE(0, cmd->level - LEVEL_PC, MAX_LEVEL - LEVEL_PC)], cmd->name);
               if (++col % 6 == 0)
                  send_to_pager_color("\n\r", ch);
            }
   }
   send_to_pager("\n\r", ch);
   return;
}

void do_restrict(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char buf[MSL];
   sh_int level, hash;
   CMDTYPE *cmd;
   bool found;

   found = FALSE;
   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Restrict which command?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg2);
   if (arg2[0] == '\0')
      level = get_trust(ch);
   else
      level = atoi(arg2);

   level = UMAX(UMIN(get_trust(ch), level), 0);

   hash = arg[0] % 126;
   for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
   {
      if (!str_prefix(arg, cmd->name) && cmd->level <= get_trust(ch))
      {
         found = TRUE;
         break;
      }
   }

   if (found)
   {
      if (!str_prefix(arg2, "show"))
      {
         sprintf(buf, "%s show", cmd->name);
         do_cedit(ch, buf);
/*    		ch_printf( ch, "%s is at level %d.\n\r", cmd->name, cmd->level );*/
         return;
      }
      cmd->level = level;
      ch_printf(ch, "You restrict %s to level %d\n\r", cmd->name, level);
      sprintf(buf, "%s restricting %s to level %d", ch->name, cmd->name, level);
      log_string(buf);
   }
   else
      send_to_char("You may not restrict that command.\n\r", ch);

   return;
}

/* 
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc(CHAR_DATA * ch, char *name)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ret_char = NULL;
   static unsigned int number_of_hits;

   number_of_hits = 0;
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->character && (!str_prefix(name, d->character->name)) && IS_WAITING_FOR_AUTH(d->character))
      {
         if (++number_of_hits > 1)
         {
            ch_printf(ch, "%s does not uniquely identify a char.\n\r", name);
            return NULL;
         }
         ret_char = d->character; /* return current char on exit */
      }
   }
   if (number_of_hits == 1)
      return ret_char;
   else
   {
      send_to_char("No one like that waiting for authorization.\n\r", ch);
      return NULL;
   }
}

// used below to load an object with one line
void newobj_to_container(CHAR_DATA * ch, OBJ_DATA * container, int vnum)
{
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *iobj;

   iobj = get_obj_index(vnum);
   if (iobj != NULL)
   {
      obj = create_object(iobj, 0);
      obj_to_obj(obj, container);
   }
}

//Been modified to create a few basic newbie items and put them in a sack
void do_newbieset(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *cobj;
   OBJ_INDEX_DATA *iobj;

   if ((ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council")) || get_trust(ch) >= LEVEL_IMM)
   {
      if (argument[0] == '\0')
      {
         iobj = get_obj_index(123);
         if (iobj != NULL)
         {
            cobj = create_object(iobj, 0);
            obj_to_char(cobj, ch);
            newobj_to_container(ch, cobj, 5400); //book
            newobj_to_container(ch, cobj, 5468); //torch
            newobj_to_container(ch, cobj, 5473); //long sword
            newobj_to_container(ch, cobj, 5477); //dagger
            newobj_to_container(ch, cobj, 5483); //staff
            newobj_to_container(ch, cobj, 5492); //knuckle
            newobj_to_container(ch, cobj, 5485); //club
            newobj_to_container(ch, cobj, 5501); //water
            newobj_to_container(ch, cobj, 5504); //mana
            newobj_to_container(ch, cobj, 5507); //hp
            newobj_to_container(ch, cobj, 5507); //hp
            newobj_to_container(ch, cobj, 5509); //move
            newobj_to_container(ch, cobj, 5509); //move
            newobj_to_container(ch, cobj, 5494); //lamb
            newobj_to_container(ch, cobj, 5494); //lamb
            newobj_to_container(ch, cobj, 5494); //lamb
            newobj_to_container(ch, cobj, 5494); //lamb
            newobj_to_container(ch, cobj, 5494); //lamb
            newobj_to_container(ch, cobj, 5514); //potion of gods
            send_to_char("A new sack with the equipment has been created.\n\r", ch);
            return;
         }
         else
         {
            bug("A newbie set was not created!");
            send_to_char("An error occured, the imms have been contacted.\n\r", ch);
         }
      }
      else if (!str_cmp(argument, "book"))
      {
         iobj = get_obj_index(5400);
         if (iobj != NULL)
         {
            cobj = create_object(iobj, 0);
            obj_to_char(cobj, ch);
         }
         send_to_char("A newbie book is now in your inventory.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Syntax: newbieset [book].\n\r", ch);
         return;
      }
   }
}
void do_goauth(CHAR_DATA * ch, char *argument)
{
   if ((ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council")) || get_trust(ch) >= LEVEL_IMM)
   {
      if (ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council"))
      {
         if (xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
            || in_hellmaze(ch)
            || IS_AFFECTED(ch, AFF_CURSE)
            || xIS_SET(ch->act, PLR_GAMBLER) || ch->fighting)
         {
            send_to_char("Cannot use this command to escape norecall rules, sorry.\n\r", ch);
            return;
         }
      }
      act(AT_ACTION, "$n disappears in a swirl of smoke.", ch, NULL, NULL, TO_ROOM);
      ch->coord->x = ch->coord->y = ch->map = -1;
      REMOVE_ONMAP_FLAG(ch);

      if (ch->mount)
      {
         ch->mount->coord->x = ch->mount->coord->y = ch->mount->map = -1;
         REMOVE_ONMAP_FLAG(ch->mount);
         char_from_room(ch->mount);
         char_to_room(ch->mount, get_room_index(ROOM_AUTH_START));
         update_objects(ch->mount, ch->mount->map, ch->mount->coord->x, ch->mount->coord->y);
         do_look(ch->mount, "auto");
      }
      if (ch->pcdata->pet)
      {
         ch->pcdata->pet->coord->x = ch->pcdata->pet->coord->y = ch->pcdata->pet->map = -1;
         REMOVE_ONMAP_FLAG(ch->pcdata->pet);
         char_from_room(ch->pcdata->pet);
         char_to_room(ch->pcdata->pet, get_room_index(ROOM_AUTH_START));
         do_look(ch->pcdata->pet, "auto");
      }
      if (ch->pcdata->mount && !ch->mount)
      {
         ch->pcdata->mount->coord->x = ch->pcdata->mount->coord->y = ch->pcdata->mount->map = -1;
         REMOVE_ONMAP_FLAG(ch->pcdata->mount);
         char_from_room(ch->pcdata->mount);
         char_to_room(ch->pcdata->mount, get_room_index(ROOM_AUTH_START));
         do_look(ch->pcdata->mount, "auto");
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
      char_from_room(ch);
      char_to_room(ch, get_room_index(ROOM_AUTH_START));
      update_objects(ch, ch->map, ch->coord->x, ch->coord->y);
      act(AT_ACTION, "$n appears in the room.", ch, NULL, NULL, TO_ROOM);
      do_look(ch, "auto");
   }
   else
   {
      send_to_char("Huh?", ch);
      return;
   }
}

//show all members in a lastname/house
void do_showhouse(CHAR_DATA *ch, char *argument)
{
   FILE *fp;
   char strsave[200];
   char members[50][13];
   char name[13];
   int x = 0;
   int y;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  showhouse <lastname>\n\r", ch);
      return;
   }
   if (!IS_IMMORTAL(ch) && str_cmp(argument, ch->last_name))
   {
      send_to_char("You can only view the members of your house.\n\r", ch);
      return;
   }
   sprintf(strsave, "%s%c/%s", LNAME_DIR, tolower(argument[0]), capitalize(argument));
   if ((fp = fopen(strsave, "r")) == NULL)
   {
      send_to_char("That house does not exist, if it should, notify an immortal.\n\r", ch);
      return;
   }
   else
   {
      ch_printf(ch, "----------House of %s ----------\n\r", argument);
      for (;;)
      {
         sprintf(name, "%s", fread_lastname_line(fp));
         if (!str_cmp(name, "S"))
            break;
         else
         {
            sprintf(members[x++], "%s", name);
         }
      }
      y = x;
      for (x = 0;;x++)
      {
         if (y != x)
         {
            ch_printf(ch, "%s\n\r", members[x]);
         }
         else
         {
            return;
         }
      }
   }
}
   

//Now Newbie Council can auth, be careful who is on it.
void do_authorize(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   char *strtime;
   CHAR_DATA *victim;
   AUTHORIZE_DATA *newauth;
   AUTHORIZE_DATA *pastauth;
   DESCRIPTOR_DATA *d;

   set_char_color(AT_LOG, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if ((ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council")) || get_trust(ch) >= LEVEL_IMM)
   {
      if (arg1[0] == '\0')
      {
         send_to_char("&c&wUsage:  authorize <player> <yes|name|lastname|disconnect>\n\r", ch);
         send_to_char("Pending authorizations:\n\r", ch);
         send_to_char("&R----------------------------------------------------------------------------\n\r", ch);
         for (d = first_descriptor; d; d = d->next)
         {
            if ((victim = d->character) != NULL && IS_WAITING_FOR_AUTH(victim))
               ch_printf(ch, "&G&W %-15s %-15s &c&w a new %s (%s)...\n\r",
                  victim->name, victim->last_name, race_table[victim->race]->race_name, "Peaceful");
         }
         return;
      }
      if (!str_cmp(arg1, "list"))
      {
         if (first_authorized)
         {
            send_to_char("&c&wName          LastName      Authed by     Date                      IP\n\r", ch);
            send_to_char("&R--------------------------------------------------------------------------------------\n\r", ch);
            for (pastauth = first_authorized; pastauth; pastauth = pastauth->next)
            {
               pager_printf(ch, "&G&W%-12s  %-12s  &C%-12s  &c%-24s  %s\n\r", pastauth->name, pastauth->lastname, pastauth->authedby, pastauth->authdate,
                  pastauth->host ? pastauth->host : "Not Available");
            }
         }
         else
         {
            send_to_char("This list is currently empty.\n\r", ch);
         }
         return;
      }

      victim = get_waiting_desc(ch, arg1);
      if (victim == NULL)
         return;

      set_char_color(AT_IMMORT, victim);
      if (arg2[0] == '\0' || !str_cmp(arg2, "accept") || !str_cmp(arg2, "yes"))
      {
         victim->pcdata->auth_state = 3;
         victim->pcdata->authwait = -1;
         CREATE(newauth, AUTHORIZE_DATA, 1);
         newauth->name = QUICKLINK(victim->name);
         newauth->lastname = QUICKLINK(victim->last_name);
         newauth->authedby = QUICKLINK(ch->name);
         strtime = ctime(&current_time);
         strtime[strlen(strtime) - 1] = '\0';
         newauth->authdate = STRALLOC(strtime);
         sprintf(buf, "%s", victim->desc->host);
         newauth->host = STRALLOC(buf);
         LINK(newauth, first_authorized, last_authorized, next, prev);
         fwrite_authlist();
         if (victim->pcdata->authed_by)
            STRFREE(victim->pcdata->authed_by);
         victim->pcdata->authed_by = QUICKLINK(ch->name);
         sprintf(buf, "%s: authorized", victim->name);
         to_channel(buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL); /* Tracker1 */

         ch_printf(ch, "You have authorized %s.\n\r", PERS_MAP(victim, ch));

         /* Below sends a message to player when name is accepted - Brittany */
         ch_printf_color(victim, /* B */
            "\n\r&GThe MUD Administrators have accepted the name %s.\n\r"
            "You may enter Rafermand by typing ---pull sword---.\n\r", victim->name);
         return;
      }
      else if (!str_cmp(arg2, "disconnect"))
      {
         send_to_char_color("&RYou have been denied access.\n\r", victim);
         sprintf(buf, "%s: denied authorization", victim->name);
         to_channel(buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL); /* Tracker 1 */
         ch_printf(ch, "You have denied %s.\n\r", victim->name);
         do_quit(victim, "");
      }

      else if (!str_cmp(arg2, "name") || !str_cmp(arg2, "n") || !str_cmp(arg2, "no"))
      {
         victim->pcdata->auth_state = 2;
         victim->pcdata->authwait = 30000; //that should do :-)
         sprintf(buf, "%s: name denied", victim->name);
         to_channel(buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL);
         ch_printf_color(victim,
            "&R\n\rThe MUD Administrators or newbie councel found the name of %s to be\n\r"
            "unacceptable.  You may choose a new name by typing name and the new name you want\n\r"
            "to use.  Please remember, the name needs to be original - no DBZ characters, no character\n\r"
            "from a major book/tv show/movie and do not use names of major mobs in the game, or names\n\r"
            "that look like the names of an Immortal ex: Xerxes.\n\r", victim->name);
         ch_printf(ch, "You requested %s change names.\n\r", victim->name);
         return;
      }
      else if (!str_cmp(arg2, "lastname"))
      {
         victim->pcdata->auth_state = 4;
         victim->pcdata->authwait = 30000; //that should do :-)
         sprintf(buf, "%s: name denied", victim->name);
         to_channel(buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL);
         ch_printf(victim, "&R\n\rThat last name has been chosen to be unacceptable.  Please pick a new one with\n\r"
                           "the 'lastname' command.\n\r");
         ch_printf(ch, "You requested %s change lastnames.\n\r", victim->last_name);
         return;
      }        
      else
      {
         send_to_char("Invalid argument.\n\r", ch);
         return;
      }
   }
   else
   {
      send_to_char("Huh?", ch);
      return;
   }
}

void do_bamfin(CHAR_DATA * ch, char *argument)
{
   if (!IS_NPC(ch))
   {
      smash_tilde(argument);
      DISPOSE(ch->pcdata->bamfin);
      ch->pcdata->bamfin = str_dup(argument);
      send_to_char_color("&YBamfin set.\n\r", ch);
   }
   return;
}

void do_bamfout(CHAR_DATA * ch, char *argument)
{
   if (!IS_NPC(ch))
   {
      smash_tilde(argument);
      DISPOSE(ch->pcdata->bamfout);
      ch->pcdata->bamfout = str_dup(argument);
      send_to_char_color("&YBamfout set.\n\r", ch);
   }
   return;
}

//Zaps a character to 1 hp, a punishment warning :-)  Thanks NEV!
void do_wizzap(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: wizzap <char>.\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, argument)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (victim == ch)
   {
      send_to_char("If you are trying to revive brain cells, this is not the way.\n\r", ch);
      return;
   }
   if (victim->fighting)
   {
      send_to_char("Might as well slay them or wait till they are done fighting.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   if (victim->position == POS_SLEEPING || victim->position == POS_RESTING)
      victim->position = POS_STANDING;
   send_to_char("\n\r&Y   \\\\\n\r", victim);
   send_to_char("&Y    \\\\\n\r", victim);
   send_to_char("&Y     \\\\\n\r", victim);
   send_to_char("&Y      \\\\\n\r", victim);
   send_to_char("&Y       \\\\\n\r", victim);
   send_to_char("&Y        \\\\\n\r", victim);
   send_to_char("&Y         \\\\   //\n\r", victim);
   send_to_char("&Y          \\\\ // \\\\\n\r", victim);
   send_to_char("&Y           //    \\\\\n\r", victim);
   send_to_char("&Y                  \\\\\n\r", victim);
   send_to_char("&Y                   \\\\\n\r", victim);
   send_to_char("&Y                    \\\\\n\r", victim);
   send_to_char("&Y                     \\\\\n\r", victim);
   send_to_char("&Y                      \\\\\n\r", victim);
   send_to_char("&Y                       \\\\\n\r\n\r", victim);
   victim->hit = 1;
   victim->mana = 1;
   victim->move = 1;
   act(AT_WHITE, "$n has zapped you with a big bolt of lightning!!!", ch, NULL, victim, TO_VICT);
   act(AT_WHITE, "You zap $N with a big bolt of lightning!!!", ch, NULL, victim, TO_CHAR);
   act(AT_WHITE, "$n zaps $N with a huge bolt of lightning, be glad it was not you.", ch, NULL, victim, TO_ROOM);
   return;
}

/* Simular to checkvnum, but will list the freevnums -- Xerves */
void do_free_vnums(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   AREA_DATA *pArea;
   char arg1[MSL];
   bool area_conflict;
   int low_range, high_range;
   int lohi[600]; /* Up to 300 and a half areas, increase if you have more -- Xerves */
   int xfin = 0;
   int w = 0;
   int x = 0;
   int y = 0;
   int z = 0;

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0')
   {
      send_to_char("Please specify the low end of the range to be searched.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Please specify the high end of the range to be searched.\n\r", ch);
      return;
   }

   low_range = atoi(arg1);
   high_range = atoi(argument);

   if (low_range < 1 || low_range > MAX_VNUM)
   {
      send_to_char("Invalid argument for bottom of range.\n\r", ch);
      return;
   }

   if (high_range < 1 || high_range > MAX_VNUM)
   {
      send_to_char("Invalid argument for top of range.\n\r", ch);
      return;
   }

   if (high_range < low_range)
   {
      send_to_char("Bottom of range must be below top of range.\n\r", ch);
      return;
   }
   /* Forces it to check in sets of 10 -- Xerves */
   low_range = low_range / 10;
   low_range = low_range * 10;
   set_char_color(AT_PLAIN, ch);

   for (pArea = first_asort; pArea; pArea = pArea->next_sort)
   {
      area_conflict = FALSE;
      if (IS_SET(pArea->status, AREA_DELETED))
         continue;
      else
      {
         if (low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range)
            area_conflict = TRUE;

         if ((low_range >= pArea->low_r_vnum) && (low_range <= pArea->hi_r_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_r_vnum) && (high_range >= pArea->low_r_vnum))
            area_conflict = TRUE;
      }
      if (area_conflict)
      {
         lohi[x] = pArea->low_r_vnum;
         x++;
         lohi[x] = pArea->hi_r_vnum;
         x++;
      }
   }
   for (pArea = first_bsort; pArea; pArea = pArea->next_sort)
   {
      area_conflict = FALSE;
      if (IS_SET(pArea->status, AREA_DELETED))
         continue;
      else
      {
         if (low_range < pArea->low_r_vnum && pArea->low_r_vnum < high_range)
            area_conflict = TRUE;

         if (low_range < pArea->hi_r_vnum && pArea->hi_r_vnum < high_range)
            area_conflict = TRUE;

         if ((low_range >= pArea->low_r_vnum) && (low_range <= pArea->hi_r_vnum))
            area_conflict = TRUE;

         if ((high_range <= pArea->hi_r_vnum) && (high_range >= pArea->low_r_vnum))
            area_conflict = TRUE;
      }

      if (area_conflict)
      {
         lohi[x] = pArea->low_r_vnum;
         x++;
         lohi[x] = pArea->hi_r_vnum;
         x++;
      }
   }
   xfin = x;
   for (y = low_range; y < high_range; y = y + 50)
   {
      area_conflict = FALSE;
      z = y + 49; /* y is min, z is max */
      for (x = 0; x < xfin; x = x + 2)
      {
         w = x + 1;

         if (y < lohi[x] && lohi[x] < z)
         {
            area_conflict = TRUE;
            break;
         }
         if (y < lohi[w] && lohi[w] < z)
         {
            area_conflict = TRUE;
            break;
         }
         if (y >= lohi[x] && y <= lohi[w])
         {
            area_conflict = TRUE;
            break;
         }
         if (z <= lohi[w] && z >= lohi[x])
         {
            area_conflict = TRUE;
            break;
         }
      }
      if (area_conflict == FALSE)
      {
         sprintf(buf, "Open: %5d - %-5d\n\r", y, z);
         send_to_char(buf, ch);
      }
   }
   return;
}

   NPCRACE_DATA *next;
   NPCRACE_DATA *prev;
   int racenum;
   char *racename;
   int willload[MAX_QUEST_DIFF];
   char *description[MAX_QUEST_DIFF];
   int sex[MAX_QUEST_DIFF];
   int fulldescription[MAX_QUEST_DIFF];
   EXT_BV flags[MAX_QUEST_DIFF];
   EXT_BV nflags[MAX_QUEST_DIFF];
   
void do_npcrace(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int x;
   int num;
   NPCRACE_DATA *npcrace;
   NPCRACE_DATA *snpcrace;
   
   if (check_npc(ch))
      return;
   if (argument[0] == '\0')
   {
      send_to_char("npcrace view [stack/number]\n\r", ch);
      send_to_char("npcrace view willload/willnotload [diff]\n\r", ch);
      send_to_char("npcrace create <name>\n\r", ch);
      send_to_char("npcrace delete <number/race name>\n\r", ch);
      send_to_char("npcrace set <number/race name> <name/number/description/fulldescription/sex/willload/flags/nflags> <value>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   if (!str_cmp(arg1, "set"))
   {
      argument = one_argument(argument, arg1);
      if (arg1[0] == '\0')
      {
         send_to_char("You need to specify an npc number or name to set.\n\r", ch);
         return;
      }
      for (npcrace = first_npcrace; npcrace; npcrace = npcrace->next)
      {
         if (!str_cmp(npcrace->racename, arg1))
            break;
      }
      if (!npcrace)
      {
         if (atoi(arg1) < 0 || atoi(arg1) > max_npc_race)
         {
            send_to_char("That is not a valid npc.\n\r", ch);
            return;
         }
         if (!npcrace_table[atoi(arg1)])
         {
            send_to_char("That is not a valid npc.\n\r", ch);
            return;
         }
         npcrace = npcrace_table[atoi(arg1)];
      }    
      argument = one_argument(argument, arg1);
      if (!str_cmp(arg1, "flags"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) < 1 || atoi(arg1) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         xCLEAR_BITS(npcrace->flags[atoi(arg1)-1]);
         for (;;)
         {
            argument = one_argument(argument, arg2);  
            if (arg2[0] == '\0')
               break;
            else
            {
               if (get_qmobflag(arg2) != -1)
                  xSET_BIT(npcrace->flags[atoi(arg1)-1], get_qmobflag(arg2));
               else if (isdigit(arg2[0]))
                  xSET_BIT(npcrace->flags[atoi(arg1)-1], atoi(arg2)-1);
               else
                  ch_printf(ch, "%s is not a valid flag\n\r", arg2);
            }
         }
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }
      if (!str_cmp(arg1, "nflags"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) < 1 || atoi(arg1) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         xCLEAR_BITS(npcrace->nflags[atoi(arg1)-1]);
         for (;;)
         {
            argument = one_argument(argument, arg2);  
            if (arg2[0] == '\0')
               break;
            else
            {
               if (get_qmobflag(arg2) != -1)
                  xSET_BIT(npcrace->nflags[atoi(arg1)-1], get_qmobflag(arg2));
               else if (isdigit(arg2[0]))
                  xSET_BIT(npcrace->nflags[atoi(arg1)-1], atoi(arg2)-1);
               else
                  ch_printf(ch, "%s is not a valid flag\n\r", arg2);
            }
         }
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }
      if (!str_cmp(arg1, "description"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) < 1 || atoi(arg1) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         STRFREE(npcrace->description[atoi(arg1)-1]);
         npcrace->description[atoi(arg1)-1] = STRALLOC(argument);
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }
      if (!str_cmp(arg1, "fulldescription"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) < 1 || atoi(arg1) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         npcrace->fulldescription[atoi(arg1)-1] = atoi(argument);
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }
      if (!str_cmp(arg1, "sex"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) < 1 || atoi(arg1) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         if (atoi(argument) < 1 || atoi(argument) > 2)
         {
            send_to_char("Range is 1 (male) to 2 (female).\n\r", ch);
            return;
         }
         npcrace->sex[atoi(arg1)-1] = atoi(argument);
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }
      if (!str_cmp(arg1, "willload"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(arg1) < 1 || atoi(arg1) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         npcrace->willload[atoi(arg1)-1] = atoi(argument);
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }
      if (!str_cmp(arg1, "name"))
      {
         STRFREE(npcrace->racename);
         npcrace->racename = STRALLOC(argument);
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }
      if (!str_cmp(arg1, "number"))
      {
         if (atoi(argument) < 0 || atoi(argument) >= MAX_NPCRACE_TABLE)
         {
            ch_printf(ch, "Range is 0 to %d", MAX_NPCRACE_TABLE-1);
            return;
         }
         if (npcrace_table[atoi(argument)])
         {
            send_to_char("You can only change the number to one not taken.\n\r", ch);
            return;
         }
         x = 0;
         if (max_npc_race-1 == npcrace->racenum && atoi(argument) < npcrace->racenum)
         {
            for (snpcrace = first_npcrace; snpcrace; snpcrace = snpcrace->next)
            {
               if (snpcrace->racenum > x && npcrace != snpcrace)
                  x = snpcrace->racenum;
            }
            max_npc_race = x+1;
         }  
         npcrace_table[npcrace->racenum] = NULL;
         npcrace->racenum = atoi(argument);
         npcrace_table[npcrace->racenum] = npcrace;
         if (npcrace->racenum >= max_npc_race)
            max_npc_race = npcrace->racenum+1;
         send_to_char("Done.\n\r", ch);
         save_npcrace_file();
         return;
      }         
   }         
   if (!str_cmp(arg1, "delete"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("You need to specify an npc number or name to set.\n\r", ch);
         return;
      }
      for (npcrace = first_npcrace; npcrace; npcrace = npcrace->next)
      {
         if (!str_cmp(npcrace->racename, argument))
            break;
      }
      if (!npcrace)
      {
         if (atoi(argument) < 0 || atoi(argument) > max_npc_race)
         {
            send_to_char("That is not a valid npc.\n\r", ch);
            return;
         }
         if (!npcrace_table[atoi(argument)])
         {
            send_to_char("That is not a valid npc.\n\r", ch);
            return;
         }
         npcrace = npcrace_table[atoi(argument)];
      } 
      STRFREE(npcrace->racename);
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
      {
         STRFREE(npcrace->description[x]);
      }
      npcrace_table[npcrace->racenum] = NULL;
      x = 0;
      if (max_npc_race-1 == npcrace->racenum)
      {
         for (snpcrace = first_npcrace; snpcrace; snpcrace = snpcrace->next)
         {
            if (snpcrace->racenum > x && npcrace != snpcrace)
               x = snpcrace->racenum;
         }
         max_npc_race = x+1;
      } 
      UNLINK(npcrace, first_npcrace, last_npcrace, next, prev);
      DISPOSE(npcrace);
      save_npcrace_file();
      send_to_char("Done.\n\r", ch);
      return;
   }      
   if (!str_cmp(arg1, "create"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Need to specify a name for the npc race you want to create.\n\r", ch);
         return;
      }
      CREATE(npcrace, NPCRACE_DATA, 1);
      npcrace->racename = STRALLOC(argument);
      num = -1;
      for (snpcrace = first_npcrace; snpcrace; snpcrace = snpcrace->next)
      {
         if (snpcrace->racenum > num)
            num = snpcrace->racenum;
      }
      if (num == MAX_NPCRACE_TABLE-1)
      {
         send_to_char("The maximum amount of NPC races has been reached, you need to modify MAX_NPCRACE_TABLE in the code to add more.\n\r", ch);
         return;
      }
      npcrace->racenum = num+1;
      if (npcrace->racenum == max_npc_race)
         max_npc_race++;
      for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
      {
         npcrace->willload[x] = npcrace->sex[x] = npcrace->fulldescription[x] = 0;
         npcrace->description[x] = STRALLOC("");
         xCLEAR_BITS(npcrace->flags[x]);
         xCLEAR_BITS(npcrace->nflags[x]);
      }
      npcrace_table[npcrace->racenum] = npcrace;
      LINK(npcrace, first_npcrace, last_npcrace, next, prev);
      save_npcrace_file();
      send_to_char("Done.\n\r", ch);
      return;
   }      
   if (!str_cmp(arg1, "view"))
   {
      one_argument(argument, arg1);
      if (!str_cmp(arg1, "willload"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(argument) < 1 || atoi(argument) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         ch_printf(ch, "Num  Name\n\r", ch);
         x = 0;
         for (num = 0; num < max_npc_race; num++)
         {
            if (npcrace_table[num])
            {
               npcrace = npcrace_table[num];
               if (npcrace->willload[atoi(argument)-1] == 1)
               {
                  ch_printf(ch, "%-3d> %-15s", npcrace->racenum, npcrace->racename);   
                  if (++x % 4 == 0)
                     ch_printf(ch, "\n\r");
               }
            }
         }
         if (x % 4 != 0)
            ch_printf(ch, "\n\r");
         return;
      }
      if (!str_cmp(arg1, "willnotload"))
      {
         argument = one_argument(argument, arg1);
         if (atoi(argument) < 1 || atoi(argument) > MAX_QUEST_DIFF)
         {
            ch_printf(ch, "Range is 1 to %d", MAX_QUEST_DIFF);
            return;
         }
         ch_printf(ch, "Num  Name\n\r", ch);
         x = 0;
         for (num = 0; num < max_npc_race; num++)
         {
            if (npcrace_table[num])
            {
               npcrace = npcrace_table[num];
               if (npcrace->willload[atoi(argument)-1] != 1)
               {
                  ch_printf(ch, "%-3d> %-15s", npcrace->racenum, npcrace->racename);   
                  if (++x % 4 == 0)
                     ch_printf(ch, "\n\r");
               }
            }
         }
         if (x % 4 != 0)
            ch_printf(ch, "\n\r");
         return;
      }
         
      if (!str_cmp(argument, "stack"))
      {
         x = 0;
         for (num = 0; num < max_npc_race; num++)
         {
            if (npcrace_table[num])
            {
               npcrace = npcrace_table[num];
               ch_printf(ch, "%-3d> %-15s", npcrace->racenum, npcrace->racename);   
               if (++x % 4 == 0)
                  ch_printf(ch, "\n\r");
            }
         }   
         if (x % 4 != 0)
            ch_printf(ch, "\n\r");
         return;
      }
      else if (argument[0] != '\0')
      {
         if (argument[0] == '\0')
         {
            send_to_char("You need to specify an npc number or name to set.\n\r", ch);
            return;
         }
         for (npcrace = first_npcrace; npcrace; npcrace = npcrace->next)
         {
            if (!str_cmp(npcrace->racename, argument))
               break;
         }
         if (!npcrace)
         {
            if (atoi(argument) < 0 || atoi(argument) > max_npc_race)
            {
               send_to_char("That is not a valid npc.\n\r", ch);
               return;
            }
            if (!npcrace_table[atoi(argument)])
            {
               send_to_char("That is not a valid npc.\n\r", ch);
               return;
            }
            npcrace = npcrace_table[atoi(argument)];
         } 
         ch_printf(ch, "Num  Name\n\r", ch);
         ch_printf(ch, "------------------------------------------\n\r"); 
         ch_printf(ch, "%-3d> %-20s\n\r", npcrace->racenum, npcrace->racename);
         ch_printf(ch, "\n\rDiff> Willload  Sex  FullDesc  Description\n\r");
         ch_printf(ch, "------------------------------------------------------------------------------------\n\r");         
         for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         {
            ch_printf(ch, "%-2d> %d %d %d %-20s ", x+1, npcrace->willload[x], npcrace->sex[x], npcrace->fulldescription[x], 
               npcrace->description[x]);
            if ((x+1)%3 == 0)
               ch_printf(ch, "\n\r");
         }
         if (x % 3 != 0)
            ch_printf(ch, "\n\r");
         ch_printf(ch, "\n\r       Diff  Flags\n\r");
         ch_printf(ch, "------------------------------------------------------------------------------------\n\r");    
         for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
         {
            ch_printf(ch, "Flags   %-2d    %s\n\r", x+1, ext_flag_string(&npcrace->flags[x], qmob_flags));
            ch_printf(ch, "NFlags  %-2d    %s\n\r", x+1, ext_flag_string(&npcrace->nflags[x], qmob_flags));
         }
            
         return;
      }
      else
      {
         ch_printf(ch, "Num  Name                  Will load (for each diff)\n\r", ch);
         for (num = 0; num < max_npc_race; num++)
         {
            if (npcrace_table[num])
            {
               npcrace = npcrace_table[num];
               ch_printf(ch, "%-3d> %-20s", npcrace->racenum, npcrace->racename);   
               for (x = 0; x <= MAX_QUEST_DIFF-1; x++)
                  ch_printf(ch, "  %d", npcrace->willload[x]);
               ch_printf(ch, "\n\r");
            }
         }
      }
      return;
   }
   do_npcrace(ch, "");
   return;
}
void do_rank(CHAR_DATA * ch, char *argument)
{
   char add_len[MIL];
   int sp1, sx;
   char finrank[MIL];

   set_char_color(AT_IMMORT, ch);
   sp1 = strlen_color(argument);
   sprintf(add_len, " ");

   for (sx = 25; sx > sp1; sx--) /* 26 - 1 for add_len init. */
      strcat(add_len, " ");
   strcpy(finrank, argument); /* Adding spaces to finish out rank --Xerves */
   strcat(finrank, add_len); /* Adding the length to the end of the argument -- Xerves */

   if (IS_NPC(ch))
      return;
   if (!argument || argument[0] == '\0')
   {
      send_to_char("Usage:  rank <string>.\n\r", ch);
      send_to_char("   or:  rank none.\n\r", ch);
      return;
   }
   smash_tilde(argument);
   DISPOSE(ch->pcdata->rank);
   if (!str_cmp(argument, "none"))
      ch->pcdata->rank = str_dup("");
   else
      ch->pcdata->rank = str_dup(finrank);
   send_to_char("Ok.\n\r", ch);
   return;
}

/*8-19-99 Moved to caste.c */
/*11-28-98 Will change Caste ranking - Xerves */
/*
void do_caste(  CHAR_DATA *ch, char *argument  )
{

  char arg1[MIL];
  char arg2[MIL];
  int amount;
  CHAR_DATA *victim;

  if (IS_NPC(ch))
  {
     send_to_char( "NOT ON NPCs\n\r", ch);
     return;
  }

  if ( argument[0] == '\0' )
  {
     send_to_char( "Syntax: caste <name> <number> 1-15\n\r", ch);
     return;
  }

argument = one_argument(argument, arg1);
argument = one_argument(argument, arg2);

  if ( arg1 == '\0' || arg2 == '\0' )
  {
     send_to_char( "Syntax: caste <name> <number> 1-40 (help caste for info)\n\r", ch);
     return;
  }

  if ( ( victim = get_char_world( ch, arg1 ) ) == NULL )
  {
	send_to_char( "They aren't here.\n\r", ch );
	return;
  }

  if ( IS_NPC( victim ))
  {
      send_to_char( "NOT ON NPCs.\n\r", ch);
      return;
  }

  if ( get_trust( ch ) <= get_trust( victim ) && ch != victim )
  {
      send_to_char( "Don't do that AGAIN!.\n\r", ch );
	return;
  }

  if ( !is_number( arg2 ) )
  {
     send_to_char( "Syntax: caste <name> <number> 1-40 (help caste for info)\n\r", ch);
     return;
  }

    amount = atoi( arg2 );

  if ( amount < 1 || amount > 40 )
  {
     send_to_char( "Syntax: caste <name> <number> 1-40 (help caste for info)\n\r", ch);
     return;
  }

  victim->pcdata->caste = amount;
  return;
}  */

/* Critt's Link Dead Punt -- Xerves 8/3/99 */
void ld_punt(CHAR_DATA * ch)
{
   char buf[MSL];

   /*
      CHAR_DATA *vch;
      CHAR_DATA *vch_next; */
   int x, y;
   int level;

   set_char_color(AT_IMMORT, ch);

   if (!IS_NPC(ch) && !ch->desc)
   {
      if (ch->position == POS_MOUNTED)
         do_dismount(ch, "");
      if (ch->riding)
         do_dismount(ch, "");
      if (ch->position == POS_RIDING)
         ch->position = POS_STANDING;
      set_char_color(AT_WHITE, ch);
      /* if (ch)
         send_to_char( "You have been punted because you are link-dead...\n\r", ch ); */
      set_char_color(AT_GREY, ch);
      quitting_char = ch;
      save_char_obj(ch);
      if (sysdata.save_pets && ch->pcdata->pet)
      {
         extract_char(ch->pcdata->pet, TRUE);
      }
      if (ch->pcdata->clan)
         save_clan(ch->pcdata->clan);
      saving_char = NULL;
      level = get_trust(ch);
      /*  sprintf(buf, "%s has been punted.\n\r",capitalize(vch->name) );
         if (ch)
         send_to_char( buf, ch ); */
      act(AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_CANSEE);
      if (!IS_IMMORTAL(ch))
      {
         sprintf(buf, "You feel that there is one less presence in %s...", sysdata.mud_name);
         talk_info(AT_BLUE, buf);
      }
      sprintf(buf, "%s has quit (Room %d).", ch->name, (ch->in_room ? ch->in_room->vnum : -1));
      log_string_plus(buf, LOG_COMM, level);
      extract_char(ch, TRUE); /* ch void after extraction -- Xerves */
      for (x = 0; x < MAX_WEAR; x++)
         for (y = 0; y < MAX_LAYERS; y++)
            save_equipment[x][y] = NULL;
   }
/*
    send_to_char( "All done.\n\r", ch ); */
   return;
}

void do_pkillcheck(CHAR_DATA *ch, char *argument)
{
   AREA_DATA *tarea;
   ROOM_INDEX_DATA *room;
   int vnum;
   
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (IS_SET(tarea->flags, AFLAG_NOKILL))
         ch_printf(ch, "&w&RArea: %s  Flag: NOPKILL\n\r", tarea->name);
      if (IS_SET(tarea->flags, AFLAG_ANITEM))
         ch_printf(ch, "&w&RArea: %s  Flag: ANITEM\n\r", tarea->name);
      if (IS_SET(tarea->flags, AFLAG_NOLOOT))
         ch_printf(ch, "&w&RArea: %s  Flag: NOLOOT\n\r", tarea->name);
         
      for (vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++)
      {
         if ((room = get_room_index(vnum)) != NULL)
         {
            if (xIS_SET(room->room_flags, ROOM_TSAFE))
               ch_printf(ch, "&w&CArea: %s  Room: %d  Flag: NOPKILL\n\r", tarea->name, vnum);
            if (xIS_SET(room->room_flags, ROOM_SAFE))
               ch_printf(ch, "&w&CArea: %s  Room: %d  Flag: >>SAFE<<\n\r", tarea->name, vnum);
            if (xIS_SET(room->room_flags, ROOM_ANITEM))
               ch_printf(ch, "&w&CArea: %s  Room: %d  Flag: ANITEM\n\r", tarea->name, vnum);
            if (xIS_SET(room->room_flags, ROOM_NOLOOT))
               ch_printf(ch, "&w&CArea: %s  Room: %d  Flag: NOLOOT\n\r", tarea->name, vnum);
         }
      }
   }
   return;
}

/* For now, just a simple one item search on mobs, typically to find how
   many mobs there are in a level range or something of that sort
   -- Xerves 12/99 */
void do_minfo(CHAR_DATA * ch, char *argument)
{
   return;
 /*   char buf[MSL];
   MOB_INDEX_DATA *mob;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   int pvnum;
   int mcount;
   int hihealth;
   int lohealth;
   int hidamage;
   int lodamage;

   mcount = 0;

   minfo <type> <first arg> <second arg>
      arg1   arg2        arg3    

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);

   if ((arg1[0] == '\0') || (arg2[0] == '\0') || (arg3[0] == '\0'))
   {
      send_to_char("Syntax: minfo <type> <first arg> <second arg>\n\rType - lv(args-lo level, hi level)\n\r", ch);
      return;
   }
   if (str_cmp(arg1, "lv"))
   {
      send_to_char("Only type right now is level of mobs\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "lv"))
   {
      if (atoi(arg2) > atoi(arg3))
      {
         send_to_char("The low has to be lower than the high\n\r", ch);
         return;
      }
      if (atoi(arg3) < atoi(arg2))
      {
         send_to_char("The high has to be higher than the low\n\r", ch);
         return;
      }
   }

   send_to_char("Vnum   Lv  Gold     Min HP  Max HP  Min Dam   Max Dam   Name\n\r", ch);
   for (pvnum = 1; pvnum < MAX_VNUM; pvnum++)
   {
      if ((mob = get_mob_index(pvnum)) == NULL)
         continue;
      else
      {
         if ((mob->level >= atoi(arg2)) && (mob->level <= atoi(arg3)))
         {
            hihealth = (mob->hitnodice * mob->hitsizedice + mob->hitplus);
            lohealth = (1 * mob->hitsizedice + mob->hitplus);
            hidamage = (mob->damnodice * mob->damsizedice + mob->damplus);
            lodamage = (1 * mob->damsizedice + mob->damplus);
            if (xIS_SET(mob->act, ACT_PACIFIST))
            {
               sprintf(buf, "&C%-5d  %-2d  %-7d  %-5d   %-5d   %-4d      %-4d      %s\n\r", mob->vnum, mob->level, mob->gold, lohealth, hihealth,
                  lodamage, hidamage, mob->short_descr);
               send_to_char(buf, ch);
            }
            else
            {
               sprintf(buf, "&G%-5d  %-2d  %-7d  %-5d   %-5d   %-4d      %-4d      %s\n\r", mob->vnum, mob->level, mob->gold, lohealth, hihealth,
                  lodamage, hidamage, mob->short_descr);
               send_to_char(buf, ch);
            }
            mcount++;
            continue;
         }
      }
   }*/
}

/* 3-12-99 Will change FLevel number - Xerves */
void do_flevel(CHAR_DATA * ch, char *argument)
{

   char arg1[MIL];
   char arg2[MIL];
   int amount;
   CHAR_DATA *victim;

   if (IS_NPC(ch))
   {
      send_to_char("NOT ON NPCs\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: flevel <name> <number> 2-7 or 0\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1 == '\0' || arg2 == '\0')
   {
      send_to_char("Syntax: flevel <name> <number> 2-7 or 0 (help flevel for info)\n\r", ch);
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
      send_to_char("Syntax: flevel <name> <number> 2-7 or 0 (help flevel for info)\n\r", ch);
      return;
   }

   amount = atoi(arg2);

   if (amount == 0)
   {
      victim->pcdata->flevel = 0;
      return;
   }

   if (amount < LEVEL_IMMORTAL || amount > MAX_LEVEL)
   {
      send_to_char("Syntax: flevel <name> <number> 2-7 or 0 (help flevel for info)\n\r", ch);
      return;
   }

   victim->pcdata->flevel = amount;
   return;
}


void do_retire(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Retire whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   if (victim->level < LEVEL_IMM) /* Tracker1 */
   {
      ch_printf(ch, "The minimum level for retirement is %d.\n\r", LEVEL_IMM);
      return;
   }
   if (IS_RETIRED(victim))
   {
      REMOVE_BIT(victim->pcdata->flags, PCFLAG_RETIRED);
      ch_printf(ch, "%s returns from retirement.\n\r", victim->name);
      ch_printf(victim, "%s brings you back from retirement.\n\r", ch->name);
   }
   else
   {
      SET_BIT(victim->pcdata->flags, PCFLAG_RETIRED);
      ch_printf(ch, "%s is now a retired immortal.\n\r", victim->name);
      ch_printf(victim, "Courtesy of %s, you are now a retired immortal.\n\r", ch->name);
   }
   return;
}

void do_delay(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg[MIL];
   int delay;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (!*arg)
   {
      send_to_char("Syntax:  delay <victim> <# of rounds>\n\r", ch);
      return;
   }
   if (!(victim = get_char_world(ch, arg)))
   {
      send_to_char("No such character online.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Mobiles are unaffected by lag.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You haven't the power to succeed against them.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!*arg)
   {
      send_to_char("For how long do you wish to delay them?\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "none"))
   {
      send_to_char("All character delay removed.\n\r", ch);
      victim->wait = 0;
      return;
   }
   delay = atoi(arg);
   if (delay < 1)
   {
      send_to_char("Pointless.  Try a positive number.\n\r", ch);
      return;
   }
   if (delay > 999)
   {
      send_to_char("You cruel bastard.  Just kill them.\n\r", ch);
      return;
   }
   WAIT_STATE(victim, delay * PULSE_VIOLENCE);
   ch_printf(ch, "You've delayed %s for %d rounds.\n\r", victim->name, delay);
   return;
}

void do_deny(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Deny whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   xSET_BIT(victim->act, PLR_DENY);
   set_char_color(AT_IMMORT, victim);
   send_to_char("You are denied access!\n\r", victim);
   ch_printf(ch, "You have denied access to %s.\n\r", victim->name);
   if (victim->fighting)
      stop_fighting(victim, TRUE); /* Blodkai, 97 */
   char_quit(ch, FALSE); /* Rantic's info channel */
   return;
}

void do_disconnect(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Disconnect whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (victim->desc == NULL)
   {
      act(AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
      return;
   }
   if (get_trust(ch) <= get_trust(victim))
   {
      send_to_char("They might not like that...\n\r", ch);
      return;
   }

   for (d = first_descriptor; d; d = d->next)
   {
      if (d == victim->desc)
      {
         close_socket(d, FALSE);
         send_to_char("Ok.\n\r", ch);
         return;
      }
   }
   bug("Do_disconnect: *** desc not found ***.", 0);
   send_to_char("Descriptor not found!\n\r", ch);
   return;
}


/*
 * Force a level one player to quit.             Gorog
 */
void do_fquit(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg1[MIL];

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   if (arg1[0] == '\0')
   {
      send_to_char("Force whom to quit?\n\r", ch);
      return;
   }
   if (!(victim = get_char_world(ch, arg1)))
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!NOT_AUTHED(victim))
   {
      send_to_char("Works only on unauthed players.\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, victim);
   send_to_char("The MUD administrators force you to quit...\n\r", victim);
   if (victim->fighting)
      stop_fighting(victim, TRUE);
   char_quit(victim, FALSE); /* Rantic's info channel */
   ch_printf(ch, "You have forced %s to quit.\n\r", victim->name);
   return;
}

void do_forceclose(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   DESCRIPTOR_DATA *d;
   int desc;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Usage: forceclose <descriptor#>\n\r", ch);
      return;
   }

   desc = atoi(arg);
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->descriptor == desc)
      {
         if (d->character && get_trust(d->character) >= get_trust(ch))
         {
            send_to_char("They might not like that...\n\r", ch);
            return;
         }
         close_socket(d, FALSE);
         send_to_char("Ok.\n\r", ch);
         return;
      }
   }
   send_to_char("Not found!\n\r", ch);
   return;
}

void echo_to_all(sh_int AT_COLOR, char *argument, sh_int tar)
{
   DESCRIPTOR_DATA *d;

   if (!argument || argument[0] == '\0')
      return;

   for (d = first_descriptor; d; d = d->next)
   {
      /* Added showing echoes to players who are editing, so they won't
         miss out on important info like upcoming reboots. --Narn */
      if (d->connected == CON_PLAYING || d->connected == CON_EDITING)
      {
         /* This one is kinda useless except for switched.. */
         if (tar == ECHOTAR_PC && IS_NPC(d->character))
            continue;
         else if (tar == ECHOTAR_IMM && !IS_IMMORTAL(d->character))
            continue;
         set_char_color(AT_COLOR, d->character);
         send_to_char(argument, d->character);
         send_to_char("\n\r", d->character);
      }
   }
   return;
}

void do_echo(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   sh_int color;
   int target;
   char *parg;

   set_char_color(AT_IMMORT, ch);

   if (xIS_SET(ch->act, PLR_NO_EMOTE))
   {
      send_to_char("You can't do that right now.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Echo what?\n\r", ch);
      return;
   }

   if ((color = get_color(argument)))
      argument = one_argument(argument, arg);
   parg = argument;
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "PC") || !str_cmp(arg, "player"))
      target = ECHOTAR_PC;
   else if (!str_cmp(arg, "imm"))
      target = ECHOTAR_IMM;
   else
   {
      target = ECHOTAR_ALL;
      argument = parg;
   }
   if (!color && (color = get_color(argument)))
      argument = one_argument(argument, arg);
   if (!color)
      color = AT_IMMORT;
   one_argument(argument, arg);
   if (!str_cmp(arg, "Thoric") || !str_cmp(arg, "Circe") || !str_cmp(arg, "Haus") || !str_cmp(arg, "Scryn") || !str_cmp(arg, "Blodkai"))
   {
      ch_printf(ch, "I don't think %s would like that!\n\r", arg);
      return;
   }
   echo_to_all(color, argument, target);
}

void echo_to_room(sh_int AT_COLOR, ROOM_INDEX_DATA * room, char *argument)
{
   CHAR_DATA *vic;

   for (vic = room->first_person; vic; vic = vic->next_in_room)
   {
      set_char_color(AT_COLOR, vic);
      send_to_char(argument, vic);
      send_to_char("\n\r", vic);
   }
}

void do_recho(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   sh_int color;

   set_char_color(AT_IMMORT, ch);

   if (xIS_SET(ch->act, PLR_NO_EMOTE))
   {
      send_to_char("You can't do that right now.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Recho what?\n\r", ch);
      return;
   }

   one_argument(argument, arg);
   if (!str_cmp(arg, "Thoric") || !str_cmp(arg, "Circe") || !str_cmp(arg, "Haus") || !str_cmp(arg, "Scryn") || !str_cmp(arg, "Blodkai"))
   {
      ch_printf(ch, "I don't think %s would like that!\n\r", arg);
      return;
   }
   if ((color = get_color(argument)))
   {
      argument = one_argument(argument, arg);
      echo_to_room(color, ch->in_room, argument);
   }
   else
      echo_to_room(AT_IMMORT, ch->in_room, argument);
}

ROOM_INDEX_DATA *find_location(CHAR_DATA * ch, char *arg)
{
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   if (is_number(arg))
      return get_room_index(atoi(arg));

   if (!str_cmp(arg, "pk")) /* "Goto pk", "at pk", etc */
      return get_room_index(last_pkroom);

   if ((victim = get_char_world(ch, arg)) != NULL)
      return victim->in_room;

   if ((obj = get_obj_world(ch, arg)) != NULL)
      return obj->in_room;

   return NULL;
}

void do_wblock(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   WBLOCK_DATA *wblock;
   WINFO_DATA *winfo;
   int cnt = 0;
   int recent = 0;
  
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("Only in the wilderness.\n\r", ch);
      return;
   }
   for (wblock = first_wblock; wblock; wblock = wblock->next)
   {   
      if (ch->coord->x <= wblock->endx && ch->coord->x >= wblock->stx 
      && ch->coord->y <= wblock->endy && ch->coord->y >= wblock->sty && ch->map == wblock->map)
      {     
         break;
      }
   }
   if (!wblock)
   {
      send_to_char("An error occured, tell Xerves.\n\r", ch);
      bug("do_wblock: In Wilderness but not in wblock section (%d %d).\n\r", ch->coord->x, ch->coord->y);
      return;
   }
  
   if (argument[0] == '\0') //Send the default info for the room you are in
   {             
      ch_printf(ch, "Startx: %d\n\r", wblock->stx);
      ch_printf(ch, "Endx:   %d\n\r", wblock->endx);
      ch_printf(ch, "Starty: %d\n\r", wblock->sty);
      ch_printf(ch, "Endy:   %d\n\r", wblock->endy); 
      ch_printf(ch, "Map     %d\n\r", wblock->map);
      ch_printf(ch, "Level   %d\n\r", wblock->lvl);
      ch_printf(ch, "Kills   %d\n\r", wblock->kills);
       
      for (winfo = wblock->first_player; winfo; winfo = winfo->next)
      {
         cnt++;
         if (time(0) - winfo->time <= 7200)
            recent++;
      }
      ch_printf(ch, "%d Players have visited\n\r", cnt);
      ch_printf(ch, "%d Of those players recently visited\n\r", recent);
      ch_printf(ch, "Type wblock players to see the full list of visited\n\r");
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_prefix(arg, "players"))
   {
      for (winfo = wblock->first_player; winfo; winfo = winfo->next)
      {
        int dtime, sec, min, hour;
        char buf[150];
        dtime = time(0) - winfo->time;
        if (dtime > 86400)
        {
           dtime = dtime/86400;
           sprintf(buf, "Days Ago: %d\n\r", dtime);
        }
        else
        {
           sec = dtime % 60;
           hour = dtime / 3600;
           if (dtime > 60)
           {
              min = dtime / 60;
              min = min % 60;
           }
           else
           {
              min = 0;
           }
           sprintf(buf, "Hours: %d Minutes %d Seconds %d", hour, min, sec);
        }
        ch_printf(ch, "Pid: %d      %s\n\r", winfo->pid, buf); 
      }
      return;
   }
   if (!str_prefix(arg, "Startx") || !str_prefix(arg, "Starty") || !str_prefix(arg, "Endx") || !str_prefix(arg, "Endy"))
   {
      send_to_char("You cannot change those, sorry.\n\r", ch);
      return;
   }
   if (!str_prefix(arg, "Kills"))
   {
      if (get_trust(ch) >= LEVEL_STAFF)
      {
         if (atoi(argument) < 0 || atoi(argument) > 100)
            send_to_char("Range is 0 to 100\n\r", ch);
         wblock->kills = atoi(argument);
         save_wblock_data();
      }
      else
      {
         send_to_char("You do not have the power to do that.\n\r", ch);
         return;
      }
   }
   if (!str_prefix(arg, "Levels"))
   {
      if (get_trust(ch) >= LEVEL_STAFF)
      {
         if (atoi(argument) < 0 || atoi(argument) > 100)
            send_to_char("Range is 0 to 100\n\r", ch);
         wblock->lvl = atoi(argument);
         save_wblock_data();
      }
      else
      {
         send_to_char("You do not have the power to do that.\n\r", ch);
         return;
      }
   }
   do_wblock(ch, "");
}
      
void do_transfer(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];
   ROOM_INDEX_DATA *location;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   sh_int x = -1;
   sh_int y = -1;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Transfer whom (and where)?\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "all"))
   {
      for (d = first_descriptor; d; d = d->next)
      {
         if (d->connected == CON_PLAYING
            && d->character != ch
            && d->character->in_room
            && d->newstate != 2
            && can_see_map(ch, d->character))
         {
            if (xIS_SET(d->character->act, PLR_NOTRANS))
            {
               if (get_trust(ch) >= get_trust(d->character))
               {
                  sprintf(buf, "%s %s", d->character->name, arg2);
                  do_transfer(ch, buf);
               }
            }
            else
               continue;
            sprintf(buf, "%s %s", d->character->name, arg2);
            do_transfer(ch, buf);
         }
      }
      return;
   }

   /*
    * Thanks to Grodyn for the optional location parameter.
    */

   if (arg2[0] == '\0')
   {
      location = ch->in_room;
   }
   else
   {
      if ((location = find_location(ch, arg2)) == NULL)
      {
         send_to_char("No such location.\n\r", ch);
         return;
      }
      if (room_is_private(location) && get_trust(ch) < sysdata.level_override_private)
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
   }
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      if (xIS_SET(victim->in_room->room_flags, ROOM_IMP))
      {
         set_char_color(AT_WHITE, ch);
         send_to_char("Sorry, the person you tried to transfer is in an admin room\n\r", ch);
         return;
      }
   }
   if (!IS_NPC(victim))
   {
      if (xIS_SET(victim->act, PLR_GAMBLER))
      {
         send_to_char("Wait till they are done gambling and try again.\n\r", ch);
         return;
      }
   }
   if (xIS_SET(victim->act, PLR_NOTRANS))
   {
      if (get_trust(ch) < get_trust(victim))
      {
         send_to_char("Sorry, the person you tried to transfer has a notrans flag\n\r", ch);
         return;
      }
   }
   if (ch == victim || (arg2[0] != '\0'))
   {
      if (!IS_NPC(victim))
      {
         if (xIS_SET(location->room_flags, ROOM_IMP) && victim->pcdata->caste < caste_Staff)
         {
            set_char_color(AT_WHITE, ch);
            set_char_color(AT_WHITE, victim);
            send_to_char("That room is for staff only!\n\r", ch);
            send_to_char("Transfer stopped because of staff only room!\n\r", victim);
            return;
         }
      }
      else
      {
         if (xIS_SET(location->room_flags, ROOM_IMP))
            return;
      }
   }
   if (NOT_AUTHED(victim))
   {
      send_to_char("They are not authorized yet!\n\r", ch);
      return;
   }
   if (!victim->in_room)
   {
      send_to_char("They have no physical location!\n\r", ch);
      return;
   }
   if (IS_NPC(victim) && (xIS_SET(victim->act, ACT_PROTECT)) && (get_trust(ch) < LEVEL_ADMIN))
   {
      send_to_char("This mob is protected against forces, transfers, slays, and purges\n\r", ch);
      return;
   }
   if (location->vnum == OVERLAND_SOLAN)
   {
      if (!xIS_SET(ch->act, PLR_ONMAP) && xIS_SET(victim->act, ACT_ONMAP))
      {
         send_to_char("Sorry, cannot transfer a mob/player around on the map while you aren't on it.\n\r", ch);
         return;
      }
   }
   if (victim->fighting)
      stop_fighting(victim, TRUE);
   act(AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM);
   victim->retran = victim->in_room->vnum;
   victim->retran_x = victim->coord->x;
   victim->retran_y = victim->coord->y;
   victim->retran_map = victim->map;
   char_from_room(victim);
   char_to_room(victim, location);

   /* To properly handle map transfers if the immortal is on the map.
    * Also handles cases of transing from one map to another - Samson 8-21-99
    */
   /* Redid to make sure it works right :-) -- Xerves */
   if (location->vnum == OVERLAND_SOLAN)
   {
      argument = one_argument(argument, arg3);
      if (arg3[0] != '\0' && argument[0] != '\0')
      {
         x = atoi(arg3);
         y = atoi(argument);

         if (x < 1 || x > MAX_X)
         {
            sprintf(buf, "Valid x coordinates are 1 to %d.\n\rThrowing out coordinates.", MAX_X);
            send_to_char(buf, ch);
            x = -1;
            y = -1;
         }

         if (y < 1 || y > MAX_Y)
         {
            sprintf(buf, "Valid y coordinates are 1 to %d.\n\rThrowing out coordinates.", MAX_Y);
            send_to_char(buf, ch);
            x = -1;
            x = -1;
         }
      }
      if (room_is_private_wilderness(ch, location, ch->coord->x, ch->coord->y, ch->map) && get_trust(ch) < sysdata.level_override_private)
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      if (!IS_NPC(victim))
      {
         if (xIS_SET(ch->act, PLR_ONMAP) && !xIS_SET(victim->act, PLR_ONMAP))
         {
            xSET_BIT(victim->act, PLR_ONMAP);

            victim->map = ch->map;
            if (x == -1 && y == -1)
            {
               victim->coord->x = ch->coord->x;
               victim->coord->y = ch->coord->y;
            }
            else
            {
               victim->coord->x = x;
               victim->coord->y = y;
            }
         }
         else if (!xIS_SET(ch->act, PLR_ONMAP) && !xIS_SET(victim->act, PLR_ONMAP))
         {
            xSET_BIT(victim->act, PLR_ONMAP);

            victim->map = MAP_SOLAN;
            if (x == -1 && y == -1)
            {
               victim->coord->x = ENTRY_X;
               victim->coord->y = ENTRY_Y;
            }
            else
            {
               victim->coord->x = x;
               victim->coord->y = y;
            }
         }
         else if (xIS_SET(ch->act, PLR_ONMAP) && xIS_SET(victim->act, PLR_ONMAP))
         {
            victim->map = ch->map;
            if (x == -1 && y == -1)
            {
               victim->coord->x = ch->coord->x;
               victim->coord->y = ch->coord->y;
            }
            else
            {
               victim->coord->x = x;
               victim->coord->y = y;
            }
         }
      }
      else
      {
         if (xIS_SET(ch->act, PLR_ONMAP) && !xIS_SET(victim->act, ACT_ONMAP))
         {
            xSET_BIT(victim->act, ACT_ONMAP);

            victim->map = ch->map;
            victim->coord->x = ch->coord->x;
            victim->coord->y = ch->coord->y;
         }
         else if (!xIS_SET(ch->act, PLR_ONMAP) && !xIS_SET(victim->act, ACT_ONMAP))
         {
            xSET_BIT(victim->act, ACT_ONMAP);

            victim->map = MAP_SOLAN;
            victim->coord->x = ENTRY_X;
            victim->coord->y = ENTRY_Y;
         }
         else if (xIS_SET(ch->act, PLR_ONMAP) && xIS_SET(victim->act, ACT_ONMAP))
         {
            victim->map = ch->map;
            victim->coord->x = ch->coord->x;
            victim->coord->y = ch->coord->y;
         }
      }
   }
   else
   {
      if (IS_NPC(victim))
         xREMOVE_BIT(victim->act, ACT_ONMAP);
      else
         xREMOVE_BIT(victim->act, PLR_ONMAP);

      victim->map = -1;
      victim->coord->x = -1;
      victim->coord->y = -1;
   }
   
   if (victim->mount)
   {
      char_from_room(victim->mount);
      char_to_room(victim->mount, victim->in_room);
      victim->mount->coord->x = victim->coord->x;
      victim->mount->coord->y = victim->coord->y;
      victim->mount->map = victim->map;
      if (victim->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->mount);
      else
         SET_ONMAP_FLAG(victim->mount);
   }  
   if (!IS_NPC(victim) && victim->pcdata->pet)
   {
      char_from_room(victim->pcdata->pet);
      char_to_room(victim->pcdata->pet, victim->in_room);
      victim->pcdata->pet->coord->x = victim->coord->x;
      victim->pcdata->pet->coord->y = victim->coord->y;
      victim->pcdata->pet->map = victim->map;
      if (victim->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->pcdata->pet);
      else
         SET_ONMAP_FLAG(victim->pcdata->pet);
   }  
   if (!IS_NPC(victim) && victim->pcdata->mount && !victim->mount)
   {
      char_from_room(victim->pcdata->mount);
      char_to_room(victim->pcdata->mount, victim->in_room);
      victim->pcdata->mount->coord->x = victim->coord->x;
      victim->pcdata->mount->coord->y = victim->coord->y;
      victim->pcdata->mount->map = victim->map;
      if (victim->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->pcdata->mount);
      else
         SET_ONMAP_FLAG(victim->pcdata->mount);
   }  
   if (victim->rider)
   {
      char_from_room(victim->rider);
      char_to_room(victim->rider, victim->in_room);
      victim->rider->coord->x = victim->coord->x;
      victim->rider->coord->y = victim->coord->y;
      victim->rider->map = victim->map;
      if (victim->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->rider);
      else
         SET_ONMAP_FLAG(victim->rider);
      update_objects(victim->rider, victim->rider->map, victim->rider->coord->x, victim->rider->coord->y);
   }  
   if (victim->riding)
   {
      char_from_room(victim->riding);
      char_to_room(victim->riding, victim->in_room);
      victim->riding->coord->x = victim->coord->x;
      victim->riding->coord->y = victim->coord->y;
      victim->riding->map = victim->map;
      if (victim->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->riding);
      else
         SET_ONMAP_FLAG(victim->riding);
      update_objects(victim->riding, victim->riding->map, victim->riding->coord->x, victim->riding->coord->y);
   }  

   if (victim->on)
   {
      victim->on = NULL;
      victim->position = POS_STANDING;
   }
   if (victim->position != POS_STANDING && victim->position != POS_RIDING)
   {
      victim->position = POS_STANDING;
   }


   update_objects(victim, victim->coord->x, victim->coord->y, victim->map);
   act(AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM);
   if (ch != victim)
      act(AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT);
   do_look(victim, "auto");
}

void do_retran(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   char buf[MSL];

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Retransfer whom?\n\r", ch);
      return;
   }
   if (!(victim = get_char_world(ch, arg)))
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   sprintf(buf, "'%s' %d", victim->name, victim->retran);
   do_transfer(ch, buf);
   victim->coord->x = victim->retran_x;
   victim->coord->y = victim->retran_y;
   victim->map = victim->retran_map;
   if (victim->coord->x >= 1 && victim->coord->y >= 1 && victim->map >= 0)
   {
      if (IS_NPC(victim))
         xSET_BIT(victim->act, ACT_ONMAP);
      else
         xSET_BIT(victim->act, PLR_ONMAP);
   }
   return;
}

void do_regoto(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];

   sprintf(buf, "%d", ch->regoto);

   if (ch->regoto_x >= 1 && ch->regoto_y >= 1 && ch->regoto_map >= 0)
      sprintf(buf, "map solan %d %d", ch->regoto_x, ch->regoto_y);

   do_goto(ch, buf);

   return;
}

/*  Added do_atmob and do_atobj to reduce lag associated with at
 *  --Shaddai
 */
void do_atmob(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *wch;
   sh_int origmap, origx, origy;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("At where what?\n\r", ch);
      return;
   }
   if ((wch = get_char_world(ch, arg)) == NULL || !IS_NPC(wch) || wch->in_room == NULL)
   {
      send_to_char("No such mobile in existance.\n\r", ch);
      return;
   }
   location = wch->in_room;
   if (room_is_private(location) || room_is_private_wilderness(ch, location, wch->coord->x, wch->coord->y, wch->map))
   {
      if (get_trust(ch) < sysdata.level_override_private) /* Tracker1 */
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Overriding private flag!\n\r", ch);
      }
   }

   origmap = ch->map;
   origx = ch->coord->x;
   origy = ch->coord->y;

   /* Bunch of checks to make sure the "ator" is temporarily on the same grid as
    * the "atee" - Samson
    */

   if (xIS_SET(location->room_flags, ROOM_MAP) && !IS_ONMAP_FLAG(ch))
   {
      SET_ONMAP_FLAG(ch);
      ch->map = wch->map;
      ch->coord->x = wch->coord->x;
      ch->coord->y = wch->coord->y;
   }
   else if (xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      ch->map = wch->map;
      ch->coord->x = wch->coord->x;
      ch->coord->y = wch->coord->y;
   }
   else if (!xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      REMOVE_ONMAP_FLAG(ch);
      ch->map = -1;
      ch->coord->x = -1;
      ch->coord->y = -1;
   }

   set_char_color(AT_PLAIN, ch);
   original = ch->in_room;
   char_from_room(ch);
   char_to_room(ch, location);
   update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
   interpret(ch, argument);

   if (IS_ONMAP_FLAG(ch) && !xIS_SET(original->room_flags, ROOM_MAP))
      REMOVE_ONMAP_FLAG(ch);
   else if (!IS_ONMAP_FLAG(ch) && xIS_SET(original->room_flags, ROOM_MAP))
      SET_ONMAP_FLAG(ch);

   ch->map = origmap;
   ch->coord->x = origx;
   ch->coord->y = origy;

   update_objects(ch, ch->coord->x, ch->coord->y, ch->map);

   if (!char_died(ch))
   {
      char_from_room(ch);
      char_to_room(ch, original);
   }
   return;
}

void do_atobj(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   OBJ_DATA *obj;
   sh_int origmap, origx, origy;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("At where what?\n\r", ch);
      return;
   }

   if ((obj = get_obj_world(ch, arg)) == NULL || !obj->in_room)
   {
      send_to_char("No such object in existance.\n\r", ch);
      return;
   }
   location = obj->in_room;
   if (room_is_private(location) || (xIS_SET(location->room_flags, ROOM_MAP) &&  room_is_private_wilderness(ch, location, obj->coord->x, obj->coord->y, obj->map)))
   {
      if (get_trust(ch) < sysdata.level_override_private) /* Tracker1 */
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Overriding private flag!\n\r", ch);
      }
   }

   origmap = ch->map;
   origx = ch->coord->x;
   origy = ch->coord->y;

   /* Bunch of checks to make sure the imm is on the same grid as the object - Samson */
   if (xIS_SET(location->room_flags, ROOM_MAP) && !IS_ONMAP_FLAG(ch))
   {
      SET_ONMAP_FLAG(ch);
      ch->map = obj->map;
      ch->coord->x = obj->coord->x;
      ch->coord->y = obj->coord->y;
   }
   else if (xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      ch->map = obj->map;
      ch->coord->x = obj->coord->x;
      ch->coord->y = obj->coord->y;
   }
   else if (!xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      REMOVE_ONMAP_FLAG(ch);
      ch->map = -1;
      ch->coord->x = -1;
      ch->coord->y = -1;
   }

   set_char_color(AT_PLAIN, ch);
   original = ch->in_room;
   char_from_room(ch);
   char_to_room(ch, location);
   update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
   interpret(ch, argument);

   if (IS_ONMAP_FLAG(ch) && !xIS_SET(original->room_flags, ROOM_MAP))
      REMOVE_ONMAP_FLAG(ch);
   else if (!IS_ONMAP_FLAG(ch) && xIS_SET(original->room_flags, ROOM_MAP))
      SET_ONMAP_FLAG(ch);

   ch->map = origmap;
   ch->coord->x = origx;
   ch->coord->y = origy;
   update_objects(ch, ch->coord->x, ch->coord->y, ch->map);

   if (!char_died(ch))
   {
      char_from_room(ch);
      char_to_room(ch, original);
   }
   return;
}

void do_at(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *location = NULL;
   ROOM_INDEX_DATA *original;
   DESCRIPTOR_DATA *d = NULL;
   sh_int cfound = 0;
   sh_int origmap, origx, origy;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("At where what?\n\r", ch);
      return;
   }
   if (is_number(arg))
      location = get_room_index(atoi(arg));
   else if (!str_cmp(arg, "pk"))
      location = get_room_index(last_pkroom);
   else
   {
      for (d = first_descriptor; d; d = d->next)
      {
         if ((d->connected != CON_PLAYING && d->connected != CON_EDITING) || !can_see_map(ch, d->character) || str_cmp(d->character->name, arg))
            continue;
         break;
      }
      if (d)
      {
         cfound = 1;
         location = d->character->in_room;
      }
   }

   if (!location)
   {
      send_to_char("No such location.\n\r", ch);
      return;
   }
   if (room_is_private(location))
   {
      if (get_trust(ch) < sysdata.level_override_private) /* Tracker1 */
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Overriding private flag!\n\r", ch);
      }
   }
   if (xIS_SET(location->room_flags, ROOM_IMP))
   {
      send_to_char("Sorry you cannot attempt to at in a staff room.\n\r", ch);
      return;
   }
   if (xIS_SET(ch->in_room->room_flags, ROOM_IMP))
   {
      send_to_char("Please leave the staff room before doing at.\n\r", ch);
      return;
   }


   origmap = ch->map;
   origx = ch->coord->x;
   origy = ch->coord->y;

   /* Bunch of checks to make sure the imm is on the same grid as the object - Samson */
   if (d)
   {
      if (xIS_SET(location->room_flags, ROOM_MAP) && !IS_ONMAP_FLAG(ch))
      {
         SET_ONMAP_FLAG(ch);
         ch->map = d->character->map;
         ch->coord->x = d->character->coord->x;
         ch->coord->y = d->character->coord->y;
      }
      else if (xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
      {
         ch->map = d->character->map;
         ch->coord->x = d->character->coord->x;
         ch->coord->y = d->character->coord->y;
      }
      else if (!xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
      {
         REMOVE_ONMAP_FLAG(ch);
         ch->map = -1;
         ch->coord->x = -1;
         ch->coord->y = -1;
      }
      set_char_color(AT_PLAIN, ch);
      original = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, location);
      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
      if (!room_is_private_wilderness(ch, location, ch->coord->x, ch->coord->y, ch->map) && get_trust(ch) >= sysdata.level_override_private)
      {
         interpret(ch, argument);
      }
      else
      {
         send_to_char("This room is private.\n\r", ch);
      }


      if (IS_ONMAP_FLAG(ch) && !xIS_SET(original->room_flags, ROOM_MAP))
         REMOVE_ONMAP_FLAG(ch);
      else if (!IS_ONMAP_FLAG(ch) && xIS_SET(original->room_flags, ROOM_MAP))
         SET_ONMAP_FLAG(ch);

      ch->map = origmap;
      ch->coord->x = origx;
      ch->coord->y = origy;

      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
   }
   else
   {
      REMOVE_ONMAP_FLAG(ch);
      ch->map = -1;
      ch->coord->x = -1;
      ch->coord->y = -1;
      set_char_color(AT_PLAIN, ch);
      original = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, location);
      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
      if (!room_is_private_wilderness(ch, location, ch->coord->x, ch->coord->y, ch->map) && get_trust(ch) >= sysdata.level_override_private)
      {
         interpret(ch, argument);
      }
      else
      {
         send_to_char("This room is private.\n\r", ch);
      }


      if (IS_ONMAP_FLAG(ch) && !xIS_SET(original->room_flags, ROOM_MAP))
         REMOVE_ONMAP_FLAG(ch);
      else if (!IS_ONMAP_FLAG(ch) && xIS_SET(original->room_flags, ROOM_MAP))
         SET_ONMAP_FLAG(ch);

      ch->map = origmap;
      ch->coord->x = origx;
      ch->coord->y = origy;

      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
   }
   if (!char_died(ch))
   {
      char_from_room(ch);
      char_to_room(ch, original);
   }
   return;
}

void do_rat(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   int Start, End, vnum;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Syntax: rat <start> <end> <command>\n\r", ch);
      return;
   }

   Start = atoi(arg1);
   End = atoi(arg2);
   if (Start < 1 || End < Start || Start > End || Start == End || End > MAX_VNUM)
   {
      send_to_char("Invalid range.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "quit"))
   {
      send_to_char("I don't think so!\n\r", ch);
      return;
   }

   original = ch->in_room;
   for (vnum = Start; vnum <= End; vnum++)
   {
      if ((location = get_room_index(vnum)) == NULL)
         continue;
      char_from_room(ch);
      char_to_room(ch, location);
      interpret(ch, argument);
   }

   char_from_room(ch);
   char_to_room(ch, original);
   send_to_char("Done.\n\r", ch);
   return;
}

void do_rstat(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   int value;
   char arg[MIL];
   char *sect;
   ROOM_INDEX_DATA *location;
   OBJ_DATA *obj;
   CHAR_DATA *rch;
   EXIT_DATA *pexit;
   AFFECT_DATA *paf;
   int cnt;
   static char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };

   one_argument(argument, arg);
   if (!str_cmp(arg, "ex") || !str_cmp(arg, "exits"))
   {
      location = ch->in_room;

      ch_printf_color(ch, "&cExits for room '&W%s&c'  Vnum &W%d\n\r", location->name, location->vnum);
      for (cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next)
         ch_printf_color(ch,
            "&W%2d) &c&w%2s to %-5d  &cKey: &c&w%d  &cFlags: &c&w%d  &cKeywords: '&c&w%s&c'\n\r     Exdesc: &c&w%s     &cBack link: &c&w%d  &cVnum: &c&w%d  &cDistance: &c&w%d  &cPulltype: &c&w%s  &cPull: &c&w%d\n\r",
            ++cnt,
            dir_text[pexit->vdir],
            pexit->to_room ? pexit->to_room->vnum : 0,
            pexit->key,
            pexit->exit_info,
            pexit->keyword,
            pexit->description[0] != '\0'
            ? pexit->description : "(none).\n\r",
            pexit->rexit ? pexit->rexit->vnum : 0, pexit->rvnum, pexit->distance, pull_type_name(pexit->pulltype), pexit->pull);
      return;
   }
   location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
   if (!location)
   {
      send_to_char("No such location.\n\r", ch);
      return;
   }

   if (ch->in_room != location && room_is_private(location))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker */
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      else
         send_to_char("Overriding private flag!\n\r", ch);
   }

   ch_printf_color(ch, "&cName: &c&w%s\n\r&cArea: &c&w%s  &cFilename: &c&w%s\n\r",
      location->name, location->area ? location->area->name : "None????", location->area ? location->area->filename : "None????");

   switch (ch->in_room->sector_type)
   {
      default:
         sect = "<???>";
         break;
      case SECT_INSIDE:
         sect = "Inside";
         break;
      case SECT_CITY:
         sect = "City";
         break;
      case SECT_FIELD:
         sect = "Field";
         break;
      case SECT_FOREST:
         sect = "Forest";
         break;
      case SECT_HILLS:
         sect = "Hills";
         break;
      case SECT_MOUNTAIN:
         sect = "Mountains";
         break;
      case SECT_WATER_SWIM:
         sect = "Swim";
         break;
      case SECT_WATER_NOSWIM:
         sect = "Noswim";
         break;
      case SECT_UNDERWATER:
         sect = "Underwater";
         break;
      case SECT_AIR:
         sect = "Air";
         break;
      case SECT_DESERT:
         sect = "Desert";
         break;
      case SECT_OCEANFLOOR:
         sect = "Oceanfloor";
         break;
      case SECT_UNDERGROUND:
         sect = "Underground";
         break;
      case SECT_MINEGOLD:
         sect = "Gold Mine";
         break;
      case SECT_MINEIRON:
         sect = "Iron Mine";
         break;
      case SECT_HCORN:
         sect = "Corn Field";
         break;
      case SECT_HGRAIN:
         sect = "Grain Field";
         break;
      case SECT_ENTER:
         sect = "Enter";
         break;
      case SECT_ROAD:
         sect = "Road";
         break;
      case SECT_STREE:
         sect = "Chopped Forest";
         break;
      case SECT_NTREE:
         sect = "Treeless Forest";
         break;
      case SECT_SGOLD:
         sect = "Mined Gold Mine";
         break;
      case SECT_NGOLD:
         sect = "Empty Gold Mine";
         break;
      case SECT_SIRON:
         sect = "Mined Iron Mine";
         break;
      case SECT_NIRON:
         sect = "Empty Iron Mine";
         break;
      case SECT_SCORN:
         sect = "Harvested Corn";
         break;
      case SECT_NCORN:
         sect = "No Corn";
         break;
      case SECT_SGRAIN:
         sect = "Harvested Grain";
         break;
      case SECT_NGRAIN:
         sect = "No Grain";
         break;
   }

   ch_printf_color(ch, "&cVnum: &c&w%d   &cSector: &c&w%d (%s)   &cLight: &c&w%d   &CResource: &c&w%d",
      location->vnum, location->sector_type, sect, location->light, location->resource);
   if (location->tunnel > 0)
      ch_printf_color(ch, "   &cTunnel: &W%d", location->tunnel);
   send_to_char("\n\r", ch);

   ch_printf_color(ch, "&cQuadrant: &c&w%d\n\r", location->quad);

   if (location->tele_delay > 0 || location->tele_vnum > 0)
      ch_printf_color(ch, "&cTeleDelay: &R%d   &cTeleVnum: &R%d\n\r", location->tele_delay, location->tele_vnum);
   ch_printf_color(ch, "&cRoom flags: &c&w%s\n\r", ext_flag_string(&location->room_flags, r_flags));
   ch_printf_color(ch, "&cDescription:\n\r&c&w%s", location->description);
   if (location->first_extradesc)
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char_color("&cExtra description keywords: &c&w'", ch);
      for (ed = location->first_extradesc; ed; ed = ed->next)
      {
         send_to_char(ed->keyword, ch);
         if (ed->next)
            send_to_char(" ", ch);
      }
      send_to_char("'\n\r", ch);
   }
   for (paf = location->first_affect; paf; paf = paf->next)
      ch_printf_color(ch, "&cAffect: &c&w%s &cby &c&w%d.\n\r", affect_loc_name(paf->location), paf->modifier);

   send_to_char_color("&cCharacters: &c&w", ch);
   for (rch = location->first_person; rch; rch = rch->next_in_room)
   {
      if (can_see(ch, rch))
      {
         send_to_char(" ", ch);
         one_argument(rch->name, buf);
         send_to_char(buf, ch);
      }
   }

   send_to_char_color("\n\r&cObjects:    &c&w", ch);
   for (obj = location->first_content; obj; obj = obj->next_content)
   {
      send_to_char(" ", ch);
      one_argument(obj->name, buf);
      send_to_char(buf, ch);
   }
   send_to_char("\n\r", ch);

   if(xIS_SET(ch->in_room->room_flags, ROOM_MANANODE))
   {
      if(!ch->in_room->node_mana)
          ch_printf_color(ch, "&cNode Mana: &c&w%d\n\r", 0);
      else
          ch_printf_color(ch, "&cNode Mana: &c&w%d\n\r", ch->in_room->node_mana);
   }
   
   if (location->first_exit)
      send_to_char_color("&c------------------- &c&wEXITS &c-------------------\n\r", ch);
      
   for (cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next)
   {
      sprintf(buf, "[ ");
      for (value = 0; value <= MAX_EXFLAG; value++)
      {
         if (IS_SET(pexit->exit_info, 1 << value))
         {
            strcat(buf, ex_flags[value]);
            strcat(buf, " ");
         }
      }
      strcat(buf, "]");
      ch_printf(ch,
         "%2d) %-2s to %-5d.  Key: %d  Keywords: %s.\n\r       Flags: %s\n\r",
         ++cnt,
         dir_text[pexit->vdir],
         pexit->to_room ? pexit->to_room->vnum : 0, pexit->key, pexit->keyword[0] != '\0' ? pexit->keyword : "(none)", buf);
   }   
   return;
}

/* Face-lift by Demora */
void do_ostat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[100];
   AFFECT_DATA *paf;
   int cnt = 0;
   OBJ_DATA *obj;
   IMBUE_DATA *imbue;

   set_char_color(AT_CYAN, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Ostat what?\n\r", ch);
      return;
   }
   if (arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg))
      strcpy(arg, argument);

   if ((obj = get_obj_world(ch, arg)) == NULL)
   {
      send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
      return;
   }
   ch_printf_color(ch, "&cName: &C%s\n\r", obj->name);
   ch_printf_color(ch, "&cVnum: &w%d  ", obj->pIndexData->vnum);
   ch_printf_color(ch, "&cType: &w%s  ", item_type_name(obj));
   ch_printf_color(ch, "&cGcount:  &w%d  ", obj->pIndexData->count);
   ch_printf_color(ch, "&cCount: &w%d\n\r", obj->count);
   ch_printf_color(ch, "&cSerial#: &w%d  ", obj->serial);
   ch_printf_color(ch, "&cTopIdxSerial#: &w%d  ", obj->pIndexData->serial);
   ch_printf_color(ch, "&cTopSerial#: &w%d\n\r", cur_obj_serial);
   ch_printf_color(ch, "&cShort description: &C%s\n\r", obj->short_descr);
   ch_printf_color(ch, "&cLong description : &C%s\n\r", obj->description);
   if (obj->action_desc[0] != '\0')
      ch_printf_color(ch, "&cAction description: &w%s\n\r", obj->action_desc);
   ch_printf_color(ch, "&cWear flags : &w%s\n\r", flag_string(obj->wear_flags, w_flags));
   ch_printf_color(ch, "&cExtra flags: &w%s\n\r", ext_flag_string(&obj->extra_flags, o_flags));
   ch_printf_color(ch, "&cMagic flags: &w%s\n\r", magic_bit_name(obj->magic_flags));
   ch_printf_color(ch, "&cNumber: &w%d/%d   ", 1, get_obj_number(obj));
   ch_printf_color(ch, "&cWeight: &w%.2f/%.2f   ", obj->weight, get_obj_weight(obj));
   ch_printf_color(ch, "&cLayers: &w%d   ", obj->pIndexData->layers);
   ch_printf_color(ch, "&cWear_loc: &w%d\n\r", obj->wear_loc);
   ch_printf_color(ch, "&cCost: &Y%d  ", obj->cost);
   ch_printf_color(ch, "&cRent: &w%d  ", obj->pIndexData->rent);
   ch_printf_color(ch, "&cSworthrestrict: &c&w%d/%d  ", obj->sworthrestrict, obj->pIndexData->sworthrestrict);
   ch_printf_color(ch, "&cGemslots:  &c&w%d/%d  ", obj->imbueslots, obj->pIndexData->imbueslots);
   ch_printf_color(ch, "\n\r&cOn map: &w%s ", IS_OBJ_STAT(obj, ITEM_ONMAP) ? map_names[obj->map] : "(none)");

   ch_printf_color(ch, "&cCoords: &w%d %d\n\r", obj->coord->x, obj->coord->y);
   send_to_char_color("&cTimer: ", ch);
   if (obj->timer > 0)
      ch_printf_color(ch, "&R%d  ", obj->timer);
   else
      ch_printf_color(ch, "&w%d  ", obj->timer);
   ch_printf_color(ch, "&cLevel: &P%d  ", obj->level);
   ch_printf_color(ch, "&cCvnum: &P%d\n\r", obj->cvnum);
   ch_printf_color(ch, "&cIn room: &w%d  ", obj->in_room == NULL ? 0 : obj->in_room->vnum);
   ch_printf_color(ch, "&cIn object: &w%s  ", obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr);
   ch_printf_color(ch, "&cCarried by: &C%s\n\r", obj->carried_by == NULL ? "(none)" : obj->carried_by->name);
   if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON)
   {
      ch_printf_color(ch, "&BBashMod &z%d    &BSlashMod &z%d     &BStabMod &z%d    &BDurability &z%d    &BParry &z%d/%d\n\r", 
          obj->value[7], obj->value[8], obj->value[9], obj->value[10], obj->value[12], obj->value[13]);
   }
   if (obj->item_type == ITEM_ARMOR)
   {
      if (obj->value[5] == 1)
         sprintf(buf, "Leather");
      else if (obj->value[5] == 2)
         sprintf(buf, "Light");
      else if (obj->value[5] == 3)
         sprintf(buf, "Medium");
      else if (obj->value[5] == 4)
         sprintf(buf, "Heavy");
      else if (obj->value[5] == 5)
         sprintf(buf, "Heaviest");
      else
         sprintf(buf, "NULL");
      ch_printf_color(ch, "&BBashMod &z%d    &BSlashMod &z%d     &BStabMod &z%d    &BArmorSize &z%s (%d)\n\r", obj->value[0], obj->value[1], obj->value[2], buf, obj->value[5]);
   }
   send_to_char("&w&R-------------------------------------------------------------------------------------------\n\r", ch);
   send_to_char("&w&W        v0    v1    v2    v3    v4    v5    v6    v7    v8    v9    v10   v11   v12   v13\n\r", ch);  
   ch_printf_color(ch, "&cIndex : &w%-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d\n\r",
      obj->pIndexData->value[0], obj->pIndexData->value[1],
      obj->pIndexData->value[2], obj->pIndexData->value[3],
      obj->pIndexData->value[4], obj->pIndexData->value[5],
      obj->pIndexData->value[6], obj->pIndexData->value[7], 
      obj->pIndexData->value[8], obj->pIndexData->value[9], obj->pIndexData->value[10], 
      obj->pIndexData->value[11], obj->pIndexData->value[12], obj->pIndexData->value[13]);
   ch_printf_color(ch, "&cObject: &w%-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d %-5d\n\r",
      obj->value[0], obj->value[1], obj->value[2], obj->value[3],
      obj->value[4], obj->value[5], obj->value[6], obj->value[7], 
      obj->value[8], obj->value[9], obj->value[10], obj->value[11],
      obj->value[12], obj->value[13]);
   send_to_char("&w&R-------------------------------------------------------------------------------------------\n\r", ch);   
   if(xIS_SET(obj->extra_flags, ITEM_BLESS) || xIS_SET(obj->extra_flags, ITEM_SANCTIFIED))
   {
        if(xIS_SET(obj->extra_flags, ITEM_SANCTIFIED))
        {
            ch_printf_color(ch, "&cBless Duration: &c&w%s\n\r", "PERMANENT");
        }
        else
        {
            ch_printf_color(ch, "&cBless Duration: &c&w%d\n\r", obj->bless_dur);
        }
   }
   
   if (obj->pIndexData->first_extradesc)
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char("Primary description keywords:   '", ch);
      for (ed = obj->pIndexData->first_extradesc; ed; ed = ed->next)
      {
         send_to_char(ed->keyword, ch);
         if (ed->next)
            send_to_char(" ", ch);
      }
      send_to_char("'.\n\r", ch);
   }
   if (obj->first_extradesc)
   {
      EXTRA_DESCR_DATA *ed;

      send_to_char("Secondary description keywords: '", ch);
      for (ed = obj->first_extradesc; ed; ed = ed->next)
      {
         send_to_char(ed->keyword, ch);
         if (ed->next)
            send_to_char(" ", ch);
      }
      send_to_char("'.\n\r", ch);
   }            
   for (paf = obj->first_affect; paf; paf = paf->next)
      ch_printf(ch, "&w&c(extra) %s", showaffect(ch, paf, 1));
   for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
      ch_printf(ch, "&w&c%s", showaffect(ch, paf, 1));
   for (imbue = obj->first_imbue; imbue; imbue = imbue->next)
   {
      cnt++;
      ch_printf(ch, "&w&R%-2d> (PLevel:%-2d)\n\r", cnt, imbue->plevel);
      showgemaff(ch, NULL, 0, imbue);
   }
   if (obj->item_type == ITEM_TGEM)
      showgemaff(ch, obj, 0, NULL);
      
   return;
}

void do_moblog(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_LOG, ch);
   send_to_char("\n\r[Date_|_Time]  Current moblog:\n\r", ch);
   show_file(ch, MOBLOG_FILE);
   return;
}

int prv(int value)
{   
   if (value == -500)
      return -1;
   if (value >= -499 && value <= 0)
      return 1;
   else
      return value;
}

void do_mstat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char hpbuf[MSL];
   char mnbuf[MSL];
   char mvbuf[MSL];
   char bdbuf[MSL];
   AFFECT_DATA *paf;
   CHAR_DATA *victim;
   SKILLTYPE *skill;
   int x;

   set_pager_color(AT_CYAN, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_pager("Mstat whom?\n\r", ch);
      return;
   }
   if (arg[0] != '\'' && arg[0] != '"' && strlen(argument) > strlen(arg))
      strcpy(arg, argument);

   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_pager("They aren't here.\n\r", ch);
      return;
   }
   if (get_trust(ch) < get_trust(victim) && !IS_NPC(victim))
   {
      set_pager_color(AT_IMMORT, ch);
      send_to_pager("Their godly glow prevents you from getting a good look.\n\r", ch);
      return;
   }
   if (victim->short_descr == '\0')
      ;
   if (IS_NPC(victim))
   {
      if (victim->cident && victim->cident > 0)
         pager_printf_color(ch, "\n\r&c%s: &C%-20s &R(&G&W%d&R) &cSerial: &w&W%d", IS_NPC(victim) ? "Mobile name" : "Name", victim->name, victim->cident, victim->serial);
      else
         pager_printf_color(ch, "\n\r&c%s: &C%-20s &cSerial: &w&W%d", IS_NPC(victim) ? "Mobile name" : "Name", victim->name, victim->serial);
   }
   else
      pager_printf_color(ch, "\n\r&c%s: &C%-20s", IS_NPC(victim) ? "Mobile name" : "Name", victim->name);
   if(!IS_NPC(victim))
      pager_printf_color(ch, "\n\r&c%s: &C%-20s", "LastName", victim->last_name); 
   if (!IS_NPC(victim) && victim->pcdata->clan)
      pager_printf_color(ch, "&c%s: &c&w%s\n\r",
         victim->pcdata->clan->clan_type == CLAN_ORDER ? "Order" :
         victim->pcdata->clan->clan_type == CLAN_GUILD ? "Guild" : "Clan", victim->pcdata->clan->name);
   if (get_trust(ch) >= LEVEL_STAFF && !IS_NPC(victim) && victim->desc)
      pager_printf_color(ch, "&cUser: &c&w%s@%s   Descriptor: %d  &cTrust: &c&w%d  &cAuthBy: &c&w%s\n\r",
         victim->desc->user, victim->desc->host, victim->desc->descriptor,
         victim->trust, victim->pcdata->authed_by[0] != '\0' ? victim->pcdata->authed_by : "(unknown)");
   if (!IS_NPC(victim) && victim->pcdata->release_date != 0)
      pager_printf_color(ch, "&cHelled until %24.24s by %s.\n\r", ctime(&victim->pcdata->release_date), victim->pcdata->helled_by);
   if (IS_NPC(victim))
   {
      pager_printf_color(ch, "\n\r&cVnum: &c&w%-5d    &cSex: &c&w%-6s    &cRoom: &c&w%-5d    &cCount: &c&w%d   &cKilled: &c&w%d\n\r",
         IS_NPC(victim) ? victim->pIndexData->vnum : 0,
         victim->sex == SEX_MALE ? "male" :
         victim->sex == SEX_FEMALE ? "female" : "neutral",
         victim->in_room == NULL ? 0 : victim->in_room->vnum,
         IS_NPC(victim) ? victim->pIndexData->count : 1,
         IS_NPC(victim) ? victim->pIndexData->killed : victim->pcdata->mdeaths + victim->pcdata->pdeaths);
   }
   else
   {
      pager_printf_color(ch, "&cVnum: &c&w%-5d    &cSex: &c&w%-6s    &cRoom: &c&w%-5d    &cCount: &c&w%d   &cKilled: &c&w%d\n\r",
         IS_NPC(victim) ? victim->pIndexData->vnum : 0,
         victim->sex == SEX_MALE ? "male" :
         victim->sex == SEX_FEMALE ? "female" : "neutral",
         victim->in_room == NULL ? 0 : victim->in_room->vnum,
         IS_NPC(victim) ? victim->pIndexData->count : 1,
         IS_NPC(victim) ? victim->pIndexData->killed : victim->pcdata->mdeaths + victim->pcdata->pdeaths);
   }
   pager_printf_color(ch, "&c( Str: &C%2d&c )( Int: &C%2d&c )( Wis: &C%2d&c )( Dex: &C%2d&c )( Con: &C%2d&c )( Lck: &C%2d&c )( Agi: &C%2d&c )\n\r",
      get_curr_str(victim),
      get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim), get_curr_con(victim), get_curr_lck(victim), get_curr_agi(victim));
   if (!IS_NPC(victim))
   {
      pager_printf_color(ch, "(   %-4d  )(   %-4d  )(   %-4d  )(   %-4d  )(   %-4d  )(   %-4d  )(   %-4d  )\n\r",
         victim->pcdata->per_str, victim->pcdata->per_int, victim->pcdata->per_wis, victim->pcdata->per_dex,
         victim->pcdata->per_con, victim->pcdata->per_lck, victim->pcdata->per_agi);
   }
   if (victim->race < max_npc_race && victim->race >= 0)
      pager_printf_color(ch, "&cRace    : &c&w%-2.2d/%-10s   &cR-Arm: &c&w%-3d  &cL-Arm: &c&w%-3d  &cR-Leg: &c&w%-3d  &cL-Leg: &c&w%-3d\n\r",
         victim->race, print_npc_race(victim->race), victim->con_rarm, victim->con_larm, victim->con_rleg, victim->con_lleg);
   else
      send_to_pager("\n\r", ch);
   sprintf(hpbuf, "%d/%d", victim->hit, victim->max_hit);
   sprintf(mnbuf, "%d/%d", victim->mana, victim->max_mana);
   sprintf(mvbuf, "%d/%d", victim->move, victim->max_move);
   if (IS_VAMPIRE(victim) && !IS_NPC(victim))
   {
      sprintf(bdbuf, "%d/%d", victim->pcdata->condition[COND_BLOODTHIRST], 10 + victim->level);
      pager_printf_color(ch, "&cHps     : &c&w%-12s    &cBlood  : &c&w%-12s    &cMove      : &c&w%-12s\n\r", hpbuf, bdbuf, mvbuf);
   }
   else
      pager_printf_color(ch, "&cHps     : &c&w%-12s    &cMana   : &c&w%-12s    &cMove      : &c&w%-12s\n\r", hpbuf, mnbuf, mvbuf);
      
   if (!IS_NPC(victim))
   {
      pager_printf_color(ch, "&c          (   %-4d   )             (   %-4d   )                (   %-4d   )\n\r",
         victim->pcdata->per_hp, victim->pcdata->per_mana, victim->pcdata->per_move);
      if (!sysdata.resetgame)
      {
         pager_printf_color(ch, "&cHBurn   : &c&w%4d:%-4d       &cMBurn  :   &c&w%-d:%-4d\n\r", victim->pcdata->hit_cnt, 
            get_sore_rate(victim->race, victim->max_hit), victim->pcdata->mana_cnt, 60);
      }
      else
      {
         pager_printf_color(ch, "&cHBurn   : &c&w%4d:%-4d       &cMBurn  :   &c&w%-d:%-4d        &cTwink Pnt : &c&w%d\n\r", victim->pcdata->hit_cnt, 
            get_sore_rate(victim->race, victim->max_hit), victim->pcdata->mana_cnt, 60, victim->pcdata->twink_points);
      }
   }
   pager_printf_color(ch, "&cWimpy   : &c&w%-5d           &cPos    : &c&w%d              &cArmor     : &c&w%d\n\r",
      victim->wimpy, victim->position, victim->armor);
 
   pager_printf_color(ch, "&cFighting: &c&w%-13s   &cMaster : &c&w%-13s   &cLeader    : &c&w%s\n\r",
      victim->fighting ? victim->fighting->who->name : "(none)",
      victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)");
   if (IS_NPC(victim))
      pager_printf_color(ch, "&cHating  : &c&w%-13s   &cHunting: &c&w%-13s   &cFearing   : &c&w%s\n\r",
         victim->hating ? victim->hating->name : "(none)",
         victim->hunting ? victim->hunting->name : "(none)", victim->fearing ? victim->fearing->name : "(none)");
   else
      pager_printf_color(ch, "&cDeity   : &c&w%-13s&c&w   &cFavor  : &c&w%-5d           &cGlory     : &c&w%-d (%d)\n\r",
         victim->pcdata->deity ? victim->pcdata->deity->name : "(none)",
         victim->pcdata->favor, victim->pcdata->quest_curr, victim->pcdata->quest_accum);
   if (!IS_NPC(victim))
      pager_printf_color(ch, "&cSPoints : &w&W%-13d&c&w   &cSWorth : &w&W%-12d    &cReward    : &c&w%5d/%-5d\n\r",
         victim->pcdata->spoints, player_stat_worth(victim), victim->pcdata->reward_curr, victim->pcdata->reward_accum);
         
   pager_printf_color(ch, "&cAffectMods:  %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n\r", 
      victim->apply_armor, victim->apply_stone, victim->apply_tohit, victim->apply_sanctify, victim->apply_shield, 
      victim->apply_wmod, victim->apply_fasting, victim->apply_manafuse, victim->apply_manashell, victim->apply_manashield, 
      victim->apply_managuard, victim->apply_manaburn, victim->apply_weaponclamp, victim->apply_arrowcatch, 
      victim->apply_bracing, victim->apply_hardening, victim->hpgen, victim->managen);
   if (IS_NPC(victim))
   {
      pager_printf_color(ch, "&w&cRFire:   &w&W%3d/%-3d  &w&cRWater:   &w&W %3d/%-3d  &w&cRAir:   &w&W%3d/%-3d  &w&cREarth:  &w&W%3d/%-3d  &w&cREnergy: &w&W%3d/%-3d\n\r",
         prv(victim->apply_res_fire[0]), prv(victim->pIndexData->apply_res_fire), prv(victim->apply_res_water[0]), prv(victim->pIndexData->apply_res_water), 
         prv(victim->apply_res_air[0]), prv(victim->pIndexData->apply_res_air), prv(victim->apply_res_earth[0]), prv(victim->pIndexData->apply_res_earth), 
         prv(victim->apply_res_energy[0]), prv(victim->pIndexData->apply_res_energy));
      pager_printf_color(ch, "&w&cRMagic:  &w&W%3d/%-3d  &w&cRNonMagic: &w&W%3d/%-3d  &w&cRBlunt: &w&W%3d/%-3d  &w&cRPierce: &w&W%3d/%-3d  &w&cRSlash:  &w&W%3d/%-3d\n\r",
         prv(victim->apply_res_magic[0]), prv(victim->pIndexData->apply_res_magic), prv(victim->apply_res_nonmagic[0]), prv(victim->pIndexData->apply_res_nonmagic), 
         prv(victim->apply_res_blunt[0]), prv(victim->pIndexData->apply_res_blunt), prv(victim->apply_res_pierce[0]), prv(victim->pIndexData->apply_res_pierce), 
         prv(victim->apply_res_slash[0]), prv(victim->pIndexData->apply_res_slash));
      pager_printf_color(ch, "&w&cRPoison: &w&W%3d/%-3d  &w&cRPara:     &w&W%3d/%-3d  &w&cRHoly:  &w&W%3d/%-3d  &w&cRUnholy: &w&W%3d/%-3d  &w&cRUndead: &w&W%3d/%-3d\n\r",
         prv(victim->apply_res_poison[0]), prv(victim->pIndexData->apply_res_poison), prv(victim->apply_res_paralysis[0]), prv(victim->pIndexData->apply_res_paralysis), 
         prv(victim->apply_res_holy[0]), prv(victim->pIndexData->apply_res_holy), prv(victim->apply_res_unholy[0]), prv(victim->pIndexData->apply_res_unholy), 
         prv(victim->apply_res_undead[0]), prv(victim->pIndexData->apply_res_undead));
   }
   else
   {
      pager_printf_color(ch, "&w&cRFire:   &w&W%-3d  &w&cRWater:    &w&W%-3d  &w&cRAir:   &w&W%-3d  &w&cREarth:  &w&W%-3d  &w&cREnergy: &w&W%-3d\n\r",
         prv(victim->apply_res_fire[0]), prv(victim->apply_res_water[0]), prv(victim->apply_res_air[0]),  prv(victim->apply_res_earth[0]), 
         prv(victim->apply_res_energy[0]));
      pager_printf_color(ch, "&w&cRMagic:  &w&W%-3d  &w&cRNonMagic: &w&W%-3d  &w&cRBlunt: &w&W%-3d  &w&cRPierce: &w&W%-3d  &w&cRSlash:  &w&W%-3d\n\r",
         prv(victim->apply_res_magic[0]),  prv(victim->apply_res_nonmagic[0]), prv(victim->apply_res_blunt[0]),  prv(victim->apply_res_pierce[0]), 
         prv(victim->apply_res_slash[0]));
      pager_printf_color(ch, "&w&cRPoison: &w&W%-3d  &w&cRPara:     &w&W%-3d  &w&cRHoly:  &w&W%-3d  &w&cRUnholy: &w&W%-3d  &w&cRUndead: &w&W%-3d\n\r",
         prv(victim->apply_res_poison[0]),  prv(victim->apply_res_paralysis[0]), prv(victim->apply_res_holy[0]), prv(victim->apply_res_unholy[0]), 
         prv(victim->apply_res_undead[0]));
   }   
   if (IS_NPC(victim))
   {
      pager_printf_color(ch, "&cToHitBash : &c&w%-2d(%-2d)     &cToHitSlash : &c&w%-2d(%-2d)   &cToHitStab : &c&w%-2d(%-2d)   &cDamAdd : &c&w%d - %d (%d - %d)\n\r",
         victim->pIndexData->tohitbash, victim->tohitbash, victim->pIndexData->tohitslash, victim->tohitslash,
         victim->pIndexData->tohitstab, victim->tohitstab, victim->pIndexData->damaddlow, victim->pIndexData->damaddhi,
         victim->damaddlow, victim->damaddhi);
      pager_printf_color(ch, "&cMob Hitdie : &C%2dd%-2d+%-5d  &cDamdie index : &C%2dd%-2d+%-3d &cDamdie loaded : &C%2dd%-2d+%-3d\n\r",
         victim->pIndexData->hitnodice,
         victim->pIndexData->hitsizedice,
         victim->pIndexData->hitplus, victim->pIndexData->damnodice, victim->pIndexData->damsizedice, victim->pIndexData->damplus,
         victim->barenumdie, victim->baresizedie, victim->damplus);
      pager_printf_color(ch, "&cm1: &w&C%-10d &w&p(&w&C%-10d&w&p) &cm2: &w&C%-10d &w&p(&w&C%-10d&w&p) &cm3: &w&C%-10d &w&p(&w&C%-10d&w&p)\n\r",
         victim->pIndexData->m1, victim->m1, victim->pIndexData->m2, victim->m2,victim->pIndexData->m3, victim->m3);
      pager_printf_color(ch, "&cm4: &w&C%-10d &w&p(&w&C%-10d&w&p) &cm5: &w&C%-10d &w&p(&w&C%-10d&w&p) &cm6: &w&C%-10d &w&p(&w&C%-10d&w&p)\n\r",
         victim->pIndexData->m4, victim->m4, victim->pIndexData->m5, victim->m5,victim->pIndexData->m6, victim->m6);
      pager_printf_color(ch, "&cm7: &w&C%-10d &w&p(&w&C%-10d&w&p) &cm8: &w&C%-10d &w&p(&w&C%-10d&w&p) &cm9: &w&C%-10d &w&p(&w&C%-10d&w&p)\n\r",
         victim->pIndexData->m7, victim->m7, victim->pIndexData->m8, victim->m8,victim->pIndexData->m9, victim->m9);
      pager_printf_color(ch, "&cm10:&w&C%-10d &w&p(&w&C%-10d&w&p) &cm11:&w&C%-10d &w&p(&w&C%-10d&w&p) &cm12:&w&C%-10d &w&p(&w&C%-10d&w&p)\n\r",
         victim->pIndexData->m10, victim->m10, victim->pIndexData->m11, victim->m11 ,victim->pIndexData->m12, victim->m12);
   }
   pager_printf_color(ch, "&cMentalState: &c&w%-3d          &cEmotionalState: &c&w%-3d   ", victim->mental_state, victim->emotional_state);
   if (!IS_NPC(victim))
      pager_printf_color(ch, "&cThirst: &c&w%d   &cFull: &c&w%d   &cDrunk: &c&w%d\n\r",
         victim->pcdata->condition[COND_THIRST], victim->pcdata->condition[COND_FULL], victim->pcdata->condition[COND_DRUNK]);
   else
      send_to_pager("\n\r", ch);
   pager_printf_color(ch, "&cSave versus: &c&w%d %d %d %d %d       &cItems: &c&w(%d/%d)  &cWeight &c&w(%.2f/%d)  &cEndurance  &c&w(%d)\n\r",
      victim->saving_poison_death,
      victim->saving_wand,
      victim->saving_para_petri,
      victim->saving_breath, victim->saving_spell_staff, get_ch_carry_number(victim), can_carry_n(victim), get_ch_carry_weight(victim), can_carry_w(victim),
      victim->mover);
   pager_printf_color(ch, "&cYear: &c&w%-5d  &cSecs: &c&w%d  &cTimer: &c&w%d  &cGold: &Y%d",
      get_age(victim), (int) victim->played, victim->timer, victim->gold);

   if (IS_NPC(victim))
      pager_printf_color(ch, "&p(&Y%d&p)\n\r", victim->pIndexData->gold);
   else
      pager_printf_color(ch, "\n\r", ch);

   if (get_timer(victim, TIMER_PKILLED))
      pager_printf_color(ch, "&cTimerPkilled:  &R%d\n\r", get_timer(victim, TIMER_PKILLED));
   if (get_timer(victim, TIMER_RECENTFIGHT))
      pager_printf_color(ch, "&cTimerRecentfight:  &R%d\n\r", get_timer(victim, TIMER_RECENTFIGHT));
   if (get_timer(victim, TIMER_ASUPRESSED))
      pager_printf_color(ch, "&cTimerAsupressed:  &R%d\n\r", get_timer(victim, TIMER_ASUPRESSED));
   if (get_timer(victim, TIMER_SHOVEDRAG))
      pager_printf_color(ch, "&cTimerShovedrag:  &R%d\n\r", get_timer(victim, TIMER_SHOVEDRAG));
   pager_printf_color(ch, "&cElement    : &B%s\n\r", flag_string(victim->elementb, element_flags));
   if (IS_NPC(victim))
   {
      pager_printf_color(ch, "&cAct Flags  : &c&w%s\n\r", ext_flag_string(&victim->act, act_flags));
      if (xIS_SET(victim->act, ACT_MILITARY))
      {
         pager_printf_color(ch, "&cMil Flags  : &c&w%s\n\r", ext_flag_string(&victim->miflags, mi_flags));
      }
      pager_printf_color(ch, "&RMap        : &c%s \n\r&w&CCoords     : &w%d %d\n\r",
         IS_ONMAP_FLAG(victim) ? map_names[victim->map] : "none", victim->coord->x, victim->coord->y);
   }
   else
   {
      pager_printf_color(ch, "&cPlayerFlags: &c&w%s\n\r", ext_flag_string(&victim->act, plr_flags));
      pager_printf_color(ch, "&cTalents    : &c&w%s\n\r", ext_flag_string(&victim->pcdata->talent, talent_flags));
      pager_printf_color(ch, "&cPcflags    : &c&w%s\n\r", flag_string(victim->pcdata->flags, pc_flags));
      if (victim->pcdata->nuisance)
      {
         pager_printf_color(ch, "&RNuisance   &cStage: (&R%d&c/%d)  Power:  &c&w%d  &cTime:  &c&w%s.\n\r", victim->pcdata->nuisance->flags,
            MAX_NUISANCE_STAGE, victim->pcdata->nuisance->power, ctime(&victim->pcdata->nuisance->time));
      }
      pager_printf_color(ch, "&cSkinColor : &c&w%-2d  &cHaircolor : &c&w%-2d   &cHairlength : &c&w%-2d   &cHairstyle: &c&w%-2d\n\r", 
         ch->pcdata->skincolor, ch->pcdata->haircolor, ch->pcdata->hairlength, ch->pcdata->hairstyle);
      pager_printf_color(ch, "&cEyecolor  : &c&w%-2d  &cCHeight   : &c&w%-2d   &cCWeight    : &c&w%-2d\n\r",
         ch->pcdata->eyecolor, ch->pcdata->cheight, ch->pcdata->cweight);
      pager_printf_color(ch, "&RMap        : &c%s \n\r&w&CCoords     : &w%d %d\n\r",
         IS_ONMAP_FLAG(victim) ? map_names[victim->map] : "none", victim->coord->x, victim->coord->y);
   }
   if (!IS_NPC(victim))
   {
      pager_printf_color(ch, "&RPkills: &c&w%d   &RPdeaths: &c&w%d   &RPranking: &c&w%d\n\r",
         victim->pcdata->pkills, victim->pcdata->pdeaths, victim->pcdata->pranking);
   }
   if (victim->morph)
   {
      if (victim->morph->morph)
         pager_printf_color(ch, "&cMorphed as : (&C%d&c) &C%s    &cTimer: &C%d\n\r",
            victim->morph->morph->vnum, victim->morph->morph->short_desc, victim->morph->timer);
      else
         pager_printf_color(ch, "&cMorphed as: Morph was deleted.\n\r");
   }
   pager_printf_color(ch, "&cAffected by: &C%s\n\r", affect_bit_name(&victim->affected_by));
   pager_printf_color(ch, "&cSpeaks: &c&w%d   &cSpeaking: &c&w%d", victim->speaks, victim->speaking);
   if (!IS_NPC(victim) && victim->wait)
      pager_printf_color(ch, "   &cWaitState: &R%d\n\r", victim->wait / 12);
   else
      send_to_pager("\n\r", ch);
   send_to_pager_color("&cLanguages  : &c&w", ch);
   for (x = 0; lang_array[x] != LANG_UNKNOWN; x++)
      if (knows_language(victim, lang_array[x], victim) || (IS_NPC(victim) && victim->speaks == 0))
      {
         if (IS_SET(lang_array[x], victim->speaking) || (IS_NPC(victim) && !victim->speaking))
            set_pager_color(AT_RED, ch);
         send_to_pager(lang_names[x], ch);
         send_to_pager(" ", ch);
         set_pager_color(AT_PLAIN, ch);
      }
      else if (IS_SET(lang_array[x], victim->speaking) || (IS_NPC(victim) && !victim->speaking))
      {
         set_pager_color(AT_PINK, ch);
         send_to_pager(lang_names[x], ch);
         send_to_pager(" ", ch);
         set_pager_color(AT_PLAIN, ch);
      }
   send_to_pager("\n\r", ch);
/* For flevel, Caste, Balance, Hometown - Xerves */
   if (!IS_NPC(victim))
   {
      pager_printf_color(ch, "&cFLevel: &c&w%d  &cCaste: &c&w%d  &cBalance: &Y%d  &cKingdom: &c&w%d/%-12s  &cTown: &c&w%-15s\n\r",
         victim->pcdata->flevel, victim->pcdata->caste, victim->pcdata->balance, victim->pcdata->hometown,
         kingdom_table[victim->pcdata->hometown]->name, victim->pcdata->town ? victim->pcdata->town->name : "None");
      pager_printf_color(ch, "&cResource: &c&w%d  &cResourcetype: &c&w%d\n\r", victim->pcdata->resource, victim->pcdata->resourcetype);
   }
   if (victim->pcdata && victim->pcdata->bestowments && victim->pcdata->bestowments[0] != '\0')
      pager_printf_color(ch, "&cBestowments: &c&w%s\n\r", victim->pcdata->bestowments);
   if (IS_NPC(victim))
      pager_printf_color(ch, "&cShortdesc  : &c&w%s\n\r&cLongdesc   : &c&w%s",
         victim->short_descr[0] != '\0' ? victim->short_descr : "(none set)", victim->long_descr[0] != '\0' ? victim->long_descr : "(none set)\n\r");
   else
   {
      if (victim->short_descr[0] != '\0')
         pager_printf_color(ch, "&cShortdesc  : &c&w%s\n\r", victim->short_descr);
      if (victim->long_descr[0] != '\0')
         pager_printf_color(ch, "&cLongdesc   : &c&w%s\n\r", victim->long_descr);
   }
   if (IS_NPC(victim) && victim->spec_fun)
      pager_printf_color(ch, "&cMobile has spec fun: &c&w%s\n\r", lookup_spec(victim->spec_fun));
   if (IS_NPC(victim))
      pager_printf_color(ch, "&cBody Parts : &c&w%s\n\r", flag_string(victim->xflags, part_flags));
   if (victim->resistant > 0)
      pager_printf_color(ch, "&cResistant  : &c&w%s\n\r", flag_string(victim->resistant, ris_flags));
   if (victim->immune > 0)
      pager_printf_color(ch, "&cImmune     : &c&w%s\n\r", flag_string(victim->immune, ris_flags));
   if (victim->susceptible > 0)
      pager_printf_color(ch, "&cSusceptible: &c&w%s\n\r", flag_string(victim->susceptible, ris_flags));
   if (IS_NPC(victim))
   {
      pager_printf_color(ch, "&cAttacks    : &c&w%s\n\r", ext_flag_string(&victim->attacks, attack_flags));
      pager_printf_color(ch, "&cDefenses   : &c&w%s\n\r", ext_flag_string(&victim->defenses, defense_flags));
   }
   if(!IS_NPC(victim))
   {
	pager_printf_color(ch, "\n\r&G**** Fame: &B%d\n\r", victim->fame);
   }
   for (paf = victim->first_affect; paf; paf = paf->next)
      if ((skill = get_skilltype(paf->type)) != NULL)
         pager_printf_color(ch,
            "&c%s: &c&w'%s' mods %s by %d for %d rnds with bits %s.\n\r",
            skill_tname[skill->type], skill->name, affect_loc_name(paf->location), paf->modifier, paf->duration, affect_bit_name(&paf->bitvector));
   return;
}

void do_mfind(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   MOB_INDEX_DATA *pMobIndex;
   int hash;
   int nMatch;
   bool fAll;

   set_pager_color(AT_PLAIN, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Mfind whom?\n\r", ch);
      return;
   }

   fAll = !str_cmp(arg, "all");
   nMatch = 0;

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_mob_index()... which loops itself, an average of 1-2 times...
    * So theoretically, the above routine may loop well over 40,000 times,
    * and my routine bellow will loop for as many index_mobiles are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for (hash = 0; hash < MAX_KEY_HASH; hash++)
      for (pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next)
         if (fAll || nifty_is_name(arg, pMobIndex->player_name))
         {
            nMatch++;
            pager_printf(ch, "[%5d] %s\n\r", pMobIndex->vnum, capitalize(pMobIndex->short_descr));
         }

   if (nMatch)
      pager_printf(ch, "Number of matches: %d\n", nMatch);
   else
      send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
   return;
}

void do_ofind(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_INDEX_DATA *pObjIndex;
   int hash;
   int nMatch;
   bool fAll;

   set_pager_color(AT_PLAIN, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Ofind what?\n\r", ch);
      return;
   }

   fAll = !str_cmp(arg, "all");
   nMatch = 0;

   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_obj_index()... which loops itself, an average of 2-3 times...
    * So theoretically, the above routine may loop well over 50,000 times,
    * and my routine bellow will loop for as many index_objects are on
    * your mud... likely under 3000 times.
    * -Thoric
    */
   for (hash = 0; hash < MAX_KEY_HASH; hash++)
      for (pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next)
         if (fAll || nifty_is_name(arg, pObjIndex->name))
         {
            nMatch++;
            pager_printf(ch, "[%5d] %s\n\r", pObjIndex->vnum, capitalize(pObjIndex->short_descr));
         }

   if (nMatch)
      pager_printf(ch, "Number of matches: %d\n", nMatch);
   else
      send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
   return;
}

/* Added support for finding trainers in mwhere......useful and easy to do for me */
void do_mwhere(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   bool found;

   set_pager_color(AT_PLAIN, ch);

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Mwhere whom?\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "trainers"))
   {
      for (victim = first_char; victim; victim = victim->next)
      {
         if (IS_NPC(victim) && victim->in_room && xIS_SET(victim->act, ACT_TRAINER))
         {
            pager_printf(ch, "[%5d] %-28s [%5d] %s\n\r",
               victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->area->name);
         }
      }
      return;
   }
   found = FALSE;
   for (victim = first_char; victim; victim = victim->next)
   {
      if (IS_NPC(victim) && victim->in_room && nifty_is_name(arg, victim->name))
      {
         found = TRUE;
         pager_printf(ch, "[%5d] %-28s [%5d] %s\n\r", victim->pIndexData->vnum, victim->short_descr, victim->in_room->vnum, victim->in_room->name);
      }
   }

   if (!found)
      act(AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR);
   return;
}

TRAP_DATA *copy_trap(TRAP_DATA *otrap, OBJ_DATA *obj)
{
   TRAP_DATA *ntrap;
   
   CREATE(ntrap, TRAP_DATA, 1);
   ntrap->maxcharges = otrap->maxcharges;
   ntrap->charges = otrap->charges;
   ntrap->type = otrap->type;
   ntrap->damhigh = otrap->damhigh;
   ntrap->damlow = otrap->damlow;
   ntrap->room = otrap->room;
   ntrap->difficulty = otrap->difficulty;
   ntrap->toolkit = otrap->toolkit;
   ntrap->onetime = otrap->onetime;
   ntrap->resetvalue = otrap->resetvalue;
   ntrap->toolnegate = otrap->toolnegate;
   ntrap->frag = otrap->frag;
   ntrap->trapflags = otrap->trapflags;
   ntrap->obj = obj;
   ntrap->uid = ++sysdata.last_invtrap_uid;
   obj->trap = ntrap;
   otrap->obj = NULL;
   otrap->area = NULL;
   LINK(ntrap, first_trap, last_trap, next, prev);
   save_trap_file(NULL, NULL);
   save_sysdata(sysdata);
   return ntrap;
}

int get_trapflag args((char *flag));
RESET_DATA *find_oreset args((CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, char *name));

   
void do_trap(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int charges, type, damlow, damhi, room, difficulty, toolkit, onetime, resetvalue, toolnegate, frag, flags;
   EXT_BV trapflags;
   TRAP_DATA *trap;
   RESET_DATA *reset;
   RESET_DATA *pReset;
   AREA_DATA *area;
   
   reset = NULL;
   obj = NULL;
   area = NULL;
   xCLEAR_BITS(trapflags);
   
   if (check_npc(ch))
      return;
   if (argument[0] == '\0')
   {
      send_to_char("&w&WTrap set <obj> <charges> <type> <damlow> <damhi> <room> <difficulty> <toolkit> <onetime> <resetvalue> <toolnegate> <frag> <flags>\n\r", ch);
      send_to_char("Trap set simple <obj> <charges> <type> <damlow> <damhi> <room> <flags>\n\r", ch);
      send_to_char("Trap create\n\r", ch);
      send_to_char("Trap show <uid/area name/all/reset/inv/loaded/notloaded/frag>\n\r", ch);
      send_to_char("Trap apply <obj name> <uid>\n\r", ch);
      send_to_char("Trap add <charges> <type> <dam low> <dam hi> <room> <difficulty> <toolkit> <onetime> <reset value> <toolnegate> <frag> <flags>\n\r", ch);
      send_to_char("Trap delete <uid>\n\r", ch);
      send_to_char("Trap edit <uid> <trap option> <value>\n\r\n\r", ch);
      send_to_char("   Trap Option - charges, maxcharges, type, damlow, damhi, room, difficulty, toolkit,\n\r", ch); 
      send_to_char("                 onetime, resetvalue, toolnegate, frag, flags\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   
   if (!str_cmp(arg, "edit"))
   {
      argument = one_argument(argument, arg);
      if (atoi(arg) <= 0)
      {
         send_to_char("UID has to be greater than 0.\n\r", ch);
         return;
      }
      for (trap = first_trap; trap; trap = trap->next)
      {
         if (trap->uid == atoi(arg))
            break;
      }
      if (!trap)
      {
         send_to_char("Could not locate a trap with that uid.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg);
      if (!str_cmp(arg, "charges")) 
      {
         argument = one_argument(argument, arg);
         if ((charges = atoi(arg)) <= 0)
         {
            send_to_char("Range is 1 or greater.\n\r", ch);
            return;
         }
         if (charges > trap->maxcharges)
         {
            send_to_char("Charges must be less than MaxCharges.\n\r", ch);
            return;
         }
         trap->charges = charges;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "maxcharges")) 
      {
         argument = one_argument(argument, arg);
         if ((charges = atoi(arg)) <= 0)
         {
            send_to_char("Range is 1 or greater.\n\r", ch);
            return;
         }
         trap->maxcharges = charges;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "type")) 
      {
         argument = one_argument(argument, arg);
         if ((type = atoi(arg)) <= 0 || type > MAX_TRAPTYPE)
         {
            ch_printf(ch, "Range is 1 to %d\n\r", MAX_TRAPTYPE);
            return;
         }
         trap->type = type;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "damlow"))
      {
         argument = one_argument(argument, arg);
         if ((damlow = atoi(arg)) < 0)
         {
            send_to_char("Minimum damage must be greater than 0.\n\r", ch);
            return;
         }
         trap->damlow = damlow;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      } 
      if (!str_cmp(arg, "damhi")) 
      {
         argument = one_argument(argument, arg);
         if ((damhi = atoi(arg)) < trap->damlow)
         {
            send_to_char("Maximum damage must be greater than the minimum damage.\n\r", ch);
            return;
         }
         trap->damhigh = damhi;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "room")) 
      {
         argument = one_argument(argument, arg);
         if ((room = atoi(arg)) < 0 || room > 1)
         {
            send_to_char("Room Value is 0 (Not a Room attack) to 1 (A room attack).\n\r", ch);
            return;
         }
         trap->room = room;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "difficulty")) 
      {
         argument = one_argument(argument, arg);
         if ((difficulty = atoi(arg)) < 0 || difficulty > 300)
         {
            send_to_char("Difficulty range is 0 to 300.\n\r", ch);
            return;
         }
         trap->difficulty = difficulty;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "toolkit"))
      {
         argument = one_argument(argument, arg);
         if ((toolkit = atoi(arg)) < 0)
         {
            send_to_char("Toolkit value must not be less than 0.\n\r", ch);
            return;
         }
         trap->toolkit = toolkit;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      } 
      if (!str_cmp(arg, "onetime")) 
      {
         argument = one_argument(argument, arg);
         if ((onetime = atoi(arg)) < 0 || onetime > 1)
         {
            send_to_char("One Time Value is 0 (Do Not Delete after Use) to 1 (Delete after Use).\n\r", ch);
            return;
         }
         trap->onetime = onetime;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "resetvalue")) 
      {
         argument = one_argument(argument, arg);
         if ((resetvalue = atoi(arg)) < 0)
         {
            send_to_char("Range is 0 (Reset with the obj), or 1 (reset everytime the area resets).\n\r", ch);
            return;
         }
         trap->resetvalue = resetvalue;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "toolnegate")) 
      {
         argument = one_argument(argument, arg);
         if ((toolnegate = atoi(arg)) < 0 || toolnegate > 300)
         {
            send_to_char("Range is 0 to 300.\n\r", ch);
            return;
         }
         trap->toolnegate = toolnegate;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      if (!str_cmp(arg, "frag"))
      {
         argument = one_argument(argument, arg);
         if ((frag = atoi(arg)) < 0 || frag > 100)
         {
            send_to_char("Range is 0 (no frag) to 100 (100 percent chance to frag the trapped object).\n\r", ch);
            return;
         }
         trap->frag = frag;
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      } 
      if (!str_cmp(arg, "flags")) 
      {
         while (argument[0] != '\0')
         {
            argument = one_argument(argument, arg);
            flags = get_trapflag(arg);
            if (flags >= 0 && flags < MAX_BITS)
               xTOGGLE_BIT(trap->trapflags, flags);
            else
            {
               send_to_char("TRAP: bad flag\n\r", ch);
               return;
            }
         }
         send_to_char("Done.\n\r", ch);
         save_trap_file(NULL, NULL);
         return;
      }
      do_trap(ch, "");
      return;
   }         
   if (!str_cmp(arg, "show"))
   {
      argument = one_argument(argument, arg);
      if (!str_cmp("all", arg))
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
               trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
               trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
               ext_flag_string(&trap->trapflags, trap_flags));
         }
         return;
      }
      if (!str_cmp("loaded", arg))
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            if (trap->obj)
            {
               ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
                  trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
                  trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
                  ext_flag_string(&trap->trapflags, trap_flags));
            }
         }
         return;
      }
      if (!str_cmp("reset", arg))
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            if (trap->uid < START_INV_TRAP)
            {
               ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
                  trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
                  trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
                  ext_flag_string(&trap->trapflags, trap_flags));
            }
         }
         return;
      }
      if (!str_cmp("reset", arg))
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            if (trap->uid < START_INV_TRAP)
            {
               ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
                  trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
                  trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
                  ext_flag_string(&trap->trapflags, trap_flags));
            }
         }
         return;
      }
      if (!str_cmp("inv", arg))
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            if (trap->uid >= START_INV_TRAP)
            {
               ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
                  trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
                  trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
                  ext_flag_string(&trap->trapflags, trap_flags));
            }
         }
         return;
      }
      if (!str_cmp("frag", arg))
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            if (trap->frag)
            {
               ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
                  trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
                  trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
                  ext_flag_string(&trap->trapflags, trap_flags));
            }
         }
         return;
      }
      if (atoi(arg) > 0)
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            if (trap->uid == atoi(arg))
            {
               ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
                  trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
                  trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
                  ext_flag_string(&trap->trapflags, trap_flags));
               return;
            }
         }
      }
      for (area = first_area; area; area = area->next)
      {
         if (!str_cmp(arg, area->name) || !str_cmp(arg, area->filename))
            break;
      }
      if (!area)
      {
         do_trap(ch, "");
         return;
      }
      else
      {
         send_to_char("Uid        Charges  Type Damlo Damhi Room Diff Tool  One Reset TNeg Frag Load   Flags\n\r", ch);
         for (trap = first_trap; trap; trap = trap->next)
         {
            if (trap->area && trap->area == area)
            {
               ch_printf(ch, "%-8d>  %-3d/%-3d  %-2d   %-4d  %-4d  %-3d  %-3d  %-4d  %-2d  %-4d  %-3d  %-3d  %-6d %s\n\r", trap->uid, 
                  trap->charges, trap->maxcharges, trap->type, trap->damlow, trap->damhigh, trap->room, trap->difficulty, trap->toolkit, 
                  trap->onetime, trap->resetvalue, trap->toolnegate, trap->frag, trap->obj ? trap->obj->pIndexData->vnum : 0,
                  ext_flag_string(&trap->trapflags, trap_flags));
            }
         }
         return;
      }
   }
   if (!str_cmp(arg, "create"))
   {
      CREATE(trap, TRAP_DATA, 1);
      trap->maxcharges = 0;
      trap->charges = 0;
      trap->type = 0;
      trap->damhigh = 0;
      trap->damlow = 0;
      trap->room = 0;
      trap->difficulty = 0;
      trap->toolkit = 0;
      trap->onetime = 0;
      trap->resetvalue = 0;
      trap->toolnegate = 0;
      trap->frag = 0;
      xCLEAR_BITS(trap->trapflags);
      trap->obj = NULL;
      trap->uid = ++sysdata.last_trap_uid;
      LINK(trap, first_trap, last_trap, next, prev);
      save_trap_file(NULL, NULL);
      save_sysdata(sysdata);
      send_to_char("An empty trap was created for you.\n\r", ch);
      return;   
   }
   if (!str_cmp(arg, "apply"))
   {
      argument = one_argument(argument, arg);
      if ((obj = get_obj_here(ch, arg)) == NULL)
      {
         send_to_char("Could not find the object you specified.  \n\rMust provide the item name and it must be on you or in the room.\n\r", ch);
         return;
      }
      if (!obj->carried_by)
      {
         if (get_trust(ch) < MAX_LEVEL && (!ch->pcdata || !(area = ch->pcdata->area)))
         {
            send_to_char("You must have an assigned area to apply a trap.\n\r", ch);
            return;
         }
         if (get_trust(ch) < MAX_LEVEL)
         {
            if (obj->pIndexData->vnum < area->low_r_vnum || obj->pIndexData->vnum > area->hi_r_vnum)
            {
               send_to_char("The object you are trying to apply a trap to is not in your assigned area.\n\r", ch);
               return;
            }
         }
      }
      argument = one_argument(argument, arg);
      for (trap = first_trap; trap; trap = trap->next)
      {
         if (trap->uid == atoi(arg))
            break;
      }
      if (!trap)
      {
         send_to_char("There is now trap with the uid you specified.\n\r", ch);
         return;
      }
      if (!obj->carried_by && !(reset = find_oreset(ch, ch->in_room->area, ch->in_room, obj->name)))
      {
         send_to_char("Could not find the object you are trying to trap in the reset list.\n\r", ch);
         return;
      }
      if (obj->trap)
      {
         send_to_char("A trap is already armed on this obj.\n\r", ch);
         return;
      }
      if (trap->obj)
      {
         send_to_char("The trap already has an object attached to it.\n\r", ch);
         return;
      }
      if (!obj->carried_by)
      {
         pReset = make_reset('A', 1, trap->uid, 0, 0, -1, -1, -1, -1, 0, 0);
         INSERT(pReset, reset, ch->in_room->area->last_reset, prev, next);
         trap->area = ch->in_room->area;
      }
      else
      {
         trap->uid = ++sysdata.last_invtrap_uid;
      }
      obj->trap = trap;
      trap->obj = obj;
      LINK(trap, first_trap, last_trap, next, prev);
      save_trap_file(NULL, NULL);
      send_to_char("Trap is now applied to the object.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "delete"))
   {
      RESET_DATA *pReset;
      int rfound = 0;
      
      argument = one_argument(argument, arg);
      for (trap = first_trap; trap; trap = trap->next)
      {
         if (trap->uid == atoi(arg))
            break;
      }
      if (!trap)
      {
         send_to_char("The Uid you specified could not be found.\n\r", ch);
         return;
      }
      if (!trap->area && get_trust(ch) < MAX_LEVEL)
      {
         send_to_char("Only the admin can remove traps not belonging to an area.\n\r", ch);
         return;
      }
      if (trap->area && get_trust(ch) < MAX_LEVEL)
      {
         area = ch->pcdata->area;
         if (get_trust(ch) < MAX_LEVEL && (!ch->pcdata || !area))
         {
            send_to_char("You must have an assigned area to delete a trap.\n\r", ch);
            return;
         }
         if (get_trust(ch) < MAX_LEVEL)
         {
            if (obj->pIndexData->vnum < area->low_r_vnum || obj->pIndexData->vnum > area->hi_r_vnum)
            {
               send_to_char("The trap you are trying to delete is not on an object in your assigned area.\n\r", ch);
               return;
            }
         }
      }
      if (trap->area)
      {
         for (pReset = trap->area->first_reset; pReset; pReset = pReset->next)
         {
            if (pReset->command == 'A' && pReset->arg1 == trap->uid)
            {
               delete_reset(trap->area, pReset);
               rfound = 1;
               break;
            }
         }
         if (!rfound)
         {
            send_to_char("Could not find the reset in the area that the trag is flagged in.\n\r", ch);
            return;
         }
      }
      if (trap->obj)
      {
         trap->obj->trap = NULL;
         trap->obj = NULL;
      }      
      UNLINK(trap, first_trap, last_trap, next, prev);
      DISPOSE(trap);
      send_to_char("Trap was removed, please remember to save/fold the area if it was reset on an object.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "set") || !str_cmp(arg, "add"))
   {
      int add = 0;
      
      if (!str_cmp(arg, "add"))
         add = 1;
      argument = one_argument(argument, arg);
      if (!str_cmp("simple", arg))
      {
         if ((obj = get_obj_here(ch, arg)) == NULL)
         {
            send_to_char("Could not find the object you specified.  \n\rMust provide the item name and it must be on you or in the room.\n\r", ch);
            return;
         }
         if (!obj->carried_by)
         {
            area = ch->pcdata->area;
            if (get_trust(ch) < MAX_LEVEL && (!ch->pcdata || !area))
            {
               send_to_char("You must have an assigned area to apply a trap.\n\r", ch);
               return;
            }
            if (get_trust(ch) < MAX_LEVEL)
            {
               if (obj->pIndexData->vnum < area->low_r_vnum || obj->pIndexData->vnum > area->hi_r_vnum)
               {
                  send_to_char("The object you are trying to apply a trap to is not in your assigned area.\n\r", ch);
                  return;
               }
            }
         }
         argument = one_argument(argument, arg);
         if ((charges = atoi(arg)) <= 0)
         {
            send_to_char("Range is 1 or greater.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((type = atoi(arg)) <= 0 || type > MAX_TRAPTYPE)
         {
            ch_printf(ch, "Range is 1 to %d\n\r", MAX_TRAPTYPE);
            return;
         }
         argument = one_argument(argument, arg);
         if ((damlow = atoi(arg)) < 0)
         {
            send_to_char("Minimum damage must be greater than 0.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((damhi = atoi(arg)) < damlow)
         {
            send_to_char("Maximum damage must be greater than the minimum damage.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((room = atoi(arg)) < 0 || room > 1)
         {
            send_to_char("Room Value is 0 (Not a Room attack) to 1 (A room attack).\n\r", ch);
            return;
         }         
         while (argument[0] != '\0')
         {
            argument = one_argument(argument, arg);
            flags = get_trapflag(arg);
            if (flags >= 0 && flags < MAX_BITS)
               xSET_BIT(trapflags, flags);
            else
            {
               send_to_char("TRAP: bad flag\n\r", ch);
               return;
            }
         }
         if (!obj->carried_by && !(reset = find_oreset(ch, ch->in_room->area, ch->in_room, obj->name)))
         {
            send_to_char("Could not find the object you are trying to trap in the reset list.\n\r", ch);
            return;
         }
         if (obj->trap)
         {
            send_to_char("A trap is already armed on this obj.\n\r", ch);
            return;
         }
         CREATE(trap, TRAP_DATA, 1);
         trap->maxcharges = charges;
         trap->charges = charges;
         trap->type = type;
         trap->damhigh = damhi;
         trap->damlow = damlow;
         trap->room = room;
         trap->difficulty = 0;
         trap->toolkit = 0;
         trap->onetime = 0;
         trap->resetvalue = 0;
         trap->toolnegate = 0;
         trap->frag = 0;
         trap->trapflags = trapflags;
         trap->uid = ++sysdata.last_trap_uid;
         if (!obj->carried_by)
         {
            trap->uid = ++sysdata.last_trap_uid;
            pReset = make_reset('A', 1, trap->uid, 0, 0, -1, -1, -1, -1, 0, 0);
            INSERT(pReset, reset, ch->in_room->area->last_reset, prev, next);
            trap->area = ch->in_room->area;
         }
         else
         {
            trap->uid = ++sysdata.last_invtrap_uid;
            trap->area = NULL;
         }
         LINK(trap, first_trap, last_trap, next, prev);
         obj->trap = trap;
         trap->obj = obj;
         save_trap_file(NULL, NULL);
         save_sysdata(sysdata);
         send_to_char("Trap created and reset to that object.  Please save/fold the area.\n\r", ch);
         return;
      }
      else
      {
         if (!add)
         {
            if ((obj = get_obj_here(ch, arg)) == NULL)
            {
               send_to_char("Could not find the object you specified.  \n\rMust provide the item name and it must be on you or in the room.\n\r", ch);
               return;
            }
            if (!obj->carried_by)
            {
               area = ch->pcdata->area;
               if (get_trust(ch) < MAX_LEVEL && (!ch->pcdata || !area))
               {
                  send_to_char("You must have an assigned area to apply a trap.\n\r", ch);
                  return;
               }
               if (get_trust(ch) < MAX_LEVEL)
               {
                  if (obj->pIndexData->vnum < area->low_r_vnum || obj->pIndexData->vnum > area->hi_r_vnum)
                  {
                     send_to_char("The object you are trying to apply a trap to is not in your assigned area.\n\r", ch);
                     return;
                  }
               }
            }
            argument = one_argument(argument, arg);
         }
         if ((charges = atoi(arg)) <= 0)
         {
            send_to_char("Range is 1 or greater.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((type = atoi(arg)) <= 0 || type > MAX_TRAPTYPE)
         {
            ch_printf(ch, "Range is 1 to %d\n\r", MAX_TRAPTYPE);
            return;
         }
         argument = one_argument(argument, arg);
         if ((damlow = atoi(arg)) < 0)
         {
            send_to_char("Minimum damage must be greater than 0.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((damhi = atoi(arg)) < damlow)
         {
            send_to_char("Maximum damage must be greater than the minimum damage.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((room = atoi(arg)) < 0 || room > 1)
         {
            send_to_char("Room Value is 0 (Not a Room attack) to 1 (A room attack).\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((difficulty = atoi(arg)) < 0 || difficulty > 300)
         {
            send_to_char("Difficulty range is 0 to 300.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((toolkit = atoi(arg)) < 0)
         {
            send_to_char("Toolkit value must not be less than 0.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((onetime = atoi(arg)) < 0 || onetime > 1)
         {
            send_to_char("One Time Value is 0 (Do Not Delete after Use) to 1 (Delete after Use).\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((resetvalue = atoi(arg)) < 0)
         {
            send_to_char("Range is 0 (Reset with the obj), or 1 (reset everytime the area resets).\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((toolnegate = atoi(arg)) < 0 || toolnegate > 300)
         {
            send_to_char("Range is 0 to 300.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if ((frag = atoi(arg)) < 0 || frag > 100)
         {
            send_to_char("Range is 0 (no frag) to 100 (100 percent chance to frag the trapped object).\n\r", ch);
            return;
         }
         while (argument[0] != '\0')
         {
            argument = one_argument(argument, arg);
            flags = get_trapflag(arg);
            if (flags >= 0 && flags < MAX_BITS)
               xSET_BIT(trapflags, flags);
            else
            {
               send_to_char("TRAP: bad flag\n\r", ch);
               return;
            }
         }
         if (!add && !obj->carried_by && !(reset = find_oreset(ch, ch->in_room->area, ch->in_room, obj->name)))
         {
            send_to_char("Could not find the object you are trying to trap in the reset list.\n\r", ch);
            return;
         }
         if (!add && obj->trap)
         {
            send_to_char("A trap is already armed on this obj.\n\r", ch);
            return;
         }
         CREATE(trap, TRAP_DATA, 1);
         trap->maxcharges = charges;
         trap->charges = charges;
         trap->type = type;
         trap->damhigh = damhi;
         trap->damlow = damlow;
         trap->room = room;
         trap->difficulty = difficulty;
         trap->toolkit = toolkit;
         trap->onetime = onetime;
         trap->resetvalue = resetvalue;
         trap->toolnegate = toolnegate;
         trap->frag = 0;
         trap->trapflags = trapflags;
         trap->uid = ++sysdata.last_trap_uid;
         if (!obj->carried_by)
         {
            trap->uid = ++sysdata.last_trap_uid;
            pReset = make_reset('A', 1, trap->uid, 0, 0, -1, -1, -1, -1, 0, 0);
            INSERT(pReset, reset, ch->in_room->area->last_reset, prev, next);
            trap->area = ch->in_room->area;
         }
         else
         {
            trap->uid = ++sysdata.last_invtrap_uid;
            trap->area = NULL;
         }
         LINK(trap, first_trap, last_trap, next, prev);
         obj->trap = trap;
         trap->obj = obj;
         save_trap_file(NULL, NULL);
         save_sysdata(sysdata);
         if (add)
            send_to_char("Trap created.  Use apply to apply it to an object.\n\r", ch);
         else
            send_to_char("Trap created and reset to that object.  Please save/fold the area.\n\r", ch);
         return;
      }
   }
   do_trap(ch, "");
   return;
}
         
            
      

/* Sunangel
   LOOP <command> [start] [end] [params]
   Added a few checks so you can only loop what I want -- Xerves
 */


void do_loop(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];
   int startvnum, endvnum, i;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);

   if (arg1[0] == '\0')
   {
      send_to_char("&C&wSyntax: loop <command> <start#> <end#> <params>\n\r", ch);
      send_to_char("  Where <command> is a valid command to execute,\n\r", ch);
      send_to_char("  <start#> and <end#> are numbers/vnums,\n\r", ch);
      send_to_char("  and <params> is a parameter list for <command>.\n\r", ch);
      send_to_char("&GEXAMPLE: LOOP MSET 22000 22100 FLAGS PROTOTYPE&C&w\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "mset") || !str_cmp(arg1, "oset") || !str_cmp(arg1, "redit")
      || !str_cmp(arg1, "atobj") || !str_cmp(arg1, "atmob") || !str_cmp(arg1, "at") || !str_cmp(arg1, "minvoke") || !str_cmp(arg1, "oinvoke"))
   {
      if (arg2[0] == '\0')
      {
         send_to_char("You must specify a start number/vnum.\n\r", ch);
         return;
      }

      if (arg3[0] == '\0')
      {
         send_to_char("You must specify an end number/vnum.\n\r", ch);
         return;
      }

      startvnum = (is_number(arg2) ? atoi(arg2) : 1);
      endvnum = (is_number(arg3) ? atoi(arg3) : 1);

      if (endvnum < 0)
         endvnum = 1;

      if (startvnum < 0)
         startvnum = 1;

      if (startvnum > endvnum)
      {
         i = endvnum;
         endvnum = startvnum;
         startvnum = i;
      }

      sprintf(buf, "Beginning loop for %s command, vnums %d to %d (%s).\n\r", arg1, startvnum, endvnum, argument);

      send_to_char(buf, ch);

      for (i = startvnum; i <= endvnum; i++)
      {
         sprintf(buf, "%s %d %s", arg1, i, (!str_cmp(arg1, "mstat") || !str_cmp(arg1, "ostat")) ? "\b" : argument);
         interpret(ch, buf);
      }

      send_to_char("Done.\n\r", ch);
      return;
   }
   else
   {
      send_to_char("Command options are oset, mset, redit, at, atmob, atobj, minvoke, oinvoke\n\r", ch);
      return;
   }
}

void do_gwhere(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   bool found = FALSE;
   int count = 0;

   for (d = first_descriptor; d; d = d->next)
      if ((d->connected == CON_PLAYING || d->connected == CON_EDITING)
         && (victim = d->character) != NULL && !IS_NPC(victim) && victim->in_room
         && can_see_map(ch, victim))
      {
         found = TRUE;
         pager_printf_color(ch, "&c&w%-12.12s   [%-5d - %-19.19s]   &c%-25.25s\n\r",
            victim->name, victim->in_room->vnum, victim->in_room->area->name, victim->in_room->name);
         count++;
      }
   pager_printf_color(ch, "&c%d pcs found.\n\r", count);
   return;
}

void do_gfighting(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   bool found = FALSE, pmobs = FALSE, ppcs = FALSE;
   int count = 0;

   if (argument[0] == '\0')
   {
      send_to_pager_color("\n\r&wSyntax:  gfighting pcs | gfighting mobs\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "mobs"))
      pmobs = TRUE;
   else if (!str_cmp(argument, "pcs"))
      ppcs = TRUE;
   else
      do_gfighting(ch, "");

   pager_printf_color(ch, "\n\r&cGlobal %s conflict:\n\r", pmobs ? "mob" : "character");
   if (ppcs)
   {
      for (d = first_descriptor; d; d = d->next)
         if ((d->connected == CON_PLAYING || d->connected == CON_EDITING)
            && (victim = d->character) != NULL && !IS_NPC(victim) && victim->in_room
            && can_see_map(ch, victim) && victim->fighting)
         {
            found = TRUE;
            pager_printf_color(ch, "&w%-16.16s &wvs &w%-16.16s [%5d]  &c%-20.20s [%5d]\n\r",
               victim->name,
               IS_NPC(victim->fighting->who) ? victim->fighting->who->short_descr : victim->fighting->who->name,
               IS_NPC(victim->fighting->who) ? victim->fighting->who->pIndexData->vnum : 0,
               victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum);
            count++;
         }
   }
   else
   {
      for (victim = first_char; victim; victim = victim->next)
         if (IS_NPC(victim) && victim->in_room && can_see_map(ch, victim) && victim->fighting)
         {
            found = TRUE;
            pager_printf_color(ch, "&w%-16.16s &wvs &w%-16.16s [%5d]  &c%-20.20s [%5d]\n\r",
               victim->name,
               IS_NPC(victim->fighting->who) ? victim->fighting->who->short_descr : victim->fighting->who->name,
               victim->pIndexData->vnum,
               victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum);
            count++;
         }
   }
   pager_printf_color(ch, "&c%d %s conflicts located.\n\r", count, pmobs ? "mob" : "character");
   return;
}

/* Added 'show' argument for lowbie imms without ostat -- Blodkai */
/* Made show the default action :) Shaddai */
/* Trimmed size, added vict info, put lipstick on the pig -- Blod */
void do_bodybag(CHAR_DATA * ch, char *argument)
{
   char buf2[MSL];
   char buf3[MSL];
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *owner;
   OBJ_DATA *obj;
   bool found = FALSE, bag = FALSE;

   argument = one_argument(argument, arg1);
   if (arg1[0] == '\0')
   {
      send_to_char_color("&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r", ch);
      return;
   }

   sprintf(buf3, " ");
   sprintf(buf2, "the corpse of %s", arg1);
   argument = one_argument(argument, arg2);

   if (arg2[0] != '\0' && (str_cmp(arg2, "yes") && str_cmp(arg2, "bag") && str_cmp(arg2, "now")))
   {
      send_to_char_color("\n\r&PSyntax:  bodybag <character> | bodybag <character> yes/bag/now\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "yes") || !str_cmp(arg2, "bag") || !str_cmp(arg2, "now"))
      bag = TRUE;

   pager_printf_color(ch, "\n\r&P%s remains of %s ... ", bag ? "Retrieving" : "Searching for", capitalize(arg1));
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->in_room && !str_cmp(buf2, obj->short_descr) && (obj->pIndexData->vnum == 11))
      {
         int cdays, rtime, cmin, chour;
         char cbuf[MSL];
         send_to_pager("\n\r", ch);
         found = TRUE;
                  
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
         sprintf(cbuf, "D:%d H:%d M:%d\n\r", cdays, chour, cmin);
         pager_printf_color(ch, "&P%s:  %s%-12.12s   &PIn:  &w%-22.22s  &P[&w%5d&P]   &PTimer: %s",
            bag ? "Bagging" : "Corpse",
            bag ? "&R" : "&w",
            capitalize(arg1),
            obj->in_room->area->name, obj->in_room->vnum, cbuf);
         if (bag)
         {
            obj_from_room(obj);
            obj = obj_to_char(obj, ch);
            obj->timer = -1;
            update_container(obj, ch->coord->x, ch->coord->y, ch->map, 0, 0, 0);
            save_char_obj(ch);
         }
      }
   }
   if (!found)
   {
      send_to_pager_color("&Pno corpse was found.\n\r", ch);
      return;
   }
   send_to_pager("\n\r", ch);
   for (owner = first_char; owner; owner = owner->next)
   {
      if (IS_NPC(owner))
         continue;
      if (can_see_map(ch, owner) && !str_cmp(arg1, owner->name))
         break;
   }
   if (owner == NULL)
   {
      pager_printf_color(ch, "&P%s is not currently online.\n\r", capitalize(arg1));
      return;
   }
   if (owner->pcdata->deity)
      pager_printf_color(ch, "&P%s has %d favor with %s (needed to supplicate: %d)\n\r",
         owner->name, owner->pcdata->favor, owner->pcdata->deity->name, owner->pcdata->deity->scorpse);
   else
      pager_printf_color(ch, "&P%s has no deity.\n\r", owner->name);
   return;
}


/* New owhere by Altrag, 03/14/96 */
void do_owhere(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   char arg1[MIL];
   OBJ_DATA *obj;
   bool found;
   int icnt = 0;

   set_pager_color(AT_PLAIN, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Owhere what?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   if (arg1[0] != '\0' && !str_prefix(arg1, "nesthunt"))
   {
      if (!(obj = get_obj_world(ch, arg)))
      {
         send_to_char("Nesthunt for what object?\n\r", ch);
         return;
      }
      for (; obj->in_obj; obj = obj->in_obj)
      {
         pager_printf(ch, "[%5d] %-28s in object [%5d] %s\n\r",
            obj->pIndexData->vnum, obj_short(obj), obj->in_obj->pIndexData->vnum, obj->in_obj->short_descr);
         ++icnt;
      }
      sprintf(buf, "[%5d] %-28s in ", obj->pIndexData->vnum, obj_short(obj));
      if (obj->carried_by)
         sprintf(buf + strlen(buf), "invent [%5d] %s\n\r",
            (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum : 0), PERS_MAP(obj->carried_by, ch));
      else if (obj->in_room)
         sprintf(buf + strlen(buf), "room   [%5d] %s\n\r", obj->in_room->vnum, obj->in_room->name);
      else if (obj->in_obj)
      {
         bug("do_owhere: obj->in_obj after NULL!", 0);
         strcat(buf, "object??\n\r");
      }
      else
      {
         strcat(buf, "the bank.\n\r");
      }
      send_to_pager(buf, ch);
      ++icnt;
      pager_printf(ch, "Nested %d levels deep.\n\r", icnt);
      return;
   }

   found = FALSE;
   for (obj = first_object; obj; obj = obj->next)
   {
      if (!nifty_is_name(arg, obj->name))
         continue;
      found = TRUE;

      sprintf(buf, "(%3d) [%5d] %-28s in ", ++icnt, obj->pIndexData->vnum, obj_short(obj));
      if (obj->carried_by)
         sprintf(buf + strlen(buf), "invent [%5d] %s\n\r",
            (IS_NPC(obj->carried_by) ? obj->carried_by->pIndexData->vnum : 0), PERS_MAP(obj->carried_by, ch));
      else if (obj->in_room)
         sprintf(buf + strlen(buf), "room   [%5d] %s\n\r", obj->in_room->vnum, obj->in_room->name);
      else if (obj->in_obj)
         sprintf(buf + strlen(buf), "object [%5d] %s\n\r", obj->in_obj->pIndexData->vnum, obj_short(obj->in_obj));
      else
      {
         strcat(buf, "the bank.\n\r");
      }
      send_to_pager(buf, ch);
   }

   if (!found)
      act(AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR);
   else
      pager_printf(ch, "%d matches.\n\r", icnt);
   return;
}

void do_reboo(CHAR_DATA * ch, char *argument)
{
   send_to_char_color("&YIf you want to REBOOT, spell it out.\n\r", ch);
   return;
}

void save_pc_corpses args((void));

void do_reboot(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   extern bool mud_down;
   CHAR_DATA *vch;
   int x;

   set_char_color(AT_IMMORT, ch);

   if (str_cmp(argument, "mud now") && str_cmp(argument, "nosave") && str_cmp(argument, "and sort skill table"))
   {
      send_to_char("Syntax:  'reboot mud now' or 'reboot nosave'\n\r", ch);
      return;
   }

   if (auction->item)
      do_auction(ch, "stop");
   sprintf(buf, "A reboot has started, please reconnect.");
   bug("A reboot has been initiated by %s", ch->name);
   do_echo(ch, buf);

   if (!str_cmp(argument, "and sort skill table"))
   {
      sort_skill_table();
      save_skill_table();
   }

   /* Save all characters before booting. */
   if (str_cmp(argument, "nosave"))
   {
      for (vch = first_char; vch; vch = vch->next)
      {
         if (!IS_NPC(vch))
            save_char_obj(vch);
      }
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
        write_kingdom_file(x);
      }
      write_kingdom_list();
      weather_update();
      save_resources("solan", 0);
      save_sysdata(sysdata);
      save_map("solan", 0);
      save_bin_data();
      remove(MILIST_FILE);
      save_wblock_data();
      save_mlist_data();
      save_extraction_data();
      write_channelhistory_file();
      save_pc_corpses();
      save_quest_data();
   }
   mud_down = TRUE;
   return;
}

void do_shutdow(CHAR_DATA * ch, char *argument)
{
   send_to_char_color("&YIf you want to SHUTDOWN, spell it out.\n\r", ch);
   return;
}

void do_shutdown(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   extern bool mud_down;
   CHAR_DATA *vch;
   int x;

   set_char_color(AT_IMMORT, ch);

   if (str_cmp(argument, "mud now") && str_cmp(argument, "nosave"))
   {
      send_to_char("Syntax:  'shutdown mud now' or 'shutdown nosave'\n\r", ch);
      return;
   }

   if (auction->item)
      do_auction(ch, "stop");
   sprintf(buf, "A shutdown has started, please reconnect soon.");
   bug("A shutdown was executed by %s", ch->name);
   append_file(ch, SHUTDOWN_FILE, buf);
   strcat(buf, "\n\r");
   do_echo(ch, buf);

   /* Save all characters before booting. */
   if (str_cmp(argument, "nosave"))
   {
      for (vch = first_char; vch; vch = vch->next)
      {
         if (!IS_NPC(vch))
            save_char_obj(vch);
      }
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
        write_kingdom_file(x);
      }
      write_kingdom_list();
      weather_update();
      save_resources("solan", 0);
      save_sysdata(sysdata);
      save_map("solan", 0);
      save_bin_data();
      remove(MILIST_FILE);
      save_wblock_data();
      save_mlist_data();
      save_extraction_data();
      write_channelhistory_file();
      save_pc_corpses();
      save_quest_data();
   }
   mud_down = TRUE;
   return;
}

void do_snoop(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Snoop whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!victim->desc)
   {
      send_to_char("No descriptor to snoop.\n\r", ch);
      return;
   }
   if (victim == ch)
   {
      send_to_char("Cancelling all snoops.\n\r", ch);
      for (d = first_descriptor; d; d = d->next)
         if (d->snoop_by == ch->desc)
            d->snoop_by = NULL;
      return;
   }
   if (victim->desc->snoop_by)
   {
      send_to_char("Busy already.\n\r", ch);
      return;
   }

   /*
    * Minimum snoop level... a secret mset value
    * makes the snooper think that the victim is already being snooped
    */
   if (get_trust(victim) >= get_trust(ch) || (victim->pcdata && victim->pcdata->min_snoop > get_trust(ch)))
   {
      send_to_char("Busy already.\n\r", ch);
      return;
   }

   if (ch->desc)
   {
      for (d = ch->desc->snoop_by; d; d = d->snoop_by)
         if (d->character == victim || d->original == victim)
         {
            send_to_char("No snoop loops.\n\r", ch);
            return;
         }
   }

/*  Snoop notification for higher imms, if desired, uncomment this */
#ifdef TOOSNOOPY
   if (get_trust(victim) >= LEVEL_HI_IMM && get_trust(ch) < LEVEL_ADMIN) /* Tracker1 */
      write_to_descriptor(victim->desc->descriptor, "\n\rYou feel like someone is watching your every move...\n\r", 0);
#endif
   victim->desc->snoop_by = ch->desc;
   send_to_char("Ok.\n\r", ch);
   return;
}

void do_switch(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Switch into whom?\n\r", ch);
      return;
   }
   if (!ch->desc)
      return;
   if (ch->desc->original)
   {
      send_to_char("You are already switched.\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (victim == ch)
   {
      send_to_char("Ok.\n\r", ch);
      return;
   }
   if (victim->desc)
   {
      send_to_char("Character in use.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && ch->level < LEVEL_STAFF) /* Tracker1 */
   {
      send_to_char("You cannot switch into a player!\n\r", ch);
      return;
   }
   if (IS_NPC(victim) && (xIS_SET(victim->act, ACT_PROTECT)) && (get_trust(ch) < LEVEL_ADMIN))
   {
      send_to_char("This mob is protected.\n\r", ch);
      return;
   }

   ch->desc->character = victim;
   ch->desc->original = ch;
   victim->desc = ch->desc;
   ch->desc = NULL;
   ch->switched = victim;
   send_to_char("Ok.\n\r", victim);
   return;
}

void do_return(CHAR_DATA * ch, char *argument)
{

   if (!IS_NPC(ch) && get_trust(ch) < LEVEL_IMMORTAL)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, ch);

   if (!ch->desc)
      return;
   if (!ch->desc->original)
   {
      send_to_char("You aren't switched.\n\r", ch);
      return;
   }

   send_to_char("You return to your original body.\n\r", ch);

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_POSSESS))
   {
      affect_strip(ch, gsn_possess);
      xREMOVE_BIT(ch->affected_by, AFF_POSSESS);
   }

   ch->desc->character = ch->desc->original;
   ch->desc->original = NULL;
   ch->desc->character->desc = ch->desc;
   ch->desc->character->switched = NULL;
   ch->desc = NULL;
   return;
}

void do_minvoke(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *victim;
   sh_int vnum;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Syntax:  minvoke <vnum>\n\r", ch);
      return;
   }
   if (!is_number(arg))
   {
      char arg2[MIL];
      int hash, cnt;
      int count = number_argument(arg, arg2);

      vnum = -1;
      for (hash = cnt = 0; hash < MAX_KEY_HASH; hash++)
         for (pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next)
            if (nifty_is_name(arg2, pMobIndex->player_name) && ++cnt == count)
            {
               vnum = pMobIndex->vnum;
               break;
            }
      if (vnum == -1)
      {
         send_to_char("No such mobile exists.\n\r", ch);
         return;
      }
   }
   else
      vnum = atoi(arg);

   if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
   {
      AREA_DATA *pArea;

      if (IS_NPC(ch))
      {
         send_to_char("Huh?\n\r", ch);
         return;
      }
      if (!ch->pcdata || !(pArea = ch->pcdata->area))
      {
         send_to_char("You must have an assigned area to invoke this mobile.\n\r", ch);
         return;
      }
      if (vnum < pArea->low_m_vnum && vnum > pArea->hi_m_vnum)
      {
         send_to_char("That number is not in your allocated range.\n\r", ch);
         return;
      }
   }
   if ((pMobIndex = get_mob_index(vnum)) == NULL)
   {
      send_to_char("No mobile has that vnum.\n\r", ch);
      return;
   }

   victim = create_mobile(pMobIndex);
   char_to_room(victim, ch->in_room);

   /* If you load one on the map, make sure it gets placed properly - Samson 8-21-99 */
   if (IS_ONMAP_FLAG(ch))
   {
      SET_ONMAP_FLAG(victim);
      victim->map = ch->map;
      victim->coord->x = ch->coord->x;
      victim->coord->y = ch->coord->y;
   }


   act(AT_IMMORT, "$n invokes $N!", ch, NULL, victim, TO_ROOM);
   /*How about seeing what we're invoking for a change. -Blodkai */
   ch_printf_color(ch, "&YYou invoke %s (&W#%d &Y- &W%s&Y)\n\r",
      pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name);
   return;
}

void do_oinvoke(CHAR_DATA * ch, char *argument)
{
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   char arg1[MIL];
   sh_int vnum;
   int cnt;

   set_char_color(AT_IMMORT, ch);

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: oinvoke <vnum> [num].\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   if (!is_number(arg1))
   {
      char arg[MIL];
      int hash, cnt;
      int count = number_argument(arg1, arg);

      vnum = -1;
      for (hash = cnt = 0; hash < MAX_KEY_HASH; hash++)
         for (pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next)
            if (nifty_is_name(arg, pObjIndex->name) && ++cnt == count)
            {
               vnum = pObjIndex->vnum;
               break;
            }
      if (vnum == -1)
      {
         send_to_char("No such object exists.\n\r", ch);
         return;
      }
   }
   else
      vnum = atoi(arg1);

   if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
   {
      AREA_DATA *pArea;

      if (IS_NPC(ch))
      {
         send_to_char("Huh?\n\r", ch);
         return;
      }
      if (!ch->pcdata || !(pArea = ch->pcdata->area))
      {
         send_to_char("You must have an assigned area to invoke this object.\n\r", ch);
         return;
      }
      if (vnum < pArea->low_o_vnum && vnum > pArea->hi_o_vnum)
      {
         send_to_char("That number is not in your allocated range.\n\r", ch);
         return;
      }
   }
   if ((pObjIndex = get_obj_index(vnum)) == NULL)
   {
      send_to_char("No object has that vnum.\n\r", ch);
      return;
   }

/* Commented out by Narn, it seems outdated
    if ( IS_OBJ_STAT( pObjIndex, ITEM_PROTOTYPE )
    &&	 pObjIndex->count > 5 )
    {
	send_to_char( "That object is at its limit.\n\r", ch );
	return;
    }
*/
   cnt = atoi(argument);
   if (cnt < 1)
      cnt = 1;
    
   if (cnt > 100)
   {
      send_to_char("You can only invoke 100 at a time, set to 100.\n\r", ch);
      cnt = 100;
   }   
   for (;cnt > 0; cnt--)
   {
      obj = create_object(pObjIndex, 0);
      if (CAN_WEAR(obj, ITEM_TAKE))
      {
         obj = obj_to_char(obj, ch);
      }
      else
      {
         obj = obj_to_room(obj, ch->in_room, ch);
         if (cnt == 1)
            act(AT_IMMORT, "$n fashions $p from ether!", ch, obj, NULL, TO_ROOM);            
      }
      if (IS_ONMAP_FLAG(ch))
      {
         SET_OBJ_STAT(obj, ITEM_ONMAP);
         obj->map = ch->map;
         obj->coord->x = ch->coord->x;
         obj->coord->y = ch->coord->y;
      }
   }
   /* I invoked what? --Blodkai */
   ch_printf_color(ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y)\n\r", pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name);
   return;
}

void do_purge(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim, *tch;
   OBJ_DATA *obj;

   set_char_color(AT_IMMORT, ch);

   /* Mset/Oset/Redit On Mode check -- Stop most building crashes -- Xerves 8/7/99 */
   if ((xIS_SET(ch->act, PLR_MSET)) || (xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_REDIT)))
   {
      send_to_char("You need to turn mset/oset/redit off\n\r", ch);
      return;
   }

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      /* 'purge' */
      CHAR_DATA *vnext;
      OBJ_DATA *obj_next;

      for (victim = ch->in_room->first_person; victim; victim = vnext)
      {
         vnext = victim->next_in_room;

         for (tch = ch->in_room->first_person; tch; tch = tch->next_in_room)
            if (!IS_NPC(tch) && tch->dest_buf == victim)
               break;
         if (tch && !IS_NPC(tch) && tch->dest_buf == victim)
            continue;

         if (IS_NPC(victim) && (xIS_SET(victim->act, ACT_PROTECT)) && (get_trust(ch) < LEVEL_STAFF))
         {
            continue;
         }

         if (IS_NPC(victim) && victim != ch)
         {
            /* If target is on a map, make sure your at the right coordinates - Samson */
            if (IS_ONMAP_FLAG(ch) && IS_ONMAP_FLAG(victim))
            {
               if (ch->map == victim->map && ch->coord->x == victim->coord->x && ch->coord->y == victim->coord->y)
                  extract_char(victim, TRUE);
               else
                  continue;
            }
            else
               extract_char(victim, TRUE);
         }
      }

      for (obj = ch->in_room->first_content; obj; obj = obj_next)
      {
         obj_next = obj->next_content;

         for (tch = ch->in_room->first_person; tch; tch = tch->next_in_room)
            if (!IS_NPC(tch) && tch->dest_buf == obj)
               break;
         if (tch && !IS_NPC(tch) && tch->dest_buf == obj)
            continue;

         if (IS_OBJ_STAT(obj, ITEM_NOPURGE))
            continue;

         /* If target is on a map, make sure your at the right coordinates - Samson */
         if (IS_ONMAP_FLAG(ch) && IS_OBJ_STAT(obj, ITEM_ONMAP))
         {
            if (ch->map == obj->map && ch->coord->x == obj->coord->x && ch->coord->y == obj->coord->y)
               extract_obj(obj);
            else
               continue;
         }
         else
            extract_obj(obj);
      }

      act(AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM);
      act(AT_IMMORT, "You have purged the room!", ch, NULL, NULL, TO_CHAR);
      return;
   }
   victim = NULL;
   obj = NULL;

   /* fixed to get things in room first -- i.e., purge portal (obj),
    * no more purging mobs with that keyword in another room first
    * -- Tri */
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL && (obj = get_obj_here(ch, arg)) == NULL)
   {
      if ((victim = get_char_world(ch, arg)) == NULL && (obj = get_obj_world(ch, arg)) == NULL) /* no get_obj_room */
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }
   }

/* Single object purge in room for high level purge - Scryn 8/12*/
   if (obj)
   {
      if (IS_ONMAP_FLAG(ch) && IS_OBJ_STAT(obj, ITEM_ONMAP))
      {
         if (ch->map != obj->map || ch->coord->x != obj->coord->x || ch->coord->y != obj->coord->y)
            return;
      }

      separate_obj(obj);
      act(AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM);
      act(AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR);
      extract_obj(obj);
      return;
   }
   if (IS_NPC(victim) && (xIS_SET(victim->act, ACT_PROTECT)) && (get_trust(ch) < LEVEL_STAFF))
   {
      send_to_char("This mob is protected against forces, transfers, slays, and purges\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Not on PC's.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("You cannot purge yourself!\n\r", ch);
      return;
   }
   if (IS_ONMAP_FLAG(ch) && IS_ONMAP_FLAG(victim))
   {
      if (ch->map != victim->map || ch->coord->x != victim->coord->x || ch->coord->y != victim->coord->y)
         return;
   }
   act(AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT);
   act(AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR);
   extract_char(victim, TRUE);
   return;
}

void do_low_purge(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Purge what?\n\r", ch);
      return;
   }

   victim = NULL;
   obj = NULL;
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL && (obj = get_obj_here(ch, arg)) == NULL)
   {
      send_to_char("You can't find that here.\n\r", ch);
      return;
   }

   if (obj)
   {
      if (IS_ONMAP_FLAG(ch) && IS_OBJ_STAT(obj, ITEM_ONMAP))
      {
         if (ch->map != obj->map || ch->coord->x != obj->coord->x || ch->coord->y != obj->coord->y)
            return;
      }
      separate_obj(obj);
      act(AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM);
      act(AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR);
      extract_obj(obj);
      return;
   }

   if (!IS_NPC(victim))
   {
      send_to_char("Not on PC's.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("You cannot purge yourself!\n\r", ch);
      return;
   }
   if (IS_ONMAP_FLAG(ch) && IS_ONMAP_FLAG(victim))
   {
      if (ch->map != victim->map || ch->coord->x != victim->coord->x || ch->coord->y != victim->coord->y)
         return;
   }
   if (IS_NPC(victim) && (xIS_SET(victim->act, ACT_PROTECT)) && (get_trust(ch) < LEVEL_STAFF))
   {
      send_to_char("This mob is protected against forces, transfers, slays, and purges\n\r", ch);
      return;
   }
   act(AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT);
   act(AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR);
   extract_char(victim, TRUE);
   return;
}

void do_balzhur(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MSL];
   char buf2[MSL];
   char *name;
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;

   set_char_color(AT_BLOOD, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Who is deserving of such a fate?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't currently playing.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("This will do little good on mobiles.\n\r", ch);
      return;
   }
   if (victim->level >= get_trust(ch))
   {
      send_to_char("I wouldn't even think of that if I were you...\n\r", ch);
      return;
   }

   set_char_color(AT_WHITE, ch);
   send_to_char("You summon the demon Balzhur to wreak your wrath!\n\r", ch);
   send_to_char("Balzhur sneers at you evilly, then vanishes in a puff of smoke.\n\r", ch);
   set_char_color(AT_IMMORT, victim);
   send_to_char("You hear an ungodly sound in the distance that makes your blood run cold!\n\r", victim);
   sprintf(buf, "Balzhur screams, 'You are MINE %s!!!'", victim->name);
   echo_to_all(AT_IMMORT, buf, ECHOTAR_ALL);
   victim->level = 1;
   victim->trust = 0;
   victim->max_hit = 10;
   victim->max_mana = 100;
   for (sn = 0; sn < top_sn; sn++)
      victim->pcdata->learned[sn] = 0;
   victim->practice = 0;
   victim->hit = victim->max_hit;
   victim->mana = victim->max_mana;
   name = capitalize(victim->name);
   sprintf(buf, "%s%s", GOD_DIR, name);

   set_char_color(AT_RED, ch);
   if (!remove(buf))
      send_to_char("Player's immortal data destroyed.\n\r", ch);
   else if (errno != ENOENT)
   {
      ch_printf(ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r", errno, strerror(errno));
      sprintf(buf2, "%s balzhuring %s", ch->name, buf);
      perror(buf2);
   }
   sprintf(buf2, "%s.are", name);
   for (pArea = first_build; pArea; pArea = pArea->next)
      if (!str_cmp(pArea->filename, buf2))
      {
         sprintf(buf, "%s%s", BUILD_DIR, buf2);
         if (IS_SET(pArea->status, AREA_LOADED))
            fold_area(pArea, buf, FALSE, 0);
         close_area(pArea);
         sprintf(buf2, "%s.bak", buf);
         set_char_color(AT_RED, ch); /* Log message changes colors */
         if (!rename(buf, buf2))
            send_to_char("Player's area data destroyed.  Area saved as backup.\n\r", ch);
         else if (errno != ENOENT)
         {
            ch_printf(ch, "Unknown error #%d - %s (area data).  Report to  Thoric.\n\r", errno, strerror(errno));
            sprintf(buf2, "%s destroying %s", ch->name, buf);
            perror(buf2);
         }
         break;
      }

   make_wizlist();
   do_help(victim, "M_BALZHUR_");
   set_char_color(AT_WHITE, victim);
   send_to_char("You awake after a long period of time...\n\r", victim);
   while (victim->first_carrying)
      extract_obj(victim->first_carrying);
   return;
}

void do_advance(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   int level;
   int iLevel;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2))
   {
      send_to_char("Syntax:  advance <character> <level>\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("That character is not on the mud.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You cannot advance a mobile.\n\r", ch);
      return;
   }
   /*You can demote yourself but not someone else at your own trust.-- Narn */
   if (get_trust(ch) <= get_trust(victim) && ch != victim)
   {
      send_to_char("You can't do that.\n\r", ch);
      return;
   }
   if ((level = atoi(arg2)) < 1 || level > MAX_LEVEL)
   {
      ch_printf(ch, "Level range is 1 to %d.\n\r", MAX_LEVEL);
      return;
   }
   if (level > get_trust(ch))
   {
      send_to_char("Level limited to your trust level.\n\r", ch);
      return;
   }
   /* Lower level:
    *   Reset to level 1.
    *   Then raise again.
    *   Currently, an imp can lower another imp.
    *   -- Swiftest
    *   Can't lower imms >= your trust (other than self) per Narn's change.
    *   Few minor text changes as well.  -- Blod
    */
   if (level <= victim->level)
   {

      set_char_color(AT_IMMORT, victim);
      if (level < victim->level)
      {
         ch_printf(ch, "Demoting %s from level %d to level %d!\n\r", victim->name, victim->level, level);
         send_to_char("Cursed and forsaken!  The gods have lowered your level...\n\r", victim);
      }
      victim->level = 1;
      victim->max_hit = 20;
      victim->max_mana = 100;
      victim->perm_agi = 15;
      victim->practice = 0;
      victim->pcdata->lore = 0;
      victim->pcdata->train = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      /* Rank fix added by Narn. */
      DISPOSE(victim->pcdata->rank);
      victim->pcdata->rank = str_dup("");
      /* Stuff added to make sure character's wizinvis level doesn't stay
         higher than actual level, take wizinvis away from advance < 50 */
      if (xIS_SET(victim->act, PLR_WIZINVIS))
         victim->pcdata->wizinvis = victim->trust;
      if (xIS_SET(victim->act, PLR_WIZINVIS) && (victim->level <= LEVEL_PC))
      {
         xREMOVE_BIT(victim->act, PLR_WIZINVIS);
         victim->pcdata->wizinvis = victim->trust;
      }
   }
   else
   {
      ch_printf(ch, "Raising %s from level %d to level %d!\n\r", victim->name, victim->level, level);
      if (victim->level >= LEVEL_PC)
      {
         set_char_color(AT_IMMORT, victim);
         act(AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at you!", ch, NULL, victim, TO_VICT);
         act(AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at $N!", ch, NULL, victim, TO_NOTVICT);
         set_char_color(AT_WHITE, victim);
         send_to_char("You suddenly feel very strange...\n\r\n\r", victim);
         set_char_color(AT_LBLUE, victim);
      }
      switch (level)
      {
         default:
            send_to_char("The gods feel fit to raise your level!\n\r", victim);
            break;
         case LEVEL_IMMORTAL:
            do_help(victim, "M_GODLVL1_");
            set_char_color(AT_WHITE, victim);
            send_to_char("You awake... all your possessions are gone.\n\r", victim);
            victim->pcdata->caste = caste_Ascender;
            while (victim->first_carrying)
               extract_obj(victim->first_carrying);
            break;
         case LEVEL_IMM: /* Tracker1 */
            victim->pcdata->caste = caste_Immortal;
            do_help(victim, "M_GODLVL2_");
            break;
         case LEVEL_HI_IMM:
            victim->pcdata->caste = caste_God;
            do_help(victim, "M_GODLVL3_");
            break;
         case LEVEL_STAFF:
            victim->pcdata->caste = caste_Staff;
            do_help(victim, "M_GODLVL4_");
            break;
         case LEVEL_HI_STAFF:
            victim->pcdata->caste = caste_Staff;
            do_help(victim, "M_GODLVL5_");
            break;
         case LEVEL_ADMIN:
            victim->pcdata->caste = caste_Admin;
            do_help(victim, "M_GODLVL6_");
      }
   }
   for (iLevel = victim->level; iLevel < level;)
   {
      victim->level += 1;
      iLevel++;
   }
   victim->trust = 0;
   return;
}

void do_trust(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   int level;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2))
   {
      send_to_char("Syntax:  trust <char> <level>.\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("That player is not here.\n\r", ch);
      return;
   }
   if ((level = atoi(arg2)) < 0 || level > MAX_LEVEL)
   {
      send_to_char("Level must be 0 (reset) or 1 to 7.\n\r", ch);
      return;
   }
   if (level > get_trust(ch))
   {
      send_to_char("Limited to your own trust.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You can't do that.\n\r", ch);
      return;
   }

   victim->trust = level;
   send_to_char("Ok.\n\r", ch);
   return;
}

bool proto_area(AREA_DATA *tarea)
{
   AREA_DATA *pArea;
   
   for (pArea = first_bsort; pArea; pArea = pArea->next_sort)
   {
      if (pArea == tarea)
         return TRUE;
   }
   return FALSE;
}

/* Summer 1997 --Blod */
//Made this a bit more fun to use, can specify area and can target self (quest purposes mostly)
void do_scatter(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   char arg[MIL];
   char arg2[MIL];
   char buf[MSL];
   ROOM_INDEX_DATA *pRoomIndex;
   int x;
   int y;
   int map;
   AREA_DATA *tarea = NULL;
   int lowvnum = 0;
   int hivnum = MAX_VNUM;
   int cnt = 0;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Scatter whom?\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "all"))
   {
      for (victim = ch->in_room->first_person; victim; victim = v_next)
      {
         v_next = victim->next_in_room;
         if (victim == ch)
            continue;
         if (!IN_SAME_ROOM(ch, victim))
            continue;
         sprintf(buf, "\"%s\" %s", victim->name, argument);
         do_scatter(ch, buf);
      }
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && get_trust(victim) >= get_trust(ch) && victim != ch)
   {
      send_to_char("You haven't the power to succeed against them.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg2);
   if (arg2[0] != '\0')
   {
      if (atoi(arg2) > 0)
      {
         lowvnum = atoi(arg2);
         hivnum = atoi(argument);
         if (lowvnum > hivnum)
         {
            send_to_char("The 2nd number has to be greater than the 1st.\n\r", ch);
            return;
         }
         if (lowvnum < 0)
            lowvnum = 0;
         if (hivnum > MAX_VNUM)
            hivnum = MAX_VNUM;
      }
      else
      {
         for (tarea = first_area; tarea; tarea = tarea->next)
            if (!str_cmp(tarea->filename, arg2))
               break;
      }
   }
   if (tarea)
   {
      lowvnum = tarea->low_r_vnum;
      hivnum = tarea->hi_r_vnum;
      if (!str_cmp(tarea->filename, "Raferover.are"))
         lowvnum = hivnum = OVERLAND_SOLAN;
   }
   x = y = map = -1;
   for (;;)
   {
      pRoomIndex = get_room_index(number_range(lowvnum, hivnum));
      if (number_range(1, 350) == 1 && ((pRoomIndex = get_room_index(OVERLAND_SOLAN)) != NULL) && (lowvnum == 0 || lowvnum == OVERLAND_SOLAN))
      {
         x = number_range(1, MAX_X);
         y = number_range(1, MAX_Y);
         map = number_range(0, MAP_MAX-1);
         if (sect_show[(int)map_sector[map][x][y]].canpass)
            break;
      }
      if (pRoomIndex)
      {
         if (!xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
         && !xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
         && !xIS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
         && !proto_area(pRoomIndex->area)
         && pRoomIndex->first_exit && str_cmp(pRoomIndex->area->filename, "limbo.are") && str_cmp(pRoomIndex->area->filename, "immhouse.are")
         && str_cmp(pRoomIndex->area->filename, "clans.are") && str_cmp(pRoomIndex->area->filename, "arena.are")
         && str_cmp(pRoomIndex->area->filename, "spellbooks.are") && str_cmp(pRoomIndex->area->filename, "arena.are")
         && str_cmp(pRoomIndex->area->filename, "Raferover.are") && (pRoomIndex->vnum < START_QUEST_VNUM || pRoomIndex->vnum > END_QUEST_VNUM))
         {
            x = y = map = -1;
            break;
         }
      }
      cnt++;
      if (cnt == 100000)
         break;
   }
   if (cnt == 100000)
   {
      send_to_char("Could not find a location.\n\r", ch);
      return;
   }
   if (victim->fighting)
      stop_fighting(victim, TRUE);
   act(AT_MAGIC, "With the sweep of an arm, $n flings $N to the winds.", ch, NULL, victim, TO_NOTVICT);
   act(AT_MAGIC, "With the sweep of an arm, $n flings you to the astral winds.", ch, NULL, victim, TO_VICT);
   act(AT_MAGIC, "With the sweep of an arm, you fling $N to the astral winds.", ch, NULL, victim, TO_CHAR);
   char_from_room(victim);
   char_to_room(victim, pRoomIndex);
   REMOVE_ONMAP_FLAG(victim);
   victim->coord->x = x;
   victim->coord->y = y;
   victim->map = map;
   if (x > -1)
   {
      SET_ONMAP_FLAG(victim);
      update_objects(victim, 0, 0, 0);
   }
   victim->position = POS_STANDING;
   act(AT_MAGIC, "$n staggers forth from a sudden gust of wind, and collapses.", victim, NULL, NULL, TO_ROOM);
   do_look(victim, "auto");
   return;
}

void update_object_contents args((OBJ_DATA *obj, int x, int y));

void do_oscatter(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   OBJ_DATA *o_next;
   char arg[MIL];
   char arg2[MIL];
   char buf[MSL];
   ROOM_INDEX_DATA *pRoomIndex;
   int x;
   int y;
   int map;
   AREA_DATA *tarea = NULL;
   int lowvnum = 0;
   int hivnum = MAX_VNUM;
   int cnt = 0;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Scatter what?\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "all"))
   {
      for (obj = ch->in_room->first_content; obj; obj = o_next)
      {
         o_next = obj->next_content;
         if (!IN_SAME_ROOM_OBJ(ch, obj))
            continue;
         sprintf(buf, "\"%s\" %s", obj->name, argument);
         do_oscatter(ch, buf);
      }
      return;
   }
   if ((obj = get_obj_here(ch, arg)) == NULL)
   {
      send_to_char("It isn't here.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg2);
   if (arg2[0] != '\0')
   {
      if (atoi(arg2) > 0)
      {
         lowvnum = atoi(arg2);
         hivnum = atoi(argument);
         if (lowvnum > hivnum)
         {
            send_to_char("The 2nd number has to be greater than the 1st.\n\r", ch);
            return;
         }
         if (lowvnum < 0)
            lowvnum = 0;
         if (hivnum > MAX_VNUM)
            hivnum = MAX_VNUM;
      }
      else
      {
         for (tarea = first_area; tarea; tarea = tarea->next)
            if (!str_cmp(tarea->filename, arg2))
               break;
      }
   }
   if (tarea)
   {
      lowvnum = tarea->low_r_vnum;
      hivnum = tarea->hi_r_vnum;
      if (!str_cmp(tarea->filename, "Raferover.are"))
         lowvnum = hivnum = OVERLAND_SOLAN;
   }
   x = y = map = -1;
   for (;;)
   {
      pRoomIndex = get_room_index(number_range(lowvnum, hivnum));
      if (number_range(1, 350) == 1 && ((pRoomIndex = get_room_index(OVERLAND_SOLAN)) != NULL) && (lowvnum == 0 || lowvnum == OVERLAND_SOLAN))
      {
         x = number_range(1, MAX_X);
         y = number_range(1, MAX_Y);
         map = number_range(0, MAP_MAX-1);
         if (sect_show[(int)map_sector[map][x][y]].canpass)
            break;
      }
      if (pRoomIndex)
      {
         if (!xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
         && !xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
         && !xIS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
         && !proto_area(pRoomIndex->area)
         && pRoomIndex->first_exit && str_cmp(pRoomIndex->area->filename, "limbo.are") && str_cmp(pRoomIndex->area->filename, "immhouse.are")
         && str_cmp(pRoomIndex->area->filename, "clans.are") && str_cmp(pRoomIndex->area->filename, "arena.are")
         && str_cmp(pRoomIndex->area->filename, "spellbooks.are") && str_cmp(pRoomIndex->area->filename, "arena.are")
         && str_cmp(pRoomIndex->area->filename, "Raferover.are") && (pRoomIndex->vnum < START_QUEST_VNUM || pRoomIndex->vnum > END_QUEST_VNUM))
         {
            x = y = map = -1;
            break;
         }
      }
      cnt++;
      if (cnt == 100000)
         break;
   }
   if (cnt == 100000)
   {
      send_to_char("Could not find a location.\n\r", ch);
      return;
   }
   act(AT_MAGIC, "With the sweep of an arm, $n flings $p to the winds.", ch, obj, NULL, TO_NOTVICT);
   act(AT_MAGIC, "With the sweep of an arm, you fling $p to the astral winds.", ch, obj, NULL, TO_CHAR);
   obj_from_room(obj);
   obj_to_room(obj, pRoomIndex, NULL);
   REMOVE_OBJ_STAT(obj, ITEM_ONMAP);
   obj->coord->x = x;
   obj->coord->y = y;
   obj->map = map;
   if (x > -1)
   {
      SET_OBJ_STAT(obj, ITEM_ONMAP);
      update_object_contents(obj, x, y);
   }
   return;
}

void do_strew(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   ROOM_INDEX_DATA *pRoomIndex;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Strew who, what?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("It would work better if they were here.\n\r", ch);
      return;
   }
   if (victim == ch)
   {
      send_to_char("Try taking it out on someone else first.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You haven't the power to succeed against them.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "coins"))
   {
      if (victim->gold < 1)
      {
         send_to_char("Drat, this one's got no gold to start with.\n\r", ch);
         return;
      }
      victim->gold = 0;
      act(AT_MAGIC, "$n gestures and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_NOTVICT);
      act(AT_MAGIC, "You gesture and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "As $n gestures, an unearthly gale sends your currency flying!", ch, NULL, victim, TO_VICT);
      return;
   }
   for (;;)
   {
      pRoomIndex = get_room_index(number_range(0, MAX_VNUM));
      if (pRoomIndex)
         if (!xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
            && !xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
            && !xIS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL) && !xIS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE))
            break;
   }
   if (!str_cmp(arg2, "inventory"))
   {
      act(AT_MAGIC, "$n speaks a single word, sending $N's possessions flying!", ch, NULL, victim, TO_NOTVICT);
      act(AT_MAGIC, "You speak a single word, sending $N's possessions flying!", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "$n speaks a single word, sending your possessions flying!", ch, NULL, victim, TO_VICT);
      for (obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next)
      {
         obj_next = obj_lose->next_content;
         obj_from_char(obj_lose);
         obj_to_room(obj_lose, pRoomIndex, ch);
         pager_printf_color(ch, "\t&w%s sent to %d\n\r", capitalize(obj_lose->short_descr), pRoomIndex->vnum);
      }
      return;
   }
   send_to_char("Strew their coins or inventory?\n\r", ch);
   return;
}

void do_strip(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   int count = 0;

   set_char_color(AT_OBJECT, ch);
   if (!argument)
   {
      send_to_char("Strip who?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("They're not here.\n\r", ch);
      return;
   }
   if (victim == ch)
   {
      send_to_char("Kinky.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You haven't the power to succeed against them.\n\r", ch);
      return;
   }
   act(AT_OBJECT, "Searching $N ...", ch, NULL, victim, TO_CHAR);
   for (obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next)
   {
      obj_next = obj_lose->next_content;
      obj_from_char(obj_lose);
      obj_to_char(obj_lose, ch);
      pager_printf_color(ch, "  &G... %s (&g%s) &Gtaken.\n\r", capitalize(obj_lose->short_descr), obj_lose->name);
      count++;
   }
   if (!count)
      pager_printf_color(ch, "&GNothing found to take.\n\r", ch);
   return;
}

void do_restore(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Restore whom?\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "all"))
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if (!ch->pcdata)
         return;

      if (get_trust(ch) < LEVEL_HI_STAFF) /* Tracker1 */
      {
         if (IS_NPC(ch))
         {
            send_to_char("You can't do that.\n\r", ch);
            return;
         }
         else
         {
            /* Check if the player did a restore all within the last 18 hours. */
            if (current_time - last_restore_all_time < RESTORE_INTERVAL)
            {
               send_to_char("Sorry, you can't do a restore all yet.\n\r", ch);
               do_restoretime(ch, "");
               return;
            }
         }
      }
      last_restore_all_time = current_time;
      ch->pcdata->restore_time = current_time;
      save_char_obj(ch);
      send_to_char("Ok.\n\r", ch);
      for (vch = first_char; vch; vch = vch_next)
      {
         vch_next = vch->next;

         if (!IS_NPC(vch) && !IS_IMMORTAL(vch) && !in_arena(vch))
         {
            vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
            vch->pcdata->condition[COND_BLOODTHIRST] = (10 + vch->level);
            update_pos(vch);
            act(AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT);
         }
      }
   }
   else
   {

      CHAR_DATA *victim;

      if ((victim = get_char_world(ch, arg)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }

      if ((!IS_NPC(victim)) && (get_trust(ch) < LEVEL_HI_IMM) && (victim != ch))
      {
         send_to_char("Xerves deems you unready to restore players", ch);
         return;
      }
      if (IS_NPC(victim) && (get_trust(ch) < LEVEL_HI_IMM) && (!xIS_SET(victim->act, ACT_PROTOTYPE)))
      {
         send_to_char("Xerves deems you unready to restore nonprototype mobs", ch);
         return;
      }

      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      if (victim->pcdata)
         victim->pcdata->condition[COND_BLOODTHIRST] = (10 + victim->level);
      update_pos(victim);
      if (ch != victim)
         act(AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT);
      send_to_char("Ok.\n\r", ch);
      return;
   }
}

void do_restoretime(CHAR_DATA * ch, char *argument)
{
   long int time_passed;
   int hour, minute;

   set_char_color(AT_IMMORT, ch);

   if (!last_restore_all_time)
      ch_printf(ch, "There has been no restore all since reboot.\n\r");
   else
   {
      time_passed = current_time - last_restore_all_time;
      hour = (int) (time_passed / 3600);
      minute = (int) ((time_passed - (hour * 3600)) / 60);
      ch_printf(ch, "The  last restore all was %d hours and %d minutes ago.\n\r", hour, minute);
   }

   if (!ch->pcdata)
      return;

   if (!ch->pcdata->restore_time)
   {
      send_to_char("You have never done a restore all.\n\r", ch);
      return;
   }

   time_passed = current_time - ch->pcdata->restore_time;
   hour = (int) (time_passed / 3600);
   minute = (int) ((time_passed - (hour * 3600)) / 60);
   ch_printf(ch, "Your last restore all was %d hours and %d minutes ago.\n\r", hour, minute);
   return;
}

void do_freeze(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_LBLUE, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Freeze whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   set_char_color(AT_LBLUE, victim);
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed, and they saw...\n\r", ch);
      ch_printf(victim, "%s is attempting to freeze you.\n\r", ch->name);
      return;
   }
   if (xIS_SET(victim->act, PLR_FREEZE))
   {
      xREMOVE_BIT(victim->act, PLR_FREEZE);
      send_to_char("Your frozen form suddenly thaws.\n\r", victim);
      ch_printf(ch, "%s is now unfrozen.\n\r", victim->name);
   }
   else
   {
      xSET_BIT(victim->act, PLR_FREEZE);
      send_to_char("A godly force turns your body to ice!\n\r", victim);
      ch_printf(ch, "You have frozen %s.\n\r", victim->name);
   }
   save_char_obj(victim);
   return;
}

void do_log(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Log whom?\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "all"))
   {
      if (fLogAll)
      {
         fLogAll = FALSE;
         send_to_char("Log ALL off.\n\r", ch);
      }
      else
      {
         fLogAll = TRUE;
         send_to_char("Log ALL on.\n\r", ch);
      }
      return;
   }

   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }

   /*
    * No level check, gods can log anyone.
    */

   if (xIS_SET(victim->act, PLR_LOG))
   {
      xREMOVE_BIT(victim->act, PLR_LOG);
      ch_printf(ch, "LOG removed from %s.\n\r", victim->name);
   }
   else
   {
      xSET_BIT(victim->act, PLR_LOG);
      ch_printf(ch, "LOG applied to %s.\n\r", victim->name);
   }
   return;
}

void do_litterbug(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Set litterbug flag on whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, victim);
   if (xIS_SET(victim->act, PLR_LITTERBUG))
   {
      xREMOVE_BIT(victim->act, PLR_LITTERBUG);
      send_to_char("You can drop items again.\n\r", victim);
      ch_printf(ch, "LITTERBUG removed from %s.\n\r", victim->name);
   }
   else
   {
      xSET_BIT(victim->act, PLR_LITTERBUG);
      send_to_char("A strange force prevents you from dropping any more items!\n\r", victim);
      ch_printf(ch, "LITTERBUG set on %s.\n\r", victim->name);
   }
   return;
}

void do_noemote(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Noemote whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, victim);
   if (xIS_SET(victim->act, PLR_NO_EMOTE))
   {
      xREMOVE_BIT(victim->act, PLR_NO_EMOTE);
      send_to_char("You can emote again.\n\r", victim);
      ch_printf(ch, "NOEMOTE removed from %s.\n\r", victim->name);
   }
   else
   {
      xSET_BIT(victim->act, PLR_NO_EMOTE);
      send_to_char("You can't emote!\n\r", victim);
      ch_printf(ch, "NOEMOTE applied to %s.\n\r", victim->name);
   }
   return;
}

void do_notell(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Notell whom?", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, victim);
   if (xIS_SET(victim->act, PLR_NO_TELL))
   {
      xREMOVE_BIT(victim->act, PLR_NO_TELL);
      send_to_char("You can use tells again.\n\r", victim);
      ch_printf(ch, "NOTELL removed from %s.\n\r", victim->name);
   }
   else
   {
      xSET_BIT(victim->act, PLR_NO_TELL);
      send_to_char("You can't use tells!\n\r", victim);
      ch_printf(ch, "NOTELL applied to %s.\n\r", victim->name);
   }
   return;
}

void do_notitle(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Notitle whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, victim);
   if (IS_SET(victim->pcdata->flags, PCFLAG_NOTITLE))
   {
      REMOVE_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
      send_to_char("You can set your own title again.\n\r", victim);
      ch_printf(ch, "NOTITLE removed from %s.\n\r", victim->name);
   }
   else
   {
      SET_BIT(victim->pcdata->flags, PCFLAG_NOTITLE);
      sprintf(buf, "...");
      set_title(victim, buf);
      send_to_char("You can't set your own title!\n\r", victim);
      ch_printf(ch, "NOTITLE set on %s.\n\r", victim->name);
   }
   return;
}

void do_silence(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Silence whom?", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, victim);
   if (xIS_SET(victim->act, PLR_SILENCE))
   {
      send_to_char("Player already silenced, use unsilence to remove.\n\r", ch);
   }
   else
   {
      xSET_BIT(victim->act, PLR_SILENCE);
      send_to_char("You can't use channels!\n\r", victim);
      ch_printf(ch, "You SILENCE %s.\n\r", victim->name);
   }
   return;
}

/* Much better than toggling this with do_silence, yech --Blodkai */
void do_unsilence(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Unsilence whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You failed.\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, victim);
   if (xIS_SET(victim->act, PLR_SILENCE))
   {
      xREMOVE_BIT(victim->act, PLR_SILENCE);
      send_to_char("You can use channels again.\n\r", victim);
      ch_printf(ch, "SILENCE removed from %s.\n\r", victim->name);
   }
   else
   {
      send_to_char("That player is not silenced.\n\r", ch);
   }
   return;
}

void do_peace(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *rch;

   act(AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM);
   act(AT_IMMORT, "You boom, 'PEACE!'", ch, NULL, NULL, TO_CHAR);
   for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
   {
      if (rch->fighting)
      {
         stop_fighting(rch, TRUE);
         do_sit(rch, "");
      }

      /* Added by Narn, Nov 28/95 */
      stop_hating(rch);
      stop_hunting(rch);
      stop_fearing(rch);
   }

   send_to_char_color("&YOk.\n\r", ch);
   return;
}

WATCH_DATA *first_watch;
WATCH_DATA *last_watch;

void save_watchlist(void)
{
   WATCH_DATA *pwatch;
   FILE *fp;

   fclose(fpReserve);
   if (!(fp = fopen(SYSTEM_DIR WATCH_LIST, "w")))
   {
      bug("Save_watchlist: Cannot open " WATCH_LIST, 0);
      perror(WATCH_LIST);
      fpReserve = fopen(NULL_FILE, "r");
      return;
   }

   for (pwatch = first_watch; pwatch; pwatch = pwatch->next)
      fprintf(fp, "%d %s~%s~%s~\n", pwatch->imm_level, pwatch->imm_name,
         pwatch->target_name ? pwatch->target_name : " ", pwatch->player_site ? pwatch->player_site : " ");
   fprintf(fp, "-1\n");
   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

void do_wizlock(CHAR_DATA * ch, char *argument)
{
   extern bool wizlock;

   wizlock = !wizlock;

   set_char_color(AT_DANGER, ch);

   if (wizlock)
      send_to_char("Game wizlocked.^x\n\r", ch);
   else
      send_to_char("Game un-wizlocked.^x\n\r", ch);
   return;
}

void do_noresolve(CHAR_DATA * ch, char *argument)
{
   sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

   if (sysdata.NO_NAME_RESOLVING)
      send_to_char_color("&YName resolving disabled.\n\r", ch);
   else
      send_to_char_color("&YName resolving enabled.\n\r", ch);
   return;
}

void flush_account_d(DESCRIPTOR_DATA *d)
{
   if (d->account)
   {
      ACCOUNT_NAME *aname;
      ACCOUNT_NAME *anext;
      
      //Don't want to lock them out of their account by getting disconnected, laugh
      if (d->account->editing == 1)
      {
         d->account->editing = 0;
         save_account(d, 0);
      }
      
      for (aname = d->account->first_player; aname; aname = anext)
      {
         anext = aname->next;
         if (aname->name)
            STRFREE(aname->name);
         DISPOSE(aname);
      }
      if (d->account->name)
         STRFREE(d->account->name);
      if (d->account->passwd)
         STRFREE(d->account->passwd);
      if (d->account->email)
         STRFREE(d->account->email);
      DISPOSE(d->account);
   }
   DISPOSE(d);
}

void flush_account(DESCRIPTOR_DATA *d)
{
   if (d->account)
   {
      ACCOUNT_NAME *aname;
      ACCOUNT_NAME *anext;
      
      //Don't want to lock them out of their account by getting disconnected, laugh
      if (d->account->editing == 1)
      {
         d->account->editing = 0;
         save_account(d, 0);
      }
      
      for (aname = d->account->first_player; aname; aname = anext)
      {
         anext = aname->next;
         if (aname->name)
            STRFREE(aname->name);
         DISPOSE(aname);
      }
      if (d->account->name)
         STRFREE(d->account->name);
      if (d->account->passwd)
         STRFREE(d->account->passwd);
      if (d->account->email)
         STRFREE(d->account->email);
      DISPOSE(d->account);
   }
}

void do_accounts(CHAR_DATA *ch, char *argument)
{
   DESCRIPTOR_DATA *d;
   DESCRIPTOR_DATA *dd;
   CHAR_DATA *lch;
   DESCRIPTOR_DATA *dnext;
   ACCOUNT_NAME *aname;
   ACCOUNT_DATA *account;
   char buf[MSL];
   char tbuf[20];
   time_t now;
   int count;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   FILE *fp;
   char *pwdnew;
   char *p;
   int fOld;
   
   if (argument[0] == '\0')
   {
      send_to_char("accounts online\n\r", ch);
      send_to_char("accounts <account>\n\r", ch);
      send_to_char("accounts <account> editing off\n\r", ch);
      send_to_char("accounts <account> changes <value>\n\r", ch);
      send_to_char("accounts <account> disconnect\n\r", ch);
      send_to_char("accounts <account> name <name> <password>\n\r", ch);
      send_to_char("accounts <account> password <password>\n\r", ch);
      send_to_char("accounts <account> email <email>\n\r", ch);
      send_to_char("accounts <account> ban <ban|allow>\n\r", ch);
      send_to_char("accounts <account> release <player> <password>\n\r", ch);
      send_to_char("accounts <account> delete <player>\n\r", ch);
      send_to_char("accounts <account> reloadplayers [account name]\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (str_cmp(arg1, "online"))
   {
      sprintf(buf, "%s%c/%s", ACCOUNT_DIR, tolower(arg1[0]), capitalize(arg1));   
      if ((fp = fopen(buf, "r")) == NULL)
      {
         send_to_char("That account does not exist.\n\r", ch);
         return;
      }
      fclose(fp);
   }
   if (!str_cmp(arg1, "online"))
   {
      count = 0;
      buf[0] = '\0';
      sprintf(buf, "Desc| Con| Idle | Account     |Edi|Chn|Reset| Player      | IP");
      strcat(buf, "\n\r----+----+------+-------------+---+---+-----+-------------+---------------------------\n\r");
      send_to_pager(buf, ch);
   
      for (d = first_descriptor; d; d = d->next)
      {
         count++;
         if (d->account && d->account->lasttimereset)
         {
            time(&now);
            now = d->account->lasttimereset - time(0) + 3600;
            strftime(tbuf, 20, "%M:%S", localtime(&now));
         }
         sprintf(buf, "%4d| %3d| %5d| %-12s| %s |%-3d|%5s| %-12s| %-16s",
            d->descriptor, d->connected, d->idle / 4, d->account && d->account->name ? d->account->name : "(none)",
            d->account && d->account->editing ? "X" : " ", d->account ? d->account->changes : 0,
            d->account && d->account->lasttimereset ? tbuf : "-----", 
            d->original ? d->original->name : d->character ? d->character->name : "(none)", d->host);
         strcat(buf, "\n\r");
         send_to_pager(buf, ch);
      }
      ch_printf(ch, "----+----+------+-------------+---+---+-----+-------------+---------------------------\n\r");
      pager_printf(ch, "%d account%s.\n\r", count, count == 1 ? "" : "s");
      return;
   }
   if (!str_cmp(arg2, "reloadplayers"))
   {
      for (d = first_descriptor; d; d = dnext)
      {
         dnext = d->next;
         if (d->account)
         {
            if (argument[0] == '\0' || !str_cmp(argument, d->account->name))
            {
               CREATE(account, ACCOUNT_DATA, 1);
               sprintf(buf, d->account->name);
               flush_account(d);
               d->account = NULL;
               fOld = load_account(d, buf, TRUE);
               if (!fOld)
               {
                  ch_printf(ch, "account %s does not exist, account being disconnected.\n\r", buf);
                  write_to_buffer(d, "Your account could not be reloaded, disconnecting.\n\r", 0);
                  close_socket(d, FALSE);
               }
               write_to_buffer(d, "Your account has been reloaded by an immortal.\n\r", 0);
            }
         }
      }  
   }
   if (!str_cmp(arg2, "delete"))
   {
      CREATE(d, DESCRIPTOR_DATA, 1);
      fOld = load_account(d, arg1, TRUE);
      if (!fOld)
      {
         send_to_char("That account does not exist.\n\r", ch);
         flush_account_d(d);
      }   
      for (aname = d->account->first_player; aname; aname = aname->next)
      {
         if (!str_cmp(aname->name, argument))
            break;
      }
      if (!aname)
      {
         send_to_char("That account does not have that player in it.\n\r", ch);
         flush_account_d(d);
         return;
      }  
      else
      {
         STRFREE(aname->name);
         UNLINK(aname, d->account->first_player, d->account->last_player, next, prev);
         DISPOSE(aname);
         save_account(d, 0);
         flush_account_d(d);
      }
      sprintf(buf, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));   
      if (remove(buf))
      {
         send_to_char("Removed the player from the account but the file was not there to delete.\n\r", ch);
         return;
      }
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "release"))
   {
      argument = one_argument(argument, arg3);
      CREATE(dd, DESCRIPTOR_DATA, 1);
      fOld = load_account(dd, arg1, TRUE);
      if (!fOld)
      {
         send_to_char("That account does not exist.\n\r", ch);
         flush_account_d(dd);
         return;
      }   
      for (aname = dd->account->first_player; aname; aname = aname->next)
      {
         if (!str_cmp(aname->name, arg3))
            break;
      }
      if (!aname)
      {
         send_to_char("That account does not have that player in it.\n\r", ch);
         flush_account_d(dd);
         return;
      }
      if ( strlen(argument) < 5 )
      {
         send_to_char("Password must be at least five characters long.\n\r", ch );
         flush_account_d(dd);
         return;
      }
      lch = ch;
      d = ch->desc;
      ch->desc->character = NULL;
      fOld = load_char_obj(d, arg3, TRUE);
         
      if (!fOld) //Well something happened, rofl
      {
         write_to_buffer(d, "The pfile does not exist to release it.\n\r", 0);
         d->character->desc = NULL;
         free_char(d->character); /* Big Memory Leak before --Shaddai */
         d->character = NULL;
         d->character = lch;
         flush_account_d(dd);
         return;
      }
      pwdnew = crypt( argument, d->character->name );
      for ( p = pwdnew; *p != '\0'; p++ )
      {
	 if ( *p == '~' )
	 {
	    write_to_buffer( d, "You cannot use &- in your passwords\n\r", 0 );
	    d->character->desc = NULL;
            free_char(d->character); /* Big Memory Leak before --Shaddai */
            d->character = NULL;
            d->character = lch;
            flush_account_d(dd);
            return;
	 }
      }
      DISPOSE(d->character->pcdata->pwd);
      d->character->pcdata->pwd = str_dup(crypt(argument, d->character->name));
      save_char_obj(d->character);
      d->character->desc = NULL;
      free_char(d->character); /* Big Memory Leak before --Shaddai */
      d->character = NULL;
      d->character = lch;
      if (aname)
      {
         STRFREE(aname->name);
         UNLINK(aname, dd->account->first_player, dd->account->last_player, next, prev);
         DISPOSE(aname);
      }
      save_account(dd, 0);
      flush_account_d(dd);
      send_to_char("Done.\n\r", ch);
      return;
   }
      
   if (!str_cmp(arg2, "ban"))
   {
      if (!str_cmp(argument, "ban"))
      {
         CREATE(d, DESCRIPTOR_DATA, 1);
         fOld = load_account(d, arg1, TRUE);
         if (!fOld)
         {
            send_to_char("That account does not exist.\n\r", ch);
            flush_account_d(d);
         }     
         d->account->ban = ABAN_BAN;
         save_account(d, 0);
         flush_account_d(d);
         send_to_char("Done.\n\r", ch);
         return;
      }
      else if (!str_cmp(argument, "allow"))
      {
         CREATE(d, DESCRIPTOR_DATA, 1);
         fOld = load_account(d, arg1, TRUE);
         if (!fOld)
         {
            send_to_char("That account does not exist.\n\r", ch);
            flush_account_d(d);
         }     
         d->account->ban = ABAN_ALLOW;
         save_account(d, 0);
         flush_account_d(d);
         send_to_char("Done.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Your only sellections are ban and allow.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg2, "email"))
   {
      CREATE(d, DESCRIPTOR_DATA, 1);
      fOld = load_account(d, arg1, TRUE);
      if (!fOld)
      {
         send_to_char("That account does not exist.\n\r", ch);
         flush_account_d(d);
      }   
      STRFREE(d->account->email);
      d->account->email = STRALLOC(argument);
      save_account(d, 0);
      flush_account_d(d);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "password"))
   {
      if ( strlen(argument) < 5 )
      {
         send_to_char("Password must be at least five characters long.\n\r", ch );
         return;
      }
      CREATE(d, DESCRIPTOR_DATA, 1);
      fOld = load_account(d, arg1, TRUE);
      if (!fOld)
      {
         send_to_char("That account does not exist.\n\r", ch);
         flush_account_d(d);
      }   
      pwdnew = crypt( argument, d->account->name );
      for ( p = pwdnew; *p != '\0'; p++ )
      {
         if ( *p == '~' )
	 {
	    send_to_char( "Passwords cannot use &-\n\r", ch );
	    flush_account_d(d);
	    return;
	 }
      }
      STRFREE(d->account->passwd);
      d->account->passwd = STRALLOC(crypt(argument, d->account->name));
      save_account(d, 0);
      flush_account_d(d);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "name"))
   {
      argument = one_argument(argument, arg3);
      if (!check_parse_name(arg3, 1))
      {
         send_to_char("That name is invalid, try another.\n\r", ch);
         return;
      }
      if (!str_cmp(arg3, arg1))
      {
         send_to_char("You have to choose a new name silly.\n\r", ch);
         return;
      }  
      CREATE(d, DESCRIPTOR_DATA, 1);
      fOld = load_account(d, arg3, TRUE);
      if (fOld)
      {
         send_to_char("That account name is already in use.\n\r", ch);
         flush_account_d(d);
         return;
      }
      flush_account_d(d);
      CREATE(d, DESCRIPTOR_DATA, 1);
      fOld = load_account(d, arg1, TRUE);
      if (!fOld)
      {
         send_to_char("That account does not exist.\n\r", ch);
         flush_account_d(d);
         return;
      }
      if ( strlen(argument) < 5 )
      {
         send_to_char("Password must be at least five characters long.\n\r", ch );
         flush_account_d(d);
         return;
      }
      //Password won't work without this, ha ha, evil crypt
      pwdnew = crypt( argument, capitalize(arg3) );
      for ( p = pwdnew; *p != '\0'; p++ )
      {
         if ( *p == '~' )
	 {
	    send_to_char( "You must first change your password!\n\r", ch );
	    flush_account_d(d);
	    return;
	 }
      }
      STRFREE(d->account->passwd);
      d->account->passwd = STRALLOC(crypt(argument, capitalize(arg3)));
      STRFREE(d->account->name);
      d->account->name = STRALLOC(capitalize(arg3));
      save_account(d, 0);
      flush_account_d(d);
      sprintf(buf, "%s%c/%s", ACCOUNT_DIR, tolower(arg1[0]), capitalize(arg1));   
      remove(buf);
      send_to_char("Done.\n\r", ch);
      return;
   }
      
   if (!str_cmp(arg2, "disconnect"))
   {
      int dcnt = 0;
      
      for (d = first_descriptor; d; d = dnext)
      {
         dnext = d->next;
         if (d->account && !str_cmp(d->account->name, arg1))
         {
            if (d->character && d->character == ch)
               continue;
            if (d->character && get_trust(ch) <= get_trust(d->character))
            {
               ch_printf(ch, "Could not disconnect %s, above or at your level.\n\r", d->character->name);
               continue;
            }
            sprintf(buf, "%s is disconnecting your account from Rafermand, goodbye.\n\r", ch->name);
            write_to_buffer(d, buf, 0);
            close_socket(d, FALSE);
            dcnt++;
         }
      }
      if (dcnt >= 1)
         send_to_char("Everyone from that account is disconnected.\n\r", ch);
      else
         send_to_char("There was no one to disconnect from that account!\n\r", ch);
      return;
   }
         
   if (!str_cmp(arg2, "changes"))
   {
      if (atoi(argument) < 0 || atoi(argument) > sysdata.max_account_changes)
      {
         ch_printf(ch, "Range is %d to %d.\n\r", 0, sysdata.max_account_changes);
         return;
      }
      CREATE(d, DESCRIPTOR_DATA, 1);
      fOld = load_account(d, arg1, TRUE);
      if (!fOld)
      {
         send_to_char("That account does not exist.\n\r", ch);
         flush_account_d(d);
      }
      d->account->changes = atoi(argument);
      save_account(d, 0);
      flush_account_d(d);
      send_to_char("Done.\n\r", ch);
      return;
   }
      
   if (!str_cmp(arg2, "editing"))
   {
      if (!str_cmp(argument, "off"))
      {
         CREATE(d, DESCRIPTOR_DATA, 1);
         fOld = load_account(d, arg1, TRUE);
         if (!fOld)
         {
            send_to_char("That account does not exist.\n\r", ch);
            flush_account_d(d);
         }  
         d->account->editing = 0;
         save_account(d, 0);
         flush_account_d(d);
         send_to_char("Done.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("You can only turn editing off by typing off.\n\r", ch);
         return;
      }
   }
   if (arg2[0] == '\0')
   {
      CREATE(d, DESCRIPTOR_DATA, 1);
      fOld = load_account(d, arg1, TRUE);
      if (!fOld)
      {
         send_to_char("That account does not exist.\n\r", ch);
         flush_account_d(d);
         return;
      }
      ch_printf(ch, "Name:     %s\n\r", d->account->name);
      ch_printf(ch, "Email:    %s\n\r", d->account->email);
      ch_printf(ch, "Editing:  %d\n\r", d->account->editing);
      ch_printf(ch, "Changes:  %d\n\r", d->account->changes);
      ch_printf(ch, "Banned:   %s\n\r", d->account->ban ? "Yes" : "No");
      for (aname = d->account->first_player; aname; aname = aname->next)
      {
         ch_printf(ch, "Player:   %s\n\r", aname->name);
      }
      flush_account_d(d);
      return;
   }  
}

void do_reward(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   char arg1[MIL];
   char arg2[MIL];
   
   if (check_npc(ch))
      return;
      
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: reward <target> <give/take> <qps>\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("Your target is not valid.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on npcs.\n\r", ch);
      return;
   }
   if (atoi(argument) < 1 || atoi(argument) > 99999)
   {
      send_to_char("Range is 1 to 99,999.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg2, "give"))
   {
      victim->pcdata->reward_curr += atoi(argument);
      victim->pcdata->reward_accum += atoi(argument);
      if (victim->pcdata->reward_curr > 99999)
         victim->pcdata->reward_curr = 99999;
      if (victim->pcdata->reward_accum > 99999)
         victim->pcdata->reward_accum = 99999;
      ch_printf(victim, "You are reward %d Reward Points from %s\n\r", atoi(argument),  PERS_MAP(ch, victim));
      ch_printf(ch, "You reward %d Reward Points to %s\n\r", atoi(argument), PERS_MAP(victim, ch));
      return;
   }
   if (!str_cmp(arg2, "take"))
   {
      victim->pcdata->reward_curr -= atoi(argument);
      if (victim->pcdata->reward_curr < 0)
         victim->pcdata->reward_curr = 0;
      ch_printf(victim, "You are docked %d Reward Points from %s\n\r", atoi(argument),  PERS_MAP(ch, victim));
      ch_printf(ch, "You docked %d Reward Points to %s\n\r", atoi(argument), PERS_MAP(victim, ch));
      return;
   }   
   do_reward(ch, "");
   return;
}
   
   

void do_users(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   DESCRIPTOR_DATA *d;
   int count;
   char arg[MIL];

   set_pager_color(AT_PLAIN, ch);

   one_argument(argument, arg);
   count = 0;
   buf[0] = '\0';

   sprintf(buf, "\n\rDesc|Con|Idle| Port | Player      @HostIP                              ");
   if (get_trust(ch) >= LEVEL_HI_IMM) /* Tracker1 */
      strcat(buf, "| Username");
   strcat(buf, "\n\r");
   strcat(buf, "----+---+----+------+--------------------------------------------------");
   if (get_trust(ch) >= LEVEL_HI_IMM) /* Tracker1 */
      strcat(buf, "+---------");
   strcat(buf, "\n\r");
   send_to_pager(buf, ch);

   for (d = first_descriptor; d; d = d->next)
   {
      if (arg[0] == '\0')
      {
         if (get_trust(ch) >= LEVEL_ADMIN /* Tracker1 */
            || (d->character && can_see_map(ch, d->character)))
         {
            count++;
            sprintf(buf,
               " %3d| %2d|%4d|%6d| %-12s@%-35s ",
               d->descriptor,
               d->connected, d->idle / 4, d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)", d->host);
            if (get_trust(ch) >= LEVEL_HI_IMM) /* Tracker1 */
               sprintf(buf + strlen(buf), "| %s", d->user);
            strcat(buf, "\n\r");
            send_to_pager(buf, ch);
         }
      }
      else
      {
         if ((get_trust(ch) >= LEVEL_ADMIN /* Tracker1 */
               || (d->character && can_see_map(ch, d->character)))
            && (!str_prefix(arg, d->host) || (d->character && !str_prefix(arg, d->character->name))))
         {
            count++;
            pager_printf(ch,
               " %3d| %2d|%4d|%6d| %-12s@%-35s ",
               d->descriptor,
               d->connected, d->idle / 4, d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)", d->host);
            buf[0] = '\0';
            if (get_trust(ch) >= LEVEL_HI_IMM) /* Tracker1 */
               sprintf(buf, "| %s", d->user);
            strcat(buf, "\n\r");
            send_to_pager(buf, ch);
         }
      }
   }
   pager_printf(ch, "%d user%s.\n\r", count, count == 1 ? "" : "s");
   return;
}

/*
 *  "Clones" immortal command
 *  Author: Cronel (supfly@geocities.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */

void do_clones(CHAR_DATA * ch, char *argument)
{
   DESCRIPTOR_DATA *dsrc, *ddst, *dsrc_next, *ddst_next;
   DESCRIPTOR_DATA *dlistf, *dlistl;
   sh_int clone_count;

   set_pager_color(AT_PLAIN, ch);
   pager_printf(ch, " %-12.12s | %-12.12s | %-s\n\r", "characters", "user", "host");
   pager_printf(ch, "--------------+--------------+---------------------------------------------\n\r");

   dlistf = dlistl = NULL;

   for (dsrc = first_descriptor; dsrc; dsrc = dsrc_next)
   {
      if ((dsrc->character && !can_see_map(ch, dsrc->character)) || !dsrc->user || !dsrc->host)
      {
         dsrc_next = dsrc->next;
         continue;
      }

      pager_printf(ch, " %-12.12s |", dsrc->original ? dsrc->original->name : (dsrc->character ? dsrc->character->name : "(No name)"));
      clone_count = 1;

      for (ddst = first_descriptor; ddst; ddst = ddst_next)
      {
         ddst_next = ddst->next;

         if (dsrc == ddst)
            continue;
         if ((ddst->character && !can_see_map(ch, ddst->character)) || !ddst->user || !ddst->host)
            continue;

         if (!str_cmp(dsrc->user, ddst->user) && !str_cmp(dsrc->host, ddst->host))
         {
            UNLINK(ddst, first_descriptor, last_descriptor, next, prev);
            LINK(ddst, dlistf, dlistl, next, prev);
            pager_printf(ch, "              |\n\r %-12.12s |",
               ddst->original ? ddst->original->name : (ddst->character ? ddst->character->name : "(No name)"));
            clone_count++;
         }
      }

      pager_printf(ch, " %-12.12s | %s (%d clone%s)\n\r", dsrc->user, dsrc->host, clone_count, clone_count > 1 ? "s" : "");

      dsrc_next = dsrc->next;

      UNLINK(dsrc, first_descriptor, last_descriptor, next, prev);
      LINK(dsrc, dlistf, dlistl, next, prev);
   }


   for (dsrc = dlistf; dsrc; dsrc = dsrc_next)
   {
      dsrc_next = dsrc->next;
      UNLINK(dsrc, dlistf, dlistl, next, prev);
      LINK(dsrc, first_descriptor, last_descriptor, next, prev);
   }
}

/*
 *  Sslist immortal command
 *  Author: Cronel (cronel_kal@hotmail.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */

void do_sslist(CHAR_DATA * ch, char *argument)
{
   return;
}


/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   bool mobsonly;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Force whom to do what?\n\r", ch);
      return;
   }

   mobsonly = get_trust(ch) < sysdata.level_forcepc;

   if (!str_cmp(arg, "all"))
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if (mobsonly)
      {
         send_to_char("Force whom to do what?\n\r", ch);
         return;
      }

      for (vch = first_char; vch; vch = vch_next)
      {
         vch_next = vch->next;

         if (!IS_NPC(vch) && get_trust(vch) < get_trust(ch))
         {
            act(AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT);
            interpret(vch, argument);
         }
      }
   }
   else
   {
      CHAR_DATA *victim;

      if ((victim = get_char_world(ch, arg)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }

      if (victim == ch)
      {
         send_to_char("Aye aye, right away!\n\r", ch);
         return;
      }

      if ((get_trust(victim) >= get_trust(ch)) || (mobsonly && !IS_NPC(victim)))
      {
         send_to_char("Do it yourself!\n\r", ch);
         return;
      }

      if (get_trust(ch) < LEVEL_HI_IMM && IS_NPC(victim) /* Tracker1 */
         && !str_prefix("mp", argument))
      {
         send_to_char("You can't force a mob to do that!\n\r", ch);
         return;
      }
      if (IS_NPC(victim) && (xIS_SET(victim->act, ACT_PROTECT)) && (get_trust(ch) < LEVEL_STAFF))
      {
         send_to_char("This mob is protected against forces, transfers, slays, and purges\n\r", ch);
         return;
      }

      act(AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT);
      if (!str_cmp(argument, "quit")) /* Rantic's info channel */
         char_quit(victim, FALSE);
      else
         interpret(victim, argument);
   }

   send_to_char("Ok.\n\r", ch);
   return;
}

void do_invis(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   sh_int level;

   set_char_color(AT_IMMORT, ch);

   /* if ( IS_NPC(ch)) return; */

   argument = one_argument(argument, arg);
   if (arg && arg[0] != '\0')
   {
      if (!is_number(arg) && str_cmp(arg, "off"))
      {
         send_to_char("Usage: invis | invis <level> | invis off\n\r", ch);
         return;
      }
      if (!str_cmp(arg, "off"))
      {
         xREMOVE_BIT(ch->act, PLR_WIZINVIS);
         act(AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
         send_to_char("You slowly fade back into existence.\n\r", ch);
         return;
      } 
      level = atoi(arg);
      if (level <= LEVEL_PC || level > get_trust(ch))
      {
         send_to_char("Invalid level.\n\r", ch);
         return;
      }

      if (!IS_NPC(ch))
      {
         ch->pcdata->wizinvis = level;
         ch_printf(ch, "Wizinvis level set to %d.\n\r", level);
      }

      if (IS_NPC(ch))
      {
         ch->mobinvis = level;
         ch_printf(ch, "Mobinvis level set to %d.\n\r", level);
      }
      return;
   }  
   if (!IS_NPC(ch))
   {
      if (ch->pcdata->wizinvis <= LEVEL_PC)
         ch->pcdata->wizinvis = ch->level;
   }
   if (IS_NPC(ch))
   {
      if (ch->mobinvis <= LEVEL_PC)
         ch->mobinvis = ch->level;
   }
   act(AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
   send_to_char("You slowly vanish into thin air.\n\r", ch);
   xSET_BIT(ch->act, PLR_WIZINVIS);
   return;
}


void do_holylight(CHAR_DATA * ch, char *argument)
{

   set_char_color(AT_IMMORT, ch);

   if (IS_NPC(ch))
      return;

   if (xIS_SET(ch->act, PLR_HOLYLIGHT))
   {
      xREMOVE_BIT(ch->act, PLR_HOLYLIGHT);
      send_to_char("Holy light mode off.\n\r", ch);
   }
   else
   {
      xSET_BIT(ch->act, PLR_HOLYLIGHT);
      send_to_char("Holy light mode on.\n\r", ch);
   }
   return;
}


/* Consolidated *assign function. 
 * Assigns room/obj/mob ranges and initializes new zone - Samson 2-12-99 
 */
void do_vassign(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   int lo, hi;
   CHAR_DATA *victim, *mob;
   ROOM_INDEX_DATA *room;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   AREA_DATA *tarea;
   char filename[256];

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   lo = atoi(arg2);
   hi = atoi(arg3);

   if (arg1[0] == '\0' || lo < 0 || hi < 0)
   {
      send_to_char("Syntax: vassign <who> <low> <high>\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("They don't seem to be around.\n\r", ch);
      return;
   }
   if (IS_NPC(victim) || get_trust(victim) < LEVEL_IMM)
   {
      send_to_char("They wouldn't know what to do with a vnum range.\n\r", ch);
      return;
   }
   if (victim->pcdata->area && lo != 0)
   {
      send_to_char("You cannot assign them a range, they already have one!\n\r", ch);
      return;
   }
   if (lo > hi)
   {
      send_to_char("Unacceptable vnum range.\n\r", ch);
      return;
   }
   if (hi >= MAX_IMM_VNUM)
   {
      ch_printf(ch, "All areas must stay below %d in vnum.  Anything else above is reserved for the system.\n\r", ch);
      return;
   }
   if (lo == 0)
      hi = 0;
   victim->pcdata->r_range_lo = lo;
   victim->pcdata->r_range_hi = hi;
   victim->pcdata->o_range_lo = lo;
   victim->pcdata->o_range_hi = hi;
   victim->pcdata->m_range_lo = lo;
   victim->pcdata->m_range_hi = hi;
   assign_area(victim);
   send_to_char("Done.\n\r", ch);
   ch_printf(victim, "%s has assigned you the vnum range %d - %d.\n\r", ch->name, lo, hi);
   assign_area(victim); /* Put back by Thoric on 02/07/96 */

   if (!victim->pcdata->area)
   {
      bug("vassign: assign_area failed", 0);
      return;
   }

   tarea = victim->pcdata->area;

   if (lo == 0) /* Scryn 8/12/95 */
   {
      REMOVE_BIT(tarea->status, AREA_LOADED);
      SET_BIT(tarea->status, AREA_DELETED);
   }
   else
   {
      SET_BIT(tarea->status, AREA_LOADED);
      REMOVE_BIT(tarea->status, AREA_DELETED);
   }

   /* Initialize first and last rooms in range */
   room = make_room(lo);
   if (!room)
   {
      bug("do_vassign: make_room failed to initialize.", 0);
      return;
   }
   room->area = tarea;

   room = make_room(hi);
   if (!room)
   {
      bug("do_vassign: make_room failed to initialize.", 0);
      return;
   }
   room->area = tarea;

   /* Initialize last mob in range */
   pMobIndex = make_mobile(hi, 0, "last mob");
   if (!pMobIndex)
   {
      log_string("do_vassign: make_mobile failed to initialize.");
      return;
   }
   mob = create_mobile(pMobIndex);
   char_to_room(mob, room);

   /* Initialize last obj in range */
   pObjIndex = make_object(hi, 0, "last obj", 0);
   if (!pObjIndex)
   {
      log_string("do_vassign: make_object failed to initialize.");
      return;
   }
   obj = create_object(pObjIndex, 0);
   obj_to_room(obj, room, ch);

   /* Save character and newly created zone */
   save_char_obj(victim);

   if (!IS_SET(tarea->status, AREA_DELETED))
   {
      sprintf(filename, "%s%s", BUILD_DIR, tarea->filename);
      fold_area(tarea, filename, FALSE, 0);
   }

   set_char_color(AT_IMMORT, ch);
   ch_printf(ch, "Vnum range set for %s and initialized.\n\r", victim->name);

   return;
}

void do_rassign(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   sh_int r_lo, r_hi;
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   r_lo = atoi(arg2);
   r_hi = atoi(arg3);

   if (arg1[0] == '\0' || r_lo < 0 || r_hi < 0)
   {
      send_to_char("Syntax: rassign <who> <low> <high>\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("They don't seem to be around.\n\r", ch);
      return;
   }
   if (IS_NPC(victim) || get_trust(victim) < LEVEL_IMM) /* Tracker1 */
   {
      send_to_char("They wouldn't know what to do with a room range.\n\r", ch);
      return;
   }
   if (r_lo > r_hi)
   {
      send_to_char("Unacceptable room range.\n\r", ch);
      return;
   }
   if (r_lo == 0)
      r_hi = 0;
   victim->pcdata->r_range_lo = r_lo;
   victim->pcdata->r_range_hi = r_hi;
   assign_area(victim);
   send_to_char("Done.\n\r", ch);
   set_char_color(AT_IMMORT, victim);
   ch_printf(victim, "%s has assigned you the room vnum range %d - %d.\n\r", ch->name, r_lo, r_hi);
   assign_area(victim); /* Put back by Thoric on 02/07/96 */
   if (!victim->pcdata->area)
   {
      bug("rassign: assign_area failed", 0);
      return;
   }

   if (r_lo == 0) /* Scryn 8/12/95 */
   {
      REMOVE_BIT(victim->pcdata->area->status, AREA_LOADED);
      SET_BIT(victim->pcdata->area->status, AREA_DELETED);
   }
   else
   {
      SET_BIT(victim->pcdata->area->status, AREA_LOADED);
      REMOVE_BIT(victim->pcdata->area->status, AREA_DELETED);
   }
   return;
}

void do_oassign(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   sh_int o_lo, o_hi;
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   o_lo = atoi(arg2);
   o_hi = atoi(arg3);

   if (arg1[0] == '\0' || o_lo < 0 || o_hi < 0)
   {
      send_to_char("Syntax: oassign <who> <low> <high>\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("They don't seem to be around.\n\r", ch);
      return;
   }
   if (IS_NPC(victim) || get_trust(victim) < LEVEL_IMM) /* Tracker1 */
   {
      send_to_char("They wouldn't know what to do with an object range.\n\r", ch);
      return;
   }
   if (o_lo > o_hi)
   {
      send_to_char("Unacceptable object range.\n\r", ch);
      return;
   }
   victim->pcdata->o_range_lo = o_lo;
   victim->pcdata->o_range_hi = o_hi;
   assign_area(victim);
   send_to_char("Done.\n\r", ch);
   set_char_color(AT_IMMORT, victim);
   ch_printf(victim, "%s has assigned you the object vnum range %d - %d.\n\r", ch->name, o_lo, o_hi);
   return;
}

void do_massign(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   sh_int m_lo, m_hi;
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   m_lo = atoi(arg2);
   m_hi = atoi(arg3);

   if (arg1[0] == '\0' || m_lo < 0 || m_hi < 0)
   {
      send_to_char("Syntax: massign <who> <low> <high>\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("They don't seem to be around.\n\r", ch);
      return;
   }
   if (IS_NPC(victim) || get_trust(victim) < LEVEL_IMM) /* Tracker */
   {
      send_to_char("They wouldn't know what to do with a monster range.\n\r", ch);
      return;
   }
   if (m_lo > m_hi)
   {
      send_to_char("Unacceptable monster range.\n\r", ch);
      return;
   }
   victim->pcdata->m_range_lo = m_lo;
   victim->pcdata->m_range_hi = m_hi;
   assign_area(victim);
   send_to_char("Done.\n\r", ch);
   set_char_color(AT_IMMORT, victim);
   ch_printf(victim, "%s has assigned you the monster vnum range %d - %d.\n\r", ch->name, m_lo, m_hi);
   return;
}

void do_cmdtable(CHAR_DATA * ch, char *argument)
{
   int hash, cnt;
   CMDTYPE *cmd;
   char arg[MIL];

   one_argument(argument, arg);

   if (strcmp(arg, "lag")) /* display normal command table */
   {
      set_pager_color(AT_IMMORT, ch);
      send_to_pager("Commands and Number of Uses This Run\n\r", ch);
      set_pager_color(AT_PLAIN, ch);
      for (cnt = hash = 0; hash < 126; hash++)
         for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
         {
            if ((++cnt) % 4)
               pager_printf(ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses);
            else
               pager_printf(ch, "%-6.6s %4d\n\r", cmd->name, cmd->userec.num_uses);
         }
      send_to_char("\n\r", ch);
   }
   else /* display commands causing lag */
   {
      set_pager_color(AT_IMMORT, ch);
      send_to_pager("Commands that have caused lag this run\n\r", ch);
      set_pager_color(AT_PLAIN, ch);
      for (cnt = hash = 0; hash < 126; hash++)
         for (cmd = command_hash[hash]; cmd; cmd = cmd->next)
         {
            if (!cmd->lag_count)
               continue;
            else if ((++cnt) % 4)
               pager_printf(ch, "%-6.6s %4d\t", cmd->name, cmd->lag_count);
            else
               pager_printf(ch, "%-6.6s %4d\n\r", cmd->name, cmd->lag_count);
         }
      send_to_char("\n\r", ch);
   }

   return;
}

void do_mortalize(CHAR_DATA * ch, char *argument)
{
   char fname[1024];
   char name[256];
   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MSL];
   char buf2[MSL];
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, name);
   if (name[0] == '\0')
   {
      send_to_char("Usage: mortalize <playername>\n\r", ch);
      return;
   }

   name[0] = UPPER(name[0]);
   sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]), capitalize(name));
   if (stat(fname, &fst) != -1)
   {
      CREATE(d, DESCRIPTOR_DATA, 1);
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE(d->outbuf, char, d->outsize);

      loaded = load_char_obj(d, name, FALSE);
      add_char(d->character);
      old_room_vnum = d->character->in_room->vnum;
      char_to_room(d->character, ch->in_room);
      if (get_trust(d->character) >= get_trust(ch))
      {
         do_say(d->character, "Do *NOT* disturb me again!");
         send_to_char("I think you'd better leave that player alone!\n\r", ch);
         d->character->desc = NULL;
         char_quit(d->character, FALSE); /* Rantic's info channel */
         return;
      }
      d->character->desc = NULL;
      victim = d->character;
      d->character = NULL;
      DISPOSE(d->outbuf);
      DISPOSE(d);
      victim->level = LEVEL_PC;
      victim->max_hit = 20;
      victim->max_mana = 20;
      for (sn = 0; sn < top_sn; sn++)
         victim->pcdata->learned[sn] = 0;
      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      DISPOSE(victim->pcdata->rank);
      victim->pcdata->rank = str_dup("");
      if (xIS_SET(victim->act, PLR_WIZINVIS))
         victim->pcdata->wizinvis = victim->trust;
      if (xIS_SET(victim->act, PLR_WIZINVIS) && (victim->level <= LEVEL_PC))
      {
         xREMOVE_BIT(victim->act, PLR_WIZINVIS);
         victim->pcdata->wizinvis = victim->trust;
      }
      sprintf(buf, "%s%s", GOD_DIR, capitalize(victim->name));

      if (!remove(buf))
         send_to_char("Player's immortal data destroyed.\n\r", ch);
      else if (errno != ENOENT)
      {
         ch_printf(ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\n\r", errno, strerror(errno));
         sprintf(buf2, "%s mortalizing %s", ch->name, buf);
         perror(buf2);
      }
      sprintf(buf2, "%s.are", capitalize(argument));
      for (pArea = first_build; pArea; pArea = pArea->next)
         if (!strcmp(pArea->filename, buf2))
         {
            sprintf(buf, "%s%s", BUILD_DIR, buf2);
            if (IS_SET(pArea->status, AREA_LOADED))
               fold_area(pArea, buf, FALSE, 0);
            close_area(pArea);
            sprintf(buf2, "%s.bak", buf);
            set_char_color(AT_RED, ch);
            if (!rename(buf, buf2))
               send_to_char("Player's area data destroyed.  Area saved as backup.\n\r", ch);
            else if (errno != ENOENT)
            {
               ch_printf(ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r", errno, strerror(errno));
               sprintf(buf2, "%s mortalizing %s", ch->name, buf);
               perror(buf2);
            }
         }
      make_wizlist();
      while (victim->first_carrying)
         extract_obj(victim->first_carrying);
      char_quit(victim, FALSE);
      return;
   }
   send_to_char("No such player.\n\r", ch);
   return;
}

/*
 * Load up a player file
 */
void do_loadup(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *temp;
   char fname[1024];
   char name[256];
   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum, old_x, old_y, old_map;
   char buf[MSL];

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, name);
   if (name[0] == '\0')
   {
      send_to_char("Usage: loadup <playername>\n\r", ch);
      return;
   }
   for (temp = first_char; temp; temp = temp->next)
   {
      if (IS_NPC(temp))
         continue;
      if (can_see_map(ch, temp) && !str_cmp(name, temp->name))
         break;
   }
   if (temp != NULL)
   {
      send_to_char("They are already playing.\n\r", ch);
      return;
   }
   name[0] = UPPER(name[0]);
   sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(name[0]), capitalize(name));

   if (stat(fname, &fst) != -1)
   {
      CREATE(d, DESCRIPTOR_DATA, 1);
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE(d->outbuf, char, d->outsize);

      loaded = load_char_obj(d, name, FALSE);
      add_char(d->character);
      old_room_vnum = d->character->in_room->vnum;
      old_x = d->character->coord->x;
      old_y = d->character->coord->y;
      old_map = d->character->map;
      char_to_room(d->character, ch->in_room);
      if (get_trust(d->character) >= get_trust(ch))
      {
         do_say(d->character, "Do *NOT* disturb me again!");
         send_to_char("I think you'd better leave that player alone!\n\r", ch);
         d->character->desc = NULL;
         char_quit(d->character, FALSE);
         return;
      }
      d->character->desc = NULL;
      d->character->retran = old_room_vnum;
      d->character->retran_x = old_x;
      d->character->retran_y = old_y;
      d->character->retran_map = old_map;
      d->character = NULL;
      DISPOSE(d->outbuf);
      DISPOSE(d);
      ch_printf(ch, "Player %s loaded from room %d.\n\r", capitalize(name), old_room_vnum);
      sprintf(buf, "%s appears from nowhere, eyes glazed over.\n\r", capitalize(name));
      act(AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM);
      send_to_char("Done.\n\r", ch);
      return;
   }
   /* else no player file */
   send_to_char("No such player.\n\r", ch);
   return;
}

void do_fixchar(CHAR_DATA * ch, char *argument)
{
   char name[MSL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   one_argument(argument, name);
   if (name[0] == '\0')
   {
      send_to_char("Usage: fixchar <playername>\n\r", ch);
      return;
   }

   victim = get_char_room_new(ch, name, 1);
   if (!victim)
   {
      send_to_char("They're not here.\n\r", ch);
      return;
   }
   fix_char(victim);
/*  victim->armor	= 100;
    victim->mod_str	= 0;
    victim->mod_dex	= 0;
    victim->mod_wis	= 0;
    victim->mod_int	= 0;
    victim->mod_con	= 0;
    victim->mod_cha	= 0;
    victim->mod_lck	= 0;
    victim->damroll	= 0;
    victim->hitroll	= 0;
    victim->alignment	= URANGE( -1000, victim->alignment, 1000 );
    victim->saving_spell_staff = 0; */
   send_to_char("Done.\n\r", ch);
}

/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names(char *inp, char *out)
{
   char buf[MIL], *pbuf = buf;
   int len;

   *out = '\0';
   while (inp && *inp)
   {
      inp = one_argument(inp, buf);
      if ((len = strlen(buf)) >= 5 && !strcmp(".are", pbuf + len - 4))
      {
         if (*out)
            strcat(out, " ");
         strcat(out, buf);
      }
   }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names(char *inp, char *out)
{
   char buf[MIL], *pbuf = buf;
   int len;

   *out = '\0';
   while (inp && *inp)
   {
      inp = one_argument(inp, buf);
      if ((len = strlen(buf)) < 5 || strcmp(".are", pbuf + len - 4))
      {
         if (*out)
            strcat(out, " ");
         strcat(out, buf);
      }
   }
}

/*
 * Allows members of the Area Council to add Area names to the bestow field.
 * Area names mus end with ".are" so that no commands can be bestowed.
 */
void do_bestowarea(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MSL];
   CHAR_DATA *victim;
   int arg_len;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);

   if (!*arg)
   {
      send_to_char("Syntax:\n\r"
         "bestowarea <victim> <filename>.are\n\r"
         "bestowarea <victim> none             removes bestowed areas\n\r"
         "bestowarea <victim> list             lists bestowed areas\n\r" "bestowarea <victim>                  lists bestowed areas\n\r", ch);
      return;
   }
   if (!(victim = get_char_world(ch, arg)))
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You can't give special abilities to a mob!\n\r", ch);
      return;
   }
   if (get_trust(victim) < LEVEL_IMMORTAL)
   {
      send_to_char("They aren't an immortal.\n\r", ch);
      return;
   }

   if (!victim->pcdata->bestowments)
      victim->pcdata->bestowments = str_dup("");

   if (!*argument || !str_cmp(argument, "list"))
   {
      extract_area_names(victim->pcdata->bestowments, buf);
      ch_printf(ch, "Bestowed areas: %s\n\r", buf);
      return;
   }
   if (!str_cmp(argument, "none"))
   {
      remove_area_names(victim->pcdata->bestowments, buf);
      DISPOSE(victim->pcdata->bestowments);
      victim->pcdata->bestowments = str_dup(buf);
      send_to_char("Done.\n\r", ch);
      return;
   }

   arg_len = strlen(argument);
   if (arg_len < 5 || argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a' || argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e')
   {
      send_to_char("You can only bestow an area name\n\r", ch);
      send_to_char("E.G. bestow joe sam.are\n\r", ch);
      return;
   }

   sprintf(buf, "%s %s", victim->pcdata->bestowments, argument);
   DISPOSE(victim->pcdata->bestowments);
   victim->pcdata->bestowments = str_dup(buf);
   set_char_color(AT_IMMORT, victim);
   ch_printf(victim, "%s has bestowed on you the area: %s\n\r", ch->name, argument);
   send_to_char("Done.\n\r", ch);
}

void do_bestow(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MSL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Bestow whom with what?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You can't give special abilities to a mob!\n\r", ch);
      return;
   }
   if (get_trust(victim) > get_trust(ch))
   {
      send_to_char("You aren't powerful enough...\n\r", ch);
      return;
   }

   if (!victim->pcdata->bestowments)
      victim->pcdata->bestowments = str_dup("");

   if (argument[0] == '\0' || !str_cmp(argument, "list"))
   {
      ch_printf(ch, "Current bestowed commands on %s: %s.\n\r", victim->name, victim->pcdata->bestowments);
      return;
   }
   if (!str_cmp(argument, "none"))
   {
      DISPOSE(victim->pcdata->bestowments);
      victim->pcdata->bestowments = str_dup("");
      ch_printf(ch, "Bestowments removed from %s.\n\r", victim->name);
      ch_printf(victim, "%s has removed your bestowed commands.\n\r", ch->name);
      return;
   }

   sprintf(buf, "%s %s", victim->pcdata->bestowments, argument);
   DISPOSE(victim->pcdata->bestowments);
   victim->pcdata->bestowments = str_dup(buf);
   set_char_color(AT_IMMORT, victim);
   ch_printf(victim, "%s has bestowed on you the command(s): %s\n\r", ch->name, argument);
   send_to_char("Done.\n\r", ch);
}

struct tm *update_time(struct tm *old_time)
{
   time_t time;

   time = mktime(old_time);
   return localtime(&time);
}

void do_set_boot_time(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg1[MIL];
   bool check;

   check = FALSE;
   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Syntax: setboot time {hour minute <day> <month> <year>}\n\r", ch);
      send_to_char("        setboot copyover <hour> <minutes> [seconds]\n\r", ch);
      send_to_char("        setboot manual {0/1}\n\r", ch);
      send_to_char("        setboot default\n\r", ch);
      ch_printf(ch, "Boot time is currently set to %s, manual bit is set to %d\n\r", reboot_time, set_boot_time->manual);
      return;
   }

   if (!str_cmp(arg, "copyover"))
   {
      struct tm *tptr;
      char *dtime;
      int currtime;
      int hours;
      int minutes;
      int seconds = 0;
      
      argument = one_argument(argument, arg);
      argument = one_argument(argument, arg1);
      if (!*arg || !*arg1 || !is_number(arg) || !is_number(arg1))
      {
         send_to_char("You must input a value for hour and minute.\n\r", ch);
         return;
      }

      currtime = time(0);
      
      if (atoi(arg) < 0 || atoi(arg) > 48)
      {
         send_to_char("Valid range for hour is 0 to 48.\n\r", ch);
         return;
      }
      hours = atoi(arg);
      if (atoi(arg1) < 0 || atoi(arg1) > 59)
      {
         send_to_char("Valid range for minute is 0 to 59.\n\r", ch);
         return;
      }
      minutes = atoi(arg1);
      argument = one_argument(argument, arg);
      if (*arg != '\0' && is_number(arg))
      {
         if (atoi(arg) < 1 && atoi(arg) > 59)
         {
            send_to_char("Range is 1 to 59 for seconds.\n\r", ch);
            return;
         }
         seconds = atoi(arg);
      }
      copyover_time = time(0) + seconds + (minutes*60) + (hours*3600);
      
      tptr = localtime(&copyover_time);
      dtime = asctime(tptr);
      ch_printf(ch, "Copyover time set to %s\n\r", dtime);
      copyover_check();
      return;
   }   

   if (!str_cmp(arg, "time"))
   {
      struct tm *now_time;

      argument = one_argument(argument, arg);
      argument = one_argument(argument, arg1);
      if (!*arg || !*arg1 || !is_number(arg) || !is_number(arg1))
      {
         send_to_char("You must input a value for hour and minute.\n\r", ch);
         return;
      }

      now_time = localtime(&current_time);
      if ((now_time->tm_hour = atoi(arg)) < 0 || now_time->tm_hour > 23)
      {
         send_to_char("Valid range for hour is 0 to 23.\n\r", ch);
         return;
      }
      if ((now_time->tm_min = atoi(arg1)) < 0 || now_time->tm_min > 59)
      {
         send_to_char("Valid range for minute is 0 to 59.\n\r", ch);
         return;
      }

      argument = one_argument(argument, arg);
      if (*arg != '\0' && is_number(arg))
      {
         if ((now_time->tm_mday = atoi(arg)) < 1 || now_time->tm_mday > 31)
         {
            send_to_char("Valid range for day is 1 to 31.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if (*arg != '\0' && is_number(arg))
         {
            if ((now_time->tm_mon = atoi(arg)) < 1 || now_time->tm_mon > 12)
            {
               send_to_char("Valid range for month is 1 to 12.\n\r", ch);
               return;
            }
            now_time->tm_mon--;
            argument = one_argument(argument, arg);
            if ((now_time->tm_year = atoi(arg) - 1900) < 0 || now_time->tm_year > 199)
            {
               send_to_char("Valid range for year is 1900 to 2099.\n\r", ch);
               return;
            }
         }
      }

      now_time->tm_sec = 0;
      if (mktime(now_time) < current_time)
      {
         send_to_char("You can't set a time previous to today!\n\r", ch);
         return;
      }
      if (set_boot_time->manual == 0)
         set_boot_time->manual = 1;
      new_boot_time = update_time(now_time);
      new_boot_struct = *new_boot_time;
      new_boot_time = &new_boot_struct;
      reboot_check(mktime(new_boot_time));
      get_reboot_string();

      ch_printf(ch, "Boot time set to %s\n\r", reboot_time);
      check = TRUE;
   }
   else if (!str_cmp(arg, "manual"))
   {
      argument = one_argument(argument, arg1);
      if (arg1[0] == '\0')
      {
         send_to_char("Please enter a value for manual boot on/off\n\r", ch);
         return;
      }
      if (!is_number(arg1))
      {
         send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
         return;
      }
      if (atoi(arg1) < 0 || atoi(arg1) > 1)
      {
         send_to_char("Value for manual must be 0 (off) or 1 (on)\n\r", ch);
         return;
      }

      set_boot_time->manual = atoi(arg1);
      ch_printf(ch, "Manual bit set to %s\n\r", arg1);
      check = TRUE;
      get_reboot_string();
      return;
   }

   else if (!str_cmp(arg, "default"))
   {
      set_boot_time->manual = 0;
      /* Reinitialize new_boot_time */
      new_boot_time = localtime(&current_time);
      new_boot_time->tm_mday += 1;
      if (new_boot_time->tm_hour > 12)
         new_boot_time->tm_mday += 1;
      new_boot_time->tm_hour = 6;
      new_boot_time->tm_min = 0;
      new_boot_time->tm_sec = 0;
      new_boot_time = update_time(new_boot_time);

      sysdata.DENY_NEW_PLAYERS = FALSE;

      send_to_char("Reboot time set back to normal.\n\r", ch);
      check = TRUE;
   }

   if (!check)
   {
      send_to_char("Invalid argument for setboot.\n\r", ch);
      return;
   }
   else
   {
      get_reboot_string();
      new_boot_time_t = mktime(new_boot_time);
   }
}

/* Online high level immortal command for displaying what the encryption
 * of a name/password would be, taking in 2 arguments - the name and the
 * password - can still only change the password if you have access to 
 * pfiles and the correct password
 */
void do_form_password(CHAR_DATA * ch, char *argument)
{
   char arg[MSL];

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   ch_printf(ch, "Those two arguments encrypted result in:  %s\n\r", crypt(arg, argument));
   return;
}

//Updates skillpoints/spherepoints on a PC
void do_updateskills(CHAR_DATA *ch, char *argument)
{
   int sn;
   int i;
   CHAR_DATA *victim;
   
   if (IS_NPC(ch))
   {
      send_to_char("Sorry.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      if (IS_IMMORTAL(ch))
         send_to_char("Syntax: updateskills <target>\n\r", ch);
      else
         send_to_char("Syntax: updateskills now\n\r", ch);
      return;
   }
   if (IS_IMMORTAL(ch))
   {
      if ((victim = get_char_world(ch, argument)) == NULL)
      {
         send_to_char("There is no such player currently playing.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPCs\n\r", ch);
         return;
      }
   }
   else
      victim = ch;
      
   for (i = 1; i <= MAX_SPHERE; i++)
      victim->pcdata->spherepoints[i] = 0;
   for (i = 1; i <= MAX_GROUP+5; i++)
      victim->pcdata->grouppoints[i] = 0;
   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      if (skill_table[sn]->group[0] > 0)
      {
         if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
         {
            victim->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP] += victim->pcdata->learned[sn];
            victim->pcdata->spherepoints[skill_table[sn]->stype] += victim->pcdata->learned[sn];
         }
         else
         {
            victim->pcdata->grouppoints[skill_table[sn]->group[0]] += victim->pcdata->learned[sn];
            victim->pcdata->spherepoints[skill_table[sn]->stype] += victim->pcdata->learned[sn];
         }
      }
   }
   send_to_char("Done.\n\r", ch);
   return;
}


/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_RED, ch);
   send_to_char("If you want to destroy a character, spell it out!\n\r", ch);
   return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area(AREA_DATA * pArea)
{
   extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
   extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
   extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
   CHAR_DATA *ech;
   CHAR_DATA *ech_next;
   OBJ_DATA *eobj;
   OBJ_DATA *eobj_next;
   int icnt;
   ROOM_INDEX_DATA *rid;
   ROOM_INDEX_DATA *rid_next;
   OBJ_INDEX_DATA *oid;
   OBJ_INDEX_DATA *oid_next;
   MOB_INDEX_DATA *mid;
   MOB_INDEX_DATA *mid_next;
   RESET_DATA *ereset;
   RESET_DATA *ereset_next;
   EXTRA_DESCR_DATA *eed;
   EXTRA_DESCR_DATA *eed_next;
   EXIT_DATA *exit;
   EXIT_DATA *exit_next;
   MPROG_ACT_LIST *mpact;
   MPROG_ACT_LIST *mpact_next;
   MPROG_DATA *mprog;
   MPROG_DATA *mprog_next;
   AFFECT_DATA *paf;
   AFFECT_DATA *paf_next;

   for (ech = first_char; ech; ech = ech_next)
   {
      ech_next = ech->next;

      if (ech->fighting)
         stop_fighting(ech, TRUE);
      if (IS_NPC(ech))
      {
         /* if mob is in area, or part of area. */
         if (URANGE(pArea->low_m_vnum, ech->pIndexData->vnum,
               pArea->hi_m_vnum) == ech->pIndexData->vnum || (ech->in_room && ech->in_room->area == pArea))
            extract_char(ech, TRUE);
         continue;
      }
      if (ech->in_room && ech->in_room->area == pArea)
         do_recall(ech, "");
   }
   for (eobj = first_object; eobj; eobj = eobj_next)
   {
      eobj_next = eobj->next;
      /* if obj is in area, or part of area. */
      if (URANGE(pArea->low_o_vnum, eobj->pIndexData->vnum,
            pArea->hi_o_vnum) == eobj->pIndexData->vnum || (eobj->in_room && eobj->in_room->area == pArea))
         extract_obj(eobj);
   }
   for (icnt = 0; icnt < MAX_KEY_HASH; icnt++)
   {
      for (rid = room_index_hash[icnt]; rid; rid = rid_next)
      {
         rid_next = rid->next;

         for (exit = rid->first_exit; exit; exit = exit_next)
         {
            exit_next = exit->next;
            if (rid->area == pArea || exit->to_room->area == pArea)
            {
               STRFREE(exit->keyword);
               STRFREE(exit->description);
               UNLINK(exit, rid->first_exit, rid->last_exit, next, prev);
               DISPOSE(exit);
               /* Crash bug fix.  I know it could go from the start several times
                * But you CAN NOT iterate over a link-list and DELETE from it or
                * Nasty things can and will happen. --Shaddai 
                */
               exit = rid->first_exit;
            }
         }
         if (rid->area != pArea)
            continue;
         STRFREE(rid->name);
         STRFREE(rid->description);
         if (rid->first_person)
         {
            bug("close_area: room with people #%d", rid->vnum);
            for (ech = rid->first_person; ech; ech = ech_next)
            {
               ech_next = ech->next_in_room;
               if (ech->fighting)
                  stop_fighting(ech, TRUE);
               if (IS_NPC(ech))
                  extract_char(ech, TRUE);
               else
                  do_recall(ech, "");
            }
         }
         if (rid->first_content)
         {
            bug("close_area: room with contents #%d", rid->vnum);
            for (eobj = rid->first_content; eobj; eobj = eobj_next)
            {
               eobj_next = eobj->next_content;
               extract_obj(eobj);
            }
         }
         for (eed = rid->first_extradesc; eed; eed = eed_next)
         {
            eed_next = eed->next;
            STRFREE(eed->keyword);
            STRFREE(eed->description);
            DISPOSE(eed);
         }
         for (mpact = rid->mpact; mpact; mpact = mpact_next)
         {
            mpact_next = mpact->next;
            STRFREE(mpact->buf);
            DISPOSE(mpact);
         }
         for (mprog = rid->mudprogs; mprog; mprog = mprog_next)
         {
            mprog_next = mprog->next;
            STRFREE(mprog->arglist);
            STRFREE(mprog->comlist);
            DISPOSE(mprog);
         }
         if (rid == room_index_hash[icnt])
            room_index_hash[icnt] = rid->next;
         else
         {
            ROOM_INDEX_DATA *trid;

            for (trid = room_index_hash[icnt]; trid; trid = trid->next)
               if (trid->next == rid)
                  break;
            if (!trid)
               bug("Close_area: rid not in hash list %d", rid->vnum);
            else
               trid->next = rid->next;
         }
         DISPOSE(rid);
      }

      for (mid = mob_index_hash[icnt]; mid; mid = mid_next)
      {
         mid_next = mid->next;

         if (mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum)
            continue;

         STRFREE(mid->player_name);
         STRFREE(mid->short_descr);
         STRFREE(mid->long_descr);
         STRFREE(mid->description);
         if (mid->pShop)
         {
            UNLINK(mid->pShop, first_shop, last_shop, next, prev);
            DISPOSE(mid->pShop);
         }
         if (mid->rShop)
         {
            UNLINK(mid->rShop, first_repair, last_repair, next, prev);
            DISPOSE(mid->rShop);
         }
         for (mprog = mid->mudprogs; mprog; mprog = mprog_next)
         {
            mprog_next = mprog->next;
            STRFREE(mprog->arglist);
            STRFREE(mprog->comlist);
            DISPOSE(mprog);
         }
         if (mid == mob_index_hash[icnt])
            mob_index_hash[icnt] = mid->next;
         else
         {
            MOB_INDEX_DATA *tmid;

            for (tmid = mob_index_hash[icnt]; tmid; tmid = tmid->next)
               if (tmid->next == mid)
                  break;
            if (!tmid)
               bug("Close_area: mid not in hash list %d", mid->vnum);
            else
               tmid->next = mid->next;
         }
         DISPOSE(mid);
      }

      for (oid = obj_index_hash[icnt]; oid; oid = oid_next)
      {
         oid_next = oid->next;

         if (oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum)
            continue;

         STRFREE(oid->name);
         STRFREE(oid->short_descr);
         STRFREE(oid->description);
         STRFREE(oid->action_desc);

         for (eed = oid->first_extradesc; eed; eed = eed_next)
         {
            eed_next = eed->next;
            STRFREE(eed->keyword);
            STRFREE(eed->description);
            DISPOSE(eed);
         }
         for (paf = oid->first_affect; paf; paf = paf_next)
         {
            paf_next = paf->next;
            DISPOSE(paf);
         }
         for (mprog = oid->mudprogs; mprog; mprog = mprog_next)
         {
            mprog_next = mprog->next;
            STRFREE(mprog->arglist);
            STRFREE(mprog->comlist);
            DISPOSE(mprog);
         }
         if (oid == obj_index_hash[icnt])
            obj_index_hash[icnt] = oid->next;
         else
         {
            OBJ_INDEX_DATA *toid;

            for (toid = obj_index_hash[icnt]; toid; toid = toid->next)
               if (toid->next == oid)
                  break;
            if (!toid)
               bug("Close_area: oid not in hash list %d", oid->vnum);
            else
               toid->next = oid->next;
         }
         DISPOSE(oid);
      }
   }
   for (ereset = pArea->first_reset; ereset; ereset = ereset_next)
   {
      ereset_next = ereset->next;
      DISPOSE(ereset);
   }
   DISPOSE(pArea->name);
   DISPOSE(pArea->filename);
   STRFREE(pArea->author);
   UNLINK(pArea, first_build, last_build, next, prev);
   UNLINK(pArea, first_asort, last_asort, next_sort, prev_sort);
   DISPOSE(pArea);
}

void do_destroy(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char buf[MSL];
   char buf2[MSL];
   char arg[MIL];
   char *name;

   set_char_color(AT_RED, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Destroy what player file?\n\r", ch);
      return;
   }

   for (victim = first_char; victim; victim = victim->next)
      if (!IS_NPC(victim) && !str_cmp(victim->name, arg))
         break;

   if (!victim)
   {
      send_to_char("You can only destroy a player that is online.\n\r", ch);
      return;
   }
   else
   {
      int x, y;

      if (victim->last_name && victim->name)
      {
         remove_from_lastname_file(victim->last_name, victim->name); //remove the player from the lastname file
      }
      else
         bug("do_destroy: That player's lastname could not be removed, need to do it manually");
            
      quitting_char = victim;
      save_char_obj(victim);
      saving_char = NULL;
      extract_char(victim, TRUE);
      for (x = 0; x < MAX_WEAR; x++)
         for (y = 0; y < MAX_LAYERS; y++)
            save_equipment[x][y] = NULL;
   }

   name = capitalize(arg);
   sprintf(buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name);
   sprintf(buf2, "%s%c/%s", BACKUP_DIR, tolower(arg[0]), name);
   if (!rename(buf, buf2))
   {
      AREA_DATA *pArea;

      set_char_color(AT_RED, ch);
      ch_printf(ch, "Player %s destroyed.  Pfile saved in backup directory.\n\r", name);
      sprintf(buf, "%s%s", GOD_DIR, name);
      if (!remove(buf))
         send_to_char("Player's immortal data destroyed.\n\r", ch);
      else if (errno != ENOENT)
      {
         ch_printf(ch, "Unknown error #%d - %s (immortal data).  Report to Thoric.\n\r", errno, strerror(errno));
         sprintf(buf2, "%s destroying %s", ch->name, buf);
         perror(buf2);
      }

      sprintf(buf2, "%s.are", name);
      for (pArea = first_build; pArea; pArea = pArea->next)
         if (!str_cmp(pArea->filename, buf2))
         {
            sprintf(buf, "%s%s", BUILD_DIR, buf2);
            if (IS_SET(pArea->status, AREA_LOADED))
               fold_area(pArea, buf, FALSE, 0);
            close_area(pArea);
            sprintf(buf2, "%s.bak", buf);
            set_char_color(AT_RED, ch); /* Log message changes colors */
            if (!rename(buf, buf2))
               send_to_char("Player's area data destroyed.  Area saved as backup.\n\r", ch);
            else if (errno != ENOENT)
            {
               ch_printf(ch, "Unknown error #%d - %s (area data).  Report to Thoric.\n\r", errno, strerror(errno));
               sprintf(buf2, "%s destroying %s", ch->name, buf);
               perror(buf2);
            }
            break;
         }
   }
   else if (errno == ENOENT)
   {
      set_char_color(AT_PLAIN, ch);
      send_to_char("Player does not exist.\n\r", ch);
   }
   else
   {
      set_char_color(AT_WHITE, ch);
      ch_printf(ch, "Unknown error #%d - %s.  Report to Thoric.\n\r", errno, strerror(errno));
      sprintf(buf, "%s destroying %s", ch->name, arg);
      perror(buf);
   }
   return;
}

extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH]; /* db.c */

/* Super-AT command:
FOR ALL <action>
FOR MORTALS <action>
FOR GODS <action>
FOR MOBS <action>
FOR EVERYWHERE <action>

Executes action several times, either on ALL players (not including yourself),
MORTALS (including trusted characters), GODS (characters with level higher than
L_HERO), MOBS (Not recommended) or every room (not recommended either!)

If you insert a # in the action, it will be replaced by the name of the target.

If # is a part of the action, the action will be executed for every target
in game. If there is no #, the action will be executed for every room containg
at least one target, but only once per room. # cannot be used with FOR EVERY-
WHERE. # can be anywhere in the action.

Example: 

FOR ALL SMILE -> you will only smile once in a room with 2 players.
FOR ALL TWIDDLE # -> In a room with A and B, you will twiddle A then B.

Destroying the characters this command acts upon MAY cause it to fail. Try to
avoid something like FOR MOBS PURGE (although it actually works at my MUD).

FOR MOBS TRANS 3054 (transfer ALL the mobs to Midgaard temple) does NOT work
though :)

The command works by transporting the character to each of the rooms with 
target in them. Private rooms are not violated.

*/

/* Expand the name of a character into a string that identifies THAT
   character within a room. E.g. the second 'guard' -> 2. guard
*/
const char *name_expand(CHAR_DATA * ch)
{
   int count = 1;
   CHAR_DATA *rch;
   char name[MIL]; /*  HOPEFULLY no mob has a name longer than THAT */

   static char outbuf[MIL];

   if (!IS_NPC(ch))
      return ch->name;

   one_argument(ch->name, name); /* copy the first word into name */

   if (!name[0]) /* weird mob .. no keywords */
   {
      strcpy(outbuf, ""); /* Do not return NULL, just an empty buffer */
      return outbuf;
   }

   /* ->people changed to ->first_person -- TRI */
   for (rch = ch->in_room->first_person; rch && (rch != ch); rch = rch->next_in_room)
      if (is_name(name, rch->name))
         count++;


   sprintf(outbuf, "%d.%s", count, name);
   return outbuf;
}

void do_for(CHAR_DATA * ch, char *argument)
{
   char range[MIL];
   char buf[MSL];
   bool fGods = FALSE, fMortals = FALSE, fMobs = FALSE, fEverywhere = FALSE, found;
   ROOM_INDEX_DATA *room, *old_room;
   CHAR_DATA *p, *p_prev; /* p_next to p_prev -- TRI */
   int i;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, range);
   if (!range[0] || !argument[0]) /* invalid usage? */
   {
      do_help(ch, "for");
      return;
   }

   if (!str_prefix("quit", argument))
   {
      send_to_char("Are you trying to crash the MUD or something?\n\r", ch);
      return;
   }


   if (!str_cmp(range, "all"))
   {
      fMortals = TRUE;
      fGods = TRUE;
   }
   else if (!str_cmp(range, "gods"))
      fGods = TRUE;
   else if (!str_cmp(range, "mortals"))
      fMortals = TRUE;
   else if (!str_cmp(range, "mobs"))
      fMobs = TRUE;
   else if (!str_cmp(range, "everywhere"))
      fEverywhere = TRUE;
   else
      do_help(ch, "for"); /* show syntax */

   /* do not allow # to make it easier */
   if (fEverywhere && strchr(argument, '#'))
   {
      send_to_char("Cannot use FOR EVERYWHERE with the # thingie.\n\r", ch);
      return;
   }

   set_char_color(AT_PLAIN, ch);
   if (strchr(argument, '#')) /* replace # ? */
   {
      /* char_list - last_char, p_next - gch_prev -- TRI */
      for (p = last_char; p; p = p_prev)
      {
         p_prev = p->prev; /* TRI */
/* p_next = p->next; *//* In case someone DOES try to AT MOBS SLAY # */
         found = FALSE;

         if (!(p->in_room) || room_is_private(p->in_room) || (p == ch))
            continue;

         if (IS_NPC(p) && fMobs)
            found = TRUE;
         else if (!IS_NPC(p) && p->level >= LEVEL_IMMORTAL && fGods)
            found = TRUE;
         else if (!IS_NPC(p) && p->level < LEVEL_IMMORTAL && fMortals)
            found = TRUE;

         /* It looks ugly to me.. but it works :) */
         if (found) /* p is 'appropriate' */
         {
            char *pSource = argument; /* head of buffer to be parsed */
            char *pDest = buf; /* parse into this */

            while (*pSource)
            {
               if (*pSource == '#') /* Replace # with name of target */
               {
                  const char *namebuf = name_expand(p);

                  if (namebuf) /* in case there is no mob name ?? */
                     while (*namebuf) /* copy name over */
                        *(pDest++) = *(namebuf++);

                  pSource++;
               }
               else
                  *(pDest++) = *(pSource++);
            } /* while */
            *pDest = '\0'; /* Terminate */

            /* Execute */
            old_room = ch->in_room;
            char_from_room(ch);
            char_to_room(ch, p->in_room);
            interpret(ch, buf);
            char_from_room(ch);
            char_to_room(ch, old_room);

         } /* if found */
      } /* for every char */
   }
   else /* just for every room with the appropriate people in it */
   {
      for (i = 0; i < MAX_KEY_HASH; i++) /* run through all the buckets */
         for (room = room_index_hash[i]; room; room = room->next)
         {
            found = FALSE;

            /* Anyone in here at all? */
            if (fEverywhere) /* Everywhere executes always */
               found = TRUE;
            else if (!room->first_person) /* Skip it if room is empty */
               continue;
            /* ->people changed to first_person -- TRI */

            /* Check if there is anyone here of the requried type */
            /* Stop as soon as a match is found or there are no more ppl in room */
            /* ->people to ->first_person -- TRI */
            for (p = room->first_person; p && !found; p = p->next_in_room)
            {

               if (p == ch) /* do not execute on oneself */
                  continue;

               if (IS_NPC(p) && fMobs)
                  found = TRUE;
               else if (!IS_NPC(p) && (p->level >= LEVEL_IMMORTAL) && fGods)
                  found = TRUE;
               else if (!IS_NPC(p) && (p->level <= LEVEL_IMMORTAL) && fMortals)
                  found = TRUE;
            } /* for everyone inside the room */

            if (found && !room_is_private(room)) /* Any of the required type here AND room not private? */
            {
               /* This may be ineffective. Consider moving character out of old_room
                  once at beginning of command then moving back at the end.
                  This however, is more safe?
                */

               old_room = ch->in_room;
               char_from_room(ch);
               char_to_room(ch, room);
               interpret(ch, argument);
               char_from_room(ch);
               char_to_room(ch, old_room);
            } /* if found */
         } /* for every room in a bucket */
   } /* if strchr */
} /* do_for */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void read_for_area(char *dirname, char *filename, char *area, CHAR_DATA * ch)
{
   FILE *fp;
   char fname[MSL];
   struct stat fst;
   bool fMatch;
   int room = 0;
   int fnd = 0;

   sprintf(fname, "%s/%s", dirname, filename);

   if (stat(fname, &fst) != -1)
   {
      if ((fp = fopen(fname, "r")) != NULL)
      {
         for (;;)
         {
            char letter;
            char *word;
            int x, y, map;

            fnd = 0;
            x = y = map = -1;
            letter = fread_letter(fp);

            if (letter != '#')
               continue;

            word = fread_word(fp);

            if (!str_cmp(word, "End"))
               break;

            if (!str_cmp(word, "PLAYER"))
            {
               for (;;)
               {
                  if (fnd == 2)
                     break;
                  if (fnd == 1)
                  {
                     ROOM_INDEX_DATA *inroom;

                     if (room)
                     {
                        inroom = get_room_index(room);
                        if (inroom && !str_cmp(inroom->area->name, area))
                           ch_printf(ch, "%-20s is in %20s in room %d at coord %dx %dy %dmap\n\r", filename, area, room, x, y, map);
                     }
                     break;
                  }

                  word = feof(fp) ? "End" : fread_word(fp);
                  fMatch = FALSE;

                  switch (UPPER(word[0]))
                  {
                     case '*':
                        fMatch = TRUE;
                        fread_to_eol(fp);
                        break;

                     case 'C':
                        if (!strcmp(word, "Coordinates"))
                        {
                           x = fread_number(fp);
                           y = fread_number(fp);
                           map = fread_number(fp);
                           if (room)
                              fnd = 1;
                        }
                        break;

                     case 'R':
                        KEY("Room", room, fread_number(fp));
                        break;

                     case 'E':
                        if (!strcmp(word, "End"))
                           fnd = 2;
                        break;

                  }
                  if (!fMatch)
                     fread_to_eol(fp);
               }
            }
            else if (!str_cmp(word, "END")) /* Done  */
               break;
         }
         FCLOSE(fp);
      }
   }
   return;
}

//Mainly scan_pfiles from pfiles code with a few mods
void scan_players_area(CHAR_DATA * ch, char *area)
{
   DIR *dp;
   struct dirent *dentry;
   char dir_name[100];

   int alpha_loop;

   for (alpha_loop = 0; alpha_loop <= 25; alpha_loop++)
   {
      sprintf(dir_name, "%s%c", PLAYER_DIR, 'a' + alpha_loop);
      dp = opendir(dir_name);
      dentry = readdir(dp);
      while (dentry)
      {
         if (dentry->d_name[0] != '.')
         {
            read_for_area(dir_name, dentry->d_name, area, ch);
         }
         dentry = readdir(dp);
      }
      closedir(dp);
   }
}

void do_startroom(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *tarea;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: startroom <area name>\n\r", ch);
      return;
   }

   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (!str_cmp(tarea->name, argument))
         break;
   }
   if (!tarea)
   {
      ch_printf(ch, "%s is not a valid area name, try again.\n\r", argument);
      return;
   }
   send_to_char("Now searching...\n\r", ch);
   scan_players_area(ch, argument);
   send_to_char("Done searching...\n\r", ch);
   return;
}

void do_cset(CHAR_DATA * ch, char *argument)
{
   char arg[MSL];
   int level;
   time_t atime;
   
   atime = (time_t)sysdata.lastaccountreset;

   set_pager_color(AT_PLAIN, ch);

   if (argument[0] == '\0')
   {
      pager_printf_color(ch, "\n\r&c&wMud_name: &G&W%s   &c&wMudVersion: &G&W%s   &c&wCodeVersion: &G&W%s", sysdata.mud_name, sysdata.mversion, sysdata.cversion);
      pager_printf_color(ch, "\n\r&c&wTop_Pid: &G&W%d   &c&wLast Account Reset: &G&W%s", sysdata.top_pid, (char *) ctime(&atime));
      pager_printf_color(ch, "&c&wAccounts Made this Hour: &G&W%d     &c&wMax Accounts per Hour(maxaccounts): &G&W%d", sysdata.accounts, sysdata.max_accounts);
      pager_printf_color(ch, "\n\r&c&wMax Changes to an account per Hour(maxachanges) : &G&W%d", sysdata.max_account_changes);
      pager_printf_color(ch, "\n\r&c&wAccount Email Policy:  &G&W%d   &w&WResetGame Value:  &G&W%d   &c&wTopGem:  &g&W%d", 
         sysdata.accountemail, sysdata.resetgame, sysdata.top_gem_num);
      pager_printf_color(ch, "\n\r&G&WMail:\n\r  &c&wRead all mail: &G&W%d  &c&wRead mail for free: &G&W%d  &c&wWrite mail for free: &G&W%d\n\r",
         sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free);
      pager_printf_color(ch, "  &c&wTake all mail: &G&W%d  &c&wIMC mail board vnum: &G&W%d\n\r", sysdata.take_others_mail, sysdata.imc_mail_vnum);
      pager_printf_color(ch, "\n\r&c&wPfile autocleanup status: &G&W%s  &c&wDays before purging newbies: &G&W%d\n\r",
         sysdata.CLEANPFILES ? "On" : "Off", sysdata.newbie_purge);
      pager_printf_color(ch, "&c&wDays before purging regular players: &G&W%d\n\r", sysdata.regular_purge);
      pager_printf_color(ch, "&G&WChannels:\n\r  &c&wMuse: &G&W%d   &c&wThink: &G&W%d   &c&wLog: &G&W%d   &c&wBuild: &G&W%d\n\r",
         sysdata.muse_level, sysdata.think_level, sysdata.log_level, sysdata.build_level);
      pager_printf_color(ch, "&G&WBuilding:\n\r  &c&wPrototype modification: &G&W%d  &c&wPlayer msetting: &G&W%d\n\r",
         sysdata.level_modify_proto, sysdata.level_mset_player);
      pager_printf_color(ch, "&G&WGuilds:\n\r  &c&wOverseer: &G&W%s   &c&wAdvisor: &G&W%s\n\r", sysdata.guild_overseer, sysdata.guild_advisor);
      pager_printf_color(ch, "&G&WIdle Data:\n\r  &c&wLoginTimeout: &G&W%d   &c&wNotesTimeout: &G&W%d    &c&wIdleTimeout: &G&W%d\n\r",
         sysdata.timeout_login/4, sysdata.timeout_notes/4, sysdata.timeout_idle/4); //4 ticks in a second
      pager_printf_color(ch, "&G&WBan Data:\n\r  &c&wBan Site Level: &G&W%d   &c&wBan Class Level: &G&W%d   ",
         sysdata.ban_site_level, sysdata.ban_class_level);
      pager_printf_color(ch, "&c&wBan Race Level: &G&W%d\n\r", sysdata.ban_race_level);
      pager_printf_color(ch, "&G&WDefenses:\n\r  &c&wDodge_mod: &G&W%d    &c&wParry_mod: &G&W%d    &c&wTumble_mod: &G&W%d\n\r",
         sysdata.dodge_mod, sysdata.parry_mod, sysdata.tumble_mod);
      pager_printf_color(ch, "&G&WOther:\n\r  &c&wForce on players:             &G&W%-2d     ", sysdata.level_forcepc);
      pager_printf_color(ch, "&c&wPrivate room override:         &G&W%-2d\n\r", sysdata.level_override_private);
      pager_printf_color(ch, "  &c&wPenalty to bash plr vs. plr:  &G&W%-7d", sysdata.bash_plr_vs_plr);
      pager_printf_color(ch, "&c&wPenalty to non-tank bash:      &G&W%-3d\n\r", sysdata.bash_nontank);
      pager_printf_color(ch, "  &c&wPenalty to gouge plr vs. plr: &G&W%-7d", sysdata.gouge_plr_vs_plr);
      pager_printf_color(ch, "&c&wPenalty to non-tank gouge:     &G&W%-3d\n\r", sysdata.gouge_nontank);
      pager_printf_color(ch, "  &c&wPenalty regular stun chance:  &G&W%-7d", sysdata.stun_regular);
      pager_printf_color(ch, "&c&wPenalty to stun plr vs. plr:   &G&W%-3d\n\r", sysdata.stun_plr_vs_plr);
      pager_printf_color(ch, "  &c&wPercent damage plr vs. plr:   &G&W%-7d", sysdata.dam_plr_vs_plr);
      pager_printf_color(ch, "&c&wPercent damage plr vs. mob:    &G&W%-3d \n\r", sysdata.dam_plr_vs_mob);
      pager_printf_color(ch, "  &c&wPercent damage mob vs. plr:   &G&W%-7d", sysdata.dam_mob_vs_plr);
      pager_printf_color(ch, "&c&wPercent damage mob vs. mob:    &G&W%-3d\n\r", sysdata.dam_mob_vs_mob);
      pager_printf_color(ch, "  &c&wGet object without take flag: &G&W%-7d", sysdata.level_getobjnotake);
      pager_printf_color(ch, "&c&wAutosave frequency (minutes):  &G&W%d\n\r", sysdata.save_frequency);
      pager_printf_color(ch, "  &c&wMax level difference bestow:  &G&W%-7d", sysdata.bestow_dif);
      pager_printf_color(ch, "&c&wChecking Imm_host is:          &G&W%s\n\r", (sysdata.check_imm_host) ? "ON" : "off");
      pager_printf_color(ch, "  &c&wMorph Optimization is:        &G&W%-7s", (sysdata.morph_opt) ? "ON" : "off");
      pager_printf_color(ch, "&c&wSaving Pets is:                &G&W%s\n\r", (sysdata.save_pets) ? "ON" : "off");
      pager_printf_color(ch, "  &c&wSave flags: &G&W%s\n\r", flag_string(sysdata.save_flags, save_flag));
      pager_printf_color(ch, "  &c&wIdents retries: &G&W%d   &c&wSpoint mod: &G&W%d  &c&wStat mod: &G&W%d", sysdata.ident_retries, sysdata.exp_percent, sysdata.stat_gain);
      pager_printf_color(ch, "  &c&wDefault Gem Vnum: &G&W%d\n\r", sysdata.gem_vnum);

      return;
   }

   argument = one_argument(argument, arg);
   smash_tilde(argument);

   if (!str_cmp(arg, "help"))
   {
      do_help(ch, "controls");
      return;
   }

   if (!str_prefix(arg, "top_pid") || !str_prefix(arg, "top pid") || !str_prefix(arg, "toppid"))
   {
      if (get_trust(ch) < LEVEL_ADMIN)
      {
         send_to_char("This value is not setable.\n\r", ch);
         return;
      }
      else
      {
         sysdata.top_pid = atoi(argument);
         send_to_char("Set, I hope you know what you are doing.\n\r", ch);
         save_sysdata(sysdata);
         return;
      }
   }
   if (!str_cmp(arg, "resetgame"))
   {
      if (get_trust(ch) < LEVEL_ADMIN)
      {
         send_to_char("This value is not setable.\n\r", ch);
         return;
      }
      else
      {
         if (atoi(argument) < 0 || atoi(argument) > 1)
         {
            send_to_char("Range is 0 to 1\n\r", ch);
            return;
         }
         sysdata.resetgame = atoi(argument);
         send_to_char("Set.\n\r", ch);
         save_sysdata(sysdata);
         return;
      }
   }
   if (!str_cmp(arg, "account_email"))
   {
      if (get_trust(ch) < LEVEL_ADMIN)
      {
         send_to_char("This value is not setable.\n\r", ch);
         return;
      }
      else
      {
         if (atoi(argument) < 0 || atoi(argument) > 2)
         {
            send_to_char("Range is 0 to 2\n\r", ch);
            return;
         }
         sysdata.accountemail = atoi(argument);
         send_to_char("Set.\n\r", ch);
         save_sysdata(sysdata);
         return;
      }
   }
   if (!str_cmp(arg, "maxachanges"))
   {
      if (atoi(argument) < 1 || atoi(argument) > 500)
      {
         send_to_char("Range is 1 to 500\n\r", ch);
         return;
      }
      sysdata.max_account_changes = atoi(argument);
      send_to_char("Max Accounts value is set.\n\r", ch);
      return;
   }     
   if (!str_cmp(arg, "maxaccounts"))
   {
      if (atoi(argument) < 1 || atoi(argument) > 500)
      {
         send_to_char("Range is 1 to 500\n\r", ch);
         return;
      }
      sysdata.max_accounts = atoi(argument);
      send_to_char("Max Accounts value is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "logintimeout") || !str_cmp(arg, "login timeout"))
   {
      if (atoi(argument) < 10 || atoi(argument) > 600)
      {
         send_to_char("Range is 10 seconds to 600 seconds (10 minutes).\n\r", ch);
         return;
      }
      sysdata.timeout_login = atoi(argument)*4;
      send_to_char("Login timeout is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "notestimeout") || !str_cmp(arg, "notes timeout"))
   {
      if ((atoi(argument) < 180 && atoi(argument) != 0) || atoi(argument) > 43200)
      {
         send_to_char("Range is 180 seconds to 43200 seconds (12 hours) or 0 for infinite.\n\r", ch);
         return;
      }
      sysdata.timeout_notes = atoi(argument)*4;
      send_to_char("Notes timeout is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "idletimeout") || !str_cmp(arg, "idle timeout"))
   {
      if ((atoi(argument) < 180 && atoi(argument) != 0) || atoi(argument) > 86400)
      {
         send_to_char("Range is 180 seconds to 86400 seconds (24 hours) or 0 for infinite.\n\r", ch);
         return;
      }
      sysdata.timeout_idle = atoi(argument)*4;
      send_to_char("Notes timeout is set.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "gemvnum") || !str_cmp(arg, "gem vnum") || !str_cmp(arg, "default gem vnum"))
   {
      if (atoi(argument) < 1 || atoi(argument) > MAX_VNUM)
      {
         send_to_char("Range is 1 to 2,000,000,000.\n\r", ch);
         return;
      }
      sysdata.gem_vnum = atoi(argument);
      send_to_char("Default gem vnum done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "pfiles"))
   {
      if (sysdata.CLEANPFILES)
         sysdata.CLEANPFILES = 0;
      else
         sysdata.CLEANPFILES = 1;
      return;
   }

   if (!str_cmp(arg, "save"))
   {
      save_sysdata(sysdata);
      send_to_char("Cset functions saved.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "mudname"))
   {
      if (sysdata.mud_name)
         DISPOSE(sysdata.mud_name);
      sysdata.mud_name = str_dup(argument);
      send_to_char("Name set.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg, "codeversion"))
   {
      if (sysdata.cversion)
         DISPOSE(sysdata.cversion);
      sysdata.cversion = str_dup(argument);
      send_to_char("Code Version set.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg, "mudversion"))
   {
      if (sysdata.mversion)
         DISPOSE(sysdata.mversion);
      sysdata.mversion = str_dup(argument);
      send_to_char("Mud Version set.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "saveflag"))
   {
      int x = get_saveflag(argument);

      if (x == -1)
         send_to_char("Not a save flag.\n\r", ch);
      else
      {
         TOGGLE_BIT(sysdata.save_flags, 1 << x);
         send_to_char("Ok.\n\r", ch);
      }
      return;
   }

   if (!str_cmp(arg, "resetcalender"))
   {
      if (get_trust(ch) < LEVEL_ADMIN)
      {
         send_to_char("Only the highest level immortals can use this command.\n\r", ch);
         return;
      }
      sysdata.start_calender = time(0);
      send_to_char("Calender Origin time Reset, I hope that is what you wanted.\n\r", ch);
      return;
   }

   if (!str_prefix(arg, "guild_overseer"))
   {
      STRFREE(sysdata.guild_overseer);
      sysdata.guild_overseer = STRALLOC(argument);
      send_to_char("Ok.\n\r", ch);
      return;
   }
   if (!str_prefix(arg, "guild_advisor"))
   {
      STRFREE(sysdata.guild_advisor);
      sysdata.guild_advisor = STRALLOC(argument);
      send_to_char("Ok.\n\r", ch);
      return;
   }

   level = (sh_int) atoi(argument);

   if (!str_prefix(arg, "savefrequency"))
   {
      sysdata.save_frequency = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "newbie_purge"))
   {
      if (level < 1)
      {
         send_to_char("You must specify a period of at least 1 day.\n\r", ch);
         return;
      }

      sysdata.newbie_purge = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "regular_purge"))
   {
      if (level < 1)
      {
         send_to_char("You must specify a period of at least 1 day.\n\r", ch);
         return;
      }

      sysdata.regular_purge = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

/* Online support for Quest...Cool --Xerves 7/10/99 */
   if (!str_cmp(arg, "Quest_item1"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item1 = level;
      send_to_char("Ok, Quest Item1 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value1"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32000\n\r", ch);
         return;
      }
      sysdata.quest_value1 = level;
      send_to_char("Ok, Quest Value1 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_item2"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item2 = level;
      send_to_char("Ok, Quest Item2 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value2"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32000\n\r", ch);
         return;
      }
      sysdata.quest_value2 = level;
      send_to_char("Ok, Quest Value2 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_item3"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item3 = level;
      send_to_char("Ok, Quest Item3 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value3"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value3 = level;
      send_to_char("Ok, Quest Value3 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_item4"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item4 = level;
      send_to_char("Ok, Quest Item4 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value4"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value4 = level;
      send_to_char("Ok, Quest Value4 Changed\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "Quest_item5"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item5 = level;
      send_to_char("Ok, Quest Item5 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value5"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value5 = level;
      send_to_char("Ok, Quest Value5 Changed\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "Quest_item6"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item6 = level;
      send_to_char("Ok, Quest Item6 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value6"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value6 = level;
      send_to_char("Ok, Quest Value6 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_item7"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item7 = level;
      send_to_char("Ok, Quest Item7 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value7"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value7 = level;
      send_to_char("Ok, Quest Value7 Changed\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "Quest_item8"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item8 = level;
      send_to_char("Ok, Quest Item8 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value8"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value8 = level;
      send_to_char("Ok, Quest Value8 Changed\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "Quest_item9"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item9 = level;
      send_to_char("Ok, Quest Item9 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value9"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value9 = level;
      send_to_char("Ok, Quest Value9 Changed\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "Quest_item10"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item10 = level;
      send_to_char("Ok, Quest Item10 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value10"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value10 = level;
      send_to_char("Ok, Quest Value10 Changed\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "Quest_item11"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item11 = level;
      send_to_char("Ok, Quest Item11 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value11"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value11 = level;
      send_to_char("Ok, Quest Value11 Changed\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "Quest_item12"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest items.\n\r", ch);
         return;
      }
      if (level > MAX_VNUM || level < 1)
      {
         send_to_char("Has to be a valid VNUM RANGE!!!!.\n\r", ch);
         return;
      }
      sysdata.quest_item12 = level;
      send_to_char("Ok, Quest Item12 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Quest_value12"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change quest values.\n\r", ch);
         return;
      }
      if (level > 32000 || level < 500)
      {
         send_to_char("Valid range is 500 to 32,000\n\r", ch);
         return;
      }
      sysdata.quest_value12 = level;
      send_to_char("Ok, Quest Value12 Changed\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "Exp_Percent") || !str_cmp(arg, "Spoint Mod"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change the Spoint mod.\n\r", ch);
         return;
      }
      if (level < 1 || level > 2000)
      {
         send_to_char("Something between 1 and 2000 please!\n\r", ch);
         return;
      }

      sysdata.exp_percent = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg, "Stat_Gain") || !str_cmp(arg, "Stat Mod"))
   {
      if (ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Only Staff level members can change the Stat mod.\n\r", ch);
         return;
      }
      if (level < 1 || level > 5)
      {
         send_to_char("Something between 1 and 5 please!\n\r", ch);
         return;
      }

      sysdata.stat_gain = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_prefix(arg, "checkimmhost"))
   {
      if (level != 0 && level != 1)
      {
         send_to_char("Use 1 to turn it on, 0 to turn in off.\n\r", ch);
         return;
      }
      sysdata.check_imm_host = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "bash_pvp"))
   {
      sysdata.bash_plr_vs_plr = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "bash_nontank"))
   {
      sysdata.bash_nontank = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "gouge_pvp"))
   {
      sysdata.gouge_plr_vs_plr = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "gouge_nontank"))
   {
      sysdata.gouge_nontank = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "dodge_mod"))
   {
      sysdata.dodge_mod = level > 0 ? level : 1;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "parry_mod"))
   {
      sysdata.parry_mod = level > 0 ? level : 1;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "tumble_mod"))
   {
      sysdata.tumble_mod = level > 0 ? level : 1;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "stun"))
   {
      sysdata.stun_regular = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "stun_pvp"))
   {
      sysdata.stun_plr_vs_plr = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "dam_pvp"))
   {
      sysdata.dam_plr_vs_plr = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "get_notake"))
   {
      sysdata.level_getobjnotake = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "dam_pvm"))
   {
      sysdata.dam_plr_vs_mob = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "dam_mvp"))
   {
      sysdata.dam_mob_vs_plr = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "dam_mvm"))
   {
      sysdata.dam_mob_vs_mob = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "imc_vnum") || !str_cmp(arg, "imc_mail_vnum"))
   {
      sysdata.imc_mail_vnum = level;
      send_to_char("Ok.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "ident_retries") || !str_cmp(arg, "ident"))
   {
      sysdata.ident_retries = level;
      if (level > 20)
         send_to_char("Caution:  This setting may cause the game to lag.\n\r", ch);
      else if (level <= 0)
         send_to_char("Ident lookups turned off.\n\r", ch);
      else
         send_to_char("Ok.\n\r", ch);
      return;
   }

   if (level < 0 || level > MAX_LEVEL)
   {
      send_to_char("Invalid value for new control.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "read_all"))
      sysdata.read_all_mail = level;
   else if (!str_cmp(arg, "read_free"))
      sysdata.read_mail_free = level;
   else if (!str_cmp(arg, "write_free"))
      sysdata.write_mail_free = level;
   else if (!str_cmp(arg, "take_all"))
      sysdata.take_others_mail = level;
   else if (!str_cmp(arg, "muse"))
      sysdata.muse_level = level;
   else if (!str_cmp(arg, "think"))
      sysdata.think_level = level;
   else if (!str_cmp(arg, "log"))
      sysdata.log_level = level;
   else if (!str_cmp(arg, "build"))
      sysdata.build_level = level;
   else if (!str_cmp(arg, "proto_modify"))
      sysdata.level_modify_proto = level;
   else if (!str_cmp(arg, "override_private"))
      sysdata.level_override_private = level;
   else if (!str_cmp(arg, "bestow_dif"))
      sysdata.bestow_dif = level > 0 ? level : 1;
   else if (!str_cmp(arg, "forcepc"))
      sysdata.level_forcepc = level;
   else if (!str_cmp(arg, "ban_site_level"))
      sysdata.ban_site_level = level;
   else if (!str_cmp(arg, "ban_race_level"))
      sysdata.ban_race_level = level;
   else if (!str_cmp(arg, "ban_class_level"))
      sysdata.ban_class_level = level;
   else if (!str_cmp(arg, "petsave"))
   {
      if (level)
         sysdata.save_pets = TRUE;
      else
         sysdata.save_pets = FALSE;
   }
   else if (!str_cmp(arg, "morph_opt"))
   {
      if (level)
         sysdata.morph_opt = TRUE;
      else
         sysdata.morph_opt = FALSE;
   }
   else if (!str_cmp(arg, "mset_player"))
      sysdata.level_mset_player = level;
   else
   {
      send_to_char("Invalid argument.\n\r", ch);
      return;
   }
   send_to_char("Ok.\n\r", ch);
   return;
}

void get_reboot_string(void)
{
   sprintf(reboot_time, "%s", asctime(new_boot_time));
}

void do_orange(CHAR_DATA * ch, char *argument)
{
   send_to_char_color("&YFunction under construction.\n\r", ch);
   return;
}

void do_mrange(CHAR_DATA * ch, char *argument)
{
   send_to_char_color("&YFunction under construction.\n\r", ch);
   return;
}

void do_hell(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg[MIL];
   sh_int time;
   bool h_d = FALSE;
   struct tm *tms;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (!*arg)
   {
      send_to_char("Hell who, and for how long?\n\r", ch);
      return;
   }
   if (!(victim = get_char_world(ch, arg)) || IS_NPC(victim))
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_IMMORTAL(victim))
   {
      send_to_char("There is no point in helling an immortal.\n\r", ch);
      return;
   }
   if (victim->pcdata->release_date != 0)
   {
      ch_printf(ch, "They are already in hell until %24.24s, by %s.\n\r", ctime(&victim->pcdata->release_date), victim->pcdata->helled_by);
      return;
   }

   argument = one_argument(argument, arg);
   if (!*arg || !is_number(arg))
   {
      send_to_char("Hell them for how long?\n\r", ch);
      return;
   }

   time = atoi(arg);
   if (time <= 0)
   {
      send_to_char("You cannot hell for zero or negative time.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   if (!*arg || !str_cmp(arg, "hours"))
      h_d = TRUE;
   else if (str_cmp(arg, "days"))
   {
      send_to_char("Is that value in hours or days?\n\r", ch);
      return;
   }
   else if (time > 30)
   {
      send_to_char("You may not hell a person for more than 30 days at a time.\n\r", ch);
      return;
   }
   tms = localtime(&current_time);

   if (h_d)
      tms->tm_hour += time;
   else
      tms->tm_mday += time;
   victim->pcdata->release_date = mktime(tms);
   victim->pcdata->helled_by = STRALLOC(ch->name);
   ch_printf(ch, "%s will be released from hell at %24.24s.\n\r", victim->name, ctime(&victim->pcdata->release_date));
   act(AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT);
   char_from_room(victim);
   char_to_room(victim, get_room_index(8));
   act(AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT);
   do_look(victim, "auto");
   ch_printf(victim, "The immortals are not pleased with your actions.\n\r"
      "You shall remain in hell for %d %s%s.\n\r", time, (h_d ? "hour" : "day"), (time == 1 ? "" : "s"));
   save_char_obj(victim); /* used to save ch, fixed by Thoric 09/17/96 */
   return;
}

void do_unhell(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg[MIL];
   ROOM_INDEX_DATA *location;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (!*arg)
   {
      send_to_char("Unhell whom..?\n\r", ch);
      return;
   }
   location = ch->in_room;
/*ch->in_room = get_room_index(8);*/
   victim = get_char_world(ch, arg);
/*ch->in_room = location;          The case of unhell self, etc.*/
   if (!victim || IS_NPC(victim))
   {
      send_to_char("No such player character present.\n\r", ch);
      return;
   }
   if (victim->in_room->vnum != 8 && victim->in_room->vnum != 1206 && victim->in_room->vnum != 6)
   {
      send_to_char("No one like that is in hell.  Report it to Xerves!\n\r", ch);
      return;
   }

   if (victim->pcdata->clan)
      location = get_room_index(victim->pcdata->clan->recall);
   else
      location = get_room_index(ROOM_VNUM_TEMPLE);
   if (!location)
      location = ch->in_room;
   MOBtrigger = FALSE;
   act(AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT);
   char_from_room(victim);
   char_to_room(victim, location);
   send_to_char("The gods have smiled on you and released you from hell early!\n\r", victim);
   do_look(victim, "auto");
   if (victim != ch)
      send_to_char("They have been released.\n\r", ch);
   if (victim->pcdata->helled_by)
   {
      if (str_cmp(ch->name, victim->pcdata->helled_by))
         ch_printf(ch, "(You should probably write a note to %s, explaining the early release.)\n\r", victim->pcdata->helled_by);
      STRFREE(victim->pcdata->helled_by);
      victim->pcdata->helled_by = NULL;
   }

   MOBtrigger = FALSE;
   act(AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT);
   victim->pcdata->release_date = 0;
   save_char_obj(victim);
   return;
}

/* Vnum search command by Swordbearer */
void do_vsearch(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   bool found = FALSE;
   OBJ_DATA *obj;
   OBJ_DATA *in_obj;
   int obj_counter = 1;
   int argi;

   set_pager_color(AT_PLAIN, ch);

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Syntax:  vsearch <vnum>.\n\r", ch);
      return;
   }

   argi = atoi(arg);
   if (argi < 0 && argi > 20000)
   {
      send_to_char("Vnum out of range.\n\r", ch);
      return;
   }
   for (obj = first_object; obj != NULL; obj = obj->next)
   {
      if (!can_see_obj_map(ch, obj) || !(argi == obj->pIndexData->vnum))
         continue;

      found = TRUE;
      for (in_obj = obj; in_obj->in_obj != NULL; in_obj = in_obj->in_obj) ;

      if (in_obj->carried_by != NULL)
         pager_printf(ch, "[%2d] Level %d %s carried by %s.\n\r", obj_counter, obj->level, obj_short(obj), PERS_MAP(in_obj->carried_by, ch));
      else
         pager_printf(ch, "[%2d] [%-5d] %s in %s.\n\r", obj_counter,
            ((in_obj->in_room) ? in_obj->in_room->vnum : 0), obj_short(obj), (in_obj->in_room == NULL) ? "somewhere" : in_obj->in_room->name);

      obj_counter++;
   }

   if (!found)
      send_to_char("Nothing like that in hell, earth, or heaven.\n\r", ch);
   return;
}

/* 
 * Simple function to let any imm make any player instantly sober.
 * Saw no need for level restrictions on this.
 * Written by Narn, Apr/96 
 */
void do_sober(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg1[MIL];

   set_char_color(AT_IMMORT, ch);

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on mobs.\n\r", ch);
      return;
   }

   if (victim->pcdata)
      victim->pcdata->condition[COND_DRUNK] = 0;
   send_to_char("Ok.\n\r", ch);
   set_char_color(AT_IMMORT, victim);
   send_to_char("You feel sober again.\n\r", victim);
   return;
}

/*
 * Free a social structure					-Thoric
 */
void free_social(SOCIALTYPE * social)
{
   if (social->name)
      DISPOSE(social->name);
   if (social->char_no_arg)
      DISPOSE(social->char_no_arg);
   if (social->others_no_arg)
      DISPOSE(social->others_no_arg);
   if (social->char_found)
      DISPOSE(social->char_found);
   if (social->others_found)
      DISPOSE(social->others_found);
   if (social->vict_found)
      DISPOSE(social->vict_found);
   if (social->char_auto)
      DISPOSE(social->char_auto);
   if (social->others_auto)
      DISPOSE(social->others_auto);
   DISPOSE(social);
}

/*
 * Remove a social from it's hash index				-Thoric
 */
void unlink_social(SOCIALTYPE * social)
{
   SOCIALTYPE *tmp, *tmp_next;
   int hash;

   if (!social)
   {
      bug("Unlink_social: NULL social", 0);
      return;
   }

   if (social->name[0] < 'a' || social->name[0] > 'z')
      hash = 0;
   else
      hash = (social->name[0] - 'a') + 1;

   if (social == (tmp = social_index[hash]))
   {
      social_index[hash] = tmp->next;
      return;
   }
   for (; tmp; tmp = tmp_next)
   {
      tmp_next = tmp->next;
      if (social == tmp_next)
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a social to the social index table			-Thoric
 * Hashed and insert sorted
 */
void add_social(SOCIALTYPE * social)
{
   int hash, x;
   SOCIALTYPE *tmp, *prev;

   if (!social)
   {
      bug("Add_social: NULL social", 0);
      return;
   }

   if (!social->name)
   {
      bug("Add_social: NULL social->name", 0);
      return;
   }

   if (!social->char_no_arg)
   {
      bug("Add_social: NULL social->char_no_arg", 0);
      return;
   }

   /* make sure the name is all lowercase */
   for (x = 0; social->name[x] != '\0'; x++)
      social->name[x] = LOWER(social->name[x]);

   if (social->name[0] < 'a' || social->name[0] > 'z')
      hash = 0;
   else
      hash = (social->name[0] - 'a') + 1;

   if ((prev = tmp = social_index[hash]) == NULL)
   {
      social->next = social_index[hash];
      social_index[hash] = social;
      return;
   }

   for (; tmp; tmp = tmp->next)
   {
      if ((x = strcmp(social->name, tmp->name)) == 0)
      {
         bug("Add_social: trying to add duplicate name to bucket %d", hash);
         free_social(social);
         return;
      }
      else if (x < 0)
      {
         if (tmp == social_index[hash])
         {
            social->next = social_index[hash];
            social_index[hash] = social;
            return;
         }
         prev->next = social;
         social->next = tmp;
         return;
      }
      prev = tmp;
   }

   /* add to end */
   prev->next = social;
   social->next = NULL;
   return;
}

/*
 * Social editor/displayer/save/delete				-Thoric
 */
void do_sedit(CHAR_DATA * ch, char *argument)
{
   SOCIALTYPE *social;
   char arg1[MIL];
   char arg2[MIL];

   set_char_color(AT_SOCIAL, ch);

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: sedit <social> [field]\n\r", ch);
      send_to_char("Syntax: sedit <social> create\n\r", ch);
      if (get_ftrust(ch) >= LEVEL_STAFF) /* Tracker1 */
         send_to_char("Syntax: sedit <social> delete\n\r", ch);
      if (get_ftrust(ch) >= LEVEL_HI_IMM) /* Tracker1 */
         send_to_char("Syntax: sedit <save>\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("  cnoarg onoarg cfound ofound vfound cauto oauto\n\r", ch);
      return;
   }

   if (get_ftrust(ch) > LEVEL_HI_IMM && !str_cmp(arg1, "save")) /* Tracker1 */
   {
      save_socials();
      send_to_char("Saved.\n\r", ch);
      return;
   }

   social = find_social(arg1);
   if (!str_cmp(arg2, "create"))
   {
      if (social)
      {
         send_to_char("That social already exists!\n\r", ch);
         return;
      }
      CREATE(social, SOCIALTYPE, 1);
      social->name = str_dup(arg1);
      sprintf(arg2, "You %s.", arg1);
      social->char_no_arg = str_dup(arg2);
      add_social(social);
      send_to_char("Social added.\n\r", ch);
      return;
   }

   if (!social)
   {
      send_to_char("Social not found.\n\r", ch);
      return;
   }

   if (arg2[0] == '\0' || !str_cmp(arg2, "show"))
   {
      ch_printf(ch, "Social: %s\n\r\n\rCNoArg: %s\n\r", social->name, social->char_no_arg);
      ch_printf(ch, "ONoArg: %s\n\rCFound: %s\n\rOFound: %s\n\r",
         social->others_no_arg ? social->others_no_arg : "(not set)",
         social->char_found ? social->char_found : "(not set)", social->others_found ? social->others_found : "(not set)");
      ch_printf(ch, "VFound: %s\n\rCAuto : %s\n\rOAuto : %s\n\r",
         social->vict_found ? social->vict_found : "(not set)",
         social->char_auto ? social->char_auto : "(not set)", social->others_auto ? social->others_auto : "(not set)");
      return;
   }
   if (get_ftrust(ch) >= LEVEL_STAFF && !str_cmp(arg2, "delete")) /* Tracker1 */
   {
      unlink_social(social);
      free_social(social);
      send_to_char("Deleted.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "cnoarg"))
   {
      if (argument[0] == '\0' || !str_cmp(argument, "clear"))
      {
         send_to_char("You cannot clear this field.  It must have a message.\n\r", ch);
         return;
      }
      if (social->char_no_arg)
         DISPOSE(social->char_no_arg);
      social->char_no_arg = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "onoarg"))
   {
      if (social->others_no_arg)
         DISPOSE(social->others_no_arg);
      if (argument[0] != '\0' && str_cmp(argument, "clear"))
         social->others_no_arg = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "cfound"))
   {
      if (social->char_found)
         DISPOSE(social->char_found);
      if (argument[0] != '\0' && str_cmp(argument, "clear"))
         social->char_found = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "ofound"))
   {
      if (social->others_found)
         DISPOSE(social->others_found);
      if (argument[0] != '\0' && str_cmp(argument, "clear"))
         social->others_found = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "vfound"))
   {
      if (social->vict_found)
         DISPOSE(social->vict_found);
      if (argument[0] != '\0' && str_cmp(argument, "clear"))
         social->vict_found = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "cauto"))
   {
      if (social->char_auto)
         DISPOSE(social->char_auto);
      if (argument[0] != '\0' && str_cmp(argument, "clear"))
         social->char_auto = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "oauto"))
   {
      if (social->others_auto)
         DISPOSE(social->others_auto);
      if (argument[0] != '\0' && str_cmp(argument, "clear"))
         social->others_auto = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (get_ftrust(ch) >= LEVEL_STAFF && !str_cmp(arg2, "name")) /* Tracker1 */
   {
      bool relocate;

      one_argument(argument, arg1);
      if (arg1[0] == '\0')
      {
         send_to_char("Cannot clear name field!\n\r", ch);
         return;
      }
      if (arg1[0] != social->name[0])
      {
         unlink_social(social);
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if (social->name)
         DISPOSE(social->name);
      social->name = str_dup(arg1);
      if (relocate)
         add_social(social);
      send_to_char("Done.\n\r", ch);
      return;
   }

   /* display usage message */
   do_sedit(ch, "");
}

/*
 * Free a command structure					-Thoric
 */
void free_command(CMDTYPE * command)
{
   if (command->name)
      DISPOSE(command->name);
   DISPOSE(command);
}

/*
 * Remove a command from it's hash index			-Thoric
 */
void unlink_command(CMDTYPE * command)
{
   CMDTYPE *tmp, *tmp_next;
   int hash;

   if (!command)
   {
      bug("Unlink_command NULL command", 0);
      return;
   }

   hash = command->name[0] % 126;

   if (command == (tmp = command_hash[hash]))
   {
      command_hash[hash] = tmp->next;
      return;
   }
   for (; tmp; tmp = tmp_next)
   {
      tmp_next = tmp->next;
      if (command == tmp_next)
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a command to the command hash table			-Thoric
 */
void add_command(CMDTYPE * command)
{
   int hash, x;
   CMDTYPE *tmp, *prev;

   if (!command)
   {
      bug("Add_command: NULL command", 0);
      return;
   }

   if (!command->name)
   {
      bug("Add_command: NULL command->name", 0);
      return;
   }

   if (!command->do_fun)
   {
      bug("Add_command: NULL command->do_fun", 0);
      return;
   }

   /* make sure the name is all lowercase */
   for (x = 0; command->name[x] != '\0'; x++)
      command->name[x] = LOWER(command->name[x]);

   hash = command->name[0] % 126;

   if ((prev = tmp = command_hash[hash]) == NULL)
   {
      command->next = command_hash[hash];
      command_hash[hash] = command;
      return;
   }

   /* add to the END of the list */
   for (; tmp; tmp = tmp->next)
      if (!tmp->next)
      {
         tmp->next = command;
         command->next = NULL;
      }
   return;
}

/*
 * Command editor/displayer/save/delete				-Thoric
 * Added support for interpret flags                            -Shaddai
 */
void do_cedit(CHAR_DATA * ch, char *argument)
{
   CMDTYPE *command;
   char arg1[MIL];
   char arg2[MIL];

   set_char_color(AT_IMMORT, ch);

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: cedit save cmdtable\n\r", ch);
      if (get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
      {
         send_to_char("Syntax: cedit <command> create [code]\n\r", ch);
         send_to_char("Syntax: cedit <command> delete\n\r", ch);
         send_to_char("Syntax: cedit <command> show\n\r", ch);
         send_to_char("Syntax: cedit <command> raise\n\r", ch);
         send_to_char("Syntax: cedit <command> lower\n\r", ch);
         send_to_char("Syntax: cedit <command> list\n\r", ch);
         send_to_char("Syntax: cedit <command> [field]\n\r", ch);
         send_to_char("\n\rField being one of:\n\r", ch);
         send_to_char("  level position log code flags fight\n\r", ch);
      }
      return;
   }

   if ((get_trust(ch) >= LEVEL_HI_STAFF || !str_cmp(ch->name, "Nivek")) /* Tracker1 */
      && !str_cmp(arg1, "save") && !str_cmp(arg2, "cmdtable"))
   {
      save_commands();
      send_to_char("Saved.\n\r", ch);
      return;
   }

   command = find_command(arg1);
   if ((!str_cmp(ch->name, "Nivek") || get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
      && !str_cmp(arg2, "create"))
   {
      if (command)
      {
         send_to_char("That command already exists!\n\r", ch);
         return;
      }
      CREATE(command, CMDTYPE, 1);
      command->lag_count = 0; /* FB */
      command->name = str_dup(arg1);
      command->level = get_trust(ch);
      if (*argument)
         one_argument(argument, arg2);
      else
         sprintf(arg2, "do_%s", arg1);
      command->do_fun = skill_function(arg2);
      add_command(command);
      send_to_char("Command added.\n\r", ch);
      if (command->do_fun == skill_notfound)
         ch_printf(ch, "Code %s not found.  Set to no code.\n\r", arg2);
      return;
   }

   if (!command)
   {
      send_to_char("Command not found.\n\r", ch);
      return;
   }
   else if (command->level > get_trust(ch) && str_cmp(ch->name, "Nivek"))
   {
      send_to_char("You cannot touch this command.\n\r", ch);
      return;
   }

   if (arg2[0] == '\0' || !str_cmp(arg2, "show"))
   {
      ch_printf(ch, "Command:  %s\n\rLevel:    %d\n\rPosition: %d\n\rLog:      %d\n\rFight:    %d\n\rCode:     %s\n\rFlags:  %s\n\r",
         command->name, command->level, command->position, command->log, command->fcommand, skill_name(command->do_fun), flag_string(command->flags, cmd_flags));
      if (command->userec.num_uses)
         send_timer(&command->userec, ch);
      return;
   }

   if (get_trust(ch) < LEVEL_HI_STAFF && str_cmp(ch->name, "Nivek")) /* Tracker1 */
   {
      do_cedit(ch, "");
      return;
   }

   if (!str_cmp(arg2, "raise"))
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if ((tmp = command_hash[hash]) == command)
      {
         send_to_char("That command is already at the top.\n\r", ch);
         return;
      }
      if (tmp->next == command)
      {
         command_hash[hash] = command;
         tmp_next = tmp->next;
         tmp->next = command->next;
         command->next = tmp;
         ch_printf(ch, "Moved %s above %s.\n\r", command->name, command->next->name);
         return;
      }
      for (; tmp; tmp = tmp->next)
      {
         tmp_next = tmp->next;
         if (tmp_next->next == command)
         {
            tmp->next = command;
            tmp_next->next = command->next;
            command->next = tmp_next;
            ch_printf(ch, "Moved %s above %s.\n\r", command->name, command->next->name);
            return;
         }
      }
      send_to_char("ERROR -- Not Found!\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "lower"))
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if (command->next == NULL)
      {
         send_to_char("That command is already at the bottom.\n\r", ch);
         return;
      }
      tmp = command_hash[hash];
      if (tmp == command)
      {
         tmp_next = tmp->next;
         command_hash[hash] = command->next;
         command->next = tmp_next->next;
         tmp_next->next = command;

         ch_printf(ch, "Moved %s below %s.\n\r", command->name, tmp_next->name);
         return;
      }
      for (; tmp; tmp = tmp->next)
      {
         if (tmp->next == command)
         {
            tmp_next = command->next;
            tmp->next = tmp_next;
            command->next = tmp_next->next;
            tmp_next->next = command;

            ch_printf(ch, "Moved %s below %s.\n\r", command->name, tmp_next->name);
            return;
         }
      }
      send_to_char("ERROR -- Not Found!\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "list"))
   {
      CMDTYPE *tmp;
      int hash = command->name[0] % 126;

      pager_printf(ch, "Priority placement for [%s]:\n\r", command->name);
      for (tmp = command_hash[hash]; tmp; tmp = tmp->next)
      {
         if (tmp == command)
            set_pager_color(AT_GREEN, ch);
         else
            set_pager_color(AT_PLAIN, ch);
         pager_printf(ch, "  %s\n\r", tmp->name);
      }
      return;
   }
   if (!str_cmp(arg2, "delete"))
   {
      unlink_command(command);
      free_command(command);
      send_to_char("Deleted.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "code"))
   {
      DO_FUN *fun = skill_function(argument);

      if (fun == skill_notfound)
      {
         send_to_char("Code not found.\n\r", ch);
         return;
      }
      command->do_fun = fun;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "level"))
   {
      int level = atoi(argument);

      if (level < 0 || level > get_trust(ch))
      {
         send_to_char("Level out of range.\n\r", ch);
         return;
      }
      command->level = level;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "fight"))
   {
      int level = atoi(argument);

      if (level < 0 || level > 1)
      {
         send_to_char("0 - No Battle Lag Check, 1 - Battle Lag Check.\n\r", ch);
         return;
      }
      command->fcommand = level;
      send_to_char("Done.\n\r", ch);
      return;
   }
        
   if (!str_cmp(arg2, "log"))
   {
      int log = atoi(argument);

      if (log < 0 || log > LOG_COMM)
      {
         send_to_char("Log out of range.\n\r", ch);
         return;
      }
      command->log = log;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "position"))
   {
      int position = atoi(argument);

      if (position < 0 || position > POS_DRAG)
      {
         send_to_char("Position out of range.\n\r", ch);
         return;
      }
      command->position = position;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "flags"))
   {
      int flag;

      if (is_number(argument))
         flag = atoi(argument);
      else
         flag = get_cmdflag(argument);
      if (flag < 0 || flag >= 32)
      {
         if (is_number(argument))
            ch_printf(ch, "Invalid flag: range is from 0 to 31.\n");
         else
            ch_printf(ch, "Unknown flag %s.\n", argument);
         return;
      }

      TOGGLE_BIT(command->flags, 1 << flag);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "name"))
   {
      bool relocate;

      one_argument(argument, arg1);
      if (arg1[0] == '\0')
      {
         send_to_char("Cannot clear name field!\n\r", ch);
         return;
      }
      if (arg1[0] != command->name[0])
      {
         unlink_command(command);
         relocate = TRUE;
      }
      else
         relocate = FALSE;
      if (command->name)
         DISPOSE(command->name);
      command->name = str_dup(arg1);
      if (relocate)
         add_command(command);
      send_to_char("Done.\n\r", ch);
      return;
   }

   /* display usage message */
   do_cedit(ch, "");
}

void do_setrace(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   int value;

   char buf[MSL];
   struct race_type *race;
   int ra;

   set_char_color(AT_PLAIN, ch);

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: setrace <race> <field> <value>\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("  name strplus dexplus wisplus\n\r", ch);
      send_to_char("  intplus conplus chaplus lckplus hit\n\r", ch);
      send_to_char("  mana affected resist suscept language\n\r", ch);
      send_to_char("  save attack defense alignment acplus \n\r", ch);
      send_to_char("  minalign maxalign height weight      \n\r", ch);
      send_to_char("  hungermod thirstmod expmultiplier    \n\r", ch);
      send_to_char("  saving_poison_death saving_wand      \n\r", ch);
      send_to_char("  saving_para_petri saving_breath      \n\r", ch);
      send_to_char("  saving_spell_staff  race_recall      \n\r", ch);
      send_to_char("  mana_regen hp_regen                  \n\r", ch);
      return;
   }
   if (is_number(arg1) && (ra = atoi(arg1)) >= 0 && ra < MAX_RACE)
      race = race_table[ra];
   else
   {
      race = NULL;
      for (ra = 0; ra < MAX_RACE && race_table[ra]; ra++)
         if (!str_cmp(race_table[ra]->race_name, arg1))
         {
            race = race_table[ra];
            break;
         }
   }
   if (!race)
   {
      send_to_char("No such race.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "save"))
   {
      write_race_file(ra);
      send_to_char("Saved.\n\r", ch);
      return;
   }
   if (!argument)
   {
      send_to_char("You must specify an argument.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "name"))
   {
      sprintf(race->race_name, "%-.16s", argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "strplus"))
   {
      race->str_plus = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "dexplus"))
   {
      race->dex_plus = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "wisplus"))
   {
      sprintf(buf, "attempting to set wisplus to %s\n\r", argument);
      send_to_char(buf, ch);
      race->wis_plus = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "intplus"))
   {
      race->int_plus = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "conplus"))
   {
      race->con_plus = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "chaplus"))
   {
      race->cha_plus = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "lckplus"))
   {
      race->lck_plus = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "hit"))
   {
      race->hit = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "mana"))
   {
      race->mana = (sh_int) atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "affected"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: setrace <race> affected <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_aflag(arg3);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            xTOGGLE_BIT(race->affected, value);
      }
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "resist"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: setrace <race> resist <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_risflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            TOGGLE_BIT(race->resist, 1 << value);
      }
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "suscept"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: setrace <race> suscept <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_risflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            TOGGLE_BIT(race->suscept, 1 << value);
      }
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "language"))
   {
      race->language = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "acplus"))
   {
      race->ac_plus = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "alignment"))
   {
      race->alignment = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

   /* not implemented */
   if (!str_cmp(arg2, "defense"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: setrace <race> defense <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_defenseflag(arg3);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            xTOGGLE_BIT(race->defenses, value);
      }
      return;
   }

   /* not implemented */
   if (!str_cmp(arg2, "attack"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: setrace <race> attack <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_attackflag(arg3);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            xTOGGLE_BIT(race->attacks, value);
      }
      return;
   }


   if (!str_cmp(arg2, "minalign"))
   {
      race->minalign = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "maxalign"))
   {
      race->maxalign = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "height"))
   {
      race->height = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "weight"))
   {
      race->weight = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "thirstmod"))
   {
      race->thirst_mod = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "hungermod"))
   {
      race->hunger_mod = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "maxalign"))
   {
      race->maxalign = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "expmultiplier"))
   {
      race->exp_multiplier = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "saving_poison_death"))
   {
      race->saving_poison_death = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "saving_wand"))
   {
      race->saving_wand = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "saving_para_petri"))
   {
      race->saving_para_petri = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "saving_breath"))
   {
      race->saving_breath = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "saving_spell_staff"))
   {
      race->saving_spell_staff = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   /* unimplemented stuff follows */
   if (!str_cmp(arg2, "mana_regen"))
   {
      race->mana_regen = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "hp_regen"))
   {
      race->hp_regen = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "race_recall"))
   {
      race->race_recall = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

#ifdef NEW_RACE_STUFF
   if (!str_cmp(arg2, "carry_weight"))
   {
      race->acplus = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "carry_number"))
   {
      race->acplus = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
#endif
   do_setrace(ch, "");

}


void do_showrace(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   struct race_type *race;
   int ra;

   set_pager_color(AT_PLAIN, ch);

   argument = one_argument(argument, arg1);
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: showrace <race> \n\r", ch);
      return;
   }
   if (is_number(arg1) && (ra = atoi(arg1)) >= 0 && ra < MAX_RACE)
      race = race_table[ra];
   else
   {
      race = NULL;
      for (ra = 0; ra < MAX_RACE && race_table[ra]; ra++)
         if (!str_cmp(race_table[ra]->race_name, arg1))
         {
            race = race_table[ra];
            break;
         }
   }
   if (!race)
   {
      send_to_char("No such race.\n\r", ch);
      return;
   }

   sprintf(buf, "RACE: %s\n\r", race->race_name);
   send_to_char(buf, ch);
   
   sprintf(buf, "Str Plus: %-3d\tDex Plus: %-3d\tWis Plus: %-3d\tInt Plus: %-3d\t\n\r",
      race->str_plus, race->dex_plus, race->wis_plus, race->int_plus);
   send_to_char(buf, ch);
   sprintf(buf, "Con Plus: %-3d\tCha Plus: %-3d\tLck Plus: %-3d\n\r", race->con_plus, race->cha_plus, race->lck_plus);
   send_to_char(buf, ch);
   sprintf(buf, "Hit Pts:  %-3d\tMana: %-3d\tAlign: %-4d\tAC: %-d\n\r", race->hit, race->mana, race->alignment, race->ac_plus);
   send_to_char(buf, ch);
   sprintf(buf, "Min Align: %d\tMax Align: %-d\t\tXP Mult: %-d%%\n\r", race->minalign, race->maxalign, race->exp_multiplier);
   send_to_char(buf, ch);
   sprintf(buf, "Height: %3d in.\t\tWeight: %4d lbs.\tHungerMod: %d\tThirstMod: %d\n\r",
      race->height, race->weight, race->hunger_mod, race->thirst_mod);
   send_to_char(buf, ch);

   send_to_char("Affected by: ", ch);
   send_to_char(affect_bit_name(&race->affected), ch);
   send_to_char("\n\r", ch);

   send_to_char("Resistant to: ", ch);
   send_to_char(flag_string(race->resist, ris_flags), ch);
   send_to_char("\n\r", ch);

   send_to_char("Susceptible to: ", ch);
   send_to_char(flag_string(race->suscept, ris_flags), ch);
   send_to_char("\n\r", ch);

   sprintf(buf, "Saves: (P/D) %d (W) %d (P/P) %d (B) %d (S/S) %d\n\r",
      race->saving_poison_death, race->saving_wand, race->saving_para_petri, race->saving_breath, race->saving_spell_staff);
   send_to_char(buf, ch);

   send_to_char("Innate Attacks: ", ch);
   send_to_char(ext_flag_string(&race->attacks, attack_flags), ch);
   send_to_char("\n\r", ch);

   send_to_char("Innate Defenses: ", ch);
   send_to_char(ext_flag_string(&race->defenses, defense_flags), ch);
   send_to_char("\n\r", ch);

}

/*
 * quest point set - TRI
 * syntax is: qpset char give/take amount
 */

void do_qpset(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char arg3[MIL];
   CHAR_DATA *victim;
   int amount;
   bool give = TRUE;

   set_char_color(AT_IMMORT, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Cannot qpset as an NPC.\n\r", ch);
      return;
   }
   if (get_trust(ch) < LEVEL_IMMORTAL)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   amount = atoi(arg3);
   if (arg[0] == '\0' || arg2[0] == '\0' || amount <= 0)
   {
      send_to_char("Syntax: qpset <character> <give/take> <amount>\n\r", ch);
      send_to_char("Amount must be a positive number greater than 0.\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("There is no such player currently playing.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Glory cannot be given to or taken from a mob.\n\r", ch);
      return;
   }

   set_char_color(AT_IMMORT, victim);
   if (nifty_is_name_prefix(arg2, "give"))
   {
      give = TRUE;
      if (str_cmp(ch->pcdata->council_name, "Quest Council") && (get_trust(ch) < LEVEL_IMM)) /* Tracker1 */
      {
         send_to_char("You must be a member of the Quest Council to give qp to a character.\n\r", ch);
         return;
      }
   }
   else if (nifty_is_name_prefix(arg2, "take"))
      give = FALSE;
   else
   {
      do_qpset(ch, "");
      return;
   }

   if (give)
   {
      victim->pcdata->quest_curr += amount;
      victim->pcdata->quest_accum += amount;
      ch_printf(victim, "Your glory has been increased by %d.\n\r", amount);
      ch_printf(ch, "You have increased the glory of %s by %d.\n\r", victim->name, amount);
   }
   else
   {
      if (victim->pcdata->quest_curr - amount < 0)
      {
         ch_printf(ch, "%s does not have %d glory to take.\n\r", victim->name, amount);
         return;
      }
      else
      {
         victim->pcdata->quest_curr -= amount;
         ch_printf(victim, "Your glory has been decreased by %d.\n\r", amount);
         ch_printf(ch, "You have decreased the glory of %s by %d.\n\r", victim->name, amount);
      }
   }
   return;
}

/* Easy way to check a player's glory -- Blodkai, June 97 */
void do_qpstat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   set_char_color(AT_IMMORT, ch);

   if (IS_NPC(ch))
      return;

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Syntax:  qpstat <character>\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("No one by that name currently in the Realms.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Mobs don't have glory.\n\r", ch);
      return;
   }
   ch_printf(ch, "%s has %d glory, out of a lifetime total of %d.\n\r", victim->name, victim->pcdata->quest_curr, victim->pcdata->quest_accum);
   return;
}

/* Simple, small way to make keeping track of small mods easier - Blod */
void do_fixed(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   struct tm *t = localtime(&current_time);

   set_char_color(AT_OBJECT, ch);
   if (argument[0] == '\0')
   {
      send_to_char("\n\rUsage:  'fixed list' or 'fixed <message>'", ch);
      if (get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
         send_to_char(" or 'fixed clear now'\n\r", ch);
      else
         send_to_char("\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "clear now") && get_trust(ch) >= LEVEL_HI_STAFF)
   { /* Tracker1 */
      FILE *fp = fopen(FIXED_FILE, "w");

      if (fp)
         fclose(fp);
      send_to_char("Fixed file cleared.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "list"))
   {
      send_to_char_color("\n\r&g[&GDate  &g|  &GVnum&g]\n\r", ch);
      show_file(ch, FIXED_FILE);
   }
   else
   {
      sprintf(buf, "&g|&G%-2.2d/%-2.2d &g| &G%5d&g|  %s:  &G%s",
         t->tm_mon + 1, t->tm_mday, ch->in_room ? ch->in_room->vnum : 0, IS_NPC(ch) ? ch->short_descr : ch->name, argument);
      append_to_file(FIXED_FILE, buf);
      send_to_char("Thanks, your modification has been logged.\n\r", ch);
   }
   return;
}

RESERVE_DATA *first_reserved;
RESERVE_DATA *last_reserved;
void save_reserved(void)
{
   RESERVE_DATA *res;
   FILE *fp;

   fclose(fpReserve);
   if (!(fp = fopen(SYSTEM_DIR RESERVED_LIST, "w")))
   {
      bug("Save_reserved: cannot open " RESERVED_LIST, 0);
      perror(RESERVED_LIST);
      fpReserve = fopen(NULL_FILE, "r");
      return;
   }
   for (res = first_reserved; res; res = res->next)
      fprintf(fp, "%s~\n", res->name);
   fprintf(fp, "$~\n");
   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

void do_reserve(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   RESERVE_DATA *res;

   set_char_color(AT_PLAIN, ch);

   argument = one_argument(argument, arg);
   if (!*arg)
   {
      int wid = 0;

      send_to_char("-- Reserved Names --\n\r", ch);
      for (res = first_reserved; res; res = res->next)
      {
         ch_printf(ch, "%c%-17s ", (*res->name == '*' ? '*' : ' '), (*res->name == '*' ? res->name + 1 : res->name));
         if (++wid % 4 == 0)
            send_to_char("\n\r", ch);
      }
      if (wid % 4 != 0)
         send_to_char("\n\r", ch);
      return;
   }
   for (res = first_reserved; res; res = res->next)
      if (!str_cmp(arg, res->name))
      {
         UNLINK(res, first_reserved, last_reserved, next, prev);
         DISPOSE(res->name);
         DISPOSE(res);
         save_reserved();
         send_to_char("Name no longer reserved.\n\r", ch);
         return;
      }
   CREATE(res, RESERVE_DATA, 1);
   res->name = str_dup(arg);
   sort_reserved(res);
   save_reserved();
   send_to_char("Name reserved.\n\r", ch);
   return;
}

/*
 * Command to display the weather status of all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void do_showweather(CHAR_DATA * ch, char *argument)
{
   send_to_char("Not in use.\n\r", ch);
   return;
}

/*
 * Command to control global weather variables and to reset weather
 * Last Modified: July 23, 1997
 * Fireblade
 */
void do_setweather(CHAR_DATA * ch, char *argument)
{
   send_to_char("NOt used.\n\r", ch);
   return;
}


void do_khistory(CHAR_DATA * ch, char *argument)
{
   MOB_INDEX_DATA *tmob;
   char arg[MIL];
   CHAR_DATA *vch;
   int track;

   if (IS_NPC(ch) || !IS_IMMORTAL(ch))
   {
      ch_printf(ch, "Huh?\n\r");
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      ch_printf(ch, "syntax: khistory <player>\n\r");
      return;
   }

   vch = get_char_world(ch, arg);

   if (!vch || IS_NPC(vch))
   {
      ch_printf(ch, "They are not here.\n\r");
      return;
   }

   set_char_color(AT_BLOOD, ch);
   ch_printf(ch, "Kill history for %s:\n\r", vch->name);

   for (track = 0; track < MAX_KILLTRACK && vch->pcdata->killed[track].vnum; track++)
   {
      tmob = get_mob_index(vch->pcdata->killed[track].vnum);

      if (!tmob)
      {
         bug("killhistory: unknown mob vnum");
         continue;
      }

      set_char_color(AT_RED, ch);
      ch_printf(ch, "   %-30s", capitalize(tmob->short_descr));
      set_char_color(AT_BLOOD, ch);
      ch_printf(ch, "(");
      set_char_color(AT_RED, ch);
      ch_printf(ch, "%-5d", tmob->vnum);
      set_char_color(AT_BLOOD, ch);
      ch_printf(ch, ")");
      set_char_color(AT_RED, ch);
      ch_printf(ch, "    - killed %d times.\n\r", vch->pcdata->killed[track].count);
   }

   return;
}

char *project_type(int type)
{   
   if (type == 1)
      return "Bug";
   else if (type == 2)
      return "Typo";     
   else if (type == 3)
      return "Idea";
   else if (type == 4)
      return "Building";
   else if (type == 5)
      return "Coding";
   else if (type == 6)
      return "Improvement";
   else
      return "????";
}  

void do_project(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   char buf2[5];
   int pcount;
   int pnum;
   PROJECT_DATA *pproject;

   if (IS_NPC(ch))
      return;

   if (!ch->desc)
   {
      bug("do_project: no descriptor", 0);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;
      case SUB_WRITING_NOTE:
         if (!ch->pnote)
         {
            bug("do_project: log got lost?", 0);
            send_to_char("Your log was lost!\n\r", ch);
            stop_editing(ch);
            return;
         }
         if (ch->dest_buf != ch->pnote)
            bug("do_project: sub_writing_note: ch->dest_buf != ch->pnote", 0);
         STRFREE(ch->pnote->text);
         ch->pnote->text = copy_buffer(ch);
         stop_editing(ch);
         ch->pnote = NULL;
         write_projects();
         return;
      case SUB_PROJ_DESC:
         if (!ch->dest_buf)
         {
            send_to_char("Your description was lost!", ch);
            bug("do_project: sub_project_desc: NULL ch->dest_buf", 0);
            ch->substate = SUB_NONE;
            return;
         }
         pproject = ch->dest_buf;
         STRFREE(pproject->description);
         pproject->description = copy_buffer(ch);
         stop_editing(ch);
         ch->substate = ch->tempnum;
         write_projects();
         return;
   }
   
   if (argument[0] == '\0')
   {
      do_help(ch, "project");
      return;
   }

   set_char_color(AT_NOTE, ch);
   argument = one_argument(argument, arg);
   smash_tilde(argument);

   if (!str_cmp(arg, "save"))
   {
      write_projects();
      ch_printf(ch, "Projects saved.\n\r");
      return;
   }

   if (!str_cmp(arg, "list"))
   {
      bool aflag, projects_available;
      int num_logs = 0;
      NOTE_DATA *log;
      int mine = FALSE;
      int all = FALSE;
      int vprivate = FALSE;
      int type = 0;
      int lasttime = 0;
      int done = 0;
      int reward = 0;

      aflag = FALSE;
      projects_available = FALSE;
      
      for (;;)
      {
         argument = one_argument(argument, arg1);
         if (arg1[0] != '\0')
         {
            if (!str_cmp(arg1, "available"))
               aflag = TRUE;
            if (!str_cmp(arg1, "mine"))
               mine = TRUE;
            if (!str_cmp(arg1, "all"))
               all = TRUE;
            if (!str_cmp(arg1, "private"))
               vprivate = TRUE;        
            if (!str_cmp(arg1, "done"))
               done = TRUE;       
            if (!str_cmp(arg1, "Bug") || !str_cmp(arg1, "Bugs"))
               type = 1;
            if (!str_cmp(arg1, "Typo") || !str_cmp(arg1, "Typos"))
               type = 2;
            if (!str_cmp(arg1, "Idea") || !str_cmp(arg1, "Ideas"))
               type = 3;
            if (!str_cmp(arg1, "Building") || !str_cmp(arg1, "Area") || !str_cmp(arg1, "Areas"))
               type = 4;
            if (!str_cmp(arg1, "Coding") || !str_cmp(arg1, "Programming") || !str_cmp(arg1, "Code"))
               type = 5;
            if (!str_cmp(arg1, "Improvement") || !str_cmp(arg1, "Improvements"))
               type = 6;
            if (!str_cmp(arg1, "Reward") || !str_cmp(arg1, "Rewards"))
               reward = TRUE;
            if (!str_cmp(arg1, "last30"))
               lasttime = 30*86400;
            if (!str_cmp(arg1, "last90"))
               lasttime = 90*86400;
            if (!str_cmp(arg1, "lastyear"))
               lasttime = 365*86400;
         }
         else
            break;
      }

      pager_printf(ch, "\n\r");
      if (reward)
      {
         pager_printf(ch, " # |Owner    |Project             |Type       |Date      |Rewardee            |Points\n\r");
         pager_printf(ch, "---|---------|--------------------|-----------|----------|--------------------|-------\n\r");
      }   
      else if (!aflag)
      {
         pager_printf(ch, " # |Owner    |Coder    |Project             |Type       |Date      |Status         |#Logs\n\r");
         pager_printf(ch, "---|---------|---------|--------------------|-----------|----------|---------------|------\n\r");
      }
      else
      {
         pager_printf(ch, " # |Owner    |Project             |Type       |Date      |#Logs\n\r");
         pager_printf(ch, "---|---------|--------------------|-----------|----------|------\n\r");
      }
      pcount = 0;
      for (pproject = first_project; pproject; pproject = pproject->next)
      {
         pcount++;
         num_logs = 0;
         if (pproject->status && !str_cmp("Done", pproject->status) && all == FALSE && done == FALSE)
            continue;
         if (done && (!pproject->status || str_cmp("Done", pproject->status)))
            continue;
         if (pproject->status && !str_cmp("Private", pproject->status) && str_cmp(ch->pcdata->council_name, "Code Council") 
         &&  get_trust(ch) < LEVEL_HI_STAFF && str_cmp(pproject->owner, ch->name))
            continue;
         if (type > 0 && pproject->type != type)
            continue;
         if (lasttime > 0 && time(0) - pproject->time > lasttime)
            continue;
         if (vprivate && str_cmp("Private", pproject->status))
            continue;
         for (log = pproject->first_log; log; log = log->next)
            num_logs++;
         if (mine == TRUE && str_cmp(pproject->owner, ch->name) && str_cmp(pproject->coder, ch->name))
            continue;    
         sprintf(buf2, "%d", pcount);
         sprintf(buf, MXPFTAG("Command 'project %d show all' desc='Click here to view the project posting'", "%d", "/Command") "%s", 
               pcount, pcount, add_space(strlen(buf2), 3));
         if (reward)
         {
            pager_printf(ch, "%3s|%-9.9s|%-20.20s|%-11s|%-10.10s|%-20.20s|%2d/%-2d\n\r", buf, pproject->owner ? pproject->owner : "(None)", 
            pproject->name, project_type(pproject->type), pproject->date, pproject->rewardee, pproject->rewardedpoints,
            pproject->points);
         }
         else if (!aflag)
            pager_printf(ch, "%3s|%-9.9s|%-9.9s|%-20.20s|%-11s|%-10.10s|%-15s|%3d\n\r",
               buf, pproject->owner ? pproject->owner : "(None)", pproject->coder ? pproject->coder : "(None)", pproject->name, 
               project_type(pproject->type), pproject->date, pproject->status ? pproject->status : "(None)", num_logs);
         else if (!pproject->coder)
         {
            if (!projects_available)
               projects_available = TRUE;
            pager_printf(ch, "%3s|%-9.9s|%-20.20s|%-11s|%-10.10s|%3d\n\r", buf, pproject->owner ? pproject->owner : "(None)", 
            pproject->name, project_type(pproject->type), pproject->date, num_logs);
         }
      }
      if (pcount == 0)
         pager_printf(ch, "No projects exist.\n\r");
      else if (aflag && !projects_available)
         pager_printf(ch, "No projects available.\n\r");
      pager_printf(ch, "\n\rClick on the following to start a new project: ");
      sprintf(buf, MXPFTAG("PCommand 'project add Bug <put short description in here>' desc='Click here to add a Bug Project'", "Bug", "/PCommand"));
      pager_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("PCommand 'project add Typo <put short description in here>' desc='Click here to add a Typo Project'", "Typo", "/PCommand"));
      pager_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("PCommand 'project add Idea <put short description in here>' desc='Click here to add an Idea Project'", "Idea", "/PCommand"));
      pager_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("PCommand 'project add Building <put short description in here>' desc='Click here to add a Building Project'", "Building", "/PCommand"));
      pager_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("PCommand 'project add Coding <put short description in here>' desc='Click here to add a Coding Project'", "Coding", "/PCommand"));
      pager_printf(ch, "%s ", buf);
      sprintf(buf, MXPFTAG("PCommand 'project add Improvement <put short description in here>' desc='Click here to add an Improvement Project'", "Improvement", "/PCommand"));
      pager_printf(ch, "%s\n\r", buf);
      return;
   }

   if (!str_cmp(arg, "add"))
   {
      PROJECT_DATA *new_project; /* Just to be safe */
      time_t now;
      struct tm *tptr;
      char tbuf[100];

      argument = one_argument(argument, arg1);
      CREATE(new_project, PROJECT_DATA, 1);
      LINK(new_project, first_project, last_project, next, prev);
      new_project->name = str_dup(argument);
      new_project->coder = NULL;
      new_project->taken = TRUE;
      new_project->owner = STRALLOC(ch->name);
      new_project->status = STRALLOC("Private");
      new_project->type = 1;
      if (!str_cmp(arg1, "Bug") || !str_cmp(arg1, "Bugs"))
         new_project->type = 1;
      if (!str_cmp(arg1, "Typo") || !str_cmp(arg1, "Typos"))
         new_project->type = 2;
      if (!str_cmp(arg1, "Idea") || !str_cmp(arg1, "Ideas"))
         new_project->type = 3;
      if (!str_cmp(arg1, "Building") || !str_cmp(arg1, "Area") || !str_cmp(arg1, "Areas"))
         new_project->type = 4;
      if (!str_cmp(arg1, "Coding") || !str_cmp(arg1, "Programming") || !str_cmp(arg1, "Code"))
         new_project->type = 5;
      if (!str_cmp(arg1, "Improvement") || !str_cmp(arg1, "Improvements"))
         new_project->type = 6;
      new_project->description = STRALLOC("");
      time(&now);
      tptr = localtime(&now);
      strftime(tbuf, 100, "%m-%d-%Y", tptr);
      new_project->date = STRALLOC(tbuf);
      new_project->time = time(0);
      if (get_trust(ch) < LEVEL_STAFF && /* Tracker1 */
         str_cmp(ch->pcdata->council_name, "Code Council"))
         CHECK_SUBRESTRICTED(ch);
      ch->tempnum = SUB_NONE;
      ch->substate = SUB_PROJ_DESC;
      ch->dest_buf = new_project;
      if (new_project->description == NULL)
         new_project->description = STRALLOC("");
      start_editing(ch, new_project->description);
      editor_desc_printf(ch, "Project description for project '%s'.", new_project->name ? new_project->name : "(No name)");
      write_projects();
      return;
   }

   if (!is_number(arg))
   {
      ch_printf(ch, "Invalid project.\n\r");
      return;
   }

   pnum = atoi(arg);
   pproject = get_project_by_number(pnum);
   if (!pproject)
   {
      ch_printf(ch, "No such project.\n\r");
      return;
   }

   argument = one_argument(argument, arg1);

   if (!str_cmp(arg1, "description"))
   {
      if (!str_cmp(ch->pcdata->council_name, "Code Council") || get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
      {
         if (get_trust(ch) < LEVEL_STAFF && /* Tracker1 */
            str_cmp(ch->pcdata->council_name, "Code Council"))
            CHECK_SUBRESTRICTED(ch);
         ch->tempnum = SUB_NONE;
         ch->substate = SUB_PROJ_DESC;
         ch->dest_buf = pproject;
         if (pproject->description == NULL)
            pproject->description = STRALLOC("");
         start_editing(ch, pproject->description);
         editor_desc_printf(ch, "Project description for project '%s'.", pproject->name ? pproject->name : "(No name)");
         return;
      }
      else
      {
         send_to_char("You cannot change the description of an already posted project.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg1, "type"))
   {
      if (!str_cmp(ch->pcdata->council_name, "Code Council") || get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
      {
         pproject->type = 1;
         if (!str_cmp(argument, "Bug") || !str_cmp(argument, "Bugs"))
            pproject->type = 1;
         if (!str_cmp(argument, "Typo") || !str_cmp(argument, "Typos"))  
            pproject->type = 2;
         if (!str_cmp(argument, "Idea") || !str_cmp(argument, "Ideas"))
            pproject->type = 3;
         if (!str_cmp(argument, "Building") || !str_cmp(argument, "Area") || !str_cmp(argument, "Areas"))
            pproject->type = 4;
         if (!str_cmp(argument, "Coding") || !str_cmp(argument, "Programming") || !str_cmp(argument, "Code"))
            pproject->type = 5;
         if (!str_cmp(argument, "Improvement") || !str_cmp(argument, "Improvements"))
            pproject->type = 6;
         send_to_char("Changed.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("You do not have permission to alter the type of a project.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg1, "delete"))
   {
      NOTE_DATA *log, *tlog;

      if (str_cmp(ch->pcdata->council_name, "Code Council") && get_trust(ch) < LEVEL_HI_STAFF) /* Tracker1 */
      {
         send_to_char("You do not have permission to delete a project.\n\r", ch);
         return;
      }

      log = pproject->last_log;
      while (log)
      {
         UNLINK(log, pproject->first_log, pproject->last_log, next, prev);
         tlog = log->prev;
         free_note(log);
         log = tlog;
      }
      UNLINK(pproject, first_project, last_project, next, prev);

      DISPOSE(pproject->name);
      if (pproject->coder)
         DISPOSE(pproject->coder);
      if (pproject->owner)
         STRFREE(pproject->owner);
      if (pproject->description)
         STRFREE(pproject->description);
      if (pproject->date)
         STRFREE(pproject->date);
      if (pproject->status)
         STRFREE(pproject->status);
      if (pproject->rewardee)
         STRFREE(pproject->rewardee);

      DISPOSE(pproject);
      write_projects();
      ch_printf(ch, "Ok.\n\r");
      return;
   }
   
   if (!str_cmp(arg1, "reward"))
   {
      argument = one_argument(argument, arg2);
      
      if (str_cmp(ch->pcdata->council_name, "Code Council") && get_trust(ch) < LEVEL_HI_STAFF) /* Tracker1 */
      {
         send_to_char("You do not have permission to reward a project.\n\r", ch);
         return;
      }
      
      if (!str_cmp(arg2, "None"))
      {
         if (pproject->rewardee)
            STRFREE(pproject->rewardee);
         pproject->rewardee = STRALLOC(arg2);
         pproject->points = 0;
         pproject->rewardedpoints = 0;
      }
      if (pproject->rewardee)
         STRFREE(pproject->rewardee);
      pproject->rewardee = STRALLOC(arg2);
      pproject->points = atoi(argument);
      pproject->rewardedpoints = 0;  
      write_projects();
      ch_printf(ch, "Ok.\n\r");
      return;
   }

   if (!str_cmp(arg1, "take"))
   {
      if (pproject->taken && pproject->owner && !str_cmp(pproject->owner, ch->name))
      {
         pproject->taken = FALSE;
         STRFREE(pproject->owner);
         pproject->owner = NULL;
         send_to_char("You removed yourself as the owner.\n\r", ch);
         write_projects();
         return;
      }
      else if (pproject->taken)
      {
         if (!str_cmp(ch->pcdata->council_name, "Code Council") || get_trust(ch) >= LEVEL_HI_STAFF)
         {
            STRFREE(pproject->owner);
            pproject->owner = STRALLOC(ch->name);
            send_to_char("You have taken ownership of this project.\n\r", ch);
            write_projects();
         }
         else
         {
            ch_printf(ch, "This project is already taken.\n\r");
            return;
         }
      }

      if (!str_cmp(ch->pcdata->council_name, "Code Council") || get_trust(ch) >= LEVEL_HI_STAFF)
      {
         if (pproject->owner)
            STRFREE(pproject->owner);
         pproject->owner = STRALLOC(ch->name);
         pproject->taken = TRUE;
         write_projects();
         ch_printf(ch, "Ok.\n\r");
         return;
      }
      else
      {
         send_to_char("You do not have permission to take ownership of a project.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg1, "name"))
   {
      if (!str_cmp(ch->pcdata->council_name, "Code Council") || get_trust(ch) >= LEVEL_HI_STAFF)
      {
         DISPOSE(pproject->name);
         pproject->name = str_dup(argument);
         write_projects();
         send_to_char("Done.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("You do not have permission to make changes.\n\r", ch);
         return;
      }
   }   
   if (!str_cmp(arg1, "coder"))
   {
      if (pproject->coder && !str_cmp(ch->name, pproject->coder))
      {
         DISPOSE(pproject->coder);
         pproject->coder = NULL;
         send_to_char("You removed yourself as the coder.\n\r", ch);
         write_projects();
         return;
      }
      else if (pproject->coder)
      {
         if (!str_cmp(ch->pcdata->council_name, "Code Council") || get_trust(ch) >= LEVEL_HI_STAFF)
         {
            DISPOSE(pproject->coder);
            pproject->coder = str_dup(ch->name);
            write_projects();
            ch_printf(ch, "Ok.\n\r");   
         }
         else
         {
            ch_printf(ch, "This project already has a coder.\n\r");
            return;
         }
      }
      if (!str_cmp(ch->pcdata->council_name, "Code Council") || get_trust(ch) >= LEVEL_HI_STAFF)
      {
         pproject->coder = str_dup(ch->name);
         write_projects();
         ch_printf(ch, "Ok.\n\r");
         return;
      }
      else
      {
         send_to_char("You do not have permission to take coding ownership of this project.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg1, "status"))
   {
      if ((pproject->owner && !str_cmp(pproject->owner, ch->name)) || (pproject->coder && !str_cmp(pproject->coder, ch->name))
      || get_trust(ch) >= LEVEL_HI_STAFF || !str_cmp(ch->pcdata->council_name, "Code Council"))
      {
         if (pproject->status)
            STRFREE(pproject->status);
         pproject->status = STRALLOC(argument);
         write_projects();
         send_to_char("Done.\n\r", ch);
         return;
      }
      else
      {
         ch_printf(ch, "This is not your project!\n\r");
         return;
      }  
   }
   if (!str_cmp(arg1, "show"))
   {
      if (pproject->description)
      {
         int pcount = 0;
         NOTE_DATA *plog;
         
         ch_printf(ch, "&w&WName:  &w&G%s\n\r&w&WDescription:\n\r&w&G", pproject->name);
         send_to_char(pproject->description, ch);
         
         if (!str_cmp(argument, "all"))
         {
            ch_printf(ch, "\n\r&w&WNOTES:  \n\r\n\r");
            for (plog = pproject->first_log; plog; plog = plog->next)
            {
               pcount++;
               pager_printf(ch, "[%3d] %s: %s\n\r%s\n\r-----------------------------\n\r%s\n\r", 
                  pcount, plog->sender, plog->subject, plog->date, plog->text);
            }
         }
         sprintf(buf, MXPFTAG("PCommand 'project %d log write <put short description in here>' desc='Click here to post a note on this project'", "Click here to write a note", "/PCommand"), pnum);
         pager_printf(ch, "\n\r%s\n\r", buf);
      }
      else
         send_to_char("That project does not have a description.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "log"))
   {
      NOTE_DATA *plog;
      
      argument = one_argument(argument, arg2);

      if (!str_cmp(arg2, "write"))
      {
         char *strtime;
         note_attach(ch);
         STRFREE(ch->pnote->subject);
         ch->pnote->subject = STRALLOC(argument);
         strtime = ctime(&current_time);
         strtime[strlen(strtime) - 1] = '\0';
         ch->pnote->date = STRALLOC(strtime);
         ch->pnote->sender = ch->name;

         plog = ch->pnote;
         LINK(plog, pproject->first_log, pproject->last_log, next, prev);
         write_projects();
         ch->substate = SUB_WRITING_NOTE;
         ch->dest_buf = ch->pnote;
         start_editing(ch, ch->pnote->text);
         editor_desc_printf(ch, "A log note in project '%s', entitled '%s'.",
            pproject->name ? pproject->name : "(No name)", ch->pnote->subject ? ch->pnote->subject : "(No subject)");
         return;
      }

      if (!str_cmp(arg2, "list"))
      {
         pcount = 0;
         pager_printf(ch, "Project: %-12s: %s\n\r", pproject->owner ? pproject->owner : "(None)", pproject->name);

         for (plog = pproject->first_log; plog; plog = plog->next)
         {
            pcount++;
            pager_printf(ch, "%2d) %-12s: %s\n\r", pcount, plog->sender, plog->subject);
         }
         if (pcount == 0)
            ch_printf(ch, "No logs available.\n\r");
         return;
      }

      if (!is_number(arg2))
      {
         ch_printf(ch, "Invalid log.\n\r");
         return;
      }

      pnum = atoi(arg2);

      plog = get_log_by_number(pproject, pnum);
      if (!plog)
      {
         ch_printf(ch, "Invalid log.\n\r");
         return;
      }


      if (!str_cmp(argument, "delete"))
      {
         if (get_trust(ch) < LEVEL_HI_STAFF
         && str_cmp(ch->pcdata->council_name, "Code Council"))
         {
            ch_printf(ch, "This is not your project!\n\r");
            return;
         }

         UNLINK(plog, pproject->first_log, pproject->last_log, next, prev);
         free_note(plog);
         write_projects();
         ch_printf(ch, "Ok.\n\r");
         return;
      }

      if (!str_cmp(argument, "read"))
      {
         pager_printf(ch, "[%3d] %s: %s\n\r%s\n\r-----------------------------\n\r%s\n\r", 
            pnum, plog->sender, plog->subject, plog->date, plog->text);
         return;
      }
   }
   do_project(ch, "");
   return;
}

PROJECT_DATA *get_project_by_number(int pnum)
{
   int pcount;
   PROJECT_DATA *pproject;

   pcount = 0;
   for (pproject = first_project; pproject; pproject = pproject->next)
   {
      pcount++;
      if (pcount == pnum)
         return pproject;
   }
   return NULL;
}

NOTE_DATA *get_log_by_number(PROJECT_DATA * pproject, int pnum)
{
   int pcount;
   NOTE_DATA *plog;

   pcount = 0;
   for (plog = pproject->first_log; plog; plog = plog->next)
   {
      pcount++;
      if (pcount == pnum)
         return plog;
   }
   return NULL;
}

/*
 * Command to check for multiple ip addresses in the mud.
 * --Shaddai
 */

 /*
  * Added this new struct to do matching
  * If ya think of a better way do it, easiest way I could think of at
  * 2 in the morning :) --Shaddai
  */

typedef struct ipcompare_data IPCOMPARE_DATA;
struct ipcompare_data
{
   struct ipcompare_data *prev;
   struct ipcompare_data *next;
   char *host;
   char *name;
   char *user;
   int connected;
   int count;
   int descriptor;
   int idle;
   int port;
   bool printed;
};

void do_ipcompare(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char arg[MIL];
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   char *addie = NULL;
   bool prefix = FALSE, suffix = FALSE, inarea = FALSE, inroom = FALSE, inworld = FALSE;
   int count = 0, times = -1;
   bool fMatch;

   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   set_pager_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (arg[0] == '\0')
   {
      send_to_char("ipcompare total\n\r", ch);
      send_to_char("ipcompare <person> [room|area|world] [#]\n\r", ch);
      send_to_char("ipcompare <site>   [room|area|world] [#]\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "total"))
   {
      IPCOMPARE_DATA *first_ip = NULL, *last_ip = NULL, *hmm, *hmm_next;

      for (d = first_descriptor; d; d = d->next)
      {
         fMatch = FALSE;
         for (hmm = first_ip; hmm; hmm = hmm->next)
            if (!str_cmp(hmm->host, d->host))
               fMatch = TRUE;
         if (!fMatch)
         {
            IPCOMPARE_DATA *temp;

            CREATE(temp, IPCOMPARE_DATA, 1);
            temp->host = str_dup(d->host);
            LINK(temp, first_ip, last_ip, next, prev);
            count++;
         }
      }
      for (hmm = first_ip; hmm; hmm = hmm_next)
      {
         hmm_next = hmm->next;
         UNLINK(hmm, first_ip, last_ip, next, prev);
         if (hmm->host)
            DISPOSE(hmm->host);
         DISPOSE(hmm);
      }
      ch_printf(ch, "There were %d unique ip addresses found.\n\r", count);
      return;
   }
   if (arg1[0] != '\0')
   {
      if (is_number(arg1))
         times = atoi(arg1);
      else
      {
         if (!str_cmp(arg1, "room"))
            inroom = TRUE;
         else if (!str_cmp(arg1, "area"))
            inarea = TRUE;
         else
            inworld = TRUE;
      }
      if (arg2[0] != '\0')
      {
         if (is_number(arg2))
            times = atoi(arg2);
         else
         {
            send_to_char("Please see help ipcompare for more info.\n\r", ch);
            return;
         }
      }
   }
   if ((victim = get_char_world(ch, arg)) != NULL && victim->desc)
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }
      addie = victim->desc->host;
   }
   else
   {
      addie = arg;
      if (arg[0] == '*')
      {
         prefix = TRUE;
         addie++;
      }
      if (addie[strlen(addie) - 1] == '*')
      {
         suffix = TRUE;
         addie[strlen(addie) - 1] = '\0';
      }
   }
   sprintf(buf, "\n\rDesc|Con|Idle| Port | Player      ");
   if (get_trust(ch) >= LEVEL_IMM) /* Tracker1 */
      strcat(buf, "@HostIP           ");
   if (get_trust(ch) >= LEVEL_STAFF) /* Tracker1 */
      strcat(buf, "| Username");
   strcat(buf, "\n\r");
   strcat(buf, "----+---+----+------+-------------");
   if (get_trust(ch) >= LEVEL_IMM) /* Tracker1 */
      strcat(buf, "------------------");
   if (get_trust(ch) >= LEVEL_STAFF) /* Tracker1 */
      strcat(buf, "+---------");
   strcat(buf, "\n\r");
   send_to_pager(buf, ch);
   for (d = first_descriptor; d; d = d->next)
   {
      if (!d->character || (d->connected != CON_PLAYING && d->connected != CON_EDITING) || !can_see_map(ch, d->character))
         continue;
      if (inroom && ch->in_room != d->character->in_room)
         continue;
      if (inarea && ch->in_room->area != d->character->in_room->area)
         continue;
      if (times > 0 && count == (times - 1))
         break;
      if (prefix && suffix && strstr(addie, d->host))
         fMatch = TRUE;
      else if (prefix && !str_suffix(addie, d->host))
         fMatch = TRUE;
      else if (suffix && !str_prefix(addie, d->host))
         fMatch = TRUE;
      else if (!str_cmp(d->host, addie))
         fMatch = TRUE;
      else
         fMatch = FALSE;
      if (fMatch)
      {
         count++;
         sprintf(buf,
            " %3d| %2d|%4d|%6d| %-12s",
            d->descriptor, d->connected, d->idle / 4, d->port, d->original ? d->original->name : d->character ? d->character->name : "(none)");
         if (get_trust(ch) >= LEVEL_IMM) /* Tracker1 */
            sprintf(buf + strlen(buf), "@%-16s ", d->host);
         if (get_trust(ch) >= LEVEL_STAFF) /* Tracker1 */
            sprintf(buf + strlen(buf), "| %s", d->user);
         strcat(buf, "\n\r");
         send_to_pager(buf, ch);
      }
   }
   pager_printf(ch, "%d user%s.\n\r", count, count == 1 ? "" : "s");
   return;
}


/*
 * New nuisance flag to annoy people that deserve it :) --Shaddai
 */
void do_nuisance(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg[MIL];
   char arg1[MIL];
   char arg2[MIL];
   struct tm *now_time;
   int time = 0, max_time = 0, power = 1;
   bool minute = FALSE, day = FALSE, hour = FALSE;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Syntax: nuisance <victim> [Options]\n\r", ch);
      send_to_char("Options:\n\r", ch);
      send_to_char("  power <level 1-10>\n\r", ch);
      send_to_char("  time  <days>\n\r", ch);
      send_to_char("  maxtime <#> <minutes/hours/days>\n\r", ch);
      send_to_char("Defaults: Time -- forever, power -- 1, maxtime 8 days.\n\r", ch);
      return;
   }

   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("There is no one on with that name.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("You can't set a nuisance flag on a mob.\n\r", ch);
      return;
   }

   if (get_trust(ch) <= get_trust(victim))
   {
      send_to_char("I don't think they would like that.\n\r", ch);
      return;
   }

   if (victim->pcdata->nuisance)
   {
      send_to_char("That flag has already been set.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);

   while (argument[0] != '\0')
   {
      if (!str_cmp(arg1, "power"))
      {
         argument = one_argument(argument, arg1);
         if (arg1[0] == '\0' || !is_number(arg1))
         {
            send_to_char("Power option syntax: power <number>\n\r", ch);
            return;
         }
         if ((power = atoi(arg1)) < 1 || power > 10)
         {
            send_to_char("Power must be 1 - 10.\n\r", ch);
            return;
         }
      }
      else if (!str_cmp(arg1, "time"))
      {
         argument = one_argument(argument, arg1);
         if (arg1[0] == '\0' || !is_number(arg1))
         {
            send_to_char("Time option syntax: time <number> (In days)\n\r", ch);
            return;
         }
         if ((time = atoi(arg1)) < 1)
         {
            send_to_char("Time must be a positive number.\n\r", ch);
            return;
         }
      }
      else if (!str_cmp(arg1, "maxtime"))
      {
         argument = one_argument(argument, arg1);
         argument = one_argument(argument, arg2);
         if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg1))
         {
            send_to_char("Maxtime option syntax: maxtime <number> <minute|day|hour>\n\r", ch);
            return;
         }
         if ((max_time = atoi(arg1)) < 1)
         {
            send_to_char("Maxtime must be a positive number.\n\r", ch);
            return;
         }
         if (!str_cmp(arg2, "minutes"))
            minute = TRUE;
         else if (!str_cmp(arg2, "hours"))
            hour = TRUE;
         else if (!str_cmp(arg2, "days"))
            day = TRUE;
      }
      else
      {
         ch_printf(ch, "Unknown option %s.\n\r", arg1);
         return;
      }
      argument = one_argument(argument, arg1);
   }

   if (minute && (max_time < 1 || max_time > 59))
   {
      send_to_char("Minutes must be 1 to 59.\n\r", ch);
      return;
   }
   else if (hour && (max_time < 1 || max_time > 23))
   {
      send_to_char("Hours must be 1 - 23.\n\r", ch);
      return;
   }
   else if (day && (max_time < 1 || max_time > 999))
   {
      send_to_char("Days must be 1 - 999.\n\r", ch);
      return;
   }
   else if (!max_time)
   {
      day = TRUE;
      max_time = 7;
   }
   CREATE(victim->pcdata->nuisance, NUISANCE_DATA, 1);
   victim->pcdata->nuisance->time = current_time;
   victim->pcdata->nuisance->flags = 1;
   victim->pcdata->nuisance->power = power;
   now_time = localtime(&current_time);

   if (minute)
      now_time->tm_min += max_time;
   else if (hour)
      now_time->tm_hour += max_time;
   else
      now_time->tm_mday += max_time;

   victim->pcdata->nuisance->max_time = mktime(now_time);
   if (time)
   {
      add_timer(victim, TIMER_NUISANCE, (28800 * time), NULL, 0);
      ch_printf(ch, "Nuisance flag set for %d days.\n\r", time);
   }
   else
      send_to_char("Nuisance flag set forever\n\r", ch);
   return;
}

void do_unnuisance(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TIMER *timer, *timer_next;
   char arg[MIL];

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   one_argument(argument, arg);

   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("There is no one on with that name.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You can't remove a nuisance flag from a mob.\n\r", ch);
      return;
   }
   if (get_trust(ch) <= get_trust(victim))
   {
      send_to_char("You can't do that.\n\r", ch);
      return;
   }
   if (!victim->pcdata->nuisance)
   {
      send_to_char("They do not have that flag set.\n\r", ch);
      return;
   }
   for (timer = victim->first_timer; timer; timer = timer_next)
   {
      timer_next = timer->next;
      if (timer->type == TIMER_NUISANCE)
         extract_timer(victim, timer);
   }
   DISPOSE(victim->pcdata->nuisance);
   send_to_char("Nuisance flag removed.\n\r", ch);
   return;
}

void do_pcrename(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg1[MIL];
   char arg2[MIL];
   char newname[MSL];
   char oldname[MSL];
   char backname[MSL];
   char buf[MSL];

   argument = one_argument(argument, arg1);
   one_argument(argument, arg2);
   smash_tilde(arg2);


   if (IS_NPC(ch))
      return;

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Syntax: rename <victim> <new name>\n\r", ch);
      return;
   }

   if (!check_parse_name(arg2, 1))
   {
      send_to_char("Illegal name.\n\r", ch);
      return;
   }
   /* Just a security precaution so you don't rename someone you don't mean 
    * too --Shaddai
    */
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      send_to_char("That person is not on the mud.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You can't rename NPC's.\n\r", ch);
      return;
   }

   if (get_trust(ch) < get_trust(victim))
   {
      send_to_char("I don't think they would like that!\n\r", ch);
      return;
   }
   sprintf(newname, "%s%c/%s", PLAYER_DIR, tolower(arg2[0]), capitalize(arg2));
   sprintf(oldname, "%s%c/%s", PLAYER_DIR, tolower(victim->pcdata->filename[0]), capitalize(victim->pcdata->filename));
   sprintf(backname, "%s%c/%s", BACKUP_DIR, tolower(victim->pcdata->filename[0]), capitalize(victim->pcdata->filename));
   if (access(newname, F_OK) == 0)
   {
      send_to_char("That name already exists.\n\r", ch);
      return;
   }

   /* Have to remove the old god entry in the directories */
   if (IS_IMMORTAL(victim))
   {
      char godname[MSL];

      sprintf(godname, "%s%s", GOD_DIR, capitalize(victim->pcdata->filename));
      remove(godname);
   }

   /* Remember to change the names of the areas */
   if (ch->pcdata->area)
   {
      char filename[MSL];
      char newfilename[MSL];

      sprintf(filename, "%s%s.are", BUILD_DIR, victim->name);
      sprintf(newfilename, "%s%s.are", BUILD_DIR, capitalize(arg2));
      rename(filename, newfilename);
      sprintf(filename, "%s%s.are.bak", BUILD_DIR, victim->name);
      sprintf(newfilename, "%s%s.are.bak", BUILD_DIR, capitalize(arg2));
      rename(filename, newfilename);
   }

   STRFREE(victim->name);
   victim->name = STRALLOC(capitalize(arg2));
   STRFREE(victim->pcdata->filename);
   victim->pcdata->filename = STRALLOC(capitalize(arg2));
   remove(backname);
   if (remove(oldname))
   {
      sprintf(buf, "Error: Couldn't delete file %s in do_rename.", oldname);
      send_to_char("Couldn't delete the old file!\n\r", ch);
      log_string(oldname);
   }
   /* Time to save to force the affects to take place */
   save_char_obj(victim);

   /* Now lets update the wizlist */
   if (IS_IMMORTAL(victim))
      make_wizlist();
   send_to_char("Character was renamed.\n\r", ch);
   return;
}
