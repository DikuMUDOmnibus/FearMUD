/************************************************************************/
/* mlkesl@stthomas.edu  =====>  Ascii Automapper utility                */
/* Let me know if you use this. Give a newbie some _credit_,            */
/* at least I'm not asking how to add classes...                        */
/* Also, if you fix something could ya send me mail about, thanks       */
/* PLEASE mail me if you use this or like it, that way I will keep it up*/
/************************************************************************/
/* MapArea ->   when given a room, ch, x, and y,...                     */
/*              this function will fill in values of map as it should   */
/* ShowMap ->   will simply spit out the contents of map array          */
/*              Would look much nicer if you built your own areas       */
/*              without all of the overlapping stock Rom has            */
/* do_map  ->   core function, takes map size as argument               */
/************************************************************************/
/* To install::                                                         */
/*      remove all occurances of "u1." (unless your exits are unioned)  */
/*      add do_map prototypes to interp.c and merc.h (or interp.h)      */
/*      customize the first switch with your own sectors (add road :)   */
/*      remove the color codes or change to suit your own color coding  */
/* Other stuff::                                                        */
/*      make a skill, call from do_move (only if ch is not in city etc) */
/*      allow players to actually make ITEM_MAP objects                 */
/*      change your areas to make them more suited to map code! :)      */
/*      I WILL be making MANY enhancements as they come but a lot of    */
/*        people want the code, and want it now :p                      */
/*      Kermit's page also has this snippet, mail me for any q's though */
/************************************************************************/
/* mlk update: map is now a 2 dimensional array of integers             */
/*      uses SECT_MAX for null                                  */
/*      uses SECT_MAX+1 for mazes or one ways to SECT_ENTER             */
/*      use the SECTOR numbers to represent sector types :)             */
/************************************************************************/
#include <ctype.h> /* for isalpha */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

#define MAX_MAP 160
#define MAX_MAP_DIR 4

int map[MAX_MAP][MAX_MAP];
int offsets[10][2] = { {-1, 0}, {0, 1}, {1, 0}, {0, -1}, {0, 0}, {0, 0}, {-1, 1}, {-1, -1}, {1, 1}, {1, -1} };
int moffsets[10][2] = { {-2, 0}, {0, 2}, {2, 0}, {0, -2}, {0, 0}, {0, 0}, {-2, 2}, {-2, -2}, {2, 2}, {2, -2} };

void show_char_to_char(CHAR_DATA * list, CHAR_DATA * ch);
void show_list_to_char(OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowN,  const int iDefaultAction);

/* used to format description, Thanks Kratas -- Xerves 11/99 */

char *format_string1(char *oldstring /*, bool fSpace */ )
{
   static char xbuf[MSL];

   char xbuf2[MSL];
   char *rdesc;
   int i = 0;
   int end_of_line;
   bool cap = TRUE;
   bool bFormat = TRUE;

   xbuf[0] = xbuf2[0] = '\0';

   i = 0;

   for (rdesc = oldstring; *rdesc; rdesc++)
   {
      if (*rdesc != '`')
      {
         if (bFormat)
         {
            if (*rdesc == '\n')
            {
               if (*(rdesc + 1) == '\r' && *(rdesc + 2) == ' ' && *(rdesc + 3) == '\n' && xbuf[i - 1] != '\r')
               {
                  xbuf[i] = '\n';
                  xbuf[i + 1] = '\r';
                  xbuf[i + 2] = '\n';
                  xbuf[i + 3] = '\r';
                  i += 4;
                  rdesc += 2;
               }
               else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == ' ' && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
               {
                  xbuf[i] = '\n';
                  xbuf[i + 1] = '\r';
                  i += 2;
               }
               else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == '\n' && xbuf[i - 1] != '\r')
               {
                  xbuf[i] = '\n';
                  xbuf[i + 1] = '\r';
                  xbuf[i + 2] = '\n';
                  xbuf[i + 3] = '\r';
                  i += 4;
                  rdesc += 1;
               }
               else if (*(rdesc + 1) == '\r' && *(rdesc + 2) == '\n' && xbuf[i - 1] == '\r')
               {
                  xbuf[i] = '\n';
                  xbuf[i + 1] = '\r';
                  i += 2;
               }
               else if (xbuf[i - 1] != ' ' && xbuf[i - 1] != '\r')
               {
                  xbuf[i] = ' ';
                  i++;
               }
            }
            else if (*rdesc == '\r') ;
            else if (*rdesc == 'i' && *(rdesc + 1) == '.' && *(rdesc + 2) == 'e' && *(rdesc + 3) == '.')
            {
               xbuf[i] = 'i';
               xbuf[i + 1] = '.';
               xbuf[i + 2] = 'e';
               xbuf[i + 3] = '.';
               i += 4;
               rdesc += 3;
            }
            else if (*rdesc == ' ')
            {
               if (xbuf[i - 1] != ' ')
               {
                  xbuf[i] = ' ';
                  i++;
               }
            }
            else if (*rdesc == ')')
            {
               if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' && (xbuf[i - 3] == '.' || xbuf[i - 3] == '?' || xbuf[i - 3] == '!'))
               {
                  xbuf[i - 2] = *rdesc;
                  xbuf[i - 1] = ' ';
                  xbuf[i] = ' ';
                  i++;
               }
               else if (xbuf[i - 1] == ' ' && (xbuf[i - 2] == ',' || xbuf[i - 2] == ';'))
               {

                  xbuf[i - 1] = *rdesc;
                  xbuf[i] = ' ';
                  i++;
               }
               else
               {
                  xbuf[i] = *rdesc;
                  i++;
               }
            }
            else if (*rdesc == ',' || *rdesc == ';')
            {
               if (xbuf[i - 1] == ' ')
               {
                  xbuf[i - 1] = *rdesc;
                  xbuf[i] = ' ';
                  i++;
               }
               else
               {
                  xbuf[i] = *rdesc;
                  if (*(rdesc + 1) != '\"')
                  {
                     xbuf[i + 1] = ' ';
                     i += 2;
                  }
                  else
                  {
                     xbuf[i + 1] = '\"';
                     xbuf[i + 2] = ' ';
                     i += 3;
                     rdesc++;
                  }
               }

            }
            else if (*rdesc == '.' || *rdesc == '?' || *rdesc == '!')
            {
               if (xbuf[i - 1] == ' ' && xbuf[i - 2] == ' ' && (xbuf[i - 3] == '.' || xbuf[i - 3] == '?' || xbuf[i - 3] == '!'))
               {
                  xbuf[i - 2] = *rdesc;
                  if (*(rdesc + 1) != '\"')
                  {
                     xbuf[i - 1] = ' ';
                     xbuf[i] = ' ';
                     i++;
                  }
                  else
                  {
                     xbuf[i - 1] = '\"';
                     xbuf[i] = ' ';
                     xbuf[i + 1] = ' ';
                     i += 2;
                     rdesc++;
                  }
               }
               else
               {
                  xbuf[i] = *rdesc;
                  if (*(rdesc + 1) != '\"')
                  {
                     xbuf[i + 1] = ' ';
                     xbuf[i + 2] = ' ';
                     i += 3;
                  }
                  else
                  {
                     xbuf[i + 1] = '\"';
                     xbuf[i + 2] = ' ';
                     xbuf[i + 3] = ' ';
                     i += 4;
                     rdesc++;
                  }
               }
               cap = TRUE;
            }
            else
            {
               xbuf[i] = *rdesc;
               if (cap)
               {
                  cap = FALSE;
                  xbuf[i] = UPPER(xbuf[i]);
               }
               i++;
            }
         }
         else
         {
            xbuf[i] = *rdesc;
            i++;
         }
      }
      else
      {
         if (*(rdesc + 1) == 'Z')
            bFormat = !bFormat;
         xbuf[i] = *rdesc;
         i++;
         rdesc++;
         xbuf[i] = *rdesc;
         i++;
      }
   }
   xbuf[i] = 0;
   strcpy(xbuf2, xbuf);


   rdesc = xbuf2;

   xbuf[0] = 0;

   for (;;)
   {
      end_of_line = 55;

      for (i = 0; i < end_of_line; i++)
      {
         if (*(rdesc + i) == '`')
         {
            end_of_line += 2;
            i++;
         }

         if (!*(rdesc + i))
            break;

         if (*(rdesc + i) == '\r')
            end_of_line = i;
      }
      if (i < end_of_line)
         break;
      if (*(rdesc + i - 1) != '\r')
      {
         for (i = (xbuf[0] ? (end_of_line - 1) : (end_of_line - 4)); i; i--)
         {
            if (*(rdesc + i) == ' ')
               break;
         }
         if (i)
         {
            *(rdesc + i) = 0;
            strcat(xbuf, rdesc);
            strcat(xbuf, "\n\r  ");
            rdesc += i + 1;
            while (*rdesc == ' ')
               rdesc++;
         }
         else
         {
            bug("Wrap_string: No spaces", 0);
            *(rdesc + (end_of_line - 2)) = 0;
            strcat(xbuf, rdesc);
            strcat(xbuf, "-\n\r  ");
            rdesc += end_of_line - 1;
         }
      }
      else
      {
         *(rdesc + i - 1) = 0;
         strcat(xbuf, rdesc);
         strcat(xbuf, "\r");
         rdesc += i;
         while (*rdesc == ' ')
            rdesc++;
      }
   }
   while (*(rdesc + i) && (*(rdesc + i) == ' ' || *(rdesc + i) == '\n' || *(rdesc + i) == '\r'))
      i--;
   *(rdesc + i + 1) = 0;
   strcat(xbuf, rdesc);
   if (xbuf[strlen(xbuf) - 2] != '\n')
      strcat(xbuf, "\n\r");

   return (xbuf);
}

void MapArea(ROOM_INDEX_DATA * room, CHAR_DATA * ch, int x, int y, int min, int max)
{
   ROOM_INDEX_DATA *prospect_room;
   EXIT_DATA *pexit;
   sh_int door;

/* marks the room as visited */
   if (room->first_person)
      map[x][y] = room->sector_type+10000;
   else
      map[x][y] = room->sector_type;
   
   for (pexit = room->first_exit; pexit; pexit = pexit->next)
   {
      if (pexit->to_room != NULL && !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_SET(pexit->exit_info, EX_HIDDEN))
      { /* if exit there */

         if ((x < min) || (y < min) || (x > max) || (y > max))
            return;
         door = pexit->vdir;
         prospect_room = pexit->to_room;

         if (pexit->vdir == 0 || pexit->vdir == 2) //north-south
            map[x + offsets[door][0]][y + offsets[door][1]] = SECT_MAX+2;
         else if (pexit->vdir == 1 || pexit->vdir == 3)//east/west
            map[x + offsets[door][0]][y + offsets[door][1]] = SECT_MAX+3;
         else if (door == 6 || door == 9)//ne sw
         {
            if (map[x + offsets[door][0]][y + offsets[door][1]] == SECT_MAX+6)
               ;
            else if (map[x + offsets[door][0]][y + offsets[door][1]] == SECT_MAX+5)
               map[x + offsets[door][0]][y + offsets[door][1]] = SECT_MAX+6;
            else  
               map[x + offsets[door][0]][y + offsets[door][1]] = SECT_MAX+4;
         }
         else if (door == 7 || door == 8)//nw se
         {
            if (map[x + offsets[door][0]][y + offsets[door][1]] == SECT_MAX+6)
               ;
            else if (map[x + offsets[door][0]][y + offsets[door][1]] == SECT_MAX+4)
               map[x + offsets[door][0]][y + offsets[door][1]] = SECT_MAX+6;
            else
               map[x + offsets[door][0]][y + offsets[door][1]] = SECT_MAX+5;
         }
      }
   }

   for (pexit = room->first_exit; pexit; pexit = pexit->next)
   {
      if (pexit->to_room != NULL && !IS_SET(pexit->exit_info, EX_CLOSED) && !IS_SET(pexit->exit_info, EX_HIDDEN))
      { /* if exit there */

         if ((x < min) || (y < min) || (x > max) || (y > max))
            return;
         door = pexit->vdir;
         prospect_room = pexit->to_room;

         if ((prospect_room->sector_type == SECT_CITY) || (prospect_room->sector_type == SECT_INSIDE) || (prospect_room->sector_type == SECT_ENTER))
         { /* players cant see past these */
            if (prospect_room->first_person)
               map[x + moffsets[door][0]][y + moffsets[door][1]] = prospect_room->sector_type+10000;
            else
               map[x + moffsets[door][0]][y + moffsets[door][1]] = prospect_room->sector_type;

         }

         if (IS_NPC(ch) || (!xIS_SET(ch->act, PLR_HOLYLIGHT)) || (!IS_IMMORTAL(ch)))
         {
            if ((prospect_room->sector_type == SECT_MOUNTAIN))
            {
               if (prospect_room->first_person)
                  map[x + moffsets[door][0]][y + moffsets[door][1]] = prospect_room->sector_type+10000;
               else
                  map[x + moffsets[door][0]][y + moffsets[door][1]] = prospect_room->sector_type;
            }
         }
         if (map[x + moffsets[door][0]][y + moffsets[door][1]] == SECT_MAX)
         {
            MapArea(pexit->to_room, ch, x + moffsets[door][0], y + moffsets[door][1], min, max);
         }

      } /* end if exit there */
   }
   return;
}

/* mlk :: shows a map, specified by size */
void ShowMap(CHAR_DATA * ch, int min, int max)
{
   int x, y;
   int reset = 0;

   for (x = min; x < max; ++x)
   { /* every row */
      for (y = min; y < max; ++y)
      { /* every column */
         if (map[x][y] >= 10000)
         {
            map[x][y] -= 10000;
            send_to_char("^c", ch);
            reset = 1;
         }
         else
            reset = 0;
         if ((y == min) || (map[x][y - 1] != map[x][y]))
         {
            switch (map[x][y])
            {
               case SECT_MAX:
                  send_to_char(" ", ch);
                  break;
               case SECT_INSIDE:
                  send_to_char("&G&W%", ch);
                  break;
               case SECT_CITY:
                  send_to_char("&G&W#", ch);
                  break;
               case SECT_ROAD:
                  send_to_char("&c&w+", ch);
                  break;
               case SECT_PATH:
                  send_to_char("&c&g+", ch);
                  break;
               case SECT_FIELD:
                  send_to_char("&G\"", ch);
                  break;
               case SECT_FOREST:
                  send_to_char("&G@", ch);
                  break;
               case SECT_HILLS:
                  send_to_char("&G^^", ch);
                  break;
               case SECT_MOUNTAIN:
                  send_to_char("&O^^", ch);
                  break;
               case SECT_MINEGOLD:
                  send_to_char("&Y^^", ch);
                  break;
               case SECT_MINEIRON:
                  send_to_char("&R^^", ch);
                  break;
               case SECT_HCORN:
                  send_to_char("&Y\"", ch);
                  break;
               case SECT_HGRAIN:
                  send_to_char("&c&w\"", ch);
                  break;
               case SECT_SCORN:
                  send_to_char("&O\"", ch);
                  break;
               case SECT_NCORN:
                  send_to_char("&G&W\"", ch);
                  break;
               case SECT_SGRAIN:
                  send_to_char("&z\"", ch);
                  break;
               case SECT_NGRAIN:
                  send_to_char("&G\"", ch);
                  break;
               case SECT_SGOLD:
                  send_to_char("&c&w^^", ch);
                  break;
               case SECT_NGOLD:
                  send_to_char("&G&W^^", ch);
                  break;
               case SECT_SIRON:
                  send_to_char("&B^^", ch);
                  break;
               case SECT_NIRON:
                  send_to_char("&z^^", ch);
                  break;
               case SECT_STREE:
                  send_to_char("&g@", ch);
                  break;
               case SECT_NTREE:
                  send_to_char("&c&w@", ch);
                  break;
               case SECT_WATER_SWIM:
                  send_to_char("&w&C~", ch);
                  break;
               case SECT_WATER_NOSWIM:
                  send_to_char("&b:", ch);
                  break;
               case SECT_UNDERWATER:
                  send_to_char("&B-", ch);
                  break;
               case SECT_AIR:
                  send_to_char("&C%", ch);
                  break;
               case SECT_DESERT:
                  send_to_char("&Y=", ch);
                  break;
               case SECT_DUNNO:
                  send_to_char("&OX", ch);
                  break;
               case SECT_OCEANFLOOR:
                  send_to_char("&b-", ch);
                  break;
               case SECT_UNDERGROUND:
                  send_to_char("&O#", ch);
                  break;
               case (SECT_MAX + 1):
                  send_to_char("&O?", ch);
                  break;
               case (SECT_MAX + 2):
                  send_to_char("&c&w|", ch);
                  break;
               case (SECT_MAX + 3):
                  send_to_char("&c&w-", ch);
                  break;
               case (SECT_MAX + 4):
                  send_to_char("&c&w/", ch);
                  break;
               case (SECT_MAX + 5):
                  send_to_char("&c&w\\", ch);
                  break;
               case (SECT_MAX + 6):
                  send_to_char("&c&wX", ch);
                  break;
               default:
                  send_to_char("&R*", ch);
            } /* end switch1 */
         }
         else
         {
            switch (map[x][y])
            {
               case SECT_MAX:
                  send_to_char(" ", ch);
                  break;
               case SECT_INSIDE:
                  send_to_char("%", ch);
                  break;
               case SECT_CITY:
                  send_to_char("#", ch);
                  break;
               case SECT_ROAD:
                  send_to_char("+", ch);
                  break;
               case SECT_PATH:
                  send_to_char("+", ch);
                  break;
               case SECT_FIELD:
                  send_to_char("\"", ch);
                  break;
               case SECT_FOREST:
                  send_to_char("@", ch);
                  break;
               case SECT_HILLS:
                  send_to_char("^^", ch);
                  break;
               case SECT_MOUNTAIN:
                  send_to_char("^^", ch);
                  break;
               case SECT_MINEGOLD:
                  send_to_char("^^", ch);
                  break;
               case SECT_MINEIRON:
                  send_to_char("^^", ch);
                  break;
               case SECT_HCORN:
                  send_to_char("\"", ch);
                  break;
               case SECT_HGRAIN:
                  send_to_char("\"", ch);
                  break;
               case SECT_SCORN:
                  send_to_char("\"", ch);
                  break;
               case SECT_NCORN:
                  send_to_char("\"", ch);
                  break;
               case SECT_SGRAIN:
                  send_to_char("\"", ch);
                  break;
               case SECT_NGRAIN:
                  send_to_char("\"", ch);
                  break;
               case SECT_SGOLD:
                  send_to_char("^^", ch);
                  break;
               case SECT_NGOLD:
                  send_to_char("^^", ch);
                  break;
               case SECT_SIRON:
                  send_to_char("^^", ch);
                  break;
               case SECT_NIRON:
                  send_to_char("^^", ch);
                  break;
               case SECT_STREE:
                  send_to_char("@", ch);
                  break;
               case SECT_NTREE:
                  send_to_char("@", ch);
                  break;
               case SECT_WATER_SWIM:
                  send_to_char("~", ch);
                  break;
               case SECT_WATER_NOSWIM:
                  send_to_char(":", ch);
                  break;
               case SECT_UNDERWATER:
                  send_to_char("-", ch);
                  break;
               case SECT_AIR:
                  send_to_char("%", ch);
                  break;
               case SECT_DESERT:
                  send_to_char("=", ch);
                  break;
               case SECT_DUNNO:
                  send_to_char("X", ch);
                  break;
               case SECT_OCEANFLOOR:
                  send_to_char("-", ch);
                  break;
               case SECT_UNDERGROUND:
                  send_to_char("#", ch);
                  break;
               case (SECT_MAX + 1):
                  send_to_char("?", ch);
                  break;
               case (SECT_MAX + 2):
                  send_to_char("|", ch);
                  break;
               case (SECT_MAX + 3):
                  send_to_char("-", ch);
                  break;
               case (SECT_MAX + 4):
                  send_to_char("/", ch);
                  break;
               case (SECT_MAX + 5):
                  send_to_char("\\", ch);
                  break;
               case (SECT_MAX + 6):
                  send_to_char("X", ch);
                  break;
               default:
                  send_to_char("*", ch);
            } /* end switch2 */
         }
         if (reset)
            send_to_char("^x", ch);
      } /* every column */

      send_to_char("\n\r", ch);
   } /*every row */

   return;
}

/* mlk :: shows map compacted, specified by size */
void ShowHalfMap(CHAR_DATA * ch, int min, int max)
{
   int x, y;
   int reset = 0;

   for (x = min; x < max; x += 2)
   { /* every row */
      for (y = min; y < max; y += 2)
      { /* every column */

/* mlk prioritizes*/
         if (map[x][y - 1] == SECT_ROAD)
            map[x][y] = SECT_ROAD;
         if (map[x][y - 1] == SECT_ENTER)
            map[x][y] = SECT_ENTER;
         if (map[x][y] >= 10000)
         {
            map[x][y] -= 10000;
            send_to_char("^c", ch);
            reset = 1;
         }
         else
            reset = 0;

         if ((y == min) || (map[x][y - 2] != map[x][y]))
         {
            switch (map[x][y])
            {
               case SECT_MAX:
                  send_to_char(" ", ch);
                  break;
               case SECT_INSIDE:
                  send_to_char("&G&W%", ch);
                  break;
               case SECT_CITY:
                  send_to_char("&G&W#", ch);
                  break;
               case SECT_ROAD:
                  send_to_char("&c&w+", ch);
                  break;
               case SECT_PATH:
                  send_to_char("&c&g+", ch);
                  break;
               case SECT_FIELD:
                  send_to_char("&G\"", ch);
                  break;
               case SECT_FOREST:
                  send_to_char("&G@", ch);
                  break;
               case SECT_HILLS:
                  send_to_char("&G^^", ch);
                  break;
               case SECT_MOUNTAIN:
                  send_to_char("&O^^", ch);
                  break;
               case SECT_MINEGOLD:
                  send_to_char("&Y^^", ch);
                  break;
               case SECT_MINEIRON:
                  send_to_char("&R^^", ch);
                  break;
               case SECT_HCORN:
                  send_to_char("&Y\"", ch);
                  break;
               case SECT_HGRAIN:
                  send_to_char("&c&w\"", ch);
                  break;
               case SECT_SCORN:
                  send_to_char("&O\"", ch);
                  break;
               case SECT_NCORN:
                  send_to_char("&G&W\"", ch);
                  break;
               case SECT_SGRAIN:
                  send_to_char("&z\"", ch);
                  break;
               case SECT_NGRAIN:
                  send_to_char("&G\"", ch);
                  break;
               case SECT_SGOLD:
                  send_to_char("&c&w^^", ch);
                  break;
               case SECT_NGOLD:
                  send_to_char("&G&W^^", ch);
                  break;
               case SECT_SIRON:
                  send_to_char("&B^^", ch);
                  break;
               case SECT_NIRON:
                  send_to_char("&z^^", ch);
                  break;
               case SECT_STREE:
                  send_to_char("&g@", ch);
                  break;
               case SECT_NTREE:
                  send_to_char("&c&w@", ch);
                  break;
               case SECT_WATER_SWIM:
                  send_to_char("&w&C~", ch);
                  break;
               case SECT_WATER_NOSWIM:
                  send_to_char("&b:", ch);
                  break;
               case SECT_UNDERWATER:
                  send_to_char("&B-", ch);
                  break;
               case SECT_AIR:
                  send_to_char("&C%", ch);
                  break;
               case SECT_DESERT:
                  send_to_char("&Y=", ch);
                  break;
               case SECT_DUNNO:
                  send_to_char("&OX", ch);
                  break;
               case SECT_OCEANFLOOR:
                  send_to_char("&b-", ch);
                  break;
               case SECT_UNDERGROUND:
                  send_to_char("&O#", ch);
                  break;
               case (SECT_MAX + 1):
                  send_to_char("&O?", ch);
                  break;
               case (SECT_MAX + 2):
                  send_to_char("&c&w|", ch);
                  break;
               case (SECT_MAX + 3):
                  send_to_char("&c&w-", ch);
                  break;
               case (SECT_MAX + 4):
                  send_to_char("&c&w/", ch);
                  break;
               case (SECT_MAX + 5):
                  send_to_char("&c&w\\", ch);
                  break;
               case (SECT_MAX + 6):
                  send_to_char("&c&wX", ch);
                  break;
               default:
                  send_to_char("&R*", ch);
            } /* end switch1 */
         }
         else
         {
            switch (map[x][y])
            {
               case SECT_ROAD:
                  send_to_char("+", ch);
                  break;
               case SECT_PATH:
                  send_to_char("+", ch);
                  break;
               case SECT_MAX:
                  send_to_char(" ", ch);
                  break;
               case SECT_INSIDE:
                  send_to_char("%", ch);
                  break;
               case SECT_CITY:
                  send_to_char("#", ch);
                  break;
               case SECT_FIELD:
                  send_to_char("\"", ch);
                  break;
               case SECT_FOREST:
                  send_to_char("@", ch);
                  break;
               case SECT_HILLS:
                  send_to_char("^^", ch);
                  break;
               case SECT_MOUNTAIN:
                  send_to_char("^^", ch);
                  break;
               case SECT_MINEGOLD:
                  send_to_char("^^", ch);
                  break;
               case SECT_MINEIRON:
                  send_to_char("^^", ch);
                  break;
               case SECT_HCORN:
                  send_to_char("\"", ch);
                  break;
               case SECT_HGRAIN:
                  send_to_char("\"", ch);
                  break;
               case SECT_SCORN:
                  send_to_char("\"", ch);
                  break;
               case SECT_NCORN:
                  send_to_char("\"", ch);
                  break;
               case SECT_SGRAIN:
                  send_to_char("\"", ch);
                  break;
               case SECT_NGRAIN:
                  send_to_char("\"", ch);
                  break;
               case SECT_SGOLD:
                  send_to_char("^^", ch);
                  break;
               case SECT_NGOLD:
                  send_to_char("^^", ch);
                  break;
               case SECT_SIRON:
                  send_to_char("^^", ch);
                  break;
               case SECT_NIRON:
                  send_to_char("^^", ch);
                  break;
               case SECT_STREE:
                  send_to_char("@", ch);
                  break;
               case SECT_NTREE:
                  send_to_char("@", ch);
                  break;
               case SECT_WATER_SWIM:
                  send_to_char("~", ch);
                  break;
               case SECT_WATER_NOSWIM:
                  send_to_char(":", ch);
                  break;
               case SECT_UNDERWATER:
                  send_to_char("-", ch);
                  break;
               case SECT_AIR:
                  send_to_char("%", ch);
                  break;
               case SECT_DESERT:
                  send_to_char("=", ch);
                  break;
               case SECT_DUNNO:
                  send_to_char("X", ch);
                  break;
               case SECT_OCEANFLOOR:
                  send_to_char("-", ch);
                  break;
               case SECT_UNDERGROUND:
                  send_to_char("#", ch);
                  break;
               case (SECT_MAX + 1):
                  send_to_char("?", ch);
                  break;
               case (SECT_MAX + 2):
                  send_to_char("|", ch);
                  break;
               case (SECT_MAX + 3):
                  send_to_char("-", ch);
                  break;
               case (SECT_MAX + 4):
                  send_to_char("/", ch);
                  break;
               case (SECT_MAX + 5):
                  send_to_char("\\", ch);
                  break;
               case (SECT_MAX + 6):
                  send_to_char("X", ch);
                  break;
               default:
                  send_to_char("*", ch);
            } /* end switch2 */
         }
         if (reset)
            send_to_char("^x", ch);
      } /* every column */

      send_to_char("\n\r", ch);
   } /*every row */

   return;
}

/*printing function*/
void do_printmap(CHAR_DATA * ch, char *argument)
{
   int x, y, min = -1, max = MAX_MAP - 1;
   FILE *fp;

   if (strcmp(ch->name, "Kroudar"))
   {
      send_to_char("Curiousity is a disease...", ch);
      return;
   }

   for (x = 0; x < MAX_MAP; ++x)
      for (y = 0; y < MAX_MAP; ++y)
         map[x][y] = SECT_MAX;

   MapArea(ch->in_room, ch, 80, 80, min, max);

   fclose(fpReserve);

   fp = fopen("WILDERNESS_MAP", "w");
   for (x = min; x < max; ++x)
   { /* every row */
      for (y = min; y < max; ++y)
      { /* every column */
         switch (map[x][y])
         {
            case SECT_ROAD:
               send_to_char("+", ch);
               break;
            case SECT_PATH:
               send_to_char("+", ch);
               break;
            case SECT_MAX:
               send_to_char(" ", ch);
               break;
            case SECT_INSIDE:
               send_to_char("%", ch);
               break;
            case SECT_CITY:
               send_to_char("#", ch);
               break;
            case SECT_FIELD:
               send_to_char("\"", ch);
               break;
            case SECT_FOREST:
               send_to_char("@", ch);
               break;
            case SECT_HILLS:
               send_to_char("^^", ch);
               break;
            case SECT_MOUNTAIN:
               send_to_char("^^", ch);
               break;
            case SECT_MINEGOLD:
               send_to_char("^^", ch);
               break;
            case SECT_MINEIRON:
               send_to_char("^^", ch);
               break;
            case SECT_HCORN:
               send_to_char("\"", ch);
               break;
            case SECT_HGRAIN:
               send_to_char("\"", ch);
               break;
            case SECT_SCORN:
               send_to_char("\"", ch);
               break;
            case SECT_NCORN:
               send_to_char("\"", ch);
               break;
            case SECT_SGRAIN:
               send_to_char("\"", ch);
               break;
            case SECT_NGRAIN:
               send_to_char("\"", ch);
               break;
            case SECT_SGOLD:
               send_to_char("^^", ch);
               break;
            case SECT_NGOLD:
               send_to_char("^^", ch);
               break;
            case SECT_SIRON:
               send_to_char("^^", ch);
               break;
            case SECT_NIRON:
               send_to_char("^^", ch);
               break;
            case SECT_STREE:
               send_to_char("@", ch);
               break;
            case SECT_NTREE:
               send_to_char("@", ch);
               break;
            case SECT_WATER_SWIM:
               send_to_char("~", ch);
               break;
            case SECT_WATER_NOSWIM:
               send_to_char(":", ch);
               break;
            case SECT_UNDERWATER:
               send_to_char("-", ch);
               break;
            case SECT_AIR:
               send_to_char("%", ch);
               break;
            case SECT_DESERT:
               send_to_char("=", ch);
               break;
            case SECT_DUNNO:
               send_to_char("X", ch);
               break;
            case SECT_OCEANFLOOR:
               send_to_char("-", ch);
               break;
            case SECT_UNDERGROUND:
               send_to_char("#", ch);
               break;
            case (SECT_MAX + 1):
               send_to_char("?", ch);
               break;
            case (SECT_MAX + 2):
               send_to_char("|", ch);
               break;
            case (SECT_MAX + 3):
               send_to_char("-", ch);
               break;
            case (SECT_MAX + 4):
                  send_to_char("/", ch);
                  break;
               case (SECT_MAX + 5):
                  send_to_char("\\", ch);
                  break;
               case (SECT_MAX + 6):
                  send_to_char("X", ch);
                  break;
            default:
               send_to_char("*", ch);
         } /* end switch2 */
      } /* every column */

      fprintf(fp, "\n");
   } /*every row */

   fclose(fp);
   fpReserve = fopen(NULL_FILE, "r");

   return;
}

/* will put a small map with current room desc and title */
void ShowRoom(CHAR_DATA * ch, int min, int max)
{
   int x, y, str_pos = 0, desc_pos = 0, start;
   char buf[500];
   char desc[500];
   char line[100];
   int reset = 0;

   strcpy(desc, ch->in_room->description);

   if ((get_trust(ch) < LEVEL_IMMORTAL) || (IS_NPC(ch)))
   {
      map[min][min] = SECT_MAX;
      map[max - 1][max - 1] = SECT_MAX; /* mlk :: rounds edges */
      map[min][max - 1] = SECT_MAX;
      map[max - 1][min] = SECT_MAX;
   }

   for (x = min; x < max - 1; ++x)
   { /* every row */
      for (y = min; y < max - 1; ++y)
      { /* every column */
         if (map[x][y] >= 10000)
         {
            map[x][y] -= 10000;
            send_to_char("^c", ch);
            reset = 1;
         }
         else
            reset = 0;
         if ((y == min) || (map[x][y - 1] != map[x][y]))
         {
            switch (map[x][y])
            {
               case SECT_MAX:
                  send_to_char("&x ", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_INSIDE:
                  send_to_char("&G&W%", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_CITY:
                  send_to_char("&G&W#", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_ROAD:
                  send_to_char("&c&w+", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_PATH:
                  send_to_char("&c&g+", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_FIELD:
                  send_to_char("&G\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_FOREST:
                  send_to_char("&G@", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_HILLS:
                  send_to_char("&G^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_MOUNTAIN:
                  send_to_char("&O^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_MINEGOLD:
                  send_to_char("&Y^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_MINEIRON:
                  send_to_char("&R^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_HCORN:
                  send_to_char("&Y\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_HGRAIN:
                  send_to_char("&c&w\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SCORN:
                  send_to_char("&O\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NCORN:
                  send_to_char("&G&W\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SGRAIN:
                  send_to_char("&z\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NGRAIN:
                  send_to_char("&G\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SGOLD:
                  send_to_char("&c&w^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NGOLD:
                  send_to_char("&G&W^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SIRON:
                  send_to_char("&B^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NIRON:
                  send_to_char("&z^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_STREE:
                  send_to_char("&g@", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NTREE:
                  send_to_char("&c&w@", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_WATER_SWIM:
                  send_to_char("&w&C~", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_WATER_NOSWIM:
                  send_to_char("&b:", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_UNDERWATER:
                  send_to_char("&B-", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_AIR:
                  send_to_char("&C%", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_DESERT:
                  send_to_char("&Y=", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_DUNNO:
                  send_to_char("&OX", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_OCEANFLOOR:
                  send_to_char("&b-", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_UNDERGROUND:
                  send_to_char("&O#", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 1):
                  send_to_char("&O?", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 2):
                  send_to_char("&c&w|", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 3):
                  send_to_char("&c&w-", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 4):
                  send_to_char("&c&w/", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 5):
                  send_to_char("&c&w\\", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 6):
                  send_to_char("&c&wX", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               default:
                  send_to_char("&R*", ch);
                  set_char_color(AT_RMDESC, ch);
            } /* end switch1 */
         }
         else
         {
            switch (map[x][y])
            {
               case SECT_MAX:
                  send_to_char("&x ", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_INSIDE:
                  send_to_char("&G&W%", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_CITY:
                  send_to_char("&G&W#", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_ROAD:
                  send_to_char("&c&w+", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_PATH:
                  send_to_char("&c&g+", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_FIELD:
                  send_to_char("&G\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_FOREST:
                  send_to_char("&G@", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_HILLS:
                  send_to_char("&G^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_MOUNTAIN:
                  send_to_char("&O^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_MINEGOLD:
                  send_to_char("&Y^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_MINEIRON:
                  send_to_char("&R^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_HCORN:
                  send_to_char("&Y\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_HGRAIN:
                  send_to_char("&c&w\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SCORN:
                  send_to_char("&O\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NCORN:
                  send_to_char("&G&W\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SGRAIN:
                  send_to_char("&z\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NGRAIN:
                  send_to_char("&G\"", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SGOLD:
                  send_to_char("&c&w^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NGOLD:
                  send_to_char("&G&W^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_SIRON:
                  send_to_char("&B^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NIRON:
                  send_to_char("&z^^", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_STREE:
                  send_to_char("&g@", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_NTREE:
                  send_to_char("&c&w@", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_WATER_SWIM:
                  send_to_char("~", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_WATER_NOSWIM:
                  send_to_char("&b:", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_UNDERWATER:
                  send_to_char("&B-", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_AIR:
                  send_to_char("&C%", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_DESERT:
                  send_to_char("&Y=", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_DUNNO:
                  send_to_char("&OX", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_OCEANFLOOR:
                  send_to_char("&b-", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case SECT_UNDERGROUND:
                  send_to_char("&O#", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 1):
                  send_to_char("&O?", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 2):
                  send_to_char("&c&w|", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 3):
                  send_to_char("&c&w-", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 4):
                  send_to_char("&c&w/", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 5):
                  send_to_char("&c&w\\", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               case (SECT_MAX + 6):
                  send_to_char("&c&wX", ch);
                  set_char_color(AT_RMDESC, ch);
                  break;
               default:
                  send_to_char("&R*", ch);
                  set_char_color(AT_RMDESC, ch);
            } /* every column */
         }
         if (reset)
            send_to_char("^x", ch);
      } /* every column */

      if (x == min)
      {
         sprintf(buf, "   %s", ch->in_room->name);
         set_char_color(AT_RMNAME, ch);
         send_to_char(buf, ch);

/* mlk :: no brief in wilderness, ascii map is automatic, autoexits off too
        if (  !IS_NPC(ch) && IS_SET(ch->act, PLR_AUTOEXIT)  ) {
        send_to_char( " {b",ch);do_exits( ch, "auto" );send_to_char( "{x",ch);
        } */

         if (IS_IMMORTAL(ch) && (IS_NPC(ch) || xIS_SET(ch->act, PLR_HOLYLIGHT)))
         { /* for showing certain people room vnum */
            sprintf(buf, " [Room %d Quadrant %d]", ch->in_room->vnum, ch->in_room->quad);
            send_to_char(buf, ch);
         } 
      }
      else
      {
         start = str_pos;
         for (desc_pos = desc_pos; desc[desc_pos] != '\0'; desc_pos++)
         {
            if (desc[desc_pos] == '\n')
            {
               line[str_pos - start] = '\0';
               str_pos += 3;
               desc_pos += 2;
               break;
            }
            else if (desc[desc_pos] == '\r')
            {
               line[str_pos - start] = '\0';
               str_pos += 2;
               break;
            }
            else
            {
               line[str_pos - start] = desc[desc_pos];
               str_pos += 1;
            }
         }
/* set the final character to \0 for descs that are longer than 5 lines? */
         line[str_pos - start] = '\0'; /* best way to clear string? */
/* maybe do strcpy(line,"  ");  instead? */
/* not needed here because the for loops stop at \0 ?? */
         if (x == min + 1)
            send_to_char("  ", ch);
         send_to_char("   ", ch);
         send_to_char(line, ch);

      } /*else */

      send_to_char("\n\r", ch);
   } /*every row */
   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOEXIT))
         do_exits(ch, "auto");
   send_to_char("\n\r", ch); /* puts a line between contents/people */
   show_list_to_char(ch->in_room->first_content, ch, FALSE, FALSE, eItemGet);
   show_char_to_char(ch->in_room->first_person, ch);
   return;
}

void do_map(CHAR_DATA * ch, char *argument)
{
   int size, center, x, y, min, max;
   char arg1[10];

   one_argument(argument, arg1);
   size = atoi(arg1);

   size = URANGE(6, size, MAX_MAP);
   center = MAX_MAP / 2 - 1;

   min = MAX_MAP / 2 - size / 2;
   max = MAX_MAP / 2 + size / 2;

   for (x = 0; x < MAX_MAP; ++x)
      for (y = 0; y < MAX_MAP; ++y)
         map[x][y] = SECT_MAX;

   /* starts the mapping with the center room */
   MapArea(ch->in_room, ch, center, center, min - 5, max + 5);

   /* marks the center, where ch is */
   map[center][center] = SECT_MAX + 10; /* can be any number above SECT_MAX+3   */
   /* switch default will print out the *  */

   if ((!IS_IMMORTAL(ch)) || (IS_NPC(ch)))
   {
      if (!xIS_SET(ch->in_room->room_flags, ROOM_WILDERNESS))
      {
         send_to_char("You can not do that here.\n\r", ch);
         return;
      }
      if (!IS_NPC(ch) && !xIS_SET(ch->act, PLR_HOLYLIGHT) && !IS_AFFECTED(ch, AFF_INFRARED) && !IS_AFFECTED(ch, AFF_TRUESIGHT) && room_is_dark(ch->in_room))
      {
         send_to_char("It is pitch black at night...\n\r", ch);
         return;
      }
      else
      {
         ShowRoom(ch, MAX_MAP / 2 - 7, MAX_MAP / 2 + 7);
         return;
      }
   }
   /* mortals not in city, enter or inside will always get a ShowRoom */
   if (IS_IMMORTAL(ch))
   {

      if (arg1[0] == '\0')
      {
         min = MAX_MAP / 2 - 7;
         max = MAX_MAP / 2 + 7;
         ShowRoom(ch, min, max);
      }
      else
         ShowMap(ch, min, max);
      return;
   }

   send_to_char("Huh?\n\r", ch);
   return;
}

void do_smallmap(CHAR_DATA * ch, char *argument)
{
   int size, center, x, y, min, max;
   char arg1[10];

   one_argument(argument, arg1);
   size = atoi(arg1);

   size = URANGE(6, size, MAX_MAP);
   center = MAX_MAP / 2;

   min = MAX_MAP / 2 - size / 2;
   max = MAX_MAP / 2 + size / 2;

   for (x = 0; x < MAX_MAP; ++x)
      for (y = 0; y < MAX_MAP; ++y)
         map[x][y] = SECT_MAX;

/* starts the mapping with the center room */
   MapArea(ch->in_room, ch, center, center, min, max);

/* marks the center, where ch is */
   map[center][center] = SECT_MAX + 10; /* can be any number above SECT_MAX+3   */
   /* switch default will print out the *  */

   if (IS_IMMORTAL(ch))
   {
      ShowHalfMap(ch, min, max);
      return;
   }

   send_to_char("Huh?\n\r", ch);
   return;
}

/* mlk :: pass it (SECTOR_XXX,"") and get back the sector ascii
          in a roleplaying format of course, not mountain_snow etc */

/* second object is for capitalized first word.  0 is no, 1 is yes -- Xerves 11/99 */
char *get_sector_name(int sector, int cap)
{
   char *sector_name;

   if (cap == 0)
      sector_name = "movement";
   else
      sector_name = "Movement";

   if (cap == 0)
   {
      switch (sector)
      {
         case SECT_FOREST:
            sector_name = "some trees";
            break;
         case SECT_FIELD:
            sector_name = "a field";
            break;
         case SECT_HILLS:
            sector_name = "some rolling hills";
            break;
         case SECT_ROAD:
            sector_name = "a road";
            break;
         case SECT_WATER_SWIM:
            sector_name = "shallow water";
            break;
         case SECT_WATER_NOSWIM:
            sector_name = "deep water";
            break;
         case SECT_UNDERWATER:
            sector_name = "an underwater area";
            break;
         case SECT_DUNNO:
            sector_name = "a strange area";
            break;
         case SECT_OCEANFLOOR:
            sector_name = "the ocean floor";
            break;
         case SECT_UNDERGROUND:
            sector_name = "an underground tunnel";
            break;
         case SECT_AIR:
            sector_name = "the sky";
            break;
         case SECT_DESERT:
            sector_name = "a lot of sand";
            break;
         case SECT_MOUNTAIN:
            sector_name = "some mountainous terrain";
            break;
         case SECT_MINEGOLD:
            sector_name = "a deep gold mine";
            break;
         case SECT_MINEIRON:
            sector_name = "a deep iron mine";
            break;
         case SECT_HCORN:
            sector_name = "a field of corn";
            break;
         case SECT_HGRAIN:
            sector_name = "a field of grain";
            break;
         case SECT_SCORN:
            sector_name = "a partially harvested field of corn";
            break;
         case SECT_NCORN:
            sector_name = "a completely harvested field of corn";
            break;
         case SECT_SGRAIN:
            sector_name = "a partially harvested field of grain";
            break;
         case SECT_NGRAIN:
            sector_name = "a completely harvested field of grain";
            break;
         case SECT_SGOLD:
            sector_name = "a partially extracted gold mine";
            break;
         case SECT_NGOLD:
            sector_name = "an empty gold mine";
            break;
         case SECT_SIRON:
            sector_name = "an prtially extracted iron mine";
            break;
         case SECT_NIRON:
            sector_name = "an empty iron mine";
            break;
         case SECT_STREE:
            sector_name = "a partially cleared forest";
            break;
         case SECT_NTREE:
            sector_name = "a completely cleared forest";
            break;
         case SECT_INSIDE:
            sector_name = "movement";
            break;
         case SECT_CITY:
            sector_name = "movement";
            break;
         case SECT_ENTER:
            sector_name = "movement";
            break;
      } /*switch1 */
   }
   else
   {
      switch (sector)
      {
         case SECT_FOREST:
            sector_name = "Some trees";
            break;
         case SECT_FIELD:
            sector_name = "A field";
            break;
         case SECT_HILLS:
            sector_name = "Some rolling hills";
            break;
         case SECT_ROAD:
            sector_name = "A road";
            break;
         case SECT_WATER_SWIM:
            sector_name = "Shallow water";
            break;
         case SECT_WATER_NOSWIM:
            sector_name = "Deep water";
            break;
         case SECT_UNDERWATER:
            sector_name = "An underwater area";
            break;
         case SECT_DUNNO:
            sector_name = "A strange area";
            break;
         case SECT_OCEANFLOOR:
            sector_name = "The ocean floor";
            break;
         case SECT_UNDERGROUND:
            sector_name = "An underground tunnel";
            break;
         case SECT_AIR:
            sector_name = "The sky";
            break;
         case SECT_DESERT:
            sector_name = "A lot of sand";
            break;
         case SECT_MOUNTAIN:
            sector_name = "Some mountainous terrain";
            break;
         case SECT_MINEGOLD:
            sector_name = "A deep gold mine";
            break;
         case SECT_MINEIRON:
            sector_name = "A deep iron mine";
            break;
         case SECT_HCORN:
            sector_name = "A field of corn";
            break;
         case SECT_HGRAIN:
            sector_name = "A field of grain";
            break;
         case SECT_SCORN:
            sector_name = "A partially harvested field of corn";
            break;
         case SECT_NCORN:
            sector_name = "A completely harvested field of corn";
            break;
         case SECT_SGRAIN:
            sector_name = "A partially harvested field of grain";
            break;
         case SECT_NGRAIN:
            sector_name = "A completely harvested field of grain";
            break;
         case SECT_SGOLD:
            sector_name = "A partially extracted gold mine";
            break;
         case SECT_NGOLD:
            sector_name = "An empty gold mine";
            break;
         case SECT_SIRON:
            sector_name = "An prtially extracted iron mine";
            break;
         case SECT_NIRON:
            sector_name = "An empty iron mine";
            break;
         case SECT_STREE:
            sector_name = "A partially cleared forest";
            break;
         case SECT_INSIDE:
            sector_name = "Movement";
            break;
         case SECT_CITY:
            sector_name = "Movement";
            break;
         case SECT_ENTER:
            sector_name = "Movement";
            break;
      } /*switch1 */
   }
   return (strdup(sector_name));
}

/* mlk ::
        when given (int array[5]) with vnum in [0], it will return
1       north sector_type,
2       east sector_type,
3       south sector_type,
4       east_sector_type &
5       number of exits leading to rooms of the same sector
 */
int *get_exit_sectors(int *exit_sectors, CHAR_DATA * ch)
{
   ROOM_INDEX_DATA *room;
   EXIT_DATA *xit;

   room = get_room_index(exit_sectors[0]);
   exit_sectors[0] = -1;
   exit_sectors[1] = -1;
   exit_sectors[2] = -1;
   exit_sectors[3] = -1;
/* Above are directions, below is used to count the number of exits -- Xerves */
   exit_sectors[4] = 0;

   for (xit = room->first_exit; xit; xit = xit->next)
   { /* cycles through for each exit */

      if (xit->vdir > 3)
      {
         bug("asciimap.c: get_exit_sectors: bad direction in wilderness", 0);
         continue;
      }
      exit_sectors[xit->vdir] = xit->to_room->sector_type;
      if (xit->to_room->sector_type == room->sector_type)
         exit_sectors[4] += 1;
   }

   return (exit_sectors);
}

/* will assign default values for NAME and DESC of wilderness */
// NO LONGER USED, DO NOT ADD BACK IN
void do_setwilderness(CHAR_DATA * ch, char *argument)
{
   char arg1[10], name[50], desc[300], desc2[300], buf[MSL];
   ROOM_INDEX_DATA *room;
   int vnum, exit[5], exitsum;


/*if (strcmp(ch->name, "Kroudar") ) {
   send_to_char("There are few beings who can do that.\n\r",ch);return;
}*/

   one_argument(argument, arg1);
   vnum = atoi(arg1);

   if (argument[0] == '\0')
   { /* for immortal command */
      vnum = (ch->in_room->vnum);
   }

   room = get_room_index(vnum);

   if (get_trust(ch) < LEVEL_ADMIN)
   {
      send_to_char("This command is for Admin only", ch);
      return;
   }
   /* Keepdesc, for wilderness rooms, but ones you don't want to have
      descriptions change -- Xerves 11/99 */
   if (!xIS_SET(room->room_flags, ROOM_WILDERNESS) || (xIS_SET(room->room_flags, ROOM_KEEPDESC)))
      return; /* for NON wilderness */

   exit[0] = vnum;
   get_exit_sectors(exit, ch);
   exitsum = exit[4];
   switch (room->sector_type)
   {
      case SECT_NTREE:
      case SECT_STREE:
      case SECT_FOREST:

         strcpy(name, "In a Forest");
         strcpy(desc, "You are in a forest teeming with life.  All around are the sure signs of nature.  ");
         if (exitsum == 4)
         {
            strcpy(name, "Within a Forest");
            strcpy(desc,
               "You are deep within in a forest, surrounded on all sides by trees.  Dead leaves blanket the ground and the lush canopy prevents much light from getting though.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;
      case SECT_FIELD:

         strcpy(name, "In a Field");
         strcpy(desc, "You are in a field of grass.  Prairie flora and shrubs are \
scattered throughout adding to the inviting aroma.  ");
         if (exitsum == 4)
         {
            strcpy(name, "On the Plains");
            strcpy(desc, "You are on the plains.  All around is grass, but not just \
any grass.  Out here it comes in many shapes, sizes, and even colors.  \
Insects fly through the air in occasional swarms.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_HILLS:
         strcpy(name, "On a Hill");
         strcpy(desc, "You are on a large and rather high hill.  You can see \
much of the surrounding area in full now more clearly.  ");
         if (exitsum == 4)
         {
            strcpy(name, "In the Foothills");
            strcpy(desc, "You are within the highlands.  \
All that surrounds is much of the same.  Hills and more hills, rolling \
across the land.  Definitely not the best route of travel.  \
");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_ROAD:
         strcpy(name, "A Road");
         strcpy(desc,
            "You are on a dirt road.  Being an easier and much safer way to travel, many would not think of leaving the safety of the road.  Hence it is somewhat highly trafficked.  ");
         if (exitsum == 4)
         {
            strcpy(name, "At a Four Way");
            strcpy(desc,
               "You are at a four way on the road.  There are roads in every direction.  The dirt road is trod down pretty well here and equally so in each direction.  \n\r");
            break;
         }
         if (exitsum == 3)
         {
            strcpy(name, "At a Fork in the Road");
            strcpy(desc, "You are at a fork in the road, there are two paths to \
choose from and no indication whatsoever which would be best. \
");
         }
         if (exitsum == 2)
         {
            strcpy(name, "A Road");
            strcpy(desc, "You are on a dirt road. Being an easier and much safer \
way to travel, many would not think of leaving the safety of the road. \
");
            if (exit[0] == SECT_ROAD)
            {
               sprintf(buf, "The road continues north and ");
               strcat(desc, buf);
               strcpy(buf, "  ");
               if (exit[1] == SECT_ROAD)
               {
                  sprintf(buf, "east.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[2] == SECT_ROAD)
               {
                  sprintf(buf, "south.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[3] == SECT_ROAD)
               {
                  sprintf(buf, "west.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
            }
            if (exit[1] == SECT_ROAD)
            {
               sprintf(buf, "The road continues east and ");
               strcat(desc, buf);
               strcpy(buf, "  ");
               if (exit[0] == SECT_ROAD)
               {
                  sprintf(buf, "north.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[2] == SECT_ROAD)
               {
                  sprintf(buf, "south.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[3] == SECT_ROAD)
               {
                  sprintf(buf, "west.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
            }
            if (exit[2] == SECT_ROAD)
            {
               sprintf(buf, "The road continues south and ");
               strcat(desc, buf);
               strcpy(buf, "  ");
               if (exit[0] == SECT_ROAD)
               {
                  sprintf(buf, "north.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[1] == SECT_ROAD)
               {
                  sprintf(buf, "east.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[3] == SECT_ROAD)
               {
                  sprintf(buf, "west.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
            }
            if (exit[3] == SECT_ROAD)
            {
               sprintf(buf, "The road continues west and ");
               strcat(desc, buf);
               strcpy(buf, "  ");
               if (exit[1] == SECT_ROAD)
               {
                  sprintf(buf, "east.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[2] == SECT_ROAD)
               {
                  sprintf(buf, "south.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }
               if (exit[0] == SECT_ROAD)
               {
                  sprintf(buf, "north.  ");
                  strcat(desc, buf);
                  strcpy(buf, "  ");
                  break;
               }

            } /* if exit[3]=SECT_ROAD */
         } /* if exitsum==2 */
         if ((exit[0] != -1) && (exit[0] != SECT_ROAD))
         {
            sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
            strcat(desc, buf);
            strcpy(buf, "  ");
         }

         if ((exit[1] != -1) && (exit[1] != SECT_ROAD))
         {
            sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
            strcat(desc, buf);
            strcpy(buf, "  ");
         }

         if ((exit[2] != -1) && (exit[2] != SECT_ROAD))
         {
            sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
            strcat(desc, buf);
            strcpy(buf, "  ");
         }

         if ((exit[3] != -1) && (exit[3] != SECT_ROAD))
         {
            sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
            strcat(desc, buf);
            strcpy(buf, "  ");
         }
         break;

      case SECT_WATER_SWIM:
         strcpy(name, "Shallow Water");
         strcpy(desc, "The water here is rather shallow.  You can see many little fish in the water, speeding to and fro in schools.  ");
         if (exitsum == 4)
         {
            strcpy(name, "Shallow Basin");
            strcpy(desc,
               "You are surrounded by shallow water.  All around in the water affluent marine life comes through, giving quite a show with the occasional leap above the waters surface.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }

         break;

      case SECT_WATER_NOSWIM:
         strcpy(name, "Deep Water");
         strcpy(desc, "The water here is of unknown depth and fairly dark.  \
The currents here are strong and the waves equally striking.  \
");
         if (exitsum == 4)
         {
            strcpy(name, "On a Sea");
            strcpy(desc, "You are on a dark sea, surrounded completely by more of the same.  \
Denizens of this dark water occasionaly make themselves seen \
out of curiousity, or perhaps necessity.  \
");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_AIR:
         strcpy(name, "In the Air");
         strcpy(desc, "You are in the air. ");
         if (exitsum == 4)
         {
            strcpy(name, "In the Air");
            strcpy(desc, "\
You are in the air, completed surrounded by more air.  Wisps of cloud-like \
moisture blows by and droplets of water condense in fatal collision with \
one another.  \
");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_DESERT:
         strcpy(name, "Desert Wasteland");
         strcpy(desc, "You are surrounded by sand dunes.  The occasional blooming \
cactus and sandstone formation give the land a simple beauty.  \
");
         if (exitsum == 4)
         {
            strcpy(name, "Deep Within the Desert");
            strcpy(desc, "\
You are deep within a barren desert.  Sand swept by the chaotic winds forms \
sand dunes that stretch on seemingly forever.  The shadow of overhead \
vultures indirectly warn of the danger in this place.  \
");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_MOUNTAIN:
         strcpy(name, "On a Mountain");
         strcpy(desc, "You are on a tranquil mountain.  Though not even remotely \
dangerous, this land has its share of cliffs and gorges.  \
");
         if (exitsum == 4)
         {
            strcpy(name, "In the Mountains");
            strcpy(desc, "You are deep within the mountains.  An occasional cliff \
add to the illusory nature of this once buried rock.  Yet, nature still \
makes every use of the land here.  Trees, shrubs and hardy animal all \
make their lives here.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_UNDERWATER:
         strcpy(name, "Under the Water.");
         strcpy(desc, "You are deep under the waters of the land.  All around the \
area, small fish and other water inhabiting creatures move around \
freely.  ");
         if (exitsum == 4)
         {
            strcpy(name, "Submerged in Water.");
            strcpy(desc, "You are completely submerged by the water.  All around \
the area, small fish and other water inhaviting creatures move around \
their natural habitat.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_OCEANFLOOR:
         strcpy(name, "On the Ocean Floor.");
         strcpy(desc, "You are now walking on the very bottom of the ocean.  All around \
the area, very large fish move around while disturbing the plant life that grows \
down here.  ");
         if (exitsum == 4)
         {
            strcpy(name, "Walking on the Ocean Floor.");
            strcpy(desc, "You are now walking around on the very bottom of the ocean.  All \
around the area, very large fish move around the water while disturbing the plant \
life that grows down here.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_UNDERGROUND:
         strcpy(name, "Deep Underground.");
         strcpy(desc, "You are now treading through a manmade tunnel.  The tunnel \
appears to be crafted with special care.  ");
         if (exitsum == 4)
         {
            strcpy(name, "An intersection Deep Underground.");
            strcpy(desc, "The tunnel cuts into four directions at this point.  You \
can choose any direction to take from this point.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_SGOLD:
      case SECT_NGOLD:
      case SECT_MINEGOLD:
         strcpy(name, "A Gold mine.");
         strcpy(desc, "You are now standing right before the entrance to a gold mine.  \
It appears that others have knowledge of this mine also because of the constant \
activity of this area.  ");
         if (exitsum == 4)
         {
            strcpy(name, "A Gold mine.");
            strcpy(desc, "In all four directions, all you can see are people extracting \
gold from the rich lands.  If you are here to extract gold, you better be doing \
it soon while there is gold left.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_SIRON:
      case SECT_NIRON:
      case SECT_MINEIRON:
         strcpy(name, "A Iron mine.");
         strcpy(desc, "You are now standing right before the entrance to a iron mine.  \
It appears that others have knowledge of this mine also because of the constant \
activity of this area.  ");
         if (exitsum == 4)
         {
            strcpy(name, "A Iron mine.");
            strcpy(desc, "In all four directions, all you can see are people extracting \
iron from the rich lands.  If you are here to extract iron, you better be doing \
it soon while there is iron left.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

      case SECT_SCORN:
      case SECT_NCORN:
      case SECT_HCORN:
         strcpy(name, "In a field of corn.");
         strcpy(desc, "The land you are standing on appears to be blessed by mother earth \
and has the ability to grow corn.  It might be a good idea to get some corn \
while it is still available.  ");
         if (exitsum == 4)
         {
            strcpy(name, "In a field of corn.");
            strcpy(desc, "Everywhere you look, all you can see is corn.  The land appears \
to be greatly blessed by mother earth and all the land around it as well.  If you \
are wanting corn, it might be a good idea to get some now.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }

         break;

      case SECT_SGRAIN:
      case SECT_NGRAIN:
      case SECT_HGRAIN:
         strcpy(name, "In a field of grain.");
         strcpy(desc, "The land you are standing on appears to be blessed by mother earth \
and has the ability to grow grain.  It might be a good idea to get some grain \
while it is still available.  ");
         if (exitsum == 4)
         {
            strcpy(name, "In a field of grain.");
            strcpy(desc, "Everywhere you look, all you can see is grain.  The land appears \
to be greatly blessed by mother earth and all the land around it as well.  If you \
are wanting grain, it might be a good idea to get some now.  ");
         }
         else
         {
            if (exit[0] != -1)
            {
               sprintf(buf, "You see %s to the north.  ", get_sector_name(exit[0], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[1] != -1)
            {
               sprintf(buf, "To the east you can see %s.  ", get_sector_name(exit[1], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[2] != -1)
            {
               sprintf(buf, "South of you, you can see %s.  ", get_sector_name(exit[2], 0));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }

            if (exit[3] != -1)
            {
               sprintf(buf, "%s can be seen to the west.  ", get_sector_name(exit[3], 1));
               strcat(desc, buf);
               strcpy(buf, "  ");
            }
         }
         break;

   } /*switch1 */

   SET_BIT(ch->in_room->area->flags, AFLAG_CHANGED);

   STRFREE(room->name);
   room->name = str_dup(name);

   STRFREE(room->description);
   sprintf(desc2, "%s", format_string1(desc));
   room->description = str_dup(desc2);

   return;
}

/* Read helpfile over the two commands here.  The first one does
   east to west areas */
void do_make_wilderness_exits(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *room;
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   int start, end, type;
   int x;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   type = atoi(arg1);
   start = atoi(arg2);
   end = atoi(argument);

   /* Rather simple, you type the command and three numbers.  The first
      option is the type (link right, link up, link down) and the last two
      numbers are used fors starting and ending points */
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: wildexits <direction> <start vnum> <end vnum>\n\rDirection: 0 - right  1 - north  2 - south\n\r", ch);
      return;
   }
   if (start > end)
   {
      send_to_char("You starting vnum needs to be lower than you final vnum\n\r", ch);
      return;
   }

   switch (type)
   {
         /* link right */
      case 0:
         {
            for (x = start; x < end + 1; x++)
            {
               if (x == end)
                  continue;
               else
               {
                  room = get_room_index(x);
                  char_from_room(ch);
                  char_to_room(ch, room);
                  sprintf(buf, "bexit e %d", x + 1);
                  do_redit(ch, buf);
               }
            }
            break;
         }
      case 1:
         {
            for (x = start; x < end + 1; x++)
            {
               room = get_room_index(x);
               char_from_room(ch);
               char_to_room(ch, room);
               sprintf(buf, "bexit n %d", x - 100);
               do_redit(ch, buf);
            }
            break;
         }
      case 2:
         {
            for (x = start; x < end + 1; x++)
            {
               room = get_room_index(x);
               char_from_room(ch);
               char_to_room(ch, room);
               sprintf(buf, "bexit s %d", x + 100);
               do_redit(ch, buf);
            }
            break;
         }
      default:
         send_to_char("Syntax: wildexits <direction> <start vnum> <end vnum>\n\rDirection: 0 - right  1 - north  2 - south\n\r", ch);
   }
   return;
}

void do_make_wilderness_exits2(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *room;
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   int start, end, type;
   int x;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   type = atoi(arg1);
   start = atoi(arg2);
   end = atoi(argument);

   /* Rather simple, you type the command and three numbers.  The first
      option is the type (link right, link up, link down) and the last two
      numbers are used fors starting and ending points */
   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: wildexits <direction> <start vnum> <end vnum>\n\rDirection: 0 - right  1 - north  2 - south\n\r", ch);
      return;
   }
   if (start > end)
   {
      send_to_char("You starting vnum needs to be lower than you final vnum\n\r", ch);
      return;
   }

   switch (type)
   {
         /* link right */
      case 0:
         {
            for (x = start; x < end + 1; x++)
            {
               room = get_room_index(x);
               char_from_room(ch);
               char_to_room(ch, room);
               sprintf(buf, "bexit e %d", x + 100);
               do_redit(ch, buf);
            }
            break;
         }
      case 1:
         {
            for (x = start; x < end + 1; x++)
            {
               if (x == start)
                  continue;
               else
               {
                  room = get_room_index(x);
                  char_from_room(ch);
                  char_to_room(ch, room);
                  sprintf(buf, "bexit n %d", x - 1);
                  do_redit(ch, buf);
               }
            }
            break;
         }
      case 2:
         {
            for (x = start; x < end + 1; x++)
            {
               if (x == end)
                  continue;
               else
               {
                  room = get_room_index(x);
                  char_from_room(ch);
                  char_to_room(ch, room);
                  sprintf(buf, "bexit s %d", x + 1);
                  do_redit(ch, buf);
               }
            }
            break;
         }
      default:
         send_to_char("Syntax: wildexits2 <direction> <start vnum> <end vnum>\n\rDirection: 0 - right  1 - north  2 - south\n\r", ch);
   }
   return;
}

/* mlk this sets the whole wilderness default descs and names*/
void do_set_wilderness_all(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *room;
   int start, end, current;

   start = 20001;
   end = 29600;

   room = get_room_index(start);
/* initialize room for this check here */

   for (current = start; current <= end; current++)
   {

      room = get_room_index(current);
      char_to_room(ch, room);
      do_setwilderness(ch, "");
      char_from_room(ch);

   }

   char_to_room(ch, room);

   return;
}
