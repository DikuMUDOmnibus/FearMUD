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
 *			   Player movement module			    *
 ****************************************************************************/


#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "mud.h"


//Such fun -- Xerves
const sh_int movement_loss[SECT_MAX] = {
   1, 2, 2, 3, 4, 6, 4, 5, 6, 10, 6, 5, 7, 4, 1, 2, 6, 6, 2, 2, 3, 3, 6, 6, 6, 6, 2, 2, 2, 2,
   5, 4, 1, 3, 5, 10, 8, 0, 0, 0, 0, 0, 0, 5, 1, 2, 1, 1, 0
};

char *const dir_name[] = {
   "north", "east", "south", "west", "up", "down",
   "northeast", "northwest", "southeast", "southwest", "somewhere"
};

const int trap_door[] = {
   TRAP_N, TRAP_E, TRAP_S, TRAP_W, TRAP_U, TRAP_D,
   TRAP_NE, TRAP_NW, TRAP_SE, TRAP_SW
};


const sh_int rev_dir[] = {
   2, 3, 0, 1, 5, 4, 9, 8, 7, 6, 10
};


ROOM_INDEX_DATA *vroom_hash[64];


/*
 * Local functions.
 */
OBJ_DATA *has_key args((CHAR_DATA * ch, int key));

/* Overland Map movement - Samson 7-31-99 */
void map_north args((CHAR_DATA * ch));
void map_south args((CHAR_DATA * ch));
void map_east args((CHAR_DATA * ch));
void map_west args((CHAR_DATA * ch));
void map_up args((CHAR_DATA * ch));
void map_down args((CHAR_DATA * ch));
void map_ne args((CHAR_DATA * ch));
void map_nw args((CHAR_DATA * ch));
void map_se args((CHAR_DATA * ch));
void map_sw args((CHAR_DATA * ch));


char *const sect_names[SECT_MAX][2] = {
   {"In a room", "inside"}, {"In a city", "cities"},
   {"In a field", "fields"}, {"In a forest", "forests"},
   {"hill", "hills"}, {"On a mountain", "mountains"},
   {"In the water", "waters"}, {"In rough water", "waters"},
   {"Underwater", "underwaters"}, {"In the air", "air"},
   {"In a desert", "deserts"}, {"Somewhere", "unknown"},
   {"ocean floor", "ocean floor"}, {"underground", "underground"}
};


const int sent_total[SECT_MAX] = {
   3, 5, 4, 4, 1, 1, 1, 1, 1, 2, 2, 25, 1, 1
};

char *const room_sents[SECT_MAX][25] = {
   {
         "rough hewn walls of granite with the occasional spider crawling around",
         "signs of a recent battle from the bloodstains on the floor",
      "a damp musty odour not unlike rotting vegetation"},

   {
         "the occasional stray digging through some garbage",
         "merchants trying to lure customers to their tents",
         "some street people putting on an interesting display of talent",
         "an argument between a customer and a merchant about the price of an item",
      "several shady figures talking down a dark alleyway"},

   {
         "sparce patches of brush and shrubs",
         "a small cluster of trees far off in the distance",
         "grassy fields as far as the eye can see",
      "a wide variety of weeds and wildflowers"},

   {
         "tall, dark evergreens prevent you from seeing very far",
         "many huge oak trees that look several hundred years old",
         "a solitary lonely weeping willow",
      "a patch of bright white birch trees slender and tall"},

   {
      "rolling hills lightly speckled with violet wildflowers"},

   {
      "the rocky mountain pass offers many hiding places"},

   {
      "the water is smooth as glass"},

   {
      "rough waves splash about angrily"},

   {
      "a small school of fish"},

   {
         "the land far below",
      "a misty haze of clouds"},

   {
         "sand as far as the eye can see",
      "an oasis far in the distance"},

   {
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual", "nothing unusual", "nothing unusual",
         "nothing unusual",
      },

   {
      "rocks and coral which litter the ocean floor."},

   {
      "a lengthy tunnel of rock."}

};

char *grab_word(char *argument, char *arg_first)
{
   char cEnd;
   sh_int count;

   count = 0;

   while (isspace(*argument))
      argument++;

   cEnd = ' ';
   if (*argument == '\'' || *argument == '"')
      cEnd = *argument++;

   while (*argument != '\0' || ++count >= 255)
   {
      if (*argument == cEnd)
      {
         argument++;
         break;
      }
      *arg_first++ = *argument++;
   }
   *arg_first = '\0';

   while (isspace(*argument))
      argument++;

   return argument;
}

char *wordwrap(char *txt, sh_int wrap)
{
   static char buf[MSL];
   char *bufp;

   buf[0] = '\0';
   bufp = buf;
   if (txt != NULL)
   {
      char line[MSL];
      char temp[MSL];
      char *ptr, *p;
      int ln, x;

      ++bufp;
      line[0] = '\0';
      ptr = txt;
      while (*ptr)
      {
         ptr = grab_word(ptr, temp);
         ln = strlen(line);
         x = strlen(temp);
         if ((ln + x + 1) < wrap)
         {
            if (ln > 0 && line[ln - 1] == '.')
               strcat(line, "  ");
            else
               strcat(line, " ");
            strcat(line, temp);
            p = strchr(line, '\n');
            if (!p)
               p = strchr(line, '\r');
            if (p)
            {
               strcat(buf, line);
               line[0] = '\0';
            }
         }
         else
         {
            strcat(line, "\r\n");
            strcat(buf, line);
            strcpy(line, temp);
         }
      }
      if (line[0] != '\0')
         strcat(buf, line);
   }
   return bufp;
}

void decorate_room(ROOM_INDEX_DATA * room)
{
   char buf[MSL];
   char buf2[MSL];
   int nRand;
   int iRand, len;
   int previous[8];
   int sector = room->sector_type;
   char *pre = "You notice ", *post = ".";

   if (room->name)
      STRFREE(room->name);
   if (room->description)
      STRFREE(room->description);

   room->name = STRALLOC("In a virtual room");
   room->description = STRALLOC("You're on a pathway.\n\r");

   return; /* Return with above till I can figure out what is
              * wrong with it. --Shaddai
            */
   room->name = STRALLOC(sect_names[sector][0]);
   buf[0] = '\0';
   nRand = number_range(1, UMIN(8, sent_total[sector]));

   for (iRand = 0; iRand < nRand; iRand++)
      previous[iRand] = -1;

   for (iRand = 0; iRand < nRand; iRand++)
   {
      while (previous[iRand] == -1)
      {
         int x, z;

         x = number_range(0, sent_total[sector] - 1);

         for (z = 0; z < iRand; z++)
            if (previous[z] == x)
               break;

         if (z < iRand)
            continue;

         previous[iRand] = x;

         len = strlen(buf);
         if (len == 0)
         {
            switch (number_range(1, 2 * (iRand == nRand - 1) ? 1 : 2))
            {
               case 1:
                  pre = "You notice ";
                  post = ".";
                  break;
               case 2:
                  pre = "You see ";
                  post = ".";
                  break;
               case 3:
                  pre = "You see ";
                  post = ", and ";
                  break;
               case 4:
                  pre = "You notice ";
                  post = ", and ";
                  break;
            }
            sprintf(buf2, "%s%s%s", pre, room_sents[sector][x], post);
         }
         else if (iRand != nRand - 1)
         {
            if (buf[len - 1] == '.')
               switch (number_range(0, 3))
               {
                  case 0:
                     pre = "you notice ";
                     post = ".";
                     break;
                  case 1:
                     pre = "you see ";
                     post = ", and ";
                     break;
                  case 2:
                     pre = "you see ";
                     post = ".";
                     break;
                  case 3:
                     pre = "over yonder ";
                     post = ", and ";
                     break;
               }
            else
               switch (number_range(0, 3))
               {
                  case 0:
                     pre = "";
                     post = ".";
                     break;
                  case 1:
                     pre = "";
                     post = " not too far away.";
                     break;
                  case 2:
                     pre = "";
                     post = ", and ";
                     break;
                  case 3:
                     pre = "";
                     post = " nearby.";
                     break;
               }
            sprintf(buf2, "%s%s%s", pre, room_sents[sector][x], post);
         }
         else
            sprintf(buf2, "%s.", room_sents[sector][x]);
         if (len > 5 && buf[len - 1] == '.')
         {
            strcat(buf, "  ");
            buf2[0] = UPPER(buf2[0]);
         }
         else if (len == 0)
            buf2[0] = UPPER(buf2[0]);
         strcat(buf, buf2);
      }
   }
   /* Below is the line that causes the uninitialized memory read --Shaddai */
   sprintf(buf2, "%s\n\r", wordwrap(buf, 78));
   room->description = STRALLOC(buf2);
}

/*
 * Remove any unused virtual rooms				-Thoric
 */
void clear_vrooms()
{
   int hash;
   ROOM_INDEX_DATA *room, *room_next, *prev;

   for (hash = 0; hash < 64; hash++)
   {
      while (vroom_hash[hash] && !vroom_hash[hash]->first_person && !vroom_hash[hash]->first_content)
      {
         room = vroom_hash[hash];
         vroom_hash[hash] = room->next;
         clean_room(room);
         DISPOSE(room);
         --top_vroom;
      }
      prev = NULL;
      for (room = vroom_hash[hash]; room; room = room_next)
      {
         room_next = room->next;
         if (!room->first_person && !room->first_content)
         {
            if (prev)
               prev->next = room_next;
            clean_room(room);
            DISPOSE(room);
            --top_vroom;
         }
         if (room)
            prev = room;
      }
   }
}

char *rev_exit(sh_int vdir)
{
   switch (vdir)
   {
      default:
         return "somewhere";
      case 0:
         return "the south";
      case 1:
         return "the west";
      case 2:
         return "the north";
      case 3:
         return "the east";
      case 4:
         return "below";
      case 5:
         return "above";
      case 6:
         return "the southwest";
      case 7:
         return "the southeast";
      case 8:
         return "the northwest";
      case 9:
         return "the northeast";
   }

   return "<???>";
}

/*
 * Function to get the equivelant exit of DIR 0-MAXDIR out of linked list.
 * Made to allow old-style diku-merc exit functions to work.	-Thoric
 */
EXIT_DATA *get_exit(ROOM_INDEX_DATA * room, sh_int dir)
{
   EXIT_DATA *xit;

   if (!room)
   {
      bug("Get_exit: NULL room", 0);
      return NULL;
   }

   for (xit = room->first_exit; xit; xit = xit->next)
      if (xit->vdir == dir)
         return xit;
   return NULL;
}

/*
 * Function to get an exit, leading the the specified room
 */
EXIT_DATA *get_exit_to(ROOM_INDEX_DATA * room, sh_int dir, int vnum)
{
   EXIT_DATA *xit;

   if (!room)
   {
      bug("Get_exit: NULL room", 0);
      return NULL;
   }

   for (xit = room->first_exit; xit; xit = xit->next)
      if (xit->vdir == dir && xit->vnum == vnum)
         return xit;
   return NULL;
}

/*
 * Function to get the nth exit of a room			-Thoric
 */
EXIT_DATA *get_exit_num(ROOM_INDEX_DATA * room, sh_int count)
{
   EXIT_DATA *xit;
   int cnt;

   if (!room)
   {
      bug("Get_exit: NULL room", 0);
      return NULL;
   }

   for (cnt = 0, xit = room->first_exit; xit; xit = xit->next)
      if (++cnt == count)
         return xit;
   return NULL;
}


/*
 * Modify movement due to encumbrance				-Thoric
 */
sh_int encumbrance(CHAR_DATA * ch, sh_int move)
{
   int cur, max;

   max = can_carry_w(ch);
   cur = get_ch_carry_weight(ch);
   if (cur >= max)
      return move * 4;
   else if (cur >= max * 0.95)
      return move * 3.5;
   else if (cur >= max * 0.90)
      return move * 3;
   else if (cur >= max * 0.85)
      return move * 2.5;
   else if (cur >= max * 0.80)
      return move * 2;
   else if (cur >= max * 0.75)
      return move * 1.5;
   else
      return move;
}


/*
 * Check to see if a character can fall down, checks for looping   -Thoric
 */
bool will_fall(CHAR_DATA * ch, int fall)
{
   if (xIS_SET(ch->in_room->room_flags, ROOM_NOFLOOR)
      && CAN_GO(ch, DIR_DOWN) && (!IS_AFFECTED(ch, AFF_FLYING) || (ch->mount && !IS_AFFECTED(ch->mount, AFF_FLYING))))
   {
      if (fall > 80)
      {
         bug("Falling (in a loop?) more than 80 rooms: vnum %d", ch->in_room->vnum);
         char_from_room(ch);
         char_to_room(ch, get_room_index(ROOM_VNUM_TEMPLE));
         fall = 0;
         return TRUE;
      }
      set_char_color(AT_FALLING, ch);
      send_to_char("You're falling down...^x\n\r", ch);
      move_char(ch, get_exit(ch->in_room, DIR_DOWN), ++fall);
      return TRUE;
   }
   return FALSE;
}


/*
 * create a 'virtual' room					-Thoric
 */
ROOM_INDEX_DATA *generate_exit(ROOM_INDEX_DATA * in_room, EXIT_DATA * pexit)
{
   EXIT_DATA *xit;
   EXIT_DATA *orig_exit = pexit;
   ROOM_INDEX_DATA *room, *backroom;
   int brvnum;
   int serial;
   int roomnum;
   int distance = -1;
   int vdir = orig_exit->vdir;
   sh_int hash;
   bool found = FALSE;

   int r1 = in_room->vnum;
   int r2 = orig_exit->vnum;

   brvnum = r1;
   backroom = in_room;
   serial = (UMAX(r1, r2) << 16) | UMIN(r1, r2);
   distance = orig_exit->distance - 1;
   roomnum = r1 < r2 ? 1 : distance;   
   hash = serial % 64;

   for (room = vroom_hash[hash]; room; room = room->next)
      if (room->vnum == serial && room->tele_vnum == roomnum)
      {
         found = TRUE;
         break;
      }
   if (!found)
   {
      CREATE(room, ROOM_INDEX_DATA, 1);
      room->area = in_room->area;
      room->vnum = serial;
      room->tele_vnum = roomnum;
      room->sector_type = in_room->sector_type;
      room->room_flags = in_room->room_flags;
      decorate_room(room);
      room->next = vroom_hash[hash];
      vroom_hash[hash] = room;
      ++top_vroom;
   }
   if (!found || (xit = get_exit(room, vdir)) == NULL)
   {
      xit = make_exit(room, orig_exit->to_room, vdir);
      xit->keyword = STRALLOC("");
      xit->description = STRALLOC("");
      xit->key = -1;
      xit->distance = distance;
   }
   if (!found)
   {
      bug("Trying to create a virtual room exit?");
   }
   pexit = xit;
   return room;
}

int get_hunt_cost(ROOM_INDEX_DATA *room)
{
   int cost;
   
   cost = movement_loss[UMIN(SECT_MAX - 1, room->sector_type)];
   
   cost = number_range(cost*.4, cost*.6);
   if (cost < 1)
      cost = 1;
      
   cost += number_range(1, 2);
   return cost;
}

int is_nighttime()
{
   if (gethour() > 21 || gethour() < 6)
      return 1;
   return 0;
}
int check_stalk_move(CHAR_DATA *ch, ROOM_INDEX_DATA *room)
{
   AFFECT_DATA *paf;
   int level;
   int plevel;
   CHAR_DATA *victim;
   int prawl = 0;
   int fst = 0;
   int finfra = 0;
   int cnt = 0;
   
   if (IS_NPC(ch))
      return 1;
   for (paf = ch->first_affect; paf; paf = paf->next)
   {    
      if (paf->type == gsn_stalk)
         break;
   }
   if (!paf)
      return 1;
   level = paf->modifier;
   paf->duration = 10+level/2;
   if (!is_nighttime() && !room_is_dark(room) && ch->pcdata->learned[gsn_lightprawl] <= 0)
   {
      level /= 3;
   }
   if (!is_nighttime() && !room_is_dark(room) && ch->pcdata->learned[gsn_lightprawl] > 0)
   {
      level /= 2;
      prawl = gsn_lightprawl;
   }
   if (is_nighttime() && room_is_dark(room))
   {
      prawl = gsn_nightprawl;
   }
   for (victim = room->first_person; victim; victim = victim->next_in_room)
   {
      if (victim == ch || is_same_group(victim, ch) || victim->master == ch)
         continue;
      if (IN_WILDERNESS(victim) && !IN_SAME_ROOM(ch, victim))
         continue;
      if (++cnt > 1)
         level = level * 90 / 100;
      if (!finfra && is_nighttime() && room_is_dark(room) && (IS_AFFECTED(victim, AFF_INFRARED) || IS_AFFECTED(victim, AFF_TRUESIGHT)))
      {
         level /= 2;
         finfra++;
      }
      if (prawl > 0 && !fst)
      {
         learn_from_success(ch, prawl, NULL);
         fst++;
         plevel = POINT_LEVEL(GET_POINTS(ch, prawl, 0, 1), GET_MASTERY(ch, prawl, 0, 1));
         level += UMIN(15, plevel/4);
      }
   }
   level = 35+UMIN(60, level*6/8);
   level += (get_curr_lck(ch)-14)/2;
   level = URANGE(25, level, 95);
   if (cnt == 0 && IN_WILDERNESS(ch))
   {
      if (number_range(1, 100) > 85)
         cnt = 1;
   }
   if (number_range(1, 100) <= level || cnt == 0)
   {
      send_to_char("You successfully stalk into the next room.\n\r", ch);
      learn_from_success(ch, gsn_stalk, NULL);
      return 1;
   }
   else
   {
      send_to_char("You fail to silently stalk into the next room.\n\r", ch);
      learn_from_failure(ch, gsn_stalk, NULL);
      return 0;
   }
}     
   
   
int check_hide_move(CHAR_DATA *ch, int sector, ROOM_INDEX_DATA *room)
{
   int prawl = -1;
   int level;
   
   if (IS_NPC(ch))
      return 0;
      
   if (ch->pcdata->learned[gsn_prawl] <= 0 && ch->pcdata->learned[gsn_nightprawl] <= 0
   &&  ch->pcdata->learned[gsn_lightprawl] <= 0)
      return 0;
      
   if (sector == SECT_ROAD || sector == SECT_PATH) 
   {
      if (ch->pcdata->learned[gsn_nightprawl] <= 0 &&  ch->pcdata->learned[gsn_lightprawl] <= 0)
         return 0;
      if (!is_nighttime() && !room_is_dark(room) && ch->pcdata->learned[gsn_lightprawl] <= 0)
         return 0;
      if (!is_nighttime() && !room_is_dark(room))   
         prawl = gsn_lightprawl;
      else
      {
         if (ch->pcdata->learned[gsn_lightprawl] <= 0)
            prawl = gsn_nightprawl;
         else
            prawl = gsn_lightprawl;
      }
   }
   else
   {
      if (!is_nighttime() && !room_is_dark(room) && ch->pcdata->learned[gsn_lightprawl] <= 0)
         return 0;
      if (!is_nighttime() && !room_is_dark(room))  
         prawl = gsn_lightprawl;
      else
      {
         if (ch->pcdata->learned[gsn_lightprawl] <= 0)
         {
            if (ch->pcdata->learned[gsn_nightprawl] <= 0)
            {
               prawl = gsn_prawl;
            }
            else
            {
               prawl = gsn_nightprawl;
            }
         }
         else
         {
            prawl = gsn_lightprawl;
         }
      }
   }
   if (prawl == -1)
   {
      bug("check_hide_move:  prawl == 0 for %s", ch->name);
      return 0;
   }
   learn_from_success(ch, prawl, NULL);
   level = POINT_LEVEL(GET_POINTS(ch, prawl, 0, 1), GET_MASTERY(ch, prawl, 0, 1));
   if (prawl == gsn_prawl)
      return 80+UMIN(10, level/6);
   else if (prawl == gsn_nightprawl)
      return 90+UMIN(5, level/12);
   else
      return 95+UMIN(5, level/12);
}

ch_ret move_char(CHAR_DATA * ch, EXIT_DATA * pexit, int fall)
{
   ROOM_INDEX_DATA *in_room;
   ROOM_INDEX_DATA *to_room;
   ROOM_INDEX_DATA *from_room;
   OBJ_DATA *climb = NULL;
   CHAR_DATA *gmob;
   OBJ_DATA *boat;
   char buf[MSL];
   char *txt;
   char *dtxt;
   ch_ret retcode;
   sh_int door, distance, pstatus;
   sh_int ht = -1;
   bool drunk = FALSE;
   bool nuisance = FALSE;
   bool brief = FALSE;
   int level;

   pstatus = 0;

   if (IS_NPC(ch) && (xIS_SET(ch->act, ACT_MILITARY) || xIS_SET(ch->act, ACT_KINGDOMMOB) || xIS_SET(ch->act, ACT_MOUNTSAVE)))
      ht = ch->m4;
   if (!IS_NPC(ch))
      ht = ch->pcdata->hometown;

   if (!IS_NPC(ch))
   {
      if (IS_DRUNK(ch, 2) && (ch->position != POS_SHOVE) && (ch->position != POS_DRAG))
         drunk = TRUE;

      /* Nuisance flag, makes them walk in random directions 50% of the time
       * -Shaddai
       */

      if (ch->pcdata->nuisance && ch->pcdata->nuisance->flags > 8 &&
         (ch->position != POS_SHOVE) && (ch->position != POS_DRAG) && number_percent() > (ch->pcdata->nuisance->flags * ch->pcdata->nuisance->power))
         nuisance = TRUE;
   }

   if ((nuisance || drunk) && !fall)
   {
      door = number_door();
      pexit = get_exit(ch->in_room, door);
   }
   
   if (!fall && ch->position != POS_SHOVE && ch->position != POS_DRAG && ch->con_rleg == -1 && ch->con_lleg == -1)
   {
      send_to_char("You aren't moving very far without any legs.\n\r", ch);
      return rNONE;
   }
   if (!fall && (IS_AFFECTED(ch, AFF_WEB) || IS_AFFECTED(ch, AFF_SNARE)))
   {
      send_to_char("You are currently bound to this spot and cannot move.\n\r", ch);
      return rNONE;
   }
   if (!fall && ch->position == POS_RIDING)
   {
      send_to_char("You cannot move when you are on someone's back.\n\r", ch);
      return rNONE;
   }
   //Break the riding if we are falling...hopefully...
   if (fall && ch->position == POS_RIDING)
   {
      if (ch->riding)
      {
         ch->rider = NULL;         
         ch->riding = NULL;
      }
      ch->position = POS_STANDING;
   }
      


#ifdef DEBUG
   if (pexit)
   {
      sprintf(buf, "move_char: %s to door %d", ch->name, pexit->vdir);
      log_string(buf);
   }
#endif

   retcode = rNONE;
   txt = NULL;

   if (!IS_NPC(ch))
      pstatus = check_room_pk(ch);

   if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MOUNTED))
      return retcode;

   in_room = ch->in_room;
   from_room = in_room;
   if (!pexit || (to_room = pexit->to_room) == NULL)
   {
      if (drunk && ch->position != POS_MOUNTED
         && ch->in_room->sector_type != SECT_WATER_SWIM
         && ch->in_room->sector_type != SECT_WATER_NOSWIM
         && ch->in_room->sector_type != SECT_UNDERWATER && ch->in_room->sector_type != SECT_OCEANFLOOR)
      {
         switch (number_bits(4))
         {
            default:
               act(AT_ACTION, "You drunkenly stumble into some obstacle.", ch, NULL, NULL, TO_CHAR);
               act(AT_ACTION, "$n drunkenly stumbles into a nearby obstacle.", ch, NULL, NULL, TO_ROOM);
               break;
            case 3:
               act(AT_ACTION, "In your drunken stupor you trip over your own feet and tumble to the ground.", ch, NULL, NULL, TO_CHAR);
               act(AT_ACTION, "$n stumbles drunkenly, trips and tumbles to the ground.", ch, NULL, NULL, TO_ROOM);
               ch->position = POS_RESTING;
               break;
            case 4:
               act(AT_SOCIAL, "You utter a string of slurred obscenities.", ch, NULL, NULL, TO_CHAR);
               act(AT_ACTION, "Something blurry and immovable has intercepted you as you stagger along.", ch, NULL, NULL, TO_CHAR);
               act(AT_HURT, "Oh geez... THAT really hurt.  Everything slowly goes dark and numb...", ch, NULL, NULL, TO_CHAR);
               act(AT_ACTION, "$n drunkenly staggers into something.", ch, NULL, NULL, TO_ROOM);
               act(AT_SOCIAL, "$n utters a string of slurred obscenities: @*&^%@*&!", ch, NULL, NULL, TO_ROOM);
               act(AT_ACTION, "$n topples to the ground with a thud.", ch, NULL, NULL, TO_ROOM);
               ch->position = POS_INCAP;
               break;
         }
      }
      else if (nuisance)
         act(AT_ACTION, "You stare around trying to remember where you where going.", ch, NULL, NULL, TO_CHAR);
      else if (drunk)
         act(AT_ACTION, "You stare around trying to make sense of things through your drunken stupor.", ch, NULL, NULL, TO_CHAR);
      else
         send_to_char("Alas, you cannot go that way.\n\r", ch);
      return rSTOP;
   }

   door = pexit->vdir;
   distance = pexit->distance;

   /*
    * Exit is only a "window", there is no way to travel in that direction
    * unless it's a door with a window in it  -Thoric
    */
   if (IS_SET(pexit->exit_info, EX_WINDOW) && !IS_SET(pexit->exit_info, EX_ISDOOR))
   {
      send_to_char("Alas, you cannot go that way.\n\r", ch);
      return rSTOP;
   }
   /* Overland Map stuff - Samson 7-31-99 */
   if (IS_SET(pexit->exit_info, EX_OVERLAND))
   {
      /* Take away Hide */
      if (IS_AFFECTED(ch, AFF_HIDE))
      {
         if (number_range(1, 100) > check_hide_move(ch, -1, pexit->to_room))
         {
            xREMOVE_BIT(ch->affected_by, AFF_HIDE);
            act(AT_RED, "$n quickly appears from the shadows.", ch, NULL, NULL, TO_ROOM);
         }
      }
      if (IS_AFFECTED(ch, AFF_STALK))
      {
         if (!check_stalk_move(ch, pexit->to_room))
         {
            affect_strip(ch, gsn_stalk);
            act(AT_RED, "$n attemps to stalk silently into the room but fails!.", ch, NULL, NULL, TO_ROOM);
         }
      }
      enter_map(ch, pexit->coord->x, pexit->coord->y, pexit->vnum);
      if (!IS_NPC(ch))
         ch->pcdata->mapdir = pexit->vdir;
      return rSTOP;
   }
   if (ch->on != NULL)
   {
      act(AT_PLAIN, "Need to get off of $p before moving on.", ch, ch->on, NULL, TO_CHAR);
      return rSTOP;
   }
   if (IS_SET(pexit->exit_info, EX_PORTAL) && IS_NPC(ch))
   {
      act(AT_PLAIN, "Mobs can't use portals.", ch, NULL, NULL, TO_CHAR);
      return rSTOP;
   }

   if (IS_SET(pexit->exit_info, EX_NOMOB) && IS_NPC(ch))
   {
      act(AT_PLAIN, "Mobs can't enter there.", ch, NULL, NULL, TO_CHAR);
      return rSTOP;
   }

   if (IS_SET(pexit->exit_info, EX_CLOSED) && (!IS_AFFECTED(ch, AFF_PASS_DOOR) || IS_SET(pexit->exit_info, EX_NOPASSDOOR)))
   {
      if (!IS_SET(pexit->exit_info, EX_SECRET) && !IS_SET(pexit->exit_info, EX_DIG))
      {
         if (drunk)
         {
            act(AT_PLAIN, "$n runs into the $d in $s drunken state.", ch, NULL, pexit->keyword, TO_ROOM);
            act(AT_PLAIN, "You run into the $d in your drunken state.", ch, NULL, pexit->keyword, TO_CHAR);
         }
         else
            act(AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR);
      }
      else
      {
         if (drunk)
            send_to_char("You stagger around in your drunken state.\n\r", ch);
         else
            send_to_char("Alas, you cannot go that way.\n\r", ch);
      }

      return rSTOP;
   }

   /*
    * Crazy virtual room idea, created upon demand.  -Thoric
    */
   if (distance > 1)
      if ((to_room = generate_exit(in_room, pexit)) == NULL)
         send_to_char("Alas, you cannot go that way.\n\r", ch);

   if (!fall && IS_AFFECTED(ch, AFF_CHARM) && ch->master && in_room == ch->master->in_room)
   {
      send_to_char("What?  And leave your beloved master?\n\r", ch);
      return rSTOP;
   }

   if (room_is_private(to_room))
   {
      send_to_char("That room is private right now.\n\r", ch);
      return rSTOP;
   }

   if (!IS_NPC(ch))
   {
      if (xIS_SET(to_room->room_flags, ROOM_IMP) && ch->pcdata->caste < LEVEL_STAFF)
      {
         send_to_char("You may not enter this room, it is for STAFF only!\n\r", ch);
         return rSTOP;
      }
   }
   else
   {
      if (xIS_SET(to_room->room_flags, ROOM_IMP))
         return rSTOP;
   }
   if (IS_NPC(ch) && xIS_SET(to_room->room_flags, ROOM_NOMILITARY) && xIS_SET(ch->act, ACT_MILITARY))
   {
      do_say(ch, "Cannot move toward that room, military is not allowed in.");
      return rSTOP;
   }
   if (!IS_NPC(ch))
   {
      if (xIS_SET(ch->act, PLR_GAMBLER))
      {
         send_to_char("I don't think leaving while gambling is a good idea!\n\r", ch);
         return rSTOP;
      }
   }

   if (!fall && !IS_NPC(ch))
   {
      /*int iClass; */
      int move;


      if (to_room->sector_type == SECT_MOUNTAIN && get_trust(ch) < LEVEL_IMMORTAL)
      {
         if (MASTERED(ch, gsn_mountain_climb) < 1)
         {
            if ((climb = get_objtype(ch, ITEM_MCLIMB)) == NULL)
            {
               send_to_char("A huge, unclimbable mountain blocks your path in that direction.\n\r", ch);
               return rSTOP;
            }
         }
      }


      if (in_room->sector_type == SECT_AIR || to_room->sector_type == SECT_AIR || IS_SET(pexit->exit_info, EX_FLY))
      {
         if (ch->mount && !IS_AFFECTED(ch->mount, AFF_FLYING))
         {
            send_to_char("Your mount can't fly.\n\r", ch);
            return rSTOP;
         }
         if (!ch->mount && !IS_AFFECTED(ch, AFF_FLYING))
         {
            send_to_char("You'd need to fly to go there.\n\r", ch);
            return rSTOP;
         }
      }

      if (in_room->sector_type == SECT_WATER_NOSWIM || to_room->sector_type == SECT_WATER_NOSWIM
      ||  in_room->sector_type == SECT_RIVER || to_room->sector_type == SECT_RIVER)
      {
         if ((ch->mount && !IS_FLOATING(ch->mount)) || !IS_FLOATING(ch))
         {
            /*
             * Look for a boat.
             * We can use the boat obj for a more detailed description.
             */
            if ((boat = get_objtype(ch, ITEM_BOAT)) != NULL)
            {
               if (drunk)
                  txt = "paddles unevenly";
               else
                  txt = "paddles";
            }
            else if (!IS_NPC(ch) && ch->pcdata->learned[gsn_swimming] >= 0)
            {
               if (ch->mount)
               {
                  send_to_char("You need to leave your mount behind if you wish to swim.\n\r", ch);
                  return rSTOP;
               }
               else
               {
                  level = POINT_LEVEL(GET_POINTS(ch, gsn_swimming, 0, 1), GET_MASTERY(ch, gsn_swimming, 0, 1));
                  if (number_range(1, 100) > 40+level)
                  {
                     send_to_char("You struggle to move forward.\n\r", ch);
                     WAIT_STATE(ch, number_range(3, 6));
                     learn_from_failure(ch, gsn_swimming, NULL);
                     return rSTOP;
                  }
                  else
                  {
                     if (drunk)
                        txt = "swims unevenly";
                     else
                        txt = "swims";   
                     learn_from_success(ch, gsn_swimming, NULL);
                  }
               }
            }
            else
            {
               if (ch->mount)
                  send_to_char("Your mount would drown!\n\r", ch);
               else
                  send_to_char("You'd need a boat to go there.\n\r", ch);
               return rSTOP;
            }
         }
      }

      if (IS_SET(pexit->exit_info, EX_CLIMB))
      {
         bool found;
         sh_int mastery;

         mastery = MASTERED(ch, gsn_climb) * 20;
         mastery = mastery - 40;

         found = FALSE;
         if (ch->mount && IS_AFFECTED(ch->mount, AFF_FLYING))
            found = TRUE;
         else if (IS_AFFECTED(ch, AFF_FLYING))
            found = TRUE;

         if (!found && !ch->mount)
         {
             /* Take away Hide */
            if (IS_AFFECTED(ch, AFF_HIDE))
            {
               if (number_range(1, 100) > check_hide_move(ch, to_room->sector_type, pexit->to_room))
               {
                  xREMOVE_BIT(ch->affected_by, AFF_HIDE);
                  act(AT_RED, "$n quickly appears from the shadows.", ch, NULL, NULL, TO_ROOM);
               }
            }
            if (IS_AFFECTED(ch, AFF_STALK))
            {
               if (!check_stalk_move(ch, pexit->to_room))
               {
                  affect_strip(ch, gsn_stalk);
                  act(AT_RED, "$n attemps to stalk silently into the room but fails!.", ch, NULL, NULL, TO_ROOM);
               }
            }
            if ((!IS_NPC(ch) && (number_percent() - mastery) > 90) || drunk || ch->mental_state < -90)
            {
               send_to_char("You start to climb... but lose your grip and fall!\n\r", ch);
               learn_from_failure(ch, gsn_climb, NULL);
               if (pexit->vdir == DIR_DOWN)
               {
                  retcode = move_char(ch, pexit, 1);
                  return retcode;
               }
               set_char_color(AT_HURT, ch);
               send_to_char("OUCH! You hit the ground!\n\r", ch);
               WAIT_STATE(ch, 20);
               retcode = damage(ch, ch, (pexit->vdir == DIR_UP ? 10 : 5), TYPE_UNDEFINED, 0, -1);
               return retcode;
            }
            found = TRUE;
            learn_from_success(ch, gsn_climb, NULL);
            WAIT_STATE(ch, skill_table[gsn_climb]->beats*2);
            txt = "climbs";
         }

         if (!found)
         {
            send_to_char("You can't climb.\n\r", ch);
            return rSTOP;
         }
      }
      for (gmob = ch->in_room->first_person; gmob; gmob = gmob->next_in_room)
      {
         if (!IS_NPC(ch) && ((xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOPASS) && gmob->m4 != ht && !IS_IMMORTAL(ch))
         || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOCLOAK) && gmob->m4 != ht && get_wear_cloak(ch) && !IS_IMMORTAL(ch))
         || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOHOOD) && gmob->m4 != ht && get_wear_hidden_cloak(ch) && !IS_IMMORTAL(ch))
         || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NEEDINTRO) && gmob->m4 != ht && !str_cmp(ch->name, PERS_KINGDOM(ch, gmob->m4)) && !IS_IMMORTAL(ch))))
         {
            if (!IS_NPC(ch) && pexit->vdir != rev_dir[ch->pcdata->mapdir])
            {
               send_to_char("\n\r", ch);
               sprintf(buf, "%s I gave you a chance, time to die now.", ch->name);
               do_tell(gmob, buf);
               retcode = one_hit(gmob, ch, TYPE_UNDEFINED, LM_BODY);
               return retcode;
            }
         }
      }
      
      if (ch->mount)
      {
         switch (ch->mount->position)
         {
            case POS_DEAD:
               send_to_char("Your mount is dead!\n\r", ch);
               return rSTOP;
               break;

            case POS_MORTAL:
            case POS_INCAP:
               send_to_char("Your mount is hurt far too badly to move.\n\r", ch);
               return rSTOP;
               break;

            case POS_STUNNED:
               send_to_char("Your mount is too stunned to do that.\n\r", ch);
               return rSTOP;
               break;

            case POS_SLEEPING:
               send_to_char("Your mount is sleeping.\n\r", ch);
               return rSTOP;
               break;

            case POS_RESTING:
               send_to_char("Your mount is resting.\n\r", ch);
               return rSTOP;
               break;

            case POS_SITTING:
               send_to_char("Your mount is sitting down.\n\r", ch);
               return rSTOP;
               break;

            default:
               break;
         }

         if (!IS_FLOATING(ch->mount))
            move = movement_loss[UMIN(SECT_MAX - 1, in_room->sector_type)];
         else
            move = 1;
            
         move = calculate_movement_cost(move, ch->mount); //Takes the normal move and calculates cost based on the stats
         if (ch->mount->move < move)
         {
            send_to_char("Your mount is too exhausted.\n\r", ch);
            return rSTOP;
         }
      }
      else
      {
         if (!IS_FLOATING(ch))
            move = encumbrance(ch, movement_loss[UMIN(SECT_MAX - 1, in_room->sector_type)]);
         else
            move = 1;
            
         if (IS_MOUNTAIN(in_room->sector_type))
         {
            if (climb)
            {
               if (climb->value[0] == 1)
                  move *= 5;
               if (climb->value[0] == 2)
                  move *= 4;
               if (climb->value[0] == 3)
                  move *= 2;
            }
            else if (MASTERED(ch, gsn_mountain_climb) >= 1)  
            {
               int mlevel;
               mlevel = POINT_LEVEL(LEARNED(ch, gsn_mountain_climb), MASTERED(ch, gsn_mountain_climb));
               move *= (500-(UMIN(400, mlevel*5))) / 100;
               learn_from_success(ch, gsn_mountain_climb, NULL);
            }
         }
            
         move = calculate_movement_cost(move, ch); //Takes the normal move and calculates cost based on the stats
         
         if (ch->move < move)
         {
            send_to_char("You are too exhausted.\n\r", ch);
            return rSTOP;
         }
      }
      if (ch->mount)
         WAIT_STATE(ch, movement_lag(ch->mount, move));
      else
         WAIT_STATE(ch, movement_lag(ch, move));
      if (ch->mount)
      {
         update_movement_points(ch->mount, move);
         ch->mount->move -= move;
      }
      else
      {
         update_movement_points(ch, move);
         ch->move -= move;
      }
   }
   /*
    * Check if player can fit in the room
    */
   if (to_room->tunnel > 0)
   {
      CHAR_DATA *ctmp;
      int count = ch->mount ? 1 : 0;

      for (ctmp = to_room->first_person; ctmp; ctmp = ctmp->next_in_room)
         if (++count >= to_room->tunnel)
         {
            if (ch->mount && count == to_room->tunnel)
               send_to_char("There is no room for both you and your mount there.\n\r", ch);
            else
               send_to_char("There is no room for you there.\n\r", ch);
            return rSTOP;
         }
   }
   /* Take away Hide */
   if (IS_AFFECTED(ch, AFF_HIDE))
   {
      if (number_range(1, 100) > check_hide_move(ch, to_room->sector_type, pexit->to_room))
      {
         xREMOVE_BIT(ch->affected_by, AFF_HIDE);
         act(AT_RED, "$n quickly appears from the shadows.", ch, NULL, NULL, TO_ROOM);
      }
   }
   if (IS_AFFECTED(ch, AFF_STALK))
   {
      if (!check_stalk_move(ch, pexit->to_room))
      {
         affect_strip(ch, gsn_stalk);
         act(AT_RED, "$n attemps to stalk silently into the room but fails!.", ch, NULL, NULL, TO_ROOM);
      }
   }
   /* check for traps on exit - later */

   if (!IS_AFFECTED(ch, AFF_SNEAK) && (IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS)))
   {
      if (fall)
         txt = "falls";
      else if (!txt)
      {
         if (ch->mount)
         {
            if (IS_AFFECTED(ch->mount, AFF_FLOATING))
               txt = "floats";
            else if (IS_AFFECTED(ch->mount, AFF_FLYING))
               txt = "flies";
            else
               txt = "rides";
         }
         else
         {
            if (IS_AFFECTED(ch, AFF_FLOATING))
            {
               if (drunk)
                  txt = "floats unsteadily";
               else
                  txt = "floats";
            }
            else if (IS_AFFECTED(ch, AFF_FLYING))
            {
               if (drunk)
                  txt = "flies shakily";
               else
                  txt = "flies";
            }
            else if (ch->position == POS_SHOVE)
               txt = "is shoved";
            else if (ch->position == POS_DRAG)
               txt = "is dragged";
            else
            {
               if (drunk)
                  txt = "stumbles drunkenly";
               else
               {
                  if (!ch->movement || (ch->movement && ch->movement[0] == '\0'))
                     txt = "leaves";
                  else
                     txt = ch->movement;
               }
            }
         }
      }
      if (ch->mount)
      {
         sprintf(buf, "$n %s %s upon $N.", txt, dir_name[door]);
         act(AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT);
      }
      else
      {
         if (ch->rider)
         {         
            sprintf(buf, "$n %s %s with $N on $s back.", txt, dir_name[door]);
            act(AT_ACTION, buf, ch, NULL, ch->rider, TO_ROOM);
         }
         else
         {
            sprintf(buf, "$n %s $T.", txt);
            act(AT_ACTION, buf, ch, NULL, dir_name[door], TO_ROOM);
         }
      }
   }

   rprog_leave_trigger(ch);
   if (char_died(ch))
      return global_retcode;

   char_from_room(ch);
   if (ch->mount)
   {
      rprog_leave_trigger(ch->mount);
      if (char_died(ch))
         return global_retcode;
      if (ch->mount)
      {
         char_from_room(ch->mount);
         char_to_room(ch->mount, to_room);
         update_objects(ch->mount, -1, -1, -1);
      }
   }


   char_to_room(ch, to_room);
   update_objects(ch, -1, -1, -1);
   if (!IS_NPC(ch))
      ch->pcdata->mapdir = pexit->vdir;

   if (!IS_AFFECTED(ch, AFF_SNEAK) && (IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS)))
   {
      if (fall)
         txt = "falls";
      else if (ch->mount)
      {
         if (IS_AFFECTED(ch->mount, AFF_FLOATING))
            txt = "floats in";
         else if (IS_AFFECTED(ch->mount, AFF_FLYING))
            txt = "flies in";
         else
            txt = "rides in";
      }
      else
      {
         if (IS_AFFECTED(ch, AFF_FLOATING))
         {
            if (drunk)
               txt = "floats in unsteadily";
            else
               txt = "floats in";
         }
         else if (IS_AFFECTED(ch, AFF_FLYING))
         {
            if (drunk)
               txt = "flies in shakily";
            else
               txt = "flies in";
         }
         else if (ch->position == POS_SHOVE)
            txt = "is shoved in";
         else if (ch->position == POS_DRAG)
            txt = "is dragged in";
         else
         {
            if (drunk)
               txt = "stumbles drunkenly in";
            else
            {
               if (!ch->movement || (ch->movement && ch->movement[0] == '\0'))
                  txt = "arrives";
               else
                  txt = ch->movement;
            }
         }
      }
      dtxt = rev_exit(door);
      if ( ch->mount )
      {
         sprintf( buf, "$n %s from %s upon $N.", txt, dtxt );
         act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
      }
      else
      {
         if (ch->rider)
         {
            if (!ch->movement || (ch->movement && ch->movement[0] == '\0'))
               sprintf( buf, "$n %s from %s with $N on $s back.", txt, dtxt );
            else
               sprintf( buf, "$n %s in from %s with $N on $s back.", txt, dtxt );
            act( AT_ACTION, buf, ch, NULL, ch->rider, TO_ROOM );
         }
         else
         {   
            if (!ch->movement || (ch->movement && ch->movement[0] == '\0'))
               sprintf( buf, "$n %s from %s.", txt, dtxt );
            else
               sprintf( buf, "$n %s in from %s.", txt, dtxt );
            act( AT_ACTION, buf, ch, NULL, NULL, TO_ROOM );
         }
      }
   }
   if (IS_AFFECTED(ch, AFF_SNEAK))
      learn_from_success(ch, gsn_sneak, NULL);
   
   //Update rider so they both see eachother...
   if (ch->rider)
   {
      char_from_room(ch->rider);
      char_to_room(ch->rider, to_room);
      update_objects(ch->rider, -1, -1, -1);   
   }

   /* Make sure everyone sees the room description of death traps. */
   if (xIS_SET(ch->in_room->room_flags, ROOM_DEATH) && !IS_IMMORTAL(ch))
   {
      if (xIS_SET(ch->act, PLR_BRIEF))
         brief = TRUE;
      xREMOVE_BIT(ch->act, PLR_BRIEF);
   }
   if (xIS_SET(ch->in_room->room_flags, ROOM_WILDERNESS) && !IS_NPC(ch))
      do_map(ch, "");
   else
   {
      do_look(ch, "auto");
      if (brief)
         xSET_BIT(ch->act, PLR_BRIEF);
   }
   //Should be the LAST thing they see so they SEE it.
   if (!IS_NPC(ch) && !IS_IMMORTAL(ch))
   {
      if (pstatus != check_room_pk(ch))
      {
         if (check_room_pk(ch) == 3)
            send_to_char("\n\r^z&R******CAUTION: You are entering &G&WAN ITEM&R loot zone******^x\n\r", ch);
         if (check_room_pk(ch) == 4)
            send_to_char("\n\r^z&R******CAUTION: You are entering &G&WFULL LOOT&R loot zone******^x\n\r", ch);
      }
   }
   /*
    * Put good-old EQ-munching death traps back in!  -Thoric
    */
   if (xIS_SET(ch->in_room->room_flags, ROOM_DEATH) && !IS_IMMORTAL(ch))
   {
      act(AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM);
      set_char_color(AT_DEAD, ch);
      send_to_char("Oopsie... you're dead!\n\r", ch);
      sprintf(buf, "%s hit a DEATH TRAP in room %d!", ch->name, ch->in_room->vnum);
      log_string(buf);
      to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
      extract_char(ch, FALSE);
      return rCHAR_DIED;
   }
   //OK repeat for anyone riding on the player...
   if (ch->rider)
   {
      /* Make sure everyone sees the room description of death traps. */
      if (xIS_SET(ch->rider->in_room->room_flags, ROOM_DEATH) && !IS_IMMORTAL(ch))
      {
         if (xIS_SET(ch->rider->act, PLR_BRIEF))
            brief = TRUE;
         xREMOVE_BIT(ch->rider->act, PLR_BRIEF);
      }
      if (xIS_SET(ch->rider->in_room->room_flags, ROOM_WILDERNESS) && !IS_NPC(ch->rider))
         do_map(ch->rider, "");
      else
      {
         do_look(ch->rider, "auto");
         if (brief)
            xSET_BIT(ch->rider->act, PLR_BRIEF);
      }
      //Should be the LAST thing they see so they SEE it.
      if (!IS_NPC(ch->rider) && !IS_IMMORTAL(ch->rider))
      {
         if (pstatus != check_room_pk(ch->rider))
         {
            if (check_room_pk(ch->rider) == 3)
               send_to_char("\n\r^z&R******CAUTION: You are entering &G&WAN ITEM&R loot zone******^x\n\r", ch->rider);
            if (check_room_pk(ch->rider) == 4)
               send_to_char("\n\r^z&R******CAUTION: You are entering &G&WFULL LOOT&R loot zone******^x\n\r", ch->rider);
         }
      }
      /*
       * Put good-old EQ-munching death traps back in!  -Thoric
       */
      if (xIS_SET(ch->rider->in_room->room_flags, ROOM_DEATH) && !IS_IMMORTAL(ch->rider))
      {
         act(AT_DEAD, "$n falls prey to a terrible death!", ch->rider, NULL, NULL, TO_ROOM);
         set_char_color(AT_DEAD, ch->rider);
         send_to_char("Oopsie... you're dead!\n\r", ch->rider);
         sprintf(buf, "%s hit a DEATH TRAP in room %d!", ch->rider->name, ch->rider->in_room->vnum);
         log_string(buf);
         to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
         extract_char(ch->rider, FALSE);
         return rCHAR_DIED;
      }
   }
   /* BIG ugly looping problem here when the character is mptransed back
      to the starting room.  To avoid this, check how many chars are in
      the room at the start and stop processing followers after doing
      the right number of them.  -- Narn
    */
   if (!fall)
   {
      CHAR_DATA *fch;
      CHAR_DATA *nextinroom;
      int chars = 0, count = 0;

      for (fch = from_room->first_person; fch; fch = fch->next_in_room)
         chars++;

      for (fch = from_room->first_person; fch && (count < chars); fch = nextinroom)
      {
         nextinroom = fch->next_in_room;
         count++;
         if (fch != ch /* loop room bug fix here by Thoric */
            && fch->master == ch && (fch->position == POS_STANDING || fch->position == POS_MOUNTED))
         {
            act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
            move_char(fch, pexit, 0);
         }
      }
   }

   if (ch->in_room->first_content)
      retcode = check_room_for_traps(ch, TRAP_ENTER_ROOM);
   if (retcode != rNONE)
      return retcode;

   if (char_died(ch))
      return retcode;

   mprog_entry_trigger(ch);
   if (char_died(ch))
      return retcode;

   rprog_enter_trigger(ch);
   if (char_died(ch))
      return retcode;

   mprog_greet_trigger(ch);
   if (char_died(ch))
      return retcode;

   oprog_greet_trigger(ch);
   if (char_died(ch))
      return retcode;

   if (!will_fall(ch, fall) && fall > 0)
   {
      if (!IS_AFFECTED(ch, AFF_FLOATING) || (ch->mount && !IS_AFFECTED(ch->mount, AFF_FLOATING)))
      {
         set_char_color(AT_HURT, ch);
         send_to_char("OUCH! You hit the ground!\n\r", ch);
         WAIT_STATE(ch, 20);
         retcode = damage(ch, ch, 20 * fall, TYPE_UNDEFINED, 0, -1);
         if (char_died(ch))
            return retcode;
      }
      else
      {
         set_char_color(AT_MAGIC, ch);
         send_to_char("You lightly float down to the ground.\n\r", ch);
      }
   }
   for (gmob = ch->in_room->first_person; gmob; gmob = gmob->next_in_room)
   {
      if (!IS_NPC(ch) && !IS_NPC(gmob))
      {
         INTRO_DATA *intro;
         
         for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
         {
            if (intro->pid == gmob->pcdata->pid)
            {
               if (time(0) - intro->lastseen > 300) //5 minutes  
               {
                  if (intro->value > 0)
                     intro->value += number_range(100, 225);
                  else
                     intro->value -= number_range(100, 225);
                  intro->lastseen = time(0);
                  break;
               }
            }
         }
         if (intro)
         {
            for (intro = gmob->pcdata->first_introduction; intro; intro = intro->next)
            {
               if (intro->pid == ch->pcdata->pid)
               {
                  if (time(0) - intro->lastseen > 300) //5 minutes  
                  {
                     if (intro->value > 0)
                        intro->value += number_range(100, 225);
                     else
                        intro->value -= number_range(100, 225);
                     intro->lastseen = time(0);
                     break;
                  }
               }
            } 
         }
      }                
      if (((xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOPASS) && gmob->m4 != ht && !IS_IMMORTAL(ch))
         || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOCLOAK) && gmob->m4 != ht && get_wear_cloak(ch) && !IS_IMMORTAL(ch))
         || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOHOOD) && gmob->m4 != ht && get_wear_hidden_cloak(ch) && !IS_IMMORTAL(ch))
         || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NEEDINTRO) && gmob->m4 != ht && !str_cmp(ch->name, PERS_KINGDOM(ch, gmob->m4)) && !IS_IMMORTAL(ch))))         
      {
         if (!IS_NPC(ch))
         {
            send_to_char("\n\r", ch);
            if (xIS_SET(gmob->miflags, KM_NOPASS))
            {
               sprintf(buf, "%s you are not welcome beyond this point, please leave the way you came in.", ch->name);
               do_tell(gmob, buf);
            }
            if (xIS_SET(gmob->miflags, KM_NOCLOAK) && get_wear_cloak(ch))
            {
               sprintf(buf, "%s you need to remove your cloak before entering.  Try to enter and you will DIE!", ch->name);
               do_tell(gmob, buf);
            }
            if (xIS_SET(gmob->miflags, KM_NOHOOD) && get_wear_hidden_cloak(ch))
            {
               sprintf(buf, "%s you need to remove your hood before entering.  Try to enter and you will DIE!", ch->name);
               do_tell(gmob, buf);
            }
            if (xIS_SET(gmob->miflags, KM_NEEDINTRO) && !str_cmp(ch->name, PERS_KINGDOM(ch, gmob->m4)))
            {
               sprintf(buf, "%s Your face is unknown here.  If you wish to enter you must first introduce yourself.  If you attempt to move forward you will be struck down!", ch->name);
               do_tell(gmob, buf);
            }
   
         }
         else
         {
            send_to_char("\n\r", ch);
            sprintf(buf, "%s I shall not allow you to ENTER!", ch->name);
            retcode = one_hit(gmob, ch, TYPE_UNDEFINED, LM_BODY);
            if (gmob)
               do_tell(gmob, buf);
            return retcode;
         }
      }
   }
   return retcode;
}


void do_north(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_north(ch);
      return;
   }
   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_NORTH), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}


void do_east(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_east(ch);
      return;
   }

   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_EAST), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}


void do_south(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_south(ch);
      return;
   }
   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_SOUTH), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}


void do_west(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_west(ch);
      return;
   }

   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_WEST), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}


void do_up(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_up(ch);
      return;
   }

   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_UP), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}


void do_down(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_down(ch);
      return;
   }

   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_DOWN), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}

void do_northeast(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_ne(ch);
      return;
   }
   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_NORTHEAST), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}

void do_northwest(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_nw(ch);
      return;
   }

   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_NORTHWEST), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}

void do_southeast(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_se(ch);
      return;
   }

   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_SOUTHEAST), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}

void do_southwest(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *was_room;

   if (IS_ONMAP_FLAG(ch))
   {
      map_sw(ch);
      return;
   }

   was_room = ch->in_room;
   move_char(ch, get_exit(ch->in_room, DIR_SOUTHWEST), 0);
   if (was_room == ch->in_room)
      free_runbuf(ch->desc);
   return;
}



EXIT_DATA *find_door(CHAR_DATA * ch, char *arg, bool quiet)
{
   EXIT_DATA *pexit;
   int door;
   char arg1[MSL];
   int number;
   int count = 0;

   if (arg == NULL || !str_cmp(arg, ""))
      return NULL;

   pexit = NULL;
   if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
      door = 0;
   else if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
      door = 1;
   else if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
      door = 2;
   else if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
      door = 3;
   else if (!str_cmp(arg, "u") || !str_cmp(arg, "up"))
      door = 4;
   else if (!str_cmp(arg, "d") || !str_cmp(arg, "down"))
      door = 5;
   else if (!str_cmp(arg, "ne") || !str_cmp(arg, "northeast") || !str_cmp(arg, "f"))
      door = 6;
   else if (!str_cmp(arg, "nw") || !str_cmp(arg, "northwest") || !str_cmp(arg, "g"))
      door = 7;
   else if (!str_cmp(arg, "se") || !str_cmp(arg, "southeast") || !str_cmp(arg, "h"))
      door = 8;
   else if (!str_cmp(arg, "sw") || !str_cmp(arg, "southwest") || !str_cmp(arg, "i"))
      door = 9;
   else
   {
      number = number_argument(arg, arg1);
      for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
      {
         if ((quiet || IS_SET(pexit->exit_info, EX_ISDOOR)) && pexit->keyword && nifty_is_name(arg1, pexit->keyword))
         {
            if (++count == number)
               return pexit;
         }
      }
      if (!quiet)
         act(AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR);
      return NULL;
   }

   if ((pexit = get_exit(ch->in_room, door)) == NULL)
   {
      if (!quiet)
         act(AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR);
      return NULL;
   }

   if (quiet)
      return pexit;

   if (IS_SET(pexit->exit_info, EX_SECRET))
   {
      act(AT_PLAIN, "You see no $T here.", ch, NULL, arg, TO_CHAR);
      return NULL;
   }

   if (!IS_SET(pexit->exit_info, EX_ISDOOR))
   {
      send_to_char("You can't do that.\n\r", ch);
      return NULL;
   }

   return pexit;
}


void set_bexit_flag(EXIT_DATA * pexit, int flag)
{
   EXIT_DATA *pexit_rev;

   SET_BIT(pexit->exit_info, flag);
   if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
      SET_BIT(pexit_rev->exit_info, flag);
}

void remove_bexit_flag(EXIT_DATA * pexit, int flag)
{
   EXIT_DATA *pexit_rev;

   REMOVE_BIT(pexit->exit_info, flag);
   if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
      REMOVE_BIT(pexit_rev->exit_info, flag);
}

void toggle_bexit_flag(EXIT_DATA * pexit, int flag)
{
   EXIT_DATA *pexit_rev;

   TOGGLE_BIT(pexit->exit_info, flag);
   if ((pexit_rev = pexit->rexit) != NULL && pexit_rev != pexit)
      TOGGLE_BIT(pexit_rev->exit_info, flag);
}

void update_indoor_status(int doornum, CHAR_DATA *ch, TOWN_DATA *town, int x, int y, int map, int type)
{
   int udoornum;
   DOOR_DATA *ddata;
   int z;
   int fnx;
   int cnt;
   int fnd;
   int fnddoor[100];
   int fx;
   
   for (z = 0; z <= 99; z++)
      fnddoor[z] = 0;
   
   udoornum = town->doorstate[4][doornum];
   
   if (town->doorstate[3][doornum] != 1)
   {
      //pretty much the same thing as a close, we need to refresh....
      for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
      {
         ddata->cansee = 0;
      }
      for (fnx = 0; fnx <= 99; fnx++)
      {
         if (town->doorstate[5][fnx] > 0 && town->doorstate[3][fnx] == 1 && town->doorstate[0][fnx] == 0)
         {
            update_indoor_status(fnx, ch, town, town->doorstate[5][fnx], town->doorstate[6][fnx], town->doorstate[7][fnx], 0);
         }
      }  
   }
   else
   {
      //Need to check other master doors, there might be more than 1 way to get into a
      //roofed building
      if (type == 1)
      {  
         //Well if there is only 1 master door in your town (doubtful) then this will
         //be short.  If not, need to reopen now...
         for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
         {
            ddata->cansee = 0;
         }
         for (fnx = 0; fnx <= 99; fnx++)
         {
            if (town->doorstate[5][fnx] > 0 && town->doorstate[3][fnx] == 1 && town->doorstate[0][fnx] == 0)
            {
               update_indoor_status(fnx, ch, town, town->doorstate[5][fnx], town->doorstate[6][fnx], town->doorstate[7][fnx], 0);
            }
         }
      }
      else
      {
         fx = 0;
         cnt = -1;
         fnddoor[fx++] = udoornum;
         for (;;)
         {
            if (fx == ++cnt) //we managed to exhaust all the doors, lets exit
               break;        
            udoornum = fnddoor[cnt];
            for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
            {
               fnd = 1;
               for (z = 0; z <= 9; z++)
               {
                  if (ddata->doorvalue[z] == udoornum)
                    break;
               }
               if (z != 10)
               {
                  ddata->cansee = 1;
                  //lets look for open doors....
                  for (z = 0; z <= 9; z++)
                  {
                     if (ddata->doorvalue[z] > 0)
                     {
                        for (fnx = 0; fnx <= 99; fnx++)
                        {
                           if (town->doorstate[4][fnx] == ddata->doorvalue[z])
                              break;
                        }
                        if (fnx <= 99 && town->doorstate[0][fnx] == 0) //open
                        {
                           for (fnx = 0; fnx <= 99; fnx++)
                           {
                              if (fnddoor[fnx] == 0)
                              {
                                 fnd = 0;
                                 break;
                              }
                              if (fnddoor[fnx] == ddata->doorvalue[z])
                              {
                                 break;
                              }
                           }
                           if (fnx == 100) //err this shouldn't happen....
                           {
                              bug("update_indoor_stat:  The fnddoor array is full for town %s", town->name);
                              return;
                           }
                           if (fnd == 0)
                           {
                              fnddoor[fx++] = ddata->doorvalue[z];
                           }
                        }
                     }
                  }
               }
            }//end ddata for check for open
         }//end infinite loop for open
      }//end check for open/close
   }//end masterdoor check
}
      
   

void do_open(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   EXIT_DATA *pexit;
   int door;
   char buf[MSL];
   int x, y, z;
   TOWN_DATA *town;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Open what?\n\r", ch);
      return;
   }

   if ((pexit = find_door(ch, arg, TRUE)) != NULL)
   {
      /* 'open door' */
      EXIT_DATA *pexit_rev;

      if (IS_SET(pexit->exit_info, EX_SECRET) && pexit->keyword && !nifty_is_name(arg, pexit->keyword))
      {
         ch_printf(ch, "You see no %s here.\n\r", arg);
         return;
      }
      if (!IS_SET(pexit->exit_info, EX_ISDOOR))
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (!IS_SET(pexit->exit_info, EX_CLOSED))
      {
         send_to_char("It's already open.\n\r", ch);
         return;
      }
      if (IS_SET(pexit->exit_info, EX_LOCKED))
      {
         send_to_char("It's locked.\n\r", ch);
         return;
      }

      if (!IS_SET(pexit->exit_info, EX_SECRET) || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
      {
         act(AT_ACTION, "$n opens the $d.", ch, NULL, pexit->keyword, TO_ROOM);
         act(AT_ACTION, "You open the $d.", ch, NULL, pexit->keyword, TO_CHAR);
         if ((pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room)
         {
            CHAR_DATA *rch;

            for (rch = pexit->to_room->first_person; rch; rch = rch->next_in_room)
               act(AT_ACTION, "The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR);
         }
         remove_bexit_flag(pexit, EX_CLOSED);
         if ((door = pexit->vdir) >= 0 && door < 10)
            check_room_for_traps(ch, trap_door[door]);
         return;
      }
   }

   if ((obj = get_obj_here(ch, arg)) != NULL)
   {
      /* 'open object' */
      if (obj->item_type != ITEM_CONTAINER)
      {
         ch_printf(ch, "%s is not a container.\n\r", capitalize(obj->short_descr));
         return;
      }
      if (!IS_SET(obj->value[1], CONT_CLOSED))
      {
         ch_printf(ch, "%s is already open.\n\r", capitalize(obj->short_descr));
         return;
      }
      if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
      {
         ch_printf(ch, "%s cannot be opened or closed.\n\r", capitalize(obj->short_descr));
         return;
      }
      if (IS_SET(obj->value[1], CONT_LOCKED))
      {
         ch_printf(ch, "%s is locked.\n\r", capitalize(obj->short_descr));
         return;
      }

      REMOVE_BIT(obj->value[1], CONT_CLOSED);
      act(AT_ACTION, "You open $p.", ch, obj, NULL, TO_CHAR);
      act(AT_ACTION, "$n opens $p.", ch, obj, NULL, TO_ROOM);
      check_for_trap(ch, obj, TRAP_OPEN, NEW_TRAP_OPEN);
      return;
   }
   x = ch->coord->x;
   y = ch->coord->y;
   if (IN_WILDERNESS(ch) && is_valid_movement(&x, &y, arg, ch))
   {
      if (map_sector[ch->map][x][y] != SECT_CDOOR)
      {
         send_to_char("There is no closed door in that direction.\n\r", ch);
         return;
      }
      town = find_town(x, y, ch->map);
      if (!town)
      {
         bug("do_open: %s at %d %d has opened a door that does not belong to a town.", ch->name, x, y);
      }
      else
      {
         for (z = 0; z <= 99; z++)
         {
            if (town->doorstate[4][z] > 0)
            {
               if (town->doorstate[5][z] == x && town->doorstate[6][z] == y && town->doorstate[7][z] == ch->map)
               {
                  town->doorstate[0][z] = 0;
                  write_kingdom_file(town->kingdom);
                  break;
               }
            }
         }
         if (z == 100)
         {
            bug("do_open: %s at %d %d in town %s has found a door not belonging to that town", ch->name, x, y, town->name);
         }
         else
         {
            update_indoor_status(z, ch, town, x, y, ch->map, 0);
         }
      }
      sprintf(buf, "$n opens the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CANSEE);
      sprintf(buf, "You open the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CHAR);
      map_sector[ch->map][x][y] = SECT_DOOR;
      return;
   }
   
   ch_printf(ch, "You see no %s here.\n\r", arg);
   return;
}



void do_close(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj;
   EXIT_DATA *pexit;
   int door;
   char buf[MSL];
   int x, y, z;
   TOWN_DATA *town;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Close what?\n\r", ch);
      return;
   }

   if ((pexit = find_door(ch, arg, TRUE)) != NULL)
   {
      /* 'close door' */
      EXIT_DATA *pexit_rev;

      if (!IS_SET(pexit->exit_info, EX_ISDOOR))
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (IS_SET(pexit->exit_info, EX_CLOSED))
      {
         send_to_char("It's already closed.\n\r", ch);
         return;
      }

      act(AT_ACTION, "$n closes the $d.", ch, NULL, pexit->keyword, TO_ROOM);
      act(AT_ACTION, "You close the $d.", ch, NULL, pexit->keyword, TO_CHAR);

      /* close the other side */
      if ((pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room)
      {
         CHAR_DATA *rch;

         SET_BIT(pexit_rev->exit_info, EX_CLOSED);
         for (rch = pexit->to_room->first_person; rch; rch = rch->next_in_room)
            act(AT_ACTION, "The $d closes.", rch, NULL, pexit_rev->keyword, TO_CHAR);
      }
      set_bexit_flag(pexit, EX_CLOSED);
      if ((door = pexit->vdir) >= 0 && door < 10)
         check_room_for_traps(ch, trap_door[door]);
      return;
   }

   if ((obj = get_obj_here(ch, arg)) != NULL)
   {
      /* 'close object' */
      if (obj->item_type != ITEM_CONTAINER)
      {
         ch_printf(ch, "%s is not a container.\n\r", capitalize(obj->short_descr));
         return;
      }
      if (IS_SET(obj->value[1], CONT_CLOSED))
      {
         ch_printf(ch, "%s is already closed.\n\r", capitalize(obj->short_descr));
         return;
      }
      if (!IS_SET(obj->value[1], CONT_CLOSEABLE))
      {
         ch_printf(ch, "%s cannot be opened or closed.\n\r", capitalize(obj->short_descr));
         return;
      }

      SET_BIT(obj->value[1], CONT_CLOSED);
      act(AT_ACTION, "You close $p.", ch, obj, NULL, TO_CHAR);
      act(AT_ACTION, "$n closes $p.", ch, obj, NULL, TO_ROOM);
      check_for_trap(ch, obj, TRAP_CLOSE, NEW_TRAP_CLOSE);
      return;
   }
   
   x = ch->coord->x;
   y = ch->coord->y;
   if (IN_WILDERNESS(ch) && is_valid_movement(&x, &y, arg, ch))
   {
      if (map_sector[ch->map][x][y] != SECT_DOOR)
      {
         send_to_char("There is no open door in that direction.\n\r", ch);
         return;
      }
      town = find_town(x, y, ch->map);
      if (!town)
      {
         bug("do_open: %s at %d %d has closed a door that does not belong to a town.", ch->name, x, y);
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
         else
         {
            update_indoor_status(z, ch, town, x, y, ch->map, 1);
         }
      }
      sprintf(buf, "$n closes the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CANSEE);
      sprintf(buf, "You close the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CHAR);
      map_sector[ch->map][x][y] = SECT_CDOOR;
      return;
   }

   ch_printf(ch, "You see no %s here.\n\r", arg);
   return;
}


/*
 * Keyring support added by Thoric
 * Idea suggested by Onyx <MtRicmer@worldnet.att.net> of Eldarion
 *
 * New: returns pointer to key/NULL instead of TRUE/FALSE
 *
 * If you want a feature like having immortals always have a key... you'll
 * need to code in a generic key, and make sure extract_obj doesn't extract it
 */
OBJ_DATA *has_key(CHAR_DATA * ch, int key)
{
   OBJ_DATA *obj, *obj2;

   for (obj = ch->first_carrying; obj; obj = obj->next_content)
   {
      if (obj->pIndexData->vnum == key || (obj->item_type == ITEM_KEY && obj->value[0] == key))
         return obj;
      else if (obj->item_type == ITEM_KEYRING)
         for (obj2 = obj->first_content; obj2; obj2 = obj2->next_content)
            if (obj2->pIndexData->vnum == key || obj2->value[0] == key)
               return obj2;
   }

   return NULL;
}
int get_kingdom_for_door(OBJ_DATA *key)
{
   int kx;
   for (kx = 2; kx < sysdata.max_kingdom; kx++)
   {
      if (kingdom_table[kx]->kpid == key->value[0])
         return kx;
   }
   return -1;
}

TOWN_DATA *get_town_for_door(OBJ_DATA *key, int kingdom)
{
   TOWN_DATA *town;
   for (town = kingdom_table[kingdom]->first_town; town; town = town->next)
   {
      if (town->tpid == key->value[1])
         return town;
   }
   return NULL;
}

void do_lock(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj, *key, *key2;
   EXIT_DATA *pexit;
   int count;
   char buf[MSL];
   int x, y, z;
   TOWN_DATA *town;
   int kingdom;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Lock what?\n\r", ch);
      return;
   }

   if ((pexit = find_door(ch, arg, TRUE)) != NULL)
   {
      /* 'lock door' */

      if (!IS_SET(pexit->exit_info, EX_ISDOOR))
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (!IS_SET(pexit->exit_info, EX_CLOSED))
      {
         send_to_char("It's not closed.\n\r", ch);
         return;
      }
      if (pexit->key < 0)
      {
         send_to_char("It can't be locked.\n\r", ch);
         return;
      }
      if ((key = has_key(ch, pexit->key)) == NULL)
      {
         send_to_char("You lack the key.\n\r", ch);
         return;
      }
      if (IS_SET(pexit->exit_info, EX_LOCKED))
      {
         send_to_char("It's already locked.\n\r", ch);
         return;
      }

      if (!IS_SET(pexit->exit_info, EX_SECRET) || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
      {
         send_to_char("*Click*\n\r", ch);
         count = key->count;
         key->count = 1;
         act(AT_ACTION, "$n locks the $d with $p.", ch, key, pexit->keyword, TO_ROOM);
         key->count = count;
         set_bexit_flag(pexit, EX_LOCKED);
         return;
      }
   }

   if ((obj = get_obj_here(ch, arg)) != NULL)
   {
      /* 'lock object' */
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
         send_to_char("It can't be locked.\n\r", ch);
         return;
      }
      if ((key = has_key(ch, obj->value[2])) == NULL)
      {
         send_to_char("You lack the key.\n\r", ch);
         return;
      }
      if (IS_SET(obj->value[1], CONT_LOCKED))
      {
         send_to_char("It's already locked.\n\r", ch);
         return;
      }

      SET_BIT(obj->value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      count = key->count;
      key->count = 1;
      act(AT_ACTION, "$n locks $p with $P.", ch, obj, key, TO_ROOM);
      key->count = count;
      return;
   }
   x = ch->coord->x;
   y = ch->coord->y;
   if (IN_WILDERNESS(ch) && is_valid_movement(&x, &y, arg, ch))
   {
      if (map_sector[ch->map][x][y] != SECT_CDOOR)
      {
         send_to_char("There is no closed door in that direction.\n\r", ch);
         return;
      }
      town = find_town(x, y, ch->map);
      if (!town)
      {
         bug("do_lock: %s at %d %d has closed a door that does not belong to a town.", ch->name, x, y);
      }
      else
      {
         for (z = 0; z <= 99; z++)
         {
            if (town->doorstate[5][z] == x && town->doorstate[6][z] == y && town->doorstate[7][z] == ch->map)
            {
               for (key = ch->first_carrying; key; key = key->next_content)
               {
                  if (key->item_type == ITEM_KEY && IS_OBJ_STAT(key, ITEM_KINGDOMKEY))
                  {
                     kingdom = get_kingdom_for_door(key);
                     if (kingdom < 2)
                        continue;
                     town = get_town_for_door(key, kingdom);
                     if (!town)
                        continue;
                     if (kingdom_sector[ch->map][x][y] != kingdom)
                        continue;
                     if (!in_town_range(town, x, y, ch->map))
                        continue;
                     if (!IS_SET(town->doorstate[2][z], key->value[2]) && key->value[2] != BV00)
                        continue;
                     break;
                  }
                  else if (key->item_type == ITEM_KEYRING)
                  {
                     for (key2 = key->first_content; key2; key2 = key2->next_content)
                     {
                        if (key2->item_type == ITEM_KEY && IS_OBJ_STAT(key2, ITEM_KINGDOMKEY))
                        {
                           kingdom = get_kingdom_for_door(key2);
                           if (kingdom < 2)
                              continue;
                           town = get_town_for_door(key2, kingdom);
                           if (!town)
                              continue;
                           if (kingdom_sector[ch->map][x][y] != kingdom)
                              continue;
                           if (!in_town_range(town, x, y, ch->map))
                              continue;
                           if (!IS_SET(town->doorstate[2][z], key2->value[2]) && key2->value[2] != BV00)
                              continue;
                           key = key2;
                           break;
                        }
                     }
                  }
               }
               if (key)
               {
                  town->doorstate[0][z] = 2;
                  write_kingdom_file(town->kingdom);
                  break;
               }
            }
         }
         if (z == 100)
         {
            send_to_char("You do not have a key to properly lock this door.\n\r", ch);
            return;
         }
      }
      sprintf(buf, "$n locks the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CANSEE);
      sprintf(buf, "You lock the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CHAR);
      map_sector[ch->map][x][y] = SECT_LDOOR;
      return;
   }

   ch_printf(ch, "You see no %s here.\n\r", arg);
   return;
}



void do_unlock(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_DATA *obj, *key, *key2;
   EXIT_DATA *pexit;
   int count;
   char buf[MSL];
   int x, y, z;
   TOWN_DATA *town;
   int kingdom;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Unlock what?\n\r", ch);
      return;
   }

   if ((pexit = find_door(ch, arg, TRUE)) != NULL)
   {
      /* 'unlock door' */

      if (!IS_SET(pexit->exit_info, EX_ISDOOR))
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (!IS_SET(pexit->exit_info, EX_CLOSED))
      {
         send_to_char("It's not closed.\n\r", ch);
         return;
      }
      if (pexit->key < 0)
      {
         send_to_char("It can't be unlocked.\n\r", ch);
         return;
      }
      if ((key = has_key(ch, pexit->key)) == NULL)
      {
         send_to_char("You lack the key.\n\r", ch);
         return;
      }
      if (!IS_SET(pexit->exit_info, EX_LOCKED))
      {
         send_to_char("It's already unlocked.\n\r", ch);
         return;
      }

      if (!IS_SET(pexit->exit_info, EX_SECRET) || (pexit->keyword && nifty_is_name(arg, pexit->keyword)))
      {
         send_to_char("*Click*\n\r", ch);
         count = key->count;
         key->count = 1;
         act(AT_ACTION, "$n unlocks the $d with $p.", ch, key, pexit->keyword, TO_ROOM);
         key->count = count;
         if (IS_SET(pexit->exit_info, EX_EATKEY))
         {
            separate_obj(key);
            extract_obj(key);
         }
         remove_bexit_flag(pexit, EX_LOCKED);
         return;
      }
   }

   if ((obj = get_obj_here(ch, arg)) != NULL)
   {
      /* 'unlock object' */
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
      if ((key = has_key(ch, obj->value[2])) == NULL)
      {
         send_to_char("You lack the key.\n\r", ch);
         return;
      }
      if (!IS_SET(obj->value[1], CONT_LOCKED))
      {
         send_to_char("It's already unlocked.\n\r", ch);
         return;
      }

      REMOVE_BIT(obj->value[1], CONT_LOCKED);
      send_to_char("*Click*\n\r", ch);
      count = key->count;
      key->count = 1;
      act(AT_ACTION, "$n unlocks $p with $P.", ch, obj, key, TO_ROOM);
      key->count = count;
      if (IS_SET(obj->value[1], CONT_EATKEY))
      {
         separate_obj(key);
         extract_obj(key);
      }
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
         bug("do_lock: %s at %d %d has closed a door that does not belong to a town.", ch->name, x, y);
      }
      else
      {
         for (z = 0; z <= 99; z++)
         {
            if (town->doorstate[5][z] == x && town->doorstate[6][z] == y && town->doorstate[7][z] == ch->map)
            {
               for (key = ch->first_carrying; key; key = key->next_content)
               {
                  if (key->item_type == ITEM_KEY && IS_OBJ_STAT(key, ITEM_KINGDOMKEY))
                  {
                     kingdom = get_kingdom_for_door(key);
                     if (kingdom < 2)
                        continue;
                     town = get_town_for_door(key, kingdom);
                     if (!town)
                        continue;
                     if (kingdom_sector[ch->map][x][y] != kingdom)
                        continue;
                     if (!in_town_range(town, x, y, ch->map))
                        continue;
                     if (!IS_SET(town->doorstate[2][z], key->value[2]) && key->value[2] != BV00)
                        continue;
                     break;
                  }
                  else if (key->item_type == ITEM_KEYRING)
                  {
                     for (key2 = key->first_content; key2; key2 = key2->next_content)
                     {
                        if (key2->item_type == ITEM_KEY && IS_OBJ_STAT(key2, ITEM_KINGDOMKEY))
                        {
                           kingdom = get_kingdom_for_door(key2);
                           if (kingdom < 2)
                              continue;
                           town = get_town_for_door(key2, kingdom);
                           if (!town)
                              continue;
                           if (kingdom_sector[ch->map][x][y] != kingdom)
                              continue;
                           if (!in_town_range(town, x, y, ch->map))
                              continue;
                           if (!IS_SET(town->doorstate[2][z], key2->value[2]) && key2->value[2] != BV00)
                              continue;
                           key = key2;
                           break;
                        }
                     }
                  }
               }
               if (key)
               {
                  town->doorstate[0][z] = 1;
                  write_kingdom_file(town->kingdom);
                  break;
               }
            }
         }
         if (z == 100)
         {
            send_to_char("You do not have a key to properly unlock this door.\n\r", ch);
            return;
         }
      }
      sprintf(buf, "$n unlocks the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CANSEE);
      sprintf(buf, "You unlock the door to the %s", arg);
      act(AT_ACTION, buf, ch, NULL, NULL, TO_CHAR);
      map_sector[ch->map][x][y] = SECT_CDOOR;
      return;
   }

   ch_printf(ch, "You see no %s here.\n\r", arg);
   return;
}

void do_bashdoor(CHAR_DATA * ch, char *argument)
{
   EXIT_DATA *pexit;
   char arg[MIL];
   sh_int mastery;

   mastery = MASTERED(ch, gsn_bashdoor) * 25;

   if (!IS_NPC(ch) && (ch->pcdata->learned[gsn_bashdoor] == 0 || ch->pcdata->ranking[gsn_bashdoor] == 0))
   {
      send_to_char("You're not enough of a warrior to bash doors!\n\r", ch);
      return;
   }

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Bash what?\n\r", ch);
      return;
   }

   if (ch->fighting)
   {
      send_to_char("You can't break off your fight.\n\r", ch);
      return;
   }

   if ((pexit = find_door(ch, arg, FALSE)) != NULL)
   {
      ROOM_INDEX_DATA *to_room;
      EXIT_DATA *pexit_rev;
      int chance;
      char *keyword;

      if (!IS_SET(pexit->exit_info, EX_CLOSED))
      {
         send_to_char("Calm down.  It is already open.\n\r", ch);
         return;
      }

      WAIT_STATE(ch, skill_table[gsn_bashdoor]->beats*2);

      if (IS_SET(pexit->exit_info, EX_SECRET))
         keyword = "wall";
      else
         keyword = pexit->keyword;
      if (!IS_NPC(ch))
         chance = mastery + (4 * get_curr_str(ch) - 19);
      else
         chance = 90;
      if (IS_SET(pexit->exit_info, EX_LOCKED))
         chance /= 3;

      if (!IS_SET(pexit->exit_info, EX_BASHPROOF)
         && ch->move >= 15 && (number_percent() < chance || MASTERED(ch, gsn_bashdoor) == 4))
      {
         REMOVE_BIT(pexit->exit_info, EX_CLOSED);
         if (IS_SET(pexit->exit_info, EX_LOCKED))
            REMOVE_BIT(pexit->exit_info, EX_LOCKED);
         SET_BIT(pexit->exit_info, EX_BASHED);

         act(AT_SKILL, "Crash!  You bashed open the $d!", ch, NULL, keyword, TO_CHAR);
         act(AT_SKILL, "$n bashes open the $d!", ch, NULL, keyword, TO_ROOM);
         learn_from_success(ch, gsn_bashdoor, NULL);

         if ((to_room = pexit->to_room) != NULL && (pexit_rev = pexit->rexit) != NULL && pexit_rev->to_room == ch->in_room)
         {
            CHAR_DATA *rch;

            REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
            if (IS_SET(pexit_rev->exit_info, EX_LOCKED))
               REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
            SET_BIT(pexit_rev->exit_info, EX_BASHED);

            for (rch = to_room->first_person; rch; rch = rch->next_in_room)
            {
               act(AT_SKILL, "The $d crashes open!", rch, NULL, pexit_rev->keyword, TO_CHAR);
            }
         }
         damage(ch, ch, (ch->max_hit / 20), gsn_bashdoor, 0, -1);

      }
      else
      {
         act(AT_SKILL, "WHAAAAM!!!  You bash against the $d, but it doesn't budge.", ch, NULL, keyword, TO_CHAR);
         act(AT_SKILL, "WHAAAAM!!!  $n bashes against the $d, but it holds strong.", ch, NULL, keyword, TO_ROOM);
         damage(ch, ch, (ch->max_hit / 20) + 10, gsn_bashdoor, 0, -1);
         learn_from_failure(ch, gsn_bashdoor, NULL);
      }
   }
   else
   {
      act(AT_SKILL, "WHAAAAM!!!  You bash against the wall, but it doesn't budge.", ch, NULL, NULL, TO_CHAR);
      act(AT_SKILL, "WHAAAAM!!!  $n bashes against the wall, but it holds strong.", ch, NULL, NULL, TO_ROOM);
      damage(ch, ch, (ch->max_hit / 20) + 10, gsn_bashdoor, 0, -1);
      learn_from_failure(ch, gsn_bashdoor, NULL);
   }
   return;
}


void do_stand(CHAR_DATA * ch, char *argument)
{

   OBJ_DATA *obj = NULL;
   int aon = 0;
   CHAR_DATA *fch = NULL;
   int val0;
   int val1;

   if (ch->position == POS_FIGHTING
      || ch->position == POS_BERSERK || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE)
   {
      send_to_char("Maybe you should finish this fight first?\n\r", ch);
      return;
   }

   if (ch->position == POS_MOUNTED)
   {
      send_to_char("You are already sitting - on your mount.\n\r", ch);
      return;
   }
   if (ch->rider)
   {
      send_to_char("You have someone on your back, need to have them get off before you can stand.\n\r", ch);
      return;
   }
   if (ch->riding)
   {
      send_to_char("You need to dismount first.\n\r", ch);
      return;
   }
   /* okay, now that we know we can sit, find an object to sit on */
   if (argument[0] != '\0')
   {
      obj = get_obj_list(ch, argument, ch->in_room->first_content);
      if (obj == NULL)
      {
         send_to_char("You don't see that here.\n\r", ch);
         return;
      }
      if (obj->item_type != ITEM_FURNITURE)
      {
         send_to_char("It has to be furniture silly.\n\r", ch);
         return;
      }
      if (!IS_SET(obj->value[2], STAND_ON) && !IS_SET(obj->value[2], STAND_IN) && !IS_SET(obj->value[2], STAND_AT))
      {
         send_to_char("You can't stand on that.\n\r", ch);
         return;
      }
      if (obj->value[0] == 0)
         val0 = 1;
      else
         val0 = obj->value[0];
      if (ch->on != obj && count_users(obj) >= val0)
      {
         act(AT_ACTION, "There's no room to stand on $p.", ch, obj, NULL, TO_CHAR);
         return;
      }
      if (ch->on == obj)
         aon = 1;
      else
         ch->on = obj;
   }
   switch (ch->position)
   {
      case POS_SLEEPING:
         if (IS_AFFECTED(ch, AFF_SLEEP))
         {
            send_to_char("You can't wake up!\n\r", ch);
            return;
         }

         if (obj == NULL)
         {
            send_to_char("You wake and stand up.\n\r", ch);
            act(AT_ACTION, "$n wakes and stands up.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], STAND_AT))
         {
            act(AT_ACTION, "You wake and stand at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes and stands at $p.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], STAND_ON))
         {
            act(AT_ACTION, "You wake and stand on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes and stands on $p.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You wake and stand in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes and stands in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_STANDING;
         do_look(ch, "auto");
         break;

      case POS_RESTING:
      case POS_SITTING:
         if (obj == NULL)
         {
            send_to_char("You stand up.\n\r", ch);
            act(AT_ACTION, "$n stands up.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], STAND_AT))
         {
            act(AT_ACTION, "You stand at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n stands at $p.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], STAND_ON))
         {
            act(AT_ACTION, "You stand on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n stands on $p.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You stand in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n stands on $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_STANDING;
         break;

      case POS_STANDING:
         if (obj != NULL && aon != 1)
         {

            if (IS_SET(obj->value[2], STAND_AT))
            {
               act(AT_ACTION, "You stand at $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n stands at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET(obj->value[2], STAND_ON))
            {
               act(AT_ACTION, "You stand on $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n stands on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
               act(AT_ACTION, "You stand in $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n stands on $p.", ch, obj, NULL, TO_ROOM);
            }
         }
         else if (aon == 1)
         {
            act(AT_ACTION, "You are already using $p for furniture.", ch, obj, NULL, TO_CHAR);
         }
         else if (ch->on != NULL && obj == NULL)
         {
            act(AT_ACTION, "You hop off of $p and stand on the ground.", ch, ch->on, NULL, TO_CHAR);
            act(AT_ACTION, "$n hops off of $p and stands on the ground.", ch, ch->on, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else
            send_to_char("You are already standing.\n\r", ch);
         break;

   }
   if (obj != NULL)
   {
      if (obj->value[1] == 0)
         val1 = 750;
      else
         val1 = obj->value[1];
      if (max_weight(obj) > val1)
      {
         act(AT_ACTION, "The shear weight of $n was too much for $p.", ch, ch->on, NULL, TO_ROOM);
         act(AT_ACTION, "Your attempt to sit on $p caused it to break.", ch, ch->on, NULL, TO_CHAR);
         for (fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room)
         {
            if (fch->on == obj)
            {
               if (fch->position == POS_RESTING)
               {
                  fch->hit = (fch->hit - 30);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your rest is disrupted by you falling to the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_SLEEPING)
               {
                  fch->hit = (fch->hit - 40);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  fch->position = POS_RESTING;
                  act(AT_ACTION, "Your sleep is disrupted by your hard landing on the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_SITTING)
               {
                  fch->hit = (fch->hit - 5);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your lounging is disrupted by $p breaking.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_STANDING)
               {
                  fch->hit = (fch->hit - 55);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "You take a very bad fall after $p breaks.", fch, fch->on, NULL, TO_CHAR);
               }
               fch->on = NULL;
            }
         }
         make_scraps(obj, ch);
      }
   }

   return;
}


void do_sit(CHAR_DATA * ch, char *argument)
{

   OBJ_DATA *obj = NULL;
   int aon = 0;
   CHAR_DATA *fch = NULL;
   int val0;
   int val1;

   if (ch->position == POS_FIGHTING
      || ch->position == POS_BERSERK || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE)
   {
      send_to_char("Maybe you should finish this fight first?\n\r", ch);
      return;
   }

   if (ch->position == POS_MOUNTED)
   {
      send_to_char("You are already sitting - on your mount.\n\r", ch);
      return;
   }
   if (IS_AFFECTED(ch, AFF_SLEEP))
   {
      send_to_char("You can't wake up!\n\r", ch);
      return;
   }
   if (ch->rider)
   {
      send_to_char("You have someone on your back, need to have them get off before you can sit.\n\r", ch);
      return;
   }
   if (ch->riding)
   {
      send_to_char("You need to dismount first.\n\r", ch);
      return;
   }
   /* okay, now that we know we can sit, find an object to sit on */
   if (argument[0] != '\0')
   {
      obj = get_obj_list(ch, argument, ch->in_room->first_content);
      if (obj == NULL)
      {
         send_to_char("You don't see that here.\n\r", ch);
         return;
      }
      if (obj->item_type != ITEM_FURNITURE)
      {
         send_to_char("It has to be furniture silly.\n\r", ch);
         return;
      }
      if (!IS_SET(obj->value[2], SIT_ON) && !IS_SET(obj->value[2], SIT_IN) && !IS_SET(obj->value[2], SIT_AT))
      {
         send_to_char("You can't sit on that.\n\r", ch);
         return;
      }
      if (obj->value[0] == 0)
         val0 = 1;
      else
         val0 = obj->value[0];
      if (ch->on != obj && count_users(obj) >= val0)
      {
         act(AT_ACTION, "There's no room to sit on $p.", ch, obj, NULL, TO_CHAR);
         return;
      }
      if (ch->on == obj)
         aon = 1;
      else
         ch->on = obj;
   }
   if (ch->on && argument[0] == '\0')
   {
      if (!IS_SET(ch->on->value[2], SIT_AT) && !IS_SET(ch->on->value[2], SIT_ON) &&
          !IS_SET(ch->on->value[2], SIT_IN))
      {
         send_to_char("You cannot rest on that.\n\r", ch);
         return;
      }
   }
   switch (ch->position)
   {
      case POS_SLEEPING:
         if (obj == NULL)
         {
            send_to_char("You wake and sit up.\n\r", ch);
            act(AT_ACTION, "$n wakes and sits up.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], SIT_AT))
         {
            act(AT_ACTION, "You wake up and sit at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], SIT_ON))
         {
            act(AT_ACTION, "You wake and sit on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes and sits at $p.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You wake and sit in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes and sits in $p.", ch, obj, NULL, TO_ROOM);
         }

         ch->position = POS_SITTING;
         break;
      case POS_RESTING:
         if (obj == NULL)
         {
            send_to_char("You stop resting.\n\r", ch);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], SIT_AT))
         {
            act(AT_ACTION, "You sit at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sits at $p.", ch, obj, NULL, TO_ROOM);
         }

         else if (IS_SET(obj->value[2], SIT_ON))
         {
            act(AT_ACTION, "You sit on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sits on $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_SITTING;
         break;
      case POS_SITTING:
         if (obj != NULL && aon != 1)
         {

            if (IS_SET(obj->value[2], SIT_AT))
            {
               act(AT_ACTION, "You sit at $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n sits at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET(obj->value[2], STAND_ON))
            {
               act(AT_ACTION, "You sit on $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n sits on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
               act(AT_ACTION, "You sit in $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n sits on $p.", ch, obj, NULL, TO_ROOM);
            }
         }
         else if (aon == 1)
         {
            act(AT_ACTION, "You are already using $p for furniture.", ch, obj, NULL, TO_CHAR);
         }
         else if (ch->on != NULL && obj == NULL)
         {
            act(AT_ACTION, "You hop off of $p and sit on the ground.", ch, ch->on, NULL, TO_CHAR);
            act(AT_ACTION, "$n hops off of $p and sits on the ground.", ch, ch->on, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else
            send_to_char("You are already sitting.\n\r", ch);
         break;
      case POS_STANDING:
         if (obj == NULL)
         {
            send_to_char("You sit down.\n\r", ch);
            act(AT_ACTION, "$n sits down on the ground.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], SIT_AT))
         {
            act(AT_ACTION, "You sit down at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sits down at $p.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], SIT_ON))
         {
            act(AT_ACTION, "You sit on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sits on $p.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You sit down in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sits down in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_SITTING;
         break;
   }
   if (obj != NULL)
   {
      if (obj->value[1] == 0)
         val1 = 750;
      else
         val1 = obj->value[1];
      if (max_weight(obj) > val1)
      {
         act(AT_ACTION, "The shear weight of $n was too much for $p.", ch, ch->on, NULL, TO_ROOM);
         act(AT_ACTION, "Your attempt to sit on $p caused it to break.", ch, ch->on, NULL, TO_CHAR);
         for (fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room)
         {
            if (fch->on == obj)
            {
               if (fch->position == POS_RESTING)
               {
                  fch->hit = (fch->hit - 30);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your rest is disrupted by you falling to the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_SLEEPING)
               {
                  fch->hit = (fch->hit - 40);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  fch->position = POS_RESTING;
                  act(AT_ACTION, "Your sleep is disrupted by your hard landing on the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }

               if (fch->position == POS_SITTING)
               {
                  fch->hit = (fch->hit - 5);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your lounging is disrupted by $p breaking.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_STANDING)
               {
                  fch->hit = (fch->hit - 55);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "You take a very bad fall after $p breaks.", fch, fch->on, NULL, TO_CHAR);
               }
               fch->on = NULL;
            }
         }
         make_scraps(obj, ch);
      }
   }

   return;
}


void do_rest(CHAR_DATA * ch, char *argument)
{

   OBJ_DATA *obj = NULL;
   int aon = 0;
   CHAR_DATA *fch = NULL;
   int val0;
   int val1;

   if (ch->position == POS_FIGHTING
      || ch->position == POS_BERSERK || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE)
   {
      send_to_char("Maybe you should finish this fight first?\n\r", ch);
      return;
   }

   if (ch->position == POS_MOUNTED)
   {
      send_to_char("You are already sitting - on your mount.\n\r", ch);
      return;
   }
   if (IS_AFFECTED(ch, AFF_SLEEP))
   {
      send_to_char("You can't wake up!\n\r", ch);
      return;
   }
   if (ch->rider)
   {
      send_to_char("You have someone on your back, need to have them get off before you can rest.\n\r", ch);
      return;
   }
   if (ch->riding)
   {
      send_to_char("You need to dismount first.\n\r", ch);
      return;
   }
   /* okay, now that we know we can sit, find an object to sit on */
   if (argument[0] != '\0')
   {
      obj = get_obj_list(ch, argument, ch->in_room->first_content);
      if (obj == NULL)
      {
         send_to_char("You don't see that here.\n\r", ch);
         return;
      }
      if (obj->item_type != ITEM_FURNITURE)
      {
         send_to_char("It has to be furniture silly.\n\r", ch);
         return;
      }
      if (!IS_SET(obj->value[2], REST_ON) && !IS_SET(obj->value[2], REST_IN) && !IS_SET(obj->value[2], REST_AT))
      {
         send_to_char("You can't rest on that.\n\r", ch);
         return;
      }
      if (obj->value[0] == 0)
         val0 = 1;
      else
         val0 = obj->value[0];
      if (ch->on != obj && count_users(obj) >= val0)
      {
         act(AT_ACTION, "There's no room to rest on $p.", ch, obj, NULL, TO_CHAR);
         return;
      }
      if (ch->on == obj)
         aon = 1;
      else
         ch->on = obj;
   }
   if (ch->on && argument[0] == '\0')
   {
      if (!IS_SET(ch->on->value[2], REST_AT) && !IS_SET(ch->on->value[2], REST_ON) &&
          !IS_SET(ch->on->value[2], REST_IN))
      {
         send_to_char("You cannot rest on that.\n\r", ch);
         return;
      }
   }
      
   switch (ch->position)
   {
      case POS_SLEEPING:
         if (obj == NULL)
         {
            send_to_char("You wake up and start resting.\n\r", ch);
            act(AT_ACTION, "$n wakes up and starts resting.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], REST_AT))
         {
            act(AT_ACTION, "You wake up and rest at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes up and rests at $p.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], REST_ON))
         {
            act(AT_ACTION, "You wake up and rest on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes up and rests on $p.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You wake up and rest in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n wakes up and rests in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_RESTING;
         break;

      case POS_RESTING:
         if (obj != NULL && aon != 1)
         {

            if (IS_SET(obj->value[2], REST_AT))
            {
               act(AT_ACTION, "You rest at $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n rests at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET(obj->value[2], REST_ON))
            {
               act(AT_ACTION, "You rest on $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n rests on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
               act(AT_ACTION, "You rest in $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n rests on $p.", ch, obj, NULL, TO_ROOM);
            }
         }
         else if (aon == 1)
         {
            act(AT_ACTION, "You are already using $p for furniture.", ch, obj, NULL, TO_CHAR);
         }
         else if (ch->on != NULL && obj == NULL)
         {
            act(AT_ACTION, "You hop off of $p and start resting on the ground.", ch, ch->on, NULL, TO_CHAR);
            act(AT_ACTION, "$n hops off of $p and starts to rest on the ground.", ch, ch->on, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else
            send_to_char("You are already resting.\n\r", ch);
         break;

      case POS_STANDING:
         if (obj == NULL)
         {
            send_to_char("You rest.\n\r", ch);
            act(AT_ACTION, "$n sits down and rests.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], REST_AT))
         {
            act(AT_ACTION, "You sit down at $p and rest.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sits down at $p and rests.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], REST_ON))
         {
            act(AT_ACTION, "You sit on $p and rest.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sits on $p and rests.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You rest in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n rests in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_RESTING;
         break;

      case POS_SITTING:
         if (obj == NULL)
         {
            send_to_char("You rest.\n\r", ch);
            act(AT_ACTION, "$n rests.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], REST_AT))
         {
            act(AT_ACTION, "You rest at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n rests at $p.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], REST_ON))
         {
            act(AT_ACTION, "You rest on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n rests on $p.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You rest in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n rests in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_RESTING;
         break;
   }
   if (obj != NULL)
   {
      if (obj->value[1] == 0)
         val1 = 750;
      else
         val1 = obj->value[1];
      if (max_weight(obj) > val1)
      {
         act(AT_ACTION, "The shear weight of $n was too much for $p.", ch, ch->on, NULL, TO_ROOM);
         act(AT_ACTION, "Your attempt to sit on $p caused it to break.", ch, ch->on, NULL, TO_CHAR);
         for (fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room)
         {
            if (fch->on == obj)
            {
               if (fch->position == POS_RESTING)
               {
                  fch->hit = (fch->hit - 30);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your rest is disrupted by you falling to the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_SLEEPING)
               {
                  fch->hit = (fch->hit - 40);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  fch->position = POS_RESTING;
                  act(AT_ACTION, "Your sleep is disrupted by your hard landing on the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_SITTING)
               {
                  fch->hit = (fch->hit - 5);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your lounging is disrupted by $p breaking.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_STANDING)
               {
                  fch->hit = (fch->hit - 55);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "You take a very bad fall after $p breaks.", fch, fch->on, NULL, TO_CHAR);
               }
               fch->on = NULL;
            }
         }
         make_scraps(obj, ch);
      }
   }

   rprog_rest_trigger(ch);
   return;
}

void do_sleep(CHAR_DATA * ch, char *argument)
{
   OBJ_DATA *obj = NULL;
   int aon = 0;
   CHAR_DATA *fch = NULL;
   int val0;
   int val1;

   if (ch->position == POS_FIGHTING
      || ch->position == POS_BERSERK || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE)
   {
      send_to_char("Maybe you should finish this fight first?\n\r", ch);
      return;
   }

   if (ch->position == POS_MOUNTED)
   {
      send_to_char("If you wish to go to sleep, get off of your mount first.\n\r", ch);
      return;
   }
   if (ch->rider)
   {
      send_to_char("You have someone on your back, need to have them get off before you can sleep.\n\r", ch);
      return;
   }
   if (ch->riding)
   {
      send_to_char("You need to dismount first.\n\r", ch);
      return;
   }
   /* okay, now that we know we can sit, find an object to sit on */
   if (argument[0] != '\0')
   {
      obj = get_obj_list(ch, argument, ch->in_room->first_content);
      if (obj == NULL)
      {
         send_to_char("You don't see that here.\n\r", ch);
         return;
      }
      if (obj->item_type != ITEM_FURNITURE)
      {
         send_to_char("It has to be furniture silly.\n\r", ch);
         return;
      }
      if (!IS_SET(obj->value[2], SLEEP_ON) && !IS_SET(obj->value[2], SLEEP_IN) && !IS_SET(obj->value[2], SLEEP_AT))
      {
         send_to_char("You can't sleep on that.\n\r", ch);
         return;
      }
      if (obj->value[0] == 0)
         val0 = 1;
      else
         val0 = obj->value[0];
      if (ch->on != obj && count_users(obj) >= val0)
      {
         act(AT_ACTION, "There's no room to sleep on $p.", ch, obj, NULL, TO_CHAR);
         return;
      }
      if (ch->on == obj)
         aon = 1;
      else
         ch->on = obj;
   }
   if (ch->on && argument[0] == '\0')
   {
      if (!IS_SET(ch->on->value[2], SLEEP_AT) && !IS_SET(ch->on->value[2], SLEEP_ON) &&
          !IS_SET(ch->on->value[2], SLEEP_IN))
      {
         send_to_char("You cannot rest on that.\n\r", ch);
         return;
      }
   }
   switch (ch->position)
   {
      case POS_SLEEPING:
         if (obj != NULL && aon != 1)
         {

            if (IS_SET(obj->value[2], SLEEP_AT))
            {
               act(AT_ACTION, "You sleep at $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n sleeps at $p.", ch, obj, NULL, TO_ROOM);
            }
            else if (IS_SET(obj->value[2], SLEEP_ON))
            {
               act(AT_ACTION, "You sleep on $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n sleeps on $p.", ch, obj, NULL, TO_ROOM);
            }
            else
            {
               act(AT_ACTION, "You sleep in $p.", ch, obj, NULL, TO_CHAR);
               act(AT_ACTION, "$n sleeps on $p.", ch, obj, NULL, TO_ROOM);
            }
         }
         else if (aon == 1)
         {
            act(AT_ACTION, "You are already using $p for furniture.", ch, obj, NULL, TO_CHAR);
         }
         else if (ch->on != NULL && obj == NULL)
         {
            act(AT_ACTION, "You hop off of $p and try to sleep on the ground.", ch, ch->on, NULL, TO_CHAR);
            act(AT_ACTION, "$n hops off of $p and falls quickly asleep on the ground.", ch, ch->on, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else
            send_to_char("You are already sleeping.\n\r", ch);
         break;
      case POS_RESTING:
         if (obj == NULL)
         {
            send_to_char("You lean your head back more and go to sleep.\n\r", ch);
            act(AT_ACTION, "$n lies back and falls asleep on the ground.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], SLEEP_AT))
         {
            act(AT_ACTION, "You sleep at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps at $p.", ch, obj, NULL, TO_ROOM);
         }

         else if (IS_SET(obj->value[2], SLEEP_ON))
         {
            act(AT_ACTION, "You sleep on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps on $p.", ch, obj, NULL, TO_ROOM);
         }

         else
         {
            act(AT_ACTION, "You sleep in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_SLEEPING;
         break;

      case POS_SITTING:
         if (obj == NULL)
         {
            send_to_char("You lay down and go to sleep.\n\r", ch);
            act(AT_ACTION, "$n lies back and falls asleep on the ground.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], SLEEP_AT))
         {
            act(AT_ACTION, "You sleep at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps at $p.", ch, obj, NULL, TO_ROOM);
         }

         else if (IS_SET(obj->value[2], SLEEP_ON))
         {
            act(AT_ACTION, "You sleep on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps on $p.", ch, obj, NULL, TO_ROOM);
         }

         else
         {
            act(AT_ACTION, "You sleep in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_SLEEPING;

         break;
      case POS_STANDING:
         if (obj == NULL)
         {
            send_to_char("You drop down and fall asleep on the ground.\n\r", ch);
            act(AT_ACTION, "$n drops down and falls asleep on the ground.", ch, NULL, NULL, TO_ROOM);
            ch->on = NULL;
         }
         else if (IS_SET(obj->value[2], SLEEP_AT))
         {
            act(AT_ACTION, "You sleep at $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps at $p.", ch, obj, NULL, TO_ROOM);
         }
         else if (IS_SET(obj->value[2], SLEEP_ON))
         {
            act(AT_ACTION, "You sleep on $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps on $p.", ch, obj, NULL, TO_ROOM);
         }
         else
         {
            act(AT_ACTION, "You sleep down in $p.", ch, obj, NULL, TO_CHAR);
            act(AT_ACTION, "$n sleeps down in $p.", ch, obj, NULL, TO_ROOM);
         }
         ch->position = POS_SLEEPING;
         break;
   }
   if (obj != NULL)
   {
      if (obj->value[1] == 0)
         val1 = 750;
      else
         val1 = obj->value[1];
      if (max_weight(obj) > val1)
      {
         act(AT_ACTION, "The shear weight of $n was too much for $p.", ch, ch->on, NULL, TO_ROOM);
         act(AT_ACTION, "Your attempt to sit on $p caused it to break.", ch, ch->on, NULL, TO_CHAR);
         for (fch = obj->in_room->first_person; fch != NULL; fch = fch->next_in_room)
         {
            if (fch->on == obj)
            {
               if (fch->position == POS_RESTING)
               {
                  fch->hit = (fch->hit - 30);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your rest is disrupted by you falling to the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_SLEEPING)
               {
                  fch->hit = (fch->hit - 40);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  fch->position = POS_RESTING;
                  act(AT_ACTION, "Your sleep is disrupted by your hard landing on the ground after $p broke.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_SITTING)
               {
                  fch->hit = (fch->hit - 5);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "Your lounging is disrupted by $p breaking.", fch, fch->on, NULL, TO_CHAR);
               }
               if (fch->position == POS_STANDING)
               {
                  fch->hit = (fch->hit - 55);
                  if (fch->hit <= 0)
                     fch->hit = 1;
                  act(AT_ACTION, "You take a very bad fall after $p breaks.", fch, fch->on, NULL, TO_CHAR);
               }
               fch->on = NULL;
            }
         }
         make_scraps(obj, ch);
      }
   }

   rprog_sleep_trigger(ch);
   return;
}


void do_wake(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   CHAR_DATA *victim;

   one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      do_stand(ch, argument);
      return;
   }

   if (!IS_AWAKE(ch))
   {
      send_to_char("You are asleep yourself!\n\r", ch);
      return;
   }

   if ((victim = get_char_room_new(ch, arg, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }

   if (IS_AWAKE(victim))
   {
      act(AT_PLAIN, "$N is already awake.", ch, NULL, victim, TO_CHAR);
      return;
   }

   if (IS_AFFECTED(victim, AFF_SLEEP) || victim->position < POS_SLEEPING)
   {
      act(AT_PLAIN, "You can't seem to wake $M!", ch, NULL, victim, TO_CHAR);
      return;
   }

   act(AT_ACTION, "You wake $M.", ch, NULL, victim, TO_CHAR);
   victim->position = POS_STANDING;
   ch->on = NULL;
   act(AT_ACTION, "$n wakes you.", ch, NULL, victim, TO_VICT);
   return;
}


/*
 * teleport a character to another room
 */
void teleportch(CHAR_DATA * ch, ROOM_INDEX_DATA * room, bool show)
{
   char buf[MSL];

   if (room_is_private(room))
      return;
   act(AT_ACTION, "$n disappears suddenly!", ch, NULL, NULL, TO_ROOM);
   char_from_room(ch);
   char_to_room(ch, room);
   act(AT_ACTION, "$n arrives suddenly!", ch, NULL, NULL, TO_ROOM);
   if (show)
      do_look(ch, "auto");
   if (xIS_SET(ch->in_room->room_flags, ROOM_DEATH) && !IS_IMMORTAL(ch))
   {
      act(AT_DEAD, "$n falls prey to a terrible death!", ch, NULL, NULL, TO_ROOM);
      set_char_color(AT_DEAD, ch);
      send_to_char("Oopsie... you're dead!\n\r", ch);
      sprintf(buf, "%s hit a DEATH TRAP in room %d!", ch->name, ch->in_room->vnum);
      log_string(buf);
      to_channel(buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL);
      extract_char(ch, FALSE);
   }
}

void teleport(CHAR_DATA * ch, sh_int room, int flags)
{
   CHAR_DATA *nch, *nch_next;
   ROOM_INDEX_DATA *start = ch->in_room, *dest;
   bool show;

   dest = get_room_index(room);
   if (!dest)
   {
      bug("teleport: bad room vnum %d", room);
      return;
   }

   if (IS_SET(flags, TELE_SHOWDESC))
      show = TRUE;
   else
      show = FALSE;
   if (!IS_SET(flags, TELE_TRANSALL))
   {
      teleportch(ch, dest, show);
      return;
   }

   /* teleport everybody in the room */
   for (nch = start->first_person; nch; nch = nch_next)
   {
      nch_next = nch->next_in_room;
      teleportch(nch, dest, show);
   }

   /* teleport the objects on the ground too */
   if (IS_SET(flags, TELE_TRANSALLPLUS))
   {
      OBJ_DATA *obj, *obj_next;

      for (obj = start->first_content; obj; obj = obj_next)
      {
         obj_next = obj->next_content;
         obj_from_room(obj);
         obj_to_room(obj, dest, nch);
      }
   }
}

/*
 * "Climb" in a certain direction.				-Thoric
 */
void do_climb(CHAR_DATA * ch, char *argument)
{
   EXIT_DATA *pexit;
   bool found;

   found = FALSE;
   if (argument[0] == '\0')
   {
      for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
         if (IS_SET(pexit->exit_info, EX_xCLIMB))
         {
            move_char(ch, pexit, 0);
            return;
         }
      send_to_char("You cannot climb here.\n\r", ch);
      return;
   }

   if ((pexit = find_door(ch, argument, TRUE)) != NULL && IS_SET(pexit->exit_info, EX_xCLIMB))
   {
      move_char(ch, pexit, 0);
      return;
   }
   send_to_char("You cannot climb there.\n\r", ch);
   return;
}

/*
 * "enter" something (moves through an exit)			-Thoric
 * Can also use to enter portals, they are not exits anymore    -Xerves
 */
void do_enter(CHAR_DATA * ch, char *argument)
{
   EXIT_DATA *pexit;
   OBJ_DATA *obj;
   bool found;
   int mapd = -1;

   found = FALSE;

   if (argument[0] == '\0')
   {
      for (obj = ch->in_room->first_content; obj; obj = obj->next_content)
      {
         if (obj->item_type == ITEM_PORTAL)
         {
            if (obj->coord->x == ch->coord->x && obj->coord->y == ch->coord->y && obj->map == ch->map)
            {
               if (xIS_SET(ch->act, PLR_PORTALHUNT))
               {
                  send_to_char("You cannot enter a portal while hunting portals.\n\r", ch);
                  return;
               }
               if (obj->value[3] == ROOM_VNUM_PORTAL)
               {
                  if (!IS_NPC(ch))
                     mapd = ch->pcdata->mapdir;
                  leave_map(ch, get_room_index(obj->value[3]), mapd, 0);
                  return;
               }
               else
               {
                  enter_map(ch, obj->value[0], obj->value[1], obj->value[2]);
                  return;
               }
            }
         }
      }
      for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
         if (IS_SET(pexit->exit_info, EX_xENTER))
         {
            move_char(ch, pexit, 0);
            return;
         }
      if (ch->in_room->sector_type != SECT_INSIDE && IS_OUTSIDE(ch))
         for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
            if (pexit->to_room && (pexit->to_room->sector_type == SECT_INSIDE || xIS_SET(pexit->to_room->room_flags, ROOM_INDOORS)))
            {
               move_char(ch, pexit, 0);
               return;
            }
      send_to_char("You cannot find an entrance here.\n\r", ch);
      return;
   }
   if (!str_cmp("portal", argument))
   {
      for (obj = ch->in_room->first_content; obj; obj = obj->next_content)
      {
         if (obj->coord->x == ch->coord->x && obj->coord->y == ch->coord->y && obj->map == ch->map)
         {
            if (obj->item_type == ITEM_PORTAL)
            {
               if (xIS_SET(ch->act, PLR_PORTALHUNT))
               {
                  send_to_char("You cannot enter a portal while hunting portals.\n\r", ch);
                  return;
               }
               if (obj->value[3] == ROOM_VNUM_PORTAL)
               {
                  if (!IS_NPC(ch))
                     mapd = ch->pcdata->mapdir;
                  leave_map(ch, get_room_index(obj->value[3]), mapd, 0);
                  return;
               }
               else
               {
                  enter_map(ch, obj->value[0], obj->value[1], obj->value[2]);
                  return;
               }
            }
         }
      }
   }
   if ((pexit = find_door(ch, argument, TRUE)) != NULL && IS_SET(pexit->exit_info, EX_xENTER))
   {
      move_char(ch, pexit, 0);
      return;
   }
   send_to_char("You cannot enter that.\n\r", ch);
   return;
}

/*
 * Leave through an exit.					-Thoric
 */
void do_leave(CHAR_DATA * ch, char *argument)
{
   EXIT_DATA *pexit;
   bool found;

   found = FALSE;
   if (argument[0] == '\0')
   {
      for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
         if (IS_SET(pexit->exit_info, EX_xLEAVE))
         {
            move_char(ch, pexit, 0);
            return;
         }
      if (ch->in_room->sector_type == SECT_INSIDE || !IS_OUTSIDE(ch))
         for (pexit = ch->in_room->first_exit; pexit; pexit = pexit->next)
            if (pexit->to_room && pexit->to_room->sector_type != SECT_INSIDE && !xIS_SET(pexit->to_room->room_flags, ROOM_INDOORS))
            {
               move_char(ch, pexit, 0);
               return;
            }
      send_to_char("You cannot find an exit here.\n\r", ch);
      return;
   }

   if ((pexit = find_door(ch, argument, TRUE)) != NULL && IS_SET(pexit->exit_info, EX_xLEAVE))
   {
      move_char(ch, pexit, 0);
      return;
   }
   send_to_char("You cannot leave that way.\n\r", ch);
   return;
}

/*
 * Check to see if an exit in the room is pulling (or pushing) players around.
 * Some types may cause damage.					-Thoric
 *
 * People kept requesting currents (like SillyMUD has), so I went all out
 * and added the ability for an exit to have a "pull" or a "push" force
 * and to handle different types much beyond a simple water current.
 *
 * This check is called by violence_update().  I'm not sure if this is the
 * best way to do it, or if it should be handled by a special queue.
 *
 * Future additions to this code may include equipment being blown away in
 * the wind (mostly headwear), and people being hit by flying objects
 *
 * TODO:
 *	handle more pulltypes
 *	give "entrance" messages for players and objects
 *	proper handling of player resistance to push/pulling
 */
ch_ret pullcheck(CHAR_DATA * ch, int pulse)
{
   ROOM_INDEX_DATA *room;
   EXIT_DATA *xtmp, *xit = NULL;
   OBJ_DATA *obj, *obj_next;
   bool move = FALSE, moveobj = TRUE, showroom = TRUE;
   int pullfact, pull;
   int resistance;
   char *tochar = NULL, *toroom = NULL, *objmsg = NULL;
   char *destrm = NULL, *destob = NULL, *dtxt = "somewhere";

   if ((room = ch->in_room) == NULL)
   {
      bug("pullcheck: %s not in a room?!?", ch->name);
      return rNONE;
   }

   /* Find the exit with the strongest force (if any) */
   for (xtmp = room->first_exit; xtmp; xtmp = xtmp->next)
      if (xtmp->pull && xtmp->to_room && (!xit || abs(xtmp->pull) > abs(xit->pull)))
         xit = xtmp;

   if (!xit)
      return rNONE;

   pull = xit->pull;

   /* strength also determines frequency */
   pullfact = URANGE(1, 20 - (abs(pull) / 5), 20);

   /* strongest pull not ready yet... check for one that is */
   if ((pulse % pullfact) != 0)
   {
      for (xit = room->first_exit; xit; xit = xit->next)
         if (xit->pull && xit->to_room)
         {
            pull = xit->pull;
            pullfact = URANGE(1, 20 - (abs(pull) / 5), 20);
            if ((pulse % pullfact) != 0)
               break;
         }

      if (!xit)
         return rNONE;
   }

   /* negative pull = push... get the reverse exit if any */
   if (pull < 0)
      if ((xit = get_exit(room, rev_dir[xit->vdir])) == NULL)
         return rNONE;

   dtxt = rev_exit(xit->vdir);

   /*
    * First determine if the player should be moved or not
    * Check various flags, spells, the players position and strength vs.
    * the pull, etc... any kind of checks you like.
    */
   switch (xit->pulltype)
   {
      case PULL_CURRENT:
      case PULL_WHIRLPOOL:
         switch (room->sector_type)
         {
               /* allow whirlpool to be in any sector type */
            default:
               if (xit->pulltype == PULL_CURRENT)
                  break;
            case SECT_WATER_SWIM:
            case SECT_WATER_NOSWIM:
               if ((ch->mount && !IS_FLOATING(ch->mount)) || (!ch->mount && !IS_FLOATING(ch)))
                  move = TRUE;
               break;

            case SECT_UNDERWATER:
            case SECT_OCEANFLOOR:
               move = TRUE;
               break;
         }
         break;
      case PULL_GEYSER:
      case PULL_WAVE:
         move = TRUE;
         break;

      case PULL_WIND:
      case PULL_STORM:
         /* if not flying... check weight, position & strength */
         move = TRUE;
         break;

      case PULL_COLDWIND:
         /* if not flying... check weight, position & strength */
         /* also check for damage due to bitter cold */
         move = TRUE;
         break;

      case PULL_HOTAIR:
         /* if not flying... check weight, position & strength */
         /* also check for damage due to heat */
         move = TRUE;
         break;

         /* light breeze -- very limited moving power */
      case PULL_BREEZE:
         move = FALSE;
         break;

         /*
          * exits with these pulltypes should also be blocked from movement
          * ie: a secret locked pickproof door with the name "_sinkhole_", etc
          */
      case PULL_EARTHQUAKE:
      case PULL_SINKHOLE:
      case PULL_QUICKSAND:
      case PULL_LANDSLIDE:
      case PULL_SLIP:
      case PULL_LAVA:
         if ((ch->mount && !IS_FLOATING(ch->mount)) || (!ch->mount && !IS_FLOATING(ch)))
            move = TRUE;
         break;

         /* as if player moved in that direction him/herself */
      case PULL_UNDEFINED:
         return move_char(ch, xit, 0);

         /* all other cases ALWAYS move */
      default:
         move = TRUE;
         break;
   }

   /* assign some nice text messages */
   switch (xit->pulltype)
   {
      case PULL_MYSTERIOUS:
         /* no messages to anyone */
         showroom = FALSE;
         break;
      case PULL_WHIRLPOOL:
      case PULL_VACUUM:
         tochar = "You are sucked $T!";
         toroom = "$n is sucked $T!";
         destrm = "$n is sucked in from $T!";
         objmsg = "$p is sucked $T.";
         destob = "$p is sucked in from $T!";
         break;
      case PULL_CURRENT:
      case PULL_LAVA:
         tochar = "You drift $T.";
         toroom = "$n drifts $T.";
         destrm = "$n drifts in from $T.";
         objmsg = "$p drifts $T.";
         destob = "$p drifts in from $T.";
         break;
      case PULL_BREEZE:
         tochar = "You drift $T.";
         toroom = "$n drifts $T.";
         destrm = "$n drifts in from $T.";
         objmsg = "$p drifts $T in the breeze.";
         destob = "$p drifts in from $T.";
         break;
      case PULL_GEYSER:
      case PULL_WAVE:
         tochar = "You are pushed $T!";
         toroom = "$n is pushed $T!";
         destrm = "$n is pushed in from $T!";
         destob = "$p floats in from $T.";
         break;
      case PULL_EARTHQUAKE:
         tochar = "The earth opens up and you fall $T!";
         toroom = "The earth opens up and $n falls $T!";
         destrm = "$n falls from $T!";
         objmsg = "$p falls $T.";
         destob = "$p falls from $T.";
         break;
      case PULL_SINKHOLE:
         tochar = "The ground suddenly gives way and you fall $T!";
         toroom = "The ground suddenly gives way beneath $n!";
         destrm = "$n falls from $T!";
         objmsg = "$p falls $T.";
         destob = "$p falls from $T.";
         break;
      case PULL_QUICKSAND:
         tochar = "You begin to sink $T into the quicksand!";
         toroom = "$n begins to sink $T into the quicksand!";
         destrm = "$n sinks in from $T.";
         objmsg = "$p begins to sink $T into the quicksand.";
         destob = "$p sinks in from $T.";
         break;
      case PULL_LANDSLIDE:
         tochar = "The ground starts to slide $T, taking you with it!";
         toroom = "The ground starts to slide $T, taking $n with it!";
         destrm = "$n slides in from $T.";
         objmsg = "$p slides $T.";
         destob = "$p slides in from $T.";
         break;
      case PULL_SLIP:
         tochar = "You lose your footing!";
         toroom = "$n loses $s footing!";
         destrm = "$n slides in from $T.";
         objmsg = "$p slides $T.";
         destob = "$p slides in from $T.";
         break;
      case PULL_VORTEX:
         tochar = "You are sucked into a swirling vortex of colors!";
         toroom = "$n is sucked into a swirling vortex of colors!";
         toroom = "$n appears from a swirling vortex of colors!";
         objmsg = "$p is sucked into a swirling vortex of colors!";
         objmsg = "$p appears from a swirling vortex of colors!";
         break;
      case PULL_HOTAIR:
         tochar = "A blast of hot air blows you $T!";
         toroom = "$n is blown $T by a blast of hot air!";
         destrm = "$n is blown in from $T by a blast of hot air!";
         objmsg = "$p is blown $T.";
         destob = "$p is blown in from $T.";
         break;
      case PULL_COLDWIND:
         tochar = "A bitter cold wind forces you $T!";
         toroom = "$n is forced $T by a bitter cold wind!";
         destrm = "$n is forced in from $T by a bitter cold wind!";
         objmsg = "$p is blown $T.";
         destob = "$p is blown in from $T.";
         break;
      case PULL_WIND:
         tochar = "A strong wind pushes you $T!";
         toroom = "$n is blown $T by a strong wind!";
         destrm = "$n is blown in from $T by a strong wind!";
         objmsg = "$p is blown $T.";
         destob = "$p is blown in from $T.";
         break;
      case PULL_STORM:
         tochar = "The raging storm drives you $T!";
         toroom = "$n is driven $T by the raging storm!";
         destrm = "$n is driven in from $T by a raging storm!";
         objmsg = "$p is blown $T.";
         destob = "$p is blown in from $T.";
         break;
      default:
         if (pull > 0)
         {
            tochar = "You are pulled $T!";
            toroom = "$n is pulled $T.";
            destrm = "$n is pulled in from $T.";
            objmsg = "$p is pulled $T.";
            objmsg = "$p is pulled in from $T.";
         }
         else
         {
            tochar = "You are pushed $T!";
            toroom = "$n is pushed $T.";
            destrm = "$n is pushed in from $T.";
            objmsg = "$p is pushed $T.";
            objmsg = "$p is pushed in from $T.";
         }
         break;
   }


   /* Do the moving */
   if (move)
   {
      /* display an appropriate exit message */
      if (tochar)
      {
         act(AT_PLAIN, tochar, ch, NULL, dir_name[xit->vdir], TO_CHAR);
         send_to_char("\n\r", ch);
      }
      if (toroom)
         act(AT_PLAIN, toroom, ch, NULL, dir_name[xit->vdir], TO_ROOM);

      /* display an appropriate entrance message */
      if (destrm && xit->to_room->first_person)
      {
         act(AT_PLAIN, destrm, xit->to_room->first_person, NULL, dtxt, TO_CHAR);
         act(AT_PLAIN, destrm, xit->to_room->first_person, NULL, dtxt, TO_ROOM);
      }


      /* move the char */
      if (xit->pulltype == PULL_SLIP)
         return move_char(ch, xit, 1);
      char_from_room(ch);
      char_to_room(ch, xit->to_room);

      if (showroom)
         do_look(ch, "auto");

      /* move the mount too */
      if (ch->mount)
      {
         char_from_room(ch->mount);
         char_to_room(ch->mount, xit->to_room);
         if (showroom)
            do_look(ch->mount, "auto");
      }
   }

   /* move objects in the room */
   if (moveobj)
   {
      for (obj = room->first_content; obj; obj = obj_next)
      {
         obj_next = obj->next_content;

         if (IS_OBJ_STAT(obj, ITEM_BURIED) || !CAN_WEAR(obj, ITEM_TAKE))
            continue;

         resistance = get_obj_weight(obj);
         if (IS_OBJ_STAT(obj, ITEM_METAL))
            resistance = (resistance * 6) / 5;
         switch (obj->item_type)
         {
            case ITEM_SCROLL:
            case ITEM_NOTE:
            case ITEM_TRASH:
               resistance >>= 2;
               break;
            case ITEM_SCRAPS:
            case ITEM_CONTAINER:
               resistance >>= 1;
               break;
            case ITEM_PEN:
            case ITEM_WAND:
               resistance = (resistance * 5) / 6;
               break;

            case ITEM_CORPSE_PC:
            case ITEM_CORPSE_NPC:
            case ITEM_FOUNTAIN:
               resistance <<= 2;
               break;
         }

         /* is the pull greater than the resistance of the object? */
         if ((abs(pull) * 10) > resistance)
         {
            if (objmsg && room->first_person)
            {
               act(AT_PLAIN, objmsg, room->first_person, obj, dir_name[xit->vdir], TO_CHAR);
               act(AT_PLAIN, objmsg, room->first_person, obj, dir_name[xit->vdir], TO_ROOM);
            }
            if (destob && xit->to_room->first_person)
            {
               act(AT_PLAIN, destob, xit->to_room->first_person, obj, dtxt, TO_CHAR);
               act(AT_PLAIN, destob, xit->to_room->first_person, obj, dtxt, TO_ROOM);
            }
            obj_from_room(obj);
            obj_to_room(obj, xit->to_room, xit->to_room->first_person);
         }
      }
   }

   return rNONE;
}

void do_jog(CHAR_DATA * ch, char *argument)
{
   char buf[MIL], arg[MAX_INPUT_LENGTH];
   char *p;
   bool dFound = FALSE;

   if (!ch->desc || *argument == '\0')
   {
      send_to_char("You run in place!\n\r", ch);
      return;
   }

   buf[0] = '\0';

   while (*argument != '\0')
   {
      argument = one_argument(argument, arg);
      strcat(buf, arg);
   }

   for (p = buf + strlen(buf) - 1; p >= buf; p--)
   {
      if (!isdigit(*p))
      {
         switch (*p)
         {
            case 'n':
            case 's':
            case 'e':
            case 'w':
            case 'u':
            case 'd':
               dFound = TRUE;
               break;

            case 'o':
               break;

            case '+':
               p--;
               break;

            default:
               send_to_char("Invalid direction!\n\r", ch);
               return;
         }
      }
      else if (!dFound)
         *p = '\0';
   }

   if (!dFound)
   {
      send_to_char("No directions specified!\n\r", ch);
      return;
   }

   ch->desc->run_buf = str_dup(buf);
   ch->desc->run_head = ch->desc->run_buf;
   send_to_char("You start running...\n\r", ch);
   return;
}

void do_run(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *from_room;
   EXIT_DATA *pexit;

   one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      send_to_char("Run where?\n\r", ch);
      return;
   }

   if (ch->position != POS_STANDING)
   {
      send_to_char("You are not in the correct position for that.\n\r", ch);
      return;
   }

   from_room = ch->in_room;

   while ((pexit = find_door(ch, arg, TRUE)) != NULL)
   {
      if (ch->move < 1)
      {
         send_to_char("You are too exhausted to run anymore.\n\r", ch);
         ch->move = 0;
         break;
      }

      ch->move -= 1;
      if (move_char(ch, pexit, 0) == rSTOP)
         break;
   }

   if (ch->in_room == from_room)
   {
      send_to_char("You try to run but don't get anywhere.\n\r", ch);
      act(AT_ACTION, "$n tries to run but doesn't get anywhere.", ch, NULL, NULL, TO_ROOM);
      return;
   }

   send_to_char("You slow down after your run.\n\r", ch);
   act(AT_ACTION, "$n slows down after $s run.", ch, NULL, NULL, TO_ROOM);
}
