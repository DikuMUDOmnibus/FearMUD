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
 * Rafermand Coded by: Xerves                                               *
 * --------------------------------------------------------------------------
 *			                   Quest System for 2.1                         *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <time.h>
#include <dirent.h>
#include "mud.h"

int top_area;
int top_quest_vnum;
int eqload;
extern int top_affect;
int ochange;
int raferquest_local_ovnum;
int global_drop_equip_message;

void adjust_static_quest_mob(CHAR_DATA *ch)
{
   MOB_INDEX_DATA *pMobIndex = ch->pIndexData;
   int hit;
   
   STRFREE(pMobIndex->player_name);
   STRFREE(pMobIndex->short_descr);
   STRFREE(pMobIndex->long_descr);
   STRFREE(pMobIndex->description);
   pMobIndex->player_name = STRALLOC(ch->name);
   pMobIndex->short_descr = STRALLOC(ch->short_descr);
   pMobIndex->long_descr = STRALLOC(ch->long_descr);
   pMobIndex->description = STRALLOC(ch->description);
   pMobIndex->miflags = ch->miflags;
   pMobIndex->act = ch->act;
   pMobIndex->affected_by = ch->affected_by;
   pMobIndex->spec_fun = ch->spec_fun;
   pMobIndex->tohitbash = ch->tohitbash;
   pMobIndex->tohitslash = ch->tohitslash;
   pMobIndex->tohitstab = ch->tohitstab;
   pMobIndex->ac = ch->armor;
   pMobIndex->max_move = ch->max_move;
   pMobIndex->gold = ch->gold;
   pMobIndex->position = ch->position;
   pMobIndex->defposition = ch->defposition;
   pMobIndex->perm_str = ch->perm_str;
   pMobIndex->perm_dex = ch->perm_dex;
   pMobIndex->perm_int = ch->perm_int;
   pMobIndex->perm_wis = ch->perm_wis;
   pMobIndex->perm_cha = ch->perm_cha;
   pMobIndex->perm_con = ch->perm_con;
   pMobIndex->perm_lck = ch->perm_lck;
   pMobIndex->perm_agi = ch->perm_agi;
   pMobIndex->race = ch->race;
   pMobIndex->xflags = ch->xflags;
   pMobIndex->resistant = ch->resistant;
   pMobIndex->immune = ch->immune;
   pMobIndex->susceptible = ch->susceptible;
   pMobIndex->attacks = ch->attacks;
   pMobIndex->defenses = ch->defenses;   
   hit = ch->hit * 2 / 10;
   pMobIndex->hitnodice = UMAX(1, hit / 10);
   pMobIndex->hitsizedice = UMAX(1, ((hit*2) - UMAX(1, (hit/10))) / UMAX(1, (hit/10)));
   pMobIndex->hitplus = ch->hit * 8 / 10;
   return;
}
//Need to copy the obj values over the the pIndexData to make sure they save...
void adjust_static_area_obj(OBJ_DATA *obj)
{ 
   OBJ_INDEX_DATA *iobj;
   AFFECT_DATA *paf, *cpaf;
   
   iobj = obj->pIndexData;
   STRFREE(iobj->name);
   STRFREE(iobj->short_descr);
   STRFREE(iobj->description);
   STRFREE(iobj->action_desc);
   iobj->name = STRALLOC(obj->name);
   iobj->short_descr = STRALLOC(obj->short_descr);
   iobj->description = STRALLOC(obj->description);
   iobj->action_desc = STRALLOC(obj->action_desc);
   iobj->item_type = obj->item_type;
   iobj->extra_flags = obj->extra_flags;
   iobj->wear_flags = obj->wear_flags;
   iobj->value[0] = obj->value[0];
   iobj->value[1] = obj->value[1];
   iobj->value[2] = obj->value[2];
   iobj->value[3] = obj->value[3];
   iobj->value[4] = obj->value[4];
   iobj->value[5] = obj->value[5];
   iobj->value[6] = obj->value[6];
   iobj->value[7] = obj->value[7];
   iobj->value[8] = obj->value[8];
   iobj->value[9] = obj->value[9];
   iobj->value[10] = obj->value[10];
   iobj->value[11] = obj->value[11];
   iobj->value[12] = obj->value[12];
   iobj->value[13] = obj->value[13];
   iobj->imbueslots = obj->imbueslots;
   iobj->weight = obj->weight;
   iobj->cost = obj->cost;
   iobj->first_affect = NULL;
   iobj->last_affect = NULL;
   for (cpaf = obj->first_affect; cpaf; cpaf = cpaf->next)
   {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = cpaf->type;
      paf->duration = cpaf->duration;
      paf->location = cpaf->location;
      paf->modifier = cpaf->modifier;
      paf->bitvector = cpaf->bitvector;
      LINK(paf, iobj->first_affect, iobj->last_affect, next, prev);
      top_affect++;
   }
   return;
}

void copy_static_area_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
   OBJ_INDEX_DATA *nobj;
   OBJ_DATA *iobj;
   AFFECT_DATA *paf, *cpaf;
   
   nobj = make_object(raferquest_local_ovnum++, 0, "Quest Obj", 0);
   xREMOVE_BIT(nobj->extra_flags, ITEM_PROTOTYPE);
   iobj = create_object(nobj, 1);
   STRFREE(iobj->name);
   STRFREE(iobj->short_descr);
   STRFREE(iobj->description);
   STRFREE(iobj->action_desc);
   iobj->name = STRALLOC(obj->name);
   iobj->short_descr = STRALLOC(obj->short_descr);
   iobj->description = STRALLOC(obj->description);
   iobj->action_desc = STRALLOC(obj->action_desc);
   iobj->item_type = obj->item_type;
   iobj->extra_flags = obj->extra_flags;
   iobj->wear_flags = obj->wear_flags;
   iobj->value[0] = obj->value[0];
   iobj->value[1] = obj->value[1];
   iobj->value[2] = obj->value[2];
   iobj->value[3] = obj->value[3];
   iobj->value[4] = obj->value[4];
   iobj->value[5] = obj->value[5];
   iobj->value[6] = obj->value[6];
   iobj->value[7] = obj->value[7];
   iobj->value[8] = obj->value[8];
   iobj->value[9] = obj->value[9];
   iobj->value[10] = obj->value[10];
   iobj->value[11] = obj->value[11];
   iobj->value[12] = obj->value[12];
   iobj->value[13] = obj->value[13];
   iobj->weight = obj->weight;
   iobj->cost = obj->cost;
   iobj->first_affect = NULL;
   iobj->last_affect = NULL;
   for (cpaf = obj->first_affect; cpaf; cpaf = cpaf->next)
   {
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = cpaf->type;
      paf->duration = cpaf->duration;
      paf->location = cpaf->location;
      paf->modifier = cpaf->modifier;
      paf->bitvector = cpaf->bitvector;
      LINK(paf, iobj->first_affect, iobj->last_affect, next, prev);
      top_affect++;
   }
   adjust_static_area_obj(iobj);
   unequip_char(ch, obj);
   separate_obj(obj);
   obj_from_char(obj);
   extract_obj(obj);
   obj_to_char(iobj, ch);
   wear_obj(ch, iobj, TRUE, -1);
}
   

void fwrite_qmob_data()
{
   FILE *fp;
   char filename[MIL];
   QMOB_DATA *qmob;
   
   sprintf(filename, "%s", QMOB_FILE);
   if ((fp = fopen(filename, "w")) == NULL)
   {   
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (qmob = first_qmob; qmob; qmob = qmob->next)
   {
      fprintf(fp, "#MOB\n\r");
      fprintf(fp, "Boss         %d\n", qmob->boss);
      fprintf(fp, "Hidiff       %d\n", qmob->hidiff);
      fprintf(fp, "Lowdiff      %d\n", qmob->lowdiff);
      fprintf(fp, "Name         %s~\n", qmob->name);
      fprintf(fp, "Race         %d\n", qmob->race);
      fprintf(fp, "Sex          %d\n", qmob->sex);
      fprintf(fp, "Flags        %s\n", print_bitvector(&qmob->flags));
      fprintf(fp, "End\n\n");
   }
   fprintf(fp, "#END\n");
   fclose(fp);
   return;
}

int is_in_range(int lowx, int hix, int lowy, int hiy)
{
   int x;
   
   for (x = lowy; x <= hiy; x++)
   {
      if (x >= lowx && x <= hix)
         return 1;
   }
   return 0;
}

char *print_qmob_flags(QMOB_DATA *qmob)
{
   static char buf[MSL];
   char sbuf[10];
   int x;
   
   strcpy(buf, "");
   
   for (x = 0; x < MAX_BITS; x++)
   {
      if (xIS_SET(qmob->flags, x))
      {
         sprintf(sbuf, "%d ", x+1);
         strcat(buf, sbuf);
      }
   }
   return buf;
}

char *print_qobj_flags(QOBJ_DATA *qobj)
{
   static char buf[MSL];
   char sbuf[10];
   int x;
   
   strcpy(buf, "");
   
   for (x = 0; x < MAX_BITS; x++)
   {
      if (xIS_SET(qobj->flags, x))
      {
         sprintf(sbuf, "%d ", x+1);
         strcat(buf, sbuf);
      }
   }
   for (x = 0; x < MAX_BITS; x++)
   {
      if (xIS_SET(qobj->flags2, x))
      {
         sprintf(sbuf, "%d ", x+MAX_BITS+1);
         strcat(buf, sbuf);
      }
   }
   return buf;
}
   
void do_qmob(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   char arg6[MIL];
   int cnt = 0;
   int race;
   QMOB_DATA *qmob;
   
   if (argument[0] == '\0')
   {
       send_to_char("Syntax:  qmob list [low diff] [hi diff]\n\r", ch);
       send_to_char("Syntax:  qmob add <boss #> <lowdiff> <hidiff> <race> <sex> <name>\n\r", ch);
       send_to_char("Syntax:  qmob delete <num>\n\r", ch);
       send_to_char("Syntax:  qmob edit <num> [boss/lodiff/hidiff/race/sex/name/flags/aflags] <value>\n\r", ch);
       ch_printf(ch, "Boss (0 - Reg, 1 - Captain, 2 - Boss) Difficulty (1-%d) Sex(0-2)\n\r", MAX_QDIFF);
       return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   
   if (!str_cmp(arg1, "edit"))
   {
      if (atoi(arg2) < 1)
      {
         send_to_char("Need to choose the number listed in qmob list\n\r", ch);
         return;
      }
      for (qmob = first_qmob; qmob; qmob = qmob->next)
      {
         if (++cnt == atoi(arg2))
         {
            if (!str_cmp(arg3, "flags"))
            {
               xCLEAR_BITS(qmob->flags);
               for (;;)
               {
                  argument = one_argument(argument, arg4);  
                  if (arg4[0] == '\0')
                     break;
                  else
                     xSET_BIT(qmob->flags, atoi(arg4)-1);
               }
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
            if (!str_cmp(arg3, "aflags"))
            {
               for (;;)
               {
                  argument = one_argument(argument, arg4);  
                  if (arg4[0] == '\0')
                     break;
                  else
                     xTOGGLE_BIT(qmob->flags, atoi(arg4)-1);
               }
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
            if (!str_cmp(arg3, "boss"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 2)
               {
                  send_to_char("Boss value is 0 - 2\n\r", ch);
                  return;
               }      
               qmob->boss = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
            if (!str_cmp(arg3, "lodiff"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_QDIFF)
               {
                  ch_printf(ch, "Difficulty value is 1 - %d\n\r", MAX_QDIFF);
                  return;
               }    
               if (atoi(argument) > qmob->hidiff)
               {
                  send_to_char("Lowdiff has to be less than or equal to hidiff\n\r", ch);
                  return;
               }
               qmob->lowdiff = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
            if (!str_cmp(arg3, "hidiff"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_QDIFF)
               {
                  ch_printf(ch, "Difficulty value is 1 - %d\n\r", MAX_QDIFF);
                  return;
               }    
               if (atoi(argument) < qmob->lowdiff)
               {
                  send_to_char("Hidiff has to be greater than or equal to lowdiff\n\r", ch);
                  return;
               }
               qmob->hidiff = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
            if (!str_cmp(arg3, "race"))
            {
               race = get_npc_race(argument);
               if (race < 0)
               {
                  send_to_char("You must supply race via the name of the race.\n\r", ch);
                  return;
               }   
               qmob->race = race;
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
            if (!str_cmp(arg3, "name"))
            {
               STRFREE(qmob->name);
               qmob->name = STRALLOC(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
            if (!str_cmp(arg3, "sex"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 2)
               {
                  send_to_char("Range is 0 to 2.\n\r", ch);
                  return;
               }
               qmob->sex = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qmob_data();
               return;
            }
         }
      }
   }
   argument = one_argument(argument, arg4);
   argument = one_argument(argument, arg5);
   argument = one_argument(argument, arg6);
   if (!str_cmp(arg1, "delete"))
   {
      if (atoi(arg2) < 1)
      {
         send_to_char("Need to choose the number listed in qmob list\n\r", ch);
         return;
      }
      for (qmob = first_qmob; qmob; qmob = qmob->next)
      {
         if (++cnt == atoi(arg2))
         {
            STRFREE(qmob->name);
            UNLINK(qmob, first_qmob, last_qmob, next, prev);
            DISPOSE(qmob);
            fwrite_qmob_data();
            send_to_char("Done.\n\r", ch);
            return;
         }
      }
      send_to_char("That number does not exist, check qmob list again.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "add"))
   {
      if (atoi(arg2) < 0 || atoi(arg2) > 2)
      {
         send_to_char("Boss value is 0 - 2\n\r", ch);
         return;
      }
      if (atoi(arg3) < 1 || atoi(arg3) > MAX_QDIFF)
      {
         ch_printf(ch, "Difficulty value is 1 - %d\n\r", MAX_QDIFF);
         return;
      }
      if (atoi(arg4) < 1 || atoi(arg4) > MAX_QDIFF)
      {
         ch_printf(ch, "Difficulty value is 1 - %dF\n\r", MAX_QDIFF);
         return;
      }
      if (atoi(arg3) > atoi(arg4))
      {
         send_to_char("Lowdiff has to be less than or equal to hidiff\n\r", ch);
         return;
      }
      race = get_npc_race(arg5);
      if (race < 0)
      {
         send_to_char("You must supply race via the name of the race.\n\r", ch);
         return;
      }
      if (atoi(arg6) < 0 || atoi(arg6) > 2)
      {
         send_to_char("Sex ranges from 0 to 2.\n\r", ch);
         return;
      }
      CREATE(qmob, QMOB_DATA, 1);
      qmob->boss = atoi(arg2);
      qmob->lowdiff = atoi(arg3);
      qmob->hidiff = atoi(arg4);
      qmob->race = race;
      qmob->sex = atoi(arg6);
      qmob->name = STRALLOC(argument);
      LINK(qmob, first_qmob, last_qmob, next, prev);
      fwrite_qmob_data();
      send_to_char("Done.\n\r", ch);
      return;
   }     
   
   if (!str_cmp(arg1, "list"))
   {
      if (atoi(arg2) > MAX_QDIFF || atoi(arg3) > MAX_QDIFF)
      {
         ch_printf(ch, "Range is 0 to %d\n\r", MAX_QDIFF);
         return;
      }
      if (atoi(arg2) < 1 || atoi(arg3) < 1)
      {
         send_to_char("&c&wNum   Name                             Boss  Lo  Hi  Sex  Race          Flags\n\r------------------------------------------------------------------------------------------\n\r", ch);
         for (qmob = first_qmob; qmob; qmob = qmob->next)
         {
            ch_printf(ch, "[%-3d] %-30s > %-4s  %-2d  %-2d  %-s   %-12s  %s\n\r", ++cnt, qmob->name, qmob->boss > 0 ? qmob->boss > 1 ? "&w&RBoss&c&w" : "&w&CCapt&c&w" : "&w&GRegu&c&w", qmob->lowdiff, qmob->hidiff, qmob->sex > 0 ? qmob->sex > 1 ? "&w&PFe&c&w" : "&w&CMa&c&w" : "&w&WNe&c&w", print_npc_race(qmob->race), print_qmob_flags(qmob));
         }
      }
      else
      {
         send_to_char("&c&wNum   Name                             Boss  Hi  Lo  Sex  Race          Flags\n\r------------------------------------------------------------------------------------------\n\r", ch);
         for (qmob = first_qmob; qmob; qmob = qmob->next)
         {
            cnt++;
            if (is_in_range(qmob->lowdiff, qmob->hidiff, atoi(arg2), atoi(arg3)))
            {
               ch_printf(ch, "[%-3d] %-30s > %-4s  %-2d  %-2d  %-s   %-12s  %s\n\r", cnt, qmob->name, qmob->boss > 0 ? qmob->boss > 1 ? "&w&RBoss&c&w" : "&w&CCapt&c&w" : "&w&GRegu&c&w", qmob->lowdiff, qmob->hidiff, qmob->sex > 0 ? qmob->sex > 1 ? "&w&PFe&c&w" : "&w&CMa&c&w" : "&w&WNe&c&w", print_npc_race(qmob->race), print_qmob_flags(qmob));
            }
         }
      }      
      return;
   }
   do_qmob(ch, "");
   return;
}

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
				
void fread_qobj_data(FILE *fp)
{
   bool fMatch;  
   char buf[MSL];   
   char *word;
   QOBJ_DATA *qobj;
   
   CREATE(qobj, QOBJ_DATA, 1);
   
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
            KEY("Boss", qobj->boss, fread_number(fp));
            break;
            
         case 'F':
            KEY("Flags", qobj->flags, fread_bitvector(fp));            
            KEY("Flags2", qobj->flags2, fread_bitvector(fp));       
            break;
            
         case 'H':
            KEY("Hidiff", qobj->hidiff, fread_number(fp));
            break;
            
         case 'L':
            KEY("Lowdiff", qobj->lowdiff, fread_number(fp));
            break;
            
         case 'N':
            KEY("Name", qobj->name, fread_string(fp));
            break;
            
         case 'R':
            KEY("Race", qobj->race, fread_number(fp));
            break;
            
         case 'Q':
            KEY("QPS", qobj->qps, fread_number(fp));
            break;
            
         case 'T':
            KEY("Type", qobj->type, fread_number(fp));
            break;
            
         case 'G':
            KEY("Gold", qobj->gold, fread_number(fp));
            break;
            
         case 'S':
            if (!str_cmp(word, "Scroll"))
            {
               qobj->value[1] = skill_lookup(fread_word(fp));
               qobj->value[2] = skill_lookup(fread_word(fp));
               qobj->value[3] = skill_lookup(fread_word(fp));
               fMatch = TRUE;
            }
            break;
            
         case 'W':
            if (!str_cmp(word, "Weapon"))
            {
               qobj->value[4] = skill_lookup(fread_word(fp));
               fMatch = TRUE;
            }
            KEY("Weight", qobj->weight, fread_float(fp));
            break;
            
         case 'V':
            if (!str_cmp(word, "Value"))
            {
               char *ln;
               int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14;
               ln = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = 0;
               sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                  &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14);
               qobj->value[0] = x1;
               qobj->value[1] = x2;
               qobj->value[2] = x3;
               qobj->value[3] = x4;
               qobj->value[4] = x5;
               qobj->value[5] = x6;
               qobj->value[6] = x7;
               qobj->value[7] = x8;
               qobj->value[8] = x9;
               qobj->value[9] = x10;
               qobj->value[10] = x11;
               qobj->value[11] = x12;
               qobj->value[12] = x13;
               qobj->value[13] = x14;
               fMatch = TRUE;
            }
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(qobj, first_qobj, last_qobj, next, prev);
               return;       
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "fread_qmob_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}        
                   
void load_qobj_data()
{
   FILE *fp;
   char filename[MIL];
   
   sprintf(filename, "%s", QOBJ_FILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("load_qobj_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "OBJ"))
         {
            fread_qobj_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
         {
            break;
         }
         else
         {
            bug("load_qobj_data: bad section.", 0);
            continue;
         }
      }
   }
   else
   {
      bug("Cannot open the qobj data file", 0);
      return;
   }
   fclose(fp);
}   				

void fwrite_qobj_data()
{
   FILE *fp;
   char filename[MIL];
   QOBJ_DATA *qobj;
   sprintf(filename, "%s", QOBJ_FILE);
   if ((fp = fopen(filename, "w")) == NULL)
   {   
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (qobj = first_qobj; qobj; qobj = qobj->next)
   {
      fprintf(fp, "#OBJ\n\r");
      fprintf(fp, "Type         %d\n", qobj->type);
      fprintf(fp, "Gold         %d\n", qobj->gold);
      fprintf(fp, "Weight       %f\n", qobj->weight);
      fprintf(fp, "Boss         %d\n", qobj->boss);
      fprintf(fp, "Hidiff       %d\n", qobj->hidiff);
      fprintf(fp, "Lowdiff      %d\n", qobj->lowdiff);
      fprintf(fp, "Name         %s~\n", qobj->name);
      fprintf(fp, "QPS          %d\n", qobj->qps);
      fprintf(fp, "Race         %d\n", qobj->race);
      fprintf(fp, "Value        %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", qobj->value[0], qobj->value[1], qobj->value[2],
         qobj->value[3], qobj->value[4], qobj->value[5], qobj->value[6], qobj->value[7], qobj->value[8], qobj->value[9], 
         qobj->value[10], qobj->value[11], qobj->value[12], qobj->value[13]);
      if (qobj->type == 3 || qobj->type == 4) //Potion or Scroll
      {
         fprintf(fp, "Scroll       '%s' '%s' '%s'\n", IS_VALID_SN(qobj->value[1]) ? skill_table[qobj->value[1]]->name : "NONE",
                    IS_VALID_SN(qobj->value[2]) ? skill_table[qobj->value[2]]->name : "NONE",
                    IS_VALID_SN(qobj->value[3]) ? skill_table[qobj->value[3]]->name : "NONE");    
      }
      if (qobj->type == 1) //Weapon
      {
         fprintf(fp, "Weapon       '%s'\n", IS_VALID_SN(qobj->value[4]) ? skill_table[qobj->value[4]]->name : "NONE");
      }
      fprintf(fp, "Flags        %s\n", print_bitvector(&qobj->flags));
      fprintf(fp, "Flags2       %s\n", print_bitvector(&qobj->flags2));
      fprintf(fp, "End\n\n");
   }
   fprintf(fp, "#END\n");
   fclose(fp);
   return;
}

void fread_quest_data(FILE *fp)
{
   bool fMatch;  
   char buf[MSL];   
   char *word;
   QUEST_DATA *quest;
   char *ln;
   int x1, x2, x3, x4, x5, x6;
   AREA_DATA *area;
   
   CREATE(quest, QUEST_DATA, 1);
   
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
            if (!str_cmp(word, "Area"))
            {
               int vnum = fread_number(fp);
               
               for (area = first_area; area; area = area->next)
               {
                  if (area->low_r_vnum == vnum)
                     quest->questarea = area;
               }
               fMatch = TRUE;
               if (!quest->questarea)
                  bug("fread_quest_data:  No area for vnum %d", vnum);
            }      
            break;
            
         case 'D':
            KEY("Difficulty", quest->difficulty, fread_number(fp));
            break;
            
         case 'K':
            KEY("Killed", quest->killed, fread_number(fp));
            break;
            
         case 'M':
            KEY("Map", quest->map, fread_number(fp));
            KEY("Mission", quest->mission, fread_number(fp));
            break;
            
         case 'P':
            if (!str_cmp(word, "Player"))
            {
               ln = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = 0;
               sscanf(ln, "%d %d %d %d %d %d", 
                  &x1, &x2, &x3, &x4, &x5, &x6);
               quest->player[0] = x1;
               quest->player[1] = x2;
               quest->player[2] = x3;
               quest->player[3] = x4;
               quest->player[4] = x5;
               quest->player[5] = x6;
               fMatch = TRUE;
            }
            break;
            
         case 'Q':
            if (!str_cmp(word, "Qp"))
            {
               ln = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = 0;
               sscanf(ln, "%d %d %d %d %d %d", 
                  &x1, &x2, &x3, &x4, &x5, &x6);
               quest->qp[0] = x1;
               quest->qp[1] = x2;
               quest->qp[2] = x3;
               quest->qp[3] = x4;
               quest->qp[4] = x5;
               quest->qp[5] = x6;
               fMatch = TRUE;
            }
            break;
            
         case 'T':
            KEY("Timeleft", quest->timeleft, fread_number(fp));
            KEY("Traveltime", quest->traveltime, fread_number(fp));
            KEY("Tillnew", quest->tillnew, fread_number(fp));
            KEY("Tokill", quest->tokill, fread_number(fp));
            break;
            
         case 'X':
            KEY("X", quest->x, fread_number(fp));
            break;
            
         case 'Y':
            KEY("Y", quest->y, fread_number(fp));
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(quest, first_quest, last_quest, next, prev);
               return;       
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "fread_quest_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}
extern bool fBootDb;
extern char strArea[MIL];
void fix_exits args((void));

void load_quest_contents()
{
   FILE *fp;
   char filename[MIL];
   CHAR_DATA *mob;
   OBJ_DATA *obj;
   OBJ_DATA *nobj;
   
   if ((fp = fopen(QUEST_LAREA, "r")) != NULL)
   {
      for (;;)
      {
         strcpy(strArea, fread_word(fp));
         if (strArea[0] == '$')
            break;
         fBootDb = TRUE;
         load_area_file(last_area, strArea);
         fBootDb = FALSE;
         remove(strArea);
      }
      fclose(fp);
      fix_exits();
      remove(QUEST_LAREA);
   }
   sprintf(filename, "%s", QUEST_OFILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("load_quest_contents: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!strcmp(word, "GROUND"))
            fread_obj(NULL, fp, OS_GROUND);
         else if (!strcmp(word, "OBJECT"))
            fread_obj(NULL, fp, OS_CARRY);
         else if (!strcmp(word, "END"))  
         {
            break;
         }
         else
         {
            bug("Load_quest_content: objs: bad section.", 0);
            break;
         }
      }
      fclose(fp);
      remove(filename);
   }
   sprintf(filename, "%s", QUEST_MFILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("load_quest_contents: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!strcmp(word, "MOBILE"))
         {
            mob = fread_mobile(fp);
            for (obj = mob->first_carrying; obj; obj = nobj)
            {
               nobj = obj->next_content;
               wear_obj(mob, obj, TRUE, -1);
            }
         }

         else if (!strcmp(word, "END"))
         {
            break;
         }
         else
         {
            bug("Load_quest_content: mobs: bad section.", 0);
            break;
         }
      }
      fclose(fp);
      remove(filename);
   }
}
void load_quest_data()
{
   FILE *fp;
   char filename[MIL];
   int first = 1;
   
   sprintf(filename, "%s", QUEST_FILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
      for (;;)
      {
         char letter;
         char *word;
         
         if (first == 1)
         {
            word = fread_word(fp);
            if (!str_cmp(word, "QuestVnum"))
            {
               top_quest_vnum = fread_number(fp);
               first = 0;
               continue;
            }      
            else
            {
               bug("load_quest_data:  First line didn't contain the top vnum.");
               fclose(fp);
               remove(filename);
               return;
            }
         }
         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            bug("load_quest_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "QUEST"))
         {
            fread_quest_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
         {
            break;
         }
         else
         {
            bug("load_quest_data: bad section.", 0);
            continue;
         }
      }
      fclose(fp);
      remove(filename);
   }
   else
      return;
}
void save_quest_contents()
{
   FILE *fp;
   char filename[MIL];
   AREA_DATA *area;
   CHAR_DATA *ch;
   OBJ_DATA *obj;          
   
   sprintf(filename, "%s", QUEST_MFILE);
   if ((fp = fopen(filename, "w")) == NULL)
   {   
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (area = first_area; area; area = area->next)
   {
      if (area->low_r_vnum >= START_QUEST_VNUM && area->hi_r_vnum <= END_QUEST_VNUM)
      {
         for (ch = first_char; ch; ch = ch->next)
         {
            if (ch->in_room && ch->in_room->vnum && (ch->in_room->vnum >= area->low_r_vnum && ch->in_room->vnum <= area->hi_r_vnum))
            {
               if (!IS_NPC(ch) || ch == supermob || xIS_SET(ch->act, ACT_PROTOTYPE) || xIS_SET(ch->act, ACT_PET)
               ||  xIS_SET(ch->act, ACT_MOUNTSAVE))
               {
                  continue;
               }
               else
               {
                  fwrite_mobile(fp, ch);
               }
            }
         }
      }
   }
   fprintf(fp, "#END\n");
   fclose(fp);
   sprintf(filename, "%s", QUEST_OFILE);
   if ((fp = fopen(filename, "w")) == NULL)
   {   
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (area = first_area; area; area = area->next)
   {
      if (area->low_r_vnum >= START_QUEST_VNUM && area->hi_r_vnum <= END_QUEST_VNUM)
      {
         for (obj = first_object; obj; obj = obj->next)
         {
            if (obj->item_type != ITEM_CORPSE_PC)
            {
               if (obj->in_room && obj->in_room->vnum && (obj->in_room->vnum >= area->low_r_vnum && obj->in_room->vnum <= area->hi_r_vnum))
               {
                  fwrite_obj(NULL, obj, fp, 0, OS_GROUND);
               }
            }
         }
      }
   }
   fprintf(fp, "#END\n");
   fclose(fp);
   sprintf(filename, "%s", QUEST_LAREA);
   if ((fp = fopen(filename, "w")) == NULL)
   {   
      bug("Cannot open: %s for writing", filename);
      return;
   }
   for (area = first_area; area; area = area->next)
   {
      if (area->low_r_vnum >= START_QUEST_VNUM && area->hi_r_vnum <= END_QUEST_VNUM)
      {
         fprintf(fp, "%s\n", area->filename);
         fold_area(area, area->filename, FALSE, 0);
      }
   }      
   fprintf(fp, "$\n");
   fclose(fp);
}
   	
void save_quest_data()
{
   QUEST_DATA *quest;
   FILE *fp;
   char filename[MIL];
   sprintf(filename, "%s", QUEST_FILE);
   
   if ((fp = fopen(filename, "w")) == NULL)
   {   
      bug("Cannot open: %s for writing", filename);
      return;
   }
   fprintf(fp, "QuestVnum   %d\n", top_quest_vnum);
   for (quest = first_quest; quest; quest = quest->next)
   {
      fprintf(fp, "#QUEST\n");
      fprintf(fp, "Player      %d %d %d %d %d %d\n", quest->player[0], quest->player[1], quest->player[2], quest->player[3],
         quest->player[4], quest->player[5]);
      fprintf(fp, "Qp          %d %d %d %d %d %d\n", quest->qp[0], quest->qp[1], quest->qp[2], quest->qp[3], quest->qp[4], 
         quest->qp[5]);
      fprintf(fp, "Timeleft    %d\n", quest->timeleft);
      fprintf(fp, "Traveltime  %d\n", quest->traveltime);
      fprintf(fp, "Tillnew     %d\n", quest->tillnew);
      fprintf(fp, "Mission     %d\n", quest->mission);
      fprintf(fp, "Tokill      %d\n", quest->tokill);
      fprintf(fp, "Killed      %d\n", quest->killed);
      fprintf(fp, "X           %d\n", quest->x);
      fprintf(fp, "Y           %d\n", quest->y);
      fprintf(fp, "Map         %d\n", quest->map);
      fprintf(fp, "Difficulty  %d\n", quest->difficulty);
      fprintf(fp, "Area        %d\n", quest->questarea->low_r_vnum);
      fprintf(fp, "End\n");
   }
   fprintf(fp, "#END\n");
   fclose(fp);
   save_quest_contents();
   return;
}

void do_qobj(CHAR_DATA *ch, char *argument)
{
   QOBJ_DATA *qobj;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   char arg6[MIL];
   char arg7[MIL];
   char arg8[MIL];
   char arg9[MIL];
   char buf[MSL];
   int cnt = 0;
   int x;
   int race;
   
   if (argument[0] == '\0')
   {
       send_to_char("Syntax:  qobj list [low diff] [hi diff]\n\r", ch);
       send_to_char("Syntax:  qobj qps [low diff] [hi diff]\n\r", ch);
       send_to_char("Syntax:  qobj add <boss #> <lowdiff> <hidiff> <race> <name> <type> <weight> <gold> <values>\n\r", ch);
       send_to_char("Syntax:  qobj delete <num>\n\r", ch);
       send_to_char("Syntax:  qobj edit <num> [boss/lodiff/hidiff/race/values/name/flags/type/weight/gold/qps/aflags] <value>\n\r", ch);
       ch_printf(ch, "Boss (0 - Reg, 1 - Captain, 2 - Boss) Difficulty (1-%d) Race(0-1)\n\r", MAX_QDIFF);
       return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   
   if (!str_cmp(arg1, "edit"))
   {
      if (atoi(arg2) < 1)
      {
         send_to_char("Need to choose the number listed in qobj list\n\r", ch);
         return;
      }
      for (qobj = first_qobj; qobj; qobj = qobj->next)
      {
         if (++cnt == atoi(arg2))
         {
            if (!str_cmp(arg3, "flags"))
            {
               xCLEAR_BITS(qobj->flags);
               xCLEAR_BITS(qobj->flags2);
               for (;;)
               {
                  argument = one_argument(argument, arg4);  
                  if (arg4[0] == '\0')
                     break;
                  else
                  {
                     if (atoi(arg4) > MAX_BITS)
                        xSET_BIT(qobj->flags2, atoi(arg4)-MAX_BITS-1);
                     else
                        xSET_BIT(qobj->flags, atoi(arg4)-1);
                  }
               }
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "aflags"))
            {
               for (;;)
               {
                  argument = one_argument(argument, arg4);  
                  if (arg4[0] == '\0')
                     break;
                  else
                  {
                     if (atoi(arg4) > MAX_BITS)
                        xTOGGLE_BIT(qobj->flags2, atoi(arg4)-MAX_BITS-1);
                     else
                        xTOGGLE_BIT(qobj->flags, atoi(arg4)-1);
                  }
               }
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "boss"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 2)
               {
                  send_to_char("Boss value is 0 - 2\n\r", ch);
                  return;
               }      
               qobj->boss = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "lodiff"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_QDIFF)
               {
                  ch_printf(ch, "Difficulty value is 1 - %d\n\r", MAX_QDIFF);
                  return;
               }    
               if (atoi(argument) > qobj->hidiff)
               {
                  send_to_char("Lowdiff has to be less than or equal to hidiff\n\r", ch);
                  return;
               }
               qobj->lowdiff = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "hidiff"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_QDIFF)
               {
                  ch_printf(ch, "Difficulty value is 1 - %d\n\r", MAX_QDIFF);
                  return;
               }    
               if (atoi(argument) < qobj->lowdiff)
               {
                  send_to_char("Hidiff has to be greater than or equal to lowdiff\n\r", ch);
                  return;
               }
               qobj->hidiff = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "race"))
            {
               race = atoi(argument);
               if (race < 0 || race > 1)
               {
                  send_to_char("Race value is 0 or 1.\n\r", ch);
                  return;
               }
               qobj->race = race;
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "name"))
            {
               STRFREE(qobj->name);
               qobj->name = STRALLOC(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "type"))
            {
               qobj->type = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "qps"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("The qps range is 0 to 9,999,999.\n\r", ch);
                  return;
               }
               qobj->qps = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "gold"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("The gold range is 0 to 9,999,999.\n\r", ch);
                  return;
               }
               qobj->gold = atoi(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
            if (!str_cmp(arg3, "weight"))
            {
               if (atof(argument) < .01 || atof(argument) > 200)
               {
                  send_to_char("The weight range is .01 to 200.\n\r", ch);
                  return;
               }
               qobj->weight = adjust_weight(argument);
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
               
            if (!str_cmp(arg3, "value"))
            {
               for (x = 0; x <= MAX_QDIFF; x++)
               {
                  argument = one_argument(argument, arg3);
                  qobj->value[x] = atoi(arg3);
               }
               send_to_char("Done.\n\r", ch);
               fwrite_qobj_data();
               return;
            }
         }
      }
   }
   argument = one_argument(argument, arg4);
   argument = one_argument(argument, arg5);
   argument = one_argument(argument, arg6);
   argument = one_argument(argument, arg7);
   argument = one_argument(argument, arg8);
   argument = one_argument(argument, arg9);
   if (!str_cmp(arg1, "delete"))
   {
      if (atoi(arg2) < 1)
      {
         send_to_char("Need to choose the number listed in qobj list\n\r", ch);
         return;
      }
      for (qobj = first_qobj; qobj; qobj = qobj->next)
      {
         if (++cnt == atoi(arg2))
         {
            STRFREE(qobj->name);
            UNLINK(qobj, first_qobj, last_qobj, next, prev);
            DISPOSE(qobj);
            fwrite_qobj_data();
            send_to_char("Done.\n\r", ch);
            return;
         }
      }
      send_to_char("That number does not exist, check qobj list again.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "add"))
   {
      if (atoi(arg2) < 0 || atoi(arg2) > 2)
      {
         send_to_char("Boss value is 0 - 2\n\r", ch);
         return;
      }
      if (atoi(arg3) < 1 || atoi(arg3) > MAX_QDIFF)
      {
         ch_printf(ch, "Difficulty value is 1 - %d\n\r", MAX_QDIFF);
         return;
      }
      if (atoi(arg4) < 1 || atoi(arg4) > MAX_QDIFF)
      {
         ch_printf(ch, "Difficulty value is 1 - %d\n\r", MAX_QDIFF);
         return;
      }
      if (atoi(arg3) > atoi(arg4))
      {
         send_to_char("Lowdiff has to be less than or equal to hidiff\n\r", ch);
         return;
      }
      race = atoi(arg5);
      if (race < 0 || race > 1)
      {
         send_to_char("Race value is 0 - 1.\n\r", ch);
         return;
      }
      if (atof(arg8) < .01 || atof(arg8) > 200)
      {
         send_to_char("Weight value is .01 - 200.\n\r", ch);
         return;
      }
      if (atoi(arg9) < 0 || atoi(arg9) > 9999999)
      {
         send_to_char("Gold value is 0 - 9999999.\n\r", ch);
         return;
      }
      CREATE(qobj, QOBJ_DATA, 1);
      qobj->boss = atoi(arg2);
      qobj->lowdiff = atoi(arg3);
      qobj->hidiff = atoi(arg4);
      qobj->race = race;
      qobj->name = STRALLOC(arg6);
      qobj->type = atoi(arg7);
      qobj->weight = adjust_weight(arg8);
      qobj->gold = atoi(arg9);
      for (x = 0; x <= MAX_QDIFF; x++)
      {
         argument = one_argument(argument, arg3);
         qobj->value[x] = atoi(arg3);
      }
      
      LINK(qobj, first_qobj, last_qobj, next, prev);
      fwrite_qobj_data();
      send_to_char("Done.\n\r", ch);
      return;
   }     
   
   if (!str_cmp(arg1, "list"))
   {
      if (atoi(arg2) > MAX_QDIFF || atoi(arg3) > MAX_QDIFF)
      {
         ch_printf(ch, "Range is 0 to %d\n\r", MAX_QDIFF);
         return;
      }
      if (atoi(arg2) < 1 || atoi(arg3) < 1)
      {
         send_to_char("&c&wNum   Name                     Boss  Lo  Hi  Race  Type  Gold     Lbs     Values                                      Flags\n\r---------------------------------------------------------------------------------------------------------------------------\n\r", ch);
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            sprintf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d", qobj->value[0], qobj->value[1], qobj->value[2],
               qobj->value[3], qobj->value[4], qobj->value[5], qobj->value[6], qobj->value[7], qobj->value[8], qobj->value[9], 
               qobj->value[10], qobj->value[11], qobj->value[12], qobj->value[13]); 
            ch_printf(ch, "[%-3d] %-22.22s > %-4s  %-2d  %-2d  %d     %-2d    %-7d  %-6.2f  %-42s  %s\n\r", ++cnt, qobj->name, 
               qobj->boss > 0 ? qobj->boss > 1 ? "&w&RBoss&c&w" : "&w&CCapt&c&w" : "&w&GRegu&c&w", qobj->lowdiff, qobj->hidiff, 
               qobj->race, qobj->type, qobj->gold, qobj->weight, buf, print_qobj_flags(qobj));
         }
      }
      else
      {
         send_to_char("&c&wNum   Name                     Boss  Lo  Hi  Race  Type  Gold     Lbs     Values                                      Flags\n\r---------------------------------------------------------------------------------------------------------------------------\n\r", ch);
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            cnt++;
            if (is_in_range(qobj->lowdiff, qobj->hidiff, atoi(arg2), atoi(arg3)))
            {
               sprintf(buf, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d", qobj->value[0], qobj->value[1], qobj->value[2],
               qobj->value[3], qobj->value[4], qobj->value[5], qobj->value[6], qobj->value[7], qobj->value[8], qobj->value[9], 
               qobj->value[10], qobj->value[11], qobj->value[12], qobj->value[13]); 
               ch_printf(ch, "[%-3d] %-22.22s > %-4s  %-2d  %-2d  %d     %-2d    %-7d  %-6.2f  %-42s  %s\n\r", cnt, qobj->name, 
                  qobj->boss > 0 ? qobj->boss > 1 ? "&w&RBoss&c&w" : "&w&CCapt&c&w" : "&w&GRegu&c&w", qobj->lowdiff, qobj->hidiff, 
                  qobj->race, qobj->type, qobj->gold, qobj->weight, buf, print_qobj_flags(qobj));
            }
         }
      }      
      return;
   }
   if (!str_cmp(arg1, "qps"))
   {
      if (atoi(arg2) > MAX_QDIFF || atoi(arg3) > MAX_QDIFF)
      {
         ch_printf(ch, "Range is 0 to %d\n\r", MAX_QDIFF);
         return;
      }
      if (atoi(arg2) < 1 || atoi(arg3) < 1)
      {
         send_to_char("&c&wNum   Name                     Boss  Lo  Hi  Race  Type  Gold     Lbs  QPS\n\r--------------------------------------------------------------------------------\n\r", ch);
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            ch_printf(ch, "[%-3d] %-22.22s > %-4s  %-2d  %-2d  %d     %-2d    %-7d  %-3d  %d\n\r", ++cnt, qobj->name, 
               qobj->boss > 0 ? qobj->boss > 1 ? "&w&RBoss&c&w" : "&w&CCapt&c&w" : "&w&GRegu&c&w", qobj->lowdiff, qobj->hidiff, 
               qobj->race, qobj->type, qobj->gold, qobj->weight, qobj->qps);
         }
      }
      else
      {
         send_to_char("&c&wNum   Name                     Boss  Lo  Hi  Race  Type  Gold     Lbs  QPS\n\r--------------------------------------------------------------------------------\n\r", ch);
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            cnt++;
            if (is_in_range(qobj->lowdiff, qobj->hidiff, atoi(arg2), atoi(arg3)))
            {
               ch_printf(ch, "[%-3d] %-22.22s > %-4s  %-2d  %-2d  %d     %-2d    %-7d  %-3d  %d\n\r", cnt, qobj->name, 
                  qobj->boss > 0 ? qobj->boss > 1 ? "&w&RBoss&c&w" : "&w&CCapt&c&w" : "&w&GRegu&c&w", qobj->lowdiff, qobj->hidiff, 
                  qobj->race, qobj->type, qobj->gold, qobj->weight, qobj->qps);
            }
         }
      }      
      return;
   }
   do_qobj(ch, "");
   return;
}
				
void fread_qmob_data(FILE *fp)
{
   bool fMatch;  
   char buf[MSL];   
   char *word;
   QMOB_DATA *qmob;
   
   CREATE(qmob, QMOB_DATA, 1);
   
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
            KEY("Boss", qmob->boss, fread_number(fp));
            break;
            
         case 'F':
            KEY("Flags", qmob->flags, fread_bitvector(fp));            
            break;
            
         case 'H':
            KEY("Hidiff", qmob->hidiff, fread_number(fp));
            break;
            
         case 'L':
            KEY("Lowdiff", qmob->lowdiff, fread_number(fp));
            break;
            
         case 'N':
            KEY("Name", qmob->name, fread_string(fp));
            break;
            
         case 'R':
            KEY("Race", qmob->race, fread_number(fp));
            break;
            
         case 'S':
            KEY("Sex", qmob->sex, fread_number(fp));
            break;
            
         case 'E':
            if (!str_cmp(word, "End"))
            {
               LINK(qmob, first_qmob, last_qmob, next, prev);
               return;       
            }
      }
      if (!fMatch)
      {
         sprintf(buf, "fread_qmob_data: no match: %s", word);
         bug(buf, 0);
      }
   }
}        
                   
void load_qmob_data()
{
   FILE *fp;
   char filename[MIL];
   
   sprintf(filename, "%s", QMOB_FILE);
   if ((fp = fopen(filename, "r")) != NULL)
   {
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
            bug("load_qmob_data: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "MOB"))
         {
            fread_qmob_data(fp);
            continue;
         }
         else if (!str_cmp(word, "END"))
         {
            break;
         }
         else
         {
            bug("load_qmob_data: bad section.", 0);
            continue;
         }
      }
   }
   else
   {
      bug("Cannot open the qmob data file", 0);
      return;
   }
   fclose(fp);
}   

void set_quest_description(ROOM_INDEX_DATA *location)
{
   char namebuf[MSL];
   char descbuf[MSL];
   int x = number_range(1, 6);
   
   if (location->sector_type == 2) //Field
   {
      STRFREE(location->name);
      STRFREE(location->description);
      if (x == 1)
      {
         sprintf(namebuf, "A large grass field.");
         sprintf(descbuf, "In this evil cave this simple field of grass is about\n\r\as tranquil as it can get.  However, in such peace\n\ris when you need to be the most careful.\n\r");
      }   
      if (x == 2)
      {
         sprintf(namebuf, "A bloodstained field.");
         sprintf(descbuf, "The once green grass now has a lovely shade of red\n\rinstead of the alive green color it use to have.\n\rIf you stick around here for to long you just might\n\radd more red to the grass.\n\r");
      }
      if (x == 3)
      {
         sprintf(namebuf, "A trampled field.");
         sprintf(descbuf, "Looking closely at the grass it is easy to see some\n\rhidious beast has been through here recently and\n\rit did not look too be prancing through either by the\n\rmarkings in the grass.\n\r");
      }
      if (x == 4)
      {
         sprintf(namebuf, "A field of grass.");
         sprintf(descbuf, "Some green grass still grows here making this perhaps\n\rone of the only few spots unsoiled in this cave.\n\rIt is nice to look down and see life, but a lot of death\n\ris still waiting you ahead.\n\r");
      }
      if (x == 5)
      {
         sprintf(namebuf, "A flat clearing of grass.");
         sprintf(descbuf, "A few strains of grass still grow here.  It is kind of\n\rfunny how a little bit of grass can grow in a cave\n\rlike this, but it still managed to live on.  Perhaps\n\rsome light is creeping through somwhere.\n\r");
      }
      if (x == 6)
      {
         sprintf(namebuf, "A little grass.");
         sprintf(descbuf, "A little bit of grass mixes in here with the flattened\n\rred grass that seems to mark the death and pain in this\n\rhorrid place.  Unless you want to turn some more\n\rgreen grass red it might be a good idea to get moving.\n\r");
      }
      location->name = STRALLOC(namebuf);
      location->description = STRALLOC(descbuf);
   }
   if (location->sector_type == 13) //Underground
   {
      STRFREE(location->name);
      STRFREE(location->description);
      if (x == 1)
      {
         sprintf(namebuf, "Another path underground.");
         sprintf(descbuf, "As with any direction that you look, it appears this horrid\n\rdungeon has no end.  Just more and more underground\n\rpaths that lead most certainly to pain and doom.\n\rNow would be the the time to flee if want!\n\r");
      }   
      if (x == 2)
      {
         sprintf(namebuf, "A bloodied wall.");
         sprintf(descbuf, "Blood trickles all over the ground and the dugout walls.  All\n\raround you is the smell of rotting humanoid creatures\n\rand monsters only seen in nightmeres of children.\n\rNow would be a good time to hug your weapon.\n\r");
      }
      if (x == 3)
      {
         sprintf(namebuf, "A little light.");
         sprintf(descbuf, "A little bit of light seems to creep in from above.  Looking up\n\rtoward the light you can see the outside world, well\n\ratleast small parts of it.  This is probably how\n\rgrass can grow here and a bit of water can be found\n\r");
      }
      if (x == 4)
      {
         sprintf(namebuf, "Bodies litter the ground.");
         sprintf(descbuf, "The horrible stench of death runs as putrid as it gets in this little\n\ralcove.  Bodies are stacked up into the sky as\n\rperhaps an offering to some evil god or as a sign\n\rto all who would see it.\n\r");
      }
      if (x == 5)
      {
         sprintf(namebuf, "Shreeks echo through the halls.");
         sprintf(descbuf, "Shreeks of the living and the soon to be dead can be heard in all directions\n\rfrom this point in the dungeon.  The sounds are mostly\n\rthe creatures of the dungeons communicating\n\rwith eachother and the sounds of victory.\n\r");
      }
      if (x == 6)
      {
         sprintf(namebuf, "Some small scribles on the cavern walls");
         sprintf(descbuf, "It is really hard to read and understand, but it appears the creatures that\n\rinhabit this foul place have left behind some sort of\n\rhorrid looking history.  These creatures sure\n\rappear to enjoy killing as noted by the last picture of bodies\n\rspread out everywhere.\n\r");
      }
      location->name = STRALLOC(namebuf);
      location->description = STRALLOC(descbuf);
   }
   if (location->sector_type == 44) //Path
   {
      STRFREE(location->name);
      STRFREE(location->description);
      if (x == 1)
      {
         sprintf(namebuf, "A path into darkness.");
         sprintf(descbuf, "The path quickly starts to turn very dark here for\n\rreally no good reason.  Light almost seems to be eaten\n\rat this point.  Whatever is down this path does\n\rnot want to be seen by anyone.\n\r");
      }   
      if (x == 2)
      {
         sprintf(namebuf, "A path full of blood.");
         sprintf(descbuf, "Blood seems to pour out of the ground here.  All around\n\rblood just drips from every corner and wall.  It almost\n\reven looks like it is a slive in the way it is\n\rmoving.  Might be a good idea to start moving again.\n\r");
      }
      if (x == 3)
      {
         sprintf(namebuf, "A small path that winds into oblivion.");
         sprintf(descbuf, "This room just oozes with bad intentions.  Evil seems to\n\rpulsating from every direction and death is the only thing\n\rcertain in all of those directions.  Now would\n\rbe a good time to prey to whatever you beleive in.\n\r");
      }
      if (x == 4)
      {
         sprintf(namebuf, "A strange light from above.");
         sprintf(descbuf, "A small bit of light shines in from above.  The light offers\n\ra small amount of relaxation in a normally horid situation, but\n\rwhenever there is calm there is always a\n\rnasty storm to follow, better be prepared\n\r");
      }
      if (x == 5)
      {
         sprintf(namebuf, "A little bit of water sprinkles down the path.");
         sprintf(descbuf, "A small bit of water runs down the path here.  A small amount\n\rof grass can be seen growing in this area, but not enough to really\n\rhave any affect.  It is nice to stand around\n\rand look at the water and dream, but it is better to live.\n\r");
      }
      if (x == 6)
      {
         sprintf(namebuf, "The sounds of battle can be hard nearby.");
         sprintf(descbuf, "The grunting of the enemy echoes loudly here.  Just a short\n\rdistance the enemy is preparing to delivery you the bloody death that\n\rthe vacation guide mentioned before arriving here.\n\rMight be a good time to get ready for battle.\n\r");
      }
      location->name = STRALLOC(namebuf);
      location->description = STRALLOC(descbuf);
   }
   if (location->sector_type == 6) //Water
   {
      STRFREE(location->name);
      STRFREE(location->description);
      if (x == 1)
      {
         sprintf(namebuf, "A small pond in the darkness.");
         sprintf(descbuf, "A small pond of water in what is normally a dreadful dungeon\n\ris here for all to appreciate.  It might not be safe\n\rto drink from it but perhaps taking a quick second\n\rto wipe the death of your weapons might be a soothing time.\n\r");
      }   
      if (x == 2)
      {
         sprintf(namebuf, "A bloody red pool.");
         sprintf(descbuf, "The blood seems just to trickle down into the small pool of\n\rwater in this room.  The once clear and perhaps drinkable\n\rwater is now a blood red.  It is even quite possible\n\rthat it is simply just blood instead of water.\n\r");
      }
      if (x == 3)
      {
         sprintf(namebuf, "A small tranquil pool.");
         sprintf(descbuf, "The water here moves slowly and creates a very soothing\n\ratmosphere.  In any other situation it might be a good idea to sit,\n\rrest, and heal a little bit, but with the critters\n\rlooking for some fresh flesh it might be a good idea to keep moving\n\r");
      }
      if (x == 4)
      {
         sprintf(namebuf, "Some soothing blue water.");
         sprintf(descbuf, "The water here is a about a foot deep, which is good for\n\ran underground cave that shouldn't have any water at all.  The water\n\rlooks very clear and purge compared to most of this\n\rhorrible place.  It is a nice token of the hell to come.\n\r");
      }
      if (x == 5)
      {
         sprintf(namebuf, "A defiled pool of water.");
         sprintf(descbuf, "The pool of water in this rooms absolutely stinks foul of\n\rthings dead many days ago.  The evil creatures here have used the\n\rwater to dump their dead and the bones of their victims\n\rinstead of burrying or burning it.\n\r");
      }
      if (x == 6)
      {
         sprintf(namebuf, "A small stream runs through.");
         sprintf(descbuf, "A small stream of water slowly trickles through this room.\n\rIt is amazing to see something so pure and tranquil in this maze\n\rof doom, but that can only mean something horrid looking\n\rand smelling is waiting for you.\n\r");
      }
      location->name = STRALLOC(namebuf);
      location->description = STRALLOC(descbuf);
   }
}
   

int get_coord_dir(int x, int y)
{
   if (x == 0 && y == 1) //n
      return 0;
   if (x == 1 && y == 0) //e
      return 1;
   if (x == 0 && y == -1) //s
      return 2;
   if (x == -1 && y == 0) //w
      return 3;
   if (x == 1 && y == 1)  //ne
      return 6;
   if (x == -1 && y == 1) //nw
      return 7;
   if (x == 1 && y == -1)  //se
      return 8;
   if (x == -1 && y == -1)  //sw
      return 9;
      
   return 0;
}
   
char gridmap[201][201];
ROOM_INDEX_DATA *roomgrid[201][201];

char *const qmob_long_descr[18] = {
   "is here looking for trouble.", "has bad intentions for YOU!", "appears to be hungry for BLOOD!",
   "is looking for another victim!", "has a taste for DEATH today", "likes to kill innocent folks",
   "is here looking for a fight.", "is here looking for lunch", "is here go rip YOU a new one!",
   "is thinking about killing YOU!", "is here planning to stop YOU!", "is taking a liking to you DEAD!",
   "is here looking for skulls to CRACK!", "is here looking for souls to STEAL!", "is here and likes the SMELL OF DEATH!",
   "is here HUNTING YOU!", "is here looking to send you to your next life.", "is here to make you FEEL PAIN!"
};

char *const qobj_long_descr[12] = {
   "has been left behind for anyone to pickup.", "has been foolishly left here.", "has been left here for the next owner",
   "appears to be looking for a new owner", "is here for you to claim.", "is here for anyone to take",
   "is looking for a new owner.", "is here collecting dust.", "is here growing old.", "is here for the taking.",
   "lies here untouched.", "has been left behind for another."
};

void apply_npcquest_flags(EXT_BV *flags, NPCRACE_DATA *npcrace, int diff)
{
   int x = 1;
   
   for (x = 1; x <= diff; x++)
   {
      xSET_BITS(*flags, npcrace->flags[x-1]);
      xREMOVE_BITS(*flags, npcrace->nflags[x-1]);
   }
}

void adjust_qmob_flags(CHAR_DATA *mob, QMOB_DATA *qmob, NPCRACE_DATA *npcrace, int diff)
{
   int histat = 0;
   int lostat = 25;
   EXT_BV flags;
   
   xCLEAR_BITS(flags);
   if (mob->perm_str > histat)
      histat = mob->perm_str;
   if (mob->perm_dex > histat)
      histat = mob->perm_dex;
   if (mob->perm_int > histat)
      histat = mob->perm_int;
   if (mob->perm_lck > histat)
      histat = mob->perm_lck;
   if (mob->perm_wis > histat)
      histat = mob->perm_wis;
   if (mob->perm_con > histat)
      histat = mob->perm_con;
      
   if (mob->perm_str < lostat)
      lostat = mob->perm_str;
   if (mob->perm_dex < lostat)
      lostat = mob->perm_dex;
   if (mob->perm_int < lostat)
      lostat = mob->perm_int;
   if (mob->perm_lck < lostat)
      lostat = mob->perm_lck;
   if (mob->perm_wis < lostat)
      lostat = mob->perm_wis;
   if (mob->perm_con < lostat)
      lostat = mob->perm_con;
      
   if (qmob)
      flags = qmob->flags;
   else
      apply_npcquest_flags(&flags, npcrace, diff);
      
   if (xIS_SET(flags, QMOB_HP))
      mob->hit = mob->hit * 120/100;
   if (xIS_SET(flags, QMOB_LHP))
      mob->hit = mob->hit * 75/100;
   if (xIS_SET(flags, QMOB_AGI))
      mob->perm_agi += 10;
   if (xIS_SET(flags, QMOB_LAGI))
      mob->perm_agi -= 10;
   if (xIS_SET(flags, QMOB_STR))
      mob->perm_str = histat+2;
   if (xIS_SET(flags, QMOB_STR2) && !xIS_SET(flags, QMOB_STR))
      mob->perm_str = histat+4;
   if (xIS_SET(flags, QMOB_WIS))
      mob->perm_wis = histat+2;
   if (xIS_SET(flags, QMOB_WIS2) && !xIS_SET(flags, QMOB_WIS))
      mob->perm_wis = histat+4;
   if (xIS_SET(flags, QMOB_INT))
      mob->perm_int = histat+2;
   if (xIS_SET(flags, QMOB_INT2) && !xIS_SET(flags, QMOB_INT))
      mob->perm_int = histat+4;
   if (xIS_SET(flags, QMOB_DEX))
      mob->perm_dex = histat+2;
   if (xIS_SET(flags, QMOB_CON))
      mob->perm_con = histat+2;
   if (xIS_SET(flags, QMOB_LCK))
      mob->perm_lck = histat+2;
   if (xIS_SET(flags, QMOB_LSTR))
      mob->perm_str = lostat-2;
   if (xIS_SET(flags, QMOB_LSTR2) && !xIS_SET(flags, QMOB_LSTR))
      mob->perm_str = lostat-4;
   if (xIS_SET(flags, QMOB_LWIS))
      mob->perm_wis = lostat-2;
   if (xIS_SET(flags, QMOB_LWIS2) && !xIS_SET(flags, QMOB_LWIS))
      mob->perm_wis = lostat-4;
   if (xIS_SET(flags, QMOB_LINT))
      mob->perm_int = lostat-2;
   if (xIS_SET(flags, QMOB_LINT2) && !xIS_SET(flags, QMOB_LINT))
      mob->perm_int = lostat-4;
   if (xIS_SET(flags, QMOB_LDEX))
      mob->perm_dex = lostat-2;
   if (xIS_SET(flags, QMOB_LCON))
      mob->perm_con = lostat-2;
   if (xIS_SET(flags, QMOB_LLCK))
      mob->perm_lck = lostat-2;
     
   if (xIS_SET(flags, QMOB_BASH))
      mob->tohitbash = mob->armor+2;
   if (xIS_SET(flags, QMOB_SLASH))
      mob->tohitslash = mob->armor+2;
   if (xIS_SET(flags, QMOB_STAB))
      mob->tohitstab = mob->armor+2;
   if (xIS_SET(flags, QMOB_LBASH))
      mob->tohitbash = mob->armor-2;
   if (xIS_SET(flags, QMOB_LSLASH))
      mob->tohitslash = mob->armor-2;
   if (xIS_SET(flags, QMOB_LSTAB))
      mob->tohitstab = mob->armor-2;
   if (xIS_SET(flags, QMOB_ARMOR))
      mob->armor += 1;
   if (xIS_SET(flags, QMOB_ARMOR2) && !xIS_SET(flags, QMOB_ARMOR))
      mob->armor += 2;
   if (xIS_SET(flags, QMOB_LARMOR))
      mob->armor -= 1;
   if (xIS_SET(flags, QMOB_LARMOR2) && !xIS_SET(flags, QMOB_LARMOR))
      mob->armor -= 2;
   if (xIS_SET(flags, QMOB_DAM))
      mob->damplus = UMAX(mob->damplus+1, mob->damplus * 120/100);
   if (xIS_SET(flags, QMOB_DAM2) && !xIS_SET(flags, QMOB_DAM))
      mob->damplus = UMAX(mob->damplus+2, mob->damplus * 140/100);
   if (xIS_SET(flags, QMOB_LDAM))
      mob->damplus = UMIN(mob->damplus-1, mob->damplus * 85/100);
   if (xIS_SET(flags, QMOB_LDAM2) && !xIS_SET(flags, QMOB_LDAM))
      mob->damplus = UMIN(mob->damplus-2, mob->damplus * 70/100);
      
   if (xIS_SET(flags, QMOB_NOGOLD))
      mob->gold = 0;
   if (xIS_SET(flags, QMOB_GOLD1))
      mob->gold = mob->gold * 150/100;
   if (xIS_SET(flags, QMOB_GOLD2) && !xIS_SET(flags, QMOB_GOLD1))
      mob->gold = mob->gold * 250/100;
   if (xIS_SET(flags, QMOB_LGOLD1))
      mob->gold = mob->gold * 60/100;
   if (xIS_SET(flags, QMOB_LGOLD2) && !xIS_SET(flags, QMOB_LGOLD1))
      mob->gold = mob->gold * 35/100;
      
   if (xIS_SET(flags, QMOB_NOAGGRO))
      xREMOVE_BIT(mob->act, ACT_AGGRESSIVE);   
   if (xIS_SET(flags, QMOB_RUNNING))
      xSET_BIT(mob->act, ACT_RUNNING);
   if (xIS_SET(flags, QMOB_UNDEAD))
      xSET_BIT(mob->act, ACT_UNDEAD);
   if (xIS_SET(flags, QMOB_LIVINGDEAD))
      xSET_BIT(mob->act, ACT_LIVING_DEAD);
   if (xIS_SET(flags, QMOB_SENTINEL))
      xSET_BIT(mob->act, ACT_SENTINEL);   
      
   if (xIS_SET(flags, QMOB_DODGE))
      xSET_BIT(mob->defenses, DFND_DODGE);
   if (xIS_SET(flags, QMOB_PARRY))
      xSET_BIT(mob->defenses, DFND_PARRY);
   if (xIS_SET(flags, QMOB_LIGHT))
      xSET_BIT(mob->defenses, DFND_CURELIGHT);
   if (xIS_SET(flags, QMOB_SERIOUS))
      xSET_BIT(mob->defenses, DFND_CURESERIOUS);
   if (xIS_SET(flags, QMOB_CRITICAL))
      xSET_BIT(mob->defenses, DFND_CURECRITICAL);
   if (xIS_SET(flags, QMOB_HEAL))
      xSET_BIT(mob->defenses, DFND_HEAL);
   if (xIS_SET(flags, QMOB_SANCTUARY))
      xSET_BIT(mob->defenses, DFND_SANCTUARY);
   if (xIS_SET(flags, QMOB_FSHIELD))
      xSET_BIT(mob->defenses, DFND_FIRESHIELD);
   if (xIS_SET(flags, QMOB_SSHIELD))
      xSET_BIT(mob->defenses, DFND_SHOCKSHIELD);
   if (xIS_SET(flags, QMOB_ISHIELD))
      xSET_BIT(mob->defenses, DFND_ICESHIELD);
   if (xIS_SET(flags, QMOB_STONESKIN))
      xSET_BIT(mob->defenses, DFND_STONESKIN);
   if (xIS_SET(flags, QMOB_DISARM))
      xSET_BIT(mob->defenses, DFND_DISARM); 
   
   if (xIS_SET(flags, QMOB_TRIP))
      xSET_BIT(mob->attacks, ATCK_TRIP);
   if (xIS_SET(flags, QMOB_ABASH))
      xSET_BIT(mob->attacks, ATCK_BASH);
   if (xIS_SET(flags, QMOB_STUN))
      xSET_BIT(mob->attacks, ATCK_STUN);
   if (xIS_SET(flags, QMOB_GOUGE))
      xSET_BIT(mob->attacks, ATCK_GOUGE);
   if (xIS_SET(flags, QMOB_BACKSTAB))
      xSET_BIT(mob->attacks, ATCK_BACKSTAB);
   if (xIS_SET(flags, QMOB_BLINDNESS))
      xSET_BIT(mob->attacks, ATCK_BLINDNESS);
   if (xIS_SET(flags, QMOB_LBREATH))
      xSET_BIT(mob->attacks, ATCK_LIGHTNBREATH);
   if (xIS_SET(flags, QMOB_GBREATH))
      xSET_BIT(mob->attacks, ATCK_GASBREATH);
   if (xIS_SET(flags, QMOB_FIREBREATH))
      xSET_BIT(mob->attacks, ATCK_FIREBREATH);
   if (xIS_SET(flags, QMOB_FROSTBREATH))
      xSET_BIT(mob->attacks, ATCK_FROSTBREATH);
   if (xIS_SET(flags, QMOB_ACIDBREATH))
      xSET_BIT(mob->attacks, ATCK_ACIDBREATH);
   if (xIS_SET(flags, QMOB_CURSE))
      xSET_BIT(mob->attacks, ATCK_CURSE);
   if (xIS_SET(flags, QMOB_HARM))
      xSET_BIT(mob->attacks, ATCK_HARM);
   if (xIS_SET(flags, QMOB_FIREBALL))
      xSET_BIT(mob->attacks, ATCK_FIREBALL);
   if (xIS_SET(flags, QMOB_WEAKEN))
      xSET_BIT(mob->attacks, ATCK_WEAKEN);
   if (xIS_SET(flags, QMOB_POISON))
      xSET_BIT(mob->attacks, ATCK_POISON);
      
   if (xIS_SET(flags, QMOB_SFIRE))
      SET_BIT(mob->susceptible, RIS_FIRE);
   if (xIS_SET(flags, QMOB_SCOLD))
      SET_BIT(mob->susceptible, RIS_WATER);
   if (xIS_SET(flags, QMOB_SELECT))
      SET_BIT(mob->susceptible, RIS_ENERGY);
   if (xIS_SET(flags, QMOB_SENERGY))
      SET_BIT(mob->susceptible, RIS_EARTH);
   if (xIS_SET(flags, QMOB_SAIR))
      SET_BIT(mob->susceptible, RIS_AIR);
   if (xIS_SET(flags, QMOB_SBLUNT))
      SET_BIT(mob->susceptible, RIS_BLUNT);
   if (xIS_SET(flags, QMOB_SPIERCE))
      SET_BIT(mob->susceptible, RIS_PIERCE);
   if (xIS_SET(flags, QMOB_SSLASH))
      SET_BIT(mob->susceptible, RIS_SLASH);
   if (xIS_SET(flags, QMOB_SSLEEP))
      SET_BIT(mob->susceptible, RIS_SLEEP);
   if (xIS_SET(flags, QMOB_SCHARM))
      SET_BIT(mob->susceptible, RIS_CHARM);
   if (xIS_SET(flags, QMOB_SNONMAGIC))
      SET_BIT(mob->susceptible, RIS_NONMAGIC);
   if (xIS_SET(flags, QMOB_SMAGIC))
      SET_BIT(mob->susceptible, RIS_MAGIC);
   if (xIS_SET(flags, QMOB_SPARALYSIS))
      SET_BIT(mob->susceptible, RIS_PARALYSIS);
      
   if (xIS_SET(flags, QMOB_RFIRE))
      SET_BIT(mob->resistant, RIS_FIRE);
   if (xIS_SET(flags, QMOB_RCOLD))
      SET_BIT(mob->resistant, RIS_WATER);
   if (xIS_SET(flags, QMOB_RELECT))
      SET_BIT(mob->resistant, RIS_ENERGY);
   if (xIS_SET(flags, QMOB_RENERGY))
      SET_BIT(mob->resistant, RIS_EARTH);
   if (xIS_SET(flags, QMOB_RAIR))
      SET_BIT(mob->resistant, RIS_AIR);
   if (xIS_SET(flags, QMOB_RBLUNT))
      SET_BIT(mob->resistant, RIS_BLUNT);
   if (xIS_SET(flags, QMOB_RPIERCE))
      SET_BIT(mob->resistant, RIS_PIERCE);
   if (xIS_SET(flags, QMOB_RSLASH))
      SET_BIT(mob->resistant, RIS_SLASH);
   if (xIS_SET(flags, QMOB_RSLEEP))
      SET_BIT(mob->resistant, RIS_SLEEP);
   if (xIS_SET(flags, QMOB_RCHARM))
      SET_BIT(mob->resistant, RIS_CHARM);
   if (xIS_SET(flags, QMOB_RNONMAGIC))
      SET_BIT(mob->resistant, RIS_NONMAGIC);
   if (xIS_SET(flags, QMOB_RMAGIC))
      SET_BIT(mob->resistant, RIS_MAGIC);
   if (xIS_SET(flags, QMOB_RPARALYSIS))
      SET_BIT(mob->resistant, RIS_PARALYSIS);
      
   if (xIS_SET(flags, QMOB_IFIRE))
      SET_BIT(mob->immune, RIS_FIRE);
   if (xIS_SET(flags, QMOB_ICOLD))
      SET_BIT(mob->immune, RIS_WATER);
   if (xIS_SET(flags, QMOB_IELECT))
      SET_BIT(mob->immune, RIS_ENERGY);
   if (xIS_SET(flags, QMOB_IENERGY))
      SET_BIT(mob->immune, RIS_EARTH);
   if (xIS_SET(flags, QMOB_IAIR))
      SET_BIT(mob->immune, RIS_AIR);
   if (xIS_SET(flags, QMOB_IBLUNT))
      SET_BIT(mob->immune, RIS_BLUNT);
   if (xIS_SET(flags, QMOB_IPIERCE))
      SET_BIT(mob->immune, RIS_PIERCE);
   if (xIS_SET(flags, QMOB_ISLASH))
      SET_BIT(mob->immune, RIS_SLASH);
   if (xIS_SET(flags, QMOB_ISLEEP))
      SET_BIT(mob->immune, RIS_SLEEP);
   if (xIS_SET(flags, QMOB_ICHARM))
      SET_BIT(mob->immune, RIS_CHARM);
   if (xIS_SET(flags, QMOB_INONMAGIC))
      SET_BIT(mob->immune, RIS_NONMAGIC);
   if (xIS_SET(flags, QMOB_IMAGIC))
      SET_BIT(mob->immune, RIS_MAGIC);
   if (xIS_SET(flags, QMOB_IPARALYSIS))
      SET_BIT(mob->immune, RIS_PARALYSIS);
      
   if (xIS_SET(flags, QMOB_INVISIBLE))
      xSET_BIT(mob->affected_by, AFF_INVISIBLE);      
   if (xIS_SET(flags, QMOB_DETECTINVIS))
      xSET_BIT(mob->affected_by, AFF_DETECT_INVIS);
   if (xIS_SET(flags, QMOB_HIDE))
      xSET_BIT(mob->affected_by, AFF_HIDE);
   if (xIS_SET(flags, QMOB_TRUESIGHT))
      xSET_BIT(mob->affected_by, AFF_TRUESIGHT);
   if (xIS_SET(flags, QMOB_SNEAK))
      xSET_BIT(mob->affected_by, AFF_SNEAK);
   if (xIS_SET(flags, QMOB_DETECTHIDDEN))
      xSET_BIT(mob->affected_by, AFF_DETECT_HIDDEN);
      
   if (mob->perm_str < lostat)
      lostat = mob->perm_str;
   if (mob->perm_dex < lostat)
      lostat = mob->perm_dex;
   if (mob->perm_int < lostat)
      lostat = mob->perm_int;
   if (mob->perm_lck < lostat)
      lostat = mob->perm_lck;
   if (mob->perm_wis < lostat)
      lostat = mob->perm_wis;
   if (mob->perm_con < lostat)
      lostat = mob->perm_con;
      
   if (mob->perm_str < 10)
      mob->perm_str = 10;
   if (mob->perm_dex < 10)
      mob->perm_dex = 10;
   if (mob->perm_int < 10)
      mob->perm_int = 10;
   if (mob->perm_lck < 10)
      mob->perm_lck = 10;
   if (mob->perm_wis < 10)
      mob->perm_wis = 10;
   if (mob->perm_con < 10)
      mob->perm_con = 10;
      
   if (mob->perm_str > 25)
      mob->perm_str = 25;
   if (mob->perm_dex > 25)
      mob->perm_dex = 25;
   if (mob->perm_int > 25)
      mob->perm_int = 25;
   if (mob->perm_lck > 25)
      mob->perm_lck = 25;
   if (mob->perm_wis > 25)
      mob->perm_wis = 25;
   if (mob->perm_con > 25)
      mob->perm_con = 25;
      
   if (mob->perm_agi < 15)
      mob->perm_agi = 15;
   if (mob->perm_agi > 100)
      mob->perm_agi = 100;
      
   if (mob->pIndexData->damplus < 1)
      mob->pIndexData->damplus = 1;
   if (mob->armor < 1)
      mob->armor = 1;
   if (mob->armor > 25)
      mob->armor = 25;
   if (mob->tohitbash < 1)
      mob->tohitbash = 1;
   if (mob->tohitslash < 1)
      mob->tohitslash = 1;
   if (mob->tohitstab < 1)
      mob->tohitstab = 1;
   if (mob->tohitbash > 25)
      mob->tohitbash = 25;
   if (mob->tohitstab > 25)
      mob->tohitstab = 25;
   if (mob->tohitslash > 25)
      mob->tohitslash = 25;
      
   mob->max_hit = mob->hit;
   return;
}

char *add_npc_name(NPCRACE_DATA *npcrace, int diff)
{
   static char buf[MSL];
   
   if (npcrace->fulldescription[diff-1])
      sprintf(buf, "%s", npcrace->description[diff-1]);
   else
      sprintf(buf, "%s %s", npcrace->description[diff-1], npcrace->racename);
      
   return buf;
}

void adjust_questmob(CHAR_DATA *mob, int difficulty, QMOB_DATA *qmob, int vsize, AREA_DATA *area, NPCRACE_DATA *npcrace)
{
   int mult;
   int inc;
   int x;
   int lvl;
   int hp;
   int dam;
   char buf[MSL];
   OBJ_DATA *weapon;
   OBJ_DATA *weapon2 = NULL;
   OBJ_DATA *rleg;
   OBJ_DATA *lleg;
   OBJ_DATA *rarm;
   OBJ_DATA *larm;
   OBJ_DATA *neck;
   OBJ_DATA *chest;
   OBJ_DATA *head;
   SLAB_DATA *firstslab;
   SLAB_DATA *secondslab;
   SLAB_DATA *thirdslab = NULL;
   OBJ_DATA *firstslabobj = NULL;
   OBJ_DATA *secondslabobj = NULL;
   OBJ_DATA *thirdslabobj = NULL;
   int gold;
   int slab1;
   int slab2;
   int slab3;
   int per1;
   int per2;
   int per3;  
   int num;
   int race;
   int diff = ((difficulty-1)/10)+1;
   
   if (!qmob && !npcrace)
   {
      bug("adjust_questmob:  A null qmob and npcrace passed to adjust_questmob.");
      return;
   }
   if (qmob && npcrace)
   {
      bug("adjust_questmob:  Both qmob and npcrace are not null.");
      return;
   }
   
   weapon=rleg=lleg=rarm=larm=neck=chest=head=NULL;
   
   mult = 4750;
   inc = 750;
   
   lvl = difficulty;
   
   if (qmob)
   {
      if (qmob->boss == 1)
         lvl += 10;
      if (qmob->boss == 2)
         lvl += 20;
   }
   if (lvl > MAX_QDIFF_VALUE)
      lvl = MAX_QDIFF_VALUE;
   if (lvl < 5)
      lvl = 5;
      
   for (x = 11; x <= lvl; x = x+10)
   {
      inc += 250;
      if (x <= 101)
         mult += inc;

   }
   mult += 250;
   inc += 250 * (lvl-x+10) /10;
   if (lvl <= 100)
      mult += inc;     
        
   hp = lvl * mult / 2000;
   hp = number_range(hp * 90, hp * 110)/100;
      
   mob->hit = hp;
   mob->max_hit = mob->hit;
   mob->perm_agi = UMIN(100, 35+(lvl*4/10));
   if (lvl > 100)
      mob->perm_agi += 5;
   if (lvl > 110)
      mob->perm_agi += 5;
   if (lvl > 120)
      mob->perm_agi += 5;
      
   if (mob->perm_agi > 100)
      mob->perm_agi = 100;
   
   if (lvl <= 8)
   {
      if (lvl <= 3)
         mob->armor = 1;
      else if (lvl <= 6)
         mob->armor = 2;
      else
         mob->armor = 3;
   }
   else
   {
      mob->armor = 3+(lvl*100/666);
   }
   mob->tohitbash = UMAX(1, number_range(mob->armor-1, mob->armor+1));
   mob->tohitslash = UMAX(1, number_range(mob->armor-1, mob->armor+1));
   mob->tohitstab = UMAX(1, number_range(mob->armor-1, mob->armor+1));
   if (qmob && qmob->boss > 0)
      mob->tohitbash = mob->tohitslash = mob->tohitstab = mob->armor+1;
   dam = 1+lvl/5;
   if (lvl >= 48)
      dam+=(lvl-38)/10;
   if (lvl >= 97)
      dam+=1;
  
   mob->pIndexData->damnodice = 1;
   mob->pIndexData->damsizedice = 1+number_range(dam/2, dam/5);
   mob->pIndexData->damplus = dam - ((1 + (mob->pIndexData->damnodice*mob->pIndexData->damsizedice))/2);
   mob->barenumdie = mob->pIndexData->damnodice;
   mob->baresizedie = mob->pIndexData->damsizedice;
   mob->damplus = mob->pIndexData->damplus;
   mob->perm_str = 13+(lvl/12)+number_range(-2, 2);
   mob->perm_dex = 13+(lvl/12)+number_range(-2, 2);
   mob->perm_wis = 13+(lvl/12)+number_range(-2, 2);
   mob->perm_int = 13+(lvl/12)+number_range(-2, 2);
   mob->perm_lck = 13+(lvl/12)+number_range(-2, 2);
   mob->perm_wis = 13+(lvl/12)+number_range(-2, 2);
   if (lvl > 70)
   {
      dam = (lvl-70)/3;
      if (lvl > 100)
         dam += UMIN(20,(lvl-100));
      if (lvl > 120)
         dam += (lvl-120)*2;
      mob->damaddlow = dam * 75 / 100;
      mob->damaddhi = dam * 125 / 100;
      mob->pIndexData->damaddlow = mob->damaddlow;
      mob->pIndexData->damaddhi = mob->damaddhi;
   }
   if (qmob)
      mob->race = qmob->race;
   else
      mob->race = npcrace->racenum;
   if (lvl <= 10) //Target the body only....
      mob->perm_int = 6;
      
   if (mob->race >= 0 && mob->race < MAX_RACE)
   {  
      mob->gold = (lvl * lvl /4) + (lvl*10); 
      if ((number_range(1, 5) >= 4 && eqload < vsize/25) || (qmob && qmob->boss > 0))
      {
         mob->gold = (lvl * lvl /2.5) + (lvl*20); 
         if (qmob && qmob->boss <= 0)
            eqload++;
         if (lvl >= 101)
            mob->gold = 7000 + ((lvl-99)*2000);
         if (lvl >= 61 && number_range(1, 3) == 1)
         {
            weapon = create_object(get_obj_index(get_wilder_weapon(((lvl-1)/10+1),1)), 1);
            weapon2 = create_object(get_obj_index(get_wilder_weapon(((lvl-1)/10+1),1)), 1);
         }
         else
            weapon = create_object(get_obj_index(get_wilder_weapon(((lvl-1)/10+1),0)), 1);
            
         num = ((lvl-1)/10+1)+number_range(-3, 3);
         if (num < 1)
            num = 1;
            
         if (num <= 2)
         {
            rarm = create_object(get_obj_index(OBJ_FORGE_CHAIN_GAUNTLET), 1);
            larm = create_object(get_obj_index(OBJ_FORGE_CHAIN_GAUNTLET), 1);  
            neck = create_object(get_obj_index(OBJ_FORGE_AVENTAIL), 1);
            head = create_object(get_obj_index(OBJ_FORGE_CABASSET), 1);
            rleg = create_object(get_obj_index(OBJ_FORGE_CHAIN_GREAVE), 1);
            lleg = create_object(get_obj_index(OBJ_FORGE_CHAIN_GREAVE), 1);
         }
         else if (num <= 6)
         {
            rarm = create_object(get_obj_index(OBJ_FORGE_RING_GAUNTLET), 1);
            larm = create_object(get_obj_index(OBJ_FORGE_RING_GAUNTLET), 1);  
            neck = create_object(get_obj_index(OBJ_FORGE_COIF), 1);
            head = create_object(get_obj_index(OBJ_FORGE_CASQUE), 1);
            rleg = create_object(get_obj_index(OBJ_FORGE_RING_GREAVE), 1);
            lleg = create_object(get_obj_index(OBJ_FORGE_RING_GREAVE), 1);
         }
         else if (num <= 10)
         {
            rarm = create_object(get_obj_index(OBJ_FORGE_GAUNTLET), 1);   
            larm = create_object(get_obj_index(OBJ_FORGE_GAUNTLET), 1);   
            neck = create_object(get_obj_index(OBJ_FORGE_DOUBLE_COIF), 1);
            head = create_object(get_obj_index(OBJ_FORGE_ARMET), 1);
            rleg = create_object(get_obj_index(OBJ_FORGE_GREAVE), 1);
            lleg = create_object(get_obj_index(OBJ_FORGE_GREAVE), 1);
         }
         else if (num >= 10)
         {
            rarm = create_object(get_obj_index(OBJ_FORGE_VAMBRACE), 1);
            larm = create_object(get_obj_index(OBJ_FORGE_VAMBRACE), 1);
            neck = create_object(get_obj_index(OBJ_FORGE_DOUBLE_COIF), 1);
            head = create_object(get_obj_index(OBJ_FORGE_HEAUME), 1);
            rleg = create_object(get_obj_index(OBJ_FORGE_CUISS), 1);
            lleg = create_object(get_obj_index(OBJ_FORGE_CUISS), 1);
         }
         if (num <= 2)
            chest = create_object(get_obj_index(OBJ_FORGE_CHAIN_MAIL), 1);
         else if (num <= 5)
            chest = create_object(get_obj_index(OBJ_FORGE_CHAIN_HAUBERK), 1);
         else if (num <= 7)
            chest = create_object(get_obj_index(OBJ_FORGE_RING_MAIL), 1);
         else if (num <= 9)
            chest = create_object(get_obj_index(OBJ_FORGE_DOUBLE_RING_MAIL), 1);
         else if (num <= 11)
            chest = create_object(get_obj_index(OBJ_FORGE_BREASTPLATE), 1);
         else if (num >= 11)
            chest = create_object(get_obj_index(OBJ_FORGE_CUIRASS), 1);
            
         slab1 = get_slab_vnum(lvl, 1);
         slab2 = get_slab_vnum(lvl, 2);
         slab3 = get_slab_vnum(lvl, 3);
         per1 = get_slab_vnum(lvl, 4);
         per2 = get_slab_vnum(lvl, 5);
         per3 = get_slab_vnum(lvl, 6);
                           
         for (firstslab = first_slab; firstslab; firstslab = firstslab->next)
         {
            if (firstslab->vnum == slab1)
               break;
         }
         for (secondslab = first_slab; secondslab; secondslab = secondslab->next)
         {
            if (secondslab->vnum == slab2)
               break;
         }
         if (slab3 > 0)
         {
            for (thirdslab = first_slab; thirdslab; thirdslab = thirdslab->next)
            {
               if (thirdslab->vnum == slab3)
               break;
            }
         }
         if (!firstslab)
         {
            bug("Adjust_Wildermob: %d is not a valid Slab", slab1);
         }
         else if (!secondslab)
         {
            bug("Adjust_Wildermob: %d is not a valid Slab", slab2);
         }
         else if (slab3 > 0 && !thirdslab)
         {
            bug("Adjust_Wildermob: %d is not a valid Slab", slab3);
         }
         else
         {
            int times;
            int lastnum[7];
            
            for (times = 1; times <= 6; times++)
               lastnum[times] = 0;
            
            race = mob->race;
            if (mob->race < 0 || mob->race >= MAX_RACE)
               mob->race = 0; //Needs a valid race   
            times = number_range(1, 2);
            if (qmob && qmob->boss == 1)
               times = number_range(2, 3);
            if (qmob && qmob->boss == 2)
               times = number_range(2, 4);
            for (x = 1; x <= times; x++)
            {
               num = number_range(1, 6);
               
               if (lastnum[num] == 1)
               {
                  x--;
                  continue;
               }
               lastnum[num] = 1;
            }
            if (lastnum[1] == 0)  
               weapon = weapon2 = NULL;
            if (lastnum[2] == 0) 
               rarm = larm = NULL;
            if (lastnum[3] == 0) 
               neck = NULL;
            if (lastnum[4] == 0)  
               head = NULL;
            if (lastnum[5] == 0)  
               rleg = lleg = NULL;
            if (lastnum[6] == 0) 
               chest = NULL;
               
            firstslabobj = create_object(get_obj_index(firstslab->vnum), 1);
            secondslabobj = create_object(get_obj_index(secondslab->vnum), 1);
            if (slab3 > 0)
               thirdslabobj = create_object(get_obj_index(thirdslab->vnum), 1);
            
            if (weapon)
            {   
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, weapon, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, weapon, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, weapon, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, weapon, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, weapon, secondslabobj, secondslab);   
               }
            }	
            if (weapon2)
            {   
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, weapon2, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, weapon2, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, weapon2, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, weapon2, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, weapon2, secondslabobj, secondslab);   
               }
            }	
            if (neck)   
            {
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, neck, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, neck, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, neck, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, neck, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, neck, secondslabobj, secondslab);   
               }
            }
            if (head)   
            {
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, head, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, head, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, head, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, head, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, head, secondslabobj, secondslab);   
               }
            }
            if (rarm)   
            {
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, rarm, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, rarm, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, rarm, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, rarm, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, rarm, secondslabobj, secondslab);   
               }
            }
            if (larm)   
            {
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, larm, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, larm, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, larm, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, larm, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, larm, secondslabobj, secondslab);   
               }
            }
            if (rleg)   
            {
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, rleg, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, rleg, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, rleg, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, rleg, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, rleg, secondslabobj, secondslab);   
               }
            }
            if (lleg)   
            {
               if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, lleg, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, lleg, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, lleg, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, lleg, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, lleg, secondslabobj, secondslab);   
               }
            }
            if (chest)
            {
              if (per3 <= 0)
               {
                  if (number_range(1, 100) <= per1)
                     alter_forge_obj(mob, chest, firstslabobj, firstslab);	
                  else
                     alter_forge_obj(mob, chest, secondslabobj, secondslab);
               }
               else
               {
                  num = number_range(1, 100);
                  if (num <= per1)
                     alter_forge_obj(mob, chest, firstslabobj, firstslab);
                  else if (num > per3)
                     alter_forge_obj(mob, chest, thirdslabobj, thirdslab);
                  else
                     alter_forge_obj(mob, chest, secondslabobj, secondslab);   
               }
            }  
            mob->race = race;
         }       
         if (rleg)
         {
            obj_to_char(rleg, mob);
            equip_char(mob, rleg, WEAR_LEG_R);  
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, rleg);
         } 
         if (lleg)
         {
            obj_to_char(lleg, mob);
            equip_char(mob, lleg, WEAR_LEG_L);   
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, lleg); 
         }
         if (larm)
         {
            obj_to_char(larm, mob);
            equip_char(mob, larm, WEAR_ARM_L); 
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, larm);
         }
         if (rarm)
         {
            obj_to_char(rarm, mob);
            equip_char(mob, rarm, WEAR_ARM_R);   
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, rarm);
         }
         if (chest)
         {
            obj_to_char(chest, mob);
            equip_char(mob, chest, WEAR_BODY);  
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, chest);
         }
         if (head)
         {
            obj_to_char(head, mob);
            equip_char(mob, head, WEAR_HEAD);  
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, head);
         }
         if (neck)
         {
            obj_to_char(neck, mob);
            equip_char(mob, neck, WEAR_NECK);  
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, neck);
         }
         if (weapon)
         {
            obj_to_char(weapon, mob);
            equip_char(mob, weapon, WEAR_WIELD); 
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, weapon);
         }
         if (weapon2)
         {
            obj_to_char(weapon2, mob);
            equip_char(mob, weapon2, WEAR_DUAL_WIELD); 
            if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
               copy_static_area_obj(mob, weapon2);
         }
         if (firstslabobj)
            extract_obj(firstslabobj);
         if (secondslabobj)
            extract_obj(secondslabobj);
         if (thirdslabobj)
            extract_obj(thirdslabobj);
      }
   }
   if (qmob)
      mob->sex = qmob->sex;
   else
      mob->sex = npcrace->sex[diff-1];
   if (area->hi_r_vnum >= START_STATICQUEST_VNUM && mob->race <= 5 && (qmob && qmob->boss > 0))
   {
      STRFREE(mob->name);
      generate_randomname(mob->race, mob->sex, buf);
      if (buf[0] != '\0')
         mob->name = STRALLOC(buf);
      else
      {       
         if (qmob)
            sprintf(buf, "%s", qmob->name);
         else
            sprintf(buf, "%s", add_npc_name(npcrace, diff));
         mob->name = STRALLOC(buf);
      }
   }
   else
   {
      STRFREE(mob->name);
      if (qmob)
         sprintf(buf, "%s", qmob->name);
      else
         sprintf(buf, "%s", add_npc_name(npcrace, diff));
      mob->name = STRALLOC(buf);
   }
   STRFREE(mob->short_descr);
   STRFREE(mob->long_descr);
   mob->short_descr = STRALLOC(buf);
   sprintf(buf, "%s %s\n\r", mob->name, qmob_long_descr[number_range(0, 17)]);
   mob->long_descr = STRALLOC(buf);
   xREMOVE_BIT(mob->act, ACT_PROTOTYPE);
   xSET_BIT(mob->act, ACT_AGGRESSIVE);
   xSET_BIT(mob->affected_by, AFF_INFRARED);
   if (number_range(1, 100) <= 85)
      xSET_BIT(mob->act, ACT_NOWANDER);
   adjust_qmob_flags(mob, qmob, npcrace, diff);
   if (difficulty >= 101)
   {
      if (qmob && qmob->boss == 1)
      {
         mob->gold = (lvl * lvl /2.5) + (lvl*20);   
         if (lvl >= 101)
            mob->gold = 25000 + ((lvl-99)*2000);
         mob->hit = mob->hit *165/100;
         mob->pIndexData->damplus += 5;
         mob->max_hit = mob->hit;
         mob->damaddhi = mob->damaddhi * 125 / 100;
         mob->damaddlow = mob->damaddlow * 125 / 100;
         mob->tohitbash = UMIN(25, mob->tohitbash+1);
         mob->tohitslash = UMIN(25, mob->tohitslash+1);
         mob->tohitstab = UMIN(25, mob->tohitstab+1);
         mob->armor = UMIN(25, mob->armor+1);
         mob->gold *= 2;
      }
      if (qmob && qmob->boss == 2)
      {
         mob->gold = (lvl * lvl /2.5) + (lvl*20);   
         if (lvl >= 101)
            mob->gold = 35000 + ((lvl-99)*2000);
         mob->hit = mob->hit * 245/100;
         mob->pIndexData->damplus += 10;
         mob->damaddhi = mob->damaddhi * 150 / 100;
         mob->damaddlow = mob->damaddlow * 150 / 100;
         mob->max_hit = mob->hit;
         mob->tohitbash = UMIN(25, mob->tohitbash+2);
         mob->tohitslash = UMIN(25, mob->tohitslash+2);
         mob->tohitstab = UMIN(25, mob->tohitstab+2);
         mob->armor = UMIN(25, mob->armor+2);
         mob->gold *= 5;
      }
   }
   else
   {
      if (qmob && qmob->boss == 1)
      {
         mob->hit = mob->hit * 125/100;
         gold = (lvl * lvl /2.5) + (lvl*20);   
         if (lvl >= 101)
            gold = 30000 + ((lvl-99)*2000);
         if (gold > mob->gold)
            mob->gold = gold;
         if (lvl >= 101)
         {
            mob->gold *= 150/100;
            if (mob->gold < 20000)
               mob->gold = 20000;
         }
         else
            mob->gold *= 6;
      }
      if (qmob && qmob->boss == 2)
      {
         mob->hit = mob->hit * 155/100;
         gold = (lvl * lvl /2.5) + (lvl*20);  
         if (lvl >= 101)
            gold = 50000 + ((lvl-99)*2000);
         if (gold > mob->gold)
            mob->gold = gold;
         if (lvl >= 101)
         {
            mob->gold *= 200/100;
            if (mob->gold < 50000)
               mob->gold = 50000;
         }
         else
            mob->gold *= 15;
      }
   }
   if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
      adjust_static_quest_mob(mob);
   return;
}

void link_paf_quest(int loc, int value, OBJ_DATA *obj)
{
   AFFECT_DATA *paf;
   CREATE(paf, AFFECT_DATA, 1);
   paf->type = -1;
   paf->duration = -1;
   paf->location = loc;
   paf->modifier = value;
   xCLEAR_BITS(paf->bitvector);
   paf->next = NULL;
   LINK(paf, obj->first_affect, obj->last_affect, next, prev);
   ++top_affect;
}
      
void adjust_qobj_flags(OBJ_DATA *obj, QOBJ_DATA *qobj)
{
   sh_int loc;
   int value;
   
   if (xIS_SET(qobj->flags, QOBJ_FINGER))
      SET_BIT(obj->wear_flags, ITEM_WEAR_FINGER);
   if (xIS_SET(qobj->flags, QOBJ_NECK))
      SET_BIT(obj->wear_flags, ITEM_WEAR_NECK);
   if (xIS_SET(qobj->flags, QOBJ_BODY))
      SET_BIT(obj->wear_flags, ITEM_WEAR_BODY);
   if (xIS_SET(qobj->flags, QOBJ_HEAD))
      SET_BIT(obj->wear_flags, ITEM_WEAR_HEAD);
   if (xIS_SET(qobj->flags, QOBJ_LEGS))
      SET_BIT(obj->wear_flags, ITEM_WEAR_LEGS);
   if (xIS_SET(qobj->flags, QOBJ_ARMS))
      SET_BIT(obj->wear_flags, ITEM_WEAR_ARMS);
   if (xIS_SET(qobj->flags, QOBJ_WSHIELD))
      SET_BIT(obj->wear_flags, ITEM_WEAR_SHIELD);
   if (xIS_SET(qobj->flags, QOBJ_ANECK))
      SET_BIT(obj->wear_flags, ITEM_WEAR_ABOUT_NECK);
   if (xIS_SET(qobj->flags, QOBJ_WAIST))
      SET_BIT(obj->wear_flags, ITEM_WEAR_WAIST);
   if (xIS_SET(qobj->flags2, QOBJ_BACK))
      SET_BIT(obj->wear_flags, ITEM_WEAR_BACK);
   if (xIS_SET(qobj->flags, QOBJ_WIELD))
      SET_BIT(obj->wear_flags, ITEM_WIELD);
   if (xIS_SET(qobj->flags, QOBJ_MAGIC))   
      xSET_BIT(obj->extra_flags, ITEM_MAGIC);
   if (xIS_SET(qobj->flags, QOBJ_NODROP))   
      xSET_BIT(obj->extra_flags, ITEM_NODROP);
   if (xIS_SET(qobj->flags, QOBJ_BLESS))   
      xSET_BIT(obj->extra_flags, ITEM_BLESS);
   if (xIS_SET(qobj->flags, QOBJ_INVENTORY))   
      xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
   if (xIS_SET(qobj->flags, QOBJ_ORGANIC))   
      xSET_BIT(obj->extra_flags, ITEM_ORGANIC);
   if (xIS_SET(qobj->flags, QOBJ_METAL))   
      xSET_BIT(obj->extra_flags, ITEM_METAL);
   if (xIS_SET(qobj->flags, QOBJ_POISONED))   
      xSET_BIT(obj->extra_flags, ITEM_POISONED);
   if (xIS_SET(qobj->flags, QOBJ_DEATHROT))   
      xSET_BIT(obj->extra_flags, ITEM_DEATHROT);
   if (xIS_SET(qobj->flags, QOBJ_NOLOCATE))   
      xSET_BIT(obj->extra_flags, ITEM_NOLOCATE);
   if (xIS_SET(qobj->flags, QOBJ_GROUNDROT))   
      xSET_BIT(obj->extra_flags, ITEM_GROUNDROT);
   if (xIS_SET(qobj->flags, QOBJ_NOGIVE))   
      xSET_BIT(obj->extra_flags, ITEM_NOGIVE);
   if (xIS_SET(qobj->flags, QOBJ_NODISARM))   
      xSET_BIT(obj->extra_flags, ITEM_NODISARM);
   if (xIS_SET(qobj->flags, QOBJ_NOBREAK))   
      xSET_BIT(obj->extra_flags, ITEM_NOBREAK);
   if (xIS_SET(qobj->flags, QOBJ_ARTIFACT))   
      xSET_BIT(obj->extra_flags, ITEM_ARTIFACT);   
   if (xIS_SET(qobj->flags, QOBJ_TWOHANDED))   
      xSET_BIT(obj->extra_flags, ITEM_TWOHANDED);
   if (xIS_SET(qobj->flags, QOBJ_IMBUABLE))   
      xSET_BIT(obj->extra_flags, ITEM_IMBUABLE);
   if (xIS_SET(qobj->flags, QOBJ_GEM))   
      xSET_BIT(obj->extra_flags, ITEM_GEM);
   if (xIS_SET(qobj->flags, QOBJ_GEM_SETTING))   
      xSET_BIT(obj->extra_flags, ITEM_GEM_SETTING);
   if (xIS_SET(qobj->flags, QOBJ_SANCTIFIED))   
      xSET_BIT(obj->extra_flags, ITEM_SANCTIFIED);
   if (xIS_SET(qobj->flags, QOBJ_CLOAK))   
      xSET_BIT(obj->extra_flags, ITEM_CLOAK);
   if (xIS_SET(qobj->flags, QOBJ_UNIQUE))   
      xSET_BIT(obj->extra_flags, ITEM_UNIQUE);
   if (xIS_SET(qobj->flags, QOBJ_STR))
   {
      loc = APPLY_STR;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_STR2))
   {
      loc = APPLY_STR;
      value = 2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_STR3))
   {
      loc = APPLY_STR;
      value = 3;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LSTR))
   {
      loc = APPLY_STR;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LSTR2))
   {
      loc = APPLY_STR;
      value = -2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DEX))
   {
      loc = APPLY_DEX;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DEX2))
   {
      loc = APPLY_DEX;
      value = 2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DEX3))
   {
      loc = APPLY_DEX;
      value = 3;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LDEX))
   {
      loc = APPLY_DEX;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LDEX2))
   {
      loc = APPLY_DEX;
      value = -2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_INT))
   {
      loc = APPLY_INT;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_INT2))
   {
      loc = APPLY_INT;
      value = 2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_INT3))
   {
      loc = APPLY_INT;
      value = 3;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LINT))
   {
      loc = APPLY_INT;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LINT2))
   {
      loc = APPLY_INT;
      value = -2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_WIS))
   {
      loc = APPLY_WIS;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_WIS2))
   {
      loc = APPLY_WIS;
      value = 2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_WIS3))
   {
      loc = APPLY_WIS;
      value = 3;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LWIS))
   {
      loc = APPLY_WIS;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LWIS2))
   {
      loc = APPLY_WIS;
      value = -2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_CON))
   {
      loc = APPLY_CON;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_CON2))
   {
      loc = APPLY_CON;
      value = 2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_CON3))
   {
      loc = APPLY_CON;
      value = 3;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LCON))
   {
      loc = APPLY_CON;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LCON2))
   {
      loc = APPLY_CON;
      value = -2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_HP))
   {
      loc = APPLY_HIT;
      value = number_range(1, 3);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_HP2))
   {
      loc = APPLY_HIT;
      value = number_range(5, 8);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_HP3))
   {
      loc = APPLY_HIT;
      value = number_range(8, 15);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_HP4))
   {
      loc = APPLY_HIT;
      value = number_range(15, 25);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_HP5))
   {
      loc = APPLY_HIT;
      value = number_range(25, 40);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_HP6))
   {
      loc = APPLY_HIT;
      value = number_range(40, 60);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_HP7))
   {
      loc = APPLY_HIT;
      value = number_range(60, 80);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LHP))
   {
      loc = APPLY_HIT;
      value = number_range(1, 2);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LHP2))
   {
      loc = APPLY_HIT;
      value = number_range(3, 5);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LHP3))
   {
      loc = APPLY_HIT;
      value = number_range(5, 10);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_MP))
   {
      loc = APPLY_MANA;
      value = number_range(1, 3);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_MP2))
   {
      loc = APPLY_MANA;
      value = number_range(5, 8);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_MP3))
   {
      loc = APPLY_MANA;
      value = number_range(8, 15);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_MP4))
   {
      loc = APPLY_MANA;
      value = number_range(15, 25);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_MP5))
   {
      loc = APPLY_MANA;
      value = number_range(25, 40);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_MP6))
   {
      loc = APPLY_MANA;
      value = number_range(40, 60);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_MP7))
   {
      loc = APPLY_MANA;
      value = number_range(60, 80);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LMP))
   {
      loc = APPLY_MANA;
      value = number_range(1, 2);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LMP2))
   {
      loc = APPLY_MANA;
      value = number_range(3, 5);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LMP3))
   {
      loc = APPLY_MANA;
      value = number_range(5, 10);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DAM))
   {
      loc = APPLY_SANCTIFY;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DAM2))
   {
      loc = APPLY_SANCTIFY;
      value = number_range(2, 3);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DAM3))
   {
      loc = APPLY_SANCTIFY;
      value = number_range(4, 5);            
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DAM4))
   {
      loc = APPLY_SANCTIFY;
      value = number_range(6, 8);        
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_DAM5))
   {
      loc = APPLY_SANCTIFY;
      value = number_range(9, 12);        
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LDAM))
   {
      loc = APPLY_SANCTIFY;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LDAM2))
   {
      loc = APPLY_SANCTIFY;
      value = number_range(-3, -2);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_TOHIT))
   {
      loc = APPLY_TOHIT;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_TOHIT2))
   {
      loc = APPLY_TOHIT;
      value = 2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_TOHIT3))
   {
      loc = APPLY_TOHIT;
      value = number_range(3, 4);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LTOHIT))
   {
      loc = APPLY_TOHIT;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LTOHIT2))
   {
      loc = APPLY_TOHIT;
      value = -2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SSKIN))
   {
      loc = APPLY_STONE;
      value = 1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SSKIN2))
   {
      loc = APPLY_STONE;
      value = number_range(1, 2);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SSKIN3))
   {
      loc = APPLY_STONE;
      value = 2;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SSKIN4))
   {
      loc = APPLY_STONE;
      value = number_range(2, 3);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SSKIN5))
   {
      loc = APPLY_STONE;
      value = number_range(4, 5);      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LSSKIN))
   {
      loc = APPLY_STONE;
      value = -1;      
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LSSKIN2))
   {
      loc = APPLY_STONE;
      value = number_range(-1, -2);      
      link_paf_quest(loc, value, obj);
   }
   
   if (xIS_SET(qobj->flags2, QOBJ_HPGEN))
      link_paf_quest(APPLY_HPTICK, number_range(125, 150), obj);
   if (xIS_SET(qobj->flags2, QOBJ_HPGEN2))
      link_paf_quest(APPLY_HPTICK, number_range(175, 200), obj);
   if (xIS_SET(qobj->flags2, QOBJ_HPGEN3))
      link_paf_quest(APPLY_HPTICK, number_range(225, 250), obj);
   if (xIS_SET(qobj->flags2, QOBJ_HPGEN4))
      link_paf_quest(APPLY_HPTICK, number_range(300, 375), obj);
   if (xIS_SET(qobj->flags2, QOBJ_HPGEN5))
      link_paf_quest(APPLY_HPTICK, number_range(400, 500), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LHPGEN))
      link_paf_quest(APPLY_HPTICK, number_range(90, 95), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LHPGEN2))
      link_paf_quest(APPLY_HPTICK, number_range(75, 85), obj);
   if (xIS_SET(qobj->flags2, QOBJ_MPGEN))
      link_paf_quest(APPLY_MANATICK, number_range(125, 150), obj);
   if (xIS_SET(qobj->flags2, QOBJ_MPGEN2))
      link_paf_quest(APPLY_MANATICK, number_range(175, 200), obj);
   if (xIS_SET(qobj->flags2, QOBJ_MPGEN3))
      link_paf_quest(APPLY_MANATICK, number_range(225, 250), obj);
   if (xIS_SET(qobj->flags2, QOBJ_MPGEN4))
      link_paf_quest(APPLY_MANATICK, number_range(300, 375), obj);
   if (xIS_SET(qobj->flags2, QOBJ_MPGEN5))
      link_paf_quest(APPLY_MANATICK, number_range(400, 500), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LMPGEN))
      link_paf_quest(APPLY_MANATICK, number_range(90, 95), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LMPGEN2))
      link_paf_quest(APPLY_MANATICK, number_range(75, 85), obj);
   if (xIS_SET(qobj->flags2, QOBJ_SHIELD))
      link_paf_quest(APPLY_SHIELD, number_range(1, 2), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_SHIELD2))
      link_paf_quest(APPLY_SHIELD, number_range(3, 5), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_SHIELD3))
      link_paf_quest(APPLY_SHIELD, number_range(7, 10), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_SHIELD4))
      link_paf_quest(APPLY_SHIELD, number_range(12, 15), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_SHIELD5))
      link_paf_quest(APPLY_SHIELD, number_range(20, 25), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_LSHIELD))
      link_paf_quest(APPLY_SHIELD, number_range(-1, -2), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_LSHIELD2))
      link_paf_quest(APPLY_SHIELD, number_range(-3, -5), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_AGI))
      link_paf_quest(APPLY_AGI, number_range(1, 2), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_AGI2))
      link_paf_quest(APPLY_AGI, number_range(3, 4), obj); 
   if (xIS_SET(qobj->flags2, QOBJ_AGI3))
      link_paf_quest(APPLY_AGI, number_range(5, 7), obj); 
   if (xIS_SET(qobj->flags2, QOBJ_AGI4))
      link_paf_quest(APPLY_AGI, number_range(8, 10), obj); 
   if (xIS_SET(qobj->flags2, QOBJ_AGI5))
      link_paf_quest(APPLY_AGI, number_range(12, 15), obj); 
   if (xIS_SET(qobj->flags2, QOBJ_LAGI))
      link_paf_quest(APPLY_AGI, number_range(-1, -2), obj);  
   if (xIS_SET(qobj->flags2, QOBJ_LAGI2))
      link_paf_quest(APPLY_AGI, number_range(-3, -4), obj); 
   if (xIS_SET(qobj->flags2, QOBJ_LAGI3))
      link_paf_quest(APPLY_AGI, number_range(-5, -8), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LCK))
      link_paf_quest(APPLY_LCK, 1, obj);  
   if (xIS_SET(qobj->flags2, QOBJ_LCK2))
      link_paf_quest(APPLY_LCK, 2, obj);  
   if (xIS_SET(qobj->flags2, QOBJ_LCK3))
      link_paf_quest(APPLY_LCK, 3, obj);  
   if (xIS_SET(qobj->flags2, QOBJ_LLCK))
      link_paf_quest(APPLY_LCK, -1, obj);  
   if (xIS_SET(qobj->flags2, QOBJ_LLCK2))
      link_paf_quest(APPLY_LCK, -2, obj);  
   if (xIS_SET(qobj->flags2, QOBJ_WMOD))
      link_paf_quest(APPLY_WMOD, number_range(90, 95), obj);
   if (xIS_SET(qobj->flags2, QOBJ_WMOD2))
      link_paf_quest(APPLY_WMOD, number_range(80, 85), obj);
   if (xIS_SET(qobj->flags2, QOBJ_WMOD3))
      link_paf_quest(APPLY_WMOD, number_range(70, 75), obj);
   if (xIS_SET(qobj->flags2, QOBJ_WMOD4))
      link_paf_quest(APPLY_WMOD, number_range(60, 65), obj);
   if (xIS_SET(qobj->flags2, QOBJ_WMOD5))
      link_paf_quest(APPLY_WMOD, number_range(40, 50), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LWMOD))
      link_paf_quest(APPLY_WMOD, number_range(105, 110), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LWMOD2))
      link_paf_quest(APPLY_WMOD, number_range(120, 125), obj);
   if (xIS_SET(qobj->flags2, QOBJ_LWMOD3))
      link_paf_quest(APPLY_WMOD, number_range(135, 150), obj);   
   if (xIS_SET(qobj->flags2, QOBJ_STAT))
   {
      int x = number_range(1, 9);
      
      ochange = 1;
      if (x == 1)
         link_paf_quest(APPLY_STR, 1, obj);
      if (x == 2)
         link_paf_quest(APPLY_CON, 1, obj);
      if (x == 3)
         link_paf_quest(APPLY_DEX, 1, obj);
      if (x == 4)
         link_paf_quest(APPLY_WIS, 1, obj);
      if (x == 5)
         link_paf_quest(APPLY_INT, 1, obj);
      if (x == 6)
         link_paf_quest(APPLY_LCK, 1, obj);
      if (x == 7)
         link_paf_quest(APPLY_AGI, number_range(1, 2), obj);
      if (x == 8)
         link_paf_quest(APPLY_HIT, number_range(1, 3), obj);
      if (x == 9)
         link_paf_quest(APPLY_MANA, number_range(1, 3), obj);
   }
   if (xIS_SET(qobj->flags2, QOBJ_STAT2))
   {
      int x;
      int cnt = 1;
      ochange = 1;
      
      for (;;)
      {
         x = number_range(1, 9);
         if (x == 1)
            link_paf_quest(APPLY_STR, 1, obj);
         if (x == 2)
            link_paf_quest(APPLY_CON, 1, obj);
         if (x == 3)
            link_paf_quest(APPLY_DEX, 1, obj);
         if (x == 4)
            link_paf_quest(APPLY_WIS, 1, obj);
         if (x == 5)
            link_paf_quest(APPLY_INT, 1, obj);
         if (x == 6)
            link_paf_quest(APPLY_LCK, 1, obj);
         if (x == 7)
            link_paf_quest(APPLY_AGI, number_range(1, 2), obj);
         if (x == 8)
            link_paf_quest(APPLY_HIT, number_range(1, 3), obj);
         if (x == 9)
            link_paf_quest(APPLY_MANA, number_range(1, 3), obj);
         if (cnt < 2)
         {
            if (number_range(1, 5) <= 2)
               cnt++;
            else
               break;
         }
         else
            break;
      }
   }
   if (xIS_SET(qobj->flags2, QOBJ_STAT3))
   {
      int x;
      int cnt = 1;
      
      ochange = 1;
      
      for (;;)
      {
         x = number_range(1, 9);
         if (x == 1)
            link_paf_quest(APPLY_STR, number_range(1, 2), obj);
         if (x == 2)   
            link_paf_quest(APPLY_CON, number_range(1, 2), obj);   
         if (x == 3)
            link_paf_quest(APPLY_DEX, number_range(1, 2), obj);
         if (x == 4)
            link_paf_quest(APPLY_WIS, number_range(1, 2), obj);
         if (x == 5)
            link_paf_quest(APPLY_INT, number_range(1, 2), obj);
         if (x == 6)
            link_paf_quest(APPLY_LCK, number_range(1, 2), obj);
         if (x == 7)
            link_paf_quest(APPLY_AGI, number_range(2, 3), obj);
         if (x == 8)
            link_paf_quest(APPLY_HIT, number_range(2, 5), obj);
         if (x == 9)
            link_paf_quest(APPLY_MANA, number_range(2, 5), obj);
         if (cnt < 2)
         {
            if (number_range(1, 10) <= 6)
               cnt++;
            else
               break;
         }
         else 
            break;
      }            
   } 
   if (xIS_SET(qobj->flags2, QOBJ_STAT4))
   {
      int x;
      int cnt = 1;
      
      ochange = 1;
      
      for (;;)
      {
         x = number_range(1, 9);
         if (x == 1) 
            link_paf_quest(APPLY_STR, 2, obj);
         if (x == 2)   
            link_paf_quest(APPLY_CON, 2, obj);
         if (x == 3)
            link_paf_quest(APPLY_DEX, 2, obj);
         if (x == 4)
            link_paf_quest(APPLY_WIS, 2, obj);
         if (x == 5)
            link_paf_quest(APPLY_INT, 2, obj);
         if (x == 6)
            link_paf_quest(APPLY_LCK, 2, obj);
         if (x == 7)
            link_paf_quest(APPLY_AGI, number_range(5, 7), obj);
         if (x == 8)
            link_paf_quest(APPLY_HIT, number_range(8, 12), obj);
         if (x == 9)
            link_paf_quest(APPLY_MANA, number_range(8, 12), obj);
         if (cnt > 1)
         {
            if (number_range(1, 10) <= 8)
                break;
         }
         else
            cnt++;
      }
   } 
   if (xIS_SET(qobj->flags2, QOBJ_STAT5))
   {
      int x;
      int cnt = 1;
      
      ochange = 1;
      for (;;)
      {
         x = number_range(1, 9);
      
         if (x == 1)
            link_paf_quest(APPLY_STR, 2, obj);
         if (x == 2)
            link_paf_quest(APPLY_CON, 2, obj);
         if (x == 3)
            link_paf_quest(APPLY_DEX, 2, obj);
         if (x == 4)
            link_paf_quest(APPLY_WIS, 2, obj);
         if (x == 5)
            link_paf_quest(APPLY_INT, 2, obj);
         if (x == 6)
            link_paf_quest(APPLY_LCK, 2, obj);
         if (x == 7)
            link_paf_quest(APPLY_AGI, number_range(8, 10), obj);
         if (x == 8)
            link_paf_quest(APPLY_HIT, number_range(15, 20), obj);
         if (x == 9)
            link_paf_quest(APPLY_MANA, number_range(15, 20), obj);
         if (cnt > 1)
         {
            if (number_range(1, 10) <= 6)
               break;
         }
         cnt++;
      }
   }
   if (xIS_SET(qobj->flags2, QOBJ_LSTAT))
   {
      int x = number_range(1, 9);
      
      ochange = 2;
      if (x == 1)
         link_paf_quest(APPLY_STR, -1, obj);
      if (x == 2)
         link_paf_quest(APPLY_CON, -1, obj);
      if (x == 3)
         link_paf_quest(APPLY_DEX, -1, obj);
      if (x == 4)
         link_paf_quest(APPLY_WIS, -1, obj);
      if (x == 5)
         link_paf_quest(APPLY_INT, -1, obj);
      if (x == 6)
         link_paf_quest(APPLY_LCK, -1, obj);
      if (x == 7)
         link_paf_quest(APPLY_AGI, number_range(-1, -2), obj);
      if (x == 8)
         link_paf_quest(APPLY_HIT, number_range(-1, -3), obj);
      if (x == 9)
         link_paf_quest(APPLY_MANA, number_range(-1, -3), obj);
   }
   if (xIS_SET(qobj->flags2, QOBJ_LSTAT2))
   {
      int x = number_range(1, 9);
      
      ochange = 2;
      if (x == 1)
         link_paf_quest(APPLY_STR, -1, obj);
      if (x == 2)
         link_paf_quest(APPLY_CON, -1, obj);
      if (x == 3)
         link_paf_quest(APPLY_DEX, -1, obj);
      if (x == 4)
         link_paf_quest(APPLY_WIS, -1, obj);
      if (x == 5)
         link_paf_quest(APPLY_INT, -1, obj);
      if (x == 6)
         link_paf_quest(APPLY_LCK, -1, obj);
      if (x == 7)
         link_paf_quest(APPLY_AGI, number_range(-2, -3), obj);
      if (x == 8)
         link_paf_quest(APPLY_HIT, number_range(-2, -5), obj);
      if (x == 9)
         link_paf_quest(APPLY_MANA, number_range(-2, -5), obj);
   }    
   if (xIS_SET(qobj->flags2, QOBJ_BATTLE))
   {
      int x = number_range(1, 7);
      
      ochange = 1;
      if (x == 1)
         link_paf_quest(APPLY_SHIELD, number_range(1, 2), obj);     
      if (x == 2)
         link_paf_quest(APPLY_MANATICK, number_range(125, 150), obj);
      if (x == 3)
         link_paf_quest(APPLY_HPTICK, number_range(125, 150), obj);
      if (x == 4)
         link_paf_quest(APPLY_STONE, 1, obj);
      if (x == 5)
         link_paf_quest(APPLY_TOHIT, 1, obj);
      if (x == 8)
         link_paf_quest(APPLY_ARMOR, 1, obj);
      if (x == 6)
         link_paf_quest(APPLY_SANCTIFY, 1, obj);
      if (x == 7)
      {
         link_paf_quest(APPLY_SAVING_POISON, -1, obj);
         link_paf_quest(APPLY_SAVING_ROD, -1, obj);
         link_paf_quest(APPLY_SAVING_PARA, -1, obj);
         link_paf_quest(APPLY_SAVING_BREATH, -1, obj);
         link_paf_quest(APPLY_SAVING_SPELL, -1, obj);
      }
   }
   if (xIS_SET(qobj->flags2, QOBJ_BATTLE2))
   {
      int x;
      int cnt = 1;
      ochange = 1;
      
      for (;;)
      {
         x = number_range(1, 7);
         if (x == 1)
            link_paf_quest(APPLY_SHIELD, number_range(3, 4), obj);     
         if (x == 2)
            link_paf_quest(APPLY_MANATICK, number_range(150, 175), obj);
         if (x == 3)
            link_paf_quest(APPLY_HPTICK, number_range(150, 175), obj);
         if (x == 4)
            link_paf_quest(APPLY_STONE, number_range(1, 2), obj);
         if (x == 5)
            link_paf_quest(APPLY_TOHIT, number_range(1, 2), obj);
         if (x == 8)
            link_paf_quest(APPLY_ARMOR, number_range(1, 2), obj);
         if (x == 6)
            link_paf_quest(APPLY_SANCTIFY, 2, obj);
         if (x == 7)
         {
            link_paf_quest(APPLY_SAVING_POISON, -2, obj);
            link_paf_quest(APPLY_SAVING_ROD, -2, obj);
            link_paf_quest(APPLY_SAVING_PARA, -2, obj);
            link_paf_quest(APPLY_SAVING_BREATH, -2, obj);
            link_paf_quest(APPLY_SAVING_SPELL, -2, obj);
         }
         if (cnt < 2)
         {
            if (number_range(1, 10) <= 3)
               cnt++;
            else
               break;
         }
         else
            break;
      }
   }
   if (xIS_SET(qobj->flags2, QOBJ_BATTLE3))
   {
      int x;
      int cnt = 1;
      
      ochange = 1;
      for (;;)
      {
         x = number_range(1, 7);
         if (x == 1)
            link_paf_quest(APPLY_SHIELD, number_range(5, 8), obj);     
         if (x == 2)
            link_paf_quest(APPLY_MANATICK, number_range(200, 225), obj);
         if (x == 3)
            link_paf_quest(APPLY_HPTICK, number_range(200, 225), obj);
         if (x == 4)
            link_paf_quest(APPLY_STONE, 2, obj);
         if (x == 5)
            link_paf_quest(APPLY_TOHIT, 2, obj);
         if (x == 8)
            link_paf_quest(APPLY_ARMOR, 2, obj);
         if (x == 6)
            link_paf_quest(APPLY_SANCTIFY, number_range(3, 4), obj);
         if (x == 7)
         {
            link_paf_quest(APPLY_SAVING_POISON, number_range(-3, -4), obj);
            link_paf_quest(APPLY_SAVING_ROD, number_range(-3, -4), obj);
            link_paf_quest(APPLY_SAVING_PARA, number_range(-3, -4), obj);
            link_paf_quest(APPLY_SAVING_BREATH, number_range(-3, -4), obj);
            link_paf_quest(APPLY_SAVING_SPELL, number_range(-3, -4), obj);
         }
         if (cnt > 1)
         {
            if (number_range(1, 10) <= 8)
               break;
         }
         else
            cnt++;
      }
   }  
   if (xIS_SET(qobj->flags2, QOBJ_BATTLE4))
   {
      int x;
      int cnt = 1;
      ochange = 1;
      
      for (;;)
      {
         x = number_range(1, 7);
         if (x == 1)
            link_paf_quest(APPLY_SHIELD, number_range(10, 12), obj);     
         if (x == 2)
            link_paf_quest(APPLY_MANATICK, number_range(250, 300), obj);
         if (x == 3)
            link_paf_quest(APPLY_HPTICK, number_range(250, 300), obj);
         if (x == 4)
            link_paf_quest(APPLY_STONE, number_range(2, 3), obj);
         if (x == 5)
            link_paf_quest(APPLY_TOHIT, number_range(2, 3), obj);
         if (x == 8)
            link_paf_quest(APPLY_ARMOR, number_range(2, 3), obj);
         if (x == 6)
            link_paf_quest(APPLY_SANCTIFY, number_range(5, 8), obj);
         if (x == 7)
         {
            link_paf_quest(APPLY_SAVING_POISON, number_range(-5, -8), obj);
            link_paf_quest(APPLY_SAVING_ROD, number_range(-5, -8), obj);
            link_paf_quest(APPLY_SAVING_PARA, number_range(-5, -8), obj);
            link_paf_quest(APPLY_SAVING_BREATH, number_range(-5, -8), obj);
            link_paf_quest(APPLY_SAVING_SPELL, number_range(-5, -8), obj);
         }
         if (cnt > 1)
         {
            if (number_range(1, 10) <= 6)
               break;
         }
         else
            cnt++;
      }
   }
   if (xIS_SET(qobj->flags2, QOBJ_LBATTLE))
   {
      int x = number_range(1, 7);
      
      ochange = 2;
      if (x == 1)
         link_paf_quest(APPLY_SHIELD, number_range(-1, -2), obj);     
      if (x == 2)
         link_paf_quest(APPLY_MANATICK, number_range(90, 96), obj);
      if (x == 3)
         link_paf_quest(APPLY_HPTICK, number_range(90, 95), obj);
      if (x == 4)
         link_paf_quest(APPLY_STONE, -1, obj);
      if (x == 5)
         link_paf_quest(APPLY_TOHIT, -1, obj);
      if (x == 8)
         link_paf_quest(APPLY_ARMOR, -1, obj);
      if (x == 6)
         link_paf_quest(APPLY_SANCTIFY, -1, obj);
      if (x == 7)
      {
         link_paf_quest(APPLY_SAVING_POISON, 1, obj);
         link_paf_quest(APPLY_SAVING_ROD, 1, obj);
         link_paf_quest(APPLY_SAVING_PARA, 1, obj);
         link_paf_quest(APPLY_SAVING_BREATH, 1, obj);
         link_paf_quest(APPLY_SAVING_SPELL, 1, obj);
      }
   }
   if (xIS_SET(qobj->flags2, QOBJ_LBATTLE2))
   {
      int x = number_range(1, 7);
      
      ochange = 2;
      if (x == 1)
         link_paf_quest(APPLY_SHIELD, number_range(-3, -4), obj);     
      if (x == 2)
         link_paf_quest(APPLY_MANATICK, number_range(80, 85), obj);
      if (x == 3)
         link_paf_quest(APPLY_HPTICK, number_range(80, 85), obj);
      if (x == 4)
         link_paf_quest(APPLY_STONE, -2, obj);
      if (x == 5)
         link_paf_quest(APPLY_TOHIT, -2, obj);
      if (x == 8)
         link_paf_quest(APPLY_ARMOR, -2, obj);
      if (x == 6)
         link_paf_quest(APPLY_SANCTIFY, -2, obj);
      if (x == 7)
      {
         link_paf_quest(APPLY_SAVING_POISON, 2, obj);
         link_paf_quest(APPLY_SAVING_ROD, 2, obj);
         link_paf_quest(APPLY_SAVING_PARA, 2, obj);
         link_paf_quest(APPLY_SAVING_BREATH, 2, obj);
         link_paf_quest(APPLY_SAVING_SPELL, 2, obj);
      }
   }
      
   if (xIS_SET(qobj->flags, QOBJ_SAVE))
   {
      loc = APPLY_SAVING_POISON;
      value = -1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = -1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = -1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = -1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = -1;
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SAVE2))
   {
      loc = APPLY_SAVING_POISON;
      value = number_range(-2, -3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = number_range(-2, -3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = number_range(-2, -3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = number_range(-2, -3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = number_range(-2, -3);
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SAVE3))
   {
      loc = APPLY_SAVING_POISON;
      value = number_range(-4, -5);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = number_range(-4, -5);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = number_range(-4, -5);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = number_range(-4, -5);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = number_range(-4, -5);
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SAVE4))
   {
      loc = APPLY_SAVING_POISON;
      value = number_range(-6, -8);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = number_range(-6, -8);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = number_range(-6, -8);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = number_range(-6, -8);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = number_range(-6, -8);
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_SAVE5))
   {
      loc = APPLY_SAVING_POISON;
      value = number_range(-9, -12);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = number_range(-9, -12);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = number_range(-9, -12);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = number_range(-9, -12);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = number_range(-9, -12);
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LSAVE))
   {
      loc = APPLY_SAVING_POISON;
      value = 1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = 1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = 1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = 1;
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = 1;
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LSAVE2))
   {
      loc = APPLY_SAVING_POISON;
      value = number_range(2, 3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = number_range(2, 3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = number_range(2, 3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = number_range(2, 3);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = number_range(2, 3);
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_LSAVE3))
   {
      loc = APPLY_SAVING_POISON;
      value = number_range(4, 6);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_ROD;
      value = number_range(4, 6);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_PARA;
      value = number_range(4, 6);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_BREATH;
      value = number_range(4, 6);
      link_paf_quest(loc, value, obj);
      loc = APPLY_SAVING_SPELL;
      value = number_range(4, 6);
      link_paf_quest(loc, value, obj);
   }
   if (xIS_SET(qobj->flags, QOBJ_RBLUNT))
      link_paf_quest(APPLY_RESISTANT, RIS_BLUNT, obj);
   if (xIS_SET(qobj->flags, QOBJ_RSLASH))
      link_paf_quest(APPLY_RESISTANT, RIS_SLASH, obj);
   if (xIS_SET(qobj->flags, QOBJ_RPIERCE))
      link_paf_quest(APPLY_RESISTANT, RIS_PIERCE, obj);
   if (xIS_SET(qobj->flags, QOBJ_RFIRE))
      link_paf_quest(APPLY_RESISTANT, RIS_FIRE, obj);
   if (xIS_SET(qobj->flags, QOBJ_RCOLD))
      link_paf_quest(APPLY_RESISTANT, RIS_WATER, obj);
   if (xIS_SET(qobj->flags, QOBJ_RELECT))
      link_paf_quest(APPLY_RESISTANT, RIS_EARTH, obj);
   if (xIS_SET(qobj->flags, QOBJ_RENERGY))
      link_paf_quest(APPLY_RESISTANT, RIS_ENERGY, obj);
   if (xIS_SET(qobj->flags, QOBJ_RAIR))
      link_paf_quest(APPLY_RESISTANT, RIS_AIR, obj);
   if (xIS_SET(qobj->flags, QOBJ_RNONMAGIC))
      link_paf_quest(APPLY_RESISTANT, RIS_NONMAGIC, obj);
   if (xIS_SET(qobj->flags, QOBJ_RMAGIC))
      link_paf_quest(APPLY_RESISTANT, RIS_MAGIC, obj);
      
   if (xIS_SET(qobj->flags, QOBJ_SBLUNT))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_BLUNT, obj);
   if (xIS_SET(qobj->flags, QOBJ_SSLASH))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_SLASH, obj);
   if (xIS_SET(qobj->flags, QOBJ_SPIERCE))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_PIERCE, obj);
   if (xIS_SET(qobj->flags, QOBJ_SFIRE))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_FIRE, obj);
   if (xIS_SET(qobj->flags, QOBJ_SCOLD))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_WATER, obj);
   if (xIS_SET(qobj->flags, QOBJ_SELECT))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_EARTH, obj);
   if (xIS_SET(qobj->flags, QOBJ_SENERGY))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_ENERGY, obj);
   if (xIS_SET(qobj->flags, QOBJ_SAIR))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_AIR, obj);
   if (xIS_SET(qobj->flags, QOBJ_SNONMAGIC))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_NONMAGIC, obj);
   if (xIS_SET(qobj->flags, QOBJ_SMAGIC))
      link_paf_quest(APPLY_SUSCEPTIBLE, RIS_MAGIC, obj);
      
   if (xIS_SET(qobj->flags, QOBJ_IBLUNT))
      link_paf_quest(APPLY_IMMUNE, RIS_BLUNT, obj);
   if (xIS_SET(qobj->flags, QOBJ_ISLASH))
      link_paf_quest(APPLY_IMMUNE, RIS_SLASH, obj);
   if (xIS_SET(qobj->flags, QOBJ_IPIERCE))
      link_paf_quest(APPLY_IMMUNE, RIS_PIERCE, obj);
   if (xIS_SET(qobj->flags, QOBJ_IFIRE))
      link_paf_quest(APPLY_IMMUNE, RIS_FIRE, obj);
   if (xIS_SET(qobj->flags, QOBJ_ICOLD))
      link_paf_quest(APPLY_IMMUNE, RIS_WATER, obj);
   if (xIS_SET(qobj->flags2, QOBJ_IELECT))
      link_paf_quest(APPLY_IMMUNE, RIS_EARTH, obj);
   if (xIS_SET(qobj->flags2, QOBJ_IENERGY))
      link_paf_quest(APPLY_IMMUNE, RIS_ENERGY, obj);
   if (xIS_SET(qobj->flags2, QOBJ_IAIR))
      link_paf_quest(APPLY_IMMUNE, RIS_AIR, obj);
   if (xIS_SET(qobj->flags2, QOBJ_INONMAGIC))
      link_paf_quest(APPLY_IMMUNE, RIS_NONMAGIC, obj);
   if (xIS_SET(qobj->flags2, QOBJ_IMAGIC))
      link_paf_quest(APPLY_IMMUNE, RIS_MAGIC, obj);
   if (xIS_SET(qobj->flags2, QOBJ_INVISIBLE))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_INVISIBLE, obj);
   if (xIS_SET(qobj->flags2, QOBJ_DETECT_INVIS))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_DETECT_INVIS, obj);
   if (xIS_SET(qobj->flags2, QOBJ_DETECT_MAGIC))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_DETECT_MAGIC, obj);
   if (xIS_SET(qobj->flags2, QOBJ_DETECT_HIDDEN))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_DETECT_HIDDEN, obj);
   if (xIS_SET(qobj->flags2, QOBJ_SANCTUARY))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_SANCTUARY, obj);
   if (xIS_SET(qobj->flags2, QOBJ_FLYING))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_FLYING, obj);
   if (xIS_SET(qobj->flags2, QOBJ_PASS_DOOR))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_PASS_DOOR, obj);
   if (xIS_SET(qobj->flags2, QOBJ_FLOATING))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_FLOATING, obj);
   if (xIS_SET(qobj->flags2, QOBJ_TRUESIGHT))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_TRUESIGHT, obj);
   if (xIS_SET(qobj->flags2, QOBJ_DETECT_TRAPS))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_DETECTTRAPS, obj);
   if (xIS_SET(qobj->flags2, QOBJ_SCRYING))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_SCRYING, obj);
   if (xIS_SET(qobj->flags2, QOBJ_FIRESHIELD))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_FIRESHIELD, obj);
   if (xIS_SET(qobj->flags2, QOBJ_SHOCKSHIELD))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_SHOCKSHIELD, obj);
   if (xIS_SET(qobj->flags2, QOBJ_ICESHIELD))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_ICESHIELD, obj);
   if (xIS_SET(qobj->flags2, QOBJ_AQUA_BREATH))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_AQUA_BREATH, obj);
   if (xIS_SET(qobj->flags2, QOBJ_WIZARDEYE))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_WIZARDEYE, obj);
   if (xIS_SET(qobj->flags2, QOBJ_EWIZARDEYE))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_E_WIZARDEYE, obj);
   if (xIS_SET(qobj->flags2, QOBJ_MWIZARDEYE))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_M_WIZARDEYE, obj);
   if (xIS_SET(qobj->flags2, QOBJ_NOHUNGER))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_NOHUNGER, obj);
   if (xIS_SET(qobj->flags2, QOBJ_NOTHIRST))
      link_paf_quest(APPLY_EXT_AFFECT, AFF_NOTHIRST, obj);
      
   return;
}

char *const weaponqobj_race[MAX_RACE] = 
{
   "A Human", "An Elven", "A Dwarven", "An Ogre", "A Hobbit", "A Fairy"
};

OBJ_DATA *adjust_questobj(CHAR_DATA *ch, OBJ_DATA *obj, int difficulty, QOBJ_DATA *qobj, int type, int dtype, int svnum)
{
   char buf[MSL];
   OBJ_DATA *nobj;
   OBJ_DATA *sobj;
   SLAB_DATA *slab;
   int y;
   int cnt = 1;
   
   if (!qobj)  // Forge equipment fun
   {
      int x;
      int race = number_range(0, MAX_RACE-1);
      int oldrace;
      
      oldrace = ch->race;
      
      if (ch->race < MAX_RACE)
         race = ch->race;
         
      if (svnum > 0)
         x = svnum;
      else
         x = number_range(OBJ_FORGE_HAND_AXE, OBJ_FORGE_PAVIS);
      obj = create_object(get_obj_index(x), 1); 
      if (dtype == 1 && difficulty < 1000)
         difficulty+=10;
      if (dtype == 2 && difficulty < 1000)
         difficulty+=20;
         
      if (difficulty >= 1001)
      {
         y = difficulty-1000;
         for (slab = first_slab; slab ; slab = slab->next) 
         {
            if (y == cnt)
               break;
            cnt++;
         }     
         difficulty -= 100;
         if (difficulty >= 120)
            difficulty = 120;    
      }
      else if (difficulty <= 100)
      {
         int lvl = difficulty;
         int slab1;
         int slab2;
         int slab3;
         SLAB_DATA *firstslab;
         SLAB_DATA *secondslab;
         SLAB_DATA *thirdslab = NULL;
         int per1, per2, per3;
         if (lvl > 111)
            lvl = 111;

         slab1 = get_slab_vnum(lvl, 1);
         slab2 = get_slab_vnum(lvl, 2);
         slab3 = get_slab_vnum(lvl, 3);
         per1 = get_slab_vnum(lvl, 4);
         per2 = get_slab_vnum(lvl, 5);
         per3 = get_slab_vnum(lvl, 6);
                           
         for (firstslab = first_slab; firstslab; firstslab = firstslab->next)
         {
            if (firstslab->vnum == slab1)
               break;
         }
         for (secondslab = first_slab; secondslab; secondslab = secondslab->next)
         {
            if (secondslab->vnum == slab2)
               break;
         }
         if (slab3 > 0)
         {
            for (thirdslab = first_slab; thirdslab; thirdslab = thirdslab->next)
            {
               if (thirdslab->vnum == slab3)
               break;
            }
         }
         if (!firstslab)
         {
            bug("adjust_questobj: %d is not a valid Slab", slab1);
            return NULL;
         }
         else if (!secondslab)
         {
            bug("adjust_questobj: %d is not a valid Slab", slab2);
            return NULL;
         }
         else if (slab3 > 0 && !thirdslab)
         {
            bug("adjust_questobj: %d is not a valid Slab", slab3);
            return NULL;
         }
         if (per3 <= 0)
         {
            if (number_range(1, 100) <= per1)
               slab = firstslab;
            else
               slab = secondslab;
         }
         else
         {
            int num = number_range(1, 100);
            if (num <= per1)
               slab = firstslab;
            else if (num > per3)
               slab = thirdslab;
            else
               slab = secondslab; 
         }	
      }
      else
      {
         if (difficulty <= 120)  //12
         {
            y = number_range(12, 16);
            for (slab = first_slab; slab ; slab = slab->next) 
            {
               if (y == cnt)
                  break;
               cnt++;
            }   
         }
         else if (difficulty <= 140) //13-14
         {
            y = number_range(17, 21);
            for (slab = first_slab; slab ; slab = slab->next) 
            {
               if (y == cnt)
                  break;
               cnt++;
            }   
         }
         else  //15
         { 
            y = number_range(22, 23);
            for (slab = first_slab; slab ; slab = slab->next) 
            {
               if (y == cnt)
                  break;
               cnt++;
            }   
         }
      }
      if (!slab)
      {
         bug("adjust_questobj:  Failed to load an ore for adjusted difficulty %d", difficulty);
         return NULL;
      }
      ch->race = race;
      nobj = create_object(get_obj_index(obj->pIndexData->vnum), 1);
      if (ch)
         obj_to_char(nobj, ch);
      extract_obj(obj);
      sobj = create_object(get_obj_index(slab->vnum), 1);
      alter_forge_obj(ch, nobj, sobj, slab);	
      extract_obj(sobj);
      ch->race = oldrace;
      CREATE(qobj, QOBJ_DATA, 1);
      ochange = 0;
      if (difficulty <= 110)
      {
         if (dtype == 0)
            xSET_BIT(qobj->flags2, QOBJ_STAT);
         if (dtype == 1)
         {
            xSET_BIT(qobj->flags2, QOBJ_STAT2);
            if (number_range(1, 5) == 1)
               xSET_BIT(qobj->flags2, QOBJ_BATTLE);
         }
         if (dtype == 2)
         {
            xSET_BIT(qobj->flags2, QOBJ_STAT3);
            if (number_range(1, 5) >= 4)
               xSET_BIT(qobj->flags2, QOBJ_BATTLE2);
            else
               xSET_BIT(qobj->flags2, QOBJ_BATTLE);
         }
      }
      else if (difficulty <= 130)
      {
         if (dtype == 0)
            xSET_BIT(qobj->flags2, QOBJ_STAT);
         if (dtype == 1)
         {
            xSET_BIT(qobj->flags2, QOBJ_STAT2);
            xSET_BIT(qobj->flags2, QOBJ_BATTLE);
         }
         if (dtype == 2)
         {
            if (number_range(1, 5) >= 4)
               xSET_BIT(qobj->flags2, QOBJ_STAT3);
            else
               xSET_BIT(qobj->flags2, QOBJ_STAT4); 
            
            if (number_range(1, 5) >= 4)
               xSET_BIT(qobj->flags2, QOBJ_BATTLE3);
            else
               xSET_BIT(qobj->flags2, QOBJ_BATTLE2);
         }
      }
      else if (difficulty > 130)
      {
         if (dtype == 0)
            xSET_BIT(qobj->flags2, QOBJ_STAT2);
         if (dtype == 1)
         {
            xSET_BIT(qobj->flags2, QOBJ_STAT3);
            xSET_BIT(qobj->flags2, QOBJ_BATTLE2);
         }
         if (dtype == 2)
         {
            if (number_range(1, 5) >= 4)
               xSET_BIT(qobj->flags2, QOBJ_STAT4);
            else
               xSET_BIT(qobj->flags2, QOBJ_STAT5); 
            
            if (number_range(1, 5) >= 4)
               xSET_BIT(qobj->flags2, QOBJ_BATTLE4);
            else
               xSET_BIT(qobj->flags2, QOBJ_BATTLE3);
         }
      }        
      adjust_qobj_flags(nobj, qobj);
      DISPOSE(qobj);
      if (ochange == 1)
      {
         sprintf(buf, "&w&C%s&w%s", nobj->short_descr, char_color_str(AT_OBJECT, ch));
         STRFREE(nobj->short_descr);  
         nobj->short_descr = STRALLOC(buf);
      }
      xSET_BIT(nobj->extra_flags, ITEM_QUESTOBJ);
      if (type == 1)
         wear_obj(ch, nobj, TRUE, -1);
      return nobj;
   }
   STRFREE(obj->name);
   STRFREE(obj->short_descr);
   STRFREE(obj->description);
   sprintf(buf, "%s", qobj->name);
   obj->name = STRALLOC(buf);
   obj->short_descr = STRALLOC(buf);
   sprintf(buf, "%s %s", qobj->name, qobj_long_descr[number_range(0, 11)]);
   obj->description = STRALLOC(buf);
   obj->weight = qobj->weight;
   obj->cost = qobj->gold;
   obj->value[0] = qobj->value[0];
   obj->value[1] = qobj->value[1];
   obj->value[2] = qobj->value[2];
   obj->value[3] = qobj->value[3];
   obj->value[4] = qobj->value[4];
   obj->value[5] = qobj->value[5];
   obj->value[6] = qobj->value[6];
   obj->value[7] = qobj->value[7];
   obj->value[8] = qobj->value[8];
   obj->value[9] = qobj->value[9];
   obj->value[10] = qobj->value[10];
   obj->value[11] = qobj->value[11];
   obj->value[12] = qobj->value[12];
   obj->value[13] = qobj->value[13];
   if (qobj->type == 1) //Weapon
   {
      SET_BIT(obj->wear_flags, ITEM_WIELD);
      obj->item_type = ITEM_WEAPON;
   }
   if (qobj->type == 2) //Armor
      obj->item_type = ITEM_ARMOR;
   if (qobj->type == 3) //Potion
      obj->item_type = ITEM_POTION;
   if (qobj->type == 4) //Scroll
      obj->item_type = ITEM_SCROLL;
   if (qobj->type == 5) //Jewelry
      obj->item_type = ITEM_TREASURE;
   if (qobj->type == 6) //Container
      obj->item_type = ITEM_CONTAINER;
   if (qobj->type == 7) //Projectile Weapon
   {
      SET_BIT(obj->wear_flags, ITEM_MISSILE_WIELD);
      obj->item_type = ITEM_MISSILE_WEAPON;
   }
   if (qobj->type == 8) //Quiver
      obj->item_type = ITEM_QUIVER;
   if (qobj->type == 9) //Furniture
      obj->item_type = ITEM_FURNITURE;
   if (qobj->type == 10) //Sheath
      obj->item_type = ITEM_SHEATH;
   if (qobj->type == 11) //Repair Hammer
      obj->item_type = ITEM_REPAIR;
   if (qobj->type == 12) //Climbing equipment
      obj->item_type = ITEM_MCLIMB;
   if (qobj->type == 13) //Drinking container
      obj->item_type = ITEM_DRINK_CON;
   if (qobj->type == 14) //Food
      obj->item_type = ITEM_FOOD;
   if (qobj->type == 15) //Light
      obj->item_type = ITEM_LIGHT;
   if (qobj->type == 16) //Rune
      obj->item_type = ITEM_RUNE;

   adjust_qobj_flags(obj, qobj);
   if (qobj->race == 1 && (qobj->type == 1 || qobj->type == 2 || qobj->type == 7))
   {
      int race = number_range(0, MAX_RACE-1);
      if (ch && ch->race < MAX_RACE)
         race = ch->race;
      if (qobj->type == 2)
      {
         if(race == 5)
		 {
			obj->weight = obj->weight * .4;
			if (obj->weight < .01)
			   obj->weight = .01;
		 }
	 	 if(race == 4)
		 {
			obj->weight = obj->weight * .6;
			if (obj->weight < .01)
			   obj->weight = .01;
		 }
		 if(race == 1)
		 {
			obj->weight = obj->weight * .85;
			if (obj->weight < .01)
			   obj->weight = .01;
		 }
		 if(race == 0)
		 {
			//essentially do nothing... this is just here in case
			//we change human base stats
			obj->weight = obj->weight * 1;
			if (obj->weight < .01)
			   obj->weight = .01;
		 }
		 if(race == 2)
		 {
			obj->weight = obj->weight * 1.2;
			if (obj->weight < .01)
			   obj->weight = .01;
		 }
		 if(race == 3)
		 {
			obj->weight = obj->weight * 1.5;
			if (obj->weight < .01)
			   obj->weight = .01;
		 }
      }
      else
      {
         obj->value[3] = race_table[race]->weaponmin + obj->value[3];
         if(race == 5)
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
 		 if(race == 4)
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
		 if(race == 1)
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
		 if(race == 0)
		 {
			//essentially do nothing... this is just here in case
			//we change human base stats
		 }
		 if(race == 2)
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
		 if(race == 3)
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
      sprintf(buf, "%s %s", weaponqobj_race[race], obj->name);
      STRFREE(obj->name);
      STRFREE(obj->short_descr);
      obj->name = STRALLOC(buf);
      obj->short_descr = STRALLOC(buf);
      sprintf(buf, "%s %s", weaponqobj_race[race], obj->description);
      STRFREE(obj->description);
      obj->description = STRALLOC(buf);
   }     
   if (type == 1)
      wear_obj(ch, obj, TRUE, -1);
   return NULL;
}

//Installs the quest area into area.lst and does a foldarea.
void do_foldqarea(CHAR_DATA *ch, char *argument)
{
   AREA_DATA *tarea;

   set_char_color(AT_IMMORT, ch);

   if (!argument || argument[0] == '\0')
   {
      send_to_char("Fold what?\n\r", ch);
      return;
   }

   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (!str_cmp(tarea->filename, argument))
      {
         break;
      }
   }
   if (!tarea)
   {
      send_to_char("No such area exists.\n\r", ch);
      return;
   }
   if (tarea->low_r_vnum < START_STATICQUEST_VNUM || tarea->low_r_vnum > END_STATICQUEST_VNUM)
   {
      send_to_char("This will only work on static quest areas.\n\r", ch);
      return;
   }
   send_to_char("Folding area then installing...\n\r", ch);
   fold_area(tarea, tarea->filename, FALSE, 0);
   write_area_list();
   set_char_color(AT_IMMORT, ch);
   send_to_char("Done.\n\r", ch);
   return;
}

int get_sworth_increase(int diff)
{
   if (diff == 1)
      return number_range(1500, 2000);
   else if (diff == 2)
      return number_range(3000, 4000);
   else if (diff == 3)
      return number_range(6000, 7000);
   else if (diff == 4)
      return number_range(10000, 12000);
   else if (diff == 5)
      return number_range(15000, 18000);
   else if (diff == 6)
      return number_range(25000, 30000);
   else if (diff == 7)
      return number_range(35000, 40000);
   else if (diff == 8)
      return number_range(50000, 55000);
   else if (diff == 9)
      return number_range(70000, 75000);
   else if (diff == 10)
      return number_range(90000, 100000);
   else if (diff == 11)
      return number_range(120000, 130000);
   else if (diff == 12)
      return number_range(180000, 200000);
   else if (diff == 13)
      return number_range(250000, 300000);
   else if (diff == 14)
      return number_range(400000, 450000);
   else
      return number_range(600000, 800000);
}

void get_qgem_name(int diff, OBJ_DATA *obj)
{
   char buf[MSL];
   char buf2[MSL];
   
   if (diff == 1)
      sprintf(buf, "a white gem");
   else if (diff == 2)
      sprintf(buf, "a yellow gem");
   else if (diff == 3)
      sprintf(buf, "a green gem");
   else if (diff == 4)
      sprintf(buf, "a blue gem");
   else if (diff == 5)
      sprintf(buf, "a purple gem");
   else if (diff == 6)
      sprintf(buf, "a pink gem");
   else if (diff == 7)
      sprintf(buf, "a red gem");
   else if (diff == 8)
      sprintf(buf, "a white diamond");
   else if (diff == 9)
      sprintf(buf, "a yellow diamond");
   else if (diff == 10)
      sprintf(buf, "a green diamond");
   else if (diff == 11)
      sprintf(buf, "a blue diamond");
   else if (diff == 12)
      sprintf(buf, "a purple diamond");
   else if (diff == 13)
      sprintf(buf, "a pink diamond");
   else if (diff == 14)
      sprintf(buf, "a red diamond");
   else    
      sprintf(buf, "a perfect diamond");
      
   STRFREE(obj->name);
   obj->name = STRALLOC(buf);
   
   if (diff == 1)
      sprintf(buf, "&w&Wa white gem");
   else if (diff == 2)
      sprintf(buf, "&w&Ya yellow gem");
   else if (diff == 3)
      sprintf(buf, "&w&Ga green gem");
   else if (diff == 4)
      sprintf(buf, "&w&Ba blue gem");
   else if (diff == 5)
      sprintf(buf, "&w&pa purple gem");
   else if (diff == 6)
      sprintf(buf, "&w&Pa pink gem");
   else if (diff == 7)
      sprintf(buf, "&w&Ra red gem");
   else if (diff == 8)
      sprintf(buf, "&w&Wa white diamond");
   else if (diff == 9)
      sprintf(buf, "&w&Ya yellow diamond");
   else if (diff == 10)
      sprintf(buf, "&w&Ga green diamond");
   else if (diff == 11)
      sprintf(buf, "&w&Ba blue diamond");
   else if (diff == 12)
      sprintf(buf, "&w&pa purple diamond");
   else if (diff == 13)
      sprintf(buf, "&w&Pa pink diamond");
   else if (diff == 14)
      sprintf(buf, "&w&Ra red diamond");
   else
      sprintf(buf, "&c&wa perfect diamond");
   
   STRFREE(obj->short_descr);
   obj->short_descr = STRALLOC(buf);
   
   sprintf(buf2, "&w&GSome has left behind a valuable %s &w&Gbehind for you.", buf);
   STRFREE(obj->description);
   obj->description = STRALLOC(buf2);
   return;
}

//1 - 5 - Armor 6 - 8 - Stone 9 - 13 - Tohit 14 - 16 - Sanctify 17-19 - Shield 20-21 - Wmod 22 - Fasting
//23 - MFuse 24 - MShell 25 - MSheild 26 - MGuard 27 - Mburn 28 - WClamp 29 - ACatch
//30 - Bracing 31 - Hardening 32-34 - HRegen 35-37 - MRegen 38 - Damage 39 - Durability
//40 - ToHitBash 41 - ToHitStab 42 - TohitSlash 43 - Weight 44 - Shieldlag 45 - Blocking %
//46 - Proj Range 47 - Parry Chance 48 - Stop Parry 49 - Spellsn + Spellstr 50 - Unbreakable
//51 - NoDisarm 52-56 - Sanctified  57 - Change Size 58-60 - Rfire 61-63 - RWater 64-66 - RAir
//67-69 - REarth 70-72 - REnergy 73 - RMagic 74 - RNonmagic 75-76 - RBlunt 77-78 - RPierce
//79-80 - RSlash 81-83 - RPoison 84 - RPara 85-87 - RHoly 88-90 - RUnholy 91-93 - RUndead
//94 - Detect Invis 95-99 - Detect Magic 100 - Detect Hidden 101-105 - Infrared
//106 - Flying 107 - Pass Door 108-109 - Truesight 110 - Detect Traps 111 - Scrying
//112 - Fireshield 113 - Shockshield 114 - Iceshield 115-117 - Aqua Breath 118 - Wizardeye
//119-121 - Nohunger 122-124 - Nothirst 125-135 - Str 136-146 - Dex 147-157 - Int 158-168 - Wis 169-179 - Con
//180-200 - Mana 201-221 - Hit 222-225 - Saves 226-236 - Luck 237-247 - agi

void create_questgem(OBJ_DATA *obj, int difficulty, int type, CHAR_DATA *victim)
{
   int slot = 0;
   int fnd = 0;
   
   difficulty = UMAX(1, ((difficulty-1)/10)+1);
   difficulty += type;
   
   for (;;)
   {
      if (number_range(1, 2) == 1)
         difficulty++;
      else
         break;
   }
   if (difficulty > MAX_QDIFF+2)
      difficulty = MAX_QDIFF+2;
   obj->item_type = ITEM_TGEM;
   for (;;)
   {
      type = number_range(1, 247);
      if (type >= 237 && type <= 247) // Agi
      {
         obj->value[slot] = 77;
         obj->value[slot+2] = UMAX(1, difficulty/3);
         obj->value[slot+3] = UMAX(1, difficulty*2/5)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 226 && type <= 236) // Luck
      {
         obj->value[slot] = 31;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = difficulty/7+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 222 && type <= 225) // Saves
      {
         obj->value[slot] = 1017;
         obj->value[slot+2] = -1*UMAX(1, difficulty/3);
         obj->value[slot+3] = -1*UMAX(1, difficulty/2);
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 201 && type <= 221) // Hit
      {
         obj->value[slot] = 13;
         obj->value[slot+2] = difficulty;
         obj->value[slot+3] = difficulty*2;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 180 && type <= 200) // Mana
      {
         obj->value[slot] = 12;
         obj->value[slot+2] = difficulty;
         obj->value[slot+3] = difficulty*2;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 169 && type <= 179) // Con
      {
         obj->value[slot] = 5;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = difficulty/7+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 158 && type <= 168) // Wis
      {
         obj->value[slot] = 4;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = difficulty/7+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 147 && type <= 157) // Int
      {
         obj->value[slot] = 3;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = difficulty/7+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 136 && type <= 146) // Dex
      {
         obj->value[slot] = 2;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = difficulty/7+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 125 && type <= 135) // Str
      {
         obj->value[slot] = 1;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = difficulty/7+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 122 && type <= 124 && difficulty <= 10) // Nothirst
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 39;
         obj->value[slot+3] = 39;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 119 && type <= 121 && difficulty <= 10) // Nohunger
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 38;
         obj->value[slot+3] = 38;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 118 && type <= 118 && difficulty <= 8) // Wizardeye
      {
         obj->value[slot] = 70;
         if (difficulty <= 3)
            obj->value[slot+2] = 34;
         else if (difficulty <= 5)
            obj->value[slot+2] = 35;
         else if (difficulty <= 8)
            obj->value[slot+2] = 36;
         obj->value[slot+3] = obj->value[slot+2];
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 115 && type <= 117 && difficulty <= 8) // Aquabreath
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 31;
         obj->value[slot+3] = 31;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 114 && type <= 114 && difficulty > 5) // Iceshield
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 28;
         obj->value[slot+3] = 28;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 113 && type <= 113 && difficulty > 5) // Shockshield
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 26;
         obj->value[slot+3] = 26;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 112 && type <= 112 && difficulty > 5) // Fireshield
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 25;
         obj->value[slot+3] = 25;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 111 && type <= 111 && difficulty > 5) // Scrying
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 24;
         obj->value[slot+3] = 24;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 110 && type <= 110 && difficulty <= 8) // Detect Traps
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 23;
         obj->value[slot+3] = 23;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 108 && type <= 109 && difficulty > 5) // True Sight
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 22;
         obj->value[slot+3] = 22;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 107 && type <= 107 && difficulty <= 8) // Pass Door
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 20;
         obj->value[slot+3] = 20;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 106 && type <= 106 && difficulty > 5) // Flying
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 19;
         obj->value[slot+3] = 19;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 101 && type <= 105 && difficulty <= 8) // Infrared
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 9;
         obj->value[slot+3] = 9;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 100 && type <= 100) // Detect Hidden
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 5;
         obj->value[slot+3] = 5;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 95 && type <= 99 && difficulty <= 8) // Detect Magic
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 4;
         obj->value[slot+3] = 4;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 94 && type <= 94) // Detect Invis
      {
         obj->value[slot] = 70;
         obj->value[slot+2] = 3;
         obj->value[slot+3] = 3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 91 && type <= 93) // RUndead
      {
         obj->value[slot] = 110;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 88 && type <= 90) // RUnholy
      {
         obj->value[slot] = 109;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 85 && type <= 87) // RHoly
      {
         obj->value[slot] = 108;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 84) // RPara
      {
         obj->value[slot] = 107;
         obj->value[slot+2] = 95 - difficulty*2;
         obj->value[slot+3] = 90 - difficulty*5/2;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 81 && type <= 83) // RPoison
      {
         obj->value[slot] = 106;
         obj->value[slot+2] = UMAX(-1, 70 - difficulty*5);
         obj->value[slot+3] = UMAX(-1, 60 - difficulty*7);
         if (obj->value[slot+2] == 0)
            obj->value[slot+2] = -1;
         if (obj->value[slot+3] == 0)
            obj->value[slot+3] = -1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 79 && type <= 80) // RSlash
      {
         obj->value[slot] = 105;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 77 && type <= 78) // RPierce
      {
         obj->value[slot] = 104;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 75 && type <= 76) // RBlunt
      {
         obj->value[slot] = 103;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 74) // RNonMagic
      {
         obj->value[slot] = 102;
         obj->value[slot+2] = 95 - difficulty*2;
         obj->value[slot+3] = 90 - difficulty*5/2;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 73) // RMagic
      {
         obj->value[slot] = 101;
         obj->value[slot+2] = 95 - difficulty*2;
         obj->value[slot+3] = 90 - difficulty*5/2;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 70 && type <= 72) // REnergy
      {
         obj->value[slot] = 100;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 67 && type <= 69) // REarth
      {
         obj->value[slot] = 99;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 64 && type <= 66) // RAir
      {
         obj->value[slot] = 98;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 61 && type <= 63) // RWater
      {
         obj->value[slot] = 97;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 58 && type <= 60) // RFire
      {
         obj->value[slot] = 96;
         obj->value[slot+2] = 95 - difficulty*5/2;
         obj->value[slot+3] = 90 - difficulty*3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 57 && difficulty > 8) // Change Size
      {
         obj->value[slot] = 1016;
         if (number_range(1, 2) == 1)
         {
            obj->value[slot+2] = 1;
            obj->value[slot+3] = 2+difficulty/14;
         }
         else
         {            
            obj->value[slot+2] = 1;
            obj->value[slot+3] = 2+difficulty/14;
         }
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 52 && type <= 56 && difficulty > 8) // Sanctified
      {
         obj->value[slot] = 1015;
         obj->value[slot+2] = 1;
         obj->value[slot+3] = 1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 51 && difficulty > 8) // Nodisarm
      {
         obj->value[slot] = 1014;
         obj->value[slot+2] = 1;
         obj->value[slot+3] = 1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 50 && difficulty > 5) // Unbreakable
      {
         obj->value[slot] = 1013;
         obj->value[slot+2] = 1;
         obj->value[slot+3] = 1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 49) // SpellSn & SpellStr
      {
         ;
      }
      else if (type == 48) // Stop Parry
      {
         obj->value[slot] = 1010;
         obj->value[slot+2] = UMAX(1, difficulty/3);
         obj->value[slot+3] = UMAX(1, difficulty/2)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 47) // Parry Chance
      {
         obj->value[slot] = 1009;
         obj->value[slot+2] = UMAX(1, difficulty/3);
         obj->value[slot+3] = UMAX(1, difficulty/2)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 46) // Proj Range
      {
         obj->value[slot] = 1008;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = UMAX(1, difficulty/14)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 45) // Blocking %
      {
         obj->value[slot] = 1007;
         obj->value[slot+2] = UMAX(1, difficulty*2/3);
         obj->value[slot+3] = UMAX(1, difficulty)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 44) // Shieldlag
      {
         obj->value[slot] = 1006;
         obj->value[slot+2] = difficulty/11;
         obj->value[slot+3] = difficulty/14+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 43) // Weight
      {
         obj->value[slot] = 1005;
         obj->value[slot+2] = UMAX(1, difficulty/4);
         obj->value[slot+3] = UMAX(1, difficulty/3)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 42) // ToHitSlash
      {
         obj->value[slot] = 1004;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = UMAX(1, difficulty/14)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 41) // ToHitStab
      {
         obj->value[slot] = 1003;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = UMAX(1, difficulty/14)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 40) // ToHitBash
      {
         obj->value[slot] = 1002;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = UMAX(1, difficulty/14)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 39) // Durability
      {
         obj->value[slot] = 1001;
         obj->value[slot+2] = UMAX(1, difficulty/20);
         obj->value[slot+3] = UMAX(1, difficulty/14)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 38) // Damage
      {
         obj->value[slot] = 1000;
         obj->value[slot+2] = UMAX(1, difficulty/6);
         obj->value[slot+3] = UMAX(1, difficulty/5)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 35 && type <= 37) // MRegen
      {
         obj->value[slot] = 83;
         obj->value[slot+2] = 105 + difficulty*15;
         obj->value[slot+3] = 115 + difficulty*20;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 32 && type <= 34) // HRegen
      {
         obj->value[slot] = 84;
         obj->value[slot+2] = 105 + difficulty*15;
         obj->value[slot+3] = 115 + difficulty*20;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      /*
      else if (type == 31) // Hardening
      {
         obj->value[slot] = 95;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      } */
      else if (type == 30) // Bracing
      {
         obj->value[slot] = 94;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 29) // ACatch
      {
         obj->value[slot] = 93;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 28) // WClamp
      {
         obj->value[slot] = 92;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 27) // MBurn
      {
         obj->value[slot] = 91;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 26) // MGuard
      {
         obj->value[slot] = 90;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 25) // MShield
      {
         obj->value[slot] = 89;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 24) // MShell
      {
         obj->value[slot] = 88;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 23) // MFuse
      {
         obj->value[slot] = 86;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type == 22) // Fasting
      {
         obj->value[slot] = 87;
         obj->value[slot+2] = 5 + difficulty*3/2;
         obj->value[slot+3] = 10 + difficulty*5/3;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 20 && type <= 21) // Wmod
      {
         obj->value[slot] = 85;
         obj->value[slot+2] = 95 - difficulty*2;
         obj->value[slot+3] = 90 - difficulty*5/2;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 17 && type <= 19) // Shield
      {
         obj->value[slot] = 79;
         obj->value[slot+2] = UMAX(1, difficulty/5);
         obj->value[slot+3] = UMAX(1, difficulty/4)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 14 && type <= 16) // Sanctify
      {
         obj->value[slot] = 81;
         obj->value[slot+2] = UMAX(1, difficulty/4);
         obj->value[slot+3] = UMAX(1, difficulty/3)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 9 && type <= 13) // Tohit
      {
         obj->value[slot] = 82;
         obj->value[slot+2] = UMAX(1, difficulty/4);
         obj->value[slot+3] = UMAX(1, difficulty/3)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 6 && type <= 8) // Stone
      {
         obj->value[slot] = 80;
         obj->value[slot+2] = UMAX(1, difficulty/6);
         obj->value[slot+3] = UMAX(1, difficulty/5)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      else if (type >= 1 && type <= 5) //Armor
      {
         obj->value[slot] = 78;
         obj->value[slot+2] = UMAX(1, difficulty/4);
         obj->value[slot+3] = UMAX(1, difficulty/3)+1;
         obj->value[slot+1] = get_sworth_increase(difficulty);
         fnd = 1;
      }
      if (fnd == 1)
      {
         if (slot == 0)
         {
            obj->value[12] = number_range(1, 4);
            if (difficulty > 5)
               if (number_range(1, 4) == 1)
                  obj->value[12] -=1;
            if (difficulty > 10)
               if (number_range(1, 2) == 1)
                  obj->value[12] -=1;
            if (difficulty > 13)
                  obj->value[12] -=1;
            obj->value[12] = UMAX(1, obj->value[12]);
            
            if (number_range(1, 1000) <= 10+difficulty*3+(UMAX(0, difficulty-10)*10))
            {
               obj->value[12] = -1;
               if (IS_NPC(victim))
                  act(AT_BLUE, "&w&BINFO: &w&WA slotless gem has loaded on $N", supermob, NULL, victim, TO_MUD);
               else
                  send_to_char("Loaded a slotless gem\n\r", victim);
            }
               
            if (number_range(1, 100) <= 10 + (difficulty/3))
            {
               slot+=4;
               fnd = 0;
               continue;  
            }
            else
               break;
         }
         else if (slot == 4)
         {
            if (number_range(1, 100) <= 5 + (difficulty * 3 / 2))
            {
               slot+=4;
               fnd = 0;
               continue;  
            }
            else
               break;
         }
         else
            break;
      }
   }  
   get_qgem_name(difficulty, obj);
   obj->cost = get_sworth_increase(difficulty)/2;
   obj->weight = .05;
   return;
} 
   
void do_loadgem(CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *obj;
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  loadgem <diff>\n\r", ch);
      return;
   }
   if (atoi(argument) < 1 || atoi(argument) > MAX_QDIFF+2)
   {
      ch_printf(ch, "Diff range is 1 to %d", MAX_QDIFF+2);
      return;
   }
   obj = create_object(get_obj_index(OBJ_VNUM_QUESTOBJ), 1);
   obj_to_char(obj, ch);
   create_questgem(obj, atoi(argument)*10-1, 0, ch);
   send_to_char("Done.\n\r", ch);
}

void do_fixgemslots(CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *obj;
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  fixgemslots <item>\n\r", ch);
      return;
   }
   if ((obj = get_obj_carry(ch, argument)) == NULL)
   {
      send_to_char("That is not in your inventory.\n\r", ch);
      return;
   }
   obj->imbueslots = obj->pIndexData->imbueslots;
   ch_printf(ch, "Fixed %s\n\r", obj->short_descr);
   return;
}
   
//type 0 - Normal   1 - Captain(Unique)   2 - Boss(Rare)
void load_questobj(CHAR_DATA *victim, int difficulty, int type, AREA_DATA *area)
{
   QOBJ_DATA *qobj;
   QOBJ_DATA *qobjqueue[1000]; //More than enough I would think
   int x = -1;
   int y;
   int aq;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *iobj;
   int lesser = 0;
   int diff = ((difficulty-1)/10)+1;
   int gemchance = 0;
   int bossgem = 0;
   
   if (type == 0)
      gemchance = 1;
   else if (type == 1)
      gemchance = 75;
   else
      gemchance = 100;
   
   for (qobj = first_qobj; qobj; qobj = qobj->next)
   {
      if (diff >= qobj->lowdiff && diff <= qobj->hidiff && qobj->boss == type && x <= 998)
      {
         qobjqueue[++x] = qobj;
      }   
   }
   if (x == -1)
   {
      bug("load_quest_obj:  There are no objs to create for %d difficulty at %d type", difficulty, type);
      return;
   }
   aq = x;
   x += UMAX(1, (x+1)/2);
   for (;;)
   {
      if (!bossgem && lesser == 0 && number_range(1, 100) <= gemchance)
      {
         if (type == 2)
         {
            bossgem = 1;
         }
         else
         {
            lesser++;
         }
         if (area->hi_r_vnum >= START_STATICQUEST_VNUM)   
         {     
            iobj = make_object(raferquest_local_ovnum++, 0, "Quest Obj", 0);
            xREMOVE_BIT(iobj->extra_flags, ITEM_PROTOTYPE);
         }
         else
            iobj = get_obj_index(OBJ_VNUM_QUESTOBJ);            
         obj = create_object(iobj, 1);
         obj_to_char(obj, victim);
         create_questgem(obj, difficulty, type, victim);
         if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
            adjust_static_area_obj(obj);
         continue;
      }
      y = number_range(0, x);
      if (y > aq && type == 0)
         continue;
      if (y > aq)
      {
         obj = adjust_questobj(victim, NULL, difficulty, NULL, 1, type, 0);
         if (area->hi_r_vnum >= START_STATICQUEST_VNUM && obj)
         {
            copy_static_area_obj(victim, obj);
         }
         if (lesser == 1)
            lesser++;
      }
      else
      {
         if (qobjqueue[y]->type == 3 || qobjqueue[y]->type == 4 || qobjqueue[y]->type == 13 || qobjqueue[y]->type == 14) 
         {
            if (lesser == 1)
               continue;
            else
               lesser++;
         }
         else
         {
            if (lesser == 1)
               lesser++;
         }
         if (area->hi_r_vnum >= START_STATICQUEST_VNUM)   
         {     
            iobj = make_object(raferquest_local_ovnum++, 0, "Quest Obj", 0);
            xREMOVE_BIT(iobj->extra_flags, ITEM_PROTOTYPE);
         }
         else
            iobj = get_obj_index(OBJ_VNUM_QUESTOBJ);            
         obj = create_object(iobj, 1);
         obj_to_char(obj, victim);
         adjust_questobj(victim, obj, difficulty, qobjqueue[y], 1, type, 0);
         if (area->hi_r_vnum >= START_STATICQUEST_VNUM)
            adjust_static_area_obj(obj);
      }
      if (lesser == 0 || lesser == 2)
         break;
   }
   return;
}

/*
int get_room_vnum_quest(AREA_DATA *area, int load, int cnt)
{
   int size = area->hi_r_vnum - area->low_r_vnum;
   int vnum;
   if (load > size)
   {
      if ((load/size) > ((cnt-1)/size))
      {
         vnum = (cnt-1)%size+1+area->low_r_vnum;
         return vnum;
      }
      cnt = cnt%size;
      load = load%size;
   }
   vnum = cnt%load;
   if (vnum == 0)
      vnum = cnt;
   vnum = vnum*100/load;
   vnum = size*vnum/100;
   vnum = area->low_r_vnum + 1+ vnum;
   if (vnum < area->low_r_vnum+1)
      vnum = area->low_r_vnum+1;
   if (vnum > area->hi_r_vnum)
      vnum = area->hi_r_vnum;
   bug("%dcnt %dload %dsize %dvnum", cnt, load, size, vnum);
   return vnum;
} */


int get_room_vnum_quest(AREA_DATA *area, int load, int cnt, int mission, int bosses, int captains)
{
   int size = area->hi_r_vnum - area->low_r_vnum;
   float vnum;
   int setback = bosses+1+((captains-1)/2);
   
   if (mission == 2)
      size = area->hi_r_vnum - area->low_r_vnum - setback;
   if (load > size)
   {
      if ((load/size) > ((cnt-1)/size))
      {
         vnum = (cnt-1)%size+1+area->low_r_vnum;
         return (int)vnum;
      }
      cnt = cnt%size;
      load = load%size;
   }
   vnum = cnt%load;
   if (vnum == 0)
      vnum = cnt;
   vnum = vnum*100/load;
   vnum = size*vnum/100;
   vnum = area->low_r_vnum + 1+ vnum;
   if (mission == 2)
   {
      if (vnum < area->low_r_vnum+1)
         vnum = area->low_r_vnum+1;
      if (vnum > area->hi_r_vnum-setback)
         vnum = area->hi_r_vnum-setback;
   }
   else
   {
      if (vnum < area->low_r_vnum+1)
         vnum = area->low_r_vnum+1;
      if (vnum > area->hi_r_vnum)
         vnum = area->hi_r_vnum;
   }
   return (int)vnum;
}

void load_quest_mobs(AREA_DATA *area, int difficulty, int vsize, int mload, int mission)
{
   MOB_INDEX_DATA *mob;
   CHAR_DATA *victim;
   QMOB_DATA *qmob;
   QMOB_DATA *qmobqueue[1000]; //check in code, if more than 999 mobs fit then we should be rejoicing
   QMOB_DATA *qmobcqueue[500]; //Captains will go here
   QMOB_DATA *qmobbqueue[500]; //Bosses will go here
   QMOB_DATA *loadqueue[51];
   NPCRACE_DATA *loadnpcque[51];
   ROOM_INDEX_DATA *room;
   int vnum = area->low_r_vnum;
   int x = -1;
   int xr = -1;
   int xc = -1;
   int xb = -1; 
   int y;
   int cnt;
   int load;
   int nmake;
   int ncmake;
   int nbmake;
   int maload;
   int commontotal;
   int npcqueue[MAX_NPCRACE_TABLE];
   NPCRACE_DATA *npcrace;
   int diff = ((difficulty-1)/10)+1;
   
   for (y = 0; y <= 27; y++)
   {
      loadqueue[y] = NULL;
      loadnpcque[y] = NULL;
   }
   for (y = 0; y < MAX_NPCRACE_TABLE; y++)
      npcqueue[y] = -1;
   for (qmob = first_qmob; qmob; qmob = qmob->next)
   {
      if (diff >= qmob->lowdiff && diff <= qmob->hidiff && qmob->boss == 0 && x <= 998)
      {
         qmobqueue[++x] = qmob;
      }
      if (diff >= qmob->lowdiff && diff <= qmob->hidiff && qmob->boss == 1 && xc <= 498)
      {
         qmobcqueue[++xc] = qmob;
      }
      if (diff >= qmob->lowdiff && diff <= qmob->hidiff && qmob->boss == 2 && xb <= 498)
      {
         qmobbqueue[++xb] = qmob;
      }
      
   }
   for (npcrace = first_npcrace; npcrace; npcrace = npcrace->next)
   {
      if (npcrace->willload[diff-1] > 0)
         npcqueue[++xr] = npcrace->racenum;
   }
   if (xr == -1)
   {
      bug("load_quest_mobs:  There are no mobs in the npc table to create for %d difficulty", difficulty);
      return;
   }
   if (xc == -1)
   {
      bug("load_quest_mobs:  There are no captain mobs to create for %d difficulty", difficulty);
      return;
   }
   if (xb == -1)
   {
      bug("load_quest_mobs:  There are no boss mobs to create for %d difficulty", difficulty);
      return;
   }
   if (mission == 2)
      maload = ((vsize*4)+4)*mload/3/100;
   else
      maload = vsize*mload/100;
      
   ncmake = URANGE(1, maload/40, 25);
   nbmake = 1+ URANGE(0, maload/240, 5);
   nmake = URANGE(3, 3+vsize/100, 20);
   
   if (mission == 2)
   {
      if (((vsize*4)+4) < 100)
         nbmake = 0;
   }
   else
   {
      if (vsize < 100)
         nbmake = 0;
   }
   if (x+1 + xr+1 < 3)
   {
      bug("load_quest_mobs:  Less than 3 mobs for difficulty %d", difficulty);
      return;
   }
   if (nmake > x+1+xr+1)
      nmake = x+1+xr+1;
   if (nbmake > xb+1)
      nbmake = xb+1;
   if (ncmake > xc+1)
      ncmake = xc+1;
   commontotal = x+1+xr+1;
   //regular mobs
   for (y = 1; y <= nmake; y++)
   {
      if (number_range(1, commontotal) <= x+1) //Qmob Que Pull
      {
         loadqueue[y] = qmobqueue[number_range(0, x)];
         if (y == 1)
         {
            mob = make_mobile(vnum++, 0, loadqueue[y]->name);   
            continue;
         }
         for (load = 1; load < y; load++)
         {
            if (loadqueue[load] == loadqueue[y])
            {
               loadqueue[y] = NULL;
               y--;
               break;
            }
         }
         if (load != y)
            continue;
         mob = make_mobile(vnum++, 0, loadqueue[y]->name);   
      }
      else
      {
         loadnpcque[y] = npcrace_table[npcqueue[number_range(0, xr)]];   
         if (y == 1)
         {
            mob = make_mobile(vnum++, 0, add_npc_name(loadnpcque[y], diff));
            continue;
         }
         for (load = 1; load < y; load++)
         {
            if (loadnpcque[load] == loadnpcque[y])
            {
               loadnpcque[y] = NULL;
               y--;
               break;
            }
         }
         if (load != y)
            continue;
         mob = make_mobile(vnum++, 0, add_npc_name(loadnpcque[y], diff));
      }
   }
   vnum = area->low_r_vnum+20;
   //Captains
   for (y = 1; y <= ncmake; y++)
   {
      loadqueue[y+20] = qmobcqueue[number_range(0, xc)];
      if (y == 1)
         continue;
      for (load = 1; load < y; load++)
      {
         if (loadqueue[load+20] == loadqueue[y+20])
         {
            y--;
            continue;
         }
      }
   }
   for (y = 1; y <= ncmake; y++)
   {
      mob = make_mobile(vnum++, 0, loadqueue[y+20]->name);   
   }   
   vnum = area->low_r_vnum+45;
   //Bosses
   for (y = 1; y <= nbmake; y++)
   {
      loadqueue[y+45] = qmobbqueue[number_range(0, xb)];
      if (y == 1)
         continue;
      for (load = 1; load < y; load++)
      {
         if (loadqueue[load+45] == loadqueue[y+45])
         {
            y--;
            continue;
         }
      }
   }
   for (y = 1; y <= nbmake; y++)
   {
      mob = make_mobile(vnum++, 0, loadqueue[y+45]->name);   
   }  
        
   load = number_range(vsize*mload/90, vsize*mload/110);
   raferquest_local_ovnum = area->low_o_vnum;
   //regular mobs
   for (cnt = 1; cnt <= load; cnt++)
   {
      if (number_range(1, 10) <= 3)
         y = 1;
      else
         y = number_range(1, nmake);
      mob = get_mob_index(area->low_r_vnum+y-1);
      victim = create_mobile(mob);
      adjust_questmob(victim, difficulty, loadqueue[y], vsize, area, loadnpcque[y]);
      room = get_room_index(get_room_vnum_quest(area, load, cnt, mission, nbmake, ncmake));
      if (number_range(1, 100) <= 5)
         load_questobj(victim, difficulty, 0, area);
      else if (number_range(1, 100) <= 1)
         load_questobj(victim, difficulty, 1, area);
      char_to_room(victim, room);
   }
   //captains
   for (cnt = 1; cnt <= ncmake; cnt++)
   {
      mob = get_mob_index(area->low_r_vnum+20+cnt-1);
      victim = create_mobile(mob);
      adjust_questmob(victim, difficulty, loadqueue[cnt+20], vsize, area, NULL);
      xSET_BIT(victim->act, ACT_CAPTAIN);
      if (mission == 2)
         room = get_room_index(area->hi_r_vnum-nbmake-((cnt-1)/2)); 
      else
         room = get_room_index(number_range(area->low_r_vnum+((area->hi_r_vnum - area->low_r_vnum)/2), area->hi_r_vnum));
      if (number_range(1, 100) <= 10)
         load_questobj(victim, difficulty, 2, area);
      else
         load_questobj(victim, difficulty, 1, area);
      char_to_room(victim, room);   
   }
   //bosses
   for (cnt = 1; cnt <= nbmake; cnt++)
   {
      mob = get_mob_index(area->low_r_vnum+45+cnt-1);
      victim = create_mobile(mob);
      adjust_questmob(victim, difficulty, loadqueue[cnt+45], vsize, area, NULL);
      xSET_BIT(victim->act, ACT_BOSS);
      room = get_room_index(area->hi_r_vnum+1-cnt);
      load_questobj(victim, difficulty, 2, area);
      char_to_room(victim, room); 
   }   
   return;
}

void load_quest_groundloot(AREA_DATA *pArea, int difficulty, int vsize, int mload)
{
   int numload;
   QOBJ_DATA *qobj;
   QOBJ_DATA *qobjqueue[1000]; //More than enough I would think
   int x = -1;
   int y;
   int z;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *iobj;
   ROOM_INDEX_DATA *room;
   char buf[MSL];
   int type;
   int lesser = 0;
   int lvl = difficulty;
   int diff = ((difficulty-1)/10)+1;
   int gold = (lvl * lvl*3/2) + (lvl*40);
   int lgold;
   
   numload = URANGE(1, (vsize*mload)/2000, 20);
   
   for (z = 1; z <= numload; z++)
   {
      lesser = 0;
      room = get_room_index(number_range(pArea->low_r_vnum+1, pArea->hi_r_vnum));
      lgold = gold * number_range(80, 120) / 100;
      if (pArea->low_r_vnum >= START_STATICQUEST_VNUM)
      {
         iobj = make_object(raferquest_local_ovnum++, OBJ_VNUM_MONEY_SOME, "coins gold", 0);
         obj = create_object(iobj, 1);
         sprintf(buf, obj->short_descr, lgold);
         STRFREE(obj->short_descr);
         STRFREE(obj->pIndexData->short_descr);
         obj->short_descr = STRALLOC(buf);
         obj->pIndexData->short_descr = STRALLOC(buf);
         obj->value[0] = lgold;
         obj->pIndexData->value[0] = lgold;
         obj->cost = lgold;
         obj->pIndexData->cost = lgold;
         xREMOVE_BIT(obj->extra_flags, ITEM_PROTOTYPE);
         xREMOVE_BIT(obj->pIndexData->extra_flags, ITEM_PROTOTYPE);
         obj_to_room(obj, room, NULL);
      }
      else
      {
         obj_to_room(create_money(lgold), room, NULL);
      }
   
      if (number_range(1, 4) == 1)
      {
         if (number_range(1, 4) == 1)
            type = 1; 
         else
            type = 0;
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            if (diff >= qobj->lowdiff && diff <= qobj->hidiff && qobj->boss == type && x <= 998)
            {
               qobjqueue[++x] = qobj;
            }   
         }
         if (x == -1)
         {
            bug("load_quest_groundloot:  There are no objs to create for %d difficulty", difficulty);
            return;
         }
         for (;;)
         {
            y = number_range(0, x);
       
            if (qobjqueue[y]->type == 3 || qobjqueue[y]->type == 4 || qobjqueue[y]->type == 13 || qobjqueue[y]->type == 14)
            {
               if (lesser == 1)
                  continue;
               else
                  lesser++;
            }
            else
            {
               if (lesser == 1)
                  lesser++;
            }
      
            iobj = get_obj_index(OBJ_VNUM_QUESTOBJ);
            obj = create_object(iobj, 1);
            obj_to_room(obj, room, NULL);
            adjust_questobj(NULL, obj, difficulty, qobjqueue[y], 0, type, 0);
            if (pArea->hi_r_vnum >= START_STATICQUEST_VNUM)
               adjust_static_area_obj(obj);
            if (lesser == 0 || lesser == 2)
                break;
         }
      }
   }
}

void load_quest_keys(AREA_DATA *pArea)
{
   ROOM_INDEX_DATA *room;
   OBJ_INDEX_DATA *ikey;
   OBJ_DATA *key;
   CHAR_DATA *mob;
   int x;
   int cnt;
   int fnd;
   int kcnt = 1;
   char buf[MSL];
   char lbuf[MSL];
   
   for (x = pArea->low_r_vnum+1; x <= pArea->hi_r_vnum; x++)
   {
      room = get_room_index(x);
      if (!room)
      {
         bug("Failed to load a key for room %d due to it not existing", x);
         continue;
      }
      cnt = 0;
      for (mob = room->first_person; mob; mob = mob->next_in_room)
         cnt++;
      if (cnt == 0)
      {
         bug("Failed to load a key for room %d due to no mobs", x);
         continue;
      }
      fnd = number_range(1, cnt);
      cnt = 1;
      for (mob = room->first_person; mob; mob = mob->next_in_room)
      {
         if (cnt++ == fnd)
            break;
      }

      sprintf(buf, "Quest Key %d", kcnt++);
      ikey = make_object(x, 0, buf, 0);
      STRFREE(ikey->short_descr);
      ikey->short_descr = STRALLOC(buf);
      sprintf(lbuf, "A %s is here.", buf);
      STRFREE(ikey->description);
      ikey->description = STRALLOC(lbuf);
      key = create_object(ikey, 1);
      key->cost = 1;
      key->item_type = ITEM_KEY;
      key->timer = 10;
      key->weight = .01;
      xREMOVE_BIT(key->extra_flags, ITEM_PROTOTYPE);
      xREMOVE_BIT(ikey->extra_flags, ITEM_PROTOTYPE);
      xSET_BIT(key->extra_flags, ITEM_CLANOBJECT);
      obj_to_char(key, mob);
   }
}

int get_token_cost(int diff)
{
   int cost;
   if (diff >= 1 && diff <= 10)
      cost = 80;
   else if (diff >= 11 && diff <= 20)
      cost = 100;
   else if (diff >= 21 && diff <= 30)
      cost = 130;
   else if (diff >= 31 && diff <= 40)
      cost = 180;
   else if (diff >= 41 && diff <= 50)
      cost = 250;
   else if (diff >= 51 && diff <= 60)
      cost = 350;
   else if (diff >= 61 && diff <= 70)
      cost = 500;
   else if (diff >= 71 && diff <= 80)
      cost = 700;
   else if (diff >= 81 && diff <= 90)
      cost = 1000;
   else if (diff >= 91 && diff <= 100)
      cost = 1350;
   else if (diff >= 101 && diff <= 110)
      cost = 1800;
   else if (diff >= 111 && diff <= 120)
      cost = 2500;
   else
      cost = 3500;
   
   cost *= 10;
   cost = cost * sysdata.exp_percent / 300;
   
   return number_range(cost*75/100, cost*135/100);
}

void load_mob_tokens(AREA_DATA *pArea, int difficulty)
{
   ROOM_INDEX_DATA *room;
   OBJ_DATA *token;
   CHAR_DATA *mob;
   int cnt = 0;
   int total = 0;
   int x;
   char lbuf[MSL];
   
   for (x = pArea->low_r_vnum+1; x <= pArea->hi_r_vnum; x++)
   {
      room = get_room_index(x);
      if (!room)
      {
         bug("Failed to load a token for room %d due to it not existing", x);
         continue;
      }
      for (mob = room->first_person; mob; mob = mob->next_in_room)
      {
         if (xIS_SET(mob->act, ACT_BOSS))
            continue;
         if (xIS_SET(mob->act, ACT_CAPTAIN))
            continue;
         if (number_range(1, 100) <= 42)
         {
            token = create_object(get_obj_index(OBJ_VNUM_QUESTOBJ), 1);
            STRFREE(token->name);
            token->name = STRALLOC("Quest Token");
            STRFREE(token->short_descr);
            token->short_descr = STRALLOC("Quest Token");
            sprintf(lbuf, "A %s is here.", token->short_descr);
            STRFREE(token->description);
            token->description = STRALLOC(lbuf);
            token->cost = get_token_cost(difficulty);            
            token->item_type = ITEM_QTOKEN;
            token->weight = .05;
            xREMOVE_BIT(token->extra_flags, ITEM_PROTOTYPE);
            obj_to_char(token, mob);
            cnt++;
         }
         total++;
      }
   }
   for (;;)
   {
      if (cnt * 100 / total > 40)
         break;
      for (x = pArea->low_r_vnum+1; x <= pArea->hi_r_vnum; x++)
      {
         room = get_room_index(x);
         for (mob = room->first_person; mob; mob = mob->next_in_room)
         {
            if (number_range(1, 100) <= 3)
            {
               token = create_object(get_obj_index(OBJ_VNUM_QUESTOBJ), 1);
               STRFREE(token->name);
               token->name = STRALLOC("Quest Token");
               STRFREE(token->short_descr);
               token->short_descr = STRALLOC("Quest Token");
               sprintf(lbuf, "A %s is here.", token->short_descr);
               STRFREE(token->description);
               token->description = STRALLOC(lbuf);
               token->cost = get_token_cost(difficulty);
               token->item_type = ITEM_QTOKEN;
               token->weight = .05;
               xREMOVE_BIT(token->extra_flags, ITEM_PROTOTYPE);
               obj_to_char(token, mob);
               cnt++;
            }
         }
      }          
   }
}
      
int load_new_quest_zone(CHAR_DATA *ch, int type, int vsize, int difficulty, int mission, int mload)
{
   AREA_DATA *pArea;
   char vnumbuf[50];
   int randommapx[20];
   int randommapy[20];
   int x;
   int y;
   int xmod;
   int ymod;
   int xadd = 1;
   int yadd = 0;
   int sector;
   int z = 0;
   int cnt = 0;
   int rdir = 0;
   int w = 0;
   int rcnt = 0;
   int first = 1;
   ROOM_INDEX_DATA *room;
   int vnum = top_quest_vnum;
   EXIT_DATA *xit;
   
   for (x = 1; x <= 200; x++)
   {
      for (y = 1; y <= 200; y++)
      {
         gridmap[x][y] = 0;
         roomgrid[x][y] = NULL;
      }
   }
   for (x = 0; x <= 19; x++)
   {
      randommapx[x] = 0;
      randommapy[x] = 0;
   }
   eqload = 0;
   x = number_range(1, 20);
   if (x >= 18) //Field
      sector = 2;
   else if (x >= 11) //Underground
      sector = 13;
   else //Path
      sector = 44;
     
   x = 100;
   y = 100;  
   
   CREATE(pArea, AREA_DATA, 1);
   pArea->first_reset = NULL;
   pArea->last_reset = NULL;
   pArea->age = 15;
   pArea->nplayer = 0;
   pArea->low_r_vnum = top_quest_vnum;
   pArea->low_o_vnum = top_quest_vnum;
   pArea->low_m_vnum = top_quest_vnum;
   if (mission == 2)
      pArea->hi_r_vnum = top_quest_vnum+(vsize/4)-1;
   else
      pArea->hi_r_vnum = top_quest_vnum+vsize-1;
   pArea->hi_o_vnum = top_quest_vnum+vsize-1;
   pArea->hi_m_vnum = top_quest_vnum+vsize-1;
   pArea->low_soft_range = 0;
   pArea->hi_soft_range = MAX_LEVEL;
   pArea->low_hard_range = 0;
   pArea->hi_hard_range = MAX_LEVEL;
   pArea->kingdom = -1;
   area_version = 0;
   SET_BIT(pArea->status, AREA_LOADED);
   
   if (mission == 2)
   {
      vsize /= 4;
      mload *= 3;
   }
   
   room = make_room(vnum);
   room->area = pArea;
   room->sector_type = sector;
   
   if (xIS_SET(ch->in_room->room_flags, ROOM_TSAFE))
   xSET_BIT(room->room_flags, ROOM_SAFE);
   xSET_BIT(room->room_flags, ROOM_NO_MOB);
   xSET_BIT(room->room_flags, ROOM_WILDERNESS);
   gridmap[x][y] = 1;
   roomgrid[x][y] = room;
   randommapx[z] = x;
   randommapy[z] = y;
        
   for (cnt = 0; cnt <= vsize-2; cnt++)
   {
      rcnt++;
      if (rcnt > 50000) //Entered a nasty loop, delete the area out and return a failure
      {
         for (cnt = top_quest_vnum; cnt <= vnum-1; cnt++)
         {
            sprintf(vnumbuf, "%d", cnt);
            room = find_location(ch, vnumbuf);
            if (room)
               delete_room(room);   
         }
         DISPOSE(pArea);
         return 0;
      }
      if (first)
         first = 0;
      else
      {
         if (number_range(1, 10) >= 9 && mission != 2)
         {
            w = number_range(0, 19);
            if ((randommapx[w] != x || randommapy[w] != y) && (randommapx[w] != 0 && randommapy[w] != 0))
            {
               x = randommapx[w];
               y = randommapy[w];
            }
         }
      }            
      for (rdir = 0; rdir <= 30; rdir++)
      {
         if (number_range(1, 10) <= 8)
         {   
            xmod = x + xadd;
            ymod = y + yadd;
         }
         else
         {
            xadd = number_range(-1, 1);
            yadd = number_range(-1, 1);
            xmod = x + xadd;
            ymod = y + yadd;
         }
         if (gridmap[xmod][ymod] == 1)
            continue;
         if (xmod < 1 || xmod > 200)
            continue;
         if (ymod < 1 || ymod > 200)
            continue;
         x = xmod;
         y = ymod;
         break;
      }
      if (rdir == 31)
      {
         cnt--;
         continue;
      }
      else
      {
         room = make_room(++vnum);
         room->area = pArea;
         room->sector_type = sector;
         xSET_BIT(room->room_flags, ROOM_WILDERNESS);
         xSET_BIT(room->room_flags, ROOM_NO_RECALL);
         xSET_BIT(room->room_flags, ROOM_NO_SUMMON);
         xSET_BIT(room->room_flags, ROOM_NO_ASTRAL);
         xSET_BIT(room->room_flags, ROOM_DARK);
         gridmap[x][y] = 1;
         roomgrid[x][y] = room;
         if (z == 19)
            z = 0;
         else
            z++;
         randommapx[z] = x;
         randommapy[z] = y;   
      }
   }
   for (x = 1; x <= 200; x++)
   {
      for (y = 1; y <= 200; y++)
      {
         if (gridmap[x][y] == 1)
         {
            for (z = x-1; z <= x+1; z++)
            {
               for (w = y-1; w <= y+1; w++)
               {
                  if (gridmap[z][w] == 1 && (z != x || y != w))
                  {
                     if (z >= 1 && z <= 200 && w >= 1 && w <= 200)
                     {
                        if (mission == 2)
                        {
                           if (roomgrid[z][w]->vnum > roomgrid[x][y]->vnum+1 || roomgrid[z][w]->vnum < roomgrid[x][y]->vnum-1)
                              continue;
                        }
                        xit = make_exit(roomgrid[x][y], roomgrid[z][w], get_coord_dir(x-z, y-w));
                        xit->description = STRALLOC("");
                        xit->exit_info = 0;
                        if (mission == 2 && roomgrid[x][y]->vnum != pArea->low_r_vnum && roomgrid[z][w]->vnum != pArea->low_r_vnum)
                        {
                           xit->key = roomgrid[x][y]->vnum;
                           xit->keyword = STRALLOC("door");
                           SET_BIT(xit->exit_info, EX_ISDOOR);
                           SET_BIT(xit->exit_info, EX_CLOSED);
                           SET_BIT(xit->exit_info, EX_LOCKED);
                        }
                        else
                        {
                           xit->keyword = STRALLOC("");
                           xit->key = -1;
                       }
                     }
                  }
               }
            }
         }
      }
   }      
   x = number_range(10, 190);
   y = number_range(10, 190);
   xmod = x+10;
   ymod = y+10;
   w = y;
   for (; x <= xmod; x++) //Water
   {
      for (y = w; y <= ymod; y++)
      {
         if (gridmap[x][y] == 1)
         {
            roomgrid[x][y]->sector_type = 6;
         }
      }          
   }
   x = number_range(26, 175);
   y = number_range(26, 175);
   z = number_range(10, 25);
   w = y;
   xmod = x+z;
   ymod = y+z;
   if (sector == 2)
   {
      if (number_range(1, 2) == 1)
         sector = 13;
      else
         sector = 44;
   }
   if (sector == 13)
   {
      if (number_range(1, 2) == 1)
         sector = 2;
      else
         sector = 44;
   }
   if (sector == 44)
   {
      if (number_range(1, 2) == 1)
         sector = 13;
      else
         sector = 2;
   }
   for (x = x-z; x <= xmod; x++)
   {
      for (y = w-z; y <= ymod; y++)
      {
         if (gridmap[x][y] == 1)
         {
            roomgrid[x][y]->sector_type = sector;
         }
      }
   }
   for (x = 1; x <= 200; x++)
   {
      for (y = 1; y <= 200; y++)
      {
         if (gridmap[x][y] == 1)
         {
            set_quest_description(roomgrid[x][y]);
         }
      }
   }
   //Sometimes will have to delete a zone, no point in doing all this crap and removing it if it happens
   sprintf(vnumbuf, "Quest Zone %d", top_quest_vnum);
   room = get_room_index(pArea->low_r_vnum);
   room->sector_type = SECT_ROAD;
   pArea->name = str_dup(vnumbuf);
   pArea->author = STRALLOC("Rafermand");
   sprintf(vnumbuf, "questzone%d.are", top_quest_vnum);
   pArea->filename = str_dup(vnumbuf);
   LINK(pArea, first_area, last_area, next, prev);
   sort_area_by_name(pArea); /* 4/27/97 */
   sort_area(pArea, FALSE);
   top_area++;
   load_quest_mobs(pArea, difficulty, vsize, mload, mission);
   load_quest_groundloot(pArea, difficulty, vsize, mload);
   if (mission == 2)
      load_quest_keys(pArea);
   if (mission == 4)
      load_mob_tokens(pArea, difficulty);
   return 1;
}  

int load_static_quest_zone(CHAR_DATA *ch, int difficulty, int vsize, int mload, char *areaname, char *filename, int resettime)
{
   AREA_DATA *pArea;
   char vnumbuf[50];
   int randommapx[20];
   int randommapy[20];
   int x;
   int y;
   int xmod;
   int ymod;
   int xadd = 1;
   int yadd = 0;
   int sector;
   int z = 0;
   int cnt = 0;
   int rdir = 0;
   int w = 0;
   int rcnt = 0;
   int first = 1;
   ROOM_INDEX_DATA *room;
   int vnum = START_STATICQUEST_VNUM-1;
   EXIT_DATA *xit;
   
   for (pArea = first_area; pArea; pArea = pArea->next)
   {
      if (pArea->hi_r_vnum > vnum && pArea->hi_r_vnum <= END_STATICQUEST_VNUM)
         vnum = pArea->hi_r_vnum;
   }
   if (vnum + vsize > END_STATICQUEST_VNUM)
   {
      bug("load_static_quest_zone:  No more vnums to use");
      return 0;
   }
   vnum++;
   
   for (x = 1; x <= 200; x++)
   {
      for (y = 1; y <= 200; y++)
      {
         gridmap[x][y] = 0;
         roomgrid[x][y] = NULL;
      }
   }
   for (x = 0; x <= 19; x++)
   {
      randommapx[x] = 0;
      randommapy[x] = 0;
   }
   eqload = 0;
   x = number_range(1, 20);
   if (x >= 18) //Field
      sector = 2;
   else if (x >= 11) //Underground
      sector = 13;
   else //Path
      sector = 44;
     
   x = 100;
   y = 100;  
   
   CREATE(pArea, AREA_DATA, 1);
   pArea->first_reset = NULL;
   pArea->last_reset = NULL;
   pArea->age = 15;
   pArea->nplayer = 0;
   pArea->low_r_vnum = vnum;
   pArea->low_o_vnum = vnum;
   pArea->low_m_vnum = vnum;
   pArea->hi_r_vnum = vnum+vsize-1;
   pArea->hi_o_vnum = vnum+vsize-1;
   pArea->hi_m_vnum = vnum+vsize-1;
   pArea->low_soft_range = 0;
   pArea->hi_soft_range = MAX_LEVEL;
   pArea->low_hard_range = 0;
   pArea->hi_hard_range = MAX_LEVEL;
   pArea->kingdom = -1;
   pArea->reset_frequency = resettime;
   area_version = 0;
   SET_BIT(pArea->status, AREA_LOADED);
   
   room = make_room(vnum);
   room->area = pArea;
   room->sector_type = sector;
   
   if (xIS_SET(ch->in_room->room_flags, ROOM_TSAFE))
   xSET_BIT(room->room_flags, ROOM_SAFE);
   xSET_BIT(room->room_flags, ROOM_NO_MOB);
   xSET_BIT(room->room_flags, ROOM_WILDERNESS);
   gridmap[x][y] = 1;
   roomgrid[x][y] = room;
   randommapx[z] = x;
   randommapy[z] = y;
   
   for (cnt = 0; cnt <= vsize-2; cnt++)
   {
      rcnt++;
      if (rcnt > 50000) //Entered a nasty loop, delete the area out and return a failure
      {
         for (cnt = top_quest_vnum; cnt <= vnum-1; cnt++)
         {
            sprintf(vnumbuf, "%d", cnt);
            room = find_location(ch, vnumbuf);
            if (room)
               delete_room(room);   
         }
         DISPOSE(pArea);
         return 0;
      }
      if (first)
         first = 0;
      else
      {
         if (number_range(1, 10) >= 9)
         {
            w = number_range(0, 19);
            if ((randommapx[w] != x || randommapy[w] != y) && (randommapx[w] != 0 && randommapy[w] != 0))
            {
               x = randommapx[w];
               y = randommapy[w];
            }
         }
      }            
      for (rdir = 0; rdir <= 30; rdir++)
      {
         if (number_range(1, 10) <= 8)
         {   
            xmod = x + xadd;
            ymod = y + yadd;
         }
         else
         {
            xadd = number_range(-1, 1);
            yadd = number_range(-1, 1);
            xmod = x + xadd;
            ymod = y + yadd;
         }
         if (gridmap[xmod][ymod] == 1)
            continue;
         if (xmod < 1 || xmod > 200)
            continue;
         if (ymod < 1 || ymod > 200)
            continue;
         x = xmod;
         y = ymod;
         break;
      }
      if (rdir == 31)
      {
         cnt--;
         continue;
      }
      else
      {
         room = make_room(++vnum);
         room->area = pArea;
         room->sector_type = sector;
         xSET_BIT(room->room_flags, ROOM_WILDERNESS);
         xSET_BIT(room->room_flags, ROOM_NO_RECALL);
         xSET_BIT(room->room_flags, ROOM_NO_SUMMON);
         xSET_BIT(room->room_flags, ROOM_NO_ASTRAL);
         xSET_BIT(room->room_flags, ROOM_DARK);
         gridmap[x][y] = 1;
         roomgrid[x][y] = room;
         if (z == 19)
            z = 0;
         else
            z++;
         randommapx[z] = x;
         randommapy[z] = y;   
      }
   }
   for (x = 1; x <= 200; x++)
   {
      for (y = 1; y <= 200; y++)
      {
         if (gridmap[x][y] == 1)
         {
            for (z = x-1; z <= x+1; z++)
            {
               for (w = y-1; w <= y+1; w++)
               {
                  if (gridmap[z][w] == 1 && (z != x || y != w))
                  {
                     if (z >= 1 && z <= 200 && w >= 1 && w <= 200)
                     {
                        xit = make_exit(roomgrid[x][y], roomgrid[z][w], get_coord_dir(x-z, y-w));
                        xit->keyword = STRALLOC("");
                        xit->description = STRALLOC("");
                        xit->key = -1;
                        xit->exit_info = 0;
                     }
                  }
               }
            }
         }
      }
   }      
   x = number_range(10, 190);
   y = number_range(10, 190);
   xmod = x+10;
   ymod = y+10;
   w = y;
   for (; x <= xmod; x++) //Water
   {
      for (y = w; y <= ymod; y++)
      {
         if (gridmap[x][y] == 1)
         {
            roomgrid[x][y]->sector_type = 6;
         }
      }          
   }
   x = number_range(26, 175);
   y = number_range(26, 175);
   z = number_range(10, 25);
   w = y;
   xmod = x+z;
   ymod = y+z;
   if (sector == 2)
   {
      if (number_range(1, 2) == 1)
         sector = 13;
      else
         sector = 44;
   }
   if (sector == 13)
   {
      if (number_range(1, 2) == 1)
         sector = 2;
      else
         sector = 44;
   }
   if (sector == 44)
   {
      if (number_range(1, 2) == 1)
         sector = 13;
      else
         sector = 2;
   }
   for (x = x-z; x <= xmod; x++)
   {
      for (y = w-z; y <= ymod; y++)
      {
         if (gridmap[x][y] == 1)
         {
            roomgrid[x][y]->sector_type = sector;
         }
      }
   }
   for (x = 1; x <= 200; x++)
   {
      for (y = 1; y <= 200; y++)
      {
         if (gridmap[x][y] == 1)
         {
            set_quest_description(roomgrid[x][y]);
         }
      }
   }
   //Sometimes will have to delete a zone, no point in doing all this crap and removing it if it happens
   pArea->name = str_dup(areaname);
   pArea->filename = str_dup(filename);
   room = get_room_index(pArea->low_r_vnum);
   room->sector_type = SECT_ROAD;
   pArea->author = STRALLOC(ch->name);
   LINK(pArea, first_area, last_area, next, prev);
   sort_area_by_name(pArea); /* 4/27/97 */
   sort_area(pArea, FALSE);
   top_area++;
   load_quest_mobs(pArea, difficulty, vsize, mload, 1);
   load_quest_groundloot(pArea, difficulty, vsize, mload);
   //Install resets, instaroom works well for this
   for (x = pArea->low_r_vnum; x <= pArea->hi_r_vnum; x++)
   {
      room = get_room_index(x);
      if (room)
         instaroom(ch, pArea, room, TRUE, FALSE);
   }
   return 1;
}
int get_drating(int gsize, int mcount, int size, int difficulty, int timeload, int mission)
{
   int drating = 0;
   int mult;
   int diff = ((difficulty-1)/10)+1;
   
   if (diff == 1)
      drating = 2;
   if (diff == 2)
      drating = 5;
   if (diff == 3)
      drating = 10;
   if (diff == 4)
      drating = 20;
   if (diff == 5)
      drating = 50;
   if (diff == 6)
      drating = 100;
   if (diff == 7)
      drating = 250;
   if (diff == 8)
      drating = 500;
   if (diff == 9)
      drating = 1500;
   if (diff == 10)
      drating = 4000;
   if (diff == 11)
      drating = 10000;
   if (diff == 12)
      drating = 20000;
   if (diff == 13)
      drating = 50000;
      
   mult = mcount * 100 / 80;
   if (mult > 1000)
      mult = 1000;
   
      
   drating = drating * mult / 100;
   
   if (gsize == 2)
      drating = drating * 225/100;
   if (gsize == 3)
      drating = drating * 350/100;
   if (gsize == 4)
      drating = drating * 500/100;
   if (gsize == 5)
      drating = drating * 650/100;
   if (gsize == 6)
      drating = drating * 800/100;
      
   drating = drating * 3 / 2;
   
   drating = drating * sysdata.exp_percent /100;
      
   if (timeload > 0)
      drating /= timeload;
   else
      return 1;
      
   if (mission != 1)
      drating = drating * 2 / 3;
   return UMAX(1, drating);
}

char *get_quest_type(int type)
{  
   if (type == 1)
      return "Slaughter";
   if (type == 2)
      return "Gauntlet";
   if (type == 3)
      return "Boss";
   if (type == 4)
      return "Collect";
      
   return "????";
}

void load_queststruct_data(CHAR_DATA *ch, int size, int mission, int mcount, int time, int difficulty, int timeload)
{
   CHAR_DATA *victim;
   QUEST_DATA *quest;
   ROOM_INDEX_DATA *room;
   EXIT_DATA *pexit;
   int gsize = 1;
   AREA_DATA *tarea;
   int x = -1;
   int dir;
   int drating;
   int diff = ((difficulty-1)/10)+1;
   
   CREATE(quest, QUEST_DATA, 1);
   LINK(quest, first_quest, last_quest, next, prev);
   quest->mission = mission;
   if (quest->mission == 2 || quest->mission == 3)
      quest->tokill = -1;
   else if (quest->mission == 4)
      quest->tokill = number_range(mcount*30/100, mcount*35/100); 
   else
      quest->tokill = number_range(mcount*60/100, mcount*70/100); 
   quest->killed = 0;
   quest->tillnew = -1;
   quest->timeleft = time;
   quest->traveltime = 20;
   quest->difficulty = difficulty;
   ch->pcdata->quest = quest;
   if (quest->mission == 2 || quest->mission == 3)
      ch_printf(ch, "You have received a %s quest at difficulty %d.\n\rYou have %d minutes to finish it.  Good luck.\n\r", 
         get_quest_type(quest->mission), diff, quest->timeleft);
   else if (quest->mission == 4)
      ch_printf(ch, "You have received a %s quest to collect %d tokens at difficulty %d.\n\rYou have %d minutes to finish it.  Good luck.\n\r", 
         get_quest_type(quest->mission), quest->tokill, diff, quest->timeleft);
   else
      ch_printf(ch, "You have received a %s quest of %d mobs at difficulty %d.\n\rYou have %d minutes to finish it.  Good luck.\n\r", 
         get_quest_type(quest->mission), quest->tokill, diff, quest->timeleft);
   for (victim = first_char; victim; victim = victim->next)
   {
      if (is_same_group(victim, ch) && victim != ch && !IS_NPC(victim))
      {
         gsize++;
      }
   }
   drating = get_drating(gsize, mcount, size, difficulty, timeload, mission);
   quest->player[++x] = ch->pcdata->pid;
   quest->qp[x] = UMAX(1,drating/gsize);
   if (gsize > 1)
   {
      for (victim = first_char; victim; victim = victim->next)
      {
         if (is_same_group(victim, ch) && victim != ch && !IS_NPC(victim))
         {
            quest->player[++x] = victim->pcdata->pid;
            quest->qp[x] = UMAX(1, drating/gsize);
            victim->pcdata->quest = quest;
            ch_printf(victim, "You have received a %s quest of %d mobs at difficulty %d.  Good luck.\n\r", 
               get_quest_type(quest->mission), quest->tokill, diff);
         }
      }
   }
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (tarea->low_r_vnum == top_quest_vnum)
      {
         quest->questarea = tarea;
         break;
      }
   }
   if (diff <= 2)
   {
      quest->x = 790;
      quest->y = 323;
      quest->map = MAP_SOLAN;
   }
   else if (diff <= 5)
   {
      quest->x = 817;
      quest->y = 359;
      quest->map = MAP_SOLAN;
   }
   else if (diff <= 8)
   {
      quest->x = 878;
      quest->y = 393;
      quest->map = MAP_SOLAN;
   }
   else if (diff <= 10)
   {
      quest->x = 855;
      quest->y = 445;
      quest->map = MAP_SOLAN;
   }
   else
   {
      quest->x = 715;
      quest->y = 457;
      quest->map = MAP_SOLAN;
   }
   room = get_room_index(tarea->low_r_vnum);
   for (dir = 0; dir <= 9; dir++)
   {
      if (dir == 4 || dir == 5)
         continue;
      if ((pexit = get_exit(room, dir)) == NULL && get_room_index(OVERLAND_SOLAN))
      {
         pexit = make_exit(room, get_room_index(OVERLAND_SOLAN), dir);
         pexit->keyword = STRALLOC("");
         pexit->description = STRALLOC("");
         pexit->key = -1;
         pexit->exit_info = 0;
         SET_BIT(pexit->exit_info, EX_OVERLAND);
         pexit->coord->x = quest->x;
         pexit->coord->y = quest->y;
         break;
      }
   }
   if (dir == 10)
   {
      bug("An exit could not be created in room %d", tarea->low_r_vnum);
   }

   return;
}
   
/*Timeload 1  - Normal time
           >1 - Multiplier of time that will be / into quest points
           -1 - All the time you need, 1 QPS, impossible to win though
*/
int get_finishtime(int size, int mcount, int difficulty, int timeload, int mission)
{
   int time;
   int diff = ((difficulty-1)/10)+1;
   
   if (size <= 50)
      time = 30;
   else if (size <= 100)
      time = 60;
   else if (size <= 150)
      time = 90;
   else if (size <= 200)
      time = 120;
   else if (size <= 250)
      time = 150;   
   else
      time = 180;
      
   if (mcount <= 80)
      time = time * 80/100;
   else if (mcount <= 110)
      time = time * 95/100;
   else if (mcount <= 140)
      time = time * 115/100;
   else if (mcount <= 170)
      time = time * 150/100;
   else if (mcount <= 200)
      time = time * 175/100;
   else
      time = time * 200/100;      
      
   if (diff == 8)
      time = time * 125/100;
   else if (diff == 9 || diff == 10)
      time = time * 135/100;
   else if (diff == 11)
      time = time * 165/100;
   else if (diff == 12)
      time = time * 200/100;
   else if (diff == 13)
      time = time * 300/100;
      
   if (timeload > 0)
      time *= timeload;
   else
      time = 20160; //2 weeks should be enough time...
      
   if (mission == 2)
      return time * 2 / 3;
      
   return time;
}

void do_unloadqarea(CHAR_DATA *ch, char *argument)
{
   AREA_DATA *pArea;
   
   if (argument[0] == '\0')
   {
      send_to_char("unloadqarea <area file name>\n\r", ch);
      return;
   }
   for (pArea = first_area; pArea; pArea = pArea->next)
   {
      if (!str_cmp(pArea->filename, argument))
         break;
   }
   if (!pArea)
   {
      send_to_char("Could not find the area you specified.\n\r", ch);
      return;
   }
   if (pArea->low_r_vnum < START_STATICQUEST_VNUM || pArea->low_r_vnum > END_STATICQUEST_VNUM)
   {
      send_to_char("This command is for unloading static quest areas only.\n\r", ch);
      return;
   }
   disposequestarea(pArea, -1);
   send_to_char("Done.\n\r", ch);
   return;
}

//Removes the area from the game, dumps any players outside and corpses go to the morgue
void disposequestarea(AREA_DATA *tarea, int difficulty)
{
   CHAR_DATA *ch;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *iobj;
   MOB_INDEX_DATA *imob;
   int x;
   CHAR_DATA *vnext;
   OBJ_DATA *obj_next;
   ROOM_INDEX_DATA *room;
   RESET_DATA *reset;
   int diff = ((difficulty-1)/10)+1;
   
   for (ch = first_char; ch; ch = ch->next)
   {
      if (IS_NPC(ch))
         continue;
      if (!ch->in_room)
         continue;
      if (ch->in_room->area != tarea)
         continue;
      if (diff < 1)
      {
         char_from_room(ch);
         char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
      }
      else if (diff <= 2)
      {
         ch->coord->x = 790;
         ch->coord->y = 323;
         ch->map = MAP_SOLAN;
         SET_ONMAP_FLAG(ch);
         char_from_room(ch);
         char_to_room(ch, get_room_index(OVERLAND_SOLAN));
      }
      else if (diff <= 5)
      {
         ch->coord->x = 817;
         ch->coord->y = 359;
         ch->map = MAP_SOLAN;
         SET_ONMAP_FLAG(ch);
         char_from_room(ch);
         char_to_room(ch, get_room_index(OVERLAND_SOLAN));
      }
      else if (diff <= 8)
      {
         ch->coord->x = 878;
         ch->coord->y = 393;
         ch->map = MAP_SOLAN;
         SET_ONMAP_FLAG(ch);
         char_from_room(ch);
         char_to_room(ch, get_room_index(OVERLAND_SOLAN));
      }
      else if (diff <= 10)
      {
         ch->coord->x = 855;
         ch->coord->y = 445;
         ch->map = MAP_SOLAN;
         SET_ONMAP_FLAG(ch);
         char_from_room(ch);
         char_to_room(ch, get_room_index(OVERLAND_SOLAN));
      }
      else
      {
         ch->coord->x = 715;
         ch->coord->y = 457;
         ch->map = MAP_SOLAN;
         SET_ONMAP_FLAG(ch);
         char_from_room(ch);
         char_to_room(ch, get_room_index(OVERLAND_SOLAN));
      }
      if (ch->pcdata->mount)
      {
         if (ch->pcdata->mount->in_room->area == tarea)
         {
            ch->pcdata->mount->coord->x = ch->coord->x;
            ch->pcdata->mount->coord->y = ch->coord->y;
            ch->pcdata->mount->map = ch->map;
            SET_ONMAP_FLAG(ch->pcdata->mount);
            char_from_room(ch->pcdata->mount);
            char_to_room(ch->pcdata->mount, get_room_index(OVERLAND_SOLAN));
         }
      }
   }
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->item_type != ITEM_CORPSE_PC)
         continue;
      if (!obj->in_room)
         continue;
      if (obj->in_room->area != tarea)
         continue;
      obj_from_room(obj);
      obj_to_room(obj, get_room_index(VNUM_ROOM_MORGUE), NULL);
   }
   for (x = tarea->low_r_vnum; x <= tarea->hi_r_vnum; x++)
   {
      room = get_room_index(x);
      if (!room)
      {
         bug("disposequestarea: Invalid room vnums of %d", x);
         continue;
      }
      for (ch = room->first_person; ch; ch = vnext)
      {
         vnext = ch->next_in_room;

         if (IS_NPC(ch))
            extract_char(ch, TRUE);
      }

      for (obj = room->first_content; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         extract_obj(obj);
      }
      delete_room(room);
   }
   for (x = tarea->low_o_vnum; x <= tarea->hi_o_vnum; x++)
   {
      iobj = get_obj_index(x);
      if (!iobj)
         continue;
      delete_obj(iobj);
   }
   for (x = tarea->low_m_vnum; x <= tarea->hi_m_vnum; x++)
   {
      imob = get_mob_index(x);
      if (!imob)
         continue;
      delete_mob(imob);
   }

   for (reset = tarea->first_reset; reset;)
   {
      /* Resets always go forward, so we can safely use the previous reset,
         providing it exists, or first_reset if it doesnt.  -- Altrag */
      RESET_DATA *prev = reset->prev;
      delete_reset(tarea, reset);
      reset = (prev ? prev->next : tarea->first_reset);
   }
   delete_area(tarea);
   return;
}
  
void do_morgue(CHAR_DATA *ch, char *argument)
{
   int qp;
   int sworth = player_stat_worth(ch);
   OBJ_DATA *obj;
   char name[MIL];
   char *pd;
   int fnd = 0;
   char arg[MIL];
   
   if (check_npc(ch))
      return;
   if (ch->in_room->vnum != VNUM_ROOM_MORGUE)
   {
      send_to_char("You can only use this command in the morgue.\n\r", ch);
      return;
   }
   if (sworth >= 300000)
      qp = 750;
   else if (sworth >= 225000)
      qp = 250;
   else if (sworth >= 150000)
      qp = 125;
   else if (sworth >= 75000)
      qp = 62;
   else if (sworth >= 35000)
      qp = 20;
   else if (sworth >= 25000)
      qp = 10;
   else if (sworth >= 15000)
      qp = 4;
   else if (sworth >= 7500)
      qp = 2;
   else
      qp = 1;
   
   if (argument[0] == '\0')
   {
      send_to_char("morgue retrieve yes\n\r", ch);
      send_to_char("morgue cost\n\r", ch);
      send_to_char("morgue newbie\n\r", ch);
      send_to_char("morgue quest\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "cost"))
   {
      ch_printf(ch, "It will cost you %d QPS to have your corpse retrieved.\n\r", qp);
      return;
   }
   if (!str_cmp(arg, "newbie"))
   {
      if (ch->max_hit > 100)
      {
         ch_printf(ch, "You can only use the morgue if you have 100 hp or less.\n\r", qp);
         return;
      }
      for (obj = first_object; obj; obj = obj->next)
      {
         if (obj->item_type != ITEM_CORPSE_PC)
            continue;
         if (!obj->in_room)
            continue;
         if (obj->in_room->vnum == VNUM_ROOM_MORGUE)
            continue;
         pd = obj->short_descr;
         pd = one_argument(pd, name);
         pd = one_argument(pd, name);
         pd = one_argument(pd, name);
         pd = one_argument(pd, name); 
         if (!str_cmp(name, ch->name))
         {
            obj_from_room(obj);
            update_container(obj, ch->coord->x, ch->coord->y, ch->map, 0, 0, 0);
            obj_to_room(obj, get_room_index(VNUM_ROOM_MORGUE), ch);
            fnd++;
         }
      }
      if (fnd == 0)
      {
         send_to_char("No corpse could be found.\n\r", ch);
         return;
      }
      else
      {
         ch->max_hit = UMAX(20, ch->max_hit-5);
         send_to_char("All corpses are now in this room, it costed you 5 HP.\n\r", ch);
         return;
      }     
   }
   if (!str_cmp(arg, "retrieve"))
   {
      if (ch->max_hit < 101)
      {
         ch_printf(ch, "You can only use retreive if you have more than 100 hp.\n\r", qp);
         return;
      }
      if (str_cmp(argument, "yes"))
      {
         send_to_char("To protect from a mistype or error, yes is required.  This command will take 5% of your hp.\n\r", ch);
         return;
      }
      for (obj = first_object; obj; obj = obj->next)
      {
         if (obj->item_type != ITEM_CORPSE_PC)
            continue;
         if (!obj->in_room)
            continue;
         if (obj->in_room->vnum == VNUM_ROOM_MORGUE)
            continue;
         pd = obj->short_descr;
         pd = one_argument(pd, name);
         pd = one_argument(pd, name);
         pd = one_argument(pd, name);
         pd = one_argument(pd, name); 
         if (!str_cmp(name, ch->name) && obj->timer <= 2865 && obj->timer > 5)
         {
            obj_from_room(obj);
            update_container(obj, ch->coord->x, ch->coord->y, ch->map, 0, 0, 0);
            obj_to_room(obj, get_room_index(VNUM_ROOM_MORGUE), ch);
            fnd++;
         }
      }
      if (fnd == 0)
      {
         send_to_char("No corpse could be found.\n\r", ch);
         return;
      }
      else
      {
         ch_printf(ch, "All corpses are now in this room, it has costed you %d HP.", ch->max_hit * 5/100);
         ch->max_hit -= ch->max_hit * 5/100;
         return;
      }     
   }
   if (!ch->pcdata->quest)
   {
      send_to_char("You can only use this while you are on a quest.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "quest"))
   {
      if (ch->pcdata->quest_curr < qp)
      {
         ch_printf(ch, "It will cost you %d QPS to do that and you don't have it.\n\r", qp);
         return;
      }
      for (obj = first_object; obj; obj = obj->next)
      {
         if (obj->item_type != ITEM_CORPSE_PC)
            continue;
         if (!obj->in_room)
            continue;
         if (obj->in_room->area != ch->pcdata->quest->questarea)
            continue;
         pd = obj->short_descr;
         pd = one_argument(pd, name);
         pd = one_argument(pd, name);
         pd = one_argument(pd, name);
         pd = one_argument(pd, name); 
         if (!str_cmp(name, ch->name) && obj->timer > 5)
         {
            obj_from_room(obj);
            obj_to_room(obj, get_room_index(VNUM_ROOM_MORGUE), ch);
            update_container(obj, ch->coord->x, ch->coord->y, ch->map, 0, 0, 0);
            fnd++;
         }
      }
      if (fnd == 0)
      {
         send_to_char("No corpse could be found.\n\r", ch);
         return;
      }
      else
      {
         ch->pcdata->quest_curr -= qp;
         ch_printf(ch, "All corpses in a quest are now in this room.  Cost was %d QPS\n\r", qp);
         return;
      }   
   }
   do_morgue(ch, "");
   return;
}     

int get_qp_forge(OBJ_INDEX_DATA *wobj, SLAB_DATA *slab, int power)
{
   int qps;   
   qps = slab->qps;
   qps = slab->qps * UMAX(1, wobj->weight/3);
   if (power == 2)
      qps *= 3;
   if (power == 3)
      qps *= 10;
   return qps;
}

char *get_name_pid(int pid)
{
   DESCRIPTOR_DATA *d;
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->character && d->character->name && d->character->pcdata && d->character->pcdata->pid)
      {
         if (d->character->pcdata->pid == pid)
            return d->character->name;
      }
   }
   return "------";
}

void do_roll(CHAR_DATA *ch, char *argument)
{
   int low;
   int high;
   int roll;
   
   char arg[MIL];
   char buf[MSL];
   argument = one_argument(argument, arg);
   low = atoi(arg);
   high = atoi(argument);
   
   if (atoi(arg) > atoi(argument))
   {
      high = atoi(arg);
      low = atoi(argument);
   }
   else
   {
      high = atoi(argument);
      low = atoi(arg);
   }
   if (low == 0 && high == 0)
   {
      send_to_char("Syntax:  roll <low num> <hi num>\n\r", ch);
      return;
   }
   roll = number_range(low, high);
   sprintf(buf, "[ROLL] $n rolls a number between %d and %d and the result is: %d", low, high, roll);
   act(AT_WHITE, buf, ch, NULL, NULL, TO_ROOM);
   sprintf(buf, "[ROLL] You roll a number between %d and %d and the result is: %d", low, high, roll);
   act(AT_WHITE, buf, ch, NULL, NULL, TO_CHAR);
   return;
}

void find_tokens_container(int *cost, int *count, OBJ_DATA *container)
{
   OBJ_DATA *token;
   OBJ_DATA *next_token;
         
   for (token = container->first_content; token; token = next_token)
   {
      next_token = token->next_content;
      if (token->item_type == ITEM_QTOKEN)
      {
         *cost += token->count * token->cost;
         *count += token->count;
         obj_from_obj(token);
         extract_obj(token);
      }
      if (token->first_content)
      {
         find_tokens_container(cost, count, token);
      }
   }
}
void do_loadquest(CHAR_DATA *ch, char *argument)
{
   int success;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   char arg6[MIL];
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *iobj;
   int autol = 0;
   int size = 0;
   int sworth;
   int mcount = 0;
   int gsize = 1;
   QOBJ_DATA *qobj;
   CHAR_DATA *victim;
   QUEST_DATA *quest;
   int difficulty;
   int losworth;
   int hisworth;
   int omcount;
   int qtype = 0;
   
   if (check_npc(ch))
      return;
   losworth = player_stat_worth(ch);
   hisworth = player_stat_worth(ch);
   
   if (argument[0] == '\0')
   {
      send_to_char("quest <difficulty> <mission> <size> <mob count> [extra time (-1 to 10)]\n\r", ch);
      send_to_char("quest <simple/easy/easy-medium/medium/medium-hard/hard>\n\r", ch);
      send_to_char("quest status\n\r", ch);
      send_to_char("quest fail\n\r", ch);
      send_to_char("quest obj diff <diff lo> <diff hi>\n\r", ch);
      send_to_char("quest obj qps <qps low> <qps high>\n\r", ch);
      send_to_char("quest obj type <type (1-15)> [qps low] [qps hi]\n\r", ch);
      send_to_char("quest obj buy <number>\n\r", ch);
      send_to_char("quest obj forge item <item #> [ore #]\n\r", ch);
      send_to_char("quest obj forge ore <ore #>\n\r", ch);
      send_to_char("quest obj forge buy <item #> <ore #> <power>\n\r", ch);
      send_to_char("quest sell tokens\n\r", ch);
      if (IS_IMMORTAL(ch))
      {
         send_to_char("quest view [quest num]\n\r", ch);
         send_to_char("quest static <difficulty> <vnums> <mob count> <area name> <filename> <reset time>\n\r", ch);
      }
      ch_printf(ch, "    difficulty <1-%d>     mission <slaughter gauntlet boss collect>\n\r", MAX_QDIFF);
      send_to_char("    size <1-6>            mob count <1-6>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   argument = one_argument(argument, arg4);
   
   if (top_quest_vnum == 0)
      top_quest_vnum = 100000;
      
   if (!str_cmp(arg1, "view") && IS_IMMORTAL(ch))
   {
      if (arg2[0] == '\0')
      {
         send_to_char("Num   Dif  Time  Player1       Player2       Player3       Player4       Player5       Player6\n\r------------------------------------------------------------------------------------------------\n\r", ch);
         for (quest = first_quest; quest; quest = quest->next)
         {
            ch_printf(ch, "[%-2d]  %-2d   %-3d   %-12.12s  %-12.12s  %-12.12s  %-12.12s  %-12.12s  %-12.12s\n\r", ++size,
               quest->difficulty, quest->timeleft,
               get_name_pid(quest->player[0]), get_name_pid(quest->player[1]), get_name_pid(quest->player[2]), 
               get_name_pid(quest->player[3]), get_name_pid(quest->player[4]), get_name_pid(quest->player[5]));
         }
      }
      else
      {
         if (atoi(arg2) <= 0)
         {
            send_to_char("You need to choose a number from quest view.\n\r", ch);
            return;
         }
         for (quest = first_quest; quest; quest = quest->next)
         {
            if (++size == atoi(arg2))
            {
               ch_printf(ch, "&w&WDifficulty: &w&C%-2d                 &w&WKill:  &w&C%4d/%-4d               &w&WMission: &w&C%d    &w&WCoords: &w&C%dx%dy\n\r", 
                  quest->difficulty, quest->killed, quest->tokill, quest->mission, quest->x, quest->y); 
               ch_printf(ch, "&w&WTraveltime:  &w&C%d                &w&WTimeleft:  &w&C%d                  &w&WTillnew:  &w&C%d\n\r", quest->traveltime, 
                  quest->timeleft, quest->tillnew);
               ch_printf(ch, "&w&W--------------------------------------------------------------------------------------------------\n\r");
               ch_printf(ch, "&w&WPlayer1: &w&C%12.12s/%-5d    &w&WPlayer2: &w&C%12.12s/%-5d    &w&WPlayer3: &w&C%12.12s/%-5d\n\r",
                  get_name_pid(quest->player[0]), quest->qp[0], get_name_pid(quest->player[1]), quest->qp[1],
                  get_name_pid(quest->player[2]), quest->qp[2]);
               ch_printf(ch, "&w&WPlayer4: &w&C%12.12s/%-5d    &w&WPlayer5: &w&C%12.12s/%-5d    &w&WPlayer6: &w&C%12.12s/%-5d\n\r",
                  get_name_pid(quest->player[3]), quest->qp[3], get_name_pid(quest->player[4]), quest->qp[4],
                  get_name_pid(quest->player[5]), quest->qp[5]);
               return;
            }
         }
         send_to_char("That is not a valid selection.\n\r", ch);
      }
      return;
   }
   
   if (!str_cmp(arg1, "sell"))
   {
      if (!str_cmp(arg2, "tokens"))
      {
         OBJ_DATA *token;
         OBJ_DATA *next_token;
         int cost = 0;
         int count = 0;
         
         for (token = ch->first_carrying; token; token = next_token)
         {
            next_token = token->next_content;
            if (token->item_type == ITEM_QTOKEN)
            {
               cost += token->count * token->cost;
               count += token->count;
               obj_from_char(token);
               extract_obj(token);
            }
            if (token->first_content)
            {
               find_tokens_container(&cost, &count, token);
            }
         }
         ch_printf(ch, "You sell %d tokens for %d coins\n\r", count, cost);
         ch->gold += cost;
         return;
      }
   }   
                 
   if (!str_cmp(arg1, "obj"))
   {
      if (!str_cmp(arg2, "forge"))
      {
         SLAB_DATA *slab = NULL;
         SLAB_DATA *sslab;
         int cnt = 1;
         FORGE_DATA *forge = NULL;
         FORGE_DATA *sforge;
         OBJ_INDEX_DATA *wobj;
         
         if (!str_cmp(arg3, "buy"))
         {
            argument = one_argument(argument, arg5);
           
            for (forge = first_forge; forge; forge = forge->next)
            {
               if (cnt++ == atoi(arg4))
                  break;
            }
            if (!forge)
            {
               send_to_char("That is not a valid item, type forge list for a list.\n\r", ch);
               return;
            }
            cnt = 1;
            for (slab = first_slab; slab; slab = slab->next) 
            {
               if (cnt++ == atoi(arg5))
                  break;
            }
            if (!slab)
            {
               send_to_char("The slab # you specified is not valid, check forge ores for a list.\n\r", ch);
               return;
            }
            if (atoi(argument) < 1 || atoi(argument) > 3)
            {
               send_to_char("Power ranges from 1 to 3.\n\r", ch);
               return;
            }
            wobj = get_obj_index(forge->vnum);
            if (get_qp_forge(wobj, slab, atoi(argument)) > ch->pcdata->quest_curr)
            {
               send_to_char("You do not have enough QPS for that.\n\r", ch);
               return;
            }
            adjust_questobj(ch, NULL, 1000+atoi(arg5), NULL, 0, atoi(argument)-1, forge->vnum);
            send_to_char("Done.\n\r", ch);
            ch->pcdata->quest_curr -= get_qp_forge(wobj, slab, atoi(argument));
            return;
         }   
         
         if (!str_cmp(arg3, "ore"))
         {
            for (slab = first_slab; slab; slab = slab->next) 
            {
               if (cnt++ == atoi(arg4))
                  break;
            }
            if (!slab)
            {
               send_to_char("The slab # you specified is not valid, check forge ores for a list.\n\r", ch);
               return;
            }

            if (atoi(argument) > 0)
            {
               cnt = 1;
               for (forge = first_forge; forge; forge = forge->next)
               {
                  if (cnt++ == atoi(argument))
                     break;
               }
               if (!forge)
               {
                  send_to_char("That is not a valid item, type forge list for a list.\n\r", ch);
                  return;
               }
            }
            send_to_char("Item                       Slab          Power1   Power2   Power3\n\r----------------------------------------------------------------------------\n\r", ch);
            for (sforge = first_forge; sforge; sforge = sforge->next)
            {
               if (forge && forge != sforge)
                  continue;
               else
               {
                  wobj = get_obj_index(sforge->vnum);
                  ch_printf(ch, "%-25s  %-12s  %-7d  %-7d  %-7d\n\r", wobj->name, slab->adj, get_qp_forge(wobj, slab, 1), 
                     get_qp_forge(wobj, slab, 2), get_qp_forge(wobj, slab, 3));
               }
            }
            return;
         }
         if (!str_cmp(arg3, "item"))
         {
            for (forge = first_forge; forge; forge = forge->next)
            {
               if (cnt++ == atoi(arg4))
                  break;
            }
            if (!forge)
            {
               send_to_char("That is not a valid item, type forge list for a list.\n\r", ch);
               return;
            }
            if (atoi(argument) > 0)
            {
               cnt = 1;
               for (slab = first_slab; slab; slab = slab->next) 
               {
                  if (cnt++ == atoi(argument))
                     break;
               }
               if (!slab)
               {
                  send_to_char("The slab # you specified is not valid, check forge ores for a list.\n\r", ch);
                  return;
               }
            }
            send_to_char("Item                       Slab          Power1   Power2   Power3\n\r----------------------------------------------------------------------------\n\r", ch);
            for (sslab = first_slab; sslab; sslab = sslab->next)
            {
               if (slab && slab != sslab)
                  continue;
               else
               {
                  wobj = get_obj_index(forge->vnum);
                  ch_printf(ch, "%-25s  %-12s  %-7d  %-7d  %-7d\n\r", wobj->name, sslab->adj, get_qp_forge(wobj, sslab, 1), 
                     get_qp_forge(wobj, sslab, 2), get_qp_forge(wobj, sslab, 3));
               }
            }
            return;
         }
      }
                     
            
      if (!str_cmp(arg2, "diff"))
      {
         if (atoi(arg3) > atoi(arg4))
         {
            send_to_char("Your low diff has to be lower than your hi diff.\n\r", ch);
            return;
         }
         send_to_char("         Name                                        Lo  Hi  Racial  QPS\n\r--------------------------------------------------------------------------\n\r", ch);                            
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            mcount++;
            if (is_in_range(qobj->lowdiff, qobj->hidiff, atoi(arg3), atoi(arg4)) && qobj->qps > 0)
            {
               ch_printf(ch, "[%-4d] > %-42s  %-2d  %-2d  %-3s     %d\n\r", mcount, qobj->name, qobj->lowdiff, qobj->hidiff, qobj->race ? "Yes" : "No", qobj->qps);
            }
         }
         return;
         
      }
      if (!str_cmp(arg2, "qps"))
      {
         if (atoi(arg3) > atoi(arg4))
         {
            send_to_char("Your low qps has to be lower than your hi qps.\n\r", ch);
            return;
         }
         send_to_char("         Name                                        Lo  Hi  Racial  QPS\n\r--------------------------------------------------------------------------\n\r", ch);   
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            mcount++;
            if (qobj->qps >= atoi(arg3) && qobj->qps <= atoi(arg4) && qobj->qps > 0)
            {
               ch_printf(ch, "[%-4d] > %-42s  %-2d  %-2d  %-3s     %d\n\r", mcount, qobj->name, qobj->lowdiff, qobj->hidiff, qobj->race ? "Yes" : "No", qobj->qps);
            }
         }
         return;
      }
      if (!str_cmp(arg2, "type"))
      {
         if (atoi(arg3) < 1 || atoi(arg3) > 15)
         {
            send_to_char("Type ranges from 1 to 15.\n\r", ch);
            return;
         }
         send_to_char("         Name                                        Lo  Hi  Racial  QPS\n\r--------------------------------------------------------------------------\n\r", ch);   
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            mcount++;
            if (atoi(arg4) > 0 && atoi(argument) > 0 && atoi(arg4) <= atoi(argument))
            {
               if (is_in_range(qobj->lowdiff, qobj->hidiff, atoi(arg4), atoi(argument)) && qobj->type == atoi(arg3) && qobj->qps > 0)
               {
                  ch_printf(ch, "[%-4d] > %-42s  %-2d  %-2d  %-3s     %d\n\r", mcount, qobj->name, qobj->lowdiff, qobj->hidiff, qobj->race ? "Yes" : "No", qobj->qps);
               }
            }
            else
            {
               if (qobj->type == atoi(arg3) && qobj->qps > 0)
               {
                  ch_printf(ch, "[%-4d] > %-42s  %-2d  %-2d  %-3s     %d\n\r", mcount, qobj->name, qobj->lowdiff, qobj->hidiff, qobj->race ? "Yes" : "No", qobj->qps);
               }
            }
         }
         return;
      }
      if (!str_cmp(arg2, "buy"))
      {
         int extract = 0;
         if (atoi(arg3) <= 0)
         {
            send_to_char("You need to choose the number you find from using the list commands.\n\r", ch);
            return;
         }
         for (qobj = first_qobj; qobj; qobj = qobj->next)
         {
            if (++mcount == atoi(arg3))
               break;
         }
         if (!qobj)
         {
            send_to_char("That number is not valid.\n\r", ch);
            return;
         }
         if (qobj->qps > ch->pcdata->quest_curr)
         {
            send_to_char("You do not have enough QPS for that.\n\r", ch);
            return;
         }
         if (qobj->qps <= 0)
         {
            send_to_char("That is not available for purchase.\n\r", ch);
            return;
         }
         iobj = get_obj_index(OBJ_VNUM_QUESTOBJ);
         obj = create_object(iobj, 1);
         adjust_questobj(ch, obj, -1, qobj, 0, 0, 0);
         if (get_ch_carry_number(ch) + (get_obj_number(obj) / obj->count) > can_carry_n(ch))
         {
            send_to_char("You cannot carry it.\n\r", ch);
            extract = 1;
         }

         if (get_ch_carry_weight(ch) + (get_obj_weight(obj) / obj->count) > can_carry_w(ch))
         {
            send_to_char("You cannot handle its weight.\n\r", ch);
            extract = 1;
         }
         if (IS_UNIQUE(ch, obj))
         {
            send_to_char("That item is unique and you already have one.\n\r", ch);
            extract = 1;
         }
         if (extract == 1)
         {
            extract_obj(obj);
            return;
         }
         else
         {
            obj_to_char(obj, ch);
            ch->pcdata->quest_curr -= qobj->qps;
            send_to_char("Done.\n\r", ch);
            return;
         }
      }
   }
         
   if (!str_cmp(arg1, "fail"))
   {
      int reduce = 0;
      if (!ch->pcdata->quest)
      {
         send_to_char("You have to have a quest to fail it.\n\r", ch);
         return;
      }
      if (ch->pcdata->quest->tillnew <= 5 && ch->pcdata->quest->timeleft == -1 && ch->pcdata->quest->traveltime == -1)
      {
         send_to_char("You have to have a quest to fail it.\n\r", ch);
         return;
      }
      if (ch->pcdata->quest->player[0] != ch->pcdata->pid)
      {
         send_to_char("You have to be the one that initiated it to fail it.\n\r", ch);
         return;
      }
      if (ch->pcdata->quest->timeleft > -1)
      {
         send_to_char("You have failed your quest.\n\r", ch);
         ch->pcdata->quest_losses++;
      }
      else
      {
         send_to_char("Reducing your quest wait to 5 minutes.\n\r", ch);
         reduce = 1;
      }
      ch->pcdata->quest->timeleft = -1;
      ch->pcdata->quest->traveltime = -1;
      ch->pcdata->quest->tillnew = 5;
      for (victim = first_char; victim; victim = victim->next)
      {
         if (IS_NPC(victim))
            continue;
         if (victim == ch)
            continue;
         if (!victim->pcdata->quest)
            continue;
         if (victim->pcdata->quest != ch->pcdata->quest)
            continue;
         victim->pcdata->quest_losses++;
         if (!reduce)
         {
            send_to_char("You have failed your quest.\n\r", victim);
            victim->pcdata->quest_losses++;
         }
         else
            send_to_char("Reducing your quest wait to 5 minutes.\n\r", ch);
      }
      return;
   }
   if (!str_cmp(arg1, "status"))
   {
      if (!ch->pcdata->quest)
      {
         send_to_char("You aren't currently involved in any quests.\n\r", ch);
      }
      else
      {
         if (ch->pcdata->quest->tillnew > -1)
         {
            ch_printf(ch, "Remaining Time Till new Quest:   %d Minutes\n\r", ch->pcdata->quest->tillnew);
         }
         else
         {
            ch_printf(ch, "Quest Type:        %s\n\r", get_quest_type(ch->pcdata->quest->mission));
            if (ch->pcdata->quest->traveltime > -1)
               ch_printf(ch, "Quest TravelTime:  %d Minutes\n\r", ch->pcdata->quest->traveltime);
            else
               ch_printf(ch, "Quest Time:        %d Minutes\n\r", ch->pcdata->quest->timeleft);
            if (ch->pcdata->quest->mission == 1)
               ch_printf(ch, "Quest Kills:       %d out of %d\n\r", ch->pcdata->quest->killed, ch->pcdata->quest->tokill);
            if (ch->pcdata->quest->mission == 4)
               ch_printf(ch, "Quest Collects:    %d out of %d\n\r", ch->pcdata->quest->killed, ch->pcdata->quest->tokill);
            ch_printf(ch, "Quest Difficulty:  %d\n\r", ((ch->pcdata->quest->difficulty-1)/10)+1);
         }
      }
      ch_printf(ch, "\n\rQPS:               %d/%d\n\r", ch->pcdata->quest_curr, ch->pcdata->quest_accum);
      ch_printf(ch, "Quest Wins:        %d\n\r", ch->pcdata->quest_wins);
      ch_printf(ch, "Quest Losses:      %d\n\r", ch->pcdata->quest_losses);
      return;
   }   
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (IS_NPC(victim))
         if (xIS_SET(victim->act, ACT_QUESTMOB))
            break;
   }
   if (!victim)
   {
      send_to_char("You can only request a quest at a quest giver.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "simple"))
      autol = 1;
   if (!str_cmp(arg1, "easy"))
      autol = 2;
   if (!str_cmp(arg1, "easy-medium"))
      autol = 3;
   if (!str_cmp(arg1, "mediun"))
      autol = 4;
   if (!str_cmp(arg1, "medium-hard"))
      autol = 5;
   if (!str_cmp(arg1, "hard"))
      autol = 6;
      
   if (autol > 0)
   {
      sworth = player_stat_worth(ch);

      if (sworth >= 300000)
         difficulty = 10;
      else if (sworth >= 225000)
         difficulty = 9;
      else if (sworth >= 150000)
         difficulty = 8;
      else if (sworth >= 75000)
         difficulty = 7;
      else if (sworth >= 35000)
         difficulty = 6;
      else if (sworth >= 25000)
         difficulty = 5;
      else if (sworth >= 15000)
         difficulty = 4;
      else if (sworth >= 7500)
         difficulty = 3;
      else if (sworth >= 3500)
         difficulty = 2;
      else
         difficulty = 1;
         
      if (autol == 3)
         difficulty -= 1;
      if (autol == 2)
         difficulty -= 2;
      if (autol == 1)
         difficulty -= 3;
      if (autol == 5)
         difficulty += 1;
      if (autol == 6)
         difficulty += 2;
      if (difficulty < 1)
         difficulty = 1;
      if (difficulty > 10)
         difficulty = 10;
         
      if (autol == 1)
      {
         size = 50;
         mcount = 80;
      }
      if (autol == 2)
      {
         size = 100;
         mcount = 110;
      }
      if (autol >= 3)
      {
         size = 150;
         mcount = 140;
      }
      for (victim = first_char; victim; victim = victim->next)
      {
         if (!IS_NPC(victim) && is_same_group(victim, ch) && victim->pcdata->quest)
         {
            send_to_char("You cannot get a quest, someone in your part is in a quest or waiting for their last one to expire.\n\r", ch);
            return;
         }
         if (!IS_NPC(victim) && is_same_group(victim, ch) && victim != ch)
         {
            gsize++;
            if (player_stat_worth(victim) < losworth)
               losworth = player_stat_worth(victim);
            if (player_stat_worth(victim) > hisworth)
               hisworth = player_stat_worth(victim);
         }
      }
      if (hisworth > losworth*2 && losworth < 150000 && !sysdata.resetgame)
      {
         send_to_char("There is a sworth restriction in your group, please try again.\n\r", ch);
         return;
      }
      omcount = mcount;
      mcount = mcount * (35 + (gsize*65)) / 100; 
      difficulty = (10*difficulty) - 10 + number_range(1, 10);
      success = load_new_quest_zone(ch, 0, size, difficulty, 1, mcount);
   
      if (success == 0)
      {
         send_to_char("Failed to load the quest, try again please.\n\r", ch);
         return;
      }
      load_queststruct_data(ch, size, 1, mcount*size/100, get_finishtime(size, omcount, difficulty, 1, 1), difficulty, 1);
      top_quest_vnum += size; 
      return;
   }
     
   if (!str_cmp(arg1, "static") && IS_IMMORTAL(ch))
   {
      argument = one_argument(argument, arg5);
      argument = one_argument(argument, arg6);
      if (get_trust(ch) < LEVEL_ADMIN)
      {
         send_to_char("Only Admin Level Immortals can use this command.\n\r", ch);
         return;
      }
      if (atoi(arg2) < 5 || atoi(arg2) > MAX_QDIFF_VALUE)
      {
         ch_printf(ch, "Difficulty rating is 5 - %d.\n\r", MAX_QDIFF_VALUE);
         return;
      }
      if (atoi(arg3) < 50 || atoi(arg3) > 2000)
      {
         send_to_char("Range is 50 to 2000.\n\r", ch);
         return;
      }
      if (atoi(arg4) < 20 || atoi(arg4) > 500)
      {
         send_to_char("Range is 20 to 500.\n\r", ch);
         return;
      }
      if (atoi(argument) < 1 || atoi(argument) > 10080)
      {
         send_to_char("Range is 1 to 10080 (7 days).\n\r", ch);
         return;
      }
      global_drop_equip_message = 1;
      success = load_static_quest_zone(ch, atoi(arg2), atoi(arg3), atoi(arg4), arg5, arg6, atoi(argument));
      global_drop_equip_message = 0;
      if (success == 0)
      {
         send_to_char("Failed to load the quest, try again please.\n\r", ch);
         return;
      }
      send_to_char("Done.\n\r", ch);
      return;
   }
   //send_to_char("quest static <difficulty> <vnums> <mob count> <area name> <filename> <reset time>\n\r", ch);
   if (atoi(arg1) >= 1)
   {
      int timeload = 1;
      if (atoi(arg1) < 1 || atoi(arg1) > MAX_QDIFF)
      {
         ch_printf(ch, "Difficulty rating is 1 - %d.\n\r", MAX_QDIFF);
         return;
      }
      if (!str_cmp(arg2, "slaughter"))
      {
         qtype = 1;
      }
      else if (!str_cmp(arg2, "gauntlet"))
      {
         qtype = 2;
      }
      else if (!str_cmp(arg2, "boss"))
      {
         qtype = 3;
      }
      else if (!str_cmp(arg2, "collect"))
      {
         qtype = 4;
      }
      else
      {
         send_to_char("Your available choices are: slaughter gauntlet boss collect.\n\r", ch);
         return;
      }
      if (atoi(arg3) < 1 || atoi(arg3) > 6)
      {
         send_to_char("Size ranges from 1 - 6.\n\r", ch);
         return;
      }
      if (atoi(arg4) < 1 || atoi(arg4) > 6)
      {
         send_to_char("Mob count ranges from 1- 6.\n\r", ch);
         return;
      }   
      if (atoi(argument) < -1 || atoi(argument) > 10)
      {
         send_to_char("Increased time ranges from -1 (till reboot/crash) to 10 (10 times)\n\r", ch);
         return;
      }
      timeload = atoi(argument);
      if (timeload == 0)
         timeload = 1;
      if (atoi(arg3) == 1)
         size = 50;
      if (atoi(arg3) == 2)
         size = 100;
      if (atoi(arg3) == 3)
         size = 150;
      if (atoi(arg3) == 4)
         size = 200;
      if (atoi(arg3) == 5)
         size = 250;
      if (atoi(arg3) == 6)
         size = 300;
      if (atoi(arg4) == 1)
         mcount = 80;
      if (atoi(arg4) == 2)
         mcount = 110;
      if (atoi(arg4) == 3)
         mcount = 140;
      if (atoi(arg4) == 4)
         mcount = 170;
      if (atoi(arg4) == 5)
         mcount = 200;
      if (atoi(arg4) == 6)
         mcount = 230;
      for (victim = first_char; victim; victim = victim->next)
      {
         if (!IS_NPC(victim) && is_same_group(victim, ch) && victim->pcdata->quest)
         {
            send_to_char("You cannot get a quest, someone in your part is in a quest or waiting for their last one to expire.\n\r", ch);
            return;
         }
         if (!IS_NPC(victim) && is_same_group(victim, ch) && victim != ch)
         {
            gsize++;
            if (player_stat_worth(victim) < losworth)
               losworth = player_stat_worth(victim);
            if (player_stat_worth(victim) > hisworth)
               hisworth = player_stat_worth(victim);
         }
      }
      if (hisworth > losworth*2 && losworth < 150000 && !sysdata.resetgame)
      {
         send_to_char("There is a sworth restriction in your group, please try again.\n\r", ch);
         return;
      }
      omcount = mcount;
      mcount = mcount * (35 + (gsize*65)) / 100;
      if (qtype == 3 && size == 50 && mcount < 160)
      {
         send_to_char("It takes a size 1 mob count 4 to load a boss or a size 2.  Cannot do a boss quest without a boss.\n\r", ch);
         return;
      }
      
      difficulty = atoi(arg1);
      difficulty = (10*difficulty) - 10 + number_range(1, 10);
      #ifdef MTRACE
      setenv( "MALLOC_TRACE", "mtrace_log.log", 0 );
      mtrace();  /* Turn on mtrace function */
      #endif
      success = load_new_quest_zone(ch, 0, size, difficulty, qtype, mcount);
   
      if (success == 0)
      {
         send_to_char("Failed to load the quest, try again please.\n\r", ch);
         return;
      }
      if (timeload == -1)
         mcount *= 10;
      load_queststruct_data(ch, size, qtype, mcount*size/100, get_finishtime(size, omcount, difficulty, timeload, qtype), difficulty, timeload);
      top_quest_vnum += size;
      return;
   }
   do_loadquest(ch, "");
}
   
   
   
   
   
   
   
   
   
   
   
   
   
   
   
