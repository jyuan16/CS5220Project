#include "common.h"
#include <cmath>

// Simulation Data Structure
struct person_t
{
  double arrival_time;
  double current_time;
  int queue_id;

  person_t(double arrival, double current, int id)
      : arrival_time(arrival), current_time(current), queue_id(id) {}
};

struct queue_t
{
  int processing_heads;
  std::priority_queue<person_t> processing_queue;
  std::queue<person_t> waiting_queue;
};

struct airport_t
{
  queue_t *entry;             // queue_id = 0
  queue_t *check_in;          // queue_id = 1
  queue_t *bag_check;         // queue_id = 2
  queue_t *security;          // queue_id = 3
  queue_t *security_precheck; // queue_id = 4
} airport;

struct CompareCurrentTime
{
  bool operator()(const person_t &p1, const person_t &p2)
  {
    return p1.current_time > p2.current_time;
  }
};

std::priority_queue<person_t, std::vector<person_t>, CompareCurrentTime> next_step;
std::mt19937 gen(std::random_device{}());
std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);
std::exponential_distribution<double> entry_dist(entry_rate);
std::normal_distribution<double> check_in_next(check_in_time, 10);
std::normal_distribution<double> bag_check_next(bag_check_time, 10);
std::normal_distribution<double> security_next(security_time, 10);
std::normal_distribution<double> precheck_next(precheck_time, 3);

void init_simulation()
{
  double time = entry_dist(gen);
  next_step.push(person_t(time, time, -1.0));
}

double simulate_one_step(double time)
{
  person_t p = next_step.top();
  next_step.pop();

  switch (p.queue_id)
  {
  case -1:
    double p_queue = uniform_dist(gen);
    if (p_queue < prob_check_in)
    {
      p.queue_id = 1;
      airport.check_in->queue.push(p);
    }
    else if (p_queue < prob_check_bag + prob_check_in)
    {
      p.queue_id = 2;
      airport.bag_check->queue.push(p);
    }
    else
    {
      security_helper();
    }

  case 0:
    double p_queue = uniform_dist(gen);
    airport.entry->queue.pop();
    if (p_queue < prob_check_in)
    {
      p.queue_id = 1;
      airport.check_in->queue.push(p);
    }
    else if (p_queue < prob_check_bag + prob_check_in)
    {
      p.queue_id = 2;
      airport.bag_check->queue.push(p);
    }
    else
    {
      p_queue = uniform_dist(gen);
      if (p_queue >= prob_precheck)
      {
        p.queue_id = 3;
        airport.security->queue.push(p);
      }
      else
      {
        p.queue_id = 4;
        airport.security_precheck->queue.push(p);
      }
    }
    double next_person_time = p.current_time + entry_dist(gen);
    next_step.push(person_t(next_person_time, next_person_time, -1.0));
    next_step.push(airport.entry->queue.front());
    return p.current_time;

  case 1:
    p.current_time = p.arrival_time + std::max(0.0, check_in_next(gen));
    airport.check_in->queue.pop();

    double p_queue = uniform_dist(gen);
    if (p_queue >= prob_precheck)
    {
      p.queue_id = 3;
      airport.security->queue.push(p);
    }
    else
    {
      p.queue_id = 4;
      airport.security_precheck->queue.push(p);
    }

    airport.check_in->queue.front().current_time = p.current_time;
    next_step.push(airport.check_in->processing_queue.top());

  case 2:
    break;

  case 3:
    break;

  case 4:
    break;
  }

  return -1; // should never happen
}
