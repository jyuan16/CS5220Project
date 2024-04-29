#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

#include <iostream>
#include <queue>
#include <random>

// Program Constants
#define entry_rate = 1;

// Simulation routine
void init_simulation(int num_people, std::exponential_distribution<> entry_time);
double simulate_one_step(int num_people, std::exponential_distribution<> entry_time);

#endif
