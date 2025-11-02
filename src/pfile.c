/*
 * pfile tool for DurisMUD
 *
 * Can be used to scan the player files for player characters 
 * matching given constraints which include class, race, name,
 * owned items.
 *
 * Added as a build target to the main Makefile.
 * Should be linked with files.c, mm.c, skills.c compiled
 * with _PFILE_ def.
 *
 * Tharkun, January 2004
 */  
#include <dirent.h>
#include <fnmatch.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "structs.h"
  
#define FLG_SUMMARY        1
#define FLG_STOP           2
#define FLG_VERBOSE        4

extern int restoreCharOnly(P_char, char *);
extern int restoreItemsOnly(P_char, int);
extern const struct race_names race_names_table[];
extern const struct class_names class_names_table[];
extern int flag2idx(int);


void print_help(char *msg)
{
  printf("Syntax error: %s\n", msg);
  printf("Usage:\n" 
          "\tpfile \"<name_pattern>\" [-c <class>] [-r <race>]\n" 
          "\t\t[-i <item_vnum> [-in \"<item_pattern>\"]] [-t] [-v] [-s]\n\n");
  printf("\t<name_pattern> - can use wildcards like '*' and '?', remember\n"
           "\t\tto surround the pattern with quotation marks\n" 
          "\t<item_pattern> - same as name pattern, applied to restrung items\n"
           "\t-t\tprint out the summary\n" 
          "\t-s\tstop after the first match has been found\n" 
          "\t-v\tprint more info, lists items matching pattern in particular\n\n");
  exit(-1);
} 

/* what in the hell is this shit? */
void scan_all(char *pattern, unsigned int pc_class, unsigned int race,
                   int item, char *ipattern, unsigned int flags)
{
  P_char ch;
  P_obj obj;
  DIR * pf_dir;
  struct dirent *pf_entry;
  char    dname[256];
  char    fname[256];
  char    letter;
  int     item_count, total_items, total;
  char    *dot_index;
  ch = (struct char_data *) malloc(sizeof(struct char_data));
  ch->only.pc = (struct pc_only_data *) malloc(sizeof(struct pc_only_data));
  total_items = total = 0;
  for (letter = 'a'; letter <= 'z'; letter++)
  {
    snprintf(dname, 256, "Players/%c", letter);
    pf_dir = opendir(dname);
    if (!pf_dir)
      print_help("can't find pfiles under Players/?");
    while (pf_entry = readdir(pf_dir))
    {
      strcpy(fname, pf_entry->d_name);
      dot_index = rindex(fname, '.');
      if (dot_index && strstr(fname, ".locker") != dot_index)
        continue;
      if (!strcmp(fname, "CVS"))
        continue;
      if (fnmatch(pattern, fname, 0))
        continue;
      if (restoreCharOnly(ch, fname) < 0)
        continue;
      if (item)
      {
        ch->carrying = NULL;
        restoreItemsOnly(ch, 0);
      }
      if (race && race != ch->player.race)
      {
        continue;
      }
      else if (pc_class && pc_class != ch->player.m_class)
      {
        continue;
      }
      else if (item)
      {
        item_count = 0;
        for (obj = ch->carrying; obj; obj = obj->next_content)
          if (obj->R_num == item)
            if (!ipattern)
              item_count++;
        
            else if (obj->short_description && !fnmatch(ipattern, obj->short_description, 0))
            {
              if (flags & FLG_VERBOSE)
                printf("%s\n", obj->short_description);
              item_count++;
            }
        if (!item_count)
          continue;
        total_items += item_count;
      }
      total++;
      if (item)
        printf("%s is a level %d %s %s [%d]\n", ch->player.name,
                ch->player.level, race_names_table[ch->player.race].normal,
                class_names_table[flag2idx(ch->player.m_class)].normal,
                item_count);
      
      else
        printf("%s is a level %d %s %s\n", ch->player.name, ch->player.level,
                race_names_table[ch->player.race].normal,
                class_names_table[flag2idx(ch->player.m_class)].normal);
      if (flags & FLG_STOP)
        break;
    }
    if (total && (flags & FLG_STOP))
      break;
  }
  if (flags & FLG_SUMMARY)
  {
    printf("\n%d pfiles matched, found %d matching items\n", total,
            total_items);
  }
}


int main(int argc, char *argv[])
{
  unsigned int race = 0, pc_class = 0, item = 0;
  char   *pattern = "*";
  char   *ipattern = NULL;
  unsigned int i, j, flags = 0;

  if (argc == 1)
    print_help("Not enough arguments");
  for (i = 1; i < argc; i++)
  {
    if (!strcmp(argv[i], "-c"))
    {
      i++;
      for (j = 1; j <= CLASS_COUNT; j++)
        if (!strcasecmp(class_names_table[j].normal, argv[i]))
          break;
      if (j > CLASS_COUNT)
      {
        print_help("Non recognized class.");
      }
      pc_class = 1 << (j - 1);
    }
    else if (!strcmp(argv[i], "-r"))
    {
      i++;
      for (j = 1; j <= LAST_RACE; j++)
        if (!strcasecmp(race_names_table[j].no_spaces, argv[i]))
          break;
      if (j > LAST_RACE)
      {
        print_help("Non recognized race.");
      }
      race = j;
    }
    else if (!strcmp(argv[i], "-i"))
    {
      i++;
      if ((item = atoi(argv[i])) <= 0)
      {
        print_help("Invalid item number.");
      }
    }
    else if (!strcmp(argv[i], "-t"))
    {
      flags |= FLG_SUMMARY;
    }
    else if (!strcmp(argv[i], "-s"))
    {
      flags |= FLG_STOP;
    }
    else if (!strcmp(argv[i], "-v"))
    {
      flags |= FLG_VERBOSE;
    }
    else if (!strcmp(argv[i], "-in"))
    {
      i++;
      ipattern = argv[i];
    }
    else
    {
      pattern = argv[i];
    }
  }
  scan_all(pattern, pc_class, race, item, ipattern, flags);
}


