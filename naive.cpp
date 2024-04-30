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

struct CompareCurrentTime
{
  bool operator()(const person_t &p1, const person_t &p2)
  {
    return p1.current_time > p2.current_time;
  }
};

struct queue_t
{
  int processing_heads;
  int processing_count;
  std::priority_queue<person_t, std::vector<person_t>, CompareCurrentTime> processing_queue;
  std::queue<person_t> waiting_queue;
};

struct airport_t
{
  queue_t *check_in;          // queue_id = 1
  queue_t *bag_check;         // queue_id = 2
  queue_t *security;          // queue_id = 3
  queue_t *security_precheck; // queue_id = 4
} airport;

std::priority_queue<person_t, std::vector<person_t>, CompareCurrentTime> next_step;
std::mt19937 gen(std::random_device{}());
std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);
std::exponential_distribution<double> entry_dist(entry_rate);
std::normal_distribution<double> check_in_dist(check_in_time, 10);
std::normal_distribution<double> bag_check_dist(bag_check_time, 10);
std::normal_distribution<double> security_dist(security_time, 10);
std::normal_distribution<double> precheck_dist(precheck_time, 3);

void init_simulation()
{
  airport.check_in = new queue_t{};
  airport.bag_check = new queue_t{};
  airport.security = new queue_t{};
  airport.security_precheck = new queue_t{};

  std::vector<int> processing_heads_count = {num_check_in, num_bag_check, num_security, num_precheck};
  int cur_q = 0;

  for (auto queue : {airport.check_in, airport.bag_check, airport.security, airport.security_precheck})
  {
    queue->processing_heads = processing_heads_count[cur_q];
    queue->processing_count = 0;
  }
  double time = entry_dist(gen);
  person_t p = person_t(time, time, 0.0);
  next_step.push(p);
}

void add_person(person_t p, queue_t *q, std::normal_distribution<double> dist)
{
  if (q->processing_count < q->processing_heads)
  {
    q->processing_count += 1;
    p.current_time += std::max(0.0, dist(gen));
    if (q->processing_count == 1)
    {
      next_step.push(p);
    }
    q->processing_queue.push(p);
  }
  else
  {
    q->waiting_queue.push(p);
  }
}

void remove_and_update(queue_t *q, std::normal_distribution<double> dist)
{
  q->processing_queue.pop();
  q->processing_count -= 1;
  if (!q->waiting_queue.empty())
  {
    person_t temp = q->waiting_queue.front();
    q->waiting_queue.pop();
    temp.current_time += std::max(0.0, dist(gen));
    q->processing_queue.push(temp);
    q->processing_count += 1;
  }
  if (q->processing_count > 0)
  {
    next_step.push(q->processing_queue.top());
  }
}

void security_handler(person_t p)
{
  double p_queue = uniform_dist(gen);
  if (p_queue < prob_precheck)
  {
    p.queue_id = 4;
    add_person(p, airport.security_precheck, precheck_dist);
  }
  else
  {
    p.queue_id = 3;
    add_person(p, airport.security, security_dist);
  }
}

double simulate_one_step(double time)
{
  if (next_step.empty())
  {
    return -1; // should never happen
  }
  person_t p = next_step.top();
  next_step.pop();

  switch (p.queue_id)
  {
  case 0:
  {
    double p_queue = uniform_dist(gen);
    if (p_queue < prob_check_in)
    {
      p.queue_id = 1;
      add_person(p, airport.check_in, check_in_dist);
    }
    else if (p_queue < prob_check_bag + prob_check_in)
    {
      p.queue_id = 2;
      add_person(p, airport.bag_check, bag_check_dist);
    }
    else
    {
      security_handler(p);
    }
    double next_person_time = p.current_time + entry_dist(gen);
    next_step.push(person_t(next_person_time, next_person_time, 0.0));
    break;
  }
  case 1:
  {
    remove_and_update(airport.check_in, check_in_dist);
    security_handler(p);
    break;
  }
  case 2:
  {
    remove_and_update(airport.bag_check, bag_check_dist);
    security_handler(p);
    break;
  }
  case 3:
  {
    remove_and_update(airport.security, security_dist);
    break;
  }
  case 4:
  {
    remove_and_update(airport.security_precheck, precheck_dist);
    break;
  }
  }
  return p.current_time;
}
