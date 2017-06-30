#pragma once

#include <fcntl.h>
#include <unistd.h>
#include <sched.h>
#include <pthread.h>
#include <memory>

namespace arrp {

inline
void set_rt_scheduling(int priority)
{
    struct sched_param param;
    param.sched_priority = priority;

    int error = pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
    if (error)
        throw std::runtime_error("Failed to set RT scheduling.");
}

inline
void pin_to_cpu(int cpu)
{
    cpu_set_t cpus;
    CPU_ZERO(&cpus);
    CPU_SET(cpu, &cpus);

    int error = sched_setaffinity(pthread_self(), sizeof(cpu_set_t), &cpus);
    if (error)
        throw std::runtime_error("Failed to set thread CPU affinity.");
}

inline
void set_max_cpu_latency(int max_latency)
{
    int fd = open("/dev/cpu_dma_latency", O_RDWR);
    if (fd < 0)
        throw std::runtime_error("Failed to open /dev/cpu_dma_latency.");

    int32_t data = max_latency;
    auto result = write(fd, &data, sizeof(data));
    if (result == -1)
    {
        close(fd);
        throw std::runtime_error("Failed write to /dev/cpu_dma_latency.");
    }
}

template <typename T>
inline
T * make_aligned(int alignment = 64)
{
    auto space = sizeof(T) + alignment;
    void * d = malloc(space);
    d = std::align(alignment, sizeof(T), d, space);
    if (d == nullptr || space < sizeof(T))
    {
        throw std::runtime_error("Could not allocate aligned storage.");
    }

    return new(d) T;
}

struct stats
{
    int64_t min;
    int64_t max;
    int64_t perc50;
    int64_t perc95;
    int64_t perc99;
    double perc_minx3;
};

inline
stats compute_stats(const vector<int64_t> & series)
{
    stats s;

    vector<int64_t> d = series;

    std::sort(d.begin(), d.end());

    s.min = d.front();
    s.max = d.back();

    s.perc50 = d[d.size() / 2];
    s.perc95 = d[size_t(d.size() * 0.95)];
    s.perc99 = d[size_t(d.size() * 0.99)];

    {
        auto limit = 3 * s.min;
        size_t n = 0;
        for (; n < d.size(); ++n)
        {
            if (d[n] > limit)
                break;
        }
        s.perc_minx3 = double(n) / double(d.size()) * 100.0;
    }

    return s;
}

inline
void report_stats(const stats & s)
{
    using namespace std;

    cout << "Min = " << s.min << endl;
    cout << "50% < " << s.perc50 << endl;
    cout << "95% < " << s.perc95 << endl;
    cout << "99% < " << s.perc99 << endl;
    cout << s.perc_minx3 << "% < " << (3 * s.min) << " (min x 3)" << endl;
    cout << "Max = " << s.max << endl;
}

}
