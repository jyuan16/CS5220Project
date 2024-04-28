// #include "common.h"
// #include <omp.h>

// // Put any static global variables here that you will use throughout the simulation.

// void init_simulation(particle_t* parts, int num_parts, double size) {
// 	// You can use this space to initialize data objects that you may need
// 	// This function will be called once before the algorithm begins
// 	// Do not do any particle simulation here
// }

// void simulate_one_step(particle_t* parts, int num_parts, double size) {
//     // Write this function
// }

#include "common.h"
#include <omp.h>
#include <cmath>

#include <stdlib.h>
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <iostream>

typedef struct node_t
{
	struct particle_t *particle;
	struct node_t *next;

	node_t(particle_t *p)
	{
		particle = p;
		next = nullptr;
	}
} node_t;

double bin_size = cutoff;
int num_bins;
// std::vector<std::vector<std::unordered_set<int>>> bins;
std::vector<std::vector<node_t *>> bins;
node_t *nodes;
// std::vector<node_t> nodes;

void rebin(int num_parts)
{
	// #pragma omp for // collapse(2) // schedule(dynamic)
	for (int x = 0; x < num_bins; x++)
	{
		for (int y = 0; y < num_bins; y++)
		{
			bins[x][y] = nullptr;
		}
	}
	// #pragma omp for // schedule(dynamic)
	for (int i = 0; i < num_parts; i++)
	{
		int x_bin = nodes[i].particle->x / bin_size;
		int y_bin = nodes[i].particle->y / bin_size;
		// #pragma omp critical
		{
			nodes[i].next = bins[x_bin][y_bin];
			bins[x_bin][y_bin] = &nodes[i];
		}
	}
}

// Apply the force from neighbor to particle
void apply_force(particle_t *particle, particle_t *neighbor)
{
	// Calculate Distance
	double dx = neighbor->x - particle->x;
	double dy = neighbor->y - particle->y;
	double r2 = dx * dx + dy * dy;

	// Check if the two particles should interact
	if (r2 > cutoff * cutoff)
		return;

	r2 = fmax(r2, min_r * min_r);
	double r = sqrt(r2);

	// Very simple short-range repulsive force
	double coef = (1 - cutoff / r) / r2 / mass;
	particle->ax += coef * dx;
	particle->ay += coef * dy;
}

// Integrate the ODE
void move(particle_t &p, double size)
{
	// Slightly simplified Velocity Verlet integration
	// Conserves energy better than explicit Euler method

	// int x_bin_old = p.x / bin_size;
	// int y_bin_old = p.y / bin_size;

	p.vx += p.ax * dt;
	p.vy += p.ay * dt;
	p.x += p.vx * dt;
	p.y += p.vy * dt;

	// Bounce from walls
	while (p.x < 0 || p.x > size)
	{
		p.x = p.x < 0 ? -p.x : 2 * size - p.x;
		p.vx = -p.vx;
	}

	while (p.y < 0 || p.y > size)
	{
		p.y = p.y < 0 ? -p.y : 2 * size - p.y;
		p.vy = -p.vy;
	}

	// int x_bin = p.x / bin_size;
	// int y_bin = p.y / bin_size;
	// if (x_bin != x_bin_old || y_bin != y_bin_old)
	// {
	//     bins[x_bin_old][y_bin_old].erase(i);
	//     bins[x_bin][y_bin].insert(i);
	// }
}

void init_simulation(particle_t *parts, int num_parts, double size)
{
// You can use this space to initialize static, global data objects
// that you may need. This function will be called once before the
// algorithm begins. Do not do any particle simulation here
#pragma omp master
	{
		num_bins = (int)((size + bin_size) / bin_size);
		nodes = (node_t *)malloc(sizeof(node_t) * num_parts);
		// nodes.reserve(num_parts);
		// bins.resize(num_bins, std::vector<std::unordered_set<int>>(num_bins, std::unordered_set<int>()));
		bins.resize(num_bins, std::vector<node_t *>(num_bins, nullptr));
		for (int i = 0; i < num_parts; i++)
		{
			// int x_bin = parts[i].x / bin_size;
			// int y_bin = parts[i].y / bin_size;
			// bins[x_bin][y_bin].insert(i);
			nodes[i] = node_t(&parts[i]);
		}
		rebin(num_parts);
	}
}

void simulate_one_step(particle_t *parts, int num_parts, double size)
{
	// #pragma omp master
	// 	{
#pragma omp barrier
#pragma omp for
	for (int x = 0; x < num_bins; x++)
	{
		for (int y = 0; y < num_bins; y++)
		{
			node_t *node = bins[x][y];
			while (node != nullptr)
			{
				node->particle->ax = node->particle->ay = 0;
				for (int a = x - 1; a <= x + 1; a++)
				{
					if (a >= 0 && a < num_bins)
					{
						for (int b = y - 1; b <= y + 1; b++)
						{
							if (b >= 0 && b < num_bins)
							{
								node_t *node2 = bins[a][b];
								while (node2 != nullptr)
								{
									apply_force(node->particle, node2->particle);
									node2 = node2->next;
								}
							}
						}
					}
				}
				node = node->next;
			}
		}
	}
// }
// #pragma omp barrier
#pragma omp master
	{
		// #pragma omp for
		for (int i = 0; i < num_parts; ++i)
		{
			move(parts[i], size);
		}
		// #pragma omp master
		// 	{
		for (int x = 0; x < num_bins; x++)
		{
			for (int y = 0; y < num_bins; y++)
			{
				bins[x][y] = nullptr;
			}
		}

		for (int i = 0; i < num_parts; i++)
		{
			int x_bin = nodes[i].particle->x / bin_size;
			int y_bin = nodes[i].particle->y / bin_size;
			// #pragma omp critical
			{
				nodes[i].next = bins[x_bin][y_bin];
				bins[x_bin][y_bin] = &nodes[i];
			}
		}
	}
	// }
}
