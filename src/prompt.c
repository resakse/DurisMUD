/*
 * ***************************************************************************
 *  File: prompt.c                                             Part of Duris *
 *  Usage: prompts, and infobars
 *  Copyright  1990, 1991 - see 'license.doc' for complete information.
 *  Copyright 1994 - 2008 - Duris Systems Ltd.
 * 
 * *************************************************************************** 
 */

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "comm.h"
#include "db.h"
#include "events.h"
#include "interp.h"
#include "olc.h"
#include "prototypes.h"
#include "structs.h"
#include "utils.h"
#include "mm.h"

extern P_desc descriptor_list;
extern P_index mob_index;

void make_prompt(void)
{
  P_char   t_ch = NULL, t_ch_f = NULL, tank = NULL, orig = NULL;
  P_obj    t_obj_f = NULL;
  P_desc   point, next_point;
  char     promptbuf[MAX_INPUT_LENGTH]; /*, pname[512]; */
  char     promptbuf2[MAX_INPUT_LENGTH], *pPrompt;
  snoop_by_data *snoop_by_ptr;
  int      percent, t_ch_p = 0;
  char     prompt_buf[256];
  bool     ansi;

  for( point = descriptor_list; point; point = next_point )
  {
    next_point = point->next;
    t_ch = point->character;
    orig = point->original;
    ansi = IS_ANSI_TERM(point);
    if (t_ch)
    {
      t_ch_f = GET_OPPONENT(t_ch);
      t_obj_f = t_ch->specials.destroying_obj;
    }

//  Dunno about this old code... Guess I'll leave it commented out.
//    if (!point->prompt_mode && !point->showstr_count && !point->str && !point->olc )
    if( !point->prompt_mode )
    {
      continue;
    }

    if (point->showstr_count)
    {
      if (ansi)
        snprintf(promptbuf, MAX_INPUT_LENGTH,
                "\n\033[32m[Return to continue, (q)uit, (r)efresh, (b)ack, or page number (%d/%d)]\033[0m\n",
                point->showstr_page, point->showstr_count);
      else
        snprintf(promptbuf, MAX_INPUT_LENGTH,
                "\n[Return to continue, (q)uit, (r)efresh, (b)ack, or page number (%d/%d)]\n",
                point->showstr_page, point->showstr_count);
      if (write_to_descriptor(point, promptbuf) < 0)
      {
        logit(LOG_COMM, "Closing socket on write error");
        close_socket(point);
      }
      // Reset the promptmode (decides when to display another prompt).
      point->prompt_mode = FALSE;
      continue;
    }
    if (point->str)
    {
      if (write_to_descriptor(point, "] ") < 0)
      {
        logit(LOG_COMM, "Closing socket on write error");
        close_socket(point);
      }
      point->prompt_mode = FALSE;
      continue;
    }
    /* Online zone editing maybe?
    if (point->olc)
    {
        olc_prompt(point->olc);
        continue;
    }
    */

    // Don't show a prompt if we're not in the game yet.
    // Don't show a prompt if awaiting a yes/no answer - SAM 7-94
    if( point->connected || point->confirm_state == CONFIRM_AWAIT )
    {
      point->prompt_mode = FALSE;
      continue;
    }

    if( t_ch )
    {
      t_ch_p = orig ? orig->only.pc->prompt : t_ch->only.pc->prompt;
    }

    if( ansi && t_ch && t_ch->desc && t_ch->desc->term_type == TERM_MSP )
    {
      strcpy(promptbuf, "\n<prompt>\n\033[32m<");
    }
    else if( ansi )
    {
      strcpy(promptbuf, "\033[32m<");
    }
    else
    {
      strcpy(promptbuf, "<");
    }

    /* infobar prompt */
    if( IS_SET((orig ? orig : t_ch)->specials.act, PLR_SMARTPROMPT) )
    {
      UpdateScreen(t_ch, 0);
      snprintf(promptbuf, MAX_INPUT_LENGTH, "&+R>&n");
      send_to_char(promptbuf, t_ch);
      point->prompt_mode = FALSE;
      return;
    }
    if( t_ch_p )
    {
      if( IS_SET(t_ch_p, PROMPT_HIT) )
      {
        if( ansi )
        {
          if( GET_MAX_HIT(t_ch) > 0 )
          {
            percent = (100 * GET_HIT(t_ch)) / GET_MAX_HIT(t_ch);
          }
          else
          {
            debug( "make_prompt: ch '%s' %d has less than 1 (%d) max hps?!?", J_NAME(t_ch),
              IS_NPC(t_ch) ? GET_VNUM(t_ch) : GET_PID(t_ch), GET_MAX_HIT(t_ch) );
            percent = -1;
          }

          // Healthy -> Green.
          if( percent >= 66 )
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;32m %dh", t_ch->points.hit);
          }
          // Wounded -> Brown.
          else if( percent >= 33 )
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[33m %dh", t_ch->points.hit);
          }
          // Hut bad -> Red.
          else if( percent >= 15 )
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[31m %dh", t_ch->points.hit);
          }
          // Nearing death -> Bright red on grey.
          else
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[31;40;1;5m %dh", t_ch->points.hit);
          }
        }
        else
        {
          // No color
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), " %dh", t_ch->points.hit);
        }
      }
      if( IS_SET(t_ch_p, PROMPT_MAX_HIT) )
      {
        if( ansi )
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;32m/%dH", GET_MAX_HIT(t_ch));
        }
        else
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "/%dH", GET_MAX_HIT(t_ch));
        }
      }
      if( IS_SET(t_ch_p, PROMPT_MANA) )
      {
        if( ansi )
        {
          if( GET_MAX_MANA(t_ch) > 0 )
          {
            percent = (100 * GET_MANA(t_ch)) / GET_MAX_MANA(t_ch);
          }
          else
          {
            debug( "make_prompt: ch '%s' %d has less than 1 (%d) max mana?!?", J_NAME(t_ch),
              IS_NPC(t_ch) ? GET_VNUM(t_ch) : GET_PID(t_ch), GET_MAX_MANA(t_ch) );
            percent = -1;
          }
          if (percent >= 66)
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;32m %dm", GET_MANA(t_ch));
          }
          else if (percent >= 33)
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;33m %dm", GET_MANA(t_ch));
          }
          else if (percent >= 0)
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;31m %dm", GET_MANA(t_ch));
          }
          else
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), " \033[31;40;1;5m%dm", GET_MANA(t_ch));
          }
        }
        else
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), " %dm", GET_MANA(t_ch));
        }
      }
      if( IS_SET(t_ch_p, PROMPT_MAX_MANA) )
      {
        if( ansi )
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;32m/%dM", GET_MAX_MANA(t_ch));
        }
        else
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "/%dM", GET_MAX_MANA(t_ch));
        }
      }
      if( IS_SET(t_ch_p, PROMPT_MOVE) )
      {
        if( ansi )
        {
          if( GET_MAX_VITALITY(t_ch) > 0 )
          {
            percent = (100 * GET_VITALITY(t_ch)) / GET_MAX_VITALITY(t_ch);
          }
          else
          {
            debug( "make_prompt: ch '%s' %d has less than 1 (%d) max moves?!?", J_NAME(t_ch),
              IS_NPC(t_ch) ? GET_VNUM(t_ch) : GET_PID(t_ch), GET_MAX_VITALITY(t_ch) );
            percent = -1;
          }

          if( percent >= 66 )
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;32m %dv", t_ch->points.vitality);
          }
          else if (percent >= 33)
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;33m %dv", t_ch->points.vitality);
          }
          else
          {
            snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;31m %dv", t_ch->points.vitality);
          }
        }
        else
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), " %dv", t_ch->points.vitality);
        }
      }
      if( IS_SET(t_ch_p, PROMPT_MAX_MOVE) )
      {
        if( ansi )
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "\033[0;32m/%dV", GET_MAX_VITALITY(t_ch));
        }
        else
        {
          snprintf(promptbuf + strlen(promptbuf), MAX_STRING_LENGTH - strlen(promptbuf), "/%dV", GET_MAX_VITALITY(t_ch));
        }
      }
      if( IS_SET(t_ch_p, PROMPT_STATUS) )
      {
        if( ansi )
        {
          strcat(promptbuf, " \033[0;36mPos:\033[0m");
        }
        else
        {
          strcat(promptbuf, " Pos:");
        }
        if( GET_POS(t_ch) == POS_STANDING )
        {
          if (ansi)
          {
            strcat(promptbuf, "\033[1;32m standing\033[0m");
          }
          else
          {
            strcat(promptbuf, " standing");
          }
        }
        else if (GET_POS(t_ch) == POS_SITTING)
        {
          if (ansi)
          {
            strcat(promptbuf, "\033[0;36m sitting\033[0m");
          }
          else
          {
            strcat(promptbuf, " sitting");
          }
        }
        else if (GET_POS(t_ch) == POS_KNEELING)
        {
          if (ansi)
          {
            strcat(promptbuf, "\033[0;36m kneeling\033[0m");
          }
          else
          {
            strcat(promptbuf, " kneeling");
          }
        }
        else if (GET_POS(t_ch) == POS_PRONE)
        {
          if (ansi)
          {
            strcat(promptbuf, "\033[0;31m on your ass\033[0m");
          }
          else
          {
            strcat(promptbuf, " on your ass");
          }
        }

#ifdef STANCES_ALLOWED
        if (ansi)
          strcat(promptbuf, " \033[0;36mSta:\033[0m");
        else
          strcat(promptbuf, " Sta:");

        if(IS_AFFECTED5(t_ch, AFF5_STANCE_DEFENSIVE))
        {
          if (ansi)
          strcat(promptbuf, "\033[0;33m Def\033[0m");
          else
          strcat(promptbuf, " Def");
        }
        else if(IS_AFFECTED5(t_ch, AFF5_STANCE_OFFENSIVE))
        {
          if (ansi)
            strcat(promptbuf, "\033[0;31m Off\033[0m");
          else
            strcat(promptbuf, " Off");
        }
        else
          if (ansi)
            strcat(promptbuf, "\033[0;32m none\033[0m");
          else
            strcat(promptbuf, " None");
#endif
      }

      if( IS_SET(t_ch_p, PROMPT_TWOLINE)
        && (t_ch_p & (PROMPT_HIT | PROMPT_MAX_HIT | PROMPT_MANA | PROMPT_MAX_MANA | PROMPT_MOVE | PROMPT_MAX_MOVE)) )
      {
        if( ansi )
        {
          strcat(promptbuf, "\033[0;32m >\n");
          snprintf(promptbuf2, MAX_STRING_LENGTH, "\033[32m<");
        }
        else
        {
          strcat(promptbuf, " >\n");
          snprintf(promptbuf2, MAX_STRING_LENGTH, "<");
        }
        pPrompt = promptbuf2 + strlen(promptbuf2);
      }
      else
      {
        pPrompt = promptbuf + strlen(promptbuf);
      }

      /* the prompt elements only active while fighting */
      if( t_ch_f && (t_ch->in_room == t_ch_f->in_room) )
      {
        /* TANK elements only active if... */
        if( (tank = GET_OPPONENT(t_ch_f)) && (t_ch->in_room == tank->in_room) )
        {
          if( IS_SET(t_ch_p, PROMPT_TANK_NAME))
          {
            if( ansi )
            {
              snprintf(pPrompt + strlen(pPrompt), MAX_STRING_LENGTH - strlen(pPrompt), " \033[0;34;1mT: %s",
                (t_ch != tank && !CAN_SEE(t_ch, tank)) ? "someone"
                : (IS_PC(tank) ? PERS(tank, t_ch, 0, true) : (FirstWord((tank)->player.name))));
            }
            else
            {
              snprintf(pPrompt + strlen(pPrompt), MAX_STRING_LENGTH - strlen(pPrompt), " T: %s",
                (t_ch != tank && !CAN_SEE(t_ch, tank)) ? "someone"
                : (IS_PC(tank) ? PERS(tank, t_ch, 0, true) : (FirstWord((tank)->player.name))));
            }
          }
          if( IS_SET(t_ch_p, PROMPT_STATUS) && IS_SET(t_ch_p, PROMPT_TANK_COND) )
          {
            if (ansi)
              strcat(pPrompt, " \033[0;36mTP:\033[0m");
            else
              strcat(pPrompt, " TP:");
            if (GET_POS(tank) == POS_STANDING)
              if (ansi)
                strcat(pPrompt, "\033[0;32m sta\033[0m");
              else
                strcat(pPrompt, " sta");
            else if (GET_POS(tank) == POS_SITTING)
              if (ansi)
                strcat(pPrompt, "\033[0;32m sit\033[0m");
              else
                strcat(pPrompt, " sit");
            else if (GET_POS(tank) == POS_KNEELING)
              if (ansi)
                strcat(pPrompt, "\033[0;32m kne\033[0m");
              else
                strcat(pPrompt, " kne");
            else if (GET_POS(tank) == POS_PRONE)
              if (ansi)
                strcat(pPrompt, "\033[0;32m ass\033[0m");
              else
                strcat(pPrompt, " ass");
          }
          if( IS_SET(t_ch_p, PROMPT_TANK_COND) )
          {
            if (ansi)
              strcat(pPrompt, "\033[0m TC:");
            else
              strcat(pPrompt, " TC:");
            if (GET_MAX_HIT(tank) > 0)
            {
              percent = (100 * GET_HIT(tank)) / GET_MAX_HIT(tank);
            }
            else
            {
              percent = -1;
            }
            if (percent >= 100)
            {
              if (ansi)
                strcat(pPrompt, "\033[0;32m excellent");
              else
                strcat(pPrompt, " excellent");
            }
            else if (percent >= 90)
            {
              if (ansi)
                strcat(pPrompt, "\033[0;33m few scratches");
              else
                strcat(pPrompt, " few scratches");
            }
            else if (percent >= 75)
            {
              if (ansi)
                strcat(pPrompt, "\033[0;33;40;1m small wounds");
              else
                strcat(pPrompt, " small wounds");
            }
            else if (percent >= 50)
            {
              if (ansi)
                strcat(pPrompt, "\033[0;35;40;1m few wounds");
              else
                strcat(pPrompt, " few wounds");
            }
            else if (percent >= 30)
            {
              if (ansi)
                strcat(pPrompt, "\033[0;35m nasty wounds");
              else
                strcat(pPrompt, " nasty wounds");
            }
            else if (percent >= 15)
            {
              if (ansi)
                strcat(pPrompt, "\033[0;31;40;1m pretty hurt");
              else
                strcat(pPrompt, " pretty hurt");
            }
            else if (percent >= 0)
            {
              if (ansi)
                strcat(pPrompt, "\033[0;31m awful");
              else
                strcat(pPrompt, " awful");
            }
            else
            {
              if (ansi)
                strcat(pPrompt, "\033[0;31m bleeding, close to death");
              else
                strcat(pPrompt, " bleeding, close to death");
            }
          }
        }
        if( IS_SET(t_ch_p, PROMPT_ENEMY) )
        {
          if( ansi )
          {
            snprintf(pPrompt + strlen(pPrompt), MAX_STRING_LENGTH - strlen(pPrompt), " \033[0;31mE: %s",
              (!CAN_SEE(t_ch, t_ch_f)) ? "someone" : (IS_PC(t_ch_f) ? PERS(t_ch_f, t_ch, 0, true)
              : (FirstWord((t_ch_f)->player.name))));
          }
          else
          {
            snprintf(pPrompt + strlen(pPrompt), MAX_STRING_LENGTH - strlen(pPrompt), " E: %s",
              (!CAN_SEE(t_ch, t_ch_f) ) ? "someone" : (IS_PC(t_ch_f) ? PERS(t_ch_f, t_ch, 0, true)
              : (FirstWord((t_ch_f)->player.name))));
          }
        }
        if( IS_SET(t_ch_p, PROMPT_STATUS) && IS_SET(t_ch_p, PROMPT_ENEMY_COND) )
        {
          if (ansi)
            strcat(pPrompt, " \033[0;36mEP:\033[0m");
          else
            strcat(pPrompt, " EP:");
          if (GET_POS(t_ch_f) == POS_STANDING)
            if (ansi)
              strcat(pPrompt, "\033[0;32m sta\033[0m");
            else
              strcat(pPrompt, " sta");
          else if (GET_POS(t_ch_f) == POS_SITTING)
            if (ansi)
              strcat(pPrompt, "\033[0;32m sit\033[0m");
            else
              strcat(pPrompt, " sit");
          else if (GET_POS(t_ch_f) == POS_KNEELING)
            if (ansi)
              strcat(pPrompt, "\033[0;32m kne\033[0m");
            else
              strcat(pPrompt, " kne");
          else if (GET_POS(t_ch_f) == POS_PRONE)
            if (ansi)
              strcat(pPrompt, "\033[0;32m ass\033[0m");
            else
              strcat(pPrompt, " ass");
        }
        if( IS_SET(t_ch_p, PROMPT_ENEMY_COND) )
        {
          if (GET_MAX_HIT(t_ch_f) > 0)
          {
            percent = (100 * GET_HIT(t_ch_f)) / GET_MAX_HIT(t_ch_f);
          }
          else
          {
            percent = -1;
          }
          if (ansi)
            strcat(pPrompt, "\033[0;36m EC:");
          else
            strcat(pPrompt, " EC:");
          if (percent >= 100)
          {
            if (ansi)
              strcat(pPrompt, "\033[0;32m excellent");
            else
              strcat(pPrompt, " excellent");
          }
          else if (percent >= 90)
          {
            if (ansi)
              strcat(pPrompt, "\033[0;33m few scratches");
            else
              strcat(pPrompt, " few scratches");
          }
          else if (percent >= 75)
          {
            if (ansi)
              strcat(pPrompt, "\033[0;33;40;1m small wounds");
            else
              strcat(pPrompt, " small wounds");
          }
          else if (percent >= 50)
          {
            if (ansi)
              strcat(pPrompt, "\033[0;35;40;1m few wounds");
            else
              strcat(pPrompt, " few wounds");
          }
          else if (percent >= 30)
          {
            if (ansi)
              strcat(pPrompt, "\033[0;35m nasty wounds");
            else
              strcat(pPrompt, " nasty wounds");
          }
          else if (percent >= 15)
          {
            if (ansi)
              strcat(pPrompt, "\033[0;31;40;1m pretty hurt");
            else
              strcat(pPrompt, " pretty hurt");
          }
          else if (percent >= 0)
          {
            if (ansi)
              strcat(pPrompt, "\033[0;31m awful");
            else
              strcat(pPrompt, " awful");
          }
          else
          {
            if (ansi)
              strcat(pPrompt, "\033[0;31m bleeding, close to death");
            else
              strcat(pPrompt, " bleeding, close to death");
          }
        }
      }
      if( t_obj_f )
      {
        if( ansi )
        {
          strcat(pPrompt, "\033[32m E: ");
          strcat(pPrompt, FirstWord(t_obj_f->name) );
          strcat(pPrompt, " EC: " );
          if(t_obj_f->condition > 90)
            strcat(pPrompt, "\033[1;32m");
          else if(t_obj_f->condition > 70)
            strcat(pPrompt, "\033[32m");
          else if(t_obj_f->condition > 50)
            strcat(pPrompt, "\033[0;33m");
          else if(t_obj_f->condition > 20)
            strcat(pPrompt, "\033[0;31m");
          else
            strcat(pPrompt, "\033[1;31m");

          snprintf(pPrompt + strlen(pPrompt), MAX_STRING_LENGTH - strlen(pPrompt), "%d ", t_obj_f->condition );
        }
        else
        {
          strcat(pPrompt, " E: ");
          strcat(pPrompt, FirstWord(t_obj_f->name) );
          snprintf(pPrompt + strlen(pPrompt), MAX_STRING_LENGTH - strlen(pPrompt), " EC: %d", t_obj_f->condition );
        }
      }

      if( IS_SET(t_ch_p, PROMPT_VIS) && IS_TRUSTED(t_ch) )
      {
        if( ansi )
        {
          strcat(pPrompt, "\033[0;35m");
        }
        snprintf(pPrompt + strlen(pPrompt), MAX_STRING_LENGTH - strlen(pPrompt), " Vis: %d", t_ch->only.pc->wiz_invis);
      }
      if( IS_SET(t_ch->specials.act, PLR_AFK) )
      {
        if( ansi )
          strcat(pPrompt, "\033[0m (\033[31;40;1mAFK\033[0m)");
        else
          strcat(pPrompt, " (AFK)");
      }
    }
    else
    {
      pPrompt = promptbuf;
    }

    if( ansi )
      strcat(pPrompt, "\033[0;32m> ");
    else
      strcat(pPrompt, " > ");

    if( t_ch && t_ch->desc && t_ch->desc->term_type == TERM_MSP )
    {
      strcat(pPrompt, "\033[0m\n</prompt>\n");
    }

    if( t_ch_p )
    {
      /* give the log some prompts */
      prompt_buf[0] = '\0';
      append_prompt(point->character, prompt_buf);
      write_to_pc_log(point->character, prompt_buf, LOG_PUBLIC);

      snoop_by_ptr = point->snoop.snoop_by_list;
      while( snoop_by_ptr )
      {
        write_to_q("&+B%&n ", &snoop_by_ptr->snoop_by->desc->output, 1);
        write_to_q(promptbuf, &snoop_by_ptr->snoop_by->desc->output, 1);
        if( IS_SET(t_ch_p, PROMPT_TWOLINE) )
        {
          write_to_q("&+B%&n ", &snoop_by_ptr->snoop_by->desc->output, 1);
          write_to_q(promptbuf2, &snoop_by_ptr->snoop_by->desc->output, 1);
        }
        write_to_q("\n", &snoop_by_ptr->snoop_by->desc->output, 1);
        process_output( snoop_by_ptr->snoop_by->desc );

        snoop_by_ptr = snoop_by_ptr->next;
      }

      point->prompt_mode = FALSE;
      if( write_to_descriptor(point, promptbuf) < 0 )
      {
        logit(LOG_COMM, "Closing socket on write error: promptbuf.");
        close_socket(point);
        continue;
      }
      if( IS_SET(t_ch_p, PROMPT_TWOLINE) && write_to_descriptor(point, promptbuf2) < 0)
      {
        logit(LOG_COMM, "Closing socket on write error: promptbuf2.");
        close_socket(point);
        continue;
      }
      if (ansi && point->character && point->character->desc && !(point->character->desc->term_type == TERM_MSP))
      {
        if (write_to_descriptor(point, "\033[0m") < 0)
        {
          logit(LOG_COMM, "Closing socket on write error: normalize.");
          close_socket(point);
          continue;
        }
      }
    }
  }
}


/* Converts numerical data to a dynamic line for infobar */
char    *make_bar(long val, long max, long len)
{
  static char buf[512];
  int      i, n;
  int      percent;

  strcpy(buf, "&+R<&n");

  percent = (100 * val) / max;
  if (percent >= 100)
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+g");
  else if (percent >= 90)
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+y");
  else if (percent >= 75)
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+Y");
  else if (percent >= 50)
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+M");
  else if (percent >= 30)
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+m");
  else if (percent >= 15)
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+R");
  else
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+r");

  if (val > max)
  {
    for (i = 0; i < len; i++)
      snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "|");
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&n+");
  }
  else
  {
    for (i = ((val * len) / max), n = 0; n < i; n++)
      snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "|");
    snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&n");
    while ((n++) < len)
      snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "|");
  }
  snprintf(buf + strlen(buf), MAX_STRING_LENGTH - strlen(buf), "&+R>&n");

  return buf;
}

/* Keeps the infobar updated. Will look like shit if the screen has not
   been previously initialized! */
void UpdateScreen(P_char ch, int update)
{
  char     buf[255];
  int      size, percent;
  P_char   enemy = NULL, tank = NULL;

  size = ch->only.pc->screen_length + 1;
  if (GET_OPPONENT(ch))
    enemy = GET_OPPONENT(ch);

/* hits */
  snprintf(buf, 255, VT_CURSAVE);
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size - 3, 13);
  send_to_char(buf, ch);
  snprintf(buf, 255, "          ");
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size - 3, 13);
  send_to_char(buf, ch);
  snprintf(buf, 255, "%s", make_bar(GET_HIT(ch), GET_MAX_HIT(ch), 50));
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURREST);
  send_to_char(buf, ch);

/* moves */
  snprintf(buf, 255, VT_CURSAVE);
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size - 2, 13);
  send_to_char(buf, ch);
  snprintf(buf, 255, "          ");
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size - 2, 13);
  send_to_char(buf, ch);
  snprintf(buf, 255, "%s", make_bar(GET_VITALITY(ch), GET_MAX_VITALITY(ch), 50));
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURREST);
  send_to_char(buf, ch);

/* mana */
  if (IS_SET(ch->only.pc->prompt, PROMPT_MANA))
  {
    snprintf(buf, 255, VT_CURSAVE);
    send_to_char(buf, ch);
    snprintf(buf, 255, VT_CURSPOS, size - 1, 13);
    send_to_char(buf, ch);
    snprintf(buf, 255, "          ");
    send_to_char(buf, ch);
    snprintf(buf, 255, VT_CURSPOS, size - 1, 13);
    send_to_char(buf, ch);
    snprintf(buf, 255, "%s", make_bar(GET_MANA(ch), GET_MAX_MANA(ch), 50));
    send_to_char(buf, ch);
    snprintf(buf, 255, VT_CURREST);
    send_to_char(buf, ch);
  }

/* God visibility */
  if (IS_TRUSTED(ch))
  {
    snprintf(buf, 255, VT_CURSAVE);
    send_to_char(buf, ch);
    snprintf(buf, 255, VT_CURSPOS, size - 1, 70);
    send_to_char(buf, ch);
    snprintf(buf, 255, "     ");
    send_to_char(buf, ch);
    snprintf(buf, 255, VT_CURSPOS, size - 1, 70);
    send_to_char(buf, ch);
    snprintf(buf, 255, "%d", ch->only.pc->wiz_invis);
    send_to_char(buf, ch);
    snprintf(buf, 255, VT_CURREST);
    send_to_char(buf, ch);
  }

/* tank */
  snprintf(buf, 255, VT_CURSAVE);
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size, 7);
  send_to_char(buf, ch);
  snprintf(buf, 255, "          ");
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size, 7);
  send_to_char(buf, ch);
  if (enemy && (enemy->in_room == ch->in_room) &&
      IS_SET(ch->specials.act, PLR_DEBUG))
  {
    if ((tank = GET_OPPONENT(enemy)) && (ch->in_room == tank->in_room))
      snprintf(buf, 255, "%s", ch == tank ? "yourself" : !CAN_SEE(ch, tank) ?
              "someone" : J_NAME(tank));
    if (ch != tank)
    {
      if (GET_MAX_HIT(tank) > 0 && GET_HIT(tank) > 0)
        percent = (100 * GET_HIT(tank)) / GET_MAX_HIT(tank);
      else
        percent = -1;
      if (percent >= 100)
        strcat(buf, " excellent");
      else if (percent >= 90)
        strcat(buf, " few scratches");
      else if (percent >= 75)
        strcat(buf, " small wounds");
      else if (percent >= 50)
        strcat(buf, " few wounds");
      else if (percent >= 30)
        strcat(buf, " nasty wounds");
      else if (percent >= 15)
        strcat(buf, " pretty hurt");
      else if (percent >= 0)
        strcat(buf, " awful");
      else
        strcat(buf, " bleeding, close to death");
    }
  }
  else
    snprintf(buf, MAX_STRING_LENGTH, " N/A");
  send_to_char(buf, ch);
  snprintf(buf, MAX_STRING_LENGTH, VT_CURREST);
  send_to_char(buf, ch);

/* enemy */
  snprintf(buf, MAX_STRING_LENGTH, VT_CURSAVE);
  send_to_char(buf, ch);
  snprintf(buf, MAX_STRING_LENGTH, VT_CURSPOS, size, 50);
  send_to_char(buf, ch);
  snprintf(buf, MAX_STRING_LENGTH, "          ");
  send_to_char(buf, ch);
  snprintf(buf, MAX_STRING_LENGTH, VT_CURSPOS, size, 50);
  send_to_char(buf, ch);
  if (enemy && IS_SET(ch->specials.act, PLR_DEBUG))
  {
    snprintf(buf, MAX_STRING_LENGTH, " %s", !CAN_SEE(ch, enemy) ? "someone" : J_NAME(enemy));
    if (GET_MAX_HIT(enemy) > 0 && GET_HIT(enemy) > 0)
      percent = (100 * GET_HIT(enemy)) / GET_MAX_HIT(enemy);
    else
      percent = -1;
    if (percent >= 100)
      strcat(buf, " excellent");
    else if (percent >= 90)
      strcat(buf, " few scratches");
    else if (percent >= 75)
      strcat(buf, " small wounds");
    else if (percent >= 50)
      strcat(buf, " few wounds");
    else if (percent >= 30)
      strcat(buf, " nasty wounds");
    else if (percent >= 15)
      strcat(buf, " pretty hurt");
    else if (percent >= 0)
      strcat(buf, " awful");
    else
      strcat(buf, " bleeding, close to death");
  }
  else
    snprintf(buf, MAX_STRING_LENGTH, " N/A");
  send_to_char(buf, ch);

  snprintf(buf, MAX_STRING_LENGTH, VT_CURREST);
  send_to_char(buf, ch);
}

/* Sets up the static aspects of the infobar, and displays first update */
void InitScreen(P_char ch)
{
  char     buf[255];
  int      size;

  size = ch->only.pc->screen_length + 1;
  snprintf(buf, 255, VT_HOMECLR);
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_MARGSET, 0, size - 5);
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size - 4, 1);
  send_to_char(buf, ch);
  snprintf(buf, 255,
          "&+r-===================================Duris=====================================-&n");
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size - 3, 1);
  send_to_char(buf, ch);
  snprintf(buf, 255, "Vitality: ");
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size - 2, 1);
  send_to_char(buf, ch);
  snprintf(buf, 255, "Stamina : ");
  send_to_char(buf, ch);
  if (IS_SET(ch->only.pc->prompt, PROMPT_MANA))
  {
    snprintf(buf, 255, VT_CURSPOS, size - 1, 1);
    send_to_char(buf, ch);
    snprintf(buf, 255, "Mental: ");
    send_to_char(buf, ch);
  }

  if (IS_TRUSTED(ch))
  {
    snprintf(buf, 255, VT_CURSPOS, size - 1, 70);
    send_to_char(buf, ch);
    snprintf(buf, 255, "Vis: ");
    send_to_char(buf, ch);
  }
  snprintf(buf, 255, VT_CURSPOS, size, 1);
  send_to_char(buf, ch);
  snprintf(buf, 255, "Tank: ");
  send_to_char(buf, ch);
  snprintf(buf, 255, VT_CURSPOS, size, 40);
  send_to_char(buf, ch);
  snprintf(buf, 255, "Enemy: ");
  send_to_char(buf, ch);

/* first update */
  UpdateScreen(ch, 0);

/* put cursor back to top */
  snprintf(buf, 255, VT_CURSPOS, 0, 0);
  send_to_char(buf, ch);
}
