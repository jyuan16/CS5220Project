#include "common.h"
#include <cmath>

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

// Apply the force from neighbor to particle
void apply_force(particle_t &particle, particle_t &neighbor)
{
  // Calculate Distance
  double dx = neighbor.x - particle.x;
  double dy = neighbor.y - particle.y;
  double r2 = dx * dx + dy * dy;

  // Check if the two particles should interact
  if (r2 > cutoff * cutoff)
    return;

  r2 = fmax(r2, min_r * min_r);
  double r = sqrt(r2);

  // Very simple short-range repulsive force
  double coef = (1 - cutoff / r) / r2 / mass;
  particle.ax += coef * dx;
  particle.ay += coef * dy;
}

// Integrate the ODE
void move(particle_t &p, double size)
{
  // Slightly simplified Velocity Verlet integration
  // Conserves energy better than explicit Euler method
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
}

void init_simulation(particle_t *parts, int num_parts, double size)
{
  // You can use this space to initialize static, global data objects
  // that you may need. This function will be called once before the
  // algorithm begins. Do not do any particle simulation here
}

void simulate_one_step(particle_t *parts, int num_parts, double size)
{
  // Compute Forces
  for (int i = 0; i < num_parts; ++i)
  {
    parts[i].ax = parts[i].ay = 0;
    for (int j = 0; j < num_parts; ++j)
    {
      apply_force(parts[i], parts[j]);
    }
  }

  // Move Particles
  for (int i = 0; i < num_parts; ++i)
  {
    move(parts[i], size);
  }
}
