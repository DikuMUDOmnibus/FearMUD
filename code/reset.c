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
 *			 Online Reset Editing Module			    *
 ****************************************************************************/

/*
 * This file relies heavily on the fact that your linked lists are correct,
 * and that pArea->reset_first is the first reset in pArea.  Likewise,
 * pArea->reset_last *MUST* be the last reset in pArea.  Weird and
 * wonderful things will happen if any of your lists are messed up, none
 * of them good.  The most important are your pRoom->contents,
 * pRoom->people, rch->carrying, obj->contains, and pArea->reset_first ..
 * pArea->reset_last.  -- Altrag
 */

#if defined(macintosh)
#include <types.h>
#else
#include <sys/types.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"


/* Externals */
extern int top_reset;
char *sprint_reset args((CHAR_DATA * ch, RESET_DATA * pReset, sh_int num, bool rlist));
RESET_DATA *parse_reset args((AREA_DATA * tarea, char *argument, CHAR_DATA * ch));
int get_wearloc args((char *type));
int get_trapflag args((char *flag));
int get_exflag args((char *flag));
int get_rflag args((char *flag));
extern char *const wear_locs[];
extern char *const ex_flags[];

#define MAX_OBJ 50
int put_index, obj_index;
int obj_array[MAX_OBJ][2];
int put_array[MAX_OBJ][3];

#define IS_OBJ_TYPE(obj) ( obj->item_type == ITEM_KEYRING \
                         ||obj->item_type == ITEM_QUIVER \
                         ||obj->item_type == ITEM_CONTAINER \
			 ||obj->item_type == ITEM_SHEATH)

#define RD RESET_DATA
RD *find_reset args((AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, int num));

#undef RD
void list_resets args((CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, int start, int end));

#define RID ROOM_INDEX_DATA
RID *find_room args((CHAR_DATA * ch, char *argument, ROOM_INDEX_DATA * pRoom));

#undef RID


RESET_DATA *find_reset(AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, int numb)
{
   RESET_DATA *pReset;
   int num = 0;

   for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
      if (is_room_reset(pReset, pRoom, pArea) && ++num >= numb)
         return pReset;
   return NULL;
}

/* Finds the mobs in the Reset Area and puts their limits on in acending order --Xerves*/
int fmob_count(AREA_DATA * pArea, int vnum)
{
   RESET_DATA *pReset;
   int count = 1;

   for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
   {
      if (pReset->arg1 == vnum && pReset->command == 'M')
      {
         pReset->arg2 = count;
         count++;
      }
   }
   return count;
}

/* This is one loopy function.  Ugh. -- Altrag */
bool is_room_reset(RESET_DATA * pReset, ROOM_INDEX_DATA * aRoom, AREA_DATA * pArea)
{
   ROOM_INDEX_DATA *pRoom;
   RESET_DATA *reset;
   int pr;

   if (!aRoom)
      return TRUE;
   switch (pReset->command)
   {
      case 'M':
      case 'O':
         pRoom = get_room_index(pReset->arg3);
         if (!pRoom || pRoom != aRoom)
            return FALSE;
         return TRUE;
      case 'P':
      case 'T':
      case 'H':
         if (pReset->command == 'H')
            pr = pReset->arg1;
         else
            pr = pReset->arg3;
         for (reset = pReset->prev; reset; reset = reset->prev)
            if ((reset->command == 'O' || reset->command == 'P' ||
                  reset->command == 'G' || reset->command == 'E') && (!pr || pr == reset->arg1) && get_obj_index(reset->arg1))
               break;
         if (reset && is_room_reset(reset, aRoom, pArea))
            return TRUE;
         return FALSE;
      case 'B':
         switch (pReset->arg2 & BIT_RESET_TYPE_MASK)
         {
            case BIT_RESET_DOOR:
            case BIT_RESET_ROOM:
               return (aRoom->vnum == pReset->arg1);
            case BIT_RESET_MOBILE:
               for (reset = pReset->prev; reset; reset = reset->prev)
                  if (reset->command == 'M' && get_mob_index(reset->arg1))
                     break;
               if (reset && is_room_reset(reset, aRoom, pArea))
                  return TRUE;
               return FALSE;
            case BIT_RESET_OBJECT:
               for (reset = pReset->prev; reset; reset = reset->prev)
                  if ((reset->command == 'O' || reset->command == 'P' ||
                        reset->command == 'G' || reset->command == 'E') &&
                     (!pReset->arg1 || pReset->arg1 == reset->arg1) && get_obj_index(reset->arg1))
                     break;
               if (reset && is_room_reset(reset, aRoom, pArea))
                  return TRUE;
               return FALSE;
         }
         return FALSE;
      case 'G':
      case 'E':
         for (reset = pReset->prev; reset; reset = reset->prev)
            if (reset->command == 'M' && get_mob_index(reset->arg1))
               break;
         if (reset && is_room_reset(reset, aRoom, pArea))
            return TRUE;
         return FALSE;
      case 'D':
      case 'R':
         pRoom = get_room_index(pReset->arg1);
         if (!pRoom || pRoom->area != pArea || (aRoom && pRoom != aRoom))
            return FALSE;
         return TRUE;
      default:
         return FALSE;
   }
   return FALSE;
}

ROOM_INDEX_DATA *find_room(CHAR_DATA * ch, char *argument, ROOM_INDEX_DATA * pRoom)
{
   char arg[MIL];

   if (pRoom)
      return pRoom;
   one_argument(argument, arg);
   if (!is_number(arg) && arg[0] != '\0')
   {
      send_to_char("Reset to which room?\n\r", ch);
      return NULL;
   }
   if (arg[0] == '\0')
      pRoom = ch->in_room;
   else
      pRoom = get_room_index(atoi(arg));
   if (!pRoom)
   {
      send_to_char("Room does not exist.\n\r", ch);
      return NULL;
   }
   return pRoom;
}

/* Separate function for recursive purposes */
#define DEL_RESET(area, reset, rprev) \
do { \
  rprev = reset->prev; \
  delete_reset(area, reset); \
  reset = rprev; \
  continue; \
} while(0)
void delete_reset(AREA_DATA * pArea, RESET_DATA * pReset)
{
   RESET_DATA *reset;
   RESET_DATA *reset_prev;

   if (pReset->command == 'M')
   {
      for (reset = pReset->next; reset; reset = reset->next)
      {
         /* Break when a new mob found */
         if (reset->command == 'M')
            break;
         /* Delete anything mob is holding */
         if (reset->command == 'G' || reset->command == 'E')
            DEL_RESET(pArea, reset, reset_prev);
         if (reset->command == 'B' && (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_MOBILE && (!reset->arg1 || reset->arg1 == pReset->arg1))
            DEL_RESET(pArea, reset, reset_prev);
      }
   }
   else if (pReset->command == 'O' || pReset->command == 'P' || pReset->command == 'G' || pReset->command == 'E')
   {
      for (reset = pReset->next; reset; reset = reset->next)
      {
         if (reset->command == 'T' && (!reset->arg3 || reset->arg3 == pReset->arg1))
            DEL_RESET(pArea, reset, reset_prev);
         if (reset->command == 'H' && (!reset->arg1 || reset->arg1 == pReset->arg1))
            DEL_RESET(pArea, reset, reset_prev);
         /* Delete nested objects, even if they are the same object. */
         if (reset->command == 'P' && (reset->arg3 > 0 ||
               pReset->command != 'P' || reset->extra - 1 == pReset->extra) && (!reset->arg3 || reset->arg3 == pReset->arg1))
            DEL_RESET(pArea, reset, reset_prev);
         if (reset->command == 'B' && (reset->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT && (!reset->arg1 || reset->arg1 == pReset->arg1))
            DEL_RESET(pArea, reset, reset_prev);
         if (reset->command == 'A')
            DEL_RESET(pArea, reset, reset_prev);
         /* Break when a new object of same type is found */
         if ((reset->command == 'O' || reset->command == 'P' || reset->command == 'G' || reset->command == 'E') && reset->arg1 == pReset->arg1)
            break;
      }
   }
   if (pReset == pArea->last_mob_reset)
      pArea->last_mob_reset = NULL;
   if (pReset == pArea->last_obj_reset)
      pArea->last_obj_reset = NULL;
   UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
   DISPOSE(pReset);
   return;
}

#undef DEL_RESET

RESET_DATA *find_oreset(CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, char *name)
{
   RESET_DATA *reset;

   if (!*name)
   {
      for (reset = pArea->last_reset; reset; reset = reset->prev)
      {
         if (!is_room_reset(reset, pRoom, pArea))
            continue;
         switch (reset->command)
         {
            default:
               continue;
            case 'O':
            case 'E':
            case 'G':
            case 'P':
               break;
         }
         break;
      }
      if (!reset)
         send_to_char("No object resets in list.\n\r", ch);
      return reset;
   }
   else
   {
      char arg[MIL];
      int cnt = 0, num = number_argument(name, arg);
      OBJ_INDEX_DATA *pObjTo = NULL;

      for (reset = pArea->first_reset; reset; reset = reset->next)
      {
         if (!is_room_reset(reset, pRoom, pArea))
            continue;
         switch (reset->command)
         {
            default:
               continue;
            case 'O':
            case 'E':
            case 'G':
            case 'P':
               break;
         }
         if ((pObjTo = get_obj_index(reset->arg1)) && (is_name(arg, pObjTo->name) || !str_cmp(arg, pObjTo->name)) && ++cnt == num)
            break;
      }
      if (!pObjTo || !reset)
      {
         send_to_char("To object not in reset list.\n\r", ch);
         return NULL;
      }
   }
   return reset;
}

RESET_DATA *find_mreset(CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, char *name)
{
   RESET_DATA *reset;

   if (!*name)
   {
      for (reset = pArea->last_reset; reset; reset = reset->prev)
      {
         if (!is_room_reset(reset, pRoom, pArea))
            continue;
         switch (reset->command)
         {
            default:
               continue;
            case 'M':
               break;
         }
         break;
      }
      if (!reset)
         send_to_char("No mobile resets in list.\n\r", ch);
      return reset;
   }
   else
   {
      char arg[MIL];
      int cnt = 0, num = number_argument(name, arg);
      MOB_INDEX_DATA *pMob = NULL;

      for (reset = pArea->first_reset; reset; reset = reset->next)
      {
         if (!is_room_reset(reset, pRoom, pArea))
            continue;
         switch (reset->command)
         {
            default:
               continue;
            case 'M':
               break;
         }
         if ((pMob = get_mob_index(reset->arg1)) && is_name(arg, pMob->player_name) && ++cnt == num)
            break;
      }
      if (!pMob || !reset)
      {
         send_to_char("Mobile not in reset list.\n\r", ch);
         return NULL;
      }
   }
   return reset;
}

bool can_mapmodify(CHAR_DATA * ch, ROOM_INDEX_DATA * room)
{
   sh_int vnum = room->vnum;
   AREA_DATA *pArea;

   if (IS_NPC(ch))
      return FALSE;

   if (get_trust(ch) >= sysdata.level_modify_proto)
      return TRUE;
   if (!xIS_SET(room->room_flags, ROOM_PROTOTYPE))
   {
      send_to_char("You cannot modify this room.\n\r", ch);
      return FALSE;
   }
   if (!ch->pcdata || !(pArea = ch->pcdata->area))
   {
      send_to_char("You must have an assigned area to modify this room.\n\r", ch);
      return FALSE;
   }
   if (vnum >= pArea->low_r_vnum && vnum <= pArea->hi_r_vnum)
      return TRUE;

   send_to_char("That room is not in your allocated range.\n\r", ch);
   return FALSE;
}

void edit_reset(CHAR_DATA * ch, char *argument, AREA_DATA * pArea, ROOM_INDEX_DATA * aRoom)
{
   char arg[MIL];
   RESET_DATA *pReset = NULL;
   RESET_DATA *reset = NULL;
   MOB_INDEX_DATA *pMob = NULL;
   ROOM_INDEX_DATA *pRoom;
   OBJ_INDEX_DATA *pObj;
   int num = 0;
   int vnum;
   char *origarg = argument;

   argument = one_argument(argument, arg);
   if (!*arg || !str_cmp(arg, "?"))
   {
      char *nm = (ch->substate == SUB_REPEATCMD ? "" : (aRoom ? "rreset " : "reset "));
      char *rn = (aRoom ? "" : " [room#]");

      ch_printf(ch, "Syntax: %s<list|edit|delete|add|insert|place%s>\n\r", nm, (aRoom ? "" : "|area"));
      ch_printf(ch, "Syntax: %sremove <#>\n\r", nm);
      ch_printf(ch, "Syntax: %smobile <mob#> [limit]%s\n\r", nm, rn);
      ch_printf(ch, "Syntax: %sobject <obj#> [limit [room%s]]\n\r", nm, rn);
      ch_printf(ch, "Syntax: %sobject <obj#> give <mob name> [limit]\n\r", nm);
      ch_printf(ch, "Syntax: %sobject <obj#> equip <mob name> <location> " "[limit]\n\r", nm);
      ch_printf(ch, "Syntax: %sobject <obj#> put <to_obj name> [limit]\n\r", nm);
      ch_printf(ch, "Syntax: %stime <#> <time to pop>\n\r", nm);
      ch_printf(ch, "Syntax: %shide <obj name>\n\r", nm);
      ch_printf(ch, "Syntax: %strap <obj name> <type> <charges> <flags>\n\r", nm);
      ch_printf(ch, "Syntax: %strap room <type> <charges> <flags>\n\r", nm);
      ch_printf(ch, "Syntax: %sbit <set|toggle|remove> door%s <dir> " "<exit flags>\n\r", nm, rn);
      ch_printf(ch, "Syntax: %sbit <set|toggle|remove> object <obj name> " "<extra flags>\n\r", nm);
      ch_printf(ch, "Syntax: %sbit <set|toggle|remove> mobile <mob name> " "<affect flags>\n\r", nm);
      ch_printf(ch, "Syntax: %sbit <set|toggle|remove> room%s <room flags>" "\n\r", nm, rn);
      ch_printf(ch, "Syntax: %srandom <last dir>%s\n\r", nm, rn);
      if (!aRoom)
      {
         send_to_char("\n\r[room#] will default to the room you are in, " "if unspecified.\n\r", ch);
      }
      return;
   }
   if (!str_cmp(arg, "on"))
   {
      ch->substate = SUB_REPEATCMD;
      ch->dest_buf = (aRoom ? (void *) aRoom : (void *) pArea);
      send_to_char("Reset mode on.\n\r", ch);
      return;
   }
   if (!aRoom && !str_cmp(arg, "area"))
   {
      if (!pArea->first_reset)
      {
         send_to_char("You don't have any resets defined.\n\r", ch);
         return;
      }
      num = pArea->nplayer;
      pArea->nplayer = 0;
      reset_area(pArea, 0);
      pArea->nplayer = num;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "list"))
   {
      int start, end;

      argument = one_argument(argument, arg);
      start = is_number(arg) ? atoi(arg) : -1;
      argument = one_argument(argument, arg);
      end = is_number(arg) ? atoi(arg) : -1;
      list_resets(ch, pArea, aRoom, start, end);
      return;
   }

   if (!str_cmp(arg, "edit"))
   {
      argument = one_argument(argument, arg);
      if (!*arg || !is_number(arg))
      {
         send_to_char("Usage: reset edit <number> <command>\n\r", ch);
         return;
      }
      num = atoi(arg);
      if (!(pReset = find_reset(pArea, aRoom, num)))
      {
         send_to_char("Reset not found.\n\r", ch);
         return;
      }
      if ((reset = parse_reset(pArea, argument, ch)) == NULL)
      {
         send_to_char("Error in reset.  Reset not changed.\n\r", ch);
         return;
      }
      reset->serial = pReset->serial;
      reset->prev = pReset->prev;
      reset->next = pReset->next;
      if (!pReset->prev)
         pArea->first_reset = reset;
      else
         pReset->prev->next = reset;
      if (!pReset->next)
         pArea->last_reset = reset;
      else
         pReset->next->prev = reset;
      DISPOSE(pReset);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "add"))
   {
      if ((pReset = parse_reset(pArea, argument, ch)) == NULL)
      {
         send_to_char("Error in reset.  Reset not added.\n\r", ch);
         return;
      }  
      reset = add_reset(pArea, pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4, pReset->arg5, pReset->arg6, -1, 0, 0);
      reset->serial = ++serialmobsloaded;
      serial_list[reset->serial] = FALSE; 
      DISPOSE(pReset);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "place"))
   {
      if ((pReset = parse_reset(pArea, argument, ch)) == NULL)
      {
         send_to_char("Error in reset.  Reset not added.\n\r", ch);
         return;
      }
      pReset->serial = ++serialmobsloaded;
      serial_list[pReset->serial] = FALSE;  
      reset = place_reset(pArea, pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4, pReset->arg5, pReset->arg6, -1, 0, 0);
      reset->serial = ++serialmobsloaded;
      serial_list[reset->serial] = FALSE; 
      DISPOSE(pReset);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "insert"))
   {
      argument = one_argument(argument, arg);
      if (!*arg || !is_number(arg))
      {
         send_to_char("Usage: reset insert <number> <command>\n\r", ch);
         return;
      }
      num = atoi(arg);
      if ((reset = find_reset(pArea, aRoom, num)) == NULL)
      {
         send_to_char("Reset not found.\n\r", ch);
         return;
      }
      if ((pReset = parse_reset(pArea, argument, ch)) == NULL)
      {
         send_to_char("Error in reset.  Reset not inserted.\n\r", ch);
         return;
      }
      pReset->serial = ++serialmobsloaded;
      serial_list[pReset->serial] = FALSE;  
      INSERT(pReset, reset, pArea->first_reset, next, prev);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "delete"))
   {
      int start, end;
      bool found;

      if (!*argument)
      {
         send_to_char("Usage: reset delete <start> [end]\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg);
      start = is_number(arg) ? atoi(arg) : -1;
      end = is_number(arg) ? atoi(arg) : -1;
      num = 0;
      found = FALSE;
      for (pReset = pArea->first_reset; pReset; pReset = reset)
      {
         reset = pReset->next;
         if (!is_room_reset(pReset, aRoom, pArea))
            continue;
         if (start > ++num)
            continue;
         if ((end != -1 && num > end) || (end == -1 && found))
            return;
         UNLINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
         if (pReset == pArea->last_mob_reset)
            pArea->last_mob_reset = NULL;
         DISPOSE(pReset);
         top_reset--;
         found = TRUE;
      }
      if (!found)
         send_to_char("Reset not found.\n\r", ch);
      else
         send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "remove"))
   {
      int iarg;

      argument = one_argument(argument, arg);
      if (arg[0] == '\0' || !is_number(arg))
      {
         send_to_char("Delete which reset?\n\r", ch);
         return;
      }
      iarg = atoi(arg);
      for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
      {
         if (is_room_reset(pReset, aRoom, pArea) && ++num == iarg)
            break;
      }
      if (!pReset)
      {
         send_to_char("Reset does not exist.\n\r", ch);
         return;
      }
      delete_reset(pArea, pReset);
      send_to_char("Reset deleted.\n\r", ch);
      return;
   }
   if (!str_prefix(arg, "time"))
   {   
      int tleft;
      int ttime = time(0);
      int sec, hour, min, days, rtime;
      char time[256];
      
      argument = one_argument(argument, arg);
      if (!*arg || (!is_number(arg) && str_cmp(arg, "left") && str_cmp(arg, "forcereset")))
      {
         send_to_char("Usage: reset time <number> <time to reset>\n\r", ch);
         send_to_char("Usage: reset time left <number>  --Shows Time Left till Reset\n\r", ch);
         send_to_char("Usage: reset time forcereset <number>  --Sets the time to reset to now, will reset next reset area tick\n\r", ch);
         send_to_char("Time to reset is in seconds.  60 seconds in a minute, 3600 seconds in an hour, 86,400 seconds in a day\n\r", ch);
         return;
      }
      if (!str_cmp(arg, "forcereset"))
      {
         num = atoi(argument);
         if (!(pReset = find_reset(pArea, aRoom, num)))
         {
            send_to_char("Reset not found.\n\r", ch);
            return;
         }
         if (pReset->resetlast < 1)
         {
            send_to_char("That reset is not a time based Reset.\n\r", ch);
            return;
         }
         pReset->resetlast = ttime-pReset->resettime;
         send_to_char("Done.\n\r", ch);
         return;
      }
      if (!str_cmp(arg, "left"))
      {
         num = atoi(argument);
         if (!(pReset = find_reset(pArea, aRoom, num)))
         {
            send_to_char("Reset not found.\n\r", ch);
            return;
         }
         if (pReset->resetlast < 1)
         {
            send_to_char("That reset is not a time based Reset.\n\r", ch);
            return;
         }
         tleft = pReset->resetlast + pReset->resettime - ttime;
         if (tleft < 1)
         {
            send_to_char("The Reset will load next time a Reset Area is issued.\n\r", ch);
            return;
         }
         if (tleft > 86400)
         {
            days = pReset->resettime / 86400;
            rtime = pReset->resettime % 86400;
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
         sprintf(time, "&w&WTime Left:  D:%d H:%d M:%d S:%d\n\r", days, hour, min, sec);
         send_to_char(time, ch);
         return;
      }  
      num = atoi(arg);
      if (!(pReset = find_reset(pArea, aRoom, num)))
      {
         send_to_char("Reset not found.\n\r", ch);
         return;
      }
      num = atoi(argument);
      if (num < -1 || num > 300000000)
      {
         send_to_char("Range is -1 (Resets once), 0 (Resets every tick) to 300mil (resets every 10 years or so)\n\r", ch);
         return;
      }
      pReset->resettime = num;
      pReset->resetlast = 0;
      ch_printf(ch, "Time of %d added.\n\r", num);
      return;
   }
   if (!str_prefix(arg, "mobile"))
   {
      argument = one_argument(argument, arg);
      if (arg[0] == '\0' || !is_number(arg))
      {
         send_to_char("Reset which mobile vnum?\n\r", ch);
         return;
      }
      if (!(pMob = get_mob_index(atoi(arg))))
      {
         send_to_char("Mobile does not exist.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg);
      if (arg[0] == '\0')
         num = 1;
      else if (!is_number(arg))
      {
         send_to_char("Reset how many mobiles?\n\r", ch);
         return;
      }
      else
         num = atoi(arg);
      if (!(pRoom = find_room(ch, argument, aRoom)))
         return;
      pReset = make_reset('M', 0, pMob->vnum, 0, pRoom->vnum, -1, -1, -1, -1, 0, 0);
      pReset->serial = ++serialmobsloaded;
      pReset->arg2 = 1;
      serial_list[pReset->serial] = FALSE;
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      send_to_char("Mobile reset added.\n\r", ch);
      return;
   }
   if (!str_prefix(arg, "object"))
   {
      argument = one_argument(argument, arg);
      if (arg[0] == '\0' || !is_number(arg))
      {
         send_to_char("Reset which object vnum?\n\r", ch);
         return;
      }
      if (!(pObj = get_obj_index(atoi(arg))))
      {
         send_to_char("Object does not exist.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg);
      if (arg[0] == '\0')
         strcpy(arg, "room");
      if (!str_prefix(arg, "put"))
      {
         argument = one_argument(argument, arg);
         if (!(reset = find_oreset(ch, pArea, aRoom, arg)))
            return;
         pReset = reset;
         /* Put in_objects after hide and trap resets */
         while (reset->next && (reset->next->command == 'H' ||
               reset->next->command == 'T' ||
               (reset->next->command == 'B' &&
(reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT && (!reset->next->arg1 || reset->next->arg1 == pReset->arg1))))
            reset = reset->next;
/*      pReset = make_reset('P', 1, pObj->vnum, num, reset->arg1);*/
         argument = one_argument(argument, arg);
         if ((vnum = atoi(arg)) < 1)
            vnum = 1;
         pReset = make_reset('P', reset->extra + 1, pObj->vnum, vnum, 0, -1, -1, -1, -1, 0, 0);
         /* Grumble.. insert puts pReset before reset, and we need it after,
            so we make a hackup and reverse all the list params.. :P.. */
         INSERT(pReset, reset, pArea->last_reset, prev, next);
         send_to_char("Object reset in object created.\n\r", ch);
         return;
      }
      if (!str_prefix(arg, "give"))
      {
         argument = one_argument(argument, arg);
         if (!(reset = find_mreset(ch, pArea, aRoom, arg)))
            return;
         pReset = reset;
         while (reset->next && reset->next->command == 'B' &&
            (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT && (!reset->next->arg1 || reset->next->arg1 == pReset->arg1))
            reset = reset->next;
         argument = one_argument(argument, arg);
         if ((vnum = atoi(arg)) < 1)
            vnum = 1;
         pReset = make_reset('G', 1, pObj->vnum, vnum, 0, -1, -1, -1, -1, 0, 0);
         INSERT(pReset, reset, pArea->last_reset, prev, next);
         send_to_char("Object reset to mobile created.\n\r", ch);
         return;
      }
      if (!str_prefix(arg, "equip"))
      {
         argument = one_argument(argument, arg);
         if (!(reset = find_mreset(ch, pArea, aRoom, arg)))
            return;
         pReset = reset;
         while (reset->next && reset->next->command == 'B' &&
            (reset->next->arg2 & BIT_RESET_TYPE_MASK) == BIT_RESET_OBJECT && (!reset->next->arg1 || reset->next->arg1 == pReset->arg1))
            reset = reset->next;
         num = get_wearloc(argument);
         if (num < 0)
         {
            send_to_char("Reset object to which location?\n\r", ch);
            return;
         }
         for (pReset = reset->next; pReset; pReset = pReset->next)
         {
            if (pReset->command == 'M')
               break;
            if (pReset->command == 'E' && pReset->arg3 == num)
            {
               send_to_char("Mobile already has an item equipped there.\n\r", ch);
               return;
            }
         }
         argument = one_argument(argument, arg);
         if ((vnum = atoi(arg)) < 1)
            vnum = 1;
         pReset = make_reset('E', 1, pObj->vnum, vnum, num, -1, -1, -1, -1, 0, 0);
         INSERT(pReset, reset, pArea->last_reset, prev, next);
         send_to_char("Object reset equipped by mobile created.\n\r", ch);
         return;
      }
      if (arg[0] == '\0' || !(num = (int) str_cmp(arg, "room")) || is_number(arg))
      {
         if (!(bool) num)
            argument = one_argument(argument, arg);
         if (!(pRoom = find_room(ch, argument, aRoom)))
            return;
         if (pRoom->area != pArea)
         {
            send_to_char("Cannot reset objects to other areas.\n\r", ch);
            return;
         }
         if ((vnum = atoi(arg)) < 1)
            vnum = 1;
         pReset = make_reset('O', 0, pObj->vnum, vnum, pRoom->vnum, -1, -1, -1, -1, 0, 0);
         LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
         send_to_char("Object reset added.\n\r", ch);
         return;
      }
      send_to_char("Reset object to where?\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "random"))
   {
      argument = one_argument(argument, arg);
      vnum = get_dir(arg);
      if (vnum < 0 || vnum > 9)
      {
         send_to_char("Reset which random doors?\n\r", ch);
         return;
      }
      if (vnum == 0)
      {
         send_to_char("There is no point in randomizing one door.\n\r", ch);
         return;
      }
      pRoom = find_room(ch, argument, aRoom);
      if (pRoom->area != pArea)
      {
         send_to_char("Cannot randomize doors in other areas.\n\r", ch);
         return;
      }
      pReset = make_reset('R', 0, pRoom->vnum, vnum, 0, -1, -1, -1, -1, 0, 0);
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      send_to_char("Reset random doors created.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "trap"))
   {
      char oname[MIL];
      int chrg, value, extra = 0;
      bool isobj;

      argument = one_argument(argument, oname);
      argument = one_argument(argument, arg);
      num = is_number(arg) ? atoi(arg) : -1;
      argument = one_argument(argument, arg);
      chrg = is_number(arg) ? atoi(arg) : -1;
      isobj = is_name("obj", argument);
      if (isobj == is_name("room", argument))
      {
         send_to_char("Reset: TRAP: Must specify ROOM or OBJECT\n\r", ch);
         return;
      }
      if (!str_cmp(oname, "room") && !isobj)
      {
         vnum = (aRoom ? aRoom->vnum : ch->in_room->vnum);
         extra = TRAP_ROOM;
      }
      else
      {
         if (is_number(oname) && !isobj)
         {
            vnum = atoi(oname);
            if (!get_room_index(vnum))
            {
               send_to_char("Reset: TRAP: no such room\n\r", ch);
               return;
            }
            reset = NULL;
            extra = TRAP_ROOM;
         }
         else
         {
            if (!(reset = find_oreset(ch, pArea, aRoom, oname)))
               return;
/*        vnum = reset->arg1;*/
            vnum = 0;
            extra = TRAP_OBJ;
         }
      }
      if (num < 1 || num > MAX_TRAPTYPE)
      {
         send_to_char("Reset: TRAP: invalid trap type\n\r", ch);
         return;
      }
      if (chrg < 0 || chrg > 10000)
      {
         send_to_char("Reset: TRAP: invalid trap charges\n\r", ch);
         return;
      }
      while (*argument)
      {
         argument = one_argument(argument, arg);
         value = get_trapflag(arg);
         if (value < 0 || value > 31)
         {
            send_to_char("Reset: TRAP: bad flag\n\r", ch);
            return;
         }
         SET_BIT(extra, 1 << value);
      }
      pReset = make_reset('T', extra, num, chrg, vnum, -1, -1, -1, -1, 0, 0);
      if (reset)
         INSERT(pReset, reset, pArea->last_reset, prev, next);
      else
         LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      send_to_char("Trap created.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "bit"))
   {
      int (*flfunc) (char *type);
      int flags = 0;
      char option[MIL];
      char *parg;
      bool ext_bv = FALSE;

      argument = one_argument(argument, option);
      if (!*option)
      {
         send_to_char("You must specify SET, REMOVE, or TOGGLE.\n\r", ch);
         return;
      }
      num = 0;
      if (!str_prefix(option, "set"))
         SET_BIT(num, BIT_RESET_SET);
      else if (!str_prefix(option, "toggle"))
         SET_BIT(num, BIT_RESET_TOGGLE);
      else if (str_prefix(option, "remove"))
      {
         send_to_char("You must specify SET, REMOVE, or TOGGLE.\n\r", ch);
         return;
      }
      argument = one_argument(argument, option);
      parg = argument;
      argument = one_argument(argument, arg);
      if (!*option)
      {
         send_to_char("Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r", ch);
         return;
      }
      if (!str_prefix(option, "door"))
      {
         SET_BIT(num, BIT_RESET_DOOR);
         if (aRoom)
         {
            pRoom = aRoom;
            argument = parg;
         }
         else if (!is_number(arg))
         {
            pRoom = ch->in_room;
            argument = parg;
         }
         else if (!(pRoom = find_room(ch, arg, aRoom)))
            return;
         argument = one_argument(argument, arg);
         if (!*arg)
         {
            send_to_char("Must specify direction.\n\r", ch);
            return;
         }
         vnum = get_dir(arg);
         SET_BIT(num, vnum << BIT_RESET_DOOR_THRESHOLD);
         vnum = pRoom->vnum;
         flfunc = &get_exflag;
         reset = NULL;
      }
      else if (!str_prefix(option, "object"))
      {
         SET_BIT(num, BIT_RESET_OBJECT);
         vnum = 0;
         flfunc = &get_oflag;
         if (!(reset = find_oreset(ch, pArea, aRoom, arg)))
            return;
         ext_bv = TRUE;
      }
      else if (!str_prefix(option, "mobile"))
      {
         SET_BIT(num, BIT_RESET_MOBILE);
         vnum = 0;
         flfunc = &get_aflag;
         if (!(reset = find_mreset(ch, pArea, aRoom, arg)))
            return;
         ext_bv = TRUE;
      }
      else if (!str_prefix(option, "room"))
      {
         SET_BIT(num, BIT_RESET_ROOM);
         if (aRoom)
         {
            pRoom = aRoom;
            argument = parg;
         }
         else if (!is_number(arg))
         {
            pRoom = ch->in_room;
            argument = parg;
         }
         else if (!(pRoom = find_room(ch, arg, aRoom)))
            return;
         vnum = pRoom->vnum;
         flfunc = &get_rflag;
         reset = NULL;
      }
      else
      {
         send_to_char("Must specify OBJECT, MOBILE, ROOM, or DOOR.\n\r", ch);
         return;
      }
      while (*argument)
      {
         int value;

         argument = one_argument(argument, arg);
         value = (*flfunc) (arg);
         if (value < 0 || (!ext_bv && value > 31))
         {
            send_to_char("Reset: BIT: bad flag\n\r", ch);
            return;
         }
         if (ext_bv) /* one per flag for extendeds */
         {
            pReset = make_reset('B', 1, vnum, num, flags, -1, -1, -1, -1, 0, 0);
            if (reset)
            {
               INSERT(pReset, reset, pArea->last_reset, prev, next);
               reset = pReset;
            }
            else
               LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
         }
         else
            SET_BIT(flags, 1 << value);
      }
      if (!flags)
      {
         send_to_char("Set which flags?\n\r", ch);
         return;
      }
      if (!ext_bv)
      {
         pReset = make_reset('B', 1, vnum, num, flags, -1, -1, -1, -1, 0, 0);
         if (reset)
            INSERT(pReset, reset, pArea->last_reset, prev, next);
         else
            LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      }
      send_to_char("Bitvector reset created.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "hide"))
   {
      argument = one_argument(argument, arg);
      if (!(reset = find_oreset(ch, pArea, aRoom, arg)))
         return;
/*    pReset = make_reset('H', 1, reset->arg1, 0, 0);*/
      pReset = make_reset('H', 1, 0, 0, 0, -1, -1, -1, -1, 0, 0);
      INSERT(pReset, reset, pArea->last_reset, prev, next);
      send_to_char("Object hide reset created.\n\r", ch);
      return;
   }
   if (ch->substate == SUB_REPEATCMD)
   {
      ch->substate = SUB_NONE;
      interpret(ch, origarg);
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = (aRoom ? do_rreset : do_reset);
   }
   else
      edit_reset(ch, "", pArea, aRoom);
   return;
}

void do_reset(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *pArea = NULL;
   char arg[MIL];
   char *parg;

   parg = one_argument(argument, arg);

   /* Mset/Oset/Redit On Mode check -- Stop most building crashes -- Xerves 8/7/99 */
   if ((xIS_SET(ch->act, PLR_MSET)) || (xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_REDIT)))
   {
      send_to_char("You need to turn mset/oset/redit off\n\r", ch);
      return;
   }

   if (ch->substate == SUB_REPEATCMD)
   {
      pArea = ch->dest_buf;
      if (pArea && pArea != ch->pcdata->area && pArea != ch->in_room->area)
      {
         AREA_DATA *tmp;

         for (tmp = first_build; tmp; tmp = tmp->next)
            if (tmp == pArea)
               break;
         if (!tmp)
            for (tmp = first_area; tmp; tmp = tmp->next)
               if (tmp == pArea)
                  break;
         if (!tmp)
         {
            send_to_char("Your area pointer got lost.  Reset mode off.\n\r", ch);
            bug("do_reset: %s's dest_buf points to invalid area", ch->name); /* why was this cast to an int? */
            ch->substate = SUB_NONE;
            ch->dest_buf = NULL;
            return;
         }
      }
      if (!*arg)
      {
         ch_printf(ch, "Editing resets for area: %s\n\r", pArea->name);
         return;
      }
      if (!str_cmp(arg, "done") || !str_cmp(arg, "off"))
      {
         send_to_char("Reset mode off.\n\r", ch);
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
         return;
      }
   }
   if (!pArea && get_trust(ch) >= LEVEL_STAFF) /* Tracker1 */
   {
      char fname[80];

      sprintf(fname, "%s.are", capitalize(arg));
      for (pArea = first_build; pArea; pArea = pArea->next)
         if (!str_cmp(fname, pArea->filename))
         {
            argument = parg;
            break;
         }
      if (!pArea)
         pArea = ch->pcdata->area;
      if (!pArea)
         pArea = ch->in_room->area;
   }
   else
      pArea = ch->pcdata->area;
   if (!pArea)
   {
      send_to_char("You do not have an assigned area.\n\r", ch);
      return;
   }
   edit_reset(ch, argument, pArea, NULL);
   return;
}

void do_rreset(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *pRoom;

   if (ch->substate == SUB_REPEATCMD)
   {
      pRoom = ch->dest_buf;
      if (!pRoom)
      {
         send_to_char("Your room pointer got lost.  Reset mode off.\n\r", ch);
         bug("do_rreset: %s's dest_buf points to invalid room", (int) ch->name);
      }
      ch->substate = SUB_NONE;
      ch->dest_buf = NULL;
      return;
   }
   else
      pRoom = ch->in_room;
   if (!can_rmodify(ch, pRoom))
      return;
   edit_reset(ch, argument, pRoom->area, pRoom);
   return;
}

void add_obj_reset(AREA_DATA * pArea, char cm, OBJ_DATA * obj, int v2, int v3)
{
   OBJ_DATA *inobj;
   RESET_DATA *pReset;
   static int iNest;
   int obj_loop, value_loop;
   int reset_count = 0;
   bool found;
   int v4 = -1;
   int v7 = -1;
   
   if (obj->value[6] > 99 && (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_ARMOR) && xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
   {
      if (cm == '0')   
         v7 = obj->value[6];
      else if (cm == 'E' || cm == 'P')
         v4 = obj->value[6];
      else if (cm == 'G')
      {
         v3 = obj->value[6];
         v4 = obj->value[11];
      }
   }

   if ((cm == 'O' || cm == 'P') && obj->pIndexData->vnum == OBJ_VNUM_TRAP)
   {
      if (cm == 'O')
         add_reset(pArea, 'T', obj->value[3], obj->value[1], obj->value[0], v3, v4, -1, -1, v7, 0, 0);
      return;
   }
   if (cm == 'O' && IS_OBJ_STAT(obj, ITEM_ONMAP))
   {
      add_reset(pArea, cm, 1, obj->pIndexData->vnum, v2, v3, obj->coord->x, obj->coord->y, obj->map, v7, 0, 0);
   }
   else
   {
      add_reset(pArea, cm, (cm == 'P' ? iNest : 1), obj->pIndexData->vnum, v2, v3, v4, -1, -1, v7, 0, 0);
   }
   /* Only add hide for in-room objects that are hidden and cant be moved, as
      hide is an update reset, not a load-only reset. */
   if (cm == 'O' && IS_OBJ_STAT(obj, ITEM_HIDDEN) && !IS_SET(obj->wear_flags, ITEM_TAKE))
      add_reset(pArea, 'H', 1, 0, 0, 0, -1, -1, -1, -1, 0, 0);
   if (obj->trap)
   {
      pReset = make_reset('A', 1, obj->trap->uid, 0, 0, -1, -1, -1, -1, 0, 0);
      LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
   }
   for (inobj = obj->first_content; inobj; inobj = inobj->next_content)
      if (inobj->pIndexData->vnum == OBJ_VNUM_TRAP)
         add_obj_reset(pArea, 'O', inobj, 0, 0);
   if (cm == 'P')
      iNest++;

   if (put_index == 0)
      for (obj_loop = 0; obj_loop < MAX_OBJ; obj_loop++)
         for (value_loop = 0; value_loop < 3; value_loop++)
            put_array[obj_loop][value_loop] = 0;

   for (inobj = obj->first_content; inobj; inobj = inobj->next_content)
   {
      if (IS_OBJ_TYPE(inobj))
      {
         found = FALSE;
         for (obj_loop = 0; obj_loop <= obj_index; obj_loop++)
         {
            if ((put_array[obj_loop][0] == inobj->pIndexData->vnum) && (put_array[obj_loop][2] == iNest))
            {
               reset_count = inobj->count + put_array[obj_loop][1];
               put_array[obj_loop][1] = reset_count;
               found = TRUE;
            }
         }
         if (!found)
         {
            reset_count = inobj->count;
            put_array[put_index][0] = inobj->pIndexData->vnum;
            put_array[put_index][1] = inobj->count;
            put_array[put_index][2] = iNest;
            put_index++;
         }
      }
      else
         reset_count = count_obj_list(get_obj_index(inobj->pIndexData->vnum), obj->first_content);

      add_obj_reset(pArea, 'P', inobj, reset_count, 0);
      if (inobj->trap)
      {
         pReset = make_reset('A', 1, obj->trap->uid, 0, 0, -1, -1, -1, -1, 0, 0);
         LINK(pReset, pArea->first_reset, pArea->last_reset, next, prev);
      }
   }
   if (cm == 'P')
      iNest--;
   return;
}

int get_reset_equiped(OBJ_DATA *obj, OBJ_DATA *fobj)
{
   int count = 0;
   
   for (; fobj; fobj = fobj->next_content)
   if (fobj->pIndexData->vnum == obj->pIndexData->vnum && fobj->wear_loc != WEAR_NONE && obj->wear_loc != WEAR_NONE)
   {
      count++;
      if (fobj->wear_loc == obj->wear_loc)
         return count;
   }
      
   return 1;
}    

int count_obj_list_inv(OBJ_INDEX_DATA * pObjIndex, OBJ_DATA * list)
{
   OBJ_DATA *obj;
   int nMatch = 0;

   for (obj = list; obj; obj = obj->next_content)
   {
      if (obj->pIndexData == pObjIndex && obj->wear_loc == WEAR_NONE)
      {
         if (obj->count > 1)
            nMatch += obj->count;
         else
            nMatch++;
      }
   }

   return nMatch;
}

void instaroom(CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, bool dodoors, bool allmap)
{
   CHAR_DATA *rch;
   OBJ_DATA *obj;
   RESET_DATA *reset;
   int obj_loop, value_loop, reset_count;
   int count;
   int x;
   bool found;

   for (rch = pRoom->first_person; rch; rch = rch->next_in_room)
   {
      if (!IS_NPC(rch))
         continue;

      if (pRoom->vnum == OVERLAND_SOLAN)
         if (allmap == FALSE)
            if ((rch->coord->x != ch->coord->x) || (rch->coord->y != ch->coord->y) || (rch->map != ch->map))
               continue;
      /* Updates the Mob count according to the current mob list, will Make sure mobs go up in
         ascending order..1, 2, 3, 4, 5, etc -- Xerves */

      reset_count = fmob_count(pArea, rch->pIndexData->vnum);
      reset = add_reset(pArea, 'M', 1, rch->pIndexData->vnum, 1, pRoom->vnum, rch->coord->x, rch->coord->y, rch->map, -1, 0, 0);
      reset->serial = rch->serial;
      serial_list[reset->serial] = TRUE;


      obj_index = 0;
      reset_count = 0;
      for (obj_loop = 0; obj_loop < MAX_OBJ; obj_loop++)
         for (value_loop = 0; value_loop < 2; value_loop++)
            obj_array[obj_loop][value_loop] = 0;

      for (obj = rch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->wear_loc == WEAR_NONE)
         {
            if (IS_OBJ_TYPE(obj))
            {
               found = FALSE;
               for (obj_loop = 0; obj_loop <= obj_index; obj_loop++)
               {
                  if (obj_array[obj_loop][0] == obj->pIndexData->vnum)
                  {
                     reset_count = obj->count + obj_array[obj_loop][1];
                     obj_array[obj_loop][1] = reset_count;
                     found = TRUE;
                  }
               }
               if (!found)
               {
                  reset_count = obj->count;
                  obj_array[obj_index][0] = obj->pIndexData->vnum;
                  obj_array[obj_index][1] = obj->count;
                  obj_index++;
               }
            }
            else
               reset_count = count_obj_list_inv(get_obj_index(obj->pIndexData->vnum), rch->first_carrying);
            put_index = 0;
            count = obj->count;
            for (x = 1; x <= count; x++)
               add_obj_reset(pArea, 'G', obj, reset_count, 0);
         }
         else
         {
            reset_count = get_reset_equiped(obj, rch->first_carrying);
            add_obj_reset(pArea, 'E', obj, reset_count, obj->wear_loc);
         }
      }
   }

   obj_index = 0;
   reset_count = 0;
   for (obj_loop = 0; obj_loop < MAX_OBJ; obj_loop++)
      for (value_loop = 0; value_loop < 2; value_loop++)
         obj_array[obj_loop][value_loop] = 0;

   for (obj = pRoom->first_content; obj; obj = obj->next_content)
   {
      if (IS_OBJ_TYPE(obj))
      {
         found = FALSE;
         for (obj_loop = 0; obj_loop <= obj_index; obj_loop++)
         {
            if (obj_array[obj_loop][0] == obj->pIndexData->vnum)
            {
               reset_count = obj->count + obj_array[obj_loop][1];
               obj_array[obj_loop][1] = reset_count;
               found = TRUE;
            }
         }
         if (!found)
         {
            reset_count = obj->count;
            obj_array[obj_index][0] = obj->pIndexData->vnum;
            obj_array[obj_index][1] = obj->count;
            obj_index++;
         }
      }
      else
         reset_count = count_obj_list(get_obj_index(obj->pIndexData->vnum), pRoom->first_content);

      if (pRoom->vnum == OVERLAND_SOLAN)
         if (allmap == FALSE)
            if ((obj->coord->x != ch->coord->x) || (obj->coord->y != ch->coord->y) || (obj->map != ch->map))
               continue;
      put_index = 0;
      add_obj_reset(pArea, 'O', obj, reset_count, pRoom->vnum);
   }
   if (dodoors)
   {
      EXIT_DATA *pexit;

      for (pexit = pRoom->first_exit; pexit; pexit = pexit->next)
      {
         int state = 0;

         if (!IS_SET(pexit->exit_info, EX_ISDOOR))
            continue;
         if (IS_SET(pexit->exit_info, EX_CLOSED))
         {
            if (IS_SET(pexit->exit_info, EX_LOCKED))
               state = 2;
            else
               state = 1;
         }
         add_reset(pArea, 'D', 0, pRoom->vnum, pexit->vdir, state, -1, -1, -1, -1, 0, 0);
      }
   }
   return;
}

void wipe_resets(AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom)
{
   RESET_DATA *pReset;

   for (pReset = pArea->first_reset; pReset;)
   {
      if (pReset->command != 'R' && is_room_reset(pReset, pRoom, pArea))
      {
         /* Resets always go forward, so we can safely use the previous reset,
            providing it exists, or first_reset if it doesnt.  -- Altrag */
         RESET_DATA *prev = pReset->prev;
         delete_reset(pArea, pReset);
         pReset = (prev ? prev->next : pArea->first_reset);
      }
      else
         pReset = pReset->next;
   }
   return;
}

void wipe_map_resets(CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, int x, int y, int map)
{
   RESET_DATA *pReset;
   RESET_DATA *prev;

   for (pReset = pArea->first_reset; pReset;)
   {
      if (pReset->command == 'M' || pReset->command == 'O')
      {
         if ((pReset->arg4 == ch->coord->x) && (pReset->arg5 == ch->coord->y) && (pReset->arg6 == ch->map))
         {
            prev = pReset->prev;
            delete_reset(pArea, pReset);
            pReset = (prev ? prev->next : pArea->first_reset);
         }
         else
         {
            pReset = pReset->next;
         }
      }
      else
      {
         pReset = pReset->next;
      }
   }
   return;
}

void do_instaroom(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *pArea;
   ROOM_INDEX_DATA *pRoom;
   bool dodoors;
   bool allmap = FALSE;
   char arg[MIL];
   int x, y, map;

   if (IS_NPC(ch) || get_trust(ch) < LEVEL_IMM || !ch->pcdata || /* Tracker1 */
      !ch->pcdata->area)
   {
      send_to_char("You don't have an assigned area to create resets for.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(argument, "nodoors"))
      dodoors = FALSE;
   else
      dodoors = TRUE;
   if (!str_cmp(argument, "allmap"))
      allmap = TRUE;
   pArea = ch->pcdata->area;
   if (!(pRoom = find_room(ch, arg, NULL)))
   {
      send_to_char("Room doesn't exist.\n\r", ch);
      return;
   }
   if (!can_mapmodify(ch, pRoom))
      return;
   if (pRoom->area != pArea && get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
   {
      send_to_char("You cannot reset that room.\n\r", ch);
      return;
   }
   /* Defaults only to change the coords, will not add an actual reset */
   if (pRoom->vnum == OVERLAND_SOLAN)
   {
      if (allmap == FALSE)
      {
         x = ch->coord->x;
         y = ch->coord->y;
         map = ch->map;
         wipe_map_resets(ch, pArea, pRoom, x, y, map);
         instaroom(ch, pArea, pRoom, FALSE, allmap);
         send_to_char("Mob coords changed (instaroom allmap to add resets to Wilderness).\n\r", ch);
         return;
      }
   }
   if (pArea->first_reset)
      wipe_resets(pArea, pRoom);
   instaroom(ch, pArea, pRoom, dodoors, allmap);
   send_to_char("Room resets installed.\n\r", ch);
}

void do_instazone(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *pArea;
   int vnum;
   ROOM_INDEX_DATA *pRoom;
   bool dodoors;

   if (IS_NPC(ch) || get_trust(ch) < LEVEL_IMM || !ch->pcdata || /* Tracker1 */
      !ch->pcdata->area)
   {
      send_to_char("You don't have an assigned area to create resets for.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "nodoors"))
      dodoors = FALSE;
   else
      dodoors = TRUE;
   pArea = ch->pcdata->area;
   if (pArea->first_reset)
      wipe_resets(pArea, NULL);
   for (vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++)
   {
      if (!(pRoom = get_room_index(vnum)) || pRoom->area != pArea)
         continue;
      instaroom(ch, pArea, pRoom, dodoors, 1);
   }
   send_to_char("Area resets installed.\n\r", ch);
   return;
}

int generate_itemlevel(AREA_DATA * pArea, OBJ_INDEX_DATA * pObjIndex)
{
   return 1;
}

int mob_in_area_count(int mvnum, int count, int vnum)
{
   CHAR_DATA *victim;
   AREA_DATA *tarea;
   int cnt = 0;
   
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
       if (vnum >= tarea->low_r_vnum && vnum <= tarea->hi_r_vnum)
          break;
   }
   if (!tarea)
   {
      for (tarea = first_build; tarea; tarea = tarea->next)
      {
         if (vnum >= tarea->low_r_vnum && vnum <= tarea->hi_r_vnum)
            break;
      }
   }     
   if (!tarea)
   {
      bug("mob_in_area_count:  Could not find the area for some reason, vnum %d", vnum);
      return FALSE;
   }
   for (victim = first_char; victim; victim = victim->next)
   {
      if (IS_NPC(victim) && victim->pIndexData->vnum == mvnum)
         if (victim->in_room->area == tarea)
         {
            cnt++;  
            if (count <= cnt)
               break;
         }
   }
   if (count <= cnt)
      return FALSE;
   else
      return TRUE;
}

int count_obj_list_eq(OBJ_INDEX_DATA * pObjIndex, OBJ_DATA * list)
{
   OBJ_DATA *obj;
   int nMatch = 0;

   for (obj = list; obj; obj = obj->next_content)
   {
      if (obj->pIndexData == pObjIndex && obj->wear_loc != WEAR_NONE)
      {
         if (obj->count > 1)
            nMatch += obj->count;
         else
            nMatch++;
      }
   }

   return nMatch;
}

/*
 * Reset one area.
 */

//ttype 0 - Regular area reset 1 - Time check reset
void reset_area(AREA_DATA * pArea, int ttype)
{
   RESET_DATA *pReset;
   CHAR_DATA *mob;
   OBJ_DATA *obj;
   OBJ_DATA *lastobj;
   ROOM_INDEX_DATA *pRoomIndex;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_INDEX_DATA *pObjToIndex;
   SLAB_DATA *slab;
   EXIT_DATA *pexit;
   OBJ_DATA *to_obj;
   int fvnum;
   int crace = 0;
   char buf[MSL];
   int level = 0;
   int race;
   int currentserial;
   int change = 0;
   void *plc = NULL;
   int timemob = 0;
   bool ext_bv = FALSE;
   TRAP_DATA *trap;

   if (!pArea)
   {
      bug("reset_area: NULL pArea", 0);
      return;
   }

   mob = NULL;
   obj = NULL;
   lastobj = NULL;
   if (!pArea->first_reset)
   {
      //bug("%s: reset_area: no resets", pArea->filename);
      return;
   }
   level = 0;
   for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
   {
      if (ttype == 1)
      {
         if (timemob && pReset->command != 'G' && pReset->command != 'E')
            timemob = 0;
         if (!timemob && pReset->resettime <= 0 && pReset->resetlast <= 0)
            continue;
         if (!timemob && pReset->resetlast + pReset->resettime > time(0))
            continue; 
      }
      switch (pReset->command)
      {
         default:
            sprintf(buf, "%s Reset_area: bad command %c.", pArea->filename, pReset->command);
            bug(buf, 0);
            break;
         case 'M':
            if (!(pMobIndex = get_mob_index(pReset->arg1)))
            {
               sprintf(buf, "%s Reset_area: 'M': bad mob vnum %d.", pArea->filename, pReset->arg1);
               bug(buf, 0);
               continue;
            }
            if (!(pRoomIndex = get_room_index(pReset->arg3)))
            {
               sprintf(buf, "%s Reset_area: 'M': bad room vnum %d.", pArea->filename, pReset->arg3);
               bug(buf, 0);
               continue;
            }
            if (pReset->resettime > 0 && pReset->resetlast > 0)
            {
               if (pReset->resetlast + pReset->resettime > time(0))
               {
                  mob = NULL;
                  break;
               }
            }
            if (pReset->serial == 0)
            {
               mob = create_mobile(pMobIndex);
               pReset->serial = serialmobsloaded;
               serial_list[pReset->serial] = TRUE;
            }
            else
            {
               if (serial_list[pReset->serial] == FALSE)
               {
                  currentserial = serialmobsloaded;
                  serialmobsloaded = pReset->serial-1;
                  mob = create_mobile(pMobIndex);
                  serialmobsloaded = currentserial;
                  serial_list[pReset->serial] = TRUE;
               }
               else
               {
                  mob = NULL;
                  break;
               }
            }               
            if (pReset->resettime == -1)
               xSET_BIT(mob->act, ACT_NORESET);
            if (pReset->resettime > 0)
               xSET_BIT(mob->act, ACT_TIMERESET); 
         /*   if (!mob_in_area_count(pMobIndex->vnum, pReset->arg2, pReset->arg3))
            {
               mob = NULL;
               break;
            }*/
            {
               ROOM_INDEX_DATA *pRoomPrev = get_room_index(pReset->arg3 - 1);

               if (pRoomPrev && xIS_SET(pRoomPrev->room_flags, ROOM_PET_SHOP))
                  xSET_BIT(mob->act, ACT_PET);
               if (pRoomPrev && xIS_SET(pRoomPrev->room_flags, ROOM_MOUNT_SHOP))
                  xSET_BIT(mob->act, ACT_MOUNTSAVE);
            }
            if (room_is_dark(pRoomIndex))
               xSET_BIT(mob->affected_by, AFF_INFRARED);
            char_to_room(mob, pRoomIndex);
            economize_mobgold(mob);
            level = 0;
            if (pRoomIndex->vnum == OVERLAND_SOLAN)
            {
               mob->coord->x = pReset->arg4;
               mob->coord->y = pReset->arg5;
               mob->map = pReset->arg6;
               SET_ONMAP_FLAG(mob);
            }
            if (ttype == 1)
               timemob = 1;
            break;
         case 'G':
         case 'E':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
               sprintf(buf, "%s Reset_area: 'E' or 'G': bad obj vnum %d.", pArea->filename, pReset->arg1);
               bug(buf, 0);
               continue;
            }
            if (!mob)
            {
               lastobj = NULL;
               break;
            }
            if (pReset->resettime > 0 && pReset->resetlast > 0)
            {
               if (pReset->resetlast + pReset->resettime > time(0))
               {
                  obj = NULL;
                  lastobj = NULL;
                  break;
               }
            }
            if ((pReset->command == 'G' && count_obj_list_inv(pObjIndex, mob->first_carrying) >= pReset->arg2)
            ||  (pReset->command == 'E' && count_obj_list_eq(pObjIndex, mob->first_carrying) >= pReset->arg2))
            {
               obj = NULL;
               lastobj = NULL;
               break;
            }
            if (mob->pIndexData->pShop)
            {
               int olevel = generate_itemlevel(pArea, pObjIndex);

               obj = create_object(pObjIndex, olevel);
               if (!xIS_SET(mob->act, ACT_CASTEMOB))
                  xSET_BIT(obj->extra_flags, ITEM_INVENTORY);
            }
            else
            {
               obj = create_object(pObjIndex, 1);
            }
            if (pReset->resettime == -1)
               xSET_BIT(obj->extra_flags, ITEM_NORESET);
            if (pReset->resettime > 0)
               xSET_BIT(obj->extra_flags, ITEM_TIMERESET); 
            obj->level = 0;
            /*       obj->count = (pReset->arg2 - count_obj_list(pObjIndex, mob->first_carrying)); */
            obj = obj_to_char(obj, mob);
            if (pReset->command == 'E')
               fvnum = pReset->arg4;
            else
               fvnum = pReset->arg3;
            if (fvnum > 100)
            {
               OBJ_DATA *oslab;
               for (slab = first_slab; slab; slab = slab->next)
               {
                  if (slab->vnum == fvnum)
                     break;
               }
               if (!slab)
               {
                  bug("%s Reset_area: 'G' or 'E': bad slab vnum %d.", pArea->filename, fvnum);
               }
               else
               {         
                  race = mob->race;
                  if (mob->race < 0 || mob->race >= MAX_RACE)
                     mob->race = 0; //Needs a valid race      
                  if (pReset->command == 'G' && pReset->arg4 > 0)
                  {
                     mob->race = pReset->arg4-1;
                     crace = pReset->arg4;
                  }
                  oslab = create_object(get_obj_index(slab->vnum), 1);
                  alter_forge_obj(mob, obj, oslab, slab);	
                  extract_obj(oslab);
                  obj->value[6] = fvnum;
                  obj->value[11] = crace;
                  mob->race = race;
               }
            }               
            if (pReset->command == 'E')
               equip_char(mob, obj, pReset->arg3);
            lastobj = obj;
            break;
         case 'O':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
/*
        sprintf (buf, "%s Reset_area: 'O': bad obj vnum %d.",
                pArea->filename, pReset->arg1 );
        bug ( buf, 0 );
*/
               continue;
            }
            if (!(pRoomIndex = get_room_index(pReset->arg3)))
            {
/*
        sprintf ( buf, "%s Reset_area: 'O': bad room vnum %d.", pArea->filename,
           pReset->arg3 );
        bug ( buf, 0 );
*/
               continue;
            }
            if (pReset->resettime > 0 && pReset->resetlast > 0)
            {
               if (pReset->resetlast + pReset->resettime > time(0))
               {
                  obj = NULL;
                  lastobj = NULL;
                  break;
               }
            }
            /* With objects it is important to load only the amount alouted the that obj */
            if ((count_obj_list(pObjIndex, pRoomIndex->first_content) >= pReset->arg2))
            {
               obj = NULL;
               lastobj = NULL;
               break;
            }
            obj = create_object(pObjIndex, 1);
            if (pReset->resettime == -1)
               xSET_BIT(obj->extra_flags, ITEM_NORESET);
            if (pReset->resettime > 0)
               xSET_BIT(obj->extra_flags, ITEM_TIMERESET);
            obj->level = 0;
/*      obj->count = ( pReset->arg2 - count_obj_list(pObjIndex, pRoomIndex->first_content)); */
            if (pRoomIndex->vnum == OVERLAND_SOLAN) // Make sure it doesn't group -- Xerves
            {
               obj->coord->x = pReset->arg4;
               obj->coord->y = pReset->arg5;
               obj->map = pReset->arg6;
               SET_OBJ_STAT(obj, ITEM_ONMAP);
            }
            obj_to_room(obj, pRoomIndex, NULL);
            if (pRoomIndex->vnum == OVERLAND_SOLAN) // Put the values back in after obj_to_room -- Xerves
            {
               obj->coord->x = pReset->arg4;
               obj->coord->y = pReset->arg5;
               obj->map = pReset->arg6;
               SET_OBJ_STAT(obj, ITEM_ONMAP);
            }
            if (pReset->arg7 > 100)
            {
               for (slab = first_slab; slab; slab = slab->next)
               {
                  if (slab->vnum == pReset->arg7)
                     break;
               }
               if (!slab)
               {
                  bug("%s Reset_area: 'G' or 'E': bad slab vnum %d.", pArea->filename, pReset->arg7);
               }
               else
               {         
                  race = mob->race;
                  if (mob->race < 0 || mob->race >= MAX_RACE)
                     mob->race = 0; //Needs a valid race      
                  alter_forge_obj(mob, obj, create_object(get_obj_index(slab->vnum), 1), slab);	
                  obj->value[6] = pReset->arg7;
                  mob->race = race;
               }
            }  
            lastobj = obj;
            break;

         //New Trap format, store only the uid here.
         case 'A': 
            for (trap = first_trap; trap; trap = trap->next)
            {
               if (trap->uid == pReset->arg1)
                  break;
            }
            if (!trap)
            {
               bug("%s Reset_area: 'A': bad trap uid of %d", pArea->filename, pReset->arg1);
               continue;
            }
            if (trap->resetvalue > 0)
            {
               trap->charges = trap->maxcharges;
            }
            if (!lastobj)
               continue;
            if (lastobj->trap)
               continue;
            if (trap->obj)
               continue;
            lastobj->trap = trap;
            trap->obj = lastobj;
            trap->area = pArea;
            trap->charges = trap->maxcharges;
            break;
         case 'P':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
/*
        sprintf ( buf, "%s Reset_area: 'P': bad obj vnum %d.", pArea->filename,
           pReset->arg1 );
        bug ( buf, 0 );
*/
               continue;
            }
            if (pReset->arg3 > 0)
            {
               if (!(pObjToIndex = get_obj_index(pReset->arg3)))
               {
/*
          sprintf(buf,"%s Reset_area: 'P': bad objto vnum %d.",pArea->filename,
                pReset->arg3 );
          bug( buf, 0 );
*/
                  continue;
               }           
               if (pReset->resettime > 0 && pReset->resetlast > 0)
               {
                  if (pReset->resetlast + pReset->resettime > time(0))
                  {
                     obj = NULL;
                     break;
                  }
               }    
               if (pArea->nplayer > 0
                  || !(to_obj = get_obj_type(pObjToIndex)) || !to_obj->in_room || (count_obj_list(pObjIndex, to_obj->first_content) >= pReset->arg2))
               {
                  obj = NULL;
                  break;
               }
               lastobj = to_obj;
            }
            else
            {
               int iNest;

               if (!lastobj)
                  break;
               to_obj = lastobj;
               for (iNest = 0; iNest < pReset->extra; iNest++)
                  if (!(to_obj = to_obj->last_content))
                  {
/*
            sprintf(buf,"%s Reset_area: 'P': Invalid nesting obj %d."
                ,pArea->filename, pReset->arg1 );
            bug( buf, 0 );
*/
                     iNest = -1;
                     break;
                  }
               if (iNest < 0)
                  continue;
            }
            obj = create_object(pObjIndex, 1);
            if (pReset->resettime == -1)
               xSET_BIT(obj->extra_flags, ITEM_NORESET);
            if (pReset->resettime > 0)
               xSET_BIT(obj->extra_flags, ITEM_TIMERESET);
            obj->level = 0;
            /*   obj->count = ( pReset->arg2 - count_obj_list(pObjIndex, to_obj->first_content));   */
            obj_to_obj(obj, to_obj);
            if (pReset->arg4 > 100)
            {
               for (slab = first_slab; slab; slab = slab->next)
               {
                  if (slab->vnum == pReset->arg4)
                     break;
               }
               if (!slab)
               {
                  bug("%s Reset_area: 'P': bad slab vnum %d.", pArea->filename, pReset->arg4);
               }
               else
               {         
                  race = mob->race;
                  if (mob->race < 0 || mob->race >= MAX_RACE)
                     mob->race = 0; //Needs a valid race      
                  alter_forge_obj(mob, obj, create_object(get_obj_index(slab->vnum), 1), slab);	
                  obj->value[6] = pReset->arg4;
                  mob->race = race;
               }
            }    
            break;

         case 'T':
            if (IS_SET(pReset->extra, TRAP_OBJ))
            {
               /* We need to preserve obj for future 'T' and 'H' checks */
               OBJ_DATA *pobj;

               if (pReset->arg3 > 0)
               {
                  if (!(pObjToIndex = get_obj_index(pReset->arg3)))
                  {
/*
            sprintf (buf,"%s Reset_area: 'T': bad objto vnum %d."
                ,pArea->filename, pReset->arg3 );
            bug ( buf, 0 );
*/
                     continue;
                  }
                  if (pArea->nplayer > 0 ||
                     !(to_obj = get_obj_type(pObjToIndex)) || (to_obj->carried_by && !IS_NPC(to_obj->carried_by)) || is_trapped(to_obj))
                     break;
               }
               else
               {
                  if (!lastobj || !obj)
                     break;
                  to_obj = obj;
               }
               pobj = make_trap(pReset->arg2, pReset->arg1, 1, pReset->extra);
               obj_to_obj(pobj, to_obj);
            }
            else
            {
               if (!(pRoomIndex = get_room_index(pReset->arg3)))
               {
/*
          sprintf(buf,"%s Reset_area: 'T': bad room %d.", pArea->filename,
                pReset->arg3 );
          bug( buf, 0 );
*/
                  continue;
               }
               if (pArea->nplayer > 0 || count_obj_list(get_obj_index(OBJ_VNUM_TRAP), pRoomIndex->first_content) > 0)
                  break;
               to_obj = make_trap(pReset->arg1, pReset->arg1, 10, pReset->extra);
               obj_to_room(to_obj, pRoomIndex, NULL);
            }
            break;

         case 'H':
            if (pReset->arg1 > 0)
            {
               if (!(pObjToIndex = get_obj_index(pReset->arg1)))
               {
/*
          sprintf(buf,"%s Reset_area: 'H': bad objto vnum %d.",pArea->filename,
                pReset->arg1 );
          bug( buf, 0 );
*/
                  continue;
               }
               if (pArea->nplayer > 0 ||
                  !(to_obj = get_obj_type(pObjToIndex)) || !to_obj->in_room || to_obj->in_room->area != pArea || IS_OBJ_STAT(to_obj, ITEM_HIDDEN))
                  break;
            }
            else
            {
               if (!lastobj || !obj)
                  break;
               to_obj = obj;
            }
            xSET_BIT(to_obj->extra_flags, ITEM_HIDDEN);
            break;

         case 'B':
            switch (pReset->arg2 & BIT_RESET_TYPE_MASK)
            {
               case BIT_RESET_DOOR:
                  {
                     int doornum;

                     if (!(pRoomIndex = get_room_index(pReset->arg1)))
                     {
/*
          sprintf(buf,"%s Reset_area: 'B': door: bad room vnum %d.",
                pArea->filename, pReset->arg1 );
          bug( buf, 0 );
*/
                        continue;
                     }
                     doornum = (pReset->arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;
                     if (!(pexit = get_exit(pRoomIndex, doornum)))
                        break;
                     plc = &pexit->exit_info;
                  }
                  break;
               case BIT_RESET_ROOM:
                  if (!(pRoomIndex = get_room_index(pReset->arg1)))
                  {
/*
          sprintf(buf,"%s Reset_area: 'B': room: bad room vnum %d.",
                pArea->filename, pReset->arg1 );
          bug(buf, 0);
*/
                     continue;
                  }
                  plc = &pRoomIndex->room_flags;
                  break;
               case BIT_RESET_OBJECT:
                  if (pReset->arg1 > 0)
                  {
                     if (!(pObjToIndex = get_obj_index(pReset->arg1)))
                     {
/*
            sprintf(buf,"%s Reset_area: 'B': object: bad objto vnum %d.",
                pArea->filename, pReset->arg1 );
            bug( buf, 0 );
*/
                        continue;
                     }
                     if (!(to_obj = get_obj_type(pObjToIndex)) || !to_obj->in_room || to_obj->in_room->area != pArea)
                        continue;
                  }
                  else
                  {
                     if (!lastobj || !obj)
                        continue;
                     to_obj = obj;
                  }
                  plc = &to_obj->extra_flags;
                  ext_bv = TRUE;
                  break;
               case BIT_RESET_MOBILE:
                  if (!mob)
                     continue;
                  plc = &mob->affected_by;
                  ext_bv = TRUE;
                  break;
               default:
/*
        sprintf(buf, "%s Reset_area: 'B': bad options %d.",
                pArea->filename, pReset->arg2 );
        bug( buf, 0 );
*/
                  continue;
            }
            if (IS_SET(pReset->arg2, BIT_RESET_SET))
            {
               if (ext_bv)
                  xSET_BIT(*(EXT_BV *) plc, pReset->arg3);
               else
                  SET_BIT(*(int *) plc, pReset->arg3);
            }
            else if (IS_SET(pReset->arg2, BIT_RESET_TOGGLE))
            {
               if (ext_bv)
                  xTOGGLE_BIT(*(EXT_BV *) plc, pReset->arg3);
               else
                  TOGGLE_BIT(*(int *) plc, pReset->arg3);
            }
            else
            {
               if (ext_bv)
                  xREMOVE_BIT(*(EXT_BV *) plc, pReset->arg3);
               else
                  REMOVE_BIT(*(int *) plc, pReset->arg3);
            }
            break;

         case 'D':
            if (!(pRoomIndex = get_room_index(pReset->arg1)))
            {
/*
        sprintf(buf, "%s Reset_area: 'D': bad room vnum %d.",
                pArea->filename, pReset->arg1 );
        bug(buf, 0);
*/
               continue;
            }
            if (!(pexit = get_exit(pRoomIndex, pReset->arg2)))
               break;
            switch (pReset->arg3)
            {
               case 0:
                  REMOVE_BIT(pexit->exit_info, EX_CLOSED);
                  REMOVE_BIT(pexit->exit_info, EX_LOCKED);
                  break;
               case 1:
                  SET_BIT(pexit->exit_info, EX_CLOSED);
                  REMOVE_BIT(pexit->exit_info, EX_LOCKED);
                  if (IS_SET(pexit->exit_info, EX_xSEARCHABLE))
                     SET_BIT(pexit->exit_info, EX_SECRET);
                  break;
               case 2:
                  SET_BIT(pexit->exit_info, EX_CLOSED);
                  SET_BIT(pexit->exit_info, EX_LOCKED);
                  if (IS_SET(pexit->exit_info, EX_xSEARCHABLE))
                     SET_BIT(pexit->exit_info, EX_SECRET);
                  break;
            }
            break;

         case 'R':
            if (!(pRoomIndex = get_room_index(pReset->arg1)))
            {
/*
        sprintf(buf,"%s Reset_area: 'R': bad room vnum %d.",
                pArea->filename, pReset->arg1 );
        bug(buf, 0);
*/
               continue;
            }
            randomize_exits(pRoomIndex, pReset->arg2 - 1);
            break;
      }
   }
   if (change == 1)
      fold_area(pArea, pArea->filename, FALSE, 1); 
   return;
}

void list_resets(CHAR_DATA * ch, AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, int start, int end)
{
   RESET_DATA *pReset;
   ROOM_INDEX_DATA *room;
   MOB_INDEX_DATA *mob;
   OBJ_INDEX_DATA *obj, *obj2;
   OBJ_INDEX_DATA *lastobj;
   RESET_DATA *lo_reset;
   bool found;
   int num = 0;
   const char *rname = "???", *mname = "???", *oname = "???";
   char buf[256];
   char time[256];
   char *pbuf;
   int sec, hour, min, days, rtime;
   TRAP_DATA *trap;

   if (!ch || !pArea)
      return;
   room = NULL;
   mob = NULL;
   obj = NULL;
   lastobj = NULL;
   lo_reset = NULL;
   found = FALSE;

   for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
   {
      if (!is_room_reset(pReset, pRoom, pArea))
         continue;
      ++num;
      sprintf(buf, "%s%3d) ", char_color_str(AT_PURPLE, ch), num);
      pbuf = buf + strlen(buf);
      
      if (pReset->resettime > 0)
      {
         if (pReset->resettime > 86400)
         {
            days = pReset->resettime / 86400;
            rtime = pReset->resettime % 86400;
         }
         else
         {
            days = 0;
            rtime = pReset->resettime;
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
         sprintf(time, "D:%d H:%d M:%d S:%d", days, hour, min, sec);
      }
      if (pReset->resettime == -1)
         sprintf(time, "*****ONE RESET*****");
      switch (pReset->command)
      {
         default:
            sprintf(pbuf, "*** BAD RESET: %c %d %d %d %d ***\n\r", pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3);
            break;
         case 'M':
            if (!(mob = get_mob_index(pReset->arg1)))
               mname = "Mobile: *BAD VNUM*";
            else
               mname = mob->player_name;
            if (!(room = get_room_index(pReset->arg3)))
               rname = "Room: *BAD VNUM*";
            else
               rname = room->name;
            if (pReset->resettime > 0 || pReset->resettime == -1)
               sprintf(pbuf, "%s%s (%d) -> %s (%d) (%d %d %d) [%d] &w&W<%s>", char_color_str(AT_GREEN, ch), mname, pReset->arg1, rname,
                  pReset->arg3, pReset->arg4, pReset->arg5, pReset->arg6, pReset->serial, time);
            else
               sprintf(pbuf, "%s%s (%d) -> %s (%d) (%d %d %d) [%d]", char_color_str(AT_GREEN, ch), mname, pReset->arg1, rname,
                  pReset->arg3, pReset->arg4, pReset->arg5, pReset->arg6, pReset->serial);
            
            if (!room)
               mob = NULL;
            if ((room = get_room_index(pReset->arg3 - 1)) && xIS_SET(room->room_flags, ROOM_PET_SHOP))
               strcat(buf, "&w&c (pet)\n\r");
            else if ((room = get_room_index(pReset->arg3 - 1)) && xIS_SET(room->room_flags, ROOM_MOUNT_SHOP))
               strcat(buf, "&w&C (mount)\n\r");
            else
               strcat(buf, "\n\r");
            break;
         case 'G':
         case 'E':
            if (!mob)
               mname = "* ERROR: NO MOBILE! *";
            if (!(obj = get_obj_index(pReset->arg1)))
               oname = "Object: *BAD VNUM*";
            else
               oname = obj->name;
            if (pReset->resettime > 0 || pReset->resettime == -1)
               sprintf(pbuf, "%s%s (%d) -> %s (%s) [%d] &w&W<%s>", pReset->command == 'G' ? char_color_str(AT_ORANGE, ch) : char_color_str(AT_YELLOW, ch),
                  oname, pReset->arg1, mname, (pReset->command == 'G' ? "carry" : wear_locs[pReset->arg3]), pReset->arg2, time);
            else
               sprintf(pbuf, "%s%s (%d) -> %s (%s) [%d]", pReset->command == 'G' ? char_color_str(AT_ORANGE, ch) : char_color_str(AT_YELLOW, ch), 
                  oname, pReset->arg1, mname, (pReset->command == 'G' ? "carry" : wear_locs[pReset->arg3]), pReset->arg2);
            if (mob && mob->pShop)
               strcat(buf, "&w&R (shop)\n\r");
            else
               strcat(buf, "\n\r");
            lastobj = obj;
            lo_reset = pReset;
            break;
         case 'A':
            for (trap = first_trap; trap; trap = trap->next)
            {
               if (trap->uid == pReset->arg1)
                  break;
            }
            if (!trap)
               sprintf(pbuf, "*INVALID TRAP UID*\n\r");
            else
               sprintf(pbuf, "%s(TRAP) %d uid %d lowdam %d hidam\n\r", char_color_str(AT_RED, ch), trap->uid, trap->damlow, trap->damhigh);
            break;
            
         case 'O':
            if (!(obj = get_obj_index(pReset->arg1)))
               oname = "Object: *BAD VNUM*";
            else
               oname = obj->name;
            if (!(room = get_room_index(pReset->arg3)))
               rname = "Room: *BAD VNUM*";
            else
               rname = room->name;
            if (pReset->resettime > 0 || pReset->resettime == -1)
               sprintf(pbuf, "%s(object) %s (%d) -> %s (%d) (%d %d %d) [%d] &w&W<%s>\n\r", char_color_str(AT_PINK, ch), oname,
                  pReset->arg1, rname, pReset->arg3, pReset->arg4, pReset->arg5, pReset->arg6, pReset->arg2, time);
            else
               sprintf(pbuf, "%s(object) %s (%d) -> %s (%d) (%d %d %d) [%d]\n\r", char_color_str(AT_PINK, ch), oname,
                  pReset->arg1, rname, pReset->arg3, pReset->arg4, pReset->arg5, pReset->arg6, pReset->arg2);

            if (!room)
               obj = NULL;
            lastobj = obj;
            lo_reset = pReset;
            break;
         case 'P':
            if (!(obj = get_obj_index(pReset->arg1)))
               oname = "Object1: *BAD VNUM*";
            else
               oname = obj->name;
            obj2 = NULL;
            if (pReset->arg3 > 0)
            {
               obj2 = get_obj_index(pReset->arg3);
               rname = (obj2 ? obj2->name : "Object2: *BAD VNUM*");
               lastobj = obj2;
            }
            else if (!lastobj)
               rname = "Object2: *NULL obj*";
            else if (pReset->extra == 0)
            {
               rname = lastobj->name;
               obj2 = lastobj;
            }
            else
            {
               int iNest;
               RESET_DATA *reset;

               reset = lo_reset->next;
               for (iNest = 0; iNest < pReset->extra; iNest++)
               {
                  for (; reset; reset = reset->next)
                     if (reset->command == 'O' || reset->command == 'G' ||
                        reset->command == 'E' || (reset->command == 'P' &&
!reset->arg3 && reset->extra == iNest && (get_obj_index(reset->arg1)->item_type == ITEM_CONTAINER)))
                        break;
                  if (!reset || reset->command != 'P')
                     break;
               }
               if (!reset)
                  rname = "Object2: *BAD NESTING*";
               else if (!(obj2 = get_obj_index(reset->arg1)))
                  rname = "Object2: *NESTED BAD VNUM*";
               else
                  rname = obj2->name;
            }
            if (pReset->resettime > 0 || pReset->resettime == -1)
               sprintf(pbuf, "%s(Put) %s (%d) -> %s (%d) [%d] {nest %d} &w&W<%s>\n\r", char_color_str(AT_DGREEN, ch), oname,
                  pReset->arg1, rname, (obj2 ? obj2->vnum : pReset->arg3), pReset->arg2, pReset->extra, time);
            else  
               sprintf(pbuf, "%s(Put) %s (%d) -> %s (%d) [%d] {nest %d}\n\r", char_color_str(AT_DGREEN, ch), oname,
                  pReset->arg1, rname, (obj2 ? obj2->vnum : pReset->arg3), pReset->arg2, pReset->extra);
            break;
         case 'T':
            sprintf(pbuf, "%sTRAP: %d %d %d %d (%s)\n\r", char_color_str(AT_BLOOD, ch), pReset->extra, pReset->arg1,
               pReset->arg2, pReset->arg3, flag_string(pReset->extra, trap_flags));
            break;
         case 'H':
            if (pReset->arg1 > 0)
               if (!(obj2 = get_obj_index(pReset->arg1)))
                  rname = "Object: *BAD VNUM*";
               else
                  rname = obj2->name;
            else if (!obj)
               rname = "Object: *NULL obj*";
            else
               rname = oname;
            sprintf(pbuf, "%sHide %s (%d)\n\r", char_color_str(AT_DGREY, ch), rname, (pReset->arg1 > 0 ? pReset->arg1 : obj ? obj->vnum : 0));
            break;
         case 'B':
            {
               char *const *flagarray;
               bool ext_bv = FALSE;

               strcpy(pbuf, "BIT: ");
               pbuf += 5;
               if (IS_SET(pReset->arg2, BIT_RESET_SET))
               {
                  strcpy(pbuf, "Set: ");
                  pbuf += 5;
               }
               else if (IS_SET(pReset->arg2, BIT_RESET_TOGGLE))
               {
                  strcpy(pbuf, "Toggle: ");
                  pbuf += 8;
               }
               else
               {
                  strcpy(pbuf, "Remove: ");
                  pbuf += 8;
               }
               switch (pReset->arg2 & BIT_RESET_TYPE_MASK)
               {
                  case BIT_RESET_DOOR:
                     {
                        int door;

                        if (!(room = get_room_index(pReset->arg1)))
                           rname = "Room: *BAD VNUM*";
                        else
                           rname = room->name;
                        door = (pReset->arg2 & BIT_RESET_DOOR_MASK) >> BIT_RESET_DOOR_THRESHOLD;
                        door = URANGE(0, door, MAX_DIR + 1);
                        sprintf(pbuf, "Exit %s%s (%d), Room %s (%d)", dir_name[door],
                           (room && get_exit(room, door) ? "" : " (NO EXIT!)"), door, rname, pReset->arg1);
                     }
                     flagarray = ex_flags;
                     break;
                  case BIT_RESET_ROOM:
                     if (!(room = get_room_index(pReset->arg1)))
                        rname = "Room: *BAD VNUM*";
                     else
                        rname = room->name;
                     sprintf(pbuf, "Room %s (%d)", rname, pReset->arg1);
                     flagarray = r_flags;
                     break;
                  case BIT_RESET_OBJECT:
                     if (pReset->arg1 > 0)
                        if (!(obj2 = get_obj_index(pReset->arg1)))
                           rname = "Object: *BAD VNUM*";
                        else
                           rname = obj2->name;
                     else if (!obj)
                        rname = "Object: *NULL obj*";
                     else
                        rname = oname;
                     sprintf(pbuf, "Object %s (%d)", rname, (pReset->arg1 > 0 ? pReset->arg1 : obj ? obj->vnum : 0));
                     flagarray = o_flags;
                     ext_bv = TRUE;
                     break;
                  case BIT_RESET_MOBILE:
                     if (pReset->arg1 > 0)
                     {
                        MOB_INDEX_DATA *mob2;

                        if (!(mob2 = get_mob_index(pReset->arg1)))
                           rname = "Mobile: *BAD VNUM*";
                        else
                           rname = mob2->player_name;
                     }
                     else if (!mob)
                        rname = "Mobile: *NULL mob*";
                     else
                        rname = mname;
                     sprintf(pbuf, "Mobile %s (%d)", rname, (pReset->arg1 > 0 ? pReset->arg1 : mob ? mob->vnum : 0));
                     flagarray = a_flags;
                     ext_bv = TRUE;
                     break;
                  default:
                     sprintf(pbuf, "bad type %d", pReset->arg2 & BIT_RESET_TYPE_MASK);
                     flagarray = NULL;
                     break;
               }
               pbuf += strlen(pbuf);
               if (flagarray)
               {
                  if (ext_bv)
                  {
                     EXT_BV tmp;

                     tmp = meb(pReset->arg3);
                     sprintf(pbuf, "; flags: %s [%d]\n\r", ext_flag_string(&tmp, flagarray), pReset->arg3);
                  }
                  else
                     sprintf(pbuf, "; flags: %s [%d]\n\r", flag_string(pReset->arg3, flagarray), pReset->arg3);
               }
               else
                  sprintf(pbuf, "; flags %d\n\r", pReset->arg3);
            }
            break;
         case 'D':
            {
               char *ef_name;

               pReset->arg2 = URANGE(0, pReset->arg2, MAX_DIR + 1);
               if (!(room = get_room_index(pReset->arg1)))
                  rname = "Room: *BAD VNUM*";
               else
                  rname = room->name;
               switch (pReset->arg3)
               {
                  default:
                     ef_name = "(* ERROR *)";
                     break;
                  case 0:
                     ef_name = "Open";
                     break;
                  case 1:
                     ef_name = "Close";
                     break;
                  case 2:
                     ef_name = "Close and lock";
                     break;
               }
               sprintf(pbuf, "%s%s [%d] the %s%s [%d] door %s (%d)\n\r", char_color_str(AT_GREY, ch), ef_name,
                  pReset->arg3, dir_name[pReset->arg2],
                  (room && get_exit(room, pReset->arg2) ? "" : " (NO EXIT!)"), pReset->arg2, rname, pReset->arg1);
            }
            break;
         case 'R':
            if (!(room = get_room_index(pReset->arg1)))
               rname = "Room: *BAD VNUM*";
            else
               rname = room->name;
            sprintf(pbuf, "%sRandomize exits 0 to %d -> %s (%d)\n\r", char_color_str(AT_BLUE, ch), pReset->arg2, rname, pReset->arg1);
            break;
      }
      if (start == -1 || num >= start)
         send_to_char(buf, ch);
      if (end != -1 && num >= end)
         break;
   }
   if (num == 0)
      send_to_char("You don't have any resets defined.\n\r", ch);
   return;
}

/* Setup put nesting levels, regardless of whether or not the resets will
   actually reset, or if they're bugged. */
void renumber_put_resets(AREA_DATA * pArea)
{
   RESET_DATA *pReset, *lastobj = NULL;
   OBJ_INDEX_DATA *putobj;

   for (pReset = pArea->first_reset; pReset; pReset = pReset->next)
   {
      switch (pReset->command)
      {
         default:
            break;
         case 'G':
         case 'E':
         case 'O':
            lastobj = pReset;
            break;
         case 'P':
            if (pReset->arg3 == 0)
            {
               if (!lastobj)
                  pReset->extra = 1000000;
               else if (lastobj->command != 'P' || lastobj->arg3 > 0)
                  pReset->extra = 0;
               else
               {
                  if ((putobj = get_obj_index(lastobj->arg1)) != NULL)
                  {
                     if ((putobj->item_type == ITEM_CONTAINER) || (putobj->item_type == ITEM_FURNITURE) || (putobj->item_type == ITEM_QUIVER))
                        pReset->extra = lastobj->extra + 1;
                     else
                        pReset->extra = UMAX((lastobj->extra - 1), 0);
                  }
               }
               lastobj = pReset;
            }
      }
   }
   return;
}

/*
 * Create a new reset (for online building)			-Thoric
 */
RESET_DATA *make_reset(char letter, int extra, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int resetlast, int resettime)
{
   RESET_DATA *pReset;

   CREATE(pReset, RESET_DATA, 1);
   pReset->command = letter;
   pReset->extra = extra;
   pReset->arg1 = arg1;
   pReset->arg2 = arg2;
   pReset->arg3 = arg3;
   pReset->arg4 = arg4;
   pReset->arg5 = arg5;
   pReset->arg6 = arg6;
   pReset->arg7 = arg7;
   pReset->resetlast = resetlast;
   pReset->resettime = resettime;
   top_reset++;
   return pReset;
}

/*
 * Add a reset to an area				-Thoric
 */
RESET_DATA *add_reset(AREA_DATA * tarea, char letter, int extra, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int resetlast, int resettime)
{
   RESET_DATA *pReset;

   if (!tarea)
   {
      bug("add_reset: NULL area!", 0);
      return NULL;
   }

   letter = UPPER(letter);
   pReset = make_reset(letter, extra, arg1, arg2, arg3, arg4, arg5, arg6, arg7, resetlast, resettime);
   
   switch (letter)
   {
      case 'M':
         tarea->last_mob_reset = pReset;
         pReset->arg2 = 1;
         break;
      case 'H':
         if (arg1 > 0)
            break;
      case 'E':
      case 'G':
      case 'P':
      case 'O':
         tarea->last_obj_reset = pReset;
         break;
      case 'T':
         if (IS_SET(extra, TRAP_OBJ) && arg1 == 0)
            tarea->last_obj_reset = pReset;
         break;
   }

   LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
   return pReset;
}

/*
 * Place a reset into an area, insert sorting it		-Thoric
 */
RESET_DATA *place_reset(AREA_DATA * tarea, char letter, int extra, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int resetlast, int resettime)
{
   RESET_DATA *pReset, *tmp, *tmp2;

   if (!tarea)
   {
      bug("place_reset: NULL area!", 0);
      return NULL;
   }

   letter = UPPER(letter);
   pReset = make_reset(letter, extra, arg1, arg2, arg3, arg4, arg5, arg6, arg7, resetlast, resettime);
   if (letter == 'M')
      tarea->last_mob_reset = pReset;

   if (tarea->first_reset)
   {
      switch (letter)
      {
         default:
            bug("place_reset: Bad reset type %c", letter);
            return NULL;
         case 'D':
         case 'R':
            for (tmp = tarea->last_reset; tmp; tmp = tmp->prev)
               if (tmp->command == letter)
                  break;
            if (tmp) /* organize by location */
               for (; tmp && tmp->command == letter && tmp->arg1 > arg1; tmp = tmp->prev) ;
            if (tmp) /* organize by direction */
               for (; tmp && tmp->command == letter && tmp->arg1 == tmp->arg1 && tmp->arg2 > arg2; tmp = tmp->prev) ;
            if (tmp)
               INSERT(pReset, tmp, tarea->first_reset, next, prev);
            else
               LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
            return pReset;
         case 'M':
         case 'O':
            /* find last reset of same type */
            for (tmp = tarea->last_reset; tmp; tmp = tmp->prev)
               if (tmp->command == letter)
                  break;
            tmp2 = tmp ? tmp->next : NULL;
            /* organize by location */
            for (; tmp; tmp = tmp->prev)
               if (tmp->command == letter && tmp->arg3 <= arg3)
               {
                  tmp2 = tmp->next;
                  /* organize by vnum */
                  if (tmp->arg3 == arg3)
                     for (; tmp; tmp = tmp->prev)
                        if (tmp->command == letter && tmp->arg3 == tmp->arg3 && tmp->arg1 <= arg1)
                        {
                           tmp2 = tmp->next;
                           break;
                        }
                  break;
               }
            /* skip over E or G for that mob */
            if (tmp2 && letter == 'M')
            {
               for (; tmp2; tmp2 = tmp2->next)
                  if (tmp2->command != 'E' && tmp2->command != 'G')
                     break;
            }
            else
               /* skip over P, T or H for that obj */
            if (tmp2 && letter == 'O')
            {
               for (; tmp2; tmp2 = tmp2->next)
                  if (tmp2->command != 'P' && tmp2->command != 'T' && tmp2->command != 'H')
                     break;
            }
            if (tmp2)
               INSERT(pReset, tmp2, tarea->first_reset, next, prev);
            else
               LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
            return pReset;
         case 'G':
         case 'E':
            /* find the last mob */
            if ((tmp = tarea->last_mob_reset) != NULL)
            {
               /*
                * See if there are any resets for this mob yet,
                * put E before G and organize by vnum
                */
               if (tmp->next)
               {
                  tmp = tmp->next;
                  if (tmp && tmp->command == 'E')
                  {
                     if (letter == 'E')
                        for (; tmp && tmp->command == 'E' && tmp->arg1 < arg1; tmp = tmp->next) ;
                     else
                        for (; tmp && tmp->command == 'E'; tmp = tmp->next) ;
                  }
                  else if (tmp && tmp->command == 'G' && letter == 'G')
                     for (; tmp && tmp->command == 'G' && tmp->arg1 < arg1; tmp = tmp->next) ;
                  if (tmp)
                     INSERT(pReset, tmp, tarea->first_reset, next, prev);
                  else
                     LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
               }
               else
                  LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
               return pReset;
            }
            break;
         case 'P':
         case 'T':
         case 'H':
            /* find the object in question */
            if (((letter == 'P' && arg3 == 0)
                  || (letter == 'T' && IS_SET(extra, TRAP_OBJ) && arg1 == 0)
                  || (letter == 'H' && arg1 == 0)) && (tmp = tarea->last_obj_reset) != NULL)
            {
               if ((tmp = tmp->next) != NULL)
                  INSERT(pReset, tmp, tarea->first_reset, next, prev);
               else
                  LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
               return pReset;
            }

            for (tmp = tarea->last_reset; tmp; tmp = tmp->prev)
               if ((tmp->command == 'O' || tmp->command == 'G' || tmp->command == 'E' || tmp->command == 'P') && tmp->arg1 == arg3)
               {
                  /*
                   * See if there are any resets for this object yet,
                   * put P before H before T and organize by vnum
                   */
                  if (tmp->next)
                  {
                     tmp = tmp->next;
                     if (tmp && tmp->command == 'P')
                     {
                        if (letter == 'P' && tmp->arg3 == arg3)
                           for (; tmp && tmp->command == 'P' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next) ;
                        else if (letter != 'T')
                           for (; tmp && tmp->command == 'P' && tmp->arg3 == arg3; tmp = tmp->next) ;
                     }
                     else if (tmp && tmp->command == 'H')
                     {
                        if (letter == 'H' && tmp->arg3 == arg3)
                           for (; tmp && tmp->command == 'H' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next) ;
                        else if (letter != 'H')
                           for (; tmp && tmp->command == 'H' && tmp->arg3 == arg3; tmp = tmp->next) ;
                     }
                     else if (tmp && tmp->command == 'T' && letter == 'T')
                        for (; tmp && tmp->command == 'T' && tmp->arg3 == arg3 && tmp->arg1 < arg1; tmp = tmp->next) ;
                     if (tmp)
                        INSERT(pReset, tmp, tarea->first_reset, next, prev);
                     else
                        LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
                  }
                  else
                     LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
                  return pReset;
               }
            break;
      }
      /* likely a bad reset if we get here... add it anyways */
   }
   LINK(pReset, tarea->first_reset, tarea->last_reset, next, prev);
   return pReset;
}

void rsmob(AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom)
{
   CHAR_DATA *rch;
   OBJ_DATA *obj;
   RESET_DATA *reset;

   if (pArea->first_reset)
      wipe_resets(pArea, pRoom);
   for (rch = pRoom->first_person; rch; rch = rch->next_in_room)
   {
      /* Small note, Xerves screwed up here *grins* -- Xerves self-note */
      if (!IS_NPC(rch))
         continue;
      reset = add_reset(pArea, 'M', 1, rch->pIndexData->vnum, 1, pRoom->vnum, rch->coord->x, rch->coord->y, rch->map, -1, 0, 0);
      reset->serial = rch->serial;
      serial_list[reset->serial] = TRUE;
      for (obj = rch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->wear_loc == WEAR_NONE)
         {
            add_obj_reset(pArea, 'G', obj, 1, 0);
         }
         else
         {
            add_obj_reset(pArea, 'E', obj, 1, obj->wear_loc);
         }
      }
   }
   return;
}

void kupkeep(AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom)
{
   CHAR_DATA *rch;
   OBJ_DATA *obj;
   RESET_DATA *reset;

   if (pArea->first_reset)
      wipe_resets(pArea, pRoom);
   for (rch = pRoom->first_person; rch; rch = rch->next_in_room)
   {
      /* Small note, Xerves screwed up here *grins* -- Xerves self-note */
      if (!IS_NPC(rch))
         continue;
      if (!xIS_SET(rch->act, ACT_CASTEMOB))
         continue;
      reset = add_reset(pArea, 'M', 1, rch->pIndexData->vnum, 1, pRoom->vnum, rch->coord->x, rch->coord->y, rch->map, -1, 0, 0);
      reset->serial = rch->serial;
      serial_list[reset->serial] = TRUE;
      for (obj = rch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->wear_loc == WEAR_NONE)
         {
            add_obj_reset(pArea, 'G', obj, 1, 0);
         }
         else
         {
            add_obj_reset(pArea, 'E', obj, 1, obj->wear_loc);
         }
      }
   }
   return;
}

void do_resetkeeper(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *pArea;
   ROOM_INDEX_DATA *pRoom;
   char arg[MIL];

   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot reset other mobs.  Sorry :-)\n\r", ch);
      return;
   }
   if (!ch->pcdata || !ch->pcdata->keeper || (ch->pcdata->caste < 7))
   {
      send_to_char("You need to have a shop keeper to reset him/her.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!(pRoom = find_room(ch, arg, NULL)))
   {
      send_to_char("Room doesn't exsist.\n\r", ch);
      return;
   }
   pArea = pRoom->area;
   if (!can_kmodify(ch, pRoom))
      return;
   if (pRoom->vnum != ch->pcdata->keeper) /* Tracker1 */
   {
      send_to_char("You cannot reset that room.\n\r", ch);
      return;
   }
   if (pArea->first_reset)
      wipe_resets(pArea, pRoom);
   resetkeeper(pArea, pRoom, FALSE);
   send_to_char("ShopKeeper is Reset.\n\r", ch);
}

bool can_kmodify(CHAR_DATA * ch, ROOM_INDEX_DATA * room)
{
   sh_int vnum = room->vnum;

   if (IS_NPC(ch))
      return FALSE;
   if (!xIS_SET(room->room_flags, ROOM_PROTOTYPE))
   {
      send_to_char("You cannot modify this room.\n\r", ch);
      return FALSE;
   }
   if (!ch->pcdata || !(ch->pcdata->keeper))
   {
      send_to_char("You must have an assigned shop keeper to modify him/her.\n\r", ch);
      return FALSE;
   }
   if (vnum == ch->pcdata->keeper)
      return TRUE;

   send_to_char("What are you trying to modify?\n\r", ch);
   return FALSE;
}

void resetkeeper(AREA_DATA * pArea, ROOM_INDEX_DATA * pRoom, bool dodoors)
{
   CHAR_DATA *rch;
   OBJ_DATA *obj;
   RESET_DATA *reset;

   for (rch = pRoom->first_person; rch; rch = rch->next_in_room)
   {
      if (!IS_NPC(rch) || !xIS_SET(rch->act, ACT_CASTEMOB))
         continue;
      reset = add_reset(pArea, 'M', 1, rch->pIndexData->vnum, 1, pRoom->vnum, rch->coord->x, rch->coord->y, rch->map, -1, 0, 0);
      reset->serial = rch->serial;
      serial_list[reset->serial] = TRUE;
      for (obj = rch->first_carrying; obj; obj = obj->next_content)
      {
         if (obj->wear_loc == WEAR_NONE)
            add_obj_reset(pArea, 'G', obj, 1, 0);
         else
            add_obj_reset(pArea, 'E', obj, 1, obj->wear_loc);
      }
   }
   fdarea(rch, pArea->filename);
   return;
}
