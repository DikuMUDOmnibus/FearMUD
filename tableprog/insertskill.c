/*  InsertSkill by Joshua Halls (AKA Xerves)
    Copyright (C) 2000  Joshua Halls

    GPL has been revoked by the author    
    
    Please Read the HOWTO and REQUESTS files before doing anything else.
    insertskill.c - Used to sort and insort a skill into the tables and
                    header file.  Please read the HOWTO file for more info
                    on how to use this program.    */
                    
  
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "central.h"

int charused[26]; //Make sure for the case statements to add the character
char space[35];
int is_first_line;
int commnt; //For comment to let the code know
int addone; //Add one, used to make the program go a bit faster and work right
char skillbuf[BUFSIZE];
char searchbuf[BUFSIZE];
char *rch;

//outputs some info to a file called bugs.txt
void bug(char *string)
{
   FILE *fp;
   
   fp = fopen( "bugs.txt", "a");
   fprintf(fp, "%s\n", string);
   fclose(fp);
   return;
}

int line_empty(char *buf)
{
   int x = 0;
   
   for(;;)
   {
      if (buf[x] == '\0' || buf[x] == '\n')
         return 1;
      if (isalnum(buf[x]))
         return 0;
      x++;
   }
   return 1;
}
// get the skill name in the header file (mud.h, merc.h, etc)
char *get_dofun_name(char *skill, int type)
{
   int x = 0;
   int y = 0;
   char c;
   int fnd;
   char *pbuf;
   
   for (x=0; x < BUFSIZE/2; x++)
   {
      if (type == 0)
         skillbuf[x] = 0;
      else
         searchbuf[x] = 0;
   }
   x = 0;
   for (;;)
   {	
      if (skill[x] == '\0')
      {
         commnt = 0;
         return NULL;
      }
      //Check for comments
      if (skill[x+1] && type == 0)
      {
         if (skill[x] == '/' && skill[x+1] == '/')
            commnt = 1;
      }  
      if (skill[x] == '(') //Check for the beginning (
      {
         for (;;)
         {
            if (skill[++x] != '\0')
               c = skill[x];
            else
            {
               if (type == 0)
               {
                  pbuf = &skillbuf[0];
                  if (skillbuf[0] != 'd' || skillbuf[1] != 'o' || skillbuf[2] != '_')
                  {
                     commnt = 0;
                     pbuf = NULL;
                  }
               }
               else
               {
                  pbuf = &searchbuf[0];
                  if (searchbuf[0] != 'd' || searchbuf[1] != 'o' || searchbuf[2] != '_')
                    pbuf = NULL;
               }
               return pbuf;
            }   
            if ((isspace(c) && fnd == 1) || skill[x] == ')')
            {
               if (type == 0)
               {
                  pbuf = &skillbuf[0];
                  if (skillbuf[0] != 'd' || skillbuf[1] != 'o' || skillbuf[2] != '_')
                  {
                     commnt = 0;
                     pbuf = NULL;
                  }
               }
               else
               {
                  pbuf = &searchbuf[0];
                  if (searchbuf[0] != 'd' || searchbuf[1] != 'o' || searchbuf[2] != '_')
                    pbuf = NULL;
               }
               return pbuf;
            }
            if (!isspace(c) && c != ')' && c != '(')
            {
               if (type == 0)
                  skillbuf[y++] = c;
               else
                  searchbuf[y++] = c;
               fnd = 0;
            }
            continue;
         }
      }
      //nothing found, next character please
      x++;
      continue;
   }
}
// gets the skill name for tables.c
char *get_skill_name(char *skill, int type)
{
   int x = 0;
   int y = 0;
   char c;
   int fnd;
   char *pbuf;   
   
   for (x=0; x < BUFSIZE; x++)
   {
      if (type == 0 || type == 2)
         skillbuf[x] = 0;
      else
         searchbuf[x] = 0;
   }
   x = 0;
   for (;;)
   {	
      if (skill[x] == '\0')
      {
         return NULL;
      }
      if (skill[x+1] && (type == 0 || type == 2))
      {
         if (skill[x] == '/' && skill[x+1] == '/')
            commnt = 1;
      }
      fnd = 0;
      y = 0;
      //check for eol, mainly to make sure there is enough room for the return
      for (;;)
      {
         if (y == 6)
            break;
         if (skill[y++] == '\0')
         {
            fnd = 1;
            break;
         }
      }
      if (fnd)
      {
         x++;
         continue;
      }
      //Check for the return statement, after that, start parsing the string for the
      //name
      if (skill[x] == 'r' && skill[x+1] == 'e' && skill[x+2] == 't' 
      && skill[x+3] == 'u' &&  skill[x+4] == 'r' && skill[x+5] == 'n')
      {
         //make sure it is not "do_return"
         if ((skill[x-4] == 34 || skill[x-4] == 39) && skill[x-3] == 'd' 
         && skill[x-2] == 'o' && skill[x-1] == '_' && (type == 0 || type == 1))
         {
            x++;
            continue;
         }
         //check for the skill before it
         if (skill[x-12] == 's' && skill[x-11] == 'k' && skill[x-10] == 'i'
         &&  skill[x-9] == 'l' && skill[x-8] == 'l' && (type == 2 || type == 3))
         {
            x++;
            continue;
         }
         x = x+5; //move the pointer up to the n
         fnd = 0;
         y = 0;
         for (;;)
         {
            if (skill[++x] != '\0')
               c = skill[x];
            else
            {
               if (type == 0 || type == 2)
               {
                  pbuf = &skillbuf[0];
                  if (skillbuf[0] != 'd' || skillbuf[1] != 'o' || skillbuf[2] != '_')
                  {
                     pbuf = NULL;
                     commnt = 0;
                  }
               }
               else
               {
                  pbuf = &searchbuf[0];
                  if (searchbuf[0] != 'd' || searchbuf[1] != 'o' || searchbuf[2] != '_')
                     pbuf = NULL;
               }
               return pbuf;
            }
            if (c == 34 && (type == 0 || type == 1))
            {
               commnt = 0;
               return NULL;
            }  
            if ((isspace(c) && fnd == 1) || skill[x] == 59 || (skill[x] == 34 && fnd == 1))
            {
               if (skill[x] != 34 && (type == 2 || type == 3))
               {
                  commnt = 0;
                  return NULL;
               }	
               if (type == 0 || type == 2)
               {
                  pbuf = &skillbuf[0];
                  if (skillbuf[0] != 'd' || skillbuf[1] != 'o' || skillbuf[2] != '_')
                  {
                     pbuf = NULL;
                     commnt = 0;
                  }
               }
               else
               {
                  pbuf = &searchbuf[0];
                  if (searchbuf[0] != 'd' || searchbuf[1] != 'o' || searchbuf[2] != '_')
                     pbuf = NULL;
               }
               return pbuf;
            }
            if (!isspace(c) && c != 34 && c != 39)
            {
               if (type == 0 || type == 2)
                  skillbuf[y++] = c;
               else
                  searchbuf[y++] = c;
               fnd = 1;
            }
            continue;
         }
      }
      //nothing found, next character please
      x++;
      continue;
   }
}

char get_table_string(char *buf) //kind of like get_table_string but will look for case statements
{
   int x = 0;
   char one;
   
   if (strstr(buf, "case"))
   {
      for(;;)
      {
         if (buf[x] == '\0')
            return 0;
         if (buf[x] == 34 || buf[x] == 39) // ' and " check, only way I could do it
         {
            x++;
            for (;;)
            {
               if (!isspace(buf[x]))
               {
                  one = buf[x];
                  return one;
               }
               if (buf[x] == '\0')
                  return 0;
               x++;
            }
          }
          x++;
       }
    }  
    return 0;
}
            
     
int no_eol(char *skill)
{
   if (skill[0] == '\0' || skill[1] == '\0' || skill[2] == '\0' || skill[3] == '\0' || skill == NULL)
      return 0;
   return 1;
}

char *get_space(char *skill)
{
   int len;
   int x;
   char *spt;
   
   space[0] = '\0';
   
   len = (30 - strlen(skill) - 1);
   for (x = 1; x < len; x++)
   {
      strcat(space, " ");
   }
   spt = space;
   return spt;
}

//actually write the file now
void write_file(int cnt, int type, char *skill, FILE *fp)
{
   int x;
   char buf[BUFSIZE];
   char ststr[BUFSIZE];
   char ltr[2];
   char *space;
   int fndstr = 0;
   char c;
   FILE *pfp;
   
   rewind(fp);
   
   sprintf(buf, " ");
   pfp = fopen( TEMPFILE2, "w");
   rewind(pfp);
   
   strcpy(ltr, "");
   
   if (addone && type > 3 && cnt < 0)
   {
      for(;;)
      {
         x = -10;
         c = fgetc( fp );
         fputc(c, pfp);
         sprintf(ltr, "%c", c);
         strcat(ststr, ltr);
         if (c == '\n' || c == '\0')
         {
            if (type <= 3)
            {
               if (strstr(ststr, CSTART))
                  break;
            }
            if (type == 4)
            {
               if (strstr(ststr, BSTART))
                  break;
            }
            if (type == 5)
            {
               if (strstr(ststr, DOSTART))
                  break;
            }
            strcpy(ststr, "");
         }
      }
   }	        
   for (x = 0; x <= cnt; x++)
   {  
      if (addone)
      {
         for (;;)
         {
            c = fgetc( fp );      
            fputc(c, pfp);
            
            if (!fndstr)
            {
               sprintf(ltr, "%c", c);
               strcat(ststr, ltr);
            }
            if (c == '\n' || c == '\0')
            {
               if (!fndstr)
               {
                  if (type <= 3)
                  {
                     if (strstr(ststr, CSTART))
                        fndstr = 1;
                  }
                  if (type == 4)
                  {
                     if (strstr(ststr, BSTART))
                        fndstr = 1;
                  }
                  if (type == 5)
                  {
                     if (strstr(ststr, DOSTART))
                        fndstr = 1;
                  }
                  strcpy(ststr, "");
                  continue;
               }
               strcpy(ststr, "");
               break;
            }
         }
      }
      else
      {
          for (;;)
          {
             c = fgetc( fp );      
             fputc(c, pfp);	
             if (c == '\n' || c == '\0')
                break;
          }
      }
   }       
            
   if (type == 1)
   {
      space = get_space(skill); //returns extract space to align everything
      if (commnt)
      {
         fprintf(pfp, "   // if ( !str_cmp( name, \"%s\"%s ))    return %s%c\n", skill, space, skill, 59);
         commnt = 0;
      }
      else
      {
         fprintf(pfp, "      if ( !str_cmp( name, \"%s\"%s ))    return %s%c\n", skill, space, skill, 59);
      }
   }
   if (type == 2)
   {
      space = get_space(skill); //returns extract space to align everything
      if (commnt)
      {
         fprintf(pfp, "   // if ( !str_cmp( name, \"%s\"%s ))    return %s%c\n", skill, space, skill, 59);
         commnt = 0;
      }
      else
      {
         fprintf(pfp, "      if ( !str_cmp( name, \"%s\"%s ))    return %s%c\n", skill, space, skill, 59);
      }
   }
   if (type == 3)
   {
      space = get_space(skill); //returns extract space to align everything 	
      fprintf(pfp, "   case '%c':\n", skill[3]);
      if (commnt)
      {
         fprintf(pfp, "   // if ( !str_cmp( name, \"%s\"%s ))    return %s%c\n", skill, space, skill, 59);
         commnt = 0;
      }
      else
      {
         fprintf(pfp, "      if ( !str_cmp( name, \"%s\"%s ))    return %s%c\n", skill, space, skill, 59);
      }
      fprintf(pfp, "      break%c\n", 59);
   }
   if (type == 4)
   {
      if (commnt)
      {
         fprintf(pfp, "   // if ( skill == %-30s )    return \"%s\"%c\n", skill, skill, 59);
         commnt = 0;
      }
      else
      {
         fprintf(pfp, "      if ( skill == %-30s )    return \"%s\"%c\n", skill, skill, 59);
      }
   }
   if (type == 5)
   {
      if (commnt)
      {
         fprintf(pfp, "//DECLARE_DO_FUN( %-30s )%c\n", skill, 59);
         commnt = 0;
      }
      else
      {
         fprintf(pfp, "DECLARE_DO_FUN( %-30s )%c\n", skill, 59);
      }
   }
   while (!feof(fp))
   {
      c = fgetc( fp );   
      if (!feof(fp))  
         fputc(c, pfp);   
   }
   fclose(fp);
   fclose(pfp);
   rename( TEMPFILE2, TEMPFILE);
   remove( TEMPFILE2);
   return;
}

//Scans to make sure the do_ is not already in, and looks for indicators
void scan_for_entry(char *skill)
{
   FILE *hfp;
   FILE *tfp;
   char *sbuf;
   int fndind[6];
   char buf[BUFSIZE];
   int match = 0; //exit if match is found at the end
   int cnt = 0;
   int type = 0;
   
   hfp = fopen(HEADER_FILE, "r");
   tfp = fopen(TABLE_FILE, "r");
   
   while( !feof(tfp))
   {
      fgets(buf, BUFSIZE, tfp);   
      if (strstr(buf, CSTART))
      {
         type = 1;
         fndind[0]++;
      }
      if (strstr(buf, BSTART))
      {
         type = 2;
         fndind[2]++;
      }
      if (strstr(buf, CEND))
      {
         type = 0;
         fndind[1]++;
      }
      if (strstr(buf, BEND))
      {
         type = 0;
         fndind[3]++;
      }
      cnt++;
      if (type == 1)
         sbuf = get_skill_name(buf, 1);
      if (type == 2)
         sbuf = get_skill_name(buf, 3);   
      if (type && sbuf)
      {
         if (strcmp(skill, sbuf) == 0)
         {
            printf("%s was found in file %s of line %d.\n", skill, TABLE_FILE, cnt);
            match++;
            continue;
         }
      }
   }
   cnt = 0;
   type = 0;
   while ( !feof(hfp))
   {
      fgets(buf, BUFSIZE, hfp);   
      if (strstr(buf, DOSTART))
      {
         type = 1;
         fndind[4]++;
      }
      if (strstr(buf, DOEND))
      {
         fndind[5]++;
         type = 0;
      }
      cnt++;
      
      if (type == 1)
         sbuf = get_dofun_name(buf, 1);
      if (type && sbuf)
      {
         if (strcmp(skill, sbuf) == 0)
         {
            printf("%s was found in file %s of line %d.\n", skill, TABLE_FILE, cnt);
            match++;
            continue;
         }
      }
   }
   fclose(hfp);
   fclose(tfp);
   if (match)
   {
      printf("%d matches to the skill %s was found, not adding.\n", match, skill);
      exit(0);
   }
   if (!fndind[0] || !fndind[1] || !fndind[2] || !fndind[3] || !fndind[4] || !fndind[5])
   {
       if (!fndind[0])
          printf("Did not find the marker %s\n", CSTART);
       if (!fndind[1])
          printf("Did not find the marker %s\n", CEND);
       if (!fndind[2])
          printf("Did not find the marker %s\n", BSTART);
       if (!fndind[3])
          printf("Did not find the marker %s\n", BEND);
       if (!fndind[4])
          printf("Did not find the marker %s\n", DOSTART);
       if (!fndind[5])
          printf("Did not find the marker %s\n", DOEND);            
       exit(0);
   }
   return;
}

void scan_cases(void)
{
   FILE *fp;
   int c;
   char buf[BUFSIZE];
   fp = fopen(TABLE_FILE, "r");  
    
   while ( !feof(fp))
   {       
      fgets(buf, BUFSIZE, fp);
      c = get_table_string(buf); 
      if (c)
      {
         if (charused[c - 97] == 0)
            charused[c - 97] = 1;
      }
   }
   fclose(fp);
   return;
}

//Take the skill and write it to the new file
void write_sort_file(char *skill, FILE *fp, int ftype)
{   
   char buf[BUFSIZE];
   int newcase = 0;
   int type;
   int cnt = -1;
   int fndstr = 0;
   char *skillbuf2;
   int fndcase = 0;
   char sch;
   
   rewind(fp); //start at the top :-)
   //First check to see if the case statement exists, charused will remember this 
   if (charused[skill[3] - 97] == 0 && ftype == 0) // 4th character because of do_
   {
      newcase = 1;
      charused[skill[3] - 97] = 1;
   }
   while ( !feof(fp))
   {    
      fgets(buf, BUFSIZE, fp);
      if (addone && !fndstr)
      {
         if (ftype == 0)
         {
            if (!strstr(buf, CSTART))
               continue;
            else
               fndstr = 1;
         }
         if (ftype == 1)
         {
            if (!strstr(buf, BSTART))
               continue;
            else
               fndstr = 1;
         }
         if (ftype == 2)
         {
            if (!strstr(buf, DOSTART))
               continue;
            else
               fndstr = 1;
         }
         continue;
      }               
      cnt++;
      if (newcase == 0 || ftype == 1 || ftype == 2)
      {
         if (ftype == 0)
            skillbuf2 = get_skill_name(buf, 1);
         if (ftype == 1)
            skillbuf2 = get_skill_name(buf, 3);
         if (ftype == 2)
            skillbuf2 = get_dofun_name(buf, 1);
         type = 1;
      }
      else
      {
         type = 0;
         sch = get_table_string(buf);
         skillbuf2 = &sch;
      }   
      if (ftype == 1)
      {
         if (strstr(buf, BEND))
         {
            write_file(--cnt, 4, skill, fp);
            return;     
         }
      }
      if (ftype == 2)
      {
         if (strstr(buf, DOEND))
         {
            write_file(--cnt, 5, skill, fp);
            return;
         }
      }
      if (fndcase == 0 && newcase == 0 && ftype == 0) //find the case statement to see if it exists
      {
         if (no_eol(skill))
         {
            sch = skill[3];
            if (get_table_string(buf) && get_table_string(buf) == sch)
            {
               fndcase = 1;
               continue;
            }
         }
      }
      if (skillbuf2 == NULL && fndcase == 0)
         continue;   
      if (skillbuf2 == NULL && fndcase == 1)
      {
         write_file(--cnt, 1, skill, fp);
         return;
      }
      if (((newcase == 0 && type) || ftype) && strcmp(skill, skillbuf2) < 0)
      {
         if (ftype == 0)
            write_file(--cnt, 2, skill, fp);
         if (ftype == 1)
            write_file(--cnt, 4, skill, fp);
         if (ftype == 2)
            write_file(--cnt, 5, skill, fp);
         return;
      }
      if (newcase == 1 && !type && strcmp(skill+3, skillbuf2) < 0)
      {
         write_file(--cnt, 3, skill, fp);
         return;
      }
   }
   //nothing found, add something
   if (ftype == 1 || ftype == 2)
   {
      if (ftype == 1)
         write_file(--cnt, 4, skill, fp);
      if (ftype == 2)
         write_file(--cnt, 5, skill, fp);
      return;
   }
   if (newcase == 1)
   {
      write_file(--cnt, 3, skill, fp);
      return;	
   }
}
void remove_spacing(char *filename)
{
   FILE *wfp;
   FILE *rfp;
   char buf[BUFSIZE];
   
   sprintf(buf, " ");
   wfp = fopen( TEMPFILE2, "w");
   rfp = fopen( filename, "r");
   while (!feof(rfp))
   {
      fgets(buf, BUFSIZE, rfp);
      if (!line_empty(buf))
         fprintf(wfp, buf);
      sprintf(buf, " ");
   }
   fclose(wfp);
   fclose(rfp);
   rename( TEMPFILE2, TEMPFILE);
   remove( TEMPFILE2);
   return;
}
//All the sorting, now the actual writing
void update_file(char *reading, char *start, char *end, int backup)
{
   char buf[BUFSIZE*5];
   char name[BUFSIZE/2];
   char c;
   char chr[2];
   FILE *fin;
   FILE *new;
   FILE *org;
   
   sprintf(buf, " ");
   fin = fopen( TEMPFILE2, "w");
   new = fopen( TEMPFILE, "r");
   org = fopen( reading, "r");
   for (;;)
   {
      c = fgetc( org );
      fputc(c, fin);
      
      if (feof(org))
      {
         fprintf(stderr, "Problem writing to file %s, did not find %s.\n", reading, start);
         exit(0);
      }  
      sprintf(chr, "%c", c);
      strcat(buf, chr);
      if (strstr(buf, start))
      {
         fgets(buf, BUFSIZE, org);
         fprintf(fin, buf);
         break;
      }
      if (c == '\0' || c == '\n')
         strcpy(buf, "");   
   }
   sprintf(buf, " ");
   while (!feof(new))
   {
      fgets(buf, BUFSIZE, new);
      if (!line_empty(buf))
         fprintf(fin, buf);
      sprintf(buf, " ");  
   }
   for (;;)
   {
      if (feof(org))
      {
         fprintf(stderr, "Problem writing to file %s, did not find %s.\n", reading, end);
         exit(0);
      }
      fgets(buf, BUFSIZE, org);
      if (strstr(buf, end))
      {
         fprintf(fin, buf);
         break;
      }
   }
   while (!feof(org))
   {
      c = fgetc( org );
      if (!feof(org))
         fputc(c, fin);
   }   
   fclose(fin);
   fclose(new);
   fclose(org);
   if (DOBACKUP && backup == 1)
   {
      sprintf(name, "%s%s", reading, BSUFFIX);
      remove(name); //make sure to remove the backup if it already exists
      rename( reading, name);
   }
   remove(reading);
   rename( TEMPFILE2, reading);
   remove( TEMPFILE2);
   remove( TEMPFILE );
   return;
}
int load_dofun_tables()
{
   FILE *fp;
   FILE *dfp;
   char buf[BUFSIZE];
   char *skill;
   
   if ( (fp = fopen(HEADER_FILE, "r")) == NULL)
   {
      fprintf(stderr, "Error opening the mud.h file.\n");
      return 0;
   }   	      
   
   while ( !feof(fp))
   {
      fgets(buf, BUFSIZE, fp);
      strcat(buf, "/0");
      if (strstr(buf, DOSTART))
      {
         dfp = fopen(TEMPFILE, "w+");
         //Start sorting/organizing
         while (!feof(fp))
         {
            fgets(buf, BUFSIZE, fp);
            if (line_empty(buf))
               continue;
            if (strstr(buf, DOEND))
            {
               fclose(dfp);
               fclose(fp);
               remove_spacing(TEMPFILE);
               update_file(HEADER_FILE, DOSTART, DOEND, 1);
               printf("Done sorting the do_fun table.\n");
               return 1;
            }
            skill = get_dofun_name(buf, 0); //Will get the skill name from the buf
            if (!skill)
               continue;
            write_sort_file(skill, dfp, 2); //Will write the skill name into the new file
            dfp = fopen(TEMPFILE, "a+");
         }
         fprintf(stderr, "The end line of %s was not found.\n", DOEND);
         remove(TEMPFILE);
         fclose(dfp);
         fclose(fp);
         return 0;
      }
   }
   fprintf(stderr, "The start line of %s was not found.\n", DOSTART);
   remove(TEMPFILE);
   fclose(fp);
   return 0;
}
int load_bottom_tables()
{
   FILE *fp;
   FILE *dfp;
   char buf[BUFSIZE];
   char *skill;
   
   if ( (fp = fopen(TABLE_FILE, "r")) == NULL)
   {
      fprintf(stderr, "Error opening the tables.c file.\n");
      return 0;
   }   	      
   while ( !feof(fp))
   {
      fgets(buf, BUFSIZE, fp);
      strcat(buf, "/0");
      if (strstr(buf, BSTART))
      {
         dfp = fopen(TEMPFILE, "w+");
         //Start sorting/organizing
         while (!feof(fp))
         {
            fgets(buf, BUFSIZE, fp);
            if (line_empty(buf))
               continue;
            if (strstr(buf, BEND))
            {
               fclose(dfp);
               fclose(fp);
               remove_spacing(TEMPFILE);
               update_file(TABLE_FILE, BSTART, BEND, 0);
               printf("Done sorting the bottom entries.\n");
               return 1;
            }
            skill = get_skill_name(buf, 2); //Will get the skill name from the buf
            if (!skill)
               continue;
            write_sort_file(skill, dfp, 1); //Will write the skill name into the new file
            dfp = fopen(TEMPFILE, "a+");
         }
         fprintf(stderr, "The end line of %s was not found.\n", BEND);
         remove(TEMPFILE);
         fclose(dfp);
         fclose(fp);
         return 0;
      }
   }
   fprintf(stderr, "The start line of %s was not found.\n", BSTART);
   remove(TEMPFILE);
   fclose(fp);
   return 0;
}
int load_case_statements()
{
   FILE *fp;
   FILE *dfp;
   char buf[BUFSIZE];
   char *skill;
   
   if ( (fp = fopen(TABLE_FILE, "r")) == NULL)
   {
      fprintf(stderr, "Error opening the tables.c file.\n");
      return 0;
   }
   
   while ( !feof(fp))
   {
      fgets(buf, BUFSIZE, fp);
      strcat(buf, "/0");
      if (strstr(buf, CSTART))
      {
         dfp = fopen(TEMPFILE, "w+");
         //Start sorting/organizing
         while (!feof(fp))
         {
            fgets(buf, BUFSIZE, fp);
            if (line_empty(buf))
               continue;
            if (strstr(buf, CEND))
            {
               fclose(dfp);
               fclose(fp);
               update_file(TABLE_FILE, CSTART, CEND, 1);
               printf("Done sorting case entries.\n");
               return 1;
            }
            skill = get_skill_name(buf, 0); //Will get the skill name from the buf
            if (!skill)
               continue;
            write_sort_file(skill, dfp, 0); //Will write the skill name into the new file
            dfp = fopen(TEMPFILE, "a+");
         }
         fprintf(stderr, "The end line of %s was not found.\n", CEND);
         remove(TEMPFILE);
         fclose(dfp);
         fclose(fp);
         return 0;
      }
   }
   fprintf(stderr, "The start line of %s was not found.\n", CSTART);
   remove(TEMPFILE);
   fclose(fp);
   return 0;
}
void copy_file(FILE *writefile, FILE *readfile)
{
   char c;
   
   while (!feof(readfile))
   {
      c = fgetc( readfile );
      if (!feof(readfile))
         fputc(c, writefile);
   }
   rewind(writefile);
   rewind(readfile);
}   
	
//Works from command prompt, no internal looping trash
int main(int argc, char *argv[])
{
   char *skill;
   char buf[BUFSIZE];
   char name[BUFSIZE/2];
   int fnd = 0;
   FILE *fp;
   FILE *dfp;
   
   commnt = 0; //If there is a comment at the line, add it back in.
   addone = 0; //Gets toggled when a do_ is getting added in.  
   if (argc == 1)
   {
      printf("Syntax: insertskill sort\n");
      printf("Syntax: insertskill <name>\n");
      printf("\nMake sure to read the HOWTO first before using\n");
      return 0;
   }
   skill = argv[1];
   if (!strcmp(argv[1], "sort"))
   {	
      if (load_case_statements() == 0)
         return 0;
      if (load_bottom_tables() == 0)
         return 0;
      if (load_dofun_tables() == 0)
         return 0;
      return 1;
   }
   //Make sure it is an actual do_ skill
   if (skill[0] == 'd' && skill[1] == 'o' && skill[2] == '_')
   {
      scan_for_entry(skill); //Scans to make sure the skill isn't already added
      scan_cases(); //Just in case the skill is a new case
      addone = 1;
      //Start writing the actual junk now :-)
      if ( (fp = fopen(TABLE_FILE, "r")) == NULL)
      {
         fprintf(stderr, "Error opening the tables.c file.\n");
         return 0;
      }
      dfp = fopen(TEMPFILE, "w+");
      copy_file(dfp, fp);
      while (!feof(dfp))
      {
         fgets(buf, BUFSIZE, dfp);
         if (strstr(buf, CSTART))
         {
            write_sort_file(skill, dfp, 0); //Will write the skill name into the new file
            fclose(fp);
            //Backup if needed
            if (DOBACKUP)
            {
               sprintf(name, "%s%s", TABLE_FILE, BSUFFIX);
               remove(name); //make sure to remove the backup if it already exists
               rename( TABLE_FILE, name);
            }
            remove(TABLE_FILE);
            rename( TEMPFILE, TABLE_FILE);
            remove( TEMPFILE);
            printf("Done adding case entries.\n");
            fnd = 1;
            break;
         }
      }
      if (fnd == 0)
      {
         fprintf(stderr, "The start line of %s was not found.\n", CSTART);
         remove(TEMPFILE);
         fclose(dfp);
         fclose(fp);
         return 0;
      }
      fnd = 0; //Bottom part now
      if ( (fp = fopen(TABLE_FILE, "r")) == NULL)
      {
         fprintf(stderr, "Error opening the tables.c file.\n");
         return 0;
      }
      dfp = fopen(TEMPFILE, "w+");
      copy_file(dfp, fp);
      while (!feof(dfp))
      {
         fgets(buf, BUFSIZE, dfp);
         if (strstr(buf, BSTART))
         {
            write_sort_file(skill, dfp, 1); //Will write the skill name into the new file
            fclose(fp);
            //Backup if needed
            remove(TABLE_FILE);
            rename( TEMPFILE, TABLE_FILE);
            remove( TEMPFILE);
            printf("Done adding bottom entries.\n");
            fnd = 1;
            break;
         }
      }
      if (fnd == 0)
      {
         fprintf(stderr, "The start line of %s was not found.\n", BSTART);
         remove(TEMPFILE);
         fclose(dfp);
         fclose(fp);
         return 0;
      }
      fnd = 0; //start mud.h
      if ( (fp = fopen(HEADER_FILE, "r")) == NULL)
      {
         fprintf(stderr, "Error opening the mud.h file.\n");
         return 0;
      }
      dfp = fopen(TEMPFILE, "w+");
      copy_file(dfp, fp);
      while (!feof(dfp))
      {
         fgets(buf, BUFSIZE, dfp);
         if (strstr(buf, DOSTART))
         {
            write_sort_file(skill, dfp, 2); //Will write the skill name into the new file
            fclose(fp);
            //Backup if needed
            if (DOBACKUP)
            {
               sprintf(name, "%s%s", HEADER_FILE, BSUFFIX);
               remove(name); //make sure to remove the backup if it already exists
               rename( HEADER_FILE, name);
            }
            remove(HEADER_FILE);
            rename( TEMPFILE, HEADER_FILE);
            remove( TEMPFILE);
            printf("Done adding dofun entries.\n");
            fnd = 1;
            break;
         }
      }
      if (fnd == 0)
      {
         fprintf(stderr, "The start line of %s was not found.\n", DOSTART);
         remove(TEMPFILE);
         fclose(dfp);
         fclose(fp);
         return 0;
      }
      return 1;
   }
   else
   {
      printf("Any new addition must start with 'do_' to add it in.\n");
      return 0;
   }
   printf("Syntax: insertskill sort\n");
   printf("Syntax: insertskill <name>\n");
   printf("\nMake sure to read the HOWTO first before using\n");
   return 0;
}
