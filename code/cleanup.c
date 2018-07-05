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
 *        Memory Cleanup  -Druid                                            *
 ****************************************************************************
 *
 * PirateMUD is a derivative work of SMAUG and therefore the SMAUG License, 
 * Merc License, and Diku License must all be followed, no exceptions. The
 * PirateMUD license simply states, "AS IS".
 * 
 * This by no means cleans all the memory, there is stuff that my mud doesn't
 * use, therefore the memory is not allocated, and hence it may not be cleaned
 * up in here.
 *
 * First, Add this file to the Makefile
 *
 * At the top of comm.c
 * Add:
 * void cleanup_memory( void );
 * Search for:
 * log_string( "Normal termination of game." );
 *
 * After that add:
 * log_string( "Cleaning up memory." );
 * cleanup_memory( );
 *
 * $Header: /home/ddruid/smaug/src/RCS/cleanup.c,v 0.4 2002/09/29 15:06:01 ddruid Exp ddruid $
 * Robert Haas  - $Date: 2002/09/29 15:06:01 $
 *
 * $Id: cleanup.c,v 0.4 2002/09/29 15:06:01 ddruid Exp ddruid $
 *
 * $State: Exp $
 *
 * REVISION HISTORY:
 * $Log: cleanup.c,v $
 * Revision 0.4  2002/09/29 15:06:01  ddruid
 * Had to modify the order a bit to fix a crash with Dmalloc.
 *
 * Revision 0.3  2002/09/03 19:43:22  ddruid
 * Random global variables
 *
 * Revision 0.2  2002/08/28 17:45:59  ddruid
 * Most things cleaned, seem to be catching global variables now.
 *
 * Revision 0.1  2002/08/27 23:04:26  ddruid
 * Check in
 *
 * Revision 0.0  2002/08/26 18:09:12  ddruid
 * Initial revision
 *
 * NOTES: 
 *  STRALLOC free with MY_STRFREE
 *  fread_string free with MY_STRFREE
 *  fread_string_nohash free with MY_DISPOSE 
 *  str_dup free with MY_DISPOSE
 */
#include <stdio.h>
#include "mud.h"

/* RCS Version */
static char clversion_rcsid[] __attribute__ ((unused)) = "$Id: cleanup.c,v 0.4 2002/09/29 15:06:01 ddruid Exp ddruid $";

extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MAP_INDEX_DATA *first_map;
extern struct act_prog_data *room_act_list;
extern struct act_prog_data *obj_act_list;
extern char *ranged_target_name;

void unlink_social( SOCIALTYPE *social );
void free_desc( DESCRIPTOR_DATA *d );

#define MY_DISPOSE( ptr ) if( (ptr) ) DISPOSE( (ptr) );
#define MY_STRFREE( ptr ) if( (ptr) ) STRFREE( (ptr) );

/*
 * Print current version
 */
void do_clversion( CHAR_DATA *ch, char *argument )
{
  ch_printf_color( ch, "&RCurrent Cleanup_Mem version: &Y%s\n\r", clversion_rcsid );
  return;
}   /* do_clversion */

/*
 * Clean all memory on exit to help find leaks
 */
void cleanup_memory( void )
{

  int hash;
  CMDTYPE *command, *cmd_next;
  CLAN_DATA *clan, *clan_next;
  SOCIALTYPE *social, *social_next;
  struct class_type *class;
  struct race_type *race;
  SKILLTYPE *skill;
  SMAUG_AFF *aff, *aff_next;
  WATCH_DATA *pw, *pw_next;
  HELP_DATA *pHelp, *pHelp_next;
  LANG_DATA *lang, *lang_next;
  LCNV_DATA *lcnv, *lcnv_next;
  CHAR_DATA *character, *char_next;
  OBJ_DATA *object, *object_next;
  MOB_INDEX_DATA *mob, *mob_indx_next;
  ROOM_INDEX_DATA *room, *room_indx_next;
  OBJ_INDEX_DATA *obj, *obj_indx_next;
  AREA_DATA *area, *area_next;
  RESET_DATA *res, *res_next;
  MAP_INDEX_DATA *map, *map_next;
  DESCRIPTOR_DATA *desc, *desc_next;
  BOARD_DATA *board, *board_next;
  NOTE_DATA *note, *note_next;
  AFFECT_DATA *paf, *paf_next;
  EXTRA_DESCR_DATA *ed, *ed_next;
  MPROG_DATA *mp, *mp_next;
  struct act_prog_data *apd, *apd_next;

  /* Close open files */
  fprintf( stdout, "Closing reserve files.\n" );
  if( fpReserve )
    fclose( fpReserve );
  if( fpLOG )
    fclose( fpLOG );
    
  
  /* Whack supermob */
  fprintf( stdout, "Whacking supermob.\n" );
  if( supermob )
  {
    char_from_room( supermob );
    UNLINK( supermob, first_char, last_char, next, prev );
    free_char( supermob );
  }

  /* Descriptors */
  fprintf( stdout, "Descriptors.\n" );
  for( desc = first_descriptor; desc; desc = desc_next )
  {
    desc_next = desc->next;
    UNLINK( desc, first_descriptor, last_descriptor, next, prev );
    free_desc( desc );
  }

  /* Free Characters */
  fprintf( stdout, "Characters.\n" );
  for( character = first_char; character; character = char_next )
  {
    if( !character )
      break;
    char_next = character->next;
    UNLINK( character, first_char, last_char, next, prev );
    char_from_room( character );
    character->desc = NULL;
    character->first_carrying = NULL;
    character->last_carrying = NULL;
    free_char( character );
  }
  clean_char_queue(  );
  first_char = NULL;
  
  /* Mob Hash */
  fprintf( stdout, "Mob hash table.\n" );
  for( hash = 0; hash < MAX_KEY_HASH; hash++ )
  {
    for( mob = mob_index_hash[hash]; mob; mob = mob_indx_next )
    {
      mob_indx_next = mob->next;
      delete_mob( mob );
    }
  }
  
  /* Free Objects */
  fprintf( stdout, "Objects.\n" );
  clean_obj_queue(  );
  for( object = first_object; object; object = object_next )
  {
    object_next = object->next;
    UNLINK( object, first_object, last_object, next, prev );
    if( object->item_type == ITEM_PORTAL )
      remove_portal( object );
    
    
    /*if( object->carried_by )
      object->carried_by = NULL;
    if( object->in_room )
      obj_from_room( object );
    else if( object->in_obj )
      obj_from_obj( object );*/
    
    for( paf = object->first_affect; paf; paf = paf_next )
    {
      paf_next = paf->next;
      MY_DISPOSE( paf );
    }

    for( ed = object->first_extradesc; ed; ed = ed_next )
    {
      ed_next = ed->next;
      MY_STRFREE( ed->description );
      MY_STRFREE( ed->keyword );
      MY_DISPOSE( ed );
    }

    if( object->mpact )
    {
      MY_DISPOSE( object->mpact->buf );
      MY_DISPOSE( object->mpact );
    }
    MY_STRFREE( object->name );
    MY_STRFREE( object->description );
    MY_STRFREE( object->short_descr );
    MY_STRFREE( object->action_desc );
    MY_DISPOSE( object );
  }

  /* Commands */
  fprintf( stdout, "Commands.\n" );
  for( hash = 0; hash < 126; hash++ )
    for( command = command_hash[hash]; command; command = cmd_next )
    {
      cmd_next = command->next;
      command->next = NULL;
      command->do_fun = NULL;
      free_command( command );
    }

  /* Clans */
  fprintf( stdout, "Clans.\n" );
  for( clan = first_clan; clan; clan = clan_next )
  {
    clan_next = clan->next;

    MY_DISPOSE( clan->filename );
    MY_STRFREE( clan->name );
    MY_STRFREE( clan->motto );
    MY_STRFREE( clan->description );
    MY_STRFREE( clan->deity );
    MY_STRFREE( clan->leader );
    MY_STRFREE( clan->number1 );
    MY_STRFREE( clan->number2 );
    MY_STRFREE( clan->badge );
    MY_DISPOSE( clan );
  }

  /* Classes */
  fprintf( stdout, "Classes.\n" );
  for( hash = 0; hash < 0; hash++ )
  {
    class = class_table[hash];

    STRFREE( class->who_name );
    DISPOSE( class );
  }

  /* socials */
  fprintf( stdout, "Socials.\n" );
  for( hash = 0; hash < 27; hash++ )
    for( social = social_index[hash]; social; social = social_next )
    {
      social_next = social->next;
      free_social( social );
    }

  /* Skills */
  fprintf( stdout, "Skills.\n" );
  for( hash = 0; hash < top_sn; hash++ )
  {
    skill = skill_table[hash];

    if( skill->affects )
    {
      for( aff = skill->affects; aff; aff = aff_next )
      {
        aff_next = aff->next;

        MY_DISPOSE( aff->duration );
        MY_DISPOSE( aff->modifier );
        MY_DISPOSE( aff );
      }
    }
    MY_DISPOSE( skill->name );
    MY_DISPOSE( skill->noun_damage );
    MY_DISPOSE( skill->msg_off );
    MY_DISPOSE( skill->hit_char );
    MY_DISPOSE( skill->hit_vict );
    MY_DISPOSE( skill->hit_room );
    MY_DISPOSE( skill->hit_dest );
    MY_DISPOSE( skill->miss_char );
    MY_DISPOSE( skill->miss_vict );
    MY_DISPOSE( skill->miss_room );
    MY_DISPOSE( skill->die_char );
    MY_DISPOSE( skill->die_vict );
    MY_DISPOSE( skill->die_room );
    MY_DISPOSE( skill->imm_char );
    MY_DISPOSE( skill->imm_vict );
    MY_DISPOSE( skill->imm_room );
    MY_DISPOSE( skill->dice );
    MY_DISPOSE( skill->components );
    MY_DISPOSE( skill->teachers );
    skill->spell_fun = NULL;
    skill->skill_fun = NULL;
    MY_DISPOSE( skill );

  }

  /* Watches */
  fprintf( stdout, "Watches.\n" );
  for( pw = first_watch; pw; pw = pw_next )
  {
    pw_next = pw->next;
    MY_DISPOSE( pw->imm_name );
    MY_DISPOSE( pw->player_site );
    MY_DISPOSE( pw->target_name );
    UNLINK( pw, first_watch, last_watch, next, prev );
    MY_DISPOSE( pw );
  }

  /* Helps */
  fprintf( stdout, "Helps.\n" );
  for( pHelp = first_help; pHelp; pHelp = pHelp_next )
  {
    pHelp_next = pHelp->next;
    UNLINK( pHelp, first_help, last_help, next, prev );
    MY_STRFREE( pHelp->text );
    MY_STRFREE( pHelp->keyword );
    MY_DISPOSE( pHelp );
  }

  /* Races */
  fprintf( stdout, "Races.\n" );
  for( hash = 0; hash < MAX_RACE; hash++ )
  {
    race = race_table[hash];
    MY_DISPOSE( race );
  }

  /* Languages */
  fprintf( stdout, "Languages.\n" );
  for( lang = first_lang; lang; lang = lang_next )
  {
    lang_next = lang->next;

    for( lcnv = lang->first_precnv; lcnv; lcnv = lcnv_next )
    {
      lcnv_next = lcnv->next;
      UNLINK( lcnv, lang->first_precnv, lang->last_precnv, next, prev );
      MY_DISPOSE( lcnv->old );
      MY_DISPOSE( lcnv->new );
      MY_DISPOSE( lcnv );
    }
    for( lcnv = lang->first_cnv; lcnv; lcnv = lcnv_next )
    {
      lcnv_next = lcnv->next;
      UNLINK( lcnv, lang->first_cnv, lang->last_cnv, next, prev );
      MY_DISPOSE( lcnv->old );
      MY_DISPOSE( lcnv->new );
      MY_DISPOSE( lcnv );
    }
    MY_STRFREE( lang->name );
    MY_STRFREE( lang->alphabet );
    MY_DISPOSE( lang );
  }

  /* Boards */
  fprintf( stdout, "Boards.\n" );
  for( board = first_board; board; board = board_next )
  {
    board_next = board->next;

    for( note = board->first_note; note; note = note_next )
    {
      note_next = note->next;
      UNLINK( note, board->first_note, board->last_note, next, prev );
      free_note( note );
    }
    UNLINK( board, first_board, last_board, next, prev );
    MY_DISPOSE( board->extra_readers );
    MY_DISPOSE( board->extra_removers );
    MY_DISPOSE( board->note_file );
    MY_DISPOSE( board->read_group );
    MY_DISPOSE( board );
  }

  /* Rooms */
  fprintf( stdout, "Room hash table.\n" );
  for( hash = 0; hash < MAX_KEY_HASH; hash++ )
    for( room = room_index_hash[hash]; room; room = room_indx_next )
    {
      room_indx_next = room->next;
      room->first_content = NULL;
      room->last_content = NULL;
      delete_room( room );
    }

  /* Obj Hash */
  fprintf( stdout, "Object hash table.\n" );
  for( hash = 0; hash < MAX_KEY_HASH; hash++ )
    for( obj = obj_index_hash[hash]; obj; obj = obj_indx_next )
    {
      obj_indx_next = obj->next;

      for( ed = obj->first_extradesc; ed; ed = ed_next )
      {
        ed_next = ed->next;

        MY_STRFREE( ed->keyword );
        MY_STRFREE( ed->description );
        MY_DISPOSE( ed );
      }

      for( paf = obj->first_affect; paf; paf = paf_next )
      {
        paf_next = paf->next;

        MY_DISPOSE( paf );
      }

      for( mp = obj->mudprogs; mp; mp = mp_next )
      {
        mp_next = mp->next;
        MY_STRFREE( mp->arglist );
        MY_STRFREE( mp->comlist );
        MY_DISPOSE( mp );
      }

      MY_STRFREE( obj->name );
      MY_STRFREE( obj->short_descr );
      MY_STRFREE( obj->description );
      MY_STRFREE( obj->action_desc );
      MY_DISPOSE( obj );
    }

  /* Area data */
  fprintf( stdout, "Area data.\n" );
  for( area = first_area; area; area = area_next )
  {
    area_next = area->next;

    for( res = area->first_reset; res; res = res_next )
    {
      res_next = res->next;
      UNLINK( res, area->first_reset, area->last_reset, next, prev );
      MY_DISPOSE( res );
    }
    MY_DISPOSE( area->name );
    MY_DISPOSE( area->filename );
    MY_STRFREE( area->author );
    MY_DISPOSE( area->resetmsg );
    MY_DISPOSE( area );
  }

  /* Map Indexes */
  fprintf( stdout, "Map indexes.\n" );
  for( map = first_map; map; map = map_next )
  {
    map_next = map->next;
    DISPOSE( map );
  }

  /* Get rid of auction pointer  MUST BE AFTER OBJECTS DESTROYED */
  fprintf( stdout, "Auction.\n" );
  MY_DISPOSE( auction );

  /* System Data */
  fprintf( stdout, "System data.\n" );
  MY_DISPOSE( sysdata.time_of_max );
  MY_DISPOSE( sysdata.mud_name );
  MY_STRFREE( sysdata.guild_overseer );
  MY_STRFREE( sysdata.guild_advisor );

  /* Herbs */
  fprintf( stdout, "Herbs.\n" );
  for( hash = 0; hash < top_herb; hash++ )
  {
    skill = herb_table[hash];

    if( skill->affects )
    {
      for( aff = skill->affects; aff; aff = aff_next )
      {
        aff_next = aff->next;

        MY_DISPOSE( aff->duration );
        MY_DISPOSE( aff->modifier );
        MY_DISPOSE( aff );
      }
    }
    MY_DISPOSE( skill->name );
    MY_DISPOSE( skill->noun_damage );
    MY_DISPOSE( skill->msg_off );
    MY_DISPOSE( skill->hit_char );
    MY_DISPOSE( skill->hit_vict );
    MY_DISPOSE( skill->hit_room );
    MY_DISPOSE( skill->hit_dest );
    MY_DISPOSE( skill->miss_char );
    MY_DISPOSE( skill->miss_vict );
    MY_DISPOSE( skill->miss_room );
    MY_DISPOSE( skill->die_char );
    MY_DISPOSE( skill->die_vict );
    MY_DISPOSE( skill->die_room );
    MY_DISPOSE( skill->imm_char );
    MY_DISPOSE( skill->imm_vict );
    MY_DISPOSE( skill->imm_room );
    MY_DISPOSE( skill->dice );
    MY_DISPOSE( skill->components );
    MY_DISPOSE( skill->teachers );
    skill->spell_fun = NULL;
    skill->skill_fun = NULL;
    MY_DISPOSE( skill );
  }

  /* Prog Act lists */
  fprintf( stdout, "Mprog act list.\n" );
  for( apd = mob_act_list; apd; apd = apd_next )
  {
    apd_next = apd->next;
    MY_DISPOSE( apd );
  }
  
  fprintf( stdout, "Rprog act list.\n" );
  for( apd = room_act_list; apd; apd = apd_next )
  {
    apd_next = apd->next;
    MY_DISPOSE( apd );
  }
  
  fprintf( stdout, "Oprog act list.\n" );
  for( apd = obj_act_list; apd; apd = apd_next )
  {
    apd_next = apd->next;
    MY_DISPOSE( apd );
  }

  /* Some freaking globals */
  fprintf( stdout, "Globals.\n" );
  MY_DISPOSE( ranged_target_name );

}   /* cleanup memory */

/* End $RCSfile: cleanup.c,v $ */
