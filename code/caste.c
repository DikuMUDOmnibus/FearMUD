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
 *			 Caste/Rafermand related skills 			    *
 *      Seefile Jobinfo.txt in /doc for quick reference                     *
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


//functions
int get_buykobjflag args((char *type));
int get_buykmobflag args((char *type));
int islinked args((CHAR_DATA *ch, int sx, int sy, int ex, int ey, char *directions, int onroad));
/* Below are globals for the blackjack :-) */
int amtbet;
char buf2[MSL];
void args(write_boards_txt());
BUYKMOB_DATA *first_buykmob;
BUYKMOB_DATA *last_buykmob;
BUYKOBJ_DATA *first_buykobj;
BUYKOBJ_DATA *last_buykobj;
BUYKBIN_DATA *first_buykbin;
BUYKBIN_DATA *last_buykbin;
BUYKTRAINER_DATA *first_buyktrainer;
BUYKTRAINER_DATA *last_buyktrainer;
BTRAINER_DATA *first_boughttrainer;
BTRAINER_DATA *last_boughttrainer;

sh_int pcard1;
sh_int pcard2;
sh_int pcard3;
sh_int pcard4;
sh_int pcard5;
sh_int gcard1;
sh_int gcard2;
sh_int gcard3;
sh_int gcard4;
sh_int gcard5;
sh_int gresult;
char logb[MSL];

const char *peacestatus[MAX_PEACEVALUE] = {
   "WAR", "Neutral", "Trading", "Peace"
};
const char *peacecolor[MAX_PEACEVALUE] = {
   "&R", "&g", "&G&W", "&Y"
};

//Mobile eq
const struct military_eq mi_eq[] = {
   {16150, 16161, 16165, 16167, 16168, 16170},
   {16156, 16160, 16168, 16171, 0, 0},
   {16157, 16163, 16169, 16171, 0, 0},
   {16152, 16160, 16164, 16168, 16170, 0},
   {16151, 16161, 16165, 16167, 16168, 16170},
   {16154, 16161, 16165, 16167, 16168, 16170},
   {0, 0, 0, 0, 0, 0},
   {16150, 16161, 16165, 16167, 16168, 16170},
   {16150, 16162, 16165, 16166, 16168, 16170},
   {16155, 16163, 16167, 16169, 16171, 0},
   {16156, 16160, 16168, 16171, 0, 0},
   {16153, 16160, 16164, 16168, 16170, 0},
   {16158, 16163, 16164, 16167, 16169, 16171}
};

int get_used_imbueslots args((OBJ_DATA *obj));

void do_lore(CHAR_DATA * ch, char *argument)
{
/* Modified identify spell with a few other new features -- Xerves 8/18/99 */
   OBJ_DATA *obj;
   OBJ_DATA *mobj;
   AREA_DATA *tarea; /* Test for lore area location */
   int avnum;
   int found = 0;
   char namefound[MSL];
   AFFECT_DATA *paf;
   char buf[MSL];
   char bsize[100];
   char arg[MIL];
   int lore;
   
   if (IS_NPC(ch))
   {
      send_to_char("This is for PCs only.\n\r", ch);
      return;
   }

   lore = POINT_LEVEL(GET_POINTS(ch, gsn_lore, 0, 1), GET_MASTERY(ch, gsn_lore, 0, 1));

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Syntax: lore <object>\n\r", ch);
      return;
   }
   /* Certain aspects are based on lore rating.  The lore raiting is based on
      the field and the level of the object.  Making it easier to know more about
      low level objects than higher level ones (more rare) */
   mobj = get_obj_mobworld(ch, arg);
   if ((obj = get_obj_carry(ch, arg)) != NULL)
   {
      check_for_trap(ch, obj, -1, NEW_TRAP_IDENTOBJ);
      if (char_died(ch))
         return;
      if (!obj)
         return; 
      set_char_color(AT_LBLUE, ch);
      if (ch->level > LEVEL_PC)
         lore = 120;
         
      if (lore >= 3)
      {
         ch_printf(ch, "\n\rObject '%s' is %s",
/*		obj->name,*/
            obj->short_descr, aoran(item_type_name(obj)));
      }
      if (lore >= 9)
      {
         if (obj->item_type != ITEM_LIGHT && obj->wear_flags - 1 > 0)
            ch_printf(ch, ", with wear location:  %s\n\r", flag_string(obj->wear_flags, w_flags));
         else
            send_to_char(".\n\r", ch);
      }
      if (lore >= 22)
      {
         ch_printf(ch, "Special properties:  %s\n\rIts weight is %.2f and value is %d.\n\r", ext_flag_string(&obj->extra_flags, o_flags),
            obj->weight, obj->cost);
         if (obj->sworthrestrict)
            ch_printf(ch, "Sworth Restriction of:  %d\n\r", obj->sworthrestrict);
         if (obj->imbueslots)
            ch_printf(ch, "Imblue Slots: %d used of %d total\n\r", get_used_imbueslots(obj), obj->imbueslots);
      }
      for (tarea = first_area; tarea; tarea = tarea->next)
      {
         avnum = obj->pIndexData->vnum;
         if ((avnum > tarea->low_o_vnum) && (avnum < tarea->hi_o_vnum))
         {
            found = 1;
            sprintf(namefound, tarea->name);
         }
      }
      if (lore >= 47)
      {
         if (found == 0)
            ch_printf(ch, "Location is Unknown.\n\r");
         else
            ch_printf(ch, "Location is %s.\n\r", namefound);
      }
      if (lore >= 63)
      {
         if (mobj != NULL)
            ch_printf(ch, "Carried by: %s\n\r", mobj->carried_by == NULL ? "(none)" : PERS_MAP(mobj->carried_by, ch));
         else
            ch_printf(ch, "Carried by: Nobody\n\r");
      }
      set_char_color(AT_MAGIC, ch);

      switch (obj->item_type)
      {
         case ITEM_CONTAINER:
            if (lore >= 31)
            {
               ch_printf(ch, "%s appears to be %s.\n\r", capitalize(obj->short_descr),
                  obj->value[0] < 76 ? "of a small capacity" :
                  obj->value[0] < 150 ? "of a small to medium capacity" :
                  obj->value[0] < 300 ? "of a medium capacity" :
                  obj->value[0] < 550 ? "of a medium to large capacity" : obj->value[0] < 751 ? "of a large capacity" : "of a giant capacity");
               ch_printf(ch, "Has a weight modification of %d.\n\r", obj->value[2] > 0 ? obj->value[2] : 100);
            }
            break;
            
         case ITEM_SCROLL:
         case ITEM_POTION:
            if (lore >= 31)
            {
               sprintf(buf, "Power level %s spells of:", get_wplevel(obj->value[5]));
               send_to_char(buf, ch);

               if (obj->value[1] >= 0 && obj->value[1] < top_sn)
               {
                  send_to_char(" '", ch);
                  send_to_char(skill_table[obj->value[1]]->name, ch);
                  send_to_char("'", ch);
               }

               if (obj->value[2] >= 0 && obj->value[2] < top_sn)
               {
                  send_to_char(" '", ch);
                  send_to_char(skill_table[obj->value[2]]->name, ch);
                  send_to_char("'", ch);
               }

               if (obj->value[3] >= 0 && obj->value[3] < top_sn)
               {
                  send_to_char(" '", ch);
                  send_to_char(skill_table[obj->value[3]]->name, ch);
                  send_to_char("'", ch);
               }

               send_to_char(".\n\r", ch);
            }
            break;

         case ITEM_SPELLBOOK:
            if (lore >= 31)
            {
               ch_printf(ch, "Contains knowledge obtainable at level %d and mastery %d of", obj->value[0], obj->value[2]);
               if (obj->value[1] >= 0 && obj->value[1] < top_sn)
               {
                  send_to_char(" '", ch);
                  send_to_char(skill_table[obj->value[1]]->name, ch);
                  send_to_char("'", ch);
               }
               send_to_char(".\n\r", ch);
               break;
            }
            
         case ITEM_TGEM:
            if (lore >= 31)
            {
               ch_printf(ch, "    Has a Slot Value of %d\n\r", obj->value[12] == 0 ? 1 : obj->value[12]);
               ch_printf(ch, showgemaff(ch, obj, 1, NULL));
               break;
            }
         case ITEM_REPAIR:
            if (lore >= 31)
            {
               ch_printf(ch, "Uses %d/%d      Fix Percent: %d      Points fixed: %d\n\r", obj->value[0], obj->value[1],
                  obj->value[2], obj->value[3]);
            }
            break;
            
         case ITEM_PROJECTILE:
            if (lore >= 31)
            {   
               ch_printf(ch, "Damage is %d to %d (average %d)%s\n\r",
                  obj->value[1], obj->value[2], (obj->value[1] + obj->value[2]) / 2, IS_OBJ_STAT(obj, ITEM_POISONED) ? ", and is poisonous." : ".");
               ch_printf(ch, "StabMod %d\n\r", obj->value[9]);
            }
            break;
         case ITEM_MISSILE_WEAPON:
            if (lore >= 31)
            {   
               ch_printf(ch, "Damage is %d to %d (average %d)%s\n\r",
                  obj->value[1], obj->value[2], (obj->value[1] + obj->value[2]) / 2, IS_OBJ_STAT(obj, ITEM_POISONED) ? ", and is poisonous." : ".");
               ch_printf(ch, "Has a weapon size of %d\n\r", obj->value[3]);
               ch_printf(ch, "StabMod %d\n\r", obj->value[9]);
               ch_printf(ch, "Condition %d\n\r", obj->value[0]);
               ch_printf(ch, "Durability %d\n\r", obj->value[10]);
               ch_printf(ch, "Range %d\n\r", obj->value[4]);
               if (obj->bless_dur > 0)
               {
                  ch_printf(ch, "Has %d more attacks before Bless wears off.\n\r", obj->bless_dur);
               }
            }
            break; 
         case ITEM_WEAPON:  
            if (lore >= 31)
            {         
               ch_printf(ch, "Damage is %d to %d (average %d)%s\n\r",
                  obj->value[1], obj->value[2], (obj->value[1] + obj->value[2]) / 2, IS_OBJ_STAT(obj, ITEM_POISONED) ? ", and is poisonous." : ".");
               ch_printf(ch, "Has a weapon size of %d\n\r", obj->value[3]);
               ch_printf(ch, "BashMod %d  SlashMod %d StabMod %d\n\r", obj->value[7], obj->value[8], obj->value[9]);
               ch_printf(ch, "Condition %d\n\r", obj->value[0]);
               ch_printf(ch, "Durability %d\n\r", obj->value[10]);
               ch_printf(ch, "Parry Mod %d  Parry Block Mod %d\n\r", obj->value[12], obj->value[13]);
               if (obj->value[4] >= 1 && obj->value[4] < top_sn)
               {
                  send_to_char("Has the spell", ch);
                  send_to_char(buf, ch);
                  send_to_char(" '", ch);
                  send_to_char(skill_table[obj->value[4]]->name, ch);
                  send_to_char("'", ch);
                  sprintf(buf, " at power level %s.\n\r", get_wplevel(obj->value[5]));
                  send_to_char(buf, ch);
               }
               if (obj->bless_dur > 0)
                  ch_printf(ch, "Has %d more attacks before Bless wears off.\n\r", obj->bless_dur);
            }
            break;

         case ITEM_ARMOR:
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))   
            {
               ch_printf(ch, "Condition %d/%d     Block Percent %d     Battle Lag %d\n\r", obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
            }
            else
            {
               ch_printf(ch, "BashMod %d SlashMod %d StabMod %d.\n\r", obj->value[0], obj->value[1], obj->value[2]);
               ch_printf(ch, "Condition %d\n\r", obj->value[3]);
               ch_printf(ch, "Durability %d\n\r", obj->value[4]);
               if (obj->value[5] == 1)
                  sprintf(bsize, "Leather");
               else if (obj->value[5] == 2)
                  sprintf(bsize, "Light");
               else if (obj->value[5] == 3)
                  sprintf(bsize, "Medium");
               else if (obj->value[5] == 4)
                  sprintf(bsize, "Heavy");
               else if (obj->value[5] == 5)
                  sprintf(bsize, "Heaviest");
               else
                  sprintf(bsize, "NULL");
               ch_printf(ch, "ArmorSize %s\n\r", bsize); 
            }
            break;
      }
      if (lore >= 41)
      {
         for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
            showaffect(ch, paf, 0);

         for (paf = obj->first_affect; paf; paf = paf->next)
            showaffect(ch, paf, 0);
      }
      set_char_color(AT_RED, ch);
      ch_printf(ch, "\n\rThat is all you know about %s\n\r", obj->short_descr);
      learn_from_success(ch, gsn_lore, NULL);
      WAIT_STATE(ch, 12);
      return;
   }
   else
   {
      ch_printf(ch, "You don't have that in your inventory\n\r");
      return;
   }
}

int get_banksize(int size)
{
   if (size == 1) return 1000;
   if (size == 2) return 2000;
   if (size == 3) return 3000;
   if (size == 4) return 4000;
   if (size == 5) return 5000;
   if (size == 6) return 10000;
   if (size == 7) return 20000;
   if (size == 8) return 50000;
   if (size == 9) return 100000;
   if (size == 10) return 200000;
   return 1000;
}

/* Like get_dir, but returns -1 if nothing is found */
int get_truedir(char *txt)
{
   int edir;
   char c1, c2;

   if (!str_cmp(txt, "northeast"))
      return DIR_NORTHEAST;
   if (!str_cmp(txt, "northwest"))
      return DIR_NORTHWEST;
   if (!str_cmp(txt, "southeast"))
      return DIR_SOUTHEAST;
   if (!str_cmp(txt, "southwest"))
      return DIR_SOUTHWEST;
   if (!str_cmp(txt, "somewhere"))
      return 10;

   c1 = txt[0];
   if (c1 == '\0')
      return -1;
   c2 = txt[1];
   edir = -1;
   switch (c1)
   {
      case 'n':
         switch (c2)
         {
            default:
               edir = 0;
               break; /* north */
            case 'e':
               edir = 6;
               break; /* ne  */
            case 'w':
               edir = 7;
               break; /* nw  */
         }
         break;
      case '0':
         edir = 0;
         break; /* north */
      case 'e':
      case '1':
         edir = 1;
         break; /* east  */
      case 's':
         switch (c2)
         {
            default:
               edir = 2;
               break; /* south */
            case 'e':
               edir = 8;
               break; /* se  */
            case 'w':
               edir = 9;
               break; /* sw  */
         }
         break;
      case '2':
         edir = 2;
         break; /* south */
      case 'w':
      case '3':
         edir = 3;
         break; /* west  */
      case 'u':
      case '4':
         edir = 4;
         break; /* up  */
      case 'd':
      case '5':
         edir = 5;
         break; /* down  */
      case '6':
         edir = 6;
         break; /* ne  */
      case '7':
         edir = 7;
         break; /* nw  */
      case '8':
         edir = 8;
         break; /* se  */
      case '9':
         edir = 9;
         break; /* sw  */
      case '?':
         edir = 10;
         break; /* somewhere */
   }
   return edir;
}

//Not used anymore
void do_peasant(CHAR_DATA * ch, char *argument)
{
   return;
}

// Not used anymore
void do_setcaste(CHAR_DATA * ch, char *argument)
{
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

//Below few functions create a list for clan members/Kingdom members
void fread_file(FILE * fp, char *fname)
{
   char *word;
   char *name = NULL;
   int caste, kingdom, race, level, sex, kpid;
   char *clan = NULL;
   CMEMBER_DATA *clist;
   KMEMBER_DATA *klist;
   CLAN_DATA *cdata;
   char buf[MSL];
   bool fMatch;

   caste = kingdom = race = level = sex = kpid = -1;

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
         case 'C':
            KEY("Caste", caste, fread_number(fp));
            KEY("Clan", clan, fread_string(fp));
            break;

         case 'E':
            if (!strcmp(word, "End"))
            {
               if (name && race > -1 && level > -1 && sex > -1)
               {
                  if (caste > 0 && kingdom > 1)
                  {
                     CREATE(klist, KMEMBER_DATA, 1);
                     klist->name = STRALLOC(name);
                     klist->caste = caste;
                     klist->kingdom = kingdom;
                     klist->level = level;
                     klist->race = race;
                     klist->kpid = kpid;
                     klist->sex = sex;
                     LINK(klist, first_kingdommember, last_kingdommember, next, prev);
                  }
                  if (clan)
                  {
                     CREATE(clist, CMEMBER_DATA, 1);
                     clist->name = STRALLOC(name);
                     clist->clan = STRALLOC(clan);
                     clist->level = level;
                     clist->race = race;
                     clist->sex = sex;
                     if ((cdata = get_clan(clan)) != NULL)
                     {
                        if (!str_cmp(name, cdata->deity))
                           sprintf(buf, "Deity");
                        else if (!str_cmp(name, cdata->leader))
                           sprintf(buf, "Leader");
                        else if (!str_cmp(name, cdata->number1))
                           sprintf(buf, "Number1");
                        else if (!str_cmp(name, cdata->number2))
                           sprintf(buf, "Number2");
                        else
                           sprintf(buf, "Member");

                        clist->rank = STRALLOC(buf);
                     }
                     LINK(clist, first_clanmember, last_clanmember, next, prev);
                  }
               }
               return;
            }

         case 'H':
            KEY("HomeTown", kingdom, fread_number(fp));
            break;
            
         case 'K':
            KEY("KingdomPid", kpid, fread_number(fp));

         case 'L':
            KEY("Level", level, fread_number(fp));
            break;

         case 'N':
            KEY("Name", name, fread_string(fp));
            break;

         case 'R':
            KEY("Race", race, fread_number(fp));
            break;

         case 'S':
            KEY("Sex", sex, fread_number(fp));
      }

      if (!fMatch)
         fread_to_eol(fp);
   }
}

void read_for_list(char *dirname, char *filename)
{
   FILE *fp;
   char fname[MSL];
   struct stat fst;

   sprintf(fname, "%s/%s", dirname, filename);

   if (stat(fname, &fst) != -1)
   {
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
               fread_file(fp, fname);
            else if (!str_cmp(word, "END")) /* Done  */
               break;
         }
         FCLOSE(fp);
      }
   }
   return;
}

//Mainly scan_pfiles from pfiles code with a few mods
void scan_players(void)
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
            read_for_list(dir_name, dentry->d_name);
         }
         dentry = readdir(dp);
      }
      closedir(dp);
   }
}

void remove_player_list(CHAR_DATA * ch, int type)
{
   CMEMBER_DATA *clist;
   KMEMBER_DATA *klist;

   if (type == 0) //Kingdom List
   {
      for (klist = first_kingdommember; klist; klist = klist->next)
      {
         if (!str_cmp(ch->name, klist->name))
         {
            UNLINK(klist, first_kingdommember, last_kingdommember, next, prev);
            STRFREE(klist->name);
            DISPOSE(klist);
            return;
         }
      }
   }
   if (type == 1)
   {
      for (clist = first_clanmember; clist; clist = clist->next)
      {
         if (!str_cmp(ch->name, clist->name))
         {
            UNLINK(clist, first_clanmember, last_clanmember, next, prev);
            STRFREE(clist->name);
            STRFREE(clist->clan);
            STRFREE(clist->rank);
            DISPOSE(clist);
            return;
         }
      }
   }
}

void sort_player_list(CMEMBER_DATA * clist, KMEMBER_DATA * klist)
{
   CMEMBER_DATA *cdata;
   KMEMBER_DATA *kdata;


   if (clist && !first_clanmember)
   {
      LINK(clist, first_clanmember, last_clanmember, next, prev);
      return; 
   }
   if (klist && !first_kingdommember)
   {
      LINK(klist, first_kingdommember, last_kingdommember, next, prev);
      return;
   }
   if (clist)
   {
      for (cdata = first_clanmember; cdata; cdata = cdata->next)
      {
         if (strcmp(clist->name, cdata->name) < 0)
         {
            INSERT(clist, cdata, first_clanmember, next, prev);
            return;
         }
      }
   }
   if (klist)
   {
      for (kdata = first_kingdommember; kdata; kdata = kdata->next)
      {
         if (strcmp(klist->name, kdata->name) < 0)
         {
            INSERT(klist, kdata, first_kingdommember, next, prev);
            return;
         }
      }
   }
   if (clist)
   {
      if (!last_clanmember || strcmp(clist->name, last_clanmember->name) > 0)
      {
         LINK(clist, first_clanmember, last_clanmember, next, prev);
         return;
      }
   }
   if (klist)
   {
      if (!last_kingdommember || strcmp(klist->name, last_kingdommember->name) > 0)
      {
         LINK(klist, first_kingdommember, last_kingdommember, next, prev);
         return;
      }
   }
}

void add_player_list(CHAR_DATA * ch, int type)
{
   CMEMBER_DATA *clist;
   KMEMBER_DATA *klist;
   char buf[MSL];

   if (type == 0) //Kingdom List
   {
      CREATE(klist, KMEMBER_DATA, 1);
      sprintf(buf, ch->name);
      klist->name = STRALLOC(buf);
      klist->caste = ch->pcdata->caste;
      klist->kingdom = ch->pcdata->hometown;
      klist->level = ch->level;
      klist->race = ch->race;
      klist->sex = ch->sex;
      sort_player_list(NULL, klist);
   }
   if (type == 1) //Clan List
   {
      CLAN_DATA *cdata;

      CREATE(clist, CMEMBER_DATA, 1);
      sprintf(buf, ch->name);
      clist->name = STRALLOC(buf);
      sprintf(buf, ch->pcdata->clan_name);
      clist->clan = STRALLOC(buf);
      clist->level = ch->level;
      clist->race = ch->race;
      clist->sex = ch->sex;
      if ((cdata = get_clan(ch->pcdata->clan_name)) != NULL)
      {
         if (!str_cmp(ch->name, cdata->deity))
            sprintf(buf, "Deity");
         else if (!str_cmp(ch->name, cdata->leader))
            sprintf(buf, "Leader");
         else if (!str_cmp(ch->name, cdata->number1))
            sprintf(buf, "Number1");
         else if (!str_cmp(ch->name, cdata->number2))
            sprintf(buf, "Number2");
         else
            sprintf(buf, "Member");

         clist->rank = STRALLOC(buf);
      }
      sort_player_list(clist, NULL);
   }
}


void do_stable(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   int num = 0;
   STABLE_DATA *stb;
   MOB_INDEX_DATA *pMob = NULL;
   CHAR_DATA *mob = NULL;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Stable commands are for PCs only.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax <args>\n\rargs - build store list retrieve release\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (ch->in_room->vnum == 0)
   {
      send_to_char("You need a stable to use this command you idiot.\n\r", ch);
      return;
   }
   if (ch->in_room->vnum != ch->pcdata->stable)
   {
      send_to_char("You need to be in your stable to use this command.", ch);
      return;
   }
   for (stb = ch->pcdata->first_stable; stb; stb = stb->next)
      num++;
   ch->pcdata->stablecurr = num;

   if (!str_cmp(arg, "release"))
   {
      if (!ch->pcdata->mount)
      {
         send_to_char("You need to have a pet mount to release it first.\n\r", ch);
         return;
      }
      if (ch->pcdata->mount->in_room != ch->in_room)
      {
         send_to_char("Your pet mount needs to be here in the stable with you.\n\r", ch);
         return;
      }
      send_to_char("You proudly, but painfully release the mount back into the Wilderness.\n\r", ch);
      ch->position = POS_STANDING;
      xREMOVE_BIT(ch->act, PLR_BOUGHT_MOUNT);
      extract_char(ch->pcdata->mount, TRUE);
      ch->pcdata->mount = NULL;
      save_char_obj(ch);
      return;
   }
   else if (!str_cmp(arg, "list"))
   {
      num = 0;
      for (stb = ch->pcdata->first_stable; stb; stb = stb->next)
      {
         num++;
         ch_printf(ch, "%d %s    Level %d  Hp %d  Move %d\n\r", num, stb->short_descr, stb->level, stb->max_hit, stb->max_move);
      }
      if (num == 0)
      {
         send_to_char("You don't have any mounts in your stable.\n\r", ch);
         return;
      }
      else
      {
         ch_printf(ch, "\n\rThat is %d out of %d mounts allowed.\n\r", ch->pcdata->stablecurr, ch->pcdata->stablenum);
         return;
      }
      return;
   }
   else if (!str_cmp(arg, "store"))
   {
      if (ch->pcdata->stablecurr + 1 > ch->pcdata->stablenum)
      {
         ch_printf(ch, "Your stable can only hold %d mounts.\n\r", ch->pcdata->stablenum);
         return;
      }
      if (!ch->pcdata->mount)
      {
         send_to_char("You need to have a pet mount to store it first.\n\r", ch);
         return;
      }
      if (ch->pcdata->mount->in_room != ch->in_room)
      {
         send_to_char("Your pet mount needs to be here in the stable with you.\n\r", ch);
         return;
      }
      ch_printf(ch, "You gently urge %s into its new home.\n\r", ch->pcdata->mount->short_descr);
      ch->position = POS_STANDING;
      xREMOVE_BIT(ch->pcdata->mount->act, ACT_MOUNTED);
      xREMOVE_BIT(ch->act, PLR_BOUGHT_MOUNT);
      ch->position = POS_STANDING;;
      CREATE(stb, STABLE_DATA, 1);
      stb->vnum = ch->pcdata->mount->pIndexData->vnum;
      stb->name = QUICKLINK(ch->pcdata->mount->name);
      stb->short_descr = QUICKLINK(ch->pcdata->mount->short_descr);
      stb->long_descr = QUICKLINK(ch->pcdata->mount->long_descr);
      stb->description = QUICKLINK(ch->pcdata->mount->description);
      stb->level = ch->pcdata->mount->level;
      stb->exp = ch->pcdata->mount->m1;
      stb->hit = ch->pcdata->mount->hit;
      stb->max_hit = ch->pcdata->mount->max_hit;
      stb->move = ch->pcdata->mount->move;
      stb->max_move = ch->pcdata->mount->max_move;
      LINK(stb, ch->pcdata->first_stable, ch->pcdata->last_stable, next, prev);
      extract_char(ch->pcdata->mount, TRUE);
      ch->pcdata->mount = NULL;
      save_char_obj(ch);
      return;
   }
   else if (!str_cmp(arg, "retrieve"))
   {
      if (ch->pcdata->mount)
      {
         send_to_char("You need to store or release your current mount first.\n\r", ch);
         return;
      }
      num = 1;
      for (stb = ch->pcdata->first_stable; stb; stb = stb->next)
      {
         if (num == atoi(argument))
         {
            if ((pMob = get_mob_index(stb->vnum)) == NULL)
            {
               send_to_char("No mobile has that vnum.  Please inform an immortal of this\n\r", ch);
               bug("Stable: %s has mobile %d that does not exist, needs fixed", ch->name, stb->vnum);
               return;
            }
         }
         mob = create_mobile(pMob);
         char_to_room(mob, ch->in_room);
         mob->coord->x = -1;
         mob->coord->y = -1;
         mob->map = -1;

         STRFREE(mob->name);
         mob->name = STRALLOC(stb->name);
         STRFREE(mob->short_descr);
         mob->short_descr = STRALLOC(stb->short_descr);
         STRFREE(mob->long_descr);
         mob->long_descr = STRALLOC(stb->long_descr);
         STRFREE(mob->description);
         mob->description = STRALLOC(stb->description);
         mob->m1 = stb->exp;
         mob->level = stb->level;
         mob->max_hit = stb->max_hit;
         mob->hit = stb->hit;
         mob->max_move = stb->max_move;
         mob->move = stb->move;
         UNLINK(stb, ch->pcdata->first_stable, ch->pcdata->last_stable, next, prev);
         REMOVE_ONMAP_FLAG(mob);
      }
      if (mob == NULL)
      {
         send_to_char("That is not a choice, type stable list and pick the number you want.\n\r", ch);
         return;
      }
      ch_printf(ch, "You joyfully call for %s to come to you.\n\r", mob->short_descr);
      ch->position = POS_STANDING;
      xSET_BIT(ch->act, PLR_BOUGHT_MOUNT);
      xSET_BIT(mob->affected_by, AFF_CHARM);
      if (mob->master == NULL)
         add_follower(mob, ch);
      ch->pcdata->mount = mob;
      ch->pcdata->mount->position = POS_STANDING;
      save_char_obj(ch);
      return;
   }
   else if (!str_cmp(arg, "build"))
   {
      for (obj = ch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->pIndexData->vnum == 9119)
            break;
      }
      if (obj == NULL)
      {
         send_to_char("You need a S Addition License to build more room for your mounts.\n\r", ch);
         return;
      }
      ch->pcdata->stablenum += 3;
      if (obj)
      {
         separate_obj(obj);
         obj_from_char(obj);
         extract_obj(obj);
      }
      else
      {
         bug("Cannot extract S add license from %s.", ch->name);
      }
      ch_printf(ch, "Your stable can now hold %d mounts.\n\r", ch->pcdata->stablenum);
      save_char_obj(ch);
      return;
   }
   else
   {
      send_to_char("Syntax <args>\n\rargs - build store list retrieve release\n\r", ch);
      return;
   }
}

/*5-00 Used to build a stable if you have a license - Xerves */
void do_makestable(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char *mapname;
   AREA_DATA *tarea;
   ROOM_INDEX_DATA *location;
   EXIT_DATA *xit;
   OBJ_DATA *obj;
   int found = 0;
   sh_int dir;
   sh_int value;
   sh_int start, vnum1;
   sh_int x, y, map;
   sh_int ox, oy;
   static char *dir_text[] = { "north", "east", "south", "west", "u", "d", "northeast", "northwest", "southeast",
      "southwest", "?"
   };

   x = ch->coord->x;
   y = ch->coord->y;
   map = ch->map;
   vnum1 = 0;


   if (IS_NPC(ch))
   {
      send_to_char("For PCs only.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: makestable <dir>", ch);
      return;
   }
   dir = get_truedir(argument);
   if ((dir < 0 || dir > 3) && (dir < 6 || dir > 9))
   {
      send_to_char("That is not a valid direction.\n\r", ch);
      return;
   }
   for (obj = ch->first_carrying; obj; obj = obj->next_content)
   {
      if (obj->pIndexData->vnum == 9118)
      {
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      send_to_char("In order to create a stable, you need a S class license.\n\r", ch);
      return;
   }
   if (ch->pcdata->stable != 0)
   {
      send_to_char("You already own a stable, creating another wouldn't be useful.\n\r", ch);
      return;
   }
   found = 0;
   if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
   {
      send_to_char("You need to be out in the Wilderness to create a stable.\n\r", ch);
      return;
   }
   if (ch->position != POS_STANDING && ch->position != POS_MOUNTED)
   {
      send_to_char("You need to be standing to create a stable.\n\r", ch);
      return;
   }
   ox = x;
   oy = y;
   x = get_x(x, dir);
   y = get_y(y, dir);
   if (sect_show[(int)map_sector[map][x][y]].sector != SECT_FIELD
      && sect_show[(int)map_sector[map][x][y]].sector != SECT_HILLS && sect_show[(int)map_sector[map][x][y]].sector != SECT_PLAINS)
   {
      send_to_char("You can only create a stable into the fields, hills, or plains.\n\r", ch);
      return;
   }
   for (start = 12100; start <= 13100; start++)
   {
      if (get_room_index(start) == NULL)
      {
         vnum1 = start;
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      bug("The area for Mount Stables is full");
      send_to_char("There is no more room available for mount stables, please notify an immortal.\n\r", ch);
      return;
   }
   /* Ok, all of that checking, now to actually build the rooms */
   add_entrance(-1, ch->map, x, y, -1, -1, vnum1);
   map_sector[ch->map][x][y] = SECT_EXIT;
   mapname = get_map_name(ch->map);
   save_map(mapname, ch->map);

   location = make_room(vnum1);
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (vnum1 > tarea->low_r_vnum && vnum1 < tarea->hi_r_vnum)
      {
         location->area = tarea;
         break;
      }
   }
   sprintf(buf, "%s's mount stables", ch->name);
   STRFREE(location->name);
   location->name = STRALLOC(buf);

   sprintf(buf, "You are standing before the stables that belong to %s.  The area \
\n\rappears to be nicely kept and the usual stench of animals is being kept to \
\n\ra minimum.  There are a few stables that are available for housing mounts, \
\n\rand it would be best not to mess with the mounts unless you own it.  The \
\n\ronly exit is %s.\n\r", ch->name, dir_text[rev_dir[dir]]);
   STRFREE(location->description);
   location->description = STRALLOC(buf);
   location->sector_type = sect_show[(int)map_sector[map][x][y]].sector;
   value = get_rflag("safe");
   xTOGGLE_BIT(location->room_flags, value);

   xit = make_exit(location, get_room_index(OVERLAND_SOLAN), rev_dir[dir]);
   xit->keyword = STRALLOC("");
   xit->description = STRALLOC("");
   xit->key = -1;
   xit->exit_info = 0;
   xit->coord->x = ox;
   xit->coord->y = oy;

   value = get_exflag("overland");
   TOGGLE_BIT(xit->exit_info, 1 << value);

   fold_area(location->area, location->area->filename, FALSE, 1);

   /* Done making, now lets set the character up */
   ch->pcdata->stable = vnum1;
   ch->pcdata->stablenum = 3;
   ch->pcdata->stablecurr = 0;

   if (obj)
   {
      separate_obj(obj);
      obj_from_char(obj);
      extract_obj(obj);
   }
   else
   {
      bug("Cannot extract S license from %s.", ch->name);
   }
   save_char_obj(ch);
   send_to_char("Your stable has been created.  Good luck.\n\r", ch);
   return;
}

/*11-28-98 Will change Caste ranking - Xerves */
//11-2001 Used to create/edit/delete kingom buy lists too.
void do_caste(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   MOB_INDEX_DATA *mob;
   int x, y;
   OBJ_INDEX_DATA *obj;
   int cnt = 0;
   BUYKMOB_DATA *kmob;
   BUYKOBJ_DATA *kobj;
   BUYKTRAINER_DATA *ktrainer;
   BUYKBIN_DATA *kbin;
   CHAR_DATA *victim;
   int amount;
   int flag;

   if (IS_NPC(ch))
   {
      return;
   }
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minhappoint ||
      ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend ||
      ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal || ch->pcdata->caste == caste_God)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: caste set <name> <number>\n\r", ch);
      if (ch->pcdata->caste >= caste_King)
      {
         send_to_char("Syntax: caste number1 <name>\n\r", ch);
         send_to_char("Syntax: caste number2 <name>\n\r", ch);
      }
      if (IS_IMMORTAL(ch))
      {
         send_to_char("-----------------------------------------------------------------------------\n\r", ch);
         send_to_char("Syntax: caste savelist\n\r", ch);
         send_to_char("-----------------------------------------------------------------------------\n\r", ch);
         send_to_char("Syntax: caste mob [vnum]\n\r", ch);
         send_to_char("Syntax: caste obj [vnum]\n\r", ch);
         send_to_char("Syntax: caste trainer [number]\n\r", ch);
         send_to_char("Syntax: caste silo [number]\n\r", ch);
         send_to_char("Syntax: caste add mob <vnum>\n\r", ch);
         send_to_char("Syntax: caste add obj <vnum>\n\r", ch);
         send_to_char("Syntax: caste add trainer\n\r", ch);
         send_to_char("Syntax: caste add silo\n\r", ch);
         send_to_char("Syntax: caste remove mob <vnum>\n\r", ch);
         send_to_char("Syntax: caste remove obj <vnum>\n\r", ch);
         send_to_char("Syntax: caste remove trainer <number>\n\r", ch);
         send_to_char("Syntax: caste remove silo <number>\n\r", ch);
         send_to_char("Syntax: caste edit mob <vnum> <option> <value>\n\r", ch);
         send_to_char("Syntax: caste edit obj <vnum> <option> <value>\n\r", ch);
         send_to_char("Syntax: caste edit trainer <number> <option> <value>\n\r", ch);
         send_to_char("Syntax: caste edit silo <number> <option> <value>\n\r", ch);
         send_to_char("Options(mob, obj) - tree, corn, grain, iron, gold, stone, coins, vnum, flags, mincaste\n\r", ch);
         send_to_char("Options(silo) - stone, coins, mincaste, hold\n\r", ch);
         send_to_char("Options(trainer) - sn1-sn20  mastery1-mastery20  name  cost\n\r", ch);
         send_to_char("---Type flags with no argument to see the flags that can be set---\n\r", ch);
         send_to_char("-----------------------------------------------------------------------------\n\r", ch);
         send_to_char("Syntax: caste fillresources <sector type> [amount]\n\r", ch);
      }  
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1 == '\0' || arg2 == '\0')
   {
      do_caste(ch, "");
      return;
   }
   if (!str_cmp(arg1, "set"))
   {
      if ((victim = get_char_world(ch, arg2)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }

      if (IS_NPC(victim))
      {
         send_to_char("NOT ON NPCs.\n\r", ch);
         return;
      }
      if (get_trust(ch) >= LEVEL_IMMORTAL)
      {
         if (get_trust(ch) <= get_trust(victim) && ch != victim)
         {
            send_to_char("Don't do that AGAIN!.\n\r", ch);
            return;
         }
      }
      else
      {
         if (ch->pcdata->caste < victim->pcdata->caste)
         {
            send_to_char("Cannot demote someone who is your own caste level or higher.\n\r", ch);
            return;
         }
      }

      if (!is_number(argument))
      {
         send_to_char("Syntax: caste set <name> <number>(help caste for info)\n\r", ch);
         return;
      }

      amount = atoi(argument);

      if (amount < 1 || amount > caste_Admin)
      {
         send_to_char("Syntax: caste set <name> <number>(help caste for info)\n\r", ch);
         return;
      }

      if (amount > ch->pcdata->caste)
      {
         send_to_char("Can only set as high as your own caste.\n\r", ch);
         return;
      }
      if (!IS_IMMORTAL(ch))
      {
         int fndo = 0;
         AREA_DATA *tarea;
         
         if ((!kingdom_table[ch->pcdata->hometown]->number1 || str_cmp(victim->name, kingdom_table[ch->pcdata->hometown]->number1))
         && ( !kingdom_table[ch->pcdata->hometown]->number2 || str_cmp(victim->name, kingdom_table[ch->pcdata->hometown]->number2)))
         {
            for (tarea = first_area; tarea; tarea = tarea->next)
            {
               if (!str_cmp(tarea->kowner, victim->name))
               {
                  fndo = 1;
               }
            }
            if (fndo == 0)
            {
               if (amount > caste_Knight)
               {
                  send_to_char("The max you can set a non-land owning members is Knight.\n\r", ch);
                  return;
               }
            }
            else
            {
               if (amount > caste_Marquis)
               {
                  send_to_char("The max you can set land-owning members to is Marquis.\n\r", ch);
                  return;
               }
            }
         }
         else
         {
            send_to_char("You cannot change their caste, try removing their status first.\n\r", ch);
            return;
         }
      }                     
      victim->pcdata->caste = amount;
      return;
   }
   if (!str_cmp(arg1, "number1"))
   {
      if (ch->pcdata->caste < caste_King)
      {
         send_to_char("Sorry, for the king only.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "\0"))
      {
         if (!kingdom_table[ch->pcdata->hometown]->number1)
         {
            send_to_char("You can only remove the number1 if one exists.\n\r", ch);
            return;
         }
         if ((victim = get_char_room_new(ch, kingdom_table[ch->pcdata->hometown]->number1, 1)) != NULL)
         {
            STRFREE(kingdom_table[ch->pcdata->hometown]->number1);
            victim->pcdata->caste = caste_Peasant;
            send_to_char("Your king has stripped you of your powers and set you back to a Peasant!!!\n\r", victim);
            send_to_char("The number 1 has been removed and the player's caste set to Peasant.\n\r", ch);
            return;
         }
         else
         {
            STRFREE(kingdom_table[ch->pcdata->hometown]->number1);
            send_to_char("It is done, make sure to reduce his/her caste when he/she logs in.\n\r", ch);
            return;
         }
      }
      else
      {
         if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
         {
            send_to_char("That player has to be in the room with you to do that.\n\r", ch);
            return;
         }
         if (victim->position != POS_STANDING)
         {
            send_to_char("The appointee needs to be standing to do this.\n\r", ch);
            return;
         }
         if (victim->pcdata->hometown != ch->pcdata->hometown)
         {
            send_to_char("You can only do this to someone in your own kingdom.\n\r", ch);
            return;
         }
         if (kingdom_table[ch->pcdata->hometown]->number1)
            STRFREE(kingdom_table[ch->pcdata->hometown]->number1);  
            
         kingdom_table[ch->pcdata->hometown]->number1 = STRALLOC(victim->name);
         if (ch->sex == 1 || ch->sex == 0)
         {
            ch_printf(ch, "You command %s to kneel and you appoint him Prince of your Kingdom.\n\r", victim->name);
            ch_printf(victim, "%s commands you to kneel, and as soon as you do you are appointed Prince.\n\r", ch->name);
            act(AT_WHITE, "$n commands $N to kneel and appoints $N to the status of Prince of the kingdom.", ch, NULL, victim, TO_NOTVICT);
            victim->pcdata->caste = caste_Prince;
            return;
         }
         if (ch->sex == 2)
         {
            ch_printf(ch, "You command %s to kneel and you appoint her Princess of your Kingdom.\n\r", victim->name);
            ch_printf(victim, "%s commands you to kneel, and as soon as you do you are appointed Princess.\n\r", ch->name);
            act(AT_WHITE, "$n commands $N to kneel and appoints $N to the status of Princess of the kingdom.", ch, NULL, victim, TO_NOTVICT);
            victim->pcdata->caste = caste_Prince;
            return;
         }
         return;
      }
      return;
   } 
   if (!str_cmp(arg1, "number2"))
   {
      if (ch->pcdata->caste < caste_King)
      {
         send_to_char("Sorry, for the king only.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "\0"))
      {
         if (!kingdom_table[ch->pcdata->hometown]->number2)
         {
            send_to_char("You can only remove the number1 if one exists.\n\r", ch);
            return;
         }
         STRFREE(kingdom_table[ch->pcdata->hometown]->number2);
         if ((victim = get_char_room_new(ch, kingdom_table[ch->pcdata->hometown]->number2, 1)) == NULL)
         {
            victim->pcdata->caste = caste_Peasant;
            send_to_char("Your king has stripped you of your powers and set you back to a Peasant!!!\n\r", victim);
            send_to_char("The number 1 has been removed and the player's caste set to Peasant.\n\r", ch);
            return;
         }
         else
         {
            send_to_char("It is done, make sure to reduce his/her caste when he/she logs in.\n\r", ch);
            return;
         }
      }
      else
      {
         if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
         {
            send_to_char("That player has to be in the room with you to do that.\n\r", ch);
            return;
         }
         if (victim->position != POS_STANDING)
         {
            send_to_char("The appointee needs to be standing to do this.\n\r", ch);
            return;
         }
         if (victim->pcdata->hometown != ch->pcdata->hometown)
         {
            send_to_char("You can only do this to someone in your own kingdom.\n\r", ch);
            return;
         }
         if (kingdom_table[ch->pcdata->hometown]->number2)
            STRFREE(kingdom_table[ch->pcdata->hometown]->number2);  
            
         kingdom_table[ch->pcdata->hometown]->number2 = STRALLOC(victim->name);
         if (ch->sex == 1 || ch->sex == 0)
         {
            ch_printf(ch, "You command %s to kneel and you appoint him Minister of your Kingdom.\n\r", victim->name);
            ch_printf(victim, "%s commands you to kneel, and as soon as you do you are appointed Minister.\n\r", ch->name);
            act(AT_WHITE, "$n commands $N to kneel and appoints $N to the status of Minister of the kingdom.", ch, NULL, victim, TO_NOTVICT);
            victim->pcdata->caste = caste_Minister;
            return;
         }
         if (ch->sex == 2)
         {
            ch_printf(ch, "You command %s to kneel and you appoint her Minister of your Kingdom.\n\r", victim->name);
            ch_printf(victim, "%s commands you to kneel, and as soon as you do you are appointed Minister.\n\r", ch->name);
            act(AT_WHITE, "$n commands $N to kneel and appoints $N to the status of Minister of the kingdom.", ch, NULL, victim, TO_NOTVICT);
            victim->pcdata->caste = caste_Minister;
            return;
         }
         return;
      }
      return;
   }    
   if (!str_cmp(arg1, "fillresources"))
   {
      int amt;
      
      if (!IN_WILDERNESS(ch))
      {
         send_to_char("You can only do this out in the wilderness.\n\r", ch);
         return;
      }
      if (atoi(arg2) < 0 || atoi(arg2) >= SECT_MAX)
      {
         send_to_char("That is not a valid sectortype.\n\r", ch);
         return;
      }
      if (isdigit(argument[0]))
      {
         amt = atoi(argument);
         if (amt < 0 || amt > 6000)
         {
            send_to_char("The range is 0 to 6000.\n\r", ch);
            return;
         }
         if (atoi(arg2) == SECT_MINEGOLD || atoi(arg2) == SECT_MINEIRON
         || atoi(arg2) == SECT_SGOLD || atoi(arg2) == SECT_NGOLD
         || atoi(arg2) == SECT_SIRON || atoi(arg2) == SECT_NIRON)
         {
            if (amt > 3000)
            {
               send_to_char("Max is 3000 for mining, amt changed to that value.\n\r", ch);
               amt = 3000;
            }
         }
      }
      else
      {
         if (atoi(arg2) == SECT_MINEGOLD || atoi(arg2) == SECT_MINEIRON
         || atoi(arg2) == SECT_SGOLD || atoi(arg2) == SECT_NGOLD
         || atoi(arg2) == SECT_SIRON || atoi(arg2) == SECT_NIRON)
            amt = 3000;
         else
            amt = 6000;
      } 
      for (x = 1; x <= MAX_X; x++)
      {
         for (y = 1; y <= MAX_Y; y++)
         {
            if ((int) map_sector[ch->map][x][y] == atoi(arg2))
            {
               resource_sector[ch->map][x][y] = amt;
            }
         }
      }
      send_to_char("All set.\n\r", ch);
      return;
   }          
   if (!str_cmp(arg1, "add") || !str_cmp(arg1, "remove") || !str_cmp(arg1, "edit")
   ||  !str_cmp(arg1, "mob") || !str_cmp(arg1, "obj") || !str_cmp(arg1, "savelist")
   ||  !str_cmp(arg1, "trainer") || !str_cmp(arg1, "silo"))
   {
      if (!str_cmp(arg1, "savelist"))
      {
         save_buykingdom_data();
         send_to_char("Saved.\n\r", ch);
         return;
      }
      if (!str_cmp(arg1, "mob") || !str_cmp(arg1, "mobs"))
      {
         if (arg2[0] == '\0')
         {
            ch_printf(ch, "Vnum   Name                            Lumber   Corn     Grain    Iron     Gold     Stone    Coins    MinCaste\n\r");
            for (kmob = first_buykmob; kmob; kmob = kmob->next)
            {
               mob = get_mob_index(kmob->vnum);
               if (!mob)
               {
                  bug("caste: Invalid mob vnum of %d in the buy mob kingdom list.\n\r", kmob->vnum);
                  continue;
               }
               ch_printf(ch, "%-5d  %-30s  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-2d", kmob->vnum, mob->short_descr,
                  kmob->tree, kmob->corn, kmob->grain, kmob->iron, kmob->gold, kmob->stone, kmob->coins, kmob->mincaste);
               ch_printf(ch, "\n\rFlags: %s\n\r", ext_flag_string(&kmob->flags, buykmob_types));
            }
            return;
         }
         else
         {
            if (!isdigit(arg2[0]))
            {
               send_to_char("Only accepts vnums of mobs.\n\r", ch);
               return;
            }
            for (kmob = first_buykmob; kmob; kmob = kmob->next)
            {
               if (kmob->vnum == atoi(arg2))
               {
                  ch_printf(ch, "Vnum   Name                            Lumber   Corn     Grain    Iron     Gold     Stone    Coins    MinCaste\n\r");
                  
                  mob = get_mob_index(kmob->vnum);
                  if (!mob)
                  {
                     bug("caste: Invalid mob vnum of %d in the buy mob kingdom list.\n\r", kmob->vnum);
                     continue;
                  }
                  ch_printf(ch, "%-5d  %-30s  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-2d", kmob->vnum, mob->short_descr,
                     kmob->tree, kmob->corn, kmob->grain, kmob->iron, kmob->gold, kmob->stone, kmob->coins, kmob->mincaste);
                  ch_printf(ch, "\n\rFlags: %s\n\r", ext_flag_string(&kmob->flags, buykmob_types));
               }
            }
         }
         return;
      }
      
      if (!str_cmp(arg1, "trainer") || !str_cmp(arg1, "trainers"))
      {
         if (arg2[0] == '\0')
         {
            ch_printf(ch, "Num  Name             Cost    Sn1  Mastery1  Sn2  Mastery2  Sn3  Mastery3  Sn4  Mastery4  Sn5  Mastery5  Pid\n\r");
            for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
            {
               cnt++;
               ch_printf(ch, "%-3d  %-15s  %-6d  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d\n\r",
                  cnt, ktrainer->name, ktrainer->cost, ktrainer->sn[0], get_mastery_name(ktrainer->mastery[0]), 
                  ktrainer->sn[1], get_mastery_name(ktrainer->mastery[1]), 
                  ktrainer->sn[2], get_mastery_name(ktrainer->mastery[2]), 
                  ktrainer->sn[3], get_mastery_name(ktrainer->mastery[3]), 
                  ktrainer->sn[4], get_mastery_name(ktrainer->mastery[4]), ktrainer->pid); 
               if (ktrainer->sn[5] > 0)
               {
                  ch_printf(ch, "                              %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s\n\r",
                     ktrainer->sn[5], get_mastery_name(ktrainer->mastery[5]), 
                     ktrainer->sn[6], get_mastery_name(ktrainer->mastery[6]), 
                     ktrainer->sn[7], get_mastery_name(ktrainer->mastery[7]), 
                     ktrainer->sn[8], get_mastery_name(ktrainer->mastery[8]), 
                     ktrainer->sn[9], get_mastery_name(ktrainer->mastery[9])); 
               }
               if (ktrainer->sn[10] > 0)
               {
                  ch_printf(ch, "                              %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s\n\r",
                     ktrainer->sn[10], get_mastery_name(ktrainer->mastery[10]), 
                     ktrainer->sn[11], get_mastery_name(ktrainer->mastery[11]), 
                     ktrainer->sn[12], get_mastery_name(ktrainer->mastery[12]), 
                     ktrainer->sn[13], get_mastery_name(ktrainer->mastery[13]), 
                     ktrainer->sn[14], get_mastery_name(ktrainer->mastery[14])); 
               }
               if (ktrainer->sn[15] > 0)
               {
                  ch_printf(ch, "                              %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s\n\r",
                     ktrainer->sn[15], get_mastery_name(ktrainer->mastery[15]), 
                     ktrainer->sn[16], get_mastery_name(ktrainer->mastery[16]), 
                     ktrainer->sn[17], get_mastery_name(ktrainer->mastery[17]), 
                     ktrainer->sn[18], get_mastery_name(ktrainer->mastery[18]), 
                     ktrainer->sn[19], get_mastery_name(ktrainer->mastery[19])); 
               }
            }
            return;
         }
         else
         {
            if (!isdigit(arg2[0]))
            {
               send_to_char("Only accepts the number of the trainer.\n\r", ch);
               return;
            }
            for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
            {
               cnt++;
               if (cnt == atoi(arg2))
               {
                  ch_printf(ch, "Num  Name             Cost    Sn1  Mastery1  Sn2  Mastery2  Sn3  Mastery3  Sn4  Mastery4  Sn5  Mastery5  Pid\n\r");
                  ch_printf(ch, "%-3d  %-15s  %-6d  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s\n\r",
                     cnt, ktrainer->name, ktrainer->cost, ktrainer->sn[0], get_mastery_name(ktrainer->mastery[0]), 
                     ktrainer->sn[1], get_mastery_name(ktrainer->mastery[1]), 
                     ktrainer->sn[2], get_mastery_name(ktrainer->mastery[2]), 
                     ktrainer->sn[3], get_mastery_name(ktrainer->mastery[3]), 
                     ktrainer->sn[4], get_mastery_name(ktrainer->mastery[4]), ktrainer->pid); 
                  if (ktrainer->sn[5] > 0)
                  {
                     ch_printf(ch, "                              %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s\n\r",
                        ktrainer->sn[5], get_mastery_name(ktrainer->mastery[5]), 
                        ktrainer->sn[6], get_mastery_name(ktrainer->mastery[6]), 
                        ktrainer->sn[7], get_mastery_name(ktrainer->mastery[7]), 
                        ktrainer->sn[8], get_mastery_name(ktrainer->mastery[8]), 
                        ktrainer->sn[9], get_mastery_name(ktrainer->mastery[9])); 
                  }
                  if (ktrainer->sn[10] > 0)
                  {
                     ch_printf(ch, "                              %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s\n\r",
                        ktrainer->sn[10], get_mastery_name(ktrainer->mastery[10]), 
                        ktrainer->sn[11], get_mastery_name(ktrainer->mastery[11]), 
                        ktrainer->sn[12], get_mastery_name(ktrainer->mastery[12]), 
                        ktrainer->sn[13], get_mastery_name(ktrainer->mastery[13]), 
                        ktrainer->sn[14], get_mastery_name(ktrainer->mastery[14])); 
                  }
                  if (ktrainer->sn[15] > 0)
                  {
                     ch_printf(ch, "                              %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s  %-3d  %-8s\n\r",
                        ktrainer->sn[15], get_mastery_name(ktrainer->mastery[15]), 
                        ktrainer->sn[16], get_mastery_name(ktrainer->mastery[16]), 
                        ktrainer->sn[17], get_mastery_name(ktrainer->mastery[17]), 
                        ktrainer->sn[18], get_mastery_name(ktrainer->mastery[18]), 
                        ktrainer->sn[19], get_mastery_name(ktrainer->mastery[19])); 
                  }
               }
            }
         }
         return;
      }
      if (!str_cmp(arg1, "silo") || !str_cmp(arg1, "silos"))
      {
         if (arg2[0] == '\0')
         {
            ch_printf(ch, "Num  Name                       Cost      Stone   Hold      MinCaste\n\r");
            for (kbin = first_buykbin; kbin; kbin = kbin->next)
            {
               cnt++;
               ch_printf(ch, "%-3d  %-25s  %-8d  %-6d  %-8d  %-3d\n\r",
                  cnt, kbin->name, kbin->coins, kbin->stone, kbin->hold, kbin->mincaste);
            }
            return;
         }
         else
         {
            if (!isdigit(arg2[0]))
            {
               send_to_char("Only accepts the number of the silo.\n\r", ch);
               return;
            }
            for (kbin = first_buykbin; kbin; kbin = kbin->next)
            {
               cnt++;
               if (cnt == atoi(arg2))
               {
                  ch_printf(ch, "Num  Name             Cost      Stone   Hold      MinCaste\n\r");
                  ch_printf(ch, "%-3d  %-15s  %-8d  %-6d  %-8s  %-3d\n\r",
                     cnt, kbin->name, kbin->coins, kbin->stone, kbin->hold, kbin->mincaste);
               }
            }
         }
         return;
      }
      
      if (!str_cmp(arg1, "obj") || !str_cmp(arg1, "objs"))
      {
         if (arg2[0] == '\0')
         {
            ch_printf(ch, "Vnum   Name                            Lumber   Corn     Grain    Iron     Gold     Stone    Coins    MinCaste\n\r");
            for (kobj = first_buykobj; kobj; kobj = kobj->next)
            {
               obj = get_obj_index(kobj->vnum);
               if (!obj)
               {
                  bug("caste: Invalid obj vnum of %d in the buy obj kingdom list.\n\r", kobj->vnum);
                  continue;
               }
               ch_printf(ch, "%-5d  %-30s  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-2d", kobj->vnum, obj->short_descr,
                  kobj->tree, kobj->corn, kobj->grain, kobj->iron, kobj->gold, kobj->stone, kobj->coins, kobj->mincaste);
               ch_printf(ch, "\n\rFlags: %s\n\r", ext_flag_string(&kobj->flags, buykobj_types));
            }
            return;
         }
         else
         {
            if (!isdigit(arg2[0]))
            {
               send_to_char("Only accepts vnums of objs.\n\r", ch);
               return;
            }
            for (kobj = first_buykobj; kobj; kobj = kobj->next)
            {
               if (kobj->vnum == atoi(arg2))
               {
                  ch_printf(ch, "Vnum   Name                            Lumber   Corn     Grain    Iron     Gold     Stone    Coins    MinCaste\n\r");
                  
                  obj = get_obj_index(kobj->vnum);
                  if (!obj)
                  {
                     bug("caste: Invalid obj vnum of %d in the buy obj kingdom list.\n\r", kobj->vnum);
                     continue;
                  }
                  ch_printf(ch, "%-5d  %-30s  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-7d  %-2d", kobj->vnum, obj->short_descr,
                     kobj->tree, kobj->corn, kobj->grain, kobj->iron, kobj->gold, kobj->stone, kobj->coins, kobj->mincaste);
                  ch_printf(ch, "\n\rFlags: %s\n\r", ext_flag_string(&kobj->flags, buykobj_types));
               }
            }
         }
         return;
      }
      if (!str_cmp(arg1, "edit"))
      {
         if (!str_cmp(arg2, "mob"))
         {
            argument = one_argument(argument, arg3);
            argument = one_argument(argument, arg4);
            if (!isdigit(arg3[0]))
            {
               send_to_char("Only accepts vnums of mobs.\n\r", ch);
               return;
            }
            for (kmob = first_buykmob; kmob; kmob = kmob->next)
            {
               if (kmob->vnum == atoi(arg3))
                  break;
            }
            if (!kmob)
            {
               send_to_char("No such mob has been added.\n\r", ch);
               return;
            } 
            if (!str_cmp(arg4, "tree") || !str_cmp(arg4, "lumber"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kmob->tree = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "corn"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kmob->corn = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "grain"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kmob->grain = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "iron"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kmob->iron = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "gold"))
            {
              if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kmob->gold = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "stone"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kmob->stone = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "coins"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kmob->coins = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "vnum"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_VNUM)
               {
                  ch_printf(ch, "Range is 1 to %d\n\r", MAX_VNUM);
                  return;
               }
               mob = get_mob_index(atoi(argument));  
               if (!mob)
               {
                  send_to_char("That mob does not exist.\n\r", ch);
                  return;
               }
               kmob->vnum = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "flags"))
            {
               if (argument[0] == '\0')
               {
                  send_to_char("Flags available: wilderness  military  hour   4month\n\r", ch);
                  return;
               }
               strcpy(arg4, "");
               while (argument[0] != '\0')
               {
                  argument = one_argument(argument, arg4);
                  flag = get_buykmobflag(arg4);
                  
                  if (flag < 0 || flag > MAX_BITS)
                     ch_printf(ch, "Unknown flag: %s\n\r", arg4);
                  else
                     xTOGGLE_BIT(kmob->flags, flag);
               }
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "mincaste"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_CASTE)
               {
                  ch_printf(ch, "Range is 1 to %d\n\r", MAX_CASTE);
                  return;
               }
               kmob->mincaste = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            send_to_char("Invalid selection.\n\r", ch);
            return;
         }
         if (!str_cmp(arg2, "obj"))
         {
            argument = one_argument(argument, arg3);
            argument = one_argument(argument, arg4);
            if (!isdigit(arg3[0]))
            {
               send_to_char("Only accepts vnums of objs.\n\r", ch);
               return;
            }
            for (kobj = first_buykobj; kobj; kobj = kobj->next)
            {
               if (kobj->vnum == atoi(arg3))
                  break;
            }
            if (!kobj)
            {
               send_to_char("No such obj has been added.\n\r", ch);
               return;
            } 
            if (!str_cmp(arg4, "tree") || !str_cmp(arg4, "lumber"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kobj->tree = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "corn"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kobj->corn = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "grain"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kobj->grain = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "iron"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kobj->iron = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "gold"))
            {
              if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kobj->gold = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "stone"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kobj->stone = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "coins"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 9999999)
               {
                  send_to_char("Range is 0 (not needed) to 9,999,999\n\r", ch);
                  return;
               }
               kobj->coins = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "vnum"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_VNUM)
               {
                  ch_printf(ch, "Range is 1 to %d\n\r", MAX_VNUM);
                  return;
               }
               obj = get_obj_index(atoi(argument));  
               if (!obj)
               {
                  send_to_char("That obj does not exist.\n\r", ch);
                  return;
               }
               kobj->vnum = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "flags"))
            {
               if (argument[0] == '\0')
               {
                  send_to_char("Flags available: hometown   wilderness   container   sigil  noteboard  toroom\n\r", ch);
                  send_to_char("                 tochar     addreset     bin\n\r", ch);
                  return;
               }
               strcpy(arg4, "");
               while (argument[0] != '\0')
               {
                  argument = one_argument(argument, arg4);
                  flag = get_buykobjflag(arg4);
                  
                  if (flag < 0 || flag > MAX_BITS)
                     ch_printf(ch, "Unknown flag: %s\n\r", arg4);
                  else
                     xTOGGLE_BIT(kobj->flags, flag);
               }
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "mincaste"))
            {
               if (atoi(argument) < 1 || atoi(argument) > MAX_CASTE)
               {
                  ch_printf(ch, "Range is 1 to %d\n\r", MAX_CASTE);
                  return;
               }
               kobj->mincaste = atoi(argument);
               return;
            }
            send_to_char("Invalid selection.\n\r", ch);
            return;
         }
         if (!str_cmp(arg2, "silo"))
         {
            argument = one_argument(argument, arg3);
            argument = one_argument(argument, arg4);
            if (!isdigit(arg3[0]))
            {
               send_to_char("Only accepts the number of the silo.\n\r", ch);
               return;
            }
            for (kbin = first_buykbin; kbin; kbin = kbin->next)
            {
               cnt++;
               if (cnt == atoi(arg3))
                  break;
            }
            if (!kbin)
            {
               send_to_char("No such silo has been added.\n\r", ch);
               return;
            } 
            if (!str_cmp(arg4, "coins") || !str_cmp(arg4, "cost"))
            {
               if (atoi(argument) < 1000 || atoi(argument) > 99999999)
               {
                  send_to_char("Max range is 1000 to 99,999,999.\n\r", ch);
                  return;
               }
               kbin->coins = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "stone"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 999999)
               {
                  send_to_char("Max range is 0 to 999,999.\n\r", ch);
                  return;
               }
               kbin->stone = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;  
            }
            if (!str_cmp(arg4, "hold"))
            {
               if (atoi(argument) < 1000 || atoi(argument) > 99999999)
               {
                  send_to_char("Max range is 1000 to 99,999,999.\n\r", ch);
                  return;
               }
               kbin->hold = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "name"))
            {
               STRFREE(kbin->name);
               kbin->name = STRALLOC(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            send_to_char("Invalid selection.\n\r", ch);
            return;
         }     
         if (!str_cmp(arg2, "trainer"))
         {
            int tcnt;
            char snbuf1[MSL];
            char masterybuf1[MSL];
            int tsn;
            
            argument = one_argument(argument, arg3);
            argument = one_argument(argument, arg4);
            if (!isdigit(arg3[0]))
            {
               send_to_char("Only accepts the number of the trainer..\n\r", ch);
               return;
            }
            for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
            {
               cnt++;
               if (cnt == atoi(arg3))
                  break;
            }
            if (!ktrainer)
            {
               send_to_char("No such trainer has been added.\n\r", ch);
               return;
            } 
            for (tcnt = 0; tcnt < 20; tcnt++)
            {
               sprintf(snbuf1, "sn%d", tcnt + 1);
               sprintf(masterybuf1, "mastery%d", tcnt + 1);
               
               if (!str_cmp(arg4, snbuf1))
               {
                  if (atoi(argument) < 0 || atoi(argument) > MAX_SKILL)
                  {
                     ch_printf(ch, "Max range is 0 to %d.\n\r", MAX_SKILL);
                     return;
                  }
                  if (skill_lookup(argument) == -1)
                  {
                     send_to_char("Invalid Skill or Spell.\n\r", ch);
                     return;
                  }
                  ktrainer->sn[tcnt] = skill_lookup(argument);
                  send_to_char("Done.\n\r", ch);
                  return;
               }
               if (!str_cmp(arg4, masterybuf1))
               {
                  if (isdigit(argument[0]))
                  {
                     if (atoi(argument) < 0 || atoi(argument) > MAX_RANKING)
                     {
                        ch_printf(ch, "The range for mastery is 0 to %d", MAX_RANKING);
                        return;
                     }
                     tsn = atoi(argument);
                  }
                  else
                  {
                     tsn = get_mastery_num(argument);
                     if (tsn == -1)
                     {
                        send_to_char("That is not a valid Mastery.\n\r", ch);
                        return;
                     }
                  }
                  ktrainer->mastery[tcnt] = tsn;
                  send_to_char("Done.\n\r", ch);
                  return;
               }
            }
           
            if (!str_cmp(arg4, "pid"))
            {
               if (atoi(argument) < 0 || atoi(argument) > 999)
               {
                  ch_printf(ch, "Range is 1 to 999.\n\r", ch);
                  return;
               }
               ktrainer->pid = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "cost"))
            {
               if (atoi(argument) < 1000 || atoi(argument) > 9999999)
               {
                  send_to_char("Max range is 1000 to 9,999,999.\n\r", ch);
                  return;
               }
               ktrainer->cost = atoi(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            if (!str_cmp(arg4, "name"))
            {
               STRFREE(ktrainer->name);
               ktrainer->name = STRALLOC(argument);
               send_to_char("Done.\n\r", ch);
               return;
            }
            send_to_char("Invalid selection.\n\r", ch);
            return;
         }
         send_to_char("That is not an option.\n\r", ch);
         return;
      }
      if (!str_cmp(arg1, "remove"))
      {
         if (!str_cmp(arg2, "silo"))
         {
            if (!isdigit(argument[0]))
            {
               send_to_char("Only accepts the number of the silo.\n\r", ch);
               return;
            }
            for (kbin = first_buykbin; kbin; kbin = kbin->next)
            {
               cnt++;
               if (cnt == atoi(argument))
                  break;
            }
            if (!kbin)
            {
               send_to_char("No such silo has been added.\n\r", ch);
               return;
            } 
            UNLINK(kbin, first_buykbin, last_buykbin, next, prev);
            STRFREE(kbin->name);
            DISPOSE(kbin);
            send_to_char("That bin has been deleted.\n\r", ch);
            return;
         }   
         if (!str_cmp(arg2, "trainer"))
         {
            if (!isdigit(argument[0]))
            {
               send_to_char("Only accepts the number of the trainer.\n\r", ch);
               return;
            }
            for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
            {
               cnt++;
               if (cnt == atoi(argument))
                  break;
            }
            if (!ktrainer)
            {
               send_to_char("No such trainer has been added.\n\r", ch);
               return;
            } 
            UNLINK(ktrainer, first_buyktrainer, last_buyktrainer, next, prev);
            STRFREE(ktrainer->name);
            DISPOSE(ktrainer);
            send_to_char("That trainer has been deleted.\n\r", ch);
            return;
         }     
         if (!str_cmp(arg2, "mob"))
         {
            if (!isdigit(argument[0]))
            {
               send_to_char("Only accepts vnums of mobs.\n\r", ch);
               return;
            }
            for (kmob = first_buykmob; kmob; kmob = kmob->next)
            {
               if (kmob->vnum == atoi(argument))
                  break;
            }
            if (!kmob)
            {
               send_to_char("No such mob has been added.\n\r", ch);
               return;
            } 
            UNLINK(kmob, first_buykmob, last_buykmob, next, prev);
            DISPOSE(kmob);
            send_to_char("That mob has been deleted.\n\r", ch);
            return;
         }    
         if (!str_cmp(arg2, "obj"))
         {
            if (!isdigit(argument[0]))
            {
               send_to_char("Only accepts vnums of objs.\n\r", ch);
               return;
            }
            for (kobj = first_buykobj; kobj; kobj = kobj->next)
            {
               if (kobj->vnum == atoi(argument))
                  break;
            }
            if (!kobj)
            {
               send_to_char("No such obj has been added.\n\r", ch);
               return;
            } 
            UNLINK(kobj, first_buykobj, last_buykobj, next, prev);
            DISPOSE(kobj);
            send_to_char("That obj has been deleted.\n\r", ch);
            return;
         }    
         return;
      }
      if (!str_cmp(arg1, "add"))
      {
         if (!str_cmp(arg2, "silo"))
         {
            CREATE(kbin, BUYKBIN_DATA, 1);
            LINK(kbin, first_buykbin, last_buykbin, next, prev);
            send_to_char("A New silo has been added.\n\r", ch);
            return;
         }
         if (!str_cmp(arg2, "trainer"))
         {
            CREATE(ktrainer, BUYKTRAINER_DATA, 1);
            LINK(ktrainer, first_buyktrainer, last_buyktrainer, next, prev);
            send_to_char("A New trainer has been added.\n\r", ch);
            return;
         }
         if (!str_cmp(arg2, "mob"))
         {
            if (!isdigit(argument[0]))
            {
               send_to_char("Only accepts vnums of mobs.\n\r", ch);
               return;
            }
            mob = get_mob_index(atoi(argument));   
            if (!mob)
            {
               send_to_char("No such mob has been created.\n\r", ch);
               return;
            }
            CREATE(kmob, BUYKMOB_DATA, 1);
            kmob->vnum = mob->vnum;
            LINK(kmob, first_buykmob, last_buykmob, next, prev);
            send_to_char("New mob has been added.\n\r", ch);
            return;
         }
         if (!str_cmp(arg2, "obj"))
         {
            if (!isdigit(argument[0]))
            {
               send_to_char("Only accepts vnums of objs.\n\r", ch);
               return;
            }
            obj = get_obj_index(atoi(argument));   
            if (!obj)
            {
               send_to_char("No such obj has been created.\n\r", ch);
               return;
            }
            CREATE(kobj, BUYKOBJ_DATA, 1);
            kobj->vnum = obj->vnum;
            LINK(kobj, first_buykobj, last_buykobj, next, prev);
            send_to_char("New obj has been added.\n\r", ch);
            return;
         }
         return;
      }
   }
}

void do_setjob(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   char jobhold[MSL];
   int num;
   CHAR_DATA *victim;

   argument = one_argument(argument, arg1);

   if (IS_NPC(ch))
   {
      send_to_char("Only players can appoint workers\n\r", ch);
      return;
   }
   /* Only Those who are High Appoint and God+ immortals can set job status for now */
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minhappoint ||
      ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend ||
      ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (arg1[0] == '\0')
   {
      send_to_char
         ("Syntax: setjob <character> <type>\n\rtype: 0 - Unemployed   1 - Extractor   2 - Merchant  3 - Gambler  4 - Carpenter  5 - Surveyor\n\r",
         ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char
         ("Syntax: setjob <character> <type>\n\rtype: 0 - Unemployed   1 - Extractor   2 - Merchant  3 - Gambler  4 - Carpenter  5 - Surveyor\n\r",
         ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Your target was not found.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("The victim cannot be a NPC either.\n\r", ch);
      return;
   }
   if (victim->pcdata->hometown != ch->pcdata->hometown)
   {
      send_to_char("They are not in your Kingdom, cannot do that.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPCs\n\r.", ch);
      return;
   }
   if (!is_number(argument))
   {
      send_to_char("Needs to be a number.\n\r", ch);
      return;
   }
   num = atoi(argument);
   if (num < 0 || num > 5)
   {
      send_to_char("Needs to be between 0 and 4.\n\r", ch);
      return;
   }
   if (num == 0)
      sprintf(jobhold, "unemployed");
   if (num == 1)
      sprintf(jobhold, "extractor");
   if (num == 2)
      sprintf(jobhold, "merchant");
   if (num == 3)
      sprintf(jobhold, "gambler");
   if (num == 4)
      sprintf(jobhold, "carpenter");
   if (num == 5)
      sprintf(jobhold, "surveyor");

   victim->pcdata->job = num;
   send_to_char("Done.\n\r", ch);
   sprintf(buf, "Job change: %s set %s to a job of %s\n\r", ch->name, victim->name, jobhold);
   sprintf(logb, "Job change: %s set %s to a job of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), PERS_KINGDOM(victim, ch->pcdata->hometown), jobhold);
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_SETJOB);
   log_string_plus(buf, LOG_COMM, LEVEL_IMM);

}

/****************************
 Below are the worker type
 jobs.  Allows for extracting
 natural goods from the world
 --Xerves 12/99
 ****************************/
 // type...0 - Empty is part of the groug 1 - Empty is not part of the group.
 
int get_resourcetype(int sector, int type)
{
   int res = KRES_UNKNOWN;

   if (sector == SECT_MINEGOLD || sector == SECT_SGOLD)
      res = KRES_GOLD;
   if (sector == SECT_MINEIRON || sector == SECT_SIRON)
      res = KRES_IRON;
   if (sector == SECT_HCORN || sector == SECT_SCORN)
      res = KRES_CORN;
   if (sector == SECT_HGRAIN || sector == SECT_SGRAIN)
      res = KRES_GRAIN;
   if (sector == SECT_FOREST || sector == SECT_STREE)
      res = KRES_LUMBER;
   if (sector == SECT_STONE || sector == SECT_SSTONE)
      res = KRES_STONE;
   if (sector == SECT_WATER_NOSWIM || sector == SECT_RIVER)
      res = KRES_FISH;


   if (type == 0)
   {
      if (sector == SECT_NGOLD)
         res = KRES_GOLD;
      if (sector == SECT_NIRON)
         res = KRES_IRON;
      if (sector == SECT_NCORN)
         res = KRES_CORN;
      if (sector == SECT_NGRAIN)
         res = KRES_GRAIN;
      if (sector == SECT_NTREE)
         res = KRES_LUMBER;
      if (sector == SECT_NSTONE)
         res = KRES_STONE;
      if (sector == SECT_WATER_NOSWIM || sector == SECT_RIVER)
         res = KRES_FISH;
   }

   return res;
}

void do_showresources(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("Showresources is only for player.\n\r", ch);
      return;
   }
   if (ch->pcdata->resourcetype > 0)
   {
      if (ch->pcdata->resourcetype == KRES_GOLD)
      {
         ch_printf_color(ch, "Type:  Gold       Amount: %d\n\r", ch->pcdata->resource);
         return;
      }
      if (ch->pcdata->resourcetype == KRES_IRON)
      {
         ch_printf_color(ch, "Type:  Iron       Amount: %d\n\r", ch->pcdata->resource);
         return;
      }
      if (ch->pcdata->resourcetype == KRES_CORN)
      {
         ch_printf_color(ch, "Type:  Corn       Amount: %d\n\r", ch->pcdata->resource);
         return;
      }
      if (ch->pcdata->resourcetype == KRES_GRAIN)
      {
         ch_printf_color(ch, "Type:  Grain      Amount: %d\n\r", ch->pcdata->resource);
         return;
      }
      if (ch->pcdata->resourcetype == KRES_LUMBER)
      {
         ch_printf_color(ch, "Type:  Wood       Amount: %d\n\r", ch->pcdata->resource);
         return;
      }
      if (ch->pcdata->resourcetype == KRES_STONE)
      {
         ch_printf_color(ch, "Type:  Stone      Amount: %d\n\r", ch->pcdata->resource);
         return;
      }
      if (ch->pcdata->resourcetype == KRES_FISH)
      {
         ch_printf_color(ch, "Type:  Fish       Amount: %d\n\r", ch->pcdata->resource);
         return;
      }
   }
   else
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
}

void remove_resources(CHAR_DATA * ch, BUYKOBJ_DATA * kobj)
{
    
   ch->pcdata->town->lumber -= kobj->tree;
   ch->pcdata->town->corn -= kobj->corn;
   ch->pcdata->town->grain -= kobj->grain;
   ch->pcdata->town->iron -= kobj->iron;
   ch->pcdata->town->gold -= kobj->gold;
   ch->pcdata->town->stone -= kobj->stone;
   ch->pcdata->town->coins -= kobj->coins;
}

void remove_resources_mobs(CHAR_DATA * ch, BUYKMOB_DATA * kmob)
{    
   ch->pcdata->town->lumber -= kmob->tree;
   ch->pcdata->town->corn -= kmob->corn;
   ch->pcdata->town->grain -= kmob->grain;
   ch->pcdata->town->iron -= kmob->iron;
   ch->pcdata->town->gold -= kmob->gold;
   ch->pcdata->town->stone -= kmob->stone;
   ch->pcdata->town->coins -= kmob->coins;
}

void remove_resources_mobs_town(TOWN_DATA *town, BUYKMOB_DATA * kmob)
{    
   town->lumber -= kmob->tree;
   town->corn -= kmob->corn;
   town->grain -= kmob->grain;
   town->iron -= kmob->iron;
   town->gold -= kmob->gold;
   town->stone -= kmob->stone;
   town->coins -= kmob->coins;
}

bool proper_resources(CHAR_DATA * ch, BUYKOBJ_DATA *kobj)
{
   if (kobj->tree > ch->pcdata->town->lumber)
      return FALSE;
   if (kobj->corn > ch->pcdata->town->corn)
      return FALSE;
   if (kobj->grain > ch->pcdata->town->grain)
      return FALSE;
   if (kobj->iron > ch->pcdata->town->iron)
      return FALSE;
   if (kobj->gold > ch->pcdata->town->gold)
      return FALSE;
   if (kobj->stone > ch->pcdata->town->stone)
      return FALSE;
   if (kobj->coins > ch->pcdata->town->coins)
      return FALSE;
      
   return TRUE;
}

bool proper_resources_mobs_town(TOWN_DATA *town, BUYKMOB_DATA *kmob)
{
   if (kmob->tree > town->lumber)
      return FALSE;
   if (kmob->corn > town->corn)
      return FALSE;
   if (kmob->grain > town->grain)
      return FALSE;
   if (kmob->iron > town->iron)
      return FALSE;
   if (kmob->gold > town->gold)
      return FALSE;
   if (kmob->stone > town->stone)
      return FALSE;
   if (kmob->coins > town->coins)
      return FALSE;
      
   return TRUE;
}

bool proper_resources_mobs(CHAR_DATA * ch, BUYKMOB_DATA *kmob)
{
   if (kmob->tree > ch->pcdata->town->lumber)
      return FALSE;
   if (kmob->corn > ch->pcdata->town->corn)
      return FALSE;
   if (kmob->grain > ch->pcdata->town->grain)
      return FALSE;
   if (kmob->iron > ch->pcdata->town->iron)
      return FALSE;
   if (kmob->gold > ch->pcdata->town->gold)
      return FALSE;
   if (kmob->stone > ch->pcdata->town->stone)
      return FALSE;
   if (kmob->coins > ch->pcdata->town->coins)
      return FALSE;
      
   return TRUE;
}

void do_clearques(CHAR_DATA *ch, char *argument)
{
   int x;
   
   if (kingdom_table[ch->pcdata->hometown]->minplace > ch->pcdata->caste)
   {
      send_to_char("You cannot use this command.\n\r", ch);
      return;
   } 
   
   for (x = 0; x <= 24; x++)
   {
      kingdom_table[ch->pcdata->hometown]->mob_que[x] = 0;
      kingdom_table[ch->pcdata->hometown]->obj_que[x] = 0;
      kingdom_table[ch->pcdata->hometown]->trainer_que[x] = 0;
   }
   send_to_char("Cleared.\n\r", ch);
   return;
}
void fix_kingdom_mque(CHAR_DATA * ch, int val)
{
   int x;

   kingdom_table[ch->pcdata->hometown]->mob_que[val] = 0;

   if (val == 24)
      return;

   if (kingdom_table[ch->pcdata->hometown]->mob_que[val + 1] == 0)
      return;
   else
   {
      for (x = val; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->mob_que[x + 1] == 0)
         {
            kingdom_table[ch->pcdata->hometown]->mob_que[x] = 0; //Clears out the last one in the list since it is reducing
            break;
         }
         else
         {
            kingdom_table[ch->pcdata->hometown]->mob_que[x] = kingdom_table[ch->pcdata->hometown]->mob_que[x + 1];
         }
      }
   }
   return;
}
void fix_kingdom_oque(CHAR_DATA * ch, int val)
{
   int x;

   kingdom_table[ch->pcdata->hometown]->obj_que[val] = 0;

   if (val == 24)
      return;

   if (kingdom_table[ch->pcdata->hometown]->obj_que[val + 1] == 0)
      return;
   else
   {
      for (x = val; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->obj_que[x + 1] == 0)
         {
            kingdom_table[ch->pcdata->hometown]->obj_que[x] = 0; //Clears out the last one in the list since it is reducing
            break;
         }
         else
         {
            kingdom_table[ch->pcdata->hometown]->obj_que[x] = kingdom_table[ch->pcdata->hometown]->obj_que[x + 1];
         }
      }
   }
   return;
}

void fix_kingdom_tque(CHAR_DATA * ch, int val)
{
   int x;

   kingdom_table[ch->pcdata->hometown]->trainer_que[val] = 0;

   if (val == 24)
      return;

   if (kingdom_table[ch->pcdata->hometown]->trainer_que[val + 1] == 0)
      return;
   else
   {
      for (x = val; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->trainer_que[x + 1] == 0)
         {
            kingdom_table[ch->pcdata->hometown]->trainer_que[x] = 0; //Clears out the last one in the list since it is reducing
            break;
         }
         else
         {
            kingdom_table[ch->pcdata->hometown]->trainer_que[x] = kingdom_table[ch->pcdata->hometown]->trainer_que[x + 1];
         }
      }
   }
   return;
}

int get_buy_good(int type)
{
   int month = getday() / 30;

   if (type == 5) //Lumber
   {
      return 30;
   }
   if (type == 3) //Corn
   {
      if (month >= 5 && month <= 8)
         return 13;
      else
         return (UMAX(13, 13 * (abs(month - 5) / 2)));
   }
   if (type == 4) //Grain
   {
      if (month >= 3 && month <= 6)
         return 13;
      else
         return (UMAX(13, 13 * (abs(month - 5) / 2)));
   }
   if (type == 6) //Stone
   {
      return 22;
   }
   if (type == 1) //Gold
   {
      return 2000;
   }
   if (type == 2) //Iron
   {
      return 1600;
   }
   return 0;
}

int get_sell_good(int type)
{
   int month = getday() / 30;

   if (type == 5) //Lumber
   {
      return 9;
   }
   if (type == 3) //Corn
   {
      if (month >= 5 && month <= 8)
         return 4;
      else
         return (UMAX(4, 4 * (abs(month - 5) / 2)));
   }
   if (type == 4) //Grain
   {
      if (month >= 3 && month <= 6)
         return 4;
      else
         return (UMAX(4, 4 * (abs(month - 5) / 2)));
   }
   if (type == 6) //Stone
   {
      return 7;
   }
   if (type == 1) //Gold
   {
      return 550;
   }
   if (type == 2) //Iron
   {
      return 350;
   }
   return 0;
}

char *get_resources_traded(TRADE_DATA *trade, int type)
{
   static char trades[MSL];
   char newbuf[30];
   
   if (type == 1)
   {
      sprintf(trades, "%s Offered ", kingdom_table[trade->offering_kingdom]->name);
      if (trade->offering_res_iron > 0)
      {
         sprintf(newbuf, "Iron: %d  ", trade->offering_res_iron);
         strcat(trades, newbuf);
      }
      if (trade->offering_res_gold > 0)
      {
         sprintf(newbuf, "Gold: %d  ", trade->offering_res_gold);
         strcat(trades, newbuf);
      }
      if (trade->offering_res_corn > 0)
      {
         sprintf(newbuf, "Corn: %d  ", trade->offering_res_corn);
         strcat(trades, newbuf);
      }
      if (trade->offering_res_grain > 0)
      {
         sprintf(newbuf, "Grain: %d  ", trade->offering_res_grain);
         strcat(trades, newbuf);
      }
      if (trade->offering_res_tree > 0)
      {
         sprintf(newbuf, "Lumber: %d  ", trade->offering_res_tree);   
         strcat(trades, newbuf);
      }
      if (trade->offering_res_stone > 0)
      {
         sprintf(newbuf, "Stone: %d  ", trade->offering_res_stone);
         strcat(trades, newbuf);
      }
      if (trade->offering_res_fish > 0)
      {
         sprintf(newbuf, "Fish: %d  ", trade->offering_res_fish);
         strcat(trades, newbuf);
      }
      if (trade->offering_gold > 0)
      {
         sprintf(newbuf, "Coins: %d  ", trade->offering_gold);
         strcat(trades, newbuf);
      }
   }
   if (type == 2)
   {   
      sprintf(newbuf, "\n\r%s Offered ", kingdom_table[trade->receiving_kingdom]->name);
      strcat(trades, newbuf);
         
      if (trade->receiving_res_iron > 0)
      {
         sprintf(newbuf, "Iron: %d  ", trade->receiving_res_iron);
         strcat(trades, newbuf);
      }
      if (trade->receiving_res_gold > 0)
      {
         sprintf(newbuf, "Gold: %d  ", trade->receiving_res_gold);
         strcat(trades, newbuf);
      }
      if (trade->receiving_res_corn > 0)
      {
         sprintf(newbuf, "Corn: %d  ", trade->receiving_res_corn);
         strcat(trades, newbuf);
      }
      if (trade->receiving_res_grain > 0)
      {
         sprintf(newbuf, "Grain: %d  ", trade->receiving_res_grain);
         strcat(trades, newbuf);
      }
      if (trade->receiving_res_tree > 0)
      {
         sprintf(newbuf, "Lumber: %d  ", trade->receiving_res_tree);   
         strcat(trades, newbuf);
      }
      if (trade->receiving_res_stone > 0)
      {
         sprintf(newbuf, "Stone: %d  ", trade->receiving_res_stone);
         strcat(trades, newbuf);
      }
      if (trade->receiving_res_fish > 0)
      {
         sprintf(newbuf, "Fish: %d  ", trade->receiving_res_fish);
         strcat(trades, newbuf);
      }
      if (trade->receiving_gold > 0)
      {
         sprintf(newbuf, "Coins: %d  ", trade->receiving_gold);
         strcat(trades, newbuf);
      }
   }
   return trades;
}

char has_trade_resources(int kingdom, TRADE_DATA *trade)
{
   TOWN_DATA *town;
   
   town = get_town(kingdom_table[kingdom]->dtown);
   
   if (!town)
      return ' ';
   
   if (trade->receiving_kingdom == kingdom)
   {
      if (town->corn < trade->receiving_res_corn)
         return ' ';
      if (town->grain < trade->receiving_res_grain)
         return ' ';
      if (town->stone < trade->receiving_res_stone)
         return ' ';
      if (town->fish < trade->receiving_res_fish)
         return ' ';
      if (town->gold < trade->receiving_res_gold)
         return ' ';
      if (town->lumber < trade->receiving_res_tree)
         return ' ';
      if (town->iron < trade->receiving_res_iron)
         return ' ';
      if (town->coins < trade->receiving_gold)
         return ' ';   
   }
   else
   {
      if (town->corn < trade->offering_res_corn)
         return ' ';
      if (town->grain < trade->offering_res_grain)
         return ' ';
      if (town->stone < trade->offering_res_stone)
         return ' ';
      if (town->fish < trade->offering_res_fish)
         return ' ';
      if (town->gold < trade->offering_res_gold)
         return ' ';
      if (town->lumber < trade->offering_res_tree)
         return ' ';
      if (town->iron < trade->offering_res_iron)
         return ' ';
      if (town->coins < trade->offering_gold)
         return ' ';   
   }
   return 'X';
}
//moved townvalues trading into this command now, used for both kingdom/town trades
//The two towns need to be connected now to trade, go road system!
void do_tradegoods(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char trade[MIL];
   char newbuf[200];
   TOWN_DATA *otown;
   CONQUER_DATA *conquer;
   TOWN_DATA *rtown;
   TRADE_DATA *trades;
   int ramount, oamount;
   int num = 1;

   if (IS_NPC(ch))
   {
      send_to_char("For PCs only.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minwithdraw ||
      ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend ||
      ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal || ch->pcdata->caste == caste_God)
   {
      if (ch->pcdata->town == NULL || str_cmp(ch->pcdata->town->mayor, ch->name))
      {
         send_to_char("Huh?\n\r", ch);
         return;
      }
   }


   if (argument[0] == '\0')
   {
      send_to_char("-----------------------Below are for trading between kingdoms-----------------------\n\r", ch);
      send_to_char("Syntax: tradegoods list.\n\r", ch);
      send_to_char("Syntax: tradegoods set <number> <item> <offer/want> <value>.\n\r", ch);
      send_to_char("Syntax: tradegoods start <kingdon name/number>.\n\r", ch);
      send_to_char("Syntax: tradegoods send <number>.\n\r", ch);
      send_to_char("Syntax: tradegoods decline <number>.\n\r", ch);
      send_to_char("Syntax: tradegoods accept <number>.\n\r", ch);
      send_to_char("-----------------------Below are for trading between towns--------------------------\n\r", ch);
      send_to_char("Syntax: tradegoods town take <name of town> <item> <number>.\n\r", ch);
      send_to_char("Syntax: tradegoods town give <name of town> <item> <number>.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (!str_cmp(arg1, "town"))
   {
      argument = one_argument(argument, arg3);
      argument = one_argument(argument, arg4);
      for (conquer = first_conquer; conquer; conquer = conquer->next)
      {
         if (conquer->town == ch->pcdata->town)
            break;
      }
      if (conquer)
      {
         send_to_char("You cannot give or take anything out of a town under siege.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "give"))
      {
         if ((rtown = get_town(arg3)) == NULL)
         {
            send_to_char("That is not an actual town.\n\r", ch);
            return;
         }
         if (rtown->kingdom != ch->pcdata->hometown)
         {
            send_to_char("That town does not belong to your kingdom.\n\r", ch);
            return;
         }
         if (rtown == ch->pcdata->town)
         {
            send_to_char("Why do you want to trade with your own town?\n\r", ch);
            return;
         }
         if ((islinked(ch, ch->pcdata->town->startx, ch->pcdata->town->starty, rtown->startx, rtown->starty, NULL, 2)) != 1)
         {
            send_to_char("That town is not connected to your town.\n\r", ch);
            return;
         }
         if (!str_cmp(arg4, "coins"))
         {
            if (atoi(argument) > ch->pcdata->town->coins || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + (atoi(argument)/100) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->coins -= atoi(argument);
            rtown->coins += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "lumber"))
         {
            if (atoi(argument) > ch->pcdata->town->lumber || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + atoi(argument) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->lumber -= atoi(argument);
            rtown->lumber += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "iron"))
         {
            if (atoi(argument) > ch->pcdata->town->iron || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + atoi(argument) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->iron -= atoi(argument);
            rtown->iron += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "gold"))
         {
            if (atoi(argument) > ch->pcdata->town->gold || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + atoi(argument) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->gold -= atoi(argument);
            rtown->gold += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "grain"))
         {
            if (atoi(argument) > ch->pcdata->town->grain || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + atoi(argument) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->grain -= atoi(argument);
            rtown->grain += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s gave %d units of grain from %s to %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "corn"))
         {
            if (atoi(argument) > ch->pcdata->town->corn || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + atoi(argument) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->corn -= atoi(argument);
            rtown->corn += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s gave %d units of corn from %s to %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "stone"))
         {
            if (atoi(argument) > ch->pcdata->town->stone || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + atoi(argument) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->stone -= atoi(argument);
            rtown->stone += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s gave %d units of stone from %s to %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "fish"))
         {
            if (atoi(argument) > ch->pcdata->town->fish || atoi(argument) < 1)
            {
               send_to_char("Either you do not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(rtown) + atoi(argument) > rtown->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            ch->pcdata->town->fish -= atoi(argument);
            rtown->fish += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s gave %d units of fish from %s to %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         return;
      }
      if (!str_cmp(arg2, "take"))
      {
         if (ch->pcdata->caste < caste_King
         && (!kingdom_table[ch->pcdata->hometown]->number1 || str_cmp(kingdom_table[ch->pcdata->hometown]->number1, ch->name))
         && (!kingdom_table[ch->pcdata->hometown]->number2 || str_cmp(kingdom_table[ch->pcdata->hometown]->number2, ch->name)))
         {
            send_to_char("Only the king and number1 and number2 can take resources from the kingdom stash.\n\r", ch);
            return;
         }
         if ((rtown = get_town(arg3)) == NULL)
         {
            send_to_char("That is not an actual town.\n\r", ch);
            return;
         }
         if (rtown->kingdom != ch->pcdata->hometown)
         {
            send_to_char("That town does not belong to your kingdom.\n\r", ch);
            return;
         }
         if (rtown == ch->pcdata->town)
         {
            send_to_char("Why do you want to trade with your own town?\n\r", ch);
            return;
         }
         if ((islinked(ch, ch->pcdata->town->startx, ch->pcdata->town->starty, rtown->startx, rtown->starty, NULL, 2)) != 1)
         {
            send_to_char("That town is not connected to your town.\n\r", ch);
            return;
         }
         for (conquer = first_conquer; conquer; conquer = conquer->next)
         {
            if (conquer->town == rtown)
               break;
         }
         if (conquer)
         {
            send_to_char("You cannot give or take anything out of a town under siege.\n\r", ch);
            return;
         }
         if (!str_cmp(arg4, "coins"))
         {
            if (atoi(argument) > rtown->coins || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + (atoi(argument)/100) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->coins  -= atoi(argument);
            ch->pcdata->town->coins += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of coins for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "lumber"))
         {
            if (atoi(argument) > rtown->lumber || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + atoi(argument) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->lumber -= atoi(argument);
            ch->pcdata->town->lumber += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of lumber for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "iron"))
         {
            if (atoi(argument) > rtown->iron || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + atoi(argument) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->iron -= atoi(argument);
            ch->pcdata->town->iron += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of iron for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "gold"))
         {
            if (atoi(argument) > rtown->gold || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + atoi(argument) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->gold -= atoi(argument);
            ch->pcdata->town->gold += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of gold for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "grain"))
         {
            if (atoi(argument) > rtown->grain || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + atoi(argument) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->grain -= atoi(argument);
            ch->pcdata->town->grain += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of grain for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "corn"))
         {
            if (atoi(argument) > rtown->corn || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + atoi(argument) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->corn -= atoi(argument);
            ch->pcdata->town->corn += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of corn for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "stone"))
         {
            if (atoi(argument) > rtown->stone || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + atoi(argument) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->stone -= atoi(argument);
            ch->pcdata->town->stone += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of stone for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         if (!str_cmp(arg4, "fish"))
         {
            if (atoi(argument) > rtown->fish || atoi(argument) < 1)
            {
               send_to_char("Either that town does not have enough, or the value is less than 1.\n\r", ch);
               return;
            }
            if (get_current_hold(ch->pcdata->town) + atoi(argument) > ch->pcdata->town->hold)
            {
               send_to_char("The destination town does not have enough hold to store that much.\n\r", ch);
               return;
            }
            rtown->fish -= atoi(argument);
            ch->pcdata->town->fish += atoi(argument);
            write_kingdom_file(ch->pcdata->hometown);
            sprintf(trade, "***TOWN TRADE*** %s took %d units of fish for %s from %s", ch->name, atoi(argument), ch->pcdata->town->name, rtown->name); 
            write_kingdom_logfile(ch->pcdata->hometown, trade, KLOG_TRADEGOODS); 
            send_to_char("Done.\n\r", ch);
         }
         return;
      }
      do_tradegoods(ch, "");
      return;
   }
   
   if (!str_cmp(arg1, "decline"))
   {
      for (trades = first_trade; trades; trades = trades->next)
      {
         if (trades->offering_kingdom == ch->pcdata->hometown || trades->receiving_kingdom == ch->pcdata->hometown)
         {
            if (num == atoi(arg2))
               break;
            num++;
         }
      }
      if (!trades)
      {
         send_to_char("That number does not exist, type tradegoods list to get a valid number.\n\r", ch);
         return;
      }  
      send_to_char("The trade has been cancelled, it has been logged in each kingdom's log.\n\r", ch);
      sprintf(trade, "***TRADE CANCELLED*** %s-%s by %s", kingdom_table[trades->offering_kingdom]->name, 
              kingdom_table[trades->receiving_kingdom]->name, PERS_KINGDOM(ch, trades->offering_kingdom));
      write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
      sprintf(trade, "***TRADE CANCELLED*** %s-%s by %s", kingdom_table[trades->offering_kingdom]->name, 
              kingdom_table[trades->receiving_kingdom]->name, PERS_KINGDOM(ch, trades->receiving_kingdom));
      write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      
      sprintf(trade, "***TRADE CANCELLED*** %s", get_resources_traded(trades, 1));
      write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
      write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      
      sprintf(trade, "***TRADE CANCELLED*** %s", get_resources_traded(trades, 2));
      write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
      write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      UNLINK(trades, first_trade, last_trade, next, prev);
      DISPOSE(trades); 
      save_trade_file();
      return;
   }   
   if (!str_cmp(arg1, "accept"))
   {
      int oking, rking;
      for (trades = first_trade; trades; trades = trades->next)
      {
         if (trades->offering_kingdom == ch->pcdata->hometown || trades->receiving_kingdom == ch->pcdata->hometown)
         {
            if (num == atoi(arg2))
               break;
            num++;
         }
      }
      if (!trades)
      {
         send_to_char("That number does not exist, type tradegoods list to get a valid number.\n\r", ch);
         return;
      }
      if (trades->offering_kingdom == ch->pcdata->hometown)
      {
         if (trades->posted == FALSE || trades->offering_read == FALSE)
         {
            send_to_char("You cannot accept this trade.  Try sending it if you haven't yet or wait for a counter-offer.\n\r", ch);
            return;
         }
      }
      else
      {
         if (trades->posted == FALSE || trades->receiving_read == FALSE)
         {
            send_to_char("You cannot accept this trade.  Try sending it if you haven't yet or wait for a counter-offer.\n\r", ch);
            return;
         }
      }
      oking = trades->offering_kingdom;
      rking = trades->receiving_kingdom;
      
      otown = get_town(kingdom_table[oking]->dtown);
      rtown = get_town(kingdom_table[rking]->dtown); 
      
      if ((islinked(ch, otown->startx, otown->starty, rtown->startx, rtown->starty, NULL, 2)) != 1)
      {
         send_to_char("The default towns of the two kingdoms are not linked together.\n\r", ch);
         return;
      }
      if (!otown)
      {
         send_to_char("Cannot find the default town for the Offering Kingdom!\n\r", ch);
         return;
      }
      if (!rtown)
      {
         send_to_char("Cannot find the default town for the Receiving Kingdom!\n\r", ch);
         return;
      }
      if (otown->corn < trades->offering_res_corn)
      {
         send_to_char("The Offering kingdom does not have the specified amount of corn!\n\r", ch);
         return;
      }
      if (otown->grain < trades->offering_res_grain)
      {
         send_to_char("The Offering kingdom does not have the specified amount of grain!\n\r", ch);
         return;
      }
      if (otown->iron < trades->offering_res_iron)
      {
         send_to_char("The Offering kingdom does not have the specified amount of iron!\n\r", ch);
         return;
      }
      if (otown->gold < trades->offering_res_gold)
      {
         send_to_char("The Offering kingdom does not have the specified amount of gold!\n\r", ch);
         return;
      }
      if (otown->stone < trades->offering_res_stone)
      {
         send_to_char("The Offering kingdom does not have the specified amount of stone!\n\r", ch);
         return;
      }
      if (otown->fish < trades->offering_res_fish)
      {
         send_to_char("The Offering kingdom does not have the specified amount of fish!\n\r", ch);
         return;
      }
      if (otown->lumber < trades->offering_res_tree)
      {
         send_to_char("The Offering kingdom does not have the specified amount of lumber!\n\r", ch);
         return;
      }
      if (otown->coins < trades->offering_gold)
      {
         send_to_char("The Offering kingdom does not have the specified amount of coins!\n\r", ch);
         return;
      }
      
      if (rtown->corn < trades->receiving_res_corn)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of corn!\n\r", ch);
         return;
      }
      if (rtown->grain < trades->receiving_res_grain)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of grain!\n\r", ch);
         return;
      }
      if (rtown->iron < trades->receiving_res_iron)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of iron!\n\r", ch);
         return;
      }
      if (rtown->gold < trades->receiving_res_gold)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of gold!\n\r", ch);
         return;
      }
      if (rtown->fish < trades->receiving_res_fish)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of fish!\n\r", ch);
         return;
      }
      if (rtown->stone < trades->receiving_res_stone)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of stone!\n\r", ch);
         return;
      }
      if (rtown->lumber < trades->receiving_res_tree)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of lumber!\n\r", ch);
         return;
      }
      if (rtown->coins < trades->receiving_gold)
      {
         send_to_char("The Receiving kingdom does not have the specified amount of coins!\n\r", ch);
         return;
      }
      ramount = (trades->receiving_gold/100) + trades->receiving_res_tree + trades->receiving_res_stone + trades->receiving_res_gold;
      ramount += trades->receiving_res_iron + trades->receiving_res_grain + trades->receiving_res_corn + trades->receiving_res_fish;
      oamount = (trades->offering_gold/100) + trades->offering_res_tree + trades->offering_res_stone + trades->offering_res_gold;
      oamount += trades->offering_res_iron + trades->offering_res_grain + trades->offering_res_corn + trades->offering_res_fish;
      
      if ((get_current_hold(otown) + ramount) > otown->hold)
      {
         send_to_char("The Receiving kingdom does not have enough silo space to store this trade.\n\r", ch);
         return;
      }
      if ((get_current_hold(rtown) + oamount) > rtown->hold)
      {
         send_to_char("The Offering kingdom does not have enough silo space to store this trade.\n\r", ch);
         return;
      }    
      otown->lumber += trades->receiving_res_tree;
      otown->corn += trades->receiving_res_corn;
      otown->grain += trades->receiving_res_grain;
      otown->iron += trades->receiving_res_iron;
      otown->gold += trades->receiving_res_gold;
      otown->stone += trades->receiving_res_stone;
      otown->fish += trades->receiving_res_fish;
      otown->coins += trades->receiving_gold;
      rtown->lumber += trades->offering_res_tree;
      rtown->corn += trades->offering_res_corn;
      rtown->grain += trades->offering_res_grain;
      rtown->iron += trades->offering_res_iron;
      rtown->gold += trades->offering_res_gold;
      rtown->stone += trades->offering_res_stone;
      rtown->fish += trades->offering_res_fish;
      rtown->coins += trades->offering_gold;
      otown->lumber -= trades->offering_res_tree;
      otown->corn -= trades->offering_res_corn;
      otown->grain -= trades->offering_res_grain;
      otown->iron -= trades->offering_res_iron;
      otown->gold -= trades->offering_res_gold;
      otown->stone -= trades->offering_res_stone;
      otown->fish -= trades->offering_res_fish;
      otown->coins -= trades->offering_gold;
      rtown->lumber -= trades->receiving_res_tree;
      rtown->corn -= trades->receiving_res_corn;
      rtown->grain -= trades->receiving_res_grain;
      rtown->iron -= trades->receiving_res_iron;
      rtown->gold -= trades->receiving_res_gold;
      rtown->stone -= trades->receiving_res_stone;
      rtown->fish -= trades->receiving_res_fish;
      rtown->coins -= trades->receiving_gold;
      
      send_to_char("The trade has been completed, it has been logged in each kingdom's log.\n\r", ch);
      sprintf(trade, "***TRADE*** %s-%s by %s", kingdom_table[trades->offering_kingdom]->name, 
              kingdom_table[trades->receiving_kingdom]->name, PERS_KINGDOM(ch, trades->offering_kingdom));
      write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
      sprintf(trade, "***TRADE*** %s-%s by %s", kingdom_table[trades->offering_kingdom]->name, 
              kingdom_table[trades->receiving_kingdom]->name, PERS_KINGDOM(ch, trades->receiving_kingdom));
      write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      
      sprintf(trade, "***TRADE*** %s", get_resources_traded(trades, 1));
      write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
      write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      
      sprintf(trade, "***TRADE*** %s", get_resources_traded(trades, 2));
      write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
      write_kingdom_logfile(trades->receiving_kingdom, trade, KLOG_TRADEGOODS); 
      UNLINK(trades, first_trade, last_trade, next, prev);
      DISPOSE(trades);      
      save_trade_file();
      return;     
   }
   if (!str_cmp(arg1, "send"))
   {
      for (trades = first_trade; trades; trades = trades->next)
      {
         if (trades->offering_kingdom == ch->pcdata->hometown || trades->receiving_kingdom == ch->pcdata->hometown)
         {
            if (num == atoi(arg2))
               break;
            num++;
         }
      }
      if (!trades)
      {
         send_to_char("That number does not exist, type tradegoods list to get a valid number.\n\r", ch);
         return;
      }
      if (trades->posted == TRUE)
      {
         send_to_char("There are no changes to send to this.\n\r", ch);
         return;
      }
      if (trades->offering_kingdom == ch->pcdata->hometown)
      {
         if (trades->offering_read == TRUE && trades->posted == FALSE)
         {
            trades->posted = TRUE;
            trades->offering_read = FALSE;
            trades->receiving_read = TRUE;
            save_trade_file();
            send_to_char("It has been set, I hope that is what you wanted.  If not decline it real quick.\n\r", ch);
            return;
         }
         
      }
      else
      {
         if (trades->receiving_read == TRUE && trades->posted == FALSE)
         {
            trades->posted = TRUE;
            trades->receiving_read = FALSE;
            trades->offering_read = TRUE;
            save_trade_file();
            send_to_char("It has been set, I hope that is what you wanted.  If not decline it real quick.\n\r", ch);
            return;
         }
      }
   }
   if (!str_cmp(arg1, "set"))
   {
      if (arg2[0] == '\0')
      {
         send_to_char("tradegoods set <number> <item> <offer/want> <value>\n\r", ch);
         send_to_char("item - lumber, corn, grain, stone, gold, iron, coins, fish.\n\r", ch);
         return;
      }
      for (trades = first_trade; trades; trades = trades->next)
      {
         if (trades->offering_kingdom == ch->pcdata->hometown || trades->receiving_kingdom == ch->pcdata->hometown)
         {
            if (num == atoi(arg2))
               break;
            num++;
         }
      }
      if (!trades)
      {
         send_to_char("No such number, please type tradegoods list.\n\r", ch);
         return;
      }   
      argument = one_argument(argument, arg3);
      argument = one_argument(argument, arg4);
      if (str_cmp(arg3, "lumber") && str_cmp(arg3, "corn") && str_cmp(arg3, "grain") && 
          str_cmp(arg3, "stone") && str_cmp(arg3, "gold") && str_cmp(arg3, "iron") && 
          str_cmp(arg3, "coins") && str_cmp(arg3, "fish"))
      {
         send_to_char("tradegoods set <number> <item> <offer/want> <value>\n\r", ch);
         send_to_char("item - lumber, corn, grain, stone, gold, iron, coins, fish.\n\r", ch);
         return;
      }
      if (str_cmp(arg4, "want") & str_cmp(arg4, "offer"))
      {
         send_to_char("You need to type either want or offer.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 99999999)
      {
         send_to_char("The range for the value is 0 to 99,999,999.\n\r", ch);
         return;
      }
      if (trades->offering_kingdom == ch->pcdata->hometown)
      {
         if (trades->offering_read == FALSE)
         {
            send_to_char("You cannot alter it, it is in the process of being alter by the other kingdom.\n\r", ch);
            return;
         }
      }
      else
      {
         if (trades->receiving_read == FALSE)
         {
            send_to_char("You cannot alter it, it is in the process of being alter by the other kingdom.\n\r", ch);
            return;
         }
      }
      if (!str_cmp(arg3, "lumber"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_res_tree = atoi(argument);
            else
               trades->receiving_res_tree = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_res_tree = atoi(argument);
            else
               trades->offering_res_tree = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      if (!str_cmp(arg3, "corn"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_res_corn = atoi(argument);
            else
               trades->receiving_res_corn = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_res_corn = atoi(argument);
            else
               trades->offering_res_corn = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      if (!str_cmp(arg3, "grain"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_res_grain = atoi(argument);
            else
               trades->receiving_res_grain = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_res_grain = atoi(argument);
            else
               trades->offering_res_grain = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      if (!str_cmp(arg3, "stone"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_res_stone = atoi(argument);
            else
               trades->receiving_res_stone = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_res_stone = atoi(argument);
            else
               trades->offering_res_stone = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      if (!str_cmp(arg3, "fish"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_res_fish = atoi(argument);
            else
               trades->receiving_res_fish = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_res_fish = atoi(argument);
            else
               trades->offering_res_fish = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      if (!str_cmp(arg3, "gold"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_res_gold = atoi(argument);
            else
               trades->receiving_res_gold = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_res_gold = atoi(argument);
            else
               trades->offering_res_gold = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      if (!str_cmp(arg3, "iron"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_res_iron = atoi(argument);
            else
               trades->receiving_res_iron = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_res_iron = atoi(argument);
            else
               trades->offering_res_iron = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      if (!str_cmp(arg3, "coins"))
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
         {
            if (!str_cmp(arg4, "offer"))
               trades->offering_gold = atoi(argument);
            else
               trades->receiving_gold = atoi(argument);
            trades->receiving_read = FALSE;
            trades->posted = FALSE;
         }
         else
         {
            if (!str_cmp(arg4, "offer"))
               trades->receiving_gold = atoi(argument);
            else
               trades->offering_gold = atoi(argument);
            trades->offering_read = FALSE;
            trades->posted = FALSE;
         }
      }
      send_to_char("It is set.  Remember you must send it again for them to accept it or counter-offer.\n\r", ch);
      save_trade_file();
      return;
   }
   if (!str_cmp(arg1, "start"))
   {
      if (atoi(arg2) < 2 || atoi(arg2) >= sysdata.max_kingdom)
      {
         send_to_char("That is not a valid kingdom.\n\r", ch);
         return;
      }
      if (atoi(arg2) == ch->pcdata->hometown)
      {
         send_to_char("You cannot start a trade with your own kingdom.\n\r", ch);
         return;
      }
      num = 0;
      for (trades = first_trade; trades; trades = trades->next)
      {
         if (trades->offering_kingdom == ch->pcdata->hometown)
            num++;
      }
      if (num > 20)
      {
         send_to_char("You can only start as many as 20 trades.\n\r", ch);
         return;
      }
      CREATE(trades, TRADE_DATA, 1);
      trades->offering_kingdom = ch->pcdata->hometown;
      trades->receiving_kingdom = atoi(arg2);
      trades->time = time(0);
      trades->posted = FALSE;
      trades->offering_read = TRUE;
      LINK(trades, first_trade, last_trade, next, prev);
      save_trade_file();
      ch_printf(ch, "A trade session has been started, it will be sent to %s when finished.\n\r", kingdom_table[atoi(arg2)]->name);
      sprintf(trade, "***TRADE STARTED*** %s-%s by %s", kingdom_table[trades->offering_kingdom]->name, 
              kingdom_table[trades->receiving_kingdom]->name, PERS_KINGDOM(ch, trades->offering_kingdom));
      write_kingdom_logfile(trades->offering_kingdom, trade, KLOG_TRADEGOODS); 
      return;
   }
      
   if (!str_cmp(arg1, "list"))
   {      
      for (trades = first_trade; trades; trades = trades->next)
      {
         if (trades->offering_kingdom == ch->pcdata->hometown || trades->receiving_kingdom == ch->pcdata->hometown)
         {
            sprintf(trade, "[%-3d] ", num);

            if (trades->offering_kingdom == ch->pcdata->hometown)
               sprintf(newbuf, "You are giving to %-10s:     ", kingdom_table[trades->receiving_kingdom]->name);
            else
               sprintf(newbuf, "You are getting from %-10s:  ", kingdom_table[trades->offering_kingdom]->name);
            
            strcat(trade, newbuf);
               
            if (trades->offering_res_iron > 0)
            {
               sprintf(newbuf, "Iron: %d  ", trades->offering_res_iron);
               strcat(trade, newbuf);
            }
            if (trades->offering_res_gold > 0)
            {
               sprintf(newbuf, "Gold: %d  ", trades->offering_res_gold);
               strcat(trade, newbuf);
            }
            if (trades->offering_res_corn > 0)
            {
               sprintf(newbuf, "Corn: %d  ", trades->offering_res_corn);
               strcat(trade, newbuf);
            }
            if (trades->offering_res_grain > 0)
            {
               sprintf(newbuf, "Grain: %d  ", trades->offering_res_grain);
               strcat(trade, newbuf);
            }
            if (trades->offering_res_tree > 0)
            {
               sprintf(newbuf, "Lumber: %d  ", trades->offering_res_tree);   
               strcat(trade, newbuf);
            }
            if (trades->offering_res_stone > 0)
            {
               sprintf(newbuf, "Stone: %d  ", trades->offering_res_stone);
               strcat(trade, newbuf);
            }
            if (trades->offering_res_fish > 0)
            {
               sprintf(newbuf, "Fish: %d  ", trades->offering_res_fish);
               strcat(trade, newbuf);
            }
            if (trades->offering_gold > 0)
            {
               sprintf(newbuf, "Coins: %d  ", trades->offering_gold);
               strcat(trade, newbuf);
            }
               
            sprintf(newbuf, "\n\r[%-3d] ", num);
            strcat(trade, newbuf);
            if (trades->receiving_kingdom == ch->pcdata->hometown)
               sprintf(newbuf, "You are giving to %-10s:     ", kingdom_table[trades->offering_kingdom]->name);
            else
               sprintf(newbuf, "You are getting from %-10s:  ", kingdom_table[trades->receiving_kingdom]->name);
            
            strcat(trade, newbuf);
               
            if (trades->receiving_res_iron > 0)
            {
               sprintf(newbuf, "Iron: %d  ", trades->receiving_res_iron);
               strcat(trade, newbuf);
            }
            if (trades->receiving_res_gold > 0)
            {
               sprintf(newbuf, "Gold: %d  ", trades->receiving_res_gold);
               strcat(trade, newbuf);
            }
            if (trades->receiving_res_corn > 0)
            {
               sprintf(newbuf, "Corn: %d  ", trades->receiving_res_corn);
               strcat(trade, newbuf);
            }
            if (trades->receiving_res_grain > 0)
            {
               sprintf(newbuf, "Grain: %d  ", trades->receiving_res_grain);
               strcat(trade, newbuf);
            }
            if (trades->receiving_res_tree > 0)
            {
               sprintf(newbuf, "Lumber: %d  ", trades->receiving_res_tree);   
               strcat(trade, newbuf);
            }
            if (trades->receiving_res_stone > 0)
            {
               sprintf(newbuf, "Stone: %d  ", trades->receiving_res_stone);
               strcat(trade, newbuf);
            }
            if (trades->receiving_res_fish > 0)
            {
               sprintf(newbuf, "Fish: %d  ", trades->receiving_res_fish);
               strcat(trade, newbuf);
            }
            if (trades->receiving_gold > 0)
            {
               sprintf(newbuf, "Coins: %d  ", trades->receiving_gold);
               strcat(trade, newbuf);
            }
            sprintf(newbuf, "\n\r[%-3d] Offering - [%c]%-10s   Receiving - [%c]%-10s    Sent[%s]   Side[%s]\n\r\n\r", num, has_trade_resources(trades->offering_kingdom, trades),
                    kingdom_table[trades->offering_kingdom]->name, has_trade_resources(trades->receiving_kingdom, trades),
                    kingdom_table[trades->receiving_kingdom]->name, trades->posted ? "X" : " ", 
                    trades->receiving_read ? "Receiving" : "Offering" );
            strcat(trade, newbuf);
            send_to_char(trade, ch);
            num++;
         }
      }
      if (num == 1)
         send_to_char("You have no trades in process.\n\r", ch);
      return;
   }
}

//Connection Code by Rameti, this level of stuff was way beyond me, ha ha

/*
	This is a basic unoptimized A* method

*/

struct asnode
{
	struct asnode *next;
	struct asnode *parent;
	int x;
	int y;
	int cost;
	int heuristic;
	int f;
};





int getnodeheuristic(int sx, int sy, int ex, int ey)
{
	/* Use the Manhatten distance */
	int dx;
	int dy;

	dx = ex - sx;
	dy = ey - sy;
	if(dx < 0) dx = -dx;
	if(dy < 0) dy = -dy;

	return dx+dy;
}

struct asnode *getbestnode(struct asnode **parent)
{
	struct asnode **bparent;
	struct asnode *best;
	struct asnode *node;
	
	if(*parent == NULL)
		return NULL;
	best = node = *parent;
	bparent = parent;
	parent = &node->next;	
	node = node->next;	
	while(node)
	{
		/* we want the path with the smallest total estimated cost */
		if(node->f < best->f)
		{
			best = node;
			bparent = parent;
		}
		parent = &node->next;
		node = node->next;
	}
	*bparent = best->next;
	best->next = NULL;
	return best;
}

int isinaslist(struct asnode *al, struct asnode *node)
{
	for(;al; al=al->next)
	{
		if(node->x == al->x && node->y == al->y)
			return 1;
	}
	return 0;
}


struct asnode *findinlist(struct asnode *al, struct asnode *node)
{
	for(;al; al=al->next)
	{
		if(node->x == al->x && node->y == al->y)
			return al;
	}
	return NULL;	
}

void pushsur(struct asnode **ao_, struct asnode **ac_, struct asnode *node, int ex, int ey, int onroad, int sx, int sy, TOWN_DATA *stown, TOWN_DATA *etown)
{
	struct xymatrix
	{
		int x;
		int y;
	};
	struct xymatrix modmat[] =
	{
		{-1, -1},
		{ 0, -1},
		{ 1, -1},
		{-1,  0},
		{ 1,  0},
		{-1,  1},
		{ 0,  1},
		{ 1,  1},
		{ 0,  0} /* terminator */
	};
	struct xymatrix *rule;
	struct asnode tmp;
	struct asnode *newnode;
	struct asnode **parent;
	

	for(rule = modmat; rule->x || rule->y; rule++)
	{
		/* 
			Apply the matrix to the parent to test each of its surrounding
			nodes.
		*/
		tmp.x = node->x + rule->x;
		tmp.y = node->y + rule->y;
		
		/* skip testing any node that falls off the edges of the map */
		/* if you support wrappable maps, recalc these values instead */
		if(tmp.x < (ex - 350) || tmp.x == 0)
			continue;
		if(tmp.y < (ey - 350) || tmp.y == 0)
			continue;
		if(tmp.x > (ex + 350) || tmp.x == MAX_X)
			continue;
		if(tmp.y > (ey + 350) || tmp.y == MAX_Y)
			continue;

		/* If onroad is TRUE, need to be a road, if false, has to be passable */
		if(tmp.x == ex && tmp.y == ey)
		{
		   ;
		}
		else
		{
		   if (onroad >= 1)
		   {
		      if (onroad == 2 && stown && etown && map_sector[0][tmp.x][tmp.y] != SECT_ROAD)
		      {
		         int inrange = 0;
		         if (kingdom_sector[0][tmp.x][tmp.y] == kingdom_sector[0][sx][sy]
		         ||  kingdom_sector[0][tmp.x][tmp.y] == kingdom_sector[0][ex][ey])
		         {
		            if (in_town_range(stown, tmp.x, tmp.y, 0) || in_town_range(etown, tmp.x, tmp.y, 0))
		               inrange = 1;
		         }
		         if(map_sector[0][tmp.x][tmp.y] != SECT_ROAD && !inrange)
		            continue;
		      }
		      else
		      {
		         if(map_sector[0][tmp.x][tmp.y] != SECT_ROAD)
		            continue;
		      }
		   }
		   else
		   {
		      if (sect_show[map_sector[0][tmp.x][tmp.y]].canpass == 0)
		         continue;
		   }
	        }
			
		/* calculate the cost of this path */	
		tmp.parent = node;
		tmp.cost = 1 + node->cost;
		tmp.heuristic = getnodeheuristic(tmp.x,tmp.y,ex,ey);
		tmp.f = tmp.cost + tmp.heuristic;

		/*
			If this x,y is in the open list and the new cost
			is higher then the old cost, skip adding it
		*/
		newnode = findinlist(*ac_, &tmp);
		if(newnode && newnode->f < tmp.f)
			continue;
			
		/*
			If this x,y is in the closed list and the new cost
			is higher then the old cost, skip adding it
		*/
		newnode = findinlist(*ao_, &tmp);
		if(newnode && newnode->f < tmp.f)
			continue;

		/* remove this x,y from the closed list */
		parent = ac_;
		newnode = *parent;
		while(newnode)
		{
			if(newnode->x == tmp.x && newnode->y == tmp.y)
			{
				*parent = newnode->next;
				free(newnode);
				newnode = *parent;
				continue;
			}
			parent = &newnode->next;
			newnode = newnode->next;
		} 

		/* remove this x,y from the open list */
		parent = ao_;
		newnode = *parent;
		while(newnode)
		{
			if(newnode->x == tmp.x && newnode->y == tmp.y)
			{
				*parent = newnode->next;
				free(newnode);
				newnode = *parent;
				continue;
			}
			parent = &newnode->next;
			newnode = newnode->next;
		}
		
		/* now we add this node to the open list */
		newnode = malloc(sizeof(tmp));
		if(newnode == NULL)
			abort();
		memcpy(newnode, &tmp, sizeof(tmp));
		newnode->next = *ao_;
		*ao_ = newnode;
	}
}



int islinked(CHAR_DATA *ch, int sx, int sy, int ex, int ey, char *directions, int onroad)
{
	struct asnode *as_open = NULL;
	struct asnode *as_closed =NULL;
	struct asnode *node;
	TOWN_DATA *stown = NULL;
	TOWN_DATA *etown = NULL;
	int hx=0;
	int hy=0;
	int numcall = 0;
	int dir = 0;
	
	if (directions != NULL)
 	{
       if (!str_cmp(directions, "move"))
       {
          dir = 1;
	      strcpy(directions, "");
	      hx = 0;
	      hy = 0;
	   }
 	}
	
	//Too much space
	if (abs(sx-ex) > 350 || abs(sy-ey) > 350)
	{
	   bug("islink: Too much space");
	   return 2;
	}
	
	if (onroad == 2)
    {
       int kd;
       TOWN_DATA *town;
	   for (kd = 2; kd < sysdata.max_kingdom; kd++)
	   {
	      if (stown && etown)
	         break;
	      for (town = kingdom_table[kd]->first_town; town; town = town->next)
	      {
	         if (town->startx == sx && town->starty == sy)
	         {
	            stown = town;
	         }
	         if (town->startx == ex && town->starty == ey)
	         {
	            etown = town;
	         }
	      }
       }
    }
    if (directions && stown && !str_cmp(directions, "newtown"))
       etown = stown;

	node = malloc(sizeof(*node));
	if(node == NULL)
		abort();
	
	node->next = NULL;	
	node->x = sx;
	node->y = sy;
	node->cost = 0;
	node->heuristic = getnodeheuristic(sx,sy,ex,ey);
	node->f = node->cost + node->heuristic;
	node->parent = NULL;
	as_open = node;
	
	while(as_open)
	{
		node = getbestnode(&as_open);
		numcall++;
		if(node->x == ex && node->y == ey)
		{
		    if (dir == 1)
		    {
 			   while(node)
		 	   {
		 	      if (hx == 0 && hy == 0)
		 	      {
		 	         hx = node->x;
		 	         hy = node->y;
		 	      }
		 	      else
		 	      {
		 	         if (hy - node->y == -1) //south
		 	         {
		 	            if (hx - node->x == -1) //south east
		 	               strcat(directions, "s+e");
		 	            else if (hx - node->x == 1) //south west
		 	               strcat(directions, "s+w");
		 	            else
		 	               strcat(directions, "s");		 	            
		 	         }
		 	         else if (hy - node->y == 1) //north
		 	         {
		 	            if (hx - node->x == -1) //north east
		 	               strcat(directions, "n+e");
		 	            else if (hx - node->x == 1) //north west
		 	               strcat(directions, "n+w");
		 	            else
		 	               strcat(directions, "n");	
		 	         }
		 	         else if (hx - node->x == -1) //east
		 	            strcat(directions, "e");
		 	         else if (hx - node->x == 1) //west
		 	            strcat(directions, "w");
		 	            
		 	         hx = node->x;
		 	         hy = node->y;
		 	      }
		 	    /*  if (numcall < 100)
		 	         ch_printf(ch, "back %i,%i\n", node->x, node->y); */

			      node = node->parent;
			   }
			}
			/*else
			{
		           while(node)
		 	   {
		 	      if (numcall < 100)
		 	         ch_printf(ch, "back %i,%i\n", node->x, node->y);

			      node = node->parent;
			   }
			} */ 
			while(as_open)
			{
				node = as_open;
				as_open = as_open->next;
				free(node);	
			}
			while(as_closed)
			{
				node = as_closed;
				as_closed = as_closed->next;
				free(node);
			}
			return 1;
		}
		if ((onroad != 2 && numcall == 3500) || (onroad == 2 && numcall == 15000))
		{
		   while(as_open)
	           {
		      node = as_open;
	              as_open = as_open->next;
		      free(node);	
		   }
		   while(as_closed)
		   {
		      node = as_closed;
		      as_closed = as_closed->next;
		      free(node);
		   }
		   bug("numcall reached");
		   return 2;  
		}
		/* push successors */
		pushsur(&as_open, &as_closed, node, ex, ey, onroad, sx, sy, stown, etown);
		/* push this node onto closed list */
		node->next = as_closed;
		as_closed = node;
	}	
    bug("outside");
	return 0;	
}

//Shows what towns you are connected to, I hope..., laugh
void do_traderoutes(CHAR_DATA *ch, char *argument)
{
   TOWN_DATA *town;
   TOWN_DATA *ytown;
   int kingdom;
   
   if (check_npc(ch))
         return;
   if (!ch->pcdata->town)
   {
      send_to_char("You need to belong to a town to use this command.\n\r", ch);
      return;
   }
   if (!IN_PLAYER_KINGDOM(ch->pcdata->hometown))
   {
      send_to_char("You need to belong to an actual kingdom to use this command.\n\r", ch);
      return;
   }
   send_to_char("Your towns\n\r-------------------------------------------------\n\r", ch);
   for (town = kingdom_table[ch->pcdata->hometown]->first_town; town; town = town->next)
   {
      if (town == ch->pcdata->town)
         continue;
      else
      {
         if ((islinked(ch, ch->pcdata->town->startx, ch->pcdata->town->starty, town->startx, town->starty, NULL, 2)) != 1)
         {
            ch_printf(ch, "Not Connected to %s\n\r", town->name);
         }
         else
         {
            ch_printf(ch, "Connected to %s\n\r", town->name);
         }
      }
   }
   ytown = get_town(kingdom_table[ch->pcdata->hometown]->dtown);
   if (!ytown)
   {
      send_to_char("Your default town is not set correctly, tell your king.\n\r", ch);
      return;
   }
   send_to_char("\n\rOther Kingdoms\n\r-------------------------------------------------\n\r", ch);
   for (kingdom = 2; kingdom < sysdata.max_kingdom; kingdom++)
   {
      if (kingdom == ch->pcdata->hometown)
         continue;
      if (kingdom_table[kingdom]->dtown)
      {
         if ((town = get_town(kingdom_table[kingdom]->dtown)) != NULL)
         {
            if ((islinked(ch, ytown->startx, ytown->starty, town->startx, town->starty, NULL, 2)) != 1)
            {
               ch_printf(ch, "Not Connected to %s\n\r", kingdom_table[kingdom]->name);
            }
            else
            {
               ch_printf(ch, "Connected to %s\n\r", kingdom_table[kingdom]->name);
            }   
         }
      }
   }
   return;
}

int ruling_caste(CHAR_DATA *ch, int kingdom)
{
   if (!str_cmp(kingdom_table[kingdom]->ruler, ch->name) || !str_cmp(kingdom_table[kingdom]->number1, ch->name)
   ||  !str_cmp(kingdom_table[kingdom]->number2, ch->name) || ch->pcdata->caste >= caste_Staff)
   {
      return TRUE;
   }
   else
      return FALSE;
}

char *parse_resource_value(int resource)
{
   if (resource == KRES_GOLD)
      return "Gold";
   if (resource == KRES_IRON)
      return "Iron";
   if (resource == KRES_CORN)
      return "Corn";
   if (resource == KRES_GRAIN)
      return "Grain";
   if (resource == KRES_LUMBER)
      return "Lumber";
   if (resource == KRES_STONE)
      return "Stone";
   if (resource == KRES_FISH)
      return "Fish";
   
   return "Unknown";
}

int parse_resource_arg(char *argument)
{
   if (atoi(argument) >= KRES_GOLD && atoi(argument) <= KRES_FISH)
      return atoi(argument);
   if (!str_cmp(argument, "Gold"))
      return KRES_GOLD;
   if (!str_cmp(argument, "Iron"))
      return KRES_IRON;
   if (!str_cmp(argument, "Corn"))
      return KRES_CORN;
   if (!str_cmp(argument, "Grain"))
      return KRES_GRAIN;
   if (!str_cmp(argument, "Lumber"))
      return KRES_LUMBER;
   if (!str_cmp(argument, "Stone"))
      return KRES_STONE;
   if (!str_cmp(argument, "Fish"))
      return KRES_FISH;
   
   return KRES_UNKNOWN;
}
            
// To schedule workers just in case you leave for a bit...
void do_schedule(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   SCHEDULE_DATA *schedule;
   SCHEDULE_DATA *nschedule;
   int x = 1;
   TOWN_DATA *town;
   
   if (check_npc(ch))
      return;
   if (!ch->pcdata->town)
   {
      send_to_char("You need to belong to a town to use this.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: schedule view <town>\n\r", ch);
      send_to_char("Syntax: schedule add <town> <start period> <end period> <resource> [reoccur]\n\r", ch);
      send_to_char("Syntax: schedule remove <town> <number>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   argument = one_argument(argument, arg4);
   argument = one_argument(argument, arg5);
   if (!str_cmp(arg1, "remove"))
   {
      town = get_town(arg2);
      if (!town)
      {
         send_to_char("That is not a valid town.\n\r", ch);
         return;
      }
      if (town->kingdom != ch->pcdata->hometown)
      {
         send_to_char("That town does not belong to your kingdom.\n\r", ch);
         return;
      }
      if (ch->pcdata->town != town && !ruling_caste(ch, ch->pcdata->hometown))
      {
         send_to_char("You need belong to that town to view it.\n\r", ch);
         return;
      }
      if (str_cmp(ch->name, ch->pcdata->town->mayor) && !ruling_caste(ch, ch->pcdata->hometown))
      {
         send_to_char("You cannot use this command, you do not have the authority.\n\r", ch);
         return;
      }
      for (schedule = town->first_schedule; schedule; schedule = schedule->next)
      {
         if (x++ == atoi(arg3))
         {
            UNLINK(schedule, town->first_schedule, town->last_schedule, next, prev);
            DISPOSE(schedule);
            send_to_char("Removed.\n\r", ch);
            return;
         }
      }
      send_to_char("That is not a valid number.\n\r", ch);
      return;
   }   
   if (!str_cmp(arg1, "add"))
   {
      town = get_town(arg2);
      if (!town)
      {
         send_to_char("That is not a valid town.\n\r", ch);
         return;
      }
      if (town->kingdom != ch->pcdata->hometown)
      {
         send_to_char("That town does not belong to your kingdom.\n\r", ch);
         return;
      }
      if (ch->pcdata->town != town && !ruling_caste(ch, ch->pcdata->hometown))
      {
         send_to_char("You need belong to that town to view it.\n\r", ch);
         return;
      }
      if (str_cmp(ch->name, ch->pcdata->town->mayor) && !ruling_caste(ch, ch->pcdata->hometown))
      {
         send_to_char("You cannot use this command, you do not have the authority.\n\r", ch);
         return;
      }
      if (atoi(arg3) < 1 || atoi(arg3) > 48)
      {
         send_to_char("Starting period ranges from 1 to 48.\n\r", ch);
         return;
      }
      if (atoi(arg4) < 1 || atoi(arg4) > 48)
      {
         send_to_char("Ending period ranges from 1 to 48.\n\r", ch);
         return;
      }
      if (atoi(arg4) < atoi(arg3))
      {
         send_to_char("Ending period needs to be after the Starting period!\n\r", ch);
         return;
      }   
      if (parse_resource_arg(arg5) == -1)
      {
         send_to_char("That is not a valid resource type.\n\r", ch);
         return;
      }
      if (!IN_WILDERNESS(ch))
      {
         send_to_char("You need to be in the wilderness to use this command.\n\r", ch);
         return;
      }
      if (!town->first_schedule)
      {
         CREATE(schedule, SCHEDULE_DATA, 1);
         schedule->start_period = atoi(arg3);
         schedule->end_period = atoi(arg4);
         schedule->resource = parse_resource_arg(arg5);
         schedule->x = ch->coord->x;
         schedule->y = ch->coord->y;
         schedule->map = ch->map;
         if (atoi(argument) > 0)
            schedule->reoccur = 1;
         LINK(schedule, town->first_schedule, town->last_schedule, next, prev);
         send_to_char("Added.\n\r", ch);
         return;
      }
      else
      {
         for (schedule = town->first_schedule; schedule; schedule = schedule->next)
         {
            if (schedule->start_period >= atoi(arg3))
            {
               CREATE(nschedule, SCHEDULE_DATA, 1);
               nschedule->start_period = atoi(arg3);
               nschedule->end_period = atoi(arg4);
               nschedule->resource = parse_resource_arg(arg5);
               nschedule->x = ch->coord->x;
               nschedule->y = ch->coord->y;
               nschedule->map = ch->map;
               if (atoi(argument) > 0)
                  nschedule->reoccur = 1;
               INSERT(nschedule, schedule, town->first_schedule, next, prev);
               send_to_char("Added.\n\r", ch);
               return;
            }
         }
         //WEll at the end so add it to the end
         CREATE(schedule, SCHEDULE_DATA, 1);
         schedule->start_period = atoi(arg3);
         schedule->end_period = atoi(arg4);
         schedule->resource = parse_resource_arg(arg5);
         schedule->x = ch->coord->x;
         schedule->y = ch->coord->y;
         schedule->map = ch->map;
         if (atoi(argument) > 0)
            schedule->reoccur = 1;
         LINK(schedule, town->first_schedule, town->last_schedule, next, prev);
         send_to_char("Added.\n\r", ch);
         return;
      }
   }        
   if (!str_cmp(arg1, "view"))
   {
      town = get_town(arg2);
      if (!town)
      {
         send_to_char("That is not a valid town.\n\r", ch);
         return;
      }
      if (town->kingdom != ch->pcdata->hometown)
      {
         send_to_char("That town does not belong to your kingdom.\n\r", ch);
         return;
      }
      if (ch->pcdata->town != town && !ruling_caste(ch, ch->pcdata->hometown))
      {
         send_to_char("You need belong to that town to view it.\n\r", ch);
         return;
      }
      if (str_cmp(ch->name, ch->pcdata->town->mayor) && !ruling_caste(ch, ch->pcdata->hometown))
      {
         send_to_char("You cannot use this command, you do not have the authority.\n\r", ch);
         return;
      }
      send_to_char("Num   St Mon Period   End Mon Period   Resource   X     Y     Reoccuring\n\r", ch);
      send_to_char("------------------------------------------------------------------------\n\r", ch);
      for (schedule = town->first_schedule; schedule; schedule = schedule->next)
      {
         ch_printf(ch, "%-3d      %-2d  %-2d           %-2d  %-2d       %-6s     %-4d  %-4d  %s\n\r", x++, (schedule->start_period-1)/4+1,
            (schedule->start_period-1)%4+1, (schedule->end_period-1)/4+1, (schedule->end_period-1)%4+1, 
            parse_resource_value(schedule->resource), schedule->x, schedule->y, schedule->reoccur ? "Yes" : "No");
      }  
      send_to_char("------------------------------------------------------------------------\n\r", ch);
      return;
   }
   do_schedule(ch, "");
   return;
}

/* All in one caste commands.  Used to get lists of things that can be purchases, etc. */
void do_buycaste(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   char buf1[MSL];
   MOB_INDEX_DATA *mob;
   OBJ_INDEX_DATA *obj;
   BUYKMOB_DATA *kmob;
   BUYKOBJ_DATA *kobj;
   BUYKBIN_DATA *kbin;
   BUYKTRAINER_DATA *ktrainer;
   int sn;
   int last = 0;
   int mplace = -1; //Holds array value for mob_que for kingdom
   int oplace = -1; //Holds array value for obj_que for kingdom
   int tplace = -1; //Holds array value for trainer_que for kingdom
   int x;

   for (x = 0; x <= 24; x++)
   {
      if (kingdom_table[ch->pcdata->hometown]->mob_que[x] == 0)
      {
         if (mplace == -1)
            mplace = x;
      }
      if (kingdom_table[ch->pcdata->hometown]->obj_que[x] == 0)
      {
         if (oplace == -1)
            oplace = x;
      }
      if (kingdom_table[ch->pcdata->hometown]->trainer_que[x] == 0)
      {
         if (tplace == -1)
            tplace = x;
      }
   }
   if (mplace == -1)
   {
      send_to_char("The mob que is full, empty a slot first.\n\r", ch);
      bug("BuyCaste: Hometown %s has a full mob que.\n\r", kingdom_table[ch->pcdata->hometown]->name);
      return;
   }
   if (oplace == -1)
   {
      send_to_char("The obj que is full, empty a slot first.\n\r", ch);
      bug("BuyCaste: Hometown %s has a full obj que.\n\r", kingdom_table[ch->pcdata->hometown]->name);
      return;
   }
   if (tplace == -1)
   {
      send_to_char("The trainer que is full, empty a slot first.\n\r", ch);
      bug("BuyCaste: Hometown %s has a full trainer que.\n\r", kingdom_table[ch->pcdata->hometown]->name);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: buycaste <option>\n\roption - mobs objects trainers silos.\n\r", ch);
      send_to_char("With trainers you can provide an extra option to list certain skills/spells.\n\r", ch);
      send_to_char("Note:  There is no queue with silos, they are placed when bought!!!\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   sprintf(buf, "%s", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
   if (!str_cmp(arg1, "mobs") || !str_cmp(arg1, "mob"))
   {
      if (isdigit(argument[0]))
      {
         for (kmob = first_buykmob; kmob; kmob = kmob->next)
         {
            last++;
         }
         if (last == 0)
         {
            send_to_char("There are no mobs to purchase at this time.\n\r", ch);
            return;
         }
         else
         {
            if (atoi(argument) < 1 || atoi(argument) > last)
            {
               send_to_char("Need to use the numbers in the braces.\n\r", ch);
               return;
            }
         }
         last = 0;
         for (kmob = first_buykmob; kmob; kmob = kmob->next)
         {
            last++;
            if (last == atoi(argument))
            {
               kingdom_table[ch->pcdata->hometown]->mob_que[mplace] = kmob->vnum;
               break;
            }
         }  
         ch_printf(ch, "The option you chose, [%d], is added to the mob que.\n\r", atoi(argument));
         return;
      }
      else
      {
         last = 1;
         ch_printf(ch, "Item to purchase                                  Resource      Amount                MinCaste\n\r");
         ch_printf(ch, "---------------------------------------------------------------------------------------------------------\n\r");
         for (kmob = first_buykmob; kmob; kmob = kmob->next)
         {
            mob = get_mob_index(kmob->vnum);
            if (!mob)
            {
               bug("buy_caste:  There is an invalid vnum of %d in the buykmob list.", kmob->vnum);
               continue;
            }
            if (xIS_SET(kmob->flags, KMOB_HOUR))
               sprintf(buf1, "/GL 1/4 Month");
            else if (xIS_SET(kmob->flags, KMOB_4MONTH))
               sprintf(buf1, "/GL 4 Months ");
            else
               sprintf(buf1, "             ");
               
            if (kmob->tree > 0) 
               ch_printf(ch, "[%-2d]   %-40s   Lumber        %-7d%s  %s\n\r", last, mob->short_descr, kmob->tree, buf1, buf);
            if (kmob->corn > 0)
               ch_printf(ch, "[%-2d]   %-40s   Corn          %-7d%s  %s\n\r", last, mob->short_descr, kmob->corn, buf1, buf);
            if (kmob->grain > 0)
               ch_printf(ch, "[%-2d]   %-40s   Grain         %-7d%s  %s\n\r", last, mob->short_descr, kmob->grain, buf1, buf);
            if (kmob->iron > 0)
               ch_printf(ch, "[%-2d]   %-40s   Iron          %-7d%s  %s\n\r", last, mob->short_descr, kmob->iron, buf1, buf);
            if (kmob->gold > 0)
               ch_printf(ch, "[%-2d]   %-40s   Gold          %-7d%s  %s\n\r", last, mob->short_descr, kmob->gold, buf1, buf);
            if (kmob->stone > 0)
               ch_printf(ch, "[%-2d]   %-40s   Stone         %-7d%s  %s\n\r", last, mob->short_descr, kmob->stone, buf1, buf);
            if (kmob->coins > 0)
               ch_printf(ch, "[%-2d]   %-40s   Gold Coins    %-7d%s  %s\n\r", last, mob->short_descr, kmob->coins, buf1, buf);
            last++;
         }
      }
   }
   else if (!str_cmp(arg1, "silos") || !str_cmp(arg1, "silo"))
   {
      last = 0;
      if (isdigit(argument[0]))
      {
         for (kbin = first_buykbin; kbin; kbin = kbin->next)
         {
            last++;
         }
         if (last == 0)
         {
            send_to_char("There are no silos to purchase at this time.\n\r", ch);
            return;
         }
         else
         {
            if (atoi(argument) < 1 || atoi(argument) > last)
            {
               send_to_char("Need to use the numbers in the braces.\n\r", ch);
               return;
            }
         }
         last = 0;
         for (kbin = first_buykbin; kbin; kbin = kbin->next)
         {
            last++;
            if (last == atoi(argument))
            {
               if (!IN_WILDERNESS(ch))
               {
                  send_to_char("You can only place a silo in the wilderness.\n\r", ch);
                  return;
               }
               if (ch->pcdata->hometown < 2)
               {
                  send_to_char("You need to belong to a kingdom to use this command.\n\r", ch);
                  return;
               }
               if (!ch->pcdata->town)
               {
                  send_to_char("You need to belong to a town to use this command.\n\r", ch);
                  return;
               }
               if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
               {
                  send_to_char("You can only place a silo in your town's area of control.\n\r", ch);
                  return;
               }
               if (map_sector[ch->map][ch->coord->x][ch->coord->y] != SECT_PAVE)
               {
                  send_to_char("You can only construct a silo on pavement.\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->coins < kbin->coins)
               {
                  send_to_char("Your town does not have enough gold coins to fund this!\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->stone < kbin->stone)
               {
                  send_to_char("Your town does not have enough stone to fund this!\n\r", ch);
                  return;
               }
               ch->pcdata->town->coins -= kbin->coins;
               ch->pcdata->town->stone -= kbin->stone;
               ch->pcdata->town->bincoords[ch->coord->x - ch->pcdata->town->startx+30][ch->coord->y - ch->pcdata->town->starty+30] = kbin->hold;
               ch->pcdata->town->hold += kbin->hold;
               write_kingdom_file(ch->pcdata->hometown);
               map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_HOLD;
               save_map("solan", 0);
               break;
            }
         }  
         ch_printf(ch, "The option you chose, [%d] has been placed here.\n\r", atoi(argument));
         return;
      }
      else
      {
         last = 1;
         ch_printf(ch, "Bin to purchase               Resource      Amount     Hold      MinCaste\n\r");
         ch_printf(ch, "--------------------------------------------------------------------------------------\n\r");
         for (kbin = first_buykbin; kbin; kbin = kbin->next)
         {              
            if (kbin->stone > 0) 
               ch_printf(ch, "[%-2d]   %-20s   Stone         %-8d   %-8d  %s\n\r", last, kbin->name, kbin->stone, kbin->hold, buf);
               
               ch_printf(ch, "[%-2d]   %-20s   Gold Coins    %-8d   %-8d  %s\n\r", last, kbin->name, kbin->coins, kbin->hold, buf);
            last++;
         }
      }
   }
   else if (!str_cmp(arg1, "objects") || !str_cmp(arg1, "objs") || !str_cmp(arg1, "object") || !str_cmp(arg1, "obj"))
   {
      last = 0;
      if (isdigit(argument[0]))
      {
         for (kobj = first_buykobj; kobj; kobj = kobj->next)
         {
            last++;
         }
         if (last == 0)
         {
            send_to_char("There are no objs to purchase at this time.\n\r", ch);
            return;
         }
         else
         {
            if (atoi(argument) < 1 || atoi(argument) > last)
            {
               send_to_char("Need to use the numbers in the braces.\n\r", ch);
               return;
            }
         }
         last = 0;
         for (kobj = first_buykobj; kobj; kobj = kobj->next)
         {
            last++;
            if (last == atoi(argument))
            {
               kingdom_table[ch->pcdata->hometown]->obj_que[oplace] = kobj->vnum;
               break;
            }
         }  
         ch_printf(ch, "The option you chose, [%d], is added to the obj que.\n\r", atoi(argument));
         return;
      }
      else
      {
         last = 1;
         ch_printf(ch, "Item to purchase                                  Resource      Amount    MinCaste\n\r");
         ch_printf(ch, "--------------------------------------------------------------------------------------\n\r");
         for (kobj = first_buykobj; kobj; kobj = kobj->next)
         {
            obj = get_obj_index(kobj->vnum);
            if (!obj)
            {
               bug("buy_caste:  There is an invalid vnum of %d in the buykobj list.", kobj->vnum);
               continue;
            }
               
            if (kobj->tree > 0) 
               ch_printf(ch, "[%-2d]   %-40s   Lumber        %-7d   %s\n\r", last, obj->short_descr, kobj->tree, buf);
            if (kobj->corn > 0)
               ch_printf(ch, "[%-2d]   %-40s   Corn          %-7d   %s\n\r", last, obj->short_descr, kobj->corn, buf);
            if (kobj->grain > 0)
               ch_printf(ch, "[%-2d]   %-40s   Grain         %-7d   %s\n\r", last, obj->short_descr, kobj->grain, buf);
            if (kobj->iron > 0)
               ch_printf(ch, "[%-2d]   %-40s   Iron          %-7d   %s\n\r", last, obj->short_descr, kobj->iron, buf);
            if (kobj->gold > 0)
               ch_printf(ch, "[%-2d]   %-40s   Gold          %-7d   %s\n\r", last, obj->short_descr, kobj->gold, buf);
            if (kobj->stone > 0)
               ch_printf(ch, "[%-2d]   %-40s   Stone         %-7d   %s\n\r", last, obj->short_descr, kobj->stone, buf);
            if (kobj->coins > 0)
               ch_printf(ch, "[%-2d]   %-40s   Gold Coins    %-7d   %s\n\r", last, obj->short_descr, kobj->coins, buf);
            last++;
         }
      }
   }
   else if (!str_cmp(arg1, "trainers") || !str_cmp(arg1, "trainer"))
   {
      last = 0;
      if (isdigit(argument[0]))
      {
         for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
         {
            last++;
         }
         if (last == 0)
         {
            send_to_char("There are no trainers to purchase at this time.\n\r", ch);
            return;
         }
         else
         {
            if (atoi(argument) < 1 || atoi(argument) > last)
            {
               send_to_char("Need to use the numbers in the braces.\n\r", ch);
               return;
            }
         }
         last = 0;
         for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
         {
            last++;
            if (last == atoi(argument))
            {
               kingdom_table[ch->pcdata->hometown]->trainer_que[oplace] = last;
               break;
            }
         }  
         ch_printf(ch, "The option you chose, [%d], is added to the trainer que.\n\r", atoi(argument));
         return;
      }
      else if ((sn = skill_lookup(argument)) >= 0)
      {
         last = 1;
         ch_printf(ch, "Name of Trainer         Cost    Skill/Spell                Mastery   Skill/Spell                Mastery   \n\r");
         for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
         {
            if (ktrainer->sn[0] == sn || ktrainer->sn[1] == sn || ktrainer->sn[2] == sn || 
                ktrainer->sn[3] == sn || ktrainer->sn[4] == sn || ktrainer->sn[5] == sn ||
                ktrainer->sn[6] == sn || ktrainer->sn[7] == sn || ktrainer->sn[9] == sn ||
                ktrainer->sn[10] == sn || ktrainer->sn[11] == sn || ktrainer->sn[12] == sn || 
                ktrainer->sn[13] == sn || ktrainer->sn[14] == sn || ktrainer->sn[15] == sn ||
                ktrainer->sn[16] == sn || ktrainer->sn[17] == sn || ktrainer->sn[19] == sn)
            {
               ch_printf(ch, "[%-3d]  %-15s  %-6d  %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                  last, ktrainer->name, ktrainer->cost, ktrainer->sn[0] > 0 ? skill_table[ktrainer->sn[0]]->name : "", ktrainer->mastery[0] > 0 ? get_mastery_name(ktrainer->mastery[0]) : "",  
                  ktrainer->sn[1] > 0 ? skill_table[ktrainer->sn[1]]->name : "", ktrainer->mastery[1] > 0 ? get_mastery_name(ktrainer->mastery[1]) : "", 
                  ktrainer->sn[2] > 0 ? skill_table[ktrainer->sn[2]]->name : "", ktrainer->mastery[2] > 0 ? get_mastery_name(ktrainer->mastery[2]) : "", 
                  ktrainer->sn[3] > 0 ? skill_table[ktrainer->sn[3]]->name : "", ktrainer->mastery[3] > 0 ? get_mastery_name(ktrainer->mastery[3]) : "", 
                  ktrainer->sn[4] > 0 ? skill_table[ktrainer->sn[4]]->name : "", ktrainer->mastery[4] > 0 ? get_mastery_name(ktrainer->mastery[4]) : ""); 
               if (ktrainer->sn[5] > 0)
               {
                  ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                     ktrainer->sn[5] > 0 ? skill_table[ktrainer->sn[5]]->name : "", ktrainer->mastery[5] > 0 ? get_mastery_name(ktrainer->mastery[5]) : "",  
                     ktrainer->sn[6] > 0 ? skill_table[ktrainer->sn[6]]->name : "", ktrainer->mastery[6] > 0 ? get_mastery_name(ktrainer->mastery[6]) : "", 
                     ktrainer->sn[7] > 0 ? skill_table[ktrainer->sn[7]]->name : "", ktrainer->mastery[7] > 0 ? get_mastery_name(ktrainer->mastery[7]) : "", 
                     ktrainer->sn[8] > 0 ? skill_table[ktrainer->sn[8]]->name : "", ktrainer->mastery[8] > 0 ? get_mastery_name(ktrainer->mastery[8]) : "", 
                     ktrainer->sn[9] > 0 ? skill_table[ktrainer->sn[9]]->name : "", ktrainer->mastery[9] > 0 ? get_mastery_name(ktrainer->mastery[9]) : "");    
               }
               if (ktrainer->sn[10] > 0)
               {
                  ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                     ktrainer->sn[10] > 0 ? skill_table[ktrainer->sn[10]]->name : "", ktrainer->mastery[10] > 0 ? get_mastery_name(ktrainer->mastery[10]) : "",  
                     ktrainer->sn[11] > 0 ? skill_table[ktrainer->sn[11]]->name : "", ktrainer->mastery[11] > 0 ? get_mastery_name(ktrainer->mastery[11]) : "", 
                     ktrainer->sn[12] > 0 ? skill_table[ktrainer->sn[12]]->name : "", ktrainer->mastery[12] > 0 ? get_mastery_name(ktrainer->mastery[12]) : "", 
                     ktrainer->sn[13] > 0 ? skill_table[ktrainer->sn[13]]->name : "", ktrainer->mastery[13] > 0 ? get_mastery_name(ktrainer->mastery[13]) : "", 
                     ktrainer->sn[14] > 0 ? skill_table[ktrainer->sn[14]]->name : "", ktrainer->mastery[14] > 0 ? get_mastery_name(ktrainer->mastery[14]) : "");    
               }
               if (ktrainer->sn[15] > 0)
               {
                  ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                     ktrainer->sn[15] > 0 ? skill_table[ktrainer->sn[15]]->name : "", ktrainer->mastery[15] > 0 ? get_mastery_name(ktrainer->mastery[15]) : "",  
                     ktrainer->sn[16] > 0 ? skill_table[ktrainer->sn[16]]->name : "", ktrainer->mastery[16] > 0 ? get_mastery_name(ktrainer->mastery[16]) : "", 
                     ktrainer->sn[17] > 0 ? skill_table[ktrainer->sn[17]]->name : "", ktrainer->mastery[17] > 0 ? get_mastery_name(ktrainer->mastery[17]) : "", 
                     ktrainer->sn[18] > 0 ? skill_table[ktrainer->sn[18]]->name : "", ktrainer->mastery[18] > 0 ? get_mastery_name(ktrainer->mastery[18]) : "", 
                     ktrainer->sn[19] > 0 ? skill_table[ktrainer->sn[19]]->name : "", ktrainer->mastery[19] > 0 ? get_mastery_name(ktrainer->mastery[19]) : "");    
               }
            }
            last++;
         }
      }  
      else
      {
         last = 1;
         ch_printf(ch, "Name of Trainer         Cost    Skill/Spell                Mastery   Skill/Spell                Mastery   \n\r");
         for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
         {
            ch_printf(ch, "[%-3d]  %-15s  %-6d  %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
               last, ktrainer->name, ktrainer->cost, ktrainer->sn[0] > 0 ? skill_table[ktrainer->sn[0]]->name : "", ktrainer->mastery[0] > 0 ? get_mastery_name(ktrainer->mastery[0]) : "",  
               ktrainer->sn[1] > 0 ? skill_table[ktrainer->sn[1]]->name : "", ktrainer->mastery[1] > 0 ? get_mastery_name(ktrainer->mastery[1]) : "", 
               ktrainer->sn[2] > 0 ? skill_table[ktrainer->sn[2]]->name : "", ktrainer->mastery[2] > 0 ? get_mastery_name(ktrainer->mastery[2]) : "", 
               ktrainer->sn[3] > 0 ? skill_table[ktrainer->sn[3]]->name : "", ktrainer->mastery[3] > 0 ? get_mastery_name(ktrainer->mastery[3]) : "", 
               ktrainer->sn[4] > 0 ? skill_table[ktrainer->sn[4]]->name : "", ktrainer->mastery[4] > 0 ? get_mastery_name(ktrainer->mastery[4]) : ""); 
            last++;
            if (ktrainer->sn[5] > 0)
            {
               ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                  ktrainer->sn[5] > 0 ? skill_table[ktrainer->sn[5]]->name : "", ktrainer->mastery[5] > 0 ? get_mastery_name(ktrainer->mastery[5]) : "",  
                  ktrainer->sn[6] > 0 ? skill_table[ktrainer->sn[6]]->name : "", ktrainer->mastery[6] > 0 ? get_mastery_name(ktrainer->mastery[6]) : "", 
                  ktrainer->sn[7] > 0 ? skill_table[ktrainer->sn[7]]->name : "", ktrainer->mastery[7] > 0 ? get_mastery_name(ktrainer->mastery[7]) : "", 
                  ktrainer->sn[8] > 0 ? skill_table[ktrainer->sn[8]]->name : "", ktrainer->mastery[8] > 0 ? get_mastery_name(ktrainer->mastery[8]) : "", 
                  ktrainer->sn[9] > 0 ? skill_table[ktrainer->sn[9]]->name : "", ktrainer->mastery[9] > 0 ? get_mastery_name(ktrainer->mastery[9]) : "");    
            }
            if (ktrainer->sn[10] > 0)
            {
               ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                  ktrainer->sn[10] > 0 ? skill_table[ktrainer->sn[10]]->name : "", ktrainer->mastery[10] > 0 ? get_mastery_name(ktrainer->mastery[10]) : "",  
                  ktrainer->sn[11] > 0 ? skill_table[ktrainer->sn[11]]->name : "", ktrainer->mastery[11] > 0 ? get_mastery_name(ktrainer->mastery[11]) : "", 
                  ktrainer->sn[12] > 0 ? skill_table[ktrainer->sn[12]]->name : "", ktrainer->mastery[12] > 0 ? get_mastery_name(ktrainer->mastery[12]) : "", 
                  ktrainer->sn[13] > 0 ? skill_table[ktrainer->sn[13]]->name : "", ktrainer->mastery[13] > 0 ? get_mastery_name(ktrainer->mastery[13]) : "", 
                  ktrainer->sn[14] > 0 ? skill_table[ktrainer->sn[14]]->name : "", ktrainer->mastery[14] > 0 ? get_mastery_name(ktrainer->mastery[14]) : "");    
            }
            if (ktrainer->sn[15] > 0)
            {
               ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                  ktrainer->sn[15] > 0 ? skill_table[ktrainer->sn[15]]->name : "", ktrainer->mastery[15] > 0 ? get_mastery_name(ktrainer->mastery[15]) : "",  
                  ktrainer->sn[16] > 0 ? skill_table[ktrainer->sn[16]]->name : "", ktrainer->mastery[16] > 0 ? get_mastery_name(ktrainer->mastery[16]) : "", 
                  ktrainer->sn[17] > 0 ? skill_table[ktrainer->sn[17]]->name : "", ktrainer->mastery[17] > 0 ? get_mastery_name(ktrainer->mastery[17]) : "", 
                  ktrainer->sn[18] > 0 ? skill_table[ktrainer->sn[18]]->name : "", ktrainer->mastery[18] > 0 ? get_mastery_name(ktrainer->mastery[18]) : "", 
                  ktrainer->sn[19] > 0 ? skill_table[ktrainer->sn[19]]->name : "", ktrainer->mastery[19] > 0 ? get_mastery_name(ktrainer->mastery[19]) : "");    
            }
         }
      }
   }
   else
   {
      send_to_char("Syntax: buycaste <option>\n\roption - mobs objects trainers silos.\n\r", ch);
      return;
   }
   return;
}

//Used to haul a storage bin back into camp
void do_carrybin(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *hch;
   OBJ_DATA *obj;
   char arg[MIL];
   
   
   if (IS_NPC(ch))
   {
      send_to_char("For Pcs only.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: carrybin on/off\n\r", ch);
      send_to_char("Syntax: carrybin <obj> <dir>\n\r", ch);
      send_to_char("See the helpfile for more detailed information.\n\r", ch);
      return;
   }
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("Only in the wilderness.\n\r", ch);
      return;
   }
   if (!str_prefix(argument, "on"))
   {
      xSET_BIT(ch->act, PLR_CARRYBIN);
      send_to_char("You are now ready to start carrying a storage bin.\n\r", ch);
      return;
   }
   if (!str_prefix(argument, "off"))
   {
      if (xIS_SET(ch->act, PLR_CARRYBIN))
      {
         xREMOVE_BIT(ch->act, PLR_CARRYBIN);
         send_to_char("You change your mind about carrying a storage bin.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("You never said you where going to carry one.\n\r", ch);
         return;
      }
   }
   argument = one_argument(argument, arg);
   if ((obj = get_obj_here(ch, arg)) == NULL)
   {
      send_to_char("That is not here.\n\r", ch);
      return;
   }
   if (obj->item_type != ITEM_HOLDRESOURCE)
   {
      send_to_char("That is not a storage bin.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "n") || !str_cmp(argument, "e") || !str_cmp(argument, "s") ||
       !str_cmp(argument, "w") || !str_cmp(argument, "north") || !str_cmp(argument, "east") ||
       !str_cmp(argument, "south") || !str_cmp(argument, "west") || !str_cmp(argument, "nw") ||
       !str_cmp(argument, "ne") || !str_cmp(argument, "sw") || !str_cmp(argument, "se") ||
       !str_cmp(argument, "northwest") || !str_cmp(argument, "northeast") || !str_cmp(argument, "southwest") ||
       !str_cmp(argument, "southeast"))
   {
      int nstr = 0;
      int hstr = 0;
      int count = 1;
      int x = ch->coord->x;
      int y = ch->coord->y;
      int nx;
      int ny;
      
      xSET_BIT(ch->act, PLR_CARRYBIN);
      
      nstr += obj->value[0]/500; //Size of the bin, heavier the bin, the more it takes to move it even empty.....
      nstr += obj->value[2]/250;
      nstr += obj->value[4]/250;
      
      hstr += get_curr_str(ch);
      
      for (hch = ch->in_room->first_person; hch; hch = hch->next_in_room)
      {
         if (hch != ch && !IS_NPC(hch) && hch->coord->x == ch->coord->x && hch->coord->y == ch->coord->y && hch->map == ch->map 
         && xIS_SET(hch->act, PLR_CARRYBIN) && hch->pcdata->hometown == ch->pcdata->hometown)
         {
            count++;
            hstr += get_curr_str(hch);
         }
      }
      if (hstr < nstr)
      {
         ch_printf(ch, "Even the strength of %d is not enough to move this bin.\n\r", count);
         return;
      }
      for (hch = ch->in_room->first_person; hch; hch = hch->next_in_room)
      {
         if (hch != ch && !IS_NPC(hch) && hch->coord->x == ch->coord->x && hch->coord->y == ch->coord->y && hch->map == ch->map 
         && xIS_SET(hch->act, PLR_CARRYBIN) && hch->pcdata->hometown == ch->pcdata->hometown)
         {
            send_to_char("It looks like the group is getting ready to move the bin.\n\r", hch);
         }
      }
      if (!str_cmp(argument, "n") || !str_cmp(argument, "north") || !str_cmp(argument, "ne")
      || !str_cmp(argument, "northeast") || !str_cmp(argument, "nw") || !str_cmp(argument, "northwest"))
         y = ch->coord->y-1;
      if (!str_cmp(argument, "s") || !str_cmp(argument, "south") || !str_cmp(argument, "se")
      || !str_cmp(argument, "sw") || !str_cmp(argument, "southeast") || !str_cmp(argument, "southwest"))
         y = ch->coord->y+1;
      if (!str_cmp(argument, "e") || !str_cmp(argument, "east") || !str_cmp(argument, "ne")
      || !str_cmp(argument, "northeast") || !str_cmp(argument, "se") || !str_cmp(argument, "southeast"))
         x = ch->coord->x+1;
      if (!str_cmp(argument, "w") || !str_cmp(argument, "west") || !str_cmp(argument, "sw")
      || !str_cmp(argument, "southwest") || !str_cmp(argument, "nw") || !str_cmp(argument, "northwest"))
         x = ch->coord->x-1;
        
       nx = ch->coord->x;
       ny = ch->coord->y;
       hstr = 0;
        
       if (sect_show[map_sector[ch->map][x][y]].canpass == FALSE || map_sector[ch->map][x][y] == SECT_EXIT)
       {
          send_to_char("You cannot carry a bin in that direction!\n\r", ch);
          return;
       }
       for (hch = ch->in_room->first_person; hch; hch = hch->next_in_room)
       {
          if (hch != ch && !IS_NPC(hch) && hch->coord->x == ch->coord->x && hch->coord->y == ch->coord->y && hch->map == ch->map 
          && xIS_SET(hch->act, PLR_CARRYBIN) && hch->pcdata->hometown == ch->pcdata->hometown)
          {
             interpret(hch, argument);
             if (hch->coord->x != nx || hch->coord->y != ny)
                hstr += get_curr_str(hch);           
          }
       }
       interpret(ch, argument);
       if (ch->coord->x != nx || ch->coord->y != ny)
          hstr += get_curr_str(ch);   
          
       if (hstr < nstr)
       {
          send_to_char("Looks like someone was not able to move with the group.\n\r", ch);
          return;
       }
       obj->coord->x = x;
       obj->coord->y = y;
       send_to_char("You successfully drag the storage bin.\n\r", ch);
       save_bin_data();
       return;
    }
    do_carrybin(ch, "");
    return;
}
         
   
// Used to place an object in the Obj Que, also gets the objque list.
void do_placeobj(CHAR_DATA * ch, char *argument)
{
   OBJ_INDEX_DATA *obj;
   OBJ_DATA *robj;
   OBJ_INDEX_DATA *objIndex;
   OBJ_DATA *finalobj;
   RESET_DATA *pReset;
   BUYKOBJ_DATA *kobj;
   char arg[MIL];
   char buf[MSL];
   int x;
   int vnum;

   if (ch->pcdata->hometown == -1)
   {
      send_to_char("Your hometown cannot use this option, sorry.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to use this command.\n\r", ch);
      return;
   }
   sprintf(buf, "%s", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
   if (argument[0] == '\0')
   {
      ch_printf(ch, "Obj in Que                              Resource      Amount    MinCaste\n\r");
      ch_printf(ch, "-----------------------------------------------------------------------------\n\r");
      for (x = 0; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->obj_que[x] > 0)
         {
            obj = get_obj_index(kingdom_table[ch->pcdata->hometown]->obj_que[x]);
            if (obj)
            for (kobj = first_buykobj; kobj; kobj = kobj->next)
            {
               if (kobj->vnum == obj->vnum)
               {
                  if (kobj->tree > 0) 
                     ch_printf(ch, "[%-2d]   %-30s   Lumber        %-7d   %s\n\r", x+1, obj->short_descr, kobj->tree, buf);
                  if (kobj->corn > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Corn          %-7d   %s\n\r", x+1, obj->short_descr, kobj->corn, buf);
                  if (kobj->grain > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Grain         %-7d   %s\n\r", x+1, obj->short_descr, kobj->grain, buf);
                  if (kobj->iron > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Iron          %-7d   %s\n\r", x+1, obj->short_descr, kobj->iron, buf);
                  if (kobj->gold > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Gold          %-7d   %s\n\r", x+1, obj->short_descr, kobj->gold, buf);
                  if (kobj->stone > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Stone         %-7d   %s\n\r", x+1, obj->short_descr, kobj->stone, buf);
                  if (kobj->coins > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Gold Coins    %-7d   %s\n\r", x+1, obj->short_descr, kobj->coins, buf);
               }
            }
         }
      }
      return;
   }
   if (!str_cmp(argument, "clear"))
   {
      for (x = 0; x <= 24; x++)
      {
         kingdom_table[ch->pcdata->hometown]->obj_que[x] = 0;
      }
      send_to_char("Cleared.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (isdigit(arg[0]))
   {
      for (x = 0; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->obj_que[x] > 0)
            continue;
         else
            break;
      }
      if (x == 0)
      {
         send_to_char("You have nothing in que to place, sorry.\n\r", ch);
         return;
      }
      if (atoi(arg) < 1 || atoi(arg) > x)
      {
         ch_printf(ch, "You choices are 1 through %d\n\r", x);
         return;
      }
      obj = get_obj_index(kingdom_table[ch->pcdata->hometown]->obj_que[atoi(arg) - 1]);
      if (!obj)
      {
         send_to_char("There is an error with the obj que, tell an immortal.\n\r", ch);
         return;
      }
      for (kobj = first_buykobj; kobj; kobj = kobj->next)
      {
         if (obj->vnum == kobj->vnum)
            break;
      }
      if (!kobj)
      {
         bug("do_placeobj:  An item in que [%d] does not exist in the list.", obj->vnum);
         send_to_char("There was an error, tell an immorta.\n\r", ch);
         return;
      }
      if (xIS_SET(kobj->flags, KOBJ_HOMETOWN))
      //if ((obj->vnum >= OBJ_KINGDOM_THRONE && obj->vnum <= OBJ_KINGDOM_ALEFOUNTAIN)
     //    || (obj->vnum >= OBJ_KINGDOM_BOX && obj->vnum <= OBJ_KINGDOM_NOTEBOARD))
      {
         if (ch->in_room->area->kingdom != ch->pcdata->hometown)
         {
            send_to_char("You can only place this object in your hometown.\n\r", ch);
            return;
         }
      }
      if (xIS_SET(kobj->flags, KOBJ_WILDERNESS))
   //   if (obj->vnum >= OBJ_KINGDOM_LS_BIN && obj->vnum <= OBJ_KINGDOM_CG_BIN)
      {
         if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
         {
            send_to_char("You can only place these items in the Wilderness.\n\r", ch);
            return;
         }
      }
      if (kingdom_table[ch->pcdata->hometown]->minplace > ch->pcdata->caste)
      {
         ch_printf(ch, "It requires caste %s to place this object.\n\r", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
         return;
      }
      if (xIS_SET(kobj->flags, KOBJ_CONTAINER))
      //if (obj->vnum >= OBJ_KINGDOM_BOX && obj->vnum <= OBJ_KINGDOM_GIGAN_CHEST)
      {
         for (robj = ch->in_room->first_content; robj; robj = robj->next_content)
            if (robj->item_type == ITEM_CONTAINER && IN_SAME_ROOM_OBJ(ch, robj))
            {
               send_to_char("If there are containers in this room, get them and try again.\n\r", ch);
               return;
            }
      }
      if (!proper_resources(ch, kobj))
      {
         ch_printf(ch, "You don't have enough resources to purchase Option %d\n\r", atoi(arg));
         return;
      }
      vnum = kingdom_table[ch->pcdata->hometown]->obj_que[atoi(arg) - 1];
      if (xIS_SET(kobj->flags, KOBJ_NOTEBOARD)) 
      {
         int fvnum, evnum = 0;
         int hlim = 0, llim = 0;
         int num = 1;
         int fnd = 0;
         OBJ_DATA *broom;
         BOARD_DATA *board;

         for (broom = ch->in_room->first_content; broom; broom = broom->next_content)
         {
            if (broom->item_type == ITEM_NOTEBOARD && IN_SAME_ROOM_OBJ(ch, broom))
            {
               send_to_char("You can only have one noteboard per room.\n\r", ch);
               return;
            }
         }
         
         hlim = ch->in_room->area->hi_o_vnum;
         llim = ch->in_room->area->low_o_vnum;
         for (fvnum = llim; fvnum; fvnum++)
         {
            if (fvnum > hlim)
            {
               bug("do_buycaste: The area of %s is full of items.", ch->in_room->area->name);
               return;
            }
            if (!get_obj_index(fvnum))
            {
               evnum = fvnum;
               break;
            }
         }
         if (argument[0] == '\0')
         {
            send_to_char("You need to supply a name for this board when you place it.\n\rex: placeobj 2 Xerves's Board.\n\r", ch);
            return;
         }
         if (!is_made_room(ch->coord->x, ch->coord->y, ch->map, ch->pcdata->town))
         {
            send_to_char("You can only place a noteboard in a room you have made with makeroom.\n\r", ch);
            return;
         }
           
         objIndex = make_object(evnum, obj->vnum, obj->name, 1);

         if (!objIndex)
         {
            log_string("placeobj: new board was note created.");
            send_to_char("A bug happened, notify the imms.\n\r", ch);
            return;
         }
         xREMOVE_BIT(objIndex->extra_flags, ITEM_PROTOTYPE);
         REMOVE_BIT(objIndex->wear_flags, ITEM_TAKE);
         STRFREE(objIndex->short_descr);
         STRFREE(objIndex->name);
         STRFREE(objIndex->description);
         sprintf(buf, "%s", argument);
         objIndex->name = STRALLOC(buf);
         sprintf(buf, "%s", argument);
         objIndex->short_descr = STRALLOC(buf);
         sprintf(buf, "%s is attached to the wall and ready for notes.", argument);
         objIndex->description = STRALLOC(buf);
         finalobj = create_object(objIndex, 0);
         //Find the board file name to save :-)
         sprintf(buf, "kingdomb1.brd");
         for (;;)
         {
            fnd = 0;
            for (board = first_board; board; board = board->next)
            {
               if (!str_cmp(buf, board->note_file))
               {
                  num++;
                  fnd = 1;
                  sprintf(buf, "kingdomb%d.brd", num);
                  break;
               }
            }
            if (fnd == 0)
               break;
         }
         //Create new board
         smash_tilde(buf);
         CREATE(board, BOARD_DATA, 1);
         LINK(board, first_board, last_board, next, prev);
         board->note_file = str_dup(strlower(buf));
         board->board_obj = finalobj->pIndexData->vnum;
         board->read_group = str_dup("");
         board->post_group = str_dup("");
         board->extra_readers = str_dup("");
         board->extra_removers = str_dup("");
         board->min_read_level = 1;
         board->min_post_level = 1;
         board->min_remove_level = LEVEL_PC;
         board->max_posts = 50;
         write_boards_txt();
      }
      else
      {
         if (xIS_SET(kobj->flags, KOBJ_ADDRESET) || xIS_SET(kobj->flags, KOBJ_CONTAINER))
         {
            if (!is_made_room(ch->coord->x, ch->coord->y, ch->map, ch->pcdata->town))
            {
               send_to_char("You can only place this obj in a room you have made with makeroom.\n\r", ch);
               return;
            }
         }
            
         objIndex = get_obj_index(vnum);
         finalobj = create_object(objIndex, 0);
         if (xIS_SET(kobj->flags, KOBJ_SIGIL))
         {
            STRFREE(finalobj->short_descr);
            STRFREE(finalobj->name);
            STRFREE(finalobj->description);
            sprintf(buf, "Sigil %s", kingdom_table[ch->pcdata->hometown]->name);
            finalobj->name = STRALLOC(buf);
            sprintf(buf, "a Sigil of the Kingdom %s", kingdom_table[ch->pcdata->hometown]->name);
            finalobj->short_descr = STRALLOC(buf);
            sprintf(buf, "A Sigil of the kingdom %s has been strangely dropped here.", kingdom_table[ch->pcdata->hometown]->name);
            finalobj->description = STRALLOC(buf);
            finalobj->level = 1;
            finalobj->pIndexData->level = 1;
         }
      }
      if (xIS_SET(kobj->flags, KOBJ_TOCHAR))
      {
         if (get_ch_carry_number(ch) + (get_obj_number(finalobj) / finalobj->count) > can_carry_n(ch))
         {
            send_to_char("You have your hands full, try putting away some items first.\n\r", ch);
            return;
         }

         if (get_ch_carry_weight(ch) + (get_obj_weight(finalobj) / finalobj->count) > can_carry_w(ch))
         {
            send_to_char("You cannot carry that much weight.\n\r", ch);
            return;
         }

         if (!can_see_obj_map(ch, finalobj))
         {
            send_to_char("What obj?  You cannot see it.\n\r", ch);
            return;
         }
         obj_to_char(finalobj, ch);
      }
      else if (xIS_SET(kobj->flags, KOBJ_TOROOM))
      {
         obj_to_room(finalobj, ch->in_room, ch);
         finalobj->in_room = ch->in_room;
      }   
      else
      {
         if (get_ch_carry_number(ch) + (get_obj_number(finalobj) / finalobj->count) > can_carry_n(ch))
         {
            send_to_char("You have your hands full, try putting away some items first.\n\r", ch);
            return;
         }

         if (get_ch_carry_weight(ch) + (get_obj_weight(finalobj) / finalobj->count) > can_carry_w(ch))
         {
            send_to_char("You cannot carry that much weight.\n\r", ch);
            return;
         }

         if (!can_see_obj(ch, finalobj))
         {
            send_to_char("What obj?  You cannot see it.\n\r", ch);
            return;
         }
         obj_to_char(finalobj, ch);
      }
      fix_kingdom_oque(ch, atoi(argument) - 1);
      finalobj->coord->x = ch->coord->x;
      finalobj->coord->y = ch->coord->y;
      finalobj->map = ch->map;
      
      if (xIS_SET(kobj->flags, KOBJ_ADDRESET))
      {
         int cnt = 0;
         
         cnt = count_obj_list(finalobj->pIndexData, ch->in_room->first_content);
         
         for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
         {
            if (pReset->command == 'O' &&  pReset->arg1 == finalobj->pIndexData->vnum && pReset->arg3 == ch->in_room->vnum)
            {
               pReset->arg2 = cnt;
            }
         }

         add_obj_reset(ch->in_room->area, 'O', finalobj, cnt, ch->in_room->vnum);
         fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
      }
      if (xIS_SET(kobj->flags, KOBJ_BIN))
      {
         BIN_DATA *blist;

         CREATE(blist, BIN_DATA, 1);
         blist->x = ch->coord->x;
         blist->y = ch->coord->y;
         blist->map = ch->map;
         blist->room = ch->in_room->vnum;
         blist->vnum = finalobj->pIndexData->vnum;
         blist->bin1 = finalobj->value[2];
         blist->bin2 = finalobj->value[4];
         blist->serial = finalobj->serial;
         LINK(blist, first_bin, last_bin, next, prev);
         save_bin_data();
      }
      if (xIS_SET(kobj->flags, KOBJ_CONTAINER))
      {
         KCHEST_DATA *kchest;

         CREATE(kchest, KCHEST_DATA, 1);
         kchest->obj = finalobj;
         LINK(kchest, first_kchest, last_kchest, next, prev);
         save_kingdom_chests(ch);
      }
      ch_printf(ch, "Option %d (obj %s) has been placed here.\n\r", atoi(arg), finalobj->short_descr);
      remove_resources(ch, kobj);
      sprintf(logb, "Placeobj: %s bought %s", PERS_KINGDOM(ch, ch->pcdata->hometown), finalobj->short_descr);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLACEOBJ);
      return;
   }
   else
   {
      send_to_char("Syntax: placeobj - List of objs in Que\n\rSyntax: placeobj [number] - Place obj (number is in que list)\n\r", ch);
      return;
   }
   return;
}

void update_kremove_resources(CHAR_DATA *ch, BUYKOBJ_DATA *kobj, BUYKMOB_DATA *kmob)
{
   if (kobj)
   {
      ch->pcdata->town->lumber +=  kobj->tree/2;
      ch->pcdata->town->corn +=  kobj->corn/2;
      ch->pcdata->town->grain += kobj->grain/2;
      ch->pcdata->town->iron +=  kobj->iron/2;
      ch->pcdata->town->gold +=  kobj->gold/2;
      ch->pcdata->town->stone += kobj->stone/2;
      ch->pcdata->town->coins +=      kobj->coins/2;
   }
   else
   {
      ch->pcdata->town->lumber +=  kmob->tree/2;
      ch->pcdata->town->corn +=  kmob->corn/2;
      ch->pcdata->town->grain += kmob->grain/2;
      ch->pcdata->town->iron +=  kmob->iron/2;
      ch->pcdata->town->gold +=  kmob->gold/2;
      ch->pcdata->town->stone += kmob->stone/2;
      ch->pcdata->town->coins +=      kmob->coins/2;
   }
}            

//Old build alter stuff plus the new mountain/desert terraforming commands.
void do_terraform(CHAR_DATA *ch, char *argument)
{
   int sector;
   char arg[MIL];
   
   if (check_npc(ch))
      return;
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("You need to be in the wilderness to use this command.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to use this command.\n\r", ch);
      return;
   }    
   if (ch->pcdata->hometown <= 1)
   {
      send_to_char("You need to belong to a kingdom to use this command.\n\r", ch);
      return;
   }
   if (ch->pcdata->job != 4 && ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minbuild)
   {
      send_to_char("Only a carpenter or those with caste equal or better than MinBuild can use this command.\n\r", ch);
      return;
   }
   if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] >= 1 &&
       kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)
   {
      send_to_char("You can only use this command on unclaimed land or your own land!\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  terraform mountains\n\r", ch);
      send_to_char("Syntax:  terraform desert\n\r", ch);
      send_to_char("Syntax:  terraform layroad [direction]\n\r", ch);
      send_to_char("Syntax:  terraform cutpath\n\r", ch);
      send_to_char("Syntax:  terraform plains\n\r", ch);
      send_to_char("Syntax:  terraform digstone\n\r", ch);
      send_to_char("Syntax:  terraform torchland\n\r", ch);
      send_to_char("Syntax:  terraform stopfire\n\r", ch);
      send_to_char("Syntax:  terraform pave\n\r", ch);
      send_to_char("Syntax:  terraform water\n\r", ch);
      send_to_char("Syntax:  terraform planttree <name of obj>\n\r", ch);
      send_to_char("Syntax:  terraform plantcorn <name of obj>\n\r", ch);
      send_to_char("Syntax:  terraform plantgrain <name of obj>\n\r", ch);
      send_to_char("Syntax:  terraform plantgrass <name of obj>\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "stopfire"))
   {
      stopfire(ch, argument);
      return;
   }
   if (!str_cmp(arg, "planttree"))
   {
      planttree(ch, argument);
      return;
   }
   if (!str_cmp(arg, "plantcorn"))
   {
      plantcorn(ch, argument);
      return;
   }
   if (!str_cmp(arg, "plantgrain"))
   {
      plantgrain(ch, argument);
      return;
   }
   if (!str_cmp(arg, "plantgrass"))
   {
      plantgrass(ch, argument);
      return;
   }
   if (!str_cmp(arg, "layroad"))
   {
      layroad(ch, argument);
      return;
   }
   if (!str_cmp(arg, "torchland"))
   {
      send_to_char("Disabled for now.\n\r", ch);
      //torchland(ch, argument);
      return;
   }
   if (!str_cmp(arg, "cutpath"))
   {
      cutpath(ch, argument);
      return;
   }
   if (!str_cmp(arg, "plains"))
   {
      change_plains(ch, argument);
      return;
   }
   if (!str_cmp(arg, "digstone"))
   {
      digstone(ch, argument);
      return;
   }
   if (!str_cmp(arg, "pave"))
   {
      if (sector != SECT_FIELD && sector != SECT_NCORN && sector != SECT_NGRAIN && sector != SECT_NTREE
      &&  sector != SECT_SWAMP && sector != SECT_JUNGLE && sector != SECT_BURNT 
      &&  sector != SECT_PLAINS && sector != SECT_HILLS && sector != SECT_ROAD && sector != SECT_PATH
      &&  sector != SECT_CITY && sector != SECT_PAVE)
      {
         send_to_char("Cannot pave over this sector.\n\r", ch);
         return;
      }
      map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_PAVE;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "water"))
   {
      if (sector != SECT_PAVE)
      {
         send_to_char("You can only start a water sector on pavement.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->coins < 5000)
      {
         send_to_char("It costs 5k coins to terraform into a water sector.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->stone < 500)
      {
         send_to_char("It costs 500 stone units to terraform into a water sector.\n\r", ch);
         return;
      }
      ch->pcdata->town->coins -= 5000;      
      ch->pcdata->town->stone -= 500;
      map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_WATER_NOSWIM;
      resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
      send_to_char("Done.\n\r", ch);
      return;
   }
  
   if (!str_cmp(arg, "desert"))
   {
      if (sector != SECT_DESERT && sector != SECT_SHORE)
      {
         send_to_char("This only works on deserts and shore sectors.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->coins < 8000)
      {
         send_to_char("It costs 8k coins to terraform a desert/shore sector.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->stone < 1000)
      {
         send_to_char("It costs 1k stone units to terraform a desert/shore sector.\n\r", ch);
         return;
      }
      if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
      {
         if (ch->pcdata->town->coins < 20000)
         {
            send_to_char("It will cost 20k coins to terraform a desert/shore sector outside your AOC.\n\r", ch);
            return;
         }
         ch->pcdata->town->coins -=20000;
      }
      else
         ch->pcdata->town->coins -= 8000;
      
      ch->pcdata->town->stone-= 1000;
      resource_sector[ch->map][ch->coord->x][ch->coord->y] = 60; // 2 Game Months
      send_to_char("The project is starting, it will take about 2 Game Months to clear out this desert/shore sector.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "mountains"))
   {    
      if (sector == SECT_MOUNTAIN || sector == SECT_NGOLD || sector == SECT_NIRON || sector == SECT_NSTONE)
      {
         if (sector != SECT_MOUNTAIN)
         {
            if (resource_sector[ch->map][ch->coord->x][ch->coord->y] > 0)
            {
               send_to_char("You can only terraform an empty mountain, extract all resources here first.\n\r", ch);
               return;
            }
         }
         if (ch->pcdata->town->coins < 40000)
         {
            send_to_char("It costs 40k coins to terraform a mountain.\n\r", ch);
            return;
         }
         if (ch->pcdata->town->stone < 5000)
         {
            send_to_char("It costs 5k units of stone to terraform a mountain.\n\r", ch);
            return;
         }
         ch->pcdata->town->coins-=40000;
         ch->pcdata->town->stone-=5000;
         map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_MOUNTAIN;
         resource_sector[ch->map][ch->coord->x][ch->coord->y] = 180; // 6 Game Months
         send_to_char("The project is starting, it will take about 6 Game Months to clear out this mountain sector.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("You can only terraform empty mountains (or resource mountains that are empty of resources).\n\r", ch);
         return;
      }
   }
   do_terraform(ch, "");
   return;
}
//removes reset items . mobs . trainers
void do_kremove(CHAR_DATA * ch, char * argument)
{
   CHAR_DATA *trainer;
   MOB_INDEX_DATA *pmob;
   CHAR_DATA *mob;
   char arg [MIL];
   char logb [MSL];
   int cnt;
   RESET_DATA *pReset;
   OBJ_DATA *obj;
   BUYKTRAINER_DATA *ktrainer;
   BTRAINER_DATA *btrainer;
   
   if (check_npc(ch))
         return;
         
   if (ch->pcdata->hometown == -1)
   {
      send_to_char("Your hometown cannot use this option, sorry.\n\r", ch);
      return;
   }   
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to do this.\n\r", ch);
      return;
   }
   if (kingdom_table[ch->pcdata->hometown]->minplace > ch->pcdata->caste)
   {
      ch_printf(ch, "It requires caste %s to do this (minplace).\n\r", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: kremove <mob/obj/trainer> <target>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   
   if (!str_cmp(arg, "mob") || !str_cmp(arg, "mobile"))
   {
      BUYKMOB_DATA *kmob;
      
      for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room)
      {
         if (nifty_is_name(argument, mob->name) && ch->coord->x == mob->coord->x && ch->coord->y == mob->coord->y && ch->map == mob->map)
         {
            for (kmob = first_buykmob; kmob; kmob = kmob->next)
            {
               if (xIS_SET(mob->act, ACT_REPAIR))
               {
                  if (xIS_SET(kmob->flags, KMOB_REPAIR))
                     break;
               }
               if (kmob->vnum == mob->pIndexData->vnum)
                  break;
            }
            if (!kmob)
            {
               send_to_char("That is not a kingdom mobile that your can purchase.\n\r", ch);
               return;
            }
            if (!IN_WILDERNESS(ch) || (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown))
            {
               send_to_char("You can only do this in a town owned by your kingdom.\n\r", ch);
               return;
            }
            if (xIS_SET(kmob->flags, KMOB_ADDRESET)) 
            {
               //now find the reset, delete it, then break
               for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
               {           
                  if (pReset->command == 'M' &&  pReset->arg1 == mob->pIndexData->vnum && pReset->arg3 == ch->in_room->vnum
                  && pReset->arg4 == mob->coord->x && pReset->arg5 == mob->coord->y && pReset->arg6 == mob->map)
                  {
                     delete_reset(ch->in_room->area, pReset);
                     break;
                  }
               }
            }
            else if (xIS_SET(kmob->flags, KMOB_FORGE))
            {
               wREMOVE_BIT(ch, ROOM_PRIVATE);
               wREMOVE_BIT(ch, ROOM_FORGEROOM);
               //now find the reset, delete it, then break
               for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
               {           
                  if (pReset->command == 'M' &&  pReset->arg1 == mob->pIndexData->vnum && pReset->arg3 == ch->in_room->vnum
                  && pReset->arg4 == ch->coord->x && pReset->arg5 == ch->coord->y && pReset->arg6 == ch->map)
                  {
                     delete_reset(ch->in_room->area, pReset);
                     break;
                  }
               }
            }
            else if (xIS_SET(kmob->flags, KMOB_MILITARY))
            {
               if (mob->m4 != ch->pcdata->hometown)
               {
                  send_to_char("You can only dismiss your own military.\n\r", ch);
                  return;
               }
               if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mingeneral)
               {
                  if (mob->m1 != ch->pcdata->town->tpid)
                  {
                     send_to_char("You can only dismiss military in your town.\n\r", ch);
                     return;
                  }
               }
               if (ch->coord->x != ch->pcdata->town->barracks[0] || ch->coord->y != ch->pcdata->town->barracks[1]
               ||  ch->map != ch->pcdata->town->barracks[2])
               {
                  send_to_char("You can only dismiss your military in the barracks.\n\r", ch);
                  return;
               }
            } 
            else if (xIS_SET(kmob->flags, KMOB_REPAIR))
            {
               REPAIR_DATA *repair;
               
               for (repair = first_repair; repair; repair = repair->next)
               {
                  if (mob->pIndexData->rShop == repair)
                     break;
               }
               if (!repair)
               {
                  send_to_char("That mobile appears not to be a Blacksmith that can repair equipment.\n\r", ch);
                  return;
               }
               mob->pIndexData->rShop = NULL;
               UNLINK(repair, first_repair, last_repair, next, prev);
               DISPOSE(repair);
               //now find the reset, delete it, then break
               for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
               {           
                  if (pReset->command == 'M' &&  pReset->arg1 == mob->pIndexData->vnum && pReset->arg3 == ch->in_room->vnum
                  && pReset->arg4 == ch->coord->x && pReset->arg5 == ch->coord->y && pReset->arg6 == ch->map)
                  {
                     delete_reset(ch->in_room->area, pReset);
                     break;
                  }
               }
            }               
            else
            {
               send_to_char("You cannot use kremove to remove that mobile, it only works on reset mobs, blacksmiths, and military mobiles.\n\r", ch);
               return;
            }
            sprintf(logb, "Kremove: %s removed mob %s", PERS_KINGDOM(ch, ch->pcdata->hometown), mob->short_descr);
            pmob = mob->pIndexData;
            extract_char(mob, TRUE);
            if (xIS_SET(kmob->flags, KMOB_REPAIR))
            {
               delete_mob(pmob);
               fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
            }
            send_to_char("Done.\n\r", ch);
            fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
            write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_KREMOVE_MOB);
            update_kremove_resources(ch, NULL, kmob);  
            return;
         }
      }
      send_to_char("There is no such mobile here.\n\r", ch);
      return;
   }            
   
   if (!str_cmp(arg, "obj") || !str_cmp(arg, "object"))
   {
      BUYKOBJ_DATA *kobj;
      ROOM_INDEX_DATA *room;
      cnt = 0;
            
      for (obj = ch->in_room->first_content; obj; obj = obj->next_content)  
      {
         if (nifty_is_name(argument, obj->name) && ch->coord->x == obj->coord->x && ch->coord->y == obj->coord->y && ch->map == obj->map)
         {
            for (kobj = first_buykobj; kobj; kobj = kobj->next) 
            {
               if (kobj->vnum == obj->pIndexData->vnum)
                  break;
            }
            if (!kobj)
            {
               send_to_char("That is not a kingdom object that your can purchase.\n\r", ch);
               return;
            }
            if (!IN_WILDERNESS(ch))
            {
               send_to_char("You can only do this in a kingdom town.\n\r", ch);
               return;
            }
            if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown
                && !xIS_SET(kobj->flags, KOBJ_BIN))
            {
               send_to_char("You can only do this in a town owned by your kingdom.\n\r", ch);
               return;
            }
            if (xIS_SET(kobj->flags, KOBJ_ADDRESET)) 
            {
               //now find the reset, delete it, then break
               for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
               {           
                  if (pReset->command == 'O' &&  pReset->arg1 == obj->pIndexData->vnum && pReset->arg3 == ch->in_room->vnum
                  && pReset->arg4 == ch->coord->x && pReset->arg5 == ch->coord->y && pReset->arg6 == ch->map)
                  {
                     delete_reset(ch->in_room->area, pReset);
                     break;
                  }
               }
               //redo the count, don't want to load too many fountains, etc
               for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
               {
                   if (pReset->command == 'O' &&  pReset->arg1 == obj->pIndexData->vnum)
                   {
                      room = get_room_index(pReset->arg3);
                      if (!room)
                         pReset->arg2 = 1;
                      else
                         pReset->arg2 = count_obj_list(obj->pIndexData, room->first_content)-1; //Obj has yet to be removed...
                   }
               }
            }
            else if (xIS_SET(kobj->flags, KOBJ_CONTAINER))
            {
               KCHEST_DATA *kchest;
               
               for (kchest = first_kchest; kchest; kchest = kchest->next)
               {
                  if (kchest->obj->pIndexData->vnum == obj->pIndexData->vnum && kchest->obj->in_room->vnum == obj->in_room->vnum
                  &&  kchest->obj->coord->x == obj->coord->x && kchest->obj->coord->y == obj->coord->y
                  &&  kchest->obj->map == obj->map)
                  {
                     if (obj->first_content)
                     {
                        send_to_char("This Chest is not empty, please empty it then kremove it.\n\r", ch);
                        return;
                     }
                     break;
                  }
               }
               if (!kchest)
               {
                  send_to_char("That chest seems to be not connected to the chest list, tell an immortal.\n\r", ch);
                  return;
               }
               UNLINK(kchest, first_kchest, last_kchest, next, prev);
               DISPOSE(kchest);
            }
            else if (xIS_SET(kobj->flags, KOBJ_BIN))
            {  
               AREA_DATA *karea;                            
               BIN_DATA *blist;
               
               save_bin_data();
               for (blist = first_bin; blist; blist = blist->next)
               {
                  if (blist->serial == obj->serial)
                     break;
               }
               if (!blist)
               {
                  send_to_char("That bin seems to be not connected to the bin list, tell an immortal.\n\r", ch);
                  return;
               }
               for (karea = first_area; karea; karea = karea->next)
               {
                  if (karea->x > 0 && karea->y > 0 && karea->map > -1 && karea->kingdom == ch->pcdata->hometown 
                  && abs(karea->x - blist->x) < 5 && abs(karea->y - blist->y) < 5 &&  blist->map == karea->map)
                     break;
               }
               if (!karea)
               {
                  send_to_char("The bin has to be within 5 squares of a town owned by your kingdom to kremove it.\n\r", ch);
                  return;
               }
               UNLINK(blist, first_bin, last_bin, next, prev);
               DISPOSE(blist);
            }
            else
            {
               send_to_char("You cannot use kremove to remove that object, it only works on reset objects, chests, and bins.\n\r", ch);
               return;
            }
            sprintf(logb, "Kremove: %s removed obj %s", PERS_KINGDOM(ch, ch->pcdata->hometown), obj->short_descr);
            separate_obj(obj);
            extract_obj(obj);
            send_to_char("Done.\n\r", ch);
            write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_KREMOVE_OBJ);
            update_kremove_resources(ch, kobj, NULL);
            fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
            if (xIS_SET(kobj->flags, KOBJ_CONTAINER))
               save_kingdom_chests(ch);            
            if (xIS_SET(kobj->flags, KOBJ_BIN))   
               save_bin_data();   
            return;
         }
      }
      send_to_char("There is no such object here.\n\r", ch);
      return;
   }            
   if (!str_cmp(arg, "trainer"))
   {
      if (!IN_WILDERNESS(ch) || (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown))
      {
         send_to_char("You can only do this in a town owned by your kingdom.\n\r", ch);
         return;
      }
      for (trainer = ch->in_room->first_person; trainer; trainer = trainer->next_in_room)
      {
         if (xIS_SET(trainer->act, ACT_TRAINER) && nifty_is_name(argument, trainer->name))
         {
            for (btrainer = first_boughttrainer; btrainer; btrainer = btrainer->next)
            {
               if (btrainer->rvnum == ch->in_room->vnum && btrainer->x == ch->coord->x &&
                   btrainer->y == ch->coord->y && btrainer->pid == trainer->m2) //found the trainer
               {
                  for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
                  {
                     if (ktrainer->pid == btrainer->pid)
                        break;
                  }
                  if (!ktrainer)
                  {
                      send_to_char("There was an error, tell an immortal.\n\r", ch);
                      bug("Pid %d on trainer in room %d does not exist in ktrainer list.", btrainer->pid, btrainer->rvnum);
                      return;
                  }
                  sprintf(logb, "Kremove: %s removed trainer %s", PERS_KINGDOM(ch, ch->pcdata->hometown), trainer->short_descr);
                  ch->pcdata->town->coins += ktrainer->cost /2;
                  extract_char(trainer, TRUE);
                  UNLINK(btrainer, first_boughttrainer, last_boughttrainer, next, prev);
                  DISPOSE(btrainer);
                  save_buykingdom_data();
                  write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_KREMOVE_TRAINER);
                  ch_printf(ch, "The trainer was removed and %d was added to your hometown.\n\r", ktrainer->cost);
                  return;
               }
            }
         }
      }
      send_to_char("There is no such trainer here.\n\r", ch);
      return;
   }
   do_kremove(ch, "");
   return;
}
   
//Used to place a trainer in the Trainer Que, also gets the trainerque list.
void do_placetrainer(CHAR_DATA * ch, char *argument)
{
   MOB_INDEX_DATA *mob;
   int x;
   char buf[MSL];
   CHAR_DATA *victim;
   CHAR_DATA *trainer;
   int cnt;
   BUYKTRAINER_DATA *ktrainer;
   BTRAINER_DATA *btrainer;
   char name[50];
   
   if (check_npc(ch))
         return;

   if (ch->pcdata->hometown == -1)
   {
      send_to_char("Your hometown cannot use this option, sorry.\n\r", ch);
      return;
   }
   
   if (!ch->pcdata->town)
   {
      send_to_char("You can only use this command if you have a town.\n\r", ch);
      return;
   }
   sprintf(buf, "%s", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
   if (argument[0] == '\0')
   {
      ch_printf(ch, "Name of Trainer         Cost    Skill/Spell                Mastery   Skill/Spell                Mastery   \n\r");
      for (x = 0; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->trainer_que[x] > 0)
         {
            cnt = 0;
            for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
            {
               cnt++;
               if (cnt == kingdom_table[ch->pcdata->hometown]->trainer_que[x])
               {
                  ch_printf(ch, "[%-3d]  %-15s  %-6d  %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                     x+1, ktrainer->name, ktrainer->cost, ktrainer->sn[0] > 0 ? skill_table[ktrainer->sn[0]]->name : "", ktrainer->mastery[0] > 0 ? get_mastery_name(ktrainer->mastery[0]) : "",  
                     ktrainer->sn[1] > 0 ? skill_table[ktrainer->sn[1]]->name : "", ktrainer->mastery[1] > 0 ? get_mastery_name(ktrainer->mastery[1]) : "", 
                     ktrainer->sn[2] > 0 ? skill_table[ktrainer->sn[2]]->name : "", ktrainer->mastery[2] > 0 ? get_mastery_name(ktrainer->mastery[2]) : "", 
                     ktrainer->sn[3] > 0 ? skill_table[ktrainer->sn[3]]->name : "", ktrainer->mastery[3] > 0 ? get_mastery_name(ktrainer->mastery[3]) : "", 
                     ktrainer->sn[4] > 0 ? skill_table[ktrainer->sn[4]]->name : "", ktrainer->mastery[4] > 0 ? get_mastery_name(ktrainer->mastery[4]) : ""); 
                  if (ktrainer->sn[5] > 0)
                  {
                     ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                        ktrainer->sn[5] > 0 ? skill_table[ktrainer->sn[5]]->name : "", ktrainer->mastery[5] > 0 ? get_mastery_name(ktrainer->mastery[5]) : "",  
                        ktrainer->sn[6] > 0 ? skill_table[ktrainer->sn[6]]->name : "", ktrainer->mastery[6] > 0 ? get_mastery_name(ktrainer->mastery[6]) : "", 
                        ktrainer->sn[7] > 0 ? skill_table[ktrainer->sn[7]]->name : "", ktrainer->mastery[7] > 0 ? get_mastery_name(ktrainer->mastery[7]) : "", 
                        ktrainer->sn[8] > 0 ? skill_table[ktrainer->sn[8]]->name : "", ktrainer->mastery[8] > 0 ? get_mastery_name(ktrainer->mastery[8]) : "", 
                        ktrainer->sn[9] > 0 ? skill_table[ktrainer->sn[9]]->name : "", ktrainer->mastery[9] > 0 ? get_mastery_name(ktrainer->mastery[9]) : "");    
                  }
                  if (ktrainer->sn[10] > 0)
                  {
                     ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                        ktrainer->sn[10] > 0 ? skill_table[ktrainer->sn[10]]->name : "", ktrainer->mastery[10] > 0 ? get_mastery_name(ktrainer->mastery[10]) : "",  
                        ktrainer->sn[11] > 0 ? skill_table[ktrainer->sn[11]]->name : "", ktrainer->mastery[11] > 0 ? get_mastery_name(ktrainer->mastery[11]) : "", 
                        ktrainer->sn[12] > 0 ? skill_table[ktrainer->sn[12]]->name : "", ktrainer->mastery[12] > 0 ? get_mastery_name(ktrainer->mastery[12]) : "", 
                        ktrainer->sn[13] > 0 ? skill_table[ktrainer->sn[13]]->name : "", ktrainer->mastery[13] > 0 ? get_mastery_name(ktrainer->mastery[13]) : "", 
                        ktrainer->sn[14] > 0 ? skill_table[ktrainer->sn[14]]->name : "", ktrainer->mastery[14] > 0 ? get_mastery_name(ktrainer->mastery[14]) : "");    
                  }
                  if (ktrainer->sn[15] > 0)
                  {
                     ch_printf(ch, "                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s  %-25s  %-8s\n\r                                %-25s  %-8s\n\r\n\r",
                        ktrainer->sn[15] > 0 ? skill_table[ktrainer->sn[15]]->name : "", ktrainer->mastery[15] > 0 ? get_mastery_name(ktrainer->mastery[15]) : "",  
                        ktrainer->sn[16] > 0 ? skill_table[ktrainer->sn[16]]->name : "", ktrainer->mastery[16] > 0 ? get_mastery_name(ktrainer->mastery[16]) : "", 
                        ktrainer->sn[17] > 0 ? skill_table[ktrainer->sn[17]]->name : "", ktrainer->mastery[17] > 0 ? get_mastery_name(ktrainer->mastery[17]) : "", 
                        ktrainer->sn[18] > 0 ? skill_table[ktrainer->sn[18]]->name : "", ktrainer->mastery[18] > 0 ? get_mastery_name(ktrainer->mastery[18]) : "", 
                        ktrainer->sn[19] > 0 ? skill_table[ktrainer->sn[19]]->name : "", ktrainer->mastery[19] > 0 ? get_mastery_name(ktrainer->mastery[19]) : "");    
                  }
                  break;
               }
            }
         }
      }
      return;
   }
   if (!str_cmp(argument, "clear"))
   {
      for (x = 0; x <= 24; x++)
      {
         kingdom_table[ch->pcdata->hometown]->trainer_que[x] = 0;
      }
      send_to_char("Cleared.\n\r", ch);
      return;
   }
   if (isdigit(argument[0]))
   {
      for (x = 0; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->trainer_que[x] > 0)
            continue;
         else
            break;
      }
      if (x == 0)
      {
         send_to_char("You have nothing in que to place, sorry.\n\r", ch);
         return;
      }
      if (atoi(argument) < 1 || atoi(argument) > x)
      {
         ch_printf(ch, "You choices are 1 through %d\n\r", x);
         return;
      }
      mob = get_mob_index(MOB_VNUM_TRAINER);
      if (!mob)
      {
         send_to_char("There is an error in the trainer que, tell an immortal.\n\r", ch);
         bug("placetrainer:  VNUM_TRAINER does not exist!!!");
         return;
      }
      cnt = 1;
      for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
      {
         if (kingdom_table[ch->pcdata->hometown]->trainer_que[atoi(argument)-1] == cnt)
            break;
         cnt++;
      }
      if (!ktrainer)
      {
         bug("do_placetrainer:  A trainer of position [%d] does not exist in the list.", kingdom_table[ch->pcdata->hometown]->trainer_que[atoi(argument)-1]);
         send_to_char("There was an error, tell an immortal.\n\r", ch);
         return;
      }
      if (!IN_WILDERNESS(ch))
      {
         send_to_char("You can only place trainers in your hometown.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->coins < ktrainer->cost)
      {
         ch_printf(ch, "The price for Option %d is %d and your town doesn't have that much.\n\r", atoi(argument), ktrainer->cost);
         return;
      }
      if (kingdom_table[ch->pcdata->hometown]->minplace > ch->pcdata->caste)
      {
         ch_printf(ch, "It requires caste %s to place this mob.\n\r", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
         return;
      }
      if (!IN_WILDERNESS(ch) || (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown))
      {
         send_to_char("You can only place trainers in your own town.\n\r", ch);
         return;
      }
      if (!is_made_room(ch->coord->x, ch->coord->y, ch->map, ch->pcdata->town))
      {
         send_to_char("You can only place trainers in your own town.\n\r", ch);
         return;
      }
      cnt = 0;
      for (trainer = ch->in_room->first_person; trainer; trainer = trainer->next_in_room)
      {
         if (IS_NPC(trainer) && xIS_SET(trainer->act, ACT_TRAINER) && IN_SAME_ROOM(ch, trainer) )
            cnt++;
      }
      if (cnt >= 2)
      {
         send_to_char("There can only be 2 trainers per room.\n\r", ch);
         return;
      }
      CREATE(btrainer, BTRAINER_DATA, 1);
      btrainer->rvnum = ch->in_room->vnum;
      btrainer->pid = ktrainer->pid;
      btrainer->x = ch->coord->x;
      btrainer->y = ch->coord->y;
      btrainer->map = ch->map;
      LINK(btrainer, first_boughttrainer, last_boughttrainer, next, prev);
      //Load mobile and place it Now!
      victim = create_mobile(get_mob_index(MOB_VNUM_TRAINER));
      char_to_room(victim, get_room_index(ch->in_room->vnum));
      victim->coord->x = ch->coord->x;
      victim->coord->y = ch->coord->y;
      victim->map = ch->map;
      victim->m2 = ktrainer->pid;
               
      STRFREE(victim->name);
      sprintf(name, ktrainer->name);
      victim->name = STRALLOC(name);
               
      STRFREE(victim->short_descr);
      sprintf(name, ktrainer->name);
      victim->short_descr = STRALLOC(name);
               
      STRFREE(victim->long_descr);
      sprintf(name, "%s is here training all that are interested.\n\r", ktrainer->name);
      victim->long_descr = STRALLOC(name);    

      fix_kingdom_tque(ch, atoi(argument) - 1);
      ch_printf(ch, "Option %d (mob %s) has been placed here.\n\r", atoi(argument), victim->short_descr);
      ch->pcdata->town->coins -= ktrainer->cost;
      sprintf(logb, "Placetrainer: %s bought %s", PERS_KINGDOM(ch, ch->pcdata->hometown), victim->short_descr);
      save_buykingdom_data();
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLACETRAINER);
      return;
   }
   else
   {
      send_to_char("Syntax: placetrainer - List of trainers in Que\n\rSyntax: placetrainer [number] - Place trainer (number is in que list)\n\r", ch);
      return;
   }
   return;
}

int get_maxunits(int size)
{
   switch(size)
   {
      case 1: case 2: case 3:
         return 1;
      case 4: case 5: case 6:
         return 2;
      case 7: case 8: case 9:
         return 3;
      case 10:
         return 5;
      default:
         return 1;
   }
   return 1;
}

// Used to place a mob in the Mob Que, also gets the mobque list.
void do_placemob(CHAR_DATA * ch, char *argument)
{
   MOB_INDEX_DATA *mob;
   char arg[MIL];
   int x;
   char buf[MSL];
   MOB_INDEX_DATA *pMobIndex;
   BUYKMOB_DATA *kmob;
   CHAR_DATA *victim = NULL;
   RESET_DATA *pReset;
   RESET_DATA *reset;
   int worker = -1;
   int cnt = 1;
   int vnum;

   if (ch->pcdata->hometown == -1)
   {
      send_to_char("Your hometown cannot use this option, sorry.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to use this command.\n\r", ch);
      return;
   }
   sprintf(buf, "%s", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
   if (argument[0] == '\0')
   {
      ch_printf(ch, "Mob in Que                              Resource      Amount    MinCaste\n\r");
      ch_printf(ch, "-----------------------------------------------------------------------------\n\r");
      for (x = 0; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->mob_que[x] > 0)
         {
            mob = get_mob_index(kingdom_table[ch->pcdata->hometown]->mob_que[x]);
            if (mob)
            for (kmob = first_buykmob; kmob; kmob = kmob->next)
            {
               if (kmob->vnum == mob->vnum)
               {
                  if (kmob->tree > 0) 
                     ch_printf(ch, "[%-2d]   %-30s   Lumber        %-7d   %s\n\r", x+1, mob->short_descr, kmob->tree, buf);
                  if (kmob->corn > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Corn          %-7d   %s\n\r", x+1, mob->short_descr, kmob->corn, buf);
                  if (kmob->grain > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Grain         %-7d   %s\n\r", x+1, mob->short_descr, kmob->grain, buf);
                  if (kmob->iron > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Iron          %-7d   %s\n\r", x+1, mob->short_descr, kmob->iron, buf);
                  if (kmob->gold > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Gold          %-7d   %s\n\r", x+1, mob->short_descr, kmob->gold, buf);
                  if (kmob->stone > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Stone         %-7d   %s\n\r", x+1, mob->short_descr, kmob->stone, buf);
                  if (kmob->coins > 0)
                     ch_printf(ch, "[%-2d]   %-30s   Gold Coins    %-7d   %s\n\r", x+1, mob->short_descr, kmob->coins, buf);
               }
            }
         }
      }
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "clear"))
   {
      for (x = 0; x <= 24; x++)
      {
         kingdom_table[ch->pcdata->hometown]->mob_que[x] = 0;
      }
      send_to_char("Cleared.\n\r", ch);
      return;
   }
   if (isdigit(arg[0]))
   {
      for (x = 0; x <= 24; x++)
      {
         if (kingdom_table[ch->pcdata->hometown]->mob_que[x] > 0)
            continue;
         else
            break;
      }
      if (x == 0)
      {
         send_to_char("You have nothing in que to place, sorry.\n\r", ch);
         return;
      }
      if (atoi(arg) < 1 || atoi(arg) > x)
      {
         ch_printf(ch, "You choices are 1 through %d\n\r", x);
         return;
      }
      mob = get_mob_index(kingdom_table[ch->pcdata->hometown]->mob_que[atoi(arg) - 1]);
      if (!mob)
      {
         send_to_char("There is an error in the mob que, tell an immortal.\n\r", ch);
         return;
      }
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (mob->vnum == kmob->vnum)
            break;
      }
      if (!kmob)
      {
         bug("do_placemob:  A mobin que [%d] does not exist in the list.", mob->vnum);
         send_to_char("There was an error, tell an immorta.\n\r", ch);
         return;
      }
      if ((ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0) && xIS_SET(kmob->flags, KMOB_WILDERNESS))
      {
         send_to_char("You can only place these mobs while on the Wilderness.\n\r", ch);
         return;
      }
      if (xIS_SET(kmob->flags, KMOB_ADDRESET) && (!IN_WILDERNESS(ch) || (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)))
      {
         send_to_char("You can only add this mob in your town.\n\r", ch);
         return;
      }
      if (!proper_resources_mobs(ch, kmob))
      {
         ch_printf(ch, "You do not have the resources to purchase option %d.\n\r", atoi(argument));
         return;
      }
      vnum = kingdom_table[ch->pcdata->hometown]->mob_que[atoi(arg) - 1];
      pMobIndex = get_mob_index(vnum);
      if (kingdom_table[ch->pcdata->hometown]->minplace > ch->pcdata->caste)
      {
         ch_printf(ch, "It requires caste %s to place this mob.\n\r", get_caste_name(kingdom_table[ch->pcdata->hometown]->minplace, ch->sex));
         return;
      }
      //m1 - town pid m2 - range m3 - cost m4 - kingdom m5 - invite range m6 - warn/attack dist m7 - x m8 - y
      //m9 - time  m10 - speed  m11 - Build time  m12 - Min Town Size to Build
      if (xIS_SET(kmob->flags, KMOB_MILITARY))
      {
         TRAINING_DATA *training;
         
         if (ch->pcdata->town->barracks[1] == 0)
         {
            send_to_char("You have not set your barracks location yet.\n\r", ch);
            return;
         }
         if (ch->pcdata->town->unitstraining == get_maxunits(ch->pcdata->town->size))
         {
            send_to_char("Your training queue is full, wait for one to finish training or remove one.\n\r", ch);
            return;
         }
         if (pMobIndex->m12 > ch->pcdata->town->size)
         {
            send_to_char("You cannot select that unit at your current size.\n\r", ch);
            return;
         }
         CREATE(training, TRAINING_DATA, 1);
         training->kmob = kmob;
         //New Units take 1 month longer to build unless 1 or 10 or "worker" units
         if (ch->pcdata->town->size >= pMobIndex->m12+2 || pMobIndex->m12 == 1 || pMobIndex->m12 == 10)
         {
            training->speed = pMobIndex->m11;
         }
         else
         {
            training->speed = pMobIndex->m11 + 2 - (ch->pcdata->town->size - pMobIndex->m12);
         }
         training->kingdom = ch->pcdata->hometown;
         training->town = ch->pcdata->town->tpid;
         training->stime = time(0);
         LINK(training, first_training, last_training, next, prev);
         fwrite_training_list();
         fix_kingdom_mque(ch, atoi(arg) - 1);
         ch_printf(ch, "Option %d (mob %s) has been ordered to start training.\n\r", atoi(arg), pMobIndex->short_descr);
         remove_resources_mobs(ch, kmob);
         bug("Placemob: %s of kingdom %s bought %s", ch->name, kingdom_table[ch->pcdata->hometown]->name, pMobIndex->short_descr);
         sprintf(logb, "Placemob: %s bought %s", ch->name, pMobIndex->short_descr);
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLACEMOB);
         ch->pcdata->town->unitstraining++;
         write_kingdom_file(ch->pcdata->hometown);         
         return;
      }
      else if (xIS_SET(kmob->flags, KMOB_FORGE))
      {
         
         if (!is_made_room(ch->coord->x, ch->coord->y, ch->map, ch->pcdata->town))
         {
            send_to_char("You can only place a forge in a room you have made with makeroom.\n\r", ch);
            return;
         }
         if (wIS_SET(ch, ROOM_FORGEROOM))
         {
            send_to_char("This room is already occupied by a Forge.\n\r", ch);
            return;
         }
         //All is good, add forge now
         wSET_BIT(ch, ROOM_FORGEROOM);
         victim = create_mobile(pMobIndex);
         char_to_room(victim, ch->in_room);
         fix_kingdom_mque(ch, atoi(arg) - 1);
         victim->coord->x = ch->coord->x;
         victim->coord->y = ch->coord->y;
         victim->map = ch->map;
         victim->m4 = ch->pcdata->hometown;         
         for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
         {
             if (pReset->command == 'M' &&  pReset->arg1 == victim->pIndexData->vnum)
             {
                cnt++;
             }
         }
         reset = add_reset(victim->in_room->area, 'M', 1, victim->pIndexData->vnum, cnt, victim->in_room->vnum, victim->coord->x, victim->coord->y, victim->map, -1, 0, 0);
         reset->serial = victim->serial;
         serial_list[reset->serial] = TRUE;
         fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
         ch_printf(ch, "Option %d (mob %s) has been placed here.\n\r", atoi(arg), victim->short_descr);
         remove_resources_mobs(ch, kmob);
      }       
      else if (xIS_SET(kmob->flags, KMOB_REPAIR))
      {
         MOB_INDEX_DATA *rmob;
         AREA_DATA *tarea;
         REPAIR_DATA *repair;
         int vnum;
         
         //need to create a whole new mobile so we can have a seperate instance of a shop....
         tarea = ch->in_room->area;
         for (vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++)
         {
            if (!get_mob_index(vnum))
               break;
         }
         if (vnum > tarea->hi_m_vnum)
         {
            send_to_char("Your area's mob list is full, tell an immortal.\n\r", ch);
            bug("The area of %s mob list is full.", tarea->name);
            return;
         }
         
         if (!is_made_room(ch->coord->x, ch->coord->y, ch->map, ch->pcdata->town))
         {
            send_to_char("You can only place a repair mob in a room you have made with makeroom.\n\r", ch);
            return;
         }
            
         rmob = make_mobile(vnum, pMobIndex->vnum, pMobIndex->player_name); //create an instance of the mob
         victim = create_mobile(rmob);
         char_to_room(victim, ch->in_room);
         fix_kingdom_mque(ch, atoi(arg) - 1);
         victim->coord->x = ch->coord->x;
         victim->coord->y = ch->coord->y;
         victim->map = ch->map;
         victim->m4 = ch->pcdata->hometown;
         xREMOVE_BIT(rmob->act, ITEM_PROTOTYPE);
         xREMOVE_BIT(victim->act, ITEM_PROTOTYPE);
         //create+assign the shop values now :-)
         if (victim->pIndexData->rShop)
         {
            send_to_char("There is a bug, please tell an immortal.\n\r", ch);
            bug("Mobile %d already has a repair shop.", victim->pIndexData->vnum);
            return;
         }

         CREATE(repair, REPAIR_DATA, 1);
         LINK(repair, first_repair, last_repair, next, prev);
         repair->keeper = victim->pIndexData->vnum;
         repair->profit_fix = 120;
         repair->shop_type = SHOP_FIX;
         repair->open_hour = 0;    
         repair->close_hour = 23;
         repair->fix_type[0] = ITEM_WEAPON;
         repair->fix_type[1] = ITEM_ARMOR;
         victim->pIndexData->rShop = repair;
         //add a reset;
         for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
         {
             if (pReset->command == 'M' &&  pReset->arg1 == victim->pIndexData->vnum)
             {
                cnt++;
             }
         }
         reset = add_reset(victim->in_room->area, 'M', 1, victim->pIndexData->vnum, cnt, victim->in_room->vnum, victim->coord->x, victim->coord->y, victim->map, -1, 0, 0);
         reset->serial = victim->serial;
         serial_list[reset->serial] = TRUE;
         fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
         ch_printf(ch, "Option %d (mob %s) has been placed here.\n\r", atoi(arg), victim->short_descr);
         remove_resources_mobs(ch, kmob);
      }       
      else
      {    
         if (xIS_SET(kmob->flags, KMOB_ADDRESET))
         {
            if (!is_made_room(ch->coord->x, ch->coord->y, ch->map, ch->pcdata->town))
            {
               send_to_char("You can only place this mob in a room you have made with makeroom.\n\r", ch);
               return;
            }
         }
         if (xIS_SET(kmob->flags, KMOB_WORKER))
         {
            OBJ_DATA *obj;
            OMAP_DATA *mobj;           
            TRAINING_DATA *training;
         
            if (ch->pcdata->town->unitstraining == get_maxunits(ch->pcdata->town->size))
            {
               send_to_char("Your training queue is full, wait for one to finish training or remove one.\n\r", ch);
               return;
            } 
         
            for (mobj = first_wilderobj; mobj; mobj = mobj->next)
            {
               obj = mobj->mapobj;

               if (obj->item_type == ITEM_HOLDRESOURCE && obj->coord->x == ch->coord->x && obj->coord->y == ch->coord->y && obj->map == ch->map)
               {
                  break;
               }
            }
            if (!mobj)
            {
               if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
               {
                  send_to_char("You can only start works in your town or at a bin\n\r", ch);
                  return;
               }
               worker = 1;
            }
            if (atoi(argument) < KRES_GOLD || atoi(argument) > KRES_FISH)
            {
               send_to_char("You need to specify a resource to extract (numerically).\n\r", ch);
               return;
            }
         
            CREATE(training, TRAINING_DATA, 1);
            training->kmob = kmob;
            //New Units take 1 month longer to build unless 1 or 10 or "worker" units
            if (ch->pcdata->town->size >= pMobIndex->m12+2 || pMobIndex->m12 == 1 || pMobIndex->m12 == 10)
            {
               training->speed = pMobIndex->m11;
            }
            else
            {
               training->speed = pMobIndex->m11 + 2 - (ch->pcdata->town->size - pMobIndex->m12);
            }
            training->kingdom = ch->pcdata->hometown;
            training->town = ch->pcdata->town->tpid;
            training->stime = time(0);
            if (worker == 0)
               training->bin = 1;
            training->x = ch->coord->x;
            training->y = ch->coord->y;
            training->map = ch->map;
            training->resource = atoi(argument);
            LINK(training, first_training, last_training, next, prev);
            fwrite_training_list();
            fix_kingdom_mque(ch, atoi(arg) - 1);
            ch_printf(ch, "Option %d (mob %s) has been ordered to start training.\n\r", atoi(arg), pMobIndex->short_descr);
            remove_resources_mobs(ch, kmob);
            bug("Placemob: %s of kingdom %s bought %s", ch->name, kingdom_table[ch->pcdata->hometown]->name, pMobIndex->short_descr);
            sprintf(logb, "Placemob: %s bought %s", ch->name, pMobIndex->short_descr);
            write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLACEMOB);
            ch->pcdata->town->unitstraining++;
            write_kingdom_file(ch->pcdata->hometown); 
            return;           
         }
         victim = create_mobile(pMobIndex);
         char_to_room(victim, ch->in_room);
         fix_kingdom_mque(ch, atoi(arg) - 1);
         victim->coord->x = ch->coord->x;
         victim->coord->y = ch->coord->y;
         victim->map = ch->map;
         victim->m4 = ch->pcdata->hometown;
         if (xIS_SET(kmob->flags, KMOB_ADDRESET))
         {            
            for (pReset = ch->in_room->area->first_reset; pReset; pReset = pReset->next)
            {
                if (pReset->command == 'M' &&  pReset->arg1 == victim->pIndexData->vnum)
                {
                   cnt++;
                }
            }
            reset = add_reset(victim->in_room->area, 'M', 1, victim->pIndexData->vnum, cnt, victim->in_room->vnum, victim->coord->x, victim->coord->y, victim->map, -1, 0, 0);
            reset->serial = victim->serial;
            serial_list[reset->serial] = TRUE;
            fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
         }   
         else
         {
            victim->m9 = time(0);
         }
         ch_printf(ch, "Option %d (mob %s) has been placed here.\n\r", atoi(arg), victim->short_descr);
         if (IN_WILDERNESS(victim))
            SET_ONMAP_FLAG(victim);
         remove_resources_mobs(ch, kmob);
      }
      sprintf(logb, "Placemob: %s bought %s", PERS_KINGDOM(ch, ch->pcdata->hometown), victim->short_descr);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLACEMOB);
      return;
   }
   else
   {
      send_to_char("Syntax: placemob - List of mobs in Que\n\rSyntax: placemob [number] - Place mob (number is in que list)\n\r", ch);
      return;
   }
   return;
}

TOWN_DATA *get_town_tpid(int kingdom, int pid)
{
   TOWN_DATA *town;
   int x;
   
   if (kingdom > 1)
   {
      for (town = kingdom_table[kingdom]->first_town; town; town = town->next)
      {
         if (town->tpid == pid)
            return town;
      }
   }
   else
   {
      for (x = 2; x < sysdata.max_kingdom; x++)
      {
         for (town = kingdom_table[kingdom]->first_town; town; town = town->next)
         {
            if (town->tpid == pid)
               return town;
         }   
      }
   }
   return NULL;
}

//6480 - 3 days GT   1.8 Hours RL time

char *get_remaining_time(int stime, int speed)
{
   int remainingtime;
   int hours;
   int days;
   int months;
   static char buf[MSL];
   
   //90 - hour 2160 - day 64,800 - month
   
   if (speed == 0)
      remainingtime = stime + (cvttime(TRAINING_TIME)/2) - time(0);
   else
      remainingtime = stime + (speed*cvttime(TRAINING_TIME)) - time(0);
     
   months = remainingtime / cvttime(64800);
   
   if (remainingtime > cvttime(64800))
      days = (remainingtime % cvttime(64800)) / cvttime(2160);
   else if (remainingtime > cvttime(2160))
      days = remainingtime / cvttime(2160);
   else
      days = 0;
   
   if (remainingtime > cvttime(2160))
      hours = (remainingtime % cvttime(2160)) / cvttime(90);
   else
      hours = remainingtime / cvttime(90);
   
   sprintf(buf, "%2dm %2dd %2dh", months, days, hours);
   return buf;
}
     
void do_training (CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   TRAINING_DATA *training;
   MOB_INDEX_DATA *mob;
   TOWN_DATA *town;
   int cnt = 0;
   
   if (check_npc(ch))
      return;
    
   if (!IN_PLAYER_KINGDOM(ch->pcdata->hometown))
   {
      send_to_char("You need to belong to a kingdom to use this command.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You need to belong to a town to use this command.\n\r", ch);
      return;
   }    
   if (get_trust(ch) < LEVEL_STAFF)
   {
      if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mintoperate)
      {
         send_to_char("You can only use this command if you have proper permission.\n\r", ch);
         return;
      }
      if (argument[0] == '\0')
      {
         send_to_char("Syntax:  training list [town]\n\r", ch);
         send_to_char("Syntax:  training remove <number>\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg1);
      if (!str_cmp(arg1, "remove"))
      {
         if (atoi(argument) < 1)
         {
            send_to_char("The value for remove is the number provied in training list.\n\r", ch);
            return;
         }
         for (training = first_training; training; training = training->next)
         {
            if (training->kingdom == ch->pcdata->hometown)
            {
               cnt++;   
               if (cnt == atoi(argument))
               {
                  mob = get_mob_index(training->kmob->vnum);
                  if (!mob)
                  {
                     bug("do_training:  A mob in the training queue does not have a mobindex or currect town pid, tpid %d", training->town);
                     send_to_char("There is an error with that trainee, tell an immortal.\n\r", ch);
                     continue;
                  }    
                  ch->pcdata->town->lumber += training->kmob->tree/2;
                  ch->pcdata->town->corn += training->kmob->corn/2;
                  ch->pcdata->town->grain += training->kmob->grain/2;
                  ch->pcdata->town->iron += training->kmob->iron/2;
                  ch->pcdata->town->gold += training->kmob->gold/2;
                  ch->pcdata->town->stone += training->kmob->stone/2;
                  ch->pcdata->town->coins += training->kmob->coins/2;      
                  ch->pcdata->town->unitstraining--;
                  UNLINK(training, first_training, last_training, next, prev);
                  DISPOSE(training);
                  fwrite_training_list();
                  ch_printf(ch, "%d was removed, half of the cost placed back in your town\n\r", cnt);
                  sprintf(buf, "Training: %s cancelled the training of %s", ch->name, mob->short_descr);
                  write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_PLACEMOB);
                  write_kingdom_file(ch->pcdata->hometown);
                  return;
               }
            }
         }
         send_to_char("That is not a valid number, type training list to get a value.\n\r", ch);
         return;
      }                  
      if (!str_cmp(arg1, "list"))
      {
         ch_printf(ch, "&w&WNum  Name               Time Remaining   Town\n\r");
         ch_printf(ch, "&c&w-------------------------------------------------------------------------&w&C\n\r");
         if (argument[0] == '\0')
         {
            for (training = first_training; training; training = training->next)
            {
               if (training->kingdom == ch->pcdata->hometown)
               {
                  cnt++;
                  mob = get_mob_index(training->kmob->vnum);
                  town = get_town_tpid(training->kingdom, training->town);
                  if (!mob || !town)
                  {
                     bug("do_training:  A mob in the training queue does not have a mobindex or currect town pid, tpid %d", training->town);
                     continue;
                  }                  
                  ch_printf(ch, "%3d  %-17s  %-11s      %s\n\r", cnt, mob->short_descr, get_remaining_time(training->stime, training->speed), 
                                                               town->name);
               }
            }
         }
         else
         {
            for (training = first_training; training; training = training->next)
            {
               if (training->kingdom == ch->pcdata->hometown)
               {
                  cnt++;
                  town = get_town_tpid(training->kingdom, training->town);
                  if (!town)
                     continue;
                  if (!str_cmp(town->name, argument))
                  {
                     mob = get_mob_index(training->kmob->vnum);
                     if (!mob || !town)
                     {
                        bug("do_training:  A mob in the training queue does not have a mobindex");
                        continue;
                     }                  
                     ch_printf(ch, "%3d  %-17s  %-11s      %s\n\r", cnt, mob->short_descr, get_remaining_time(training->stime, training->speed), 
                                                               town->name);      
                  }
               }
            }
         }
      }
   }
   else
   {
      if (argument[0] == '\0')
      {
         send_to_char("Syntax:  training list [kingdom/town]\n\r", ch);
         send_to_char("Syntax:  training remove <number>\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg1);
      if (!str_cmp(arg1, "remove"))
      {
         if (atoi(argument) < 1)
         {
            send_to_char("The value for remove is the number provied in training list.\n\r", ch);
            return;
         }
         for (training = first_training; training; training = training->next)
         {
            cnt++;   
            if (cnt == atoi(argument))
            {
               mob = get_mob_index(training->kmob->vnum);
               town = get_town_tpid(training->kingdom, training->town);
               if (!mob || !town)
               {
                  bug("do_training:  A mob in the training queue does not have a mobindex or currect town pid, tpid %d", training->town);
                  send_to_char("There is an error with that trainee, tell an immortal.\n\r", ch);
                  continue;
               }    
               town->lumber += training->kmob->tree/2;
               town->corn += training->kmob->corn/2;
               town->grain += training->kmob->grain/2;
               town->iron += training->kmob->iron/2;
               town->gold += training->kmob->gold/2;
               town->stone += training->kmob->stone/2;
               town->coins += training->kmob->coins/2;       
               town->unitstraining--; 
               write_kingdom_file(training->kingdom);             
               ch_printf(ch, "%d was removed, half of the cost placed back to the town.\n\r", cnt);
               sprintf(buf, "Training: %s cancelled the training of %s", ch->name, mob->short_descr);
               write_kingdom_logfile(training->kingdom, buf, KLOG_PLACEMOB);
               UNLINK(training, first_training, last_training, next, prev);
               DISPOSE(training);
               fwrite_training_list();
               return;
            }
         }
         send_to_char("That is not a valid number, type training list to get a value.\n\r", ch);
         return;
      }                  
      if (!str_cmp(arg1, "list"))
      {
         ch_printf(ch, "&w&WNum  Name               Time Remaining   Town             Kingdom\n\r");
         ch_printf(ch, "&c&w-----------------------------------------------------------------------------&w&C\n\r");
         if (argument[0] == '\0')
         {
            for (training = first_training; training; training = training->next)
            {
               cnt++;
               mob = get_mob_index(training->kmob->vnum);
               town = get_town_tpid(training->kingdom, training->town);
               if (!mob || !town || !IN_PLAYER_KINGDOM(training->kingdom))
               {
                  bug("do_training:  A mob in the training queue does not have a mobindex or currect town pid, tpid %d", training->town);
                  continue;
               }                  
               ch_printf(ch, "%3d  %-17s  %-11s      %-15s  %s\n\r", cnt, mob->short_descr, get_remaining_time(training->stime, training->speed), 
                                                               town->name, kingdom_table[training->kingdom]->name);
            }
         }
         else
         {
            for (training = first_training; training; training = training->next)
            {
               cnt++;
               town = get_town_tpid(training->kingdom, training->town);
               if (!town)
                  continue;
               if (!IN_PLAYER_KINGDOM(training->kingdom))
                  continue;
               if (!str_cmp(town->name, argument) || !str_cmp(kingdom_table[training->kingdom]->name, argument))
               {
                  mob = get_mob_index(training->kmob->vnum);
                  if (!mob || !town)
                  {
                     bug("do_training:  A mob in the training queue does not have a mobindex");
                     continue;
                  }                  
                  ch_printf(ch, "%3d  %-17s  %-11s      %-15s  %s\n\r", cnt, mob->short_descr, get_remaining_time(training->stime, training->speed), 
                                                               town->name, kingdom_table[training->kingdom]->name);      
               }
            }
         }
      }
   }
}   
   
//Change the plains into a nice grain field
void plantgrain(CHAR_DATA * ch, char *argument)
{
   int sector;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can plant grain.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only plant grain if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector != SECT_PLAINS)
   {
      send_to_char("You cannot do that on this sectortype, see the helpfile for more info.\n\r", ch);
      return;
   }
   obj = get_obj_carry(ch, argument); /* does char have the item ? */

   if (obj == NULL)
   {
      send_to_char("You aren't carrying that.\n\r", ch);
      return;
   }
   if (obj->pIndexData->vnum != GRAIN_O_VNUM)
   {
      send_to_char("You need some grain seed to use this command.\n\r", ch);
      return;
   }
   separate_obj(obj);
   obj_from_char(obj);
   extract_obj(obj);
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_HGRAIN;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
   send_to_char("You plant some Grain Seed and hope for a good harvest.\n\r", ch);
   sprintf(logb, "%s planted some Grain", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLANTGRAIN);
   return;
}

//Change the plains into a corn field
void plantcorn(CHAR_DATA * ch, char *argument)
{
   int sector;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can plant corn.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only plant corn if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector != SECT_PLAINS)
   {
      send_to_char("You cannot do that on this sectortype, see the helpfile for more info.\n\r", ch);
      return;
   }
   obj = get_obj_carry(ch, argument); /* does char have the item ? */

   if (obj == NULL)
   {
      send_to_char("You aren't carrying that.\n\r", ch);
      return;
   }
   if (obj->pIndexData->vnum != CORN_O_VNUM)
   {
      send_to_char("You need some corn seed to use this command.\n\r", ch);
      return;
   }
   separate_obj(obj);
   obj_from_char(obj);
   extract_obj(obj);
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_HCORN;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
   send_to_char("You plant some Corn Seed and hope for a good harvest.\n\r", ch);
   sprintf(logb, "%s planted some corn", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLANTCORN);
   return;
}

//Change the plains into a nice Field of grass :-)
void plantgrass(CHAR_DATA * ch, char *argument)
{
   int sector;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can plant grass.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only plant grass if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector != SECT_PLAINS)
   {
      send_to_char("You cannot do that on this sectortype, see the helpfile for more info.\n\r", ch);
      return;
   }
   obj = get_obj_carry(ch, argument); /* does char have the item ? */

   if (obj == NULL)
   {
      send_to_char("You aren't carrying that.\n\r", ch);
      return;
   }
   if (obj->pIndexData->vnum != GRASS_O_VNUM)
   {
      send_to_char("You need some grass seed to use this command.\n\r", ch);
      return;
   }
   separate_obj(obj);
   obj_from_char(obj);
   extract_obj(obj);
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_FIELD;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
   send_to_char("You plant some Grass Seed and watch as it grows.\n\r", ch);
   sprintf(logb, "%s planted some grass", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLANTGRASS);
   return;
}

//Plants a tree, wow, trees good for wood they say :-)
void planttree(CHAR_DATA * ch, char *argument)
{
   int sector;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can plant trees.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only plant trees if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector != SECT_PLAINS && sector != SECT_FIELD)
   {
      send_to_char("You cannot do that on this sectortype, see the helpfile for more info.\n\r", ch);
      return;
   }
   obj = get_obj_carry(ch, argument); /* does char have the item ? */

   if (obj == NULL)
   {
      send_to_char("You aren't carrying that.\n\r", ch);
      return;
   }
   if (obj->pIndexData->vnum != TREE_O_VNUM)
   {
      send_to_char("You need a Tree Sprout to use this command.\n\r", ch);
      return;
   }
   separate_obj(obj);
   obj_from_char(obj);
   extract_obj(obj);
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_FOREST;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 100;
   send_to_char("A newly grown Tree Sprout is planted where you stand.\n\r", ch);
   sprintf(logb, "%s planted a tree", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLANTTREE);
   return;
}


//Used to stop fires out in the Wilderness, requires a water item on them (selected through the argument) 
void stopfire(CHAR_DATA * ch, char *argument)
{
   int sector;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can stop fires.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only stop fires if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector != SECT_FIRE)
   {
      send_to_char("This only works if there is a fire at this coordinate.\n\r", ch);
      return;
   }
   obj = get_obj_carry(ch, argument); /* does char have the item ? */

   if (obj == NULL)
   {
      send_to_char("You aren't carrying that.\n\r", ch);
      return;
   }
   if (obj->item_type != ITEM_DRINK_CON)
   {
      send_to_char("You have to use something that contains water silly.\n\r", ch);
      return;
   }
   if (obj->value[1] < 200)
   {
      send_to_char("It requires a lot of water to put a fire out, get a larger container or fill it up more.\n\r", ch);
      return;
   }
   obj->value[1] -= 200;
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_BURNT;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
   send_to_char("With a lot of water, you get lucky and put the fire out.\n\r", ch);
   sprintf(logb, "%s stopped a bit of fire", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_STOPFIRE);
   return;
}

//Used to change a sector to plains
//Changed in 2.1 to level just about anything that is not moveable, water, or
//not flat....
void change_plains(CHAR_DATA * ch, char *argument)
{
   int sector;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can use this command.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only use this command if you are in the Wilderness.\n\r", ch);
      return;
   }
   
   //seems like a lot but that leaves about 30 sectors, laugh
   if (sector == SECT_HILLS || sector == SECT_MOUNTAIN
   ||  sector == SECT_UNDERWATER || sector == SECT_AIR || sector == SECT_DESERT || sector == SECT_DUNNO
   ||  sector == SECT_OCEANFLOOR || sector == SECT_UNDERGROUND || sector == SECT_ENTER
   ||  sector == SECT_MINEGOLD || sector == SECT_MINEIRON || sector == SECT_RIVER || sector == SECT_ICE
   ||  sector == SECT_SHORE || sector == SECT_OCEAN || sector == SECT_LAVA || sector == SECT_TREE
   ||  sector == SECT_QUICKSAND || sector == SECT_SGOLD || sector == SECT_NGOLD || sector == SECT_SIRON
   ||  sector == SECT_NIRON || sector == SECT_WALL || sector == SECT_GLACIER || sector == SECT_EXIT
   ||  sector == SECT_BRIDGE || sector == SECT_VOID || sector == SECT_STABLE || sector == SECT_FIRE
   ||  sector == SECT_DWALL || sector == SECT_NBWALL || sector == SECT_DOOR || sector == SECT_CDOOR
   ||  sector == SECT_LDOOR || sector == SECT_INSIDE || sector == SECT_HOLD
   ||  sector == SECT_STONE || sector == SECT_SSTONE || sector == SECT_NSTONE)      
   {
      send_to_char("You cannot use this command on this sector, see help plains for more info.\n\r", ch);
      return;
   }
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_PLAINS;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
   send_to_char("Alright, this current sector is changed to Plains.\n\r", ch);
   sprintf(logb, "%s cut some land into plains", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_PLAINS);
   return;
}

//Survey the mountains for possible Resources
void do_survey(CHAR_DATA * ch, char *argument)
{
   int chance;
   int sector;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobs.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to use this command.\n\r", ch);
      return;
   }
   if (ch->pcdata->job != 5)
   {
      send_to_char("Need to be a Surveyor to use the survey command.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: survey <option>\n\rOption - room or resources\n\r", ch);
      return;
   }
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only use this command if you are in the Wilderness.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!str_cmp(argument, "room"))
   {
      if (sector != SECT_MOUNTAIN)
         ch_printf(ch, "%s has %d Units left", sect_show[sector].desc, resource_sector[ch->map][ch->coord->x][ch->coord->y]);
      else
      {
         if (resource_sector[ch->map][ch->coord->x][ch->coord->y] == -1)
            send_to_char("This section of mountain range has already been surveyed and is empty.\n\r", ch);
         else
            send_to_char("This section of mountain range has yet to be surveyed.\n\r", ch);
      }
   }
   else if (!str_cmp(argument, "resources"))
   {
      if (map_sector[ch->map][ch->coord->x][ch->coord->y] != SECT_MOUNTAIN)
      {
         send_to_char("You can only use this command at a Mountain Sector.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->coins < 3000)
      {
         send_to_char("It costs 3000 coins to survey a location, your town does not have that.\n\r", ch);
         return;
      }
      sprintf(logb, "%s is surveying the mountains", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_SURVEY);
      chance = number_range(1, 1000);

      if (resource_sector[ch->map][ch->coord->x][ch->coord->y] == -1)
      {
         send_to_char("This section of mountain range is empty of resources.\n\r", ch);
         return;
      }
      ch->pcdata->town->coins -= 3000;
      //1.3 percent of a chance for gold, gold is rare
      if (chance <= 13)
      {
         send_to_char("You strike it lucky as you find Gold at this location.\n\r", ch);
         map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_MINEGOLD;
         resource_sector[ch->map][ch->coord->x][ch->coord->y] = 3000;
         sprintf(logb, "%s strikes GOLD!!!", PERS_KINGDOM(ch, ch->pcdata->hometown));
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_SURVEY_STRIKES);
         return;
      }
      // 4.5 percent for iron
      if (chance <= 45)
      {
         send_to_char("You have discovered a good deal of Iron at this location.\n\r", ch);
         map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_MINEIRON;
         resource_sector[ch->map][ch->coord->x][ch->coord->y] = 3000;
         sprintf(logb, "%s strikes IRON!", PERS_KINGDOM(ch, ch->pcdata->hometown));
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_SURVEY_STRIKES);
         return;
      }
      // 33 percent chance for stone
      if (chance <= 330) 
      {
         send_to_char("You have discovered a good deal of Stone at this location.\n\r", ch);
         map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_STONE;
         resource_sector[ch->map][ch->coord->x][ch->coord->y] = 6000;
         sprintf(logb, "%s strikes STONE!", PERS_KINGDOM(ch, ch->pcdata->hometown));
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_SURVEY_STRIKES);
      }
      if (chance > 330)
      {
         send_to_char("This section of mountain range is empty of resources.\n\r", ch);
         resource_sector[ch->map][ch->coord->x][ch->coord->y] = -1;
      }
   }
   else
   {
      do_survey(ch, "");
   }
   return;
}
void do_leavekingdom(CHAR_DATA * ch, char *argument)
{
   int ht;
   int kdm;
   char buf[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("This command is for players only.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste == caste_King)
   {
      send_to_char("You are the king, you cannot leave the Kingdom.\n\r", ch);
      return;
   }
   ht = ch->pcdata->hometown;
   if (!str_cmp(kingdom_table[ht]->name, "Rafermand") || !str_cmp(kingdom_table[ht]->name, "Niemria"))
   {
      send_to_char("You cannot leave those kingdoms.\n\r", ch);
      return;
   }
   else
   {
      int p;

      for (p = 0; p < sysdata.last_portal; p++)
      {
         for (kdm = 0; kdm < sysdata.max_kingdom; kdm++)
         {
            if (kdm > 1)
            {
               sprintf(buf, "%s Portal", kingdom_table[kdm]->name);
               if (!str_cmp(portal_show[p]->desc, buf))
                  xREMOVE_BIT(ch->pcdata->portalfnd, p);
            }
         }
      }
      sprintf(logb, "%s just left your kingdom", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_LEAVEKINGDOM);
      if (kingdom_table[ch->pcdata->hometown]->number1 && !str_cmp(ch->name, kingdom_table[ch->pcdata->hometown]->number1))
         STRFREE(kingdom_table[ch->pcdata->hometown]->number1);
      if (kingdom_table[ch->pcdata->hometown]->number2 && !str_cmp(ch->name, kingdom_table[ch->pcdata->hometown]->number2))
         STRFREE(kingdom_table[ch->pcdata->hometown]->number2);
      ch->pcdata->caste = caste_Peasant;
      ch->pcdata->hometown = 1;
      ch->pcdata->resourcetype = 0;
      ch->pcdata->resource = 0;
      ch->pcdata->job = 1;
      ch->pcdata->kingdompid = kingdom_table[ch->pcdata->hometown]->kpid;
      ch->pcdata->town = NULL;
      remove_player_list(ch, 0);
      send_to_char("Sent back to the default Kingdom, be careful.\n\r", ch);
      return;
   }
}

TOWN_DATA *get_town(char *townname)
{
   int x;
   TOWN_DATA *town;
   
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      for (town = kingdom_table[x]->first_town; town; town = town->next)
      {
         if (!str_cmp(town->name, townname))
            return town;
      }
   }
   return NULL;
}

void do_tkickout(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TOWN_DATA *town;
   int ht;

   if (IS_NPC(ch))
   {
      send_to_char("This command is for players only.\n\r", ch);
      return;
   }
   ht = ch->pcdata->hometown;
   
   if (!ch->pcdata->town)
   {
      send_to_char("How do you plan on kicking someone out of a town you are not in?\n\r", ch);
      return;
   }

   if (ch->pcdata->caste < ch->pcdata->town->minhappoint)
   {
      send_to_char("Your caste is not high enough to use this command.\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, argument)) == NULL)
   {
      send_to_char("That person is not online right now.\n\r", ch);
      return;
   }
   if (ch == victim)
   {
      send_to_char("You cannot kick yourself out, sorry.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You can only kick out players, not NPCs.\n\r", ch);
      return;
   }
   if (ch->pcdata->town == victim->pcdata->town)
   {
      send_to_char("You can only use this to boot people out of your town.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < victim->pcdata->caste)
   {
      send_to_char("Your target has a higher caste than you, would not do that.\n\r", ch);
      return;
   }
   town = get_town(kingdom_table[ht]->dtown);
   if (town)
   {
      sprintf(logb, "%s just kicked %s out of the town of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), PERS_KINGDOM(victim, victim->pcdata->hometown), ch->pcdata->town->name);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_TKICKOUT);
      victim->pcdata->town = town;
      ch_printf(ch, "You kicked %s out of your town.\n\r", PERS_MAP(victim, ch));
      ch_printf(victim, "%s kicked you out of the town, sorry.\n\r", PERS_MAP(ch, victim));
      return;
   }
   else
   {
      send_to_char("Your kingdom does not have a default town, please tell the king.\n\r", ch);
      return;
   }
}

void do_kickout(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   int ht;

   if (IS_NPC(ch))
   {
      send_to_char("This command is for players only.\n\r", ch);
      return;
   }
   ht = ch->pcdata->hometown;

   if (kingdom_table[ht]->minhappoint > ch->pcdata->caste)
   {
      send_to_char("Your caste is not high enough to use this command.\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, argument)) == NULL)
   {
      send_to_char("That person is not online right now.\n\r", ch);
      return;
   }
   if (ch == victim)
   {
      send_to_char("You cannot kick yourself out, sorry.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("You can only kick out players, not NPCs.\n\r", ch);
      return;
   }
   if (ch->pcdata->hometown != victim->pcdata->hometown)
   {
      send_to_char("You can only kickout players in your own kingdom.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < victim->pcdata->caste)
   {
      send_to_char("Your target has a higher caste than you, would not do that.\n\r", ch);
      return;
   }
   sprintf(logb, "%s just kicked %s out of your kingdom", PERS_KINGDOM(ch, ch->pcdata->hometown), PERS_KINGDOM(victim, victim->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_KICKOUT);
   if (kingdom_table[victim->pcdata->hometown]->number1 && !str_cmp(victim->name, kingdom_table[victim->pcdata->hometown]->number1))
      STRFREE(kingdom_table[victim->pcdata->hometown]->number1);
   if (kingdom_table[victim->pcdata->hometown]->number2 && !str_cmp(victim->name, kingdom_table[victim->pcdata->hometown]->number2))
      STRFREE(kingdom_table[victim->pcdata->hometown]->number2);
   victim->pcdata->hometown = 1;
   victim->pcdata->kingdompid = kingdom_table[victim->pcdata->hometown]->kpid;
   victim->pcdata->caste = caste_Peasant;  
   victim->pcdata->town = NULL;
   victim->pcdata->resourcetype = 0;
   victim->pcdata->resource = 0;
   victim->pcdata->job = 1;
   remove_player_list(victim, 0);
   ch_printf(ch, "You kicked %s out of your kingdom.\n\r", PERS_MAP(victim, ch));
   ch_printf(victim, "%s kicked you out of the kingdom, sorry.\n\r", PERS_MAP(ch, victim));
   return;
}

int get_real_kingdom(int kpid)
{
   int x;
   
   for (x = 2; x < sysdata.max_kingdom; x++)
   {
      if (kingdom_table[x]->kpid == kpid)
         return x;
   }
   return 0;
}

//Will show the lists, aka kingdom/clan lists
void do_showlist(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int x;
   int start, next;
   int found = 0;
   CLAN_DATA *cdata;
   KMEMBER_DATA *klist;
   CMEMBER_DATA *clist;

   if (argument[0] == '\0')
   {
      send_to_char
         ("Syntax: showlist <list> <name or all> <sort>\n\rlist: kingdom  sort:caste, name, level\n\rlist: clan     sort:rank, name, level\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      do_showlist(ch, "");
      return;
   }
   if (str_cmp(arg1, "kingdom") && str_cmp(arg1, "clan"))
   {
      send_to_char("Values for list is either kingdom or clan.\n\r", ch);
      return;
   }
   for (x = 2; x < sysdata.max_kingdom; x++)
   {
      if (!str_cmp(arg2, kingdom_table[x]->name))
         found = 1;
   }
   for (cdata = first_clan; cdata; cdata = cdata->next)
   {
      if (!str_cmp(arg2, cdata->name))
         found = 2;
   }
   if (!str_cmp(arg2, "all"))
   {
      if (!str_cmp(arg1, "kingdom"))
         found = 3;
      if (!str_cmp(arg1, "clan"))
         found = 4;
   }

   if (found < 1)
   {
      send_to_char("The name you supplied does not exist, remember this search is case sensitive.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "caste"))
   {
      if (found == 1 || found == 3)
      {
         send_to_char("Name                  Kingdom       Lv  Caste          Race\n\r", ch);
         send_to_char
            ("&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = 2; x < sysdata.max_kingdom; x++)
         {
            if ((found == 1 && !str_cmp(arg2, kingdom_table[x]->name)) || found == 3)
            {
               start = MAX_CASTE;
               while (start)
               {
                  next = 0;
                  for (klist = first_kingdommember; klist; klist = klist->next)
                  {
                     if (get_real_kingdom(klist->kpid) == x)
                     {
                        if (klist->caste == start)
                        {
                           ch_printf(ch, "%-20s  %-12s  %2d  %s%-13s&c&w  %-10s\n\r",
                              klist->name, kingdom_table[get_real_kingdom(klist->kpid)]->name, klist->level,
                              get_caste_color(klist->caste), get_caste_name(klist->caste, klist->sex), 
                              race_table[klist->race]->race_name);
                        }
                        else
                        {
                           if (klist->caste > next && klist->caste < start)
                              next = klist->caste;
                        }
                     }
                  }
                  start = next;
               }
            }
         }
      }
      else
      {
         do_showlist(ch, "");
         return;
      }
      return;
   }
   if (!str_cmp(argument, "rank"))
   {
      if (found == 2 || found == 4)
      {
         send_to_char("Name                  Clan          Lv  Rank        Race\n\r", ch);
         send_to_char
            ("&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (cdata = first_clan; cdata; cdata = cdata->next)
         {
            if ((found == 2 && !str_cmp(arg2, cdata->name)) || found == 4)
            {
               start = 1;
               next = 4;
               while (start)
               {

                  for (clist = first_clanmember; clist; clist = clist->next)
                  {
                     if (!str_cmp(cdata->name, clist->clan))
                     {
                        if ((start == 4 && !str_cmp(clist->rank, "Leader"))
                           || (start == 3 && !str_cmp(clist->rank, "Number1"))
                           || (start == 2 && !str_cmp(clist->rank, "Number2")) || (start == 1 && !str_cmp(clist->rank, "Member")))
                        {
                           ch_printf(ch, "%-20s  %-12s  %2d  %-10s  %-10s\n\r",
                              clist->name, cdata->name, clist->level, clist->rank,
                              race_table[clist->race]->race_name);
                        }
                     }
                  }
                  --next;
                  if (next == 0)
                     start = 0;
               }
            }
         }
      }
      else
      {
         do_showlist(ch, "");
         return;
      }
      return;
   }
   if (!str_cmp(argument, "level"))
   {
      if (found == 1 || found == 3)
      {
         send_to_char("Name                  Kingdom       Lv  Caste          Race\n\r", ch);
         send_to_char
            ("&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = 2; x < sysdata.max_kingdom; x++)
         {
            if ((found == 1 && !str_cmp(arg2, kingdom_table[x]->name)) || found == 3)
            {
               start = MAX_LEVEL;
               while (start)
               {
                  next = 0;
                  for (klist = first_kingdommember; klist; klist = klist->next)
                  {
                     if (get_real_kingdom(klist->kpid) == x)
                     {
                        if (klist->level == start)
                        {
                           ch_printf(ch, "%-20s  %-12s  %2d  %s%-13s&c&w  %-10s\n\r",
                              klist->name, kingdom_table[get_real_kingdom(klist->kpid)]->name, klist->level,
                              get_caste_color(klist->caste), get_caste_name(klist->caste, klist->sex), 
                              race_table[klist->race]->race_name);
                        }
                        else
                        {
                           if (klist->level > next && klist->level < start)
                              next = klist->level;
                        }
                     }
                  }
                  start = next;
               }
            }
         }
      }
      if (found == 2 || found == 4)
      {
         send_to_char("Name                  Clan          Lv  Rank        Race\n\r", ch);
         send_to_char
            ("&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (cdata = first_clan; cdata; cdata = cdata->next)
         {
            if ((found == 2 && !str_cmp(arg2, cdata->name)) || found == 4)
            {
               start = MAX_LEVEL;
               while (start)
               {
                  next = 0;
                  for (clist = first_clanmember; clist; clist = clist->next)
                  {
                     if (!str_cmp(cdata->name, clist->clan))
                     {
                        if (clist->level == start)
                        {
                           ch_printf(ch, "%-20s  %-12s  %2d  %-10s  %-10s\n\r",
                              clist->name, cdata->name, clist->level, clist->rank,
                              race_table[clist->race]->race_name);
                        }
                        else
                        {
                           if (clist->level > next && clist->level < start)
                              next = clist->level;
                        }
                     }
                  }
                  start = next;
               }
            }
         }
      }
      return;
   }
   if (!str_cmp(argument, "name"))
   {
      if (found == 1 || found == 3)
      {
         send_to_char("Name                  Kingdom       Lv  Caste          Race\n\r", ch);
         send_to_char
            ("&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (x = 2; x < sysdata.max_kingdom; x++)
         {
            if ((found == 1 && !str_cmp(arg2, kingdom_table[x]->name)) || found == 3)
            {
               for (klist = first_kingdommember; klist; klist = klist->next)
               {
                  if (get_real_kingdom(klist->kpid) == x)
                  {
                     ch_printf(ch, "%-20s  %-12s  %2d  %s%-13s&c&w  %-10s\n\r",
                        klist->name, kingdom_table[get_real_kingdom(klist->kpid)]->name, klist->level,
                        get_caste_color(klist->caste), get_caste_name(klist->caste, klist->sex), 
                        race_table[klist->race]->race_name);
                  }
               }
            }
         }
      }
      if (found == 2 || found == 4)
      {
         send_to_char("Name                  Clan          Lv  Rank        Race\n\r", ch);
         send_to_char
            ("&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&R-&B-&c&w\n\r",
            ch);
         for (cdata = first_clan; cdata; cdata = cdata->next)
         {
            if ((found == 2 && !str_cmp(arg2, cdata->name)) || found == 4)
            {
               for (clist = first_clanmember; clist; clist = clist->next)
               {
                  if (!str_cmp(cdata->name, clist->clan))
                  {
                     ch_printf(ch, "%-20s  %-12s  %2d  %-10s  %-10s\n\r",
                        clist->name, cdata->name, clist->level, clist->rank, race_table[clist->race]->race_name);
                  }
               }
            }
         }
      }
      return;
   }
   return;
}

void check_kingdom_areas(int kingdom)
{
   TOWN_DATA *town;
   int tx;
   int x, xx, y;
   int size;
   
   for (x = 2; x < sysdata.max_kingdom; x++)
   {  
      for (town = kingdom_table[x]->first_town; town; town = town->next)
      {
         if (town->kingdom > kingdom)
            town->kingdom--;
         
         if (town->kpid > 0 && town->kingdom > 0 && kingdom_table[town->kingdom]->kpid != town->kpid)
         {
            for (tx = 0; tx < sysdata.max_kingdom; tx++)
            {
               if (kingdom_table[tx]->kpid == town->kpid)
               {
                  town->kingdom = tx;
                  break;
               }
            }
            if (tx == sysdata.max_kingdom)
            {
               town->kingdom = 0;
               town->kpid = 0;
               bug("Check_kingdom_areas: Area %s has been placed in the beginning Town", town->name);
            }
         }
         if (town->kingdom < 0 || town->kingdom >= sysdata.max_kingdom)
         {
            town->kingdom = 0;   
            town->kpid = 0;
            bug("Check_kingdom_areas: Town %s has been placed in the beginning Kingdom, needs to be manually removed", town->name);
         }   
         if (town->kpid == 0 && town->kingdom > 0)
         {
            town->kpid = kingdom_table[town->kingdom]->kpid;
         }
         size = get_control_size(town->size);
         for (xx = town->startx - size; xx <= town->startx+size; xx++)
         {
            for (y = town->starty - size; y <= town->starty+size; y++)
            {
                kingdom_sector[town->startmap][xx][y] = town->kingdom;
            }
         } 
      }
   }
}

void fix_kingdom_online_players(int kingdom)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ch;
   int kx;
   
         
   for (d = first_descriptor; d; d = d->next)
   {       
      if (d->character && d->character->pcdata)
      {
         ch = d->character;     
         if (ch->pcdata->hometown == kingdom)
         {
            send_to_char("It appears your kingdom has been destroyed!!!!!!!!\n\r", ch);
            ch->pcdata->hometown = 0;
            ch->pcdata->kingdompid = 0;
            ch->pcdata->town = NULL;
            save_char_obj(ch);
            continue;
         }
         if (ch->pcdata->hometown > kingdom)
            ch->pcdata->hometown--;
         if (ch->pcdata->kingdompid > 1)
         {    
            if (ch->pcdata->kingdompid != kingdom_table[ch->pcdata->hometown]->kpid)
            {
               for (kx = 2; kx < sysdata.max_kingdom; kx++)
               {
                  if (ch->pcdata->kingdompid == kingdom_table[kx]->kpid)
                  {
                     ch->pcdata->hometown = kingdom_table[kx]->num;
                     break;
                  }
               }
               if (kx == sysdata.max_kingdom)
               {
                  send_to_char("It appears your kingdom has been destroyed!!!!!!!!\n\r", ch);
                  ch->pcdata->hometown = 0;
                  ch->pcdata->kingdompid = 0;
                  ch->pcdata->town = NULL;
               }
            }
         } 
         if (ch->pcdata->hometown < 0 || ch->pcdata->hometown >= sysdata.max_kingdom)
         {
            send_to_char("It appears your kingdom has been destroyed!!!!!!!!\n\r", ch);
            ch->pcdata->hometown = 0;
            ch->pcdata->kingdompid = 0;
            ch->pcdata->town = NULL;
            save_char_obj(ch);
            continue;
         }    
         if (ch->pcdata->kingdompid == 0 && ch->pcdata->hometown != 0)
         {
            ch->pcdata->kingdompid = kingdom_table[ch->pcdata->hometown]->kpid;
         }
         save_char_obj(ch);
      }
   }
}

void fix_kingdom_mobiles(int kingdom)
{
   CHAR_DATA *ch;
   CHAR_DATA *gch_prev;
   for (ch = last_char; ch; ch = gch_prev)
   {
      gch_prev = ch->prev;
      
      if (IS_NPC(ch) && (xIS_SET(ch->act, ACT_MILITARY) || xIS_SET(ch->act, ACT_EXTRACTMOB) || xIS_SET(ch->act, ACT_KINGDOMMOB) || xIS_SET(ch->act, ACT_MOUNTSAVE)))
      {
         if (ch->m4 == kingdom)
         {
            if (xIS_SET(ch->act, ACT_MOUNTSAVE))
            {
              ch->m4 = 0;
              continue;
            }
            else
            {
               extract_char(ch, TRUE);
               continue;
            }
         }
         if (ch->m4 > kingdom)
         {
            ch->m4--;
            continue;
         }
      }
   }
}
             

//Removes a kingdom from the game, rather messy!!!
void remove_kingdom(int kingdom)
{
   int x;
   int y;
   sh_int npeace[MAX_KINGDOM][MAX_KINGDOM];
   char buf[MSL];
   KINGDOM_DATA *kingdomdata;
   CONQUER_DATA *conquer;
   CONQUER_DATA *next_conquer;
   TOWN_DATA *town;
   TRAINING_DATA *train;
   TRAINING_DATA *next_train;
   TRADE_DATA *trade;
   TRADE_DATA *trade_next;
   
   fix_kingdom_mobiles(kingdom);
   
   if (kingdom != sysdata.max_kingdom-1);
   {
      for (x = 2; x < sysdata.max_kingdom; x++)
      {
         if (x > kingdom)
         {
            kingdom_table[x]->num--;
         }
      }
      for (x = 0; x < MAX_KINGDOM; x++)
         for (y = 0; y < MAX_KINGDOM; y++)
            npeace[x][y] = 0;
      for (x = 0; x < sysdata.max_kingdom; x++)
         for (y = 0; y < sysdata.max_kingdom; y++)
            npeace[x][y] = kingdom_table[x]->peace[y];   
      
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
         for (y = 0; y < sysdata.max_kingdom; y++)
         {
            if (y >= kingdom)
               kingdom_table[x]->peace[y] = npeace[x][y+1];
            if (y == sysdata.max_kingdom-1)
               kingdom_table[x]->peace[y] = 0;
         }
      }
   }
   for (train = first_training; train; train = next_train)
   {
      next_train = train->next;
      if (train->kingdom == kingdom)
      {
         UNLINK(train, first_training, last_training, next, prev);
         DISPOSE(train);
      }
   }
   fwrite_training_list();
   for (trade = first_trade; trade; trade = trade_next)
   {
      trade_next = trade->next;
      if (trade->offering_kingdom == kingdom || trade->receiving_kingdom == kingdom)
      {
         UNLINK(trade, first_trade, last_trade, next, prev);
         DISPOSE(trade);
      }
   }
   save_trade_file();
   if (kingdom_table[kingdom]->first_town)
   {
      TOWN_DATA *nexttown;
      for (town = kingdom_table[kingdom]->first_town; town; town = nexttown)
      {
         nexttown = town->next;
         UNLINK(town, kingdom_table[kingdom]->first_town, kingdom_table[kingdom]->last_town, next, prev);
         STRFREE(town->mayor);
         STRFREE(town->name);
         DISPOSE(town);        
      } 
   }
   fix_kingdom_online_players(kingdom);
   for (x = 2; x < sysdata.max_kingdom; x++)
   {
      if (x == kingdom)
      {
         kingdomdata = kingdom_table[x];
         sprintf(buf, "%s%s.kingdom", KINGDOM_DIR, parse_save_file(kingdomdata->name));
         remove(buf);
         sprintf(buf, "%s%slog.txt", KINGDOM_DIR, parse_save_file(kingdomdata->name));
         remove(buf);
         sprintf(buf, "%s%s.depo", KINGDOM_DIR, parse_save_file(kingdomdata->name));
         remove(buf);
         STRFREE(kingdomdata->name);
         STRFREE(kingdomdata->logfile);
         STRFREE(kingdomdata->dtown);
         STRFREE(kingdomdata->ruler);
         if (kingdomdata->number1)
            STRFREE(kingdomdata->number1);
         if (kingdomdata->number2)
            STRFREE(kingdomdata->number2);
         DISPOSE(kingdomdata);
         kingdom_table[x] = kingdom_table[x+1];
      }
      if (x > kingdom)
         kingdom_table[x] = kingdom_table[x+1];
   }
   sysdata.max_kingdom--;
   check_kingdom_areas(kingdom);
   write_kingdom_list();
   save_mlist_data();
   for (conquer = first_conquer; conquer; conquer = next_conquer)
   {
      next_conquer = conquer->next;
      
      if (conquer->akingdom == kingdom || conquer->rkingdom == kingdom)
      {
         STRFREE(conquer->ntown);
         UNLINK(conquer, first_conquer, last_conquer, next, prev);
         DISPOSE(conquer);   
         continue;
      }
      if (conquer->akingdom > kingdom)
         conquer->akingdom--;
      if (conquer->rkingdom > kingdom)
         conquer->rkingdom--;
   }
   save_conquer_file();
   for (x = 0; x < sysdata.max_kingdom; x++)
      write_kingdom_file(x);
}

void remove_town(TOWN_DATA *town, int type)
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ch;
   TOWN_DATA *stown;
   char logb[200];
   int kx;
   int x, y;
   int size = get_control_size(town->size);
   int destroy = 0;
   TRAINING_DATA *train;
   TRAINING_DATA *next_train;
   
   for (x = town->startx - size; x <= town->startx+size; x++)
   {
      for (y = town->starty - size; y <= town->starty+size; y++)
      {
         kingdom_sector[town->startmap][x][y] = 0;
      }
   } 
   //since it can overwrite some aoc that belongs to another town...
   for (stown = kingdom_table[town->kingdom]->first_town; stown; stown = stown->next)
   {
      if (stown != town)
      {
         size = get_control_size(stown->size);
         for (x = stown->startx - size; x <= stown->startx+size; x++)
         {
            for (y = stown->starty - size; y <= stown->starty+size; y++)
            {
               kingdom_sector[stown->startmap][x][y] = stown->kingdom;
            }
         } 
      }
   }
   for (train = first_training; train; train = next_train)
   {
      next_train = train->next;
      if (train->town == town->tpid)
      {
         UNLINK(train, first_training, last_training, next, prev);
         DISPOSE(train);
      }
   }
   fwrite_training_list();
   kx = town->kingdom;
   if (town->kingdom > 1 && town->kingdom < sysdata.max_kingdom)
   {
      if (!str_cmp(kingdom_table[kx]->dtown, town->name))
      {
         destroy = 1;
         for (stown = kingdom_table[town->kingdom]->first_town; stown; stown = stown->next)
         {
            if (stown->kingdom == town->kingdom && stown != town)
            {
               STRFREE(kingdom_table[kx]->dtown);
               kingdom_table[kx]->dtown = STRALLOC(stown->name);    
               sprintf(logb, "Since %s was conquered %s has became your default town", town->name, stown->name);
               write_kingdom_logfile(kx, logb, KLOG_FIRE);  
               write_kingdom_file(kx);
               destroy = 0;
               for (d = first_descriptor; d; d = d->next)
               {
                  if (d->character && d->character->pcdata && d->character->pcdata->town && d->character->pcdata->town == town)
                  {
                     ch = d->character;
                     if (d->connected == CON_PLAYING)
                     {
                        send_to_char("The town you are in has been destroyed, you are being placed in the new default town.\n\r", ch);
                     }
                     ch->pcdata->town = stown;
                  }
               }
               break;
            }
         }
      }
      else
      {
         stown = get_town(kingdom_table[kx]->dtown);            
         for (d = first_descriptor; d; d = d->next)
         {
            if (d->character && d->character->pcdata && d->character->pcdata->town && d->character->pcdata->town == town)
            {
               ch = d->character;
               if (d->connected == CON_PLAYING)
               {
                  send_to_char("The town you are in has been destroyed, you are being placed in the default town.\n\r", ch);
               }
               ch->pcdata->town = stown;
            }
         }   
      }
      if (destroy == 1 && type == 0) //no more towns left, bye bye kingdom
      {
         if (town->kingdom > 1 && town->kingdom < sysdata.max_kingdom)
            remove_kingdom(town->kingdom);
         return;
      }
   }
   UNLINK(town, kingdom_table[kx]->first_town, kingdom_table[kx]->last_town, next, prev);
   STRFREE(town->mayor);
   STRFREE(town->name);
   DISPOSE(town);        
   write_kingdom_file(kx);
}   

void remove_all_towns(int kingdom)
{
   TOWN_DATA *town;
   TOWN_DATA *ntown;
   
   for (town = kingdom_table[kingdom]->first_town; town; town = ntown)
   {
      ntown = town->next;
      remove_town(town, 1);
   }
}
void do_removekingdom(CHAR_DATA *ch, char *argument)
{
   if (get_trust(ch) < LEVEL_ADMIN)
   {
      send_to_char("Sorry this is only for Admin Level Staff.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: removekingdom <kingdom number>\n\r", ch);
      return;
   }
   if (atoi(argument) < 2 || atoi(argument) >= sysdata.max_kingdom)
   {
      ch_printf(ch, "Your choices for kingdom number are 2 - %d\n\r", sysdata.max_kingdom-1);
      return;
   }
   remove_all_towns(atoi(argument));
   remove_kingdom(atoi(argument));
   send_to_char("Done.\n\r", ch);
   return;
}
void do_removetown(CHAR_DATA *ch, char *argument)
{
   TOWN_DATA *town;
   
   if (get_trust(ch) < LEVEL_ADMIN)
   {
      send_to_char("Sorry this is only for Admin Level Staff.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: removetown <area name>\n\r", ch);
      return;
   } 
   town = get_town(argument);
   if (town)
   {
      remove_town(town, 0);
      send_to_char("Done.\n\r", ch);
      return;
   }
   else
   {
      send_to_char("That is not a valid town, please type zones.\n\r", ch);
      return;
   }
}     

//Shows conquer data, for imms and players
void do_conquer(CHAR_DATA *ch, char *argument)
{ 
   CONQUER_DATA *conquer;
   int tleft, days, rtime, min, sec, hour;
   char timeleft[150];
   char buf[200];
   
   if (IS_NPC(ch))
   {
      send_to_char("This is not for you!\n\r", ch);
      return;
   }
   send_to_char("&w&WConquering Kingdom     Receiving Kingdom      Target Town            Time Left\n\r", ch);
   for (conquer = first_conquer; conquer; conquer = conquer->next)
   {
      if (get_trust(ch) > LEVEL_PC || (ch->pcdata->hometown == conquer->akingdom || ch->pcdata->hometown == conquer->rkingdom))
      {
         tleft = conquer->time + 259200 - time(0);
         if (tleft < 1)
         {
            sprintf(timeleft, "Any time Now!!!");
         }
         else
         {
            if (tleft > 86400)
            {
               days = tleft / 86400;
               rtime = tleft % 86400;
            }
            else
            {
               days = 0;
               rtime = tleft;
            }
            sec = rtime % 60;
            hour = rtime / 3600;
            if (rtime > 60)
            {
               min = rtime / 60;
               min = min % 60;
            }
            else
            {
               min = 0;
            }
            sprintf(timeleft, "&c&wD:%d H:%d M:%d S:%d\n\r", days, hour, min, sec);
         }
         sprintf(buf, "&w&R%-20s   %-20s   &c&g%-20s   %s", kingdom_table[conquer->akingdom]->name, kingdom_table[conquer->rkingdom]->name, 
            conquer->town->name, timeleft);       
         send_to_char(buf, ch);
      }
   }
}
      
//Passes ownership of a kingdom to another player...
void do_givecrown(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *king;
   CHAR_DATA *victim;
   char arg1[MIL];
   
   if (IS_NPC(ch))
   {
      send_to_char("Foolish mob!!\n\r", ch);
      return;
   }
   
   if (argument[0] == '\0')
   {
      if (ch->pcdata->caste >= MAX_CASTE)
         send_to_char("givecrown <king> <new king>\n\r", ch);
      else if (ch->pcdata->caste == caste_King)
         send_to_char("givecrown <player>\n\r", ch);
      else
         send_to_char("You are not a king or an Admin so you cannot use this command.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   if (ch->pcdata->caste >= MAX_CASTE)
   {
      if ((king = get_char_world(ch, arg1)) == NULL)
      {
         send_to_char("The king is not logged in, loadup the player or wait.\n\r", ch);
         return;
      }
      if ((victim = get_char_world(ch, argument)) == NULL)
      {
         send_to_char("The target is not logged in, loadup the player or wait.\n\r", ch);
         return;
      }
      if (IS_NPC(king) || IS_NPC(victim))
      {
         send_to_char("Only on PCs.\n\r", ch);
         return;
      }
      //check to make sure the king is really the king
      if (king->pcdata->caste != caste_King)
      {
         send_to_char("The target you chose for king is not a king.\n\r", ch);
         return;
      }
      if (king->pcdata->hometown <= 1 || king->pcdata->hometown >= sysdata.max_kingdom)
      {
         send_to_char("The target you chose for king is not in a valid kingdom.\n\r", ch);
         return;
      }
      //moment of fun
      king->pcdata->caste = 1;
      victim->pcdata->caste = caste_King;
      victim->pcdata->hometown = king->pcdata->hometown;
      STRFREE(kingdom_table[king->pcdata->hometown]->ruler);
      kingdom_table[king->pcdata->hometown]->ruler = STRALLOC(victim->name);
      ch_printf(king, "%s has stripped you of your crown and gave it to %s", PERS_MAP(ch, king), PERS_MAP(victim, king));
      ch_printf(victim, "%s has stripped %s of the crown and gave it to you!!!", PERS_MAP(ch, victim), PERS_MAP(king, victim));
      ch_printf(ch, "You strip %s of the crown and give it to %s", PERS_MAP(king, ch), PERS_MAP(victim, ch));
      return;
   }  
   if (ch->pcdata->caste == caste_King)
   {
      char buf[200];
      if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
      {
         send_to_char("That player is not in the room with you.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Give your crown to a mob, how delightful.\n\r", ch);
         return;
      }
      if (ch->pcdata->hometown <= 1 || ch->pcdata->hometown >= sysdata.max_kingdom)
      {
         send_to_char("The are not king of a valid kingdom, tell an immortal.\n\r", ch);
         return;
      }
      if (ch->pcdata->hometown != victim->pcdata->hometown)
      {
         send_to_char("Why would you want to appoint a king not in your own kingdom?\n\r", ch);
         return;
      }
      //moment of fun
      ch->pcdata->caste = 1;
      victim->pcdata->caste = caste_King;
      STRFREE(kingdom_table[ch->pcdata->hometown]->ruler);
      kingdom_table[ch->pcdata->hometown]->ruler = STRALLOC(victim->name);
      sprintf(buf, "$n removes $s crown and puts it upon $N's head and appoints $M %s", ch->sex == 2 ? "Queen" : "King");
      act(AT_WHITE, buf, ch, NULL, victim, TO_ROOM);
      sprintf(buf, "You remove your crown and put it upon $n's head and appoint $M %s", ch->sex == 2 ? "Queen" : "King");
      act(AT_WHITE, buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "$n removes $s crown and puts it upon your head and appoints you %s", ch->sex == 2 ? "Queen" : "King");
      act(AT_WHITE, buf, ch, NULL, victim, TO_VICT);
      return; 
   }
   send_to_char("You are not a king or Admin, so you cannot use this command.\n\r", ch);
   return;
}
      
void do_tinduct(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TOWN_DATA *town = NULL;
   char arg[MIL];
   int ht;

   if (IS_NPC(ch))
   {
      send_to_char("This command is for players only.\n\r", ch);
      return;
   }
   ht = ch->pcdata->hometown;
   if (ch->pcdata->caste < ch->pcdata->town->minhappoint)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("Hard to induct someone if you don't belong to a town.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("tinduct <player> [town}.\n\r", ch);
      send_to_char("Note, town selection is only for kings.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("That person is not online right now.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Cannot use this command on mobiles, sorry.\n\r", ch);
      return;
   }
   if (victim->pcdata->hometown != ht)
   {
      send_to_char("This person is in another kingdom.\n\r", ch);
      return;
   }
   if (argument[0] != '\0')
   {
      if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minswitchtown)
      {
         send_to_char("Only those with MinSwitchTown ranking can put people in selected towns.\n\r", ch);
         return;
      }
      town = get_town(argument);
      if (!town || town->kingdom != ch->pcdata->hometown)
      {
         send_to_char("Your kingdom does not run that town.\n\r", ch);
         return;
      }
   }
   if (!town)
      town = ch->pcdata->town;
   victim->pcdata->town = town;
   ch_printf(victim, "%s has inducted you into the town of %s.\n\r", PERS_MAP(ch, victim), town->name);
   sprintf(logb, "%s inducted %s into the town of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), PERS_KINGDOM(victim, victim->pcdata->hometown), town->name);
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_TINDUCT);
   send_to_char("Done.\n\r", ch);
   return;
}
void do_kinduct(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TOWN_DATA *town;
   int ht;
   char mlogbuf[500];

   if (IS_NPC(ch))
   {
      send_to_char("This command is for players only.\n\r", ch);
      return;
   }
   ht = ch->pcdata->hometown;
   if (ch->pcdata->caste < kingdom_table[ht]->minhappoint)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, argument)) == NULL)
   {
      send_to_char("That person is not online right now.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Cannot use this command on mobiles, sorry.\n\r", ch);
      return;
   }
   if (ch == victim)
   {
      send_to_char("You are already in your own kingdom.\n\r", ch);
      return;
   }
   if (victim->pcdata->hometown > 1)
   {
      send_to_char("This person is already in a kingdom.\n\r", ch);
      return;
   }
   if (kingdom_table[ch->pcdata->hometown]->dtown == NULL)
   {
      send_to_char("Your kingdom doesn't have a default town yet, tell the king.\n\r", ch);
      return;
   }
   town = get_town(kingdom_table[ch->pcdata->hometown]->dtown);
   if (town)
   {
      victim->pcdata->town = town;
      victim->pcdata->hometown = ch->pcdata->hometown;
      victim->pcdata->kingdompid = kingdom_table[victim->pcdata->hometown]->kpid;
      victim->pcdata->caste = caste_Peasant;
      add_player_list(victim, 0);
      ch_printf(victim, "%s has inducted you into the kingdom of %s.\n\r", PERS_MAP(ch, victim), kingdom_table[ht]->name);
      sprintf(logb, "%s inducted", PERS_KINGDOM(ch, ch->pcdata->hometown));
      sprintf(mlogbuf, " %s into your kingdom", PERS_KINGDOM(victim, victim->pcdata->hometown));
      strcat(logb, mlogbuf);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_KINDUCT);
      send_to_char("Done.\n\r", ch);
      return;
   }
   else
   {
      send_to_char("Your kingdom's town is invalid, tell the king.\n\r", ch);
      return;
   }
}

void do_joinkingdom(CHAR_DATA * ch, char *argument)
{
   int ht;
   TOWN_DATA *town;

   if (IS_NPC(ch))
   {
      send_to_char("This command is for players only.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Num  Ruler           Name        Allowing?\n\r", ch);
      for (ht = 0; ht < sysdata.max_kingdom; ht++)
      {
         ch_printf(ch, "%-2d>  %-14s  %-10s  %s\n\r", ht, kingdom_table[ht]->ruler,
            kingdom_table[ht]->name, kingdom_table[ht]->allowjoin > 0 ? "Yes" : "No");
      }
      send_to_char("\n\rTo choose a Kingdom use joinkingdom <num>.\n\r", ch);
      return;
   }
   else
   {
      if (!isdigit(argument[0]))
      {
         do_joinkingdom(ch, "");
         return;
      }
      else
      {
         ht = atoi(argument);
         if (ht >= sysdata.max_kingdom || ht < 0)
         {
            send_to_char("That value is not in range.\n\r", ch);
            return;
         }
         else
         {
            if (ch->pcdata->hometown > 1)
            {
               send_to_char("You can only join a Kingdom if you belong to Rafermand or Niemria\n\r", ch);
               return;
            }
         }
         if (kingdom_table[ht]->allowjoin == 0)
         {
            send_to_char("You cannot Join this Kingdom, they are not allowing anyone to join ATM.\n\r", ch);
            return;
         }
         if (kingdom_table[ht]->dtown == NULL)
         {
            send_to_char("This Kingdom hasn't selected a default town yet, please contact the king.\n\r", ch);
            return;
         }
         town = get_town(kingdom_table[ht]->dtown);
         if (town)
         {
            ch->pcdata->town = town;
         }
         else
         {
            send_to_char("The Default Town of the kingdom is not valid, tell the king.\n\r", ch);
            return;
         }
         ch->pcdata->hometown = ht;
         ch->pcdata->kingdompid = kingdom_table[ht]->kpid;
         ch->pcdata->caste = caste_Peasant;
         add_player_list(ch, 0);
         ch_printf(ch, "You have joined the Kingdom of %s.\n\r", kingdom_table[ht]->name);
         sprintf(logb, "%s joined your kingdom", PERS_KINGDOM(ch, ch->pcdata->hometown));
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_JOINKINGDOM);
         return;
      }
   }
}

//Sets a mount free
void do_setfree(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *mount;
   
   if (IS_NPC(ch))
   {
      send_to_char("Only for PCs.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("setfree <target>\n\r", ch);
      send_to_char("setfree lost (to remove your flag if you lost your mount)\n\r", ch);
      return;
   }
   if (!ch->pcdata->mount)
   {
      if (!str_cmp(argument, "lost"))
      {
         xREMOVE_BIT(ch->act, PLR_BOUGHT_MOUNT);
         send_to_char("Removing the flag so you can get another mount.\n\r", ch);
         return;
      }
      send_to_char("You do not even have a mount!\n\r", ch);
      return;
   }
   if (!xIS_SET(ch->act, PLR_BOUGHT_MOUNT))
   {
      send_to_char("You cannot set free that kind of mount.\n\r", ch);
      return;
   }
   if ((mount = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("That mount is not in the room with you.\n\r", ch);
      return;
   }
   if (ch->pcdata->mount != mount)
   {
      send_to_char("The mount you are targetting is not your mount.\n\r", ch);
      return;
   }
   if (!xIS_SET(mount->act, ACT_MOUNTABLE) || !xIS_SET(mount->act, ACT_MOUNTSAVE))
   {
      send_to_char("Won't work on that kind of mount.\n\r", ch);
      return;
   }
   extract_char(mount, TRUE);
   send_to_char("You watch in silence as your trust mount returns back into the wilderness.\n\r", ch);
   xREMOVE_BIT(ch->act, PLR_BOUGHT_MOUNT);
   return;
}      

//Shows any offers from other kingdoms.     
void do_offers(CHAR_DATA * ch, char *argument)
{
   int ht;
   int x, y = 0;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobs.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minhappoint
      || ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend
      || ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal || ch->pcdata->caste == caste_God)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   ht = ch->pcdata->hometown;
   if (ht == -1)
   {
      send_to_char("You need to pick a Kingdom First.\n\r", ch);
      return;
   }
   ch_printf(ch,
         "&c&wKingdom          Status   Kingdom          Status   Kingdom          Status   \n\r----------------------------------------------------------------------------\n\r");
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      if (ch->pcdata->hometown == x)
         continue;

      if (kingdom_table[x]->cpeace[ht] == -1)
         ch_printf(ch, "&c&w%-15s  &zNone     ", kingdom_table[x]->name);
      else
         ch_printf(ch, "&c&w%-15s  %s%-7s  ", kingdom_table[x]->name,
            peacecolor[kingdom_table[x]->cpeace[ht]], peacestatus[kingdom_table[x]->cpeace[ht]]);

      y++;
      if (y % 3 == 0)
         ch_printf(ch, "\n\r");
   }
   if (y % 3 != 0)
      ch_printf(ch, "\n\r");
}

//Show offers you have made
void do_offered(CHAR_DATA * ch, char *argument)
{
   int ht;
   int x, y = 0;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobs.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minhappoint
      || ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend
      || ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal || ch->pcdata->caste == caste_God)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   ht = ch->pcdata->hometown;
   if (ht == -1)
   {
      send_to_char("You need to pick a Kingdom First.\n\r", ch);
      return;
   }
   ch_printf(ch,
         "&c&wKingdom          Status   Kingdom          Status   Kingdom          Status   \n\r----------------------------------------------------------------------------\n\r");
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      if (ch->pcdata->hometown == x)
         continue;

      if (kingdom_table[ht]->cpeace[x] == -1)
         ch_printf(ch, "&c&w%-15s  &zNone     ", kingdom_table[x]->name);
      else
         ch_printf(ch, "&c&w%-15s  %s%-7s  ", kingdom_table[x]->name,
            peacecolor[kingdom_table[ht]->cpeace[x]], peacestatus[kingdom_table[ht]->cpeace[x]]);

      y++;
      if (y % 3 == 0)
         ch_printf(ch, "\n\r");
   }
   if (y % 3 != 0)
      ch_printf(ch, "\n\r");
}

//Declare War, Peace, Neutral, Trade, etc       
void do_declare(CHAR_DATA * ch, char *argument)
{
   int ht;
   int oht = 0;
   int pstatus;
   int x;
   char arg[MIL];

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobs.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste != caste_King)
   {
      if (ch->pcdata->caste <= caste_Staff)
      {
         send_to_char("This command is for Kings or Staff level immortals.\n\r", ch);
         return;
      }
   }
   ht = ch->pcdata->hometown;
   if (ht == -1)
   {
      send_to_char("You need to pick a Kingdom First.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: declare <argument> <kingdom name/num>\n\rargument - war, neutral, trading, peace, remove, reject\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);

   for (x = 0; x < MAX_PEACEVALUE; x++)
   {
      if (!str_prefix(arg, peacestatus[x]))
         break;
   }
   if (x == MAX_PEACEVALUE && str_prefix(arg, "reject") && str_prefix(arg, "remove"))
   {
      do_declare(ch, "");
      return;
   }
   pstatus = x;
   if (!str_prefix(arg, "reject") || !str_prefix(arg, "remove"))
   {
      if (!str_prefix(arg, "reject"))
         pstatus = -1;
      else
         pstatus = -2;
   }
   if (isdigit(argument[0]))
   {
      if (atoi(argument) < 0 || atoi(argument) >= sysdata.max_kingdom)
      {
         ch_printf(ch, "Values for Kingdoms are 0 to %d", sysdata.max_kingdom - 1);
         return;
      }
      oht = atoi(argument);
   }
   else
   {
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
         if (!str_prefix(argument, kingdom_table[x]->name))
         {
            oht = x;
            break;
         }
      }
      if (x == sysdata.max_kingdom)
      {
         send_to_char("That is not a valid Kingdom.\n\r", ch);
         return;
      }
   }
   if (oht == ht)
   {
      send_to_char("You cannot make an offer to your own kingdom.\n\r", ch);
      return;
   }
   if (pstatus == kingdom_table[ht]->peace[oht])
   {
      ch_printf(ch, "That is your current status with %s.\n\r", kingdom_table[oht]->name);
      return;
   }
   //Don't accept an offer from another kingdom
   if (pstatus == -1)
   {
      if (kingdom_table[oht]->cpeace[ht] == -1)
      {
         ch_printf(ch, "The kingdom of %s has not made you an offer yet.\n\r", kingdom_table[oht]->name);
         return;
      }
      else
      {
         ch_printf(ch, "Rejecting the kingdom of %s's offer of %s.\n\r", kingdom_table[oht]->name, peacestatus[kingdom_table[oht]->cpeace[ht]]);
         sprintf(logb, "%s rejected %s's offer of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), kingdom_table[oht]->name, peacestatus[kingdom_table[oht]->cpeace[ht]]);
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_DECLARE);
         sprintf(logb, "%s rejected your offer of %s", PERS_KINGDOM(ch, oht), peacestatus[kingdom_table[oht]->cpeace[ht]]);
         write_kingdom_logfile(oht, logb, KLOG_DECLARE);
         kingdom_table[oht]->cpeace[ht] = -1;
         write_kingdom_file(oht);
         write_kingdom_list();
         return;
      }
   }
   //Remove your offer to the other kingdom
   if (pstatus == -2)
   {
      if (kingdom_table[ht]->cpeace[oht] == -1)
      {
         ch_printf(ch, "There is no offer to the kingdom of %s to revoke.\n\r", kingdom_table[oht]->name);
         return;
      }
      else
      {
         ch_printf(ch, "Revoking the offer of %s toward the kingdom of %s.\n\r",
            peacestatus[kingdom_table[ht]->cpeace[oht]], kingdom_table[oht]->name);
         sprintf(logb, "%s Revoked the offer of %s toward %s", PERS_KINGDOM(ch, ch->pcdata->hometown), peacestatus[kingdom_table[ht]->cpeace[oht]], kingdom_table[oht]->name);
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_DECLARE);
         sprintf(logb, "%s Revoked the offer of %s toward you", PERS_KINGDOM(ch, oht), peacestatus[kingdom_table[ht]->cpeace[oht]]);
         write_kingdom_logfile(oht, logb, KLOG_DECLARE);
         kingdom_table[ht]->cpeace[oht] = -1;
         write_kingdom_file(ht);
         write_kingdom_list();
         return;
      }
   }
   //Don't need both sides to agree to go down (such as Trading to Neutral, etc)
   //If the kingdom is Rafer/Niemria, can toggle on will  
   if ((pstatus < kingdom_table[ht]->peace[oht]) || oht <= 1)
   {
      ch_printf(ch, "You are now declaring %s towards the kingdom of %s\n\r", peacestatus[pstatus], kingdom_table[oht]->name);
      sprintf(logb, "%s is declaring %s toward %s", PERS_KINGDOM(ch, ch->pcdata->hometown), peacestatus[pstatus], kingdom_table[oht]->name);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_DECLARE);
      sprintf(logb, "%s is declaring %s toward you", PERS_KINGDOM(ch, oht), peacestatus[pstatus]);
      write_kingdom_logfile(oht, logb, KLOG_DECLARE);
      kingdom_table[ht]->peace[oht] = pstatus;
      kingdom_table[oht]->peace[ht] = pstatus;
      kingdom_table[ht]->cpeace[oht] = -1;
      kingdom_table[oht]->cpeace[ht] = -1;
      write_kingdom_file(oht);
      write_kingdom_file(ht);
      write_kingdom_list();
      return;
   }
   else //Both sides must agree to go up (such as War to Trading)
   {
      if (kingdom_table[oht]->cpeace[ht] == pstatus) //Both sides agree, go
      {
         ch_printf(ch, "You accepted the kingdom of %s's %s offer.\n\r", kingdom_table[oht]->name, peacestatus[pstatus]);
         sprintf(logb, "%s accepted the %s's %s offer", PERS_KINGDOM(ch, ch->pcdata->hometown), kingdom_table[oht]->name, peacestatus[pstatus]);
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_DECLARE);
         sprintf(logb, "%s accepted the offer of %s from you", PERS_KINGDOM(ch, oht), peacestatus[pstatus]);
         write_kingdom_logfile(oht, logb, KLOG_DECLARE);
         kingdom_table[ht]->peace[oht] = pstatus;
         kingdom_table[oht]->peace[ht] = pstatus;
         kingdom_table[ht]->cpeace[oht] = -1;
         kingdom_table[oht]->cpeace[ht] = -1;
         write_kingdom_file(oht);
         write_kingdom_file(ht);
         write_kingdom_list();
         return;
      }
      if (kingdom_table[oht]->cpeace[ht] == -1) //Make an offer
      {
         ch_printf(ch, "You make an offer of %s to the kingdom of %s.\n\r", peacestatus[pstatus], kingdom_table[oht]->name);
         sprintf(logb, "%s made an offer of %s to %s", PERS_KINGDOM(ch, ch->pcdata->hometown), peacestatus[pstatus], kingdom_table[oht]->name);
         write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_DECLARE);
         sprintf(logb, "%s made an offer of %s to you", PERS_KINGDOM(ch, oht), peacestatus[pstatus]);
         write_kingdom_logfile(oht, logb, KLOG_DECLARE);
         kingdom_table[ht]->cpeace[oht] = pstatus;
         write_kingdom_file(ht);
         write_kingdom_list();
         return;
      }
      //Ok you are making an offer, but it is not what the other side proposed
      if ((kingdom_table[oht]->cpeace[ht] != -1) && (kingdom_table[oht]->cpeace[ht] != pstatus))
      {
         ch_printf(ch, "The kingdom of %s made an offer of %s not %s", kingdom_table[oht]->name,
            peacestatus[kingdom_table[oht]->cpeace[ht]], peacestatus[pstatus]);
         return;
      }
   }
   do_declare(ch, "");
}


int klog_linecount(char *logfile)
{
   int cnt = 0;
   FILE *fp;
   
   if ((fp = fopen(logfile, "r")) == NULL)
      return 0;
   
   while (!feof(fp))
   {
      fread_to_eol(fp);
      cnt++;
   }
   //Evil mistake :-( -- 04/10/03
   fclose(fp);
   return cnt;
}

void klog_showline(CHAR_DATA *ch, char *logfile, int count, int endline)
{
   int cnt = 0;
   FILE *fp;
   int startline = 0;
   char *buf;
   char time[MSL];
   char field[MSL];
   struct tm *tmptr;
   time_t timevalue;
   
   if ((fp = fopen(logfile, "r")) == NULL)
      return;
      
   if (endline > 0)
      startline = count;
      
   while (!feof(fp))
   {
      buf = fread_line(fp);
      cnt++;
      //Do the checks to make sure we should display this line, no point in parsing it if we are not
      if (startline > 0)
      {
         if (cnt < startline || cnt > endline)
            continue;
      }
      else
      {
         if (cnt > count && count > 0)
            continue;
      }
      buf = one_argument(buf, time);  //Time
      buf = one_argument(buf, field); //Field
      //Rest should be the actual logfile
      timevalue = atoi(time);
      tmptr = localtime(&timevalue);
      //Jan 5 22:05:02 2003
      strftime(time, 80, "%b %d %H:%M:%S %Y", tmptr);
      pager_printf(ch, "&w&W[%-4d] %-20s  [%-15s]  %s", cnt, time, field, buf);         
   }
   fclose(fp);
}      
   
//Feb 18 2:50 2003
//2:18:11:50:2003   
//Parses a time argument, such such fun
int get_klog_time_argument(char *argument)
{ 
   int x;
   char buf[MSL];
   int month, day, hour, minute, second, year, gtime, ly;
   int on_leapyear = 0;
   //month:day:hour:minute:second:year
   
   //Put spaces in, easier to parse this way...
   for (x = 0;;x++)
   {
      if (argument[x] == '\0')
         break;
      if (argument[x] == ':')
         argument[x] = ' ';
   } 
   argument = one_argument(argument, buf);
   month = atoi(buf);
   argument = one_argument(argument, buf);
   day = atoi(buf);
   argument = one_argument(argument, buf);
   hour = atoi(buf);
   argument = one_argument(argument, buf);
   minute = atoi(buf);
   argument = one_argument(argument, buf);
   second = atoi(buf);
   argument = one_argument(argument, buf);
   year = atoi(buf);
   
   if ((year < 2000 || year > 2040) || (month < 1 || month > 12) || (day < 1 || day > 31) || (hour < 0 || hour > 23)
   || (minute < 0 || minute > 59) || (second < 0 || second > 59))
      return -1;
   
   day--;
   ly = (year - 1968) / 4;
   if ((year - 1968) % 4 == 0)
      on_leapyear = 1;
   year = year - 1970;
   gtime = year * 31536000; //No Leap year calculated
   gtime += ly * 86400; //Leap Year addition
   gtime += hour * 3600;
   gtime += minute * 60;
   gtime += second;
   
   //Jan - 31 Feb - 28 Mar - 31 Apr - 30 May - 31 Jun - 30 Jul - 31 Aug - 31 Sep - 30 Oct - 31 Nov - 30 Dec - 31
   if (month == 1)
   {
      gtime += day * 86400;
      if (on_leapyear)
         gtime -= 86400;
   }
   if (month == 2)
   {
      gtime += (31 * 86400) + day * 86400;
      if (on_leapyear)
         gtime -= 86400;
   }
   if (month == 3)
      gtime += (59 * 86400) + day * 86400;
   if (month == 4)
      gtime += (90 * 86400) + day * 86400;
   if (month == 5)
      gtime += (120 * 86400) + day * 86400;
   if (month == 6)
      gtime += (151 * 86400) + day * 86400;
   if (month == 7)
      gtime += (181 * 86400) + day * 86400;
   if (month == 8)
      gtime += (212 * 86400) + day * 86400;
   if (month == 9)
      gtime += (243 * 86400) + day * 86400;
   if (month == 10)
      gtime += (273 * 86400) + day * 86400;
   if (month == 11)
      gtime += (304 * 86400) + day * 86400;
   if (month == 12)
      gtime += (334 * 86400) + day * 86400;
      
   gtime += 21600; // 6 Days, honestly not sure why it is off 6 days
   //-3600 for daylight savings time, 0 when it ends
   gtime -= 3600;
      
   return gtime;
}
void klog_fullsearch(CHAR_DATA *ch, char *logfile, int count, int endline, char *type, char *search, int stype)
{
   int cnt = 0;
   FILE *fp;
   int startline = 0;
   char *buf;
   char time[MSL];
   char field[MSL];
   struct tm *tmptr;
   time_t timevalue;
   
   if ((fp = fopen(logfile, "r")) == NULL)
      return;
      
   if (stype == 0)
      startline = count;
      
   if ((stype == 3 || stype == 7) && endline > 0)
      startline = count;
      
   while (!feof(fp))
   {
      buf = fread_line(fp);
      //Check the time, to see if it is in the time range
      buf = one_argument(buf, time);  //Time
      buf = one_argument(buf, field); //Field
      timevalue = atoi(time);
      if (stype == 1 || stype == 2 || stype == 3 || stype == 4)
      {
         if (str_cmp(field, type))
            continue;
      }
      if (stype == 5 || stype == 6 || stype == 7 || stype == 8)
      {
         if (str_infix(search, buf))
            continue;
      }
      if (stype == 0 || stype == 2 || stype == 6)
      {
         if (timevalue < startline || timevalue > endline)
            continue;
      }
      cnt++;
      if ((stype == 3 || stype == 7) && startline > 0)
      {
         if (cnt < startline || cnt > endline)
            continue;
      }
      else if ((stype == 3 || stype == 7) && startline <= 0)
      {
         if (cnt > count && count > 0)
            continue;
      }
      //Rest should be the actual logfile
      tmptr = localtime(&timevalue);
      //Jan 5 22:05:02 2003
      strftime(time, 80, "%b %d %H:%M:%S %Y", tmptr);
      if (stype != 4 && stype != 8)
         pager_printf(ch, "&w&W[%-4d] %-20s  [%-15s]  %s", cnt, time, field, buf);         
   }
   if (stype == 4 || stype == 8)
      ch_printf(ch, "There are %d lines\n\r", cnt);
      
   fclose(fp);
}      

//7 days 12 hours 15 minutes 5 seconds
//7:12:15:5
int get_prune_time(char *argument)
{
   int x;
   char buf[MSL];
   int day, hour, minute, second, time;
   //month:day:hour:minute:second:year
   
   //Put spaces in, easier to parse this way...
   for (x = 0;;x++)
   {
      if (argument[x] == '\0')
         break;
      if (argument[x] == ':')
         argument[x] = ' ';
   } 
   argument = one_argument(argument, buf);
   day = atoi(buf);
   argument = one_argument(argument, buf);
   hour = atoi(buf);
   argument = one_argument(argument, buf);
   minute = atoi(buf);
   argument = one_argument(argument, buf);
   second = atoi(buf);
   
   if ((day < 0 || day > 365) || (hour < 0 || hour > 23)
   || (minute < 0 || minute > 59) || (second < 0 || second > 59))
      return -1;   
   
   time = (day*86400) + (hour*3600) + (minute*60) + second;
   return time;
}   

void save_sysdata args((SYSTEM_DATA sys));


//adjusts the game clock, so you can move time forward/backwards
void do_advanceclock(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   int ctime;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  advanceclock <forward/backwards> <time>\n\r", ch);
      send_to_char("Syntax:  Uses same syntax parsing as klog prune time\n\r", ch);
      send_to_char("NOTE:  It is dangerous to move the time backwards\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "forward"))
   {
      ctime = get_prune_time(argument);
      if (ctime <= 0)
      {
         send_to_char("Inproper Syntax, help klog and see syntax for klog prune time.\n\r", ch);
         return;
      }
      if (sysdata.start_calender - ctime/40 <= 0)
      {
         send_to_char("Cannot pust the start calender value past 0, sorry.\n\r", ch);
         return;
      }
      if (ctime < 40)
      {
         send_to_char("40 seconds is the lowest value you can enter, because we are converting\n\rreal time to game time.\n\r", ch);
         return;
      }
      sysdata.start_calender -= ctime /40;
      send_to_char("Done.\n\r", ch);
      save_sysdata(sysdata);
      return;
   }
   if (!str_cmp(arg, "backwards"))
   {
      ctime = get_prune_time(argument);
      if (ctime <= 0)
      {
         send_to_char("Inproper Syntax, help klog and see syntax for klog prune time.\n\r", ch);
         return;
      }
      if (sysdata.start_calender + ctime/40 >= time(0))
      {
         send_to_char("This pushes the start of the game calender past the current time, not possible!\n\r", ch);
         return;
      }
      if (ctime < 40)
      {
         send_to_char("40 seconds is the lowest value you can enter, because we are converting\n\rreal time to game time.\n\r", ch);
         return;
      }
      sysdata.start_calender += ctime /40;
      save_sysdata(sysdata);
      send_to_char("Done.\n\r", ch);
      return;
   }
   do_advanceclock(ch, "");
   return;
}
void do_kingdomlog(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   int count = 0;
   int endline = 0;
   
   set_char_color(AT_PLAIN, ch);
   
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minreadlog)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: klog list linecount\n\r", ch);
      send_to_char("Syntax: klog list [lines num]\n\r", ch);
      send_to_char("Syntax: klog list <start line> <end line>\n\r", ch);
      send_to_char("Syntax: klog time <start time> <end time>\n\r", ch);
      send_to_char("Syntax: klog type <catagory> linecount\n\r", ch);
      send_to_char("Syntax: klog type <catagory> [lines num]\n\r", ch);
      send_to_char("Syntax: klog type <catagory> <start line> <end line>\n\r", ch);
      send_to_char("Syntax: klog type <catagory> time <start time> <end time>\n\r", ch);
      send_to_char("Syntax: klog search <keyword> linecount\n\r", ch);
      send_to_char("Syntax: klog search <keyword> [lines num]\n\r", ch);
      send_to_char("Syntax: klog search <keyword> <start line> <end line>\n\r", ch);
      send_to_char("Syntax: klog search <keyword> time <start time> <end time>\n\r", ch);
      if (ch->pcdata->caste == caste_King || ch->pcdata->caste >= caste_Staff)
      {
         send_to_char("Syntax: klog prune time <time/-1>\n\r", ch);
         send_to_char("Syntax: klog prune lines <lines/-1>\n\r", ch);
         send_to_char("Syntax: klog clear now\n\r", ch);
      }
      send_to_char("\n\rhelp klog for more information over the variety of options\n\r", ch);
      return;
   }
   
   if (!str_cmp(argument, "clear now") && (ch->pcdata->caste == caste_King || ch->pcdata->caste >= caste_Staff))
   {
      remove(kingdom_table[ch->pcdata->hometown]->logfile);
      send_to_char_color("Kingdom Log file cleared.\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (!str_cmp(arg1, "prune") && (ch->pcdata->caste == caste_King || ch->pcdata->caste >= caste_Staff))
   {
      if (!str_cmp(arg2, "time"))
      {
         if (atoi(argument) == -1)
         {
            kingdom_table[ch->pcdata->hometown]->maxtimelog = 0;
            send_to_char("Time Pruning has been disabled.\n\r", ch);
            return;
         }
         else
         {
            count = get_prune_time(argument);
            if (count == -1)
            {
               send_to_char("That is improper syntax, help klog for more info.\n\r", ch);
               return;
            }
            kingdom_table[ch->pcdata->hometown]->maxtimelog = count;
            return;
         }
      }
      if (!str_cmp(arg2, "lines"))
      {
         if (atoi(argument) == -1)
         {
            kingdom_table[ch->pcdata->hometown]->maxlinelog = 0;
            send_to_char("Line Pruning has been disabled.\n\r", ch);
            return;
         }
         else
         {
            if (atoi(argument) <= 0 || atoi(argument) > 100000)
            {
               send_to_char("Range is 1 to 100,000\n\r", ch);
               return;
            }
            kingdom_table[ch->pcdata->hometown]->maxlinelog = atoi(argument);
            return;
         }
      }   
   }   
   if (!str_cmp(arg1, "search"))
   {
      argument = one_argument(argument, arg3);
      argument = one_argument(argument, arg4);
      
      if (arg3[0] == '\0' || !str_cmp(arg3, "linecount"))
      {
         if (!str_cmp(arg3, "linecount"))
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, 0, 0, NULL, arg2, 8);   
         else
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, 0, 0, NULL, arg2, 5);   
         return;
      }
      else
      {
         if (!str_cmp(arg3, "time"))
         {
            count = get_klog_time_argument(arg4);
            endline = get_klog_time_argument(argument);
            if (count == -1 || endline == -1)
            {
               send_to_char("That is not proper syntax, please see help klog for more info.\n\r", ch);
                return;
            }
            if (count > endline)
            {
               send_to_char("Start time cannot be after End time.\n\r", ch);
               return;
            }
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, count, endline, NULL, arg2, 6);   
            return;
         }
         else
         {
            if (arg3[0] != '\0')
            {
               count = atoi(arg3);
            }
            if (arg4[0] != '\0')
            {
               endline = atoi(arg4);
            }
            if (endline != 0 && count != 0)
            {
               if (count > endline)
               {
                  send_to_char("start line cannot be greater than end line.\n\r", ch);
                   return;
               }
            }
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, count, endline, NULL, arg2, 7);
            return;
         }
      }
   }
   
   if (!str_cmp(arg1, "type"))
   {
      argument = one_argument(argument, arg3);
      argument = one_argument(argument, arg4);
      
      if ((count = search_logname(arg2)) == KLOG_UNKNOWN)
      {
         send_to_char("That is not a viable catagory.\n\r", ch);
         return;
      }
      count = 0;
      if (arg3[0] == '\0' || !str_cmp(arg3, "linecount"))
      {
         if (!str_cmp(arg3, "linecount"))
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, 0, 0, arg2, NULL, 4);   
         else
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, 0, 0, arg2, NULL, 1);   
         return;
      }
      else
      {
         if (!str_cmp(arg3, "time"))
         {
            count = get_klog_time_argument(arg4);
            endline = get_klog_time_argument(argument);
            if (count == -1 || endline == -1)
            {
               send_to_char("That is not proper syntax, please see help klog for more info.\n\r", ch);
                return;
            }
            if (count > endline)
            {
               send_to_char("Start time cannot be after End time.\n\r", ch);
               return;
            }
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, count, endline, arg2, NULL, 2);   
            return;
         }
         else
         {
            if (arg3[0] != '\0')
            {
               count = atoi(arg3);
            }
            if (arg4[0] != '\0')
            {
               endline = atoi(arg4);
            }
            if (endline != 0 && count != 0)
            {
               if (count > endline)
               {
                  send_to_char("start line cannot be greater than end line.\n\r", ch);
                   return;
               }
            }
            klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, count, endline, arg2, NULL, 3);
            return;
         }
      }
   }     
   if (!str_cmp(arg1, "time"))
   {
      count = get_klog_time_argument(arg2);
      endline = get_klog_time_argument(argument);
      if (count == -1 || endline == -1)
      {
         send_to_char("That is not proper syntax, please see help klog for more info.\n\r", ch);
         return;
      }
      if (count > endline)
      {
         send_to_char("Start time cannot be after End time.\n\r", ch);
         return;
      }
      klog_fullsearch(ch, kingdom_table[ch->pcdata->hometown]->logfile, count, endline, NULL, NULL, 0);
      return;
   }
   if (!str_cmp(arg1, "list"))
   {
      if (!str_cmp(arg2, "linecount"))
      {
         ch_printf(ch, "There are %d lines in your logfile\n\r", klog_linecount(kingdom_table[ch->pcdata->hometown]->logfile));
         return;
      }
      if (arg2[0] != '\0')
      {
         count = atoi(arg2);
      }
      if (argument[0] != '\0')
      {
         endline = atoi(argument);
      }
      if (endline != 0 && count != 0)
      {
         if (count > endline)
         {
            send_to_char("start line cannot be greater than end line.\n\r", ch);
            return;
         }
      }
      klog_showline(ch, kingdom_table[ch->pcdata->hometown]->logfile, count, endline);
      return;
   }
   do_kingdomlog(ch, "");
   return;
}
bool is_made_room(int x, int y, int map, TOWN_DATA *town)
{
   int z;
   
   for (z = 1; z <= 150; z++)
   {
      if (town->roomcoords[z][0] == x && town->roomcoords[z][1] == y && town->roomcoords[z][2] == map)
      {
         return TRUE;
      }
   } 
   return FALSE;
}
bool in_town_range(TOWN_DATA *town, int x, int y, int map)
{
   int size = get_control_size(town->size);
   
   if (x > town->startx + size || x < town->startx - size || town->startmap != map
   ||  y > town->starty + size || y < town->starty - size)
      return FALSE;
      
   return TRUE;
}
   
int get_current_hold(TOWN_DATA *town)
{
   int hold;
  
   hold = town->lumber + town->gold + town->iron;
   hold += town->grain + town->corn + town->stone + town->fish;
   hold += town->coins/100;
   return hold;
}
   
int gettday(int stime) //Gets the days 0 - 359
{
   int day;
   int difftime;

   difftime = stime - sysdata.start_calender;
   day = (difftime / 777600);
   day = difftime - (day * 777600);
   day = day / (777600 / 360);
   return day;
}   

void do_showkingdoms(CHAR_DATA * ch, char *argument)
{
   TOWN_DATA *town;
   int x;
   int first = 0;
   int day;
   char month[10];
   char buf[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot see kingdom stats.\n\r", ch);
      return;
   }
   if (ch->level > LEVEL_PC && !str_cmp(argument, "all"))
   {
      send_to_char
         ("&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&RDEFAULT KINGDOMS&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-\n\r",
         ch);
      send_to_char("&G&WNum  Name             Ruler            DTown\n\r", ch);
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
         if (x == 2) //first real kingdom
         {
            send_to_char
            ("&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&RKINGDOMS&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-\n\r",
            ch);
            send_to_char("&G&WNum  Name             Ruler            DTown\n\r", ch);
         }
         sprintf(buf, "&c&w%2d>  %-15s  %-15s  %s\n\r", x, kingdom_table[x]->name, kingdom_table[x]->ruler, kingdom_table[x]->dtown);
         send_to_char(buf, ch);
      }
      send_to_char
         ("&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&RTOWNS&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-\n\r",
         ch);
      send_to_char("&G&WName             Coins      Gold      Iron      Stone     Corn      Grain     Fish      Lumber    Size    X     Y\n\r", ch);
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
         for (town = kingdom_table[x]->first_town; town; town = town->next)
         {
            sprintf(buf, "&c&w%-15s  %-9d  %-8d  %-8d  %-8d  %-8d  %-8d  %-8d  %-8d  %2d/%-3d  %-4d  %-4d\n\r", town->name,
               town->coins, town->gold, town->iron,
               town->stone, town->corn, town->grain, town->fish, town->lumber, town->size, town->moral, town->startx, town->starty);
            send_to_char(buf, ch);
         }
      }
      return;
   }
   
   if (ch->pcdata->hometown > 1)
   {
      x = ch->pcdata->hometown;
      if (!ch->pcdata->town)
      {
         send_to_char("You are not in a town, your default town is probably screwed up.\n\r", ch);
         return;
      }
      send_to_char
         ("&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&RKINGDOMS&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-\n\r",
         ch);
      send_to_char("&G&WName             Tax       Dtown            Ruler\n\r", ch);
      sprintf(buf, "&c&w%-15s  %-4d      %-15s  %s\n\r", kingdom_table[x]->name, kingdom_table[x]->poptax, 
         kingdom_table[x]->dtown, kingdom_table[x]->ruler);
      send_to_char(buf, ch);   

      if (ch->pcdata->town)
      {
         town = ch->pcdata->town;
         day = gettday(ch->pcdata->town->month);
         sprintf(month, "%s", getmonth(day));
         day = getdayofmonth(day);
         send_to_char("&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&RDEFAULT TOWN&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-\n\r",ch);
         send_to_char("&G&WName             Coins      Gold      Iron       Stone      Corn      Size        Morale  Fish\n\r", ch);
         sprintf(buf, "&c&w%-15s  %-9d  %-8d  %-8d   %-8d   %-8d  %-2d          %-2d     %-8d\n\r", town->name,
            town->coins, town->gold, town->iron, town->stone, town->corn, town->size, town->moral, town->fish);
         send_to_char(buf, ch);  
         send_to_char("&G&W                 Grain      Lumber    Recall     Death      Tax       Mayor       GMonth\n\r", ch);
         sprintf(buf, "&c&w                 %-8d   %-8d  %-4d.%-4d  %-4d.%-4d  %-3d       %-10.10s  %-3s %-2d\n\r",
            town->grain, town->lumber, town->recall[0], town->recall[1], town->death[0], town->death[1], town->poptax, 
            town->mayor, month, day);
         send_to_char(buf, ch);
      }
      for (town = kingdom_table[ch->pcdata->hometown]->first_town; town; town = town->next)
      {
         if (ch->pcdata->town && town == ch->pcdata->town)
            continue;
         if (first == 0)
         {
            send_to_char("&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&ROTHER TOWNS&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*\n\r", ch);
            first=1;
         }
         day = gettday(town->month);
         sprintf(month, "%s", getmonth(day));
         day = getdayofmonth(day);
         send_to_char("&G&WName             Coins      Gold      Iron       Stone      Corn      Size        Morale  Fish\n\r", ch);
         sprintf(buf, "&c&w%-15s  %-9d  %-8d  %-8d   %-8d   %-8d  %-2d          %-2d      %-8d\n\r", town->name,
            town->coins, town->gold, town->iron, town->stone, town->corn, town->size, town->moral, town->fish);
         send_to_char(buf, ch);  
         send_to_char("&G&W                 Grain      Lumber    Recall     Death      Tax       Mayor       GMonth\n\r", ch);
         sprintf(buf, "&c&w                 %-8d   %-8d  %-4d.%-4d  %-4d.%-4d  %-3d       %-10.10s  %-3s %-2d\n\r",
            town->grain, town->lumber, town->recall[0], town->recall[1], town->death[0], town->death[1], town->poptax, 
            town->mayor, month, day);
         send_to_char(buf, ch);
      }
      return;
   }
   if (ch->pcdata->hometown <= 1)
   {
      send_to_char
         ("&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&RKINGDOMS&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-&C*&c-\n\r",
         ch);
      send_to_char("&G&WNum  Name        Tax  Dtown            Ruler\n\r", ch);
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
         sprintf(buf, "&c&w%d    %-10s  %-4d %-15s  %s\n\r", x, kingdom_table[x]->name, kingdom_table[x]->poptax, 
            kingdom_table[x]->dtown, kingdom_table[x]->ruler);
         send_to_char(buf, ch);
      }
      return;
   }
   return;
}

#define MAX_SET_RESOURCE 1000000 //simpler this way

//Used to set stuff in a kingdom or a town.  Will probably have most of the
//commands you cannot do with townvalues/castevalues like setting resources
void do_setkingdom(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   TOWN_DATA *town;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: setkingdom town <name of town> [resource] [amount]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> size [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> morale [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> poptax [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> units [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> training [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> maxhold [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> rooms [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> banksize [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> expansions [value]\n\r", ch);
      send_to_char("Syntax: setkingdom town <name of town> popunit [training list number]\n\r", ch);
      send_to_char("Syntax: setkingdom kingdom <name of kingdom> poptax [value]\n\r", ch);
      send_to_char("resource - gold iron stone corn grain lumber coins fish\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   if (!str_cmp(arg1, "kingdom"))
   {
      int x;
      for (x = 2; x < sysdata.max_kingdom; x++)
      {
         if (!str_cmp(arg2, kingdom_table[x]->name))
            break;
      }
      if (x == sysdata.max_kingdom)
      {
         send_to_char("That is not an actual kingdom, use showkingdoms all for a list of kingdoms.\n\r", ch);
         return;
      }
      if (!str_cmp("poptax", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > 300)
         {
            send_to_char("Range is 0 to 300.\n\r", ch);
            return;
         }
         kingdom_table[x]->poptax = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(x);
         return;
      }
   }
   if (!str_cmp(arg1, "town"))
   {
      if ((town = get_town(arg2)) == NULL)
      {
         send_to_char("That is not an actual town, use showkindoms all for a list of towns.\n\r", ch);
         return;
      }
      if (!str_cmp("expansions", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > 4)
         {
            send_to_char("The range is 0 to 4.\n\r", ch);
            return;
         }
         town->expansions = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("popunit", arg3))
      {
         TRAINING_DATA *training;
         int cnt = 0;
         
         if (argument[0] == '\0' || atoi(argument) <= 0)
         {
            send_to_char("You need to specify the unit's number in the training list for that town.\n\r", ch);
            return;
         }
         for (training = first_training; training; training = training->next)
         {
            if (training->kingdom == town->kingdom)
            {
               cnt++;
               if (atoi(argument) == cnt)
               {
                  if (training->speed == 0)
                     training->stime = training->stime - (TRAINING_TIME/2);
                  else
                     training->stime = training->stime - (training->speed*TRAINING_TIME);   
                  send_to_char("Done.\n\r", ch);
                  return;
               }
            }
         }
         send_to_char("Invalid number.\n\r", ch);
         return;
      }         
      if (!str_cmp("rooms", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > 150)
         {
            ch_printf(ch, "Range is 0 to 150\n\r");
            return;
         }
         town->rooms = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("maxhold", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE*10)
         {
            ch_printf(ch, "Range is 0 to %d\n\r", MAX_SET_RESOURCE*10);
            return;
         }
         town->hold = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("banksize", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > 200000)
         {
            send_to_char("Range is 0 to 200,000.\n\r", ch);
            return;
         }
         town->banksize = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
    
      if (!str_cmp("training", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > 100)
         {
            send_to_char("Range is 0 to 100.\n\r", ch);
            return;
         }
         town->unitstraining = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("poptax", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > 300)
         {
            send_to_char("Range is 0 to 300.\n\r", ch);
            return;
         }
         town->poptax = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("morale", arg3))
      {
         if (atoi(argument) < 1 || atoi(argument) > 183)
         {
            send_to_char("Range is 1 to 183.\n\r", ch);
            return;
         }
         town->moral = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("size", arg3))
      {
         int x, y, size;
         if (atoi(argument) < 1 || atoi(argument) > 10)
         {
            send_to_char("Range is 1 to 10 for size.\n\r", ch);
            return;
         }
         size = get_control_size(atoi(argument));
         //Check to see if you can grow first....
         for (x = town->startx - size; x <= town->startx+size; x++)
         {
            if (x > town->startx)
            {
               for (y = town->starty - abs(x - town->startx - size); y <= town->starty + abs(x - town->startx - size); y++)
               {
                  if (kingdom_sector[town->startmap][x][y] > 1 && kingdom_sector[town->startmap][x][y] != town->kingdom)
                  {
                     send_to_char("That would conflict with someone else's AOC, cannot do that.\n\r", ch);
                     return;
                  }
               }
            }
            else
            {   
               for (y = town->starty - abs(x - town->startx + size); y <= town->starty + abs(x - town->startx + size); y++)
               {
                  if (kingdom_sector[town->startmap][x][y] > 1 && kingdom_sector[town->startmap][x][y] != town->kingdom)
                  {
                     send_to_char("That would conflict with someone else's AOC, cannot do that.\n\r", ch);
                     return;
                  }
               }
            }
         }
         size = get_control_size(town->size);
         for (x = town->startx - size; x <= town->startx+size; x++)
         {
            for (y = town->starty - size; y <= town->starty+size; y++)
            {
               kingdom_sector[town->startmap][x][y] = 0;
            }
         } 
         size = get_control_size(atoi(argument));
         town->size = atoi(argument);
         for (x = town->startx - size; x <= town->startx+size; x++)
         {
            for (y = town->starty - size; y <= town->starty+size; y++)
            {
               kingdom_sector[town->startmap][x][y] = town->kingdom;
            }
         } 
         town->maxsize = totalrooms(town->size);
         if (town->size == 5 || town->size == 6)
            town->expansions = 1;
         if (town->size == 7 || town->size == 8)
            town->expansions = 2;
         if (town->size == 9)
            town->expansions = 3;
         if (town->size == 10)
            town->expansions = 4;    
         town->banksize = get_banksize(town->size);
         send_to_char("Max Rooms and AOC changed, you still need to set morale (help morale).\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("gold", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE);
            return;
         }
         town->gold = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("iron", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE);
            return;
         }
         town->iron = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("stone", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE);
            return;
         }
         town->stone = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("fish", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE);
            return;
         }
         town->fish= atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("grain", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE);
            return;
         }
         town->grain = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("corn", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE);
            return;
         }
         town->corn = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("lumber", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE);
            return;
         }
         town->lumber = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
      if (!str_cmp("coins", arg3))
      {
         if (atoi(argument) < 0 || atoi(argument) > MAX_SET_RESOURCE*100)
         {
            ch_printf(ch, "Invalid Range, the range is 0 to %d\n\r", MAX_SET_RESOURCE*100);
            return;
         }
         town->coins = atoi(argument);
         send_to_char("Done.\n\r", ch);
         write_kingdom_file(town->kingdom);
         return;
      }
   }
   do_setkingdom(ch, "");
   return;
}
           
void do_cityvalues(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MSL];
   int day;
   char month[10];
   char fcon[10], scon[10], lcon[10], ccon[10];

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobs.\n\r", ch);
      return;
   }
   if (ch->pcdata->town == NULL)
   {
      send_to_char("You do not belong to a town!\n\r", ch);
      return;
   }
   if (str_cmp(ch->pcdata->town->mayor, ch->name)
      && ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mintoperate)
   {
      send_to_char("This command is for City Owners or people with rights from the Kingdom.\n\r", ch);
      return;
   }
   if (ch->pcdata->town->foodconsump == 0)
      sprintf(fcon, "Double");
   else if (ch->pcdata->town->foodconsump == 1)
      sprintf(fcon, "Single");
   else
      sprintf(fcon, "None");
      
   if (ch->pcdata->town->stoneconsump == 0)
      sprintf(scon, "Double");
   else if (ch->pcdata->town->stoneconsump == 1)
      sprintf(scon, "Single");
   else
      sprintf(scon, "None");
     
   if (ch->pcdata->town->lumberconsump == 0)
      sprintf(lcon, "Double");
   else if (ch->pcdata->town->lumberconsump == 1)
      sprintf(lcon, "Single");
   else
      sprintf(lcon, "None");
      
   if (ch->pcdata->town->coinconsump == 0)
      sprintf(ccon, "Double");
   else if (ch->pcdata->town->coinconsump == 1)
      sprintf(ccon, "Single");
   else
      sprintf(ccon, "None");
      
   day = gettday(ch->pcdata->town->month);
   sprintf(month, "%s", getmonth(day));
   day = getdayofmonth(day);
   if (argument[0] == '\0')
   {
      ch_printf(ch, "Name             Mayor          Recall      Death       MinHighAppnt  SalesTax     PopTax\n\r");
      ch_printf(ch, "%-15s  %-13s  %-4d.%-4d   %-4d.%-4d   %-2d            %-3d          %-3d\n\r",
         ch->pcdata->town->name, ch->pcdata->town->mayor, ch->pcdata->town->recall[0],
         ch->pcdata->town->recall[1], ch->pcdata->town->death[0], ch->pcdata->town->death[1],
         ch->pcdata->town->minhappoint, ch->pcdata->town->salestax, ch->pcdata->town->poptax);
      ch_printf(ch, "\n\r                 Size           StartPnt    GMonth      Morale        Rooms        MaxRooms\n\r");   
      ch_printf(ch, "                 %-2d             %-4d.%-4d   %-3s %-2d      %-2d            %-3d          %-3d\n\r",
         ch->pcdata->town->size, ch->pcdata->town->startx, ch->pcdata->town->starty, 
         month, day, ch->pcdata->town->moral, ch->pcdata->town->rooms,
         ch->pcdata->town->maxsize);
      ch_printf(ch, "\n\r                 Barracks       Coins       Gold        Iron          Stone        Lumber\n\r");
      ch_printf(ch, "                 %-4d.%-4d      %-10d  %-10d  %-10d    %-8d     %-10d\n\r",
         ch->pcdata->town->barracks[0], ch->pcdata->town->barracks[1], ch->pcdata->town->coins, ch->pcdata->town->gold,
         ch->pcdata->town->iron, ch->pcdata->town->stone, ch->pcdata->town->lumber);
      ch_printf(ch, "\n\r                 Corn           Grain       Used Hold   Max Hold      Expansion    Expansions\n\r");
      ch_printf(ch, "                 %-10d     %-10d  %-10d  %-10d    %-3s          %d\n\r",
         ch->pcdata->town->corn, ch->pcdata->town->grain, get_current_hold(ch->pcdata->town), ch->pcdata->town->hold,
         ch->pcdata->town->allowexpansions ? "Yes" : "No", ch->pcdata->town->expansions);   
      ch_printf(ch, "\n\r                 Units          Training    Bank Used   Bank Size     MinWithdraw  Fish\n\r");
      ch_printf(ch, "                 %2d/%-2d          %-2d/%2d       %-10d  %-10d    %-2d           %-8d\n\r", 
         get_kingdom_units(ch->pcdata->town->tpid), max_allowedunits(ch->pcdata->town->size),
         ch->pcdata->town->unitstraining, get_maxunits(ch->pcdata->town->size),
         get_townbank_weight(ch->pcdata->town), ch->pcdata->town->banksize, ch->pcdata->town->minwithdraw, ch->pcdata->town->fish );   
      ch_printf(ch, "\n\r                                  Consumption");       
      ch_printf(ch, "\n\r                 Food           Lumber      Stone       Coins\n\r");
      ch_printf(ch, "                 %-6s         %-6s      %-6s      %-6s\n\r", 
         fcon, lcon, scon, ccon);
      return;
         
   }
   sprintf(buf, "%s - townvalues %s", PERS_KINGDOM(ch, ch->pcdata->hometown), argument);
   write_kingdom_logfile(ch->pcdata->hometown,  buf, KLOG_TOWNVALUES);  
   argument = one_argument(argument, arg);
   
   if (!str_cmp(arg, "Food"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Food Consumption.  Arguments:  Single, Double, None\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "Single"))
      {
         ch->pcdata->town->foodconsump = 1;
         send_to_char("Set to Single.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else if (!str_cmp(argument, "Double"))
      {
         ch->pcdata->town->foodconsump = 0;
         send_to_char("Set to Double.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else if (!str_cmp(argument, "None"))
      {
         ch->pcdata->town->foodconsump = 2;
         send_to_char("Set to None.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else
      {
         send_to_char("That is not a choice.  Your choices are: Single, Double, None\n\r", ch);
         return;
      }
      return;
   }
   if (!str_cmp(arg, "Coins"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Food Consumption.  Arguments:  Single, Double, None\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "Single"))
      {
         ch->pcdata->town->coinconsump = 1;
         send_to_char("Set to Single.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else if (!str_cmp(argument, "Double"))
      {
         ch->pcdata->town->coinconsump = 0;
         send_to_char("Set to Double.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else if (!str_cmp(argument, "None"))
      {
         ch->pcdata->town->coinconsump = 2;
         send_to_char("Set to None.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else
      {
         send_to_char("That is not a choice.  Your choices are: Single, Double, None\n\r", ch);
         return;
      }
      return;
   }
   if (!str_cmp(arg, "Stone"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Stone Consumption.  Arguments:  Single, Double, None\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "Single"))
      {
         ch->pcdata->town->stoneconsump = 1;
         send_to_char("Set to Single.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else if (!str_cmp(argument, "Double"))
      {
         ch->pcdata->town->stoneconsump = 0;
         send_to_char("Set to Double.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else if (!str_cmp(argument, "None"))
      {
         ch->pcdata->town->stoneconsump = 2;
         send_to_char("Set to None.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else
      {
         send_to_char("That is not a choice.  Your choices are: Single, Double, None\n\r", ch);
         return;
      }
      return;
   }
   if (!str_cmp(arg, "Lumber"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Lumber Consumption.  Arguments:  Single, Double, None\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "Single"))
      {
         ch->pcdata->town->lumberconsump = 1;
         send_to_char("Set to Single.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      if (!str_cmp(argument, "Double"))
      {
         ch->pcdata->town->lumberconsump = 0;
         send_to_char("Set to Double.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      if (!str_cmp(argument, "None"))
      {
         ch->pcdata->town->lumberconsump = 2;
         send_to_char("Set to None.\n\r", ch);
         write_kingdom_file(ch->pcdata->hometown);
         return;
      }
      else
      {
         send_to_char("That is not a choice.  Your choices are: Single, Double, None\n\r", ch);
         return;
      }
      return;
   }
   
   if (!str_prefix(arg, "minhighappnt"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the High Appointment Value.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      ch->pcdata->town->minhappoint = atoi(argument);
      send_to_char("Value for High Appointment has been changed.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
   if (!str_prefix(arg, "minwithdraw"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Minimum caste to withdraw Value.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      ch->pcdata->town->minwithdraw = atoi(argument);
      send_to_char("Value for minimum withdraw has been changed.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }

   if (!str_prefix(arg, "name"))
   {
      CONQUER_DATA *conquer;
      if (argument[0] == '\0')
      {
         send_to_char("Used to change the name of the town, max is 15 characters.\n\r", ch);
         return;
      }
      if (strlen(argument) > 15)
      {
         send_to_char("Name can only be up to 15 characters.\n\r", ch);
         return;
      }
      for (conquer = first_conquer; conquer; conquer = conquer->next)
      {
         if (conquer->town == ch->pcdata->town)
         {
            send_to_char("Your town is in the process of being occupied, you cannot change its name.\n\r", ch);
            return;
         }
      } 
      STRFREE(ch->pcdata->town->name);
      ch->pcdata->town->name = STRALLOC(argument);
      write_kingdom_file(ch->pcdata->hometown);
      send_to_char("City Name changed.\n\r", ch);
      return;
   }
   if (!str_prefix(arg, "Mayor"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Used to change the mayor/owner of the town.\n\r", ch);
         return;
      }
      STRFREE(ch->pcdata->town->mayor);
      ch->pcdata->town->mayor = STRALLOC(argument);
      send_to_char("The Mayor/Onwer of the city has now changed hands.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
   if (!str_cmp(arg, "expansion"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Either choose yes or no to allow new towns to expand from you.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "yes"))
      {
         ch->pcdata->town->allowexpansions = 1;
      }
      else if (!str_cmp(argument, "no"))
      {
         ch->pcdata->town->allowexpansions = 0;
      }
      else
      {
         send_to_char("Either choose yes or no to allow new towns to expand from you.\n\r", ch);
         return;
      }
      send_to_char("Allowing of Expansion for the town has changed.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
      
   if (!str_prefix(arg, "Salestax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("The salestax on goods, 70 is 7.0 and 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 300)
      {
         send_to_char("Range is 0 to 300.\n\r", ch);
         return;
      }
      ch->pcdata->town->salestax = atoi(argument);
      send_to_char("Salestax for the town has changed.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
   if (!str_cmp(arg, "poptax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("The tax on the population, 70 is 7.0 and 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 300)
      {
         send_to_char("Range is 0 to 300.\n\r", ch);
         return;
      }
      ch->pcdata->town->poptax = atoi(argument);
      send_to_char("Poptax for the town has changed.\n\r", ch);
      ch->pcdata->town->lasttaxchange = time(0);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
   if (!str_prefix(arg, "barracks"))
   {
      if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)
      {
         send_to_char("You can only set a barracks spot in your town in your kingdom.\n\r", ch);
         return;
      }
      if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
      {
         send_to_char("You can only set a barracks spot in your town only.\n\r", ch);
         return;
      }
      ch->pcdata->town->barracks[0] = ch->coord->x;
      ch->pcdata->town->barracks[1] = ch->coord->y;
      ch->pcdata->town->barracks[2] = ch->map;
      send_to_char("Barracks spot changed to where you are standing.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
   if (!str_prefix(arg, "recall"))
   {
      if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)
      {
         send_to_char("You can only set a recall spot in your town in your kingdom.\n\r", ch);
         return;
      }
      if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
      {
         send_to_char("You can only set a recall spot in your town only.\n\r", ch);
         return;
      }
      ch->pcdata->town->recall[0] = ch->coord->x;
      ch->pcdata->town->recall[1] = ch->coord->y;
      ch->pcdata->town->recall[2] = ch->map;
      send_to_char("Recall spot changed to where you are standing.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
   if (!str_prefix(arg, "death"))
   {
      if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)
      {
         send_to_char("You can only set a death spot in your town in your kingdom.\n\r", ch);
         return;
      }
      if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
      {
         send_to_char("You can only set a death spot in your town only.\n\r", ch);
         return;
      }
      ch->pcdata->town->death[0] = ch->coord->x;
      ch->pcdata->town->death[1] = ch->coord->y;
      ch->pcdata->town->death[2] = ch->map;
      send_to_char("Death spot changed to where you are standing.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      return;
   }
   do_cityvalues(ch, "");
   return;
}

void do_logsettings(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   KINGDOM_DATA *k;
   int cnt;
   int x;
   
   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobs.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minlogsettings)
   {
      send_to_char("You do not have enough rank to use this command.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: logsettings list\n\r", ch);
      send_to_char("Syntax: logsettings <field> <log|nolog>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   k = kingdom_table[ch->pcdata->hometown];
   
   if (!str_cmp(arg, "list"))
   {
      cnt = x = 0;
      for (;;)
      {
         if (x == KLOG_LASTENTRY)
            break;
 
         ch_printf(ch, "&G&W%-15s  &G&R[&c&C%s&G&R]&G&W  ", return_logname(x), xIS_SET(k->logsettings, x) ? " " : "X");
         x++;
         if (++cnt == 4)
         {
            cnt = 0;
            ch_printf(ch, "\n\r");
         }
      }
      if (cnt != 0)
         send_to_char("\n\r", ch);
      send_to_char("\n\rUse logsettings <field> <log|nolog> to change a field\n\r", ch);
      return;
   }
   
   x = search_logname(arg);
   
   if (x == KLOG_UNKNOWN)
   {
      send_to_char("Either your selection was not found or you are trying to toggle Unkown, which you cannot do.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "log"))
      xREMOVE_BIT(k->logsettings, x);
   else if (!str_cmp(argument, "nolog"))
      xSET_BIT(k->logsettings, x);
   else
   {
      send_to_char("Either log or nolog.\n\r", ch);
      return;
   }
   write_kingdom_file(ch->pcdata->hometown);
   send_to_char("Done.\n\r", ch);
   return;
}

char *parse_logtime(int tlog)
{
   static char buf[MSL];
   
   int second, minute, hour, day;
   
   second=minute=hour=day=0;
   
   if (tlog >= 86400)
      day = tlog / 86400;
   if (tlog >= 3600)
      hour = (tlog / 3600) % 24;
   if (tlog >= 60)
      minute = (tlog / 60) % 60;
   
   second = tlog % 60;
      
   sprintf(buf, "%dd %dh %dm %ds", day, hour, minute, second);
   return buf;
}

void do_castevalues(CHAR_DATA * ch, char *argument)
{
   int ht;
   TOWN_DATA *town;
   char arg[MIL];
   char buf[MSL];
   int x;
   int y = 0;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobs.\n\r", ch);
      return;
   }
   if (ch->pcdata->caste != caste_King)
   {
      if (ch->pcdata->caste < caste_Staff 
      && (!kingdom_table[ch->pcdata->hometown]->number1 || str_cmp(kingdom_table[ch->pcdata->hometown]->number1, ch->name))
      && (!kingdom_table[ch->pcdata->hometown]->number2 || str_cmp(kingdom_table[ch->pcdata->hometown]->number2, ch->name)))
      {
         send_to_char("This command is for Kings, Princes, Ministers, or Staff level immortals.\n\r", ch);
         return;
      }
   }
   ht = ch->pcdata->hometown;
   if (ht == -1)
   {
      send_to_char("You need to pick a Kingdom First.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      KINGDOM_DATA *k = kingdom_table[ht];
      
      ch_printf(ch, "&G&WName        Ruler          Race           AllowJoin  Number1        Number2        DTown\n\r");
      ch_printf(ch, "&Y%-10s  %-13s  %-13s  %-3s        %-13s  %-13s  %s\n\r\n\r", 
         k->name, k->ruler, print_npc_race(k->race), k->allowjoin == 1 ? "Yes" : "No", k->number1 ? k->number1 : "No One", k->number2 ? k->number2 : "No One", k->dtown ? k->dtown : "None");
         
      ch_printf(ch, "            &G&WMinBuild  MinPlace  MinAppoint  MinHighAppnt  MinWithdraw  MinCommand  MinReadLog\n\r");
      ch_printf(ch, "            &c&Y%-2d        %-2d        %-2d          %-2d            %-2d           %-2d          %-2d\n\r\n\r",
         k->minbuild, k->minplace, k->minappoint, k->minhappoint, k->minwithdraw, k->mincommand, k->minreadlog);
         
      ch_printf(ch, "            &G&WMinSwitchTown  MinTOperate  MinDepository  MinLogSettings  MinBookTax  MinTrainerTax\n\r");
      ch_printf(ch, "            &c&Y%-2d             %-2d           %-2d             %-2d              %-2d          %-2d\n\r\n\r", 
        k->minswitchtown, k->mintoperate, k->mindepository, k->minlogsettings, k->minbooktax, k->mintrainertax);          
        
      ch_printf(ch, "            &G&WMinGeneral  LogTime           LogLines  FishTax\n\r");
      ch_printf(ch, "            &c&Y%-2d          %-16s  %-6d    %-4d\n\r\n\r",
        k->mingeneral, parse_logtime(k->maxtimelog), k->maxlinelog, k->fish_tax);
      
      ch_printf(ch, "            &G&WSalestax  PopTax  TreeTax  CornTax  GrainTax  StoneTax  IronTax  GoldTax\n\r", ch);
      ch_printf(ch, "            &c&Y%-4d      %-4d    %-4d     %-4d     %-4d      %-4d      %-4d     %-4d\n\n\n\r", 
        k->salestax, k->poptax, k->tree_tax, k->corn_tax, k->grain_tax, k->stone_tax, k->iron_tax, k->gold_tax);
        
      ch_printf(ch, "            &G&WTier1TrainerTax  Tier2TrainerTax  Tier3TrainerTax  Tier4TrainerTax  VisitorTrainerTax\n\r", ch);
      ch_printf(ch, "            &c&Y%-3d              %-3d              %-3d              %-3d              %-4d\n\r\n\r",
        k->tier1, k->tier2, k->tier3, k->tier4, k->tvisitor);
        
      ch_printf(ch, "            &w&WTier1BookTax  Tier2BookTax  Tier3BookTax  Tier4BookTax  VisitorBookTax\n\r", ch);
      ch_printf(ch, "            &c&Y%-3d           %-3d           %-3d           %-3d           %-4d\n\r\n\r",
        k->tier1book, k->tier2book, k->tier3book, k->tier4book, k->bvisitor);
        
      ch_printf(ch,
         "&c&wCity             Size     City             Size     City             Size  \n\r----------------------------------------------------------------------------\n\r");
      for (town = k->first_town; town; town = town->next)
      {
         ch_printf(ch, "&O%-15s  %-2d       ", town->name, town->size);
         y++;
         if (y % 3 == 0)
            ch_printf(ch, "\n\r");
      }
      if (y % 3 != 0)
         ch_printf(ch, "\n\r");
      ch_printf(ch, "\n\r");
      ch_printf(ch,
         "&c&wKingdom          Status   Kingdom          Status   Kingdom          Status   \n\r----------------------------------------------------------------------------\n\r");
      y = 0;
      for (x = 0; x < sysdata.max_kingdom; x++)
      {
         if (ch->pcdata->hometown == x)
            continue;

         ch_printf(ch, "&c&w%-15s  %s%-7s  ", kingdom_table[x]->name,
            peacecolor[kingdom_table[ht]->peace[x]], peacestatus[kingdom_table[ht]->peace[x]]);
         y++;
         if (y % 3 == 0)
            ch_printf(ch, "\n\r");
      }
      if (y % 3 != 0)
         ch_printf(ch, "\n\r");
      ch_printf(ch, "\n\r");
      do_offers(ch, "");
      ch_printf(ch, "\n\r");
      do_offered(ch, "");
      ch_printf(ch, "\n\r");
      ch_printf(ch, "&c&wYou can use this command to set the fields listed above. (Use lower case)\n\r", ch);
      return;
   }
   sprintf(buf, "%s - castevalues %s", PERS_KINGDOM(ch, ch->pcdata->hometown), argument);
   write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_CASTEVALUES);  
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "help"))
   {
      send_to_char("Syntax: castevalues   or    Syntax: castevalues <option> <setting>\n\r", ch);
      send_to_char("Options: name, salestax, minbuild, minplace, allowjoin, poptax, mintoperate\n\r", ch);
      send_to_char("Options: minappoint, minhighappnt, dtown, minwithdraw, mincommand, minreadlog\n\r", ch);
      send_to_char("Options: minlogsettings, race, mindepository, mingeneral\n\r", ch); 
      send_to_char("tier1trainertax, tier2trainertax, tier3trainertax, tier4trainertax, visitortrainertax\n\r", ch);
      send_to_char("tier1booktax, tier2booktax, tier3booktax, tier4booktax, visitorbooktax\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "treetax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Lumber intake.  70 is 7.0, 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1000)
      {
         send_to_char("Range is 0 to 1000.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tree_tax = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "goldtax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Gold intake.  70 is 7.0, 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1000)
      {
         send_to_char("Range is 0 to 1000.\n\r", ch);
         return;
      }
      kingdom_table[ht]->gold_tax = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "irontax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Iron intake.  70 is 7.0, 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1000)
      {
         send_to_char("Range is 0 to 1000.\n\r", ch);
         return;
      }
      kingdom_table[ht]->gold_tax = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "corntax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Corn intake.  70 is 7.0, 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1000)
      {
         send_to_char("Range is 0 to 1000.\n\r", ch);
         return;
      }
      kingdom_table[ht]->corn_tax = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "graintax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Grain intake.  70 is 7.0, 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1000)
      {
         send_to_char("Range is 0 to 1000.\n\r", ch);
         return;
      }
      kingdom_table[ht]->grain_tax = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "stonetax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Stone intake.  70 is 7.0, 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1000)
      {
         send_to_char("Range is 0 to 1000.\n\r", ch);
         return;
      }
      kingdom_table[ht]->stone_tax = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "fishtax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Fish intake.  70 is 7.0, 75 is 7.5, etc.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1000)
      {
         send_to_char("Range is 0 to 1000.\n\r", ch);
         return;
      }
      kingdom_table[ht]->fish_tax = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "tier1trainertax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier1 Trainers.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier1 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "tier2trainertax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier2 Trainers.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier2 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "tier3trainertax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier3 Trainers.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier3 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "tier4trainertax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier4 Trainers.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier4 = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "visitortrainertax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on non-kingdom trainer users.  Max is 1500 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 100 || atoi(argument) > 1500)
      {
         send_to_char("Range is 100 to 1500.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tvisitor = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg, "tier1booktax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier1 Books.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier1book = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "tier2booktax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier2 Books.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier2book = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "tier3booktax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier3 Books.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier3book = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "tier4booktax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on Tier4 Books.  Max is 100 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 100)
      {
         send_to_char("Range is 0 to 100.\n\r", ch);
         return;
      }
      kingdom_table[ht]->tier4book = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "visitorbooktax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Tax on non-kingdom book purchasers.  Max is 1500 percent.\n\r", ch);
         return;
      }
      if (atoi(argument) < 100 || atoi(argument) > 1500)
      {
         send_to_char("Range is 100 to 1500.\n\r", ch);
         return;
      }
      kingdom_table[ht]->bvisitor = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "default town") || !str_cmp(arg, "dtown"))
   {

      if (argument[0] == '\0')
      {
         send_to_char("Used to change the default town.\n\r", ch);
         return;
      }
      for (town = kingdom_table[ch->pcdata->hometown]->first_town; town; town = town->next)
      {
         if (!str_cmp(town->name, argument))
            break;
      }
      if (!town)
      {
         send_to_char("That is not a name of a city (case sensitive).\n\r", ch);
         return;
      }
      if (town->kingdom != ch->pcdata->hometown)
      {
         send_to_char("That city is not in your kingdom, you cannot do that.\n\r", ch);
         return;
      }
      STRFREE(kingdom_table[ht]->dtown);
      kingdom_table[ht]->dtown = STRALLOC(argument);
      send_to_char("Your Default Town has been changed.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "name"))
   {
      int p;

      if (argument[0] == '\0')
      {
         send_to_char("Sets the kingdom name, provide the name and keep it 20 characters or less.\n\r", ch);
         return;
      }
      if (strlen(argument) > 20)
      {
         send_to_char("Your kingdom name cannot be more than 20 characters.\n\r", ch);
         return;
      }
      for (p = 0; p < sysdata.last_portal; p++)
      {
         sprintf(buf, "%s Portal", kingdom_table[ht]->name);
         if (!str_cmp(portal_show[p]->desc, buf)) ;
         break;
      }
      if (p == sysdata.last_portal)
      {
         bug("Kingdom %s does not have a portal.", kingdom_table[ht]->name);
      }
      else
      {
         STRFREE(portal_show[p]->desc);
         sprintf(buf, "%s Portal", argument);
         portal_show[p]->desc = STRALLOC(argument);
         write_portal_file();
      }
      STRFREE(kingdom_table[ht]->name);
      kingdom_table[ht]->name = STRALLOC(argument);
      write_kingdom_file(ht);
      write_kingdom_list();
      send_to_char("Name of the Kingdom changed.\n\r", ch);
      return;
   }
   /*if (!str_cmp(arg, "recall"))
      {
      if ( argument[0] == '\0' )
      {
      send_to_char("Sets the recall spot, must use the vnum (can get that with roomstat).\n\r", ch);
      return;
      }
      troom = get_room_index(atoi(argument));
      if (troom && troom->area->kingdom == ht)
      {
      kingdom_table[ht]->recall = atoi(argument);
      send_to_char("Recall spot changed.\n\r", ch);
      return;
      }
      else
      {
      send_to_char("This area does not belong to you, sorry you cannot do that.\n\r", ch);
      return;
      }
      }
      if (!str_cmp(arg, "death"))
      {
      if ( argument[0] == '\0' )
      {
      send_to_char("Sets the death spot, must use the vnum (can get that with roomstat).\n\r", ch);
      return;
      }
      troom = get_room_index(atoi(argument));
      if (troom && troom->area->kingdom == ht)
      {
      kingdom_table[ht]->death = atoi(argument);
      send_to_char("Death spot changed.\n\r", ch);
      return;
      }
      else
      {
      send_to_char("This area does not belong to you, sorry you cannot do that.\n\r", ch);
      return;
      }
      } */
   if (!str_cmp(arg, "race"))
   {
      if (kingdom_table[ht]->raceset == 1)
      {
         send_to_char("Your race has already been set.\n\r", ch);
         return;
      }
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Race base of your kingdom, accepts only pc races.\n\r", ch);
         return;
      }
      if (isdigit(argument[0]))
      {
         if (atoi(argument) < 0 || atoi(argument) >= MAX_RACE)
         {
            ch_printf(ch, "Range is 0 to %d", MAX_RACE-1);
            return;
         }
         kingdom_table[ht]->race = atoi(argument);
         kingdom_table[ht]->raceset = 1;
         send_to_char("Your race base has been set.\n\r", ch);
         return;
      }
      else
      {
         if (!str_cmp(argument, "human"))
         {
            kingdom_table[ht]->race = 0;
            kingdom_table[ht]->raceset = 1;
            send_to_char("Your race base has been set.\n\r", ch);
            return;   
         }
         if (!str_cmp(argument, "elf"))
         {
            kingdom_table[ht]->race = 1;
            kingdom_table[ht]->raceset = 1;
            send_to_char("Your race base has been set.\n\r", ch);
            return;   
         }
         if (!str_cmp(argument, "dwarf"))
         {
            kingdom_table[ht]->race = 2;
            kingdom_table[ht]->raceset = 1;
            send_to_char("Your race base has been set.\n\r", ch);
            return;   
         }
         if (!str_cmp(argument, "ogre"))
         {
            kingdom_table[ht]->race = 3;
            kingdom_table[ht]->raceset = 1;
            send_to_char("Your race base has been set.\n\r", ch);
            return;   
         }
         if (!str_cmp(argument, "hobbit"))
         {
            kingdom_table[ht]->race = 4;
            kingdom_table[ht]->raceset = 1;
            send_to_char("Your race base has been set.\n\r", ch);
            return;   
         }
         if (!str_cmp(argument, "fairy"))
         {
            kingdom_table[ht]->race = 5;
            kingdom_table[ht]->raceset = 1;
            send_to_char("Your race base has been set.\n\r", ch);
            return;   
         }
      }
      send_to_char("Invalid selection.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "poptax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Population tax rate.  75 is 7.5 percent, max is 300 (30 percent).\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 300)
      {
         send_to_char("Range is 0 to 300\n\r", ch);
         return;
      }
      kingdom_table[ht]->poptax = atoi(argument);
      kingdom_table[ht]->lasttaxchange = time(0);
      send_to_char("Population tax set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "salestax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the salestax rate.  75 is 7.5 percent, max is 300 (30 percent).\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 300)
      {
         send_to_char("Range is 0 to 300\n\r", ch);
         return;
      }
      kingdom_table[ht]->salestax = atoi(argument);
      send_to_char("Sales tax set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minbooktax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to buy books tax free.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minbooktax = atoi(argument);
      send_to_char("Minimum requirement for tax free books are set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "mintrainertax"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to use trainers tax free.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->mintrainertax = atoi(argument);
      send_to_char("Minimum requirement for tax free trainers are set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "mindepository"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to take from the depository.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->mindepository = atoi(argument);
      send_to_char("Minimum requirement for depository use is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minswitchtown"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to join any town.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minswitchtown = atoi(argument);
      send_to_char("Minimum requirement for switching towns is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minbuild"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to Build.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minbuild = atoi(argument);
      send_to_char("Minimum requirement for build use is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "toperate") || !str_cmp(arg, "mintoperate"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to Operate a Town.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->mintoperate = atoi(argument);
      send_to_char("Minimum requirement for Town Operation use is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minlogsettings"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to pick what gets logged.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minlogsettings = atoi(argument);
      send_to_char("Minimum requirement for LogSettings use is set.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg, "minplace"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to Place mobs/objs.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minplace = atoi(argument);
      send_to_char("Minimum requirement for placing mobs/objs is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "mingeneral"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to command all troops.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->mingeneral = atoi(argument);
      send_to_char("Minimum requirement for commanding all troops is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "mincommand"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to command troops.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->mincommand = atoi(argument);
      send_to_char("Minimum requirement for commanding troops is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minreadlog"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to read kingdom logs.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minreadlog = atoi(argument);
      send_to_char("Minimum requirement for reading kindom logs is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "allowjoin"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the Kingdom to allow or not allow new members (1 - Yes 0 - No).\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 1)
      {
         send_to_char("Choices are 1 - Yes or 0 - No.\n\r", ch);
         return;
      }
      kingdom_table[ht]->allowjoin = atoi(argument);
      send_to_char("Allowing of new members has been changed.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minwithdraw"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to withdraw Kingdom money.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minwithdraw = atoi(argument);
      send_to_char("Minimum requirement for withdrawing Kingdom money is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minappoint"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to use appointment commands.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minappoint = atoi(argument);
      send_to_char("Minimum requirement for appointing is set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "minhighappnt"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Sets the minimum caste required to do high appointments.\n\r", ch);
         return;
      }
      if (atoi(argument) < caste_Serf || atoi(argument) > caste_King)
      {
         send_to_char("Range is Serf to King.\n\r", ch);
         return;
      }
      if (atoi(argument) < kingdom_table[ht]->minappoint)
      {
         send_to_char("Your high appointment cannot be below your low appointment value.\n\r", ch);
         return;
      }
      if (atoi(argument) > ch->pcdata->caste)
      {
         send_to_char("You cannot set a value greater than your caste.\n\r", ch);
         return;
      }
      kingdom_table[ht]->minhappoint = atoi(argument);
      send_to_char("Minimum requirement for doing high appointments is set.\n\r", ch);
      return;
   }
   do_castevalues(ch, "help");
   return;
}

//Sort of like the run command, allows to send multiple direction commands
sh_int check_speedwalk_directions(CHAR_DATA * ch, char *dir)
{
   char *d;

   if (strlen(dir) > 300)
   {
      send_to_char("You can only input up to 300 characters.\n\r", ch);
      return FALSE;
   }
   for (d = dir; *d != '\0'; ++d) //Simple check to make sure it is valid first
   {
      if (*d != 'n' && *d != 'e' && *d != 'w' && *d != 's' && *d != 'u' && *d != 'd' && *d != '+' && !isdigit(*d))
      {
         send_to_char("You can only use nsewdu+ or 0-9 in the directions.\n\r", ch);
         return FALSE;
      }
      if ((isdigit(*d) && d >= dir) && (isdigit(*(d + 1)) && d >= dir) && (isdigit(*(d + 2)) && d >= dir))
      {
         send_to_char("You cannot have a value over 99.\n\r", ch);
         return FALSE;
      }
      if (*d == '+')
      {
         if ((*(d - 1) == 'n' && (*(d + 1) != 'e' && *(d + 1) != 'w'))
            || (*(d - 1) == 's' && (*(d + 1) != 'e' && *(d + 1) != 'w')) || (*(d - 1) != 'n' && *(d - 1) != 's'))
         {
            send_to_char("Only accepts n+w, n+e, s+w, s+e.\n\r", ch);
            return FALSE;
         }
      }
      if ((*(d + 1) == '\0') && isdigit(*d))
      {
         send_to_char("The last value entered cannot be a number.\n\r", ch);
         return FALSE;
      }
   }
   return TRUE;
}

void set_command_buf(CHAR_DATA * ch, char *argument)
{
   char buf[301];

   sprintf(buf, argument);

   if (!ch->midata)
   {
      CREATE(ch->midata, MI_DATA, 1);
      ch->midata->x = ch->coord->x;
      ch->midata->y = ch->coord->y;
      ch->midata->map = ch->map;
      ch->midata->in_room = ch->in_room;
      ch->midata->command = STRALLOC("");
   }
   if (ch->midata->command)
      STRFREE(ch->midata->command);

   ch->midata->command = STRALLOC(buf);
}

//Pretty simple, see the orders given
void do_seeorders(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char buf[MSL];
   char buf2[MSL];
   int cnt = 0;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   /* Only Those who are High Appoint and God+ immortals can set job status for now */
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mincommand ||
      ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend ||
      ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      strcpy(buf, "");
      if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MILITARY) && in_same_room(ch, victim))
      {
         cnt++;
         if (xIS_SET(victim->miflags, KM_CONQUER))
         {
            sprintf(buf2, "CQ ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_STATIONARY))
         {
            sprintf(buf2, "ST ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_SENTINEL))
         {
            sprintf(buf2, "SE ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_PATROL))
         {
            sprintf(buf2, "PT(%d) ", victim->m2);
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_WARN))
         {
            sprintf(buf2, "WN(%d) ", victim->m6);
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_ATTACKE))
         {
            sprintf(buf2, "AE(%d) ", victim->m6);
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_ATTACKN))
         {
            sprintf(buf2, "AN(%d) ", victim->m6);
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_ATTACKA))
         {
            sprintf(buf2, "AA(%d) ", victim->m6);
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_NOPASS))
         {
            sprintf(buf2, "NP ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_REPORT))
         {
            sprintf(buf2, "RP ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_INVITE))
         {
            sprintf(buf2, "IV(%d) ", victim->m5);
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_NOASSIST))
         {
            sprintf(buf2, "NA ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_NOCLOAK))
         {
            sprintf(buf2, "NC ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_NOHOOD))
         {
            sprintf(buf2, "NH ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_ATTACKH))
         {
            sprintf(buf2, "AH ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_ATTACKC))
         {
            sprintf(buf2, "AC ");
            strcat(buf, buf2);
         }
         if (xIS_SET(victim->miflags, KM_NEEDINTRO))
         {
            sprintf(buf2, "NI ");
            strcat(buf, buf2);
         }
         ch_printf(ch, "%-35s >    %s\n\r", victim->name, buf);
      }
   }
   if (cnt == 0)
      send_to_char("There are no mobs in this room with orders.\n\r", ch);
}

void alter_kingdom_eq(CHAR_DATA *victim, OBJ_DATA *obj)
{
   char buf[MSL];
   
   if (IN_PLAYER_KINGDOM(victim->m4))
   {
      sprintf(buf, "-%s- %s", kingdom_table[victim->m4]->name, obj->name);
      STRFREE(obj->name);
      obj->name = STRALLOC(buf);
   
      sprintf(buf, "-%s- %s", kingdom_table[victim->m4]->name, obj->short_descr);
      STRFREE(obj->short_descr);
      obj->short_descr = STRALLOC(buf);
   
      sprintf(buf, "-%s- %s", kingdom_table[victim->m4]->name, obj->description);
      STRFREE(obj->description);
      obj->description = STRALLOC(buf);
   }
   obj->cost = 0;
   xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
   xSET_BIT(obj->extra_flags, ITEM_KINGDOMEQ);
}

//Gets a weapon to equip
//Soldiers - Swords/Axes
//Guard - Swords/Blunt
//Mages - Staves/Daggers
//Cleric - Staves/Blunt
//Scount - Knifes/Swords
void get_arm_weapon(int type, SLAB_DATA *slab, BUYKMOB_DATA *kmob, CHAR_DATA *victim, int replace)
{
   int x;
   OBJ_DATA *obj = NULL;
   OBJ_DATA *quiver;
   int deq = 0;
   
   if (replace > 1) //Use default equipment instead of forge equipment
   {
      deq = 1;
      replace -=10;
   }
   
   if (replace && get_eq_char(victim, WEAR_WIELD) != NULL)
      return;      

   if (xIS_SET(kmob->flags, KMOB_ARCHER))
   {
      obj = create_object(get_obj_index(OBJ_MIL_CROSSBOW), 1);
      obj->value[0] = 1000;
      obj->value[7] = 1;
      if (victim->m12 <= 3)
      {
         obj->value[1] = 2;
         obj->value[2] = 5;
         obj->value[3] = race_table[victim->race]->weaponmin+4;
         obj->value[4] = 2;
         obj->value[9] = 1;
         obj->value[10] = 7;
         STRFREE(obj->name);
         obj->name = STRALLOC("makeshift crossbow");   
         STRFREE(obj->short_descr);
         obj->short_descr = STRALLOC("makeshift crossbow");  
         STRFREE(obj->description);
         obj->description = STRALLOC("Some has left a makesift crossbow here.");
      }
      else if (victim->m12 <= 6)
      {
         obj->value[1] = 4;
         obj->value[2] = 6;
         obj->value[3] = race_table[victim->race]->weaponmin+3;
         obj->value[4] = 4;
         obj->value[9] = 3;
         obj->value[10] = 10;
         STRFREE(obj->name);
         obj->name = STRALLOC("military crossbow");   
         STRFREE(obj->short_descr);
         obj->short_descr = STRALLOC("military crossbow");  
         STRFREE(obj->description);
         obj->description = STRALLOC("Some has left a military crossbow here.");
      }
      else if (victim->m12 <= 9)
      {
         obj->value[1] = 5;
         obj->value[2] = 8;
         obj->value[3] = race_table[victim->race]->weaponmin+2;
         obj->value[4] = 6;
         obj->value[9] = 5;
         obj->value[10] = 14;
         STRFREE(obj->name);
         obj->name = STRALLOC("well crafted crossbow");   
         STRFREE(obj->short_descr);
         obj->short_descr = STRALLOC("well crafted crossbow");  
         STRFREE(obj->description);
         obj->description = STRALLOC("Some has left a well crafted crossbow here.");
      }
      else
      {
         obj->value[1] = 7;
         obj->value[2] = 10;
         obj->value[3] = race_table[victim->race]->weaponmin+1;
         obj->value[4] = 8;
         obj->value[9] = 7;
         obj->value[10] = 18;
         obj->name = STRALLOC("perfect crossbow");   
         STRFREE(obj->short_descr);
         obj->short_descr = STRALLOC("perfect crossbow");  
         STRFREE(obj->description);
         obj->description = STRALLOC("Some has left a perfect crossbow here.");
      }       
      alter_kingdom_eq(victim, obj);
      obj_to_char(obj, victim);
      equip_char(victim, obj, WEAR_MISSILE_WIELD);
      quiver = create_object(get_obj_index(OBJ_MIL_QUIVER), 1);
      if (quiver)
      {
         obj_to_char(quiver, victim);
         for (x = 1; x <= 10; x++)
         {
            obj = create_object(get_obj_index(OBJ_FORGE_ARROW), 1);
            alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
            alter_kingdom_eq(victim, obj);
            xREMOVE_BIT(obj->extra_flags, ITEM_FORGEABLE);
            obj_to_obj(obj, quiver);
         }
         equip_char(victim, quiver, WEAR_BACK);
      }
   }
      
   if (type == 1 || type == 2) //Same weapons for Leather/Light
   {
      if (xIS_SET(kmob->flags, KMOB_SOLDIER))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_HAND_AXE), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_AXE), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_SHORT_SWORD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_CUTLASS), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_GUARD))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_CLUB), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_HAMMER), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_SHORT_SWORD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_CUTLASS), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_SCOUT) || xIS_SET(kmob->flags, KMOB_WORKER) || xIS_SET(kmob->flags, KMOB_ARCHER))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_DIRK), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_KRIS), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_SHORT_SWORD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_CUTLASS), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_MAGE))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_DIRK), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_KRIS), 1); 
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_SCEPTRE), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_ROD), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_CLERIC))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_CLUB), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_HAMMER), 1);   
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_SCEPTRE), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_ROD), 1); 
      }
      alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
      if (deq)
         alter_kingdom_eq(victim, obj);
      obj_to_char(obj, victim);
      equip_char(victim, obj, WEAR_WIELD);
   }
   else if (type == 3) //Medium
   {
      if (xIS_SET(kmob->flags, KMOB_SOLDIER))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_DOUBLE_AXE), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_MATTOCK), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_BROAD_SWORD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_LONG_SWORD), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_GUARD))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_WAR_HAMMER), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_FLAIL), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_BROAD_SWORD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_LONG_SWORD), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_SCOUT) || xIS_SET(kmob->flags, KMOB_WORKER) || xIS_SET(kmob->flags, KMOB_ARCHER))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_MAIN_GAUCHE), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_STILETTO), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_BROAD_SWORD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_LONG_SWORD), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_MAGE))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_MAIN_GAUCHE), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_STILETTO), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_WEIGHTED_ROD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_STAFF), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_CLERIC))
      {
         x = number_range(1, 4);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_WAR_HAMMER), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_FLAIL), 1);
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_WEIGHTED_ROD), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_STAFF), 1); 
      }
      alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
      if (deq)
         alter_kingdom_eq(victim, obj);
      obj_to_char(obj, victim);
      equip_char(victim, obj, WEAR_WIELD);
   }
   else if (type == 4) //Heavy
   {
      if (xIS_SET(kmob->flags, KMOB_SOLDIER))
      {
         x = number_range(1, 4);

         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_GREAT_AXE), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_BATTLE_AXE), 1); 
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_CLAYMORE), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_FLAMBERGE), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_GUARD))
      {
         x = number_range(1, 5);  
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_GREAT_FLAIL), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_MORNING_STAR), 1);        
         else if (x == 5)
            obj = create_object(get_obj_index(OBJ_FORGE_MAUL), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_CLAYMORE), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_FLAMBERGE), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_SCOUT) || xIS_SET(kmob->flags, KMOB_WORKER) || xIS_SET(kmob->flags, KMOB_ARCHER))
      {
         x = number_range(3, 4);
         if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_CLAYMORE), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_FLAMBERGE), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_MAGE))
      {
         x = number_range(3, 4);
         if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_BATTLE_STAFF), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_BLADED_STAFF), 1); 
      }
      if (xIS_SET(kmob->flags, KMOB_CLERIC))
      {
         x = number_range(1, 5);
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_GREAT_FLAIL), 1);   
         else if (x == 2)
            obj = create_object(get_obj_index(OBJ_FORGE_MORNING_STAR), 1);        
         else if (x == 5)
            obj = create_object(get_obj_index(OBJ_FORGE_MAUL), 1);  
         else if (x == 3)
            obj = create_object(get_obj_index(OBJ_FORGE_BATTLE_STAFF ), 1); 
         else if (x == 4)
            obj = create_object(get_obj_index(OBJ_FORGE_BLADED_STAFF), 1); 
      }
      alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
      if (deq)
         alter_kingdom_eq(victim, obj);
      obj_to_char(obj, victim);
      equip_char(victim, obj, WEAR_WIELD);
   }
}       

void get_arm_armor(int type, SLAB_DATA *slab, BUYKMOB_DATA *kmob, CHAR_DATA *victim, int replace)
{
   int x;
   OBJ_DATA *obj;
   int deq = 0;
   
   if (replace > 1) //Use default equipment instead of forge equipment
   {
      deq = 1;
      replace -=10;
   }
   
   x = number_range(1, 2);
   
   if (type == 2)
   {  
      if (!replace || get_eq_char(victim, WEAR_BODY) == NULL)
      {
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_CHAIN_MAIL), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_CHAIN_HAUBERK), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_BODY);   
      }
      
      if (!replace || get_eq_char(victim, WEAR_ARM_R) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_CHAIN_GAUNTLET), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_ARM_R);  
      }
      
      if (!replace || get_eq_char(victim, WEAR_ARM_L) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_CHAIN_GAUNTLET), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_ARM_L);
      }
      
      if (!replace || get_eq_char(victim, WEAR_LEG_R) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_CHAIN_GREAVE), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_LEG_R);  
      }
      
      if (!replace || get_eq_char(victim, WEAR_LEG_L) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_CHAIN_GREAVE), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_LEG_L);
      }
      
      if (!replace || get_eq_char(victim, WEAR_NECK) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_AVENTAIL), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_NECK);
      }
      
      if (!replace || get_eq_char(victim, WEAR_HEAD) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_CABASSET), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_HEAD);
      }
      
      if (xIS_SET(kmob->flags, KMOB_GUARD))
      {
         if (!replace || get_eq_char(victim, WEAR_SHIELD) == NULL)
         {
            obj = create_object(get_obj_index(OBJ_FORGE_BUCKLER), 1); 
            alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
            if (deq)
               alter_kingdom_eq(victim, obj);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_SHIELD);
         }
      }
   }
   if (type == 3)
   {
      if (!replace || get_eq_char(victim, WEAR_BODY) == NULL)
      {
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_RING_MAIL), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_DOUBLE_RING_MAIL), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_BODY);   
      }
      
      if (!replace || get_eq_char(victim, WEAR_ARM_R) == NULL)
      {
         if (x == 1)
         obj = create_object(get_obj_index(OBJ_FORGE_RING_GAUNTLET), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_GAUNTLET), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_ARM_R);  
      } 
      
      if (!replace || get_eq_char(victim, WEAR_ARM_L) == NULL)
      {
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_RING_GAUNTLET), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_GAUNTLET), 1);
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_ARM_L);
      }
      
      if (!replace || get_eq_char(victim, WEAR_LEG_R) == NULL)
      {
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_RING_GREAVE), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_GREAVE), 1);
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_LEG_R);  
      }
      
      if (!replace || get_eq_char(victim, WEAR_LEG_L) == NULL)
      {
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_RING_GREAVE), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_GREAVE), 1);
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_LEG_L);
      }
       
      if (!replace || get_eq_char(victim, WEAR_NECK) == NULL)
      {     
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_COIF), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_DOUBLE_COIF), 1);
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_NECK);
      }
         
      if (!replace || get_eq_char(victim, WEAR_HEAD) == NULL)
      {
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_CASQUE), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_ARMET), 1);
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_HEAD);
      }
      
      if (xIS_SET(kmob->flags, KMOB_GUARD))
      {
         if (!replace || get_eq_char(victim, WEAR_SHIELD) == NULL)
         {
            if (x == 1)
              obj = create_object(get_obj_index(OBJ_FORGE_ROUNDSHIELD), 1); 
            else
               obj = create_object(get_obj_index(OBJ_FORGE_HEATER), 1);
            alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
            if (deq)
               alter_kingdom_eq(victim, obj);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_SHIELD);
         }
      }  
   }
   if (type == 4)
   {
      if (!replace || get_eq_char(victim, WEAR_BODY) == NULL)
      {
         if (x == 1)
            obj = create_object(get_obj_index(OBJ_FORGE_BREASTPLATE), 1); 
         else
            obj = create_object(get_obj_index(OBJ_FORGE_CUIRASS), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_BODY);   
      }
      
      if (!replace || get_eq_char(victim, WEAR_ARM_R) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_VAMBRACE), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_ARM_R);  
      }
      
      if (!replace || get_eq_char(victim, WEAR_ARM_L) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_VAMBRACE), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_ARM_L);
      }
      
      if (!replace || get_eq_char(victim, WEAR_LEG_R) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_CUISS), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_LEG_R);  
      }
      
      if (!replace || get_eq_char(victim, WEAR_LEG_L) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_CUISS), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_LEG_L);
      }
      
      if (!replace || get_eq_char(victim, WEAR_NECK) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_GORGET), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_NECK);
      }
      
      if (!replace || get_eq_char(victim, WEAR_HEAD) == NULL)
      {
         obj = create_object(get_obj_index(OBJ_FORGE_HEAUME), 1); 
         alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
         if (deq)
            alter_kingdom_eq(victim, obj);
         obj_to_char(obj, victim);
         equip_char(victim, obj, WEAR_HEAD);
      }
      if (xIS_SET(kmob->flags, KMOB_GUARD))
      {
         if (!replace || get_eq_char(victim, WEAR_SHIELD) == NULL)
         {
            if (x == 1)
              obj = create_object(get_obj_index(OBJ_FORGE_KITESHIELD), 1); 
            else
               obj = create_object(get_obj_index(OBJ_FORGE_TOWERSHIELD), 1);
            alter_forge_obj(victim, obj, create_object(get_obj_index(slab->vnum), 1), slab);
            if (deq)
               alter_kingdom_eq(victim, obj);
            obj_to_char(obj, victim);
            equip_char(victim, obj, WEAR_SHIELD);
         }
      }
   }
}  
int get_arm_coins_requirements(int type, int race, int size)
{
   int cost = 0;
   
   if (race == RACE_OGRE) 
   {
      if (type == 1)
         cost = 30;
      if (type == 2)
         cost = 450;
      if (type == 3)
         cost = 700;
      if (type == 4)
         cost = 1300;
   }
   if (race == RACE_FAIRY) 
   {
      if (type == 1)
         cost = 10;
      if (type == 2)
         cost = 100;
      if (type == 3)
         cost = 200;
      if (type == 4)
         cost = 350;
   }
   if (race == RACE_HOBBIT) 
   {
      if (type == 1)
         cost = 10;
      if (type == 2)
         cost = 150;
      if (type == 3)
         cost = 300;
      if (type == 4)
         cost = 500;
   }
   if (race == RACE_DWARF) 
   {
      if (type == 1)
         cost = 20;
      if (type == 2)
         cost = 350;
      if (type == 3)
         cost = 600;
      if (type == 4)
         cost = 1050;
   }
   if (race == RACE_ELF) 
   {
      if (type == 1)
         cost = 20;
      if (type == 2)
         cost = 200;
      if (type == 3)
         cost = 350;
      if (type == 4)
         cost = 600;
   }
   if (race == RACE_HUMAN) 
   {
      if (type == 1)
         cost = 20;
      if (type == 2)
         cost = 300;
      if (type == 3)
         cost = 500;
      if (type == 4)
         cost = 900;
   }
   cost = cost * size * 1.5;
   return cost;
}

int get_arm_ore_requirements(int type, int race)
{
   if (race == RACE_OGRE) 
   {
      if (type == 1)
         return 3;
      if (type == 2)
         return 45;
      if (type == 3)
         return 70;
      if (type == 4)
         return 130;
   }
   if (race == RACE_FAIRY) 
   {
      if (type == 1)
         return 1;
      if (type == 2)
         return 10;
      if (type == 3)
         return 20;
      if (type == 4)
         return 35;
   }
   if (race == RACE_HOBBIT) 
   {
      if (type == 1)
         return 1;
      if (type == 2)
         return 15;
      if (type == 3)
         return 30;
      if (type == 4)
         return 50;
   }
   if (race == RACE_DWARF) 
   {
      if (type == 1)
         return 2;
      if (type == 2)
         return 35;
      if (type == 3)
         return 60;
      if (type == 4)
         return 105;
   }
   if (race == RACE_ELF) 
   {
      if (type == 1)
         return 2;
      if (type == 2)
         return 20;
      if (type == 3)
         return 35;
      if (type == 4)
         return 60;
   }
   if (race == RACE_HUMAN) 
   {
      if (type == 1)
         return 2;
      if (type == 2)
         return 30;
      if (type == 3)
         return 50;
      if (type == 4)
         return 90;
   }
   return 2;
}

//Repairs the objs or returns the number of slabs needed to do so...
void check_arm_repair(CHAR_DATA *ch, CHAR_DATA *victim, BUYKMOB_DATA *kmob, char *argument)
{
   OBJ_DATA *obj;
   int type = 0;
   char buf[MSL];
   SLAB_DATA *slab;
   DEPO_ORE_DATA *dore;
   int points = 0;
   int deq = 0;
   
   //Find out what kind of armor we have first...
   if ((obj = get_eq_char(victim, WEAR_BODY)) != NULL)
   {
      if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_ARMOR)
         type = 1;
      if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_MAIL || obj->pIndexData->vnum == OBJ_FORGE_CHAIN_HAUBERK)
         type = 2;
      if (obj->pIndexData->vnum == OBJ_FORGE_RING_MAIL || obj->pIndexData->vnum == OBJ_FORGE_DOUBLE_RING_MAIL)
         type = 3;
      if (obj->pIndexData->vnum == OBJ_FORGE_BREASTPLATE || obj->pIndexData->vnum == OBJ_FORGE_CUIRASS)
         type = 4;
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_HEAD)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_HEAD)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CABASSET)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_CASQUE || obj->pIndexData->vnum == OBJ_FORGE_ARMET)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_HEAUME)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_NECK)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_NECK)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_AVENTAIL)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_COIF || obj->pIndexData->vnum == OBJ_FORGE_DOUBLE_COIF)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_GORGET)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_ARM_R)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GAUNTLET)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GAUNTLET)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GAUNTLET || obj->pIndexData->vnum == OBJ_FORGE_GAUNTLET)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_VAMBRACE)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_ARM_L)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GAUNTLET)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GAUNTLET)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GAUNTLET || obj->pIndexData->vnum == OBJ_FORGE_GAUNTLET)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_VAMBRACE)
            type = 4;
      }
   }  
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_LEG_R)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GREAVE)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GREAVE)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GREAVE || obj->pIndexData->vnum == OBJ_FORGE_GREAVE)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_CUISS)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_LEG_L)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GREAVE)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GREAVE)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GREAVE || obj->pIndexData->vnum == OBJ_FORGE_GREAVE)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_CUISS)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_SHIELD)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_BUCKLER)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_ROUNDSHIELD || obj->pIndexData->vnum == OBJ_FORGE_HEATER)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_KITESHIELD || obj->pIndexData->vnum == OBJ_FORGE_TOWERSHIELD)
            type = 4;
      }
   }  
   if (!obj)
   {
      send_to_char("This unit has no equipment, you need to arm it, not repair it.\n\r", ch);
      return;
   }
   if (IS_OBJ_STAT(obj, ITEM_KINGDOMEQ))
      deq = 1;
   if (type <= 1)
   {
      send_to_char("This unit is using leather, you cannot repair leather, just replace it.\n\r", ch);
      return;
   }
   for(slab = first_slab; slab; slab = slab->next)
   {
      if(is_name(slab->adj, obj->name) )
         break;
   }
   if (!slab)
   {
      send_to_char("Could not find the ore on that unit, going to have to replace the equipment.\n\r", ch);
      return;
   }
   //body-25 head-10 neck-10 weapon-15 rarm-10 larm-10 lleg-10 rleg-10
   //body-20 head-10 neck-10 weapon-10 shield-10 rarm-10 larm-10 lleg-10 rlleg-10
   
   if (xIS_SET(kmob->flags, KMOB_GUARD))
   {
      if ((obj = get_eq_char(victim, WEAR_BODY)) != NULL)
         points += (1000-obj->value[3])*20;
      else
         points += 1000*20;
         
      if ((obj = get_eq_char(victim, WEAR_SHIELD)) != NULL)
         points += (1000-(1000 * obj->value[0] / obj->value[1])) *10;
      else
         points += 1000*10;
         
      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
         points += (1000 - obj->value[0])*10;
      else
         points += 1000*10;
   }
   else
   {
      if ((obj = get_eq_char(victim, WEAR_BODY)) != NULL)
         points += (1000-obj->value[3])*25;
      else
         points += 1000*25;
         
      if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)
         points += (1000 - obj->value[0])*15;
      else
         points += 1000*15;   
   }
   if ((obj = get_eq_char(victim, WEAR_HEAD)) != NULL)
      points += (1000-obj->value[3])*10;
   else
      points += 1000*10;
   if ((obj = get_eq_char(victim, WEAR_NECK)) != NULL)
      points += (1000-obj->value[3])*10;
   else
      points += 1000*10;
   if ((obj = get_eq_char(victim, WEAR_ARM_R)) != NULL)
      points += (1000-obj->value[3])*10;
   else
      points += 1000*10;
   if ((obj = get_eq_char(victim, WEAR_ARM_L)) != NULL)
      points += (1000-obj->value[3])*10;
   else
      points += 1000*10;
   if ((obj = get_eq_char(victim, WEAR_LEG_R)) != NULL)
      points += (1000-obj->value[3])*10;
   else
      points += 1000*10;
   if ((obj = get_eq_char(victim, WEAR_LEG_L)) != NULL)
      points += (1000-obj->value[3])*10;
   else
      points += 1000*10;
   
   //Get the percent of damage done, then take it against the ore cost   
   //Since we now have deq, it costs coins to repair default eq, same thing, take the percentage...
   points = 100 * points / 100000;
   if (deq == 0)
      points = get_arm_ore_requirements(type, victim->race) * points / 100;
   else
      points = get_arm_coins_requirements(type, victim->race, victim->pIndexData->m12) * points / 100;
   if (!str_cmp(argument, "appraise"))
   {
      if (deq == 0)
         ch_printf(ch, "It would require %d ore to fix this unit.\n\r", points);
      else
         ch_printf(ch, "It would require %d coins to fix this unit.\n\r", points);
      return;
   }
   if (deq == 0)
   {
      for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
      {
         if (dore->vnum == slab->vnum)
            break;
      }
      if (!dore)
      {
         send_to_char("Your depository is not working correctly, notify an imm.\n\r", ch);
         return;
      }
      if (dore->count < get_arm_ore_requirements(type, victim->race))
      {
         ch_printf(ch, "You need %d ore to do that, you only have %d.\n\r", points, dore->count);
         return;
      }
      sprintf(buf, "%s repaired %s costing the kingdom %d slabs of %s", ch->name, victim->short_descr, points, slab->adj);
      write_kingdom_logfile(ch->pcdata->town->kingdom, buf, KLOG_ARMMILITARY);
      dore->count -= points;
   }
   else
   {
      if (ch->pcdata->town->coins < points)
      {
         ch_printf(ch, "You need %d coins to do that, you only have %d.\n\r", points, ch->pcdata->town->coins);
         return;
      }
      sprintf(buf, "%s repaired %s costing the kingdom %d coins", ch->name, victim->short_descr, points);
      write_kingdom_logfile(ch->pcdata->town->kingdom, buf, KLOG_ARMMILITARY);
      ch->pcdata->town->coins -= points;
   }
   if ((obj = get_eq_char(victim, WEAR_BODY)) != NULL)   
      obj->value[3] = 1000;
   if ((obj = get_eq_char(victim, WEAR_NECK)) != NULL)   
      obj->value[3] = 1000;
   if ((obj = get_eq_char(victim, WEAR_HEAD)) != NULL)   
      obj->value[3] = 1000;
   if ((obj = get_eq_char(victim, WEAR_ARM_R)) != NULL)   
      obj->value[3] = 1000;
   if ((obj = get_eq_char(victim, WEAR_ARM_L)) != NULL)   
      obj->value[3] = 1000;
   if ((obj = get_eq_char(victim, WEAR_LEG_R)) != NULL)   
      obj->value[3] = 1000;
   if ((obj = get_eq_char(victim, WEAR_LEG_L)) != NULL)   
      obj->value[3] = 1000;
   if ((obj = get_eq_char(victim, WEAR_WIELD)) != NULL)   
      obj->value[0] = 1000;
   if ((obj = get_eq_char(victim, WEAR_SHIELD)) != NULL)   
      obj->value[0] = obj->value[1];
      
   //will reequip anything missing
   get_arm_weapon(type, slab, kmob, victim, 1+(deq*10));
   get_arm_armor(type, slab, kmob, victim, 1+(deq*10));
   send_to_char("Repaired.\n\r", ch);
   return;
}

SLAB_DATA *find_kore_type(TOWN_DATA *town, CHAR_DATA *victim)
{        
   SLAB_DATA *slab;
   SLAB_DATA *pslab = NULL;
   int lvalue = 0; 
   for(slab = first_slab; slab; slab = slab->next)
   {
      if (slab->kmob <= town->size && slab->kmob > lvalue)
      {
         pslab = slab;
         lvalue = slab->kmob;   
      }
   }   
   if (!pslab)   
   {
      bug("A slab could not be found for default slab for mob %d", victim->pIndexData->vnum);
      return NULL;
   }          
   return pslab;
}
char *get_type_name(int type)
{
   static char buf[15];
   
   if (type == 1)
      sprintf(buf, "Leather");
   else if (type == 2)
      sprintf(buf, "Light");
   else if (type == 3)
      sprintf(buf, "Medium");
   else if (type == 4)
      sprintf(buf, "Heavy");
   else
      sprintf(buf, "Unknown");
   return buf;
}
     
void do_armmilitary(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char buf[MSL];
   DEPO_ORE_DATA *dore;
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   BUYKMOB_DATA *kmob;
   SLAB_DATA *slab;
   int type = 0;
   int single = 0;
   
   if (check_npc(ch))
         return;
   
   if (!ch->pcdata->town)
   {
      send_to_char("You need to belong to a town to use this command.\n\r", ch);
      return;
   }
   if (!IN_PLAYER_KINGDOM(ch->pcdata->hometown))
   {
      send_to_char("You need to belong to an actual kingdom to use this command.\n\r", ch);
      return;
   }
   
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mincommand)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mindepository)
   {
      send_to_char("You have to have rights to use the depository to use this command.\n\r", ch);
      return;
   }
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: armmilitary <unit/all> <leather/light/medium/heavy> <ore/default>\n\r", ch);
      send_to_char("Syntax: armmilitary <unit/all> repair [appraise]\n\r", ch);
      send_to_char("Syntax: armmilitary remount\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg2);
   
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("You need to be at your barracks to do this.\n\r", ch);
      return;
   }
   if (ch->coord->x != ch->pcdata->town->barracks[0] || ch->coord->y != ch->pcdata->town->barracks[1] ||
       ch->map != ch->pcdata->town->barracks[2])
   {
      send_to_char("You need to be at your barracks to do this.\n\r", ch);
      return;
   }
   
   //New horses for ones missing horses
   if (!str_cmp(arg, "remount"))
   {
      CHAR_DATA *mount;
      int mcnt = 0;
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if (IN_SAME_ROOM(ch, victim))
         {
            if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MILITARY))
            {
               for (kmob = first_buykmob; kmob; kmob = kmob->next)
               {
                  if (kmob->vnum == victim->pIndexData->vnum && xIS_SET(kmob->flags, KMOB_MOUNTED) && !victim->mount)
                  {
                     mount = create_mobile(get_mob_index(MOB_KMOB_HORSE));
                     char_to_room(mount, get_room_index(OVERLAND_SOLAN));
                     mount->coord->x = victim->coord->x;
                     mount->coord->y = victim->coord->y;
                     mount->map = victim->map;
                     mount->m4 = victim->m4;
                     SET_ONMAP_FLAG(mount); 
                     xSET_BIT(mount->act, ACT_MOUNTED);
                     victim->mount = mount;
                     victim->position = POS_MOUNTED;  
                     mcnt++;
                     break;
                  }
               }
            }
         }
      }
      ch_printf(ch, "%d soldier%s reported in for a new mount.\n\r", mcnt, mcnt==1 ? "" : "s");
      return;
   }
   
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (single == 1)
         break;
      if (str_cmp(arg, "all"))
      {
         if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
         {
            send_to_char("They aren't here.\n\r", ch);
            return;
         }   
         single = 1;
      }
      else
      {
         if (!IN_SAME_ROOM(ch, victim))
            continue;
         if (HAS_WAIT(victim))
         {
            continue;
         }
         if (!xIS_SET(victim->act, ACT_MILITARY))
         {
            continue;
         }  
         if (victim->m4 != ch->pcdata->hometown && !IS_IMMORTAL(ch))
         {
            continue;
         }   
         if (victim->m1 != ch->pcdata->town->tpid)
         {
            if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mingeneral)
            {
               continue;
            }
         }
      }
      if (HAS_WAIT(victim))
      {
         send_to_char("The mobile looks a bit too busy for that.\n\r", ch);
         continue;
      }
      if (!xIS_SET(victim->act, ACT_MILITARY))
      {
         send_to_char("You can only arm military units.\n\r", ch);
         continue;
      }
   
      if (victim->m4 != ch->pcdata->hometown && !IS_IMMORTAL(ch))
      {
         send_to_char("That one is not in your kingdom.\n\r", ch);
         continue;
      }   
      if (victim->m1 != ch->pcdata->town->tpid)
      {
         if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mingeneral)
         {
            send_to_char("You can only command units that belong to your town.\n\r", ch);
            continue;
         }
      }
      if (!str_cmp(arg2, "repair"))
      {
         for (kmob = first_buykmob; kmob; kmob = kmob->next)
         {
            if (victim->pIndexData->vnum == kmob->vnum)
               break;
         }
         if (!kmob)
         {
            send_to_char("That mob is not in the list of mobs you can purchase, tell an imm.\n\r", ch);
            continue;
         }
         if (!xIS_SET(kmob->flags, KMOB_CLERIC) && !xIS_SET(kmob->flags, KMOB_WORKER) && !xIS_SET(kmob->flags, KMOB_GUARD) && 
             !xIS_SET(kmob->flags, KMOB_SOLDIER) && !xIS_SET(kmob->flags, KMOB_SCOUT) && !xIS_SET(kmob->flags, KMOB_MAGE) &&
             !xIS_SET(kmob->flags, KMOB_ARCHER))
         {
            send_to_char("That unit is not flagged correctly, tell an immortal.\n\r", ch);
            continue;
         }
         check_arm_repair(ch, victim, kmob, argument);
         write_depo_list();
         continue;
      }   
      if (!str_cmp(arg2, "leather"))
         type = 1;
      if (!str_cmp(arg2, "light"))
         type = 2;
      if (!str_cmp(arg2, "medium"))
         type = 3;
      if (!str_cmp(arg2, "heavy"))
         type = 4;
      
      if (type == 0)
      {
         send_to_char("Choices are leather/light/medium/heavy.\n\r", ch);
         continue;
      }
      //If they are wearing anything, need to get rid of it.
      remove_obj(victim, WEAR_NECK, 2);
      remove_obj(victim, WEAR_ARM_L, 2); 
      remove_obj(victim, WEAR_ARM_R, 2);
      remove_obj(victim, WEAR_LEG_L, 2);
      remove_obj(victim, WEAR_LEG_R, 2);
      remove_obj(victim, WEAR_BODY, 2);
      remove_obj(victim, WEAR_HEAD, 2);
      remove_obj(victim, WEAR_SHIELD, 2);
      remove_obj(victim, WEAR_WIELD, 2);
   
      for (obj = victim->first_carrying; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         obj_from_char(obj);
         extract_obj(obj);
      }
      for(slab = first_slab; slab; slab = slab->next)
      {
         if(!str_cmp(slab->adj, argument))
             break;
      }
      if (!slab)
      {
         if (str_cmp(argument, "default"))
         {
            send_to_char("Could not find the ore you speak of.\n\r", ch);
            continue;
         }
      }
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (victim->pIndexData->vnum == kmob->vnum)
            break;
      }
      if (!kmob)
      {
         send_to_char("That mob is not in the list of mobs you can purchase, tell an imm.\n\r", ch);
         continue;
      }
      if (!xIS_SET(kmob->flags, KMOB_CLERIC) && !xIS_SET(kmob->flags, KMOB_WORKER) && !xIS_SET(kmob->flags, KMOB_GUARD) && 
          !xIS_SET(kmob->flags, KMOB_SOLDIER) && !xIS_SET(kmob->flags, KMOB_SCOUT) && !xIS_SET(kmob->flags, KMOB_MAGE) &&
          !xIS_SET(kmob->flags, KMOB_ARCHER))
      {
         send_to_char("That unit is not flagged correctly, tell an immortal.\n\r", ch);
         continue;
      }
      if (type > 1 && xIS_SET(kmob->flags, KMOB_LEATHERONLY))
      {
         send_to_char("This unit can only don leather armor.\n\r", ch);
         return;
      }
      if (type > 2 && xIS_SET(kmob->flags, KMOB_LIGHTONLY))
      {
         send_to_char("This unit can only don leather or light armor.\n\r", ch);
         return;
      }
      if (type > 3 && xIS_SET(kmob->flags, KMOB_MEDIUMONLY))
      {
         send_to_char("This unit can only don leather, light, or medium armor.\n\r", ch);
         return;
      }
      if (!slab)
         slab = find_kore_type(ch->pcdata->town, victim);
      if (!slab)
      {
         send_to_char("Could not find a default ore for the unit, tell an immortal.\n\r", ch);
         bug("Could not find a default ore for %d", victim->pIndexData->vnum);
         continue;
      }
      for (dore = kingdom_table[ch->pcdata->hometown]->first_ore; dore; dore = dore->next)
      {
         if (dore->vnum == slab->vnum)
            break;
      }
      if (!dore)
      {
         send_to_char("Your depository is not working correctly, notify an imm.\n\r", ch);
         continue;
      }
      if (str_cmp(argument, "default"))
      {
         if (dore->count < get_arm_ore_requirements(type, victim->race))
         {
            ch_printf(ch, "You need %d ore to do that, you only have %d.\n\r", get_arm_ore_requirements(type, victim->race), dore->count);
            continue;
         }
      }
      if (!str_cmp(argument, "default"))
      {        
         if (ch->pcdata->town->coins < get_arm_coins_requirements(type, victim->race, victim->pIndexData->m12) && !xIS_SET(victim->miflags, KM_KEQUIPED))
         {
            ch_printf(ch, "You need %d coins to do that, you only have %d.\n\r", get_arm_coins_requirements(type, victim->race, victim->pIndexData->m12), ch->pcdata->town->coins);
            continue;
         }
         get_arm_weapon(type, slab, kmob, victim, 10);
         if (type >= 2)
            get_arm_armor(type, slab, kmob, victim, 10);
      }
      else
      {        
         get_arm_weapon(type, slab, kmob, victim, 0);
         if (type >= 2)
            get_arm_armor(type, slab, kmob, victim, 0);
      }
      if (!str_cmp(argument, "default"))
      {
         if (xIS_SET(victim->miflags, KM_KEQUIPED))
         {
            ch_printf(ch, "It costs you %d coins to equiped that mob.\n\r", get_arm_coins_requirements(type, victim->race, victim->pIndexData->m12));
            sprintf(buf, "%s equiped %s with %s armor costing the kingdom nothing (default).", ch->name, victim->short_descr, get_type_name(type));
            sprintf(buf, "%s equiped %s with %s armor costing the kingdom %d coins.", ch->name, victim->short_descr, get_type_name(type), get_arm_coins_requirements(type, victim->race, victim->pIndexData->m12));
         }
         else
         {
            ch_printf(ch, "You equip your initial default equiped, any further equips or repairs will cost you!\n\r");
            sprintf(buf, "%s equiped %s with %s armor costing the kingdom nothing (default).", ch->name, victim->short_descr, get_type_name(type));
         }
         ch->pcdata->town->coins -= get_arm_coins_requirements(type, victim->race, victim->pIndexData->m12);
         if (!xIS_SET(victim->miflags, KM_KEQUIPED))
            xSET_BIT(victim->miflags, KM_KEQUIPED);
         write_kingdom_logfile(ch->pcdata->town->kingdom, buf, KLOG_ARMMILITARY);
      }
      else
      {
         ch_printf(ch, "It costs you %d slabs to equip that mob.\n\r", get_arm_ore_requirements(type, victim->race));
         sprintf(buf, "%s equiped %s with %s armor costing you %d slabs of %s", ch->name, victim->short_descr, get_type_name(type), get_arm_ore_requirements(type, victim->race), slab->adj);
         dore->count -= get_arm_ore_requirements(type, victim->race);
         write_kingdom_logfile(ch->pcdata->town->kingdom, buf, KLOG_ARMMILITARY);
      }
   }
   write_depo_list();
   return;
}

bool is_coord(char *argument)
{
   char *x;
   
   for (x = argument; *x != '\0'; x++)
   {
      if (!isdigit(*x) && *x != '-')
         return FALSE;
   }
   return TRUE;
}

int get_unit_weight(CHAR_DATA *victim)
{
   int type = 1;
   OBJ_DATA *obj;
   
   //Find out what kind of armor we have first...
   if ((obj = get_eq_char(victim, WEAR_BODY)) != NULL)
   {
      if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_ARMOR)
         type = 1;
      if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_MAIL || obj->pIndexData->vnum == OBJ_FORGE_CHAIN_HAUBERK)
         type = 2;
      if (obj->pIndexData->vnum == OBJ_FORGE_RING_MAIL || obj->pIndexData->vnum == OBJ_FORGE_DOUBLE_RING_MAIL)
         type = 3;
      if (obj->pIndexData->vnum == OBJ_FORGE_BREASTPLATE || obj->pIndexData->vnum == OBJ_FORGE_CUIRASS)
         type = 4;
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_HEAD)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_HEAD)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CABASSET)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_CASQUE || obj->pIndexData->vnum == OBJ_FORGE_ARMET)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_HEAUME)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_NECK)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_NECK)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_AVENTAIL)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_COIF || obj->pIndexData->vnum == OBJ_FORGE_DOUBLE_COIF)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_GORGET)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_ARM_R)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GAUNTLET)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GAUNTLET)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GAUNTLET || obj->pIndexData->vnum == OBJ_FORGE_GAUNTLET)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_VAMBRACE)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_ARM_L)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GAUNTLET)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GAUNTLET)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GAUNTLET || obj->pIndexData->vnum == OBJ_FORGE_GAUNTLET)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_VAMBRACE)
            type = 4;
      }
   }  
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_LEG_R)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GREAVE)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GREAVE)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GREAVE || obj->pIndexData->vnum == OBJ_FORGE_GREAVE)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_CUISS)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_LEG_L)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_STUDDED_LEATHER_GREAVE)
            type = 1;
         if (obj->pIndexData->vnum == OBJ_FORGE_CHAIN_GREAVE)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_RING_GREAVE || obj->pIndexData->vnum == OBJ_FORGE_GREAVE)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_CUISS)
            type = 4;
      }
   }
   if (!obj)
   {
      if ((obj = get_eq_char(victim, WEAR_SHIELD)) != NULL)   
      {
         if (obj->pIndexData->vnum == OBJ_FORGE_BUCKLER)
            type = 2;
         if (obj->pIndexData->vnum == OBJ_FORGE_ROUNDSHIELD || obj->pIndexData->vnum == OBJ_FORGE_HEATER)
            type = 3;
         if (obj->pIndexData->vnum == OBJ_FORGE_KITESHIELD || obj->pIndexData->vnum == OBJ_FORGE_TOWERSHIELD)
            type = 4;
      }
   }
   return type;
}

int adjust_movement(CHAR_DATA *ch, int move)
{
   CHAR_DATA *victim;
   BUYKMOB_DATA *kmob;
   int add = 0;
   int utype;

   if (!IS_NPC(ch))
      return move;
   if (!IN_WILDERNESS(ch))
      return move;
   if (!xIS_SET(ch->act, ACT_MILITARY))
      return move;
     
   utype = get_unit_weight(ch);
   if (utype == 2)
      move+=1;
   if (utype == 3)
      move+=3;
   if (utype == 4)
      move+=5;
   if (ch->position == POS_MOUNTED)
      return UMAX(1, move-5); 
   else
   {
      if (ch->race == RACE_FAIRY)
         add+=2;
   }
      
   for (kmob = first_buykmob; kmob; kmob = kmob->next)
   {
      if (kmob->vnum == ch->pIndexData->vnum)
      {
         if (xIS_SET(kmob->flags, KMOB_SOLDIERONEMOVE))
            return UMAX(1, move-2-add);
         if (xIS_SET(kmob->flags, KMOB_SOLDIERADDMOVE))  
            break;
      }
   }
   if (!kmob)
      return UMAX(1, move-add);
   for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
   {
      if (!xIS_SET(victim->act, ACT_MILITARY))
         continue;
      for (kmob = first_buykmob; kmob; kmob = kmob->next)
      {
         if (kmob->vnum == victim->pIndexData->vnum)
            if (xIS_SET(kmob->flags, KMOB_SOLDIERMOVE))  
               if (abs(ch->coord->x - victim->coord->x) <= 10 && abs(ch->coord->y - victim->coord->y) <= 10
               &&  victim->m4 == ch->m4)
                  break;
      }
      if (kmob)
         return UMAX(1, move-2-add);
   }
   return UMAX(1, move-add);
}  

int get_kingdom_units(int tpid)
{
   ROOM_INDEX_DATA *room;
   CHAR_DATA *ch;
   int cnt = 4;
   
   room = get_room_index(OVERLAND_SOLAN);
   
   if (!room)
   {
      bug("Could not find Overland room, something is horribly wrong.");
      return 0;
   }
   for (ch = room->first_person; ch; ch = ch->next_in_room)
   {          
      if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY) && ch->m1 == tpid)
         cnt++;  
      if (IS_NPC(ch) && xIS_SET(ch->act, ACT_EXTRACTMOB) && ch->m7 == tpid)
         cnt++;
   }
   return cnt/5;
}



//Command troops.
void do_command(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   int x, y;
   char buf[MSL];
   CHAR_DATA *victim = NULL;
   int range = 0;
   int cnt = 0;
   int maxspd = 0;
   int isreturn = 0;
   TOWN_DATA *town;
   TOWN_DATA *stown;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!IN_PLAYER_KINGDOM(ch->pcdata->hometown))
   {
      send_to_char("You need to belong to a player owned kingdom to use this command.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You need to belong to a town to use this command.\n\r", ch);
      return;
   }
   /* Only Those who are High Appoint and God+ immortals can set job status for now */
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mincommand ||
      ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend ||
      ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: command [mob/all/range] <dir>.\n\r", ch);
      send_to_char("Syntax: command [mob/all/range] <x> <y>.\n\r", ch);
      send_to_char("Syntax: command return [range].\n\r", ch);
      send_to_char("Syntax: command town <townname>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   //Return just pulls everyone back to you now
   if (!str_cmp(arg1, "return"))
   {
      if (atoi(argument) < 1 || atoi(argument) > 15)
      {
         send_to_char("You are only capable of commanding in a range of 1 to 15.\n\r", ch);
         return;
      }
      range = atoi(argument);
      cnt = 0;
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if ((victim->map == ch->map) && (abs(victim->coord->x - ch->coord->x) <= range && abs(victim->coord->y - ch->coord->y) <= range))
         {
            if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
            {
               continue;
            }
            sprintf(buf, "move");
            isreturn = islinked(ch, ch->coord->x, ch->coord->y, victim->coord->x, victim->coord->y, buf, FALSE);
            if (isreturn == 1)
            {
               if (check_speedwalk_directions(ch, buf) == FALSE)
                  return;
               set_command_buf(victim, buf);
               if (victim->midata)
                  victim->midata->mspeed = 0;
               victim->midata->mspeed = adjust_movement(victim, victim->m10);
               victim->midata->mtick = 0;
            }
            if (isreturn == 2)
            {
               sprintf(buf, "%s I cannot find a route my lord!", ch->name);
               do_tell(victim, buf);
            }
         }
      } 
      send_to_char("Done.\n\r", ch);
      sprintf(logb, "%s commanded some troops", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_COMMAND);
      return;   
   }
   if (!str_cmp(arg1, "town"))
   {
      if ((town = get_town(argument)) == NULL)
      {
         send_to_char("That is not a valid town you selected.\n\r", ch);
         return;
      }
      if (town->kingdom != ch->pcdata->hometown)
      {
         send_to_char("That town does not belong to your kingdom you fool!\n\r", ch);
         return;
      }
      for (stown = kingdom_table[ch->pcdata->hometown]->first_town; stown; stown = stown->next)
      {
         if (stown->startx == ch->coord->x && stown->starty == ch->coord->y && stown->startmap == ch->map)
            break;
      }
      if (!stown)
      {
         send_to_char("You are not standing at the start point of a town that belongs to your kingdom.\n\r", ch);
         return;
      }
      if (stown == town)
      {
         send_to_char("You are already at your destination silly!\n\r", ch);
         return;
      }
      sprintf(buf, "move");
      if (islinked(ch, town->startx, town->starty, stown->startx, stown->starty, buf, TRUE) == 1)
      {
         for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
         {
            if (victim->coord->x == ch->coord->x && victim->coord->y == ch->coord->y && victim->map == ch->map)
            {
               if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
               {
                  continue;
               }
               if (check_speedwalk_directions(ch, buf) == FALSE)
                  return;
               if (victim->midata)
                  victim->midata->mspeed = 0;
               set_command_buf(victim, buf);
               victim->midata->mspeed = adjust_movement(victim, victim->m10);
               if (victim->midata->mspeed > maxspd)
                  maxspd = victim->midata->mspeed;
               victim->midata->mtick = 0;
               cnt++;
            }
         }
         if (cnt > 1)
         {
            for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
            {
               if (victim->coord->x == ch->coord->x && victim->coord->y == ch->coord->y && victim->map == ch->map)
               {
                  if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
                  {
                     continue;
                  }

                  if (!victim->midata)
                  {
                     if (!victim->midata)
                     {
                        CREATE(victim->midata, MI_DATA, 1);
                        victim->midata->x = victim->coord->x;
                        victim->midata->y = victim->coord->y;
                        victim->midata->map = victim->map;
                        victim->midata->in_room = victim->in_room;
                        victim->midata->command = STRALLOC("");
                     }
                  }
                  victim->midata->mspeed = maxspd;
               }
            }
         }   
      }
      else
      {
         ch_printf(ch, "You are not connected to the town of %s", argument);
         return;
      }
      send_to_char("Done.\n\r", ch);
      sprintf(logb, "%s commanded some troops", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_COMMAND);
      return;
   }  
   if (str_cmp(arg1, "all") && !isdigit(arg1[0]))
   {
      if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
      {
         send_to_char("That soldier is not here to give commands to.\n\r", ch);
         return;
      }
   }    
   else
   {
      if (isdigit(arg1[0]))
      {
         if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
         {
            send_to_char("You can only specify a range while out in the Wilderness.\n\r", ch);
            return;
         }
         if (atoi(arg1) < 1 || atoi(arg1) > 10)
         {
            send_to_char("You are only capable of commanding in a range of 1 to 10.\n\r", ch);
            return;
         }
         range = atoi(arg1);
      }
   }
   argument = one_argument(argument, arg2);
   if (victim != NULL) //Commanding one individual
   {
      if (arg2[0] == '\0')
      {
         send_to_char("You must provide directions such as neen or n2en or coords like 5 -5.\n\r", ch);
         return;
      }
      if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
      {
         send_to_char("This mobile is not a soldier you can command.\n\r", ch);
         return;
      }
      if (is_coord(arg2))
      {
         x = atoi(arg2);
         y = atoi(argument);
         
         if ((victim->coord->x + x) > MAX_X || (victim->coord->x + x) < 1 || (victim->coord->y + y) > MAX_Y
         ||  (victim->coord->y + y) < 1)
         {
            send_to_char("Doing that would put you outside of the map!\n\r", ch);
            return;
         }
         if (!sect_show[(int)map_sector[ch->map][ch->coord->x+x][ch->coord->y+y]].canpass)
         {
            send_to_char("That would put that unit in a inpassable sectortype.\n\r", ch);
            return;
         }
         sprintf(buf, "move");
         if (islinked(ch, victim->coord->x+x, victim->coord->y+y, victim->coord->x, victim->coord->y, buf, FALSE) == 1)
         {
            if (check_speedwalk_directions(ch, buf) == FALSE)
               return;
            if (victim->midata)
               victim->midata->mspeed = 0;
            set_command_buf(victim, buf);
            victim->midata->mspeed = adjust_movement(victim, victim->m10);
            victim->midata->mtick = 0;
         }
         else
         {
            sprintf(buf, "%s I cannot find a route my lord!", ch->name);
            do_tell(victim, buf);
         }
      }
      else
      {         
         if (check_speedwalk_directions(ch, arg2) == FALSE)
            return;
         set_command_buf(victim, arg2);
         if (victim->midata)
            victim->midata->mspeed = 0;
         victim->midata->mtick = 0;
      }
      send_to_char("Done.\n\r", ch);
      sprintf(logb, "%s commanded some troops", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_COMMAND);
      return;
   }
   if (!str_cmp(arg1, "all"))
   {
      cnt = 0;
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if (victim->coord->x == ch->coord->x && victim->coord->y == ch->coord->y && victim->map == ch->map)
         {
            if (arg2[0] == '\0')
            {
               send_to_char("You must provide directions such as neen or n2en or coords like 5 -5.\n\r", ch);
               return;
            }
            if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
            {
               continue;
            }
            if (is_coord(arg2))
            {
               x = atoi(arg2);
               y = atoi(argument);
         
               if ((victim->coord->x + x) > MAX_X || (victim->coord->x + x) < 1 || (victim->coord->y + y) > MAX_Y
               ||  (victim->coord->y + y) < 1)
               {
                  send_to_char("Doing that would put you outside of the map!\n\r", ch);
                  return;
               }
               if (!sect_show[(int)map_sector[ch->map][ch->coord->x+x][ch->coord->y+y]].canpass)
               {
                  send_to_char("That would put that unit in a inpassable sectortype.\n\r", ch);
                  return;
               }
               sprintf(buf, "move");
               cnt++;
               if (islinked(ch, victim->coord->x+x, victim->coord->y+y, victim->coord->x, victim->coord->y, buf, FALSE) == 1)
               {
                  if (check_speedwalk_directions(ch, buf) == FALSE)
                     return;
                  set_command_buf(victim, buf);
                  if (victim->midata)
                     victim->midata->mspeed = 0;
                  victim->midata->mspeed = adjust_movement(victim, victim->m10);
                  if (victim->midata->mspeed > maxspd)
                     maxspd = victim->midata->mspeed;
                  victim->midata->mtick = 0;
                  if (cnt == 10)
                     break;
               }
               else
               {
                  sprintf(buf, "%s I cannot find a route my lord!", ch->name);
                  do_tell(victim, buf);
               }                  
            }
            else
            {
               if (check_speedwalk_directions(ch, arg2) == FALSE)
                  return;
               set_command_buf(victim, arg2);
               cnt++;
               if (victim->midata)
                  victim->midata->mspeed = 0;
               victim->midata->mspeed = adjust_movement(victim, victim->m10);
               if (victim->midata->mspeed > maxspd)
                  maxspd = victim->midata->mspeed;
               victim->midata->mtick = 0;
               if (cnt == 10)
                  break;
            }
         }
      }
      if (cnt > 1)
      {
         for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
         {
            if (victim->coord->x == ch->coord->x && victim->coord->y == ch->coord->y && victim->map == ch->map)
            {
               if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
               {
                  continue;
               }
               cnt++;
               if (!victim->midata)
               {
                  CREATE(victim->midata, MI_DATA, 1);
                  victim->midata->x = victim->coord->x;
                  victim->midata->y = victim->coord->y;
                  victim->midata->map = victim->map;
                  victim->midata->in_room = victim->in_room;
                  victim->midata->command = STRALLOC("");
               }
               victim->midata->mspeed = maxspd;
               if (cnt == 10)
                  break;
            }
         }
      }   
      send_to_char("Done.\n\r", ch);
      sprintf(logb, "%s commanded some troops", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_COMMAND);
      return;
   }
   if (isdigit(arg1[0]))
   {
      cnt = 0;
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if ((victim->map == ch->map) && (abs(victim->coord->x - ch->coord->x) <= atoi(arg1) && abs(victim->coord->y - ch->coord->y) <= atoi(arg1)))
         {
            if (arg2[0] == '\0')
            {
               send_to_char("You must provide directions such as neen or n2en or coords like 5 -5.\n\r", ch);
               return;
            }
            if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
            {
               continue;
            }
            if (is_coord(arg2))
            {
               x = atoi(arg2);
               y = atoi(argument);
         
               if ((victim->coord->x + x) > MAX_X || (victim->coord->x + x) < 1 || (victim->coord->y + y) > MAX_Y
               ||  (victim->coord->y + y) < 1)
               {
                  send_to_char("Doing that would put you outside of the map!\n\r", ch);
                  return;
               }
               if (!sect_show[(int)map_sector[victim->map][victim->coord->x+x][victim->coord->y+y]].canpass)
               {
                  continue;
               }
               sprintf(buf, "move");
               cnt++;
               if (islinked(ch, victim->coord->x+x, victim->coord->y+y, victim->coord->x, victim->coord->y, buf, FALSE) == 1)
               {
                  if (check_speedwalk_directions(ch, buf) == FALSE)
                     return;
                  set_command_buf(victim, buf);
                  if (victim->midata)
                     victim->midata->mspeed = 0;
                  victim->midata->mspeed = adjust_movement(victim, victim->m10);
                  if (victim->midata->mspeed > maxspd)
                     maxspd = victim->midata->mspeed;
                  victim->midata->mtick = 0;
                  if (cnt == 10)
                     break;
               }
               else
               {
                  sprintf(buf, "%s I cannot find a route my lord!", ch->name);
                  do_tell(victim, buf);
               }
            }
            else
            {
               if (check_speedwalk_directions(ch, arg2) == FALSE)
                  continue;
               set_command_buf(victim, arg2);
               cnt++;
               if (victim->midata)
                  victim->midata->mspeed = 0;
               victim->midata->mspeed = adjust_movement(victim, victim->m10);
               if (victim->midata->mspeed > maxspd)
                  maxspd = victim->midata->mspeed;
               victim->midata->mtick = 0;
               if (cnt == 10)
                  break;
            }
         }
      }
      if (cnt > 1)
      {
         for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
         {
            if ((victim->map == ch->map) && (abs(victim->coord->x - ch->coord->x) <= atoi(arg1) && abs(victim->coord->y - ch->coord->y) <= atoi(arg1)))
            {
               if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
               {
                  continue;
               }
               if (!victim->midata)
               {
                  CREATE(victim->midata, MI_DATA, 1);
                  victim->midata->x = victim->coord->x;
                  victim->midata->y = victim->coord->y;
                  victim->midata->map = victim->map;
                  victim->midata->in_room = victim->in_room;
                  victim->midata->command = STRALLOC("");
               }
               victim->midata->mspeed = maxspd;
               if (cnt == 10)
                  break;
            }
         }
      }
      send_to_char("Done.\n\r", ch);
      sprintf(logb, "%s commanded some troops", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_COMMAND);
      return;
   }
   do_command(ch, "");
   return;
}

sh_int check_patrol_route(CHAR_DATA * victim)
{
   sh_int stx, sty, endx, endy, x, y, range;

   range = victim->m2;
   stx = UMAX(victim->m7 - range, 1);
   sty = UMAX(victim->m8 - range, 1);
   endx = UMIN(victim->m7 + range, MAX_X);
   endy = UMIN(victim->m8 + range, MAX_Y);

   //note only check corners because patrol works in a square
   for (x = stx; x <= endx; x++)
      for (y = sty; y <= endy; y++)
      {
         if (x == stx) //Left Side
         {
            if (!sect_show[(int)map_sector[victim->map][x][y]].canpass || map_sector[victim->map][x][y] == SECT_ENTER)
               return FALSE;
            else
               continue;
         }
         if (x == endx) //Right Side
         {
            if (!sect_show[(int)map_sector[victim->map][x][y]].canpass || map_sector[victim->map][x][y] == SECT_ENTER)
               return FALSE;
            else
               continue;
         }
         if (y == sty) //Top Side
         {
            if (!sect_show[(int)map_sector[victim->map][x][y]].canpass || map_sector[victim->map][x][y] == SECT_ENTER)
               return FALSE;
            else
               continue;
         }
         if (y == endy) //Bottom Side
         {
            if (!sect_show[(int)map_sector[victim->map][x][y]].canpass || map_sector[victim->map][x][y] == SECT_ENTER)
               return FALSE;
            else
               continue;
         }
      }
   return TRUE;
}

int give_orders(CHAR_DATA * ch, CHAR_DATA * victim, char *argument, int type)
{
   char arg[MIL];

   for (;;)
   {
      argument = one_argument(argument, arg);

      if (arg[0] == '\0')
         break;

      if (!str_prefix(arg, "stationary"))
      {
         xREMOVE_BIT(victim->miflags, KM_PATROL);
         xREMOVE_BIT(victim->miflags, KM_SENTINEL);
         xSET_BIT(victim->miflags, KM_STATIONARY);
         if (type == 0)
            send_to_char("Changed orders to Stationary, any patrolling has stopped.\n\r", ch);
      }
      if (!str_prefix(arg, "sentinel"))
      {
         xREMOVE_BIT(victim->miflags, KM_PATROL);
         xSET_BIT(victim->miflags, KM_SENTINEL);
         xREMOVE_BIT(victim->miflags, KM_STATIONARY);
         if (type == 0)
            send_to_char("Changed orders to Sentinel, any patrolling has stopped.\n\r", ch);
      }   
      if (!str_prefix(arg, "patrol"))
      {
         argument = one_argument(argument, arg);
         if (!isdigit(arg[0]))
         {
            send_to_char("ORDERS ERROR: No value provided with patrol, stopping.\n\r", ch);
            return FALSE;
         }
         if (atoi(arg) < 1 || atoi(arg) > 15)
         {
            send_to_char("ORDERS ERROR: Value provided with patrol is < 1 or > 15.\n\r", ch);
            return FALSE;
         }
         xREMOVE_BIT(victim->miflags, KM_STATIONARY);
         xREMOVE_BIT(victim->miflags, KM_SENTINEL);
         xSET_BIT(victim->miflags, KM_PATROL);
         victim->m2 = atoi(arg);
         victim->m7 = victim->coord->x;
         victim->m8 = victim->coord->y;
         if (check_patrol_route(victim) == 0)
         {
            send_to_char("There is an unpassable sector in the patrol area, aborting.\n\r", ch);
            xREMOVE_BIT(victim->miflags, KM_PATROL);
            xSET_BIT(victim->miflags, KM_STATIONARY);
            return FALSE;
         }
         if (type == 0)
            send_to_char("Changed orders to Patrolling, all units are now moving.\n\r", ch);
      }
      if (!str_prefix(arg, "warn"))
      {
         argument = one_argument(argument, arg);
         if (!isdigit(arg[0]))
         {
            send_to_char("ORDERS ERROR: No value provided with warn, stopping.\n\r", ch);
            return FALSE;
         }
         if (atoi(arg) < 0 || atoi(arg) > 10)
         {
            send_to_char("ORDERS ERROR: Value provided with warn is < 0 or > 10.\n\r", ch);
            return FALSE;
         }
         xREMOVE_BIT(victim->miflags, KM_ATTACKE);
         xREMOVE_BIT(victim->miflags, KM_ATTACKN);
         xREMOVE_BIT(victim->miflags, KM_ATTACKA);
         xSET_BIT(victim->miflags, KM_WARN);
         victim->m6 = atoi(arg);
         if (type == 0)
            send_to_char("Changed orders to warn only.\n\r", ch);
      }

      if (!str_prefix(arg, "attacke"))
      {
         argument = one_argument(argument, arg);
         if (!isdigit(arg[0]))
         {
            send_to_char("ORDERS ERROR: No value provided with attacke, stopping.\n\r", ch);
            return FALSE;
         }
         if (atoi(arg) < 0 || atoi(arg) > 10)
         {
            send_to_char("ORDERS ERROR: Value provided with attacke is < 0 or > 10.\n\r", ch);
            return FALSE;
         }
         xREMOVE_BIT(victim->miflags, KM_WARN);
         xREMOVE_BIT(victim->miflags, KM_ATTACKN);
         xREMOVE_BIT(victim->miflags, KM_ATTACKA);
         xSET_BIT(victim->miflags, KM_ATTACKE);
         victim->m6 = atoi(arg);
         if (type == 0)
            send_to_char("Changed orders to attack enemy.\n\r", ch);
      }
      if (!str_prefix(arg, "attackn"))
      {
         argument = one_argument(argument, arg);
         if (!isdigit(arg[0]))
         {
            send_to_char("ORDERS ERROR: No value provided with attackn, stopping.\n\r", ch);
            return FALSE;
         }
         if (atoi(arg) < 0 || atoi(arg) > 10)
         {
            send_to_char("ORDERS ERROR: Value provided with attackn is < 0 or > 10.\n\r", ch);
            return FALSE;
         }
         xREMOVE_BIT(victim->miflags, KM_ATTACKE);
         xREMOVE_BIT(victim->miflags, KM_WARN);
         xREMOVE_BIT(victim->miflags, KM_ATTACKA);
         xSET_BIT(victim->miflags, KM_ATTACKN);
         victim->m6 = atoi(arg);
         if (type == 0)
            send_to_char("Changed orders to attack neutral.\n\r", ch);
      }
      if (!str_prefix(arg, "attacka"))
      {
         argument = one_argument(argument, arg);
         if (!isdigit(arg[0]))
         {
            send_to_char("ORDERS ERROR: No value provided with attacka, stopping.\n\r", ch);
            return FALSE;
         }
         if (atoi(arg) < 0 || atoi(arg) > 10)
         {
            send_to_char("ORDERS ERROR: Value provided with attacka is < 0 or > 10.\n\r", ch);
            return FALSE;
         }
         xREMOVE_BIT(victim->miflags, KM_ATTACKE);
         xREMOVE_BIT(victim->miflags, KM_ATTACKN);
         xREMOVE_BIT(victim->miflags, KM_WARN);
         xSET_BIT(victim->miflags, KM_ATTACKA);
         victim->m6 = atoi(arg);
         if (type == 0)
            send_to_char("Changed orders to attack all.\n\r", ch);
      }
      if (!str_prefix(arg, "nopass"))
      {
         if (xIS_SET(victim->miflags, KM_NOPASS))
         {
            xREMOVE_BIT(victim->miflags, KM_NOPASS);
            if (type == 0)
               send_to_char("Removed the orders to let no one pass.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_NOPASS);
            if (type == 0)
               send_to_char("Changed orders to let no one pass.\n\r", ch);
         }
      }
      if (!str_prefix(arg, "needintro"))
      {
         if (xIS_SET(victim->miflags, KM_NEEDINTRO))
         {
            xREMOVE_BIT(victim->miflags, KM_NEEDINTRO);
            if (type == 0)
               send_to_char("Removed the orders to attack individuals whom are unknown.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_NEEDINTRO);
            if (type == 0)
               send_to_char("Changed orders to not attack individuals whom are unknown.\n\r", ch);
         }
      }
      if (!str_prefix(arg, "attackh"))
      {
         if (xIS_SET(victim->miflags, KM_ATTACKH))
         {
            xREMOVE_BIT(victim->miflags, KM_ATTACKH);
            if (type == 0)
               send_to_char("Removed the orders to attack individuals with hoods.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_ATTACKH);
            if (type == 0)
               send_to_char("Changed orders to not attack individuals wearing hoods.\n\r", ch);
         }
      }
      if (!str_prefix(arg, "attackc"))
      {
         if (xIS_SET(victim->miflags, KM_ATTACKC))
         {
            xREMOVE_BIT(victim->miflags, KM_ATTACKC);
            if (type == 0)
               send_to_char("Removed the orders to attack individuals with cloaks.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_ATTACKC);
            if (type == 0)
               send_to_char("Changed orders to not attack individuals wearing cloaks.\n\r", ch);
         }
      }  
      if (!str_prefix(arg, "nocloak"))
      {
         if (xIS_SET(victim->miflags, KM_NOCLOAK))
         {
            xREMOVE_BIT(victim->miflags, KM_NOCLOAK);
            if (type == 0)
               send_to_char("Removed the orders of no pass for individuals in cloaks.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_NOCLOAK);
            if (type == 0)
               send_to_char("Changed orders to not let anyone pass whom is wearing a cloak.\n\r", ch);
         }
      }
      if (!str_prefix(arg, "nohood"))
      {
         if (xIS_SET(victim->miflags, KM_NOHOOD))
         {
            xREMOVE_BIT(victim->miflags, KM_NOHOOD);
            if (type == 0)
               send_to_char("Removed the orders of no pass for individuals with hoods on.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_NOHOOD);
            if (type == 0)
               send_to_char("Changed orders to not let anyone pass whom is wearing a hood.\n\r", ch);
         }
      }  
      if (!str_prefix(arg, "conquer"))
      {
         if (xIS_SET(victim->miflags, KM_CONQUER))
         {
            xREMOVE_BIT(victim->miflags, KM_CONQUER);
            if (type == 0)
               send_to_char("Removed the orders to conquer.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_CONQUER);
            if (type == 0)
               send_to_char("Changed orders to conquer.\n\r", ch);
         }
      }
      if (!str_prefix(arg, "report"))
      {
         xSET_BIT(victim->miflags, KM_REPORT);
         if (type == 0)
            send_to_char("Changed orders to report.\n\r", ch);
      }
      if (!str_prefix(arg, "noassist"))
      {
         if (xIS_SET(victim->miflags, KM_NOASSIST))
         {
            xREMOVE_BIT(victim->miflags, KM_NOASSIST);
            if (type == 0)
               send_to_char("Changed orders to assist once again.\n\r", ch);
         }
         else
         {
            xSET_BIT(victim->miflags, KM_NOASSIST);
            if (type == 0)
               send_to_char("Changed orders to NOASSIST.\n\r", ch);
         }
      }
      if (!str_prefix(arg, "invite"))
      {
         argument = one_argument(argument, arg);
         if (!isdigit(arg[0]))
         {
            send_to_char("ORDERS ERROR: No value provided with invite, stopping.\n\r", ch);
            return FALSE;
         }
         if (atoi(arg) < 0 || atoi(arg) > 10)
         {
            send_to_char("ORDERS ERROR: Value provided with invite is < 0 or > 10.\n\r", ch);
            return FALSE;
         }
         xSET_BIT(victim->miflags, KM_INVITE);
         if (type == 0)
            send_to_char("Changed orders to invite all for help.\n\r", ch);
         victim->m5 = atoi(arg);
      }
   }
   sprintf(logb, "%s changed orders for some troops", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_GIVEORDERS);
   return TRUE;
}

//Can give orders to mob to behave in certain ways
void do_giveorders(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   CHAR_DATA *victim = NULL;
   int num = 0;
   int range = 0;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   /* Only Those who are High Appoint and God+ immortals can set job status for now */
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->mincommand ||
      ch->pcdata->caste == caste_Avatar || ch->pcdata->caste == caste_Legend ||
      ch->pcdata->caste == caste_Ascender || ch->pcdata->caste == caste_Immortal)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: giveorders [mob/all/range] <orders>.\n\r", ch);
      send_to_char("orders - stationary patrol warn attacke attackn attacka nopass report invite noassist sentinel conquer.\n\r         nocloak nohood attackh attackc needintro\n\r", ch);
      send_to_char("Please see the helpfile (help giveorders) for more info.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   if (str_cmp(arg1, "all") && !isdigit(arg1[0]))
   {
      if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
      {
         send_to_char("That soldier is not here to give orders to.\n\r", ch);
         return;
      }
   }
   else
   {
      if (isdigit(arg1[0]))
      {
         if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
         {
            send_to_char("You can only specify a range while out in the Wilderness.\n\r", ch);
            return;
         }
         if (atoi(arg1) < 1 || atoi(arg1) > 10)
         {
            send_to_char("You are only capable of commanding in a range of 1 to 10.\n\r", ch);
            return;
         }
         range = atoi(arg1);
      }
   }
   if (victim != NULL) //Commanding one individual
   {
      if (argument[0] == '\0')
      {
         do_giveorders(ch, "");
         return;
      }
      if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
      {
         send_to_char("This mobile is not a soldier you can command.\n\r", ch);
         return;
      }
      if (give_orders(ch, victim, argument, num) == FALSE)
         return;
   }
   if (!str_cmp(arg1, "all"))
   {
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if (victim->coord->x == ch->coord->x && victim->coord->y == ch->coord->y && victim->map == ch->map)
         {
            if (argument[0] == '\0')
            {
               do_giveorders(ch, "");
               return;
            }
            if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
               continue;
            if (give_orders(ch, victim, argument, num) == FALSE)
               return;
            num++;
         }
      }
   }
   if (isdigit(arg1[0]))
   {
      for (victim = ch->in_room->first_person; victim; victim = victim->next_in_room)
      {
         if ((victim->map == ch->map) && (abs(victim->coord->x - ch->coord->x) <= atoi(arg1) && abs(victim->coord->y - ch->coord->y) <= atoi(arg1)))
         {
            if (argument[0] == '\0')
            {
               do_giveorders(ch, "");
               return;
            }
            if (!xIS_SET(victim->act, ACT_MILITARY) || victim->m4 != ch->pcdata->hometown)
               continue;
            if (give_orders(ch, victim, argument, num) == FALSE)
               return;
            num++;
         }
      }
   }
//   do_giveorders(ch, "");    
   return;
}

void save_sysdata args((SYSTEM_DATA sys));

bool build_onmap(int sector)
{
   if (sector == SECT_MOUNTAIN || sector == SECT_WATER_NOSWIM 
   ||  sector == SECT_UNDERWATER || sector == SECT_AIR || sector == SECT_DESERT || sector == SECT_DUNNO
   ||  sector == SECT_OCEANFLOOR || sector == SECT_UNDERGROUND || sector == SECT_ENTER
   ||  sector == SECT_MINEGOLD || sector == SECT_MINEIRON || sector == SECT_RIVER || sector == SECT_ICE
   ||  sector == SECT_SHORE || sector == SECT_OCEAN || sector == SECT_LAVA || sector == SECT_TREE
   ||  sector == SECT_QUICKSAND || sector == SECT_SGOLD || sector == SECT_NGOLD || sector == SECT_SIRON
   ||  sector == SECT_NIRON || sector == SECT_WALL || sector == SECT_GLACIER || sector == SECT_EXIT
   ||  sector == SECT_BRIDGE || sector == SECT_VOID || sector == SECT_STABLE || sector == SECT_FIRE
   ||  sector == SECT_DWALL || sector == SECT_NBWALL || sector == SECT_DOOR || sector == SECT_CDOOR
   ||  sector == SECT_LDOOR || sector == SECT_INSIDE || sector == SECT_HOLD)      
      return FALSE;
   else
      return TRUE;
}

void do_startkingdom(CHAR_DATA * ch, char *argument)
{
   struct kingdom_data *kingdom;
   char buf[MSL];
   char name[MSL];
   char *mapname;
   DOOR_LIST *dlist;
   TOWN_DATA *town;
   int sector;
   int x;
   int y;
   int map = ch->map;
   int size = get_control_size(3);

   if (argument[0] == '\0')
   {
      send_to_char("Need to provide a name for the kingdom.\n\r", ch);
      return;
   }
   if (strlen(argument) > 15)
   {
      send_to_char("Your kingdom name cannot be more than 15 characters.\n\r", ch);
      return;
   }
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only use this command if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (ch->pcdata->hometown > 1)
   {
      send_to_char("You have to leave your kingdom before you can create a new one.\n\r", ch);
      return;
   }
   for (x = 2; x < sysdata.max_kingdom; x++)
   {
      if (!str_cmp(kingdom_table[x]->name, argument))
      {
         send_to_char("There is already a kingdom with that name, choose another.\n\r", ch);
         return;
      }
      for (town = kingdom_table[x]->first_town; town; town = town->next)
      {
         if (!str_cmp(town->name, argument))
         {
            send_to_char("There is already a town with that name, choose another.\n\r", ch);
            return;
         }   
      }
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!build_onmap(sector))
   {
      send_to_char("You cannot build a town here.  Help startkingdom for a list of sectors.\n\r", ch);
      return;
   }

   if (ch->gold < 400000 && !sysdata.resetgame)
   {
      send_to_char("Costs 400,000 to startup a new kingdom.\n\r", ch);
      return;
   }
   for (x = ch->coord->x - size; x <= ch->coord->x+size; x++)
   {
      if (x > ch->coord->x)
      {
         for (y = ch->coord->y - abs(x - ch->coord->x - size); y <= ch->coord->y + abs(x - ch->coord->x - size); y++)
         {
            if (kingdom_sector[map][x][y] > 1)
            {
               send_to_char("You cannot claim this land because your aoc would interfere with another\n\r"
                            "Kingdom.  Choose another spot (help aoc).\n\r", ch);
               return;
            }
         }
      }
      else
      {   
         for (y = ch->coord->y - abs(x - ch->coord->x + size); y <= ch->coord->y + abs(x - ch->coord->x + size); y++)
         {
            if (kingdom_sector[map][x][y] > 1)
            {
               send_to_char("You cannot claim this land because your aoc would interfere with another\n\r"
                            "Kingdom.  Choose another spot (help aoc).\n\r", ch);
               return;
            }
         }
      }
   }
   //Start kingdom creation
   CREATE(kingdom, struct kingdom_data, 1);
   kingdom->num = sysdata.max_kingdom;
   sysdata.max_kingdom++;

   for (x = 0; x <= 25; x++)
   {
      kingdom->mob_que[x] = 0;
      kingdom->obj_que[x] = 0;
   }
   for (x = 0; x < sysdata.max_kingdom; x++)
      kingdom->cpeace[x] = -1;
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      kingdom->peace[x] = 1;
   }
   if (kingdom->name)
      STRFREE(kingdom->name);
   kingdom->name = STRALLOC(argument);
   if (kingdom->ruler)
      STRFREE(kingdom->ruler);
   kingdom->ruler = STRALLOC(ch->name);
   if (kingdom->dtown)
      STRFREE(kingdom->dtown);
   kingdom->dtown = STRALLOC(argument);
   kingdom->tree_tax = kingdom->corn_tax = kingdom->grain_tax = kingdom->iron_tax = kingdom->gold_tax = kingdom->stone_tax = kingdom->fish_tax = 150;
   kingdom->salestax = 75;
   kingdom->poptax = 0;
   kingdom->minbuild = caste_Minister;
   kingdom->minplace = caste_Minister;
   kingdom->minappoint = caste_Minister;
   kingdom->minhappoint = caste_Minister;
   kingdom->mintoperate = caste_Minister;
   kingdom->minwithdraw = caste_Minister;
   kingdom->mincommand = caste_Minister;
   kingdom->mingeneral = caste_Minister;
   kingdom->minreadlog = caste_Minister;
   kingdom->mindepository = caste_Minister;
   kingdom->minlogsettings = caste_Minister;
   kingdom->mintrainertax = caste_Minister;
   kingdom->minswitchtown = caste_Minister;
   kingdom->minbooktax = caste_Minister;
   kingdom->tier1 = 100;
   kingdom->tier2 = 100;
   kingdom->tier3 = 100;
   kingdom->tier4 = 100;
   kingdom->tvisitor = 100;
   kingdom->tier1book = 100;
   kingdom->tier2book = 100;
   kingdom->tier3book = 100;
   kingdom->tier4book = 100;
   kingdom->bvisitor = 100;
   kingdom->maxlinelog = 500;
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      if (x < sysdata.max_kingdom - 1)
      {
         kingdom_table[x]->cpeace[sysdata.max_kingdom - 1] = -1;
         kingdom_table[x]->peace[sysdata.max_kingdom - 1] = 1;
      }
   }
   kingdom_table[kingdom->num] = kingdom;
   ch->pcdata->hometown = kingdom->num;
  /* CREATE(portal, struct portal_data, 1);

   for (;;)
   {
      x = ch->coord->x + number_range(-10, 10);
      y = ch->coord->y + number_range(-10, 10);

      sector = map_sector[ch->map][x][y];
      if (sector == SECT_FIELD || sector == SECT_PATH || sector == SECT_ROAD || sector == SECT_PLAINS || sector == SECT_HILLS)
         break;
   }
   portal->x = x;
   portal->y = y;
   portal->map = ch->map;
   sprintf(buf, "%s Portal", argument);
   portal->desc = STRALLOC(buf);
   portal_show[sysdata.last_portal] = portal;
   sysdata.last_portal++;
   write_portal_file();*/

   //Begin Town Creation
   sprintf(name, "%s", argument);
   sprintf(buf, "%s is creating a new kingdom", ch->name);
   log_string_plus(buf, LOG_NORMAL, ch->level);
   CREATE(town, TOWN_DATA, 1);
   LINK(town, kingdom_table[kingdom->num]->first_town, kingdom_table[kingdom->num]->last_town, next, prev);
   town->name = STRALLOC(argument);
   town->kpid = sysdata.top_kpid++;
   town->tpid = sysdata.top_tpid++;
   kingdom->kpid = town->kpid;
   ch->pcdata->kingdompid = town->kpid;
   
   save_sysdata(sysdata);
   town->roomcoords[1][0] = ch->coord->x;
   town->roomcoords[1][1] = ch->coord->y;
   town->roomcoords[1][2] = ch->map;
   sprintf(buf, "The first room of %s", town->name);
   sprintf(town->roomtitles[1], buf);
   //town->roomtitles[1] = STRALLOC(buf);
   town->kingdom = ch->pcdata->hometown;
   town->mayor = STRALLOC(ch->name);
   ch->pcdata->town = town;
   town->maxsize = 15;
   town->size = 3;
   town->rooms = 1;
   town->moral = 13;
   town->hold = 30000;
   town->banksize = get_banksize(town->size);
   town->month = time(0);
   town->growthcheck = time(0);
   town->corn = 5000;
   town->coins = 400000;
   
   town->death[0] = ch->coord->x;
   town->death[1] = ch->coord->y;
   town->death[2] = ch->map;
   town->recall[0] = ch->coord->x;
   town->recall[1] = ch->coord->y;
   town->recall[2] = ch->map;
   town->barracks[0] = ch->coord->x;
   town->barracks[1] = ch->coord->y;
   town->barracks[2] = ch->map;
   town->startx = ch->coord->x;
   town->starty = ch->coord->y;
   town->startmap = ch->map;
   
   town->poptax = 65;
   town->salestax = 75;
   town->minhappoint = caste_Minister;
   town->minwithdraw = caste_Minister;
   
   for (x = town->startx - size; x <= town->startx+size; x++)
   {
      for (y = town->starty - size; y <= town->starty+size; y++)
      {
         kingdom_sector[map][x][y] = town->kingdom;
      }
   } 

   kingdom_sector[ch->map][ch->coord->x][ch->coord->y] = town->kingdom;
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_PAVE;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
   //Kind of need this I guess
   CREATE(dlist, DOOR_LIST, 1);
   LINK(dlist, ch->pcdata->town->first_doorlist, ch->pcdata->town->last_doorlist, next, prev);
   if (kingdom_table[kingdom->num]->first_ore == NULL) //no depository loaded up yet...
   {
      DEPO_ORE_DATA *dore;
      DEPO_WEAPON_DATA *dweapon;
      SLAB_DATA *slab;	
      FORGE_DATA *forge;
      for (slab = first_slab; slab ; slab = slab->next)
      {
         CREATE(dore, DEPO_ORE_DATA, 1);
         dore->vnum = slab->vnum;
         LINK(dore, kingdom_table[kingdom->num]->first_ore, kingdom_table[kingdom->num]->last_ore, next, prev);
         for (forge = first_forge; forge; forge = forge->next)
         {
            CREATE(dweapon, DEPO_WEAPON_DATA, 1);
            dweapon->vnum = forge->vnum;
            LINK(dweapon, dore->first_weapon, dore->last_weapon, next, prev);
         }
      }
   }
   write_kingdom_file(ch->pcdata->hometown);
   write_kingdom_list();
   save_sysdata(sysdata);
   mapname = get_map_name(ch->map);
   save_map(mapname, ch->map);
   if (ch->pcdata->caste < caste_King)
      ch->pcdata->caste = caste_King;
   if (!sysdata.resetgame)
      ch->gold -= 400000;
   save_char_obj(ch);
   send_to_char("Your town has been created at the spot you stand.\n\r", ch);
   return;
}
/*
void do_startkingdom(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *robj;
   MOB_INDEX_DATA *imob;
   OBJ_INDEX_DATA *iobj;
   struct kingdom_data *kingdom;
   ROOM_INDEX_DATA *room;
   EXIT_DATA *xit;
   EXTRA_DESCR_DATA *ed;
   char buf[MSL];
   char name[MSL];
   char filename[MSL];
   AREA_DATA *tarea;
   char *mapname;
   int start = ROOM_VNUM_STOWN;
   int none = 1;
   int sector;
   int x;
   int y;
   int fnd = 0;

   if (argument[0] == '\0')
   {
      send_to_char("Need to provide a name for the kingdom.\n\r", ch);
      return;
   }
   if (strlen(argument) > 20)
   {
      send_to_char("Your kingdom name cannot be more than 20 characters.\n\r", ch);
      return;
   }
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only use this command if you are in the Wilderness.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (sector != SECT_FIELD && sector != SECT_PATH && sector != SECT_ROAD && sector != SECT_PLAINS && sector != SECT_HILLS)
   {
      send_to_char("You can only start a town on fields, path, road, plains, or hills.\n\r", ch);
      return;
   }
   for (x = ch->coord->x - 10; x <= ch->coord->x + 10; x++)
   {
      for (y = ch->coord->y - 10; y <= ch->coord->y + 10; y++)
      {
         sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
         if (sector == SECT_FIELD || sector == SECT_PATH || sector == SECT_ROAD || sector == SECT_PLAINS || sector == SECT_HILLS)
         {
            fnd = 1;
            break;
         }
      }
   }
   if (fnd == 0)
   {
      send_to_char("You need some nearby Fields, path, road, plains, or hills, to start a kingdom.\n\r", ch);
      return;
   }

   if (ch->gold < 400000)
   {
      send_to_char("Costs 400,000 to startup a new kingdom.\n\r", ch);
      return;
   }
   CREATE(kingdom, struct kingdom_data, 1);
   kingdom->num = sysdata.max_kingdom;
   sysdata.max_kingdom++;

   for (x = 0; x <= 25; x++)
   {
      kingdom->mob_que[x] = 0;
      kingdom->obj_que[x] = 0;
   }
   for (x = 0; x < sysdata.max_kingdom; x++)
      kingdom->cpeace[x] = -1;
   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      kingdom->peace[x] = 1;
   }
   if (kingdom->name)
      STRFREE(kingdom->name);
   kingdom->name = STRALLOC(argument);
   if (kingdom->ruler)
      STRFREE(kingdom->ruler);
   kingdom->ruler = STRALLOC(ch->name);
   if (kingdom->dtown)
      STRFREE(kingdom->dtown);
   kingdom->dtown = STRALLOC(argument);

   for (x = 0; x < sysdata.max_kingdom; x++)
   {
      if (x < sysdata.max_kingdom - 1)
      {
         kingdom_table[x]->cpeace[sysdata.max_kingdom - 1] = -1;
         kingdom_table[x]->peace[sysdata.max_kingdom - 1] = 1;
      }
   }
   kingdom_table[kingdom->num] = kingdom;
   ch->pcdata->hometown = kingdom->num;
   sprintf(name, "%s.are", argument);
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (!str_cmp(name, tarea->filename))
      {
         send_to_char("There is an area already with that filename.\n\r", ch);
         return;
      }
   }
   for (tarea = first_build; tarea; tarea = tarea->next)
   {
      if (!str_cmp(name, tarea->filename))
      {
         send_to_char("There is an area already with that filename.\n\r", ch);
         return;
      }
   }
   for (tarea = first_asort; tarea; tarea = tarea->next_sort)
   {
      if (IS_SET(tarea->status, AREA_DELETED))
         continue;
      if (tarea->low_r_vnum < ROOM_VNUM_STOWN)
         continue;
      if (tarea->hi_r_vnum < ROOM_VNUM_ETOWN)
         none = 0;
      if (!tarea->next_sort || (tarea->next_sort->low_r_vnum - tarea->hi_r_vnum >= 500
            && tarea->next_sort->low_m_vnum - tarea->hi_m_vnum >= 500 && tarea->next_sort->low_o_vnum - tarea->hi_o_vnum >= 500))
      {
         start = tarea->hi_r_vnum;
         if (tarea->hi_m_vnum > start)
            start = tarea->hi_m_vnum;
         if (tarea->hi_o_vnum > start)
            start = tarea->hi_o_vnum;
         if (start + 500 > ROOM_VNUM_ETOWN)
         {
            send_to_char("There is no more space, tell an immortal please.\n\r", ch);
            return;
         }
         start++;
         break;
      }
   }
   if (!tarea && !none)
   {
      send_to_char("There is no more space, tell an immortal please.\n\r", ch);
      return;
   }
   sprintf(name, "%s", argument);
   sprintf(buf, "%s is creating a new kingdom", ch->name);
   log_string_plus(buf, LOG_NORMAL, ch->level);
   CREATE(tarea, AREA_DATA, 1);
   LINK(tarea, first_area, last_area, next, prev);
   tarea->first_reset = NULL;
   tarea->last_reset = NULL;
   tarea->name = str_dup(argument);
   sprintf(filename, "%s.are", parse_save_file(name));
   tarea->filename = str_dup(filename);
   sprintf(buf, "%s", ch->name);
   tarea->author = STRALLOC(buf);
   tarea->age = 0;
   tarea->kpid = sysdata.top_kpid++;
   save_sysdata(sysdata);
   tarea->nplayer = 0;
   tarea->x = ch->coord->x;
   tarea->y = ch->coord->y;
   tarea->map = ch->map;
   tarea->kingdom = ch->pcdata->hometown;
   tarea->kowner = STRALLOC(buf);
   tarea->low_r_vnum = start;
   tarea->low_o_vnum = start;
   tarea->low_m_vnum = start;
   tarea->hi_r_vnum = start + 499;
   tarea->hi_o_vnum = start + 499;
   tarea->hi_m_vnum = start + 499;
   SET_BIT(tarea->flags, AFLAG_NOWDAM);
   SET_BIT(tarea->flags, AFLAG_ANITEM);
   SET_BIT(tarea->flags, AFLAG_CARPENTER);
   sprintf(buf, "The streets of the Kingdom suddenly come alive.");
   tarea->resetmsg = str_dup(buf);
   tarea->reset_frequency = 15;
   tarea->low_soft_range = 1;
   tarea->hi_soft_range = 75;
   tarea->low_hard_range = 1;
   tarea->hi_hard_range = 75;
   tarea->population = 1000;
   tarea->death = ROOM_VNUM_ALTAR;
   tarea->recall = ROOM_VNUM_TEMPLE;

   room = make_room(start);
   if (!room)
   {
      bug("uselicense maclass: make_room failed to initialize.", 0);
      return;
   }
   room->area = tarea;
   xSET_BIT(room->room_flags, ROOM_NO_MOB);
   xSET_BIT(room->room_flags, ROOM_WILDERNESS);
   STRFREE(room->name);
   sprintf(buf, "Kingdom Entrance");
   room->name = STRALLOC(buf);

   room = make_room(start + 499);
   if (!room)
   {
      bug("uselicense maclass: make_room failed to initialize.", 0);
      return;
   }
   room->area = tarea;
   xSET_BIT(room->room_flags, ROOM_NO_MOB);
   xSET_BIT(room->room_flags, ROOM_WILDERNESS);

   imob = make_mobile(start + 499, 0, "last mob");
   if (!imob)
   {
      log_string("uselicense maclass: make_mobile failed to initialize.");
      return;
   }
   imob = make_mobile(start, 0, "first mob");
   if (!imob)
   {
      log_string("uselicense maclass: make_mobile failed to initialize.");
      return;
   }

   iobj = make_object(start + 499, 0, "last obj", 0);
   if (!iobj)
   {
      log_string("uselicense maclass: make_object failed to initialize.");
      return;
   }
   iobj = make_object(start, 0, "first obj", 0);
   if (!iobj)
   {
      log_string("uselicense maclass: make_object failed to initialize.");
      return;
   }
   STRFREE(iobj->name);
   sprintf(buf, "Sign");
   iobj->name = STRALLOC(buf);
   STRFREE(iobj->short_descr);
   iobj->short_descr = STRALLOC(buf);
   sprintf(buf, "A small sign is firmly stuck in the ground.");
   STRFREE(iobj->description);
   iobj->description = STRALLOC(buf);
   REMOVE_BIT(iobj->wear_flags, ITEM_TAKE);
   iobj->item_type = ITEM_FURNITURE;
   ed = SetOExtraProto(iobj, "sign");
   if (ed->description)
      STRFREE(ed->description);
   sprintf(buf, "This town is part of a kingdom, do not enter unless authorized.\n\r");
   ed->description = STRALLOC(buf);
   iobj->cost = 20;
   xREMOVE_BIT(iobj->extra_flags, ITEM_PROTOTYPE);
   robj = create_object(iobj, 0);
   room = get_room_index(start);
   room->sector_type = SECT_ROAD;
   obj_to_room(robj, room, ch);
   robj->coord->x = -1;
   robj->coord->y = -1;
   robj->map = -1;
   add_obj_reset(tarea, 'O', robj, 1, room->vnum);

   xit = make_exit(get_room_index(start), get_room_index(OVERLAND_SOLAN), 2);
   xit->keyword = STRALLOC("");
   xit->description = STRALLOC("");
   xit->key = -1;
   xit->exit_info = 0;
   SET_BIT(xit->exit_info, EX_OVERLAND);
   xit->coord->x = ch->coord->x;
   xit->coord->y = ch->coord->y;
   add_entrance(-1, ch->map, ch->coord->x, ch->coord->y, -1, -1, start);
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_EXIT;
   sort_area(tarea, FALSE);
   fold_area(tarea, tarea->filename, FALSE, 1);
   fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
   write_area_list();
   write_kingdom_file(ch->pcdata->hometown);
   write_kingdom_list();
   save_sysdata(sysdata);
   mapname = get_map_name(ch->map);
   save_map(mapname, ch->map);
   ch->pcdata->town = tarea;
   if (ch->pcdata->caste < caste_King)
      ch->pcdata->caste = caste_King;
   ch->gold -= 400000;
   save_char_obj(ch);
   send_to_char("Your town has been created at the spot you stand.\n\r", ch);
   return;
} */

//Uses licenses you have on you.
void do_uselicense(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   MOB_INDEX_DATA *imob;
   CHAR_DATA *mob;
   SHOP_DATA *shop;
   char arg[MIL];
   char arg2[MIL];
   char buf[MSL];
   char name[MSL];
   DOOR_LIST *dlist;
   int x;
   TOWN_DATA *town;
   TOWN_DATA *dtown;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: uselicense <license>\n\rlicenses: mclass maclass\n\r", ch);
      send_to_char("Syntax: uselicense maclass <expansion town> <new town name>\n\r", ch);
      send_to_char("Note: must provide name of town with maclass\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "maclass"))
   {
      char *mapname;
      int sector;
      int y;
      int map = ch->map;
      int size = get_control_size(2);
      
      argument = one_argument(argument, arg2);
      
      if (check_npc(ch))
         return;
         
      if (ch->pcdata->hometown <= 1)
      {
         send_to_char("You need to be in a kingdom to use this command.\n\r", ch);
         return;
      }
      
      if (arg2[0] == '\0')
      {
         send_to_char("Need to provide a name of a city to build from.\n\r", ch);
         return;
      }
      
      dtown = get_town(arg2);
      
      if (!dtown)
      {
         send_to_char("That is not a town, try again.\n\r", ch);
         return;
      }
      if (dtown->kingdom != ch->pcdata->hometown)
      {
         send_to_char("That town does not belong in your kingdom.\n\r", ch);
         return;
      }
      if (dtown->allowexpansions == 0)
      {
         send_to_char("That town is currently not allowing Expansions from it.\n\r", ch);
         return;
      }

      if (argument[0] == '\0')
      {
         send_to_char("Need to provide a name for the city.\n\r", ch);
         return;
      }
      if (strlen(argument) > 15)
      {
         send_to_char("Your hometown name cannot be more than 15 characters.\n\r", ch);
         return;
      }
      if (!IS_ONMAP_FLAG(ch))
      {
         send_to_char("You can only use this command if you are in the Wilderness.\n\r", ch);
         return;
      }
      sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
      if (!build_onmap(sector))
      {
         send_to_char("You cannot build a town here.  Help startkingdom for a list of sectors.\n\r", ch);
         return;
      }

      for (obj = ch->first_carrying; obj; obj = obj->next_content)
         if (obj->pIndexData->vnum == OBJ_KINGDOM_MAYOR)
            break;

      if (!obj)
      {
         send_to_char("You need to have a Mayor's License on you.\n\r", ch);
         return;
      }
      
      if (dtown->coins < 80000)
      {
         send_to_char("Costs 80 thousand to startup a new town.\n\r", ch);
         return;
      }
      if (dtown->lumber < 30000)
      {
         send_to_char("Costs 30,000 units of wood to startup a new town.\n\r", ch);
         return;
      }
      if (dtown->stone < 20000)
      {
         send_to_char("Costs 20,000 units of stone to startup a new town.\n\r", ch);
         return;
      }
      for (x = 2; x < sysdata.max_kingdom; x++)
      {
         if (!str_cmp(kingdom_table[x]->name, argument))
         {
            send_to_char("There is already a kingdom with that name, choose another.\n\r", ch);
            return;
         }
         for (town = kingdom_table[x]->first_town; town; town = town->next)
         {
            if (!str_cmp(town->name, argument))
            {
               send_to_char("There is already a town with that name, choose another.\n\r", ch);
               return;
            }   
         }
      }
      for (x = ch->coord->x - size; x <= ch->coord->x+size; x++)
      {
         if (x > ch->coord->x)
         {
            for (y = ch->coord->y - abs(x - ch->coord->x - size); y <= ch->coord->y + abs(x - ch->coord->x - size); y++)
            {
               if (kingdom_sector[map][x][y] > 1 && kingdom_sector[map][x][y] != ch->pcdata->hometown)
               {
                  send_to_char("You cannot claim this land because your aoc would interfere with another\n\r"
                               "Kingdom.  Choose another spot (help aoc).\n\r", ch);
                  return;
               }
            }
         }
         else
         {   
            for (y = ch->coord->y - abs(x - ch->coord->x + size); y <= ch->coord->y + abs(x - ch->coord->x + size); y++)
            {
               if (kingdom_sector[map][x][y] > 1 && kingdom_sector[map][x][y] != ch->pcdata->hometown)
               {
                  send_to_char("You cannot claim this land because your aoc would interfere with another\n\r"
                               "Kingdom.  Choose another spot (help aoc).\n\r", ch);
                  return;
               }
            }
         }
      }
      if (islinked(ch, dtown->startx, dtown->starty, ch->coord->x, ch->coord->y, "newtown", 2) != 1)
      {
         send_to_char("This town is either not connected to your default town or out of range.\n\r", ch);
         return;
      }
      if (dtown->size < 5) //automatic size reduction
      {
         dtown->size--;
         sprintf(buf, "---SIZE REDUCTION--- %s lost size because of the expansion of %s", dtown->name, argument);
         write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_USELICENSE);
      }
      else
      {
         if (dtown->expansions-1 < 0)
         {
            dtown->size--;
            sprintf(buf, "---SIZE REDUCTION--- %s lost size because of the expansion of %s", dtown->name, argument);
            write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_USELICENSE);   
         }
         else
            dtown->expansions--;
      }
      dtown->coins -= 80000;
      dtown->lumber -= 30000;
      dtown->stone -= 20000;

      sprintf(name, "%s", argument);
      sprintf(buf, "%s is creating a new town", ch->name);
      log_string_plus(buf, LOG_NORMAL, ch->level);
      CREATE(town, TOWN_DATA, 1);
      LINK(town, kingdom_table[ch->pcdata->hometown]->first_town, kingdom_table[ch->pcdata->hometown]->last_town, next, prev);
      town->name = STRALLOC(argument);
      town->kpid = kingdom_table[ch->pcdata->hometown]->kpid;
      town->tpid = sysdata.top_tpid++;
   
      save_sysdata(sysdata);
      town->roomcoords[1][0] = ch->coord->x;
      town->roomcoords[1][1] = ch->coord->y;
      town->roomcoords[1][2] = ch->map;
      sprintf(buf, "The first room of %s", town->name);
      sprintf(town->roomtitles[1], buf);
      //town->roomtitles[1] = STRALLOC(buf);
      town->kingdom = ch->pcdata->hometown;
      town->mayor = STRALLOC(ch->name);
      ch->pcdata->town = town;
      town->maxsize = 10;
      town->size = 2;
      town->rooms = 1;
      town->moral = 7;
      town->hold = 30000;
      town->banksize = get_banksize(town->size);
      town->month = time(0);
      town->growthcheck = time(0);
      town->corn = 1000;
      town->coins = 40000;
   
      town->death[0] = ch->coord->x;
      town->death[1] = ch->coord->y;
      town->death[2] = ch->map;
      town->recall[0] = ch->coord->x;
      town->recall[1] = ch->coord->y;
      town->recall[2] = ch->map;
      town->barracks[0] = ch->coord->x;
      town->barracks[1] = ch->coord->y;
      town->barracks[2] = ch->map;
      town->startx = ch->coord->x;
      town->starty = ch->coord->y;
      town->startmap = ch->map;
      town->poptax = 65;
      town->salestax = 75;
      town->minhappoint = caste_Minister;
      town->minwithdraw = caste_Minister;
      for (x = town->startx - size; x <= town->startx+size; x++)
      {
         for (y = town->starty - size; y <= town->starty+size; y++)
         {
            kingdom_sector[map][x][y] = town->kingdom;
         }
      } 
      kingdom_sector[ch->map][ch->coord->x][ch->coord->y] = town->kingdom;
      map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_PAVE;
      resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
      //Kind of need this I guess
      CREATE(dlist, DOOR_LIST, 1);
      LINK(dlist, ch->pcdata->town->first_doorlist, ch->pcdata->town->last_doorlist, next, prev);
      write_kingdom_file(ch->pcdata->hometown);
      write_kingdom_list();
      save_sysdata(sysdata);
      mapname = get_map_name(ch->map);
      save_map(mapname, ch->map);
      //ch->pcdata->town = tarea;
      if (ch->pcdata->caste < caste_King)
         ch->pcdata->caste = caste_King;
      save_char_obj(ch);
      send_to_char("Your town has been created at the spot you stand.\n\r", ch);
      sprintf(buf, "---NEW TOWN--- %s has expanded from %s because of %s", argument, dtown->name, ch->name);
      write_kingdom_logfile(ch->pcdata->hometown, buf, KLOG_USELICENSE);
      return;
   }

   if (!str_cmp(arg, "mclass"))
   {
      if (get_mob_index(ch->in_room->vnum) != NULL)
      {
         send_to_char("Sorry there is already a mob being used here, pick another room.\n\r", ch);
         return;
      }
      if (ch->in_room->area->kingdom != ch->pcdata->hometown)
      {
         send_to_char("You can only use licenses in your hometown.\n\r", ch);
         return;
      }
      if (ch->pcdata->keeper > 0)
      {
         send_to_char("You already have a shopkeeper, sorry.\n\r", ch);
         return;
      }
      if (ch->pcdata->caste < caste_Merchant)
      {
         send_to_char("You need to atleast have merchant caste or higher.\n\r", ch);
         return;
      }
      for (obj = ch->first_carrying; obj; obj = obj->next_content)
         if (obj->pIndexData->vnum == OBJ_KINGDOM_MERCHANT)
            break;

      if (!obj)
      {
         send_to_char("You need to have a M class License on you.\n\r", ch);
         return;
      }
      ch->pcdata->keeper = ch->in_room->vnum;
      sprintf(buf, "%s's Shopkeeper", ch->name);

      imob = make_mobile(ch->in_room->vnum, 0, buf);
      STRFREE(imob->short_descr);
      STRFREE(imob->long_descr);
      STRFREE(imob->description);
      imob->short_descr = STRALLOC(buf);
      sprintf(buf, "%s's Shopkeeper is here waiting for the next costumer to arrive.\n\r", ch->name);
      imob->long_descr = STRALLOC(buf);
      sprintf(buf, "An average looking human that is doing business for %s.\n\r", ch->name);
      imob->description = STRALLOC(buf);
      imob->level = 0;
      imob->hitnodice = 5;
      imob->hitsizedice = 5;
      imob->hitplus = 10000;
      imob->sex = 1;
      xSET_BIT(imob->act, ACT_CASTEMOB);
      xSET_BIT(imob->act, ACT_PACIFIST);
      xSET_BIT(imob->act, ACT_SENTINEL);
      xSET_BIT(imob->act, ACT_IMMORTAL);
      xREMOVE_BIT(imob->act, ACT_PROTOTYPE);
      imob->m1 = 50;
      imob->m2 = 100000;
      imob->m3 = 100;
      imob->m4 = 1;
      imob->m5 = 50;
      imob->gold = 1000;
      imob->hitroll = 5; //creates complex mob so m values will save
      mob = create_mobile(imob);
      char_to_room(mob, ch->in_room);

      CREATE(shop, SHOP_DATA, 1);
      LINK(shop, first_shop, last_shop, next, prev);
      shop->keeper = mob->pIndexData->vnum;
      shop->profit_buy = 120;
      shop->profit_sell = 90;
      shop->open_hour = 0;
      shop->close_hour = 23;
      mob->pIndexData->pShop = shop;
      add_reset(ch->in_room->area, 'M', 1, mob->pIndexData->vnum, 1, mob->in_room->vnum, -1, -1, -1, -1, 0, 0);
      fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 1);
      separate_obj(obj);
      obj_from_char(obj);
      extract_obj(obj);
      sprintf(logb, "%s used the M License", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_USELICENSE);
      return;
   }
   do_uselicense(ch, "");
   return;
}

void do_changes(CHAR_DATA *ch, char *argument)
{
   FILE *fpout;
   int lines;
   char filename[MIL];
   char buf[MSL];
   char arg[MIL];
   int c;
   int num = 0;
   int cnt = 0;
   static char cbuf[MSL];
   FILE *ofp;
   FILE *nfp;
   char nfilename[MSL];
   int line = 1;
   
   if (IS_NPC(ch))
      return;

   if (!ch->desc)
   {
      return;
   }
   
   switch (ch->substate)
   {
      default:
         break;
      case SUB_CHANGES:
         sprintf(cbuf, "%s", copy_buffer(ch));
         stop_editing(ch);
         ch->substate = ch->tempnum;
         return;
   }
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  changes <num of lines>     - Max count for lines is 100\n\r", ch);
      if (get_trust(ch) >= LEVEL_STAFF)
      {
         send_to_char("Syntax:  changes edit\n\r", ch);
         send_to_char("Syntax:  changes view\n\r", ch);
         send_to_char("Syntax:  changes post\n\r", ch);
         send_to_char("Syntax:  changes delete <lines>\n\r", ch);
      }
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "edit"))
   {
   	 if(get_trust(ch) < LEVEL_STAFF) {
   	   send_to_char("You can't do that!\n\r",ch);
   	   return;
   	 }
    //  if (get_trust(ch) < LEVEL_STAFF && /* Tracker1 */
    //     str_cmp(ch->pcdata->council_name, "Code Council"))
    //     CHECK_SUBRESTRICTED(ch);
      ch->tempnum = SUB_NONE;
      ch->substate = SUB_CHANGES;
      ch->dest_buf = NULL;
      start_editing(ch, cbuf);
      editor_desc_printf(ch, "Changes to Change file");   
      return;
   }
   if (!str_cmp(arg, "view"))
   {
   	 if(get_trust(ch) < LEVEL_STAFF) {
   	   send_to_char("You can't do that!\n\r",ch);
   	   return;
   	 }
      ch_printf(ch, cbuf);
      return;
   }
   if (!str_cmp(arg, "delete"))
   {
   	 if(get_trust(ch) < LEVEL_STAFF) {
   	   send_to_char("You can't do that!\n\r",ch);
   	   return;
   	 }

      if (atoi(argument) < 1 || atoi(argument) > 200)
      {
         send_to_char("Range is 1-200 lines.\n\r", ch);
         return;
      }
      num = atoi(argument);
      sprintf(filename, "%s", CHANGES_LIST);
      if ((ofp = fopen(filename, "r")) == NULL)
      {
         send_to_char("Cannot post, the original change file does not exist.\n\r", ch);
         return;
      }
      sprintf(nfilename, "%s", CCHANGES_LIST);
      if ((nfp = fopen(nfilename, "w")) == NULL)
      {
         send_to_char("Cannot open the new change file to post to it.\n\r", ch);
         return;
      }
      for(;;)
      {        
         if (line == 7)
         {
            cnt = 0;
            for (;;)
            {
               c = fgetc(ofp); 
               if (feof(ofp))
                  break;  
               if (c == '\n')
               {
                  if (++cnt == num)
                     break;
               }
            }
            line++;
            continue;
         }
         c = fgetc(ofp);
         if (feof(ofp))
            break;
         fputc(c, nfp);
         if (c == '\n')
           line++;
      }
      fclose(nfp);
      fclose(ofp);
      remove(filename);
      rename(nfilename, filename);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "post"))
   {    
   	 if(get_trust(ch) < LEVEL_STAFF) {
   	   send_to_char("You can't do that!\n\r",ch);
   	   return;
   	 }
  
      sprintf(filename, "%s", CHANGES_LIST);
      if ((ofp = fopen(filename, "r")) == NULL)
      {
         send_to_char("Cannot post, the original change file does not exist.\n\r", ch);
         return;
      }
      sprintf(nfilename, "%s", CCHANGES_LIST);
      if ((nfp = fopen(nfilename, "w")) == NULL)
      {
         send_to_char("Cannot open the new change file to post to it.\n\r", ch);
         return;
      }
      for(;;)
      {        
         if (line == 7)
         {
            c = 0;
            for (;;)
            {
               if (cbuf[c] == '\0')
                  break;
               if (cbuf[c] == '\r')
               {
                  c++;
                  continue;
               }
               fputc(cbuf[c], nfp);
               c++;
            }
            line++;
            continue;
         }
         c = fgetc(ofp);
         if (feof(ofp))
            break;
         fputc(c, nfp);
         if (c == '\n')
           line++;
      }
      fclose(nfp);
      fclose(ofp);
      remove(filename);
      rename(nfilename, filename);
      strcpy(cbuf, "");
      send_to_char("Done.\n\r", ch);
      return;
   }
      
      
   if (atoi(arg) < 1 || atoi(arg) > 100)
      lines = 30;
   else
      lines = atoi(arg);
      
   sprintf(filename, "%s", CHANGES_LIST);
   if ((fpout = fopen(filename, "r")) == NULL)
   {
      send_to_char("There is no changes file to look at.\n\r", ch);
      return;
   }
   send_to_pager_color("&w&W", ch);
   for (;;)
   {
      if (feof(fpout) || cnt == lines)
      {
         fclose(fpout);
         ch_printf(ch, "\n\r");
         return;
      }
      else
      {
         while (!feof(fpout))
         {
            if (cnt == lines)
               break;
            while ((buf[num] = fgetc(fpout)) != EOF && buf[num] != '\n' && buf[num] != '\r' && num < (MSL - 2))
               num++;
            c = fgetc(fpout);
            if ((c != '\n' && c != '\r') || c == buf[num])
               ungetc(c, fpout);
            buf[num++] = '\n';
            buf[num++] = '\r';
            buf[num] = '\0';
            send_to_pager_color(buf, ch);
            num = 0;
            cnt++;
         }
      }
   }
}

//Creates a license for a member of your kingdom.   
void do_grantlicense(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (ch->pcdata->caste != caste_Staff && ch->pcdata->caste != caste_King && ch->pcdata->caste != caste_Admin)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: grantlicense <target> <license>\n\rlicenses: merchant mayor\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);

   victim = get_char_room_new(ch, arg, 1);

   if (!victim)
   {
      send_to_char("Your recepient is not in this room.\n\r", ch);
      return;
   }
   if (ch->pcdata->hometown != victim->pcdata->hometown)
   {
      send_to_char("You cannot grant licenses to players not in your kingdom.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "merchant"))
   {
      obj = create_object(get_obj_index(OBJ_KINGDOM_MERCHANT), 0);
      obj_to_char(obj, victim);

      act(AT_WHITE, "You reward $N with a Merchant's License.", ch, NULL, victim, TO_CHAR);
      act(AT_WHITE, "$n rewards you with a Merchant's License.", ch, NULL, victim, TO_VICT);
      act(AT_WHITE, "$n rewards $N with a Merchant's License.", ch, NULL, victim, TO_ROOM);
      sprintf(logb, "%s awared %s the Merchant's License", PERS_KINGDOM(ch, ch->pcdata->hometown), PERS_KINGDOM(victim, victim->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_GRANTLICENSE);
      return;
   }
   if (!str_cmp(argument, "mayor"))
   {
      obj = create_object(get_obj_index(OBJ_KINGDOM_MAYOR), 0);
      obj_to_char(obj, victim);

      act(AT_WHITE, "You reward $N with a Mayor's License.", ch, NULL, victim, TO_CHAR);
      act(AT_WHITE, "$n rewards you with a Mayor's License.", ch, NULL, victim, TO_VICT);
      act(AT_WHITE, "$n rewards $N with a Mayor's License.", ch, NULL, victim, TO_ROOM);
      sprintf(logb, "%s awared %s the Mayor's License", PERS_KINGDOM(ch, ch->pcdata->hometown), PERS_KINGDOM(victim, victim->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_GRANTLICENSE);
      return;
   }
   do_grantlicense(ch, "");
   return;
}

//Used to find stone in non mountain sectors
void digstone(CHAR_DATA * ch, char *argument)
{
   int sector;
   OBJ_DATA *obj;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can lay roads.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to lay a road.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only dig stone if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector == SECT_FIELD || sector == SECT_HILLS || sector == SECT_PLAINS)
   {
      /* v0 is type, v1 is efficiency...Types are the same as rtype */
      for (obj = ch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->item_type == ITEM_EXTRACTOBJ)
         {
            if (obj->value[0] != 6)
            {
               continue;
            }
            else
            {
               break;
            }
         }
      } 
      if (!obj)
      {
         send_to_char("You need an object in which you can extract stone with to dig for stone.\n\r", ch);
         return;
      }  
      map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_STONE;
      resource_sector[ch->map][ch->coord->x][ch->coord->y] = number_range(1000, 3500);
      ch_printf(ch, "You dig into the earth and find a reserve of %d stone", resource_sector[ch->map][ch->coord->x][ch->coord->y]);
      return;
   }
   else
   {
      send_to_char("You can only dig for stone in fields, hills, and plains.\n\r", ch);
      return;
   }
}
   

//Used to lay roads, can only do on certain sectortypes
void layroad(CHAR_DATA * ch, char *argument)
{
   int sector;
   int type = 0;
   int gold;
   int stone;
   int x;
   int y;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can lay roads.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to lay a road.\n\r", ch);
      return;
   }
   x = ch->coord->x;
   y = ch->coord->y;
   if (argument[0] != '\0')
   {
      if (!is_valid_movement(&x, &y, argument, ch))
         return;
   }
   sector = map_sector[ch->map][x][y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only lay roads if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER || sector == SECT_OCEANFLOOR || sector == SECT_OCEAN || sector == SECT_RIVER)
      type = 1;
   if (sector != SECT_PATH && sector != SECT_PLAINS && sector != SECT_HILLS && sector != SECT_PAVE)
   {
      if (type == 0)
      {
         send_to_char("You cannot use this command on this sector, see the helpfile layroad for more info.\n\r", ch);
         return;
      }
   }
   if (type == 1)
   {
      gold = 10000;
      stone = 1000;
   }
   else
   {
      gold = 1000;
      stone = 700;
   }
   if (ch->pcdata->town->coins < gold)
   {
      if (type == 1)
         send_to_char("It costs 10k to lay a bridge down, your Kingdom does not have that.\n\r", ch);
      else
         send_to_char("It costs 1k to lay down a track of road, your Kingdom does not have that.\n\r", ch);
      return;
   }
   if (ch->pcdata->town->stone < stone)
   {
      if (type == 1)
         send_to_char("It takes 1000 units of stone to lay a bridge down.\n\r", ch);
      else
         send_to_char("It costs 700 units of stone to lay down a track of road.\n\r", ch);
      return;
   }
   ch->pcdata->town->coins -= gold;
   ch->pcdata->town->stone -= stone;
   resource_sector[ch->map][x][y] = 0;
   if (type == 1)
      map_sector[ch->map][x][y] = SECT_BRIDGE;
   else
      map_sector[ch->map][x][y] = SECT_ROAD;
   resource_sector[ch->map][x][y] = 0;
   if (type == 0)
   {
      send_to_char("A road was placed where you commanded.\n\r", ch);
      sprintf(logb, "%s placed some road", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_ROADS);
   }
   else
   {
      send_to_char("A bridge was placed where you commanded.\n\r", ch);
      sprintf(logb, "%s placed a bridge", PERS_KINGDOM(ch, ch->pcdata->hometown));
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_ROADS);
   }
   return;
}

//Used to clear the land to build a new sectortypes 
void torchland(CHAR_DATA * ch, char *argument)
{
   int sector;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can torch the lands.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only torch the lands if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector != SECT_FIELD && sector != SECT_FOREST && sector != SECT_ROAD && sector != SECT_HCORN
      && sector != SECT_HGRAIN && sector != SECT_STREE && sector != SECT_NTREE &&
      sector != SECT_SCORN && sector != SECT_NCORN && sector != SECT_SGRAIN &&
      sector != SECT_NGRAIN && sector != SECT_SWAMP && sector != SECT_JUNGLE &&
      sector != SECT_BRIDGE && sector != SECT_PATH && sector != SECT_PAVE
      && sector != SECT_STONE && sector != SECT_SSTONE && sector != SECT_NSTONE)
   {
      send_to_char("You cannot use this command on this sector, see the helpfile for more info.\n\r", ch);
      return;
   }
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_FIRE;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 500; //Takes time to burn it down
   send_to_char("Your quickly light some brush on the ground on fire and watch as it starts to burst in flames.\n\r", ch);
   sprintf(logb, "%s burned some ground", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_FIRE);
   return;
}
void cutpath(CHAR_DATA * ch, char *argument)
{
   int sector;

   if (IS_NPC(ch))
   {
      send_to_char("Only players can cut paths.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to cut a path.\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You can only cuth paths if you are in the Wilderness.\n\r", ch);
      return;
   }
   if (sector != SECT_FIELD && sector != SECT_NCORN && sector != SECT_NGRAIN && sector != SECT_NTREE
      && sector != SECT_SWAMP && sector != SECT_JUNGLE && sector != SECT_BURNT 
      && sector != SECT_PLAINS && sector != SECT_HILLS && sector != SECT_PAVE)
   {
      send_to_char("You cannot use this command on this sector, see the helpfile for more info.\n\r", ch);
      return;
   }
   if (ch->pcdata->town->stone < 400)
   {
      send_to_char("It takes 400 units of stone to lay a path, your Kingdom does not have that.\n\r", ch);
      return;
   }
   ch->pcdata->town->stone -= 400;
   map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_PATH;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
   send_to_char("You start to remove trees, branches, and debris that stand in the way.\n\r", ch);
   sprintf(logb, "%s cut a small pathway", PERS_KINGDOM(ch, ch->pcdata->hometown));
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_CUTPATH);
   return;
}

int parse_door_flag(char *argument)
{
   if (atoi(argument) <= 0 || atoi(argument) > 31)
      return -1;
   return 1 << atoi(argument);
}

char *print_door_flag(int flag)
{
   int x;
   static char flags[200];
   char sbuf[20];
   strcpy(flags, "");
   
   for (x = 0; x <= 31; x++)
   {
      if (IS_SET(flag, 1 << x))
      {
         sprintf(sbuf, " %d ", x);
         strcat(flags, sbuf);
      }
   }
   return flags;
}
//Used to view the keys in your kingdom, load one up for someone, or make a new one
void do_keys(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char buf[MSL];
   int num = 0;
   int x, y, z;
   KEY_DATA *key;
   
   if (ch->pcdata->hometown < 2 || ch->pcdata->hometown > sysdata.max_kingdom)
   {
      send_to_char("You need to belong to a kingdom to use this command.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You need to belong to a town to use this command.\n\r", ch);
      return;
   }   
   
   if (!ruling_caste(ch, ch->pcdata->hometown) && str_cmp(ch->name, ch->pcdata->town->mayor))
   {
      send_to_char("You need to belong to the ruling caste or mayor to use this command.\n\r", ch);
      return;
   }
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  keys list\n\r", ch);
      send_to_char("Syntax:  keys load <number> [perm]\n\r", ch);
      send_to_char("Syntax:  keys create <name of key>\n\r", ch);
      send_to_char("Syntax:  keys rename <number> <name of key>\n\r", ch);
      send_to_char("Syntax:  keys setdoor <direction> <flag 1> [flag 2]...\n\r", ch);
      send_to_char("Syntax:  keys showdoor <direction>\n\r", ch);
      return;
   }
   if (!ch->pcdata->town->first_key)
   {
      CREATE(key, KEY_DATA, 1);
      key->flag = BV00;
      sprintf(buf, "%s master key", ch->pcdata->town->name);
      key->name = STRALLOC(buf);
      LINK(key, ch->pcdata->town->first_key, ch->pcdata->town->last_key, next, prev);
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "create"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("You need to provide a name for the key before you can create it.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->last_key->flag == BV31)
      {
         send_to_char("You can only have 32 keys (0-31).\n\r", ch);
         return;
      }
      CREATE(key, KEY_DATA, 1);
      key->flag = ch->pcdata->town->last_key->flag*2; //moves the bitvector up one....
      sprintf(buf, "%s", argument);
      key->name = STRALLOC(buf);
      LINK(key, ch->pcdata->town->first_key, ch->pcdata->town->last_key, next, prev);
      send_to_char("A new key has been added.\n\r", ch);
      return;
   }
      
   if (!str_cmp(arg, "list"))
   {   
      send_to_char("Num     Name\n\r--------------------------------------\n\r", ch);
      for (key = ch->pcdata->town->first_key; key; key = key->next)
      {
         ch_printf(ch, "%-3d     %s\n\r", num++, key->name);
      }
      return;
   }
   argument = one_argument(argument, arg2);
   if (!str_cmp(arg, "showdoor"))
   {
      x = ch->coord->x;
      y = ch->coord->y;
      if (IN_WILDERNESS(ch) && is_valid_movement(&x, &y, arg2, ch))
      {
         for (z = 0; z <= 99; z++)
         {
            if (ch->pcdata->town->doorstate[5][z] == x && ch->pcdata->town->doorstate[6][z] == y && ch->pcdata->town->doorstate[7][z] == ch->map)
            {
               ch_printf(ch, "Flags: %s\n\r", print_door_flag(ch->pcdata->town->doorstate[2][z]));
               return;
            }
         }
         if (z == 100)
         {
            send_to_char("There is no door in that direction!\n\r", ch);
            return;
         }
         return;
      }
      else
      {
         send_to_char("You need to be in the wilderness to use this command.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg, "setdoor"))
   {
      x = ch->coord->x;
      y = ch->coord->y;
      if (IN_WILDERNESS(ch) && is_valid_movement(&x, &y, arg2, ch))
      {
         for (z = 0; z <= 99; z++)
         {
            if (ch->pcdata->town->doorstate[5][z] == x && ch->pcdata->town->doorstate[6][z] == y && ch->pcdata->town->doorstate[7][z] == ch->map)
            {
                for (;;)
                {
                   argument = one_argument(argument, arg2);
                   if (arg2[0] != '\0')
                   {
                      if (parse_door_flag(arg2) != -1)
                          SET_BIT(ch->pcdata->town->doorstate[2][z], parse_door_flag(arg2));
                   }
                   else
                      break;
               }
               send_to_char("Set.\n\r", ch);
               return;
            }
         }
         if (z == 100)
         {
            send_to_char("There is no door in that direction!\n\r", ch);
            return;
         }
      }
      else
      {
         send_to_char("You need to be in the wilderness to use this command.\n\r", ch);
         return;
      }
   }
      
   if (!str_cmp(arg, "rename"))
   {
      for (key = ch->pcdata->town->first_key; key; key = key->next)
      {
         if (num++ == atoi(arg2))
         {
            STRFREE(key->name);
            sprintf(buf, "%s", argument);
            key->name = STRALLOC(buf);
            send_to_char("The name of the key was changed, you will need to load a new one to see the results.\n\r", ch);
            return;
         }
      }
      send_to_char("That is not a valid key number.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "load"))
   {
      for (key = ch->pcdata->town->first_key; key; key = key->next)
      {
         if (num++ == atoi(arg2))
         {
            OBJ_DATA *nkey;
            nkey = create_object(get_obj_index(OBJ_KINGDOM_KEY), 1);
            sprintf(buf, "%s", uncolorify(key->name));
            STRFREE(nkey->name);
            nkey->name = STRALLOC(buf);
            STRFREE(nkey->short_descr);
            nkey->short_descr = STRALLOC(key->name);
            sprintf(buf, "Some left %s here.", key->name);
            STRFREE(nkey->description);
            nkey->description = STRALLOC(buf);
            nkey->value[0] = kingdom_table[ch->pcdata->hometown]->kpid;
            nkey->value[1] = ch->pcdata->town->tpid;
            nkey->value[2] = key->flag;
            if (!str_cmp(argument, "perm"))
               xREMOVE_BIT(nkey->extra_flags, ITEM_INVENTORY);
            obj_to_char(nkey, ch);
            send_to_char("The key has been loaded, it is in your inventory.\n\r", ch);
            return;
         }
      }   
      send_to_char("That is not a valid key number.\n\r", ch);
      return;
   }
}

//Creates a "room" outside in the wilderness, requires indoor sectortype, a door, and walls
//surrounding it.  Ex:
// IIIIIII          I - Wall  % - Inside   D - Door (sector values of this writing)
// I%%%%%I
// I%%%%%I
// III%III
//   IDI

void do_buildroom(CHAR_DATA *ch, char *argument)
{
   int x, y;
   int cx, cy;
   int z;
   int ds;
   int dv;
   int cnt;
   DOOR_DATA *ddata;
   DOOR_LIST *dlist;
   
   if (check_npc(ch))
      return;
   
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minbuild || ch->pcdata->job != 4)
   {
      send_to_char("Only those with the authorize to minbuild and are carpenters can do this!\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: roof build\n\r", ch);
      send_to_char("Syntax: roof update\n\r", ch);
      send_to_char("Syntax: roof destroy\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "update") || !str_cmp(argument, "destroy")) //Same thing pretty much, update is just destroy/build
   {
      x = ch->coord->x;
      y = ch->coord->y;
      if (map_sector[ch->map][x][y] != SECT_INSIDE)
      {
         send_to_char("You need to use this command on an indoor point inside your room.\n\r", ch);
         return;
      }
      if (ch->pcdata->hometown < 2 || ch->pcdata->hometown > sysdata.max_kingdom)
      {
         send_to_char("You need to belong to a kingdom to use this command.\n\r", ch);
         return;
      }
      if (!ch->pcdata->town)
      {
         send_to_char("You need to belong to a town to use this command.\n\r", ch);
         return;
      }   
      if (kingdom_sector[ch->map][x][y] != ch->pcdata->hometown)
      {
         send_to_char("That room does not belong to your kingdom!.\n\r", ch);
         return;
      }
      if (!in_town_range(ch->pcdata->town, x, y, ch->map))
      {
         send_to_char("That room does not belong to your town!.\n\r", ch);
         return;
      }
      if (ch->pcdata->town->usedpoint[x - ch->pcdata->town->startx+30][y - ch->pcdata->town->starty+30] == 0)
      {
         send_to_char("This room is not roofed, you cannot do this!\n\r", ch);
         return;
      }
      if (!ch->pcdata->town->first_doorlist || !ch->pcdata->town->first_doorlist->first_door)
      {
         send_to_char("There was an error, tell an immortal.\n\r", ch);
         bug("%s in town %s has a usedpoint but not a door list!", ch->name, ch->pcdata->town->name);
         return;
      }
      for (ddata = ch->pcdata->town->first_doorlist->first_door; ddata; ddata = ddata->next)
      {
         for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
         {
            if (ddata->roomcoordx[ds] == x && ddata->roomcoordy[ds] == y && ddata->roomcoordmap[ds] == ch->map)
               break;
         } 
         if (ds != MAX_HPOINTS)
            break;
      }
      if (!ddata)
      {
         send_to_char("There was an error, tell an immortal.\n\r", ch);
         bug("%s in town %s could not find the coords in ddata!", ch->name, ch->pcdata->town->name);
         return;
      }
      //remove the usedpoint data...
      for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
      {
         if (ddata->roomcoordx[ds] > 0)
         {
            ch->pcdata->town->usedpoint[ddata->roomcoordx[ds] - ch->pcdata->town->startx+30][ddata->roomcoordy[ds] - ch->pcdata->town->starty+30] = 0;
         }
      }    
      UNLINK(ddata, ch->pcdata->town->first_doorlist->first_door, ch->pcdata->town->first_doorlist->last_door, next, prev);
      DISPOSE(ddata);
      write_kingdom_file(ch->pcdata->hometown);
      if (!str_cmp(argument, "destroy"))
      {
         send_to_char("The room's roof has been removed, I hope this is what you wanted!\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Updating.....\n\r", ch);
      }
   }  
   x = ch->coord->x;
   y = ch->coord->y;
   if (map_sector[ch->map][x][y] != SECT_INSIDE)
   {
      send_to_char("You need to use this command on an indoor point inside your room.\n\r", ch);
      return;
   }
   if (ch->pcdata->hometown < 2 || ch->pcdata->hometown > sysdata.max_kingdom)
   {
      send_to_char("You need to belong to a kingdom to use this command.\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You need to belong to a town to use this command.\n\r", ch);
      return;
   }
   CREATE(ddata, DOOR_DATA, 1);
   z = 0;
   for (;;) //and it begins..
   {
      if (z != 0)
      {
         if (ddata->roomcoordx[z] == 0) //we are done, oh my!
            break;
         else
         {
            x = ddata->roomcoordx[z];
            y = ddata->roomcoordy[z];
         }
      }
      if (kingdom_sector[ch->map][x][y] != ch->pcdata->hometown)
      {
         send_to_char("Error:  A room was found that doesn't belong to your kingdom.\n\r", ch);
         DISPOSE(ddata);
         return;
      }
      if (!in_town_range(ch->pcdata->town, x, y, ch->map))
      {
         send_to_char("Error:  A room was found that doesn't belong to your town.\n\r", ch);
         DISPOSE(ddata);
         return;
      }
      //make sure for some reason we aren't outside of a town's range..
      if (abs(x - ch->pcdata->town->startx) >= 30 || abs(y - ch->pcdata->town->starty) >= 30)
      {
         send_to_char("Error:  Outside your town's range.\n\r", ch);
         DISPOSE(ddata);
         return;
      }
      if (ch->pcdata->town->usedpoint[x - ch->pcdata->town->startx+30][y - ch->pcdata->town->starty+30] == 1) //room is already used
      {
         ch_printf(ch, "Error:  This room point is already under a roof, remove it or tell an imm.\n\r");
         DISPOSE(ddata);
         return;
      }
      //any new rooms are added in sequence, so we know if roomcoordx == 0, we haven't added
      //any more new rooms, so quit!
      ddata->roomcoordx[z] = x;
      ddata->roomcoordy[z] = y;
      ddata->roomcoordmap[z++] = ch->map;
      //check all 8 directions to make sure this place is enclosed..
      for (cnt = 1; cnt <= 8; cnt++)
      {
         cx = x;
         cy = y;
         
         if (cnt == 1 || cnt == 5 || cnt == 7) //east
            cx = x+1;
         if (cnt == 2 || cnt == 7 || cnt == 8) //south
            cy = y+1;
         if (cnt == 3 || cnt == 6 || cnt == 8) //west
            cx = x-1;
         if (cnt == 4 || cnt == 5 || cnt == 6) //north
            cy = y-1;   
          
         if (map_sector[ch->map][cx][cy] == SECT_DOOR || map_sector[ch->map][cx][cy] == SECT_CDOOR
         ||  map_sector[ch->map][cx][cy] == SECT_LDOOR)
         {
            for (ds = 0; ds <= 99; ds++)
            {
               if (ch->pcdata->town->doorstate[5][ds] == cx && ch->pcdata->town->doorstate[6][ds] == cy
               &&  ch->pcdata->town->doorstate[7][ds] == ch->map)
                  break;
            }
            if (ds == 100)
            {
               send_to_char("Error:  A door was found, but it wasn't in your town, tell an imm.\n\r", ch);
               bug("do_buildroom:  %s found a door at %d %d but it wasn't in the town of %s", ch->name,
                  cx, cy, ch->pcdata->town->name);
               DISPOSE(ddata);
               return;
            }
            else
            {
               for (dv = 0; dv <= 9; dv++)
               {
                  if (ddata->doorvalue[dv] == ch->pcdata->town->doorstate[4][ds])
                     break;
               }
               if (dv == 10)
               {
                  for (dv = 0; dv <= 9; dv++)
                  {
                     if (ddata->doorvalue[dv] == 0)
                     {
                        ddata->doorvalue[dv] = ch->pcdata->town->doorstate[4][ds];
                        break;
                     }
                  }
                  if (dv == 10)
                  {
                     send_to_char("Error:  11 Doors were found in your room, 10 is the max.\n\r", ch);
                     DISPOSE(ddata);
                     return;
                  }
               }
            }
         }//end door check
         else if (map_sector[ch->map][cx][cy] == SECT_INSIDE)
         {
            if (ch->pcdata->town->usedpoint[cx - ch->pcdata->town->startx+30][cy - ch->pcdata->town->starty+30] == 1) //room is already used
            {
               ch_printf(ch, "Error:  This room point is already under a roof, remove it or tell an imm.\n\r");
               DISPOSE(ddata);
               return;
            }
            for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
            {
               if (ddata->roomcoordx[ds] == cx && ddata->roomcoordy[ds] == cy && ddata->roomcoordmap[ds] == ch->map)
                  break;
            }
            if (ds == MAX_HPOINTS)
            {
               for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
               {
                  if (ddata->roomcoordx[ds] == 0)
                  {
                     ddata->roomcoordx[ds] = cx;
                     ddata->roomcoordy[ds] = cy;
                     ddata->roomcoordmap[ds] = ch->map;
                     break;
                  }
               }
               if (ds == MAX_HPOINTS)
               {
                  ch_printf(ch, "Error:  You exceded the max points per room of %d\n\r", MAX_HPOINTS);
                  DISPOSE(ddata);
                  return;
               }
            }
         }
         else if (map_sector[ch->map][cx][cy] != SECT_DOOR && map_sector[ch->map][cx][cy] != SECT_CDOOR && 
                  map_sector[ch->map][cx][cy] != SECT_LDOOR && map_sector[ch->map][cx][cy] != SECT_WALL && 
                  map_sector[ch->map][cx][cy] != SECT_DWALL && map_sector[ch->map][cx][cy] != SECT_NBWALL && 
                  map_sector[ch->map][cx][cy] != SECT_INSIDE)
         {
            send_to_char("Error:  Found a sector that wasn't a door, wall, or inside sector.\n\r", ch);
            DISPOSE(ddata);
            return;
         }
      }//end directional for check
   }//end infinite loop
   
   //hrm don't really need this doorlist thing afterall, so if one is already created, we don't
   //need another....
   if (!ch->pcdata->town->first_doorlist)
   {
      CREATE(dlist, DOOR_LIST, 1);
      LINK(dlist, ch->pcdata->town->first_doorlist, ch->pcdata->town->last_doorlist, next, prev);
   }
   else
   {
      dlist = ch->pcdata->town->first_doorlist;
   }
   LINK(ddata, dlist->first_door, dlist->last_door, next, prev);
   send_to_char("A room was found and added to your town!\n\r", ch);
   for (x = 0; x <= MAX_HPOINTS-1; x++)
   {
      if (ddata->roomcoordx[x] > 0)
      {
         if (ch->pcdata->town->usedpoint[ddata->roomcoordx[x]-ch->pcdata->town->startx+30][ddata->roomcoordy[x]-ch->pcdata->town->starty+30] == 1)
         {
            bug("do_buildroom (roof): Coords %d %d was found in usedpoint when it should not be!", ddata->roomcoordx[x],
               ddata->roomcoordy[x]);
         }
         else
         {
            ch->pcdata->town->usedpoint[ddata->roomcoordx[x]-ch->pcdata->town->startx+30][ddata->roomcoordy[x]-ch->pcdata->town->starty+30] = 1;
         }
      }
   }
   for (ds = 0; ds <= 99; ds++)
   {
      if (ch->pcdata->town->doorstate[5][ds] > 0)
      {
         //update all the doors just in case...
         update_indoor_status(ds, ch, ch->pcdata->town, ch->pcdata->town->doorstate[5][ds], ch->pcdata->town->doorstate[6][ds], ch->map, 1);
         return;
      }
   }
   write_kingdom_file(ch->pcdata->hometown);
   return;
}
     
                    
       

//two commands for manually taking down/repairing walls.
void do_repairwall(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   int x, y;
   char buf[MSL];
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  repairwall <direction>\n\r", ch);
      return;
   }
   x = ch->coord->x;
   y = ch->coord->y;
   if (!str_cmp(argument, "n") || !str_cmp(argument, "north"))
      y = ch->coord->y-1;
   if (!str_cmp(argument, "s") || !str_cmp(argument, "south"))
      y = ch->coord->y+1;
   if (!str_cmp(argument, "e") || !str_cmp(argument, "east"))
      x = ch->coord->x+1;
   if (!str_cmp(argument, "w") || !str_cmp(argument, "west"))
      x = ch->coord->x-1;
   if (!str_cmp(argument, "nw") || !str_cmp(argument, "northwest"))
   {
      y = ch->coord->y-1;
      x = ch->coord->x-1;    
   }
   if (!str_cmp(argument, "ne") || !str_cmp(argument, "northeast"))
   {
      y = ch->coord->y-1;
      x = ch->coord->x+1;
   }
   if (!str_cmp(argument, "sw") || !str_cmp(argument, "southwest"))
   {
      y = ch->coord->y+1;
      x = ch->coord->x-1; 
   }
   if (!str_cmp(argument, "se") || !str_cmp(argument, "southeast"))
   {
      y = ch->coord->y+1;
      x = ch->coord->x+1; 
   }   
   if (x < 1 || x > MAX_X || y < 1 || y > MAX_Y)
   {
      send_to_char("That is off the map, you cannot do that.\n\r", ch);
      return;
   }
   if (ch->coord->x == x && ch->coord->y == y)
   {
      send_to_char("You have to choose a direction, ex: n nw n northwest\n\r", ch);
      return;
   }
   if (map_sector[ch->map][x][y] != SECT_WALL && map_sector[ch->map][x][y] != SECT_DWALL
   &&  map_sector[ch->map][x][y] != SECT_NBWALL && map_sector[ch->map][x][y] != SECT_DOOR
   &&  map_sector[ch->map][x][y] != SECT_CDOOR && map_sector[ch->map][x][y] != SECT_LDOOR)
   {
      send_to_char("You can only repair walls and doors with this command!\n\r", ch);
      return;
   }
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
   {
      if (IS_OBJ_STAT(obj, ITEM_REPAIRWALL))
         break;
   }
   if (!obj)
   {
      send_to_char("You do not have a wall repairing item on you, purchase one from your kingdom!\n\r", ch);
      return;
   }
   if (resource_sector[ch->map][x][y] == 10000)
   {
      send_to_char("The wall is already at full strength.\n\r", ch);
      return;
   }
   WAIT_STATE(ch, PULSE_PER_SECOND); //1 second
   sprintf(buf, "$n pulls out %s and repairs the wall to the %s", obj->short_descr, argument);
   act(AT_ACTION, buf, ch, NULL, NULL, TO_CANSEE);
   ch_printf(ch, "You pull out %s and repair the wall to the %s\n\r", obj->short_descr, argument);
   
   resource_sector[ch->map][x][y] += number_range(.8 * obj->value[0], 1.2 * obj->value[0]);
   --obj->value[1];
   if (obj->value[1] <= 0)
   {
      ch_printf(ch, "%s breaks from all the wear on it.\n\r", obj->short_descr);
      separate_obj(obj);
      obj_from_char(obj);
      extract_obj(obj);
   }
   if (map_sector[ch->map][x][y] == SECT_WALL || map_sector[ch->map][x][y] == SECT_DWALL
   ||  map_sector[ch->map][x][y] == SECT_NBWALL)
   {  
      if (resource_sector[ch->map][x][y] > 6000)
      {
         map_sector[ch->map][x][y] = SECT_WALL;
         if (resource_sector[ch->map][x][y] > 10000)
         {
            resource_sector[ch->map][x][y] = 10000;
            send_to_char("The wall is fully restored to its perfect state!\n\r", ch);
            return;
         }
      }
      else if (resource_sector[ch->map][x][y] > 2000)
         map_sector[ch->map][x][y] = SECT_DWALL;
   }
   return;
}
//If a wall breaks down, need to update the status of the rooms it is next to.
//If there is an inside sector next to it and the other sectors aren't walls
//or inside sectors from that same room, we need to start removing some things.
//Will also merge rooms if a new "path" is created to it, or remove them both
//if they are now open.  A rather loopy fun thing to write!!!!
void update_roof_stat(CHAR_DATA *ch, int x, int y)
{
   TOWN_DATA *town;
   KINGDOM_DATA *kingdom;
   int cnt, ncnt;
   int cx, cy, ds, dv;
   int nx, ny;
   int inside = 0;
   int merged = 0;
   int doorcnt = 0, roomcnt = 0;
   DOOR_DATA *ddata;
   DOOR_DATA *nddata;
   
   town = find_town(x, y, ch->map);
   if (!town)
   {
      bug("update_roof_stat:  A wall was destroyed at %d %d that did not belong to a town.\n\r", x, y);
      return;
   }
   if (kingdom_sector[ch->map][x][y] <= 1 || kingdom_sector[ch->map][x][y] > sysdata.max_kingdom)
   {
      bug("update_roof_stat:  A wall was destroyed at %d %d that did not belong to a kingdom.\n\r", x, y);
      return;
   }
   kingdom = kingdom_table[kingdom_sector[ch->map][x][y]];
   
   for (cnt = 1; cnt <= 8; cnt++)
   {
      cx = x;
      cy = y;
         
      if (cnt == 1 || cnt == 5 || cnt == 7) //east
         cx = x+1;
      if (cnt == 2 || cnt == 7 || cnt == 8) //south
         cy = y+1;
      if (cnt == 3 || cnt == 6 || cnt == 8) //west
         cx = x-1;
      if (cnt == 4 || cnt == 5 || cnt == 6) //north
         cy = y-1;  
       
      //well if we find a bad sector, might as well start removing things...  
      if (map_sector[ch->map][cx][cy] != SECT_INSIDE && map_sector[ch->map][cx][cy] != SECT_WALL
      &&  map_sector[ch->map][cx][cy] != SECT_DWALL && map_sector[ch->map][cx][cy] != SECT_NBWALL
      &&  map_sector[ch->map][cx][cy] != SECT_DOOR && map_sector[ch->map][cx][cy] != SECT_CDOOR
      &&  map_sector[ch->map][cx][cy] != SECT_LDOOR)
      {
         for (ncnt = 1; ncnt <= 8; ncnt++)
         {         
            nx = x;
            ny = y;
         
            if (ncnt == 1 || ncnt == 5 || ncnt == 7) //east
               nx = x+1;
            if (ncnt == 2 || ncnt == 7 || ncnt == 8) //south
               ny = y+1;
            if (ncnt == 3 || ncnt == 6 || ncnt == 8) //west
               nx = x-1;
            if (ncnt == 4 || ncnt == 5 || ncnt == 6) //north
               ny = y-1;
           
            if (map_sector[ch->map][nx][ny] == SECT_INSIDE)
            {
               for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
               {
                  for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
                  {
                     if (ddata->roomcoordx[ds] == nx && ddata->roomcoordy[ds] == ny && ddata->roomcoordmap[ds] == ch->map)
                        break;
                  }
                  if (ds != MAX_HPOINTS)
                     break;
               } 
               if (!ddata)  //Not really an inside room
               {
                  continue;
               } 
               //remove the usedpoint data...
               for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
               {
                  if (ddata->roomcoordx[ds] > 0)
                  {
                     town->usedpoint[ddata->roomcoordx[ds] - town->startx+30][ddata->roomcoordy[ds] - town->starty+30] = 0;
                  }
               }  
               UNLINK(ddata, town->first_doorlist->first_door, town->first_doorlist->last_door, next, prev);
               DISPOSE(ddata);
               write_kingdom_file(kingdom->num);  
            }
         }
         inside = merged = 0;
         break;
      }
      if (map_sector[ch->map][cx][cy] == SECT_INSIDE)
      {
         for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
         {
             for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
             {
                if (ddata->roomcoordx[ds] == cx && ddata->roomcoordy[ds] == cy && ddata->roomcoordmap[ds] == ch->map)
                   break;
             }
             if (ds != MAX_HPOINTS)
                break;
         } 
         if (!ddata)  //Not really an inside room..lets pave it.
         {
            bug("1    %d %d", cx, cy);
            map_sector[ch->map][cx][cy] = SECT_PAVE;
            resource_sector[ch->map][cx][cy] = 0;
            cnt--;
            continue;
         }
         inside = 1; //to properly change the sectortype, if it was inside, it will become inside...
         
         //make sure first we didn't just create a new way into a room, rofl....
         for (ncnt = 1; ncnt <= 8; ncnt++)
         {         
            nx = x;
            ny = y;
         
            if (ncnt == 1 || ncnt == 5 || ncnt == 7) //east
               nx = x+1;
            if (ncnt == 2 || ncnt == 7 || ncnt == 8) //south
               ny = y+1;
            if (ncnt == 3 || ncnt == 6 || ncnt == 8) //west
               nx = x-1;
            if (ncnt == 4 || ncnt == 5 || ncnt == 6) //north
               ny = y-1;
           
            if (map_sector[ch->map][nx][ny] == SECT_INSIDE)
            {
               for (nddata = town->first_doorlist->first_door; nddata; nddata = nddata->next)
               {
                  for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
                  {
                     if (nddata->roomcoordx[ds] == nx && nddata->roomcoordy[ds] == ny && nddata->roomcoordmap[ds] == ch->map)
                        break;
                  }
                  if (ds != MAX_HPOINTS)
                     break;
               } 
               if (!nddata)  //Not really an inside room
               {
                  continue;
               }
               if (ddata != nddata) //Time to merge...god what I do to make this not break, rofl
               {
                  //Door check time...
                  for (dv = 0; dv <= 9; dv++)
                  {
                     if (ddata->doorvalue[dv] > 0)
                        doorcnt++;
                     if (nddata->doorvalue[dv] > 0)
                        doorcnt++;
                  }
                  //need 1 room for our broken wall... -1 beomes -2
                  for (ds = 0; ds <= MAX_HPOINTS-2; ds++)
                  {
                     if (ddata->roomcoordx[ds] > 0)
                        roomcnt++;
                     if (nddata->roomcoordx[ds] > 0)
                        roomcnt++;
                  }
                  //well I would of merged them, but we have an invalid amount of doors/rooms
                  if (doorcnt > 10 || roomcnt >= MAX_HPOINTS)  
                  {
                     //remove the usedpoint data...
                     for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
                     {
                        if (ddata->roomcoordx[ds] > 0)
                        {
                           town->usedpoint[ddata->roomcoordx[ds] - town->startx+30][ddata->roomcoordy[ds] - town->starty+30] = 0;
                        }
                        if (nddata->roomcoordx[ds] > 0)
                        {
                           town->usedpoint[nddata->roomcoordx[ds] - town->startx+30][nddata->roomcoordy[ds] - town->starty+30] = 0;
                        }
                     }    
                     UNLINK(ddata, town->first_doorlist->first_door, town->first_doorlist->last_door, next, prev);
                     UNLINK(nddata, town->first_doorlist->first_door, town->first_doorlist->last_door, next, prev);
                     DISPOSE(ddata);
                     DISPOSE(nddata);
                     write_kingdom_file(kingdom->num);
                     break;
                  }
                  //merge time...
                  for (dv = 0; dv <= 9; dv++)
                  {
                     if (nddata->doorvalue[dv] > 0)
                     {
                        for (ds = 0; ds <= 9; ds++)
                        {
                           if (ddata->doorvalue[ds] == 0)
                           {
                              ddata->doorvalue[ds]= nddata->doorvalue[dv];
                              break;
                           }
                        }
                     }
                  }
                  for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
                  {
                     if (nddata->roomcoordx[ds] > 0)
                     {
                        for (dv = 0; dv <= MAX_HPOINTS-1; dv++)
                        {
                           if (ddata->roomcoordx[dv] == 0)
                           {
                              ddata->roomcoordx[dv] = nddata->roomcoordx[ds];
                              ddata->roomcoordy[dv] = nddata->roomcoordy[ds];
                              ddata->roomcoordmap[dv] = nddata->roomcoordmap[ds];
                              break;
                           }
                        }
                     }
                  }      
                  if (merged == 0)
                  {
                     for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
                     {
                        if (ddata->roomcoordx[ds] == 0)
                        {
                           ddata->roomcoordx[ds] = x;
                           ddata->roomcoordy[ds] = y;
                           ddata->roomcoordmap[ds] = ch->map;
                           break;
                        }
                     }
                     town->usedpoint[x - town->startx+30][y - town->starty+30] = 1;
                     map_sector[ch->map][x][y] = SECT_INSIDE;
                     resource_sector[ch->map][x][y] = 0;
                  }
                  merged = 1;
                  UNLINK(nddata, town->first_doorlist->first_door, town->first_doorlist->last_door, next, prev);
                  DISPOSE(nddata);
                  write_kingdom_file(kingdom->num);     
               }//End ddata != nddata
            }//End Inside Merge Ifcheck                      
        }//End Merge Check
     }//End Inside Ifcheck
   }//End 8 Directional For statement
   if (inside == 0) //Outside, pave it..
   {
      map_sector[ch->map][x][y] = SECT_PAVE;
      resource_sector[ch->map][x][y] = 0;
      return;
   }
   else if (merged == 0)//Need to add it somewhere....
   {
      for (cnt = 1; cnt <= 8; cnt++)
      {
         cx = x;
         cy = y;
         
         if (cnt == 1 || cnt == 5 || cnt == 7) //east
            cx = x+1;
         if (cnt == 2 || cnt == 7 || cnt == 8) //south
            cy = y+1;
         if (cnt == 3 || cnt == 6 || cnt == 8) //west
            cx = x-1;
         if (cnt == 4 || cnt == 5 || cnt == 6) //north
            cy = y-1; 
            
         if (map_sector[ch->map][cx][cy] == SECT_INSIDE)
         {
            for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
            {
               for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
               {
                  if (ddata->roomcoordx[ds] == cx && ddata->roomcoordy[ds] == cy && ddata->roomcoordmap[ds] == ch->map)
                      break;
                }
                if (ds != MAX_HPOINTS)
                   break;
            } 
            if (!ddata)  //Not really an inside room..lets pave it.
            {
               continue;
            }
            for (ds = 0; ds <= MAX_HPOINTS-1; ds++)
            {
               if (ddata->roomcoordx[ds] == 0)
               {
                  ddata->roomcoordx[ds] = x;
                  ddata->roomcoordy[ds] = y;
                  ddata->roomcoordmap[ds] = ch->map;
                  break;
               }
            }
            town->usedpoint[x - town->startx+30][y - town->starty+30] = 1;
            map_sector[ch->map][x][y] = SECT_INSIDE;
            resource_sector[ch->map][x][y] = 0;
            write_kingdom_file(kingdom->num);   
            return;  
         }
      }
   }
   else //already added via merge
   {
      return;
   }
   bug("update_roof_stat:  After processing, at %d %d could not find an indoor room!!??", x, y);
   map_sector[ch->map][x][y] = SECT_PAVE;
   resource_sector[ch->map][x][y] = 0;
   return;
}

void update_door_stat(CHAR_DATA *ch, int x, int y)
{
   int z;
   int dx;
   TOWN_DATA *town;
   DOOR_DATA *ddata;
   
   town = find_town(x, y, ch->map);
   if (!town)
   {
      bug("update_door_stat:  A wall was destroyed at %d %d that did not belong to a town.\n\r", x, y);
      return;
   }
   
   for (z = 0; z <= 99; z++)
   {
      if (town->doorstate[5][z] == x && town->doorstate[6][z] == y && town->doorstate[7][z] == ch->map)
      {
         for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
         {
            for (dx = 0; dx <= 9; dx++)
            {
               if (ddata->doorvalue[dx] == town->doorstate[3][z])
                  ddata->doorvalue[dx] = 0;
            }
         }
         town->doorstate[0][z] = town->doorstate[1][z] = town->doorstate[2][z] = town->doorstate[3][z] = 0;
         town->doorstate[4][z] = town->doorstate[5][z] = town->doorstate[6][z] = town->doorstate[7][z] = 0;
         return;
      }
   }
}       

//checks the doors to make sure a new master door or two wasn't just created....
//Only need coords in a town to perform check...don't need an actual door coords
void update_master_stat(CHAR_DATA *ch, int x, int y)
{
   TOWN_DATA *town;
   int ds;
   int cx, cy, cnt, dx, dy, dcnt, z;
   
   town = find_town(x, y, ch->map);
   if (!town)
   {
      bug("update_door_stat:  A wall was destroyed at %d %d that did not belong to a town.\n\r", x, y);
      return;
   }
   
   for (ds = 0; ds <= 99; ds++)
   {
      if (town->doorstate[5][ds] > 0)
      {
         cx = town->doorstate[5][ds];
         cy = town->doorstate[6][ds];  
         for (cnt = 1; cnt <= 8; cnt++)
         {
            if (cnt == 1 || cnt == 5 || cnt == 7) //east
               cx = cx+1;
            if (cnt == 2 || cnt == 7 || cnt == 8) //south
               cy = cy+1;
            if (cnt == 3 || cnt == 6 || cnt == 8) //west
               cx = cx-1;
            if (cnt == 4 || cnt == 5 || cnt == 6) //north
               cy = cy-1;  
                        
            if (map_sector[ch->map][cx][cy] == SECT_DOOR || map_sector[ch->map][cx][cy] == SECT_CDOOR
            ||  map_sector[ch->map][cx][cy] == SECT_LDOOR)
            {
               for (dcnt = 1; dcnt <= 8; dcnt++)
               {
                  dx = cx;
                  dy = cy;
         
                  if (dcnt == 1 || dcnt == 5 || dcnt == 7) //east
                     dx = cx+1;
                  if (dcnt == 2 || dcnt == 7 || dcnt == 8) //south
                     dy = cy+1;
                  if (dcnt == 3 || dcnt == 6 || dcnt == 8) //west
                     dx = cx-1;
                  if (dcnt == 4 || dcnt == 5 || dcnt == 6) //north
                     dy = cy-1;  
                              
                  if (map_sector[ch->map][dx][dy] != SECT_INSIDE && map_sector[ch->map][dx][dy] != SECT_WALL
                  &&  map_sector[ch->map][dx][dy] != SECT_DWALL && map_sector[ch->map][dx][dy] != SECT_NBWALL
                  &&  map_sector[ch->map][dx][dy] != SECT_DOOR && map_sector[ch->map][dx][dy] != SECT_CDOOR
                  &&  map_sector[ch->map][dx][dy] != SECT_LDOOR)
                  {
                     for (z = 0; z <= 99; z++)
                     {
                        if (town->doorstate[5][z] == cx && town->doorstate[6][z] == cy 
                        &&  town->doorstate[7][z] == ch->map)
                        {
                           ch->pcdata->town->doorstate[3][z] = 1; //Master door, goes outside..
                           break;
                        }
                     }
                     if (z == 100)
                     {
                        bug("A door was found at %d %d, but no record of it is in the kingdom.", cx, cy);
                        break;
                     }
                     else
                     {
                        break;
                     }
                  }
               }
               if (dcnt == 9)
               {
                  for (z = 0; z <= 99; z++)
                  {
                     if (town->doorstate[5][z] == cx && town->doorstate[6][z] == cy 
                     &&  town->doorstate[7][z] == ch->map)
                     {
                        ch->pcdata->town->doorstate[3][z] = 0; //Not a master door
                        break;
                     }
                  }
                  if (z == 100)
                  {
                     bug("A door was found at %d %d, but no record of it is in the kingdom.", cx, cy);
                     break;
                  }
               }
            }//end door ifcheck     
         }//end cnt for statement
      }//end town->doorstate[5][ds] ifcheck
   }//end ds loop
   for (ds = 0; ds <= 99; ds++)
   {
      if (town->doorstate[5][ds] > 0)
      {
         //update all the doors just in case...
         update_indoor_status(ds, ch, town, town->doorstate[5][ds], town->doorstate[6][ds], ch->map, 1);
         return;
      }
   }
}
   
void do_breakwall(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   int x, y;
   int dam;
   char buf[MSL];
   char sbuf[10];
   TOWN_DATA *town;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  breakwall <direction>\n\r", ch);
      return;
   }
   if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
   {
      send_to_char("You need to be wielding something to damage a wall.\n\r", ch);
      return;
   }
   x = ch->coord->x;
   y = ch->coord->y;
   if (!str_cmp(argument, "n") || !str_cmp(argument, "north"))
      y = ch->coord->y-1;
   if (!str_cmp(argument, "s") || !str_cmp(argument, "south"))
      y = ch->coord->y+1;
   if (!str_cmp(argument, "e") || !str_cmp(argument, "east"))
      x = ch->coord->x+1;
   if (!str_cmp(argument, "w") || !str_cmp(argument, "west"))
      x = ch->coord->x-1;
   if (!str_cmp(argument, "nw") || !str_cmp(argument, "northwest"))
   {
      y = ch->coord->y-1;
      x = ch->coord->x-1;    
   }
   if (!str_cmp(argument, "ne") || !str_cmp(argument, "northeast"))
   {
      y = ch->coord->y-1;
      x = ch->coord->x+1;
   }
   if (!str_cmp(argument, "sw") || !str_cmp(argument, "southwest"))
   {
      y = ch->coord->y+1;
      x = ch->coord->x-1; 
   }
   if (!str_cmp(argument, "se") || !str_cmp(argument, "southeast"))
   {
      y = ch->coord->y+1;
      x = ch->coord->x+1; 
   }   
   if (x < 1 || x > MAX_X || y < 1 || y > MAX_Y)
   {
      send_to_char("That is off the map, you cannot do that.\n\r", ch);
      return;
   }
   if (ch->coord->x == x && ch->coord->y == y)
   {
      send_to_char("You have to choose a direction, ex: n nw n northwest\n\r", ch);
      return;
   }
   if (map_sector[ch->map][x][y] != SECT_WALL && map_sector[ch->map][x][y] != SECT_DWALL
   &&  map_sector[ch->map][x][y] != SECT_NBWALL && map_sector[ch->map][x][y] != SECT_DOOR
   &&  map_sector[ch->map][x][y] != SECT_CDOOR && map_sector[ch->map][x][y] != SECT_LDOOR)
   {
      send_to_char("You can only damage walls with this command!\n\r", ch);
      return;
   }
   if (map_sector[ch->map][x][y] == SECT_WALL || map_sector[ch->map][x][y] == SECT_DWALL
   ||  map_sector[ch->map][x][y] == SECT_NBWALL)
      sprintf(sbuf, "wall");
   
   if (map_sector[ch->map][x][y] == SECT_DOOR ||  map_sector[ch->map][x][y] == SECT_CDOOR 
   || map_sector[ch->map][x][y] == SECT_LDOOR)
      sprintf(sbuf, "door");
      
   sprintf(buf, "$n swings wildly at the %s to the %s trying to damage it!", sbuf, argument);
   act(AT_ACTION, buf, ch, NULL, NULL, TO_CANSEE);
   ch_printf(ch, "You swing wildly at the %s to the %s trying to damage it!\n\r", sbuf, argument);
   dam = number_range(obj->value[1], obj->value[2]) + str_app[get_curr_str(ch)].todam;
   damage_obj(obj, ch, 0, dam*2.5);
   dam = UMAX(1, number_range(dam*.5, dam*.8));
   WAIT_STATE(ch, PULSE_PER_SECOND); //1 second
   if (resource_sector[ch->map][x][y] - dam <= 0)
   {
      sprintf(buf, "$n's strike causes the %s to crumble into the ground!", sbuf);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CANSEE);
      ch_printf(ch, "Your strike causes the %s to crumble into the ground!\n\r", sbuf);
      //If a wall crumbles, it can introduce light into a room....Need to remove the
      //roof if it does.
      town = find_town(x, y, ch->map);
      if (!town || (kingdom_sector[ch->map][x][y] <= 1 || kingdom_sector[ch->map][x][y] > sysdata.max_kingdom))
      {
         map_sector[ch->map][x][y] = SECT_PAVE;
         resource_sector[ch->map][x][y] = 0;
      }
      else
      {
         if (!str_cmp(sbuf, "door"))
            update_door_stat(ch, x, y);
         update_roof_stat(ch, x, y);
         update_master_stat(ch, x, y);
      }
      return;
   }
   resource_sector[ch->map][x][y] -= dam;
   
   if (map_sector[ch->map][x][y] == SECT_WALL || map_sector[ch->map][x][y] == SECT_DWALL
   ||  map_sector[ch->map][x][y] == SECT_NBWALL)
   {
      if (resource_sector[ch->map][x][y] <= 2000)
         map_sector[ch->map][x][y] = SECT_NBWALL;
      else if (resource_sector[ch->map][x][y] <= 6000)
         map_sector[ch->map][x][y] = SECT_DWALL;
   }
   return;
}
   


/* Middle caste command - Allows a Baronet or higher assign a worker to
   a job */
void do_makeworker(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   char jobhold[MSL];
   int num;
   CHAR_DATA *victim;

   argument = one_argument(argument, arg1);

   if (IS_NPC(ch))
   {
      send_to_char("Only players can appoint workers\n\r", ch);
      return;
   }
   if (ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minappoint)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: makeworker <character> <type>\n\rtype: 1 - Gold   2 - Iron  3 - Corn  4 - Grain  5 - Lumber  6 - Stone  7 - Fish\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: makeworker <character> <type>\n\rtype: 1 - Gold   2 - Iron  3 - Corn  4 - Grain  5 - Lumber  6 - Stone  7 - Fish\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Your target was not found.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Not on NPCs.\n\r", ch);
      return;
   }
   if (victim->pcdata->job != 1)
   {
      send_to_char("Your target needs to be an extractor.\n\r", ch);
      return;
   }
   if (victim->pcdata->hometown != ch->pcdata->hometown)
   {
      send_to_char("Your targets hometown is not that of yours.\n\r", ch);
      return;
   }
   num = parse_resource_arg(argument);
   if (num < 1)
   {
      send_to_char("Not a valid Resource.\n\r", ch);
      return;
   }
   
   if (num == KRES_GOLD)
      sprintf(jobhold, "gold");
   if (num == KRES_IRON)
      sprintf(jobhold, "iron");
   if (num == KRES_CORN)
      sprintf(jobhold, "corn");
   if (num == KRES_GRAIN)
      sprintf(jobhold, "grain");
   if (num == KRES_LUMBER)
      sprintf(jobhold, "lumber");
   if (num == KRES_STONE)
      sprintf(jobhold, "stone");
   if (num == KRES_FISH)
      sprintf(jobhold, "fish");

   sprintf(buf, "You are now able to extract %s from the lands.\n\r", jobhold);

   victim->pcdata->resourcetype = num;
   victim->pcdata->resource = 0;
   victim->pcdata->job = 1;
   send_to_char("Done.\n\r", ch);
   sprintf(logb, "%s changed %s extraction duties to %s", PERS_KINGDOM(ch, ch->pcdata->hometown), PERS_KINGDOM(victim, victim->pcdata->hometown), jobhold);
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_MAKEWORKER);
   send_to_char(buf, victim);

}

void add_resources(CHAR_DATA *ch, int resourcetype, int hometown, TOWN_DATA *town, TOWN_DATA *dtown, int amount)
{
   char buf[20];
   int amount2k = 0;
   int dfull = 0;
   
   if (resourcetype == KRES_GOLD)
   {
      sprintf(buf, "gold");
      amount2k = amount * kingdom_table[hometown]->gold_tax / 1000;
      amount = amount - amount2k;         
      if ((get_current_hold(town) + amount) <= town->hold)
         town->gold += amount;
      else
      {
         amount = town->hold - get_current_hold(town);
         town->gold += amount;
         dfull = 1;
      }   
      if ((get_current_hold(dtown) + amount2k) <= dtown->hold)
         dtown->gold += amount2k;
      else
      {
         amount2k = dtown->hold - get_current_hold(dtown);
         dtown->gold += amount2k;
         if (dfull == 1)
            dfull = 3;
         else             
            dfull = 2;       
      }                 
   }
   if (resourcetype == KRES_IRON)
   {
      sprintf(buf, "iron");
      amount2k = amount * kingdom_table[hometown]->iron_tax / 1000;
      amount = amount - amount2k;         
      if ((get_current_hold(town) + amount) <= town->hold)
         town->iron += amount;
      else
      {
         amount = town->hold - get_current_hold(town);
         town->iron += amount;
         dfull = 1;
      }   
      if ((get_current_hold(dtown) + amount2k) <= dtown->hold)
         dtown->iron += amount2k;
      else
      {
         amount2k = dtown->hold - get_current_hold(dtown);
         dtown->iron += amount2k;
         if (dfull == 1)
            dfull = 3;
         else             
            dfull = 2;       
      }  
   }
   if (resourcetype == KRES_CORN)
   {
      sprintf(buf, "corn");
      amount2k = amount * kingdom_table[hometown]->corn_tax / 1000;
      amount = amount - amount2k;         
      if ((get_current_hold(town) + amount) <= town->hold)
         town->corn += amount;
      else
      {
         amount = town->hold - get_current_hold(town);
         town->corn += amount;
         dfull = 1;
      }   
      if ((get_current_hold(dtown) + amount2k) <= dtown->hold)
         dtown->corn += amount2k;
      else
      {
         amount2k = dtown->hold - get_current_hold(dtown);
         dtown->corn += amount2k;
         if (dfull == 1)
            dfull = 3;
         else             
            dfull = 2;       
      }   
   }
   if (resourcetype == KRES_GRAIN)
   {
      sprintf(buf, "grain");
      amount2k = amount * kingdom_table[hometown]->grain_tax / 1000;
      amount = amount - amount2k;         
      if ((get_current_hold(town) + amount) <= town->hold)
         town->grain += amount;
      else
      {
         amount = town->hold - get_current_hold(town);
         town->grain += amount;
         dfull = 1;
      }   
      if ((get_current_hold(dtown) + amount2k) <= dtown->hold)
         dtown->grain += amount2k;
      else
      {
         amount2k = dtown->hold - get_current_hold(dtown);
         dtown->grain += amount2k;
         if (dfull == 1)
            dfull = 3;
         else             
            dfull = 2;       
      }  
   }
   if (resourcetype == KRES_LUMBER)
   {
      sprintf(buf, "lumber");
      amount2k = amount * kingdom_table[hometown]->tree_tax / 1000;
      amount = amount - amount2k;         
      if ((get_current_hold(town) + amount) <= town->hold)
         town->lumber += amount;
      else
      {
         amount = town->hold - get_current_hold(town);
         town->lumber += amount;
         dfull = 1;
      }   
      if ((get_current_hold(dtown) + amount2k) <= dtown->hold)
         dtown->lumber += amount2k;
      else
      {
         amount2k = dtown->hold - get_current_hold(dtown);
         dtown->lumber += amount2k;
         if (dfull == 1)
            dfull = 3;
         else             
            dfull = 2;       
      }  
   }
   if (resourcetype == KRES_STONE)
   {
      sprintf(buf, "stone");
      amount2k = amount * kingdom_table[hometown]->stone_tax / 1000;
      amount = amount - amount2k;         
      if ((get_current_hold(town) + amount) <= town->hold)
         town->stone += amount;
      else
      {
         amount = town->hold - get_current_hold(town);
         town->stone += amount;
         dfull = 1;
      }   
      if ((get_current_hold(dtown) + amount2k) <= dtown->hold)
         dtown->stone += amount2k;
      else
      {
         amount2k = dtown->hold - get_current_hold(dtown);
         dtown->stone += amount2k;
         if (dfull == 1)
            dfull = 3;
         else             
            dfull = 2;       
      }  
   }
   if (resourcetype == KRES_FISH)
   {
      sprintf(buf, "fish");
      amount2k = amount * kingdom_table[hometown]->fish_tax / 1000;
      amount = amount - amount2k;         
      if ((get_current_hold(town) + amount) <= town->hold)
         town->fish += amount;
      else
      {
         amount = town->hold - get_current_hold(town);
         town->fish += amount;
         dfull = 1;
      }   
      if ((get_current_hold(dtown) + amount2k) <= dtown->hold)
         dtown->fish += amount2k;
      else
      {
         amount2k = dtown->hold - get_current_hold(dtown);
         dtown->fish += amount2k;
         if (dfull == 1)
            dfull = 3;
         else             
            dfull = 2;       
      }  
   }
   ch_printf(ch, "Your city now has %d more units of %s\n\r", amount, buf);
   ch_printf(ch, "You paid your tax of %d more units of %s\n\r", amount2k, buf);
   if (amount2k+amount > 0)
   {
       if (dfull == 1)
       {
          sprintf(logb, "*FULL* %s added %d units of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), amount, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
          sprintf(logb, "%s gave %d units of %s for tax", PERS_KINGDOM(ch, ch->pcdata->hometown), amount2k, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
       }
       else if (dfull == 2)
       {
          sprintf(logb, "%s added %d units of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), amount, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
          sprintf(logb, "*FULL* %s gave %d units of %s for tax", PERS_KINGDOM(ch, ch->pcdata->hometown), amount2k, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
       } 
       else if (dfull == 3)
       {
          sprintf(logb, "*FULL* %s added %d units of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), amount, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
          sprintf(logb, "*FULL* %s gave %d units of %s for tax", PERS_KINGDOM(ch, ch->pcdata->hometown), amount2k, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
       }  
       else
       {
          sprintf(logb, "%s added %d units of %s", PERS_KINGDOM(ch, ch->pcdata->hometown), amount, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
          sprintf(logb, "%s gave %d units of %s for tax", PERS_KINGDOM(ch, ch->pcdata->hometown), amount2k, buf);
          write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_RESOURCES);
       }
   }
}
/* Allows you to dump the goods into a holding bin -- Xerves 12/99 */
void do_dumpgoods(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   char buf[MSL];
   char arg1[MIL];
   int rtype = 0;
   TOWN_DATA *town;
   sh_int amount = 0;

   argument = one_argument(argument, arg1);

   if (IS_NPC(ch))
   {
      send_to_char("Only players can dump resoures in a container.\n\r", ch);
      return;
   }
   if (ch->pcdata->resourcetype == 0)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (ch->pcdata->job != 1)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to dumpgoods into it.\n\r", ch);
      return;
   }
   if (ch->pcdata->resourcetype > MAX_RTYPE)
   {
      sprintf(buf, "%s has a resourcetype over the max", ch->name);
      bug(buf, 0);
      return;
   }
   rtype = ch->pcdata->resourcetype;
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: dump <amount/all> [obj]\n\r", ch);
      send_to_char("Syntax: dump town <amount/all/obj>\n\r", ch);
      send_to_char("Syntax: dump waste\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "waste"))
   {
      ch->pcdata->resource = 0;
      send_to_char("Removed all resources on you.\n\r", ch);
      return;
   }
   if (!is_number(arg1) && str_cmp(arg1, "all") && str_cmp(arg1, "town"))
   {
      send_to_char("Need an amount or all.\n\r", ch);
      return;
   }
   if ((obj = get_obj_here(ch, argument)) != NULL)
   {
      if (obj->item_type != ITEM_HOLDRESOURCE)
      {
         send_to_char("That is not a proper bin to dump resources in.\n\r", ch);
         return;
      }
      if (!str_cmp(arg1, "town"))
      {
         if (!IN_WILDERNESS(ch))
         {
            send_to_char("You can only do this in a kingdom.\n\r", ch);
            return;
         }
         if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)
         {
            send_to_char("Your kingdom does now own this town.\n\r", ch);
            return;
         }
         if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
         {
            send_to_char("You can only dump resources into YOUR town.\n\r", ch);
            return;
         }
         town = get_town(kingdom_table[ch->pcdata->hometown]->dtown);
         if (!town)
         {
            send_to_char("Could not find yoru default town, tell the kingdom.\n\r", ch);
            return;
         }
         if ((get_current_hold(ch->pcdata->town) + obj->value[2] +  obj->value[4]) > ch->pcdata->town->hold)
         {
            send_to_char("Your town doesn't have enough capacity to hold these resources.\n\r", ch);
            return;
         }       
         add_resources(ch, obj->value[1], ch->pcdata->hometown, ch->pcdata->town, town, obj->value[2]);
         add_resources(ch, obj->value[3], ch->pcdata->hometown, ch->pcdata->town, town, obj->value[4]);
         obj->value[2] = 0;
         obj->value[4] = 0;
         return;
      }
      if (!str_cmp(arg1, "all"))
         amount = ch->pcdata->resource;
      else
         amount = atoi(arg1);
   /* v0 - Max in each holder  v1 - First type  v2 - First quantity
      v3 - Second type   v4 - Second quantity  -- Xerves - 12/99 */
      rtype = ch->pcdata->resourcetype;

      if (ch->pcdata->resource - amount < 0)
      {
         send_to_char("You don't have that much resources on you.\n\r", ch);
         return;
      }
      if (rtype != obj->value[1] && rtype != obj->value[3])
      {
         send_to_char("This bin cannot hold that type of resource\n\r", ch);
         return;
      }
      if (rtype == obj->value[1] && obj->value[2] + amount > obj->value[0])
      {
         send_to_char("If you dumped your resources here, you would overflow the bin.\n\r", ch);
         return;
      }
      if (rtype == obj->value[3] && obj->value[4] + amount > obj->value[0])
      {
         send_to_char("If you dumped your resources here, you would overflow the bin.\n\r", ch);
         return;
      }
      if (rtype == obj->value[1])
      {
         obj->value[2] = obj->value[2] + amount;
         ch->pcdata->resource -= amount;
         send_to_char("Ok.\n\r", ch);
      }
      if (rtype == obj->value[3])
      {
         obj->value[4] = obj->value[4] + amount;
         ch->pcdata->resource -= amount;
         send_to_char("Ok.\n\r", ch);
      }
      save_bin_data();
   }
   else
   {
      if (str_cmp(arg1, "town"))
      {
         do_dumpgoods(ch, "");
         return;
      }
      if (!IN_WILDERNESS(ch))
      {
         send_to_char("You can only do this in a kingdom.\n\r", ch);
         return;
      }
      if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)
      {
         send_to_char("Your kingdom does now own this town.\n\r", ch);
         return;
      }
      if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
      {
         send_to_char("You can only dump resources into YOUR town.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "all"))
         amount = ch->pcdata->resource;
      else
         amount = atoi(argument);
      rtype = ch->pcdata->resourcetype;

      if (ch->pcdata->resource - amount < 0)
      {
         send_to_char("You don't have that much resources on you.\n\r", ch);
         return;
      }
      town = get_town(kingdom_table[ch->pcdata->hometown]->dtown);
      if (!town)
      {
         send_to_char("Could not find your default town, tell the kingdom.\n\r", ch);
         return;
      }
      if ((get_current_hold(ch->pcdata->town) + amount) > ch->pcdata->town->hold)
      {
         send_to_char("Your town doesn't have enough capacity to hold these resources.\n\r", ch);
         return;
      }
      add_resources(ch, ch->pcdata->resourcetype, ch->pcdata->hometown, ch->pcdata->town, town, amount);
      ch->pcdata->resource = 0;
      return;
   }
}

/* Used to take resources from bins and put them on your character or into another bin in the room -- Xerves Jan 2002 */
void do_getresources(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   OBJ_DATA *objto = NULL;
   int type;
   int amount;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char logb[MIL];

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (ch->pcdata->job != 1)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("getresources <bin> <type> <amount> [target bin]\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   
   obj = get_obj_here(ch, arg1);
   
   if (!obj)
   {
      send_to_char("That bin is not here.\n\r", ch);
      return;
   }
   if (obj->item_type != ITEM_HOLDRESOURCE)
   {
      send_to_char("That is not a storage bin.\n\r", ch);
      return;
   }
   type = atoi(arg2);
   
   if (type < 1 || type > 6)
   {
      send_to_char("That is an invalid type.  Your select is.\n\r1 - Gold    2 - Iron   3 - Corn    4 - Grain    5 - Lumber    6 - Stone\n\r", ch);
      return;
   }
   amount = atoi(arg3);
   if (amount < 1 && str_cmp(arg3, "all"))
   { 
      send_to_char("Amount has to be greater than 0.\n\r", ch);
      return;
   }
   if (argument[0] != '\0')
   {
      objto = get_obj_here(ch, argument);
   
      if (!objto)
      {
         send_to_char("That bin is not here.\n\r", ch);
         return;
      }
      if (objto->item_type != ITEM_HOLDRESOURCE)
      {
         send_to_char("That is not a storage bin.\n\r", ch);
         return;
      }
      if (objto == obj)
      {
         send_to_char("You cannot transfer resources to the same bin.\n\r", ch);
         return;
      }
   }    
   if (!objto)
   {
      int mod = 1;  
      if (obj->value[1] != type && obj->value[3] != type)
      {
         ch_printf(ch, "%s does not hold that resource type.\n\r", obj->short_descr);
         return;
      }
      if (ch->pcdata->resourcetype != type)
      {
         send_to_char("You are not extracting that resource, so you cannot take that resource.\n\r", ch);
         return;
      }
      if (type == 1 || type == 2)
         mod = 3;
         
      if (str_cmp(arg3, "all") && (ch->pcdata->resource + amount > (get_curr_str(ch) * 6 / mod)))
      {
         ch_printf(ch, "You can hold only %d resources, not %d\n\r", get_curr_str(ch) * 6 / mod, ch->pcdata->resource+amount);
         return;
      }
      if (!str_cmp(arg3, "all"))
      {
         amount = (get_curr_str(ch) * 6 / mod) - ch->pcdata->resource;
      }
      if (obj->value[1] == type)
      {
         if (amount > obj->value[2])
         {
            if (!str_cmp(arg3, "all"))
            {
               amount = obj->value[2];
            }
            else
            {
               ch_printf(ch, "There is not enough of that resource left in %s\n\r", obj->short_descr);
               return;
            }
         }
         obj->value[2] -= amount;
      }
      if (obj->value[3] == type)
      {
         if (amount > obj->value[4])
         {
            if (!str_cmp(arg3, "all"))
            {
               amount = obj->value[4];
            }
            else
            {
               ch_printf(ch, "There is not enough of that resource left in %s\n\r", obj->short_descr);
               return;
            }
         }
         obj->value[4] -= amount;
      }
      ch->pcdata->resource += amount;
      ch_printf(ch, "You take %d resources from %s\n\r", amount, obj->short_descr);
      sprintf(logb, "%s takes %d resources from %s", PERS_KINGDOM(ch, ch->pcdata->hometown), amount, obj->short_descr);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_GETRESOURCES);
      save_bin_data();
      return;
   }
   else
   {
      int tvalue;
      
      if (obj->value[1] != type && obj->value[3] != type)
      {
         ch_printf(ch, "%s does not hold that resource type.\n\r", obj->short_descr);
         return;
      }
      if (objto->value[1] != type && objto->value[3] != type)
      {
         ch_printf(ch, "%s does not hold that resource type.\n\r", objto->short_descr);
         return;
      }
      if (objto->value[1] == type)
         tvalue = 1;
      else
         tvalue = 3;
      if (obj->value[1] == type)
      {
         if (str_cmp(arg3, "all") && (obj->value[2] - amount < 0))
         {
            ch_printf(ch, "%s only has %d left of that resource.\n\r", obj->short_descr, obj->value[2]);
            return;
         }
         if (str_cmp(arg3, "all") && (objto->value[tvalue+1] + amount > objto->value[0]))
         {
            ch_printf(ch, "%s only can hold %d of that resource.\n\r", objto->short_descr, obj->value[0]);
            return;
         }
         if (!str_cmp(arg3, "all"))
         {
            amount = obj->value[2];
            if (objto->value[0] - objto->value[tvalue+1] < amount)
               amount = objto->value[0] - objto->value[tvalue+1];
         }
         obj->value[2] -= amount;
         objto->value[tvalue+1] += amount;
      }
      if (obj->value[3] == type)
      {
         if (str_cmp(arg3, "all") && (obj->value[4] - amount < 0))
         {
            ch_printf(ch, "%s only has %d left of that resource.\n\r", obj->short_descr, obj->value[4]);
            return;
         }
         if (str_cmp(arg3, "all") && (objto->value[tvalue+1] + amount > objto->value[0]))
         {
            ch_printf(ch, "%s only can hold %d of that resource.\n\r", objto->short_descr, obj->value[0]);
            return;
         }
         if (!str_cmp(arg3, "all"))
         {
            amount = obj->value[4];
            if (objto->value[0] - objto->value[tvalue+1] < amount)
               amount = objto->value[0] - objto->value[tvalue+1];
         }
         obj->value[4] -= amount;
         objto->value[tvalue+1] += amount;
      }      
      ch_printf(ch, "You take %d resources from %s and put it in %s.\n\r", amount, obj->short_descr, objto->short_descr);
      sprintf(logb, "%s takes %d resources from %s and puts it in %s", PERS_KINGDOM(ch, ch->pcdata->hometown), amount, obj->short_descr, objto->short_descr);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_GETRESOURCES);
      save_bin_data();
      return;
   }   
   do_getresources(ch, "");
   return;
}  

//NO SPACES, will screw up the parsing in the Names
char *return_logname(int flag)
{
   static char logname[50];
   
   switch (flag)
   {
      case KLOG_SETJOB:            sprintf(logname, "SetJob");             break;
      case KLOG_TRADEGOODS:        sprintf(logname, "Tradegoods");         break;
      case KLOG_RESOURCES:         sprintf(logname, "Resources");          break;
      case KLOG_PLACEOBJ:          sprintf(logname, "Placeobj");           break;
      case KLOG_KREMOVE_TRAINER:   sprintf(logname, "KremoveTrainer");     break;
      case KLOG_KREMOVE_OBJ:       sprintf(logname, "KremoveObj");         break;
      case KLOG_PLACETRAINER:      sprintf(logname, "PlaceTrainer");       break;
      case KLOG_PLACEMOB:          sprintf(logname, "Placemob");           break;
      case KLOG_PLANTGRAIN:        sprintf(logname, "Plantgrain");         break;      
      case KLOG_PLANTCORN:         sprintf(logname, "Plantcorn");          break;
      case KLOG_PLANTGRASS:        sprintf(logname, "Plantgrass");         break;
      case KLOG_PLANTTREE:         sprintf(logname, "Planttree");          break;
      case KLOG_STOPFIRE:          sprintf(logname, "Stopfire");           break;
      case KLOG_PLAINS:            sprintf(logname, "Plains");             break;
      case KLOG_SURVEY:            sprintf(logname, "Survey");             break;
      case KLOG_SURVEY_STRIKES:    sprintf(logname, "SurveyStrikes");      break;
      case KLOG_LEAVEKINGDOM:      sprintf(logname, "Leavekingdom");       break;     
      case KLOG_TKICKOUT:          sprintf(logname, "Tkickout");           break;
      case KLOG_KICKOUT:           sprintf(logname, "Kickout");            break;
      case KLOG_TINDUCT:           sprintf(logname, "Tinduct");            break;
      case KLOG_KINDUCT:           sprintf(logname, "Kinduct");            break;
      case KLOG_JOINKINGDOM:       sprintf(logname, "Joinkingdom");        break;
      case KLOG_DECLARE:           sprintf(logname, "Declare");            break;
      case KLOG_COMMAND:           sprintf(logname, "Command");            break;
      case KLOG_GIVEORDERS:        sprintf(logname, "Giveorders");         break;
      case KLOG_USELICENSE:        sprintf(logname, "Uselicense");         break;     
      case KLOG_GRANTLICENSE:      sprintf(logname, "Grantlicense");       break;
      case KLOG_ROADS:             sprintf(logname, "Roads");              break;
      case KLOG_FIRE:              sprintf(logname, "Fire");               break;
      case KLOG_CUTPATH:           sprintf(logname, "Cutpath");            break;
      case KLOG_MAKEWORKER:        sprintf(logname, "Makeworkers");        break;
      case KLOG_GETRESOURCES:      sprintf(logname, "Getresources");       break;
      case KLOG_DEPOSIT:           sprintf(logname, "Deposit");            break;
      case KLOG_WITHDRAW:          sprintf(logname, "Withdraw");           break;
      case KLOG_CREATE_ROOM:       sprintf(logname, "Create Room");        break;
      case KLOG_EDIT_ROOM:         sprintf(logname, "Edit Room");          break;
      case KLOG_POP_VICTIM:        sprintf(logname, "PopVictim");          break;     
      case KLOG_POP_ATTACKER:      sprintf(logname, "PopAttacker");        break;
      case KLOG_MIL_VICTIM:        sprintf(logname, "MilVictim");          break;
      case KLOG_MIL_ATTACKER:      sprintf(logname, "MilAttacker");        break;
      case KLOG_MIL_COLLECTION:    sprintf(logname, "MilCollection");      break;
      case KLOG_TAX:               sprintf(logname, "Tax");                break;
      case KLOG_POPULATION:        sprintf(logname, "Population");         break;
      case KLOG_EXTRACTION:        sprintf(logname, "Extraction");         break;
      case KLOG_TRAINERTAX:        sprintf(logname, "Trainertax");         break;
      case KLOG_BOOKTAX:           sprintf(logname, "Booktax");            break;
      case KLOG_CASTEVALUES:       sprintf(logname, "Castevalues");        break;
      case KLOG_TOWNVALUES:        sprintf(logname, "Townvalues");         break;
      case KLOG_KREMOVE_MOB:       sprintf(logname, "Kremove Mob");        break;
      case KLOG_WARLOSSES:         sprintf(logname, "Warlosses");          break;
      case KLOG_SCHEDULE:          sprintf(logname, "Schedule");           break;
      case KLOG_ARMMILITARY:       sprintf(logname, "Armmilitary");           break;
      case KLOG_UNKNOWN:           sprintf(logname, "Unknown");            break;
      default:                     sprintf(logname, "Unknown");            break;
   }
   return logname;
}

//Keep these in order since I am returning them in order :-)
//NO SPACES, will screw up the parsing in the Names
int search_logname(char *logname)
{
   if (!str_cmp(logname, "SetJob"))
      return 0;
   if (!str_cmp(logname, "Tradegoods"))
      return 1;   
   if (!str_cmp(logname, "Resources"))
      return 2;
   if (!str_cmp(logname, "Placeobj"))
      return 3;
   if (!str_cmp(logname, "KremoveTrainer"))
      return 4;
   if (!str_cmp(logname, "KremoveObj"))
      return 5;
   if (!str_cmp(logname, "PlaceTrainer"))
      return 6;
   if (!str_cmp(logname, "Placemob"))
      return 7;
   if (!str_cmp(logname, "Plantgrain"))  
      return 8;   
   if (!str_cmp(logname, "Plantcorn"))
      return 9;
   if (!str_cmp(logname, "Plantgrass"))
      return 10;
   if (!str_cmp(logname, "Planttree"))
      return 11;
   if (!str_cmp(logname, "Stopfire"))
      return 12;
   if (!str_cmp(logname, "Plains"))
      return 13;
   if (!str_cmp(logname, "Survey"))
      return 14;
   if (!str_cmp(logname, "SurveyStrikes"))
      return 15;
   if (!str_cmp(logname, "Leavekingdom"))  
      return 16;  
   if (!str_cmp(logname, "Tkickout"))
      return 17;
   if (!str_cmp(logname, "Kickout"))
      return 18;
   if (!str_cmp(logname, "Tinduct"))
      return 19;
   if (!str_cmp(logname, "Kinduct"))
      return 20;
   if (!str_cmp(logname, "Joinkingdom"))
      return 21;
   if (!str_cmp(logname, "Declare"))
      return 22;
   if (!str_cmp(logname, "Command"))
      return 23;
   if (!str_cmp(logname, "Giveorders"))
      return 24;
   if (!str_cmp(logname, "Uselicense"))  
      return 25;  
   if (!str_cmp(logname, "Grantlicense"))
      return 26;
   if (!str_cmp(logname, "Roads"))
      return 27;
   if (!str_cmp(logname, "Fire"))
      return 28;
   if (!str_cmp(logname, "Cutpath"))
      return 29;
   if (!str_cmp(logname, "Makeworkers"))
      return 30;
   if (!str_cmp(logname, "Getresources"))
      return 31;
   if (!str_cmp(logname, "Deposit"))
      return 32;
   if (!str_cmp(logname, "Withdraw"))
      return 33;
   if (!str_cmp(logname, "CreateRoom"))
      return 34;
   if (!str_cmp(logname, "EditRoom"))
      return 35;
   if (!str_cmp(logname, "PopVictim"))
      return 36;
   if (!str_cmp(logname, "PopAttacker"))
      return 37;
   if (!str_cmp(logname, "MilVictim"))
      return 38;
   if (!str_cmp(logname, "MilAttacker"))
      return 39;
   if (!str_cmp(logname, "MilCollection"))
      return 40;
   if (!str_cmp(logname, "KlogTax"))
      return 41;
   if (!str_cmp(logname, "Population"))
      return 42;
   if (!str_cmp(logname, "Extraction"))
      return 43;
   if (!str_cmp(logname, "Trainertax"))
      return 44;
   if (!str_cmp(logname, "Booktax"))
      return 45;
   if (!str_cmp(logname, "Castevalues"))
      return 46;
   if (!str_cmp(logname, "Townvalues"))
      return 47;
   if (!str_cmp(logname, "Kremove Mob"))
      return 48;
   if (!str_cmp(logname, "Warlosses"))
      return 49;
   if (!str_cmp(logname, "Unknown"))
      return 50;
   if (!str_cmp(logname, "Schedule"))
      return 51;
   if (!str_cmp(logname, "Armmilitary"))
      return 52;
   
   return KLOG_UNKNOWN;
}

//Searching will be done with klog, so we can simplify this a bit
void write_kingdom_logfile(int ht, char *buf, int flag)
{
   char kbuf[MIL];
   char fbuf[MIL];
   
   sprintf(kbuf, "%-10d  %-15s  %s", (int)time(0), return_logname(flag), buf);
   sprintf(fbuf, "%-15s  %-10d  %-15s  %s", kingdom_table[ht]->name, (int)time(0), return_logname(flag), buf);
   
   if (!xIS_SET(kingdom_table[ht]->logsettings, flag))
   {
      append_to_file(kingdom_table[ht]->logfile, kbuf);
      append_to_file(KINGDOM_MLOG_FILE, fbuf);
   }
   else
   {  
      append_to_file(KINGDOM_MLOG_FILE, fbuf);
   }
}

void do_extract(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   sh_int rtype = 0;
   char *message;
   char *rmessage;
   char buf[MSL];
   int ofound = 0;
   int rtaken = 0;
   int mod = 1;
   int sector;
   int move;

   if (IS_NPC(ch))
   {
      send_to_char("Extract is only for players.\n\r", ch);
      return;
   }
   if (ch->pcdata->job != 1)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("Can only extract the Earth while on the Wilderness.\n\r", ch);
      return;
   }
   
   if (ch->position != POS_STANDING)
   {
      send_to_char("You can only do this standing up.\n\r", ch);
      return;
   }
   /* Behold the power of the IF CHECKS!!!! -- Xerves 12/99 */
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   rtype = get_resourcetype(sector, 0);

   if (rtype == 0)
   {
      send_to_char("There are no natural products to extract here.\n\r", ch);
      return;
   }
   if (resource_sector[ch->map][ch->coord->x][ch->coord->y] <= 0 && rtype > 0)
   {
      send_to_char("All the natural products are extracted here.\n\r", ch);
      return;
   }
   if (ch->pcdata->resourcetype != rtype)
   {
      send_to_char("You are not properly trained to extract here.\n\r", ch);
      return;
   }
   /* v0 is type, v1 is efficiency...Types are the same as rtype */
   for (obj = ch->first_carrying; obj; obj = obj->next_content)
   {
      if (obj->item_type == ITEM_EXTRACTOBJ)
      {
         if (rtype != obj->value[0])
         {
            ofound = 2;
            continue;
         }
         else
         {
            ofound = 1;
            break;
         }
      }
      continue;
   }
   if (ofound == 2)
   {
      send_to_char("You don't have the right equipment to extract here.\n\r", ch);
      return;
   }
   if (ofound == 0)
   {
      send_to_char("You don't have any extraction equipment, that makes it kind of hard!\n\r", ch);
      return;
   }
   if (obj->value[1] == 0)
   {
      sprintf(buf, "%d: Type extractobj with no v1", obj->pIndexData->vnum);
      bug(buf, 0);
      send_to_char("There is a problem with your equipment, ask an immortal for help.\n\r", ch);
      return;
   }
   if (rtype == KRES_GOLD || rtype == KRES_IRON)
   {
      rtaken = obj->value[1] / 3;
      mod = 3;
   }
   if (rtype == KRES_CORN || rtype == KRES_GRAIN || rtype == KRES_LUMBER || rtype == KRES_FISH)
   {
      rtaken = obj->value[1] / 2.5;
   }
   if (rtype == KRES_STONE)
   {
      rtaken = obj->value[1] / 2;
   }
   if (((get_curr_str(ch) * 6) / mod) == ch->pcdata->resource)
   {
      send_to_char("You cannot hold any more\n\r", ch);
      return;
   }
   if (((get_curr_str(ch) * 6) / mod) < ch->pcdata->resource + rtaken)
   {
      rtaken = ((get_curr_str(ch) * 6) / mod) - ch->pcdata->resource;
   }
   if (resource_sector[ch->map][ch->coord->x][ch->coord->y] - rtaken < 0)
   {
      rtaken = resource_sector[ch->map][ch->coord->x][ch->coord->y];
   }
   if (resource_sector[ch->map][ch->coord->x][ch->coord->y] == 0)
   {
      send_to_char("This resource is completely empty.\n\r", ch);
      return;
   }
   ch->pcdata->resource += rtaken;
   resource_sector[ch->map][ch->coord->x][ch->coord->y] -= rtaken;
   if (rtype == KRES_CORN || rtype == KRES_GRAIN || rtype == KRES_LUMBER || rtype == KRES_STONE)
   {
      if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 2400)
      {
         if (rtype == KRES_CORN)
            sector = SECT_SCORN;
         if (rtype == KRES_GRAIN)
            sector = SECT_SGRAIN;
         if (rtype == KRES_LUMBER)
            sector = SECT_STREE;
         if (rtype == KRES_STONE)
            sector = SECT_SSTONE;
      }
      if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 1)
      {
         if (rtype == KRES_CORN)
            sector = SECT_NCORN;
         if (rtype == KRES_GRAIN)
            sector = SECT_NGRAIN;
         if (rtype == KRES_LUMBER)
            sector = SECT_NTREE;
         if (rtype == KRES_STONE)
            sector = SECT_NSTONE;
      }
   }
   if (rtype == KRES_GOLD || rtype == KRES_IRON)
   {
      if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 1200)
      {
         if (rtype == KRES_GOLD)
            sector = SECT_SGOLD;
         if (rtype == KRES_IRON)
            sector = SECT_SIRON;
      }
      if (resource_sector[ch->map][ch->coord->x][ch->coord->y] < 1)
      {
         if (rtype == KRES_GOLD)
            sector = SECT_NGOLD;
         if (rtype == KRES_IRON)
            sector = SECT_SIRON;
      }
   }
   message = " ";
   rmessage = " ";
   if (rtype == KRES_GOLD)
   {
      message = "You quickly pull out your equipment and mine the gold.";
      rmessage = "$n quickly pulls out $s equipment and mines some gold.";
   }
   if (rtype == KRES_IRON)
   {
      message = "You quickly pull out your equipment and mine the iron.";
      rmessage = "$n quickly pulls out $s equipment and mines some iron.";
   }
   if (rtype == KRES_CORN)
   {
      message = "You quickly pull out your equipment and remove some corn from the land.";
      rmessage = "$n quickly pulls out $s equipment and removes some corn from the land.";
   }
   if (rtype == KRES_GRAIN)
   {
      message = "You quickly pull out your equipment and remove some grain from the land.";
      rmessage = "$n quickly pulls out $s equipment and removes some grain from the land.";
   }
   if (rtype == KRES_LUMBER)
   {
      message = "You quickly pull out your equipment and start to chop down some trees.";
      rmessage = "$n quickly pulls out $s equipment and starts to chop down some trees.";
   }
   if (rtype == KRES_STONE)
   {
      message = "You quickly pull out your equipment and start to extract some stone.";
      rmessage = "$n quickly pulls out $s equipment and starts to extract some stone.";
   }
   if (rtype == KRES_FISH)
   {
      message = "You quickly pull out your fishing pole and start to fish.";
      rmessage = "$n quickly pulls out $s fishing pole and starts to fish.";
   }
   act(AT_ACTION, message, ch, NULL, NULL, TO_CHAR);
   act(AT_ACTION, rmessage, ch, NULL, NULL, TO_CANSEE);
   
   if (ch->mover < 15)
      move = 20 * (5 - (.1 * (ch->mover-5)));
   else if (ch->mover < 25)
      move = 20 * (4 - (.1 * (ch->mover-15)));
   else if (ch->mover < 35)
      move = 20 * (3 - (.1 * (ch->mover-25)));
   else if (ch->mover < 45)
      move = 20 * (2 - (.1 * (ch->mover-35)));
   else if (ch->mover < 55)
      move = 20 * (1.3 - (.07 * (ch->mover-45)));
   else if (ch->mover < 65)
      move = 20 * (.95 - (.035 * (ch->mover-55)));
   else
      move = 20 * (.8 - (.015 * (ch->mover-65)));
      
   if (rtype == KRES_GOLD || rtype == KRES_IRON)
   {
      move = move * 1.3;
      ch->move -= move;
   }
   if (rtype >= KRES_CORN || rtype <= KRES_FISH)
      ch->move -= move;
   map_sector[ch->map][ch->coord->x][ch->coord->y] = sector;
   update_movement_points(ch, move);
   WAIT_STATE(ch, 4);

}


/*******************************************************
     Merchant Code and Handling by Xerves
     The Below Code will handle all the merchants
     and the mobs.

     Note:  Moved all over the place because it was
     easier that way.

     Xerves -- 9/1/99
 *******************************************************/

 /******************************************************
     OOH another one.  Gambling Job system starts below.

     Xerves - 10/4/99
   *****************************************************/
void do_setgambler(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gambler;
   int found = 0;
   char arg1[MIL];
   int value;

   argument = one_argument(argument, arg1);


   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot have a gambler\n\r", ch);
      return;
   }
   if (!ch->pcdata || !ch->pcdata->keeper || (ch->pcdata->caste < 7))
   {
      send_to_char("You need to have a gambler to see its stats.\n\r", ch);
      return;
   }

   for (gambler = ch->in_room->first_person; gambler; gambler = gambler->next_in_room)
   {
      if ((IS_NPC(gambler)) && (xIS_SET(gambler->act, ACT_CASTEMOB)) && (gambler->pIndexData->m5 == ch->pcdata->keeper))
      {
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      send_to_char("Either you don't have a gambler or you are not in the room with it\n\r", ch);
      return;
   }
   if (arg1[0] == '\0')
   {
      send_to_char("Usage: setgambler <field> value\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("minbet maxbet reward\n\r", ch);
      if (gambler->pIndexData->cident == 5)
         send_to_char("holdat\n\r", ch);
      return;
   }
   value = atoi(argument);
   if (!str_cmp(arg1, "minbet"))
   {
      if (value > gambler->pIndexData->m2)
      {
         send_to_char("Your minimum cannot be higher than the maximum\n\r", ch);
         return;
      }
      if (value < 1)
      {
         send_to_char("Value needs to be 1 or greater\n\r", ch);
         return;
      }
      gambler->pIndexData->m1 = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (gambler->pIndexData->cident == 5)
   {
      if (!str_cmp(arg1, "holdat"))
      {
         if (value > 21 || value < 0)
         {
            send_to_char("21 is the max to hold on while 0 in the min\n\r", ch);
            return;
         }
         gambler->pIndexData->m6 = value;
         send_to_char("Done.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg1, "maxbet"))
   {
      if (value < gambler->pIndexData->m1)
      {
         send_to_char("You maximum cannot be lower than the minimum\n\r", ch);
         return;
      }
      if (value < 1)
      {
         send_to_char("Value needs to be 1 or greater\n\r", ch);
         return;
      }
      gambler->pIndexData->m2 = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "reward"))
   {
      if (value < 1)
      {
         send_to_char("Value needs to be 1 or greater\n\r", ch);
         return;
      }
      gambler->pIndexData->m4 = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   do_setgambler(ch, "");
   return;
}
void do_showgambler(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gambler;
   int found = 0;

   /* Going to use this to show all gambling mobs.  Will identify the
      cident of the mob first and then start that code */
   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot have a gambler\n\r", ch);
      return;
   }
   for (gambler = ch->in_room->first_person; gambler; gambler = gambler->next_in_room)
   {
      if ((IS_NPC(gambler)) && (xIS_SET(gambler->act, ACT_CASTEMOB)) && (gambler->pIndexData->m5 == ch->pcdata->keeper))
      {
         found = 1;
         break;
      }
   }
   if (found == 0)
   {
      send_to_char("Either you don't have a shopkeeper or you are not in the room with it\n\r", ch);
      return;
   }
   if (!ch->pcdata || (ch->pcdata->keeper != gambler->pIndexData->m5))
   {
      send_to_char("You need to have a gambler to see its stats.\n\r", ch);
      return;
   }
   if (gambler->pIndexData->cident == 3)
      ch_printf_color(ch, "Type: Shells          Reward: %-3d\n\r", gambler->pIndexData->m4);
   if (gambler->pIndexData->cident == 4)
      ch_printf_color(ch, "Type: Coins           Reward: %-3d\n\r", gambler->pIndexData->m4);
   if (gambler->pIndexData->cident == 5)
      ch_printf_color(ch, "Type: Blackjack       Reward: %-3d    Hold At: %-2d\n\r", gambler->pIndexData->m4, gambler->pIndexData->m6);

   ch_printf_color(ch, "Min Bet: %-10s   ", punct(gambler->pIndexData->m1));
   ch_printf_color(ch, "Max Bet: %-10s\n\r", punct(gambler->pIndexData->m2));

   return;
}
void do_flipcoin(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gambler;
   char arg[MIL];
   char arg2[MIL];
   char buf[MIL];
   int result;
   int goldbid;
   int MAX_BET;
   int MIN_BET;
   sh_int REWARD;
   AREA_DATA *pArea;

   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg2);
   result = number_range(1, 2);

   goldbid = atoi(arg);

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot gamble.\n\r", ch);
      return;
   }
   /* Castemob?  Gotta love em :-) */
   for (gambler = ch->in_room->first_person; gambler != NULL; gambler = gambler->next_in_room)
   {
      if (IS_NPC(gambler) && xIS_SET(gambler->act, ACT_CASTEMOB) && (gambler->pIndexData->cident == 4))
         break;
   }

   if (gambler == NULL)
   {
      send_to_char("You can't do that here!.\n\r", ch);
      return;
   }
   pArea = gambler->in_room->area;
   if (arg[0] == '\0' || arg2[0] == '\0') /* You must bet something */
   {
      send_to_char("Syntax: flipcoin <bid> <side>\n\r", ch);
      send_to_char("<side> : heads or tails\n\r", ch);
      return;
   }
   if (gambler->pIndexData->m1 != 0 && gambler->pIndexData->m2 != 0 && gambler->pIndexData->m4 != 0)
   {
      MIN_BET = gambler->pIndexData->m1;
      MAX_BET = gambler->pIndexData->m2;
      REWARD = (gambler->pIndexData->m4 * goldbid) / 100;
   }
   else
   {
      MIN_BET = 200;
      MAX_BET = 5000;
      REWARD = (150 * goldbid) / 100;
   }
   if (goldbid < MIN_BET || goldbid > MAX_BET)
   {
      sprintf(buf, "Min on the house is %d, while the Max is %d \n\r", MIN_BET, MAX_BET);
      send_to_char(buf, ch);
      return;
   }

   /* Checks mob is not fighting */
   if (gambler->position == POS_FIGHTING)
   {
      send_to_char("Might want to wait for the fight to stop.\n\r", ch);
      return;
   }
   /* Checks player has enough money */
   if (ch->gold < goldbid)
   {
      sprintf(buf, "You must have %d gold coins, at least, to do bets.\n\r", goldbid);
      send_to_char(buf, ch);
      return;
   }
   /* Check to see if the gambler has enough money :-) -- Xerves */
   if (gambler->gold < goldbid)
   {
      sprintf(buf, "%s does not have that much money, bet a bit lower.\n\r", gambler->name);
      send_to_char(buf, ch);
      return;
   }
   if (!strcmp(arg2, "heads") || !strcmp(arg2, "tails"))
   {
      ch->gold -= goldbid; /* He takes away bet amount from your gold */
      gambler->pIndexData->gold += goldbid;
      gambler->gold += goldbid;
      /* Variable reponses :-) -- Xerves */
      /* Entry Program that will do this
         sprintf(buf, "$N puts a coin on the table and places a cup over it, and then $E\n\rplaces 2 small cups on the table.  In a blink of an eye, $N starts\n\rto move the cups in a very rapid fashion.  $N continues for a few\n\rseconds and suddenly comes to a stop.  $N laughs and asks $n to\n\rSHELLUNCOVER one of the cups.");

         act(AT_SAY,buf,ch,NULL,gambler,TO_CHAR);
         act(AT_SAY,buf,ch,NULL,gambler,TO_ROOM); */

      sprintf(buf, "$n bets %d gold coins and tells $N the coin will land %s.\n\r", goldbid, arg2);

      act(AT_PLAIN, buf, ch, NULL, gambler, TO_CHAR);
      act(AT_PLAIN, buf, ch, NULL, gambler, TO_ROOM);

      /* Compare choosen cup with random choice */
      if ((!strcmp(arg2, "heads") && result == 1) || (!strcmp(arg2, "tails") && result == 2))
      {
         sprintf(buf,
            "$N flips the coin high into the air and lets it \n\rdrop on $S palm.  $N quickly moves the coin around in $S \n\rhand and then opens it.  $N appears to be very bitter \n\rbecause it landed on %s.  $N quickly hands over %d to $n \n\rand asks if anyone else wants to play?",
            arg2, REWARD);

         act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
         act(AT_SAY, buf, ch, NULL, gambler, TO_ROOM);

         ch->gold += REWARD; /* Pay winner bet */
         gambler->pIndexData->gold -= REWARD;
         gambler->gold -= REWARD;
      }
      else
      {
         sprintf(buf,
            "$N flips the coin high into the air and lets it\n\rdrop on $S palm.  $N quickly moves the coin around in his\n\r hand and then opens it.  $N grins with delight because $E won.\n\r$N smiles more and asks if anyone else wants to play?");

         act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
         act(AT_SAY, buf, ch, NULL, gambler, TO_ROOM);
      }
      fdarea(gambler, pArea->filename);
   }
   else
   {
      send_to_char("Need to choose heads or tails\n\r", ch);
      return;
   }
   return;
}

/* Multiple function game that uses the blackjack globals defined at the
   beginning of this file.  Only 1 can play blackjack with a mob in the
   room, and that 1 cannot leave unless they quit, go link dead, or ask
   the gambler to stop -- Xerves 10/7/99 */
void draw_card(CHAR_DATA * gambler)
{
   sh_int gvalue1 = 0;
   sh_int gvalue2 = 0;
   sh_int gvalue3 = 0;
   sh_int gvalue4 = 0;
   sh_int gvalue5 = 0;
   sh_int x = 0;
   sh_int y = 0;
   sh_int ace1;
   sh_int ace2;
   sh_int left;
   sh_int holdat;

   holdat = gambler->pIndexData->m6;
   if (holdat == 0)
      holdat = 15;

   gcard1 = number_range(1, 13);
   gcard2 = number_range(1, 13);
   if (gcard1 == 1 && gcard2 == 1)
      gcard2 = number_range(2, 13);

   if (gcard1 != 0)
   {
      gvalue1 = gcard1;
      if ((gcard1 == 11) || (gcard1 == 12) || (gcard1 == 13))
      {
         gvalue1 = 10;
      }
      if (gcard1 == 1)
      {
         gvalue1 = 0;
      }
   }
   if (gcard2 != 0)
   {
      gvalue2 = gcard2;
      if ((gcard2 == 11) || (gcard2 == 12) || (gcard2 == 13))
      {
         gvalue2 = 10;
      }
      if (gcard2 == 1)
      {
         gvalue2 = 0;
      }
   }
   gresult = gvalue1 + gvalue2;
   if (gcard1 == 1)
   {
      ace1 = 1;
      ace2 = 11;
      y = 1;
   }
   if (gcard2 == 1)
   {
      ace1 = 1;
      ace2 = 11;
      y = 1;
   }
   left = 21 - gresult;
   x = 0;
   if (y == 1)
   {
      if (left >= 11)
         x = 11;
      else
         x = 1;
   }
   gresult = gresult + x;
   if (gresult >= holdat)
      return; /* 3rd Card below */
   if (gresult < holdat)
   {
      gcard3 = number_range(1, 13);
      if ((gcard1 == 1 && gcard3 == 1) || (gcard2 == 1 && gcard3 == 1))
         gcard3 = number_range(2, 13);
      if (gcard3 != 0)
      {
         gvalue3 = gcard3;
         if ((gcard3 == 11) || (gcard3 == 12) || (gcard3 == 13))
         {
            gvalue3 = 10;
         }
         if (gcard3 == 1)
         {
            gvalue3 = 0;
         }
      }
      y = 0;
      gresult = gresult + gvalue3;
      if (gcard3 == 1)
      {
         ace1 = 1;
         ace2 = 11;
         y = 1;
      }
      left = 21 - gresult;
      x = 0;
      if (y == 1)
      {
         if (left >= 11)
            x = 11;
         else
            x = 1;
      }
      gresult = gresult + x;
   }
   if (gresult >= holdat)
      return;
   if (gresult < holdat) /* 4th card below */
   {
      gcard4 = number_range(1, 13);
      if ((gcard1 == 1 && gcard4 == 1) || (gcard2 == 1 && gcard4 == 1) || (gcard3 == 1 && gcard4 == 1))
         gcard4 = number_range(2, 13);
      if (gcard4 != 0)
      {
         gvalue4 = gcard4;
         if ((gcard4 == 11) || (gcard4 == 12) || (gcard4 == 13))
         {
            gvalue4 = 10;
         }
         if (gcard4 == 1)
         {
            gvalue4 = 0;
         }
      }
      y = 0;
      gresult = gresult + gvalue4;
      if (gcard4 == 1)
      {
         ace1 = 1;
         ace2 = 11;
         y = 1;
      }
      left = 21 - gresult;
      x = 0;
      if (y == 1)
      {
         if (left >= 11)
            x = 11;
         else
            x = 1;
      }
      gresult = gresult + x;
   }
   if (gresult >= holdat)
      return;
   if (gresult < holdat) /* 5th and final card */
   {
      gcard5 = number_range(1, 13);
      if ((gcard1 == 1 && gcard5 == 1) || (gcard2 == 1 && gcard5 == 1) || (gcard3 == 1 && gcard5 == 1) || (gcard4 == 1 && gcard5 == 1))
         gcard5 = number_range(2, 13);
      if (gcard5 != 0)
      {
         gvalue5 = gcard5;
         if ((gcard5 == 11) || (gcard5 == 12) || (gcard5 == 13))
         {
            gvalue5 = 10;
         }
         if (gcard5 == 1)
         {
            gvalue5 = 0;
         }
      }
      y = 0;
      gresult = gresult + gvalue5;
      if (gcard5 == 1)
      {
         ace1 = 1;
         ace2 = 11;
         y = 1;
      }
      left = 21 - gresult;
      x = 0;
      if (y == 1)
      {
         if (left >= 11)
            x = 11;
         else
            x = 1;
      }
      gresult = gresult + x;
   }
   return;
}
void do_blackjack(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gambler;
   CHAR_DATA *compet;
   char buf[MSL];
   char arg[MIL];
   int MAX_BET;
   int MIN_BET;

   argument = one_argument(argument, arg);

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot gamble.\n\r", ch);
      return;
   }
   /* Castemob?  Gotta love em :-) */
   for (gambler = ch->in_room->first_person; gambler != NULL; gambler = gambler->next_in_room)
   {
      if (IS_NPC(gambler) && xIS_SET(gambler->act, ACT_CASTEMOB) && (gambler->pIndexData->cident == 5))
         break;
   }

   if (gambler == NULL)
   {
      send_to_char("You can't do that here!.\n\r", ch);
      return;
   }
   /* Is there a gambler in the room or not? */
   for (compet = ch->in_room->first_person; compet != NULL; compet = compet->next_in_room)
   {
      if (xIS_SET(compet->act, PLR_GAMBLER))
      {
         act(AT_PLAIN, "You must wait till $N finishes first.", ch, NULL, compet, TO_CHAR);
         return;
      }
   }
   /* pArea = gambler->in_room->area; */
   if (arg[0] == '\0') /* You must bet something */
   {
      send_to_char("Syntax: blackjack <bid>\n\r", ch);
      return;
   }
   if (gambler->pIndexData->m1 != 0 && gambler->pIndexData->m2 != 0 && gambler->pIndexData->m4 != 0)
   {
      MIN_BET = gambler->pIndexData->m1;
      MAX_BET = gambler->pIndexData->m2;
   }
   else
   {
      MIN_BET = 200;
      MAX_BET = 5000;
   }
   amtbet = atoi(arg);
   if (amtbet < MIN_BET || amtbet > MAX_BET)
   {
      sprintf(buf, "Min on the house is %d, while the Max is %d \n\r", MIN_BET, MAX_BET);
      send_to_char(buf, ch);
      return;
   }

   /* Checks mob is not fighting */
   if (gambler->position == POS_FIGHTING)
   {
      send_to_char("Might want to wait for the fight to stop.\n\r", ch);
      return;
   }
   /* Checks player has enough money */
   if (ch->gold < amtbet)
   {
      sprintf(buf, "You must have %d gold coins, at least, to do bets.\n\r", amtbet);
      send_to_char(buf, ch);
      return;
   }
   /* Check to see if the gambler has enough money :-) -- Xerves */
   if (gambler->gold < amtbet)
   {
      sprintf(buf, "%s does not have that much money, bet a bit lower.\n\r", gambler->name);
      send_to_char(buf, ch);
      return;
   }
   ch->gold -= amtbet; /* He takes away bet amount from your gold */
   gambler->pIndexData->gold += amtbet;
   gambler->gold += amtbet;

   strcpy(buf2, " ");
   /* Clean out el globels -- Xerves */
   pcard1 = 0;
   pcard2 = 0;
   pcard3 = 0;
   pcard4 = 0;
   pcard5 = 0;
   gcard1 = 0;
   gcard2 = 0;
   gcard3 = 0;
   gcard4 = 0;
   gcard5 = 0;
   gresult = 0;
   /* 1 - Ace  2-2 3-3 4-4 5-5 6-6 7-7 8-8 9-9 10-10 11-Jack 12-Queen
      13- King */
   pcard1 = number_range(1, 13);
   pcard2 = number_range(1, 13);
   if (pcard1 == 1)
      strcat(buf2, " an ace");
   if (pcard1 == 2)
      strcat(buf2, " a 2");
   if (pcard1 == 3)
      strcat(buf2, " a 3");
   if (pcard1 == 4)
      strcat(buf2, " a 4");
   if (pcard1 == 5)
      strcat(buf2, " a 5");
   if (pcard1 == 6)
      strcat(buf2, " a 6");
   if (pcard1 == 7)
      strcat(buf2, " a 7");
   if (pcard1 == 8)
      strcat(buf2, " a 8");
   if (pcard1 == 9)
      strcat(buf2, " a 9");
   if (pcard1 == 10)
      strcat(buf2, " a 10");
   if (pcard1 == 11)
      strcat(buf2, " a jack");
   if (pcard1 == 12)
      strcat(buf2, " a queen");
   if (pcard1 == 13)
      strcat(buf2, " a king");
   if (pcard2 == 1)
      strcat(buf2, " an ace");
   if (pcard2 == 2)
      strcat(buf2, " a 2");
   if (pcard2 == 3)
      strcat(buf2, " a 3");
   if (pcard2 == 4)
      strcat(buf2, " a 4");
   if (pcard2 == 5)
      strcat(buf2, " a 5");
   if (pcard2 == 6)
      strcat(buf2, " a 6");
   if (pcard2 == 7)
      strcat(buf2, " a 7");
   if (pcard2 == 8)
      strcat(buf2, " a 8");
   if (pcard2 == 9)
      strcat(buf2, " a 9");
   if (pcard2 == 10)
      strcat(buf2, " a 10");
   if (pcard2 == 11)
      strcat(buf2, " a jack");
   if (pcard2 == 12)
      strcat(buf2, " a queen");
   if (pcard2 == 13)
      strcat(buf2, " a king");

   sprintf(buf, "$n smiles as $n bets %d coins.  \n\r$N sits down and starts to deal.", amtbet);
   act(AT_SAY, buf, ch, NULL, gambler, TO_ROOM);
   sprintf(buf,
      "$N quickly cuts the deck and hands out two hards\n\rto both $Mself and you.  You receive your cards \n\rand take a quick glance.  You have%s.\n\r\n\r  Now, you may either BHOLD or BCARD.",
      buf2);
   act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
   act(AT_SAY, "\n\r$N quickly cuts the deck and deals out\n\rto $n and $mself", ch, NULL, gambler, TO_ROOM);
   draw_card(gambler);
   xSET_BIT(ch->act, PLR_GAMBLER);
   return;
}
void do_bcard(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gambler;
   char buf[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot gamble.\n\r", ch);
      return;
   }
   /* Castemob?  Gotta love em :-) */
   for (gambler = ch->in_room->first_person; gambler != NULL; gambler = gambler->next_in_room)
   {
      if (IS_NPC(gambler) && xIS_SET(gambler->act, ACT_CASTEMOB) && (gambler->pIndexData->cident == 5))
         break;
   }

   if (gambler == NULL)
   {
      send_to_char("You can't do that here!.\n\r", ch);
      return;
   }
   if (!xIS_SET(ch->act, PLR_GAMBLER))
   {
      send_to_char("You have to be in a blackjack game before you ask for a card.\n\r", ch);
      return;
   }
   if (pcard3 == 0)
   {
      pcard3 = number_range(1, 13);
      if (pcard3 == 1)
         strcat(buf2, " an ace");
      if (pcard3 == 2)
         strcat(buf2, " a 2");
      if (pcard3 == 3)
         strcat(buf2, " a 3");
      if (pcard3 == 4)
         strcat(buf2, " a 4");
      if (pcard3 == 5)
         strcat(buf2, " a 5");
      if (pcard3 == 6)
         strcat(buf2, " a 6");
      if (pcard3 == 7)
         strcat(buf2, " a 7");
      if (pcard3 == 8)
         strcat(buf2, " a 8");
      if (pcard3 == 9)
         strcat(buf2, " a 9");
      if (pcard3 == 10)
         strcat(buf2, " a 10");
      if (pcard3 == 11)
         strcat(buf2, " a jack");
      if (pcard3 == 12)
         strcat(buf2, " a queen");
      if (pcard3 == 13)
         strcat(buf2, " a king");
      sprintf(buf, "$N deals you another card because you requested it.\n\rYou now have%s", buf2);
      act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
      act(AT_SAY, "$n asks for another card, and $N deals it out.", ch, NULL, gambler, TO_CANSEE);
      return;
   }
   if (pcard4 == 0)
   {
      pcard4 = number_range(1, 13);
      if (pcard4 == 1)
         strcat(buf2, " an ace");
      if (pcard4 == 2)
         strcat(buf2, " a 2");
      if (pcard4 == 3)
         strcat(buf2, " a 3");
      if (pcard4 == 4)
         strcat(buf2, " a 4");
      if (pcard4 == 5)
         strcat(buf2, " a 5");
      if (pcard4 == 6)
         strcat(buf2, " a 6");
      if (pcard4 == 7)
         strcat(buf2, " a 7");
      if (pcard4 == 8)
         strcat(buf2, " a 8");
      if (pcard4 == 9)
         strcat(buf2, " a 9");
      if (pcard4 == 10)
         strcat(buf2, " a 10");
      if (pcard4 == 11)
         strcat(buf2, " a jack");
      if (pcard4 == 12)
         strcat(buf2, " a queen");
      if (pcard4 == 13)
         strcat(buf2, " a king");
      sprintf(buf, "$N deals you another card because you requested it.\n\rYou now have%s", buf2);
      act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
      act(AT_SAY, "$n aks for another card, and $N deals it out.", ch, NULL, gambler, TO_CANSEE);
      return;
   }
   if (pcard5 == 0)
   {
      pcard5 = number_range(1, 13);
      if (pcard5 == 1)
         strcat(buf2, " an ace");
      if (pcard5 == 2)
         strcat(buf2, " a 2");
      if (pcard5 == 3)
         strcat(buf2, " a 3");
      if (pcard5 == 4)
         strcat(buf2, " a 4");
      if (pcard5 == 5)
         strcat(buf2, " a 5");
      if (pcard5 == 6)
         strcat(buf2, " a 6");
      if (pcard5 == 7)
         strcat(buf2, " a 7");
      if (pcard5 == 8)
         strcat(buf2, " a 8");
      if (pcard5 == 9)
         strcat(buf2, " a 9");
      if (pcard5 == 10)
         strcat(buf2, " a 10");
      if (pcard5 == 11)
         strcat(buf2, " a jack");
      if (pcard5 == 12)
         strcat(buf2, " a queen");
      if (pcard5 == 13)
         strcat(buf2, " a king");
      sprintf(buf, "$N deals you another card because you requested it.\n\rYou now have%s", buf2);
      act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
      act(AT_SAY, "$n aks for another card, and $N deals it out.", ch, NULL, gambler, TO_CANSEE);
      return;
   }
   if (pcard5 != 0)
   {
      act(AT_SAY, "$N says 'I can only give you 5 cards $n.'", ch, NULL, gambler, TO_CHAR);
      return;
   }
}

 /* Shells, works simular to Desden's shell snippet, but uses different
    support -- Xerves 10/4/99 */
 /* Eh just drop bet and uncover into the same function, makes sense to me.. */
void do_bhold(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gambler;
   char buf[MSL];
   sh_int result;
   sh_int pvalue1 = 0;
   sh_int pvalue2 = 0;
   sh_int pvalue3 = 0;
   sh_int pvalue4 = 0;
   sh_int pvalue5 = 0;
   sh_int s;
   sh_int w;
   sh_int x = 0;
   sh_int y;
   sh_int z = 0;
   sh_int left;
   sh_int ace[50];
   sh_int results[100];
   sh_int win = 0;
   sh_int REWARD;
   AREA_DATA *pArea;

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot gamble.\n\r", ch);
      return;
   }
   /* Castemob?  Gotta love em :-) */
   for (gambler = ch->in_room->first_person; gambler != NULL; gambler = gambler->next_in_room)
   {
      if (IS_NPC(gambler) && xIS_SET(gambler->act, ACT_CASTEMOB) && (gambler->pIndexData->cident == 5))
         break;
   }

   if (gambler == NULL)
   {
      send_to_char("You can't do that here!.\n\r", ch);
      return;
   }
   if (!xIS_SET(ch->act, PLR_GAMBLER))
   {
      send_to_char("You have to be in a blackjack game before you can finish it..\n\r", ch);
      return;
   }
   if (pcard1 != 0)
   {
      pvalue1 = pcard1;
      if ((pcard1 == 11) || (pcard1 == 12) || (pcard1 == 13))
      {
         pvalue1 = 10;
      }
      if (pcard1 == 1)
      {
         pvalue1 = 0;
      }
   }
   if (pcard2 != 0)
   {
      pvalue2 = pcard2;
      if ((pcard2 == 11) || (pcard2 == 12) || (pcard2 == 13))
      {
         pvalue2 = 10;
      }
      if (pcard2 == 1)
      {
         pvalue2 = 0;
      }
   }
   if (pcard3 != 0)
   {
      pvalue3 = pcard3;
      if ((pcard3 == 11) || (pcard3 == 12) || (pcard3 == 13))
      {
         pvalue3 = 10;
      }
      if (pcard3 == 1)
      {
         pvalue3 = 0;
      }
   }
   if (pcard4 != 0)
   {
      pvalue4 = pcard4;
      if ((pcard4 == 11) || (pcard4 == 12) || (pcard4 == 13))
      {
         pvalue4 = 10;
      }
      if (pcard4 == 1)
      {
         pvalue4 = 0;
      }
   }
   if (pcard5 != 0)
   {
      pvalue5 = pcard5;

      if ((pcard5 == 11) || (pcard5 == 12) || (pcard5 == 13))
      {
         pvalue5 = 10;
      }
      if (pcard5 == 1)
      {
         pvalue5 = 0;
      }
   }
   result = pvalue1 + pvalue2 + pvalue3 + pvalue4 + pvalue5;
   w = 1;
   y = 0;
   if (pcard1 == 1)
   {
      ace[w] = 1;
      w++;
      y++;
      ace[w] = 11;
      w++;
      y++;
   }
   if (pcard2 == 1)
   {
      ace[w] = 1;
      w++;
      y++;
      ace[w] = 11;
      w++;
      y++;
   }
   if (pcard3 == 1)
   {
      ace[w] = 1;
      w++;
      y++;
      ace[w] = 11;
      w++;
      y++;
   }
   if (pcard4 == 1)
   {
      ace[w] = 1;
      w++;
      y++;
      ace[w] = 11;
      w++;
      y++;
   }
   if (pcard5 == 1)
   {
      ace[w] = 1;
      w++;
      y++;
      ace[w] = 11;
      w++;
      y++;
   }
   left = 21 - result;
   x = 0;
   if (y == 2)
   {
      if (left >= 11)
         x = 11;
      else
         x = 1;
   }
   if (y == 4)
   {
      results[1] = (ace[1] + ace[3]);
      results[2] = (ace[1] + ace[4]);
      results[3] = (ace[2] + ace[3]);
      results[4] = (ace[2] + ace[4]);

      results[5] = left - results[1];
      results[6] = left - results[2];
      results[7] = left - results[3];
      results[8] = left - results[4];
      y = 9;
      z = 0;
      s = 13;
      if (results[5] >= 0)
      {
         results[y] = results[5];
         results[s] = results[1];
         y++;
         s++;
         z++;
      }
      if (results[6] >= 0)
      {
         results[y] = results[6];
         results[s] = results[2];
         y++;
         s++;
         z++;
      }
      if (results[7] >= 0)
      {
         results[y] = results[7];
         results[s] = results[3];
         y++;
         s++;
         z++;
      }
      if (results[8] >= 0)
      {
         results[y] = results[8];
         results[s] = results[4];
         y++;
         s++;
         z++;
      }
      if (z == 0)
         x = 21;
      if (z == 1)
         x = results[13];
      if (z == 2)
      {
         if (results[9] <= results[10])
            x = results[13];
         else
            x = results[14];
      }
      if (z == 3)
      {
         if ((results[9] <= results[10]) && (results[9] <= results[11]))
            x = results[13];
         if ((results[10] <= results[9]) && (results[10] <= results[11]))
            x = results[14];
         if ((results[11] <= results[9]) && (results[11] <= results[10]))
            x = results[15];
      }
      if (z == 4)
      {
         if ((results[9] <= results[10]) && (results[9] <= results[11]) && (results[9] <= results[12]))
            x = results[13];
         if ((results[10] <= results[9]) && (results[10] <= results[11]) && (results[10] <= results[12]))
            x = results[14];
         if ((results[11] <= results[10]) && (results[11] <= results[9]) && (results[11] <= results[12]))
            x = results[15];
         if ((results[12] <= results[11]) && (results[12] <= results[10]) && (results[12] <= results[9]))
            x = results[16];
      }
   }
   result = result + x;

   if (gambler->pIndexData->m1 != 0 && gambler->pIndexData->m2 != 0 && gambler->pIndexData->m4 != 0)
      REWARD = (gambler->pIndexData->m4 * amtbet) / 100;
   else
      REWARD = (200 * amtbet) / 100;

   if (result > 21 && gresult > 21)
   {
      send_to_char("You both lose", ch);
      win = 1;
   }
   if (result > 21 && gresult <= 21)
   {
      send_to_char("You lose", ch);
      win = 2;
   }
   if (result <= 21 && gresult > 21)
   {
      send_to_char("You win", ch);
      win = 3;
   }
   if (result < 22 && gresult < 22)
   {
      if (result > gresult)
      {
         send_to_char("You win", ch);
         win = 3;
      }
   }
   if (result < 22 && gresult < 22)
   {
      if (result < gresult)
      {
         send_to_char("You lose", ch);
         win = 2;
      }
   }
   if (result < 22 && gresult < 22)
   {
      if (result == gresult)
      {
         send_to_char("A tie", ch);
         win = 1;
      }
   }
   /* win var 1 - tie  2 - lose  3 - win */
   send_to_char("\n\r", ch);
   sprintf(buf, "You: %d  Opp: %dt %d %d %d %d %d (%d)\n\r", result, gresult, gcard1, gcard2, gcard3, gcard4, gcard5, z);
   send_to_char(buf, ch);
   if (win == 3)
   {
      ch->gold += REWARD; /* Pay winner bet */
      gambler->pIndexData->gold -= REWARD;
      gambler->gold -= REWARD;
   }
   if (win == 1)
   {
      ch->gold += amtbet;
      gambler->pIndexData->gold -= amtbet;
      gambler->gold -= amtbet;
   }
   xREMOVE_BIT(ch->act, PLR_GAMBLER);
   if (win == 1)
      act(AT_SAY,
         "$n looks at $s cards and then drops them \n\ron the table.  $N smiles and does the \n\rsame thing.  They both quickly look at their cards and find out \n\rtheir results tied.  $N gives back $n $s bet",
         ch, NULL, gambler, TO_ROOM);
   if (win == 2)
      act(AT_SAY,
         "$n looks at $s cards and then drops them \n\ron the table.  $N smiles greatly and does \n\rthe same thing.  They both quickly look at their cards and \n\r$N smiles because $E won.",
         ch, NULL, gambler, TO_ROOM);
   if (win == 3)
      act(AT_SAY,
         "$n looks at $s cards and then drops them \n\ron the table.  $N smiles and does the \n\rsame thing.  They both quickly look at their cards and \n\r$n smiles because $e has won.",
         ch, NULL, gambler, TO_ROOM);
   pArea = gambler->in_room->area;
   fdarea(gambler, pArea->filename);
   return;
}

 /* Shells, works simular to Desden's shell snippet, but uses different
    support -- Xerves 10/4/99 */
 /* Eh just drop bet and uncover into the same function, makes sense to me.. */
void do_shells(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gambler;
   char buf[MSL];
   char arg[MIL];
   char arg2[MIL];
   int result;
   int goldbid;
   int MAX_BET;
   int MIN_BET;
   sh_int REWARD;
   AREA_DATA *pArea; /* Passing resets to special mobs */


   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg2);
   result = number_range(1, 3);

   goldbid = atoi(arg);

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot gamble.\n\r", ch);
      return;
   }
   /* Castemob?  Gotta love em :-) */
   for (gambler = ch->in_room->first_person; gambler != NULL; gambler = gambler->next_in_room)
   {
      if (IS_NPC(gambler) && xIS_SET(gambler->act, ACT_CASTEMOB) && (gambler->pIndexData->cident == 3))
         break;
   }

   if (gambler == NULL)
   {
      send_to_char("You can't do that here!.\n\r", ch);
      return;
   }
   pArea = gambler->in_room->area;
   if (arg[0] == '\0' || arg2[0] == '\0') /* You must bet something */
   {
      send_to_char("Syntax: shells <bid> <cup>\n\r", ch);
      send_to_char("<cup> : cup1, cup2, or cup3\n\r", ch);
      return;
   }
   if (gambler->pIndexData->m1 != 0 && gambler->pIndexData->m2 != 0 && gambler->pIndexData->m4 != 0)
   {
      MIN_BET = gambler->pIndexData->m1;
      MAX_BET = gambler->pIndexData->m2;
      REWARD = (gambler->pIndexData->m4 * goldbid) / 100;
   }
   else
   {
      MIN_BET = 200;
      MAX_BET = 5000;
      REWARD = (200 * goldbid) / 100;
   }
   if (goldbid < MIN_BET || goldbid > MAX_BET)
   {
      sprintf(buf, "Min on the house is %d, while the Max is %d \n\r", MIN_BET, MAX_BET);
      send_to_char(buf, ch);
      return;
   }

   /* Checks mob is not fighting */
   if (gambler->position == POS_FIGHTING)
   {
      send_to_char("Might want to wait for the fight to stop.\n\r", ch);
      return;
   }
   /* Checks player has enough money */
   if (ch->gold < goldbid)
   {
      sprintf(buf, "You must have %d gold coins, at least, to do bets.\n\r", goldbid);
      send_to_char(buf, ch);
      return;
   }
   /* Check to see if the gambler has enough money :-) -- Xerves */
   if (gambler->gold < goldbid)
   {
      sprintf(buf, "%s does not have that much money, bet a bit lower.\n\r", gambler->name);
      send_to_char(buf, ch);
      return;
   }
   if (!strcmp(arg2, "cup1") || !strcmp(arg2, "cup2") || !strcmp(arg2, "cup3"))
   {
      ch->gold -= goldbid; /* He takes away bet amount from your gold */
      gambler->pIndexData->gold += goldbid;
      gambler->gold += goldbid;
      /* Variable reponses :-) -- Xerves */
      /* Entry Program that will do this
         sprintf(buf, "$N puts a coin on the table and places a cup over it, and then $E\n\rplaces 2 small cups on the table.  In a blink of an eye, $N starts\n\rto move the cups in a very rapid fashion.  $N continues for a few\n\rseconds and suddenly comes to a stop.  $N laughs and asks $n to\n\rSHELLUNCOVER one of the cups.");

         act(AT_SAY,buf,ch,NULL,gambler,TO_CHAR);
         act(AT_SAY,buf,ch,NULL,gambler,TO_ROOM); */

      sprintf(buf, "$n bets %d gold coins and tells $N the coin is under %s.\n\r", goldbid, arg2);

      act(AT_PLAIN, buf, ch, NULL, gambler, TO_CHAR);
      act(AT_PLAIN, buf, ch, NULL, gambler, TO_ROOM);

      /* Compare choosen cup with random choice */
      if ((!strcmp(arg2, "cup1") && result == 1) || (!strcmp(arg2, "cup2") && result == 2) || (!strcmp(arg2, "cup3") && result == 3))
      {
         sprintf(buf,
            "$n uncovers %s and much to $s surprise, $e chose\n\rright.  Sickened by $S loss, $N quickly hands over %d gold coins to\n\r$n.  After the transaction, $N asks if anyone else wants to try it?",
            arg2, REWARD);

         act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
         act(AT_SAY, buf, ch, NULL, gambler, TO_ROOM);

         ch->gold += REWARD; /* Pay winner bet */
         gambler->pIndexData->gold -= REWARD;
         gambler->gold -= REWARD;
      }
      else
      {
         sprintf(buf,
            "$n uncovers the %s cup and smiles gently at $s misfortune.\n\r$N laughs gently and asks if anyone else would like to give it a try?",
            arg2);

         act(AT_SAY, buf, ch, NULL, gambler, TO_CHAR);
         act(AT_SAY, buf, ch, NULL, gambler, TO_ROOM);
      }
      fdarea(gambler, pArea->filename);
   }
   else
   {
      send_to_char("Need to choose cup1, cup2, or cup3.\n\r", ch);
      return;
   }
   return;
}
void do_massgoto(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   int x, y, z;

   argument = one_argument(argument, arg);
   y = atoi(arg);
   z = atoi(argument);

   for (x = y; x < z + 1; x++)
   {
      sprintf(buf, "%d", x);
      do_goto(ch, buf);
   }
   return;
}
