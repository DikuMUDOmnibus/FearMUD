/*  InsertSkill by Joshua Halls (AKA Xerves)
    Copyright (C) 2000  Joshua Halls

    GPL has been revoked by author    
    
    Please Read the HOWTO and REQUESTS files before doing anything else.
    central.h -   Contains the variables used by the insertskill.c file.
                  Each variable's use is covered in the HOWTO file  */
                    
                    

#define BUFSIZE      120  //Don't change this unless you know what you are doing
//Name of the files used, see the HOWTO for more info
#define TABLE_FILE   "tables.c"
#define HEADER_FILE  "mud.h"
#define TEMPFILE     "temp1.c"
#define TEMPFILE2    "temp2.c"
//Name of the starting/ending markers, see the HOWTO for more info
#define CSTART       "/*//T1*/"
#define CEND         "/*//T2*/" 
#define BSTART       "/*//T3*/"
#define BEND         "/*//T4*/" 
#define DOSTART      "/*//T5*/" 
#define DOEND        "/*//T6*/" 
//change dobackup to toggle backup usage (1 yes, 0 no).  
#define DOBACKUP     1
//If you need to change the backup suffix, change the below define
#define BSUFFIX      ".bak" 
