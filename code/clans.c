/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 *			     Special clan module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
/* #include <stdlib.h> */
#include <time.h>
#include "mud.h"



#define MAX_NEST	100
static OBJ_DATA *rgObjNest[MAX_NEST];

CLAN_DATA *first_clan;
CLAN_DATA *last_clan;
COUNCIL_DATA *first_council;
COUNCIL_DATA *last_council;

/* local routines */
void fread_clan args((CLAN_DATA * clan, FILE * fp));
bool load_clan_file args((char *clanfile));
void write_clan_list args((void));

void fread_council args((COUNCIL_DATA * council, FILE * fp));
bool load_council_file args((char *councilfile));
void write_council_list args((void));

/*
 * Get pointer to clan structure from clan name.
 */
CLAN_DATA *get_clan(char *name)
{
   CLAN_DATA *clan;

   for (clan = first_clan; clan; clan = clan->next)
      if (!str_cmp(name, clan->name))
         return clan;
   return NULL;
}

COUNCIL_DATA *get_council(char *name)
{
   COUNCIL_DATA *council;

   for (council = first_council; council; council = council->next)
      if (!str_cmp(name, council->name))
         return council;
   return NULL;
}

void write_clan_list()
{
   CLAN_DATA *tclan;
   FILE *fpout;
   char filename[256];

   sprintf(filename, "%s%s", CLAN_DIR, CLAN_LIST);
   fpout = fopen(filename, "w");
   if (!fpout)
   {
      bug("FATAL: cannot open clan.lst for writing!\n\r", 0);
      return;
   }
   for (tclan = first_clan; tclan; tclan = tclan->next)
      fprintf(fpout, "%s\n", tclan->filename);
   fprintf(fpout, "$\n");
   fclose(fpout);
}

void write_council_list()
{
   COUNCIL_DATA *tcouncil;
   FILE *fpout;
   char filename[256];

   sprintf(filename, "%s%s", COUNCIL_DIR, COUNCIL_LIST);
   fpout = fopen(filename, "w");
   if (!fpout)
   {
      bug("FATAL: cannot open council.lst for writing!\n\r", 0);
      return;
   }
   for (tcouncil = first_council; tcouncil; tcouncil = tcouncil->next)
      fprintf(fpout, "%s\n", tcouncil->filename);
   fprintf(fpout, "$\n");
   fclose(fpout);
}

/*
 * Save a clan's data to its data file
 */
void save_clan(CLAN_DATA * clan)
{
   FILE *fp;
   char filename[256];
   char buf[MSL];

   if (!clan)
   {
      bug("save_clan: null clan pointer!", 0);
      return;
   }

   if (!clan->filename || clan->filename[0] == '\0')
   {
      sprintf(buf, "save_clan: %s has no filename", clan->name);
      bug(buf, 0);
      return;
   }

   sprintf(filename, "%s%s", CLAN_DIR, clan->filename);

   fclose(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_clan: fopen", 0);
      perror(filename);
   }
   else
   {
      fprintf(fp, "#CLAN\n");
      fprintf(fp, "Name         %s~\n", clan->name);
      fprintf(fp, "Filename     %s~\n", clan->filename);
      fprintf(fp, "Motto        %s~\n", clan->motto);
      fprintf(fp, "Description  %s~\n", clan->description);
      fprintf(fp, "Deity        %s~\n", clan->deity);
      fprintf(fp, "Leader       %s~\n", clan->leader);
      fprintf(fp, "NumberOne    %s~\n", clan->number1);
      fprintf(fp, "NumberTwo    %s~\n", clan->number2);
      fprintf(fp, "Badge        %s~\n", clan->badge);
      fprintf(fp, "PKillRangeNew   %d %d %d %d %d %d %d\n",
         clan->pkills[0], clan->pkills[1], clan->pkills[2], clan->pkills[3], clan->pkills[4], clan->pkills[5], clan->pkills[6]);
      fprintf(fp, "PDeathRangeNew  %d %d %d %d %d %d %d\n",
         clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2], clan->pdeaths[3], clan->pdeaths[4], clan->pdeaths[5], clan->pdeaths[6]);
      fprintf(fp, "MKills       %d\n", clan->mkills);
      fprintf(fp, "MDeaths      %d\n", clan->mdeaths);
      fprintf(fp, "IllegalPK    %d\n", clan->illegal_pk);
      fprintf(fp, "Score        %d\n", clan->score);
      fprintf(fp, "Type         %d\n", clan->clan_type);
      fprintf(fp, "Favour       %d\n", clan->favour);
      fprintf(fp, "Strikes      %d\n", clan->strikes);
      fprintf(fp, "Members      %d\n", clan->members);
      fprintf(fp, "MemLimit     %d\n", clan->mem_limit);
      fprintf(fp, "Alignment    %d\n", clan->alignment);
      fprintf(fp, "Board        %d\n", clan->board);
      fprintf(fp, "ClanObjOne   %d\n", clan->clanobj1);
      fprintf(fp, "ClanObjTwo   %d\n", clan->clanobj2);
      fprintf(fp, "ClanObjThree %d\n", clan->clanobj3);
      fprintf(fp, "ClanObjFour  %d\n", clan->clanobj4);
      fprintf(fp, "ClanObjFive  %d\n", clan->clanobj5);
      fprintf(fp, "Recall       %d\n", clan->recall);
      fprintf(fp, "Storeroom    %d\n", clan->storeroom);
      fprintf(fp, "GuardOne     %d\n", clan->guard1);
      fprintf(fp, "GuardTwo     %d\n", clan->guard2);
      fprintf(fp, "End\n\n");
      fprintf(fp, "#END\n");
   }
   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

/*
 * Save a council's data to its data file
 */
void save_council(COUNCIL_DATA * council)
{
   FILE *fp;
   char filename[256];
   char buf[MSL];

   if (!council)
   {
      bug("save_council: null council pointer!", 0);
      return;
   }

   if (!council->filename || council->filename[0] == '\0')
   {
      sprintf(buf, "save_council: %s has no filename", council->name);
      bug(buf, 0);
      return;
   }

   sprintf(filename, "%s%s", COUNCIL_DIR, council->filename);

   fclose(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_council: fopen", 0);
      perror(filename);
   }
   else
   {
      fprintf(fp, "#COUNCIL\n");
      fprintf(fp, "Name         %s~\n", council->name);
      fprintf(fp, "Filename     %s~\n", council->filename);
      fprintf(fp, "Description  %s~\n", council->description);
      fprintf(fp, "Head         %s~\n", council->head);
      if (council->head2 != NULL)
         fprintf(fp, "Head2        %s~\n", council->head2);
      fprintf(fp, "Members      %d\n", council->members);
      fprintf(fp, "Board        %d\n", council->board);
      fprintf(fp, "Meeting      %d\n", council->meeting);
      fprintf(fp, "Powers       %s~\n", council->powers);
      fprintf(fp, "End\n\n");
      fprintf(fp, "#END\n");
   }
   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}


/*
 * Read in actual clan data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

/*
 * Reads in PKill and PDeath still for backward compatibility but now it
 * should be written to PKillRange and PDeathRange for multiple level pkill
 * tracking support. --Shaddai
 * Added a hardcoded limit memlimit to the amount of members a clan can 
 * have set using setclan.  --Shaddai
 */

void fread_clan(CLAN_DATA * clan, FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;

   clan->mem_limit = 0; /* Set up defaults */
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
            KEY("Alignment", clan->alignment, fread_number(fp));
            break;

         case 'B':
            KEY("Badge", clan->badge, fread_string(fp));
            KEY("Board", clan->board, fread_number(fp));
            break;

         case 'C':
            KEY("ClanObjOne", clan->clanobj1, fread_number(fp));
            KEY("ClanObjTwo", clan->clanobj2, fread_number(fp));
            KEY("ClanObjThree", clan->clanobj3, fread_number(fp));
            KEY("ClanObjFour", clan->clanobj4, fread_number(fp));
            KEY("ClanObjFive", clan->clanobj5, fread_number(fp));
            break;

         case 'D':
            KEY("Deity", clan->deity, fread_string(fp));
            KEY("Description", clan->description, fread_string(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               if (!clan->name)
                  clan->name = STRALLOC("");
               if (!clan->leader)
                  clan->leader = STRALLOC("");
               if (!clan->description)
                  clan->description = STRALLOC("");
               if (!clan->motto)
                  clan->motto = STRALLOC("");
               if (!clan->number1)
                  clan->number1 = STRALLOC("");
               if (!clan->number2)
                  clan->number2 = STRALLOC("");
               if (!clan->deity)
                  clan->deity = STRALLOC("");
               if (!clan->badge)
                  clan->badge = STRALLOC("");
               return;
            }
            break;

         case 'F':
            KEY("Favour", clan->favour, fread_number(fp));
            KEY("Filename", clan->filename, fread_string_nohash(fp));

         case 'G':
            KEY("GuardOne", clan->guard1, fread_number(fp));
            KEY("GuardTwo", clan->guard2, fread_number(fp));
            break;

         case 'I':
            KEY("IllegalPK", clan->illegal_pk, fread_number(fp));
            break;

         case 'L':
            KEY("Leader", clan->leader, fread_string(fp));
            break;

         case 'M':
            KEY("MDeaths", clan->mdeaths, fread_number(fp));
            KEY("Members", clan->members, fread_number(fp));
            KEY("MemLimit", clan->mem_limit, fread_number(fp));
            KEY("MKills", clan->mkills, fread_number(fp));
            KEY("Motto", clan->motto, fread_string(fp));
            break;

         case 'N':
            KEY("Name", clan->name, fread_string(fp));
            KEY("NumberOne", clan->number1, fread_string(fp));
            KEY("NumberTwo", clan->number2, fread_string(fp));
            break;

         case 'P':
            KEY("PDeaths", clan->pdeaths[6], fread_number(fp));
            KEY("PKills", clan->pkills[6], fread_number(fp));
            /* Addition of New Ranges */
            if (!str_cmp(word, "PDeathRange"))
            {
               fMatch = TRUE;
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
            }
            if (!str_cmp(word, "PDeathRangeNew"))
            {
               fMatch = TRUE;
               clan->pdeaths[0] = fread_number(fp);
               clan->pdeaths[1] = fread_number(fp);
               clan->pdeaths[2] = fread_number(fp);
               clan->pdeaths[3] = fread_number(fp);
               clan->pdeaths[4] = fread_number(fp);
               clan->pdeaths[5] = fread_number(fp);
               clan->pdeaths[6] = fread_number(fp);
            }
            if (!str_cmp(word, "PKillRangeNew"))
            {
               fMatch = TRUE;
               clan->pkills[0] = fread_number(fp);
               clan->pkills[1] = fread_number(fp);
               clan->pkills[2] = fread_number(fp);
               clan->pkills[3] = fread_number(fp);
               clan->pkills[4] = fread_number(fp);
               clan->pkills[5] = fread_number(fp);
               clan->pkills[6] = fread_number(fp);
            }
            if (!str_cmp(word, "PKillRange"))
            {
               fMatch = TRUE;
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
               fread_number(fp);
            }
            break;

         case 'R':
            KEY("Recall", clan->recall, fread_number(fp));
            break;

         case 'S':
            KEY("Score", clan->score, fread_number(fp));
            KEY("Strikes", clan->strikes, fread_number(fp));
            KEY("Storeroom", clan->storeroom, fread_number(fp));
            break;

         case 'T':
            KEY("Type", clan->clan_type, fread_number(fp));
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_clan: no match: %s", word);
         bug(buf, 0);
      }
   }
}

/*
 * Read in actual council data.
 */

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}

void fread_council(COUNCIL_DATA * council, FILE * fp)
{
   char buf[MSL];
   char *word;
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

         case 'B':
            KEY("Board", council->board, fread_number(fp));
            break;

         case 'D':
            KEY("Description", council->description, fread_string(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               if (!council->name)
                  council->name = STRALLOC("");
               if (!council->description)
                  council->description = STRALLOC("");
               if (!council->powers)
                  council->powers = STRALLOC("");
               return;
            }
            break;

         case 'F':
            KEY("Filename", council->filename, fread_string_nohash(fp));
            break;

         case 'H':
            KEY("Head", council->head, fread_string(fp));
            KEY("Head2", council->head2, fread_string(fp));
            break;

         case 'M':
            KEY("Members", council->members, fread_number(fp));
            KEY("Meeting", council->meeting, fread_number(fp));
            break;

         case 'N':
            KEY("Name", council->name, fread_string(fp));
            break;

         case 'P':
            KEY("Powers", council->powers, fread_string(fp));
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_council: no match: %s", word);
         bug(buf, 0);
      }
   }
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

void get_version(FILE * fp)
{
   char *word;
   bool fMatch;
   char buf[MSL];

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

         case 'V':
            KEY("Version", file_ver, fread_number(fp));
            break;

         case 'E':
            if (!str_cmp(word, "End"))
               return;
      }
      if (!fMatch)
      {
         sprintf(buf, "get_version: no match: %s", word);
         bug(buf, 0);
      }
   }
}

/*
 * Load a clan file
 */

bool load_clan_file(char *clanfile)
{
   char filename[256];
   CLAN_DATA *clan;
   FILE *fp;
   bool found;

   CREATE(clan, CLAN_DATA, 1);

   /* Make sure we default these to 0 --Shaddai */
   clan->pkills[0] = 0;
   clan->pkills[1] = 0;
   clan->pkills[2] = 0;
   clan->pkills[3] = 0;
   clan->pkills[4] = 0;
   clan->pkills[5] = 0;
   clan->pkills[6] = 0;
   clan->pdeaths[0] = 0;
   clan->pdeaths[1] = 0;
   clan->pdeaths[2] = 0;
   clan->pdeaths[3] = 0;
   clan->pdeaths[4] = 0;
   clan->pdeaths[5] = 0;
   clan->pdeaths[6] = 0;

   found = FALSE;
   sprintf(filename, "%s%s", CLAN_DIR, clanfile);

   if ((fp = fopen(filename, "r")) != NULL)
   {

      found = TRUE;
      for (;;)
      {
         char letter;
         char *word;

         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            bug("Load_clan_file: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "CLAN"))
         {
            fread_clan(clan, fp);
            break;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            char buf[MSL];

            sprintf(buf, "Load_clan_file: bad section: %s.", word);
            bug(buf, 0);
            break;
         }
      }
      fclose(fp);
   }

   if (found)
   {
      ROOM_INDEX_DATA *storeroom;

      LINK(clan, first_clan, last_clan, next, prev);

      if (clan->storeroom == 0 || (storeroom = get_room_index(clan->storeroom)) == NULL)
      {
         log_string("Storeroom not found");
         return found;
      }

      sprintf(filename, "%s%s.vault", CLAN_DIR, clan->filename);
      if ((fp = fopen(filename, "r")) != NULL)
      {
         int iNest;
         bool found;
         OBJ_DATA *tobj, *tobj_next;

         log_string("Loading clan storage room");
         rset_supermob(storeroom);
         for (iNest = 0; iNest < MAX_NEST; iNest++)
            rgObjNest[iNest] = NULL;

         found = TRUE;
         for (;;)
         {
            char letter;
            char *word;

            letter = fread_letter(fp);
            if (letter == '*')
            {
               fread_to_eol(fp);
               continue;
            }

            if (letter != '#')
            {
               bug("Load_clan_vault: # not found.", 0);
               bug(clan->name, 0);
               break;
            }

            word = fread_word(fp);
            if (!str_cmp(word, "OBJECT")) /* Objects */
               fread_obj(supermob, fp, OS_CARRY);
            else if (!str_cmp(word, "VERSION")) // Version
               get_version(fp);
            else if (!str_cmp(word, "END")) /* Done  */
               break;
            else
            {
               bug("Load_clan_vault: bad section.", 0);
               bug(clan->name, 0);
               break;
            }
         }
         file_ver = 0;
         fclose(fp);
         for (tobj = supermob->first_carrying; tobj; tobj = tobj_next)
         {
            tobj_next = tobj->next_content;
            obj_from_char(tobj);
            obj_to_room(tobj, storeroom, supermob);
         }
         release_supermob();
      }
      else
         log_string("Cannot open clan vault");
   }
   else
      DISPOSE(clan);

   return found;
}

/*
 * Load a council file
 */

bool load_council_file(char *councilfile)
{
   char filename[256];
   COUNCIL_DATA *council;
   FILE *fp;
   bool found;

   CREATE(council, COUNCIL_DATA, 1);

   found = FALSE;
   sprintf(filename, "%s%s", COUNCIL_DIR, councilfile);

   if ((fp = fopen(filename, "r")) != NULL)
   {

      found = TRUE;
      for (;;)
      {
         char letter;
         char *word;

         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            bug("Load_council_file: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "COUNCIL"))
         {
            fread_council(council, fp);
            break;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            bug("Load_council_file: bad section.", 0);
            break;
         }
      }
      fclose(fp);
   }

   if (found)
      LINK(council, first_council, last_council, next, prev);

   else
      DISPOSE(council);

   return found;
}

/*
 * Load in all the clan files.
 */
void load_clans()
{
   FILE *fpList;
   char *filename;
   char clanlist[256];
   char buf[MSL];


   first_clan = NULL;
   last_clan = NULL;

   log_string("Loading clans...");

   sprintf(clanlist, "%s%s", CLAN_DIR, CLAN_LIST);
   fclose(fpReserve);
   if ((fpList = fopen(clanlist, "r")) == NULL)
   {
      perror(clanlist);
      exit(1);
   }

   for (;;)
   {
      filename = feof(fpList) ? "$" : fread_word(fpList);
      log_string(filename);
      if (filename[0] == '$')
         break;

      if (!load_clan_file(filename))
      {
         sprintf(buf, "Cannot load clan file: %s", filename);
         bug(buf, 0);
      }
   }
   fclose(fpList);
   log_string(" Done clans ");
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

/*
 * Load in all the council files.
 */
void load_councils()
{
   FILE *fpList;
   char *filename;
   char councillist[256];
   char buf[MSL];


   first_council = NULL;
   last_council = NULL;

   log_string("Loading councils...");

   sprintf(councillist, "%s%s", COUNCIL_DIR, COUNCIL_LIST);
   fclose(fpReserve);
   if ((fpList = fopen(councillist, "r")) == NULL)
   {
      perror(councillist);
      exit(1);
   }

   for (;;)
   {
      filename = feof(fpList) ? "$" : fread_word(fpList);
      log_string(filename);
      if (filename[0] == '$')
         break;

      if (!load_council_file(filename))
      {
         sprintf(buf, "Cannot load council file: %s", filename);
         bug(buf, 0);
      }
   }
   fclose(fpList);
   log_string(" Done councils ");
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

void do_make(CHAR_DATA * ch, char *argument)
{
   send_to_char("Not used anymore.\n\r", ch);
   return;   
}

void do_induct(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   CLAN_DATA *clan;

   if (IS_NPC(ch) || !ch->pcdata->clan)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   clan = ch->pcdata->clan;

   if ((ch->pcdata && ch->pcdata->bestowments
         && is_name("induct", ch->pcdata->bestowments))
      || !str_cmp(ch->name, clan->deity)
      || !str_cmp(ch->name, clan->leader) || !str_cmp(ch->name, clan->number1) || !str_cmp(ch->name, clan->number2))
      ;
   else
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Induct whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("That player is not here.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }

   if (IS_IMMORTAL(victim))
   {
      send_to_char("You can't induct such a godly presence.\n\r", ch);
      return;
   }

   if (victim->pcdata->clan)
   {
      if (victim->pcdata->clan->clan_type == CLAN_ORDER)
      {
         if (victim->pcdata->clan == clan)
            send_to_char("This player already belongs to your order!\n\r", ch);
         else
            send_to_char("This player already belongs to an order!\n\r", ch);
         return;
      }
      else if (victim->pcdata->clan->clan_type == CLAN_GUILD)
      {
         if (victim->pcdata->clan == clan)
            send_to_char("This player already belongs to your guild!\n\r", ch);
         else
            send_to_char("This player already belongs to an guild!\n\r", ch);
         return;
      }
      else
      {
         if (victim->pcdata->clan == clan)
            send_to_char("This player already belongs to your clan!\n\r", ch);
         else
            send_to_char("This player already belongs to a clan!\n\r", ch);
         return;
      }
   }
   if (clan->mem_limit && clan->members >= clan->mem_limit)
   {
      send_to_char("Your clan is too big to induct anymore players.\n\r", ch);
      return;
   }
   clan->members++;
   if (clan->clan_type != CLAN_ORDER && clan->clan_type != CLAN_GUILD)
      SET_BIT(victim->speaks, LANG_CLAN);

   victim->pcdata->clan = clan;
   STRFREE(victim->pcdata->clan_name);
   victim->pcdata->clan_name = QUICKLINK(clan->name);
   act(AT_MAGIC, "You induct $N into $t", ch, clan->name, victim, TO_CHAR);
   act(AT_MAGIC, "$n inducts $N into $t", ch, clan->name, victim, TO_NOTVICT);
   act(AT_MAGIC, "$n inducts you into $t", ch, clan->name, victim, TO_VICT);
   save_char_obj(victim);
   add_player_list(victim, 1);
   save_clan(clan);
   return;
}

void do_council_induct(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   COUNCIL_DATA *council;

   if (IS_NPC(ch) || !ch->pcdata->council)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   council = ch->pcdata->council;

   if ((council->head == NULL || str_cmp(ch->name, council->head))
      && (council->head2 == NULL || str_cmp(ch->name, council->head2)) && str_cmp(council->name, "mortal council"))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Induct whom into your council?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("That player is not here.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }

   if (victim->pcdata->council)
   {
      send_to_char("This player already belongs to a council!\n\r", ch);
      return;
   }

   council->members++;
   victim->pcdata->council = council;
   STRFREE(victim->pcdata->council_name);
   victim->pcdata->council_name = QUICKLINK(council->name);
   act(AT_MAGIC, "You induct $N into $t", ch, council->name, victim, TO_CHAR);
   act(AT_MAGIC, "$n inducts $N into $t", ch, council->name, victim, TO_ROOM);
   act(AT_MAGIC, "$n inducts you into $t", ch, council->name, victim, TO_VICT);
   save_char_obj(victim);
   save_council(council);
   return;
}

void do_outcast(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   CLAN_DATA *clan;

   if (IS_NPC(ch) || !ch->pcdata->clan)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   clan = ch->pcdata->clan;

   if ((ch->pcdata && ch->pcdata->bestowments
         && is_name("outcast", ch->pcdata->bestowments))
      || !str_cmp(ch->name, clan->deity)
      || !str_cmp(ch->name, clan->leader) || !str_cmp(ch->name, clan->number1) || !str_cmp(ch->name, clan->number2))
      ;
   else
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Outcast whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("That player is not here.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      if (ch->pcdata->clan->clan_type == CLAN_ORDER)
      {
         send_to_char("Kick yourself out of your own order?\n\r", ch);
         return;
      }
      else if (ch->pcdata->clan->clan_type == CLAN_GUILD)
      {
         send_to_char("Kick yourself out of your own guild?\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Kick yourself out of your own clan?\n\r", ch);
         return;
      }
   }

   if (victim->level > ch->level)
   {
      send_to_char("This player is too powerful for you to outcast.\n\r", ch);
      return;
   }

   if (victim->pcdata->clan != ch->pcdata->clan)
   {
      if (ch->pcdata->clan->clan_type == CLAN_ORDER)
      {
         send_to_char("This player does not belong to your order!\n\r", ch);
         return;
      }
      else if (ch->pcdata->clan->clan_type == CLAN_GUILD)
      {
         send_to_char("This player does not belong to your guild!\n\r", ch);
         return;
      }
      else
      {
         send_to_char("This player does not belong to your clan!\n\r", ch);
         return;
      }
   }

   if (victim->speaking & LANG_CLAN)
      victim->speaking = LANG_COMMON;
   REMOVE_BIT(victim->speaks, LANG_CLAN);
   --clan->members;
   if (!str_cmp(victim->name, ch->pcdata->clan->number1))
   {
      STRFREE(ch->pcdata->clan->number1);
      ch->pcdata->clan->number1 = STRALLOC("");
   }
   if (!str_cmp(victim->name, ch->pcdata->clan->number2))
   {
      STRFREE(ch->pcdata->clan->number2);
      ch->pcdata->clan->number2 = STRALLOC("");
   }
   victim->pcdata->clan = NULL;
   STRFREE(victim->pcdata->clan_name);
   victim->pcdata->clan_name = STRALLOC("");
   remove_player_list(victim, 1);
   act(AT_MAGIC, "You outcast $N from $t", ch, clan->name, victim, TO_CHAR);
   act(AT_MAGIC, "$n outcasts $N from $t", ch, clan->name, victim, TO_ROOM);
   act(AT_MAGIC, "$n outcasts you from $t", ch, clan->name, victim, TO_VICT);

/* Outcast flag setting removed by Narn.  It's useless now that deadlies
   remain deadly even on being cast out of a clan.
*/
/*    if ( clan->clan_type != CLAN_GUILD )
	xSET_BIT(victim->act, PLR_OUTCAST);
*/
   save_char_obj(victim); /* clan gets saved when pfile is saved */
   save_clan(clan);
   return;
}

void do_council_outcast(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   COUNCIL_DATA *council;

   if (IS_NPC(ch) || !ch->pcdata->council)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   council = ch->pcdata->council;

   if ((council->head == NULL || str_cmp(ch->name, council->head))
      && (council->head2 == NULL || str_cmp(ch->name, council->head2)) && str_cmp(council->name, "mortal council"))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Outcast whom from your council?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("That player is not here.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Kick yourself out of your own council?\n\r", ch);
      return;
   }

   if (victim->pcdata->council != ch->pcdata->council)
   {
      send_to_char("This player does not belong to your council!\n\r", ch);
      return;
   }

   --council->members;
   victim->pcdata->council = NULL;
   STRFREE(victim->pcdata->council_name);
   victim->pcdata->council_name = STRALLOC("");
   act(AT_MAGIC, "You outcast $N from $t", ch, council->name, victim, TO_CHAR);
   act(AT_MAGIC, "$n outcasts $N from $t", ch, council->name, victim, TO_ROOM);
   act(AT_MAGIC, "$n outcasts you from $t", ch, council->name, victim, TO_VICT);
   save_char_obj(victim);
   save_council(council);
   return;
}

void do_setclan(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CLAN_DATA *clan;

   set_char_color(AT_PLAIN, ch);
   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Usage: setclan <clan> <field> <deity|leader|number1|number2> <player>\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char(" deity leader number1 number2\n\r", ch);
      send_to_char(" members board recall storage guard1 guard2\n\r", ch);
      send_to_char(" align (not functional) memlimit", ch);
      send_to_char(" obj1 obj2 obj3 obj4 obj5\n\r", ch);
      if (get_trust(ch) >= LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char(" name filename motto desc\n\r", ch);
         send_to_char(" favour strikes type\n\r", ch);
      }
      if (get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
         send_to_char(" pkill1-7 pdeath1-7\n\r", ch);
      return;
   }

   clan = get_clan(arg1);
   if (!clan)
   {
      send_to_char("No such clan.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "deity"))
   {
      STRFREE(clan->deity);
      clan->deity = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "leader"))
   {
      STRFREE(clan->leader);
      clan->leader = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "number1"))
   {
      STRFREE(clan->number1);
      clan->number1 = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "number2"))
   {
      STRFREE(clan->number2);
      clan->number2 = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "badge"))
   {
      STRFREE(clan->badge);
      clan->badge = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "board"))
   {
      clan->board = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "memlimit"))
   {
      clan->mem_limit = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "members"))
   {
      clan->members = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "recall"))
   {
      clan->recall = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "storage"))
   {
      clan->storeroom = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "obj1"))
   {
      clan->clanobj1 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "obj2"))
   {
      clan->clanobj2 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "obj3"))
   {
      clan->clanobj3 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "obj4"))
   {
      clan->clanobj4 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "obj5"))
   {
      clan->clanobj5 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "guard1"))
   {
      clan->guard1 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "guard2"))
   {
      clan->guard2 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
   {
      do_setclan(ch, "");
      return;
   }
   if (!str_cmp(arg2, "align"))
   {
      clan->alignment = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "type"))
   {
      if (!str_cmp(argument, "order"))
         clan->clan_type = CLAN_ORDER;
      else if (!str_cmp(argument, "guild"))
         clan->clan_type = CLAN_GUILD;
      else
         clan->clan_type = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "name"))
   {
      STRFREE(clan->name);
      clan->name = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "filename"))
   {
      DISPOSE(clan->filename);
      clan->filename = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      write_clan_list();
      return;
   }
   if (!str_cmp(arg2, "motto"))
   {
      STRFREE(clan->motto);
      clan->motto = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (!str_cmp(arg2, "desc"))
   {
      STRFREE(clan->description);
      clan->description = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_clan(clan);
      return;
   }
   if (get_trust(ch) < LEVEL_HI_STAFF) /* Tracker1 */
   {
      do_setclan(ch, "");
      return;
   }
   if (!str_prefix("pkill", arg2))
   {
      int temp_value;

      if (!str_cmp(arg2, "pkill1"))
         temp_value = 0;
      else if (!str_cmp(arg2, "pkill2"))
         temp_value = 1;
      else if (!str_cmp(arg2, "pkill3"))
         temp_value = 2;
      else if (!str_cmp(arg2, "pkill4"))
         temp_value = 3;
      else if (!str_cmp(arg2, "pkill5"))
         temp_value = 4;
      else if (!str_cmp(arg2, "pkill6"))
         temp_value = 5;
      else if (!str_cmp(arg2, "pkill7"))
         temp_value = 6;
      else
      {
         do_setclan(ch, "");
         return;
      }
      clan->pkills[temp_value] = atoi(argument);
      send_to_char("Ok.\n\r", ch);
      return;
   }
   if (!str_prefix("pdeath", arg2))
   {
      int temp_value;

      if (!str_cmp(arg2, "pdeath1"))
         temp_value = 0;
      else if (!str_cmp(arg2, "pdeath2"))
         temp_value = 1;
      else if (!str_cmp(arg2, "pdeath3"))
         temp_value = 2;
      else if (!str_cmp(arg2, "pdeath4"))
         temp_value = 3;
      else if (!str_cmp(arg2, "pdeath5"))
         temp_value = 4;
      else if (!str_cmp(arg2, "pdeath6"))
         temp_value = 5;
      else if (!str_cmp(arg2, "pdeath7"))
         temp_value = 6;
      else
      {
         do_setclan(ch, "");
         return;
      }
      clan->pdeaths[temp_value] = atoi(argument);
      send_to_char("Ok.\n\r", ch);
      return;
   }
   do_setclan(ch, "");
   return;
}

void do_setcouncil(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   COUNCIL_DATA *council;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Usage: setcouncil <council> <field> <deity|leader|number1|number2> <player>\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char(" head head2 members board meeting\n\r", ch);
      if (get_trust(ch) >= LEVEL_STAFF) /* Tracker1 */
         send_to_char(" name filename desc\n\r", ch);
      if (get_trust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
         send_to_char(" powers\n\r", ch);
      return;
   }

   council = get_council(arg1);
   if (!council)
   {
      send_to_char("No such council.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "head"))
   {
      STRFREE(council->head);
      council->head = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }

   if (!str_cmp(arg2, "head2"))
   {
      if (council->head2 != NULL)
         STRFREE(council->head2);
      if (!str_cmp(argument, "none") || !str_cmp(argument, "clear"))
         council->head2 = NULL;
      else
         council->head2 = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }
   if (!str_cmp(arg2, "board"))
   {
      council->board = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }
   if (!str_cmp(arg2, "members"))
   {
      council->members = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }
   if (!str_cmp(arg2, "meeting"))
   {
      council->meeting = atoi(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }
   if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
   {
      do_setcouncil(ch, "");
      return;
   }
   if (!str_cmp(arg2, "name"))
   {
      STRFREE(council->name);
      council->name = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }
   if (!str_cmp(arg2, "filename"))
   {
      DISPOSE(council->filename);
      council->filename = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      write_council_list();
      return;
   }
   if (!str_cmp(arg2, "desc"))
   {
      STRFREE(council->description);
      council->description = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }
   if (get_trust(ch) < LEVEL_HI_STAFF) /* Tracker1 */
   {
      do_setcouncil(ch, "");
      return;
   }
   if (!str_cmp(arg2, "powers"))
   {
      STRFREE(council->powers);
      council->powers = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      save_council(council);
      return;
   }

   do_setcouncil(ch, "");
   return;
}

/*
 * Added multiple levels on pkills and pdeaths. -- Shaddai
 */

void do_showclan(CHAR_DATA * ch, char *argument)
{
   CLAN_DATA *clan;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Usage: showclan <clan>\n\r", ch);
      return;
   }

   clan = get_clan(argument);
   if (!clan)
   {
      send_to_char("No such clan, guild or order.\n\r", ch);
      return;
   }

   ch_printf_color(ch, "\n\r&w%s     : &W%s\t\tBadge: %s\n\r&wFilename : &W%s\n\r&wMotto    : &W%s\n\r",
      clan->clan_type == CLAN_ORDER ? "Order" :
      (clan->clan_type == CLAN_GUILD ? "Guild" : "Clan"), clan->name, clan->badge ? clan->badge : "(not set)", clan->filename, clan->motto);
   ch_printf_color(ch, "&wDesc     : &W%s\n\r&wDeity    : &W%s\n\r&wLeader   : &W%s\n\r", clan->description, clan->deity, clan->leader);
   ch_printf_color(ch, "&wNumber1  : &W%s\n\r&wNumber2  : &W%s\n\r", clan->number1, clan->number2);
   ch_printf_color(ch, "&wPKills   : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\n\r",
      clan->pkills[0], clan->pkills[1], clan->pkills[2], clan->pkills[3], clan->pkills[4], clan->pkills[5], clan->pkills[6]);
   ch_printf_color(ch, "&wPDeaths  : &w1-9:&W%-3d &w10-14:&W%-3d &w15-19:&W%-3d &w20-29:&W%-3d &w30-39:&W%-3d &w40-49:&W%-3d &w50:&W%-3d\n\r",
      clan->pdeaths[0], clan->pdeaths[1], clan->pdeaths[2], clan->pdeaths[3], clan->pdeaths[4], clan->pdeaths[5], clan->pdeaths[6]);
   ch_printf_color(ch, "&wIllegalPK: &W%-6d\n\r", clan->illegal_pk);
   ch_printf_color(ch, "&wMKills   : &W%-6d   &wMDeaths: &W%-6d\n\r", clan->mkills, clan->mdeaths);
   ch_printf_color(ch, "&wScore    : &W%-6d   &wFavor  : &W%-6d   &wStrikes: &W%d\n\r", clan->score, clan->favour, clan->strikes);
   ch_printf_color(ch, "&wMembers  : &W%-6d   &wMemLimit  : &W%-6d   &wAlign  : &W%-6d", clan->members, clan->mem_limit, clan->alignment);
   send_to_char("\n\r", ch);
   ch_printf_color(ch, "&wBoard    : &W%-5d    &wRecall : &W%-5d    &wStorage: &W%-5d\n\r", clan->board, clan->recall, clan->storeroom);
   ch_printf_color(ch, "&wGuard1   : &W%-5d    &wGuard2 : &W%-5d\n\r", clan->guard1, clan->guard2);
   ch_printf_color(ch, "&wObj1( &W%d &w)  Obj2( &W%d &w)  Obj3( &W%d &w)  Obj4( &W%d &w)  Obj5( &W%d &w)\n\r",
      clan->clanobj1, clan->clanobj2, clan->clanobj3, clan->clanobj4, clan->clanobj5);
   return;
}

void do_showcouncil(CHAR_DATA * ch, char *argument)
{
   COUNCIL_DATA *council;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Usage: showcouncil <council>\n\r", ch);
      return;
   }

   council = get_council(argument);
   if (!council)
   {
      send_to_char("No such council.\n\r", ch);
      return;
   }

   ch_printf_color(ch, "\n\r&wCouncil :  &W%s\n\r&wFilename:  &W%s\n\r", council->name, council->filename);
   ch_printf_color(ch, "&wHead:      &W%s\n\r", council->head);
   ch_printf_color(ch, "&wHead2:     &W%s\n\r", council->head2);
   ch_printf_color(ch, "&wMembers:   &W%-d\n\r", council->members);
   ch_printf_color(ch, "&wBoard:     &W%-5d\n\r&wMeeting:   &W%-5d\n\r&wPowers:    &W%s\n\r", council->board, council->meeting, council->powers);
   ch_printf_color(ch, "&wDescription:\n\r&W%s\n\r", council->description);
   return;
}

void do_makeclan(CHAR_DATA * ch, char *argument)
{
   char filename[256];
   CLAN_DATA *clan;
   bool found;

   set_char_color(AT_IMMORT, ch);

   if (!argument || argument[0] == '\0')
   {
      send_to_char("Usage: makeclan <clan name>\n\r", ch);
      return;
   }

   found = FALSE;
   sprintf(filename, "%s%s", CLAN_DIR, strlower(argument));

   CREATE(clan, CLAN_DATA, 1);
   LINK(clan, first_clan, last_clan, next, prev);

   clan->name = STRALLOC(argument);
   clan->motto = STRALLOC("");
   clan->description = STRALLOC("");
   clan->deity = STRALLOC("");
   clan->leader = STRALLOC("");
   clan->number1 = STRALLOC("");
   clan->number2 = STRALLOC("");
   clan->badge = STRALLOC("");
}

void do_makecouncil(CHAR_DATA * ch, char *argument)
{
   char filename[256];
   COUNCIL_DATA *council;
   bool found;

   set_char_color(AT_IMMORT, ch);

   if (!argument || argument[0] == '\0')
   {
      send_to_char("Usage: makecouncil <council name>\n\r", ch);
      return;
   }

   found = FALSE;
   sprintf(filename, "%s%s", COUNCIL_DIR, strlower(argument));

   CREATE(council, COUNCIL_DATA, 1);
   LINK(council, first_council, last_council, next, prev);
   council->name = STRALLOC(argument);
   council->head = STRALLOC("");
   council->head2 = NULL;
   council->powers = STRALLOC("");
}

/*
 * Added multiple level pkill and pdeath support. --Shaddai
 */

void do_clans(CHAR_DATA * ch, char *argument)
{
   CLAN_DATA *clan;
   int count = 0;

   if (argument[0] == '\0')
   {
      set_char_color(AT_BLOOD, ch);
      send_to_char
         ("\n\rClan          Deity         Leader           Pkills:    Avatar      Other\n\r_________________________________________________________________________\n\r\n\r",
         ch);
      for (clan = first_clan; clan; clan = clan->next)
      {
         if (clan->clan_type == CLAN_ORDER || clan->clan_type == CLAN_GUILD)
            continue;
         set_char_color(AT_GREY, ch);
         ch_printf(ch, "%-13s %-13s %-13s", clan->name, clan->deity, clan->leader);
         set_char_color(AT_BLOOD, ch);
         ch_printf(ch, "                %5d      %5d\n\r", clan->pkills[6], (clan->pkills[2] + clan->pkills[3] + clan->pkills[4] + clan->pkills[5]));
         count++;
      }
      set_char_color(AT_BLOOD, ch);
      if (!count)
         send_to_char("There are no Clans currently formed.\n\r", ch);
      else
         send_to_char
            ("_________________________________________________________________________\n\r\n\rUse 'clans <clan>' for detailed information and a breakdown of victories.\n\r",
            ch);
      return;
   }

   clan = get_clan(argument);
   if (!clan || clan->clan_type == CLAN_GUILD || clan->clan_type == CLAN_ORDER)
   {
      set_char_color(AT_BLOOD, ch);
      send_to_char("No such clan.\n\r", ch);
      return;
   }
   set_char_color(AT_BLOOD, ch);
   ch_printf(ch, "\n\r%s, '%s'\n\r\n\r", clan->name, clan->motto);
   set_char_color(AT_GREY, ch);
   send_to_char_color("Victories:&w\n\r", ch);
   ch_printf_color(ch, "    &w15-19...  &r%-4d\n\r    &w20-29...  &r%-4d\n\r    &w30-39...  &r%-4d\n\r    &w40-49...  &r%-4d\n\r",
      clan->pkills[2], clan->pkills[3], clan->pkills[4], clan->pkills[5]);
   ch_printf_color(ch, "   &wAvatar...  &r%-4d\n\r", clan->pkills[6]);
   set_char_color(AT_GREY, ch);
   ch_printf(ch, "Clan Leader:  %s\n\rNumber One :  %s\n\rNumber Two :  %s\n\rClan Deity :  %s\n\r",
      clan->leader, clan->number1, clan->number2, clan->deity);
   if (!str_cmp(ch->name, clan->deity) || !str_cmp(ch->name, clan->leader) || !str_cmp(ch->name, clan->number1) || !str_cmp(ch->name, clan->number2))
      ch_printf(ch, "Members    :  %d\n\r", clan->members);
   set_char_color(AT_BLOOD, ch);
   ch_printf(ch, "\n\rDescription:  %s\n\r", clan->description);
   return;
}

void do_orders(CHAR_DATA * ch, char *argument)
{
   CLAN_DATA *order;
   int count = 0;

   if (argument[0] == '\0')
   {
      set_char_color(AT_DGREEN, ch);
      send_to_char
         ("\n\rOrder            Deity          Leader           Mkills      Mdeaths\n\r____________________________________________________________________\n\r\n\r",
         ch);
      set_char_color(AT_GREEN, ch);
      for (order = first_clan; order; order = order->next)
         if (order->clan_type == CLAN_ORDER)
         {
            ch_printf(ch, "%-16s %-14s %-14s   %-7d       %5d\n\r", order->name, order->deity, order->leader, order->mkills, order->mdeaths);
            count++;
         }
      set_char_color(AT_DGREEN, ch);
      if (!count)
         send_to_char("There are no Orders currently formed.\n\r", ch);
      else
         send_to_char
            ("____________________________________________________________________\n\r\n\rUse 'orders <order>' for more detailed information.\n\r",
            ch);
      return;
   }

   order = get_clan(argument);
   if (!order || order->clan_type != CLAN_ORDER)
   {
      set_char_color(AT_DGREEN, ch);
      send_to_char("No such Order.\n\r", ch);
      return;
   }

   set_char_color(AT_DGREEN, ch);
   ch_printf(ch, "\n\rOrder of %s\n\r'%s'\n\r\n\r", order->name, order->motto);
   set_char_color(AT_GREEN, ch);
   ch_printf(ch, "Deity      :  %s\n\rLeader     :  %s\n\rNumber One :  %s\n\rNumber Two :  %s\n\r",
      order->deity, order->leader, order->number1, order->number2);
   if (!str_cmp(ch->name, order->deity)
      || !str_cmp(ch->name, order->leader) || !str_cmp(ch->name, order->number1) || !str_cmp(ch->name, order->number2))
      ch_printf(ch, "Members    :  %d\n\r", order->members);
   set_char_color(AT_DGREEN, ch);
   ch_printf(ch, "\n\rDescription:\n\r%s\n\r", order->description);
   return;
}

void do_councils(CHAR_DATA * ch, char *argument)
{
   COUNCIL_DATA *council;

   set_char_color(AT_CYAN, ch);
   if (!first_council)
   {
      send_to_char("There are no councils currently formed.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char_color("\n\r&cTitle                    Head\n\r", ch);
      for (council = first_council; council; council = council->next)
      {
         if (council->head2 != NULL)
            ch_printf_color(ch, "&w%-24s %s and %s\n\r", council->name, council->head, council->head2);
         else
            ch_printf_color(ch, "&w%-24s %-14s\n\r", council->name, council->head);
      }
      send_to_char_color("&cUse 'councils <name of council>' for more detailed information.\n\r", ch);
      return;
   }
   council = get_council(argument);
   if (!council)
   {
      send_to_char_color("&cNo such council exists...\n\r", ch);
      return;
   }
   ch_printf_color(ch, "&c\n\r%s\n\r", council->name);
   if (council->head2 == NULL)
      ch_printf_color(ch, "&cHead:     &w%s\n\r&cMembers:  &w%d\n\r", council->head, council->members);
   else
      ch_printf_color(ch, "&cCo-Heads:     &w%s &cand &w%s\n\r&cMembers:  &w%d\n\r", council->head, council->head2, council->members);
   ch_printf_color(ch, "&cDescription:\n\r&w%s\n\r", council->description);
   return;
}

void do_guilds(CHAR_DATA * ch, char *argument)
{
   CLAN_DATA *guild;
   int count = 0;

   if (argument[0] == '\0')
   {
      set_char_color(AT_HUNGRY, ch);
      send_to_char
         ("\n\rGuild                  Leader             Mkills      Mdeaths\n\r_____________________________________________________________\n\r\n\r",
         ch);
      set_char_color(AT_YELLOW, ch);
      for (guild = first_clan; guild; guild = guild->next)
         if (guild->clan_type == CLAN_GUILD)
         {
            ++count;
            ch_printf(ch, "%-20s   %-14s     %-6d       %6d\n\r", guild->name, guild->leader, guild->mkills, guild->mdeaths);
         }
      set_char_color(AT_HUNGRY, ch);
      if (!count)
         send_to_char("There are no Guilds currently formed.\n\r", ch);
      else
         send_to_char("_____________________________________________________________\n\r\n\rUse 'guilds <guild>' for more detailed information.\n\r",
            ch);
      return;
   }

   guild = get_clan(argument);
   if (!guild || guild->clan_type != CLAN_GUILD)
   {
      set_char_color(AT_HUNGRY, ch);
      send_to_char("No such Guild.\n\r", ch);
      return;
   }
   set_char_color(AT_HUNGRY, ch);
   ch_printf(ch, "\n\r%s\n\r", guild->name);
   set_char_color(AT_YELLOW, ch);
   ch_printf(ch, "Leader:    %s\n\rNumber 1:  %s\n\rNumber 2:  %s\n\rMotto:     %s\n\r", guild->leader, guild->number1, guild->number2, guild->motto);
   if (!str_cmp(ch->name, guild->deity)
      || !str_cmp(ch->name, guild->leader) || !str_cmp(ch->name, guild->number1) || !str_cmp(ch->name, guild->number2))
      ch_printf(ch, "Members:   %d\n\r", guild->members);
   set_char_color(AT_HUNGRY, ch);
   ch_printf(ch, "Guild Description:\n\r%s\n\r", guild->description);
   return;
}

void do_victories(CHAR_DATA * ch, char *argument)
{
   char filename[256];

   if (IS_NPC(ch) || !ch->pcdata->clan)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (ch->pcdata->clan->clan_type != CLAN_ORDER && ch->pcdata->clan->clan_type != CLAN_GUILD)
   {
      sprintf(filename, "%s%s.record", CLAN_DIR, ch->pcdata->clan->name);
      set_pager_color(AT_PURPLE, ch);
      if (!str_cmp(ch->name, ch->pcdata->clan->leader) && !str_cmp(argument, "clean"))
      {
         FILE *fp = fopen(filename, "w");

         if (fp)
            fclose(fp);
         send_to_pager("\n\rVictories ledger has been cleared.\n\r", ch);
         return;
      }
      else
      {
         send_to_pager("\n\rLVL  Character       LVL  Character\n\r", ch);
         show_file(ch, filename);
         return;
      }
   }
   else
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
}


void do_shove(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   int exit_dir;
   EXIT_DATA *pexit;
   CHAR_DATA *victim;
   int curpos;
   bool nogo;
   ROOM_INDEX_DATA *to_room;
   int chance = 0;

   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg2);

   if (IS_NPC(ch))
   {
      send_to_char("Only PCs can shove.\n\r", ch);
      return;
   }

   if (arg[0] == '\0')
   {
      send_to_char("Shove whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("You shove yourself around, to no avail.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You can only shove PCs.\n\r", ch);
      return;
   }

   if (victim->position != POS_STANDING && victim->position != POS_MOUNTED)
   {
      act(AT_PLAIN, "$N isn't standing or mounted.", ch, NULL, victim, TO_CHAR);
      return;
   }
   curpos = victim->position;

   if (arg2[0] == '\0')
   {
      send_to_char("Shove them in which direction?\n\r", ch);
      return;
   }

   exit_dir = get_dir(arg2);
   if ((check_room_pk(ch) == 1) && get_timer(victim, TIMER_SHOVEDRAG) <= 0)
   {
      send_to_char("That character cannot be shoved right now.\n\r", ch);
      return;
   }
   victim->position = POS_SHOVE;
   nogo = FALSE;
   if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL)
      nogo = TRUE;
   else if (IS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_SET(pexit->exit_info, EX_NOPASSDOOR)))
      nogo = TRUE;
   if (nogo)
   {
      send_to_char("There's no exit in that direction.\n\r", ch);
      victim->position = curpos;
      return;
   }
   to_room = pexit->to_room;
   if (xIS_SET(to_room->room_flags, ROOM_DEATH))
   {
      send_to_char("You cannot shove someone into a death trap.\n\r", ch);
      victim->position = curpos;
      return;
   }

   if (ch->in_room->area != to_room->area && !in_hard_range(victim, to_room->area))
   {
      send_to_char("That character cannot enter that area.\n\r", ch);
      victim->position = curpos;
      return;
   }
   chance = number_range(35, 50);
   chance += ((get_curr_str(ch) - get_curr_str(victim)) * 10);

   //Make it much harder to push a mounted individual, almost impossible for magic users
   if (curpos == POS_MOUNTED)
      chance -= 30;

   /* Debugging purposes - show percentage for testing */

   /* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
      act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
    */

   if (chance < number_percent())
   {
      send_to_char("You failed.\n\r", ch);
      remove_timer(victim, TIMER_SHOVEDRAG); //cannot force after this
      victim->position = curpos;
      act(AT_ACTION, "$n tries to shove you but fails.", ch, NULL, victim, TO_VICT);
      return;
   }
   act(AT_ACTION, "You shove $M.", ch, NULL, victim, TO_CHAR);
   if (curpos == POS_MOUNTED)
   {
      act(AT_ACTION, "$n shoves you off of your mount and into the next room.", ch, NULL, victim, TO_VICT);
      if (victim->mount)
      {
         xREMOVE_BIT(victim->mount->act, ACT_MOUNTED);
         victim->mount = NULL;
         ch->position = POS_STANDING;
      }
   }
   else
   {
      act(AT_ACTION, "$n shoves you.", ch, NULL, victim, TO_VICT);
   }
   move_char(victim, get_exit(ch->in_room, exit_dir), 0);
   if (!char_died(victim))
      victim->position = POS_STANDING;
   if (curpos == POS_MOUNTED)
      damage(victim, victim, number_range(10, 25), TYPE_UNDEFINED, 0, -1);
   else
      damage(victim, victim, number_range(1, 5), TYPE_UNDEFINED, 0, -1);

   WAIT_STATE(ch, 12);
   /* Remove protection from shove/drag if char shoves -- Blodkai */
}

void do_consent(CHAR_DATA * ch, char *argument)
{
   DESCRIPTOR_DATA *d;
   
   if (check_npc(ch))
      return;
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  consent <player>\n\r", ch);
      send_to_char("Syntax:  consent none\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "none"))
   {
      ch->pcdata->consent = 0;
      return;
   }
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->character && d->character->name && d->character->short_descr && d->character->pcdata)
      {
         if (!str_prefix(argument, PERS_MAP(d->character, ch)))
            break;
      }
   }
   if (d)
   {
      ch->pcdata->consent = d->character->pcdata->pid;
      ch_printf(ch, "You offer %s consent to drag your corpse.\n\r", PERS_MAP(d->character, ch));
      ch_printf(d->character, "%s gives you consent to drag his/her corpse.\n\r", PERS_MAP(ch, d->character));
      return;
   }
   else
   {
      ch_printf(ch, "No %s could be found to offer consent.\n\r", argument);
      return;
   }
}

void write_corpses args((CHAR_DATA * ch, char *name, OBJ_DATA * objrem));
void update_player_container args((CHAR_DATA * ch, OBJ_DATA *iobj));

void do_drag(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   int exit_dir;
   CHAR_DATA *victim;
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *to_room;
   int curpos;
   bool nogo;
   int chance = 0;

   if (IS_NPC(ch))
      /* || !IS_SET( ch->pcdata->flags, PCFLAG_DEADLY ) )  */
   {
      send_to_char("Only characters can drag.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  drag <victim> <dir>\n\r", ch);
      send_to_char("Syntax:  drag corpse <corpse> <dir>\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg2);
   
   if (!str_cmp(arg, "corpse"))
   {
      OBJ_DATA *obj;
      char *pd;
      char name[MIL];
      int consent = 0;
      DESCRIPTOR_DATA *d;
      ROOM_INDEX_DATA *in_room;
      ROOM_INDEX_DATA *to_room;
      int x, nx;
      int y, ny;
      int map, nmap;
      
      if ((obj = get_obj_list_rev(ch, arg2, ch->in_room->last_content)) == NULL)
      {
         send_to_char("That corpse is not here.\n\r", ch);
         return;
      }
      if (obj->item_type != ITEM_CORPSE_PC)
      {
         send_to_char("You can only drag a PC corpse.\n\r", ch);
         return;
      }
      if (ch->position != POS_STANDING)
      {
         send_to_char("You can only do this if you are standing.\n\r", ch);
         return;
      }
      pd = obj->short_descr;
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      if (!str_cmp(name, ch->name))
         consent = 1;
      for (d = first_descriptor; d; d = d->next)
      {
         if (d->character && d->character->name && !str_cmp(d->character->name, name))
         {
            if (d->character->master && is_same_group(ch, d->character))
               consent = 1;
            if (d->character->pcdata && d->character->pcdata->consent == ch->pcdata->pid)
               consent = 1;
         }
      }
      if (consent == 0)
      {
         send_to_char("You do not have consent to drag that corpse.\n\r", ch);
         return;
      }
      in_room = ch->in_room;
      x = ch->coord->x;
      y = ch->coord->y;
      map = ch->map;
      exit_dir = get_dir(argument);
      if (exit_dir == 0)
         do_north(ch, "");
      if (exit_dir == 1)
         do_east(ch, "");
      if (exit_dir == 2)
         do_south(ch, "");
      if (exit_dir == 3)
         do_west(ch, "");
      if (exit_dir == 4)
         do_up(ch, "");
      if (exit_dir == 5)
         do_down(ch, "");
      if (exit_dir == 6)
         do_northeast(ch, "");
      if (exit_dir == 7)
         do_northwest(ch, "");
      if (exit_dir == 8)
         do_southeast(ch, "");
      if (exit_dir == 9)
         do_southwest(ch, "");
      to_room = ch->in_room;
      nx = ch->coord->x;
      ny = ch->coord->y;
      nmap = ch->map;
      if (in_room == to_room && x == nx && y == ny && map == nmap)
      {
         send_to_char("You cannot go that direction so you cannot drag a corpse that direction.\n\r", ch);
         return;
      }
      char_from_room(ch);
      char_to_room(ch, in_room);
      ch->coord->x = x;
      ch->coord->y = y;
      ch->map = map;
      act(AT_WHITE, "$n departs with a corpse in haul.", ch, NULL, NULL, TO_ROOM);
      char_from_room(ch);
      char_to_room(ch, to_room);
      ch->coord->x = nx;
      ch->coord->y = ny;
      ch->map = nmap;
      act(AT_WHITE, "$n hauls in a corpse.", ch, NULL, NULL, TO_ROOM);
      act(AT_WHITE, "You drag in the corpse", ch, NULL, NULL, TO_CHAR);
      obj_from_room(obj);
      obj_to_room(obj, to_room, ch);
      update_player_container(ch, obj);
      write_corpses(NULL, obj->short_descr + 14, NULL);
      return;
   }  

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("You take yourself by the scruff of your neck, but go nowhere.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
      /* || !IS_SET( victim->pcdata->flags, PCFLAG_DEADLY ) ) */
   {
      send_to_char("You can only drag characters.\n\r", ch);
      return;
   }

   if (victim->fighting)
   {
      send_to_char("You try, but can't get close enough.\n\r", ch);
      return;
   }
   if (victim->position <= 4 || victim->position == POS_RESTING || victim->position == POS_SITTING)
   {
      ;
   }
   else
   {
      send_to_char("They don't seem to need your assistance.\n\r", ch);
      return;
   }
   curpos = victim->position;
   if (arg2[0] == '\0')
   {
      send_to_char("Drag them in which direction?\n\r", ch);
      return;
   }

   exit_dir = get_dir(arg2);
   
   if ((check_room_pk(ch) == 1) && get_timer(victim, TIMER_SHOVEDRAG) <= 0)
   {
      send_to_char("That character cannot be dragged right now.\n\r", ch);
      return;
   }

   nogo = FALSE;
   if ((pexit = get_exit(ch->in_room, exit_dir)) == NULL)
      nogo = TRUE;
   else if (IS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_SET(pexit->exit_info, EX_NOPASSDOOR)))
      nogo = TRUE;
   if (nogo)
   {
      send_to_char("There's no exit in that direction.\n\r", ch);
      return;
   }

   to_room = pexit->to_room;
   if (xIS_SET(to_room->room_flags, ROOM_DEATH))
   {
      send_to_char("You cannot drag someone into a death trap.\n\r", ch);
      return;
   }

   if (ch->in_room->area != to_room->area && !in_hard_range(victim, to_room->area))
   {
      send_to_char("That character cannot enter that area.\n\r", ch);
      return;
   }

   chance = number_range(30, 45);
   chance += ((get_curr_str(ch) - get_curr_str(victim)) * 10);

   if (curpos == POS_DEAD || curpos == POS_MORTAL || curpos == POS_INCAP)
      chance += 40;
   if (curpos == POS_STUNNED)
      chance += 35;
   if (curpos == POS_SLEEPING)
      chance += 25;
   if (curpos == POS_RESTING)
      chance += 20;
   if (curpos == POS_SITTING)
      chance += 10;

   /* Debugging purposes - show percentage for testing */

   /* sprintf(buf, "Shove percentage of %s = %d", ch->name, chance);
      act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
    */
   if (chance < number_percent())
   {
      send_to_char("You failed.\n\r", ch);
      remove_timer(victim, TIMER_SHOVEDRAG); //cannot force after this
      victim->position = POS_STANDING;
      return;
   }
   if (victim->position < POS_STANDING)
   {
      sh_int temp;

      temp = victim->position;
      victim->position = POS_DRAG;
      act(AT_ACTION, "You drag $M into the next room.", ch, NULL, victim, TO_CHAR);
      act(AT_ACTION, "$n grabs your hair and drags you.", ch, NULL, victim, TO_VICT);
      move_char(victim, get_exit(ch->in_room, exit_dir), 0);
      if (!char_died(victim))
         victim->position = temp;
      /* Move ch to the room too.. they are doing dragging - Scryn */
      move_char(ch, get_exit(ch->in_room, exit_dir), 0);
      WAIT_STATE(ch, 12);
      return;
   }
   send_to_char("You cannot do that to someone who is standing.\n\r", ch);
   return;
}
