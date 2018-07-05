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
 *			   "Special procedure" module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"




/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN(spec_breath_any);
DECLARE_SPEC_FUN(spec_breath_acid);
DECLARE_SPEC_FUN(spec_breath_fire);
DECLARE_SPEC_FUN(spec_breath_frost);
DECLARE_SPEC_FUN(spec_breath_gas);
DECLARE_SPEC_FUN(spec_breath_lightning);
DECLARE_SPEC_FUN(spec_cast_adept);
DECLARE_SPEC_FUN(spec_cast_cleric);
DECLARE_SPEC_FUN(spec_cast_mage);
DECLARE_SPEC_FUN(spec_cast_undead);
DECLARE_SPEC_FUN(spec_executioner);
DECLARE_SPEC_FUN(spec_fido);
DECLARE_SPEC_FUN(spec_guard);
DECLARE_SPEC_FUN(spec_janitor);
DECLARE_SPEC_FUN(spec_mayor);
DECLARE_SPEC_FUN(spec_poison);
DECLARE_SPEC_FUN(spec_thief);
DECLARE_SPEC_FUN(spec_questmaster);
DECLARE_SPEC_FUN(spec_gemcutter);


/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup(const char *name)
{
   if (!str_cmp(name, "spec_breath_any"))
      return spec_breath_any;
   if (!str_cmp(name, "spec_breath_acid"))
      return spec_breath_acid;
   if (!str_cmp(name, "spec_breath_fire"))
      return spec_breath_fire;
   if (!str_cmp(name, "spec_breath_frost"))
      return spec_breath_frost;
   if (!str_cmp(name, "spec_breath_gas"))
      return spec_breath_gas;
   if (!str_cmp(name, "spec_breath_lightning"))
      return spec_breath_lightning;
   if (!str_cmp(name, "spec_cast_adept"))
      return spec_cast_adept;
   if (!str_cmp(name, "spec_cast_cleric"))
      return spec_cast_cleric;
   if (!str_cmp(name, "spec_cast_mage"))
      return spec_cast_mage;
   if (!str_cmp(name, "spec_cast_undead"))
      return spec_cast_undead;
   if (!str_cmp(name, "spec_executioner"))
      return spec_executioner;
   if (!str_cmp(name, "spec_fido"))
      return spec_fido;
   if (!str_cmp(name, "spec_guard"))
      return spec_guard;
   if (!str_cmp(name, "spec_janitor"))
      return spec_janitor;
   if (!str_cmp(name, "spec_mayor"))
      return spec_mayor;
   if (!str_cmp(name, "spec_poison"))
      return spec_poison;
   if (!str_cmp(name, "spec_thief"))
      return spec_thief;
   if (!str_cmp(name, "spec_questmaster"))
      return spec_questmaster;
   if (!str_cmp(name, "spec_gemcutter"))
      return spec_gemcutter;
   return 0;
}

/*
 * Given a pointer, return the appropriate spec fun text.
 */
char *lookup_spec(SPEC_FUN * special)
{
   if (special == spec_breath_any)
      return "spec_breath_any";

   if (special == spec_breath_acid)
      return "spec_breath_acid";
   if (special == spec_breath_fire)
      return "spec_breath_fire";
   if (special == spec_breath_frost)
      return "spec_breath_frost";
   if (special == spec_breath_gas)
      return "spec_breath_gas";
   if (special == spec_breath_lightning)
      return "spec_breath_lightning";
   if (special == spec_cast_adept)
      return "spec_cast_adept";
   if (special == spec_cast_cleric)
      return "spec_cast_cleric";
   if (special == spec_cast_mage)
      return "spec_cast_mage";
   if (special == spec_cast_undead)
      return "spec_cast_undead";
   if (special == spec_executioner)
      return "spec_executioner";
   if (special == spec_fido)
      return "spec_fido";
   if (special == spec_guard)
      return "spec_guard";
   if (special == spec_janitor)
      return "spec_janitor";
   if (special == spec_mayor)
      return "spec_mayor";
   if (special == spec_poison)
      return "spec_poison";
   if (special == spec_thief)
      return "spec_thief";
   if (special == spec_questmaster)
      return "spec_questmaster";
   if (special == spec_gemcutter)
      return "spec_gemcutter";
   return "";
}


/* if a spell casting mob is hating someone... try and summon them */
void summon_if_hating(CHAR_DATA * ch)
{
   CHAR_DATA *victim;
   char buf[MSL];
   char name[MIL];
   bool found = FALSE;

   if (ch->position <= POS_SLEEPING)
      return;

   if (ch->fighting || ch->fearing || !ch->hating || is_room_safe(ch))
      return;

   /* if player is close enough to hunt... don't summon */
   if (ch->hunting)
      return;

   one_argument(ch->hating->name, name);

   /* make sure the char exists - works even if player quits */
   for (victim = first_char; victim; victim = victim->next)
   {
      if (!str_cmp(ch->hating->name, victim->name))
      {
         found = TRUE;
         break;
      }
   }

   if (!found)
      return;
   if (ch->in_room == victim->in_room)
      return;
   if (!IS_NPC(victim))
      sprintf(buf, "summon 0.%s", name);
   else
      sprintf(buf, "summon %s", name);
   do_cast(ch, buf);
   return;
}

/*
 * Core procedure for dragons.
 */
bool dragon(CHAR_DATA * ch, char *spell_name)
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   int sn;

   if (ch->position != POS_FIGHTING
      && ch->position != POS_EVASIVE && ch->position != POS_DEFENSIVE && ch->position != POS_AGGRESSIVE && ch->position != POS_BERSERK)
      return FALSE;

   for (victim = ch->in_room->first_person; victim; victim = v_next)
   {
      v_next = victim->next_in_room;
      if (who_fighting(victim) == ch && number_bits(2) == 0)
         break;
   }

   if (!victim)
      return FALSE;

   if ((sn = skill_lookup(spell_name)) < 0)
      return FALSE;
   (*skill_table[sn]->spell_fun) (sn, ch->level, ch, victim);
   return TRUE;
}



/*
 * Special procedures for mobiles.
 */
bool spec_breath_any(CHAR_DATA * ch)
{
   if (ch->position != POS_FIGHTING
      && ch->position != POS_EVASIVE && ch->position != POS_DEFENSIVE && ch->position != POS_AGGRESSIVE && ch->position != POS_BERSERK)
      return FALSE;

   switch (number_bits(3))
   {
      case 0:
         return spec_breath_fire(ch);
      case 1:
      case 2:
         return spec_breath_lightning(ch);
      case 3:
         return spec_breath_gas(ch);
      case 4:
         return spec_breath_acid(ch);
      case 5:
      case 6:
      case 7:
         return spec_breath_frost(ch);
   }

   return FALSE;
}



bool spec_breath_acid(CHAR_DATA * ch)
{
   return dragon(ch, "acid breath");
}



bool spec_breath_fire(CHAR_DATA * ch)
{
   return dragon(ch, "fire breath");
}



bool spec_breath_frost(CHAR_DATA * ch)
{
   return dragon(ch, "frost breath");
}



bool spec_breath_gas(CHAR_DATA * ch)
{
   int sn;

   if (ch->position != POS_FIGHTING
      && ch->position != POS_EVASIVE && ch->position != POS_DEFENSIVE && ch->position != POS_AGGRESSIVE && ch->position != POS_BERSERK)
      return FALSE;

   if ((sn = skill_lookup("gas breath")) < 0)
      return FALSE;
   (*skill_table[sn]->spell_fun) (sn, ch->level, ch, NULL);
   return TRUE;
}



bool spec_breath_lightning(CHAR_DATA * ch)
{
   return dragon(ch, "lightning breath");
}



bool spec_cast_adept(CHAR_DATA * ch)
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;

   if (!IS_AWAKE(ch) || ch->fighting)
      return FALSE;

   for (victim = ch->in_room->first_person; victim; victim = v_next)
   {
      v_next = victim->next_in_room;
      if (victim != ch && can_see(ch, victim))
         break;
   }

   if (!victim)
      return FALSE;

   switch (number_bits(3))
   {
      case 0:
         act(AT_MAGIC, "$n utters the word 'ciroht'.", ch, NULL, NULL, TO_ROOM);
         spell_smaug(skill_lookup("armor"), ch->level, ch, victim);
         return TRUE;

      case 1:
         act(AT_MAGIC, "$n utters the word 'sunimod'.", ch, NULL, NULL, TO_ROOM);
         spell_smaug(skill_lookup("bless"), ch->level, ch, victim);
         return TRUE;

      case 2:
         act(AT_MAGIC, "$n utters the word 'suah'.", ch, NULL, NULL, TO_ROOM);
         spell_cure_blindness(skill_lookup("cure blindness"), ch->level, ch, victim);
         return TRUE;

      case 3:
         act(AT_MAGIC, "$n utters the word 'nran'.", ch, NULL, NULL, TO_ROOM);
         spell_smaug(skill_lookup("cure light"), ch->level, ch, victim);
         return TRUE;

      case 4:
         act(AT_MAGIC, "$n utters the word 'nyrcs'.", ch, NULL, NULL, TO_ROOM);
         spell_cure_poison(skill_lookup("cure poison"), ch->level, ch, victim);
         return TRUE;

      case 5:
         act(AT_MAGIC, "$n utters the word 'gartla'.", ch, NULL, NULL, TO_ROOM);
         spell_smaug(skill_lookup("refresh"), ch->level, ch, victim);
         return TRUE;

      case 6:
         act(AT_MAGIC, "$n utters the word 'naimad'.", ch, NULL, NULL, TO_ROOM);
         spell_smaug(skill_lookup("cure serious"), ch->level, ch, victim);
         return TRUE;

      case 7:
         act(AT_MAGIC, "$n utters the word 'gorog'.", ch, NULL, NULL, TO_ROOM);
         spell_remove_curse(skill_lookup("remove curse"), ch->level, ch, victim);
         return TRUE;

   }

   return FALSE;
}



bool spec_cast_cleric(CHAR_DATA * ch)
{
   return FALSE;
}



bool spec_cast_mage(CHAR_DATA * ch)
{
   return FALSE;
}



bool spec_cast_undead(CHAR_DATA * ch)
{
   return FALSE;
}



bool spec_executioner(CHAR_DATA * ch)
{
   char buf[MSL];
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   char *crime;

   if (!IS_AWAKE(ch) || ch->fighting)
      return FALSE;

   crime = "";
   for (victim = ch->in_room->first_person; victim; victim = v_next)
   {
      v_next = victim->next_in_room;

      if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_KILLER))
      {
         crime = "KILLER";
         break;
      }

      if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_THIEF))
      {
         crime = "THIEF";
         break;
      }
   }

   if (!victim)
      return FALSE;

   if (is_room_safe(ch))
   {
      sprintf(buf, "%s You are a COWARD!", victim->name);
      do_yell(ch, buf);
      return TRUE;
   }

   sprintf(buf, "%s PROTECT THE INNOCENT!  MORE BLOOOOD!!!", victim->name);
   do_tell(ch, buf);
   one_hit(ch, victim, TYPE_UNDEFINED, LM_BODY);
   if (char_died(ch))
      return TRUE;

   /* Added log in case of missing cityguard -- Tri */
 /*
   cityguard = get_mob_index(MOB_VNUM_CITYGUARD);

   if (!cityguard)
   {
      sprintf(buf, "Missing Cityguard - Vnum:[%d]", MOB_VNUM_CITYGUARD);
      bug(buf, 0);
      return TRUE;
   }

   char_to_room(create_mobile(cityguard), ch->in_room);
   char_to_room(create_mobile(cityguard), ch->in_room); */
   return TRUE;
}



bool spec_fido(CHAR_DATA * ch)
{
   OBJ_DATA *corpse;
   OBJ_DATA *c_next;
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;

   if (!IS_AWAKE(ch))
      return FALSE;

   for (corpse = ch->in_room->first_content; corpse; corpse = c_next)
   {
      c_next = corpse->next_content;
      if (corpse->item_type != ITEM_CORPSE_NPC)
         continue;

      act(AT_ACTION, "$n savagely devours a corpse.", ch, NULL, NULL, TO_ROOM);
      for (obj = corpse->first_content; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         obj_from_obj(obj);
         obj_to_room(obj, ch->in_room, ch);
      }
      extract_obj(corpse);
      return TRUE;
   }

   return FALSE;
}

bool spec_questmaster(CHAR_DATA * ch)
{
   if (!IS_NPC(ch))
      return FALSE;
   else
      return TRUE;
}

bool spec_gemcutter(CHAR_DATA * ch)
{
   if (!IS_NPC(ch))
      return FALSE;
   else
      return TRUE;
}

bool spec_guard(CHAR_DATA * ch)
{
   char buf[MSL];
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   CHAR_DATA *ech;
   char *crime;
   int max_evil;

   if (!IS_AWAKE(ch) || ch->fighting)
      return FALSE;

   max_evil = 300;
   ech = NULL;
   crime = "";

   for (victim = ch->in_room->first_person; victim; victim = v_next)
   {
      v_next = victim->next_in_room;

      if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_KILLER))
      {
         crime = "KILLER";
         break;
      }

      if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_THIEF))
      {
         crime = "THIEF";
         break;
      }

      if (victim->fighting && who_fighting(victim) != ch && victim->alignment < max_evil)
      {
         max_evil = victim->alignment;
         ech = victim;
      }
   }

   if (victim && is_room_safe(ch))
   {
      sprintf(buf, "%s You are a COWARD!", victim->name);
      do_tell(ch, buf);
      return TRUE;
   }

   if (victim)
   {
      sprintf(buf, "%s PROTECT THE INNOCENT!!  BANZAI!!", victim->name);
      do_tell(ch, buf);
      one_hit(ch, victim, TYPE_UNDEFINED, LM_BODY);
      return TRUE;
   }

   if (ech)
   {
      act(AT_YELL, "$n screams 'PROTECT THE INNOCENT!!  BANZAI!!", ch, NULL, NULL, TO_ROOM);
      one_hit(ch, ech, TYPE_UNDEFINED, LM_BODY);
      return TRUE;
   }

   return FALSE;
}



bool spec_janitor(CHAR_DATA * ch)
{
   OBJ_DATA *trash;
   OBJ_DATA *trash_next;

   if (!IS_AWAKE(ch))
      return FALSE;

   for (trash = ch->in_room->first_content; trash; trash = trash_next)
   {
      trash_next = trash->next_content;
      if (!IS_SET(trash->wear_flags, ITEM_TAKE) || IS_OBJ_STAT(trash, ITEM_BURIED))
         continue;
      if (trash->item_type == ITEM_DRINK_CON
         || trash->item_type == ITEM_TRASH || trash->cost < 10 || (trash->pIndexData->vnum == OBJ_VNUM_SHOPPING_BAG && !trash->first_content))
      {
         act(AT_ACTION, "$n picks up some trash.", ch, NULL, NULL, TO_ROOM);
         obj_from_room(trash);
         obj_to_char(trash, ch);
         return TRUE;
      }
   }

   return FALSE;
}



bool spec_mayor(CHAR_DATA * ch)
{
   static const char open_path[] = "W3a3003b33000c111d0d111Oe333333Oe22c222112212111a1S.";

   static const char close_path[] = "W3a3003b33000c111d0d111CE333333CE22c222112212111a1S.";

   static const char *path;
   static int pos;
   static bool move;

   if (!move)
   {
      if (gethour() == 6)
      {
         path = open_path;
         move = TRUE;
         pos = 0;
      }

      if (gethour() == 20)
      {
         path = close_path;
         move = TRUE;
         pos = 0;
      }
   }

   if (ch->fighting)
      return spec_cast_cleric(ch);
   if (!move || ch->position < POS_SLEEPING)
      return FALSE;

   switch (path[pos])
   {
      case '0':
      case '1':
      case '2':
      case '3':
         move_char(ch, get_exit(ch->in_room, path[pos] - '0'), 0);
         break;

      case 'W':
         ch->position = POS_STANDING;
         act(AT_ACTION, "$n awakens and groans loudly.", ch, NULL, NULL, TO_ROOM);
         break;

      case 'S':
         ch->position = POS_SLEEPING;
         act(AT_ACTION, "$n lies down and falls asleep.", ch, NULL, NULL, TO_ROOM);
         break;

      case 'a':
         act(AT_SAY, "$n says 'Hello Honey!'", ch, NULL, NULL, TO_ROOM);
         break;

      case 'b':
         act(AT_SAY, "$n says 'What a view!  I must do something about that dump!'", ch, NULL, NULL, TO_ROOM);
         break;

      case 'c':
         act(AT_SAY, "$n says 'Vandals!  Youngsters have no respect for anything!'", ch, NULL, NULL, TO_ROOM);
         break;

      case 'd':
         act(AT_SAY, "$n says 'Good day, citizens!'", ch, NULL, NULL, TO_ROOM);
         break;

      case 'e':
         act(AT_SAY, "$n says 'I hereby declare the town of Darkhaven open!'", ch, NULL, NULL, TO_ROOM);
         break;

      case 'E':
         act(AT_SAY, "$n says 'I hereby declare the town of Darkhaven closed!'", ch, NULL, NULL, TO_ROOM);
         break;

      case 'O':
         do_unlock(ch, "gate");
         do_open(ch, "gate");
         break;

      case 'C':
         do_close(ch, "gate");
         do_lock(ch, "gate");
         break;

      case '.':
         move = FALSE;
         break;
   }

   pos++;
   return FALSE;
}



bool spec_poison(CHAR_DATA * ch)
{
   CHAR_DATA *victim;

   if (ch->position != POS_FIGHTING
      && ch->position != POS_EVASIVE && ch->position != POS_DEFENSIVE && ch->position != POS_AGGRESSIVE && ch->position != POS_BERSERK)
      return FALSE;

   if ((victim = who_fighting(ch)) == NULL || number_percent() > 75)
      return FALSE;

   act(AT_HIT, "You bite $N!", ch, NULL, victim, TO_CHAR);
   act(AT_ACTION, "$n bites $N!", ch, NULL, victim, TO_NOTVICT);
   act(AT_POISON, "$n bites you!", ch, NULL, victim, TO_VICT);
   spell_poison(gsn_poison, ch->level, ch, victim);
   return TRUE;
}



bool spec_thief(CHAR_DATA * ch)
{
   CHAR_DATA *victim;
   CHAR_DATA *v_next;
   int gold;

   if (ch->position != POS_STANDING)
      return FALSE;

   for (victim = ch->in_room->first_person; victim; victim = v_next)
   {
      v_next = victim->next_in_room;

      if (IS_NPC(victim) || victim->level >= LEVEL_IMMORTAL || number_bits(2) != 0 || !can_see(ch, victim)) /* Thx Glop */
         continue;

      if (IS_AWAKE(victim) && number_range(0, 20) == 0)
      {
         act(AT_ACTION, "You discover $n's hands in your sack of gold!", ch, NULL, victim, TO_VICT);
         act(AT_ACTION, "$N discovers $n's hands in $S sack of gold!", ch, NULL, victim, TO_NOTVICT);
         return TRUE;
      }
      else
      {
         gold = number_range(victim->gold/15, victim->gold/25);
         ch->gold += gold;
         victim->gold -= gold;
         return TRUE;
      }
   }

   return FALSE;
}
