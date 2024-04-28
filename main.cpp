#include "common.h"
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

// =================
// Helper Functions
// =================

// I/O routines
void save(std::ofstream &fsave, person_t *parts, int num_parts, double size)
{
    static bool first = true;

    if (first)
    {
        fsave << num_parts << " " << size << "\n";
        first = false;
    }

    for (int i = 0; i < num_parts; ++i)
    {
        fsave << parts[i].x << " " << parts[i].y << "\n";
    }

    fsave << std::endl;
}

// Particle Initialization
void init_particles(int num_people, int part_seed, std::queue<double> *entry)
{
    std::random_device rd;
    std::mt19937 gen(part_seed ? part_seed : rd());

    std::vector<int> shuffle(num_people);
    for (int i = 0; i < shuffle.size(); ++i)
    {
        shuffle[i] = i;
    }
    double time = 0;
    for (int i = 0; i < num_people; ++i)
    {
        // Make sure particles are not spatially sorted
        std::uniform_int_distribution<int> rand_int(0, num_people - i - 1);
        int j = rand_int(gen);
        int k = shuffle[j];
        shuffle[j] = shuffle[num_people - i - 1];

        // Distribute particles evenly to ensure proper spacing
        std::exponential_distribution<> rand_time(10);
        time += rand_time(gen);
        entry->push(time);
    }
}

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
        std::cout << "-n <int>: set number of particles" << std::endl;
        std::cout << "-o <filename>: set the output file name" << std::endl;
        std::cout << "-s <int>: set particle initialization seed" << std::endl;
        return 0;
    }

    // Open Output File
    char *savename = find_string_option(argc, argv, "-o", nullptr);
    std::ofstream fsave(savename);

    // Initialize Particles
    int num_parts = find_int_arg(argc, argv, "-n", 1000);
    int part_seed = find_int_arg(argc, argv, "-s", 0);
    int end_time = 1000;

    std::queue<double> *entry;

    init_particles(num_parts, part_seed, entry);

    // Algorithm
    auto start_time = std::chrono::steady_clock::now();

    init_simulation(num_parts, entry);

    double next_time = 0;

#ifdef _OPENMP
#pragma omp parallel default(shared)
#endif
    {
        while (next_time < end_time)
        // for (int step = 0; step < 1; ++step)
        {
            double next_time = simulate_one_step(num_parts, entry);

            // Save state if necessary
#ifdef _OPENMP
#pragma omp master
#endif
            if (fsave.good() && (step % savefreq) == 0)
            {
                save(fsave, parts, num_parts, size);
            }
        }
    }

    auto end_time = std::chrono::steady_clock::now();

    std::chrono::duration<double> diff = end_time - start_time;
    double seconds = diff.count();

    // Finalize
    std::cout << "Simulation Time = " << seconds << " seconds for " << num_parts << " particles.\n";
    fsave.close();
    delete[] parts;
}
