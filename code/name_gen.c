
/****************************************************************************/
/* NAMN.C - Create random names.                                            */
/* Somewhat modified for CGI-option...                                      */
/****************************************************************************/
/* Johan Danforth of http://spitfire.ausys.se/johan/workshop/               */
/*                                                                          */
/* version 2.0                                                              */
/* syntax: namn <rule-file> <nr of names to create>                         */
/*         namn alver.nam 100                                               */
/*                                                                          */
/* Default extension for rule-files are  *.nam                              */
/*                                                                          */
/****************************************************************************/

/****************************************************************************
 * Modified by Jack (June 2002)
 *
 * + Better ANSI C compliance (will compile on other OSs/compilers!!
 * + Handles files in both UNIX & the other line ending types
 * + More paranoid about memory usage & more tolerant of errors in nam file
 *
 ****************************************************************************/
 
/****************************************************************************
 * Modified by Xerves (Sept 2004) for Rafermand support
 ****************************************************************************/ 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mud.h"

/****************************************************************************/
/* Compile Time parameters                                                  */
/****************************************************************************/

#define SYLLABLES_PER_SECTION 100 //Number of options per section
#define SYLLABLE_LENGTH       12  //Max length of a single section
#define NAME_LENGTH           36  //Max length of the name

#define NAME_DEVIL_FEMALE          NAMESET_DIR "devil_female.nam"
#define NAME_DEVIL_MALE            NAMESET_DIR "devil_male.nam"
#define NAME_DWARF_FEMALE          NAMESET_DIR "dwarf_female.nam"
#define NAME_DWARF_MALE            NAMESET_DIR "dwarf_male.nam"
#define NAME_ELF_FEMALE            NAMESET_DIR "elf_female.nam"
#define NAME_ELF_MALE              NAMESET_DIR "elf_male.nam"
#define NAME_FAIRY_FEMALE          NAMESET_DIR "fairy_female.nam"
#define NAME_FAIRY_MALE            NAMESET_DIR "fairy_male.nam"
#define NAME_HUMAN_FEMALE          NAMESET_DIR "human_female.nam"
#define NAME_HUMAN_MALE            NAMESET_DIR "human_male.nam"
#define NAME_HOBBIT_FEMALE         NAMESET_DIR "hobbit_female.nam"
#define NAME_HOBBIT_MALE           NAMESET_DIR "hobbit_male.nam"
#define NAME_OGRE_FEMALE           NAMESET_DIR "ogre_female.nam"
#define NAME_OGRE_MALE             NAMESET_DIR "ogre_male.nam"
#define NAME_FELINE                NAMESET_DIR "feline.nam"

void get_generate_filename(int race, int sex, char *filename)
{
   if (race == 1 && sex == 1)
      sprintf(filename, NAME_ELF_MALE);
   else if (race == 1 && sex == 2)
      sprintf(filename, NAME_ELF_FEMALE);
   else if (race == 2 && sex == 1)
      sprintf(filename, NAME_DWARF_MALE);
   else if (race == 2 && sex == 2)
      sprintf(filename, NAME_DWARF_FEMALE);
   else if (race == 3 && sex == 1)
      sprintf(filename, NAME_OGRE_MALE);
   else if (race == 3 && sex == 2)
      sprintf(filename, NAME_OGRE_FEMALE);
   else if (race == 4 && sex == 1)
      sprintf(filename, NAME_HOBBIT_MALE);
   else if (race == 4 && sex == 2)
      sprintf(filename, NAME_HOBBIT_FEMALE);
   else if (race == 5 && sex == 1)
      sprintf(filename, NAME_FAIRY_MALE);
   else if (race == 5 && sex == 2)
      sprintf(filename, NAME_FAIRY_FEMALE);      
   else if (sex == 1)
      sprintf(filename, NAME_HUMAN_MALE);
   else
      sprintf(filename, NAME_HUMAN_FEMALE);
   return;
}

//Only need to generate 1 name at a time.  Need to supply the race and sex so the
//proper rulefile can be used.  Lastly, the value is copied into the buffer supplied
//so that the command can be called numerious times if needed
void generate_randomname(int race, int sex, char *buffer)
{
  time_t t;
  int gstart = 0;
  int gmiddle = 0;
  int gend = 0;
  int cnt = 0;
  char tempstring[151];
  char filename[256];
  char start[SYLLABLES_PER_SECTION][SYLLABLE_LENGTH];                      /* start syllable               */
  char middle[SYLLABLES_PER_SECTION][SYLLABLE_LENGTH];                     /* middle syllable              */
  char end[SYLLABLES_PER_SECTION][SYLLABLE_LENGTH];                        /* ending syllable              */
  char name[NAME_LENGTH];                                                  /* name                         */
  FILE *fp;

  memset (start, 0, SYLLABLES_PER_SECTION*SYLLABLE_LENGTH);
  memset (middle, 0, SYLLABLES_PER_SECTION*SYLLABLE_LENGTH);
  memset (end, 0, SYLLABLES_PER_SECTION*SYLLABLE_LENGTH);
  memset (name, 0, NAME_LENGTH);

  strcpy(buffer, "");
  if (race < 0 || race >= max_npc_race)
  {
     bug("generate_randomname: invalid race of %d", race);
     return;
  }     
  if (sex == 0)
     sex = 1; //Set a male name if sex is 0
  get_generate_filename(race, sex, filename);
  fp = fopen(filename, "r");   
  if (fp == NULL)             
  {
	  bug("Cannot open file: %s",filename);
	  return;
  }

  /* read file until [start] it found (starting syllable)                 */
  while (strcmp(tempstring, "[start]") != 0)
  {
     if (++cnt == SYLLABLES_PER_SECTION)
     {
        bug("generate_random_name:  Aborting before [start] in %s", filename);
        return;
     }
     memset (tempstring, 0, sizeof(tempstring));
     fgets(tempstring, 150, fp);
     if (tempstring[strlen(tempstring)-2] == '\r') 
     {
        tempstring[strlen(tempstring)-2] = '\0';
     }
     else
  	    tempstring[strlen(tempstring)-1] = '\0';/* remove linefeed          */
  }
  cnt = 0;
  /* read file until [middle] is found (middle syllable)                    */
  while (strcmp(tempstring, "[middle]") != 0)
  {
     if (++cnt == SYLLABLES_PER_SECTION)
     {
        bug("generate_random_name:  Aborting before [middle] in %s", filename);
        return;
     }
     memset (tempstring, 0, sizeof(tempstring));
     fgets(tempstring, 150, fp);
     if (tempstring[strlen(tempstring)-2] == '\r') 
     {
	    tempstring[strlen(tempstring)-2] = '\0';
     }
     else
	    tempstring[strlen(tempstring)-1] = '\0';/* remove linefeed          */
     if ((tempstring[0] != '/') && (tempstring[0] != '['))
	 {
	    strncpy(start[gstart], tempstring, strlen(tempstring));
        gstart++;
	 }
  }
  cnt = 0;
  /* read file until [end] is found (ending syllable)                    */
  while (strcmp(tempstring, "[end]") != 0)
  {
     if (++cnt == SYLLABLES_PER_SECTION)
     {
        bug("generate_random_name:  Aborting before [end] in %s", filename);
        return;
     }
     memset (tempstring, 0, sizeof(tempstring));
     fgets(tempstring, 150, fp);
     if (tempstring[strlen(tempstring)-2] == '\r') 
     {
	    tempstring[strlen(tempstring)-2] = '\0';
     }
     else
	    tempstring[strlen(tempstring)-1] = '\0';/* remove linefeed          */
     if ((tempstring[0] != '/') && (tempstring[0] != '['))
	 {
        strncpy(middle[gmiddle], tempstring, strlen(tempstring));
	    gmiddle++;
  	 }
  }
  cnt = 0;
  /* read file until [stop] is found (end of syllables)                       */
  while (strcmp(tempstring, "[stop]") != 0)
  {
     if (++cnt == SYLLABLES_PER_SECTION)
     {
        bug("generate_random_name:  Aborting before [stop] in %s", filename);
        return;
     }
     memset (tempstring, 0, sizeof(tempstring));
     fgets(tempstring, 150, fp);      
     if (tempstring[strlen(tempstring)-2] == '\r') 
     { 
	    tempstring[strlen(tempstring)-2] = '\0';
     }
     else
	    tempstring[strlen(tempstring)-1] = '\0';/* remove linefeed          */
     if ((tempstring[0] != '/') && (tempstring[0] != '['))
	 {
	    strncpy(end[gend], tempstring, strlen(tempstring));
	    gend++;
	 } 
  } 
  fclose(fp);                         
  srand((unsigned)time(&t));                /* Kick on the rand-generator... */
  
  /* correct the number of available syllables...                                 */
  
  gstart--;
  gmiddle--;
  gend--;

  strcpy(name, start[number_range(0, gstart)]);    /* get a start                  */
  strcat(name, middle[number_range(0, gmiddle)]);  /* get a middle                 */
  strcat(name, end[number_range(0, gend)]);        /* get an ending                */
  sprintf(buffer, "%s", name);
  return;                  
} 

void do_generatename(CHAR_DATA *ch, char *argument)
{
   char arg1[MIL];
   char arg2[MIL];
   char buf[MSL];
   int race;
   int sex;
   int cnt = 0;
   int x;
   
   if (argument[0] == '\0')
   {
      send_to_char("Syntax:  generatename <race> <sex> [#names]\n\r", ch);
      return;
   }
   argument = one_argument(argument, arg1);
   argument = one_argument(argument, arg2);
   race = atoi(arg1);
   if (race < 0 || race >= max_npc_race)
   {
      ch_printf(ch, "Range is 0 to %d", max_npc_race);
      return;
   }
   sex = atoi(arg2);
   if (sex < 1 || sex > 2)
   {
      send_to_char("Your choices are 1 for male or 2 for female.\n\r", ch);
      return;
   }
   cnt = atoi(argument);
   if (cnt < 0 || cnt > 20)
   {
      send_to_char("Range for # of names is 0 to 20.\n\r", ch);
      return;
   }
   if (cnt < 1)
      cnt = 1;
   for (x = 1; x <= cnt; x++)
   {
      generate_randomname(race, sex, buf);
      ch_printf(ch, "%-15.15s  ", buf);
      if (x % 5 == 0)
         send_to_char("\n\r", ch);
   }
   if (--x % 5 != 0)
      send_to_char("\n\r", ch);
   return;
}
   
   

