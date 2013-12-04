#!/bin/bash

#Parses the file for attribs
parsefile ( )
{
  declare -i COUNT number
  declare -i COUNT1 number
  declare -i COUNT2 number
  declare -i FOUNDIT number
  FOUNDIT=0

  FILENAME=`grep -l "$FUNCTIONNAME" ./src/*.c`
  echo $FUNCTIONNAME

  cat $FILENAME | 
  while read LINE;
  do
    if [[ $LINE = "$FUNCTIONNAME"* ]]; then
      # Count the damn brackets and do the math
      COUNT1=`grep -o "{" <<<"$LINE" | wc -l`
      COUNT2=`grep -o "}" <<<"$LINE" | wc -l`
      COUNT=$COUNT1-$COUNT2
      # FOUNDIT -> We're inside the function.
      FOUNDIT=1
    elif [[ $FOUNDIT = 1 ]]; then
      # Hunt for the GET_C_...( crap.
      ATTRIB=$ATTRIB grep -o "GET_C_...(.." <<<"$LINE"
      # Count the damn brackets and do the math
      COUNT1=`grep -o "{" <<<"$LINE" | wc -l`
      COUNT2=`grep -o "}" <<<"$LINE" | wc -l`
      COUNT+=$COUNT1-$COUNT2

      # if at end of function
      if [[ $COUNT = 0 ]]; then
        break
      fi
    fi
  done

  echo $ATTRIB
}

#remove the old command_att. file.
rm -f command_attributes.txt

FUNCTIONNAME="void do_apply_poison"
parsefile

FUNCTIONNAME="bool backstab"
parsefile

FUNCTIONNAME="void bash"
parsefile

FUNCTIONNAME="void event_combination"
parsefile

FUNCTIONNAME="void do_charge"
parsefile

FUNCTIONNAME="bool circle"
parsefile

FUNCTIONNAME="void do_craft"
parsefile

FUNCTIONNAME="void do_dirttoss"
parsefile

FUNCTIONNAME="void do_disarm"
parsefile

FUNCTIONNAME="void do_drag"
parsefile

FUNCTIONNAME="void do_flee"
parsefile

FUNCTIONNAME="void do_flurry_of_blows"
parsefile

FUNCTIONNAME="void do_hamstring"
parsefile

FUNCTIONNAME="void do_hit"
parsefile

FUNCTIONNAME="void do_hitall"
parsefile

FUNCTIONNAME="void kick"
parsefile

FUNCTIONNAME="void do_murder"
parsefile

FUNCTIONNAME="void do_search"
parsefile

FUNCTIONNAME="void do_smith"
parsefile

FUNCTIONNAME="void do_stampede"
parsefile

FUNCTIONNAME="void do_stance"
parsefile

FUNCTIONNAME="void do_subterfuge"
parsefile

FUNCTIONNAME="void do_tackle"
parsefile

FUNCTIONNAME="void do_throat_crush"
parsefile

FUNCTIONNAME="void do_throw_potion"
parsefile

FUNCTIONNAME="void do_trap"
parsefile

FUNCTIONNAME="void do_war_cry"
parsefile

exit

actoff.c:void do_barrage(P_char ch, char *argument, int cmd)
actoff.c:void do_bash(P_char ch, char *argument, int cmd)
actoff.c:void do_buck(P_char ch, char *argument, int cmd)
actoff.c:void do_rush(P_char ch, char *argument, int cmd)
actoff.c:void do_rescue(P_char ch, char *argument, int cmd)
actoff.c:void do_maul(P_char ch, char *argument, int cmd)
actoff.c:void do_restrain(P_char ch, char *argument, int cmd)
actoff.c:void do_kick(P_char ch, char *argument, int cmd)
actoff.c:void do_roundkick(P_char ch, char *argument, int cmd)
actoff.c:void do_assist_core(P_char ch, P_char victim)
actoff.c:void do_assist(P_char ch, char *argument, int cmd)
actoff.c:void do_headbutt(P_char ch, char *argument, int cmd)
actoff.c:void do_mug(P_char ch, char *argument, int cmd)
actoff.c:void do_sneaky_strike(P_char ch, char *argument, int cmd)
actoff.c:void do_parlay(P_char ch, char *argument, int cmd)
actoff.c:void do_tackle(P_char ch, char *arg, int cmd)
actoff.c:void do_disengage(P_char ch, char *arg, int cmd)
actoff.c:void do_retreat(P_char ch, char *arg, int cmd)
actoff.c:void do_shieldpunch(P_char ch, char *argument, int cmd)
actoff.c:void do_sweeping_thrust(P_char ch, char *argument, int cmd)
actoff.c:void do_rearkick(P_char ch, char *argument, int cmd)
actoff.c:void do_trample(P_char ch, char *argument, int cmd)
actoff.c:void do_bodyslam(P_char ch, char *arg, int cmd)
actoff.c:void do_springleap(P_char ch, char *argument, int cmd)
actoff.c:void do_whirlwind(P_char ch, char *argument, int cmd)
actoff.c:void do_trip(P_char ch, char *argument, int cmd)
actoff.c:void do_flank(P_char ch, char *argument, int cmd)
actoff.c:void do_call_grave(P_char ch, char *argument, int cmd)
actoff.c:void do_battle_orders(P_char ch, char *argument, int cmd)
actoff.c:void do_gaze(P_char ch, char *argument, int cmd)
actoff.c:void do_shriek(P_char ch, char *argument, int cmd)
actoff.c:void do_dreadnaught(P_char ch, char *, int)
actoff.c:void do_shadowstep(P_char ch, char *, int)
actoff.c:void do_garrote(P_char ch, char *argument, int cmd)
actoth.c:void do_terrain(P_char ch, char *arg, int cmd)
actoth.c:void do_multiclass(P_char ch, char *arg, int cmd)
actoth.c:void do_testcolor(P_char ch, char *arg, int cmd)
actoth.c:void do_target(P_char ch, char *arg, int cmd)
actoth.c:void do_order_target(P_char ch, P_char vict, char *arg, int cmd)
actoth.c:void do_camp(P_char ch, char *arg, int cmd)
actoth.c:void do_berserk(P_char ch, char *argument, int cmd)
actoth.c:void do_rampage(P_char ch, char *argument, int cmd)
actoth.c:void do_infuriate(P_char ch, char *argument, int cmd)
actoth.c:void do_rage(P_char ch, char *argument, int cmd)
actoth.c:void do_forage(P_char ch, char *arg, int cmd)
actoth.c:void do_qui(P_char ch, char *argument, int cmd)
actoth.c:void do_quit(P_char ch, char *argument, int cmd)
actoth.c:void do_save_silent(P_char ch, int type)
actoth.c:void do_save(P_char ch, char *argument, int cmd)
actoth.c:void do_not_here(P_char ch, char *argument, int cmd)
actoth.c:void do_no_buy(P_char ch, char *argument, int cmd)
actoth.c:void do_balance(P_char ch, char *argument, int cmd)
actoth.c:void do_deposit(P_char ch, char *argument, int cmd)
actoth.c:void do_withdraw(P_char ch, char *argument, int cmd)
actoth.c:void do_sneak(P_char ch, char *argument, int cmd)
actoth.c:void do_hide(P_char ch, char *argument, int cmd)
actoth.c:void do_listen(P_char ch, char *argument, int cmd)
actoth.c:void do_steal(P_char ch, char *argument, int cmd)
actoth.c:void do_newsteal(P_char ch, char *argument, int cmd)
actoth.c:void do_explist(P_char ch, char *argument, int cmd)
actoth.c:void do_idea(P_char ch, char *argument, int cmd)
actoth.c:void do_typo(P_char ch, char *argument, int cmd)
actoth.c:void do_bug(P_char ch, char *argument, int cmd)
actoth.c:void do_cheat(P_char ch, char *argument, int cmd)
actoth.c:void do_brief(P_char ch, char *argument, int cmd)
actoth.c:void do_area(P_char ch, char *argument, int cmd)
actoth.c:void do_anonymous(P_char ch, char *argument, int cmd)
actoth.c:void do_compact(P_char ch, char *argument, int cmd)
actoth.c:void do_quaff(P_char ch, char *argument, int cmd)
actoth.c:void do_recite(P_char ch, char *argument, int cmd)
actoth.c:void do_use(P_char ch, char *argument, int cmd)
actoth.c:void do_more(P_char ch, char *arg, int cmd)
actoth.c:void do_toggle(P_char ch, char *arg, int cmd)
actoth.c:void do_rub(P_char ch, char *argument, int cmd)
actoth.c:void do_split(P_char ch, char *argument, int cmd)
actoth.c:void do_reboot(P_char ch, char *argument, int cmd)
actoth.c:void do_bury(P_char ch, char *argument, int cmd)
actoth.c:void do_dig(P_char ch, char *argument, int cmd)
actoth.c:void do_donate(P_char ch, char *argument, int cmd)
actoth.c:void do_fly(P_char ch, char *argument, int cmd)
actoth.c:void do_swim(P_char ch, char *argument, int cmd)
actoth.c:void do_suicide(P_char ch, char *argument, int cmd)
actoth.c:void do_climb(P_char ch, char *argument, int cmd)
actoth.c:void do_lick(P_char ch, char *argument, int cmd)
actoth.c:void do_nothing(P_char ch, char *argument, int cmd)
actoth.c:void do_blood_scent(P_char ch, char *argument, int cmd)
actoth.c:void do_ascend(P_char ch, char *arg, int cmd)
actoth.c:void do_descend(P_char ch, char *arg, int cmd)
actoth.c:void do_old_descend(P_char ch, char *arg, int cmd)
actset.c:void do_setbit(P_char ch, char *arg, int cmd)
actwiz.c:void do_reload_help(P_char ch, char *arg, int cmd)
actwiz.c:void do_reboot_restore(P_char ch, P_char victim)
actwiz.c:void do_read_player(P_char ch, char *arg, int cmd)
actwiz.c:void do_release(P_char ch, char *argument, int cmd)
actwiz.c:void do_emote(P_char ch, char *argument, int cmd)
actwiz.c:void do_echo(P_char ch, char *argument, int cmd)
actwiz.c:void do_newbie(P_char ch, char *argument, int cmd)
actwiz.c:void do_make_guide(P_char ch, char *argument, int cmd)
actwiz.c:void do_trans(P_char ch, char *argument, int cmd)
actwiz.c:void do_at(P_char ch, char *argument, int cmd)
actwiz.c:void do_goto(P_char ch, char *argument, int cmd)
actwiz.c:void do_affectpurge(P_char ch, char *argument, int cmd)
actwiz.c:void do_deathobj(P_char ch, char *argument, int cmd)
actwiz.c:void do_stat(P_char ch, char *argument, int cmd)
actwiz.c:void do_echoa(P_char ch, char *argument, int cmd)
actwiz.c:void do_echoz(P_char ch, char *arg, int cmd)
actwiz.c:void do_echog(P_char ch, char *arg, int cmd)
actwiz.c:void do_echoe(P_char ch, char *arg, int cmd)
actwiz.c:void do_echou(P_char ch, char *arg, int cmd)
actwiz.c:void do_shutdow(P_char ch, char *argument, int cmd)
actwiz.c:void do_wizlock(P_char ch, char *arg, int cmd)
actwiz.c:void do_nchat(P_char ch, char *argument, int cmd)
actwiz.c:void do_wizmsg(P_char ch, char *arg, int cmd)
actwiz.c:void do_shutdown(P_char ch, char *argument, int cmd)
actwiz.c:void do_snoop(P_char ch, char *argument, int cmd)
actwiz.c:void do_switch(P_char ch, char *argument, int cmd)
actwiz.c:void do_return(P_char ch, char *argument, int cmd)
actwiz.c:void do_force(P_char ch, char *argument, int cmd)
actwiz.c:void do_load(P_char ch, char *argument, int cmd)
actwiz.c:void do_purge(P_char ch, char *argument, int cmd)
actwiz.c:void do_start(P_char ch, int nomsg)
actwiz.c:void do_advance(P_char ch, char *argument, int cmd)
actwiz.c:void do_reroll(P_char ch, char *argument, int cmd)
actwiz.c:void do_restore(P_char ch, char *argument, int cmd)
actwiz.c:void do_freeze(P_char ch, char *argument, int cmd)
actwiz.c:void do_zreset(P_char ch, char *argument, int cmd)
actwiz.c:void do_reinitphys(P_char ch, char *arg, int cmd)
actwiz.c:void do_silence(P_char ch, char *argument, int cmd)
actwiz.c:void do_vis(P_char ch, char *argument, int cmd)
actwiz.c:void do_law_flags(P_char ch, char *argument, int cmd)
actwiz.c:void do_demote(P_char ch, char *argument, int cmd)
actwiz.c:void do_setattr(P_char ch, char *arg, int cmd)
actwiz.c:void do_poofIn(P_char ch, char *argument, int cmd)
actwiz.c:void do_poofOut(P_char ch, char *argument, int cmd)
actwiz.c:void do_poofInSound(P_char ch, char *argument, int cmd)
actwiz.c:void do_poofOutSound(P_char ch, char *argument, int cmd)
actwiz.c:void do_teleport(P_char ch, char *argument, int cmd)
actwiz.c:void do_wizhost(P_char ch, char *argument, int cmd)
actwiz.c:void do_ban(P_char ch, char *argument, int cmd)
actwiz.c:void do_allow(P_char ch, char *argument, int cmd)
actwiz.c:void do_secret(P_char ch, char *argument, int cmd)
actwiz.c:void do_lookup(P_char ch, char *argument, int cmd)
actwiz.c:void do_money_supply(P_char ch, char *argument, int cmd)
actwiz.c:void do_ptell(P_char ch, char *arg, int cmd)
actwiz.c:void do_finger(P_char ch, char *arg, int cmd)
actwiz.c:void do_decline(P_char ch, char *arg, int cmd)
actwiz.c:void do_accept(P_char ch, char *arg, int cmd)
actwiz.c:void do_invite(P_char ch, char *arg, int cmd)
actwiz.c:void do_uninvite(P_char ch, char *arg, int cmd)
actwiz.c:void do_clone(P_char ch, char *argument, int cmd)
actwiz.c:void do_knock(P_char ch, char *arg, int cmd)
actwiz.c:void do_ingame(P_char ch, char *args, int cmd)
actwiz.c:void do_inroom(P_char ch, char *args, int cmd)
actwiz.c:void do_which(P_char ch, char *args, int cmd)
actwiz.c:void do_grant(P_char ch, char *args, int cmd)
actwiz.c:void do_revoke(P_char ch, char *args, int cmd)
actwiz.c:void do_revoketitle(P_char ch, char *args, int cmd)
actwiz.c:void do_sethome(P_char ch, char *args, int cmd)
actwiz.c:void do_proc(P_char ch, char *args, int cmd)
actwiz.c:void do_terminate(P_char ch, char *argument, int cmd)
actwiz.c:void do_sacrifice(P_char ch, char *argument, int cmd)
actwiz.c:void do_echot(P_char ch, char *argument, int cmd)
actwiz.c:void do_unspec(P_char ch, char *argument, int cmd)
actwiz.c:void do_RemoveSpecTimer(P_char ch, char *argument, int cmd)
actwiz.c:void do_tranquilize(P_char ch, char *argument, int cmd)
actwiz.c:void do_storage(P_char ch, char *arg, int cmd)
actwiz.c:void do_newb_spellup_all(P_char ch, char *arg, int cmd)
actwiz.c:void do_newb_spellup(P_char ch, char *arg, int cmd)
actwiz.c:void do_givepet(P_char ch, char *arg, int cmd)
actwiz.c:void do_petition_block(P_char ch, char *argument, int cmd)
actwiz.c:void do_questwhere(P_char ch, char *arg, int cmd)
alliances.c:void do_alliance(P_char ch, char *arg, int cmd)
alliances.c:void do_acc(P_char ch, char *argument, int cmd)
artifact.c:void do_artifact(P_char ch, char *arg, int cmd)
assocs.c:void do_supervise(P_char god, char *argument, int cmd)
assocs.c:void do_asclist(P_char god, char *argument, int cmd)
assocs.c:void do_society(P_char member, char *argument, int cmd)
assocs.c:void do_gmotd(P_char ch, char *argument, int cmd)
assocs.c:void do_prestige(P_char ch, char *argument, int cmd)
assocs.c:void do_prestige(P_char ch, char *argument, int cmd)
auction.c:void do_auction(P_char ch, char *argument, int dummy)
automatic_rules.c:void do_raid(P_char ch, char *argument, int cmd)
avengers.c:void do_holy_smite(P_char ch, char *argument, int cmd)
bard.c:void do_bardsing(P_char ch, char *arg)
bard.c:void do_bardcheck_action(P_char ch, char *arg, int cmd)
bard.c:void do_play(P_char ch, char *arg, int cmd)
bard.c:void do_riff(P_char ch, char *arg, int cmd)
boon.c:void do_boon(P_char ch, char *argument, int cmd)
buildings.c:void do_build(P_char ch, char *argument, int cmd)
ctf.c:void do_ctf(P_char ch, char *arg, int cmd) 
ctf.c:void do_ctf(P_char ch, char *arg, int cmd)
debug.c:void do_debug(P_char ch, char *argument, int cmd)
debug.c:void do_mreport(P_char ch, char *argument, int cmd)
disguise.c:void do_disguise(P_char ch, char *arg, int cmd)
drannak.c:void do_conjure(P_char ch, char *argument, int cmd)
drannak.c:void do_dismiss(P_char ch, char *argument, int cmd)
drannak.c:void do_enhance(P_char ch, char *argument, int cmd)
dreadlord.c:void do_dread_wrath(P_char ch, P_char victim)
epic_bonus.c:void do_epic_bonus(P_char ch, char *arg, int cmd)
epic.c:void do_summon_blizzard(P_char ch, char *argument, int cmd)
epic.c:void do_summon_familiar(P_char ch, char *argument, int cmd)
epic.c:void do_epic(P_char ch, char *arg, int cmd)
epic.c://void do_epic_zones(P_char ch, char *arg, int cmd)
epic.c:/*void do_epic_reset(P_char ch, char *arg, int cmd)
epic.c:void do_epic_zones(P_char ch, char *arg, int cmd)
epic.c:void do_epic_share(P_char ch, char *arg, int cmd)
epic.c:void do_epic_trophy(P_char ch, char *arg, int cmd)
epic.c:void do_epic_skills(P_char ch, char *arg, int cmd)
epic.c:void do_infuse(P_char ch, char *arg, int cmd)
epic.c:void do_epic_reset_norefund(P_char ch, char *arg, int cmd)
epic.c:void do_epic_reset(P_char ch, char *arg, int cmd)
fight.c:void do_trophy_mob(P_char ch, char *arg, int cmd)
grapple.c:void do_bearhug(P_char ch, char *argument, int cmd)
grapple.c:void do_headlock(P_char ch, char *argument, int cmd)
grapple.c:void do_leglock(P_char ch, char *argument, int cmd)
grapple.c:void do_groundslam(P_char ch, char *argument, int cmd)
group.c:void do_group(P_char ch, char *argument, int cmd)
group.c:void do_disband(P_char ch, char *arg, int cmd)
guard.c:void do_guard(P_char ch, char *argument, int cmd)
guild.c:void do_practice_new(P_char ch, char *arg, int cmd);
guild.c:void do_spells(P_char ch, char *argument, int cmd)
guild.c:void do_skills(P_char ch, char *argument, int cmd)
guild.c:void do_practice(P_char ch, char *arg, int cmd)
guild.c:void do_practice_new( P_char ch, char *arg, int cmd )
guildhall_cmds.c:void do_construct_overmax(P_char ch, char *arg);
guildhall_cmds.c:void do_construct_guildhall(P_char ch, char *arg);
guildhall_cmds.c:void do_construct_destroy(P_char ch, char *arg);
guildhall_cmds.c:void do_construct_reset(P_char ch, char *arg);
guildhall_cmds.c:void do_construct_room(P_char ch, char *arg);
guildhall_cmds.c:void do_construct_golem(P_char ch, char *arg);
guildhall_cmds.c:void do_construct_upgrade(P_char ch, char *arg);
guildhall_cmds.c:void do_construct_rename(P_char ch, char *arg);
guildhall_cmds.c:void do_guildhall_list(P_char ch, char *arg);
guildhall_cmds.c:void do_guildhall_destroy(P_char ch, char *arg);
guildhall_cmds.c:void do_guildhall_reload(P_char ch, char *arg);
guildhall_cmds.c:void do_guildhall_status(P_char ch, char *arg);
guildhall_cmds.c:void do_guildhall_info(P_char ch, char *arg);
guildhall_cmds.c:void do_guildhall_move(P_char ch, char *arg);
guildhall_cmds.c:void do_construct(P_char ch, char *arg, int cmd)
guildhall_cmds.c:void do_construct_overmax(P_char ch, char *arg)
guildhall_cmds.c:void do_construct_guildhall(P_char ch, char *arg)
guildhall_cmds.c:void do_construct_room(P_char ch, char *arg)
guildhall_cmds.c:void do_construct_golem(P_char ch, char *arg)
guildhall_cmds.c:void do_construct_upgrade(P_char ch, char *arg)
guildhall_cmds.c:void do_construct_rename(P_char ch, char *arg)
guildhall_cmds.c:void do_guildhall(P_char ch, char *arg, int cmd)
guildhall_cmds.c:void do_guildhall_list(P_char ch, char *arg)
guildhall_cmds.c:void do_guildhall_destroy(P_char ch, char *arg)
guildhall_cmds.c:void do_guildhall_reload(P_char ch, char *arg)
guildhall_cmds.c:void do_guildhall_status(P_char ch, char *arg)
guildhall_cmds.c:void do_guildhall_info(P_char ch, char *arg)
guildhall_cmds.c:void do_guildhall_move(P_char ch, char *arg)
innates.c:void do_disappear(P_char ch, char *arg, int cmd)
innates.c:void do_innate_embrace_death(P_char ch)
innates.c:void do_innate_anti_evil(P_char ch, P_char vict)
innates.c:void do_innate_decrepify(P_char ch, P_char victim)
innates.c:void do_throw_lightning(P_char ch, char *argument, int cmd)
innates.c:void do_levitate(P_char ch, char *arg, int cmd)
innates.c:void do_bite(P_char ch, char *arg, int cmd)
innates.c:void do_innate_gaze(P_char ch, char *arg, int cmd)
innates.c:void do_darkness(P_char ch, char *arg, int cmd)
innates.c:void do_shift_astral(P_char ch, char *arg, int cmd)
innates.c:void do_shift_ethereal(P_char ch, char *arg, int cmd)
innates.c:void do_shift_prime(P_char ch, char *arg, int cmd)
innates.c:void do_blast(P_char ch, char *arg, int cmd)
innates.c:void do_faerie_fire(P_char ch, char *arg, int cmd)
innates.c:void do_ud_invisibility(P_char ch, char *arg, int cmd)
innates.c:void do_strength(P_char ch, char *arg, int cmd)
innates.c:void do_reduce(P_char ch, char *arg, int cmd)
innates.c:void do_enlarge(P_char ch, char *arg, int cmd)
innates.c:void do_wall_climbing(P_char ch, char *arg, int cmd)
innates.c:void do_flurry(P_char ch, char *arg, int cmd)
innates.c:void do_plane_shift(P_char ch, char *arg, int cmd)
innates.c:void do_charm_animal(P_char ch, char *arg, int cmd)
innates.c:void do_dispel_magic(P_char ch, char *arg, int cmd)
innates.c:void do_mass_dispel(P_char ch, char *arg, int cmd)
innates.c:void do_globe_of_darkness(P_char ch, char *arg, int cmd)
innates.c:void do_innate_hide(P_char ch, char *arg, int cmd)
innates.c:void do_project_image(P_char ch, char *arg, int cmd)
innates.c:void do_fireball(P_char ch, char *arg, int cmd)
innates.c:void do_fireshield(P_char ch, char *arg, int cmd)
innates.c:void do_firestorm(P_char ch, char *arg, int cmd)
innates.c:void do_dimension_door(P_char ch, char *arg, int cmd)
innates.c:void do_battle_rage(P_char ch, char *arg, int cmd)
innates.c:void do_phantasmal_form(P_char ch, char *arg, int cmd)
innates.c:void do_stone_skin(P_char ch, char *arg, int cmd)
innates.c:void do_shade_movement(P_char ch, char *arg, int cmd)
innates.c:void do_god_call(P_char ch, char *args, int cmd)
innates.c:void do_doorbash(P_char ch, char *arg, int cmd)
innates.c:void do_doorkick(P_char ch, char *arg, int cmd)
innates.c:void do_tupor(P_char ch, char *arg, int cmd)
innates.c:void do_breath(P_char ch, char *arg, int cmd)
innates.c:void do_stomp(P_char ch, char *arg, int cmd)
innates.c:void do_sweep(P_char ch, char *arg, int cmd)
innates.c:void do_innate(P_char ch, char *arg, int cmd)
innates.c:void do_conjure_water(P_char ch, char *arg, int cmd)
innates.c:void do_foundry(P_char ch, char *arg, int cmd)
innates.c:void do_engulf(P_char ch, char *argument, int cmd)
innates.c:void do_slime(P_char ch, char *argument, int cmd)
innates.c:void do_branch(P_char ch, char *argument, int cmd)
innates.c:void do_webwrap(P_char ch, char *argument, int cmd)
innates.c:void do_summon_imp(P_char ch, char *argument, int cmd)
innates.c:void do_lifedrain(P_char ch, char *argument, int cmd)
innates.c:void do_immolate(P_char ch, char *argument, int cmd)
innates.c:void do_fade(P_char ch, char *argument, int cmd)
innates.c:void do_layhand(P_char ch, char *argument, int cmd)
innates.c:void do_aura_protection(P_char ch, char *arg, int cmd) {
innates.c:void do_aura_precision(P_char ch, char *arg, int cmd) {
innates.c:void do_aura_battlelust(P_char ch, char *arg, int cmd) {
innates.c:void do_aura_healing(P_char ch, char *arg, int cmd) {
innates.c:void do_aura_endurance(P_char ch, char *arg, int cmd) {
innates.c:void do_aura_vigor(P_char ch, char *arg, int cmd) {
innates.c:void do_divine_force(P_char ch, char *arg, int cmd)
interp.c:void do_prestige(P_char ch, char *argument, int cmd);
interp.c:void do_confirm(P_char ch, int yes)
justice.c:void do_justice(P_char ch, char *arg, int cmd)
justice.c:void do_report_crime(P_char ch, char *arg, int cmd)
justice.c:void do_sorta_yell(P_char ch, char *str)
language.c:void do_speak(P_char ch, char *argument, int cmd)
magic.c:void do_nothing_spell(int level, P_char ch, char *arg, int type,
magic.c:void do_pleasantry(P_char ch, char *argument, int cmd)
magic.c:void do_soulbind(P_char ch, char *argument, int cmd)
mail.c:void do_mail(P_char ch, char *arg, int cmd)
makeexit.c:void do_makeexit(P_char ch, char *arg, int cmd)
memorize.c:void do_assimilate(P_char ch, char *argument, int cmd)
memorize.c:void do_npc_commune(P_char ch)
memorize.c:void do_memorize(P_char ch, char *argument, int cmd)
memorize.c:void do_forget(P_char ch, char *argument, int cmd)
memorize.c:void do_teach(P_char ch, char *arg, int cmd)
memorize.c:void do_scribe(P_char ch, char *arg, int cmd)
mobact.c:void do_npc_commune(P_char ch);
modify.c:void do_string(P_char ch, char *arg, int cmd)
modify.c:void do_rename(P_char ch, char *arg, int cmd)
mount.c:void do_mount(P_char ch, char *argument, int cmd)
mount.c:void do_dismount(P_char ch, char *argument, int cmd)
mount.c:void do_hitch_vehicle(P_char ch, char *arg, int cmd)
mount.c:void do_unhitch_vehicle(P_char ch, char *arg, int cmd)
multiplay_whitelist.c:void do_whitelist_help(P_char ch)
multiplay_whitelist.c:void do_whitelist(P_char ch, char *argument, int cmd)
necromancy.c:void do_exhume(P_char ch, char *argument, int cmd)
necromancy.c:void do_spawn(P_char ch, char *argument, int cmd)
necromancy.c:void do_summon_host(P_char ch, char *argument, int cmd)
necromancy.c:void do_remort(P_char ch, char *arg, int cmd) 
new_combat_user.c:void do_condition(P_char ch, char *argument, int cmd)
new_skills.c:void do_awareness(P_char ch, char *argument, int cmd)
new_skills.c:void do_feign_death(P_char ch, char *arg, int cmd)
new_skills.c:void do_first_aid(P_char ch, char *arg, int cmd)
new_skills.c:void do_chant(P_char ch, char *argument, int cmd)
new_skills.c:void do_dragon_punch(P_char ch, char *argument, int cmd)
new_skills.c:void do_OLD_bandage(P_char ch, char *arg, int cmd)
new_skills.c:void do_summon_book(P_char ch, char *arg, int cmd)
new_skills.c:void do_summon_totem(P_char ch, char *arg, int cmd)
new_skills.c:void do_summon_mount(P_char ch, char *arg, int cmd)
new_skills.c:void do_summon_warg(P_char ch, char *arg, int cmd)
new_skills.c:void do_summon_orc(P_char ch, char *arg, int cmd)
new_skills.c:void do_ogre_roar(P_char ch, char *argument, int cmd)
new_skills.c:void do_carve(struct char_data *ch, char *argument, int cmd)
new_skills.c:void do_bind(P_char ch, char *arg, int cmd)
new_skills.c:void do_unbind(P_char ch, char *arg, int cmd)
new_skills.c:void do_capture(P_char ch, char *argument, int cmd)
new_skills.c:void do_appraise(P_char ch, char *argument, int cmd)
new_skills.c:void do_chi(P_char ch, char *argument, int cmd)
new_skills.c:void do_lotus(P_char ch, char *argument, int cmd)
new_skills.c:void do_true_strike(P_char ch, char *argument, int cmd)
nexus_stones.c:void do_nexus(P_char ch, char *arg, int cmd) 
nexus_stones.c:void do_nexus(P_char ch, char *arg, int cmd)
nq.c:void do_quest2(P_char ch, char *args, int cmd)
olc.c:void do_olc(P_char ch, char *arg, int cmd)
old_guildhalls.c:void do_build_secret(P_house house, P_char ch);
old_guildhalls.c:void do_construct_guild(P_char, int);
old_guildhalls.c:void do_delete_room(P_house house, P_char ch, char *arg);
old_guildhalls.c:void do_construct(P_char ch, char *arg, int cmd)
old_guildhalls.c:void do_hcontrol(P_char ch, char *arg, int cmd)
old_guildhalls.c:void do_house(P_char ch, char *arg, int cmd)
old_guildhalls.c:void do_delete_room(P_house house, P_char ch, char *arg)
old_guildhalls.c:void do_build_window(P_house house, P_char ch)
old_guildhalls.c:void do_build_movedoor(P_house house, P_char ch, char *arg)
old_guildhalls.c:void do_build_heal(P_house house, P_char ch)
old_guildhalls.c:void do_build_secret(P_house house, P_char ch)
old_guildhalls.c:void do_build_holy(P_house house, P_char ch)
old_guildhalls.c:void do_build_unholy(P_house house, P_char ch)
old_guildhalls.c:void do_build_mouth(P_house house, P_char ch)
old_guildhalls.c:void do_build_teleporter(P_house house, P_char ch, char *arg)
old_guildhalls.c:void do_build_shop(P_house house, P_char ch)
old_guildhalls.c:void do_build_chest(P_house house, P_char ch)
old_guildhalls.c:void do_build_fountain(P_house house, P_char ch)
old_guildhalls.c:void do_build_inn(P_house house, P_char ch)
old_guildhalls.c:void do_build_golem(P_house house, P_char ch, int type)
old_guildhalls.c:void do_describe_room(P_house house, P_char ch, char *arg)
old_guildhalls.c:void do_name_room(P_house house, P_char ch, char *arg)
old_guildhalls.c:void do_construct_room(P_house house, P_char ch, char *arg)
old_guildhalls.c:void do_show_q(P_char ch)
old_guildhalls.c:void do_sack(P_char ch, char *arg, int cmd)
old_guildhalls.c:void do_construct_guild(P_char ch, int type)
old_guildhalls.c:void do_stathouse(P_char ch, char *argument, int cmd)
outposts.c:void do_outpost(P_char ch, char *arg, int cmd) 
outposts.c:void do_outpost(P_char ch, char *arg, int cmd)
paladins.c:void do_aura(P_char ch, int aura)
properties.c:void do_properties(P_char ch, char *args, int cmd)
psionics.c:void do_drain(P_char ch, char *arg, int cmd)
psionics.c:void do_gith_neckbite(P_char ch, char *arg, int cmd)
psionics.c:void do_absorbe(P_char ch, char *arg, int cmd)
quest.c:void do_reload_quest(P_char ch, char *arg, int cmd)
randobj.c:void do_randobj(P_char ch, char *strn, int val)
range.c:void do_gather(P_char ch, char *argument, int cmd)
range.c:void do_fire(P_char ch, char *argument, int cmd)
range.c:void do_throw(P_char ch, char *argument, int cmd)
range.c:void do_load_weapon(P_char ch, char *argument, int cmd)
range.c:void do_cover(P_char ch, char *argument, int cmd)
rogues.c:void do_slip(P_char ch, char *argument, int cmd)
salchemist.c:void do_mix(P_char ch, char *argument, int cmd)
salchemist.c:void do_spellbind (P_char ch, char *argument, int cmd)
salchemist.c:void do_encrust(P_char ch, char *argument, int cmd)
salchemist.c:void do_fix(P_char ch, char *argument, int cmd)
salchemist.c:void do_smelt(P_char ch, char *arg, int cmd)
salchemist.c:void do_enchant(P_char ch, char *argument, int cmd)
shadow.c:void do_shadow(P_char ch, char *argument, int cmd)
shadow.c:void do_grapple(P_char ch, char *arg, int cmd) {
skills.c:extern void do_epic_reset(P_char ch, char *arg, int cmd);
skills.c:extern void do_epic_reset_norefund(P_char ch, char *arg, int cmd);
smagic.c:void do_point(P_char ch, P_char victim)
sparser.c:void do_will(P_char ch, char *argument, int cmd)
sparser.c:void do_cast(P_char ch, char *argument, int cmd)
specializations.c:void do_spec_list(P_char ch)
specializations.c:void do_specialize(P_char ch, char *argument, int cmd)
specs.dragonnia.c:void do_mobdisarm(P_char ch, char *argument, int cmd)
specs.eth2.c:void do_assist_core(P_char ch, P_char victim);
specs.library.c:void do_proclibObj(P_char ch, char *argument)
specs.library.c:void do_proclibMob(P_char ch, char *argument)
specs.library.c:void do_proclibRoom(P_char ch, char *argument)
specs.library.c:void do_proclib(P_char ch, char *argument, int cmd)
specs.zion.c:void do_dispator_remove(P_char ch)
sql.c:void do_sql(P_char ch, char *argument, int cmd)
sql.c:void do_sql(P_char ch, char *argument, int cmd)
statistics.c:void do_statistic(P_char ch, char *argument, int val)
testcmd.c:void do_test_room(P_char ch, char *arg, int cmd)
testcmd.c:void do_test_writemap(P_char ch, char *arg, int cmd)
testcmd.c:void do_test_add_epic_skillpoint(P_char ch, const char *charname)
testcmd.c:void do_test(P_char ch, char *arg, int cmd)
tether.c:void do_tether( P_char ch, char *argument, int cmd )
track.c:void do_track_not_in_use(P_char ch, char *arg, int cmd)
track.c:void do_track(P_char ch, char *arg, int cmd) //do_track_not_in_use
tradeskill.c:void do_forge(P_char ch, char *argument, int cmd)
tradeskill.c:void do_fish(P_char ch, char*, int cmd)
tradeskill.c:void do_mine(P_char ch, char *arg, int cmd)
tradeskill.c:void do_bandage(P_char ch, char *arg, int cmd)
tradeskill.c:void do_salvation(P_char ch, char *arg, int cmd)
tradeskill.c:void do_drandebug(P_char ch, char *arg, int cmd)
tradeskill.c:void do_refine(P_char ch, char *arg, int cmd)
tradeskill.c:void do_dice(P_char ch, char *arg, int cmd)
trap.c:void do_trapremove(P_char ch, char *argument, int cmd)
trap.c:void do_trapstat(P_char ch, char *argument, int cmd)
trap.c:void do_traplist(P_char ch, char *argument, int cmd)
trap.c:void do_trapset(P_char ch, char *argument, int cmd)
trophy.c:void do_trophy(P_char ch, char *arg, int cmd)
utility.c:void do_introduce(P_char ch, char *arg, int level)
utility.c:void do_testdesc(P_char ch, char *arg, int level)
world_quest.c:void do_quest(P_char ch, char *args, int cmd)
