#ifndef __PROFILE_H__
#define __PROFILE_H__

#define DO_PROFILE

#ifdef DO_PROFILE

// list of active profiles
#define PROFILES(action) \
  PROFILE_##action(connections) \
  PROFILE_##action(commands) \
  PROFILE_##action(prompts) \
  PROFILE_##action(activities) \
  PROFILE_##action(combat) \
  PROFILE_##action(pulse_reset) \
  PROFILE_##action(event_loop) \
  PROFILE_##action(event_func) \
  PROFILE_##action(mundane_quest) \
  PROFILE_##action(mundane_autoinvis) \
  PROFILE_##action(mundane_wagon) \
  PROFILE_##action(mundane_wakeup) \
  PROFILE_##action(mundane_justice) \
  PROFILE_##action(mundane_commune) \
  PROFILE_##action(mundane_autostand) \
  PROFILE_##action(mundane_specproc) \
  PROFILE_##action(mundane_mobcast) \
  PROFILE_##action(mundane_track) \
  PROFILE_##action(mundane_track_1) \
  PROFILE_##action(mundane_track_2) \
  PROFILE_##action(mundane_track_3) \
  PROFILE_##action(mundane_track_4) \
  PROFILE_##action(mundane_charmbreak) \
  PROFILE_##action(mundane_curepoison) \
  PROFILE_##action(mundane_wallbreak) \
  PROFILE_##action(mundane_picktarget) \
  PROFILE_##action(mundane_attack) \
  PROFILE_##action(mundane_assist) \
  PROFILE_##action(mundane_wander) \
  PROFILE_##action(mundane_newevent)
   



#define PROFILE_DEFINE(var) \
clock_t var##_profile_beg; \
clock_t var##_profile_end; \
double var##_profile_total_inside; \
double var##_profile_total_outside; \
unsigned var##_profile_total;

#define PROFILE_DECLARE(var) \
extern clock_t var##_profile_beg; \
extern clock_t var##_profile_end; \
extern double var##_profile_total_inside; \
extern double var##_profile_total_outside; \
extern unsigned var##_profile_total;

#define PROFILE_RESET(var) \
var##_profile_beg = clock(); \
var##_profile_end = clock(); \
var##_profile_total_inside = 0; \
var##_profile_total_outside = 0; \
var##_profile_total = 0;

#define PROFILE_COUNT(var) { if (do_profile) { \
var##_profile_total++; \
}}

#define PROFILE_START(var) { if (do_profile) { \
var##_profile_beg = clock(); \
var##_profile_total_outside += (double)(var##_profile_beg - var##_profile_end); \
}}

#define PROFILE_END(var) { if (do_profile) { \
var##_profile_end = clock(); \
var##_profile_total_inside += (double)(var##_profile_end - var##_profile_beg); \
var##_profile_total++; \
}}

#define PROFILE_SAVE(var) \
save_profile_data(#var, var##_profile_total_inside, var##_profile_total_outside, var##_profile_total);

#define PROFILE_REGISTER_CALL(func, time) { if (do_profile) { \
register_func_call((void*)(func), (double) (time)); \
}}
    
extern void save_profile_data(const char* name, double total_inside, double total_outside, unsigned total);
extern bool do_profile;

extern void init_func_call_info();
extern void save_func_call_info();
extern void reset_func_call_info();

#else

#define PROFILES(action)
#define PROFILE_DEFINE(var)
#define PROFILE_DECLARE(var)
#define PROFILE_RESET(var)
#define PROFILE_START(var) 
#define PROFILE_END(var)
#define PROFILE_SAVE(var)
#define PROFILE_REGISTER_CALL(func, time)

#endif

PROFILES(DECLARE);

#endif // __PROFILE_H__

