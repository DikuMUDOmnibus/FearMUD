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
 *			   Player communication module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"

/*
#ifndef WIN32
  #include <regex.h>
#endif
  */
/*
 *  Externals
 */

/*
 * Local functions.
 */
void talk_channel args((CHAR_DATA * ch, char *argument, int channel, const char *verb));

char *scramble args((const char *argument, int modifier));
char *drunk_speech args((const char *argument, CHAR_DATA * ch));

/*
 *  Profanity handler stuff  (forgive me for what i must do)
 */
void add_profane_word(char *word);
int is_profane(char *what);
char *bigregex = NULL;
char *preg;



/* Text scrambler -- Altrag */
char *scramble(const char *argument, int modifier)
{
   static char arg[MIL];
   sh_int position;
   sh_int conversion = 0;

   modifier %= number_range(80, 300); /* Bitvectors get way too large #s */
   for (position = 0; position < MIL; position++)
   {
      if (argument[position] == '\0')
      {
         arg[position] = '\0';
         return arg;
      }
      else if (argument[position] >= 'A' && argument[position] <= 'Z')
      {
         conversion = -conversion + position - modifier + argument[position] - 'A';
         conversion = number_range(conversion - 5, conversion + 5);
         while (conversion > 25)
            conversion -= 26;
         while (conversion < 0)
            conversion += 26;
         arg[position] = conversion + 'A';
      }
      else if (argument[position] >= 'a' && argument[position] <= 'z')
      {
         conversion = -conversion + position - modifier + argument[position] - 'a';
         conversion = number_range(conversion - 5, conversion + 5);
         while (conversion > 25)
            conversion -= 26;
         while (conversion < 0)
            conversion += 26;
         arg[position] = conversion + 'a';
      }
      else if (argument[position] >= '0' && argument[position] <= '9')
      {
         conversion = -conversion + position - modifier + argument[position] - '0';
         conversion = number_range(conversion - 2, conversion + 2);
         while (conversion > 9)
            conversion -= 10;
         while (conversion < 0)
            conversion += 10;
         arg[position] = conversion + '0';
      }
      else
         arg[position] = argument[position];
   }
   arg[position] = '\0';
   return arg;
}

/* I'll rewrite this later if its still needed.. -- Altrag
char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
	return "";
}
*/

LANG_DATA *get_lang(const char *name)
{
   LANG_DATA *lng;

   for (lng = first_lang; lng; lng = lng->next)
      if (!str_cmp(lng->name, name))
         return lng;
   return NULL;
}

/* percent = percent knowing the language. */
char *translate(int percent, const char *in, const char *name)
{
   LCNV_DATA *cnv;
   static char buf[256];
   char buf2[256];
   const char *pbuf;
   char *pbuf2 = buf2;
   LANG_DATA *lng;

   if (percent > 99 || !str_cmp(name, "common"))
      return (char *) in;

   /* If we don't know this language... use "default" */
   if (!(lng = get_lang(name)))
      if (!(lng = get_lang("default")))
         return (char *) in;

   for (pbuf = in; *pbuf;)
   {
      for (cnv = lng->first_precnv; cnv; cnv = cnv->next)
      {
         if (!str_prefix(cnv->old, pbuf))
         {
            if (percent && (rand() % 100) < percent)
            {
               strncpy(pbuf2, pbuf, cnv->olen);
               pbuf2[cnv->olen] = '\0';
               pbuf2 += cnv->olen;
            }
            else
            {
               strcpy(pbuf2, cnv->new);
               pbuf2 += cnv->nlen;
            }
            pbuf += cnv->olen;
            break;
         }
      }
      if (!cnv)
      {
         if (isalpha(*pbuf) && (!percent || (rand() % 100) > percent))
         {
            *pbuf2 = lng->alphabet[LOWER(*pbuf) - 'a'];
            if (isupper(*pbuf))
               *pbuf2 = UPPER(*pbuf2);
         }
         else
            *pbuf2 = *pbuf;
         pbuf++;
         pbuf2++;
      }
   }
   *pbuf2 = '\0';
   for (pbuf = buf2, pbuf2 = buf; *pbuf;)
   {
      for (cnv = lng->first_cnv; cnv; cnv = cnv->next)
         if (!str_prefix(cnv->old, pbuf))
         {
            strcpy(pbuf2, cnv->new);
            pbuf += cnv->olen;
            pbuf2 += cnv->nlen;
            break;
         }
      if (!cnv)
         *(pbuf2++) = *(pbuf++);
   }
   *pbuf2 = '\0';
#if 0
   for (pbuf = in, pbuf2 = buf; *pbuf && *pbuf2; pbuf++, pbuf2++)
      if (isupper(*pbuf))
         *pbuf2 = UPPER(*pbuf2);
   /* Attempt to align spacing.. */
      else if (isspace(*pbuf))
         while (*pbuf2 && !isspace(*pbuf2))
            pbuf2++;
#endif
   return buf;
}


char *drunk_speech(const char *argument, CHAR_DATA * ch)
{
   const char *arg = argument;
   static char buf[MIL * 2];
   char buf1[MIL * 2];
   sh_int drunk;
   char *txt;
   char *txt1;

   if (IS_NPC(ch) || !ch->pcdata)
      return (char *) argument;

   drunk = ch->pcdata->condition[COND_DRUNK];

   if (drunk <= 0)
      return (char *) argument;

   buf[0] = '\0';
   buf1[0] = '\0';

   if (!argument)
   {
      bug("Drunk_speech: NULL argument", 0);
      return "";
   }

   /*
      if ( *arg == '\0' )
      return (char *) argument;
    */

   txt = buf;
   txt1 = buf1;

   while (*arg != '\0')
   {
      if (toupper(*arg) == 'T')
      {
         if (number_percent() < (drunk * 2)) /* add 'h' after an 'T' */
         {
            *txt++ = *arg;
            *txt++ = 'h';
         }
         else
            *txt++ = *arg;
      }
      else if (toupper(*arg) == 'X')
      {
         if (number_percent() < (drunk * 2 / 2))
         {
            *txt++ = 'c', *txt++ = 's', *txt++ = 'h';
         }
         else
            *txt++ = *arg;
      }
      else if (number_percent() < (drunk * 2 / 5)) /* slurred letters */
      {
         sh_int slurn = number_range(1, 2);
         sh_int currslur = 0;

         while (currslur < slurn)
            *txt++ = *arg, currslur++;
      }
      else
         *txt++ = *arg;

      arg++;
   };

   *txt = '\0';

   txt = buf;

   while (*txt != '\0') /* Let's mess with the string's caps */
   {
      if (number_percent() < (2 * drunk / 2.5))
      {
         if (isupper(*txt))
            *txt1 = tolower(*txt);
         else if (islower(*txt))
            *txt1 = toupper(*txt);
         else
            *txt1 = *txt;
      }
      else
         *txt1 = *txt;

      txt1++, txt++;
   };

   *txt1 = '\0';
   txt1 = buf1;
   txt = buf;

   while (*txt1 != '\0') /* Let's make them stutter */
   {
      if (*txt1 == ' ') /* If there's a space, then there's gotta be a */
      { /* along there somewhere soon */

         while (*txt1 == ' ') /* Don't stutter on spaces */
            *txt++ = *txt1++;

         if ((number_percent() < (2 * drunk / 4)) && *txt1 != '\0')
         {
            sh_int offset = number_range(0, 2);
            sh_int pos = 0;

            while (*txt1 != '\0' && pos < offset)
               *txt++ = *txt1++, pos++;

            if (*txt1 == ' ') /* Make sure not to stutter a space after */
            { /* the initial offset into the word */
               *txt++ = *txt1++;
               continue;
            }

            pos = 0;
            offset = number_range(2, 4);
            while (*txt1 != '\0' && pos < offset)
            {
               *txt++ = *txt1;
               pos++;
               if (*txt1 == ' ' || pos == offset) /* Make sure we don't stick */
               { /* A hyphen right before a space */
                  txt1--;
                  break;
               }
               *txt++ = '-';
            }
            if (*txt1 != '\0')
               txt1++;
         }
      }
      else
         *txt++ = *txt1++;
   }

   *txt = '\0';

   return buf;
}

/*
 * Generic channel function.
 *
 * Some color added to make it look kinda RoT/Romish.  Also, due to some
 * nice function in ansi.c, color selections for channels are still used
 * --Xerves 12/99
 */
void talk_channel(CHAR_DATA * ch, char *argument, int channel, const char *verb)
{
   char buf[MSL];
   char buf2[MSL];
   DESCRIPTOR_DATA *d;
   int position;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif

   if (IS_NPC(ch) && channel == CHANNEL_CLAN)
   {
      send_to_char("Mobs can't be in clans.\n\r", ch);
      return;
   }
   if (IS_NPC(ch) && channel == CHANNEL_ORDER)
   {
      send_to_char("Mobs can't be in orders.\n\r", ch);
      return;
   }

   if (IS_NPC(ch) && channel == CHANNEL_COUNCIL)
   {
      send_to_char("Mobs can't be in councils.\n\r", ch);
      return;
   }

   if (IS_NPC(ch) && channel == CHANNEL_GUILD)
   {
      send_to_char("Mobs can't be in guilds.\n\r", ch);
      return;
   }

   if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE) || wIS_SET(ch, ROOM_SILENCE))
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Hard to talk when you are GAGGED!\n\r", ch);
      act(AT_WHITE, "$n tries to say something, but $e is GAGGED, how funny!", ch, NULL, NULL, TO_NOTVICT);
      return;
   }

   if (IS_NPC(ch) && IS_AFFECTED(ch, AFF_CHARM))
   {
      if (ch->master)
         send_to_char("I don't think so...\n\r", ch->master);
      return;
   }

   if (argument[0] == '\0')
   {
      sprintf(buf, "%s what?\n\r", verb);
      buf[0] = UPPER(buf[0]);
      send_to_char(buf, ch); /* where'd this line go? */
      return;
   }

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_SILENCE))
   {
      ch_printf(ch, "You can't %s.\n\r", verb);
      return;
   }

   REMOVE_BIT(ch->deaf, channel);

   switch (channel)
   {
      default:
         set_char_color(AT_GOSSIP, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_GOSSIP, ch), argument);
         break;
      case CHANNEL_HIGH:
         set_char_color(AT_THINK, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_THINK, ch), argument);
         break;
      case CHANNEL_HIGHGOD:
         set_char_color(AT_MUSE, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_MUSE, ch), argument);
         break;
      case CHANNEL_BUILD:
         set_char_color(AT_CYAN, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_CYAN, ch), argument);
         break;
      case CHANNEL_TALKQUEST:
         set_char_color(AT_WHITE, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_WHITE, ch), argument);
         break;
      case CHANNEL_RACETALK:
         set_char_color(AT_RACETALK, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_RACETALK, ch), argument);
         break;
      case CHANNEL_KINGDOM:
         set_char_color(AT_GREY, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_GREY, ch), argument);
         break;
      case CHANNEL_WARTALK:
         set_char_color(AT_WARTALK, ch);
         ch_printf(ch, "&c&wYou [&c%s&c&w] %s'%s'\n\r", verb, char_color_str(AT_WARTALK, ch), argument);
         break;
      case CHANNEL_IMMTALK:
         sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_WHITE, ch));
         position = ch->position;
         ch->position = POS_STANDING;
         act(AT_WHITE, buf, ch, argument, NULL, TO_CHAR);
         ch->position = position;
         break;
      case CHANNEL_AVTALK:
      case CHANNEL_IMMREMINDER:
         sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_IMMORT, ch));
         position = ch->position;
         ch->position = POS_STANDING;
         act(AT_IMMORT, buf, ch, argument, NULL, TO_CHAR);
         ch->position = position;
         break;
   }

   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf2, "%s: %s (%s)", IS_NPC(ch) ? ch->short_descr : ch->name, argument, verb);
      append_to_file(LOG_FILE, buf2);
   }


#ifdef HMM
   if (is_profane(argument))
   {
      sprintf(buf2, "%s Profanity warning: %s: %s (%s)", "say", IS_NPC(ch) ? ch->short_descr : ch->name, argument, verb);
      /* force Puff mpat 6 mpforce imp mpat 1 say hi */

      puff = get_char_world(ch, "Puff");
      if (puff != NULL)
      {
         if ((location = get_room_index(1)) != NULL)
         {
            original = puff->in_room;
            char_from_room(puff);
            char_to_room(puff, location);
            interpret(puff, buf2);
            char_to_room(puff, original);
         }
      }
   }
#endif

   for (d = first_descriptor; d; d = d->next)
   {
      CHAR_DATA *och;
      CHAR_DATA *vch;

      och = d->original ? d->original : d->character;
      vch = d->character;

      switch (channel)
      {
         default:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_GOSSIP, vch));
            break;
         case CHANNEL_HIGH:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_THINK, vch));
            break;
         case CHANNEL_HIGHGOD:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_MUSE, vch));
            break;
         case CHANNEL_BUILD:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_CYAN, vch));
            break;
         case CHANNEL_TALKQUEST:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_WHITE, vch));
            break;
         case CHANNEL_RACETALK:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_RACETALK, vch));
            break;
         case CHANNEL_KINGDOM:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_GREY, vch));
            break;
         case CHANNEL_WARTALK:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_WARTALK, vch));
            break;
         case CHANNEL_IMMTALK:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_WHITE, vch));
            break;
         case CHANNEL_AVTALK:
         case CHANNEL_IMMREMINDER:
            sprintf(buf, "&c&w$n [&c%ss&c&w] %s'$t'", verb, char_color_str(AT_IMMORT, vch));
            break;
      }

      if (d->connected == CON_PLAYING && vch != ch && !IS_SET(och->deaf, channel))
      {
         char *sbuf = argument;
         char lbuf[MIL + 4]; /* invis level string + buf */

         if (channel == CHANNEL_IMMTALK && !IS_IMMORTAL(och))
            continue;
         if (channel == CHANNEL_WARTALK && NOT_AUTHED(och))
            continue;
         if (channel == CHANNEL_AVTALK && player_stat_worth(och) < 100000)
            continue;
         if (channel == CHANNEL_KINGDOM)
         {
            int cht;
            int vht;
            
            if (IS_NPC(ch))
            {
               if (xIS_SET(ch->act, ACT_MILITARY))
                  cht = ch->m4;
               else
                  cht = -1;
            }
            else
            {
               cht = ch->pcdata->hometown;
            }
            if (IS_NPC(och))
            {
               if (xIS_SET(och->act, ACT_MILITARY))
                  vht = och->m4;
               else
                  vht = -1;
            }
            else
            {
               vht = och->pcdata->hometown;
            }
            if (vht == -1 || cht == -1 || vht != cht)
               continue;
         }
         if (channel == CHANNEL_HIGHGOD && get_trust(och) < sysdata.muse_level)
            continue;
         if (channel == CHANNEL_HIGH && get_trust(och) < sysdata.think_level)
            continue;

         /* Fix by Narn to let newbie council members see the newbie channel. */
         if (channel == CHANNEL_NEWBIE &&
            (!IS_IMMORTAL(och) && get_trust(och) > 2 && !(och->pcdata->council && !str_cmp(och->pcdata->council->name, "Newbie Council"))))
            continue;
         if (xIS_SET(vch->in_room->room_flags, ROOM_SILENCE) || wIS_SET(vch, ROOM_SILENCE))
            continue;

         if (ch->coord->x > -1 || ch->coord->y > -1 || ch->map > -1)
         {
            if (channel == CHANNEL_YELL
               && (abs(ch->coord->x - vch->coord->x) > 30 || abs(ch->coord->y - vch->coord->y) > 30 || ch->map != vch->map))
               continue;
         }
         else
         {
            if (channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area)
               continue;
         }

         if (channel == CHANNEL_CLAN || channel == CHANNEL_ORDER || channel == CHANNEL_GUILD)
         {
            if (IS_NPC(vch))
               continue;
            if (vch->pcdata->clan != ch->pcdata->clan)
               continue;
         }

         if (channel == CHANNEL_COUNCIL)
         {
            if (IS_NPC(vch))
               continue;
            if (vch->pcdata->council != ch->pcdata->council)
               continue;
         }


         if (channel == CHANNEL_RACETALK)
            if (vch->race != ch->race)
               continue;

         if (xIS_SET(ch->act, PLR_WIZINVIS) && can_see_map(vch, ch) && IS_IMMORTAL(vch))
         {
            sprintf(lbuf, "(%d) ", (!IS_NPC(ch)) ? ch->pcdata->wizinvis : ch->mobinvis);
         }
         else
         {
            lbuf[0] = '\0';
         }

         position = vch->position;
         if (channel != CHANNEL_SHOUT && channel != CHANNEL_YELL)
            vch->position = POS_STANDING;
#ifndef SCRAMBLE
         if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
         {
            int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
               knows_language(ch, ch->speaking, vch));

            if (speakswell < 85)
               sbuf = translate(speakswell, argument, lang_names[speaking]);
         }
#else
         if (!knows_language(vch, ch->speaking, ch) && (!IS_NPC(ch) || ch->speaking != 0))
            sbuf = scramble(argument, ch->speaking);
#endif
         /*  Scramble speech if vch or ch has nuisance flag */

         if (!IS_NPC(ch) && ch->pcdata->nuisance
            && ch->pcdata->nuisance->flags > 7 && (number_percent() < ((ch->pcdata->nuisance->flags - 7) * 10 * ch->pcdata->nuisance->power)))
            sbuf = scramble(argument, number_range(1, 10));

         if (!IS_NPC(vch) && vch->pcdata->nuisance &&
            vch->pcdata->nuisance->flags > 7 && (number_percent() < ((vch->pcdata->nuisance->flags - 7) * 10 * vch->pcdata->nuisance->power)))
            sbuf = scramble(argument, number_range(1, 10));

         MOBtrigger = FALSE;
         if (channel == CHANNEL_AVTALK || channel == CHANNEL_IMMREMINDER)
            act(AT_IMMORT, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_IMMTALK)
            act(AT_WHITE, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_WARTALK)
            act(AT_WARTALK, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_KINGDOM)
            act(AT_GREY, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_RACETALK)
            act(AT_RACETALK, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_HIGH)
            act(AT_THINK, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_HIGHGOD)
            act(AT_MUSE, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_BUILD)
            act(AT_CYAN, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else if (channel == CHANNEL_TALKQUEST)
            act(AT_WHITE, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         else
            act(AT_GOSSIP, strcat(lbuf, buf), ch, sbuf, vch, TO_VICT);
         vch->position = position;
      }
   }

   /* too much system degradation with 300+ players not to charge 'em a bit */
   /* 600 players now, but waitstate on clantalk is bad for pkillers */
   if ((ch->level < LEVEL_IMMORTAL) && (channel != CHANNEL_WARTALK) && (channel != CHANNEL_CLAN))
      WAIT_STATE(ch, 4);

   return;
}

void to_channel(const char *argument, int channel, const char *verb, sh_int level)
{
   char buf[MSL];
   DESCRIPTOR_DATA *d;

   if (!first_descriptor || argument[0] == '\0')
      return;

   sprintf(buf, "%s: %s\r\n", verb, argument);

   for (d = first_descriptor; d; d = d->next)
   {
      CHAR_DATA *och;
      CHAR_DATA *vch;

      och = d->original ? d->original : d->character;
      vch = d->character;

      if (!och || !vch)
         continue;
         
      if (channel == CHANNEL_AUTH)
      {
         if (d->connected == CON_PLAYING && !IS_SET(och->deaf, channel) && vch->pcdata
         &&  vch->pcdata->council && !str_cmp(vch->pcdata->council->name, "Newbie Council"))
         {
            set_char_color(AT_LOG, vch);
            send_to_char_color(buf, vch);   
            continue;
         }
      }
      if (!IS_IMMORTAL(vch)
         || (get_trust(vch) < sysdata.build_level && channel == CHANNEL_BUILD)
         || (get_trust(vch) < sysdata.log_level
&& (channel == CHANNEL_LOG || channel == CHANNEL_HIGH || channel == CHANNEL_WARN || channel == CHANNEL_COMM)))
         continue;

      if (d->connected == CON_PLAYING && !IS_SET(och->deaf, channel) && get_trust(vch) >= level)
      {
         set_char_color(AT_LOG, vch);
         send_to_char_color(buf, vch);
      }
   }

   return;
}

void do_chistory(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   CHANNEL_HISTORY *chistory;
   INTRO_DATA *intro;
   int cnt = 0;
   int scount;
   int channel;
   int count;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  chistory <channel> [count] [name of person]\n\r", ch);
      if (IS_IMMORTAL(ch))
         send_to_char("            current channels:  chat tell say kingdom newbie avtalk immortal\n\r", ch);
      else   
         send_to_char("            current channels:  chat tell say kingdom newbie avtalk\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (!str_cmp(arg1, "chat"))
      channel = CHANNEL_CHAT;
   else if (!str_cmp(arg1, "newbie"))
      channel = CHANNEL_NEWBIE;
   else if (!str_cmp(arg1, "avtalk") && player_stat_worth(ch) >= 100000)
      channel = CHANNEL_AVTALK;
   else if (!str_cmp(arg1, "kingdom"))
      channel = CHANNEL_KINGDOM;
   else if (!str_cmp(arg1, "immortal") && IS_IMMORTAL(ch))
      channel = CHANNEL_IMMTALK;
   else if (!str_cmp(arg1, "tell"))
      channel = CHANNEL_TELLS;
   else if (!str_cmp(arg1, "say"))
      channel = -1;
   else
   {
      do_chistory(ch, "");
      return;
   }
   if (atoi(arg2) <= 0)
      count = 100;
   else
      count = atoi(arg2);
      
     
   if (channel == CHANNEL_CHAT || channel == CHANNEL_KINGDOM || channel == CHANNEL_IMMTALK || channel == CHANNEL_NEWBIE
   ||  channel == CHANNEL_AVTALK)
   { 
      for (chistory = first_channelhistory; chistory; chistory = chistory->next)
      {
         if (chistory->channel == channel)
         {
            if (chistory->kpid < 0 || (IN_VALID_KINGDOM(ch->pcdata->hometown) 
            &&  chistory->kpid == kingdom_table[ch->pcdata->hometown]->kpid))
            {
               if (argument[0] == '\0' || !str_cmp(argument, chistory->sender))
               {
                  cnt++;
               }
            }
         }
      }
      scount = cnt - count;
      cnt = 0;
      for (chistory = first_channelhistory; chistory; chistory = chistory->next)
      {
         if (chistory->channel == channel)
         {
            if (chistory->kpid < 0 || (IN_VALID_KINGDOM(ch->pcdata->hometown) 
            &&  chistory->kpid == kingdom_table[ch->pcdata->hometown]->kpid))
            {
               if (argument[0] == '\0' || !str_cmp(argument, chistory->sender))
               {
                  cnt++;
                  if (cnt <= scount)
                     continue;
                  if (IS_SET(chistory->flags, CHISTORY_CLOAKED))
                  {
                     if (xIS_SET(ch->act, PLR_SHOWNAMES))
                        pager_printf(ch, "%s: %s\n\r", chistory->sender, chistory->text);
                     else
                        pager_printf(ch, "Cloaked: %s\n\r", chistory->text);
                     continue;
                  }
                  if (IS_SET(chistory->flags, CHISTORY_WIZINVIS) && ch->level < chistory->level)
                  {
                     pager_printf(ch, "Wizinvis: %s\n\r", chistory->text);
                     continue;
                  }
                  if (IS_SET(chistory->flags, CHISTORY_INVIS) && !xIS_SET(ch->affected_by, AFF_DETECT_INVIS) &&
                      !xIS_SET(ch->affected_by, AFF_TRUESIGHT))
                  {
                     if (IS_NPC(ch) || (!IS_NPC(ch) && !xIS_SET(ch->act, PLR_HOLYLIGHT)))
                     {
                        pager_printf(ch, "Invis: %s\n\r", chistory->text);
                        continue;
                     }
                  }
                  if (chistory->pid == -1 || xIS_SET(ch->act, PLR_SHOWNAMES) || !str_cmp(ch->name, chistory->sender)
                  ||  IS_SET(chistory->flags, CHISTORY_FAMOUS))
                  {
                     pager_printf(ch, "%s: %s\n\r", chistory->sender, chistory->text);
                     continue;
                  }
                  for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
                  {
                     if (intro->pid == chistory->pid && abs(chistory->pid) > 100000)
                     {
                        pager_printf(ch, "%s: %s\n\r", chistory->sender, chistory->text);
                        break;
                     }
                  }
                  if (!intro)
                     pager_printf(ch, "Unknown: %s\n\r", chistory->text);
                  continue;
               }
            }
         }
      }
   }
   else
   {
      for (chistory = ch->pcdata->first_messagehistory; chistory; chistory = chistory->next)
      {
         if (chistory->channel == channel)
         {
            if (argument[0] == '\0' || !str_cmp(argument, chistory->sender))
            {
               cnt++;
            }
         }
      }
      scount = cnt - count;
      cnt = 0;
      for (chistory = ch->pcdata->first_messagehistory; chistory; chistory = chistory->next)
      {
         if (chistory->channel == channel)
         {
            if (argument[0] == '\0' || !str_cmp(argument, chistory->sender))
            {
               cnt++;
               if (cnt <= scount)
                  continue;
               if (IS_SET(chistory->flags, CHISTORY_CLOAKED))
               {
                  if (xIS_SET(ch->act, PLR_SHOWNAMES))
                     pager_printf(ch, "%s: %s\n\r", chistory->sender, chistory->text);
                  else
                     pager_printf(ch, "Cloaked: %s\n\r", chistory->text);
                  continue;
               }
               if (IS_SET(chistory->flags, CHISTORY_WIZINVIS) && ch->level < chistory->level)
               {
                  pager_printf(ch, "Wizinvis: %s\n\r", chistory->text);
                  continue;
               }
               if (IS_SET(chistory->flags, CHISTORY_INVIS) && !xIS_SET(ch->affected_by, AFF_DETECT_INVIS) &&
                   !xIS_SET(ch->affected_by, AFF_TRUESIGHT))
               {
                  if (IS_NPC(ch) || (!IS_NPC(ch) && !xIS_SET(ch->act, PLR_HOLYLIGHT)))
                  {
                     pager_printf(ch, "Invis: %s\n\r", chistory->text);
                     continue;
                  }
               }
               if (chistory->pid == -1 || xIS_SET(ch->act, PLR_SHOWNAMES) || !str_cmp(ch->name, chistory->sender)
               ||  IS_SET(chistory->flags, CHISTORY_FAMOUS))
               {
                  pager_printf(ch, "%s: %s\n\r", chistory->sender, chistory->text);
                  continue;
               }
               for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
               {
                  if (intro->pid == chistory->pid && abs(chistory->pid) > 100000)
                  {
                     pager_printf(ch, "%s: %s\n\r", chistory->sender, chistory->text);
                     break;
                  }
               }
               if (!intro)
                  pager_printf(ch, "Unknown: %s\n\r", chistory->text);
               continue;
            }
         }
      }
   }   
}
                  
void create_channel_history(CHAR_DATA *ch, char *argument, int channel, CHAR_DATA *victim)
{
   CHANNEL_HISTORY *chistory;
   CHANNEL_HISTORY *rchannel;
   int count = 0;
   
   if (IS_NPC(ch))
   {
      if (!victim)
         return;
      if (IS_NPC(victim))
         return;
   }
   CREATE(chistory, CHANNEL_HISTORY, 1);
   if (channel == CHANNEL_CHAT || channel == CHANNEL_KINGDOM || channel == CHANNEL_IMMTALK || channel == CHANNEL_NEWBIE
   ||  channel == CHANNEL_AVTALK)
   {
      for (rchannel = first_channelhistory; rchannel; rchannel = rchannel->next)
      {
         count++;
      }
      if (count >= 700)
      {
         rchannel = first_channelhistory;
         
         UNLINK(rchannel, first_channelhistory, last_channelhistory, next, prev);  
         STRFREE(rchannel->sender);
         STRFREE(rchannel->text);
         DISPOSE(rchannel);
      }
      LINK(chistory, first_channelhistory, last_channelhistory, next, prev);
   }
   else
   {
      if (IS_NPC(ch) || (victim && IS_NPC(victim)))
      {
         DISPOSE(chistory);
         return;
      }
      else
      {
         if (victim)
         {
            for (rchannel = victim->pcdata->first_messagehistory; rchannel; rchannel = rchannel->next)
            {
               count++;
            }
            if (count >= 200)
            {
               rchannel = victim->pcdata->first_messagehistory;
         
               UNLINK(rchannel, victim->pcdata->first_messagehistory, victim->pcdata->last_messagehistory, next, prev);  
               STRFREE(rchannel->sender);
               STRFREE(rchannel->text);
               DISPOSE(rchannel);
            }
            LINK(chistory, victim->pcdata->first_messagehistory, victim->pcdata->last_messagehistory, next, prev);
         }
         else
         {
            for (rchannel = ch->pcdata->first_messagehistory; rchannel; rchannel = rchannel->next)
            {
               count++;
            }
            if (count >= 200)
            {
               rchannel = ch->pcdata->first_messagehistory;
         
               UNLINK(rchannel, ch->pcdata->first_messagehistory, ch->pcdata->last_messagehistory, next, prev);  
               STRFREE(rchannel->sender);
               STRFREE(rchannel->text);
               DISPOSE(rchannel);
            }
            LINK(chistory, ch->pcdata->first_messagehistory, ch->pcdata->last_messagehistory, next, prev);
         }
      }
   }
   chistory->channel = channel;
   chistory->sender = STRALLOC(ch->name);
   chistory->text = STRALLOC(argument);
   if (IS_NPC(ch))
      chistory->pid = -1;
   else
      chistory->pid = ch->pcdata->pid;
   if (chistory->channel == CHANNEL_KINGDOM && !IS_NPC(ch))
      chistory->kpid = kingdom_table[ch->pcdata->hometown]->kpid;
   else
      chistory->kpid = -1;
   if (get_wear_hidden_cloak(ch))
      SET_BIT(chistory->flags, CHISTORY_CLOAKED);
   if (!IS_NPC(ch) && IS_IMMORTAL(ch) && ch->pcdata->wizinvis > 1 && xIS_SET(ch->act, PLR_WIZINVIS))
   {
      SET_BIT(chistory->flags, CHISTORY_WIZINVIS);
      chistory->level = ch->pcdata->wizinvis;
   }
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOBINVIS))
   {
      SET_BIT(chistory->flags, CHISTORY_WIZINVIS);
      chistory->level = 2;
   }
   if (IS_AFFECTED(ch, AFF_INVISIBLE))
      SET_BIT(chistory->flags, CHISTORY_INVIS);
   if (ch->fame >= 10000 || (!IS_NPC(ch) && xIS_SET(ch->act, PLR_UKNOWN)))
      SET_BIT(chistory->flags, CHISTORY_FAMOUS);
}      

void do_chat(CHAR_DATA * ch, char *argument)
{
   
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_CHAT, "chat");
   create_channel_history(ch, argument, CHANNEL_CHAT, NULL);
   return;
}

void do_gocial(CHAR_DATA * ch, char *command, char *argument)
{
   char arg[MIL], buf[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   SOCIALTYPE *social;

   if ((social = find_social(command)) == NULL)
   {
      send_to_char("That is not a social.\n\r", ch);
      return;
   }

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_NO_EMOTE))
   {
      send_to_char("You are anti-social!\n\r", ch);
      return;
   }

   switch (ch->position)
   {
      case POS_DEAD:
         send_to_char("Lie still; you are DEAD.\n\r", ch);
         return;
      case POS_INCAP:
      case POS_MORTAL:
         send_to_char("You are hurt far too bad for that.\n\r", ch);
         return;
      case POS_STUNNED:
         send_to_char("You are too stunned to do that.\n\r", ch);
         return;
      case POS_SLEEPING:
         /*
            * I just know this is the path to a 12" 'if' statement.  :(
            * But two players asked for it already!  -- Furey
          */
         if (!str_cmp(social->name, "snore"))
            break;
         send_to_char("In your dreams, or what?\n\r", ch);
         return;
   }

   one_argument(argument, arg);
   victim = NULL;

   if (arg[0] == '\0')
   {
      send_to_char("You should send it to an actual person.\n\r", ch);
      return;
   }

   if ((victim = get_char_world(ch, arg)) == NULL)
   {
      send_to_char("You really shouldn't talk about people who aren't logged in.\n\r", ch);
      return;
   }

   if (victim == ch)
   {
      sprintf(buf, "[GSOCIAL] %s", social->char_auto);
      act(AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
      return;
   }
   else
   {
      sprintf(buf, "[GSOCIAL] %s", social->char_found);
      act(AT_SOCIAL, buf, ch, NULL, victim, TO_CHAR);
      sprintf(buf, "[GSOCIAL] %s", social->vict_found);
      act(AT_SOCIAL, buf, ch, NULL, victim, TO_VICT);
      return;
   }
}

void do_gsocial(CHAR_DATA * ch, char *argument)
{
   char arg[MIL], buf[MAX_INPUT_LENGTH];
   int loop;

   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '@')
   {
      argument = one_argument(argument, arg);
      for (loop = 0; loop < strlen(arg); loop++)
      {
         buf[loop] = arg[loop + 1];
      }
      do_gocial(ch, buf, argument);
      return;
   }
}

void do_kingdomtalk(CHAR_DATA * ch, char *argument)
{
   int ht;

   ht = ch->pcdata->hometown;
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_KINGDOM, "kingdomtalk");
   create_channel_history(ch, argument, CHANNEL_KINGDOM, NULL);
   return;
}

void do_clantalk(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || !ch->pcdata->clan || ch->pcdata->clan->clan_type == CLAN_ORDER || ch->pcdata->clan->clan_type == CLAN_GUILD)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_CLAN, "clantalk");
   return;
}

void do_newbiechat(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch) || (get_trust(ch) > 2 && !IS_IMMORTAL(ch) && !(ch->pcdata->council && !str_cmp(ch->pcdata->council->name, "Newbie Council"))))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_NEWBIE, "newbiechat");
   create_channel_history(ch, argument, CHANNEL_NEWBIE, NULL);
   return;
}

void do_ot(CHAR_DATA * ch, char *argument)
{
   do_ordertalk(ch, argument);
}

void do_ordertalk(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || !ch->pcdata->clan || ch->pcdata->clan->clan_type != CLAN_ORDER)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_ORDER, "ordertalk");
   return;
}

void do_counciltalk(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || !ch->pcdata->council)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_COUNCIL, "counciltalk");
   return;
}

void do_guildtalk(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   if (IS_NPC(ch) || !ch->pcdata->clan || ch->pcdata->clan->clan_type != CLAN_GUILD)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_GUILD, "guildtalk");
   return;
}

void do_music(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_MUSIC, "music");
   return;
}

/*
 * Just pop this into act_comm.c somewhere. (Or anywhere else)
 * It's pretty much say except modified to take args.
 *
 * Written by Kratas (moon@deathmoon.com)
 */

void do_say_to_char(CHAR_DATA * ch, char *argument)
{
   char arg[MIL], last_char;
   char buf[MSL];
   CHAR_DATA *vch;
   CHAR_DATA *victim;
   EXT_BV actflags;
   int arglen;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif
   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Say what to whom?\n\r", ch);
      return;
   }
   if ((victim = get_char_world(ch, arg)) == NULL
      || (IS_NPC(victim) && victim->in_room != ch->in_room) || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch)))
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Hard to talk when you are GAGGED!\n\r", ch);
      act(AT_WHITE, "$n tries to say something, but $e is GAGGED, how funny!", ch, NULL, NULL, TO_NOTVICT);
      return;
   }

   if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE) || wIS_SET(ch, ROOM_SILENCE))
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }

   arglen = strlen(argument) - 1;
   /* Remove whitespace and tabs. */
   while (argument[arglen] == ' ' || argument[arglen] == '\t')
      --arglen;
   last_char = argument[arglen];

   actflags = ch->act;
   if (IS_NPC(ch))
      xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
   {
      char *sbuf = argument;

      if (vch == ch)
         continue;

      /* Check to see if a player on a map is at the same coords as the recipient */
      if (IS_ONMAP_FLAG(ch))
      {
         if (vch->coord->x != ch->coord->x || vch->coord->y != ch->coord->y)
            continue;
      }

      /* Check to see if character is ignoring speaker */
      if (is_ignoring(vch, ch))
      {
         /* continue unless speaker is an immortal */
         if (!IS_IMMORTAL(ch) || get_trust(vch) > get_trust(ch))
            continue;
         else
         {
            set_char_color(AT_IGNORE, vch);
            ch_printf(vch, "You attempt to ignore %s, but" " are unable to do so.\n\r", PERS_MAP(ch, vch));
         }
      }

#ifndef SCRAMBLE
      if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
      {
         int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
            knows_language(ch, ch->speaking, vch));

         if (speakswell < 75)
            sbuf = translate(speakswell, argument, lang_names[speaking]);
      }
#else
      if (!knows_language(vch, ch->speaking, ch) && (!IS_NPC(ch) || ch->speaking != 0))
         sbuf = scramble(argument, ch->speaking);
#endif
      sbuf = drunk_speech(sbuf, ch);
      create_channel_history(ch, argument, -1, vch);
      MOBtrigger = FALSE;

   }
   ch->act = actflags;
   MOBtrigger = FALSE;

   switch (last_char)
   {
      case '?':
         act(AT_SAY, "You ask $N, '$t&c'", ch, drunk_speech(argument, ch), victim, TO_CHAR);
         act(AT_SAY, "$n asks $N, '$t&c'", ch, drunk_speech(argument, ch), victim, TO_NOTVICT);
         act(AT_SAY, "$n asks you '$t&c'", ch, drunk_speech(argument, ch), victim, TO_VICT);
         break;

      case '!':
         act(AT_SAY, "You exclaim at $N, '$t&c'", ch, drunk_speech(argument, ch), victim, TO_CHAR);
         act(AT_SAY, "$n exclaims at $N, '$t&c'", ch, drunk_speech(argument, ch), victim, TO_NOTVICT);
         act(AT_SAY, "$n exclaims to you, '$t&c'", ch, drunk_speech(argument, ch), victim, TO_VICT);
         break;

      default:
         act(AT_SAY, "You say to $N '$t&c'", ch, drunk_speech(argument, ch), victim, TO_CHAR);
         act(AT_SAY, "$n says to $N '$t&c'", ch, drunk_speech(argument, ch), victim, TO_NOTVICT);
         act(AT_SAY, "$n says to you '$t&c'", ch, drunk_speech(argument, ch), victim, TO_VICT);
         break;
   }
   create_channel_history(ch, argument, -1, NULL);
   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf, "%s: %s", IS_NPC(ch) ? ch->short_descr : ch->name, argument);
      append_to_file(LOG_FILE, buf);
   }
   mprog_speech_trigger(argument, ch);
   if (char_died(ch))
      return;
   oprog_speech_trigger(argument, ch);
   if (char_died(ch))
      return;
   rprog_speech_trigger(argument, ch);
   return;
}

void do_talkquest(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_TALKQUEST, "talkquest");
   return;
}

void do_ask(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_ASK, "ask");
   return;
}



void do_answer(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_ASK, "answer");
   return;
}



void do_shout(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, drunk_speech(argument, ch), CHANNEL_SHOUT, "shout");
   WAIT_STATE(ch, 12);
   return;
}



void do_yell(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, drunk_speech(argument, ch), CHANNEL_YELL, "yell");
   return;
}



void do_immtalk(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   talk_channel(ch, argument, CHANNEL_IMMTALK, "immtalk");
   create_channel_history(ch, argument, CHANNEL_IMMTALK, NULL);
   return;
}

/* For reminder channel */
void do_immreminder(CHAR_DATA * ch, char *argument)
{
   if (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   talk_channel(ch, argument, CHANNEL_IMMREMINDER, "remind");
   return;
}


void do_muse(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_HIGHGOD, "muse");
   return;
}


void do_think(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_HIGH, "think");
   return;
}


void do_avtalk(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot avtalk.\n\r", ch);
      return;
   }
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (player_stat_worth(ch) < 100000)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }

   talk_channel(ch, drunk_speech(argument, ch), CHANNEL_AVTALK, "avtalk");
   create_channel_history(ch, argument, CHANNEL_AVTALK, NULL);
   return;
}


void do_say(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   CHAR_DATA *vch;
   EXT_BV actflags;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif

   if (argument[0] == '\0')
   {
      send_to_char("Say what?\n\r", ch);
      return;
   }

   if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE) || wIS_SET(ch, ROOM_SILENCE))
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Hard to talk when you are GAGGED!\n\r", ch);
      act(AT_WHITE, "$n tries to say something, but $e is GAGGED, how funny!", ch, NULL, NULL, TO_NOTVICT);
      return;
   }

   actflags = ch->act;
   if (IS_NPC(ch))
      xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
   {
      char *sbuf = argument;

      if (vch == ch)
         continue;

      /* Check to see if a player on a map is at the same coords as the recipient */
      if (IS_ONMAP_FLAG(ch))
      {
         if (vch->coord->x != ch->coord->x || vch->coord->y != ch->coord->y)
            continue;
      }

      /* Check to see if character is ignoring speaker */
      if (is_ignoring(vch, ch))
      {
         /* continue unless speaker is an immortal */
         if (!IS_IMMORTAL(ch) || get_trust(vch) > get_trust(ch))
            continue;
         else
         {
            set_char_color(AT_IGNORE, vch);
            ch_printf(vch, "You attempt to ignore %s, but" " are unable to do so.\n\r", PERS_MAP(ch, vch));
         }
      }

#ifndef SCRAMBLE
      if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
      {
         int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
            knows_language(ch, ch->speaking, vch));

         if (speakswell < 75)
            sbuf = translate(speakswell, argument, lang_names[speaking]);
      }
#else
      if (!knows_language(vch, ch->speaking, ch) && (!IS_NPC(ch) || ch->speaking != 0))
         sbuf = scramble(argument, ch->speaking);
#endif
      sbuf = drunk_speech(sbuf, ch);

      MOBtrigger = FALSE;
      act(AT_SAY, "$n says '$t'", ch, sbuf, vch, TO_VICT);
      create_channel_history(ch, argument, -1, vch);
   }
/*    MOBtrigger = FALSE;
    act( AT_SAY, "$n says '$T'", ch, NULL, argument, TO_ROOM );*/
   ch->act = actflags;
   MOBtrigger = FALSE;
   create_channel_history(ch, argument, -1, NULL);
   act(AT_SAY, "You say '$T'", ch, NULL, drunk_speech(argument, ch), TO_CHAR);
   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf, "%s: %s", IS_NPC(ch) ? ch->short_descr : ch->name, argument);
      append_to_file(LOG_FILE, buf);
   }
   mprog_speech_trigger(argument, ch);
   if (char_died(ch))
      return;
   oprog_speech_trigger(argument, ch);
   if (char_died(ch))
      return;
   rprog_speech_trigger(argument, ch);
   return;
}

/* Installed by Samson on unknown date, allows user to beep other users */
void do_beep(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   argument = one_argument(argument, arg);

   if (!*arg || !(victim = get_char_world(ch, arg)))
   {
      send_to_char("Beep who?\n\r", ch);
      return;
   }

   /* NPC check added by Samson 2-15-98 */
   if (IS_NPC(victim))
   {
      send_to_char("Beep who?\n\r", ch);
      return;
   }

   /* PCFLAG_NOBEEP check added by Samson 2-15-98 */
   if (IS_SET(victim->pcdata->flags, PCFLAG_NOBEEP))
   {
      ch_printf(ch, "%s is not accepting beeps at this time.\n\r", PERS_MAP(victim, ch));
      return;
   }

   ch_printf(victim, "%s is beeping you!\a\n\r", PERS_MAP(ch, victim));
   ch_printf(ch, "You beep %s.\n\r", PERS_MAP(victim, ch));
   return;
}

void do_whisper(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MIL];
   CHAR_DATA *victim;
   int position;
   int speaking = -1, lang;

#ifndef SCRAMBLE

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif

   REMOVE_BIT(ch->deaf, CHANNEL_WHISPER);

   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Whisper to whom what?\n\r", ch);
      return;
   }

   if (IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Hard to talk when you are GAGGED!\n\r", ch);
      act(AT_WHITE, "$n tries to say something, but $e is GAGGED, how funny!", ch, NULL, NULL, TO_NOTVICT);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (ch == victim)
   {
      send_to_char("You have a nice little chat with yourself.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && (victim->switched) && !IS_AFFECTED(victim->switched, AFF_POSSESS))
   {
      send_to_char("That player is switched.\n\r", ch);
      return;
   }
   else if (!IS_NPC(victim) && (!victim->desc))
   {
      send_to_char("That player is link-dead.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK))
   {
      send_to_char("That player is afk.\n\r", ch);
      return;
   }
   if (IS_SET(victim->deaf, CHANNEL_WHISPER) && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
   {
      act(AT_PLAIN, "$E has $S whispers turned off.", ch, NULL, victim, TO_CHAR);
      return;
   }
   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_SILENCE))
      send_to_char("That player is silenced.  They will receive your message but can not respond.\n\r", ch);

   if (victim->desc /* make sure desc exists first  -Thoric */
      && victim->desc->connected == CON_EDITING && get_trust(ch) < LEVEL_IMM)
   {
      act(AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR);
      return;
   }

   /* Check to see if target of tell is ignoring the sender */
   if (is_ignoring(victim, ch))
   {
      /* If the sender is an imm then they cannot be ignored */
      if (!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
      {
         set_char_color(AT_IGNORE, ch);
         ch_printf(ch, "%s is ignoring you.\n\r", PERS_MAP(victim, ch));
         return;
      }
      else
      {
         set_char_color(AT_IGNORE, victim);
         ch_printf(victim, "You attempt to ignore %s, but " "are unable to do so.\n\r", PERS_MAP(ch, victim));
      }
   }

   act(AT_WHISPER, "You whisper to $N '$t'", ch, argument, victim, TO_CHAR);
   position = victim->position;
   victim->position = POS_STANDING;
   if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
   {
      int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
         knows_language(ch, ch->speaking, victim));

      if (speakswell < 85)
         act(AT_WHISPER, "$n whispers to you '$t'", ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT);

      else
         act(AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT);
   }
   else
      act(AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT);

   if (!xIS_SET(ch->in_room->room_flags, ROOM_SILENCE) && !wIS_SET(ch, ROOM_SILENCE))
      act(AT_WHISPER, "$n whispers something to $N.", ch, argument, victim, TO_NOTVICT);

   victim->position = position;
   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf, "%s: %s (whisper to) %s.", IS_NPC(ch) ? ch->short_descr : ch->name, argument, IS_NPC(victim) ? victim->short_descr : victim->name);
      append_to_file(LOG_FILE, buf);
   }

   mprog_speech_trigger(argument, ch);
   return;
}

void do_tell(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MIL];
   CHAR_DATA *victim;
   int position;
   CHAR_DATA *switched_victim = NULL;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif

   REMOVE_BIT(ch->deaf, CHANNEL_TELLS);
   if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE) || wIS_SET(ch, ROOM_SILENCE))
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Hard to talk when you are GAGGED!\n\r", ch);
      act(AT_WHITE, "$n tries to say something, but $e is GAGGED, how funny!", ch, NULL, NULL, TO_NOTVICT);
      return;
   }

   if (!IS_NPC(ch) && (xIS_SET(ch->act, PLR_SILENCE) || xIS_SET(ch->act, PLR_NO_TELL)))
   {
      send_to_char("You can't do that.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Tell whom what?\n\r", ch);
      return;
   }

   if ((victim = get_char_world(ch, arg)) == NULL
      || (IS_NPC(victim) && victim->in_room != ch->in_room) || (!NOT_AUTHED(ch) && NOT_AUTHED(victim) && !IS_IMMORTAL(ch)))
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (ch == victim)
   {
      send_to_char("You have a nice little chat with yourself.\n\r", ch);
      return;
   }

   if (NOT_AUTHED(ch) && !NOT_AUTHED(victim) && !IS_IMMORTAL(victim))
   {
      send_to_char("They can't hear you because you are not authorized.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && (victim->switched) && (get_trust(ch) > LEVEL_PC) && !IS_AFFECTED(victim->switched, AFF_POSSESS))
   {
      send_to_char("That player is switched.\n\r", ch);
      return;
   }

   else if (!IS_NPC(victim) && (victim->switched) && IS_AFFECTED(victim->switched, AFF_POSSESS))
      switched_victim = victim->switched;

   else if (!IS_NPC(victim) && (!victim->desc))
   {
      send_to_char("That player is link-dead.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK))
   {
      send_to_char("That player is afk.\n\r", ch);
      return;
   }

   if (IS_SET(victim->deaf, CHANNEL_TELLS) && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
   {
      act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_SILENCE))
      send_to_char("That player is silenced.  They will receive your message but can not respond.\n\r", ch);

   if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) || (!IS_NPC(victim) 
   &&  (xIS_SET(victim->in_room->room_flags, ROOM_SILENCE || wIS_SET(victim, ROOM_SILENCE)))))
   {
      act(AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
   }

   if (victim->desc /* make sure desc exists first  -Thoric */
      && victim->desc->connected == CON_EDITING && get_trust(ch) < LEVEL_IMM)
   {
      act(AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR);
      return;
   }

   /* Check to see if target of tell is ignoring the sender */
   if (is_ignoring(victim, ch))
   {
      /* If the sender is an imm then they cannot be ignored */
      if (!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
      {
         set_char_color(AT_IGNORE, ch);
         ch_printf(ch, "%s is ignoring you.\n\r", PERS_MAP(victim, ch));
         return;
      }
      else
      {
         set_char_color(AT_IGNORE, victim);
         ch_printf(victim, "You attempt to ignore %s, but " "are unable to do so.\n\r", PERS_MAP(ch, victim));
      }
   }

   ch->retell = victim;

   if (!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->pcdata->tell_history && isalpha(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]))
   {
      sprintf(buf, "%s told you '%s'\n\r", capitalize(IS_NPC(ch) ? ch->short_descr : PERS_MAP(ch, victim)), argument);

      /* get lasttell index... assumes names begin with characters */
      victim->pcdata->lt_index = tolower(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]) - 'a';

      /* get rid of old messages */
      if (victim->pcdata->tell_history[victim->pcdata->lt_index])
         STRFREE(victim->pcdata->tell_history[victim->pcdata->lt_index]);

      /* store the new message */
      victim->pcdata->tell_history[victim->pcdata->lt_index] = STRALLOC(buf);
   }

   if (switched_victim)
      victim = switched_victim;

   act(AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR);
   create_channel_history(ch, argument, CHANNEL_TELLS, NULL);
   create_channel_history(ch, argument, CHANNEL_TELLS, victim);
   position = victim->position;
   victim->position = POS_STANDING;
   if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
   {
      int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
         knows_language(ch, ch->speaking, victim));

      if ( speakswell < 85 )
         act( AT_TELL, MXPTAG ("player $n") "$n" MXPTAG ("/player")" tells you '$t'", ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT );
      else
 	 act( AT_TELL, MXPTAG ("player $n") "$n" MXPTAG ("/player")" tells you '$t'", ch, argument, victim, TO_VICT );
      }
      else
 	 act( AT_TELL, MXPTAG ("player $n") "$n" MXPTAG ("/player")" tells you '$t'", ch, argument, victim, TO_VICT );

   victim->position = position;
   victim->reply = ch;
   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf, "%s: %s (tell to) %s.", IS_NPC(ch) ? ch->short_descr : ch->name, argument, IS_NPC(victim) ? victim->short_descr : victim->name);
      append_to_file(LOG_FILE, buf);
   }

   mprog_speech_trigger(argument, ch);
   return;
}



void do_reply(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   CHAR_DATA *victim;
   int position;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif


   REMOVE_BIT(ch->deaf, CHANNEL_TELLS);
   if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE) || wIS_SET(ch, ROOM_SILENCE))
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_SILENCE))
   {
      send_to_char("Your message didn't get through.\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Hard to talk when you are GAGGED!\n\r", ch);
      act(AT_WHITE, "$n tries to say something, but $e is GAGGED, how funny!", ch, NULL, NULL, TO_NOTVICT);
      return;
   }

   if ((victim = ch->reply) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && (victim->switched) && can_see_map(ch, victim) && (get_trust(ch) > LEVEL_PC))
   {
      send_to_char("That player is switched.\n\r", ch);
      return;
   }
   else if (!IS_NPC(victim) && (!victim->desc))
   {
      send_to_char("That player is link-dead.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK))
   {
      send_to_char("That player is afk.\n\r", ch);
      return;
   }

   if (IS_SET(victim->deaf, CHANNEL_TELLS) && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
   {
      act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) || (!IS_NPC(victim) 
   &&  (xIS_SET(victim->in_room->room_flags, ROOM_SILENCE) || wIS_SET(victim, ROOM_SILENCE))))
   {
      act(AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
   }

   if (victim->desc /* make sure desc exists first  -Thoric */
      && victim->desc->connected == CON_EDITING && get_trust(ch) < LEVEL_IMM)
   {
      act(AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR);
      return;
   }

   /* Check to see if the receiver is ignoring the sender */
   if (is_ignoring(victim, ch))
   {
      /* If the sender is an imm they cannot be ignored */
      if (!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
      {
         set_char_color(AT_IGNORE, ch);
         ch_printf(ch, "%s is ignoring you.\n\r", PERS_MAP(victim, ch));
         return;
      }
      else
      {
         set_char_color(AT_IGNORE, victim);
         ch_printf(victim, "You attempt to ignore %s, but " "are unable to do so.\n\r", PERS_MAP(ch, victim));
      }
   }

   act(AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR);
   create_channel_history(ch, argument, CHANNEL_TELLS, NULL);
   create_channel_history(ch, argument, CHANNEL_TELLS, victim);
   position = victim->position;
   victim->position = POS_STANDING;
#ifndef SCRAMBLE
   if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
   {
      int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
         knows_language(ch, ch->speaking, victim));

      if (speakswell < 85)
         act(AT_TELL, "$n tells you '$t'", ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT);
      else
         act(AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT);
   }
   else
      act(AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT);
#else
   if (knows_language(victim, ch->speaking, ch) || (IS_NPC(ch) && !ch->speaking))
      act(AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT);
   else
      act(AT_TELL, "$n tells you '$t'", ch, scramble(argument, ch->speaking), victim, TO_VICT);
#endif
   victim->position = position;
   victim->reply = ch;
   ch->retell = victim;
   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf, "%s: %s (reply to) %s.", IS_NPC(ch) ? ch->short_descr : ch->name, argument, IS_NPC(victim) ? victim->short_descr : victim->name);
      append_to_file(LOG_FILE, buf);
   }

   if (!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->pcdata->tell_history && isalpha(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]))
   {
      sprintf(buf, "%s told you '%s'\n\r", capitalize(IS_NPC(ch) ? ch->short_descr : PERS_MAP(ch, victim)), argument);

      /* get lasttell index... assumes names begin with characters */
      victim->pcdata->lt_index = tolower(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]) - 'a';

      /* get rid of old messages */
      if (victim->pcdata->tell_history[victim->pcdata->lt_index])
         STRFREE(victim->pcdata->tell_history[victim->pcdata->lt_index]);

      /* store the new message */
      victim->pcdata->tell_history[victim->pcdata->lt_index] = STRALLOC(buf);
   }

   return;
}

void do_retell(CHAR_DATA * ch, char *argument)
{
   char buf[MIL];
   CHAR_DATA *victim;
   int position;
   CHAR_DATA *switched_victim = NULL;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif
   REMOVE_BIT(ch->deaf, CHANNEL_TELLS);
   if (xIS_SET(ch->in_room->room_flags, ROOM_SILENCE) || wIS_SET(ch, ROOM_SILENCE))
   {
      send_to_char("You can't do that here.\n\r", ch);
      return;
   }

   if (!IS_NPC(ch) && (xIS_SET(ch->act, PLR_SILENCE) || xIS_SET(ch->act, PLR_NO_TELL)))
   {
      send_to_char("You can't do that.\n\r", ch);
      return;
   }
   
   if (IS_AFFECTED(ch, AFF_GAGGED))
   {
      send_to_char("Hard to talk when you are GAGGED!\n\r", ch);
      act(AT_WHITE, "$n tries to say something, but $e is GAGGED, how funny!", ch, NULL, NULL, TO_NOTVICT);
      return;
   }

   if (argument[0] == '\0')
   {
      ch_printf(ch, "What message do you wish to send?\n\r");
      return;
   }

   victim = ch->retell;

   if (!victim)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && (victim->switched) && (get_trust(ch) > LEVEL_PC) && !IS_AFFECTED(victim->switched, AFF_POSSESS))
   {
      send_to_char("That player is switched.\n\r", ch);
      return;
   }
   else if (!IS_NPC(victim) && (victim->switched) && IS_AFFECTED(victim->switched, AFF_POSSESS))
   {
      switched_victim = victim->switched;
   }
   else if (!IS_NPC(victim) && (!victim->desc))
   {
      send_to_char("That player is link-dead.\n\r", ch);
      return;
   }

   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_AFK))
   {
      send_to_char("That player is afk.\n\r", ch);
      return;
   }

   if (IS_SET(victim->deaf, CHANNEL_TELLS) && (!IS_IMMORTAL(ch) || (get_trust(ch) < get_trust(victim))))
   {
      act(AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (!IS_NPC(victim) && xIS_SET(victim->act, PLR_SILENCE))
      send_to_char("That player is silenced. They will receive your message, but can not respond.\n\r", ch);

   if ((!IS_IMMORTAL(ch) && !IS_AWAKE(victim)) || (!IS_NPC(victim) 
   && (xIS_SET(victim->in_room->room_flags, ROOM_SILENCE) || wIS_SET(victim, ROOM_SILENCE))))
   {
      act(AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR);
      return;
   }

   if (victim->desc && victim->desc->connected == CON_EDITING && get_trust(ch) < LEVEL_IMM)
   {
      act(AT_PLAIN, "$E is currently in a writing buffer. Please " "try again in a few minutes.", ch, 0, victim, TO_CHAR);
      return;
   }

   /* check to see if the target is ignoring the sender */
   if (is_ignoring(victim, ch))
   {
      /* if the sender is an imm then they cannot be ignored */
      if (!IS_IMMORTAL(ch) || get_trust(victim) > get_trust(ch))
      {
         set_char_color(AT_IGNORE, ch);
         ch_printf(ch, "%s is ignoring you.\n\r", PERS_MAP(victim, ch));
         return;
      }
      else
      {
         set_char_color(AT_IGNORE, victim);
         ch_printf(victim, "You attempy to ignore %s, but " "are unable to do so.\n\r", PERS_MAP(ch, victim));
      }
   }

   /* store tell history for victim */
   if (!IS_NPC(victim) && IS_IMMORTAL(victim) && victim->pcdata->tell_history && isalpha(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]))
   {
      sprintf(buf, "%s told you '%s'\n\r", capitalize(IS_NPC(ch) ? ch->short_descr : PERS_MAP(ch, victim)), argument);

      /* get lasttel index... assumes names begin with chars */
      victim->pcdata->lt_index = tolower(IS_NPC(ch) ? ch->short_descr[0] : ch->name[0]) - 'a';

      /* get rid of old messages */
      if (victim->pcdata->tell_history[victim->pcdata->lt_index])
         STRFREE(victim->pcdata->tell_history[victim->pcdata->lt_index]);

      /* store the new messagec */
      victim->pcdata->tell_history[victim->pcdata->lt_index] = STRALLOC(buf);
   }

   if (switched_victim)
      victim = switched_victim;

   act(AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR);
   create_channel_history(ch, argument, CHANNEL_TELLS, NULL);
   create_channel_history(ch, argument, CHANNEL_TELLS, victim);
   position = victim->position;
   victim->position = POS_STANDING;
#ifndef SCRAMBLE
   if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
   {
      int speakswell = UMIN(knows_language(victim, ch->speaking, ch),
         knows_language(ch, ch->speaking, victim));

      if (speakswell < 85)
         act(AT_TELL, "$n tells you '$t'", ch, translate(speakswell, argument, lang_names[speaking]), victim, TO_VICT);
      else
         act(AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT);
   }
   else
      act(AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT);
#else
   if (knows_language(victim, ch->speaking, ch) || (IS_NPC(ch) && !ch->speaking))
   {
      act(AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT);
   }
   else
   {
      act(AT_TELL, "$n tells you '$t'", ch, scramble(argument, ch->speaking), victim, TO_VICT);
   }
#endif
   victim->position = position;
   victim->reply = ch;
   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf, "%s: %s (retell to) %s.", IS_NPC(ch) ? ch->short_descr : ch->name, argument, IS_NPC(victim) ? victim->short_descr : victim->name);
      append_to_file(LOG_FILE, buf);
   }

   mprog_speech_trigger(argument, ch);
   return;
}

void do_repeat(CHAR_DATA * ch, char *argument)
{
   int index;

   if (IS_NPC(ch) || !IS_IMMORTAL(ch) || !ch->pcdata->tell_history)
   {
      ch_printf(ch, "Huh?\n\r");
      return;
   }

   if (argument[0] == '\0')
   {
      index = ch->pcdata->lt_index;
   }
   else if (isalpha(argument[0]) && argument[1] == '\0')
   {
      index = tolower(argument[0]) - 'a';
   }
   else
   {
      ch_printf(ch, "You may only index your tell history using " "a single letter.\n\r");
      return;
   }

   if (ch->pcdata->tell_history[index])
   {
      set_char_color(AT_TELL, ch);
      ch_printf(ch, ch->pcdata->tell_history[index]);
   }
   else
   {
      ch_printf(ch, "No one like that has sent you a tell.\n\r");
   }

   return;
}


void do_emote(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char *plast;
   CHAR_DATA *vch;
   EXT_BV actflags;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif

   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_NO_EMOTE))
   {
      send_to_char("You can't show your emotions.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
   {
      send_to_char("Emote what?\n\r", ch);
      return;
   }

   actflags = ch->act;
   if (IS_NPC(ch))
      xREMOVE_BIT(ch->act, ACT_SECRETIVE);
   for (plast = argument; *plast != '\0'; plast++)
      ;

   strcpy(buf, argument);
   if (isalpha(plast[-1]))
      strcat(buf, ".");
   for (vch = ch->in_room->first_person; vch; vch = vch->next_in_room)
   {
      char *sbuf = buf;

      /* Check to see if a player on a map is at the same coords as the recipient */
      if (IS_ONMAP_FLAG(ch))
      {
         if (vch->coord->x != ch->coord->x || vch->coord->y != ch->coord->y)
            continue;
      }

      /* Check to see if character is ignoring emoter */
      if (is_ignoring(vch, ch))
      {
         /* continue unless emoter is an immortal */
         if (!IS_IMMORTAL(ch) || get_trust(vch) > get_trust(ch))
            continue;
         else
         {
            set_char_color(AT_IGNORE, vch);
            ch_printf(vch, "You attempt to ignore %s, but" " are unable to do so.\n\r", PERS_MAP(ch, vch));
         }
      }
#ifndef SCRAMBLE
      if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
      {
         int speakswell = UMIN(knows_language(vch, ch->speaking, ch),
            knows_language(ch, ch->speaking, vch));

         if (speakswell < 85)
            sbuf = translate(speakswell, argument, lang_names[speaking]);
      }
#else
      if (!knows_language(vch, ch->speaking, ch) && (!IS_NPC(ch) && ch->speaking != 0))
         sbuf = scramble(buf, ch->speaking);
#endif
      MOBtrigger = FALSE;
      act(AT_SOCIAL, "$n $t", ch, sbuf, vch, (vch == ch ? TO_CHAR : TO_VICT));
   }
/*    MOBtrigger = FALSE;
    act( AT_SOCIAL, "$n $T", ch, NULL, buf, TO_ROOM );
    MOBtrigger = FALSE;
    act( AT_SOCIAL, "$n $T", ch, NULL, buf, TO_CHAR );*/
   ch->act = actflags;
   if (xIS_SET(ch->in_room->room_flags, ROOM_LOGSPEECH))
   {
      sprintf(buf, "%s %s (emote)", IS_NPC(ch) ? ch->short_descr : ch->name, argument);
      append_to_file(LOG_FILE, buf);
   }
   return;
}


void do_bug(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_PLAIN, ch);
   if (argument[0] == '\0')
   {
      send_to_char_color("\n\rUsage:  'bug <message>'  (your location is automatically recorded)\n\r", ch);
      if (get_trust(ch) >= LEVEL_IMM)
         send_to_char_color("Usage:  'bug list'\n\r", ch);
      if (get_trust(ch) >= LEVEL_STAFF)
         send_to_char_color("Usage:  'bug clear now'\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "clear now") && get_trust(ch) >= LEVEL_STAFF)
   {
      FILE *fp = fopen(PBUG_FILE, "w");

      if (fp)
         fclose(fp);
      send_to_char_color("Bug file cleared.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "list") && get_trust(ch) >= LEVEL_IMM)
   {
      send_to_char_color("\n\r VNUM \n\r.......\n\r", ch);
      show_file(ch, PBUG_FILE);
   }
   else
   {
      append_file(ch, PBUG_FILE, argument);
      send_to_char_color("Thanks, your bug notice has been recorded.\n\r", ch);
   }
   return;
}

void do_ide(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_PLAIN, ch);
   send_to_char_color("\n\rIf you want to send an idea, type 'idea <message>'.\n\r", ch);
   send_to_char_color("If you want to identify an object, use the identify spell.\n\r", ch);
   return;
}

void do_idea(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_PLAIN, ch);
   if (argument[0] == '\0')
   {
      send_to_char_color("\n\rUsage:  'idea <message>'  (your location is automatically recorded)\n\r", ch);
      if (get_trust(ch) >= LEVEL_IMM)
         send_to_char_color("Usage:  'idea list'\n\r", ch);
      if (get_trust(ch) >= LEVEL_STAFF)
         send_to_char("Usage:  'idea clear now'\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "clear now") && get_trust(ch) >= LEVEL_STAFF)
   {
      FILE *fp = fopen(IDEA_FILE, "w");

      if (fp)
         fclose(fp);
      send_to_char_color("Idea file cleared.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "list") && get_trust(ch) >= LEVEL_IMM)
   {
      send_to_char_color("\n\r VNUM \n\r.......\n\r", ch);
      show_file(ch, IDEA_FILE);
   }
   else
   {
      append_file(ch, IDEA_FILE, argument);
      send_to_char_color("Thanks, your idea notice has been recorded.\n\r", ch);
   }
   return;
}

void do_ahelp(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_PLAIN, ch);
   if (argument[0] == '\0')
   {
      send_to_char_color("\n\rUsage:  'ahelp list' or 'ahelp clear now'\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "clear now"))
   {
      FILE *fp = fopen(HELP_FILE, "w");

      if (fp)
         fclose(fp);
      send_to_char_color("Add Help file cleared.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "list"))
   {
      send_to_char_color("\n\r VNUM \n\r.......\n\r", ch);
      show_file(ch, HELP_FILE);
   }
   else
   {
      send_to_char_color("\n\rUsage:  'ahelp list' or 'ahelp clear now'\n\r", ch);
      return;
   }
   return;
}

void do_typo(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_PLAIN, ch);
   if (argument[0] == '\0')
   {
      send_to_char_color("\n\rUsage:  'typo <message>'  (your location is automatically recorded)\n\r", ch);
      if (get_trust(ch) >= LEVEL_IMM)
         send_to_char_color("Usage:  'typo list'\n\r", ch);
      if (get_trust(ch) >= LEVEL_STAFF)
         send_to_char("Usage:  'typo clear now'\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "clear now") && get_trust(ch) >= LEVEL_STAFF)
   {
      FILE *fp = fopen(TYPO_FILE, "w");

      if (fp)
         fclose(fp);
      send_to_char_color("Typo file cleared.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "list") && get_trust(ch) >= LEVEL_IMM)
   {
      send_to_char_color("\n\r VNUM \n\r.......\n\r", ch);
      show_file(ch, TYPO_FILE);
   }
   else
   {
      append_file(ch, TYPO_FILE, argument);
      send_to_char_color("Thanks, your typo notice has been recorded.\n\r", ch);
   }
   return;
}

void do_rent(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_WHITE, ch);
   send_to_char("There is no rent here.  Just save and quit.\n\r", ch);
   return;
}



void do_qui(CHAR_DATA * ch, char *argument)
{
   set_char_color(AT_RED, ch);
   send_to_char("If you want to QUIT, you have to spell it out.\n\r", ch);
   return;
}

void do_quit(CHAR_DATA * ch, char *argument)
{
   char_quit(ch, TRUE);
}

/* Rantic's info channel */
void char_quit(CHAR_DATA * ch, bool broad_quit)
{

   int x, y;
   int level;
   char buf[MSL];
   if (IS_NPC(ch))
      return;

   if (ch->desc && ch->desc->arena)
   {
      send_to_char("You cannot quit while in the Battle Arena, wait or go LD.\n\r", ch);
      return;
   }
   
   if (xIS_SET(ch->in_room->room_flags, ROOM_PERMDEATH))
   {
      send_to_char("You are in a permdeath arena, you cannot quit.\n\r", ch);
      return;
   }

   if (ch->position == POS_FIGHTING
      || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK)
   {
      set_char_color(AT_RED, ch);
      send_to_char("No way! You are fighting.\n\r", ch);
      return;
   }
   if (ch->position < POS_STUNNED)
   {
      set_char_color(AT_BLOOD, ch);
      send_to_char("You're not DEAD yet.\n\r", ch);
      return;
   }
   if (get_timer(ch, TIMER_RECENTFIGHT) >= 1)
   {
      send_to_char("Your blood is pumping too much to do this at this time.\n\r", ch);
      return;
   }

   if (ch->pcdata->challenged)
   {
      ch->pcdata->challenged = NULL;
      act(AT_RED, "&w&RINFO: &w&W%n has quit! Challenge is void. WHAT A WUSS!", ch, NULL, NULL, TO_MUD);
   }

   if (auction->item != NULL && ((ch == auction->buyer) || (ch == auction->seller)))
   {
      send_to_char("Wait until you have bought/sold the item on auction.\n\r", ch);
      return;

   }

   if (xIS_SET(ch->act, PLR_OSET))
      xREMOVE_BIT(ch->act, PLR_OSET);
   if (xIS_SET(ch->act, PLR_MSET))
      xREMOVE_BIT(ch->act, PLR_MSET);
   if (xIS_SET(ch->act, PLR_REDIT))
      xREMOVE_BIT(ch->act, PLR_REDIT);
   if (xIS_SET(ch->act, PLR_GAMBLER))
      xREMOVE_BIT(ch->act, PLR_GAMBLER);

   /* Get 'em dismounted until we finish mount saving -- Blodkai, 4/97 */
   if (ch->position == POS_MOUNTED)
      do_dismount(ch, "");
   if (ch->riding)
      do_dismount(ch, "");
   if (ch->position == POS_RIDING)
      ch->position = POS_STANDING;
   set_char_color(AT_WHITE, ch);
   send_to_char
      ("You start to feel odd, kind of like your body is starting to itch.  Just as you reach down to answer the itch, Xerves's foot kicks you in the ass, and you going flying out of Rafermand head first.  We do hope you come back.\n\r\n\r",
      ch);

   act(AT_SAY, "Xerves voice says, 'We await your return, $n...'", ch, NULL, NULL, TO_CHAR);
   act(AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_CANSEE);
   set_char_color(AT_GREY, ch);

   /* Cronel, upgrade (Rantic) */
   if (broad_quit && !IS_IMMORTAL(ch))
   {
      sprintf(buf, "You feel that there is one less presence in %s...", sysdata.mud_name);
      talk_info(AT_BLUE, buf);
   }


   sprintf(log_buf, "%s has quit (Room %d).", ch->name, (ch->in_room ? ch->in_room->vnum : -1));
   quitting_char = ch;
   save_char_obj(ch);

   if (sysdata.save_pets && ch->pcdata->pet)
   {
      act(AT_BYE, "$N follows $s master into the Void.", ch, NULL, ch->pcdata->pet, TO_ROOM);
      saving_mount_on_quit = 1;
      extract_char(ch->pcdata->pet, TRUE);
      saving_mount_on_quit = 0;
   }
   if (ch->pcdata->mount)
   {
      act(AT_BYE, "$N follows $s master into the Void.", ch, NULL, ch->pcdata->mount, TO_ROOM);
      saving_mount_on_quit = 1;
      extract_char(ch->pcdata->mount, TRUE);
      saving_mount_on_quit = 0;
   }
   if (ch->pcdata->in_progress)
      free_global_note(ch->pcdata->in_progress);

   /* Synch clandata up only when clan member quits now. --Shaddai
    */
   if (ch->pcdata->clan)
      save_clan(ch->pcdata->clan);

   saving_char = NULL;

   level = get_trust(ch);
   /*
    * After extract_char the ch is no longer valid!
    */
   extract_char(ch, TRUE);
   for (x = 0; x < MAX_WEAR; x++)
      for (y = 0; y < MAX_LAYERS; y++)
         save_equipment[x][y] = NULL;

   /* don't show who's logging off to leaving player */
/*
    to_channel( log_buf, CHANNEL_MONITOR, "Monitor", level );
*/
   log_string_plus(log_buf, LOG_COMM, level);
   return;
}


void send_rip_screen(CHAR_DATA * ch)
{
   FILE *rpfile;
   int num = 0;
   char BUFF[MSL * 2];

   if ((rpfile = fopen(RIPSCREEN_FILE, "r")) != NULL)
   {
      while ((BUFF[num] = fgetc(rpfile)) != EOF)
         num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc, BUFF, num);
   }
}

void send_rip_title(CHAR_DATA * ch)
{
   FILE *rpfile;
   int num = 0;
   char BUFF[MSL * 2];

   if ((rpfile = fopen(RIPTITLE_FILE, "r")) != NULL)
   {
      while ((BUFF[num] = fgetc(rpfile)) != EOF)
         num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc, BUFF, num);
   }
}

void send_ansi_title(CHAR_DATA * ch)
{
   FILE *rpfile;
   int num = 0;
   char BUFF[MSL * 2];

   if ((rpfile = fopen(ANSITITLE_FILE, "r")) != NULL)
   {
      while ((BUFF[num] = fgetc(rpfile)) != EOF)
         num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc, BUFF, num);
   }
}

void send_ascii_title(CHAR_DATA * ch)
{
   FILE *rpfile;
   int num = 0;
   char BUFF[MSL];

   if ((rpfile = fopen(ASCTITLE_FILE, "r")) != NULL)
   {
      while ((BUFF[num] = fgetc(rpfile)) != EOF)
         num++;
      fclose(rpfile);
      BUFF[num] = 0;
      write_to_buffer(ch->desc, BUFF, num);
   }
}

void do_rip(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Rip ON or OFF?\n\r", ch);
      return;
   }
   if ((strcmp(arg, "on") == 0) || (strcmp(arg, "ON") == 0))
   {
      send_rip_screen(ch);
      xSET_BIT(ch->act, PLR_RIP);
      xSET_BIT(ch->act, PLR_ANSI);
      return;
   }

   if ((strcmp(arg, "off") == 0) || (strcmp(arg, "OFF") == 0))
   {
      xREMOVE_BIT(ch->act, PLR_RIP);
      send_to_char("!|*\n\rRIP now off...\n\r", ch);
      return;
   }
}

void do_ansi(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("ANSI ON or OFF?\n\r", ch);
      return;
   }
   if ((strcmp(arg, "on") == 0) || (strcmp(arg, "ON") == 0))
   {
      xSET_BIT(ch->act, PLR_ANSI);
      set_char_color(AT_WHITE + AT_BLINK, ch);
      send_to_char("ANSI ON!!!\n\r", ch);
      return;
   }

   if ((strcmp(arg, "off") == 0) || (strcmp(arg, "OFF") == 0))
   {
      xREMOVE_BIT(ch->act, PLR_ANSI);
      send_to_char("Okay... ANSI support is now off\n\r", ch);
      return;
   }
}

void do_save(CHAR_DATA * ch, char *argument)
{
   if (IS_NPC(ch))
      return;
   //Don't want them to be saved if in auth, etc...
   if (IS_SET(ch->pcdata->flags, PCFLAG_UNAUTHED))
   {
      send_to_char("You cannot save when you are unauthed.\n\r", ch);
      return;
   }
   WAIT_STATE(ch, 2); /* For big muds with save-happy players, like RoD */
   update_aris(ch); /* update char affects and RIS */
   save_char_obj(ch);
   saving_char = NULL;
   send_to_char("Saved...\n\r", ch);
   return;
}


/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)			-Thoric
 */
bool circle_follow(CHAR_DATA * ch, CHAR_DATA * victim)
{
   CHAR_DATA *tmp;

   for (tmp = victim; tmp; tmp = tmp->master)
      if (tmp == ch)
         return TRUE;
   return FALSE;
}


void do_dismiss(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Dismiss whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if ((IS_AFFECTED(victim, AFF_CHARM)) && (IS_NPC(victim)) && (victim->master == ch))
   {
      stop_follower(victim);
      stop_hating(victim);
      stop_hunting(victim);
      stop_fearing(victim);
      act(AT_ACTION, "$n dismisses $N.", ch, NULL, victim, TO_NOTVICT);
      act(AT_ACTION, "You dismiss $N.", ch, NULL, victim, TO_CHAR);
   }
   else
   {
      send_to_char("You cannot dismiss them.\n\r", ch);
   }

   return;
}

void do_follow(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Follow whom?\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (IS_AFFECTED(ch, AFF_CHARM) && ch->master)
   {
      act(AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR);
      return;
   }

   if (victim == ch)
   {
      if (!ch->master)
      {
         send_to_char("You already follow yourself.\n\r", ch);
         return;
      }
      stop_follower(ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      if (IS_SET(victim->pcdata->flags, PCFLAG_NOFOLLOW))
      {
         if (victim->pcdata->council && !str_cmp(victim->pcdata->council->name, "Newbie Council") && get_trust(ch) > 7)
         {
            send_to_char("This individual is not taking followers right now.\n\r", ch);
            return;
         }
      }
   }
   if (circle_follow(ch, victim))
   {
      send_to_char("Following in loops is not allowed... sorry.\n\r", ch);
      return;
   }

   if (ch->master)
      stop_follower(ch);

   add_follower(ch, victim);
   return;
}



void add_follower(CHAR_DATA * ch, CHAR_DATA * master)
{
   if (ch->master)
   {
      bug("Add_follower: non-null master.", 0);
      return;
   }

   ch->master = master;
   ch->leader = NULL;

   /* Support for saving pets --Shaddai */
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_PET) && !IS_NPC(master))
      master->pcdata->pet = ch;
   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOUNTSAVE) && !IS_NPC(master))
      master->pcdata->mount = ch;



   if (can_see(master, ch))
      act(AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT);

   act(AT_ACTION, "You now follow $N.", ch, NULL, master, TO_CHAR);

   return;
}



void stop_follower(CHAR_DATA * ch)
{
   if (!ch->master)
   {
      bug("Stop_follower: null master.", 0);
      return;
   }

   if (IS_NPC(ch) && !IS_NPC(ch->master) && ch->master->pcdata && ch->master->pcdata->pet == ch)
      ch->master->pcdata->pet = NULL;

   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOUNTSAVE))
      return;

   if (IS_AFFECTED(ch, AFF_CHARM))
   {
      xREMOVE_BIT(ch->affected_by, AFF_CHARM);
      affect_strip(ch, gsn_charm_person);
   }

   if (can_see(ch->master, ch))
      if (!(!IS_NPC(ch->master) && IS_IMMORTAL(ch) && !IS_IMMORTAL(ch->master)))
         act(AT_ACTION, "$n stops following you.", ch, NULL, ch->master, TO_VICT);
   act(AT_ACTION, "You stop following $N.", ch, NULL, ch->master, TO_CHAR);

   ch->master = NULL;
   ch->leader = NULL;
   return;
}



void die_follower(CHAR_DATA * ch)
{
   CHAR_DATA *fch;

   if (ch->master)
      stop_follower(ch);

   ch->leader = NULL;

   for (fch = first_char; fch; fch = fch->next)
   {
      if (fch->master == ch)
         stop_follower(fch);
      if (fch->leader == ch)
         fch->leader = fch;
   }
   return;
}



void do_order(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char argbuf[MIL];
   CHAR_DATA *victim;
   CHAR_DATA *och;
   CHAR_DATA *och_next;
   bool found;
   bool fAll;

   strcpy(argbuf, argument);
   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Order whom to do what?\n\r", ch);
      return;
   }

   if (IS_AFFECTED(ch, AFF_CHARM))
   {
      send_to_char("You feel like taking, not giving, orders.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "all"))
   {
      fAll = TRUE;
      victim = NULL;
   }
   else
   {
      fAll = FALSE;
      if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }

      if (victim == ch)
      {
         send_to_char("Aye aye, right away!\n\r", ch);
         return;
      }

      if (!IS_AFFECTED(victim, AFF_CHARM) || victim->master != ch)
      {
         send_to_char("Do it yourself!\n\r", ch);
         return;
      }
   }

   found = FALSE;
   for (och = ch->in_room->first_person; och; och = och_next)
   {
      och_next = och->next_in_room;

      if (IS_AFFECTED(och, AFF_CHARM) && och->master == ch && (fAll || och == victim))
      {
         found = TRUE;
         act(AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT);
         interpret(och, argument);
      }
   }

   if (found)
   {
      sprintf(log_buf, "%s: order %s.", ch->name, argbuf);
      log_string_plus(log_buf, LOG_NORMAL, ch->level);
      send_to_char("Ok.\n\r", ch);
      WAIT_STATE(ch, 12);
   }
   else
      send_to_char("You have no followers here.\n\r", ch);
   return;
}

/*
char *itoa(int foo)
{
  static char bar[256];

  sprintf(bar,"%d",foo);
  return(bar);

}
*/

/* Overhauled 2/97 -- Blodkai */
void do_group(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      CHAR_DATA *gch;
      CHAR_DATA *leader;

      leader = ch->leader ? ch->leader : ch;
      set_char_color(AT_DGREEN, ch);
      ch_printf(ch, "\n\r&w&WFollowing %-12.12s\n\r---------------------------------------------------------------------------------------\n\r", 
         PERS_MAP(leader, ch));     
      ch_printf(ch, "&w&GName          [hitpnts]   [ magic ] [mst] [mv] [race] [End] [RArm] [LArm] [RLeg] [LLeg]\n\r");
      for (gch = first_char; gch; gch = gch->next)
      {
         if (is_same_group(gch, ch))
         {
            set_char_color(AT_DGREEN, ch);
            if (IS_AFFECTED(gch, AFF_POSSESS)) /*reveal no information */
               ch_printf(ch,
                  "%-16s %4s/%4s hp %4s/%4s %s %4s/%4s mv\n\r",
                  capitalize(PERS_MAP(gch, ch)), "????", "????", "????", "????", IS_VAMPIRE(gch) ? "bp" : "mana", "????", "????");
            set_char_color(AT_GREEN, ch);
            ch_printf(ch, "%-12.12s ", capitalize(PERS_MAP(gch, ch)));
            if (gch->hit < gch->max_hit / 4)
               set_char_color(AT_DANGER, ch);
            else if (gch->hit < gch->max_hit / 2.5)
               set_char_color(AT_YELLOW, ch);
            else
               set_char_color(AT_GREY, ch);
            ch_printf(ch, "%5d", gch->hit);
            set_char_color(AT_GREY, ch);
            ch_printf(ch, "/%-5d ", gch->max_hit);
            set_char_color(AT_LBLUE, ch);
            ch_printf(ch, "%5d/%-5d ",
                  gch->mana, gch->max_mana);
            if (gch->mental_state < -25 || gch->mental_state > 25)
               set_char_color(AT_YELLOW, ch);
            else
               set_char_color(AT_GREEN, ch);
            ch_printf(ch, "%3.3s  ",
               gch->mental_state > 75 ? "+++" :
               gch->mental_state > 50 ? "=++" :
               gch->mental_state > 25 ? "==+" :
               gch->mental_state > -25 ? "===" : gch->mental_state > -50 ? "-==" : gch->mental_state > -75 ? "--=" : "---");
            set_char_color(AT_DGREEN, ch);
            ch_printf(ch, "%4d ", gch->move);
            ch_printf(ch, "%6s ",
               gch->race == 0 ? "human" :
               gch->race == 1 ? "elf" :
               gch->race == 2 ? "dwarf" :
               gch->race == 3 ? "ogre" :
               gch->race == 4 ? "hobbit" :
               gch->race == 5 ? "fairy" : "");
            ch_printf(ch, "&w&c%3d    %5s  %5s  %5s  %5s", gch->mover,
               gch->con_rarm >= 80 ? "&w&R+++++" : gch->con_rarm >= 60 ? "&w&R++++&w&r-" : gch->con_rarm >= 40 ? "&w&R+++&w&r--" : 
               gch->con_rarm >= 20 ? "&w&R++&w&r---" : gch->con_rarm >= 1 ? "&w&R+&w&r----" : "&w&r-----",
               gch->con_larm >= 80 ? "&w&R+++++" : gch->con_larm >= 60 ? "&w&R++++&w&r-" : gch->con_larm >= 40 ? "&w&R+++&w&r--" : 
               gch->con_larm >= 20 ? "&w&R++&w&r---" : gch->con_larm >= 1 ? "&w&R+&w&r----" : "&w&r-----",
               gch->con_rleg >= 80 ? "&w&R+++++" : gch->con_rleg >= 60 ? "&w&R++++&w&r-" : gch->con_rleg >= 40 ? "&w&R+++&w&r--" : 
               gch->con_rleg >= 20 ? "&w&R++&w&r---" : gch->con_rleg >= 1 ? "&w&R+&w&r----" : "&w&r-----",
               gch->con_lleg >= 80 ? "&w&R+++++" : gch->con_lleg >= 60 ? "&w&R++++&w&r-" : gch->con_lleg >= 40 ? "&w&R+++&w&r--" : 
               gch->con_lleg >= 20 ? "&w&R++&w&r---" : gch->con_lleg >= 1 ? "&w&R+&w&r----" : "&w&r-----");
            set_char_color(AT_GREEN, ch);
            send_to_char("\n\r", ch);
         }
      }
      return;
   }

   if (!strcmp(arg, "disband"))
   {
      CHAR_DATA *gch;
      int count = 0;

      if (ch->leader || ch->master)
      {
         send_to_char("You cannot disband a group or followers if you're following someone.\n\r", ch);
         return;
      }

      for (gch = first_char; gch; gch = gch->next)
      {
         if ((is_same_group(ch, gch) || (gch->leader == ch || gch->master == ch)) && (ch != gch))
         {
            if (IS_NPC(gch))
               continue;
            count++;
            if (gch->leader)
               send_to_char("Your group is disbanded.\n\r", gch);
            else
               ch_printf(gch, "You are forced to stop following %s.\n\r", ch);
            gch->leader = NULL;
            gch->master = NULL;
         }
      }

      if (count == 0)
         send_to_char("You have no group members or followers to disband.\n\r", ch);
      else
         send_to_char("You disband your group and followers.\n\r", ch);

      return;
   }

   if (!strcmp(arg, "all"))
   {
      CHAR_DATA *rch;
      int count = 0;

      for (rch = ch->in_room->first_person; rch; rch = rch->next_in_room)
      {
         if (ch != rch
            && !IS_NPC(rch)
            && can_see_map(ch, rch) && rch->master == ch && !ch->master && !ch->leader && !is_same_group(rch, ch))
         {
            rch->leader = ch;
            count++;
         }
      }

      if (count == 0)
         send_to_char("You have no eligible group members.\n\r", ch);
      else
      {
         act(AT_ACTION, "$n groups $s followers.", ch, NULL, NULL, TO_ROOM);
         send_to_char("You group your followers.\n\r", ch);
      }
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (ch->master || (ch->leader && ch->leader != ch))
   {
      send_to_char("But you are following someone else!\n\r", ch);
      return;
   }

   if (victim->master != ch && ch != victim)
   {
      act(AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (victim == ch)
   {
      act(AT_PLAIN, "You can't group yourself.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (IS_NPC(victim) && xIS_SET(victim->act, ACT_MOUNTSAVE))
   {
      act(AT_PLAIN, "$N is your loyal mount, that is not a good idea.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (is_same_group(victim, ch) && ch != victim)
   {
      victim->leader = NULL;
      act(AT_ACTION, "$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT);
      act(AT_ACTION, "$n removes you from $s group.", ch, NULL, victim, TO_VICT);
      act(AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR);
      return;
   }

   victim->leader = ch;
   act(AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT);
   act(AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT);
   act(AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR);
   return;
}



/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   char arg[MIL];
   CHAR_DATA *gch;
   int members;
   int amount;
   int share;
   int extra;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Split how much?\n\r", ch);
      return;
   }

   amount = atoi(arg);

   if (amount < 0)
   {
      send_to_char("Your group wouldn't like that.\n\r", ch);
      return;
   }

   if (amount == 0)
   {
      send_to_char("You hand out zero coins, but no one notices.\n\r", ch);
      return;
   }

   if (ch->gold < amount)
   {
      send_to_char("You don't have that much gold.\n\r", ch);
      return;
   }

   members = 0;
   for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
   {
      if (is_same_group(gch, ch))
         members++;
   }


   if (xIS_SET(ch->act, PLR_AUTOGOLD) && members < 2)
      return;

   if (members < 2)
   {
      send_to_char("Just keep it all.\n\r", ch);
      return;
   }

   share = amount / members;
   extra = amount % members;

   if (share == 0)
   {
      send_to_char("Don't even bother, cheapskate.\n\r", ch);
      return;
   }

   ch->gold -= amount;
   ch->gold += share + extra;

   set_char_color(AT_GOLD, ch);
   ch_printf(ch, "You split %d gold coins.  Your share is %d gold coins.\n\r", amount, share + extra);

   sprintf(buf, "$n splits %d gold coins.  Your share is %d gold coins.", amount, share);

   for (gch = ch->in_room->first_person; gch; gch = gch->next_in_room)
   {
      if (gch != ch && is_same_group(gch, ch))
      {
         act(AT_GOLD, buf, ch, NULL, gch, TO_VICT);
         gch->gold += share;
      }
   }
   return;
}



void do_gtell(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *gch;

#ifndef SCRAMBLE
   int speaking = -1, lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (ch->speaking & lang_array[lang])
      {
         speaking = lang;
         break;
      }
#endif

   if (argument[0] == '\0')
   {
      send_to_char("Tell your group what?\n\r", ch);
      return;
   }

   if (xIS_SET(ch->act, PLR_NO_TELL))
   {
      send_to_char("Your message didn't get through!\n\r", ch);
      return;
   }

   /*
    * Note use of send_to_char, so gtell works on sleepers.
    */
/*    sprintf( buf, "%s tells the group '%s'.\n\r", ch->name, argument );*/
   for (gch = first_char; gch; gch = gch->next)
   {
      if (is_same_group(gch, ch))
      {
         set_char_color(AT_GTELL, gch);
         /* Groups unscrambled regardless of clan language.  Other languages
            still garble though. -- Altrag */
#ifndef SCRAMBLE
         if (speaking != -1 && (!IS_NPC(ch) || ch->speaking))
         {
            int speakswell = UMIN(knows_language(gch, ch->speaking, ch),
               knows_language(ch, ch->speaking, gch));

            if (speakswell < 85)
               ch_printf(gch, "%s tells the group '%s'.\n\r", PERS_MAP(ch, gch), translate(speakswell, argument, lang_names[speaking]));
            else
               ch_printf(gch, "%s tells the group '%s'.\n\r", PERS_MAP(ch, gch), argument);
         }
         else
            ch_printf(gch, "%s tells the group '%s'.\n\r", PERS_MAP(ch, gch), argument);
#else
         if (knows_language(gch, ch->speaking, gch) || (IS_NPC(ch) && !ch->speaking))
            ch_printf(gch, "%s tells the group '%s'.\n\r", PERS_MAP(ch, gch), argument);
         else
            ch_printf(gch, "%s tells the group '%s'.\n\r", PERS_MAP(ch, gch), scramble(argument, ch->speaking));
#endif
      }
   }

   return;
}


/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group(CHAR_DATA * ach, CHAR_DATA * bch)
{
   if (ach->leader)
      ach = ach->leader;
   if (bch->leader)
      bch = bch->leader;
   return ach == bch;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */

void talk_auction(char *argument)
{
   DESCRIPTOR_DATA *d;
   char buf[MSL];
   CHAR_DATA *original;

   sprintf(buf, "Auction: %s", argument); /* last %s to reset color */

   for (d = first_descriptor; d; d = d->next)
   {
      original = d->original ? d->original : d->character; /* if switched */
      if ((d->connected == CON_PLAYING) && !IS_SET(original->deaf, CHANNEL_AUCTION)
         && !xIS_SET(original->in_room->room_flags, ROOM_SILENCE) 
         && !wIS_SET(original, ROOM_SILENCE) && !NOT_AUTHED(original))
         act(AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR);
   }
}

/*
 * Language support functions. -- Altrag
 * 07/01/96
 *
 * Modified to return how well the language is known 04/04/98 - Thoric
 * Currently returns 100% for known languages... but should really return
 * a number based on player's wisdom (maybe 50+((25-wisdom)*2) ?)
 */
int knows_language(CHAR_DATA * ch, int language, CHAR_DATA * cch)
{
   sh_int sn;

   if (!IS_NPC(ch) && IS_IMMORTAL(ch))
      return 100;
   if (IS_NPC(ch) && !ch->speaks) /* No langs = knows all for npcs */
      return 100;
   if (IS_NPC(ch) && IS_SET(ch->speaks, (language & ~LANG_CLAN)))
      return 100;
   /* everyone KNOWS common tongue */
   if (IS_SET(language, LANG_COMMON))
      return 100;
   if (language & LANG_CLAN)
   {
      /* Clan = common for mobs.. snicker.. -- Altrag */
      if (IS_NPC(ch) || IS_NPC(cch))
         return 100;
      if (ch->pcdata->clan == cch->pcdata->clan && ch->pcdata->clan != NULL)
         return 100;
   }
   if (!IS_NPC(ch))
   {
      int lang;

      /* Racial languages for PCs */
      if (IS_SET(race_table[ch->race]->language, language))
         return 100;

      for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
         if (IS_SET(language, lang_array[lang]) && IS_SET(ch->speaks, lang_array[lang]))
         {
            if ((sn = skill_lookup(lang_names[lang])) != -1)
               return ch->pcdata->learned[sn] * 5;
         }
   }
   return 0;
}

bool can_learn_lang(CHAR_DATA * ch, int language)
{
   if (language & LANG_CLAN)
      return FALSE;
   if (IS_NPC(ch) || IS_IMMORTAL(ch))
      return FALSE;
   if (race_table[ch->race]->language & language)
      return FALSE;
   if (ch->speaks & language)
   {
      int lang;

      for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
         if (language & lang_array[lang])
         {
            int sn;

            if (!(VALID_LANGS & lang_array[lang]))
               return FALSE;
            if ((sn = skill_lookup(lang_names[lang])) < 0)
            {
               bug("Can_learn_lang: valid language without sn: %d", lang);
               continue;
            }
            if (ch->pcdata->learned[sn] >= 99)
               return FALSE;
         }
   }
   if (VALID_LANGS & language)
      return TRUE;
   return FALSE;
}

int const lang_array[] = {
   LANG_COMMON, LANG_ELVEN, LANG_DWARVEN, LANG_OGRE,
   LANG_HOBBIT, LANG_FAIRY, LANG_TROLLISH, LANG_RODENT,
   LANG_INSECTOID, LANG_MAMMAL, LANG_REPTILE,
   LANG_DRAGON, LANG_SPIRITUAL, LANG_MAGICAL,
   LANG_GOBLIN, LANG_GOD, LANG_ANCIENT, LANG_HALFLING,
   LANG_CLAN, LANG_GITH, LANG_UNKNOWN
};

char *const lang_names[] = {
   "common", "elvish", "dwarven", "ogre", "hobbit",
   "fairy", "trollese", "rodent", "insectoid",
   "mammal", "reptile", "dragon", "spiritual",
   "magical", "goblin", "god", "ancient",
   "halfling", "clan", "gith", ""
};


/* Note: does not count racial language.  This is intentional (for now). */
int countlangs(int languages)
{
   int numlangs = 0;
   int looper;

   for (looper = 0; lang_array[looper] != LANG_UNKNOWN; looper++)
   {
      if (lang_array[looper] == LANG_CLAN)
         continue;
      if (languages & lang_array[looper])
         numlangs++;
   }
   return numlangs;
}

void do_speak(CHAR_DATA * ch, char *argument)
{
   int langs;
   char arg[MIL];

   argument = one_argument(argument, arg);

   if (!str_cmp(arg, "all") && IS_IMMORTAL(ch))
   {
      set_char_color(AT_SAY, ch);
      ch->speaking = ~LANG_CLAN;
      send_to_char("Now speaking all languages.\n\r", ch);
      return;
   }
   for (langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++)
      if (!str_prefix(arg, lang_names[langs]))
         if (knows_language(ch, lang_array[langs], ch))
         {
            if (lang_array[langs] == LANG_CLAN && (IS_NPC(ch) || !ch->pcdata->clan))
               continue;
            ch->speaking = lang_array[langs];
            set_char_color(AT_SAY, ch);
            ch_printf(ch, "You now speak %s.\n\r", lang_names[langs]);
            return;
         }
   set_char_color(AT_SAY, ch);
   send_to_char("You do not know that language.\n\r", ch);
}

void do_languages(CHAR_DATA * ch, char *argument)
{
   int lang;

   for (lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++)
      if (knows_language(ch, lang_array[lang], ch))
      {
         if (ch->speaking & lang_array[lang] || (IS_NPC(ch) && !ch->speaking))
            set_char_color(AT_SAY, ch);
         else
            set_char_color(AT_PLAIN, ch);
         send_to_char(lang_names[lang], ch);
         send_to_char("\n\r", ch);
      }
   send_to_char("\n\r", ch);
   return;
}

void do_wartalk(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_WARTALK, "war");
   return;
}

void do_racetalk(CHAR_DATA * ch, char *argument)
{
   if (NOT_AUTHED(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   talk_channel(ch, argument, CHANNEL_RACETALK, "racetalk");
   return;
}


void init_profanity_checker()
{

}

void add_profane_word(char *word)
{
#ifndef WIN32
   char _word[4096];
   int i, j;

   j = 0;
   _word[j] = '\\';
   j++;
   _word[j] = '|';
   j++;
   _word[j] = '\\';
   j++;
   _word[j] = '(';
   j++;
   for (i = 0; i < strlen(word); i++)
   {
      _word[j] = '[';
      j++;
      _word[j] = tolower(word[i]);
      j++;
      _word[j] = toupper(word[i]);
      j++;
      _word[j] = ']';
      j++;
      _word[j] = '+';
      j++;
      _word[j] = '[';
      j++;



      _word[j] = '-';
      j++;
      _word[j] = ' ';
      j++;
      _word[j] = '\t';
      j++;

      _word[j] = '`';
      j++;
      _word[j] = '~';
      j++;
      _word[j] = '1';
      j++;
      _word[j] = '!';
      j++;
      _word[j] = '2';
      j++;
      _word[j] = '@';
      j++;
      _word[j] = '3';
      j++;
      _word[j] = '#';
      j++;
      _word[j] = '4';
      j++;
      _word[j] = '5';
      j++;
      _word[j] = '%';
      j++;
      _word[j] = '6';
      j++;
      _word[j] = '7';
      j++;
      _word[j] = '&';
      j++;
      _word[j] = '8';
      j++;
      _word[j] = '9';
      j++;
      _word[j] = '0';
      j++;
      _word[j] = '_';
      j++;
      _word[j] = ';';
      j++;
      _word[j] = ':';
      j++;
      _word[j] = ',';
      j++;
      _word[j] = '<';
      j++;
      /* These need to be escaped  for C */


      _word[j] = '\'';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '\"';
      j++;

      /* These need to be escaped  for regex */
      _word[j] = '\\';
      j++;
      _word[j] = '$';
      j++;

      _word[j] = '>';
      j++;
      _word[j] = '/';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '^';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '.';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = ')';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '?';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '*';
      j++;

      _word[j] = '\\';
      j++;
      _word[j] = '(';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '[';
      j++;

      _word[j] = '\\';
      j++;
      _word[j] = '{';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '+';
      j++;

#ifdef BIG
      /* i don't get what the deal is with this guy, it seems unescapable,
         so to speak. */
      _word[j] = '\\';
      j++;
      _word[j] = ']';
      j++;
#endif
      _word[j] = '\\';
      j++;
      _word[j] = '}';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '|';
      j++;
      _word[j] = '\\';
      j++;
      _word[j] = '=';
      j++;

      /* close up funny characters */
      _word[j] = ']';
      j++;
      _word[j] = '*';
      j++;
   }
   _word[j] = '\\';
   j++;
   _word[j] = ')';
   j++;
   _word[j] = '\0';

   strcat(bigregex, _word);
#endif
}

int is_profane(char *what)
{
   return (0);
}
