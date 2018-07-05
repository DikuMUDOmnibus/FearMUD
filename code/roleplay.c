/****************************************************************************
*   *`-.._  |    |`!  |`.  -_ -__ -_ _- _-_- -__-_ - .'|.;'   |   _.    *   *
*   *      | `-!._  |  `;!  ;. ______________________ ,'| .-' |   _!.i' *   *
* A *..__  |     |`-!._ | `.| |______________________| ."'|  _!.;'      * A *
*   *   |``"..__ |    |`";.| i|_|   ,-' :  : `-.   |_|'| _!-|   |   _|. *   *
* R *   |      |``--..|_ | `;!| |  / :  :  :  . \  | |.'j   |_..!-'|    * R *
*   *   |      |    |   |`-,!_|_| |_ ;   __:  ;  | |_||.!-;'  |    |    *   * 
* C *___|______|____!.,.!,.!,!| | )  .  :)(.  :  | | |,!,.!.,.!..__|___ * C *
*   *      |     |    |  |  | |_| |"    (##)  _  | |_|| |   |   |    |  *   *
* H *      |     |    |..!-;'i| | |  :  ;`'  (_) ( | | |`-..|   |    |  * H *
*   *      |    _!.-j'  | _!,"|_| |  :  :  .     | |_||!._|  `i-!.._ |  *   *
* W *     _!.-'|    | _."|  !;| | )_ :  ,  ;  ;  | | |`.| `-._|    |``- * W *
*   *..-i'     |  _.''|  !-| !|_| | .  .  :  :   | |_|.|`-. | ``._ |    *   *
* A *   |      |.|    |.|  !| | | |" .  :  :  .  | | ||`. |`!   | `".   * A *
*   *   |  _.-'  |  .'  |.' |/|_| |______________| |_|! |`!  `,.|    |- *   *
* Y *  _!"'|     !.'|  .'| .'|[@]                  [@] \|  `. | `._  |  * Y *
*   *-'    |   .'   |.|  |/| /         .----.         \|`.  |`!    |.|  *   *
*   *       _.'|   .' | .' |/         (_^..^_)         \  \ |  `.  | `. *   *
*   *                                   ||||                            *   *
*   *                                                                   *   *
****** The Archway of Lost Souls code and additions by Rythmic **************
*                           Roleplay module                                 *
*****************************************************************************
* Provided by Rythmic for use in Fear 2.x codebase    --Xerves              *
*****************************************************************************/

#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <time.h>
#include "mud.h"

bool    check_social    args( ( CHAR_DATA *ch, char *command, char *argument ) );
char *  scramble        args( ( const char *argument, int modifier ) );             
char *  drunk_speech    args( ( const char *argument, CHAR_DATA *ch ) );


/*
 * New argument3 for supporting do_sing
 */
char *one_argument3( char *argument, char *arg_first )
{
    char last; 
    sh_int length;
    length = 0;

    if ( !argument || argument[0] == '\0' )
    {
    arg_first[0] = '\0';
    return argument;
    }

    last = '+'; /** Symbol used to return lines **/

    if ( *argument == '\'' || *argument == '"' )
    last = *argument++;

    while ( *argument != '\0' || ++length >= 255 )
    {
      if ( *argument == last )
      {
        argument++;
        break;
      }

      *arg_first = (*argument);
      arg_first++;
      argument++;
    }

    *arg_first = '\0';
    return argument;
}


/*
 * GS style sing command
 *
 * You sing crypticly:
 *
 *         "Mama Mama can't you see?" 
 *         "What playing muds has done to me!" 
 */
void do_sing (CHAR_DATA * ch, char *argument)
{
    char buf [MSL];
    char buf1[MSL];
    char arg1[MIL];

    if (argument[0] == '\0')
    {
      send_to_char ("Sing what?\n\r", ch);
      return;
    }

    argument = one_argument3 (argument, arg1);

    if ( (( ch->tone ) == NULL ) || ( strlen( ch->tone ) < 1 ) )
    {
    act (AT_SAY, "\n\rYou sing:", ch, NULL, NULL, TO_CHAR);
    act (AT_SAY, "\n\r$n sings:", ch, NULL, NULL, TO_ROOM);
    }
    else
    {
    sprintf (buf, "%s", ch->tone );
    act (AT_SAY, "\n\rYou sing $t:", ch, buf, NULL, TO_CHAR);
    act (AT_SAY, "\n\r$n sings $t:", ch, buf, NULL, TO_ROOM);
    }

    sprintf (buf1, "\n\r\t \"&w%s&D\" \r", arg1);

    act (AT_SAY, "$t", ch, buf1, NULL, TO_CHAR);
    act (AT_SAY, "$t", ch, buf1, NULL, TO_ROOM);
  
  while (argument[0] != '\0')
    {
      argument = one_argument3 (argument, arg1);
      sprintf (buf1, "\t \"&w%s&D\" \r", arg1);
      act (AT_SAY, "$t", ch, buf1, NULL, TO_ROOM);
      act (AT_SAY, "$t", ch, buf1, NULL, TO_CHAR);
    }
  return;
}

void show_tones ( CHAR_DATA *ch )
{
   send_to_char( "Syntax: tone <#>\n\r\n\r", ch );
   send_to_char( "tone types:\n\r", ch );
   send_to_char( " 1. none           2. abrupt         3. accusing          \n\r", ch );
   send_to_char( " 4. amused         5. angry          6. anxious          \n\r", ch );
   send_to_char( " 7. appreciative   8. approving      9. argumentative    \n\r", ch ); 
   send_to_char( "10. arrogant      11. bashful       12. calm             \n\r", ch );
   send_to_char( "13. careful       14. cautious      15. cheerful         \n\r", ch );
   send_to_char( "16. clear         17. cold          18. condescending    \n\r", ch ); 
   send_to_char( "19. confident     20. confused      21. coy               \n\r", ch );
   send_to_char( "22. cryptic       23. dark          24. deep              \n\r", ch );
   send_to_char( "25. disapproving  26. dismissive    27. dreamy            \n\r", ch );
   send_to_char( "31. exasperated   32. excited       33. firm              \n\r", ch );
   send_to_char( "34. flirtatious   35. frustrated    36. gleeful           \n\r", ch ); 
   send_to_char( "37. grateful      38. grave         39. greedy            \n\r", ch );
   send_to_char( "40. grim          41. grumpy        42. happy         \n\r", ch );
   send_to_char( "43. harsh         44. haughty       45. hesitant          \n\r", ch );
   send_to_char( "46. hopeful       47. innocent      48. joking            \n\r", ch );
   send_to_char( "49. jubilant      50. knowing       51. meek              \n\r", ch );
   send_to_char( "52. melodic       53. menacing      54. merry         \n\r", ch );
   send_to_char( "55. mocking       56. monotonous    57. mournful          \n\r", ch ); 
   send_to_char( "58. mysterious    59. nervous       60. patient           \n\r", ch );
   send_to_char( "61. pedantic      62. playful       63. proud         \n\r", ch );
   send_to_char( "64. quick         65. quiet         66. regretful     \n\r", ch );
   send_to_char( "67. remorseful    68. rude          69. sad               \n\r", ch );
   send_to_char( "70. sarcastic     71. seductive     72. serene            \n\r", ch );
   send_to_char( "73. sheepish      74. skeptical     75. sleepy            \n\r", ch );
   send_to_char( "76. slow          77. sly           78. smug              \n\r", ch );
   send_to_char( "79. soft          80. somber        81. spiteful          \n\r", ch );
   send_to_char( "82. squeaky       83. stern         84. stupid            \n\r", ch );
   send_to_char( "85. sweet         86. tactful       87. teasing           \n\r", ch );
   send_to_char( "88. tender        89. terse         90. thoughtful        \n\r", ch );
   send_to_char( "91. tired         92. understanding 93. uneasy            \n\r", ch );
   send_to_char( "94. warm          95. wary          96. weak              \n\r", ch );
   send_to_char( "97. weary         98. wistful       99. worrid            \n\r", ch );
   send_to_char( "100. wry                                                 \n\r", ch );
}


/*
 *  Express yourself better with some tone!
 */
void do_tone( CHAR_DATA *ch , char *argument )
{
    char arg[MIL];
    int number;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    show_tones( ch );
    return;
    }
    
    if ( is_number(arg) )
    {
    number = atoi(arg);
    }
    else
    {
    show_tones( ch );
    return;
    }

    switch( number )
    {
        default:    ch->tone = STRALLOC("");                    break;
        case    1:  ch->tone = STRALLOC("");                    break;
        case    2:  ch->tone = STRALLOC("abruptly");            break;
        case    3:  ch->tone = STRALLOC("accusingly");          break;
        case    4:  ch->tone = STRALLOC("amusedly");            break;
        case    5:  ch->tone = STRALLOC("angrily");             break;
        case    6:  ch->tone = STRALLOC("anxiously");           break;
        case    7:  ch->tone = STRALLOC("appreciatively");      break;
        case    8:  ch->tone = STRALLOC("approvingly");         break;
        case    9:  ch->tone = STRALLOC("argumentatively");     break;
        case   10:  ch->tone = STRALLOC("arrogantly");          break;
        case   11:  ch->tone = STRALLOC("bashfully");           break;
        case   12:  ch->tone = STRALLOC("calmly");              break;
        case   13:  ch->tone = STRALLOC("carefully");           break;
        case   14:  ch->tone = STRALLOC("cautiously");          break;
        case   15:  ch->tone = STRALLOC("cheerfully");          break;
        case   16:  ch->tone = STRALLOC("clearly");             break;
        case   17:  ch->tone = STRALLOC("coldly");              break;
        case   18:  ch->tone = STRALLOC("condescendingly");     break;
        case   19:  ch->tone = STRALLOC("confidently");         break;
        case   20:  ch->tone = STRALLOC("confusedly");          break;
        case   21:  ch->tone = STRALLOC("coyly");               break;
        case   22:  ch->tone = STRALLOC("crypticly");           break;
        case   23:  ch->tone = STRALLOC("darkly");              break;
        case   24:  ch->tone = STRALLOC("deeply");              break;
        case   25:  ch->tone = STRALLOC("disapprovingly");      break;
        case   26:  ch->tone = STRALLOC("dismissively");        break;
        case   27:  ch->tone = STRALLOC("dreamily");            break;
        case   28:  ch->tone = STRALLOC("dubiously");           break;
        case   29:  ch->tone = STRALLOC("emphaticly");          break;
        case   30:  ch->tone = STRALLOC("encouragingly");       break;
        case   31:  ch->tone = STRALLOC("exasperatedly");       break;
        case   32:  ch->tone = STRALLOC("excitedly");           break;
        case   33:  ch->tone = STRALLOC("firmly");              break;
        case   34:  ch->tone = STRALLOC("flirtatiously");       break;
        case   35:  ch->tone = STRALLOC("frustratedly");        break;
        case   36:  ch->tone = STRALLOC("gleefully");           break;
        case   37:  ch->tone = STRALLOC("gratefully");          break;
        case   38:  ch->tone = STRALLOC("gravely");             break;
        case   39:  ch->tone = STRALLOC("greedily");            break;
        case   40:  ch->tone = STRALLOC("grimly");              break;
        case   41:  ch->tone = STRALLOC("grumpily");            break;
        case   42:  ch->tone = STRALLOC("happily");             break;
        case   43:  ch->tone = STRALLOC("harshly");             break;
        case   44:  ch->tone = STRALLOC("haughtily");           break;
        case   45:  ch->tone = STRALLOC("hesitantly");          break;
        case   46:  ch->tone = STRALLOC("hopefully");           break;
        case   47:  ch->tone = STRALLOC("innocently");          break;
        case   48:  ch->tone = STRALLOC("jokingly");            break;
        case   49:  ch->tone = STRALLOC("jubilantly");          break;
        case   50:  ch->tone = STRALLOC("knowingly");           break;   
        case   51:  ch->tone = STRALLOC("meekly");              break;
        case   52:  ch->tone = STRALLOC("melodicly");           break;
        case   53:  ch->tone = STRALLOC("menacingly");          break;
        case   54:  ch->tone = STRALLOC("merrily");             break;
        case   55:  ch->tone = STRALLOC("mockingly");           break;
        case   56:  ch->tone = STRALLOC("monotonously");        break;
        case   57:  ch->tone = STRALLOC("mournfully");          break;
        case   58:  ch->tone = STRALLOC("mysteriously");        break;
        case   59:  ch->tone = STRALLOC("nervously");           break;
        case   60:  ch->tone = STRALLOC("patiently");           break; 
        case   61:  ch->tone = STRALLOC("pedanticly");          break;
        case   62:  ch->tone = STRALLOC("playfully");           break;
        case   63:  ch->tone = STRALLOC("proudly");             break;
        case   64:  ch->tone = STRALLOC("quickly");             break;
        case   65:  ch->tone = STRALLOC("quietly");             break;
        case   66:  ch->tone = STRALLOC("regretfully");         break;
        case   67:  ch->tone = STRALLOC("remorsefully");        break;
        case   68:  ch->tone = STRALLOC("rudely");              break;
        case   69:  ch->tone = STRALLOC("sadly");               break;
        case   70:  ch->tone = STRALLOC("sarcasticly");         break; 
        case   71:  ch->tone = STRALLOC("seductively");         break;
        case   72:  ch->tone = STRALLOC("serenely");            break;
        case   73:  ch->tone = STRALLOC("sheepishly");          break;
        case   74:  ch->tone = STRALLOC("skeptically");         break;
        case   75:  ch->tone = STRALLOC("sleepily");            break;
        case   76:  ch->tone = STRALLOC("slowly");              break;
        case   77:  ch->tone = STRALLOC("slyly");               break;
        case   78:  ch->tone = STRALLOC("smugly");              break;
        case   79:  ch->tone = STRALLOC("softly");              break;
        case   80:  ch->tone = STRALLOC("somberly");            break;    
        case   81:  ch->tone = STRALLOC("spitefully");          break;
        case   82:  ch->tone = STRALLOC("squeakily");           break;
        case   83:  ch->tone = STRALLOC("sternly");             break;
        case   84:  ch->tone = STRALLOC("stupidly");            break;
        case   85:  ch->tone = STRALLOC("sweetly");             break;
        case   86:  ch->tone = STRALLOC("tactfully");           break;
        case   87:  ch->tone = STRALLOC("teasingly");           break;
        case   88:  ch->tone = STRALLOC("tenderly");            break;
        case   89:  ch->tone = STRALLOC("tersely");             break;
        case   90:  ch->tone = STRALLOC("thoughtfully");        break;
        case   91:  ch->tone = STRALLOC("tiredly");             break;
        case   92:  ch->tone = STRALLOC("understandingly");     break;
        case   93:  ch->tone = STRALLOC("uneasily");            break;
        case   94:  ch->tone = STRALLOC("warmly");              break;
        case   95:  ch->tone = STRALLOC("warily");              break;
        case   96:  ch->tone = STRALLOC("weakly");              break;
        case   97:  ch->tone = STRALLOC("wearily");             break;
        case   98:  ch->tone = STRALLOC("wistfully");           break;
        case   99:  ch->tone = STRALLOC("worridly");            break;
        case  100:  ch->tone = STRALLOC("wryly");               break;
    }

     if ( (( ch->tone ) == NULL ) || ( strlen( ch->tone ) < 1 ) )
     {
     send_to_char( "Tone of voice: not set.\n\r", ch );
     }
     else
     {
     ch_printf(ch, "Tone setting: %s\n\r",ch->tone );
     }

    return;
}

void show_movements ( CHAR_DATA *ch )
{
    send_to_char( "Syntax: movement <#>\n\r\n\r", ch );
    send_to_char( "movement types:\n\r", ch );
    send_to_char( "(1) none    (2) walk    (3) stride    (4) strut\n\r", ch );
    send_to_char( "(5) skip    (6) hop     (7) limp      (8) crawl\n\r", ch );
    send_to_char( "(9) slink  (10) waddle  (11) scamper (12) stroll\n\r", ch );
}

/*
 *  Adds some style to pc movement
 *  General list for both pc and mobs for now...
 */
void do_movement( CHAR_DATA *ch , char *argument )
{
    char arg[MIL];
    int number;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    show_movements( ch );
    return;
    }
    
    if ( is_number(arg) )
    {
    number = atoi(arg);
    }
    else
    {
    show_movements( ch );
    return;
    }
    if (ch->movement)
       STRFREE(ch->movement);
    switch( number )
    {
        default:    ch->movement = STRALLOC("");                break;
        case    1:  ch->movement = STRALLOC("");                break;
        case    2:  ch->movement = STRALLOC("walks");           break;
        case    3:  ch->movement = STRALLOC("strides");         break;
        case    4:  ch->movement = STRALLOC("struts");          break;
        case    5:  ch->movement = STRALLOC("skips");           break;
        case    6:  ch->movement = STRALLOC("hops");            break;
        case    7:  ch->movement = STRALLOC("limps");           break;
        case    8:  ch->movement = STRALLOC("crawls");          break;
        case    9:  ch->movement = STRALLOC("slinks");          break;
        case   10:  ch->movement = STRALLOC("waddles");         break;
        case   11:  ch->movement = STRALLOC("scampers");        break;
        case   12:  ch->movement = STRALLOC("strolls");         break;      
    }

     if (!ch->movement || (ch->movement && ch->movement[0] == '\0'))
     send_to_char( "Movement setting: none\n\r", ch );
     else
     ch_printf(ch, "Movement setting: %s",ch->movement );

    return;
}

/*
 *  Show off your objects to other folks
 */
void do_show( CHAR_DATA *ch, char *argument )
{
    char arg1 [MIL];
    char arg2 [MIL];
    CHAR_DATA *victim;
    OBJ_DATA  *obj;

    argument = one_argument( argument, arg1 );
    argument = one_argument( argument, arg2 );
    if ( !str_cmp( arg2, "to" ) && argument[0] != '\0' )
    argument = one_argument( argument, arg2 );

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
    send_to_char( "show what to whom?\n\r", ch );
    return;
    }

    if ( ( obj = get_obj_list( ch, arg1, ch->first_carrying )) == NULL )
    {
    send_to_char( "You do not have that item.\n\r", ch );
    return;
    }

    if ( ( victim = get_char_room( ch, arg2 ) ) == NULL )
    {
    send_to_char( "They aren't here.\n\r", ch );
    return;
    }

    if ( !can_see_obj( victim, obj ) )
    {
    act( AT_PLAIN, "$N can't see it.", ch, NULL, victim, TO_CHAR );
    return;
    }

    act(AT_ACTION, "$n shows you $p.",   ch, obj, victim, TO_VICT   );
    act(AT_ACTION, "You show $p to $N.", ch, obj, victim, TO_CHAR   );

    oprog_examine_trigger( victim, obj );
    return;
}


/** Insult Support **/
char *const insult1[4] =
    {
   "Thou", "You", "You are a", "You're a"
    };

char *const insult2[27] =
    {
   "reeky", "shallow", "goatish", "deranged", "deviant", "three-eyed",
   "snivelling", "bug-eyed", "loathsome", "dankish", "bootless", "frothy",
   "rank", "gore-bellied", "artless", "lumpish", "puny", "gleeking",
   "warped", "mangled", "vain", "ugly", "muddle-headed", "butter-bellied",
   "scummy", "savage", "uneducated"
    };

char *const insult3[27] =
    {
   "excuse for a", "nauseating", "rump-fed", "flap-mouthed", "onion-eyed", "disease-ridden",
   "fly-bitten", "pox-marked", "toad-spotted", "elf-skinned", "dungeon-eared", "beetle-headed",
   "dismal-lookin'", "reeling-ripe", "cross-eyed", "two-faced", "thick-skulled", "disease-ridden",
   "blitherin'", "slack-gutted", "blathering", "second-hand", "inadequate", "bungling", "infested",
   "putrid", "flea-bitten"
    };

char *const insult4[27] =
    {
   "toad", "lewdster", "toe-picker", "frog-boil", "toad-licker", "maggot",
   "scum-bag", "nobody", "bugbear", "malt-worm", "horn-beast", "flax-wench",
   "harpy", "lout", "hack", "rat", "varlet", "hedge-pig",
   "idiot", "scoundrel", "oaf", "tub-bucket", "buffoon", "fraud", "scalawag",
   "slug", "fungus"
    };

/*
 *  You insult yourself 'You loathsome dismal-lookin' toad-licker!'
 */
void do_insults( CHAR_DATA *ch, char *argument )
{
  char arg [MIL];
  char buf [MSL];
  CHAR_DATA *victim;
  char *insult_one;
  char *insult_two;
  char *insult_three;
  char *insult_four;
  int rand;

    argument = one_argument( argument, arg );

    if ( arg[0] == '\0' )
    {
    send_to_char( "Insult who?\n\r", ch );
    return;
    }

    if ( ( victim = get_char_room( ch, arg ) ) == NULL )
    {
    send_to_char( "They aren't here.\n\r", ch );
    return;
    }

    rand = number_range( 0, 3 );
    insult_one = insult1[rand];
    
    rand = number_range( 0, 26 );
    insult_two = insult2[rand];

    rand = number_range( 0, 26 );
    insult_three = insult3[rand];

    rand = number_range( 0, 26 );
    insult_four = insult4[rand];

    sprintf (buf, "%s %s %s %s!",insult_one, insult_two, insult_three, insult_four );

    if ( ch == victim )
    {
    act(AT_SAY, "You insult yourself '&w$t&D'", ch, buf, victim, TO_CHAR);
    act(AT_SAY, "$n insults $mself '&w$t&D'",   ch, buf, victim, TO_NOTVICT);
    }
    else
    {
    act(AT_SAY, "You insult $N '&w$t&D'", ch, buf, victim, TO_CHAR);
    act(AT_SAY, "$n insults you '&w$t&D'",   ch, buf, victim, TO_VICT);
    act(AT_SAY, "$n insults $N '&w$t&D'",   ch, buf, victim, TO_NOTVICT);
    }

return;
}






