#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

#include <iostream>
#include <queue>
#include <random>

// Program Constants
#define entry_rate 1
#define stddev 10
#define check_in_time 30
#define bag_check_time 30
#define security_time 30
#define precheck_time 15
#define prob_precheck 0.1
#define prob_check_in 0.2
#define prob_check_bag 0.4
#define num_check_in 10
#define num_bag_check 15
#define num_security 20
#define num_precheck 5
#define num_queue 10

// Simulation routine
void init_simulation();
void simulate_one_step(double *current_time);
void run_monte_carlo(int sim_count, int end);

#endif
