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
#define precheck_time 10
#define prob_precheck 0.1
#define prob_check_in 0.2
#define prob_check_bag 0.4
#define num_check_in 10
#define num_bag_check 15
#define num_security 20
#define num_precheck 5

// Simulation routine
// void init_simulation(double entry, double check_in, double bag_check, double security, double precheck, double p_precheck, double p_check_in, double p_check_bag, int check_in_count, int bag_check_count, int security_count, int precheck_count);
void init_simulation();
double simulate_one_step(double current_time);

#endif
