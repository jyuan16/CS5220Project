#include "common.h"
#include <chrono>
#include <cmath>
#include <cstring>
#include <cuda.h>
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
        return 0;
    }

    // Open Output File
    char *savename = find_string_option(argc, argv, "-o", nullptr);
    std::ofstream fsave(savename);

    // Initialize Particles
    int end = find_int_arg(argc, argv, "-d", 1) * (24 * 60 * 60);

    // int num_parts = find_int_arg(argc, argv, "-n", 1000);
    // int part_seed = find_int_arg(argc, argv, "-s", 0);
    // double size = sqrt(density * num_parts);

    // particle_t *parts = new particle_t[num_parts];

    // init_particles(parts, num_parts, size, part_seed);

    // particle_t *parts_gpu;
    // cudaMalloc((void **)&parts_gpu, num_parts * sizeof(particle_t));
    // cudaMemcpy(parts_gpu, parts, num_parts * sizeof(particle_t), cudaMemcpyHostToDevice);

    // Algorithm
    auto start_time = std::chrono::steady_clock::now();

    init_simulation();
    double current_time_val = 0;
    double *current_time = &current_time_val;

    while (*current_time < end)
    {
        simulate_one_step(current_time);
        cudaDeviceSynchronize();
    }

    cudaDeviceSynchronize();
    auto end_time = std::chrono::steady_clock::now();

    std::chrono::duration<double> diff = end_time - start_time;
    double seconds = diff.count();

    // Finalize
    std::cout << "Simulation Time = " << seconds << " seconds for " << end / (24 * 60 * 60) << " days.\n";
}