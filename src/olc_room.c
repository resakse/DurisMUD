



/* Copyright (c) 1996 by Gary Dezern */

/* Room (.wld) related OLC stuff... */

#define _OLC_SOURCE_            /* to get that extra Kick from olc.h */

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "comm.h"
#include "olc.h"
#include "prototypes.h"
#include "utils.h"

static void olc_room_xtra_list(char *str, int rnum);
static void olc_save_room(FILE * f1, int real_num);


void olc_room_string_add(struct olc_data *data, char *str)
{
  int      i, j;
  struct extra_descr_data *ex_desc;

  switch (data->mode)
  {
  case OLC_MODE_ROOM0:         /* main "room" menu */
    if (!str)
    {
      olc_end(data);
      return;
    }
    if (strlen(str) != 1)       /* all options are 1 char options */
      *str = '?';
    switch (toupper(*str))
    {
    case 'A':                  /* name */
      data->mode = OLC_MODE_ROOM1;
      SEND_TO_Q("Enter the new room name.\r\n", data->desc);
      break;
    case 'B':                  /* desc */
      str = str_dup(world[data->rnum].description);
      /* check for that silly trailing &n */
      if ((str[strlen(str) - 4] == '&') && (str[strlen(str) - 3] == 'n'))
        str[strlen(str) - 4] = '\0';
      data->mode = OLC_MODE_ROOM2;
      edit_start(data->desc, str, 20, olc_room_callback, (int) data);
      FREE(str);
      break;
    case 'C':                  /* flags */
      data->mode = OLC_MODE_ROOM_FLAGS;
      olc_show_menu(data);
      break;
    case 'D':                  /* sector */
      data->mode = OLC_MODE_ROOM_SECT;
      olc_show_menu(data);
      break;
    case 'E':                  /* keywords */
      data->mode = OLC_MODE_XTRA;
      olc_show_menu(data);
      break;
    case 'F':                  /* exits */
      data->mode = OLC_MODE_EXIT;
      olc_show_menu(data);
      break;
    case 'G':                  /* misc */
      SEND_TO_Q("Not yet implemented!\r\n", data->desc);
      break;
    case 'S':
      SEND_TO_Q("Saving... ", data->desc);
      olc_save_wld(world[data->rnum].zone);
      SEND_TO_Q("&+WDone&N\r\n", data->desc);
      break;
    default:
      SEND_TO_Q("Unknown menu option!\r\n", data->desc);
    }                           /* switch ROOM0 command */
    break;

  case OLC_MODE_ROOM1:
    if (!str)
    {
      SEND_TO_Q("&+MAborted!&N\r\n", data->desc);
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }
    /* check for tildes */
    if (strchr(str, '~'))
    {
      SEND_TO_Q("&+RTildes aren't yet allowed in room names.&N\r\n",
                data->desc);
      return;
    }
    FREE(world[data->rnum].name);
    world[data->rnum].name = str_dup(str);
    data->mode = OLC_MODE_ROOM0;
    SEND_TO_Q("&+MDone&N\r\n", data->desc);
    olc_show_menu(data);
    break;

  case OLC_MODE_ROOM_FLAGS:
    if (!str)
    {
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }
    if (is_number(str))
      j = atoi(str) - 1;
    else
      j = -1;

    /* now we have a number... but before we blindly toggle the flag,
       first make damn sure it was a valid menu option */

    for (i = 0; *room_bits[i] != '\n'; i++)
      if (i == j)
      {
        world[data->rnum].room_flags =
          world[data->rnum].room_flags ^ (1 << i);
        SEND_TO_Q("Done\r\n\r\n", data->desc);
        olc_show_menu(data);
        return;
      }
    SEND_TO_Q("&+R&-LInvalid option&N\r\n", data->desc);
    break;

  case OLC_MODE_ROOM_SECT:
    if (!str)
    {
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }
    if (is_number(str))
      j = atoi(str) - 1;
    else
      j = -1;

    /* now we have a number... but before we blindly set the type,
       first make damn sure it was a valid menu option */

    for (i = 0; *room_bits[i] != '\n'; i++)
      if (i == j)
      {
        world[data->rnum].sector_type = i;
        SEND_TO_Q("Done\r\n\r\n", data->desc);
        olc_show_menu(data);
        return;
      }
    SEND_TO_Q("&+R&-LInvalid option&N\r\n", data->desc);
    break;

  case OLC_MODE_XTRA:
    if (!str)
    {
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }
    /* first check for the "valid" command letters */
    if (strlen(str) == 1)
    {
      if (toupper(*str) == 'A')
      {
        CREATE(ex_desc, extra_descr_data, 1, MEM_TAG_EXDESCD);

        ex_desc->keyword = str_dup("&+CNEW EXTRA DESCRIPTION&N");
        ex_desc->description = NULL;
        ex_desc->next = world[data->rnum].ex_description;
        world[data->rnum].ex_description = ex_desc;
        olc_show_menu(data);
        return;
      }
      else if (toupper(*str) == 'B')
      {
        data->mode = OLC_MODE_XTRA_DEL;
        olc_show_menu(data);
        return;
      }
    }
    if (is_number(str))
    {
      j = atoi(str);
    }
    else
    {
      SEND_TO_Q("&+R&-LInvalid option&N\r\n", data->desc);
      return;
    }
    /* find the j'th extra desc... */
    ex_desc = world[data->rnum].ex_description;
    while (--j && ex_desc)
      ex_desc = ex_desc->next;

    if (!ex_desc)
    {
      SEND_TO_Q("&+RNo such extra description!&N\r\n", data->desc);
      return;
    }
    /* okay... ex_desc is the "one". */
    data->mode = OLC_MODE_XTRA1;
    data->misc = ex_desc;
    olc_show_menu(data);
    break;

  case OLC_MODE_XTRA1:
    if (!str)
    {
      data->mode = OLC_MODE_XTRA;
      olc_show_menu(data);
      return;
    }
    if (strlen(str) == 1)
    {
      if (toupper(*str) == 'A')
      {
        data->mode = OLC_MODE_XTRA2;
        SEND_TO_Q
          ("\r\nEnter a new keyword, multiple entries seperated with spaces:\r\n",
           data->desc);
        break;
      }
      else if (toupper(*str) == 'B')
      {
        data->mode = OLC_MODE_XTRA3;
        ex_desc = (struct extra_descr_data *) data->misc;
        edit_start(data->desc, ex_desc->description, 20,
                   olc_room_callback, (int) data);
        break;
      }
    }
  case OLC_MODE_XTRA2:
    if (!str)
    {
      data->mode = OLC_MODE_XTRA1;
      olc_show_menu(data);
      return;
    }
    /* check for tildes */
    if (strchr(str, '~'))
    {
      SEND_TO_Q("Tildes aren't yet allowed in description names.\r\n",
                data->desc);
      return;
    }
    ex_desc = (struct extra_descr_data *) data->misc;
    FREE(ex_desc->keyword);
    ex_desc->keyword = str_dup(str);
    strToLower(ex_desc->keyword);
    data->mode = OLC_MODE_XTRA1;
    SEND_TO_Q("Done\r\n\r\n", data->desc);
    break;

  case OLC_MODE_XTRA_DEL:
    if (!str)
    {
      data->mode = OLC_MODE_XTRA;
      olc_show_menu(data);
      return;
    }
    if (is_number(str))
    {
      j = atoi(str);
    }
    else
    {
      SEND_TO_Q("&+R&-LInvalid option&N\r\n", data->desc);
      return;
    }
    /* find the j'th extra desc... */
    ex_desc = world[data->rnum].ex_description;

    i = 0;
    if (j == 1)
    {
      world[data->rnum].ex_description = ex_desc->next;
      if (ex_desc->keyword)
        FREE(ex_desc->keyword);
      if (ex_desc->description)
        FREE(ex_desc->description);
      FREE(ex_desc);
      i = 1;
    }
    else
    {
      i = 2;
      while (ex_desc->next)
        if (j == i++)
        {
          if (ex_desc->next->keyword)
            FREE(ex_desc->next->keyword);
          if (ex_desc->next->description)
            FREE(ex_desc->next->description);
          ex_desc->next = ex_desc->next->next;
          FREE(ex_desc->next);
          i = 1;
          break;
        }
    }

    if (i != 1)
    {
      SEND_TO_Q("&+RNo such extra description!&N\r\n", data->desc);
      return;
    }
    SEND_TO_Q("&+GDeleted!&N\r\n", data->desc);
    olc_show_menu(data);
    break;

  case OLC_MODE_EXIT:
    if (!str)
    {
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }
#if 1
    if (is_number(str))
    {
      j = atoi(str);
      if (j == 1)
      {
        /* create new exit field. Be sure it doesnt already exist! */

      }
      else if (j == 2)
      {
        data->mode = OLC_MODE_EXIT_DEL;
        olc_show_menu(data);
        break;
      }
      else
      {
        SEND_TO_Q("&+R&-LInvalid option&N\r\n", data->desc);
        return;
      }
    }
    else if (toupper(*str) == 'N' || toupper(*str) == 'E'
             || toupper(*str) == 'W' || toupper(*str) == 'S'
             || toupper(*str) == 'U' || toupper(*str) == 'D')
    {
      /* entered direction letter */
      /* edit existing direction field */
      data->mode = OLC_MODE_EXIT1;
      olc_show_menu(data);
      break;
    }
    SEND_TO_Q("&+R&-LInvalid option&N\r\n", data->desc);
    return;
    break;
  case OLC_MODE_EXIT_DEL:
    if (!str)
    {
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }

    if (is_number(str))
      j = atoi(str);
    else
    {
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }
    if (world[data->rnum].dir_option[j])
    {
      if (world[data->rnum].dir_option[j]->general_description)
        FREE(world[data->rnum].dir_option[j]->general_description);
      if (world[data->rnum].dir_option[j]->keyword)
        FREE(world[data->rnum].dir_option[j]->keyword);
      if (world[data->rnum].dir_option[j]->exit_info)
        world[data->rnum].dir_option[j]->exit_info = 0;
      if (world[data->rnum].dir_option[j]->key)
        world[data->rnum].dir_option[j]->key = 0;
      if (world[data->rnum].dir_option[j]->to_room)
        world[data->rnum].dir_option[j]->to_room = 0;
      world[data->rnum].dir_option[j] = NULL;
    }
    SEND_TO_Q("&+GDeleted!&N\r\n", data->desc);
    olc_show_menu(data);
    break;
  case OLC_MODE_EXIT1:
    if (!str)
    {
      data->mode = OLC_MODE_EXIT;
      olc_show_menu(data);
      return;
    }
/*
            CREATE(world[data->rnum].dir_option[dir], struct room_direction_data, 1);
            world[data->rnum].dir_option[dir]->exit_info = EX_ISDOOR;
*/
    if (toupper(*str) == 'N')
      j = 0;
    else if (toupper(*str) == 'E')
      j = 1;
    else if (toupper(*str) == 'S')
      j = 2;
    else if (toupper(*str) == 'W')
      j = 3;
    else if (toupper(*str) == 'U')
      j = 4;
    else if (toupper(*str) == 'D')
      j = 5;
    else
    {
      data->mode = OLC_MODE_ROOM0;
      olc_show_menu(data);
      return;
    }

    if (world[data->rnum].dir_option[j])
    {
      if (world[data->rnum].dir_option[j]->to_room)
        world[data->rnum].dir_option[j]->to_room = 0;
    }
    olc_show_menu(data);
    break;

#endif
    break;
  default:
    SEND_TO_Q("Unknown OLC room mode, how can I parse commands?!\r\n",
              data->desc);

  }                             /* switch */
}



void olc_room_menu(char *buf1, struct olc_data *data)
{
  struct extra_descr_data *ex_desc;
  int      i, j;

  switch (data->mode)
  {
  case OLC_MODE_ROOM0:
    strcpy(buf1, "&+YRoom Editor&N\r\n\r\n");
    sprintf(buf1 + strlen(buf1),
            " &+WA&N. Edit room name (%.55s&N)\r\n", world[data->rnum].name);
    strcat(buf1, " &+WB&N. Edit room description\r\n");
    sprintf(buf1 + strlen(buf1),
            " &+WC&N. Edit room flags (&+L%lu&N)\r\n",
            world[data->rnum].room_flags);
    sprintf(buf1 + strlen(buf1),
            " &+WD&N. Edit room sector type (&+y%s&N)\r\n",
            sector_types[(int) world[data->rnum].sector_type]);

    ex_desc = world[data->rnum].ex_description;
    i = 0;
    while (ex_desc)
    {
      ex_desc = ex_desc->next;
      i++;
    }
    sprintf(buf1 + strlen(buf1),
            " &+WE&N. Edit room extra descriptions (&+y%d keyword%s exist&N)\r\n",
            i, (i == 1) ? "" : "s");
    strcat(buf1, " &+WF&N. Edit room exits (&+yexits existing:");
    for (i = 0; i < NUM_EXITS; i++)
      if (world[data->rnum].dir_option[i])
        sprintf(buf1 + strlen(buf1), " %c", *olc_dirs[i]);

    strcat(buf1, "&N)\r\n");

    strcat(buf1, " &+LG&N. Edit fall chance, mana, and current.\r\n");
    sprintf(buf1 + strlen(buf1),
            "\r\n &+WS&N. Save all rooms in this zone. (&+y%.43s&N)\r\n",
            zone_table[world[data->rnum].zone].filename);
    strcat(buf1, "\r\n"
           " Hit &+W[ENTER]&N on a blank line when done, and preceeded by a '&+W?&N' for\r\n"
           " this menu again.\r\n" "\r\n");
    break;

  case OLC_MODE_ROOM_FLAGS:
    strcpy(buf1, "&+YRoom Flag Editor&N\r\n\r\n");
    olc_build_bitflag_menu32(buf1, world[data->rnum].room_flags, room_bits);
    strcat(buf1,
           "\r\nChoose a flag to toggle, or hit &+W[ENTER]&N on a blank line when done.\r\n\r\n");
    break;

  case OLC_MODE_ROOM_SECT:
    strcpy(buf1, "&+YRoom Sector Editor&N\r\n\r\n");
    olc_build_flag_menu8(buf1, (ubyte) world[data->rnum].sector_type,
                         sector_types);
    strcat(buf1,
           "\r\nChoose a new sector type, and hit &+W[ENTER]&N on a blank line when done.\r\n\r\n");
    break;

  case OLC_MODE_XTRA:
    strcpy(buf1, "&+YRoom Extra Description Editor&N\r\n\r\n");

    olc_room_xtra_list(buf1, data->rnum);

    strcat(buf1, "\r\n" " &+WA.&N Add a new extra description\r\n");
    if (world[data->rnum].ex_description)
      strcat(buf1, " &+WB.&N Delete an extra description\r\n");
    strcat(buf1,
           "\r\nChoose a keyword to edit, an option, or hit &+W[ENTER]&N on a blank line when done.\r\n\r\n");
    break;

  case OLC_MODE_XTRA1:
    sprintf(buf1, "&+YEditting Extra Description:&N &+c%s&N\r\n\r\n",
            ((struct extra_descr_data *) (data->misc))->keyword);
    strcat(buf1, " &+WA.&N Edit Keyword(s)\r\n");
    strcat(buf1, " &+WB.&N Edit Description\r\n");
    strcat(buf1,
           "\r\nChoose an option, or hit &+W[ENTER]&N on a blank line when done\r\n\r\n");
    break;

  case OLC_MODE_XTRA_DEL:
    if (!world[data->rnum].ex_description)
    {
      SEND_TO_Q("&+RNo extra descriptions are defined!\r\n", data->desc);
      data->mode = OLC_MODE_XTRA;
      olc_show_menu(data);
      return;
    }
    sprintf(buf1, "&+YDELETING Extra Description&N\r\n\r\n");
    olc_room_xtra_list(buf1, data->rnum);
    strcat(buf1,
           "\r\nChoose an extra description to delete, or hit [ENTER] on a blank line when done.\r\n\r\n");
    break;

  case OLC_MODE_EXIT:
    strcpy(buf1, "&+YRoom Exit Editor&N\r\n\r\n");
    j = 0;
    for (i = 0; i < NUM_EXITS; i++)
      if (world[data->rnum].dir_option[i])
      {
        j++;
        sprintf(buf1 + strlen(buf1),
                " &+W%c.&N Edit %s exit\r\n", *olc_dirs[i], dirs[i]);
      }
    strcat(buf1, "\r\n");
    if (j != 6)
      strcat(buf1, " &+W1.&N Add another exit\r\n");
    if (j)
      strcat(buf1, " &+W2.&N Delete an exit\r\n");

    strcat(buf1,
           "\r\nChoose an exit to edit, an option, or hit &+W[ENTER]&N on a blank line when done.\r\n\r\n");
    break;

  }
  return;
}











static void olc_room_xtra_list(char *str, int rnum)
{
  int      i;
  struct extra_descr_data *ex_desc;

  ex_desc = world[rnum].ex_description;
  i = 1;
  while (ex_desc)
  {
    sprintf(str + strlen(str), " &+W%2d.&N %s\r\n", i, ex_desc->keyword);
    ex_desc = ex_desc->next;
    i++;
  }
}

static void olc_save_room(FILE * f1, int real_num)
{
  struct extra_descr_data *ex_data;
  char    *t;
  int      i;

  /*

     #<room number>
     <name of room >~
     <description>
     ~
     <zone #> <room_flags> <sector_type>
     <direction fields>
     <direction sub-fields (if direction exists)>
     <extra description fields>
     <extra description sub-fields (if extra descriptions exist)>
     <optional fields>

   */

  fprintf(f1, "#%d\n", world[real_num].number);
  fprintf(f1, "%s~\n", world[real_num].name);

  /* Because db.c insists on embedding CR's in the description (and
     any other multi-line field from the wld file..), I need to write
     it out to disk the very hard way.. */

  t = world[real_num].description;
  if (t)
    while (*t)
    {
      if (*t != '\r')
        fputc(*t, f1);
      t++;
    }
  fputs("~\n", f1);

  /* VERY, VERY, VERY IMPORTANT NOTE!!!!!!! */

  /*  Note: I'm writing the zone number as zone 0.  Why?  because not
     only does db.c not give a fuck whats in this field, but it
     doesn't even save it in a variable. */

  fprintf(f1, "0 %lu %d\n",
          world[real_num].room_flags, world[real_num].sector_type);

  /* DIRECTIONAL FIELDS */
/*
   Dx 
   <exit desc>
   ~
   <keyword list>~
   <door flag> <key_number> <to-room>
 */

  for (i = 0; i < NUM_EXITS; i++)
    if (world[real_num].dir_option[i])
    {
      struct room_direction_data *ddata;
      int      j;

      ddata = world[real_num].dir_option[i];

      fprintf(f1, "D%d\n", i);

      if (ddata->general_description)
      {
        t = ddata->general_description;
        while (*t)
        {
          if (*t != '\r')
            fputc(*t, f1);
          t++;
        }
      }
      fputs("~\n", f1);
      if (ddata->keyword)
        fputs(ddata->keyword, f1);
      fputs("~\n", f1);

      j = 0;
      /* build the door flag data */
      if (ddata->exit_info & EX_ISDOOR)
        j = 1;
      if (ddata->exit_info & EX_PICKABLE)
        j = 2;
      if (ddata->exit_info & EX_PICKPROOF)
        j = 3;

      if (ddata->exit_info & EX_SECRET)
        j += 4;

      if (ddata->exit_info & EX_BLOCKED)
        j += 8;

      fprintf(f1, "%d %d %d\n", j, ddata->key,
              (ddata->to_room == NOWHERE) ? -1 :
              world[ddata->to_room].number);
    }
  ex_data = world[real_num].ex_description;
  while (ex_data)
  {
    fprintf(f1, "E\n%s~\n", ex_data->keyword);
    t = ex_data->description;
    while (*t)
    {
      if (*t != '\r')
        fputc(*t, f1);
      t++;
    }
    fputs("~\n", f1);
    ex_data = ex_data->next;
  }
  if (world[real_num].chance_fall)
    fprintf(f1, "F\n%d\n", world[real_num].chance_fall);
  if (world[real_num].current_speed)
    fprintf(f1, "C\n%d %d\n", world[real_num].current_speed,
            world[real_num].current_direction);
  fprintf(f1, "S\n");

}


void olc_save_wld(int zone)
{
  char     file[MAX_STRING_LENGTH], filebak[MAX_STRING_LENGTH];
  char    *zone_name, *t;
  int      i;
  FILE    *f1;

  /* 1) build a filename */

  strcpy(file, "areas/wld/");

  t = zone_name = str_dup(zone_table[zone].filename);
  while (*t)
  {
    if (*t == ' ')
      *t = '_';
    t++;
  }
  strcat(file, zone_name);
  FREE(zone_name);
  strcat(file, ".wld");
  sprintf(filebak, "%s.bak", file);

  /* 2) open it */

  f1 = fopen(filebak, "w");
  if (!f1)
  {
    logit(LOG_EXIT, "Unable to open file: %s", file);
    raise(SIGSEGV);
  }
  /* 3) for each room in this zone, call olc_save_room() */

  for (i = zone_table[zone].real_bottom; i <= zone_table[zone].real_top; i++)
    olc_save_room(f1, i);

  fclose(f1);

  /* 4) Sinc it musta worked by here, move olc version to original */
  if (rename(filebak, file) == -1)
  {
    logit(LOG_FILE, "Problem moving %s to %s during OLC.\n", filebak, file);
    return;
  }
}





/* function used as editor callback */
void olc_room_callback(P_desc desc, int e_data, char *txt)
{
  struct olc_data *data;
  struct extra_descr_data *ex_data;
  int      i;

  data = (struct olc_data *) e_data;

  switch (data->mode)
  {
  case OLC_MODE_ROOM2:         /* room desc */
    data->mode = OLC_MODE_ROOM0;
    if (!txt)
      return;

    if (strchr(txt, '~'))
    {
      SEND_TO_Q("You can't currently have tildes in room descriptions.\r\n",
                desc);
      SEND_TO_Q("Aborted!\r\n", desc);
      FREE(txt);
    }
    /* loop in search of duped descs.. */
    for (i = 0; i <= top_of_world; i++)
      if ((i != data->rnum) &&
          (world[i].description == world[data->rnum].description))
        break;
    /* ONLY if no dupes found, free the old one... */
    if (i > top_of_world)
      FREE(world[i].description);

    strcat(txt, "&n");
    world[data->rnum].description = txt;
    break;

  case OLC_MODE_XTRA3:
    data->mode = OLC_MODE_XTRA1;
    if (!txt)
      return;

    if (txt[strlen(txt) - 1] != '\n')
      strcat(txt, "\n");

    ex_data = (struct extra_descr_data *) data->misc;
    if (ex_data->description)
      FREE(ex_data->description);

    ex_data->description = txt;
    break;

  default:
    logit(LOG_EXIT, "olc_room_callback: called with impossible mode");
    raise(SIGSEGV);
  }

}
