#include "common.h"
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

// Command Line Option Processing
int find_arg_idx(int argc, char **argv, const char *option)
{
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], option) == 0)
        {
            return i;
        }
    }
    return -1;
}

int find_int_arg(int argc, char **argv, const char *option, int default_value)
{
    int iplace = find_arg_idx(argc, argv, option);

    if (iplace >= 0 && iplace < argc - 1)
    {
        return std::stoi(argv[iplace + 1]);
    }

    return default_value;
}

char *find_string_option(int argc, char **argv, const char *option, char *default_value)
{
    int iplace = find_arg_idx(argc, argv, option);

    if (iplace >= 0 && iplace < argc - 1)
    {
        return argv[iplace + 1];
    }

    return default_value;
}

// ==============
// Main Function
// ==============

int main(int argc, char **argv)
{
    // Parse Args
    if (find_arg_idx(argc, argv, "-h") >= 0)
    {
        std::cout << "Options:" << std::endl;
        std::cout << "-h: see this help" << std::endl;
        std::cout << "-d: <int>: set end time (in seconds)" << std::endl;
        std::cout << "-s: <int>: set number of simulation runs" << std::endl;
        return 0;
    }

    int end = find_int_arg(argc, argv, "-d", 1) * (24 * 60 * 60);
    int sim_count = find_int_arg(argc, argv, "-s", 1);

    // Algorithm
    auto start_time = std::chrono::steady_clock::now();
// std::cout << "yoo" << std::endl;
#pragma omp parallel default(shared)
    {
        run_monte_carlo(sim_count, end);
    }

    auto end_time = std::chrono::steady_clock::now();

    std::chrono::duration<double> diff = end_time - start_time;
    double seconds = diff.count();

    // Finalize
    std::cout << "Simulation Time = " << seconds << " seconds for " << sim_count << " simulation runs of " << end / (24 * 60 * 60) << " days.\n";
}
