#include "common.h"
#include <cmath>
#include <omp.h>
#include <cstdlib>
#include <queue>
#include <random>
#include <vector>
#include <algorithm>

// Simulation Data Structure
struct person_t
{
	double arrival_time;
	double current_time;
	int queue_id;
	int queue_line;

	person_t(double arrival, double current, int id, int line)
			: arrival_time(arrival), current_time(current), queue_id(id), queue_line(line) {}
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
	omp_lock_t writelock;
};

struct airport_t
{
	std::vector<queue_t *> check_in;					// queue_id = 1
	std::vector<queue_t *> bag_check;					// queue_id = 2
	std::vector<queue_t *> security;					// queue_id = 3
	std::vector<queue_t *> security_precheck; // queue_id = 4
};

std::mt19937 gen(std::random_device{}());
std::uniform_int_distribution<> queue_generator(0, 9);
std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);
std::exponential_distribution<double> entry_dist(entry_rate);
std::normal_distribution<double> check_in_dist(check_in_time, 10);
std::normal_distribution<double> bag_check_dist(bag_check_time, 10);
std::normal_distribution<double> security_dist(security_time, 10);
std::normal_distribution<double> precheck_dist(precheck_time, 3);

void init_simulation(airport_t &airport, person_t next_person)
{
	// Resize the vectors to the number of queues needed for each type of queue
	airport.check_in.resize(num_queue);
	airport.bag_check.resize(num_queue);
	airport.security.resize(num_queue);
	airport.security_precheck.resize(num_queue);

	// Initialize the queues in each vector
	for (int i = 0; i < num_queue; i++)
	{
		airport.check_in[i] = new queue_t;
		airport.check_in[i]->processing_heads = num_check_in;
		airport.check_in[i]->processing_count = 0;
		omp_init_lock(&airport.check_in[i]->writelock);

		airport.bag_check[i] = new queue_t;
		airport.bag_check[i]->processing_heads = num_bag_check;
		airport.bag_check[i]->processing_count = 0;
		omp_init_lock(&airport.bag_check[i]->writelock);

		airport.security[i] = new queue_t;
		airport.security[i]->processing_heads = num_security;
		airport.security[i]->processing_count = 0;
		omp_init_lock(&airport.security[i]->writelock);

		airport.security_precheck[i] = new queue_t;
		airport.security_precheck[i]->processing_heads = num_precheck;
		airport.security_precheck[i]->processing_count = 0;
		omp_init_lock(&airport.security_precheck[i]->writelock);
	}

	// Schedule the first person to arrive
	double time = entry_dist(gen);
	next_person = person_t(time, time, 0, 0);
}

void add_person(person_t p, queue_t *q, std::normal_distribution<double> dist)
{
	omp_set_lock(&q->writelock);
	if (q->processing_count < q->processing_heads)
	{
		p.current_time += std::max(0.0, dist(gen));

		q->processing_count += 1;
		q->processing_queue.push(p);
	}
	else
	{
		q->waiting_queue.push(p);
	}
	omp_unset_lock(&q->writelock);
}

void remove_and_update(queue_t *q, std::normal_distribution<double> dist)
{
	omp_set_lock(&q->writelock);

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
	omp_unset_lock(&q->writelock);
}

void security_handler(person_t p, airport_t &airport)
{
	double p_queue = uniform_dist(gen);
	int next_queue = queue_generator(gen);
	if (p_queue < prob_precheck)
	{
		p.queue_id = 4;
		p.queue_line = next_queue;
		add_person(p, airport.security_precheck[next_queue], precheck_dist);
	}
	else
	{
		p.queue_id = 3;
		p.queue_line = next_queue;
		add_person(p, airport.security[next_queue], security_dist);
	}
}

void step(person_t p, airport_t &airport)
{
	switch (p.queue_id)
	{
	case 0:
	{
		double p_queue = uniform_dist(gen);
		if (p_queue < prob_check_in)
		{
			p.queue_id = 1;
			p.queue_line = queue_generator(gen);
			add_person(p, airport.check_in[p.queue_line], check_in_dist);
		}
		else if (p_queue < prob_check_bag + prob_check_in)
		{
			p.queue_id = 2;
			p.queue_line = queue_generator(gen);
			add_person(p, airport.bag_check[p.queue_line], bag_check_dist);
		}
		else
		{
			security_handler(p, airport);
		}
		break;
	}
	case 1:
	{
		remove_and_update(airport.check_in[p.queue_line], check_in_dist);
		security_handler(p, airport);
		break;
	}
	case 2:
	{
		remove_and_update(airport.bag_check[p.queue_line], bag_check_dist);
		security_handler(p, airport);
		break;
	}
	case 3:
	{
		remove_and_update(airport.security[p.queue_line], security_dist);
		break;
	}
	case 4:
	{
		remove_and_update(airport.security_precheck[p.queue_line], precheck_dist);
		break;
	}
	}
}

void simulate_one_step(double &current_time, airport_t &airport, person_t &next_person)
{
	// Handle the next person arrival
	{
		double next_person_time = current_time + entry_dist(gen);
		step(next_person, airport);
		next_person = person_t(next_person_time, next_person_time, 0, 0);
	}

	// Process queues
	for (int n = 0; n < num_queue; ++n)
	{
		for (auto queue : {airport.check_in, airport.bag_check, airport.security, airport.security_precheck})
		{
			queue_t *q = queue[n];
			if (q->processing_count == q->processing_heads && !q->processing_queue.empty())
			{
				person_t p = q->processing_queue.top();
				step(p, airport);
			}
		}
	}

	// Update current time
	for (int n = 0; n < num_queue; ++n)
	{
		for (auto queue : {airport.check_in, airport.bag_check, airport.security, airport.security_precheck})
		{
			if (queue[n]->processing_count > 0 && !queue[n]->processing_queue.empty())
			{
				current_time = std::max(current_time, queue[n]->processing_queue.top().current_time);
			}
		}
	}
}

void run_monte_carlo(int sim_count, int end)
{
#pragma omp for
	for (int i = 0; i < sim_count; ++i)
	{
		person_t next_person(0, 0, 0, 0);
		airport_t private_airport;

		// Initialize the private airport for each thread
		init_simulation(private_airport, next_person);

		double current_time = 0;

		while (current_time < end)
		{
			simulate_one_step(current_time, private_airport, next_person);
		}
	}
}
