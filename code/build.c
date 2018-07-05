/****************************************************************************
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.0 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
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
 *		       Online Building and Editing Module		    *
 ****************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "mud.h"



extern int top_affect;
extern int top_reset;
extern int top_ed;
extern bool fBootDb;
int xitfailed;

/*
 * Exit Pull/push types
 * (water, air, earth, fire)
 */
char *const ex_pmisc[] = { "undefined", "vortex", "vacuum", "slip", "ice", "mysterious" };

char *const ex_pwater[] = { "current", "wave", "whirlpool", "geyser" };

char *const ex_pair[] = { "wind", "storm", "coldwind", "breeze" };

char *const ex_pearth[] = { "landslide", "sinkhole", "quicksand", "earthquake" };

char *const ex_pfire[] = { "lava", "hotair" };


char *const ex_flags[] = {
   "isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
   "dig", "eatkey", "nopassdoor", "hidden", "passage", "portal", "r1", "r2",
   "can_climb", "can_enter", "can_leave", "auto", "noflee", "searchable",
   "bashed", "bashproof", "nomob", "window", "can_look", "overland"
};

/* r_flags is now extended -- Xerves 11/99 */
char *const r_flags[] = {
   "dark", "death", "nomob", "indoors", "lawful", "neutral", "chaotic",
   "nomagic", "tunnel", "private", "safe", "solitary", "petshop", "norecall",
   "donation", "nodropall", "silence", "logspeech", "nodrop", "clanstoreroom",
   "nosummon", "noastral", "teleport", "teleshowdesc", "nofloor",
   "nosupplicate", "arena", "nomissile", "noexit", "imp", "prototype", "casteroom",
   "mark", "wilderness", "keepdesc", "map", "mountshop", "portal", "freekill", "anitem",
   "noloot", "tsafe", "nowdam", "forgeroom", "mananode", "nomilitary", "marketplace",
   "permdeath"
};

char *const o_flags[] = {
   "glow", "hum", "dark", "loyal", "evil", "invis", "magic", "nodrop", "bless",
   "antigood", "antievil", "antineutral", "noremove", "inventory",
   "antimage", "antithief", "antiwarrior", "anticleric", "organic", "metal",
   "donation", "clanobject", "clancorpse", "antivampire", "antidruid",
   "hidden", "poisoned", "covering", "deathrot", "buried", "prototype",
   "nolocate", "groundrot", "nogive", "nopurge", "antimonk", "antipaladin",
   "antiranger", "antiaugurer", "nodisarm", "nobreak", "onmap", "lodged", "piece",
   "artifact", "forgeable", "coal", "ore", "slab", "twohanded", "imbuable", "gem",
   "gemsetting", "sanctified", "noreset", "timereset", "cloak","mortar", "powreag", 
   "affreag", "mixed", "permreag","hideidentity", "repairwall", "kingdomkey",
   "kingdomeq", "gagremove", "corpserevive", "unique", "questobj", "typesword",
   "typeaxe", "typedagger", "typepolearm", "typestaves", "typeblunt", "qtokenlooted",
   "monkweapon"
};

char *const talent_flags[] = {
   "hp1", "hp2", "hp3", "hp4", "hp5", "mp1", "mp2", "mp3", "mp4", "mp5", "str1",
   "str2", "str3", "con1", "con2", "con3", "int1", "int2", "int3", "wis1", "wis2",
   "wis3", "dex1", "dex2", "dex3", "agi1", "agi2", "agi3", "end1", "end2", "end3",
   "sp1", "sp2", "sp3", "sp4", "sp5", "damcap1", "damcap2", "damcap3"
};

char *const mag_flags[] = {
   "returning", "backstabber", "bane", "loyal", "haste", "drain",
   "lightning_blade"
};

char *const w_flags[] = {
   "take", "finger", "neck", "body", "head", "legs", "arms",
   "shield", "waist", "wield", "_dual_", "missile", "lodge_rib",
   "lodge_arm", "lodge_leg", "aneck", "nocked", "back", "r2", "r3", "r4", "r5", "r6",
   "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
};

char *const area_flags[] = {
   "nopkill", "freekill", "noteleport", "noquest", "changed", "resource", "carpenter", "noarea", "anitem",
   "noloot", "nowdam", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
   "r18", "r19", "r20", "r21", "r22", "r23", "r24",
   "r25", "r26", "r27", "r28", "r29", "r30", "r31"
};

char *const o_types[] = {
   "none", "light", "scroll", "wand", "staff", "weapon", "_fireweapon", "_missile",
   "treasure", "armor", "potion", "_worn", "furniture", "trash", "_oldtrap",
   "container", "_note", "drinkcon", "key", "food", "money", "pen", "boat",
   "corpse", "corpse_pc", "fountain", "pill", "blood", "bloodstain",
   "scraps", "pipe", "herbcon", "herb", "incense", "fire", "book", "switch",
   "lever", "pullchain", "button", "dial", "rune", "runepouch", "match", "trap",
   "map", "portal", "paper", "tinder", "lockpick", "spike", "disease", "oil",
   "fuel", "_empty1", "_empty2", "missileweapon", "projectile", "quiver", "shovel",
   "salve", "cook", "keyring", "odor", "mountfood", "holdresource", "extractobj",
   "spellbook", "sheath", "noteboard", "repair", "mclimb", "gag", "flint", "toolkit",
   "gem", "questtoken"
};

char *const buykobj_types[] = 
{
   "hometown", "wilderness", "container", "sigil", "noteboard", "toroom", "tochar", "addreset", "bin",
   "wallrepair"
};

char *const buykmob_types[] = 
{
   "wilderness", "military", "hour", "4month", "addreset", "repair", "forge", "cleric", "worker",
   "guard", "soldier", "scout", "mage", "soldieragi", "soldiermove", "soldierdam", "soldierunarmed",
   "mounted", "soldieraddagi", "soldieraddmove", "soldieronemove", "tohit1", "tohit2", "tohit3", "tohit4",
   "ac1", "ac2", "ac3", "ac4", "dam1", "dam2", "dam3", "dam4", "curelight", "cureserious", "curecritical",
   "heal", "divinity", "powerheal", "bless", "armor", "stoneskin", "sanctify", "fleetarms", "sanctuary",
   "harm", "clericdamage", "leatheronly", "lightonly", "mediumonly", "shield", "kindred", "slink", "fireshield",
   "iceshield", "shockshield", "antimagicshell", "magedamage1", "magedamage2", "magedamage3", "magedamage4",
   "archer"
};
   
char *const qmob_flags[] = {   
   "hp", "agi", "str", "str2", "int", "int2", "lint", "lint2", "lhp", "lagi", "lstr",
   "lstr2", "dex", "ldex", "con", "lcon", "wis", "wis2", "lwis", "lwis2", "lck", "llck",
   "armor", "armor2", "larmor", "larmor2", "bash", "lbash", "slash", "lslash", "stab", "lstab",
   "dam", "dam2", "ldam", "ldam2", "nogold", "gold1", "gold2", "lgold1", "lgold2",
   "noaggro", "running", "undead", "livingdead", "sentinel", "dodge", "parry",
   "light", "serious", "critical", "heal", "sanctuary", "fshield", "sshield", 
   "ishield", "stoneskin", "disarm", "trip", "abash", "stun", "gouge", "backstab", "blindness",
   "lbreath", "gbreath", "firebreath", "frostbreath", "acidbreath", "curse", "harm", "fireball",
   "weaken", "poison", "sfire", "scold", "select", "senergy", "sblunt", "spierce", "sslash", "ssleep",
   "scharm", "snonmagic", "smagic", "sparalysis", "sair", "rfire", "rcold", "relect", "renergy", "rblunt",
   "rpierce", "rslash", "rsleep", "rcharm", "rnonmagic", "rmagic", "rparalysis", "rair", "ifire", "icold",
   "ielect", "ienergy", "iblunt", "ipierce", "islash", "isleep", "icharm", "inonmagic", "imagic", "iparalysis",
   "iair", "invisibile", "detectinvis", "hide", "truesight", "sneak", "detecthidden"
};
   
   
char *const a_types[] = {
   "none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
   "sex", "class", "level", "age", "height", "weight", "mana", "hit", "move",
   "gold", "experience", "ac", "hitroll", "damroll", "save_poison", "save_rod",
   "save_para", "save_breath", "save_spell", "charisma", "affected", "resistant",
   "immune", "susceptible", "weaponspell", "luck", "backstab", "pick", "track",
   "steal", "sneak", "hide", "palm", "detrap", "dodge", "peek", "scan", "gouge",
   "search", "mount", "disarm", "kick", "parry", "bash", "stun", "punch", "climb",
   "grip", "scribe", "brew", "wearspell", "removespell", "mentalstate", "emotion",
   "stripsn", "remove", "dig", "full", "thirst", "drunk", "blood", "cook",
   "recurringspell", "contagious", "xaffected", "odor", "roomflag", "sectortype",
   "roomlight", "televnum", "teledelay", "agi", "armor", "shield", "stone", "sanctify",
   "tohit", "managen", "hpgen", "weightmod", "manafuse", "fasting", "manashell", "manashield",
   "managuard", "manaburn", "weaponclamp", "arrowcatch", "bracing", "hardening",
   "rfire", "rwater", "rair", "rearth", "renergy", "rmagic", "rnonmagic", "rblunt", "rpierce",
   "rslash", "rpoison", "rparalysis", "rholy", "runholy", "rundead"
};

char *const a_flags[] = {
   "blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
   "detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
   "_flaming", "poison", "protect", "_paralysis", "sneak", "hide", "sleep",
   "charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
   "scrying", "fireshield", "shockshield", "r1", "iceshield", "possess",
   "berserk", "aqua_breath", "recurringspell", "contagious", "wizardeye",
   "e_wizardeye", "m_wizardeye", "balance", "nohunger", "nothirst", "gagged",
   "rez", "web", "snare", "nervepinch", "nyiji", "stalk"
};

char *const mi_flags[] = {
   "stationary", "patrol", "warn", "attacke", "attackn", "attacka", "nopass", "report",
   "invite", "noassist", "sentinel", "conquer", "nocloak", "nohood", "attackh", "attackc",
   "needintro", "kequiped"
};

char *const act_flags[] = {
   "npc", "sentinel", "scavenger", "banker", "castemob", "aggressive", "stayarea",
   "wimpy", "pet", "train", "practice", "immortal", "noquest", "polyself",
   "meta_aggr", "guardian", "running", "nowander", "mountable", "mounted",
   "scholar", "secretive", "notrack", "mobinvis", "noassist", "autonomous",
   "pacifist", "noattack", "annoying", "healer", "prototype", "r1", "protect",
   "scared", "trainer", "onmap", "mountsave", "watermob", "extractmob", "dumpgoods",
   "noupdn", "movemap", "undead", "kingdommob", "military", "sbseller", "restorelimbs", 
   "livingdead", "noreset", "timereset", "extractgoods", "repair", "noinsta", "nomercy",
   "forgemob", "extracttown", "grabbed", "captain", "boss", "questmob", "allowride"
};

char *const pc_flags[] = {
   "r1", "deadly", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
   "nosummon", "pager", "notitled", "groupwho", "diagnose", "highgag", "watch",
   "nstart", "nobeep", "nofinger", "nogroup", "autoproto", "cnoassist", "r15", "r16", "r17", "r18",
   "r19", "r20", "r21", "r22", "r23", "r24", "r25"
};

char *const plr_flags[] = {
   "npc", "boughtpet", "shovedrag", "autoexits", "autoloot", "autosac", "blank",
   "outcast", "brief", "combine", "prompt", "telnet_ga", "holylight",
   "wizinvis", "roomvnum", "silence", "noemote", "attacker", "notell", "log",
   "deny", "freeze", "thief", "killer", "litterbug", "ansi", "rip", "nice",
   "flee", "autogold", "automap", "afk", "invisprompt", "questor", "remort",
   "statquestor", "oset", "mset", "redit", "gambler", "notrans", "onmap", "mapedit",
   "boughtmount", "pov", "portalhunt", "arenachar", "away", "pkreset", "warned",
   "noweather", "target", "carrybin", "Mxp", "rpsetup", "showasimm", "shownames",
   "uknown", "showpc", "haslastname", "onduty", "nosimiliar", "kreset", "mmobiles",
   "parry", "dodge", "tumble", "mapwindow", "autosplit", "wildertiles", "noriders",
   "notohit", "questloot", "counter"
};

char *const trap_flags[] = {
   "room", "obj", "enter", "leave", "open", "close", "get", "put", "pick",
   "unlock", "north", "south", "east", "west", "up", "down", "examine",
   "northeast", "northwest", "southeast", "southwest", "getobj", "putobj", "wearobj", 
   "dropobj", "identobj", "giveobj", "sacobj", "removeobj"
};

char *const cmd_flags[] = {
   "possessed", "polymorphed", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
   "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
   "r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30"
};

char *const element_flags[] = {
   "air", "earth", "water", "fire", "energy", "unholy", "divine", "undead"
};

char *const wear_locs[] = {
   "light", "finger1", "finger2", "neck", "aneck", "body", "head", "leg1",
   "leg2", "arm1", "arm2", "shield", "waist", "wield", "dual_wield", "missile_wield",
   "lodge_rib", "lodge_arm", "lodge_leg", "nocked", "back"
};

char *const ris_flags[] = {
   "fire", "cold", "earth", "energy", "blunt", "pierce", "slash", "acid",
   "poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
   "plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "air", "unholy", "holy",
   "undead", "r5", "r6", "r7", "r8", "r9", "r10"
};

char *const trig_flags[] = {
   "up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
   "d_down", "door", "container", "open", "close", "passage", "oload", "mload",
   "teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
   "rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
   "showroomdesc", "autoreturn", "r2", "r3"
};

char *const part_flags[] = {
   "head", "arms", "legs", "heart", "brains", "guts", "hands", "feet", "fingers",
   "ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
   "tail", "scales", "claws", "fangs", "horns", "tusks", "tailattack",
   "sharpscales", "beak", "haunches", "hooves", "paws", "forelegs", "feathers",
   "r1", "r2"
};

char *const attack_flags[] = {
   "bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
   "gouge", "backstab", "feed", "drain", "firebreath", "frostbreath",
   "acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
   "blindness", "causeserious", "earthquake", "causecritical", "curse",
   "flamestrike", "harm", "fireball", "colorspray", "weaken", "r1"
};

char *const defense_flags[] = {
   "parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
   "dispelmagic", "dispelevil", "sanctuary", "fireshield", "shockshield",
   "shield", "bless", "stoneskin", "teleport", "monsum1", "monsum2", "monsum3",
   "monsum4", "disarm", "iceshield", "grip", "truesight", "r4", "r5", "r6", "r7",
   "r8", "r9", "r10", "r11", "r12"
};

/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 * 
 */
char *const mprog_flags[] = {
   "act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
   "allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
   "look", "exa", "zap", "get", "drop", "damage", "repair", "randiw",
   "speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use"
};


char *flag_string(int bitvector, char *const flagarray[])
{
   static char buf[MSL];
   int x;

   buf[0] = '\0';
   for (x = 0; x < 32; x++)
      if (IS_SET(bitvector, 1 << x))
      {
         strcat(buf, flagarray[x]);
         strcat(buf, " ");
      }
   if ((x = strlen(buf)) > 0)
      buf[--x] = '\0';

   return buf;
}

char *ext_flag_string(EXT_BV * bitvector, char *const flagarray[])
{
   static char buf[MSL];
   int x;

   buf[0] = '\0';
   for (x = 0; x < MAX_BITS; x++)
      if (xIS_SET(*bitvector, x))
      {
         strcat(buf, flagarray[x]);
         strcat(buf, " ");
      }
   if ((x = strlen(buf)) > 0)
      buf[--x] = '\0';

   return buf;
}

/* Below 4 functions are local handler functions for do_build -- Xerves 12/99 */

int resource_value(int value)
{
   int rvalue = 0;

   if (value == ROOM_NO_MAGIC
      || value == ROOM_PRIVATE
      || value == ROOM_SOLITARY || value == ROOM_SILENCE || value == ROOM_NODROP || value == ROOM_NO_SUMMON || value == ROOM_NO_ASTRAL)
      rvalue = 3000;

   if (value == ROOM_NO_MOB || value == ROOM_NODROPALL)
      rvalue = 2000;

   return rvalue;
}

int resource_value2(int value)
{
   int rvalue = 0;


   if (value == EX_ISDOOR)
      rvalue = 100;
   if (value == EX_SECRET || value == EX_HIDDEN)
      rvalue = 1000;
   if (value == EX_PICKPROOF || value == EX_BASHPROOF || value == EX_NOPASSDOOR)
      rvalue = 3000;

   return rvalue;
}

bool can_build(CHAR_DATA * ch, int x, int y)
{
   int cnt;
   
   if (IS_NPC(ch))
      return FALSE;
      
   for (cnt = 1; cnt <= 150; cnt++)
   {
      if (ch->pcdata->town->roomcoords[cnt][0] == x && ch->pcdata->town->roomcoords[cnt][1] == y
      &&  ch->pcdata->town->roomcoords[cnt][2] == ch->map)
      {     
         break;
      }
   }
   if (cnt == 151)
      return FALSE;
   if (kingdom_sector[ch->map][x][y] != ch->pcdata->hometown)
      return FALSE;
   if (!in_town_range(ch->pcdata->town, x, y, ch->map))
      return FALSE;
      
   return TRUE;
}

bool can_rmodify(CHAR_DATA * ch, ROOM_INDEX_DATA * room)
{
   sh_int vnum = room->vnum;
   AREA_DATA *pArea;

   if (IS_NPC(ch))
      return FALSE;

   if (IS_ONMAP_FLAG(ch))
   {
      send_to_char("You cannot use redit from the overland maps.\n\r", ch);
      return FALSE;
   }
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

bool is_prototype(CHAR_DATA *ch, OBJ_DATA * obj, CHAR_DATA *mob)
{
   if (ch && !IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_AUTOPROTO))
      return TRUE;
   if (obj)
   {
      if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
         return TRUE;
   }
   if (mob)
   {
      if (IS_ACT_FLAG(mob, ACT_PROTOTYPE))
         return TRUE;
   }
   return FALSE;
}


bool can_omodify(CHAR_DATA * ch, OBJ_DATA * obj)
{
   sh_int vnum = obj->pIndexData->vnum;
   AREA_DATA *pArea;

   if (IS_NPC(ch))
      return FALSE;
   if (get_trust(ch) >= sysdata.level_modify_proto)
      return TRUE;
   if (!IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
   {
      send_to_char("You cannot modify this object.\n\r", ch);
      return FALSE;
   }
   if (!ch->pcdata || !(pArea = ch->pcdata->area))
   {
      send_to_char("You must have an assigned area to modify this object.\n\r", ch);
      return FALSE;
   }
   if (vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum)
      return TRUE;

   send_to_char("That object is not in your allocated range.\n\r", ch);
   return FALSE;
}

bool can_oedit(CHAR_DATA * ch, OBJ_INDEX_DATA * obj)
{
   sh_int vnum = obj->vnum;
   AREA_DATA *pArea;

   if (IS_NPC(ch))
      return FALSE;
   if (get_trust(ch) >= sysdata.level_modify_proto) /* Tracker1 */
      return TRUE;
   if (!IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
   {
      send_to_char("You cannot modify this object.\n\r", ch);
      return FALSE;
   }
   if (!ch->pcdata || !(pArea = ch->pcdata->area))
   {
      send_to_char("You must have an assigned area to modify this object.\n\r", ch);
      return FALSE;
   }
   if (vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum)
      return TRUE;

   send_to_char("That object is not in your allocated range.\n\r", ch);
   return FALSE;
}


bool can_mmodify(CHAR_DATA * ch, CHAR_DATA * mob)
{
   sh_int vnum;
   AREA_DATA *pArea;

   if (mob == ch)
      return TRUE;

   if (!IS_NPC(mob))
   {
      if (get_trust(ch) >= sysdata.level_modify_proto && get_trust(ch) > get_trust(mob))
         return TRUE;
      else
         send_to_char("You can't do that.\n\r", ch);
      return FALSE;
   }

   vnum = mob->pIndexData->vnum;

   if (IS_NPC(ch))
      return FALSE;
   if (get_trust(ch) >= sysdata.level_modify_proto)
      return TRUE;
   if (!xIS_SET(mob->act, ACT_PROTOTYPE))
   {
      send_to_char("You cannot modify this mobile.\n\r", ch);
      return FALSE;
   }
   if (!ch->pcdata || !(pArea = ch->pcdata->area))
   {
      send_to_char("You must have an assigned area to modify this mobile.\n\r", ch);
      return FALSE;
   }
   if (vnum >= pArea->low_m_vnum && vnum <= pArea->hi_m_vnum)
      return TRUE;

   send_to_char("That mobile is not in your allocated range.\n\r", ch);
   return FALSE;
}

bool can_medit(CHAR_DATA * ch, MOB_INDEX_DATA * mob)
{
   sh_int vnum = mob->vnum;
   AREA_DATA *pArea;

   if (IS_NPC(ch))
      return FALSE;
   if (get_trust(ch) >= sysdata.level_modify_proto) /* Tracker1 */
      return TRUE;
   if (!xIS_SET(mob->act, ACT_PROTOTYPE))
   {
      send_to_char("You cannot modify this mobile.\n\r", ch);
      return FALSE;
   }
   if (!ch->pcdata || !(pArea = ch->pcdata->area))
   {
      send_to_char("You must have an assigned area to modify this mobile.\n\r", ch);
      return FALSE;
   }
   if (vnum >= pArea->low_m_vnum && vnum <= pArea->hi_m_vnum)
      return TRUE;

   send_to_char("That mobile is not in your allocated range.\n\r", ch);
   return FALSE;
}

int get_otype(char *type)
{
   int x;

   for (x = 0; x < (sizeof(o_types) / sizeof(o_types[0])); x++)
      if (!str_cmp(type, o_types[x]))
         return x;
   return -1;
}

int get_aflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(a_flags) / sizeof(a_flags[0])); x++)
      if (!str_cmp(flag, a_flags[x]))
         return x;
   return -1;
}

int get_qmobflag(char *flag)
{
   int x;
   
   for (x = 0; x < (sizeof(qmob_flags) / sizeof(qmob_flags[0])); x++)
      if (!str_cmp(flag, qmob_flags[x]))
         return x;
   return -1;
}

int get_trapflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(trap_flags) / sizeof(trap_flags[0])); x++)
      if (!str_cmp(flag, trap_flags[x]))
         return x;
   return -1;
}

int get_atype(char *type)
{
   int x;

   for (x = 0; x < (sizeof(a_types) / sizeof(a_types[0])); x++)
      if (!str_cmp(type, a_types[x]))
         return x;
   return -1;
}

char *print_npc_race(int race)
{
   if (npcrace_table[race])
      return npcrace_table[race]->racename;
   else
      return "????";
}

int get_npc_race(char *type)
{
   int x;

   for (x = 0; x < MAX_NPCRACE_TABLE; x++)
   {
      if (npcrace_table[x] && !str_cmp(type, npcrace_table[x]->racename))
         return x;
   }
   return -1;
}

int get_wearloc(char *type)
{
   int x;

   for (x = 0; x < (sizeof(wear_locs) / sizeof(wear_locs[0])); x++)
      if (!str_cmp(type, wear_locs[x]))
         return x;
   return -1;
}

int get_exflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(ex_flags) / sizeof(ex_flags[0])); x++)
      if (!str_cmp(flag, ex_flags[x]))
         return x;
   return -1;
}

int get_pulltype(char *type)
{
   int x;

   if (!str_cmp(type, "none") || !str_cmp(type, "clear"))
      return 0;

   for (x = 0; x < (sizeof(ex_pmisc) / sizeof(ex_pmisc[0])); x++)
      if (!str_cmp(type, ex_pmisc[x]))
         return x;

   for (x = 0; x < (sizeof(ex_pwater) / sizeof(ex_pwater[0])); x++)
      if (!str_cmp(type, ex_pwater[x]))
         return x + PT_WATER;
   for (x = 0; x < (sizeof(ex_pair) / sizeof(ex_pair[0])); x++)
      if (!str_cmp(type, ex_pair[x]))
         return x + PT_AIR;
   for (x = 0; x < (sizeof(ex_pearth) / sizeof(ex_pearth[0])); x++)
      if (!str_cmp(type, ex_pearth[x]))
         return x + PT_EARTH;
   for (x = 0; x < (sizeof(ex_pfire) / sizeof(ex_pfire[0])); x++)
      if (!str_cmp(type, ex_pfire[x]))
         return x + PT_FIRE;
   return -1;
}

int get_rflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(r_flags) / sizeof(r_flags[0])); x++)
      if (!str_cmp(flag, r_flags[x]))
         return x;
   return -1;
}

int get_mpflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(mprog_flags) / sizeof(mprog_flags[0])); x++)
      if (!str_cmp(flag, mprog_flags[x]))
         return x;
   return -1;
}

int get_oflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(o_flags) / sizeof(o_flags[0])); x++)
      if (!str_cmp(flag, o_flags[x]))
         return x;
   return -1;
}

int get_areaflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(area_flags) / sizeof(area_flags[0])); x++)
      if (!str_cmp(flag, area_flags[x]))
         return x;
   return -1;
}

int get_wflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(w_flags) / sizeof(w_flags[0])); x++)
      if (!str_cmp(flag, w_flags[x]))
         return x;
   return -1;
}

int get_miflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(mi_flags) / sizeof(mi_flags[0])); x++)
      if (!str_cmp(flag, mi_flags[x]))
         return x;
   return -1;
}

int get_actflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(act_flags) / sizeof(act_flags[0])); x++)
      if (!str_cmp(flag, act_flags[x]))
         return x;
   return -1;
}

int get_talentflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(talent_flags) / sizeof(talent_flags[0])); x++)
      if (!str_cmp(flag, talent_flags[x]))
         return x;
   return -1;
}

int get_buykmobflag(char *type)
{
   int x;

   for (x = 0; x < (sizeof(buykmob_types) / sizeof(buykmob_types[0])); x++)
      if (!str_cmp(type, buykmob_types[x]))
         return x;
   return -1;
}

int get_buykobjflag(char *type)
{
   int x;

   for (x = 0; x < (sizeof(buykobj_types) / sizeof(buykobj_types[0])); x++)
      if (!str_cmp(type, buykobj_types[x]))
         return x;
   return -1;
}

int get_pcflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(pc_flags) / sizeof(pc_flags[0])); x++)
      if (!str_cmp(flag, pc_flags[x]))
         return x;
   return -1;
}

int get_plrflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(plr_flags) / sizeof(plr_flags[0])); x++)
      if (!str_cmp(flag, plr_flags[x]))
         return x;
   return -1;
}

int get_elementflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(element_flags) / sizeof(element_flags[0])); x++)
      if (!str_cmp(flag, element_flags[x]))
         return x;
   return -1;
}

int get_risflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(ris_flags) / sizeof(ris_flags[0])); x++)
      if (!str_cmp(flag, ris_flags[x]))
         return x;
   return -1;
}

/*
 * For use with cedit --Shaddai
 */

int get_cmdflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(cmd_flags) / sizeof(cmd_flags[0])); x++)
      if (!str_cmp(flag, cmd_flags[x]))
         return x;
   return -1;
}

int get_trigflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(trig_flags) / sizeof(trig_flags[0])); x++)
      if (!str_cmp(flag, trig_flags[x]))
         return x;
   return -1;
}

int get_partflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(part_flags) / sizeof(part_flags[0])); x++)
      if (!str_cmp(flag, part_flags[x]))
         return x;
   return -1;
}

int get_attackflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(attack_flags) / sizeof(attack_flags[0])); x++)
      if (!str_cmp(flag, attack_flags[x]))
         return x;
   return -1;
}

int get_defenseflag(char *flag)
{
   int x;

   for (x = 0; x < (sizeof(defense_flags) / sizeof(defense_flags[0])); x++)
      if (!str_cmp(flag, defense_flags[x]))
         return x;
   return -1;
}

int get_langflag(char *flag)
{
   int x;

   for (x = 0; lang_array[x] != LANG_UNKNOWN; x++)
      if (!str_cmp(flag, lang_names[x]))
         return lang_array[x];
   return LANG_UNKNOWN;
}
int get_langnum(char *flag)
{
   int x;

   for (x = 0; lang_array[x] != LANG_UNKNOWN; x++)
      if (!str_cmp(flag, lang_names[x]))
         return x;
   return -1;
}

/*
 * Remove carriage returns from a line
 */
char *strip_cr(char *str)
{
   static char newstr[MSL];
   int i, j;

   for (i = j = 0; str[i] != '\0'; i++)
      if (str[i] != '\r')
      {
         newstr[j++] = str[i];
      }
   newstr[j] = '\0';
   return newstr;
}

void goto_char(CHAR_DATA * ch, CHAR_DATA * wch, char *argument)
{
   ROOM_INDEX_DATA *location, *in_room;
   CHAR_DATA *fch, *fch_next;

   set_char_color(AT_IMMORT, ch);
   location = wch->in_room;

   if (is_ignoring(wch, ch))
   {
      send_to_char("No such location.\n\r", ch);
      return;
   }

   if (room_is_private(location) || room_is_private_wilderness(ch, location, wch->coord->x, wch->coord->y, wch->map))
   {
      if (get_trust(ch) < sysdata.level_override_private)
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Overriding private flag!\n\r", ch);
      }
   }

   if (xIS_SET(location->room_flags, ROOM_MAP) && !IS_ONMAP_FLAG(ch))
   {
      SET_ONMAP_FLAG(ch);
      ch->map = wch->map;
      ch->coord->x = wch->coord->x;
      ch->coord->y = wch->coord->y;
   }
   else if (xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      ch->map = wch->map;
      ch->coord->x = wch->coord->x;
      ch->coord->y = wch->coord->y;
   }
   else if (!xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      REMOVE_ONMAP_FLAG(ch);
      ch->map = -1;
      ch->coord->x = -1;
      ch->coord->y = -1;
   }

   in_room = ch->in_room;
   if (ch->fighting)
      stop_fighting(ch, TRUE);

   /* Modified bamfout processing by Altrag, installed by Samson 12-10-97 */

   if (!xIS_SET(ch->act, PLR_WIZINVIS))
   {
	if(ch->pcdata && ch->pcdata->bamfout[0] != '\0')
	{
		act(AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfout, TO_ROOM);
	}
	else
	{
      		act(AT_IMMORT, "$T", ch, NULL, "leaves in a swirling mist.", TO_ROOM);
   	}
   }
   ch->regoto = ch->in_room->vnum;
   ch->regoto_x = ch->coord->x;
   ch->regoto_y = ch->coord->y;
   ch->regoto_map = ch->map;
   char_from_room(ch);
   if (ch->mount)
   {
      char_from_room(ch->mount);
      char_to_room(ch->mount, location);
   }
   char_to_room(ch, location);

   /* Modified bamfin processing by Altrag, installed by Samson 12-10-97 */
   if (!xIS_SET(ch->act, PLR_WIZINVIS))
   {
     	 if(ch->pcdata && ch->pcdata->bamfin[0] != '\0')
	 {
		act(AT_IMMORT, "$T", ch, NULL, ch->pcdata->bamfin, TO_ROOM);
	 }
	 else
	 {
	 	act(AT_IMMORT, "$T", ch, NULL, "appears in a swirling mist.", TO_ROOM);
   	 }
   }
   do_look(ch, "auto");

   if (ch->in_room == in_room)
      return;
   for (fch = in_room->first_person; fch; fch = fch_next)
   {
      fch_next = fch->next_in_room;
      if (fch->master == ch && IS_IMMORTAL(fch))
      {
         act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
         do_goto(fch, argument);
      }
/* Experimental change by Gorog so imm's personal mobs follow them */
      else if (IS_NPC(fch) && fch->master == ch)
      {
         char_from_room(fch);
         char_to_room(fch, location);
      }
   }
   return;
}

void goto_obj(CHAR_DATA * ch, OBJ_DATA * obj, char *argument)
{
   ROOM_INDEX_DATA *location, *in_room;
   CHAR_DATA *fch, *fch_next;

   set_char_color(AT_IMMORT, ch);
   location = obj->in_room;

   if (room_is_private(location) || (xIS_SET(location->room_flags, ROOM_MAP) &&  room_is_private_wilderness(ch, location, obj->coord->x, obj->coord->y, obj->map)))
   {
      if (get_trust(ch) < sysdata.level_override_private)
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Overriding private flag!\n\r", ch);
      }
   }

   if (xIS_SET(location->room_flags, ROOM_MAP) && !IS_ONMAP_FLAG(ch))
   {
      SET_ONMAP_FLAG(ch);
      ch->map = obj->map;
      ch->coord->x = obj->coord->x;
      ch->coord->y = obj->coord->y;
   }
   else if (xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      ch->map = obj->map;
      ch->coord->x = obj->coord->x;
      ch->coord->y = obj->coord->y;
   }
   else if (!xIS_SET(location->room_flags, ROOM_MAP) && IS_ONMAP_FLAG(ch))
   {
      REMOVE_ONMAP_FLAG(ch);
      ch->map = -1;
      ch->coord->x = -1;
      ch->coord->y = -1;
   }

   in_room = ch->in_room;
   if (ch->fighting)
      stop_fighting(ch, TRUE);

   /* Modified bamfout processing by Altrag, installed by Samson 12-10-97 */
   if (!xIS_SET(ch->act, PLR_WIZINVIS))
      act(AT_IMMORT, "$n $T", ch, NULL, (ch->pcdata && ch->pcdata->bamfout[0] != '\0') ? ch->pcdata->bamfout : "leaves in a swirling mist.", TO_ROOM);

   ch->regoto = ch->in_room->vnum;
   ch->regoto_x = ch->coord->x;
   ch->regoto_y = ch->coord->y;
   ch->regoto_map = ch->map;
   char_from_room(ch);
   if (ch->mount)
   {
      char_from_room(ch->mount);
      char_to_room(ch->mount, location);
   }
   char_to_room(ch, location);

   /* Modified bamfin processing by Altrag, installed by Samson 12-10-97 */
   if (!xIS_SET(ch->act, PLR_WIZINVIS))
      act(AT_IMMORT, "$n $T", ch, NULL, (ch->pcdata && ch->pcdata->bamfin[0] != '\0') ? ch->pcdata->bamfin : "appears in a swirling mist.", TO_ROOM);

   do_look(ch, "auto");

   if (ch->in_room == in_room)
      return;
   for (fch = in_room->first_person; fch; fch = fch_next)
   {
      fch_next = fch->next_in_room;
      if (fch->master == ch && IS_IMMORTAL(fch))
      {
         act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
         do_goto(fch, argument);
      }
/* Experimental change by Gorog so imm's personal mobs follow them */
      else if (IS_NPC(fch) && fch->master == ch)
      {
         char_from_room(fch);
         char_to_room(fch, location);
      }
   }
   return;
}

void do_goto(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *location;
   CHAR_DATA *fch;
   CHAR_DATA *wch;
   OBJ_DATA *obj;
   CHAR_DATA *fch_next;
   ROOM_INDEX_DATA *in_room;
   AREA_DATA *pArea;
   sh_int vnum;

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Goto where?\n\r", ch);
      return;
   }
   if (IS_NPC(ch))
   {
      send_to_char("Mobs cannot use goto, they need to use mpgoto.\n\r", ch);
      return;
   }

   /* Begin Overland Map additions */
   if (!str_cmp(arg, "map"))
   {
      char arg1[MIL];
      char arg2[MIL];
      int x, y;
      int map = -1;

      argument = one_argument(argument, arg1);
      argument = one_argument(argument, arg2);

      if (arg1[0] == '\0')
      {
         send_to_char("Goto which map??\n\r", ch);
         return;
      }

      if (!str_cmp(arg1, "solan"))
         map = MAP_SOLAN;

      if (map == -1)
      {
         ch_printf(ch, "There isn't a map for '%s'.\n\r", arg1);
         return;
      }

      if (arg2[0] == '\0' && argument[0] == '\0')
      {
         enter_map(ch, 258, 250, map);
         return;
      }

      if (arg2[0] == '\0' || argument[0] == '\0')
      {
         send_to_char("Usage: goto map <mapname> <X> <Y>\n\r", ch);
         return;
      }

      x = atoi(arg2);
      y = atoi(argument);

      if (x < 1 || x > MAX_X)
      {
         ch_printf(ch, "Valid x coordinates are 1 to %d.\n\r", MAX_X);
         return;
      }

      if (y < 1 || y > MAX_Y)
      {
         ch_printf(ch, "Valid y coordinates are 1 to %d.\n\r", MAX_Y);
         return;
      }
      
      if (room_is_private_wilderness(ch, get_room_index(OVERLAND_SOLAN), x, y, map))
      {
         if (get_trust(ch) < sysdata.level_override_private)
         {
            send_to_char("That room is private right now.\n\r", ch);
            return;
         }
         else
            send_to_char("Overriding private flag!\n\r", ch);
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

      ch->regoto = ch->in_room->vnum;
      ch->regoto_x = ch->coord->x;
      ch->regoto_y = ch->coord->y;
      ch->regoto_map = ch->map;
      enter_map(ch, x, y, map);
      update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
      return;
   }

   if (!is_number(arg))
   {
      if (ch->on)
      {
         ch->on = NULL;
         ch->position = POS_STANDING;
      }
      if (ch->position != POS_STANDING)
      {
         ch->position = POS_STANDING;
      }
      if ((wch = get_char_world(ch, arg)) != NULL && wch->in_room != NULL)
      {
         goto_char(ch, wch, argument);

         update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
         return;
      }

      if ((obj = get_obj_world(ch, arg)) != NULL && obj->in_room != NULL)
      {
         goto_obj(ch, obj, argument);
         update_objects(ch, ch->coord->x, ch->coord->y, ch->map);
         return;
      }
   }
   /* End of Overland Map additions */

   if ((location = find_location(ch, arg)) == NULL)
   {
      vnum = atoi(arg);
      if (vnum < 0 || get_room_index(vnum))
      {
         send_to_char("You cannot find that...\n\r", ch);
         return;
      }
      if (get_trust(ch) < LEVEL_IMM /* Tracker1 */
         || vnum < 1 || IS_NPC(ch) || !ch->pcdata->area)
      {
         send_to_char("No such location.\n\r", ch);
         return;
      }
      if (get_trust(ch) < sysdata.level_modify_proto)
      {
         if (!ch->pcdata || !(pArea = ch->pcdata->area))
         {
            send_to_char("You must have an assigned area to create rooms.\n\r", ch);
            return;
         }
         if (vnum < pArea->low_r_vnum || vnum > pArea->hi_r_vnum)
         {
            send_to_char("That room is not within your assigned range.\n\r", ch);
            return;
         }
      }
      location = make_room(vnum);
      if (!location)
      {
         bug("Goto: make_room failed", 0);
         return;
      }
      location->area = ch->pcdata->area;
      set_char_color(AT_WHITE, ch);
      send_to_char("Waving your hand, you form order from swirling chaos,\n\rand step into a new reality...\n\r", ch);
   }

   if (room_is_private(location))
   {
      if (get_trust(ch) < sysdata.level_override_private)
      {
         send_to_char("That room is private right now.\n\r", ch);
         return;
      }
      else
         send_to_char("Overriding private flag!\n\r", ch);
   }

   if (!IS_NPC(ch))
   {
      if (xIS_SET(location->room_flags, ROOM_IMP) && ch->pcdata->caste < caste_Staff)
      {
         send_to_char("Nice Try, for Staff only\n\r", ch);
         return;
      }
   }
   else
   {
      if (xIS_SET(location->room_flags, ROOM_IMP))
         return;
   }
   if (!IS_NPC(ch))
   {
      if (xIS_SET(ch->act, PLR_GAMBLER))
      {
         send_to_char("I don't think leaving while gambling is a good idea!\n\r", ch);
         return;
      }
   }
   in_room = ch->in_room;
   if (ch->fighting)
      stop_fighting(ch, TRUE);

   if (!xIS_SET(ch->act, PLR_WIZINVIS))
      act(AT_IMMORT, "$n $T", ch, NULL, (ch->pcdata && ch->pcdata->bamfout[0] != '\0') ? ch->pcdata->bamfout : "leaves in a swirling mist.", TO_ROOM);

   ch->regoto = ch->in_room->vnum;
   ch->regoto_x = ch->coord->x;
   ch->regoto_y = ch->coord->y;
   ch->regoto_map = ch->map;
   char_from_room(ch);
   if (ch->mount)
   {
      char_from_room(ch->mount);
      char_to_room(ch->mount, location);
   }
   char_to_room(ch, location);
   if (IS_ONMAP_FLAG(ch))
   {
      REMOVE_ONMAP_FLAG(ch);
      REMOVE_PLR_FLAG(ch, PLR_MAPEDIT); /* Just in case they were editing */

      ch->coord->x = -1;
      ch->coord->y = -1;
      ch->map = -1;
      update_objects(ch, -1, -1, -1);
   }
   if (ch->mount)
   {
      char_from_room(ch->mount);
      char_to_room(ch->mount, ch->in_room);
      ch->mount->coord->x = ch->coord->x;
      ch->mount->coord->y = ch->coord->y;
      ch->mount->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(ch->mount);
      else
         SET_ONMAP_FLAG(ch->mount);
   }  
   if (!IS_NPC(ch) && ch->pcdata->pet)
   {
      char_from_room(ch->pcdata->pet);
      char_to_room(ch->pcdata->pet, ch->in_room);
      ch->pcdata->pet->coord->x = ch->coord->x;
      ch->pcdata->pet->coord->y = ch->coord->y;
      ch->pcdata->pet->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(ch->pcdata->pet);
      else
         SET_ONMAP_FLAG(ch->pcdata->pet);
   }  
   if (!IS_NPC(ch) && ch->pcdata->mount && !ch->mount)
   {
      char_from_room(ch->pcdata->mount);
      char_to_room(ch->pcdata->mount, ch->in_room);
      ch->pcdata->mount->coord->x = ch->coord->x;
      ch->pcdata->mount->coord->y = ch->coord->y;
      ch->pcdata->mount->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(ch->pcdata->mount);
      else
         SET_ONMAP_FLAG(ch->pcdata->mount);
   }  
   if (ch->rider)
   {
      char_from_room(ch->rider);
      char_to_room(ch->rider, ch->in_room);
      ch->rider->coord->x = ch->coord->x;
      ch->rider->coord->y = ch->coord->y;
      ch->rider->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(ch->rider);
      else
         SET_ONMAP_FLAG(ch->rider);
      update_objects(ch->rider, ch->rider->map, ch->rider->coord->x, ch->rider->coord->y);
   }  
   if (ch->riding)
   {
      char_from_room(ch->riding);
      char_to_room(ch->riding, ch->in_room);
      ch->riding->coord->x = ch->coord->x;
      ch->riding->coord->y = ch->coord->y;
      ch->riding->map = ch->map;
      if (ch->coord->x < 1)
         REMOVE_ONMAP_FLAG(ch->riding);
      else
         SET_ONMAP_FLAG(ch->riding);
      update_objects(ch->riding, ch->riding->map, ch->riding->coord->x, ch->riding->coord->y);
   }  
   if (ch->on)
   {
      ch->on = NULL;
      ch->position = POS_STANDING;
   }
   if (ch->position != POS_STANDING && ch->position != POS_RIDING)
   {
      ch->position = POS_STANDING;
   }

   if (!xIS_SET(ch->act, PLR_WIZINVIS))
      act(AT_IMMORT, "$n $T", ch, NULL, (ch->pcdata && ch->pcdata->bamfin[0] != '\0') ? ch->pcdata->bamfin : "appears in a swirling mist.", TO_ROOM);

   do_look(ch, "auto");

   if (ch->in_room == in_room)
      return;
   for (fch = in_room->first_person; fch; fch = fch_next)
   {
      fch_next = fch->next_in_room;
      if (fch->master == ch && IS_IMMORTAL(fch))
      {
         act(AT_ACTION, "You follow $N.", fch, NULL, ch, TO_CHAR);
         do_goto(fch, argument);
      }
/* Experimental change by Gorog so imm's personal mobs follow them */
      else if (IS_NPC(fch) && fch->master == ch)
      {
         char_from_room(fch);
         char_to_room(fch, location);
      }
   }
   return;
}

//Destory a trainer :=)
void do_tdelete(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TRAINER_DATA *tdata;
   char buf[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobiles.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: tdelete <mobile name>\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Only works on NPCs.\n\r", ch);
      return;
   }
   //Find the trainer and remove it, else tell player
   for (tdata = first_trainer; tdata; tdata = tdata->next)
   {
      if (tdata->vnum == victim->pIndexData->vnum)
      {
         xREMOVE_BIT(victim->act, ACT_TRAINER);
         xREMOVE_BIT(victim->act, ACT_CASTEMOB);
         xREMOVE_BIT(victim->act, ACT_PACIFIST);
         xREMOVE_BIT(victim->act, ACT_SENTINEL);
         xREMOVE_BIT(victim->pIndexData->act, ACT_TRAINER);
         xREMOVE_BIT(victim->pIndexData->act, ACT_CASTEMOB);
         xREMOVE_BIT(victim->pIndexData->act, ACT_PACIFIST);
         xREMOVE_BIT(victim->pIndexData->act, ACT_SENTINEL);
         UNLINK(tdata, first_trainer, last_trainer, next, prev);
         DISPOSE(tdata);
         fold_area(victim->in_room->area, victim->in_room->area->filename, FALSE, 1);
         sprintf(buf, "%s has removed %s as a trainer.", ch->name, victim->name);
         log_string_plus(buf, LOG_BUILD, ch->level);
         save_trainer_data();
         return;
      }
   }
   send_to_char("The Mob is no longer a trainer, remember pacifist and sentinel removed.\n\r", ch);
   return;
}


//Create a trainer setting for the poor mobile
void do_tcreate(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TRAINER_DATA *tdata;
   char buf[MSL];
   int x;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobiles.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: tcreate <mobile name>\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Only works on NPCs.\n\r", ch);
      return;
   }
   //Make sure the mobile isn't already a trainer
   for (tdata = first_trainer; tdata; tdata = tdata->next)
   {
      if (tdata->vnum == victim->pIndexData->vnum)
      {
         send_to_char("This mobile is already a trainer, make sure it has a TRAINER flag.\n\r", ch);
         return;
      }
   }
   //It is clear now to add the mobile
   CREATE(tdata, TRAINER_DATA, 1);
   for (x = 0; x < 20; x++)
   {
      tdata->sn[x] = 0;
      tdata->mastery[x] = 0;
   }
   tdata->vnum = victim->pIndexData->vnum;
   xSET_BIT(victim->act, ACT_TRAINER);
   xSET_BIT(victim->act, ACT_CASTEMOB);
   xSET_BIT(victim->act, ACT_SENTINEL);
   xSET_BIT(victim->act, ACT_PACIFIST);
   xSET_BIT(victim->pIndexData->act, ACT_TRAINER);
   xSET_BIT(victim->pIndexData->act, ACT_CASTEMOB);
   xSET_BIT(victim->pIndexData->act, ACT_SENTINEL);
   xSET_BIT(victim->pIndexData->act, ACT_PACIFIST);
   LINK(tdata, first_trainer, last_trainer, next, prev);
   fold_area(victim->in_room->area, victim->in_room->area->filename, FALSE, 1);
   sprintf(buf, "%s has added %s as a trainer.", ch->name, victim->name);
   log_string_plus(buf, LOG_BUILD, ch->level);
   save_trainer_data();
   send_to_char("Done.\n\r", ch);
   return;
}

//View the trainers stats
void do_tstat(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TRAINER_DATA *tdata;
   BUYKTRAINER_DATA *ktrainer =NULL;
   int x;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobiles.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: tstat <mobile name>\n\r", ch);
      return;
   }
   if ((victim = get_char_room_new(ch, argument, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Only works on NPCs.\n\r", ch);
      return;
   }
   //Find the trainer
   for (tdata = first_trainer; tdata; tdata = tdata->next)
   {
      if (tdata->vnum == victim->pIndexData->vnum)
      {
         break; //Trainer found, now break;
      }
   }
   if (!tdata)
   {
      for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
      {
         if (victim->pIndexData->vnum == MOB_VNUM_TRAINER && victim->m2 == ktrainer->pid)
            break;
      }
   }
   if (tdata == NULL && ktrainer == NULL)
   {
      send_to_char("That mobile is not a trainer, to make the mobile a trainer, use tcreate.\n\r", ch);
      return;
   }
   for (x = 0; x < 20; x++)
   {
      if (tdata)
      {
         if (tdata->sn[x] != 0 && tdata->mastery[x] > 0)
         {
            if (!IS_VALID_SN(tdata->sn[x]))
            {
               bug("%s has an invalid sn at slot %d", victim->name, x + 1);
               ch_printf(ch, "&G&WSlot %d:  &c&wERROR: Has an invalid Sn.\n\r", x + 1);
               continue;
            }
            ch_printf(ch, "&G&WSlot %d:  &c&wMastery: &C%-8s   &c&wSkill: &c%s\n\r", x + 1,
               get_mastery_name(tdata->mastery[x]), skill_table[tdata->sn[x]]->name);
         }
      }
      else
      {
         if (ktrainer->sn[x] != 0 && ktrainer->mastery[x] > 0)
         {
            if (!IS_VALID_SN(ktrainer->sn[x]))
            {
               bug("%s has an invalid sn at slot %d", victim->name, x + 1);
               ch_printf(ch, "&G&WSlot %d:  &c&wERROR: Has an invalid Sn.\n\r", x + 1);
               continue;
            }
            ch_printf(ch, "&G&WSlot %d:  &c&wMastery: &C%-8s   &c&wSkill: &c%s\n\r", x + 1,
               get_mastery_name(ktrainer->mastery[x]), skill_table[ktrainer->sn[x]]->name);
         }
      }
   }
}

//Set the trainer values
void do_tset(CHAR_DATA * ch, char *argument)
{
   CHAR_DATA *victim;
   TRAINER_DATA *tdata;
   BUYKTRAINER_DATA *ktrainer = NULL;
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   char masterybuf1[MSL];
   char masterybuf2[MSL];
   char snbuf1[MSL];
   char snbuf2[MSL];
   int cnt;
   int sn;

   if (IS_NPC(ch))
   {
      send_to_char("This command is not for mobiles.\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: tset <mobile> <field> <value>\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("Sn1-20, Mastery1-20\n\r", ch);
      return;
   }
   sprintf(buf, "tset: %s  tset %s", ch->name, argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
   {
      send_to_char("They aren't here.\n\r", ch);
      return;
   }
   if (!IS_NPC(victim))
   {
      send_to_char("Only works on NPCs.\n\r", ch);
      return;
   }
   //Make sure the mobile isn't already a trainer
   for (tdata = first_trainer; tdata; tdata = tdata->next)
   {
      if (tdata->vnum == victim->pIndexData->vnum)
      {
         break; //Found it, so break
      }
   }
   if (!tdata)
   {
      for (ktrainer = first_buyktrainer; ktrainer; ktrainer = ktrainer->next)
      {
         if (victim->pIndexData->vnum == MOB_VNUM_TRAINER && victim->m2 == ktrainer->pid)
            break;
      }
   }
   if (tdata == NULL && ktrainer == NULL)
   {
      send_to_char("That mobile is not a trainer, to make the mobile a trainer, use tcreate.\n\r", ch);
      return;
   }
   log_string_plus(buf, LOG_BUILD, ch->level);
   //A simple loop which creates checks for sn1-20, etc without wasting MANY lines
   for (cnt = 0; cnt < 20; cnt++)
   {
      sprintf(snbuf1, "sn%d", cnt + 1);
      sprintf(snbuf2, "Sn%d", cnt + 1);
      sprintf(masterybuf1, "mastery%d", cnt + 1);
      sprintf(masterybuf2, "Mastery%d", cnt + 1);

      if (!str_cmp(arg2, snbuf1) || !str_cmp(arg2, snbuf2))
      {
         if (isdigit(argument[0]))
         {
            if (!IS_VALID_SN(atoi(argument)) && (atoi(argument) != 0))
            {
               send_to_char("The sn you provided is invalid.\n\r", ch);
               return;
            }
            if (tdata)
               tdata->sn[cnt] = atoi(argument);
            else
               ktrainer->sn[cnt] = atoi(argument);
         }
         else
         {
            sn = skill_lookup(argument);
            if (sn == -1)
            {
               send_to_char("Invalid Skill or Spell.\n\r", ch);
               return;
            }
            if (tdata)
               tdata->sn[cnt] = sn;
            else
               ktrainer->sn[cnt] = sn;
         }
         send_to_char("Done.\n\r", ch);
         if (tdata)
            save_trainer_data();
         else
            save_buykingdom_data();
         return;
      }
      if (!str_cmp(arg2, masterybuf1) || !str_cmp(arg2, masterybuf2))
      {
         if (isdigit(argument[0]))
         {
            if (atoi(argument) < 0 || atoi(argument) > MAX_RANKING)
            {
               ch_printf(ch, "The range for mastery is 0 to %d", MAX_RANKING);
               return;
            }
            if (tdata)
               tdata->mastery[cnt] = atoi(argument);
            else
               ktrainer->mastery[cnt] = atoi(argument);
         }
         else
         {
            sn = get_mastery_num(argument);
            if (sn == -1)
            {
               send_to_char("That is not a valid Mastery.\n\r", ch);
               return;
            }
            if (tdata)
               tdata->mastery[cnt] = sn;
            else
               ktrainer->sn[cnt] = sn;
         }
         send_to_char("Done.\n\r", ch);
         if (tdata)
            save_trainer_data();
         else
            save_buykingdom_data();
         return;
      }
   }
   do_tset(ch, "");
   return;
}

void do_addgem(CHAR_DATA * ch, char *argument)
{
   GEM_DATA *newgem;

   //char buf[MSL];
   char vnumarg[MIL];
   char costarg[MIL];
   char rarityarg[MIL];


   if (argument[0] == '\0')
   {
      send_to_char("Syntax: addgem <vnum> <cost> <rarity>\n\r", ch);
      send_to_char("		rarity: 1 -- common, 2 -- uncommon, 3 -- rare\n\r", ch);
      return;
   }

   argument = one_argument(argument, vnumarg);
   if (vnumarg[0] == '\0')
   {
      send_to_char("What vnum?\n\r", ch);
      return;
   }
   argument = one_argument(argument, costarg);
   if (costarg[0] == '\0')
   {
      send_to_char("It needs a cost!!\n\r", ch);
      return;
   }
   argument = one_argument(argument, rarityarg);
   if (rarityarg[0] == '\0')
   {
      send_to_char("Set a rarity.\n\r", ch);
      return;
   }

   //Check to make sure a numerical value is present
   if (!isdigit(vnumarg[0]))
   {
      send_to_char("You need to provide a vnum for the gem, not a name.\n\r", ch);
      return;
   }
   if (!isdigit(costarg[0]))
   {
      send_to_char("You need to provide a numerical value for the cost.\n\r", ch);
      return;
   }
   if (!isdigit(rarityarg[0]))
   {
      send_to_char("You need to provide a numerical value for the rarity (1-3) \n\r", ch);
      return;
   }
   //Make sure the object isn't already a gem
   for (newgem = first_gem; newgem; newgem = newgem->next)
   {
      if (newgem->vnum == atoi(vnumarg))
      {
         send_to_char("This object is already a gem.\n\r", ch);
         return;
      }
   }
   if ((get_obj_index(atoi(vnumarg))) == NULL)
   {
      send_to_char("This gem does not actually exist.\n\r", ch);
      return;
   }
   //It is clear now to add the object
   CREATE(newgem, GEM_DATA, 1);

   newgem->vnum = atoi(vnumarg);
   newgem->cost = atoi(costarg);
   newgem->rarity = atoi(rarityarg);
   gem_num += 1;

   LINK(newgem, first_gem, last_gem, next, prev);
   //sprintf(buf, "%s has added %s as a gem.", ch->name, newgem->vnum);
   //log_string_plus(buf, LOG_BUILD, ch->level);
   save_gem_data();
   send_to_char("Done.\n\r", ch);
   return;
}
void do_addbox(CHAR_DATA * ch, char *argument)
{
   BOX_DATA *newbox;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: addbox <vnum>\n\r", ch);
      return;
   }

   //Check to make sure a numerical value is present
   if (!isdigit(argument[0]))
   {
      send_to_char("You need to provide a vnum for the gem, not a name.\n\r", ch);
      return;
   }

   //Make sure the object isn't already a box
   for (newbox = first_box; newbox; newbox = newbox->next)
   {
      if (newbox->vnum == atoi(argument))
      {
         send_to_char("This object is already a box.\n\r", ch);
         return;
      }
   }

   if ((get_obj_index(atoi(argument))) == NULL)
   {
      send_to_char("This box does not actually exist.\n\r", ch);
      return;
   }

   //It is clear now to add the object
   CREATE(newbox, BOX_DATA, 1);

   newbox->vnum = atoi(argument);
   box_num += 1;

   LINK(newbox, first_box, last_box, next, prev);
   //sprintf(buf, "%s has added %s as a treasure box.", ch->name, newbox->vnum);
   //log_string_plus(buf, LOG_BUILD, ch->level);
   save_box_data();
   send_to_char("Done.\n\r", ch);
   return;
}

void do_mset(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];
   char outbuf[MSL];
   int num, size, plus;
   int v2;
   char char1, char2;
   CHAR_DATA *victim;
   int value;
   int minattr, maxattr;
   bool lockvictim;
   char *origarg = argument;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Mob's can't mset\n\r", ch);
      return;
   }

   if (!ch->desc)
   {
      send_to_char("You have no descriptor\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;
      case SUB_MOB_DESC:
         if (!ch->dest_buf)
         {
            send_to_char("Fatal error: report to Thoric.\n\r", ch);
            bug("do_mset: sub_mob_desc: NULL ch->dest_buf", 0);
            ch->substate = SUB_NONE;
            return;
         }
         victim = ch->dest_buf;
         if (char_died(victim))
         {
            send_to_char("Your victim died!\n\r", ch);
            stop_editing(ch);
            return;
         }
         STRFREE(victim->description);
         victim->description = copy_buffer(ch);
         if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         {
            STRFREE(victim->pIndexData->description);
            victim->pIndexData->description = QUICKLINK(victim->description);
         }
         stop_editing(ch);
         ch->substate = ch->tempnum;
         return;
   }

   victim = NULL;
   lockvictim = FALSE;
   smash_tilde(argument);

   if (ch->substate == SUB_REPEATCMD)
   {
      victim = ch->dest_buf;

      if (char_died(victim))
      {
         send_to_char("Your victim died!\n\r", ch);
         victim = NULL;
         argument = "done";
      }
      if (argument[0] == '\0' || !str_cmp(argument, " ") || !str_cmp(argument, "stat"))
      {
         if (victim)
            do_mstat(ch, victim->name);
         else
            send_to_char("No victim selected.  Type '?' for help.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "done") || !str_cmp(argument, "off"))
      {
         send_to_char("Mset mode off.\n\r", ch);
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
         if (ch->pcdata && ch->pcdata->subprompt)
         {
            STRFREE(ch->pcdata->subprompt);
            ch->pcdata->subprompt = NULL;
            if (xIS_SET(ch->act, PLR_MSET)) /* mset flags, Crash no like -- Xerves */
               xREMOVE_BIT(ch->act, PLR_MSET);
         }
         return;
      }
   }
   if (victim)
   {
      lockvictim = TRUE;
      strcpy(arg1, victim->name);
      argument = one_argument(argument, arg2);
      strcpy(arg3, argument);
   }
   else
   {
      lockvictim = FALSE;
      argument = one_argument(argument, arg1);
      argument = one_argument(argument, arg2);
      strcpy(arg3, argument);
   }

   if (!str_cmp(arg1, "on"))
   {
      send_to_char("Syntax: mset <victim|vnum> on.\n\r", ch);
      return;
   }

   if (arg1[0] == '\0' || (arg2[0] == '\0' && ch->substate != SUB_REPEATCMD) || !str_cmp(arg1, "?"))
   {
      if (ch->substate == SUB_REPEATCMD)
      {
         if (victim)
            send_to_char("Syntax: <field>  <value>\n\r", ch);
         else
            send_to_char("Syntax: <victim> <field>  <value>\n\r", ch);
      }
      else
         send_to_char("Syntax: mset <victim> <field>  <value>\n\r", ch);
      send_to_char("Syntax: mset <victim> autostat <level>\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Field being one of:\n\r", ch);
      send_to_char("  str int wis dex con lck agi endurance sex\n\r", ch);
      send_to_char("  gold hp mana move practice align race\n\r", ch);
      send_to_char("  strper, intper, wisper, conper, dexper, lckper\n\r", ch);
      send_to_char("  agiper, hpper, manaper, moveper\n\r", ch);
      send_to_char("  hitroll damroll armor affected\n\r", ch);
      send_to_char("  rarmper larmper llegper rlegper\n\r", ch);
      send_to_char("  tohitstab tohitbash tohitslash\n\r", ch);
      send_to_char("  thirst drunk full blood flags talent\n\r", ch);
      send_to_char("  pos defpos part (see BODYPARTS)\n\r", ch);
      send_to_char("  sav1 sav2 sav4 sav4 sav5 (see SAVINGTHROWS)\n\r", ch);
      send_to_char("  m1 m2 m3 m4 m5 m6 m7 m8 m9 m10 m11 m12 (see CASTESLOTS)\n\r", ch);
      send_to_char("  rfire rwater rair rearth renergy rmagic rnonmagic rblunt\n\r", ch);
      send_to_char("  rpierce rslash rpoison rpara rholy runholy rundead (see NEWRESISTS)\n\r", ch);
      send_to_char("  resistant immune susceptible element(see RIS)\n\r", ch);
      send_to_char("  skincolor haircolor hairlength hairstyle eyecolor cheight cweight\n\r", ch);
      send_to_char("  attack defense\n\r", ch);
      if (sysdata.resetgame)
         send_to_char("  twinkpoints\n\r", ch);
      send_to_char("  speaking speaks (see LANGUAGES)\n\r", ch);
      send_to_char("  name short long description title spec clan\n\r", ch);
      send_to_char("  council favor deity cident fame spoints\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("For editing index/prototype mobiles:\n\r", ch);
      send_to_char("  hitnumdie hitsizedie hitplus (hit points)\n\r", ch);
      send_to_char("  damnumdie damsizedie damplus (damage roll)\n\r", ch);
      send_to_char("  damaddlow damaddhi\n\r", ch);
      send_to_char("  Map resets:  coordx, coordy, onmap\n\r", ch);
      send_to_char("To toggle area flag: aloaded\n\r", ch);
      send_to_char("To toggle pkill flag: pkill\n\r", ch);
      return;
   }

   if (!victim && get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
   {
      if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }
   }
   else if (!victim)
   {
      if ((victim = get_char_world(ch, arg1)) == NULL)
      {
         send_to_char("No one like that in all the realms.\n\r", ch);
         return;
      }
   }

   if (get_trust(ch) < get_trust(victim) && !IS_NPC(victim))
   {
      send_to_char("You can't do that!\n\r", ch);
      ch->dest_buf = NULL;
      return;
   }
   /* Pondering where this went.....Will protect lower imms from setting players */
   if (!IS_NPC(victim) && get_trust(ch) < sysdata.level_mset_player)
   {
      send_to_char("Xerves deems you unready to fool in the ways of mortals.\n\r", ch);
      ch->dest_buf = NULL;
      return;
   }
   if (IS_NPC(victim) && (xIS_SET(victim->act, ACT_PROTECT)) && (get_trust(ch) < LEVEL_STAFF))
   {
      send_to_char("This mob is protected.  If you need to change it, ask Staff.\n\r", ch);
      ch->dest_buf = NULL;
      return;
   }
   if (lockvictim)
      ch->dest_buf = victim;

   minattr = 6;
   maxattr = 25;

   if (!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_AUTOPROTO))
   {
      send_to_char("WARNING:  Making Changes in AUTOPROTO MODE!\n\r", ch);
   }
   if (!str_cmp(arg2, "on"))
   {
      if ((xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_MSET)))
      {
         send_to_char("Trying to turn on, while already on another, BLAH!\n\r", ch);
         return;
      }
      CHECK_SUBRESTRICTED(ch);
      ch_printf(ch, "Mset mode on. (Editing %s).\n\r", victim->name);
      ch->substate = SUB_REPEATCMD;
      ch->dest_buf = victim;
      if (ch->pcdata)
      {
         if (ch->pcdata->subprompt)
            STRFREE(ch->pcdata->subprompt);
         if (IS_NPC(victim))
            sprintf(buf, "<&CMset &W#%d&w> %%i", victim->pIndexData->vnum);
         else
            sprintf(buf, "<&CMset &W%s&w> %%i", victim->name);
         ch->pcdata->subprompt = STRALLOC(buf);
         if (!xIS_SET(ch->act, PLR_MSET)) /* Mset flags, Crash no like -- Xerves */
            xSET_BIT(ch->act, PLR_MSET);
      }
      return;
   }
   value = is_number(arg3) ? atoi(arg3) : -1;

   if (atoi(arg3) < -1 && value == -1)
      value = atoi(arg3);

   if (!str_cmp(arg2, "autostat"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 130)
      {
         send_to_char("Range is 1 - 130 (help mobchart)\n\r", ch);
         return;
      }
      adjust_wildermob(victim, value, 0);
      send_to_char("Done.\n\r", ch);
      return;
   }  
   if (!str_cmp(arg2, "str"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < minattr || value > maxattr)
      {
         ch_printf(ch, "Strength range is %d to %d.\n\r", minattr, maxattr);
         return;
      }
      victim->perm_str = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_str = value;
      return;
   }

   if (!str_cmp(arg2, "int"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < minattr || value > maxattr)
      {
         ch_printf(ch, "Intelligence range is %d to %d.\n\r", minattr, maxattr);
         return;
      }
      victim->perm_int = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_int = value;
      return;
   }
   
   if (!str_cmp(arg2, "afffix"))
   {
      if (!can_mmodify(ch, victim))
         return;
      victim->apply_armor = victim->apply_shield = victim->apply_stone = victim->apply_sanctify = 0;
      victim->apply_tohit = victim->managen = victim->hpgen = victim->apply_wmod = 0;
      return;
   }   
   
   if (!str_cmp(arg2, "endurance"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         if (value < 10 || value > 104)
         {
            send_to_char("The values for Player Endurance are 10 to 104\n\r", ch);
            return;
         }
      }
      else
      {
         if (value < 10 || value > 135)
         {
            send_to_char("The values for Mobile Endurance ars 10 to 135\n\r", ch);
            return;
         }
      }
      victim->mover = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->mover = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg2, "twinkpoints") && sysdata.resetgame)
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
         return;
      if (value < 0 || value > 100)
      {
         send_to_char("The value ranges from 0 to 100.\n\r", ch);
         return;
      }
      victim->pcdata->twink_points = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 

   if (!str_cmp(arg2, "rarmper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000)
      {
         send_to_char("The value ranges from -1 to 1000.\n\r", ch);
         return;
      }
      victim->con_rarm = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "larmper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000)
      {
         send_to_char("The value ranges from -1 to 1000.\n\r", ch);
         return;
      }
      victim->con_larm = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "llegper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000)
      {
         send_to_char("The value ranges from -1 to 1000.\n\r", ch);
         return;
      }
      victim->con_lleg = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "rlegper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000)
      {
         send_to_char("The value ranges from -1 to 1000.\n\r", ch);
         return;
      }
      victim->con_rleg = value;
      send_to_char("Done.\n\r", ch);
      return;
   }  
   
   if (!str_cmp(arg2, "strper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 9999)
      {
         send_to_char("The value ranges from 0 to 9999.\n\r", ch);
         return;
      }
      victim->pcdata->per_str = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "intper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 9999)
      {
         send_to_char("The value ranges from 0 to 9999.\n\r", ch);
         return;
      }
      victim->pcdata->per_int = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "wisper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 9999)
      {
         send_to_char("The value ranges from 0 to 9999.\n\r", ch);
         return;
      }
      victim->pcdata->per_wis = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "dexper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 9999)
      {
         send_to_char("The value ranges from 0 to 9999.\n\r", ch);
         return;
      }
      victim->pcdata->per_dex = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "conper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 9999)
      {
         send_to_char("The value ranges from 0 to 9999.\n\r", ch);
         return;
      }
      victim->pcdata->per_con = value;
      send_to_char("Done.\n\r", ch);
      return;
   }  
   if (!str_cmp(arg2, "lckper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 9999)
      {
         send_to_char("The value ranges from 0 to 9999.\n\r", ch);
         return;
      }
      victim->pcdata->per_lck = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "agiper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 999)
      {
         send_to_char("The value ranges from 0 to 999.\n\r", ch);
         return;
      }
      victim->pcdata->per_agi = value;
      send_to_char("Done.\n\r", ch);
      return;
   } 
   if (!str_cmp(arg2, "hpper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 999)
      {
         send_to_char("The value ranges from 0 to 999.\n\r", ch);
         return;
      }
      victim->pcdata->per_hp = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "manaper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 999)
      {
         send_to_char("The value ranges from 0 to 999.\n\r", ch);
         return;
      }
      victim->pcdata->per_mana = value;
      send_to_char("Done.\n\r", ch);
      return;
   }   
   if (!str_cmp(arg2, "moveper"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 999)
      {
         send_to_char("The value ranges from 0 to 999.\n\r", ch);
         return;
      }
      victim->pcdata->per_move = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg2, "spoints"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (IS_NPC(victim))
      {
         send_to_char("On Pcs only.\n\r", ch);
         return;
      }
      if (value < 0 || value > 99999999)
      {
         send_to_char("The value ranges from 0 to 99999999.\n\r", ch);
         return;
      }
      victim->pcdata->spoints = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "rair"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_air[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_air = value;
      return;
   }
   if (!str_cmp(arg2, "rearth"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_earth[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_earth = value;
      return;
   }
   if (!str_cmp(arg2, "renergy"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_energy[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_energy = value;
      return;
   }
   if (!str_cmp(arg2, "rmagic"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_magic[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_magic = value;
      return;
   }
   if (!str_cmp(arg2, "rnonmagic"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_nonmagic[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_nonmagic = value;
      return;
   }
   if (!str_cmp(arg2, "rblunt"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_blunt[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_blunt = value;
      return;
   }
   if (!str_cmp(arg2, "rpierce"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_pierce[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_pierce = value;
      return;
   }
   if (!str_cmp(arg2, "rslash"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_slash[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_slash = value;
      return;
   }
   if (!str_cmp(arg2, "rpoison"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_poison[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_poison = value;
      return;
   }
   if (!str_cmp(arg2, "rpara"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_paralysis[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_paralysis = value;
      return;
   }
   if (!str_cmp(arg2, "rholy"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_holy[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_holy = value;
      return;
   }
   if (!str_cmp(arg2, "runholy"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_unholy[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_unholy = value;
      return;
   }
   if (!str_cmp(arg2, "rundead"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_undead[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_undead = value;
      return;
   }
   if (!str_cmp(arg2, "rwater"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_water[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_water = value;
      return;
   }    
   if (!str_cmp(arg2, "rfire"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1 || value > 1000 || value == 0)
      {
         ch_printf(ch, "Range is -1 to 1000.  0 is invalid (equivalant to 100).  See HELP NEWRESETS\n\r");
         return;
      }
      if (value == -1)
         value = -500;
      victim->apply_res_fire[0] = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->apply_res_fire = value;
      return;
   }
   if (!str_cmp(arg2, "tohitstab"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_TOHITAC)
      {
         ch_printf(ch, "ToHitStab range is 0 to %d.\n\r", MAX_TOHITAC);
         return;
      }
      if (!IS_NPC(victim))
      {
         send_to_char("Only on npcs.\n\r", ch);
         return;
      }
      victim->tohitstab = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->tohitstab = value;
      return;
   }
   if (!str_cmp(arg2, "tohitslash"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_TOHITAC)
      {
         ch_printf(ch, "ToHitSlash range is 0 to %d.\n\r", MAX_TOHITAC);
         return;
      }
      if (!IS_NPC(victim))
      {
         send_to_char("Only on npcs.\n\r", ch);
         return;
      }
      victim->tohitslash = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->tohitslash = value;
      return;
   }
   if (!str_cmp(arg2, "tohitbash"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_TOHITAC)
      {
         ch_printf(ch, "ToHitBash range is 0 to %d.\n\r", MAX_TOHITAC);
         return;
      }
      if (!IS_NPC(victim))
      {
         send_to_char("Only on npcs.\n\r", ch);
         return;
      }
      victim->tohitbash = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->tohitbash = value;
      return;
   }
   
   if (!str_cmp(arg2, "armor"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_TOHITAC)
      {
         ch_printf(ch, "Armor range is 0 to %d.\n\r", MAX_TOHITAC);
         return;
      }
      if (!IS_NPC(victim))
      {
         send_to_char("Only on npcs.\n\r", ch);
         return;
      }
      victim->armor = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->ac = value;
      return;
   }

   if (!str_cmp(arg2, "wis"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < minattr || value > maxattr)
      {
         ch_printf(ch, "Wisdom range is %d to %d.\n\r", minattr, maxattr);
         return;
      }
      victim->perm_wis = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_wis = value;
      return;
   }

   if (!str_cmp(arg2, "dex"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < minattr || value > maxattr)
      {
         ch_printf(ch, "Dexterity range is %d to %d.\n\r", minattr, maxattr);
         return;
      }
      victim->perm_dex = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_dex = value;
      return;
   }

   if (!str_cmp(arg2, "con"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < minattr || value > maxattr)
      {
         ch_printf(ch, "Constitution range is %d to %d.\n\r", minattr, maxattr);
         return;
      }
      victim->perm_con = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_con = value;
      return;
   }

   if (!str_cmp(arg2, "cha"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < minattr || value > maxattr)
      {
         ch_printf(ch, "Charisma range is %d to %d.\n\r", minattr, maxattr);
         return;
      }
      victim->perm_cha = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_cha = value;
      return;
   }

   if (!str_cmp(arg2, "lck"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < minattr || value > maxattr)
      {
         ch_printf(ch, "Luck range is %d to %d.\n\r", minattr, maxattr);
         return;
      }
      victim->perm_lck = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_lck = value;
      return;
   }

   if (!str_cmp(arg2, "agi"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 10 || value > 100)
      {
         send_to_char("Agility Range is 10 to 100.\n\r", ch);
         return;
      }
      victim->perm_agi = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->perm_agi = value;
      return;
   }

   if (!str_cmp(arg2, "sav1"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -30 || value > 30)
      {
         send_to_char("Saving throw range is -30 to 30.\n\r", ch);
         return;
      }
      victim->saving_poison_death = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->saving_poison_death = value;
      return;
   }

   if (!str_cmp(arg2, "sav2"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -30 || value > 30)
      {
         send_to_char("Saving throw range is -30 to 30.\n\r", ch);
         return;
      }
      victim->saving_wand = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->saving_wand = value;
      return;
   }

   if (!str_cmp(arg2, "sav3"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -30 || value > 30)
      {
         send_to_char("Saving throw range is -30 to 30.\n\r", ch);
         return;
      }
      victim->saving_para_petri = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->saving_para_petri = value;
      return;
   }

   if (!str_cmp(arg2, "sav4"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -30 || value > 30)
      {
         send_to_char("Saving throw range is -30 to 30.\n\r", ch);
         return;
      }
      victim->saving_breath = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->saving_breath = value;
      return;
   }

   if (!str_cmp(arg2, "sav5"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -30 || value > 30)
      {
         send_to_char("Saving throw range is -30 to 30.\n\r", ch);
         return;
      }
      victim->saving_spell_staff = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->saving_spell_staff = value;
      return;
   }

   if (!str_cmp(arg2, "sex"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 2)
      {
         send_to_char("Sex range is 0 to 2.\n\r", ch);
         return;
      }
      victim->sex = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->sex = value;
      return;
   }
   if (!str_cmp(arg2, "race"))
   {
      if (!can_mmodify(ch, victim))
         return;
      value = get_npc_race(arg3);
      if (value < 0)
         value = atoi(arg3);
      if (!IS_NPC(victim) && (value < 0 || value >= MAX_RACE))
      {
         ch_printf(ch, "Race range is 0 to %d.\n", MAX_RACE - 1);
         return;
      }
      if (IS_NPC(victim) && (value < 0 || value >= max_npc_race))
      {
         ch_printf(ch, "Race range is 0 to %d.\n", max_npc_race - 1);
         return;
      }
      victim->race = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->race = value;
      return;
   }


   if (!str_cmp(arg2, "gold"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 10000000)
      {
         send_to_char("Sorry, gold range is between 0 and 10,000,000. Please try again.\n\r", ch);
         return;
      }
      victim->gold = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->gold = value;
      return;
   }

   if (!str_cmp(arg2, "hitroll"))
   {
      if (!can_mmodify(ch, victim))
         return;
      victim->hitroll = URANGE(0, value, 85);
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->hitroll = victim->hitroll;
      return;
   }

   if (!str_cmp(arg2, "damroll"))
   {
      if (!can_mmodify(ch, victim))
         return;
      victim->damroll = URANGE(0, value, 65);
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->damroll = victim->damroll;
      return;
   }

   if (!str_cmp(arg2, "hp"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > MAX_HPMANA)
      {
         ch_printf(ch, "Hp range is 1 to %d hit points.\n\r", MAX_HPMANA);
         return;
      }
      victim->max_hit = value;
      return;
   }

   if (!str_cmp(arg2, "mana"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_HPMANA)
      {
         ch_printf(ch, "Mana range is 0 to %d mana points.\n\r", MAX_HPMANA);
         return;
      }
      victim->max_mana = value;
      return;
   }

   if (!str_cmp(arg2, "move"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 30000)
      {
         send_to_char("Move range is 0 to 30,000 move points.\n\r", ch);
         return;
      }
      victim->max_move = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->max_move = value;
      return;
   }

   if (!str_cmp(arg2, "practice"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 500)
      {
         send_to_char("Practice range is 0 to 500 sessions.\n\r", ch);
         return;
      }
      victim->practice = value;
      return;
   }

   if (!str_cmp(arg2, "align"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < -1000 || value > 1000)
      {
         send_to_char("Alignment range is -1000 to 1000.\n\r", ch);
         return;
      }
      victim->alignment = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->alignment = value;
      return;
   }
   if (!str_cmp(arg2, "cweight"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Can only adjust this on PCs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 13)
      {
         send_to_char("Range is 1 to 13.\n\r", ch);
         return;
      }
      victim->pcdata->cweight = value;
      victim->height = number_range(race_table[victim->race]->height * .9, race_table[victim->race]->height * 1.1);
      victim->weight = number_range(race_table[victim->race]->weight * .9, race_table[victim->race]->weight * 1.1);
      victim->height = victim->height * get_heightweight_percent(victim->pcdata->cheight) / 100;
      victim->weight = victim->weight * get_heightweight_percent(victim->pcdata->cweight) / 100;
      return;
   }
   if (!str_cmp(arg2, "cheight"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Can only adjust this on PCs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 13)
      {
         send_to_char("Range is 1 to 13.\n\r", ch);
         return;
      }
      victim->pcdata->cheight = value;
      return;
   }
   if (!str_cmp(arg2, "eyecolor"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Can only adjust this on PCs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 24)
      {
         send_to_char("Range is 1 to 24.\n\r", ch);
         return;
      }
      victim->pcdata->eyecolor = value;
      return;
   }
   if (!str_cmp(arg2, "hairstyle"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Can only adjust this on PCs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 15)
      {
         send_to_char("Range is 1 to 15.\n\r", ch);
         return;
      }
      victim->pcdata->hairstyle = value;
      return;
   }
   if (!str_cmp(arg2, "hairlength"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Can only adjust this on PCs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 13)
      {
         send_to_char("Range is 1 to 13.\n\r", ch);
         return;
      }
      victim->pcdata->hairlength = value;
      return;
   }
   if (!str_cmp(arg2, "haircolor"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Can only adjust this on PCs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 40)
      {
         send_to_char("Range is 1 to 40.\n\r", ch);
         return;
      }
      victim->pcdata->haircolor = value;
      return;
   }
   if (!str_cmp(arg2, "skincolor"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Can only adjust this on PCs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 1 || value > 15)
      {
         send_to_char("Range is 1 to 15.\n\r", ch);
         return;
      }
      victim->pcdata->skincolor = value;
      return;
   }
   if (!str_cmp(arg2, "m1"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -10000000 || value > 10000000)
      {
         send_to_char("m1 range is -10,000,000 to 10,000,000.\n\r", ch);
         return;
      }
      victim->m1 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m1 = value;
      return;
   }
   if (!str_cmp(arg2, "m2"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -10000000 || value > 10000000)
      {
         send_to_char("m2 range is -10,000,000 to 10,000,000.\n\r", ch);
         return;
      }
      victim->m2 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m2 = value;
      return;
   }
   if (!str_cmp(arg2, "m3"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -10000000 || value > 10000000)
      {
         send_to_char("m3 range is -10,000,000 to 10,000,000.\n\r", ch);
         return;
      }
      victim->m3 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m3 = value;
      return;
   }
   if (!str_cmp(arg2, "m4"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m4 range is -2,000,000,000 to 2,000,000,000.\n\r", ch);
         return;
      }
      victim->m4 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m4 = value;
      return;
   }
   if (!str_cmp(arg2, "m5"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m5 range is -2,000,000,000 to 2,000,000,000.\n\r", ch);
         return;
      }
      victim->m5 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m5 = value;
      return;
   }
   if (!str_cmp(arg2, "m6"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m6 range is -2,000,000,000 to 2,000,000,000.\n\r", ch);
         return;
      }
      victim->m6 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m6 = value;
      return;
   }

   if (!str_cmp(arg2, "m7"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m7 range is 2,000,000,000 to -2,000,000,000\n\r", ch);
         return;
      }
      victim->m7 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m7 = value;
      return;
   }
   if (!str_cmp(arg2, "m8"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m8 range is 2,000,000,000 to -2,000,000,000\n\r", ch);
         return;
      }
      victim->m8 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m8 = value;
      return;
   }
   if (!str_cmp(arg2, "m9"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m9 range is 2,000,000,000 to -2,000,000,000\n\r", ch);
         return;
      }
      victim->m9 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m9 = value;
      return;
   }
   if (!str_cmp(arg2, "m10"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m6 range is -2,000,000,000 to 2,000,000,000.\n\r", ch);
         return;
      }
      victim->m10 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m10 = value;
      return;
   }
   if (!str_cmp(arg2, "m11"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m6 range is -2,000,000,000 to 2,000,000,000.\n\r", ch);
         return;
      }
      victim->m11 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m11 = value;
      return;
   }
   if (!str_cmp(arg2, "m12"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }
      if (value < -2000000000 || value > 2000000000)
      {
         send_to_char("m6 range is -2,000,000,000 to 2,000,000,000.\n\r", ch);
         return;
      }
      victim->m12 = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->m12 = value;
      return;
   }
   if (!str_cmp(arg2, "coordx"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if ((value < 1 || value > MAX_X) && (value != 0))
      {
         ch_printf(ch, "X Range is 1 from %d or -1.\n\r", MAX_X);
         return;
      }
      victim->coord->x = value;
      return;
   }
   if (!str_cmp(arg2, "coordy"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if ((value < -1 || value > MAX_Y) && (value != 0))
      {
         ch_printf(ch, "Y Range is 1 from %d or -1.\n\r", MAX_Y);
         return;
      }
      victim->coord->y = value;
      return;
   }
   if (!str_cmp(arg2, "onmap"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAP_MAX)
      {
         ch_printf(ch, "X Range is 0 from %d.\n\rHelp onmap for map numbers.", MAP_MAX);
         return;
      }
      victim->map = value;
      return;
   }
   if (!str_cmp(arg2, "password"))
   {
      char *pwdnew;
      char *p;

      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Staff Only memebers can set this.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Mobs don't have passwords.\n\r", ch);
         return;
      }

      if (strlen(arg3) < 5)
      {
         send_to_char("New password must be at least five characters long.\n\r", ch);
         return;
      }

      /*
       * No tilde allowed because of player file format.
       */
      pwdnew = crypt(arg3, ch->name);
      for (p = pwdnew; *p != '\0'; p++)
      {
         if (*p == '~')
         {
            send_to_char("New password not acceptable, try again.\n\r", ch);
            return;
         }
      }

      DISPOSE(victim->pcdata->pwd);
      victim->pcdata->pwd = str_dup(pwdnew);
      if (IS_SET(sysdata.save_flags, SV_PASSCHG))
         save_char_obj(victim);
      send_to_char("Ok.\n\r", ch);
      ch_printf(victim, "Your password has been changed by %s.\n\r", ch->name);
      return;
   }
   if (!str_cmp(arg2, "cident"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Sorry only staff can configure Caste mobs.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Can only set this on mobs!", ch);
         return;
      }
      if (value > 100)
      {
         send_to_char("Help Castemob for a valid range.\n\r", ch);
         return;
      }
      victim->cident = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->cident = value;
      return;
   }
   if(!str_cmp(arg2, "fame"))
   {
		if(IS_NPC(victim))
		{
			send_to_char("You can only set the fame of PLAYERS!\n\r", ch);
			return;
		}
		if(get_trust(ch) < LEVEL_STAFF)
		{
			send_to_char("Only Staff can set fame!\n\r", ch);
			return;
		}
		if(value < 0)
		{
			send_to_char("You can not set someone's fame to less than zero!\n\r", ch);
			return;
		}
		
		victim->fame = value;
		return;
   }
   if (!str_cmp(arg2, "rank"))
   {
      if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }
      smash_tilde(argument);
      DISPOSE(victim->pcdata->rank);
      if (!argument || argument[0] == '\0' || !str_cmp(argument, "none"))
         victim->pcdata->rank = str_dup("");
      else
         victim->pcdata->rank = str_dup(argument);
      send_to_char("Ok.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg2, "rewarda"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      victim->pcdata->reward_accum = value;
      return;
   }

   if (!str_cmp(arg2, "reward"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < 0 || value > 99999)
      {
         send_to_char("The current quest point range is 0 to 99999.\n\r", ch);
         return;
      }

      victim->pcdata->reward_curr = value;
      return;
   }
    

   if (!str_cmp(arg2, "qpa"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      victim->pcdata->quest_accum = value;
      return;
   }

   if (!str_cmp(arg2, "qp"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < 0 || value > 50000000)
      {
         send_to_char("The current quest point range is 0 to 50,000,00.\n\r", ch);
         return;
      }

      victim->pcdata->quest_curr = value;
      return;
   }

   if (!str_cmp(arg2, "favor"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < -2500 || value > 2500)
      {
         send_to_char("Range is from -2500 to 2500.\n\r", ch);
         return;
      }

      victim->pcdata->favor = value;
      return;
   }

   if (!str_cmp(arg2, "town"))
   {
      TOWN_DATA *town;
      
      town = get_town(argument);
      
      if (!town)
      {
         send_to_char("That is not a town.\n\r", ch);
         return;
      }
      victim->pcdata->town = town;
      send_to_char("Set.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "hometown") || !str_cmp(arg2, "kingdom"))
   {
      TOWN_DATA *town;
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < 0 || value >= sysdata.max_kingdom)
      {
         ch_printf(ch, "Range is 0 - %d.  The list is available at showkingdoms.\n\r", sysdata.max_kingdom - 1);
         return;
      }
      if (victim->pcdata)
      {
         if (victim->pcdata->hometown != value)
         {
            if (victim->pcdata->hometown > 1)
               remove_player_list(victim, 0);

            victim->pcdata->hometown = value;
            victim->pcdata->kingdompid = kingdom_table[value]->kpid;

            if (value > 1)
               add_player_list(victim, 0);
               
            town = get_town(kingdom_table[value]->dtown);
            if (town)
               victim->pcdata->town = town;
            else
            {
               send_to_char("That kingdom does not have a dtown.\n\r", ch);
               victim->pcdata->town = NULL;
            }
            return;
         }
         victim->pcdata->hometown = value;
         victim->pcdata->kingdompid = kingdom_table[value]->kpid;

         return;
      }
   }

   if (!str_cmp(arg2, "mentalstate"))
   {
      if (value < -100 || value > 100)
      {
         send_to_char("Value must be in range -100 to +100.\n\r", ch);
         return;
      }
      victim->mental_state = value;
      return;
   }

   if (!str_cmp(arg2, "emotion"))
   {
      if (value < -100 || value > 100)
      {
         send_to_char("Value must be in range -100 to +100.\n\r", ch);
         return;
      }
      victim->emotional_state = value;
      return;
   }

   if (!str_cmp(arg2, "thirst"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < 0 || value > 100)
      {
         send_to_char("Thirst range is 0 to 100.\n\r", ch);
         return;
      }

      victim->pcdata->condition[COND_THIRST] = value;
      return;
   }

   if (!str_cmp(arg2, "drunk"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < 0 || value > 100)
      {
         send_to_char("Drunk range is 0 to 100.\n\r", ch);
         return;
      }

      victim->pcdata->condition[COND_DRUNK] = value;
      return;
   }

   if (!str_cmp(arg2, "full"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < 0 || value > 100)
      {
         send_to_char("Full range is 0 to 100.\n\r", ch);
         return;
      }

      victim->pcdata->condition[COND_FULL] = value;
      return;
   }

   if (!str_cmp(arg2, "blood"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (value < 0 || value > MAX_LEVEL + 10)
      {
         ch_printf(ch, "Blood range is 0 to %d.\n\r", MAX_LEVEL + 10);
         return;
      }

      victim->pcdata->condition[COND_BLOODTHIRST] = value;
      return;
   }

   if (!str_cmp(arg2, "name"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_ADMIN) /* Tracker1 */
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }

      STRFREE(victim->name);
      victim->name = STRALLOC(arg3);
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
      {
         STRFREE(victim->pIndexData->player_name);
         victim->pIndexData->player_name = QUICKLINK(victim->name);
      }
      return;
   }

   if (!str_cmp(arg2, "balance"))
   {
      if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }
      if (victim->pcdata)
      {
         victim->pcdata->balance = value;
         return;
      }
   }

   if (!str_cmp(arg2, "minsnoop"))
   {
      if (get_trust(ch) < LEVEL_HI_STAFF) /* Tracker1 */
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }
      if (victim->pcdata)
      {
         victim->pcdata->min_snoop = value;
         return;
      }
   }

   if (!str_cmp(arg2, "clan"))
   {
      CLAN_DATA *clan;

      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (!arg3 || arg3[0] == '\0')
      {
         /* Crash bug fix, oops guess I should have caught this one :)
          * But it was early in the morning :P --Shaddai 
          */
         if (victim->pcdata->clan == NULL)
            return;
         /* Added a check on immortals so immortals don't take up
          * any membership space. --Shaddai
          */
         if (!IS_IMMORTAL(victim))
         {
            --victim->pcdata->clan->members;
            save_clan(victim->pcdata->clan);
         }
         STRFREE(victim->pcdata->clan_name);
         victim->pcdata->clan_name = STRALLOC("");
         victim->pcdata->clan = NULL;
         remove_player_list(victim, 1);
         return;
      }
      clan = get_clan(arg3);
      if (!clan)
      {
         send_to_char("No such clan.\n\r", ch);
         return;
      }
      if (victim->pcdata->clan != NULL && !IS_IMMORTAL(victim))
      {
         --victim->pcdata->clan->members;
         save_clan(victim->pcdata->clan);
      }
      if (victim->pcdata->clan != NULL)
         remove_player_list(victim, 1);
      STRFREE(victim->pcdata->clan_name);
      victim->pcdata->clan_name = QUICKLINK(clan->name);
      victim->pcdata->clan = clan;
      add_player_list(victim, 1);
      if (!IS_IMMORTAL(victim))
      {
         ++victim->pcdata->clan->members;
         save_clan(victim->pcdata->clan);
      }
      return;
   }

   if (!str_cmp(arg2, "deity"))
   {
      DEITY_DATA *deity;

      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (!arg3 || arg3[0] == '\0')
      {
         STRFREE(victim->pcdata->deity_name);
         victim->pcdata->deity_name = STRALLOC("");
         victim->pcdata->deity = NULL;
         send_to_char("Deity removed.\n\r", ch);
         return;
      }

      deity = get_deity(arg3);
      if (!deity)
      {
         send_to_char("No such deity.\n\r", ch);
         return;
      }
      STRFREE(victim->pcdata->deity_name);
      victim->pcdata->deity_name = QUICKLINK(deity->name);
      victim->pcdata->deity = deity;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "council"))
   {
      COUNCIL_DATA *council;

      if (get_trust(ch) < LEVEL_HI_STAFF) /* Tracker1 */
      {
         send_to_char("You can't do that.\n\r", ch);
         return;
      }
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      if (!arg3 || arg3[0] == '\0')
      {
         STRFREE(victim->pcdata->council_name);
         victim->pcdata->council_name = STRALLOC("");
         victim->pcdata->council = NULL;
         send_to_char("Removed from council.\n\rPlease make sure you adjust that council's members accordingly.\n\r", ch);
         return;
      }

      council = get_council(arg3);
      if (!council)
      {
         send_to_char("No such council.\n\r", ch);
         return;
      }
      STRFREE(victim->pcdata->council_name);
      victim->pcdata->council_name = QUICKLINK(council->name);
      victim->pcdata->council = council;
      send_to_char("Done.\n\rPlease make sure you adjust that council's members accordingly.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "short"))
   {
      STRFREE(victim->short_descr);
      victim->short_descr = STRALLOC(arg3);
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
      {
         STRFREE(victim->pIndexData->short_descr);
         victim->pIndexData->short_descr = QUICKLINK(victim->short_descr);
      }
      return;
   }

   if (!str_cmp(arg2, "long"))
   {
      STRFREE(victim->long_descr);
      strcpy(buf, arg3);
      strcat(buf, "\n\r");
      victim->long_descr = STRALLOC(buf);
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
      {
         STRFREE(victim->pIndexData->long_descr);
         victim->pIndexData->long_descr = QUICKLINK(victim->long_descr);
      }
      return;
   }

   if (!str_cmp(arg2, "description"))
   {
      if (arg3[0])
      {
         STRFREE(victim->description);
         victim->description = STRALLOC(arg3);
         if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         {
            STRFREE(victim->pIndexData->description);
            victim->pIndexData->description = QUICKLINK(victim->description);
         }
         return;
      }
      CHECK_SUBRESTRICTED(ch);
      if (ch->substate == SUB_REPEATCMD)
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_MOB_DESC;
      ch->dest_buf = victim;
      start_editing(ch, victim->description);
      if (IS_NPC(victim))
         editor_desc_printf(ch, "Description of mob, vnum %ld (%s).", victim->pIndexData->vnum, victim->name);
      else
         editor_desc_printf(ch, "Description of player %s.", capitalize(victim->name));
      return;
   }

   if (!str_cmp(arg2, "title"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Not on NPC's.\n\r", ch);
         return;
      }

      set_title(victim, arg3);
      return;
   }

   if (!str_cmp(arg2, "spec"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (!IS_NPC(victim))
      {
         send_to_char("Not on PC's.\n\r", ch);
         return;
      }

      if (!str_cmp(arg3, "none"))
      {
         victim->spec_fun = NULL;
         send_to_char("Special function removed.\n\r", ch);
         if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
            victim->pIndexData->spec_fun = victim->spec_fun;
         return;
      }

      if ((victim->spec_fun = spec_lookup(arg3)) == 0)
      {
         send_to_char("No such spec fun.\n\r", ch);
         return;
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->spec_fun = victim->spec_fun;
      return;
   }
   
   if (!str_cmp(arg2, "talent"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("This only works on PCs.\n\r", ch);
         return;
      }
      if (get_trust(ch) < LEVEL_HI_IMM)
      {
         send_to_char("You are not high enough in level to do this.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> talent <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_talentflag(arg3);

         if (value < 0 || value >= TALENT_MAX)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
         {
            xTOGGLE_BIT(victim->pcdata->talent, value);
         }
      }
      return;
   }

   if (!str_cmp(arg2, "flags"))
   {
      bool pcflag;

      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's flags.\n\r", ch);
         return;
      }

      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> flags <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         pcflag = FALSE;
         argument = one_argument(argument, arg3);
         value = IS_NPC(victim) ? get_actflag(arg3) : get_plrflag(arg3);

         if (!IS_NPC(victim) && (value < 0 || value > MAX_BITS))
         {
            pcflag = TRUE;
            value = get_pcflag(arg3);
         }
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
         {
            if (IS_NPC(victim) && value == ACT_PROTOTYPE && get_trust(ch) < sysdata.level_modify_proto /* Tracker1 */
               && !is_name("protoflag", ch->pcdata->bestowments))
               send_to_char("You cannot change the prototype flag.\n\r", ch);
            else if (IS_NPC(victim) && value == ACT_IS_NPC)
               send_to_char("If that could be changed, it would cause many problems.\n\r", ch);
            else
            {
               if (pcflag)
                  TOGGLE_BIT(victim->pcdata->flags, 1 << value);
               else
               {
                  xTOGGLE_BIT(victim->act, value);
                  /* NPC check added by Gorog */
                  if (IS_NPC(victim) && value == ACT_PROTOTYPE)
                     victim->pIndexData->act = victim->act;
               }
            }
         }
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->act = victim->act;
      return;
   }

   if (!str_cmp(arg2, "affected"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's flags.\n\r", ch);
         return;
      }

      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> affected <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_aflag(arg3);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
         {
            xTOGGLE_BIT(victim->affected_by, value);
         }
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->affected_by = victim->affected_by;
      return;
   }

   /*
    * save some more finger-leather for setting RIS stuff
    */
   if (!str_cmp(arg2, "r"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's ris.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sprintf(outbuf, "%s resistant %s", arg1, arg3);
      do_mset(ch, outbuf);
      return;
   }
   if (!str_cmp(arg2, "i"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM)
         /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's ris.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;


      sprintf(outbuf, "%s immune %s", arg1, arg3);
      do_mset(ch, outbuf);
      return;
   }
   if (!str_cmp(arg2, "s"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's ris.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sprintf(outbuf, "%s susceptible %s", arg1, arg3);
      do_mset(ch, outbuf);
      return;
   }
   if (!str_cmp(arg2, "ri"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's ris.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sprintf(outbuf, "%s resistant %s", arg1, arg3);
      do_mset(ch, outbuf);
      sprintf(outbuf, "%s immune %s", arg1, arg3);
      do_mset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "rs"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's ris.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sprintf(outbuf, "%s resistant %s", arg1, arg3);
      do_mset(ch, outbuf);
      sprintf(outbuf, "%s susceptible %s", arg1, arg3);
      do_mset(ch, outbuf);
      return;
   }
   if (!str_cmp(arg2, "is"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's ris.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sprintf(outbuf, "%s immune %s", arg1, arg3);
      do_mset(ch, outbuf);
      sprintf(outbuf, "%s susceptible %s", arg1, arg3);
      do_mset(ch, outbuf);
      return;
   }
   if (!str_cmp(arg2, "ris"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's ris.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sprintf(outbuf, "%s resistant %s", arg1, arg3);
      do_mset(ch, outbuf);
      sprintf(outbuf, "%s immune %s", arg1, arg3);
      do_mset(ch, outbuf);
      sprintf(outbuf, "%s susceptible %s", arg1, arg3);
      do_mset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "element"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You cannot modify a mobs Element\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> element <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_elementflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
         {
            victim->elementb = 0;
            TOGGLE_BIT(victim->elementb, 1 << value);
         }
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->elementb = victim->elementb;
      return;
   }


   if (!str_cmp(arg2, "resistant"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's resistancies.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> resistant <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_risflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            TOGGLE_BIT(victim->resistant, 1 << value);
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->resistant = victim->resistant;
      return;
   }

   if (!str_cmp(arg2, "immune"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's immunities.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> immune <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_risflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            TOGGLE_BIT(victim->immune, 1 << value);
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->immune = victim->immune;
      return;
   }

   if (!str_cmp(arg2, "susceptible"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's susceptibilities.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> susceptible <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_risflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            TOGGLE_BIT(victim->susceptible, 1 << value);
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->susceptible = victim->susceptible;
      return;
   }

   if (!str_cmp(arg2, "part"))
   {
      if (!IS_NPC(victim) && get_trust(ch) < LEVEL_IMM) /* Tracker1 */
      {
         send_to_char("You can only modify a mobile's parts.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> part <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_partflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            TOGGLE_BIT(victim->xflags, 1 << value);
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->xflags = victim->xflags;
      return;
   }

   if (!str_cmp(arg2, "attack"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("You can only modify a mobile's attacks.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> attack <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_attackflag(arg3);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            xTOGGLE_BIT(victim->attacks, value);
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->attacks = victim->attacks;
      return;
   }

   if (!str_cmp(arg2, "defense"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("You can only modify a mobile's defenses.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> defense <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_defenseflag(arg3);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            xTOGGLE_BIT(victim->defenses, value);
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->defenses = victim->defenses;
      return;
   }

   if (!str_cmp(arg2, "pos"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > POS_STANDING)
      {
         ch_printf(ch, "Position range is 0 to %d.\n\r", POS_STANDING);
         return;
      }
      victim->position = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->position = victim->position;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "defpos"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > POS_STANDING)
      {
         ch_printf(ch, "Position range is 0 to %d.\n\r", POS_STANDING);
         return;
      }
      victim->defposition = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->defposition = victim->defposition;
      send_to_char("Done.\n\r", ch);
      return;
   }

   /*
    * save some finger-leather
    */
   if (!str_cmp(arg2, "hitdie"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sscanf(arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus);
      sprintf(outbuf, "%s hitnumdie %d", arg1, num);
      do_mset(ch, outbuf);

      sprintf(outbuf, "%s hitsizedie %d", arg1, size);
      do_mset(ch, outbuf);

      sprintf(outbuf, "%s hitplus %d", arg1, plus);
      do_mset(ch, outbuf);
      return;
   }
   /*
    * save some more finger-leather
    */
   if (!str_cmp(arg2, "damdie"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;

      sscanf(arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus);
      sprintf(outbuf, "%s damnumdie %d", arg1, num);
      do_mset(ch, outbuf);
      sprintf(outbuf, "%s damsizedie %d", arg1, size);
      do_mset(ch, outbuf);
      sprintf(outbuf, "%s damplus %d", arg1, plus);
      do_mset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "hitnumdie"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_HPMANA)
      {
         ch_printf(ch, "Number of hitpoint dice range is 0 to %d.\n\r", MAX_HPMANA);
         return;
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->hitnodice = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "hitsizedie"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_HPMANA)
      {
         ch_printf(ch, "Hitpoint dice size range is 0 to %d.\n\r", MAX_HPMANA);
         return;
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->hitsizedice = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "hitplus"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > MAX_HPMANA)
      {
         ch_printf(ch, "Hitpoint bonus range is 0 to %d.\n\r", MAX_HPMANA);
         return;
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->hitplus = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "damnumdie"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 100)
      {
         send_to_char("Number of damage dice range is 0 to 100.\n\r", ch);
         return;
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->damnodice = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "damsizedie"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 100)
      {
         send_to_char("Damage dice size range is 0 to 100.\n\r", ch);
         return;
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->damsizedice = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "damplus"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 1000)
      {
         send_to_char("Damage bonus range is 0 to 1000.\n\r", ch);
         return;
      }

      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->damplus = value;
      send_to_char("Done.\n\r", ch);
      return;

   }

   if (!str_cmp(arg2, "damaddhi") || !str_cmp(arg2, "damaddhigh"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 1000)
      {
         send_to_char("Damage bonus range is 0 to 1000.\n\r", ch);
         return;
      }
      victim->damaddhi = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->damaddhi = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "damaddlow") || !str_cmp(arg2, "damaddlo"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Mobiles only.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (value < 0 || value > 1000)
      {
         send_to_char("Damage bonus range is 0 to 1000.\n\r", ch);
         return;
      }
      victim->damaddlow = value;
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->damaddlow = value;
      send_to_char("Done.\n\r", ch);
      return;
   }


   if (!str_cmp(arg2, "aloaded"))
   {
      if (IS_NPC(victim))
      {
         send_to_char("Player Characters only.\n\r", ch);
         return;
      }


      if (!can_mmodify(ch, victim))
         return;

      if (!IS_SET(victim->pcdata->area->status, AREA_LOADED))
      {
         SET_BIT(victim->pcdata->area->status, AREA_LOADED);
         send_to_char("Your area set to LOADED!\n\r", victim);
         if (ch != victim)
            send_to_char("Area set to LOADED!\n\r", ch);
         return;
      }
      else
      {
         REMOVE_BIT(victim->pcdata->area->status, AREA_LOADED);
         send_to_char("Your area set to NOT-LOADED!\n\r", victim);
         if (ch != victim)
            send_to_char("Area set to NON-LOADED!\n\r", ch);
         return;
      }
   }



   if (!str_cmp(arg2, "speaks"))
   {
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> speaks <language> [language] ...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_langflag(arg3);
         if (value == LANG_UNKNOWN)
            ch_printf(ch, "Unknown language: %s\n\r", arg3);
         else if (!IS_NPC(victim))
         {
            int valid_langs = LANG_COMMON | LANG_ELVEN | LANG_DWARVEN | LANG_OGRE | LANG_HOBBIT | LANG_FAIRY;

            if (!(value &= valid_langs))
            {
               ch_printf(ch, "Players may not know %s.\n\r", arg3);
               continue;
            }
         }

         v2 = get_langnum(arg3);
         if (v2 == -1)
            ch_printf(ch, "Unknown language: %s\n\r", arg3);
         else
            TOGGLE_BIT(victim->speaks, 1 << v2);
      }
      if (!IS_NPC(victim))
      {
         REMOVE_BIT(victim->speaks, race_table[victim->race]->language);
         if (!knows_language(victim, victim->speaking, victim))
            victim->speaking = race_table[victim->race]->language;
      }
      else if (is_prototype(ch, NULL, victim))
         victim->pIndexData->speaks = victim->speaks;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "speaking"))
   {
      if (!IS_NPC(victim))
      {
         send_to_char("Players must choose the language they speak themselves.\n\r", ch);
         return;
      }
      if (!can_mmodify(ch, victim))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: mset <victim> speaking <language> [language]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_langflag(arg3);
         if (value == LANG_UNKNOWN)
            ch_printf(ch, "Unknown language: %s\n\r", arg3);
         else
         {
            v2 = get_langnum(arg3);
            if (v2 == -1)
               ch_printf(ch, "Unknown language: %s\n\r", arg3);
            else
               TOGGLE_BIT(victim->speaking, 1 << v2);
         }
      }
      if (IS_NPC(victim) && (is_prototype(ch, NULL, victim)))
         victim->pIndexData->speaking = victim->speaking;
      send_to_char("Done.\n\r", ch);
      return;
   }

   /*
    * Generate usage message.
    */
   if (ch->substate == SUB_REPEATCMD)
   {
      ch->substate = SUB_RESTRICTED;
      interpret(ch, origarg);
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_mset;
   }
   else
      do_mset(ch, "");
   return;
}

float adjust_weight(char *argument)
{
   float value;
   char vbuf[MSL];
   
   value = atof(argument);
   sprintf(vbuf, "%.2f", value);
   value = atof(vbuf);
   if (value < .01)
      return .01;
   else
      return value;
}
//1000 - Damage  1001 - Durability  1002 - TohitBash  1003 - TohitStab   1004 - TohitSlash
//1005 - Weight  1006 - Shieldlag   1007 - Blocking % 1008 - Proj Range  1009 - Parry Chance 1010 - Stop Parry
//1011 - SpellSN 1012 - SpellStr    1013 - Unbeakable 1014 - Nodisarm    1015 - Sanctified 1016 - Change Size
int get_gemspec(char *argument)
{
   if (!str_cmp(argument, "spec_damage"))
      return 1000;
   else if (!str_cmp(argument, "spec_durability"))
      return 1001;
   else if (!str_cmp(argument, "spec_tohitbash"))
      return 1002;
   else if (!str_cmp(argument, "spec_tohitstab"))
      return 1003;
   else if (!str_cmp(argument, "spec_tohitslash"))
      return 1004;
   else if (!str_cmp(argument, "spec_weight"))
      return 1005;
   else if (!str_cmp(argument, "spec_shieldlag"))
      return 1006;
   else if (!str_cmp(argument, "spec_blocking_percent"))
      return 1007;
   else if (!str_cmp(argument, "spec_proj_range"))
      return 1008;
   else if (!str_cmp(argument, "spec_parry_chance"))
      return 1009;
   else if (!str_cmp(argument, "spec_stop_parry"))
      return 1010;
   else if (!str_cmp(argument, "spec_spellsn"))
      return 1011;
   else if (!str_cmp(argument, "spec_spellstr"))
      return 1012;
   else if (!str_cmp(argument, "spec_unbreakable"))
      return 1013;
   else if (!str_cmp(argument, "spec_nodisarm"))
      return 1014;
   else if (!str_cmp(argument, "spec_sanctified"))
      return 1015;
   else if (!str_cmp(argument, "spec_change_size"))
      return 1016;
   else if (!str_cmp(argument, "spec_saves"))
      return 1017;
   else
      return -1;
}

void do_oset(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];
   char outbuf[MSL];
   OBJ_DATA *obj, *tmpobj;
   EXTRA_DESCR_DATA *ed;
   bool lockobj;
   char *origarg = argument;

   int value, tmp;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Mob's can't oset\n\r", ch);
      return;
   }

   if (!ch->desc)
   {
      send_to_char("You have no descriptor\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;

      case SUB_OBJ_EXTRA:
         if (!ch->dest_buf)
         {
            send_to_char("Fatal error: report to Thoric.\n\r", ch);
            bug("do_oset: sub_obj_extra: NULL ch->dest_buf", 0);
            ch->substate = SUB_NONE;
            return;
         }
         /*
          * hopefully the object didn't get extracted...
          * if you're REALLY paranoid, you could always go through
          * the object and index-object lists, searching through the
          * extra_descr lists for a matching pointer...
          */
         ed = ch->dest_buf;
         STRFREE(ed->description);
         ed->description = copy_buffer(ch);
         tmpobj = ch->spare_ptr;
         stop_editing(ch);
         ch->dest_buf = tmpobj;
         ch->substate = ch->tempnum;
         return;

      case SUB_OBJ_LONG:
         if (!ch->dest_buf)
         {
            send_to_char("Fatal error: report to Thoric.\n\r", ch);
            bug("do_oset: sub_obj_long: NULL ch->dest_buf", 0);
            ch->substate = SUB_NONE;
            return;
         }
         obj = ch->dest_buf;
         if (obj && obj_extracted(obj))
         {
            send_to_char("Your object was extracted!\n\r", ch);
            stop_editing(ch);
            return;
         }
         STRFREE(obj->description);
         obj->description = copy_buffer(ch);
         if (is_prototype(ch, obj, NULL))
         {
            if (can_omodify(ch, obj))
            {
               STRFREE(obj->pIndexData->description);
               obj->pIndexData->description = QUICKLINK(obj->description);
            }
         }
         tmpobj = ch->spare_ptr;
         stop_editing(ch);
         ch->substate = ch->tempnum;
         ch->dest_buf = tmpobj;
         return;
   }

   obj = NULL;
   smash_tilde(argument);

   if (ch->substate == SUB_REPEATCMD)
   {
      obj = ch->dest_buf;
      if (obj && obj_extracted(obj))
      {
         send_to_char("Your object was extracted!\n\r", ch);
         obj = NULL;
         argument = "done";
      }
      if (argument[0] == '\0' || !str_cmp(argument, " ") || !str_cmp(argument, "stat"))
      {
         if (obj)
            do_ostat(ch, obj->name);
         else
            send_to_char("No object selected.  Type '?' for help.\n\r", ch);
         return;
      }
      if (!str_cmp(argument, "done") || !str_cmp(argument, "off"))
      {
         send_to_char("Oset mode off.\n\r", ch);
         ch->substate = SUB_NONE;
         ch->dest_buf = NULL;
         if (ch->pcdata && ch->pcdata->subprompt)
         {
            STRFREE(ch->pcdata->subprompt);
            ch->pcdata->subprompt = NULL;
            if (xIS_SET(ch->act, PLR_OSET)) /* oset flags, Crash no like -- Xerves */
               xREMOVE_BIT(ch->act, PLR_OSET);
         }
         return;
      }
   }
   if (obj)
   {
      lockobj = TRUE;
      strcpy(arg1, obj->name);
      argument = one_argument(argument, arg2);
      strcpy(arg3, argument);
   }
   else
   {
      lockobj = FALSE;
      argument = one_argument(argument, arg1);
      argument = one_argument(argument, arg2);
      strcpy(arg3, argument);
   }

   if (!str_cmp(arg1, "on"))
   {
      send_to_char("Syntax: oset <object|vnum> on.\n\r", ch);
      return;
   }

   if (arg1[0] == '\0' || arg2[0] == '\0' || !str_cmp(arg1, "?"))
   {
      if (ch->substate == SUB_REPEATCMD)
      {
         if (obj)
            send_to_char("Syntax: <field>  <value>\n\r", ch);
         else
            send_to_char("Syntax: <object> <field>  <value>\n\r", ch);
      }
      else
         send_to_char("Syntax: oset <object> <field>  <value>\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Field being one of:\n\r", ch);
      send_to_char("  flags wear level weight cost rent timer\n\r", ch);
      send_to_char("  name short long ed rmed actiondesc\n\r", ch);
      send_to_char("  type value0 value1 value2 value3 value4 value5\n\r", ch); 
      send_to_char("  value6 value7 value8 value9 value10 value11 value12 value13\n\r", ch);
      send_to_char("  bashmod stabmod slashmod\n\r", ch);
      send_to_char("  affect rmaffect layers cident sworthrestrict gemslots\n\r", ch);
      send_to_char("For scrolls, potions and pills:\n\r", ch);
      send_to_char("  slevel spell1 spell2 spell3\n\r", ch);
      send_to_char("For wands and staves:\n\r", ch);
      send_to_char("  slevel spell maxcharges charges\n\r", ch);
      send_to_char("For containers:          For levers and switches:\n\r", ch);
      send_to_char("  cflags key capacity      tflags\n\r", ch);
      return;
   }

   if (!obj && get_trust(ch) < LEVEL_HI_IMM)
   {
      if ((obj = get_obj_here(ch, arg1)) == NULL)
      {
         send_to_char("You can't find that here.\n\r", ch);
         return;
      }
   }
   else if (!obj)
   {
      if ((obj = get_obj_world(ch, arg1)) == NULL)
      {
         send_to_char("There is nothing like that in all the realms.\n\r", ch);
         return;
      }
   }
   if (lockobj)
      ch->dest_buf = obj;
   else
      ch->dest_buf = NULL;

   separate_obj(obj);
   value = atoi(arg3);

   if (!str_cmp(arg2, "on"))
   {
      if ((xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_REDIT)))
      {
         send_to_char("Trying to turn on, while already on another, BLAH!\n\r", ch);
         return;
      }
      ch_printf(ch, "Oset mode on. (Editing '%s' vnum %d).\n\r", obj->name, obj->pIndexData->vnum);
      ch->substate = SUB_REPEATCMD;
      ch->dest_buf = obj;
      if (ch->pcdata)
      {
         if (ch->pcdata->subprompt)
            STRFREE(ch->pcdata->subprompt);
         sprintf(buf, "<&COset &W#%d&w> %%i", obj->pIndexData->vnum);
         ch->pcdata->subprompt = STRALLOC(buf);
         if (!xIS_SET(ch->act, PLR_OSET)) /* Oset flags, Crash no like -- Xerves */
            xSET_BIT(ch->act, PLR_OSET);
      }
      return;
   }
   
   if (!IS_NPC(ch) && IS_SET(ch->pcdata->flags, PCFLAG_AUTOPROTO))
   {
      send_to_char("WARNING:  Making Changes in AUTOPROTO MODE!\n\r", ch);
   }

   if (!str_cmp(arg2, "name"))
   {
      bool proto = FALSE;

      if (is_prototype(ch, obj, NULL))
         proto = TRUE;
      if (proto && !can_omodify(ch, obj))
         return;
      STRFREE(obj->name);
      obj->name = STRALLOC(arg3);
      if (proto)
      {
         STRFREE(obj->pIndexData->name);
         obj->pIndexData->name = QUICKLINK(obj->name);
      }
      return;
   }

   if (!str_cmp(arg2, "short"))
   {
      if (is_prototype(ch, obj, NULL))
      {
         if (!can_omodify(ch, obj))
            return;
         STRFREE(obj->short_descr);
         obj->short_descr = STRALLOC(arg3);
         STRFREE(obj->pIndexData->short_descr);
         obj->pIndexData->short_descr = QUICKLINK(obj->short_descr);
      }
      else
         /* Feature added by Narn, Apr/96
            * If the item is not proto, add the word 'rename' to the keywords
            * if it is not already there.
          */
      {
         STRFREE(obj->short_descr);
         obj->short_descr = STRALLOC(arg3);
         if (str_infix("rename", obj->name))
         {
            sprintf(buf, "%s %s", obj->name, "rename");
            STRFREE(obj->name);
            obj->name = STRALLOC(buf);
         }
      }
      return;
   }

   if (!str_cmp(arg2, "long"))
   {
      if (arg3[0])
      {
         if (is_prototype(ch, obj, NULL))
         {
            if (!can_omodify(ch, obj))
               return;
            STRFREE(obj->description);
            obj->description = STRALLOC(arg3);
            STRFREE(obj->pIndexData->description);
            obj->pIndexData->description = QUICKLINK(obj->description);
            return;
         }
         STRFREE(obj->description);
         obj->description = STRALLOC(arg3);
         return;
      }
      CHECK_SUBRESTRICTED(ch);
      if (ch->substate == SUB_REPEATCMD)
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      if (lockobj)
         ch->spare_ptr = obj;
      else
         ch->spare_ptr = NULL;
      ch->substate = SUB_OBJ_LONG;
      ch->dest_buf = obj;
      start_editing(ch, obj->description);
      editor_desc_printf(ch, "Object long desc, vnum %ld (%s).", obj->pIndexData->vnum, obj->short_descr);
      return;
   }

   if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[0] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[0] = value;
      return;
   }

   if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[1] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[1] = value;
      return;
   }

   if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[2] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[2] = value;
      return;
   }

   if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[3] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[3] = value;
      return;
   }

   if (!str_cmp(arg2, "value4") || !str_cmp(arg2, "v4"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[4] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[4] = value;
      return;
   }

   if (!str_cmp(arg2, "value5") || !str_cmp(arg2, "v5"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[5] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[5] = value;
      return;
   }

   if (!str_cmp(arg2, "value6") || !str_cmp(arg2, "v6"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[6] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[6] = value;
      return;
   }

   if (!str_cmp(arg2, "value7") || !str_cmp(arg2, "v7"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[7] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[7] = value;
      return;
   }

   if (!str_cmp(arg2, "value8") || !str_cmp(arg2, "v8"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[8] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[8] = value;
      return;
   }

   if (!str_cmp(arg2, "value9") || !str_cmp(arg2, "v9"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[9] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[9] = value;
      return;
   }
   
   if (!str_cmp(arg2, "value10") || !str_cmp(arg2, "v10"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[10] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[10] = value;
      return;
   }
   
   if (!str_cmp(arg2, "value11") || !str_cmp(arg2, "v11"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[11] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[11] = value;
      return;
   }
   
   if (!str_cmp(arg2, "value12") || !str_cmp(arg2, "v12"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[12] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[12] = value;
      return;
   }
   
   if (!str_cmp(arg2, "value13") || !str_cmp(arg2, "v13"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[13] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[13] = value;
      return;
   }

   if (!str_cmp(arg2, "bashmod"))
   {
      if (!can_omodify(ch, obj))
         return;

      if (value < 0 || value > MAX_TOHITAC)
      {
         ch_printf(ch, "Range is 0 to %d.\n\r", MAX_TOHITAC);
         return;
      }
      if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON)
      {
         obj->value[7] = value;
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[7] = value;
      }
      else if (obj->item_type == ITEM_ARMOR)
      {
         obj->value[0] = value;
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[0] = value;
      }
      else
      {
         send_to_char("This can only be used on weapons and armor.\n\r", ch);
      }
      return;
   }
   if (!str_cmp(arg2, "stabmod"))
   {
      if (!can_omodify(ch, obj))
         return;

      if (value < 0 || value > MAX_TOHITAC)
      {
         ch_printf(ch, "Range is 0 to %d.\n\r", MAX_TOHITAC);
         return;
      }
      if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON)
      {
         obj->value[9] = value;
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[9] = value;
      }
      else if (obj->item_type == ITEM_ARMOR)
      {
         obj->value[2] = value;
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[2] = value;
      }
      else
      {
         send_to_char("This can only be used on weapons and armor.\n\r", ch);
      }
      return;
   }
   if (!str_cmp(arg2, "slashmod"))
   {
      if (!can_omodify(ch, obj))
         return;

      if (value < 0 || value > MAX_TOHITAC)
      {
         ch_printf(ch, "Range is 0 to %d.\n\r", MAX_TOHITAC);
         return;
      }
      if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON)
      {
         obj->value[8] = value;
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[8] = value;
      }
      else if (obj->item_type == ITEM_ARMOR)
      {
         obj->value[1] = value;
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[1] = value;
      }
      else
      {
         send_to_char("This can only be used on weapons and armor.\n\r", ch);
      }
      return;
   }
   if (!str_cmp(arg2, "gemslots"))
   {
      if (!can_omodify(ch, obj))
         return;
      if (value < 0 || value > 10)
      {
         send_to_char("Range is 0 to 10.\n\r", ch);
         return;
      }
      obj->imbueslots = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->imbueslots = value;
      return;
   }
   if (!str_cmp(arg2, "sworthrestrict"))
   {
      if (!can_omodify(ch, obj))
         return;
      if (value < 0)
      {
         send_to_char("Range is 0 to 2 billion.\n\r", ch);
         return;
      }
      obj->sworthrestrict = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->sworthrestrict = value;
      return;
   }
   if (!str_cmp(arg2, "cident"))
   {
      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("Sorry only staff can configure Caste mobs.\n\r", ch);
         return;
      }
      if (!can_omodify(ch, obj))
         return;
      if (value > 100)
      {
         send_to_char("Help Castemob for a valid range.\n\r", ch);
         return;
      }
      obj->cident = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->cident = value;
      return;
   }

   if (!str_cmp(arg2, "type"))
   {
      if (!can_omodify(ch, obj))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: oset <object> type <type>\n\r", ch);
         return;
      }
      value = get_otype(argument);
      if (value < 1)
      {
         ch_printf(ch, "Unknown type: %s\n\r", arg3);
         return;
      }
      obj->item_type = (sh_int) value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->item_type = obj->item_type;
      return;
   }

   if (!str_cmp(arg2, "flags"))
   {
      if (!can_omodify(ch, obj))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: oset <object> flags <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_oflag(arg3);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
         {
            if (value == ITEM_PROTOTYPE && get_trust(ch) < sysdata.level_modify_proto && !is_name("protoflag", ch->pcdata->bestowments))
               send_to_char("You cannot change the prototype flag.\n\r", ch);
            else
            {
               xTOGGLE_BIT(obj->extra_flags, value);
               if (value == ITEM_PROTOTYPE)
                  obj->pIndexData->extra_flags = obj->extra_flags;
            }
         }
      }
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->extra_flags = obj->extra_flags;
      return;
   }

   if (!str_cmp(arg2, "wear"))
   {
      if (!can_omodify(ch, obj))
         return;
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: oset <object> wear <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_wflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
            TOGGLE_BIT(obj->wear_flags, 1 << value);
      }

      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->wear_flags = obj->wear_flags;
      return;
   }

   if (!str_cmp(arg2, "level"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->level = value;
      return;
   }

   if (!str_cmp(arg2, "weight"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->weight = adjust_weight(arg3);
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->weight = adjust_weight(arg3);
      return;
   }

   if (!str_cmp(arg2, "cost"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->cost = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->cost = value;
      return;
   }

   if (!str_cmp(arg2, "rent"))
   {
      if (!can_omodify(ch, obj))
         return;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->rent = value;
      else
         send_to_char("Item must have prototype flag to set this value.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "layers"))
   {
      if (!can_omodify(ch, obj))
         return;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->layers = value;
      else
         send_to_char("Item must have prototype flag to set this value.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "timer"))
   {
      if (!can_omodify(ch, obj))
         return;
      obj->timer = value;
      return;
   }

   if (!str_cmp(arg2, "actiondesc"))
   {
      if (strstr(arg3, "%n") || strstr(arg3, "%d") || strstr(arg3, "%l"))
      {
         send_to_char("Illegal characters!\n\r", ch);
         return;
      }
      STRFREE(obj->action_desc);
      obj->action_desc = STRALLOC(arg3);
      if (is_prototype(ch, obj, NULL))
      {
         STRFREE(obj->pIndexData->action_desc);
         obj->pIndexData->action_desc = QUICKLINK(obj->action_desc);
      }
      return;
   }

   if (!str_cmp(arg2, "gem"))
   {
      sh_int loc;
      int bitv;
      int startslot;
      int value2 = -10000;

      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0' || !argument || argument[0] == 0)
      {
         send_to_char("Usage: oset <object> gem <slot1-3> <field> <low value> [hi value]\n\r", ch);
         send_to_char("Usage: oset <object> gem <slot1-3> <spec field> <low value> <hi value>\n\r", ch);
         send_to_char("Usage: oset <object> gem <slot1-3> sworth <value>\n\r", ch);
         send_to_char("Usage: oset <object> gem slotnum <value>\n\r", ch);
         send_to_char("See (HELP GEMSPEC) for a list of special fields.\n\r", ch);
         return;
      }
      if (!str_cmp(arg2, "slot1"))
         startslot = 0;
      else if (!str_cmp(arg2, "slot2"))
         startslot = 4;
      else if (!str_cmp(arg2, "slot3"))
         startslot = 8;
      else if (!str_cmp(arg2, "slotnum"))
      {
         if (atoi(argument) < -1 || atoi(argument) > 5)
         {
            send_to_char("Range is -1 (no slots) to 5.\n\r", ch);
            return;
         }
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[12] = atoi(argument);
         obj->value[12] = atoi(argument);
         send_to_char("Done.\n\r", ch);
         return;
      }            
      else
      {
         send_to_char("Your choices are slot1, slot2, and slot3.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg2);
      if (!str_cmp(arg2, "sworth"))
      {
         if (atoi(argument) < 0)
         {
            send_to_char("Sworth has to be equal to or more than 0.\n\r", ch);
            return;
         }
         if (is_prototype(ch, obj, NULL))
            obj->pIndexData->value[startslot+1] = atoi(argument);
         obj->value[startslot+1] = atoi(argument);
         send_to_char("Done.\n\r", ch);
         return;
      }
         
      loc = get_atype(arg2);
      if (loc < 1)
      {
         if ((loc = get_gemspec(arg2)) < 1)
         {
            ch_printf(ch, "Unknown field: %s\n\r", arg2);
            return;
         }
         else
         {
            argument = one_argument(argument, arg3);
            if (is_prototype(ch, obj, NULL))
            {
               obj->pIndexData->value[startslot] = loc;
               obj->pIndexData->value[startslot+2] = atoi(arg3);
               obj->pIndexData->value[startslot+3] = atoi(argument);
            }
            obj->value[startslot] = loc;
            obj->value[startslot+2] = atoi(arg3);
            obj->value[startslot+3] = atoi(argument);
            send_to_char("Done.\n\r", ch);
            return;
         }            
            
      }
      if (loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL)
      {
         bitv = 0;
         while (argument[0] != '\0')
         {
            argument = one_argument(argument, arg3);
            if (loc == APPLY_AFFECT)
               value = get_aflag(arg3);
            else
               value = get_risflag(arg3);
            if (value < 0 || value > 31)
               ch_printf(ch, "Unknown flag: %s\n\r", arg3);
            else
               SET_BIT(bitv, 1 << value);
         }
         if (!bitv)
            return;
         value = bitv;
      }
      else if (loc == APPLY_EXT_AFFECT)
      {
         value = get_aflag(argument);
         if (value < 0 || value > MAX_BITS)
         {
           ch_printf(ch, "Unknown flag: %s\n\r", argument);
           return;
         }
      }
      else
      {
         argument = one_argument(argument, arg3);
         if ((loc == APPLY_WEARSPELL || loc == APPLY_WEAPONSPELL || loc == APPLY_REMOVESPELL || loc == APPLY_STRIPSN
         ||   loc == APPLY_RECURRINGSPELL) && !is_number(arg3))
         {
            value = bsearch_skill_exact(arg3, gsn_first_spell, gsn_first_skill - 1);
            if (value == -1)
            {
/*		    printf("%s\n\r", arg3);	*/
               send_to_char("Unknown spell name.\n\r", ch);
               return;
            }
         }
         else
         {
            value = atoi(arg3);
            value2 = atoi(argument);
         }
      }
      if (is_prototype(ch, obj, NULL))
      {
         obj->pIndexData->value[startslot] = loc;
         obj->pIndexData->value[startslot+2] = value;
         if (value2 != -10000)
            obj->pIndexData->value[startslot+3] = value2;
         else
            obj->pIndexData->value[startslot+3] = value;
      }
      obj->value[startslot] = loc;
      obj->value[startslot+2] = value;
      if (value2 != -10000)
         obj->value[startslot+3] = value2;
      else
         obj->value[startslot+3] = value;
      send_to_char("Done.\n\r", ch);
      return;
   }

   /* Crash fix and name support by Shaddai */
   if (!str_cmp(arg2, "affect"))
   {
      AFFECT_DATA *paf;
      sh_int loc;
      int bitv;

      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0' || !argument || argument[0] == 0)
      {
         send_to_char("Usage: oset <object> affect <field> <value>\n\r", ch);
         return;
      }
      loc = get_atype(arg2);
      if (loc < 1)
      {
         ch_printf(ch, "Unknown field: %s\n\r", arg2);
         return;
      }
      if (loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL)
      {
         bitv = 0;
         while (argument[0] != '\0')
         {
            argument = one_argument(argument, arg3);
            if (loc == APPLY_AFFECT)
               value = get_aflag(arg3);
            else
               value = get_risflag(arg3);
            if (value < 0 || value > 31)
               ch_printf(ch, "Unknown flag: %s\n\r", arg3);
            else
               SET_BIT(bitv, 1 << value);
         }
         if (!bitv)
            return;
         value = bitv;
      }
      else if (loc == APPLY_EXT_AFFECT)
      {
         value = get_aflag(argument);
         if (value < 0 || value > MAX_BITS)
         {
           ch_printf(ch, "Unknown flag: %s\n\r", argument);
           return;
         }
      }
      else
      {
         one_argument(argument, arg3);
         if ((loc == APPLY_WEARSPELL || loc == APPLY_WEAPONSPELL || loc == APPLY_REMOVESPELL || loc == APPLY_STRIPSN
         ||   loc == APPLY_RECURRINGSPELL) && !is_number(arg3))
         {
            value = bsearch_skill_exact(arg3, gsn_first_spell, gsn_first_skill - 1);
            if (value == -1)
            {
/*		    printf("%s\n\r", arg3);	*/
               send_to_char("Unknown spell name.\n\r", ch);
               return;
            }
         }
         else
            value = atoi(arg3);
      }
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = loc;
      paf->modifier = value;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      if (is_prototype(ch, obj, NULL))
         LINK(paf, obj->pIndexData->first_affect, obj->pIndexData->last_affect, next, prev);
      else
         LINK(paf, obj->first_affect, obj->last_affect, next, prev);
      ++top_affect;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "rmaffect"))
   {
      AFFECT_DATA *paf;
      sh_int loc, count;

      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: oset <object> rmaffect <affect#>\n\r", ch);
         return;
      }
      loc = atoi(argument);
      if (loc < 1)
      {
         send_to_char("Invalid number.\n\r", ch);
         return;
      }

      count = 0;

      if (is_prototype(ch, obj, NULL))
      {
         OBJ_INDEX_DATA *pObjIndex;

         pObjIndex = obj->pIndexData;
         for (paf = pObjIndex->first_affect; paf; paf = paf->next)
         {
            if (++count == loc)
            {
               UNLINK(paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev);
               DISPOSE(paf);
               send_to_char("Removed.\n\r", ch);
               --top_affect;
               return;
            }
         }
         send_to_char("Not found.\n\r", ch);
         return;
      }
      else
      {
         for (paf = obj->first_affect; paf; paf = paf->next)
         {
            if (++count == loc)
            {
               UNLINK(paf, obj->first_affect, obj->last_affect, next, prev);
               DISPOSE(paf);
               send_to_char("Removed.\n\r", ch);
               --top_affect;
               return;
            }
         }
         send_to_char("Not found.\n\r", ch);
         return;
      }
   }

   if (!str_cmp(arg2, "ed"))
   {
      if (!arg3 || arg3[0] == '\0')
      {
         send_to_char("Syntax: oset <object> ed <keywords>\n\r", ch);
         return;
      }
      CHECK_SUBRESTRICTED(ch);
      if (obj->timer)
      {
         send_to_char("It's not safe to edit an extra description on an object with a timer.\n\rTurn it off first.\n\r", ch);
         return;
      }
      if (obj->item_type == ITEM_PAPER)
      {
         send_to_char("You can not add an extra description to a note paper at the moment.\n\r", ch);
         return;
      }
      if (is_prototype(ch, obj, NULL))
         ed = SetOExtraProto(obj->pIndexData, arg3);
      else
         ed = SetOExtra(obj, arg3);
      if (ch->substate == SUB_REPEATCMD)
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      if (lockobj)
         ch->spare_ptr = obj;
      else
         ch->spare_ptr = NULL;
      ch->substate = SUB_OBJ_EXTRA;
      ch->dest_buf = ed;
      start_editing(ch, ed->description);
      editor_desc_printf(ch, "Extra description '%s' on object vnum %d (%s).", arg3, obj->pIndexData->vnum, obj->short_descr);
      return;
   }

   if (!str_cmp(arg2, "rmed"))
   {
      if (!arg3 || arg3[0] == '\0')
      {
         send_to_char("Syntax: oset <object> rmed <keywords>\n\r", ch);
         return;
      }
      if (is_prototype(ch, obj, NULL))
      {
         if (DelOExtraProto(obj->pIndexData, arg3))
            send_to_char("Deleted.\n\r", ch);
         else
            send_to_char("Not found.\n\r", ch);
         return;
      }
      if (DelOExtra(obj, arg3))
         send_to_char("Deleted.\n\r", ch);
      else
         send_to_char("Not found.\n\r", ch);
      return;
   }
   /*
    * save some finger-leather
    */
   if (!str_cmp(arg2, "ris"))
   {
      sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
      do_oset(ch, outbuf);
      sprintf(outbuf, "%s affect immune %s", arg1, arg3);
      do_oset(ch, outbuf);
      sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
      do_oset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "r"))
   {
      sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
      do_oset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "i"))
   {
      sprintf(outbuf, "%s affect immune %s", arg1, arg3);
      do_oset(ch, outbuf);
      return;
   }
   if (!str_cmp(arg2, "s"))
   {
      sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
      do_oset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "ri"))
   {
      sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
      do_oset(ch, outbuf);
      sprintf(outbuf, "%s affect immune %s", arg1, arg3);
      do_oset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "rs"))
   {
      sprintf(outbuf, "%s affect resistant %s", arg1, arg3);
      do_oset(ch, outbuf);
      sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
      do_oset(ch, outbuf);
      return;
   }

   if (!str_cmp(arg2, "is"))
   {
      sprintf(outbuf, "%s affect immune %s", arg1, arg3);
      do_oset(ch, outbuf);
      sprintf(outbuf, "%s affect susceptible %s", arg1, arg3);
      do_oset(ch, outbuf);
      return;
   }

   /*
    * Make it easier to set special object values by name than number
    *       -Thoric
    */
   tmp = -1;
   switch (obj->item_type)
   {
      case ITEM_WEAPON:
         if (!str_cmp(arg2, "weapontype"))
         {
            int x;

            value = -1;
            for (x = 0; x < sizeof(attack_table) / sizeof(attack_table[0]); x++)
               if (!str_cmp(arg3, attack_table[x]))
                  value = x;
            if (value < 0)
            {
               send_to_char("Unknown weapon type.\n\r", ch);
               return;
            }
            tmp = 3;
            break;
         }
         if (!str_cmp(arg2, "condition"))
            tmp = 0;
         break;
      case ITEM_ARMOR:
         if (!str_cmp(arg2, "condition"))
            tmp = 3;
         if (!str_cmp(arg2, "ac"))
            tmp = 1;
         break;
      case ITEM_SALVE:
         break;
      case ITEM_SCROLL:
      case ITEM_POTION:
         if (!str_cmp(arg2, "slevel"))
            tmp = 0;
         if (!str_cmp(arg2, "spell1"))
            tmp = 1;
         if (!str_cmp(arg2, "spell2"))
            tmp = 2;
         if (!str_cmp(arg2, "spell3"))
            tmp = 3;
         if (tmp >= 1 && tmp <= 3)
            value = skill_lookup(arg3);
         break;
      case ITEM_CONTAINER:
         if (!str_cmp(arg2, "capacity"))
            tmp = 0;
         if (!str_cmp(arg2, "cflags"))
            tmp = 1;
         if (!str_cmp(arg2, "key"))
            tmp = 2;
         break;
      case ITEM_SWITCH:
      case ITEM_LEVER:
      case ITEM_PULLCHAIN:
      case ITEM_BUTTON:
         if (!str_cmp(arg2, "tflags"))
         {
            tmp = 0;
            value = get_trigflag(arg3);
         }
         break;
   }
   if (tmp >= 0 && tmp <= 3)
   {
      if (!can_omodify(ch, obj))
         return;
      obj->value[tmp] = value;
      if (is_prototype(ch, obj, NULL))
         obj->pIndexData->value[tmp] = value;
      return;
   }

   /*
    * Generate usage message.
    */
   if (ch->substate == SUB_REPEATCMD)
   {
      ch->substate = SUB_RESTRICTED;
      interpret(ch, origarg);
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_oset;
   }
   else
      do_oset(ch, "");
   return;
}


/*
 * Obsolete Merc room editing routine
 */
void do_rset(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   ROOM_INDEX_DATA *location;
   int value;

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   strcpy(arg3, argument);

   if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0')
   {
      send_to_char("Syntax: rset <location> <field> value\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Field being one of:\n\r", ch);
      send_to_char("  flags sector\n\r", ch);
      return;
   }

   if ((location = find_location(ch, arg1)) == NULL)
   {
      send_to_char("No such location.\n\r", ch);
      return;
   }

   if (!can_rmodify(ch, location))
      return;

   if (!is_number(arg3))
   {
      send_to_char("Value must be numeric.\n\r", ch);
      return;
   }
   value = atoi(arg3);

   /*
    * Set something.
    */
   /*
      if ( !str_cmp( arg2, "flags" ) )
      {  */
   /*
    * Protect from messing up prototype flag
    */
   /*if ( xIS_SET( location->room_flags, ROOM_PROTOTYPE ) )
      proto = TRUE;
      else
      proto = FALSE;
      location->room_flags = value;
      if ( proto )
      xSET_BIT( location->room_flags, ROOM_PROTOTYPE );
      return;
      }     */

   if (!str_cmp(arg2, "sector"))
   {
      location->sector_type = value;
      return;
   }

   /*
    * Generate usage message.
    */
   do_rset(ch, "");
   return;
}

/*
 * Returns value 0 - 9 based on directional text.
 */
 //Use get_dir to call this function
int get_new_dir(char *txt, int type)
{
   int edir;
   char c1, c2;

   if (!str_cmp(txt, "northeast"))
      return DIR_NORTHEAST;
   if (!str_cmp(txt, "northwest"))
      return DIR_NORTHWEST;
   if (!str_cmp(txt, "southeast"))
      return DIR_SOUTHEAST;
   if (!str_cmp(txt, "southwest"))
      return DIR_SOUTHWEST;
   if (!str_cmp(txt, "somewhere"))
      return 10;

   c1 = txt[0];
   if (c1 == '\0')
      return 0;
   c2 = txt[1];
   if (type == 1)
      edir = -1;
   else
      edir = 0;
   switch (c1)
   {
      case 'n':
         switch (c2)
         {
            default:
               edir = 0;
               break; /* north */
            case 'e':
               edir = 6;
               break; /* ne  */
            case 'w':
               edir = 7;
               break; /* nw  */
         }
         break;
      case '0':
         edir = 0;
         break; /* north */
      case 'e':
      case '1':
         edir = 1;
         break; /* east  */
      case 's':
         switch (c2)
         {
            default:
               edir = 2;
               break; /* south */
            case 'e':
               edir = 8;
               break; /* se  */
            case 'w':
               edir = 9;
               break; /* sw  */
         }
         break;
      case '2':
         edir = 2;
         break; /* south */
      case 'w':
      case '3':
         edir = 3;
         break; /* west  */
      case 'u':
      case '4':
         edir = 4;
         break; /* up  */
      case 'd':
      case '5':
         edir = 5;
         break; /* down  */
      case '6':
         edir = 6;
         break; /* ne  */
      case '7':
         edir = 7;
         break; /* nw  */
      case '8':
         edir = 8;
         break; /* se  */
      case '9':
         edir = 9;
         break; /* sw  */
      case '?':
         edir = 10;
         break; /* somewhere */
   }
   return edir;
}
int get_dir(char *txt)
{
   return get_new_dir(txt, -1);
}

char *sprint_reset(CHAR_DATA * ch, RESET_DATA * pReset, sh_int num, bool rlist)
{
   static char buf[MSL];
   char mobname[MSL];
   char roomname[MSL];
   char objname[MSL];
   static ROOM_INDEX_DATA *room;
   static OBJ_INDEX_DATA *obj, *obj2;
   static MOB_INDEX_DATA *mob;
   int rvnum = -1;

   if (ch->in_room)
      rvnum = ch->in_room->vnum;
   if (num == 1)
   {
      room = NULL;
      obj = NULL;
      obj2 = NULL;
      mob = NULL;
   }

   switch (pReset->command)
   {
      default:
         sprintf(buf, "%2d) *** BAD RESET: %c %d %d %d %d ***\n\r", num, pReset->command, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3);
         break;
      case 'M':
         mob = get_mob_index(pReset->arg1);
         room = get_room_index(pReset->arg3);
         if (mob)
            strcpy(mobname, mob->player_name);
         else
            strcpy(mobname, "Mobile: *BAD VNUM*");
         if (room)
            strcpy(roomname, room->name);
         else
            strcpy(roomname, "Room: *BAD VNUM*");
         sprintf(buf, "%2d) %s (%d) -> %s (%d) [%d]\n\r", num, mobname, pReset->arg1, roomname, pReset->arg3, pReset->arg2);
         break;
      case 'E':
         if (!mob)
            strcpy(mobname, "* ERROR: NO MOBILE! *");
         if ((obj = get_obj_index(pReset->arg1)) == NULL)
            strcpy(objname, "Object: *BAD VNUM*");
         else
            strcpy(objname, obj->name);
         sprintf(buf, "%2d) %s (%d) -> %s (%s) [%d]\n\r", num, objname, pReset->arg1, mobname, wear_locs[pReset->arg3], pReset->arg2);
         break;
      case 'H':
         if (pReset->arg1 > 0 && (obj = get_obj_index(pReset->arg1)) == NULL)
            strcpy(objname, "Object: *BAD VNUM*");
         else if (!obj)
            strcpy(objname, "Object: *NULL obj*");
         sprintf(buf, "%2d) Hide %s (%d)\n\r", num, objname, obj ? obj->vnum : pReset->arg1);
         break;
      case 'G':
         if (!mob)
            strcpy(mobname, "* ERROR: NO MOBILE! *");
         if ((obj = get_obj_index(pReset->arg1)) == NULL)
            strcpy(objname, "Object: *BAD VNUM*");
         else
            strcpy(objname, obj->name);
         sprintf(buf, "%2d) %s (%d) -> %s (carry) [%d]\n\r", num, objname, pReset->arg1, mobname, pReset->arg2);
         break;
      case 'O':
         if ((obj = get_obj_index(pReset->arg1)) == NULL)
            strcpy(objname, "Object: *BAD VNUM*");
         else
            strcpy(objname, obj->name);
         room = get_room_index(pReset->arg3);
         if (!room)
            strcpy(roomname, "Room: *BAD VNUM*");
         else
            strcpy(roomname, room->name);
         sprintf(buf, "%2d) (object) %s (%d) -> %s (%d) [%d]\n\r", num, objname, pReset->arg1, roomname, pReset->arg3, pReset->arg2);
         break;
      case 'P':
         if ((obj2 = get_obj_index(pReset->arg1)) == NULL)
            strcpy(objname, "Object1: *BAD VNUM*");
         else
            strcpy(objname, obj2->name);
         if (pReset->arg3 > 0 && (obj = get_obj_index(pReset->arg3)) == NULL)
            strcpy(roomname, "Object2: *BAD VNUM*");
         else if (!obj)
            strcpy(roomname, "Object2: *NULL obj*");
         else
            strcpy(roomname, obj->name);
         sprintf(buf, "%2d) (Put) %s (%d) -> %s (%d) [%d]\n\r", num, objname, pReset->arg1, roomname, obj ? obj->vnum : pReset->arg3, pReset->arg2);
         break;
      case 'D':
         if (pReset->arg2 < 0 || pReset->arg2 > MAX_DIR + 1)
            pReset->arg2 = 0;
         if ((room = get_room_index(pReset->arg1)) == NULL)
         {
            strcpy(roomname, "Room: *BAD VNUM*");
            sprintf(objname, "%s (no exit)", dir_name[pReset->arg2]);
         }
         else
         {
            strcpy(roomname, room->name);
            sprintf(objname, "%s%s", dir_name[pReset->arg2], get_exit(room, pReset->arg2) ? "" : " (NO EXIT!)");
         }
         switch (pReset->arg3)
         {
            default:
               strcpy(mobname, "(* ERROR *)");
               break;
            case 0:
               strcpy(mobname, "Open");
               break;
            case 1:
               strcpy(mobname, "Close");
               break;
            case 2:
               strcpy(mobname, "Close and lock");
               break;
         }
         sprintf(buf, "%2d) %s [%d] the %s [%d] door %s (%d)\n\r", num, mobname, pReset->arg3, objname, pReset->arg2, roomname, pReset->arg1);
         break;
      case 'R':
         if ((room = get_room_index(pReset->arg1)) == NULL)
            strcpy(roomname, "Room: *BAD VNUM*");
         else
            strcpy(roomname, room->name);
         sprintf(buf, "%2d) Randomize exits 0 to %d -> %s (%d)\n\r", num, pReset->arg2, roomname, pReset->arg1);
         break;
      case 'T':
         sprintf(buf, "%2d) TRAP: %d %d %d %d (%s)\n\r",
            num, pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3, flag_string(pReset->extra, trap_flags));
         break;
   }
   if (rlist && (!room || (room && room->vnum != rvnum)))
      return NULL;
   return buf;
}

TOWN_DATA *find_town(int x, int y, int map)
{
   int kdm;
   TOWN_DATA *town;
   
   if (map < 0 || map >= MAP_MAX || x < 0 || x > MAX_X || y < 0 || y > MAX_Y)
      return NULL;

   if ((kdm = kingdom_sector[map][x][y]) <= 1)
      return NULL;
      
   for (town = kingdom_table[kdm]->first_town; town; town = town->next)
   {
      if (in_town_range(town, x, y, map))
         return town;
   }
   return NULL;
}
         
void do_roomstat(CHAR_DATA * ch, char *argument)
{
   TOWN_DATA *town;
   int sector;
   KINGDOM_DATA *kingdom;
   int x;
   int curx, cury;

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (!IN_WILDERNESS(ch))
   {
      send_to_char("Can only use this command in the wilderness.\n\r", ch);
      return;
   }
   curx = ch->coord->x;
   cury = ch->coord->y;
   is_valid_movement(&curx, &cury, argument, NULL);
   sector = map_sector[ch->map][curx][cury];
   if (get_trust(ch) < LEVEL_STAFF)
   {
      if (ch->pcdata->job != 4)
      {
         send_to_char("Huh?\n\r", ch);
         return;
      }
      if (ch->pcdata->hometown < 2)
      {
         send_to_char("You need to belong to a kingdom first.\n\r", ch);
         return;
      }
      if (kingdom_sector[ch->map][curx][cury] != ch->pcdata->hometown)
      {
         send_to_char("You can only do a roomstat in a room that belongs to your kingdom.\n\r", ch);
         return;
      }
      if (!ch->pcdata->town)
      {
          send_to_char("You do not belong to a town.\n\r", ch);
          return;
      }
      town = ch->pcdata->town;
      kingdom = kingdom_table[ch->pcdata->hometown];
   }
   else
   {
      if (kingdom_sector[ch->map][curx][cury] < 2 
      ||  kingdom_sector[ch->map][curx][cury] >= sysdata.max_kingdom)
      {
         send_to_char("This room does not belong to a kingdom.\n\r", ch);
         return;
      }
      kingdom = kingdom_table[kingdom_sector[ch->map][curx][cury]];
      town = find_town(curx, cury, ch->map);
      if (!town)
      {
         send_to_char("This room does not belong to a town???.\n\r", ch);
         bug("do_roomstat:  %s %s belongs to kingdom %d but does not belong to a town.", curx, cury, kingdom->num);
         return;
      }
   }
   
   for (x = 1; x <= 150; x++)
   {
      if (town->roomcoords[x][0] == curx && town->roomcoords[x][1] == cury
      &&  town->roomcoords[x][2] == ch->map)
      {     
         break;
      }
   }
   if (x == 151)
   {
      send_to_char("This room has not been modified yet, first create it with makeroom.\n\r", ch);
      return;
   }
   ch_printf(ch, "&w&cName:  &c&w%s\n\r&w&cKingdom:  &c&w%-15s   &w&cTown:  &c&w%-15s   &w&cMayor:  &c&w%s\n\r", 
       town->roomtitles[x], kingdom->name, town->name, town->mayor);
   ch_printf(ch, "&w&cX:  &c&w%-4d   &w&cY:  &c&w%-4d   &w&cMap:  &c&w%-4d   &w&cSector:  &c&w%s\n\r", 
      ch->coord->x, ch->coord->y, ch->map, sector_message[sector]);
   if (sector == SECT_WALL || sector == SECT_DWALL || sector == SECT_NBWALL || sector == SECT_DOOR
   ||  sector == SECT_CDOOR || sector == SECT_LDOOR)
   {
      ch_printf(ch, "&w&cCondition:  &w&c%d%%\n\r", resource_sector[ch->map][curx][cury]/100);
   }
   ch_printf_color(ch, "&cRoom flags: &c&w%s\n\r", ext_flag_string(&town->roomflags[x], r_flags));
   return;
}

/* Used with functin below this one.  This one creates a room for a carpenter
   class job.  --Xerves 12/99 */
// Changed for outside in the wilderness, ooh yeah!
void do_makeroom(CHAR_DATA * ch, char *argument)
{
   int x;
   int sector;
   char logb[MSL];

   if (IS_NPC(ch))
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (ch->pcdata->job != 4)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   if (argument[0] == '\0')
   {
      send_to_char("Syntax: makeroom yes   (Prevents waste of materials on typos.)\n\r", ch);
      return;
   }
   if (str_cmp(argument, "yes"))
   {
      send_to_char("Syntax: makeroom yes   (Prevents waste of materials on typos.)\n\r", ch);
      return;
   }
   if (kingdom_sector[ch->map][ch->coord->x][ch->coord->y] != ch->pcdata->hometown)
   {
      send_to_char("You can only start a room that belongs to your kingdom!\n\r", ch);
      return;
   }
   if (!in_town_range(ch->pcdata->town, ch->coord->x, ch->coord->y, ch->map))
   {
      send_to_char("You can only start a room that belongs to your town!\n\r", ch);
      return;
   }
   sector = map_sector[ch->map][ch->coord->x][ch->coord->y];

   //note: tundra/hills can be created on, but you cannot modify tundra at all and hills cannot be made into plains
   //Plains check mostly, plains can be made on all of these...
   if (sector == SECT_MOUNTAIN || sector == SECT_WATER_NOSWIM 
   ||  sector == SECT_UNDERWATER || sector == SECT_AIR || sector == SECT_DESERT || sector == SECT_DUNNO
   ||  sector == SECT_OCEANFLOOR || sector == SECT_UNDERGROUND || sector == SECT_ENTER
   ||  sector == SECT_MINEGOLD || sector == SECT_MINEIRON || sector == SECT_RIVER || sector == SECT_ICE
   ||  sector == SECT_SHORE || sector == SECT_OCEAN || sector == SECT_LAVA || sector == SECT_TREE
   ||  sector == SECT_QUICKSAND || sector == SECT_SGOLD || sector == SECT_NGOLD || sector == SECT_SIRON
   ||  sector == SECT_NIRON || sector == SECT_WALL || sector == SECT_GLACIER || sector == SECT_EXIT
   ||  sector == SECT_BRIDGE || sector == SECT_VOID || sector == SECT_STABLE || sector == SECT_FIRE
   ||  sector == SECT_DWALL || sector == SECT_NBWALL || sector == SECT_DOOR || sector == SECT_CDOOR
   ||  sector == SECT_LDOOR || sector == SECT_INSIDE || sector == SECT_HOLD)      
   {
      send_to_char("You cannot start a room on that sector type, help makeroom for more info.\n\r", ch);
      return;
      
   }
   if (ch->pcdata->town->lumber < 4000)
   {
      send_to_char("Your town has less than 4000 lumber, you need more lumber.\n\r", ch);
      return;
   }
   if (ch->pcdata->town->rooms >= ch->pcdata->town->maxsize)
   {
      ch_printf(ch, "You can only have %d rooms because of your size\n\r", ch->pcdata->town->maxsize);
      return;
   }
   for (x = 1; x <= 150; x++)
   {
      if (ch->pcdata->town->roomcoords[x][0] == ch->coord->x && ch->pcdata->town->roomcoords[x][1] == ch->coord->y &&
          ch->pcdata->town->roomcoords[x][2] == ch->map)
      {     
         send_to_char("This room has already be created.\n\r", ch);
         return;
      }
   }
   for (x = 1; x <= 150; x++)
   {
      if (ch->pcdata->town->roomcoords[x][0] == 0)
      {     
         break;
      }
   }
   if (x == 151)
   {
      send_to_char("You have maxed out your kingdom and the availability of rooms.\n\r", ch);
      return;
   }
   ch->pcdata->town->lumber -= 4000;
   ch->pcdata->town->roomcoords[x][0] = ch->coord->x;
   ch->pcdata->town->roomcoords[x][1] = ch->coord->y;
   ch->pcdata->town->roomcoords[x][2] = ch->map;
   ch->pcdata->town->rooms++;
   sprintf(logb, "A Room empty of any design.");
   sprintf(ch->pcdata->town->roomtitles[x], logb);
   ch_printf(ch, "A room was created for you at coords %d %d\n\r", ch->coord->x, ch->coord->y);
   sprintf(logb, "%s created a room: coords %d %d", PERS_KINGDOM(ch, ch->pcdata->hometown), ch->coord->x, ch->coord->y);
   write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_CREATE_ROOM);
   write_kingdom_file(ch->pcdata->hometown);
   return;
}

//Find out problems when creating a link 0 is ok 1 is Overlap 2 is not possible
//Such as creating an exit south to a room north of your position
sh_int room_overlap(CHAR_DATA * ch, int edir, int evnum, int output)
{
   EXIT_DATA *pexit;
   ROOM_INDEX_DATA *curroom;
   ROOM_INDEX_DATA *holdroom;
   int currx, curry;
   int holdx, holdy;
   int start, finish, x, y;
   int foundnew;
   int type = 0; //If there is a problem, what kind
   sh_int roomcoords[101][101]; //Decent enough room to keep track of coords
   sh_int mark[501]; //Mark if the room has been found

   for (x = 1; x <= 500; x++)
      mark[x] = 0;
   start = ch->in_room->area->low_r_vnum;
   finish = ch->in_room->area->hi_r_vnum;
   roomcoords[50][50] = ch->in_room->vnum;
   curroom = ch->in_room;
   currx = 50;
   curry = 50;
   holdroom = curroom;
   holdx = currx;
   holdy = curry;
   for (x = 1; x <= 100; x++)
      for (y = 1; y <= 100; y++)
         roomcoords[x][y] = 0;
   mark[ch->in_room->vnum - start + 1] = 1;
   roomcoords[50][50] = ch->in_room->vnum;

   for (;;)
   {
      foundnew = 0;
      for (pexit = curroom->first_exit; pexit; pexit = pexit->next)
      {
         if (pexit->to_room->area == curroom->area && mark[pexit->to_room->vnum - start + 1] == 0)
         {
            holdx = get_x(currx, pexit->vdir);
            holdy = get_y(curry, pexit->vdir);
            holdroom = pexit->to_room;
            roomcoords[holdx][holdy] = holdroom->vnum;
            mark[holdroom->vnum - start + 1] = 1;
            foundnew = 1;
         }
      }
      // If something found, take the last values and use those, if not look for something unused
      if (foundnew == 1)
      {
         currx = holdx;
         curry = holdy;
         curroom = holdroom;
         continue;
      }
      else
      {
         for (x = 1; x <= 100; x++)
         {
            for (y = 1; y <= 100; y++)
            {
               if (roomcoords[x][y] != 0)
               {
                  curroom = get_room_index(roomcoords[x][y]);
                  for (pexit = curroom->first_exit; pexit; pexit = pexit->next)
                  {
                     if (pexit->to_room->area == curroom->area && mark[pexit->to_room->vnum - start + 1] == 0)
                     {
                        currx = get_x(x, pexit->vdir);
                        curry = get_y(y, pexit->vdir);
                        curroom = pexit->to_room;
                        roomcoords[currx][curry] = curroom->vnum;
                        mark[curroom->vnum - start + 1] = 1;
                        foundnew = 1;
                        break;
                     }
                  }
               }
               if (foundnew == 1)
                  break;
            }
            if (foundnew == 1)
               break;
         }
      }
      if (foundnew == 0)
         break;
   }
   foundnew = 0;
   currx = get_x(50, edir);
   curry = get_y(50, edir);
   if (mark[evnum - start + 1] == 1)
      foundnew = 1;

   if (output == 1)
   {
      for (y = 1; y <= 100; y++)
      {
         send_to_char("\n\r", ch);
         for (x = 1; x <= 100; x++)
         {
            if (roomcoords[x][y] == 0)
               send_to_char(" ", ch);
            else
               send_to_char("+", ch);
         }
      }
   }

   //Direction is empty, make sure the room pointed to is empty too
   if (roomcoords[currx][curry] == 0)
   {
      if (foundnew == 0)
         type = 0;
      else
         type = 2;
   }
   else
   {
      if (roomcoords[currx][curry] == evnum) //A room exists, check to make sure it is the room being pointed at
         type = 0;

      else
         type = 1;
   }

   return type;
}

//Shows freerooms in your building area
void do_freerooms(CHAR_DATA *ch, char *argument)
{
   return;
}

int parse_sector(char *sector)
{
   if (!str_cmp(sector, "inside"))
      return 0;
   if (!str_cmp(sector, "city"))
      return 1;
   if (!str_cmp(sector, "waterswim"))
      return 6;
   if (!str_cmp(sector, "waternoswim"))
      return 7;
   if (!str_cmp(sector, "road"))
      return 14;
   if (!str_cmp(sector, "wall"))
      return 40;
   if (!str_cmp(sector, "path"))
      return 44;
   if (!str_cmp(sector, "pave"))
      return 46;
   if (!str_cmp(sector, "bridge"))
      return 47;
   if (!str_cmp(sector, "door"))
      return 57;
   return -1;
}


void do_build(CHAR_DATA *ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char logb[MSL];
   DOOR_DATA *ddata;
   int rvalue;
   int x, y;
   int z, value;
   int dcnt, dx, dy;
   int ddx, zz;
   
   set_char_color(AT_PLAIN, ch);
   if (!ch->desc)
   {
      send_to_char("You have no descriptor.\n\r", ch);
      return;
   }
   if (check_npc(ch))
      return;
      
   if (!ch->pcdata->town)
   {
      send_to_char("You have to belong to a town to use this command.\n\r", ch);
      return;
   }        
   
   smash_tilde(argument);
   argument = one_argument(argument, arg);
   argument = one_argument(argument, arg2);
   
   if (arg[0] == '\0' || !str_cmp(arg, "?"))
   {
      if (ch->pcdata->job == 4)
      {
         send_to_char("Syntax: build <direction/here> <field> value\n\r", ch);
         send_to_char("Build Field being one of:\n\r", ch);
         send_to_char("  name sector flags\n\r", ch);
         send_to_char("  The options in the line above have helpfiles, help <option>\n\r", ch);
         return;
      }
      else
      {
         send_to_char("Huh?\n\r", ch);
         return;
      }
   }
   if (!IS_ONMAP_FLAG(ch))
   {
      send_to_char("You have to be in the Wilderness to use this command.\n\r", ch);
      return;
   }
   if (ch->pcdata->job != 4)
   {
      do_build(ch, "");
      return;
   }
   x = ch->coord->x;
   y = ch->coord->y;
   if (str_cmp(arg, "here"))
   {
      if (!is_valid_movement(&x, &y, arg, ch))
         return;
   }
   if (!can_build(ch, x, y))
   {
      send_to_char("You cannot build in that room, you might need to do a makeroom first.\n\r", ch);
      return;
   }
   if (!str_cmp(arg2, "name"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Set the room name.  A very brief single line room description.\n\r", ch);
         send_to_char("Usage: build <direction> name <Room summary>\n\r", ch);
         return;
      }
      if (strlen(argument) >= 80)
      {
         send_to_char("Need to keep the titles to 79 characters or less.\n\r", ch);
         return;
      }
      for (z = 1; z <= 150; z++)
      {
         if (ch->pcdata->town->roomcoords[z][0] == x && ch->pcdata->town->roomcoords[z][1] == y 
         &&  ch->pcdata->town->roomcoords[z][2] == ch->map)
         {
            sprintf(ch->pcdata->town->roomtitles[z], argument);
            break;
         }
      }
      if (z == 151)
      {
         send_to_char("There has been an error, tell an immortal.\n\r", ch);
         bug("do_build:  %s tried to edit a room name at %d %d and the coords could not be found.", ch->name, x, y);
         return;
      }
      send_to_char("Done.\n\r", ch);
      write_kingdom_file(ch->pcdata->hometown);
      sprintf(logb, "%s changed the room name at %dx %dy", PERS_KINGDOM(ch, ch->pcdata->hometown), x, y);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_EDIT_ROOM);
      return;
   }
   if (!str_cmp(arg2, "flags"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Toggle the room flags.\n\r", ch);
         send_to_char("Usage: build <direction> flags <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg2);
         value = get_rflag(arg2);
         if (value < 0 || value > MAX_BITS)
         {
            ch_printf(ch, "Unknown flag: %s\n\r", arg2);
            continue;
         }
         if (value == ROOM_NO_MAGIC || value == ROOM_PRIVATE || value == ROOM_SOLITARY || ROOM_NO_MOB
            || value == ROOM_NODROPALL || value == ROOM_SILENCE || value == ROOM_NODROP || value == ROOM_NO_SUMMON || value == ROOM_NO_ASTRAL)
         {
            rvalue = resource_value(value);
            if ((enough_resources(ch, ch->pcdata->hometown, rvalue, 0)) == FALSE)
               continue;
            for (z = 1; z <= 150; z++)
            {
               if (ch->pcdata->town->roomcoords[z][0] == x && ch->pcdata->town->roomcoords[z][1] == y 
               &&  ch->pcdata->town->roomcoords[z][2] == ch->map)
               {
                  xTOGGLE_BIT(ch->pcdata->town->roomflags[z], value);
                  break;
               }
            }
            if (z == 151)
            {
               send_to_char("There has been an error, tell an immortal.\n\r", ch);
               bug("do_build:  %s tried to edit a room flag at %d %d and the coords could not be found.", ch->name, x, y);
               return;
            }
            send_to_char("Done.\n\r", ch);
         }
         else
            ch_printf(ch, "Unused flag: %s\n\r", arg2);
      }
      write_kingdom_file(ch->pcdata->hometown);
      sprintf(logb, "%s changed a flag(s) at room %d %d", PERS_KINGDOM(ch, ch->pcdata->hometown), x, y);
      write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_EDIT_ROOM);
      return;
   }
   if (!str_cmp(arg2, "sector"))
   {  
      int sector; 
      
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Set the sector type.\n\r", ch);
         send_to_char("Usage: build <direction> sector <value>\n\r", ch);
         send_to_char("Sectors: inside city waterswim waternoswim wall road path bridge pave", ch);
         send_to_char("\n\r         (type the words not the value)\n\r", ch);
         return;
      }
      value = parse_sector(argument);
      if (value < 0 || value >= SECT_MAX)
      {
         send_to_char("That is not a valid sectortype, type build <direction> sector for a list.\n\r.", ch);
      }
      else
      {
         //Seems a little redundant with build commands for editing terrain, but oh well, rofl
         if (value == SECT_INSIDE || value == SECT_CITY || value == SECT_WATER_SWIM || value == SECT_WATER_NOSWIM 
         || value == SECT_WALL || value == SECT_ROAD || value == SECT_BRIDGE || value == SECT_PAVE || value == SECT_PATH
         || value == SECT_DOOR)
         {
            sector = map_sector[ch->map][x][y];
            if (value != SECT_WALL && value != SECT_ROAD && value != SECT_PATH && value != SECT_BRIDGE
            &&  value != SECT_DOOR)
            {
               if (sector != SECT_FIELD && sector != SECT_NCORN && sector != SECT_NGRAIN && sector != SECT_NTREE
               &&  sector != SECT_SWAMP && sector != SECT_JUNGLE && sector != SECT_BURNT 
               &&  sector != SECT_PLAINS && sector != SECT_HILLS && sector != SECT_ROAD && sector != SECT_PATH
               &&  sector != SECT_CITY && sector != SECT_PAVE && sector != SECT_WATER_SWIM
               &&  sector != SECT_WATER_NOSWIM && sector != SECT_BRIDGE && sector != SECT_INSIDE
               &&  sector != SECT_HOLD)
               {
                  send_to_char("Cannot build that sectortype here, help buildsectors for valid sectortypes.\n\r", ch);
                  return;
               }
               if (sector == SECT_INSIDE)
               {
                  if (ch->pcdata->town->usedpoint[ch->coord->x - ch->pcdata->town->startx+30][ch->coord->y - ch->pcdata->town->starty+30] == 1)
                  {
                     send_to_char("The sector you want to remove is part of a roof, destroy the roof first!\n\r", ch);
                     return;
                  }
               }
               if (sector == SECT_HOLD && value != SECT_PAVE)
               {
                  send_to_char("You can only replace silos with the pave sector.\n\r", ch);
                  return;
               }
               if (value == SECT_PAVE && sector == SECT_HOLD)
               {
                  ch->pcdata->town->hold -= ch->pcdata->town->bincoords[x - ch->pcdata->town->startx+30][y - ch->pcdata->town->starty+30];
                  ch->pcdata->town->bincoords[x - ch->pcdata->town->startx+30][y - ch->pcdata->town->starty+30] = 0;
                  if (get_current_hold(ch->pcdata->town) > ch->pcdata->town->hold)
                  {
                     int rcnt = 0;
                     int chold = get_current_hold(ch->pcdata->town);
                     
                     for (;;)
                     {
                        if (rcnt == 0)
                        {
                           if (ch->pcdata->town->corn > 0)
                           {
                              ch->pcdata->town->corn--;
                              chold--;
                           }
                        }
                        if (rcnt == 1)
                        {
                           if (ch->pcdata->town->grain > 0)
                           {
                              ch->pcdata->town->grain--;
                              chold--;
                           }
                        }
                        if (rcnt == 2)
                        {
                           if (ch->pcdata->town->iron > 0)
                           {
                              ch->pcdata->town->iron--;
                              chold--;
                           }
                        }
                        if (rcnt == 3)
                        {
                           if (ch->pcdata->town->gold > 0)
                           {
                              ch->pcdata->town->gold--;
                              chold--;
                           }
                        }
                        if (rcnt == 4)
                        {
                           if (ch->pcdata->town->lumber > 0)
                           {
                              ch->pcdata->town->lumber--;
                              chold--;
                           }
                        }
                        if (rcnt == 5)
                        {
                           if (ch->pcdata->town->stone > 0)
                           {
                              ch->pcdata->town->stone--;
                              chold--;
                           }
                        }
                        if (rcnt == 6)
                        {
                           if (ch->pcdata->town->coins > 99)
                           {
                              ch->pcdata->town->coins-=100;
                              chold--;
                           }
                        }  
                        if (rcnt != 6)
                           rcnt++;
                        else
                           rcnt = 0;
                           
                        if (chold == ch->pcdata->town->hold)
                           break;                           
                     }
                  }
               }
               write_kingdom_file(ch->pcdata->hometown);
            }  
            if (value == SECT_DOOR)
            {
               if (sector != SECT_WALL && sector != SECT_DWALL && sector != SECT_NBWALL)
               {
                  send_to_char("You can only build a door into a wall.\n\r", ch);
                  return;
               }
               send_to_char("REMINDER:  You can only have 100 doors in a single town, so use them wisely.\n\r", ch);
               //ok need to create the door data so it is stored in the town.
               
               for (z = 0; z <= 99; z++)
               {
                  if (ch->pcdata->town->doorstate[4][z] == 0)
                  {
                     ch->pcdata->town->doorstate[4][z] = ++ch->pcdata->town->max_dvalue; //sets the door unique value;
                     ch->pcdata->town->doorstate[0][z] = 0; //open
                     ch->pcdata->town->doorstate[5][z] = x;
                     ch->pcdata->town->doorstate[6][z] = y;
                     ch->pcdata->town->doorstate[7][z] = ch->map;
                     break;
                  }
               }
               if (z == 100)
               {
                  send_to_char("Your town is full, you cannot create any more doors.\n\r", ch);
                  return;
               }
               //master door check time....
               for (dcnt = 1; dcnt <= 8; dcnt++)
               {
                  dx = x;
                  dy = y;
         
                  if (dcnt == 1 || dcnt == 5 || dcnt == 7) //east
                     dx = x+1;
                  if (dcnt == 2 || dcnt == 7 || dcnt == 8) //south
                     dy = y+1;
                  if (dcnt == 3 || dcnt == 6 || dcnt == 8) //west
                     dx = x-1;
                  if (dcnt == 4 || dcnt == 5 || dcnt == 6) //north
                     dy = y-1;  
                              
                  if (map_sector[ch->map][dx][dy] != SECT_INSIDE && map_sector[ch->map][dx][dy] != SECT_WALL
                  &&  map_sector[ch->map][dx][dy] != SECT_DWALL && map_sector[ch->map][dx][dy] != SECT_NBWALL
                  &&  map_sector[ch->map][dx][dy] != SECT_DOOR && map_sector[ch->map][dx][dy] != SECT_CDOOR
                  &&  map_sector[ch->map][dx][dy] != SECT_LDOOR)
                  {
                     ch->pcdata->town->doorstate[3][z] = 1; //Master door, goes outside..
                     break;
                  }
               }
               if (dcnt == 9)
               {
                  ch->pcdata->town->doorstate[3][z] = 0; //Not a master door
               }
               for (dcnt = 1; dcnt <= 8; dcnt++)
               {
                  dx = x;
                  dy = y;
         
                  if (dcnt == 1 || dcnt == 5 || dcnt == 7) //east
                     dx = x+1;
                  if (dcnt == 2 || dcnt == 7 || dcnt == 8) //south
                     dy = y+1;
                  if (dcnt == 3 || dcnt == 6 || dcnt == 8) //west
                     dx = x-1;
                  if (dcnt == 4 || dcnt == 5 || dcnt == 6) //north
                     dy = y-1;  
                              
                  if (map_sector[ch->map][dx][dy] == SECT_INSIDE)
                  {
                     for (ddata = ch->pcdata->town->first_doorlist->first_door; ddata; ddata = ddata->next)
                     {
                        for (zz = 0; zz <= MAX_HPOINTS-1; zz++)
                        {
                           if (ddata->roomcoordx[zz] == dx && ddata->roomcoordy[zz] == dy && ddata->roomcoordmap[zz] == ch->map)
                           {
                              for (ddx = 0; ddx <= 9; ddx++)
                              {
                                 if (ddata->doorvalue[ddx] == 0)
                                 {
                                    ddata->doorvalue[ddx] = ch->pcdata->town->doorstate[4][z];
                                    break;
                                 }
                              }
                              if (ddx == 10)
                              {
                                 send_to_char("The door would make one of the rooms have 10 doors, not added to the room!\n\r", ch);
                              }
                              break;
                           }
                        }
                     }
                  }
               }
               update_indoor_status(z, ch, ch->pcdata->town, ch->pcdata->town->doorstate[5][z], ch->pcdata->town->doorstate[6][z], ch->map, 1);
               write_kingdom_file(ch->pcdata->hometown);
            }
            if (value == SECT_WALL)
            {
               if (sector != SECT_PATH && sector != SECT_PLAINS && sector != SECT_HILLS
               &&  sector != SECT_ROAD && sector != SECT_CITY && sector != SECT_WATER_SWIM && sector != SECT_PAVE)
               {
                  send_to_char("Cannot build a wall here, help buildwall for valid sectortypes.\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->stone < 1000)
               {
                  send_to_char("It takes 1000 units of stone to build a wall.\n\r", ch);
                  return;
               }
               ch->pcdata->town->stone -= 1000;
            }
            if (value == SECT_ROAD)
            {
               if (sector != SECT_PATH && sector != SECT_PLAINS && sector != SECT_HILLS
               &&  sector != SECT_PAVE)
               {
                  send_to_char("Cannot build a road here, help layroad for valid sectortypes.\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->stone < 700)
               {
                  send_to_char("It takes 700 units of stone to build a road.\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->coins < 1000)
               {
                  send_to_char("It takes 1000 gold coins to build a road.\n\r", ch);
                  return;
               }
               ch->pcdata->town->stone -= 700;
               ch->pcdata->town->coins -= 1000;
            }   
            if (value == SECT_PATH)
            {
               if (sector != SECT_FIELD && sector != SECT_NCORN && sector != SECT_NGRAIN && sector != SECT_NTREE
               &&  sector != SECT_SWAMP && sector != SECT_JUNGLE && sector != SECT_BURNT 
               &&  sector != SECT_PLAINS && sector != SECT_HILLS && sector != SECT_PAVE)
               {
                  send_to_char("Cannot build a path here, help cutpath for valid sectortypes.\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->stone < 400)
               {
                  send_to_char("It takes 400 units of stone to build a path.\n\r", ch);
                  return;
               }
               ch->pcdata->town->stone -= 400;
            }
            if (value == SECT_BRIDGE)
            {
               if (sector != SECT_WATER_NOSWIM && sector != SECT_UNDERWATER && sector != SECT_RIVER)
               {
                  send_to_char("Cannot build a bridge here, help layroad for valid sectortypes.\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->stone < 1000)
               {
                  send_to_char("It takes 1000 units of stone to build a bridge.\n\r", ch);
                  return;
               }
               if (ch->pcdata->town->coins < 10000)
               {
                  send_to_char("It takes 10000 gold coins to build a bridge.\n\r", ch);
                  return;
               }
               ch->pcdata->town->stone -= 1000;
               ch->pcdata->town->coins -= 10000;
            }
            map_sector[ch->map][x][y] = value;
            if (value != SECT_DOOR) //a door is a wall, so we don't want to remove that health
               resource_sector[ch->map][x][y] = 0;
            if (value == SECT_WALL) //health is recorded via resources
               resource_sector[ch->map][x][y] = 10000;
            if (value == SECT_WALL || value == SECT_INSIDE)
            {
               update_master_stat(ch, x, y);
               write_kingdom_file(ch->pcdata->hometown);
            }
               
            save_map("solan", 0);
            send_to_char("Done.\n\r", ch);
            sprintf(logb, "%s changed the sector in %dx %dy to %d", PERS_KINGDOM(ch, ch->pcdata->hometown), x, y, value);
            write_kingdom_logfile(ch->pcdata->hometown, logb, KLOG_EDIT_ROOM);
         }
         else
         {
            send_to_char("That is an unused sectortype, cannot use it.\n\r", ch);
         }
      }
      return;
   }
   do_build(ch, "");
   return;
}
void do_redit(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char buf[MSL];
   ROOM_INDEX_DATA *location, *tmp;
   EXTRA_DESCR_DATA *ed;
   EXIT_DATA *xit, *texit;
   int value;
   int edir, ekey, evnum;
   char *origarg = argument;

   set_char_color(AT_PLAIN, ch);
   if (!ch->desc)
   {
      send_to_char("You have no descriptor.\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;
      case SUB_ROOM_DESC:
         location = ch->dest_buf;
         if (!location)
         {
            bug("redit: sub_room_desc: NULL ch->dest_buf", 0);
            location = ch->in_room;
         }
         STRFREE(location->description);
         location->description = copy_buffer(ch);
         stop_editing(ch);
         ch->substate = ch->tempnum;
         return;
      case SUB_ROOM_EXTRA:
         ed = ch->dest_buf;
         if (!ed)
         {
            bug("redit: sub_room_extra: NULL ch->dest_buf", 0);
            stop_editing(ch);
            return;
         }
         STRFREE(ed->description);
         ed->description = copy_buffer(ch);
         stop_editing(ch);
         ch->substate = ch->tempnum;
         return;
   }

   location = ch->in_room;

   smash_tilde(argument);
   argument = one_argument(argument, arg);
   if (ch->substate == SUB_REPEATCMD)
   {
      if (arg[0] == '\0')
      {
         do_rstat(ch, "");
         return;
      }
      if (!str_cmp(arg, "done") || !str_cmp(arg, "off"))
      {
         send_to_char("Redit mode off.\n\r", ch);
         if (ch->pcdata && ch->pcdata->subprompt)
         {
            STRFREE(ch->pcdata->subprompt);
            ch->pcdata->subprompt = NULL;
         }
         ch->substate = SUB_NONE;
         if (xIS_SET(ch->act, PLR_REDIT)) /* redit flags, Crash no like -- Xerves */
            xREMOVE_BIT(ch->act, PLR_REDIT);
         return;
      }
   }
   if (arg[0] == '\0' || !str_cmp(arg, "?"))
   {
      if (ch->substate == SUB_REPEATCMD)
         send_to_char("Syntax: <field> value\n\r", ch);
      else
         send_to_char("Syntax: redit <field> value\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Field being one of:\n\r", ch);
      send_to_char("  name desc ed rmed\n\r", ch);
      send_to_char("  exit bexit exdesc exflags exname exkey excoord\n\r", ch);
      send_to_char("  flags nodemana sector teledelay televnum tunnel resource quadrant\n\r", ch);
      send_to_char("  rlist exdistance pulltype pull push\n\r", ch);
      return;
   }

   if (!can_rmodify(ch, location))
      return;

   if (!str_cmp(arg, "on"))
   {
      if ((xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_MSET)))
      {
         send_to_char("Trying to turn on, while already on another, BLAH!\n\r", ch);
         return;
      }
      send_to_char("Redit mode on.\n\r", ch);
      ch->substate = SUB_REPEATCMD;
      if (ch->pcdata)
      {
         if (ch->pcdata->subprompt)
            STRFREE(ch->pcdata->subprompt);
         ch->pcdata->subprompt = STRALLOC("<&CRedit &W#%r&w> %i");
         if (!xIS_SET(ch->act, PLR_REDIT)) /* redit flags, Crash no like -- Xerves */
            xSET_BIT(ch->act, PLR_REDIT);
      }
      return;
   }

   if (!str_cmp(arg, "name"))
   {
      if (argument[0] == '\0')
      {
         send_to_char("Set the room name.  A very brief single line room description.\n\r", ch);
         send_to_char("Usage: redit name <Room summary>\n\r", ch);
         return;
      }
      STRFREE(location->name);
      location->name = STRALLOC(argument);
      return;
   }

   if (!str_cmp(arg, "desc"))
   {
      if (ch->substate == SUB_REPEATCMD)
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_ROOM_DESC;
      ch->dest_buf = location;
      start_editing(ch, location->description);
      editor_desc_printf(ch, "Description of room vnum %d (%s).", location->vnum, location->name);
      return;
   }

   if (!str_cmp(arg, "tunnel"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Set the maximum characters allowed in the room at one time. (0 = unlimited).\n\r", ch);
         send_to_char("Usage: redit tunnel <value>\n\r", ch);
         return;
      }
      location->tunnel = URANGE(0, atoi(argument), 1000);
      send_to_char("Done.\n\r", ch);
      return;
   }

   /* Crash fix and name support by Shaddai */
   if (!str_cmp(arg, "affect"))
   {
      AFFECT_DATA *paf;
      sh_int loc;
      int bitv;

      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0' || !argument || argument[0] == 0)
      {
         send_to_char("Usage: redit affect <field> <value>\n\r", ch);
         return;
      }
      loc = get_atype(arg2);
      if (loc < 1)
      {
         ch_printf(ch, "Unknown field: %s\n\r", arg2);
         return;
      }
      if (loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL)
      {
         bitv = 0;
         while (argument[0] != '\0')
         {
            argument = one_argument(argument, arg3);
            if (loc == APPLY_AFFECT)
               value = get_aflag(arg3);
            else
               value = get_risflag(arg3);
            if (value < 0 || value > 31)
               ch_printf(ch, "Unknown flag: %s\n\r", arg3);
            else
               SET_BIT(bitv, 1 << value);
         }
         if (!bitv)
            return;
         value = bitv;
      }
      else
      {
         one_argument(argument, arg3);
         if ((loc == APPLY_WEARSPELL || loc == APPLY_WEAPONSPELL || loc == APPLY_REMOVESPELL || loc == APPLY_STRIPSN
         ||   loc == APPLY_RECURRINGSPELL) && !is_number(arg3))
         {
            value = bsearch_skill_exact(arg3, gsn_first_spell, gsn_first_skill - 1);
            if (value == -1)
            {
/*		    printf("%s\n\r", arg3);	*/
               send_to_char("Unknown spell name.\n\r", ch);
               return;
            }
         }
         else
            value = atoi(arg3);
      }
      CREATE(paf, AFFECT_DATA, 1);
      paf->type = -1;
      paf->duration = -1;
      paf->location = loc;
      paf->modifier = value;
      xCLEAR_BITS(paf->bitvector);
      paf->next = NULL;
      LINK(paf, location->first_affect, location->last_affect, next, prev);
      ++top_affect;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "rmaffect"))
   {
      AFFECT_DATA *paf;
      sh_int loc, count;

      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: redit rmaffect <affect#>\n\r", ch);
         return;
      }
      loc = atoi(argument);
      if (loc < 1)
      {
         send_to_char("Invalid number.\n\r", ch);
         return;
      }

      count = 0;

      for (paf = location->first_affect; paf; paf = paf->next)
      {
         if (++count == loc)
         {
            UNLINK(paf, location->first_affect, location->last_affect, next, prev);
            DISPOSE(paf);
            send_to_char("Removed.\n\r", ch);
            --top_affect;
            return;
         }
      }
      send_to_char("Not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "ed"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Create an extra description.\n\r", ch);
         send_to_char("You must supply keyword(s).\n\r", ch);
         return;
      }
      CHECK_SUBRESTRICTED(ch);
      ed = SetRExtra(location, argument);
      if (ch->substate == SUB_REPEATCMD)
         ch->tempnum = SUB_REPEATCMD;
      else
         ch->tempnum = SUB_NONE;
      ch->substate = SUB_ROOM_EXTRA;
      ch->dest_buf = ed;
      start_editing(ch, ed->description);
      editor_desc_printf(ch, "Extra description '%s' on room %d (%s).", argument, location->vnum, location->name);
      return;
   }

   if (!str_cmp(arg, "rmed"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Remove an extra description.\n\r", ch);
         send_to_char("You must supply keyword(s).\n\r", ch);
         return;
      }
      if (DelRExtra(location, argument))
         send_to_char("Deleted.\n\r", ch);
      else
         send_to_char("Not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "rlist"))
   {
      RESET_DATA *pReset;
      char *bptr;
      AREA_DATA *tarea;
      sh_int num;

      tarea = location->area;
      if (!tarea->first_reset)
      {
         send_to_char("This area has no resets to list.\n\r", ch);
         return;
      }
      num = 0;
      for (pReset = tarea->first_reset; pReset; pReset = pReset->next)
      {
         num++;
         if ((bptr = sprint_reset(ch, pReset, num, TRUE)) == NULL)
            continue;
         send_to_char(bptr, ch);
      }
      return;
   }
   if (!str_cmp(arg, "flags"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Toggle the room flags.\n\r", ch);
         send_to_char("Usage: redit flags <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg2);
         value = get_rflag(arg2);
         if (value < 0 || value > MAX_BITS)
            ch_printf(ch, "Unknown flag: %s\n\r", arg2);
         else
         {
            if (value == ROOM_PROTOTYPE && get_trust(ch) < sysdata.level_modify_proto) /* Tracker1 */
               send_to_char("You cannot change the prototype flag.\n\r", ch);
            else
               xTOGGLE_BIT(location->room_flags, value);
         }
      }
      return;
   }

   if (!str_cmp(arg, "teledelay"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Set the delay of the teleport. (0 = off).\n\r", ch);
         send_to_char("Usage: redit teledelay <value>\n\r", ch);
         return;
      }
      location->tele_delay = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg, "nodemana"))
   {
      if (!argument || argument[0] == '\0')
      {
         if(ch->level == LEVEL_PC)
         {
            send_to_char("You can not set this value.\r\n", ch);
            return;
         }
         send_to_char("Set the amount of mana in the node.\n\r", ch);
         send_to_char("Usage: redit nodemana <value>\n\r", ch);
         return;
      }
      if(!xIS_SET(location->room_flags, ROOM_MANANODE))
      {
        send_to_char("This room is not a node, you can not set it's mana level.\r\n", ch);
        return;
      }
      if(atoi(argument) < 0)
      {
        send_to_char("You can not set the node's mana level below zero.\r\n", ch);
        return;
      }
      location->node_mana = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   
   if (!str_cmp(arg, "televnum"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Set the vnum of the room to teleport to.\n\r", ch);
         send_to_char("Usage: redit televnum <vnum>\n\r", ch);
         return;
      }
      location->tele_vnum = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "quadrant"))
   {
      if (!IS_STAFF(ch))
      {
         send_to_char("For Staff use only, sorry.\n\r", ch);
         return;
      }
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: redit quadrant <number>\n\r", ch);
         return;
      }
      if (atoi(argument) < 0 || atoi(argument) > 300)
      {
         send_to_char("Range is 0 to 300\n\r", ch);
         return;
      }
      location->quad = atoi(argument);
      send_to_char("Done.\n\r", ch);

      return;
   }
   if (!str_cmp(arg, "resource"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: redit resource <amount>\n\r", ch);
         return;
      }
      if (atoi(argument) < 1 || atoi(argument) > 30000)
      {
         send_to_char("Range is 1 to 30000\n\r", ch);
         return;
      }
      location->resource = atoi(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "sector"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Set the sector type.\n\r", ch);
         send_to_char("Usage: redit sector <value>\n\r", ch);
         return;
      }
      location->sector_type = atoi(argument);
      if (location->sector_type < 0 || location->sector_type >= SECT_MAX)
      {
         location->sector_type = 1;
         send_to_char("Out of range\n\r.", ch);
      }
      else
         send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "exkey"))
   {
      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);
      if (arg2[0] == '\0' || arg3[0] == '\0')
      {
         send_to_char("Usage: redit exkey <dir> <key vnum>\n\r", ch);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      value = atoi(arg3);
      if (!xit)
      {
         send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
         return;
      }
      xit->key = value;
      send_to_char("Done.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "excoord"))
   {
      int x, y;

      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);
      if (arg2[0] == '\0' || arg3[0] == '\0' || argument[0] == '\0')
      {
         send_to_char("Usage: redit excoord <dir> <X> <Y>\n\r", ch);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }

      x = atoi(arg3);
      y = atoi(argument);

      if (x < 1 || x > MAX_X)
      {
         sprintf(buf, "Valid X coordinates are 1 to %d.\n\r", MAX_X);
         send_to_char(buf, ch);
         return;
      }

      if (y < 1 || y > MAX_Y)
      {
         sprintf(buf, "Valid Y coordinates are 1 to %d.\n\r", MAX_Y);
         send_to_char(buf, ch);
         return;
      }

      if (!xit)
      {
         send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
         return;
      }
      xit->coord->x = x;
      xit->coord->y = y;
      send_to_char("Exit coordinates set.\n\r", ch);
      return;
   }
   if (!str_cmp(arg, "exname"))
   {
      argument = one_argument(argument, arg2);
      if (arg2[0] == '\0')
      {
         send_to_char("Change or clear exit keywords.\n\r", ch);
         send_to_char("Usage: redit exname <dir> [keywords]\n\r", ch);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      if (!xit)
      {
         send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
         return;
      }
      STRFREE(xit->keyword);
      xit->keyword = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "exflags"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Toggle or display exit flags.\n\r", ch);
         send_to_char("Usage: redit exflags <dir> <flag> [flag]...\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg2);
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      if (!xit)
      {
         send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
         return;
      }
      if (argument[0] == '\0')
      {
         sprintf(buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\n\r[ ", xit->vdir, xit->keyword, xit->key);
         for (value = 0; value <= MAX_EXFLAG; value++)
         {
            if (IS_SET(xit->exit_info, 1 << value))
            {
               strcat(buf, ex_flags[value]);
               strcat(buf, " ");
            }
         }
         strcat(buf, "]\n\r");
         send_to_char(buf, ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg2);
         value = get_exflag(arg2);
         if (value < 0 || value > MAX_EXFLAG)
            ch_printf(ch, "Unknown flag: %s\n\r", arg2);
         else
            TOGGLE_BIT(xit->exit_info, 1 << value);
      }
      return;
   }

   if (!str_cmp(arg, "exit"))
   {
      bool addexit, numnotdir;

      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);
      if (!arg2 || arg2[0] == '\0')
      {
         send_to_char("Create, change or remove an exit.\n\r", ch);
         send_to_char("Usage: redit exit <dir> [room] [flags] [key] [keywords]\n\r", ch);
         return;
      }
      addexit = numnotdir = FALSE;
      switch (arg2[0])
      {
         default:
            edir = get_dir(arg2);
            break;
         case '+':
            edir = get_dir(arg2 + 1);
            addexit = TRUE;
            break;
         case '#':
            edir = atoi(arg2 + 1);
            numnotdir = TRUE;
            break;
      }
      if (!arg3 || arg3[0] == '\0')
         evnum = 0;
      else
         evnum = atoi(arg3);
      if (numnotdir)
      {
         if ((xit = get_exit_num(location, edir)) != NULL)
            edir = xit->vdir;
      }
      else
         xit = get_exit(location, edir);
      if (!evnum)
      {
         if (xit)
         {
            extract_exit(location, xit);
            send_to_char("Exit removed.\n\r", ch);
            return;
         }
         send_to_char("No exit in that direction.\n\r", ch);
         return;
      }
      if (evnum < 1 || evnum > MAX_VNUM)
      {
         send_to_char("Invalid room number.\n\r", ch);
         return;
      }
      if ((tmp = get_room_index(evnum)) == NULL)
      {
         send_to_char("Non-existant room.\n\r", ch);
         return;
      }
      if (addexit || !xit)
      {
         if (numnotdir)
         {
            send_to_char("Cannot add an exit by number, sorry.\n\r", ch);
            return;
         }
         if (addexit && xit && get_exit_to(location, edir, tmp->vnum))
         {
            send_to_char("There is already an exit in that direction leading to that location.\n\r", ch);
            return;
         }
         xit = make_exit(location, tmp, edir);
         xit->keyword = STRALLOC("");
         xit->description = STRALLOC("");
         xit->key = -1;
         xit->exit_info = 0;
         act(AT_IMMORT, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM);
      }
      else
         act(AT_IMMORT, "Something is different...", ch, NULL, NULL, TO_ROOM);
      if (xit->to_room != tmp)
      {
         xit->to_room = tmp;
         xit->vnum = evnum;
         texit = get_exit_to(xit->to_room, rev_dir[edir], location->vnum);
         if (texit)
         {
            texit->rexit = xit;
            xit->rexit = texit;
         }
      }
      argument = one_argument(argument, arg3);
      if (arg3 && arg3[0] != '\0')
         xit->exit_info = atoi(arg3);
      if (argument && argument[0] != '\0')
      {
         one_argument(argument, arg3);
         ekey = atoi(arg3);
         if (ekey != 0 || arg3[0] == '0')
         {
            argument = one_argument(argument, arg3);
            xit->key = ekey;
         }
         if (argument && argument[0] != '\0')
         {
            STRFREE(xit->keyword);
            xit->keyword = STRALLOC(argument);
         }
      }
      send_to_char("Done.\n\r", ch);
      return;
   }

   /*
    * Twisted and evil, but works    -Thoric
    * Makes an exit, and the reverse in one shot.
    */
   if (!str_cmp(arg, "bexit"))
   {
      EXIT_DATA *xit, *rxit;
      char tmpcmd[MIL];
      ROOM_INDEX_DATA *tmploc;
      int vnum, exnum;
      char rvnum[MIL];
      bool numnotdir;

      argument = one_argument(argument, arg2);
      argument = one_argument(argument, arg3);
      if (!arg2 || arg2[0] == '\0')
      {
         send_to_char("Create, change or remove a two-way exit.\n\r", ch);
         send_to_char("Usage: redit bexit <dir> [room] [flags] [key] [keywords]\n\r", ch);
         return;
      }
      numnotdir = FALSE;
      switch (arg2[0])
      {
         default:
            edir = get_dir(arg2);
            break;
         case '#':
            numnotdir = TRUE;
            edir = atoi(arg2 + 1);
            break;
         case '+':
            edir = get_dir(arg2 + 1);
            break;
      }
      tmploc = location;
      exnum = edir;
      if (numnotdir)
      {
         if ((xit = get_exit_num(tmploc, edir)) != NULL)
            edir = xit->vdir;
      }
      else
         xit = get_exit(tmploc, edir);
      rxit = NULL;
      vnum = 0;
      rvnum[0] = '\0';
      if (xit)
      {
         vnum = xit->vnum;
         if (arg3[0] != '\0')
            sprintf(rvnum, "%d", tmploc->vnum);
         if (xit->to_room)
            rxit = get_exit(xit->to_room, rev_dir[edir]);
         else
            rxit = NULL;
      }
      sprintf(tmpcmd, "exit %s %s %s", arg2, arg3, argument);
      do_redit(ch, tmpcmd);
      if (numnotdir)
         xit = get_exit_num(tmploc, exnum);
      else
         xit = get_exit(tmploc, edir);
      if (!rxit && xit)
      {
         vnum = xit->vnum;
         if (arg3[0] != '\0')
            sprintf(rvnum, "%d", tmploc->vnum);
         if (xit->to_room)
            rxit = get_exit(xit->to_room, rev_dir[edir]);
         else
            rxit = NULL;
      }
      if (vnum)
      {
         sprintf(tmpcmd, "%d redit exit %d %s %s", vnum, rev_dir[edir], rvnum, argument);
         do_at(ch, tmpcmd);
      }
      return;
   }

   if (!str_cmp(arg, "pulltype") || !str_cmp(arg, "pushtype"))
   {
      int pt;

      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0')
      {
         ch_printf(ch, "Set the %s between this room, and the destination room.\n\r", arg);
         ch_printf(ch, "Usage: redit %s <dir> <type>\n\r", arg);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      if (xit)
      {
         if ((pt = get_pulltype(argument)) == -1)
            ch_printf(ch, "Unknown pulltype: %s.  (See help PULLTYPES)\n\r", argument);
         else
         {
            xit->pulltype = pt;
            send_to_char("Done.\n\r", ch);
            return;
         }
      }
      send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "pull"))
   {
      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0')
      {
         send_to_char("Set the 'pull' between this room, and the destination room.\n\r", ch);
         send_to_char("Usage: redit pull <dir> <force (0 to 100)>\n\r", ch);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      if (xit)
      {
         xit->pull = URANGE(-100, atoi(argument), 100);
         send_to_char("Done.\n\r", ch);
         return;
      }
      send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "push"))
   {
      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0')
      {
         send_to_char("Set the 'push' away from the destination room in the opposite direction.\n\r", ch);
         send_to_char("Usage: redit push <dir> <force (0 to 100)>\n\r", ch);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      if (xit)
      {
         xit->pull = URANGE(-100, -(atoi(argument)), 100);
         send_to_char("Done.\n\r", ch);
         return;
      }
      send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "exdistance"))
   {
      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0')
      {
         send_to_char("Set the distance (in rooms) between this room, and the destination room.\n\r", ch);
         send_to_char("Usage: redit exdistance <dir> [distance]\n\r", ch);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      if (xit)
      {
         xit->distance = URANGE(1, atoi(argument), 50);
         send_to_char("Done.\n\r", ch);
         return;
      }
      send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
      return;
   }

   if (!str_cmp(arg, "exdesc"))
   {
      argument = one_argument(argument, arg2);
      if (!arg2 || arg2[0] == '\0')
      {
         send_to_char("Create or clear a description for an exit.\n\r", ch);
         send_to_char("Usage: redit exdesc <dir> [description]\n\r", ch);
         return;
      }
      if (arg2[0] == '#')
      {
         edir = atoi(arg2 + 1);
         xit = get_exit_num(location, edir);
      }
      else
      {
         edir = get_dir(arg2);
         xit = get_exit(location, edir);
      }
      if (xit)
      {
         STRFREE(xit->description);
         if (!argument || argument[0] == '\0')
            xit->description = STRALLOC("");
         else
         {
            sprintf(buf, "%s\n\r", argument);
            xit->description = STRALLOC(buf);
         }
         send_to_char("Done.\n\r", ch);
         return;
      }
      send_to_char("No exit in that direction.  Use 'redit exit ...' first.\n\r", ch);
      return;
   }

   /*
    * Generate usage message.
    */
   if (ch->substate == SUB_REPEATCMD)
   {
      ch->substate = SUB_RESTRICTED;
      interpret(ch, origarg);
      ch->substate = SUB_REPEATCMD;
      ch->last_cmd = do_redit;
   }
   else
      do_redit(ch, "");
   return;
}

void odelete(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_INDEX_DATA *obj;
   OBJ_DATA *temp;

   argument = one_argument(argument, arg);

   if (arg[0] == '\0')
   {
      bug("odelete: No argument passed to function", 0);
      return;
   }

   /* Find the object. */
   if (!(obj = get_obj_index(atoi(arg))))
   {
      if (!(temp = get_obj_here(ch, arg)))
      {
         bug("odelete: no obj to delete (not here)", 0);
         return;
      }
      obj = temp->pIndexData;
   }

   /* Ok, we've determined that the room exists, it is empty and the
      player has the authority to delete it, so let's dump the thing.
      The function to do it is in db.c so it can access the top-room
      variable. */
   delete_obj(obj);
   return;
}

OBJ_DATA *shop_oclean(CHAR_DATA * keeper, OBJ_DATA * obj)
{
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *cobj;
   SLAB_DATA *slab;

   if ((pObjIndex = get_obj_index(obj->cvnum)) == NULL)
   {
      bug("Shop_oclean: Cvnum is pointing to an item not in the game anymore", 0);
      return obj;
   }
   cobj = create_object(pObjIndex, obj->level);
   if (xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
   {
      int oldrace;
      
      for(slab = first_slab; slab; slab = slab->next)
      {
         if(slab->vnum == obj->value[6] )
            break;
      }
      if (!slab)
      {
         bug("shop_oclean:  Invalid vnum on a forge weapon at vnum %d", obj->pIndexData->vnum);
         return cobj;
      }
      oldrace = keeper->race;
      if (obj->value[11] > 0)
         keeper->race = obj->value[11]-1;    
      alter_forge_obj(keeper, cobj, create_object(get_obj_index(slab->vnum), 1), slab);
      keeper->race = oldrace;
   }
   return cobj;
}

OBJ_DATA *shop_ocreate(CHAR_DATA * keeper, OBJ_DATA * obj)
{
   int vnum, fvnum, evnum = 0;
   AREA_DATA *tarea;
   int hlim = 0, llim = 0;
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *cobj;

   vnum = keeper->pIndexData->vnum;
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if ((vnum >= tarea->low_m_vnum) && (vnum <= tarea->hi_m_vnum))
      {
         hlim = tarea->hi_o_vnum;
         llim = tarea->low_o_vnum;
         break;
      }
   }
   for (fvnum = llim; fvnum; fvnum++)
   {
      if (fvnum > hlim)
      {
         bug("shop_ocreate: Mob %d has filled up the vnums in the shop area.", vnum);
         return obj;
      }
      if (!get_obj_index(fvnum))
      {
         evnum = fvnum;
         break;
      }
   }
   if (evnum == 0)
   {
      bug("shop_ocreate: Mob %d did not find a vnum!.", vnum);
      return obj;
   }
   pObjIndex = make_object(evnum, obj->pIndexData->vnum, obj->pIndexData->name, 1);

   if (!pObjIndex)
   {
      log_string("keeper_create: make_object failed.");
      return obj;
   }
   xREMOVE_BIT(pObjIndex->extra_flags, ITEM_PROTOTYPE);
   cobj = create_object(pObjIndex, obj->level);
   if (xIS_SET(obj->extra_flags, ITEM_FORGEABLE))
   {
      FORGE_DATA *forge;
      SLAB_DATA *slab;	 
      char *wbuf;
      char wname[MIL];
      int race = 0;
      int oldrace;
      
      for(slab = first_slab; slab; slab = slab->next)
      {
         if(is_name(slab->adj, obj->name) )
            break;
      }
      if (!slab)
      {
         bug("sell: Forgeable has an invalid ore.");
         return cobj;
      }
      for (forge = first_forge; forge; forge = forge->next)
      {
         if (forge->vnum == obj->pIndexData->vnum)
            break;
      }
      if (!forge)
      {
         bug("selle: Could not find the weapon/armor in the forge list");
         return cobj;
      }
      wbuf = obj->name;
      wbuf = one_argument(wbuf, wname);
      wbuf = one_argument(wbuf, wname);
      wname[0] = UPPER(wname[0]);
       
      if (!str_prefix(wname, "Fairy"))
         race = 6;
      else if (!str_prefix(wname, "Hobbit"))
         race = 5;
      else if (!str_prefix(wname, "Ogre"))
         race = 4;
      else if (!str_prefix(wname, "Dwarven"))
         race = 3;
      else if (!str_prefix(wname, "Elven"))
         race = 2;
      else if (!str_prefix(wname, "Human"))
         race = 1;
      else
      {
         bug("Sell: Invalid Race Name %s, on player %s", wname, keeper->name);
         return cobj;
      }
      cobj->value[6] = slab->vnum;
      cobj->value[11] = race;
      oldrace = keeper->race;
      keeper->race = race-1;    
      alter_forge_obj(keeper, cobj, create_object(get_obj_index(slab->vnum), 1), slab);
      keeper->race = oldrace;
   }  
   return cobj;
}

void do_ocreate(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int vnum, cvnum;

   if (IS_NPC(ch))
   {
      send_to_char("Mobiles cannot create.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   vnum = is_number(arg) ? atoi(arg) : -1;

   if (vnum == -1 || !argument || argument[0] == '\0')
   {
      send_to_char("Usage:  ocreate <vnum> [copy vnum] <item name>\n\r", ch);
      return;
   }

   if (vnum < 1 || vnum > MAX_VNUM)
   {
      send_to_char("Vnum out of range.\n\r", ch);
      return;
   }

   one_argument(argument, arg2);
   cvnum = atoi(arg2);
   if (cvnum != 0)
      argument = one_argument(argument, arg2);
   if (cvnum < 1)
      cvnum = 0;

   if (get_obj_index(vnum))
   {
      send_to_char("An object with that number already exists.\n\r", ch);
      return;
   }

   if (IS_NPC(ch))
      return;
   if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
   {
      AREA_DATA *pArea;

      if (!ch->pcdata || !(pArea = ch->pcdata->area))
      {
         send_to_char("You must have an assigned area to create objects.\n\r", ch);
         return;
      }
      if (vnum < pArea->low_o_vnum || vnum > pArea->hi_o_vnum)
      {
         send_to_char("That number is not in your allocated range.\n\r", ch);
         return;
      }
   }

   pObjIndex = make_object(vnum, cvnum, argument, 1);
   if (!pObjIndex)
   {
      send_to_char("Error.\n\r", ch);
      log_string("do_ocreate: make_object failed.");
      return;
   }
   obj = create_object(pObjIndex, get_trust(ch));
   obj_to_char(obj, ch);
   act(AT_IMMORT, "$n makes arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM);
   ch_printf_color(ch, "&YYou make arcane gestures, and open your hands to reveal %s!\n\rObjVnum:  &W%d   &YKeywords:  &W%s\n\r",
      pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name);
}

void do_addforge(CHAR_DATA * ch, char *argument)
{
    FORGE_DATA *newforge;
    char vnumarg[MIL];
    char namearg[MIL];
    char slabnum[MIL];
    char type[MIL];


    if (argument[0] == '\0')
    {
       send_to_char("Syntax: addforge <name> <vnum> <# of slabs> <type> \n\r", ch);
       send_to_char("(ie: addforge \"Broad Sword\" 21002 3 0)\n\r", ch);
       send_to_char("See help addforge for more info.\n\r", ch);
       return;
    }

    argument = one_argument(argument, namearg);
    if (namearg[0] == '\0')
    {
       send_to_char("It needs a name!!\n\r", ch);
       return;
    }
    argument = one_argument(argument, vnumarg);
    if (vnumarg[0] == '\0')
    {
       send_to_char("What vnum?\n\r", ch);
       return;
    }
    argument = one_argument(argument, slabnum);
    if (slabnum[0] == '\0')
    {
       send_to_char("How many slabs does it take to forge?\n\r", ch);
       return;
    }
    argument = one_argument(argument, type);
    if (type[0] == '\0')
    {
       send_to_char("You forgot to add in a type.\n\r", ch);
       return;
    }

    //Check to make sure a numerical value is present
    if (!isdigit(vnumarg[0]))
    {
       send_to_char("You need to provide a vnum for the forge item, not a name.\n\r", ch);
       return;
    }
    if (!isdigit(slabnum[0]))
    {
       send_to_char("Number of slabs to forge...?\n\r", ch);
       return;
    }
    if (!isdigit(type[0]))
    {
       send_to_char("Will only take numeric values for type.\n\r", ch);
       return;
    }

    //Make sure the object isn't already a forge item
    for (newforge = first_forge; newforge; newforge = newforge->next)
    {
       if (newforge->vnum == atoi(vnumarg))
       {
          send_to_char("This object is already a forge item.\n\r", ch);
          return;
       }
    }
    if ((get_obj_index(atoi(vnumarg))) == NULL)
    {
       send_to_char("This forge item does not actually exist.\n\r", ch);
       return;
    }
    //It is clear now to add the object
    CREATE(newforge, FORGE_DATA, 1);

    newforge->name = STRALLOC(namearg);
    newforge->vnum = atoi(vnumarg);
    newforge->slabnum = atoi(slabnum);
    newforge->type = atoi(type);
    forge_num += 1;

    LINK(newforge, first_forge, last_forge, next, prev);
    //sprintf(buf, "%s has added %s as a forge item.", ch->name, newforge->vnum);
    //log_string_plus(buf, LOG_BUILD, ch->level);
    save_forge_data();
    send_to_char("Done.\n\r", ch);
    return;
}

void do_addslab(CHAR_DATA * ch, char *argument)
{
    SLAB_DATA *newslab;
    char vnumarg[MIL];
    char namearg[MIL];
    char adjarg[MIL];


    if (argument[0] == '\0')
    {
       send_to_char("Syntax: addslab <metalSLAB> <vnum> <adjective> <qps>\n\r", ch);
       return;
    }

    argument = one_argument(argument, namearg);
    if (namearg[0] == '\0')
    {
       send_to_char("It needs a name!!\n\r", ch);
       return;
    }
    argument = one_argument(argument, vnumarg);
    if (vnumarg[0] == '\0')
    {
       send_to_char("What vnum?\n\r", ch);
       return;
    }
    argument = one_argument(argument, adjarg);
    if (adjarg[0] == '\0')
    {
       send_to_char("What is the slab's adjective?\n\r", ch);
       return;
    }
    if (atoi(argument) < 1)
    {
       send_to_char("The qps of an ore has to be greater than 0.\n\r", ch);
       return;
    }
    //Check to make sure a numerical value is present
    if (!isdigit(vnumarg[0]))
    {
       send_to_char("You need to provide a vnum for the slab, not a name.\n\r", ch);
       return;
    }
  
    //Make sure the object isn't already a forge item
    for (newslab = first_slab; newslab; newslab = newslab->next)
    {
       if (newslab->vnum == atoi(vnumarg))
       {
          send_to_char("This object is already a slab item.\n\r", ch);
          return;
       }
    }
    if ((get_obj_index(atoi(vnumarg))) == NULL)
    {
       send_to_char("This slab item does not actually exist.\n\r", ch);
       return;
    }
    //It is clear now to add the object    
    CREATE(newslab, SLAB_DATA, 1);

    newslab->name = STRALLOC(namearg);
    newslab->vnum = atoi(vnumarg);
    newslab->adj = STRALLOC(adjarg);
    newslab->qps = atoi(argument);
    slab_num += 1;

    LINK(newslab, first_slab, last_slab, next, prev);
    //sprintf(buf, "%s has added %s as a slab item.", ch->name, newforge->vnum);
    //log_string_plus(buf, LOG_BUILD, ch->level);
    save_slab_data();  
    send_to_char("Done.\n\r", ch);
    return;             
}
MOB_INDEX_DATA *generate_mobstats(MOB_INDEX_DATA * mIndex, int level)
{
   sh_int l;
   sh_int damnum, damsize, damplus, damf, daml;
   sh_int hitnum, hitsize, hitplus, hitf, hitl;
   sh_int hbefore, hafter, hp, count = 0;
   sh_int attacks[6];
   const int holdhp[12] = {
      0, 65, 120, 270, 430, 860, 1080, 1510, 1940, 2375, 3025, 3900
   };

   l = 1;
   mIndex->level = 0;
   mIndex->hitroll = UMAX(1, l / 2 - 5);
   if (mIndex->hitroll < 1)
      mIndex->hitroll = 1;

   mIndex->gold = UMAX(100 + (l * 15), l * l);
   mIndex->gold = mIndex->gold * .85;

   // HP - dice is 15 percent
   if (l % 5 == 0)
   {
      hitf = holdhp[l / 5] * .15;
      hitl = holdhp[l / 5] * .85;
   }
   else
   {
      hbefore = l / 5;
      hafter = hbefore + 1;
      hp = ((holdhp[hafter] - holdhp[hbefore]) / 5) * abs((hafter * 5) - l - 5);
      hp = holdhp[hbefore] + hp;

      hitf = hp * .15;
      hitl = hp * .85;
   }
   hitplus = hitl;
   hitnum = 2 + (l / 3);
   hitsize = hitf / hitnum;
   if (hitsize < 1)
      hitsize = 1;


   mIndex->hitnodice = hitnum;
   mIndex->hitsizedice = hitsize;
   mIndex->hitplus = hitplus;
   // Damage dice is 65 percent, damplus is 35 //
   damf = (l - 2) * .65;
   daml = (l - 2) * .35;

   if (damf < 1)
      damf = 1;

   if (daml < 1)
      daml = 1;

   damnum = l / 8;
   if (damnum < 1)
      damnum = 1;

   damsize = damf / damnum;
   if (damsize < 1)
      damnum = 1;

   damplus = daml;
   if (damplus < 1)
      damplus = 1;

   mIndex->damnodice = damnum;
   mIndex->damsizedice = damsize;
   mIndex->damplus = damplus;
   if (l > 7)
   {
      if (number_range(1, 2) == 1)
         xTOGGLE_BIT(mIndex->defenses, DFND_PARRY);
      else
         xTOGGLE_BIT(mIndex->defenses, DFND_DODGE);
   }
   for (;;)
   {
      attacks[count] = 0;
      count++;

      if (count >= 6)
         break;
   }
   count = 0;
   for (;;)
   {
      if (number_range(1, 5) > 4)
      {
         if (attacks[0] == 0)
         {
            xTOGGLE_BIT(mIndex->attacks, ATCK_PUNCH);
            attacks[0] = 1;
         }
      }
      if (number_range(1, 5) > 4)
      {
         if (attacks[1] == 0)
         {
            xTOGGLE_BIT(mIndex->attacks, ATCK_KICK);
            attacks[1] = 1;
         }
      }
      if (number_range(1, 7) > 6)
      {
         if (attacks[2] == 0)
         {
            xTOGGLE_BIT(mIndex->attacks, ATCK_TRIP);
            attacks[2] = 1;
         }
      }
      if (number_range(1, 7) > 6)
      {
         if (attacks[3] == 0)
         {
            xTOGGLE_BIT(mIndex->attacks, ATCK_BASH);
            attacks[3] = 1;
         }
      }
      if (number_range(1, 12) > 11)
      {
         if (attacks[4] == 0)
         {
            xTOGGLE_BIT(mIndex->attacks, ATCK_STUN);
            attacks[4] = 1;
         }
      }
      if (number_range(1, 7) > 6)
      {
         if (attacks[5] == 0)
         {
            xTOGGLE_BIT(mIndex->attacks, ATCK_GOUGE);
            attacks[5] = 1;
         }
      }

      count++;
      if (count >= 1 + (l / 10))
         break;
   }
   return mIndex;
}

void do_mcreate(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *mob;
   int vnum, cvnum, makeauto = 0;

   if (IS_NPC(ch))
   {
      send_to_char("Mobiles cannot create.\n\r", ch);
      return;
   }

   argument = one_argument(argument, arg);

   vnum = is_number(arg) ? atoi(arg) : -1;

   if (vnum == -1 || !argument || argument[0] == '\0')
   {
      send_to_char("Usage:  mcreate <vnum> [cvnum] <mobile name>\n\r", ch);
      send_to_char("OR:     mcreate <vnum> [cvnum] auto <level> <name>\n\r", ch);
      return;
   }

   if (vnum < 1 || vnum > MAX_VNUM)
   {
      send_to_char("Vnum out of range.\n\r", ch);
      return;
   }

   one_argument(argument, arg2);
   cvnum = atoi(arg2);
   if (cvnum != 0)
      argument = one_argument(argument, arg2);
   if (cvnum < 1)
      cvnum = 0;

   one_argument(argument, arg3);
   if (!str_cmp(arg3, "auto"))
   {
      argument = one_argument(argument, arg3);
      one_argument(argument, arg4);

      if (atoi(arg4) < 1 || atoi(arg4) > 130)
      {
         send_to_char("Syntax: mcreate <vnum> [cvnum] auto <level> <name>\n\r", ch);
         send_to_char("Level range is 1 to 130.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg4);
      makeauto = 1;
   }

   if (get_mob_index(vnum))
   {
      send_to_char("A mobile with that number already exists.\n\r", ch);
      return;
   }

   if (IS_NPC(ch))
      return;
   if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
   {
      AREA_DATA *pArea;

      if (!ch->pcdata || !(pArea = ch->pcdata->area))
      {
         send_to_char("You must have an assigned area to create mobiles.\n\r", ch);
         return;
      }
      if (vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum)
      {
         send_to_char("That number is not in your allocated range.\n\r", ch);
         return;
      }
   }

   pMobIndex = make_mobile(vnum, cvnum, argument);
   if (!pMobIndex)
   {
      send_to_char("Error.\n\r", ch);
      log_string("do_mcreate: make_mobile failed.");
      return;
   }
   mob = create_mobile(pMobIndex);
   if (makeauto == 1)
      adjust_wildermob(mob, atoi(arg4), 0);

   char_to_room(mob, ch->in_room);

   /* If you create one on the map, make sure it gets placed properly - Samson 8-21-99 */
   if (IS_ONMAP_FLAG(ch))
   {
      mob->map = ch->map;
      mob->coord->x = ch->coord->x;
      mob->coord->y = ch->coord->y;
   }

   act(AT_IMMORT, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM);
   ch_printf_color(ch, "&YYou wave your arms about, and %s appears at your command!\n\rMobVnum:  &W%d   &YKeywords:  &W%s\n\r",
      pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name);
}

void free_reset(AREA_DATA * are, RESET_DATA * res)
{
   UNLINK(res, are->first_reset, are->last_reset, next, prev);
   DISPOSE(res);
}

void free_area(AREA_DATA * are)
{
   DISPOSE(are->name);
   DISPOSE(are->filename);
   while (are->first_reset)
      free_reset(are, are->first_reset);
   DISPOSE(are);
   are = NULL;
}

void assign_area(CHAR_DATA * ch)
{
   char buf[MSL];
   char buf2[MSL];
   char taf[1024];
   AREA_DATA *tarea, *tmp;
   bool created = FALSE;

   if (IS_NPC(ch))
      return;
   if (get_trust(ch) > LEVEL_IMMORTAL && ch->pcdata->r_range_lo && ch->pcdata->r_range_hi)
   {
      tarea = ch->pcdata->area;
      sprintf(taf, "%s.are", capitalize(ch->name));
      if (!tarea)
      {
         for (tmp = first_build; tmp; tmp = tmp->next)
            if (!str_cmp(taf, tmp->filename))
            {
               tarea = tmp;
               break;
            }
      }
      if (!tarea)
      {
         sprintf(buf, "Creating area entry for %s", ch->name);
         log_string_plus(buf, LOG_NORMAL, ch->level);
         CREATE(tarea, AREA_DATA, 1);
         LINK(tarea, first_build, last_build, next, prev);
         tarea->first_reset = NULL;
         tarea->last_reset = NULL;
         sprintf(buf, "{PROTO} %s's area in progress", ch->name);
         tarea->name = str_dup(buf);
         tarea->filename = str_dup(taf);
         sprintf(buf2, "%s", ch->name);
         tarea->author = STRALLOC(buf2);
         tarea->age = 0;
         tarea->nplayer = 0;
         tarea->kingdom = -1;

         created = TRUE;
      }
      else
      {
         sprintf(buf, "Updating area entry for %s", ch->name);
         log_string_plus(buf, LOG_NORMAL, ch->level);
      }
      tarea->low_r_vnum = ch->pcdata->r_range_lo;
      tarea->low_o_vnum = ch->pcdata->o_range_lo;
      tarea->low_m_vnum = ch->pcdata->m_range_lo;
      tarea->hi_r_vnum = ch->pcdata->r_range_hi;
      tarea->hi_o_vnum = ch->pcdata->o_range_hi;
      tarea->hi_m_vnum = ch->pcdata->m_range_hi;
      ch->pcdata->area = tarea;
      if (created)
         sort_area(tarea, TRUE);
   }
}

void do_aassign(CHAR_DATA * ch, char *argument)
{
   char buf[MSL];
   AREA_DATA *tarea, *tmp;

   set_char_color(AT_IMMORT, ch);

   if (IS_NPC(ch))
      return;

   if (argument[0] == '\0')
   {
      send_to_char("Syntax: aassign <filename.are>\n\r", ch);
      return;
   }

   if (!str_cmp("none", argument) || !str_cmp("null", argument) || !str_cmp("clear", argument))
   {
      ch->pcdata->area = NULL;
      assign_area(ch);
      if (!ch->pcdata->area)
         send_to_char("Area pointer cleared.\n\r", ch);
      else
         send_to_char("Originally assigned area restored.\n\r", ch);
      return;
   }

   sprintf(buf, "%s", argument);
   tarea = NULL;

/*	if ( get_trust(ch) >= sysdata.level_modify_proto )   */

   if (get_trust(ch) >= LEVEL_HI_STAFF /* Tracker1 */
      || (is_name(buf, ch->pcdata->bestowments) && get_trust(ch) >= sysdata.level_modify_proto))
      for (tmp = first_area; tmp; tmp = tmp->next)
         if (!str_cmp(buf, tmp->filename))
         {
            tarea = tmp;
            break;
         }

   if (!tarea)
      for (tmp = first_build; tmp; tmp = tmp->next)
         if (!str_cmp(buf, tmp->filename))
         {
/*		if ( get_trust(ch) >= sysdata.level_modify_proto  */
            if (get_trust(ch) >= LEVEL_STAFF /* Tracker1 */
               || is_name(tmp->filename, ch->pcdata->bestowments) || (ch->pcdata->council && is_name("aassign", ch->pcdata->council->powers)))
            {
               tarea = tmp;
               break;
            }
            else
            {
               send_to_char("You do not have permission to use that area.\n\r", ch);
               return;
            }
         }

   if (!tarea)
   {
      if (get_trust(ch) >= sysdata.level_modify_proto)
         send_to_char("No such area.  Use 'zones'.\n\r", ch);
      else
         send_to_char("No such area.  Use 'newzones'.\n\r", ch);
      return;
   }
   ch->pcdata->area = tarea;
   ch_printf(ch, "Assigning you: %s\n\r", tarea->name);
   return;
}


EXTRA_DESCR_DATA *SetRExtra(ROOM_INDEX_DATA * room, char *keywords)
{
   EXTRA_DESCR_DATA *ed;

   for (ed = room->first_extradesc; ed; ed = ed->next)
   {
      if (is_name(keywords, ed->keyword))
         break;
   }
   if (!ed)
   {
      CREATE(ed, EXTRA_DESCR_DATA, 1);
      LINK(ed, room->first_extradesc, room->last_extradesc, next, prev);
      ed->keyword = STRALLOC(keywords);
      ed->description = STRALLOC("");
      top_ed++;
   }
   return ed;
}

bool DelRExtra(ROOM_INDEX_DATA * room, char *keywords)
{
   EXTRA_DESCR_DATA *rmed;

   for (rmed = room->first_extradesc; rmed; rmed = rmed->next)
   {
      if (is_name(keywords, rmed->keyword))
         break;
   }
   if (!rmed)
      return FALSE;
   UNLINK(rmed, room->first_extradesc, room->last_extradesc, next, prev);
   STRFREE(rmed->keyword);
   STRFREE(rmed->description);
   DISPOSE(rmed);
   top_ed--;
   return TRUE;
}

EXTRA_DESCR_DATA *SetOExtra(OBJ_DATA * obj, char *keywords)
{
   EXTRA_DESCR_DATA *ed;

   for (ed = obj->first_extradesc; ed; ed = ed->next)
   {
      if (is_name(keywords, ed->keyword))
         break;
   }
   if (!ed)
   {
      CREATE(ed, EXTRA_DESCR_DATA, 1);
      LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev);
      ed->keyword = STRALLOC(keywords);
      ed->description = STRALLOC("");
      top_ed++;
   }
   return ed;
}

bool DelOExtra(OBJ_DATA * obj, char *keywords)
{
   EXTRA_DESCR_DATA *rmed;

   for (rmed = obj->first_extradesc; rmed; rmed = rmed->next)
   {
      if (is_name(keywords, rmed->keyword))
         break;
   }
   if (!rmed)
      return FALSE;
   UNLINK(rmed, obj->first_extradesc, obj->last_extradesc, next, prev);
   STRFREE(rmed->keyword);
   STRFREE(rmed->description);
   DISPOSE(rmed);
   top_ed--;
   return TRUE;
}

EXTRA_DESCR_DATA *SetOExtraProto(OBJ_INDEX_DATA * obj, char *keywords)
{
   EXTRA_DESCR_DATA *ed;

   for (ed = obj->first_extradesc; ed; ed = ed->next)
   {
      if (is_name(keywords, ed->keyword))
         break;
   }
   if (!ed)
   {
      CREATE(ed, EXTRA_DESCR_DATA, 1);
      LINK(ed, obj->first_extradesc, obj->last_extradesc, next, prev);
      ed->keyword = STRALLOC(keywords);
      ed->description = STRALLOC("");
      top_ed++;
   }
   return ed;
}

bool DelOExtraProto(OBJ_INDEX_DATA * obj, char *keywords)
{
   EXTRA_DESCR_DATA *rmed;

   for (rmed = obj->first_extradesc; rmed; rmed = rmed->next)
   {
      if (is_name(keywords, rmed->keyword))
         break;
   }
   if (!rmed)
      return FALSE;
   UNLINK(rmed, obj->first_extradesc, obj->last_extradesc, next, prev);
   STRFREE(rmed->keyword);
   STRFREE(rmed->description);
   DISPOSE(rmed);
   top_ed--;
   return TRUE;
}

void fold_area(AREA_DATA * tarea, char *filename, bool install, int message)
{
   RESET_DATA *treset;
   ROOM_INDEX_DATA *room;
   MOB_INDEX_DATA *pMobIndex;
   OBJ_INDEX_DATA *pObjIndex;
   MPROG_DATA *mprog;
   EXIT_DATA *xit;
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *paf;
   SHOP_DATA *pShop;
   REPAIR_DATA *pRepair;
   char buf[MSL];
   FILE *fpout;
   int vnum;
   int val0, val1, val2, val3, val4, val5, val6, val7, val8, val9, val10, val11, val12, val13;
   bool complexmob;

   if (message == 0)
   {
      sprintf(buf, "Saving %s...", tarea->filename);
      log_string_plus(buf, LOG_NORMAL, LEVEL_HI_IMM); /* Tracker1 */
   }
   sprintf(buf, "%s.bak", filename);
   rename(filename, buf);
   fclose(fpReserve);
   if ((fpout = fopen(filename, "w")) == NULL)
   {
      bug("fold_area: fopen", 0);
      perror(filename);
      fpReserve = fopen(NULL_FILE, "r");
      return;
   }

   fprintf(fpout, "#AREA   %s~\n\n\n\n", tarea->name);
   fprintf(fpout, "#VERSION %d\n", AREA_VERSION_WRITE);
   fprintf(fpout, "#AUTHOR %s~\n\n", tarea->author);
   fprintf(fpout, "#RANGES\n");
   fprintf(fpout, "%d %d %d %d %d %d %d %d %d\n", tarea->low_soft_range,
      tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range, tarea->kingdom, tarea->x, tarea->y, tarea->map, tarea->kpid);
   fprintf(fpout, "$\n\n");
   if (tarea->kowner)
      fprintf(fpout, "#KOWNER %s~\n\n", tarea->kowner);

   if (tarea->resetmsg) /* Rennard */
      fprintf(fpout, "#RESETMSG %s~\n\n", tarea->resetmsg);
   if (tarea->reset_frequency)
      fprintf(fpout, "#FLAGS\n%d %d\n\n", tarea->flags, tarea->reset_frequency);
   else
      fprintf(fpout, "#FLAGS\n%d\n\n", tarea->flags);

   fprintf(fpout, "#ECONOMY %d %d %d\n\n", tarea->high_economy, tarea->low_economy, tarea->population);

   /* Climate info - FB */
   /*  fprintf( fpout, "#CLIMATE %d %d %d\n\n", tarea->weather->climate_temp,
      tarea->weather->climate_precip,
      tarea->weather->climate_wind); */

   /* save mobiles */
   fprintf(fpout, "#MOBILES\n");
   for (vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++)
   {
      if ((pMobIndex = get_mob_index(vnum)) == NULL)
         continue;
      if (install)
         xREMOVE_BIT(pMobIndex->act, ACT_PROTOTYPE);
      if (pMobIndex->perm_str != 13 || pMobIndex->perm_int != 13
         || pMobIndex->perm_wis != 13 || pMobIndex->perm_dex != 13
         || pMobIndex->perm_con != 13 || pMobIndex->perm_cha != 13
         || pMobIndex->perm_lck != 13 || pMobIndex->perm_agi != 15
         || pMobIndex->hitroll != 0 || pMobIndex->damroll != 0
         || pMobIndex->race != 0
         || !xIS_EMPTY(pMobIndex->attacks)
         || !xIS_EMPTY(pMobIndex->defenses)
         || pMobIndex->height != 0 || pMobIndex->weight != 0 || pMobIndex->speaks != 0 || pMobIndex->speaking != 0 || pMobIndex->xflags != 0)
         complexmob = TRUE;
      else
         complexmob = FALSE;
      fprintf(fpout, "#%d\n", vnum);
      fprintf(fpout, "%s~\n", pMobIndex->player_name);
      fprintf(fpout, "%s~\n", pMobIndex->short_descr);
      fprintf(fpout, "%s~\n", strip_cr(pMobIndex->long_descr));
      fprintf(fpout, "%s~\n", strip_cr(pMobIndex->description));
      fprintf(fpout, "%s ", print_bitvector(&pMobIndex->act));
      fprintf(fpout, "%s ", print_bitvector(&pMobIndex->miflags));
      fprintf(fpout, "%s %d %c\n", print_bitvector(&pMobIndex->affected_by), pMobIndex->alignment, complexmob ? 'C' : 'S');
      fprintf(fpout, "0 0 %d %d %d %d\n", pMobIndex->ac, pMobIndex->tohitbash, pMobIndex->tohitslash, pMobIndex->tohitstab);
      fprintf(fpout, "%dd%d+%d ", pMobIndex->hitnodice, pMobIndex->hitsizedice, pMobIndex->hitplus);
      fprintf(fpout, "%dd%d+%d %d %d %d\n", pMobIndex->damnodice, pMobIndex->damsizedice, pMobIndex->damplus, pMobIndex->max_move,
         pMobIndex->damaddlow, pMobIndex->damaddhi);
      fprintf(fpout, "%d 0\n", pMobIndex->gold);
/* Need to convert to new positions correctly on loadup sigh -Shaddai */
      fprintf(fpout, "%d %d %d\n", pMobIndex->position + 100, pMobIndex->defposition + 100, pMobIndex->sex);
      if (complexmob)
      {
         fprintf(fpout, "%d %d %d %d %d %d %d %d\n",
            pMobIndex->perm_str,
            pMobIndex->perm_int, pMobIndex->perm_wis, pMobIndex->perm_dex, pMobIndex->perm_con, pMobIndex->perm_cha, pMobIndex->perm_lck,
            pMobIndex->perm_agi);
         fprintf(fpout, "%d %d %d %d %d\n",
            pMobIndex->saving_poison_death,
            pMobIndex->saving_wand, pMobIndex->saving_para_petri, pMobIndex->saving_breath, pMobIndex->saving_spell_staff);
         fprintf(fpout, "%d 0 %d %d %d %d %d\n",
            pMobIndex->race, pMobIndex->height, pMobIndex->weight, pMobIndex->speaks, pMobIndex->speaking, pMobIndex->perm_agi);

         fprintf(fpout, "%d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            pMobIndex->m1, pMobIndex->m2, pMobIndex->m3, pMobIndex->m4, pMobIndex->m5, pMobIndex->m6, pMobIndex->m7, 
            pMobIndex->m8, pMobIndex->m9, pMobIndex->cident, pMobIndex->m10, pMobIndex->m11, pMobIndex->m12);
         fprintf(fpout, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            pMobIndex->apply_res_fire, pMobIndex->apply_res_water, pMobIndex->apply_res_air, pMobIndex->apply_res_earth,
            pMobIndex->apply_res_energy, pMobIndex->apply_res_magic, pMobIndex->apply_res_nonmagic, pMobIndex->apply_res_blunt,
            pMobIndex->apply_res_pierce, pMobIndex->apply_res_slash, pMobIndex->apply_res_poison, pMobIndex->apply_res_paralysis,
            pMobIndex->apply_res_holy, pMobIndex->apply_res_unholy, pMobIndex->apply_res_undead);
         fprintf(fpout, "%d %d %d %d %d %d %d %s ",
            pMobIndex->hitroll,
            pMobIndex->damroll,
            pMobIndex->xflags,
            pMobIndex->resistant, pMobIndex->immune, pMobIndex->susceptible, pMobIndex->elementb, print_bitvector(&pMobIndex->attacks));
         fprintf(fpout, "%s\n", print_bitvector(&pMobIndex->defenses));
      }

      if (pMobIndex->mudprogs)
      {
         for (mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next)
            fprintf(fpout, "> %s %s~\n%s~\n", mprog_type_to_name(mprog->type), mprog->arglist, strip_cr(mprog->comlist));
         fprintf(fpout, "|\n");
      }
   }
   fprintf(fpout, "#0\n\n\n");
   if (install && vnum < tarea->hi_m_vnum)
      tarea->hi_m_vnum = vnum - 1;

   /* save objects */
   fprintf(fpout, "#OBJECTS\n");
   for (vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++)
   {
      if ((pObjIndex = get_obj_index(vnum)) == NULL)
         continue;
      if (install)
         xREMOVE_BIT(pObjIndex->extra_flags, ITEM_PROTOTYPE);
      fprintf(fpout, "#%d\n", vnum);
      fprintf(fpout, "%s~\n", pObjIndex->name);
      fprintf(fpout, "%s~\n", pObjIndex->short_descr);
      fprintf(fpout, "%s~\n", pObjIndex->description);
      fprintf(fpout, "%s~\n", pObjIndex->action_desc);
      if (pObjIndex->layers)
         fprintf(fpout, "%d %s %d %d\n", pObjIndex->item_type, print_bitvector(&pObjIndex->extra_flags), pObjIndex->wear_flags, pObjIndex->layers);
      else
         fprintf(fpout, "%d %s %d\n", pObjIndex->item_type, print_bitvector(&pObjIndex->extra_flags), pObjIndex->wear_flags);

      val0 = pObjIndex->value[0];
      val1 = pObjIndex->value[1];
      val2 = pObjIndex->value[2];
      val3 = pObjIndex->value[3];
      val4 = pObjIndex->value[4];
      val5 = pObjIndex->value[5];
      val6 = pObjIndex->value[6];
      val7 = pObjIndex->value[7];
      val8 = pObjIndex->value[8];
      val9 = pObjIndex->value[9];
      val10 = pObjIndex->value[10];
      val11 = pObjIndex->value[11];
      val12 = pObjIndex->value[12];
      val13 = pObjIndex->value[13];
      switch (pObjIndex->item_type)
      {
         case ITEM_POTION:
         case ITEM_SCROLL:
            if (IS_VALID_SN(val1))
            {
               if (AREA_VERSION_WRITE == 0)
                  val1 = skill_table[val1]->slot;
               else
                  val1 = HAS_SPELL_INDEX;
            }
            if (IS_VALID_SN(val2))
            {
               if (AREA_VERSION_WRITE == 0)
                  val2 = skill_table[val2]->slot;
               else
                  val1 = HAS_SPELL_INDEX;
            }
            if (IS_VALID_SN(val3))
            {
               if (AREA_VERSION_WRITE == 0)
                  val3 = skill_table[val3]->slot;
               else
                  val1 = HAS_SPELL_INDEX;
            }
            break;
         case ITEM_WEAPON:
            if (IS_VALID_SN(val5))
            {
               if (AREA_VERSION_WRITE == 0)
                  val5 = skill_table[val5]->slot;
               else
                  val5 = HAS_SPELL_INDEX;
            }
            break;
         case ITEM_SALVE:
            break;
         case ITEM_SPELLBOOK:
            if (IS_VALID_SN(val1))
               val1 = HAS_SPELL_INDEX;
            break;
         case ITEM_SHEATH:
            if (IS_VALID_SN(val4))
               val4 = HAS_SPELL_INDEX;
            break;
			case ITEM_TREASURE:
 			if (xIS_SET(pObjIndex->extra_flags,ITEM_AFFREAG))
 			{
 				if(val0 == APPLY_WEARSPELL)
 				{
 					if (IS_VALID_SN(val1))
 					{
 					   if (AREA_VERSION_WRITE == 0)
 						  val5 = skill_table[val1]->slot;
 					   else
 						  val5 = HAS_SPELL_INDEX;
 					}	
 				}
 			}
      }
      if (val4 || val5 || val6 || val7 || val8 || val9 || val10 || val11 || val12 || val13)
         fprintf(fpout, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n", 
            val0, val1, val2, val3, val4, val5, val6, val7, val8, val9, val10, val11, val12, val13);
      else
         fprintf(fpout, "%d %d %d %d\n", val0, val1, val2, val3);
      /* Rent might be used later for something else, till then, foldarea will set all
         the values to 0! --Xerves */
      /*
      if(AREA_VERSION_WRITE > 38)
      {
        fprintf(fpout, "%d %d %d %d %d %d\n", pObjIndex->weight, pObjIndex->cost, pObjIndex->bless_dur, 0, pObjIndex->cvnum, pObjIndex->cident);
      }
      */
      if (AREA_VERSION_WRITE >= 11)
      {
         fprintf(fpout, "%f %d %d %d %d %d %d\n", pObjIndex->weight, pObjIndex->cost, 0, pObjIndex->cvnum, pObjIndex->cident, pObjIndex->sworthrestrict, pObjIndex->imbueslots);
      }
      else
      {
         fprintf(fpout, "%f %d %d\n", pObjIndex->weight, pObjIndex->cost, 0);
      }
      if (AREA_VERSION_WRITE > 0)
         switch (pObjIndex->item_type)
         {
            case ITEM_PILL:
               fprintf(fpout, "'%s' '%s' '%s'\n",
                  IS_VALID_SN(pObjIndex->value[1]) ?
                  skill_table[pObjIndex->value[1]]->name : "NONE",
                  IS_VALID_SN(pObjIndex->value[2]) ?
                  skill_table[pObjIndex->value[2]]->name : "NONE",
                  IS_VALID_SN(pObjIndex->value[3]) ? 
                  skill_table[pObjIndex->value[3]]->name : "NONE");
               break;
               
            case ITEM_WEAPON:
               fprintf(fpout, "'%s'\n",
                  IS_VALID_SN(pObjIndex->value[4]) ?
                  skill_table[pObjIndex->value[4]]->name : "NONE");
               break;
               
            case ITEM_POTION:
            case ITEM_SCROLL:
               fprintf(fpout, "'%s' '%s' '%s'\n",
                  IS_VALID_SN(pObjIndex->value[1]) ?
                  skill_table[pObjIndex->value[1]]->name : "NONE",
                  IS_VALID_SN(pObjIndex->value[2]) ?
                  skill_table[pObjIndex->value[2]]->name : "NONE",
                  IS_VALID_SN(pObjIndex->value[3]) ? skill_table[pObjIndex->value[3]]->name : "NONE");
               break;
            case ITEM_STAFF:
            case ITEM_WAND:
               fprintf(fpout, "'NONE'\n");

               break;
            case ITEM_SALVE:
               fprintf(fpout, "'NONE' 'NONE'\n");
               break;
            case ITEM_SPELLBOOK:
               fprintf(fpout, "'%s'\n", IS_VALID_SN(pObjIndex->value[1]) ? skill_table[pObjIndex->value[1]]->name : "NONE");
               break;
            case ITEM_SHEATH:
               fprintf(fpout, "'%s'\n", IS_VALID_SN(pObjIndex->value[4]) ? skill_table[pObjIndex->value[4]]->name : "NONE");
         }

      for (ed = pObjIndex->first_extradesc; ed; ed = ed->next)
         fprintf(fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr(ed->description));

      for (paf = pObjIndex->first_affect; paf; paf = paf->next)
         fprintf(fpout, "A\n%d %d %d\n", paf->location,
            ((paf->location == APPLY_WEAPONSPELL
|| paf->location == APPLY_WEARSPELL
|| paf->location == APPLY_REMOVESPELL
|| paf->location == APPLY_STRIPSN || paf->location == APPLY_RECURRINGSPELL) && IS_VALID_SN(paf->modifier)) ? skill_table[paf->modifier]->slot : paf->modifier, paf->gemnum);

      if (pObjIndex->mudprogs)
      {
         for (mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next)
            fprintf(fpout, "> %s %s~\n%s~\n", mprog_type_to_name(mprog->type), mprog->arglist, strip_cr(mprog->comlist));
         fprintf(fpout, "|\n");
      }
   }
   fprintf(fpout, "#0\n\n\n");
   if (install && vnum < tarea->hi_o_vnum)
      tarea->hi_o_vnum = vnum - 1;

   /* save rooms   */
   fprintf(fpout, "#ROOMS\n");
   for (vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++)
   {
      if ((room = get_room_index(vnum)) == NULL)
         continue;
      if (install)
      {
         CHAR_DATA *victim, *vnext;
         OBJ_DATA *obj, *obj_next;

         /* remove prototype flag from room */
         xREMOVE_BIT(room->room_flags, ROOM_PROTOTYPE);
         /* purge room of (prototyped) mobiles */
         for (victim = room->first_person; victim; victim = vnext)
         {
            vnext = victim->next_in_room;
            if (IS_NPC(victim))
               extract_char(victim, TRUE);
         }
         /* purge room of (prototyped) objects */
         for (obj = room->first_content; obj; obj = obj_next)
         {
            obj_next = obj->next_content;
            extract_obj(obj);
         }
      }
      fprintf(fpout, "#%d\n", vnum);
      fprintf(fpout, "%s~\n", room->name);
      fprintf(fpout, "%s~\n", strip_cr(room->description));
      fprintf(fpout, "%s\n", print_bitvector(&room->room_flags));
      if ((room->tele_delay > 0 && room->tele_vnum > 0) || room->tunnel > 0 || room->resource != 0)
         fprintf(fpout, "0 %d %d %d %d %d\n", room->sector_type, room->tele_delay, room->tele_vnum, room->tunnel, room->resource);
      else
         fprintf(fpout, "0 %d\n", room->sector_type);
      fprintf(fpout, "%d\n", room->quad);
      fprintf(fpout, "%d\n", room->node_mana); /* folds the mana in the node (node_mana) */
      for (xit = room->first_exit; xit; xit = xit->next)
      {
         if (IS_SET(xit->exit_info, EX_PORTAL)) /* don't fold portals */
            continue;
         fprintf(fpout, "D%d\n", xit->vdir);
         fprintf(fpout, "%s~\n", strip_cr(xit->description));
         fprintf(fpout, "%s~\n", strip_cr(xit->keyword));
         if (xit->distance > 1 || xit->pull)
            fprintf(fpout, "%d %d %d %d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum, xit->distance, xit->pulltype, xit->pull);
         else
            fprintf(fpout, "%d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum);


         fprintf(fpout, "%d %d\n", xit->coord->x, xit->coord->y);

      }
      for (ed = room->first_extradesc; ed; ed = ed->next)
         fprintf(fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr(ed->description));

      if (room->map) /* maps */
      {
#ifdef OLDMAPS
         fprintf(fpout, "M\n");
         fprintf(fpout, "%s~\n", strip_cr(room->map));
#else
         fprintf(fpout, "M %d %d %d %c\n", room->map->vnum, room->map->x, room->map->y, room->map->entry);
#endif
      }
      if (room->mudprogs)
      {
         for (mprog = room->mudprogs; mprog; mprog = mprog->next)
            fprintf(fpout, "> %s %s~\n%s~\n", mprog_type_to_name(mprog->type), mprog->arglist, strip_cr(mprog->comlist));
         fprintf(fpout, "|\n");
      }
      fprintf(fpout, "S\n");
   }
   fprintf(fpout, "#0\n\n\n");
   if (install && vnum < tarea->hi_r_vnum)
      tarea->hi_r_vnum = vnum - 1;

   /* save resets   */
   fprintf(fpout, "#RESETS\n");
   for (treset = tarea->first_reset; treset; treset = treset->next)
   {
      switch (treset->command) /* extra arg1 arg2 arg3 */
      {
         default:
         case '*':
            break;
         
         case 'e':
         case 'E':
            fprintf(fpout, "%c %d %d %d %d %d %d %d\n", UPPER(treset->command), treset->extra, treset->arg1, treset->arg2, treset->arg3, treset->arg4, treset->resetlast, treset->resettime);
            break;
         case 'G':
         case 'g':
            fprintf(fpout, "%c %d %d %d %d %d %d %d\n", UPPER(treset->command), treset->extra, treset->arg1, treset->arg2, treset->arg3, treset->arg4, treset->resetlast, treset->resettime);
            break;
         case 'O':
         case 'o':   
            fprintf(fpout, "%c %d %d %d %d %d %d %d %d %d %d\n", UPPER(treset->command),
               treset->extra, treset->arg1, treset->arg2, treset->arg3, treset->arg4, treset->arg5, treset->arg6, treset->arg7, treset->resetlast, treset->resettime);
            break;
         case 'p':
         case 'P':
            fprintf(fpout, "%c %d %d %d %d %d %d %d\n", UPPER(treset->command), treset->extra, treset->arg1, treset->arg2, treset->arg3, treset->arg4, treset->resetlast, treset->resettime);
            break;
         case 'd':
         case 'D':
         case 't':
         case 'T':
         case 'b':
         case 'B':
            fprintf(fpout, "%c %d %d %d %d\n", UPPER(treset->command), treset->extra, treset->arg1, treset->arg2, treset->arg3);
            break;
         case 'm':
         case 'M':
         case 'h':
         case 'H':
            fprintf(fpout, "%c %d %d %d %d %d %d %d %d %d\n", UPPER(treset->command),
               treset->extra, treset->arg1, treset->arg2, treset->arg3, treset->arg4, treset->arg5, treset->arg6, treset->resetlast, treset->resettime);
            break;
         case 'r':
         case 'R':
            fprintf(fpout, "%c %d %d %d\n", UPPER(treset->command), treset->extra, treset->arg1, treset->arg2);
            break;
         case 'a':
         case 'A':
            fprintf(fpout, "%c %d %d %d\n", UPPER(treset->command), treset->extra, treset->arg1, treset->arg2);
            break;
      }
   }
   fprintf(fpout, "S\n\n\n");

   /* save shops */
   fprintf(fpout, "#SHOPS\n");
   for (vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++)
   {
      if ((pMobIndex = get_mob_index(vnum)) == NULL)
         continue;
      if ((pShop = pMobIndex->pShop) == NULL)
         continue;
      fprintf(fpout, " %d   %2d %2d %2d %2d %2d   %3d %3d",
         pShop->keeper,
         pShop->buy_type[0], pShop->buy_type[1], pShop->buy_type[2], pShop->buy_type[3], pShop->buy_type[4], pShop->profit_buy, pShop->profit_sell);
      fprintf(fpout, "        %2d %2d    ; %s\n", pShop->open_hour, pShop->close_hour, pMobIndex->short_descr);
   }
   fprintf(fpout, "0\n\n\n");

   /* save repair shops */
   fprintf(fpout, "#REPAIRS\n");
   for (vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++)
   {
      if ((pMobIndex = get_mob_index(vnum)) == NULL)
         continue;
      if ((pRepair = pMobIndex->rShop) == NULL)
         continue;
      fprintf(fpout, " %d   %2d %2d %2d         %3d %3d",
         pRepair->keeper, pRepair->fix_type[0], pRepair->fix_type[1], pRepair->fix_type[2], pRepair->profit_fix, pRepair->shop_type);
      fprintf(fpout, "        %2d %2d    ; %s\n", pRepair->open_hour, pRepair->close_hour, pMobIndex->short_descr);
   }
   fprintf(fpout, "0\n\n\n");

   /* save specials */
   fprintf(fpout, "#SPECIALS\n");
   for (vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++)
   {
      if ((pMobIndex = get_mob_index(vnum)) == NULL)
         continue;
      if (!pMobIndex->spec_fun)
         continue;
      fprintf(fpout, "M  %d %s\n", pMobIndex->vnum, lookup_spec(pMobIndex->spec_fun));
   }
   fprintf(fpout, "S\n\n\n");

   /* END */
   fprintf(fpout, "#$\n");
   fclose(fpout);
   fpReserve = fopen(NULL_FILE, "r");
   return;
}

void do_savearea(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *tarea;
   char filename[256];

   set_char_color(AT_IMMORT, ch);

   if (IS_NPC(ch) || get_trust(ch) < LEVEL_IMM || !ch->pcdata || (argument[0] == '\0' && !ch->pcdata->area))
   {
      send_to_char("You don't have an assigned area to save.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
      tarea = ch->pcdata->area;
   else
   {
      bool found;

      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("You can only save your own area.\n\r", ch);
         return;
      }
      for (found = FALSE, tarea = first_build; tarea; tarea = tarea->next)
         if (!str_cmp(tarea->filename, argument))
         {
            found = TRUE;
            break;
         }
      if (!found)
      {
         send_to_char("Area not found.\n\r", ch);
         return;
      }
   }

   if (!tarea)
   {
      send_to_char("No area to save.\n\r", ch);
      return;
   }

/* Ensure not wiping out their area with save before load - Scryn 8/11 */
   if (!IS_SET(tarea->status, AREA_LOADED))
   {
      send_to_char("Your area is not loaded!\n\r", ch);
      return;
   }

   sprintf(filename, "%s%s", BUILD_DIR, tarea->filename);
   send_to_char("Saving area...\n\r", ch);
   fold_area(tarea, filename, FALSE, 0);
   set_char_color(AT_IMMORT, ch);
   send_to_char("Done.\n\r", ch);
}

void do_loadarea(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *tarea;
   char filename[256];
   int tmp;

   set_char_color(AT_IMMORT, ch);

   if (IS_NPC(ch) || get_trust(ch) < LEVEL_IMM || !ch->pcdata || (argument[0] == '\0' && !ch->pcdata->area))
   {
      send_to_char("You don't have an assigned area to load.\n\r", ch);
      return;
   }

   if (argument[0] == '\0')
      tarea = ch->pcdata->area;
   else
   {
      bool found;

      if (get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char("You can only load your own area.\n\r", ch);
         return;
      }
      for (found = FALSE, tarea = first_build; tarea; tarea = tarea->next)
         if (!str_cmp(tarea->filename, argument))
         {
            found = TRUE;
            break;
         }
      if (!found)
      {
         send_to_char("Area not found.\n\r", ch);
         return;
      }
   }

   if (!tarea)
   {
      send_to_char("No area to load.\n\r", ch);
      return;
   }

/* Stops char from loading when already loaded - Scryn 8/11 */
   if (IS_SET(tarea->status, AREA_LOADED))
   {
      send_to_char("Your area is already loaded.\n\r", ch);
      return;
   }
   sprintf(filename, "%s%s", BUILD_DIR, tarea->filename);
   send_to_char("Loading...\n\r", ch);
   load_area_file(tarea, filename);
   send_to_char("Planing...\n\r", ch);
   check_planes(NULL);
   send_to_char("Linking exits...\n\r", ch);
   fix_area_exits(tarea);
   if (tarea->first_reset)
   {
      tmp = tarea->nplayer;
      tarea->nplayer = 0;
      send_to_char("Resetting area...\n\r", ch);
      reset_area(tarea, 0);
      tarea->nplayer = tmp;
   }
   send_to_char("Done.\n\r", ch);
}

/*
 * Dangerous command.  Can be used to install an area that was either:
 *   (a) already installed but removed from area.lst
 *   (b) designed offline
 * The mud will likely crash if:
 *   (a) this area is already loaded
 *   (b) it contains vnums that exist
 *   (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.		-Thoric
 */
void do_unfoldarea(CHAR_DATA * ch, char *argument)
{

   set_char_color(AT_IMMORT, ch);

   if (!argument || argument[0] == '\0')
   {
      send_to_char("Unfold what?\n\r", ch);
      return;
   }

   fBootDb = TRUE;
   load_area_file(last_area, argument);
   fBootDb = FALSE;
   check_planes(NULL);
   return;
}

void fdarea(CHAR_DATA * keeper, char *argument)
{
   AREA_DATA *tarea;
   char buf[MSL];

   if (!argument || argument[0] == '\0')
   {
      sprintf(buf, "** No area passed to fdarea, mob %d **", keeper->pIndexData->vnum);
      bug(buf, 0);
      return;
   }

   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (!str_cmp(tarea->filename, argument))
      {
         fold_area(tarea, tarea->filename, FALSE, 1);
         return;
      }
   }
   sprintf(buf, "** Area passed to fdarea is no good, mob %d **", keeper->pIndexData->vnum);
   bug(buf, 0);
   return;
}

void do_updatearea(CHAR_DATA * ch, char *argument)
{
   if (ch->pcdata->job != 4 && ch->pcdata->caste < kingdom_table[ch->pcdata->hometown]->minbuild)
   {
      send_to_char("Huh?\n\r", ch);
      return;
   }
   set_char_color(AT_IMMORT, ch);
   if (!IS_SET(ch->in_room->area->flags, AFLAG_CARPENTER))
   {
      send_to_char("The area you are standing in cannot be updated.\n\r", ch);
      return;
   }
   send_to_char("Updating area...\n\r", ch);
   fold_area(ch->in_room->area, ch->in_room->area->filename, FALSE, 0);
   write_portal_file();
   set_char_color(AT_IMMORT, ch);
   send_to_char("Done.\n\r", ch);
   return;
}


void do_foldarea(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *tarea;

   set_char_color(AT_IMMORT, ch);

   if (!argument || argument[0] == '\0')
   {
      send_to_char("Fold what?\n\r", ch);
      return;
   }

   if (!str_cmp(argument, "allareas"))
   {
      if (get_trust(ch) < LEVEL_STAFF)
      {
         send_to_char("No such area exists.\n\r", ch);
         return;
      }
      for (tarea = first_area; tarea; tarea = tarea->next)
      {
         fold_area(tarea, tarea->filename, FALSE, 0);
      }
      send_to_char("Done folding all nonproto areas.\n\r", ch);
      return;
   }


   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (!str_cmp(tarea->filename, argument))
      {
         send_to_char("Remember: New additions had to be prototype to save!!\n\r", ch);
         send_to_char("If you don't understand, please contact Xerves\n\r", ch);
         send_to_char("Folding area...\n\r", ch);
         fold_area(tarea, tarea->filename, FALSE, 0);
         set_char_color(AT_IMMORT, ch);
         send_to_char("Done.\n\r", ch);
         return;
      }
   }
   send_to_char("No such area exists.\n\r", ch);
   return;
}

extern int top_area;

void write_area_list()
{
   AREA_DATA *tarea;
   FILE *fpout;

   fpout = fopen(AREA_LIST, "w");
   if (!fpout)
   {
      bug("FATAL: cannot open area.lst for writing!\n\r", 0);
      return;
   }
   fprintf(fpout, "help.are\n");
   for (tarea = first_area; tarea; tarea = tarea->next)
   {
      if (tarea->low_r_vnum >= START_QUEST_VNUM && tarea->low_r_vnum <= END_QUEST_VNUM)
         continue;
      fprintf(fpout, "%s\n", tarea->filename);
   }
   fprintf(fpout, "$\n");
   fclose(fpout);
}

/*
 * A complicated to use command as it currently exists.		-Thoric
 * Once area->author and area->name are cleaned up... it will be easier
 */
void do_installarea(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *tarea;
   char arg[MIL];
   char buf[MSL];
   int num;
   DESCRIPTOR_DATA *d;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg);
   if (arg[0] == '\0')
   {
      send_to_char("Syntax: installarea <filename> [Area title]\n\r", ch);
      return;
   }

   for (tarea = first_build; tarea; tarea = tarea->next)
   {
      if (!str_cmp(tarea->filename, arg))
      {
         if (argument && argument[0] != '\0')
         {
            DISPOSE(tarea->name);
            tarea->name = str_dup(argument);
         }

         /* Fold area with install flag -- auto-removes prototype flags */
         send_to_char("Saving and installing file...\n\r", ch);
         fold_area(tarea, tarea->filename, TRUE, 0);

         /* Remove from prototype area list */
         UNLINK(tarea, first_build, last_build, next, prev);

         /* Add to real area list */
         LINK(tarea, first_area, last_area, next, prev);

         /* Fix up author if online */
         for (d = first_descriptor; d; d = d->next)
            if (d->character && d->character->pcdata && d->character->pcdata->area == tarea)
            {
               /* remove area from author */
               d->character->pcdata->area = NULL;
               /* clear out author vnums  */
               d->character->pcdata->r_range_lo = 0;
               d->character->pcdata->r_range_hi = 0;
               d->character->pcdata->o_range_lo = 0;
               d->character->pcdata->o_range_hi = 0;
               d->character->pcdata->m_range_lo = 0;
               d->character->pcdata->m_range_hi = 0;
            }

         top_area++;
         send_to_char("Writing area.lst...\n\r", ch);
         write_area_list();
         send_to_char("Resetting new area.\n\r", ch);
         num = tarea->nplayer;
         tarea->nplayer = 0;
         reset_area(tarea, 0);
         tarea->nplayer = num;
         send_to_char("Renaming author's building file.\n\r", ch);
         sprintf(buf, "%s%s.installed", BUILD_DIR, tarea->filename);
         sprintf(arg, "%s%s", BUILD_DIR, tarea->filename);
         rename(arg, buf);
         send_to_char("Done.\n\r", ch);
         return;
      }
   }
   send_to_char("No such area exists.\n\r", ch);
   return;
}

//No longer used
void add_reset_nested(AREA_DATA * tarea, OBJ_DATA * obj)
{
   int limit;

   for (obj = obj->first_content; obj; obj = obj->next_content)
   {
      limit = obj->pIndexData->count;
      if (limit < 1)
         limit = 1;
      add_reset(tarea, 'P', 1, obj->pIndexData->vnum, limit, obj->in_obj->pIndexData->vnum, -1, -1, -1, -1, 0, 0);
      if (obj->first_content)
         add_reset_nested(tarea, obj);
   }
}


/*
 * Parse a reset command string into a reset_data structure
 */
RESET_DATA *parse_reset(AREA_DATA * tarea, char *argument, CHAR_DATA * ch)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   char arg5[MIL];
   char arg6[MIL];
   char arg7[MIL];
   char letter;
   int extra, val1, val2, val3, val4, val5, val6;
   int value;
   ROOM_INDEX_DATA *room;
   EXIT_DATA *pexit;

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   argument = one_argument(argument, arg4);
   argument = one_argument(argument, arg5);
   argument = one_argument(argument, arg6);
   argument = one_argument(argument, arg7);
   extra = 0;
   letter = '*';
   val1 = atoi(arg2);
   val2 = atoi(arg3);
   val3 = atoi(arg4);
   val4 = atoi(arg5);
   val5 = atoi(arg6);
   val6 = atoi(arg7);

   val4 = -1;
   val5 = -1;
   val6 = -1;
   if (arg1[0] == '\0')
   {
      send_to_char("Reset commands: mob obj give equip door rand trap hide.\n\r", ch);
      return NULL;
   }

   if (!str_cmp(arg1, "hide"))
   {
      if (arg2[0] != '\0' && !get_obj_index(val1))
      {
         send_to_char("Reset: HIDE: no such object\n\r", ch);
         return NULL;
      }
      else
         val1 = 0;
      extra = 1;
      val2 = 0;
      val3 = 0;
      letter = 'H';
   }
   else if (arg2[0] == '\0')
   {
      send_to_char("Reset: not enough arguments.\n\r", ch);
      return NULL;
   }
   else if (val1 < 1 || val1 > MAX_VNUM)
   {
      send_to_char("Reset: value out of range.\n\r", ch);
      return NULL;
   }
   else if (!str_cmp(arg1, "mob"))
   {
      if (!get_mob_index(val1))
      {
         send_to_char("Reset: MOB: no such mobile\n\r", ch);
         return NULL;
      }
      if (!get_room_index(val2))
      {
         send_to_char("Reset: MOB: no such room\n\r", ch);
         return NULL;
      }
      val3 = 1;
      if (val4 < 1 || val4 > MAX_X)
         val4 = val5 = val6 = -1;
      if (val5 < 1 || val5 > MAX_Y)
         val4 = val5 = val6 = -1;
      if (val6 < 0 || val6 >= MAP_MAX)
         val4 = val5 = val6 = -1;
      letter = 'M';
   }
   else if (!str_cmp(arg1, "obj"))
   {
      if (!get_obj_index(val1))
      {
         send_to_char("Reset: OBJ: no such object\n\r", ch);
         return NULL;
      }
      if (!get_room_index(val2))
      {
         send_to_char("Reset: OBJ: no such room\n\r", ch);
         return NULL;
      }
      if (val3 < 1)
         val3 = 1;
      letter = 'O';
   }
   else if (!str_cmp(arg1, "give"))
   {
      if (!get_obj_index(val1))
      {
         send_to_char("Reset: GIVE: no such object\n\r", ch);
         return NULL;
      }
      if (val2 < 1)
         val2 = 1;
      val3 = val2;
      val2 = 0;
      extra = 1;
      letter = 'G';
   }
   else if (!str_cmp(arg1, "equip"))
   {
      if (!get_obj_index(val1))
      {
         send_to_char("Reset: EQUIP: no such object\n\r", ch);
         return NULL;
      }
      if (!is_number(arg3))
         val2 = get_wearloc(arg3);
      if (val2 < 0 || val2 >= MAX_WEAR)
      {
         send_to_char("Reset: EQUIP: invalid wear location\n\r", ch);
         return NULL;
      }
      if (val3 < 1)
         val3 = 1;
      extra = 1;
      letter = 'E';
   }
   else if (!str_cmp(arg1, "put"))
   {
      if (!get_obj_index(val1))
      {
         send_to_char("Reset: PUT: no such object\n\r", ch);
         return NULL;
      }
      if (val2 > 0 && !get_obj_index(val2))
      {
         send_to_char("Reset: PUT: no such container\n\r", ch);
         return NULL;
      }
      extra = UMAX(val3, 0);
      argument = one_argument(argument, arg4);
      val3 = (is_number(argument) ? atoi(arg4) : 0);
      if (val3 < 0)
         val3 = 0;
      letter = 'P';
   }
   else if (!str_cmp(arg1, "door"))
   {
      if ((room = get_room_index(val1)) == NULL)
      {
         send_to_char("Reset: DOOR: no such room\n\r", ch);
         return NULL;
      }
      if (val2 < 0 || val2 > 9)
      {
         send_to_char("Reset: DOOR: invalid exit\n\r", ch);
         return NULL;
      }
      if ((pexit = get_exit(room, val2)) == NULL || !IS_SET(pexit->exit_info, EX_ISDOOR))
      {
         send_to_char("Reset: DOOR: no such door\n\r", ch);
         return NULL;
      }
      if (val3 < 0 || val3 > 2)
      {
         send_to_char("Reset: DOOR: invalid door state (0 = open, 1 = close, 2 = lock)\n\r", ch);
         return NULL;
      }
      letter = 'D';
      value = val3;
      val3 = val2;
      val2 = value;
   }
   else if (!str_cmp(arg1, "rand"))
   {
      if (!get_room_index(val1))
      {
         send_to_char("Reset: RAND: no such room\n\r", ch);
         return NULL;
      }
      if (val2 < 0 || val2 > 9)
      {
         send_to_char("Reset: RAND: invalid max exit\n\r", ch);
         return NULL;
      }
      val3 = val2;
      val2 = 0;
      letter = 'R';
   }
   else if (!str_cmp(arg1, "trap"))
   {
      if (val2 < 1 || val2 > MAX_TRAPTYPE)
      {
         send_to_char("Reset: TRAP: invalid trap type\n\r", ch);
         return NULL;
      }
      if (val3 < 0 || val3 > 10000)
      {
         send_to_char("Reset: TRAP: invalid trap charges\n\r", ch);
         return NULL;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg4);
         value = get_trapflag(arg4);
         if (value >= 0 && value < 32)
            SET_BIT(extra, 1 << value);
         else
         {
            send_to_char("Reset: TRAP: bad flag\n\r", ch);
            return NULL;
         }
      }
      if (IS_SET(extra, TRAP_ROOM) && IS_SET(extra, TRAP_OBJ))
      {
         send_to_char("Reset: TRAP: Must specify room OR object, not both!\n\r", ch);
         return NULL;
      }
      if (IS_SET(extra, TRAP_ROOM) && !get_room_index(val1))
      {
         send_to_char("Reset: TRAP: no such room\n\r", ch);
         return NULL;
      }
      if (IS_SET(extra, TRAP_OBJ) && val1 > 0 && !get_obj_index(val1))
      {
         send_to_char("Reset: TRAP: no such object\n\r", ch);
         return NULL;
      }
      if (!IS_SET(extra, TRAP_ROOM) && !IS_SET(extra, TRAP_OBJ))
      {
         send_to_char("Reset: TRAP: Must specify ROOM or OBJECT\n\r", ch);
         return NULL;
      }
      /* fix order */
      value = val1;
      val1 = val2;
      val2 = value;
      letter = 'T';
   }
   if (letter == '*')
      return NULL;
   else
      return make_reset(letter, extra, val1, val3, val2, val4, val5, val6, -1, 0, 0);
}

void do_astat(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *tarea;
   bool proto, found;
   int pdeaths = 0, pkills = 0, mdeaths = 0, mkills = 0;

   found = FALSE;
   proto = FALSE;

   set_char_color(AT_PLAIN, ch);

   if (!str_cmp("summary", argument))
   {
      for (tarea = first_area; tarea; tarea = tarea->next)
      {
         pdeaths += tarea->pdeaths;
         mdeaths += tarea->mdeaths;
         pkills += tarea->pkills;
         mkills += tarea->mkills;
      }
      ch_printf_color(ch, "&WTotal pdeaths:      &w%d\n\r", pdeaths);
      ch_printf_color(ch, "&WTotal pkills:       &w%d\n\r", pkills);
      ch_printf_color(ch, "&WTotal mdeaths:      &w%d\n\r", mdeaths);
      ch_printf_color(ch, "&WTotal mkills:       &w%d\n\r", mkills);
      return;
   }

   for (tarea = first_area; tarea; tarea = tarea->next)
      if (!str_cmp(tarea->filename, argument))
      {
         found = TRUE;
         break;
      }

   if (!found)
      for (tarea = first_build; tarea; tarea = tarea->next)
         if (!str_cmp(tarea->filename, argument))
         {
            found = TRUE;
            proto = TRUE;
            break;
         }

   if (!found)
   {
      if (argument && argument[0] != '\0')
      {
         send_to_char("Area not found.  Check 'zones'.\n\r", ch);
         return;
      }
      else
      {
         tarea = ch->in_room->area;
      }
   }

   ch_printf_color(ch, "\n\r&wName:     &W%-20s  &wKingdom: &W%s\n\r&wFilename: &W%-20s  &wPrototype: &W%s\n\r&wAuthor:   &W%s\n\r",
      tarea->name, tarea->kingdom >= 0 ? kingdom_table[tarea->kingdom]->name : "None", tarea->filename, proto ? "yes" : "no", tarea->author);
   ch_printf_color(ch, "&wAge: &W%-3d  &wCurrent number of players: &W%-3d  &wMax players: &W%d\n\r", tarea->age, tarea->nplayer, tarea->max_players);
   if (!proto)
   {
      if (tarea->high_economy)
         ch_printf_color(ch, "&wArea economy: &W%d &wbillion and &W%d gold coins.\n\r", tarea->high_economy, tarea->low_economy);
      else
         ch_printf_color(ch, "&wArea economy: &W%d &wgold coins.\n\r", tarea->low_economy);
      ch_printf_color(ch, "&wGold Looted:  &W%d\n\r", tarea->gold_looted);
      ch_printf_color(ch, "&wMdeaths: &W%d   &wMkills: &W%d   &wPdeaths: &W%d   &wPkills: &W%d   &wIllegalPK: &W%d\n\r",
         tarea->mdeaths, tarea->mkills, tarea->pdeaths, tarea->pkills, tarea->illegal_pk);
   }
   ch_printf_color(ch, "&wlow_room: &W%5d    &whi_room: &W%5d\n\r", tarea->low_r_vnum, tarea->hi_r_vnum);
   ch_printf_color(ch, "&wlow_obj : &W%5d    &whi_obj : &W%5d\n\r", tarea->low_o_vnum, tarea->hi_o_vnum);
   ch_printf_color(ch, "&wlow_mob : &W%5d    &whi_mob : &W%5d\n\r", tarea->low_m_vnum, tarea->hi_m_vnum);
   ch_printf_color(ch, "&wsoft range: &W%d - %d    &whard range: &W%d - %d\n\r",
      tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range);
   ch_printf_color(ch, "&wArea flags: &W%s\n\r", flag_string(tarea->flags, area_flags));
   ch_printf_color(ch, "&wResetmsg: &W%s\n\r", tarea->resetmsg ? tarea->resetmsg : "(default)"); /* Rennard */
   ch_printf_color(ch, "&wReset frequency: &W%d &wminutes.\n\r", tarea->reset_frequency ? tarea->reset_frequency : 15);
}


void do_aset(CHAR_DATA * ch, char *argument)
{
   AREA_DATA *tarea;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   bool proto, found;
   int vnum, value;

   set_char_color(AT_IMMORT, ch);

   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   vnum = atoi(argument);
   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Usage: aset <area filename> <field> <value>\n\r", ch);
      send_to_char("\n\rField being one of:\n\r", ch);
      send_to_char("  low_room hi_room low_obj hi_obj low_mob hi_mob\n\r", ch);
      send_to_char("  name filename low_soft hi_soft low_hard hi_hard\n\r", ch);
      send_to_char("  author resetmsg resetfreq flags kingdom x y map\n\r", ch);
      return;
   }

   found = FALSE;
   proto = FALSE;
   for (tarea = first_area; tarea; tarea = tarea->next)
      if (!str_cmp(tarea->filename, arg1))
      {
         found = TRUE;
         break;
      }

   if (!found)
      for (tarea = first_build; tarea; tarea = tarea->next)
         if (!str_cmp(tarea->filename, arg1))
         {
            found = TRUE;
            proto = TRUE;
            break;
         }

   if (!found)
   {
      send_to_char("Area not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "name"))
   {
      DISPOSE(tarea->name);
      tarea->name = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "filename"))
   {
      DISPOSE(tarea->filename);
      tarea->filename = str_dup(argument);
      write_area_list();
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "low_economy"))
   {
      tarea->low_economy = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "x"))
   {
      if (atoi(arg2) >= 1 || atoi(arg2) <= MAX_X)
      {
         tarea->x = vnum;
         send_to_char("Done.\n\r", ch);
      }
      else
      {
         ch_printf(ch, "The range is 1 to %d", MAX_X);
      }
      return;
   }
   if (!str_cmp(arg2, "y"))
   {
      if (atoi(arg2) >= 1 || atoi(arg2) <= MAX_Y)
      {
         tarea->y = vnum;
         send_to_char("Done.\n\r", ch);
      }
      else
      {
         ch_printf(ch, "The range is 1 to %d", MAX_Y);
      }
      return;
   }
   if (!str_cmp(arg2, "map"))
   {
      if (atoi(arg2) >= 0 || atoi(arg2) < MAP_MAX)
      {
         tarea->map = vnum;
         send_to_char("Done.\n\r", ch);
      }
      else
      {
         ch_printf(ch, "The range is 0 to %d", MAP_MAX);
      }
      return;
   }

   if (!str_cmp(arg2, "high_economy"))
   {
      tarea->high_economy = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "low_room"))
   {
      tarea->low_r_vnum = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "kingdom"))
   {
      if (vnum < 0 || vnum >= sysdata.max_kingdom)
      {
         send_to_char("That is not in the valid range, showkingdoms for proper values.\n\r", ch);
         return;
      }
      else
      {
         tarea->kingdom = vnum;
         send_to_char("Done.\n\r", ch);
         return;
      }
   }

   if (!str_cmp(arg2, "hi_room"))
   {
      tarea->hi_r_vnum = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "low_obj"))
   {
      tarea->low_o_vnum = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "hi_obj"))
   {
      tarea->hi_o_vnum = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "low_mob"))
   {
      tarea->low_m_vnum = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "hi_mob"))
   {
      tarea->hi_m_vnum = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "low_soft"))
   {
      if (vnum < 0 || vnum > MAX_LEVEL)
      {
         send_to_char("That is not an acceptable value.\n\r", ch);
         return;
      }

      tarea->low_soft_range = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "hi_soft"))
   {
      if (vnum < 0 || vnum > MAX_LEVEL)
      {
         send_to_char("That is not an acceptable value.\n\r", ch);
         return;
      }

      tarea->hi_soft_range = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "low_hard"))
   {
      if (vnum < 0 || vnum > MAX_LEVEL)
      {
         send_to_char("That is not an acceptable value.\n\r", ch);
         return;
      }

      tarea->low_hard_range = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "hi_hard"))
   {
      if (vnum < 0 || vnum > MAX_LEVEL)
      {
         send_to_char("That is not an acceptable value.\n\r", ch);
         return;
      }

      tarea->hi_hard_range = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "author"))
   {
      STRFREE(tarea->author);
      tarea->author = STRALLOC(argument);
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "resetmsg"))
   {
      if (tarea->resetmsg)
         DISPOSE(tarea->resetmsg);
      if (str_cmp(argument, "clear"))
         tarea->resetmsg = str_dup(argument);
      send_to_char("Done.\n\r", ch);
      return;
   } /* Rennard */

   if (!str_cmp(arg2, "resetfreq"))
   {
      tarea->reset_frequency = vnum;
      send_to_char("Done.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "flags"))
   {
      if (!argument || argument[0] == '\0')
      {
         send_to_char("Usage: aset <filename> flags <flag> [flag]...\n\r", ch);
         return;
      }
      while (argument[0] != '\0')
      {
         argument = one_argument(argument, arg3);
         value = get_areaflag(arg3);
         if (value < 0 || value > 31)
            ch_printf(ch, "Unknown flag: %s\n\r", arg3);
         else
         {
            if (IS_SET(tarea->flags, 1 << value))
               REMOVE_BIT(tarea->flags, 1 << value);
            else
               SET_BIT(tarea->flags, 1 << value);
         }
      }
      return;
   }

   do_aset(ch, "");
   return;
}


void do_rlist(CHAR_DATA * ch, char *argument)
{
   ROOM_INDEX_DATA *room;
   int vnum;
   char arg1[MIL];
   char arg2[MIL];
   AREA_DATA *tarea;
   int lrange;
   int trange;

   set_pager_color(AT_PLAIN, ch);

   if (IS_NPC(ch) || get_trust(ch) < LEVEL_IMM || !ch->pcdata || (!ch->pcdata->area && get_trust(ch) < LEVEL_STAFF)) /* Tracker1 */
   {
      send_to_char_color("&YYou don't have an assigned area.\n\r", ch);
      return;
   }

   tarea = ch->pcdata->area;
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (tarea)
   {
      if (arg1[0] == '\0') /* cleaned a big scary mess */
         lrange = tarea->low_r_vnum; /* here.     -Thoric */
      else
         lrange = atoi(arg1);
      if (arg2[0] == '\0')
         trange = tarea->hi_r_vnum;
      else
         trange = atoi(arg2);

      if ((lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum) && get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char_color("&YThat is out of your vnum range.\n\r", ch);
         return;
      }
   }
   else
   {
      lrange = (is_number(arg1) ? atoi(arg1) : 1);
      trange = (is_number(arg2) ? atoi(arg2) : 1);
   }

   for (vnum = lrange; vnum <= trange; vnum++)
   {
      if ((room = get_room_index(vnum)) == NULL)
         continue;
      pager_printf(ch, "%5d) %s\n\r", vnum, room->name);
   }
   return;
}

void do_olist(CHAR_DATA * ch, char *argument)
{
   OBJ_INDEX_DATA *obj;
   int vnum;
   AREA_DATA *tarea;
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   int lrange;
   int trange;
   int lsgold = 0;
   char *pro;
   char lev[2];

   /*
    * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
    */

   set_pager_color(AT_PLAIN, ch);

   if (IS_NPC(ch) || get_trust(ch) < LEVEL_IMM || !ch->pcdata || (!ch->pcdata->area && get_trust(ch) < LEVEL_STAFF)) /* Tracker1 */
   {
      send_to_char_color("&YYou don't have an assigned area.\n\r", ch);
      return;
   }
   tarea = ch->pcdata->area;
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (tarea)
   {
      if (arg1[0] == '\0') /* cleaned a big scary mess */
         lrange = tarea->low_o_vnum; /* here.     -Thoric */
      else
         lrange = atoi(arg1);
      if (arg2[0] == '\0')
         trange = tarea->hi_o_vnum;
      else
         trange = atoi(arg2);

      if ((lrange < tarea->low_o_vnum || trange > tarea->hi_o_vnum) && get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char_color("&YThat is out of your vnum range.\n\r", ch);
         return;
      }
   }
   else
   {
      lrange = (is_number(arg1) ? atoi(arg1) : 1);
      trange = (is_number(arg2) ? atoi(arg2) : 3);
   }
   argument = one_argument(argument, arg3);
   if (!str_cmp(arg3, "gold"))
      lsgold = 1;

   for (vnum = lrange; vnum <= trange; vnum++)
   {
      if ((obj = get_obj_index(vnum)) == NULL)
         continue;

      if (IS_OBJ_STAT(obj, ITEM_PROTOTYPE))
         pro = "P";
      else
         pro = "NP";

      if (obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_ARMOR)
         sprintf(lev, "%d", obj->value[5]);
      else
         sprintf(lev, "  ");

      if (lsgold == 0)
      {
         pager_printf(ch, "&G%5d (%-2s) (%-2s) %-20s (%s)\n\r", vnum, pro, lev, obj->name, obj->short_descr);
      }
      else
      {
         pager_printf(ch, "&G%5d (%-2s) (%-2s) (%6dG) %-20s (%s)\n\r", vnum, pro, lev, obj->cost, obj->name, obj->short_descr);
      }
   }
   return;
}

void do_mlist(CHAR_DATA * ch, char *argument)
{
   MOB_INDEX_DATA *mob;
   int vnum;
   AREA_DATA *tarea;
   char arg1[MIL];
   char arg2[MIL];
   int lrange;
   int trange;

   set_pager_color(AT_PLAIN, ch);

   if (IS_NPC(ch) || get_trust(ch) < LEVEL_IMM || !ch->pcdata || (!ch->pcdata->area && get_trust(ch) < LEVEL_STAFF)) /* Tracker1 */
   {
      send_to_char_color("&YYou don't have an assigned area.\n\r", ch);
      return;
   }

   tarea = ch->pcdata->area;
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);

   if (tarea)
   {
      if (arg1[0] == '\0') /* cleaned a big scary mess */
         lrange = tarea->low_m_vnum; /* here.     -Thoric */
      else
         lrange = atoi(arg1);
      if (arg2[0] == '\0')
         trange = tarea->hi_m_vnum;
      else
         trange = atoi(arg2);

      if ((lrange < tarea->low_m_vnum || trange > tarea->hi_m_vnum) && get_trust(ch) < LEVEL_STAFF) /* Tracker1 */
      {
         send_to_char_color("&YThat is out of your vnum range.\n\r", ch);
         return;
      }
   }
   else
   {
      lrange = (is_number(arg1) ? atoi(arg1) : 1);
      trange = (is_number(arg2) ? atoi(arg2) : 1);
   }

   for (vnum = lrange; vnum <= trange; vnum++)
   {
      if ((mob = get_mob_index(vnum)) == NULL)
         continue;
      pager_printf(ch, "&G%5d (%-2s) %-20s '%s'\n\r",
         vnum, xIS_SET(mob->act, ACT_PROTOTYPE) ? "P" : "NP", mob->player_name, mob->short_descr);
   }
}

void mpedit(CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, char *argument)
{
   if (mptype != -1)
   {
      mprg->type = mptype;
      if (mprg->arglist)
         STRFREE(mprg->arglist);
      mprg->arglist = STRALLOC(argument);
   }
   ch->substate = SUB_MPROG_EDIT;
   ch->dest_buf = mprg;
   if (!mprg->comlist)
      mprg->comlist = STRALLOC("");
   start_editing(ch, mprg->comlist);
   editor_desc_printf(ch, "Program '%s %s'.", mprog_type_to_name(mprg->type), mprg->arglist);
   return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
void do_mpedit(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   CHAR_DATA *victim;
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Mob's can't mpedit\n\r", ch);
      return;
   }

   if (!ch->desc)
   {
      send_to_char("You have no descriptor\n\r", ch);
      return;
   }

   /* Mset/Oset/Redit On Mode check -- Stop most building crashes -- Xerves 8/7/99 */
   if ((xIS_SET(ch->act, PLR_MSET)) || (xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_REDIT)))
   {
      send_to_char("You need to turn mset/oset/redit off\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;
      case SUB_MPROG_EDIT:
         if (!ch->dest_buf)
         {
            send_to_char("Fatal error: report to Thoric.\n\r", ch);
            bug("do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0);
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ch->dest_buf;
         if (mprog->comlist)
            STRFREE(mprog->comlist);
         mprog->comlist = copy_buffer(ch);
         stop_editing(ch);
         return;
   }

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   value = atoi(arg3);
   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Syntax: mpedit <victim> <command> [number] <program> <value>\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Command being one of:\n\r", ch);
      send_to_char("  add delete insert edit list\n\r", ch);
      send_to_char("Program being one of:\n\r", ch);
      send_to_char("  act speech rand fight hitprcnt greet allgreet\n\r", ch);
      send_to_char("  entry give bribe death time hour script\n\r", ch);
      return;
   }

   if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
   {
      if ((victim = get_char_room_new(ch, arg1, 1)) == NULL)
      {
         send_to_char("They aren't here.\n\r", ch);
         return;
      }
   }
   else
   {
      if ((victim = get_char_world(ch, arg1)) == NULL)
      {
         send_to_char("No one like that in all the realms.\n\r", ch);
         return;
      }
   }

   if (get_trust(ch) < victim->level || !IS_NPC(victim))
   {
      send_to_char("You can't do that!\n\r", ch);
      return;
   }

   if (!can_mmodify(ch, victim))
      return;

   if (!is_prototype(ch, NULL, victim))
   {
      send_to_char("A mobile must have a prototype flag to be mpset.\n\r", ch);
      return;
   }

   mprog = victim->pIndexData->mudprogs;

   set_char_color(AT_GREEN, ch);

   if (!str_cmp(arg2, "list"))
   {
      cnt = 0;
      if (!mprog)
      {
         send_to_char("That mobile has no mob programs.\n\r", ch);
         return;
      }

      if (value < 1)
      {
         if (strcmp("full", arg3))
         {
            for (mprg = mprog; mprg; mprg = mprg->next)
            {
               ch_printf(ch, "%d>%s %s\n\r", ++cnt, mprog_type_to_name(mprg->type), mprg->arglist);
            }

            return;
         }
         else
         {
            for (mprg = mprog; mprg; mprg = mprg->next)
            {
               ch_printf(ch, "%d>%s %s\n\r%s\n\r", ++cnt, mprog_type_to_name(mprg->type), mprg->arglist, mprg->comlist);
            }

            return;
         }
      }

      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value)
         {
            ch_printf(ch, "%d>%s %s\n\r%s\n\r", cnt, mprog_type_to_name(mprg->type), mprg->arglist, mprg->comlist);
            break;
         }
      }

      if (!mprg)
         send_to_char("Program not found.\n\r", ch);

      return;
   }

   if (!str_cmp(arg2, "edit"))
   {
      if (!mprog)
      {
         send_to_char("That mobile has no mob programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg4);
      if (arg4[0] != '\0')
      {
         mptype = get_mpflag(arg4);
         if (mptype == -1)
         {
            send_to_char("Unknown program type.\n\r", ch);
            return;
         }
      }
      else
         mptype = -1;
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = 0;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value)
         {
            mpedit(ch, mprg, mptype, argument);
            xCLEAR_BITS(victim->pIndexData->progtypes);
            for (mprg = mprog; mprg; mprg = mprg->next)
               xSET_BIT(victim->pIndexData->progtypes, mprg->type);
            return;
         }
      }
      send_to_char("Program not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "delete"))
   {
      int num;
      bool found;

      if (!mprog)
      {
         send_to_char("That mobile has no mob programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg4);
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = 0;
      found = FALSE;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value)
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if (!found)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = num = 0;
      for (mprg = mprog; mprg; mprg = mprg->next)
         if (mprg->type == mptype)
            num++;
      if (value == 1)
      {
         mprg_next = victim->pIndexData->mudprogs;
         victim->pIndexData->mudprogs = mprg_next->next;
      }
      else
         for (mprg = mprog; mprg; mprg = mprg_next)
         {
            mprg_next = mprg->next;
            if (++cnt == (value - 1))
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if (mprg_next)
      {
         STRFREE(mprg_next->arglist);
         STRFREE(mprg_next->comlist);
         DISPOSE(mprg_next);
         if (num <= 1)
            xREMOVE_BIT(victim->pIndexData->progtypes, mptype);
         send_to_char("Program removed.\n\r", ch);
      }
      return;
   }

   if (!str_cmp(arg2, "insert"))
   {
      if (!mprog)
      {
         send_to_char("That mobile has no mob programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg4);
      mptype = get_mpflag(arg4);
      if (mptype == -1)
      {
         send_to_char("Unknown program type.\n\r", ch);
         return;
      }
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      if (value == 1)
      {
         CREATE(mprg, MPROG_DATA, 1);
         xSET_BIT(victim->pIndexData->progtypes, mptype);
         mpedit(ch, mprg, mptype, argument);
         mprg->next = mprog;
         victim->pIndexData->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value && mprg->next)
         {
            CREATE(mprg_next, MPROG_DATA, 1);
            xSET_BIT(victim->pIndexData->progtypes, mptype);
            mpedit(ch, mprg_next, mptype, argument);
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      send_to_char("Program not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "add"))
   {
      mptype = get_mpflag(arg3);
      if (mptype == -1)
      {
         send_to_char("Unknown program type.\n\r", ch);
         return;
      }
      if (mprog != NULL)
         for (; mprog->next; mprog = mprog->next) ;
      CREATE(mprg, MPROG_DATA, 1);
      if (mprog)
         mprog->next = mprg;
      else
         victim->pIndexData->mudprogs = mprg;
      xSET_BIT(victim->pIndexData->progtypes, mptype);
      mpedit(ch, mprg, mptype, argument);
      mprg->next = NULL;
      return;
   }

   do_mpedit(ch, "");
}

void do_opedit(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   char arg4[MIL];
   OBJ_DATA *obj;
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Mob's can't opedit\n\r", ch);
      return;
   }

   if (!ch->desc)
   {
      send_to_char("You have no descriptor\n\r", ch);
      return;
   }

   /* Mset/Oset/Redit On Mode check -- Stop most building crashes -- Xerves 8/7/99 */
   if ((xIS_SET(ch->act, PLR_MSET)) || (xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_REDIT)))
   {
      send_to_char("You need to turn mset/oset/redit off\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;
      case SUB_MPROG_EDIT:
         if (!ch->dest_buf)
         {
            send_to_char("Fatal error: report to Thoric.\n\r", ch);
            bug("do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0);
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ch->dest_buf;
         if (mprog->comlist)
            STRFREE(mprog->comlist);
         mprog->comlist = copy_buffer(ch);
         stop_editing(ch);
         return;
   }

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   argument = one_argument(argument, arg3);
   value = atoi(arg3);

   if (arg1[0] == '\0' || arg2[0] == '\0')
   {
      send_to_char("Syntax: opedit <object> <command> [number] <program> <value>\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Command being one of:\n\r", ch);
      send_to_char("  add delete insert edit list\n\r", ch);
      send_to_char("Program being one of:\n\r", ch);
      send_to_char("  act speech rand wear remove sac zap get\n\r", ch);
      send_to_char("  drop damage repair greet exa use\n\r", ch);
      send_to_char("  pull push (for levers,pullchains,buttons)\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Object should be in your inventory to edit.\n\r", ch);
      return;
   }

   if (get_trust(ch) < LEVEL_HI_IMM) /* Tracker1 */
   {
      if ((obj = get_obj_carry(ch, arg1)) == NULL)
      {
         send_to_char("You aren't carrying that.\n\r", ch);
         return;
      }
   }
   else
   {
      if ((obj = get_obj_world(ch, arg1)) == NULL)
      {
         send_to_char("Nothing like that in all the realms.\n\r", ch);
         return;
      }
   }

   if (!can_omodify(ch, obj))
      return;

   if (!is_prototype(ch, obj, NULL))
   {
      send_to_char("An object must have a prototype flag to be opset.\n\r", ch);
      return;
   }

   mprog = obj->pIndexData->mudprogs;

   set_char_color(AT_GREEN, ch);

   if (!str_cmp(arg2, "list"))
   {
      cnt = 0;
      if (!mprog)
      {
         send_to_char("That object has no obj programs.\n\r", ch);
         return;
      }
      for (mprg = mprog; mprg; mprg = mprg->next)
         ch_printf(ch, "%d>%s %s\n\r%s\n\r", ++cnt, mprog_type_to_name(mprg->type), mprg->arglist, mprg->comlist);
      return;
   }

   if (!str_cmp(arg2, "edit"))
   {
      if (!mprog)
      {
         send_to_char("That object has no obj programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg4);
      if (arg4[0] != '\0')
      {
         mptype = get_mpflag(arg4);
         if (mptype == -1)
         {
            send_to_char("Unknown program type.\n\r", ch);
            return;
         }
      }
      else
         mptype = -1;
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = 0;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value)
         {
            mpedit(ch, mprg, mptype, argument);
            xCLEAR_BITS(obj->pIndexData->progtypes);
            for (mprg = mprog; mprg; mprg = mprg->next)
               xSET_BIT(obj->pIndexData->progtypes, mprg->type);
            return;
         }
      }
      send_to_char("Program not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "delete"))
   {
      int num;
      bool found;

      if (!mprog)
      {
         send_to_char("That object has no obj programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg4);
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = 0;
      found = FALSE;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value)
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if (!found)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = num = 0;
      for (mprg = mprog; mprg; mprg = mprg->next)
         if (mprg->type == mptype)
            num++;
      if (value == 1)
      {
         mprg_next = obj->pIndexData->mudprogs;
         obj->pIndexData->mudprogs = mprg_next->next;
      }
      else
         for (mprg = mprog; mprg; mprg = mprg_next)
         {
            mprg_next = mprg->next;
            if (++cnt == (value - 1))
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if (mprg_next)
      {
         STRFREE(mprg_next->arglist);
         STRFREE(mprg_next->comlist);
         DISPOSE(mprg_next);
         if (num <= 1)
            xREMOVE_BIT(obj->pIndexData->progtypes, mptype);
         send_to_char("Program removed.\n\r", ch);
      }
      return;
   }

   if (!str_cmp(arg2, "insert"))
   {
      if (!mprog)
      {
         send_to_char("That object has no obj programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg4);
      mptype = get_mpflag(arg4);
      if (mptype == -1)
      {
         send_to_char("Unknown program type.\n\r", ch);
         return;
      }
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      if (value == 1)
      {
         CREATE(mprg, MPROG_DATA, 1);
         xSET_BIT(obj->pIndexData->progtypes, mptype);
         mpedit(ch, mprg, mptype, argument);
         mprg->next = mprog;
         obj->pIndexData->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value && mprg->next)
         {
            CREATE(mprg_next, MPROG_DATA, 1);
            xSET_BIT(obj->pIndexData->progtypes, mptype);
            mpedit(ch, mprg_next, mptype, argument);
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      send_to_char("Program not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg2, "add"))
   {
      mptype = get_mpflag(arg3);
      if (mptype == -1)
      {
         send_to_char("Unknown program type.\n\r", ch);
         return;
      }
      if (mprog != NULL)
         for (; mprog->next; mprog = mprog->next) ;
      CREATE(mprg, MPROG_DATA, 1);
      if (mprog)
         mprog->next = mprg;
      else
         obj->pIndexData->mudprogs = mprg;
      xSET_BIT(obj->pIndexData->progtypes, mptype);
      mpedit(ch, mprg, mptype, argument);
      mprg->next = NULL;
      return;
   }

   do_opedit(ch, "");
}



/*
 * RoomProg Support
 */
void rpedit(CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, char *argument)
{
   if (mptype != -1)
   {
      mprg->type = mptype;
      if (mprg->arglist)
         STRFREE(mprg->arglist);
      mprg->arglist = STRALLOC(argument);
   }
   ch->substate = SUB_MPROG_EDIT;
   ch->dest_buf = mprg;
   if (!mprg->comlist)
      mprg->comlist = STRALLOC("");
   start_editing(ch, mprg->comlist);
   /*shiver.. this function is actualy dead code.. */
   /*set_editor_desc( ch, "A roomprog of some kind.." ); dead code */
   return;
}

void do_rpedit(CHAR_DATA * ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char arg3[MIL];
   MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
   int value, mptype = -1, cnt;

   set_char_color(AT_PLAIN, ch);

   if (IS_NPC(ch))
   {
      send_to_char("Mob's can't rpedit\n\r", ch);
      return;
   }

   if (!ch->desc)
   {
      send_to_char("You have no descriptor\n\r", ch);
      return;
   }

   /* Mset/Oset/Redit On Mode check -- Stop most building crashes -- Xerves 8/7/99 */
   if ((xIS_SET(ch->act, PLR_MSET)) || (xIS_SET(ch->act, PLR_OSET)) || (xIS_SET(ch->act, PLR_REDIT)))
   {
      send_to_char("You need to turn mset/oset/redit off\n\r", ch);
      return;
   }

   switch (ch->substate)
   {
      default:
         break;
      case SUB_MPROG_EDIT:
         if (!ch->dest_buf)
         {
            send_to_char("Fatal error: report to Thoric.\n\r", ch);
            bug("do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0);
            ch->substate = SUB_NONE;
            return;
         }
         mprog = ch->dest_buf;
         if (mprog->comlist)
            STRFREE(mprog->comlist);
         mprog->comlist = copy_buffer(ch);
         stop_editing(ch);
         return;
   }

   smash_tilde(argument);
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   value = atoi(arg2);
   /* argument = one_argument( argument, arg3 ); */

   if (arg1[0] == '\0')
   {
      send_to_char("Syntax: rpedit <command> [number] <program> <value>\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("Command being one of:\n\r", ch);
      send_to_char("  add delete insert edit list\n\r", ch);
      send_to_char("Program being one of:\n\r", ch);
      send_to_char("  act speech rand sleep rest rfight enter\n\r", ch);
      send_to_char("  leave death\n\r", ch);
      send_to_char("\n\r", ch);
      send_to_char("You should be standing in room you wish to edit.\n\r", ch);
      return;
   }

   if (!can_rmodify(ch, ch->in_room))
      return;

   mprog = ch->in_room->mudprogs;

   set_char_color(AT_GREEN, ch);

   if (!str_cmp(arg1, "list"))
   {
      cnt = 0;
      if (!mprog)
      {
         send_to_char("This room has no room programs.\n\r", ch);
         return;
      }
      for (mprg = mprog; mprg; mprg = mprg->next)
         ch_printf(ch, "%d>%s %s\n\r%s\n\r", ++cnt, mprog_type_to_name(mprg->type), mprg->arglist, mprg->comlist);
      return;
   }

   if (!str_cmp(arg1, "edit"))
   {
      if (!mprog)
      {
         send_to_char("This room has no room programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg3);
      if (arg3[0] != '\0')
      {
         mptype = get_mpflag(arg3);
         if (mptype == -1)
         {
            send_to_char("Unknown program type.\n\r", ch);
            return;
         }
      }
      else
         mptype = -1;
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = 0;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value)
         {
            mpedit(ch, mprg, mptype, argument);
            xCLEAR_BITS(ch->in_room->progtypes);
            for (mprg = mprog; mprg; mprg = mprg->next)
               xSET_BIT(ch->in_room->progtypes, mprg->type);
            return;
         }
      }
      send_to_char("Program not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "delete"))
   {
      int num;
      bool found;

      if (!mprog)
      {
         send_to_char("That room has no room programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg3);
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = 0;
      found = FALSE;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value)
         {
            mptype = mprg->type;
            found = TRUE;
            break;
         }
      }
      if (!found)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      cnt = num = 0;
      for (mprg = mprog; mprg; mprg = mprg->next)
         if (mprg->type == mptype)
            num++;
      if (value == 1)
      {
         mprg_next = ch->in_room->mudprogs;
         ch->in_room->mudprogs = mprg_next->next;
      }
      else
         for (mprg = mprog; mprg; mprg = mprg_next)
         {
            mprg_next = mprg->next;
            if (++cnt == (value - 1))
            {
               mprg->next = mprg_next->next;
               break;
            }
         }
      if (mprg_next)
      {
         STRFREE(mprg_next->arglist);
         STRFREE(mprg_next->comlist);
         DISPOSE(mprg_next);
         if (num <= 1)
            xREMOVE_BIT(ch->in_room->progtypes, mptype);
         send_to_char("Program removed.\n\r", ch);
      }
      return;
   }

   if (!str_cmp(arg2, "insert"))
   {
      if (!mprog)
      {
         send_to_char("That room has no room programs.\n\r", ch);
         return;
      }
      argument = one_argument(argument, arg3);
      mptype = get_mpflag(arg2);
      if (mptype == -1)
      {
         send_to_char("Unknown program type.\n\r", ch);
         return;
      }
      if (value < 1)
      {
         send_to_char("Program not found.\n\r", ch);
         return;
      }
      if (value == 1)
      {
         CREATE(mprg, MPROG_DATA, 1);
         xSET_BIT(ch->in_room->progtypes, mptype);
         mpedit(ch, mprg, mptype, argument);
         mprg->next = mprog;
         ch->in_room->mudprogs = mprg;
         return;
      }
      cnt = 1;
      for (mprg = mprog; mprg; mprg = mprg->next)
      {
         if (++cnt == value && mprg->next)
         {
            CREATE(mprg_next, MPROG_DATA, 1);
            xSET_BIT(ch->in_room->progtypes, mptype);
            mpedit(ch, mprg_next, mptype, argument);
            mprg_next->next = mprg->next;
            mprg->next = mprg_next;
            return;
         }
      }
      send_to_char("Program not found.\n\r", ch);
      return;
   }

   if (!str_cmp(arg1, "add"))
   {
      mptype = get_mpflag(arg2);
      if (mptype == -1)
      {
         send_to_char("Unknown program type.\n\r", ch);
         return;
      }
      if (mprog)
         for (; mprog->next; mprog = mprog->next) ;
      CREATE(mprg, MPROG_DATA, 1);
      if (mprog)
         mprog->next = mprg;
      else
         ch->in_room->mudprogs = mprg;
      xSET_BIT(ch->in_room->progtypes, mptype);
      mpedit(ch, mprg, mptype, argument);
      mprg->next = NULL;
      return;
   }

   do_rpedit(ch, "");
}

void do_rdelete(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   ROOM_INDEX_DATA *location;

   argument = one_argument(argument, arg);

   /* Temporarily disable this command. */
/*    return;*/

   if (arg[0] == '\0')
   {
      send_to_char("Delete which room?\n\r", ch);
      return;
   }

   /* Find the room. */
   if ((location = find_location(ch, arg)) == NULL)
   {
      send_to_char("No such location.\n\r", ch);
      return;
   }

   /* Does the player have the right to delete this room? */
   if (get_trust(ch) < sysdata.level_modify_proto && (location->vnum < ch->pcdata->r_range_lo || location->vnum > ch->pcdata->r_range_hi))
   {
      send_to_char("That room is not in your assigned range.\n\r", ch);
      return;
   }

   /* We could go to the trouble of clearing out the room, but why? */
   /* Delete_room does that anyway, but this is probably safer:) */
   if (location->first_person || location->first_content)
   {
      send_to_char("The room must be empty first.\n\r", ch);
      return;
   }

   /* Ok, we've determined that the room exists, it is empty and the 
      player has the authority to delete it, so let's dump the thing. 
      The function to do it is in db.c so it can access the top-room 
      variable. */
   delete_room(location);

   send_to_char("Room deleted.\n\r", ch);
   return;
}

void do_odelete(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   OBJ_INDEX_DATA *obj;
   OBJ_DATA *temp;
   DESCRIPTOR_DATA *d;

   argument = one_argument(argument, arg);

   /* Temporarily disable this command. */
/*    return;*/

   if (arg[0] == '\0')
   {
      send_to_char("Delete which object?\n\r", ch);
      return;
   }

   /* Find the object. */
   if (!(obj = get_obj_index(atoi(arg))))
   {
      if (!(temp = get_obj_here(ch, arg)))
      {
         send_to_char("No such object.\n\r", ch);
         return;
      }
      obj = temp->pIndexData;
   }

   /* Does the player have the right to delete this room? */
   if (get_trust(ch) < sysdata.level_modify_proto && (obj->vnum < ch->pcdata->o_range_lo || obj->vnum > ch->pcdata->o_range_hi))
   {
      send_to_char("That object is not in your assigned range.\n\r", ch);
      return;
   }
   
   for (d = first_descriptor; d; d = d->next)
   {
      OBJ_DATA *dobj;
      if (d->character && d->character->substate && d->character->dest_buf)
      {
         dobj = d->character->dest_buf;
         if ((d->character->substate == SUB_RESTRICTED || d->character->substate == SUB_REPEATCMD)
         &&  dobj->pIndexData && dobj->pIndexData == obj)
         {
            break;
         }
      }
   }
   if (d)
   {
      send_to_char("That object is currently being modified by a player, try again later.\n\r", ch);
      return;
   }
   /* Ok, we've determined that the room exists, it is empty and the 
      player has the authority to delete it, so let's dump the thing.
      The function to do it is in db.c so it can access the top-room 
      variable. */
   delete_obj(obj);

   send_to_char("Object deleted.\n\r", ch);
   return;
}

void do_mdelete(CHAR_DATA * ch, char *argument)
{
   char arg[MIL];
   DESCRIPTOR_DATA *d;
   MOB_INDEX_DATA *mob;
   CHAR_DATA *temp;

   argument = one_argument(argument, arg);

   /* Temporarily disable this command. */
/*    return;*/

   if (arg[0] == '\0')
   {
      send_to_char("Delete which mob?\n\r", ch);
      return;
   }

   /* Find the mob. */
   if (!(mob = get_mob_index(atoi(arg))))
   {
      if (!(temp = get_char_room_new(ch, arg, 1)) || !IS_NPC(temp))
      {
         send_to_char("No such mob.\n\r", ch);
         return;
      }
      mob = temp->pIndexData;
   }

   /* Does the player have the right to delete this room? */
   if (get_trust(ch) < sysdata.level_modify_proto && (mob->vnum < ch->pcdata->m_range_lo || mob->vnum > ch->pcdata->m_range_hi))
   {
      send_to_char("That mob is not in your assigned range.\n\r", ch);
      return;
   }

   for (d = first_descriptor; d; d = d->next)
   {
      CHAR_DATA *dch;
      if (d->character && d->character->substate && d->character->dest_buf)
      {
         dch = d->character->dest_buf;
         if ((d->character->substate == SUB_RESTRICTED || d->character->substate == SUB_REPEATCMD)
         &&  dch->pIndexData && dch->pIndexData == mob)
         {
            break;
         }
      }
   }
   if (d)
   {
      send_to_char("That mobile is currently being modified by a player, try again later.\n\r", ch);
      return;
   }
   
   /* Ok, we've determined that the mob exists and the player has the
      authority to delete it, so let's dump the thing.
      The function to do it is in db.c so it can access the top_mob_index
      variable. */
   delete_mob(mob);

   send_to_char("Mob deleted.\n\r", ch);
   return;
}

char *const weathv[10] = {
   "&g", "&w&W", "&c&w", "&C", "&c", "&z", "&Y", "&O", "&R", "&r"
};

/*
 * function to allow modification of an area's climate
 * Last modified: July 15, 1997
 * Fireblade
 */
void do_climate(CHAR_DATA * ch, char *argument)
{
   FRONT_DATA *fnt;
   int sx;
   int sy;
   int x;
   int y;
   int map;
   char arg1[MIL];
   char arg2[MIL];

   if (ch->coord->x < 1 || ch->coord->y < 1 || ch->map < 0)
   {
      if (ch->in_room->area->x < 1 || ch->in_room->area->y < 1 || ch->in_room->map < 0)
      {
         send_to_char("This area needs weather adjustments, please tell an imm.\n\r", ch);
         return;
      }
      x = ch->in_room->area->x;
      y = ch->in_room->area->y;
      map = ch->in_room->area->map;
   }
   else
   {
      x = ch->coord->x;
      y = ch->coord->y;
      map = ch->map;
   }

   if (argument[0] == '\0')
   {
      for (fnt = first_front; fnt; fnt = fnt->next)
      {
         ch_printf(ch, "Front  (%d) Map[%d]  %d  %d\n\r", fnt->type, fnt->map, fnt->x, fnt->y);
      }
      send_to_char("----------------------------------------------------------------------\n\r", ch);
      for (sy = y - 40; sy <= y + 40; sy++)
      {
         if (sy < 1 || sy > MAX_Y)
            continue;
         send_to_char("\n\r", ch);
         for (sx = x - 40; sx <= x + 40; sx++)
         {
            if (sx < 1 || sx > MAX_X)
               continue;
            if (sx == x && sy == y)
               send_to_char("&PX", ch);
            else
            {
               if (sx == x - 40 || (sx == x + 1 && sy == sy))
                  ch_printf(ch, "%s%d", weathv[weather_sector[map][sx][sy] % 10], weather_sector[map][sx][sy] % 10);
               else if (weather_sector[map][sx][sy] % 10 != weather_sector[map][sx - 1][sy] % 10)
                  ch_printf(ch, "%s%d", weathv[weather_sector[map][sx][sy] % 10], weather_sector[map][sx][sy] % 10);
               else
                  ch_printf(ch, "%d", weather_sector[map][sx][sy] % 10);
            }
         }
      }
      send_to_char("\n\r", ch);
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   if (!str_cmp(arg1, "tornado"))
   {
      TORNADO_DATA *torn;

      if (atoi(arg2) == 0 || atoi(argument) == 0)
      {
         send_to_char("Syntax: climate tornado <x> <y>\n\r", ch);
         return;
      }
      if (atoi(arg2) < 1 || atoi(arg2) > MAX_X || atoi(argument) < 1 || atoi(argument) > MAX_Y)
      {
         ch_printf(ch, "1 to %d is the range", MAX_X);
         return;
      }
      CREATE(torn, TORNADO_DATA, 1);
      torn->x = atoi(arg2);
      torn->y = atoi(argument);
      torn->map = map;
      torn->power = 18;
      torn->turns = 43;
      torn->dir = number_range(0, 3);
      LINK(torn, first_tornado, last_tornado, next, prev);
      send_to_char("A tornado has been created.\n\r", ch);
      bug("%s has created a tornado at %d %d  Map%d", ch->name, torn->x, torn->y, torn->map);
   }
   if (!str_cmp(arg1, "coldfront") || !str_cmp(arg1, "warmfront"))
   {
      FRONT_DATA *fnt;
      int cnt;
      int pc;
      int ttype;

      CREATE(fnt, FRONT_DATA, 1);
      if (!str_cmp(arg1, "coldfront"))
         fnt->type = 0;
      else
         fnt->type = 1;
      fnt->x = x;
      fnt->y = y;
      fnt->map = map;
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
      send_to_char("A front has been created at your location.\n\r", ch);
   }
   return;
}
