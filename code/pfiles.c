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
 *			   Pfile autocleanup code				          *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "mud.h"

/* Globals */
time_t pfile_time;
HOUR_MIN_SEC set_pfile_time_struct;
HOUR_MIN_SEC *set_pfile_time;
struct tm *new_pfile_time;
struct tm new_pfile_struct;
time_t new_pfile_time_t;

void init_pfile_scan_time(void)
{
   /*
    * Init pfile scan time.
    */
   set_pfile_time = &set_pfile_time_struct;

   new_pfile_time = update_time(localtime(&current_time));
   /* Copies *new_pfile_time to new_pfile_struct, and then points
      new_pfile_time to new_pfile_struct again. -- Alty */
   new_pfile_struct = *new_pfile_time;
   new_pfile_time = &new_pfile_struct;
   new_pfile_time->tm_mday += 1;
   if (new_pfile_time->tm_hour > 12)
      new_pfile_time->tm_mday += 1;
   new_pfile_time->tm_sec = 0;
   new_pfile_time->tm_min = 0;
   new_pfile_time->tm_hour = 3;

   /* Update new_pfile_time (due to day increment) */
   new_pfile_time = update_time(new_pfile_time);
   new_pfile_struct = *new_pfile_time;
   new_pfile_time = &new_pfile_struct;
   /* Bug fix submitted by Gabe Yoder */
   new_pfile_time_t = mktime(new_pfile_time);
   /* check_pfiles(mktime(new_pfile_time)); */

   return;
}

time_t now_time;
int deleted = 0;
int days = 0;

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

void fread_pfile(FILE * fp, time_t tdiff, char *fname)
{
   char *word;
   char *name = NULL;
   int level = 0;
   int file_ver = 0;
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

         case 'E':
            if (!strcmp(word, "End"))
               goto timecheck;
            break;

         case 'L':
            KEY("Level", level, fread_number(fp));
            break;

         case 'N':
            KEY("Name", name, fread_string(fp));
            break;

         case 'V':
            KEY("Version", file_ver, fread_number(fp));
            break;
      }

      if (!fMatch)
         fread_to_eol(fp);
   }

 timecheck:

   if (tdiff > sysdata.newbie_purge)
   {
      if (level < LEVEL_IMMORTAL)
      {
         if (unlink(fname) == -1)
            perror("Unlink");
         else
         {
            days = sysdata.newbie_purge;
            sprintf(log_buf, "Player %s was deleted. Exceeded time limit of %d days.", name, days);
            log_string(log_buf);
#ifdef AUTO_AUTH
            remove_from_auth(name);
#endif
            ++deleted;
            return;
         }
      }
   }
}

void read_pfile(char *dirname, char *filename)
{
   FILE *fp;
   char fname[MSL];
   struct stat fst;
   time_t tdiff;

   now_time = time(0);

   sprintf(fname, "%s/%s", dirname, filename);

   if (stat(fname, &fst) != -1)
   {
      tdiff = (now_time - fst.st_mtime) / 86400;

      if ((fp = fopen(fname, "r")) != NULL)
      {
         for (;;)
         {
            char letter;
            char *word;

            letter = fread_letter(fp);

            if (letter != '#')
               continue;

            word = fread_word(fp);

            if (!str_cmp(word, "End"))
               break;

            if (!str_cmp(word, "PLAYER"))
               fread_pfile(fp, tdiff, fname);
            else if (!str_cmp(word, "END")) /* Done  */
               break;
         }
         FCLOSE(fp);
      }
   }
   return;
}

void pfile_scan(void)
{
   DIR *dp;
   struct dirent *dentry;
   char dir_name[100];

   int alpha_loop;
   int cou = 0;

#ifdef RENTCODE
   extern int num_pfiles;
#endif

   now_time = time(0);
   nice(20);

   for (alpha_loop = 0; alpha_loop <= 25; alpha_loop++)
   {
      sprintf(dir_name, "%s%c", PLAYER_DIR, 'a' + alpha_loop);
      dp = opendir(dir_name);
      dentry = readdir(dp);
      while (dentry)
      {
         if (dentry->d_name[0] != '.')
         {
            read_pfile(dir_name, dentry->d_name);
            cou++;
         }
         dentry = readdir(dp);
      }
      closedir(dp);
   }

   log_string("Pfile cleanup completed.");

   sprintf(log_buf, "Total pfiles scanned: %d", cou);
   log_string(log_buf);

   sprintf(log_buf, "Total pfiles deleted: %d", deleted);
   log_string(log_buf);

   sprintf(log_buf, "Total pfiles remaining: %d", cou - deleted);
#ifdef RENTCODE
   num_pfiles = cou - deleted;
#endif
   log_string(log_buf);

   return;
}

void do_pfiles(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot use this command!\n\r", ch);
      return;
   }

   /* Makes a backup copy of existing pfiles just in case - Samson */
   strcpy(buf, "tar -cf ");
   strcat(buf, PLAYER_DIR);
   strcat(buf, "pfiles.tar ");
   strcat(buf, PLAYER_DIR);
   strcat(buf, "*/*");

   /* GAH, the shell pipe won't process the command that gets pieced
      together in the preceeding lines! God only knows why. - Samson */
   system(buf);

   sprintf(log_buf, "Manual pfile cleanup started by %s.", ch->name);
   log_string(log_buf);
   pfile_scan();

   return;
}

void save_sysdata args((SYSTEM_DATA sys));

void check_pfiles(time_t reset)
{

   if (new_pfile_time_t <= current_time)
   {
      if (sysdata.CLEANPFILES == TRUE)
      {

         char buf[MSL];

         /* Makes a backup copy of existing pfiles just in case - Samson */
         strcpy(buf, "tar -cf ");
         strcat(buf, PLAYER_DIR);
         strcat(buf, "pfiles.tar ");
         strcat(buf, PLAYER_DIR);
         strcat(buf, "*/*");

         /* Would use the shell pipe for this, but alas, it requires a ch in order
            to work, and I can't figure a way to get around that. Not that it
            matters anyway, the shell pipe didn't like the command for some
            reason that's WAY over my head. - Samson */
         system(buf);

         new_pfile_time_t = current_time + 86400;
         sysdata.purgetime = new_pfile_time_t;
         save_sysdata(sysdata);
         log_string("Automated pfile cleanup beginning....");
         pfile_scan();
      }
      else
      {
         new_pfile_time_t = current_time + 86400;
         sysdata.purgetime = new_pfile_time_t;
         save_sysdata(sysdata);
      }
   }
   return;
}
