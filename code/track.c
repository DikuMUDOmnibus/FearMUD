/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 *			 Tracking/hunting module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


#define BFS_ERROR	   -1
#define BFS_ALREADY_THERE  -2
#define BFS_NO_PATH	   -3

#define TRACK_THROUGH_DOORS

extern sh_int top_room;

/* You can define or not define TRACK_THOUGH_DOORS, above, depending on
   whether or not you want track to find paths which lead through closed
   or hidden doors.
*/

typedef struct bfs_queue_struct BFS_DATA;
struct bfs_queue_struct
{
   ROOM_INDEX_DATA *room;
   char dir;
   BFS_DATA *next;
};

static BFS_DATA *queue_head = NULL, *queue_tail = NULL, *room_queue = NULL;

/* Utility macros */
#define MARK(room)	(xSET_BIT(	(room)->room_flags, ROOM_MARK) )
#define UNMARK(room)	(xREMOVE_BIT(	(room)->room_flags, ROOM_MARK) )
#define IS_MARKED(room)	(xIS_SET(	(room)->room_flags, ROOM_MARK) )

bool valid_edge(EXIT_DATA * pexit)
{
   if (pexit->to_room
#ifndef TRACK_THROUGH_DOORS
      && !IS_SET(pexit->exit_info, EX_CLOSED)
#endif
      && !IS_MARKED(pexit->to_room))
      return TRUE;
   else
      return FALSE;
}

void bfs_enqueue(ROOM_INDEX_DATA * room, char dir)
{
   BFS_DATA *curr;

   curr = malloc(sizeof(BFS_DATA));
   curr->room = room;
   curr->dir = dir;
   curr->next = NULL;

   if (queue_tail)
   {
      queue_tail->next = curr;
      queue_tail = curr;
   }
   else
      queue_head = queue_tail = curr;
}


void bfs_dequeue(void)
{
   BFS_DATA *curr;

   curr = queue_head;

   if (!(queue_head = queue_head->next))
      queue_tail = NULL;
   free(curr);
}


void bfs_clear_queue(void)
{
   while (queue_head)
      bfs_dequeue();
}

void room_enqueue(ROOM_INDEX_DATA * room)
{
   BFS_DATA *curr;

   curr = malloc(sizeof(BFS_DATA));
   curr->room = room;
   curr->next = room_queue;

   room_queue = curr;
}

void clean_room_queue(void)
{
   BFS_DATA *curr, *curr_next;

   for (curr = room_queue; curr; curr = curr_next)
   {
      UNMARK(curr->room);
      curr_next = curr->next;
      free(curr);
   }
   room_queue = NULL;
}


int find_first_step(ROOM_INDEX_DATA * src, ROOM_INDEX_DATA * target, int maxdist, int *steps)
{
   int curr_dir, count;
   EXIT_DATA *pexit;

   if (!src || !target)
   {
      bug("Illegal value passed to find_first_step (track.c)", 0);
      return BFS_ERROR;
   }

   if (src == target)
      return BFS_ALREADY_THERE;

   if (src->area != target->area)
      return BFS_NO_PATH;

   room_enqueue(src);
   MARK(src);

   /* first, enqueue the first steps, saving which direction we're going. */
   for (pexit = src->first_exit; pexit; pexit = pexit->next)
      if (valid_edge(pexit))
      {
         curr_dir = pexit->vdir;
         MARK(pexit->to_room);
         room_enqueue(pexit->to_room);
         bfs_enqueue(pexit->to_room, curr_dir);
      }

   count = 0;
   while (queue_head)
   {
      if (++count > maxdist)
      {
         bfs_clear_queue();
         clean_room_queue();
         return BFS_NO_PATH;
      }
      if (queue_head->room == target)
      {
         curr_dir = queue_head->dir;
         bfs_clear_queue();
         clean_room_queue();
         if (steps)
            *steps = count;
         return curr_dir;
      }
      else
      {
         for (pexit = queue_head->room->first_exit; pexit; pexit = pexit->next)
            if (valid_edge(pexit))
            {
               curr_dir = pexit->vdir;
               MARK(pexit->to_room);
               room_enqueue(pexit->to_room);
               bfs_enqueue(pexit->to_room, queue_head->dir);
            }
         bfs_dequeue();
      }
   }
   clean_room_queue();

   return BFS_NO_PATH;
}

char *get_wtrack_dir(CHAR_DATA *ch, CHAR_DATA *victim)
{
   int diffx, diffy;
   int distx, disty, total;
   
   diffx = victim->coord->x - ch->coord->x;
   diffy = victim->coord->y - ch->coord->y;
   distx = abs(ch->coord->x - victim->coord->x);
   disty = abs(ch->coord->y - victim->coord->y);
   total = distx+disty;
   
   if (total == 1)
   {
      if (diffy > 0)
         return "south";
      else if (diffy < 0)
         return "north";
      else if (diffx > 0)
         return "east";
      else
         return "west";
   }
   if (diffy >= total/2) //S
   {
      if (diffx >= total/2) //se
         return "southeast";
      else if (diffx <= total/-2) //sw
         return "southwest";
      else
         return "south";
   }
   if (diffy <= total/-2) //N
   {
      if (diffx >= total/2) //ne
         return "northeast";
      else if (diffx <= total/-2) //nw
         return "northwest";
      else
         return "north";
   }   
   if (diffx >= total/2) //E
   {
      if (diffy >= total/2) //se
         return "southeast";
      else if (diffy <= total/-2) //ne
         return "northeast";
      else
         return "east";
   }
   if (diffx <= total/-2) //W
   {
      if (diffy >= total/2) //we
         return "southwest";
      else if (diffy <= total/-2) //we
         return "northwest";
      else
         return "west";
   }    
   return "unknown";
}
   

void do_track(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *vict;
   char arg[MIL];
   char buf[MSL];
   char name[MSL];
   int dir, maxdist;
   sh_int level;
   sh_int percent;
   int steps;
   int fnd = 0;
   int cnt = 0;

   level = POINT_LEVEL(LEARNED(ch, gsn_track), MASTERED(ch, gsn_track));

   if (!IS_NPC(ch) && (ch->pcdata->learned[gsn_track] <= 0 || ch->pcdata->ranking[gsn_track] <= 0))
   {
      send_to_char("You do not know of this skill yet.\n\r", ch);
      return;
   }
   
   one_argument(argument, arg);
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  track <victim>\n\r", ch);
      send_to_char("Syntax:  track area [color] [color..]\n\r", ch);
      send_to_char("Color being: red pink yellow cyan white green\n\r", ch);
      return;
   }

   WAIT_STATE(ch, skill_table[gsn_track]->beats);
   
   if (!str_cmp(arg, "area"))
   {
      int red, pink, yellow, white, green, cyan;
      red=pink=yellow=white=green=cyan=1;
      
      argument = one_argument(argument, arg);
      
      if (argument[0] != '\0')
      {
         red=pink=yellow=white=green=cyan=0;
         for (;;)
         {
            argument = one_argument(argument, arg);
            if (arg[0] == '\0')
               break;
            else
            {
               if (!str_cmp(arg, "red"))
                  red = 1;
               if (!str_cmp(arg, "pink"))
                  pink = 1;
               if (!str_cmp(arg, "yellow"))
                  yellow = 1;
               if (!str_cmp(arg, "cyan"))
                  cyan = 1;
               if (!str_cmp(arg, "white"))
                  white = 1;
               if (!str_cmp(arg, "green"))
                  green = 1;
            }
         }
      }             
      for (vict = first_char; vict; vict = vict->next)
      {
         if (vict->in_room && vict->in_room->area == ch->in_room->area)
         {
            if (xIS_SET(vict->act, ACT_NOTRACK))
               continue;   
            if (!can_see_map(ch, vict))
               continue;
            if (IN_WILDERNESS(ch))
            {
               percent = UMIN(5+level/2, 45);
               if (abs(ch->coord->x - vict->coord->x) > percent || abs(ch->coord->y - vict->coord->y) > percent)
                  continue;       
               steps = UMAX(abs(ch->coord->x - vict->coord->x), abs(ch->coord->y - vict->coord->y));
               steps = steps * 5;
            }
            else
            {
               maxdist = 250;
               if (!IS_NPC(ch))
                  maxdist = (maxdist * level / 60);
               steps = 0;
               dir = find_first_step(ch->in_room, vict->in_room, maxdist, &steps);
               if (dir == BFS_NO_PATH || dir == BFS_ERROR)
                  continue;
            }
            percent = 50 + level;
            if (number_range(1, 100) > percent)
               continue;
            fnd++;
            sprintf(name, "%.20s", PERS_MAP_NAME(vict, ch));
            sprintf(buf, MXPFTAG("Command 'track \"%s\"' desc='Click here to track the target'", "%s", "/Command") "%s", 
              PERS_MAP_NAME(vict, ch), name, add_space(strlen(name), 20));
            if (steps <= 15 && red)
            {
               ch_printf(ch, "   &w&R%s", buf);
               cnt++;
            }
            else if (steps >= 16 && steps <= 35 && pink)
            {
               ch_printf(ch, "   &w&P%s", buf);
               cnt++;
            }
            else if (steps >= 36 && steps <= 60 && yellow)
            {
               ch_printf(ch, "   &w&Y%s", buf);
               cnt++;
            }
            else if (steps >= 61 && steps <= 100 && cyan)
            {
               ch_printf(ch, "   &w&c%s", buf);
               cnt++;
            }
            else if (steps >= 101 && steps <= 150 && white)
            {
               ch_printf(ch, "   &w&W%s", buf);
               cnt++;
            }
            else if (steps > 150 && green)
            {
               ch_printf(ch, "   &w&G%s", buf);
               cnt++;
            }
            else
               continue;
            if (cnt % 4 == 0)
               send_to_char("\n\r", ch);
         } 
      }
      if (fnd > 0)
         learn_from_success(ch, gsn_track, vict);
      if (cnt > 0)
         send_to_char("\n\r", ch);
      return;
   }
   if (!(vict = get_char_world(ch, arg)))
   {
      send_to_char("You can't find a trail of anyone like that.\n\r", ch);
      return;
   }

   if (xIS_SET(vict->act, ACT_NOTRACK))
   {
      send_to_char("You can't find a trail of anyone like that.\n\r", ch);
      return;
   }
   if (IN_WILDERNESS(ch) || IN_WILDERNESS(vict))
   {
      if (IN_WILDERNESS(ch) && IN_WILDERNESS(vict))
      {
         percent = UMIN(10+level, 90);
         if (abs(ch->coord->x - vict->coord->x) > percent || abs(ch->coord->y - vict->coord->y) > percent)
         {
            send_to_char("You cannot pickup a trail from here.\n\r", ch);
            learn_from_failure(ch, gsn_track, vict);
            return;
         }
         else
         {
            percent = 10 + level*3/2;
            if (number_range(1, 100) <= percent)
            {
               ch_printf(ch, "You sense %s is located to the %s\n\r", PERS_MAP_NAME(vict, ch), get_wtrack_dir(ch, vict));
               learn_from_success(ch, gsn_track, vict);
               return;
            }
            else
            {
               send_to_char("You cannot sense a trail from here.\n\r", ch);
               learn_from_failure(ch, gsn_track, vict);
               return;
            }
         }
      }
      else
      {
         send_to_char("You can't find a trail of anyone like that.\n\r", ch);
         return;
      }
   }

   maxdist = 500;
   if (!IS_NPC(ch))
      maxdist = (maxdist * level / 60);

   percent = 10 + level*3/2;
   if (number_range(1, 100) > percent)
   {
      send_to_char("You can't sense a trail from here.\n\r", ch);
      learn_from_failure(ch, gsn_track, vict);
      return;
   }   

   dir = find_first_step(ch->in_room, vict->in_room, maxdist, NULL);

   switch (dir)
   {
      case BFS_ERROR:
         send_to_char("Hmm... something seems to be wrong.\n\r", ch);
         break;
      case BFS_ALREADY_THERE:
         send_to_char("You're already in the same room!\n\r", ch);
         break;
      case BFS_NO_PATH:
         send_to_char("You can't sense a trail from here.\n\r", ch);
         learn_from_failure(ch, gsn_track, vict);
         break;
      default:
         ch_printf(ch, "You sense a trail %s from here...\n\r", dir_name[dir]);
         learn_from_success(ch, gsn_track, vict);
         break;
   }
}


void found_prey(CHAR_DATA * ch, CHAR_DATA * victim)
{
   char buf[MSL];
   char victname[MSL];

   if (victim == NULL)
   {
      bug("Found_prey: null victim", 0);
      return;
   }

   if (victim->in_room == NULL)
   {
      bug("Found_prey: null victim->in_room", 0);
      return;
   }
   if (!IS_AWAKE(ch) || IS_AFFECTED(ch, AFF_WEB))
      return;
   sprintf(victname, IS_NPC(victim) ? victim->short_descr : victim->name);

   if (!can_see(ch, victim))
   {
      if (number_percent() < 90)
         return;
      switch (number_bits(3))
      {
         case 0:
            sprintf(buf, "Don't make me find you, %s!", victname);
            do_say(ch, buf);
            break;
         case 1:
         case 2:
            act(AT_ACTION, "$n sniffs around the room for $N.", ch, NULL, victim, TO_NOTVICT);
            act(AT_ACTION, "You sniff around the room for $N.", ch, NULL, victim, TO_CHAR);
            act(AT_ACTION, "$n sniffs around the room for you.", ch, NULL, victim, TO_VICT);
            sprintf(buf, "I can smell your blood!");
            do_say(ch, buf);
            break;
         case 3:
            sprintf(buf, "I'm going to tear %s apart!", victname);
            do_yell(ch, buf);
            break;
         case 4:
         case 5:
            do_say(ch, "Just wait until I find you...");
            break;
         case 6:
         case 7:
            do_say(ch, "To hell with you...");
            stop_hunting(ch);
            stop_hating(ch);
            if (IN_WILDERNESS(ch))
               find_next_hunt(ch, 0);
            break;
      }
      return;
   }

   if (is_room_safe(ch))
   {
      if (number_percent() < 90)
         return;
      switch (number_bits(2))
      {
         case 0:
            do_say(ch, "C'mon out, you coward!");
            sprintf(buf, "%s is a bloody coward!", victname);
            do_yell(ch, buf);
            break;
         case 1:
            sprintf(buf, "Let's take this outside, %s", victname);
            do_say(ch, buf);
            break;
         case 2:
            sprintf(buf, "%s is a yellow-bellied wimp!", victname);
            do_yell(ch, buf);
            break;
         case 3:
            act(AT_ACTION, "$n takes a few swipes at $N.", ch, NULL, victim, TO_NOTVICT);
            act(AT_ACTION, "You try to take a few swipes $N.", ch, NULL, victim, TO_CHAR);
            act(AT_ACTION, "$n takes a few swipes at you.", ch, NULL, victim, TO_VICT);
            break;
      }
      return;
   }

   switch (number_bits(2))
   {
      case 0:
         sprintf(buf, "%s Your blood is mine!", victname);
         do_tell(ch, buf);
         break;
      case 1:
         sprintf(buf, "%s Alas, we meet again!", victname);
         do_tell(ch, buf);
         break;
      case 2:
         sprintf(buf, "%s What do you want on your tombstone?", victname);
         do_tell(ch, buf);
         break;
      case 3:
         act(AT_ACTION, "$n lunges at $N from out of nowhere!", ch, NULL, victim, TO_NOTVICT);
         act(AT_ACTION, "You lunge at $N catching $M off guard!", ch, NULL, victim, TO_CHAR);
         act(AT_ACTION, "$n lunges at you from out of nowhere!", ch, NULL, victim, TO_VICT);
   }
   //stop_hunting(ch);
   set_fighting(ch, victim);
   one_hit(ch, victim, TYPE_UNDEFINED, LM_BODY);
   return;
}

void hunt_victim(CHAR_DATA * ch)
{
   bool found;
   CHAR_DATA *tmp;
   EXIT_DATA *pexit;
   sh_int ret;
   int chance;

   if (!ch || !ch->hunting)
      return;
      
   if (ch->fight_timer > 0)
      return;
      
   /* make sure the char still exists */
   for (found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next)
      if (ch->hunting->who == tmp)
         found = TRUE;

   if (!found)
   {
      do_say(ch, "Damn!  My prey is gone!!");
      stop_hunting(ch);
      return;
   }

   if (ch->in_room == ch->hunting->who->in_room)
   {
      if (ch->fighting)
         return;
      found_prey(ch, ch->hunting->who);
      return;
   }

   ret = find_first_step(ch->in_room, ch->hunting->who->in_room, 500, NULL);
   if (ret < 0)
   {
      do_say(ch, "Damn!  Lost my prey!");
      stop_hunting(ch);
      return;
   }
   else
   {
      if ((pexit = get_exit(ch->in_room, ret)) == NULL)
      {
         bug("Hunt_victim: lost exit?", 0);
         return;
      }
      chance = get_hunt_cost(pexit->to_room);
      if (chance <= 1 || number_range(1, chance) == 1)
      {
         if (!xIS_SET(ch->act, ACT_MILITARY) || (xIS_SET(ch->act, ACT_MILITARY) && !xIS_SET(ch->miflags, KM_SENTINEL)))
            move_char(ch, pexit, FALSE);
      }
      if (!ch->hunting)
      {
         if (!ch->in_room)
         {
            bug("Hunt_victim: no ch->in_room!  Mob #%d, name: %s.  Placing mob in limbo.", ch->pIndexData->vnum, ch->name);
            char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
            return;
         }
         do_say(ch, "Damn!  Lost my prey!");
         return;
      }
      if (ch->in_room == ch->hunting->who->in_room)
         found_prey(ch, ch->hunting->who);
      else
      {
         CHAR_DATA *vch;

         /* perform a ranged attack if possible */
         /* Changed who to name as scan_for_victim expects the name and
            * Not the char struct. --Shaddai
          */
         if ((vch = scan_for_victim(ch, pexit, ch->hunting->name)) != NULL)
         {
            if (!mob_fire(ch, ch->hunting->who->name, pexit->vdir))
            {
               /* ranged spell attacks go here */
            }
         }
      }
      return;
   }
}

/* Below is used to hunt/track on Wilderness Map */
int find_dir(int cx, int cy, int vx, int vy)
{
   if (cx > vx && cy > vy)
      return 7;
   if (cx < vx && cy > vy)
      return 6;
   if (cx > vx && cy < vy)
      return 9;
   if (cx < vx && cy < vy)
      return 8;
   if (cx == vx && cy > vy)
      return 0;
   if (cx == vx && cy < vy)
      return 2;
   if (cx > vx && cy == vy)
      return 3;
   if (cx < vx && cy == vy)
      return 1;

   return -1;
}
int get_x(int currx, int dir)
{
   if (dir == 6 || dir == 1 || dir == 8)
      return currx + 1;
   if (dir == 7 || dir == 3 || dir == 9)
      return currx - 1;

   return currx;
}
int get_y(int curry, int dir)
{
   if (dir == 7 || dir == 0 || dir == 6)
      return curry - 1;
   if (dir == 9 || dir == 2 || dir == 8)
      return curry + 1;

   return curry;
}

int find_slot(dir)
{
   if (dir == 0)
      return 1;
   if (dir == 6)
      return 2;
   if (dir == 1)
      return 3;
   if (dir == 8)
      return 4;
   if (dir == 2)
      return 5;
   if (dir == 9)
      return 6;
   if (dir == 3)
      return 7;
   if (dir == 7)
      return 8;

   return 0;
}

int alt_dir(CHAR_DATA * ch, int x, int y, int dir)
{
   int gox;
   int goy;
   int godir;
   int newdir;
   int addto;
   int tries;

   const int spin[25] = {
      0, 0, 6, 1, 8, 2, 9, 3, 7, 0, 6, 1, 8, 2, 9, 3, 7, 0, 6, 1, 8, 2, 9, 3, 7
   };

   for (tries = 1; tries < 9; tries++)
   {
      addto = find_slot(dir);
      if (tries % 2 == 1)
         godir = ((tries + 1) / 2);
      else
         godir = (tries / 2);

      if (tries != 9)
         if (tries % 2 == 1)
            newdir = spin[8 + addto + godir];
         else
            newdir = spin[8 + addto - godir];
      else
         newdir = spin[8 + addto - 4];

      gox = get_x(x, newdir);
      goy = get_y(y, newdir);

      if (sect_show[(int)map_sector[ch->map][gox][goy]].canpass == FALSE
      || is_set_wilderness(ch, ROOM_NO_MOB, gox, goy, ch->map))
         continue;
      else
         return newdir;
   }
   return -1;
}

// 0 - Wilderness 1 -Battle
int go_find_victim(CHAR_DATA * ch, CHAR_DATA * victim, int type)
{
   sh_int dir;
   sh_int count;
   sh_int numcount;
   sh_int times;
   sh_int locount[10];
   sh_int lo = 1000;
   sh_int lonum = -1;
   sh_int initdir[10];
   sh_int currx, curry, gox, goy;

   if (ch->coord->x == victim->coord->x && ch->coord->y == victim->coord->y && ch->map == victim->map)
      return -2;
   if (ch->in_room->area != victim->in_room->area)
   {
      return -1;
   }

   numcount = 15;
   if (xIS_SET(ch->act, ACT_MILITARY))
      numcount = ch->m6 + 2;

   for (times = 0; times < 5; times++)
   {
      currx = ch->coord->x;
      curry = ch->coord->y;
      for (count = 1; count <= numcount; count++)
      {
         dir = find_dir(currx, curry, victim->coord->x, victim->coord->y);
         gox = get_x(currx, dir);
         goy = get_y(curry, dir);
         if (sect_show[(int)map_sector[ch->map][gox][goy]].canpass == FALSE || map_sector[ch->map][gox][goy] == SECT_EXIT
         || is_set_wilderness(ch, ROOM_NO_MOB, gox, goy, ch->map))
         {
            dir = alt_dir(ch, currx, curry, dir);
            gox = get_x(currx, dir);
            goy = get_y(curry, dir);
         }
         if (count == 1)
            initdir[times] = dir;
         currx = gox;
         curry = goy;
         if (currx == victim->coord->x && curry == victim->coord->y)
            break;

         if (count == numcount)
            count = 1000;
      }
      locount[times] = count;
   }
   for (times = 0; times < 5; times++)
   {
      if (locount[times] < lo)
      {
         lo = locount[times];
         lonum = times;
      }
   }
   if (lo >= 1000 || lonum == -1)
      return -1;
   else
      return initdir[lonum];
}

int get_distform args((int x, int y, int vx, int vy));
void hunt_victim_map(CHAR_DATA * ch)
{
   bool found;
   CHAR_DATA *tmp;
   CHAR_DATA *vch;
   OBJ_DATA *bow;
   sh_int ret, chance;
   sh_int maxx, maxy;
   sh_int x, y;

   if (!ch || !ch->hunting)
      return;

   x = ch->coord->x;
   y = ch->coord->y;
   maxx = MAX_X;
   maxy = MAX_Y;
   
   if (ch->fight_timer > 0)
      return;
   
   /* make sure the char still exists */
   for (found = FALSE, tmp = first_char; tmp && !found; tmp = tmp->next)
      if (ch->hunting->who == tmp)
         found = TRUE;

   if (!found)
   {
      do_say(ch, "Damn!  My prey is gone!!");
      stop_hunting(ch);
      find_next_hunt(ch, 0);
      return;
   }

    if (ch->coord->x == ch->hunting->who->coord->x && ch->coord->y == ch->hunting->who->coord->y && ch->map == ch->hunting->who->map)
    {
      if (ch->fighting)
         return;
      found_prey(ch, ch->hunting->who);
      return;
   }

   ret = go_find_victim(ch, ch->hunting->who, 0);

   if (ret < 0)
   {
      if (ret == -1)
      {
         do_say(ch, "Damn!  Lost my prey!");
         stop_hunting(ch);
         find_next_hunt(ch, 0);
         return;
      }
   }
   else
   {
      switch (ret)
      {
         case DIR_NORTH:
            y -= 1;
            break;
         case DIR_EAST:
            x += 1;
            break;
         case DIR_SOUTH:
            y += 1;
            break;
         case DIR_WEST:
            x -= 1;
            break;
         case DIR_NORTHEAST:
            x += 1;
            y -= 1;
            break;
         case DIR_NORTHWEST:
            x -= 1;
            y -= 1;
            break;
         case DIR_SOUTHEAST:
            x += 1;
            y += 1;
            break;
         case DIR_SOUTHWEST:
            x -= 1;
            y += 1;
            break;
      }
      switch (ret)
      {
         case DIR_NORTH:
            if (y == 0)
               return;
            break;

         case DIR_EAST:
            if (x == maxx + 1)
               return;
            break;

         case DIR_SOUTH:
            if (y == maxy + 1)
               return;
            break;

         case DIR_WEST:
            if (x == 0)
               return;
            break;

         case DIR_NORTHEAST:
            if (x == maxx + 1 || y == 0)
               return;
            break;

         case DIR_NORTHWEST:
            if (x == 0 || y == 0)
               return;
            break;

         case DIR_SOUTHEAST:
            if (x == maxx + 1 || y == maxy + 1)
               return;
            break;

         case DIR_SOUTHWEST:
            if (x == 0 || y == maxy + 1)
               return;
            break;
      }
      vch = ch->hunting->who;
      //Why would an archer chase down a target when it can fire at it?
      if (xIS_SET(ch->act, ACT_MILITARY)
         && (bow = get_eq_char(ch, WEAR_MISSILE_WIELD)) != NULL
         && get_distform(ch->coord->x, ch->coord->y, vch->coord->x, vch->coord->y) <= URANGE(1, bow->value[3], 10)
         && mob_fire(ch, ch->hunting->who->name, 1))
      {
         ;
      }
      else
      {
         chance = get_wilderness_hunt_cost(ch, x, y);
         if (chance <= 1 || number_range(1, chance) == 1)
         {
            if (!xIS_SET(ch->act, ACT_MILITARY) || (xIS_SET(ch->act, ACT_MILITARY) && !xIS_SET(ch->miflags, KM_SENTINEL)))
            { 
               if (IS_AWAKE(ch) && !IS_AFFECTED(ch, AFF_WEB))
               {
                  ch->coord->x = x;
                  ch->coord->y = y;
                  update_objects(ch, 0, 0, 0);
               }
            }
         }
      }
      if (!ch->hunting)
      {
         if (!ch->in_room)
         {
            bug("Hunt_victim: no ch->in_room!  Mob #%d, name: %s.  Placing mob in limbo.", ch->pIndexData->vnum, ch->name);
            char_to_room(ch, get_room_index(ROOM_VNUM_LIMBO));
            return;
         }
         do_say(ch, "Damn!  Lost my prey!");
         return;
      }
      if (ch->coord->x == ch->hunting->who->coord->x && ch->coord->y == ch->hunting->who->coord->y && ch->map == ch->hunting->who->map)
         found_prey(ch, ch->hunting->who);
      return;
   }
}
