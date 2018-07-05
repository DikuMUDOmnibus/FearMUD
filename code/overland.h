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
 *			    Overland Map Display and Support Headers              *
 ****************************************************************************/

#define OVERLAND_SOLAN  16000
#define OVERLAND_LOW_MOB 16000
#define OVERLAND_HI_MOB  16499
#define TREE_O_VNUM 16070
#define GRASS_O_VNUM 16071
#define CORN_O_VNUM 16119
#define GRAIN_O_VNUM 16120
#define MAX_X 1500
#define MAX_Y 1000
#define ENTRY_X 258
#define ENTRY_Y 250
#define LAST_PORTAL 100
#define MAX_MOB_HOLDER 200

typedef enum
{
   MAP_SOLAN, MAP_MAX
}
map_types;

#define NO_SNOW(sector)  (no_snow(sector))

//Globals

/* 3 arrays */
extern unsigned char map_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1];
extern unsigned char kingdom_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1];
extern short resource_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1];
extern char weather_sector[MAP_MAX][MAX_X + 1][MAX_Y + 1];
extern const int front_cr[8][30];
extern char *const owindd[16];
extern char *const map_names[MAP_MAX];
extern int winddir;
extern int windstr;

#define MAP_DIR "../maps/"
#define ENTRANCE_FILE "entrances.dat"
#define SNOW_FILE   SYSTEM_DIR "snow.dat"

typedef struct coord_data COORD_DATA;
typedef struct entrance_data ENTRANCE_DATA;
typedef struct portal_data PORTAL_DATA;

extern ENTRANCE_DATA *first_entrance;
extern ENTRANCE_DATA *last_entrance;

struct coord_data
{
   sh_int x;
   sh_int y;
};

struct portal_data
{
   sh_int x;
   sh_int y;
   sh_int map;
   char *desc;
};

struct entrance_data
{
   ENTRANCE_DATA *next;
   ENTRANCE_DATA *prev;
   COORD_DATA *here; /* Coordinates the entrance is at */
   COORD_DATA *there; /* Coordinates the entrance goes to, if any */
   sh_int tomap; /* Map it goes to, if any */
   sh_int onmap; /* Which map it's on */
   int vnum; /* Target vnum if it goes to a regular zone */
};

struct sect_color_type
{
   sh_int sector; /* Terrain sector */
   char *color; /* Color to display as */
   char *symbol; /* Symbol you see for the sector */
   char *print; /* Printing symbol */
   char *invert; /* Background color?? */
   sh_int encounter; /* Encounter chance */
   sh_int move; /* Movement costs */
   char *desc; /* Description of sector type */
   bool canpass; /* Impassable terrain */
   char *tilefile; //Filename of the Tile used
};

DECLARE_DO_FUN(do_mapedit); /* Map OLC function */
DECLARE_DO_FUN(do_mapat); /* Ranged map OLC editor */
DECLARE_DO_FUN(do_coords); /* To jump to different coordinates on the map */

void save_snow args((void));
void load_snow args((void));
ENTRANCE_DATA *check_entrance args((CHAR_DATA * ch, int map, int x, int y));
bool no_snow args((int sector));
void enter_map args((CHAR_DATA * ch, int x, int y, int continent));
void leave_map args((CHAR_DATA * ch, ROOM_INDEX_DATA * target, int dir, int qexit));
void generate_wind_dir args((CHAR_DATA * ch, int x, int y, int map));
void create_front args((int type));
void check_torn_damage args((int x, int y, int map));
void show_temp args((CHAR_DATA * ch, int x, int y, int map));
int generate_temperature args((CHAR_DATA * ch, int x, int y, int map));
void update_local_weather args((FRONT_DATA * fnt, int type));
void generate_forecast args((CHAR_DATA * ch, int x, int y, int map));
void display_map args((CHAR_DATA * ch, sh_int vx, sh_int vy, sh_int eoc));
int get_curr_dir args((int x, int fx, int y, int fy));
