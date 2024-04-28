#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

#include <iostream>
#include <queue>
#include <random>

// Program Constants
#define entry_rate = 1;

// Particle Data Structure
typedef struct person_t
{
    int id;
    double arrival_time;
    double end_time;
} person_t;

typedef struct queue_t
{
    int processing_heads;
    std::queue<person_t> queue;
    double next_time;
    bool is_norm;
    std::normal_distribution<> norm_dist;
    std::exponential_distribution<> exp_dist;
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
void init_simulation(int num_people, std::queue<double> entry);
double simulate_one_step(int num_people, std::queue<double> entry);

#endif
