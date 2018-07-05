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
 *			     Spell handling module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#ifdef sun
#include <strings.h>
#endif
#include <time.h>
#include "mud.h"


/*
 * Local functions.
 */
void say_spell args((CHAR_DATA * ch, int sn));

/*
CHAR_DATA *make_poly_mob args( (CHAR_DATA *ch, int vnum) );
*/
ch_ret spell_affect args((int sn, int level, CHAR_DATA * ch, void *vo));
ch_ret spell_affectchar args((int sn, int level, CHAR_DATA * ch, void *vo));
int dispel_casting(AFFECT_DATA * paf, CHAR_DATA * ch, CHAR_DATA * victim, int affect, bool dispel);


//Othertypes is used for acutal resist flag checks.  Set to -1 to disable
/*
 * Is immune to a damage type
 */
bool is_immune(CHAR_DATA * ch, sh_int damtype, int othertypes)
{
   if (othertypes >= 0)
   {
      if (IS_SET(ch->immune, othertypes))
         return TRUE;
      if (othertypes == RIS_MAGIC)
         if (ch->apply_res_magic[0] == -1)
            return TRUE;
      if (othertypes == RIS_NONMAGIC)
         if (ch->apply_res_nonmagic[0] == -1)
            return TRUE;
      if (othertypes == RIS_SLEEP)
         if (ch->apply_res_nonmagic[0] == -1)
            return TRUE;
      return FALSE;
   }
   switch (damtype)
   {
      case SD_FIRE:
         if (IS_SET(ch->immune, RIS_FIRE) || ch->apply_res_fire[0] == -500)
            return TRUE;
         break;
      case SD_WATER:
         if (IS_SET(ch->immune, RIS_WATER) || ch->apply_res_water[0] == -500)
            return TRUE;
         break;
      case SD_EARTH:
         if (IS_SET(ch->immune, RIS_EARTH) || ch->apply_res_earth[0] == -500)
            return TRUE;
         break;
      case SD_ENERGY:
         if (IS_SET(ch->immune, RIS_ENERGY) || ch->apply_res_energy[0] == -500)
            return TRUE;
         break;
      case SD_AIR:
         if (IS_SET(ch->immune, RIS_AIR) || ch->apply_res_air[0] == -500)
            return TRUE;
         break;
      case SD_HOLY:
         if (IS_SET(ch->immune, RIS_HOLY) || ch->apply_res_holy[0] == -500)
            return TRUE;
         break;
      case SD_UNHOLY:
         if (IS_SET(ch->immune, RIS_UNHOLY) || ch->apply_res_unholy[0] == -500)
            return TRUE;
         break;
      case SD_UNDEAD:
         if (IS_SET(ch->immune, RIS_UNDEAD) || ch->apply_res_undead[0] == -500)
            return TRUE;
         break;
   }
   return FALSE;
}

/*
 * Lookup a skill by name, only stopping at skills the player has.
 */
int ch_slookup(CHAR_DATA * ch, const char *name)
{
   int sn;

   if (IS_NPC(ch))
      return skill_lookup(name);
   for (sn = 0; sn < top_sn; sn++)
   {
      if (!skill_table[sn]->name)
         break;
      if (ch->pcdata->learned[sn] > 0
         && ch->pcdata->ranking[sn] > 0 && LOWER(name[0]) == LOWER(skill_table[sn]->name[0]) && !str_prefix(name, skill_table[sn]->name))
         return sn;
   }

   return -1;
}

/*
 * Lookup an herb by name.
 */
int herb_lookup(const char *name)
{
   int sn;

   for (sn = 0; sn < top_herb; sn++)
   {
      if (!herb_table[sn] || !herb_table[sn]->name)
         return -1;
      if (LOWER(name[0]) == LOWER(herb_table[sn]->name[0]) && !str_prefix(name, herb_table[sn]->name))
         return sn;
   }
   return -1;
}

/*
 * Lookup a personal skill
 * Unused for now.  In place to allow a player to have a custom spell/skill.
 * When this is put in make sure you put in cleanup code if you do any
 * sort of allocating memory in free_char --Shaddai
 */
int personal_lookup(CHAR_DATA * ch, const char *name)
{
   int sn;

   if (!ch->pcdata)
      return -1;
   for (sn = 0; sn < MAX_PERSONAL; sn++)
   {
      if (!ch->pcdata->special_skills[sn] || !ch->pcdata->special_skills[sn]->name)
         return -1;
      if (LOWER(name[0]) == LOWER(ch->pcdata->special_skills[sn]->name[0]) && !str_prefix(name, ch->pcdata->special_skills[sn]->name))
         return sn;
   }
   return -1;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup(const char *name)
{
   int sn;

   if ((sn = bsearch_skill_exact(name, gsn_first_spell, gsn_first_skill - 1)) == -1)
      if ((sn = bsearch_skill_exact(name, gsn_first_skill, gsn_first_weapon - 1)) == -1)
         if ((sn = bsearch_skill_exact(name, gsn_first_weapon, gsn_first_tongue - 1)) == -1)
            if ((sn = bsearch_skill_exact(name, gsn_first_tongue, gsn_top_sn - 1)) == -1)
               if ((sn = bsearch_skill_prefix(name, gsn_first_spell, gsn_first_skill - 1)) == -1)
                  if ((sn = bsearch_skill_prefix(name, gsn_first_skill, gsn_first_weapon - 1)) == -1)
                     if ((sn = bsearch_skill_prefix(name, gsn_first_weapon, gsn_first_tongue - 1)) == -1)
                        if ((sn = bsearch_skill_prefix(name, gsn_first_tongue, gsn_top_sn - 1)) == -1 && gsn_top_sn < top_sn)
                        {
                           for (sn = gsn_top_sn; sn < top_sn; sn++)
                           {
                              if (!skill_table[sn] || !skill_table[sn]->name)
                                 return -1;
                              if (LOWER(name[0]) == LOWER(skill_table[sn]->name[0]) && !str_prefix(name, skill_table[sn]->name))
                                 return sn;
                           }
                           return -1;
                        }
   return sn;
}

/*
 * Return a skilltype pointer based on sn			-Thoric
 * Returns NULL if bad, unused or personal sn.
 */
SKILLTYPE *get_skilltype(int sn)
{
   if (sn >= TYPE_PERSONAL)
      return NULL;
   if (sn >= TYPE_HERB)
      return IS_VALID_HERB(sn - TYPE_HERB) ? herb_table[sn - TYPE_HERB] : NULL;
   if (sn >= TYPE_HIT)
      return NULL;
   return IS_VALID_SN(sn) ? skill_table[sn] : NULL;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for prefix matches
 */
int bsearch_skill_prefix(const char *name, int first, int top)
{
   int sn;

   for (;;)
   {
      sn = (first + top) >> 1;

      if (LOWER(name[0]) == LOWER(skill_table[sn]->name[0]) && !str_prefix(name, skill_table[sn]->name))
         return sn;
      if (first >= top)
         return -1;
      if (strcmp(name, skill_table[sn]->name) < 1)
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for exact matches only
 */
int bsearch_skill_exact(const char *name, int first, int top)
{
   int sn;

   for (;;)
   {
      sn = (first + top) >> 1;
      if (!str_cmp(name, skill_table[sn]->name))
         return sn;
      if (first >= top)
         return -1;
      if (strcmp(name, skill_table[sn]->name) < 1)
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check exact match first, then a prefix match
 */
int bsearch_skill(const char *name, int first, int top)
{
   int sn = bsearch_skill_exact(name, first, top);

   return (sn == -1) ? bsearch_skill_prefix(name, first, top) : sn;
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
int ch_bsearch_skill_prefix(CHAR_DATA * ch, const char *name, int first, int top)
{
   int sn;

   for (;;)
   {
      sn = (first + top) >> 1;

      if (LOWER(name[0]) == LOWER(skill_table[sn]->name[0])
         && !str_prefix(name, skill_table[sn]->name) && ch->pcdata->learned[sn] > 0 && ch->pcdata->ranking[sn] > 0)
         return sn;
      if (first >= top)
         return -1;
      if (strcmp(name, skill_table[sn]->name) < 1)
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}

int ch_bsearch_skill_exact(CHAR_DATA * ch, const char *name, int first, int top)
{
   int sn;

   for (;;)
   {
      sn = (first + top) >> 1;

      if (!str_cmp(name, skill_table[sn]->name) && ch->pcdata->learned[sn] > 0 && ch->pcdata->ranking[sn] > 0)
         return sn;
      if (first >= top)
         return -1;
      if (strcmp(name, skill_table[sn]->name) < 1)
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}

int ch_bsearch_skill(CHAR_DATA * ch, const char *name, int first, int top)
{
   int sn = ch_bsearch_skill_exact(ch, name, first, top);

   return (sn == -1) ? ch_bsearch_skill_prefix(ch, name, first, top) : sn;
}

int find_spell(CHAR_DATA * ch, const char *name, bool know)
{
   if (IS_NPC(ch) || !know)
      return bsearch_skill(name, gsn_first_spell, gsn_first_skill - 1);
   else
      return ch_bsearch_skill(ch, name, gsn_first_spell, gsn_first_skill - 1);
}

int find_skill(CHAR_DATA * ch, const char *name, bool know)
{
   if (IS_NPC(ch) || !know)
      return bsearch_skill(name, gsn_first_skill, gsn_first_weapon - 1);
   else
      return ch_bsearch_skill(ch, name, gsn_first_skill, gsn_first_weapon - 1);
}

int find_weapon(CHAR_DATA * ch, const char *name, bool know)
{
   if (IS_NPC(ch) || !know)
      return bsearch_skill(name, gsn_first_weapon, gsn_first_tongue - 1);
   else
      return ch_bsearch_skill(ch, name, gsn_first_weapon, gsn_first_tongue - 1);
}

int find_tongue(CHAR_DATA * ch, const char *name, bool know)
{
   if (IS_NPC(ch) || !know)
      return bsearch_skill(name, gsn_first_tongue, gsn_top_sn - 1);
   else
      return ch_bsearch_skill(ch, name, gsn_first_tongue, gsn_top_sn - 1);
}


/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
int slot_lookup(int slot)
{
   extern bool fBootDb;
   int sn;

   if (slot <= 0)
      return -1;

   for (sn = 0; sn < top_sn; sn++)
      if (slot == skill_table[sn]->slot)
         return sn;

   if (fBootDb)
   {
      bug("Slot_lookup: bad slot %d.", slot);
      return -1;
   }

   return -1;
}

/*
 * Handler to tell the victim which spell is being affected.
 * Shaddai
 */
int dispel_casting(AFFECT_DATA * paf, CHAR_DATA * ch, CHAR_DATA * victim, int affect, bool dispel)
{
   char buf[MSL];
   char *spell;
   SKILLTYPE *sktmp;
   int has_detect = FALSE;
   EXT_BV ext_bv = meb(affect);

   if (IS_AFFECTED(ch, AFF_DETECT_MAGIC))
      has_detect = TRUE;

   if (paf)
   {
      if ((sktmp = get_skilltype(paf->type)) == NULL)
         return 0;
      spell = sktmp->name;
   }
   else
      spell = affect_bit_name(&ext_bv);

   set_char_color(AT_MAGIC, ch);
   set_char_color(AT_HITME, victim);

   if (!can_see(ch, victim))
      strcpy(buf, "Someone");
   else
   {
      strcpy(buf, (IS_NPC(victim) ? victim->short_descr : PERS_MAP(victim, ch)));
      buf[0] = toupper(buf[0]);
   }

   if (dispel)
   {
         return 0; /* So we give the default Ok. Message */
   }
   else
   {
         return 0; /* The wonderful Failed. Message */
   }
   return 1;
}

/*
 * Fancy message handling for a successful casting		-Thoric
 */
void successful_casting(SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj)
{
   sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
   sh_int chit = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
   sh_int chitme = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

   if (skill->target != TAR_CHAR_OFFENSIVE)
   {
      chit = chitroom;
      chitme = chitroom;
   }

   if (ch && ch != victim)
   {
      if (skill->hit_char && skill->hit_char[0] != '\0')
         act(chit, skill->hit_char, ch, obj, victim, TO_CHAR);
      else if (skill->type == SKILL_SPELL)
         act(chit, "Ok.", ch, NULL, NULL, TO_CHAR);
   }
   if (ch && skill->hit_room && skill->hit_room[0] != '\0')
      act(chitroom, skill->hit_room, ch, obj, victim, TO_NOTVICT);
   if (ch && victim && skill->hit_vict && skill->hit_vict[0] != '\0')
   {
      if (ch != victim)
         act(chitme, skill->hit_vict, ch, obj, victim, TO_VICT);
      else
         act(chitme, skill->hit_vict, ch, obj, victim, TO_CHAR);
   }
   else if (ch && ch == victim && skill->type == SKILL_SPELL)
      act(chitme, "Ok.", ch, NULL, NULL, TO_CHAR);
}

/*
 * Fancy message handling for a failed casting			-Thoric
 */
void failed_casting(SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj)
{
   sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
   sh_int chit = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
   sh_int chitme = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

   if (skill->target != TAR_CHAR_OFFENSIVE)
   {
      chit = chitroom;
      chitme = chitroom;
   }

   if (ch && ch != victim)
   {
      if (skill->miss_char && skill->miss_char[0] != '\0')
         act(chit, skill->miss_char, ch, obj, victim, TO_CHAR);
      else if (skill->type == SKILL_SPELL)
         act(chit, "You failed.", ch, NULL, NULL, TO_CHAR);
   }
   if (ch && skill->miss_room && skill->miss_room[0] != '\0' && str_cmp(skill->miss_room, "supress"))
      act(chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT);
   if (ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0')
   {
      if (ch != victim)
         act(chitme, skill->miss_vict, ch, obj, victim, TO_VICT);
      else
         act(chitme, skill->miss_vict, ch, obj, victim, TO_CHAR);
   }
   else if (ch && ch == victim)
   {
      if (skill->miss_char && skill->miss_char[0] != '\0')
         act(chitme, skill->miss_char, ch, obj, victim, TO_CHAR);
      else if (skill->type == SKILL_SPELL)
         act(chitme, "You failed.", ch, NULL, NULL, TO_CHAR);
   }
}

/*
 * Fancy message handling for being immune to something		-Thoric
 */
void immune_casting(SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj)
{
   sh_int chitroom = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION);
   sh_int chit = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT);
   sh_int chitme = (skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME);

   if (skill->target != TAR_CHAR_OFFENSIVE)
   {
      chit = chitroom;
      chitme = chitroom;
   }

   if (ch && ch != victim)
   {
      if (skill->imm_char && skill->imm_char[0] != '\0')
         act(chit, skill->imm_char, ch, obj, victim, TO_CHAR);
      else if (skill->miss_char && skill->miss_char[0] != '\0')
         act(chit, skill->hit_char, ch, obj, victim, TO_CHAR);
      else if (skill->type == SKILL_SPELL || skill->type == SKILL_SKILL)
         act(chit, "That appears to have no effect.", ch, NULL, NULL, TO_CHAR);
   }
   if (ch && skill->imm_room && skill->imm_room[0] != '\0')
      act(chitroom, skill->imm_room, ch, obj, victim, TO_NOTVICT);
   else if (ch && skill->miss_room && skill->miss_room[0] != '\0')
      act(chitroom, skill->miss_room, ch, obj, victim, TO_NOTVICT);
   if (ch && victim && skill->imm_vict && skill->imm_vict[0] != '\0')
   {
      if (ch != victim)
         act(chitme, skill->imm_vict, ch, obj, victim, TO_VICT);
      else
         act(chitme, skill->imm_vict, ch, obj, victim, TO_CHAR);
   }
   else if (ch && victim && skill->miss_vict && skill->miss_vict[0] != '\0')
   {
      if (ch != victim)
         act(chitme, skill->miss_vict, ch, obj, victim, TO_VICT);
      else
         act(chitme, skill->miss_vict, ch, obj, victim, TO_CHAR);
   }
   else if (ch && ch == victim)
   {
      if (skill->imm_char && skill->imm_char[0] != '\0')
         act(chit, skill->imm_char, ch, obj, victim, TO_CHAR);
      else if (skill->miss_char && skill->miss_char[0] != '\0')
         act(chit, skill->hit_char, ch, obj, victim, TO_CHAR);
      else if (skill->type == SKILL_SPELL || skill->type == SKILL_SKILL)
         act(chit, "That appears to have no affect.", ch, NULL, NULL, TO_CHAR);
   }
}


/*
 * Utter mystical words for an sn.
 */
void say_spell(CHAR_DATA * ch, int sn)
{
   char buf[MSL];
   char buf2[MSL];
   CHAR_DATA *rch;
   char *pName;
   int iSyl;
   int length;
   SKILLTYPE *skill = get_skilltype(sn);

   struct syl_type
   {
      char *old;
      char *new;
   };

   static const struct syl_type syl_table[] = {
      {" ", " "},
      {"ar", "abra"},
      {"au", "kada"},
      {"bless", "fido"},
      {"blind", "nose"},
      {"bur", "mosa"},
      {"cu", "judi"},
      {"de", "oculo"},
      {"en", "unso"},
      {"light", "dies"},
      {"lo", "hi"},
      {"mor", "zak"},
      {"move", "sido"},
      {"ness", "lacri"},
      {"ning", "illa"},
      {"per", "duda"},
      {"polymorph", "iaddahs"},
      {"ra", "gru"},
      {"re", "candus"},
      {"son", "sabru"},
      {"tect", "infra"},
      {"tri", "cula"},
      {"ven", "nofo"},
      {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
      {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
      {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
      {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
      {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
      {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
      {"y", "l"}, {"z", "k"},
      {"", ""}
   };

   buf[0] = '\0';
   for (pName = skill->name; *pName != '\0'; pName += length)
   {
      for (iSyl = 0; (length = strlen(syl_table[iSyl].old)) != 0; iSyl++)
      {
         if (!str_prefix(syl_table[iSyl].old, pName))
         {
            strcat(buf, syl_table[iSyl].new);
            break;
         }
      }

      if (length == 0)
         length = 1;
   }

   sprintf(buf2, "$n utters the words, '%s'.", buf);
   sprintf(buf, "$n utters the words, '%s'.", skill->name);

   for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
   {
      if (rch != ch && rch->coord->x == ch->coord->x && rch->coord->y == ch->coord->y
         && ch->map == rch->map)
      {
         act(AT_MAGIC, buf2, ch, NULL, rch, TO_VICT);
      }
   }

   return;
}


/*
 * Make adjustments to saving throw based in RIS		-Thoric
 */
int ris_save(CHAR_DATA * ch, int chance, int ris)
{   
   sh_int modifier = 0;
   
   if (ris == RIS_FIRE)
      modifier = ch->apply_res_fire[0];
   if (ris == RIS_WATER)
      modifier = ch->apply_res_water[0];
   if (ris == RIS_EARTH)
      modifier = ch->apply_res_earth[0];
   if (ris == RIS_ENERGY)
      modifier = ch->apply_res_energy[0];
   if (ris == RIS_AIR)
      modifier = ch->apply_res_air[0];
   if (ris == RIS_NONMAGIC)
      modifier = ch->apply_res_nonmagic[0];
   if (ris == RIS_MAGIC)
      modifier = ch->apply_res_magic[0];
   if (ris == RIS_BLUNT)
      modifier = ch->apply_res_blunt[0];
   if (ris == RIS_PIERCE)
      modifier = ch->apply_res_pierce[0];
   if (ris == RIS_SLASH)
      modifier = ch->apply_res_slash[0];
   if (ris == RIS_PARALYSIS)
      modifier = ch->apply_res_paralysis[0];
   if (ris == RIS_POISON)
      modifier = ch->apply_res_poison[0];
   if (ris == RIS_UNHOLY)
      modifier = ch->apply_res_unholy[0];
   if (ris == RIS_HOLY)
      modifier = ch->apply_res_holy[0];

   if (modifier >= -499 && modifier <= 0)
      modifier = 1;
   if (modifier == -500)
      return 1000;

   if (IS_SET(ch->immune, ris) && !IS_SET(ch->no_immune, ris))
      return 1000;
   if (IS_SET(ch->resistant, ris) && !IS_SET(ch->no_resistant, ris))
      modifier -= 30;
   if (IS_SET(ch->susceptible, ris) && !IS_SET(ch->no_susceptible, ris))
      modifier += 50;
   return (chance * modifier) / 100;
}


/*								    -Thoric
 * Fancy dice expression parsing complete with order of operations,
 * simple exponent support, dice support as well as a few extra
 * variables: L = level, H = hp, M = mana, V = move, S = str, X = dex
 *            I = int, W = wis, C = con, A = cha, U = luck, A = age
 *            R = Ranking/Mastery (New) P = Skill Points (New)
              B - Point based Level (New)
 * Used for spell dice parsing, ie: 3d8+L-6
 *
 */
 
/* Usage Notes because I spent too much time fucking with this thing
 * Parses in correct order, will do * before +, etc
 * If you plan on using { or } make sure to put the comparing arguments
 * in ( ), ex: (2d3+b*2)}15 
 * Do not use spaces
 */
int rd_parse(CHAR_DATA * ch, int level, char *exp, int sn)
{
   int x, lop = 0, gop = 0, eop = 0;
   char operation;
   char *sexp[2];
   int total = 0, len = 0;
   int isobj = 0;
   int tlevel;

   if (level >= SPOWER_MIN)
   {
      isobj = 1; //mob or obj now, no difference
      tlevel = level;
   }
   else
      tlevel = level;

    /* take care of nulls coming in */
    if (!exp || !strlen(exp))
	return 0;

    /* get rid of brackets if they surround the entire expresion */
    if ( (*exp == '(') && !index(exp+1,'(') && exp[strlen(exp)-1] == ')' )
    {
	exp[strlen(exp)-1] = '\0';
	exp++;
    }

    /* check if the expresion is just a number */
    len = strlen(exp);
    if ( len == 1 && isalpha(exp[0]) )
    {
	switch(exp[0])
	{
	    case 'L': case 'l':	return level;
	    case 'H': case 'h':	return ch->hit;
	    case 'M': case 'm':	return ch->mana;
	    case 'V': case 'v':	return ch->move;
	    case 'S': case 's':	return get_curr_str(ch);
	    case 'I': case 'i':	return get_curr_int(ch);
	    case 'W': case 'w':	return get_curr_wis(ch);
	    case 'X': case 'x':	return get_curr_dex(ch);
	    case 'C': case 'c':	return get_curr_con(ch);
	    case 'A': case 'a':	return get_curr_cha(ch);
	    case 'U': case 'u':	return get_curr_lck(ch);
	    case 'Y': case 'y':	return get_age(ch);
	    case 'R': case 'r': 
	       if (isobj)
	          return GET_MASTERY(ch, sn, isobj, tlevel);
	       else 
	          return MASTERED(ch, sn);
        case 'P': case 'p': 
           if (isobj)
              return GET_POINTS(ch, sn, isobj, tlevel);
           else
              return LEARNED(ch, sn);        
 
        case 'B': case 'b': 
           if (isobj)
              return POINT_LEVEL(GET_POINTS(ch, sn, isobj, tlevel), GET_MASTERY(ch, sn, isobj, tlevel));
           else
              return POINT_LEVEL(LEARNED(ch, sn), MASTERED(ch, sn));
	}
    }

    for (x = 0; x < len; ++x)
	if (!isdigit(exp[x]) && !isspace(exp[x]))
	    break;
    if (x == len)
	return atoi(exp);
  
    /* break it into 2 parts */
    for (x = 0; x < strlen(exp); ++x)
	switch(exp[x])
	{
	    case '^':
	      if (!total)
		eop = x;
	      break;
	    case '-': case '+':
	      if (!total) 
		lop = x;
	      break;
	    case '*': case '/': case '%': case 'd': case 'D':
	    case '<': case '>': case '{': case '}': case '=':
	      if (!total) 
		gop =  x;
	      break;
	    case '(':
	      ++total;
	      break;
	    case ')':
	      --total;
	      break;
	}
    if (lop)
	x = lop;
    else
    if (gop)
	x = gop;
    else
	x = eop;
    operation = exp[x];
    exp[x] = '\0';
    sexp[0] = exp;
    sexp[1] = (char *)(exp+x+1);

    /* work it out */
    total = rd_parse(ch, level, sexp[0], sn);
    switch(operation)
    {
	case '-':		total -= rd_parse(ch, level, sexp[1], sn);	break;
	case '+':		total += rd_parse(ch, level, sexp[1], sn);	break;
	case '*':		total *= rd_parse(ch, level, sexp[1], sn);	break;
	case '/':		total /= rd_parse(ch, level, sexp[1], sn);	break;
	case '%':		total %= rd_parse(ch, level, sexp[1], sn);	break;
	case 'd': case 'D':	total = dice( total, rd_parse(ch, level, sexp[1], sn) );	break;
	case '<':		total = (total < rd_parse(ch, level, sexp[1], sn));		break;
	case '>':		total = (total > rd_parse(ch, level, sexp[1], sn));		break;
	case '=':		total = (total == rd_parse(ch, level, sexp[1], sn));	break;
	case '{':	        total = UMIN( total, rd_parse(ch, level, sexp[1], sn) );   break;
	case '}':		total = UMAX( total, rd_parse(ch, level, sexp[1], sn) );   break;

	case '^':
	{
	    int y = rd_parse(ch, level, sexp[1], sn), z = total;

	    for (x = 1; x < y; ++x, z *= total);
	    total = z;
	    break;
	}
    }
    return total;
} 

/* wrapper function so as not to destroy exp */
/* sn added for dice checks on mastery rankings -- Xerves 1/00 */
int dice_parse(CHAR_DATA * ch, int level, char *exp, int sn)
{
   char buf[MIL];

   strcpy(buf, exp);
   return rd_parse(ch, level, buf, sn);
}

bool save_chance(CHAR_DATA *ch, int save)
{
   int ms;
   
   ms = 10 - abs(ch->mental_state);
   
   if (ms <= -90)
      ms = -25;
   else if (ms <= -70)
      ms = -15;
   else if (ms <= -50)
      ms = -10;
   else if (ms <= -30)
      ms = -7;
   else if (ms <= -10)
      ms = -5;
   else if (ms <= 0)
      ms = -3;
   else
      ms = 0;
   save = URANGE(5, 50+(save*3/4), 100);
   if ((number_percent() + UMIN(5, (get_curr_lck(ch)-14)/2) + ms) >= save)
      return TRUE;
   else
      return FALSE;
}
/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_poison_death(int level, CHAR_DATA * victim)
{
   int save;
   
   if (is_immune(victim, -1, RIS_MAGIC))
      return TRUE;

   save = level + (victim->saving_poison_death * 5); //negative still better
   return save_chance(victim, save);
}

bool saves_wands(int level, CHAR_DATA * victim)
{
   int save;
   
   if (is_immune(victim, -1, RIS_MAGIC))
      return TRUE;
   
   save = level + (victim->saving_wand * 5);
   return save_chance(victim, save);
}

bool saves_para_petri(int level, CHAR_DATA * victim)
{
   int save;
   
   if (is_immune(victim, -1, RIS_MAGIC))
      return TRUE;

   save = level + (victim->saving_para_petri * 5);
   return save_chance(victim, save);
}

bool saves_breath(int level, CHAR_DATA * victim)
{
   int save;
   
   if (is_immune(victim, -1, RIS_MAGIC))
      return TRUE;

   save = level + (victim->saving_breath * 5);
   return save_chance(victim, save);
}

bool saves_spell_staff(int level, CHAR_DATA * victim)
{
   int save;
   bool saved;

   if (is_immune(victim, -1, RIS_MAGIC))
      return TRUE;

   save = level + (victim->saving_spell_staff * 5);
   saved = save_chance(victim, save);
   return saved;
}


/*
 * Process the spell's required components, if any		-Thoric
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */
bool process_spell_components(CHAR_DATA * ch, int sn)
{
   SKILLTYPE *skill = get_skilltype(sn);
   char *comp = skill->components;
   char *check;
   char arg[MIL];
   bool consume, fail, found;
   int val, value;
   OBJ_DATA *obj;

   /* if no components necessary, then everything is cool */
   if (!comp || comp[0] == '\0')
      return TRUE;

   while (comp[0] != '\0')
   {
      comp = one_argument(comp, arg);
      consume = TRUE;
      fail = found = FALSE;
      val = -1;
      switch (arg[1])
      {
         default:
            check = arg + 1;
            break;
         case '!':
            check = arg + 2;
            fail = TRUE;
            break;
         case '+':
            check = arg + 2;
            consume = FALSE;
            break;
         case '@':
            check = arg + 2;
            val = 0;
            break;
         case '#':
            check = arg + 2;
            val = 1;
            break;
         case '$':
            check = arg + 2;
            val = 2;
            break;
         case '%':
            check = arg + 2;
            val = 3;
            break;
         case '^':
            check = arg + 2;
            val = 4;
            break;
         case '&':
            check = arg + 2;
            val = 5;
            break;
            /*   reserve '*', '(' and ')' for v6, v7 and v8   */
      }
      value = atoi(check);
      obj = NULL;
      switch (UPPER(arg[0]))
      {
         case 'T':
            for (obj = ch->first_carrying; obj; obj = obj->next_content)
               if (obj->item_type == value)
               {
                  if (fail)
                  {
                     send_to_char("Something disrupts the casting of this spell...\n\r", ch);
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'V':
            for (obj = ch->first_carrying; obj; obj = obj->next_content)
               if (obj->pIndexData->vnum == value)
               {
                  if (fail)
                  {
                     send_to_char("Something disrupts the casting of this spell...\n\r", ch);
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'K':
            for (obj = ch->first_carrying; obj; obj = obj->next_content)
               if (nifty_is_name(check, obj->name))
               {
                  if (fail)
                  {
                     send_to_char("Something disrupts the casting of this spell...\n\r", ch);
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'G':
            if (ch->gold >= value)
            {
               if (fail)
               {
                  send_to_char("Something disrupts the casting of this spell...\n\r", ch);
                  return FALSE;
               }
               else
               {
                  if (consume)
                  {
                     set_char_color(AT_GOLD, ch);
                     send_to_char("You feel a little lighter...\n\r", ch);
                     ch->gold -= value;
                  }
                  continue;
               }
            }
            break;
         case 'H':
            if (ch->hit >= value)
            {
               if (fail)
               {
                  send_to_char("Something disrupts the casting of this spell...\n\r", ch);
                  return FALSE;
               }
               else
               {
                  if (consume)
                  {
                     set_char_color(AT_BLOOD, ch);
                     send_to_char("You feel a little weaker...\n\r", ch);
                     ch->hit -= value;
                     update_pos(ch);
                  }
                  continue;
               }
            }
            break;
      }
      /* having this component would make the spell fail... if we get
         here, then the caster didn't have that component */
      if (fail)
         continue;
      if (!found)
      {
         send_to_char("Something is missing...\n\r", ch);
         return FALSE;
      }
      if (obj)
      {
         if (val >= 0 && val < 6)
         {
            separate_obj(obj);
            if (obj->value[val] <= 0)
            {
               act(AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR);
               act(AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM);
               extract_obj(obj);
               return FALSE;
            }
            else if (--obj->value[val] == 0)
            {
               act(AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR);
               act(AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM);
               extract_obj(obj);
            }
            else
               act(AT_MAGIC, "$p glows briefly and a whisp of smoke rises from it.", ch, obj, NULL, TO_CHAR);
         }
         else if (consume)
         {
            separate_obj(obj);
            act(AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR);
            act(AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
         }
         else
         {
            int count = obj->count;

            obj->count = 1;
            act(AT_MAGIC, "$p glows briefly.", ch, obj, NULL, TO_CHAR);
            obj->count = count;
         }
      }
   }
   return TRUE;
}

int pAbort;

/*
 * Locate targets.
 */
void *locate_targets(CHAR_DATA * ch, char *arg, int sn, CHAR_DATA ** victim, OBJ_DATA ** obj)
{
   SKILLTYPE *skill = get_skilltype(sn);
   void *vo = NULL;

   *victim = NULL;
   *obj = NULL;

   switch (skill->target)
   {
      default:
         bug("Do_cast: bad target for sn %d.", sn);
         return &pAbort;

      case TAR_IGNORE:
         break;

      case TAR_CHAR_OFFENSIVE:
         if (arg[0] == '\0')
         {
            if ((*victim = who_fighting(ch)) == NULL)
            {
               send_to_char("Cast the spell on whom?\n\r", ch);
               return &pAbort;
            }
            //There should be some kind of stop here if you aren't in the room ya
            if (*victim)
            {
               if (!IN_SAME_ROOM(ch, *victim))
               {
                  send_to_char("Your target is not in the same room as you!\n\r", ch);
                  return &pAbort;
               }
            }
         }
         else
         {
            if ((*victim = get_char_room_new(ch, arg, 1)) == NULL)
            {
               send_to_char("They aren't here.\n\r", ch);
               return &pAbort;
            }
         }

         /* Offensive spells will choose the ch up to 92% of the time
          * if the nuisance flag is set -- Shaddai 
          */
         if (!IS_NPC(ch) && ch->pcdata->nuisance &&
            ch->pcdata->nuisance->flags > 5 && number_percent() < (((ch->pcdata->nuisance->flags - 5) * 8) + ch->pcdata->nuisance->power * 6))
            *victim = ch;

         if (is_safe(ch, *victim))
            return &pAbort;

         if (ch == *victim)
         {
            if (SPELL_FLAG(get_skilltype(sn), SF_NOSELF))
            {
               send_to_char("You can't cast this on yourself!\n\r", ch);
               return &pAbort;
            }
            send_to_char("Cast this on yourself?  Okay...\n\r", ch);
            /*
               send_to_char( "You can't do that to yourself.\n\r", ch );
               return &pAbort;
             */
         }

         if (!IS_NPC(ch))
         {
            if (!IS_NPC(*victim))
            {
               if (xIS_SET(ch->act, PLR_NICE) && ch != *victim)
               {
                  send_to_char("You are too nice to attack another player.\n\r", ch);
                  return &pAbort;
               }
            }

            if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == *victim)
            {
               send_to_char("You can't do that on your own follower.\n\r", ch);
               return &pAbort;
            }
         }

         check_illegal_pk(ch, *victim);
         vo = (void *) *victim;
         break;

      case TAR_CHAR_DEFENSIVE:
         if (arg[0] == '\0')
            *victim = ch;
         else
         {
            if ((*victim = get_char_room_new(ch, arg, 1)) == NULL)
            {
               send_to_char("They aren't here.\n\r", ch);
               return &pAbort;
            }
         }

         /* Nuisance flag will pick who you are fighting for defensive
          * spells up to 36% of the time -- Shaddai
          */

         if (!IS_NPC(ch) && ch->fighting && ch->pcdata->nuisance &&
            ch->pcdata->nuisance->flags > 5 && number_percent() < (((ch->pcdata->nuisance->flags - 5) * 8) + 6 * ch->pcdata->nuisance->power))
            *victim = who_fighting(ch);

         if (ch == *victim && SPELL_FLAG(get_skilltype(sn), SF_NOSELF))
         {
            send_to_char("You can't cast this on yourself!\n\r", ch);
            return &pAbort;
         }

         vo = (void *) *victim;
         break;

      case TAR_CHAR_SELF:
         if (arg[0] != '\0' && !nifty_is_name(arg, ch->name))
         {
            send_to_char("You cannot cast this spell on another.\n\r", ch);
            return &pAbort;
         }

         vo = (void *) ch;
         break;

      case TAR_OBJ_INV:
         if (arg[0] == '\0')
         {
            send_to_char("What should the spell be cast upon?\n\r", ch);
            return &pAbort;
         }

         if ((*obj = get_obj_carry(ch, arg)) == NULL)
         {
            send_to_char("You are not carrying that.\n\r", ch);
            return &pAbort;
         }

         vo = (void *) *obj;
         break;
         
      case TAR_OBJ_ROOM:
         if (arg[0] == '\0')
         {
            send_to_char("What should the spell be cast upon?\n\r", ch);
            return &pAbort;
         }

         if ((*obj = get_obj_here(ch, arg)) == NULL)
         {
            send_to_char("You are not in the room with that.\n\r", ch);
            return &pAbort;
         }

         vo = (void *) *obj;
         break;
   }

   return vo;
}

void gain_mana_per(CHAR_DATA *ch, CHAR_DATA *victim, int mana)
{
   if (!IS_NPC(ch))
   {
      int mmana;
      int manai = 1;
      
      ch->pcdata->mana_cnt++;
      
      mmana = mana;     
         
      if (ch->perm_int < 11)
         mmana *= .6;
      if (ch->perm_int >= 11 && ch->perm_int <= 13)
         mmana *= .85;
      if (ch->perm_int >= 14 && ch->perm_int <= 16)
         mmana *= 1.05;
      if (ch->perm_int >= 17 && ch->perm_int <= 19)
         mmana *= 1.15;
      if (ch->perm_int >= 20 && ch->perm_int <= 22)
         mmana *= 1.25;
      if (ch->perm_int >= 23 && ch->perm_int < 25)
         mmana *= 1.4;
      if (ch->perm_int == 25)
         mmana *= 1.6;  
    
      if (ch->max_mana < 50)
         mmana *= 1.7;
      else if (ch->max_mana < 100)
         mmana *= 1.3;
      else if (ch->max_mana < 150)
         mmana *= 1;
      else if (ch->max_mana < 200)
         mmana *= .8;
      else if (ch->max_mana < 250)
         mmana *= .6;
      else if (ch->max_mana < 300)
         mmana *= .5;
      else if (ch->max_mana < 350)
         mmana *= .4;
      else if (ch->max_mana < 400)
         mmana *= .3;
      else if (ch->max_mana < 450)
         mmana *= .25;
      else if (ch->max_mana < 500)
         mmana *= .2;
      else if (ch->max_mana < 550)
         mmana *= .1;
      else if (ch->max_mana < 600)
         mmana *= .095;
      else if (ch->max_mana < 650)
         mmana *= .09;
      else if (ch->max_mana < 700)
         mmana *= .08;
      else if (ch->max_mana < 800)
         mmana *= .07;   
      else if (ch->max_mana < 1000)
         mmana *= .06;   
      else if (ch->max_mana < 1500)
         mmana *= .05;   
      else
         mmana *= .04;
        
      if (sysdata.stat_gain <= 1)
         mmana = number_range(mmana*.35, mmana*.45);
      else if (sysdata.stat_gain <= 3)
         mmana = number_range(mmana*.5, mmana*.65);
      else if (sysdata.stat_gain >= 5)
         mmana = number_range(mmana*.75, mmana);
         
      if (ch->pcdata->mana_cnt >= 60)
      {
         send_to_char("&rYour body is starting to weaken from so much casting, might want to stop for awhile.\n\r", ch);
         ch->mana -= UMAX(2, ch->max_mana/30);
         if (ch->mana < 0)
            ch->mana = 0;
         mmana = number_range(-20, -30);
      }
         
      if (mmana == 0)
         mmana = 1;   
      
      if (mmana > 0 && ch->max_mana >= (800+get_talent_increase(ch, 10)) && ch->pcdata->per_mana >= 300)
         mmana = 0;
         
      if (mmana < 0 && ch->max_mana <= 20)
         mmana = 0;
            
      if (victim && IS_NPC(victim) && xIS_SET(victim->act, ACT_MOUNTSAVE))
      {
         if (mmana > 0)
            mmana = 0;
      } 
            
      ch->pcdata->per_mana += mmana;
        
      if (ch->pcdata->per_mana > 1000)
      {
         if (sysdata.stat_gain <= 1)
         {
            if (ch->max_mana < 70)
               manai = number_range(5, 7);
            else if (ch->max_mana < 100)
               manai = number_range(4, 6);
            else if (ch->max_mana < 150)
               manai = number_range(2, 4);
            else if (ch->max_mana < 200)
               manai = number_range(1, 3);
            else
               manai = 1;
         }
         else if (sysdata.stat_gain <= 2)
         {
            if (ch->max_mana < 70)
               manai = number_range(5, 7);
            else if (ch->max_mana < 100)
               manai = number_range(4, 6);
            else if (ch->max_mana < 200)
               manai = number_range(3, 5);
            else if (ch->max_mana < 300)
               manai = number_range(2, 4);
            else
               manai = number_range(1, 2);
         }
         else if (sysdata.stat_gain >= 4)
         {
            if (ch->max_mana < 150)
               manai = number_range(5, 7);
            else if (ch->max_mana < 300)
               manai = number_range(4, 6);
            else if (ch->max_mana < 600)
               manai = number_range(3, 5);
            else if (ch->max_mana < 900)
               manai = number_range(2, 4);
            else
               manai = number_range(1, 2);
         }
         ch->max_mana += manai;
         send_to_char("&B************************************\n\r", ch);
         ch_printf(ch, "&B******You Gain %d Point of Mana******\n\r", manai);
         send_to_char("&B************************************\n\r", ch);
         ch->pcdata->per_mana = 0;
      }  
      if (ch->pcdata->per_mana < 0)
      {
         if (ch->max_mana < 70)
            manai = number_range(5, 7);
         else if (ch->max_mana < 100)
            manai = number_range(4, 6);
         else if (ch->max_mana < 150)
            manai = number_range(2, 4);
         else if (ch->max_mana < 200)
            manai = number_range(1, 3);
         else
            manai = 1;
         ch->max_mana -= manai;
         send_to_char("&b************************************\n\r", ch);
         ch_printf(ch, "&b******You Lose %d Point of Mana******\n\r", manai);
         send_to_char("&b************************************\n\r", ch);
         ch->pcdata->per_mana = 999;
      }     
   }
}


/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
char *ranged_target_name = NULL;


/*
 * Cast a spell.  Multi-caster and component support by Thoric
 */
void do_cast(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   static char staticbuf[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   void *vo = NULL;
   int mana;
   int blood;
   int sn;
   int suc;
   ch_ret retcode;
   bool dont_wait = FALSE;
   OBJ_DATA *tmpobj;
   SKILLTYPE *skill = NULL;
   struct timeval time_used;

   retcode = rNONE;

   switch (ch->substate)
   {
      default:
         /* no ordering charmed mobs to cast spells */

         if (IS_NPC(ch) && (IS_AFFECTED(ch, AFF_CHARM) || IS_AFFECTED(ch, AFF_POSSESS)))
         {
            send_to_char("You can't seem to do that right now...\n\r", ch);
            return;
         }

         if (xIS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) || wIS_SET(ch, ROOM_NO_MAGIC))
         {
            set_char_color(AT_MAGIC, ch);
            send_to_char("You failed.\n\r", ch);
            return;
         }
         if (ch->fighting && (tmpobj = get_eq_char(ch, WEAR_SHIELD)) != NULL && IS_OBJ_STAT(tmpobj, ITEM_TWOHANDED))
         {
            send_to_char("You cannot cast spells in battle if you have a two-handed shield.\n\r", ch);
            return;
         }
         
         if (IS_AFFECTED(ch, AFF_GAGGED))
         {
            send_to_char("Hard to say the words to a spell when you are GAGGED!\n\r", ch);
            return;
         }

         target_name = one_argument(argument, arg1);
         one_argument(target_name, arg2);
         if (ranged_target_name)
            DISPOSE(ranged_target_name);
         ranged_target_name = str_dup(target_name);

         if (arg1[0] == '\0')
         {
            send_to_char("Cast which what where?\n\r", ch);
            return;
         }
         /* Regular mortal spell casting */
         if (get_trust(ch) < LEVEL_IMM) /* Tracker1 */
         {
            if ((sn = find_spell(ch, arg1, TRUE)) < 0 || (!IS_NPC(ch) && (ch->pcdata->ranking[sn] < 1 || ch->pcdata->ranking[sn] > MAX_RANKING)))
            {
               send_to_char("You can't do that.\n\r", ch);
               return;
            }
            if ((skill = get_skilltype(sn)) == NULL)
            {
               send_to_char("You can't do that right now...\n\r", ch);
               return;
            }
         }
         else
            /*
             * Godly "spell builder" spell casting with debugging messages
             */
         {
            if ((sn = skill_lookup(arg1)) < 0)
            {
               send_to_char("We didn't create that yet...\n\r", ch);
               return;
            }
            if (!IS_NPC(ch) && (ch->pcdata->ranking[sn] < 1 || ch->pcdata->ranking[sn] > MAX_RANKING))
            {
               send_to_char("You can't do that.\n\r", ch);
               return;
            }
            if (sn >= MAX_SKILL)
            {
               send_to_char("Hmm... that might hurt.\n\r", ch);
               return;
            }
            if ((skill = get_skilltype(sn)) == NULL)
            {
               send_to_char("Something is severely wrong with that one...\n\r", ch);
               return;
            }
            if (skill->type != SKILL_SPELL)
            {
               send_to_char("That isn't a spell.\n\r", ch);
               return;
            }
            if (!skill->spell_fun)
            {
               send_to_char("We didn't finish that one yet...\n\r", ch);
               return;
            }
         }

         /*
          * Something else removed by Merc   -Thoric
          */
         /* Band-aid alert!  !IS_NPC check -- Blod */
         if (ch->position < skill->minimum_position && !IS_NPC(ch))
         {
            switch (ch->position)
            {
               default:
                  send_to_char("You can't concentrate enough.\n\r", ch);
                  break;
               case POS_SITTING:
                  send_to_char("You can't summon enough energy sitting down.\n\r", ch);
                  break;
               case POS_RESTING:
                  send_to_char("You're too relaxed to cast that spell.\n\r", ch);
                  break;
               case POS_FIGHTING:
                  if (skill->minimum_position <= POS_EVASIVE)
                  {
                     send_to_char("This fighting style is too demanding for that!\n\r", ch);
                  }
                  else
                  {
                     send_to_char("No way!  You are still fighting!\n\r", ch);
                  }
                  break;
               case POS_DEFENSIVE:
                  if (skill->minimum_position <= POS_EVASIVE)
                  {
                     send_to_char("This fighting style is too demanding for that!\n\r", ch);
                  }
                  else
                  {
                     send_to_char("No way!  You are still fighting!\n\r", ch);
                  }
                  break;
               case POS_AGGRESSIVE:
                  if (skill->minimum_position <= POS_EVASIVE)
                  {
                     send_to_char("This fighting style is too demanding for that!\n\r", ch);
                  }
                  else
                  {
                     send_to_char("No way!  You are still fighting!\n\r", ch);
                  }
                  break;
               case POS_BERSERK:
                  if (skill->minimum_position <= POS_EVASIVE)
                  {
                     send_to_char("This fighting style is too demanding for that!\n\r", ch);
                  }
                  else
                  {
                     send_to_char("No way!  You are still fighting!\n\r", ch);
                  }
                  break;
               case POS_EVASIVE:
                  send_to_char("No way!  You are still fighting!\n\r", ch);
                  break;
               case POS_SLEEPING:
                  send_to_char("You dream about great feats of magic.\n\r", ch);
                  break;
            }
            return;
         }

         if (skill->spell_fun == spell_null)
         {
            send_to_char("That's not a spell!\n\r", ch);
            return;
         }

         if (!skill->spell_fun)
         {
            send_to_char("You cannot cast that... yet.\n\r", ch);
            return;
         }

         mana = IS_NPC(ch) ? 0 : skill->min_mana;
         if (ch->race == RACE_FAIRY)
         {
            mana = mana * 66 / 100;
         }
         if (ch->race == RACE_ELF)
         {
            mana = mana * 85 / 100;
         }
         
         if (!IS_NPC(ch) && ch->pcdata->ranking[sn] >= 3)
         {
            if (ch->pcdata->ranking[sn] == 3)
               mana = mana * 80 / 100;
            if (ch->pcdata->ranking[sn] == 4)
               mana = mana * 66 / 100;
            if (ch->pcdata->ranking[sn] == 5)
               mana = mana * 50 / 100;
            if (ch->pcdata->ranking[sn] == 6)
               mana = mana * 33 / 100;
         }
         mana = UMAX(1, mana);

         if (HAS_WAIT(ch))
         {
            send_to_char("You are too busy in battle to do that.\n\r", ch);
            return;
         }

         /*
          * Locate targets.
          */
         vo = locate_targets(ch, arg2, sn, &victim, &obj);
         if (vo == &pAbort)
            return;

         if (skill->target == TAR_CHAR_OFFENSIVE)
            if (!IS_NPC(ch) && !IS_NPC(victim))
               if ((IS_IMMORTAL(ch) && !IS_IMMORTAL(victim)) || (!IS_IMMORTAL(ch) && IS_IMMORTAL(victim)))
               {
                  sprintf(log_buf, "%s or %s is casting a spell on the other.", ch->name, victim->name);
                  log_string_plus(log_buf, LOG_NORMAL, LEVEL_ADMIN);
                  send_to_char("Sorry, offensive spells cannot be casted by or towards an immortal.\n\r", ch);
                  return;
               }
         /*
          * Vampire spell casting    -Thoric
          */
         blood = UMAX(1, (mana + 4) / 8); /* NPCs don't have PCDatas. -- Altrag */
         if (IS_VAMPIRE(ch))
         {
            if (ch->pcdata->condition[COND_BLOODTHIRST] < blood)
            {
               send_to_char("You don't have enough blood power.\n\r", ch);
               return;
            }
         }
         else if (!IS_NPC(ch) && ch->mana < mana)
         {
            send_to_char("You don't have enough mana.\n\r", ch);
            return;
         }

         if (skill->participants <= 1)
            break;

         /* multi-participant spells   -Thoric */
         add_timer(ch, TIMER_DO_FUN, UMIN(skill->beats, 3), do_cast, 1);
         act(AT_MAGIC, "You begin to chant...", ch, NULL, NULL, TO_CHAR);
         act(AT_MAGIC, "$n begins to chant...", ch, NULL, NULL, TO_ROOM);
         sprintf(staticbuf, "%s %s", arg2, target_name);
         ch->alloc_ptr = str_dup(staticbuf);
         ch->tempnum = sn;
         return;
      case SUB_TIMER_DO_ABORT:
         DISPOSE(ch->alloc_ptr);
         if (IS_VALID_SN((sn = ch->tempnum)))
         {
            if ((skill = get_skilltype(sn)) == NULL)
            {
               send_to_char("Something went wrong...\n\r", ch);
               bug("do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn);
               return;
            }
            mana = IS_NPC(ch) ? 0 : skill->min_mana;
            blood = UMAX(1, (mana + 4) / 8);
            if (IS_VAMPIRE(ch))
               gain_condition(ch, COND_BLOODTHIRST, -UMAX(1, blood / 3));
            else if (ch->level < LEVEL_IMMORTAL) /* so imms dont lose mana */
               ch->mana -= mana / 3;
         }
         set_char_color(AT_MAGIC, ch);
         send_to_char("You stop chanting...\n\r", ch);
         /* should add chance of backfire here */
         return;
      case 1:
         sn = ch->tempnum;
         if ((skill = get_skilltype(sn)) == NULL)
         {
            send_to_char("Something went wrong...\n\r", ch);
            bug("do_cast: substate 1: bad sn %d", sn);
            return;
         }
         if (!ch->alloc_ptr || !IS_VALID_SN(sn) || skill->type != SKILL_SPELL)
         {
            send_to_char("Something cancels out the spell!\n\r", ch);
            bug("do_cast: ch->alloc_ptr NULL or bad sn (%d)", sn);
            return;
         }
         mana = IS_NPC(ch) ? 0 : skill->min_mana;
         blood = UMAX(1, (mana + 4) / 8);
         strcpy(staticbuf, ch->alloc_ptr);
         target_name = one_argument(staticbuf, arg2);
         DISPOSE(ch->alloc_ptr);
         ch->substate = SUB_NONE;
         if (skill->participants > 1)
         {
            int cnt = 1;
            CHAR_DATA *tmp;
            TIMER *t;

            for (tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room)
               if (tmp != ch
                  && (t = get_timerptr(tmp, TIMER_DO_FUN)) != NULL
                  && t->count >= 1 && t->do_fun == do_cast && tmp->tempnum == sn && tmp->alloc_ptr && !str_cmp(tmp->alloc_ptr, staticbuf))
                  ++cnt;
            if (cnt >= skill->participants)
            {
               for (tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room)
                  if (tmp != ch
                     && (t = get_timerptr(tmp, TIMER_DO_FUN)) != NULL
                     && t->count >= 1 && t->do_fun == do_cast && tmp->tempnum == sn && tmp->alloc_ptr && !str_cmp(tmp->alloc_ptr, staticbuf))
                  {
                     extract_timer(tmp, t);
                     act(AT_MAGIC, "Channeling your energy into $n, you help cast the spell!", ch, NULL, tmp, TO_VICT);
                     act(AT_MAGIC, "$N channels $S energy into you!", ch, NULL, tmp, TO_CHAR);
                     act(AT_MAGIC, "$N channels $S energy into $n!", ch, NULL, tmp, TO_NOTVICT);
                     if (victim)
                        learn_from_success(tmp, sn, victim);
                     else
                        learn_from_success(tmp, sn, NULL);
                     if (IS_VAMPIRE(ch))
                        gain_condition(tmp, COND_BLOODTHIRST, -blood);
                     else
                        tmp->mana -= mana;
                     tmp->substate = SUB_NONE;
                     tmp->tempnum = -1;
                     DISPOSE(tmp->alloc_ptr);
                  }
               dont_wait = TRUE;
               send_to_char("You concentrate all the energy into a burst of mystical words!\n\r", ch);
               vo = locate_targets(ch, arg2, sn, &victim, &obj);
               if (vo == &pAbort)
                  return;
            }
            else
            {
               set_char_color(AT_MAGIC, ch);
               send_to_char("There was not enough power for the spell to succeed...\n\r", ch);
               if (IS_VAMPIRE(ch))
                  gain_condition(ch, COND_BLOODTHIRST, -UMAX(1, blood / 2));
               else if (ch->level < LEVEL_IMMORTAL) /* so imms dont lose mana */
                  ch->mana -= mana / 2;
               if (victim)
                  learn_from_failure(ch, sn, victim);
               else
                  learn_from_success(ch, sn, NULL);
               return;
            }
         }
   }

   /* uttering those magic words unless casting "ventriloquate" */
   if (str_cmp(skill->name, "ventriloquate"))
      say_spell(ch, sn);

   /*
    * Getting ready to cast... check for spell components -Thoric
    */
   if (!process_spell_components(ch, sn))
   {
      if (IS_VAMPIRE(ch))
         gain_condition(ch, COND_BLOODTHIRST, -UMAX(1, blood / 2));
      else if (ch->level < LEVEL_IMMORTAL) /* so imms dont lose mana */
         ch->mana -= mana / 2;
      if (victim)
         learn_from_failure(ch, sn, victim);
      else
         learn_from_failure(ch, sn, NULL);
      
      if (!dont_wait)
      {
         if (!ch->fighting)
            WAIT_STATE(ch, skill->beats*2);
         else
            ch->fight_timer = get_btimer(ch, sn, NULL);
      }
      return;
   }

   suc = 1;
   if (!IS_NPC(ch))
   {
      if ((number_percent() + skill->difficulty * 5) > (90 + (ch->pcdata->learned[sn]*10)))
         suc = 0;
   }


   if (suc == 0)
   {
      /* Some more interesting loss of concentration messages  -Thoric */
      switch (number_bits(2))
      {
         case 0: /* too busy */
            if (ch->fighting)
               send_to_char("This round of battle is too hectic to concentrate properly.\n\r", ch);
            else
               send_to_char("You lost your concentration.\n\r", ch);
            break;
         case 1: /* irritation */
            if (number_bits(2) == 0)
            {
               switch (number_bits(2))
               {
                  case 0:
                     send_to_char("A tickle in your nose prevents you from keeping your concentration.\n\r", ch);
                     break;
                  case 1:
                     send_to_char("An itch on your leg keeps you from properly casting your spell.\n\r", ch);
                     break;
                  case 2:
                     send_to_char("Something in your throat prevents you from uttering the proper phrase.\n\r", ch);
                     break;
                  case 3:
                     send_to_char("A twitch in your eye disrupts your concentration for a moment.\n\r", ch);
                     break;
               }
            }
            else
               send_to_char("Something distracts you, and you lose your concentration.\n\r", ch);
            break;
         case 2: /* not enough time */
            if (ch->fighting)
               send_to_char("There wasn't enough time this round to complete the casting.\n\r", ch);
            else
               send_to_char("You lost your concentration.\n\r", ch);
            break;
         case 3:
            send_to_char("You get a mental block mid-way through the casting.\n\r", ch);
            break;
      }
      if (IS_VAMPIRE(ch))
         gain_condition(ch, COND_BLOODTHIRST, -UMAX(1, blood / 2));
      else if (ch->level < LEVEL_IMMORTAL) /* so imms dont lose mana */
         ch->mana -= mana / 2;
      if (victim)
         learn_from_failure(ch, sn, victim);
      else
         learn_from_failure(ch, sn, NULL);
      return;
   }
   else
   {
      int mint = 0, bint;
      
      //There should be some kind of stop here if you aren't in the room ya
      if (skill->target == TAR_CHAR_OFFENSIVE && victim)
      {
         if (!IN_SAME_ROOM(ch, victim))
         {
            send_to_char("Your target is not in the same room as you!\n\r", ch);
            return;
         }
      }
      
      if (IS_VAMPIRE(ch))
         gain_condition(ch, COND_BLOODTHIRST, -blood);
      else
         ch->mana -= mana;
      
      if (!IS_NPC(ch))
      {           
         if (sysdata.stat_gain <= 1)
            mint = number_range(4+(mana/24), 7+(mana/16));
         else if (sysdata.stat_gain <= 3)
            mint = number_range(7+(mana/16), 10+(mana/12));
         else if (sysdata.stat_gain >= 5)
            mint = number_range(10+(mana/10), 10+(mana/7));
         
         bint = 14 + race_table[ch->race]->int_plus;
         if (mint == bint - 4)
            bint *= 2;
         if (mint == bint - 3)
            bint *= 1.7;
         if (mint == bint - 2)
            bint *= 1.5;
         if (mint == bint - 1)
            bint *= 1.2;
         if (mint == bint)
            bint *= 1;
         if (mint == bint + 1)
            bint *= .85;
         if (mint == bint + 2)
            bint *= .7;
         if (mint == bint + 3)
            bint *= .6;
         if (mint == bint + 4)
            bint *= .4;
         if (mint == bint + 5)
            bint *= .3;
         if (mint == bint + 6)
            bint *= .275;
         if (mint == bint + 7)
            bint *= .25;
         if (mint == bint + 8)
            bint *= .225;
         if (mint > bint + 8) //Base + 8 should be the max unless you screwed it up
            bint = 0;
         else
         {
            if (mint == 0)
               mint = 1;
         }
      
         if (ch->perm_int >= (14 + race_table[ch->race]->int_plus + race_table[ch->race]->int_range + get_talent_increase(ch, 3)) && ch->pcdata->per_int >= 3000)
            mint = 0;
            
         if (victim && IS_NPC(victim) && xIS_SET(victim->act, ACT_MOUNTSAVE))
         {
            if (mint > 0)
               mint = 0;
         } 
         
         ch->pcdata->per_int += mint;
         if (ch->pcdata->per_int > 10000)
         {
            ch->perm_int++;
            send_to_char("&R******************************************\n\r", ch);
            send_to_char("&R*****You Gain 1 Point of Intelligence*****\n\r", ch);
            send_to_char("&R******************************************\n\r", ch);
            ch->pcdata->per_int = 0;
         }
         if (ch->pcdata->per_lck < 0)
         {
            ch->perm_int--;
            send_to_char("&r******************************************\n\r", ch);
            send_to_char("&r*****You Lose 1 Point of Intelligence*****\n\r", ch);
            send_to_char("&r******************************************\n\r", ch);
            ch->pcdata->per_int = 9999;
         }
      } 
      gain_mana_per(ch, victim, mana);
      /*
       * If it is offensive or ignore it won't work
       * 
       * otherwise spells will have to check themselves
       */
      if (((skill->target == TAR_CHAR_OFFENSIVE || skill->target == TAR_IGNORE) && victim && is_immune(victim, -1, RIS_MAGIC)))
      {
         immune_casting(skill, ch, victim, NULL);
         retcode = rSPELL_FAILED;
      }
      else
      {
         int level;
         
         start_timer(&time_used);
         level = POINT_LEVEL(LEARNED(ch, sn), MASTERED(ch, sn));
         retcode = (*skill->spell_fun) (sn, level, ch, vo);
         end_timer(&time_used);
         update_userec(&time_used, &skill->userec);
      }
   }

   if (retcode == rCHAR_DIED || retcode == rERROR || char_died(ch))
      return;

   /* learning */
   if (retcode != rSPELL_FAILED)
      learn_from_success(ch, sn, victim);
   else
      learn_from_failure(ch, sn, victim);

   /* favor adjustments */
   if (victim && victim != ch && !IS_NPC(victim) && skill->target == TAR_CHAR_DEFENSIVE)
      adjust_favor(ch, 7, 1);

   if (victim && victim != ch && !IS_NPC(ch) && skill->target == TAR_CHAR_DEFENSIVE)
      adjust_favor(victim, 13, 1);

   if (victim && victim != ch && !IS_NPC(ch) && skill->target == TAR_CHAR_OFFENSIVE)
      adjust_favor(ch, 4, 1);

   /*
    * Fixed up a weird mess here, and added double safeguards -Thoric
    */
   if (skill->target == TAR_CHAR_OFFENSIVE && victim && !char_died(victim) && victim != ch)
   {
      CHAR_DATA *vch, *vch_next;

      for (vch = ch->in_room->first_person; vch; vch = vch_next)
      {
         vch_next = vch->next_in_room;

         if (vch == victim)
         {
            if (victim->master != ch && !victim->fighting)
               retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
            break;
         }
      }
   }
   if (!dont_wait)
   {
      int beats = 0;
      if (MASTERED(ch, sn) == 4)
         beats++;
      if (MASTERED(ch, sn) == 6)
         beats++;
      if (!ch->fighting)
      {
         
         WAIT_STATE(ch, UMAX(1, skill_table[sn]->beats-beats)*2);
      }
      else
      {
         ch->fight_timer = get_btimer(ch, sn, NULL);
         ch->fight_timer = UMAX(1, ch->fight_timer -= beats);
      }
         
   }

   return;
}

/*
 * Will find if they can use the "portal" system.
 */
bool can_use_portal(CHAR_DATA * ch, int type)
{
   int p;
   int count = 0;
   int standing = 0;

   if (in_hellmaze(ch)
      || (!IS_NPC(ch) && ch->pcdata->caste < 2) || (!IS_NPC(ch) && xIS_SET(ch->act, PLR_GAMBLER)))
   {
      if (type == 3)
         send_to_char("Sorry, you cannot use your rune right now.\n\r", ch);
      else
         send_to_char("Sorry, you cannot cast that spell right now.\n\r", ch);
      return FALSE;
   }
   if (type != 2)
   {
      if (xIS_SET(ch->act, PLR_PORTALHUNT))
      {
         send_to_char("Sorry, you cannot use this spell while portal hunting.\n\r", ch);
         return FALSE;
      }
   }
   for (p = 0; p < sysdata.last_portal; p++)
   {
      if (xIS_SET(ch->pcdata->portalfnd, p))
      {
         count++;
         if (portal_show[p]->x == ch->coord->x && portal_show[p]->y == ch->coord->y && portal_show[p]->map == ch->map)
         {
            standing++;
         }
      }
   }
   if (count == 0)
   {
      send_to_char("You have not found any portals, you need to hunt some down.\n\r", ch);
      return FALSE;
   }
   if (ch->in_room->vnum == ROOM_VNUM_PORTAL)
      standing++;

   if (standing == 0 && (type == 0 || type == 3))
   {
      if (type == 3)
         send_to_char("You can only use this rune at a portal or poral room.\n\r", ch);
      else
         send_to_char("You can only cast this spell at a portal or portal room.\n\r", ch);
      return FALSE;
   }
   return TRUE;
}

/*
 * Cast spells at targets using a magical object.
 */
ch_ret obj_cast_spell(int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj)
{
   void *vo;
   ch_ret retcode = rNONE;
   SKILLTYPE *skill = get_skilltype(sn);
   struct timeval time_used;
   int passarg = 0;

   if (sn >= 9999)
   {
      sn -= 10000;
      skill = get_skilltype(sn);
      passarg = 1;
   }
   if (sn == -1)
      return retcode;
   if (!skill || !skill->spell_fun)
   {
      bug("Obj_cast_spell: bad sn %d.", sn);
      return rERROR;
   }

   if (xIS_SET(ch->in_room->room_flags, ROOM_NO_MAGIC) || wIS_SET(ch, ROOM_NO_MAGIC))
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("Nothing seems to happen...\n\r", ch);
      return rNONE;
   }

   if (ch && victim && !IS_NPC(ch) && !IS_NPC(victim))
   {
      if ((check_room_pk(ch) == 1) && skill->target == TAR_CHAR_OFFENSIVE)
      {
         set_char_color(AT_MAGIC, ch);
         send_to_char("Nothing seems to happen...\n\r", ch);
         return rNONE;
      }
   }
   else
   {
      if (is_room_safe(ch) && skill->target == TAR_CHAR_OFFENSIVE)
      {
         set_char_color(AT_MAGIC, ch);
         send_to_char("Nothing seems to happen...\n\r", ch);
         return rNONE;
      }
   }
   
   
   if (!passarg)
      target_name = "";
   switch (skill->target)
   {
      default:
         bug("Obj_cast_spell: bad target for sn %d.", sn);
         return rERROR;

      case TAR_IGNORE:
         vo = NULL;
         if (victim)
            target_name = victim->name;
         else if (obj)
            target_name = obj->name;
         break;

      case TAR_CHAR_OFFENSIVE:
         if (victim != ch)
         {
            if (!victim)
               victim = who_fighting(ch);
            if (!victim || (!IS_NPC(victim) && !in_arena(victim)))
            {
               send_to_char("You can't do that.\n\r", ch);
               return rNONE;
            }
         }
         if (ch != victim && is_safe(ch, victim))
            return rNONE;
         vo = (void *) victim;
         break;

      case TAR_CHAR_DEFENSIVE:
         if (victim == NULL)
            victim = ch;
         vo = (void *) victim;
         break;

      case TAR_CHAR_SELF:
         vo = (void *) ch;
         break;

      case TAR_OBJ_INV: case TAR_OBJ_ROOM:
         if (obj == NULL)
         {
            send_to_char("You can't do that.\n\r", ch);
            return rNONE;
         }
         vo = (void *) obj;
         break;
   }
   
   if (victim && !IS_NPC(ch) && IS_NPC(victim))
   {
      if (xIS_SET(victim->act, ACT_MOUNTSAVE))
         return rNONE;
      if (victim->master == ch)
         return rNONE;
   }
       
   start_timer(&time_used);
   retcode = (*skill->spell_fun) (sn, level, ch, vo);
   end_timer(&time_used);
   update_userec(&time_used, &skill->userec);

   if (retcode == rSPELL_FAILED)
      retcode = rNONE;

   if (retcode == rCHAR_DIED || retcode == rERROR)
      return retcode;

   if (char_died(ch))
      return rCHAR_DIED;
      
   if (victim && victim != ch && skill->target == TAR_CHAR_OFFENSIVE)
      retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
   /*if (SPELL_FLAG(skill, SF_AREA))
   {     
      for (vch = ch->in_room->first_person; vch; vch = vch_next)
      {
         vch_next = vch->next_in_room;
         if (ch->coord->x != vch->coord->x || ch->coord->y != vch->coord->y || ch->map != vch->map)
            continue;
         if (!IS_NPC(ch) && IS_NPC(vch))
         {
            if (!IS_NPC(ch) && xIS_SET(vch->act, ACT_MOUNTSAVE))
               continue;
            if (IS_NPC(vch) && IS_AFFECTED(vch, AFF_CHARM) && vch->master == ch)
               continue;         
            if (IS_NPC(vch) && xIS_SET(ch->act, ACT_MOUNTABLE))
               continue;
            if (vch->master == ch)
               continue;
         }
         if (vch == ch)
            continue;
         obj_cast_spell(sn, level, ch, vch, obj);
      }   
   }*/
   return retcode;
}

ch_ret spell_web(int sn, int level, CHAR_DATA *ch, void *vo)
{
   sh_int points;
   int chance;
   sh_int isobj = 0;
   AFFECT_DATA af;
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int str;
   
   if (level >= 999)
   {
      isobj = 1;
   }
   points = POINT_LEVEL(GET_POINTS(ch, sn, isobj, level), GET_MASTERY(ch, sn, isobj, level));
   
   str = get_curr_str(victim);
   chance = 20+(UMIN(25, level/3))-((str-16)*3);
   chance = ris_save(victim, chance, RIS_PARALYSIS);
   if (chance < 1000)
      chance = URANGE(5, chance, 55);
   
   if (number_range(1, 100) <= chance && chance != 1000)
   {
      af.type = sn;
      af.duration = 40+UMIN(15, level/5);
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb(AFF_WEB);
      affect_join(victim, &af);
      act(AT_MAGIC, "[WEB] $N tries to move but $S feet are cought in some nasty webs.", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "[WEB] $N tries to move but $S feet are cought in some nasty webs.", ch, NULL, victim, TO_NOTVICT);
      act(AT_MAGIC, "[WEB] You try to move but your feet are cought in some nasty webs.", ch, NULL, victim, TO_VICT);
      adjust_aggression_list(victim, ch, 20, 4, 0);
      return rNONE;
   }
   else
   {
      act(AT_MAGIC, "[web] You try to surround $N's feet in webs but fails.", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "[web] $n tries to surround $N's feet in webs but fails.", ch, NULL, victim, TO_NOTVICT);
      act(AT_MAGIC, "[web] $n tries to surround your feet in webs but fails.", ch, NULL, victim, TO_VICT);  
      adjust_aggression_list(victim, ch, 5, 4, 0);
      return rSPELL_FAILED;
   } 
   return rNONE;
}

ch_ret spell_snare(int sn, int level, CHAR_DATA *ch, void *vo)
{
   sh_int points;
   int chance;
   sh_int isobj = 0;
   AFFECT_DATA af;
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int str;
   
   if (level >= 999)
   {
      isobj = 1;
   }
   points = POINT_LEVEL(GET_POINTS(ch, sn, isobj, level), GET_MASTERY(ch, sn, isobj, level));
   
   str = get_curr_str(victim);
   chance = 40+(UMIN(25, level/3))-((str-16)*3);
   chance = ris_save(victim, chance, RIS_PARALYSIS);
   if (chance < 1000)
      chance = URANGE(5, chance, 75);
   
   if (number_range(1, 100) <= chance && chance != 1000)
   {
      af.type = sn;
      af.duration = 70+UMIN(20, level/4);
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb(AFF_WEB);
      af.bitvector = meb(AFF_SNARE); //For damage break checks mainly, harder to break a snare
      affect_join(victim, &af);
      act(AT_MAGIC, "[SNARE] $N tries to move but $S feet are cought in a nasty magical snare.", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "[SNARE] $N tries to move but $S feet are cought in a nasty magical snare.", ch, NULL, victim, TO_NOTVICT);
      act(AT_MAGIC, "[SNARE] You try to move but your feet are cought in a nasty magical snare.", ch, NULL, victim, TO_VICT);
      adjust_aggression_list(victim, ch, 20, 4, 0);
      return rNONE;
   }
   else
   {
      act(AT_MAGIC, "[snare] You try to surround $N's feet in a magical snare but fails.", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "[snare] $n tries to surround $N's feet in a magical snare but fails.", ch, NULL, victim, TO_NOTVICT);
      act(AT_MAGIC, "[snare] $n tries to surround your feet in a magical snare but fails.", ch, NULL, victim, TO_VICT);  
      adjust_aggression_list(victim, ch, 5, 4, 0);
      return rSPELL_FAILED;
   } 
   return rNONE;
}
      
ch_ret spell_greater_resurrection(int sn, int level, CHAR_DATA *ch, void *vo)
{
   OBJ_DATA *corpse = (OBJ_DATA *) vo;
   sh_int points;
   sh_int mastery; 
   sh_int isobj = 0;
   char *pd;
   DESCRIPTOR_DATA *d;
   char name[MSL];
   int rezpercent;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   points = POINT_LEVEL(points, mastery);   
   
   if (!corpse)
   {
      send_to_char("That corpse is not here!\n\r", ch);
      return rSPELL_FAILED;
   }
   if (corpse->item_type != ITEM_CORPSE_PC)
   {
      send_to_char("That is not a corpse!\n\r", ch);
      return rSPELL_FAILED;
   }
   pd = corpse->short_descr;
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2 && d->character && d->character->pcdata)
      {
         if (!str_cmp(name, d->character->name))
            break;
      }
   }
   if (!d)
   {
      send_to_char("Your target is not online or present atm.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (d->character->pcdata->lostcon >= 0)
   {
      send_to_char("Your target has already been revived, you cannot do it again.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (IN_WILDERNESS(ch))
   {
      if (room_is_private_wilderness(ch, ch->in_room, ch->coord->x, ch->coord->y, ch->map))
      {
         send_to_char("This room is private.\n\r", ch);
         return rSPELL_FAILED;
      }
   }
   else
   {
      if (room_is_private(ch->in_room))
      {
         send_to_char("This room is private.\n\r", ch);
         return rSPELL_FAILED;
      }
   }
   if (IS_OBJ_STAT(corpse, ITEM_CORPSEREVIVE))
   {
      send_to_char("This corpse has already been granted a revival, no use doing it twice.\n\r", ch);
      return rSPELL_FAILED;
   }
   rezpercent = (60+UMIN(40, (points * 57 / 100)));
   if (rezpercent <= d->character->pcdata->rezpercent)
   {
      send_to_char("Your target already has a better resurrection offered.\n\r", ch);
      return rSPELL_FAILED;
   }
   d->character->pcdata->rezpercent = rezpercent;
   d->character->pcdata->rezcorpse = corpse;
   d->character->pcdata->rezroom = ch->in_room;
   d->character->pcdata->rezx = ch->coord->x;
   d->character->pcdata->rezy = ch->coord->y;
   d->character->pcdata->rezmap = ch->map;
   ch_printf(d->character, "You are being offered a %d%% Resurrection by %s.\n\r", d->character->pcdata->rezpercent, PERS_MAP(ch, d->character));
   ch_printf(ch, "You offered your target a %d%% Resurrection.\n\r", d->character->pcdata->rezpercent);
   return rNONE;
}

ch_ret spell_summon_corpse(int sn, int level, CHAR_DATA *ch, void *vo)
{
   OBJ_DATA *obj;
   OBJ_DATA *idol;
   OBJ_INDEX_DATA *iobj;
   CHAR_DATA *victim;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   char *pd;
   char name[MSL];
   int fnd = 0;
   int quest = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   points = POINT_LEVEL(points, mastery);  
   
   if ((victim = get_char_world(ch, target_name)) == NULL)
   {
      send_to_char("Failed to locate your target.\n\r", ch);
      return rSPELL_FAILED;
   }
   
   if (IS_NPC(victim))
   {
      send_to_char("Not on npcs.\n\r", ch);
      return rSPELL_FAILED;
   }
   
   if ((iobj = get_obj_index(OBJ_VNUM_IDOL)) == NULL)
   {
      send_to_char("Idol does not exist in the game, notify an immortal.\n\r", ch);
      bug("Idol needed for summon corpse does not exist!!!");
      return rSPELL_FAILED;
   }
   
   idol = get_obj_carry(ch, iobj->name);
   if (!idol || idol->pIndexData->vnum != iobj->vnum)
   {
      send_to_char("You need a Divine Idol to cast this spell.\n\r", ch);
      return rSPELL_FAILED;
   }
   
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->item_type != ITEM_CORPSE_PC)
         continue;
      if (!obj->in_room)
         continue;
      if (obj->in_room->area != ch->in_room->area)
         continue;
      pd = obj->short_descr;
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      pd = one_argument(pd, name);
      pd = one_argument(pd, name); 
      if (!str_cmp(name, victim->name))
      {
         obj_from_room(obj);
         obj_to_room(obj, ch->in_room, ch);
         fnd++;
      }
   } 
   if (fnd > 0)
   {
      act(AT_BLUE, "You summon the corpse(s) of $N", ch, NULL, victim, TO_CHAR);
      act(AT_BLUE, "$n summons the corpse(s) of $N", ch, NULL, victim, TO_ROOM);
      if (ch->in_room->area->low_r_vnum >= START_QUEST_VNUM && ch->in_room->area->low_r_vnum <= END_QUEST_VNUM)
         quest = 1;
      if (number_range(1, 100) > UMAX(20, level/4) && !quest)
      {
         act(AT_RED, "The idol shimmers and is gone.", ch, NULL, victim, TO_ROOM);
         act(AT_RED, "The idol shimmers and is gone.", ch, NULL, victim, TO_CHAR);
         separate_obj(idol);
         obj_from_char(idol);
         extract_obj(idol);
      }
      learn_from_success(ch, gsn_summon_corpse, NULL);
      return rNONE;
   }
   else
   {
      send_to_char("No corpses could be found in this area.\n\r", ch);
      return rSPELL_FAILED;
   }
}

ch_ret spell_lesser_resurrection(int sn, int level, CHAR_DATA *ch, void *vo)
{
   OBJ_DATA *corpse = (OBJ_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   char *pd;
   DESCRIPTOR_DATA *d;
   char name[MSL];
   int rezpercent;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   points = POINT_LEVEL(points, mastery);   

   
   if (!corpse)
   {
      send_to_char("That corpse is not here!\n\r", ch);
      return rSPELL_FAILED;
   }
   if (corpse->item_type != ITEM_CORPSE_PC)
   {
      send_to_char("That is not a corpse!\n\r", ch);
      return rSPELL_FAILED;
   }
   pd = corpse->short_descr;
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2 && d->character && d->character->pcdata)
      {
         if (!str_cmp(name, d->character->name))
            break;
      }
   }
   if (corpse->timer < 2880 - 60 - (level*3))
   {
      send_to_char("Your target has been dead for too long to use this spell.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (!d)
   {
      send_to_char("Your target is not online or present atm.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (d->character->pcdata->lostcon >= 0)
   {
      send_to_char("Your target has already been revived, you cannot do it again.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (IN_WILDERNESS(ch))
   {
      if (room_is_private_wilderness(ch, ch->in_room, ch->coord->x, ch->coord->y, ch->map))
      {
         send_to_char("This room is private.\n\r", ch);
         return rSPELL_FAILED;
      }
   }
   else
   {
      if (room_is_private(ch->in_room))
      {
         send_to_char("This room is private.\n\r", ch);
         return rSPELL_FAILED;
      }
   }
      
   if (IS_OBJ_STAT(corpse, ITEM_CORPSEREVIVE))
   {
      send_to_char("This corpse has already been granted a revival, no use doing it twice.\n\r", ch);
      return rSPELL_FAILED;
   }
   rezpercent = UMAX(20, points/4);
   if (rezpercent <= d->character->pcdata->rezpercent)
   {
      send_to_char("Your target already has a better resurrection offered.\n\r", ch);
      return rSPELL_FAILED;
   }
   d->character->pcdata->rezpercent = rezpercent;
   d->character->pcdata->rezcorpse = corpse;
   d->character->pcdata->rezroom = ch->in_room;
   d->character->pcdata->rezx = ch->coord->x;
   d->character->pcdata->rezy = ch->coord->y;
   d->character->pcdata->rezmap = ch->map;
   ch_printf(d->character, "You are being offered a %d%% Resurrection by %s.\n\r", d->character->pcdata->rezpercent, PERS_MAP(ch, d->character));
   ch_printf(ch, "You offered your target a %d%% Resurrection.\n\r", d->character->pcdata->rezpercent);
   return rNONE;
}

ch_ret spell_resurrection(int sn, int level, CHAR_DATA *ch, void *vo)
{
   OBJ_DATA *corpse = (OBJ_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   char *pd;
   DESCRIPTOR_DATA *d;
   char name[MSL];
   int rezpercent;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   points = POINT_LEVEL(points, mastery);   

   
   if (!corpse)
   {
      send_to_char("That corpse is not here!\n\r", ch);
      return rSPELL_FAILED;
   }
   if (corpse->item_type != ITEM_CORPSE_PC)
   {
      send_to_char("That is not a corpse!\n\r", ch);
      return rSPELL_FAILED;
   }
   pd = corpse->short_descr;
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   pd = one_argument(pd, name);
   
   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING && d->character->in_room && d->newstate != 2 && d->character && d->character->pcdata)
      {
         if (!str_cmp(name, d->character->name))
            break;
      }
   }
   if (corpse->timer < 2880 - 60 - (level*3))
   {
      send_to_char("Your target has been dead for too long to use this spell.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (!d)
   {
      send_to_char("Your target is not online or present atm.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (d->character->pcdata->lostcon >= 0)
   {
      send_to_char("Your target has already been revived, you cannot do it again.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (IN_WILDERNESS(ch))
   {
      if (room_is_private_wilderness(ch, ch->in_room, ch->coord->x, ch->coord->y, ch->map))
      {
         send_to_char("This room is private.\n\r", ch);
         return rSPELL_FAILED;
      }
   }
   else
   {
      if (room_is_private(ch->in_room))
      {
         send_to_char("This room is private.\n\r", ch);
         return rSPELL_FAILED;
      }
   }
      
   if (IS_OBJ_STAT(corpse, ITEM_CORPSEREVIVE))
   {
      send_to_char("This corpse has already been granted a revival, no use doing it twice.\n\r", ch);
      return rSPELL_FAILED;
   }
   rezpercent = (20+UMIN(40, (points * 57 / 100)));
   if (rezpercent <= d->character->pcdata->rezpercent)
   {
      send_to_char("Your target already has a better resurrection offered.\n\r", ch);
      return rSPELL_FAILED;
   }
   d->character->pcdata->rezpercent = rezpercent;
   d->character->pcdata->rezcorpse = corpse;
   d->character->pcdata->rezroom = ch->in_room;
   d->character->pcdata->rezx = ch->coord->x;
   d->character->pcdata->rezy = ch->coord->y;
   d->character->pcdata->rezmap = ch->map;
   ch_printf(d->character, "You are being offered a %d%% Resurrection by %s.\n\r", d->character->pcdata->rezpercent, PERS_MAP(ch, d->character));
   ch_printf(ch, "You offered your target a %d%% Resurrection.\n\r", d->character->pcdata->rezpercent);
   return rNONE;
}
void write_corpses args((CHAR_DATA * ch, char *name, OBJ_DATA * objrem));

//Used with the resurrection spells to accept a rez
void do_resurrection(CHAR_DATA *ch, char *argument)
{
   AFFECT_DATA af;
   AFFECT_DATA af2;
   AFFECT_DATA af3;
   AFFECT_DATA af4;
   
   if (check_npc(ch))
      return;
   if (argument[0] == '\0')
   {
      if (ch->pcdata->rezpercent == 0)
      {
         send_to_char("No one has attempted to resurrect you yet.\n\r", ch);
      }
      else
      {
         ch_printf(ch, "You have been offered a %d%% resurrection.\n\r", ch->pcdata->rezpercent);
      }
      send_to_char("Syntax:  resurrection yes\n\r", ch);
      send_to_char("Syntax:  resurrection cost\n\r", ch);
      return;
   }
   if (ch->position < POS_STANDING)
   {
      send_to_char("You can only do this standing or mounted.\n\r", ch);
      return;
   }
   if (!ch->pcdata->rezcorpse || !ch->pcdata->rezroom)
   {
      send_to_char("There is a problem with the resurrection spell, tell an immortal.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "cost"))
   {
      int hpcost = 0;
      if (ch->max_hit > 100)
      {
         hpcost = ch->max_hit * 10/100;
         hpcost -= hpcost * ch->pcdata->rezpercent / 100;
         ch->max_hit -= hpcost;
      }   
      ch_printf(ch, "Your cost to your total HP would be %d", hpcost);
      return;
   }
   if (!str_cmp(argument, "yes"))
   {
      int hpcost = 0;
      act(AT_MAGIC, "$n disappears as an angel of light sweeps $m away.", ch, NULL, NULL, TO_CANSEE);
      char_from_room(ch);
      char_to_room(ch, ch->pcdata->rezroom);
      ch->coord->x = ch->pcdata->rezx;
      ch->coord->y = ch->pcdata->rezx;
      ch->map = ch->pcdata->rezmap;
      ch->hit = ch->max_hit * ch->pcdata->rezpercent / 100;
      ch->mana = ch->max_mana * ch->pcdata->rezpercent / 100;
      ch->move = ch->max_move * ch->pcdata->rezpercent / 100;
      ch->con_rarm = 1000 * ch->pcdata->rezpercent / 100;
      ch->con_larm = 1000 * ch->pcdata->rezpercent / 100;
      ch->con_rleg = 1000 * ch->pcdata->rezpercent / 100;
      ch->con_lleg = 1000 * ch->pcdata->rezpercent / 100;
      ch->pcdata->lostcon = 0;
      if (ch->max_hit > 100)
      {
         hpcost = ch->max_hit * 10/100;
         hpcost -= hpcost * ch->pcdata->rezpercent / 100;
         ch->max_hit -= hpcost;
      }
      ch_printf(ch, "The cost of your actions is %d HP\n\r", hpcost);
      if (IN_WILDERNESS(ch))
         SET_ONMAP_FLAG(ch);
      else
         REMOVE_ONMAP_FLAG(ch); 
      xSET_BIT(ch->pcdata->rezcorpse->extra_flags, ITEM_CORPSEREVIVE);
      if (ch->pcdata->mount)
      {
         char_from_room(ch->pcdata->mount);
         char_to_room(ch->pcdata->mount, ch->pcdata->rezroom);
         ch->pcdata->mount->coord->x = ch->pcdata->rezx;
         ch->pcdata->mount->coord->y = ch->pcdata->rezx;
         ch->pcdata->mount->map = ch->pcdata->rezx;
         if (IN_WILDERNESS(ch))
            SET_ONMAP_FLAG(ch->pcdata->mount);
         else
            REMOVE_ONMAP_FLAG(ch->pcdata->mount); 
      }
      update_objects(ch, ch->map, ch->coord->x, ch->coord->y);
      write_corpses(NULL, ch->pcdata->rezcorpse->short_descr + 14, NULL);
      ch->pcdata->rezx = ch->pcdata->rezy = ch->pcdata->rezmap = ch->pcdata->rezpercent = 0;
      ch->pcdata->rezroom = NULL;
      ch->pcdata->rezcorpse = NULL;
      act(AT_MAGIC, "You disappear as an angel of light sweeps you away and drops you off at your corpse.", ch, NULL, NULL, TO_CHAR);
      act(AT_MAGIC, "$n appears as an angel of light drops $m in this room with you.", ch, NULL, NULL, TO_CANSEE);
      af.type = gsn_resurrection;
      af.location = APPLY_ARMOR;
      af.modifier = -2;
      af.duration = 60;
      af.bitvector = meb(AFF_REZ);
      affect_to_char(ch, &af);
      
      af2.type = gsn_resurrection;
      af2.location = APPLY_TOHIT;
      af2.modifier = -2;
      af2.duration = 60;
      af2.bitvector = meb(AFF_REZ);
      affect_to_char(ch, &af2);
      
      af3.type = gsn_resurrection;
      af3.location = APPLY_MANATICK;
      af3.modifier = 40;
      af3.duration = 60;
      af3.bitvector = meb(AFF_REZ);
      affect_to_char(ch, &af3);
      
      af4.type = gsn_resurrection;
      af4.location = APPLY_HPTICK;
      af4.modifier = 40;
      af4.duration = 60;
      af4.bitvector = meb(AFF_REZ);
      affect_to_char(ch, &af4);
         
      do_look(ch, "auto");
      save_char_obj(ch);
      return;
   }
   do_resurrection(ch, "");
}
   
   

/*
 * Spell functions.
 */
ch_ret spell_blindness(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
   int tmp;
   sh_int tl;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   
   

   tmp = POINT_LEVEL(points, mastery);
   tl = tmp;

   if (SPELL_FLAG(skill, SF_PKSENSITIVE) && !IS_NPC(ch) && !IS_NPC(victim))
   {
      tmp = tmp / 2;
      tl = tl / 2;
   }

   if (is_immune(victim, -1, RIS_MAGIC))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (IS_AFFECTED(victim, AFF_BLIND) || saves_spell_staff(tl*.4, victim))
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   af.type = sn;
   af.duration = (4+tmp/5);
   af.bitvector = meb(AFF_BLIND);
   af.location = APPLY_NONE;
   af.modifier = 0;
   affect_to_char(victim, &af);
   set_char_color(AT_MAGIC, victim);
   send_to_char("You are blinded!\n\r", victim);
   if (ch != victim)
   {
      act(AT_MAGIC, "You weave a spell of blindness around $N.", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "$n weaves a spell of blindness about $N.", ch, NULL, victim, TO_NOTVICT);
   }
   return rNONE;
}

ch_ret spell_revitalize_spirit(int sn, int level, CHAR_DATA *ch, void *vo)
{
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   CHAR_DATA *victim = (CHAR_DATA *) vo;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   level = POINT_LEVEL(points, mastery);

   if (!victim)
   {
      send_to_char("That target is not here!\n\r", ch);
      return rSPELL_FAILED;
   }
   if (victim->mental_state > 0)
   {
      victim->mental_state -= UMAX(1, level/3);
      if (victim->mental_state < 0)
         victim->mental_state = 0;
      act(AT_WHITE, "$n is filled with a revitalizing aura that lifts $s spirit.", victim, NULL, NULL, TO_NOTVICT);
      act(AT_WHITE, "You are filled with a a revitalizing aura that lifts your spirit.", victim, NULL, NULL, TO_CHAR);
      return rNONE;
   }
   else
   {
      victim->mental_state += UMAX(1, level/8);
      if (victim->mental_state > 0)
         victim->mental_state = 0;
      act(AT_WHITE, "$n is filled with a revitalizing aura that lifts $s spirit.", victim, NULL, NULL, TO_NOTVICT);
      act(AT_WHITE, "You are filled with a a revitalizing aura that lifts your spirit.", victim, NULL, NULL, TO_CHAR);
      return rNONE;
   }
   return rSPELL_FAILED;
}
      
   
ch_ret spell_restore_limb(int sn, int level, CHAR_DATA * ch, void *vo)
{
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   CHAR_DATA *victim = (CHAR_DATA *) vo;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   if (!victim)
   {
      send_to_char("That target is not here!\n\r", ch);
      return rSPELL_FAILED;
   }
   
   if (victim->con_rarm < 0)
   {
      victim->con_rarm = 1+ ((mastery-1) * 50) + ((points-1) * 3);
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S broken right arm and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's broken right arm and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your broken right arm and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your broken right arm and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's broken right arm and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   if (victim->con_larm < 0)
   {
      victim->con_larm = 1+ ((mastery-1) * 50) + ((points-1) * 3);
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S broken left arm and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's broken left arm and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your broken left arm and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your broken left arm and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's broken left arm and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   if (victim->con_rleg < 0)
   {
      victim->con_rleg = 1+ ((mastery-1) * 50) + ((points-1) * 3);
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S broken right leg and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's broken right leg and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your broken right leg and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your broken right leg and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's broken right leg and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   if (victim->con_lleg < 0)
   {
      victim->con_lleg = 1+ ((mastery-1) * 50) + ((points-1) * 3);
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S broken left leg and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's broken left leg and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your broken left leg and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your broken left leg and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's broken left leg and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   send_to_char("There are no limbs to restore on your target.\n\r", ch);
   return rSPELL_FAILED;
}

ch_ret spell_revitalize_limb(int sn, int level, CHAR_DATA * ch, void *vo)
{
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   CHAR_DATA *victim = (CHAR_DATA *) vo;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   if (!victim)
   {
      send_to_char("That target is not here!\n\r", ch);
      return rSPELL_FAILED;
   }
   
   if (victim->con_rarm > -1 && victim->con_rarm < 1000)
   {
      victim->con_rarm = UMIN(1000, victim->con_rarm + 50+ ((mastery-1) * 80) + ((points-1) * 10));
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S injured right arm and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's injured right arm and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your injured right arm and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your injured right arm and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's injured right arm and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   if (victim->con_larm > -1 && victim->con_larm < 1000)
   {
      victim->con_larm = UMIN(1000, victim->con_larm + 50+ ((mastery-1) * 80) + ((points-1) * 10));
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S injured left arm and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's injured left arm and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your injured left arm and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your injured left arm and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's injured left arm and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   if (victim->con_rleg > -1 && victim->con_rleg < 1000)
   {
      victim->con_rleg = UMIN(1000, victim->con_rleg + 50+ ((mastery-1) * 80) + ((points-1) * 10));
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S injured right leg and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's injured right leg and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your injured right leg and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your injured right leg and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's injured right leg and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   if (victim->con_lleg > -1 && victim->con_lleg < 1000)
   {
      victim->con_lleg = UMIN(1000, victim->con_lleg + 50+ ((mastery-1) * 80) + ((points-1) * 10));
      if (ch == victim)
         act(AT_WHITE, "$n holds $s hands in front of $S injured left leg and heals it.", ch, NULL, victim, TO_CANSEE);
      else
         act(AT_WHITE, "$n holds $s hands in front of $N's injured left leg and heals it.", ch, NULL, victim, TO_CANSEE); 
      act(AT_WHITE, "$n holds $s hands in front of your injured left leg and heals it.", ch, NULL, victim, TO_VICT);
      if (ch == victim)
         act(AT_WHITE, "You hold you hands in front of your injured left leg and heal it.", ch, NULL, victim, TO_CHAR);
      else  
         act(AT_WHITE, "You hold you hands in front of $N's injured left leg and heal it.", ch, NULL, victim, TO_CHAR);
      return rNONE;
   }
   send_to_char("There are no limbs to revitalize on your target.\n\r", ch);
   return rSPELL_FAILED;
}

ch_ret spell_cause_light(int sn, int level, CHAR_DATA * ch, void *vo)
{
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   return damage(ch, (CHAR_DATA *) vo, UMAX(1, points-1), sn, 0, -1);
}



ch_ret spell_cause_critical(int sn, int level, CHAR_DATA * ch, void *vo)
{
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   return damage(ch, (CHAR_DATA *) vo, UMAX(5, POINT_LEVEL(points, mastery)/2+2), sn, 0, -1);
}



ch_ret spell_cause_serious(int sn, int level, CHAR_DATA * ch, void *vo)
{
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   return damage(ch, (CHAR_DATA *) vo, UMAX(2, POINT_LEVEL(points, mastery)/2-5), sn, 0, -1);
}

ch_ret spell_charm_person(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
   int chance;
   char buf[MSL];
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery)/2;

   if (victim == ch)
   {
      send_to_char("You like yourself even better!\n\r", ch);
      return rSPELL_FAILED;
   }

   if (is_immune(victim, -1, RIS_MAGIC) || is_immune(victim, -1, RIS_CHARM))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   if (!IS_NPC(victim) && !IS_NPC(ch))
   {
      send_to_char("I don't think so...\n\r", ch);
      send_to_char("You feel charmed...\n\r", victim);
      return rSPELL_FAILED;
   }
   chance = ris_save(victim, level, RIS_CHARM);
   level = level-45;

   if (IS_AFFECTED(victim, AFF_CHARM)
      || chance == 1000 || IS_AFFECTED(ch, AFF_CHARM) || circle_follow(victim, ch) || saves_spell_staff(chance, victim))
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (victim->max_hit > ch->max_hit*2)
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (victim->max_hit > 3000)
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (victim->max_hit > ch->max_hit)
   {
      chance = 70 + ((((victim->max_hit*100) / ch->max_hit)-100)*30/100);
      if (number_range(1, 100) <= chance)
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }   
   }
   if (victim->master)
      stop_follower(victim);
   add_follower(victim, ch);
   af.type = sn;
   af.duration = level*6+600;
   af.location = 0;
   af.modifier = 0;
   af.bitvector = meb(AFF_CHARM);
   affect_to_char(victim, &af);
/*    act( AT_MAGIC, "Isn't $n just so nice?", ch, NULL, victim, TO_VICT );
    act( AT_MAGIC, "$N's eyes glaze over...", ch, NULL, victim, TO_ROOM );
    if ( ch != victim )
	send_to_char( "Ok.\n\r", ch );*/
   successful_casting(skill, ch, victim, NULL);

   sprintf(buf, "%s has charmed %s.", ch->name, victim->name);
   log_string_plus(buf, LOG_NORMAL, ch->level);
   if (IS_NPC(victim))
   {
      start_hating(victim, ch);
      start_hunting(victim, ch);
   }
/*
    to_channel( buf, CHANNEL_MONITOR, "Monitor", UMAX( LEVEL_IMMORTAL, ch->level ) );
*/
   return rNONE;
}

ch_ret spell_extradimensional_portal(int sn, int level, CHAR_DATA *ch, void *vo)
{
   OBJ_DATA *portal;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   level = POINT_LEVEL(GET_POINTS(ch, sn, isobj, level), GET_MASTERY(ch, sn, isobj, level));

   portal = create_object(get_obj_index(OBJ_EXTRADIMENSIONAL_PORTAL), 0);
   portal->value[0] = 20 + level*4;
   portal->value[2] = 90 - (UMIN(60, level * 3 / 4));
   act(AT_MAGIC, "$p suddenly appears.", ch, portal, NULL, TO_ROOM);
   act(AT_MAGIC, "$p suddenly appears.", ch, portal, NULL, TO_CHAR);
   portal = obj_to_room(portal, ch->in_room, ch);
   return rNONE;
}
ch_ret spell_create_food(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *mushroom;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   mushroom = create_object(get_obj_index(OBJ_VNUM_MUSHROOM), 0);
   mushroom->value[0] = 2 + level/10;
   act(AT_MAGIC, "$p suddenly appears.", ch, mushroom, NULL, TO_ROOM);
   act(AT_MAGIC, "$p suddenly appears.", ch, mushroom, NULL, TO_CHAR);
   mushroom = obj_to_room(mushroom, ch->in_room, ch);
   return rNONE;
}

ch_ret spell_holy_food(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *apple;
   sh_int isobj = 0;
   int numapp;
   sh_int points;
   sh_int mastery;
   char buf[MSL];

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   level = POINT_LEVEL(points, mastery);
   
   level *= 3;
   numapp = 1 + level/100;
   apple = create_object(get_obj_index(number_range(27, 28)), ch->level);
   obj_to_room(apple, ch->in_room, ch);
   if (numapp >= 2)
   {
      apple = create_object(get_obj_index(number_range(27, 28)), ch->level);
      obj_to_room(apple, ch->in_room, ch);
   }
   if (numapp >= 3)
   {
      apple = create_object(get_obj_index(number_range(27, 29)), ch->level);
      obj_to_room(apple, ch->in_room, ch);
   }
   if (numapp >= 4)
   {
      apple = create_object(get_obj_index(number_range(28, 29)), ch->level);
      obj_to_room(apple, ch->in_room, ch);
   }
   if (number_range(1, 100) <= level%100)
   {
      apple = create_object(get_obj_index(number_range(27, 29)), ch->level);
      obj_to_room(apple, ch->in_room, ch);
      numapp++;
   }   
   sprintf(buf, "%d $ps%s suddenly appear.", numapp, char_color_str(AT_MAGIC, ch));
   act(AT_MAGIC, buf, ch, apple, NULL, TO_CHAR);
   act(AT_MAGIC, buf, ch, apple, NULL, TO_ROOM);
   return rNONE;
}

ch_ret spell_create_water(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj = (OBJ_DATA *) vo;
   int water;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (obj->item_type != ITEM_DRINK_CON)
   {
      send_to_char("It is unable to hold water.\n\r", ch);
      return rSPELL_FAILED;
   }

   if (obj->value[2] != LIQ_WATER && obj->value[1] != 0)
   {
      send_to_char("It contains some other liquid.\n\r", ch);
      return rSPELL_FAILED;
   }

   water = 1;

   if (water > 0)
   {
      separate_obj(obj);
      obj->value[2] = LIQ_WATER;
      obj->value[1] += water;
      if (!is_name("water", obj->name))
      {
         char buf[MSL];

         sprintf(buf, "%s water", obj->name);
         STRFREE(obj->name);
         obj->name = STRALLOC(buf);
      }
      act(AT_MAGIC, "$p is filled.", ch, obj, NULL, TO_CHAR);
   }

   return rNONE;
}

ch_ret spell_cure_blindness(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int chance;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance = number_range(1, 100);

   if ((20 + (level * 2)) < chance)
   {
      send_to_char("You attempt to work your cure, but you must of chanted something wrong.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {
      set_char_color(AT_MAGIC, ch);

      if (!is_affected(victim, gsn_blindness))
      {
         if (ch != victim)
            send_to_char("You work your cure, but it has no apparent effect.\n\r", ch);
         else
            send_to_char("You don't seem to be blind.\n\r", ch);
         return rSPELL_FAILED;
      }
      affect_strip(victim, gsn_blindness);
      set_char_color(AT_MAGIC, victim);
      send_to_char("Your vision returns!\n\r", victim);
      if (ch != victim)
         send_to_char("You work your cure, restoring vision.\n\r", ch);
      return rNONE;
   }
}

/* Next 3 are Paladin spells, until we update smaugspells -- Blodkai, 4/97 */

ch_ret spell_sacral_divinity(int sn, int level, CHAR_DATA * ch, void *vo)
{
   AFFECT_DATA af;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (ch->alignment < 350)
   {
      act(AT_MAGIC, "Your prayer goes unanswered.", ch, NULL, NULL, TO_CHAR);
      return rSPELL_FAILED;
   }
   if (IS_AFFECTED(ch, AFF_SANCTUARY))
      return rSPELL_FAILED;
   af.type = sn;
   af.duration = level * 3;
   af.location = APPLY_AFFECT;
   af.modifier = 0;
   af.bitvector = meb(AFF_SANCTUARY);
   affect_to_char(ch, &af);
   act(AT_MAGIC, "A shroud of glittering light slowly wraps itself about $n.", ch, NULL, NULL, TO_ROOM);
   act(AT_MAGIC, "A shroud of glittering light slowly wraps itself around you.", ch, NULL, NULL, TO_CHAR);
   return rNONE;
}

ch_ret spell_expurgation(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int chance;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance = number_range(1, 100);

   if ((70 + level) < chance)
   {
      send_to_char("You attempt to work your cure, but you must of chanted something wrong.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {
      if (!is_affected(victim, gsn_poison))
         return rSPELL_FAILED;
      affect_strip(victim, gsn_poison);
      act(AT_MAGIC, "You speak an ancient prayer, begging your god for purification.", ch, NULL, NULL, TO_CHAR);
      act(AT_MAGIC, "$n speaks an ancient prayer begging $s god for purification.", ch, NULL, NULL, TO_ROOM);
      return rNONE;
   }
}

ch_ret spell_bethsaidean_touch(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int chance;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance = number_range(1, 100);

   if ((70 + level) < chance)
   {
      send_to_char("You attempt to work your cure, but you must of chanted something wrong.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {
      if (!is_affected(victim, gsn_blindness))
         return rSPELL_FAILED;
      affect_strip(victim, gsn_blindness);
      set_char_color(AT_MAGIC, victim);
      send_to_char("Your sight is restored!\n\r", victim);
      if (ch != victim)
      {
         act(AT_MAGIC, "$n lays $s hands over your eyes and concentrates...", ch, NULL, victim, TO_VICT);
         act(AT_MAGIC, "$n lays $s hands over $N's eyes and concentrates...", ch, NULL, victim, TO_NOTVICT);
         act(AT_MAGIC, "Laying your hands on $N's eyes, you pray to life $S blindness.", ch, NULL, victim, TO_CHAR);
      }
      return rNONE;
   }
}


ch_ret spell_holy_cleansing(int sn, int level, CHAR_DATA *ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int chance;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance = number_range(1, 100);

   if ((50 + (level * 2)) < chance)
   {
      send_to_char("You attempt to work your cure, but you must of chanted something wrong.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {
      if (is_affected(victim, skill_lookup("weaken")))
         affect_strip(victim, skill_lookup("weaken"));
      if (mastery >= 2 && is_affected(victim, skill_lookup("faerie fire")))
         affect_strip(victim, skill_lookup("faerie fire"));
      if (mastery >= 2 && is_affected(victim, skill_lookup("poison")))
         affect_strip(victim, skill_lookup("poison"));
      if (mastery >= 3 && is_affected(victim, skill_lookup("blindness")))
         affect_strip(victim, skill_lookup("blindness"));
      if (mastery >= 3 && is_affected(victim, skill_lookup("curse")))
         affect_strip(victim, skill_lookup("curse"));
      if (mastery >= 4 && is_affected(victim, skill_lookup("unravel defense")))
         affect_strip(victim, skill_lookup("unravel defense"));
       
      send_to_char("A holy wave of fresh air overtakes your body.\n\r", victim);
      if (ch != victim)
      {
         act(AT_MAGIC, "A holy wave of fresh air overtakes $N.", ch, NULL, victim, TO_NOTVICT);
         act(AT_MAGIC, "You bless $N's body with a holy wave of fresh air.", ch, NULL, victim, TO_CHAR);
      }
      return rNONE;
   }
}

ch_ret spell_cure_poison(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int chance;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance = number_range(1, 100);

   if ((50 + (level * 2)) < chance)
   {
      send_to_char("You attempt to work your cure, but you must of chanted something wrong.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {
      if (is_affected(victim, gsn_poison))
      {
         affect_strip(victim, gsn_poison);
         set_char_color(AT_MAGIC, victim);
         send_to_char("A warm feeling runs through your body.\n\r", victim);
         victim->mental_state = URANGE(-100, victim->mental_state, -10);
         if (ch != victim)
         {
            act(AT_MAGIC, "A flush of health washes over $N.", ch, NULL, victim, TO_NOTVICT);
            act(AT_MAGIC, "You lift the poison from $N's body.", ch, NULL, victim, TO_CHAR);
         }
         return rNONE;
      }
      else
      {
         set_char_color(AT_MAGIC, ch);
         if (ch != victim)
            send_to_char("You work your cure, but it has no apparent effect.\n\r", ch);
         else
            send_to_char("You don't seem to be poisoned.\n\r", ch);
         return rSPELL_FAILED;
      }
   }
}

ch_ret spell_curse(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (is_immune(victim, -1, RIS_MAGIC))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   if (IS_AFFECTED(victim, AFF_CURSE) || saves_spell_staff(level, victim))
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.duration = 10+level*2;
   af.location = APPLY_TOHIT;
   af.modifier = -1 -(level/30);
   af.bitvector = meb(AFF_CURSE);
   affect_to_char(victim, &af);

   af.location = APPLY_SAVING_SPELL;
   af.modifier = 1 + (level/20);
   affect_to_char(victim, &af);

   set_char_color(AT_MAGIC, victim);
   send_to_char("You feel unclean.\n\r", victim);
   if (ch != victim)
   {
      act(AT_MAGIC, "You utter a curse upon $N.", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "$n utters a curse upon $N.", ch, NULL, victim, TO_NOTVICT);
   }
   return rNONE;
}


ch_ret spell_possess( int sn, int level, CHAR_DATA *ch, void *vo )
{
    CHAR_DATA *victim;
    char buf[MSL];
    AFFECT_DATA af;
    SKILLTYPE *skill = get_skilltype(sn);
    int try;
    sh_int points;
    sh_int mastery;
    sh_int isobj = 0;
    if (level >= 999)
    {
        isobj = 1;
    }
    points = GET_POINTS(ch, sn, isobj, level);
    mastery = GET_MASTERY(ch, sn, isobj, level);

    level = POINT_LEVEL(points, mastery);

    if (ch->desc->original)
    {
	send_to_char("You are not in your original state.\n\r", ch);
	return rSPELL_FAILED;
    }

    if ( (victim = get_char_room_new( ch, target_name, 1 ) ) == NULL)
    {
	send_to_char("They aren't here!\n\r", ch);
	return rSPELL_FAILED;
    }

    if (victim == ch)
    {
	send_to_char("You can't possess yourself!\n\r", ch);
	return rSPELL_FAILED;
    }

    if (!IS_NPC(victim))
    {
	send_to_char("You can't possess another player!\n\r", ch);
	return rSPELL_FAILED;
    }

    if (victim->desc)
    {
	ch_printf(ch, "%s is already possessed.\n\r", victim->short_descr);
	return rSPELL_FAILED;
    }

    if ( is_immune(victim, -1, RIS_MAGIC)
    ||   is_immune(victim, -1, RIS_CHARM) )
    {
	immune_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    try = ris_save( victim, level, RIS_CHARM );

    if ( IS_AFFECTED(victim, AFF_POSSESS)
    ||   IS_AFFECTED(ch, AFF_CHARM)
    ||  victim->desc
    ||   saves_spell_staff( try, victim )
    ||  !chance(ch, 25) )
    {
	failed_casting( skill, ch, victim, NULL );
	return rSPELL_FAILED;
    }

    af.type      = sn;
    af.duration  = (level / 5) + 20;
    af.location  = 0;
    af.modifier  = 0;
    af.bitvector = meb(AFF_POSSESS);
    affect_to_char( victim, &af );

    sprintf(buf, "You have possessed %s!\n\r", victim->short_descr);

    ch->desc->character = victim;
    ch->desc->original  = ch;
    victim->desc        = ch->desc;
    ch->desc            = NULL;
    ch->switched        = victim;
    send_to_char( buf, victim );

    return rNONE;
}

ch_ret spell_detect_poison(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj = (OBJ_DATA *) vo;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   set_char_color(AT_MAGIC, ch);
   if (obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_FOOD || obj->item_type == ITEM_COOK)
   {
      if (obj->item_type == ITEM_COOK && obj->value[2] == 0)
         send_to_char("It looks undercooked.\n\r", ch);
      else if (obj->value[3] != 0)
         send_to_char("You smell poisonous fumes.\n\r", ch);
      else
         send_to_char("It looks very delicious.\n\r", ch);
   }
   else
   {
      send_to_char("It doesn't look poisoned.\n\r", ch);
   }

   return rNONE;
}

/*
 * New version of dispel magic fixes alot of bugs, and allows players
 * to not lose thie affects if they have the spell and the affect.
 * Also prints a message to the victim, and does various other things :)
 * Shaddai
 */

ch_ret spell_dispel_magic(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int cnt = 0, affect_num, affected_by = 0, times = 0;
   int chance;
   int points, mastery;
   SKILLTYPE *skill = get_skilltype(sn);
   AFFECT_DATA *paf;
   bool found = FALSE, twice = FALSE, three = FALSE;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }

   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   level = GET_POINTS(ch, sn, isobj, level);
   set_char_color(AT_MAGIC, ch);

   chance = (get_curr_int(ch) - get_curr_int(victim));

   if (is_immune(victim, -1, RIS_MAGIC))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   if (ch == victim)
   {
      if (ch->first_affect)
      {
         send_to_char("You pass your hands around your body...\n\r", ch);
         while (ch->first_affect)
            affect_remove(ch, ch->first_affect);
         if (!IS_NPC(ch)) /* Stop the NPC bug  Shaddai */
            update_aris(victim);
         return rNONE;
      }
      else
      {
         send_to_char("You pass your hands around your body...\n\r", ch);
         return rNONE;
      }
   }
   if (!IS_AFFECTED(ch, AFF_DETECT_MAGIC))
   {
      send_to_char("You don't sense a magical aura to dispel.\n\r", ch);
      return rERROR; /* You don't cast it so don't attack */
   }

   if (number_percent() > (75 - chance))
   {
      twice = TRUE;
      if (number_percent() > (75 - chance))
         three = TRUE;
   }

 start_loop:

   /* Grab affected_by from mobs first */
   if (IS_NPC(victim) && !xIS_EMPTY(victim->affected_by))
   {
      for (;;)
      {
         affected_by = number_range(0, MAX_AFFECTED_BY - 1);
         if (xIS_SET(victim->affected_by, affected_by))
         {
            found = TRUE;
            break;
         }
         if (cnt++ > 30)
         {
            found = FALSE;
            break;
         }
      }
      if (found) /* Ok lets see if it is a spell */
      {
         for (paf = victim->first_affect; paf; paf = paf->next)
            if (xIS_SET(paf->bitvector, affected_by))
               break;
         if (paf) /*It is a spell lets remove the spell too */
         {

            if (saves_spell_staff(level, victim))
            {
               if (!dispel_casting(paf, ch, victim, FALSE, FALSE))
                  failed_casting(skill, ch, victim, NULL);
               return rSPELL_FAILED;
            }
            if (SPELL_FLAG(get_skilltype(paf->type), SF_NODISPEL))
            {
               if (!dispel_casting(paf, ch, victim, FALSE, FALSE))
                  failed_casting(skill, ch, victim, NULL);
               return rSPELL_FAILED;
            }
            if (!dispel_casting(paf, ch, victim, FALSE, TRUE) && times == 0)
               successful_casting(skill, ch, victim, NULL);
            affect_remove(victim, paf);
            if ((twice && times < 1) || (three && times < 2))
            {
               times++;
               goto start_loop;
            }
            return rNONE;
         }
         else /* Nope not a spell just remove the bit *For Mobs Only* */
         {
            if (saves_spell_staff(level, victim))
            {
               if (!dispel_casting(NULL, ch, victim, affected_by, FALSE))
                  failed_casting(skill, ch, victim, NULL);
               return rSPELL_FAILED;
            }
            if (!dispel_casting(NULL, ch, victim, affected_by, TRUE) && times == 0)
               successful_casting(skill, ch, victim, NULL);
            xREMOVE_BIT(victim->affected_by, affected_by);
            if ((twice && times < 1) || (three && times < 2))
            {
               times++;
               goto start_loop;
            }
            return rNONE;
         }
      }
   }

   /* Ok mob has no affected_by's or we didn't catch them lets go to
    * first_affect. SHADDAI
    */

   if (!victim->first_affect)
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   cnt = 0;

   /*
    * Need to randomize the affects, yes you have to loop on average 1.5 times
    * but dispel magic only takes at worst case 256 uSecs so who cares :)
    * Shaddai
    */

   for (paf = victim->first_affect; paf; paf = paf->next)
      cnt++;

   paf = victim->first_affect;

   for (affect_num = number_range(0, (cnt - 1)); affect_num > 0; affect_num--)
      paf = paf->next;

   if (saves_spell_staff(level, victim))
   {
      if (!dispel_casting(paf, ch, victim, FALSE, FALSE))
         failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   /* Need to make sure we have an affect and it isn't no dispel */
   if (!paf || SPELL_FLAG(get_skilltype(paf->type), SF_NODISPEL))
   {
      if (!dispel_casting(paf, ch, victim, FALSE, FALSE))
         failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (!dispel_casting(paf, ch, victim, FALSE, TRUE) && times == 0)
      successful_casting(skill, ch, victim, NULL);
   affect_remove(victim, paf);
   if ((twice && times < 1) || (three && times < 2))
   {
      times++;
      goto start_loop;
   }

   /* Have to reset victim affects */

   if (!IS_NPC(victim))
      update_aris(victim);
   return rNONE;
}



ch_ret spell_polymorph(int sn, int level, CHAR_DATA * ch, void *vo)
{
   MORPH_DATA *morph;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }

   morph = find_morph(ch, target_name, TRUE);
   if (!morph)
   {
      send_to_char("You can't morph into anything like that!\n\r", ch);
      return rSPELL_FAILED;
   }
   if (!do_morph_char(ch, morph))
   {
      failed_casting(skill, ch, NULL, NULL);
      return rSPELL_FAILED;
   }
   return rNONE;
}

ch_ret spell_earthquake(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *vch;
   CHAR_DATA *vch_next;
   bool ch_died;
   ch_ret retcode;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);


   ch_died = FALSE;
   retcode = rNONE;

   if (is_room_safe(ch))
   {
      failed_casting(skill, ch, NULL, NULL);
      return rSPELL_FAILED;
   }
   if (IS_SET(ch->in_room->area->flags, AFLAG_NOAREA))
   {
      send_to_char("A mystical force in this area blocks your area attack.\n\r", ch);
      failed_casting(skill, ch, NULL, NULL);
      return rSPELL_FAILED;
   }

   act(AT_MAGIC, "The earth trembles beneath your feet!", ch, NULL, NULL, TO_CHAR);
   act(AT_MAGIC, "$n makes the earth tremble and shiver.", ch, NULL, NULL, TO_ROOM);

   for (vch = first_char; vch; vch = vch_next)
   {
      vch_next = vch->next;
      if (!vch->in_room)
         continue;

      if (ch->coord->x != vch->coord->x || ch->coord->y != vch->coord->y
         || ch->map != vch->map)
         continue;

      if (vch->in_room == ch->in_room)
      {
         if (!IS_NPC(vch) && xIS_SET(vch->act, PLR_WIZINVIS) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL)
            continue;
         
         if (IS_NPC(vch) && IS_AFFECTED(vch, AFF_CHARM) && vch->master == ch)
            continue;
         
         if (!IS_NPC(ch) && IS_NPC(vch) && xIS_SET(ch->act, ACT_MOUNTABLE))
            continue;
         
         if (vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch)) && !IS_AFFECTED(vch, AFF_FLYING) && !IS_AFFECTED(vch, AFF_FLOATING))
            retcode = damage(ch, vch, points, sn, 0, -1);
         if (retcode == rCHAR_DIED || char_died(ch))
         {
            ch_died = TRUE;
            break;
         }
         if (char_died(vch))
            continue;
      }

      if (!ch_died && vch->in_room->area == ch->in_room->area)
      {
         if (number_bits(3) == 0)
            send_to_char_color("&BThe earth trembles and shivers.\n\r", vch);
      }
   }

   if (ch_died)
      return rCHAR_DIED;
   else
      return rNONE;
}

ch_ret spell_bless_weapon(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj = (OBJ_DATA *) vo;
   int pAge;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (obj->item_type != ITEM_WEAPON || IS_OBJ_STAT(obj, ITEM_MAGIC) || xIS_SET(obj->extra_flags, ITEM_BLESS) || xIS_SET(obj->extra_flags, ITEM_SANCTIFIED))
   {
      act(AT_MAGIC, "You attempt to put your power into $p, but nothing happens.", ch, obj, NULL, TO_CHAR);
      act(AT_MAGIC, "$n concentrates on $p, but stops with a confused look.", ch, obj, NULL, TO_ROOM);
      return rSPELL_FAILED;
   }
   
   xTOGGLE_BIT(obj->extra_flags, ITEM_BLESS);
   act(AT_MAGIC, "You put your power into $p, and it begins to glow faintly.", ch, obj, NULL, TO_CHAR);
   act(AT_MAGIC, "$n concentrates on $p, and it begins to glow faintly.", ch, obj, NULL, TO_ROOM);
   
   pAge = get_age(ch);
   obj->bless_dur = 30+level*5+URANGE(1, (pAge-18)*3, 300)+(get_curr_wis(ch)-14)*20;
   
   if (obj->bless_dur < 30)
      obj->bless_dur = 30;
   
   return rNONE;
}

ch_ret spell_enchant_weapon(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj = (OBJ_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   int inc;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   separate_obj(obj);
   if (obj->item_type != ITEM_WEAPON || IS_OBJ_STAT(obj, ITEM_MAGIC) || obj->first_affect)
   {
      act(AT_MAGIC, "Your magic twists and winds around $p but cannot take hold.", ch, obj, NULL, TO_CHAR);
      act(AT_MAGIC, "$n's magic twists and winds around $p but cannot take hold.", ch, obj, NULL, TO_NOTVICT);
      return rSPELL_FAILED;
   }
   xSET_BIT(obj->extra_flags, ITEM_MAGIC);
   
   inc = URANGE(1, level/14, 5);
   obj->value[1] += inc;
   
   inc = URANGE(1, level/14, 5);
   obj->value[2] += inc;
   
   if (mastery >= 3)
   {
      obj->value[7]++;
      obj->value[8]++;
      obj->value[9]++;
   }
   if (mastery <= 2)
      obj->value[10] -= 2;
   else
      obj->value[10] -= 1; 
      
   if (obj->item_type != ITEM_WEAPON || IS_OBJ_STAT(obj, ITEM_MAGIC) || obj->first_affect)
   {
      act(AT_MAGIC, "A yellow light shoots out of $p and it starts to glow for a short period of time.", ch, obj, NULL, TO_CHAR);
      act(AT_MAGIC, "$n's casts an enchantment on $p and it starts to glow with a bright yellow light.", ch, obj, NULL, TO_NOTVICT);
      return rSPELL_FAILED;
   }

   return rNONE;
}

ch_ret spell_disenchant_weapon(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj = (OBJ_DATA *) vo;
   AFFECT_DATA *paf;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }

   if (obj->item_type != ITEM_WEAPON)
   {
      send_to_char("You can only disenchant weapons.", ch);
      return rSPELL_FAILED;
   }

   if (!IS_OBJ_STAT(obj, ITEM_MAGIC) || !obj->first_affect)
   {
      send_to_char("This weapon appears to have no enchantments on it.", ch);
      return rSPELL_FAILED;
   }

   if (xIS_SET(obj->pIndexData->extra_flags, ITEM_MAGIC))
   {
      send_to_char("You can't disenchant a weapon that's inherently magical.", ch);
      return rSPELL_FAILED;
   }

   if (xIS_SET(obj->pIndexData->extra_flags, ITEM_ANTI_GOOD) || xIS_SET(obj->pIndexData->extra_flags, ITEM_ANTI_EVIL))
   {
      send_to_char("You can't disenchant a weapon that's inherently good or evil.", ch);
      return rSPELL_FAILED;
   }

   separate_obj(obj);
   for (paf = obj->first_affect; paf; paf = paf->next)
   {
      if (paf->location == APPLY_HITROLL || paf->location == APPLY_DAMROLL)
      {
         UNLINK(paf, obj->first_affect, obj->last_affect, next, prev);
      }
   }

   if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
   {
      xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_EVIL);
      act(AT_BLUE, "$p momentarily absorbs all blue light around it.", ch, obj, NULL, TO_CHAR);
   }
   if (IS_OBJ_STAT(obj, ITEM_ANTI_EVIL))
   {
      xREMOVE_BIT(obj->extra_flags, ITEM_ANTI_GOOD);
      act(AT_RED, "$p momentarily absorbs all red light around it.", ch, obj, NULL, TO_CHAR);
   }

/*    send_to_char( "Ok.\n\r", ch );*/
   successful_casting(get_skilltype(sn), ch, NULL, obj);
   return rNONE;
}

ch_ret spell_wizard_eye(int sn, int level, CHAR_DATA * ch, void *vo)
{
   AFFECT_DATA af;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }

   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   level = POINT_LEVEL(points, mastery);

   if (IS_AFFECTED(ch, AFF_WIZARDEYE) || IS_AFFECTED(ch, AFF_E_WIZARDEYE) || IS_AFFECTED(ch, AFF_M_WIZARDEYE))
      return rSPELL_FAILED;
   af.type = sn;
   af.duration = 1500+(level*15);
   af.location = APPLY_AFFECT;
   af.modifier = 0;
   if (mastery == 4)
      af.bitvector = meb(AFF_M_WIZARDEYE);
   else if (mastery == 3)
      af.bitvector = meb(AFF_E_WIZARDEYE);
   else
      af.bitvector = meb(AFF_WIZARDEYE);
   affect_to_char(ch, &af);

   act(AT_MAGIC, "Your eye's open up to your surroundings.", ch, NULL, NULL, TO_CHAR);
   act(AT_MAGIC, "$n eye's sparkle as they open up to view the world.", ch, NULL, NULL, TO_ROOM);
   return rNONE;
}

ch_ret spell_faerie_fire(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (is_immune(victim, -1, RIS_MAGIC))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.duration = 10+level*2;
   af.location = APPLY_ARMOR;
   af.modifier = -1 - (level/30);
   af.bitvector = meb(AFF_FAERIE_FIRE);
   affect_to_char(victim, &af);
   act(AT_PINK, "You are surrounded by a pink outline.", victim, NULL, NULL, TO_CHAR);
   act(AT_PINK, "$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
   return rNONE;
}



ch_ret spell_faerie_fog(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *ich;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   act(AT_MAGIC, "$n conjures a cloud of purple smoke.", ch, NULL, NULL, TO_ROOM);
   act(AT_MAGIC, "You conjure a cloud of purple smoke.", ch, NULL, NULL, TO_CHAR);

   for (ich = ch->in_room->first_person; ich; ich = ich->next_in_room)
   {
      if (!IN_SAME_ROOM(ich, ch))
         continue;
      if (!IS_NPC(ich) && xIS_SET(ich->act, PLR_WIZINVIS))
         continue;

      if (ich == ch || saves_spell_staff(level, ich))
         continue;

      affect_strip(ich, gsn_invis);
      affect_strip(ich, gsn_mass_invis);
      affect_strip(ich, gsn_sneak);
      affect_strip(ich, gsn_stalk);
      xREMOVE_BIT(ich->affected_by, AFF_HIDE);
      xREMOVE_BIT(ich->affected_by, AFF_INVISIBLE);
      xREMOVE_BIT(ich->affected_by, AFF_SNEAK);
      xREMOVE_BIT(ich->affected_by, AFF_STALK);
      act(AT_MAGIC, "$n is revealed!", ich, NULL, NULL, TO_ROOM);
      act(AT_MAGIC, "You are revealed!", ich, NULL, NULL, TO_CHAR);
   }
   return rNONE;
}

ch_ret spell_harm(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int dam;
   int points, mastery;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (is_immune(victim, -1, RIS_MAGIC))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   dam = UMAX(8, POINT_LEVEL(points, mastery)*6/10);
   if (saves_spell_staff(level, victim))
      dam = UMIN(30, dam / 4);
   dam = UMIN(60, dam);
   return damage(ch, victim, dam, sn, 0, -1);
}


ch_ret spell_knowenemy(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim;
   char hpbuf[MSL];
   char mnbuf[MSL];
   char mvbuf[MSL];
   AFFECT_DATA *paf;
   SKILLTYPE *skill;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;
   sh_int identpnts;
   sh_int mlvl;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (target_name[0] == '\0')
   {
      send_to_char("What should the spell be cast upon?\n\r", ch);
      return rSPELL_FAILED;
   }
   if ((victim = get_char_room_new(ch, target_name, 1)) == NULL)
   {
      send_to_pager("That mobile is not in this room.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Can only cast this spell on NPCs.\n\r", ch);
      return rSPELL_FAILED;
   }
   identpnts = level * number_range(8, 12);
   mlvl = 50;

   if (mlvl * 3 <= identpnts)
   {
      ch_printf(ch, "&G&WName:  &c%-20s", victim->name);
      ch_printf(ch, "\n\r", ch);
   }
   else
   {
      ch_printf(ch, "You try to identify the mobile, but you cannot learn anything.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (mlvl * 6 <= identpnts)
   {
      ch_printf(ch,
         "&G&WStr: &c%2d&G&W )( Int: &c%2d&G&W )( Wis: &c%2d&G&W )( Dex: &c%2d&G&W )( Con: &c%2d&G&W )( Cha: &c%2d&G&W )( Lck: &c%2d&G&W )\n\r",
         get_curr_str(victim), get_curr_int(victim), get_curr_wis(victim), get_curr_dex(victim), get_curr_con(victim), get_curr_cha(victim),
         get_curr_lck(victim));
   }
   if (mlvl * 5 <= identpnts)
   {
      if (victim->race < max_npc_race && victim->race >= 0)
         ch_printf(ch, "&G&WRace   : &c%-2.2d/%-10s\n\r",
            victim->race, print_npc_race(victim->race));
   }
   sprintf(hpbuf, "%d/%d", victim->hit, victim->max_hit);
   sprintf(mnbuf, "%d/%d", victim->mana, victim->max_mana);
   sprintf(mvbuf, "%d/%d", victim->move, victim->max_move);
   if (mlvl * 15 <= identpnts)
   {
      ch_printf(ch, "&G&WHps     : &c%-12s   ", hpbuf);
   }
   if (mlvl * 8 <= identpnts)
   {
      ch_printf(ch, "&G&WMana    : &c%-12s   &G&WMove   : &c%-12s \n\r", mnbuf, mvbuf);
   }
   if (mlvl * 13 <= identpnts)
   {
      ch_printf(ch, "&G&WHitroll : &c%-5d          &G&WDamroll : &c%-5d          ", GET_HITROLL(victim), GET_DAMROLL(victim));
   }
   if (mlvl * 10 <= identpnts)
   {
      ch_printf(ch, "&G&WWimpy  : &c%-5d           \n\r", victim->wimpy);
   }
   if (identpnts >= 720)
   {
      if (mlvl * 17 <= identpnts)
      {
         ch_printf(ch, "&G&WDamdie  : &c%2.2dd%-2.2d+%-3.3d      &G&WAgility : &c%2d             &G&WHitdie : &c%4.4dd%-4.4d+%-5.5d \n\r",
            victim->pIndexData->damnodice,
            victim->pIndexData->damsizedice,
            victim->pIndexData->damplus,
            victim->pIndexData->perm_agi, victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice, victim->pIndexData->hitplus);
      }
   }
   if (mlvl * 13 <= identpnts)
   {
      ch_printf(ch, "&G&WSaves: &c%-2d %-2d %-2d %-2d %-2d    ",
         victim->saving_poison_death, victim->saving_wand, victim->saving_para_petri, victim->saving_breath, victim->saving_spell_staff);
   }
   if (mlvl * 11 <= identpnts)
   {
      ch_printf(ch, "&G&WGold: &c%-7d            \n\r", victim->gold);
   }
   if (identpnts >= 720)
   {
      if (mlvl * 15 <= identpnts)
      {
         if (victim->resistant > 0)
            ch_printf(ch, "&G&WResistant  : &c%s\n\r", flag_string(victim->resistant, ris_flags));
         if (victim->immune > 0)
            ch_printf(ch, "&G&WImmune     : &c%s\n\r", flag_string(victim->immune, ris_flags));
      }
      if (mlvl * 19 <= identpnts)
      {
         if (victim->susceptible > 0)
            ch_printf(ch, "&G&WSusceptible: &c%s\n\r", flag_string(victim->susceptible, ris_flags));
      }
      if (mlvl * 14 <= identpnts)
      {
         ch_printf(ch, "&G&WAttacks    : &c%s\n\r", ext_flag_string(&victim->attacks, attack_flags));
         ch_printf(ch, "&G&WDefenses   : &c%s\n\r", ext_flag_string(&victim->defenses, defense_flags));
      }
      if (mlvl * 16 <= identpnts)
      {
         for (paf = victim->first_affect; paf; paf = paf->next)
         {
            if ((skill = get_skilltype(paf->type)) != NULL)
            {
               if (mlvl * 18 <= identpnts)
               {
                  ch_printf(ch, "G&W%s: &c'%s' mods %s by %d for %d rnds with bits %s.\n\r",
                     skill_tname[skill->type],
                     skill->name, affect_loc_name(paf->location), paf->modifier, paf->duration, affect_bit_name(&paf->bitvector));
               }
               else
               {
                  ch_printf(ch, "G&W%s: &c'%s' mods %s by %d for %d rnds with bits %s.\n\r",
                     skill_tname[skill->type],
                     skill->name, affect_loc_name(paf->location), paf->modifier, paf->duration, affect_bit_name(&paf->bitvector));
               }
            }
         }
      }
   }
   return rNONE;
}

int get_used_imbueslots args((OBJ_DATA *obj));

int code_identify(CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *victim, int sn, char *dbuf)
{
   AFFECT_DATA *paf;
   SKILLTYPE *sktmp;
   SKILLTYPE *skill = get_skilltype(sn);
   char buf[MIL];
   static char sbuf[MSL*2];
   char bsize[100];
   char *name;
   
   strcpy(sbuf, "");
   
   if (obj)
   {
      check_for_trap(ch, obj, -1, NEW_TRAP_IDENTOBJ);
      if (char_died(ch))
         return rSPELL_FAILED;
      if (global_retcode == rOBJ_SCRAPPED)
         return rSPELL_FAILED;
      sprintf(buf, "\n\r&w&CObject '%s' is %s",
/*	  obj->name,*/
         obj->short_descr, aoran(item_type_name(obj)));
      strcat(sbuf, buf);
      if (obj->item_type != ITEM_LIGHT && obj->wear_flags - 1 > 0)
      {
         sprintf(buf, ", with wear location:  %s\n\r", flag_string(obj->wear_flags, w_flags));
         strcat(sbuf, buf);
      }
      else
      {
         sprintf(buf, ".\n\r");         
         strcat(sbuf, buf);
      }
      sprintf(buf, "Special properties:  %s\n\rIts weight is %.2f and value is %d.\n\r", ext_flag_string(&obj->extra_flags, o_flags),
         /* magic_bit_name( obj->magic_flags ), -- unused for now */
         obj->weight, obj->cost);
      strcat(sbuf, buf);
         
      if (obj->sworthrestrict)
      {
         sprintf(buf, "Sworth Restriction of:  %d\n\r", obj->sworthrestrict);         
         strcat(sbuf, buf);
      }
      if (obj->imbueslots)
      {
         sprintf(buf, "Imblue Slots: %d used of %d total\n\r", get_used_imbueslots(obj), obj->imbueslots);
         strcat(sbuf, buf);
      }
      sprintf(buf, "&w&B");
      strcat(sbuf, buf);
      
      switch (obj->item_type)
      {
         case ITEM_CONTAINER:
            sprintf(buf, "%s appears to be %s.\n\r", capitalize(obj->short_descr),
               obj->value[0] < 76 ? "of a small capacity" :
               obj->value[0] < 150 ? "of a small to medium capacity" :
               obj->value[0] < 300 ? "of a medium capacity" :
               obj->value[0] < 550 ? "of a medium to large capacity" : obj->value[0] < 751 ? "of a large capacity" : "of a giant capacity");
            strcat(sbuf, buf); 
            sprintf(buf, "Has a weight modification of %d.\n\r", obj->value[2] > 0 ? obj->value[2] : 100);
            strcat(sbuf, buf);
            break;

         case ITEM_SCROLL:
         case ITEM_POTION:
            sprintf(buf, "Power level %s spells of:", get_wplevel(obj->value[5]));
            strcat(sbuf, buf);
            if (obj->value[1] >= 0 && obj->value[1] < top_sn)
            {
               sprintf(buf, " '");
               strcat(sbuf, buf);
               sprintf(buf, skill_table[obj->value[1]]->name);
               strcat(sbuf, buf);
               sprintf(buf, "'");
               strcat(sbuf, buf);
            }
            if (obj->value[2] >= 0 && obj->value[2] < top_sn)
            {
               sprintf(buf, " '");
               strcat(sbuf, buf);
               sprintf(buf, skill_table[obj->value[2]]->name);
               strcat(sbuf, buf);
               sprintf(buf, "'");
               strcat(sbuf, buf);
            }
            if (obj->value[3] >= 0 && obj->value[3] < top_sn)
            {
               sprintf(buf, " '");
               strcat(sbuf, buf);
               sprintf(buf, skill_table[obj->value[3]]->name);
               strcat(sbuf, buf);
               sprintf(buf, "'");
               strcat(sbuf, buf);
            }
            sprintf(buf, ".\n\r");
            break;
         case ITEM_SPELLBOOK:
            sprintf(buf, "Contains knowledge obtainable at level %d and mastery %d of", obj->value[0], obj->value[2]);
            strcat(sbuf, buf);
            if (obj->value[1] >= 0 && obj->value[1] < top_sn)
            {
               sprintf(buf, " '");
               strcat(sbuf, buf);
               sprintf(buf, skill_table[obj->value[1]]->name);
               strcat(sbuf, buf);
               sprintf(buf, "'");
               strcat(sbuf, buf);
            }
            sprintf(buf, ".\n\r");
            strcat(sbuf, buf);
            break;
            
         case ITEM_TGEM:
           sprintf(buf, "    Has a Slot Value of %d\n\r", obj->value[12] == 0 ? 1 : obj->value[12]);
           strcat(sbuf, buf);
           sprintf(buf, showgemaff(ch, obj, 1, NULL));
           strcat(sbuf, buf);
           break;           

         case ITEM_PROJECTILE:
            sprintf(buf, "Damage is %d to %d (average %d)%s\n\r",
               obj->value[1], obj->value[2], (obj->value[1] + obj->value[2]) / 2, IS_OBJ_STAT(obj, ITEM_POISONED) ? ", and is poisonous." : ".");
            strcat(sbuf, buf);
            sprintf(buf, "StabMod %d\n\r", obj->value[9]);
            strcat(sbuf, buf);
            break;
         case ITEM_MISSILE_WEAPON:
            sprintf(buf, "Damage is %d to %d (average %d)%s\n\r",
               obj->value[1], obj->value[2], (obj->value[1] + obj->value[2]) / 2, IS_OBJ_STAT(obj, ITEM_POISONED) ? ", and is poisonous." : ".");
            strcat(sbuf, buf);
            sprintf(buf, "Has a weapon size of %d\n\r", obj->value[3]);
            strcat(sbuf, buf);
            sprintf(buf, "StabMod %d\n\r", obj->value[9]);
            strcat(sbuf, buf);
            sprintf(buf, "Condition %d\n\r", obj->value[0]);
            strcat(sbuf, buf);
            sprintf(buf, "Durability %d\n\r", obj->value[10]);
            strcat(sbuf, buf);
            sprintf(buf, "Range %d\n\r", obj->value[4]);
            strcat(sbuf, buf);
            if (obj->bless_dur > 0)
            {
               sprintf(buf, "Has %d more attacks before Bless wears off.\n\r", obj->bless_dur);
               strcat(sbuf, buf);
            }
            break; 

         case ITEM_WEAPON:           
            if (IS_OBJ_STAT(obj, ITEM_TWOHANDED) && ch)
            {
               int tpoints = POINT_LEVEL(LEARNED(ch, gsn_weapon_twohanded), MASTERED(ch, gsn_weapon_twohanded));
               int tmod = obj->value[3] - race_table[ch->race]->weaponmin + 1;
               int tpercent = 0;
               int hidam = obj->value[2];
               int lodam = obj->value[1];
               int tohitadd = 0;
               if (tmod >= 5 && LEARNED(ch, gsn_weapon_twohanded))
               {
                  if (tpoints >= 90)
                     tohitadd=3;
                  else if (tpoints >= 60)
                     tohitadd=2;
                  else if (tpoints >= 30)
                     tohitadd=1;
                  tpercent += tpoints*2/5;
                  if (tmod == 5)
                  {
                     lodam += lodam * tpercent / 100;
                     hidam += hidam * tpercent / 100;
                  }
                  if (tmod == 6)
                  {
                     lodam += lodam * tpercent * 2 / 100;
                     hidam += hidam * tpercent * 2 / 100;
                  }
                  if (tmod == 7)
                  {
                     lodam += lodam * tpercent * 3 / 100;
                     hidam += hidam * tpercent * 3 / 100;
                  }
               }
               sprintf(buf, "Damage is %d to %d (average %d)%s\n\r",
                  lodam, hidam, (lodam + hidam) / 2, IS_OBJ_STAT(obj, ITEM_POISONED) ? ", and is poisonous." : ".");    
               strcat(sbuf, buf);
               sprintf(buf, "Has a weapon size of %d\n\r", obj->value[3]);
               strcat(sbuf, buf);
               sprintf(buf, "BashMod %d  SlashMod %d StabMod %d\n\r", obj->value[7]+tohitadd, obj->value[8]+tohitadd, obj->value[9]+tohitadd);
               strcat(sbuf, buf);    
            }
            else
            {
               sprintf(buf, "Damage is %d to %d (average %d)%s\n\r",
                  obj->value[1], obj->value[2], (obj->value[1] + obj->value[2]) / 2, IS_OBJ_STAT(obj, ITEM_POISONED) ? ", and is poisonous." : ".");
               strcat(sbuf, buf);
               sprintf(buf, "Has a weapon size of %d\n\r", obj->value[3]);
               strcat(sbuf, buf);
               sprintf(buf, "BashMod %d  SlashMod %d StabMod %d\n\r", obj->value[7], obj->value[8], obj->value[9]);
               strcat(sbuf, buf);
            }
            sprintf(buf, "Condition %d\n\r", obj->value[0]);
            strcat(sbuf, buf);
            sprintf(buf, "Durability %d\n\r", obj->value[10]);
            strcat(sbuf, buf);
            sprintf(buf, "Parry Mod %d  Parry Block Mod %d\n\r", obj->value[12], obj->value[13]);
            strcat(sbuf, buf);
            if (obj->value[4] >= 1 && obj->value[4] < top_sn)
            {
               sprintf(buf, "Has the spell '%s' at the power level of %s\n\r", skill_table[obj->value[4]]->name, get_wplevel(obj->value[5]));
               strcat(sbuf, buf);
            }
            if (obj->bless_dur > 0)
            {
               sprintf(buf, "Has %d more attacks before Bless wears off.\n\r", obj->bless_dur);
               strcat(sbuf, buf);
            }
            break;
               
         case ITEM_REPAIR:
            sprintf(buf, "Uses %d/%d      Fix Percent: %d      Points fixed: %d\n\r", obj->value[0], obj->value[1],
               obj->value[2], obj->value[3]);
            strcat(sbuf, buf);
            break;

         case ITEM_ARMOR:
            if (IS_SET(obj->wear_flags, ITEM_WEAR_SHIELD))   
            {
               sprintf(buf, "Condition %d/%d     Block Percent %d     Battle Lag %d\n\r", obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
               strcat(sbuf, buf);
            }
            else
            {
               sprintf(buf, "BashMod %d SlashMod %d StabMod %d.\n\r", obj->value[0], obj->value[1], obj->value[2]);
               strcat(sbuf, buf);
               sprintf(buf, "Condition %d\n\r", obj->value[3]);
               strcat(sbuf, buf);
               sprintf(buf, "Durability %d\n\r", obj->value[4]);
               strcat(sbuf, buf);
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
               sprintf(buf, "ArmorSize %s\n\r", bsize); 
               strcat(sbuf, buf);
            }
            break;
      }
      for (paf = obj->pIndexData->first_affect; paf; paf = paf->next)
         strcat(sbuf, showaffect(ch, paf, 1));

      for (paf = obj->first_affect; paf; paf = paf->next)
         strcat(sbuf, showaffect(ch, paf, 1));

      if (dbuf)
         sprintf(dbuf, sbuf);
      else
         send_to_char(sbuf, ch);

      return rNONE;
   }
   else if (victim)
   {
      if (IS_SET(victim->immune, RIS_MAGIC))
      {
         immune_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }

      /* If they are morphed or a NPC use the appropriate short_desc otherwise
       * use their name -- Shaddai
       */

      if (victim->morph && victim->morph->morph)
         name = capitalize(victim->morph->morph->short_desc);
      else if (IS_NPC(victim))
         name = capitalize(victim->short_descr);
      else
         name = PERS_MAP(victim, ch);

      if (IS_NPC(victim) && victim->morph)
         ch_printf(ch, "%s appears to truly be %s.\n\r", name, PERS_MAP(victim, ch));
         
      ch_printf(ch, "%s looks like %s.\n\r", name, aoran(get_race(victim)));
      
      ch_printf(ch, "%s appears to be affected by: ", name);

      if (!victim->first_affect)
      {
         send_to_char("nothing.\n\r", ch);
         return rNONE;
      }

      for (paf = victim->first_affect; paf; paf = paf->next)
      {
         if (victim->first_affect != victim->last_affect)
         {
            if (paf != victim->last_affect && (sktmp = get_skilltype(paf->type)) != NULL)
               ch_printf(ch, "%s, ", sktmp->name);
            if (paf == victim->last_affect && (sktmp = get_skilltype(paf->type)) != NULL)
            {
               ch_printf(ch, "and %s.\n\r", sktmp->name);
               return rNONE;
            }
         }
         else
         {
            if ((sktmp = get_skilltype(paf->type)) != NULL)
               ch_printf(ch, "%s.\n\r", sktmp->name);
            else
               send_to_char("\n\r", ch);
            return rNONE;
         }
      }
   }
   return rSPELL_FAILED;
}

ch_ret spell_identify(int sn, int level, CHAR_DATA * ch, void *vo)
{
/* Modified by Scryn to work on mobs/players/objs */
/* Made it show short descrs instead of keywords, seeing as you need
   to know the keyword anyways, we may as well make it look nice -- Alty */
   OBJ_DATA *obj;
   CHAR_DATA *victim;
   sh_int points;
   sh_int mastery;
   sh_int chance1;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (target_name[0] == '\0')
   {
      send_to_char("What should the spell be cast upon?\n\r", ch);
      return rSPELL_FAILED;
   }

   chance1 = number_range(1, 100);

   if ((55 + level) < chance1)
   {
      send_to_char("You attempt to use identify but you get distracted by the item.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {      
      if ((obj = get_obj_carry(ch, target_name)) != NULL)
      {
         return code_identify(ch, obj, NULL, sn, NULL);
      }
      else if ((victim = get_char_room_new(ch, target_name, 1)) != NULL)
      {
         return code_identify(ch, NULL, victim, sn, NULL);
      }
      else
      {
         ch_printf(ch, "You can't find %s!\n\r", target_name);
         return rSPELL_FAILED;
      }  
   }
}

ch_ret spell_invis(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int chance1;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance1 = number_range(1, 100);

   if ((70 + level) < chance1)
   {
      send_to_char("You attempt to cast invis but it seems your mind is elsewhere.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {

/* Modifications on 1/2/96 to work on player/object - Scryn */

      if (!target_name)
         victim = (CHAR_DATA *) vo;
      else if (target_name[0] == '\0')
         victim = ch;
      else
         victim = get_char_room_new(ch, target_name, 1);

      if (victim)
      {
         AFFECT_DATA af;

         if (IS_AFFECTED(victim, AFF_INVISIBLE))
         {
            failed_casting(skill, ch, victim, NULL);
            return rSPELL_FAILED;
         }

         act(AT_MAGIC, "$n fades out of existence.", victim, NULL, NULL, TO_ROOM);
         af.type = sn;
         af.duration = 1500+level*15;
         af.location = APPLY_NONE;
         af.modifier = 0;
         af.bitvector = meb(AFF_INVISIBLE);
         affect_to_char(victim, &af);
         act(AT_MAGIC, "You fade out of existence.", victim, NULL, NULL, TO_CHAR);


         return rNONE;
      }
      else
      {
         OBJ_DATA *obj;

         obj = get_obj_carry(ch, target_name);

         if (obj)
         {
            separate_obj(obj); /* Fix multi-invis bug --Blod */
            if (IS_OBJ_STAT(obj, ITEM_INVIS) || chance(ch, 40 + level / 10))
            {
               failed_casting(skill, ch, NULL, NULL);
               return rSPELL_FAILED;
            }

            xSET_BIT(obj->extra_flags, ITEM_INVIS);
            act(AT_MAGIC, "$p fades out of existence.", ch, obj, NULL, TO_CHAR);
            return rNONE;
         }
      }
      ch_printf(ch, "You can't find %s!\n\r", target_name);
      return rSPELL_FAILED;
   }
}

ch_ret spell_locate_object(int sn, int level, CHAR_DATA * ch, void *vo)
{
   char buf[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *in_obj;
   int cnt, found = 0;
   sh_int points;
   sh_int mastery;
   sh_int chance1;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance1 = number_range(1, 100);

   if ((35 + level) < chance1)
   {
      send_to_char("You attempt to cast locate, but you lose focus.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {

      for (obj = first_object; obj; obj = obj->next)
      {
         if (!can_see_obj_map(ch, obj) || !nifty_is_name(target_name, obj->name))
            continue;
         if ((IS_OBJ_STAT(obj, ITEM_PROTOTYPE) || IS_OBJ_STAT(obj, ITEM_NOLOCATE)) && !IS_IMMORTAL(ch))
            continue;
            
         found++;

         for (cnt = 0, in_obj = obj; in_obj->in_obj && cnt < 100; in_obj = in_obj->in_obj, ++cnt)
            ;
         if (cnt >= MAX_NEST)
         {
            sprintf(buf, "spell_locate_obj: object [%d] %s is nested more than %d times!", obj->pIndexData->vnum, obj->short_descr, MAX_NEST);
            bug(buf, 0);
            continue;
         }

         if (in_obj->carried_by)
         {
            if (IS_IMMORTAL(in_obj->carried_by)
               && !IS_NPC(in_obj->carried_by)
               && (get_trust(ch) < in_obj->carried_by->pcdata->wizinvis) && xIS_SET(in_obj->carried_by->act, PLR_WIZINVIS))
            {
               found--;
               continue;
            }

            sprintf(buf, "%s carried by %s.\n\r", obj_short(obj), PERS_MAP(in_obj->carried_by, ch));
         }
         else
         {
            sprintf(buf, "%s in %s.\n\r", obj_short(obj), in_obj->in_room == NULL ? "the bank" : in_obj->in_room->name);
         }

         buf[0] = UPPER(buf[0]);
         set_char_color(AT_MAGIC, ch);
         send_to_char(buf, ch);
      }

      if (!found)
      {
         send_to_char("Nothing like that exists.\n\r", ch);
         return rSPELL_FAILED;
      }
      return rNONE;
   }
}

ch_ret spell_pass_door(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int chance1;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);
   level = level / 4;
   chance1 = number_range(1, 100);

   if ((40 + level) < chance1)
   {
      send_to_char("You attempt to phase out but you fail in the process.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {
      if (IS_AFFECTED(victim, AFF_PASS_DOOR))
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }
      af.type = sn;
      af.duration = 1000+level*10;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb(AFF_PASS_DOOR);
      affect_to_char(victim, &af);
      act(AT_MAGIC, "$n turns translucent.", victim, NULL, NULL, TO_ROOM);
      act(AT_MAGIC, "You turn translucent.", victim, NULL, NULL, TO_CHAR);
      return rNONE;
   }
}



ch_ret spell_poison(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
   int chance;
   bool first = TRUE;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery)/2;
   chance = ris_save(victim, level, RIS_POISON);
   level = level-30;
   if (chance == 1000 || saves_poison_death(chance, victim))
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("Your magic fails to take hold.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (IS_AFFECTED(victim, AFF_POISON))
      first = FALSE;
   af.type = sn;
   af.duration = 10+level*2;
   af.location = APPLY_STR;
   af.modifier = -1 - (level/20);
   af.bitvector = meb(AFF_POISON);
   affect_join(victim, &af);
   set_char_color(AT_GREEN, victim);
   send_to_char("You feel very sick.\n\r", victim);
   victim->mental_state = URANGE(20, victim->mental_state + (first ? 5 : 0), 100);
   if (ch != victim)
   {
      act(AT_GREEN, "$N shivers as your poison spreads through $S body.", ch, NULL, victim, TO_CHAR);
      act(AT_GREEN, "$N shivers as $n's poison spreads through $S body.", ch, NULL, victim, TO_NOTVICT);
   }
   return rNONE;
}


ch_ret spell_remove_curse(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj;
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int chance1;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance1 = number_range(1, 100);

   if ((30 + level) <= chance1)
   {
      send_to_char("You attempt to work your cure, but you must of chanted something wrong.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {

      if (is_affected(victim, gsn_curse))
      {
         affect_strip(victim, gsn_curse);
         set_char_color(AT_MAGIC, victim);
         send_to_char("The weight of your curse is lifted.\n\r", victim);
         if (ch != victim)
         {
            act(AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR);
            act(AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT);
         }
      }
      else if (victim->first_carrying)
      {
         for (obj = victim->first_carrying; obj; obj = obj->next_content)
            if (!obj->in_obj && IS_OBJ_STAT(obj, ITEM_NOREMOVE))
            {
               if (IS_OBJ_STAT(obj, ITEM_NOREMOVE))
                  xREMOVE_BIT(obj->extra_flags, ITEM_NOREMOVE);
               set_char_color(AT_MAGIC, victim);
               send_to_char("You feel a burden released.\n\r", victim);
               if (ch != victim)
               {
                  act(AT_MAGIC, "You dispel the curses afflicting $N.", ch, NULL, victim, TO_CHAR);
                  act(AT_MAGIC, "$n's dispels the curses afflicting $N.", ch, NULL, victim, TO_NOTVICT);
               }
               return rNONE;
            }
      }
      return rNONE;
   }
}

ch_ret spell_remove_trap(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj;
   OBJ_DATA *trap;
   TRAP_DATA *ntrap;
   int retcode;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int isobj = 0;
   int percent;
   
   ntrap = NULL;
   trap = NULL;

   if (level >= 999)
   {
      isobj = 1;
   }

   if (!target_name || target_name[0] == '\0')
   {
      send_to_char("Remove trap on what?\n\r", ch);
      return rSPELL_FAILED;
   }

   if ((obj = get_obj_here(ch, target_name)) == NULL)
   {
      send_to_char("You can't find that here.\n\r", ch);
      return rSPELL_FAILED;
   }
   
   trap = get_trap(obj);
   if (obj->trap)
   {
      ntrap = obj->trap;
      trap = NULL;
   }
   if (!trap && !ntrap)
   {
      failed_casting(skill, ch, NULL, NULL);
      return rSPELL_FAILED;
   }
   percent = UMIN(100, 50 + (level*2/3) + URANGE(-8, (14-get_curr_int(ch))*2, 12));
   if (ntrap)
      percent -= ntrap->difficulty;
   if (number_range(1, 100) > percent)
   {
      send_to_char("Ooops!\n\r", ch);
      retcode = pre_spring_trap(ch, trap, ntrap, obj);
      if (retcode == rNONE)
         retcode = rSPELL_FAILED;
      return retcode;
   }

   if (ntrap)
   {
      if (ntrap->uid >= START_INV_TRAP)
      {
         UNLINK(ntrap, first_trap, last_trap, next, prev);
         ntrap->obj->trap = NULL;
         DISPOSE(ntrap);   
         save_trap_file(NULL, NULL);
      }
      else
      {
         ntrap->obj->trap = NULL;
         ntrap->obj = NULL;
         ntrap->area = NULL;
      }
   }
   if (trap)
      extract_obj(trap);

   successful_casting(skill, ch, NULL, NULL);
   return rNONE;
}

ch_ret spell_eye_of_god(int sn, int level, CHAR_DATA *ch, void *vo)
{
   int points;
   int mastery;
   int isobj = 0;
   INTRO_DATA *intro;
   CHAR_DATA *victim;
   int percent;
   
   if (level >= 999)
   {
      isobj = 1;
   }
   
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);
   level = POINT_LEVEL(points, mastery);
   
   if ((victim = get_char_room_new(ch, target_name, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return rSPELL_FAILED;
   }
   if (victim == ch)
   {
      send_to_char("You already know yourself I hope.\n\r", ch);
      return rSPELL_FAILED;
   }
   percent = URANGE(5, 15+level+(get_curr_wis(ch)-14)+(get_curr_int(ch)-14), 85);
   
   if (percent <= number_range(1, 100)) //success
   {
      set_char_color(AT_MAGIC, ch);
      ch_printf(ch, "The heavens open up and reveil the identify of your target as %s\n\r", victim->name);
      if (mastery == 4) //Actually remember now..
      {
         for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
         {
            if (victim->pcdata->pid == intro->pid)
            {
               intro->value = 150000;
               intro->lastseen = time(0);
               break;
            }
         }
         if (!intro)
         {
            CREATE(intro, INTRO_DATA, 1);
            intro->value = 150000;
            intro->pid = victim->pcdata->pid;
            intro->lastseen = time(0);
            LINK(intro, ch->pcdata->first_introduction, ch->pcdata->last_introduction, next, prev);
         }   
         ch_printf(ch, "You now will remember %s\n\r", victim->name);
      }
      return rNONE;
   }
   else //failure
   {
      act(AT_MAGIC, "You try to peer into $N's soul to discover $S identify but fail!", ch, NULL, victim, TO_CHAR);
      if (number_range(1, 10) >= 8)
      {
         act(AT_MAGIC, "$N attempts to peer into your soul and discover who you are!", victim, NULL, ch, TO_VICT);
         act(AT_MAGIC, "You botched that spell so much that $N now knows you attempted it!", ch, NULL, victim, TO_CHAR);
      }
      return rSPELL_FAILED;
   }
   return rNONE;
}
   
   
ch_ret spell_sleep(int sn, int level, CHAR_DATA * ch, void *vo)
{
   AFFECT_DATA af;
   int retcode;
   int chance;
   int tmp;
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if ((victim = get_char_room_new(ch, target_name, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return rSPELL_FAILED;
   }

   if (!IS_NPC(victim) && victim->fighting)
   {
      send_to_char("You cannot sleep a fighting player.\n\r", ch);
      return rSPELL_FAILED;
   }

   if (is_safe(ch, victim))
      return rSPELL_FAILED;

   if (is_immune(victim, -1, RIS_MAGIC))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   if (SPELL_FLAG(skill, SF_PKSENSITIVE) && !IS_NPC(ch) && !IS_NPC(victim))
      tmp = level / 2;
   else
      tmp = level;

   if (IS_AFFECTED(victim, AFF_SLEEP)
      || (chance = ris_save(victim, tmp, RIS_SLEEP)) == 1000
      || (victim != ch && is_safe(ch, victim)) || saves_spell_staff(chance*.6, victim))
   {
      failed_casting(skill, ch, victim, NULL);
      if (ch == victim)
         return rSPELL_FAILED;
      if (!victim->fighting)
      {
         retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
         if (retcode == rNONE)
            retcode = rSPELL_FAILED;
         return retcode;
      }
   }
   if (victim->max_hit > ch->max_hit*2)
      return rSPELL_FAILED;
      
   af.type = sn;
   af.duration = 10+level*3;
   af.location = APPLY_NONE;
   af.modifier = 0;
   af.bitvector = meb(AFF_SLEEP);
   affect_join(victim, &af);

   /* Added by Narn at the request of Dominus. */
   if (!IS_NPC(victim))
   {
      sprintf(log_buf, "%s has cast sleep on %s.", ch->name, victim->name);
      log_string_plus(log_buf, LOG_NORMAL, ch->level);
      to_channel(log_buf, CHANNEL_MONITOR, "Monitor", UMAX(LEVEL_IMMORTAL, ch->level));
   }

   if (IS_AWAKE(victim))
   {
      act(AT_MAGIC, "You feel very sleepy ..... zzzzzz.", victim, NULL, NULL, TO_CHAR);
      act(AT_MAGIC, "$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
      victim->position = POS_SLEEPING;
   }
   if (IS_NPC(victim))
      start_hating(victim, ch);

   return rNONE;
}

ch_ret spell_summon(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim;
   char buf[MSL];
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if ((victim = get_char_world(ch, target_name)) == NULL
      || victim == ch
      || !victim->in_room
      || xIS_SET(ch->in_room->room_flags, ROOM_NO_ASTRAL)
      || wIS_SET(ch, ROOM_NO_ASTRAL)
      || xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
      || wIS_SET(victim, ROOM_PRIVATE)
      || xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
      || wIS_SET(victim, ROOM_SOLITARY)
      || xIS_SET(victim->in_room->room_flags, ROOM_IMP)
      || xIS_SET(victim->in_room->room_flags, ROOM_NO_SUMMON)
      || wIS_SET(victim, ROOM_NO_SUMMON)
      || xIS_SET(victim->in_room->room_flags, ROOM_NO_RECALL)
      || (IS_NPC(victim) && xIS_SET(victim->act, ACT_TRAINER))
      || in_hellmaze(ch)
      || in_hellmaze(victim)
      || victim->ship || ch->ship
      || xIS_SET(ch->act, PLR_PORTALHUNT)
      || xIS_SET(victim->act, PLR_PORTALHUNT)
      || ch->in_room->area != victim->in_room->area
      || victim->fighting
      || (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE))
      || (IS_NPC(victim) && saves_spell_staff(level*.4, victim))
      || (!IS_NPC(victim) && victim->pcdata->caste < 2)
      || (!IS_NPC(ch) && !IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_NOSUMMON))
      || (!IS_NPC(victim) && xIS_SET(victim->act, PLR_GAMBLER)))
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (IS_NPC(victim))
   {
      send_to_char("Cannot summon NPCs any longer.\n\r", ch);
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (ch->in_room->area != victim->in_room->area)
   {
      if (((IS_NPC(ch) != IS_NPC(victim)) && chance(ch, 30)) || ((IS_NPC(ch) == IS_NPC(victim)) && chance(ch, 60)))
      {
         failed_casting(skill, ch, victim, NULL);
         set_char_color(AT_MAGIC, victim);
         send_to_char("You feel a strange pulling sensation...\n\r", victim);
         return rSPELL_FAILED;
      }
   }

   if (!IS_NPC(ch))
   {
      act(AT_MAGIC, "You feel a wave of nausea overcome you...", ch, NULL, NULL, TO_CHAR);
      act(AT_MAGIC, "$n collapses, stunned!", ch, NULL, NULL, TO_ROOM);
      ch->position = POS_STUNNED;

      sprintf(buf, "%s summoned %s to room %d.", ch->name, victim->name, ch->in_room->vnum);
      log_string_plus(buf, LOG_NORMAL, ch->level);
      to_channel(buf, CHANNEL_MONITOR, "Monitor", UMAX(LEVEL_IMMORTAL, ch->level));
   }

   act(AT_MAGIC, "$n disappears suddenly.", victim, NULL, NULL, TO_ROOM);
   char_from_room(victim);
   if (victim->on)
   {
      victim->on = NULL;
      victim->position = POS_STANDING;
   }
   if (victim->position != POS_STANDING)
   {
      victim->position = POS_STANDING;
   }
   char_to_room(victim, ch->in_room);
   victim->coord->x = ch->coord->x;
   victim->coord->y = ch->coord->y;
   victim->map = ch->map;
   if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
   {
      SET_ONMAP_FLAG(victim);
   }
   else
   {
      REMOVE_ONMAP_FLAG(victim);
   }
   update_objects(victim, victim->map, victim->coord->x, victim->coord->y);
   if (victim->mount)
   {
      char_from_room(victim->mount);
      char_to_room(victim->mount, ch->in_room);
      victim->mount->coord->x = ch->coord->x;
      victim->mount->coord->y = ch->coord->y;
      victim->mount->map = victim->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->mount);
      else
         SET_ONMAP_FLAG(victim->mount);
   }  
   if (!IS_NPC(victim) && victim->pcdata->pet)
   {
      char_from_room(victim->pcdata->pet);
      char_to_room(victim->pcdata->pet, ch->in_room);
      victim->pcdata->pet->coord->x = ch->coord->x;
      victim->pcdata->pet->coord->y = ch->coord->y;
      victim->pcdata->pet->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->pcdata->pet);
      else
         SET_ONMAP_FLAG(victim->pcdata->pet);
   }  
   if (!IS_NPC(victim) && victim->pcdata->mount && !victim->mount)
   {
      char_from_room(victim->pcdata->mount);
      char_to_room(victim->pcdata->mount, ch->in_room);
      victim->pcdata->mount->coord->x = ch->coord->x;
      victim->pcdata->mount->coord->y = ch->coord->y;
      victim->pcdata->mount->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->pcdata->mount);
      else
         SET_ONMAP_FLAG(victim->pcdata->mount);
   }  
   if (victim->rider)
   {
      char_from_room(victim->rider);
      char_to_room(victim->rider, ch->in_room);
      victim->rider->coord->x = ch->coord->x;
      victim->rider->coord->y = ch->coord->y;
      victim->rider->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->rider);
      else
         SET_ONMAP_FLAG(victim->rider);
      update_objects(victim->rider, victim->rider->map, victim->rider->coord->x, victim->rider->coord->y);
   } 
   if (victim->riding)
   {
      char_from_room(victim->riding);
      char_to_room(victim->riding, ch->in_room);
      victim->riding->coord->x = ch->coord->x;
      victim->riding->coord->y = ch->coord->y;
      victim->riding->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(victim->riding);
      else
         SET_ONMAP_FLAG(victim->riding);
      update_objects(victim->riding, victim->riding->map, victim->riding->coord->x, victim->riding->coord->y);
   }  

   act(AT_MAGIC, "$n arrives suddenly.", victim, NULL, NULL, TO_ROOM);
   act(AT_MAGIC, "$N has summoned you!", victim, NULL, ch, TO_CHAR);
   do_look(victim, "auto");
   return rNONE;
}

/*
 * Travel via the astral plains to quickly travel to desired location
 *	-Thoric
 *
 * Uses SMAUG spell messages is available to allow use as a SMAUG spell
 */
ch_ret spell_astral_walk(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim;
   struct skill_type *skill = get_skilltype(sn);
   sh_int isobj = 0;
   int p = 0;
   int count = 0;
   int fnd = 0;
   int x, y, map, vnum;

   if (level >= 999)
   {
      isobj = 1;
   }
   if (isdigit(target_name[0]))
   {
      if (can_use_portal(ch, 1) == FALSE)
         return rSPELL_FAILED;

      for (p = 0; p < sysdata.last_portal; p++)
      {
         if (xIS_SET(ch->pcdata->portalfnd, p))
         {
            count++;
         }
         if (count == atoi(target_name))
         {
            fnd++;
            break;
         }
      }
      if (fnd == 0)
      {
         send_to_char("What you selected is not a choice, select again.\n\r", ch);
         return rSPELL_FAILED;
      }
      x = portal_show[p]->x;
      y = portal_show[p]->y;
      map = portal_show[p]->map;
      vnum = OVERLAND_SOLAN;;
   }
   else if (!str_cmp(target_name, "home"))
   {
      if (can_use_portal(ch, 2) == FALSE)
         return rSPELL_FAILED;

      x = -1;
      y = -1;
      map = -1;
      vnum = ROOM_VNUM_PORTAL;
   }
   else
   {
      if ((victim = get_char_world(ch, target_name)) == NULL
         || !can_astral(ch, victim)
         || in_hellmaze(ch)
         || in_hellmaze(victim)
         || victim->ship || ch->ship
         || xIS_SET(ch->act, PLR_PORTALHUNT)
         || ch->in_room->area != victim->in_room->area
         || (!IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_NOSUMMON))
         || (IS_NPC(victim) && xIS_SET(victim->act, ACT_TRAINER)) || (!IS_NPC(ch) && xIS_SET(ch->act, PLR_GAMBLER)))
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }
      x = victim->coord->x;
      y = victim->coord->y;
      map = victim->map;
      vnum = victim->in_room->vnum;
   }

   act(AT_MAGIC, "$n disappears in a flash of light!", ch, NULL, NULL, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, get_room_index(vnum));
   if (x > -1 || y > -1 || map > -1)
   {
      SET_ONMAP_FLAG(ch);
      ch->coord->x = x;
      ch->coord->y = y;
      ch->map = map;
   }
   else
   {
      REMOVE_ONMAP_FLAG(ch);
      ch->coord->x = -1;
      ch->coord->y = -1;
      ch->map = -1;
      x = y = map = -1;
   }
   update_objects(ch, ch->map, ch->coord->x, ch->coord->y);
   if (ch->mount)
   {
      char_from_room(ch->mount);
      char_to_room(ch->mount, ch->in_room);
      ch->mount->coord->x = ch->coord->x;
      ch->mount->coord->y = ch->coord->y;
      ch->mount->map = ch->map;
      if (x == -1)
         REMOVE_ONMAP_FLAG(ch->mount);
      else
         SET_ONMAP_FLAG(ch->mount);
   }  
   if (!IS_NPC(ch) && ch->pcdata->pet)
   {
      char_from_room(ch->pcdata->pet);
      char_to_room(ch->pcdata->pet, ch->in_room);
      ch->pcdata->pet->coord->x = ch->coord->x;
      ch->pcdata->pet->coord->y = ch->coord->y;
      ch->pcdata->pet->map = ch->map;
      if (x == -1)
         REMOVE_ONMAP_FLAG(ch->pcdata->pet);
      else
         SET_ONMAP_FLAG(ch->pcdata->pet);
   }  
   if (!IS_NPC(ch) && ch->pcdata->mount && !ch->mount)
   {
      char_from_room(ch->pcdata->mount);
      char_to_room(ch->pcdata->mount, ch->in_room);
      ch->pcdata->mount->coord->x = ch->coord->x;
      ch->pcdata->mount->coord->y = ch->coord->y;
      ch->pcdata->mount->map = ch->map;
      if (x == -1)
         REMOVE_ONMAP_FLAG(ch->pcdata->mount);
      else
         SET_ONMAP_FLAG(ch->pcdata->mount);
   }  
   if (ch->rider)
   {
      char_from_room(ch->rider);
      char_to_room(ch->rider, ch->in_room);
      ch->rider->coord->x = ch->coord->x;
      ch->rider->coord->y = ch->coord->y;
      ch->rider->map = ch->map;
      if (x == -1)
         REMOVE_ONMAP_FLAG(ch->rider);
      else
         SET_ONMAP_FLAG(ch->rider);
      update_objects(ch->rider, ch->rider->map, ch->rider->coord->x, ch->rider->coord->y);
   } 
   if (ch->riding)
   {
      char_from_room(ch->riding);
      char_to_room(ch->riding, ch->in_room);
      ch->riding->coord->x = ch->coord->x;
      ch->riding->coord->y = ch->coord->y;
      ch->riding->map = ch->map;
      if (x == -1)
         REMOVE_ONMAP_FLAG(ch->riding);
      else
         SET_ONMAP_FLAG(ch->riding);
      update_objects(ch->riding, ch->riding->map, ch->riding->coord->x, ch->riding->coord->y);
   }  
   if (ch->on)
   {
      ch->on = NULL;
      ch->position = POS_STANDING;
   }
   if (ch->position != POS_STANDING && ch->position != POS_RIDING)
   {
      ch->position = POS_STANDING;
   }

   act(AT_MAGIC, "$n appears in a flash of light!", ch, NULL, NULL, TO_ROOM);
   do_look(ch, "auto");
   return rNONE;
}

ch_ret spell_weaken(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   AFFECT_DATA af;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   set_char_color(AT_MAGIC, ch);
   if (is_immune(victim, -1, RIS_MAGIC))
   {
      immune_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (is_affected(victim, sn) || saves_wands(level, victim))
   {
      send_to_char("Your magic fails to take hold.\n\r", ch);
      return rSPELL_FAILED;
   }
   af.type = sn;
   af.duration = 10+level*2;
   af.location = APPLY_STR;
   af.modifier = -1 - (level/30);
   xCLEAR_BITS(af.bitvector);
   affect_to_char(victim, &af);
   set_char_color(AT_MAGIC, victim);
   send_to_char("Your muscles seem to atrophy!\n\r", victim);
   if (ch != victim)
   {
      act(AT_MAGIC, "You induce a mild atrophy in $N's muscles.", ch, NULL, victim, TO_CHAR);
      act(AT_MAGIC, "$n induces a mild atrophy in $N's muscles.", ch, NULL, victim, TO_NOTVICT);
   }
   return rNONE;
}



/*
 * A spell as it should be				-Thoric
 */
ch_ret spell_word_of_recall(int sn, int level, CHAR_DATA * ch, void *vo)
{
   sh_int cvnum;
   sh_int nvnum;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   cvnum = ch->in_room->vnum;
   do_recall(ch, target_name);
   nvnum = ch->in_room->vnum;
   return rNONE;
}


/*
 * NPC spells.
 */
ch_ret spell_acid_breath(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   OBJ_DATA *obj_lose;
   OBJ_DATA *obj_next;
   int dam, points, mastery;
   int hpch;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (chance(ch, 2 * level) && !saves_breath(level, victim) && !IS_AFFECTED(victim, AFF_NYIJI))
   {
      for (obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next)
      {

         obj_next = obj_lose->next_content;

         if (number_range(1, 6) > 1)
            continue;
         if (IS_OBJ_STAT(obj_lose, ITEM_NOBREAK))
            continue;
         
         switch (obj_lose->item_type)
         {
            case ITEM_ARMOR:
               if (obj_lose->value[0] > 0)
               {
                  act(AT_DAMAGE, "$p is pitted and etched!", victim, obj_lose, NULL, TO_CHAR);
                  damage_obj(obj_lose, NULL, 0, number_range(150, 250));
               }
               break;

            case ITEM_CONTAINER:
               separate_obj(obj_lose);
               act(AT_DAMAGE, "$p fumes and dissolves!", victim, obj_lose, NULL, TO_CHAR);
               act(AT_OBJECT, "The contents of $p held by $N spill onto the ground.", victim, obj_lose, victim, TO_ROOM);
               act(AT_OBJECT, "The contents of $p spill out onto the ground!", victim, obj_lose, NULL, TO_CHAR);
               empty_obj(obj_lose, NULL, victim->in_room);
               extract_obj(obj_lose);
               break;
         }
      }
   }

   hpch = UMAX(10, ch->hit);
   dam = number_range(hpch / 16 + 1, hpch / 12);
   if (saves_breath(level, victim))
      dam /= 2;
   return damage(ch, victim, dam, sn, 0, number_range(LM_BODY, LM_NECK));
}



ch_ret spell_fire_breath(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   OBJ_DATA *obj_lose;
   OBJ_DATA *obj_next;
   int dam, points, mastery;
   int hpch;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (chance(ch, 2 * level) && !saves_breath(level, victim) && !IS_AFFECTED(victim, AFF_NYIJI))
   {
      for (obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next)
      {
         char *msg;

         obj_next = obj_lose->next_content;
         if (number_range(1, 6) > 1)
            continue;
         if (IS_OBJ_STAT(obj_lose, ITEM_NOBREAK))
            continue;
         switch (obj_lose->item_type)
         {
            default:
               continue;
            case ITEM_CONTAINER:
               msg = "$p ignites and burns!";
               break;
            case ITEM_POTION:
               msg = "$p bubbles and boils!";
               break;
            case ITEM_SPELLBOOK:
            case ITEM_SCROLL:
               msg = "$p crackles and burns!";
               break;
            case ITEM_COOK:
            case ITEM_FOOD:
               msg = "$p blackens and crisps!";
               break;
         }

         separate_obj(obj_lose);
         act(AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR);
         if (obj_lose->item_type == ITEM_CONTAINER)
         {
            act(AT_OBJECT, "The contents of $p held by $N spill onto the ground.", victim, obj_lose, victim, TO_ROOM);
            act(AT_OBJECT, "The contents of $p spill out onto the ground!", victim, obj_lose, NULL, TO_CHAR);
            empty_obj(obj_lose, NULL, victim->in_room);
         }
         extract_obj(obj_lose);
      }
   }

   hpch = UMAX(10, ch->hit);
   dam = number_range(hpch / 16 + 1, hpch / 12);
   if (saves_breath(level, victim))
      dam /= 2;
   return damage(ch, victim, dam, sn, 0, number_range(LM_BODY, LM_NECK));
}



ch_ret spell_frost_breath(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   OBJ_DATA *obj_lose;
   OBJ_DATA *obj_next;
   int dam, points, mastery;
   int hpch;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if (chance(ch, 2 * level) && !saves_breath(level, victim) && !IS_AFFECTED(victim, AFF_NYIJI))
   {
      for (obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next)
      {
         char *msg;

         obj_next = obj_lose->next_content;
         if (number_range(1, 6) > 1)
            continue;
         if (IS_OBJ_STAT(obj_lose, ITEM_NOBREAK))
            continue;
         switch (obj_lose->item_type)
         {
            default:
               continue;
            case ITEM_CONTAINER:
            case ITEM_DRINK_CON:
            case ITEM_POTION:
               msg = "$p freezes and shatters!";
               break;
         }

         separate_obj(obj_lose);
         act(AT_DAMAGE, msg, victim, obj_lose, NULL, TO_CHAR);
         if (obj_lose->item_type == ITEM_CONTAINER)
         {
            act(AT_OBJECT, "The contents of $p held by $N spill onto the ground.", victim, obj_lose, victim, TO_ROOM);
            act(AT_OBJECT, "The contents of $p spill out onto the ground!", victim, obj_lose, NULL, TO_CHAR);
            empty_obj(obj_lose, NULL, victim->in_room);
         }
         extract_obj(obj_lose);
      }
   }

   hpch = UMAX(10, ch->hit);
   dam = number_range(hpch / 16 + 1, hpch / 12);
   if (saves_breath(level, victim))
      dam /= 2;
   return damage(ch, victim, dam, sn, 0, number_range(LM_BODY, LM_NECK));
}



ch_ret spell_gas_breath(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *vch;
   CHAR_DATA *vch_next;
   int dam, points, mastery;
   int hpch;
   bool ch_died;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   ch_died = FALSE;

   if (is_room_safe(ch))
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("You fail to breathe.\n\r", ch);
      return rNONE;
   }

   for (vch = ch->in_room->first_person; vch; vch = vch_next)
   {
      vch_next = vch->next_in_room;
      if (!IS_NPC(vch) && xIS_SET(vch->act, PLR_WIZINVIS) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL)
         continue;

      if (ch->coord->x != vch->coord->x || ch->coord->y != vch->coord->y
         || ch->map != vch->map)
         continue;

      if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
      {
         hpch = UMAX(10, ch->hit);
         dam = number_range(hpch / 16 + 1, hpch / 14);
         if (saves_breath(level, vch))
            dam /= 2;
         if (damage(ch, vch, dam, sn, 0, -1) == rCHAR_DIED || char_died(ch))
            ch_died = TRUE;
      }
   }
   if (ch_died)
      return rCHAR_DIED;
   else
      return rNONE;
}



ch_ret spell_lightning_breath(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int dam, points, mastery;
   int hpch;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   hpch = UMAX(10, ch->hit);
   dam = number_range(hpch / 16 + 1, hpch / 12);
   if (saves_breath(level, victim))
      dam /= 2;
   return damage(ch, victim, dam, sn, 0, -1);
}

ch_ret spell_null(int sn, int level, CHAR_DATA * ch, void *vo)
{
   send_to_char("That's not a spell!\n\r", ch);
   return rNONE;
}

/* don't remove, may look redundant, but is important */
ch_ret spell_notfound(int sn, int level, CHAR_DATA * ch, void *vo)
{
   send_to_char("That's not a spell!\n\r", ch);
   return rNONE;
}

bool portal_in_room(CHAR_DATA * ch, int x, int y, int map, int roomvnum)
{
   OBJ_DATA *obj;
   OMAP_DATA *mobj;

   if (ch->in_room->vnum == ROOM_VNUM_PORTAL)
   {
      for (obj = first_object; obj; obj = obj->next)
      {
         if (obj->item_type == ITEM_PORTAL)
         {
            if (obj->in_room && obj->in_room->vnum == ROOM_VNUM_PORTAL)
            {
               send_to_char("The room you are in already has a portal.\n\r", ch);
               return TRUE;
            }
         }
      }
   }
   else
   {
      for (mobj = first_wilderobj; mobj; mobj = mobj->next)
      {
         if (mobj->mapobj->item_type == ITEM_PORTAL)
         {
            if (mobj->mapobj->value[0] == ch->coord->x && mobj->mapobj->value[1] == ch->coord->y && mobj->mapobj->value[2] == ch->map)
            {
               send_to_char("The room you are in already has a portal.\n\r", ch);
               return TRUE;
            }
         }
      }
   }
   if (roomvnum == OVERLAND_SOLAN)
   {
      for (mobj = first_wilderobj; mobj; mobj = mobj->next)
      {
         if (mobj->mapobj->item_type == ITEM_PORTAL)
         {
            if (mobj->mapobj->value[0] == x && mobj->mapobj->value[1] == y && mobj->mapobj->value[2] == map)
            {
               send_to_char("The room you want to portal to already has a portal in it.\n\r", ch);
               return TRUE;
            }
         }
      }
   }
   else
   {
      for (obj = first_object; obj; obj = obj->next)
      {
         if (obj->item_type == ITEM_PORTAL)
         {
            if (obj->in_room && obj->in_room->vnum == roomvnum)
            {
               send_to_char("The room you want to portal to already has a portal in it.\n\r", ch);
               return TRUE;
            }
         }
      }
   }
   return FALSE;
}

/*
 * Syntax portal (mob/char)
 * opens a 2-way EX_PORTAL from caster's room to room inhabited by
 *  mob or character won't mess with existing exits
 *
 * do_mp_open_passage, combined with spell_astral
 */
ch_ret spell_portal(int sn, int level, CHAR_DATA * ch, void *vo)
{
   ROOM_INDEX_DATA *targetRoom, *fromRoom;
   int targetRoomVnum;
   OBJ_DATA *portalObj = NULL;
   char buf[MSL];
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int x, y, map;
   sh_int mastery;
   sh_int chance1;
   sh_int p, count = 0;
   sh_int isobj = 0;

   p = 0;
   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance1 = number_range(1, 100);

   if ((35 + level) < chance1)
   {
      send_to_char("You attempt to form a portal, but it decides to close.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {

      if (xIS_SET(ch->act, PLR_PORTALHUNT))
      {
         failed_casting(skill, ch, NULL, NULL);
         return rSPELL_FAILED;
      }

      if (can_use_portal(ch, 0) == FALSE)
         return rSPELL_FAILED;

      if (isdigit(target_name[0]))
      {
         for (p = 0; p < sysdata.last_portal; p++)
         {
            if (xIS_SET(ch->pcdata->portalfnd, p))
            {
               count++;
            }
            if (count == atoi(target_name))
               break;
         }
      }
      if (count == 0)
      {
         if (!str_cmp(target_name, "home"))
         {
            if (ch->in_room->vnum == ROOM_VNUM_PORTAL)
            {
               send_to_char("You cannot portal to the room you are in.\n\r", ch);
               return rSPELL_FAILED;
            }
            else
               count = 10000; //Should be enough, god forbid 10000 portal spots -- Xerves
         }
      }
      if (count == 0)
      {
         send_to_char("You can either portal to somewhere in your list or use 'home' for back to the city.\n\r", ch);
         return rSPELL_FAILED;
      }
      if (count == 10000)
         targetRoomVnum = ROOM_VNUM_PORTAL;
      else
         targetRoomVnum = OVERLAND_SOLAN;

      fromRoom = ch->in_room;
      targetRoom = get_room_index(targetRoomVnum);

      if (count != 10000 && count != 10001)
      {
         x = portal_show[p]->x;
         y = portal_show[p]->y;
         map = portal_show[p]->map;
         if (ch->coord->x == x && ch->coord->y == y && ch->map == map)
         {
            send_to_char("You cannot cast portal to the room you are in.\n\r", ch);
            return rSPELL_FAILED;
         }
      }
      else
      {
         x = -1;
         y = -1;
         map = -1;
         if (ch->in_room->vnum == targetRoomVnum)
         {
            send_to_char("You cannot cast portal to the room you are in.\n\r", ch);
            return rSPELL_FAILED;
         }

      }
      if (portal_in_room(ch, x, y, map, targetRoomVnum) == TRUE)
         return rSPELL_FAILED;

      portalObj = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
      portalObj->timer = 1;
      sprintf(buf, "a portal created by a sorcerer");
      STRFREE(portalObj->short_descr);
      portalObj->short_descr = STRALLOC(buf);
      portalObj->value[0] = x;
      portalObj->value[1] = y;
      portalObj->value[2] = map;
      portalObj->value[3] = targetRoomVnum;

      /* support for new casting messages */
      if (!skill->hit_char || skill->hit_char[0] == '\0')
      {
         set_char_color(AT_MAGIC, ch);
         send_to_char("You utter an incantation, and a portal forms in front of you!\n\r", ch);
      }
      else
         act(AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR);
      if (!skill->hit_room || skill->hit_room[0] == '\0')
         act(AT_MAGIC, "$n utters an incantation, and a portal forms in front of you!", ch, NULL, NULL, TO_ROOM);
      else
         act(AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM);
      portalObj = obj_to_room(portalObj, ch->in_room, ch);


      portalObj = create_object(get_obj_index(OBJ_VNUM_PORTAL), 0);
      portalObj->timer = 2+(level/10);
      STRFREE(portalObj->short_descr);
      portalObj->short_descr = STRALLOC(buf);
      portalObj->value[0] = ch->coord->x;
      portalObj->value[1] = ch->coord->y;
      portalObj->value[2] = ch->map;
      portalObj->value[3] = fromRoom->vnum;
      portalObj = obj_to_room(portalObj, targetRoom, ch);

      if (x > -1 || y > -1 || map > -1)
      {
         SET_OBJ_STAT(portalObj, ITEM_ONMAP);
         portalObj->coord->x = x;
         portalObj->coord->y = y;
         portalObj->map = map;
      }
      else
      {
         REMOVE_OBJ_STAT(portalObj, ITEM_ONMAP);
         portalObj->coord->x = -1;
         portalObj->coord->y = -1;
         portalObj->map = -1;
      }

      return rNONE;
   }
}

ch_ret spell_farsight(int sn, int level, CHAR_DATA * ch, void *vo)
{
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *victim;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   int x, y, map, ox, oy, omap;
   sh_int mastery;
   sh_int chance1;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance1 = number_range(1, 100);

   if ((40 + level) < chance1)
   {
      send_to_char("You attempt to expand your view, but you cannot quite see what you are looking for.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {

      if (mastery != 4)
      {
         /* The spell fails if the victim isn't playing, the victim is the caster,
            the target room has private, solitary, noastral, death or proto flags,
            the caster's room is norecall, the victim is too high in level, the
            victim is a proto mob, the victim makes the saving throw or the pkill
            flag on the caster is not the same as on the victim.  Got it?
          */
         if ((victim = get_char_world(ch, target_name)) == NULL
            || victim == ch
            || !victim->in_room
            || xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
            || wIS_SET(victim, ROOM_PRIVATE)
            || xIS_SET(victim->in_room->room_flags, ROOM_IMP)
            || xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
            || wIS_SET(victim, ROOM_SOLITARY)
            || xIS_SET(victim->in_room->room_flags, ROOM_NO_ASTRAL)
            || wIS_SET(victim, ROOM_NO_ASTRAL)
            || xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
            || (!IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_NOSUMMON))
            || xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL)
            || (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE)) || (IS_NPC(victim) && saves_spell_staff(level*.3, victim)))
         {
            failed_casting(skill, ch, victim, NULL);
            return rSPELL_FAILED;
         }
      }
      else
      {
         /* Less Restricted -- Xerves */
         if ((victim = get_char_world(ch, target_name)) == NULL
            || victim == ch
            || !victim->in_room
            || xIS_SET(victim->in_room->room_flags, ROOM_PRIVATE)
            || wIS_SET(victim, ROOM_PRIVATE)
            || xIS_SET(victim->in_room->room_flags, ROOM_IMP)
            || xIS_SET(victim->in_room->room_flags, ROOM_SOLITARY)
            || wIS_SET(victim, ROOM_SOLITARY)
            || xIS_SET(victim->in_room->room_flags, ROOM_DEATH)
            || (!IS_NPC(victim) && IS_SET(victim->pcdata->flags, PCFLAG_NOSUMMON))
            || (IS_NPC(victim) && xIS_SET(victim->act, ACT_PROTOTYPE)))
         {
            failed_casting(skill, ch, victim, NULL);
            return rSPELL_FAILED;
         }
      }

      location = victim->in_room;
      if (!location)
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }
      x = victim->coord->x;
      y = victim->coord->y;
      map = victim->map;
      ox = ch->coord->x;
      oy = ch->coord->y;
      omap = ch->map;
      successful_casting(skill, ch, victim, NULL);
      original = ch->in_room;
      char_from_room(ch);
      char_to_room(ch, location);
      if (x > -1)
         SET_ONMAP_FLAG(ch);
      else
         REMOVE_ONMAP_FLAG(ch);
      ch->coord->x = x;
      ch->coord->y = y;
      ch->map = map;
      do_look(ch, "auto");
      if (ox > -1)
         SET_ONMAP_FLAG(ch);
      else
         REMOVE_ONMAP_FLAG(ch);
      ch->coord->x = ox;
      ch->coord->y = oy;
      ch->map = omap;
      char_from_room(ch);
      char_to_room(ch, original);
      return rNONE;
   }
}

ch_ret spell_recharge(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj = (OBJ_DATA *) vo;
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      level = level - 1000;
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   if ((sn = obj->value[4]) < 0 || sn >= top_sn || skill_table[sn]->spell_fun == NULL || obj->value[5] == 0)
   {
      send_to_char("You cannot recharge that.\n\r", ch);
      return rSPELL_FAILED;
   }
   separate_obj(obj);
   level = 30 + level/2;
   if (mastery == 1 || mastery == 2)
   {
      if (dice(1, 100) > UMIN(level, 70))
      {
         if (dice(1, 100) > 75)
         {
            act(AT_WHITE, "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR);
            act(AT_WHITE, "$n's attempt at recharging fails, and $p disintegrates.", ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
            return rSPELL_FAILED;
         }
         else
         {
            send_to_char("Nothing happens.\n\r", ch);
            return rSPELL_FAILED;
         }
      }
      else
      {
         act(AT_YELLOW, "$p glows brightly for a few seconds.... ", ch, obj, NULL, TO_CHAR);
         obj->value[0] = UMIN(obj->value[0] + level, obj->pIndexData->value[0]);
      }
   }
   if (mastery == 3)
   {
      if (dice(1, 100) > UMIN(level + 20, 85))
      {
         if (dice(1, 100) > 90)
         {
            act(AT_WHITE, "$p disintegrates into a void.", ch, obj, NULL, TO_CHAR);
            act(AT_WHITE, "$n's attempt at recharging fails, and $p disintegrates.", ch, obj, NULL, TO_ROOM);
            extract_obj(obj);
            return rSPELL_FAILED;
         }
         else
         {
            send_to_char("Nothing happens.\n\r", ch);
            return rSPELL_FAILED;
         }
      }
      else
      {
         act(AT_YELLOW, "$p glows brightly for a few seconds.... ", ch, obj, NULL, TO_CHAR);
         obj->value[0] = UMIN(obj->value[0] + level*2, obj->pIndexData->value[0]);
      }
   }
   if (mastery >= 4)
   {
      if (dice(1, 100) > UMIN(level + 25, 95))
      {
         send_to_char("Nothing happens.\n\r", ch);
         return rSPELL_FAILED;
      }
      else
      {
         act(AT_YELLOW, "$p glows brightly for a few seconds.... ", ch, obj, NULL, TO_CHAR);
         obj->value[0] = UMIN(obj->value[0] + level*3, obj->pIndexData->value[0]);
      }
   }
   return rNONE;
}

/* Scryn 2/2/96 */
ch_ret spell_remove_invis(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int chance1;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   chance1 = number_range(1, 100);

   if (target_name[0] == '\0')
   {
      send_to_char("What should the spell be cast upon?\n\r", ch);
      return rSPELL_FAILED;
   }

   obj = get_obj_carry(ch, target_name);

   if ((55 + level) < chance1)
   {
      send_to_char("Your attempt to cast remove invis but it seems to not be working.\n\r", ch);
      return rSPELL_FAILED;
   }
   else
   {
      if (obj)
      {
         if (!IS_OBJ_STAT(obj, ITEM_INVIS))
         {
            send_to_char("Its not invisible!\n\r", ch);
            return rSPELL_FAILED;
         }

         xREMOVE_BIT(obj->extra_flags, ITEM_INVIS);
         act(AT_MAGIC, "$p becomes visible again.", ch, obj, NULL, TO_CHAR);

         send_to_char("Ok.\n\r", ch);
         return rNONE;
      }
      else
      {
         CHAR_DATA *victim;

         victim = get_char_room_new(ch, target_name, 1);

         if (victim)
         {
            if (!can_see(ch, victim))
            {
               ch_printf(ch, "You don't see %s!\n\r", target_name);
               return rSPELL_FAILED;
            }

            if (!IS_AFFECTED(victim, AFF_INVISIBLE))
            {
               send_to_char("They are not invisible!\n\r", ch);
               return rSPELL_FAILED;
            }

            if (is_safe(ch, victim))
            {
               failed_casting(skill, ch, victim, NULL);
               return rSPELL_FAILED;
            }

            if (is_immune(victim, -1, RIS_MAGIC))
            {
               immune_casting(skill, ch, victim, NULL);
               return rSPELL_FAILED;
            }
            if (!IS_NPC(victim))
            {
               if (chance(ch, 50))
               {
                  failed_casting(skill, ch, victim, NULL);
                  return rSPELL_FAILED;
               }
               else
                  check_illegal_pk(ch, victim);
            }
            else
            {
               if (chance(ch, 50))
               {
                  failed_casting(skill, ch, victim, NULL);
                  return rSPELL_FAILED;
               }
            }

            affect_strip(victim, gsn_invis);
            affect_strip(victim, gsn_mass_invis);
            xREMOVE_BIT(victim->affected_by, AFF_INVISIBLE);
/*	    send_to_char( "Ok.\n\r", ch );*/
            successful_casting(skill, ch, victim, NULL);
            return rNONE;
         }

         ch_printf(ch, "You can't find %s!\n\r", target_name);
         return rSPELL_FAILED;
      }
   }
}

/*
 * Animate Dead: Scryn 3/2/96
 * Modifications by Altrag 16/2/96
 * Modifications by Xerves even more so!
 */
ch_ret spell_animate_dead(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *mob;
   OBJ_DATA *corpse;
   OBJ_DATA *corpse_next;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;
   bool found;
   MOB_INDEX_DATA *pMobIndex;
   AFFECT_DATA af;
   char buf[MSL];
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int points;
   sh_int mastery;
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }
   points = GET_POINTS(ch, sn, isobj, level);
   mastery = GET_MASTERY(ch, sn, isobj, level);

   level = POINT_LEVEL(points, mastery);

   found = FALSE;

   for (corpse = ch->in_room->first_content; corpse; corpse = corpse_next)
   {
      corpse_next = corpse->next_content;

      if (IN_SAME_ROOM(ch, corpse) && corpse->item_type == ITEM_CORPSE_NPC && corpse->cost != -5)
      {
         found = TRUE;
         break;
      }
   }

   if (!found)
   {
      send_to_char("You cannot find a suitable corpse here.\n\r", ch);
      return rSPELL_FAILED;
   }

   if (get_mob_index(MOB_VNUM_ANIMATED_CORPSE) == NULL)
   {
      bug("Vnum 5 not found for spell_animate_dead!", 0);
      return rNONE;
   }


   if ((pMobIndex = get_mob_index((sh_int) abs(corpse->cost))) == NULL)
   {
      bug("Can not find mob for cost of corpse, spell_animate_dead", 0);
      return rSPELL_FAILED;
   }

   if (mastery == 1 || mastery == 2)
   {
      if ((dice(1, 100) > level + 20))
      {
         if (!IS_IMMORTAL(ch))
         {
            failed_casting(skill, ch, NULL, NULL);
            return rSPELL_FAILED;
         }
      }
   }
   else if (mastery == 3)
   {
      if ((dice(1, 100) > level + 30))
      {
         if (!IS_IMMORTAL(ch))
         {
            failed_casting(skill, ch, NULL, NULL);
            return rSPELL_FAILED;
         }
      }
   }
   else if (mastery >= 4)
   {
      if ((dice(1, 100) > level + 35))
      {
         if (!IS_IMMORTAL(ch))
         {
            failed_casting(skill, ch, NULL, NULL);
            return rSPELL_FAILED;
         }
      }
   }
   else
   {
      failed_casting(skill, ch, NULL, NULL);
      bug("%s's animate dead has a bad mastery", ch->name);
      return rSPELL_FAILED;
   }
   mob = create_mobile(get_mob_index(MOB_VNUM_ANIMATED_CORPSE));
   char_to_room(mob, ch->in_room);
   mob->coord->x = ch->coord->x;
   mob->coord->y = ch->coord->y;
   mob->map = ch->map;
   if (IN_WILDERNESS(mob))
      SET_ONMAP_FLAG(mob);
   mob->race = pMobIndex->race; /* should be undead */

   /* Fix so mobs wont have 0 hps and crash mud - Scryn 2/20/96 */
   mob->max_hit = number_range(10+level*1.5, 10+level*3);
   mob->armor = UMIN(14, number_range(points+1, points+3));
   mob->tohitslash = UMIN(13, number_range(points, points+1));
   mob->tohitbash = UMIN(13, number_range(points, points+1));
   mob->tohitstab = UMIN(13, number_range(points, points+1));
   
   mob->barenumdie = UMIN(6, number_range(1+(points*3/10), 1+(points*6/10)));
   mob->baresizedie = UMIN(8, number_range(1+(points*5/10), 1+(points*8/10)));
   mob->damplus = UMIN(11, 1+points/6);
   mob->perm_agi = UMIN(70, 40 + number_range(level*.5, level*.75));

   if (mastery == 3)
   {
      mob->max_hit = mob->max_hit * 1.2;
      mob->hit = mob->max_hit;
      mob->barenumdie++;
   }
   else if (mastery == 4)
   {
      mob->max_hit = mob->max_hit * 1.5;
      mob->hit = mob->max_hit;
      mob->perm_agi+=5;
      mob->barenumdie++;
      mob->damplus+=2;
      mob->armor++;
      mob->tohitslash++;
      mob->tohitbash++;
      mob->tohitstab++;
   }
   else if (mastery == 5)
   {
      mob->max_hit = mob->max_hit * 1.75;
      mob->hit = mob->max_hit;
      mob->perm_agi+=10;
      mob->barenumdie++;
      mob->damplus+=4;
      mob->armor++;
      mob->tohitslash++;
      mob->tohitbash++;
      mob->tohitstab++;
   }
   else if (mastery == 6)
   {
      mob->max_hit = mob->max_hit * 2;
      mob->hit = mob->max_hit;
      mob->perm_agi+=15;
      mob->barenumdie+=2;
      mob->damplus+=6;
      mob->armor+=2;
      mob->tohitslash+=2;
      mob->tohitbash+=2;
      mob->tohitstab+=2;
   }
   else
   {
      mob->hit = mob->max_hit;
   }

   act(AT_MAGIC, "$n makes $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_ROOM);
   act(AT_MAGIC, "You make $T rise from the grave!", ch, NULL, pMobIndex->short_descr, TO_CHAR);

   sprintf(buf, "animated skeleton");
   STRFREE(mob->name);
   mob->name = STRALLOC(buf);

   sprintf(buf, "Animated skeleton");
   STRFREE(mob->short_descr);
   mob->short_descr = STRALLOC(buf);

   sprintf(buf, "An animated skeleton struggles with the horror of its undeath.\n\r");
   STRFREE(mob->long_descr);
   mob->long_descr = STRALLOC(buf);
   add_follower(mob, ch);
   af.type = sn;
   af.duration = level*20+1000;
   af.location = 0;
   af.modifier = 0;
   af.bitvector = meb(AFF_CHARM);
   affect_to_char(mob, &af);

   if (corpse->first_content)
      for (obj = corpse->first_content; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         obj_from_obj(obj);
         obj_to_room(obj, corpse->in_room, ch);
      }

   separate_obj(corpse);
   extract_obj(corpse);
   return rNONE;
}

/* Ignores pickproofs, but can't unlock containers. -- Altrag 17/2/96 */
ch_ret spell_knock(int sn, int level, CHAR_DATA * ch, void *vo)
{
   EXIT_DATA *pexit;
   SKILLTYPE *skill = get_skilltype(sn);
   sh_int isobj = 0;

   if (level >= 999)
   {
      isobj = 1;
   }

   set_char_color(AT_MAGIC, ch);
   /*
    * shouldn't know why it didn't work, and shouldn't work on pickproof
    * exits.  -Thoric
    */
   if (!(pexit = find_door(ch, target_name, FALSE))
      || !IS_SET(pexit->exit_info, EX_CLOSED) || !IS_SET(pexit->exit_info, EX_LOCKED) || IS_SET(pexit->exit_info, EX_PICKPROOF))
   {
      failed_casting(skill, ch, NULL, NULL);
      return rSPELL_FAILED;
   }
   REMOVE_BIT(pexit->exit_info, EX_LOCKED);
   send_to_char("*Click*\n\r", ch);
   if (pexit->rexit && pexit->rexit->to_room == ch->in_room)
      REMOVE_BIT(pexit->rexit->exit_info, EX_LOCKED);
   check_room_for_traps(ch, TRAP_UNLOCK | trap_door[pexit->vdir]);
   return rNONE;
}

 /*******************************************************
	 * Everything after this point is part of SMAUG SPELLS *
	 *******************************************************/

/*
 * saving throw check						-Thoric
 */
bool check_save(int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim)
{
   SKILLTYPE *skill = get_skilltype(sn);
   bool saved = FALSE;

   if (SPELL_FLAG(skill, SF_PKSENSITIVE) && !IS_NPC(ch) && !IS_NPC(victim))
      level /= 2;

   if (skill->saves)
      switch (skill->saves)
      {
         case SS_POISON_DEATH:
            saved = saves_poison_death(level, victim);
            break;
         case SS_ROD_WANDS:
            saved = saves_wands(level, victim);
            break;
         case SS_PARA_PETRI:
            saved = saves_para_petri(level, victim);
            break;
         case SS_BREATH:
            saved = saves_breath(level, victim);
            break;
         case SS_SPELL_STAFF:
            saved = saves_spell_staff(level, victim);
            break;
      }
   return saved;
}

//Damage objects, for magic use only :-)
void magic_obj_damage(OBJ_DATA *obj, int power)
{
   CHAR_DATA *ch = obj->carried_by;
   if (in_arena(ch))
   {
      act(AT_OBJECT, "(Xerves magically prevents $p from being damaged)", ch, obj, NULL, TO_CHAR);
      return;
   }

   if (IS_OBJ_STAT(obj, ITEM_NOBREAK))
   {

      if (obj->item_type != ITEM_LIGHT)
         oprog_damage_trigger(ch, obj);
      else if (!in_arena(ch))
         oprog_damage_trigger(ch, obj);

      return;
   }

   separate_obj(obj);

   if (power == SP_NONE)
      obj->value[3] -= number_range(1, 2);
   if (power == SP_MINOR)
      obj->value[3] -= number_range(2, 4);
   if (power == SP_GREATER)
      obj->value[3] -= number_range(3, 6);
   if (power == SP_MAJOR)
      obj->value[3] -= number_range(5, 12);

   if (obj->value[3] <= 0)
   {
      make_scraps(obj, ch);
   }
   return;
}

/*
 * Generic offensive spell damage attack			-Thoric
 */
ch_ret spell_attack(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   OBJ_DATA *damobj;
   SKILLTYPE *skill = get_skilltype(sn);
   bool saved = check_save(sn, level, ch, victim);
   int dam;
   ch_ret retcode = rNONE;

   if (saved && SPELL_SAVE(skill) == SE_NEGATE)
   {
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }
   if (skill->dice)
      dam = UMAX(0, dice_parse(ch, level, skill->dice, sn));
   else
      dam = dice(1, level / 2);
   if (saved)
   {
      switch (SPELL_SAVE(skill))
      {
         case SE_3QTRDAM:
            dam = (dam * 3) / 4;
            break;
         case SE_HALFDAM:
            dam >>= 1;
            break;
         case SE_QUARTERDAM:
            dam >>= 2;
            break;
         case SE_EIGHTHDAM:
            dam >>= 3;
            break;

         case SE_ABSORB: /* victim absorbs spell for hp's */
            act(AT_MAGIC, "$N absorbs your $t!", ch, skill->noun_damage, victim, TO_CHAR);
            act(AT_MAGIC, "You absorb $N's $t!", victim, skill->noun_damage, ch, TO_CHAR);
            act(AT_MAGIC, "$N absorbs $n's $t!", ch, skill->noun_damage, victim, TO_NOTVICT);
            victim->hit = URANGE(0, victim->hit + dam, victim->max_hit);
            update_pos(victim);
            if (skill->affects)
               retcode = spell_affectchar(sn, level, ch, victim);
            return retcode;

         case SE_REFLECT: /* reflect the spell to the caster */
            return spell_attack(sn, level, victim, ch);
      }
   }
   if (SPELL_FLAG(skill, SF_DEQ))
   {
      switch (SPELL_POWER(skill))
      {
         case SP_NONE:     //damage chest
         {
            damobj = get_eq_char(victim, WEAR_BODY);
            if (damobj)
               if (number_range(1, 100) > (70 + (get_obj_resistance(damobj, victim) * 1.5)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
         }
         
         case SP_MINOR:    //damage chest/arms
         {
            damobj = get_eq_char(victim, WEAR_BODY);
            if (damobj)
               if (number_range(1, 100) > (60 + (get_obj_resistance(damobj, victim) * 2)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_ARM_R);
            if (damobj)
               if (number_range(1, 100) > (60 + (get_obj_resistance(damobj, victim) * 2)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_ARM_L);
            if (damobj)
               if (number_range(1, 100) > (60 + (get_obj_resistance(damobj, victim) * 2)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
         }
         
         case SP_GREATER:  //damage chest/arms/legs
         {
            damobj = get_eq_char(victim, WEAR_BODY);
            if (damobj)
               if (number_range(1, 100) > (40 + (get_obj_resistance(damobj, victim) * 3)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_ARM_R);
            if (damobj)
               if (number_range(1, 100) > (40 + (get_obj_resistance(damobj, victim) * 3)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_ARM_L);
            if (damobj)
               if (number_range(1, 100) > (40 + (get_obj_resistance(damobj, victim) * 3)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_LEG_R);
            if (damobj)
               if (number_range(1, 100) > (40 + (get_obj_resistance(damobj, victim) * 3)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_LEG_L);
            if (damobj)
               if (number_range(1, 100) > (40 + (get_obj_resistance(damobj, victim) * 3)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
         }
         
         case SP_MAJOR:    //damage all
         {
            damobj = get_eq_char(victim, WEAR_BODY);
            if (damobj)
               if (number_range(1, 100) > (20 + (get_obj_resistance(damobj, victim) * 4)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_ARM_R);
            if (damobj)
               if (number_range(1, 100) > (20 + (get_obj_resistance(damobj, victim) * 4)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_ARM_L);
            if (damobj)
               if (number_range(1, 100) > (20 + (get_obj_resistance(damobj, victim) * 4)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_LEG_R);
            if (damobj)
               if (number_range(1, 100) > (20 + (get_obj_resistance(damobj, victim) * 4)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_LEG_L);
            if (damobj)
               if (number_range(1, 100) > (20 + (get_obj_resistance(damobj, victim) * 4)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_HEAD);
            if (damobj)
               if (number_range(1, 100) > (20 + (get_obj_resistance(damobj, victim) * 4)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
            damobj = get_eq_char(victim, WEAR_NECK);
            if (damobj)
               if (number_range(1, 100) > (20 + (get_obj_resistance(damobj, victim) * 4)))
                  magic_obj_damage(damobj, SPELL_POWER(skill));
         }
      }
   }
   retcode = damage(ch, victim, dam, sn, 0, -1);
   if (ch && skill->affects
   && !char_died(ch) && (!is_affected(victim, sn) || SPELL_FLAG(skill, SF_ACCUMULATIVE) || SPELL_FLAG(skill, SF_RECASTABLE)))
   {
      if (!char_died(victim) || !victim)
         retcode = spell_affectchar(sn, level, ch, victim);
      else
         retcode = spell_affectchar(sn, level, ch, NULL);
   }
   return retcode;
}

/*
 * Generic area attack						-Thoric
 */
ch_ret spell_area_attack(int sn, int level, CHAR_DATA * ch, void *vo)
{
   CHAR_DATA *vch, *vch_next;
   SKILLTYPE *skill = get_skilltype(sn);
   bool saved;
   bool affects;
   int dam;
   bool ch_died = FALSE;
   ch_ret retcode = rNONE;

   if (is_room_safe(ch))
   {
      failed_casting(skill, ch, NULL, NULL);
      return rSPELL_FAILED;
   }

   affects = (skill->affects ? TRUE : FALSE);
   if (skill->hit_char && skill->hit_char[0] != '\0')
      act(AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR);
   if (skill->hit_room && skill->hit_room[0] != '\0')
      act(AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM);

   for (vch = ch->in_room->first_person; vch; vch = vch_next)
   {
      vch_next = vch->next_in_room;

      if (!IS_NPC(vch) && xIS_SET(vch->act, PLR_WIZINVIS) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL)
         continue;

      if (vch == ch)
         continue;
         
      if (vch->coord->x != ch->coord->x || vch->coord->y != ch->coord->y || ch->map != vch->map)
         continue;

      if (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch))
      {
         saved = check_save(sn, level, ch, vch);
         
         if (!IS_NPC(ch) && IS_NPC(vch) && IS_ACT_FLAG(vch, ACT_MOUNTSAVE))
            continue;
         if (IS_NPC(vch) && IS_AFFECTED(vch, AFF_CHARM) && vch->master == ch)
            continue;
         if (saved && SPELL_SAVE(skill) == SE_NEGATE)
         {
            failed_casting(skill, ch, vch, NULL);
            continue;
         }
         else if (skill->dice)
            dam = dice_parse(ch, level, skill->dice, sn);
         else
            dam = dice(1, level / 2);
         if (saved)
         {
            switch (SPELL_SAVE(skill))
            {
               case SE_3QTRDAM:
                  dam = (dam * 3) / 4;
                  break;
               case SE_HALFDAM:
                  dam >>= 1;
                  break;
               case SE_QUARTERDAM:
                  dam >>= 2;
                  break;
               case SE_EIGHTHDAM:
                  dam >>= 3;
                  break;

               case SE_ABSORB: /* victim absorbs spell for hp's */
                  act(AT_MAGIC, "$N absorbs your $t!", ch, skill->noun_damage, vch, TO_CHAR);
                  act(AT_MAGIC, "You absorb $N's $t!", vch, skill->noun_damage, ch, TO_CHAR);
                  act(AT_MAGIC, "$N absorbs $n's $t!", ch, skill->noun_damage, vch, TO_NOTVICT);
                  vch->hit = URANGE(0, vch->hit + dam, vch->max_hit);
                  update_pos(vch);
                  continue;

               case SE_REFLECT: /* reflect the spell to the caster */
                  retcode = spell_attack(sn, level, vch, ch);
                  if (char_died(ch))
                  {
                     ch_died = TRUE;
                     break;
                  }
                  continue;
            }
         }
         retcode = damage(ch, vch, dam, sn, 0, -1);
      }
      if (retcode == rNONE && affects && !char_died(ch) && !char_died(vch)
         && (!is_affected(vch, sn) || SPELL_FLAG(skill, SF_ACCUMULATIVE) || SPELL_FLAG(skill, SF_RECASTABLE)))
         retcode = spell_affectchar(sn, level, ch, vch);
      if (retcode == rCHAR_DIED || char_died(ch))
      {
         ch_died = TRUE;
         break;
      }
   }
   return retcode;
}



ch_ret spell_affectchar(int sn, int level, CHAR_DATA * ch, void *vo)
{
   AFFECT_DATA af;
   SMAUG_AFF *saf;
   SKILLTYPE *skill = get_skilltype(sn);
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   int chance;
   ch_ret retcode = rNONE;
   int healmod;

   if (SPELL_FLAG(skill, SF_RECASTABLE) && victim)
      affect_strip(victim, sn);
   for (saf = skill->affects; saf; saf = saf->next)
   {
      if (saf->location >= REVERSE_APPLY)
         victim = ch;
      else
         victim = (CHAR_DATA *) vo;
         
      if (!victim)
         return rNONE;
      /* Check if char has this bitvector already */
      af.bitvector = meb(saf->bitvector);
      
      if (saf->bitvector >= 0 && xIS_SET(victim->affected_by, saf->bitvector) && !SPELL_FLAG(skill, SF_ACCUMULATIVE))
         continue;
      /*
       * necessary for affect_strip to work properly...
       */
      switch (saf->bitvector)
      {
         default:
            af.type = sn;
            break;
         case AFF_POISON:
            af.type = gsn_poison;
            chance = ris_save(victim, level, RIS_POISON);
            if (chance == 1000)
            {
               retcode = rVICT_IMMUNE;
               if (SPELL_FLAG(skill, SF_STOPONFAIL))
                  return retcode;
               continue;
            }
            if (saves_poison_death(chance, victim))
            {
               if (SPELL_FLAG(skill, SF_STOPONFAIL))
                  return retcode;
               continue;
            }
            victim->mental_state = URANGE(30, victim->mental_state + 2, 100);
            break;
         case AFF_BLIND:
            af.type = gsn_blindness;
            break;
         case AFF_CURSE:
            af.type = gsn_curse;
            break;
         case AFF_INVISIBLE:
            af.type = gsn_invis;
            break;
         case AFF_SLEEP:
            af.type = gsn_sleep;
            chance = ris_save(victim, level, RIS_SLEEP);
            if (chance == 1000)
            {
               retcode = rVICT_IMMUNE;
               if (SPELL_FLAG(skill, SF_STOPONFAIL))
                  return retcode;
               continue;
            }
            break;
         case AFF_CHARM:
            af.type = gsn_charm_person;
            chance = ris_save(victim, level, RIS_CHARM);
            if (chance == 1000)
            {
               retcode = rVICT_IMMUNE;
               if (SPELL_FLAG(skill, SF_STOPONFAIL))
                  return retcode;
               continue;
            }
            break;
      }
      af.duration = dice_parse(ch, level, saf->duration, sn);
      af.modifier = dice_parse(ch, level, saf->modifier, sn);
      af.location = saf->location % REVERSE_APPLY;
      if (af.duration == 0)
      {
         switch (af.location)
         {
            case APPLY_HIT:
               if (ch->race == RACE_DWARF && af.modifier > 0)
                  healmod = af.modifier * 150 / 100;
               else
                  healmod = af.modifier;
               if ((xIS_SET(victim->act, ACT_UNDEAD) || xIS_SET(victim->act, ACT_LIVING_DEAD) || (!IS_NPC(victim) && IS_SET(victim->elementb, ELEMENT_UNDEAD)))
               && healmod > 0)
                  healmod = healmod * -2;
               if (healmod < 0)
               {
                  global_retcode = damage(ch, victim, healmod*-1, sn, 0, -1);
               }
               else
               {
                  victim->hit = URANGE(1, victim->hit + healmod, victim->max_hit);
                  if (!IS_NPC(victim) && af.modifier > 0)
                     victim->pcdata->hit_cnt = UMAX(0, victim->pcdata->hit_cnt - healmod);
                  adjust_aggression_list(victim, ch, healmod, 3, -1); //Mobs don't like you healing their targets...
                  update_pos(victim);
               }
               break;
            case APPLY_MANA:
               victim->mana = URANGE(0, victim->mana + af.modifier, victim->max_mana);
               update_pos(victim);
               break;
            case APPLY_MOVE:
               victim->move = URANGE(0, victim->move + af.modifier, victim->max_move);
               update_pos(victim);
               break;
            default:
               affect_modify(victim, &af, TRUE);
               break;
         }
      }
      else if (af.location == APPLY_RECURRINGSPELL)
      {
         if (!IS_NPC(ch) && IS_NPC(victim))
         {
            one_hit(victim, ch, TYPE_HIT, LM_BODY);
         }
         affect_to_char(victim, &af);
      }
      else if (SPELL_FLAG(skill, SF_ACCUMULATIVE))
         affect_join(victim, &af);
      else
         affect_to_char(victim, &af);
   }
   update_pos(victim);
   return retcode;
}

int is_applied(CHAR_DATA *ch, int sn)
{
   AFFECT_DATA *paf;
   SMAUG_AFF *aff;
   SKILLTYPE *skill = get_skilltype(sn);
   
   for (aff = skill->affects; aff; aff = aff->next)
   {
      for (paf = ch->first_affect; paf; paf = paf->next)
      {
         if (aff->location == paf->location)
            return TRUE;
      }
   }
   return FALSE;
}
      

/*
 * Generic spell affect						-Thoric
 */
ch_ret spell_affect(int sn, int level, CHAR_DATA * ch, void *vo)
{
   SMAUG_AFF *saf;
   SKILLTYPE *skill = get_skilltype(sn);
   CHAR_DATA *victim = (CHAR_DATA *) vo;
   bool groupsp;
   bool areasp;
   bool hitchar = FALSE, hitroom = FALSE, hitvict = FALSE;
   ch_ret retcode;

   if (!skill->affects)
   {
      bug("spell_affect has no affects sn %d", sn);
      return rNONE;
   }
   if (SPELL_FLAG(skill, SF_GROUPSPELL))
      groupsp = TRUE;
   else
      groupsp = FALSE;

   if (SPELL_FLAG(skill, SF_AREA))
      areasp = TRUE;
   else
      areasp = FALSE;
     
   if (!groupsp && !areasp)
   {
      /* Can't find a victim */
      if (!victim)
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }

      if (((is_immune(victim, SPELL_DAMAGE(skill), -1)
      || is_immune(victim, -1, RIS_MAGIC)) && (skill->target == TAR_IGNORE || skill->target == TAR_CHAR_OFFENSIVE)))
      {
         immune_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }

      /* Spell is already on this guy */
      if (is_affected(victim, sn) && !SPELL_FLAG(skill, SF_ACCUMULATIVE) && !SPELL_FLAG(skill, SF_RECASTABLE))
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }
      // Used to match an APPLY_ on a spell so you cannot stack a weak/medium/strong spell to get something unholy
      if (is_applied(victim, sn) && SPELL_FLAG(skill, SF_NOAPPLYSTACK))
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }

      if ((saf = skill->affects) && !saf->next && saf->location == APPLY_STRIPSN && !is_affected(victim, dice_parse(ch, level, saf->modifier, sn)))
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }

      if (check_save(sn, level, ch, victim))
      {
         failed_casting(skill, ch, victim, NULL);
         return rSPELL_FAILED;
      }
   }
   else
   {
      if (skill->hit_char && skill->hit_char[0] != '\0')
      {
         if (strstr(skill->hit_char, "$N"))
            hitchar = TRUE;
         else
            act(AT_MAGIC, skill->hit_char, ch, NULL, NULL, TO_CHAR);
      }
      if (skill->hit_room && skill->hit_room[0] != '\0')
      {
         if (strstr(skill->hit_room, "$N"))
            hitroom = TRUE;
         else
            act(AT_MAGIC, skill->hit_room, ch, NULL, NULL, TO_ROOM);
      }
      if (skill->hit_vict && skill->hit_vict[0] != '\0')
         hitvict = TRUE;
      if (victim)
         victim = victim->in_room->first_person;
      else
         victim = ch->in_room->first_person;
   }
   if (!victim)
   {
      bug("spell_affect: could not find victim: sn %d", sn);
      failed_casting(skill, ch, victim, NULL);
      return rSPELL_FAILED;
   }

   for (; victim; victim = victim->next_in_room)
   {
      if (groupsp || areasp)
      {
         if ((groupsp && !is_same_group(victim, ch))
            || is_immune(victim, -1, RIS_MAGIC)
            || is_immune(victim, SPELL_DAMAGE(skill), -1)
            || check_save(sn, level, ch, victim) || (!SPELL_FLAG(skill, SF_RECASTABLE) && is_affected(victim, sn)))
            continue;
            
         if (!IN_SAME_ROOM(victim, ch))
            continue;

         if (hitvict && ch != victim)
         {
            act(AT_MAGIC, skill->hit_vict, ch, NULL, victim, TO_VICT);
            if (hitroom)
            {
               act(AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_NOTVICT);
               act(AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_CHAR);
            }
         }
         else if (hitroom)
            act(AT_MAGIC, skill->hit_room, ch, NULL, victim, TO_ROOM);
         if (ch == victim)
         {
            if (hitvict)
               act(AT_MAGIC, skill->hit_vict, ch, NULL, ch, TO_CHAR);
            else if (hitchar)
               act(AT_MAGIC, skill->hit_char, ch, NULL, ch, TO_CHAR);
         }
         else if (hitchar)
            act(AT_MAGIC, skill->hit_char, ch, NULL, victim, TO_CHAR);
      }
      retcode = spell_affectchar(sn, level, ch, victim);
      if (!groupsp && !areasp)
      {
         if (retcode == rVICT_IMMUNE)
            immune_casting(skill, ch, victim, NULL);
         else
            successful_casting(skill, ch, victim, NULL);
         break;
      }
   }
   return rNONE;
}

/*
 * Generic inventory object spell				-Thoric
 */
ch_ret spell_obj_inv(int sn, int level, CHAR_DATA * ch, void *vo)
{
   OBJ_DATA *obj = (OBJ_DATA *) vo;
   SKILLTYPE *skill = get_skilltype(sn);

   if (!obj)
   {
      failed_casting(skill, ch, NULL, NULL);
      return rNONE;
   }

   switch (SPELL_ACTION(skill))
   {
      default:
      case SA_NONE:
         return rNONE;

      case SA_CREATE:
         if (SPELL_FLAG(skill, SF_WATER)) /* create water */
         {
            int water;

            if (obj->item_type != ITEM_DRINK_CON)
            {
               send_to_char("It is unable to hold water.\n\r", ch);
               return rSPELL_FAILED;
            }

            if (obj->value[2] != LIQ_WATER && obj->value[1] != 0)
            {
               send_to_char("It contains some other liquid.\n\r", ch);
               return rSPELL_FAILED;
            }

            water = UMIN((skill->dice ? dice_parse(ch, level, skill->dice, sn) : level) * 2, obj->value[0] - obj->value[1]);

            if (water > 0)
            {
               separate_obj(obj);
               obj->value[2] = LIQ_WATER;
               obj->value[1] += water;
               if (!is_name("water", obj->name))
               {
                  char buf[MSL];

                  sprintf(buf, "%s water", obj->name);
                  STRFREE(obj->name);
                  obj->name = STRALLOC(buf);
               }
            }
            successful_casting(skill, ch, NULL, obj);
            return rNONE;
         }
         if (SPELL_DAMAGE(skill) == SD_FIRE)
         {
            //return rNONE;
         }
         if (SPELL_CLASS(skill) == SC_DEATH)
         {
            switch (obj->item_type)
            {
               default:
                  failed_casting(skill, ch, NULL, obj);
                  break;
               case ITEM_COOK:
               case ITEM_FOOD:
               case ITEM_DRINK_CON:
                  separate_obj(obj);
                  obj->value[3] = 1;
                  successful_casting(skill, ch, NULL, obj);
                  break;
            }
            return rNONE;
         }
         if (SPELL_CLASS(skill) == SC_LIFE /* purify food/water */
            && (obj->item_type == ITEM_FOOD || obj->item_type == ITEM_DRINK_CON || obj->item_type == ITEM_COOK))
         {
            switch (obj->item_type)
            {
               default:
                  failed_casting(skill, ch, NULL, obj);
                  break;
               case ITEM_COOK:
               case ITEM_FOOD:
               case ITEM_DRINK_CON:
                  separate_obj(obj);
                  obj->value[3] = 0;
                  successful_casting(skill, ch, NULL, obj);
                  break;
            }
            return rNONE;
         }

         if (SPELL_CLASS(skill) != SC_NONE)
         {
            failed_casting(skill, ch, NULL, obj);
            return rNONE;
         }
         switch (SPELL_POWER(skill)) /* clone object */
         {
               OBJ_DATA *clone;

            default:
            case SP_NONE:
               if (number_range(1, 10) >= 2) //10 pecent
               {
                  failed_casting(skill, ch, NULL, obj);
                  return rNONE;
               }
               break;
            case SP_MINOR:
               if (number_range(1, 10) >= 3) //20 percent
               {
                  failed_casting(skill, ch, NULL, obj);
                  return rNONE;
               }
               break;
            case SP_GREATER:
               if (number_range(1, 10) >= 4) //30 percent
               {
                  failed_casting(skill, ch, NULL, obj);
                  return rNONE;
               }
               break;
            case SP_MAJOR:
               if (number_range(1, 100) >= 46) // 45 Percent
               {
                  failed_casting(skill, ch, NULL, obj);
                  return rNONE;
               }
               break;
               clone = clone_object(obj);
               clone->timer = skill->dice ? dice_parse(ch, level, skill->dice, sn) : 0;
               obj_to_char(clone, ch);
               successful_casting(skill, ch, NULL, obj);
         }
         return rNONE;

      case SA_DESTROY:
      case SA_RESIST:
      case SA_SUSCEPT:
      case SA_DIVINATE:
         break;
      case SA_OBSCURE: /* make obj invis */
         if (IS_OBJ_STAT(obj, ITEM_INVIS) || chance(ch, skill->dice ? dice_parse(ch, level, skill->dice, sn) : 20))
         {
            failed_casting(skill, ch, NULL, NULL);
            return rSPELL_FAILED;
         }
         successful_casting(skill, ch, NULL, obj);
         xSET_BIT(obj->extra_flags, ITEM_INVIS);
         return rNONE;

      case SA_CHANGE:
         return rNONE;
   }
   return rNONE;
}

/*
 * Generic object creating spell				-Thoric
 */
ch_ret spell_create_obj(int sn, int level, CHAR_DATA * ch, void *vo)
{
   SKILLTYPE *skill = get_skilltype(sn);
   int lvl;
   int vnum = skill->value;
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *oi;

   lvl = 1;

   /*
    * Add predetermined objects here
    */

   if ((oi = get_obj_index(vnum)) == NULL || (obj = create_object(oi, lvl)) == NULL)
   {
      failed_casting(skill, ch, NULL, NULL);
      return rNONE;
   }
   obj->timer = skill->dice ? dice_parse(ch, level, skill->dice, sn) : 0;
   if (SPELL_ACTION(skill) == SA_CREATE && SPELL_DAMAGE(skill) == SD_FIRE && SPELL_CLASS(skill) == SC_SOLAR) /* continual light */
       obj->value[2] = 30+(level*2);
   successful_casting(skill, ch, NULL, obj);
   if (CAN_WEAR(obj, ITEM_TAKE))
      obj_to_char(obj, ch);
   else
      obj_to_room(obj, ch->in_room, ch);
   return rNONE;
}

/*
 * Generic mob creating spell					-Thoric
 */
ch_ret spell_create_mob(int sn, int level, CHAR_DATA * ch, void *vo)
{
   SKILLTYPE *skill = get_skilltype(sn);
   int lvl;
   int vnum = skill->value;
   CHAR_DATA *mob;
   MOB_INDEX_DATA *mi;
   AFFECT_DATA af;

   /* set maximum mob level */
   lvl = 0;

   /*
    * Add predetermined mobiles here
    */

   if ((mi = get_mob_index(vnum)) == NULL || (mob = create_mobile(mi)) == NULL)
   {
      failed_casting(skill, ch, NULL, NULL);
      return rNONE;
   }
   mob->level = 0;

   mob->max_hit = 20;
   mob->hit = mob->max_hit;
   mob->gold = 0;
   successful_casting(skill, ch, mob, NULL);
   char_to_room(mob, ch->in_room);
   add_follower(mob, ch);
   af.type = sn;
   af.duration = level*10+1000;
   af.location = 0;
   af.modifier = 0;
   af.bitvector = meb(AFF_CHARM);
   affect_to_char(mob, &af);
   return rNONE;
}

ch_ret ranged_attack(CHAR_DATA *, char *, OBJ_DATA *, OBJ_DATA *, sh_int, sh_int);

/*
 * Generic handler for new "SMAUG" spells			-Thoric
 */
ch_ret spell_smaug(int sn, int level, CHAR_DATA * ch, void *vo)
{
   struct skill_type *skill = get_skilltype(sn);
   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return rNONE;
   }
   switch (skill->target)
   {
      case TAR_IGNORE:

         /* offensive area spell */
         if (SPELL_FLAG(skill, SF_AREA)
            && ((SPELL_ACTION(skill) == SA_DESTROY
&& SPELL_CLASS(skill) == SC_LIFE) || (SPELL_ACTION(skill) == SA_CREATE && SPELL_CLASS(skill) == SC_DEATH)))
            return spell_area_attack(sn, level, ch, vo);

         if (SPELL_ACTION(skill) == SA_CREATE)
         {
            if (SPELL_FLAG(skill, SF_OBJECT)) /* create object */
               return spell_create_obj(sn, level, ch, vo);
            if (SPELL_CLASS(skill) == SC_LIFE) /* create mob */
               return spell_create_mob(sn, level, ch, vo);
         }

         /* affect a distant player */
         if (SPELL_FLAG(skill, SF_DISTANT) && SPELL_FLAG(skill, SF_CHARACTER))
            return spell_affect(sn, level, ch, get_char_world(ch, target_name));

         /* affect a player in this room (should have been TAR_CHAR_XXX) */
         if (SPELL_FLAG(skill, SF_CHARACTER))
            return spell_affect(sn, level, ch, get_char_room_new(ch, target_name, 1));

         if (skill->range > 0 && (
               (SPELL_ACTION(skill) == SA_DESTROY
&& SPELL_CLASS(skill) == SC_LIFE) || (SPELL_ACTION(skill) == SA_CREATE && SPELL_CLASS(skill) == SC_DEATH)))
            return ranged_attack(ch, ranged_target_name, NULL, NULL, sn, skill->range);
         /* will fail, or be an area/group affect */
         return spell_affect(sn, level, ch, vo);

      case TAR_CHAR_OFFENSIVE:

         /* a regular damage inflicting spell attack */
         if ((SPELL_ACTION(skill) == SA_DESTROY
               && SPELL_CLASS(skill) == SC_LIFE) || (SPELL_ACTION(skill) == SA_CREATE && SPELL_CLASS(skill) == SC_DEATH))
            return spell_attack(sn, level, ch, vo);

         /* a nasty spell affect */
         return spell_affect(sn, level, ch, vo);

      case TAR_CHAR_DEFENSIVE:
      case TAR_CHAR_SELF:
         if (SPELL_FLAG(skill, SF_NOFIGHT) &&
            (ch->position == POS_FIGHTING
|| ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK))
         {
            send_to_char("You can't concentrate enough for that!\n\r", ch);
            return rNONE;
         }

         if (vo && SPELL_ACTION(skill) == SA_DESTROY)
         {
            CHAR_DATA *victim = (CHAR_DATA *) vo;

            /* cure blindness */
            if (SPELL_CLASS(skill) == SC_ILLUSION)
            {
               if (is_affected(victim, gsn_blindness))
               {
                  affect_strip(victim, gsn_blindness);
                  successful_casting(skill, ch, victim, NULL);
                  return rNONE;
               }
               failed_casting(skill, ch, victim, NULL);
               return rSPELL_FAILED;
            }
         }
         return spell_affect(sn, level, ch, vo);

      case TAR_OBJ_INV: case TAR_OBJ_ROOM:
         return spell_obj_inv(sn, level, ch, vo);
   }
   return rNONE;
}
