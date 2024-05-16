// Serial implementation so make works
#include "common.h"
#include <cmath>

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
};

struct airport_t
{
    std::vector<queue_t *> check_in;          // queue_id = 1
    std::vector<queue_t *> bag_check;         // queue_id = 2
    std::vector<queue_t *> security;          // queue_id = 3
    std::vector<queue_t *> security_precheck; // queue_id = 4
} airport;

person_t next_person(0, 0, 0, 0);
std::mt19937 gen(std::random_device{}());
std::uniform_int_distribution<> queue_generator(0, 9);
std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);
std::exponential_distribution<double> entry_dist(entry_rate);
std::normal_distribution<double> check_in_dist(check_in_time, 10);
std::normal_distribution<double> bag_check_dist(bag_check_time, 10);
std::normal_distribution<double> security_dist(security_time, 10);
std::normal_distribution<double> precheck_dist(precheck_time, 3);

void init_simulation()
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

        airport.bag_check[i] = new queue_t;
        airport.bag_check[i]->processing_heads = num_bag_check;
        airport.bag_check[i]->processing_count = 0;

        airport.security[i] = new queue_t;
        airport.security[i]->processing_heads = num_security;
        airport.security[i]->processing_count = 0;

        airport.security_precheck[i] = new queue_t;
        airport.security_precheck[i]->processing_heads = num_precheck;
        airport.security_precheck[i]->processing_count = 0;
    }

    // Schedule the first person to arrive
    double time = entry_dist(gen);
    next_person = person_t(time, time, 0, 0);
}

void add_person(person_t p, queue_t *q, std::normal_distribution<double> dist)
{
    if (q->processing_count < q->processing_heads)
    {
        q->processing_count += 1;
        p.current_time += std::max(0.0, dist(gen));
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
}

void security_handler(person_t p)
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

void step(person_t p)
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
            security_handler(p);
        }
        break;
    }
    case 1:
    {
        remove_and_update(airport.check_in[p.queue_line], check_in_dist);
        security_handler(p);
        break;
    }
    case 2:
    {
        remove_and_update(airport.bag_check[p.queue_line], bag_check_dist);
        security_handler(p);
        break;
    }
    case 3:
    {
        // std::cout << p.current_time - p.arrival_time << std::endl;
        remove_and_update(airport.security[p.queue_line], security_dist);
        break;
    }
    case 4:
    {
        // std::cout << p.current_time - p.arrival_time << std::endl;
        remove_and_update(airport.security_precheck[p.queue_line], precheck_dist);
        break;
    }
    }
}

void simulate_one_step(double *time)
{
    double next_person_time = next_person.current_time + entry_dist(gen);
    step(next_person);
    next_person = person_t(next_person_time, next_person_time, 0, 0);

    for (auto queue : {airport.check_in, airport.bag_check, airport.security, airport.security_precheck})
    {
        for (int n = 0; n < num_queue; ++n)
        {
            queue_t *q = queue[n];
            if (q->processing_count == q->processing_heads)
            {
                person_t p = q->processing_queue.top();
                step(p);
            }
        }
    }
    for (auto queue : {airport.check_in, airport.bag_check, airport.security, airport.security_precheck})
    {
        for (int n = 0; n < num_queue; ++n)
        {
            if (queue[n]->processing_count > 0)
            {
                if (queue[n]->processing_queue.top().current_time > *time)
                {
                    *time = queue[n]->processing_queue.top().current_time;
                }
            }
        }
    }
}

void run_monte_carlo(int sim_count, int end)
{
    for (int i = 0; i < sim_count; ++i)
    {
        double current_time_val = 0;
        double *current_time = &current_time_val;
        init_simulation();

        while (*current_time < end)
        {
            simulate_one_step(current_time);
        }
    }
}

// #include "common.h"
// #include <cmath>
// #include <cuda.h>
// #include <curand_kernel.h>

// // Simulation Data Structure
// struct person_t
// {
//     double arrival_time;
//     double current_time;
//     int queue_id;
//     int queue_line;

//     __device__ person_t(double arrival = 0, double current = 0, int id = 0, int line = 0)
//         : arrival_time(arrival), current_time(current), queue_id(id), queue_line(line) {}
// };

// struct queue_t
// {
//     int processing_heads;
//     int processing_count;
//     person_t *processing_queue;
//     int processing_queue_size;
//     person_t *waiting_queue;
//     int waiting_queue_size;
//     int waiting_queue_capacity;

//     __device__ queue_t() : processing_heads(0), processing_count(0), processing_queue(nullptr), processing_queue_size(0),
//                            waiting_queue(nullptr), waiting_queue_size(0), waiting_queue_capacity(0) {}
// };

// struct airport_t
// {
//     queue_t *check_in;
//     queue_t *bag_check;
//     queue_t *security;
//     queue_t *security_precheck;
// } airport;

// __device__ person_t next_person;

// __global__ void setup_kernel(curandState *state)
// {
//     int idx = threadIdx.x + blockIdx.x * blockDim.x;
//     curand_init(1234, idx, 0, &state[idx]);
// }

// __global__ void init_simulation(queue_t *check_in, queue_t *bag_check, queue_t *security, queue_t *security_precheck)
// {
//     int idx = threadIdx.x + blockIdx.x * blockDim.x;

//     // Allocate memory for queues on the device
//     for (int i = 0; i < num_queue; ++i)
//     {
//         check_in[i].processing_heads = num_check_in;
//         check_in[i].processing_count = 0;
//         check_in[i].processing_queue = new person_t[1000];
//         check_in[i].processing_queue_size = 0;
//         check_in[i].waiting_queue = new person_t[1000];
//         check_in[i].waiting_queue_size = 0;
//         check_in[i].waiting_queue_capacity = 1000;

//         bag_check[i].processing_heads = num_bag_check;
//         bag_check[i].processing_count = 0;
//         bag_check[i].processing_queue = new person_t[1000];
//         bag_check[i].processing_queue_size = 0;
//         bag_check[i].waiting_queue = new person_t[1000];
//         bag_check[i].waiting_queue_size = 0;
//         bag_check[i].waiting_queue_capacity = 1000;

//         security[i].processing_heads = num_security;
//         security[i].processing_count = 0;
//         security[i].processing_queue = new person_t[1000];
//         security[i].processing_queue_size = 0;
//         security[i].waiting_queue = new person_t[1000];
//         security[i].waiting_queue_size = 0;
//         security[i].waiting_queue_capacity = 1000;

//         security_precheck[i].processing_heads = num_precheck;
//         security_precheck[i].processing_count = 0;
//         security_precheck[i].processing_queue = new person_t[1000];
//         security_precheck[i].processing_queue_size = 0;
//         security_precheck[i].waiting_queue = new person_t[1000];
//         security_precheck[i].waiting_queue_size = 0;
//         security_precheck[i].waiting_queue_capacity = 1000;
//     }

//     // Schedule the first person to arrive
//     curandState state;
//     curand_init(1234, idx, 0, &state);
//     double time = -log(1.0 - curand_uniform(&state)) / entry_rate; // Using exponential distribution
//     next_person = person_t(time, time, 0, 0);
// }

// __device__ void add_person(person_t p, queue_t *q, double dist_mean, double dist_stddev, curandState *state)
// {
//     double processing_time = curand_normal(state) * dist_stddev + dist_mean;
//     if (q->processing_count < q->processing_heads)
//     {
//         q->processing_count += 1;
//         p.current_time += max(0.0, processing_time);
//         q->processing_queue[q->processing_queue_size++] = p;
//     }
//     else
//     {
//         q->waiting_queue[q->waiting_queue_size++] = p;
//     }
// }

// __device__ void remove_and_update(queue_t *q, double dist_mean, double dist_stddev, curandState *state)
// {
//     q->processing_queue_size -= 1;
//     q->processing_count -= 1;
//     if (q->waiting_queue_size > 0)
//     {
//         person_t temp = q->waiting_queue[0];
//         for (int i = 1; i < q->waiting_queue_size; ++i)
//         {
//             q->waiting_queue[i - 1] = q->waiting_queue[i];
//         }
//         q->waiting_queue_size -= 1;
//         double processing_time = curand_normal(state) * dist_stddev + dist_mean;
//         temp.current_time += max(0.0, processing_time);
//         q->processing_queue[q->processing_queue_size++] = temp;
//         q->processing_count += 1;
//     }
// }

// __device__ void security_handler(person_t p, curandState *state)
// {
//     double p_queue = curand_uniform(state);
//     int next_queue = curand(state) % num_queue;
//     if (p_queue < prob_precheck)
//     {
//         p.queue_id = 4;
//         p.queue_line = next_queue;
//         add_person(p, &airport.security_precheck[next_queue], precheck_time, 3.0, state);
//     }
//     else
//     {
//         p.queue_id = 3;
//         p.queue_line = next_queue;
//         add_person(p, &airport.security[next_queue], security_time, 10.0, state);
//     }
// }

// __device__ void step(person_t p, curandState *state)
// {
//     switch (p.queue_id)
//     {
//     case 0:
//     {
//         double p_queue = curand_uniform(state);
//         if (p_queue < prob_check_in)
//         {
//             p.queue_id = 1;
//             p.queue_line = curand(state) % num_queue;
//             add_person(p, &airport.check_in[p.queue_line], check_in_time, 10.0, state);
//         }
//         else if (p_queue < prob_check_bag + prob_check_in)
//         {
//             p.queue_id = 2;
//             p.queue_line = curand(state) % num_queue;
//             add_person(p, &airport.bag_check[p.queue_line], bag_check_time, 10.0, state);
//         }
//         else
//         {
//             security_handler(p, state);
//         }
//         break;
//     }
//     case 1:
//     {
//         remove_and_update(&airport.check_in[p.queue_line], check_in_time, 10.0, state);
//         security_handler(p, state);
//         break;
//     }
//     case 2:
//     {
//         remove_and_update(&airport.bag_check[p.queue_line], bag_check_time, 10.0, state);
//         security_handler(p, state);
//         break;
//     }
//     case 3:
//     {
//         remove_and_update(&airport.security[p.queue_line], security_time, 10.0, state);
//         break;
//     }
//     case 4:
//     {
//         remove_and_update(&airport.security_precheck[p.queue_line], precheck_time, 3.0, state);
//         break;
//     }
//     }
// }

// __global__ void simulate_one_step(double *time, curandState *states)
// {
//     int idx = threadIdx.x + blockIdx.x * blockDim.x;
//     curandState state = states[idx];
//     double next_person_time = next_person.current_time + -log(1.0 - curand_uniform(&state)) / entry_rate;
//     step(next_person, &state);
//     next_person = person_t(next_person_time, next_person_time, 0, 0);

//     for (int n = 0; n < num_queue; ++n)
//     {
//         if (airport.check_in[n].processing_count == airport.check_in[n].processing_heads)
//         {
//             person_t p = airport.check_in[n].processing_queue[0];
//             step(p, &state);
//         }
//         if (airport.bag_check[n].processing_count == airport.bag_check[n].processing_heads)
//         {
//             person_t p = airport.bag_check[n].processing_queue[0];
//             step(p, &state);
//         }
//         if (airport.security[n].processing_count == airport.security[n].processing_heads)
//         {
//             person_t p = airport.security[n].processing_queue[0];
//             step(p, &state);
//         }
//         if (airport.security_precheck[n].processing_count == airport.security_precheck[n].processing_heads)
//         {
//             person_t p = airport.security_precheck[n].processing_queue[0];
//             step(p, &state);
//         }
//     }

//     for (int n = 0; n < num_queue; ++n)
//     {
//         if (airport.check_in[n].processing_count > 0 && airport.check_in[n].processing_queue[0].current_time > *time)
//         {
//             *time = airport.check_in[n].processing_queue[0].current_time;
//         }
//         if (airport.bag_check[n].processing_count > 0 && airport.bag_check[n].processing_queue[0].current_time > *time)
//         {
//             *time = airport.bag_check[n].processing_queue[0].current_time;
//         }
//         if (airport.security[n].processing_count > 0 && airport.security[n].processing_queue[0].current_time > *time)
//         {
//             *time = airport.security[n].processing_queue[0].current_time;
//         }
//         if (airport.security_precheck[n].processing_count > 0 && airport.security_precheck[n].processing_queue[0].current_time > *time)
//         {
//             *time = airport.security_precheck[n].processing_queue[0].current_time;
//         }
//     }
//     states[idx] = state;
// }

// void run_monte_carlo(int sim_count, int end)
// {
//     // Allocate memory for queues on the host
//     queue_t *h_check_in = (queue_t *)malloc(num_queue * sizeof(queue_t));
//     queue_t *h_bag_check = (queue_t *)malloc(num_queue * sizeof(queue_t));
//     queue_t *h_security = (queue_t *)malloc(num_queue * sizeof(queue_t));
//     queue_t *h_security_precheck = (queue_t *)malloc(num_queue * sizeof(queue_t));

//     // Allocate memory for queues on the device
//     queue_t *d_check_in;
//     queue_t *d_bag_check;
//     queue_t *d_security;
//     queue_t *d_security_precheck;
//     cudaMalloc((void **)&d_check_in, num_queue * sizeof(queue_t));
//     cudaMalloc((void **)&d_bag_check, num_queue * sizeof(queue_t));
//     cudaMalloc((void **)&d_security, num_queue * sizeof(queue_t));
//     cudaMalloc((void **)&d_security_precheck, num_queue * sizeof(queue_t));

//     // Initialize the simulation
//     init_simulation<<<1, 1>>>(d_check_in, d_bag_check, d_security, d_security_precheck);
//     cudaMemcpy(h_check_in, d_check_in, num_queue * sizeof(queue_t), cudaMemcpyDeviceToHost);
//     cudaMemcpy(h_bag_check, d_bag_check, num_queue * sizeof(queue_t), cudaMemcpyDeviceToHost);
//     cudaMemcpy(h_security, d_security, num_queue * sizeof(queue_t), cudaMemcpyDeviceToHost);
//     cudaMemcpy(h_security_precheck, d_security_precheck, num_queue * sizeof(queue_t), cudaMemcpyDeviceToHost);

//     // Setup curand states
//     curandState *d_states;
//     cudaMalloc((void **)&d_states, sim_count * sizeof(curandState));
//     setup_kernel<<<1, sim_count>>>(d_states);

//     for (int i = 0; i < sim_count; ++i)
//     {
//         double current_time_val = 0;
//         double *d_current_time;
//         cudaMalloc((void **)&d_current_time, sizeof(double));
//         cudaMemcpy(d_current_time, &current_time_val, sizeof(double), cudaMemcpyHostToDevice);

//         while (current_time_val < end)
//         {
//             simulate_one_step<<<1, sim_count>>>(d_current_time, d_states);
//             cudaMemcpy(&current_time_val, d_current_time, sizeof(double), cudaMemcpyDeviceToHost);
//         }

//         cudaFree(d_current_time);
//     }
// }
