#include "comm.h"
#include "new_combat.h"
#include "prototypes.h"
#include "structs.h"
#include "utils.h"

/*
 * getBodyLocColorPercent : return address of a string based on percent (1-100)
 *
 *    percent : 1-100, percentage of curr out of max
 */

const char *getBodyLocColorPercent(const int percent)
{
  if (percent >= 100)
    return "&+g";
  else if (percent >= 90)
    return "&+y";
  else if (percent >= 75)
    return "&+Y";
  else if (percent >= 50)
    return "&+M";
  else if (percent >= 30)
    return "&+m";
  else if (percent >= 15)
    return "&+R";
  else if (percent >= 0)
    return "&+r";
  else
    return "&=rl";
}

/*
 * display_condition_body_loc : displays info about a char's specific body
 *                              body location to another char
 *
 *     ch : char to send info to
 *   vict : char sending info about
 *    loc : location getting info on
 */

void display_condition_body_loc(const P_char ch, const P_char vict,
                                const int loc)
{
  char     strn[256], strn2[256];
  int      currloc, maxloc, percent;

  if (!ch ||
      !vict /*|| (loc < BODY_LOC_LOWEST) || (loc > BODY_LOC_HIGHEST) */ )
  {
    if (ch)
      send_to_char("error in display_condition_body_loc\r\n", ch);
    return;
  }

  currloc = getBodyLocCurrHP(vict, loc);
  if (currloc < 0)
    currloc = 0;

  maxloc = getBodyLocMaxHP(vict, loc);

  percent = ((float) currloc / (float) maxloc) * 100;

  snprintf(strn2, 256, "%s%s&n", getBodyLocColorPercent(percent),
          getBodyLocStrn(loc, vict));

  if (IS_TRUSTED(ch) || (ch == vict))
  {
    snprintf(strn, 256, "  %48s &+L-&n %d&+g/&n%d &+c(%s%u%%&n&+c)&n\r\n",
            strn2, currloc, maxloc, getBodyLocColorPercent(percent), percent);
  }
  else
  {
    snprintf(strn, 256, "  %48s &+L-&n %s%u%%&n\r\n",
            strn2, getBodyLocColorPercent(percent), percent);
  }

  send_to_char(strn, ch);
}


/*
 * display_condition_paired_body_loc : displays info about two of char's body
 *                                     locs to another
 *
 *     ch : char to send info to
 *   vict : char sending info about
 *    loc : location getting info on
 *   loc2 : second location
 */

void display_condition_paired_body_loc(const P_char ch, const P_char vict,
                                       const int loc, const int loc2)
{
  char     strn[512], strn2[256];
  int      currloc, maxloc, percent, currloc2, maxloc2, percent2;

  if (!ch || !vict              /*|| (loc < BODY_LOC_LOWEST) || (loc > BODY_LOC_HIGHEST) ||
                                   (loc2 < BODY_LOC_LOWEST) || (loc2 > BODY_LOC_HIGHEST) */ )
  {
    if (ch)
      send_to_char("error in display_condition_paired_body_loc\r\n", ch);
    return;
  }

  currloc = getBodyLocCurrHP(vict, loc);
  if (currloc < 0)
    currloc = 0;

  maxloc = getBodyLocMaxHP(vict, loc);

  percent = ((float) currloc / (float) maxloc) * 100;


  currloc2 = getBodyLocCurrHP(vict, loc2);
  if (currloc2 < 0)
    currloc2 = 0;

  maxloc2 = getBodyLocMaxHP(vict, loc2);

  percent2 = ((float) currloc2 / (float) maxloc2) * 100;


  snprintf(strn2, 256, "%s%s&n&+w/&n%s%s&n",
          getBodyLocColorPercent(percent), getBodyLocStrn(loc, vict),
          getBodyLocColorPercent(percent2), getBodyLocStrn(loc2, vict));

  if (IS_TRUSTED(ch) || (ch == vict))
  {
    snprintf(strn, 512, "  %58s &+L-&n %d&+g/&n%d&+G|&n%d&+g/&n%d "
            "&+c(%s%u%%&n&+c/%s%u%%&n&+c)&n\r\n",
            strn2, currloc, maxloc, currloc2, maxloc2,
            getBodyLocColorPercent(percent), percent,
            getBodyLocColorPercent(percent2), percent2);
  }
  else
  {
    snprintf(strn, 512, "  %58s &+L-&n %s%u%%&n&+w/&n%s%u%%&n\r\n",
            strn2, getBodyLocColorPercent(percent), percent,
            getBodyLocColorPercent(percent2), percent2);
  }

  send_to_char(strn, ch);
}


/*
 * do_condition : user interface for displaying condition on somebody
 */

void do_condition(P_char ch, char *argument, int cmd)
{
  P_char   victim = NULL;
  char     name[MAX_STRING_LENGTH];
  int      phys_type;

  if (!argument || !*argument)
  {
    victim = ch;
  }
  else
  {
    one_argument(argument, name);

    if (!(victim = get_char_room_vis(ch, name)))
    {
      send_to_char("Check on the condition of who?\r\n", ch);
      return;
    }
  }

  if (!victim)
  {
    send_to_char("error..\r\n", ch);
    return;
  }

  phys_type = GET_PHYS_TYPE(victim);

  act("&+WCondition info for $N&+W -\r\n", FALSE, ch, 0, victim, TO_CHAR);

  switch (phys_type)
  {
  case PHYS_TYPE_HUMANOID:
    display_condition_body_loc(ch, victim, BODY_LOC_HUMANOID_HEAD);

    display_condition_body_loc(ch, victim, BODY_LOC_HUMANOID_CHIN);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_LEFT_EYE,
                                      BODY_LOC_HUMANOID_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_LEFT_EAR,
                                      BODY_LOC_HUMANOID_RIGHT_EAR);

    display_condition_body_loc(ch, victim, BODY_LOC_HUMANOID_NECK);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_UPPER_TORSO,
                                      BODY_LOC_HUMANOID_LOWER_TORSO);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_LEFT_SHOULDER,
                                      BODY_LOC_HUMANOID_RIGHT_SHOULDER);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_UPPER_LEFT_ARM,
                                      BODY_LOC_HUMANOID_LOWER_LEFT_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_UPPER_RIGHT_ARM,
                                      BODY_LOC_HUMANOID_LOWER_RIGHT_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_LEFT_ELBOW,
                                      BODY_LOC_HUMANOID_RIGHT_ELBOW);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_LEFT_WRIST,
                                      BODY_LOC_HUMANOID_LEFT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_RIGHT_WRIST,
                                      BODY_LOC_HUMANOID_RIGHT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_UPPER_LEFT_LEG,
                                      BODY_LOC_HUMANOID_LOWER_LEFT_LEG);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_UPPER_RIGHT_LEG,
                                      BODY_LOC_HUMANOID_LOWER_RIGHT_LEG);

    display_condition_paired_body_loc(ch, victim, BODY_LOC_HUMANOID_LEFT_KNEE,
                                      BODY_LOC_HUMANOID_RIGHT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_LEFT_ANKLE,
                                      BODY_LOC_HUMANOID_LEFT_FOOT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_HUMANOID_RIGHT_ANKLE,
                                      BODY_LOC_HUMANOID_RIGHT_FOOT);

    break;

  case PHYS_TYPE_FOUR_ARMED_HUMANOID:
    display_condition_body_loc(ch, victim, BODY_LOC_FOUR_ARMED_HUMANOID_HEAD);

    display_condition_body_loc(ch, victim, BODY_LOC_FOUR_ARMED_HUMANOID_CHIN);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LEFT_EYE,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LEFT_EAR,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_RIGHT_EAR);

    display_condition_body_loc(ch, victim, BODY_LOC_FOUR_ARMED_HUMANOID_NECK);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_TORSO,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_TORSO);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_LEFT_SHOULDER,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_RIGHT_SHOULDER);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_LEFT_UPPER_ARM,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_LEFT_UPPER_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_RIGHT_UPPER_ARM,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_RIGHT_UPPER_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_LEFT_ELBOW,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_RIGHT_ELBOW);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_LEFT_WRIST,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_LEFT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_RIGHT_WRIST,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_RIGHT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_LEFT_SHOULDER,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_RIGHT_SHOULDER);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_LEFT_LOWER_ARM,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_LEFT_LOWER_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_RIGHT_LOWER_ARM,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_RIGHT_LOWER_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_LEFT_ELBOW,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_RIGHT_ELBOW);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_LEFT_WRIST,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_LEFT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_RIGHT_WRIST,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_RIGHT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_LEFT_LEG,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_LEFT_LEG);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_UPPER_RIGHT_LEG,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LOWER_RIGHT_LEG);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LEFT_KNEE,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_RIGHT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LEFT_ANKLE,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_LEFT_FOOT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_RIGHT_ANKLE,
                                      BODY_LOC_FOUR_ARMED_HUMANOID_RIGHT_FOOT);

    break;

  case PHYS_TYPE_QUADRUPED:
    display_condition_body_loc(ch, victim, BODY_LOC_QUADRUPED_HEAD);

    display_condition_body_loc(ch, victim, BODY_LOC_QUADRUPED_CHIN);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_LEFT_EYE,
                                      BODY_LOC_QUADRUPED_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_LEFT_EAR,
                                      BODY_LOC_QUADRUPED_RIGHT_EAR);

    display_condition_body_loc(ch, victim, BODY_LOC_QUADRUPED_NECK);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_FRONT_BODY,
                                      BODY_LOC_QUADRUPED_REAR_BODY);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_UPPER_LEFT_FRONT_LEG,
                                      BODY_LOC_QUADRUPED_LOWER_LEFT_FRONT_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_QUADRUPED_LEFT_FRONT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_LEFT_FRONT_ANKLE,
                                      BODY_LOC_QUADRUPED_LEFT_FRONT_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_UPPER_RIGHT_FRONT_LEG,
                                      BODY_LOC_QUADRUPED_LOWER_RIGHT_FRONT_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_QUADRUPED_RIGHT_FRONT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_RIGHT_FRONT_ANKLE,
                                      BODY_LOC_QUADRUPED_RIGHT_FRONT_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_UPPER_LEFT_REAR_LEG,
                                      BODY_LOC_QUADRUPED_LOWER_LEFT_REAR_LEG);

    display_condition_body_loc(ch, victim, BODY_LOC_QUADRUPED_LEFT_REAR_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_LEFT_REAR_ANKLE,
                                      BODY_LOC_QUADRUPED_LEFT_REAR_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_UPPER_RIGHT_REAR_LEG,
                                      BODY_LOC_QUADRUPED_LOWER_RIGHT_REAR_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_QUADRUPED_RIGHT_REAR_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_QUADRUPED_RIGHT_REAR_ANKLE,
                                      BODY_LOC_QUADRUPED_RIGHT_REAR_HOOF);

    break;

  case PHYS_TYPE_CENTAUR:
    display_condition_body_loc(ch, victim, BODY_LOC_CENTAUR_HEAD);

    display_condition_body_loc(ch, victim, BODY_LOC_CENTAUR_CHIN);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_LEFT_EYE,
                                      BODY_LOC_CENTAUR_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_LEFT_EAR,
                                      BODY_LOC_CENTAUR_RIGHT_EAR);

    display_condition_body_loc(ch, victim, BODY_LOC_CENTAUR_NECK);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_UPPER_TORSO,
                                      BODY_LOC_CENTAUR_LOWER_TORSO);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_LEFT_SHOULDER,
                                      BODY_LOC_CENTAUR_RIGHT_SHOULDER);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_UPPER_LEFT_ARM,
                                      BODY_LOC_CENTAUR_LOWER_LEFT_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_UPPER_RIGHT_ARM,
                                      BODY_LOC_CENTAUR_LOWER_RIGHT_ARM);

    display_condition_paired_body_loc(ch, victim, BODY_LOC_CENTAUR_LEFT_ELBOW,
                                      BODY_LOC_CENTAUR_RIGHT_ELBOW);

    display_condition_paired_body_loc(ch, victim, BODY_LOC_CENTAUR_LEFT_WRIST,
                                      BODY_LOC_CENTAUR_LEFT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_RIGHT_WRIST,
                                      BODY_LOC_CENTAUR_RIGHT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_FRONT_HORSE_BODY,
                                      BODY_LOC_CENTAUR_REAR_HORSE_BODY);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_UPPER_LEFT_FRONT_LEG,
                                      BODY_LOC_CENTAUR_LOWER_LEFT_FRONT_LEG);

    display_condition_body_loc(ch, victim, BODY_LOC_CENTAUR_LEFT_FRONT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_LEFT_FRONT_ANKLE,
                                      BODY_LOC_CENTAUR_LEFT_FRONT_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_UPPER_RIGHT_FRONT_LEG,
                                      BODY_LOC_CENTAUR_LOWER_RIGHT_FRONT_LEG);

    display_condition_body_loc(ch, victim, BODY_LOC_CENTAUR_RIGHT_FRONT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_RIGHT_FRONT_ANKLE,
                                      BODY_LOC_CENTAUR_RIGHT_FRONT_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_UPPER_LEFT_REAR_LEG,
                                      BODY_LOC_CENTAUR_LOWER_LEFT_REAR_LEG);

    display_condition_body_loc(ch, victim, BODY_LOC_CENTAUR_LEFT_REAR_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_LEFT_REAR_ANKLE,
                                      BODY_LOC_CENTAUR_LEFT_REAR_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_UPPER_RIGHT_REAR_LEG,
                                      BODY_LOC_CENTAUR_LOWER_RIGHT_REAR_LEG);

    display_condition_body_loc(ch, victim, BODY_LOC_CENTAUR_RIGHT_REAR_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_CENTAUR_RIGHT_REAR_ANKLE,
                                      BODY_LOC_CENTAUR_RIGHT_REAR_HOOF);

    break;

  case PHYS_TYPE_BIRD:
    display_condition_body_loc(ch, victim, BODY_LOC_BIRD_HEAD);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_BIRD_LEFT_EYE,
                                      BODY_LOC_BIRD_RIGHT_EYE);

    display_condition_body_loc(ch, victim, BODY_LOC_BIRD_BEAK);

    display_condition_body_loc(ch, victim, BODY_LOC_BIRD_NECK);

    display_condition_body_loc(ch, victim, BODY_LOC_BIRD_BODY);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_BIRD_LEFT_WING,
                                      BODY_LOC_BIRD_RIGHT_WING);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_BIRD_UPPER_LEFT_LEG,
                                      BODY_LOC_BIRD_LOWER_LEFT_LEG);

    display_condition_body_loc(ch, victim, BODY_LOC_BIRD_LEFT_KNEE);

    display_condition_paired_body_loc(ch, victim, BODY_LOC_BIRD_LEFT_ANKLE,
                                      BODY_LOC_BIRD_LEFT_FOOT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_BIRD_UPPER_RIGHT_LEG,
                                      BODY_LOC_BIRD_LOWER_RIGHT_LEG);

    display_condition_body_loc(ch, victim, BODY_LOC_BIRD_RIGHT_KNEE);

    display_condition_paired_body_loc(ch, victim, BODY_LOC_BIRD_RIGHT_ANKLE,
                                      BODY_LOC_BIRD_RIGHT_FOOT);

    break;

  case PHYS_TYPE_WINGED_HUMANOID:
    display_condition_body_loc(ch, victim, BODY_LOC_WINGED_HUMANOID_HEAD);

    display_condition_body_loc(ch, victim, BODY_LOC_WINGED_HUMANOID_CHIN);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_EYE,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_EAR,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_EAR);

    display_condition_body_loc(ch, victim, BODY_LOC_WINGED_HUMANOID_NECK);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_WING,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_WING);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_UPPER_TORSO,
                                      BODY_LOC_WINGED_HUMANOID_LOWER_TORSO);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_SHOULDER,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_SHOULDER);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_UPPER_LEFT_ARM,
                                      BODY_LOC_WINGED_HUMANOID_LOWER_LEFT_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_UPPER_RIGHT_ARM,
                                      BODY_LOC_WINGED_HUMANOID_LOWER_RIGHT_ARM);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_ELBOW,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_ELBOW);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_WRIST,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_WRIST,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_HAND);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_UPPER_LEFT_LEG,
                                      BODY_LOC_WINGED_HUMANOID_LOWER_LEFT_LEG);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_UPPER_RIGHT_LEG,
                                      BODY_LOC_WINGED_HUMANOID_LOWER_RIGHT_LEG);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_KNEE,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_ANKLE,
                                      BODY_LOC_WINGED_HUMANOID_LEFT_FOOT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_ANKLE,
                                      BODY_LOC_WINGED_HUMANOID_RIGHT_FOOT);

    break;

  case PHYS_TYPE_WINGED_QUADRUPED:
    display_condition_body_loc(ch, victim, BODY_LOC_WINGED_QUADRUPED_HEAD);

    display_condition_body_loc(ch, victim, BODY_LOC_WINGED_QUADRUPED_CHIN);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_LEFT_EYE,
                                      BODY_LOC_WINGED_QUADRUPED_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_LEFT_EAR,
                                      BODY_LOC_WINGED_QUADRUPED_RIGHT_EAR);

    display_condition_body_loc(ch, victim, BODY_LOC_WINGED_QUADRUPED_NECK);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_LEFT_WING,
                                      BODY_LOC_WINGED_QUADRUPED_RIGHT_WING);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_FRONT_BODY,
                                      BODY_LOC_WINGED_QUADRUPED_REAR_BODY);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_UPPER_LEFT_FRONT_LEG,
                                      BODY_LOC_WINGED_QUADRUPED_LOWER_LEFT_FRONT_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_WINGED_QUADRUPED_LEFT_FRONT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_LEFT_FRONT_ANKLE,
                                      BODY_LOC_WINGED_QUADRUPED_LEFT_FRONT_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_UPPER_RIGHT_FRONT_LEG,
                                      BODY_LOC_WINGED_QUADRUPED_LOWER_RIGHT_FRONT_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_WINGED_QUADRUPED_RIGHT_FRONT_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_RIGHT_FRONT_ANKLE,
                                      BODY_LOC_WINGED_QUADRUPED_RIGHT_FRONT_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_UPPER_LEFT_REAR_LEG,
                                      BODY_LOC_WINGED_QUADRUPED_LOWER_LEFT_REAR_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_WINGED_QUADRUPED_LEFT_REAR_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_LEFT_REAR_ANKLE,
                                      BODY_LOC_WINGED_QUADRUPED_LEFT_REAR_HOOF);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_UPPER_RIGHT_REAR_LEG,
                                      BODY_LOC_WINGED_QUADRUPED_LOWER_RIGHT_REAR_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_WINGED_QUADRUPED_RIGHT_REAR_KNEE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_WINGED_QUADRUPED_RIGHT_REAR_ANKLE,
                                      BODY_LOC_WINGED_QUADRUPED_RIGHT_REAR_HOOF);

    break;

  case PHYS_TYPE_NO_EXTREMITIES:
    display_condition_body_loc(ch, victim, BODY_LOC_NO_EXTREMITIES_HEAD);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_NO_EXTREMITIES_LEFT_EYE,
                                      BODY_LOC_NO_EXTREMITIES_RIGHT_EYE);

    display_condition_body_loc(ch, victim, BODY_LOC_NO_EXTREMITIES_BODY);

    break;

  case PHYS_TYPE_INSECTOID:
    display_condition_body_loc(ch, victim, BODY_LOC_INSECTOID_HEAD);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_LEFT_EYE,
                                      BODY_LOC_INSECTOID_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_UPPER_BODY,
                                      BODY_LOC_INSECTOID_LOWER_BODY);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_FRONT_LEFT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_FRONT_LEFT_UPPER_LEG,
                                      BODY_LOC_INSECTOID_FRONT_LEFT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_FRONT_LEFT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_FRONT_RIGHT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_FRONT_RIGHT_UPPER_LEG,
                                      BODY_LOC_INSECTOID_FRONT_RIGHT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_FRONT_RIGHT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_MIDDLE_LEFT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_MIDDLE_LEFT_UPPER_LEG,
                                      BODY_LOC_INSECTOID_MIDDLE_LEFT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_MIDDLE_LEFT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_MIDDLE_RIGHT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_MIDDLE_RIGHT_UPPER_LEG,
                                      BODY_LOC_INSECTOID_MIDDLE_RIGHT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_MIDDLE_RIGHT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_BACK_LEFT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_BACK_LEFT_UPPER_LEG,
                                      BODY_LOC_INSECTOID_BACK_LEFT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_BACK_LEFT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_BACK_RIGHT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_INSECTOID_BACK_RIGHT_UPPER_LEG,
                                      BODY_LOC_INSECTOID_BACK_RIGHT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_INSECTOID_BACK_RIGHT_LOWER_JOINT);

    break;

  case PHYS_TYPE_ARACHNID:
    display_condition_body_loc(ch, victim, BODY_LOC_ARACHNID_HEAD);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_LEFT_EYE,
                                      BODY_LOC_ARACHNID_RIGHT_EYE);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_UPPER_BODY,
                                      BODY_LOC_ARACHNID_LOWER_BODY);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_FRONT_LEFT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_FRONT_LEFT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_FRONT_LEFT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_FRONT_LEFT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_FRONT_RIGHT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_FRONT_RIGHT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_FRONT_RIGHT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_FRONT_RIGHT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE_LEFT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_MIDDLE_LEFT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_MIDDLE_LEFT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE_LEFT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE_RIGHT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_MIDDLE_RIGHT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_MIDDLE_RIGHT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE_RIGHT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE2_LEFT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_MIDDLE2_LEFT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_MIDDLE2_LEFT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE2_LEFT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE2_RIGHT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_MIDDLE2_RIGHT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_MIDDLE2_RIGHT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_MIDDLE2_RIGHT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_BACK_LEFT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_BACK_LEFT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_BACK_LEFT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_BACK_LEFT_LOWER_JOINT);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_BACK_RIGHT_UPPER_JOINT);

    display_condition_paired_body_loc(ch, victim,
                                      BODY_LOC_ARACHNID_BACK_RIGHT_UPPER_LEG,
                                      BODY_LOC_ARACHNID_BACK_RIGHT_LOWER_LEG);

    display_condition_body_loc(ch, victim,
                               BODY_LOC_ARACHNID_BACK_RIGHT_LOWER_JOINT);

    break;

  case PHYS_TYPE_BEHOLDER:
    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_BODY);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_MAIN_EYE);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK1);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK2);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK3);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK4);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK5);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK6);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK7);

    display_condition_body_loc(ch, victim, BODY_LOC_BEHOLDER_EYESTALK8);

    break;

  default:
    send_to_char
      ("victim seems to have unrecognized phys type, tell a god.\r\n", ch);
    return;
  }


  if (ch != victim)
  {
    act("$n&n judges your condition with a quick glance.", TRUE, ch, 0,
        victim, TO_VICT);
    act("$n&n judges $N&n's condition with a quick glance.", TRUE, ch, 0,
        victim, TO_NOTVICT);
  }
}
