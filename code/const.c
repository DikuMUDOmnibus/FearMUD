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
 *			     Mud constants module			    *
 ****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"


/* undef these at EOF */
#define AM 95
#define AC 95
#define AT 85
#define AW 85
#define AV 95
#define AD 95
#define AR 90
#define AA 95

char *const npc_class[MAX_NPC_CLASS] = {
   "mage", "cleric", "thief", "warrior", "ranger", "paladin", "monk",
   "swordsman", "chaplain", "bandit", "swordsman", "pc11", "pc12", "pc13",
   "pc14", "pc15", "pc16", "pc17", "savage", "pirate",
   "baker", "butcher", "blacksmith", "mayor", "king", "queen"
};

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[26] = {
   {-5, -4, 0, 0, 12}, /* 0  */
   {-5, -4, 3, 1, 14}, /* 1  */
   {-3, -2, 3, 2, 16},
   {-3, -1, 10, 3, 18}, /* 3  */
   {-2, -1, 25, 4, 20},
   {-2, -1, 55, 5, 22}, /* 5  */
   {-1, 0, 80, 6, 25},
   {-1, 0, 90, 7, 27},
   {0, 0, 100, 8, 30},
   {0, 0, 100, 9, 32},
   {0, 0, 115, 10, 35}, /* 10  */
   {0, 0, 130, 11, 37},
   {0, 0, 210, 12, 39},   //Fairy High Str
   {0, 1, 240, 13, 49}, /* 13  */
   {1, 1, 270, 15, 59},   //Hobbit High Str
   {1, 1, 300, 17, 72}, /* 15  */
   {2, 2, 330, 19, 85},   //Elf High Str
   {3, 3, 360, 21, 102},   //Human High Str
   {3, 3, 390, 24, 110}, /* 18  */
   {4, 4, 420, 27, 119},  //Dwarf High Str
   {4, 4, 450, 30, 130}, /* 20  */
   {5, 5, 480, 34, 140},
   {6, 6, 550, 39, 152},  //Ogre High Str
   {7, 6, 610, 45, 165},
   {8, 7, 680, 51, 170},
   {10, 8, 750, 60, 185} /* 25   */
};


 /* Second is for Lore */
const struct int_app_type int_app[26] = {
   {20, -2}, /*  0 */
   {21, -2}, /*  1 */
   {22, -2},
   {22, -2}, /*  3 */
   {23, -2},
   {24, -2}, /*  5 */
   {24, -1},
   {25, -1},
   {25, -1},
   {26, -1},
   {28, 0}, /* 10 */
   {30, 0},
   {31, 0},
   {33, 0},
   {35, 0},
   {38, 1}, /* 15 */
   {40, 1},
   {43, 1},
   {46, 1}, /* 18 */
   {47, 1},
   {50, 2}, /* 20 */
   {51, 2},
   {55, 2},
   {60, 2},
   {65, 2},
   {75, 3} /* 25 */
};


   /* Second is for lore */
const struct wis_app_type wis_app[26] = {
   {0, -2}, /*  0 */
   {0, -2}, /*  1 */
   {0, -2},
   {0, -2}, /*  3 */
   {0, -2},
   {0, -2}, /*  5 */
   {0, -1},
   {0, -1},
   {0, -1},
   {0, -1},
   {1, 0}, /* 10 */
   {1, 0},
   {1, 0},
   {1, 0},
   {1, 0},
   {1, 1}, /* 15 */
   {1, 1},
   {1, 1},
   {1, 1}, /* 18 */
   {1, 1},
   {1, 2}, /* 20 */
   {1, 2},
   {1, 2},
   {2, 2},
   {2, 2},
   {3, 3} /* 25 */
};


//No longer used -- Xerves
const struct dex_app_type dex_app[26] = {
   {60}, /* 0 */
   {50}, /* 1 */
   {50},
   {40},
   {30},
   {20}, /* 5 */
   {10},
   {0},
   {0},
   {0},
   {0}, /* 10 */
   {0},
   {0},
   {0},
   {0},
   {-10}, /* 15 */
   {-15},
   {-20},
   {-30},
   {-40},
   {-50}, /* 20 */
   {-60},
   {-75},
   {-90},
   {-105},
   {-120} /* 25 */
};



const struct con_app_type con_app[26] = {
   {-4, 20}, /*  0 */
   {-3, 25}, /*  1 */
   {-2, 30},
   {-2, 35}, /*  3 */
   {-1, 40},
   {-1, 45}, /*  5 */
   {-1, 50},
   {0, 55},
   {0, 60},
   {0, 65},
   {0, 70}, /* 10 */
   {0, 75},
   {0, 80},
   {0, 85},
   {0, 88},
   {1, 90}, /* 15 */
   {2, 95},
   {2, 97},
   {3, 99}, /* 18 */
   {3, 99},
   {4, 99}, /* 20 */
   {4, 99},
   {5, 99},
   {6, 99},
   {7, 99},
   {8, 99} /* 25 */
};


const struct cha_app_type cha_app[26] = {
   {-60}, /* 0 */
   {-50}, /* 1 */
   {-50},
   {-40},
   {-30},
   {-20}, /* 5 */
   {-10},
   {-5},
   {-1},
   {0},
   {0}, /* 10 */
   {0},
   {0},
   {0},
   {1},
   {5}, /* 15 */
   {10},
   {20},
   {30},
   {40},
   {50}, /* 20 */
   {60},
   {70},
   {80},
   {90},
   {99} /* 25 */
};

/* Have to fix this up - not exactly sure how it works (Scryn) */
const struct lck_app_type lck_app[26] = {
   {60}, /* 0 */
   {50}, /* 1 */
   {50},
   {40},
   {30},
   {20}, /* 5 */
   {10},
   {0},
   {0},
   {0},
   {0}, /* 10 */
   {0},
   {0},
   {0},
   {0},
   {-10}, /* 15 */
   {-15},
   {-20},
   {-30},
   {-40},
   {-50}, /* 20 */
   {-60},
   {-75},
   {-90},
   {-105},
   {-120} /* 25 */
};


/*
 * Liquid properties.
 * Used in #OBJECT section of area file.
 */
const struct liq_type liq_table[LIQ_MAX] = {
   {"water", "clear", {0, 1, 10}}, /*  0 */
   {"beer", "amber", {3, 2, 5}},
   {"wine", "rose", {5, 2, 5}},
   {"ale", "brown", {2, 2, 5}},
   {"dark ale", "dark", {1, 2, 5}},

   {"whisky", "golden", {6, 1, 4}}, /*  5 */
   {"lemonade", "pink", {0, 1, 8}},
   {"firebreather", "boiling", {10, 0, 0}},
   {"local specialty", "everclear", {3, 3, 3}},
   {"slime mold juice", "green", {0, 4, -8}},

   {"milk", "white", {0, 3, 6}}, /* 10 */
   {"tea", "tan", {0, 1, 6}},
   {"coffee", "black", {0, 1, 6}},
   {"blood", "red", {0, 2, -1}},
   {"salt water", "clear", {0, 1, -2}},

   {"cola", "cherry", {0, 1, 5}}, /* 15 */
   {"mead", "honey color", {4, 2, 5}}, /* 16 */
   {"grog", "thick brown", {3, 2, 5}} /* 17 */
};

char *const attack_table[1] = {
   "hit"
};

char *s_blade_messages[24] = {
   "miss", "barely scratch", "scratch", "nick", "cut", "hit",
   "tear", "rip", "gash", "lacerate", "hack", "maul",
   "rend", "decimate", "_mangle_", "_devastate_", "_cleave_",
   "_butcher_", "DISEMBOWEL", "DISFIGURE", "GUT",
   "EVISCERATE", "* SLAUGHTER *", "*** ANNIHILATE ***"
};

char *p_blade_messages[24] = {
   "misses", "barely scratches", "scratches", "nicks", "cuts",
   "hits", "tears", "rips", "gashes", "lacerates", "hacks",
   "mauls", "rends", "decimates", "_mangles_", "_devastates_",
   "_cleaves_", "_butchers_", "DISEMBOWELS", "DISFIGURES",
   "GUTS", "EVISCERATES", "* SLAUGHTERS *", "*** ANNIHILATES ***"
};

char *s_blunt_messages[24] = {
   "miss", "barely scuff", "scuff", "pelt", "bruise", "strike",
   "thrash", "batter", "flog", "pummel", "smash", "maul",
   "bludgeon", "decimate", "_shatter_", "_devastate_", "_maim_",
   "_cripple_", "MUTILATE", "DISFIGURE", "MASSACRE", "PULVERIZE",
   "* OBLITERATE *", "*** ANNIHILATE ***"
};

char *p_blunt_messages[24] = {
   "misses", "barely scuffs", "scuffs", "pelts", "bruises",
   "strikes", "thrashes", "batters", "flogs", "pummels",
   "smashes", "mauls", "bludgeons", "decimates", "_shatters_",
   "_devastates_", "_maims_", "_cripples_", "MUTILATES",
   "DISFIGURES", "MASSACRES", "PULVERIZES", "* OBLITERATES *",
   "*** ANNIHILATES ***"
};

char *s_generic_messages[24] = {
   "miss", "brush", "scratch", "graze", "nick", "jolt",
   "wound", "injure", "hit", "jar", "thrash", "maul",
   "decimate", "_traumatize_", "_devastate_", "_maim_",
   "_demolish_", "MUTILATE", "MASSACRE", "PULVERIZE",
   "DESTROY", "* OBLITERATE *", "*** ANNIHILATE ***",
   "**** SMITE ****"
};

char *p_generic_messages[24] = {
   "misses", "brushes", "scratches", "grazes", "nicks", "jolts",
   "wounds", "injures", "hits", "jars", "thrashes", "mauls",
   "decimates", "_traumatizes_", "_devastates_", "_maims_",
   "_demolishes_ ", "MUTILATES", "MASSACRES", "PULVERIZES",
   "DESTROYS", "* OBLITERATES *", "*** ANNIHILATES ***",
   "**** SMITES ****"
};

char **const s_message_table[1] = {
   s_generic_messages /* hit */
};

char **const p_message_table[1] = {
   p_generic_messages /* hit */
};

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections.
 */
#define SLOT(n)	n
#define LI LEVEL_IMMORTAL

#undef AM
#undef AC
#undef AT
#undef AW
#undef AV
#undef AD
#undef AR
#undef AA

#undef LI
