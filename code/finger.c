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
 *			           Finger Module                                  *
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

/* Finger snippet courtesy of unknown author. Installed by Samson 4-6-98 */
/* File read/write code redone using standard Smaug I/O routines - Samson 9-12-98 */
/* Data gathering now done via the pfiles, eliminated separate finger files - Samson 12-21-98 */
void do_finger(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char buf[MSL];
   char wbuf[MSL];

   buf[0] = '\0'; /* Clear out buffer, just in case. */

   if (IS_NPC(ch))
   {
      send_to_char("Mobs can't use the finger command.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Finger whom?\n\r", ch);
      return;
   }
   strcat(buf, "0.");
   strcat(buf, argument);
   victim = get_char_world(ch, buf);

   if ((victim == NULL) || (!victim))
   {
      read_finger(ch, argument);
      return;
   }
   if (!can_see_map(ch, victim))
   {
      send_to_char("Finger whom?\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on victim.\n\r", ch);
      return;
   }
   if (IS_SET(victim->pcdata->flags, PCFLAG_NOFINGER))
   {
      if (IS_IMMORTAL(victim) && !IS_IMMORTAL(ch))
      {
         send_to_char("Cannot finger an immortal.\n\r", ch);
         return;
      }
      if ((ch->level < victim->level && ch->level < LEVEL_STAFF) && IS_IMMORTAL(ch))
      {
         send_to_char("Cannot finger an immortal above your own level.\n\r", ch);
         return;
      }
   }
   sprintf(wbuf, "%s%s %s", victim->pcdata->pretit, victim->name, victim->last_name);
   if (str_cmp(PERS_MAP(victim, ch), wbuf))
   {
      read_finger(ch, argument);
      return;
   }
   send_to_char("&c&w          Finger Info\n\r", ch);
   send_to_char("          -----------\n\r", ch);
   ch_printf(ch, "&c&wName : &G%-20s &wMUD Age: &G%d\n\r", victim->name, get_age(victim));
   ch_printf(ch, "&c&wSex  : &G%-20s &w   Race: &G%s\n\r",
      victim->sex == SEX_MALE ? "Male" : victim->sex == SEX_FEMALE ? "Female" : "Neutral", capitalize(print_npc_race(victim->race)));
   ch_printf(ch, "&c&wTitle: &G%s\n\r", victim->pcdata->title);
   ch_printf(ch, "&c&wPreTitle: &G%s\n\r", victim->pcdata->pretit);
   ch_printf(ch, "&RPkills: &G%d   &RPdeaths: &G%d   &RPranking: &G%d\n\r",
      victim->pcdata->pkills, victim->pcdata->pdeaths, victim->pcdata->pranking);
   ch_printf(ch, "&c&wHomepage: &G%s\n\r", victim->pcdata->homepage != NULL ? victim->pcdata->homepage : "Not specified");
   ch_printf(ch, "&c&wEmail: &G%s\n\r", victim->pcdata->email != NULL ? victim->pcdata->email : "Not specified");
   ch_printf(ch, "&c&wKingdom: &G%s\n\r", kingdom_table[victim->pcdata->hometown]->name);
   ch_printf(ch, "&c&wHometown: &G%s\n\r", victim->pcdata->town ? victim->pcdata->town->name : "None");
   ch_printf(ch, "&c&wICQ#: &G%d\n\r", victim->pcdata->icq);
   ch_printf(ch, "&c&wLast on: &G%s\n\r", (char *) ctime(&ch->pcdata->logon));
   if (IS_IMMORTAL(ch))
   {

      char ipbuf[MSL];

      if (victim->desc)
         sprintf(ipbuf, "%s@%s", victim->desc->user, victim->desc->host);
      else
         strcpy(ipbuf, "(Link-Dead)");
      ch_printf(ch, "&c&wIP Info: &G%s\n\r", ipbuf);
      ch_printf(ch, "&c&wAuthorized by: &G%s\n\r", victim->pcdata->authed_by ? victim->pcdata->authed_by : "Unknown");
      ch_printf(ch, "&c&wCame From: &w&G%s\n\r", victim->pcdata->came_from ? victim->pcdata->came_from : "Unknown");
   }
   ch_printf(ch, "&wBio:\n\r&G%s\n\r", victim->pcdata->bio ? victim->pcdata->bio : "Not created");
   return;
}

void read_finger(CHAR_DATA * ch, char *argument)
{
   FILE *fpFinger;
   char fingload[MIL];
   char *laston = NULL;
   struct stat fst;

   fingload[0] = '\0';
   sprintf(fingload, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));

   if (stat(fingload, &fst) == -1)
   {
      send_to_char("&YNo such player exists.\n\r", ch);
      return;
   }
   laston = ctime(&fst.st_mtime);

   if (stat(fingload, &fst) != -1)
   {
      if ((fpFinger = fopen(fingload, "r")) != NULL)
      {
         for (;;)
         {
            char letter;
            char *word;

            letter = fread_letter(fpFinger);

            if (letter != '#')
               continue;

            word = fread_word(fpFinger);
            if (!str_cmp(word, "End"))
               break;

            if (!str_cmp(word, "PLAYER"))
               fread_finger(ch, fpFinger, laston);
            else if (!str_cmp(word, "END")) /* Done  */
               break;
         }
         fclose(fpFinger);
      }
   }
   return;
}

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

void fread_finger(CHAR_DATA * ch, FILE * fp, char *laston)
{
   char *word;
   char *email = NULL;
   char *homepage = NULL;
   char *name = NULL;
   char *town = NULL;
   char *site = NULL;
   char *title = NULL;
   char *bio = NULL;
   char *came_from = NULL;
   char *authed = NULL;
   char *temp = NULL;
   int race = 0, sex = 0, icq = 0, pranking = 0, pdeaths = 0, pkills = 0;
   int hometown = 0;
   bool fMatch;

   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'A':
            KEY("AuthedBy", authed, fread_string(fp));
            break;

         case 'B':
            KEY("Bio", bio, fread_string(fp));
            break;
            
         case 'C':
            KEY("CameFrom", came_from, fread_string(fp));
            break;

         case 'D':
            KEY("Description", temp, fread_string(fp));
            break;

         case 'E':
            if (!strcmp(word, "End"))
               goto finger_display;
            KEY("Email", email, fread_string_nohash(fp));
            break;

         case 'H':
            KEY("Homepage", homepage, fread_string_nohash(fp));
            if (!strcmp(word, "HomeTown"))
            {
               hometown = fread_number(fp);
               if (hometown >= sysdata.max_kingdom)
               {
                  bug("fread_finger: %s has an invalid kingdom value of %d", name, hometown);
                  hometown = 0;
               }
               fMatch = TRUE;
               break;
            }            
            break;

         case 'I':
            KEY("ICQ", icq, fread_number(fp));
            break;

         case 'N':
            KEY("Name", name, fread_string(fp));
            break;

         case 'P':
            KEY("PDeaths", pdeaths, fread_number(fp));
            KEY("PKills", pkills, fread_number(fp));
            KEY("PRanking", pranking, fread_number(fp));
            break;

         case 'R':
            KEY("Race", race, fread_number(fp));
            break;

         case 'S':
            KEY("Sex", sex, fread_number(fp));

            if (!strcmp(word, "Site"))
            {
               site = STRALLOC(fread_word(fp));
               fMatch = TRUE;
            }
            break;

         case 'T':
            KEY("Town", town, fread_string(fp));
            KEY("Title", title, fread_string(fp));
            break;
      }

      if (!fMatch)
         fread_to_eol(fp);
   }

/* Extremely ugly and disgusting goto hack, if there's a better way to
   do this, I'd sure like to know - Samson */

 finger_display:

   send_to_char("&c&w          Finger Info\n\r", ch);
   send_to_char("          -----------\n\r", ch);
   ch_printf(ch, "&&cwName : &G%-20s\n\r", name);
   ch_printf(ch, "&c&wSex  : &G%-20s &c&w   Race: &G%s\n\r",
      sex == SEX_MALE ? "Male" : sex == SEX_FEMALE ? "Female" : "Neutral", capitalize(print_npc_race(race)));
   ch_printf(ch, "&c&wTitle: &G%s\n\r", title);
   ch_printf(ch, "&RPkills: &G%d   &RPdeaths: &G%d   &RPranking: &G%d\n\r", pkills, pdeaths, pranking);
   ch_printf(ch, "&c&wHomepage: &G%s\n\r", homepage ? homepage : "Not specified");
   ch_printf(ch, "&c&wEmail: &G%s\n\r", email ? email : "Not specified");
   ch_printf(ch, "&c&wKingdom: &G%s\n\r", kingdom_table[hometown]->name);
   ch_printf(ch, "&c&wTown: &G%s\n\r", town);
   ch_printf(ch, "&c&wICQ#: &G%d\n\r", icq);
   ch_printf(ch, "&c&wLast on: &G%s\n\r", laston);
   if (IS_IMMORTAL(ch))
   {
      ch_printf(ch, "&c&wLast IP: &G%s\n\r", site);
      ch_printf(ch, "&c&wAuthorized by: &G%s\n\r", authed ? authed : "Unknown");
      ch_printf(ch, "&c&wCame from: &w&G%s\n\r", came_from ? came_from : "Unknown");
   }
   ch_printf(ch, "&c&wBio:\n\r&G%s\n\r", bio ? bio : "Not created");

   STRFREE(site);
   return;
}

/* Added a clone of homepage to let players input their email addy - Samson 4-18-98 */
void do_email(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];

   if (IS_NPC(ch))
      return;

   if (argument[0] == '\0')
   {
      if (!ch->pcdata->email)
         ch->pcdata->email = str_dup("");
      ch_printf(ch, "Your email address is: %s\n\r", show_tilde(ch->pcdata->email));
      return;
   }

   if (!str_cmp(argument, "clear"))
   {
      if (ch->pcdata->email)
         DISPOSE(ch->pcdata->email);
      ch->pcdata->email = str_dup("");
      send_to_char("Email address cleared.\n\r", ch);
      return;
   }

   strcpy(buf, argument);

   if (strlen(buf) > 70)
      buf[70] = '\0';

   hide_tilde(buf);
   if (ch->pcdata->email)
      DISPOSE(ch->pcdata->email);
   ch->pcdata->email = str_dup(buf);
   send_to_char("Email address set.\n\r", ch);
}

void do_icq_number(CHAR_DATA * ch, char *argument)
{
   int icq;

   if (IS_NPC(ch))
      return;

   if (argument[0] == '\0')
   {
      if (!ch->pcdata->icq)
         ch->pcdata->icq = 0;
      ch_printf(ch, "Your ICQ# is: %d\n\r", ch->pcdata->icq);
      return;
   }

   if (!str_cmp(argument, "clear"))
   {
      ch->pcdata->icq = 0;
      send_to_char("ICQ# cleared.\n\r", ch);
      return;
   }

   if (!is_number(argument))
   {
      send_to_char("You must enter numeric data.\n\r", ch);
      return;
   }

   icq = atoi(argument);

   if (icq < 1)
   {
      send_to_char("Valid range is greater than 0.\n\r", ch);
      return;
   }

   ch->pcdata->icq = icq;

   send_to_char("ICQ# set.\n\r", ch);
   return;
}
