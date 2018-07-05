/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 ****************************************************************************
 *  The MUDprograms are heavily based on the original MOBprogram code that  *
 *  was written by N'Atas-ha.						    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

char *mprog_type_to_name args((int type));

char *mprog_type_to_name(int type)
{
   switch (type)
   {
      case IN_FILE_PROG:
         return "in_file_prog";
      case ACT_PROG:
         return "act_prog";
      case SPEECH_PROG:
         return "speech_prog";
      case RAND_PROG:
         return "rand_prog";
      case FIGHT_PROG:
         return "fight_prog";
      case HITPRCNT_PROG:
         return "hitprcnt_prog";
      case DEATH_PROG:
         return "death_prog";
      case ENTRY_PROG:
         return "entry_prog";
      case GREET_PROG:
         return "greet_prog";
      case ALL_GREET_PROG:
         return "all_greet_prog";
      case GIVE_PROG:
         return "give_prog";
      case BRIBE_PROG:
         return "bribe_prog";
      case HOUR_PROG:
         return "hour_prog";
      case TIME_PROG:
         return "time_prog";
      case WEAR_PROG:
         return "wear_prog";
      case REMOVE_PROG:
         return "remove_prog";
      case SAC_PROG:
         return "sac_prog";
      case LOOK_PROG:
         return "look_prog";
      case EXA_PROG:
         return "exa_prog";
      case ZAP_PROG:
         return "zap_prog";
      case GET_PROG:
         return "get_prog";
      case DROP_PROG:
         return "drop_prog";
      case REPAIR_PROG:
         return "repair_prog";
      case DAMAGE_PROG:
         return "damage_prog";
      case PULL_PROG:
         return "pull_prog";
      case PUSH_PROG:
         return "push_prog";
      case SCRIPT_PROG:
         return "script_prog";
      case SLEEP_PROG:
         return "sleep_prog";
      case REST_PROG:
         return "rest_prog";
      case LEAVE_PROG:
         return "leave_prog";
      case USE_PROG:
         return "use_prog";
      default:
         return "ERROR_PROG";
   }
}

/* A trivial rehack of do_mstat.  This doesnt show all the data, but just
 * enough to identify the mob and give its basic condition.  It does however,
 * show the MUDprograms which are set.
 */
void do_mpstat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   MPROG_DATA *mprg;
   CHAR_DATA *victim;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("MProg stat whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim))
   {
      send_to_char("Only Mobiles can have MobPrograms!\n\r", ch);
      return;
   }

   if (xIS_EMPTY(victim->pIndexData->progtypes))
   {
      send_to_char("That Mobile has no Programs set.\n\r", ch);
      return;
   }

   ch_printf(ch, "Name: %s.  Vnum: %d.\n\r", victim->name, victim->pIndexData->vnum);

   ch_printf(ch, "Short description: %s.\n\rLong  description: %s",
      victim->short_descr, victim->long_descr[0] != '\0' ? victim->long_descr : "(none).\n\r");

   ch_printf(ch, "Hp: %d/%d.  Mana: %d/%d.  Move: %d/%d. \n\r",
      victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move);

   ch_printf(ch,
      "Gold: %d.\n\r",
      victim->gold);

   for (mprg = victim->pIndexData->mudprogs; mprg; mprg = mprg->next)
      ch_printf(ch, ">%s %s\n\r%s\n\r", mprog_type_to_name(mprg->type), mprg->arglist, mprg->comlist);
   return;
}

/* Opstat - Scryn 8/12*/
void do_opstat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   MPROG_DATA *mprg;
   OBJ_DATA *obj;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("OProg stat what?\n\r", ch);
      return;
   }

   if ((obj = get_obj_world(ch, arg)) == NULL)
   {
      send_to_char("You cannot find that.\n\r", ch);
      return;
   }

   if (xIS_EMPTY(obj->pIndexData->progtypes))
   {
      send_to_char("That object has no programs set.\n\r", ch);
      return;
   }

   ch_printf(ch, "Name: %s.  Vnum: %d.\n\r", obj->name, obj->pIndexData->vnum);

   ch_printf(ch, "Short description: %s.\n\r", obj->short_descr);

   for (mprg = obj->pIndexData->mudprogs; mprg; mprg = mprg->next)
      ch_printf(ch, ">%s %s\n\r%s\n\r", mprog_type_to_name(mprg->type), mprg->arglist, mprg->comlist);

   return;

}

/* Rpstat - Scryn 8/12 */
void do_rpstat(CHAR_DATA * ch, char *argument)
{
   MPROG_DATA *mprg;

   if (xIS_EMPTY(ch->in_room->progtypes))
   {
      send_to_char("This room has no programs set.\n\r", ch);
      return;
   }

   ch_printf(ch, "Name: %s.  Vnum: %d.\n\r", ch->in_room->name, ch->in_room->vnum);

   for (mprg = ch->in_room->mudprogs; mprg; mprg = mprg->next)
      ch_printf(ch, ">%s %s\n\r%s\n\r", mprog_type_to_name(mprg->type), mprg->arglist, mprg->comlist);
   return;
}

/* Woowoo - Blodkai, November 1997 */
void do_mpasupress(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   int rnds;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Mpasupress who?\n\r", ch);
      progbug("Mpasupress:  invalid (nonexistent?) argument", ch);
      return;
   }
   if (arg2[0] == '\0')
   {
      send_to_char("Supress their attacks for how many rounds?\n\r", ch);
      progbug("Mpasupress:  invalid (nonexistent?) argument", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("No such victim in the room.\n\r", ch);
      progbug("Mpasupress:  victim not present", ch);
      return;
   }
   rnds = atoi(arg2);
   if (rnds < 0 || rnds > 32000)
   {
      send_to_char("Invalid number of rounds to supress attacks.\n\r", ch);
      progbug("Mpsupress:  invalid (nonexistent?) argument", ch);
      return;
   }
   add_timer(victim, TIMER_ASUPRESSED, rnds, NULL, 0);
   return;
}

OBJ_DATA *get_obj_carry_prog(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   int number, count, vnum;

   number = number_argument(argument, arg);
   vnum = atoi(arg);

   count = 0;
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
      if (can_see_obj(ch, obj) && (nifty_is_name(arg, obj->name) || obj->pIndexData->vnum == vnum))
         if ((count += obj->count) >= number)
            return obj;

   if (vnum != -1)
      return NULL;

   /*
    * If we didn't find an exact match, run through the list of objects
    * again looking for prefix matching, ie swo == sword.
    * Added by Narn, Sept/96
    */
   count = 0;
   for (obj = ch->last_carrying; obj; obj = obj->prev_content)
      if (can_see_obj(ch, obj) && nifty_is_name_prefix(arg, obj->name))
         if ((count += obj->count) >= number)
            return obj;

   return NULL;
}

//Takes an item from a player, not a lot of checking going on here....
void do_mptake(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (!IS_NPC(ch))
      return;

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      return;
   }
   if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
   {
      return;
   }
   
   if ((obj = get_obj_carry_prog(victim, arg1)) == NULL)
   {
      return;
   }

   if (obj->wear_loc != WEAR_NONE)
       unequip_char(victim, obj);
       
   separate_obj(obj);
   obj_from_char(obj);
   obj = obj_to_char(obj, ch);
   if (IS_SET(sysdata.save_flags, SV_RECEIVE) && !char_died(victim))
      save_char_obj(victim);
   if (!str_cmp(argument, "destroy"))
      extract_obj(obj);
   return;
}
//Will give an item to a player, less checking here to make sure item is given...
void do_mpgive(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   
   if (!IS_NPC(ch))
      return;

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      return;
   }
   
   if ((obj = get_obj_carry_prog(ch, arg1)) == NULL)
   {
      return;
   }

   if ((victim = get_char_room_new(ch, arg2, 1)) == NULL)
   {
      return;
   }

   if (get_ch_carry_number(victim) + (get_obj_number(obj) / obj->count) > can_carry_n(victim))
   {
      progbug("Mpgive:  Trying to give an item and target cannot carry it.", ch);
      return;
   }

   if (get_ch_carry_weight(victim) + (get_obj_weight(obj) / obj->count) > can_carry_w(victim))
   {
      progbug("Mpgive:  Trying to give an item and target cannot carry it.", ch);
      return;
   }

   if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE) && !can_take_proto(victim))
   {
      progbug("Mpgive:  Trying to give away a prototype item, failing...", ch);
      return;
   }
   if (obj->wear_loc != WEAR_NONE)
       unequip_char(victim, obj);
   separate_obj(obj);
   obj_from_char(obj);
   obj = obj_to_char(obj, victim);
   if (IS_SET(sysdata.save_flags, SV_GIVE) && !char_died(victim))
         save_char_obj(victim);
   return;
}

/* This and do practice run the practicing scheme for the whole skill system.  This
prog will teach a different mastery.  It is setup so it will find out if the skill
is standalone or in a group then it will do a lot of stuff.  If you want to know
the stuff, go look at all the functions in handler for most of it.  -- Xerves 1/00 */
//Not used anymore, replaced with do_learn
void do_mpteach(CHAR_DATA * ch, char *argument)
{
   return;   
}

void do_mpvalue(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   
   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!ch)
   {
      bug("Nonexistent ch in do_mpkill!", 0);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpvalue - no argument", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      progbug("Mpvalue - Victim not in room", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      progbug("Mpvalue - No value provided", ch);
      return;
   }
   if (argument[0] == '+')
      victim->tcount += atoi(&argument[1]);
   else if (argument[0] == '-')
      victim->tcount -= atoi(&argument[1]);
   else
      victim->tcount = atoi(argument);
   return;
}

/* lets the mobile kill any player or mobile without murder*/
void do_mpkill(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;


   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!ch)
   {
      bug("Nonexistent ch in do_mpkill!", 0);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("MpKill - no argument", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      progbug("MpKill - Victim not in room", ch);
      return;
   }

   if (victim == ch)
   {
      progbug("MpKill - Bad victim to attack", ch);
      return;
   }

   if (ch->position == POS_FIGHTING
      || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
   {
      progbug("MpKill - Already fighting", ch);
      return;
   }

   one_hit(ch, victim, TYPE_UNDEFINED, LM_BODY);
   return;
}


/* lets the mobile destroy an object in its inventory
   it can also destroy a worn object and it can destroy
   items using all.xxxxx or just plain all of them */

void do_mpjunk(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   OBJ_DATA *obj_next;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpjunk - No argument", ch);
      return;
   }

   if (str_cmp(arg, "all") && str_prefix("all.", arg))
   {
      if ((obj = get_obj_wear(ch, arg)) != NULL)
      {
         unequip_char(ch, obj);
         extract_obj(obj);
         return;
      }
      if ((obj = get_obj_carry(ch, arg)) == NULL)
         return;
      extract_obj(obj);
   }
   else
      for (obj = ch->first_carrying; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         if (arg[3] == '\0' || is_name(&arg[4], obj->name))
         {
            if (obj->wear_loc != WEAR_NONE)
               unequip_char(ch, obj);
            extract_obj(obj);
         }
      }

   return;

}

/*
 * This function examines a text string to see if the first "word" is a
 * color indicator (e.g. _red, _whi_, _blu).  -  Gorog
 */
int get_color(char *argument) /* get color code from command string */
{
   char color[MIL];
   char *cptr;
   static char const *color_list = "_bla_red_dgr_bro_dbl_pur_cya_cha_dch_ora_gre_yel_blu_pin_lbl_whi";
   static char const *blink_list = "*bla*red*dgr*bro*dbl*pur*cya*cha*dch*ora*gre*yel*blu*pin*lbl*whi";

   one_argument(argument, color);
   if (color[0] != '_' && color[0] != '*')
      return 0;
   if ((cptr = strstr(color_list, color)))
      return (cptr - color_list) / 4;
   if ((cptr = strstr(blink_list, color)))
      return (cptr - blink_list) / 4 + AT_BLINK;
   return 0;
}

/* Prints the argument to all the rooms around the mobile */
void do_mpasound(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   ROOM_INDEX_DATA *was_in_room;
   EXIT_DATA *pexit;
   sh_int color;
   EXT_BV actflags;

   if (!ch)
   {
      bug("Nonexistent ch in do_mpasound!", 0);
      return;
   }
   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      progbug("Mpasound - No argument", ch);
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   if ((color = get_color(argument)))
      argument = one_argument(argument, arg1);
   was_in_room = ch->in_room;
   for (pexit = was_in_room->first_exit; pexit; pexit = pexit->next)
   {
      if (pexit->to_room && pexit->to_room != was_in_room)
      {
         ch->in_room = pexit->to_room;
         MOBtrigger = FALSE;
         if (color)
            act(color, argument, ch, NULL, NULL, TO_ROOM);
         else
            act(AT_SAY, argument, ch, NULL, NULL, TO_ROOM);
      }
   }
   ch->act = actflags;
   ch->in_room = was_in_room;
   return;
}

/* prints the message to all in the room other than the mob and victim */
void do_mpechoaround(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   EXT_BV actflags;
   sh_int color;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpechoaround - No argument", ch);
      return;
   }

   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      progbug("Mpechoaround - victim does not exist", ch);
      return;
   }

   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);

   if ((color = get_color(argument)))
   {
      argument = one_argument(argument, arg);
      act(color, argument, ch, NULL, victim, TO_NOTVICT);
   }
   else
      act(AT_ACTION, argument, ch, NULL, victim, TO_NOTVICT);

   ch->act = actflags;
}


/* prints message only to victim */

void do_mpechoat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   EXT_BV actflags;
   sh_int color;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      progbug("Mpechoat - No argument", ch);
      return;
   }

   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      progbug("Mpechoat - victim does not exist", ch);
      return;
   }

   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);

   if ((color = get_color(argument)))
   {
      argument = one_argument(argument, arg);
      act(color, argument, ch, NULL, victim, TO_VICT);
   }
   else
      act(AT_ACTION, argument, ch, NULL, victim, TO_VICT);

   ch->act = actflags;
}


/* prints message to room at large. */

void do_mpecho(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   sh_int color;
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   if (argument[0] == '\0')
   {
      progbug("Mpecho - called w/o argument", ch);
      return;
   }

   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);

   if ((color = get_color(argument)))
   {
      argument = one_argument(argument, arg1);
      act(color, argument, ch, NULL, NULL, TO_ROOM);
   }
   else
      act(AT_ACTION, argument, ch, NULL, NULL, TO_ROOM);

   ch->act = actflags;
}

/* sound support -haus */

void do_mpsoundaround(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char sound[MIL];
   CHAR_DATA *victim;
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpsoundaround - No argument", ch);
      return;
   }

   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      progbug("Mpsoundaround - victim does not exist", ch);
      return;
   }

   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);

   sprintf(sound, "!!SOUND(%s)\n", argument);
   act(AT_ACTION, sound, ch, NULL, victim, TO_NOTVICT);

   ch->act = actflags;
}


/* prints message only to victim */

void do_mpsoundat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char sound[MIL];
   CHAR_DATA *victim;
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      progbug("Mpsoundat - No argument", ch);
      return;
   }

   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      progbug("Mpsoundat - victim does not exist", ch);
      return;
   }

   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);

   sprintf(sound, "!!SOUND(%s)\n", argument);
   act(AT_ACTION, sound, ch, NULL, victim, TO_VICT);

   ch->act = actflags;
}


/* prints message to room at large. */

void do_mpsound(CHAR_DATA * ch, char *argument)
{
   char sound[MIL];
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   if (argument[0] == '\0')
   {
      progbug("Mpsound - called w/o argument", ch);
      return;
   }

   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);

   sprintf(sound, "!!SOUND(%s)\n", argument);
   act(AT_ACTION, sound, ch, NULL, NULL, TO_ROOM);

   ch->act = actflags;
}

/* end sound stuff ----------------------------------------*/

/* Music stuff, same as above, at zMUD coders' request -- Blodkai */
void do_mpmusicaround(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char music[MIL];
   CHAR_DATA *victim;
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      progbug("Mpmusicaround - No argument", ch);
      return;
   }
   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      progbug("Mpmusicaround - victim does not exist", ch);
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   sprintf(music, "!!MUSIC(%s)\n", argument);
   act(AT_ACTION, music, ch, NULL, victim, TO_NOTVICT);
   ch->act = actflags;
   return;
}
void do_mpmusic(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char music[MIL];
   CHAR_DATA *victim;
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      progbug("Mpmusic - No argument", ch);
      return;
   }
   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      progbug("Mpmusic - victim does not exist", ch);
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   sprintf(music, "!!MUSIC(%s)\n", argument);
   act(AT_ACTION, music, ch, NULL, victim, TO_ROOM);
   ch->act = actflags;
   return;
}
void do_mpmusicat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char music[MIL];
   CHAR_DATA *victim;
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      progbug("Mpmusicat - No argument", ch);
      return;
   }
   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      progbug("Mpmusicat - victim does not exist", ch);
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   sprintf(music, "!!MUSIC(%s)\n", argument);
   act(AT_ACTION, music, ch, NULL, victim, TO_VICT);
   ch->act = actflags;
   return;
}

/* lets the mobile load an item or mobile.  All items
are loaded into inventory.  you can specify a level with
the load object portion as well. */
void do_mpmload(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *victim;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0' || !is_number(arg))
   {
      progbug("Mpmload - Bad vnum as arg", ch);
      return;
   }

   if ((pMobIndex = get_mob_index(atoi(arg))) == NULL)
   {
      progbug("Mpmload - Bad mob vnum", ch);
      return;
   }

   victim = create_mobile(pMobIndex);
   char_to_room(victim, ch->in_room);

   if (IS_ONMAP_FLAG(ch))
   {
      SET_ONMAP_FLAG(victim);
      victim->map = ch->map;
      victim->coord->x = ch->coord->x;
      victim->coord->y = ch->coord->y;
   }

   return;
}

void do_mpoload(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int timer = 0;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0' || !is_number(arg1))
   {
      progbug("Mpoload - Bad syntax", ch);
      return;
   }

   if ((pObjIndex = get_obj_index(atoi(arg1))) == NULL)
   {
      progbug("Mpoload - Bad vnum arg", ch);
      return;
   }

   obj = create_object(pObjIndex, 1);
   obj->timer = timer;
   if (CAN_WEAR(obj, ITEM_TAKE))
      obj_to_char(obj, ch);
   else
      obj_to_room(obj, ch->in_room, ch);

   return;
}

/* Just a hack of do_pardon from act_wiz.c -- Blodkai, 6/15/97 */
void do_mppardon(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      progbug("Mppardon:  missing argument", ch);
      send_to_char("Mppardon who for what?\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      progbug("Mppardon: offender not present", ch);
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      progbug("Mppardon:  trying to pardon NPC", ch);
      send_to_char("Not on NPC's.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "attacker"))
   {
      if (xIS_SET(victim->act, PLR_ATTACKER))
      {
         xREMOVE_BIT(victim->act, PLR_ATTACKER);
         send_to_char("Attacker flag removed.\n\r", ch);
         send_to_char("Your crime of attack has been pardoned.\n\r", victim);
      }
      return;
   }
   if (!str_cmp(arg2, "killer"))
   {
      if (xIS_SET(victim->act, PLR_KILLER))
      {
         xREMOVE_BIT(victim->act, PLR_KILLER);
         send_to_char("Killer flag removed.\n\r", ch);
         send_to_char("Your crime of murder has been pardoned.\n\r", victim);
      }
      return;
   }
   if (!str_cmp(arg2, "litterbug"))
   {
      if (xIS_SET(victim->act, PLR_LITTERBUG))
      {
         xREMOVE_BIT(victim->act, PLR_LITTERBUG);
         send_to_char("Litterbug flag removed.\n\r", ch);
         send_to_char("Your crime of littering has been pardoned./n/r", victim);
      }
      return;
   }
   if (!str_cmp(arg2, "thief"))
   {
      if (xIS_SET(victim->act, PLR_THIEF))
      {
         xREMOVE_BIT(victim->act, PLR_THIEF);
         send_to_char("Thief flag removed.\n\r", ch);
         send_to_char("Your crime of theft has been pardoned.\n\r", victim);
      }
      return;
   }
   send_to_char("Pardon who for what?\n\r", ch);
   progbug("Mppardon: Invalid argument", ch);
   return;
}

/* lets the mobile purge all objects and other npcs in the room,
   or purge a specified object or mob in the room.  It can purge
   itself, but this had best be the last command in the MUDprogram
   otherwise ugly stuff will happen */
void do_mppurge(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      /* 'purge' */
      CHAR_DATA *vnext;

      for (victim = ch->in_room->first_person; victim; victim = vnext)
      {
         vnext = victim->next_in_room;
         if (IS_NPC(victim) && victim != ch)
            extract_char(victim, TRUE);
      }
      while (ch->in_room->first_content)
         extract_obj(ch->in_room->first_content);

      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      if ((obj = get_obj_here(ch, arg)) != NULL)
         extract_obj(obj);
      else
         progbug("Mppurge - Bad argument", ch);
      return;
   }

   if (!IS_NPC(victim))
   {
      progbug("Mppurge - Trying to purge a PC", ch);
      return;
   }

   if (victim == ch)
   {
      progbug("Mppurge - Trying to purge oneself", ch);
      return;
   }

   if (IS_NPC(victim) && victim->pIndexData->vnum == 3)
   {
      progbug("Mppurge: trying to purge supermob", ch);
      return;
   }

   extract_char(victim, TRUE);
   return;
}


/* Allow mobiles to go wizinvis with programs -- SB */

void do_mpinvis(CHAR_DATA * ch, char *argument)
{
   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   ch->mobinvis = 2;

   if (xIS_SET(ch->act, ACT_MOBINVIS))
   {
      xREMOVE_BIT(ch->act, ACT_MOBINVIS);
      act(AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You slowly fade back into existence.\n\r", ch);
   }
   else
   {
      xSET_BIT(ch->act, ACT_MOBINVIS);
      act(AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
      send_to_char("You slowly vanish into thin air.\n\r", ch);
   }
   return;
}

/* lets the mobile goto any location it wishes that is not private */
/* Mounted chars follow their mobiles now - Blod, 11/97 */
void do_mpgoto(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *location;
   CHAR_DATA *fch;
   CHAR_DATA *fch_next;

//    CHAR_DATA *wch;
//    OBJ_DATA *obj;
   ROOM_INDEX_DATA *in_room;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      progbug("Mpgoto - No argument", ch);
      return;
   }
   /* Begin Overland Map additions */
   if (!str_cmp(arg, "map"))
   {
      char arg1[MIL];
      char arg2[MIL];
      char buf[MSL];
      int x, y;
      int map = -1;

      argument = one_argument(argument, arg1);
      argument = one_argument(argument, arg2);

      if (arg1[0] == '\0')
      {
         progbug("Mpgoto - No map supplied for a map goto.\n\r", ch);
         return;
      }

      if (!str_cmp(arg1, "solan"))
         map = MAP_SOLAN;

      if (map == -1)
      {
         sprintf(buf, "Mpgoto - There isn't a map for '%s'.\n\r", arg1);
         progbug(buf, ch);
         return;
      }

      if (arg2[0] == '\0' && argument[0] == '\0')
      {
         enter_map(ch, 258, 250, map);
         return;
      }

      if (arg2[0] == '\0' || argument[0] == '\0')
      {
         progbug("Mpgoto - Going to a map but did not supply coordinates", ch);
         return;
      }

      x = atoi(arg2);
      y = atoi(argument);

      if (x < 1 || x > MAX_X)
      {
         progbug("Mpgoto - Invalid x coordinate", ch);
         return;
      }

      if (y < 1 || y > MAX_Y)
      {
         progbug("Mpgoto - Invalid y coordinate", ch);
         return;
      }
      
      if (room_is_private(get_room_index(OVERLAND_SOLAN)))
      {
         progbug("Mpgoto - Private room", ch);
         return;
      }
      
      if (room_is_private_wilderness(ch, get_room_index(OVERLAND_SOLAN), ch->coord->x, ch->coord->y, ch->map))
      {
         progbug("Mpgoto - Private room", ch);
         return;
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
      enter_map(ch, x, y, map);
      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
      return;
   }

   /* End of Overland Map additions */
   if ((location = find_location(ch, arg)) == NULL)
   {
      progbug("Mpgoto - No such location", ch);
      return;
   }
   
   if (room_is_private(location))
   {
      progbug("Mpgoto - Private room", ch);
      return;
   }

   in_room = ch->in_room;
   if (ch->fighting)
      stop_fighting(ch, TRUE);
   char_from_room(ch);
   if (ch->on)
   {
      ch->on = NULL;
      ch->position = POS_STANDING;
   }
   if (ch->position != POS_STANDING)
   {
      ch->position = POS_STANDING;
   }
   char_to_room(ch, location);
   for (fch = in_room->first_person; fch; fch = fch_next)
   {
      fch_next = fch->next_in_room;
      if (fch->mount && fch->mount == ch)
      {
         char_from_room(fch);
         char_to_room(fch, location);
      }
   }
   return;
}

/* lets the mobile do a command at another location. Very useful */

void do_mpat(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      progbug("Mpat - Bad argument", ch);
      return;
   }

   if ((location = find_location(ch, arg)) == NULL)
   {
      progbug("Mpat - No such location", ch);
      return;
   }

   // origmap = ch->map;
   // origx = ch->coord->x;
   // origy = ch->coord->y;

   /* Bunch of checks to make sure the imm is on the same grid as the object - Samson */
   /*  if( xIS_SET( location->room_flags, ROOM_MAP ) && !IS_ACT_FLAG( ch, ACT_ONMAP ) )
      {
      SET_ACT_FLAG( ch, ACT_ONMAP );
      ch->map = obj->map;
      ch->coord->x = obj->coord->x;
      ch->coord->y = obj->coord->y;
      }
      else if( xIS_SET( location->room_flags, ROOM_MAP ) && IS_ACT_FLAG( ch, ACT_ONMAP ) )
      {
      ch->map = obj->map;
      ch->coord->x = obj->coord->x;
      ch->coord->y = obj->coord->y;
      }
      else if( !xIS_SET( location->room_flags, ROOM_MAP ) && IS_ACT_FLAG( ch, ACT_ONMAP ) )
      {
      REMOVE_ACT_FLAG( ch, ACT_ONMAP );
      ch->map = -1;
      ch->coord->x = -1;
      ch->coord->y = -1;
      }       */

   original = ch->in_room;
   char_from_room(ch);
   char_to_room(ch, location);
   //  update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
   interpret(ch, argument);

   //   if( IS_PLR_FLAG( ch, PLR_ONMAP ) && !xIS_SET( original->room_flags, ROOM_MAP ) )
// REMOVE_PLR_FLAG( ch, PLR_ONMAP );
//    else if( !IS_PLR_FLAG( ch, PLR_ONMAP ) && xIS_SET( original->room_flags, ROOM_MAP ) )
// SET_PLR_FLAG( ch, PLR_ONMAP );

   //   ch->map = origmap;
   //   ch->coord->x = origx;
   //   ch->coord->y = origy;
//    update_objects(ch, ch->coord->x, ch->coord->y, ch->map);

   if (!char_died(ch))
   {
      char_from_room(ch);
      char_to_room(ch, original);
   }

   return;
}

/* allow a mobile to advance a player's level... very dangerous */
//removed -- Xerves
void do_mpadvance(CHAR_DATA * ch, char *argument)
{
   return;
}



/* lets the mobile transfer people.  the all argument transfers
   everyone in the current room to the specified location 
   the area argument transfers everyone in the current area to the
   specified location */

void do_mptransfer(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];
   ROOM_INDEX_DATA *location;
   CHAR_DATA *victim;
   CHAR_DATA *nextinroom;
   CHAR_DATA *immortal;
   DESCRIPTOR_DATA *d;
   sh_int x = -1;
   sh_int y = -1;


   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0')
   {
      progbug("Mptransfer - Bad syntax", ch);
      return;
   }

   /* Put in the variable nextinroom to make this work right. -Narn */
   if (!str_cmp(arg1, "all"))
   {
      for (victim = ch->in_room->first_person; victim; victim = nextinroom)
      {
         nextinroom = victim->next_in_room;
         if (victim != ch && !NOT_AUTHED(victim) && can_see(ch, victim))
         {
            sprintf(buf, "%s %s", victim->name, arg2);
            do_mptransfer(ch, buf);
         }
      }
      return;
   }
   // Transfer only players in the room...
   if (!str_cmp(arg1, "players"))
   {
      for (victim = ch->in_room->first_person; victim; victim = nextinroom)
      {
         nextinroom = victim->next_in_room;
         if (victim != ch
            && !NOT_AUTHED(victim) && can_see(ch, victim) && !IS_NPC(victim))
         {
            sprintf(buf, "%s %s", victim->name, arg2);
            do_mptransfer(ch, buf);
         }
      }
      return;
   }
   /* This will only transfer PC's in the area not Mobs --Shaddai */
   if (!str_cmp(arg1, "area"))
   {
      for (d = first_descriptor; d; d = d->next)
      {
         if (!d->character || (d->connected != CON_PLAYING &&
               d->connected != CON_EDITING) || !can_see_map(ch, d->character)
            || ch->in_room->area != d->character->in_room->area || NOT_AUTHED(d->character))
            continue;

         sprintf(buf, "%s %s", d->character->name, arg2);
         do_mptransfer(ch, buf);
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
         progbug("Mptransfer - No such location", ch);
         return;
      }

      if (room_is_private(location))
      {
         progbug("Mptransfer - Private room", ch);
         return;
      }
      if (room_is_private_wilderness(ch, location, ch->coord->x, ch->coord->y, ch->map))
      {
         progbug("Mptransfer - Private room", ch);
         return;
      }
   }

   if ((victim = get_char_world(ch, arg1)) == NULL)
   {
      progbug("Mptransfer - No such person", ch);
      return;
   }

   if (!victim->in_room)
   {
      progbug("Mptransfer - Victim in Limbo", ch);
      return;
   }

   if (NOT_AUTHED(victim) && location->area != victim->in_room->area)
   {
      sprintf(buf, "Mptransfer - unauthed char (%s)", victim->name);
      progbug(buf, ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      if (xIS_SET(victim->act, PLR_GAMBLER))
      {
         progbug("Mptransfer - Gambling victim trying to be transed", ch);
         return;
      }
   }
   if (location->vnum == OVERLAND_SOLAN)
   {
      if (!IS_ONMAP_FLAG(ch) && IS_ONMAP_FLAG(victim))
      {
         progbug("Mptransfer - Trying to transfer a player/mob around map while off map", ch);
         return;
      }
   }


   if (victim->fighting)
      stop_fighting(victim, TRUE);

/* hey... if an immortal's following someone, they should go with a mortal
 * when they're mptrans'd, don't you think?
 *  -- TRI
 */

   for (immortal = victim->in_room->first_person; immortal; immortal = nextinroom)
   {
      nextinroom = immortal->next_in_room;
      if (IS_NPC(immortal) || get_trust(immortal) < LEVEL_IMMORTAL || immortal->master != victim)
         continue;
      if (immortal->fighting)
         stop_fighting(immortal, TRUE);
      char_from_room(immortal);
      char_to_room(immortal, location);
   }

   char_from_room(victim);
   if (victim->on)
   {
      victim->on = NULL;
      victim->position = POS_STANDING;
   }
   if (victim->position != POS_STANDING && victim->position != POS_RIDING)
   {
      victim->position = POS_STANDING;
   }
   char_to_room(victim, location);

   if (location->vnum == OVERLAND_SOLAN)
   {
      argument = one_argument(argument, arg3);
      if (arg3[0] != '\0' && argument[0] != '\0')
      {
         x = atoi(arg3);
         y = atoi(argument);

         if (x < 1 || x > MAX_X)
         {
            sprintf(buf, "Mptransfer - Valid x coordinates are 1 to %d.\n\rThrowing out coordinates.", MAX_X);
            progbug(buf, ch);
            x = -1;
            y = -1;
         }

         if (y < 1 || y > MAX_Y)
         {
            sprintf(buf, "Mptransfer - Valid y coordinates are 1 to %d.\n\rThrowing out coordinates.", MAX_Y);
            progbug(buf, ch);
            x = -1;
            x = -1;
         }
      }
      SET_ONMAP_FLAG(victim);
      victim->map = MAP_SOLAN;
      victim->coord->x = x;
      victim->coord->y = y;
   }
   else
   {
      REMOVE_ONMAP_FLAG(victim);

      victim->map = -1;
      victim->coord->x = -1;
      victim->coord->y = -1;
   }
   if (immortal)
   {
      if (IS_ONMAP_FLAG(victim))
         SET_ONMAP_FLAG(immortal);
      else
         REMOVE_ONMAP_FLAG(immortal);


      immortal->map = victim->map;
      immortal->coord->x = victim->coord->x;
      immortal->coord->y = victim->coord->y;
      update_objects(immortal, immortal->coord->x, immortal->coord->y, immortal->map);
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

   update_objects(victim, victim->coord->x, victim->coord->y, victim->map);
   return;
}

/* lets the mobile force someone to do something.  must be mortal level
   and the all argument only affects those in the room with the mobile */

void do_mpforce(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      progbug("Mpforce - Bad syntax", ch);
      return;
   }

   if (!str_cmp(arg, "all"))
   {
      CHAR_DATA *vch;

      for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
         if (get_trust(vch) < get_trust(ch) && can_see(ch, vch))
            interpret(vch, argument);
   }
   // only players
   if (!str_cmp(arg, "players"))
   {
      CHAR_DATA *vch;

      for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
         if (get_trust(vch) < get_trust(ch) && can_see(ch, vch) && !IS_NPC(ch))
            interpret(vch, argument);
   }
   else
   {
      CHAR_DATA *victim;

      if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
      {
         progbug("Mpforce - No such victim", ch);
         return;
      }
      if (get_trust(victim) > LEVEL_PC)
      {
         progbug("Mpforce - Trying to force an immortal", ch);
         return;
      }
      if (victim == ch)
      {
         progbug("Mpforce - Forcing oneself", ch);
         return;
      }

      if (!IS_NPC(victim) && (!victim->desc) && IS_IMMORTAL(victim))
      {
         progbug("Mpforce - Attempting to force link dead immortal", ch);
         return;
      }


      interpret(victim, argument);
   }

   return;
}


/*
 * mpnuisance mpunnuisance just incase we need them later --Shaddai
 */

void do_mpnuisance(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg1[MSL];
   struct tm *now_time;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0')
   {
      progbug("Mpnuisance - called w/o enough argument(s)", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpnuisance: victim not in room", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      progbug("Mpnuisance: victim is a mob", ch);
      return;
   }
   if (IS_IMMORTAL(victim))
   {
      progbug("Mpnuisance: not allowed on immortals", ch);
      return;
   }
   if (victim->pcdata->nuisance)
   {
      progbug("Mpnuisance: victim is already nuisanced", ch);
      return;
   }
   CREATE(victim->pcdata->nuisance, NUISANCE_DATA, 1);
   victim->pcdata->nuisance->time = current_time;
   victim->pcdata->nuisance->flags = 1;
   victim->pcdata->nuisance->power = 2;
   now_time = localtime(&current_time);
   now_time->tm_mday += 1;
   victim->pcdata->nuisance->max_time = mktime(now_time);
   add_timer(victim, TIMER_NUISANCE, (28800 * 2), NULL, 0);
   return;
}

void do_mpunnuisance(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TIMER *timer, *timer_next;
   char arg1[MSL];

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0')
   {
      progbug("Mpunnuisance - called w/o enough argument(s)", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpunnuisance: victim not in room", ch);
      return;
   }

   if (IS_NPC(victim))
   {
      progbug("Mpunnuisance: victim was a mob", ch);
      return;
   }

   if (IS_IMMORTAL(victim))
   {
      progbug("Mpunnuisance: victim was an immortal", ch);
      return;
   }

   if (!ch->pcdata->nuisance)
   {
      progbug("Mpunnuisance: victim is not nuisanced", ch);
      return;
   }
   for (timer = victim->first_timer; timer; timer = timer_next)
   {
      timer_next = timer->next;
      if (timer->type == TIMER_NUISANCE)
         extract_timer(victim, timer);
   }
   DISPOSE(victim->pcdata->nuisance);
   return;
}

/*
 * mpbodybag for mobs to do cr's  --Shaddai
 */
void do_mpbodybag(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   char arg[MSL];
   char buf2[MSL];
   char buf3[MSL];
   char buf4[MSL];


   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpbodybag - called w/o enough argument(s)", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpbodybag: victim not in room", ch);
      return;
   }
   if (IS_NPC(victim))
   {
      progbug("Mpbodybag: bodybagging a npc corpse", ch);
      return;
   }
   sprintf(buf3, " ");
   sprintf(buf2, "the corpse of %s", arg);
   for (obj = first_object; obj; obj = obj->next)
   {
      if (obj->in_room && !str_cmp(buf2, obj->short_descr) && (obj->pIndexData->vnum == 11))
      {
         obj_from_room(obj);
         obj = obj_to_char(obj, ch);
         obj->timer = -1;
      }
   }
   /* Maybe should just make the command logged... Shrug I am not sure
    * --Shaddai
    */
   sprintf(buf4, "Mpbodybag: Grabbed %s", buf2);
   progbug(buf4, ch);
   return;
}

/*
 * mpmorph and mpunmorph for morphing people with mobs. --Shaddai
 */

void do_mpmorph(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   MORPH_DATA *morph;
   char arg1[MIL];
   char arg2[MIL];

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      progbug("Mpmorph - called w/o enough argument(s)", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpmorph: victim not in room", ch);
      return;
   }


   if (!is_number(arg2))
      morph = get_morph(arg2);
   else
      morph = get_morph_vnum(atoi(arg2));
   if (!morph)
   {
      progbug("Mpmorph - unknown morph", ch);
      return;
   }
   if (victim->morph)
   {
      progbug("Mpmorph - victim already morphed", ch);
      return;
   }
   do_morph_char(victim, morph);
   return;
}

void do_mpunmorph(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   char arg[MSL];

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpmorph - called w/o an argument", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpunmorph: victim not in room", ch);
      return;
   }
   if (!victim->morph)
   {
      progbug("Mpunmorph: victim not morphed", ch);
      return;
   }
   do_unmorph_char(victim);
   return;
}

void do_mpechozone(CHAR_DATA * ch, char *argument) /* Blod, late 97 */
{
   char arg1[MIL];
   CHAR_DATA *vch;
   CHAR_DATA *vch_next;
   sh_int color;
   EXT_BV actflags;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      progbug("Mpechozone - called w/o argument", ch);
      return;
   }
   actflags = ch->act;
   xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   if ((color = get_color(argument)))
      argument = one_argument(argument, arg1);
   for (vch = first_char; vch; vch = vch_next)
   {
      vch_next = vch->next;
      if (vch->in_room->area == ch->in_room->area && !IS_NPC(vch) && IS_AWAKE(vch))
      {
         if (color)
            act(color, argument, vch, NULL, NULL, TO_CHAR);
         else
            act(AT_ACTION, argument, vch, NULL, NULL, TO_CHAR);
      }
   }
   ch->act = actflags;
}

/*
 *  Haus' toys follow:
 */

/*
 * syntax:  mppractice victim spell_name max%
 *
 */
void do_mp_practice(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MIL];
   CHAR_DATA *victim;
   int sn, max, tmp, adept;
   char *skill_name;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);

   if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
   {
      send_to_char("Mppractice: bad syntax", ch);
      progbug("Mppractice - Bad syntax", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Mppractice: Student not in room? Invis?", ch);
      progbug("Mppractice: Invalid student not in room", ch);
      return;
   }

   if ((sn = skill_lookup(arg2)) < 0)
   {
      send_to_char("Mppractice: Invalid spell/skill name", ch);
      progbug("Mppractice: Invalid spell/skill name", ch);
      return;
   }


   if (IS_NPC(victim))
   {
      send_to_char("Mppractice: Can't train a mob", ch);
      progbug("Mppractice: Can't train a mob", ch);
      return;
   }

   skill_name = skill_table[sn]->name;

   max = atoi(arg3);
   if ((max < 0) || (max >= MAX_SKPOINTS))
   {
      sprintf(log_buf, "mp_practice: Invalid maxpercent: %d", max);
      send_to_char(log_buf, ch);
      progbug(log_buf, ch);
      return;
   }

   adept = MAX_SKPOINTS;

   if ((victim->pcdata->learned[sn] >= adept) || (victim->pcdata->learned[sn] >= max))
   {
      sprintf(buf, "$n shows some knowledge of %s, but yours is clearly superior.", skill_name);
      act(AT_TELL, buf, ch, NULL, victim, TO_VICT);
      return;
   }


   /* past here, victim learns something */
   tmp = UMIN(victim->pcdata->learned[sn] + int_app[get_curr_int(victim)].learn, max);
   act(AT_ACTION, "$N demonstrates $t to you.  You feel more learned in this subject.", victim, skill_table[sn]->name, ch, TO_CHAR);

   victim->pcdata->learned[sn] = max;


   if (victim->pcdata->learned[sn] >= adept)
   {
      victim->pcdata->learned[sn] = adept;
      act(AT_TELL, "$n tells you, 'You have learned all I know on this subject...'", ch, NULL, victim, TO_VICT);
   }
   return;

}

/*
void do_mpstrew( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    CHAR_DATA *victim;
    OBJ_DATA *obj_next;
    OBJ_DATA *obj_lose;
    ROOM_INDEX_DATA *pRoomIndex;
    int low_vnum, high_vnum, rvnum;

    set_char_color( AT_IMMORT, ch );

    if ( !IS_NPC( ch ) || ch->desc || IS_AFFECTED( ch, AFF_CHARM ))
    {
          send_to_char( "Huh?\n\r", ch );
          return;
    }

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' ) {
     send_to_char( "Mpstrew whom?\n\r", ch );
     progbug( "Mpstrew: invalid (nonexistent?) argument", ch );
     return;
    }
    if ( ( victim = get_char_room( ch, arg1 ) ) == NULL ) {
      send_to_char( "Victim must be in room.\n\r", ch );
      progbug( "Mpstrew: victim not in room", ch );
      return;
    }
    if ( IS_IMMORTAL( victim ) && get_trust( victim ) >= get_trust( ch ) ) {
      send_to_char( "You haven't the power to succeed against this victim.\n\r", ch );
      progbug( "Mpstrew: victim level too high", ch );
      return;
    }
    if ( !str_cmp( arg2, "coins"  ) ) {
      if ( victim->gold < 1) {
        send_to_char( "Drat, this one's got no gold to start with.\n\r", ch );
        return;
      }
      victim->gold = 0;
      return;
    }
    if (arg2[0] == '\0') {
      send_to_char( "You must specify a low vnum.\n\r", ch );
      progbug( "Mpstrew:  missing low vnum", ch );
      return;
    }
    if (argument[0] == '\0') {
      send_to_char( "You must specify a high vnum.\n\r", ch );
      progbug( "Mpstrew:  missing high vnum", ch );
      return;
    }
   low_vnum = atoi( arg2 ); high_vnum = atoi( argument );
    if ( low_vnum < 1 || high_vnum < low_vnum || low_vnum > high_vnum || low_vnum == high_vnum || high_vnum > 32767 ) {
        send_to_char( "Invalid range.\n\r", ch );
        progbug( "Mpstrew:  invalid range", ch );
        return;
    }
    for ( ; ; ) {
      rvnum = number_range( low_vnum, high_vnum );
      pRoomIndex = get_room_index( rvnum );
      if ( pRoomIndex )
        break;
    }
    if ( !str_cmp( arg2, "inventory" ) ) {
      for ( obj_lose=victim->first_carrying; obj_lose; obj_lose=obj_next ) {
        obj_next = obj_lose->next_content;
        obj_from_char( obj_lose );
        obj_to_room( obj_lose, rvnum );
        pager_printf_color( ch, "\t&w%s sent to %d\n\r",
 	  capitalize(obj_lose->short_descr), pRoomIndex->vnum );
      }
      return;
    }
    send_to_char( "Strew their coins or inventory?\n\r", ch );
    progbug( "Mpstrew:  no arguments", ch );
    return;
}
*/
void do_mpscatter(CHAR_DATA * ch, char *argument)
{
   char arg1[MSL];
   char arg2[MSL];
   CHAR_DATA *victim;
   ROOM_INDEX_DATA *pRoomIndex;
   int low_vnum, high_vnum, rvnum;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("Mpscatter whom?\n\r", ch);
      progbug("Mpscatter: invalid (nonexistent?) argument", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpscatter: victim not in room", ch);
      return;
   }
   if (IS_IMMORTAL(victim) && get_trust(victim) >= get_trust(ch))
   {
      send_to_char("You haven't the power to succeed against this victim.\n\r", ch);
      progbug("Mpscatter: victim level too high", ch);
      return;
   }
   if (arg2[0] == '\0')
   {
      send_to_char("You must specify a low vnum.\n\r", ch);
      progbug("Mpscatter:  missing low vnum", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("You must specify a high vnum.\n\r", ch);
      progbug("Mpscatter:  missing high vnum", ch);
      return;
   }
   low_vnum = atoi(arg2);
   high_vnum = atoi(argument);
   if (low_vnum < 1 || high_vnum < low_vnum || low_vnum > high_vnum || low_vnum == high_vnum || high_vnum > MAX_VNUM)
   {
      send_to_char("Invalid range.\n\r", ch);
      progbug("Mpscatter:  invalid range", ch);
      return;
   }
   while (1)
   {
      rvnum = number_range(low_vnum, high_vnum);
      pRoomIndex = get_room_index(rvnum);
/*    sprintf( log_buf, "Scattering.  Checking room %d..", rvnum);
      log_string( log_buf ); */
      if (pRoomIndex)
/*      if ( !xIS_SET(pRoomIndex->room_flags, ROOM_PRIVATE)
      &&   !xIS_SET(pRoomIndex->room_flags, ROOM_SOLITARY)
      &&   !xIS_SET(pRoomIndex->room_flags, ROOM_NO_ASTRAL)
      &&   !xIS_SET(pRoomIndex->room_flags, ROOM_PROTOTYPE) )
      -- still causing problems if every room in range matches
         these flags, removed for now, flag checks aren't necessary
	 for this right now anyway */
         break;
   }
   if (victim->fighting)
      stop_fighting(victim, TRUE);
   char_from_room(victim);
   char_to_room(victim, pRoomIndex);
   victim->position = POS_RESTING;
   do_look(victim, "auto");
   return;
}

/*
 * syntax: mpslay (character)
 */
void do_mp_slay(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   CHAR_DATA *victim;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   if (arg1[0] == '\0')
   {
      send_to_char("mpslay whom?\n\r", ch);
      progbug("Mpslay: invalid (nonexistent?) argument", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpslay: victim not in room", ch);
      return;
   }

   if (victim == ch)
   {
      send_to_char("You try to slay yourself.  You fail.\n\r", ch);
      progbug("Mpslay: trying to slay self", ch);
      return;
   }

   if (IS_NPC(victim) && victim->pIndexData->vnum == 3)
   {
      send_to_char("You cannot slay supermob!\n\r", ch);
      progbug("Mpslay: trying to slay supermob", ch);
      return;
   }

   if (victim->level < LEVEL_IMMORTAL)
   {
      act(AT_IMMORT, "You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
      act(AT_IMMORT, "$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
      act(AT_IMMORT, "$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
      set_cur_char(victim);
      raw_kill(ch, victim);
      stop_fighting(ch, FALSE);
      stop_hating(ch);
      stop_fearing(ch);
      stop_hunting(ch);
   }
   else
   {
      act(AT_IMMORT, "You attempt to slay $M and fail!", ch, NULL, victim, TO_CHAR);
      act(AT_IMMORT, "$n attempts to slay you.  What a kneebiter!", ch, NULL, victim, TO_VICT);
      act(AT_IMMORT, "$n attempts to slay $N.  Needless to say $e fails.", ch, NULL, victim, TO_NOTVICT);
   }
   return;
}

/*
 * syntax: mpdamage (character) (#hps)
 */
void do_mp_damage(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   CHAR_DATA *victim;
   CHAR_DATA *nextinroom;
   int dam;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (arg1[0] == '\0')
   {
      send_to_char("mpdamage whom?\n\r", ch);
      progbug("Mpdamage: invalid argument1", ch);
      return;
   }
/* Am I asking for trouble here or what?  But I need it. -- Blodkai */
   if (!str_cmp(arg1, "all"))
   {
      for (victim = ch->in_room->first_person; victim; victim = nextinroom)
      {
         nextinroom = victim->next_in_room;
         if (victim != ch && can_see(ch, victim)) /* Could go either way */
         {
            sprintf(buf, "'%s' %s", victim->name, arg2);
            do_mp_damage(ch, buf);
         }
      }
      return;
   }
   if (arg2[0] == '\0')
   {
      send_to_char("mpdamage inflict how many hps?\n\r", ch);
      progbug("Mpdamage: invalid argument2", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpdamage: victim not in room", ch);
      return;
   }
   if (victim == ch)
   {
      send_to_char("You can't mpdamage yourself.\n\r", ch);
      progbug("Mpdamage: trying to damage self", ch);
      return;
   }
   dam = atoi(arg2);
   if ((dam < 0) || (dam > 32000))
   {
      send_to_char("Mpdamage how much?\n\r", ch);
      progbug("Mpdamage: invalid (nonexistent?) argument", ch);
      return;
   }
   //Removed the simple damage junk, I blow my nose on who did that
   if (damage(victim, victim, dam, TYPE_UNDEFINED, 0, -1) == rVICT_DIED)
   {
      stop_fighting(ch, FALSE);
      stop_hating(ch);
      stop_fearing(ch);
      stop_hunting(ch);
   }
   if (IS_NPC(ch) && !IS_NPC(victim))
   {
      int per = 0;
      int level;
         
      if (LEARNED(victim, gsn_greater_focus_aggression))
      {
         level = POINT_LEVEL(LEARNED(victim, gsn_greater_focus_aggression), MASTERED(victim, gsn_greater_focus_aggression));
         per = 35+level/4;
         learn_from_success(victim, gsn_greater_focus_aggression, ch);
      }
      else if (LEARNED(victim, gsn_focus_aggression))
      {
         level = POINT_LEVEL(LEARNED(victim, gsn_focus_aggression), MASTERED(victim, gsn_focus_aggression));
         per = 20+level/5;
         learn_from_success(victim, gsn_focus_aggression, ch);
      }
      else if (LEARNED(victim, gsn_greater_draw_aggression))
      {
         level = POINT_LEVEL(LEARNED(victim, gsn_greater_draw_aggression), MASTERED(victim, gsn_greater_draw_aggression));
         per = 10+level/7;
         learn_from_success(victim, gsn_greater_draw_aggression, ch);
      }
      else if (LEARNED(victim, gsn_draw_aggression))
      {
         level = POINT_LEVEL(LEARNED(victim, gsn_draw_aggression), MASTERED(victim, gsn_draw_aggression));
         per = 2+level/10;
         learn_from_success(victim, gsn_draw_aggression, ch);
      }
      if (per > 0)
      {
         per = UMAX(1, dam * per /100);
         adjust_aggression_list(ch, victim, per, 4, -1);   
      }
   }
   return;
}

void do_mp_log(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   struct tm *t = localtime(&current_time);

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      progbug("Mp_log:  non-existent entry", ch);
      return;
   }
   sprintf(buf, "&p%-2.2d/%-2.2d | %-2.2d:%-2.2d  &P%s:  &p%s", t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, ch->short_descr, argument);
   append_to_file(MOBLOG_FILE, buf);
   return;
}

/*
 * syntax: mprestore (character) (#hps)                Gorog
 */
void do_mp_restore(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   int hp;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0')
   {
      send_to_char("mprestore whom?\n\r", ch);
      progbug("Mprestore: invalid argument1", ch);
      return;
   }

   if (arg2[0] == '\0')
   {
      send_to_char("mprestore how many hps?\n\r", ch);
      progbug("Mprestore: invalid argument2", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mprestore: victim not in room", ch);
      return;
   }

   hp = atoi(arg2);

   if ((hp < 0) || (hp > 32000))
   {
      send_to_char("Mprestore how much?\n\r", ch);
      progbug("Mprestore: invalid (nonexistent?) argument", ch);
      return;
   }
   hp += victim->hit;
   victim->hit = (hp > MAX_HPMANA || hp < 0 || hp > victim->max_hit) ? victim->max_hit : hp;
}

/*
 * Syntax mpfavor target number
 * Raise a player's favor in progs.
 */
void do_mpfavor(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHAR_DATA *victim;
   int favor;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (arg1[0] == '\0')
   {
      send_to_char("mpfavor whom?\n\r", ch);
      progbug("Mpfavor: invalid argument1", ch);
      return;
   }

   if (arg2[0] == '\0')
   {
      send_to_char("mpfavor how much favor?\n\r", ch);
      progbug("Mpfavor: invalid argument2", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("Victim must be in room.\n\r", ch);
      progbug("Mpfavor: victim not in room", ch);
      return;
   }

   favor = atoi(arg2);
   victim->pcdata->favor = URANGE(-1000, victim->pcdata->favor + favor, 1000);
}

/*
 * Syntax mp_open_passage x y z
 *
 * opens a 1-way passage from room x to room y in direction z
 *
 *  won't mess with existing exits
 */
void do_mp_open_passage(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   ROOM_INDEX_DATA *targetRoom, *fromRoom;
   int targetRoomVnum, fromRoomVnum, exit_num;
   EXIT_DATA *pexit;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);

   if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
   {
      progbug("MpOpenPassage - Bad syntax", ch);
      return;
   }

   if (!is_number(arg1))
   {
      progbug("MpOpenPassage - Bad syntax", ch);
      return;
   }

   fromRoomVnum = atoi(arg1);
   if ((fromRoom = get_room_index(fromRoomVnum)) == NULL)
   {
      progbug("MpOpenPassage - Bad syntax", ch);
      return;
   }

   if (!is_number(arg2))
   {
      progbug("MpOpenPassage - Bad syntax", ch);
      return;
   }

   targetRoomVnum = atoi(arg2);
   if ((targetRoom = get_room_index(targetRoomVnum)) == NULL)
   {
      progbug("MpOpenPassage - Bad syntax", ch);
      return;
   }

   if (!is_number(arg3))
   {
      progbug("MpOpenPassage - Bad syntax", ch);
      return;
   }

   exit_num = atoi(arg3);
   if ((exit_num < 0) || (exit_num > MAX_DIR))
   {
      progbug("MpOpenPassage - Bad syntax", ch);
      return;
   }

   if ((pexit = get_exit(fromRoom, exit_num)) != NULL)
   {
      if (!IS_SET(pexit->exit_info, EX_PASSAGE))
         return;
      progbug("MpOpenPassage - Exit exists", ch);
      return;
   }

   pexit = make_exit(fromRoom, targetRoom, exit_num);
   pexit->keyword = STRALLOC("");
   pexit->description = STRALLOC("");
   pexit->key = -1;
   pexit->exit_info = EX_PASSAGE;

   /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_CHAR ); */
   /* act( AT_PLAIN, "A passage opens!", ch, NULL, NULL, TO_ROOM ); */

   return;
}


/*
 * Syntax mp_fillin x
 * Simply closes the door
 */
void do_mp_fill_in(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   EXIT_DATA *pexit;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if ((pexit = find_door(ch, arg, TRUE)) == NULL)
   {
      progbug("MpFillIn - Exit does not exist", ch);
      return;
   }
   SET_BIT(pexit->exit_info, EX_CLOSED);
   return;
}

/*
 * Syntax mp_close_passage x y 
 *
 * closes a passage in room x leading in direction y
 *
 * the exit must have EX_PASSAGE set
 */
void do_mp_close_passage(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   ROOM_INDEX_DATA *fromRoom;
   int fromRoomVnum, exit_num;
   EXIT_DATA *pexit;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);

   if (arg1[0] == '\0' || arg2[0] == '\0' || arg2[0] == '\0')
   {
      progbug("MpClosePassage - Bad syntax", ch);
      return;
   }

   if (!is_number(arg1))
   {
      progbug("MpClosePassage - Bad syntax", ch);
      return;
   }

   fromRoomVnum = atoi(arg1);
   if ((fromRoom = get_room_index(fromRoomVnum)) == NULL)
   {
      progbug("MpClosePassage - Bad syntax", ch);
      return;
   }

   if (!is_number(arg2))
   {
      progbug("MpClosePassage - Bad syntax", ch);
      return;
   }

   exit_num = atoi(arg2);
   if ((exit_num < 0) || (exit_num > MAX_DIR))
   {
      progbug("MpClosePassage - Bad syntax", ch);
      return;
   }

   if ((pexit = get_exit(fromRoom, exit_num)) == NULL)
   {
      return; /* already closed, ignore...  so rand_progs */
      /*                            can close without spam */
   }

   if (!IS_SET(pexit->exit_info, EX_PASSAGE))
   {
      progbug("MpClosePassage - Exit not a passage", ch);
      return;
   }

   extract_exit(fromRoom, pexit);

   /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_CHAR ); */
   /* act( AT_PLAIN, "A passage closes!", ch, NULL, NULL, TO_ROOM ); */

   return;
}



/*
 * Does nothing.  Used for scripts.
 */
void do_mpnothing(CHAR_DATA * ch, char *argument)
{
   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   return;
}


/*
 *   Sends a message to sleeping character.  Should be fun
 *    with room sleep_progs
 *
 */
void do_mpdream(CHAR_DATA * ch, char *argument)
{
   char arg1[MSL];
   CHAR_DATA *vict;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   argument = one_argument(argument, arg1);

   if ((vict = get_char_world(ch, arg1)) == NULL)
   {
      progbug("Mpdream: No such character", ch);
      return;
   }

   if (vict->position <= POS_SLEEPING)
   {
      send_to_char(argument, vict);
      send_to_char("\n\r", vict);
   }
   return;
}

void do_mpapply(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      progbug("Mpapply - bad syntax", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      progbug("Mpapply - no such player in room.", ch);
      return;
   }

   if (!victim->desc)
   {
      send_to_char("Not on linkdeads.\n\r", ch);
      return;
   }

   if (!NOT_AUTHED(victim))
      return;

   if (victim->pcdata->auth_state >= 1)
      return;

   sprintf(log_buf, "%s@%s new %s %s applying...",
      victim->name, victim->desc->host, race_table[victim->race]->race_name, "(Peaceful)");
/*  log_string( log_buf );*/
   to_channel(log_buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL);
   victim->pcdata->auth_state = 1;
   return;
}

void do_mpapplyb(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;


   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      progbug("Mpapplyb - bad syntax", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      progbug("Mpapplyb - no such player in room.", ch);
      return;
   }

   if (!victim->desc)
   {
      send_to_char("Not on linkdeads.\n\r", ch);
      return;
   }

   if (!NOT_AUTHED(victim))
      return;

   if (get_timer(victim, TIMER_APPLIED) >= 1)
      return;

   switch (victim->pcdata->auth_state)
   {
      case 0:
      case 1:
      default:
         send_to_char("You attempt to regain the gods' attention.\n\r", victim);
         sprintf(log_buf, "%s@%s new %s %s applying...",
            victim->name, victim->desc->host, race_table[victim->race]->race_name, "(Peaceful)");
         log_string(log_buf);
         to_channel(log_buf, CHANNEL_AUTH, "Auth", LEVEL_IMMORTAL);
         add_timer(victim, TIMER_APPLIED, 10, NULL, 0);
         victim->pcdata->auth_state = 1;
         break;

      case 2:
         send_to_char("Your name has been deemed unsuitable by the gods.  Please choose a more medieval name with the 'name' command.\n\r", victim);
         add_timer(victim, TIMER_APPLIED, 10, NULL, 0);
         break;

      case 3:
         ch_printf(victim, "The gods permit you to enter the %s.\n\r", sysdata.mud_name);
         REMOVE_BIT(victim->pcdata->flags, PCFLAG_UNAUTHED);
         if (victim->fighting)
            stop_fighting(victim, TRUE);
         char_from_room(victim);
         char_to_room(victim, get_room_index(ROOM_VNUM_SCHOOL));
         act(AT_WHITE, "$n enters this world from within a column of blinding light!", victim, NULL, NULL, TO_ROOM);
         do_look(victim, "auto");
         break;
         
      case 4:
         send_to_char("Your last name has been deemed unsuitable by the gods.  Please choose a new one with 'lastname' command.\n\r", victim);
         add_timer(victim, TIMER_APPLIED, 10, NULL, 0);
         break;
         
   }

   return;
}

/*
 * Deposit some gold into the current area's economy		-Thoric
 */
void do_mp_deposit(CHAR_DATA * ch, char *argument)
{
   char arg[MSL];
   int gold;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpdeposit - bad syntax", ch);
      return;
   }
   gold = atoi(arg);
   if (gold <= ch->gold && ch->in_room)
   {
      ch->gold -= gold;
      boost_economy(ch->in_room->area, gold);
   }
}


/*
 * Withdraw some gold from the current area's economy		-Thoric
 */
void do_mp_withdraw(CHAR_DATA * ch, char *argument)
{
   char arg[MSL];
   int gold;

   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }


   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      progbug("Mpwithdraw - bad syntax", ch);
      return;
   }
   gold = atoi(arg);
   if (ch->gold < 1000000000 && gold < 1000000000 && ch->in_room && economy_has(ch->in_room->area, gold))
   {
      ch->gold += gold;
      lower_economy(ch->in_room->area, gold);
   }
}

void do_mpdelay(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;
   int delay;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   if (!*arg)
   {
      send_to_char("Delay for how many rounds?n\r", ch);
      progbug("Mpdelay: no duration specified", ch);
      return;
   }
   if (!(victim = get_char_room_new(ch, arg, 1)))
   {
      send_to_char("They aren't here.\n\r", ch);
      progbug("Mpdelay: target not in room", ch);
      return;
   }
   if (IS_IMMORTAL(victim))
   {
      send_to_char("Not against immortals.\n\r", ch);
      progbug("Mpdelay: target is immortal", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!*arg || !is_number(arg))
   {
      send_to_char("Delay them for how many rounds?\n\r", ch);
      progbug("Mpdelay: invalid (nonexistant?) argument", ch);
      return;
   }
   delay = atoi(arg);
   if (delay < 1 || delay > 120)
   {
      send_to_char("Argument out of range.\n\r", ch);
      progbug("Mpdelay:  argument out of range (1 to 120)", ch);
      return;
   }
   WAIT_STATE(victim, delay * 2 * PULSE_VIOLENCE);
   send_to_char("Mpdelay applied.\n\r", ch);
   return;
}

void do_mppeace(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *rch;
   CHAR_DATA *victim;

   if (!IS_NPC(ch) || ch->desc || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);
   if (!*arg)
   {
      send_to_char("Who do you want to mppeace?\n\r", ch);
      progbug("Mppeace: invalid (nonexistent?) argument", ch);
      return;
   }
   if (!str_cmp(arg, "all"))
   {
      for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
      {
         if (rch->fighting)
         {
            stop_fighting(rch, TRUE);
            do_sit(rch, "");
         }
         stop_hating(rch);
         stop_hunting(rch);
         stop_fearing(rch);
      }
      send_to_char("Ok.\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They must be in the room.n\r", ch);
      progbug("Mppeace: target not in room", ch);
      return;
   }
   if (victim->fighting)
      stop_fighting(victim, TRUE);
   stop_hating(ch);
   stop_hunting(ch);
   stop_fearing(ch);
   stop_hating(victim);
   stop_hunting(victim);
   stop_fearing(victim);
   send_to_char("Ok.\n\r", ch);
   return;
}

void do_mppkset(CHAR_DATA * ch, char *argument)
{
   return;
}
