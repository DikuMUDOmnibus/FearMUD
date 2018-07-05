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
 *			     Player skills module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"


char *const spell_flag[] = { "water", "earth", "air", "astral", "area", "distant", "reverse",
   "noself", "_unused2_", "accumulative", "recastable", "noscribe",
   "nobrew", "group", "object", "character", "secretskill", "pksensitive",
   "stoponfail", "nofight", "nodispel", "deq", "noapplystack", "r3", "r4", "r5", "r6",
   "r7", "r8", "r9", "r10", "r11"
};

char *const spell_saves[] = { "none", "poison_death", "wands", "para_petri", "breath", "spell_staff" };

char *const spell_save_effect[] = { "none", "negate", "eightdam", "quarterdam", "halfdam", "3qtrdam",
   "reflect", "absorb"
};

char *const spell_damage[] = { "none", "fire", "water", "earth", "energy", "air", "holy", "unholy", "undead" };

char *const spell_action[] = { "none", "create", "destroy", "resist", "suscept", "divinate", "obscure",
   "change"
};

char *const spell_power[] = { "none", "minor", "greater", "major" };

char *const spell_class[] = { "none", "lunar", "solar", "travel", "summon", "life", "death", "illusion" };

char *const target_type[] = { "ignore", "offensive", "defensive", "self", "objinv", "objroom" };


void show_char_to_char(CHAR_DATA * list, CHAR_DATA * ch);
void show_list_to_char(OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowN,  const int iDefaultAction);

int ris_save(CHAR_DATA * ch, int chance, int ris);
bool check_illegal_psteal(CHAR_DATA * ch, CHAR_DATA * victim);

/* from magic.c */
void failed_casting(struct skill_type *skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj);


/*
 * Dummy function
 */
void skill_notfound(CHAR_DATA * ch, char *argument)
{
   send_to_char("Huh?\n\r", ch);
   return;
}


int get_ssave(char *name)
{
   int x;

   for (x = 0; x < sizeof(spell_saves) / sizeof(spell_saves[0]); x++)
      if (!str_cmp(name, spell_saves[x]))
         return x;
   return -1;
}

int get_starget(char *name)
{
   int x;

   for (x = 0; x < sizeof(target_type) / sizeof(target_type[0]); x++)
      if (!str_cmp(name, target_type[x]))
         return x;
   return -1;
}

int get_sflag(char *name)
{
   int x;

   for (x = 0; x < sizeof(spell_flag) / sizeof(spell_flag[0]); x++)
      if (!str_cmp(name, spell_flag[x]))
         return x;
   return -1;
}

int get_sdamage(char *name)
{
   int x;

   for (x = 0; x < sizeof(spell_damage) / sizeof(spell_damage[0]); x++)
      if (!str_cmp(name, spell_damage[x]))
         return x;
   return -1;
}

int get_saction(char *name)
{
   int x;

   for (x = 0; x < sizeof(spell_action) / sizeof(spell_action[0]); x++)
      if (!str_cmp(name, spell_action[x]))
         return x;
   return -1;
}

int get_ssave_effect(char *name)
{
   int x;

   for (x = 0; x < sizeof(spell_save_effect) / sizeof(spell_save_effect[0]); x++)
      if (!str_cmp(name, spell_save_effect[x]))
         return x;
   return -1;
}

int get_spower(char *name)
{
   int x;

   for (x = 0; x < sizeof(spell_power) / sizeof(spell_power[0]); x++)
      if (!str_cmp(name, spell_power[x]))
         return x;
   return -1;
}

int get_sclass(char *name)
{
   int x;

   for (x = 0; x < sizeof(spell_class) / sizeof(spell_class[0]); x++)
      if (!str_cmp(name, spell_class[x]))
         return x;
   return -1;
}

bool is_legal_kill(CHAR_DATA * ch, CHAR_DATA * vch)
{
   if (IS_NPC(ch) || IS_NPC(vch))
      return TRUE;
   if (ch->pcdata->clan && vch->pcdata->clan && ch->pcdata->clan == vch->pcdata->clan)
      return FALSE;
   if (is_safe(ch, vch))
      return FALSE;
   return TRUE;
}


extern char *target_name; /* from magic.c */

int check_twohand_shield(CHAR_DATA *ch)
{
   OBJ_DATA *shield;
   
   if ((shield = get_eq_char(ch, WEAR_SHIELD)) && IS_OBJ_STAT(shield, ITEM_TWOHANDED))
   {
      send_to_char("You cannot do that while wearing a two-handed shield.\n\r", ch);
      return FALSE;
   }
   else
      return TRUE;
}


/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
bool check_skill(CHAR_DATA * ch, char *command, char *argument)
{
   int sn;
   int first = gsn_first_skill;
   int top = gsn_first_weapon - 1;
   int mana, blood;
   struct timeval time_used;
   CHAR_DATA *starget;
   sh_int begmod = 0; // Used to penalize beginners on the to hit on skills

   /* bsearch for the skill */
   for (;;)
   {
      sn = (first + top) >> 1;

      if (LOWER(command[0]) == LOWER(skill_table[sn]->name[0])
         && !str_prefix(command, skill_table[sn]->name)
         && (skill_table[sn]->skill_fun || skill_table[sn]->spell_fun != spell_null) && (can_use_skill(ch, 0, sn)))
         break;
      if (first >= top)
         return FALSE;
      if (strcmp(command, skill_table[sn]->name) < 1)
         top = sn - 1;
      else
         first = sn + 1;
   }

   if (!check_pos(ch, skill_table[sn]->minimum_position))
      return TRUE;

   if (IS_NPC(ch) && (IS_AFFECTED(ch, AFF_CHARM) || IS_AFFECTED(ch, AFF_POSSESS)))
   {
      send_to_char("For some reason, you seem unable to perform that...\n\r", ch);
      act(AT_GREY, "$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM);
      return TRUE;
   }

   /* check if mana is required */
   if (skill_table[sn]->min_mana)
   {
      mana = IS_NPC(ch) ? 0 : skill_table[sn]->min_mana;
      blood = UMAX(1, (mana + 4) / 8); /* NPCs don't have PCDatas. -- Altrag */
      if (IS_VAMPIRE(ch))
      {
         if (ch->pcdata->condition[COND_BLOODTHIRST] < blood)
         {
            send_to_char("You don't have enough blood power.\n\r", ch);
            return TRUE;
         }
      }
      else if (!IS_NPC(ch) && ch->mana < mana)
      {
         send_to_char("You don't have enough mana.\n\r", ch);
         return TRUE;
      }
   }
   else
   {
      mana = 0;
      blood = 0;
   }

   /*
    * Is this a real do-fun, or a really a spell?
    */
   if (!skill_table[sn]->skill_fun)
   {
      ch_ret retcode = rNONE;
      void *vo = NULL;
      CHAR_DATA *victim = NULL;
      OBJ_DATA *obj = NULL;
      int suc;

      target_name = "";

      switch (skill_table[sn]->target)
      {
         default:
            bug("Check_skill: bad target for sn %d.", sn);
            send_to_char("Something went wrong...\n\r", ch);
            return TRUE;

         case TAR_IGNORE:
            vo = NULL;
            starget = NULL;
            if (argument[0] == '\0')
            {
               if ((victim = who_fighting(ch)) != NULL)
                  target_name = victim->name;
            }
            else
               target_name = argument;
            break;

         case TAR_CHAR_OFFENSIVE:
            if (argument[0] == '\0' && (victim = who_fighting(ch)) == NULL)
            {
               ch_printf(ch, "Confusion overcomes you as your '%s' has no target.\n\r", skill_table[sn]->name);
               return TRUE;
            }
            else if (argument[0] != '\0' && (victim = get_char_room_new(ch, argument, 1)) == NULL)
            {
               send_to_char("They aren't here.\n\r", ch);
               return TRUE;
            }
            if (is_safe(ch, victim))
               return TRUE;

            if (ch == victim && SPELL_FLAG(skill_table[sn], SF_NOSELF))
            {
               send_to_char("You can't target yourself!\n\r", ch);
               return TRUE;
            }

            if (!IS_NPC(ch))
            {

               if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim)
               {
                  send_to_char("You can't do that on your own follower.\n\r", ch);
                  return TRUE;
               }
            }

            check_illegal_pk(ch, victim);
            vo = (void *) victim;
            starget = victim;
            break;

         case TAR_CHAR_DEFENSIVE:
            if (argument[0] != '\0' && (victim = get_char_room_new(ch, argument, 1)) == NULL)
            {
               send_to_char("They aren't here.\n\r", ch);
               return TRUE;
            }
            if (!victim)
               victim = ch;

            if (ch == victim && SPELL_FLAG(skill_table[sn], SF_NOSELF))
            {
               send_to_char("You can't target yourself!\n\r", ch);
               return TRUE;
            }

            vo = (void *) victim;
            starget = victim;
            break;

         case TAR_CHAR_SELF:
            vo = (void *) ch;
            starget = ch;
            break;

         case TAR_OBJ_INV:
            if ((obj = get_obj_carry(ch, argument)) == NULL)
            {
               send_to_char("You can't find that.\n\r", ch);
               return TRUE;
            }
            vo = (void *) obj;
            starget = NULL;
            break;
            
         case TAR_OBJ_ROOM:
            if ((obj = get_obj_here(ch, argument)) == NULL)
            {
               send_to_char("You are not in the room with that.\n\r", ch);
               return TRUE;
            }

            vo = (void *) obj;
            starget = NULL;
            break;
      }

      /* waitstate */
      if (!ch->fighting)
         WAIT_STATE(ch, skill_table[sn]->beats*2);
      else
         ch->fight_timer = get_btimer(ch, sn, NULL);
      /* check for failure */
      if (!IS_NPC(ch) && ch->pcdata->ranking[sn] <= 1)
         begmod = 30;
      if (!IS_NPC(ch) && ch->pcdata->ranking[sn] == 2)
         begmod = 15;
      if (!IS_NPC(ch) && ch->pcdata->ranking[sn] == 4)
         begmod = -15;

      suc = 1;
      if (IS_NPC(ch))
      {
          if (number_percent() > 85)
             suc = 0;
      }
      else
      {
          if ((number_percent() + skill_table[sn]->difficulty * 5) > (90 + (ch->pcdata->learned[sn]*10)))
             suc = 0;
      }

      if (!suc)
      {
         failed_casting(skill_table[sn], ch, vo, obj);         
         learn_from_failure(ch, sn, starget);
            
         if (mana)
         {
            if (IS_VAMPIRE(ch))
               gain_condition(ch, COND_BLOODTHIRST, -blood / 2);
            else
            {
               ch->mana -= mana / 2;
               if (skill_table[sn]->type == SKILL_SKILL)
                  gain_mana_per(ch, victim, mana/2);
            }
         }
         return TRUE;
      }
      if (mana)
      {
         if (IS_VAMPIRE(ch))
            gain_condition(ch, COND_BLOODTHIRST, -blood);
         else
         {
            ch->mana -= mana;
            if (skill_table[sn]->type == SKILL_SKILL)
                  gain_mana_per(ch, victim, mana/2);
         }
      }
      start_timer(&time_used);
      retcode = (*skill_table[sn]->spell_fun) (sn, ch->level, ch, vo);
      end_timer(&time_used);
      update_userec(&time_used, &skill_table[sn]->userec);

      if (retcode == rCHAR_DIED || retcode == rERROR)
         return TRUE;

      if (char_died(ch))
         return TRUE;

      if (retcode == rSPELL_FAILED)
      {
         learn_from_failure(ch, sn, starget);
         retcode = rNONE;
      }
      else
      {
         learn_from_success(ch, sn, starget);
      }

      if (skill_table[sn]->target == TAR_CHAR_OFFENSIVE && victim != ch && !char_died(victim))
      {
         CHAR_DATA *vch;
         CHAR_DATA *vch_next;

         for (vch = ch->in_room->first_person; vch; vch = vch_next)
         {
            vch_next = vch->next_in_room;
            if (victim == vch && !victim->fighting && victim->master != ch)
            {
               retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
               break;
            }
         }
      }
      return TRUE;
   }
   
   //for flee, don't remove
   if(!ch->fighting && !IS_NPC(ch))
        ch->fight_timer = 0;
        
   if(ch->fight_timer > 0)
   {
     displayFightTimer(ch);
     return TRUE;
   }
   if (mana)
   {
      if (IS_VAMPIRE(ch))
         gain_condition(ch, COND_BLOODTHIRST, -blood);
      else
      {
         ch->mana -= mana;
         if (skill_table[sn]->type == SKILL_SKILL)
            gain_mana_per(ch, NULL, mana/2);
      }
   }
   ch->prev_cmd = ch->last_cmd; /* haus, for automapping */
   ch->last_cmd = skill_table[sn]->skill_fun;
   start_timer(&time_used);
   (*skill_table[sn]->skill_fun) (ch, argument);
   end_timer(&time_used);
   update_userec(&time_used, &skill_table[sn]->userec);
   
   tail_chain();
   return TRUE;
}

void do_skin(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *korps;
   OBJ_DATA *corpse;
   OBJ_DATA *obj;
   OBJ_DATA *skin;
   bool found;
   char *name;
   char buf[MSL];

   found = FALSE;

   if (argument[0] == '\0')
   {
      send_to_char("Whose corpse do you wish to skin?\n\r", ch);
      return;
   }
   if ((corpse = get_obj_here(ch, argument)) == NULL)
   {
      send_to_char("You cannot find that here.\n\r", ch);
      return;
   }
   if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
   {
      send_to_char("You have no weapon with which to perform this deed.\n\r", ch);
      return;
   }
   if (corpse->item_type != ITEM_CORPSE_PC)
   {
      send_to_char("You can only skin the bodies of player characters.\n\r", ch);
      return;
   }
   if (wielding_skill_weapon(ch, 0) != 7)
   {
      send_to_char("There is nothing you can do with this corpse.\n\r", ch);
      return;
   }
   if (get_obj_index(OBJ_VNUM_SKIN) == NULL)
   {
      bug("Vnum 23 (OBJ_VNUM_SKIN) not found for do_skin!", 0);
      return;
   }
   korps = create_object(get_obj_index(OBJ_VNUM_CORPSE_PC), 0);
   skin = create_object(get_obj_index(OBJ_VNUM_SKIN), 0);
   name = IS_NPC(ch) ? korps->short_descr : corpse->short_descr;
   sprintf(buf, skin->short_descr, name);
   STRFREE(skin->short_descr);
   skin->short_descr = STRALLOC(buf);
   sprintf(buf, skin->description, name);
   STRFREE(skin->description);
   skin->description = STRALLOC(buf);
   act(AT_BLOOD, "$n strips the skin from $p.", ch, corpse, NULL, TO_ROOM);
   act(AT_BLOOD, "You strip the skin from $p.", ch, corpse, NULL, TO_CHAR);
/*  act( AT_MAGIC, "\nThe skinless corpse is dragged through the ground by a strange force...", ch, corpse, NULL, TO_CHAR);
    act( AT_MAGIC, "\nThe skinless corpse is dragged through the ground by a strange force...", ch, corpse, NULL, TO_ROOM);
    extract_obj( corpse ); */
   obj_to_char(skin, ch);
   return;
}

/*
 * Lookup a skills information
 * High god command
 */
void do_slookup(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   int sn;
   int iRace;
   SKILLTYPE *skill = NULL;

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Slookup what?\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "all"))
   {
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
         pager_printf(ch, "Sn: %4d Slot: %4d Skill/spell: '%-20s' Damtype: %s\n\r",
            sn, skill_table[sn]->slot, skill_table[sn]->name, spell_damage[SPELL_DAMAGE(skill_table[sn])]);
   }
   else if (!str_cmp(arg, "mana"))
   {
      pager_printf(ch, " Sn   Name                       Mana  Tier  Sphere  Beats  Group\n\r");
      pager_printf(ch, "---------------------------------------------------------------------------\n\r");
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (atoi(argument) >= 1 && atoi(argument) <= 4)
         {
            if (atoi(argument) == skill_table[sn]->masterydiff[0])
            {
               pager_printf(ch, "%4d  %-25s  %-3d   %d     %d       %-2d     %d\n\r", sn, skill_table[sn]->name,
               skill_table[sn]->min_mana, skill_table[sn]->masterydiff[0], skill_table[sn]->stype, 
               skill_table[sn]->beats, skill_table[sn]->group[0]);
            }
         }
      }
      return;
   }

   else if (!str_cmp(arg, "prototype"))
   {
      pager_printf(ch, " Sn   Name                  Creator\n\r");
      pager_printf(ch, "-------------------------------------------\n\r");
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
         if (skill_table[sn]->prototype > 0)
         {
            pager_printf(ch, "%4d  %-20s  %-15s\n\r", sn, skill_table[sn]->name, skill_table[sn]->made_char);
         }
   }

   else if (!str_cmp(arg, "herbs"))
   {
      for (sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name; sn++)
         pager_printf(ch, "%d) %s\n\r", sn, herb_table[sn]->name);
   }
   else
   {
      SMAUG_AFF *aff;
      int cnt = 0;

      if (arg[0] == 'h' && is_number(arg + 1))
      {
         sn = atoi(arg + 1);
         if (!IS_VALID_HERB(sn))
         {
            send_to_char("Invalid herb.\n\r", ch);
            return;
         }
         skill = herb_table[sn];
      }
      else if (is_number(arg))
      {
         sn = atoi(arg);
         if ((skill = get_skilltype(sn)) == NULL)
         {
            send_to_char("Invalid sn.\n\r", ch);
            return;
         }
         sn %= 1000;
      }
      else if ((sn = skill_lookup(arg)) >= 0)
         skill = skill_table[sn];
      else if ((sn = herb_lookup(arg)) >= 0)
         skill = herb_table[sn];
      else
      {
         send_to_char("No such skill, spell, proficiency or tongue.\n\r", ch);
         return;
      }
      if (!skill)
      {
         send_to_char("Not created yet.\n\r", ch);
         return;
      }

      ch_printf(ch, "Sn: %4d Slot: %4d %s: '%-20s'\n\r", sn, skill->slot, skill_tname[skill->type], skill->name);
      if (skill->prototype)
         ch_printf(ch, "Prototype:  %s  ", (skill->prototype == 0) ? "No" : "Yes");
      if (skill->made_char)
         ch_printf(ch, "Made By:  %s  ", skill->made_char);
      ch_printf(ch, "\n\r");
      if (skill->info)
         ch_printf(ch, "DamType: %s  ActType: %s   ClassType: %s   PowerType: %s\n\r",
            spell_damage[SPELL_DAMAGE(skill)], spell_action[SPELL_ACTION(skill)], spell_class[SPELL_CLASS(skill)], spell_power[SPELL_POWER(skill)]);
      if (skill->flags)
      {
         int x;

         strcpy(buf, "Flags:");
         for (x = 0; x < 32; x++)
            if (SPELL_FLAG(skill, 1 << x))
            {
               strcat(buf, " ");
               strcat(buf, spell_flag[x]);
            }
         strcat(buf, "\n\r");
         send_to_char(buf, ch);
      }
      ch_printf(ch, "Saves: %s  SaveEffect: %s\n\r", spell_saves[(int) skill->saves], spell_save_effect[SPELL_SAVE(skill)]);

      if (skill->difficulty != '\0')
         ch_printf(ch, "Difficulty: %d\n\r", (int) skill->difficulty);

      ch_printf(ch, "Tier %d  TrainerNum %d  Group %d  Sphere %d", skill->masterydiff[0], skill->bookinfo[0], skill->group[0], skill->stype);
      if (skill->targetlimb > 0)
         ch_printf(ch, " Targetlimb %d\n\r", skill->targetlimb);
      else
         send_to_char("\n\r", ch);

      ch_printf(ch, "Type: %s  Target: %s  Minpos: %d  Mana: %d  Beats: %d  Range: %d\n\r",
         skill_tname[skill->type],
         target_type[URANGE(TAR_IGNORE, skill->target, TAR_OBJ_ROOM)], skill->minimum_position, skill->min_mana, skill->beats, skill->range);
      ch_printf(ch, "Flags: %d  Guild: %d  Value: %d  Info: %d  Code: %s\n\r",
         skill->flags, skill->guild, skill->value, skill->info, skill->skill_fun ? skill_name(skill->skill_fun) : spell_name(skill->spell_fun));
      ch_printf(ch, "Dammsg: %s\n\rWearoff: %s\n", skill->noun_damage, skill->msg_off ? skill->msg_off : "(none set)");
      if (skill->dice && skill->dice[0] != '\0')
         ch_printf(ch, "Dice: %s\n\r", skill->dice);
      if (skill->teachers && skill->teachers[0] != '\0')
         ch_printf(ch, "Teachers: %s\n\r", skill->teachers);
      if (skill->components && skill->components[0] != '\0')
         ch_printf(ch, "Components: %s\n\r", skill->components);
      if (skill->participants)
         ch_printf(ch, "Participants: %d\n\r", (int) skill->participants);
      if (skill->userec.num_uses)
         send_timer(&skill->userec, ch);
      for (aff = skill->affects; aff; aff = aff->next)
      {
         if (aff == skill->affects)
            send_to_char("\n\r", ch);
         sprintf(buf, "Affect %d", ++cnt);
         if (aff->location)
         {
            strcat(buf, " modifies ");
            strcat(buf, a_types[aff->location % REVERSE_APPLY]);
            strcat(buf, " by '");
            strcat(buf, aff->modifier);
            if (aff->bitvector != -1)
               strcat(buf, "' and");
            else
               strcat(buf, "'");
         }
         if (aff->bitvector != -1)
         {
            strcat(buf, " applies ");
            strcat(buf, a_flags[aff->bitvector]);
         }
         if (aff->duration[0] != '\0' && aff->duration[0] != '0')
         {
            strcat(buf, " for '");
            strcat(buf, aff->duration);
            strcat(buf, "' rounds");
         }
         if (aff->location >= REVERSE_APPLY)
            strcat(buf, " (affects caster only)");
         strcat(buf, "\n\r");
         send_to_char(buf, ch);
         if (!aff->next)
            send_to_char("\n\r", ch);
      }
      if (skill->hit_char && skill->hit_char[0] != '\0')
         ch_printf(ch, "Hitchar   : %s\n\r", skill->hit_char);
      if (skill->hit_vict && skill->hit_vict[0] != '\0')
         ch_printf(ch, "Hitvict   : %s\n\r", skill->hit_vict);
      if (skill->hit_room && skill->hit_room[0] != '\0')
         ch_printf(ch, "Hitroom   : %s\n\r", skill->hit_room);
      if (skill->hit_dest && skill->hit_dest[0] != '\0')
         ch_printf(ch, "Hitdest   : %s\n\r", skill->hit_dest);
      if (skill->miss_char && skill->miss_char[0] != '\0')
         ch_printf(ch, "Misschar  : %s\n\r", skill->miss_char);
      if (skill->miss_vict && skill->miss_vict[0] != '\0')
         ch_printf(ch, "Missvict  : %s\n\r", skill->miss_vict);
      if (skill->miss_room && skill->miss_room[0] != '\0')
         ch_printf(ch, "Missroom  : %s\n\r", skill->miss_room);
      if (skill->die_char && skill->die_char[0] != '\0')
         ch_printf(ch, "Diechar   : %s\n\r", skill->die_char);
      if (skill->die_vict && skill->die_vict[0] != '\0')
         ch_printf(ch, "Dievict   : %s\n\r", skill->die_vict);
      if (skill->die_room && skill->die_room[0] != '\0')
         ch_printf(ch, "Dieroom   : %s\n\r", skill->die_room);
      if (skill->imm_char && skill->imm_char[0] != '\0')
         ch_printf(ch, "Immchar   : %s\n\r", skill->imm_char);
      if (skill->imm_vict && skill->imm_vict[0] != '\0')
         ch_printf(ch, "Immvict   : %s\n\r", skill->imm_vict);
      if (skill->imm_room && skill->imm_room[0] != '\0')
         ch_printf(ch, "Immroom   : %s\n\r", skill->imm_room);
      if (skill->type != SKILL_HERB)
      {
         if (skill->type == SKILL_RACIAL)
         {
            send_to_char("\n\r--------------------------[RACE USE]--------------------------\n\r", ch);
            for (iRace = 0; iRace < MAX_RACE; iRace++)
            {
               sprintf(buf, "%8.8s) lvl: %3d max: %2d%%", race_table[iRace]->race_name, skill->race_level[iRace], skill->race_adept[iRace]);
               if (!strcmp(race_table[iRace]->race_name, "unused"))
                  sprintf(buf, "                           ");
               if ((iRace > 0) && (iRace % 2 == 1))
                  strcat(buf, "\n\r");
               else
                  strcat(buf, "  ");
               send_to_char(buf, ch);
            }
         }

      }
      send_to_char("\n\r", ch);

   }

   return;
}

/*
 * Set a skill's attributes or what skills a player has.
 * High god command, with support for creating skills/spells/herbs/etc
 */
void do_sset(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL]; /* Used for mastery setting -- Xerves 2/00 */
   CHAR_DATA *victim;
   int value;
   int mvalue = -1;
   int sn, i;
   bool fAll;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Syntax: sset <victim> <skill> <point value/keep> [mastery value]\n\r", ch);
      send_to_char("or:     sset <victim> all     <point value/keep> [mastery value]\n\r", ch);
      if (get_ftrust(ch) >= LEVEL_HI_STAFF) /* Tracker1 */
      {
         send_to_char("or:     sset save skill table\n\r", ch);
         send_to_char("or:     sset save herb table\n\r", ch);
         send_to_char("or:     sset create skill 'new skill'\n\r", ch);
         send_to_char("or:     sset create herb 'new herb'\n\r", ch);
         send_to_char("or:     sset create ability 'new ability'\n\r", ch);
      }
      if (get_ftrust(ch) >= LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("or:     sset <sn>     <field> <value>\n\r", ch);
         send_to_char("\n\rField being one of:\n\r", ch);
         send_to_char("  name code target minpos slot mana beats dammsg wearoff guild minlevel\n\r", ch);
         send_to_char("  type damtype acttype classtype powertype seffect flag dice value difficulty\n\r", ch);
         send_to_char("  affect rmaffect mastery hit miss die imm (char/vict/room) targetlimb\n\r", ch);
         send_to_char("  components teachers racelevel raceadept prototype madeby group tier sphere\n\r", ch);
         send_to_char("Affect having the fields: <location> <modfifier> [duration] [bitvector]\n\r", ch);
         send_to_char("(See AFFECTTYPES for location, and AFFECTED_BY for bitvector)\n\r", ch);
      }
      send_to_char("Skill being any skill or spell.\n\r", ch);
      return;
   }

   if (get_ftrust(ch) > LEVEL_HI_STAFF /* Tracker1 */
      && !str_cmp(arg1, "save") && !str_cmp(argument, "table"))
   {
      if (!str_cmp(arg2, "skill"))
      {
         send_to_char("Saving skill table...\n\r", ch);
         save_skill_table();
         save_classes();
         /* save_races(); */
         return;
      }
      if (!str_cmp(arg2, "herb"))
      {
         send_to_char("Saving herb table...\n\r", ch);
         save_herb_table();
         return;
      }
   }
   if (get_ftrust(ch) > LEVEL_HI_STAFF /* Tracker1 */
      && !str_cmp(arg1, "create") && (!str_cmp(arg2, "skill") || !str_cmp(arg2, "herb") || !str_cmp(arg2, "ability")))
   {
      struct skill_type *skill;
      sh_int type = SKILL_UNKNOWN;

      if (!str_cmp(arg2, "herb"))
      {
         type = SKILL_HERB;
         if (top_herb >= MAX_HERB)
         {
            ch_printf(ch, "The current top herb is %d, which is the maximum.  "
               "To add more herbs,\n\rMAX_HERB will have to be " "raised in mud.h, and the mud recompiled.\n\r", top_sn);
            return;
         }
      }
      else if (top_sn >= MAX_SKILL)
      {
         ch_printf(ch, "The current top sn is %d, which is the maximum.  "
            "To add more skills,\n\rMAX_SKILL will have to be " "raised in mud.h, and the mud recompiled.\n\r", top_sn);
         return;
      }
      CREATE(skill, struct skill_type, 1);
      skill->slot = 0;
      if (type == SKILL_HERB)
      {
         int max, x;

         herb_table[top_herb++] = skill;
         for (max = x = 0; x < top_herb - 1; x++)
            if (herb_table[x] && herb_table[x]->slot > max)
               max = herb_table[x]->slot;
         skill->slot = max + 1;
      }
      else
         skill_table[top_sn++] = skill;
      skill->min_mana = 0;
      skill->name = str_dup(argument);
      skill->noun_damage = str_dup("");
      skill->msg_off = str_dup("");
      skill->spell_fun = spell_smaug;
      skill->type = type;
      skill->prototype = 1;
      skill->made_char = ch->name;
      if (!str_cmp(arg2, "ability"))
         skill->type = SKILL_RACIAL;

      for (i = 0; i < MAX_RACE; i++)
      {
         skill->race_level[i] = LEVEL_IMMORTAL;
         skill->race_adept[i] = 95;
      }

      send_to_char("Done.\n\r", ch);
      return;
   }

   if (arg1[0] == 'h')
      sn = atoi(arg1 + 1);
   else
      sn = atoi(arg1);
   if (get_trust(ch) > LEVEL_STAFF /* Tracker1 */
      && ((arg1[0] == 'h' && is_number(arg1 + 1) && (sn = atoi(arg1 + 1)) >= 0) || (is_number(arg1) && (sn = atoi(arg1)) >= 0)))
   {
      struct skill_type *skill;

      if (arg1[0] == 'h')
      {
         if (sn >= top_herb)
         {
            send_to_char("Herb number out of range.\n\r", ch);
            return;
         }
         skill = herb_table[sn];
      }
      else
      {
         if ((skill = get_skilltype(sn)) == NULL)
         {
            send_to_char("Skill number out of range.\n\r", ch);
            return;
         }
         sn %= 1000;
      }

      if (!str_cmp(arg2, "difficulty"))
      {
         skill->difficulty = atoi(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "participants"))
      {
         skill->participants = atoi(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "damtype"))
      {
         int x = get_sdamage(argument);

         if (x == -1)
            send_to_char("Not a spell damage type.\n\r", ch);
         else
         {
            SET_SDAM(skill, x);
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }
      if (!str_cmp(arg2, "acttype"))
      {
         int x = get_saction(argument);

         if (x == -1)
            send_to_char("Not a spell action type.\n\r", ch);
         else
         {
            SET_SACT(skill, x);
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }
         
      if (!str_cmp(arg2, "classtype"))
      {
         int x = get_sclass(argument);

         if (x == -1)
            send_to_char("Not a spell class type.\n\r", ch);
         else
         {
            SET_SCLA(skill, x);
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }
      if (!str_cmp(arg2, "powertype"))
      {
         int x = get_spower(argument);

         if (x == -1)
            send_to_char("Not a spell power type.\n\r", ch);
         else
         {
            SET_SPOW(skill, x);
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }
      if (!str_cmp(arg2, "seffect"))
      {
         int x = get_ssave_effect(argument);

         if (x == -1)
            send_to_char("Not a spell save effect type.\n\r", ch);
         else
         {
            SET_SSAV(skill, x);
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }
      if (!str_cmp(arg2, "flag"))
      {
         int x = get_sflag(argument);

         if (x == -1)
            send_to_char("Not a spell flag.\n\r", ch);
         else
         {
            TOGGLE_BIT(skill->flags, 1 << x);
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }
      if (!str_cmp(arg2, "saves"))
      {
         int x = get_ssave(argument);

         if (x == -1)
            send_to_char("Not a saving type.\n\r", ch);
         else
         {
            skill->saves = x;
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }

      if (!str_cmp(arg2, "code"))
      {
         SPELL_FUN *spellfun;
         DO_FUN *dofun;

         if ((spellfun = spell_function(argument)) != spell_notfound)
         {
            skill->spell_fun = spellfun;
            skill->skill_fun = NULL;
         }
         else if ((dofun = skill_function(argument)) != skill_notfound)
         {
            skill->skill_fun = dofun;
            skill->spell_fun = NULL;
         }
         else
         {
            send_to_char("Not a spell or skill.\n\r", ch);
            return;
         }
         send_to_char("Ok.\n\r", ch);
         return;
      }

      if (!str_cmp(arg2, "target"))
      {
         int x = get_starget(argument);

         if (x == -1)
            send_to_char("Not a valid target type.\n\r", ch);
         else
         {
            skill->target = x;
            send_to_char("Ok.\n\r", ch);
         }
         return;
      }
      if (!str_cmp(arg2, "sphere"))
      {
         if (atoi(argument) < 1 || atoi(argument) > MAX_SPHERE)
         {
            ch_printf(ch, "Valid range is 1 to %d.\n\r", MAX_SPHERE);
            return;
         }
         else
         {
            skill->stype = atoi(argument);
            send_to_char("Ok.\n\r", ch);
            return;
         }
      }
      if (!str_cmp(arg2, "targetlimb"))
      {
         if (atoi(argument) < 0 || atoi(argument) > 5)
         {
            send_to_char("Range is 0 to 5.\n\r", ch);
            return;
         }
         else
         {
            skill->targetlimb = atoi(argument);
            send_to_char("Ok.\n\r", ch);
            return;
         }
      }
      if (!str_cmp(arg2, "group"))
      {
         if (atoi(argument) < 1 || atoi(argument) > MAX_GROUP)
         {
            ch_printf(ch, "Valid range is 1 to %d.\n\r", MAX_GROUP);
            return;
         }
         else
         {
            skill->group[0] = atoi(argument);
            send_to_char("Ok.\n\r", ch);
            return;
         }
      }
      if (!str_cmp(arg2, "masterydiff") || !str_cmp(arg2, "mastery diff") || !str_cmp(arg2, "tier"))
      {
         if (atoi(argument) < 1 || atoi(argument) > MAX_RANKING)
         {
            ch_printf(ch, "Valid range is 1 to %d.\n\r", MAX_RANKING);
            return;
         }
         else
         {
            skill->masterydiff[0] = atoi(argument);
            send_to_char("Ok.\n\r", ch);
            return;
         }
      }
      if (!str_cmp(arg2, "minpos"))
      {
         skill->minimum_position = URANGE(POS_DEAD, atoi(argument), POS_DRAG);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "minlevel"))
      {
         skill->min_level = URANGE(1, atoi(argument), MAX_LEVEL);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "slot"))
      {
         skill->slot = URANGE(0, atoi(argument), 30000);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "mana"))
      {
         skill->min_mana = URANGE(0, atoi(argument), 2000);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "beats"))
      {
         skill->beats = URANGE(0, atoi(argument), 120);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "range"))
      {
         skill->range = URANGE(0, atoi(argument), 20);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "guild"))
      {
         skill->guild = atoi(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "value"))
      {
         skill->value = atoi(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "type"))
      {
         skill->type = get_skill(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "rmaffect"))
      {
         SMAUG_AFF *aff = skill->affects;
         SMAUG_AFF *aff_next;
         int num = atoi(argument);
         int cnt = 1;

         if (!aff)
         {
            send_to_char("This spell has no special affects to remove.\n\r", ch);
            return;
         }
         if (num == 1)
         {
            skill->affects = aff->next;
            DISPOSE(aff->duration);
            DISPOSE(aff->modifier);
            DISPOSE(aff);
            send_to_char("Removed.\n\r", ch);
            return;
         }
         for (; aff; aff = aff->next)
         {
            if (++cnt == num && (aff_next = aff->next) != NULL)
            {
               aff->next = aff_next->next;
               DISPOSE(aff_next->duration);
               DISPOSE(aff_next->modifier);
               DISPOSE(aff_next);
               send_to_char("Removed.\n\r", ch);
               return;
            }
         }
         send_to_char("Not found.\n\r", ch);
         return;
      }
      /*
       * affect <location> <modifier> <duration> <bitvector>
       */
      if (!str_cmp(arg2, "affect"))
      {
         char location[MIL];
         char modifier[MIL];
         char duration[MIL];

/*	    char bitvector[MIL];	*/
         int loc, bit, tmpbit;
         SMAUG_AFF *aff;

         argument = one_argument(argument, location);
         argument = one_argument(argument, modifier);
         argument = one_argument(argument, duration);

         if (location[0] == '!')
            loc = get_atype(location + 1) + REVERSE_APPLY;
         else
            loc = get_atype(location);
         if ((loc % REVERSE_APPLY) < 0 || (loc % REVERSE_APPLY) >= MAX_APPLY_TYPE)
         {
            send_to_char("Unknown affect location.  See AFFECTTYPES.\n\r", ch);
            return;
         }
         bit = -1;
         if (argument[0] != 0)
         {
            if ((tmpbit = get_aflag(argument)) == -1)
               ch_printf(ch, "Unknown bitvector: %s.  See AFFECTED_BY\n\r", argument);
            else
               bit = tmpbit;
         }
         CREATE(aff, SMAUG_AFF, 1);
         if (!str_cmp(duration, "0"))
            duration[0] = '\0';
         if (!str_cmp(modifier, "0"))
            modifier[0] = '\0';
         aff->duration = str_dup(duration);
         aff->location = loc;
         aff->modifier = str_dup(modifier);
         aff->bitvector = bit;
         aff->next = skill->affects;
         skill->affects = aff;
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "racelevel"))
      {
         char arg3[MIL];
         int race;

         argument = one_argument(argument, arg3);
         race = atoi(arg3);
         if (race >= MAX_RACE || race < 0)
            send_to_char("Not a valid race.\n\r", ch);
         else
            skill->race_level[race] = URANGE(0, atoi(argument), MAX_LEVEL);
         return;
      }
      if (!str_cmp(arg2, "raceadept"))
      {
         char arg3[MIL];
         int race;

         argument = one_argument(argument, arg3);
         race = atoi(arg3);
         if (race >= MAX_RACE || race < 0)
            send_to_char("Not a valid race.\n\r", ch);
         else
            skill->race_adept[race] = URANGE(0, atoi(argument), 100);
         return;
      }


      if (!str_cmp(arg2, "name"))
      {
         DISPOSE(skill->name);
         skill->name = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "prototype"))
      {
         int pro = atoi(argument);

         if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
         {
            send_to_char("Sorry, only staff can change this.\n\r", ch);
            return;
         }
         if (pro > 1 || pro < 0)
         {
            send_to_char("0 for no, 1 for yes\n\r", ch);
            return;
         }
         skill->prototype = atoi(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "madeby"))
      {
         if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
         {
            send_to_char("Sorry, only staff can change this.\n\r", ch);
            return;
         }
         DISPOSE(skill->made_char);
         skill->made_char = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "dammsg"))
      {
         DISPOSE(skill->noun_damage);
         if (!str_cmp(argument, "clear"))
            skill->noun_damage = str_dup("");
         else
            skill->noun_damage = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "wearoff"))
      {
         DISPOSE(skill->msg_off);
         if (str_cmp(argument, "clear"))
            skill->msg_off = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "hitchar"))
      {
         if (skill->hit_char)
            DISPOSE(skill->hit_char);
         if (str_cmp(argument, "clear"))
            skill->hit_char = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "hitvict"))
      {
         if (skill->hit_vict)
            DISPOSE(skill->hit_vict);
         if (str_cmp(argument, "clear"))
            skill->hit_vict = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "hitroom"))
      {
         if (skill->hit_room)
            DISPOSE(skill->hit_room);
         if (str_cmp(argument, "clear"))
            skill->hit_room = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "hitdest"))
      {
         if (skill->hit_dest)
            DISPOSE(skill->hit_dest);
         if (str_cmp(argument, "clear"))
            skill->hit_dest = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "misschar"))
      {
         if (skill->miss_char)
            DISPOSE(skill->miss_char);
         if (str_cmp(argument, "clear"))
            skill->miss_char = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "missvict"))
      {
         if (skill->miss_vict)
            DISPOSE(skill->miss_vict);
         if (str_cmp(argument, "clear"))
            skill->miss_vict = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "missroom"))
      {
         if (skill->miss_room)
            DISPOSE(skill->miss_room);
         if (str_cmp(argument, "clear"))
            skill->miss_room = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "diechar"))
      {
         if (skill->die_char)
            DISPOSE(skill->die_char);
         if (str_cmp(argument, "clear"))
            skill->die_char = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "dievict"))
      {
         if (skill->die_vict)
            DISPOSE(skill->die_vict);
         if (str_cmp(argument, "clear"))
            skill->die_vict = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "dieroom"))
      {
         if (skill->die_room)
            DISPOSE(skill->die_room);
         if (str_cmp(argument, "clear"))
            skill->die_room = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "immchar"))
      {
         if (skill->imm_char)
            DISPOSE(skill->imm_char);
         if (str_cmp(argument, "clear"))
            skill->imm_char = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "immvict"))
      {
         if (skill->imm_vict)
            DISPOSE(skill->imm_vict);
         if (str_cmp(argument, "clear"))
            skill->imm_vict = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "immroom"))
      {
         if (skill->imm_room)
            DISPOSE(skill->imm_room);
         if (str_cmp(argument, "clear"))
            skill->imm_room = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "dice"))
      {
         if (skill->dice)
            DISPOSE(skill->dice);
         if (str_cmp(argument, "clear"))
            skill->dice = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "components"))
      {
         if (skill->components)
            DISPOSE(skill->components);
         if (str_cmp(argument, "clear"))
            skill->components = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "teachers"))
      {
         if (skill->teachers)
            DISPOSE(skill->teachers);
         if (str_cmp(argument, "clear"))
            skill->teachers = str_dup(argument);
         send_to_char("Ok.\n\r", ch);
         return;
      }
      do_sset(ch, "");
      return;
   }
   //sset <victim> <skill> <point value> <mastery value>
   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      if ((sn = skill_lookup(arg1)) >= 0)
      {
         sprintf(arg1, "%d %s %s", sn, arg2, argument);
         do_sset(ch, arg1);
      }
      else
         send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg3);
   fAll = !str_cmp(arg2, "all");
   sn = 0;
   if (!fAll && (sn = skill_lookup(arg2)) < 0)
   {
      send_to_char("No such skill or spell.\n\r", ch);
      return;
   }
   if (!is_number(arg3) && str_cmp(arg3, "keep"))
   {
      send_to_char("Point Value must be numeric.\n\r", ch);
      return;
   }

   value = atoi(arg3);
   if (!str_cmp(arg3, "keep"))
      value = -1;
   if (value < -1 || value > MAX_SKPOINTS)
   {
      ch_printf(ch, "Point Value range is 0 to %d.\n\r", MAX_SKPOINTS);
      return;
   }
   if (argument[0] != '\0')
   {
      if (!is_number(argument))
      {
         send_to_char("Mastery Value must be numeric.\n\r", ch);
         return;
      }

      mvalue = atoi(argument);
      if (mvalue < 0 || mvalue > MAX_RANKING)
      {
         ch_printf(ch, "Mastery Value range is 0 to %d.\n\r", MAX_RANKING);
         return;
      }
   }  
   if (fAll)
   {
      for (sn = 0; sn < top_sn; sn++)
      {
         /* Fix by Narn to prevent ssetting skills the player shouldn't have. */
         if (skill_table[sn]->name)
         {
            if (value >= 0)
               victim->pcdata->learned[sn] = value;
            if (mvalue >= 0)
               victim->pcdata->ranking[sn] = mvalue;
            if ((skill_table[sn]->group[0] < 23 || skill_table[sn]->group[0] > 29)
            && (skill_table[sn]->group[0] < 31 || skill_table[sn]->group[0] > 34))
            {
               if (value >= 0)
                  victim->pcdata->spellpoints[skill_table[sn]->group[0]] = value;
               if (mvalue >= 0)
                  victim->pcdata->spellgroups[skill_table[sn]->group[0]] = mvalue;
            }        
         }
      }
   }
   else
   {
      if ((skill_table[sn]->group[0] < 23 || skill_table[sn]->group[0] > 29)
      && (skill_table[sn]->group[0] < 31 || skill_table[sn]->group[0] > 34))
      {
         if (value >= 0)
            victim->pcdata->spellpoints[skill_table[sn]->group[0]] = value;
         if (mvalue >= 0)
            victim->pcdata->spellgroups[skill_table[sn]->group[0]] = mvalue;
      }
      if (value >= 0)  
         victim->pcdata->learned[sn] = value;
      if (mvalue >= 0)
         victim->pcdata->ranking[sn] = mvalue;
   }
   return;
}
int group_in_sphere(int group)
{
   switch (group)
   {
      case 1:
      case 2:
      case 3:
      case 4:
      case 5:
         return 3;
      
      case 6:
      case 19:
      case 20:
      case 21:
      case 22:
      case 23:
         return 4;
      
      case 7:
      case 8:
      case 9:
      case 10:
      case 18:
         return 5;
         
      case 11:
      case 12:
      case 13:
         return 2;
      
      case 14:
      case 15:
      case 16:
      case 17:
         return 1;
      
   }
   return 0;
}

//Gets values to modify flux on a skill/spell, the actually "curving" of the system to prevent super characters
//Can make some beginnger skills/spells master difficulty to learn if you are going against the grain
int get_skillflux_value(CHAR_DATA *ch, int ssn)
{  
   int sn;
   int i;
   int snvalue = 0;   //Point value for the sn's group/mastery
   int restvalue = 0; //Point value for the other groups/mastery
   int totalvalue = 0; //Point total for all spheres.
   int diff = 0;
   int master = 0;
   int specialist = 0;
   //Check to see if points are setup yet in spherepoints and grouppoints
   if (ch->pcdata->spherepoints[1] == -1 || ch->pcdata->grouppoints[1] == -1)
   {
      for (i = 1; i <= MAX_SPHERE; i++)
         ch->pcdata->spherepoints[i] = 0;
      for (i = 1; i <= MAX_GROUP+5; i++)
         ch->pcdata->grouppoints[i] = 0;
      for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
      {
         if (skill_table[sn]->group[0] > 0)
         {
            if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
            {
               ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP] += ch->pcdata->learned[sn];
               ch->pcdata->spherepoints[skill_table[sn]->stype] += ch->pcdata->learned[sn];
            }
            else
            {
               ch->pcdata->grouppoints[skill_table[sn]->group[0]] += ch->pcdata->learned[sn];
               ch->pcdata->spherepoints[skill_table[sn]->stype] += ch->pcdata->learned[sn];
            }
         }
      }
   }
   //Check to see if player is beyond Beginner
   for (sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++)
   {
      if ((ch->pcdata->ranking[sn] >= 2 || ch->pcdata->learned[sn] >= 6) 
      &&  (skill_table[sn]->type == SKILL_SPELL || skill_table[sn]->type == SKILL_SKILL))
      {
         diff = 1; //Beyond the "grace period" above the curve
         break;
      }
   }
   //Next check groups, if the sn is in the "master" sphere, compare against other groups instead of spheres
   //Has to possess majority of points or more
   for (i = 1; i <= MAX_SPHERE; i++)
   { 
      totalvalue += ch->pcdata->spherepoints[i];
   }
   for (i = 1; i <= MAX_SPHERE; i++)
   {
      if (ch->pcdata->spherepoints[i] > totalvalue/2) // > 50 percent of all points
         break;
   }
   if (ch->pcdata->spherepoints[skill_table[ssn]->stype] == totalvalue)
   {
      specialist = 1;
      master = 1;
   }
   else if (i > MAX_SPHERE) 
      master = 0;      //no master sphere 
   else if (skill_table[ssn]->stype == i)
      master = 1;      //skill is in master sphere
   else if (i <= MAX_SPHERE) 
      master = 0;      //there is a master sphere, but you aren't in it
      
   if (master == 1 && diff == 1) //spell/skill is in the master sphere   
   {
      int group = skill_table[ssn]->group[0];
      
      if (skill_table[ssn]->stype == 4 && skill_table[ssn]->group[0] != 6)
         group = skill_table[ssn]->group[0]+MAX_GROUP;
      for (i = 1; i <= MAX_GROUP+5; i++)
      {
         if (group_in_sphere(i) == skill_table[ssn]->stype)
         {
            if (i == group)
               snvalue += ch->pcdata->grouppoints[i];
            else
               restvalue += ch->pcdata->grouppoints[i];
         }
      }
   }
   else //spell/skill is not in the master sphere
   {
      for (i = 1; i <= MAX_SPHERE; i++)
      {
         if (skill_table[ssn]->stype == i)
            snvalue += ch->pcdata->spherepoints[i];
         else
            restvalue += ch->pcdata->spherepoints[i];
      }
   }
   //now we have all the values talied, lets spit out the percents and modify based on the "curve"
   
   //a slightly less mean curve when you first start out so you can start a few different things at once before
   //getting blasted by the curve
   if (diff == 0)
   {
      if (snvalue > restvalue)
         return 100;
      else
         return 70;
   }   	
   if (restvalue <= 0 || specialist == 1)
      diff = 1000;
   else
      diff = 1000*snvalue/(restvalue+snvalue);
   //1000 diff means 1:1  500 diff means 1:2  10000 diff means 10:1, etc.  Higher the more dominant the group/sphere
   //Lower the less dominant.  Higher dominance equals more gain, pretty straight.
   if (master == 0) //no dominant group, lets make one :-)
   {
      if (diff < 1000)
      {
         return (diff*100/1000);
      }
      else
         return 100;
   }
   if (master == 1) //bonus if you are learning in the master sphere
   {
      if (diff < 1000)
      {
         return 60+(diff*40/1000);
      }
      else
         return 100;
   }
   return 100;
}

//return points earned on percent based on mastery alone, used for learn_from_success/failure
int base_learn_value(CHAR_DATA *ch, int sn)
{
   int change = 0;
   int pos = 0;
   
   if (ch->pcdata->learned[sn] >= 1 && ch->pcdata->learned[sn] <= 4)
   {
      if (ch->pcdata->learned[sn] == 1)
         change = number_range(500, 750);
      if (ch->pcdata->learned[sn] == 2)
         change = number_range(400, 600);
      if (ch->pcdata->learned[sn] == 3)
         change = number_range(350, 550);
      if (ch->pcdata->learned[sn] == 4)
         change = number_range(300, 500);
   }
   else if (ch->pcdata->learned[sn] >= 5 && ch->pcdata->learned[sn] <= 7)
   {
      if (ch->pcdata->learned[sn] == 5)
         change = number_range(200, 300);
      if (ch->pcdata->learned[sn] == 6)
         change = number_range(170, 250);
      if (ch->pcdata->learned[sn] == 7)
         change = number_range(130, 190);
      
      if (ch->pcdata->ranking[sn] == 1 && ch->pcdata->learned[sn] >= 5)
         change /= 3;
   }
   else if (ch->pcdata->learned[sn] >= 8 && ch->pcdata->learned[sn] <= 10)
   {
      if (ch->pcdata->learned[sn] == 8)
         change = number_range(80, 120);
      if (ch->pcdata->learned[sn] == 9)
         change = number_range(65, 90);
      if (ch->pcdata->learned[sn] == 10)
         change = number_range(50, 70);
         
      if (ch->pcdata->ranking[sn] == 1)
         change = 1;
      if (ch->pcdata->ranking[sn] == 2 && ch->pcdata->learned[sn] >= 8)
         change /= 3;
   }
   else if (ch->pcdata->learned[sn] >= 11 && ch->pcdata->learned[sn] <= 13)
   {
      if (ch->pcdata->learned[sn] == 11)
         change = number_range(35, 45);
      if (ch->pcdata->learned[sn] == 12)
         change = number_range(25, 30);
      if (ch->pcdata->learned[sn] == 13)
         change = number_range(20, 25);
         
      if (ch->pcdata->ranking[sn] < 4)
         change = 1;
   }
   else if (ch->pcdata->learned[sn] >= 14 && ch->pcdata->learned[sn] <= 19)
   {
      change = number_range(5, 15);
      
      if (ch->pcdata->learned[sn] >= 15 && ch->pcdata->ranking[sn] < 5)
         change = 0;
      if (ch->pcdata->learned[sn] >= 18 && ch->pcdata->ranking[sn] < 6)
         change = 0;
   }
   else
   {
      if (ch->pcdata->spercent[sn] > 3000)
         change = 0;
      else
         change = number_range(3, 10);
   }
   
   change = change * sysdata.exp_percent / 100;
      
   if (change > 0)
      pos = 1;
      
   if (ch->race == RACE_HUMAN)
      change = change * 150 / 100;
   if (sn > 0 && sn < top_sn && skill_table[sn] && skill_table[sn]->name && skill_table[sn]->type == SKILL_SPELL && ch->race == RACE_OGRE)
      change = change * 20 / 100;
   else
   {
      if (ch->race == RACE_OGRE)
         change = change * 80 / 100;
   }
   if (sn > 0 && sn < top_sn && skill_table[sn] && skill_table[sn]->name && skill_table[sn]->type == SKILL_SPELL && ch->race == RACE_FAIRY)
      change = change * 175 / 100;
      
   //Agent sphere for hobbits
   if (sn > 0 && sn < top_sn && skill_table[sn] && skill_table[sn]->name && skill_table[sn]->stype == 2 && ch->race == RACE_HOBBIT)
      change = change * 175 / 100;   
   if (skill_table[sn]->difficulty >= 0 && skill_table[sn]->difficulty <= 13)
   {
      if (skill_table[sn]->difficulty == 1)
         change = change * 95 / 100;   
      if (skill_table[sn]->difficulty == 2) 
         change *= .9;
      if (skill_table[sn]->difficulty == 3)
         change = change * 80 / 100;   
      if (skill_table[sn]->difficulty == 4)
         change = change * 70 / 100;   
      if (skill_table[sn]->difficulty == 5)
         change = change * 65 / 100;
      if (skill_table[sn]->difficulty == 6)
         change = change * 50 / 100;
      if (skill_table[sn]->difficulty == 7)
         change = change * 33 / 100;
      if (skill_table[sn]->difficulty == 8)
         change = change * 25 / 100;
      if (skill_table[sn]->difficulty == 9)
         change = change * 20 / 100;
      if (skill_table[sn]->difficulty == 10)
         change = change * 15 / 100;
      if (skill_table[sn]->difficulty == 11)
         change = change * 10 / 100;
      if (skill_table[sn]->difficulty == 12)
         change = change * 5 / 100;
      if (skill_table[sn]->difficulty == 13)
         change = change * 1 / 100;   
   }
   else
   {
      bug("Sn %d as a difficulty out of the range >= 0 <= 13", sn);
   } 
   change = change * get_skillflux_value(ch, sn) / 100;  
   if (change < 1 && pos == 1)
      change = 1;

   return change;
}

/* Added these back in, but made the chance to increase minimal -- Xerves 11/1/99 */
// Put back in for new skill system, a more hybrid version of stock/new system -- Xerves 2001
void learn_from_success(CHAR_DATA * ch, int sn, CHAR_DATA *target)
{
   int change;
   SKILLTYPE *skill = get_skilltype(sn);
   int pos = 0;
   int x;
   
   if (IS_NPC(ch))
      return;
      
   if (sn == -1)
      return;
      
   for (x = 0; x < 5; x++)  //no learning list
   {
      if (ch->pcdata->nolearn[x] == sn)
         return;
   }
      
   change = base_learn_value(ch, sn); 
   if (change > 0)
      pos = 1;  
   
   if (target && !IS_NPC(target) && is_affected(target, sn) && !SPELL_FLAG(skill, SF_ACCUMULATIVE) && !SPELL_FLAG(skill, SF_RECASTABLE))
      change /= 3; //Come on quit casting the bloody spell already if they have it already
   
   if (target && IS_NPC(target) && xIS_SET(target->act, ACT_MOUNTSAVE))
   {
      change = 0;
      pos = 0;
   }
   if (change < 1 && pos == 1)
      change = 1;  
   ch->pcdata->spercent[sn] += change;
   if (ch->pcdata->spercent[sn] >= 10000)
   {
      ch_printf(ch, "&R*********************************************************************\n\r");
      ch_printf(ch, "&R*****Your skills in %s has increased 1 Point.*****\n\r", skill_table[sn]->name);
      ch_printf(ch, "&R*********************************************************************\n\r");
      ch->pcdata->learned[sn]++;
      ch->pcdata->spercent[sn] = 200; //So skill doesn't go down in a few minutes
      if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
      {
         ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP]++;
         ch->pcdata->spherepoints[skill_table[sn]->stype]++;
      }
      else
      {
         ch->pcdata->grouppoints[skill_table[sn]->group[0]]++;
         ch->pcdata->spherepoints[skill_table[sn]->stype]++;
      }
   }
   return;
}

void learn_from_failure(CHAR_DATA * ch, int sn, CHAR_DATA *target)
{
   int change;
   SKILLTYPE *skill = get_skilltype(sn);
   int x;
   
   int pos = 0;
   
   if (IS_NPC(ch))
      return;
      
   if (sn == 1)
      return;
      
   for (x = 0; x < 5; x++)  //no learning list
   {
      if (ch->pcdata->nolearn[x] == sn)
         return;
   }
      
   change = base_learn_value(ch, sn); 
   if (change > 0)
      pos = 1;  
      
   change /= 3; //Failure gives you only 1/3 of the points
   
   if (sn == gsn_critical) //slow this down quite a bit on failures...
      change /= 4;
   
   if (target && !IS_NPC(target) && is_affected(target, sn) && !SPELL_FLAG(skill, SF_ACCUMULATIVE) && !SPELL_FLAG(skill, SF_RECASTABLE))
      change /= 3; //Come on quit casting the bloody spell already if they have it already
   
   if (target && IS_NPC(target) && xIS_SET(target->act, ACT_MOUNTSAVE))
   {
      change = 0;
      pos = 0;
   }
   if (change < 1 && pos == 1)
      change = 1;  
      
   ch->pcdata->spercent[sn] += change;
   if (ch->pcdata->spercent[sn] >= 10000)
   {
      ch_printf(ch, "&R*********************************************************************\n\r");
      ch_printf(ch, "&R*****Your skills in %s has increased 1 Point.*****\n\r", skill_table[sn]->name);
      ch_printf(ch, "&R*********************************************************************\n\r");
      ch->pcdata->learned[sn]++;
      ch->pcdata->spercent[sn] = 200; //So skill does not go down in a few minutes
      if (skill_table[sn]->stype == 4 && skill_table[sn]->group[0] != 6)
      {
         ch->pcdata->grouppoints[skill_table[sn]->group[0]+MAX_GROUP]++;
         ch->pcdata->spherepoints[skill_table[sn]->stype]++;
      }
      else
      {
         ch->pcdata->grouppoints[skill_table[sn]->group[0]]++;
         ch->pcdata->spherepoints[skill_table[sn]->stype]++;
      }
   }
   return;
}
void do_gouge(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   AFFECT_DATA af;
   sh_int dam;
   int chance;
   sh_int level;
   sh_int mastery;

   mastery = MASTERED(ch, gsn_gouge);
   level = POINT_LEVEL(LEARNED(ch, gsn_gouge), MASTERED(ch, gsn_gouge));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (!can_use_skill(ch, 0, gsn_gouge))
   {
      send_to_char("You do not yet know of this skill.\n\r", ch);
      return;
   }

   if (ch->mount)
   {
      send_to_char("You can't get close enough while mounted.\n\r", ch);
      return;
   }

   if ((victim = who_fighting(ch)) == NULL)
   {
      send_to_char("You aren't fighting anyone.\n\r", ch);
      return;
   }
   chance = 15+(level/6)+((get_curr_dex(ch) - get_curr_dex(victim))*2);
   chance = ((get_curr_dex(victim) - get_curr_dex(ch)) * 10) + 10;
   if (!IS_NPC(ch) && !IS_NPC(victim))
      chance += sysdata.gouge_plr_vs_plr;
   if (victim->fighting && victim->fighting->who != ch)
      chance += sysdata.gouge_nontank;
   if (number_range(1, 100) <= chance)
   {
      dam = number_range(1, 1+(level/20));
      global_retcode = damage(ch, victim, dam, gsn_gouge, 0, -1);
      if (global_retcode == rNONE)
      {
         if (!IS_AFFECTED(victim, AFF_BLIND))
         {
            af.type = gsn_blindness;
            af.location = APPLY_TOHIT;
            af.modifier = -4;
            af.duration = number_range(8+(level/10), 10+(level/8));
            af.bitvector = meb(AFF_BLIND);
            affect_to_char(victim, &af);
            act(AT_SKILL, "You can't see a thing!", victim, NULL, NULL, TO_CHAR);
         }
         ch->fight_timer = skill_table[gsn_gouge]->beats;
         victim->fight_timer += 2*skill_table[gsn_gouge]->beats;
      }
      else if (global_retcode == rVICT_DIED)
      {
         act(AT_BLOOD, "Your fingers plunge into your victim's brain, causing immediate death!", ch, NULL, NULL, TO_CHAR);
      }
      if (global_retcode != rCHAR_DIED && global_retcode != rVICT_DIED && global_retcode != rBOTH_DIED)
         learn_from_success(ch, gsn_gouge, victim);
   }
   else
   {
      ch->fight_timer = skill_table[gsn_gouge]->beats;
      global_retcode = damage(ch, victim, 0, gsn_gouge, 0, -1);
      learn_from_failure(ch, gsn_gouge, victim);
   }

   return;
}

void do_manashot(CHAR_DATA *ch, char *argument)
{
   int level;
   CHAR_DATA *victim = NULL;
   int dam;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_manashot), MASTERED(ch, gsn_manashot));
   
   if (ch->fighting)
      victim = ch->fighting->who;
   if (argument[0] != '\0')
   {
      if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
      {
         send_to_char("Your target is not in the room with you.\n\r", ch);
         return;
      }
   }
   if (!check_twohand_shield(ch))
      return;
   if (!victim)
   {
      send_to_char("Syntax:  manashot <target>\n\rDo not need to supply a target if fighting.\n\r", ch);
      return;
   }
   if (is_room_safe(ch))
   {
      send_to_char("This room is a safe area, you cannot fight here.\n\r", ch);
      return;
   }
   dam = UMAX(6, level-2);
   dam = number_range(dam*80/100, dam*120/100);
   if (is_immune(victim, -1, RIS_MAGIC))
   {
      act(AT_WHITE, "$n releases a powerful manashot toward $N, but $N is immune to magic.", ch, NULL, victim, TO_NOTVICT);
      act(AT_WHITE, "$n releases a powerful manashot toward you, but you are immune to magic.", ch, NULL, victim, TO_VICT);
      act(AT_WHITE, "You release a powerful manashot toward $N, but $N is immune to magic.", ch, NULL, victim, TO_CHAR);
      damage(ch, victim, 0, gsn_manashot, 0, -1);
      return;
   }
   if (saves_spell_staff(level, victim))
      dam = dam * 3 / 4;
   act(AT_WHITE, "$n channels $s energy and blasts $N with a powerful manashot.", ch, NULL, victim, TO_NOTVICT);
   act(AT_WHITE, "$n channels $s energy and blasts you with a powerful manashot.", ch, NULL, victim, TO_VICT);
   act(AT_WHITE, "You channel your energy and blast $N with a powerful manashot.", ch, NULL, victim, TO_CHAR);
   damage(ch, victim, dam, gsn_manashot, 0, -1);
   learn_from_success(ch, gsn_manashot, victim);
   if (ch->fighting)
      ch->fight_timer = get_btimer(ch, gsn_manashot, NULL);
   else
      WAIT_STATE(ch, skill_table[gsn_manashot]->beats*2);
   return;
}

void do_manaburst(CHAR_DATA *ch, char *argument)
{
   int level;
   CHAR_DATA *vch;
   CHAR_DATA *vch_next;
   int dam;
   int num = 0;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_manaburst), MASTERED(ch, gsn_manaburst));
   if (is_room_safe(ch))
   {
      send_to_char("This room is a safe area, you cannot fight here.\n\r", ch);
      return;
   }
   if (!check_twohand_shield(ch))
      return;
   act(AT_WHITE, "$n starts scream and a large amount of energy pours and hits everything around $m.", ch, NULL, NULL, TO_ROOM);
   act(AT_WHITE, "You start screaming and a large amount of energy pours out and hits everything around you.", ch, NULL, NULL, TO_CHAR);
   for (vch = ch->in_room->first_person; vch; vch = vch_next)
   {
      vch_next = vch->next_in_room;

      if (ch->coord->x != vch->coord->x || ch->coord->y != vch->coord->y
         || ch->map != vch->map)
         continue;

      if (!IS_NPC(vch) && xIS_SET(vch->act, PLR_WIZINVIS) && vch->pcdata->wizinvis >= LEVEL_IMMORTAL)
         continue;
         
      if (IS_NPC(vch) && IS_AFFECTED(vch, AFF_CHARM) && vch->master == ch)
         continue;
         
      if (!IS_NPC(ch) && IS_NPC(vch) && IS_ACT_FLAG(vch, ACT_MOUNTSAVE))
         continue;

      if (vch != ch && (IS_NPC(ch) ? !IS_NPC(vch) : IS_NPC(vch)))
      {
         num++;
         dam = 30+level*15/10;
         dam = number_range(dam*80/100, dam*120/100);
         if (saves_spell_staff(level, vch))
            dam = dam * 3 / 4;
         damage(ch, vch, dam, gsn_manaburst, 0, -1);
      }
      if (char_died(ch))
         return;
   }
   if (ch->fighting)
      ch->fight_timer = get_btimer(ch, gsn_manaburst, NULL);
   else
      WAIT_STATE(ch, skill_table[gsn_manaburst]->beats*2);
   if (num)
      learn_from_success(ch, gsn_manaburst, NULL);
   return;
}

void do_nervestrike(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   AFFECT_DATA af;
   int chance;
   int level;
   bool fail;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_nervestrike), MASTERED(ch, gsn_nervestrike));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }
   if (!IS_NPC(ch) && ch->pcdata->ranking[gsn_nervestrike] <= 0)
   {
      send_to_char("Hard to strike at a nerve if you don't even know where one is.\n\r", ch);
      return;
   }
   if (!ch->fighting || !ch->fighting->who)
   {
      send_to_char("You can only use this during the heat of battle.\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      victim = ch->fighting->who;
   }
   if (is_safe(ch, victim))
   {
      send_to_char("You cannot nervestrike that target.\n\r", ch);
      return;
   }
   check_illegal_pk(ch, victim);
   check_attacker(ch, victim);
   if (!IS_NPC(ch) && HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }
   ch->fight_timer = get_btimer(ch, gsn_nervestrike, NULL);
   fail = FALSE;
      
   chance = 5+level/4;
      
   if (!IS_NPC(ch) && !IS_NPC(victim))
      chance -= sysdata.stun_plr_vs_plr;
   else
      chance -= sysdata.stun_regular;
      
   chance += ((get_curr_dex(victim) + get_curr_str(victim)) - (get_curr_dex(ch) + get_curr_str(ch))) * 3;
   chance += URANGE(-10, (get_curr_int(ch)-14)/2 + (get_curr_wis(ch)-14)/2, 10);
   chance += victim->saving_para_petri;
   if (victim->max_hit > 2000)
      chance /= 2;
   if (victim->max_hit > 5000)
      chance /= 2;
   chance = URANGE(5, chance, 45);
   if (!fail)
   {
      if (number_range(1, 100) > chance)
         fail = TRUE;
   }
   if (!fail)
   {
      learn_from_success(ch, gsn_nervestrike, victim);
      ch->fight_timer = skill_table[gsn_nervestrike]->beats;
      act(AT_SKILL, "$N lunges at your neck and strikes a vital nerve.  You are PARALYZED TEMPORARILY!", victim, NULL, ch, TO_CHAR);
      act(AT_SKILL, "You lunge at $N's neck and strike a vital nerve.  $N is PARALYZED TEMPORARILY!", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n lunges at $N's neck and strike a vital nerve.  $N is PARALYZED TEMPORARILY!", ch, NULL, victim, TO_NOTVICT);
      if (!IS_AFFECTED(victim, AFF_PARALYSIS))
      {
         af.type = gsn_stun;
         af.location = APPLY_ARMOR;
         af.modifier = -3;
         af.duration = 5+(MASTERED(ch, gsn_nervestrike)*3/2);
         if (IS_NPC(ch))
            af.duration = 2+MASTERED(ch, gsn_nervestrike);
         af.bitvector = meb(AFF_PARALYSIS);
         affect_to_char(victim, &af);
         update_pos(victim);
      }
      start_hating(victim, ch);
      start_hunting(victim, ch);
   }
   else
   {
      ch->fight_timer = skill_table[gsn_nervestrike]->beats;
      learn_from_failure(ch, gsn_nervestrike, victim);
      act(AT_SKILL, "$n lunges at your neck and attempts to strike a vital nerve, but $e misses.", ch, NULL, victim, TO_VICT);
      act(AT_SKILL, "You try to strike a vital nerve on $N's neck, but you miss.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n lunges at $N's neck and attempts to strike a vital nerve, but $e fails.", ch, NULL, victim, TO_NOTVICT);
      start_hating(victim, ch);
      start_hunting(victim, ch);
   }
   return;
}

void do_quickcombo(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim = NULL;
   int level;
   int sn[6];
   char arg[MIL];
   int x, y;
   int right;
   char buf[MSL];
   int remain;
   
   for (x = 0; x <= 5; x++)
      sn[x] = 0;
   level = POINT_LEVEL(LEARNED(ch, gsn_quickcombo), MASTERED(ch, gsn_quickcombo));
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  quickcombo <right/left> <strike1> <strike2> [strike3] [strike4] [strike5] [victim]\n\r", ch);
      send_to_char("Note: strike3-5 is available if you have expert/elite/flawless in quickcombo.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "right"))
      right = 1;
   else
      right = 0;
   argument = one_argument(argument, arg);
   if ((sn[1] = skill_lookup(arg)) < 1)
   {
      send_to_char("Strike1 is not a valid strike.\n\r", ch);
      return;
   }
   if (skill_table[sn[1]]->group[0] != 7) //Group with all the strikes
   {
      send_to_char("Strike1 has to be an actual strike (ex: blitz)\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if ((sn[2] = skill_lookup(arg)) < 1)
   {
      send_to_char("Strike2 is not a valid strike.\n\r", ch);
      return;
   }
   if (skill_table[sn[2]]->group[0] != 7) //Group with all the strikes
   {
      send_to_char("Strike2 has to be an actual strike (ex: blitz)\n\r", ch);
      return;
   }
   one_argument(argument, arg);
   if ((sn[3] = skill_lookup(arg)) >= 1)
   {
      argument = one_argument(argument, arg); 
      if (MASTERED(ch, gsn_quickcombo) < 3)
      {
         send_to_char("You can only specify a 3rd strike if you have expert or higher mastery.\n\r", ch);
         return;
      }
      if (skill_table[sn[3]]->group[0] != 7) //Group with all the strikes
      {
         send_to_char("Strike3 has to be an actual strike (ex: blitz)\n\r", ch);
         return;
      }
   }
   one_argument(argument, arg);
   if ((sn[4] = skill_lookup(arg)) >= 1)
   {
      argument = one_argument(argument, arg); 
      if (MASTERED(ch, gsn_quickcombo) < 5)
      {
         send_to_char("You can only specify a 4rd strike if you have elite or higher mastery.\n\r", ch);
         return;
      }
      if (skill_table[sn[4]]->group[0] != 7) //Group with all the strikes
      {
         send_to_char("Strike4 has to be an actual strike (ex: blitz)\n\r", ch);
         return;
      }
   }
   one_argument(argument, arg);
   if ((sn[5] = skill_lookup(arg)) >= 1)
   {
      argument = one_argument(argument, arg); 
      if (MASTERED(ch, gsn_quickcombo) < 5)
      {
         send_to_char("You can only specify a 5th strike if you have flawless or higher mastery.\n\r", ch);
         return;
      }
      if (skill_table[sn[5]]->group[0] != 7) //Group with all the strikes
      {
         send_to_char("Strike5 has to be an actual strike (ex: blitz)\n\r", ch);
         return;
      }
   }
   for (x = 1; x <= 5; x++)
   {
      for (y = 1; y <= 5; y++)
      {
         if (x == y)
            continue;
         if (sn[x] <= 0 || sn[y] <= 0)
            continue;
         if (skill_table[sn[x]]->targetlimb == skill_table[sn[y]]->targetlimb)
            break;
      }
      if (y != 6)
      {    
         send_to_char("Each strike has to target a different limb.\n\r", ch);
         return;
      }
   } 
   if (ch->fighting)
      victim = ch->fighting->who;
   if (argument[0] != '\0')
   {
      if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
      {
         send_to_char("Your target is not in the room with you.\n\r", ch);
         return;
      }
   }
   if (!victim)
   {
      do_quickcombo(ch, "");
      return;
   }
   if (is_room_safe(ch))
   {
      send_to_char("This room is a safe area, you cannot fight here.\n\r", ch);
      return;
   }
   sprintf(buf, "'%s' %s", victim->name, right ? "right" : "left");
   //Legs/Arms need right/left argument
   if (skill_table[sn[1]]->targetlimb == 1 || skill_table[sn[1]]->targetlimb == 2)
      (*skill_table[sn[1]]->skill_fun) (ch, buf);
   else
      (*skill_table[sn[1]]->skill_fun) (ch, victim->name);
      
   if (skill_table[sn[2]]->targetlimb == 1 || skill_table[sn[2]]->targetlimb == 2)
      (*skill_table[sn[2]]->skill_fun) (ch, buf);
   else
      (*skill_table[sn[2]]->skill_fun) (ch, victim->name);
      
   if (sn[3] > 0)
   {
      if (skill_table[sn[3]]->targetlimb == 1 || skill_table[sn[3]]->targetlimb == 2)
         (*skill_table[sn[3]]->skill_fun) (ch, buf);
      else
         (*skill_table[sn[3]]->skill_fun) (ch, victim->name);
   }
   if (sn[4] > 0)
   {
      if (skill_table[sn[4]]->targetlimb == 1 || skill_table[sn[4]]->targetlimb == 2)
         (*skill_table[sn[4]]->skill_fun) (ch, buf);
      else
         (*skill_table[sn[4]]->skill_fun) (ch, victim->name);
   }
   if (sn[5] > 0)
   {
      if (skill_table[sn[5]]->targetlimb == 1 || skill_table[sn[5]]->targetlimb == 2)
         (*skill_table[sn[5]]->skill_fun) (ch, buf);
      else
         (*skill_table[sn[5]]->skill_fun) (ch, victim->name);
   }
   remain = ch->fight_timer * UMAX(40, (100-(level*2/3))) % 100;
   ch->fight_timer = ch->fight_timer * UMAX(40, (100-(level*2/3))) / 100;
   if (number_range(1, 100) <= remain)
      ch->fight_timer += 1;
   if (ch->fight_timer < 1)
      ch->fight_timer = 1;
   learn_from_success(ch, gsn_quickcombo, victim);
   return;
}

void do_kickdirt(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   AFFECT_DATA af;
   int chance;
   sh_int level;
   sh_int mastery;
   int sector;

   mastery = MASTERED(ch, gsn_kickdirt);
   level = POINT_LEVEL(LEARNED(ch, gsn_kickdirt), MASTERED(ch, gsn_kickdirt));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (!can_use_skill(ch, 0, gsn_kickdirt))
   {
      send_to_char("You do not yet know of this skill.\n\r", ch);
      return;
   }

   if (ch->mount)
   {
      send_to_char("What do you want to do, blind your target with your mount?.\n\r", ch);
      return;
   }

   if ((victim = who_fighting(ch)) == NULL)
   {
      send_to_char("You aren't fighting anyone.\n\r", ch);
      return;
   }
   if (IN_WILDERNESS(ch))
   {
      sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   }
   else
   {
      sector = ch->in_room->sector_type;   
   }   
   if (sector != SECT_FIELD && sector != SECT_FOREST && sector != SECT_HILLS && sector != SECT_MOUNTAIN
   &&  sector != SECT_DESERT && sector != SECT_UNDERGROUND && sector != SECT_MINEGOLD && sector != SECT_MINEIRON
   &&  sector != SECT_HCORN && sector != SECT_HGRAIN && sector != SECT_STREE && sector != SECT_NTREE
   &&  sector != SECT_SCORN && sector != SECT_NCORN && sector != SECT_SIRON && sector != SECT_NIRON
   &&  sector != SECT_SGRAIN && sector != SECT_NGRAIN && sector != SECT_JUNGLE && sector != SECT_SHORE
   &&  sector != SECT_SWAMP && sector != SECT_PATH && sector != SECT_PLAINS && sector != SECT_BURNT
   &&  sector != SECT_STONE && sector != SECT_SSTONE && sector != SECT_NSTONE)
   {
      send_to_char("There has to be dirt on the ground to kick.\n\r", ch);
      return;
   }
   chance = 15+(level/4)+((get_curr_dex(ch) - get_curr_dex(victim))*5);
   if (number_range(1, 100) <= chance)
   {
      act(AT_RED, "$n kicks dirt into the eyes of $N blinding $M.", ch, NULL, victim, TO_NOTVICT);
      act(AT_RED, "$n kicks dirt into your eyes blinding you!", ch, NULL, victim, TO_VICT);
      act(AT_RED, "You kick dirt into the eyes of $N blinding $M.", ch, NULL, victim, TO_CHAR);
      if (!IS_AFFECTED(victim, AFF_BLIND))
      {
         af.type = gsn_blindness;
         af.location = APPLY_TOHIT;
         af.modifier = -2;
         af.duration = number_range(4+(level/10), 6+(level/8));
         af.bitvector = meb(AFF_BLIND);
         affect_to_char(victim, &af);
         act(AT_SKILL, "You can't see a thing!", victim, NULL, NULL, TO_CHAR);
      }
      ch->fight_timer = skill_table[gsn_kickdirt]->beats;
      learn_from_success(ch, gsn_kickdirt, victim);
   }
   else
   {
      act(AT_RED, "$n kicks dirt toward the eyes of $N, but $E dodges it.", ch, NULL, victim, TO_NOTVICT);
      act(AT_RED, "$n kicks dirt toward your eyes, but you happen to dodge it.", ch, NULL, victim, TO_VICT);
      act(AT_RED, "You kick dirt toward the eyes of $N, but $E dodges it.", ch, NULL, victim, TO_CHAR);
      ch->fight_timer = skill_table[gsn_kickdirt]->beats;
      learn_from_failure(ch, gsn_kickdirt, victim);
   }
   return;
}

void do_detrap(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *trap;
   TRAP_DATA *ntrap;
   OBJ_DATA *tool;
   int percent;
   sh_int level;
   sh_int mastery;
   
   trap = NULL;
   ntrap = NULL;

   mastery = MASTERED(ch, gsn_detrap);
   level = POINT_LEVEL(LEARNED(ch, gsn_detrap), MASTERED(ch, gsn_detrap));

   switch (ch->substate)
   {
      default:
         if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
         {
            send_to_char("You can't concentrate enough for that.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if (!can_use_skill(ch, 0, gsn_detrap))
         {
            send_to_char("You do not yet know of this skill.\n\r", ch);
            return;
         }
         if (arg[0] == '\0')
         {
            send_to_char("Detrap what?\n\r", ch);
            return;
         }
         if (ms_find_obj(ch))
            return;
         if (ch->mount)
         {
            send_to_char("You can't do that while mounted.\n\r", ch);
            return;
         }
         if ((obj = get_obj_here(ch, arg)) == NULL)
         {
            send_to_char("You can't find that here.\n\r", ch);
            return;
         }
         act(AT_ACTION, "You carefully begin your attempt to remove a trap from $p...", ch, obj, NULL, TO_CHAR);
         act(AT_ACTION, "$n carefully attempts to remove a trap from $p...", ch, obj, NULL, TO_ROOM);
         ch->alloc_ptr = str_dup(obj->name);
         add_timer(ch, TIMER_DO_FUN, 6, do_detrap, 1);
         WAIT_STATE( ch, skill_table[gsn_detrap]->beats*2 );
         return;
      case 1:
         if (!ch->alloc_ptr)
         {
            send_to_char("Your detrapping was interrupted!\n\r", ch);
            bug("do_detrap: ch->alloc_ptr NULL!", 0);
            return;
         }
         strcpy(arg, ch->alloc_ptr);
         DISPOSE(ch->alloc_ptr);
         ch->alloc_ptr = NULL;
         ch->substate = SUB_NONE;
         break;
      case SUB_TIMER_DO_ABORT:
         DISPOSE(ch->alloc_ptr);
         ch->substate = SUB_NONE;
         send_to_char("You carefully stop what you were doing.\n\r", ch);
         return;
   }

   if ((obj = get_obj_here(ch, arg)) == NULL)
   {
      send_to_char("You can't find that here.\n\r", ch);
      return;
   }
   trap = get_trap(obj);
   if (obj->trap)
   {
      ntrap = obj->trap;
      trap = NULL;
   }
   if (!trap && !ntrap)
   {
      send_to_char("You find no trap on that.\n\r", ch);
      return;
   }

   percent = UMIN(100, 30 + level + URANGE(-15, (14-get_curr_lck(ch))*3, 20));
   if (ntrap)
      percent -= ntrap->difficulty;
   
   if (ntrap && ntrap->toolkit > 0)
   {
      for (tool = ch->first_carrying; tool; tool = tool->next_content) 
      {
         if (tool->item_type == ITEM_TRAPTOOL && tool->value[0] == ntrap->toolkit)
            percent += ntrap->toolnegate;
      }
   }

   separate_obj(obj);
   if (number_range(1, 100) > percent)
   {
      send_to_char("Ooops!\n\r", ch);
      pre_spring_trap(ch, trap, ntrap, obj);
      learn_from_failure(ch, gsn_detrap, NULL);
      return;
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

   send_to_char("You successfully remove a trap.\n\r", ch);
   learn_from_success(ch, gsn_detrap, NULL);
   return;
}

void do_dig(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *startobj;
   bool found, shovel;
   EXIT_DATA *pexit;
   sh_int mastery;

   mastery = 10;

   if (IS_ONMAP_FLAG(ch))
   {
      send_to_char("You cannot dig on an overland map.\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
         {
            send_to_char("You can't concentrate enough for that.\n\r", ch);
            return;
         }
         if (ch->mount)
         {
            send_to_char("You can't do that while mounted.\n\r", ch);
            return;
         }
         one_argument(argument, arg);
         if (arg[0] != '\0')
         {
            if ((pexit = find_door(ch, arg, TRUE)) == NULL && get_dir(arg) == -1)
            {
               send_to_char("What direction is that?\n\r", ch);
               return;
            }
            if (pexit)
            {
               if (!IS_SET(pexit->exit_info, EX_DIG) && !IS_SET(pexit->exit_info, EX_CLOSED))
               {
                  send_to_char("There is no need to dig out that exit.\n\r", ch);
                  return;
               }
            }
         }
         else
         {
            switch (ch->in_room->sector_type)
            {
               case SECT_CITY:
               case SECT_INSIDE:
                  send_to_char("The floor is too hard to dig through.\n\r", ch);
                  return;
               case SECT_WATER_SWIM:
               case SECT_WATER_NOSWIM:
               case SECT_UNDERWATER:
                  send_to_char("You cannot dig here.\n\r", ch);
                  return;
               case SECT_AIR:
                  send_to_char("What?  In the air?!\n\r", ch);
                  return;
            }
         }
         add_timer(ch, TIMER_DO_FUN, 3, do_dig, 1);
         ch->alloc_ptr = str_dup(arg);
         send_to_char("You begin digging...\n\r", ch);
         act(AT_PLAIN, "$n begins digging...", ch, NULL, NULL, TO_ROOM);
         return;

      case 1:
         if (!ch->alloc_ptr)
         {
            send_to_char("Your digging was interrupted!\n\r", ch);
            act(AT_PLAIN, "$n's digging was interrupted!", ch, NULL, NULL, TO_ROOM);
            bug("do_dig: alloc_ptr NULL", 0);
            return;
         }
         strcpy(arg, ch->alloc_ptr);
         DISPOSE(ch->alloc_ptr);
         break;

      case SUB_TIMER_DO_ABORT:
         DISPOSE(ch->alloc_ptr);
         ch->substate = SUB_NONE;
         send_to_char("You stop digging...\n\r", ch);
         act(AT_PLAIN, "$n stops digging...", ch, NULL, NULL, TO_ROOM);
         return;
   }

   ch->substate = SUB_NONE;

   /* not having a shovel makes it harder to succeed */
   shovel = FALSE;
   for (obj = ch->first_carrying; obj; obj = obj->next_content)
      if (obj->item_type == ITEM_SHOVEL)
      {
         shovel = TRUE;
         break;
      }

   /* dig out an EX_DIG exit... */
   if (arg[0] != '\0')
   {
      if ((pexit = find_door(ch, arg, TRUE)) != NULL && IS_SET(pexit->exit_info, EX_DIG) && IS_SET(pexit->exit_info, EX_CLOSED))
      {
         if (number_range(1, 100) <= mastery * (shovel ? 8 : 1))
         {
            REMOVE_BIT(pexit->exit_info, EX_CLOSED);
            send_to_char("You dig open a passageway!\n\r", ch);
            act(AT_PLAIN, "$n digs open a passageway!", ch, NULL, NULL, TO_ROOM);
            return;
         }
      }
      send_to_char("Your dig did not discover any exit...\n\r", ch);
      act(AT_PLAIN, "$n's dig did not discover any exit...", ch, NULL, NULL, TO_ROOM);
      return;
   }

   startobj = ch->in_room->first_content;
   found = FALSE;

   for (obj = startobj; obj; obj = obj->next_content)
   {
      /* twice as hard to find something without a shovel */
      if (IS_OBJ_STAT(obj, ITEM_BURIED) && (number_range(1, 100) <= mastery * (shovel ? 8 : 1)))
      {
         found = TRUE;
         break;
      }
   }

   if (!found)
   {
      send_to_char("Your dig uncovered nothing.\n\r", ch);
      act(AT_PLAIN, "$n's dig uncovered nothing.", ch, NULL, NULL, TO_ROOM);
      return;
   }

   separate_obj(obj);
   xREMOVE_BIT(obj->extra_flags, ITEM_BURIED);
   act(AT_SKILL, "Your dig uncovered $p!", ch, obj, NULL, TO_CHAR);
   act(AT_SKILL, "$n's dig uncovered $p!", ch, obj, NULL, TO_ROOM);
   if (obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC)
      adjust_favor(ch, 14, 1);

   return;
}


void do_search(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *container;
   OBJ_DATA *startobj;
   int percent, door;
   sh_int mastery;

   mastery = 30;

   door = -1;
   switch (ch->substate)
   {
      default:
         if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
         {
            send_to_char("You can't concentrate enough for that.\n\r", ch);
            return;
         }
         if (ch->mount)
         {
            send_to_char("You can't do that while mounted.\n\r", ch);
            return;
         }
         argument = one_argument(argument, arg);
         if (arg[0] != '\0' && (door = get_door(arg)) == -1)
         {
            container = get_obj_here(ch, arg);
            if (!container)
            {
               send_to_char("You can't find that here.\n\r", ch);
               return;
            }
            if (container->item_type != ITEM_CONTAINER)
            {
               send_to_char("You can't search in that!\n\r", ch);
               return;
            }
            if (IS_SET(container->value[1], CONT_CLOSED))
            {
               send_to_char("It is closed.\n\r", ch);
               return;
            }
         }
         add_timer(ch, TIMER_DO_FUN, 3, do_search, 1);
         send_to_char("You begin your search...\n\r", ch);
         ch->alloc_ptr = str_dup(arg);
         return;

      case 1:
         if (!ch->alloc_ptr)
         {
            send_to_char("Your search was interrupted!\n\r", ch);
            bug("do_search: alloc_ptr NULL", 0);
            return;
         }
         strcpy(arg, ch->alloc_ptr);
         DISPOSE(ch->alloc_ptr);
         break;
      case SUB_TIMER_DO_ABORT:
         DISPOSE(ch->alloc_ptr);
         ch->substate = SUB_NONE;
         send_to_char("You stop your search...\n\r", ch);
         return;
   }
   ch->substate = SUB_NONE;
   if (arg[0] == '\0')
      startobj = ch->in_room->first_content;
   else
   {
      if ((door = get_door(arg)) != -1)
         startobj = NULL;
      else
      {
         container = get_obj_here(ch, arg);
         if (!container)
         {
            send_to_char("You can't find that here.\n\r", ch);
            return;
         }
         startobj = container->first_content;
      }
   }

   if ((!startobj && door == -1) || IS_NPC(ch))
   {
      send_to_char("You find nothing.\n\r", ch);
      return;
   }

   percent = mastery;

   if (door != -1)
   {
      EXIT_DATA *pexit;

      if ((pexit = get_exit(ch->in_room, door)) != NULL
         && IS_SET(pexit->exit_info, EX_SECRET) && IS_SET(pexit->exit_info, EX_xSEARCHABLE) && number_range(1,100) <= mastery)
      {
         act(AT_SKILL, "Your search reveals the $d!", ch, NULL, pexit->keyword, TO_CHAR);
         act(AT_SKILL, "$n finds the $d!", ch, NULL, pexit->keyword, TO_ROOM);
         REMOVE_BIT(pexit->exit_info, EX_SECRET);
         return;
      }
   }
   else
      for (obj = startobj; obj; obj = obj->next_content)
      {
         if (IS_OBJ_STAT(obj, ITEM_HIDDEN) && number_range(1, 100) <= mastery)
         {
            separate_obj(obj);
            xREMOVE_BIT(obj->extra_flags, ITEM_HIDDEN);
            act(AT_SKILL, "Your search reveals $p!", ch, obj, NULL, TO_CHAR);
            act(AT_SKILL, "$n finds $p!", ch, obj, NULL, TO_ROOM);
            return;
         }
      }

   send_to_char("You find nothing.\n\r", ch);
   return;
}

void set_thief(CHAR_DATA *ch, CHAR_DATA *victim)
{
   INTRO_DATA *intro = NULL;
   CHAR_DATA *rch;
   
   if (!IS_NPC(victim))
   {
      for (intro = victim->pcdata->first_introduction; intro; intro = intro->next)
      {
         if (intro->pid == ch->pcdata->pid && can_see_intro(ch, victim))
         {
            if (intro->value > 0)
               intro->value *=-1;
            intro->value -=15000;
            if (intro->value < -150000)
               intro->value = -150000;
            SET_BIT(intro->flags, INTRO_MYTHIEF);
            REMOVE_BIT(intro->flags, INTRO_THIEF);
            intro->lastseen = time(0);
            break;
         }
      }
   }
   
   if (!IS_NPC(victim) && !intro && can_see_intro(ch, victim))
   {
      CREATE(intro, INTRO_DATA, 1);
      intro->pid = ch->pcdata->pid;
      intro->value = -15000;
      intro->lastseen = time(0);
      SET_BIT(intro->flags, INTRO_MYTHIEF);
      LINK(intro, victim->pcdata->first_introduction, victim->pcdata->last_introduction, next, prev);   
   }

   for (rch = ch->in_room->first_person; rch; rch = rch->next)
   {
      if (!IS_NPC(rch))
      {
         for (intro = rch->pcdata->first_introduction; intro; intro = intro->next)
         {
            if (rch->pcdata->pid == intro->pid && rch != ch && rch != victim && can_see_intro(rch, victim))
            {
               if (intro->value > 100000)
                  continue;
               if (intro->value > 0)
                  intro->value *=-1;
               intro->value -=15000;
               if (intro->value < -150000)
                  intro->value = -150000;
               intro->lastseen = time(0);
              
               if (!IS_SET(intro->flags, INTRO_MYTHIEF))
                   SET_BIT(intro->flags, INTRO_THIEF);
               break;
            }
         }
         if (!intro && can_see_intro(rch, victim) && rch != ch && rch != victim)
         {
            CREATE(intro, INTRO_DATA, 1);
            intro->pid = ch->pcdata->pid;
            intro->value = -15000;
            intro->lastseen = time(0);
            SET_BIT(intro->flags, INTRO_THIEF);
            LINK(intro, rch->pcdata->first_introduction, rch->pcdata->last_introduction, next, prev);   
         }  
         save_char_obj(rch);
      }
   }
   if (!IS_NPC(victim))
      save_char_obj(victim);       
   save_char_obj(ch);
}

void do_gathertinder(CHAR_DATA *ch, char *argument)
{
   int chance = 0;
   OBJ_DATA *tinder;
   int sector;
   
   if (IN_WILDERNESS(ch))
   {
      sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   }
   else
   {
      sector = ch->in_room->sector_type;   
   }
   if (sector == SECT_FOREST || sector == SECT_STREE || sector == SECT_NTREE)
   {
      if (sector == SECT_FOREST)   
         chance = 85;
      if (sector == SECT_STREE)
         chance = 55;
      if (sector == SECT_NTREE)
         chance = 20;
      if (number_range(1, 100) <= chance)
      {
         act(AT_ORANGE, "$n starts to search around for some free tinder and happens to find some.", ch, NULL, NULL, TO_ROOM);
         act(AT_ORANGE, "You start to search around for some free tinder and happen to find some!", ch, NULL, NULL, TO_CHAR);
         tinder = create_object(get_obj_index(OBJ_VNUM_TINDER), 1);
         if (!tinder)
         {
            bug("do_gathertinder.  Missing the tinder vnum!");
            return;
         }
         obj_to_room(tinder, ch->in_room, ch);
         WAIT_STATE(ch, PULSE_PER_SECOND*4);
         return;
      }
      else
      {
         act(AT_GREEN, "$n starts to search around for some free tinder, but happens to find none.", ch, NULL, NULL, TO_ROOM);
         act(AT_GREEN, "You start to search around for some free tinder, but you happen to fine none.", ch, NULL, NULL, TO_CHAR);
         WAIT_STATE(ch, PULSE_PER_SECOND*10);
         return;
      }
   }
   else
   {
      send_to_char("This will only work in an area highly populated with trees.\n\r", ch);
      return;
   }
}

void do_startfire(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *tinder;
   OBJ_DATA *flint;
   OBJ_DATA *knife;
   OBJ_DATA *fire;
   int sector;
   int chance;
   int level = POINT_LEVEL(LEARNED(ch, gsn_startfire), MASTERED(ch, gsn_startfire));
   
   if (IN_WILDERNESS(ch))
   {
      sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   }
   else
   {
      sector = ch->in_room->sector_type;   
   }
   if ((knife = get_eq_char(ch, WEAR_WIELD)) == NULL)
   {
      send_to_char("You need to be wielding some knife like weapon to do this.\n\r", ch);
      return;
   }
   if (wielding_skill_weapon(ch, 0) != 7)
   {
      send_to_char("You need to be wielding some knife like weapon to do this.\n\r", ch);
      return;
   }
   for (tinder = ch->first_carrying; tinder; tinder = tinder->next_content)
   {
      if (tinder->item_type == ITEM_TINDER)
      {
         break;
      }
   }
   if (!tinder)
   {
      send_to_char("You need to have some tinder to start a fire.  See (HELP STARTFIRE) to see how to get tinder.\n\r", ch);
      return;
   }
   for (flint = ch->first_carrying; flint; flint = flint->next_content)
   {
      if (flint->item_type == ITEM_FLINT)
      {
         break;
      }
   }
   if (!flint)
   {
      send_to_char("You need to have a flint to start a fire.  See (HELP STARTFIRE) to see how to get a flint.\n\r", ch);
      return;
   }
   if (sector == SECT_WATER_SWIM || sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER || sector == SECT_AIR
   ||  sector == SECT_OCEANFLOOR || sector == SECT_OCEAN || sector == SECT_RIVER)
   {
      send_to_char("You need dry ground to start a fire.\n\r", ch);
      return;
   }
   
   chance = 1 + (level*3/2);
   
   if (number_range(1, 100) <= chance)
   {
      act(AT_RED, "$n pulls out a $p and strikes it against a flint....a fire starts burning up the tinder.", ch, knife, NULL, TO_ROOM);
      act(AT_RED, "You pull out a $p and strike it against a flint....luckily a fire starts burning up the tinder.", ch, knife, NULL, TO_CHAR);
      learn_from_success(ch, gsn_startfire, NULL);
      fire = create_object(get_obj_index(OBJ_VNUM_FIRE), 1);
      if (!fire)
      {
         bug("do_startfire.  Missing the fire vnum!");
         return;
      }
      WAIT_STATE(ch, PULSE_PER_SECOND*2);
      obj_to_room(fire, ch->in_room, ch);
      separate_obj(tinder);
      obj_from_char(tinder);
      extract_obj(tinder);
      return;
   }
   else
   {
      act(AT_GREEN, "$n pulls out a $p and strikes it against a flint....no such luck though creating a fire.", ch, knife, NULL, TO_ROOM);
      act(AT_GREEN, "You pull out a $p and strike it against a flint.....no luck today, the tinder did not catch fire.", ch, knife, NULL, TO_CHAR);
      learn_from_failure(ch, gsn_startfire, NULL);
      WAIT_STATE(ch, PULSE_PER_SECOND*5);
      return;
   }
}
   
void do_forage(CHAR_DATA * ch, char *argument)
{
   int sector;
   int food;
   char name[MSL];
   char shortd[MSL];
   char longd[MSL];
   char arg[MIL];
   int fv=1;
   int chance;
   int speed = PULSE_PER_SECOND*6;
   OBJ_DATA *ofood;
   
   if (check_npc(ch))
      return;
      
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  forage food [quick/normal/long]\n\r", ch);
      send_to_char("Syntax:  forage water [quick/normal/long]\n\r", ch);
      return;
   }
   
   if (IN_WILDERNESS(ch))
   {
      sector = map_sector[ch->map][ch->coord->x][ch->coord->y];
   }
   else
   {
      sector = ch->in_room->sector_type;   
   }
   argument = one_argument(argument, arg);
   
   if (!str_cmp(arg, "water"))
   {
      for (ofood = ch->first_carrying; ofood; ofood = ofood->next_content)
      {
         if (ofood->item_type == ITEM_DRINK_CON)
         {
            break;
         }
      }
      if (!ofood)
      {
         send_to_char("You need something to hold the water.\n\r", ch);
         return;
      }
      if (ofood->value[1] >= ofood->value[0])
      {
         send_to_char("Your drinking container is full.\n\r", ch);
         return;
      }
      if (sector == SECT_WATER_SWIM || sector == SECT_WATER_NOSWIM || sector == SECT_RIVER)
      {
         act(AT_GREEN, "$n gets on $s knees and fills up $p with fresh water.", ch, ofood, NULL, TO_ROOM);
         act(AT_GREEN, "You get on your knees and fill up $p with fresh water.", ch, ofood, NULL, TO_CHAR);
         ofood->value[1] = ofood->value[0];
         learn_from_success(ch, gsn_forage, NULL);
         WAIT_STATE(ch, speed);
         return;
      }
      else if (sector == SECT_FIELD || sector == SECT_FOREST || sector == SECT_HILLS || sector == SECT_PLAINS ||
               sector == SECT_HCORN || sector == SECT_HGRAIN || sector == SECT_STREE || sector == SECT_NTREE ||
               sector == SECT_SCORN || sector == SECT_NCORN || sector == SECT_SGRAIN || sector == SECT_NGRAIN ||
               sector == SECT_PATH || sector == SECT_STONE || sector == SECT_SSTONE || sector == SECT_NSTONE)
      {
         chance = 5 + (POINT_LEVEL(LEARNED(ch, gsn_forage), MASTERED(ch, gsn_forage)/2));
         if (!str_cmp(argument, "quick"))
         {
            chance -= 15;
            speed = PULSE_PER_SECOND*3;
         }
         else if (!str_cmp(argument, "long"))
         {
            chance += 15;
            speed = PULSE_PER_SECOND*14;
         }
         if (number_range(1, 100) > chance)
         {
            act(AT_GREEN, "$n gets on $s knees and starts to search for water....but finds none.", ch, NULL, NULL, TO_ROOM);
            act(AT_GREEN, "You get on your knees and search for water....no water here.", ch, NULL, NULL, TO_CHAR);
            learn_from_failure(ch, gsn_forage, NULL);
            WAIT_STATE(ch, speed);
            return;
         }   
         else
         {
            act(AT_GREEN, "$n gets on $s knees and fills up $p with fresh water.", ch, ofood, NULL, TO_ROOM);
            act(AT_GREEN, "You get on your knees and fill up $p with fresh water.", ch, ofood, NULL, TO_CHAR);
            separate_obj(ofood);
            ofood->value[1] = ofood->value[0];
            learn_from_success(ch, gsn_forage, NULL);
            WAIT_STATE(ch, speed);
            return;
         }  
      }
      else if (sector == SECT_OCEANFLOOR || sector == SECT_OCEAN || sector == SECT_UNDERWATER)
      {
         send_to_char("You cannot find fresh water here.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("You cannot forage for water here, sorry.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg, "food"))
   {
      if (IN_WILDERNESS(ch) && sector == SECT_HCORN && resource_sector[ch->map][ch->coord->x][ch->coord->y] >= 3000)
      {
         chance = 30 + (POINT_LEVEL(LEARNED(ch, gsn_forage), MASTERED(ch, gsn_forage)/2));
         if (!str_cmp(argument, "quick"))
         {
            chance -= 15;
            speed = PULSE_PER_SECOND*3;
         }
         else if (!str_cmp(argument, "long"))
         {
            chance += 20;
            speed = PULSE_PER_SECOND*14;
         }
         if (number_range(1, 100) > chance)
         {
            act(AT_GREEN, "$n gets on $s knees and starts to forage for food, but is able to find nothing.", ch, NULL, NULL, TO_ROOM);
            act(AT_GREEN, "You get on your knees and start to forage for food, but you are not able to find anything.", ch, NULL, NULL, TO_CHAR);
            learn_from_failure(ch, gsn_forage, NULL);
            WAIT_STATE(ch, speed);
            return;
         }         
         sprintf(name, "fresh corn");
         sprintf(shortd, "fresh corn");
         sprintf(longd, "Someone has left behind some fresh corn for you to eat.\n\r");
         fv = 3;
      }
      else if (sector == SECT_FIELD || sector == SECT_FOREST || sector == SECT_HILLS || sector == SECT_PLAINS
      ||       sector == SECT_STREE || sector == SECT_NTREE)
      {
         chance = 5 + (POINT_LEVEL(LEARNED(ch, gsn_forage), MASTERED(ch, gsn_forage)/2));
         if (!str_cmp(argument, "quick"))
         {
            chance -= 15;
            speed = PULSE_PER_SECOND*3;
         }
         else if (!str_cmp(argument, "long"))
         {
            chance += 15;
            speed = PULSE_PER_SECOND*14;
         }
         if (number_range(1, 100) > chance)
         {
            act(AT_GREEN, "$n gets on $s knees and starts to forage for food, but is able to find nothing.", ch, NULL, NULL, TO_ROOM);
            act(AT_GREEN, "You get on your knees and start to forage for food, but you are not able to find anything.", ch, NULL, NULL, TO_CHAR);
            learn_from_failure(ch, gsn_forage, NULL);
            WAIT_STATE(ch, speed);
            return;
         }
       
         food = number_range(1, 10);
      
         if (food == 1)
         {
            sprintf(name, "barries");
            sprintf(shortd, "barries");
            sprintf(longd, "Someone has left behind some edible barries for you to eat.\n\r");
            fv = 3;
         } 
         if (food == 2)
         {
            sprintf(name, "roots");
            sprintf(shortd, "roots");
            sprintf(longd, "Someone has left behind some tasty looking roots to eat.\n\r");
            fv = 2;
         }      
         if (food == 3)
         {
            sprintf(name, "non-poisonous mushrooms");
            sprintf(shortd, "non-poionous mushrooms");
            sprintf(longd, "Someone has left behind some non-poisonous mushrooms for you to eat.\n\r");
            fv = 3;
         }
         if (food == 4)
         {
            sprintf(name, "fresh grubs");
            sprintf(shortd, "fresh grubs");
            sprintf(longd, "Some ever so tasteful fresh grubs has been left here for all to enjoy.\n\r");
            fv = 2;
         }
         if (food == 5)
         {
            sprintf(name, "deer meat");
            sprintf(shortd, "left over deer meat");
            sprintf(longd, "Someone has left behind some left over deet meat for you to enjoy.\n\r");
            fv = 5;
         }
         if (food == 6)
         {
            sprintf(name, "small apples");
            sprintf(shortd, "small apples");
            sprintf(longd, "Someone has left behind a small assortment of apples for you to enjoy.\n\r");
            fv = 3;
         }
         if (food == 7)
         {
            sprintf(name, "fresh grapes");
            sprintf(shortd, "some frash grapes");
            sprintf(longd, "Someone has left behind a small pile of fresh grapes for you to enjoy.\n\r");
            fv = 3;
         }
         if (food == 8)
         {
            sprintf(name, "honey");
            sprintf(shortd, "fresh honey");
            sprintf(longd, "Someone has left behind a small batch of fresh honey, how delightful!\n\r");
            fv = 3;
         }
         if (food == 9)
         {
            sprintf(name, "eggs");
            sprintf(shortd, "mystery eggs");
            sprintf(longd, "Someone has left behind some eggs, eggs of what, who knows.\n\r");
            fv = 3;
         }
         if (food == 10)
         {
            sprintf(name, "assorted fresh weeds");
            sprintf(shortd, "assorted fresh weeds");
            sprintf(longd, "Someone has left behind an assortment of fresh, edible weeds\n\r");
            fv = 3;
         }
      }
      else
      {
         send_to_char("You cannot forage here, sorry.\n\r", ch);
         return;
      }
      ofood = create_object(get_obj_index(OBJ_VNUM_MUSHROOM), 1);
      STRFREE(ofood->name);
      STRFREE(ofood->short_descr);
      STRFREE(ofood->description);
      ofood->name = STRALLOC(name);
      ofood->short_descr = STRALLOC(shortd);
      ofood->description = STRALLOC(longd);
      ofood->value[0] = fv;
      if (get_ch_carry_number(ch) + (get_obj_number(ofood) / ofood->count) > can_carry_n(ch))
      {
         obj_to_room(ofood, ch->in_room, ch);
         sprintf(name, "Your hands are full, just going to have to leave it on the ground.\n\r");
      }
      else if (get_ch_carry_weight(ch) + (get_obj_weight(ofood) / ofood->count) > can_carry_w(ch))
      {
         obj_to_room(ofood, ch->in_room, ch);
         sprintf(name, "You cannot carry the extra weight, leaving it on the ground.\n\r");
      }
      else
      {
         obj_to_char(ofood, ch);       
         strcpy(name, "");
      }
      act(AT_GREEN, "$n gets on $s knees and starts to forage for food .... and finds $p", ch, ofood, NULL, TO_ROOM);
      act(AT_GREEN, "You get on your knees and start to forage for food .... and you find $p", ch, ofood, NULL, TO_CHAR);
      send_to_char(name, ch);
      WAIT_STATE(ch, speed);
      return;
   }
   else
   { 
      do_forage(ch, "");
      return;
   }
}

void do_cutpurse(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   int percent;
   sh_int mastery, m;
   sh_int level;
   int amount;

   mastery = MASTERED(ch, gsn_cutpurse);
   
   level = POINT_LEVEL(LEARNED(ch, gsn_cutpurse), MASTERED(ch, gsn_cutpurse));

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Attempt a cutpurse on whom?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("That's pointless.\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("A magical force interrupts you.\n\r", ch);
      return;
   }
   
   if (IS_IMMORTAL(victim))
   {
      send_to_char("You cannot steal from immortals.\n\r", ch);
      bug("%s tried to steal from an immortal", ch->name);
      return;
   }

   if (xIS_SET(victim->act, ACT_PACIFIST)) /* Gorog */
   {
      send_to_char("They are a pacifist - Shame on you!\n\r", ch);
      return;
   }
   
   if (wielding_skill_weapon(ch, 0) != 7)
   {
      send_to_char("You can only cutpurse if you are wielding a dagger like weapon.\n\r", ch);
      return;
   }

   if (!ch->fighting)
      WAIT_STATE(ch, skill_table[gsn_cutpurse]->beats*2);
   else
      ch->fight_timer = get_btimer(ch, gsn_cutpurse, NULL);
      
   percent = 20+(level/2) + (IS_AWAKE(victim) ? 0 : 50) + UMIN(10, ((get_curr_lck(ch)-14)/2) + ((get_curr_dex(ch)-14)/2));
   if (mastery < 3 && ch->position != POS_STANDING)
      percent = 0;
   else
   {
       if (ch->position == POS_BERSERK || ch->position == POS_AGGRESSIVE || ch->position == POS_FIGHTING 
       ||  ch->position == POS_DEFENSIVE || ch->position == POS_EVASIVE)
       {
          if (mastery < 4)
             percent /= 2;
       }
       else
          percent = 0;
   }
   if (percent > 0)
   {
      if (mastery == 3)
         percent+=5;
      if (mastery == 4)
         percent+=10;
   }
   if (!IS_NPC(victim) && check_room_pk(ch) < 2)
   {
      send_to_char("You can only steal coins from players in noloot zones or higher.\n\r", ch);
      return;
   }
   if (!can_see(victim, ch))
      percent+=20;
   percent = UMIN(95, percent);
   if (level < number_range(1, 100))
   {
      /*
       * Failure.
       */
      send_to_char("Oops...\n\r", ch);
      learn_from_failure(ch, gsn_cutpurse, victim);
      affect_strip(ch, gsn_invis);
      affect_strip(ch, gsn_mass_invis);
      affect_strip(ch, gsn_sneak);
      affect_strip(ch, gsn_stalk);
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      xREMOVE_BIT(ch->affected_by, AFF_STALK);
      m = 25+(mastery*10);
      if (number_range(1, 100) > m)
      {      
         act(AT_ACTION, "$n tried to steal money from you!", ch, NULL, victim, TO_VICT);
         act(AT_ACTION, "$n tried to steal money from $N.", ch, NULL, victim, TO_NOTVICT);

         if (!IS_NPC(victim))
         {
            if (legal_loot_coins(ch, victim))
            {
               set_thief(ch, victim);
            }
         }
         else
            global_retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
      }
      return;
   }

   if (IS_NPC(ch))
   {
      amount = (int) (victim->gold * number_range(12, 16) / 100);
   }
   else
   {
      if (mastery == 4)
         amount = (int) (victim->gold * number_range(20, 25) / 100);
      else if (mastery == 3)
         amount = (int) (victim->gold * number_range(15, 20) / 100);
      else if (mastery == 2)
         amount = (int) (victim->gold * number_range(12, 16) / 100);
      else
         amount = (int) (victim->gold * number_range(8, 13) / 100);
   }

   if (amount <= 0)
   {
      send_to_char("You couldn't get any gold.\n\r", ch);
      learn_from_failure(ch, gsn_cutpurse, victim);
      return;
   }

   ch->gold += amount;
   victim->gold -= amount;
   ch_printf(ch, "Aha!  You got %d gold coins.\n\r", amount);
   learn_from_success(ch, gsn_cutpurse, victim);
   return;
}
void do_grab(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   int percent;
   sh_int mastery;
   sh_int level;

   mastery = MASTERED(ch, gsn_grab);
   
   level = POINT_LEVEL(LEARNED(ch, gsn_grab), MASTERED(ch, gsn_grab));

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Grab what from whom?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("That's pointless.\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("A magical force interrupts you.\n\r", ch);
      return;
   }
   
   if (IS_IMMORTAL(victim))
   {
      send_to_char("You cannot steal from immortals.\n\r", ch);
      bug("%s tried to steal from an immortal", ch->name);
      return;
   }

   if (xIS_SET(victim->act, ACT_PACIFIST)) /* Gorog */
   {
      send_to_char("They are a pacifist - Shame on you!\n\r", ch);
      return;
   }
   if (ch->position != POS_STANDING)
   {
      send_to_char("You can only grab an item if you are standing.\n\r", ch);
      return;
   }
   if (can_see(victim, ch))
   {
      send_to_char("This will only work if the target cannot see you.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && check_room_pk(ch) < 4)
   {
      send_to_char("You can only steal items from players in fulloot zones or higher.\n\r", ch);
      return;
   }
   if (xIS_SET(victim->act, ACT_GRABBED))
   {
      send_to_char("Your target has already had an attempt made, he/she will not fall for it again.\n\r", ch);
      return;
   }
   if ((obj = get_obj_wear(victim, arg1)) == NULL)
   {
      send_to_char("You can't seem to find the object in question.\n\r", ch);
      return;
   }
   if (!ch->fighting)
      WAIT_STATE(ch, skill_table[gsn_steal]->beats*2);
   else
      ch->fight_timer = get_btimer(ch, gsn_steal, NULL);
      
   percent = 20+(level/2) + (IS_AWAKE(victim) ? 0 : 50) + UMIN(10, ((get_curr_lck(ch)-14)/2) + ((get_curr_dex(ch)-14)/2));
   if (mastery == 3)
      percent+=5;
   if (mastery == 4)
      percent+=10;

   percent = UMIN(95, percent);
   if (level < number_range(1, 100))
   {
      /*
       * Failure.
       */
      send_to_char("Oops...\n\r", ch);
      learn_from_failure(ch, gsn_grab, victim);
      affect_strip(ch, gsn_invis);
      affect_strip(ch, gsn_mass_invis);
      affect_strip(ch, gsn_sneak);
      affect_strip(ch, gsn_stalk);
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      xREMOVE_BIT(ch->affected_by, AFF_STALK);
      if (IS_NPC(victim))
      xSET_BIT(victim->act, ACT_GRABBED);
  
      act(AT_ACTION, "$n tried to grab $p from you!", ch, obj, victim, TO_VICT);
      act(AT_ACTION, "$n tried to grab $p from $N.", ch, obj, victim, TO_NOTVICT);

      if (!IS_NPC(victim))
      {
         if (legal_loot(ch, victim))
         {
            set_thief(ch, victim);
         }
      }
      else
      {
         global_retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
      }
      return;
   }

   if (!can_drop_obj(ch, obj) || IS_OBJ_STAT(obj, ITEM_INVENTORY) || IS_OBJ_STAT(obj, ITEM_PROTOTYPE) || obj->level > ch->level)
   {
      send_to_char("You can't manage to pry it away.\n\r", ch);
      return;
   }
   if (IS_OBJ_STAT(obj, ITEM_NOGIVE))
   {
      send_to_char("You cannot grab an item that is nogive, sorry.\n\r", ch);
      return;
   }
   if (IS_UNIQUE(ch, obj))
   {
      send_to_char("You already have one, cannot grab another.\n\r", ch);
      return;
   }

   if (get_ch_carry_number(ch) + (get_obj_number(obj) / obj->count) > can_carry_n(ch))
   {
      send_to_char("You have your hands full.\n\r", ch);
      return;
   }

   if (get_ch_carry_weight(ch) + (get_obj_weight(obj) / obj->count) > can_carry_w(ch))
   {
      send_to_char("You can't carry that much weight.\n\r", ch);
      return;
   }

   separate_obj(obj);
   unequip_char(victim, obj);
   obj_from_char(obj);
   obj_to_char(obj, ch);
   learn_from_success(ch, gsn_grab, victim);
   adjust_favor(ch, 9, 1);
   affect_strip(ch, gsn_invis);
   affect_strip(ch, gsn_mass_invis);
   affect_strip(ch, gsn_sneak);
   affect_strip(ch, gsn_stalk);
   xREMOVE_BIT(ch->affected_by, AFF_HIDE);
   xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
   xREMOVE_BIT(ch->affected_by, AFF_SNEAK); 
   xREMOVE_BIT(ch->affected_by, AFF_STALK); 
   act(AT_ACTION, "You grab $p from $N.", ch, obj, victim, TO_CHAR);
   act(AT_ACTION, "$n grabs $p from you!", ch, obj, victim, TO_VICT);
   act(AT_ACTION, "$n grabs $p from $N.", ch, obj, victim, TO_NOTVICT);
   if (IS_NPC(victim))
      xSET_BIT(victim->act, ACT_GRABBED);

   if (!IS_NPC(ch))
   {
      if (legal_loot(ch, victim))
      {
         set_thief(ch, victim);
      }
   }
   else
      global_retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
   return;
}
void do_steal(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   int percent;
   sh_int mastery, m;
   sh_int level;

   mastery = MASTERED(ch, gsn_steal);
   
   level = POINT_LEVEL(LEARNED(ch, gsn_steal), MASTERED(ch, gsn_steal));

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Steal what from whom?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("That's pointless.\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("A magical force interrupts you.\n\r", ch);
      return;
   }
   if (IS_IMMORTAL(victim))
   {
      send_to_char("You cannot steal from immortals.\n\r", ch);
      bug("%s tried to steal from an immortal", ch->name);
      return;
   }


/* Disabled stealing among players because of complaints naked avatars were
   running around stealing eq from equipped pkillers. -- Narn
*/
/*    if ( check_illegal_psteal( ch, victim ) )
    {
	send_to_char( "You can't steal from that player.\n\r", ch );
	return;
    }
*/

 /*  if (!IS_NPC(ch) && !IS_NPC(victim))
   {
      set_char_color(AT_IMMORT, ch);
      send_to_char("The gods forbid theft between players.\n\r", ch);
      return;
   } */

   if (xIS_SET(victim->act, ACT_PACIFIST)) /* Gorog */
   {
      send_to_char("They are a pacifist - Shame on you!\n\r", ch);
      return;
   }

   if (!ch->fighting)
      WAIT_STATE(ch, skill_table[gsn_steal]->beats*2);
   else
      ch->fight_timer = get_btimer(ch, gsn_steal, NULL);
   percent = 25+(level*2/3) + (IS_AWAKE(victim) ? 0 : 50) + UMIN(10, ((get_curr_lck(ch)-14)/2) + ((get_curr_dex(ch)-14)/2));
   if (mastery < 3 && ch->position != POS_STANDING)
      percent = 0;
   else
   {
       if (ch->position == POS_BERSERK || ch->position == POS_AGGRESSIVE || ch->position == POS_FIGHTING 
       ||  ch->position == POS_DEFENSIVE || ch->position == POS_EVASIVE)
       {
          if (mastery < 4)
             percent /= 2;
       }
       else
          percent = 0;
   }
   if (percent > 0)
   {
      if (mastery == 3)
         percent+=10;
      if (mastery == 4)
         percent+=20;
   }
   if (!can_see(victim, ch))
      percent+=30;
   if (str_cmp(arg1, "coin") && str_cmp(arg1, "coins") && str_cmp(arg1, "gold"))
   {
      percent/=2;
      if (check_room_pk(ch) < 4 && !IS_NPC(victim))
      {
         send_to_char("You can only steal items from players in fulloot zones or higher.\n\r", ch);
         return;
      }
   }
   else
   {
      if (check_room_pk(ch) < 2 && !IS_NPC(victim))
      {
         send_to_char("You can only steal coins from players in noloot zones or higher.\n\r", ch);
         return;
      }
   }
   percent = UMIN(95, percent);
   if (level < number_range(1, 100))
   {
      /*
       * Failure.
       */
      send_to_char("Oops...\n\r", ch);
      learn_from_failure(ch, gsn_steal, victim);
      affect_strip(ch, gsn_invis);
      affect_strip(ch, gsn_mass_invis);
      affect_strip(ch, gsn_sneak);
      affect_strip(ch, gsn_stalk);
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK); 
      xREMOVE_BIT(ch->affected_by, AFF_STALK); 
      m = 35+(mastery*15);
      if (number_range(1, 100) > m)
      {      
         act(AT_ACTION, "$n tried to steal from you!", ch, NULL, victim, TO_VICT);
         act(AT_ACTION, "$n tried to steal from $N.", ch, NULL, victim, TO_NOTVICT);

         if (!IS_NPC(victim))
         {
            if (legal_loot_coins(ch, victim))
            {
               set_thief(ch, victim);
            }
         }
         else
            global_retcode = one_hit(victim, ch, TYPE_UNDEFINED, LM_BODY);
      }
      return;
   }

   if (!str_cmp(arg1, "coin") || !str_cmp(arg1, "coins") || !str_cmp(arg1, "gold"))
   {
      int amount;

      if (IS_NPC(ch))
      {
         amount = (int) (victim->gold * number_range(1, 7) / 100);
      }
      else
      {
         if (mastery == 4)
            amount = (int) (victim->gold * number_range(3, 13) / 100);
         else if (mastery == 3)
            amount = (int) (victim->gold * number_range(1, 10) / 100);
         else if (mastery == 2)
            amount = (int) (victim->gold * number_range(1, 7) / 100);
         else
            amount = (int) (victim->gold * number_range(1, 5) / 100);
      }

      if (amount <= 0)
      {
         send_to_char("You couldn't get any gold.\n\r", ch);
         learn_from_failure(ch, gsn_steal, victim);
         return;
      }

      ch->gold += amount;
      victim->gold -= amount;
      ch_printf(ch, "Aha!  You got %d gold coins.\n\r", amount);
      learn_from_success(ch, gsn_steal, victim);
      return;
   }

   if ((obj = get_obj_carry(victim, arg1)) == NULL)
   {
      send_to_char("You can't seem to find it.\n\r", ch);
      return;
   }

   if (!can_drop_obj(ch, obj) || IS_OBJ_STAT(obj, ITEM_INVENTORY) || IS_OBJ_STAT(obj, ITEM_PROTOTYPE) || obj->level > ch->level)
   {
      send_to_char("You can't manage to pry it away.\n\r", ch);
      return;
   }
   if (IS_OBJ_STAT(obj, ITEM_NOGIVE))
   {
      send_to_char("You cannot steal an item that is nogive, sorry.\n\r", ch);
      return;
   }
   if (IS_UNIQUE(ch, obj))
   {
      send_to_char("You already have one, cannot steal another.\n\r", ch);
      return;
   }

   if (get_ch_carry_number(ch) + (get_obj_number(obj) / obj->count) > can_carry_n(ch))
   {
      send_to_char("You have your hands full.\n\r", ch);
      return;
   }

   if (get_ch_carry_weight(ch) + (get_obj_weight(obj) / obj->count) > can_carry_w(ch))
   {
      send_to_char("You can't carry that much weight.\n\r", ch);
      return;
   }

   separate_obj(obj);
   obj_from_char(obj);
   obj_to_char(obj, ch);
   if (!can_see(victim, ch))
      level += 60;
   if (number_range(1, 100) > level/2)
   {
      act(AT_ACTION, "You steal $p from $N.  It appears $N might of noticed", ch, obj, victim, TO_CHAR);
      act(AT_ACTION, "You feel like you are missing something.....", ch, obj, victim, TO_VICT);
   }
   else
   {
      act(AT_ACTION, "You steal $p from $N.  It appears $N did not notice it.", ch, obj, victim, TO_CHAR);
   }
   learn_from_success(ch, gsn_steal, victim);
   adjust_favor(ch, 9, 1);
   return;
}

void do_assassinate(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   int percent;
   sh_int mastery;
   int level;

   mastery = MASTERED(ch, gsn_assassinate);
   level = POINT_LEVEL(LEARNED(ch, gsn_assassinate), MASTERED(ch, gsn_assassinate));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't do that right now.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't get close enough while mounted.\n\r", ch);
      return;
   }

   if (arg[0] == '\0')
   {
      send_to_char("Assassinate whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("How can you sneak up on yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;

   if (wielding_skill_weapon(ch, 0) != 7)
   {
      send_to_char("You can only assassinate with a dagger.\n\r", ch);
      return;
   }
   
   if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
   {
      send_to_char("You are not wielding a weapon.\n\r", ch);
      return;
   }

   if (victim->fighting)
   {
      send_to_char("You can't assassinate someone who is in combat.\n\r", ch);
      return;
   }

   /* Can backstab a char even if it's hurt as long as it's sleeping. -Narn */
   if (victim->hit < victim->max_hit && IS_AWAKE(victim))
   {
      act(AT_PLAIN, "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR);
      return;
   }
   percent = 5 + (level/3) + UMIN(10, ((get_curr_dex(ch)-14)/2) + ((get_curr_lck(ch)-14)/2));
   if (can_see(victim, ch))
      percent /= 3;
   percent -= (victim->max_hit - ch->max_hit) / 30;
   if (victim->max_hit >= 5000)
      percent = 0;
   else
      percent = URANGE(5, percent, 95);
   check_attacker(ch, victim);
   if (!IS_AWAKE(victim) || number_range(1, 100) <= percent)
   {
      learn_from_success(ch, gsn_assassinate, victim);
      act(AT_RED, "You slide behind $N and in a quick motion to slit $S throat.  Blood pours EVERYWHERE!", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n slides behind you and in a quick motion slitting your throat.", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n slides behind $N and in a quick motion to slit $S throat.  Blood pours EVERYWHERE!", ch, NULL, victim, TO_NOTVICT);
      damage(ch, victim, 50, TYPE_HIT, DM_DEATH, LM_NECK);
   }
   else
   {
      act(AT_RED, "You slide behind $N and in a quick motion to slit $S throat, but you fail", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n slides behind you and in a quick motion to slit your throat, but fails.", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n slides behind $N and in a quick motion to slit $S throat, but fails.", ch, NULL, victim, TO_NOTVICT);
      
      
      learn_from_failure(ch, gsn_assassinate, victim);
      one_hit(ch, victim, TYPE_HIT, LM_BODY);
   }
   return;
}  

void do_backstab(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   int percent;
   sh_int mastery;
   int level;

   mastery = MASTERED(ch, gsn_backstab);
   level = POINT_LEVEL(LEARNED(ch, gsn_backstab), MASTERED(ch, gsn_backstab));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't do that right now.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't get close enough while mounted.\n\r", ch);
      return;
   }

   if (arg[0] == '\0')
   {
      send_to_char("Backstab whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("How can you sneak up on yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;

   if (wielding_skill_weapon(ch, 0) != 7)
   {
      send_to_char("You can only backstab with a dagger.\n\r", ch);
      return;
   }
   
   if (ch->grip != GRIP_STAB)
   {
      send_to_char("You can only use backstab if you are gripping your dagger for a stab strike.\n\r", ch);
      return;
   }
   if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL)
   {
      send_to_char("You are not wielding a weapon.\n\r", ch);
      return;
   }

   if (victim->fighting)
   {
      send_to_char("You can't backstab someone who is in combat.\n\r", ch);
      return;
   }

   /* Can backstab a char even if it's hurt as long as it's sleeping. -Narn */
   if (victim->hit < victim->max_hit && IS_AWAKE(victim))
   {
      act(AT_PLAIN, "$N is hurt and suspicious ... you can't sneak up.", ch, NULL, victim, TO_CHAR);
      return;
   }
   percent = 40 + (level/2) + UMIN(10, (14-get_curr_dex(ch)) + (14-get_curr_lck(ch)));
   if (!can_see(victim, ch))
      percent+=30;
   if (mastery == 4)
      percent=95;
   percent = UMIN(95, percent);
   check_attacker(ch, victim);
   if (!IS_AWAKE(victim) || number_range(1, 100) <= percent)
   {
      learn_from_success(ch, gsn_backstab, victim);
      global_retcode = one_hit(ch, victim, gsn_backstab, LM_BODY);
      adjust_favor(ch, 10, 1);
      check_illegal_pk(ch, victim);

   }
   else
   {
      learn_from_failure(ch, gsn_backstab, victim);
      global_retcode = damage(ch, victim, 0, gsn_backstab, 0, LM_BODY);
      ch->fight_timer = get_btimer(ch, -1, NULL);

   }
   return;
}


void do_rescue(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   CHAR_DATA *fch;
   int percent;
   int level;
   int adiff = 0;
   int aggroc, aggrov;
   AGGRO_DATA *aggro;
   
   aggroc = aggrov = -1;

   level = POINT_LEVEL(LEARNED(ch, gsn_rescue), MASTERED(ch, gsn_rescue));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (IS_AFFECTED(ch, AFF_BERSERK))
   {
      send_to_char("You aren't thinking clearly...\n\r", ch);
      return;
   }

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Rescue whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("How about fleeing instead?\n\r", ch);
      return;
   }

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }

   if (!IS_NPC(ch) && IS_NPC(victim))
   {
      send_to_char("They don't need your help!\n\r", ch);
      return;
   }

   if (!ch->fighting)
   {
      send_to_char("Too late...\n\r", ch);
      return;
   }

   if ((fch = who_fighting(victim)) == NULL)
   {
      send_to_char("They are not fighting right now.\n\r", ch);
      return;
   }

   if (who_fighting(victim) == ch)
   {
      send_to_char("Just running away would be better...\n\r", ch);
      return;
   }

   if (IS_AFFECTED(victim, AFF_BERSERK))
   {
      send_to_char("Stepping in front of a berserker would not be an intelligent decision.\n\r", ch);
      return;
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }

   if (IS_NPC(victim->fighting->who))
   {    
      for (aggro = victim->fighting->who->first_aggro; aggro; aggro = aggro->next)
      {
         if (aggro->ch == victim)
            aggrov = aggro->value;
         if (aggro->ch == ch)
            aggroc = aggro->value;
      }
      adiff = URANGE(-20, (aggrov - aggroc)/3, 30);
   }
   percent = 10 + URANGE(-3, (get_curr_lck(ch) - 14)/2, 3) + URANGE(-3, (get_curr_lck(victim) - 14)/2, 3);
   percent += URANGE(1, level/3, 25);
   percent -= adiff;
   percent = URANGE(5, percent, 70);

   if (!ch->fighting)
      WAIT_STATE(ch, skill_table[gsn_rescue]->beats*2);
   else
      ch->fight_timer = get_btimer(ch, gsn_rescue, NULL);
      
   if (number_range(1, 100) > percent)
   {
      send_to_char("You fail the rescue.\n\r", ch);
      act(AT_SKILL, "$n tries to rescue you!", ch, NULL, victim, TO_VICT);
      act(AT_SKILL, "$n tries to rescue $N!", ch, NULL, victim, TO_NOTVICT);
      learn_from_failure(ch, gsn_rescue, victim);
      return;
   }

   act(AT_SKILL, "You rescue $N!", ch, NULL, victim, TO_CHAR);
   act(AT_SKILL, "$n rescues you!", ch, NULL, victim, TO_VICT);
   act(AT_SKILL, "$n moves in front of $N!", ch, NULL, victim, TO_NOTVICT);

   learn_from_success(ch, gsn_rescue, victim);
   adjust_favor(ch, 8, 1);
   if (aggrov > aggroc)
   {
      for (aggro = victim->fighting->who->first_aggro; aggro; aggro = aggro->next)
      {
         if (aggro->ch == ch)
            aggro->value = aggrov+1;
      }
   }
   stop_fighting(fch, FALSE);
   stop_fighting(victim, FALSE);
   if (ch->fighting)
      stop_fighting(ch, FALSE);
   set_fighting(ch, fch);
   set_fighting(fch, ch);
   return;
}

//Take up less space, put all the normal checks in here -- Xerves
int check_battle_skills(CHAR_DATA *ch, CHAR_DATA *victim, sh_int usgn)
{

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return 0;
   }

   if (IS_NPC(ch) || ch->pcdata->ranking[usgn] <= 0)
   {
      send_to_char("You better leave the martial arts to those who have the knowledge.\n\r", ch);
      return 0;
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return 0;
   }
   return 1;
}

int get_limb_location args((int limb));

//Similar to one_hit, checks to make sure you hit, adds bonuses
int check_combat_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dam, int at, int wear, int usgn)
{
   sh_int victim_ac = 0;
   sh_int armor_ac = 0;
   sh_int weapon_ac = 0;
   int block = 0;
   sh_int tohit, mastery, hnum, percent, holder;
   OBJ_DATA *eq;
   OBJ_DATA *wield;
   OBJ_DATA *damobj;
   sh_int dameq;
   int miss = 0;
   int counter = 0;
   sh_int mry, pv, tol, lvl;
   int suc = 0;
   int limb;
   OBJ_DATA *shield;
   int cond;
   int noarmor = 0;
   
   holder = -1;
   mry    = MASTERED(ch, usgn);
   pv     = LEARNED(ch, usgn);
   lvl = POINT_LEVEL(pv, mry);
   tol = 2+lvl/6;
   blockdam = 0;
   
   if (wear == WEAR_HEAD)
      limb = LM_HEAD;
   else if (wear == WEAR_NECK)
      limb = LM_NECK;
   else if (wear == WEAR_ARM_R)
      limb = LM_RARM;
   else if (wear == WEAR_ARM_L)
      limb = LM_LARM;
   else if (wear == WEAR_LEG_R)
      limb = LM_RLEG;
   else if (wear == WEAR_LEG_L)
      limb = LM_LLEG;
   else
      limb = LM_BODY;
   
   if ((eq = get_eq_char(victim, wear)) == NULL)
   {
      if (IS_NPC(victim))
      {
         armor_ac = victim->armor;
         weapon_ac += tol;
         if (victim->armor == 0)
            noarmor = 1;
         if (wear == WEAR_HEAD)
            armor_ac += 2;
         else if (wear == WEAR_NECK)
            armor_ac += 3;          
      }
      else
         armor_ac = 0;
   }
   else
   {
      if (at == GRIP_BASH)
      {
         armor_ac = eq->value[0];
         weapon_ac += tol;
      }
      else if (at == GRIP_STAB)
      {
         armor_ac = eq->value[2];
          weapon_ac += tol;
      }
      else if (at == GRIP_SLASH)
      {
         armor_ac = eq->value[1];
          weapon_ac += tol;
      }
   }
   if (armor_ac == 0)
      noarmor = 1;
   if (eq)
      cond = eq->value[3];
   else
      cond = 1000*victim->hit/victim->max_hit;
      
   if ((wield = get_eq_char(ch, WEAR_WIELD)) != NULL)
   {
      if (IS_OBJ_STAT(wield, ITEM_MONKWEAPON))
      {
         if (at == GRIP_BASH)
            weapon_ac += wield->value[7];
         if (at == GRIP_SLASH)
            weapon_ac += wield->value[8];
         if (at == GRIP_STAB)
            weapon_ac += wield->value[9];
      }
   }
   
   if (armor_ac > 0)
   {
      armor_ac -= (3 -(cond / 251));
      if (armor_ac < 1)
         armor_ac = 1;
   }

   victim_ac = armor_ac - weapon_ac;  //negative is a better to hit //negative is a better to hit
   
   /* if you can't see what's coming... */
   if (!can_see(ch, victim))
      victim_ac += 1;
   if (!can_see(victim, ch))
      victim_ac -= 1;
      
   tohit = get_hit_or_miss(ch, victim, victim_ac, at, limb, noarmor, NULL, -1);     
   
   if ((shield = get_eq_char(victim, WEAR_SHIELD)) != NULL && IS_AWAKE(victim))
   {
      int spoints = POINT_LEVEL(GET_POINTS(victim, gsn_shieldblock, 0, 1), GET_MASTERY(victim, gsn_shieldblock, 0, 1));
      if (spoints > 0 && number_range(1, 100) <= UMIN(95, (UMIN(15, URANGE(-20, victim->apply_shield, 20)/2))+ shield->value[2]+ (spoints/4)))
      {
         learn_from_success(victim, gsn_shieldblock, NULL);
         block = 1;
      }
      else
      {
         if (spoints > 0)
            learn_from_failure(victim, gsn_shieldblock, NULL);
      }
   }
   else if (victim->apply_shield > 0)
   {
      if (number_range(1, 100) <= URANGE(-20, victim->apply_shield, 20))
         block = 1;
   }
   
   if (tohit == DM_MISS || block) //miss or block
   {
      /* Miss. */
      miss = 1;
   }
   dam += str_app[get_curr_str(ch)].todam; //Strength Bonus
   if (wield)
   {
      if (IS_OBJ_STAT(wield, ITEM_MONKWEAPON))
      {
         dam += number_range(wield->value[1], wield->value[2]);
      }
   }
   if (dam <= 3)
   {
      if (number_range(1, 5) == 1)
         suc = 1;
   }
   else if (dam <= 6)
   {
      if (number_range(1, 3) == 1)
         suc = 1;
   }
   else if (dam <= 10)
   {
      if (number_range(1, 2) == 1)
         suc = 1;
   }
   else
   {
      suc = 1;
   }
   
   if (miss != 1)
   {
      dam -= URANGE(-6, victim->apply_stone, 6);
      dam -= URANGE(-4, victim->apply_hardening, 4);
      if (victim->apply_hardening > 0)
         check_aff_learn(victim, "bracing", 0, ch, 1);
      if (!IS_NPC(victim) && victim->pcdata->learned[gsn_krundi_style] > 0)
      {
         int level;
         level = POINT_LEVEL(LEARNED(victim, gsn_krundi_style), MASTERED(victim, gsn_krundi_style));
         level = number_range(level*3, level*4);
         level += 100;
         level = URANGE(100, level, 600);
         dam -= level/100;
         level = level % 100;
         if (number_range(1, 100) <= level)
         {
            dam--;
         }
         learn_from_success(victim, gsn_krundi_style, ch);  
      }  
      dam = UMAX(1, dam);   
   }
   
   //Calculated fightstyle damage
   if (miss != 1 && suc)
   {
      dam = get_fightingstyle_dam(victim, dam, ch, 0, 1);
      dam = get_fightingstyle_dam(ch, dam, victim, 1, 1);
   }
   else
   {
      dam = get_fightingstyle_dam(victim, dam, ch, 0, 0);
      dam = get_fightingstyle_dam(ch, dam, victim, 1, 0);   
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_concentration] > 0)
   {
      int dam1, dam2, level;
      level = POINT_LEVEL(LEARNED(ch, gsn_concentration), MASTERED(ch, gsn_concentration));
      mastery = MASTERED(ch, gsn_concentration);
      
      dam1 = level/15;

      if (mastery == 6)
         hnum = number_range(35, 30);
      else if (mastery == 5)
         hnum = number_range(30, 25);
      else if (mastery == 4)
         hnum = number_range(18, 25);
      else if (mastery == 3)
         hnum = number_range(10, 17);
      else if (mastery == 2)
         hnum = number_range(5, 10);
      else
         hnum = number_range(1, 5);
      dam2 = (int) (dam * hnum / 150);
      
      if (dam1 < dam2)
         dam += dam1;
      else
         dam += dam2;
      if (miss != 1 && suc)
         learn_from_success(ch, gsn_concentration, victim);
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_enhanced_damage] > 0)
   {
      int dam1, dam2, level;
      level = POINT_LEVEL(LEARNED(ch, gsn_enhanced_damage), MASTERED(ch, gsn_enhanced_damage));
      mastery = MASTERED(ch, gsn_enhanced_damage);
      
      dam1 = level/10;

      if (mastery == 6)
         hnum = number_range(35, 30);
      else if (mastery == 5)
         hnum = number_range(30, 25);
      else if (mastery == 4)
         hnum = number_range(18, 25);
      else if (mastery == 3)
         hnum = number_range(10, 17);
      else if (mastery == 2)
         hnum = number_range(5, 10);
      else
         hnum = number_range(1, 5);
      dam2 = (int) (dam * hnum / 100);
      
      if (dam1 < dam2)
         dam += dam1;
      else
         dam += dam2;
      if (miss != 1 && suc)
         learn_from_success(ch, gsn_enhanced_damage, victim);
   }
   if (!IS_NPC(ch) && ch->pcdata->learned[gsn_deadly_accuracy] > 0)
   {
      int dam1, dam2, level;
      level = POINT_LEVEL(LEARNED(ch, gsn_deadly_accuracy), MASTERED(ch, gsn_deadly_accuracy));
      mastery = MASTERED(ch, gsn_deadly_accuracy);
      
      dam1 = level/15;

      if (mastery == 6)
         hnum = number_range(35, 30);
      else if (mastery == 5)
         hnum = number_range(30, 25);
      else if (mastery == 4)
         hnum = number_range(18, 25);
      else if (mastery == 3)
         hnum = number_range(10, 17);
      else if (mastery == 2)
         hnum = number_range(5, 10);
      else
         hnum = number_range(1, 5);

      dam2 = (int) (dam * hnum / 150);
      
      if (dam1 < dam2)
         dam += dam1;
      else
         dam += dam2;
      if (miss != 1 && suc)
         learn_from_success(ch, gsn_deadly_accuracy, victim);
   }
   if (!IS_AWAKE(victim))
      dam *= 2;
   
   if (!IS_NPC(victim) && victim->pcdata->learned[gsn_counter] > 0 && victim->pcdata->ranking[gsn_counter] > 0 && !xIS_SET(victim->act, PLR_COUNTER)) 
   {
      percent = number_range(1, 10000);
      mastery = MASTERED(ch, gsn_counter);

      if (mastery == 6)
         hnum = number_range(220, 200);
      else if (mastery == 5)
         hnum = number_range(180, 160);
      else if (mastery == 4)
         hnum = number_range(135, 150);
      else if (mastery == 3)
         hnum = number_range(90, 105);
      else if (mastery == 2)
         hnum = number_range(50, 90);
      else
         hnum = number_range(20, 45);

      if (victim->pcdata->tier > 1)
         holder = holder + 200;
      if (victim->pcdata->caste > 5)
         holder = holder + 100;
      holder += 100 * (get_curr_dex(victim) - get_curr_dex(ch));
      holder += 50 * (get_curr_lck(victim) - get_curr_lck(ch));
      holder += 100 * (get_curr_str(victim) - get_curr_str(ch));
      if (percent < holder)
      {
         counter = 1;
         learn_from_success(victim, gsn_counter, ch);
      }
   }
   if (dam <= 0)
      dam = 1;

   /* immune to damage */
   if (dam == -1)
   {
      return 0;
   }
   
   if(xIS_SET(victim->act, ACT_UNDEAD))
   {
       if(!wield || (wield && !xIS_SET(wield->extra_flags, ITEM_BLESS) && !xIS_SET(wield->extra_flags, ITEM_SANCTIFIED)))
       {
           return 0;
       }
       if(xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur > 0)
       {
          if (dam)
             wield->bless_dur -= 1;
          if(xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur <= 0)
          {
             xTOGGLE_BIT(wield->extra_flags, ITEM_BLESS);
             act(AT_MAGIC, "Your $p stops glowing...\n\r", ch, wield, NULL, TO_CHAR);
          }
       }
   }
   if(xIS_SET(victim->act, ACT_LIVING_DEAD))
   {
       if(!wield || (wield && !xIS_SET(wield->extra_flags, ITEM_BLESS) && !xIS_SET(wield->extra_flags, ITEM_SANCTIFIED)))
       {
          dam = dam / 2;
       }
       if(wield && (xIS_SET(wield->extra_flags, ITEM_BLESS) || xIS_SET(wield->extra_flags, ITEM_SANCTIFIED)))
       {
          dam = dam * 2;
       }
       if(wield && xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur > 0)
       {
          if (dam)
             wield->bless_dur -= 1;
          if(xIS_SET(wield->extra_flags, ITEM_BLESS) && wield->bless_dur <= 0)
          {
              xTOGGLE_BIT(wield->extra_flags, ITEM_BLESS);
              act(AT_MAGIC, "Your $p stops glowing...\n\r", ch, wield, NULL, TO_CHAR);
          }
       }
   }     
   
   
   /* Counter Attack Check -- Xerves 8/25/99 */
   if (counter == 1)
   {  
      damage(victim, ch, dam, TYPE_HIT, DM_COUNTER, LM_BODY);
      return 0;
   }
   
   if (!block && (tohit == DM_HIT || tohit == DM_CRITICAL || tohit == DM_SLICEDLIMB))
   {
      /* damage eq */
      dameq = get_limb_location(limb);
      damobj = get_eq_char(victim, dameq);
      if (damobj)
      {
         if (number_range(1, 100) > (50 + (get_obj_resistance(damobj, victim) * 5/2)))
         {
            set_cur_obj(damobj);
            damage_obj(damobj, victim, 0, dam);
         }
      }
   }
   damobj = NULL;
   // Weapon deteriates over time...
   if (wield && (tohit == DM_HIT || tohit == DM_CRITICAL || tohit == DM_SLICEDLIMB))
   {
      damobj = wield;
      if (damobj)
      {
         if (number_range(1, 100) > (40 + (get_obj_resistance(damobj, ch) * 3)))
         {
            set_cur_obj(damobj);   
            damage_obj(damobj, ch, 0, dam);
         }
      }
   } 
   if (block)
   {
      damobj = get_eq_char(victim, WEAR_SHIELD);
      if (damobj)
      {
         set_cur_obj(damobj);
         damage_obj(damobj, ch, 0, dam);
      }
   }
   
    
   if (tohit == DM_DEATH)  
      return 5000; //Critical attack
   else if (block == 1)
   {
      blockdam = dam;
      return -1;
   }
   else if (tohit == DM_MISS)
      return 0;
   else
   {
      if (tohit > 200)
         return (tohit-100)*dam*100;
      dam = dam*(tohit-100) / 100;
      dam += URANGE(-15, ch->apply_sanctify, 15);
      dam += URANGE(-10, ch->apply_manaburn, 10); 
      if (ch->apply_manaburn > 0 && skill_lookup("arrowcatch") > 0 && LEARNED(ch, skill_lookup("arrowcatch")) > 0)
         learn_from_success(ch, skill_lookup("arrowcatch"), victim);
      if (tohit > -300 && dam <= 0)
         dam = 0;
      return dam;
   }
   return 1;
}

CHAR_DATA *get_battle_target(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim = NULL;
 
   victim = who_fighting(ch);
   
   if ((victim = get_char_room_new(ch, argument, 1)) != NULL)
      return victim;  
   else if ((victim = who_fighting(ch)) != NULL)
      return victim;
   else
      return NULL;
}    
   
	
//Below are the 20 combat skills, enjoy -- Xerves
void do_roundhouse(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_SLASH;
   usgn   = gsn_roundhouse;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = pv;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_HEAD, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_HEAD); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_HEAD);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_HEAD);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_HEAD);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_HEAD);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_HEAD);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}
void do_spinkick(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_SLASH;
   usgn   = gsn_spinkick;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 3 + lvl * 30 / 100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_HEAD, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_HEAD); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_HEAD);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_HEAD);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_HEAD);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_HEAD);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_HEAD);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}
void do_tornadokick(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_tornadokick;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 5 + lvl*40/100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_HEAD, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_HEAD); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_HEAD);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_HEAD);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_HEAD);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_HEAD);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_HEAD);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}
void do_niburo(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_niburo;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 7 + lvl * 55/100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_HEAD, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_HEAD); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_HEAD);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_HEAD);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_HEAD);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_HEAD);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_HEAD);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_neckpinch(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_STAB;
   usgn   = gsn_neckpinch;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
  if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = pv+1;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_NECK, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_NECK); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_NECK);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_NECK);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_NECK);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_NECK);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_NECK);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}
void do_neckchop(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_SLASH;
   usgn   = gsn_neckchop;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 3 + lvl * 35 / 100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_NECK, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_NECK); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_NECK);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_NECK);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_NECK);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_NECK);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_NECK);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}
void do_neckrupture(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_STAB;
   usgn   = gsn_neckrupture;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 5 + lvl * 45/ 100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_NECK, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_NECK); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_NECK);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_NECK);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_NECK);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_NECK);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_NECK);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}
void do_emeru(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_STAB;
   usgn   = gsn_emeru;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 7 + lvl * 60 / 100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_NECK, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_NECK); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_NECK);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_NECK);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_NECK);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_NECK);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_NECK);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_elbowjab(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg[MIL];
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_STAB;
   usgn   = gsn_elbowjab;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
      
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which arm do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_ARM_R;
      limb = LM_RARM;
   }
   else
   {
      wtype = WEAR_ARM_L;
      limb = LM_LARM;
   }

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = pv;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_NECK); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_NECK);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_NECK);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_NECK);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_NECK);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_NECK);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_elbowstab(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   char arg[MIL];
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_STAB;
   usgn   = gsn_elbowstab;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which arm do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_ARM_R;
      limb = LM_RARM;
   }
   else
   {
      wtype = WEAR_ARM_L;
      limb = LM_LARM;
   }

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 3 + lvl * 27/100;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, limb); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, limb);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, limb);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, limb);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, limb);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, limb);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_elbowbreak(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   char arg[MIL];
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_elbowbreak;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which arm do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_ARM_R;
      limb = LM_RARM;
   }
   else
   {
      wtype = WEAR_ARM_L;
      limb = LM_LARM;
   }


   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 5 + lvl*37/100;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, limb); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, limb);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, limb);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, limb);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, limb);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, limb);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_amberio(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   char arg[MIL];
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_SLASH;
   usgn   = gsn_amberio;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which arm do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_ARM_R;
      limb = LM_RARM;
   }
   else
   {
      wtype = WEAR_ARM_L;
      limb = LM_LARM;
   }


   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 7 + lvl*52/100;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, limb); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, limb);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, limb);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, limb);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, limb);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, limb);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_sidekick(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   char arg[MIL];
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_sidekick;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which leg do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_LEG_R;
      limb = LM_RLEG;
   }
   else
   {
      wtype = WEAR_LEG_L;
      limb = LM_LLEG;
   }


   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = pv;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, limb); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, limb);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, limb);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, limb);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, limb);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, limb);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_kneestrike(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   char arg[MIL];
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_kneestrike;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   if (!check_twohand_shield(ch))
      return;
      
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which leg do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_LEG_R;
      limb = LM_RLEG;
   }
   else
   {
      wtype = WEAR_LEG_L;
      limb = LM_LLEG;
   }


   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 3 + lvl * 28/100;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, limb); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, limb);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, limb);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, limb);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, limb);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, limb);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_kneecrusher(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   char arg[MIL];
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_SLASH;
   usgn   = gsn_kneecrusher;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
      
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which leg do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_LEG_R;
      limb = LM_RLEG;
   }
   else
   {
      wtype = WEAR_LEG_L;
      limb = LM_LLEG;
   }


   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 5 + lvl*38/100;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, limb); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, limb);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, limb);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, limb);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, limb);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, limb);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_lembecu(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   char arg[MIL];
   int wtype;
   int limb;
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_lembecu;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   argument = one_argument(argument, arg);
   victim = get_battle_target(ch, arg);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;
   
   if (str_cmp(argument, "right") && str_cmp(argument, "left"))
   {
      send_to_char("Which LEG do you want to strike?.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
   {
      wtype = WEAR_LEG_R;
      limb = LM_RLEG;
   }
   else
   {
      wtype = WEAR_LEG_L;
      limb = LM_LLEG;
   }

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 7 + lvl*53/100;
      dam = check_combat_hit(ch, victim, dam, at, wtype, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, limb); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, limb);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, limb);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, limb);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, limb);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, limb);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_blitz(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_blitz;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = pv;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_BODY, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_BODY); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_BODY);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_BODY);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_BODY);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_BODY);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_BODY);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_spear(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_BASH;
   usgn   = gsn_spear;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 3 + lvl * 25 / 100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_BODY, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_BODY); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_BODY);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_BODY);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_BODY);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_BODY);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_BODY);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_ribpuncture(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_STAB;
   usgn   = gsn_ribpuncture;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 5 + lvl*35/100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_BODY, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_BODY); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_BODY);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_BODY);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_BODY);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_BODY);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_BODY);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_timmuru(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   sh_int lvl; //Point_level
   sh_int mry; //Mastery
   sh_int pv;  //Points
   sh_int usgn; //Gn of the skill
   sh_int at; //Attack type
   sh_int dam;
   
   at     = GRIP_SLASH;
   usgn   = gsn_timmuru;
   mry    = MASTERED(ch, usgn);
   pv     = UMIN(13, LEARNED(ch, usgn));
   lvl    = UMIN(80, POINT_LEVEL(LEARNED(ch, usgn), MASTERED(ch, usgn)));
   victim = get_battle_target(ch, argument);
   
   if (!victim)
   {
      send_to_char("Your target is not here.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   
   if (ch == victim)
   {
      send_to_char("Targetting yourself is not a wise idea.\n\r", ch);
      return;
   }
   
   //Checks the normal stuff to see if they can attack
   if (check_battle_skills(ch, victim, usgn) == 0)
      return;

   ch->fight_timer += get_btimer(ch, usgn, NULL);
   if (can_use_skill(ch, number_percent()-((mry-1)*10), usgn))
   {
      learn_from_success(ch, usgn, victim);
      dam = 7 + lvl*50/100;
      dam = check_combat_hit(ch, victim, dam, at, WEAR_BODY, usgn);
      if (dam == 5000)
         global_retcode = damage(ch, victim, 50, usgn, DM_DEATH, LM_BODY); 
      else if (dam == -1)
         global_retcode = damage(ch, victim, dam, usgn, DM_BLOCK, LM_BODY);
      else if (dam == 0)
         global_retcode = damage(ch, victim, 0, usgn, DM_MISS, LM_BODY);  
      else if (dam <= -2)
         global_retcode = damage(ch, victim, dam*-1, usgn, DM_SLICEDLIMB, LM_BODY);  
      else if (dam >= 20000)
         global_retcode = damage(ch, victim, dam/10000, usgn, DM_CRITICAL, LM_BODY);  
      else
         global_retcode = damage(ch, victim, dam, usgn, 0, LM_BODY);
   }
   else
   {
      learn_from_failure(ch, usgn, victim);
      global_retcode = damage(ch, victim, 0, usgn, 0, -1);
   }
   return;
}

void do_nervepinch(CHAR_DATA *ch, char *argument)
{
   int level;
   CHAR_DATA *victim = NULL;
   int chance = 0;
   AFFECT_DATA af;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_nervepinch), MASTERED(ch, gsn_nervepinch));
   
   if (argument[0] != '\0')
   {
      if ((victim = get_char_room_new(ch, argument, 1)) == NULL)   
      {
         send_to_char("Your target is not here.\n\r", ch);
         return;
      }
   }
   if (!victim && !ch->fighting)
   {
      send_to_char("Syntax:  nervepinch <target>\n\r", ch);
      send_to_char("Note:  You do not have to specify a target if you are fighting.\n\r", ch);
      return;
   }
   
   if (!check_twohand_shield(ch))
      return;
   if (!victim)
      victim = ch->fighting->who;
   
   if (victim == ch)
   {
      send_to_char("Pinching yourself is not a good idea you idiot.\n\r", ch);
      return;
   }
   if (IS_AFFECTED(victim, AFF_NERVEPINCH))
   {
      send_to_char("Your victim is already in pain from a nervepinch, adding to it will not help.\n\r", ch);
      return;
   }
   if (!victim->fighting)
      chance += 15;
   if (!can_see(victim, ch))
      chance += 20;
      
   chance += URANGE(-4, (14 - get_curr_int(victim))*2, 4);
   chance += URANGE(-4, (14 - get_curr_dex(victim))*2, 4);
   chance += level/2;
   chance = URANGE(1, chance, 60);   
   if (victim->max_hit > 1000)
      chance/=2;
   if (victim->max_hit > 5000)
      chance/=2;
   if (victim->max_hit > 10000)
      chance/=4;
   chance = URANGE(1, chance, 60);
   
   if (number_range(1, 100) <= chance)
   {
      act(AT_RED, "$n quickly strikes $N with a deadly nervepinch.  $N starts to twitch in pain!", ch, NULL, victim, TO_NOTVICT);
      act(AT_RED, "You quickly strike at $N with a deadly nervepinch.  $N twiches in pain, success!", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n quickly strikes at you with a deadly nervepinch.  A horribly pain shoots down your spine!", ch, NULL, victim, TO_VICT);
      af.type = gsn_nervepinch;
      af.duration = 2+level/3;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb(AFF_NERVEPINCH);
      affect_join(victim, &af);
      learn_from_success(ch, gsn_nervepinch, victim);
      if (ch->fighting)
         ch->fight_timer = skill_table[gsn_kickdirt]->beats;
      else
         WAIT_STATE(ch, skill_table[gsn_kickdirt]->beats*2);
      if (victim->fighting)
         victim->fight_timer *= 2;
      else
         set_fighting(victim, ch);
      return;
   }
   else
   {
      act(AT_GREEN, "$n quickly strikes $N with a deadly nervepinch, but $N quickly evades it.", ch, NULL, victim, TO_NOTVICT);
      act(AT_GREEN, "You quickly strike at $N with a deadly nervepinch, but $N quickly evades it.", ch, NULL, victim, TO_CHAR);
      act(AT_GREEN, "$n quickly strikes at you with a deadly nervepinch, but you quickly evades it.", ch, NULL, victim, TO_VICT);
      learn_from_failure(ch, gsn_nervepinch, victim);
      if (ch->fighting)
         ch->fight_timer = skill_table[gsn_kickdirt]->beats;
      else
         WAIT_STATE(ch, skill_table[gsn_kickdirt]->beats*2);
      if (!victim->fighting)
         set_fighting(victim, ch);
      return;
   }
}  

//Attempt to cure poison or weaken
void do_cleansing(CHAR_DATA *ch, char *argument)
{
   int level;
   int succ = 0;
   int weaken = (skill_lookup("weaken"));
   
   if (!is_affected(ch, gsn_poison) && !is_affected(ch, weaken))
   {
      send_to_char("You need to be affected by poison or weaken to use this.\n\r", ch);
      return;
   }
   level = POINT_LEVEL(LEARNED(ch, gsn_cleansing), MASTERED(ch, gsn_cleansing));
   act(AT_GREEN, "$n closes $s eyes and starts to chant for spiritual cleansing.", ch, NULL, NULL, TO_ROOM);
   act(AT_GREEN, "You close your eyes and start to chant for spiritual cleansing.", ch, NULL, NULL, TO_CHAR);
   
   level += URANGE(-4, get_curr_wis(ch) - 14, 6);
   level = URANGE(3, level, 90);
   if (number_range(1, 100) > level)
   {
      act(AT_DGREEN, "You are unable to pull up enough energy to cleanse your body.", ch, NULL, NULL, TO_CHAR);
      learn_from_failure(ch, gsn_cleansing, NULL);
      WAIT_STATE(ch, skill_table[gsn_cleansing]->beats*2);
      return;
   }
   if (is_affected(ch, gsn_poison))
   {
      affect_strip(ch, gsn_poison);
      act(AT_BLUE, "You feel the poison being ripped from your body.", ch, NULL, NULL, TO_CHAR);
      ch->mental_state = URANGE(-100, ch->mental_state, -10);
      learn_from_success(ch, gsn_cleansing, NULL);
      succ = 1;
   }
   if (is_affected(ch, weaken))
   {
      affect_strip(ch, weaken);
      act(AT_BLUE, "You feel the weakening in your bones start to lift.", ch, NULL, NULL, TO_CHAR);
      ch->mental_state = URANGE(-100, ch->mental_state, -10);
      if (succ == 0)
         learn_from_success(ch, gsn_cleansing, NULL);
   }
   WAIT_STATE(ch, skill_table[gsn_cleansing]->beats*2);
   return;
}

void do_daze(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   AFFECT_DATA af;
   int chance;
   int level;
   bool fail;

   level = POINT_LEVEL(LEARNED(ch, gsn_daze), MASTERED(ch, gsn_daze));


   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_daze] <= 0)
   {
      send_to_char("You better leave the martial arts to those who are skilled.\n\r", ch);
      return;
   }

   if (!ch->fighting)
   {
      send_to_char("You can only use this during the heat of battle.\n\r", ch);
      return;
   }
   
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      victim = ch->fighting->who;
   }
   if (is_safe(ch, victim))
   {
      send_to_char("You cannot daze that target.\n\r", ch);
      return;
   }
   check_illegal_pk(ch, victim);
   check_attacker(ch, victim);

   if (!IS_NPC(ch) && ch->move < ch->max_move / 10)
   {
      set_char_color(AT_SKILL, ch);
      send_to_char("You are far too tired to do that.\n\r", ch);
      return; /* missing return fixed March 11/96 */
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }

   ch->fight_timer = get_btimer(ch, gsn_daze, NULL);
   fail = FALSE;
   chance = ris_save(victim, level, RIS_PARALYSIS);
   if (chance == 1000)
      fail = TRUE;
      
   chance = 5+chance/4;
      
   if (!IS_NPC(ch) && !IS_NPC(victim))
      chance -= sysdata.stun_plr_vs_plr;
   else
      chance -= sysdata.stun_regular;
      
   chance += ((get_curr_dex(victim) + get_curr_str(victim)) - (get_curr_dex(ch) + get_curr_str(ch))) * 3;
   chance += victim->saving_para_petri;
   chance = URANGE(5, chance, 35);
   if (!fail)
   {
      if (number_range(1, 100) > chance)
         fail = TRUE;
   }

   if (!fail)
   {
      learn_from_success(ch, gsn_daze, victim);
      /*    DO *NOT* CHANGE!    -Thoric    */
      if (!IS_NPC(ch))
      {
         if (MASTERED(ch, gsn_daze) == 4)
            ch->move -= ch->max_move / 20;
         else
            ch->move -= ch->max_move / 15;
      }
      ch->fight_timer = skill_table[gsn_daze]->beats;
      act(AT_SKILL, "$N smashes into you, leaving you dazed!", victim, NULL, ch, TO_CHAR);
      act(AT_SKILL, "You smash into $N, leaving $M dazed!", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n smashes into $N, leaving $M dazed!", ch, NULL, victim, TO_NOTVICT);
      if (!IS_AFFECTED(victim, AFF_PARALYSIS))
      {
         af.type = gsn_daze;
         af.location = APPLY_ARMOR;
         af.modifier = -2;
         af.duration = 4+(MASTERED(ch, gsn_stun));
         af.bitvector = meb(AFF_PARALYSIS);
         affect_to_char(victim, &af);
         update_pos(victim);
      }
      start_hating(victim, ch);
      start_hunting(victim, ch);
   }
   else
   {
      ch->fight_timer = skill_table[gsn_daze]->beats;
      if (!IS_NPC(ch))
      {
         if (MASTERED(ch, gsn_daze) == 4)
            ch->move -= ch->max_move / 30;
         else
            ch->move -= ch->max_move / 25;
      }
      learn_from_failure(ch, gsn_daze, victim);
      act(AT_SKILL, "$n charges at you trying to daze you, but you dodge out of the way.", ch, NULL, victim, TO_VICT);
      act(AT_SKILL, "You try to daze $N, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n charges at $N trying to daze $M, but keeps going right on past.", ch, NULL, victim, TO_NOTVICT);
      start_hating(victim, ch);
      start_hunting(victim, ch);
   }
   return;
}

void do_stun(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   AFFECT_DATA af;
   int chance;
   int level;
   bool fail;

   level = POINT_LEVEL(LEARNED(ch, gsn_stun), MASTERED(ch, gsn_stun));


   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }
   if (!IS_NPC(ch) && ch->pcdata->ranking[gsn_stun] <= 0)
   {
      send_to_char("You better leave the martial arts to those who are skilled.\n\r", ch);
      return;
   }
   if (!ch->fighting || !ch->fighting->who)
   {
      send_to_char("You can only use this during the heat of battle.\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      victim = ch->fighting->who;
   }
   if (is_safe(ch, victim))
   {
      send_to_char("You cannot stun that target.\n\r", ch);
      return;
   }
   check_illegal_pk(ch, victim);
   check_attacker(ch, victim);
   if (!IS_NPC(ch) && ch->move < ch->max_move / 10)
   {
      set_char_color(AT_SKILL, ch);
      send_to_char("You are far too tired to do that.\n\r", ch);
      return; /* missing return fixed March 11/96 */
   }
   if (!IS_NPC(ch) && HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }
   ch->fight_timer = get_btimer(ch, gsn_stun, NULL);
   fail = FALSE;
   chance = ris_save(victim, level, RIS_PARALYSIS);
   if (chance == 1000)
      fail = TRUE;
      
   chance = 15+chance/4;
      
   if (!IS_NPC(ch) && !IS_NPC(victim))
      chance -= sysdata.stun_plr_vs_plr;
   else
      chance -= sysdata.stun_regular;
      
   chance += ((get_curr_dex(victim) + get_curr_str(victim)) - (get_curr_dex(ch) + get_curr_str(ch))) * 3;
   chance += victim->saving_para_petri;
   chance = URANGE(5, chance, 45);
   if (!fail)
   {
      if (number_range(1, 100) > chance)
         fail = TRUE;
   }

   if (!fail)
   {
      learn_from_success(ch, gsn_stun, victim);
      /*    DO *NOT* CHANGE!    -Thoric    */
      if (!IS_NPC(ch))
      {
         if (MASTERED(ch, gsn_stun) == 4)
            ch->move -= ch->max_move / 20;
         else
            ch->move -= ch->max_move / 15;
      }
      ch->fight_timer = skill_table[gsn_stun]->beats;
      act(AT_SKILL, "$N smashes into you, leaving you stunned!", victim, NULL, ch, TO_CHAR);
      act(AT_SKILL, "You smash into $N, leaving $M stunned!", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n smashes into $N, leaving $M stunned!", ch, NULL, victim, TO_NOTVICT);
      if (!IS_AFFECTED(victim, AFF_PARALYSIS))
      {
         af.type = gsn_stun;
         af.location = APPLY_ARMOR;
         af.modifier = -4;
         af.duration = 4+(MASTERED(ch, gsn_stun)*3/2);
         if (IS_NPC(ch))
            af.duration = 2+MASTERED(ch, gsn_stun);
         af.bitvector = meb(AFF_PARALYSIS);
         affect_to_char(victim, &af);
         update_pos(victim);
      }
      start_hating(victim, ch);
      start_hunting(victim, ch);
   }
   else
   {
      ch->fight_timer = skill_table[gsn_stun]->beats;
      if (!IS_NPC(ch))
      {
         if (MASTERED(ch, gsn_stun) == 4)
            ch->move -= ch->max_move / 30;
         else
            ch->move -= ch->max_move / 25;
      }
      learn_from_failure(ch, gsn_stun, victim);
      act(AT_SKILL, "$n charges at you screaming, but you dodge out of the way.", ch, NULL, victim, TO_VICT);
      act(AT_SKILL, "You try to stun $N, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n charges screaming at $N, but keeps going right on past.", ch, NULL, victim, TO_NOTVICT);
      start_hating(victim, ch);
      start_hunting(victim, ch);
   }
   return;
}

/*
 * Disarm a creature.
 * Caller must check for successful attack.
 * Check for loyalty flag (weapon disarms to inventory) for pkillers -Blodkai
 */
void disarm(CHAR_DATA * ch, CHAR_DATA * victim)
{
   OBJ_DATA *obj, *tmpobj;
   char buf[MSL];
   int level;
   int trigger;

   if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL)
      return;

   if ((tmpobj = get_eq_char(victim, WEAR_DUAL_WIELD)) != NULL && number_bits(1) == 0)
      obj = tmpobj;

   if (get_eq_char(ch, WEAR_WIELD) == NULL && number_bits(1) == 0)
   {
      learn_from_failure(ch, gsn_disarm, victim);
      return;
   }

   if (IS_NPC(ch) && !can_see_obj(ch, obj) && number_bits(1) == 0)
   {
      learn_from_failure(ch, gsn_disarm, victim);
      return;
   }

   if (HAS_WAIT(ch))
   {
      return;
   }

   if (check_grip(ch, victim))
   {
      learn_from_failure(ch, gsn_disarm, victim);
      return;
   }

   if (IS_OBJ_STAT(obj, ITEM_NODISARM))
   {
      sprintf(buf, "%s has a nodisarm object", victim->name);
      if (get_trust(ch) > LEVEL_HI_IMM)
         level = get_trust(ch);
      else
         level = LEVEL_HI_IMM;
      log_string_plus(buf, LOG_COMM, level);
      return;
   }
   trigger = MOBtrigger;
   MOBtrigger = TRUE;
   act(AT_SKILL, "&G$n &R****[&G&WDISARMS&R]****&G you!", ch, NULL, victim, TO_VICT);
   act(AT_SKILL, "&GYou &R****[&G&WDISARM&R]****&G $N!", ch, NULL, victim, TO_CHAR);
   act(AT_SKILL, "$n disarms $N!", ch, NULL, victim, TO_NOTVICT);
   MOBtrigger = trigger;
   learn_from_success(ch, gsn_disarm, victim);

   if (obj == get_eq_char(victim, WEAR_WIELD) && (tmpobj = get_eq_char(victim, WEAR_DUAL_WIELD)) != NULL)
      tmpobj->wear_loc = WEAR_WIELD;

   if ((!IS_NPC(victim) && victim->pcdata->quest && victim->pcdata->quest->questarea == victim->in_room->area)
   ||  (IS_OBJ_STAT(obj, ITEM_NOGIVE)) || (IS_OBJ_STAT(obj, ITEM_NODROP)))
   {
      unequip_char(victim, obj);
      obj->wear_loc = -1;
   }
   else
   {
      unequip_char(victim, obj);
      obj->wear_loc = -1;
      obj_from_char(obj);
      obj_to_room(obj, victim->in_room, victim);
   }

   return;
}


void do_disarm(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   int percent;
   sh_int points;

   points = POINT_LEVEL(LEARNED(ch, gsn_disarm), MASTERED(ch, gsn_disarm));
   
   points /= 2;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_disarm] <= 0)
   {
      send_to_char("You don't know how to disarm opponents.\n\r", ch);
      return;
   }

   if (get_eq_char(ch, WEAR_WIELD) == NULL)
   {
      send_to_char("You must wield a weapon to disarm.\n\r", ch);
      return;
   }

   if ((victim = who_fighting(ch)) == NULL)
   {
      send_to_char("You aren't fighting anyone.\n\r", ch);
      return;
   }

   if ((obj = get_eq_char(victim, WEAR_WIELD)) == NULL)
   {
      send_to_char("Your opponent is not wielding a weapon.\n\r", ch);
      return;
   }


   ch->fight_timer = get_btimer(ch, gsn_disarm, NULL);
   percent = points + URANGE(-5, get_curr_lck(ch)-get_curr_lck(victim), 5);
   percent = percent + URANGE(-5, get_curr_str(ch)-get_curr_str(victim), 5);
   percent = URANGE(1, percent, 45);
   if (!can_see_obj(ch, obj))
      percent /= 2;
   if (number_range(1, 100) <= percent)
      disarm(ch, victim);
   else
   {
      send_to_char("You failed.\n\r", ch);
      learn_from_failure(ch, gsn_disarm, victim);
   }
   return;
}


/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip(CHAR_DATA * ch, CHAR_DATA * victim)
{
   int chance;
   
   if (IS_AFFECTED(victim, AFF_FLYING) || IS_AFFECTED(victim, AFF_FLOATING))
      return;

   if (HAS_WAIT(ch))
   {
      return;
   }
   
   chance = 50-((get_curr_str(victim)-14)*2)-((get_curr_dex(victim)-14)*3)-((get_curr_lck(victim)-14));
   chance = URANGE(15, chance, 85);

   if (victim->mount)
   {
      chance /= 3;
      if (IS_AFFECTED(victim->mount, AFF_FLYING) || IS_AFFECTED(victim->mount, AFF_FLOATING))
         return;
      if (number_range(1, 100) <= chance)
      {
         act(AT_SKILL, "$n trips your mount and you fall off!", ch, NULL, victim, TO_VICT);
         act(AT_SKILL, "You trip $N's mount and $N falls off!", ch, NULL, victim, TO_CHAR);
         act(AT_SKILL, "$n trips $N's mount and $N falls off!", ch, NULL, victim, TO_NOTVICT);
         xREMOVE_BIT(victim->mount->act, ACT_MOUNTED);
         victim->mount = NULL;
         if (victim->fighting)
         {
            victim->fight_timer+=2;
         }
         else
         {
            WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
            victim->position = POS_RESTING;
         }
         return;
      }
      else
      {
         act(AT_SKILL, "$n attempts to trip your mount but it does not budge!", ch, NULL, victim, TO_VICT);
         act(AT_SKILL, "You attempt to trip $N's mount but it will not budge!", ch, NULL, victim, TO_CHAR);
         act(AT_SKILL, "$n attempts to trip $N's mount but it will not budge!", ch, NULL, victim, TO_NOTVICT);
      }  
   }
   else
   {
      if (number_range(1, 100) <= chance)
      {
         act(AT_SKILL, "$n trips you and you go down!", ch, NULL, victim, TO_VICT);
         act(AT_SKILL, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR);
         act(AT_SKILL, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT);

         if (victim->fighting)
         {
            victim->fight_timer+=2;
         }
         else
         {
            WAIT_STATE(victim, 2 * PULSE_VIOLENCE);
            victim->position = POS_RESTING;
         }
      }
      else
      {
         act(AT_SKILL, "$n attempts to trip you but fails!", ch, NULL, victim, TO_VICT);
         act(AT_SKILL, "You attempt to trip $N but you fail!", ch, NULL, victim, TO_CHAR);
         act(AT_SKILL, "$n attempts to trip $N but fails!", ch, NULL, victim, TO_NOTVICT);
      }   
   }

   return;
}

void do_begging(CHAR_DATA * ch, char *argument)
{
   int level;
   CHAR_DATA *victim;
   int chance;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_begging), MASTERED(ch, gsn_begging));
   
   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_begging] <= 0)
   {
      send_to_char("You don't know how to beg.\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("Your target is not here for you to beg from.\n\r", ch);
      return;
   }

   if (xIS_SET(victim->act, ACT_AGGRESSIVE))
   {
      send_to_char("Your target doesn't look very friendly, begging is not a good idea.\n\r", ch);
      return;
   }
   if (IS_AFFECTED(ch, AFF_HIDE) || IS_AFFECTED(ch, AFF_INVISIBLE) || IS_AFFECTED(ch, AFF_STALK))
   {
      send_to_char("You can only beg if you are visible.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("You can only use this on NPCs, if you want to beg from players use say.\n\r", ch);
      return;
   }
   if (IS_ACT_FLAG(victim, ACT_MOUNTSAVE))
   {
      send_to_char("You mount doesn't look very interested in helping you out.\n\r", ch);
      return;
   }
   if (victim->race >= MAX_PC_RACE)
   {
      send_to_char("You can only beg from humanoid NPCs.\n\r", ch);
      return;
   }
   if (ch->position != POS_STANDING)
   {
      send_to_char("You have to be standing to beg.\n\r", ch);
      return;
   }
   chance = 30 + (level/2);
   chance -= victim->begatt;
   if (xIS_SET(victim->act, ACT_PACIFIST))
      chance /=2;
    
   WAIT_STATE(ch, skill_table[gsn_begging]->beats*2);
   if (number_range(1, 100) > chance)
   {
      learn_from_failure(ch, gsn_begging, victim);
      victim->begatt++;
      if (!xIS_SET(victim->act, ACT_PACIFIST))
      {
         if (number_range(1, 100) > UMIN(95, 45+(level/2)))
         {
            act(AT_RED, "$n tries to beg for some money from $N, but $N decides $n should DIE instead!", ch, NULL, victim, TO_NOTVICT);
            act(AT_RED, "You try to beg for some money from $N, but $N decides you should DIE instead!", ch, NULL, victim, TO_CHAR);
            one_hit(victim, ch, TYPE_HIT, LM_BODY);
            return;
         }
      }
      act(AT_RED, "$n tries to beg for some money from $N, but $N refuses to give $n any money.", ch, NULL, victim, TO_NOTVICT);
      act(AT_RED, "You try to beg for some money from $N, but $N refuses to give you any money.", ch, NULL, victim, TO_CHAR);
      return;
   }
   else
   {
      learn_from_success(ch, gsn_begging, victim);
      victim->begatt++;
      ch->gold += URANGE(2, number_range(2+level/6, 5+level/6), 15);
      act(AT_RED, "$n tries to beg for some money from $N and $N gives $n a few coins.", ch, NULL, victim, TO_NOTVICT);
      act(AT_RED, "You try to beg for some money from $N and $N gives you a few coins.", ch, NULL, victim, TO_CHAR);
      return;
   }
}

char *const insult_target[4] = {
   "mother", "father", "family", "lover"
};

char *const insult_fun[18] = {
   "a useless tool", "a giant failure", "a nasty cesspool", "a filthy maggot", "a scorge",
   "a scar on humanity", "a worthless f**k", "a stupid f**k", "a giant pile of lard", "an eyesore",
   "a bastard", "a waste of space", "a fairy's b***h", "uglier than a troll", "another stupid fool",
   "a bastard child", "the stupidest motherf****r on this earth", "a son of a b***h"
};

void do_insult(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   int level;
   int chance;
   char buf[MSL];
   int x;
   int y;
   
   if (check_npc(ch))
      return;
   if (argument[0] != '\0')
   {
      if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
      {
         send_to_char("Your target is not here.\n\r", ch);
         return;
      }
   }
   else
   {
      if (!ch->fighting || !ch->fighting->who)
      {
         send_to_char("You need to be fighting to insult someone.\n\r", ch);
         return;
      }
      victim = ch->fighting->who;
   }
   if (!victim->fighting)
   {
      send_to_char("You can only taunt a target that is fighting.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Save the smack for the say and chat channels ok.\n\r", ch);
      return;
   }
   x = number_range(0, 4);
   y = number_range(0, 17);
   if (x == 4)
   {
      sprintf(buf, "$n calls $N %s", insult_fun[y]);
      act(AT_RED, buf, ch, NULL, victim, TO_ROOM);
      sprintf(buf, "You call $N %s", insult_fun[y]);
      act(AT_RED, buf, ch, NULL, victim, TO_CHAR); 
   }
   else
   {
      sprintf(buf, "$n calls $N's %s %s", insult_target[x], insult_fun[y]);
      act(AT_RED, buf, ch, NULL, victim, TO_ROOM);
      sprintf(buf, "You call $N's %s %s", insult_target[x], insult_fun[y]);
      act(AT_RED, buf, ch, NULL, victim, TO_CHAR); 
   }   
   ch->fight_timer = 2;
   level = POINT_LEVEL(LEARNED(ch, gsn_insult), MASTERED(ch, gsn_insult));
   chance = 50 + (level * 5 / 6);
   if (get_curr_int(victim) <= 14)
      chance += 20;
   else if (get_curr_int(victim) <= 16)
      chance += 10;
   else if (get_curr_int(victim) <= 18)
      chance -= 10;
   else if (get_curr_int(victim) <= 20)
      chance -= 30;
   else if (get_curr_int(victim) <= 22)
      chance -= 60;
   else if (get_curr_int(victim) <= 24)
      chance -= 90;
   else
      chance = 0;
      
   if (number_range(1, 100) <= chance)
   {
      learn_from_success(ch, gsn_insult, victim);
      adjust_aggression_list(victim, ch, 0, 2, gsn_insult);
   }
   else
      learn_from_failure(ch, gsn_insult, victim);

   return;
}
            
   
void do_pick(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *gch;
   OBJ_DATA *obj;
   EXIT_DATA *pexit;
   int x, y, z;
   sh_int percent;
   int level;
   TOWN_DATA *town;

   level = POINT_LEVEL(LEARNED(ch, gsn_pick_lock), MASTERED(ch, gsn_pick_lock));


   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Pick what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }
   WAIT_STATE(ch, skill_table[gsn_pick_lock]->beats*2);

   /* look for guards */
   for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
   {
      if (IN_SAME_ROOM(gch, ch) && IS_NPC(gch) && IS_AWAKE(gch))
      {
         act(AT_PLAIN, "$N is standing too close to the lock.", ch, NULL, gch, TO_CHAR);
         return;
      }
   }

   percent = 35 + (level*2/3) + (14-get_curr_lck(ch));
   if (number_range(1, 100) > percent)
   {
      send_to_char("You failed.\n\r", ch);
      learn_from_failure(ch, gsn_pick_lock, NULL);
/*        for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
        {
          if ( IS_NPC(gch) && IS_AWAKE(gch) && xIS_SET(gch->act, ACT_GUARDIAN ) )
            one_hit( gch, ch, TYPE_UNDEFINED );
        }
*/
      return;
   }

   if ((pexit = find_door(ch, arg, TRUE)) != NULL)
   {
      /* 'pick door' */
/*	ROOM_INDEX_DATA *to_room; *//* Unused */
      EXIT_DATA *pexit_rev;

      if (!IS_SET(pexit->exit_info, EX_CLOSED))
      {
         send_to_char("It's not closed.\n\r", ch);
         return;
      }
      if (pexit->key < 0)
      {
         send_to_char("It can't be picked.\n\r", ch);
         return;
      }
      if (!IS_SET(pexit->exit_info, EX_LOCKED))
      {
         send_to_char("It's already unlocked.\n\r", ch);
         return;
      }
      if (IS_SET(pexit->exit_info, EX_PICKPROOF))
      {
         send_to_char("You failed.\n\r", ch);
         learn_from_failure(ch, gsn_pick_lock, NULL);
         check_room_for_traps(ch, TRAP_PICK | trap_door[pexit->vdir]);
         return;
      }

      REMOVE_BIT(pexit->exit_info, EX_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act(AT_ACTION, "$n picks the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      learn_from_success(ch, gsn_pick_lock, NULL);
      adjust_favor(ch, 9, 1);
      /* pick the other side */
      if ((pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room)
      {
         REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
      }
      check_room_for_traps(ch, TRAP_PICK | trap_door[pexit->vdir]);
      return;
   }

   if ((obj = get_obj_here(ch, arg)) != NULL)
   {
      /* 'pick object' */
      if (obj->item_type != ITEM_CONTAINER)
      {
         send_to_char("That's not a container.\n\r", ch);
         return;
      }
      if (!IS_SET(obj->value[1], CONT_CLOSED))
      {
         send_to_char("It's not closed.\n\r", ch);
         return;
      }
      if (obj->value[2] < 0)
      {
         send_to_char("It can't be unlocked.\n\r", ch);
         return;
      }
      if (!IS_SET(obj->value[1], CONT_LOCKED))
      {
         send_to_char("It's already unlocked.\n\r", ch);
         return;
      }
      if (IS_SET(obj->value[1], CONT_PICKPROOF))
      {
         send_to_char("You failed.\n\r", ch);
         learn_from_failure(ch, gsn_pick_lock, NULL);
         check_for_trap(ch, obj, TRAP_PICK, NEW_TRAP_PICK);
         return;
      }

      separate_obj(obj);
      REMOVE_BIT(obj->value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      act(AT_ACTION, "$n picks $p.", ch, obj, NULL, TO_ROOM);
      learn_from_success(ch, gsn_pick_lock, NULL);
      adjust_favor(ch, 9, 1);
      check_for_trap(ch, obj, TRAP_PICK, NEW_TRAP_PICK);
      return;
   }
   x = ch->coord->x;
   y = ch->coord->y;
   if (IN_WILDERNESS(ch) && is_valid_movement(&x, &y, arg, ch))
   {
      if (map_sector[ch->map][x][y] != SECT_LDOOR)
      {
         send_to_char("There is no locked door in that direction.\n\r", ch);
         return;
      }
      town = find_town(x, y, ch->map);
      if (!town)
      {
         bug("do_pick: %s at %d %d has picked a door that does not belong to a town.", ch->name, x, y);
      }
      else
      {
         for (z = 0; z <= 99; z++)
         {
            if (town->doorstate[4][z] > 0)
            {
               if (town->doorstate[5][z] == x && town->doorstate[6][z] == y && town->doorstate[7][z] == ch->map)
               {
                  town->doorstate[0][z] = 1;
                  write_kingdom_file(town->kingdom);
                  break;
               }
            }
         }
         if (z == 100)
         {
            bug("do_open: %s at %d %d in town %s has found a door not belonging to that town", ch->name, x, y, town->name);
         }
      }
      send_to_char("*Click*\n\r", ch);
      act(AT_ACTION, "$n picks the door to the $d.", ch, NULL, arg, TO_ROOM);
      map_sector[ch->map][x][y] = SECT_LDOOR;
      return;
   }

   ch_printf(ch, "You see no %s here.\n\r", arg);
   return;
}

//Allows you to climb over walls depending on what is on the otherside.  Does not allow
//allow moving into "bad" sectors or into roofed areas.  As soon as one is encountered
//it fails.  If you cannot get past the layers of walls you cannot climb in either.
//--Xerves
void do_climbwall(CHAR_DATA *ch, char *argument)
{
   int x = ch->coord->x;
   int y = ch->coord->y;
   int cnt=0;
   int level;
   TOWN_DATA *town;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: climbwall [direction]\n\r", ch);
      return;
   }
   if (!is_valid_movement(&x, &y, argument, ch))
      return;
   if (ch->position != POS_STANDING)
   {
      send_to_char("You have to be standing to climb over a wall.\n\r", ch);
      return;
   }
   if (map_sector[ch->map][x][y] != SECT_WALL && map_sector[ch->map][x][y] != SECT_DWALL
   &&  map_sector[ch->map][x][y] != SECT_NBWALL && map_sector[ch->map][x][y] != SECT_DOOR
   &&  map_sector[ch->map][x][y] != SECT_CDOOR && map_sector[ch->map][x][y] != SECT_LDOOR)
   {
      send_to_char("There is not a wall in that direction.\n\r", ch);
      return;
   }
   level = POINT_LEVEL(LEARNED(ch, gsn_gag), MASTERED(ch, gsn_gag));
   for (;;)
   {
      if (cnt++ > 2+(level/10))
      {
         send_to_char("There is simply too much wall for you to climb in that direction.\n\r", ch);
         return;
      }
      town = find_town(x, y, ch->map);
      if (map_sector[ch->map][x][y] != SECT_WALL && map_sector[ch->map][x][y] != SECT_DWALL
      &&  map_sector[ch->map][x][y] != SECT_NBWALL && map_sector[ch->map][x][y] != SECT_DOOR
      &&  map_sector[ch->map][x][y] != SECT_CDOOR && map_sector[ch->map][x][y] != SECT_LDOOR
      &&  (!town || (town && town->usedpoint[x - town->startx+30][y - town->starty+30] == 0)))  
      {
         if (!sect_show[(int)map_sector[ch->map][x][y]].canpass)
         {
            send_to_char("There is a nopass sector in your wall that direction.\n\r", ch);
            return;
         }
         //Looks to be a free sector lets jump in it
         if (40+level > number_range(1, 100)+(cnt-2)*15)
         {
            send_to_char("You carefully crawl up the wall and land SUCCESSFULLY on the other side.\n\r", ch);
            ch->coord->x = x;
            ch->coord->y = y;
            update_objects(ch, x, y, ch->map);
            if (ch->rider)
            {
               act(AT_WHITE, "$n successfully climbs over the wall with you on $s back.", ch, NULL, ch->rider, TO_VICT);
               ch->rider->coord->x = x;
               ch->rider->coord->y = y;
               do_look(ch->rider, "auto");
               update_objects(ch->rider, x, y, ch->map);
            }
            do_look(ch, "auto");
            learn_from_success(ch, gsn_climbwall, NULL);
            return;
         }
         else
         {
            send_to_char("You attempt to climb the wall but you end up falling off it instead!.\n\r", ch);
            WAIT_STATE(ch, 20);
            learn_from_failure(ch, gsn_climbwall, NULL);
            damage(ch, ch, number_range(5, 10), TYPE_UNDEFINED, 0, -1);
            return;
         }
      }
      else
      {
         if (!is_valid_movement(&x, &y, argument, ch))
            return;  
      }
   }
} 

   

//Removes the gag affect if you happen to have a handy knife around
void do_cutgag(CHAR_DATA *ch, char *argument)
{
   OBJ_DATA *obj;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  cutgag <object>\n\r", ch);
      send_to_char("Syntax:  cutgag none\n\r", ch);
      return;
   }
   if (IS_NPC(ch))
   {
      send_to_char("Not for NPCS!.\n\r", ch);
      return;
   }
   if (ch->position <= POS_SLEEPING)
   {
      send_to_char("You need to be fighting/standing/mounted/resting to remove a gag.\n\r", ch);
      return;
   }
   if (!IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Your aren't gagged, no real reason to do that now is there?", ch);
      return;
   } 
      
   if ((obj = get_obj_carry(ch, argument)) == NULL)
   {
      if (!str_cmp(argument, "none"))
         ;
      else
      {
         send_to_char("You cannot seem to find the object in your inventory.\n\r", ch);
         return;
      }
   }
   if (obj && !IS_OBJ_STAT(obj, ITEM_GAGREMOVE))
   {
      send_to_char("That cannot be used to remove gags.\n\r", ch);
      return;
   }
   affect_strip(ch, gsn_gag);
   if (obj)
   {
      act(AT_WHITE, "$n pulls out $p and cuts the gag off.", ch, obj, NULL, TO_NOTVICT);
      act(AT_WHITE, "You pull out $p and cut the gag off.", ch, obj, NULL, TO_CHAR);
   }
   else
   {
      act(AT_WHITE, "$n works franticly to remove the gag.", ch, NULL, NULL, TO_NOTVICT);
      act(AT_WHITE, "You work franticly to remove the gag.", ch, NULL, NULL, TO_CHAR);
   }
   if (!ch->fighting)
   {
      if (obj)
         WAIT_STATE(ch, 1);
      else
         WAIT_STATE(ch, 8);
   }
   else
   {
      if (obj)
         ch->fight_timer = 1;
      else
         ch->fight_timer = 8;
   }
}
void do_gag(CHAR_DATA * ch, char *argument)
{
   AFFECT_DATA af;
   CHAR_DATA *victim;
   int level;
   int fighting=0;
   char arg1[MIL];
   OBJ_DATA *obj;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  gag <object> <target>\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   
   level = POINT_LEVEL(LEARNED(ch, gsn_gag), MASTERED(ch, gsn_gag));
   
   if (IS_NPC(ch))
   {
      send_to_char("Not for NPCS!.\n\r", ch);
      return;
   }
   if (ch->position != POS_STANDING && ch->position != POS_EVASIVE && ch->position != POS_DEFENSIVE
   &&  ch->position != POS_FIGHTING && ch->position != POS_AGGRESSIVE)
   {
      send_to_char("You can only use this if you are standing, or fighting no higher than aggressive style.\n\r", ch);
      return;
   }
   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }
   
   if (!ch->fighting)
      fighting = 1;
      
   if ((obj = get_obj_carry(ch, arg1)) == NULL)
   {
      send_to_char("You cannot seem to find the object in your inventory.\n\r", ch);
      return;
   }
   if (obj->item_type != ITEM_GAG)
   {
      send_to_char("You cannot use that to gag someone!\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("Your target is not here with you.\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(victim, AFF_GAGGED))
   {
      send_to_char("Your target is already gagged, no use in doing it twice!", ch);
      return;
   } 
   if (IS_ACT_FLAG(victim, ACT_PACIFIST))
   {
      send_to_char("Target is pacifist, cannot do that now!\n\r", ch);
      return;
   }
   if (is_safe(ch, victim))
      return;
   if (!IS_NPC(victim) && !IS_NPC(ch) && get_trust(victim) >= LEVEL_IMMORTAL)
   {
      send_to_char("Sorry, you cannot gag immortals.\n\r", ch);
      return;
   }
     
   separate_obj(obj);
   if (number_range(1, 100) <= (10+(fighting*40)+UMIN(50, level * 2 / 3)))
   {
      //success
      act(AT_WHITE, "$n pulls out $p and shoves it in $N's mouth to shut $M up.", ch, obj, victim, TO_NOTVICT);
      act(AT_WHITE, "You pull out $p and shove it in $N's mouth to shut $M up.", ch, obj, victim, TO_CHAR);
      act(AT_WHITE, "$n pulls out $p and shoves it in your mouth to shut you up.", ch, obj, victim, TO_VICT);
      af.type = gsn_gag;
      af.duration = 5+UMIN(12, level/6);
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb(AFF_GAGGED);
      affect_to_char(victim, &af);
      if (ch->fighting)
         ch->fight_timer = 5 - UMIN(2, level/30);
      else
         WAIT_STATE(ch, 5 - UMIN(2, level/30));
      learn_from_success(ch, gsn_gag, victim);
      obj_from_char(obj);
      extract_obj(obj);
      if (IS_NPC(victim) && !victim->fighting)
         one_hit(victim, ch, TYPE_HIT, LM_BODY);
   }
   else
   {
      act(AT_WHITE, "$n pulls out $p and tries to gag $N but fails.", ch, obj, victim, TO_NOTVICT);
      act(AT_WHITE, "You pull out $p and try to gag $N but fail.", ch, obj, victim, TO_CHAR);
      act(AT_WHITE, "$n pulls out $p and tries to gag you but fails", ch, obj, victim, TO_VICT);
      if (ch->fighting)
         ch->fight_timer = 7 - UMIN(2, level/30);
      else
         WAIT_STATE(ch, 7 - UMIN(2, level/30));
      learn_from_failure(ch, gsn_gag, victim);
      obj_from_char(obj);
      extract_obj(obj);
      if (IS_NPC(victim) && !victim->fighting)
         one_hit(victim, ch, TYPE_HIT, LM_BODY);
   }
}

void get_wilderness_move(CHAR_DATA *ch, int dir)
{
   if (dir == 0)
      do_north(ch, "");
   else if (dir == 1)
      do_east(ch, "");
   else if (dir == 2)
      do_south(ch, "");
   else if (dir == 3)
      do_west(ch, "");
   else if (dir == 6)
      do_northeast(ch, "");
   else if (dir == 7)
      do_northwest(ch, "");
   else if (dir == 8)
      do_southeast(ch, "");
   else if (dir == 9)
      do_southwest(ch, "");
}

void do_stalk(CHAR_DATA * ch, char *argument)
{
   AFFECT_DATA af;
   sh_int level;
   int dir;

   level = POINT_LEVEL(LEARNED(ch, gsn_stalk), MASTERED(ch, gsn_stalk));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }
   
   if ((dir = get_door(argument)) == -1)
   {
      send_to_char("Stalk in WHAT direction?\n\r", ch);
      return;
   }
   if (!IN_WILDERNESS(ch) && !(get_exit(ch->in_room, dir)))
   {
      send_to_char("You cannot go that direction.\n\r", ch);
      return;
   }

   ch_printf(ch, "You attempt to stalk silently %s\n\r", argument);
   affect_strip(ch, gsn_stalk);

   af.type = gsn_stalk;
   af.duration = 10+level/2;
   af.location = APPLY_NONE;
   af.modifier = level;
   af.bitvector = meb(AFF_STALK);
   affect_to_char(ch, &af);
   if (IN_WILDERNESS(ch))
   {
      get_wilderness_move(ch, dir);
   }
   else
      move_char(ch, get_exit(ch->in_room, dir), 0);
   return;
}

void do_sneak(CHAR_DATA * ch, char *argument)
{
   AFFECT_DATA af;
   sh_int level;
   sh_int mastery;

   mastery = MASTERED(ch, gsn_sneak) * 20;
   mastery = mastery - 60;

   level = POINT_LEVEL(LEARNED(ch, gsn_sneak), MASTERED(ch, gsn_sneak));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }

   send_to_char("You attempt to move silently.....\n\r", ch);
   affect_strip(ch, gsn_sneak);

   if (can_use_skill(ch, number_percent() - mastery, gsn_sneak))
   {
      send_to_char("You feel you have the ability to move secretly now...\n\r", ch);
      af.type = gsn_sneak;
      af.duration = 600+level*6;
      af.location = APPLY_NONE;
      af.modifier = 0;
      af.bitvector = meb(AFF_SNEAK);
      affect_to_char(ch, &af);
      learn_from_success(ch, gsn_sneak, ch);
   }
   else
      learn_from_failure(ch, gsn_sneak, ch);

   return;
}



void do_hide(CHAR_DATA * ch, char *argument)
{
   sh_int mastery;

   mastery = MASTERED(ch, gsn_hide) * 20;
   mastery = mastery - 60;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }
   
   if (!IS_NPC(ch) && !is_nighttime() && LEARNED(ch, gsn_lightprawl) <= 0)
   {
      send_to_char("You can only hide during the day if you have knowledge of lightprawl.\n\r", ch);
      return;
   }

   send_to_char("You attempt to hide....\n\r", ch);

   if (IS_AFFECTED(ch, AFF_HIDE))
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      
   if (ch->race == RACE_FAIRY)
   {
      xSET_BIT(ch->affected_by, AFF_HIDE);
      if (ch->pcdata->learned[gsn_hide] > 0)
         learn_from_success(ch, gsn_hide, ch);
      return;
   }

   if (can_use_skill(ch, number_percent() - mastery, gsn_hide))
   {
      xSET_BIT(ch->affected_by, AFF_HIDE);
      learn_from_success(ch, gsn_hide, ch);
   }
   else
      learn_from_failure(ch, gsn_hide, ch);
   return;
}



/*
 * Contributed by Alander.
 */
void do_visible(CHAR_DATA * ch, char *argument)
{
   affect_strip(ch, gsn_invis);
   affect_strip(ch, gsn_mass_invis);
   affect_strip(ch, gsn_sneak);
   affect_strip(ch, gsn_stalk);
   xREMOVE_BIT(ch->affected_by, AFF_HIDE);
   xREMOVE_BIT(ch->affected_by, AFF_INVISIBLE);
   xREMOVE_BIT(ch->affected_by, AFF_SNEAK); 
   xREMOVE_BIT(ch->affected_by, AFF_STALK); 
   send_to_char("Ok.\n\r", ch);
   return;
}


void do_recall(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *location;
   CHAR_DATA *opponent;

   location = NULL;

   if (IS_NPC(ch))
      return;

   if (!IS_NPC(ch) && ch->pcdata->caste < 2)
      location = get_room_index(5644);

   if (!IS_NPC(ch) && ch->pcdata->clan)
      location = get_room_index(ch->pcdata->clan->recall);

   /* Hometown code - Xerves */
   /* Replaced with Kingdom, pretty much the same damn thing -- Xerves 12/99 */
   if (!location && ch->pcdata->town)
      location = get_room_index(OVERLAND_SOLAN);

   if (!location)
      location = get_room_index(ROOM_VNUM_TEMPLE);
      
   if (!str_cmp(argument, "rolen"))
      location = get_room_index(ROOM_VNUM_TEMPLE);

   if (!location)
   {
      send_to_char("You are completely lost.\n\r", ch);
      return;
   }

   if (ch->in_room == location && location->vnum != OVERLAND_SOLAN)
      return;
      
   if (ch->pcdata->town && location->vnum == OVERLAND_SOLAN && ch->coord->x == ch->pcdata->town->recall[0]
   &&  ch->coord->y == ch->pcdata->town->recall[1] && ch->map == ch->pcdata->town->recall[2])
      return;

   if (ch->fighting)
   {
      send_to_char("You cannot recall during a battle now, flee.\n\r", ch);
      return;
   }

   if (xIS_SET(ch->in_room->room_flags, ROOM_NO_RECALL))
   {
      send_to_char("For some strange reason... nothing happens.\n\r", ch);
      return;
   }

   if (in_hellmaze(ch))
   {
      send_to_char("The only way out of the maze from hell is death or reaching the end.\n\r", ch);
      return;
   }
   
   if (get_timer(ch, TIMER_RECENTFIGHT) >= 1)
   {
      send_to_char("Your blood is pumping too much to do this at this time.\n\r", ch);
      return;
   }

   if (IS_AFFECTED(ch, AFF_CURSE))
   {
      send_to_char("You are cursed and cannot recall!\n\r", ch);
      return;
   }

   if (xIS_SET(ch->act, PLR_GAMBLER))
   {
      send_to_char("You cannot recall while you are gambling!\n\r", ch);
      return;
   }
   
   if (ch->ship)
   {
      send_to_char("You cannot recall while on a ship.\n\r", ch);
      return;
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }

   if ((opponent = who_fighting(ch)) != NULL)
   {
      if (number_bits(1) == 0 || (!IS_NPC(opponent) && number_bits(3) > 1))
      {
         WAIT_STATE(ch, 4);
         ch_printf(ch, "You failed!\n\r");
         return;
      }
      ch_printf(ch, "You recall from combat!\n\r");
      stop_fighting(ch, TRUE);
   }
   act(AT_ACTION, "$n disappears in a swirl of smoke.", ch, NULL, NULL, TO_ROOM);
   ch->coord->x = ch->coord->y = ch->map = -1;
   REMOVE_ONMAP_FLAG(ch);
   if (ch->on)
   {
      ch->on = NULL;
      ch->position = POS_STANDING;
   }
   if (ch->position != POS_STANDING && ch->position != POS_RIDING)
   {
      ch->position = POS_STANDING;
   }

   char_from_room(ch);
   char_to_room(ch, location);
   if (location->vnum == OVERLAND_SOLAN)
   {
      ch->coord->x = ch->pcdata->town->recall[0];
      ch->coord->y = ch->pcdata->town->recall[1];
      ch->map = ch->pcdata->town->recall[2];
      SET_ONMAP_FLAG(ch);
      if (ch->mount)
      {
         char_from_room(ch->mount);
         char_to_room(ch->mount, location);
         ch->mount->coord->x = ch->coord->x;
         ch->mount->coord->y = ch->coord->y;
         ch->mount->map = ch->map;
         SET_ONMAP_FLAG(ch->mount);
         do_look(ch->mount, "auto");
      }  
      if (!IS_NPC(ch) && ch->pcdata->pet)
      {
         char_from_room(ch->pcdata->pet);
         char_to_room(ch->pcdata->pet, location);
         ch->pcdata->pet->coord->x = ch->coord->x;
         ch->pcdata->pet->coord->y = ch->coord->y;
         ch->pcdata->pet->map = ch->map;
         SET_ONMAP_FLAG(ch->pcdata->pet);
         do_look(ch->pcdata->pet, "auto");
      }  
      if (!IS_NPC(ch) && ch->pcdata->mount && !ch->mount)
      {
         char_from_room(ch->pcdata->mount);
         char_to_room(ch->pcdata->mount, location);
         ch->pcdata->mount->coord->x = ch->coord->x;
         ch->pcdata->mount->coord->y = ch->coord->y;
         ch->pcdata->mount->map = ch->map;
         SET_ONMAP_FLAG(ch->pcdata->mount);
         do_look(ch->pcdata->mount, "auto");
      }  
      if (ch->rider)
      {
         char_from_room(ch->rider);
         char_to_room(ch->rider, location);
         ch->rider->coord->x = ch->coord->x;
         ch->rider->coord->y = ch->coord->y;
         ch->rider->map = ch->map;
         SET_ONMAP_FLAG(ch->rider);
         update_objects(ch->rider, ch->rider->map, ch->rider->coord->x, ch->rider->coord->y);
         do_look(ch->rider, "auto");
      }  
      if (ch->riding)
      {
         char_from_room(ch->riding);
         char_to_room(ch->riding, location);
         ch->riding->coord->x = ch->coord->x;
         ch->riding->coord->y = ch->coord->y;
         ch->riding->map = ch->map;
         SET_ONMAP_FLAG(ch->riding);
         update_objects(ch->riding, ch->riding->map, ch->riding->coord->x, ch->riding->coord->y);
         do_look(ch->riding, "auto");
      } 
   }
   else
   {
      if (ch->mount)
      {
         ch->mount->coord->x = ch->mount->coord->y = ch->mount->map = -1;
         REMOVE_ONMAP_FLAG(ch->mount);
         char_from_room(ch->mount);
         char_to_room(ch->mount, location);
         update_objects(ch->mount, ch->mount->map, ch->mount->coord->x, ch->mount->coord->y);
         do_look(ch->mount, "auto");
      }
      if (ch->pcdata->pet)
      {
         ch->pcdata->pet->coord->x = ch->pcdata->pet->coord->y = ch->pcdata->pet->map = -1;
         REMOVE_ONMAP_FLAG(ch->pcdata->pet);
         char_from_room(ch->pcdata->pet);
         char_to_room(ch->pcdata->pet, location);
         do_look(ch->pcdata->pet, "auto");
      }
      if (ch->pcdata->mount && !ch->mount)
      {
         ch->pcdata->mount->coord->x = ch->pcdata->mount->coord->y = ch->pcdata->mount->map = -1;
         REMOVE_ONMAP_FLAG(ch->pcdata->mount);
         char_from_room(ch->pcdata->mount);
         char_to_room(ch->pcdata->mount, location);
         do_look(ch->pcdata->mount, "auto");
      }
      if (ch->rider)
      {
         ch->rider->coord->x = ch->rider->coord->y = ch->rider->map = -1;
         REMOVE_ONMAP_FLAG(ch->rider);
         char_from_room(ch->rider);
         char_to_room(ch->rider, location);
         do_look(ch->rider, "auto");
      }   
      if (ch->riding)
      {
         ch->riding->coord->x = ch->riding->coord->y = ch->riding->map = -1;
         REMOVE_ONMAP_FLAG(ch->riding);
         char_from_room(ch->riding);
         char_to_room(ch->riding, location);
         do_look(ch->riding, "auto");
      }   
   }
   update_objects(ch, ch->map, ch->coord->x, ch->coord->y);
   act(AT_ACTION, "$n appears in the room.", ch, NULL, NULL, TO_ROOM);
   do_look(ch, "auto");

   return;
}


void do_aid(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   int percent;
   int mastery;

   mastery = MASTERED(ch, gsn_aid) * 15;
   mastery = mastery - 60;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Aid whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (IS_NPC(victim)) /* Gorog */
   {
      send_to_char("Not on mobs.\n\r", ch);
      return;
   }

   if (ch->mount)
   {
      send_to_char("You can't do that while mounted.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Aid yourself?\n\r", ch);
      return;
   }

   if (victim->position > POS_STUNNED)
   {
      act(AT_PLAIN, "$N doesn't need your help.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (victim->hit <= -6)
   {
      act(AT_PLAIN, "$N's condition is beyond your aiding ability.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }

   percent = number_percent() - (get_curr_lck(ch) - 13);
   if (!ch->fighting)
      WAIT_STATE(ch, skill_table[gsn_aid]->beats*2);
   else
      ch->fight_timer = get_btimer(ch, gsn_aid, NULL);
      
   if (!can_use_skill(ch, percent + mastery, gsn_aid))
   {
      send_to_char("You fail.\n\r", ch);
      learn_from_failure(ch, gsn_aid, victim);
      return;
   }

   act(AT_SKILL, "You aid $N!", ch, NULL, victim, TO_CHAR);
   act(AT_SKILL, "$n aids $N!", ch, NULL, victim, TO_NOTVICT);
   learn_from_success(ch, gsn_aid, victim);
   adjust_favor(ch, 8, 1);
   if (victim->hit < 1)
      victim->hit = 1;

   update_pos(victim);
   act(AT_SKILL, "$n aids you!", ch, NULL, victim, TO_VICT);
   return;
}

//Allow a PC to give another PC a ride....Be scared of the hobbit totting
//ogres
void do_piggyback(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("You can't find that here.\n\r", ch);
      return;
   }
   if (ch->mount)
   {
      send_to_char("Might be a good idea to dismount whatever you are on right now.\n\r", ch);
      return;
   }
   if (victim->rider)
   {
      send_to_char("Your target already has a rider.\n\r", ch);
      return;
   }
   if (ch->riding)
   {
      send_to_char("You are already hitching a ride.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_NORIDERS))
   {
      send_to_char("Your target would crush you like a peanut if you even attempted.\n\r", ch);
      return;
   }
   if (IS_NPC(victim) && !xIS_SET(victim->act, ACT_ALLOWRIDE))
   {
      send_to_char("You cannot piggyback that mob.\n\r", ch);
      return;
   }

   if (victim->position < POS_STANDING)
   {
      send_to_char("Your target must be standing.\n\r", ch);
      return;
   }

   if (victim->position == POS_FIGHTING || victim->fighting)
   {
      send_to_char("Your mount is moving around too much.\n\r", ch);
      return;
   }
   if (get_ch_carry_weight(ch)+ ch->weight > can_carry_w(victim))
   {
      send_to_char("You weight too much for your target to carry.\n\r", ch);
      return;
   }
   ch->riding = victim;
   victim->rider = ch;
    /* Take away Hide + Sneak */
    
   if (IS_AFFECTED(ch, AFF_STALK))
   {
      xREMOVE_BIT(ch->affected_by, AFF_STALK);
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      affect_strip(ch, gsn_stalk);
      act(AT_SKILL, "You appear from the shadows amd jump on $N's back.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n appears from the shadows and jumps on the back of $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n appears from the shadows and then jumps on YOUR BACK.", ch, NULL, victim, TO_VICT);
   }
   else if (IS_AFFECTED(ch, AFF_HIDE) && IS_AFFECTED(ch, AFF_SNEAK))
   {
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      act(AT_SKILL, "You stop sneaking, appear from the shadows, and then jump on $N's back.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n stops sneaking, appears from the shadows, and jumps on the back of $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n stops sneaking, appears from the shadows, and then jumps on YOUR BACK.", ch, NULL, victim, TO_VICT);
   }
   else if (IS_AFFECTED(ch, AFF_HIDE))
   {
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      act(AT_SKILL, "You appear from the shadows amd jump on $N's back.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n appears from the shadows and jumps on the back of $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n appears from the shadows and then jumps on YOUR BACK.", ch, NULL, victim, TO_VICT);
   }
   else if (IS_AFFECTED(ch, AFF_SNEAK))
   {
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      act(AT_SKILL, "You stop sneaking and then jump on $N's back.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n stops sneaking and jumps on the back of $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n stops sneaking and then jumps on YOUR BACK.", ch, NULL, victim, TO_VICT);
   }
   else
   {   
      act(AT_SKILL, "You jump on the back of $N.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n skillfully jumps on the back of $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n jumps on YOUR BACK.", ch, NULL, victim, TO_VICT);
   }
   ch->position = POS_RIDING;
   return;
}

void do_mount(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;

   if (ch->mount)
   {
      send_to_char("You're already mounted!\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("You can't find that here.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) || !xIS_SET(victim->act, ACT_MOUNTABLE))
   {
      send_to_char("You can't mount that!\n\r", ch);
      return;
   }
   
   if (ch->riding)
   {
      send_to_char("You need to be not riding someone to use mount.\n\r", ch);
      return;
   }

   if (xIS_SET(victim->act, ACT_MOUNTED))
   {
      send_to_char("That mount already has a rider.\n\r", ch);
      return;
   }

   if (victim->position < POS_STANDING)
   {
      send_to_char("Your mount must be standing.\n\r", ch);
      return;
   }

   if (victim->position == POS_FIGHTING || victim->fighting)
   {
      send_to_char("Your mount is moving around too much.\n\r", ch);
      return;
   }
   if (xIS_SET(victim->act, ACT_MOUNTSAVE) && (ch->pcdata->mount == NULL))
   {
      send_to_char("Only the owner can mount this one.\n\r", ch);
      return;
   }
   if (xIS_SET(victim->act, ACT_MOUNTSAVE) && (ch->pcdata->mount != victim))
   {
      send_to_char("Only the owner can mount this one.\n\r", ch);
      return;
   }
   if (get_ch_carry_weight(ch) > can_carry_w(victim))
   {
      send_to_char("Your mount thinks you weight a bit too much to try that.\n\r", ch);
      return;
   }
   xSET_BIT(victim->act, ACT_MOUNTED);
   ch->mount = victim;
    /* Take away Hide + Sneak */
   
   if (IS_AFFECTED(ch, AFF_STALK))
   {
      xREMOVE_BIT(ch->affected_by, AFF_STALK);
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      affect_strip(ch, gsn_stalk);
      act(AT_SKILL, "You appear from the shadows to mount $N.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n appears from the shadows and skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n appears from the shadows to mounts you.", ch, NULL, victim, TO_VICT);
   } 
   else if (IS_AFFECTED(ch, AFF_HIDE) && IS_AFFECTED(ch, AFF_HIDE))
   {
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      act(AT_SKILL, "You stop sneaking and appear from the shadows to mount $N.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n stops sneaking and appears from the shadows to skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n stops sneaking and appears from the shadows to mounts you.", ch, NULL, victim, TO_VICT);
   }
   else if (IS_AFFECTED(ch, AFF_HIDE))
   {
      xREMOVE_BIT(ch->affected_by, AFF_HIDE);
      act(AT_SKILL, "You appear from the shadows to mount $N.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n appears from the shadows and skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n appears from the shadows to mounts you.", ch, NULL, victim, TO_VICT);
   }
   else if (IS_AFFECTED(ch, AFF_SNEAK))
   {
      xREMOVE_BIT(ch->affected_by, AFF_SNEAK);
      act(AT_SKILL, "You stop sneaking and mount $N.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n stops sneaking and skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n stops sneaking and mounts you.", ch, NULL, victim, TO_VICT);
   }
   else
   {   
      act(AT_SKILL, "You mount $N.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n mounts you.", ch, NULL, victim, TO_VICT);
   }
   ch->position = POS_MOUNTED;
   return;
}

void do_toss(CHAR_DATA *ch, char *argument)
{
   int dam;
   CHAR_DATA *victim;
   char arg[MIL];
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  toss [soft/hard]\n\r", ch);
      send_to_char("Syntax:  toss at <victim>\n\r", ch);
      return;
   }
   if (!ch->rider)
   {
      send_to_char("You have no one currently on your back to toss off.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "soft"))
   {       
      act(AT_SKILL, "You gently toss $N off of his back and onto the ground.", ch, NULL, ch->rider, TO_CHAR);
      act(AT_SKILL, "$n gently tosses $N off of $s back and onto the ground.", ch, NULL, ch->rider, TO_NOTVICT);
      act(AT_SKILL, "$n gently tosses you off of $s back and onto the ground.", ch, NULL, ch->rider, TO_VICT);
      ch->rider->position = POS_STANDING;
      ch->rider->riding = NULL;
      ch->rider = NULL;
      return;
   }
   if (!str_cmp(arg, "hard"))
   {
      act(AT_SKILL, "You grab $N and toss $S with great velocity toward the ground.", ch, NULL, ch->rider, TO_CHAR);
      act(AT_SKILL, "$n grabs $N and tosses $S with great velocity toward the ground.", ch, NULL, ch->rider, TO_NOTVICT);
      act(AT_SKILL, "$n grabs you and tosses you with great velocity toward the ground.", ch, NULL, ch->rider, TO_VICT);
      dam = URANGE(-4, (get_curr_str(ch)-13)/2, 4)+5;
      ch->rider->position = POS_STANDING;
      ch->rider->riding = NULL;
      damage(ch->rider, ch->rider, dam, TYPE_UNDEFINED, 0, -1);
      ch->rider = NULL;
      return;   
   }
   if (!str_cmp(arg, "at"))
   {
      if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
      {
         send_to_char("Your target is not in the room with you.\n\r", ch);
         return;
      }
      if (is_safe(ch, victim))
      {
         send_to_char("This room is safe from fighting, sorry.\n\r", ch);
         return;
      }
      if (!IS_NPC(victim) && !IS_NPC(ch) && get_trust(ch) >= LEVEL_IMMORTAL)
      {
         sprintf(log_buf, "%s: immortal attempting player murder of %s.", ch->name, victim->name);
         log_string_plus(log_buf, LOG_NORMAL, LEVEL_ADMIN);
         send_to_char("Sorry, you cannot murder players.\n\r", ch);
         return;
      }
      if (!IS_NPC(victim) && !IS_NPC(ch) && get_trust(victim) >= LEVEL_IMMORTAL)
      {
         sprintf(log_buf, "%s: player attempted immortal murder of %s.", ch->name, victim->name);
         log_string_plus(log_buf, LOG_NORMAL, LEVEL_ADMIN);
         send_to_char("Sorry, you cannot murder immortals.\n\r", ch);
         return;
      }
      check_illegal_pk(ch, victim);
      check_attacker(ch, victim);
      act(AT_SKILL, "You are grabbed and thrown toward $N, this cannot be good!", ch->rider, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n is promply thrown from $s riding position toward $N.  Look out!", ch->rider, NULL, victim, TO_NOTVICT);
      act(AT_SKILL, "$n is being thrown toward you, stupid little bastards!.", ch->rider, NULL, victim, TO_VICT);
      dam = URANGE(-4, (get_curr_str(ch)-13)/2, 4)+5;
      ch->rider->position = POS_STANDING;
      ch->rider->riding = NULL;
      damage(ch->rider, ch->rider, dam, TYPE_UNDEFINED, 0, -1);
      if (!char_died(ch->rider))      
         damage(ch->rider, victim, dam, TYPE_UNDEFINED, 0, -1);
      ch->rider = NULL;
      return;
   }
   do_toss(ch, "");
   return;
}

void do_dismount(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   
   if (ch->riding)
   {
      act(AT_SKILL, "You jump off the back of $N.", ch, NULL, ch->riding, TO_CHAR);
      act(AT_SKILL, "$n skillfully jumps off the back of $N.", ch, NULL, ch->riding, TO_NOTVICT);
      act(AT_SKILL, "$n jumps of your back...about damn time!", ch, NULL, ch->riding, TO_VICT);
      ch->riding->rider = NULL;
      ch->riding = NULL;
      ch->position = POS_STANDING;
      return;   
   }

   if ((victim = ch->mount) == NULL)
   {
      send_to_char("You're not mounted.\n\r", ch);
      return;
   }
   act(AT_SKILL, "You dismount $N.", ch, NULL, victim, TO_CHAR);
   act(AT_SKILL, "$n skillfully dismounts $N.", ch, NULL, victim, TO_NOTVICT);
   act(AT_SKILL, "$n dismounts you.  Whew!", ch, NULL, victim, TO_VICT);
   xREMOVE_BIT(victim->act, ACT_MOUNTED);
   ch->mount = NULL;
   ch->position = POS_STANDING;
   return;
}


/**************************************************************************/


/*
 * Check for parry.
 */
int check_parry(CHAR_DATA * ch, CHAR_DATA *victim)
{
   sh_int mastery;
   sh_int level;
   OBJ_DATA *aweapon;
   OBJ_DATA *vweapon;
   int aparry = 0, vparry = 0;
   int chance;

   mastery = MASTERED(victim, gsn_parry);
   level = POINT_LEVEL(LEARNED(victim, gsn_parry), MASTERED(victim, gsn_parry));

   if (!IS_AWAKE(victim))
      return FALSE;

   if (IS_NPC(victim) && !xIS_SET(victim->defenses, DFND_PARRY))
      return FALSE;
  

   aweapon = get_eq_char(ch, WEAR_WIELD);
   vweapon = get_eq_char(victim, WEAR_WIELD);
   
   if (!vweapon && !IS_NPC(victim))
      return FALSE;

   if (aweapon)
   {
      aparry = aweapon->value[13];
      aparry += (get_curr_str(ch) - 14)/2;
      if (victim->position == POS_BERSERK)
         aparry *= 140/100;
      if (victim->position == POS_AGGRESSIVE)
         aparry *= 120/100;
      if (victim->position == POS_FIGHTING)
         aparry *= 100/100;
      if (victim->position == POS_DEFENSIVE)
         aparry *= 80/100;
      if (victim->position == POS_EVASIVE)
         aparry *= 60/100;
   }
   else if (!aweapon && IS_NPC(ch))
   {
      aparry = URANGE(0, (get_curr_str(ch)-14)*2, 20);
   }
   if (vweapon)
   {
      vparry = vweapon->value[12];
      vparry += (get_curr_dex(victim) - 14)/2;
      if (victim->position == POS_BERSERK)
         vparry *= 60/100;
      if (victim->position == POS_AGGRESSIVE)
         vparry *= 80/100;
      if (victim->position == POS_FIGHTING)
         vparry *= 100/100;
      if (victim->position == POS_DEFENSIVE)
         vparry *= 120/100;
      if (victim->position == POS_EVASIVE)
         vparry *= 140/100;
   }
   else if (!vweapon && IS_NPC(victim))
   {
      vparry = URANGE(0, (get_curr_dex(victim)-14)*4, 40);
   }
   if (!IS_NPC(victim))
   {
      vparry += number_range(level/3, level/4);
   }
   chance = vparry - aparry;
   //bug("v%d a%d t%d", vparry, aparry, chance);
   chance = chance * sysdata.parry_mod / 100;
   if (victim->morph)
      chance += victim->morph->parry;
   chance = URANGE(1, chance, 40);
   if (chance >= number_range(1, 100))
   {
      //slight gain to dex, not as much as dodge...
      if (!IS_NPC(victim))
      {
         int mdex = 1;         
         int dex = victim->perm_dex;
         int bdex = 14 + race_table[victim->race]->dex_plus;
         
         if (sysdata.stat_gain <= 1)
            mdex = number_range(2,3);
         else if (sysdata.stat_gain <= 3)
            mdex = number_range(3,5);
         else if (sysdata.stat_gain >= 5)
            mdex = number_range(6,9);
         
         if (dex == bdex - 4)
            mdex *= 2;
         if (dex == bdex - 3)
            mdex *= 1.7;
         if (dex == bdex - 2)
            mdex *= 1.5;
         if (dex == bdex - 1)
            mdex *= 1.2;
         if (dex == bdex)
            mdex *= 1;
         if (dex == bdex + 1)
            mdex *= .85;
         if (dex == bdex + 2)
            mdex *= .7;
         if (dex == bdex + 3)
            mdex *= .6;
         if (dex == bdex + 4)
            mdex *= .4;
         if (dex == bdex + 5)
            mdex *= .3;
         if (dex == bdex + 6)
            mdex *= .275;
         if (dex == bdex + 7)
            mdex *= .25;
         if (dex == bdex + 8)
            mdex *= .225;
         if (dex > bdex + 8) //Base + 8 should be the max unless you screwed it up
            mdex = 0;
         else
         {
            if (mdex == 0)
               mdex = 1;
         }
            
         if (victim->perm_dex == (14 + race_table[victim->race]->dex_plus + race_table[victim->race]->dex_range + get_talent_increase(victim, 2)) && victim->pcdata->per_dex >= 3000 && mdex > 0)
            mdex = 0;
         victim->pcdata->per_dex += mdex;
         if (victim->pcdata->per_dex > 10000)
         {
            victim->perm_dex++;
            send_to_char("&G***************************************\n\r", victim);
            send_to_char("&G*****You Gain 1 Point of Dexterity*****\n\r", victim);
            send_to_char("&G***************************************\n\r", victim);
            victim->pcdata->per_dex = 0;
         }
      }
      if (victim->position == POS_EVASIVE || (victim->position == POS_DEFENSIVE && mastery >= 3)
      ||  (victim->position == POS_FIGHTING && mastery >= 4) || IS_NPC(victim))
      {
         if (!IS_NPC(victim) && !IS_SET(victim->pcdata->flags, PCFLAG_GAG))
            act(AT_SKILL, "You parry $n's attack.", ch, NULL, victim, TO_VICT);

         if (!IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_GAG)) /* SB */
            act(AT_SKILL, "$N parries your attack.", ch, NULL, victim, TO_CHAR);
         
    //     act(AT_SKILL, "$N parries $n's attack.", ch, NULL, victim, TO_NOTVICT);  
         learn_from_success(victim, gsn_parry, ch);
         return TRUE; //No damage
      }
      else
      {
         if (!IS_NPC(victim) && !IS_SET(victim->pcdata->flags, PCFLAG_GAG))
            act(AT_SKILL, "You partially deflect $n's attack with a parry.", ch, NULL, victim, TO_VICT);

         if (!IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_GAG)) /* SB */
            act(AT_SKILL, "$N partially deflect your attack with a parry.", ch, NULL, victim, TO_CHAR);
         
   //      act(AT_SKILL, "$N partially deflect $n's attack with a parry.", ch, NULL, victim, TO_NOTVICT);  
         learn_from_success(victim, gsn_parry, ch);
      }
      
      if (victim->position == POS_EVASIVE)
         return TRUE; //No damage
      if (victim->position == POS_DEFENSIVE)
      {
         if (mastery == 1)
            return 10; // 10 percent damage
         else if (mastery == 2)
            return 5;
         else
            return TRUE;
      }
      if (victim->position == POS_FIGHTING)
      {
         if (mastery == 1)
            return 20;
         else if (mastery == 2)
            return 15;
         else if (mastery == 3)
            return 10;
         else
            return TRUE;
      }
      if (victim->position == POS_AGGRESSIVE)
      {
         if (mastery == 1)
            return 30;
         else if (mastery == 2)
            return 25;
         else if (mastery == 3)
            return 20;
         else
            return 15;
      }
      if (victim->position == POS_BERSERK)
      {
         if (mastery == 1)
            return 50;
         else if (mastery == 2)
            return 45;
         else if (mastery == 3)
            return 40;
         else
            return 35;
      }    
   }
   else
      return FALSE;
      
   return FALSE;
}



/*
 * Check for dodge.
 */
bool check_dodge(CHAR_DATA * ch, CHAR_DATA * victim, int limb)
{
   int percent;
   int mdex;
   int bdex;
   int dex = victim->perm_dex;
   int empty = 0;
   sh_int mastery, level;
   OBJ_DATA *armor;
   int limbadd = 0;
   int hobbit;
   int tlevel;
   int diff;

   mastery = MASTERED(victim, gsn_dodge);
   level = POINT_LEVEL(LEARNED(victim, gsn_dodge), MASTERED(victim, gsn_dodge));  
   tlevel = POINT_LEVEL(LEARNED(victim, gsn_tumble), MASTERED(victim, gsn_tumble));  

   if (!IS_AWAKE(victim))
      return FALSE;

   if (IS_NPC(victim))
   {
      int chance = 0;
      
      if (!xIS_SET(victim->defenses, DFND_DODGE))
         return FALSE;
      
      chance = URANGE(10, 15 + ((get_curr_dex(victim) - 14)*3), 35);
      if (number_range(1, 100) >= chance)
         return FALSE;
      else
      {
         if (!IS_NPC(victim) && !IS_SET(victim->pcdata->flags, PCFLAG_GAG))
            act(AT_SKILL, "You dodge $n's attack.", ch, NULL, victim, TO_VICT);

         if (!IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_GAG))
            act(AT_SKILL, "$N dodges your attack.", ch, NULL, victim, TO_CHAR);
      
         act(AT_SKILL, "$N dodges $n's attack.", ch, NULL, victim, TO_NOTVICT);  
         return TRUE;
      }
   }
   if (!IS_NPC(victim) && victim->pcdata->learned[gsn_dodge] == 0)
      level = 0;
      
   if (limb == LM_HEAD)
      limbadd = 10;
   else if (limb == LM_NECK)
      limbadd = 15;
   else if (limb == LM_RARM || limb == LM_LARM)
      limbadd = 7;
   else if (limb == LM_RLEG || limb == LM_LLEG)
      limbadd = 3;
   else if (limb == LM_BODY)
      limbadd = -5;
      
   percent = 10 + URANGE(-12, (get_curr_dex(victim) - 15) * 2, 14) + URANGE(-6, get_curr_lck(victim) - 14, 7);
   percent += limbadd;
   diff = percent;
   
   if (victim->race == RACE_HOBBIT)
      hobbit = 2;
   else
      hobbit = 0;

   if ((armor = get_eq_char(victim, WEAR_HEAD)) == NULL)
   {
      empty++;
      percent += 3;
   }
   else
   {
      if (armor->value[5] == 0)
      {
         if (armor->pIndexData->value[5] == 0)
         {
            bug("%s on %s has an invalid v5 for armorsize", armor->name, victim->name);
            send_to_char("There is some problem with your armor, tell an immortal.\n\r", victim);
         }
         else
            armor->value[5] = armor->pIndexData->value[5];
      }
      if (armor->value[5] == ASIZE_LEATHER)
         percent += 1+hobbit;
      else if (armor->value[5] == ASIZE_LIGHT)
         percent -= 1-hobbit;
      else if (armor->value[5] == ASIZE_MEDIUM)
         percent -= 2;
      else if (armor->value[5] == ASIZE_HEAVY)
         percent -= 4;
      else if (armor->value[5] == ASIZE_HEAVIEST)
         percent -= 7;
   }
   if ((armor = get_eq_char(victim, WEAR_NECK)) == NULL)
   {
      empty++;
      percent += 3;
   }
   else
   {
      if (armor->value[5] == 0)
      {
         if (armor->pIndexData->value[5] == 0)
         {
            bug("%s on %s has an invalid v5 for armorsize", armor->name, victim->name);
            send_to_char("There is some problem with your armor, tell an immortal.\n\r", victim);
         }
         else
            armor->value[5] = armor->pIndexData->value[5];
      }
      if (armor->value[5] == ASIZE_LEATHER)
         percent += 1+hobbit;
      else if (armor->value[5] == ASIZE_LIGHT)
         percent -= 1-hobbit;
      else if (armor->value[5] == ASIZE_MEDIUM)
         percent -= 2;
      else if (armor->value[5] == ASIZE_HEAVY)
         percent -= 4;
      else if (armor->value[5] == ASIZE_HEAVIEST)
         percent -= 6;
   }
   if ((armor = get_eq_char(victim, WEAR_ARM_R)) == NULL)
   {
      empty++;
      percent += 3;
   }
   else
   {
      if (armor->value[5] == 0)
      {
         if (armor->pIndexData->value[5] == 0)
         {
            bug("%s on %s has an invalid v5 for armorsize", armor->name, victim->name);
            send_to_char("There is some problem with your armor, tell an immortal.\n\r", victim);
         }
         else
            armor->value[5] = armor->pIndexData->value[5];
      }
      if (armor->value[5] == ASIZE_LEATHER)
         percent += 1+hobbit;
      else if (armor->value[5] == ASIZE_LIGHT)
         percent -= 1-hobbit;
      else if (armor->value[5] == ASIZE_MEDIUM)
         percent -= 3;
      else if (armor->value[5] == ASIZE_HEAVY)
         percent -= 5;
      else if (armor->value[5] == ASIZE_HEAVIEST)
         percent -= 8;
   }
   if ((armor = get_eq_char(victim, WEAR_ARM_L)) == NULL)
   {
      empty++;
      percent += 3;
   }
   else
   {
      if (armor->value[5] == 0)
      {
         if (armor->pIndexData->value[5] == 0)
         {
            bug("%s on %s has an invalid v5 for armorsize", armor->name, victim->name);
            send_to_char("There is some problem with your armor, tell an immortal.\n\r", victim);
         }
         else
            armor->value[5] = armor->pIndexData->value[5];
      }
      if (armor->value[5] == ASIZE_LEATHER)
         percent += 1+hobbit;
      else if (armor->value[5] == ASIZE_LIGHT)
         percent -= 1-hobbit;
      else if (armor->value[5] == ASIZE_MEDIUM)
         percent -= 3;
      else if (armor->value[5] == ASIZE_HEAVY)
         percent -= 5;
      else if (armor->value[5] == ASIZE_HEAVIEST)
         percent -= 8;
   }
   if ((armor = get_eq_char(victim, WEAR_LEG_R)) == NULL)
   {
      empty++;
      percent += 3;
   }
   else
   {
      if (armor->value[5] == 0)
      {
         if (armor->pIndexData->value[5] == 0)
         {
            bug("%s on %s has an invalid v5 for armorsize", armor->name, victim->name);
            send_to_char("There is some problem with your armor, tell an immortal.\n\r", victim);
         }
         else
            armor->value[5] = armor->pIndexData->value[5];
      }
      if (armor->value[5] == ASIZE_LEATHER)
         percent += 1+hobbit;
      else if (armor->value[5] == ASIZE_LIGHT)
         percent -= 1-hobbit;
      else if (armor->value[5] == ASIZE_MEDIUM)
         percent -= 3;
      else if (armor->value[5] == ASIZE_HEAVY)
         percent -= 6;
      else if (armor->value[5] == ASIZE_HEAVIEST)
         percent -= 9;
   }
   if ((armor = get_eq_char(victim, WEAR_LEG_L)) == NULL)
   {
      empty++;
      percent += 3;
   }
   else
   {
      if (armor->value[5] == 0)
      {
         if (armor->pIndexData->value[5] == 0)
         {
            bug("%s on %s has an invalid v5 for armorsize", armor->name, victim->name);
            send_to_char("There is some problem with your armor, tell an immortal.\n\r", victim);
         }
         else
            armor->value[5] = armor->pIndexData->value[5];
      }
      if (armor->value[5] == ASIZE_LEATHER)
         percent += 1+hobbit;
      else if (armor->value[5] == ASIZE_LIGHT)
         percent -= 1-hobbit;
      else if (armor->value[5] == ASIZE_MEDIUM)
         percent -= 3;
      else if (armor->value[5] == ASIZE_HEAVY)
         percent -= 6;
      else if (armor->value[5] == ASIZE_HEAVIEST)
         percent -= 9;
   }
   if ((armor = get_eq_char(victim, WEAR_BODY)) == NULL)
   {
      empty++;
      percent += 5;
   }
   else
   {
      if (hobbit > 0)
         hobbit+=1;
      if (armor->value[5] == 0)
      {
         if (armor->pIndexData->value[5] == 0)
         {
            bug("%s on %s has an invalid v5 for armorsize", armor->name, victim->name);
            send_to_char("There is some problem with your armor, tell an immortal.\n\r", victim);
         }
         else
            armor->value[5] = armor->pIndexData->value[5];
      }
      if (armor->value[5] == ASIZE_LEATHER)
         percent += 2+hobbit;
      else if (armor->value[5] == ASIZE_LIGHT)
         percent -= 1-hobbit;
      else if (armor->value[5] == ASIZE_MEDIUM)
         percent -= 4;
      else if (armor->value[5] == ASIZE_HEAVY)
         percent -= 8;
      else if (armor->value[5] == ASIZE_HEAVIEST)
         percent -= 12;
   }
   diff = percent - diff;
   if (diff > 0 && tlevel > 0)
   {
      percent += 1 + number_range(tlevel/5, tlevel/4);
   }
   if (!IS_NPC(ch))
   {
      percent += race_table[ch->race]->dodge_bonus;
   }
   percent = percent * sysdata.dodge_mod / 100;
   if (victim->morph)
      percent += victim->morph->dodge;
   percent += number_range(level/4, level/3);
   percent = URANGE(1, percent, 50);
   
   if ((armor = get_eq_char(victim, WEAR_SHIELD)) != NULL)
   {
      percent = percent - UMAX(0, (armor->value[2] - 15));
      percent = URANGE(1, percent, 75);
   }
   if (!IS_NPC(victim))
   {
      if (percent <= 3)
         mdex = number_range(-2, -1);
      else if (percent <= 7)
         mdex = number_range(-1, 0);
      else if (percent <= 15)
         mdex = number_range(-1, 2);
      else if (percent <= 25)
         mdex = number_range(1, 2);
      else if (percent <= 40)
         mdex = number_range(2, 3);
      else if (percent <= 60)
         mdex = number_range(3, 5);
      else if (percent <= 80)
         mdex = number_range(5, 7);
      else
         mdex = number_range(7, 9);
         
      if (sysdata.stat_gain <= 3 && mdex > 0)
         mdex = number_range(150*mdex/100, 180*mdex/100);
      else if (sysdata.stat_gain >= 5 && mdex > 0)
         mdex = number_range(250*mdex/100, 300*mdex/100);
      
   
      //dex mods
      if (mdex > 0)
      {
         bdex = 14 + race_table[victim->race]->dex_plus;
         if (dex == bdex - 4)
            mdex *= 2;
         if (dex == bdex - 3)
            mdex *= 1.7;
         if (dex == bdex - 2)
            mdex *= 1.5;
         if (dex == bdex - 1)
            mdex *= 1.2;
         if (dex == bdex)
            mdex *= 1;
         if (dex == bdex + 1)
            mdex *= .85;
         if (dex == bdex + 2)
            mdex *= .7;
         if (dex == bdex + 3)
            mdex *= .6;
         if (dex == bdex + 4)
            mdex *= .4;
         if (dex == bdex + 5)
            mdex *= .3;
         if (dex == bdex + 6)
            mdex *= .275;
         if (dex == bdex + 7)
            mdex *= .25;
         if (dex == bdex + 8)
            mdex *= .225;
         if (dex > bdex + 8) //Base + 8 should be the max unless you screwed it up
            mdex = 0;
         else
         {
            if (mdex == 0)
               mdex = 1;
         }
      }
      else
      {
         bdex = 14 + race_table[victim->race]->dex_plus;
         if (dex == bdex - 4)
            mdex = 0;
         if (dex == bdex - 3)
            mdex *= .3;
         if (dex == bdex - 2)
            mdex *= .4;
         if (dex == bdex - 1)
            mdex *= .6;
         if (dex == bdex)
            mdex *= 1;
         if (dex == bdex + 1)
            mdex *= 1.2;
         if (dex == bdex + 2)
            mdex *= 1.4;
         if (dex == bdex + 3)
            mdex *= 1.6;
         if (dex == bdex + 4)
            mdex *= 1.8;
         if (dex == bdex + 5)
            mdex *= 2;
      }
      if (victim->perm_dex == (14 + race_table[victim->race]->dex_plus + race_table[victim->race]->dex_range + get_talent_increase(victim, 2)) && victim->pcdata->per_dex >= 3000 && mdex > 0)
         mdex = 0;
      if (victim->perm_dex == (14 + race_table[victim->race]->dex_plus - 5 + race_table[victim->race]->dex_range) && victim->pcdata->per_dex <= 3000 && mdex < 0)
         mdex = 0;
      victim->pcdata->per_dex += mdex;
      if (victim->pcdata->per_dex > 10000)
      {
         victim->perm_dex++;
         send_to_char("&G***************************************\n\r", victim);
         send_to_char("&G*****You Gain 1 Point of Dexterity*****\n\r", victim);
         send_to_char("&G***************************************\n\r", victim);
         victim->pcdata->per_dex = 0;
      }
      if (victim->pcdata->per_dex < 0)
      {
         victim->perm_dex--;
         send_to_char("&g***************************************\n\r", victim);
         send_to_char("&g*****You Lose 1 Point of Dexterity*****\n\r", victim);
         send_to_char("&g***************************************\n\r", victim);
         victim->pcdata->per_dex = 9999;
      }
   }
   if (xIS_SET(victim->act, PLR_DODGE))
      return FALSE;
   if (number_range(1, 100) > percent)
      return FALSE;

   if (diff > 0 && tlevel > 0)
   {
      if (!IS_NPC(victim) && !IS_SET(victim->pcdata->flags, PCFLAG_GAG))
         act(AT_SKILL, "You tumble out of the way of $n's attack.", ch, NULL, victim, TO_VICT);

      if (!IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_GAG))
         act(AT_SKILL, "$N tumbles away from your attack.", ch, NULL, victim, TO_CHAR);
      
   //      act(AT_SKILL, "$N tumbles away from $n's attack.", ch, NULL, victim, TO_NOTVICT);
         
      learn_from_success(victim, gsn_tumble, ch);
   }  
   else
   { 
      if (!IS_NPC(victim) && !IS_SET(victim->pcdata->flags, PCFLAG_GAG))
         act(AT_SKILL, "You dodge $n's attack.", ch, NULL, victim, TO_VICT);

      if (!IS_NPC(ch) && !IS_SET(ch->pcdata->flags, PCFLAG_GAG))
         act(AT_SKILL, "$N dodges your attack.", ch, NULL, victim, TO_CHAR);   
   //   act(AT_SKILL, "$N dodges $n's attack.", ch, NULL, victim, TO_NOTVICT);
   }

   learn_from_success(victim, gsn_dodge, ch);
   return TRUE;
}

void do_poison_weapon(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj;
   OBJ_DATA *pobj;
   OBJ_DATA *wobj;
   char arg[MIL];
   int percent;
   sh_int mastery, level;

   mastery = MASTERED(ch, gsn_poison_weapon) * 20;
   mastery = mastery - 70;
   level = POINT_LEVEL(LEARNED(ch, gsn_poison_weapon), MASTERED(ch, gsn_poison_weapon));

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_poison_weapon] <= 0)
   {
      send_to_char("What do you think you are, a talented thief?\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("What are you trying to poison?\n\r", ch);
      return;
   }
   if (ch->fighting)
   {
      send_to_char("While you're fighting?  Nice try.\n\r", ch);
      return;
   }
   if (ms_find_obj(ch))
      return;

   if (!(obj = get_obj_carry(ch, arg)))
   {
      send_to_char("You do not have that weapon.\n\r", ch);
      return;
   }
   if (obj->item_type != ITEM_WEAPON)
   {
      send_to_char("That item is not a weapon.\n\r", ch);
      return;
   }
   if (IS_OBJ_STAT(obj, ITEM_POISONED))
   {
      send_to_char("That weapon is already poisoned.\n\r", ch);
      return;
   }
   if (IS_OBJ_STAT(obj, ITEM_CLANOBJECT))
   {
      send_to_char("It doesn't appear to be fashioned of a poisonable material.\n\r", ch);
      return;
   }
   /* Now we have a valid weapon...check to see if we have the powder. */
   for (pobj = ch->first_carrying; pobj; pobj = pobj->next_content)
   {
      if (pobj->pIndexData->vnum == OBJ_VNUM_BLACK_POWDER)
         break;
   }
   if (!pobj)
   {
      send_to_char("You do not have the black poison powder.\n\r", ch);
      return;
   }
   /* Okay, we have the powder...do we have water? */
   for (wobj = ch->first_carrying; wobj; wobj = wobj->next_content)
   {
      if (wobj->item_type == ITEM_DRINK_CON && wobj->value[1] > 0 && wobj->value[2] == 0)
         break;
   }
   if (!wobj)
   {
      send_to_char("You have no water to mix with the powder.\n\r", ch);
      return;
   }
   if (!ch->fighting)
      WAIT_STATE(ch, skill_table[gsn_poison_weapon]->beats*2);
   else
      ch->fight_timer = get_btimer(ch, gsn_poison_weapon, NULL);

   percent = (number_percent() - ((get_curr_wis(ch) - 15)*5) - ((get_curr_dex(ch) - 15)*5) - ((get_curr_lck(ch) - 14)*5) - mastery);

   /* Check the skill percentage */
   separate_obj(pobj);
   separate_obj(wobj);
   if (!can_use_skill(ch, percent, gsn_poison_weapon))
   {
      set_char_color(AT_RED, ch);
      send_to_char("You failed and spill some on yourself.  Ouch!\n\r", ch);
      set_char_color(AT_GREY, ch);
      damage(ch, ch, 3+level/2, gsn_poison_weapon, 0, -1);
      if (number_range(1, 5) == 1) //20 percent
      {
         AFFECT_DATA af;

         af.type = gsn_poison;
         af.duration = 60;
         af.location = APPLY_STR;
         af.modifier = -1;
         af.bitvector = meb(AFF_POISON);
         affect_join(ch, &af);
         ch->mental_state = URANGE(20, (ch->mental_state + 2), 100);
         send_to_char("You feel your skin start to swell, you have poisoned yourself!!!!!\n\r", ch);
      }
      act(AT_RED, "$n spills the poison all over!", ch, NULL, NULL, TO_ROOM);
      extract_obj(pobj);
      extract_obj(wobj);
      learn_from_failure(ch, gsn_poison_weapon, NULL);
      return;
   }
   separate_obj(obj);
   /* Well, I'm tired of waiting.  Are you? */
   act(AT_RED, "You mix $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_CHAR);
   act(AT_RED, "$n mixes $p in $P, creating a deadly poison!", ch, pobj, wobj, TO_ROOM);
   act(AT_GREEN, "You pour the poison over $p, which glistens wickedly!", ch, obj, NULL, TO_CHAR);
   act(AT_GREEN, "$n pours the poison over $p, which glistens wickedly!", ch, obj, NULL, TO_ROOM);
   xSET_BIT(obj->extra_flags, ITEM_POISONED);
   obj->cost *= 2;
   /* Set an object timer.  Don't want proliferation of poisoned weapons */
   obj->timer = 5+level*2;

   if (IS_OBJ_STAT(obj, ITEM_GLOW))
      obj->timer *= 1.3;

   if (IS_OBJ_STAT(obj, ITEM_MAGIC))
      obj->timer *= 1.5;

   /* WHAT?  All of that, just for that one bit?  How lame. ;) */
   act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_CHAR);
   act(AT_BLUE, "The remainder of the poison eats through $p.", ch, wobj, NULL, TO_ROOM);
   extract_obj(pobj);
   extract_obj(wobj);
   learn_from_success(ch, gsn_poison_weapon, NULL);
   return;
}

void do_manatap(CHAR_DATA *ch, char *argument)
{
   int level;
   int mana;
   int maxgain;
  
   level = POINT_LEVEL(LEARNED(ch, gsn_manatap), MASTERED(ch, gsn_manatap));
   maxgain = UMAX(ch->max_mana * level / 120, level*35/10);
   
   if (IS_NPC(ch))
      return;
   
   if (ch->position != POS_STANDING && ch->position != POS_SITTING && ch->position != POS_RESTING && ch->position != POS_MOUNTED)
   {
      send_to_char("You can only use this while standing, sitting, mounted, or resting.\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_manatap] <= 0)
   {
      send_to_char("A skill such as this requires more magical ability than that of your skills.\n\r", ch);
      return;
   }
   if (ch->mana < ch->max_mana)
   {
      send_to_char("You can only tap mana when your mana is full.\n\r", ch);
      return;
   }
   if (ch->mana >= ch->max_mana + maxgain)
   {
      send_to_char("Your mana is maxed out, you cannot tap any more.\n\r", ch);
      return;
   }
   if (number_range(1, 100) <= (40+level/2))
   {
      act(AT_MAGIC, "You tap your surroundings and gain additional mana.", ch, NULL, NULL, TO_CHAR);
      act(AT_MAGIC, "$n taps the surroundings area and gains additional mana.", ch, NULL, NULL, TO_CANSEE);
      mana = UMAX(5, level/2);
      if (ch->mana + mana >= (ch->max_mana + maxgain))
         mana = ch->max_mana + maxgain - ch->mana;
      ch->mana+=mana;
      learn_from_success(ch, gsn_manatap, NULL);
   }
   else
   {
      act(AT_MAGIC, "You attempt to tap your surroundings for additional mana but fail.", ch, NULL, NULL, TO_CHAR);
      act(AT_MAGIC, "$n attempts to tap the surrounding area for additional mana but fails.", ch, NULL, NULL, TO_CANSEE);
      learn_from_failure(ch, gsn_manatap, NULL);
   }
   WAIT_STATE(ch, skill_table[gsn_manatap]->beats*2);
}
   
void do_scribe(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *scroll;
   int sn;
   char buf1[MSL];
   char buf2[MSL];
   char buf3[MSL];
   int mana;
   sh_int mastery;
   int fvnum;
   int points;
   int chance;
   int strength;
   
   points = POINT_LEVEL(LEARNED(ch, gsn_scribe), MASTERED(ch, gsn_scribe));

   mastery = MASTERED(ch, gsn_scribe) * 25;
   mastery = mastery - 100;

   if (IS_NPC(ch))
      return;

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_scribe] <= 0)
   {
      send_to_char("A skill such as this requires more magical ability than that of your skills.\n\r", ch);
      return;
   }

   if (argument[0] == '\0' || !str_cmp(argument, ""))
   {
      send_to_char("Scribe what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if ((sn = find_spell(ch, argument, TRUE)) < 0)
   {
      send_to_char("You have not learned that spell.\n\r", ch);
      return;
   }

   if (skill_table[sn]->spell_fun == spell_null)
   {
      send_to_char("That's not a spell!\n\r", ch);
      return;
   }

   if (SPELL_FLAG(skill_table[sn], SF_NOSCRIBE))
   {
      send_to_char("You cannot scribe that spell.\n\r", ch);
      return;
   }

   mana = IS_NPC(ch) ? 0 : skill_table[sn]->min_mana;

   if (MASTERED(ch, gsn_scribe) == 6)
      mana *= 3;
   else if (MASTERED(ch, gsn_scribe) == 5)
      mana = mana * 7 / 2;
   else if (MASTERED(ch, gsn_scribe) == 4)
      mana *= 4;
   else
      mana *= 5;

   if (!IS_NPC(ch) && ch->mana < mana)
   {
      send_to_char("You don't have enough mana.\n\r", ch);
      return;
   }
   
   if (MASTERED(ch, sn) < 4)
   {
      send_to_char("You can only scribe a spell that you have mastered.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 5 && MASTERED(ch, gsn_scribe) < 6)
   {
      send_to_char("You can only scribe a Tier 5 spell if you have a mastery of flawless in scribe.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 4 && MASTERED(ch, gsn_scribe) < 5)
   {
      send_to_char("You can only scribe a Tier 4 spell if you have a mastery of elite in scribe.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 3 && MASTERED(ch, gsn_scribe) < 4)
   {
      send_to_char("You can only scribe a Tier 3 spell if you have mastered scribe.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 2 && MASTERED(ch, gsn_scribe) < 3)
   {
      send_to_char("You can only scribe a Tier 2 spell if you at least an expert scribe.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 3 || skill_table[sn]->masterydiff[0] == 4 || skill_table[sn]->masterydiff[0] == 5)
      fvnum = OBJ_VNUM_SCROLL_SCRIBING_TIER3;
   else if (skill_table[sn]->masterydiff[0] == 2)
      fvnum = OBJ_VNUM_SCROLL_SCRIBING_TIER2;
   else
      fvnum = OBJ_VNUM_SCROLL_SCRIBING;
   for (scroll = ch->first_carrying; scroll; scroll = scroll->next_content)
   {
      if (scroll->pIndexData->vnum == fvnum && scroll->value[1] == -1)
         break;
   }

   if (!scroll)
   {
      send_to_char("You must have a blank scroll in your inventory to scribe it.\n\r", ch);
      return;
   }
   if ((scroll->value[1] != -1) && (scroll->pIndexData->vnum == fvnum))
   {
      send_to_char("That scroll has already been inscribed.\n\r", ch);
      return;
   }

   if (!process_spell_components(ch, sn))
   {
      learn_from_failure(ch, gsn_scribe, NULL);
      ch->mana -= (mana / 2);
      gain_mana_per(ch, NULL, mana/2);
      return;
   }
   
   chance = 15 + (points * 123 / 100);
   chance = URANGE(15, chance, 95);

   if (number_range(1, 100) > chance)
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("You failed.\n\r", ch);
      learn_from_failure(ch, gsn_scribe, NULL);
      ch->mana -= (mana / 2);
      gain_mana_per(ch, NULL, mana/2);
      return;
   }
   
   strength = SPOWER_MIN + (points/10) - ((skill_table[sn]->masterydiff[0]*2)-2);
   strength = URANGE(SPOWER_MIN, strength, SPOWER_GREATEST);
   chance = 1+ (points/10) + UMIN(5, get_curr_lck(ch)-14);
   if (number_range(1, 100) <= chance)
   {
      strength = SPOWER_GREATEST;
      send_to_char("&w&WYour magical blessing is so pure that you create a very powerful scroll!\n\r", ch);
   }
   scroll->value[5] = strength;

   scroll->value[1] = sn;
   scroll->value[0] = ch->level;
   sprintf(buf1, "%s scroll", skill_table[sn]->name);
   STRFREE(scroll->short_descr);
   scroll->short_descr = STRALLOC(aoran(buf1));

   sprintf(buf2, "A glowing scroll inscribed '%s' lies in the dust.", skill_table[sn]->name);

   STRFREE(scroll->description);
   scroll->description = STRALLOC(buf2);

   sprintf(buf3, "scroll scribing %s", skill_table[sn]->name);
   STRFREE(scroll->name);
   scroll->name = STRALLOC(buf3);

   act(AT_MAGIC, "$n magically scribes $p.", ch, scroll, NULL, TO_ROOM);
   act(AT_MAGIC, "You magically scribe $p.", ch, scroll, NULL, TO_CHAR);

   learn_from_success(ch, gsn_scribe, NULL);

   ch->mana -= mana;
   gain_mana_per(ch, NULL, mana);

}

void do_brew(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *potion;
   OBJ_DATA *fire;
   int sn;
   char buf1[MSL];
   char buf2[MSL];
   char buf3[MSL];
   int mana;
   sh_int mastery;
   bool found;
   int fvnum;
   int points;
   int chance;
   int strength;
   
   points = POINT_LEVEL(LEARNED(ch, gsn_brew), MASTERED(ch, gsn_brew));


   if (IS_NPC(ch))
      return;

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_brew] <= 0)
   {
      send_to_char("A skill such as this requires more magical ability than that of your abilities.\n\r", ch);
      return;
   }

   if (argument[0] == '\0' || !str_cmp(argument, ""))
   {
      send_to_char("Brew what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if ((sn = find_spell(ch, argument, TRUE)) < 0)
   {
      send_to_char("You have not learned that spell.\n\r", ch);
      return;
   }

   if (skill_table[sn]->spell_fun == spell_null)
   {
      send_to_char("That's not a spell!\n\r", ch);
      return;
   }

   if (SPELL_FLAG(skill_table[sn], SF_NOBREW))
   {
      send_to_char("You cannot brew that spell.\n\r", ch);
      return;
   }

   mana = IS_NPC(ch) ? 0 : skill_table[sn]->min_mana;

   mastery = MASTERED(ch, gsn_brew);

   if (mastery == 6)      
      mana *= 2;
   if (mastery == 5)      
      mana = mana * 5 / 2;
   if (mastery == 4)      
      mana *= 3;
   else
      mana *= 4;

   if (!IS_NPC(ch) && ch->mana < mana)
   {
      send_to_char("You don't have enough mana.\n\r", ch);
      return;
   }

   found = FALSE;

   for (fire = ch->in_room->first_content; fire; fire = fire->next_content)
   {
      if (fire->item_type == ITEM_FIRE)
      {
         found = TRUE;
         break;
      }
   }
   /* Masters don't need a fire */
   if (!found && mastery != 4)
   {
      send_to_char("There must be a fire in the room to brew a potion.\n\r", ch);
      return;
   }
   
   if (MASTERED(ch, sn) < 4)
   {
      send_to_char("You can only brew a spell that you have mastered.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 5 && MASTERED(ch, gsn_brew) < 6)
   {
      send_to_char("You can only brew a Tier 3 spell if you have mastered brew.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 4 && MASTERED(ch, gsn_brew) < 5)
   {
      send_to_char("You can only brew a Tier 3 spell if you have mastered brew.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 3 && MASTERED(ch, gsn_brew) < 4)
   {
      send_to_char("You can only brew a Tier 3 spell if you have mastered brew.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 2 && MASTERED(ch, gsn_brew) < 3)
   {
      send_to_char("You can only brew a Tier 2 spell if you at least an expert brewer.\n\r", ch);
      return;
   }
   if (skill_table[sn]->masterydiff[0] == 3 || skill_table[sn]->masterydiff[0] == 4 || skill_table[sn]->masterydiff[0] == 5)
      fvnum = OBJ_VNUM_FLASK_BREWING_TIER3;
   else if (skill_table[sn]->masterydiff[0] == 2)
      fvnum = OBJ_VNUM_FLASK_BREWING_TIER2;
   else
      fvnum = OBJ_VNUM_FLASK_BREWING;
      
   for (potion = ch->first_carrying; potion; potion = potion->next_content)
   {
      if (potion->pIndexData->vnum == fvnum && potion->value[1] == -1)
         break;
   }
   if (!potion)
   {
      send_to_char("You must have an empty flask in your inventory to brew a potion.\n\r", ch);
      return;
   }
   mastery = mastery * 30;
   mastery = mastery - 120;
   chance = 15 + (points * 123 / 100);
   chance = URANGE(15, chance, 95);

   if ((potion->value[1] != -1) && (potion->pIndexData->vnum == fvnum))
   {
      send_to_char("That's not an empty flask.\n\r", ch);
      return;
   }

   if (!process_spell_components(ch, sn))
   {
      learn_from_failure(ch, gsn_brew, NULL);
      ch->mana -= (mana / 2);
      gain_mana_per(ch, NULL, mana/2);
      return;
   }

   if (number_range(1, 100) > chance)
   {
      set_char_color(AT_MAGIC, ch);
      send_to_char("You failed.\n\r", ch);
      learn_from_failure(ch, gsn_brew, NULL);
      ch->mana -= (mana / 2);
      gain_mana_per(ch, NULL, mana/2);
      return;
   }
   strength = SPOWER_MIN + (points/10) - ((skill_table[sn]->masterydiff[0]*2)-2);
   strength = URANGE(SPOWER_MIN, strength, SPOWER_GREATEST);
   chance = 1+ (points/10) + UMIN(5, get_curr_lck(ch)-14);
   if (number_range(1, 100) <= chance)
   {
      strength = SPOWER_GREATEST;
      send_to_char("&w&WYour magical blessing is so pure that you create a very powerful potion!\n\r", ch);
   }
   potion->value[5] = strength;
   potion->value[1] = sn;
   potion->value[0] = 0;
   sprintf(buf1, "%s potion", skill_table[sn]->name);
   STRFREE(potion->short_descr);
   potion->short_descr = STRALLOC(aoran(buf1));

   sprintf(buf2, "A strange potion labelled '%s' sizzles in a glass flask.", skill_table[sn]->name);

   STRFREE(potion->description);
   potion->description = STRALLOC(buf2);

   sprintf(buf3, "flask potion %s", skill_table[sn]->name);
   STRFREE(potion->name);
   potion->name = STRALLOC(buf3);

   act(AT_MAGIC, "$n brews up $p.", ch, potion, NULL, TO_ROOM);
   act(AT_MAGIC, "You brew up $p.", ch, potion, NULL, TO_CHAR);

   learn_from_success(ch, gsn_brew, NULL);

   ch->mana -= mana;
   gain_mana_per(ch, NULL, mana);

}

bool check_grip(CHAR_DATA * ch, CHAR_DATA * victim)
{
   int chance = 0;
   int percent = 0;
   sh_int mastery, level;

   mastery = MASTERED(victim, gsn_grip);
   level = POINT_LEVEL(LEARNED(victim, gsn_grip), MASTERED(victim, gsn_grip));

   if (!IS_AWAKE(victim))
      return FALSE;

   if (IS_NPC(victim) && !xIS_SET(victim->defenses, DFND_GRIP))
      return FALSE;

   if (IS_NPC(victim))
      chance = URANGE(5, level, 50);
   else
   {
      percent = 10+level*1.5;
      chance = (int) (percent / 3 * 2);
   }

   /* Consider luck+str as a factor */
   chance +=  get_curr_lck(victim) - 14;
   chance +=  get_curr_str(victim) - get_curr_str(ch);
   chance = URANGE(5, chance, 85);
   if (number_range(1, 100) >= chance)
   {
      learn_from_failure(victim, gsn_grip, ch);
      return FALSE;
   }
   act(AT_SKILL, "You evade $n's attempt to disarm you.", ch, NULL, victim, TO_VICT);
   act(AT_SKILL, "$N holds $S weapon strongly, and is not disarmed.", ch, NULL, victim, TO_CHAR);
   learn_from_success(victim, gsn_grip, ch);
   return TRUE;
}
//Sort of like shove but a bit more nasty
void do_drive(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   EXIT_DATA *pexit = NULL;
   int dir;
   int level;
   int nomove = 0;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Attempt to drive whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Why would you want to do that to yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;
   
   if (wielding_skill_weapon(ch, 0) != 3 || !get_eq_char(ch, WEAR_WIELD))
   {
      send_to_char("You have to wielding a polearm to make use of this command..\n\r", ch);
      return;
   }
   if ((dir = get_truedir(argument)) == -1)
   {
      send_to_char("That is not a valid direction!\n\r", ch);
      return;
   }
   if (!IN_WILDERNESS(ch))
   {
      ROOM_INDEX_DATA *to_room;
      if ((pexit = get_exit(ch->in_room, dir)) == NULL)
      {
         send_to_char("There's no exit in that direction.\n\r", ch);
         return;
      }
      else if (IS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(victim, AFF_PASS_DOOR) || IS_SET(pexit->exit_info, EX_NOPASSDOOR)))
      {
         send_to_char("There's no exit in that direction.\n\r", ch);
         return;
      }
      to_room = pexit->to_room;
      if (xIS_SET(to_room->room_flags, ROOM_DEATH))
      {
         send_to_char("You cannot drive someone into a deathtrap!.\n\r", ch);
         return;
      }
   }
   level = POINT_LEVEL(LEARNED(ch, gsn_drive), MASTERED(ch, gsn_drive))/2;
   level += 35;
   if (ch->position == POS_MOUNTED)
      level +=15;
   if (victim->position == POS_MOUNTED)
      level -=10;
   level += (get_curr_str(ch) - get_curr_str(victim))*2;
   check_attacker(ch, victim);
   if (number_range(1, 100) <= level)
   {
      learn_from_success(ch, gsn_drive, victim);
      global_retcode = one_hit(ch, victim, gsn_drive, LM_BODY);
      if (char_died(victim))
         return;
      
      if (victim->position == POS_MOUNTED)
      {
         if (victim->mount)
         {
            xREMOVE_BIT(victim->mount->act, ACT_MOUNTED);
            victim->mount = NULL;
         }
      }
      victim->position = POS_STANDING;
      if (IN_WILDERNESS(ch))
      {
         process_movement_value(victim, dir);
         if (IN_SAME_ROOM(ch, victim))
            nomove = 1;
      }  
  
      if (!IN_WILDERNESS(ch))
      {
         if ((move_char(victim, get_exit(ch->in_room, dir), 0)) == rSTOP)
            nomove = 1;
      }
      if (!nomove)
      {
         act(AT_RED, "You drive into $N with your weapon and roll $M into another room.", ch, NULL, victim, TO_CHAR);
         act(AT_RED, "$n drives into you with $s weapon and rolls you into another room.", ch, NULL, victim, TO_VICT);
         act(AT_RED, "$n drives into $N and rolls him into another room.", ch, NULL, victim, TO_NOTVICT);
         act(AT_RED, "$N drove into $n rolling him into this room with you!", victim, NULL, ch, TO_NOTVICT);
      }
      else
      {
         act(AT_RED, "You drive into $N with your weapon but there is nowhere for $M to go that way!", ch, NULL, victim, TO_CHAR);
         act(AT_RED, "$n drives into you with your weapon but there is nowhere for you to go that way!", ch, NULL, victim, TO_VICT);
         act(AT_RED, "$n drives into $N with your weapon but there is nowhere for $M to go that way!", ch, NULL, victim, TO_NOTVICT);
      }
      stop_fighting(victim, FALSE);
      if (IS_NPC(victim))
         victim->fight_timer = 2 + MASTERED(ch, gsn_drive);
      else
         WAIT_STATE(victim, 4 + MASTERED(ch, gsn_drive)*2);
   }
   else
   {
      learn_from_failure(ch, gsn_drive, victim);
      act(AT_RED, "You drive into $N but $E refuses to budge.", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n drives into you but you refuse to budge.", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n drives into $N but $E refuses to budge.", ch, NULL, victim, TO_NOTVICT);
      global_retcode = one_hit(ch, victim, gsn_drive, LM_BODY);   
      return;
   }
}
//pretty much is do_fire but with a room check/gsn addition
void do_perfectshot(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim = NULL;
   OBJ_DATA *arrow;
   OBJ_DATA *bow;

   if ((bow = get_eq_char(ch, WEAR_MISSILE_WIELD)) == NULL)
   {
      send_to_char("But you are not wielding a missile weapon!!\n\r", ch);
      return;
   }

   one_argument(argument, arg);
   if (arg[0] == '\0' && ch->fighting == NULL)
   {
      send_to_char("Perform a perfect shot at whom or what?\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "none") || !str_cmp(arg, "self") || victim == ch)
   {
      send_to_char("How exactly did you plan on firing at yourself?\n\r", ch);
      return;
   }

   if ((arrow = get_eq_char(ch, WEAR_NOCKED)) == NULL)
   {
      send_to_char("You are not holding a projectile!\n\r", ch);
      return;
   }

   if (arrow->item_type != ITEM_PROJECTILE)
   {
      send_to_char("You are not holding a projectile!\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "target"))
   {
      if (ch->pcdata->aimtarget == NULL)
      {
         send_to_char("Your target does not exist anymore, sorry.\n\r", ch);
         return;
      }
      if (!IN_WILDERNESS(ch))
      {
         send_to_char("Can only fire at a target out in the Wilderness.\n\r", ch);
         return;
      }
      victim = ch->pcdata->aimtarget;
   }
   else if (arg[0] == '\0')
   {
      if (ch->fighting == NULL)
      {
         send_to_char("Your target does not seem to exist anymore.\n\r", ch);
         return;
      }
      victim = ch->fighting->who;
   }       
   else if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (!IN_SAME_ROOM(ch, victim))
   {
      send_to_char("You can only use this command if you are in the same room as the target!\n\r", ch);
      return;
   }

   if (bow->value[7] != arrow->value[7])
   {
      char *msg = "You have nothing to fire...\n\r";

      send_to_char(msg, ch);
      return;
   }

   /* Add wait state to fire for pkill, etc... */
   ch->fight_timer = get_btimer(ch, 1000, NULL);


   /* handle the ranged attack */
   learn_from_success(ch, gsn_perfect_shot, victim);
   ranged_attack(ch, argument, bow, arrow, gsn_perfect_shot, 1);

   return;
}
void do_deshield(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *shield;
   int level;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't deshield an individual while mounted.\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Attempt to deshield the shield of whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Why would you want to do that to yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;
   
   if (wielding_skill_weapon(ch, 0) != 5 || !get_eq_char(ch, WEAR_WIELD))
   {
      send_to_char("You have to wielding a staff to make use of this command..\n\r", ch);
      return;
   }
   if ((shield = get_eq_char(victim, WEAR_SHIELD)) == NULL)
   {
      send_to_char("Your target is not holding a shield.\n\r", ch);
      return;
   }
   
   level = POINT_LEVEL(LEARNED(ch, gsn_deshield), MASTERED(ch, gsn_deshield))*3/2;
   if (!victim->fighting) //If you aren't expecting it....
      level+=50;
   level += (get_curr_str(ch) - 14)/2;
   level += get_curr_dex(ch) - get_curr_dex(victim)*3;
   check_attacker(ch, victim);
   if (number_range(1, 1000) <= level)
   {
      learn_from_success(ch, gsn_deshield, victim);
      ch->fight_timer = get_btimer(ch, gsn_deshield, NULL);
      act(AT_RED, "You lunge at $N's shield and manage to knock it out of $S grasp!", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n lunges at your shield and manages to knock it out of your grasp!", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n lunges at $N's shield and manages to knock it out of $S grasp!", ch, NULL, victim, TO_NOTVICT);
      if ((!IS_NPC(victim) && victim->pcdata->quest && victim->pcdata->quest->questarea == victim->in_room->area)
      ||  (IS_OBJ_STAT(shield, ITEM_NOGIVE)) || (IS_OBJ_STAT(shield, ITEM_NODROP)))
      {
         unequip_char(victim, shield);
         shield->wear_loc = -1;
      }
      else
      {
         unequip_char(victim, shield);
         shield->wear_loc = -1;
         obj_from_char(shield);
         obj_to_room(shield, victim->in_room, victim);
      }
      if (!victim->fighting && IS_NPC(victim))
         one_hit(victim, ch, TYPE_HIT, LM_BODY);
   }
   else
   {
      learn_from_failure(ch, gsn_deshield, victim);
      ch->fight_timer = get_btimer(ch, gsn_deshield, NULL);
      act(AT_RED, "You lunge at $N's shield, but cannot manage to knock it out of $S grasp!", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n lunges at your shield, but cannot manage to knock it out of your grasp!", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n lunges at $N's shield, but cannot manage to knock it out of $S grasp!", ch, NULL, victim, TO_NOTVICT);
      if (!victim->fighting && IS_NPC(victim))
         one_hit(victim, ch, TYPE_HIT, LM_BODY);
   }
   return;
}

void do_weaponbreak(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *wield;
   OBJ_DATA *awield;
   int level;
   int percent;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't weaponbreak while mounted.\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Attempt to break the weapon of whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Why would you want to do that to yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;
   
   if (wielding_skill_weapon(ch, 0) != 4 || !get_eq_char(ch, WEAR_WIELD))
   {
      send_to_char("You have to wielding a blunt weapon to make use of this command..\n\r", ch);
      return;
   }
   if ((wield = get_eq_char(victim, WEAR_WIELD)) == NULL)
   {
      send_to_char("Your target is not wielding a weapon.\n\r", ch);
      return;
   }

   level = POINT_LEVEL(LEARNED(ch, gsn_weaponbreak), MASTERED(ch, gsn_weaponbreak));
   if (!victim->fighting)
      level+=15;
   level += (get_curr_str(ch) - 14);
   check_attacker(ch, victim);
   percent = 200 + level*5;
   //first attempt to snap it in half, slight chance of execution compared to actual damage
   if (number_range(1, 10000) <= percent)
   {
      learn_from_success(ch, gsn_weaponbreak, victim);
      ch->fight_timer = get_btimer(ch, gsn_weaponbreak, NULL);
      act(AT_RED, "You swing wildly at $N's weapon and snap it into two!", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n swings wildly at your weapon and snaps it into two!", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n swings wildly at $N's weapon and snaps it into two!", ch, NULL, victim, TO_NOTVICT);
      make_scraps(wield, victim);
      if ((wield =get_eq_char(victim, WEAR_DUAL_WIELD)) != NULL)
      {
         wield->wear_loc = WEAR_WIELD;
      }
      if (!victim->fighting && IS_NPC(victim))
         one_hit(victim, ch, TYPE_HIT, LM_BODY);
   }
   level+= 10;
   if (number_range(1, 100) <= level/3*2)
   {
      int sdiff;
      int dam;
      learn_from_success(ch, gsn_weaponbreak, victim);
      ch->fight_timer = get_btimer(ch, gsn_weaponbreak, NULL);
      act(AT_RED, "You swing wildly at $N's weapon and manage only to damage it!", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n swings wildly at your weapon and manages only to damage it!", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n swings wildly at $N's weapon and manages only to damage it!!", ch, NULL, victim, TO_NOTVICT); 
      
      awield = get_eq_char(ch, WEAR_WIELD);
      sdiff = awield->value[3] - wield->value[3]; 
      dam = URANGE(20, level*10 + sdiff*50, 1000);
      damage_obj(wield, ch, 0, dam);
      if (!victim->fighting && IS_NPC(victim))
         one_hit(victim, ch, TYPE_HIT, LM_BODY);
   }
   else
   {
      learn_from_failure(ch, gsn_weaponbreak, victim);
      ch->fight_timer = get_btimer(ch, gsn_weaponbreak, NULL);
      act(AT_RED, "You swing wildly at $N's weapon and miss!", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n swings wildly at your weapon and misses!", ch, NULL, victim, TO_VICT);
      act(AT_RED, "$n swings wildly at $N's weapon and misses!", ch, NULL, victim, TO_NOTVICT); 
      if (!victim->fighting && IS_NPC(victim))
         one_hit(victim, ch, TYPE_HIT, LM_BODY);
   }
   return;
}

void do_powerslice(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   int limb = 0;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't powerslice while mounted.\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Attempt to slice off the arm of whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "right"))
      limb = LM_RARM;
   if (!str_cmp(argument, "left"))
      limb = LM_LARM;
   if (limb == 0)
   {
      send_to_char("You need to select a limb to attempt to slice off\n\r", ch);
      return;
   }
   
   if (victim == ch)
   {
      send_to_char("Why would you want to do that to yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;
   
   if (wielding_skill_weapon(ch, 0) != 2 || !get_eq_char(ch, WEAR_WIELD))
   {
      send_to_char("You have to be wielding a sword to use this command.\n\r", ch);
      return;
   }
   if (ch->grip != GRIP_SLASH)
   {
      send_to_char("You need to be have a slash grip to use this skill.\n\r", ch);
      return;
   }
   check_attacker(ch, victim);
   global_retcode = one_hit(ch, victim, gsn_powerslice, limb);
   adjust_favor(ch, 10, 1);
   check_illegal_pk(ch, victim);
   return;
}
void do_pincer(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *wield;
   OBJ_DATA *dual;
   int level;

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't pincer while mounted.\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Pincer whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Why would you want to do that to yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;
   
   if (wielding_skill_weapon(ch, 0) != 1 || !get_eq_char(ch, WEAR_WIELD))
   {
      send_to_char("You have to be dual wielding axes to use this command.\n\r", ch);
      return;
   }
   else
   {
      if ((dual = get_eq_char(ch, WEAR_DUAL_WIELD)) == NULL)
      {
         send_to_char("You have to be dual wielding axes to use this command.\n\r", ch);
         return;
      }
      else
      {
         wield = get_eq_char(ch, WEAR_WIELD);
         dual->wear_loc = WEAR_WIELD;
         wield->wear_loc = WEAR_DUAL_WIELD;
         if (wielding_skill_weapon(ch, 0) != 1)
         {
            send_to_char("You have to be dual wielding axes to use this command.\n\r", ch);
            wield->wear_loc = WEAR_WIELD;
            dual->wear_loc = WEAR_DUAL_WIELD;
            return;
         }
         wield->wear_loc = WEAR_WIELD;
         dual->wear_loc = WEAR_DUAL_WIELD;
      }
   }
   level = 30;
   level += POINT_LEVEL(LEARNED(ch, gsn_pincer), MASTERED(ch, gsn_pincer))/2;
   level += (get_curr_lck(ch) - 14)/2;
   check_attacker(ch, victim);
   if (number_range(1, 100) <= level)
   { 
      learn_from_success(ch, gsn_pincer, victim);
      act(AT_RED, "You pince $N together with your axes", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n pinces you together with $s axes", ch, NULL, victim, TO_VICT);
      global_retcode = one_hit(ch, victim, gsn_pincer, LM_BODY);
      adjust_favor(ch, 10, 1);
      check_illegal_pk(ch, victim);
   }
   else
   {
      learn_from_failure(ch, gsn_pincer, victim);
      act(AT_RED, "You try to pince $N together with your axes but fail", ch, NULL, victim, TO_CHAR);
      act(AT_RED, "$n tries to pinces you together with $s axes but fails", ch, NULL, victim, TO_VICT);
      global_retcode = one_hit(ch, victim, TYPE_HIT, LM_BODY);
      adjust_favor(ch, 10, 1);
      check_illegal_pk(ch, victim);
   }
   return;
}

void do_kick_back(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   int percent;
   int level;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_kick_back), MASTERED(ch, gsn_kick_back));

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't kick back while mounted.\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Kick back whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("Why would you want to do that to yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;

   if (wielding_skill_weapon(ch, 0) != 2)
   {
      send_to_char("You can only use a sword to do a kickback.\n\r", ch);
      return;
   }
   if (ch->grip == GRIP_BASH)
   {
      send_to_char("Have to be using slash or stab grips to use kickback.\n\r", ch);
      return;
   }
   
   percent = 35+ UMIN(15, ((get_curr_str(ch) - get_curr_str(victim))*2) + ((get_curr_lck(ch) - get_curr_lck(victim))));
   percent += level*2/3;
   
   if (victim->fighting && victim->fighting->who == ch)
      percent -=30;
      
   percent = URANGE(5, percent, 95);

   check_attacker(ch, victim);
   if (number_range(1, 100) <= percent)
   {
      learn_from_success(ch, gsn_kick_back, victim);
      global_retcode = one_hit(ch, victim, gsn_kick_back, LM_BODY);
      adjust_favor(ch, 10, 1);
      check_illegal_pk(ch, victim);
   }
   else
   {
      learn_from_failure(ch, gsn_kick_back, victim);
      global_retcode = one_hit(ch, victim, TYPE_HIT, LM_BODY);
      check_illegal_pk(ch, victim);
   }
   return;
}

void do_circle(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   int percent;
   int level;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_circle), MASTERED(ch, gsn_circle));


   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (ch->mount)
   {
      send_to_char("You can't circle while mounted.\n\r", ch);
      return;
   }

   if (arg[0] == '\0')
   {
      send_to_char("Circle around whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("How can you sneak up on yourself?\n\r", ch);
      return;
   }

   if (is_safe(ch, victim))
      return;

   if (wielding_skill_weapon(ch, 0) != 7)
   {
      send_to_char("You can only circle with a dagger.\n\r", ch);
      return;
   }
   
   if (ch->grip != GRIP_STAB)
   {
      send_to_char("You can only use circle if you are gripping your dagger for a stab strike.\n\r", ch);
      return;
   }
   percent = 35+ UMIN(15, ((get_curr_dex(ch) - get_curr_dex(victim))*2) + ((get_curr_lck(ch) - get_curr_lck(victim))));
   percent += level*2/3;
   
   if (victim->fighting && victim->fighting->who == ch)
      percent -=30;
      
   percent = URANGE(5, percent, 95);

   check_attacker(ch, victim);
   if (number_range(1, 100) <= percent)
   {
      learn_from_success(ch, gsn_circle, victim);
      global_retcode = one_hit(ch, victim, gsn_circle, LM_BODY);
      adjust_favor(ch, 10, 1);
      check_illegal_pk(ch, victim);
   }
   else
   {
      learn_from_failure(ch, gsn_circle, victim);
      global_retcode = one_hit(ch, victim, TYPE_HIT, LM_BODY);
      check_illegal_pk(ch, victim);
   }
   return;
}

//Kind of what bash use to do in stock, but made more useful in the new system.
//Not quite as powerful as stun (not asleep), but it takes time to recover from
//plus it should land a bit more than stun
void do_bash(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *victim;
   int chance;
   int level;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_bash), MASTERED(ch, gsn_bash));


   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_bash] <= 0)
   {
      send_to_char("You better leave the martial arts to those who are skilled.\n\r", ch);
      return;
   }

   if ((victim = who_fighting(ch)) == NULL)
   {
      send_to_char("You aren't fighting anyone.\n\r", ch);
      return;
   }

   if (!IS_NPC(ch) && ch->move < ch->max_move / 15)
   {
      set_char_color(AT_SKILL, ch);
      send_to_char("You are far too tired to do that.\n\r", ch);
      return; /* missing return fixed March 11/96 */
   }

   if (!ch->fighting)
   {
      send_to_char("You can only use this command in battle.\n\r", ch);
      return;
   }

   ch->fight_timer = get_btimer(ch, gsn_bash, NULL);
   chance = level/5*2;
   chance += (get_curr_str(ch) - get_curr_str(victim)) * 3;
   chance -= UMIN(((get_curr_dex(victim) - 14) *2), 16);
   chance += get_curr_lck(ch) - get_curr_lck(victim);
   chance = URANGE(1, chance, 50);
   if (number_range(1, 100) <= chance)
   {
      learn_from_success(ch, gsn_bash, victim);
      if (!IS_NPC(ch))
      {
         if (MASTERED(ch, gsn_bash) == 4)
            ch->move -= ch->max_move / 30;
         else
            ch->move -= ch->max_move / 25;
      }
      victim->fight_timer += skill_table[gsn_bash]->beats + 3 + UMIN(level/10, 7);
      victim->fight_timer = UMIN(victim->fight_timer, 24);
      act(AT_SKILL, "$N bashes into you, leaving you dazed!", victim, NULL, ch, TO_CHAR);
      act(AT_SKILL, "You bash into $N, leaving $M dazed!", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n bashes into $N, leaving $M dazed!", ch, NULL, victim, TO_NOTVICT);
   }
   else
   {
      if (!IS_NPC(ch))
      {
         if (MASTERED(ch, gsn_bash) == 4)
            ch->move -= ch->max_move / 40;
         else
            ch->move -= ch->max_move / 35;
      }
      learn_from_failure(ch, gsn_bash, victim);
      act(AT_SKILL, "$n charges at you shoulder first, but you dodge out of the way.", ch, NULL, victim, TO_VICT);
      act(AT_SKILL, "You try to bash $N, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR);
      act(AT_SKILL, "$n charges shoulder first at $N, but keeps going right on past.", ch, NULL, victim, TO_NOTVICT);
   }
   return;
}

//Attempts to make the enemies in the room stop fighting with you, rather useful
//against lower level mobs if you are in a hurry.  Also removes the
//aggro flags from mobs in actual areas (wilderness mobs, well they just
//go away)
void do_roar(CHAR_DATA *ch, char *argument)
{
   int level;
   int chance;
   CHAR_DATA *victim;
   CHAR_DATA *next_victim;
   int succ = 0;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_roar), MASTERED(ch, gsn_roar));
   
   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You can't concentrate enough for that.\n\r", ch);
      return;
   }
   
   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_roar] <= 0)
   {
      send_to_char("You better leave the roaring to those who are skilled.\n\r", ch);
      return;
   }
   
   if (!ch->fighting)
   {
      send_to_char("You have to be fighting to use this command.\n\r", ch);
      return;
   }
   ch->fight_timer = get_btimer(ch, gsn_roar, NULL);
   for (victim = ch->in_room->first_person; victim; victim = next_victim)
   {
      next_victim = victim->next_in_room;
      
      if (!IN_SAME_ROOM(ch, victim))
         continue;
      if (!victim->fighting)
         continue;
      if (!IS_NPC(victim))
         continue;
      chance = level;
      chance -= victim->max_hit/5; //Won't work on any mob over 600 hp or so because of this
      chance -= (get_curr_int(victim) - 11)*2;
      chance -= (get_curr_str(victim) - 11)*2;
      chance += ch->max_hit/10;
      chance += (get_curr_str(ch) - 16)*2;
      
      if (number_range(1, 100) <= chance)
      {
         act(AT_RED, "$N cowers in fear from the mighty roar of $n", ch, NULL, victim, TO_ROOM);
         act(AT_RED, "Your mighty roar causes $N to cower in fear and cease fighting you!", ch, NULL, victim, TO_CHAR);
         succ++;
         if (IN_WILDERNESS(ch))
         {
            stop_fighting(victim, FALSE);
            extract_char(victim, TRUE);
         }
         else
         {
            stop_fighting(victim, TRUE);
            stop_hating(victim);
            stop_hunting(victim);
            start_fearing(victim, ch);
            if (xIS_SET(victim->act, ACT_AGGRESSIVE))
            {
               xREMOVE_BIT(victim->act, ACT_AGGRESSIVE);
            }
         }
      }
      else
      {
         act(AT_RED, "$n lets out a mighty roar but $N seems unimpressed", ch, NULL, victim, TO_ROOM);
         act(AT_RED, "You let out a mighty roar but $N seems not to care.", ch, NULL, victim, TO_CHAR);
      }
   }
   if (succ == 0)
      learn_from_failure(ch, gsn_roar, NULL);
   else
      learn_from_success(ch, gsn_roar, NULL);
}

/* Berserk and HitAll. -- Altrag */
void do_berserk(CHAR_DATA * ch, char *argument)
{
   sh_int percent;
   AFFECT_DATA af;
   int level;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_berserk), MASTERED(ch, gsn_berserk));

   if (IS_AFFECTED(ch, AFF_BERSERK))
   {
      send_to_char("Your rage is already at its peak!\n\r", ch);
      return;
   }
   percent = level+20;
   if (!ch->fighting)
      WAIT_STATE(ch, skill_table[gsn_berserk]->beats*2);
   else
      ch->fight_timer = get_btimer(ch, gsn_berserk, NULL);
      
   if (number_range(1, 100) > percent)
   {
      send_to_char("You couldn't build up enough rage.\n\r", ch);
      learn_from_failure(ch, gsn_berserk, ch);
      return;
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }

   af.type = gsn_berserk;
   af.duration = number_range(level/2, level)+10;
   af.modifier = level;
   af.location = APPLY_NONE;
   af.bitvector = meb(AFF_BERSERK);
   affect_to_char(ch, &af);
   send_to_char("You start to lose control..\n\r", ch);
   learn_from_success(ch, gsn_berserk, ch);
   return;
}

/* External from fight.c */
ch_ret one_hit args((CHAR_DATA * ch, CHAR_DATA * victim, int dt, int limb));
void do_hitall(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *vch;
   CHAR_DATA *vch_next;
   CHAR_DATA *mount = NULL;
   sh_int nvict = 0;
   OBJ_DATA *shield;
   sh_int nhit = 0;
   int pcount = 0;
   sh_int percent, level, mastery;
   mastery = MASTERED(ch, gsn_hitall);
   level = POINT_LEVEL(LEARNED(ch, gsn_hitall), mastery);

   if (is_room_safe(ch))
   {
      send_to_char_color("&BA godly force prevents you.\n\r", ch);
      return;
   }

   if (!ch->in_room->first_person)
   {
      send_to_char("There's no one else here!\n\r", ch);
      return;
   }
   if (IS_SET(ch->in_room->area->flags, AFLAG_NOAREA))
   {
      send_to_char("A mystical force in this area blocks your area attack.\n\r", ch);
      return;
   }
   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }
   if ((shield = get_eq_char(ch, WEAR_SHIELD)) && IS_OBJ_STAT(shield, ITEM_TWOHANDED))
   {
      send_to_char("Cannot use hitall while wearing a two-handed shield!\n\r", ch);
      return;
   }

   if (ch->position == POS_BERSERK)
      pcount = 2;
   if (ch->position == POS_AGGRESSIVE)
      pcount = 1;
   if (ch->position == POS_DEFENSIVE)
      pcount = -1;
   if (ch->position == POS_EVASIVE)
      pcount = -2;
   percent = URANGE(10, level*4/3+UMIN(get_curr_lck(ch)-14, 8), 95);
   act(AT_RED, "$n pulls back to attempt to hit everyone in the room!", ch, NULL, NULL, TO_ROOM);
   act(AT_RED, "You pull back and attempt to hit everyone in the room!", ch, NULL, NULL, TO_CHAR);
   for (vch = ch->in_room->first_person; vch; vch = vch_next)
   {
      vch_next = vch->next_in_room;
      
      if (ch->coord->x != vch->coord->x || ch->coord->y != vch->coord->y || ch->map != vch->map)
         continue;
         
      if (is_same_group(ch, vch) || !is_legal_kill(ch, vch) || !can_see(ch, vch) || is_safe(ch, vch))
         continue;

      if (++nvict > UMAX(1, URANGE(1, level / 10, 7)+pcount))
         break;
         
      check_illegal_pk(ch, vch);
      
      if (xIS_SET(vch->act, ACT_MOUNTSAVE))
         mount = vch;
         
      if (number_range(1, 100) <= percent || nhit == 0)
      {
         nhit++;
         global_retcode = one_hit(ch, vch, TYPE_HIT, ch->grip);
      }
      else
         global_retcode = damage(ch, vch, 0, TYPE_HIT, 0, -1);
       
      /* Fireshield, etc. could kill ch too.. :>.. -- Altrag */
      if (global_retcode == rCHAR_DIED || global_retcode == rBOTH_DIED || char_died(ch))
         return;
   }
   if (!nvict)
   {
      send_to_char("There's no one else here!\n\r", ch);
      return;
   }
   if (nhit)
   {
      learn_from_success(ch, gsn_hitall, mount);
      if (nhit > 1)
         ch->fight_timer = URANGE(2, nhit * get_btimer(ch, -1, NULL) * (95-(level/4))/100, 48); //24 seconds should be plenty, ha ha
      else
         ch->fight_timer = get_btimer(ch, -1, NULL);
   }
   else
   {
      if (ch->fighting)
         ch->fight_timer = get_btimer(ch, -1, NULL);
      else
         WAIT_STATE(ch, 2*get_btimer(ch, -1, NULL));
      learn_from_failure(ch, gsn_hitall, mount);
   }
   return;
}



bool check_illegal_psteal(CHAR_DATA * ch, CHAR_DATA * victim)
{
   return FALSE;
}

void do_scan(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_in_room;
   EXIT_DATA *pexit;
   sh_int dir = -1;
   sh_int dist;
   sh_int max_dist = 6;
   sh_int mastery;

   mastery = MASTERED(ch, gsn_scan) * 30;
   mastery = mastery - 90;

   set_char_color(AT_ACTION, ch);

   if (IS_AFFECTED(ch, AFF_BLIND))
   {
      send_to_char("Not very effective when you're blind...\n\r", ch);
      return;
   }
   if (ch->position <= POS_SLEEPING)
   {
      send_to_char("You cannot do that in your sleep.\n\r", ch);
      return;
   }
   if (ch->pcdata->learned[gsn_scan] <= 0 || ch->pcdata->ranking[gsn_scan] <= 0)
   {
      send_to_char("You are not skilled enough to use this skill.\n\r", ch);
      return;
   }
   if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
   {
      display_map(ch, 6000, 6000, 0);
      learn_from_success(ch, gsn_scan, NULL);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Scan in a direction...\n\r", ch);
      return;
   }

   if ((dir = get_door(argument)) == -1)
   {
      send_to_char("Scan in WHAT direction?\n\r", ch);
      return;
   }

   was_in_room = ch->in_room;
   act(AT_GREY, "Scanning $t...", ch, dir_name[dir], NULL, TO_CHAR);
   act(AT_GREY, "$n scans $t.", ch, dir_name[dir], NULL, TO_ROOM);

   if (!can_use_skill(ch, number_percent() - mastery, gsn_scan))
   {
      act(AT_GREY, "You stop scanning $t as your vision blurs.", ch, dir_name[dir], NULL, TO_CHAR);
      learn_from_failure(ch, gsn_scan, NULL);
      return;
   }

   mastery = MASTERED(ch, gsn_scan);

   if (IS_VAMPIRE(ch))
   {
      if ((gethour() < 21 && gethour() > 5) && mastery < 4)
      {
         send_to_char("You have trouble seeing clearly through all the " "light.\n\r", ch);
         max_dist = 1;
      }
   }

   if ((pexit = get_exit(ch->in_room, dir)) == NULL)
   {
      act(AT_GREY, "You can't see $t.", ch, dir_name[dir], NULL, TO_CHAR);
      return;
   }

   max_dist = max_dist + (mastery - 2);
   if (mastery <= 2)
      max_dist--;

   for (dist = 1; dist <= max_dist;)
   {
      if (IS_SET(pexit->exit_info, EX_CLOSED))
      {
         if (IS_SET(pexit->exit_info, EX_SECRET) || IS_SET(pexit->exit_info, EX_DIG))
            act(AT_GREY, "Your view $t is blocked by a wall.", ch, dir_name[dir], NULL, TO_CHAR);
         else
            act(AT_GREY, "Your view $t is blocked by a door.", ch, dir_name[dir], NULL, TO_CHAR);
         break;
      }
      if (room_is_private(pexit->to_room) && ch->level < LEVEL_STAFF) /* Tracker1 */
      {
         act(AT_GREY, "Your view $t is blocked by a private room.", ch, dir_name[dir], NULL, TO_CHAR);
         break;
      }
      if (xIS_SET(pexit->to_room->room_flags, ROOM_IMP) && ch->pcdata->caste < caste_Staff)
      {
         send_to_char("No peeking, for Staff only!\n\r", ch);
         break;
      }
      if (xIS_SET(pexit->to_room->room_flags, ROOM_MAP))
      {
         send_to_char("Your view $t is stopped at the Wilderness.\n\r", ch);
         break;
      }

      char_from_room(ch);
      char_to_room(ch, pexit->to_room);
      set_char_color(AT_RMNAME, ch);
      send_to_char(ch->in_room->name, ch);
      send_to_char("\n\r", ch);
      show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE, eItemNothing );
      show_char_to_char(ch->in_room->first_person, ch);

      switch (ch->in_room->sector_type)
      {
         default:
            dist++;
            break;
         case SECT_AIR:
            if (number_percent() < 80)
               dist++;
            break;
         case SECT_INSIDE:
         case SECT_FIELD:
         case SECT_UNDERGROUND:
            dist++;
            break;
         case SECT_FOREST:
         case SECT_CITY:
         case SECT_DESERT:
         case SECT_HILLS:
            dist += 2;
            break;
         case SECT_WATER_SWIM:
         case SECT_WATER_NOSWIM:
            dist += 3;
            break;
         case SECT_MOUNTAIN:
         case SECT_UNDERWATER:
         case SECT_OCEANFLOOR:
            dist += 4;
            break;
      }

      if (dist >= max_dist)
      {
         act(AT_GREY, "Your vision blurs with distance and you see no " "farther $t.", ch, dir_name[dir], NULL, TO_CHAR);
         break;
      }
      if ((pexit = get_exit(ch->in_room, dir)) == NULL)
      {
         act(AT_GREY, "Your view $t is blocked by a wall.", ch, dir_name[dir], NULL, TO_CHAR);
         break;
      }
   }

   char_from_room(ch);
   char_to_room(ch, was_in_room);
   learn_from_success(ch, gsn_scan, NULL);

   return;
}

// Like get_group_name, but will return Holy/Evil, used for do_study
char *sp_get_group_name(int group)
{
   if (group == 30)
      return "Holy and Evil";
   else
      return get_group_name(group);
}

/* Allows PCs to learn spells embedded in object. Should prove interesting. - Samson 8-9-98 */
/* Added Int support based off of AD&D rules -- Xerves 7/3/99 */
void do_study(CHAR_DATA * ch, char *argument) /* study by Absalom */
{
   char arg[MIL];
   OBJ_DATA *obj;
   int sn = 0;

   //   int ls;
   int mastery;
   int group;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Study what?\n\r", ch);
      return;
   }

   if ((obj = get_obj_carry(ch, arg)) == NULL)
   {
      send_to_char("You do not have that item.\n\r", ch);
      return;
   }

   if (obj->item_type != ITEM_SPELLBOOK)
   {
      send_to_char("You can only study spell books.\n\r", ch);
      return;
   }

   act(AT_MAGIC, "$n studies $p.", ch, obj, NULL, TO_ROOM);
   act(AT_MAGIC, "You study $p.", ch, obj, NULL, TO_CHAR);

   if (obj->item_type == ITEM_SPELLBOOK)
   {
      sn = obj->value[1];
      mastery = obj->value[2];
      group = skill_table[sn]->group[0];

      mastery = URANGE(0, mastery, 4);

      if (mastery == 0)
         mastery = skill_table[sn]->masterydiff[0];

      if (sn < 0 || sn >= MAX_SKILL || skill_table[sn]->spell_fun == spell_null)
      {
         bug("Do_study: bad sn %d.", sn);
         return;
      }
      //Put check in here to make sure they can learn it based off of group requirements
      if (ch->pcdata->learned[sn])
      {
         send_to_char("You already know that spell!\n\r", ch);
         return;
      }
      if (skill_table[sn]->type != SKILL_SPELL)
      {
         send_to_char("You can only study books of magical learning.\n\r", ch);
         return;
      }
      ch->pcdata->ranking[sn] = 1;
      ch->pcdata->learned[sn] = 1;

      act(AT_MAGIC, "You have learned the spell $t!", ch, skill_table[sn]->name, NULL, TO_CHAR);
      act(AT_FIRE, "$p breaks down from constant use and becomes nothing but trash.", ch, obj, NULL, TO_CHAR);
      extract_obj(obj);
      return;
   }

}


/*
 * Basically the same guts as do_scan() from above (please keep them in
 * sync) used to find the victim we're firing at.	-Thoric
 */

// moved to archery.c 
//CHAR_DATA *scan_for_victim( CHAR_DATA *ch, EXIT_DATA *pexit, char *name )


/*
 * Search inventory for an appropriate projectile to fire.
 * Also search open quivers.					-Thoric
 */
// Moved to archery.c
//OBJ_DATA *find_projectile( CHAR_DATA *ch, int type )



ch_ret spell_attack(int, int, CHAR_DATA *, void *);

/*
 * Perform the actual attack on a victim			-Thoric
 */
 // Moved to archery.c
//ch_ret ranged_got_target( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *weapon,
// OBJ_DATA *projectile, sh_int dist, sh_int dt, char *stxt, sh_int color )

/*
 * Generic use ranged attack function			-Thoric & Tricops
 */
 // Moved to archery.c
//ch_ret ranged_attack( CHAR_DATA *ch, char *argument, OBJ_DATA *weapon,
//        OBJ_DATA *projectile, sh_int dt, sh_int range )


/*
 * Fire <direction> <target>
 *
 * Fire a projectile from a missile weapon (bow, crossbow, etc)
 *
 * Design by Thoric, coding by Thoric and Tricops.
 *
 * Support code (see projectile_hit(), quiver support, other changes to
 * fight.c, etc by Thoric.
 */
 // Moved to archery.c
//void do_fire( CHAR_DATA *ch, char *argument )


/*
 * Attempt to fire at a victim.
 * Returns FALSE if no attempt was made
 */
//moved to archery.c
//bool mob_fire( CHAR_DATA *ch, char *name )


/* -- working on --
 * Syntaxes: throw object  (assumed already fighting)
 *	     throw object direction target  (all needed args for distance 
 *	          throwing)
 *	     throw object  (assumed same room throw)

void do_throw( CHAR_DATA *ch, char *argument )
{
  ROOM_INDEX_DATA *was_in_room;
  CHAR_DATA *victim;
  OBJ_DATA *throw_obj;
  EXIT_DATA *pexit;
  sh_int dir;
  sh_int dist;
  sh_int max_dist = 3;
  char arg[MIL];
  char arg1[MIL];
  char arg2[MIL];

  argument = one_argument( argument, arg );
  argument = one_argument( argument, arg1 );
  argument = one_argument( argument, arg2 );

  for ( throw_obj = ch->last_carrying; throw_obj;
	throw_obj = throw_obj=>prev_content )
  {
---    if ( can_see_obj( ch, throw_obj )
	&& ( throw_obj->wear_loc == WEAR_HELD || throw_obj->wear_loc == 
	WEAR_WIELDED || throw_obj->wear_loc == WEAR_DUAL_WIELDED )
	&& nifty_is_name( arg, throw_obj->name ) )
      break;
 ----
    if ( can_see_obj( ch, throw_obj ) && nifty_is_name( arg, throw_obj->name )
      break;
  }

  if ( !throw_obj )
  {
    send_to_char( "You aren't holding or wielding anything like that.\n\r", ch );
    return;
  }

----
  if ( ( throw_obj->item_type != ITEM_WEAPON)
  {
    send_to_char("You can only throw weapons.\n\r", ch );
    return;
  }
----

  if (get_obj_weight( throw_obj ) - ( 3 * (get_curr_str(ch) - 15) ) > 0)
  {
    send_to_char("That is too heavy for you to throw.\n\r", ch);
    if (!number_range(0,10))
      learn_from_failure( ch, gsn_throw );
    return;
  }

  if ( ch->fighting )
    victim = ch->fighting;
   else
    {
      if ( ( ( victim = get_char_room( ch, arg1 ) ) == NULL )
  	&& ( arg2[0] == '\0' ) )
      {
        act( AT_GREY, "Throw $t at whom?", ch, obj->short_descr, NULL,  
	  TO_CHAR );
        return;
      }
    }
}*/

void do_slice(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *corpse;
   OBJ_DATA *obj;
   OBJ_DATA *slice;
   bool found;
   MOB_INDEX_DATA *pMobIndex;
   char buf[MSL];
   char buf1[MSL];

   found = FALSE;


   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!IS_IMMORTAL(ch) && ch->pcdata->ranking[gsn_slice] <= 0)
   {
      send_to_char("You are not learned in this skill.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("From what do you wish to slice meat?\n\r", ch);
      return;
   }


   if ((obj = get_eq_char(ch, WEAR_WIELD)) == NULL || (obj->value[3] != 1 && obj->value[3] != 2 && obj->value[3] != 3 && obj->value[3] != 11))
   {
      send_to_char("You need to wield a sharp weapon.\n\r", ch);
      return;
   }

   if ((corpse = get_obj_here(ch, argument)) == NULL)
   {
      send_to_char("You can't find that here.\n\r", ch);
      return;
   }

   if (corpse->item_type != ITEM_CORPSE_NPC || corpse->value[3] < 75)
   {
      send_to_char("That is not a suitable source of meat.\n\r", ch);
      return;
   }

   if ((pMobIndex = get_mob_index((sh_int) - (corpse->value[2]))) == NULL)
   {
      bug("Can not find mob for value[2] of corpse, do_slice", 0);
      return;
   }

   if (get_obj_index(OBJ_VNUM_SLICE) == NULL)
   {
      bug("Vnum 24 not found for do_slice!", 0);
      return;
   }

   if (!can_use_skill(ch, number_percent(), gsn_slice) && !IS_IMMORTAL(ch))
   {
      send_to_char("You fail to slice the meat properly.\n\r", ch);
      learn_from_failure(ch, gsn_slice, NULL); /* Just in case they die :> */
      if (number_percent() + (get_curr_dex(ch) - 13) < 10)
      {
         act(AT_BLOOD, "You cut yourself!", ch, NULL, NULL, TO_CHAR);
         damage(ch, ch, number_range(2, 10), gsn_slice, 0, -1);
      }
      return;
   }

   slice = create_object(get_obj_index(OBJ_VNUM_SLICE), 0);

   sprintf(buf, "meat fresh slice %s", pMobIndex->player_name);
   STRFREE(slice->name);
   slice->name = STRALLOC(buf);

   sprintf(buf, "a slice of raw meat from %s", pMobIndex->short_descr);
   STRFREE(slice->short_descr);
   slice->short_descr = STRALLOC(buf);

   sprintf(buf1, "A slice of raw meat from %s lies on the ground.", pMobIndex->short_descr);
   STRFREE(slice->description);
   slice->description = STRALLOC(buf1);

   act(AT_BLOOD, "$n cuts a slice of meat from $p.", ch, corpse, NULL, TO_ROOM);
   act(AT_BLOOD, "You cut a slice of meat from $p.", ch, corpse, NULL, TO_CHAR);

   obj_to_char(slice, ch);
   corpse->value[3] -= 25;
   learn_from_success(ch, gsn_slice, NULL);
   return;
}

/*------------------------------------------------------------
 *  Fighting Styles - haus
 */ void do_style(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

/*  char buf[MIL];
    int percent; */

   if (IS_NPC(ch))
      return;

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      ch_printf_color(ch, "&wAdopt which fighting style?  (current:  %s&w)\n\r",
         ch->style == STYLE_BERSERK ? "&Rberserk" :
         ch->style == STYLE_DIVINE ? "&Ydivine" :
         ch->style == STYLE_WIZARDRY ? "&Ywizardry" :
         ch->style == STYLE_AGGRESSIVE ? "&Raggressive" :
         ch->style == STYLE_DEFENSIVE ? "&Ydefensive" : ch->style == STYLE_EVASIVE ? "&Yevasive" : "standard");
      return;
   }

   if (HAS_WAIT(ch))
   {
      send_to_char("You are too busy in battle to do that.\n\r", ch);
      return;
   }

   if (!str_prefix(arg, "evasive") || !str_prefix(arg, "divine") || !str_prefix(arg, "wizardry"))
   {
      int erank = 0;
      char styleb[15];
      int astyle = 0;
      char buf[MSL];
      
      if (ch->pcdata->ranking[gsn_style_evasive] <= 0 && ch->pcdata->ranking[gsn_style_divine] <= 0
      &&  ch->pcdata->ranking[gsn_style_wizardry] <= 0)
      {
         send_to_char("You have not yet learned enough to fight evasively.\n\r", ch);
         return;
      }
      if (!ch->fighting)
         WAIT_STATE(ch, 4);
      else
         ch->fight_timer = 2;
         
      if (!str_prefix(arg, "evasive"))
      {
         erank = POINT_LEVEL(LEARNED(ch, gsn_style_evasive), MASTERED(ch, gsn_style_evasive)); 
         sprintf(styleb, "evasive");
         astyle = STYLE_EVASIVE;
      }
      if (!str_prefix(arg, "divine"))
      {
         erank = POINT_LEVEL(LEARNED(ch, gsn_style_divine), MASTERED(ch, gsn_style_divine)); 
         sprintf(styleb, "divine");
         astyle = STYLE_DIVINE;
      }
      if (!str_prefix(arg, "wizardry"))
      {
         erank = POINT_LEVEL(LEARNED(ch, gsn_style_wizardry), MASTERED(ch, gsn_style_wizardry)); 
         sprintf(styleb, "wizardry");
         astyle = STYLE_WIZARDRY;
      }
         
      if (number_percent() < erank + 40)
      {
         /* success */
         if (ch->fighting)
         {
            ch->position = POS_EVASIVE;
            sprintf(buf, "$n changes stances to %s.", styleb);
            act(AT_ACTION, buf, ch, NULL, NULL, TO_ROOM);
         }
         ch->style = astyle;;
         sprintf(buf, "You change your stance to %s\n\r", styleb); 
         send_to_char(buf, ch);
         return;
      }
      else
      {
         /* failure */
         ch_printf(ch, "You nearly trip in a lame attempt to change your style to %s.\n\r", styleb);
         return;
      }
   }
   else if (!str_prefix(arg, "defensive"))
   {
      if (ch->pcdata->ranking[gsn_style_defensive] <= 0)
      {
         send_to_char("You have not yet learned enough to fight defensively.\n\r", ch);
         return;
      }
      if (!ch->fighting)
         WAIT_STATE(ch, 4);
      else
         ch->fight_timer = 2;
      if (number_percent() < POINT_LEVEL(LEARNED(ch, gsn_style_defensive), MASTERED(ch, gsn_style_defensive)) + 40)
      {
         /* success */
         if (ch->fighting)
         {
            ch->position = POS_DEFENSIVE;
            learn_from_success(ch, gsn_style_defensive, NULL);
            act(AT_ACTION, "$n moves into a defensive posture.", ch, NULL, NULL, TO_ROOM);
         }
         ch->style = STYLE_DEFENSIVE;
         send_to_char("You adopt a defensive fighting style.\n\r", ch);
         return;
      }
      else
      {
         /* failure */
         send_to_char("You nearly trip in a lame attempt to adopt a defensive fighting style.\n\r", ch);
         return;
      }
   }
   else if (!str_prefix(arg, "standard"))
   {
      if (ch->pcdata->ranking[gsn_style_standard] <= 0)
      {
         send_to_char("You have not yet learned enough to fight in the standard style.\n\r", ch);
         return;
      }
      if (!ch->fighting)
         WAIT_STATE(ch, 4);
      else
         ch->fight_timer = 2;
      if (number_percent() < POINT_LEVEL(LEARNED(ch, gsn_style_standard), MASTERED(ch, gsn_style_standard)) + 40)
      {
         /* success */
         if (ch->fighting)
         {
            ch->position = POS_FIGHTING;
            learn_from_success(ch, gsn_style_standard, NULL);
            act(AT_ACTION, "$n switches to a standard fighting style.", ch, NULL, NULL, TO_ROOM);
         }
         ch->style = STYLE_FIGHTING;
         send_to_char("You adopt a standard fighting style.\n\r", ch);
         return;
      }
      else
      {
         /* failure */
         send_to_char("You nearly trip in a lame attempt to adopt a standard fighting style.\n\r", ch);
         return;
      }
   }
   else if (!str_prefix(arg, "aggressive"))
   {
      if (ch->pcdata->ranking[gsn_style_aggressive] <= 0)
      {
         send_to_char("You have not yet learned enough to fight aggressively.\n\r", ch);
         return;
      }
      if (!ch->fighting)
         WAIT_STATE(ch, 4);
      else
         ch->fight_timer = 2;
      if (number_percent() < POINT_LEVEL(LEARNED(ch, gsn_style_aggressive), MASTERED(ch, gsn_style_aggressive)) + 40)
      {
         /* success */
         if (ch->fighting)
         {
            ch->position = POS_AGGRESSIVE;
            learn_from_success(ch, gsn_style_aggressive, NULL);
            act(AT_ACTION, "$n assumes an aggressive stance.", ch, NULL, NULL, TO_ROOM);
         }
         ch->style = STYLE_AGGRESSIVE;
         send_to_char("You adopt an aggressive fighting style.\n\r", ch);
         return;
      }
      else
      {
         /* failure */
         send_to_char("You nearly trip in a lame attempt to adopt an aggressive fighting style.\n\r", ch);
         return;
      }
   }
   else if (!str_prefix(arg, "berserk"))
   {
      if (ch->pcdata->ranking[gsn_style_berserk] <= 0)
      {
         send_to_char("You have not yet learned enough to fight as a berserker.\n\r", ch);
         return;
      }
      if (!ch->fighting)
         WAIT_STATE(ch, 4);
      else
         ch->fight_timer = 2;
      if (number_percent() < POINT_LEVEL(LEARNED(ch, gsn_style_berserk), MASTERED(ch, gsn_style_berserk)) + 40)
      {
         /* success */
         if (ch->fighting)
         {
            ch->position = POS_BERSERK;
            learn_from_success(ch, gsn_style_berserk, NULL);
            act(AT_ACTION, "$n enters a wildly aggressive style.", ch, NULL, NULL, TO_ROOM);
         }
         ch->style = STYLE_BERSERK;
         send_to_char("You adopt a berserk fighting style.\n\r", ch);
         return;
      }
      else
      {
         /* failure */
         send_to_char("You nearly trip in a lame attempt to adopt a berserk fighting style.\n\r", ch);
         return;
      }
   }

   send_to_char("Adopt which fighting style?\n\r", ch);

   return;
}

/*  New check to see if you can use skills to support morphs --Shaddai */
bool can_use_skill(CHAR_DATA * ch, int percent, int gsn)
{
   bool check = FALSE;
   int suc;
   
   suc = 1;
   if (!IS_NPC(ch))
   {
      if ((percent + skill_table[gsn]->difficulty * 5) > (85 + (ch->pcdata->learned[gsn]*5)))
         suc = 0;
   }

   if (!IS_NPC(ch) && ((ch->pcdata->learned[gsn] < 1 || ch->pcdata->learned[gsn] > MAX_SKPOINTS)
         || (ch->pcdata->ranking[gsn] < 1 || ch->pcdata->ranking[gsn] > MAX_RANKING)))
      return FALSE;
   if (IS_NPC(ch) && percent < 85)
      check = TRUE;
   else if (suc == 1)
      check = TRUE;
   else if (ch->morph && ch->morph->morph && ch->morph->morph->skills &&
      ch->morph->morph->skills[0] != '\0' && is_name(skill_table[gsn]->name, ch->morph->morph->skills) && percent < 85)
      check = TRUE;
   if (ch->morph && ch->morph->morph && ch->morph->morph->no_skills &&
      ch->morph->morph->no_skills[0] != '\0' && is_name(skill_table[gsn]->name, ch->morph->morph->no_skills))
      check = FALSE;
   return check;
}

/*
 * Cook was coded by Blackmane and heavily modified by Shaddai
 */
void do_cook(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *food, *fire;
   char arg[MIL];
   char buf[MSL];
   sh_int level;
   
   level = POINT_LEVEL(LEARNED(ch, gsn_cook), MASTERED(ch, gsn_cook));

   one_argument(argument, arg);
   if (IS_NPC(ch) || ch->pcdata->ranking[gsn_cook] <= 0)
   {
      send_to_char("That skill is beyond your understanding.\n\r", ch);
      return;
   }
   if (arg[0] == '\0')
   {
      send_to_char("Cook what?\n\r", ch);
      return;
   }

   if (ms_find_obj(ch))
      return;

   if ((food = get_obj_carry(ch, arg)) == NULL)
   {
      send_to_char("You do not have that item.\n\r", ch);
      return;
   }
   if (food->item_type != ITEM_COOK)
   {
      send_to_char("How can you cook that?\n\r", ch);
      return;
   }
   if (food->value[2] > 2)
   {
      send_to_char("That is already burnt to a crisp.\n\r", ch);
      return;
   }
   for (fire = ch->in_room->first_content; fire; fire = fire->next_content)
   {
      if (fire->item_type == ITEM_FIRE)
         break;
   }
   if (!fire)
   {
      send_to_char("There is no fire here!\n\r", ch);
      return;
   }
   if (number_percent() > level+35)
   {
      food->timer = food->timer / 2;
      food->value[0] = 0;
      food->value[2] = 3;
      act(AT_MAGIC, "$p catches on fire burning it to a crisp!\n\r", ch, food, NULL, TO_CHAR);
      act(AT_MAGIC, "$n catches $p on fire burning it to a crisp.", ch, food, NULL, TO_ROOM);
      sprintf(buf, "a burnt %s", food->pIndexData->name);
      STRFREE(food->short_descr);
      food->short_descr = STRALLOC(buf);
      sprintf(buf, "A burnt %s.", food->pIndexData->name);
      STRFREE(food->description);
      food->description = STRALLOC(buf);
      return;
   }

   if (number_percent() > level +55)
   {
      food->timer = food->timer * 3;
      food->value[2] += 2;
      act(AT_MAGIC, "$n overcooks a $p.", ch, food, NULL, TO_ROOM);
      act(AT_MAGIC, "You overcook a $p.", ch, food, NULL, TO_CHAR);
      sprintf(buf, "an overcooked %s", food->pIndexData->name);
      STRFREE(food->short_descr);
      food->short_descr = STRALLOC(buf);
      sprintf(buf, "An overcooked %s.", food->pIndexData->name);
      STRFREE(food->description);
      food->description = STRALLOC(buf);
   }
   else
   {
      food->timer = food->timer * 4;
      food->value[0] *= 2;
      act(AT_MAGIC, "$n roasts a $p.", ch, food, NULL, TO_ROOM);
      act(AT_MAGIC, "You roast a $p.", ch, food, NULL, TO_CHAR);
      sprintf(buf, "a roasted %s", food->pIndexData->name);
      STRFREE(food->short_descr);
      food->short_descr = STRALLOC(buf);
      sprintf(buf, "A roasted %s.", food->pIndexData->name);
      STRFREE(food->description);
      food->description = STRALLOC(buf);
      food->value[2]++;
   }
   learn_from_success(ch, gsn_cook, NULL);
}

//restores limbs, used at a mob, cost is based on health
void do_restorelimbs(CHAR_DATA *ch, char *argument)
{
   CHAR_DATA *mob;
   int cost;
   char buf[MSL];

    /* Search for an act_healer */
   for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room)
   {
      if (IS_NPC(mob) && xIS_SET(mob->act, ACT_RESTORELIMBS))
         break;
   }
   if (mob == NULL)
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }

   cost = (20+ch->max_hit) * ch->max_hit *.4;
   if (argument[0] == '\0')
   {
      sprintf(buf, "It will cost you %d to restore a limb.  Choices for limbs are: rarm  larm  rleg  lleg", cost);
      do_say(mob, buf);
      return;
   }
   sprintf(buf, "It will cost you %d to restore a limb.", cost);
   if (!str_cmp(argument, "rarm"))
   {
      if (ch->con_rarm != -1)
      {
         do_say(mob, "You can only restore broken limbs.");
         return;
      }
      if (ch->gold < cost)
      {
         do_say(mob, buf);
         return;
      }
      ch->gold -= cost;
      ch->con_rarm = 100;
      act(AT_WHITE, "$N waves $S hands around and restores $n's right arm.", ch, NULL, mob, TO_ROOM);
      act(AT_WHITE, "$N waves $S hands around and restores your right arm.", ch, NULL, mob, TO_CHAR);
      return;
   }  
   if (!str_cmp(argument, "larm"))
   {
      if (ch->con_larm != -1)
      {
         do_say(mob, "You can only restore broken limbs.");
         return;
      }
      if (ch->gold < cost)
      {
         do_say(mob, buf);
         return;
      }
      ch->gold -= cost;
      ch->con_larm = 100;
      act(AT_WHITE, "$N waves $S hands around and restores $n's left arm.", ch, NULL, mob, TO_ROOM);
      act(AT_WHITE, "$N waves $S hands around and restores your left arm.", ch, NULL, mob, TO_CHAR);
      return;
   }  
   if (!str_cmp(argument, "rleg"))
   {
      if (ch->con_rleg != -1)
      {
         do_say(mob, "You can only restore broken limbs.");
         return;
      }
      if (ch->gold < cost)
      {
         do_say(mob, buf);
         return;
      }
      ch->gold -= cost;
      ch->con_rleg = 100;
      act(AT_WHITE, "$N waves $S hands around and restores $n's right leg.", ch, NULL, mob, TO_ROOM);
      act(AT_WHITE, "$N waves $S hands around and restores your right leg.", ch, NULL, mob, TO_CHAR);
      return;
   }  
   if (!str_cmp(argument, "lleg"))
   {
      if (ch->con_lleg != -1)
      {
         do_say(mob, "You can only restore broken limbs.");
         return;
      }
      if (ch->gold < cost)
      {
         do_say(mob, buf);
         return;
      }
      ch->gold -= cost;
      ch->con_lleg = 100;
      act(AT_WHITE, "$N waves $S hands around and restores $n's left leg.", ch, NULL, mob, TO_ROOM);
      act(AT_WHITE, "$N waves $S hands around and restores your left leg.", ch, NULL, mob, TO_CHAR);
      return;
   }  
   do_restorelimbs(ch, "");
   return;
}
/* Healer, for lower levels and just for fun -- Xerves */
void do_heal(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *mob;
   char arg[MIL];
   int cost, sn;
   SPELL_FUN *spell;
   char *words;

   /* Search for an act_healer */
   for (mob = ch->in_room->first_person; mob; mob = mob->next_in_room)
   {
      if (IS_NPC(mob) && xIS_SET(mob->act, ACT_HEALER))
         break;
   }

   if (mob == NULL)
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      /* display price list */
      act(AT_PLAIN, "$N says 'I offer the following spells:'", ch, NULL, mob, TO_CHAR);
      send_to_char("\n  &Rlight:      &Ccure light wounds        &Y200  gold\n\r", ch);
      send_to_char("  &Rserious:    &Ccure serious wounds      &Y300  gold\n\r", ch);
      send_to_char("  &Rcritical:   &Ccure critical wounds     &Y400  gold\n\r", ch);
      send_to_char("  &Rheal:       &Chealing spell            &Y1000 gold\n\r", ch);
      send_to_char("  &Rblind:      &Ccure blindness           &Y1200  gold\n\r", ch);
      send_to_char("  &Rpoison:     &Ccure poison              &Y1200  gold\n\r", ch);
      send_to_char("  &Rcurse:      &Cremove curse             &Y2500  gold\n\r", ch);
      send_to_char("  &Rsanctuary:  &Csanctuary                &Y5000 gold\n\r", ch);
      send_to_char("  &Rstrength:   &Ckindred strength         &Y4000 gold\n\r", ch);
      send_to_char("  &Rdexterity:  &Cslink                    &Y4000 gold\n\r", ch);
      send_to_char("  &Rwisdom:     &Csagacity                 &Y4000 gold\n\r", ch);
      send_to_char("  &Rint:        &Cdragon wit               &Y4000 gold\n\r", ch);
      send_to_char("  &Rcon:        &Ctrollish vigor           &Y4000 gold\n\r", ch);
      send_to_char("  &Rarmor:      &Carmor                    &Y2000 gold\n\r", ch);
      send_to_char("  &Rshield:     &Cshield                   &Y2500 gold\n\r", ch);
      send_to_char("  &Rrefresh:    &Crestore movement         &Y500  gold\n\r", ch);
      send_to_char("  &Rmana:       &Crestore mana             &Y800  gold\n\n\r", ch);
      send_to_char("  &c&wType heal <type> to be healed.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "light"))
   {
      spell = spell_smaug;
      sn = skill_lookup("cure light");
      words = "judicandus dies";
      cost = 200;
   }

   else if (!str_cmp(arg, "serious"))
   {
      spell = spell_smaug;
      sn = skill_lookup("cure serious");
      words = "judicandus gzfuajg";
      cost = 300;
   }

   else if (!str_cmp(arg, "critical"))
   {
      spell = spell_smaug;
      sn = skill_lookup("cure critical");
      words = "judicandus qfuhuqar";
      cost = 400;
   }

   else if (!str_cmp(arg, "heal"))
   {
      spell = spell_smaug;
      sn = skill_lookup("heal");
      words = "pzar";
      cost = 1000;
   }

   else if (!str_cmp(arg, "blind"))
   {
      spell = spell_cure_blindness;
      sn = skill_lookup("cure blindness");
      words = "judicandus noselacri";
      cost = 1200;
   }

   else if (!str_cmp(arg, "poison"))
   {
      spell = spell_cure_poison;
      sn = skill_lookup("cure poison");
      words = "judicandus sausabru";
      cost = 1200;
   }

   else if (!str_cmp(arg, "curse"))
   {
      spell = spell_remove_curse;
      sn = skill_lookup("remove curse");
      words = "candussido judifgz";
      cost = 2500;
   }

   else if (!str_cmp(arg, "mana"))
   {
      spell = NULL;
      sn = -1;
      words = "energizer";
      cost = 800;
   }


   else if (!str_cmp(arg, "refresh"))
   {
      spell = spell_smaug;
      sn = skill_lookup("refresh");
      words = "candusima";
      cost = 500;
   }

   else if (!str_cmp(arg, "sanctuary"))
   {
      spell = spell_smaug;
      sn = skill_lookup("sanctuary");
      words = "heliamheldra";
      cost = 5000;
   }

   else if (!str_cmp(arg, "strength"))
   {
      spell = spell_smaug;
      sn = skill_lookup("kindred strength");
      words = "stremdupha";
      cost = 4000;
   }

   else if (!str_cmp(arg, "wisdom"))
   {
      spell = spell_smaug;
      sn = skill_lookup("sagacity");
      words = "wifda";
      cost = 4000;
   }

   else if (!str_cmp(arg, "dexterity"))
   {
      spell = spell_smaug;
      sn = skill_lookup("slink");
      words = "zooooom";
      cost = 4000;
   }

   else if (!str_cmp(arg, "int"))
   {
      spell = spell_smaug;
      sn = skill_lookup("dragon wit");
      words = "knofdapiel";
      cost = 4000;
   }

   else if (!str_cmp(arg, "con"))
   {
      spell = spell_smaug;
      sn = skill_lookup("trollish vigor");
      words = "skupfta";
      cost = 4000;
   }

   else if (!str_cmp(arg, "armor"))
   {
      spell = spell_smaug;
      sn = skill_lookup("armor");
      words = "feskunda";
      cost = 2000;
   }

   else if (!str_cmp(arg, "shield"))
   {
      spell = spell_smaug;
      sn = skill_lookup("shield");
      words = "rayoprot";
      cost = 2500;
   }

   else
   {
      act(AT_PLAIN, "$N says ' Type 'heal' for a list of spells.'", ch, NULL, mob, TO_CHAR);
      return;
   }

   if (cost > ch->gold)
   {
      act(AT_PLAIN, "$N says 'You do not have enough gold for my services.'", ch, NULL, mob, TO_CHAR);
      return;
   }

   WAIT_STATE(ch, PULSE_VIOLENCE);
   ch->gold -= cost;
   act(AT_PLAIN, "$n utters the words '$T'.", mob, NULL, words, TO_ROOM);

   if (spell == NULL)
   {
      ch->mana += dice(10, 5);
      if (ch->mana > ch->max_mana)
         ch->mana = ch->max_mana;
      send_to_char("A warm glow passes through you.\n\r", ch);
      return;
   }

   if (sn == -1)
      return;

   spell(sn, mob->level, mob, ch);
}
