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
 *		     Character saving and loading module		    *
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

/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][8];
CHAR_DATA *quitting_char, *loading_char, *saving_char;

/*
 * Externals
 */
void fwrite_comments(CHAR_DATA * ch, FILE * fp);
void fread_comment(CHAR_DATA * ch, FILE * fp);

// Globals
int file_ver;
int global_eworth;
int global_sworth;
int global_plevel;

/*
 * Array of containers read for proper re-nesting of objects.
 */
static OBJ_DATA *rgObjNest[MAX_NEST];

/*
 * Local functions.
 */
void fwrite_introduction args((CHAR_DATA *ch, FILE *fp));
void fread_introduction args((CHAR_DATA *ch, FILE *fp));
void fwrite_char args((CHAR_DATA * ch, FILE * fp));
void fread_char args((CHAR_DATA * ch, FILE * fp, bool preload));
void write_corpses args((CHAR_DATA * ch, char *name, OBJ_DATA * objrem));
void fwrite_stable args((FILE * fp, STABLE_DATA * stb));
STABLE_DATA *fread_stable args((FILE * fp));
void fwrite_mount args((FILE * fp, CHAR_DATA * mob));
CHAR_DATA *fread_mount args((FILE * fp));

#ifdef WIN32 /* NJG */
UINT timer_code = 0; /* needed to kill the timer */

/* Note: need to include: WINMM.LIB to link to timer functions */
void caught_alarm();
void CALLBACK alarm_handler(UINT IDEvent, /* identifies timer event */
   UINT uReserved, /* not used */
   DWORD dwUser, /* application-defined instance data */
   DWORD dwReserved1, /* not used */
   DWORD dwReserved2) /* not used */
{
   caught_alarm();
}

void kill_timer()
{
   if (timer_code)
      timeKillEvent(timer_code);
   timer_code = 0;
}

#endif



/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char(CHAR_DATA * ch)
{
   char buf[MSL];
   OBJ_DATA *obj;
   int x, y;

   for (x = 0; x < MAX_WEAR; x++)
      for (y = 0; y < MAX_LAYERS; y++)
         save_equipment[x][y] = NULL;

   for (obj = ch->first_carrying; obj; obj = obj->next_content)
   {
      if (obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR)
      {
         if (get_trust(ch) >= obj->level)
         {
            for (x = 0; x < MAX_LAYERS; x++)
            {
               if (!save_equipment[obj->wear_loc][x])
               {
                  save_equipment[obj->wear_loc][x] = obj;
                  break;
               }
               if (x == MAX_LAYERS)
               {
                  sprintf(buf, "%s had on more than %d layers of clothing in one location (%d): %s", ch->name, MAX_LAYERS, obj->wear_loc, obj->name);
                  bug(buf, 0);
               }
            }
         }
         else
         {
            sprintf(buf, "%s had on %s:  ch->level = %d  obj->level = %d", ch->name, obj->name, ch->level, obj->level);
            bug(buf, 0);
         }
         unequip_char(ch, obj);
      }
   }
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char(CHAR_DATA * ch)
{
   int x, y;

   for (x = 0; x < MAX_WEAR; x++)
      for (y = 0; y < MAX_LAYERS; y++)
         if (save_equipment[x][y] != NULL)
         {
            if (quitting_char != ch)
            {
               equip_char(ch, save_equipment[x][y], x);
            }
            save_equipment[x][y] = NULL;
         }
         else
            break;
}

char *fread_lastname_line(FILE *fp)
{
   char *word;
   word = feof(fp) ? "End" : fread_word(fp);
   if (!str_cmp(word, "End"))
      return "S";
   else
      return fread_string(fp);
}

void remove_from_lastname_file(char *lastname, char *firstname)
{
   FILE *fp;
   char strsave[200];
   int x = 0;
   int y;
   char name[13];
   char members[50][13];
   
   sprintf(strsave, "%s%c/%s", LNAME_DIR, tolower(lastname[0]), capitalize(lastname));
   if ((fp = fopen(strsave, "r")) == NULL)
   {
      bug("The lastname of %s is not in the directory, but on player %s", lastname, firstname);
      return;
   }
   else
   {
      for (;;)
      {
         sprintf(name, "%s", fread_lastname_line(fp));
         if (!str_cmp(name, "S"))
            break;
         else
         {
            if (str_cmp(name, firstname))
               sprintf(members[x++], "%s", name);
         }
      }
      if (x == 0) //remove this lastname because this person was the last...
      {
         fclose(fp);
         remove(strsave);
         return;
      }
      y = x;
      fclose(fp);
      fp = fopen(strsave, "w");
      for (x = 0;;x++)
      {
         if (y != x)
         {
            fprintf(fp, "Char %s~\n", members[x]);
         }
         else
         {
            fprintf(fp, "End\n");
            fclose(fp);
            return;
         }
      }
   }
}
//Stores a list of lastnames used for searches mainly and to keep people from picking the same lastnames
void write_lastname_file(char *lastname, char *firstname)
{
   FILE *fp;
   char strsave[200];
   int x = 0;
   int y;
   char name[13];
   char members[50][13];
   
   sprintf(strsave, "%s%c/%s", LNAME_DIR, tolower(lastname[0]), capitalize(lastname));
   if ((fp = fopen(strsave, "r")) == NULL)
   {
      fp = fopen(strsave, "w");
      fprintf(fp, "Char %s~\n", firstname);
      fprintf(fp, "End\n");
      fclose(fp);
      return;
   }
   else
   {
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
      fclose(fp);
      fp = fopen(strsave, "w");
      for (x = 0;;x++)
      {
         if (y != x)
         {
            fprintf(fp, "Char %s~\n", members[x]);
         }
         else
         {
            fprintf(fp, "Char %s~\n", firstname);
            fprintf(fp, "End\n");
            fclose(fp);
            return;
         }
      }
   }
}    

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj(CHAR_DATA * ch)
{
   char strsave[MIL];
   char strback[MIL];
   STABLE_DATA *stb;
   FILE *fp;
   ACCOUNT_NAME *aname; 

   if (!ch)
   {
      bug("Save_char_obj: null ch!", 0);
      return;
   }
   if (IS_NPC(ch))
      return;
      
   //Don't want them to be saved if in auth, etc...
   if (IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED))
      return;

   if (ch->desc && ch->desc->arena)
      return;

   saving_char = ch;

   if (ch->desc && ch->desc->original)
      ch = ch->desc->original;
   global_eworth = player_equipment_worth(ch);
   global_sworth = player_stat_worth(ch);
   global_plevel = get_player_statlevel(ch);
   de_equip_char(ch);
   
   //Need to write the lastname files too....
   if (ch->desc && ch->pcdata->slastname == 0 && ch->desc->connected == CON_PLAYING)
   {
      write_lastname_file(ch->last_name, ch->name);
      ch->pcdata->slastname = 1;
   }

   ch->pcdata->save_time = current_time;
   sprintf(strsave, "%s%c/%s", PLAYER_DIR, tolower(ch->pcdata->filename[0]), capitalize(ch->pcdata->filename));

   /*
    * Auto-backup pfile (can cause lag with high disk access situtations).
    */
   /* Backup of each pfile on save as above can cause lag in high disk
      access situations on big muds like Realms.  Quitbackup saves most
      of that and keeps an adequate backup -- Blodkai, 10/97 */

   if (IS_SET(sysdata.save_flags, SV_BACKUP) || (IS_SET(sysdata.save_flags, SV_QUITBACKUP) && quitting_char == ch))
   {
      sprintf(strback, "%s%c/%s", BACKUP_DIR, tolower(ch->pcdata->filename[0]), capitalize(ch->pcdata->filename));
      rename(strsave, strback);
   }
   /*
    * Save immortal stats, level & vnums for wizlist  -Thoric
    * and do_vnums command
    *
    * Also save the player flags so we the wizlist builder can see
    * who is a guest and who is retired.
    */
   if (ch->level >= LEVEL_IMMORTAL)
   {
      sprintf(strback, "%s%s", GOD_DIR, capitalize(ch->pcdata->filename));

      if ((fp = fopen(strback, "w")) == NULL)
      {
         bug("Save_god_level: fopen", 0);
         perror(strsave);
      }
      else
      {
         fprintf(fp, "Level        %d\n", ch->level);
         fprintf(fp, "Pcflags      %d\n", ch->pcdata->flags);
         if (ch->pcdata->r_range_lo && ch->pcdata->r_range_hi)
            fprintf(fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi);
         if (ch->pcdata->o_range_lo && ch->pcdata->o_range_hi)
            fprintf(fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi);
         if (ch->pcdata->m_range_lo && ch->pcdata->m_range_hi)
            fprintf(fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi);
         fclose(fp);
      }
   }

   if ((fp = fopen(strsave, "w")) == NULL)
   {
      bug("Save_char_obj: fopen", 0);
      perror(strsave);
   }
   else
   {
      fwrite_char(ch, fp);
      if (ch->morph)
         fwrite_morph_data(ch, fp);
      if (ch->first_carrying)
         fwrite_obj(ch, ch->last_carrying, fp, 0, OS_CARRY);
      if (ch->pcdata->first_bankobj)
         fwrite_obj(ch, ch->pcdata->last_bankobj, fp, 0, OS_BANK);
      if (sysdata.save_pets && ch->pcdata->pet)
         fwrite_mobile(fp, ch->pcdata->pet);

      if (ch->pcdata->mount)
         fwrite_mount(fp, ch->pcdata->mount);
      for (stb = ch->pcdata->first_stable; stb; stb = stb->next)
         fwrite_stable(fp, stb);
      if (ch->comments) /* comments */
         fwrite_comments(ch, fp); /* comments */
      if (ch->pcdata->first_introduction)
         fwrite_introduction(ch, fp); //Introductions
      fprintf(fp, "#END\n");
      fclose(fp);
   }

   re_equip_char(ch);
   
   if (ch->desc && ch->desc->account)
   {
      for (aname = ch->desc->account->first_player; aname; aname = aname->next)
      {
         if (!str_cmp(aname->name, ch->name))
            break;
      }
      if (!aname)
      {
         CREATE(aname, ACCOUNT_NAME, 1);
         aname->name = STRALLOC(ch->name);
         LINK(aname, ch->desc->account->first_player, ch->desc->account->last_player, next, prev);   
         save_account(ch->desc, 0);
      }
   }

   quitting_char = NULL;
   saving_char = NULL;
   return;
}

void fwrite_introduction(CHAR_DATA * ch, FILE * fp)
{
   INTRO_DATA *intro;
   
   for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
   {
      fprintf(fp, "#INTRO\n");
      fprintf(fp, "Pid       %d\n", intro->pid);
      fprintf(fp, "Flags     %d\n", intro->flags);
      fprintf(fp, "Value     %d\n", intro->value);
      fprintf(fp, "Lastseen  %d\n", intro->lastseen);
      fprintf(fp, "End\n");
   }
}  
   
/*
 * Write the char.
 */
void fwrite_char(CHAR_DATA * ch, FILE * fp)
{
   ALIAS_DATA *pal;
   AFFECT_DATA *paf;
   int sn, track, i;
   sh_int pos;
   PKILLED_DATA *pkl;
   SKILLTYPE *skill = NULL;
   CHANNEL_HISTORY *chistory;

   fprintf(fp, "#PLAYER\n");

   fprintf(fp, "Version      %d\n", SAVEVERSION);
   fprintf(fp, "Name         %s~\n", ch->name);
   fprintf(fp, "LastName     %s~\n", ch->last_name);
   if (ch->pcdata->autocommand)
      fprintf(fp, "Autocommand  %s~\n", ch->pcdata->autocommand);
   if (ch->pcdata->offeredlname)
      fprintf(fp, "Offeredlname %s~\n", ch->pcdata->offeredlname);
   fprintf(fp, "SLastName    %d\n", ch->pcdata->slastname);
   fprintf(fp, "Pid	     %d\n", ch->pcdata->pid);
   if (ch->description[0] != '\0')
      fprintf(fp, "Description  %s~\n", ch->description);
   fprintf(fp, "Sex          %d\n", ch->sex);
   fprintf(fp, "Race         %d\n", ch->race);
   fprintf(fp, "Languages    %d %d\n", ch->speaks, ch->speaking);
   fprintf(fp, "Level        %d\n", ch->level);
   fprintf(fp, "Played       %d\n", ch->played + (int) (current_time - ch->pcdata->logon));
   fprintf(fp, "Room         %d\n", (ch->in_room == get_room_index(ROOM_VNUM_LIMBO) && ch->was_in_room) ? ch->was_in_room->vnum : ch->in_room->vnum);
   fprintf(fp, "HpManaMove   %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move);
   fprintf(fp, "Limbs        %d %d %d %d\n", ch->con_rarm, ch->con_larm, ch->con_rleg, ch->con_lleg);
   fprintf(fp, "HitCount    %d\n", ch->pcdata->hit_cnt);
   fprintf(fp, "ManaCount    %d\n", ch->pcdata->mana_cnt);
   fprintf(fp, "Righthanded  %d\n", ch->pcdata->righthanded);
   fprintf(fp, "Gold         %d\n", ch->gold);
   fprintf(fp, "Height          %d\n", ch->height);
   fprintf(fp, "Weight          %d\n", ch->weight);
   if (!xIS_EMPTY(ch->act))
      fprintf(fp, "Act          %s\n", print_bitvector(&ch->act));
   if (!xIS_EMPTY(ch->pcdata->talent))
      fprintf(fp, "Talent          %s\n", print_bitvector(&ch->pcdata->talent));
   if (!xIS_EMPTY(ch->affected_by))
      fprintf(fp, "AffectedBy   %s\n", print_bitvector(&ch->affected_by));
   if (!xIS_EMPTY(ch->no_affected_by))
      fprintf(fp, "NoAffectedBy %s\n", print_bitvector(&ch->no_affected_by));
   if (ch->grip != 0)
      fprintf(fp, "Grip         %d\n", ch->grip);

   /*
    * Strip off fighting positions & store as
    * new style (pos>=100 flags new style in character loading)
    */
   pos = ch->position;
   if (pos == POS_BERSERK || pos == POS_AGGRESSIVE || pos == POS_FIGHTING || pos == POS_DEFENSIVE || pos == POS_EVASIVE)
      pos = POS_STANDING;
   pos += 100;
   fprintf(fp, "Position     %d\n", pos);

   fprintf(fp, "Style     %d\n", ch->style);

   fprintf(fp, "Practice     %d\n", ch->practice);
   fprintf(fp, "MoveR	     %d\n", ch->mover);
   fprintf(fp, "Train        %d\n", ch->pcdata->train);
   fprintf(fp, "Resource     %d\n", ch->pcdata->resource);
   fprintf(fp, "Resourcetype %d\n", ch->pcdata->resourcetype);
   fprintf(fp, "TwinkPoints  %d\n", ch->pcdata->twink_points);
   fprintf(fp, "SavingThrows %d %d %d %d %d\n",
      ch->saving_poison_death, ch->saving_wand, ch->saving_para_petri, ch->saving_breath, ch->saving_spell_staff);
   fprintf(fp, "Alignment    %d\n", ch->alignment);
   fprintf(fp, "Favor	       %d\n", ch->pcdata->favor);
   fprintf(fp, "Balance      %d\n", ch->pcdata->balance);
   fprintf(fp, "GPrompt      %d\n", ch->pcdata->gprompt);
   fprintf(fp, "LastInterest %d\n", ch->pcdata->lastinterest);
   fprintf(fp, "LastIntroCheck %d\n", ch->pcdata->lastintrocheck);
   fprintf(fp, "Caste        %d\n", ch->pcdata->caste);
   fprintf(fp, "SPoints      %d\n", ch->pcdata->spoints);
   fprintf(fp, "PowerRanking %d\n", ch->pcdata->power_ranking);
   fprintf(fp, "FLevel       %d\n", ch->pcdata->flevel);
   fprintf(fp, "HomeTown     %d\n", ch->pcdata->hometown);
   fprintf(fp, "KingdomPid   %d\n", ch->pcdata->kingdompid);
   fprintf(fp, "TimeoutLogin %d\n", ch->pcdata->timeout_login);
   fprintf(fp, "TimeoutNotes %d\n", ch->pcdata->timeout_notes);
   fprintf(fp, "TimeoutIdle  %d\n", ch->pcdata->timeout_idle);
   fprintf(fp, "Banksize     %d\n", ch->pcdata->banksize);
   fprintf(fp, "LostCon      %d\n", ch->pcdata->lostcon);
   if (ch->speed > 0)
   fprintf(fp, "Speed        %d\n", ch->speed);
   if (ch->pcdata->town)
      fprintf(fp, "Town         %s~\n", ch->pcdata->town->name);
   if (!xIS_EMPTY(ch->pcdata->portalfnd))
      fprintf(fp, "PortalsFnd   %s\n", print_bitvector(&ch->pcdata->portalfnd));
   if (ch->tone)
      fprintf(fp, "Tone         %s~\n", ch->tone);
   if (ch->movement)
      fprintf(fp, "MoveMessage  %s~\n", ch->movement);
   fprintf(fp, "Incr         %d\n", ch->pcdata->incarnations);
   fprintf(fp, "GtRemort     %d\n", ch->pcdata->gt_remort);
   fprintf(fp, "Tier         %d\n", ch->pcdata->tier);
   fprintf(fp, "Lore         %d\n", ch->pcdata->lore);
   fprintf(fp, "Whonum       %d\n", ch->pcdata->whonum);
   fprintf(fp, "Mapdir       %d\n", ch->pcdata->mapdir);
   fprintf(fp, "Stable       %d\n", ch->pcdata->stable);
   fprintf(fp, "Stablenum    %d\n", ch->pcdata->stablenum);
   fprintf(fp, "Stablecurr   %d\n", ch->pcdata->stablecurr);
   fprintf(fp, "Glory        %d\n", ch->pcdata->quest_curr);
   fprintf(fp, "MGlory       %d\n", ch->pcdata->quest_accum);
   fprintf(fp, "Reward       %d\n", ch->pcdata->reward_curr);
   fprintf(fp, "Reward_Accum %d\n", ch->pcdata->reward_accum);
   fprintf(fp, "Quest_Wins   %d\n", ch->pcdata->quest_wins);
   fprintf(fp, "Quest_Loss   %d\n", ch->pcdata->quest_losses);
   fprintf(fp, "Job          %d\n", ch->pcdata->job);
   fprintf(fp, "Hitroll      %d\n", ch->hitroll);
   fprintf(fp, "Damroll      %d\n", ch->damroll);
   fprintf(fp, "Target       %d\n", ch->pcdata->target);
   fprintf(fp, "Target_Limb  %d\n", ch->pcdata->target_limb);
   fprintf(fp, "Armor        %d\n", ch->armor);
   fprintf(fp, "Skincolor    %d\n", ch->pcdata->skincolor);
   fprintf(fp, "Haircolor    %d\n", ch->pcdata->haircolor);
   fprintf(fp, "Hairlength   %d\n", ch->pcdata->hairlength);
   fprintf(fp, "Hairstyle    %d\n", ch->pcdata->hairstyle);
   fprintf(fp, "Eyecolor     %d\n", ch->pcdata->eyecolor);
   fprintf(fp, "Cheight      %d\n", ch->pcdata->cheight);
   fprintf(fp, "Cweight      %d\n", ch->pcdata->cweight);
   fprintf(fp, "DOffer_time     %d\n", ch->pcdata->duel_offer_time);
   fprintf(fp, "DReceive_time   %d\n", ch->pcdata->duel_receive_time);
   fprintf(fp, "DOffier_name    %d\n", ch->pcdata->duel_offer_name);
   fprintf(fp, "DReceive_name   %d\n", ch->pcdata->dual_receive_name);
   fprintf(fp, "DOffer_prank    %d\n", ch->pcdata->duel_offer_pranking);
   fprintf(fp, "DReceive_prank  %d\n", ch->pcdata->duel_receive_pranking);
   fprintf(fp, "SOffier_name    %d\n", ch->pcdata->spar_offer_name);
   fprintf(fp, "SReceive_name   %d\n", ch->pcdata->spar_receive_name);
   fprintf(fp, "PLevel          %d\n", global_plevel);
   fprintf(fp, "Sworth          %d\n", global_sworth);
   fprintf(fp, "EWorth          %d\n", global_eworth);
   if (ch->wimpy)
      fprintf(fp, "Wimpy        %d\n", ch->wimpy);
   if (ch->deaf)
      fprintf(fp, "Deaf         %d\n", ch->deaf);
   if (ch->pcdata->imc_deaf)
      fprintf(fp, "IMC          %ld\n", ch->pcdata->imc_deaf);
   if (ch->pcdata->imc_allow)
      fprintf(fp, "IMCAllow     %ld\n", ch->pcdata->imc_allow);
   if (ch->pcdata->imc_deny)
      fprintf(fp, "IMCDeny      %ld\n", ch->pcdata->imc_deny);
   fprintf(fp, "ICEListen %s~\n", ch->pcdata->ice_listen);
   if (ch->resistant)
      fprintf(fp, "Resistant    %d\n", ch->resistant);
   if (ch->no_resistant)
      fprintf(fp, "NoResistant  %d\n", ch->no_resistant);
   if (ch->immune)
      fprintf(fp, "Immune       %d\n", ch->immune);
   if (ch->no_immune)
      fprintf(fp, "NoImmune     %d\n", ch->no_immune);
   if (ch->susceptible)
      fprintf(fp, "Susceptible  %d\n", ch->susceptible);
   if (ch->no_susceptible)
      fprintf(fp, "NoSusceptible  %d\n", ch->no_susceptible);
   if (ch->elementb)
      fprintf(fp, "Elements     %d\n", ch->elementb);
   if (ch->pcdata && ch->pcdata->outcast_time)
      fprintf(fp, "Outcast_time %ld\n", ch->pcdata->outcast_time);
   if (ch->pcdata && ch->pcdata->nuisance)
      fprintf(fp, "NuisanceNew %ld %ld %d %d\n", ch->pcdata->nuisance->time,
         ch->pcdata->nuisance->max_time, ch->pcdata->nuisance->flags, ch->pcdata->nuisance->power);
   if (ch->mental_state != -10)
      fprintf(fp, "Mentalstate  %d\n", ch->mental_state);
      
   //Keep these two below together
   fprintf(fp, "Title        %s~\n", ch->pcdata->title); //___IMPORTANT___ Title and Password have to go in order!!!!
   if (ch->pcdata->pwd)
      fprintf(fp, "Password     %s~\n", ch->pcdata->pwd);
   //Keep these two above together
   
   if (ch->pcdata->rank && ch->pcdata->rank[0] != '\0')
      fprintf(fp, "Rank         %s~\n", ch->pcdata->rank);
   if (ch->pcdata->bestowments && ch->pcdata->bestowments[0] != '\0')
      fprintf(fp, "Bestowments  %s~\n", ch->pcdata->bestowments);
   fprintf(fp, "Pretit       %s~\n", ch->pcdata->pretit); /* Xerves 8-2-99 */
   if (ch->pcdata->homepage && ch->pcdata->homepage[0] != '\0')
      fprintf(fp, "Homepage     %s~\n", ch->pcdata->homepage);
   if (ch->pcdata->email && ch->pcdata->email[0] != '\0') /* Samson 4-19-98 */
      fprintf(fp, "Email	     %s~\n", ch->pcdata->email);
   if (ch->pcdata->icq > 0) /* Samson 1-4-99 */
      fprintf(fp, "ICQ          %d\n", ch->pcdata->icq);
   if (ch->pcdata->bio && ch->pcdata->bio[0] != '\0')
      fprintf(fp, "Bio          %s~\n", ch->pcdata->bio);
   if (ch->pcdata->authed_by && ch->pcdata->authed_by[0] != '\0')
      fprintf(fp, "AuthedBy     %s~\n", ch->pcdata->authed_by);
   if (ch->pcdata->came_from && ch->pcdata->came_from[0] != '\0')
      fprintf(fp, "CameFrom     %s~\n", ch->pcdata->came_from);
   if (ch->pcdata->min_snoop)
      fprintf(fp, "Minsnoop     %d\n", ch->pcdata->min_snoop);
   if (ch->pcdata->prompt && *ch->pcdata->prompt)
      fprintf(fp, "Prompt       %s~\n", ch->pcdata->prompt);
   if (ch->pcdata->fprompt && *ch->pcdata->fprompt)
      fprintf(fp, "FPrompt	     %s~\n", ch->pcdata->fprompt);
   if (ch->pcdata->pagerlen != 24)
      fprintf(fp, "Pagerlen     %d\n", ch->pcdata->pagerlen);

   /* Save note board status */
   /* Save number of boards in case that number changes */
   fprintf(fp, "Boards       %d ", MAX_BOARD);
   for (i = 0; i < MAX_BOARD; i++)
      fprintf(fp, "%s %ld ", boards[i].short_name, ch->pcdata->last_note[i]);
   fprintf(fp, "\n");

   for (pal = ch->pcdata->first_alias; pal; pal = pal->next) 
   {
      if (!pal->name || !pal->cmd || !*pal->name || !*pal->cmd)
         continue;
      fprintf(fp, "Alias           %s~ %s~\n", pal->name, pal->cmd);
   }

   /* If ch is ignoring players then store those players */
   {
      IGNORE_DATA *temp;

      for (temp = ch->pcdata->first_ignored; temp; temp = temp->next)
      {
         fprintf(fp, "Ignored      %s~\n", temp->name);
      }
   }

   if (IS_IMMORTAL(ch))
   {
      if (ch->pcdata->bamfin && ch->pcdata->bamfin[0] != '\0')
         fprintf(fp, "Bamfin       %s~\n", ch->pcdata->bamfin);
      if (ch->pcdata->bamfout && ch->pcdata->bamfout[0] != '\0')
         fprintf(fp, "Bamfout      %s~\n", ch->pcdata->bamfout);
      if (ch->trust)
         fprintf(fp, "Trust        %d\n", ch->trust);
      if (ch->pcdata && ch->pcdata->restore_time)
         fprintf(fp, "Restore_time %ld\n", ch->pcdata->restore_time);
      fprintf(fp, "WizInvis     %d\n", ch->pcdata->wizinvis);
      if (ch->pcdata->r_range_lo && ch->pcdata->r_range_hi)
         fprintf(fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi);
      if (ch->pcdata->o_range_lo && ch->pcdata->o_range_hi)
         fprintf(fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi);
      if (ch->pcdata->m_range_lo && ch->pcdata->m_range_hi)
         fprintf(fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi);
   }
   if (ch->pcdata->council)
      fprintf(fp, "Council      %s~\n", ch->pcdata->council_name);
   if (ch->pcdata->keeper)
      fprintf(fp, "Keeper       %d\n", ch->pcdata->keeper);
   if (ch->pcdata->deity_name && ch->pcdata->deity_name[0] != '\0')
      fprintf(fp, "Deity	     %s~\n", ch->pcdata->deity_name);
   if (ch->pcdata->clan_name && ch->pcdata->clan_name[0] != '\0')
      fprintf(fp, "Clan         %s~\n", ch->pcdata->clan_name);
   fprintf(fp, "Flags        %d\n", ch->pcdata->flags);
   if (ch->pcdata->release_date > current_time)
      fprintf(fp, "Helled       %d %s~\n", (int) ch->pcdata->release_date, ch->pcdata->helled_by);
   fprintf(fp, "PKills       %d\n", ch->pcdata->pkills);
   fprintf(fp, "PDeaths      %d\n", ch->pcdata->pdeaths);
   fprintf(fp, "PRanking     %d\n", ch->pcdata->pranking);
   fprintf(fp, "LastPRankingCheck %d\n", ch->pcdata->lastprankingcheck);
   fprintf(fp, "Fame	     %d\n", ch->fame);
   fprintf(fp, "PkPower      %d\n", ch->pcdata->pkpower);
   if (get_timer(ch, TIMER_PKILLED) && (get_timer(ch, TIMER_PKILLED) > 0))
      fprintf(fp, "PTimer       %d\n", get_timer(ch, TIMER_PKILLED));
   fprintf(fp, "MKills       %d\n", ch->pcdata->mkills);
   fprintf(fp, "MDeaths      %d\n", ch->pcdata->mdeaths);
   fprintf(fp, "IllegalPK    %d\n", ch->pcdata->illegal_pk);
   fprintf(fp, "AttrPer      %d %d %d %d %d %d %d\n",
      ch->pcdata->per_str, ch->pcdata->per_int, ch->pcdata->per_wis, ch->pcdata->per_dex, ch->pcdata->per_con,
      ch->pcdata->per_lck, ch->pcdata->per_agi);
   fprintf(fp, "HpMpMvPer    %d %d %d", ch->pcdata->per_hp, ch->pcdata->per_mana, ch->pcdata->per_move);
   fprintf(fp, "AttrPerm     %d %d %d %d %d %d %d %d\n",
      ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck, ch->perm_agi);

   fprintf(fp, "AttrMod      %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
      ch->mod_str, ch->mod_int, ch->mod_wis, ch->mod_dex, ch->mod_con, ch->mod_cha, ch->mod_lck, ch->mod_agi,
      ch->apply_armor, ch->apply_shield, ch->apply_stone, ch->apply_sanctify, ch->apply_tohit, ch->managen, ch->hpgen,
      ch->apply_wmod, ch->apply_manafuse, ch->apply_fasting, ch->apply_manashell, ch->apply_manashield, ch->apply_managuard,
      ch->apply_manaburn, ch->apply_weaponclamp, ch->apply_arrowcatch, ch->apply_bracing, ch->apply_hardening);
   fprintf(fp, "NewResists   %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->apply_res_fire[0], ch->apply_res_water[0], ch->apply_res_air[0], ch->apply_res_earth[0],
            ch->apply_res_energy[0], ch->apply_res_magic[0], ch->apply_res_nonmagic[0], ch->apply_res_blunt[0],
            ch->apply_res_pierce[0], ch->apply_res_slash[0], ch->apply_res_poison[0], ch->apply_res_paralysis[0],
            ch->apply_res_holy[0], ch->apply_res_unholy[0], ch->apply_res_undead[0]);
   fprintf(fp, "NewResists1  %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->apply_res_fire[1], ch->apply_res_water[1], ch->apply_res_air[1], ch->apply_res_earth[1],
            ch->apply_res_energy[1], ch->apply_res_magic[1], ch->apply_res_nonmagic[1], ch->apply_res_blunt[1],
            ch->apply_res_pierce[1], ch->apply_res_slash[1], ch->apply_res_poison[1], ch->apply_res_paralysis[1],
            ch->apply_res_holy[1], ch->apply_res_unholy[1], ch->apply_res_undead[1]);
   fprintf(fp, "NewResists2  %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            ch->apply_res_fire[2], ch->apply_res_water[2], ch->apply_res_air[2], ch->apply_res_earth[2],
            ch->apply_res_energy[2], ch->apply_res_magic[2], ch->apply_res_nonmagic[2], ch->apply_res_blunt[2],
            ch->apply_res_pierce[2], ch->apply_res_slash[2], ch->apply_res_poison[2], ch->apply_res_paralysis[2],
            ch->apply_res_holy[2], ch->apply_res_unholy[2], ch->apply_res_undead[2]);
   fprintf(fp, "Condition    %d %d %d %d\n", ch->pcdata->condition[0], ch->pcdata->condition[1], ch->pcdata->condition[2], ch->pcdata->condition[3]);
   if (ch->desc && ch->desc->host)
      fprintf(fp, "Site         %s\n", ch->desc->host);
   else
      fprintf(fp, "Site         (Link-Dead)\n");

   for (sn = 1; sn < top_sn; sn++)
   {
      if (skill_table[sn]->name && ch->pcdata->learned[sn] > 0)
         switch (skill_table[sn]->type)
         {
            default:
               fprintf(fp, "Skill        %d %d %d '%s'\n", ch->pcdata->spercent[sn], ch->pcdata->learned[sn], ch->pcdata->ranking[sn], skill_table[sn]->name);
               break;
            case SKILL_SPELL:
               fprintf(fp, "Spell        %d %d %d '%s'\n", ch->pcdata->spercent[sn], ch->pcdata->learned[sn], ch->pcdata->ranking[sn], skill_table[sn]->name);
               break;
            case SKILL_TONGUE:
               fprintf(fp, "Tongue       %d %d %d '%s'\n", ch->pcdata->spercent[sn], ch->pcdata->learned[sn], ch->pcdata->ranking[sn], skill_table[sn]->name);
               break;
         }
   }
   for (sn = 0; sn < 5; sn++)
   {
      if (ch->pcdata->forget[sn] > 0)
         fprintf(fp, "Forget          %d '%s'\n", sn, skill_table[ch->pcdata->forget[sn]]->name);
      if (ch->pcdata->nolearn[sn] > 0)
         fprintf(fp, "NoLearn         %d '%s'\n", sn, skill_table[ch->pcdata->nolearn[sn]]->name);
   }
   for (paf = ch->first_affect; paf; paf = paf->next)
   {
      if (paf->type >= 0 && (skill = get_skilltype(paf->type)) == NULL)
         continue;

      if (paf->type >= 0 && paf->type < TYPE_PERSONAL)
      {
         if ((paf->location == APPLY_WEAPONSPELL
         || paf->location == APPLY_WEARSPELL
         || paf->location == APPLY_REMOVESPELL
         || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL) && IS_VALID_SN(paf->modifier))
         {
            fprintf(fp, "AffectDataRecur  '%s' %d '%s' %d %s\n", skill->name, paf->duration, skill_table[paf->modifier]->name, paf->location, print_bitvector(&paf->bitvector));
         }
         else
         {
            fprintf(fp, "AffectData   '%s' %d %d %d %s\n",
               skill->name, paf->duration, paf->modifier, paf->location, print_bitvector(&paf->bitvector));
         }
      }
      else
      {
         if ((paf->location == APPLY_WEAPONSPELL
         || paf->location == APPLY_WEARSPELL
         || paf->location == APPLY_REMOVESPELL
         || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL) && IS_VALID_SN(paf->modifier))
         {
            fprintf(fp, "AffectRecur  %d %d '%s' %d %s\n", paf->type, paf->duration, skill_table[paf->modifier]->name, paf->location, print_bitvector(&paf->bitvector));
         }
         else
         {
            fprintf(fp, "Affect       %d %d %d %d %s\n", paf->type, paf->duration, paf->modifier, paf->location, print_bitvector(&paf->bitvector));
         }
      }
   }

   track = 1;
   for (sn = 0; sn < track; sn++)
   {
      if (ch->pcdata->killed[sn].vnum == 0)
         break;
      fprintf(fp, "Killed       %d %d\n", ch->pcdata->killed[sn].vnum, ch->pcdata->killed[sn].count);
   }
   sn = 0;
   for (pkl = ch->pcdata->first_pkilled; pkl; pkl = pkl->next)
   {
      fprintf(fp, "Pkilled       %s~\n", pkl->name);
      sn++;
      if (sn > ch->pcdata->pkilled)
         bug("fwrite_char: %s pkilled count is lower than the actual pkilled list", ch->name);
   }
   for (chistory = ch->pcdata->first_messagehistory; chistory; chistory = chistory->next)
   {
      fprintf(fp, "CHISTORY\n");
      fprintf(fp, "%d %d %d %d %d %s~ %s~\n", chistory->channel, chistory->pid, chistory->flags, chistory->level, 
                      chistory->kpid, chistory->sender, chistory->text);
   }
   /* Overland Map - Samson 7-31-99 */
   fprintf(fp, "Coordinates	%d %d %d\n", ch->coord->x, ch->coord->y, ch->map);
   if (ch->ship)
      fprintf(fp, "ShipUID      %d\n", ch->ship->uid);
   /* Save color values - Samson 9-29-98 */
   {
      int x;

      fprintf(fp, "MaxColors    %d\n", MAX_COLORS);
      fprintf(fp, "Colors       ");
      for (x = 0; x < MAX_COLORS; x++)
         fprintf(fp, "%d ", ch->pcdata->colors[x]);
      fprintf(fp, "\n");
   }

   fprintf(fp, "End\n\n");
   return;
}



/*
 * Write an object and its contents.
 */
void fwrite_obj(CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest, sh_int os_type)
{
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *paf;
   SKILLTYPE *skill = NULL;
   sh_int wear, wear_loc, x;
   int otype;
   char buf[20];

   if (iNest >= MAX_NEST)
   {
      bug("fwrite_obj: iNest hit MAX_NEST %d", iNest);
      return;
   }

   /*
    * Slick recursion to write lists backwards,
    *   so loading them will load in forwards order.
    */
   if (obj->prev_content && os_type != OS_CORPSE && os_type != OS_KINGDOM && os_type != OS_GROUND && os_type != OS_MARKET)
   {
      if (os_type == OS_BANK)
         otype = OS_BANK;
      else
         otype = OS_CARRY;
      fwrite_obj(ch, obj->prev_content, fp, iNest, otype);
   }

   /*
    * Castrate storage characters.
    * Catch deleted objects                                    -Thoric
    * Do NOT save prototype items!    -Thoric
    */
   if ((obj->item_type == ITEM_KEY && !IS_OBJ_STAT(obj, ITEM_CLANOBJECT) && !IS_OBJ_STAT(obj, ITEM_KINGDOMKEY)) 
   ||  obj_extracted(obj) || IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
      return;
   
   /* Corpse saving. -- Altrag */
   if (os_type == OS_BANK)
      sprintf(buf, "#BANK");
   else if (os_type == OS_MARKET)
      sprintf(buf, "#MARKETOBJ");
   else if (os_type == OS_GROUND)
      sprintf(buf, "#GROUND");
   else if (os_type == OS_CORPSE)
      sprintf(buf, "#CORPSE");
   else
      sprintf(buf, "#OBJECT");
      
   
   fprintf(fp, "%s\n", buf);
   fprintf(fp, "Version       %d\n", SAVEVERSION);
   if (iNest)
      fprintf(fp, "Nest         %d\n", iNest);
   if (obj->count > 1)
      fprintf(fp, "Count        %d\n", obj->count);
   if (!QUICKMATCH(obj->name, obj->pIndexData->name))
      fprintf(fp, "Name         %s~\n", obj->name);
   if (!QUICKMATCH(obj->short_descr, obj->pIndexData->short_descr))
      fprintf(fp, "ShortDescr   %s~\n", obj->short_descr);
   if (obj->trap)
   {
      fprintf(fp, "Trap\n");
      save_trap_file(obj->trap, fp);
   }
   if (!QUICKMATCH(obj->description, obj->pIndexData->description))
      fprintf(fp, "Description  %s~\n", obj->description);
   if (!QUICKMATCH(obj->action_desc, obj->pIndexData->action_desc))
      fprintf(fp, "ActionDesc   %s~\n", obj->action_desc);
   fprintf(fp, "Vnum         %d\n", obj->pIndexData->vnum);
   if ((os_type == OS_CORPSE || os_type == OS_GROUND) && obj->in_room)
      fprintf(fp, "Room         %d\n", obj->in_room->vnum);
   if (!xSAME_BITS(obj->extra_flags, obj->pIndexData->extra_flags))
      fprintf(fp, "ExtraFlags   %s\n", print_bitvector(&obj->extra_flags));
   if (obj->wear_flags != obj->pIndexData->wear_flags)
      fprintf(fp, "WearFlags    %d\n", obj->wear_flags);
   wear_loc = -1;
   for (wear = 0; wear < MAX_WEAR; wear++)
      for (x = 0; x < MAX_LAYERS; x++)
         if (obj == save_equipment[wear][x])
         {
            wear_loc = wear;
            break;
         }
         else if (!save_equipment[wear][x])
            break;
   if (wear_loc != -1)
      fprintf(fp, "WearLoc      %d\n", wear_loc);
   if (obj->item_type != obj->pIndexData->item_type)
      fprintf(fp, "ItemType     %d\n", obj->item_type);
   if (obj->weight != obj->pIndexData->weight)
      fprintf(fp, "Weight       %f\n", obj->weight);
   if (obj->bless_dur != obj->pIndexData->bless_dur)
      fprintf(fp, "BlessDuration	%d\n", obj->bless_dur);
   if (obj->level)
      fprintf(fp, "Level        %d\n", obj->level);
   fprintf(fp, "Sworthrestrict %d\n", obj->sworthrestrict);
   if (obj->imbueslots)
      fprintf(fp, "Imbueslots   %d\n", obj->imbueslots);
   if (obj->first_imbue)
   {
      IMBUE_DATA *imbue;
      for (imbue = obj->first_imbue; imbue; imbue = imbue->next)
      {
         if (imbue->type > 0)
         {
            fprintf(fp, "Gem        %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", imbue->type, imbue->sworth, 
               imbue->lowvalue, imbue->highvalue, imbue->value, imbue->type2, imbue->sworth2, imbue->lowvalue2, 
               imbue->highvalue2, imbue->value2, imbue->type3, imbue->sworth3, imbue->lowvalue3, imbue->highvalue3, 
               imbue->value2, imbue->plevel, imbue->gemnum);
         }
      }
   }
   if (obj->timer)
      fprintf(fp, "Timer        %d\n", obj->timer);
   if (obj->cost != obj->pIndexData->cost)
      fprintf(fp, "Cost         %d\n", obj->cost);
   fprintf(fp, "Coords	%d %d %d\n", obj->map, obj->coord->x, obj->coord->y);
   if (obj->value[0] || obj->value[1] || obj->value[2]
      || obj->value[3] || obj->value[4] || obj->value[5] || obj->value[6] || obj->value[7] || obj->value[8] || obj->value[9] || obj->value[10]
      || obj->value[11])
      fprintf(fp, "Values       %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
         obj->value[0], obj->value[1], obj->value[2],
         obj->value[3], obj->value[4], obj->value[5], obj->value[6], 
         obj->value[7], obj->value[8], obj->value[9], obj->value[10],
         obj->value[11], obj->value[12], obj->value[13]);

   switch (obj->item_type)
   {
      case ITEM_PILL: /* was down there with staff and wand, wrongly - Scryn */
         break;
         
      case ITEM_WEAPON:
         if (IS_VALID_SN(obj->value[4]) && obj->value[4] > 0)
            fprintf(fp, "Spell 4      '%s'\n", skill_table[obj->value[4]]->name);
         break;   
        
      case ITEM_POTION:
      case ITEM_SCROLL:
         if (IS_VALID_SN(obj->value[1]))
            fprintf(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]]->name);

         if (IS_VALID_SN(obj->value[2]))
            fprintf(fp, "Spell 2      '%s'\n", skill_table[obj->value[2]]->name);

         if (IS_VALID_SN(obj->value[3]))
            fprintf(fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name);

         break;
      case ITEM_SALVE:
         break;
      case ITEM_SPELLBOOK:
         if (IS_VALID_SN(obj->value[1]))
            fprintf(fp, "Spell 1      '%s'\n", skill_table[obj->value[1]]->name);
         break;
   }

   for (paf = obj->first_affect; paf; paf = paf->next)
   {
      /*
       * Save extra object affects    -Thoric
       */
      if (paf->type < 0 || paf->type >= top_sn)
      {             
         if ((paf->location == APPLY_WEAPONSPELL
         || paf->location == APPLY_WEARSPELL
         || paf->location == APPLY_REMOVESPELL
         || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL) && IS_VALID_SN(paf->modifier))
         {
            fprintf(fp, "AffectRecur  %d %d '%s' %d %s %d\n", paf->type, paf->duration, skill_table[paf->modifier]->name, paf->location, print_bitvector(&paf->bitvector), paf->gemnum);
         }
         else
         {
            fprintf(fp, "Affect       %d %d %d %d %s %d\n", paf->type, paf->duration, paf->modifier, paf->location, print_bitvector(&paf->bitvector), paf->gemnum);
         }
      }        
      else
      {
         if (paf->type >= 0 && (skill = get_skilltype(paf->type)) == NULL)
            continue;
         if ((paf->location == APPLY_WEAPONSPELL
         || paf->location == APPLY_WEARSPELL
         || paf->location == APPLY_REMOVESPELL
         || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL) && IS_VALID_SN(paf->modifier))
         {
            fprintf(fp, "AffectDataRecur  '%s' %d '%s' %d %s %d\n", skill->name, paf->duration, skill_table[paf->modifier]->name, paf->location, print_bitvector(&paf->bitvector), paf->gemnum);
         }
         else
         {
            fprintf(fp, "AffectData   '%s' %d %d %d %s %d\n",
               skill->name, paf->duration, paf->modifier, paf->location, print_bitvector(&paf->bitvector), paf->gemnum);
         }
      }
   }

   for (ed = obj->first_extradesc; ed; ed = ed->next)
      fprintf(fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description);

   fprintf(fp, "End\n\n");

   if (obj->first_content)
      fwrite_obj(ch, obj->last_content, fp, iNest + 1, os_type == OS_BANK ? OS_BANK : OS_CARRY);

   return;
}

void save_account(DESCRIPTOR_DATA *d, int editing)
{
   FILE *fp;
   char strsave[MIL];
   ACCOUNT_NAME *aname;
   
   sprintf(strsave, "%s%c/%s", ACCOUNT_DIR, tolower(d->account->name[0]), capitalize(d->account->name));
   
   if ((fp = fopen(strsave, "w")) != NULL)  
   {
      fprintf(fp, "Name          %s~\n", d->account->name);
      fprintf(fp, "Passwd        %s~\n", d->account->passwd);
      fprintf(fp, "Email         %s~\n", d->account->email);
      fprintf(fp, "Editing       %d\n", editing);
      fprintf(fp, "Changes       %d\n", d->account->changes);
      fprintf(fp, "LastTimeReset %d\n", d->account->lasttimereset);
      fprintf(fp, "Ban           %d\n", d->account->ban);
      fprintf(fp, "SkipLMenu     %d\n", d->account->skiplmenu);
      fprintf(fp, "NoEmail       %d\n", d->account->noemail);
      for (aname = d->account->first_player; aname; aname = aname->next)
      {
         fprintf(fp, "Player       %s~\n", aname->name);
      }
      fprintf(fp, "QPlayer1      %s~\n", d->account->qplayer1);
      fprintf(fp, "QPlayer2      %s~\n", d->account->qplayer2);
      fprintf(fp, "QPlayer3      %s~\n", d->account->qplayer3);
      fprintf(fp, "QPlayer4      %s~\n", d->account->qplayer4);
      fprintf(fp, "End\n");
      fclose(fp);
   }
   else
   {
      write_to_buffer(d, "There has been a problem saving your account, notify Xerves of this problem.\n\r", 0);
   }
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

//Loads a player account into a descriptor
bool load_account(DESCRIPTOR_DATA * d, char *name, bool preload)
{
   ACCOUNT_DATA *account;
   ACCOUNT_NAME *aname;
   char strsave[MIL];
   char *rname;
   int fndq = 0;
   FILE *fp;
   char *word;
   bool fMatch;
   
   CREATE(account, ACCOUNT_DATA, 1);
   d->account = account;
   
   sprintf(strsave, "%s%c/%s", ACCOUNT_DIR, tolower(name[0]), capitalize(name));
   
   if ((fp = fopen(strsave, "r")) != NULL)
   {
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
               KEY("Ban", account->ban, fread_number(fp));
               break;
            
            case 'C':
               KEY("Changes", account->changes, fread_number(fp));
               break;
               
            case 'L':
               KEY("LastTimeReset", account->lasttimereset, fread_number(fp));
               break;
            
            case 'P':
               if (!str_cmp(word, "Player"))
               {
                  fMatch = TRUE;
                  rname = fread_string(fp);
                  CREATE(aname, ACCOUNT_NAME, 1);
                  aname->name = STRALLOC(rname);
                  LINK(aname, account->first_player, account->last_player, next, prev);
               }                  
               KEY("Passwd", account->passwd, fread_string(fp));
            break;
            
            case 'Q':
               if (!str_cmp(word, "QPlayer1") || !str_cmp(word, "QPlayer2") || !str_cmp(word, "QPlayer3") || 
                   !str_cmp(word, "QPlayer4"))
               {
                  fndq = 1;
               }
               KEY("QPlayer1", account->qplayer1, fread_string(fp));
               KEY("QPlayer2", account->qplayer2, fread_string(fp));
               KEY("QPlayer3", account->qplayer3, fread_string(fp));
               KEY("QPlayer4", account->qplayer4, fread_string(fp));
               break;
                           
            case 'N':
               KEY("Name", account->name, fread_string(fp));
               KEY("NoEmail", account->noemail, fread_number(fp));
               break;    
               
            case 'S':
               KEY("SkipLMenu", account->skiplmenu, fread_number(fp));
               break;        

            case 'E':
               KEY("Email", account->email, fread_string(fp));
               KEY("Editing", account->editing, fread_number(fp));
               if (!str_cmp(word, "End"))
               {
                  if (fndq == 0)
                  {
                     account->qplayer1 = STRALLOC("");
                     account->qplayer2 = STRALLOC("");
                     account->qplayer3 = STRALLOC("");
                     account->qplayer4 = STRALLOC("");
                  }
                  fclose(fp);
                  return TRUE;
               }
         }
      }
   }
   return FALSE;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj(DESCRIPTOR_DATA * d, char *name, bool preload)
{
   char strsave[MIL];
   CHAR_DATA *ch;
   FILE *fp;
   bool found;
   struct stat fst;
   int i, x;
   extern FILE *fpArea;
   extern char strArea[MIL];
   char buf[MIL];

   CREATE(ch, CHAR_DATA, 1);
   for (x = 0; x < MAX_WEAR; x++)
      for (i = 0; i < MAX_LAYERS; i++)
         save_equipment[x][i] = NULL;
   clear_char(ch);
   loading_char = ch;

   CREATE(ch->pcdata, PC_DATA, 1);
   d->character = ch;
   ch->desc = d;
   ch->pcdata->filename = STRALLOC(name);
   ch->name = NULL;
   ch->act = multimeb(PLR_BLANK, PLR_COMBINE, PLR_PROMPT, -1);
   ch->pcdata->first_stable = NULL;
   ch->pcdata->last_stable = NULL;
   ch->pcdata->first_pkilled = NULL;
   ch->pcdata->last_pkilled = NULL;
   ch->pcdata->first_introduction = ch->pcdata->last_introduction = NULL;
   ch->pcdata->pkilled = 0;
   ch->perm_str = 13;
   ch->perm_int = 13;
   ch->perm_wis = 13;
   ch->perm_dex = 13;
   ch->perm_con = 13;
   ch->perm_cha = 13;
   ch->perm_lck = 13;
   ch->perm_agi = 15;
   ch->pcdata->duel_offer_time = 0;
   ch->pcdata->duel_receive_time = 0;
   ch->pcdata->duel_offer_name = -1;
   ch->pcdata->dual_receive_name = -1;
   ch->pcdata->duel_offer_pranking = 0; 
   ch->pcdata->duel_receive_pranking = 0;
   ch->pcdata->spar_offer_name = -1;
   ch->pcdata->spar_receive_name = -1;
   ch->speed = 3;
   ch->desc->speed = 3;
   ch->pcdata->per_str = 3000;
   ch->pcdata->per_int = 3000;
   ch->pcdata->per_wis = 3000;
   ch->pcdata->per_dex = 3000;
   ch->pcdata->per_con = 3000;
   ch->pcdata->per_lck = 3000;
   ch->pcdata->per_agi = 300;
   ch->pcdata->per_hp =  300;
   ch->pcdata->per_mana = 300;
   ch->pcdata->per_move = 300;
   ch->apply_res_fire[0] = 100;
   ch->apply_res_water[0] = 100;
   ch->apply_res_air[0] = 100;
   ch->apply_res_earth[0] = 100;
   ch->pcdata->twink_points = 0;
   ch->pcdata->power_ranking = 20;
   ch->pcdata->lastprankingcheck = 0;
   ch->apply_res_energy[0] = 100;
   ch->apply_res_magic[0] = 100;
   ch->apply_res_nonmagic[0] = 100;
   ch->apply_res_blunt[0] = 100;
   ch->apply_res_pierce[0] = 100;
   ch->apply_res_slash[0] = 100;
   ch->apply_res_poison[0] = 100;
   ch->apply_res_paralysis[0] = 100;
   ch->apply_res_holy[0] = 100;
   ch->apply_res_unholy[0] = 100;
   ch->apply_res_undead[0] = 100;
   ch->con_rarm = 1000;
   ch->con_larm = 1000;
   ch->move = 1000;
   ch->max_move = 1000;
   ch->mover = 30;
   ch->con_rleg = 1000;
   ch->con_lleg = 1000;
   ch->pcdata->banksize = 100;
   ch->pcdata->righthanded = 1;
   SET_BIT(ch->grip, GRIP_BASH);
   ch->no_resistant = 0;
   ch->no_susceptible = 0;
   ch->no_immune = 0;
   ch->was_in_room = NULL;
   ch->elementb = 0;
   xCLEAR_BITS(ch->no_affected_by);
   xCLEAR_BITS(ch->pcdata->portalfnd);
   ch->pcdata->condition[COND_THIRST] = 48;
   ch->pcdata->condition[COND_FULL] = 48;
   ch->pcdata->condition[COND_BLOODTHIRST] = 10;
   ch->pcdata->nuisance = NULL;
   ch->pcdata->wizinvis = 0;
   ch->pcdata->balance = 0;
   ch->pcdata->gprompt = 0;
   ch->pcdata->caste = 2;
   ch->pcdata->flevel = 1;
   ch->pcdata->lore = 0;
   ch->pcdata->town = NULL;
   ch->pcdata->target = GRIP_BASH;
   ch->pcdata->target_limb = LM_BODY;

   ch->pcdata->hometown = 0;
   ch->pcdata->kingdompid = 0;
   ch->pcdata->incarnations = 0;
   ch->pcdata->gt_remort = 0;
   ch->pcdata->tier = 1;
   ch->pcdata->pranking = 0;
   ch->pcdata->pkpower = 0;
   ch->pcdata->train = 0;
   ch->pcdata->lastintrocheck = 0;
   ch->pcdata->skincolor = 0;
   ch->pcdata->haircolor = 0;
   ch->pcdata->hairlength = 0;
   ch->pcdata->hairstyle = 0;
   ch->pcdata->eyecolor = 0;
   ch->pcdata->cheight = 0;
   ch->pcdata->cweight = 0;
   ch->pcdata->lostcon = 0;
   ch->pcdata->quest_wins = 0;
   ch->pcdata->quest_losses = 0;
   ch->pcdata->timeout_login = sysdata.timeout_login;
   ch->pcdata->timeout_notes = sysdata.timeout_notes;
   ch->pcdata->timeout_idle = sysdata.timeout_idle; 
   ch->pcdata->whonum = 1;
   ch->pcdata->stable = 0;
   ch->pcdata->stablenum = 0;
   ch->pcdata->stablecurr = 0;
   ch->pcdata->resource = 0;
   ch->pcdata->resourcetype = 0;
   ch->pcdata->job = 0;
   ch->pcdata->lastinterest = 0;
   ch->pcdata->authwait = -1;
   /* Also needed for Remort */
   ch->mental_state = -10;
   ch->mobinvis = 0;
   //spherepoints & grouppoints are used to keep track of groups/spheres for skill improvement/degeneration
   for (i = 1; i <= MAX_SPHERE; i++)
      ch->pcdata->spherepoints[i] = -1;
   for (i = 1; i <= MAX_GROUP+5; i++)
      ch->pcdata->grouppoints[i] = -1;
   for (i = 0; i < MAX_SKILL; i++)
      ch->pcdata->learned[i] = 0;
   for (i = 0; i < 5; i++)
   {
      ch->pcdata->forget[i] = 0;
      ch->pcdata->nolearn[i] = 0;
   }
   for (i = 0; i < MAX_GROUP + 2; i++)
      ch->pcdata->spellgroups[i] = 0;
   for (i = 0; i < MAX_GROUP + 2; i++)
      ch->pcdata->spellpoints[i] = 0;
   ch->pcdata->mapdir = -1;
   ch->pcdata->release_date = 0;
   ch->pcdata->helled_by = NULL;
   ch->saving_poison_death = 0;
   ch->saving_wand = 0;
   ch->saving_para_petri = 0;
   ch->pcdata->spoints = 0;
   ch->saving_breath = 0;
   ch->saving_spell_staff = 0;
   ch->pcdata->logon = current_time;
   ch->style = STYLE_FIGHTING;
   ch->comments = NULL; /* comments */
   ch->pcdata->pagerlen = 24;
   ch->pcdata->first_ignored = NULL; /* Ignore list */
   ch->pcdata->last_ignored = NULL;
   ch->pcdata->aimtarget = NULL; //Target in battle to aim at in Wilderness.
   ch->pcdata->tell_history = NULL; /* imm only lasttell cmnd */
   ch->pcdata->lt_index = 0; /* last tell index */
   ch->morph = NULL;
   ch->pcdata->secedit = 18; /* Initialize Map OLC sector - Samson 8-1-99 */
   ch->map = -1; /* Initialize map they're on - Samson 8-3-99 */
   CREATE(ch->coord, COORD_DATA, 1);
   ch->coord->x = -1;
   ch->coord->y = -1;
   ch->fcounter = 0;
   ch->pcdata->email = NULL; /* Initialize email address - Samson 1-4-99 */
   ch->pcdata->homepage = NULL; /* Initialize homepage - Samson 1-4-99 */
   ch->pcdata->icq = 0; /* Initalize icq# - Samson 1-4-99 */
   /* Set up defaults for imc stuff */
   ch->pcdata->imc_deaf = 0;
   ch->pcdata->imc_deny = 0;
   ch->pcdata->imc_allow = 0;
   ch->pcdata->ice_listen = NULL;



   found = FALSE;
   sprintf(strsave, "%s%c/%s", PLAYER_DIR, tolower(name[0]), capitalize(name));
   if (stat(strsave, &fst) != -1)
   {
      if (fst.st_size == 0)
      {
         sprintf(strsave, "%s%c/%s", BACKUP_DIR, tolower(name[0]), capitalize(name));
         send_to_char("Restoring your backup player file...", ch);
      }
      else
      {
         sprintf(buf, "%s player data for: %s (%dK)", preload ? "Preloading" : "Loading", ch->pcdata->filename, (int) fst.st_size / 1024);
         log_string_plus(buf, LOG_COMM, LEVEL_HI_IMM);
      }
   }
   /* else no player file */

   if ((fp = fopen(strsave, "r")) != NULL)
   {
      int iNest;

      for (iNest = 0; iNest < MAX_NEST; iNest++)
         rgObjNest[iNest] = NULL;

      found = TRUE;
      /* Cheat so that bug will show line #'s -- Altrag */
      fpArea = fp;
      strcpy(strArea, strsave);
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
            bug("Load_char_obj: # not found.", 0);
            bug(name, 0);
            break;
         }

         word = fread_word(fp);
         if (!strcmp(word, "PLAYER"))
         {
            fread_char(ch, fp, preload);
            if (preload)
               break;
         }
         else if (!strcmp(word, "OBJECT")) /* Objects */
            fread_obj(ch, fp, OS_CARRY);
         else if (!strcmp(word, "BANK")) 
            fread_obj(ch, fp, OS_BANK);
         else if (!strcmp(word, "MorphData")) /* Morphs */
            fread_morph_data(ch, fp);
         else if (!strcmp(word, "COMMENT"))
            fread_comment(ch, fp); /* Comments */
         else if (!strcmp(word, "INTRO"))
            fread_introduction(ch, fp);
         else if (!strcmp(word, "STABLE"))
         {
            STABLE_DATA *stb;

            stb = fread_stable(fp);
            LINK(stb, ch->pcdata->first_stable, ch->pcdata->last_stable, next, prev);
         }
         else if (!strcmp(word, "MOBILE"))
         {
            CHAR_DATA *mob;

            mob = fread_mobile(fp);
            ch->pcdata->pet = mob;
            mob->master = ch;
            if (IS_ONMAP_FLAG(ch))
            {
               mob->coord->x = ch->coord->x;
               mob->coord->y = ch->coord->y;
               mob->map = ch->map;
               SET_ONMAP_FLAG(mob);
            }
            if (ch->in_room->vnum == mob->in_room->vnum)
            {
               if (xIS_SET(mob->act, ACT_MOUNTABLE))
               {
                  ch->mount = mob;
                  xSET_BIT(mob->act, ACT_MOUNTED);
               }
            }
            else
            {
               ch->mount = NULL;
               xREMOVE_BIT(mob->act, ACT_MOUNTED);
            }
            xSET_BIT(mob->affected_by, AFF_CHARM);
         }
         else if (!strcmp(word, "MOUNT"))
         {
            CHAR_DATA *mount;

            mount = fread_mount(fp);
            ch->pcdata->mount = mount;
            mount->master = ch;
            REMOVE_ONMAP_FLAG(mount);
            if (mount->in_room->vnum != OVERLAND_SOLAN)
               REMOVE_ONMAP_FLAG(mount);
            else
               SET_ONMAP_FLAG(mount);

            if (ch->in_room->vnum == OVERLAND_SOLAN && mount->in_room->vnum == OVERLAND_SOLAN)
            {
               mount->coord->x = ch->coord->x;
               mount->coord->y = ch->coord->y;
               mount->map = ch->map;
               SET_ONMAP_FLAG(mount);
            }
            if (mount->in_room->vnum == OVERLAND_SOLAN && ch->in_room->vnum != OVERLAND_SOLAN)
            {
               char_from_room(mount);
               char_to_room(mount, get_room_index(ROOM_VNUM_TEMPLE));
               mount->coord->x = -1;
               mount->coord->y = -1;
               mount->map = -1;
               REMOVE_ONMAP_FLAG(mount);
            }

            if (ch->in_room->vnum == mount->in_room->vnum)
            {
               if (xIS_SET(mount->act, ACT_MOUNTABLE))
               {
                  ch->mount = mount;
                  xSET_BIT(mount->act, ACT_MOUNTED);
               }
            }
            else
            {
               ch->mount = NULL;
               xREMOVE_BIT(mount->act, ACT_MOUNTED);
            }
            if (ch->ship)
            {
               mount->ship = ch->ship;
               mount->coord->x = ch->ship->x;
               mount->coord->y = ch->ship->y;
               LINK(mount, ch->ship->first_char, ch->ship->last_char, next_ship, prev_ship);
            }
            xSET_BIT(mount->affected_by, AFF_CHARM);
            mount->m4 = ch->pcdata->hometown;
         }
         else if (!strcmp(word, "END")) /* Done  */
            break;
         else
         {
            bug("Load_char_obj: bad section.", 0);
            bug(name, 0);
            break;
         }
      }
      fclose(fp);
      fpArea = NULL;
      strcpy(strArea, "$");
   }

   if (ch->pcdata->ice_listen == NULL)
      ch->pcdata->ice_listen = str_dup("");

   if (!found)
   {
      ch->name = STRALLOC(name);
      ch->short_descr = STRALLOC("");
      ch->long_descr = STRALLOC("");
      ch->description = STRALLOC("");
      ch->editor = NULL;
      ch->pcdata->clan_name = STRALLOC("");
      ch->pcdata->clan = NULL;
      ch->pcdata->council_name = STRALLOC("");
      ch->pcdata->council = NULL;
      ch->pcdata->deity_name = STRALLOC("");
      ch->pcdata->deity = NULL;
      ch->pcdata->first_alias = NULL;
      ch->pcdata->last_alias = NULL;
      ch->pcdata->pet = NULL;
      ch->pcdata->mount = NULL;
      /* every characters starts at default board from login.. this board
         should be read_level == 0 !
       */
      ch->pcdata->board = &boards[DEFAULT_BOARD];
      ch->pcdata->bamfin = str_dup("");
      ch->pcdata->bamfout = str_dup("");
      ch->pcdata->rank = str_dup("");
      ch->pcdata->bestowments = str_dup("");
      ch->pcdata->title = STRALLOC("");
      ch->pcdata->pretit = str_dup(""); /* Xerves 8-2-99 */
      ch->pcdata->homepage = str_dup("");
      ch->pcdata->email = str_dup(""); /* Samson 4-19-98 */
      ch->pcdata->icq = 0; /* Samson 1-4-99 */
      ch->pcdata->bio = STRALLOC("");
      ch->pcdata->authed_by = STRALLOC("");
      ch->pcdata->came_from = STRALLOC("");
      ch->pcdata->prompt = STRALLOC("");
      ch->pcdata->fprompt = STRALLOC("");
      ch->tone = STRALLOC("");
      ch->movement = STRALLOC("");
      ch->pcdata->r_range_lo = 0;
      ch->pcdata->r_range_hi = 0;
      ch->pcdata->m_range_lo = 0;
      ch->pcdata->m_range_hi = 0;
      ch->pcdata->o_range_lo = 0;
      ch->pcdata->o_range_hi = 0;
      ch->pcdata->wizinvis = 0;
   }
   else
   {
      if (!ch->name)
         ch->name = STRALLOC(name);
      if (!ch->pcdata->clan_name)
      {
         ch->pcdata->clan_name = STRALLOC("");
         ch->pcdata->clan = NULL;
      }
      if (!ch->pcdata->council_name)
      {
         ch->pcdata->council_name = STRALLOC("");
         ch->pcdata->council = NULL;
      }
      if (!ch->pcdata->deity_name)
      {
         ch->pcdata->deity_name = STRALLOC("");
         ch->pcdata->deity = NULL;
      }
      if (!ch->pcdata->bio)
         ch->pcdata->bio = STRALLOC("");
         
      if (!ch->pcdata->came_from)
         ch->pcdata->came_from = STRALLOC("");

      if (!ch->pcdata->pretit)
         ch->pcdata->pretit = str_dup(""); /* Xerves 8-2-99 */

      if (!ch->pcdata->authed_by)
         ch->pcdata->authed_by = STRALLOC("");

      if (xIS_SET(ch->act, PLR_FLEE))
         xREMOVE_BIT(ch->act, PLR_FLEE);

      if (IS_IMMORTAL(ch))
      {
         if (ch->pcdata->wizinvis < 2)
            ch->pcdata->wizinvis = ch->level;
         assign_area(ch);
      }
      if (file_ver > 1)
      {
         for (i = 0; i < MAX_WEAR; i++)
         {
            for (x = 0; x < MAX_LAYERS; x++)
            {
               if (save_equipment[i][x])
               {
                  equip_char(ch, save_equipment[i][x], i);
                  save_equipment[i][x] = NULL;
               }
               else
                  break;
            }
         }
      }

      /* Must be done *AFTER* eq is worn because of wis/int modifiers */
/*	if ( !IS_IMMORTAL(ch) )
		REMOVE_BIT(ch->speaks, LANG_COMMON | race_table[ch->race]->language);
	if ( countlangs(ch->speaks) < (ch->level / 10) && !IS_IMMORTAL(ch) )
	{
		int prct = 5 + (get_curr_int(ch) / 6) + (get_curr_wis(ch) / 7);

		do
		{
			int iLang;
			int lang = 1;
			int need = (ch->level / 10) - countlangs(ch->speaks);
			int prac = 2 - (get_curr_cha(ch) / 17) * (70 / prct) * need;
				
			if ( ch->practice >= prac )
				break;
			
			for ( iLang = 1; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
				if ( number_range( 1, iLang ) == 1 )
					lang = iLang;
			if ( (iLang = bsearch_skill_exact( lang_names[lang], gsn_first_tongue, gsn_top_sn-1  )) < 0 )
				continue;
			if ( ch->pcdata->learned[iLang] > 0 )
				continue;
			SET_BIT(ch->speaks, lang_array[lang]);
			ch->pcdata->learned[iLang] = 70;
			ch->speaks &= VALID_LANGS;
			REMOVE_BIT(ch->speaks,
					   LANG_COMMON | race_table[ch->race]->language);
		}
	}*/
   }

   /* Rebuild affected_by and RIS to catch errors - FB */
   update_aris(ch);
   loading_char = NULL;
   return found;
}

void fread_arena_affect(CHAR_DATA * ch, FILE * fp)
{
   char *word;

   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);

      switch (UPPER(word[0]))
      {
         case '*':
            fread_to_eol(fp);
            break;

         case 'A':
            if (!strcmp(word, "AffectData"))
            {
               AFFECT_DATA *paf;
               int sn;
               char *sname = fread_word(fp);

               CREATE(paf, AFFECT_DATA, 1);

               if ((sn = skill_lookup(sname)) < 0)
               {
                  if ((sn = herb_lookup(sname)) < 0)
                     bug("Fread_arena_affect: unknown skill.", 0);
                  else
                     sn += TYPE_HERB;
               }
               paf->type = sn;
               paf->duration = fread_number(fp);
               paf->modifier = fread_number(fp);
               paf->location = fread_number(fp);
               if (paf->location == APPLY_WEAPONSPELL
                  || paf->location == APPLY_WEARSPELL
                  || paf->location == APPLY_REMOVESPELL || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL)
                  paf->modifier = slot_lookup(paf->modifier);
               paf->bitvector = fread_bitvector(fp);
               LINK(paf, ch->first_affect, ch->last_affect, next, prev);
               break;
            }

         case 'E':
            if (!strcmp(word, "End"))
               return;
      }
   }
}



void read_obj_arena(CHAR_DATA * ch, char *argument)
{
   FILE *pfile;
   char pload[MIL];
   struct stat fst;
   int x, i;

   for (x = 0; x < MAX_WEAR; x++)
      for (i = 0; i < MAX_LAYERS; i++)
         save_equipment[x][i] = NULL;

   pload[0] = '\0';
   sprintf(pload, "%s%c/%s", PLAYER_DIR, tolower(argument[0]), capitalize(argument));

   if (stat(pload, &fst) == -1)
   {
      bug("%s's file cannot be found, not good.", ch->name);
      return;
   }

   if (stat(pload, &fst) != -1)
   {
      if ((pfile = fopen(pload, "r")) != NULL)
      {
         for (;;)
         {
            char letter;
            char *word;

            letter = fread_letter(pfile);

            if (letter != '#')
               continue;

            word = fread_word(pfile);

            if (!str_cmp(word, "PLAYER"))
               fread_arena_affect(ch, pfile);

            else if (!str_cmp(word, "OBJECT"))
               fread_obj(ch, pfile, OS_CARRY);

            else if (!str_cmp(word, "END"))
               break;
         }
         fclose(pfile);
      }
   }
   for (i = 0; i < MAX_WEAR; i++)
   {
      for (x = 0; x < MAX_LAYERS; x++)
      {
         if (save_equipment[i][x])
         {
            equip_char(ch, save_equipment[i][x], i);
            save_equipment[i][x] = NULL;
         }
         else
            break;
      }
   }
   return;
}

/*
 * Read in a char.
 */

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

void fread_char(CHAR_DATA * ch, FILE * fp, bool preload)
{
   char buf[MSL];
   char *line;
   char *word;
   int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15, x16, x17, x18, x19, x20, x21, x22, x23, x24, x25;
   int max_colors = 0; /* Color code */
   sh_int killcnt;
   bool fMatch;
   CHANNEL_HISTORY *chistory;

   file_ver = 0;
   killcnt = 0;
   /* Setup color values in case player has none set - Samson */
   memcpy(&ch->pcdata->colors, &default_set, sizeof(default_set));
   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;
      
      if (!str_cmp(word, "EWorth") || !str_cmp(word, "Sworth") || !str_cmp(word, "PLevel"))
      {
         fread_to_eol(fp);
         fMatch = TRUE;
         continue;
      }

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'A':
            KEY("Act", ch->act, fread_bitvector(fp));
            KEY("AffectedBy", ch->affected_by, fread_bitvector(fp));
            KEY("Alignment", ch->alignment, fread_number(fp));
            KEY("Armor", ch->armor, fread_number(fp));

            if (!strcmp(word, "Affect") || !strcmp(word, "AffectData") || !strcmp(word, "AffectRecur") || !strcmp(word, "AffectDataRecur"))
            {
               AFFECT_DATA *paf;
               int recur = 0;
               
               if (!str_cmp(word, "AffectRecur") || !str_cmp(word, "AffectDataRecur"))
                  recur = 1;

               if (preload)
               {
                  fMatch = TRUE;
                  fread_to_eol(fp);
                  break;
               }
               CREATE(paf, AFFECT_DATA, 1);
               if (!strcmp(word, "Affect") || !strcmp(word, "AffectRecur"))
               {
                  paf->type = fread_number(fp);
               }
               else
               {
                  int sn;
                  char *sname = fread_word(fp);

                  if ((sn = skill_lookup(sname)) < 0)
                  {
                     if ((sn = herb_lookup(sname)) < 0)
                        bug("Fread_char: unknown skill.", 0);
                     else
                        sn += TYPE_HERB;
                  }
                  paf->type = sn;
               }

               paf->duration = fread_number(fp);
               if (recur)
               {
                  paf->modifier = skill_lookup(fread_word(fp));
               }
               else
               {
                  paf->modifier = fread_number(fp);
               }
               paf->location = fread_number(fp);                   
               paf->bitvector = fread_bitvector(fp);
               LINK(paf, ch->first_affect, ch->last_affect, next, prev);
               fMatch = TRUE;
               break;
            }

            if (!strcmp(word, "AttrMod"))
            {
               int x26;
               line = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 13;
               x8 = 10;
               x9=x10=x11=x12=x13=x14=x15=x16=x17=x18=x19=x20=x21=x22=x23=x24=x25=x26=0;
               sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                             &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15, 
                             &x16, &x17, &x18, &x19, &x20, &x21, &x22, &x23, &x24, &x25, &x26);
               ch->mod_str = x1;
               ch->mod_int = x2;
               ch->mod_wis = x3;
               ch->mod_dex = x4;
               ch->mod_con = x5;
               ch->mod_cha = x6;
               ch->mod_lck = x7;
               ch->mod_agi = x8;
               ch->apply_armor = x9;
               ch->apply_shield = x10;
               ch->apply_stone = x11;
               ch->apply_sanctify = x12;
               ch->apply_tohit = x13;
               ch->managen = x14;
               ch->hpgen = x15;
               ch->apply_wmod = x16;
               ch->apply_manafuse = x17;
               ch->apply_fasting = x18;              
               ch->apply_manashell = x19;
               ch->apply_manashield = x20;
               ch->apply_managuard = x21;
               ch->apply_manaburn = x22;
               ch->apply_weaponclamp = x23;
               ch->apply_arrowcatch = x24;
               ch->apply_bracing = x25;
               ch->apply_hardening = x26;
               if (!x7)
                  ch->mod_lck = 0;
               fMatch = TRUE;
               break;
            }
            
            if (!strcmp(word, "AttrPer"))
            {
               line = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
               sscanf(line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7);
               ch->pcdata->per_str = x1;
               ch->pcdata->per_int = x2;
               ch->pcdata->per_wis = x3;
               ch->pcdata->per_dex = x4;
               ch->pcdata->per_con = x5;
               ch->pcdata->per_lck = x6;
               ch->pcdata->per_agi = x7;
               fMatch = TRUE;
               break;
            }
            if (!str_cmp(word, "Alias"))
            {
               ALIAS_DATA *pal;

               if (preload)
               {
                  fMatch = TRUE;
                  fread_to_eol(fp);
                  break;
               }
               CREATE(pal, ALIAS_DATA, 1);

               pal->name = fread_string_nohash(fp);
               pal->cmd = fread_string_nohash(fp);
               LINK(pal, ch->pcdata->first_alias, ch->pcdata->last_alias, next, prev);
               fMatch = TRUE;
               break;
            }


            if (!strcmp(word, "AttrPerm"))
            {
               line = fread_line(fp);
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = 0;
               sscanf(line, "%d %d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8);
               ch->perm_str = x1;
               ch->perm_int = x2;
               ch->perm_wis = x3;
               ch->perm_dex = x4;
               ch->perm_con = x5;
               ch->perm_cha = x6;
               ch->perm_lck = x7;
               ch->perm_agi = x8;
               if (!x7 || x7 == 0)
                  ch->perm_lck = 13;
               fMatch = TRUE;
               break;
            }
            KEY("AuthedBy", ch->pcdata->authed_by, fread_string(fp));
            KEY("Autocommand", ch->pcdata->autocommand, fread_string(fp));
            break;

         case 'B':
            KEY("Balance", ch->pcdata->balance, fread_number(fp));
            KEY("Banksize", ch->pcdata->banksize, fread_number(fp));
            KEY("Bamfin", ch->pcdata->bamfin, fread_string_nohash(fp));
            KEY("Bamfout", ch->pcdata->bamfout, fread_string_nohash(fp));
            /* Read in board status */
            if (!str_cmp(word, "Boards"))
            {
               int i, num = fread_number(fp); /* number of boards saved */
               char *boardname;

               for (; num; num--) /* for each of the board saved */
               {
                  boardname = fread_word(fp);
                  i = board_lookup(boardname); /* find board number */

                  if (i == BOARD_NOTFOUND) /* Does board still exist ? */
                  {
                     sprintf(buf, "fread_char: %s had unknown board name: %s. Skipped.", ch->name, boardname);
                     log_string(buf);
                     fread_number(fp); /* read last_note and skip info */
                  }
                  else /* Save it */
                     ch->pcdata->last_note[i] = fread_number(fp);
               } /* for */

               fMatch = TRUE;
            } /* Boards */
            KEY("Bestowments", ch->pcdata->bestowments, fread_string_nohash(fp));
            KEY("Bio", ch->pcdata->bio, fread_string(fp));
            break;

         case 'C':
            KEY("CameFrom", ch->pcdata->came_from, fread_string(fp));
            KEY("Cheight", ch->pcdata->cheight, fread_number(fp));
            if (!str_cmp(word, "CHISTORY"))
            {
               CREATE(chistory, CHANNEL_HISTORY, 1);
               LINK(chistory, ch->pcdata->first_messagehistory, ch->pcdata->last_messagehistory, next, prev);
               chistory->channel = fread_number(fp);
               chistory->pid = fread_number(fp);
               chistory->flags = fread_number(fp);
               chistory->level = fread_number(fp);
               chistory->kpid = fread_number(fp);
               chistory->sender = fread_string(fp);
               chistory->text = fread_string(fp);
               fMatch = TRUE;
               break;
            }
                  
            KEY("Cweight", ch->pcdata->cweight, fread_number(fp));
            KEY("Caste", ch->pcdata->caste, fread_number(fp));
            if (!strcmp(word, "Clan"))
            {
               ch->pcdata->clan_name = fread_string(fp);

               if (!preload && ch->pcdata->clan_name[0] != '\0' && (ch->pcdata->clan = get_clan(ch->pcdata->clan_name)) == NULL)
               {
                  sprintf(buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
                     ch->pcdata->clan_name);
                  send_to_char(buf, ch);
                  STRFREE(ch->pcdata->clan_name);
                  ch->pcdata->clan_name = STRALLOC("");
               }
               fMatch = TRUE;
               break;
            }
            if (!strcmp(word, "Condition"))
            {
               line = fread_line(fp);
               sscanf(line, "%d %d %d %d", &x1, &x2, &x3, &x4);
               ch->pcdata->condition[0] = x1;
               ch->pcdata->condition[1] = x2;
               ch->pcdata->condition[2] = x3;
               ch->pcdata->condition[3] = x4;
               fMatch = TRUE;
               break;
            }

/* Load color values - Samson 9-29-98 */
            {
               int x;

               if (!str_cmp(word, "Colors"))
               {
                  for (x = 0; x < max_colors; x++)
                     ch->pcdata->colors[x] = fread_number(fp);
                  fMatch = TRUE;
                  break;
               }
            }

            if (!str_cmp(word, "Coordinates"))
            {
               ch->coord->x = fread_number(fp);
               ch->coord->y = fread_number(fp);
               ch->map = fread_number(fp);

               if (!IS_ONMAP_FLAG(ch))
               {
                  ch->coord->x = -1;
                  ch->coord->y = -1;
                  ch->map = -1;
               }

               fMatch = TRUE;
               break;
            }

            if (!strcmp(word, "Council"))
            {
               ch->pcdata->council_name = fread_string(fp);
               if (!preload && ch->pcdata->council_name[0] != '\0' && (ch->pcdata->council = get_council(ch->pcdata->council_name)) == NULL)
               {
                  sprintf(buf, "Warning: the council %s no longer exists, and herefore you no longer\n\rbelong to a council.\n\r",
                     ch->pcdata->council_name);
                  send_to_char(buf, ch);
                  STRFREE(ch->pcdata->council_name);
                  ch->pcdata->council_name = STRALLOC("");
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'D':
            KEY("Damroll", ch->damroll, fread_number(fp));
            KEY("Deaf", ch->deaf, fread_number(fp));
            
            KEY("DOffer_time", ch->pcdata->duel_offer_time, fread_number(fp));
            KEY("DReceive_time", ch->pcdata->duel_receive_time, fread_number(fp));
            KEY("DOffier_name", ch->pcdata->duel_offer_name, fread_number(fp));
            KEY("DReceive_name", ch->pcdata->dual_receive_name, fread_number(fp));
            KEY("DOffer_prank", ch->pcdata->duel_offer_pranking, fread_number(fp));
            KEY("DReceive_prank", ch->pcdata->duel_receive_pranking, fread_number(fp));
            if (!strcmp(word, "Deity"))
            {
               ch->pcdata->deity_name = fread_string(fp);

               if (!preload && ch->pcdata->deity_name[0] != '\0' && (ch->pcdata->deity = get_deity(ch->pcdata->deity_name)) == NULL)
               {
                  sprintf(buf, "Warning: the deity %s no longer exists.\n\r", ch->pcdata->deity_name);
                  send_to_char(buf, ch);
                  STRFREE(ch->pcdata->deity_name);
                  ch->pcdata->deity_name = STRALLOC("");
                  ch->pcdata->favor = 0;
               }
               fMatch = TRUE;
               break;
            }
            KEY("Description", ch->description, fread_string(fp));
            break;

            /* 'E' was moved to after 'S' */
         case 'F':
	    KEY("Fame", ch->fame, fread_number(fp));
            KEY("Favor", ch->pcdata->favor, fread_number(fp));
            if (!strcmp(word, "Filename"))
            {
               /*
                * File Name already set externally.
                */
               fread_to_eol(fp);
               fMatch = TRUE;
               break;
            }
            KEY("Flags", ch->pcdata->flags, fread_number(fp));
            KEY("FLevel", ch->pcdata->flevel, fread_number(fp));
            if (!strcmp(word, "Forget"))
            {
               int x;
               int sn;
               x = fread_number(fp);
               sn = skill_lookup(fread_word(fp));
               ch->pcdata->forget[x] = sn;
               fMatch = TRUE;
               break;
            }
            KEY("FPrompt", ch->pcdata->fprompt, fread_string(fp));
            break;

         case 'G':
            KEY("Glory", ch->pcdata->quest_curr, fread_number(fp));
            KEY("Gold", ch->gold, fread_number(fp));
            KEY("GtRemort", ch->pcdata->gt_remort, fread_number(fp));
            KEY("Grip", ch->grip, fread_number(fp));
            KEY("GPrompt", ch->pcdata->gprompt, fread_number(fp));
            /* temporary measure */
            if (!strcmp(word, "Guild"))
            {
               ch->pcdata->clan_name = fread_string(fp);

               if (!preload && ch->pcdata->clan_name[0] != '\0' && (ch->pcdata->clan = get_clan(ch->pcdata->clan_name)) == NULL)
               {
                  sprintf(buf, "Warning: the organization %s no longer exists, and therefore you no longer\n\rbelong to that organization.\n\r",
                     ch->pcdata->clan_name);
                  send_to_char(buf, ch);
                  STRFREE(ch->pcdata->clan_name);
                  ch->pcdata->clan_name = STRALLOC("");
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'H':
            KEY("Haircolor", ch->pcdata->haircolor, fread_number(fp));
            KEY("Hairlength", ch->pcdata->hairlength, fread_number(fp));
            KEY("Hairstyle", ch->pcdata->hairstyle, fread_number(fp));
            KEY("Height", ch->height, fread_number(fp));

            if (!strcmp(word, "Helled"))
            {
               ch->pcdata->release_date = fread_number(fp);
               ch->pcdata->helled_by = fread_string(fp);
               if (ch->pcdata->release_date < current_time)
               {
                  STRFREE(ch->pcdata->helled_by);
                  ch->pcdata->helled_by = NULL;
                  ch->pcdata->release_date = 0;
               }
               fMatch = TRUE;
               break;
            }
            KEY("HitCount", ch->pcdata->hit_cnt, fread_number(fp));
            KEY("Hitroll", ch->hitroll, fread_number(fp));
            KEY("Homepage", ch->pcdata->homepage, fread_string_nohash(fp));
            if (!strcmp(word, "HomeTown"))
            {
               ch->pcdata->hometown = fread_number(fp);
               if (ch->pcdata->hometown >= sysdata.max_kingdom)
               {
                  bug("load_char:  %s had an invalid kingdom of %d", ch->name, ch->pcdata->hometown);
                  ch->pcdata->hometown = 0;
               }
               fMatch = TRUE;
               break;
            }
            
            if (!strcmp(word, "HpMpMvPer"))
            {
               ch->pcdata->per_hp = fread_number(fp);
               ch->pcdata->per_mana = fread_number(fp);
               ch->pcdata->per_move = fread_number(fp);
               fMatch = TRUE;
               break;
            }

            if (!strcmp(word, "HpManaMove"))
            {
               int toss;
               ch->hit = fread_number(fp);
               ch->max_hit = fread_number(fp);
               ch->mana = fread_number(fp);
               ch->max_mana = fread_number(fp);
               ch->move = fread_number(fp);
               toss = fread_number(fp);
               ch->max_move = 1000;
               fMatch = TRUE;
               break;
            }
            break;

         case 'I':
            KEY("ICQ", ch->pcdata->icq, fread_number(fp));
            if (!strcmp(word, "Ignored"))
            {
               char *temp;
               char fname[1024];
               struct stat fst;
               int ign;
               IGNORE_DATA *inode;

               /* Get the name */
               temp = fread_string(fp);

               sprintf(fname, "%s%c/%s", PLAYER_DIR, tolower(temp[0]), capitalize(temp));

               /* If there isn't a pfile for the name */
               /* then don't add it to the list       */
               if (stat(fname, &fst) == -1)
               {
                  fMatch = TRUE;
                  break;
               }

               /* Count the number of names already ignored */
               for (ign = 0, inode = ch->pcdata->first_ignored; inode; inode = inode->next)
               {
                  ign++;
               }

               /* Add the name unless the limit has been reached */
               if (ign >= MAX_IGN)
               {
                  bug("fread_char: too many ignored names");
               }
               else
               {
                  /* Add the name to the list */
                  CREATE(inode, IGNORE_DATA, 1);
                  inode->name = STRALLOC(temp);
                  inode->next = NULL;
                  inode->prev = NULL;

                  LINK(inode, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev);
               }

               fMatch = TRUE;
               break;
            }
            KEY("IllegalPK", ch->pcdata->illegal_pk, fread_number(fp));
            KEY("IMC", ch->pcdata->imc_deaf, fread_number(fp));
            KEY("IMCAllow", ch->pcdata->imc_allow, fread_number(fp));
            KEY("IMCDeny", ch->pcdata->imc_deny, fread_number(fp));
            KEY("ICEListen", ch->pcdata->ice_listen, fread_string_nohash(fp));
            KEY("Immune", ch->immune, fread_number(fp));
            KEY("Incr", ch->pcdata->incarnations, fread_number(fp));
            break;

         case 'J':
            KEY("Job", ch->pcdata->job, fread_number(fp));
            break;

         case 'K':
            KEY("Keeper", ch->pcdata->keeper, fread_number(fp));
            if (!strcmp(word, "Killed"))
            {
               fMatch = TRUE;
               if (killcnt >= MAX_KILLTRACK)
                  bug("fread_char: killcnt (%d) >= MAX_KILLTRACK", killcnt);
               else
               {
                  ch->pcdata->killed[killcnt].vnum = fread_number(fp);
                  ch->pcdata->killed[killcnt++].count = fread_number(fp);
               }
            }
            KEY("KingdomPid", ch->pcdata->kingdompid, fread_number(fp));
            break;

         case 'L':
            KEY("LastIntroCheck", ch->pcdata->lastintrocheck, fread_number(fp));
            KEY("LastInterest", ch->pcdata->lastinterest, fread_number(fp));
            KEY("LastPRankingCheck", ch->pcdata->lastprankingcheck, fread_number(fp));
     	    KEY("LastName", ch->last_name, fread_string(fp));        
            KEY("Level", ch->level, fread_number(fp));
            KEY("LongDescr", ch->long_descr, fread_string(fp));
            KEY("Lore", ch->pcdata->lore, fread_number(fp));
            KEY("LostCon", ch->pcdata->lostcon, fread_number(fp));
            if (!strcmp(word, "Languages"))
            {
               ch->speaks = fread_number(fp);
               ch->speaking = fread_number(fp);
               fMatch = TRUE;
            }
            if (!strcmp(word, "Limbs"))
            {
               ch->con_rarm = fread_number(fp);
               ch->con_larm = fread_number(fp);
               ch->con_rleg = fread_number(fp);
               ch->con_lleg = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            break;

         case 'M':
            KEY("Mapdir", ch->pcdata->mapdir, fread_number(fp));
            KEY("ManaCount", ch->pcdata->mana_cnt, fread_number(fp));
            KEY("MaxColors", max_colors, fread_number(fp));
            KEY("MDeaths", ch->pcdata->mdeaths, fread_number(fp));
            KEY("Mentalstate", ch->mental_state, fread_number(fp));
            KEY("MGlory", ch->pcdata->quest_accum, fread_number(fp));
            KEY("Minsnoop", ch->pcdata->min_snoop, fread_number(fp));
            KEY("MKills", ch->pcdata->mkills, fread_number(fp));
            KEY("MoveMessage", ch->movement, fread_string(fp));
            KEY("Mobinvis", ch->mobinvis, fread_number(fp));
            if (!strcmp(word, "MobRange"))
            {
               ch->pcdata->m_range_lo = fread_number(fp);
               ch->pcdata->m_range_hi = fread_number(fp);
               fMatch = TRUE;
            }
            KEY("MoveR", ch->mover, fread_number(fp));
            break;

         case 'N':
            KEY("Name", ch->name, fread_string(fp));
            KEY("NoAffectedBy", ch->no_affected_by, fread_bitvector(fp));
            KEY("NoImmune", ch->no_immune, fread_number(fp));
            if (!strcmp(word, "NewResists"))
            {
               line = fread_line(fp);
               x1=x2=x3=x4=x5=x6=x7=x8=x9=x10=x11=x12=x13=x14=x15=0;
               sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                             &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15);
               ch->apply_res_fire[0] = x1;
               ch->apply_res_water[0] = x2;
               ch->apply_res_air[0] = x3;
               ch->apply_res_earth[0] = x4;
               ch->apply_res_energy[0] = x5;
               ch->apply_res_magic[0] = x6;
               ch->apply_res_nonmagic[0] = x7;
               ch->apply_res_blunt[0] = x8;
               ch->apply_res_pierce[0] = x9;
               ch->apply_res_slash[0] = x10;
               ch->apply_res_poison[0] = x11;
               ch->apply_res_paralysis[0] = x12;
               ch->apply_res_holy[0] = x13;
               ch->apply_res_unholy[0] = x14;
               ch->apply_res_undead[0] = x15;
               fMatch = TRUE;
               break;
            }
            if (!strcmp(word, "NewResists1"))
            {
               line = fread_line(fp);
               x1=x2=x3=x4=x5=x6=x7=x8=x9=x10=x11=x12=x13=x14=x15=0;
               sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                             &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15);
               ch->apply_res_fire[1] = x1;
               ch->apply_res_water[1] = x2;
               ch->apply_res_air[1] = x3;
               ch->apply_res_earth[1] = x4;
               ch->apply_res_energy[1] = x5;
               ch->apply_res_magic[1] = x6;
               ch->apply_res_nonmagic[1] = x7;
               ch->apply_res_blunt[1] = x8;
               ch->apply_res_pierce[1] = x9;
               ch->apply_res_slash[1] = x10;
               ch->apply_res_poison[1] = x11;
               ch->apply_res_paralysis[1] = x12;
               ch->apply_res_holy[1] = x13;
               ch->apply_res_unholy[1] = x14;
               ch->apply_res_undead[1] = x15;
               fMatch = TRUE;
               break;
            }
            if (!strcmp(word, "NewResists2"))
            {
               line = fread_line(fp);
               x1=x2=x3=x4=x5=x6=x7=x8=x9=x10=x11=x12=x13=x14=x15=0;
               sscanf(line, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                             &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14, &x15);
               ch->apply_res_fire[2] = x1;
               ch->apply_res_water[2] = x2;
               ch->apply_res_air[2] = x3;
               ch->apply_res_earth[2] = x4;
               ch->apply_res_energy[2] = x5;
               ch->apply_res_magic[2] = x6;
               ch->apply_res_nonmagic[2] = x7;
               ch->apply_res_blunt[2] = x8;
               ch->apply_res_pierce[2] = x9;
               ch->apply_res_slash[2] = x10;
               ch->apply_res_poison[2] = x11;
               ch->apply_res_paralysis[2] = x12;
               ch->apply_res_holy[2] = x13;
               ch->apply_res_unholy[2] = x14;
               ch->apply_res_undead[2] = x15;
               fMatch = TRUE;
               break;
            }
            if (!strcmp(word, "NoLearn"))
            {
               int x;
               int sn;
               x = fread_number(fp);
               sn = skill_lookup(fread_word(fp));
               ch->pcdata->nolearn[x] = sn;
               fMatch = TRUE;
               break;
            }
            KEY("NoResistant", ch->no_resistant, fread_number(fp));
            KEY("NoSusceptible", ch->no_susceptible, fread_number(fp));
            if (!strcmp("Nuisance", word))
            {
               fMatch = TRUE;
               CREATE(ch->pcdata->nuisance, NUISANCE_DATA, 1);
               ch->pcdata->nuisance->time = fread_number(fp);
               ch->pcdata->nuisance->max_time = fread_number(fp);
               ch->pcdata->nuisance->flags = fread_number(fp);
               ch->pcdata->nuisance->power = 1;
            }
            if (!strcmp("NuisanceNew", word))
            {
               fMatch = TRUE;
               CREATE(ch->pcdata->nuisance, NUISANCE_DATA, 1);
               ch->pcdata->nuisance->time = fread_number(fp);
               ch->pcdata->nuisance->max_time = fread_number(fp);
               ch->pcdata->nuisance->flags = fread_number(fp);
               ch->pcdata->nuisance->power = fread_number(fp);
            }
            break;
         case 'O':
            KEY("Offeredlname", ch->pcdata->offeredlname, fread_string(fp));
            KEY("Outcast_time", ch->pcdata->outcast_time, fread_number(fp));
            if (!strcmp(word, "ObjRange"))
            {
               ch->pcdata->o_range_lo = fread_number(fp);
               ch->pcdata->o_range_hi = fread_number(fp);
               fMatch = TRUE;
            }
            break;

         case 'P':
            KEY("Pagerlen", ch->pcdata->pagerlen, fread_number(fp));
            KEY("Password", ch->pcdata->pwd, fread_string_nohash(fp));
            KEY("PDeaths", ch->pcdata->pdeaths, fread_number(fp));
            KEY("Pid", ch->pcdata->pid, fread_number(fp));
            KEY("PkPower", ch->pcdata->pkpower, fread_number(fp));
            KEY("PRanking", ch->pcdata->pranking, fread_number(fp));
            if (!strcmp(word, "Pkilled"))
            {
               PKILLED_DATA *pkl;

               fMatch = TRUE;

               CREATE(pkl, PKILLED_DATA, 1);
               pkl->name = fread_string(fp);
               LINK(pkl, ch->pcdata->first_pkilled, ch->pcdata->last_pkilled, next, prev);
               ch->pcdata->pkilled++;
               if (ch->pcdata->pkilled > MAX_PKILLTRACK)
                  bug("%s pkill number is over 10 with %d", ch->name, ch->pcdata->pkilled);
            }
            KEY("PKills", ch->pcdata->pkills, fread_number(fp));
            KEY("Played", ch->played, fread_number(fp));
            KEY("PortalsFnd", ch->pcdata->portalfnd, fread_bitvector(fp));
            KEY("PowerRanking", ch->pcdata->power_ranking, fread_number(fp));
            /* KEY( "Position", ch->position,  fread_number( fp ) ); */
            /*
             *  new positions are stored in the file from 100 up
             *  old positions are from 0 up
             *  if reading an old position, some translation is necessary
             */
            if (!strcmp(word, "Position"))
            {
               ch->position = fread_number(fp);
               if (ch->position < 100)
               {
                  switch (ch->position)
                  {
                     default:;
                     case 0:;
                     case 1:;
                     case 2:;
                     case 3:;
                     case 4:
                        break;
                     case 5:
                        ch->position = 6;
                        break;
                     case 6:
                        ch->position = 8;
                        break;
                     case 7:
                        ch->position = 9;
                        break;
                     case 8:
                        ch->position = 12;
                        break;
                     case 9:
                        ch->position = 13;
                        break;
                     case 10:
                        ch->position = 14;
                        break;
                     case 11:
                        ch->position = 15;
                        break;
                  }
                  fMatch = TRUE;
               }
               else
               {
                  ch->position -= 100;
                  fMatch = TRUE;
               }
            }
            KEY("Practice", ch->practice, fread_number(fp));
            KEY("Pretit", ch->pcdata->pretit, fread_string_nohash(fp));
            KEY("Prompt", ch->pcdata->prompt, fread_string(fp));
            if (!strcmp(word, "PTimer"))
            {
               add_timer(ch, TIMER_PKILLED, fread_number(fp), NULL, 0);
               fMatch = TRUE;
               break;
            }
            break;
 
         case 'Q':
            KEY("Quest_Loss", ch->pcdata->quest_losses, fread_number(fp));
            KEY("Quest_Wins", ch->pcdata->quest_wins, fread_number(fp));
            break;
            
         case 'R':
            KEY("Race", ch->race, fread_number(fp));
            KEY("Rank", ch->pcdata->rank, fread_string_nohash(fp));
            KEY("Resistant", ch->resistant, fread_number(fp));
            KEY("Resource", ch->pcdata->resource, fread_number(fp));
            KEY("Resourcetype", ch->pcdata->resourcetype, fread_number(fp));
            KEY("Restore_time", ch->pcdata->restore_time, fread_number(fp));
            KEY("Reward", ch->pcdata->reward_curr, fread_number(fp));
            KEY("Reward_Accum", ch->pcdata->reward_accum, fread_number(fp));
            KEY("Righthanded", ch->pcdata->righthanded, fread_number(fp));

            if (!strcmp(word, "Room"))
            {
               ch->in_room = get_room_index(fread_number(fp));
               if (!ch->in_room)
                  ch->in_room = get_room_index(ROOM_VNUM_LIMBO);
               fMatch = TRUE;
               break;
            }
            if (!strcmp(word, "RoomRange"))
            {
               ch->pcdata->r_range_lo = fread_number(fp);
               ch->pcdata->r_range_hi = fread_number(fp);
               fMatch = TRUE;
            }
            break;

         case 'S':
            KEY("Sex", ch->sex, fread_number(fp));
            if (!str_cmp(word, "ShipUID"))
            {
               SHIP_DATA *ship;
               int uid = fread_number(fp);
               for (ship = first_ship; ship; ship = ship->next)
               {
                  if (ship->uid == uid)
                  {
                     ch->ship = ship;
                     ch->coord->x = ship->x;
                     ch->coord->y = ship->y;
                     LINK(ch, ship->first_char, ship->last_char, next_ship, prev_ship);
                     break;
                  }
               }
               if (!ship)
               {
                  bug("Player %s is attached to an invalid ship", ch->name);
                  char_from_room(ch);
                  char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
                  ch->coord->x = ch->coord->y = 0;
                  ch->map = -1;
                  REMOVE_ONMAP_FLAG(ch);
               }
               fMatch = TRUE;
               break;
            }
            KEY("SLastName", ch->pcdata->slastname, fread_number(fp));
            KEY("ShortDescr", ch->short_descr, fread_string(fp));
            KEY("Skincolor", ch->pcdata->skincolor, fread_number(fp));
            if (!str_cmp(word, "Speed"))
            {
               ch->speed = fread_number(fp);
               if (ch->desc)
                  ch->desc->speed = ch->speed;
               fMatch = TRUE;
               break;
            }
            KEY("SOffier_name", ch->pcdata->spar_offer_name, fread_number(fp));
            KEY("SReceive_name", ch->pcdata->spar_receive_name, fread_number(fp));
            KEY("SPoints", ch->pcdata->spoints, fread_number(fp));
            KEY("Stable", ch->pcdata->stable, fread_number(fp));
            KEY("Stablecurr", ch->pcdata->stablecurr, fread_number(fp));
            KEY("Stablenum", ch->pcdata->stablenum, fread_number(fp));
            KEY("Style", ch->style, fread_number(fp));
            KEY("Susceptible", ch->susceptible, fread_number(fp));
            if (!strcmp(word, "SavingThrow"))
            {
               ch->saving_wand = fread_number(fp);
               ch->saving_poison_death = ch->saving_wand;
               ch->saving_para_petri = ch->saving_wand;
               ch->saving_breath = ch->saving_wand;
               ch->saving_spell_staff = ch->saving_wand;
               fMatch = TRUE;
               break;
            }

            if (!strcmp(word, "SavingThrows"))
            {
               ch->saving_poison_death = fread_number(fp);
               ch->saving_wand = fread_number(fp);
               ch->saving_para_petri = fread_number(fp);
               ch->saving_breath = fread_number(fp);
               ch->saving_spell_staff = fread_number(fp);
               fMatch = TRUE;
               break;
            }

            if (!strcmp(word, "Site"))
            {
               if (!preload)
               {
                  sprintf(buf, "Last connected from: %s\n\r", fread_word(fp));
                  send_to_char(buf, ch);
               }
               else
                  fread_to_eol(fp);
               fMatch = TRUE;
               if (preload)
                  word = "End";
               else
                  break;
            }
            
            if (!strcmp(word, "Skill"))
            {
               int sn;
               int value;
               int mastery;
               int spercent;

               if (preload)
                  word = "End";
               else
               {
                  spercent = fread_number(fp);
                  value = fread_number(fp);
                  mastery = fread_number(fp);
                  if (file_ver < 3)
                     sn = skill_lookup(fread_word(fp));
                  else
                     sn = bsearch_skill_exact(fread_word(fp), gsn_first_skill, gsn_first_weapon - 1);
                  if (sn < 0)
                     bug("Fread_char: unknown skill.", 0);
                  else
                  {
                     ch->pcdata->spercent[sn] = spercent;
                     ch->pcdata->learned[sn] = value;
                     ch->pcdata->ranking[sn] = mastery;
                  }
                  fMatch = TRUE;
                  break;
               }
            }

            if (!strcmp(word, "Spell"))
            {
               int sn;
               int value;
               int mastery;
               int spercent;

               if (preload)
                  word = "End";
               else
               {
               	  spercent = fread_number(fp);
                  value = fread_number(fp);
                  mastery = fread_number(fp);

                  sn = bsearch_skill_exact(fread_word(fp), gsn_first_spell, gsn_first_skill - 1);
                  if (sn < 0)
                     bug("Fread_char: unknown spell.", 0);
                  else
                  {
                     ch->pcdata->spercent[sn] = spercent;
                     ch->pcdata->learned[sn] = value;
                     ch->pcdata->ranking[sn] = mastery;
                  }
                  fMatch = TRUE;
                  break;
               }
            }
            if (strcmp(word, "End"))
               break;

         case 'E':
            if (!strcmp(word, "End"))
            {
               if (!ch->short_descr)
                  ch->short_descr = STRALLOC("");
               if (!ch->long_descr)
                  ch->long_descr = STRALLOC("");
               if (!ch->description)
                  ch->description = STRALLOC("");
               if (!ch->pcdata->bamfin)
                  ch->pcdata->bamfin = str_dup("");
               if (!ch->pcdata->bamfout)
                  ch->pcdata->bamfout = str_dup("");
               if (!ch->pcdata->bio)
                  ch->pcdata->bio = STRALLOC("");
               if (!ch->pcdata->came_from)
                  ch->pcdata->came_from = STRALLOC("");
               if (!ch->pcdata->rank)
                  ch->pcdata->rank = str_dup("");
               if (!ch->pcdata->bestowments)
                  ch->pcdata->bestowments = str_dup("");
               if (!ch->pcdata->title)
                  ch->pcdata->title = STRALLOC("");
               if (!ch->pcdata->pretit)
                  ch->pcdata->pretit = str_dup("");

               if (!ch->pcdata->homepage)
                  ch->pcdata->homepage = str_dup("");
               if (!ch->pcdata->email)
                  ch->pcdata->email = str_dup("");
               if (!ch->pcdata->authed_by)
                  ch->pcdata->authed_by = STRALLOC("");
               if (!ch->pcdata->prompt)
                  ch->pcdata->prompt = STRALLOC("");
               if (!ch->pcdata->fprompt)
                  ch->pcdata->fprompt = STRALLOC("");
               ch->editor = NULL;
               killcnt = 1;
               if (killcnt < MAX_KILLTRACK)
                  ch->pcdata->killed[killcnt].vnum = 0;

               /* no good for newbies at all */
               if (!IS_IMMORTAL(ch) && !ch->speaking)
                  ch->speaking = LANG_COMMON;
               /* ch->speaking = race_table[ch->race]->language; */
               if (IS_IMMORTAL(ch))
               {
                  int i;

                  ch->speaks = ~0;
                  if (ch->speaking == 0)
                     ch->speaking = ~0;

                  CREATE(ch->pcdata->tell_history, char *, 26);

                  for (i = 0; i < 26; i++)
                     ch->pcdata->tell_history[i] = NULL;
               }
               if (!ch->pcdata->prompt)
                  ch->pcdata->prompt = STRALLOC("");

               /* this disallows chars from being 6', 180lbs, but easier than a flag */
               if (ch->height == 72)
                  ch->height = number_range(race_table[ch->race]->height * .9, race_table[ch->race]->height * 1.1);
               if (ch->weight == 180)
                  ch->weight = number_range(race_table[ch->race]->weight * .9, race_table[ch->race]->weight * 1.1);

               REMOVE_PLR_FLAG(ch, PLR_MAPEDIT); /* In case they saved while editing */

               return;
            }
            KEY("Email", ch->pcdata->email, fread_string_nohash(fp));
            KEY("Elements", ch->elementb, fread_number(fp));
            KEY("Eyecolor", ch->pcdata->eyecolor, fread_number(fp));
            break;

         case 'T':
            KEY("Talent", ch->pcdata->talent, fread_bitvector(fp));
            KEY("Target", ch->pcdata->target, fread_number(fp));
            KEY("Target_Limb", ch->pcdata->target_limb, fread_number(fp));
            if (!strcmp(word, "Tongue"))
            {
               int sn;
               int value;
               int mastery;
               int spercent;

               if (preload)
                  word = "End";
               else
               {
                  spercent = fread_number(fp);
                  value = fread_number(fp);
                  mastery = fread_number(fp);

                  sn = bsearch_skill_exact(fread_word(fp), gsn_first_tongue, gsn_top_sn - 1);
                  if (sn < 0)
                     bug("Fread_char: unknown tongue.", 0);
                  else
                  {
                     ch->pcdata->spercent[sn] = spercent;
                     ch->pcdata->learned[sn] = value;
                     ch->pcdata->ranking[sn] = mastery;
                  }
                  fMatch = TRUE;
               }
               break;
            }
            if (!strcmp(word, "Town"))
            {
               char *town;
               TOWN_DATA *gtown;

               town = fread_string(fp);
               
               gtown = get_town(town);

               if (gtown)
                  ch->pcdata->town = gtown;
               else
               {
                  ch->pcdata->town = NULL;
                  bug("%s has a bad town field of %s.\n\r", ch->name, town);
               }
               fMatch = TRUE;
            }
            KEY("Tier", ch->pcdata->tier, fread_number(fp));
            KEY("TimeoutLogin", ch->pcdata->timeout_login, fread_number(fp));
            KEY("TimeoutNotes", ch->pcdata->timeout_notes, fread_number(fp));
            KEY("TimeoutIdle", ch->pcdata->timeout_idle, fread_number(fp));
            KEY("Tone", ch->tone, fread_string(fp));
            KEY("Train", ch->pcdata->train, fread_number(fp));
            KEY("Trust", ch->trust, fread_number(fp));
            KEY("TwinkPoints", ch->pcdata->twink_points, fread_number(fp));
            /* Let no character be trusted higher than one below maxlevel -- Narn */
            ch->trust = UMIN(ch->trust, MAX_LEVEL - 1);

            if (!strcmp(word, "Title"))
            {
               ch->pcdata->title = fread_string(fp);
               if (isalpha(ch->pcdata->title[0]) || isdigit(ch->pcdata->title[0]))
               {
                  sprintf(buf, " %s", ch->pcdata->title);
                  if (ch->pcdata->title)
                     STRFREE(ch->pcdata->title);
                  ch->pcdata->title = STRALLOC(buf);
               }
               fMatch = TRUE;
               break;
            }

            break;

         case 'V':
            if (!strcmp(word, "Vnum"))
            {
               ch->pIndexData = get_mob_index(fread_number(fp));
               fMatch = TRUE;
               break;
            }
            KEY("Version", file_ver, fread_number(fp));
            break;

         case 'W':
            KEY("Weight", ch->weight, fread_number(fp));
            KEY("Whonum", ch->pcdata->whonum, fread_number(fp));
            KEY("Wimpy", ch->wimpy, fread_number(fp));
            KEY("WizInvis", ch->pcdata->wizinvis, fread_number(fp));
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_char: no match: %s", word);
         bug(buf, 0);
      }
   }
}


void fread_obj(CHAR_DATA * ch, FILE * fp, sh_int os_type)
{
   OBJ_DATA *obj;
   char *word;
   char buf[MSL];
   int iNest;
   bool fMatch;
   int version = 3;
   bool fNest;
   bool fVnum;
   ROOM_INDEX_DATA *room = NULL;

   if (ch)
      room = ch->in_room;
   CREATE(obj, OBJ_DATA, 1);
   obj->count = 1;
   obj->wear_loc = -1;
   obj->weight = 1;
   CREATE(obj->coord, COORD_DATA, 1);
   obj->map = -1;
   obj->coord->x = -1;
   obj->coord->y = -1;

   fNest = TRUE; /* Requiring a Nest 0 is a waste */
   fVnum = TRUE;
   iNest = 0;

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
            KEY("ActionDesc", obj->action_desc, fread_string(fp));            
            
            if (!strcmp(word, "Affect") || !strcmp(word, "AffectData") || !strcmp(word, "AffectRecur") || !strcmp(word, "AffectDataRecur"))
            {
               AFFECT_DATA *paf;
               int recur = 0;
               
               if (!str_cmp(word, "AffectRecur") || !str_cmp(word, "AffectDataRecur"))
                  recur = 1;

               CREATE(paf, AFFECT_DATA, 1);
               if (!strcmp(word, "Affect") || !strcmp(word, "AffectRecur"))
               {
                  paf->type = fread_number(fp);
               }
               else
               {
                  int sn;

                  sn = skill_lookup(fread_word(fp));
                  if (sn < 0)
                     bug("Fread_obj: unknown skill.", 0);
                  else
                     paf->type = sn;
               }
               paf->duration = fread_number(fp);
               if (recur)
               {
                  paf->modifier = skill_lookup(fread_word(fp));
               }
               else
               {
                  paf->modifier = fread_number(fp);
               }
               paf->location = fread_number(fp);
               paf->bitvector = fread_bitvector(fp);
               if (version >= 5)
                  paf->gemnum = fread_number(fp);
               LINK(paf, obj->first_affect, obj->last_affect, next, prev);
               fMatch = TRUE;
               break;
            }
            break;

         case 'C':
            if (!strcmp(word, "Coords"))
            {
               obj->map = fread_number(fp);
               obj->coord->x = fread_number(fp);
               obj->coord->y = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            KEY("Cost", obj->cost, fread_number(fp));
            KEY("Count", obj->count, fread_number(fp));
            break;

         case 'D':
            KEY("Description", obj->description, fread_string(fp));
            break;

         case 'E':
            KEY("ExtraFlags", obj->extra_flags, fread_bitvector(fp));

            if (!strcmp(word, "ExtraDescr"))
            {
               EXTRA_DESCR_DATA *ed;

               CREATE(ed, EXTRA_DESCR_DATA, 1);
               ed->keyword = fread_string(fp);
               ed->description = fread_string(fp);
               LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev);
               fMatch = TRUE;
            }

            if (!strcmp(word, "End"))
            {
               if (!fNest || !fVnum)
               {
                  if (obj->name)
                     sprintf(buf, "Fread_obj: %s incomplete object.", obj->name);
                  else
                     sprintf(buf, "Fread_obj: incomplete object.");
                  bug(buf, 0);
                  if (obj->name)
                     STRFREE(obj->name);
                  if (obj->description)
                     STRFREE(obj->description);
                  if (obj->short_descr)
                     STRFREE(obj->short_descr);
                  DISPOSE(obj);
                  return;
               }
               else
               {
                  sh_int wear_loc = obj->wear_loc;

                  if (!obj->name)
                     obj->name = QUICKLINK(obj->pIndexData->name);
                  if (!obj->description)
                     obj->description = QUICKLINK(obj->pIndexData->description);
                  if (!obj->short_descr)
                     obj->short_descr = QUICKLINK(obj->pIndexData->short_descr);
                  if (!obj->action_desc)
                     obj->action_desc = QUICKLINK(obj->pIndexData->action_desc);
                  LINK(obj, first_object, last_object, next, prev);
                  obj->pIndexData->count += obj->count;
                  if (!obj->serial)
                  {
                     cur_obj_serial = UMAX((cur_obj_serial + 1) & (BV30 - 1), 1);
                     obj->serial = obj->pIndexData->serial = cur_obj_serial;
                  }
                  if (fNest)
                     rgObjNest[iNest] = obj;
                  numobjsloaded += obj->count;
                  ++physicalobjects;
                  if (file_ver > 1 || obj->wear_loc < -1 || obj->wear_loc >= MAX_WEAR)
                     obj->wear_loc = -1;
                  /* Corpse saving. -- Altrag */
                  if (os_type == OS_CORPSE)
                  {
                     if (!room)
                     {
                        bug("Fread_obj: Corpse without room, moving to the morgue", 0);
                        room = get_room_index(VNUM_ROOM_MORGUE);
                     }
                     /* Give the corpse a timer if there isn't one */

                     if (obj->timer < 1)
                        obj->timer = 40;
                     obj = obj_to_room(obj, room, NULL);
                  }
                  else if (iNest == 0 || rgObjNest[iNest] == NULL)
                  {
                     int slot = -1;
                     bool reslot = FALSE;

                     if (file_ver > 1 && wear_loc > -1 && wear_loc < MAX_WEAR)
                     {
                        int x;

                        for (x = 0; x < MAX_LAYERS; x++)
                           if (!save_equipment[wear_loc][x])
                           {
                              save_equipment[wear_loc][x] = obj;
                              slot = x;
                              reslot = TRUE;
                              break;
                           }
                        if (x == MAX_LAYERS)
                           bug("Fread_obj: too many layers %d", wear_loc);
                     }
                     if (os_type == OS_BANK)
                     {
                        //Set in tables.c when a bank obj is loaded in for a town
                        if (globaltownload)
                           obj = obj_to_townbank(obj, globaltownptr);
                        else
                           obj = obj_to_bank(obj, ch);
                     }
                     else if (os_type == OS_MARKET)
                     {
                        globalmarketptr->obj = obj;
                     }
                     else if (os_type == OS_GROUND)
                     {
                        if (!room)
                        {
                           bug("Fread_obj: Cannot find anywhere to put a quest obj, moving it to the morgue", 0);
                           room = get_room_index(VNUM_ROOM_MORGUE);
                        }
                        obj = obj_to_room(obj, room, NULL);
                     }
                     else
                        obj = obj_to_char(obj, ch);
                     if (reslot && slot != -1)
                        save_equipment[wear_loc][slot] = obj;
                  }
                  else
                  {
                     if (rgObjNest[iNest - 1])
                     {
                        separate_obj(rgObjNest[iNest - 1]);
                        obj = obj_to_obj(obj, rgObjNest[iNest - 1]);
                     }
                     else
                        bug("Fread_obj: nest layer missing %d", iNest - 1);
                  }
                  if (fNest)
                     rgObjNest[iNest] = obj;
                  return;
               }
            }
            break;
            
         case 'G':
            if (!str_cmp(word, "Gem"))
            {
               IMBUE_DATA *imbue;
               
               CREATE(imbue, IMBUE_DATA, 1);
               LINK(imbue, obj->first_imbue, obj->last_imbue, next, prev);
               imbue->type = fread_number(fp);
               imbue->sworth = fread_number(fp);
               imbue->lowvalue = fread_number(fp);
               imbue->highvalue = fread_number(fp);
               imbue->value = fread_number(fp);
               imbue->type2 = fread_number(fp);
               imbue->sworth2 = fread_number(fp);
               imbue->lowvalue2 = fread_number(fp);
               imbue->highvalue2 = fread_number(fp);
               imbue->value2 = fread_number(fp);
               imbue->type3 = fread_number(fp);
               imbue->sworth3 = fread_number(fp);
               imbue->lowvalue3 = fread_number(fp);
               imbue->highvalue3 = fread_number(fp);
               imbue->value3 = fread_number(fp);
               imbue->plevel = fread_number(fp);
               imbue->gemnum = fread_number(fp);
               fMatch = TRUE;
            }
            break;

         case 'I':
            KEY("ItemType", obj->item_type, fread_number(fp));
            KEY("Imbueslots", obj->imbueslots, fread_number(fp));
            break;

         case 'L':
            KEY("Level", obj->level, fread_number(fp));
            break;

         case 'N':
            KEY("Name", obj->name, fread_string(fp));

            if (!strcmp(word, "Nest"))
            {
               iNest = fread_number(fp);
               if (iNest < 0 || iNest >= MAX_NEST)
               {
                  bug("Fread_obj: bad nest %d.", iNest);
                  iNest = 0;
                  fNest = FALSE;
               }
               fMatch = TRUE;
            }
            break;

         case 'R':
            KEY("Room", room, get_room_index(fread_number(fp)));

         case 'S':
            KEY("ShortDescr", obj->short_descr, fread_string(fp));
            KEY("Sworthrestrict", obj->sworthrestrict, fread_number(fp));
            if (!strcmp(word, "Spell"))
            {
               int iValue;
               int sn;

               iValue = fread_number(fp);
               sn = skill_lookup(fread_word(fp));
               if (iValue < 0 || iValue > 5)
                  bug("Fread_obj: bad iValue %d.", iValue);
               else if (sn < 0)
                  bug("Fread_obj: unknown skill.", 0);
               else
                  obj->value[iValue] = sn;
               fMatch = TRUE;
               break;
            }

            break;

         case 'T':
            KEY("Timer", obj->timer, fread_number(fp));
            if (!str_cmp(word, "Trap"))
            {
               TRAP_DATA *trap;
               trap = load_trap_file(fp);
               if (!trap)
               {
                  bug("fread_obj:  Did not load trap file correctly.");
               }
               else
               {
                  obj->trap = trap;
                  trap->obj = obj;
               }
               fMatch = TRUE;
            }
            break;

         case 'V':
            KEY("Version", version, fread_number(fp));
            if (!strcmp(word, "Values"))
            {
               int x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14;
               char *ln = fread_line(fp);

               x1 = x2 = x3 = x4 = x5 = x6 = x7 = x8 = x9 = x10 = x11 = x12 = x13 = x14 = 0;
               sscanf(ln, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d", 
                  &x1, &x2, &x3, &x4, &x5, &x6, &x7, &x8, &x9, &x10, &x11, &x12, &x13, &x14);
               if (obj->pIndexData && (obj->pIndexData->vnum < 21101 || obj->pIndexData->vnum > 21145))
               { 
                  obj->value[0] = x1;
                  obj->value[1] = x2;
                  obj->value[2] = x3;
                  obj->value[3] = x4;
                  obj->value[4] = x5;
                  obj->value[5] = x6;
                  obj->value[6] = x7;
                  obj->value[7] = x8;
                  obj->value[8] = x9;
                  obj->value[9] = x10;
                  obj->value[10] = x11;
                  obj->value[11] = x12;
                  obj->value[12] = x13;
                  obj->value[13] = x14;
               }
               else if (obj->pIndexData && obj->pIndexData->vnum >= 21101 && obj->pIndexData->vnum <= 21145)
               {
                  obj->value[0] = obj->pIndexData->value[0];
                  obj->value[1] = obj->pIndexData->value[1];
                  obj->value[2] = obj->pIndexData->value[2];
                  obj->value[3] = obj->pIndexData->value[3];
                  obj->value[4] = obj->pIndexData->value[4];
                  obj->value[5] = obj->pIndexData->value[5];
                  obj->value[6] = obj->pIndexData->value[6];
                  obj->value[7] = obj->pIndexData->value[7];
                  obj->value[8] = obj->pIndexData->value[8];
                  obj->value[9] = obj->pIndexData->value[9];
                  obj->value[10] = obj->pIndexData->value[10];
                  obj->value[11] = obj->pIndexData->value[11];
                  obj->value[12] = obj->pIndexData->value[12];
                  obj->value[13] = obj->pIndexData->value[13];
               }
               if (obj->item_type == ITEM_WEAPON && obj->pIndexData)
               {
                  if (obj->value[12] == 0)
                     obj->value[12] = obj->pIndexData->value[12];
                  if (obj->value[13] == 0)
                     obj->value[13] = obj->pIndexData->value[13];
               }
               fMatch = TRUE;
               break;
            }

            if (!strcmp(word, "Vnum"))
            {
               int vnum;

               vnum = fread_number(fp);
               /*  bug( "Fread_obj: bad vnum %d.", vnum );  */
               if ((obj->pIndexData = get_obj_index(vnum)) == NULL)
                  fVnum = FALSE;
               else
               {
                  fVnum = TRUE;
                  obj->cost = obj->pIndexData->cost;
                  obj->weight = obj->pIndexData->weight;
                  obj->bless_dur = obj->pIndexData->bless_dur;
                  obj->item_type = obj->pIndexData->item_type;
                  obj->wear_flags = obj->pIndexData->wear_flags;
                  obj->extra_flags = obj->pIndexData->extra_flags;
               }
               fMatch = TRUE;
               break;
            }
            break;

         case 'W':
            KEY("WearFlags", obj->wear_flags, fread_number(fp));
            KEY("WearLoc", obj->wear_loc, fread_number(fp));
            KEY("Weight", obj->weight, fread_float(fp));
            break;
            
        case 'B':
            KEY("BlessDuration", obj->bless_dur, fread_number(fp));
            break;

      }

      if (!fMatch)
      {
         EXTRA_DESCR_DATA *ed;
         AFFECT_DATA *paf;

         bug("Fread_obj: no match.", 0);
         bug(word, 0);
         fread_to_eol(fp);
         if (obj->name)
            STRFREE(obj->name);
         if (obj->description)
            STRFREE(obj->description);
         if (obj->short_descr)
            STRFREE(obj->short_descr);
         while ((ed = obj->first_extradesc) != NULL)
         {
            STRFREE(ed->keyword);
            STRFREE(ed->description);
            UNLINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev);
            DISPOSE(ed);
         }
         while ((paf = obj->first_affect) != NULL)
         {
            UNLINK(paf, obj->first_affect, obj->last_affect, next, prev);
            DISPOSE(paf);
         }
         DISPOSE(obj);
         return;
      }
   }
}

void set_alarm(long seconds)
{
#ifdef WIN32
   kill_timer(); /* kill old timer */
   timer_code = timeSetEvent(seconds * 1000L, 1000, alarm_handler, 0, TIME_PERIODIC);
#else
   alarm(seconds);
#endif
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   char name[MIL];
   struct stat fst;

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Usage: last <playername>\n\r", ch);
      send_to_char("Usage: last <# of entries OR \'-1\' for all entries OR \'today\' for all of today's entries>\n\r", ch);
      send_to_char("Usage: last <playername> <count>\n\r", ch);
      return;
   }
   if (get_trust(ch) < LEVEL_STAFF)
   {
      set_char_color(AT_IMMORT, ch);
      send_to_char("Their godly glow prevents you from getting a good look.\n\r", ch);
      return;
   }
   if (isdigit(arg[0]) || atoi(arg) == -1 || !str_cmp(arg, "today")) //View list instead of players
   {
      send_to_char("Name                     Time                        Host/Ip\n\r---------------------------------------------------------------------------\n\r", ch);
      if (!str_cmp(arg, "today"))
         read_last_file(ch, -2, NULL);
      else
         read_last_file(ch, atoi(arg), NULL);
      return;
   }  
   strcpy(name, capitalize(arg));
   if (argument[0] != '\0')
   {
      send_to_char("Name                     Time                        Host/Ip\n\r---------------------------------------------------------------------------\n\r", ch);
      read_last_file(ch, atoi(argument), name);
      return;
   }
   sprintf(buf, "%s%c/%s", PLAYER_DIR, tolower(arg[0]), name);
   
   if (stat(buf, &fst) != -1)
      sprintf(buf, "%s was last on: %s\r", name, ctime(&fst.st_mtime));
   else
      sprintf(buf, "%s was not found.\n\r", name);
   send_to_char(buf, ch);
}

/*
 * Added support for removeing so we could take out the write_corpses
 * so we could take it out of the save_char_obj function. --Shaddai
 */

void write_corpses(CHAR_DATA * ch, char *name, OBJ_DATA * objrem)
{
   OBJ_DATA *corpse;
   FILE *fp = NULL;

   /* Name and ch support so that we dont have to have a char to save their
      corpses.. (ie: decayed corpses while offline) */
   if (ch && IS_NPC(ch))
   {
      bug("Write_corpses: writing NPC corpse.", 0);
      return;
   }
   if (ch)
      name = ch->name;
   /* Go by vnum, less chance of screwups. -- Altrag */
   for (corpse = first_object; corpse; corpse = corpse->next)
      if (corpse->pIndexData->vnum == OBJ_VNUM_CORPSE_PC && corpse->in_room != NULL && !str_cmp(corpse->short_descr + 14, name) && objrem != corpse)
      {
         if (!fp)
         {
            char buf[127];

            sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
            if (!(fp = fopen(buf, "w")))
            {
               bug("Write_corpses: Cannot open file.", 0);
               perror(buf);
               return;
            }
         }
         fwrite_obj(ch, corpse, fp, 0, OS_CORPSE);
      }
   if (fp)
   {
      fprintf(fp, "#END\n\n");
      fclose(fp);
   }
   else
   {
      char buf[127];

      sprintf(buf, "%s%s", CORPSE_DIR, capitalize(name));
      remove(buf);
   }
   return;
}

void load_corpses(void)
{
   DIR *dp;
   struct dirent *de;
   extern FILE *fpArea;
   extern char strArea[MIL];
   extern int falling;

   if (!(dp = opendir(CORPSE_DIR)))
   {
      bug("Load_corpses: can't open CORPSE_DIR", 0);
      perror(CORPSE_DIR);
      return;
   }

   falling = 1; /* Arbitrary, must be >0 though. */
   while ((de = readdir(dp)) != NULL)
   {
      if (de->d_name[0] != '.')
      {
         sprintf(strArea, "%s%s", CORPSE_DIR, de->d_name);
         fprintf(stderr, "Corpse -> %s\n", strArea);
         if (!(fpArea = fopen(strArea, "r")))
         {
            perror(strArea);
            continue;
         }
         for (;;)
         {
            char letter;
            char *word;

            letter = fread_letter(fpArea);
            if (letter == '*')
            {
               fread_to_eol(fpArea);
               continue;
            }
            if (letter != '#')
            {
               bug("Load_corpses: # not found.", 0);
               break;
            }
            word = fread_word(fpArea);
            if (!strcmp(word, "CORPSE"))
               fread_obj(NULL, fpArea, OS_CORPSE);
            else if (!strcmp(word, "OBJECT"))
               fread_obj(NULL, fpArea, OS_CARRY);
            else if (!strcmp(word, "END"))
               break;
            else
            {
               bug("Load_corpses: bad section.", 0);
               break;
            }
         }
         fclose(fpArea);
      }
   }
   fpArea = NULL;
   strcpy(strArea, "$");
   closedir(dp);
   falling = 0;
   return;
}

void fwrite_stable(FILE * fp, STABLE_DATA * stb)
{
   fprintf(fp, "#STABLE\n");
   fprintf(fp, "Vnum           %d\n", stb->vnum);
   fprintf(fp, "Name           %s~\n", stb->name);
   fprintf(fp, "Short          %s~\n", stb->short_descr);
   fprintf(fp, "Long           %s~\n", stb->long_descr);
   fprintf(fp, "Description    %s~\n", stb->description);
   fprintf(fp, "Endurance      %d\n", stb->level);
   fprintf(fp, "Exp            %d\n", stb->exp);
   fprintf(fp, "Hp             %d\n", stb->hit);
   fprintf(fp, "MaxHp          %d\n", stb->max_hit);
   fprintf(fp, "Move           %d\n", stb->move);
   fprintf(fp, "MaxMove        %d\n", stb->max_move);
   fprintf(fp, "EndMobile\n");
   return;
}

void fwrite_mount(FILE * fp, CHAR_DATA * mob)
{
   if (!IS_NPC(mob) || !fp)
      return;
   fprintf(fp, "#MOUNT\n");
   fprintf(fp, "Vnum	%d\n", mob->pIndexData->vnum);
   if (mob->in_room)
      fprintf(fp, "Room	%d\n", (mob->in_room == get_room_index(ROOM_VNUM_LIMBO) && mob->was_in_room) ? mob->was_in_room->vnum : mob->in_room->vnum);
   if (!QUICKMATCH(mob->name, mob->pIndexData->player_name))
      fprintf(fp, "Name     %s~\n", mob->name);
   if (!QUICKMATCH(mob->short_descr, mob->pIndexData->short_descr))
      fprintf(fp, "Short	%s~\n", mob->short_descr);
   if (!QUICKMATCH(mob->long_descr, mob->pIndexData->long_descr))
      fprintf(fp, "Long	%s~\n", mob->long_descr);
   if (!QUICKMATCH(mob->description, mob->pIndexData->description))
      fprintf(fp, "Description %s~\n", mob->description);
   fprintf(fp, "Position %d\n", mob->position);
   fprintf(fp, "Flags    %s\n", print_bitvector(&mob->act));
   fprintf(fp, "Endurance %d\n", mob->mover);
   fprintf(fp, "Hp       %d\n", mob->hit);
   fprintf(fp, "Maxhp    %d\n", mob->max_hit);
   fprintf(fp, "Move     %d\n", mob->move);
   fprintf(fp, "Maxmove  %d\n", mob->max_move);
   fprintf(fp, "Exp      %d\n", mob->m1);
/* Might need these later --Shaddai
  de_equip_char( mob );
  re_equip_char( mob );
  */
   if (mob->first_carrying)
      fwrite_obj(mob, mob->last_carrying, fp, 0, OS_CARRY);
   fprintf(fp, "EndMobile\n");
   return;
}

STABLE_DATA *fread_stable(FILE * fp)
{
   STABLE_DATA *stb;
   char *word;
   bool fMatch;

   CREATE(stb, STABLE_DATA, 1);

   word = feof(fp) ? "EndMobile" : fread_word(fp);
   if (!strcmp(word, "Vnum"))
      switch (UPPER(word[0]))
      {
         case '*':
            fread_to_eol(fp);
            break;
         case 'V':
            KEY("Vnum", stb->vnum, fread_number(fp));
            break;
      }
   for (;;)
   {
      word = feof(fp) ? "EndMobile" : fread_word(fp);
      fMatch = FALSE;
      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'D':
            KEY("Description", stb->description, fread_string(fp));
            break;
         case 'E':
            if (!strcmp(word, "EndMobile"))
            {
               return stb;
            }
            KEY("Exp", stb->exp, fread_number(fp));
            break;
         case 'H':
            KEY("Hp", stb->hit, fread_number(fp));
            break;
         case 'L':
            KEY("Endurance", stb->level, fread_number(fp));
            KEY("Long", stb->long_descr, fread_string(fp));
            break;
         case 'M':
            KEY("MaxHp", stb->max_hit, fread_number(fp));
            KEY("MaxMove", stb->max_move, fread_number(fp));
            KEY("Move", stb->move, fread_number(fp));
            break;
         case 'N':
            KEY("Name", stb->name, fread_string(fp));
            break;
         case 'S':
            KEY("Short", stb->short_descr, fread_string(fp));
            break;
      }
      if (!fMatch)
      {
         bug("Fread_mobile: no match.", 0);
         bug(word, 0);
      }
   }
   return NULL;
}



void fread_introduction(CHAR_DATA *ch, FILE *fp)
{
   INTRO_DATA *intro;
   char *word;
   bool fMatch;

   CREATE(intro, INTRO_DATA, 1);

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
         case 'F':
            KEY("Flags", intro->flags, fread_number(fp));
            break;
            
         case 'L':
            KEY("Lastseen", intro->lastseen, fread_number(fp));
            break;
            
         case 'P':
            KEY("Pid", intro->pid, fread_number(fp));
            break;
           
         case 'V':
            KEY("Value", intro->value, fread_number(fp));
            break;
            
         case 'E':
            if (!strcmp(word, "End"))
            {
               LINK(intro, ch->pcdata->first_introduction, ch->pcdata->last_introduction, next, prev);
               return;
            }
      }
      if (!fMatch)
      {
         bug("Fread_introduction: no match.", 0);
         bug(word, 0);
      }
   }
}
/*
 * This will read one mobile structure pointer to by fp --Shaddai
 */
CHAR_DATA *fread_mount(FILE * fp)
{
   CHAR_DATA *mob = NULL;
   char *word;
   bool fMatch;
   int inroom = 0;
   ROOM_INDEX_DATA *pRoomIndex = NULL;

   word = feof(fp) ? "EndMobile" : fread_word(fp);
   if (!strcmp(word, "Vnum"))
   {
      int vnum;

      vnum = fread_number(fp);
      mob = create_mobile(get_mob_index(vnum));
      if (!mob)
      {
         for (;;)
         {
            word = feof(fp) ? "EndMobile" : fread_word(fp);
            /* So we don't get so many bug messages when something messes up
             * --Shaddai
             */
            if (!strcmp(word, "EndMobile"))
               break;
         }
         bug("Fread_mobile: No index data for vnum %d", vnum);
         return NULL;
      }
   }
   else
   {
      for (;;)
      {
         word = feof(fp) ? "EndMobile" : fread_word(fp);
         /* So we don't get so many bug messages when something messes up
          * --Shaddai
          */
         if (!strcmp(word, "EndMobile"))
            break;
      }
      extract_char(mob, TRUE);
      bug("Fread_mobile: Vnum not found", 0);
      return NULL;
   }
   for (;;)
   {
      word = feof(fp) ? "EndMobile" : fread_word(fp);
      fMatch = FALSE;
      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
         case '#':
            if (!strcmp(word, "#OBJECT"))
               fread_obj(mob, fp, OS_CARRY);
         case 'D':
            KEY("Description", mob->description, fread_string(fp));
            break;
         case 'E':
            if (!strcmp(word, "EndMobile"))
            {
               if (inroom == 0)
                  inroom = ROOM_VNUM_TEMPLE;
               pRoomIndex = get_room_index(inroom);
               if (!pRoomIndex)
                  pRoomIndex = get_room_index(ROOM_VNUM_TEMPLE);
               char_to_room(mob, pRoomIndex);
               return mob;
            }
            KEY("Endurance", mob->mover, fread_number(fp));
            KEY("Exp", mob->m1, fread_number(fp));
            break;
         case 'F':
            KEY("Flags", mob->act, fread_bitvector(fp));
            break;
         case 'H':
            KEY("Hp", mob->hit, fread_number(fp));
            break;
         case 'L':
            KEY("Long", mob->long_descr, fread_string(fp));
            break;
         case 'M':
            KEY("Maxhp", mob->max_hit, fread_number(fp));
            KEY("Maxmove", mob->max_move, fread_number(fp));
            KEY("Move", mob->move, fread_number(fp));
            break;
         case 'N':
            KEY("Name", mob->name, fread_string(fp));
            break;
         case 'P':
            KEY("Position", mob->position, fread_number(fp));
            break;
         case 'R':
            KEY("Room", inroom, fread_number(fp));
            break;
         case 'S':
            KEY("Short", mob->short_descr, fread_string(fp));
            break;
      }
      if (!fMatch)
      {
         bug("Fread_mobile: no match.", 0);
         bug(word, 0);
      }
   }
   return NULL;
}


/*
 * This will write one mobile structure pointed to be fp --Shaddai
 */

void fwrite_mobile(FILE * fp, CHAR_DATA * mob)
{
   AFFECT_DATA *paf;
   SKILLTYPE *skill = NULL;
   
   if (!IS_NPC(mob) || !fp)
      return;
   fprintf(fp, "#MOBILE\n");
   fprintf(fp, "Vnum	     %d\n", mob->pIndexData->vnum);
   if (mob->in_room)
      fprintf(fp, "Room	        %d\n", (mob->in_room == get_room_index(ROOM_VNUM_LIMBO) && mob->was_in_room) ? mob->was_in_room->vnum : mob->in_room->vnum);
   if (!QUICKMATCH(mob->name, mob->pIndexData->player_name))
      fprintf(fp, "Name         %s~\n", mob->name);
   if (!QUICKMATCH(mob->short_descr, mob->pIndexData->short_descr))
      fprintf(fp, "Short        %s~\n", mob->short_descr);
   if (!QUICKMATCH(mob->long_descr, mob->pIndexData->long_descr))
      fprintf(fp, "Long	        %s~\n", mob->long_descr);
   if (!QUICKMATCH(mob->description, mob->pIndexData->description))
      fprintf(fp, "Description  %s~\n", mob->description);
   fprintf(fp, "Position     %d\n", mob->position);
   fprintf(fp, "Flags        %s\n", print_bitvector(&mob->act));
   //Added for quest saving
   fprintf(fp, "Race         %d\n", mob->race);
   fprintf(fp, "Sex          %d\n", mob->sex);
   fprintf(fp, "Gold         %d\n", mob->gold );
   fprintf(fp, "HpManaMove   %d %d %d %d %d %d\n", mob->hit, mob->max_hit, mob->mana, mob->max_mana, mob->move, mob->max_move );
   fprintf(fp, "Stats        %d %d %d %d %d %d\n", mob->perm_str, mob->perm_int, mob->perm_wis, mob->perm_lck, mob->perm_dex, mob->perm_agi);
   fprintf(fp, "Limbs        %d %d %d %d\n", mob->con_rarm, mob->con_larm, mob->con_rleg, mob->con_lleg);
   fprintf(fp, "ACTohit      %d %d %d %d\n", mob->tohitbash, mob->tohitslash, mob->tohitstab, mob->armor);
   fprintf(fp, "Attacks      %s\n", print_bitvector(&mob->attacks));
   fprintf(fp, "Defenses     %s\n", print_bitvector(&mob->defenses));
   fprintf(fp, "Immune       %d\n", mob->immune);
   fprintf(fp, "Resistant    %d\n", mob->resistant);
   fprintf(fp, "Susc         %d\n", mob->susceptible);
   for ( paf = mob->first_affect; paf; paf = paf->next )
   {
      if ( paf->type >= 0 && (skill=get_skilltype(paf->type)) == NULL )
	     continue;

	  if ( paf->type >= 0 && paf->type < TYPE_PERSONAL )
	  {
	     fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n",  skill->name, paf->duration, paf->modifier, paf->location, 
	        print_bitvector(&paf->bitvector) );
	  }
	  else
	  {
	     fprintf( fp, "Affect       %3d %3d %3d %3d %s\n", paf->type, paf->duration, paf->modifier, paf->location, 
	        print_bitvector(&paf->bitvector) );
	  }
   }
/* Might need these later --Shaddai
  de_equip_char( mob );
  re_equip_char( mob );
  */
   if (mob->first_carrying)
      fwrite_obj(mob, mob->last_carrying, fp, 0, OS_CARRY);
   fprintf(fp, "EndMobile\n");
   return;
}

/*
 * This will read one mobile structure pointer to by fp --Shaddai
 */
CHAR_DATA *fread_mobile(FILE * fp)
{
   CHAR_DATA *mob = NULL;
   char *word;
   bool fMatch;
   int inroom = 0;
   ROOM_INDEX_DATA *pRoomIndex = NULL;

   word = feof(fp) ? "EndMobile" : fread_word(fp);
   if (!strcmp(word, "Vnum"))
   {
      int vnum;

      vnum = fread_number(fp);
      mob = create_mobile(get_mob_index(vnum));
      if (!mob)
      {
         for (;;)
         {
            word = feof(fp) ? "EndMobile" : fread_word(fp);
            /* So we don't get so many bug messages when something messes up
             * --Shaddai
             */
            if (!strcmp(word, "EndMobile"))
               break;
         }
         bug("Fread_mobile: No index data for vnum %d", vnum);
         return NULL;
      }
   }
   else
   {
      for (;;)
      {
         word = feof(fp) ? "EndMobile" : fread_word(fp);
         /* So we don't get so many bug messages when something messes up
          * --Shaddai
          */
         if (!strcmp(word, "EndMobile"))
            break;
      }
      extract_char(mob, TRUE);
      bug("Fread_mobile: Vnum not found", 0);
      return NULL;
   }
   for (;;)
   {
      word = feof(fp) ? "EndMobile" : fread_word(fp);
      fMatch = FALSE;
      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;
         case '#':
            if (!strcmp(word, "#OBJECT"))
            {
               fread_obj(mob, fp, OS_CARRY);
               fMatch = TRUE;
            }
            break;
         case 'A':
	        if ( !str_cmp( word, "Affect" ) || !str_cmp( word, "AffectData" ) )
	        {
   	           AFFECT_DATA *paf;

               CREATE( paf, AFFECT_DATA, 1 );
               if ( !str_cmp( word, "Affect" ) )
               {
                  paf->type   = fread_number( fp );
               }
               else
               {
                  int sn;
                  char *sname = fread_word(fp);

                  if ( (sn=skill_lookup(sname)) < 0 )
                  {
                     if ( (sn=herb_lookup(sname)) < 0 )
                         bug( "%s", "load_mobile: unknown skill." );
                     else
                         sn += TYPE_HERB;
                  }
                  paf->type = sn;
               }

               paf->duration = fread_number( fp );
               paf->modifier = fread_number( fp );
               paf->location = fread_number( fp );
               if ( paf->location == APPLY_WEAPONSPELL || paf->location == APPLY_WEARSPELL || paf->location == APPLY_REMOVESPELL
               || paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL)
                  paf->modifier   = slot_lookup( paf->modifier );
               paf->bitvector    = fread_bitvector( fp );
               LINK( paf, mob->first_affect, mob->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            if (!str_cmp(word, "ACTohit"))
            {
               mob->tohitbash = fread_number(fp);
               mob->tohitslash = fread_number(fp);
               mob->tohitstab = fread_number(fp);
               mob->armor = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            KEY( "AffectedBy",    mob->affected_by,   fread_bitvector( fp ) );
            KEY("Attacks", mob->attacks, fread_bitvector(fp));
            break;
         case 'D':
            KEY("Defenses", mob->defenses, fread_bitvector(fp));
            KEY("Description", mob->description, fread_string(fp));
            break;
         case 'E':
            if (!strcmp(word, "EndMobile"))
            {
               if (inroom == 0)
                  inroom = ROOM_VNUM_TEMPLE;
               pRoomIndex = get_room_index(inroom);
               if (!pRoomIndex)
                  pRoomIndex = get_room_index(ROOM_VNUM_TEMPLE);
               char_to_room(mob, pRoomIndex);
               mob->apply_res_fire[0] = 100;
               mob->apply_res_water[0] = 100;
               mob->apply_res_air[0] = 100;
               mob->apply_res_earth[0] = 100;
               mob->apply_res_energy[0] = 100;
               mob->apply_res_magic[0] = 100;
               mob->apply_res_nonmagic[0] = 100;
               mob->apply_res_blunt[0] = 100;
               mob->apply_res_pierce[0] = 100;
               mob->apply_res_slash[0] = 100;
               mob->apply_res_poison[0] = 100;
               mob->apply_res_paralysis[0] = 100;
               mob->apply_res_holy[0] = 100;
               mob->apply_res_unholy[0] = 100;
               mob->apply_res_undead[0] = 100; 
               return mob;
            }
            break;
         case 'F':
            KEY("Flags", mob->act, fread_bitvector(fp));
            break;
         case 'G':
            KEY( "Gold", mob->gold, fread_number(fp));
	 	    break;
	 	 case 'H':
            if ( !str_cmp( word, "HpManaMove" ) )
            {
               mob->hit = fread_number( fp );
               mob->max_hit = fread_number( fp );
               mob->mana = fread_number( fp );
               mob->max_mana = fread_number( fp );
               mob->move = fread_number( fp );
               mob->max_move = fread_number( fp );

               fMatch = TRUE;
            }
            break;
         case 'I':
            KEY("Immune", mob->immune, fread_number(fp));
         case 'L':
            if (!str_cmp(word, "Limbs"))
            {
               mob->con_rarm = fread_number(fp);
               mob->con_larm = fread_number(fp);
               mob->con_rleg = fread_number(fp);
               mob->con_lleg = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            KEY("Long", mob->long_descr, fread_string(fp));
            break;
         case 'N':
            KEY("Name", mob->name, fread_string(fp));
            break;
         case 'P':
            KEY("Position", mob->position, fread_number(fp));
            break;
         case 'R':
            KEY("Race", mob->race, fread_number(fp));
            KEY("Resistant", mob->resistant, fread_number(fp));
            KEY("Room", inroom, fread_number(fp));
            break;
         case 'S':
            KEY("Sex", mob->sex, fread_number(fp));
            KEY("Short", mob->short_descr, fread_string(fp));
            if (!str_cmp(word, "Stats"))
            {
               mob->perm_str = fread_number(fp);
               mob->perm_int = fread_number(fp);
               mob->perm_wis = fread_number(fp);
               mob->perm_lck = fread_number(fp);
               mob->perm_dex = fread_number(fp);
               mob->perm_agi = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            KEY("Susc", mob->susceptible, fread_number(fp));
            break;
      }
      if (!fMatch)
      {
         bug("Fread_mobile: no match.", 0);
         bug(word, 0);
      }
   }
   return NULL;
}

/*
 * This will write in the saved mobile for a char --Shaddai
 */
void write_char_mobile(CHAR_DATA * ch, char *argument)
{
   FILE *fp;
   CHAR_DATA *mob;
   char buf[MSL];

   if (IS_NPC(ch) || !ch->pcdata->pet)
      return;

   fclose(fpReserve);
   if ((fp = fopen(argument, "w")) == NULL)
   {
      sprintf(buf, "Write_char_mobile: couldn't open %s for writing!\n\r", argument);
      bug(buf, 0);
      fpReserve = fopen(NULL_FILE, "r");
      return;
   }
   mob = ch->pcdata->pet;
   xSET_BIT(mob->affected_by, AFF_CHARM);
   fwrite_mobile(fp, mob);
   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

/*
 * This will read in the saved mobile for a char --Shaddai
 */

void read_char_mobile(char *argument)
{
   FILE *fp;
   CHAR_DATA *mob;
   char buf[MSL];

   fclose(fpReserve);
   if ((fp = fopen(argument, "r")) == NULL)
   {
      sprintf(buf, "Read_char_mobile: couldn't open %s for reading!\n\r", argument);
      bug(buf, 0);
      fpReserve = fopen(NULL_FILE, "r");
      return;
   }
   mob = fread_mobile(fp);
   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}
