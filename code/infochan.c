/*
 *  Simple Info channel
 *  Author: Rantic (supfly@geocities.com)
 *  of FrozenMUD (empire.digiunix.net 4000)
 *
 *  Permission to use and distribute this code is granted provided
 *  this header is retained and unaltered, and the distribution
 *  package contains all the original files unmodified.
 *  If you modify this code and use/distribute modified versions
 *  you must give credit to the original author(s).
 */
#include <stdio.h>
#include "mud.h"

void talk_info(sh_int AT_COLOR, char *argument)
{
   DESCRIPTOR_DATA *d;
   char buf[MSL];
   CHAR_DATA *original;
   int position;

   sprintf(buf, "%s", argument);

   for (d = first_descriptor; d; d = d->next)
   {
      original = d->original ? d->original : d->character;
      if ((d->connected == CON_PLAYING) && !IS_SET(original->deaf, CHANNEL_INFO)
         && !xIS_SET(original->in_room->room_flags, ROOM_SILENCE) 
         && !wIS_SET(original, ROOM_SILENCE) && !NOT_AUTHED(original))
      {
         position = original->position;
         original->position = POS_STANDING;
         act(AT_COLOR, buf, original, NULL, NULL, TO_CHAR);
         original->position = position;
      }
   }
}
