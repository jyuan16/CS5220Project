#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

// Program Constants
#define nsteps 1000
#define savefreq 10
#define density 0.0005
#define mass 0.01
#define cutoff 0.01
#define min_r (cutoff / 100)
#define dt 0.0005

// Particle Data Structure
typedef struct person_t
{
    double arrival_time
} person_t;

typedef struct queue_t
{
    double process_rate;
    int processing_heads;
    bool is_normal;
    queue<person_t> queue;
} queue_t;

typedef struct airport_t
{
    queue_t *entry;
    queue_t *check_in;
    queue_t *bag_check;
    queue_t *security;
    queue_t *security_precheck;
    double p_precheck;
    double p_checkin;
    double p_check_bag;
} airport_t;

// Simulation routine
void init_simulation(person_t *parts, int num_people, double end_time);
void simulate_one_step(person_t *parts, int num_people, double end_time);

#endif
