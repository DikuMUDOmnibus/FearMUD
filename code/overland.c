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
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 *			    Overland Map Display and Support Code			    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "mud.h"

unsigned char map_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1]; /* Initializes the sector array */
char weather_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1]; /* Initializes the weather array */
short resource_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1]; /* Initalizes the resource array */
unsigned char kingdom_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1]; /* Initalizes the kingdom array */
int winddir;
int windstr;
int snows;
int cur_ship_uid;

//Use this instead of the table to show colors, uses less color parsing
char *show_room args((int sector, int x, int y, int map, int begin));
int get_distform args((int x, int y, int vx, int vy));
int check_ship_borders args((SHIP_DATA *ship));

ENTRANCE_DATA *first_entrance;
ENTRANCE_DATA *last_entrance;
FRONT_DATA *first_front;
FRONT_DATA *last_front;

void shutdown_mud args((char *reason));

char *const map_names[MAP_MAX] = {
   "Solan"
};

char *const map_name[MAP_MAX] = {
   "solan"
};

/* Sectortypes
 */
char *const sector_message[SECT_MAX] = {
   "Inside",
   "City",
   "Field",
   "Forest",
   "Hills",
   "Mountain",
   "Water Swim",
   "Water Noswim",
   "Underwater",
   "Air",
   "Desert",
   "Dunno",
   "Oceanfloor",
   "Underground",
   "Road",
   "Enter",
   "Mine Gold",
   "Mine Iron",
   "Corn Field",
   "Grain Field",
   "Chopped Trees",
   "Completely Chopped Trees",
   "Mined Gold mine",
   "Empty Gold mine",
   "Mined Iron mine",
   "Empty Iron mine",
   "Harvested Corn Field",
   "Empty Corn Field",
   "Harvested Grain Field",
   "Empty Grain Field",
   "River",
   "Jungle",
   "Shore",
   "Tundra",
   "Ice",
   "Ocean",
   "Lava",
   "Impass-Tree",
   "Impass-Stone",
   "Impass-Quicksand",
   "Wall",
   "Impass-Glacier",
   "Exit",
   "Swamp",
   "Path",
   "Plains",
   "Pavement",
   "Bridge",
   "Void",
   "Stable",
   "Fire",
   "Burnt",
   "Stone",
   "Some Stone",
   "No Stone",
   "Damaged Wall",
   "Broken Wall",
   "Door",
   "Closed Door",
   "Locked Door",
   "Hold",
   "Exit",
   "Ship"
};

/* Note - this message array is used to broadcast both the in sector messages,
 *  as well as the messages sent to PCs when they can't move into the sector
 */
char *const impass_message[SECT_MAX] = {
   "Inside",
   "City",
   "Field",
   "Forest",
   "Hills",
   "Mountain",
   "Water Swim",
   "Water Noswim",
   "Underwater",
   "Air",
   "Desert",
   "It is unknown, not quite safe to go into.",
   "Oceanfloor",
   "Underground",
   "Road",
   "Enter",
   "Mine Gold",
   "Mine Iron",
   "Corn Field",
   "Grain Field",
   "Chopped Trees",
   "Completely Chopped Trees",
   "Mined Gold mine",
   "Empty Gold mine",
   "Mined Iron mine",
   "Empty Iron mine",
   "Harvested Corn Field",
   "Empty Corn Field",
   "Harvested Grain Field",
   "Empty Grain Field",
   "River",
   "Jungle",
   "Shore",
   "Tundra",
   "Ice",
   "Ocean",
   "Lava",
   "The trees are too dense for you to pass through",
   "A huge massive boulder blocks your passage this direction",
   "Walking that direction into the quicksand is not a good idea.",
   "The wall is a little too big for you to pass over.",
   "A huge chunk of ice blocks you passage.",
   "Exit",
   "Swamp",
   "Path",
   "Plains",
   "Pavement",
   "Bridge",
   "That direction is blocked, you cannot enter.",
   "Stable",
   "Fire",
   "Burnt",
   "Stone",
   "Some Stone",
   "No Stone",
   "The damaged wall is a little too big for you to pass over.",
   "The nearly broken wall is a little too big for you to pass over."
   "Door",
   "You need to open the door first.",
   "You need to unlock and open the door first.",
   "Hold",
   "Exit",
   "Ship"
};

/* Changing this to use numbers instead SECT_, it really isn't needed and will allow for a ton
   of sectors and easy addition online */

void do_showascii(CHAR_DATA * ch, char *argument)
{
   int x;
   int sp;
   int mf;

   sp = 0;
   ch_printf(ch, "&R* &c&w Symbol for your location on the Map.\n\r");
   ch_printf(ch, "&G&W* &c&wSymbol for the location of another player on the Map.\n\r");
   ch_printf(ch, "^cC^x &c&wThe teal background indicated a mob (can be over any symbol.)\n\r");
   ch_printf(ch, "F &c&wAn F indicated there is a Fight going at that point on the map.\n\r");
   ch_printf(ch, "&BO &c&wA portal, help portal for more info.\n\r");
   ch_printf(ch, "&C# &c&wA transporter, typically used for intro to Fsanc.\n\r");
   ch_printf(ch, "&p+ &c&wAnything that uses the purple color has an item in the room.\n\r\n\r");
   ch_printf(ch, "symbol mv   Sector Type               symbol mv   Sector Type\n\r");
   ch_printf(ch, "     ratio                                 ratio\n\r");
   for (x = 0; x < SECT_MAX; x++)
   {
      if (sp % 2 == 0)
         ch_printf(ch, "\n\r");
      mf = sect_show[x].move / 5;
      ch_printf(ch, "   %s&c&w   %2d   %-23s   ", show_room(x, -10, -10, -10, -1), mf, sect_show[x].desc);
      sp += 1;
   }
   ch_printf(ch, "\n\r");
}

/*
const struct portal_data portal_show[] =
{
    { 259, 254, 0, "Rafermand City and Niemria" },
    { 279, 247, 0, "Wolf Forest and Shattered Refuge" },
    { 305, 224, 0, "Dwarven City and Anirandi Village" },
    { 333, 228, 0, "Unholy Grounds" },
    { 310, 268, 0, "Fire Newts" },
    { 298, 299, 0, "Field of Combat" },
    { 241, 307, 0, "Redferne" },
    { 214, 299, 0, "Dulavan Village" },
    { 274, 346, 0, "Midennir" },
    { 472, 372, 0, "Tree Village and Bluehaven" },
    { 280, 385, 0, "North Lake Alternity" },
    { 277, 404, 0, "Middle Lake Alternity" },
    { 315, 450, 0, "South Lake Alternity" },
    { 378, 419, 0, "East Lake Alternity" },
    { 243, 412, 0, "West Lake Alternity" },
    { 454, 332, 0, "Crystal Skull Palace" },
    { 429, 263, 0, "Seregon" },
    { 402, 262, 0, "New Thalos" },
    { 156, 375, 0, "Shadow Forest" },
    { 227, 238, 0, "Pixie Forest" },
    { 206, 220, 0, "Gallery" },
    { 239, 200, 0, "Nature's Retreat" },
    { 189, 194, 0, "Berum Manor" },
    { 191, 171, 0, "Black Hand Kingdom and Berum Manor" },
    { 294, 151, 0, "Aquarian Caves" },
    { 430, 45,  0, "Dragon Valley" },
    { 410, 127, 0, "Tree of Wisdom" },
    { 360, 95,  0, "Trail of Blood" },
    { 277, 222, 0, "Graveyard" },
    {  46, 442, 0, "Cove of Fire" },
    { 126, 167, 0, "Bood Caves" },
    { 360, 328, 0, "Fortress Morgond" },
    { 390, 420, 1, "Argoth Portal" },
    { 400, 211, 1, "Shikara Portal" },
    { 235, 330, 0, "Midnight Patch" },
    { 320, 340, 0, "PureLand" } 
};
 */
//Going to use functions instead of structures to give the information, will save on color usage.
char *show_room(int sector, int x, int y, int map, int begin)
{
   // -10 Use Color auto
   // -20 Don't use Color auto

   int type = 0; //no color
   int otype;

   if (x > -1 || y > -1 || map > -1)
   {
      if (map_sector[map][x - 1][y] != sector)
         type = 1;
      if (weather_sector[map][x - 1][y] >= 10)
         type = 1;
      if (begin == x)
         type = 1;
   }
   else
   {
      if (x == -10 || y == -10 || map == -10)
         type = 1;
   }

   otype = type;
   if (x > -1 && y > -1 && map > -1 && weather_sector[map][x][y] >= 10)
   {
      type = 2;
      if (NO_SNOW(map_sector[map][x][y]))
      {
         type = otype;
      }
   }

   if (type == 2 && snows == 0)
   {
      snows = 1;
      switch (sector)
      {
         case SECT_INSIDE:
            return "&w&W%";
         case SECT_CITY:
            return "&w&W#";
         case SECT_FIELD:
            return "&w&W\"";
         case SECT_FOREST:
            return "&w&W@";
         case SECT_HILLS:
            return "&w&W^^";
         case SECT_MOUNTAIN:
            return "&w&W^^";
         case SECT_WATER_SWIM:
            return "&w&W~";
         case SECT_WATER_NOSWIM:
            return "&w&W~";
         case SECT_UNDERWATER:
            return "&w&W~";
         case SECT_AIR:
            return "&w&W%";
         case SECT_DESERT:
            return "&w&W=";
         case SECT_DUNNO:
            return "&w&W?";
         case SECT_OCEANFLOOR:
            return "&w&W~";
         case SECT_UNDERGROUND:
            return "&w&W#";
         case SECT_ROAD:
            return "&w&W+";
         case SECT_ENTER:
            return "&w&W#";
         case SECT_MINEGOLD:
            return "&w&W^^";
         case SECT_MINEIRON:
            return "&w&W^^";
         case SECT_HCORN:
            return "&w&W\"";
         case SECT_HGRAIN:
            return "&w&W\"";
         case SECT_STREE:
            return "&w&W@";
         case SECT_NTREE:
            return "&w&W@";
         case SECT_SGOLD:
            return "&w&W^^";
         case SECT_NGOLD:
            return "&w&W^^";
         case SECT_SIRON:
            return "&w&W^^";
         case SECT_NIRON:
            return "&w&W^^";
         case SECT_SCORN:
            return "&w&W\"";
         case SECT_NCORN:
            return "&w&W\"";
         case SECT_SGRAIN:
            return "&w&W\"";
         case SECT_NGRAIN:
            return "&w&W\"";
         case SECT_RIVER:
            return "&w&W-";
         case SECT_JUNGLE:
            return "&w&W*";
         case SECT_SHORE:
            return "&w&W.";
         case SECT_TUNDRA:
            return "&w&W+";
         case SECT_ICE:
            return "&w&WI";
         case SECT_OCEAN:
            return "&w&W~";
         case SECT_LAVA:
            return "&w&W:";
         case SECT_TREE:
            return "&w&W*";
         case SECT_NOSTONE:
            return "&w&W^^";
         case SECT_QUICKSAND:
            return "&w&W%";
         case SECT_WALL:
            return "&w&WI";
         case SECT_DWALL:
            return "&w&OI";
         case SECT_NBWALL:
            return "&w&RI";
         case SECT_DOOR:
            return "&w&GD";
         case SECT_CDOOR:
            return "&w&YD";
         case SECT_LDOOR:
            return "&w&RD";
         case SECT_GLACIER:
            return "&w&W=";
         case SECT_EXIT:
            return "&w&W#";
         case SECT_SWAMP:
            return "&w&W%";
         case SECT_PATH:
            return "&w&W+";
         case SECT_PLAINS:
            return "&w&W~";
         case SECT_PAVE:
            return "&w&W#";
         case SECT_BRIDGE:
            return "&w&W=";
         case SECT_VOID:
            return "&w&W ";
         case SECT_STABLE:
            return "&w&W#";
         case SECT_FIRE:
            return "&w&W#";
         case SECT_BURNT:
            return "&w&W+";
         case SECT_STONE:
            return "&w&W*";
         case SECT_SSTONE:
            return "&w&W*";
         case SECT_NSTONE:
            return "&w&W*";
         case SECT_HOLD:
            return "&w&W0";
         case SECT_QEXIT:
            return "&w&W#";
         case SECT_SHIP:
            return "&w&RS";
         default:
            return "&w&W?";
      }
   }
   if (type == 2 && snows == 1)
   {
      switch (sector)
      {
         case SECT_INSIDE:
            return "%";
         case SECT_CITY:
            return "#";
         case SECT_FIELD:
            return "\"";
         case SECT_FOREST:
            return "@";
         case SECT_HILLS:
            return "^^";
         case SECT_MOUNTAIN:
            return "^^";
         case SECT_WATER_SWIM:
            return "~";
         case SECT_WATER_NOSWIM:
            return "~";
         case SECT_UNDERWATER:
            return "~";
         case SECT_AIR:
            return "%";
         case SECT_DESERT:
            return "=";
         case SECT_DUNNO:
            return "?";
         case SECT_OCEANFLOOR:
            return "~";
         case SECT_UNDERGROUND:
            return "#";
         case SECT_ROAD:
            return "+";
         case SECT_ENTER:
            return "#";
         case SECT_MINEGOLD:
            return "^^";
         case SECT_MINEIRON:
            return "^^";
         case SECT_HCORN:
            return "\"";
         case SECT_HGRAIN:
            return "\"";
         case SECT_STREE:
            return "@";
         case SECT_NTREE:
            return "@";
         case SECT_SGOLD:
            return "^^";
         case SECT_NGOLD:
            return "^^";
         case SECT_SIRON:
            return "^^";
         case SECT_NIRON:
            return "^^";
         case SECT_SCORN:
            return "\"";
         case SECT_NCORN:
            return "\"";
         case SECT_SGRAIN:
            return "\"";
         case SECT_NGRAIN:
            return "\"";
         case SECT_RIVER:
            return "-";
         case SECT_JUNGLE:
            return "*";
         case SECT_SHORE:
            return ".";
         case SECT_TUNDRA:
            return "+";
         case SECT_ICE:
            return "I";
         case SECT_OCEAN:
            return "~";
         case SECT_LAVA:
            return ":";
         case SECT_TREE:
            return "*";
         case SECT_NOSTONE:
            return "^^";
         case SECT_QUICKSAND:
            return "%";
         case SECT_WALL: case SECT_DWALL: case SECT_NBWALL:
            return "I";
         case SECT_DOOR: case SECT_CDOOR: case SECT_LDOOR:
            return "D";
         case SECT_GLACIER:
            return "=";
         case SECT_EXIT:
            return "#";
         case SECT_SWAMP:
            return "%";
         case SECT_PATH:
            return "+";
         case SECT_PLAINS:
            return "~";
         case SECT_PAVE:
            return "#";
         case SECT_BRIDGE:
            return "=";
         case SECT_VOID:
            return " ";
         case SECT_STABLE:
            return "#";
         case SECT_FIRE:
            return "#";
         case SECT_BURNT:
            return "+";
         case SECT_STONE:
            return "*";
         case SECT_SSTONE:
            return "*";
         case SECT_NSTONE:
            return "*";
         case SECT_HOLD:
            return "0";
         case SECT_QEXIT:
            return "#";
         case SECT_SHIP:
            return "S";
         default:
            return "?";
      }
   }

   snows = 0;
   if (type == 1)
   {
      switch (sector)
      {
         case SECT_INSIDE:
            return "&G&W%";
         case SECT_CITY:
            return "&G&W#";
         case SECT_FIELD:
            return "&G\"";
         case SECT_FOREST:
            return "&G@";
         case SECT_HILLS:
            return "&G^^";
         case SECT_MOUNTAIN:
            return "&O^^";
         case SECT_WATER_SWIM:
            return "&C~";
         case SECT_WATER_NOSWIM:
            return "&c~";
         case SECT_UNDERWATER:
            return "&B~";
         case SECT_AIR:
            return "&C%";
         case SECT_DESERT:
            return "&Y=";
         case SECT_DUNNO:
            return "&x?";
         case SECT_OCEANFLOOR:
            return "&b~";
         case SECT_UNDERGROUND:
            return "&O#";
         case SECT_ROAD:
            return "&c&w+";
         case SECT_ENTER:
            return "&G&W#";
         case SECT_MINEGOLD:
            return "&Y^^";
         case SECT_MINEIRON:
            return "&R^^";
         case SECT_HCORN:
            return "&Y\"";
         case SECT_HGRAIN:
            return "&z\"";
         case SECT_STREE:
            return "&g@";
         case SECT_NTREE:
            return "&c&w@";
         case SECT_SGOLD:
            return "&c&w^^";
         case SECT_NGOLD:
            return "&b^^";
         case SECT_SIRON:
            return "&B^^";
         case SECT_NIRON:
            return "&z^^";
         case SECT_SCORN:
            return "&O\"";
         case SECT_NCORN:
            return "&b\"";
         case SECT_SGRAIN:
            return "&z\"";
         case SECT_NGRAIN:
            return "&G\"";
         case SECT_RIVER:
            return "&B-";
         case SECT_JUNGLE:
            return "&g*";
         case SECT_SHORE:
            return "&Y.";
         case SECT_TUNDRA:
            return "&G&W+";
         case SECT_ICE:
            return "&G&WI";
         case SECT_OCEAN:
            return "&b~";
         case SECT_LAVA:
            return "&R:";
         case SECT_TREE:
            return "&G*";
         case SECT_NOSTONE:
            return "&c&w^^";
         case SECT_QUICKSAND:
            return "&O%";
         case SECT_WALL:
            return "&c&wI";
         case SECT_DWALL:
            return "&OI";
         case SECT_NBWALL:
            return "&RI";
         case SECT_DOOR:
            return "&w&GD";
         case SECT_CDOOR:
            return "&w&YD";
         case SECT_LDOOR:
            return "&w&RD";
         case SECT_GLACIER:
            return "&G&W=";
         case SECT_EXIT:
            return "&G&W#";
         case SECT_SWAMP:
            return "&g%";
         case SECT_PATH:
            return "&g+";
         case SECT_PLAINS:
            return "&O~";
         case SECT_PAVE:
            return "&z#";
         case SECT_BRIDGE:
            return "&C=";
         case SECT_VOID:
            return "&x ";
         case SECT_STABLE:
            return "&c&w#";
         case SECT_FIRE:
            return "&R#";
         case SECT_BURNT:
            return "&r+";
         case SECT_STONE:
            return "&c&w*";
         case SECT_SSTONE:
            return "&z*";
         case SECT_NSTONE:
            return "&Y*";
         case SECT_HOLD:
            return "&w&R0";
         case SECT_QEXIT:
            return "&w&P#";
         case SECT_SHIP:
            return "&w&RS";
         default:
            return "&G?";
      }
   }
   else
   {
      if (begin == x)
      {
         return show_room(sector, -10, -10, -10, -1);
      }
      switch (sector)
      {
         case SECT_INSIDE:
            return "%";
         case SECT_CITY:
            return "#";
         case SECT_FIELD:
            return "\"";
         case SECT_FOREST:
            return "@";
         case SECT_HILLS:
            return "^^";
         case SECT_MOUNTAIN:
            return "^^";
         case SECT_WATER_SWIM:
            return "~";
         case SECT_WATER_NOSWIM:
            return "~";
         case SECT_UNDERWATER:
            return "~";
         case SECT_AIR:
            return "%";
         case SECT_DESERT:
            return "=";
         case SECT_DUNNO:
            return "?";
         case SECT_OCEANFLOOR:
            return "~";
         case SECT_UNDERGROUND:
            return "#";
         case SECT_ROAD:
            return "+";
         case SECT_ENTER:
            return "#";
         case SECT_MINEGOLD:
            return "^^";
         case SECT_MINEIRON:
            return "^^";
         case SECT_HCORN:
            return "\"";
         case SECT_HGRAIN:
            return "\"";
         case SECT_STREE:
            return "@";
         case SECT_NTREE:
            return "@";
         case SECT_SGOLD:
            return "^^";
         case SECT_NGOLD:
            return "^^";
         case SECT_SIRON:
            return "^^";
         case SECT_NIRON:
            return "^^";
         case SECT_SCORN:
            return "\"";
         case SECT_NCORN:
            return "\"";
         case SECT_SGRAIN:
            return "\"";
         case SECT_NGRAIN:
            return "\"";
         case SECT_RIVER:
            return "-";
         case SECT_JUNGLE:
            return "*";
         case SECT_SHORE:
            return ".";
         case SECT_TUNDRA:
            return "+";
         case SECT_ICE:
            return "I";
         case SECT_OCEAN:
            return "~";
         case SECT_LAVA:
            return ":";
         case SECT_TREE:
            return "*";
         case SECT_NOSTONE:
            return "^^";
         case SECT_QUICKSAND:
            return "%";
         case SECT_WALL: case SECT_DWALL: case SECT_NBWALL:
            return "I";
         case SECT_DOOR: case SECT_CDOOR: case SECT_LDOOR:
            return "D";
         case SECT_GLACIER:
            return "=";
         case SECT_EXIT:
            return "#";
         case SECT_SWAMP:
            return "%";
         case SECT_PATH:
            return "+";
         case SECT_PLAINS:
            return "~";
         case SECT_PAVE:
            return "#";
         case SECT_BRIDGE:
            return "=";
         case SECT_VOID:
            return " ";
         case SECT_STABLE:
            return "#";
         case SECT_FIRE:
            return "#";
         case SECT_BURNT:
            return "+";
         case SECT_STONE:
            return "*";
         case SECT_SSTONE:
            return "*";
         case SECT_NSTONE:
            return "*";
         case SECT_HOLD:
            return "0";
         case SECT_QEXIT:
            return "#";
         case SECT_SHIP:
            return "S";
         default:
            return "?";
      }
   }
   return "&G?";
}

/* Values are base 9, 9 being good, 1 being bad */
const struct sect_color_type sect_show[] = {
   {SECT_INSIDE, "&G&W", "%", "%", "", 0, 5, "inside", TRUE, "c.gif"},
   {SECT_CITY, "&G&W", "#", "#", "", 0, 10, "city", TRUE, "c.gif"},
   {SECT_FIELD, "&G", "\"", "\"", "", 2, 14, "field", TRUE, "f.gif"},
   {SECT_FOREST, "&G", "@", "@", "", 4, 25, "forest", TRUE, "f2.gif"},
   {SECT_HILLS, "&G", "^^", "^", "", 3, 32, "hills", TRUE, "h.gif"},
   {SECT_MOUNTAIN, "&O", "^^", "^", "", 5, 45, "mountain", TRUE, "m.gif"},
   {SECT_WATER_SWIM, "&C", "~", "~", "", 2, 35, "shallow water", TRUE, "w3.gif"},
   {SECT_WATER_NOSWIM, "&c", "~", "~", "", 3, 40, "deep water", TRUE, "w2.gif"},
   {SECT_UNDERWATER, "&B", "~", "~", "", 3, 42, "underwater", TRUE, "o.gif"},
   {SECT_AIR, "&C", "%", "%", "", 2, 70, "air", TRUE, "f.gif"},
   {SECT_DESERT, "&Y", "=", "=", "", 2, 45, "desert", TRUE, "d.gif"},
   {SECT_DUNNO, "&x", "?", "?", "", 2, 39, "Unknown", FALSE, "f.gif"},
   {SECT_OCEANFLOOR, "&b", "~", "~", "", 3, 50, "ocean floor", TRUE, "o.gif"},
   {SECT_UNDERGROUND, "&O", "#", "#", "", 3, 34, "underground", TRUE, "f.gif"},
   {SECT_ROAD, "&c&w", "+", "+", "", 1, 5, "road", TRUE, "r.gif"},
   {SECT_ENTER, "&G&W", "#", "#", "", 0, 12, "enter", TRUE, "e.gif"},
   {SECT_MINEGOLD, "&Y", "^^", "^", "", 5, 45, "gold mine", TRUE, "m.gif"},
   {SECT_MINEIRON, "&R", "^^", "^", "", 5, 45, "iron mine", TRUE, "m.gif"},
   {SECT_HCORN, "&Y", "\"", "\"", "", 2, 14, "corn field", TRUE, "f.gif"},
   {SECT_HGRAIN, "&z", "\"", "\"", "", 2, 14, "grain field", TRUE, "f.gif"},
   {SECT_STREE, "&g", "@", "@", "", 3, 25, "chopped trees", TRUE, "f2.gif"},
   {SECT_NTREE, "&c&w", "@", "@", "", 3, 25, "tree stumps", TRUE, "f2.gif"},
   {SECT_SGOLD, "&c&w", "^^", "^", "", 5, 45, "mined gold mine", TRUE, "m.gif"},
   {SECT_NGOLD, "&b", "^^", "^", "", 5, 45, "empty gold mine", TRUE, "m.gif"},
   {SECT_SIRON, "&B", "^^", "^", "", 5, 45, "mined iron mine", TRUE, "m.gif"},
   {SECT_NIRON, "&z", "^^", "^", "", 5, 45, "empty iron mine", TRUE, "m.gif"},
   {SECT_SCORN, "&O", "\"", "\"", "", 2, 14, "havested corn field", TRUE, "f.gif"},
   {SECT_NCORN, "&b", "\"", "\"", "", 2, 14, "empty corn field", TRUE, "f.gif"},
   {SECT_SGRAIN, "&z", "\"", "\"", "", 2, 14, "harvested grain field", TRUE, "f.gif"},
   {SECT_NGRAIN, "&G", "\"", "\"", "", 2, 14, "empty grain field", TRUE, "f.gif"},
   {SECT_RIVER, "&B", "-", "-", "", 2, 42, "river", TRUE, "r2.gif"},
   {SECT_JUNGLE, "&g", "*", "*", "", 3, 35, "jungle", TRUE, "j.gif"},
   {SECT_SHORE, "&Y", ".", ".", "", 2, 9, "shoreline", TRUE, "s.gif"},
   {SECT_TUNDRA, "&G&W", "+", "+", "", 3, 21, "tundra", TRUE, "i.gif"},
   {SECT_ICE, "&G&W", "I", "I", "", 4, 40, "ice", TRUE, "i.gif"},
   {SECT_OCEAN, "&b", "~", "~", "", 4, 80, "ocean", FALSE, "o.gif"},
   {SECT_LAVA, "&R", ":", ":", "", 5, 68, "lava", TRUE, "l.gif"},
   {SECT_TREE, "&G", "*", "*", "", 0, 0, "impassable forest", FALSE, "f2.gif"},
   {SECT_NOSTONE, "&c&w", "^^", "^", "", 0, 0, "impassable mountain", FALSE, "m.gif"},
   {SECT_QUICKSAND, "&O", "%", "%", "", 0, 0, "quicksand(nopass swamp)", FALSE, "f.gif"},
   {SECT_WALL, "&c&w", "I", "I", "", 0, 0, "wall", FALSE, "w.gif"},
   {SECT_GLACIER, "&W", "=", "=", "", 0, 0, "glacier(nopass ice)", FALSE, "f.gif"},
   {SECT_EXIT, "&G&W", "#", "#", "", 0, 0, "exit", TRUE, "e.gif"},
   {SECT_SWAMP, "&g", "%", "%", "", 4, 38, "swamp", TRUE, "s4.gif"},
   {SECT_PATH, "&g", "+", "+", "", 1, 5, "path", TRUE, "r.gif"},
   {SECT_PLAINS, "&O", "~", "~", "", 2, 12, "plains", TRUE, "p.gif"},
   {SECT_PAVE, "&z", "#", "#", "", 2, 7, "pavement", TRUE, "r.gif"},
   {SECT_BRIDGE, "&C", "=", "=", "", 2, 7, "bridge", TRUE, "r.gif"},
   {SECT_VOID, "&x", " ", " ", "", 0, 0, "void", FALSE, "v.gif"},
   {SECT_STABLE, "&c&w", "#", "#", "", 0, 0, "stable", TRUE, "r.gif"},
   {SECT_FIRE, "&R", "#", "#", "", 0, 20, "fire", TRUE, "f.gif"},
   {SECT_BURNT, "&r", "+", "+", "", 2, 14, "burnt", TRUE, "f.gif"},
   {SECT_STONE, "&c&w", "*", "*", "", 3, 27, "stone", TRUE, "s3.gif"},
   {SECT_SSTONE, "&z", "*", "*", "", 3, 27, "some stone", TRUE, "s3.gif"},
   {SECT_NSTONE, "&Y", "*", "*", "", 3, 27, "no stone", TRUE, "s3.gif"},
   {SECT_DWALL, "&O", "I", "I", "", 0, 0, "damaged wall", FALSE, "w.gif"},
   {SECT_NBWALL, "&R", "I", "I", "", 0, 0, "nearly broke wall", FALSE, "w.gif"},
   {SECT_DOOR, "&G", "D", "D", "", 0, 5, "door", TRUE, "d2.gif"},
   {SECT_CDOOR, "&Y", "D", "D", "", 0, 0, "closed door", FALSE, "d2.gif"},
   {SECT_LDOOR, "&R", "D", "D", "", 0, 0, "locked door", FALSE, "d2.gif"},
   {SECT_HOLD,  "&R", "0", "0", "", 0, 5, "hold", TRUE, "s2.gif"},
   {SECT_QEXIT, "&w&P", "#", "#", "", 0, 0, "exit", TRUE, "q.gif"},
   {SECT_SHIP, "&w&R", "S", "S", "", 0, 0, "ship", FALSE, "q.gif"}
   
};

//Checks to see if the movement is valid, and changes the pointers
//to x and y with the new coords.  Passes back failure messages if
//ch is passed to it!

bool is_valid_movement(int *px, int *py, char *arg, CHAR_DATA *ch)
{
   int x, y;
   x = *px;
   y = *py;
   if (!str_cmp(arg, "n") || !str_cmp(arg, "north"))
      y = y-1;
   if (!str_cmp(arg, "s") || !str_cmp(arg, "south"))
      y = y+1;
   if (!str_cmp(arg, "e") || !str_cmp(arg, "east"))
      x = x+1;
   if (!str_cmp(arg, "w") || !str_cmp(arg, "west"))
      x = x-1;
   if (!str_cmp(arg, "nw") || !str_cmp(arg, "northwest"))
   {
      y = y-1;
      x = x-1;    
   }
   if (!str_cmp(arg, "ne") || !str_cmp(arg, "northeast"))
   {
      y = y-1;
      x = x+1;
   }
   if (!str_cmp(arg, "sw") || !str_cmp(arg, "southwest"))
   {
      y = y+1;
      x = x-1; 
   }
   if (!str_cmp(arg, "se") || !str_cmp(arg, "southeast"))
   {
      y = y+1;
      x = x+1; 
   }   
   if (x < 1 || x > MAX_X || y < 1 || y > MAX_Y)
   {
      if (ch)
         send_to_char("That is off the map, you cannot do that.\n\r", ch);
      return FALSE;
   }
   if (*px == x && *py == y)
   {
      if (ch)
         send_to_char("You have to choose a direction, ex: n nw n northwest\n\r", ch);
      return FALSE;
   }
   *px = x;
   *py = y;
   return TRUE;
}
   

void add_entrance(int tomap, int onmap, int hereX, int hereY, int thereX, int thereY, int vnum)
{
   ENTRANCE_DATA *enter;

   CREATE(enter, ENTRANCE_DATA, 1);
   LINK(enter, first_entrance, last_entrance, next, prev);
   enter->tomap = tomap;
   enter->onmap = onmap;
   CREATE(enter->here, COORD_DATA, 1);
   CREATE(enter->there, COORD_DATA, 1);
   enter->here->x = hereX;
   enter->here->y = hereY;
   enter->there->x = thereX;
   enter->there->y = thereY;
   enter->vnum = vnum;

   return;
}

void delete_entrance(ENTRANCE_DATA * enter)
{
   enter->tomap = 0;
   enter->onmap = 0;
   enter->here->x = 0;
   enter->here->y = 0;
   enter->there->x = 0;
   enter->there->y = 0;
   enter->vnum = 0;
   UNLINK(enter, first_entrance, last_entrance, next, prev);

   return;
}

#if defined(KEY)
#undef KEY
#endif

#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}

void fread_entrance(ENTRANCE_DATA * enter, FILE * fp)
{
   char buf[MSL];
   char *word;
   bool fMatch;

   for (;;)
   {
      word = feof(fp) ? "End" : fread_word(fp);
      fMatch = FALSE;

      switch (UPPER(word[0]))
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol(fp);
            break;

         case 'E':
            if (!str_cmp(word, "End"))
            {
               return;
            }
            break;

         case 'H':
            if (!str_cmp(word, "Here"))
            {
               enter->here->x = fread_number(fp);
               enter->here->y = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            break;

         case 'O':
            KEY("OnMap", enter->onmap, fread_number(fp));
            break;

         case 'T':
            if (!str_cmp(word, "There"))
            {
               enter->there->x = fread_number(fp);
               enter->there->y = fread_number(fp);
               fMatch = TRUE;
               break;
            }
            KEY("ToMap", enter->tomap, fread_number(fp));
            break;

         case 'V':
            KEY("Vnum", enter->vnum, fread_number(fp));
            break;
      }

      if (!fMatch)
      {
         sprintf(buf, "Fread_entrance: no match: %s", word);
         bug(buf, 0);
      }
   }
}

void load_entrances()
{
   char filename[256];
   ENTRANCE_DATA *enter;
   FILE *fp;

   first_entrance = NULL;
   last_entrance = NULL;

   sprintf(filename, "%s%s", MAP_DIR, ENTRANCE_FILE);

   if ((fp = fopen(filename, "r")) != NULL)
   {
      for (;;)
      {
         char letter;
         char *word;

         letter = fread_letter(fp);
         if (letter == '*')
         {
            fread_to_eol(fp);
            continue;
         }

         if (letter != '#')
         {
            bug("Load_entrances: # not found.", 0);
            break;
         }

         word = fread_word(fp);
         if (!str_cmp(word, "ENTRANCE"))
         {
            CREATE(enter, ENTRANCE_DATA, 1);
            CREATE(enter->here, COORD_DATA, 1);
            CREATE(enter->there, COORD_DATA, 1);
            fread_entrance(enter, fp);
            LINK(enter, first_entrance, last_entrance, next, prev);
            continue;
         }
         else if (!str_cmp(word, "END"))
            break;
         else
         {
            char buf[MSL];

            sprintf(buf, "Load_entrances: bad section: %s.", word);
            bug(buf, 0);
            continue;
         }
      }
      FCLOSE(fp);
   }

   return;
}

char *get_area_from_vnum(int vnum)
{
   static char area[200];
   AREA_DATA *tarea;
   
   for (tarea = first_asort; tarea; tarea = tarea->next)
   {
      if (vnum >= tarea->low_r_vnum && vnum <= tarea->hi_r_vnum)
         break;
   }
   if (!tarea)
   {
      for (tarea = first_bsort; tarea; tarea = tarea->next)
      {
         if (vnum >= tarea->low_r_vnum && vnum <= tarea->hi_r_vnum)
            break;
      }
   }
   if (!tarea)
      return "Unknown";
   else
   {
      sprintf(area, "%s", tarea->name);
      return &area[0];
   }
}

//sorts and cleans entrances.  Removing those that are linked oddly
void sort_entrances()
{
   ENTRANCE_DATA *enter;
   ENTRANCE_DATA *senter;
   ENTRANCE_DATA *nenter;
   ENTRANCE_DATA *next_entrance;
   ENTRANCE_DATA *first_sort_entrance = NULL;
   ENTRANCE_DATA *last_sort_entrance = NULL;
   
   for (enter = first_entrance; enter; enter = next_entrance)
   {
     next_entrance = enter->next;
     if (!first_sort_entrance)
        LINK(enter, first_sort_entrance, last_sort_entrance, next, prev);
     else
     {
        if (enter->vnum <= first_sort_entrance->vnum)
        {
           nenter = first_sort_entrance;
           enter->prev = NULL;
           nenter->prev = enter;
           enter->next = nenter;
           first_sort_entrance = enter;
           continue;
        }
        for (senter = first_sort_entrance; senter; senter = senter->next)
        {
           if (senter->next == NULL)
           {
              LINK(enter, first_sort_entrance, last_sort_entrance, next, prev);
              break;
           }
           if (enter->vnum <= senter->next->vnum)
           {
              nenter = senter->next;
              nenter->prev->next = enter;
              enter->prev = nenter->prev;
              nenter->prev = enter;
              enter->next = nenter;
              break;
           }
        }
     }
  }
  first_entrance = first_sort_entrance;
  last_entrance = last_sort_entrance;
  for (enter = first_entrance; enter; enter = next_entrance)
  {
     next_entrance = enter->next;
     
     if (enter->next && enter->next->vnum == enter->vnum && enter->next->here->x == enter->here->x && enter->next->here->y == enter->here->y)
     {
        UNLINK(enter, first_entrance, last_entrance, next, prev);
     }
  }
  for (enter = first_entrance; enter; enter = next_entrance)
  {
     next_entrance = enter->next;
     
     if (!str_cmp(get_area_from_vnum(enter->vnum), "Unknown"))
     {
        UNLINK(enter, first_entrance, last_entrance, next, prev);
     }
  }
}

void do_showentrances(CHAR_DATA *ch, char *argument)
{
   ENTRANCE_DATA *enter;
   int num = 1;
   
   sort_entrances();
   send_to_char("Num  ToMap  Onmap  Here       There       Vnum   Area\n\r-----------------------------------------------------------\n\r", ch);
   for (enter = first_entrance; enter; enter = enter->next)
   {
      ch_printf(ch, "%-3d  %-2d     %-2d     %-4d %-4d  %-4d  %-4d  %-5d  %s\n\r", num, enter->tomap, enter->onmap, enter->here->x, enter->here->y,
                     enter->there->x, enter->there->y, enter->vnum, get_area_from_vnum(enter->vnum));
      num++;
   }
}    

void save_entrances()
{
   ENTRANCE_DATA *enter;
   FILE *fp;
   char filename[256];

   sprintf(filename, "%s%s", MAP_DIR, ENTRANCE_FILE);

   FCLOSE(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_entrances: fopen", 0);
      perror(filename);
   }
   else
   {
      for (enter = first_entrance; enter; enter = enter->next)
      {
         fprintf(fp, "#ENTRANCE\n");
         fprintf(fp, "ToMap	%d\n", enter->tomap);
         fprintf(fp, "OnMap	%d\n", enter->onmap);
         fprintf(fp, "Here	%d %d\n", enter->here->x, enter->here->y);
         fprintf(fp, "There	%d %d\n", enter->there->x, enter->there->y);
         fprintf(fp, "Vnum	%d\n", enter->vnum);
         fprintf(fp, "End\n\n");
      }
      fprintf(fp, "#END\n");
      FCLOSE(fp);
   }
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

ENTRANCE_DATA *check_entrance(CHAR_DATA * ch, int map, int x, int y)
{
   ENTRANCE_DATA *enter;

   for (enter = first_entrance; enter; enter = enter->next)
   {
      if (enter->onmap == map)
      {
         if (enter->here->x == x && enter->here->y == y)
            return enter;
      }
   }
   return NULL;
}
char *get_map_name(int map)
{
   if (map < 0 || map > MAP_MAX)
      return "unknown";
   else
      return map_names[map];
}

bool can_see_room(int x, int y, int map, int sector)
{
   TOWN_DATA *town;
   DOOR_DATA *ddata;
   int z;
   
   //the map does a lot of processing, lets not do more than is needed...
   if (sector != SECT_INSIDE)
      return TRUE;
   if (kingdom_sector[map][x][y] < 2 || kingdom_sector[map][x][y] > sysdata.max_kingdom)
      return TRUE;
      
   town = find_town(x, y, map);
   
   if (town)
   {
      for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
      {
         for (z = 0; z <= MAX_HPOINTS-1; z++)
         {
            if (ddata->roomcoordx[z] == x && ddata->roomcoordy[z] == y && ddata->roomcoordmap[z] == map)
            {
               if (ddata->cansee == 0)
                  return FALSE;
               else
                  return TRUE;
            }
         }
      }
   }
   return TRUE;
}

int update_inside_stat(int fnddoor[], CHAR_DATA *ch, TOWN_DATA *town, int x, int y, int map)
{
   int udoornum;
   DOOR_DATA *ddata;
   int z;
   int fnx;
   int cnt;
   int fnd;
   int fx;
   int dx;
   
   fx = 0;
   cnt = -1;
   
   for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
   {
      for (z = 0; z <= MAX_HPOINTS-1; z++)
      {
         if (ddata->roomcoordx[z] == x && ddata->roomcoordy[z] == y && ddata->roomcoordmap[z] == map)
            break;
      }
      if (z != MAX_HPOINTS)
      {
         for (dx = 0; dx <= 9; dx++)
         {
            if (ddata->doorvalue[dx] > 0)
            {
               for (fnx = 0; fnx <= 99; fnx++)
               {
                  if (town->doorstate[4][fnx] == ddata->doorvalue[dx] && town->doorstate[0][fnx] == 0)
                  {
                     fnddoor[fx++] = town->doorstate[4][fnx];
                     ddata->cannotsee = 1;
                     break;
                  }
               }
               if (fnx <= 99)
                  break;
            }
         }
      }
   }
   if (fx == 0)
      return 0;
   
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
            ddata->cannotsee = 1;
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
                  if (town->doorstate[0][fnx] == 0 && town->doorstate[3][fnx] == 1) //Open Master Door, I can see light!!!
                     return 1;
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
                        bug("update_inside_stat:  The fnddoor array is full for town %s", town->name);
                        return 1;
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
   return 0;
}

void do_mxp(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   
   if (check_npc(ch))
      return;
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  mxp mapwindow [on/off]\n\r", ch);
      send_to_char("Syntax:  mxp mapwindow [close/open]\n\r", ch);
      send_to_char("Syntax:  mxp wildernesstiles [on/off]\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg);
   if (!str_cmp(arg, "wildernesstiles"))
   {
      if (!str_cmp(argument, "on"))
      {
         xSET_BIT(ch->act, PLR_WILDERTILES);
         send_to_char("Wilderness Tiles set to load, make sure you have downloaded them or you will get nothing.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "off"))
      {
         xREMOVE_BIT(ch->act, PLR_WILDERTILES);
         send_to_char("Wilderness Tiles set to not load.\n\r", ch);
         return;
      }
   }
   if (!str_cmp(arg, "mapwindow"))
   {
      if (!str_cmp(argument, "on"))
      {
         xSET_BIT(ch->act, PLR_MAPWINDOW);
         ch->pcdata->xsize = 0;
         ch->pcdata->ysize = 0;
         ch_printf(ch, "%s", MXPTAG("FRAME Name=\"Map\" FLOATING Left=\"-21c\" Top=\"0\" Width=\"21c\" Height=\"21c\"")); 
         send_to_char("The window was turned on and loaded, if you do not see it, you might not have mxp support.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "off"))
      {
         xREMOVE_BIT(ch->act, PLR_MAPWINDOW);
         ch->pcdata->xsize = 0;
         ch->pcdata->ysize = 0;
         ch_printf(ch, "%s", MXPTAG("FRAME Map CLOSE"));
         send_to_char("The window was turned off and the window was closed.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "close"))
      {
         ch_printf(ch, "%s", MXPTAG("FRAME Map CLOSE"));
         send_to_char("Window closed, but not turned off.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "open"))
      {
         ch_printf(ch, "%s", MXPTAG("FRAME Name=\"Map\" FLOATING Left=\"-21c\" Top=\"0\" Width=\"21c\" Height=\"21c\"")); 
         send_to_char("Window opened but not turned on/off.\n\r", ch);
         return;
      }
   }
   do_mxp(ch, "");
   return;
}  
        
void do_portal(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   int p;
   int count = 0;
   int x, y, map;
   PORTAL_DATA *portal;
   
   x=y=0;
   map=-1;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  portal list\n\r", ch);
      send_to_char("Syntax:  portal add [x] [y] [map] <Description>\n\r", ch);
      send_to_char("Syntax:  portal remove [x] [y] [map]\n\r", ch);
      return;
   }
   
   argument = one_argument(argument, arg1);
   
   if (!str_cmp(arg1, "list"))
   {
      for (p = 0; p < sysdata.last_portal; p++)
      {
         ch_printf(ch, "[%2d]  %-4dX  %-4dY  %s\n\r", ++count, portal_show[p]->x, portal_show[p]->y, portal_show[p]->desc);
      }
      return;
   }    
   if (!str_cmp(arg1, "remove"))
   {
      if (atoi(argument) > 0)
      {
         argument = one_argument(argument, arg2);
         argument = one_argument(argument, arg3);
         argument = one_argument(argument, arg4);
         x = atoi(arg2);
         y = atoi(arg3);
         map = atoi(arg4);
      }
      if (x == 0)
      {
         if (!IN_WILDERNESS(ch))
         {
            send_to_char("You need to be in the wilderness to remove a portal.\n\r", ch);
            return;
         }
         x = ch->coord->x;
         y = ch->coord->y;
         map = ch->map;         
      }
      if (x < 1 || x > MAX_X || y < 1 || y > MAX_Y || map < 0 || map >= MAP_MAX)
      {
         send_to_char("That is not a valid coordinates.\n\r", ch);
         return;
      }
      for (p = 0; p < sysdata.last_portal; p++)
      {
         if (portal_show[p]->x == x && portal_show[p]->y == y && portal_show[p]->map == map)
            break;
      }
      if (p == sysdata.last_portal)
      {
         send_to_char("There is no portal to remove!!!\n\r", ch);
         return;
      }
      STRFREE(portal_show[p]->desc);
      for (x = p+1; x < sysdata.last_portal; x++)
      {
         portal_show[x-1] = portal_show[x];
      }
      portal_show[x-1] = NULL;
      sysdata.last_portal--;
      send_to_char("Done.\n\r", ch);
      save_portal_file();
      return;
   }
   if (!str_cmp(arg1, "add"))
   {
      if (sysdata.last_portal == LAST_PORTAL)
      {
         send_to_char("There are 100 portals, that is the max, increase LAST_PORTAL variable to add more.\n\r", ch);
         return;
      }
      if (atoi(argument) > 0)
      {
         argument = one_argument(argument, arg2);
         argument = one_argument(argument, arg3);
         argument = one_argument(argument, arg4);
         x = atoi(arg2);
         y = atoi(arg3);
         map = atoi(arg4);
      }
      if (x == 0)
      {
         if (!IN_WILDERNESS(ch))
         {
            send_to_char("You need to be in the wilderness to add a portal.\n\r", ch);
            return;
         }
         x = ch->coord->x;
         y = ch->coord->y;
         map = ch->map;         
      }
      if (x < 1 || x > MAX_X || y < 1 || y > MAX_Y || map < 0 || map >= MAP_MAX)
      {
         send_to_char("That is not a valid coordinates.\n\r", ch);
         return;
      }
      for (p = 0; p < sysdata.last_portal; p++)
      {
         if (portal_show[p]->x == x && portal_show[p]->y == y && portal_show[p]->map == map)
            break;
      }
      if (p < sysdata.last_portal)
      {
         send_to_char("There is already a portal here, remove it first if you want to add a new one.\n\r", ch);
         return;
      }
      CREATE(portal, PORTAL_DATA, 1);
      portal->x = x;
      portal->y = y;
      portal->map = map;
      portal->desc = STRALLOC(argument);
      portal_show[sysdata.last_portal++] = portal;
      send_to_char("Done.\n\r", ch);
      save_portal_file();
      return;
   }
   do_portal(ch, "");
   return;
}

void do_ships(CHAR_DATA *ch, char *argument)
{
   SHIP_DATA *ship;
   int num = 1;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  ships create\n\r", ch);
      send_to_char("Syntax:  ships list\n\r", ch);
      send_to_char("Syntax:  ships delete <number>\n\r", ch);
      send_to_char("Syntax:  ships edit <number> <size/route/routetime/occupants> <value>\n\r", ch);
      return;
   }  
   if (!str_cmp(argument, "list"))
   {
      send_to_char("Num   X     Y     TX    TY    Dir    Occupants  Routetime  Route  Size\n\r-----------------------------------------------\n\r", ch);
      for (ship = first_ship; ship; ship = ship->next)
      {
         ch_printf(ch, "%-3d>  %-4d  %-4d  %-4d  %-4d  %-5s  %-3d        %-3d         %-3s    %d\n\r", num++, ship->x, ship->y, ship->tx, ship->ty,
            dir_name[ship->direction], ship->occupants, ship->routetime, ship->travelroute ? ship->travelroute[0] != '\0' ? "Yes" : "No" : "No", 
            ship->size);
      }
      return;
   }
   if (!str_cmp(argument, "create"))
   {
      CREATE(ship, SHIP_DATA, 1);
      ship->x = ch->coord->x;
      ship->y = ch->coord->y;
      ship->map = ch->map;
      ship->tx = ship->ty = ship->tmap = -1;
      ship->direction = 0;
      ship->size = 2;
      if (!check_ship_borders(ship))
      {
         send_to_char("Something is blocking you from creating the ship, move a bit further into the ocean.\n\r", ch);
         DISPOSE(ship);
         return;
      }
      ship->travelroute = STRALLOC("");
      ship->uid = ++cur_ship_uid;
      LINK(ship, first_ship, last_ship, next, prev);
      set_ship_sector(ship, 0, 1);
      send_to_char("Done.\n\r", ch);
      fwrite_ship_data();
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   if (!str_cmp(arg1, "delete"))
   {
      for (ship = first_ship; ship; ship = ship->next)
      {
         if (atoi(arg2) == num++)
            break;
      }
      if (!ship)
      {
         send_to_char("The number you specified does not exist.\n\r", ch);
         return;
      }
      if (ship->occupants > 0)
      {
         send_to_char("There is someone on that ship, you cannot delete it.\n\r", ch);
         return;
      }
      set_ship_sector(ship, 1, 0);
      STRFREE(ship->travelroute);
      UNLINK(ship, first_ship, last_ship, next, prev);
      DISPOSE(ship);
      send_to_char("Deleted.\n\r", ch);
      return;
   }
   if (!str_cmp(arg1, "edit"))
   {
      for (ship = first_ship; ship; ship = ship->next)
      {
         if (atoi(arg2) == num++)
            break;
      }
      if (!ship)
      {
         send_to_char("The number you specified does not exist.\n\r", ch);
         return;
      }
      if (!str_cmp(arg3, "size"))
      {
         int oldsize = ship->size;
         if (atoi(argument) < 1 || atoi(argument) > 10)
         {
            send_to_char("Range is 1 to 10.\n\r", ch);
            return;
         }
         set_ship_sector(ship, 1, 0);
         ship->size = atoi(argument);
         if (check_ship_borders(ship))
         {
            set_ship_sector(ship, 0, 1);
            fwrite_ship_data();
            send_to_char("Done.\n\r", ch);
            return;
         }
         else
         {
            ship->size = oldsize;
            set_ship_sector(ship, 0, 1);
            send_to_char("Cannot change the size, something is blocking you from doing so.\n\r", ch);
            return;
         }
      }
      if (!str_cmp(arg3, "route"))
      {
         if (ship->travelroute)
            STRFREE(ship->travelroute);
         ship->travelroute = STRALLOC(argument);
         if (argument[0] == '\0')
         {
            ship->routeplace = 0;
            ship->routedir = 0;
            ship->routetick = 0;
         }
         send_to_char("Done.\n\r", ch);
         fwrite_ship_data();
         return;
      }
      if (!str_cmp(arg3, "occupants"))
      {
         if (atoi(argument) < 0 || atoi(argument) > 600)
         {
            send_to_char("Range is 0 to 600.\n\r", ch);
            return;
         }
         ship->occupants = atoi(argument);
         send_to_char("Done.\n\r", ch);
         fwrite_ship_data();
         return;
      }
      if (!str_cmp(arg3, "routetime"))
      {
         if (atoi(argument) < 0 || atoi(argument) > 600)
         {
            send_to_char("Range is 0 to 600.\n\r", ch);
            return;
         }
         ship->routetime = atoi(argument);
         send_to_char("Done.\n\r", ch);
         fwrite_ship_data();
         return;
      }
   }
   do_ships(ch, "");
   return;
}
/* Below are the ship designs for the 10 sizes.  The design will shift for north/south and
east/west facing

 x

 x
xxx
 x

 x
xxx
xxx
xxx
 x

 xxx
xxxxx
xxxxx
xxxxx
 xxx
 
 xxx
xxxxx
xxxxx
xxxxx
xxxxx
xxxxx
 xxx
 
 xxxxx
xxxxxxx
xxxxxxx
xxxxxxx
xxxxxxx
xxxxxxx
 xxxxx
 
 xxxxx
xxxxxxx
xxxxxxx
xxxxxxx
xxxxxxx
xxxxxxx
xxxxxxx
xxxxxxx
 xxxxx

  xxxxx
 xxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
 xxxxxxx
  xxxxx
 
  xxxxx
 xxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
xxxxxxxxx
 xxxxxxx
  xxxxx
 
   xxxxx
  xxxxxxx
 xxxxxxxxx
xxxxxxxxxxx
xxxxxxxxxxx
xxxxxxxxxxx
xxxxxxxxxxx
xxxxxxxxxxx
 xxxxxxxxx
  xxxxxxx
   xxxxx
               */
   

//Reset 0 - Do not reset sectors (make ship sectors) 1 - Reset (make ocean sectors)
void set_ship_sector(SHIP_DATA *ship, int reset, int save)
{
   int sector;
   int sy, sx;
   
   if (reset == 0)
      sector = SECT_SHIP;
   else
      sector = SECT_OCEAN;
      
   map_sector[ship->map][ship->x][ship->y] = sector;
   
   if (ship->direction == 0 || ship->direction == 2) //North - South
   {
      if (ship->size == 2)
      {
         for (sy = ship->y-3; sy <= ship->y+3; sy++)
         {
            for (sx = ship->x-1; sx <= ship->x+1; sx++)
            {
               if ((sy == ship->y-3 || sy == ship->y+3) && sx != ship->x)
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 3)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-1; sx <= ship->x+1; sx++)
            {
               if ((sy == ship->y-4 || sy == ship->y+4) && sx != ship->x)
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 4)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-2; sx <= ship->x+2; sx++)
            {
               if ((sy == ship->y-4 || sy == ship->y+4) && (sx == ship->x-2 || sx == ship->x+2))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 5)
      {
         for (sy = ship->y-5; sy <= ship->y+5; sy++)
         {
            for (sx = ship->x-2; sx <= ship->x+2; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-2 || sx == ship->x+2))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 6)
      {
         for (sy = ship->y-5; sy <= ship->y+5; sy++)
         {
            for (sx = ship->x-3; sx <= ship->x+3; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-3 || sx == ship->x+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 7)
      {
         for (sy = ship->y-6; sy <= ship->y+6; sy++)
         {
            for (sx = ship->x-3; sx <= ship->x+3; sx++)
            {
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx == ship->x-3 || sx == ship->x+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 8)
      {
         for (sy = ship->y-6; sy <= ship->y+6; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-4 || sx == ship->x+4))
                  continue;
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx <= ship->x-3 || sx >= ship->x+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 9)
      {
         for (sy = ship->y-7; sy <= ship->y+7; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx == ship->x-4 || sx == ship->x+4))
                  continue;
               if ((sy == ship->y-7 || sy == ship->y+7) && (sx <= ship->x-3 || sx >= ship->x+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 10)
      {
         for (sy = ship->y-7; sy <= ship->y+7; sy++)
         {
            for (sx = ship->x-5; sx <= ship->x+5; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-5 || sx == ship->x+5))
                  continue;
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx <= ship->x-4 || sx >= ship->x+4))
                  continue;
               if ((sy == ship->y-7 || sy == ship->y+7) && (sx <= ship->x-3 || sx >= ship->x+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
   }
   else
   {
      if (ship->size == 2)
      {
         for (sy = ship->y-1; sy <= ship->y+1; sy++)
         {
            for (sx = ship->x-3; sx <= ship->x+3; sx++)
            {
               if ((sx == ship->x-3 || sx == ship->x+3) && sy != ship->y)
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 3)
      {
         for (sy = ship->y-1; sy <= ship->y+1; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sx == ship->x-4 || sx == ship->x+4) && sy != ship->y)
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 4)
      {
         for (sy = ship->y-2; sy <= ship->y+2; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sx == ship->x-4 || sx == ship->x+4) && (sy == ship->y-2 || sy == ship->y+2))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 5)
      {
         for (sy = ship->y-2; sy <= ship->y+2; sy++)
         {
            for (sx = ship->x-5; sx <= ship->x+5; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-2 || sy == ship->y+2))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 6)
      {
         for (sy = ship->y-3; sy <= ship->y+3; sy++)
         {
            for (sx = ship->x-5; sx <= ship->x+5; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-3 || sy == ship->y+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 7)
      {
         for (sy = ship->y-3; sy <= ship->y+3; sy++)
         {
            for (sx = ship->x-6; sx <= ship->x+6; sx++)
            {
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy == ship->y-3 || sy == ship->y+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 8)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-6; sx <= ship->x+6; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-4 || sy == ship->y+4))
                  continue;
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy <= ship->y-3 || sy >= ship->y+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 9)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-7; sx <= ship->x+7; sx++)
            {
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy == ship->y-4 || sy == ship->y+4))
                  continue;
               if ((sx == ship->x-7 || sx == ship->x+7) && (sy <= ship->y-3 || sy >= ship->y+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
      if (ship->size == 10)
      {
         for (sy = ship->y-5; sy <= ship->y+5; sy++)
         {
            for (sx = ship->x-7; sx <= ship->x+7; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-5 || sy == ship->y+5))
                  continue;
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy <= ship->y-4 || sy >= ship->y+4))
                  continue;
               if ((sx == ship->x-7 || sx == ship->x+7) && (sy <= ship->y-3 || sy >= ship->y+3))
                  continue;
               map_sector[ship->map][sx][sy] = sector;
            }
         }
      }
   }   
   //if (save)
   //   save_map("solan", 0);   
}

int check_ship_borders(SHIP_DATA *ship)
{
   int sx;
   int sy;
   
   if (ship->x < 1 || ship->x > MAX_X || ship->y < 1 || ship->y > MAX_Y)
      return 0;
   if (map_sector[ship->map][ship->x][ship->y] != SECT_OCEAN)
      return 0;
      
   if (ship->direction == 0 || ship->direction == 2) //North - South
   {
      if (ship->size == 2)
      {
         for (sy = ship->y-3; sy <= ship->y+3; sy++)
         {
            for (sx = ship->x-1; sx <= ship->x+1; sx++)
            {
               if ((sy == ship->y-3 || sy == ship->y+3) && sx != ship->x)
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 3)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-1; sx <= ship->x+1; sx++)
            {
               if ((sy == ship->y-4 || sy == ship->y+4) && sx != ship->x)
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 4)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-2; sx <= ship->x+2; sx++)
            {
               if ((sy == ship->y-4 || sy == ship->y+4) && (sx == ship->x-2 || sx == ship->x+2))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 5)
      {
         for (sy = ship->y-5; sy <= ship->y+5; sy++)
         {
            for (sx = ship->x-2; sx <= ship->x+2; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-2 || sx == ship->x+2))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 6)
      {
         for (sy = ship->y-5; sy <= ship->y+5; sy++)
         {
            for (sx = ship->x-3; sx <= ship->x+3; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-3 || sx == ship->x+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 7)
      {
         for (sy = ship->y-6; sy <= ship->y+6; sy++)
         {
            for (sx = ship->x-3; sx <= ship->x+3; sx++)
            {
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx == ship->x-3 || sx == ship->x+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 8)
      {
         for (sy = ship->y-6; sy <= ship->y+6; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-4 || sx == ship->x+4))
                  continue;
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx <= ship->x-3 || sx >= ship->x+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 9)
      {
         for (sy = ship->y-7; sy <= ship->y+7; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx == ship->x-4 || sx == ship->x+4))
                  continue;
               if ((sy == ship->y-7 || sy == ship->y+7) && (sx <= ship->x-3 || sx >= ship->x+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 10)
      {
         for (sy = ship->y-7; sy <= ship->y+7; sy++)
         {
            for (sx = ship->x-5; sx <= ship->x+5; sx++)
            {
               if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-5 || sx == ship->x+5))
                  continue;
               if ((sy == ship->y-6 || sy == ship->y+6) && (sx <= ship->x-4 || sx >= ship->x+4))
                  continue;
               if ((sy == ship->y-7 || sy == ship->y+7) && (sx <= ship->x-3 || sx >= ship->x+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
   }
   else
   {
      if (ship->size == 2)
      {
         for (sy = ship->y-1; sy <= ship->y+1; sy++)
         {
            for (sx = ship->x-3; sx <= ship->x+3; sx++)
            {
               if ((sx == ship->x-3 || sx == ship->x+3) && sy != ship->y)
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 3)
      {
         for (sy = ship->y-1; sy <= ship->y+1; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sx == ship->x-4 || sx == ship->x+4) && sy != ship->y)
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 4)
      {
         for (sy = ship->y-2; sy <= ship->y+2; sy++)
         {
            for (sx = ship->x-4; sx <= ship->x+4; sx++)
            {
               if ((sx == ship->x-4 || sx == ship->x+4) && (sy == ship->y-2 || sy == ship->y+2))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 5)
      {
         for (sy = ship->y-2; sy <= ship->y+2; sy++)
         {
            for (sx = ship->x-5; sx <= ship->x+5; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-2 || sy == ship->y+2))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 6)
      {
         for (sy = ship->y-3; sy <= ship->y+3; sy++)
         {
            for (sx = ship->x-5; sx <= ship->x+5; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-3 || sy == ship->y+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 7)
      {
         for (sy = ship->y-3; sy <= ship->y+3; sy++)
         {
            for (sx = ship->x-6; sx <= ship->x+6; sx++)
            {
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy == ship->y-3 || sy == ship->y+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 8)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-6; sx <= ship->x+6; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-4 || sy == ship->y+4))
                  continue;
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy <= ship->y-3 || sy >= ship->y+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 9)
      {
         for (sy = ship->y-4; sy <= ship->y+4; sy++)
         {
            for (sx = ship->x-7; sx <= ship->x+7; sx++)
            {
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy == ship->y-4 || sy == ship->y+4))
                  continue;
               if ((sx == ship->x-7 || sx == ship->x+7) && (sy <= ship->y-3 || sy >= ship->y+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
      if (ship->size == 10)
      {
         for (sy = ship->y-5; sy <= ship->y+5; sy++)
         {
            for (sx = ship->x-7; sx <= ship->x+7; sx++)
            {
               if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-5 || sy == ship->y+5))
                  continue;
               if ((sx == ship->x-6 || sx == ship->x+6) && (sy <= ship->y-4 || sy >= ship->y+4))
                  continue;
               if ((sx == ship->x-7 || sx == ship->x+7) && (sy <= ship->y-3 || sy >= ship->y+3))
                  continue;
               if (sx < 1 || sx > MAX_X || sy < 1 || sy > MAX_Y)
                  return 0;
               if (map_sector[ship->map][sx][sy] != SECT_OCEAN)
                  return 0;
            }
         }
      }
   }   
   return 1;
}   

SHIP_DATA *is_ship_sector(int x, int y, int map)
{
   SHIP_DATA *ship;
   int sx;
   int sy;
   
   for (ship = first_ship; ship; ship = ship->next)
   {
      if (ship->x == x && ship->y == y && ship->map == map)
         return ship;
      //Do a proximity check.  No need to loop through all of this if we aren't even near a ship
      if ((ship->direction == 0 || ship->direction == 2) && (abs(x + y - ship->x - ship->y) < ship->size+3))
      {
         if (ship->size == 2)
         {
            for (sy = ship->y-3; sy <= ship->y+3; sy++)
            {
               for (sx = ship->x-1; sx <= ship->x+1; sx++)
               {
                  if ((sy == ship->y-3 || sy == ship->y+3) && sx != ship->x)
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 3)
         {
            for (sy = ship->y-4; sy <= ship->y+4; sy++)
            {
               for (sx = ship->x-1; sx <= ship->x+1; sx++)
               {
                  if ((sy == ship->y-4 || sy == ship->y+4) && sx != ship->x)
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 4)
         {
            for (sy = ship->y-4; sy <= ship->y+4; sy++)
            {
               for (sx = ship->x-2; sx <= ship->x+2; sx++)
               {
                  if ((sy == ship->y-4 || sy == ship->y+4) && (sx == ship->x-2 || sx == ship->x+2))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 5)
         {
            for (sy = ship->y-5; sy <= ship->y+5; sy++)
            {
               for (sx = ship->x-2; sx <= ship->x+2; sx++)
               {
                  if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-2 || sx == ship->x+2))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 6)
         {
            for (sy = ship->y-5; sy <= ship->y+5; sy++)
            {
               for (sx = ship->x-3; sx <= ship->x+3; sx++)
               {
                  if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-3 || sx == ship->x+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 7)
         {
            for (sy = ship->y-6; sy <= ship->y+6; sy++)
            {
               for (sx = ship->x-3; sx <= ship->x+3; sx++)
               {
                  if ((sy == ship->y-6 || sy == ship->y+6) && (sx == ship->x-3 || sx == ship->x+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 8)
         {
            for (sy = ship->y-6; sy <= ship->y+6; sy++)
            {
               for (sx = ship->x-4; sx <= ship->x+4; sx++)
               {
                  if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-4 || sx == ship->x+4))
                     continue;
                  if ((sy == ship->y-6 || sy == ship->y+6) && (sx <= ship->x-3 || sx >= ship->x+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 9)
         {
            for (sy = ship->y-7; sy <= ship->y+7; sy++)
            {
               for (sx = ship->x-4; sx <= ship->x+4; sx++)
               {
                  if ((sy == ship->y-6 || sy == ship->y+6) && (sx == ship->x-4 || sx == ship->x+4))
                     continue;
                  if ((sy == ship->y-7 || sy == ship->y+7) && (sx <= ship->x-3 || sx >= ship->x+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 10)
         {
            for (sy = ship->y-7; sy <= ship->y+7; sy++)
            {
               for (sx = ship->x-5; sx <= ship->x+5; sx++)
               {
                  if ((sy == ship->y-5 || sy == ship->y+5) && (sx == ship->x-5 || sx == ship->x+5))
                     continue;
                  if ((sy == ship->y-6 || sy == ship->y+6) && (sx <= ship->x-4 || sx >= ship->x+4))
                     continue;
                  if ((sy == ship->y-7 || sy == ship->y+7) && (sx <= ship->x-3 || sx >= ship->x+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
      }
      else if ((ship->direction == 1 || ship->direction == 3) && (abs(x + y - ship->x - ship->y) < ship->size+3))
      {
         if (ship->size == 2)
         {
            for (sy = ship->y-1; sy <= ship->y+1; sy++)
            {
               for (sx = ship->x-3; sx <= ship->x+3; sx++)
               {
                  if ((sx == ship->x-3 || sx == ship->x+3) && sy != ship->y)
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 3)
         {
            for (sy = ship->y-1; sy <= ship->y+1; sy++)
            {
               for (sx = ship->x-4; sx <= ship->x+4; sx++)
               {
                  if ((sx == ship->x-4 || sx == ship->x+4) && sy != ship->y)
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 4)
         {
            for (sy = ship->y-2; sy <= ship->y+2; sy++)
            {
               for (sx = ship->x-4; sx <= ship->x+4; sx++)
               {
                  if ((sx == ship->x-4 || sx == ship->x+4) && (sy == ship->y-2 || sy == ship->y+2))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 5)
         {
            for (sy = ship->y-2; sy <= ship->y+2; sy++)
            {
               for (sx = ship->x-5; sx <= ship->x+5; sx++)
               {
                  if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-2 || sy == ship->y+2))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 6)
         {
            for (sy = ship->y-3; sy <= ship->y+3; sy++)
            {
               for (sx = ship->x-5; sx <= ship->x+5; sx++)
               {
                  if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-3 || sy == ship->y+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 7)
         {
            for (sy = ship->y-3; sy <= ship->y+3; sy++)
            {
               for (sx = ship->x-6; sx <= ship->x+6; sx++)
               {
                  if ((sx == ship->x-6 || sx == ship->x+6) && (sy == ship->y-3 || sy == ship->y+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 8)
         {
            for (sy = ship->y-4; sy <= ship->y+4; sy++)
            {
               for (sx = ship->x-6; sx <= ship->x+6; sx++)
               {
                  if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-4 || sy == ship->y+4))
                     continue;
                  if ((sx == ship->x-6 || sx == ship->x+6) && (sy <= ship->y-3 || sy >= ship->y+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 9)
         {
            for (sy = ship->y-4; sy <= ship->y+4; sy++)
            {
               for (sx = ship->x-7; sx <= ship->x+7; sx++)
               {
                  if ((sx == ship->x-6 || sx == ship->x+6) && (sy == ship->y-4 || sy == ship->y+4))
                     continue;
                  if ((sx == ship->x-7 || sx == ship->x+7) && (sy <= ship->y-3 || sy >= ship->y+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
         if (ship->size == 10)
         {
            for (sy = ship->y-5; sy <= ship->y+5; sy++)
            {
               for (sx = ship->x-7; sx <= ship->x+7; sx++)
               {
                  if ((sx == ship->x-5 || sx == ship->x+5) && (sy == ship->y-5 || sy == ship->y+5))
                     continue;
                  if ((sx == ship->x-6 || sx == ship->x+6) && (sy <= ship->y-4 || sy >= ship->y+4))
                     continue;
                  if ((sx == ship->x-7 || sx == ship->x+7) && (sy <= ship->y-3 || sy >= ship->y+3))
                     continue;
                  if (sx == x && sy == y && ship->map == map)
                     return ship;
               }
            }
         }
      }   
   }
   return NULL;
}

void update_ship_chars(SHIP_DATA *ship)
{
   CHAR_DATA *ch;
   
   for (ch = ship->first_char; ch; ch = ch->next_ship)
   {
      ch->coord->x = ship->x;
      ch->coord->y = ship->y;
      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
      do_look(ch, "auto");
   }
}

void update_object_contents(OBJ_DATA *obj, int x, int y)
{
   OBJ_DATA *cobj;
   obj->coord->x = x;
   obj->coord->y = y;
   
   for (cobj = obj->first_content; cobj; cobj = cobj->next_content)
   {
      update_object_contents(cobj, x, y);
   }
}

void steer_ship(CHAR_DATA *ch, SHIP_DATA *ship, int dir)
{
   int ox;
   int oy;
   int odir;
   int x;
   int y;
   ROOM_INDEX_DATA *room;
   char buf[MSL];
   OBJ_DATA *obj;
   
   x = y = 0;
   
   if (dir == 0)
      y = -1;
   if (dir == 1)
      x = 1;
   if (dir == 2)
      y = 1;
   if (dir == 3)
      x = -1;
   ox = ship->x;
   oy = ship->y;
   odir = ship->direction;
   set_ship_sector(ship, 1, 0);
   ship->x += x;
   ship->y += y;
   ship->direction = dir;
   if (!ch)
      ch = ship->first_char;
   if (check_ship_borders(ship))
   {
      if (ch)
      {
         sprintf(buf, "The ship sails to the %s", dir_name[dir]);
         act(AT_WHITE, buf, ch, NULL, NULL, TO_ROOM);
         act(AT_WHITE, buf, ch, NULL, NULL, TO_CHAR);
      }
      set_ship_sector(ship, 0, 1);
      update_ship_chars(ship);
      fwrite_ship_data();
      if (ch)
         room = ch->in_room;
      else
         room = get_room_index(OVERLAND_SOLAN);
      for (obj = room->first_content; obj; obj = obj->next_content)
      {
         if (obj->coord->x == ox && obj->coord->y == oy)
         {
            update_object_contents(obj, ship->x, ship->y);
         }
      }
      return;
   }
   else
   {
      ship->x -= x;
      ship->y -= y;
      ship->direction = odir;
      set_ship_sector(ship, 0, 0);
      if (ch)
         send_to_char("Something is blocking you from going that direction.\n\r", ch);
      return;
   }
}

void do_steership(CHAR_DATA *ch, char *argument)
{  
   int dir = -1;
   
   if (get_trust(ch) < LEVEL_IMMORTAL || IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  steership <direction>\n\r", ch);
      send_to_char("Ships can only move in 4 directions - north west south east.\n\r", ch);
      return;
   }
   if (!str_cmp(argument, "n") || !str_cmp(argument, "north"))
      dir = 0;
   if (!str_cmp(argument, "e") || !str_cmp(argument, "east"))
      dir = 1;
   if (!str_cmp(argument, "s") || !str_cmp(argument, "south"))
      dir = 2;
   if (!str_cmp(argument, "w") || !str_cmp(argument, "west"))
      dir = 3;
      
   if (dir == -1)
   {
      do_steership(ch, "");
      return;
   }
   if (!ch->ship)
   {
      send_to_char("You have to be on a ship to steer it.\n\r", ch);
      return;
   }
   steer_ship(ch, ch->ship, dir);
} 

void new_map_to_char(CHAR_DATA * ch, int startx, int starty, int endx, int endy, int showeoc)
{
   int x, y, p;
   int sx, sy;
   int stx, sty, enx, eny;
   int seeobj;
   int bfight, bobj, bportal, bcolor;
   sh_int seemap, seeportal, eoc, eeoc, istown;
   CMAP_DATA *mch;
   OMAP_DATA *mobj;
   //int bsize = ((endx - startx) * (endy - starty)) * 20;
   char buf[MSL];
   //char hbuf[bsize]; /* Sent in one huge buf, might save sending time */
   char hbuf[MSL];
   DESCRIPTOR_DATA *d;
   sh_int tbufx[10];
   sh_int tbufy[10];
   char fground[10];
   char rground[10];
   int cannotseeindoors = 0;
   sh_int tbx, ttorn, tcur, tlast;
   TORNADO_DATA *torn;
   TOWN_DATA *town;
   DOOR_DATA *ddata;
   int cnt, cx, cy;
   int curx, cury;
   int lastblank = 0;
   int isinside = 0;
   int dx;
   int fnddoor[100];
   int canseeinside[101][101];
   tcur = tlast = 0;

   for (tbx = 0; tbx < 10; tbx++)
   {
      tbufx[tbx] = 0;
      tbufy[tbx] = 0;
   }
   ttorn = 0; //Tornado present int

   sprintf(hbuf, "\n\r");
   bfight = bobj = bportal = 0;
   stx = startx;
   sty = starty;
   enx = endx;
   eny = endy;
   snows = 0;

   if (stx < 1)
      stx = 1;

   if (sty < 1)
      sty = 1;

   if (enx > MAX_X)
      enx = MAX_X;

   if (eny > MAX_Y)
      eny = MAX_Y;

   for (torn = first_tornado; torn; torn = torn->next)
   {
      tbx = 0;
      if (torn->x >= stx && torn->x <= enx && torn->y >= sty && torn->y <= eny && ch->map == torn->map)
      {
         tbufx[tbx] = torn->x;
         tbufy[tbx] = torn->y;
         tbx++;
         ttorn = 1;
      }
   }
   //time for Inside check....
   for (cx = 0; cx <= 99; cx++)
      fnddoor[cx] = 0;
        
   town = find_town(ch->coord->x, ch->coord->y, ch->map);
   
   if (town)
   {
      for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
         ddata->cannotsee = 0; 
   }
      
   curx = ch->coord->x;
   cury = ch->coord->y;
      
   if (town && (map_sector[ch->map][curx][cury] == SECT_INSIDE || map_sector[ch->map][curx][cury] == SECT_DOOR
   ||  map_sector[ch->map][curx][cury] == SECT_CDOOR || map_sector[ch->map][curx][cury] == SECT_LDOOR))
   {
      if (map_sector[ch->map][curx][cury] != SECT_INSIDE) //door
      {
         for (dx = 0; dx <= 99; dx++)
         {
            if (town->doorstate[5][dx] == curx && town->doorstate[6][dx] == cury && town->doorstate[7][dx] == ch->map) 
            {
               for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
               {
                  for (x = 0; x <= 9; x++)
                  {
                     if (ddata->doorvalue[x] == town->doorstate[4][dx])
                     {
                        for (y = 0; y <= MAX_HPOINTS-1; y++)
                        {
                           if (ddata->roomcoordx[y] > 0)
                           {
                              curx = ddata->roomcoordx[y];
                              cury = ddata->roomcoordy[y];
                              break;
                           }
                        }
                        if (curx != ch->coord->x && cury != ch->coord->y)
                           break;
                     }
                  }
                  if (curx != ch->coord->x && cury != ch->coord->y)
                     break;
               }
               if (curx != ch->coord->x && cury != ch->coord->y)
                  break;
            }
         }
      }
      if (map_sector[ch->map][curx][cury] == SECT_INSIDE)
      {
         for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
         {
            for (dx = 0; dx <= MAX_HPOINTS-1; dx++)
            {
               if (ddata->roomcoordx[dx] == curx && ddata->roomcoordy[dx] == cury &&
                   ddata->roomcoordmap[dx] == ch->map)
               {
                  if (update_inside_stat(fnddoor, ch, town, curx, cury, ch->map) == 0) //are indeed inside
                  {
                     isinside = 1;
                     for (x = 0; x <= 100; x++)
                     {
                        for (y = 0; y <= 100; y++)
                        {  
                           canseeinside[x][y] = 0;
                        }
                     }
                     canseeinside[50][50] = 1;
                     for (dx = 0; dx <= MAX_HPOINTS-1; dx++)
                     {
                        if (ddata->roomcoordx[dx] > 0)
                        {
                           for (cnt = 1; cnt <= 8; cnt++)
                           {
                              cx = ddata->roomcoordx[dx];
                              cy = ddata->roomcoordy[dx];
            
                              if (cnt == 1 || cnt == 5 || cnt == 7) //east
                                 cx = cx+1;
                              if (cnt == 2 || cnt == 7 || cnt == 8) //south
                                 cy = cy+1;
                              if (cnt == 3 || cnt == 6 || cnt == 8) //west
                                 cx = cx-1;
                              if (cnt == 4 || cnt == 5 || cnt == 6) //north
                                 cy = cy-1;  
                                    
                              canseeinside[50+ch->coord->x-cx][50+ch->coord->y-cy] = 1;
                           }
                        }
                     }
                     for (ddata = town->first_doorlist->first_door; ddata; ddata = ddata->next)
                     {
                        if (ddata->cannotsee == 1)
                        {
                           for (dx = 0; dx <= MAX_HPOINTS-1; dx++)
                           {
                              if (ddata->roomcoordx[dx] > 0)
                              {
                                 canseeinside[50+ch->coord->x-ddata->roomcoordx[dx]][50+ch->coord->y-ddata->roomcoordy[dx]] = 1;
                                 for (cnt = 1; cnt <= 8; cnt++)
                                 {
                                    cx = ddata->roomcoordx[dx];
                                    cy = ddata->roomcoordy[dx];
         
                                    if (cnt == 1 || cnt == 5 || cnt == 7) //east
                                       cx = cx+1;
                                    if (cnt == 2 || cnt == 7 || cnt == 8) //south
                                       cy = cy+1;
                                    if (cnt == 3 || cnt == 6 || cnt == 8) //west
                                       cx = cx-1;
                                    if (cnt == 4 || cnt == 5 || cnt == 6) //north
                                       cy = cy-1;  
                                    
                                    canseeinside[50+ch->coord->x-cx][50+ch->coord->y-cy] = 1;
                                 }
                              }
                           }
                        }
                     }
                     break;
                  }//end update_inside_stat brace
               }
            }
            if (isinside == 1)
               break;
         }
      }
   }                   
   //ch_printf(ch, "%s", MXPTAG("FRAME Name=\"Map\" FLOATING Left=\"-55c\" Top=\"0\" Width=\"55c\" Height=\"21c\""));     
   //ch_printf(ch, "%s %s %s", MXPTAG("DEST Map EOF"),  MXPTAG("/DEST"), MXPTAG("DEST Map"));  
   //ch_printf(ch, "%s", MXPTAG("DEST Map"));      
   if (xIS_SET(ch->act, PLR_MAPWINDOW) && !IS_NPC(ch))
   {
      if (ch->pcdata->xsize != endx-startx+2 && ch->pcdata->ysize != endy-starty+1)
      {
         ch_printf(ch, "%s" "FRAME Name=\"Map\" FLOATING Left=\"-%dc\" Top=\"0\" Width=\"%dc\" Height=\"%dc\"" "%s", MXP_BEG,
            endx-startx+2, endx-startx+2, endy-starty+1, MXP_END);
         ch->pcdata->xsize = endx-startx+2;
         ch->pcdata->ysize = endy-starty+1;
      }
      ch_printf(ch, "%s %s %s", MXPTAG("DEST Map EOF"),  MXPTAG("/DEST"), MXPTAG("DEST Map"));  
   } 
   for (y = sty; y < eny + 1; y += 1)
   {
      int fnd = 0;

      sy = y;
      for (x = stx; x < enx + 1; x += 1)
      {
         if (tcur == 1)
            tlast = 1;
         tcur = 0;
         seemap = 0;
         seeobj = 0;
         seeportal = 0;
         bcolor = 0;
         eoc = 0;
         eeoc = 0;
         istown = 0;
         bfight = UMAX(0, bfight - 1);
         bobj = UMAX(0, bobj - 1);
         bportal = UMAX(0, bportal - 1);

         if (ttorn == 1)
         {
            for (tbx = 0; tbx < 9; tbx++) //Tornado Present at this location
            {
               if (x == tbufx[tbx] && y == tbufy[tbx])
               {
                  strcat(hbuf, "&PT");
                  snows = 0;
                  fnd = 0;
                  tcur = 1;
                  break;
               }
            }
         }
         if (tcur == 1)
            continue;

         if (bfight > 0 || bobj > 0 || bportal > 0 || (ch->coord->x + 1 == x && ch->coord->y == y))
            bcolor = 1;

         sx = x;
         for (d = first_descriptor; d; d = d->next)
         {
            if (d->connected == CON_PLAYING
               && d->character != ch
               && d->character->in_room
               && d->newstate != 2
               && can_see_map(ch, d->character) && (d->character->coord->x + 1 == x && d->character->coord->y == y && d->character->map == ch->map))
            {
               bcolor = 1;
            }

            if (d->connected == CON_PLAYING
               && d->character != ch
               && d->character->in_room
               && d->newstate != 2
               && can_see_map(ch, d->character) && (d->character->coord->x == x && d->character->coord->y == y && d->character->map == ch->map))
            {
               fnd = 1;
               if (IS_AFFECTED(d->character, AFF_INVISIBLE) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
                  fnd = 0;

               if (IS_AFFECTED(d->character, AFF_HIDE) && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
                  fnd = 0;
                  
               if (IS_AFFECTED(d->character, AFF_STALK))
                  fnd = 0;

               if (xIS_SET(ch->act, PLR_HOLYLIGHT))
                  fnd = 1;

               if (xIS_SET(d->character->act, PLR_WIZINVIS) && get_trust(ch) < d->character->pcdata->wizinvis)
                  fnd = 0;
            }
         }
         for (p = 0; p < sysdata.last_portal; p++)
         {
            if (portal_show[p]->map == ch->map && portal_show[p]->x == x && portal_show[p]->y == y)
            {
               if (abs(x - ch->coord->x) <= 3 && abs(y - ch->coord->y) <= 3 && xIS_SET(ch->act, PLR_PORTALHUNT))
               {
                  seeportal = 1;
                  bportal = 2;
               }
               if (xIS_SET(ch->pcdata->portalfnd, p))
               {
                  seeportal = 1;
                  bportal = 2;
               }
            }
         }
         for (mobj = first_wilderobj; mobj; mobj = mobj->next)
         {
            if ((mobj->mapobj->coord->x == x && mobj->mapobj->coord->y == y && mobj->mapobj->map == ch->map))
            {
               seeobj = 1;
               bobj = 2;
            }
            else
               continue;
            if (IS_OBJ_STAT(mobj->mapobj, ITEM_INVIS))
            {
               if (!IS_AFFECTED(ch, AFF_DETECT_INVIS) && !IS_AFFECTED(ch, AFF_TRUESIGHT))
               {
                  seeobj = 0;
                  bobj = 0;
               }
            }
            if (mobj->mapobj->item_type == ITEM_PORTAL)
            {
               seeobj = 0;
               bobj = 0;
            }
            if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT))
            {
               seeobj = 1;
               bobj = 2;
            }
         }
         if (showeoc && kingdom_sector[ch->map][x][y] > 1 && kingdom_sector[ch->map][x][y] == ch->pcdata->hometown)
            eoc = 1;
         if (showeoc && kingdom_sector[ch->map][x][y] > 1 && kingdom_sector[ch->map][x][y] != ch->pcdata->hometown)
         {
            if (kingdom_table[ch->pcdata->hometown]->peace[kingdom_sector[ch->map][x][y]] == 0) //war
               eeoc = 1;
            if (kingdom_table[ch->pcdata->hometown]->peace[kingdom_sector[ch->map][x][y]] == 1) //neutral
               eeoc = 2;
            if (kingdom_table[ch->pcdata->hometown]->peace[kingdom_sector[ch->map][x][y]] == 2) //trading
               eeoc = 3;
            if (kingdom_table[ch->pcdata->hometown]->peace[kingdom_sector[ch->map][x][y]] == 3) //peace
               eeoc = 4;
         }
         if (eoc || eeoc)
         {
            for (town = kingdom_table[kingdom_sector[ch->map][x][y]]->first_town; town; town = town->next)
            {
               if (town->startx == x && town->starty == y && town->startmap == ch->map)
                  istown = 1;
            }
         }
         if (!can_see_room(x, y, ch->map, map_sector[ch->map][x][y]))
            cannotseeindoors = 1;
         if (isinside == 1 && canseeinside[50+ch->coord->x-x][50+ch->coord->y-y] == 0)
            cannotseeindoors = 1;
         if (isinside == 1 && canseeinside[50+ch->coord->x-x][50+ch->coord->y-y] == 1)
            cannotseeindoors = 0; 
         for (mch = first_wilderchar; mch; mch = mch->next)
         {
            if (mch->mapch->coord->x == x && mch->mapch->coord->y == y && mch->mapch->map == ch->map && cannotseeindoors == 0)
            {
               seemap = 1;
               /* seemap = 0 see only terrain 1 = see vis mobs 2 = see hide mobs 3 = see invis mobs 4 = see both */
               if (!xIS_SET(ch->act, PLR_HOLYLIGHT) || !IS_AFFECTED(ch, AFF_TRUESIGHT))
               {
                  if (IS_AFFECTED(mch->mapch, AFF_INVISIBLE) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
                     seemap = 0;
                  if (IS_AFFECTED(mch->mapch, AFF_HIDE) && !IS_AFFECTED(ch, AFF_DETECT_HIDDEN))
                     seemap = 0;
                  if (IS_AFFECTED(mch->mapch, AFF_STALK))
                     seemap = 0;


                  if (IS_AFFECTED(ch, AFF_BLIND))
                     seemap = 0;
               }
               if (IS_AFFECTED(ch, AFF_TRUESIGHT))
                  seemap = 1;

               if (xIS_SET(mch->mapch->act, ACT_MOBINVIS) && get_trust(ch) <= LEVEL_GUEST)
                  seemap = 0;
               if (xIS_SET(mch->mapch->act, ACT_TRAINER))
                  if ((abs(ch->coord->x - mch->mapch->coord->x) > 3) || (abs(ch->coord->y - mch->mapch->coord->y) > 3))
                     seemap = 0;
               if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_HOLYLIGHT))
                  seemap = 1;

               if (seemap == 1 && (IS_NPC(ch) || !xIS_SET(ch->act, PLR_WILDERTILES)))
               {
                  if (eoc == 1)
                     sprintf(fground, "^b");
                  else if (eeoc == 1)
                     sprintf(fground, "^r");
                  else if (eeoc == 2)
                     sprintf(fground, "^g");
                  else if (eeoc == 3)
                     sprintf(fground, "^w");
                  else if (eeoc == 4)
                     sprintf(fground, "^O");
                  else
                     sprintf(fground, "^c");
                  if (istown)
                  {
                     snows = 0;
                     sprintf(buf, "%s&RT&w^x", fground);
                     strcat(hbuf, buf);
                     break;
                  }
                  else if ((global_x[0] == x && global_y[0] == y) && (abs(global_x[0] - ch->coord->x) <= 3 && abs(global_y[0] - ch->coord->y) <= 3))
                  {
                     snows = 0;
                     sprintf(buf, "%s&C#^x", fground);
                     strcat(hbuf, buf);
                     break;
                  }
                  else
                  {
                     if (x == ch->coord->x && y == ch->coord->y)
                     {
                        sprintf(buf, "%s&R*^x", fground);
                        strcat(hbuf, buf);
                        snows = 0;
                     }
                     else if (seeportal)
                     {                   
                        sprintf(buf, "%s&BO^x", fground);
                        strcat(hbuf, buf);
                        snows = 0;
                     }
                     else if (fnd == 1)
                     {
                        snows = 0;
                        if (seeobj == 1)
                           sprintf(buf, "%s&p*^x", fground);
                        else
                           sprintf(buf, "%s&G&W*^x", fground);
                        strcat(hbuf, buf);
                     }
                     else
                     {
                        if (seeobj == 1)
                        {
                           sprintf(buf, "%s&p%s^x", fground, show_room(map_sector[ch->map][x][y], -20, -20, -20, -1));
                           snows = 0;
                        }
                        else
                        {
                           if (bcolor == 1 || tlast == 1)
                              sprintf(buf, "%s%s^x", fground, show_room(map_sector[ch->map][x][y], x, y, ch->map, x));
                           else if (lastblank == 1)
                           {
                              sprintf(buf, "%s%s^x", fground, show_room(map_sector[ch->map][x][y], -10, -10, -10, -1));
                              lastblank = 0;
                           }
                           else
                              sprintf(buf, "%s%s^x", fground, show_room(map_sector[ch->map][x][y], x, y, ch->map, stx));
                        }
                        strcat(hbuf, buf);
                     }
                     break;
                  }
               }
               else if (seemap == 1 && !IS_NPC(ch) && xIS_SET(ch->act, PLR_WILDERTILES))
               {
                  if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_WILDERTILES))
                  {
                     sprintf(buf, "\x03Image a3.gif align=bottom\x04");
                     strcat(hbuf, buf);
                     break;
                  }
               }
            }
         }//end mobile check
         if (seemap == 0 && cannotseeindoors == 0)
         {
            if (eoc == 1)
            {
               sprintf(fground, "^b");
               sprintf(rground, "^x");
            }
            else if (eeoc == 1)
            {
               sprintf(fground, "^r");
               sprintf(rground, "^x");
            }
            else if (eeoc == 2)
            {
               sprintf(fground, "^g");
               sprintf(rground, "^x");
            }
            else if (eeoc == 3)
            {
               sprintf(fground, "^w");
               sprintf(rground, "^x");
            }
            else if (eeoc == 4)
            {
               sprintf(fground, "^O");
               sprintf(rground, "^x");
            }
            else
            {
               strcpy(fground, "");
               strcpy(rground, "");
            }
            if (istown)
            {
               snows = 0;
               sprintf(buf, "%s&RT&w%s", fground, rground);
               strcat(hbuf, buf);
            }
            else if ((global_x[0] == x && global_y[0] == y) && (abs(global_x[0] - ch->coord->x) <= 3 && abs(global_y[0] - ch->coord->y) <= 3))
            {
               snows = 0;
               sprintf(buf, "%s&C#%s", fground, rground);
               strcat(hbuf, buf);
            }
            else
            {
               if (x == ch->coord->x && y == ch->coord->y)
               {
                  if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_WILDERTILES))
                  {
                     sprintf(buf, "\x03Image a.gif align=bottom\x04");
                     strcat(hbuf, buf);
                  }
                  else
                  {
                     sprintf(buf, "%s&R*%s", fground, rground);
                     strcat(hbuf, buf);
                  }
                  snows = 0;
               } 
               else if (seeportal)
               {
                  if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_WILDERTILES))
                  {
                     sprintf(buf, "\x03Image p2.gif align=bottom\x04");
                     strcat(hbuf, buf);
                  }
                  else
                  {
                     sprintf(buf, "%s&BO%s", fground, rground);
                     strcat(hbuf, buf);
                  }
                  snows = 0;
               }
               else if (fnd == 1)
               {
                  snows = 0;
                  if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_WILDERTILES))
                  {
                     sprintf(buf, "\x03Image a2.gif align=bottom\x04");
                     strcat(hbuf, buf);
                  }
                  else
                  {
                     if (seeobj == 1)
                        sprintf(buf, "%s&p*%s", fground, rground);
                     else
                        sprintf(buf, "%s&G&W*%s", fground, rground);                                             
                     strcat(hbuf, buf);
                  }
               }
               else
               {
                  if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_WILDERTILES))
                  {
                     sprintf(buf, "\x03Image %s align=bottom\x04", sect_show[map_sector[ch->map][x][y]].tilefile);
                     strcat(hbuf, buf);
                  }
                  else if (seeobj == 1)
                  {
                     sprintf(buf, "%s&p%s%s", fground, show_room(map_sector[ch->map][x][y], -20, -20, -20, -1), rground);
                     strcat(hbuf, buf);
                     snows = 0;
                  }
                  else
                  {
                     if (bcolor == 1 || tlast == 1)
                        sprintf(buf, "%s%s^x%s", fground, show_room(map_sector[ch->map][x][y], x, y, ch->map, x), rground);
                     else if (lastblank == 1)
                     {
                        sprintf(buf, "%s%s^x%s", fground, show_room(map_sector[ch->map][x][y], -10, -10, -10, -1), rground);
                        lastblank = 0;
                     }
                     else
                        sprintf(buf, "%s%s^x%s", fground, show_room(map_sector[ch->map][x][y], x, y, ch->map, stx), rground);
                     strcat(hbuf, buf);
                  }
               }
               x = sx;
               y = sy;
            }
            x = sx;
            y = sy;
         }
         if (cannotseeindoors == 1)
         {
            if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_WILDERTILES))
            {
               sprintf(buf, "\x03Image v.gif align=bottom\x04");
               strcat(hbuf, buf);
            }
            else
            {
               strcat(hbuf, " ");
               lastblank = 1; //to refresh color...
            }
         }
         fnd = 0;
         cannotseeindoors = 0;
      }
      /*
      NorthWest  North  NorthEast
             \     |     /
      West   -           -   East
             /     |     \
      Southwest  South  SouthEast  */
      
      if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_AUTOEXIT) && y >= endy-4 && ch->desc && ch->desc->mxp && !xIS_SET(ch->act, PLR_MAPWINDOW) && !xIS_SET(ch->act, PLR_WILDERTILES))
      {
         strcat(hbuf, "&w&G");
         if (y == endy-4)
            strcat(hbuf, "  " MXPFTAG ("Ex", "NorthWest", "/Ex") "  " MXPFTAG("Ex", "North", "/Ex") "  " MXPFTAG("Ex", "NorthEast", "/Ex"));
         if (y == endy-3)
            strcat(hbuf, "         \\     |     /");
         if (y == endy-2)
            strcat(hbuf, "  " MXPFTAG ("Ex", "West", "/Ex") "   -           -   " MXPFTAG ("Ex", "East", "/Ex"));
         if (y == endy-1)
            strcat(hbuf, "         /     |     \\");
         if (y == endy)
            strcat(hbuf, "  " MXPFTAG ("Ex", "SouthWest", "/Ex") "  " MXPFTAG("Ex", "South", "/Ex") "  " MXPFTAG("Ex", "SouthEast", "/Ex"));
      }        
      strcat(hbuf, "\n\r");
      send_to_char(hbuf, ch);
      strcpy(hbuf, "");
   }
   send_to_char(hbuf, ch);
   if (!IS_NPC(ch) && xIS_SET(ch->act, PLR_MAPWINDOW))
      ch_printf(ch, "%s", MXPTAG("/DEST"));
}

void display_map(CHAR_DATA * ch, sh_int vx, sh_int vy, sh_int eoc)
{
   sh_int startx, starty, endx, endy;
   sh_int sector;
   sh_int x, y, z;
   int mod;
   CHAR_DATA *sch;
   TOWN_DATA *town;

   sch = ch;

   if (ch->map == -1)
   {
      bug("display_map: Player %s on an invalid map! Moving them to Solan.", ch->name);
      send_to_char("&RYou were found on an invalid map and have been moved to Solan.\n\r", ch);
      enter_map(ch, 260, 250, MAP_SOLAN);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];

   if (IS_IMMORTAL(ch) && vx != 6000 && vy != 6000)
   {
      if (vx == -1 && vy == -1)
      {
         startx = ch->coord->x - 8;
         endx = ch->coord->x + 8;
         starty = ch->coord->y - 8;
         endy = ch->coord->y + 8;
      }
      else
      {
         startx = ch->coord->x - vx;
         endx = ch->coord->x + vx;
         starty = ch->coord->y - vy;
         endy = ch->coord->y + vy;
      }
   }
   else
   {
      OBJ_DATA *light;

      if ((vx == 5000 && vy == 5000))
      {
         mod = 8;
         if (!IS_AFFECTED(ch, AFF_WIZARDEYE) && !IS_AFFECTED(ch, AFF_E_WIZARDEYE) && !IS_AFFECTED(ch, AFF_M_WIZARDEYE))
         {
            light = get_eq_char(ch, WEAR_LIGHT);

            if (gethour() == 6 || gethour() == 21)
               mod = 7;

            if (gethour() > 21 || gethour() < 6)
               mod = 3;
               
            if (light != NULL || IS_AFFECTED(ch, AFF_INFRARED))
            {
               if (IS_AFFECTED(ch, AFF_INFRARED))
               {
                  if (gethour() > 21 || gethour() < 6)
                     mod += 4;
                  else
                     mod += 0;
               }
               else if (light->item_type == ITEM_LIGHT && (gethour() > 21 || gethour() < 6))
                  mod += 4;
            }
            if (!IS_NPC(ch) && LEARNED(ch, gsn_thiefeye) > 0)
            {
               if (MASTERED(ch, gsn_thiefeye) == 6)
                  mod += 5;
               if (MASTERED(ch, gsn_thiefeye) == 5)
                  mod += 4;
               if (MASTERED(ch, gsn_thiefeye) == 4)
                  mod += 3;
               if (MASTERED(ch, gsn_thiefeye) == 3)
                  mod += 2;
               if (MASTERED(ch, gsn_thiefeye) == 2)
                  mod += 1;
               if (MASTERED(ch, gsn_thiefeye) == 1)
                  mod += 1;
               learn_from_success(ch, gsn_thiefeye, NULL);
            }
                  
         }        
      }
      else if (vx == 6000 && vy == 6000) //scan
      {
         mod = 8;
         light = get_eq_char(ch, WEAR_LIGHT);

         if (gethour() == 6 || gethour() == 21)
            mod = 7;

         if (gethour() > 21 || gethour() < 6)
            mod = 3;
               
         if (light != NULL || IS_AFFECTED(ch, AFF_INFRARED))
         {
            if (IS_AFFECTED(ch, AFF_INFRARED))
            {
               if (gethour() > 21 || gethour() < 6)
                  mod += 4;
               else
                  mod += 0;
            }
            else if (light->item_type == ITEM_LIGHT && (gethour() > 21 || gethour() < 6))
               mod += 4;
         }
         if (MASTERED(ch, gsn_scan) == 5)
            mod += 7;
         else if (MASTERED(ch, gsn_scan) == 4)
            mod += 6;
         else if (MASTERED(ch, gsn_scan) == 3)
            mod += 4;
         else if (MASTERED(ch, gsn_scan) == 2)
            mod += 2;
         else if (MASTERED(ch, gsn_scan) == 1)
            mod += 1;
         else
            mod += 9;
         mod = UMAX(1, mod);
      }   
      else
      {
         mod = 5;
         if (!IS_AFFECTED(ch, AFF_WIZARDEYE) && !IS_AFFECTED(ch, AFF_E_WIZARDEYE) && !IS_AFFECTED(ch, AFF_M_WIZARDEYE))
         {
            light = get_eq_char(ch, WEAR_LIGHT);

            if (gethour() == 6 || gethour() == 21)
               mod = 4;

            if (gethour() > 21 || gethour() < 6)
               mod = 2;

            if (light != NULL || IS_AFFECTED(ch, AFF_INFRARED))
            {
               if (IS_AFFECTED(ch, AFF_INFRARED))
               {
                  if (gethour() > 21 || gethour() < 6)
                     mod += 2;
                  else
                     mod += 0;
               }
               else if (light->item_type == ITEM_LIGHT && (gethour() > 21 || gethour() < 6))
                  mod += 2;
            }
            if (!IS_NPC(ch) && LEARNED(ch, gsn_thiefeye) > 0)
            {
               if (MASTERED(ch, gsn_thiefeye) == 6)
                  mod += 5;
               if (MASTERED(ch, gsn_thiefeye) == 5)
                  mod += 4;
               if (MASTERED(ch, gsn_thiefeye) == 4)
                  mod += 3;
               if (MASTERED(ch, gsn_thiefeye) == 3)
                  mod += 2;
               if (MASTERED(ch, gsn_thiefeye) == 2)
                  mod += 1;
               if (MASTERED(ch, gsn_thiefeye) == 1)
                  mod += 1;
               learn_from_success(ch, gsn_thiefeye, NULL);
            }
         }
      }
      x = ch->coord->x;
      y = ch->coord->y;

      if (!IS_NPC(ch) && mod <= 12)
      {
         if (IS_AFFECTED(ch, AFF_WIZARDEYE))
            mod = UMAX(mod, 6);
         if (IS_AFFECTED(ch, AFF_E_WIZARDEYE))
            mod = UMAX(mod, 7);
         if (IS_AFFECTED(ch, AFF_M_WIZARDEYE))
            mod = UMAX(mod, 8);
      }

      startx = ch->coord->x - mod;
      starty = ch->coord->y - mod;
      endx = ch->coord->x + mod;
      endy = ch->coord->y + mod;
   }
   if (IS_PLR_FLAG(ch, PLR_MAPEDIT) && sector != SECT_EXIT)
   {
      map_sector[ch->map][ch->coord->x][ch->coord->y] = ch->pcdata->secedit;
      resource_sector[ch->map][ch->coord->x][ch->coord->y] = 0;
      sector = ch->pcdata->secedit;
   }
   new_map_to_char(ch, startx, starty, endx, endy, eoc);
   /* Kept dumping the character here, just put it back in ???  Hell if I know */
   ch = sch;
   if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] > 1)
   {
      for (town = kingdom_table[kingdom_sector[ch->map][ch->coord->x][ch->coord->y]]->first_town; town; town = town->next)
      {
         for (z = 1; z <= 150; z++)
         {
            if (town->roomcoords[z][0] == ch->coord->x && town->roomcoords[z][1] == ch->coord->y && town->roomcoords[z][2] == ch->map)
            {
               set_char_color(AT_RMNAME, ch);
               ch_printf(ch, "\n\r%s\n\r", town->roomtitles[z]);
               set_char_color(AT_RMNAME, ch);
               break;
            }
         }
         if (z < 151)
            break;
      }
   }
   if (IS_IMMORTAL(ch))
   {
      ch_printf(ch, "\n\r&GSector type: %s. Coordinates: %dX, %dY  Resources: %d  Kingdom: %d\n\r",
         sect_show[sector].desc, ch->coord->x, ch->coord->y,
         resource_sector[ch->map][ch->coord->x][ch->coord->y],
         kingdom_sector[ch->map][ch->coord->x][ch->coord->y]);

      if (IS_PLR_FLAG(ch, PLR_MAPEDIT))
      {
         ch_printf(ch, "&YYou are currently creating %s sectors.&z\n\r", sect_show[ch->pcdata->secedit].desc);
      }
   }
   return;
}

void update_player_container(CHAR_DATA * ch, OBJ_DATA *iobj)
{
   OBJ_DATA *cobj;
   int x, y, map;
   
   x = ch->coord->x;
   y = ch->coord->y;
   map = ch->map;
   
   for (cobj = iobj->first_content; cobj; cobj = cobj->next_content)
   {
      if (x > -1 || y > -1 || map > -1)
         SET_OBJ_STAT(cobj, ITEM_ONMAP);
      else
         REMOVE_OBJ_STAT(cobj, ITEM_ONMAP);

      cobj->coord->x = ch->coord->x;
      cobj->coord->y = ch->coord->y;
      cobj->map = ch->map;
      if (cobj->first_content)
         update_player_container(ch, cobj);
   }   
}

void update_objects(CHAR_DATA * ch, sh_int map, sh_int x, sh_int y)
{
   OBJ_DATA *iobj;
   
   /* Hrm, too lazy to go back and change, but don't really need the
      x, y and map, just make sure to update objects after you update
      the players/followers -- Xerves */
   x = ch->coord->x;
   y = ch->coord->y;
   map = ch->map;
   for (iobj = ch->last_carrying; iobj; iobj = iobj->prev_content)
   {
      if (x > -1 || y > -1 || map > -1)
         SET_OBJ_STAT(iobj, ITEM_ONMAP);
      else
         REMOVE_OBJ_STAT(iobj, ITEM_ONMAP);

      iobj->coord->x = ch->coord->x;
      iobj->coord->y = ch->coord->y;
      iobj->map = ch->map;
      if (iobj->first_content)
         update_player_container(ch, iobj);
   }
}

/* who - 1 only mount and player 2 - only player 3 - Check choords*/
void update_players_map(CHAR_DATA * ch, int x, int y, int map, int who, ROOM_INDEX_DATA * room)
{
   CHAR_DATA *nextinroom;
   CHAR_DATA *fch;
   sh_int type, pstatus;
   sh_int ox, oy, omap;

   if (room->vnum == OVERLAND_SOLAN)
   {
      type = 1;
      ox = ch->coord->x;
      oy = ch->coord->y;
      omap = ch->map;
      ch->map = map;
      ch->coord->x = x;
      ch->coord->y = y;
      update_objects(ch, x, y, map);
      SET_ONMAP_FLAG(ch);
   }
   else
   {
      type = 0;
      ox = ch->coord->x;
      oy = ch->coord->y;
      omap = ch->map;
      ch->coord->x = x;
      ch->coord->y = y;
      ch->map = map;
      update_objects(ch, x, y, map);
      REMOVE_ONMAP_FLAG(ch);
   }

   if (who == 2)
   {
      char_from_room(ch);
      char_to_room(ch, room);
      return;
   }
   if (ch->mount && ch->mount->con_rleg != -1 && ch->mount->con_lleg != -1)
   {
      if (ch->in_room && ch->mount->in_room && ch->in_room == ch->mount->in_room)
      {
         if (who == 3)
         {
            if (ch->mount->coord->x == ox && ch->mount->coord->y == oy && ch->mount->map == omap)
            {
               char_from_room(ch->mount);
               char_to_room(ch->mount, room);
               ch->mount->coord->x = x;
               ch->mount->coord->y = y;
               ch->mount->map = map;

               if (type == 1)
               {
                  SET_ONMAP_FLAG(ch->mount);
               }
               else
               {
                  REMOVE_ONMAP_FLAG(ch->mount);
               }
            }
         }
         else
         {
            char_from_room(ch->mount);
            char_to_room(ch->mount, room);
            ch->mount->coord->x = x;
            ch->mount->coord->y = y;
            ch->mount->map = map;

            if (type == 1)
            {
               SET_ONMAP_FLAG(ch->mount);
            }
            else
            {
               REMOVE_ONMAP_FLAG(ch->mount);
            }
         }
      }
   }
   if (who == 1)
   {
      char_from_room(ch);
      char_to_room(ch, room);
      return;
   }
   for (fch = ch->in_room->first_person; fch; fch = nextinroom)
   {
      nextinroom = fch->next_in_room;
      if (fch->con_rleg == -1 && fch->con_lleg == -1 && fch->position != POS_SHOVE && fch->position != POS_DRAG)
      {
         send_to_char("It is hard to move without any legs.\n\r", fch);
         continue;
      }
      if (fch != ch /* loop room bug fix here by Thoric */
         && fch->master == ch
         && (ch->in_room && fch->in_room)
         && (ch->in_room == fch->in_room) && (fch->position == POS_STANDING || fch->position == POS_MOUNTED) && fch != ch->mount)
      {
         if (who == 3)
         {
            if (fch->coord->x == ox && fch->coord->y == oy && fch->map == omap)
            {
               if (IS_NPC(fch))
               {
                  char_from_room(fch);
                  char_to_room(fch, room);
                  fch->coord->x = x;
                  fch->coord->y = y;
                  fch->map = map;
                  if (type == 1)
                     SET_ONMAP_FLAG(fch);
                  else
                     REMOVE_ONMAP_FLAG(fch);

                  update_objects(fch, x, y, map);
               }
               else
               {
                  if (type == 1)
                     SET_ONMAP_FLAG(fch);
                  else
                     REMOVE_ONMAP_FLAG(fch);

                  update_players_map(fch, x, y, map, who, room);
                  fch->coord->x = x;
                  fch->coord->y = y;
                  fch->map = map;
                  update_objects(fch, x, y, map);
               }
            }
         }
         else
         {
            if (IS_NPC(fch))
            {
               char_from_room(fch);
               char_to_room(fch, room);
               fch->coord->x = x;
               fch->coord->y = y;
               fch->map = map;
               if (type == 1)
                  SET_ONMAP_FLAG(fch);
               else
                  REMOVE_ONMAP_FLAG(fch);

               update_objects(fch, x, y, map);
            }
            else
            {
               fch->coord->x = x;
               fch->coord->y = y;
               fch->map = map;
               if (type == 1)
                  SET_ONMAP_FLAG(fch);
               else
                  REMOVE_ONMAP_FLAG(fch);

               update_players_map(fch, x, y, map, who, room);
               update_objects(fch, x, y, map);
            }
         }
      }
   }
   if (ch->rider)
   { 
      fch = ch->rider;
      char_from_room(fch);
      char_to_room(fch, room);
      fch->coord->x = x;
      fch->coord->y = y;
      fch->map = map;
      if (type == 1)
         SET_ONMAP_FLAG(fch);
      else
         REMOVE_ONMAP_FLAG(fch);

      update_players_map(fch, x, y, map, who, room);
      update_objects(fch, x, y, map);
   }
   pstatus = check_room_pk(ch);
   char_from_room(ch);
   char_to_room(ch, room);
   do_look(ch, "auto");
   return;
}

int find_mob_diff(int x, int y, int map)
{
   WBLOCK_DATA *wblock;
   
   for (wblock = first_wblock; wblock; wblock = wblock->next)
   {
      if (x >= wblock->stx && y >= wblock->sty && x <= wblock->endx && y <= wblock->endy && map == wblock->map)
      {
         return wblock->lvl;
      }
   }
   return 1;
}

//7> has a chance for a dual wield
int get_wilder_weapon(int lvl, int dual)
{
   int chance;
   
   if (lvl <= 2)
      chance = number_range(1, 12);
   else if (lvl <= 4)
      chance = number_range(7, 18);
   else if (lvl <= 6)
      chance = number_range(19, 32);
   else if (lvl <= 9)
      chance = number_range(29, 38);
   else if (lvl <= 12)
      chance = number_range(35, 45);
   else
      chance = number_range(39, 45);
   
   if (dual > 0 && lvl >= 7)
   {
      if (lvl <= 9)
         chance = number_range(13, 28);
      if (lvl > 9)
         chance = number_range(19, 28);
   }
   
   if (chance == 1)   
      return OBJ_FORGE_HAND_AXE;
   if (chance == 2)
      return OBJ_FORGE_SHORT_SWORD;
   if (chance == 3)   
      return OBJ_FORGE_KNIFE;
   if (chance == 4)   
      return OBJ_FORGE_CLUB;
   if (chance == 5)   
      return OBJ_FORGE_SCEPTRE;
   if (chance == 6)
      return OBJ_FORGE_PILUM;
   if (chance == 7)   
      return OBJ_FORGE_AXE;
   if (chance == 8)   
      return OBJ_FORGE_CUTLASS;
   if (chance == 9)   
      return OBJ_FORGE_DAGGER;
   if (chance == 10)   
      return OBJ_FORGE_HAMMER;
   if (chance == 11)   
      return OBJ_FORGE_ROD;
   if (chance == 12)
      return OBJ_FORGE_LANCE;
   if (chance == 13)   
      return OBJ_FORGE_WARAXE;
   if (chance == 14)   
      return OBJ_FORGE_RAPIER;
   if (chance == 15)   
      return OBJ_FORGE_DIRK;
   if (chance == 16)   
      return OBJ_FORGE_KRIS;
   if (chance == 17)   
      return OBJ_FORGE_MACE;
   if (chance == 18)   
      return OBJ_FORGE_WEIGHTED_ROD;
   if (chance == 19)   
      return OBJ_FORGE_DOUBLE_AXE;
   if (chance == 20)   
      return OBJ_FORGE_KATANA;
   if (chance == 21)   
      return OBJ_FORGE_BROAD_SWORD;
   if (chance == 22)   
      return OBJ_FORGE_CLEAVER;
   if (chance == 23)   
      return OBJ_FORGE_MAIN_GAUCHE;
   if (chance == 24)   
      return OBJ_FORGE_WAR_HAMMER;
   if (chance == 25)   
      return OBJ_FORGE_STAFF;
   if (chance == 26)   
      return OBJ_FORGE_LONG_SWORD;
   if (chance == 27)   
      return OBJ_FORGE_STILETTO;
   if (chance == 28)                      //Last 1h weapon
      return OBJ_FORGE_FLAIL;
   if (chance == 29)   
      return OBJ_FORGE_MATTOCK;
   if (chance == 30)   
      return OBJ_FORGE_BASTARD_SWORD;
   if (chance == 31)   
      return OBJ_FORGE_SPEAR;
   if (chance == 32)   
      return OBJ_FORGE_HALBERD;
   if (chance == 33)   
      return OBJ_FORGE_GREAT_FLAIL;
   if (chance == 34)   
      return OBJ_FORGE_QUARTER_STAFF;
   if (chance == 35)   
      return OBJ_FORGE_GREAT_AXE;
   if (chance == 36)   
      return OBJ_FORGE_CLAYMORE;
   if (chance == 37)   
      return OBJ_FORGE_GLAIVE;
   if (chance == 38)   
      return OBJ_FORGE_GUISARME;
   if (chance == 39)   
      return OBJ_FORGE_MORNING_STAR;
   if (chance == 40)   
      return OBJ_FORGE_BATTLE_STAFF;
   if (chance == 41)   
      return OBJ_FORGE_BATTLE_AXE;
   if (chance == 42)   
      return OBJ_FORGE_FLAMBERGE;
   if (chance == 43)   
      return OBJ_FORGE_TRIDENT;
   if (chance == 44)   
      return OBJ_FORGE_MAUL;
   if (chance == 45)   
      return OBJ_FORGE_BLADED_STAFF;
      
   return OBJ_FORGE_HAND_AXE;
}

int get_slab_vnum(int lvl, int num)
{
    int num1;
    int num2;
    int num3;
    int per1;
    int per2;
    int per3;
    int x;
    
    num1=num2=num3=per1=per2=per3=0;

    if (lvl >= 121)
    {
       x = number_range(1, 5);      
       num1 = OBJ_FORGE_ANGANAR + ((x-1)*2);
       num2 = OBJ_FORGE_MAEGLUIN + ((x-1)*2);
       per1 = UMIN(100, (3 * (lvl - 121))+3);
       per2 = 100-per1;
    }
       
    if (lvl >= 101)
    {
       x = number_range(1, 5);
       x = OBJ_FORGE_MAEGLUIN + ((x-1)*2);
       num1 = x;
       num2 = OBJ_FORGE_D_MITHRIL;
       num3 = OBJ_FORGE_MITHRILSLAB;
       per1 = UMIN(100, (5 * (lvl - 101))+5);
       per2 = UMIN(100-per1, (10 * (lvl-95))+10);
       per3 = 100-per1-per2;
    }
    else if (lvl >= 95)
    {
       num1 = OBJ_FORGE_D_MITHRIL;
       num2 = OBJ_FORGE_MITHRILSLAB;
       num3 = OBJ_FORGE_TUNGSTEN;
       per1 = UMIN(100, (10 * (lvl - 95))+10);
       per2 = UMIN(100-per1, (10 * (lvl - 90))+10);
       per3 = 100-per1-per2;
    }
    else if (lvl >= 91)
    {
       num1 = OBJ_FORGE_MITHRILSLAB;
       num2 = OBJ_FORGE_TUNGSTEN;
       per1 = UMIN(100, (10 * (lvl - 91))+10);
       per2 = 100-per1;
    }
    else if (lvl >= 75)
    {
       num1 = OBJ_FORGE_TUNGSTEN;
       num2 = OBJ_FORGE_TITANIUM;
       per1 = UMIN(100, (10 * (lvl - 75))+10);
       per2 = 100-per1;
    }
    else if (lvl >= 65)
    {
       num1 = OBJ_FORGE_TITANIUM;
       num2 = OBJ_FORGE_COBALT;
       per1 = UMIN(100, (10 * (lvl - 65))+10);
       per2 = 100-per1;
    }
    else if (lvl >= 55)
    {
       num1 = OBJ_FORGE_COBALT;
       num2 = OBJ_FORGE_BRASS;
       per1 = UMIN(100, (10 * (lvl - 55))+10);
       per2 = 100-per1;
    }
    else if (lvl >= 45)
    {
       num1 = OBJ_FORGE_BRASS;
       num2 = OBJ_FORGE_D_STEELSLAB;
       per1 = UMIN(100, (10 * (lvl - 45))+10);
       per2 = 100-per1;
    }
    else if (lvl >= 31)
    {
       num1 = OBJ_FORGE_D_STEELSLAB;
       num2 = OBJ_FORGE_STEELSLAB;
       num3 = OBJ_FORGE_IRONSLAB;
       per1 = UMIN(100, (10 * (lvl - 31))+10);
       per2 = UMIN(100-per1, (10 * (lvl - 25))+10);
       per3 = 100-per1-per2;
    }
    else if (lvl >= 25)
    {
       num1 = OBJ_FORGE_STEELSLAB;
       num2 = OBJ_FORGE_IRONSLAB;
       per1 = UMIN(100, (10 * (lvl - 25))+10);
       per2 = 100-per1;
    }
    else if (lvl >= 15)
    {
       num1 = OBJ_FORGE_IRONSLAB;
       num2 = OBJ_FORGE_BRONZESLAB;
       per1 = UMIN(100, (10 * (lvl - 15))+10);
       per2 = 100-per1;
    }
    else if (lvl >= 5)
    {
       num1 = OBJ_FORGE_BRONZESLAB;
       num2 = OBJ_FORGE_COPPERSLAB;
       per1 = UMIN(100, (10 * (lvl - 5))+10);
       per2 = 100-per1;
    }
    else
    {
       num1 = OBJ_FORGE_COPPERSLAB;
       num2 = OBJ_FORGE_COPPERSLAB;
       per1 = 100;
       per2 = 100;
    }
    if (num == 1)
       return num1;
    if (num == 2)
       return num2;
    if (num == 3)
       return num3;
    if (num == 4)
       return per1;
    if (num == 5)
       return per2;
    if (num == 6)
       return per3;
       
    return num1;
}       
    

void adjust_wildermob(CHAR_DATA *mob, int lvl, int wilderness)
{
   OBJ_DATA *weapon;
   OBJ_DATA *rleg;
   OBJ_DATA *lleg;
   OBJ_DATA *rarm;
   OBJ_DATA *larm;
   OBJ_DATA *neck;
   OBJ_DATA *chest;
   OBJ_DATA *head;
   SLAB_DATA *firstslab;
   SLAB_DATA *secondslab;
   SLAB_DATA *thirdslab = NULL;
   int dam;
   int mult;
   int inc;
   int race;
   int num;
   int hp;
   int slab1;
   int slab2;
   int slab3;
   int per1;
   int per2;
   int per3;                                                  
   int x;
   
   weapon=rleg=lleg=rarm=larm=neck=chest=head=NULL;
   
   mult = 4750;
   inc = 750;
      
   for (x = 11; x <= lvl; x = x+10)
   {
      inc += 250;
      if (x <= 101)
         mult += inc;

   }
   mult += 250;
   inc += 250 * (lvl-x+10) /10;
   if (lvl <= 100)
      mult += inc;     
        
   hp = lvl * mult / 2000;
   hp = number_range(hp * 90, hp * 110)/100;
      
   mob->hit = hp;
   mob->max_hit = mob->hit;
   mob->perm_agi = UMIN(100, 35+(lvl*4/10));
   
   if (lvl > 100)
      mob->perm_agi += 5;
   if (lvl > 110)
      mob->perm_agi += 5;
   if (lvl > 120)
      mob->perm_agi += 5;
   
   if (lvl <= 8)
   {
      if (lvl <= 3)
         mob->armor = 1;
      else if (lvl <= 6)
         mob->armor = 2;
      else
         mob->armor = 3;
   }
   else
   {
      mob->armor = 3+(lvl*100/666);
   }
   mob->tohitbash = UMAX(1, number_range(mob->armor-1, mob->armor+1));
   mob->tohitslash = UMAX(1, number_range(mob->armor-1, mob->armor+1));
   mob->tohitstab = UMAX(1, number_range(mob->armor-1, mob->armor+1));
   dam = 1+lvl/5;
   if (lvl >= 48)
      dam+=(lvl-38)/10;
   if (lvl >= 97)
      dam+=1;
   mob->pIndexData->damnodice = 1;
   mob->pIndexData->damsizedice = 1+number_range(dam/2, dam/5);
   mob->pIndexData->damplus = dam - ((1 + (mob->pIndexData->damnodice*mob->pIndexData->damsizedice))/2);
   if (lvl > 70)
   {
      dam = (lvl-70)/3;
      if (lvl > 100)
         dam += UMIN(20,(lvl-100));
      if (lvl > 120)
         dam += (lvl-120)*2;
      mob->damaddlow = dam * 75 / 100;
      mob->damaddhi = dam * 125 / 100;
   }
   if (mob->race >= 0 && mob->race < MAX_RACE)
      mob->gold = (lvl * lvl /2.5) + (lvl*20);
   else
      mob->gold = 0;
   if (wilderness == 0)
   {
      mob->pIndexData->tohitbash = mob->tohitbash;
      mob->pIndexData->tohitslash = mob->tohitslash;
      mob->pIndexData->tohitstab = mob->tohitstab;
      mob->pIndexData->ac = mob->armor;
      mob->pIndexData->gold = mob->gold;
      mob->pIndexData->perm_agi = mob->perm_agi;
      mob->pIndexData->damaddlow = mob->damaddlow;
      mob->pIndexData->damaddhi = mob->damaddhi;
      if (lvl > 100)
      {
         mob->pIndexData->hitnodice = UMAX(1, mob->hit/200);
         mob->pIndexData->hitsizedice = UMAX(1, mob->hit/50);
      }
      else if (lvl > 70)
      {
         mob->pIndexData->hitnodice = UMAX(1, mob->hit/100);
         mob->pIndexData->hitsizedice = UMAX(1, mob->hit/25);
      }
      else if (lvl > 40)
      {
         mob->pIndexData->hitnodice = UMAX(1, mob->hit/50);
         mob->pIndexData->hitsizedice = UMAX(1, mob->hit/12);
      }
      else if (lvl > 20)
      {
         mob->pIndexData->hitnodice = UMAX(1, mob->hit/25);
         mob->pIndexData->hitsizedice = UMAX(1, mob->hit/10);
      }
      else
      {
         mob->pIndexData->hitnodice = UMAX(1, mob->hit/25);
         mob->pIndexData->hitsizedice = UMAX(5, mob->hit/8);
      }
      mob->pIndexData->hitplus = mob->hit - ((mob->pIndexData->hitnodice*mob->pIndexData->hitsizedice)/2);
      mob->pIndexData->perm_str = 13+(lvl/12)+number_range(-2, 2);
      mob->pIndexData->perm_dex = 13+(lvl/12)+number_range(-2, 2);
      mob->pIndexData->perm_wis = 13+(lvl/12)+number_range(-2, 2);
      mob->pIndexData->perm_int = 13+(lvl/12)+number_range(-2, 2);
      mob->pIndexData->perm_lck = 13+(lvl/12)+number_range(-2, 2);
      mob->pIndexData->perm_wis = 13+(lvl/12)+number_range(-2, 2);
      if (lvl <= 10)
      {
         mob->pIndexData->perm_int = 6;
         mob->pIndexData->perm_wis = 6;
      }
   }
   if (mob->race >= 0 && mob->race < MAX_RACE && wilderness == 1)
   {
      int load = 0;
      
      if (number_range(1, 5) == 1)
         weapon = create_object(get_obj_index(get_wilder_weapon(((lvl-1)/10+1),0)), 1);

      if (number_range(1, 5) == 1)
      {
         load = 1;
         num = number_range(1, 8);
         if (num <= 2)
            neck = create_object(get_obj_index(OBJ_FORGE_AVENTAIL), 1);
         else if (num <= 5)
            neck = create_object(get_obj_index(OBJ_FORGE_COIF), 1);
         else if (num <= 8)
            neck = create_object(get_obj_index(OBJ_FORGE_DOUBLE_COIF), 1);
      }
      
      if (number_range(1, 5) == 1)   
      {
         load = 1;
         num = number_range(1, 10);
         if (num <= 4)
            head = create_object(get_obj_index(OBJ_FORGE_CABASSET), 1);
         else if (num <= 8)
            head = create_object(get_obj_index(OBJ_FORGE_CASQUE), 1);
         else if (num <= 10)
            head = create_object(get_obj_index(OBJ_FORGE_ARMET), 1);
      }
      
      if (number_range(1, 5) == 1)   
      {    
         load = 1;  
         num = number_range(1, 8);
         if (num <= 3)
         {
            rarm = create_object(get_obj_index(OBJ_FORGE_CHAIN_GAUNTLET), 1);
            larm = create_object(get_obj_index(OBJ_FORGE_CHAIN_GAUNTLET), 1);  
         }
         else if (num <= 6)
         {
            rarm = create_object(get_obj_index(OBJ_FORGE_RING_GAUNTLET), 1);
            larm = create_object(get_obj_index(OBJ_FORGE_RING_GAUNTLET), 1);  
         }
         
         else if (num <= 8)
         {
            rarm = create_object(get_obj_index(OBJ_FORGE_GAUNTLET), 1);   
            larm = create_object(get_obj_index(OBJ_FORGE_GAUNTLET), 1);   
         }
      }
      if (number_range(1, 5) == 1)   
      {
         load = 1;
         num = number_range(1, 8);
         if (num <= 3)
         {
            rleg = create_object(get_obj_index(OBJ_FORGE_CHAIN_GREAVE), 1);
            lleg = create_object(get_obj_index(OBJ_FORGE_CHAIN_GREAVE), 1);
         }
         else if (num <= 6)
         {
            rleg = create_object(get_obj_index(OBJ_FORGE_RING_GREAVE), 1);
            lleg = create_object(get_obj_index(OBJ_FORGE_RING_GREAVE), 1);
         }
         else if (num <= 8)
         {
            rleg = create_object(get_obj_index(OBJ_FORGE_GREAVE), 1);
            lleg = create_object(get_obj_index(OBJ_FORGE_GREAVE), 1);
         }   
      }
      if (number_range(1, 5) == 1 || load == 0)   
      {
         num = number_range(1, 10);
         if (num <= 4)
            chest = create_object(get_obj_index(OBJ_FORGE_CHAIN_MAIL), 1);
         else if (num <= 7)
            chest = create_object(get_obj_index(OBJ_FORGE_CHAIN_HAUBERK), 1);
         else if (num <= 9)
            chest = create_object(get_obj_index(OBJ_FORGE_RING_MAIL), 1);
         else if (num <= 10)
            chest = create_object(get_obj_index(OBJ_FORGE_DOUBLE_RING_MAIL), 1);
      }
         
      slab1 = get_slab_vnum(lvl, 1);
      slab2 = get_slab_vnum(lvl, 2);
      slab3 = get_slab_vnum(lvl, 3);
      per1 = get_slab_vnum(lvl, 4);
      per2 = get_slab_vnum(lvl, 5);
      per3 = get_slab_vnum(lvl, 6);
                           
      for (firstslab = first_slab; firstslab; firstslab = firstslab->next)
      {
         if (firstslab->vnum == slab1)
            break;
      }
      for (secondslab = first_slab; secondslab; secondslab = secondslab->next)
      {
         if (secondslab->vnum == slab2)
            break;
      }
      if (slab3 > 0)
      {
         for (thirdslab = first_slab; thirdslab; thirdslab = thirdslab->next)
         {
            if (thirdslab->vnum == slab3)
               break;
         }
      }
      if (!firstslab)
      {
         bug("Adjust_Wildermob: %d is not a valid Slab", slab1);
      }
      else if (!secondslab)
      {
         bug("Adjust_Wildermob: %d is not a valid Slab", slab2);
      }
      else if (slab3 > 0 && !thirdslab)
      {
         bug("Adjust_Wildermob: %d is not a valid Slab", slab3);
      }
      else
      {
         race = mob->race;
         if (per3 > 0)
         {
            per3 = per1+per2;
         }
         if (mob->race < 0 || mob->race >= MAX_RACE)
            mob->race = 0; //Needs a valid race   
         if (weapon)
         {   
            if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, weapon, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, weapon, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, weapon, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, weapon, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, weapon, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }	
         if (neck)   
         {
            if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, neck, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, neck, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, neck, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, neck, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, neck, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }
         if (head)   
         {
            if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, head, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, head, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, head, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, head, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, head, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }
         if (rarm)   
         {
            if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, rarm, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, rarm, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, rarm, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, rarm, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, rarm, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }
         if (larm)   
         {
            if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, larm, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, larm, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, larm, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, larm, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, larm, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }
         if (rleg)   
         {
            if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, rleg, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, rleg, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, rleg, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, rleg, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, rleg, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }
         if (lleg)   
         {
            if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, lleg, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, lleg, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, lleg, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, lleg, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, lleg, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }
         if (chest)
         {
           if (per3 <= 0)
            {
               if (number_range(1, 100) <= per1)
                  alter_forge_obj(mob, chest, create_object(get_obj_index(firstslab->vnum), 1), firstslab);	
               else
                  alter_forge_obj(mob, chest, create_object(get_obj_index(secondslab->vnum), 1), secondslab);
            }
            else
            {
               num = number_range(1, 100);
               if (num <= per1)
                  alter_forge_obj(mob, chest, create_object(get_obj_index(firstslab->vnum), 1), firstslab);
               else if (num > per3)
                  alter_forge_obj(mob, chest, create_object(get_obj_index(thirdslab->vnum), 1), thirdslab);
               else
                  alter_forge_obj(mob, chest, create_object(get_obj_index(secondslab->vnum), 1), secondslab);   
            }
         }  
         mob->race = race;
      }       
      if (rleg)
      {
         obj_to_char(rleg, mob);
         equip_char(mob, rleg, WEAR_LEG_R);  
      } 
      if (lleg)
      {
         obj_to_char(lleg, mob);
         equip_char(mob, lleg, WEAR_LEG_L);   
      }
      if (larm)
      {
         obj_to_char(larm, mob);
         equip_char(mob, larm, WEAR_ARM_L);
      }
      if (rarm)
      {
         obj_to_char(rarm, mob);
         equip_char(mob, rarm, WEAR_ARM_R);  
      }
      if (chest)
      {
         obj_to_char(chest, mob);
         equip_char(mob, chest, WEAR_BODY); 
      }
      if (head)
      {
         obj_to_char(head, mob);
         equip_char(mob, head, WEAR_HEAD); 
      }
      if (neck)
      {
         obj_to_char(neck, mob);
         equip_char(mob, neck, WEAR_NECK); 
      }
      if (weapon)
      {
         obj_to_char(weapon, mob);
         equip_char(mob, weapon, WEAR_WIELD);
      }
   }
}

bool can_see_target(CHAR_DATA * ch, MOB_INDEX_DATA *mob)
{
   CHAR_DATA *fch;
   
   if (can_see_index(mob, ch))
   {
      return TRUE;
   }
   else
   {
      for (fch = ch->in_room->first_person; fch; fch = fch->next_in_room)
      {
         if (fch != ch
         && !IS_NPC(fch)
         && fch->master == ch)
         {
            if (can_see_index(mob, fch))
               return TRUE;
         }
      }
   }
   return FALSE;
}
bool load_mapmobiles(CHAR_DATA * ch, int ox, int oy)
{
   int vnum;
   int vnumlist[MAX_MOB_HOLDER];
   int x = 0;
   int xload = 0; //number actually found
   int y = 0;
   int vn;
   int numload;
   int goload;
   int lvl;
   int cnt = 0;
   int en = sect_show[(int)map_sector[ch->map][ox][oy]].encounter;
   char moblist[MSL];
   char buf[MSL];
   CHAR_DATA *victim;
   CHAR_DATA *fch;
   MOB_INDEX_DATA *mob;

   lvl = find_mob_diff(ch->coord->x, ch->coord->y, ch->map);

   for (vnum = OVERLAND_LOW_MOB; vnum < OVERLAND_HI_MOB; vnum++)
   {
      if ((mob = get_mob_index(vnum)) != NULL)
      {
         if (mob->m1 >= 1 && mob->m1 == lvl/10+1)
         {
            if (map_sector[ch->map][ch->coord->x][ch->coord->y] == SECT_UNDERWATER
               || map_sector[ch->map][ch->coord->x][ch->coord->y] == SECT_WATER_NOSWIM
               || map_sector[ch->map][ch->coord->x][ch->coord->y] == SECT_OCEAN)
            {
               if (!IS_ACT_FLAG(mob, ACT_PROTOTYPE) && !IS_ACT_FLAG(mob, ACT_SENTINEL) && !IS_ACT_FLAG(mob, ACT_TRAINER)
                  && !IS_ACT_FLAG(mob, ACT_MOVEMAP) && IS_ACT_FLAG(mob, ACT_WATERMOB))
               {
                  if (can_see_target(ch, mob)) //can the target party be seen, no sense loading mobs if they cannot
                  {
                     vnumlist[x] = mob->vnum;
                     x++;
                     xload++;
                  }
                  else
                  {
                     xload++;   
                  }
               }
            }
            else
            {
               if (!IS_ACT_FLAG(mob, ACT_PROTOTYPE) && !IS_ACT_FLAG(mob, ACT_SENTINEL) && !IS_ACT_FLAG(mob, ACT_TRAINER)
                  && !IS_ACT_FLAG(mob, ACT_MOVEMAP))
               {
                  if (map_sector[ch->map][ch->coord->x][ch->coord->y] == SECT_WATER_SWIM
                     || map_sector[ch->map][ch->coord->x][ch->coord->y] == SECT_RIVER)
                  {
                     if (can_see_target(ch, mob)) //can the target party be seen, no sense loading mobs if they cannot
                     {
                        vnumlist[x] = mob->vnum;
                        x++;
                        xload++;
                     }
                     else
                     {
                        xload++;   
                     }
                  }
                  else
                  {
                     if (!IS_ACT_FLAG(mob, ACT_WATERMOB))
                     {
                        if (can_see_target(ch, mob)) //can the target party be seen, no sense loading mobs if they cannot
                        {
                           vnumlist[x] = mob->vnum;
                           x++;
                           xload++;
                        }
                        else
                        {
                           xload++;   
                        }
                     }
                  }
               }
            }
         }
      }
   }
   if (xload == 0)
   {
      bug("load_mapmobiles: Not enough mobs for the level of %d", lvl);
      return FALSE;
   }
   if (x <= 0) //mobiles in this part cannot see our targets
      return FALSE;
   //Might as well add a bit more difficulty if there are larger groups.
   cnt = 1;
   for (fch = ch->in_room->first_person; fch; fch = fch->next_in_room)
   {
      if (fch != ch
      && !IS_NPC(fch)
      && fch->master == ch)
      {
         if (fch->coord->x < 1 || fch->coord->y < 1 || fch->map < 0)
            continue;
         if (!IS_WITHIN(fch->coord->x, 8) || !IS_WITHIN(fch->coord->y, 8) || ch->map != fch->map)
            continue;
            
         cnt++;
         if (cnt > 3)
            en++;
         if (cnt > 5)
            en++;
      }
   }
   numload = number_range(en - 1, en);
   numload = UMAX(1, numload);
   strcpy(moblist, "");
   for (goload = 1; goload <= numload; goload++)
   {
      vn = number_range(0, x-1);
      mob = get_mob_index(vnumlist[vn]);
      if (!mob)
      {
         bug("load_mapmobiles: Null mob vnum %d", vn);
         continue;
      }
      victim = create_mobile(mob);
      adjust_wildermob(victim, lvl, 1);
      char_to_room(victim, ch->in_room);
      victim->coord->x = number_range(-4, 4)+ch->coord->x;
      victim->coord->y = number_range(-4, 4)+ch->coord->y;
      victim->map = ch->map;
      SET_ONMAP_FLAG(victim);
      sprintf(buf, "%s\n\r", victim->short_descr);
      cnt = 0;
      if (goload == 1)
         sprintf(moblist, buf);
      else
         strcat(moblist, buf);
      for (fch = ch->in_room->first_person; fch; fch = fch->next_in_room)
      {
         if (fch != ch
         && !IS_NPC(fch)
         && fch->master == ch)
         {
            if (fch->coord->x < 1 || fch->coord->y < 1 || fch->map < 0)
               continue;
            if (!IS_WITHIN(fch->coord->x, 8) || !IS_WITHIN(fch->coord->y, 8) || ch->map != fch->map)
               continue;
            cnt++;
         }
      }
      cnt++; //add character
      cnt = number_range(1, cnt);
      y = 1;
      if (cnt != 1)
      {
         for (fch = ch->in_room->first_person; fch; fch = fch->next_in_room)
         {
            if (fch != ch
            && !IS_NPC(fch)
            && fch->master == ch)
            {
               if (fch->coord->x < 1 || fch->coord->y < 1 || fch->map < 0)
                  continue;
               if (!IS_WITHIN(fch->coord->x, 8) || !IS_WITHIN(fch->coord->y, 8) || ch->map != fch->map)
                  continue;
               y++;
               if (y == cnt)
                  break;
            }
         }
      }
      else
      {
         fch = ch;
      }
      if (can_see_map(victim, fch) || fch->level >= LEVEL_IMMORTAL) //make sure they target the visible player, hate for them to cheat this way :-)
         start_hunting(victim, fch);
      else
         find_next_hunt(victim, 1);
   }
   act(AT_CYAN, "\n\rYour enemies are.\n\r-------------------------------------", ch, NULL, NULL, TO_CHAR);
   send_to_char(moblist, ch);
   
   for (fch = ch->in_room->first_person; fch; fch = fch->next_in_room)
   {
      if (fch != ch
      && (fch->master == ch || fch->riding == ch)
      && (fch->position == POS_STANDING || fch->position == POS_MOUNTED || fch->position == POS_RIDING))
      {
         if (fch->coord->x > -1 || fch->coord->y > -1 || fch->map > -1)
            continue;
         if (!IS_WITHIN(fch->coord->x, 8) || !IS_WITHIN(fch->coord->y, 8) || ch->map != fch->map)
            continue;

         send_to_char("Your party is being attacked, prepare for battle.\n\r", ch);
         act(AT_CYAN, "\n\rYour enemies are.\n\r-------------------------------------", fch, NULL, NULL, TO_CHAR);
         send_to_char(moblist, fch);
      }
   }
   return TRUE;
}

bool map_battle(CHAR_DATA * ch, int ox, int oy, int map)
{
   sh_int en = sect_show[(int)map_sector[ch->map][ox][oy]].encounter;
   sh_int gofight = 0;

   if (get_trust(ch) >= LEVEL_IMMORTAL && !xIS_SET(ch->act, PLR_MMOBILES))
      return FALSE;

   if (IS_NPC(ch))
      return FALSE;
      
   if (wIS_SET(ch, ROOM_NO_MOB))
      return FALSE;
      
   if (map_sector[ch->map][ox][oy] == SECT_ROAD)
      en = 0;

   ch->fcounter++;
   if (ch->fcounter >= 11 && ch->fcounter <= 15)
   {
      if (number_range(1, 100) <= 1)
         gofight = 1;
   }
   if (ch->fcounter > 15 && ch->fcounter <= 20)
   {
      if (number_range(1, 100) <= UMAX(1, en - 1))
         gofight = 1;
   }
   if (ch->fcounter > 20 && ch->fcounter <= 30)
   {
      if (number_range(1, 100) <= en + 1)
         gofight = 1;
   }
   if (ch->fcounter > 30 && ch->fcounter <= 35)
   {
      if (number_range(1, 100) <= en + 2)
         gofight = 1;
   }
   if (ch->fcounter >= 36 && ch->fcounter <= 40)
   {
      if (number_range(1, 100) <= en + 4)
         gofight = 1;
   }
   if (ch->fcounter > 40)
   {
      if (number_range(1, 100) <= en + 6)
         gofight = 1;
   }

   if (gofight == 1)
   {
      ch->fcounter = 0;

      load_mapmobiles(ch, ox, oy);
      return TRUE;
   }
   return FALSE;
}

int get_wilderness_hunt_cost(CHAR_DATA *ch, int x, int y)
{
   int move;
   move = sect_show[(int)map_sector[ch->map][x][y]].move;
   
   move = number_range(move*.08, move*.12);
   if (move < 1)
      move = 1;
      
   move += number_range(1, 2);
   return move;
}

int ship_is_full(SHIP_DATA *ship, int extra)
{
   int capacity;
   if (ship->size == 1)
      capacity = 1;
   else if (ship->size == 2)
      capacity = 3;
   else if (ship->size == 3)
      capacity = 5;
   else if (ship->size == 4)
      capacity = 10;
   else if (ship->size == 5)
      capacity = 20;
   else if (ship->size == 6)
      capacity = 30;
   else if (ship->size == 7)
      capacity = 50;
   else if (ship->size == 8)
      capacity = 75;
   else if (ship->size == 9)
      capacity = 100;
   else
      capacity = 150;
      
   if (ship->occupants + extra >= capacity)
      return 1;
   else
      return 0;
}
void do_entership(CHAR_DATA *ch, char *argument)
{
   int x;
   int y;
   int ox;
   int oy;
   CHAR_DATA *fch;
   SHIP_DATA *ship;
   CHAR_DATA *nextinroom;
   int cnt = 0;
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("You can only enter a ship out in the wilderness.\n\r", ch);
      return;
   }
   if (ch->ship)
   {
      send_to_char("You are already aboard a ship.\n\r", ch);
      return;
   }
   if (ch->rider || ch->mount)
      cnt = 1;
   for (x = ch->coord->x-1; x <= ch->coord->x+1; x++)
   {
      for (y = ch->coord->y-1; y <= ch->coord->y+1; y++)
      {
         if ((ship = is_ship_sector(x, y, ch->map)) != NULL)
         {
            if (ship_is_full(ship, cnt))
            {
               send_to_char("The ship's is filled to its capacity, you cannot enter it.\n\r", ch);
               return;
            }
            act(AT_WHITE, "$n quickly boards the ship that is anchored in the water.", ch, NULL, NULL, TO_CANSEE);
            ch->ship = ship;
            ox = ch->coord->x;
            oy = ch->coord->y;
            ch->coord->x = ship->x;
            ch->coord->y = ship->y;
            LINK(ch, ship->first_char, ship->last_char, next_ship, prev_ship);
            ship->occupants++;
            act(AT_WHITE, "$n joins you on the ship.", ch, NULL, NULL, TO_CANSEE);
            act(AT_WHITE, "You quickly board the ship that is anchored in the water.", ch, NULL, NULL, TO_CHAR);
            update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
            if (ch->rider)
            {
               act(AT_ACTION, "You board the ship along with $N", ch->rider, NULL, ch, TO_CHAR);
               act(AT_WHITE, "$n quickly boards the ship on the back of $N.", ch->rider, NULL, ch, TO_CANSEE);
               ch->rider->ship = ship;
               ch->rider->coord->x = ship->x;
               ch->rider->coord->y = ship->y;
               LINK(ch->rider, ship->first_char, ship->last_char, next_ship, prev_ship);
               ship->occupants++;
               act(AT_WHITE, "$n joins you on the ship.", ch->rider, NULL, NULL, TO_CANSEE);
               act(AT_WHITE, "$N carries you into the ship anchored in the water.", ch->rider, NULL, ch, TO_CHAR);
               update_objects(ch->rider, ch->rider->coord->x, ch->rider->coord->y, ch->rider->map);
               do_look(ch->rider, "auto");
            }
            for (fch = ch->in_room->first_person; fch; fch = nextinroom)
            {
               nextinroom = fch->next_in_room;
               if (fch != ch /* loop room bug fix here by Thoric */
               && fch->master == ch && fch->position == POS_STANDING && fch->coord->x == ox 
               && fch->coord->y == oy && fch->map == ch->map)
               {
                  act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
                  do_entership(fch, "");
               }
            }               
            do_look(ch, "auto");
            return;
         }
      }
   }
   send_to_char("There is no ship anchored in the water for you to board.\n\r", ch);
   return;
}

void do_leaveship(CHAR_DATA *ch, char *argument)
{
   int x;
   int y;
   int tx;
   int ty;
   int ox;
   int oy;
   CHAR_DATA *fch;
   SHIP_DATA *ship;
   CHAR_DATA *nextinroom;
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("You can only enter a ship out in the wilderness.\n\r", ch);
      return;
   }
   if (!ch->ship)
   {
      send_to_char("You are not aboard a ship.\n\r", ch);
      return;
   }
   for (x = ch->coord->x-7; x <= ch->coord->x+7; x++)
   {
      for (y = ch->coord->y-7; y <= ch->coord->y+7; y++)
      {
         if ((ship = is_ship_sector(x, y, ch->map)) != NULL)
         {
            for (tx = x-1; tx <= x+1; tx++)
            {
               for (ty = y-1; ty <= y+1; ty++)
               {
                  if (sect_show[(int)map_sector[ch->map][tx][ty]].canpass)
                  {
                     act(AT_WHITE, "$n leaves the ship for dry land.", ch, NULL, NULL, TO_CANSEE);
                     ox = ch->coord->x;
                     oy = ch->coord->y;
                     ch->coord->x = tx;
                     ch->coord->y = ty;
                     UNLINK(ch, ch->ship->first_char, ch->ship->last_char, next_ship, prev_ship);
                     ch->ship->occupants--;
                     ch->ship = NULL;
                     act(AT_WHITE, "$n arrives from the ship that is anchored in the water.", ch, NULL, NULL, TO_CANSEE);
                     act(AT_WHITE, "You leave the ship for dry land.", ch, NULL, NULL, TO_CHAR);
                     update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
                     if (ch->rider)
                     {
                        act(AT_ACTION, "You leave the ship along with $N", ch->rider, NULL, ch, TO_CHAR);
                        act(AT_WHITE, "$n, on the back of $N, leaves the ship for dry land.", ch->rider, NULL, ch, TO_CANSEE);
                        ch->coord->x = tx;
                        ch->coord->y = ty;
                        UNLINK(ch, ch->ship->first_char, ch->ship->last_char, next_ship, prev_ship);
                        ch->ship->occupants--;
                        ch->ship = NULL;
                        act(AT_WHITE, "$n, on the back of $N, arrives from the ship that is anchored in the water.", ch->rider, NULL, ch, TO_CANSEE);
                        act(AT_WHITE, "$N carries you off the ship.", ch->rider, NULL, ch, TO_CHAR);
                        update_objects(ch->rider, ch->rider->coord->x, ch->rider->coord->y, ch->rider->map);
                        do_look(ch->rider, "auto");
                     }
                     for (fch = ch->in_room->first_person; fch; fch = nextinroom)
                     {
                        nextinroom = fch->next_in_room;
                        if (fch != ch /* loop room bug fix here by Thoric */
                        && fch->master == ch && fch->position == POS_STANDING && fch->coord->x == ox 
                        && fch->coord->y == oy && fch->map == ch->map)
                        {
                           act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
                           do_leaveship(fch, "");
                        }
                     }
                     do_look(ch, "auto");
                     return;
                  }
               }
            }
         }
      }
   }
   send_to_char("There is no dry land near your ship.\n\r", ch);
   return;
}
   

void process_exit(CHAR_DATA * ch, sh_int map, sh_int x, sh_int y, sh_int dir, sh_int first)
{
   char buf[MSL];
   sh_int sector = map_sector[map][x][y];
   int maxx, maxy, ht;
   int nosnow = -1;
   int move;
   char *txt;
   char *dtxt;
   bool drunk = FALSE;
   CHAR_DATA *fch;
   CHAR_DATA *gmob;
   CHAR_DATA *nextinroom;
   ROOM_INDEX_DATA *from_room;
   WBLOCK_DATA *wblock;
   OBJ_DATA *boat;
   OBJ_DATA *climb = NULL;
   int chars = 0, count = 0;
   int fx, fy, fmap;
   int level;

   from_room = ch->in_room;
   ht = -1;
   fx = ch->coord->x;
   fy = ch->coord->y;
   fmap = ch->map;

   if (!IS_NPC(ch))
   {
      if (IS_DRUNK(ch, 2) && (ch->position != POS_SHOVE) && (ch->position != POS_DRAG))
         drunk = TRUE;
   }

   if (ch->on != NULL)
   {
      act(AT_PLAIN, "Need to get off of $p before moving on.", ch, ch->on, NULL, TO_CHAR);
      return;
   }
   if (ch->con_rleg == -1 && ch->con_lleg == -1 && ch->position != POS_SHOVE && ch->position != POS_DRAG)
   {
      send_to_char("You cannot move without any legs.\n\r", ch);
      return;
   }
   if (IS_AFFECTED(ch, AFF_WEB) || IS_AFFECTED(ch, AFF_SNARE))
   {
      send_to_char("You are currently bound to this spot and cannot move.\n\r", ch);
      return;
   }
   if (ch->position == POS_RIDING)
   {
      send_to_char("You cannot move when you are on someone's back.\n\r", ch);
      return;
   }
   if (ch->ship)
   {
      send_to_char("You cannot move around on a ship.  To leave the ship type leaveship.\n\r", ch);
      return;
   }
   if (sector == SECT_QEXIT)
   {
      ROOM_INDEX_DATA *toroom = NULL;
      
      if (check_npc(ch))
         return;
      if (!ch->pcdata->quest)
      {
         send_to_char("You have to be on a quest to use this exit.\n\r", ch);
         return;
      }
      if (ch->rider)
      {
         send_to_char("You need to lose your rider before entering a quest.\n\r", ch);
         return;
      }
      if (x != ch->pcdata->quest->x || y != ch->pcdata->quest->y || map != ch->pcdata->quest->map)
      {
         send_to_char("This is not the exit to your quest.\n\r", ch);
         return;
      }
      if ((toroom = get_room_index(ch->pcdata->quest->questarea->low_r_vnum)) == NULL)
      {
         bug("Target vnum %d for map quest exit does not exist!", ch->pcdata->quest->questarea->low_r_vnum);
         send_to_char("Ooops. Something bad happened. Contact the immortals ASAP.\n\r", ch);
         return;
      }
      leave_map(ch, toroom, dir, 1);
      if (ch->mount)
         leave_map(ch->mount, toroom, dir, 1);

      for (fch = from_room->first_person; fch; fch = fch->next_in_room)
         chars++;

      for (fch = from_room->first_person; fch && (count < chars); fch = nextinroom)
      {
         nextinroom = fch->next_in_room;
         count++;
         if (fch != ch /* loop room bug fix here by Thoric */
         && fch->master == ch && fch->position == POS_STANDING && fch->coord->x == fx && fch->coord->y == fy && fch->map == fmap
         && !IS_NPC(fch) && fch->pcdata->quest && fch->pcdata->quest == ch->pcdata->quest)
         {
            act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
            leave_map(fch, toroom, dir, 1);
         }
      }
      ch->pcdata->quest->traveltime = -1;
      return;
   }
         
   if (sector == SECT_EXIT)
   {
      ENTRANCE_DATA *enter;
      ROOM_INDEX_DATA *toroom = NULL;

      enter = check_entrance(ch, map, x, y);
      
      //military does not need to be in areas...if you so choose to remove, unit checks are done in the
      //wilderness room only, so you will need to fix that
      if (IS_NPC(ch) && xIS_SET(ch->act, ACT_MILITARY))
         return;
      //check the mount of a military mob, don't want it roaming off...
      if (xIS_SET(ch->act, ACT_MOUNTABLE))
      {
         for (fch = first_char; fch; fch = fch->next)
         {
            if (fch->mount == ch)
            {
               if (IS_NPC(fch) && xIS_SET(fch->act, ACT_MILITARY))
                  return;
            }
         }
      }
      if (enter != NULL && !IS_PLR_FLAG(ch, PLR_MAPEDIT))
      {
         if (enter->tomap != -1) /* Means exit goes to another map */
         {
            enter_map(ch, enter->there->x, enter->there->y, enter->tomap);
            if (ch->mount)
               enter_map(ch->mount, enter->there->x, enter->there->y, enter->tomap);
            if (ch->rider)
               enter_map(ch->rider, enter->there->x, enter->there->y, enter->tomap);

            for (fch = from_room->first_person; fch; fch = fch->next_in_room)
               chars++;

            for (fch = from_room->first_person; fch && (count < chars); fch = nextinroom)
            {
               nextinroom = fch->next_in_room;
               count++;
               if (fch != ch /* loop room bug fix here by Thoric */
                  && fch->master == ch && fch->position == POS_STANDING && fch->coord->x == fx && fch->coord->y == fy && fch->map == fmap)
               {
                  act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
                  enter_map(fch, enter->there->x, enter->there->y, enter->tomap);
               }
            }
            return;
         }

         if ((toroom = get_room_index(enter->vnum)) == NULL)
         {
            bug("Target vnum %d for map exit does not exist!", enter->vnum);
            send_to_char("Ooops. Something bad happened. Contact the immortals ASAP.\n\r", ch);
            return;
         }

         leave_map(ch, toroom, dir, 0);
         if (ch->mount)
            leave_map(ch->mount, toroom, dir, 0);
         if (ch->rider)
            leave_map(ch->rider, toroom, dir, 0);

         for (fch = from_room->first_person; fch; fch = fch->next_in_room)
            chars++;

         for (fch = from_room->first_person; fch && (count < chars); fch = nextinroom)
         {
            nextinroom = fch->next_in_room;
            count++;
            if (fch != ch /* loop room bug fix here by Thoric */
               && fch->master == ch && fch->position == POS_STANDING && fch->coord->x == fx && fch->coord->y == fy && fch->map == fmap)
            {
               act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
               leave_map(fch, toroom, dir, 0);
            }
         }
         return;
      }

      if (enter != NULL && IS_PLR_FLAG(ch, PLR_MAPEDIT))
      {
         delete_entrance(enter);
         map_sector[ch->map][x][y] = ch->pcdata->secedit;
         send_to_char("&RMap exit deleted.\n\r", ch);
      }

   }
   if (x == global_x[0] && y == global_y[0])
   {
      act(AT_ACTION, "You hit a transporter and are whisped away.", ch, NULL, ch->mount, TO_CHAR);
      ch->coord->x = 205;
      ch->coord->y = 460;
      if (ch->mount)
      {
         ch->mount->coord->x = 205;
         ch->mount->coord->y = 460;
      }
      if (ch->rider)
      {
         ch->rider->coord->x = 205;
         ch->rider->coord->y = 460;
      }
      do_look(ch, "auto");
      if (ch->rider) 
         do_look(ch->rider, "auto");
      for (fch = from_room->first_person; fch; fch = nextinroom)
      {
         nextinroom = fch->next_in_room;
         if (fch != ch /* loop room bug fix here by Thoric */
            && fch->master == ch
            && (fch->position == POS_STANDING || fch->position == POS_MOUNTED) && fch->coord->x == fx && fch->coord->y == fy && fch->map == map)
         {
            act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
            if (IS_NPC(fch))
            {
               fch->coord->x = 205;
               fch->coord->y = 460;
               update_objects(fch, x, y, map);
            }
            else
            {
               process_exit(fch, map, x, y, dir, 1);
            }
         }
      }
      return;
   }

   if (!sect_show[sector].canpass && !IS_IMMORTAL(ch))
   {
      ch_printf(ch, "%s\n\r", impass_message[sector]);
      return;
   }

   maxx = MAX_X;
   maxy = MAX_Y;
   switch (dir)
   {
      case DIR_NORTH:
         if (y == 0)
         {
            send_to_char("You cannot go any further north!\n\r", ch);
            return;
         }
         break;

      case DIR_EAST:
         if (x == maxx + 1)
         {
            send_to_char("You cannot go any further east!\n\r", ch);
            return;
         }
         break;

      case DIR_SOUTH:
         if (y == maxy + 1)
         {
            send_to_char("You cannot go any further south!\n\r", ch);
            return;
         }
         break;

      case DIR_WEST:
         if (x == 0)
         {
            send_to_char("You cannot go any further west!\n\r", ch);
            return;
         }
         break;

      case DIR_NORTHEAST:
         if (x == maxx + 1 || y == 0)
         {
            send_to_char("You cannot go any further northeast!\n\r", ch);
            return;
         }
         break;

      case DIR_NORTHWEST:
         if (x == 0 || y == 0)
         {
            send_to_char("You cannot go any further northwest!\n\r", ch);
            return;
         }
         break;

      case DIR_SOUTHEAST:
         if (x == maxx + 1 || y == maxy + 1)
         {
            send_to_char("You cannot go any further southeast!\n\r", ch);
            return;
         }
         break;

      case DIR_SOUTHWEST:
         if (x == 0 || y == maxy + 1)
         {
            send_to_char("You cannot go any further southwest!\n\r", ch);
            return;
         }
         break;
   }
   if (IS_MOUNTAIN(sect_show[sector].sector))
   {
      if (ch->mount)
      {
         send_to_char("You cannot hope to climb a mountain while mounted!\n\r", ch);
         return;
      }
   }
   if (IS_MOUNTAIN(sect_show[sector].sector) && get_trust(ch) < LEVEL_IMMORTAL)
   {
      if (MASTERED(ch, gsn_mountain_climb) < 1)
      {
         if ((climb = get_objtype(ch, ITEM_MCLIMB)) == NULL)
         {
            send_to_char("A huge, unclimbable mountain blocks your path in that direction.\n\r", ch);
            return;
         }
      }
   }

   if (sect_show[sector].sector == SECT_AIR)
   {
      if (ch->mount && !IS_AFFECTED(ch->mount, AFF_FLYING))
      {
         send_to_char("Your mount can't fly.\n\r", ch);
         return;
      }
      if (!ch->mount && !IS_AFFECTED(ch, AFF_FLYING))
      {
         send_to_char("You'd need to fly to go there.\n\r", ch);
         return;
      }
   }

   if (IS_NOSWIM(sect_show[sector].sector))
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
               return;
            }
            else
            {
               level = POINT_LEVEL(GET_POINTS(ch, gsn_swimming, 0, 1), GET_MASTERY(ch, gsn_swimming, 0, 1));
               if (number_range(1, 100) > 40+level)
               {
                  send_to_char("You struggle to move forward.\n\r", ch);
                  WAIT_STATE(ch, number_range(3, 6));
                  learn_from_failure(ch, gsn_swimming, NULL);
                  return;
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
            return;
         }
      }
   }
   if (IS_NPC(ch) && (xIS_SET(ch->act, ACT_MILITARY) || xIS_SET(ch->act, ACT_KINGDOMMOB) || xIS_SET(ch->act, ACT_MOUNTSAVE)))
      ht = ch->m4;
   if (!IS_NPC(ch))
      ht = ch->pcdata->hometown;
   for (gmob = ch->in_room->first_person; gmob; gmob = gmob->next_in_room)
   {
      if (!IS_NPC(ch) && ((xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOPASS) && in_same_room(ch, gmob) && gmob->m4 != ht)
      || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOCLOAK) && get_wear_cloak(ch) && in_same_room(ch, gmob) && gmob->m4 != ht)
      || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOHOOD) && get_wear_hidden_cloak(ch) && in_same_room(ch, gmob) && gmob->m4 != ht)
      || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NEEDINTRO) && !str_cmp(ch->name, PERS_KINGDOM(ch, gmob->m4)) && in_same_room(ch, gmob) && gmob->m4 != ht)))
      {
         if (!IS_NPC(ch) && dir != rev_dir[ch->pcdata->mapdir])
         {
            send_to_char("\n\r", ch);
            sprintf(buf, "%s I gave you a chance, time to die now.", ch->name);
            do_tell(gmob, buf);
            one_hit(gmob, ch, TYPE_UNDEFINED, LM_BODY);
            return;
         }
      }
   }
   if (ch->mount)
   {
      switch (ch->mount->position)
      {
         case POS_DEAD:
            send_to_char("Your mount is dead!\n\r", ch);
            return;
            break;

         case POS_MORTAL:
         case POS_INCAP:
            send_to_char("Your mount is hurt far too badly to move.\n\r", ch);
            return;
            break;

         case POS_STUNNED:
            send_to_char("Your mount is too stunned to do that.\n\r", ch);
            return;
            break;

         case POS_SLEEPING:
            send_to_char("Your mount is sleeping.\n\r", ch);
            return;
            break;

         case POS_RESTING:
            send_to_char("Your mount is resting.\n\r", ch);
            return;
            break;

         case POS_SITTING:
            send_to_char("Your mount is sitting down.\n\r", ch);
            return;
            break;

         default:
            break;
      }
      if (!IS_FLOATING(ch->mount))
         move = sect_show[(int)map_sector[ch->map][get_x(ch->coord->x, dir)][get_y(ch->coord->y, dir)]].move;
      else
         move = 5;
         
      move = calculate_movement_cost(move, ch->mount); //Takes the normal move and calculates cost based on the stats
      
      if (ch->mount->move < move)
      {
         send_to_char("Your mount is too exhausted.\n\r", ch);
         return;
      }
   }
   else
   {
      if (!IS_FLOATING(ch) || sect_show[sector].sector == SECT_MOUNTAIN)
         move = encumbrance(ch, sect_show[(int)map_sector[ch->map][get_x(ch->coord->x, dir)][get_y(ch->coord->y, dir)]].move);
      else
         move = 5;
         
      if (IS_MOUNTAIN(sect_show[sector].sector))
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
         return;
      }
   }
   nosnow = 0;
   if (NO_SNOW(map_sector[ch->map][get_x(ch->coord->x, dir)][get_y(ch->coord->y, dir)]))
      nosnow = 1;

   if (weather_sector[ch->map][ch->coord->x][ch->coord->y] >= 10 && !nosnow)
   {
      if (ch->mount)
      {
         if (!IS_FLOATING(ch->mount))
            move *= 2.5;
         if (ch->mount->move < move)
         {
            send_to_char("Your mount is too exhausted.\n\r", ch);
            return;
         }

      }
      else
      {
         if (!IS_FLOATING(ch))
            move *= 3;
         if (ch->move < move)
         {
            send_to_char("You are too exhausted.\n\r", ch);
            return;
         }
      }
   }
   if (ch->mount)
      WAIT_STATE(ch, movement_lag(ch->mount, move/5));
   else
      WAIT_STATE(ch, movement_lag(ch, move/5));
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
      

   if (ch->mount)
   {
      if (IS_AFFECTED(ch->mount, AFF_FLOATING))
         txt = "floats";
      else if (IS_AFFECTED(ch->mount, AFF_FLYING))
         txt = "flies";
      else
         txt = "rides";
   }
   else if (IS_AFFECTED(ch, AFF_FLOATING))
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
   if (!IS_AFFECTED(ch, AFF_SNEAK) && (IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS)))
   {
      if (ch->mount)
      {
         sprintf(buf, "$n %s %s upon $N.", txt, dir_name[dir]);
         act(AT_ACTION, buf, ch, NULL, ch->mount, TO_NOTVICT);
      }
      else
      {
         if (!IS_NPC(ch) && ch->rider)
         {
            sprintf(buf, "$n %s %s with $N on $s back.", txt, dir_name[dir]);
            act(AT_ACTION, buf, ch, NULL, ch->rider, TO_ROOM);
         }
         else
         {
            sprintf(buf, "$n %s $T.", txt);
            act(AT_ACTION, buf, ch, NULL, dir_name[dir], TO_ROOM);
         }
      }
   }


   if (!IS_NPC(ch))
      ch->pcdata->mapdir = dir;
   ch->coord->x = x;
   ch->coord->y = y;
   update_objects(ch, x, y, map);
   /* Take away Hide */
   if (IS_AFFECTED(ch, AFF_HIDE))
   {
      if (number_range(1, 100) > check_hide_move(ch, map_sector[ch->map][x][y], ch->in_room))
      {
         xREMOVE_BIT(ch->affected_by, AFF_HIDE);
         act(AT_RED, "$n quickly appears from the shadows.", ch, NULL, NULL, TO_ROOM);
      }
   }
   if (IS_AFFECTED(ch, AFF_STALK))
   {
      if (!check_stalk_move(ch, ch->in_room))
      {
         affect_strip(ch, gsn_stalk);
         act(AT_RED, "$n attemps to stalk silently into the room but fails!.", ch, NULL, NULL, TO_ROOM);
      }
   }
   // WAIT_STATE( ch, move );

   if (ch->mount)
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
   dtxt = rev_exit(dir);
   if (!IS_AFFECTED(ch, AFF_SNEAK) && (IS_NPC(ch) || !xIS_SET(ch->act, PLR_WIZINVIS)))
   {
      if ( ch->mount )
      {
         sprintf( buf, "$n %s from %s upon $N.", txt, dtxt );
         act( AT_ACTION, buf, ch, NULL, ch->mount, TO_ROOM );
      }
      else
      {
         if (!IS_NPC(ch) && ch->rider)
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
      ch->rider->coord->x = x;
      ch->rider->coord->y = y;
      update_objects(ch->rider, x, y, map);
   }
   do_look(ch, "auto");
   if (ch->rider)
      do_look(ch->rider, "auto");
   for (fch = from_room->first_person; fch; fch = nextinroom)
   {
      nextinroom = fch->next_in_room;
      if (fch != ch /* loop room bug fix here by Thoric */
         && fch->master == ch
         && (fch->position == POS_STANDING || fch->position == POS_MOUNTED) && fch->coord->x == fx && fch->coord->y == fy && fch->map == map)
      {
         act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
         if (IS_NPC(fch))
         {
            fch->coord->x = x;
            fch->coord->y = y;
            update_objects(fch, x, y, map);
         }
         else
         {
            process_exit(fch, map, x, y, dir, 1);
         }
      }
   }
   mprog_entry_trigger(ch);
   if (char_died(ch))
      return;

   rprog_enter_trigger(ch);
   if (char_died(ch))
      return;

   mprog_greet_trigger(ch);
   if (char_died(ch))
      return;

   oprog_greet_trigger(ch);
   if (char_died(ch))
      return;
      
   for (wblock = first_wblock; wblock; wblock = wblock->next)
   {
      if (ch->coord->x <= wblock->endx && ch->coord->x >= wblock->stx 
      && ch->coord->y <= wblock->endy && ch->coord->y >= wblock->sty && ch->map == wblock->map && !IS_NPC(ch))
      {
         WINFO_DATA *winfo;
         if (!wblock->first_player)
         {
            CREATE(winfo, WINFO_DATA, 1);
            winfo->pid = ch->pcdata->pid;
            winfo->time = time(0);
            LINK(winfo, wblock->first_player, wblock->last_player, next, prev);
         }
         else
         {
            for (winfo = wblock->first_player; winfo; winfo = winfo->next)
            {
               if (winfo->pid == ch->pcdata->pid)
               {
                  if (time(0) - winfo->time > 7200) //2 hours real time
                     winfo->time = time(0);
                  break;
               }
            }
            if (!winfo)
            {
               CREATE(winfo, WINFO_DATA, 1);
               winfo->pid = ch->pcdata->pid;
               winfo->time = time(0);
               LINK(winfo, wblock->first_player, wblock->last_player, next, prev);
            }
         }
      }
   }
   if (first == 0)
      map_battle(ch, x, y, map);

   for (gmob = ch->in_room->first_person; gmob; gmob = gmob->next_in_room)
   {
      if (!IS_NPC(ch) && !IS_NPC(gmob))
      {
         INTRO_DATA *intro;
         OBJ_DATA *light;
         int mod;
         
         if (gethour() > 21 || gethour() < 6)
         {
            mod = 2;
            light = get_eq_char(ch, WEAR_LIGHT);
            if (light && light->item_type == ITEM_LIGHT)
               mod = 4;
         }
         else
         {
            mod = 5;
         }
         for (intro = ch->pcdata->first_introduction; intro; intro = intro->next)
         {
            if (intro->pid == gmob->pcdata->pid)
            {
               if (time(0) - intro->lastseen > 300) //5 minutes  
               {
                  if (abs(ch->coord->x - gmob->coord->x) > mod || abs(ch->coord->y - gmob->coord->y) > mod)
                     continue; 
                  if (intro->value > 0)
                     intro->value += number_range(100, 225);
                  else
                     intro->value -= number_range(100, 225);
                  intro->lastseen = 0;
                  break;
               }
            }
         }
         if (intro)
         {
            if (gethour() > 21 || gethour() < 6)
            {
               mod = 2;
               light = get_eq_char(gmob, WEAR_LIGHT);
               if (light && light->item_type == ITEM_LIGHT)
                  mod = 4;
            }
            else
            {
               mod = 5;
            }
            for (intro = gmob->pcdata->first_introduction; intro; intro = intro->next)
            {
               if (intro->pid == ch->pcdata->pid)
               {
                  if (time(0) - intro->lastseen > 300) //5 minutes  
                  {
                     if (abs(ch->coord->x - gmob->coord->x) > mod || abs(ch->coord->y - gmob->coord->y) > mod)
                        continue; 
                     if (intro->value > 0)
                        intro->value += number_range(100, 225);
                     else
                        intro->value -= number_range(100, 225);
                     intro->lastseen = 0;
                     break;
                  }
               }
            } 
         }
      }
      if (!IS_NPC(ch) && ((xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOPASS) && in_same_room(ch, gmob) && gmob->m4 != ht)
      || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOCLOAK) && get_wear_cloak(ch) && in_same_room(ch, gmob) && gmob->m4 != ht)
      || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NOHOOD) && get_wear_hidden_cloak(ch) && in_same_room(ch, gmob) && gmob->m4 != ht)
      || (xIS_SET(gmob->act, ACT_MILITARY) && xIS_SET(gmob->miflags, KM_NEEDINTRO) && !str_cmp(ch->name, PERS_KINGDOM(ch, gmob->m4)) && in_same_room(ch, gmob) && gmob->m4 != ht)))
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
            sprintf(buf, "%s, I shall not allow you to ENTER!", ch->name);
            do_say(gmob, buf);
            one_hit(gmob, ch, TYPE_UNDEFINED, LM_BODY);
            return;
         }
      }
   }

   return;
}

void map_north(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x, ch->coord->y - 1, DIR_NORTH, 0);
   return;
}

void map_east(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x + 1, ch->coord->y, DIR_EAST, 0);
   return;
}

void map_south(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x, ch->coord->y + 1, DIR_SOUTH, 0);
   return;
}

void map_west(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x - 1, ch->coord->y, DIR_WEST, 0);
   return;
}

void map_up(CHAR_DATA * ch)
{
   send_to_char("Huh?\n\r", ch);
   return;
}

void map_down(CHAR_DATA * ch)
{
   send_to_char("Huh?\n\r", ch);
   return;
}

void map_ne(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x + 1, ch->coord->y - 1, DIR_NORTHEAST, 0);
   return;
}

void map_nw(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x - 1, ch->coord->y - 1, DIR_NORTHWEST, 0);
   return;
}

void map_se(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x + 1, ch->coord->y + 1, DIR_SOUTHEAST, 0);
   return;
}

void map_sw(CHAR_DATA * ch)
{
   process_exit(ch, ch->map, ch->coord->x - 1, ch->coord->y + 1, DIR_SOUTHWEST, 0);
   return;
}

void process_movement_value(CHAR_DATA *ch, int dir)
{
   if (dir == 0)
       map_north(ch);
   if (dir == 1)
       map_east(ch);
   if (dir == 2)
       map_south(ch);
   if (dir == 3)
       map_west(ch);
   if (dir == 6)
       map_ne(ch);
   if (dir == 7)
       map_nw(ch);
   if (dir == 8)
       map_se(ch);
   if (dir == 9)
       map_sw(ch);
}

ROOM_INDEX_DATA *find_continent(CHAR_DATA * ch, int maproom)
{
   ROOM_INDEX_DATA *location = NULL;

   if (maproom == OVERLAND_SOLAN)
      location = get_room_index(OVERLAND_SOLAN);

   return location;
}

void enter_map(CHAR_DATA * ch, int x, int y, int continent)
{
   ROOM_INDEX_DATA *maproom = NULL;

   if (continent > MAP_MAX) /* Sending the vnum of the room to goto, make sure the target room is not in limbo.are */
      maproom = find_continent(ch, continent);

   else /* Means you are likely an immortal using the goto command */
   {
      switch (continent)
      {
         case MAP_SOLAN:
            maproom = get_room_index(OVERLAND_SOLAN);
            ch->map = MAP_SOLAN;
            break;
         default:
            bug("Invalid target map specified: %d", continent);
            return;
      }
   }

   if (!maproom)
   {
      bug("enter_map: Overland map room is missing!", 0);
      send_to_char("Woops. Something is majorly wrong here - inform the immortals.\n\r", ch);
      return;
   }

   if (maproom->vnum == OVERLAND_SOLAN)
      ch->map = MAP_SOLAN;

   if (!IS_NPC(ch))
      if (ch->pcdata->mapdir == -1)
         ch->pcdata->mapdir = 1;

   update_players_map(ch, x, y, ch->map, 0, maproom);
   update_objects(ch, x, y, ch->map);
   return;
}

void leave_map(CHAR_DATA * ch, ROOM_INDEX_DATA * target, int dir, int qexit)
{
   if (!IS_NPC(ch))
      REMOVE_PLR_FLAG(ch, PLR_MAPEDIT); /* Just in case they were editing */

   if (!IS_NPC(ch))
      ch->pcdata->mapdir = dir;


   if (!qexit)
      update_players_map(ch, -1, -1, -1, 3, target);
   else
   {
      char_from_room(ch);
      char_to_room(ch, target);
      ch->coord->x = -1;
      ch->coord->y = -1;
      ch->map = -1;
      REMOVE_ONMAP_FLAG(ch);
      do_look(ch, "");
   }
   update_objects(ch, -1, -1, -1);
   return;
}

/*
void do_joinfight(CHAR_DATA * ch, char *argument)
{
   DESCRIPTOR_DATA *d;
   CMAP_DATA *mch;
   CHAR_DATA *fch;
   int found = 0;
   int used = -1;

   if (ch->fcoord->x > -1 || ch->fcoord->y > -1 || ch->fightm > -1)
   {
      send_to_char("You cannot join a fight while you are in one.\n\r", ch);
      return;
   }

   for (d = first_descriptor; d; d = d->next)
   {
      if (d->connected == CON_PLAYING
         && d->character != ch
         && d->character->in_room
         && d->newstate != 2
         && can_see_map(ch, d->character)
         && (d->character->coord->x == ch->coord->x && d->character->coord->y == ch->coord->y && d->character->map == ch->map))
      {
         if (d->character->fcoord->x > -1 && d->character->fcoord->y > -1 && d->character->fightm > -1)
         {
            found = 1;
            used = d->character->fightm;
         }
      }
   }
   for (mch = first_wilderchar; mch; mch = mch->next)
   {
      if (mch->mapch->coord->x == ch->coord->x && mch->mapch->coord->y == ch->coord->y && mch->mapch->map == ch->map)
         if (mch->mapch->fcoord->x > -1 && mch->mapch->fcoord->y > -1 && mch->mapch->fightm > -1)
         {
            found = 1;
            used = mch->mapch->fightm;
         }
   }
   if (found == 0 || used == -1)
   {
      if (found == 0 && used != -1)
         bug("do_joinfight:  used has a value while found doesn't.", 0);
      if (found != 0 && used == -1)
         bug("do_joinfight:  found has a value while used doesn't.", 0);

      send_to_char("There is no fight here to join.\n\r", ch);
      return;
   }
   else
   {
      ch->fightm = used;
      ch->fcoord->x = number_range(8, 14);
      ch->fcoord->y = number_range(8, 14);
      ch->fcounter = 0;

      update_followers(ch, ch->fcoord->x, ch->fcoord->y, ch->fightm, ch->coord->x, ch->coord->y, ch->map);
      do_look(ch, "auto");

      act(AT_CYAN, "\n\r[$n] has joined the chaos.", ch, NULL, NULL, TO_FIGHT);

      for (fch = ch->in_room->first_person; fch; fch = fch->next_in_room)
      {
         if (fch != ch
            && fch->master == ch
            && (ch->in_room && fch->in_room)
            && !IS_NPC(fch)
            && fch->coord->x == ch->coord->x
            && fch->coord->y == ch->coord->y
            && fch->map == ch->map && (ch->in_room == fch->in_room) && (fch->position == POS_STANDING || fch->position == POS_MOUNTED))
         {
            act(AT_CYAN, "[$n] has joined the chaos.", fch, NULL, NULL, TO_FIGHT);
         }
      }
   }
   return;
}
*/
void do_lookaround(CHAR_DATA * ch, char *argument)
{
   if (IS_IMMORTAL(ch))
   {
      send_to_char("This command is for mortals only (he he you cannot use it).\n\r", ch);
      return;
   }
   if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
   {
      send_to_char("You can only use this command while out in the Wilderness.\n\r", ch);
      return;
   }
   if (ch->position <= POS_SLEEPING)
   {
      send_to_char("You cannot do that in your sleep.\n\r", ch);
      return;
   }

   display_map(ch, 5000, 5000, 0);
   return;
}

void do_coords(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char buf[MSL];
   int x, y;

   if (IS_NPC(ch))
   {
      send_to_char("NPCs cannot use this command.\n\r", ch);
      return;
   }

   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("This command can only be used from the overland maps.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   if (arg[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Usage: coords <x> <y>\n\r", ch);
      send_to_char("Usage: coords view <x> <y>\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "view"))
   {
      char arg2[MIL];

      argument = one_argument(argument, arg2);

      x = atoi(arg2);
      y = atoi(argument);

      if ((x > 50 || y > 50) || (x < 1 || y < 1))
      {
         send_to_char("Valid range is 1 to 50 for both x and y.\n\r", ch);
         return;
      }

      display_map(ch, x, y, 0);
      return;
   }


   x = atoi(arg);
   y = atoi(argument);

   if (x < 1 || x > MAX_X)
   {
      sprintf(buf, "Valid x coordinates are 1 to %d.\n\r", MAX_X);
      send_to_char(buf, ch);
      return;
   }

   if (y < 1 || y > MAX_Y)
   {
      sprintf(buf, "Valid y coordinates are 1 to %d.\n\r", MAX_Y);
      send_to_char(buf, ch);
      return;
   }

   ch->coord->x = x;
   ch->coord->y = y;

   update_objects(ch, x, y, ch->map);
   do_look(ch, "auto");

   return;
}
void load_solan()
{
   FILE *fp;
   char filename[256];
   int stx, sty, endx, endy, sector;
   int x, y;

   sprintf(filename, "%ssolan.map", MAP_DIR);

   if ((fp = fopen(filename, "r")) == NULL)
   {
      bug("load_solan: File for map not found!", 0);
      shutdown_mud("Missing solan.map file");
      exit(1);
   }
   for (;;)
   {
      char letter;
      char *word;
      
      if (feof(fp))
         break;

      letter = fread_letter(fp);
      if (letter != '#')
      {
         bug("load_solan: # not found, possibly not a map file.", 0);
         shutdown_mud("Bad file data in solan.map");
         exit(1);
      }

      word = fread_word(fp);

      if (!str_cmp(word, "MAP"))
      {
         for (;;)
         {
            if (feof(fp))
            {
               bug("load_solan: Reached EOL.");
               break;
            }
            stx = fread_number(fp);
            sty = fread_number(fp);
            endx = fread_number(fp);
            endy = fread_number(fp);
            sector = fread_number(fp);
            for (x = stx; x <= endx; x++)
            {
               if (x == stx)
                  y = sty;
               else
                  y = 1;
                  
               for (;; y++)
               {
                 if ((x == endx && y > endy) || y == MAX_Y+1)
                    break;
                 map_sector[0][x][y] = sector;
               }
            } 
            if (endx == MAX_X && endy == MAX_Y)
               break;
         }
      }

      else if (!str_cmp(word, "END"))
         break;
      else
      {
         char buf[MSL];

         sprintf(buf, "load_solan: bad section: %s.", word);
         bug(buf, 0);
         continue;
      }

   }
   FCLOSE(fp);
   for (x = 1; x <= MAX_X; x++)
   {
      for (y = 1; y <= MAX_Y; y++)
      {
         if (map_sector[0][x][y] == SECT_SHIP)
            map_sector[0][x][y] = SECT_OCEAN;
      }
   }
   return;
}

//Average weather for the center of the map, adjust as needed
const int avg_temp[12][4] = {
   {62, 60, 58, 57}, //Jan
   {56, 57, 59, 61}, //Feb
   {63, 65, 68, 70}, //Mar
   {74, 78, 80, 84}, //Apr
   {88, 92, 97, 102}, //May
   {105, 107, 110, 113}, //Jun
   {116, 118, 120, 124}, //Jul
   {126, 130, 123, 118}, //Aug
   {115, 110, 102, 97}, //Sep
   {95, 93, 90, 86}, //Oct
   {81, 77, 74, 71}, //Nov
   {69, 67, 65, 63} //Dec
};

int get_avg_temp()
{
   int day;
   int month;
   int week;

   day = getday();
   month = day / 30;
   day = getdayofmonth(day);
   week = (day / 7.5) + 1;
   if (day == 30)
      week = 4;
   return avg_temp[month][week];
}

int get_curr_dir(int x, int fx, int y, int fy)
{
   if (y < fy) //Front is South of Player
   {
      if (x > fx) //Front is West of Player
      {
         if (y == fy)
            return 4;
         if ((((abs(x - fx) * 100) / abs(y - fy)) > 150) || (((abs(x - fx) * 100) / abs(y - fy)) < 60))
         {
            if (((abs(x - fx) * 100) / abs(y - fy)) >= 500)
               return 4;
            if (((abs(x - fx) * 100) / abs(y - fy)) <= 20)
               return 0;
            if (((abs(x - fx) * 100) / abs(y - fy)) > 150)
               return 3;
            if (((abs(x - fx) * 100) / abs(y - fy)) < 60)
               return 1;
         }
         else
            return 2;
      }
      if (x < fx) //Front is East of Player
      {
         if (y == fy)
            return 12;
         if ((((abs(x - fx) * 100) / abs(y - fy)) > 150) || (((abs(x - fx) * 100) / abs(y - fy)) < 60))
         {
            if (((abs(x - fx) * 100) / abs(y - fy)) >= 500)
               return 12;
            if (((abs(x - fx) * 100) / abs(y - fy)) <= 20)
               return 0;
            if (((abs(x - fx) * 100) / abs(y - fy)) > 150)
               return 13;
            if (((abs(x - fx) * 100) / abs(y - fy)) < 60)
               return 15;
         }
         else
            return 14;
      }
   }
   else //Front is north of Player
   {
      if (x > fx) //Front is West of Player
      {
         if (y == fy)
            return 4;
         if ((((abs(x - fx) * 100) / abs(y - fy)) > 150) || (((abs(x - fx) * 100) / abs(y - fy)) < 60))
         {
            if (((abs(x - fx) * 100) / abs(y - fy)) >= 500)
               return 4;
            if (((abs(x - fx) * 100) / abs(y - fy)) <= 20)
               return 8;
            if (((abs(x - fx) * 100) / abs(y - fy)) > 150)
               return 5;
            if (((abs(x - fx) * 100) / abs(y - fy)) < 60)
               return 7;
         }
         else
            return 6;
      }
      if (x < fx) //Front is East of Player
      {
         if (y == fy)
            return 12;
         if ((((abs(x - fx) * 100) / abs(y - fy)) > 150) || (((abs(x - fx) * 100) / abs(y - fy)) < 60))
         {
            if (((abs(x - fx) * 100) / abs(y - fy)) >= 500)
               return 12;
            if (((abs(x - fx) * 100) / abs(y - fy)) <= 20)
               return 8;
            if (((abs(x - fx) * 100) / abs(y - fy)) > 150)
               return 11;
            if (((abs(x - fx) * 100) / abs(y - fy)) < 60)
               return 9;
         }
         else
            return 10;
      }
   }
   return 8;
}

float getsl(float lx, float ly, float hx, float hy)
{
   float diff1;
   float diff2;

   if (ly > hy)
      diff1 = ly - hy;
   else
      diff1 = hy - ly;

   if (lx > hx)
      diff2 = lx - hx;
   else
      diff2 = hx - lx;

   if (diff1 <= 0 || diff2 <= 0)
      return 0;

   return (float) diff1 / diff2;
}

float find_fsqrt(float dist)
{
   float x = 1.0;

   for (;;)
   {
      if ((x * x) >= dist)
         return x;
      x++;
   }
   return 1.0;
}

void check_torn_damage(int x, int y, int map)
{
   int stx;
   int sty;
   int sector;

   for (stx = x - 5; stx <= x + 5; stx++)
   {
      for (sty = y - 5; sty <= y + 5; sty++)
      {
         if (stx < 1 || stx > MAX_X || sty < 1 || sty > MAX_Y)
            continue;
         if ((((x - stx) * (x - stx)) + ((y - sty) * (y - sty))) <= 25)
         {
            sector = map_sector[map][stx][sty];
            if (sector == SECT_FOREST || sector == SECT_ROAD || sector == SECT_HCORN ||
               sector == SECT_HGRAIN || sector == SECT_STREE || sector == SECT_NTREE ||
               sector == SECT_SCORN || sector == SECT_NCORN || sector == SECT_SGRAIN ||
               sector == SECT_NGRAIN || sector == SECT_JUNGLE || sector == SECT_PATH || sector == SECT_PAVE || sector == SECT_BRIDGE)
            {
               if (sector != SECT_BRIDGE)
                  sector = SECT_FIELD;
               else
                  sector = SECT_WATER_NOSWIM;
            }
            resource_sector[map][stx][sty] -= 150;
            if (resource_sector[map][stx][sty] <= 0)
            {
               map_sector[map][stx][sty] = sector;
               resource_sector[map][stx][sty] = 0;
            }
            
         }
      }
   }
}

const int front_offsetx[8][30] = {
   {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15},
   {0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7},
   {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8},
   {1, 0, 2, 1, 2, 2, 3, 2, 4, 3, 4, 4, 5, 4, 6, 5, 6, 6, 7, 6, 8, 7, 8, 8, 9, 8, 10, 9, 10, 10},
   {1, 0, 2, 1, 2, 2, 2, 2, 3, 2, 4, 3, 4, 4, 4, 4, 5, 4, 6, 5, 6, 6, 6, 6, 7, 6, 8, 7, 8, 8},
   {0, 1, 0, 2, 1, 2, 2, 2, 2, 3, 2, 4, 3, 4, 4, 4, 4, 5, 4, 6, 5, 6, 6, 6, 6, 7, 6, 8, 7, 8},
   {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15},
   {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5}
};

const int front_offsety[8][30] = {
   {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15},
   {1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 8, 8},
   {0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7},
   {0, 0, 1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7},
   {0, 1, 0, 2, 1, 2, 2, 2, 2, 3, 2, 4, 3, 4, 4, 4, 4, 5, 4, 6, 5, 6, 6, 6, 6, 7, 6, 8, 7, 8},
   {1, 0, 2, 1, 2, 2, 2, 2, 3, 2, 4, 3, 4, 4, 4, 4, 5, 4, 6, 5, 6, 6, 6, 6, 7, 6, 8, 7, 8, 8},
   {1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5},
   {1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14, 15, 15}
};

//Math at its finest (well my finest, rofl)
void update_local_weather(FRONT_DATA * fnt, int type)
{
   float x; //Start x
   float y; //Start y
   float cx; //Right move
   float cy; //Right move
   float ox; //Left move
   float oy; //Left move
   float map; //Map
   float lx; //Lowest X
   float ly; //Lowest y
   float hx; //Highest x
   float hy; //Highest y
   float stx;
   float sty;
   float bx;
   float by;
   float endx;
   float endy;
   float add;
   float snow;
   float temp;
   float sl;
   float r;
   float dist;
   float dist2;

   x = (float) fnt->x;
   y = (float) fnt->y;
   cx = x;
   cy = y;
   ox = cx;
   oy = cy;
   lx = hx = ox;
   ly = hy = oy;
   map = (float) fnt->map;

   if (fnt->size % 2 == 0) //Even
   {
      ox -= (float) front_offsetx[fnt->typec][fnt->size - 1];
      oy += (float) front_offsety[fnt->typec][fnt->size - 1];
      cx += (float) front_offsetx[fnt->typec][fnt->size - 2];
      cy -= (float) front_offsety[fnt->typec][fnt->size - 2];
   }
   else
   {
      ox -= (float) front_offsetx[fnt->typec][fnt->size - 2];
      oy += (float) front_offsety[fnt->typec][fnt->size - 2];
      cx += (float) front_offsetx[fnt->typec][fnt->size - 1];
      cy -= (float) front_offsety[fnt->typec][fnt->size - 1];
   }

   if (cx > hx)
      hx = cx;
   if (ox > hx)
      hx = ox;
   if (cx < lx)
      lx = cx;
   if (ox < lx)
      lx = ox;
   if (cy > hy)
      hy = cy;
   if (oy > hy)
      hy = oy;
   if (cy < ly)
      ly = cy;
   if (oy < ly)
      ly = oy;
   if (lx < 1)
      lx = 1;
   if (ly < 1)
      ly = 1;
   if (hx > MAX_X)
      hx = MAX_X;
   if (hy > MAX_Y)
      hy = MAX_Y;

   bx = lx - 10;
   by = ly - 10;
   endx = hx + 10;
   endy = hy + 10;

   if (bx < 1)
      bx = 1;
   if (by < 1)
      by = 1;
   if (endx > MAX_X)
      endx = MAX_X;
   if (endy > MAX_Y)
      endy = MAX_Y;
   r = ((x - endx) * (x - endx)) + ((y - endy) * (y - endy));
   r = find_fsqrt(r);
   sl = getsl(lx, ly, hx, hy);


   //Due to how the circle works, it will form a cone instead of a circle, so
   //don't look for half a circle to show up, but a cone
   for (sty = by; sty <= endy; sty++)
   {
      temp = generate_temperature(NULL, fnt->x, (int) sty, fnt->map);
      for (stx = bx; stx <= endx; stx++)
      {
         //Never thought you would see geometry used, neither did I till now
         if (fnt->type == 0) //Cold
         {
            //                   Slope           Cng X   Y axis
            if (sty <= (sl * (hx - stx) + ly)) //NW of line
            {
               //             x^2              y^2               r^2
               if ((((x - stx) * (x - stx)) + ((y - sty) * (y - sty))) <= (r * r)) //within the circle
               {
                  //Find the shortest distance from the line
                  dist = get_distform(stx, (sl * (hx - stx) + ly), stx, sty);
                  dist2 = get_distform((hx - (sty / sl) + (ly / sl)), sty, stx, sty);
                  if (dist2 < dist)
                     dist = dist2;
                  snow = weather_sector[(int) map][(int) stx][(int) sty] / 10;
                  if (snow < 0)
                     snow = 0;
                  add = (int) (UMAX(1, 6 - (dist / 4)));

                  if (weather_sector[(int) map][(int) stx][(int) sty] + add >= (snow + 1) * 10)
                     weather_sector[(int) map][(int) stx][(int) sty] = (int) snow *10 + 9;

                  else
                     weather_sector[(int) map][(int) stx][(int) sty] += (int) add;

                  if (temp >= 20 && temp <= 35)
                  {
                     if (snow == 0 && add > 0)
                        weather_sector[(int) map][(int) stx][(int) sty] += 20;
                     if (add >= 1 && add <= 2)
                        if (number_range(1, 3) == 1)
                           weather_sector[(int) map][(int) stx][(int) sty] += 10;
                     if (add >= 3 && add <= 4)
                        if (number_range(1, 2) == 1)
                           weather_sector[(int) map][(int) stx][(int) sty] += 10;
                     if (add > 4)
                        weather_sector[(int) map][(int) stx][(int) sty] += 10;
                  }

                  if (weather_sector[(int) map][(int) stx][(int) sty] < 0)
                     weather_sector[(int) map][(int) stx][(int) sty] = (int) add;
               }
            }

         }
         if (fnt->type == 1) //Warm
         {
            //                   Slope           Cng X   Y axis
            if (sty >= (hy - (sl * (hx - stx)))) //NW of line
            {
               //             x^2              y^2               r^2
               if ((((x - stx) * (x - stx)) + ((y - sty) * (y - sty))) <= (r * r)) //within the circle
               {
                  //Find the shortest distance from the line
                  dist = get_distform(stx, (hy - (sl * (hx - stx))), stx, sty);
                  dist2 = get_distform((sty / sl + hx - (hy / sl)), sty, stx, sty);
                  if (dist2 < dist)
                     dist = dist2;
                  snow = weather_sector[(int) map][(int) stx][(int) sty] / 10;
                  if (snow < 0)
                     snow = 0;
                  add = (int) (UMAX(1, 6 - (dist / 4)));
                  if (weather_sector[(int) map][(int) stx][(int) sty] + add >= (snow + 1) * 10)
                     weather_sector[(int) map][(int) stx][(int) sty] = (int) snow *10 + 9;

                  else
                     weather_sector[(int) map][(int) stx][(int) sty] += (int) add;

                  if (temp >= 20 && temp <= 35)
                  {
                     if (snow == 0 && add > 0)
                        weather_sector[(int) map][(int) stx][(int) sty] += 20;
                     if (add >= 1 && add <= 2)
                        if (number_range(1, 3) == 1)
                           weather_sector[(int) map][(int) stx][(int) sty] += 10;
                     if (add >= 3 && add <= 4)
                        if (number_range(1, 2) == 1)
                           weather_sector[(int) map][(int) stx][(int) sty] += 10;
                     if (add > 4)
                        weather_sector[(int) map][(int) stx][(int) sty] += 10;
                  }

                  if (weather_sector[(int) map][(int) stx][(int) sty] < 0)
                     weather_sector[(int) map][(int) stx][(int) sty] = (int) add;
               }
            }
         }
      }
   }
}

//Lovely thing for generating wind direction, a similar formula is used for generating bad
//weather
void generate_wind(CHAR_DATA * ch, int x, int y, int map)
{
   FRONT_DATA *front;
   int curdir = -1;
   int curstr = -1;
   int diff;

   if (ch != NULL && x == -1 && y == -1 && map == -1)
   {
      x = ch->coord->x;
      y = ch->coord->y;
      map = ch->map;
   }
   winddir = windstr = -1;

   for (front = first_front; front; front = front->next)
   {
      if (map == front->map && abs(x - front->x) <= 50 && abs(y - front->y) <= 50)
      {
         curdir = get_curr_dir(x, front->x, y, front->y);
         curstr = (100 - (abs(x - front->x) + abs((y - front->y)))) / 2;
         if (windstr != -1 && winddir != curdir)
         {
            if ((abs(winddir - curdir > 8)))
               diff = 8 - (abs(winddir - curdir) - 8);
            else
               diff = abs(winddir - curdir);

            if (diff <= 2)
            {
               if (diff == 1)
               {
                  if (curstr > windstr)
                     winddir = curdir;
               }
               if (diff == 2)
               {
                  if (curstr > windstr)
                     winddir = curdir;
                  if (windstr != 0 && ((curstr * 10) / windstr >= 7))
                  {
                     if ((winddir == 15 && curdir == 1) || (curdir == 15 && winddir == 1))
                        winddir = 0;
                     else if ((winddir == 14 && curdir == 0) || (curdir == 14 && winddir == 0))
                        winddir = 15;
                     else
                        winddir = (curdir + winddir) / 2;
                  }
               }
               windstr += (((curstr * 10) * (5 + (diff * 2))) / 100);
            }
            else
               windstr -= (((curstr * 10) * (6 + ((diff - 1) / 2))) / 100);

            if (windstr < 0)
            {
               windstr = abs(windstr);
               winddir = curdir;
            }
         }
         else if (windstr == -1)
         {
            winddir = curdir;
            windstr = curstr;
         }
         else
            windstr += curstr;
      }
   }
}

char *const owindd[16] = {
   "south", "south/southwest", "southwest", "west/southwest", "west", "west/northwest",
   "northwest", "north/northwest", "north", "north/northeast", "northeast", "east/northeast",
   "east", "east/southeast", "southeast", "south/southeast"
};

char *const windd[16] = {
   "north", "north/northeast", "northeast", "east/northeast", "east", "east/southeast",
   "southeast", "south/southeast", "south", "south/southwest", "southwest", "west/southwest",
   "west", "west/northwest", "northwest", "north/northwest"
};

const int front_cr[8][30] = {
   {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
   {0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1},
   {1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0},
   {2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2, 0, 0, 2, 2},
   {0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0},
   {0, 2, 0, 2, 2, 0, 2, 0, 0, 2, 0, 2, 2, 0, 2, 0, 0, 2, 0, 2, 2, 0, 2, 0, 0, 2, 0, 2, 2, 0},
   {0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0},
   {1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1, 1, 2, 1}
};

void create_front(int type)
{
   FRONT_DATA *fnt;
   int pc;
   int cnt;
   int ttype;
   int x;

   for (x = 0; x < MAP_MAX; x++)
   {
      CREATE(fnt, FRONT_DATA, 1);
      fnt->type = type;
      if (type == 0) //Cold
      {
         fnt->x = number_range(1, 450);
         fnt->y = number_range(1, 450);
      }
      else
      {
         fnt->x = MAX_X - number_range(1, 450);
         fnt->y = number_range(1, 450);
      }
      fnt->map = x;
      pc = number_range(20, 30); //Size of front
      fnt->size = pc;
      ttype = number_range(0, 7); //Creates 8 different kinds, produces slightly different angles.
      fnt->speed = number_range(1, 3);
      for (cnt = 0; cnt < 30; cnt++)
         fnt->f[cnt] = -1;
      for (cnt = 0; cnt < pc; cnt++)
      {
         fnt->f[cnt] = front_cr[ttype][cnt];
      }
      fnt->typec = ttype;
      LINK(fnt, first_front, last_front, next, prev);
   }
}

//Generate temperatures based on front locations
int get_front_temp(CHAR_DATA * ch, int x, int y, int map)
{
   FRONT_DATA *fnt;
   int dist = 0;
   int neg = 1;
   int diff = 0;

   for (fnt = first_front; fnt; fnt = fnt->next)
   {
      if (abs(fnt->x - x) <= 50 && abs(fnt->y - y) <= 50)
      {
         if (fnt->type == 1) //Warm Front
         {
            if (x < fnt->x && y < fnt->y) //Player is W to N of front
               dist += 71 - get_distform(x, y, fnt->x, fnt->y);
         }
         if (fnt->type == 0) //Cold front
         {
            if (x < fnt->x && y > fnt->y) //Player is W to S of front
               dist -= get_distform(x, y, fnt->x, fnt->y) - 71;
         }
      }
   }
   if (dist == 0)
      return 0;
   if (dist * -1 > 0)
      neg = -1;
   if (abs(dist) >= 10 && abs(dist) <= 20)
      diff = abs(dist) / 5.9;
   if (abs(dist) > 20 && abs(dist) <= 30)
      diff = abs(dist) / 5.5;
   if (abs(dist) > 30 && abs(dist) <= 40)
      diff = abs(dist) / 5.1;
   if (abs(dist) > 40 && abs(dist) <= 50)
      diff = abs(dist) / 4.7;
   if (abs(dist) > 50 && abs(dist) <= 60)
      diff = abs(dist) / 4.3;
   if (abs(dist) > 60)
      diff = 18;

   return diff * neg;
}


int generate_temperature(CHAR_DATA * ch, int x, int y, int map)
{
   int temp;
   int windmod = 0;
   int fntmod = 0;
   int hour;

   if (ch != NULL && x == -1 && y == -1 && map == -1)
   {
      x = ch->coord->x;
      y = ch->coord->y;
      map = ch->map;
   }

   temp = get_avg_temp();

   temp -= ((abs(750 - y) / 75) * 10); //Position change, farther from center the colder it is.
   hour = gethour();

   if (hour >= 0 && hour <= 4)
      temp -= 8;
   if (hour >= 5 && hour <= 7)
      temp -= 5;
   if (hour >= 8 && hour <= 10)
      temp -= 3;
   if (hour >= 12 && hour <= 14)
      temp += 5;
   if (hour >= 18 && hour <= 19)
      temp -= 3;
   if (hour >= 20 && hour <= 22)
      temp -= 5;
   if (hour == 23)
      temp -= 8;

   generate_wind(ch, x, y, map);

   if (windstr >= 0 && windstr <= 10)
      windmod = 1;
   if (windstr >= 11 && windstr <= 20)
      windmod = (windstr / 9.5);
   if (windstr >= 21 && windstr <= 40)
      windmod = (windstr / 9.3);
   if (windstr >= 41 && windstr <= 60)
      windmod = (windstr / 9.2);
   if (windstr >= 61 && windstr <= 90)
      windmod = (windstr / 9);
   if (windstr > 90)
      windmod = 12;

   if (temp > 85) //Warm weather is harder to cool down
      windmod /= 4;
   if (temp < 32) //Make it chill the bone more :-)
      windmod *= 1.2;

   temp -= windmod;
   fntmod = get_front_temp(ch, x, y, map);

   if (fntmod < 0 && temp < 32)
      fntmod /= 2;
   if (fntmod > 0 && temp > 80)
      fntmod /= 2;
   temp += fntmod;
   return temp;
}

bool no_snow(int sector)
{
   if (sector == SECT_WATER_SWIM || sector == SECT_WATER_NOSWIM || sector == SECT_UNDERWATER ||
      sector == SECT_DESERT || sector == SECT_OCEANFLOOR || sector == SECT_UNDERGROUND ||
      sector == SECT_MINEGOLD || sector == SECT_MINEIRON || sector == SECT_SGOLD ||
      sector == SECT_NGOLD || sector == SECT_SIRON || sector == SECT_NIRON ||
      sector == SECT_RIVER || sector == SECT_SHORE || sector == SECT_ICE ||
      sector == SECT_OCEAN || sector == SECT_LAVA || sector == SECT_TREE ||
      sector == SECT_NOSTONE || sector == SECT_QUICKSAND || sector == SECT_VOID ||
      sector == SECT_FIRE || sector == SECT_STONE || sector == SECT_SSTONE || sector == SECT_NSTONE || sector == SECT_AIR)
      return TRUE;
   else
      return FALSE;
}

void generate_forecast(CHAR_DATA * ch, int x, int y, int map)
{
   char tbuf[8];
   char dbuf[25];
   FRONT_DATA *fnt;
   int cnt;
   int cx, cy, ox, oy;
   int dir = 0;
   int dist = 0;

   if (!ch)
      return;

   if (ch != NULL && x == -1 && y == -1 && map == -1)
   {
      x = ch->coord->x;
      y = ch->coord->y;
      map = ch->map;
   }

   for (fnt = first_front; fnt; fnt = fnt->next)
   {
      if (abs(fnt->x - x) <= 70 && abs(fnt->y - y) <= 70 && fnt->map == ch->map)
      {
         cx = fnt->x;
         cy = fnt->y;
         ox = cx;
         oy = cy;
         dist = 150;
         dir = 0;
         for (cnt = 2; cnt < 31; cnt++)
         {
            if (fnt->f[cnt - 2] == 0)
            {
               if (cnt % 2)
                  cx++;
               else
                  ox--;
            }
            if (fnt->f[cnt - 2] == 1)
            {
               if (cnt % 2)
               {
                  if (fnt->type == 1) //Warm
                     cy++;
                  else
                     cy--;
               }
               else
               {
                  if (fnt->type == 1) //Warm
                     oy--;
                  else
                     oy++;
               }
            }
            if (fnt->f[cnt - 2] == 2)
            {
               if (cnt % 2)
               {
                  cx++;
                  if (fnt->type == 1)
                     cy++;
                  else
                     cy--;
               }
               else
               {
                  ox--;
                  if (fnt->type == 1) //Warm
                     oy--;
                  else
                     oy++;
               }
            }
            if (cnt % 2)
            {
               if (get_distform(x, y, cx, cy) < dist)
               {
                  dist = get_distform(x, y, cx, cy);
                  dir = get_curr_dir(x, cx, y, cy);
               }
            }
            else
            {
               if (get_distform(x, y, ox, oy) < dist)
               {
                  dist = get_distform(x, y, ox, oy);
                  dir = get_curr_dir(x, ox, y, oy);
               }
            }
            if (fnt->f[cnt - 2] == -1)
               break;
         }
         sprintf(dbuf, "%s", owindd[dir]);
         if (fnt->type == 0)
            sprintf(tbuf, "Cold");
         else
            sprintf(tbuf, "Warm");
         if (dist >= 0 && dist <= 5)
            ch_printf(ch, "A %s front to the %s is just a few steps away from you.\n\r", tbuf, dbuf);
         if (dist >= 6 && dist <= 15)
            ch_printf(ch, "A %s front to the %s is a short distance away.\n\r", tbuf, dbuf);
         if (dist >= 16 && dist <= 25)
            ch_printf(ch, "A %s front to the %s is a moderate distance away.\n\r", tbuf, dbuf);
         if (dist >= 26 && dist <= 40)
            ch_printf(ch, "A %s front to the %s is a large distance away.\n\r", tbuf, dbuf);
         if (dist >= 41 && dist <= 55)
            ch_printf(ch, "A %s front is very far away in the general direction of %s.\n\r", tbuf, dbuf);
         if (dist >= 55 && dist <= 75)
            ch_printf(ch, "A %s front is a very SIZEABLE distance in the general direction of %s.\n\r", tbuf, dbuf);
         if (dist > 75)
            ch_printf(ch, "A %s front is a good FEW DAYS travel in the general direction of %s.\n\r", tbuf, dbuf);
      }
   }
}

void show_temp(CHAR_DATA * ch, int x, int y, int map)
{
   int temp;

   if (!ch)
      return;

   if (ch != NULL && x == -1 && y == -1 && map == -1)
   {
      x = ch->coord->x;
      y = ch->coord->y;
      map = ch->map;
   }
   //This is based in F, not C
   temp = generate_temperature(ch, x, y, map);


   if (temp < -30)
      send_to_char("As the frigid air flows by you, you can hear the voice of death in your ear.\n\r", ch);
   if (temp >= -29 && temp <= -20)
      send_to_char("If you do not find heat soon, your chances of surviving for long is low.\n\r", ch);
   if (temp >= -19 && temp <= -10)
      send_to_char("Any open bodypart will quickly freeze at this temperature.\n\r", ch);
   if (temp >= -9 && temp <= 5)
      send_to_char("It is very cold, but with enough clothing it is bareable.\n\r", ch);
   if (temp >= 6 && temp <= 20)
      send_to_char("A thick layer of clothing is a good idea because it is a bit below freezing.\n\r", ch);
   if (temp >= 21 && temp <= 35)
      send_to_char("It is right at the freezing mark, it is a good idea to be completely clothed.\n\r", ch);
   if (temp >= 36 && temp <= 45)
      send_to_char("It is a little chilly, but above freezing.\n\r", ch);
   if (temp >= 46 && temp <= 55)
      send_to_char("The air still has a bit of cold to it, but a pleasant cold.\n\r", ch);
   if (temp >= 56 && temp <= 65)
      send_to_char("The weather is pleasant, not too cold, not too hot.\n\r", ch);
   if (temp >= 66 && temp <= 80)
      send_to_char("The weather is pleasant, a bit warm, but not hot.\n\r", ch);
   if (temp >= 81 && temp <= 90)
      send_to_char("The air is a little humid, a little bit of cool air would be nice.\n\r", ch);
   if (temp >= 91 && temp <= 100)
      send_to_char("It is just below blistering hot, good swimming weather.\n\r", ch);
   if (temp >= 100 && temp <= 115)
      send_to_char("You can feel the sun burning your skin, it is rather uncomfertable.\n\r", ch);
   if (temp >= 116 && temp <= 130)
      send_to_char("It is hot enough to kill someone if they do not drink and stay out too long.\n\r", ch);
   if (temp > 130)
      send_to_char("Remember those bones you see in the desert, yeah this heat causes that.\n\r", ch);
}

//Gives out the messages for the wind direction
void generate_wind_dir(CHAR_DATA * ch, int x, int y, int map)
{
   char buf[MIL];

   if (!ch)
      return;

   if (ch != NULL && x == -1 && y == -1 && map == -1)
   {
      x = ch->coord->x;
      y = ch->coord->y;
      map = ch->map;
   }

   generate_wind(ch, x, y, map); //ALWAYS do this first so it sets the global variables.   

   if (windstr != -1 && winddir != -1)
      sprintf(buf, "%s", windd[winddir]);
   if (windstr == -1)
      ch_printf(ch, "This wind has died down to nothing.\n\r");
   if (windstr >= 0 && windstr <= 10)
      ch_printf(ch, "A gentle %s wind blows through the air.\n\r", buf);
   if (windstr >= 11 && windstr <= 20)
      ch_printf(ch, "A light %s wind flows pleasantly through the air.\n\r", buf);
   if (windstr >= 21 && windstr <= 40)
      ch_printf(ch, "The %s wind blows through the air ruffling everything in its path.\n\r", buf);
   if (windstr >= 41 && windstr <= 60)
      ch_printf(ch, "A strong %s wind buzzes through the air distrupting everything in sight.\n\r", buf);
   if (windstr >= 61 && windstr <= 90)
      ch_printf(ch, "A very strong %s wind is making everything in sight move or bend.\n\r", buf);
   if (windstr > 90)
      ch_printf(ch, "The %s wind is literally moving you, it is time to find a shelter.\n\r", buf);

//   ch_printf(ch, "\n\r%d %d\n\r", dir, str); 
   return;
}

//Load a new/fresh weather system if one is not present.
void init_area_weather()
{
   int temp;
   int cnt;

   temp = get_avg_temp(); //Temp

   first_front = last_front = NULL;

   //Create a few fronts, cold start nw, warm ne and move in opposite directions along the path
   for (cnt = 1; cnt <= 6; cnt++)
   {
      create_front(0);
      create_front(1);
   }

}

void save_snow()
{
   FILE *fp;
   char filename[256];
   int x, y, map;

   sprintf(filename, "%s", SNOW_FILE);

   FCLOSE(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_snow: fopen", 0);
      perror(filename);
   }
   else
   {
      fprintf(fp, "#SNOW\n");

      for (x = 1; x < MAX_X + 1; x++)
      {
         for (y = 1; y < MAX_Y + 1; y++)
         {
            for (map = 0; map < MAP_MAX; map++)
            {
               if (weather_sector[map][x][y] >= 10)
               {
                  fprintf(fp, "%d %d %d %d\n", weather_sector[map][x][y], x, y, map);
               }
            }
         }
      }
      fprintf(fp, "-1 -1 -1 -1\n");
      FCLOSE(fp);
   }
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

void load_snow()
{
   FILE *fp;
   char filename[256];
   char *ln;
   int x1, x2, x3, x4;

   sprintf(filename, "%s", SNOW_FILE);

   if ((fp = fopen(filename, "r")) == NULL)
   {
      bug("load_snow: File for resources not found!", 0);
      return;
   }
   for (;;)
   {
      char letter;
      char *word;

      letter = fread_letter(fp);
      if (letter != '#')
      {
         bug("load_caste_resources: # not found, possibly not a file.", 0);
         return;
      }

      word = fread_word(fp);

      if (!str_cmp(word, "SNOW"))
      {
         for (;;)
         {
            ln = fread_line(fp);
            x1 = x2 = x3 = x4 = 0;
            sscanf(ln, "%d %d %d %d", &x1, &x2, &x3, &x4);
            if (x1 == -1 && x2 == -1 && x3 == -1 && x4 == -1)
            {
               FCLOSE(fp);
               return;
            }
            weather_sector[x4][x2][x3] = x1;
         }
      }
   }
}

void load_resources()
{
    
    FILE *fp;
   char filename[256];
   int stx, sty, endx, endy, sector;
   int x, y;

   sprintf(filename, "%ssolan.res", MAP_DIR);

   if ((fp = fopen(filename, "r")) == NULL)
   {
      bug("load_resources: File for resources not found!", 0);
      shutdown_mud("Missing solan.res file");
      exit(1);
   }
   for (;;)
   {
      char letter;
      char *word;
      
      if (feof(fp))
         break;

      letter = fread_letter(fp);
      if (letter != '#')
      {
         bug("load_resources: # not found, possibly not a resource file.", 0);
         shutdown_mud("Bad file data in solan.res");
         exit(1);
      }

      word = fread_word(fp);

      if (!str_cmp(word, "RESOURCES"))
      {
         for (;;)
         {
            if (feof(fp))
            {
               bug("load_resources: Reached EOL.");
               break;
            }
            stx = fread_number(fp);
            sty = fread_number(fp);
            endx = fread_number(fp);
            endy = fread_number(fp);
            sector = fread_number(fp);
            for (x = stx; x <= endx; x++)
            {
               if (x == stx)
                  y = sty;
               else
                  y = 1;
                  
               for (;; y++)
               {
                 if ((x == endx && y > endy) || y == MAX_Y+1)
                    break;
                 resource_sector[0][x][y] = sector;
               }
            } 
            if (endx == MAX_X && endy == MAX_Y)
               break;
         }
      }

      else if (!str_cmp(word, "END"))
         break;
      else
      {
         char buf[MSL];

         sprintf(buf, "load_resources: bad section: %s.", word);
         bug(buf, 0);
         continue;
      }

   }
   FCLOSE(fp);
   return;
}

void load_maps(void)
{

   log_string("Loading continent of Solan....");
   load_solan();

   log_string("Loading resources for Solan....");
   load_resources();

   log_string("Loading overland map exits....");
   load_entrances();

   return;
}

void save_mapoutline(char *name, int map)
{
   FILE *fp;
   char filename[256];
   int x, y;

   name = strlower(name); /* Forces filename into lowercase */

   sprintf(filename, "%s%s.mapout", MAP_DIR, name);

   FCLOSE(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_map: fopen", 0);
      perror(filename);
   }
   else
   {

      for (y = 1; y < MAX_Y + 1; y++)
      {
         for (x = 1; x < MAX_X + 1; x++)
         {
            fprintf(fp, "%s", sect_show[(int)map_sector[map][x][y]].print);
         }
         fprintf(fp, "\n\r");
      }
      FCLOSE(fp);
   }
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

void save_resources(char *name, int map)
{
    FILE *fp;
   char filename[256];
   int x, y;
   int stx, sty;
   int cursect = -1;
   
   stx=sty=1;

   name = strlower(name);
   sprintf(filename, "%s%s.res", MAP_DIR, name);

   FCLOSE(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_resources: fopen", 0);
      perror(filename);
   }
   else
   {
      fprintf(fp, "#RESOURCES\n");

      for (x = 1; x < MAX_X + 1; x++)
      {
         for (y = 1; y < MAX_Y + 1; y++)
         {
            if (cursect == -1)
            {
               cursect = resource_sector[map][x][y];
               continue;
            }
            if (x == MAX_X && y == MAX_Y)
            {
               if (resource_sector[map][x][y] == cursect)
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x, y, cursect);
               }
               else
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x, y-1, cursect);
                  fprintf(fp, "%d %d %d %d %d\n", x, y, x, y, resource_sector[map][x][y]);
               }
               continue;
            }
            if (resource_sector[map][x][y] == cursect)
               continue;
            else
            {
               if (y > 1)
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x, y-1, cursect);
                  stx = x;
                  sty = y;
                  cursect = resource_sector[map][x][y];
                  continue;
               }
               else
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x-1, MAX_Y, cursect);
                  stx = x;
                  sty = y;
                  cursect = resource_sector[map][x][y];
                  continue;
               }
            }
         }
      }
      fprintf(fp, "#END\n");
      FCLOSE(fp);
   }
   fpReserve = fopen(NULL_FILE, "r");
   return;
}


void save_map(char *name, int map)
{
   FILE *fp;
   char filename[256];
   int x, y;
   int stx, sty;
   int cursect = -1;
   
   stx=sty=1;

   name = strlower(name);

   sprintf(filename, "%s%s.map", MAP_DIR, name);

   FCLOSE(fpReserve);
   if ((fp = fopen(filename, "w")) == NULL)
   {
      bug("save_map: fopen", 0);
      perror(filename);
   }
   else
   {
      fprintf(fp, "#MAP\n");

      for (x = 1; x < MAX_X + 1; x++)
      {
         for (y = 1; y < MAX_Y + 1; y++)
         {
            if (cursect == -1)
            {
               cursect = map_sector[map][x][y];
               continue;
            }
            if (x == MAX_X && y == MAX_Y)
            {
               if (map_sector[map][x][y] == cursect)
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x, y, cursect);
               }
               else
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x, y-1, cursect);
                  fprintf(fp, "%d %d %d %d %d\n", x, y, x, y, map_sector[map][x][y]);
               }
               continue;
            }
            if (map_sector[map][x][y] == cursect)
               continue;
            else
            {
               if (y > 1)
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x, y-1, cursect);
                  stx = x;
                  sty = y;
                  cursect = map_sector[map][x][y];
                  continue;
               }
               else
               {
                  fprintf(fp, "%d %d %d %d %d\n", stx, sty, x-1, MAX_Y, cursect);
                  stx = x;
                  sty = y;
                  cursect = map_sector[map][x][y];
                  continue;
               }
            }
         }
      }
      fprintf(fp, "#END\n");
      FCLOSE(fp);
   }
   fpReserve = fopen(NULL_FILE, "r");
   save_entrances();
   return;
}


void do_mapedit(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char buf[MSL];
   int value;
   int fnd = 0;

   if (IS_NPC(ch))
   {
      send_to_char("Sorry, NPCs can't edit the overland maps.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);

   if (arg1[0] == '\0')
   {
      if (!IS_ONMAP_FLAG(ch))
      {
         send_to_char("This command can only be used from an overland map.\n\r", ch);
         return;
      }
      if (IS_PLR_FLAG(ch, PLR_MAPEDIT))
      {
         REMOVE_PLR_FLAG(ch, PLR_MAPEDIT);
         send_to_char("&GMap editing mode is now OFF.\n\r", ch);
         return;
      }

      SET_PLR_FLAG(ch, PLR_MAPEDIT);
      send_to_char("&RMap editing mode is now ON.\n\r", ch);
      ch_printf(ch, "&YYou are currently creating %s sectors.&z\n\r", sect_show[ch->pcdata->secedit].desc);
      return;
   }

   if (!str_cmp(arg1, "help"))
   {
      send_to_char("Usage: mapedit sector <sector number>\n\r", ch);
      send_to_char("Usage: mapedit save <mapname>\n\r", ch);
      send_to_char("Usage: mapedit exit <vnum>\n\r", ch);
      send_to_char("Usage: mapedit exit map <mapname> <X> <Y>\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg1, "cleanresources"))
   {
      int x, y, changed;
      
      changed = 0;
      
      for (x = 1; x <= MAX_X; x++)
      {
         for (y = 1; y <= MAX_Y; y++)
         {
            if (map_sector[MAP_SOLAN][x][y] != SECT_SCORN && map_sector[MAP_SOLAN][x][y] != SECT_SGRAIN 
            && map_sector[MAP_SOLAN][x][y] != SECT_STREE && map_sector[MAP_SOLAN][x][y] != SECT_SSTONE
            && map_sector[MAP_SOLAN][x][y] != SECT_NCORN && map_sector[MAP_SOLAN][x][y] != SECT_NGRAIN
            && map_sector[MAP_SOLAN][x][y] != SECT_NTREE && map_sector[MAP_SOLAN][x][y] != SECT_NSTONE 
            && map_sector[MAP_SOLAN][x][y] != SECT_SGOLD && map_sector[MAP_SOLAN][x][y] != SECT_NGOLD 
            && map_sector[MAP_SOLAN][x][y] != SECT_SIRON && map_sector[MAP_SOLAN][x][y] != SECT_NIRON 
            && map_sector[MAP_SOLAN][x][y] != SECT_MINEGOLD && map_sector[MAP_SOLAN][x][y] != SECT_MINEIRON
            && map_sector[MAP_SOLAN][x][y] != SECT_FOREST && map_sector[MAP_SOLAN][x][y] != SECT_STONE
            && map_sector[MAP_SOLAN][x][y] != SECT_HCORN && map_sector[MAP_SOLAN][x][y] != SECT_HGRAIN)
            {
               resource_sector[MAP_SOLAN][x][y] = 0;
               changed++;
            }
         }
      }
      ch_printf(ch, "%d resource sectors have been changed.\n\r", changed);
      return;
   }
   if (!str_cmp(arg1, "putstones"))
   {
      int x, y, sx, sy;

      for (x = 1; x <= MAX_X; x++)
      {
         for (y = 1; y <= MAX_Y; y++)
         {
            fnd = 0;
            if (map_sector[MAP_SOLAN][x][y] == SECT_FIELD
               || map_sector[MAP_SOLAN][x][y] == SECT_HILLS || map_sector[MAP_SOLAN][x][y] == SECT_JUNGLE
               || map_sector[MAP_SOLAN][x][y] == SECT_SWAMP || map_sector[MAP_SOLAN][x][y] == SECT_PLAINS)
            {
               for (sx = x - 3; sx <= x + 3; sx++)
               {
                  for (sy = y - 3; sy <= y + 3; sy++)
                  {
                     if (map_sector[MAP_SOLAN][sx][sy] == SECT_RIVER)
                     {
                        if (number_range(1, 6) == 3)
                        {
                           map_sector[MAP_SOLAN][x][y] = SECT_STONE;
                           resource_sector[MAP_SOLAN][x][y] = 6000;
                           fnd = 1;
                        }
                        else
                           fnd = 1;
                     }
                     if (fnd == 1)
                        break;
                  }
                  if (fnd == 1)
                     break;
               }
            }
         }
      }
   }

   if (!str_cmp(arg1, "exit"))
   {
      ROOM_INDEX_DATA *location;
      char arg2[MIL];
      int vnum;

      if (!IS_ONMAP_FLAG(ch))
      {
         send_to_char("This command can only be used from an overland map.\n\r", ch);
         return;
      }
      if (ch->map == -1)
      {
         bug("do_mapedit: %s is not on a valid map!", ch->name);
         send_to_char("Can't do that - your on an invalid map.\n\r", ch);
         return;
      }

      argument = one_argument(argument, arg2);

      if (arg2[0] == '\0')
      {
         send_to_char("Usage: mapedit exit <vnum>\n\r", ch);
         send_to_char("Usage: mapedit exit map <mapname> <X> <Y>\n\r", ch);
         return;
      }

      if (!str_cmp(arg2, "map"))
      {
         char arg3[MIL];
         char arg4[MIL];
         int x, y;
         int map = -1;

         argument = one_argument(argument, arg3);
         argument = one_argument(argument, arg4);

         if (!IS_ONMAP_FLAG(ch))
         {
            send_to_char("This command can only be used from an overland map.\n\r", ch);
            return;
         }

         if (arg3[0] == '\0')
         {
            send_to_char("Make an exit to what map??\n\r", ch);
            return;
         }

         if (!str_cmp(arg3, "solan"))
            map = MAP_SOLAN;

         if (map == -1)
         {
            ch_printf(ch, "There isn't a map for '%s'.\n\r", arg3);
            return;
         }

         x = atoi(arg4);
         y = atoi(argument);

         if (x < 1 || x > MAX_X)
         {
            sprintf(buf, "Valid x coordinates are 1 to %d.\n\r", MAX_X);
            send_to_char(buf, ch);
            return;
         }

         if (y < 1 || y > MAX_Y)
         {
            sprintf(buf, "Valid y coordinates are 1 to %d.\n\r", MAX_Y);
            send_to_char(buf, ch);
            return;
         }

         add_entrance(map, ch->map, ch->coord->x, ch->coord->y, x, y, -1);
         ch_printf(ch, "Exit created to map of %s, at %dX, %dY.\n\r", arg3, x, y);

         map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_EXIT;
         return;
      }

      vnum = atoi(arg2);

      if ((location = get_room_index(vnum)) == NULL)
      {
         send_to_char("No such room exists.\n\r", ch);
         return;
      }

      add_entrance(-1, ch->map, ch->coord->x, ch->coord->y, -1, -1, vnum);
      ch_printf(ch, "Exit created to room %d.\n\r", vnum);
      map_sector[ch->map][ch->coord->x][ch->coord->y] = SECT_EXIT;
      return;
   }
   if (!str_cmp(arg1, "save"))
   {
      char buf[MSL];
      int map = -1;

      if (!IS_ONMAP_FLAG(ch))
      {
         send_to_char("This command can only be used from an overland map.\n\r", ch);
         return;
      }

      if (argument[0] == '\0')
      {
         char *mapname;

         if (ch->map == -1)
         {
            bug("do_mapedit: %s is not on a valid map!", ch->name);
            send_to_char("Can't do that - your on an invalid map.\n\r", ch);
            return;
         }
         mapname = map_name[ch->map];

         sprintf(buf, "Saving map of %s....\n\r", mapname);

         send_to_pager(buf, ch);
         save_map(mapname, ch->map);
         return;
      }

      if (!str_cmp(argument, "solan"))
         map = MAP_SOLAN;

      if (map == -1)
      {
         ch_printf(ch, "There isn't a map for '%s'.\n\r", arg1);
         return;
      }

      sprintf(buf, "Saving map of %s....", argument);
      send_to_pager(buf, ch);
      save_map(argument, map);
      return;
   }

   if (!str_cmp(arg1, "saveoutline"))
   {
      char buf[MSL];
      int map = -1;

      if (!IS_ONMAP_FLAG(ch))
      {
         send_to_char("This command can only be used from an overland map.\n\r", ch);
         return;
      }

      if (argument[0] == '\0')
      {
         char *mapname;

         if (ch->map == -1)
         {
            bug("do_mapedit: %s is not on a valid map!", ch->name);
            send_to_char("Can't do that - your on an invalid map.\n\r", ch);
            return;
         }
         mapname = map_name[ch->map];

         sprintf(buf, "Saving mapoutline of %s....\n\r", mapname);

         send_to_pager(buf, ch);
         save_mapoutline(mapname, ch->map);
         return;
      }

      if (!str_cmp(argument, "solan"))
         map = MAP_SOLAN;

      if (map == -1)
      {
         ch_printf(ch, "There isn't a map for '%s'.\n\r", arg1);
         return;
      }

      sprintf(buf, "Saving mapoutline  of %s....", argument);
      send_to_pager(buf, ch);
      save_mapoutline(argument, map);
      return;
   }

   if (!str_cmp(arg1, "sector"))
   {
      char arg2[MIL];

      argument = one_argument(argument, arg2);

      if (!IS_ONMAP_FLAG(ch))
      {
         send_to_char("This command can only be used from an overland map.\n\r", ch);
         return;
      }

      if (isalpha(arg2[0]))
      {
         send_to_char("Only takes numeric values, help sectortypes", ch);
         return;
      }

      value = atoi(arg2);
      if (value < 0 || value >= SECT_MAX)
      {
         send_to_char("Invalid sector type.\n\r", ch);
         return;
      }

      if (!str_cmp(arg2, "exit"))
      {
         send_to_char("You cannot place exits this way.\n\r", ch);
         send_to_char("Usage: mapedit exit <vnum>\n\r", ch);
         send_to_char("Usage: mapedit exit map <mapname> <X> <Y>\n\r", ch);
         return;
      }

      ch->pcdata->secedit = value;
      ch_printf(ch, "&YYou are now creating %s sectors.\n\r", sect_show[value].desc);
      return;
   }

   send_to_char("Usage: mapedit sector <sector number>\n\r", ch);
   send_to_char("Usage: mapedit save <mapname>\n\r", ch);
   send_to_char("Usage: mapedit exit <vnum>\n\r", ch);
   send_to_char("Usage: mapedit exit map <mapname> <X> <Y>\n\r", ch);
   return;
}

//Moves a block of wilderness to a different block, BE CAREFUL when using, it isn't meant to be used more than once or twice
void do_blockmove(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   ENTRANCE_DATA *enter;
   ROOM_INDEX_DATA *room;
   EXIT_DATA *pexit;
   int xstart, ystart, xend, yend, xshift, yshift, x, y;
   WBLOCK_DATA *wblock;
   WBLOCK_DATA *sblock;
   
   if (IS_NPC(ch))
   {
      send_to_char("NPCs cannot do this.\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   argument = one_argument(argument, arg4);
   argument = one_argument(argument, arg5);


   if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' || arg5[0] == '\0' || argument[0] == '\0')
   {
      send_to_char("Syntax: blockmove <startx> <starty> <endx> <endy> <xshift> <yshift>\n\r", ch);
      send_to_char("WARNING:  This command moves a block of sectors and remaps it to the new shift location.\n\r          It then deletes the block you shifted, so make a backup.\n\r", ch);
      return;
   }
   xstart = atoi(arg1);
   ystart = atoi(arg2);
   xend = atoi(arg3);
   yend = atoi(arg4);
   xshift = atoi(arg5);
   yshift = atoi(argument);
   if (xstart < 1 || xstart > MAX_X || ystart < 1 || ystart > MAX_Y || xend < 1 || xend > MAX_X || yend < 1 || yend > MAX_Y)
   {
      ch_printf(ch, "Valid coordinates are from 1 to %d.\n\r", MAX_X);
      return;
   }

   if (xend < xstart)
   {
      send_to_char("Invalid X range specified.\n\r", ch);
      return;
   }

   if (yend < ystart)
   {
      send_to_char("Invalid Y range specified.\n\r", ch);
      return;
   }
   if (xshift < 0)
   {
      if (xshift < -MAX_X)
      {
         ch_printf(ch, "xshift cannot be less than %d.\n\r", MAX_X*-1);
         return;
      }
      if (xstart + xshift < 1)
      {
         send_to_char("When applying the xshift, the startx value will be less than 1.\n\r", ch);
         return;
      }
   }
   else
   {
      if (xshift > MAX_X)
      {
         ch_printf(ch, "xshift cannot be greater than %d.\n\r", MAX_X);
         return;
      }
      if (xend + xshift > MAX_X)
      {
         ch_printf(ch, "When applying the xshift, the endx value will be greater than %d.\n\r", MAX_X);
         return;
      }
   }
   
   if (yshift < 0)
   {
      if (yshift < -MAX_Y)
      {
         ch_printf(ch, "yxshift cannot be less than %d.\n\r", MAX_Y*-1);
         return;
      }
      if (ystart + yshift < 1)
      {
         send_to_char("When applying the yshift, the starty value will be less than 1.\n\r", ch);
         return;
      }
   }
   else
   {
      if (yshift > MAX_Y)
      {
         ch_printf(ch, "yshift cannot be greater than %d.\n\r", MAX_Y);
         return;
      }
      if (yend + yshift > MAX_Y)
      {
         ch_printf(ch, "When applying the yshift, the endx value will be greater than %d.\n\r", MAX_X);
         return;
      }
   }
   
   
   for (y = ystart; y < yend + 1; y++)
   {
      for (x = xstart; x < xend + 1; x++)
      {
         map_sector[ch->map][x+xshift][y+yshift] = map_sector[ch->map][x][y];
      }
   }
   for (y = ystart; y < yend + 1; y++)
   {
      for (x = xstart; x < xend + 1; x++)
      {
         map_sector[ch->map][x][y] = SECT_FIELD;
      }
   }
   for (y = ystart; y < yend + 1; y++)
   {
      for (x = xstart; x < xend + 1; x++)
      {
         resource_sector[ch->map][x+xshift][y+yshift] = resource_sector[ch->map][x][y];
      }
   }
   for (y = ystart; y < yend + 1; y++)
   {
      for (x = xstart; x < xend + 1; x++)
      {
         resource_sector[ch->map][x][y] = 0;
      }
   }
   ch_printf(ch, "All map sectors between %dX %dY and %dX %dY shifted %dX and %dY.\n\r", xstart, ystart, xend, yend, xshift, yshift);
   //do exits now
   for (enter = first_entrance; enter; enter = enter->next)
   {
      if (enter->here->x >= xstart && enter->here->x <= xend && enter->here->y >= ystart && enter->here->y <= yend)
      {
         room = get_room_index(enter->vnum);
         for (pexit = room->first_exit; pexit; pexit = pexit->next)
         {
            if (pexit->coord && pexit->coord->x >= xstart && pexit->coord->y <= xend && pexit->coord->y >= ystart && pexit->coord->y <= yend)
            {
               pexit->coord->x += xshift;
               pexit->coord->y += yshift;
               fold_area(room->area, room->area->filename, FALSE, 1);  
            }
         }
      }
   }
   ch_printf(ch, "All areas with exits out into the block have been corrected.\n\r", ch);   
   for (enter = first_entrance; enter; enter = enter->next)
   {
      if (enter->here->x >= xstart && enter->here->x <= xend && enter->here->y >= ystart && enter->here->y <= yend)
      {
         enter->here->x += xshift;
         enter->here->y += yshift;
      }
   }
   send_to_char("All Entrances in the block area have been shifted to their correct spots.\n\r", ch);
   
   xstart = (xstart /50);
   xstart = xstart * 50;
   xend = (xend /50)+1;
   xend = xend * 50 - 1;
   
   ystart = (ystart /50);
   ystart = ystart * 50;
   yend = (yend /50)+1;
   yend = yend * 50 - 1;
   
         
   for (wblock = first_wblock; wblock; wblock = wblock->next)
   {
      if (wblock->stx >= xstart && wblock->sty >= ystart && wblock->endx <= xend && wblock->endy <= yend)
      {
          for (sblock = first_wblock; sblock; sblock = sblock->next)     
          {
             if (wblock->stx+xshift == sblock->stx && wblock->sty+yshift == sblock->sty)
             {
                sblock->lvl = wblock->lvl;
                wblock->lvl = 10;
                sblock->kills = wblock->kills;
                wblock->kills = 0;
             }
          }
       }
   }
   return;
}

//Replaces all sectors in a certain direction till it meets a certain sectortype, good for fill out
//an outline
void do_mapline(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   int max;
   int skip;
   int cnt;
   int x;
   int y;
   
   if (IS_NPC(ch))
   {
      send_to_char("NPCs cannot edit the overland maps.\n\r", ch);
      return;
   }
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  mapline <dir> <start> <end> <new sector> <till sector> [skips]\n\r", ch);
      send_to_char("   dir:  0 - North, 1 - East, 2 - South, 3 - West\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   argument = one_argument(argument, arg4);
   argument = one_argument(argument, arg5);
   
   if (atoi(arg1) < 0 || atoi(arg1) > 3)
   {
      send_to_char("Direction is 0 to 3.\n\r", ch);
      return;
   }
   if (atoi(arg1) == 0 || atoi(arg1) == 2)
      max = MAX_X;
   else
      max = MAX_Y;
   
   if (atoi(arg2) < 1 || atoi(arg2) > max)
   {
      send_to_char("That is not a valid starting coordinate.\n\r", ch);
      return;
   }
   if (atoi(arg3) < 1 || atoi(arg3) > max)
   {
      send_to_char("That is not a valid ending coordinate.\n\r", ch);
      return;
   }
   if (atoi(arg3) < atoi(arg2))
   {
      send_to_char("That is not a valid range.\n\r", ch);
      return;
   }
   if (atoi(arg4) < 0 || atoi(arg4) > SECT_MAX)
   {
      send_to_char("That is not a valid sectortype.\n\r", ch);
      return;
   }
   if (atoi(arg5) < 0 || atoi(arg5) > SECT_MAX)
   {
      send_to_char("That is not a valid sectortype.\n\r", ch);
      return;
   }
   if (atoi(argument) > 0)
      skip = atoi(argument);
   else
      skip = 0;
      
   if (atoi(arg1) == 0 || atoi(arg1) == 2)
   {
      for (x = atoi(arg2); x <= atoi(arg3); x++)
      {
         cnt = 0;
         if (atoi(arg1) == 0)
         {
            for (y = ch->coord->y; y >= 1; y--)
            {
               if (map_sector[ch->map][x][y] == atoi(arg5))
               {
                  if (skip == 0)
                     break;
                  else
                  {
                     if (cnt == skip)
                        break;
                     else
                        cnt++;
                  }
               }  
               map_sector[ch->map][x][y] = atoi(arg4);
            }
         }
         else
         {
            for (y = ch->coord->y; y <= MAX_Y; y++)
            {
               if (map_sector[ch->map][x][y] == atoi(arg5))
               {
                  if (skip == 0)
                     break;
                  else
                  {
                     if (cnt == skip)
                        break;
                     else
                        cnt++;
                  }
               }  
               map_sector[ch->map][x][y] = atoi(arg4);
            }
         }
      }
      send_to_char("Done.\n\r", ch);
      return;
   }
   else
   {
      for (y = atoi(arg2); y <= atoi(arg3); y++)
      {
         cnt = 0;
         if (atoi(arg1) == 3)
         {
            for (x = ch->coord->x; x >= 1; x--)
            {
               if (map_sector[ch->map][x][y] == atoi(arg5))
               {
                  if (skip == 0)
                     break;
                  else
                  {
                     if (cnt == skip)
                        break;
                     else
                        cnt++;
                  }
               }  
               map_sector[ch->map][x][y] = atoi(arg4);
            }
         }
         else
         {
            for (x = ch->coord->x; x <= MAX_X; x++)
            {
               if (map_sector[ch->map][x][y] == atoi(arg5))
               {
                  if (skip == 0)
                     break;
                  else
                  {
                     if (cnt == skip)
                        break;
                     else
                        cnt++;
                  }
               }  
               map_sector[ch->map][x][y] = atoi(arg4);
            }
         }
      }   
      send_to_char("Done.\n\r", ch);
      return;
   }
   do_mapline(ch, "");
   return;
}
       
      
   
   
   

void do_mapat(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   char arg6[MIL];
   char buf[MSL];
   int count, cx;
   int fnd = 0;
   int keep[10]; //Max 10
   int xstart, ystart, xend, yend, sector, x, y;

   count = -1;

   if (IS_NPC(ch))
   {
      send_to_char("NPCs cannot edit the overland maps.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   argument = one_argument(argument, arg4);
   argument = one_argument(argument, arg5);


   if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0' || arg4[0] == '\0' || arg5[0] == '\0')
   {
      send_to_char
         ("Usage: mapat <startx> <starty> <endx> <endy> <sectortype> <keep sectors>\n\rKeep Sectors has a limit of 10 sectors, values only.\n\r", ch);
      return;
   }

   xstart = atoi(arg1);
   ystart = atoi(arg2);
   xend = atoi(arg3);
   yend = atoi(arg4);

   if (xstart < 1 || xstart > MAX_X || ystart < 1 || ystart > MAX_Y || xend < 1 || xend > MAX_X || yend < 1 || yend > MAX_Y)
   {
      sprintf(buf, "Valid coordinates are from 1 to %d.\n\r", MAX_X);
      send_to_char(buf, ch);
      return;
   }

   if (xend < xstart)
   {
      send_to_char("Invalid X range specified.\n\r", ch);
      return;
   }

   if (yend < ystart)
   {
      send_to_char("Invalid Y range specified.\n\r", ch);
      return;
   }

   sector = atoi(arg5);

   if (sector < 0 || sector > SECT_MAX)
   {
      send_to_char("Invalid sector type.\n\r", ch);
      return;
   }

   if (sector == SECT_EXIT)
   {
      send_to_char("Exits cannot be range edited!\n\r", ch);
      return;
   }
   for (;;)
   {
      if (argument[0] == '\0')
      {
         break;
      }
      else
      {
         if (count == 9)
         {
            send_to_char("You can only have 10 do not change values.\n\r", ch);
            return;
         }
         else
         {
            count++;
            argument = one_argument(argument, arg6);
            keep[count] = atoi(arg6);
         }
      }
   }
   

   for (y = ystart; y < yend + 1; y++)
   {
      for (x = xstart; x < xend + 1; x++)
      {
         fnd = 0;
         if (count != -1)
         {
            for (cx = 0; cx <= count; cx++)
            {
               if (map_sector[ch->map][x][y] == keep[cx])
                  fnd = 1;
            }
         }
         if (fnd == 1)
            continue;
         if (map_sector[ch->map][x][y] != SECT_EXIT)
            map_sector[ch->map][x][y] = sector;
      }
   }
   ch_printf(ch, "All map sectors between %dX %dY and %dX %dY changed to %d.\n\r", xstart, ystart, xend, yend, sector);
}
