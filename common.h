#ifndef __CS267_COMMON_H__
#define __CS267_COMMON_H__

#include <iostream>
#include <queue>
#include <random>

// Program Constants
#define entry_rate = 1;

// Simulation routine
void init_simulation(int num_people, std::queue<double> entry);
double simulate_one_step(int num_people, std::queue<double> entry);

#endif
